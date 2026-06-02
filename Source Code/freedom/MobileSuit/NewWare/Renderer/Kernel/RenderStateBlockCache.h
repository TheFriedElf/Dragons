
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderStateBlockCache.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERSTATEBLOCKCACHE_H__
#define _RENDERER_KERNEL_RENDERSTATEBLOCKCACHE_H__

#include "RenderStateBit.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


class RenderStateBlockCache
{
public:
    struct RenderStateBlock
    {
        RenderStateBlock() { /* Nothing */ };

        void Clear()
        {
            kStateBit.Reset();

            spPropertyState = 0;
            spEffectState = 0;
        }

        Tools::BitFlag<RenderStateBit::StateBitType> kStateBit;
        NiPropertyStatePtr spPropertyState;
        NiDynamicEffectStatePtr spEffectState;
    };
private:
    typedef std::vector<RenderStateBlock*>       RenderStateBlockList;
    typedef RenderStateBlockList::iterator       RenderStateBlockListIter;
    typedef RenderStateBlockList::const_iterator RenderStateBlockListCIter;

    enum { RENDER_STATEBLOCK_LIST_DEF_SIZE = 1024 };


private:
    // FAST search for RenderStateBlockList. (Category dictionary)
    struct RSBlockCategory
    {
        explicit RSBlockCategory( Tools::BitFlag<RenderStateBit::StateBitType> const& kStateBit )
            : kCategoryBit(kStateBit) { vRSBlockIndexs.reserve( 512 ); };

        Tools::BitFlag<RenderStateBit::StateBitType> kCategoryBit;
        std::vector<unsigned int> vRSBlockIndexs;
    };
    typedef std::vector< RSBlockCategory > RSBlockCategoryList;

    enum { RSBLOCKCATEGORY_LIST_DEF_SIZE = 8 };


public:
    RenderStateBlockCache();
    ~RenderStateBlockCache();


    int CreateStateBlockNumber( NiGeometry& kObject, DWORD& dwTextureHashKey );
    RenderStateBlock* GetStateBlock( int const iStateNum ) const { 
        assert( 0 <= iStateNum && (int)m_vRenderStateBlockList.size() > iStateNum );
        return m_vRenderStateBlockList[iStateNum];
    }
    void RemoveAllStateBlock();


private:
    int GetStateBlockNumber( NiGeometry& kObject );
    RenderStateBlock* CreateStateBlock( NiGeometry& kObject, DWORD& dwTextureHashKey ) const;

    bool FindStateBlockCategory( NiGeometry const& kObject, unsigned int& uiCategory ) const;
    bool FindStateBlockCategory( Tools::BitFlag<RenderStateBit::StateBitType> const& kStateBit, unsigned int& uiCategory ) const;
    void InsertStateBlockCategory( Tools::BitFlag<RenderStateBit::StateBitType> const& kStateBit, int const iStateNum )
    {
        unsigned int uiCategory;
        if ( FindStateBlockCategory(kStateBit, uiCategory) == false )
            m_vRSBlockCategoryList.push_back( RSBlockCategory(kStateBit) );
        m_vRSBlockCategoryList[ uiCategory ].vRSBlockIndexs.push_back( iStateNum );
    }

    void ReserveStateBlockList() { m_vRenderStateBlockList.reserve( RENDER_STATEBLOCK_LIST_DEF_SIZE ); };
    void ReserveStateBlockCategoryList() { m_vRSBlockCategoryList.reserve( RSBLOCKCATEGORY_LIST_DEF_SIZE ); };

    void GetVertexFormatStateBit( Tools::BitFlag<RenderStateBit::StateBitType>& kStateBit, 
                                  NiGeometry const& kObject ) const
    {
        RenderStateBit::SetSkinning( kStateBit, (kObject.GetSkinInstance() != NULL) );
        RenderStateBit::SetVertexColor( kStateBit, (kObject.GetColors() != NULL) );
        RenderStateBit::SetNormals( kStateBit, (kObject.GetNormals() != NULL) );
        RenderStateBit::SetTextureSets( kStateBit, kObject.GetTextureSets() );
    }


private:
    bool IsEqualVertexFormat( Tools::BitFlag<RenderStateBit::StateBitType> const& kStateBit, 
                              NiGeometry const& kObject ) const
    {
        if ( RenderStateBit::GetSkinning(kStateBit) != (kObject.GetSkinInstance() != NULL) )
            return false;
        if ( RenderStateBit::GetVertexColor(kStateBit) != (kObject.GetColors() != NULL) )
            return false;
        if ( RenderStateBit::GetNormals(kStateBit) != (kObject.GetNormals() != NULL) )
            return false;
        if ( RenderStateBit::GetTextureSets(kStateBit) != (BYTE)kObject.GetTextureSets() )
            return false;
        return true;
    }
    bool IsEqualEffectState( NiDynamicEffectState const* pkEffect1, NiDynamicEffectState const* pkEffect2 ) const
    {
        if ( (pkEffect1 && !pkEffect2) || (!pkEffect1 && pkEffect2) )
            return false;
        if ( pkEffect1->Equal(pkEffect2) == false )
            return false;
        return true;
    }
    bool IsEqualAlphaProperty( NiAlphaProperty* pkAlpha1, NiAlphaProperty* pkAlpha2 )
    {
        if ( (pkAlpha1 && !pkAlpha2) || (!pkAlpha1 && pkAlpha2) )
            return false;
        if ( pkAlpha1 && pkAlpha1->IsEqual(pkAlpha2) == false ) // NiStream macro NOT const IsEqual!!!!!!
            return false;
        return true;
    }
    bool IsEqualStencilProperty( NiStencilProperty const* pkStencil1, NiStencilProperty const* pkStencil2 ) const
    {
        if ( NULL == pkStencil1 || NULL == pkStencil2 )
            return false;
        if ( pkStencil1->GetDrawMode() != pkStencil2->GetDrawMode() )
            return false;
        return true;
    }
    enum CheckTex { CT_FALSE, CT_TRUE, CT_FOUND };
    CheckTex CheckTexturingProperty( NiTexturingProperty const* pkTexturing1, NiTexturingProperty const* pkTexturing2 ) const
    {
        if ( pkTexturing1->IsEqualFast(*pkTexturing2) )
            return CT_FOUND; // return value (found!!!!)
        if ( pkTexturing1->GetMaps().GetEffectiveSize() != pkTexturing2->GetMaps().GetEffectiveSize())
            return CT_FALSE; // false
        return CT_TRUE; // true
    }
    bool IsEqualBaseMap( NiTexturingProperty const* pkTexturing1, NiTexturingProperty const* pkTexturing2 ) const
    {
        if ( ( pkTexturing1->GetBaseMap() && !pkTexturing2->GetBaseMap()) || 
             (!pkTexturing1->GetBaseMap() &&  pkTexturing2->GetBaseMap()) )
            return false;
        if ( pkTexturing1->GetBaseMap() )
        {
            if ( pkTexturing1->GetBaseClampMode() != pkTexturing2->GetBaseClampMode() )
                return false;

            NiSourceTexture const* pkSrcTex1 = (NiSourceTexture*)pkTexturing1->GetBaseMap()->GetTexture();
            NiSourceTexture const* pkSrcTex2 = (NiSourceTexture*)pkTexturing2->GetBaseMap()->GetTexture();
            if ( pkSrcTex1->GetFilename() != pkSrcTex2->GetFilename() )
                return false;
            if ( pkSrcTex1->GetFilename().GetLength()==0 || pkSrcTex2->GetFilename().GetLength()==0 )
                return false;
        }
        return true;
    }
    bool IsEqualGlowMap( NiTexturingProperty::Map const* pkMap1, NiTexturingProperty::Map const* pkMap2 ) const
    {
        if ( (pkMap1 && !pkMap2) || (!pkMap1 && pkMap2) )
            return false;
        if ( pkMap1 )
        {
            if ( pkMap1->GetClampMode() != pkMap2->GetClampMode() )
                return false;
            if ( pkMap1->GetEnable() != pkMap2->GetEnable() )
                return false;
            if ( ((NiSourceTexture*)pkMap1->GetTexture())->GetFilename() != 
                 ((NiSourceTexture*)pkMap2->GetTexture())->GetFilename() )
                return false;
        }
        return true;
    }
    bool IsEqualDetailMap( NiTexturingProperty::Map const* pkMap1, NiTexturingProperty::Map const* pkMap2 ) const
    {
        if ( (pkMap1 && !pkMap2) || (!pkMap1 && pkMap2) )
            return false;
        if ( pkMap1 )
        {
            if ( pkMap1->GetClampMode() != pkMap2->GetClampMode() )
                return false;
            if ( ((NiSourceTexture*)pkMap1->GetTexture())->GetFilename() != 
                 ((NiSourceTexture*)pkMap2->GetTexture())->GetFilename() )
                return false;
        }
        return true;
    }


private:
    RenderStateBlockList m_vRenderStateBlockList;
    RSBlockCategoryList m_vRSBlockCategoryList;
};


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERSTATEBLOCKCACHE_H__
