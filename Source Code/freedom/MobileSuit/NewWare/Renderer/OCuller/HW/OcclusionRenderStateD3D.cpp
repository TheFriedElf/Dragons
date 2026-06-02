
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionRenderStateD3D.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#include "stdafx.h"
#include "OcclusionRenderStateD3D.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

OcclusionRenderStateD3D::OcclusionRenderStateD3D()
    : m_eStateCache(NOT_SET)
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool OcclusionRenderStateD3D::SetRenderState( State const eState )
{
    if ( eState == m_eStateCache )
        return true;

    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );
    if ( NULL == pkRenderer )
        return false;

    NiDX9RenderState* pkRenderState = pkRenderer->GetRenderState();
    assert( pkRenderState );
    if ( NULL == pkRenderState )
        return false;

    switch ( eState )
    {
    case COLOR_DRAW_STATE: ColorDrawState( pkRenderState ); break;
    case DEPTH_DRAW_STATE: DepthDrawState( pkRenderState ); break;
    case DEPTH_TEST_STATE: DepthTestState( pkRenderState ); break;
    }

    m_eStateCache = eState;
    return true;
}

//-----------------------------------------------------------------------------------

void OcclusionRenderStateD3D::ColorDrawState( NiDX9RenderState* pkRenderState )
{
    assert( pkRenderState );

    // Restore cullmode and alpha and color write states.
    pkRenderState->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    pkRenderState->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pkRenderState->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

    pkRenderState->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    pkRenderState->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );

    pkRenderState->SetRenderState( D3DRS_COLORWRITEENABLE, 
                                   D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|
                                   D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA );

    // Set the fill mode to solid fill.
    pkRenderState->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Set Pixel and Vertex shaders to null.
    pkRenderState->SetPixelShader( NULL );
    pkRenderState->SetVertexShader( NULL );
}

//-----------------------------------------------------------------------------------

void OcclusionRenderStateD3D::DepthDrawState( NiDX9RenderState* pkRenderState )
{
    assert( pkRenderState );

    // Disable color write.
    pkRenderState->SetRenderState( D3DRS_COLORWRITEENABLE, FALSE );

    // Enable z-buffering and z-write.
    pkRenderState->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    pkRenderState->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE );

    // Enable backface culling.
    pkRenderState->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

    // Restore alpha state.
    pkRenderState->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    pkRenderState->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

    // Restore the backface culling process.
    pkRenderState->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Set the fill mode to solid fill.
    pkRenderState->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Set Pixel and Vertex shaders to null.
    pkRenderState->SetPixelShader( NULL );
    pkRenderState->SetVertexShader( NULL );
}

//-----------------------------------------------------------------------------------

// Depth test state disables color and depth writes.
void OcclusionRenderStateD3D::DepthTestState( NiDX9RenderState* pkRenderState )
{
    assert( pkRenderState );

    // Disable color write.
    pkRenderState->SetRenderState( D3DRS_COLORWRITEENABLE, FALSE );

    // Enable z-buffering and disable z-write.
    pkRenderState->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    pkRenderState->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );

    // Turn backface culling off.
    pkRenderState->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Alpha off.
    pkRenderState->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    pkRenderState->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

    // Set the fill mode to solid fill.
    pkRenderState->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Set Pixel and Vertex shaders to null.
    pkRenderState->SetPixelShader( NULL );
    pkRenderState->SetVertexShader( NULL );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
