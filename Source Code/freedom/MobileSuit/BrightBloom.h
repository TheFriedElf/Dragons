#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_BRIGHTBLOOM_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_BRIGHTBLOOM_H

#include "PgIPostProcessEffect.h"
#include <NiMain.H>
#include "HDRScene.h"

class BrightBloom : public PgIPostProcessEffect
{

public:
    BrightBloom(bool bActive);

	virtual int GetType() const {	return	PgIPostProcessEffect::BRIGHT_BLOOM;	}

    virtual bool CreateEffect();
    virtual void CleanupEffect();
	virtual NiRenderTargetGroup* GetRenderTargetGroup()	{	return	m_spRenderTargetGroup;	}
	virtual NiRenderedTexture* GetTargetTexture()	{	return	m_spFrameBuffer;	}
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);
	virtual void	SetParam(char const *ParamName,float fValue);
	virtual float GetParam(char const *ParamName);

	void SetupFilterKernel();
	void UpdateSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights);

	void	SetGaussBlurDev(float fValue);
	void	SetGaussBlurMul(float fValue);
	void	SetSceneIntensity(float fValue);
	void	SetBloomIntensity(float fValue);
	void	SetHighlightIntensity(float fValue);

	void	SetAntiAlias(bool bEnable);

	void	ShutDown();
	void	Restart();

private:

	void	CreateFrameTarget(int iWidth,int iHeight);
	LPDIRECT3DSURFACE9	CreateMSAARenderTargetSurface(int iWidth,int iHeight);
	void	SetSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights,bool bIsHoriz);

private:

	LPDIRECT3DSURFACE9	m_pkFrameSurface;
	Ni2DBufferPtr	m_spFrameSurfaceBuffer;
	NiRenderedTexturePtr	m_spFrameBuffer;

	NiRenderTargetGroupPtr	m_spRenderTargetGroup,m_spRTGDownScaled,m_spRTGBlur,m_spRTGBloomX,m_spRTGBloomY;

	NiScreenElementsPtr	m_spSE_Composite,m_spSE_DownScale,m_spSE_GaussBlur,m_spSE_Bloom;

	NiRenderedTexturePtr	m_spDownScaled[2];

	float	m_fGaussBlurDev,m_fGaussBlurMul;
	float	m_fSceneIntensity,m_fBloomIntensity,m_fBloomHighlightIntensity;


	bool	m_bBloomConstantSetted;

	float	m_fDownScale;

	float	m_fOffsets_Horiz[16*2];
	float	m_fWeights_Horiz[16];

	float	m_fOffsets_Vert[16*2];
	float	m_fWeights_Vert[16];


};




#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_BRIGHTBLOOM_H