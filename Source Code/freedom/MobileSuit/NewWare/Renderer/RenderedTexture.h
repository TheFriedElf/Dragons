
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : RenderedTexture.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/02/19 LeeJR First Created
//

#ifndef _RENDERER_RENDEREDTEXTURE_H__
#define _RENDERER_RENDEREDTEXTURE_H__


namespace NewWare
{

namespace Renderer
{


class RenderedTexture
{
public:
    RenderedTexture();
    virtual ~RenderedTexture();


    bool Click( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, 
                unsigned int uiWidth, unsigned int uiHeight, NiTexture::FormatPrefs& kFormat, 
                bool bScreenSpaceCamera = true, NiColorA const& kClearColor = NiColorA::WHITE, 
                unsigned int uiClearMode = NiRenderer::CLEAR_ALL );
    virtual void Reset();


    bool IsValidClickableState( NiRenderer* pkRenderer ) const
    {
        assert( pkRenderer );
        return ((pkRenderer->GetInsideFrameState() == true) && 
                (pkRenderer->IsRenderTargetGroupActive() == false));
    }


protected:
    virtual void DoRenderToTexture( NiRenderer* pkRenderer ) = 0;


private:
    void ApplyTextureToTargetObject( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, 
                        unsigned int uiWidth, unsigned int uiHeight, NiTexture::FormatPrefs& kFormat );
    void CreateRenderTargetGroup( NiRenderer* pkRenderer );
    void RenderToTexture( NiRenderer* pkRenderer, bool bScreenSpaceCamera = true, 
                          NiColorA const& kClearColor = NiColorA::WHITE, 
                          unsigned int uiClearMode = NiRenderer::CLEAR_ALL );


private:
    NiRenderedTexturePtr m_spTexture;
    NiRenderTargetGroupPtr m_spRenderTargetGroup;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_RENDEREDTEXTURE_H__
