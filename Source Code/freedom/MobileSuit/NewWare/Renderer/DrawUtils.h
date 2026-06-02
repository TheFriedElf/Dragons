
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DrawUtils.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_DRAWUTILS_H__
#define _RENDERER_DRAWUTILS_H__


namespace NewWare
{

namespace Renderer
{


namespace DrawUtils
{

    // Construct the potentially visible set(PVS).
    inline unsigned int GetPVSFromCullScene( NiCamera const* pkCamera, NiCullingProcess& kCuller, 
                                             NiAVObject* pkScene, NiVisibleArray& kVisibleSet, bool bResetArray )
    {
        assert( pkCamera &&  pkScene );
        unsigned int uiPreviousElementsInArray = 0;
        if ( bResetArray )
            kVisibleSet.RemoveAll();
        else
            uiPreviousElementsInArray = kVisibleSet.GetCount();
        kCuller.Process( pkCamera, pkScene, &kVisibleSet );
        return kVisibleSet.GetCount() - uiPreviousElementsInArray;
    }


    inline bool PartialRenderAppend( NiCamera const* pkCamera, NiCullingProcess& kCuller, 
                                     NiAVObject* pkAVObject, NiVisibleArray& kVisibleSet, bool bResetArray )
    {
        assert( pkCamera &&  pkAVObject );
        if ( GetPVSFromCullScene(pkCamera, kCuller, pkAVObject, kVisibleSet, bResetArray) > 0 )
        {
            NiDrawVisibleArrayAppend( kVisibleSet );
            return true;
        }
        return false;
    }
    inline bool PartialRenderClick( NiCamera* pkCamera, NiCullingProcess& kCuller, 
                                    NiAVObject* pkAVObject, NiVisibleArray& kVisibleSet, bool bResetArray )
    {
        assert( pkCamera &&  pkAVObject );
        if ( GetPVSFromCullScene(pkCamera, kCuller, pkAVObject, kVisibleSet, bResetArray) > 0 )
        {
            NiDrawVisibleArray( pkCamera, kVisibleSet );
            return true;
        }
        return false;
    }
    inline bool PartialRenderImmediate( NiCamera* pkCamera, NiCullingProcess& kCuller, 
                                        NiAVObject* pkAVObject, NiVisibleArray& kVisibleSet, bool bResetArray )
    {
        assert( pkCamera &&  pkAVObject );
        if ( GetPVSFromCullScene(pkCamera, kCuller, pkAVObject, kVisibleSet, bResetArray) > 0 )
        {
            NiRenderer* pkRenderer = NiRenderer::GetRenderer();
            for ( unsigned int ui = 0; ui < kVisibleSet.GetCount(); ++ui )
                kVisibleSet.GetAt(ui).RenderImmediate( pkRenderer );
            return true;
        }
        return false;
    }


    // Not culling process!!
    void DrawImmediate( NiRenderer* pkRenderer, NiAVObject const* pkAVObject );


    unsigned int GetLightCountByAffectedObject( NiAVObject const* pkAVObject );

    unsigned int GetTriangleCount( NiVisibleArray const& kVisibleList );
    unsigned int GetTriangleCount( NiAVObject const* pkAVObject );


    inline bool EnableGlobalZBuffer( BOOL bEnable )
    {
        NiDX9Renderer* pkRenderer = static_cast<NiDX9Renderer*>(NiRenderer::GetRenderer());
        assert( NiIsExactKindOf(NiDX9Renderer, pkRenderer) && pkRenderer->GetD3DDevice() );
        return SUCCEEDED(pkRenderer->GetD3DDevice()->SetRenderState(D3DRS_ZENABLE, bEnable));
    }
    bool EnableGlobalZBufferSafe( BOOL const bEnable );


    void SetBaseTextureFiltering( NiAVObject* pkAVObject, NiTexturingProperty::FilterMode eMode );

} //namespace DrawUtils


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_DRAWUTILS_H__
