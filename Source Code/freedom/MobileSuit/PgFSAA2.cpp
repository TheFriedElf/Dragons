#include "stdafx.h"
#include "PgFSAA2.H"
#include "PgRenderer.H"

PgFSAA2::PgFSAA2(bool bActive): PgIPostProcessEffect(bActive)
{
}
bool PgFSAA2::CreateEffect()
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fResize = lua_tinker::call<float>("GetFSAAResize");

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
	// FSAA2
	//
	m_spScreenElement = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spScreenElement->Insert(4);
    m_spScreenElement->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spScreenElement->UpdateBound();
    m_spScreenElement->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
	
	NiShaderPtr spFSAA2 = NiShaderFactory::GetInstance()->RetrieveShader("FSAA2", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spFSAA2);
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
void PgFSAA2::CleanupEffect()
{
	m_spRenderTargetGroup = 0;
	m_spRenderTarget = 0;
	m_spScreenElement = 0;
}
void PgFSAA2::RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{
	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenElement->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
}

bool	PgFSAA2::GetUseShaderFSAA()
{
	NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(!pDX9Renderer)
	{
		return	false;
	}

	return	(pDX9Renderer->GetPresentParams()->MultiSampleType == D3DMULTISAMPLE_NONE);
}