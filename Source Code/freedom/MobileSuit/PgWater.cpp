#include "Stdafx.h"
#include "PgWater.h"
#include "PgNifMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgWorld.h"
#include "PgPostProcessMan.h"
#include "BrightBloom.h"

PgWater::stWaterNode*	PgWater::CreateNewWaterNode(NiAVObjectPtr	spWaterNode,NiRenderer *pkRenderer)
{
	if(NiIsKindOf(NiGeometry, spWaterNode)== false) return 0;

	stWaterNode	*pNewWater = new stWaterNode();
	pNewWater->m_spWaterNode = NiDynamicCast(NiGeometry,spWaterNode);
	
	pNewWater->m_spRT_UponWater = NiRenderedTexture::Create(m_iTexWidth, m_iTexHeight, pkRenderer);
	pNewWater->m_spRTG_UponWater = NiRenderTargetGroup::Create(pNewWater->m_spRT_UponWater->GetBuffer(), pkRenderer, true, true);;

	NiGeometry *pGeom = pNewWater->m_spWaterNode;

	NiAlphaProperty	*pAlpha = (NiAlphaProperty*)pGeom->GetProperty(NiAlphaProperty::GetType());
	if(!pAlpha)
	{
		pAlpha = NiNew NiAlphaProperty();
		pGeom->AttachProperty(pAlpha);
	}
	pAlpha->SetAlphaBlending(false);

	NiTexturingProperty	*pTexturing = (NiTexturingProperty*)pGeom->GetProperty(NiTexturingProperty::GetType());
	if(pTexturing == 0)
	{
		pTexturing = NiNew NiTexturingProperty();
		pTexturing->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
		pTexturing->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		pGeom->AttachProperty(pTexturing);
	}
	int iTextureSet = pGeom->GetModelData()->GetTextureSets();

	NiTexture*	pkBaseMap = pTexturing->GetBaseTexture();

	pTexturing->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spRT_UnderWater,0));//,NiTexturingProperty::WRAP_S_WRAP_T,NiTexturingProperty::FILTER_BILERP,0));
	pTexturing->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(pNewWater->m_spRT_UponWater,0));//,NiTexturingProperty::WRAP_S_WRAP_T,NiTexturingProperty::FILTER_BILERP,1));
	pTexturing->SetShaderMap(2,NiNew NiTexturingProperty::ShaderMap(m_spWaterBump[0],0));//,NiTexturingProperty::WRAP_S_WRAP_T,NiTexturingProperty::FILTER_BILERP,2));
	pTexturing->SetShaderMap(3,NiNew NiTexturingProperty::ShaderMap(m_spWaterBump[1],0));//,NiTexturingProperty::WRAP_S_WRAP_T,NiTexturingProperty::FILTER_BILERP,3));

	NiShaderPtr spWaterShader = NiShaderFactory::GetInstance()->RetrieveShader("WaterShader", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spWaterShader);
	pGeom->ApplyAndSetActiveMaterial(pkMaterial);

	
	spWaterNode->UpdateEffects();
    spWaterNode->UpdateProperties();
    spWaterNode->Update(0.0f);

	return	pNewWater;
}
void	PgWater::InitFromNode(NiNodePtr spWaterRootNode)
{
	InitFromNodeRecursive(NiDynamicCast(NiAVObject,spWaterRootNode));

	spWaterRootNode->SetAppCulled(true);
}
void	PgWater::InitFromNodeRecursive(NiAVObjectPtr spWaterNode)
{

	stWaterNode	*pNewNode;
	if(NiIsKindOf(NiGeometry,spWaterNode))
	{
		pNewNode = CreateNewWaterNode(spWaterNode,NiRenderer::GetRenderer());
		if(pNewNode)
			m_WaterNodeList.push_back(pNewNode);
		return;
	}

	if(NiIsKindOf(NiNode,spWaterNode))
	{
		NiNode *pNode = NiDynamicCast(NiNode, spWaterNode);

		int iChildCount = pNode->GetArrayCount();
		if(iChildCount == 0) return;

		for(int i=0;i<iChildCount;i++)
			InitFromNodeRecursive(pNode->GetAt(i));
	}
}

void	PgWater::Init()
{
	m_fTime = 0;

	// QUESTION: LOW QUALITY옵션에서는 256x256으로 만들어도 될까?
	m_iTexWidth = 512;
	m_iTexHeight = 512;

	NiSourceTexturePtr	spBumpTemp = 0;

	// TODO: NULL체크 하자.
	m_spWaterBump[0] = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/WaterBumps1.tga");
	m_spWaterBump[1] = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/WaterBumps2.tga");
	spBumpTemp = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/WaterBumps1.tga");

    NiDX9TextureData* pkTexData = (NiDX9TextureData*)m_spWaterBump[0]->GetRendererData();
    LPDIRECT3DTEXTURE9 pkD3DTextureDest = (LPDIRECT3DTEXTURE9)pkTexData->GetD3DTexture();

	pkTexData = (NiDX9TextureData*)spBumpTemp->GetRendererData();
    LPDIRECT3DTEXTURE9 pkD3DTextureSrc = (LPDIRECT3DTEXTURE9)pkTexData->GetD3DTexture();

	D3DXComputeNormalMap(pkD3DTextureDest,pkD3DTextureSrc,0,0,D3DX_CHANNEL_RED,20.0f);

	spBumpTemp = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/WaterBumps2.tga");
    pkTexData = (NiDX9TextureData*)m_spWaterBump[0]->GetRendererData();
    pkD3DTextureDest = (LPDIRECT3DTEXTURE9)pkTexData->GetD3DTexture();

	pkTexData = (NiDX9TextureData*)spBumpTemp->GetRendererData();
    pkD3DTextureSrc = (LPDIRECT3DTEXTURE9)pkTexData->GetD3DTexture();

	D3DXComputeNormalMap(pkD3DTextureDest,pkD3DTextureSrc,0,0,D3DX_CHANNEL_RED,20.0f);

	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();

	m_spRT_UnderWater = NiRenderedTexture::Create(m_iTexWidth, m_iTexHeight, pkRenderer);
	m_spRTG_UnderWater = NiRenderTargetGroup::Create(m_spRT_UnderWater->GetBuffer(), pkRenderer, true, true);;

}
void	PgWater::SetWaterInfoFromXML(const TiXmlNode *pkNode)
{
	pkNode = pkNode->FirstChild();
	while(pkNode)
	{
		int const iType = pkNode->Type();

		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				PG_ASSERT_LOG(pkElement);
				
				char const *pcTagName = pkElement->Value();	
				if(strcmp(pcTagName,"WATER")==0)
				{
					TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					stWaterNode	*pWaterNode = 0;
					//	먼저 이름을 찾는다.
					while(pkAttr)
					{
						if(strcmp(pkAttr->Name(), "NAME") == 0)
						{
							pWaterNode = GetWaterNode(pkAttr->Value());
							break;
						}
						pkAttr = pkAttr->Next();
					}

					PG_ASSERT_LOG(pWaterNode);
					if(pWaterNode == 0)
					{
						PgError("Cannot Find Water Node");
						return;
					}

					pkAttr = pkElement->FirstAttribute();
					while(pkAttr)
					{
						if(strcmp(pkAttr->Name(), "NAME") == 0)
						{
						}
						else if(strcmp(pkAttr->Name(), "WATER_SPEED") == 0)
						{
							pWaterNode->m_fWaterSpeed = (float)atof(pkAttr->Value());
						}
						else if(strcmp(pkAttr->Name(), "WATER_WAVE_SIZE") == 0)
						{
							pWaterNode->m_fWaterWaveSize = (float)atof(pkAttr->Value());
						}
						else if(strcmp(pkAttr->Name(), "WATER_REFLECT") == 0)
						{
							pWaterNode->m_fWaterReflect = (float)atof(pkAttr->Value());
						}
						else if(strcmp(pkAttr->Name(), "WATER_WAVE_HEIGHT") == 0)
						{
							pWaterNode->m_fWaterWaveHeight = (float)atof(pkAttr->Value());
						}
						else if(strcmp(pkAttr->Name(), "WATER_DETAIL_DENSITY") == 0)
						{
							pWaterNode->m_fWaterDetailDensity = (float)atof(pkAttr->Value());
						}
						else
						{
							PgError1("Unknown attribute name %s", pkAttr->Name());
						}
						pkAttr = pkAttr->Next();
					}
				}
			}
		}


		pkNode = pkNode->NextSibling();
	}
}
PgWater::stWaterNode*	PgWater::GetWaterNode(char const *strNodeName)
{
	stWaterNode *pNode;
	for(WaterNodeList::iterator itor = m_WaterNodeList.begin(); itor != m_WaterNodeList.end(); itor++)
	{
		pNode = *itor;
		if(pNode->m_spWaterNode != 0)
		{
			if(pNode->m_spWaterNode->GetName().Equals(strNodeName)) return pNode;
		}
	}
	return 0;
}

void	PgWater::Destroy()
{
	m_spRT_UnderWater = 0;
	m_spRTG_UnderWater = 0;

	m_spWaterBump[0] = 0;
	m_spWaterBump[1] = 0;

	for(WaterNodeList::iterator itor = m_WaterNodeList.begin(); itor != m_WaterNodeList.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	m_WaterNodeList.clear();

}
void	PgWater::UpdateWaterNode(stWaterNode *pWaterNode,PgRenderer *pkRenderer, NiCameraPtr spCamera,PgWorld *pWorld,float fFrameTime)
{
	float	fWaterLevel =pWaterNode->m_spWaterNode->GetWorldBound().GetCenter().z;

	D3DXPLANE WaterPlane(0,0,-1,fWaterLevel);

	NiPoint3	vCam = spCamera->GetWorldLocation();
	NiPoint3	vCamTo = vCam + spCamera->GetWorldDirection()*10;

	D3DXMATRIX	matReflectInMirror;
	D3DXMatrixReflect( &matReflectInMirror, &WaterPlane );

    NiDX9Renderer* pDX9Renderer = NiDynamicCast(NiDX9Renderer,NiRenderer::GetRenderer());
	NiCameraPtr spNewCamera = (NiCamera*)spCamera->Clone();

	D3DXVECTOR3	vNewCamPos,vNewCamLookAt;
	D3DXVec3TransformCoord(&vNewCamPos,&D3DXVECTOR3(vCam.x,vCam.y,vCam.z),&matReflectInMirror);
	D3DXVec3TransformCoord(&vNewCamLookAt,&D3DXVECTOR3(vCamTo.x,vCamTo.y,vCamTo.z),&matReflectInMirror);
	spNewCamera->SetTranslate(NiPoint3(vNewCamPos.x,vNewCamPos.y,vNewCamPos.z));
	spNewCamera->Update(0);
	spNewCamera->LookAtWorldPoint(NiPoint3(vNewCamLookAt.x,vNewCamLookAt.y,vNewCamLookAt.z),NiPoint3::UNIT_Z);
	spNewCamera->Update(0);
	pkRenderer->SetDefaultRenderTargetGroup(pWaterNode->m_spRTG_UponWater);
	pWorld->DrawReflectionWorldScene( pkRenderer, spNewCamera, fFrameTime );
	pkRenderer->SetDefaultRenderTargetGroup(0);
	spNewCamera=0;

	D3DXMATRIX	matView,matProj,matTemp,TrafoMatrix;
	memset(&TrafoMatrix,0,sizeof(D3DXMATRIX));
	TrafoMatrix._11 = 0.5;	TrafoMatrix._12 = 0;		TrafoMatrix._13 = 0; TrafoMatrix._14 = 0;
    TrafoMatrix._21 = 0;	TrafoMatrix._22 = -0.5;		TrafoMatrix._23 = 0; TrafoMatrix._24 = 0;
    TrafoMatrix._31 = 0;	TrafoMatrix._32 = 0;		TrafoMatrix._33 = 0; TrafoMatrix._34 = 0;
    TrafoMatrix._41 = 0.5;	TrafoMatrix._42 = 0.5;		TrafoMatrix._43 = 1; TrafoMatrix._44 = 1;  

	memcpy(&matView,&pDX9Renderer->GetD3DView(),sizeof(float)*4*4);
	memcpy(&matProj,&pDX9Renderer->GetD3DProj(),sizeof(float)*4*4);
	D3DXMatrixMultiply(&matTemp,&matView,&matProj);
	D3DXMatrixMultiply(&pWaterNode->m_matReflectMirrorView,&matTemp,&TrafoMatrix);

}

void	PgWater::UpdateScene(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime,PgWorld *pWorld)
{
	if(m_WaterNodeList.size() == 0) return;

	//	후처리 효과 OFF
	PgPostProcessMan *pPostProcessMan = pkRenderer->m_pkPostProcessMan;
	PgIPostProcessEffect *pBloom = 0;
	if(pPostProcessMan)
	{
		pBloom = pPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
		if(pBloom)
		{
			pBloom->SetActive(false);
		}
	}

	m_bNowUpdating = true;

	for(WaterNodeList::iterator itor = m_WaterNodeList.begin(); itor != m_WaterNodeList.end(); itor++)
	{
		UpdateWaterNode((*itor),pkRenderer,spCamera,pWorld,fFrameTime);
	}

	pkRenderer->SetDefaultRenderTargetGroup(m_spRTG_UnderWater);
	pWorld->DrawReflectionWorldScene( pkRenderer, spCamera, fFrameTime );
	pkRenderer->SetDefaultRenderTargetGroup(0);

	m_bNowUpdating = false;

	//	후처리 효과 ONN
	if(pBloom)
	{
		pBloom->SetActive(true);
	}
}

void	PgWater::RenderImmediate(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime)
{
	if(m_WaterNodeList.size() == 0 || m_bNowUpdating) return;

	m_fTime+= fFrameTime;
	float	fCamPos[3]={spCamera->GetWorldLocation().x,spCamera->GetWorldLocation().y,spCamera->GetWorldLocation().z};

	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fCameraPos"),sizeof(float)*3, fCamPos);
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterTime"),sizeof(float), &m_fTime);

	stWaterNode	*pNode;

	NiGeometry	*pGeom;
	for(WaterNodeList::iterator itor = m_WaterNodeList.begin(); itor != m_WaterNodeList.end(); itor++)
	{
		pNode = *itor;
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("ReflectTexTransform"),sizeof(float)*4*4, &pNode->m_matReflectMirrorView);

		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterSpeed"),sizeof(float), &pNode->m_fWaterSpeed);
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterWaveSize"),sizeof(float), &pNode->m_fWaterWaveSize);
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterReflect"),sizeof(float), &pNode->m_fWaterReflect);
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterWaveHeight"),sizeof(float), &pNode->m_fWaterWaveHeight);
		NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fWaterDetailDensity"),sizeof(float), &pNode->m_fWaterDetailDensity);

		pGeom = (NiGeometry*)pNode->m_spWaterNode;
		pGeom->RenderImmediate(pkRenderer->GetRenderer());

	}
}