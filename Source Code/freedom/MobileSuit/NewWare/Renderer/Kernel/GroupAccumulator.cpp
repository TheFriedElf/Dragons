
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : GroupAccumulator.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#include "stdafx.h"
#include "GroupAccumulator.h"

#include "BatchRenderUtils.h"
#include "RenderStateTagExtraData.h"

#include "../../Scene/NodeTraversal.h"
#include "../../Scene/ApplyProperty.h"

#define _COMPILE_TEXTURE_SORTED_OPAQUE_RENDERING


/** OpenMP 설정 관련: <보관용 - False Sharing 문제 때문에 보류!>
    1) Manifest
       <dependency>
         <dependentAssembly>
           <assemblyIdentity type="win32" name="Microsoft.VC80.OpenMP" version="8.0.50727.4053" processorArchitecture="x86" publicKeyToken="1fc8b3b9a1e18e3b"/>
         </dependentAssembly>
       </dependency>
    2) Source File
       #include <omp.h>
       #if defined(_OPENMP)
           #pragma omp parallel for
       #endif //#if defined(_OPENMP)
    3) Compile/Link Option
       OpenMP활성
*/


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

NiImplementRTTI(GroupAccumulator, NiAccumulator);

DWORD GroupAccumulator::CREATION_NUMBER = 0;

//-----------------------------------------------------------------------------------

GroupAccumulator::GroupAccumulator( std::auto_ptr<RenderStateBlockCache> spRSBlockCache, 
                                    unsigned int uiRenderGroupListSize )
    : m_spRenderStateBlockCache(spRSBlockCache), 
      m_bSortByClosestPoint(false), 
      m_bObserveAlphaNoSortHint(true), 
      m_bObserveOpaqueNoSortHint(false), 
      m_bIsDepthSortedOpaqueRendering(true), 
      m_bIsDumpRenderGroupList(false)
{
    assert( m_spRenderStateBlockCache.get() != NULL );

    ReserveRenderGroupList( uiRenderGroupListSize );
    ResizeAssortHelpers( ASSORTHELPERS_DEF_SIZE );
}

//-----------------------------------------------------------------------------------

GroupAccumulator::~GroupAccumulator()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::BuildRenderStateBlockCache( NiAVObject* pkAVObject )
{
    struct _Functor
    {
        explicit _Functor( GroupAccumulator* pkAccumu ) : pkAccumulator(pkAccumu) { assert( pkAccumulator ); };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeometry = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeometry && pkGeometry->GetSortObject() )
            {
                NiPropertyState const* pkState = pkGeometry->GetPropertyState();
                assert( pkState );
                NiAlphaProperty const* pkAlpha = pkState->GetAlpha();
                assert( pkAlpha );

                if ( Scene::ApplyProperty::Override::RepairFalseAlphaGeometry(pkGeometry) || 
                     ((pkAlpha->GetAlphaBlending() == false) && 
                      (pkAccumulator->GetObserveOpaqueNoSortHint() == false) && 
                      (!pkAccumulator->GetObserveAlphaNoSortHint() || pkAlpha->GetNoSorter())) )
                {
                    RenderStateTagExtraData* pkExtraData;
                    pkAccumulator->CreateRenderStateBlock( *pkGeometry, pkExtraData );
                }
            }
        }

        GroupAccumulator* pkAccumulator;
    } kFunctor( this );

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Emergent internal use only stuff

void GroupAccumulator::StartAccumulating( NiCamera const* pkCamera )
{
    PROFILE_FUNC();

    assert( IsAccumulating() == false );

    NiAccumulator::StartAccumulating( pkCamera );
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::RegisterObjectArray( NiVisibleArray& kArray )
{
    PROFILE_FUNC();

    assert( IsAccumulating() == true );

	NiRenderer* pkRenderer = NiRenderer::GetRenderer();

	const unsigned int uiQuantity = kArray.GetCount();
	for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
	{
		NiGeometry& kObject = kArray.GetAt( ui );

		const NiPropertyState* pkState = kObject.GetPropertyState();
		assert( pkState );

        AddToRenderGroup( pkRenderer, kObject, pkState->GetAlpha() );
	}
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::FinishAccumulating()
{
    PROFILE_FUNC();

    assert( IsAccumulating() == true );

    SortRenderGroupItems();

    OnDumpRenderGroupList();

    FlushRenderGroupItems();

    ClearAccumulateBuffer();

    NiAccumulator::FinishAccumulating();
}

// Emergent internal use only stuff
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::SortRenderGroupItems()
{
    PROFILE_FUNC();

    if ( m_bIsDepthSortedOpaqueRendering )
    {
        RenderGroupList& vOpaqueGroupList = m_avRenderGroupList[RGL_OPAQUE];
        for ( RenderGroupListIter iter = vOpaqueGroupList.begin(); iter != vOpaqueGroupList.end(); ++iter )
        {
            SortByDepth( (*iter)->kSortItems );
        }
    }

    RenderGroupList& vAlphaGroupList = m_avRenderGroupList[RGL_ALPHA];
    for ( RenderGroupListIter iter = vAlphaGroupList.begin(); iter != vAlphaGroupList.end(); ++iter )
    {
        SortByDepth( (*iter)->kSortItems );
    }

#if defined(_COMPILE_TEXTURE_SORTED_OPAQUE_RENDERING)
    SortOpaqueGroupByTexture();
#endif //#if defined(_COMPILE_TEXTURE_SORTED_OPAQUE_RENDERING)
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::ClearRenderGroupItems()
{
    PROFILE_FUNC();

    for ( unsigned int ui = 0; ui < RGL_MAX_COUNT; ++ui )
    {
        for ( RenderGroupListIter iter = m_avRenderGroupList[ui].begin(); 
              iter != m_avRenderGroupList[ui].end(); ++iter )
        {
            (*iter)->Clear();
            delete (*iter);
        }
        m_avRenderGroupList[ui].clear();
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Flush functions stuff

void GroupAccumulator::FlushAlphaRenderGroupItems( NiRenderer* pkRenderer )
{
    PROFILE_FUNC();

    assert( pkRenderer );

    int iCurrItem;
    NiGeometry* pkItem;

    RenderGroupList& vRenderGroupList = m_avRenderGroupList[RGL_ALPHA];

    for ( RenderGroupListCIter citer = vRenderGroupList.begin(); citer != vRenderGroupList.end(); ++citer )
    {
        SortItems const& kSortItems = (*citer)->kSortItems;

        iCurrItem = kSortItems.iNumItems;

        // Back to front rendering!
        pkItem = (0 != iCurrItem)? kSortItems.ppkItems[--iCurrItem]: NULL;
        while ( pkItem )
        {
            pkItem->RenderImmediate( pkRenderer );

            pkItem = (0 != iCurrItem)? kSortItems.ppkItems[--iCurrItem]: NULL;
        }
    }
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::FlushOpaqueRenderGroupItems( NiRenderer* pkRenderer )
{
    PROFILE_FUNC();

    assert( pkRenderer );

    RenderGroupList& vRenderGroupList = m_avRenderGroupList[RGL_OPAQUE];

    if ( m_bIsDepthSortedOpaqueRendering )
    {
        for ( RenderGroupListCIter citer = vRenderGroupList.begin(); 
              citer != vRenderGroupList.end(); ++citer )
        {
            BatchRenderUtils::DrawSortedRenderGroup( pkRenderer, *(*citer), 
                                    *(m_spRenderStateBlockCache->GetStateBlock((*citer)->iNumber)) );
        }
    }
    else
    {
        for ( RenderGroupListCIter citer = vRenderGroupList.begin(); 
              citer != vRenderGroupList.end(); ++citer )
        {
            BatchRenderUtils::DrawUnSortedRenderGroup( pkRenderer, *(*citer), 
                                    *(m_spRenderStateBlockCache->GetStateBlock((*citer)->iNumber)) );
        }
    }
}

// Flush functions stuff
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::SortByDepth( SortItems& kItems )
{
    kItems.iNumItems = kItems.kItemList.GetSize();
    if ( 0 == kItems.iNumItems )
        return;

    if ( kItems.iNumItems > kItems.iMaxItems )
    {
        kItems.iMaxItems = kItems.iNumItems;

        NiFree( kItems.ppkItems );
        kItems.ppkItems = NiAlloc( NiGeometry*, kItems.iMaxItems );
        assert( kItems.ppkItems );

        NiFree( kItems.pfDepths );
        kItems.pfDepths = NiAlloc( float, kItems.iMaxItems );
        assert( kItems.pfDepths );
    }

    NiTListIterator pkPos = kItems.kItemList.GetHeadPos();

    assert( m_pkCamera );
    NiPoint3 kViewDir = m_pkCamera->GetWorldDirection();

    if ( m_bSortByClosestPoint )
    {
        for ( int i = 0; i < kItems.iNumItems; ++i )
        {
            kItems.ppkItems[i] = kItems.kItemList.GetNext( pkPos );
            kItems.pfDepths[i] = kItems.ppkItems[i]->GetWorldBound().GetCenter() * kViewDir - 
                                 kItems.ppkItems[i]->GetWorldBound().GetRadius();
        }
    }
    else
    {
        for ( int i = 0; i < kItems.iNumItems; ++i )
        {
            kItems.ppkItems[i] = kItems.kItemList.GetNext( pkPos );
            kItems.pfDepths[i] = kItems.ppkItems[i]->GetWorldBound().GetCenter() * kViewDir;
        }
    }

    SortObjectsByDepth( 0, kItems.iNumItems-1, kItems );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::AddToRenderGroup( NiRenderer* pkRenderer, 
                                         NiGeometry& kObject, NiAlphaProperty const* pkAlpha )
{
    assert( pkRenderer );
    assert( pkAlpha ); // Every property state should have a valid alpha property.

    if ( kObject.GetSortObject() )
    {
        if ( pkAlpha->GetAlphaBlending() && 
             !(m_bObserveAlphaNoSortHint && pkAlpha->GetNoSorter()) )
        {
            AddToAlphaRenderGroup( kObject, pkAlpha->GetAlphaGroup() );
        }
        else
        {
            if ( m_bObserveOpaqueNoSortHint || (AddToOpaqueRenderGroup(kObject) == false) )
                kObject.RenderImmediate( pkRenderer );
        }
    }
    else
    {
        kObject.RenderImmediate( pkRenderer );
    }
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::AddToAlphaRenderGroup( NiGeometry& kObject, int iGroupNum )
{
    RenderGroupList& vRenderGroupList = m_avRenderGroupList[RGL_ALPHA];

    bool bSmallGroupNumber = false;

    // @note - iGroupNum : (3) -> (1) -> (-8) 순으로 랜더링 되므로 큰수부터 작은수로 정렬함.
    for ( RenderGroupListIter iter = vRenderGroupList.begin(); iter != vRenderGroupList.end(); ++iter )
    {
        if ( (*iter)->iNumber == iGroupNum )
        {
            (*iter)->kSortItems.kItemList.AddTail( &kObject );
            return;
        }
        else if ( (*iter)->iNumber > iGroupNum )
        {
            bSmallGroupNumber = true;
        }
        else if ( (*iter)->iNumber < iGroupNum )
        {
            vRenderGroupList.insert( iter, CreateRenderGroup(kObject, iGroupNum) );
            return;
        }
    }

    if ( bSmallGroupNumber || vRenderGroupList.empty() )
    {
        vRenderGroupList.push_back( CreateRenderGroup(kObject, iGroupNum) );
    }
}

//-----------------------------------------------------------------------------------

bool GroupAccumulator::AddToOpaqueRenderGroup( NiGeometry& kObject )
{
    RenderStateTagExtraData* pkExtraData;
    if ( CreateRenderStateBlock(kObject, pkExtraData) == false )
        return false;

    AddToOpaqueRGroupAsStateNum( kObject, pkExtraData->GetRenderStateNumber(), pkExtraData->GetTextureHashKey() );
    return true;
}

//-----------------------------------------------------------------------------------

bool GroupAccumulator::CreateRenderStateBlock( NiGeometry& kObject, RenderStateTagExtraData*& pkExtraData )
{
    pkExtraData = NiDynamicCast(RenderStateTagExtraData, 
                                kObject.GetExtraData(RenderStateTagExtraData::ms_RTTI.GetName()));
    if ( NULL == pkExtraData )
    {
        pkExtraData = NiNew RenderStateTagExtraData();
        kObject.AddExtraData( RenderStateTagExtraData::ms_RTTI.GetName(), pkExtraData );

        //	A geometry that has a geometry morph controller cannot be rendered using batch rendering
        if ( BatchRenderUtils::IsBatchable(kObject) == false )
        {
            pkExtraData->SetRenderStateNumber( RenderStateTagExtraData::NOT_USED_RENDERSTATE );
            return	false;
        }
    }
    else
    {
        if ( pkExtraData->GetRenderStateNumber() == RenderStateTagExtraData::NOT_USED_RENDERSTATE )
            return false;

        if ( pkExtraData->GetCreationNumber() != CREATION_NUMBER )
            pkExtraData->ResetRenderStateNumber();
    }

    if ( pkExtraData->IsResetRenderStateNumber() )
    {
        // 새로운 객체나 변경된 객체임(Render State Block Cache에서 Render State Block 생성이나 갱신함).
        DWORD dwTextureHashKey;
        pkExtraData->SetRenderStateNumber( 
                        m_spRenderStateBlockCache->CreateStateBlockNumber(kObject, dwTextureHashKey) );
        pkExtraData->SetTextureHashKey( dwTextureHashKey );

        pkExtraData->SetCreationNumber( CREATION_NUMBER );
    }
    return true;
}

//-----------------------------------------------------------------------------------

RenderGroup* GroupAccumulator::CreateRenderGroup( NiGeometry& kObject, int iNum, DWORD const dwTextureHashKey )
{
    assert( RenderGroup::NUMBER_ZERO < iNum );

    RenderGroup* pkRenderGroup = new RenderGroupPool;
    assert( pkRenderGroup && pkRenderGroup->iNumber != iNum );

    pkRenderGroup->iNumber = iNum;
    pkRenderGroup->dwTextureHashKey = dwTextureHashKey;
    pkRenderGroup->kSortItems.kItemList.AddTail( &kObject );

    return pkRenderGroup;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::AddToOpaqueRGroupAsStateNum( NiGeometry& kObject, int iStateNum, 
                                                    DWORD const dwTextureHashKey )
{
    RenderGroupList& vRenderGroupList = m_avRenderGroupList[RGL_OPAQUE];

    if ( (int)m_vAssortHelpers.size() <= iStateNum )
    {
        int iResize = m_vAssortHelpers.size() * 2;
        ResizeAssortHelpers( std::max(iResize, iStateNum+1) );
    }

    if ( m_vAssortHelpers[iStateNum] >= 0 )
    {
        // Found!!
        assert( vRenderGroupList[m_vAssortHelpers[iStateNum]]->iNumber == iStateNum );
        vRenderGroupList[m_vAssortHelpers[iStateNum]]->kSortItems.kItemList.AddTail( &kObject );
    }
    else
    {
        // New!!
        m_vAssortHelpers[iStateNum] = (GroupNumber)vRenderGroupList.size();
        vRenderGroupList.push_back( CreateRenderGroup(kObject, iStateNum, dwTextureHashKey) );
    }
}

//-----------------------------------------------------------------------------------

void GroupAccumulator::ReserveRenderGroupList( unsigned int uiSize )
{
    assert( uiSize > 0 );
    uiSize = (std::max)( uiSize, (unsigned int)8 );

    for ( unsigned int ui = 0; ui < RGL_MAX_COUNT; ++ui )
    {
        m_avRenderGroupList[ui].reserve( uiSize );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Quick sorting(ascending sort) functions stuff

void GroupAccumulator::SortObjectsByDepth( int l, int r, SortItems& kItems )
{
    if ( r > l )
    {
        int i, j;

        i = l - 1;
        j = r + 1;
        float fPivot = ChoosePivot( l, r, kItems.pfDepths );

        for ( ;; )
        {
            do
            {
                --j;
            } while ( fPivot < kItems.pfDepths[j] );

            do
            {
                ++i;
            } while ( kItems.pfDepths[i] < fPivot );

            if ( i < j )
            {
                NiGeometry* pkObjTemp = kItems.ppkItems[i];
                kItems.ppkItems[i] = kItems.ppkItems[j];
                kItems.ppkItems[j] = pkObjTemp;
                float fTemp = kItems.pfDepths[i];
                kItems.pfDepths[i] = kItems.pfDepths[j];
                kItems.pfDepths[j] = fTemp;
            }
            else
            {
                break;
            }
        }

        if ( j == r )
        {
            SortObjectsByDepth( l, j-1, kItems );
        }
        else
        {
            SortObjectsByDepth( l, j, kItems );
            SortObjectsByDepth( j+1, r, kItems );
        }
    }
}

//-----------------------------------------------------------------------------------

float GroupAccumulator::ChoosePivot( int l, int r, float const* pfDepths ) const
{
    // Check the first, middle, and last element. Choose the one which falls
    // between the other two. This has a good chance of discouraging 
    // quadratic behavior from qsort.
    // In the case when all three are equal, this code chooses the middle
    // element, which will prevent quadratic behavior for a list with 
    // all elements equal.

    int m = (l + r) >> 1;

    if ( pfDepths[l] < pfDepths[m] )
    {
        if ( pfDepths[m] < pfDepths[r] )
        {
            return pfDepths[m];
        }
        else
        {
            if ( pfDepths[l] < pfDepths[r] )
                return pfDepths[r];
            else
                return pfDepths[l];
        }
    }
    else
    {
        if ( pfDepths[l] < pfDepths[r] )
        {
            return pfDepths[l];
        }
        else
        {
            if ( pfDepths[m] < pfDepths[r] )
                return pfDepths[r];
            else
                return pfDepths[m];
        }
    }
}

// Quick sorting(ascending sort) functions stuff
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void GroupAccumulator::DumpRenderGroupList()
{
    unsigned int uiGroup, uiIndex;

    for ( unsigned int ui = 0; ui < RGL_MAX_COUNT; ++ui )
    {
        Tools::OutputDebugFile( "------------------------------------ GroupAccumulator[%d] ------------------------------------\n", ui );

        uiGroup = 0;
        for ( RenderGroupListCIter citer = m_avRenderGroupList[ui].begin(); 
              citer != m_avRenderGroupList[ui].end(); ++citer, ++uiGroup )
        {
            Tools::OutputDebugFile( "## Group[%d] - AlphaGroup/OpaqueState number: %d, TextureHashKey: %d ##\n", 
                                    uiGroup, (*citer)->iNumber, (*citer)->dwTextureHashKey );

            // Sorted!
            uiIndex = 0;
            while ( uiIndex < (unsigned int)(*citer)->kSortItems.iNumItems )
            {
                Tools::OutputDebugFile( "Sorted[%d] - Name:%s, Depth:%f\n", uiIndex, 
                                        (*citer)->kSortItems.ppkItems[uiIndex]->GetName(), 
                                        (*citer)->kSortItems.pfDepths[uiIndex] );
                ++uiIndex;
            }

            // Unsorted!
            uiIndex = 0;
            NiSortedObjectList const& kItemList = (*citer)->kSortItems.kItemList;
            NiTListIterator kPos = kItemList.GetHeadPos();
            while ( kPos )
            {
                Tools::OutputDebugFile( "Unsorted[%d] - Name:%s\n", uiIndex, kItemList.GetNext(kPos)->GetName() );
                ++uiIndex;
            }
        }
    }

    m_bIsDumpRenderGroupList = false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SetRenderStateNumber( NiGeometry& kObject, int iRenderStateNumber )
{
    RenderStateTagExtraData* pkExtraData = 
        NiDynamicCast(RenderStateTagExtraData, kObject.GetExtraData(RenderStateTagExtraData::ms_RTTI.GetName()));
    if ( NULL == pkExtraData )
    {
        pkExtraData = NiNew RenderStateTagExtraData();
        kObject.AddExtraData( RenderStateTagExtraData::ms_RTTI.GetName(), pkExtraData );
    }
    else if ( pkExtraData->GetRenderStateNumber() == RenderStateTagExtraData::NOT_USED_RENDERSTATE )
    {
        return;
    }
    pkExtraData->SetRenderStateNumber( iRenderStateNumber );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
