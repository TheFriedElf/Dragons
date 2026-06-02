
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : GroupAccumulator.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_GROUPACCUMULATOR_H__
#define _RENDERER_KERNEL_GROUPACCUMULATOR_H__

#include <NiAccumulator.h>
#include <NiCamera.h>
#include <NiGeometry.h>

#include "RenderGroupPool.h"
#include "RenderStateBlockCache.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{

struct SortItems;
struct RenderGroup;
class RenderStateTagExtraData;


/** Accumulation / Sorting Design Rule
    1) 랜더타겟 별로 불투명과 투명 객체를 분류
       - 랜더타겟 별로 GroupAccumulator를 가짐(필요없어서 미 구현).
    2) 분류된 객체별 누적/정렬 처리
       a) 불투명 (Opaque)
          1) 텍스쳐/머터리얼/상태 별로 그룹화 (배칭처리)
          2) 그룹화 된 내부의 메쉬를 카메라 기준 거리별로 정렬(앞->뒤)
          3) 그룹화 된 것을 그룹 별로 텍스쳐 기준으로 정렬 (배칭처리)
       b) 투명 (Alpha)
          1) 알파 그룹화(그리기 우선 순위되로 삽입 정렬)
          2) 알파 그룹내에서 카메라 기준 거리별로 정렬(뒤->앞)
*/
class GroupAccumulator : public NiAccumulator
{
    NiDeclareRTTI;

private:
    enum RenderGroupLayer
    {
        RGL_OPAQUE = 0, RGL_ALPHA, RGL_MAX_COUNT
    };
    typedef std::vector<RenderGroup*>       RenderGroupList;
    typedef RenderGroupList::iterator       RenderGroupListIter;
    typedef RenderGroupList::const_iterator RenderGroupListCIter;

    enum { ASSORTHELPERS_DEF_SIZE = 1024 };
    typedef int GroupNumber;
    typedef std::vector<GroupNumber>      AssortHelpers;
    typedef AssortHelpers::iterator       AssortHelpersIter;
    typedef AssortHelpers::const_iterator AssortHelpersCIter;


public:
    explicit GroupAccumulator( std::auto_ptr<RenderStateBlockCache> spRSBlockCache, 
                               unsigned int uiRenderGroupListSize = RENDERGROUP_POOL_COUNT );
	virtual ~GroupAccumulator();


    void Reset()
    {
        ClearAccumulateBuffer();
        ClearRenderStateBlockCache();
        ++CREATION_NUMBER;
    }


    void SetRenderStateBlockCache( std::auto_ptr<RenderStateBlockCache> spRSBlockCache )
    {
        assert( spRSBlockCache.get() != NULL );
        m_spRenderStateBlockCache = spRSBlockCache;
    }
    void BuildRenderStateBlockCache( NiAVObject* pkAVObject );


    void SetSortByClosestPoint( bool bClosest ) { m_bSortByClosestPoint = bClosest; };
    bool GetSortByClosestPoint() const { return m_bSortByClosestPoint; };


    // Observe the "no sort" NiAlphaProperty hint flags?
    void SetObserveAlphaNoSortHint( bool bObserve ) { m_bObserveAlphaNoSortHint = bObserve; };
    bool GetObserveAlphaNoSortHint() const { return m_bObserveAlphaNoSortHint; };

    void SetObserveOpaqueNoSortHint( bool bObserve ) { m_bObserveOpaqueNoSortHint = bObserve; };
    bool GetObserveOpaqueNoSortHint() const { return m_bObserveOpaqueNoSortHint; };


    void EnableDepthSortedOpaqueRendering( bool bEnable = true ) { m_bIsDepthSortedOpaqueRendering = bEnable; };
    void EnableDumpRenderGroupList( bool bEnable = true ) { m_bIsDumpRenderGroupList = bEnable; };


    // *** begin Emergent internal use only ***

    virtual void StartAccumulating( NiCamera const* pkCamera );
	virtual void RegisterObjectArray( NiVisibleArray& kArray );
    virtual void FinishAccumulating();

    // *** end Emergent internal use only ***


protected:
    void ClearAccumulateBuffer()
    {
        PROFILE_FUNC();
        ClearRenderGroupItems();
        ClearAssortHelpers();
    }
    void ClearRenderStateBlockCache()
    {
        PROFILE_FUNC();
        assert( m_spRenderStateBlockCache.get() != NULL );
        m_spRenderStateBlockCache->RemoveAllStateBlock();
    }

    virtual void SortRenderGroupItems();
    virtual void ClearRenderGroupItems();

    virtual void FlushRenderGroupItems()
    {
        NiRenderer* pkRenderer = NiRenderer::GetRenderer();
        assert( pkRenderer );
        FlushOpaqueRenderGroupItems( pkRenderer );
        FlushAlphaRenderGroupItems( pkRenderer );
    }
    virtual void FlushAlphaRenderGroupItems( NiRenderer* pkRenderer );
    virtual void FlushOpaqueRenderGroupItems( NiRenderer* pkRenderer );

	virtual void SortByDepth( SortItems& kItems );
    virtual void SortOpaqueGroupByTexture()
    {
        PROFILE_FUNC();
        std::sort( m_avRenderGroupList[RGL_OPAQUE].begin(), m_avRenderGroupList[RGL_OPAQUE].end(), RenderGroup::QSortMethod );
    }

    virtual void AddToRenderGroup( NiRenderer* pkRenderer, 
                                   NiGeometry& kObject, NiAlphaProperty const* pkAlpha );
    virtual void AddToAlphaRenderGroup( NiGeometry& kObject, int iGroupNum );
    virtual bool AddToOpaqueRenderGroup( NiGeometry& kObject );
    virtual bool CreateRenderStateBlock( NiGeometry& kObject, RenderStateTagExtraData*& pkExtraData );
    virtual RenderGroup* CreateRenderGroup( NiGeometry& kObject, int iNum, DWORD const dwTextureHashKey = 0 );


private:
    void AddToOpaqueRGroupAsStateNum( NiGeometry& kObject, int iStateNum, DWORD const dwTextureHashKey );

    void ReserveRenderGroupList( unsigned int uiSize );

    void ResizeAssortHelpers( unsigned int uiSize = ASSORTHELPERS_DEF_SIZE )
        { m_vAssortHelpers.resize( (uiSize>0)? uiSize: 1, -1 ); };
    void ClearAssortHelpers()
    {
        PROFILE_FUNC();
        m_vAssortHelpers.clear();
        ResizeAssortHelpers( (unsigned int)m_vAssortHelpers.capacity() );
    }

    void SortObjectsByDepth( int l, int r, SortItems& kItems );
    float ChoosePivot( int l, int r, float const* pfDepths ) const;

    void OnDumpRenderGroupList()
    {
        if ( m_bIsDumpRenderGroupList )
            DumpRenderGroupList();
    }
    void DumpRenderGroupList();


private:
    std::auto_ptr<RenderStateBlockCache> m_spRenderStateBlockCache;

    bool m_bSortByClosestPoint;

    bool m_bObserveAlphaNoSortHint;
    bool m_bObserveOpaqueNoSortHint;

    RenderGroupList m_avRenderGroupList[RGL_MAX_COUNT];
    AssortHelpers m_vAssortHelpers;

    bool m_bIsDepthSortedOpaqueRendering;
    bool m_bIsDumpRenderGroupList;

    static DWORD CREATION_NUMBER;
};

NiSmartPointer(GroupAccumulator);


void SetRenderStateNumber( NiGeometry& kObject, int iRenderStateNumber );


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_GROUPACCUMULATOR_H__
