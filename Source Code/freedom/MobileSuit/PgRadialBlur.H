#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGRADIALBLUR_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGRADIALBLUR_H
#include "PgIPostProcessEffect.h"
#include <NiMain.H>

class	PgRadialBlur : public PgIPostProcessEffect
{
public:

	 PgRadialBlur(bool bActive);

	 virtual int GetType() const {	return	PgIPostProcessEffect::RADIAL_BLUR;	}

    virtual bool CreateEffect();
    virtual void CleanupEffect();
	virtual NiRenderTargetGroup* GetRenderTargetGroup()	{	return	m_spRenderTargetGroup;	}
	virtual NiRenderedTexture* GetTargetTexture()	{	return	m_spRenderTarget;	}
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);

public:

	NiRenderTargetGroupPtr	m_spRenderTargetGroup;
	NiRenderedTexturePtr	m_spRenderTarget;
	NiScreenElementsPtr m_spScreenElement;

	NiPoint3	m_kPrevCamPos;

};
#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGRADIALBLUR_H