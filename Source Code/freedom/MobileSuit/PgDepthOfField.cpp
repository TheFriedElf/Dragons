#include "stdafx.h"
#include "PgDepthOfField.H"
#include "PgRenderer.h"
#include "PgUIScene.H"

PgDepthOfField::PgDepthOfField(bool bActive)  : PgIPostProcessEffect(bActive)
{
}
bool PgDepthOfField::CreateEffect()
{ 
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	NiTexture::FormatPrefs kPrefs,kPrefs2;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
	kPrefs2.m_ePixelLayout = NiTexture::FormatPrefs::DOUBLE_COLOR_32;
	kPrefs2.m_eAlphaFmt = NiTexture::FormatPrefs::NONE;
	m_spFrameBuffer = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
	m_spDepthBuffer = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs2);
    m_spRenderTargetGroup = NiRenderTargetGroup::Create(2,pkRenderer);
	m_spRenderTargetGroup->AttachBuffer(m_spFrameBuffer->GetBuffer(),0);
	m_spRenderTargetGroup->AttachBuffer(m_spDepthBuffer->GetBuffer(),1);
	m_spRenderTargetGroup->AttachDepthStencilBuffer(NiRenderer::GetRenderer()->GetDefaultDepthStencilBuffer());


	m_ffocalLen = 4.5;
	m_fDlens = 120;
	m_fZfocus = 0;
	m_fMaxCoC = 30;
	m_fScale = 30;
	m_fSceneRange = XUIMgr.GetResolutionSize().x;
/*
	NiD3DShaderConstantMap::m_ffocalLen = m_ffocalLen;
	NiD3DShaderConstantMap::m_fDlens = m_fDlens; 
	NiD3DShaderConstantMap::m_fZfocus = m_fZfocus;
	NiD3DShaderConstantMap::m_fMaxCoC = m_fMaxCoC;  
	NiD3DShaderConstantMap::m_fScale = m_fScale; 
	NiD3DShaderConstantMap::m_fSceneRange = m_fSceneRange;
*/

	SetupFilterKernel();


	float	fDownScaleRate = 1.0f;

	m_spDownScaled[0] = NiRenderedTexture::Create((unsigned int)(uiWidth*fDownScaleRate), (unsigned int)(uiHeight*fDownScaleRate), pkRenderer,kPrefs);
	m_spDownScaled[1] = NiRenderedTexture::Create((unsigned int)(uiWidth*fDownScaleRate), (unsigned int)(uiHeight*fDownScaleRate), pkRenderer,kPrefs);
	
	m_spSE_DownScale = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spSE_DownScale->Insert(4);
    m_spSE_DownScale->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_DownScale->UpdateBound();
    m_spSE_DownScale->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spDownSample = NiShaderFactory::GetInstance()->RetrieveShader("downsample4x", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spDownSample); 
	m_spSE_DownScale->ApplyAndSetActiveMaterial(pkMaterial);
 
	 NiTexturingProperty * pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
  
	m_spSE_DownScale->AttachProperty(pkProperty);
	m_spSE_DownScale->UpdateEffects();
    m_spSE_DownScale->UpdateProperties();
    m_spSE_DownScale->Update(0.0f);	

	//	Gauss Blur
	m_spSE_GaussBlur = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spSE_GaussBlur->Insert(4);
    m_spSE_GaussBlur->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_GaussBlur->UpdateBound();
    m_spSE_GaussBlur->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spGaussBlur = NiShaderFactory::GetInstance()->RetrieveShader("GaussBlur", NiShader::DEFAULT_IMPLEMENTATION, true);
	pkMaterial = NiSingleShaderMaterial::Create(spGaussBlur); 
	m_spSE_GaussBlur->ApplyAndSetActiveMaterial(pkMaterial);
 
	pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0], 0));
  

	m_spSE_GaussBlur->AttachProperty(pkProperty);
	m_spSE_GaussBlur->UpdateEffects();
    m_spSE_GaussBlur->UpdateProperties();
    m_spSE_GaussBlur->Update(0.0f);	

	//	Gauss Bloom

	m_spSE_Bloom = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spSE_Bloom->Insert(4);
    m_spSE_Bloom->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_Bloom->UpdateBound();
    m_spSE_Bloom->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spGaussBloom = NiShaderFactory::GetInstance()->RetrieveShader("GaussBloom", NiShader::DEFAULT_IMPLEMENTATION, true);
	pkMaterial = NiSingleShaderMaterial::Create(spGaussBloom); 
	m_spSE_Bloom->ApplyAndSetActiveMaterial(pkMaterial);
 
	pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0], 0));
  
	m_spSE_Bloom->AttachProperty(pkProperty);
	m_spSE_Bloom->UpdateEffects();
    m_spSE_Bloom->UpdateProperties();
    m_spSE_Bloom->Update(0.0f);	

	m_spSE_DOF = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	m_spSE_DOF->Insert(4);
    m_spSE_DOF->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    m_spSE_DOF->UpdateBound();
    m_spSE_DOF->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spDOF = NiShaderFactory::GetInstance()->RetrieveShader("DepthOfField", NiShader::DEFAULT_IMPLEMENTATION, true);
	pkMaterial = NiSingleShaderMaterial::Create(spDOF); 
	m_spSE_DOF->ApplyAndSetActiveMaterial(pkMaterial);
 
	pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(m_spFrameBuffer, 0));
	pkProperty->SetShaderMap(1, NiNew NiTexturingProperty::ShaderMap(m_spDepthBuffer, 0));
	pkProperty->SetShaderMap(2, NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[1], 0));
  
	m_spSE_DOF->AttachProperty(pkProperty);
	m_spSE_DOF->UpdateEffects();
    m_spSE_DOF->UpdateProperties();
    m_spSE_DOF->Update(0.0f);

    m_spRTGDownScaled = NiRenderTargetGroup::Create(m_spDownScaled[0]->GetBuffer(),pkRenderer,false);
    m_spRTGBlur = NiRenderTargetGroup::Create(m_spDownScaled[1]->GetBuffer(),pkRenderer,false);
    m_spRTGBloomX = NiRenderTargetGroup::Create(m_spDownScaled[0]->GetBuffer(),pkRenderer,false);
    m_spRTGBloomY = NiRenderTargetGroup::Create(m_spDownScaled[1]->GetBuffer(),pkRenderer,false);

	return	true;
} 
void PgDepthOfField::CleanupEffect()
{
	m_spDepthBuffer = 0;
	m_spFrameBuffer = 0; 
	m_spRenderTargetGroup = 0; 
	m_spSE_DOF = 0;

	m_spRTGDownScaled = 0;
	m_spRTGBlur = 0;
	m_spRTGBloomX = 0;
	m_spRTGBloomY = 0;
	m_spSE_DownScale = 0;
	m_spSE_GaussBlur = 0;
	m_spSE_Bloom = 0;
	m_spDownScaled[0]=m_spDownScaled[1]=0;
}
void PgDepthOfField::RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{

	NiPoint2 akSampleOffsets[16];
	float afSampleOffsets[16];
	HDRScene::Point4 akSampleWeights[16];


    NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(pDX9Renderer)
	{
		if(pDX9Renderer->GetPresentParams()->MultiSampleType>0)
			pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,FALSE);

		const	D3DXMATRIX	&kView = pDX9Renderer->GetD3DView();
		D3DXVECTOR3	kPos(g_kPlayerLoc.x,g_kPlayerLoc.y,g_kPlayerLoc.z),kViewPos;

		D3DXVec3TransformCoord(&kViewPos,&kPos,&kView);

		m_fZfocus = kViewPos.z;
		//NiD3DShaderConstantMap::m_fZfocus = m_fZfocus;
	}

  	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("maxCoC"),sizeof(float), &m_fMaxCoC);

	//	Down Scale
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGDownScaled, NiRenderer::CLEAR_ALL); 
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_DownScale->Draw(pkRenderer->GetRenderer());
 	pkRenderer->EndUsingRenderTargetGroup();



	HDRScene::GetSampleOffsets_GaussBlur5x5(m_spDownScaled[0]->GetWidth(), 
		m_spDownScaled[0]->GetHeight(), akSampleOffsets, 
		akSampleWeights);

	UpdateSampleConstants(akSampleOffsets, akSampleWeights);

	//	Gauss Blur
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBlur, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_GaussBlur->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();


	////////////////////////////
	// Bloom Pass for Horizontal
	HDRScene::GetSampleOffsets_Bloom(m_spDownScaled[0]->GetWidth(), 
		afSampleOffsets, akSampleWeights, 3.0f, 1.0f );
	for (int i = 0; i < 16; i++ )
		akSampleOffsets[i] = NiPoint2(afSampleOffsets[i], 0.0f);

	UpdateSampleConstants(akSampleOffsets, akSampleWeights);

	NiTexturingProperty *pkTP = (NiTexturingProperty*)m_spSE_Bloom->GetProperty(NiProperty::TEXTURING);
	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[1],0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBloomX, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_Bloom->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();


	////////////////////////////
	// Bloom Pass for Vertical

	HDRScene::GetSampleOffsets_Bloom(m_spDownScaled[0]->GetHeight(), 
		afSampleOffsets, akSampleWeights, 3.0f, 1.0f);
	for (int i = 0; i < 16; i++ )
		akSampleOffsets[i] = NiPoint2(0.0f, afSampleOffsets[i]);

	UpdateSampleConstants(akSampleOffsets, akSampleWeights);

	pkTP->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spDownScaled[0],0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRTGBloomY, NiRenderer::CLEAR_ALL);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_Bloom->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();


	//	Composition 
	pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget, NiRenderer::CLEAR_ALL); 
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_DOF->Draw(pkRenderer->GetRenderer());
 	pkRenderer->EndUsingRenderTargetGroup();

	if(pDX9Renderer)
	{ 
		if(pDX9Renderer->GetPresentParams()->MultiSampleType>0)
			pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,TRUE);
	}

}
void	PgDepthOfField::SetupFilterKernel()
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	dx = 0.5f/(float)uiWidth;
	float	dy = 0.5f/(float)uiHeight;

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

void	PgDepthOfField::SetParam(char const *ParamName,float fValue)
{
	if(_stricmp(ParamName,"f")==0)
	{
		m_ffocalLen = fValue;
	}
	if(_stricmp(ParamName,"D")==0)
	{
		m_fDlens = fValue;
	}
	if(_stricmp(ParamName,"MaxCoC")==0)
	{
		m_fMaxCoC = fValue;
	}
	if(_stricmp(ParamName,"Scale")==0)
	{
		m_fScale = fValue;
	}
	if(_stricmp(ParamName,"SceneRange")==0)
	{
		m_fSceneRange = fValue;
	}
	/*
	NiD3DShaderConstantMap::m_ffocalLen = m_ffocalLen;
	NiD3DShaderConstantMap::m_fDlens = m_fDlens; 
	NiD3DShaderConstantMap::m_fMaxCoC = m_fMaxCoC;  
	NiD3DShaderConstantMap::m_fScale = m_fScale; 
	NiD3DShaderConstantMap::m_fSceneRange = m_fSceneRange;
	*/
}
float PgDepthOfField::GetParam(char const *ParamName)
{
	return	0;
}
void PgDepthOfField::UpdateSampleConstants(NiPoint2* pkOffsets, HDRScene::Point4* pkWeights)
{

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
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB_0"),sizeof(float), &(pkWeights+0)->x);
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB_1"),sizeof(float), &(pkWeights+1)->x);

        if(NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("gakSampleWeights_BB"),
			sizeof(float) * 16, fData) == false)
		{
			MessageBox(0,_T("UpdateSampleConstants [gakSampleWeights_BB] Failed"),_T(""),MB_OK);
			return;
		}
    }
}
