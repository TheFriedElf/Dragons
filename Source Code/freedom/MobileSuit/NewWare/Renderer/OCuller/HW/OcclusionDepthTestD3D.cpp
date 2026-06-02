
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionDepthTestD3D.cpp
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
#include "OcclusionDepthTestD3D.h"

#include <NiD3DShaderProgramFactory.h>
#include <NiD3DUtility.h>
#include <NiDX9Renderer.h>

#include <d3dx9math.h>


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

NiImplementRTTI(OcclusionDepthTestD3D::DepthShader, NiD3DShader);

//-----------------------------------------------------------------------------------

OcclusionDepthTestD3D::DepthShader::DepthShader( NiD3DVertexShader* pkDepthOnlyVertexShader, 
                                                 NiD3DPixelShader* pkDepthOnlyPixelShader )
{
    assert( pkDepthOnlyVertexShader && pkDepthOnlyPixelShader );

    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );

    // Setup the shader declaration.
    NiD3DShaderDeclaration* pkShaderDecl = NiDX9ShaderDeclaration::Create( pkRenderer, 1, 1 );
    assert( pkShaderDecl );
    SetShaderDecl( pkShaderDecl );

    pkShaderDecl->SetEntry( 0, 0, NiShaderDeclaration::SHADERPARAM_NI_POSITION, NiShaderDeclaration::SPTYPE_FLOAT3, 0 );

    NiD3DPassPtr spDepthPass = NiD3DPass::CreateNewPass();
    m_kPasses.SetAt( 0, spDepthPass );
    m_uiPassCount = 1;

	spDepthPass->SetPixelShader( pkDepthOnlyPixelShader );
	spDepthPass->SetVertexShader( pkDepthOnlyVertexShader );

	SetUsesNiRenderState( false );
	SetUsesNiLightState( false );

	SetName( "DepthShader" );

	SetIsBestImplementation( true );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

OcclusionDepthTestD3D::OcclusionDepthTestD3D()
    : m_bInit(false)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

OcclusionDepthTestD3D::~OcclusionDepthTestD3D()
{
    Exit();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionDepthTestD3D::Init()
{
    Exit();

    CreateDepthOnlyVertexShader();
    assert( m_spDepthOnlyVertexShader );
    CreateDepthOnlyPixelShader();
    assert( m_spDepthOnlyPixelShader );

    m_spDepthShader = NiNew DepthShader( m_spDepthOnlyVertexShader, m_spDepthOnlyPixelShader );
    m_spDepthMaterial = NiSingleShaderMaterial::Create( m_spDepthShader );

    m_bInit = true;
}

//-----------------------------------------------------------------------------------

void OcclusionDepthTestD3D::Exit()
{
    if ( false == m_bInit )
        return;

    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );

    IDirect3DDevice9* pkDevice = pkRenderer->GetD3DDevice();
    assert( pkDevice );
    pkDevice->SetVertexShader( NULL );
    pkDevice->SetVertexDeclaration( NULL );

    m_bInit = false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionDepthTestD3D::UpdateViewProjMatrix()
{
    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );

    D3DXALIGNEDMATRIX kProjMatrix = pkRenderer->GetD3DProj();
    D3DXALIGNEDMATRIX kViewMatrix = pkRenderer->GetD3DView();
    m_kViewProjMatrix = kViewMatrix * kProjMatrix;
}

//-----------------------------------------------------------------------------------

void OcclusionDepthTestD3D::DrawTestDepth( NiTriShape* pkProxyMesh )
{
    assert( pkProxyMesh && pkProxyMesh->GetSkinInstance() == NULL );

    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );
    if ( pkRenderer->IsDeviceLost() )
        return;

    IDirect3DDevice9* pkDevice = pkRenderer->GetD3DDevice();
    assert( pkDevice );

    D3DXALIGNEDMATRIX kD3DMat;
    NiD3DUtility::GetD3DFromNi( kD3DMat, pkProxyMesh->GetWorldTransform() );
    D3DXALIGNEDMATRIX kWVPMatrix = kD3DMat * m_kViewProjMatrix;

    // Set the world-view-projection matrix.
    pkDevice->SetVertexShaderConstantF( 0, (float*)&kWVPMatrix, 4 );

    // Test depth.
    pkProxyMesh->RenderImmediate( pkRenderer );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionDepthTestD3D::CreateDepthOnlyVertexShader()
{
    // Depth only vertex shader source.
    char const source[] = "float4x4 worldViewProjMatrix : register(c0);\n"\
                          "void main(in float4 IN: POSITION, out float4 OUT : POSITION)\n"\
                          "{\n"\
                          "   OUT = mul(worldViewProjMatrix, IN);\n"\
                          "}\n";

    m_spDepthOnlyVertexShader = 
        NiD3DShaderProgramFactory::CreateVertexShaderFromBuffer( source, (UINT)strlen(source), ".hlsl", 
                                                    "DepthOnlyVertexShader", "main", NULL, NULL, 0 );
}

//-----------------------------------------------------------------------------------

void OcclusionDepthTestD3D::CreateDepthOnlyPixelShader()
{
    // Depth only pixel shader source.
    char const source[] = "float4 main(void) : COLOR\n"\
                          "{\n"\
                          "   return 0;\n"\
                          "}\n";

    m_spDepthOnlyPixelShader = 
        NiD3DShaderProgramFactory::CreatePixelShaderFromBuffer( source, (UINT)strlen(source), ".hlsl", 
                                                                "DepthOnlyPixelShader", "main", NULL );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
