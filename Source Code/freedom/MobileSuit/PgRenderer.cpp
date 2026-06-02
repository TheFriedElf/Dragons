#include "StdAfx.h"
#include "PgRenderer.h"
#include "PgPostProcessMan.h"
#include "NiOcclusionCuller.H"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Renderer/DrawWorkflow.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"


PgRenderer* PgRenderer::ms_pkPgRenderer = NULL;
int	PgRenderer::m_iBonesPerPartition = 4;

PgRenderer::PgRenderer(void)
    : m_kCuller_Deprecated(NULL)
{
	ms_pkPgRenderer = this;

	m_spRenderer = 0;

	m_pkCamera = NULL;

	m_pkPostProcessMan = NiNew PgPostProcessMan;

	m_spDefaultRenderTargetGroup = 0;
	m_dwVertexShaderVersion = 0;
	m_dwPixelShaderVersion = 0;

	m_pGrayScalePS = 0;
}

PgRenderer::~PgRenderer(void)
{
    ms_pkPgRenderer = NULL;

	m_spDefaultRenderTargetGroup = 0;

	m_spRenderer = 0;

	if(m_pGrayScalePS)
	{
		m_pGrayScalePS->Release();
	}
	m_pGrayScalePS = NULL;

	SAFE_DELETE_NI(m_pkPostProcessMan);
}

PgRenderer *PgRenderer::Create(NiRenderer *pkRenderer)
{
	PgRenderer *pkSelf = NiNew PgRenderer();
	PG_ASSERT_LOG(pkSelf);

	NiDX9Renderer* renderer = (NiDX9Renderer*)pkRenderer;
	if (renderer)
	{
		const D3DCAPS9* caps = renderer->GetDeviceCaps();
		if (caps)
		{
			pkSelf->SetVertexShaderVersion(renderer->GetVertexShaderVersion());
			pkSelf->SetPixelShaderVersion(renderer->GetPixelShaderVersion());
		}
	}

	pkSelf->m_spRenderer = pkRenderer;

	LPD3DXBUFFER pGrayScalePSBuffer = NULL;
	HRESULT	hr = D3DXAssembleShaderFromFile(_T("../Shader/DX9/GrayScale.psh"),NULL,NULL,0,&pGrayScalePSBuffer,NULL);
	if(hr == S_OK)
	{
		renderer->GetD3DDevice()->CreatePixelShader((DWORD*)pGrayScalePSBuffer->GetBufferPointer(),&pkSelf->m_pGrayScalePS);
	}

/*	LPD3DXBUFFER	pHighLightScalePSBuffer = NULL;

	hr = D3DXAssembleShaderFromFile(_T("../Shader/DX9/HighLightScale.psh"),NULL,NULL,0,&pHighLightScalePSBuffer,NULL);
	if(hr == S_OK)
	{
		pkDXRenderer->GetD3DDevice()->CreatePixelShader((DWORD*)pHighLightScalePSBuffer->GetBufferPointer(),&pkSelf->m_pHighlightScalePS);
	}*/

	return pkSelf;
}

bool PgRenderer::BeginFrame()
{
    PROFILE_FUNC();

    return m_spRenderer->BeginFrame();
}

bool PgRenderer::EndFrame()
{
    PROFILE_FUNC();

    return m_spRenderer->EndFrame();
}

bool PgRenderer::DisplayFrame()
{
    PROFILE_FUNC();

    return m_spRenderer->DisplayFrame();
}


void PgRenderer::SetCameraData( NiCamera* pkCamera )
{
	m_spRenderer->SetCameraData( pkCamera );
    NiAccumulator* pkAccumulator = m_spRenderer->GetSorter();
    if ( pkAccumulator )
	    pkAccumulator->StartAccumulating( pkCamera );

	m_pkCamera = pkCamera;
	m_kFrustumPlanes.Set( *m_pkCamera );
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Deprecated ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
NiVisibleArray* PgRenderer::GetVisibleArray_Deprecated()
{
    return &m_kVisibleScene_Deprecated;
}
void PgRenderer::CullingProcess_Deprecated(NiCamera *pkCamera, NiAVObject *pkObject, NiVisibleArray *pkVisibleArray,bool bReset)
{
    NewWare::Renderer::DrawUtils::GetPVSFromCullScene( pkCamera, m_kCuller_Deprecated, pkObject, *pkVisibleArray, bReset );
}
void PgRenderer::PartialRenderClick_Deprecated(NiAVObject *pkObject)
{
    if ( NewWare::Renderer::DrawUtils::PartialRenderClick(m_pkCamera, m_kCuller_Deprecated, pkObject, m_kVisibleScene_Deprecated, true) )
    {
        NiAccumulator* pkAccumulator = m_spRenderer->GetSorter();
        if ( pkAccumulator )
           pkAccumulator->StartAccumulating( m_pkCamera );
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Deprecated ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////



NiRenderTargetGroup *PgRenderer::GetDefaultRenderTargetGroup()
{
	if(m_spDefaultRenderTargetGroup != 0) return m_spDefaultRenderTargetGroup;

	return m_spRenderer->GetDefaultRenderTargetGroup();
}
void	PgRenderer::SetDefaultRenderTargetGroup(NiRenderTargetGroup *pkGroup)	//	pkGroup 을 0으로 설정할 경우, NiRenderer의 DefaultRenderTargetGroup 으로 설정된다. 
{
	m_spDefaultRenderTargetGroup = pkGroup;
}

bool PgRenderer::BeginUsingDefaultRenderTargetGroup(unsigned int uiClearMode)
{
	if(m_spDefaultRenderTargetGroup != 0)
		return m_spRenderer->BeginUsingRenderTargetGroup(m_spDefaultRenderTargetGroup,uiClearMode);

	return m_spRenderer->BeginUsingDefaultRenderTargetGroup(uiClearMode);
}

bool PgRenderer::BeginUsingRenderTargetGroup(NiRenderTargetGroup *pkTarget, unsigned int uiClearMode)
{
	return m_spRenderer->BeginUsingRenderTargetGroup(pkTarget, uiClearMode);
}

bool PgRenderer::EndUsingRenderTargetGroup()
{
	return m_spRenderer->EndUsingRenderTargetGroup();
}

bool PgRenderer::IsRenderTargetGroupActive()
{
	return m_spRenderer->IsRenderTargetGroupActive();
}

void PgRenderer::GetBackgroundColor(NiColorA& kColor)
{
	m_spRenderer->GetBackgroundColor(kColor);
}

void PgRenderer::SetBackgroundColor(const NiColor& kColor)
{
	m_spRenderer->SetBackgroundColor(kColor);
}

void PgRenderer::SetBackgroundColor(const NiColorA& kColor)
{
	m_spRenderer->SetBackgroundColor(kColor);
}

void PgRenderer::SetScreenSpaceCameraData(const NiRect<float>* pkPort)
{
	m_spRenderer->SetScreenSpaceCameraData(pkPort);
}

void	PgRenderer::EnableMipMap(NiAVObject *pkAVObject,bool bEnable)
{
	if(!pkAVObject)
	{
		return;
	}
	if(NiIsKindOf(NiNode, pkAVObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode, pkAVObject);
		if (pkNode == NULL)
			return;

		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject	*pkChild = NULL;
		for(int i=0;i<iChildArrayCount;i++)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				EnableMipMap(pkChild,bEnable);
			}
		}
	}
	else if(NiIsKindOf(NiGeometry, pkAVObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry, pkAVObject);
		if (pkGeom)
		{
			NiPropertyState	*pkProperties = pkGeom->GetPropertyState();
			if(!pkProperties) return;

			NiTexturingProperty	*pkTexturing = pkProperties->GetTexturing();
			if(!pkTexturing) return;

			NiTexturingProperty::NiMapArray const	&kMapArray = pkTexturing->GetMaps();
			int	iTotalMap = kMapArray.GetSize();

			for(int i=0;i<iTotalMap;i++)
			{
				NiTexturingProperty::Map *pkMap = kMapArray.GetAt(i);
				if(!pkMap) continue;

				if(bEnable)
				{
					pkMap->SetFilterMode(NiTexturingProperty::FILTER_TRILERP);
				}
				else
				{
					pkMap->SetFilterMode(NiTexturingProperty::FILTER_BILERP);
				}

			}
		}
	}
}
int		PgRenderer::CountLight(NiAVObject *pkAffectedObject)	//	pkAffectedObject 에 적용되고 있는 포인트 라이트 갯수를 리턴한다.
{
	int	iCount = 0;

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAffectedObject);
	if(!pkNode)
	{
		return	0;
	}


	NiDynamicEffectList	const &kList =pkNode->GetEffectList();
    NiTListIterator kPos = kList.GetHeadPos();
    while (kPos)
    {
        NiDynamicEffect* pkEffect = kList.GetNext(kPos);

		NiDynamicEffect::EffectType const kEffectType = pkEffect->GetEffectType();

		if(kEffectType == NiDynamicEffect::POINT_LIGHT ||
			kEffectType == NiDynamicEffect::DIR_LIGHT ||
			kEffectType == NiDynamicEffect::SPOT_LIGHT)
		{
			iCount++;
		}
    }

	return	iCount;
}

void	PgRenderer::PrecacheGeometry(NiAVObject *pkAVObject)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					PrecacheGeometry(pkChild);
				}
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer, 
			NiRenderer::GetRenderer());

		if(pkDXRenderer)
		{
			pkDXRenderer->PrecacheGeometry(pkGeom,0,0);
		}
	}
}
void	PgRenderer::RestartAnimation(NiAVObject *pkAVObject,float fCurrentAccumTime)
{
	if(!pkAVObject)
	{
		return;
	}
	NiTimeController	*pkController = pkAVObject->GetControllers();

	while(pkController)
	{
		pkController->SetPhase(-fCurrentAccumTime);
		pkController = pkController->GetNext();
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int iChildCount = pkNode->GetArrayCount();
		for(int i=0;i<iChildCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				RestartAnimation(pkChild,fCurrentAccumTime);
			}
		}
	}
	
}
void	PgRenderer::SetAlphaBlendMode(NiAVObject *pkAVObject,NiAlphaProperty::AlphaFunction kSrcBlend,NiAlphaProperty::AlphaFunction kDestBlend)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					SetAlphaBlendMode(pkChild,kSrcBlend,kDestBlend);
				}
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		if(pkGeom->GetPropertyState())
		{
			NiAlphaProperty	*pkAlpha = pkGeom->GetPropertyState()->GetAlpha();
			if(pkAlpha)
			{
				pkAlpha->SetSrcBlendMode(kSrcBlend);
				pkAlpha->SetDestBlendMode(kDestBlend);
			}
		}
	}
}
void	PgRenderer::SetZBufferWrite(NiAVObject *pkAVObject,bool bZBufferWrite)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					SetZBufferWrite(pkChild,bZBufferWrite);
				}
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		if(pkGeom->GetPropertyState())
		{
			NiZBufferProperty	*pkZBuffer = pkGeom->GetPropertyState()->GetZBuffer();
			if(!pkZBuffer)
			{
				pkZBuffer = NiNew NiZBufferProperty();
			}
			pkZBuffer->SetZBufferWrite(bZBufferWrite);
		}
	}
}
void	PgRenderer::CountSameTexture(NiAVObject *pkAVObject,stSameTextureCounter &kCounter)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					CountSameTexture(pkChild,kCounter);
				}
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		if(pkGeom->GetPropertyState())
		{
			NiTexturingProperty	*pTexturing = pkGeom->GetPropertyState()->GetTexturing();
			if(pTexturing && pTexturing->GetBaseTexture())
			{
				NiSourceTexture	*pkSourceTexture = NiDynamicCast(NiSourceTexture,pTexturing->GetBaseTexture());
				kCounter.AddTexture(std::string(pkSourceTexture->GetFilename()));
			}
		}
	}
}

void	PgRenderer::EnableAlphaBlending(NiAVObject *pkAVObject,bool bEnable)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					EnableAlphaBlending(pkChild,bEnable);
				}
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		if(pkGeom->GetPropertyState())
		{
			NiAlphaProperty	*pkAlpha = pkGeom->GetPropertyState()->GetAlpha();
			if(pkAlpha)
			{
				pkAlpha->SetAlphaBlending(bEnable);
			}
		}
	}
}
void	ChangeShaderEx(NiAVObject *pkAVObject,NiMaterial *pkSkinning,NiMaterial *pkSkinningGlow,NiMaterial *pkSkinningPL,NiMaterial *pkSkinningGlowPL)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					ChangeShaderEx(pkChild,pkSkinning,pkSkinningGlow,pkSkinningPL,pkSkinningGlowPL);
				}
			}
		}
	}

	NiTriBasedGeom	*pkGeom = NiDynamicCast(NiTriBasedGeom,pkAVObject);
	if(pkGeom)
	{
		if(pkGeom->GetSkinInstance())
		{
			bool	bGlow = false;
			bool	bProjectionLight = false;

			if(pkGeom->GetPropertyState())
			{
				NiTexturingProperty	*pkTexturing = pkGeom->GetPropertyState()->GetTexturing();
				if(pkTexturing)
				{
					if(pkTexturing->GetGlowMap() && pkTexturing->GetGlowMap()->GetEnable())
					{
						bGlow = true;
					}
				}	
			}

			if(pkGeom->GetEffectState())
			{
				NiDynamicEffectState	*pkState = pkGeom->GetEffectState();
				if(pkState->GetProjLightHeadPos() != NULL)
				{
					NiDynEffectStateIter	iter = pkState->GetProjLightHeadPos();
					NiTextureEffect	*pkEffect = pkState->GetNextProjLight(iter);
					if(pkEffect  && pkEffect->GetSwitch() == true)
					{
						bProjectionLight = true;
					}
				}
			}

			NiMaterial	*pkSelectedMaterial = pkSkinning;

			if(bGlow && !bProjectionLight)
			{
				pkSelectedMaterial = pkSkinningGlow;
			}
			else if(!bGlow && bProjectionLight)
			{
				pkSelectedMaterial = pkSkinningPL;
			}
			else if(bGlow && bProjectionLight)
			{
				pkSelectedMaterial = pkSkinningGlowPL;
			}

			pkGeom->ApplyAndSetActiveMaterial(pkSelectedMaterial);
			pkGeom->SetActiveMaterial(pkSelectedMaterial);
		}
	}
}
void	PgRenderer::ChangeShader(NiAVObject *pkAVObject,char const *strShaderName)
{
	NiShaderPtr spShader = NiShaderFactory::GetInstance()->RetrieveShader(strShaderName, NiShader::DEFAULT_IMPLEMENTATION, true);
	NiMaterialPtr spMaterialSkinning = NiSingleShaderMaterial::Create(spShader); 

	std::string	kShaderName = std::string(strShaderName)+"_Glow";
	spShader = NiShaderFactory::GetInstance()->RetrieveShader(kShaderName.c_str(), NiShader::DEFAULT_IMPLEMENTATION, true);
	NiMaterialPtr spMaterialSkinningGlow = NiSingleShaderMaterial::Create(spShader); 

	kShaderName = std::string(strShaderName)+"_ProjectionLight";
	spShader = NiShaderFactory::GetInstance()->RetrieveShader(strShaderName, NiShader::DEFAULT_IMPLEMENTATION, true);
	NiMaterialPtr spMaterialSkinningPL = NiSingleShaderMaterial::Create(spShader); 

	kShaderName = std::string(strShaderName)+"_GlowProjectionLight";
	spShader = NiShaderFactory::GetInstance()->RetrieveShader(strShaderName, NiShader::DEFAULT_IMPLEMENTATION, true);
	NiMaterialPtr spMaterialSkinningGlowPL = NiSingleShaderMaterial::Create(spShader); 

	ChangeShaderEx(pkAVObject,spMaterialSkinning,spMaterialSkinningGlow,spMaterialSkinningPL,spMaterialSkinningGlowPL);

    //NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkAVObject, false );
}

int	PgRenderer::CountNode(NiAVObject *pkAVObject)	///	pkAVObject 의 자식들의 갯수를 세어준다.
{
	int	iCount = 1;
	if(NiIsKindOf(NiNode,pkAVObject))
	{

		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					iCount += CountNode(pkChild);
				}
			}
		}
	}

	return	iCount;
}
//void PgRenderer::SetSelectiveUpdateFlags(NiAVObject *pkAVObject,bool bSelectiveUpdate,bool bSelectiveUpdateTransforms,bool bSelectiveUpdateProperty,bool bSelectiveUpdateRigid)
//{
//	return;
//
//	if(NiIsKindOf(NiNode,pkAVObject))
//	{
//		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
//		if(pkNode)
//		{
//			int iChildCount = pkNode->GetArrayCount();
//			for(int i=0;i<iChildCount;i++)
//			{
//				NiAVObject	*pkChild = pkNode->GetAt(i);
//				if(pkChild)
//				{
//					SetSelectiveUpdateFlags(pkChild,bSelectiveUpdate,bSelectiveUpdateTransforms,bSelectiveUpdateProperty,bSelectiveUpdateRigid);
//				}
//			}
//		}
//	}
//
//	pkAVObject->SetSelectiveUpdate(bSelectiveUpdate);
//	pkAVObject->SetSelectiveUpdateTransforms(bSelectiveUpdateTransforms);
//	pkAVObject->SetSelectiveUpdatePropertyControllers(bSelectiveUpdateProperty);
//	pkAVObject->SetSelectiveUpdateRigid(bSelectiveUpdateRigid);
//
//}
void	PgRenderer::DetachAllAffectedNode(NiAVObject *pkLightRoot)
{
	if(!pkLightRoot)
	{
		return;
	}

	NiNode *pkNode = NiDynamicCast(NiNode,pkLightRoot);
	if(pkNode)
	{
		int iChildCount = pkNode->GetArrayCount();
		for(int i=0;i<iChildCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				DetachAllAffectedNode(pkChild);
			}
		}

		return;
	}

	NiDynamicEffect	*pkDE = NiDynamicCast(NiDynamicEffect,pkLightRoot);
	if(pkDE)
	{
		pkDE->DetachAllAffectedNodes();
		pkDE->DetachAllUnaffectedNodes();
	}
}
void	PgRenderer::VerifyNodes(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}
	NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int iChildCount = pkNode->GetArrayCount();
		for(int i=0;i<iChildCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(((DWORD)pkChild)%2 != 0)
			{
				__asm	int	3;
			}

			if(pkChild)
			{
				VerifyNodes(pkChild);
			}
		}
	}
}
bool	PgRenderer::HasGeomMorphController(NiGeometry *pkGeom)
{
	if(!pkGeom)
	{
		return false;
	}
	NiTimeController	*pkController = pkGeom->GetControllers();
	while(pkController)
	{
		if(NiDynamicCast(NiGeomMorpherController,pkController))
		{
			return	true;
		}

		pkController=pkController->GetNext();
	}
	return	false;
}
NiPropertyState*	PgRenderer::CopyPropertyState(NiPropertyState *pkSource)
{
	assert(pkSource);
	if(!pkSource)
	{
		return	NULL;
	}

	NiProperty* apkProps[NiProperty::MAX_TYPES];
	pkSource->GetProperties(apkProps);

	NiPropertyState	*pkNewPropertyState = NiNew NiPropertyState();

	for(int i=0;i<NiProperty::MAX_TYPES;++i)
	{
		NiProperty	*pkSourceProperty = apkProps[i];
		if(!pkSourceProperty)
		{
			continue;
		}

		NiProperty	*pkNewProperty = NiDynamicCast(NiProperty,pkSourceProperty->Clone());
		pkNewPropertyState->SetProperty(pkNewProperty);
	}

	return	pkNewPropertyState;
}
bool	PgRenderer::HasTextureTransformController(NiGeometry *pkGeom)
{
 	NiPropertyList &kPropertyList = pkGeom->GetPropertyList();

	NiTListIterator kPos = kPropertyList.GetHeadPos();
    while (kPos)
    {
        NiProperty* pkProperty = kPropertyList.GetNext(kPos);
        if (pkProperty)
		{
			NiTimeController	*pkController = pkProperty->GetControllers();

			while(pkController)
			{
				if(NiDynamicCast(NiTextureTransformController,pkController))
				{
					return	true;
				}

				pkController = pkController->GetNext();
			}
		}
    }
	return	false;
}


void	PgRenderer::AddToVisibleArray(NiVisibleArray &rkVisibleArray,NiAVObject *pkAVObject)
{
	if(pkAVObject->GetAppCulled() )
	{
		return;
	}

	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					AddToVisibleArray(rkVisibleArray,pkChild);
				}
			}
		}
		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		rkVisibleArray.Add(*pkGeom);
	}
}
bool PgRenderer::HasTimeController(NiAVObject *pkAVObject)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{
		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					if(HasTimeController(pkChild)) return true;
				}
			}
		}
	}
	
	if(pkAVObject->GetControllers()) return true;

 	NiPropertyList &kPropertyList = pkAVObject->GetPropertyList();

	NiTListIterator kPos = kPropertyList.GetHeadPos();
    while (kPos)
    {
        NiProperty* pkProperty = kPropertyList.GetNext(kPos);
        if (pkProperty && pkProperty->GetControllers())
		{
			return	true;
		}
    }

	return false;
}

//void	PgRenderer::SetSelectiveUpdateFlags(NiAVObject *pkAVObject)
//{
//	if(NiIsKindOf(NiNode, pkAVObject))
//	{
//		NiNode	*pkNode = NiDynamicCast(NiNode, pkAVObject);
//		int	iChildArrayCount = pkNode->GetArrayCount();
//		NiAVObject	*pkChild = NULL;
//		for(int i=0;i<iChildArrayCount;i++)
//		{
//			pkChild = pkNode->GetAt(i);
//			if(pkChild)
//			{
//				SetSelectiveUpdateFlags(pkChild);
//			}
//		}
//	}
//
//	bool	bSelectiveUpdate,bRigid;
//
//	pkAVObject->SetSelectiveUpdateFlags(bSelectiveUpdate,false,bRigid);
//}

void	PgRenderer::ReloadTexture(NiAVObject *pkAVObject,int iMipMapSkipLevel)
{
	if(!pkAVObject)
	{
		return;
	}
	if(NiIsKindOf(NiNode, pkAVObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode, pkAVObject);
		if (pkNode == NULL)
			return;

		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject	*pkChild = NULL;
		for(int i=0;i<iChildArrayCount;i++)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				ReloadTexture(pkChild,iMipMapSkipLevel);
			}
		}
	}
	else if(NiIsKindOf(NiGeometry, pkAVObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry, pkAVObject);
		if (pkGeom)
		{
			NiPropertyState	*pkProperties = pkGeom->GetPropertyState();
			if(!pkProperties) return;

			NiTexturingProperty	*pkTexturing = pkProperties->GetTexturing();
			if(!pkTexturing) return;

			NiTexturingProperty::NiMapArray const	&kMapArray = pkTexturing->GetMaps();
			int	iTotalMap = kMapArray.GetSize();

			for(int i=0;i<iTotalMap;i++)
			{
				NiTexturingProperty::Map *pkMap = kMapArray.GetAt(i);
				if(!pkMap) continue;

				NiTexture	*pkTexture = pkMap->GetTexture();
				if(!pkTexture) continue;

				NiSourceTexture	*pkSourceTexture = NiDynamicCast(NiSourceTexture,pkTexture);
				if(!pkSourceTexture) continue;

				if(pkSourceTexture->GetMipMapSkipLevel() == iMipMapSkipLevel) continue;

				if(pkSourceTexture->GetWidth()<=256 || pkSourceTexture->GetHeight()<=256) continue;

				NiTexture::RendererData *pkRendererData = pkSourceTexture->GetRendererData();
				if(pkRendererData)
				{
					NiDelete pkRendererData;
				}
				pkSourceTexture->SetRendererData(NULL);

				pkSourceTexture->Reload(iMipMapSkipLevel);
			}
		}
	}
}

void	PgRenderer::SetWorldQuality(NiAVObject *pkObject, bool bFullOption)
{
	if(!g_pkWorld)
	{
		return;
	}
	if(NULL == pkObject)
	{
		return;
	}

	if(true == NiIsKindOf(NiNode, pkObject))
	{
		NiObjectList kGeomList;
		g_pkWorld->GetAllGeometries(static_cast<NiNode*>(pkObject), kGeomList);
		NiTListIterator kGeomIter = kGeomList.GetHeadPos();
		while(NULL != kGeomIter)
		{
			NiGeometry* pkGeom = NiDynamicCast(NiGeometry, kGeomList.GetNext(kGeomIter));
			if(NULL == pkGeom)
			{
				return;
			}
			NiPropertyList& kPropList = pkGeom->GetPropertyList();
			NiTListIterator kPropIter = kPropList.GetHeadPos();

			while(NULL != kPropIter)
			{
				NiProperty* pkProperty = kPropList.GetNext(kPropIter);
				if(NULL == pkProperty)
				{
					return;
				}
				if(NiProperty::TEXTURING == pkProperty->Type() )
				{
					NiTexturingProperty* pkTexProp = static_cast<NiTexturingProperty*>(pkProperty);
					if( NULL != pkTexProp->GetBumpMap())
					{
						pkTexProp->GetBumpMap()->SetEnable(bFullOption);
					}
					if(	NULL != pkTexProp->GetGlossMap())
					{
						pkTexProp->GetGlossMap()->SetEnable(bFullOption);
					}
				}
			}
		}
	}
}

void	PgRenderer::EnableGlowMap(NiAVObject* pkAVObject,bool bEnable)
{
	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iTotal = pkNode->GetArrayCount();
		for(int i=0;i<iTotal;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				EnableGlowMap(pkChild,bEnable);
			}
		}
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(!pkGeom) return;

	NiPropertyState	*pkProp = pkGeom->GetPropertyState();
	if(!pkProp)	return;

	NiTexturingProperty	*pkTexturing = pkProp->GetTexturing();
	if(!pkTexturing)	return;

	NiTexturingProperty::Map *pkGlowMap = pkTexturing->GetGlowMap();
	if(!pkGlowMap)	return;

	pkGlowMap->SetEnable(bEnable);

    NewWare::Renderer::SetRenderStateTagExtraDataNumber( *pkGeom, 
                            NewWare::Renderer::Kernel::RenderStateTagExtraData::RESET_RENDERSTATE );
}
void	PgRenderer::RunUpParticleSystem(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}

	float fMinKeyTime = NiParticleSystem::GetMinBeginKeyTime(pkAVObject);
	for (float fTime = fMinKeyTime; fTime < 0; fTime += 0.0167f)
	{
		pkAVObject->Update(fTime);
	}
}
