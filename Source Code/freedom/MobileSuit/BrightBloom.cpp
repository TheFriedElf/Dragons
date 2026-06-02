#include "stdafx.h"
#include "BrightBloom.h"
#include "PgRenderer.h"
#include "HDRScene.h"
#include "PgOption.h"
#include "PgFSAA2.H"

BrightBloom::BrightBloom(bool bActive) : PgIPostProcessEffect(bActive)
{
	m_bBloomConstantSetted = false;
	m_fDownScale = 1.0f/2.0f;
	m_pkFrameSurface = NULL;
	m_spFrameSurfaceBuffer = 0;

	memset(m_fOffsets_Horiz,0,sizeof(m_fOffsets_Horiz));
	memset(m_fWeights_Horiz,0,sizeof(m_fWeights_Horiz));
	memset(m_fOffsets_Vert,0,sizeof(m_fOffsets_Vert));
	memset(m_fWeights_Vert,0,sizeof(m_fWeights_Vert));
}

void BrightBloom::CleanupEffect()
{
	m_spFrameBuffer = 0; 
	m_spRenderTargetGroup = 0; 
	m_spSE_Composite = 0;

	m_spRTGDownScaled = 0;
	m_spRTGBlur = 0;
	m_spRTGBloomX = 0;
	m_spRTGBloomY = 0;
	m_spSE_DownScale = 0;
	m_spSE_GaussBlur = 0;
	m_spSE_Bloom = 0;
	m_spDownScaled[0]=m_spDownScaled[1]=0;

	m_spFrameSurfaceBuffer = 0;
}
void	BrightBloom::ShutDown()
{
	if(PgFSAA2::GetUseShaderFSAA())
	{
		return;
	}

	if(!m_spRenderTargetGroup)
	{
		return;
	}

    Ni2DBuffer* pkBuffer = m_spRenderTargetGroup->GetBuffer(0);
	if(!pkBuffer)
	{
		return;
	}

    NiDX9Direct3DBufferData* pkBufferData = 
        (NiDX9Direct3DBufferData*)pkBuffer->GetRendererData();
	if(pkBufferData)
	{
		pkBufferData->PurgeBufferData();
	}

}
void	BrightBloom::Restart()
{
	if(PgFSAA2::GetUseShaderFSAA())
	{
		return;
	}
	if(!m_spRenderTargetGroup)
	{
		return;
	}

    Ni2DBuffer* pkBuffer = m_spRenderTargetGroup->GetBuffer(0);
	if(!pkBuffer)
	{
		return;
	}

    NiDX9Direct3DBufferData* pkBufferData = 
        (NiDX9Direct3DBufferData*)pkBuffer->GetRendererData();
	if(!pkBufferData)
	{
		return;
	}

	if(!m_spFrameBuffer)
	{
		return;
	}

    m_pkFrameSurface = CreateMSAARenderTargetSurface(m_spFrameBuffer->GetWidth(),m_spFrameBuffer->GetHeight());
	if(!m_pkFrameSurface)
	{
		return;
	}

    pkBufferData->RecreateBufferData(m_pkFrameSurface);

}
LPDIRECT3DSURFACE9 BrightBloom::CreateMSAARenderTargetSurface(int iWidth,int iHeight)
{
	NiDX9Renderer* pkDX9Renderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert (pkDX9Renderer);

    D3DFORMAT eFormat = NiDX9PixelFormat::DetermineD3DFormat(
        *m_spFrameBuffer->GetPixelFormat());

    // Get multisample information from renderer
    unsigned int uiWidth;
    unsigned int uiHeight;
    unsigned int uiUseFlags;
    NiWindowRef kWndDevice;
    NiWindowRef kWndFocus;
    unsigned int uiAdapter;
    NiDX9Renderer::DeviceDesc eDesc;
    NiDX9Renderer::FrameBufferFormat eFBFormat;
    NiDX9Renderer::DepthStencilFormat eDSFormat;
    NiDX9Renderer::PresentationInterval ePresentationInterval;
    NiDX9Renderer::SwapEffect eSwapEffect;
    unsigned int uiFBMode;
    unsigned int uiBackBufferCount; 
    unsigned int uiRefreshRate;

    pkDX9Renderer->GetCreationParameters(uiWidth, uiHeight, uiUseFlags, 
        kWndDevice, kWndFocus, uiAdapter, eDesc, eFBFormat, eDSFormat, 
        ePresentationInterval, eSwapEffect, uiFBMode, uiBackBufferCount, 
        uiRefreshRate);
    D3DMULTISAMPLE_TYPE eMultisampleType = NiDX9Renderer::GetMultiSampleType(uiFBMode);
    int uiMultisampleQuality = 0;
    if (eMultisampleType == D3DMULTISAMPLE_NONMASKABLE)
    {
        uiMultisampleQuality = 
            (uiFBMode & ~NiDX9Renderer::FBMODE_MULTISAMPLES_NONMASKABLE);
    }

    // Create render target surface
    LPDIRECT3DDEVICE9 pkDevice = pkDX9Renderer->GetD3DDevice();

    LPDIRECT3DSURFACE9 pkRenderSurface = NULL;
    HRESULT hr = E_FAIL;

    hr = pkDevice->CreateRenderTarget(iWidth,
        iHeight, eFormat, eMultisampleType, 
        uiMultisampleQuality, FALSE, &pkRenderSurface, NULL);

    assert (SUCCEEDED(hr) && pkRenderSurface);

    return pkRenderSurface;
}
void	BrightBloom::CreateFrameTarget(int iWidth,int iHeight)
{
	if(PgFSAA2::GetUseShaderFSAA())
	{
		return;
	}

    m_pkFrameSurface = CreateMSAARenderTargetSurface(iWidth,iHeight);
	if(!m_pkFrameSurface)
	{
		return;
	}

    Ni2DBuffer* pkColorBuffer = NULL;
    NiDX92DBufferData* pkColorBufferData = 
        NiDX9Direct3DBufferData::Create(m_pkFrameSurface,
        pkColorBuffer);
    assert (pkColorBufferData && pkColorBuffer);

    // Create NiRenderTargetGroup using default depth/stencil buffer
    // We know that the size and MS levels of the new render target surface
    // are identical to the default back buffer, so we know the default 
    // depth/stencil buffer is compatible.
    m_spRenderTargetGroup = NiRenderTargetGroup::Create(
		pkColorBuffer, NiRenderer::GetRenderer(), true);
}
void	BrightBloom::SetAntiAlias(bool bEnable)
{
	if(PgFSAA2::GetUseShaderFSAA() == false)
	{
		return;
	}

	if(!m_spFrameBuffer)
	{
		return;
	}
	
	NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());
	if(!pkRenderer)
	{
		return;
	}

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fDownScaleRate = m_fDownScale;
	if(!m_spFrameBuffer)
	{
		return;
	}

	int	iFrameBufferWidth = m_spFrameBuffer->GetWidth();
	int	iFrameBufferHeight = m_spFrameBuffer->GetHeight();

	if(bEnable)
	{
		float	fSizeMul = lua_tinker::call<float>("GetFSAAResize");

		uiWidth = uiWidth * fSizeMul;
		uiHeight = uiHeight * fSizeMul;
	}

	if(iFrameBufferWidth != uiWidth || iFrameBufferHeight != uiHeight)
	{
		//	새로 만들자
		NiTexture::FormatPrefs kPrefs,kPrefs2;
		kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
		kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
		kPrefs.m_eMipMapped = NiTexture::FormatPrefs::NO;
		m_spFrameBuffer = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
		if(!m_spFrameBuffer)
		{
			return;
		}
		m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spFrameBuffer->GetBuffer(),pkRenderer->GetRenderer(),true,true);
		if(!m_spRenderTargetGroup)
		{
			return;
		}

		if(m_spSE_DownScale)
		{
			NiPropertyState	*pkProp = m_spSE_DownScale->GetPropertyState();
			if(pkProp)
			{
				NiTexturingProperty	*pkTexturing = pkProp->GetTexturing();
				if(pkTexturing)
				{
					pkTexturing->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
				}
			}
		}
		if(m_spSE_Composite)
		{
			NiPropertyState	*pkProp = m_spSE_Composite->GetPropertyState();
			if(pkProp)
			{
				NiTexturingProperty	*pkTexturing = pkProp->GetTexturing();
				if(pkTexturing)
				{
					pkTexturing->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
				}
			}
		}

	}

}
bool BrightBloom::CreateEffect()
{
	NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	PG_ASSERT_LOG(pkRenderer);

	if(!pkRenderer)
	{
		return	false;
	}

	if(pkRenderer->GetVertexShaderVersion() < D3DVS_VERSION(1,1) ||
		pkRenderer->GetPixelShaderVersion() < D3DPS_VERSION(2,0))
	{
		return	false;
	}

	unsigned int uiScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	unsigned int uiWidth = uiScreenWidth;
	unsigned int uiHeight = uiScreenHeight;

	m_fDownScale = lua_tinker::call<float>("GetBloomDownScale");

	float	fDownScaleRate =  m_fDownScale;

	bool bFSAA = g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS) == 1;
	
	float	const	fSizeMul = lua_tinker::call<float>("GetFSAAResize");

	if (bFSAA)
	{
		uiWidth = uiWidth * fSizeMul;
		uiHeight = uiHeight * fSizeMul;
	}

	NiTexture::FormatPrefs kPrefs,kPrefs2;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
	kPrefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

	if(PgFSAA2::GetUseShaderFSAA())
	{
		m_spFrameBuffer = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
		if(!m_spFrameBuffer || !m_spFrameBuffer->GetRendererData())
		{
			return	false;
		}

		m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spFrameBuffer->GetBuffer(),
			pkRenderer->GetRenderer(),true,true);
	}
	else
	{
		m_spFrameBuffer = NiRenderedTexture::Create(uiScreenWidth, uiScreenHeight, pkRenderer,kPrefs);
		if(!m_spFrameBuffer)
		{
			return	false;
		}

		CreateFrameTarget(uiScreenWidth,uiScreenHeight);
	}

	SetupFilterKernel();

	m_spDownScaled[0] = NiRenderedTexture::Create((unsigned int)(uiScreenWidth*fDownScaleRate), (unsigned int)(uiScreenHeight*fDownScaleRate), pkRenderer,kPrefs);
	m_spDownScaled[1] = NiRenderedTexture::Create((unsigned int)(uiScreenWidth*fDownScaleRate), (unsigned int)(uiScreenHeight*fDownScaleRate), pkRenderer,kPrefs);
	if(!m_spDownScaled[0] || !m_spDownScaled[1])
	{
		return	false;
	}
	
	m_spSE_DownScale = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	if(!m_spSE_DownScale)
	{
		return false;
	}
	m_spSE_DownScale->Insert(4);
    m_spSE_DownScale->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_DownScale->UpdateBound();
    m_spSE_DownScale->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiSourceTexturePtr	spLumiLookUpMap = NiSourceTexture::Create("../Shader/Textures/LumiLookUp.dds");
	NiShaderPtr spDownSample = NiShaderFactory::GetInstance()->RetrieveShader("downsample4x", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spDownSample); 
	m_spSE_DownScale->ApplyAndSetActiveMaterial(pkMaterial);
 
	 NiTexturingProperty * pkProperty = NiNew NiTexturingProperty;
	 if(pkProperty)
	 {
		 pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
		 pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		 pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
		 pkProperty->SetShaderMap(1, NiNew NiTexturingProperty::ShaderMap(spLumiLookUpMap, 0));

		 m_spSE_DownScale->AttachProperty(pkProperty);
		 m_spSE_DownScale->UpdateEffects();
		 m_spSE_DownScale->UpdateProperties();
		 m_spSE_DownScale->Update(0.0f);	
	 }

	//	Gauss Blur
	m_spSE_GaussBlur = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	if(!m_spSE_GaussBlur)
	{
		return false;
	}
	m_spSE_GaussBlur->Insert(4);
    m_spSE_GaussBlur->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_GaussBlur->UpdateBound();
    m_spSE_GaussBlur->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spGaussBlur = NiShaderFactory::GetInstance()->RetrieveShader("GaussBlur", NiShader::DEFAULT_IMPLEMENTATION, true);
	if(spGaussBlur)
	{	
		pkMaterial = NiSingleShaderMaterial::Create(spGaussBlur); 
		m_spSE_GaussBlur->ApplyAndSetActiveMaterial(pkMaterial);
	}
 
	pkProperty = NiNew NiTexturingProperty;
	if(pkProperty)
	{
		pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
		pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0], 0));

		m_spSE_GaussBlur->AttachProperty(pkProperty);
		m_spSE_GaussBlur->UpdateEffects();
		m_spSE_GaussBlur->UpdateProperties();
		m_spSE_GaussBlur->Update(0.0f);	
	}	

	//	Gauss Bloom

	m_spSE_Bloom = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	if(m_spSE_Bloom)
	{
		m_spSE_Bloom->Insert(4);
		m_spSE_Bloom->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
		m_spSE_Bloom->UpdateBound();
		m_spSE_Bloom->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	NiShaderPtr spGaussBloom = NiShaderFactory::GetInstance()->RetrieveShader("GaussBloom", NiShader::DEFAULT_IMPLEMENTATION, true);
	if(spGaussBloom)
	{
		pkMaterial = NiSingleShaderMaterial::Create(spGaussBloom); 
		m_spSE_Bloom->ApplyAndSetActiveMaterial(pkMaterial);

		pkProperty = NiNew NiTexturingProperty;
		pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
		pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[1], 0));

		m_spSE_Bloom->AttachProperty(pkProperty);
		m_spSE_Bloom->UpdateEffects();
		m_spSE_Bloom->UpdateProperties();
		m_spSE_Bloom->Update(0.0f);	
	}

	m_spSE_Composite = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	if(m_spSE_Composite)
	{
		m_spSE_Composite->Insert(4);
		m_spSE_Composite->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
		m_spSE_Composite->UpdateBound();
		m_spSE_Composite->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

		NiShaderPtr spComposite = NiShaderFactory::GetInstance()->RetrieveShader("BloomComposite", NiShader::DEFAULT_IMPLEMENTATION, true);
		pkMaterial = NiSingleShaderMaterial::Create(spComposite); 
		m_spSE_Composite->ApplyAndSetActiveMaterial(pkMaterial);
	}
	pkProperty = NiNew NiTexturingProperty;
	if(pkProperty)
	{
		pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
		pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
		pkProperty->SetShaderMap(1, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0], 0));

		m_spSE_Composite->AttachProperty(pkProperty);
		m_spSE_Composite->UpdateEffects();
		m_spSE_Composite->UpdateProperties();
		m_spSE_Composite->Update(0.0f);
	}

    m_spRTGDownScaled = NiRenderTargetGroup::Create(m_spDownScaled[0]->GetBuffer(),pkRenderer,false);
    m_spRTGBlur = NiRenderTargetGroup::Create(m_spDownScaled[1]->GetBuffer(),pkRenderer,false);
    m_spRTGBloomX = NiRenderTargetGroup::Create(m_spDownScaled[1]->GetBuffer(),pkRenderer,false);
    m_spRTGBloomY = NiRenderTargetGroup::Create(m_spDownScaled[0]->GetBuffer(),pkRenderer,false);

	m_bBloomConstantSetted = false;

	return	true;
}

void	BrightBloom::SetSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights,bool bIsHoriz)
{
	//float fData[16*4];

	for(int i=0;i<16;i++) 
	{
		if(bIsHoriz)
		{
			m_fOffsets_Horiz[i*2+0] = (pkOffsets+i)->x;
			m_fOffsets_Horiz[i*2+1] = (pkOffsets+i)->y;
		}
		else
		{
			m_fOffsets_Vert[i*2+0] = (pkOffsets+i)->x;
			m_fOffsets_Vert[i*2+1] = (pkOffsets+i)->y;
		}
	}

    if (pkWeights)
    {
		for(int i=0;i<16;i++) 
		{
			if(bIsHoriz)
			{
				m_fWeights_Horiz[i] = (pkWeights+i)->x;
			}
			else
			{
				m_fWeights_Vert[i] = (pkWeights+i)->x;
			}
		}
    }
}
void BrightBloom::RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{

	//static	DWORD	dwStartTime = BM::GetTime32();
	//static	stTimeCheck	kTC[30];
	//kTC[0].Start();

	if(!m_bBloomConstantSetted)
	{
		m_bBloomConstantSetted = true;

		SetGaussBlurDev(lua_tinker::call<float>("GetBloom_GaussBlurDev"));
		SetGaussBlurMul(lua_tinker::call<float>("GetBloom_GaussBlurMul"));

		SetSceneIntensity(lua_tinker::call<float>("GetBloom_SceneIntensity"));
		SetBloomIntensity(lua_tinker::call<float>("GetBloom_BloomIntensity"));
		SetHighlightIntensity(lua_tinker::call<float>("GetBloom_HilightIntensity"));

		NiPoint2 akSampleOffsets[16];
		float afSampleOffsets[16];
		HDRScene::Point4 akSampleWeights[16];

		HDRScene::GetSampleOffsets_Bloom(m_spDownScaled[0]->GetWidth(), 
			afSampleOffsets, akSampleWeights, m_fGaussBlurDev, m_fGaussBlurMul );
		for (int i = 0; i < 16; i++ )
			akSampleOffsets[i] = NiPoint2(afSampleOffsets[i], 0.0f);

		SetSampleConstants(akSampleOffsets, akSampleWeights,true);

		HDRScene::GetSampleOffsets_Bloom(m_spDownScaled[0]->GetHeight(), 
			afSampleOffsets, akSampleWeights, m_fGaussBlurDev, m_fGaussBlurMul);
		for (int i = 0; i < 16; i++ )
			akSampleOffsets[i] = NiPoint2(0.0f, afSampleOffsets[i]);

		SetSampleConstants(akSampleOffsets, akSampleWeights,false);

	}

	bool	bFSAA = g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS) == 1;

    NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(!pDX9Renderer)
	{
		return;
	}
	//	Down Scale

	//kTC[1].Start();
	if(PgFSAA2::GetUseShaderFSAA() == false)
	{
		NiDX9SourceTextureData* pkTargetTexData = (NiDX9SourceTextureData *)m_spFrameBuffer->GetRendererData();
		if(!pkTargetTexData)
		{
			return;	//	어떻게 이런일이??
		}
		LPDIRECT3DTEXTURE9 pkTargetD3DTexture = (LPDIRECT3DTEXTURE9)pkTargetTexData->GetD3DTexture();
		if(!pkTargetD3DTexture)
		{
			return;
		}
		LPDIRECT3DSURFACE9	pkTargetSurface = NULL;
		pkTargetD3DTexture->GetSurfaceLevel(0,&pkTargetSurface);
		if(!pkTargetSurface)
		{
			return;
		}
		if(!pDX9Renderer->GetD3DDevice())
		{
			return;
		}
		pDX9Renderer->GetD3DDevice()->StretchRect(m_pkFrameSurface,NULL,pkTargetSurface,NULL, D3DTEXF_POINT);

		pkTargetSurface->Release();
	}
	//kTC[1].End();

	//if(pDX9Renderer)
	//{
	//	if(pDX9Renderer->GetPresentParams()->MultiSampleType>0)
	//	{
	//		pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,FALSE);
	//	}
	//}

	//kTC[2].Start();
	if(!pkRenderer)
	{
		return;
	}
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGDownScaled, NiRenderer::CLEAR_NONE); 
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	if(!m_spSE_DownScale)
	{
		return;
	}
	m_spSE_DownScale->Draw(pkRenderer->GetRenderer());
 	pkRenderer->EndUsingRenderTargetGroup();
	//kTC[2].End();

	//kTC[3].Start();
	//HDRScene::GetSampleOffsets_GaussBlur5x5(m_spDownScaled[0]->GetWidth(), 
	//	m_spDownScaled[0]->GetHeight(), akSampleOffsets, 
	//	akSampleWeights);

	//UpdateSampleConstants(akSampleOffsets, akSampleWeights);
	//kTC[3].End();

	//kTC[4].Start();
	////	Gauss Blur
	//pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBlur, NiRenderer::CLEAR_NONE);
	//pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	//m_spSE_GaussBlur->Draw(pkRenderer->GetRenderer());
	//pkRenderer->EndUsingRenderTargetGroup();
	//kTC[4].End();

	////////////////////////////
	// Bloom Pass for Horizontal
	//kTC[5].Start();
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleOffsets_BB"),
        sizeof(float) * 16 * 2, m_fOffsets_Horiz);
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB"),
				sizeof(float) * 16, m_fWeights_Horiz);
	//kTC[5].End();

	//kTC[6].Start();
	NiTexturingProperty *pkTP = (NiTexturingProperty*)m_spSE_Bloom->GetProperty(NiProperty::TEXTURING);
	if(!pkTP)
	{
		return;
	}
	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0],0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBloomX, NiRenderer::CLEAR_NONE);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	if(!m_spSE_Bloom)
	{
		return;
	}
	m_spSE_Bloom->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
	//kTC[6].End();


	////////////////////////////
	// Bloom Pass for Vertical

	//kTC[7].Start();
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleOffsets_BB"),
        sizeof(float) * 16 * 2, m_fOffsets_Vert);
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB"),
				sizeof(float) * 16, m_fWeights_Vert);
	//kTC[7].End();;

	//kTC[8].Start();
	if(!pkTP)
	{
		return;
	}
	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[1],0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBloomY, NiRenderer::CLEAR_NONE);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	if(!m_spSE_Bloom)
	{
		return;
	}
	m_spSE_Bloom->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
	//kTC[8].End();


	////	Composition 
	//kTC[9].Start();
	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_NONE); 
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	if(!m_spSE_Composite)
	{
		return;
	}
	m_spSE_Composite->Draw(pkRenderer->GetRenderer());
 	pkRenderer->EndUsingRenderTargetGroup();
	//kTC[9].End();

	//if(bFSAA)
	//{
	//	if(pDX9Renderer)
	//	{ 
	//		if(pDX9Renderer->GetPresentParams()->MultiSampleType>0)
	//			pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,TRUE);
	//	}
	//}

	//kTC[0].End();

	//if(BM::GetTime32() - dwStartTime>10000)
	//{
	//	int	iTotal = 10;
	//	char	msg[512];
	//	msg[0]= '\0';
	//	for(int i=0;i<iTotal;i++)
	//	{
	//		char	msg2[100];
	//		sprintf_s(msg2,"[%d]%d ",i,kTC[i].dwElapsedTime);
	//		strcat(msg,msg2);
	//		kTC[i].Reset();
	//	}

	//	::MessageBox(NULL,UNI(msg),_T("TimeCheck"),MB_OK);
	//	dwStartTime = BM::GetTime32();
	//}

}
void	BrightBloom::SetGaussBlurDev(float fValue)
{
	m_fGaussBlurDev = fValue;
}
void	BrightBloom::SetGaussBlurMul(float fValue)
{
	m_fGaussBlurMul = fValue;
}
void	BrightBloom::SetSceneIntensity(float fValue)
{
	m_fSceneIntensity = fValue;

	NiShaderFactory::UpdateGlobalShaderConstant("Bloom_SceneIntensity", sizeof(float), &m_fSceneIntensity);
}
void	BrightBloom::SetHighlightIntensity(float fValue)
{
	m_fBloomHighlightIntensity = fValue;

	NiShaderFactory::UpdateGlobalShaderConstant("Bloom_HighlightIntensity", sizeof(float), &m_fBloomHighlightIntensity);
}
void	BrightBloom::SetBloomIntensity(float fValue)
{
	m_fBloomIntensity = fValue;

	NiShaderFactory::UpdateGlobalShaderConstant("Bloom_BloomIntensity", sizeof(float), &m_fBloomIntensity);
}
void	BrightBloom::SetupFilterKernel()
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	float	fDownScale = m_fDownScale;

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	dx = 1.0f/((float)uiWidth*fDownScale);
	float	dy = 1.0f/((float)uiHeight*fDownScale);

	D3DXVECTOR4	v[12];

	v[0]	=	D3DXVECTOR4(-0.326212f * dx, -0.40581f * dy, 0.0f, 0.0f);
	v[1]	=	D3DXVECTOR4(-0.840144f * dx, -0.07358f * dy, 0.0f, 0.0f);
	v[2]	=	D3DXVECTOR4(-0.695914f * dx, 0.457137f * dy, 0.0f, 0.0f);
	v[3]	=	D3DXVECTOR4(-0.203345f * dx, 0.620716f * dy, 0.0f, 0.0f);
	v[4]	=	D3DXVECTOR4(0.96234f * dx, -0.194983f * dy, 0.0f, 0.0f);
	v[5]	=	D3DXVECTOR4(0.473434f * dx, -0.480026f * dy, 0.0f, 0.0f);
	v[6]	=	D3DXVECTOR4(0.519456f * dx, 0.767022f * dy, 0.0f, 0.0f);
	v[7]	=	D3DXVECTOR4(0.185461f * dx, -0.893124f * dy, 0.0f, 0.0f);
	v[8]	=	D3DXVECTOR4(0.507431f * dx, 0.064425f * dy, 0.0f, 0.0f);
	v[9]	=	D3DXVECTOR4(0.89642f * dx, 0.412458f * dy, 0.0f, 0.0f);
	v[10]	=	D3DXVECTOR4(-0.32194f * dx, -0.932615f * dy, 0.0f, 0.0f);
	v[11]	=	D3DXVECTOR4(-0.791559f * dx, -0.59771f * dy, 0.0f, 0.0f);

	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("filterTaps"),sizeof(D3DXVECTOR4)*12, v);
}

void	BrightBloom::SetParam(char const *ParamName,float fValue)
{
/*	if(_stricmp(ParamName,"Brightness")==0) m_fBrightness = fValue;
	if(_stricmp(ParamName,"BlurWidth")==0) m_fBlurWidth = fValue;
	if(_stricmp(ParamName,"SceneIntensity")==0) m_fSceneIntensity = fValue;
	if(_stricmp(ParamName,"GlowIntensity")==0) m_fGlowIntensity = fValue;
	if(_stricmp(ParamName,"HighlightIntensity")==0) m_fHighLightIntensity = fValue;
	if(_stricmp(ParamName,"BlurCount")==0) m_iBlurCount = (int)fValue;

	NiShaderFactory::UpdateGlobalShaderConstant("g_fBloomScale", sizeof(float), &m_fBrightness);
	NiShaderFactory::UpdateGlobalShaderConstant("g_fBlur", sizeof(float), &m_fBlurWidth);
	NiShaderFactory::UpdateGlobalShaderConstant("g_fSceneIntensity", sizeof(float), &m_fSceneIntensity);
	NiShaderFactory::UpdateGlobalShaderConstant("g_fGlowIntensity", sizeof(float), &m_fGlowIntensity);
	NiShaderFactory::UpdateGlobalShaderConstant("g_fHighlightIntensity", sizeof(float), &m_fHighLightIntensity);
*/
}
float BrightBloom::GetParam(char const *ParamName)
{
	/*
	if(_stricmp(ParamName,"Brightness")==0) return m_fBrightness;
	if(_stricmp(ParamName,"BlurWidth")==0) return m_fBlurWidth;
	if(_stricmp(ParamName,"SceneIntensity")==0) return m_fSceneIntensity;
	if(_stricmp(ParamName,"GlowIntensity")==0) return m_fGlowIntensity;
	if(_stricmp(ParamName,"HighlightIntensity")==0) return m_fHighLightIntensity;
	if(_stricmp(ParamName,"BlurCount")==0) return (float)m_iBlurCount;
*/
	return 0;
}
void BrightBloom::UpdateSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights)
{
	if(!pkOffsets)
	{
		return;
	}

	float fData[16*4];

	for(int i=0;i<16;i++) 
	{
		fData[i*2+0] = (pkOffsets+i)->x;
		fData[i*2+1] = (pkOffsets+i)->y;
	}

    PG_ASSERT_LOG(pkOffsets);
    if(NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleOffsets_BB"),
        sizeof(float) * 16 * 2, fData) == false)
	{
		MessageBox(0,_T("UpdateSampleConstants [gakSampleOffsets_BB] Failed"),_T(""),MB_OK);
		return;
	}

    if (pkWeights)
    {
		for(int i=0;i<16;i++) 
		{
			fData[i] = (pkWeights+i)->x;
		}
        if(NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB"),
			sizeof(float) * 16, fData) == false)
		{
			MessageBox(0,_T("UpdateSampleConstants [gakSampleWeights_BB] Failed"),_T(""),MB_OK);
			return;
		}
    }
}
