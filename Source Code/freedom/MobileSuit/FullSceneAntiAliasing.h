#pragma once

#include "PgIPostProcessEffect.h"
#include <NiMain.H>
#include "HDRScene.h"

class FullSceneAntiAliasing : public PgIPostProcessEffect
{

public:
    FullSceneAntiAliasing(bool bActive);

	virtual int GetType() const {	return	PgIPostProcessEffect::FSAA;	}

    virtual bool CreateEffect();
    virtual void CleanupEffect();
	virtual NiRenderTargetGroup* GetRenderTargetGroup()	{	return	m_spRTGroupFrameBuffer;	}
	virtual NiRenderedTexture* GetTargetTexture()	{	return	m_spRenderedTextureFrameBuffer;	}
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);

private:
	NiRenderedTexturePtr m_spRenderedTextureFrameBuffer;
	NiRenderedTexturePtr m_spRenderedTextureBlur1;
	NiRenderedTexturePtr m_spRenderedTextureBlur2;

    NiRenderTargetGroupPtr m_spRTGroupFrameBuffer;
	NiRenderTargetGroupPtr m_spRTGroupBlur1;
	NiRenderTargetGroupPtr m_spRTGroupBlur2;

	NiShaderPtr m_spShaderPower;
	NiShaderPtr m_spShaderBlurX;
	NiShaderPtr m_spShaderBlurY;
	NiShaderPtr m_spShaderBlend;
	NiShaderPtr m_spShaderAniso;
	
	NiTexturingPropertyPtr m_spTexturePropertyPower;
	NiTexturingPropertyPtr m_spTexturePropertyBlurX;
	NiTexturingPropertyPtr m_spTexturePropertyBlurY;
	NiTexturingPropertyPtr m_spTexturePropertyBlend;
	NiTexturingPropertyPtr m_spTexturePropertyCopy1;
	NiTexturingPropertyPtr m_spTexturePropertyCopy2;

	NiScreenElementsPtr m_spScreenElementBlur1;
	NiScreenElementsPtr m_spScreenElementBlur2;
	NiScreenElementsPtr m_spScreenElementFinal;

	bool m_bFirstFrame;
};