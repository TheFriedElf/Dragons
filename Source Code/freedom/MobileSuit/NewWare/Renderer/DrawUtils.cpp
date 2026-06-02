
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DrawUtils.cpp
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
#include "DrawUtils.h"

#include "../Scene/NodeTraversal.h"


namespace NewWare
{

namespace Renderer
{


namespace DrawUtils
{

/////////////////////////////////////////////////////////////////////////////////////////////
//

void DrawImmediate( NiRenderer* pkRenderer, NiAVObject const* pkAVObject )
{
    assert( pkRenderer && pkAVObject );

    struct _Functor
    {
        explicit _Functor( NiRenderer* pkRender ) : pkRenderer(pkRender) { assert( pkRenderer ); };

        bool operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
            {
                if ( pkGeo->GetAppCulled() == false )
                    pkGeo->RenderImmediate( pkRenderer );
                return false;
            }
            return true;
        }

        NiRenderer* pkRenderer;
    } kFunctor( pkRenderer );

    Scene::NodeTraversal::DepthFirst::PreRecurse( pkAVObject, kFunctor );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

unsigned int GetLightCountByAffectedObject( NiAVObject const* pkAVObject )
{
    struct _Functor
    {
        _Functor() : uiCounts(0) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
            if ( pkNode )
            {
                NiDynamicEffectList const& kList = pkNode->GetEffectList();
                NiTListIterator kPos = kList.GetHeadPos();
                while ( kPos )
                {
                    NiDynamicEffect* pkEffect = kList.GetNext( kPos );
                    assert( pkEffect );

                    NiDynamicEffect::EffectType const eType = pkEffect->GetEffectType();
                    if ( NiDynamicEffect::DIR_LIGHT == eType   || /*NiDynamicEffect::AMBIENT_LIGHT == eType || */
                         NiDynamicEffect::POINT_LIGHT == eType || NiDynamicEffect::SPOT_LIGHT == eType )
                    {
                        ++uiCounts;
                    }
                }
            }
        }

        unsigned int uiCounts;
    } kFunctor;

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
    return kFunctor.uiCounts;
}

//-----------------------------------------------------------------------------------

unsigned int GetTriangleCount( NiVisibleArray const& kVisibleList )
{
    unsigned int uiCounts = 0;

    for ( unsigned int ui = 0; ui < kVisibleList.GetCount(); ++ui )
    {
        NiTriBasedGeom const* pkTri = NiDynamicCast(NiTriBasedGeom, &kVisibleList.GetAt(ui));
        if ( pkTri )
            uiCounts += pkTri->GetTriangleCount();
    }

    return uiCounts;
}

//-----------------------------------------------------------------------------------

unsigned int GetTriangleCount( NiAVObject const* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        _Functor() : uiCounts(0) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiTriBasedGeom const* pkTri = NiDynamicCast(NiTriBasedGeom, pkObject);
            if ( pkTri )
                uiCounts += pkTri->GetTriangleCount();
        }

        unsigned int uiCounts;
    } kFunctor;

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
    return kFunctor.uiCounts;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool EnableGlobalZBufferSafe( BOOL const bEnable )
{
    NiDX9Renderer* pkRenderer = static_cast<NiDX9Renderer*>(NiRenderer::GetRenderer());
    assert( NiIsExactKindOf(NiDX9Renderer, pkRenderer) );

    LPDIRECT3DDEVICE9 pkD3DDev = pkRenderer->GetD3DDevice();
    assert( pkD3DDev );
    DWORD bZEnable;
    if ( FAILED(pkD3DDev->GetRenderState(D3DRS_ZENABLE, &bZEnable)) )
        return false;

    if ( bEnable != bZEnable )
        return SUCCEEDED(pkD3DDev->SetRenderState(D3DRS_ZENABLE, bEnable));
    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SetBaseTextureFiltering( NiAVObject* pkAVObject, NiTexturingProperty::FilterMode eMode )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( NiTexturingProperty::FilterMode _eMode ) : eMode(_eMode) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
            {
                NiTexturingProperty* pkTexProp = (NiTexturingProperty*)pkGeo->GetProperty( NiProperty::TEXTURING );
                if ( pkTexProp )
                    pkTexProp->SetBaseFilterMode( eMode );
            }
        }

        NiTexturingProperty::FilterMode const eMode;
    } kFunctor( eMode );

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////

} //namespace DrawUtils


} //namespace Renderer

} //namespace NewWare
