
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionDepthTestD3D.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONDEPTHTESTD3D_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONDEPTHTESTD3D_H__

#include <NiD3DShader.h>
class NiSingleShaderMaterial;


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


class OcclusionDepthTestD3D
{
private:
    class DepthShader : public NiD3DShader
    {
        NiDeclareRTTI;

    public:
        DepthShader( NiD3DVertexShader* pkDepthOnlyVertexShader, NiD3DPixelShader* pkDepthOnlyPixelShader );
    };
    typedef NiPointer<DepthShader> DepthShaderPtr;

    typedef NiPointer<NiSingleShaderMaterial> NiSingleShaderMaterialPtr;


public:
    OcclusionDepthTestD3D();
    ~OcclusionDepthTestD3D();


    void Init();
    void Exit();

    void UpdateViewProjMatrix();
    void DrawTestDepth( NiTriShape* pkProxyMesh );

    NiSingleShaderMaterial* GetDepthMaterial() const { return m_spDepthMaterial; };


protected:
    void CreateDepthOnlyVertexShader();
    void CreateDepthOnlyPixelShader();


private:
    bool m_bInit;
    D3DXALIGNEDMATRIX m_kViewProjMatrix;
    NiD3DVertexShaderPtr m_spDepthOnlyVertexShader;
    NiD3DPixelShaderPtr m_spDepthOnlyPixelShader;
    DepthShaderPtr m_spDepthShader;
    NiSingleShaderMaterialPtr m_spDepthMaterial;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONDEPTHTESTD3D_H__
