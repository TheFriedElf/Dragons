#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGFSAA2_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGFSAA2_H
#include "PgIPostProcessEffect.h"
#include <NiMain.H>

class	PgFSAA2 : public PgIPostProcessEffect
{
public:

	 PgFSAA2(bool bActive);

	virtual int GetType() const {	return	PgIPostProcessEffect::FSAA;	}

    virtual bool CreateEffect();
    virtual void CleanupEffect();
	virtual NiRenderTargetGroup* GetRenderTargetGroup()	{	return	m_spRenderTargetGroup;	}
	virtual NiRenderedTexture* GetTargetTexture()	{	return	m_spRenderTarget;	}
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);

	static	bool	GetUseShaderFSAA();

public:

	NiRenderTargetGroupPtr	m_spRenderTargetGroup;
	NiRenderedTexturePtr	m_spRenderTarget;
	NiScreenElementsPtr m_spScreenElement;

};

#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGFSAA2_H