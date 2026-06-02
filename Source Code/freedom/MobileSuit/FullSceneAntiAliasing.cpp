#include "stdafx.h"
#include "FullSceneAntiAliasing.h"
#include "PgRenderer.h"
#include "HDRScene.h"

FullSceneAntiAliasing::FullSceneAntiAliasing(bool bActive) : PgIPostProcessEffect(bActive)
{
}

void FullSceneAntiAliasing::CleanupEffect()
{
	m_spRenderedTextureFrameBuffer = 0;
	m_spRenderedTextureBlur1 = 0;
	m_spRenderedTextureBlur2 = 0;

	m_spRTGroupFrameBuffer = 0;
	m_spRTGroupBlur1 = 0;
	m_spRTGroupBlur2 = 0;

	m_spShaderPower = 0;
	m_spShaderBlurX = 0;
	m_spShaderBlurY = 0;
	m_spShaderBlend = 0;
	m_spShaderAniso = 0;

	m_spTexturePropertyCopy1 = 0;
	m_spTexturePropertyCopy2 = 0;
	m_spTexturePropertyPower = 0;
	m_spTexturePropertyBlurX = 0;
	m_spTexturePropertyBlurY = 0;
	m_spTexturePropertyBlend = 0;

	m_spScreenElementBlur1 = 0;
	m_spScreenElementBlur2 = 0;
	m_spScreenElementFinal = 0;

	m_bFirstFrame = false;
}

bool FullSceneAntiAliasing::CreateEffect()
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	assert(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	// Render Target
 
	NiTexture::FormatPrefs kPrefs;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;

	m_spRenderedTextureBlur1 = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
	m_spRTGroupBlur1 = NiRenderTargetGroup::Create(m_spRenderedTextureBlur1->GetBuffer(), pkRenderer, true, true);

	m_spRenderedTextureBlur2 = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
	m_spRTGroupBlur2 = NiRenderTargetGroup::Create(m_spRenderedTextureBlur2->GetBuffer(), pkRenderer, true, true);

	m_spRenderedTextureFrameBuffer = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
    m_spRTGroupFrameBuffer = NiRenderTargetGroup::Create(1,pkRenderer);
	m_spRTGroupFrameBuffer->AttachBuffer(m_spRenderedTextureFrameBuffer->GetBuffer(),0);
	//m_spRTGroupFrameBuffer->AttachBuffer(m_spRenderedTextureBlur1->GetBuffer(),1);
	//m_spRTGroupFrameBuffer->AttachBuffer(m_spRenderedTextureBlur2->GetBuffer(),2);
	m_spRTGroupFrameBuffer->AttachDepthStencilBuffer(NiRenderer::GetRenderer()->GetDefaultDepthStencilBuffer());
	
	m_spShaderPower = NiShaderFactory::GetInstance()->RetrieveShader("Power", NiShader::DEFAULT_IMPLEMENTATION, true);
	m_spShaderBlurX = NiShaderFactory::GetInstance()->RetrieveShader("BlurX", NiShader::DEFAULT_IMPLEMENTATION, true);
	m_spShaderBlurY = NiShaderFactory::GetInstance()->RetrieveShader("BlurY", NiShader::DEFAULT_IMPLEMENTATION, true);
	m_spShaderBlend = NiShaderFactory::GetInstance()->RetrieveShader("Blend", NiShader::DEFAULT_IMPLEMENTATION, true);
	m_spShaderAniso = NiShaderFactory::GetInstance()->RetrieveShader("AnisotropicAA", NiShader::DEFAULT_IMPLEMENTATION, true);

	m_spTexturePropertyCopy1 = NiNew NiTexturingProperty;
	m_spTexturePropertyCopy1->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	m_spTexturePropertyCopy1->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	m_spTexturePropertyCopy1->SetBaseTexture(m_spRenderedTextureFrameBuffer);

	m_spTexturePropertyCopy2 = NiNew NiTexturingProperty;
	m_spTexturePropertyCopy2->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	m_spTexturePropertyCopy2->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	m_spTexturePropertyCopy2->SetBaseTexture(m_spRenderedTextureBlur1);

	m_spTexturePropertyPower = NiNew NiTexturingProperty;
	m_spTexturePropertyPower->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	m_spTexturePropertyPower->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	m_spTexturePropertyPower->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureFrameBuffer, 0));

	m_spTexturePropertyBlurX = NiNew NiTexturingProperty;
	m_spTexturePropertyBlurX->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	m_spTexturePropertyBlurX->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	m_spTexturePropertyBlurX->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureBlur2, 0));	

	////! BlurY와 Power는 같이 쓸 수도 있을거 같다.
	//m_spTexturePropertyBlurY = NiNew NiTexturingProperty;
	//m_spTexturePropertyBlurY->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	//m_spTexturePropertyBlurY->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	//m_spTexturePropertyBlurY->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureBlur2, 0));

	m_spTexturePropertyBlend = NiNew NiTexturingProperty;
	m_spTexturePropertyBlend->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	m_spTexturePropertyBlend->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	m_spTexturePropertyBlend->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureBlur2, 0));

	m_spScreenElementBlur1 = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spScreenElementBlur1->Insert(4);
    m_spScreenElementBlur1->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spScreenElementBlur1->UpdateBound();
    m_spScreenElementBlur1->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
	m_spScreenElementBlur1->AttachProperty(m_spTexturePropertyCopy1);
	m_spScreenElementBlur1->UpdateEffects();
    m_spScreenElementBlur1->UpdateProperties();
    m_spScreenElementBlur1->Update(0.0f);

	m_spScreenElementBlur2 = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spScreenElementBlur2->Insert(4);
	m_spScreenElementBlur2->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
	m_spScreenElementBlur2->UpdateBound();
	m_spScreenElementBlur2->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
	m_spScreenElementBlur2->AttachProperty(m_spTexturePropertyCopy2);
	m_spScreenElementBlur2->UpdateEffects();
	m_spScreenElementBlur2->UpdateProperties();
	m_spScreenElementBlur2->Update(0.0f);	

	m_spScreenElementFinal = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spScreenElementFinal->Insert(4);
    m_spScreenElementFinal->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spScreenElementFinal->UpdateBound();
    m_spScreenElementFinal->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.00f);
	m_spScreenElementFinal->AttachProperty(m_spTexturePropertyPower);
  	m_spScreenElementFinal->UpdateEffects();
	m_spScreenElementFinal->UpdateProperties();
    m_spScreenElementFinal->Update(0.0f);

	float y = -3.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y0", sizeof(float), &y);
	y = -2.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y1", sizeof(float), &y);
	y = -1.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y2", sizeof(float), &y);
	y = 1.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y4", sizeof(float), &y);
	y = 2.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y5", sizeof(float), &y);
	y = 3.0f;
	NiShaderFactory::UpdateGlobalShaderConstant("fass_y6", sizeof(float), &y);	

	m_bFirstFrame = true;
	
	return	true;
}
void FullSceneAntiAliasing::RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{
	NiSingleShaderMaterial* pkMaterial = NULL;
	NiTexturingProperty *pkTP = NULL;
	
	//! FrameBuffer -> Blur1
	//pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur1, NiRenderer::CLEAR_ALL);
	//pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	//m_spScreenElementBlur1->Draw(pkRenderer->GetRenderer());
	//pkRenderer->EndUsingRenderTargetGroup();

	//! FrameBuffer AntiAliasing
	NiSingleShaderMaterial* pkMaterialAniso = NiSingleShaderMaterial::Create(m_spShaderAniso);
	m_spScreenElementFinal->ApplyAndSetActiveMaterial(pkMaterialAniso);
	pkTP = (NiTexturingProperty*)m_spScreenElementFinal->GetProperty(NiProperty::TEXTURING);
	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureFrameBuffer,0));
	m_spScreenElementFinal->UpdateEffects();
	m_spScreenElementFinal->UpdateProperties();
	m_spScreenElementFinal->Update(0.0f);

	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementFinal->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();

	////! Blur
	//if (m_bFirstFrame == false)
	//{
	//	NiSingleShaderMaterial* pkMaterialBlend = NiSingleShaderMaterial::Create(m_spShaderBlend);
	//	m_spScreenElementFinal->ApplyAndSetActiveMaterial(pkMaterialBlend);	
	//	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureFrameBuffer,0));
	//	pkTP->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(m_spRenderedTextureBlur2,0));
	//	m_spScreenElementFinal->UpdateEffects();
	//	m_spScreenElementFinal->UpdateProperties();
	//	m_spScreenElementFinal->Update(0.0f);
	//	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL);
	//	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	//	m_spScreenElementFinal->Draw(pkRenderer->GetRenderer());
	//	pkRenderer->EndUsingRenderTargetGroup();
	//}
	//else
	//	m_bFirstFrame = false;

	////! Blur1 -> Blur2
	//pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur2, NiRenderer::CLEAR_ALL);
	//pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	//m_spScreenElementBlur2->Draw(pkRenderer->GetRenderer());
	//pkRenderer->EndUsingRenderTargetGroup();

/*
	//! Copy FrameBuffer to Blur2
	m_spScreenElementBlur2->AttachProperty(m_spTexturePropertyCopy);
	m_spScreenElementBlur2->UpdateEffects();
	m_spScreenElementBlur2->UpdateProperties();
	m_spScreenElementBlur2->Update(0.0f);
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur2, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementBlur2->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();

	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupFrameBuffer, NiRenderer::CLEAR_ALL);

	//! Power(Blur2 -> Blur1)
	pkMaterial = NiSingleShaderMaterial::Create(m_spShaderPower);
	m_spScreenElementBlur1->ApplyAndSetActiveMaterial(pkMaterial);
	m_spScreenElementBlur1->AttachProperty(m_spTexturePropertyPower);
	m_spScreenElementBlur1->UpdateEffects();
	m_spScreenElementBlur1->UpdateProperties();
	m_spScreenElementBlur1->Update(0.0f);

	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur1, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementBlur1->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();

	//! BlurX(Blur1 -> Blur2)
	pkMaterial = NiSingleShaderMaterial::Create(m_spShaderBlurX);
	m_spScreenElementBlur2->ApplyAndSetActiveMaterial(pkMaterial);
	m_spScreenElementBlur2->AttachProperty(m_spTexturePropertyBlurX);
	m_spScreenElementBlur2->UpdateEffects();
	m_spScreenElementBlur2->UpdateProperties();
	m_spScreenElementBlur2->Update(0.0f);

	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur2, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementBlur2->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();

	//! BlurY(Blur2 -> Blur1)
	pkMaterial = NiSingleShaderMaterial::Create(m_spShaderBlurY);
	m_spScreenElementBlur1->ApplyAndSetActiveMaterial(pkMaterial);
	m_spScreenElementBlur1->AttachProperty(m_spTexturePropertyBlurY);
	m_spScreenElementBlur1->UpdateEffects();
	m_spScreenElementBlur1->UpdateProperties();
	m_spScreenElementBlur1->Update(0.0f);
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGroupBlur1, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementBlur1->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();

	//! Final Blending (FrameBuffer + Blur1)
	pkMaterial = NiSingleShaderMaterial::Create(m_spShaderBlend);
	m_spScreenElementFinal->ApplyAndSetActiveMaterial(pkMaterial);
	m_spScreenElementFinal->AttachProperty(m_spTexturePropertyBlend);
	m_spScreenElementFinal->UpdateEffects();
	m_spScreenElementFinal->UpdateProperties();
	m_spScreenElementFinal->Update(0.0f);
	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElementFinal->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
*/
}