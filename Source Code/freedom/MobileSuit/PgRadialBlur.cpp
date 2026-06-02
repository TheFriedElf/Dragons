#include "stdafx.h"
#include "PgRadialBlur.H"
#include "PgRenderer.H"
#include "PgPilotMan.H"
#include "PgActor.H"

PgRadialBlur::PgRadialBlur(bool bActive): PgIPostProcessEffect(bActive)
{
}
bool PgRadialBlur::CreateEffect()
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fResize = 1.0f;

	NiTexture::FormatPrefs kPrefs;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	m_spRenderTarget = NiRenderedTexture::Create(uiWidth*fResize, uiHeight*fResize, pkRenderer,kPrefs);
	if(!m_spRenderTarget) return false;

    m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderTarget->GetBuffer(), pkRenderer, true, true);
	if(!m_spRenderTargetGroup)
	{
		m_spRenderTarget = 0;
		return	false;
	}

	//////
	//
	// RadialBlur
	//
	m_spScreenElement = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spScreenElement->Insert(4);
    m_spScreenElement->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spScreenElement->UpdateBound();
    m_spScreenElement->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
	
	NiShaderPtr spRadialBlur = NiShaderFactory::GetInstance()->RetrieveShader("RADIAL_BLUR", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spRadialBlur);
	m_spScreenElement->ApplyAndSetActiveMaterial(pkMaterial);

	NiTexturingProperty *pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spRenderTarget, 0));

	m_spScreenElement->AttachProperty(pkProperty);
	m_spScreenElement->UpdateEffects();
    m_spScreenElement->UpdateProperties();
    m_spScreenElement->Update(0.0f);	

	return	true;
}
void PgRadialBlur::CleanupEffect()
{
	m_spRenderTargetGroup = 0;
	m_spRenderTarget = 0;
	m_spScreenElement = 0;
}
void PgRadialBlur::RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{
	NiCamera	*pkCamera = pkRenderer->GetCameraData();
	if(!pkCamera)
	{
		return;
	}

	NiPoint3	const	&kCamPos = pkCamera->GetWorldLocation();
	NiPoint3	const	&kCamDir = pkCamera->GetWorldDirection();

	NiPoint3	kCamMove = kCamPos - m_kPrevCamPos;
	m_kPrevCamPos = kCamPos;
	float	fCamMoveLength = kCamMove.Unitize();

	float	fDot = NiMax(fabs(kCamDir.Dot(kCamMove)),0.0f);

	float	fCamMoveSpeed = NiMin(fCamMoveLength*fDot*0.08,50.0f)/fTime;

	PgActor	*pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(!pkPlayerActor)
	{
		return;
	}


	NiPoint2	kCenterPos,kBlurWidth;
	pkCamera->WorldPtToScreenPt(pkPlayerActor->GetWorldTranslate(),kCenterPos.x,kCenterPos.y);

	kCenterPos.y = 1 - kCenterPos.y;

	kBlurWidth.x = fCamMoveSpeed/1024.0f;
	kBlurWidth.y = fCamMoveSpeed/768.0f;

	NiShaderFactory::UpdateGlobalShaderConstant("g_fRadialBlurCenter", sizeof(float)*2, &kCenterPos);
	NiShaderFactory::UpdateGlobalShaderConstant("g_fRadialBlurWidth", sizeof(float)*2, &kBlurWidth);

	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElement->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
}

