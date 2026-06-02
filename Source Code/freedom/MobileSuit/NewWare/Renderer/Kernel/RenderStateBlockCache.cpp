
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderStateBlockCache.cpp
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
#include "RenderStateBlockCache.h"

#include "../../Scene/ApplyProperty.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

RenderStateBlockCache::RenderStateBlockCache()
{
    ReserveStateBlockList();
    ReserveStateBlockCategoryList();
}

//-----------------------------------------------------------------------------------

RenderStateBlockCache::~RenderStateBlockCache()
{
    RemoveAllStateBlock();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

int RenderStateBlockCache::CreateStateBlockNumber( NiGeometry& kObject, DWORD& dwTextureHashKey )
{
    int iStateNum = GetStateBlockNumber( kObject );
    if ( 0 <= iStateNum )
        return iStateNum;

    // @note - std::vector [] index를 State-Block number로 씀.
    // @warning - State-Block number구하는 것하고 CreateStateBlock()생성하고 순서 바뀌면 안됨!
    iStateNum = (int)m_vRenderStateBlockList.size();
    RenderStateBlock* pkRSBlock = CreateStateBlock( kObject, dwTextureHashKey );
    InsertStateBlockCategory( pkRSBlock->kStateBit, iStateNum );
    m_vRenderStateBlockList.push_back( pkRSBlock );
    return iStateNum;
}

//-----------------------------------------------------------------------------------

void RenderStateBlockCache::RemoveAllStateBlock()
{
    for ( RenderStateBlockListIter iter = m_vRenderStateBlockList.begin(); 
          iter != m_vRenderStateBlockList.end(); ++iter )
    {
        delete (*iter);
    }
    m_vRenderStateBlockList.clear();

    m_vRSBlockCategoryList.clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

int RenderStateBlockCache::GetStateBlockNumber( NiGeometry& kObject )
{
    NiPropertyState* pkProperty = kObject.GetPropertyState();
    NiDynamicEffectState* pkEffect = kObject.GetEffectState();
    assert( pkProperty && pkEffect );

    NiTexturingProperty* pkTexturing = pkProperty->GetTexturing();
    if ( NULL == pkTexturing )
    {
        assert( 0 );
        return -1;
    }

    unsigned int uiCategory;
    if ( FindStateBlockCategory(kObject, uiCategory) == false )
        return -1;

    unsigned int uiIndex;
    RenderStateBlock* pkBlock;
    CheckTex eCheck;

    std::vector<unsigned int>& vRSBlockIndexs = m_vRSBlockCategoryList[uiCategory].vRSBlockIndexs;

    for ( std::vector<unsigned int>::const_iterator citer = vRSBlockIndexs.begin(); 
          citer != vRSBlockIndexs.end(); ++citer )
    {
        uiIndex = (*citer);
        pkBlock = m_vRenderStateBlockList[ uiIndex ];
        assert( pkBlock );

        if ( IsEqualEffectState(pkBlock->spEffectState, pkEffect) == false )
            continue;
        if ( IsEqualAlphaProperty(pkBlock->spPropertyState->GetAlpha(), pkProperty->GetAlpha()) == false )
            continue;
        if ( IsEqualStencilProperty(pkBlock->spPropertyState->GetStencil(), pkProperty->GetStencil()) == false )
            continue;

        eCheck = CheckTexturingProperty( pkBlock->spPropertyState->GetTexturing(), pkTexturing );
        if ( CT_FOUND == eCheck )
            return uiIndex;
        else if ( CT_FALSE == eCheck )
            continue;

        if ( IsEqualBaseMap(pkBlock->spPropertyState->GetTexturing(), pkTexturing) == false )
            continue;
        if ( IsEqualGlowMap(pkBlock->spPropertyState->GetTexturing()->GetGlowMap(), 
                            pkTexturing->GetGlowMap()) == false )
            continue;
        if ( IsEqualDetailMap(pkBlock->spPropertyState->GetTexturing()->GetDetailMap(), 
                              pkTexturing->GetDetailMap()) == false )
            continue;

        return uiIndex;
    }

    return -1;
}

//-----------------------------------------------------------------------------------

RenderStateBlockCache::
RenderStateBlock* RenderStateBlockCache::CreateStateBlock( NiGeometry& kObject, DWORD& dwTextureHashKey ) const
{
    RenderStateBlock* pkStateBlock = new RenderStateBlock;
    assert( pkStateBlock );

    NiPropertyState* pkProperty = kObject.GetPropertyState();
    if ( pkProperty )
    {
        pkStateBlock->spPropertyState = Scene::ApplyProperty::CopyPropertyState( pkProperty );
        assert( pkStateBlock->spPropertyState );

        // Get the texture hash key!!
        NiTexturingProperty* pkTexProp = pkProperty->GetTexturing();
        if ( NULL != pkTexProp )
        {
            NiTexturingProperty::Map* pkBaseMap = pkTexProp->GetBaseMap();
            if ( pkBaseMap )
                dwTextureHashKey = (DWORD)(char const*)((NiSourceTexture*)(pkBaseMap->GetTexture()))->GetFilename();
            else
                dwTextureHashKey = 0;
        }
    }

    NiDynamicEffectState* pkEffect = kObject.GetEffectState();
    if ( pkEffect )
    {
        pkStateBlock->spEffectState = pkEffect->Copy();
    }

    GetVertexFormatStateBit( pkStateBlock->kStateBit, kObject );

    return pkStateBlock;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool RenderStateBlockCache::FindStateBlockCategory( NiGeometry const& kObject, unsigned int& uiCategory ) const
{
    Tools::BitFlag<RenderStateBit::StateBitType> kStateBit;
    GetVertexFormatStateBit( kStateBit, kObject );

    for ( uiCategory = 0; uiCategory < m_vRSBlockCategoryList.size(); ++uiCategory )
    {
        if ( m_vRSBlockCategoryList[uiCategory].kCategoryBit == kStateBit )
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

bool RenderStateBlockCache::FindStateBlockCategory( 
                Tools::BitFlag<RenderStateBit::StateBitType> const& kStateBit, unsigned int& uiCategory ) const
{
    for ( uiCategory = 0; uiCategory < m_vRSBlockCategoryList.size(); ++uiCategory )
    {
        if ( m_vRSBlockCategoryList[uiCategory].kCategoryBit == kStateBit )
            return true;
    }
    return false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
