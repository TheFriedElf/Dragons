#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGDEPTHOFFIELD_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGDEPTHOFFIELD_H

#include "PgIPostProcessEffect.h"
#include <NiMain.H>
#include "HDRScene.h"
class PgDepthOfField : public PgIPostProcessEffect
{


public:
	PgDepthOfField(bool bActive);

	virtual int GetType() const {	return	PgIPostProcessEffect::DEPTH_OF_FIELD;	}

    virtual bool CreateEffect();
    virtual void CleanupEffect();
	virtual NiRenderTargetGroup* GetRenderTargetGroup()	{	return	m_spRenderTargetGroup;	}
	virtual NiRenderedTexture* GetTargetTexture()	{	return	m_spFrameBuffer;	}
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);
	virtual void	SetParam(char const *ParamName,float fValue);
	virtual float GetParam(char const *ParamName);

	void	SetupFilterKernel();
	void UpdateSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights);
public:

	float	m_ffocalLen,m_fDlens,m_fZfocus,m_fMaxCoC,m_fScale,m_fSceneRange;

private:

	NiRenderedTexturePtr	m_spFrameBuffer;
	NiRenderedTexturePtr	m_spDepthBuffer;

	NiRenderTargetGroupPtr	m_spRenderTargetGroup,m_spRTGDownScaled,m_spRTGBlur,m_spRTGBloomX,m_spRTGBloomY;

	NiScreenElementsPtr	m_spSE_DOF,m_spSE_DownScale,m_spSE_GaussBlur,m_spSE_Bloom;

	NiRenderedTexturePtr	m_spDownScaled[2];

};
#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGDEPTHOFFIELD_H