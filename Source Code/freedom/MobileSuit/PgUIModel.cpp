#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgUIModel.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgAMPool.h"
#include "PgWorkerThread.h"
#include "PgUIScene.h"
#include "PgParticle.h"
#include "PgNifMan.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgAction.H"
#include "PgActorPet.h"
#include "Variant/PgStringUtil.h"

PgUIModel::PgUIModel(POINT2 const& ptWndSize) :
	m_spCamera(0),
	m_pkModelScreenTexture(0),
	m_spRenderTargetGroup(0),
	m_spRenderTexture(0),
	m_kWndSize(0,0),
	m_fMinCameraZoom(0.0f),
	m_fMaxCameraZoom(0.0f),
	m_fCurCameraZoom(0.0f),
	m_bAutoRotateCamera(false),
	m_bEnableWorldSpotLight(false),
	m_pkRendererData(NULL)
{
	m_kAutoDegreeToAxis = NiPoint3::ZERO;
	m_kTotalDegreeToAxis = NiPoint3::ZERO;
	m_kLimitDegreeToAxis = NiPoint3::ZERO;
	m_kWndSize = ptWndSize;
	m_bEnableUpdate = true;
}

PgUIModel::~PgUIModel()
{
	Terminate();
}

PgUIModel* PgUIModel::Create(POINT2 const& ptWndSize, bool bUpdate, bool bOrtho)
{
	PgUIModel* pkThis = new PgUIModel(ptWndSize);
	if(!pkThis)
	{
		return NULL;
	}

	if(!pkThis->Initialize(ptWndSize, bOrtho))
	{
		return NULL;
	}

	pkThis->SetEnableUpdate(bUpdate);

	return pkThis;
}

void PgUIModel::Terminate()
{
	m_spCamera = NULL;

	m_spRenderTargetGroup = NULL;
	m_spRenderTexture = NULL;
	m_pkRendererData = NULL;
	
	SAFE_DELETE_NI(m_pkModelScreenTexture);

	Clear();
}

void PgUIModel::Clear()
{
	ContActorItor itor = m_kActorContainer.begin();
	while(m_kActorContainer.end() != itor)
	{
		(*itor).second = NULL;
		itor = m_kActorContainer.erase(itor);
	}

	ContNIFItor itor2 = m_kNIFContainer.begin();
	while(m_kNIFContainer.end() != itor2)
	{
		(*itor2).second = NULL;
		itor2 = m_kNIFContainer.erase(itor2);
	}

	ContPgActor::iterator itor3 = m_kPgActorContainer.begin();
	while(m_kPgActorContainer.end() != itor3)
	{
		if(g_pkWorld)
		{
			g_pkWorld->RemoveObjectOnNextUpdate((*itor3).second->m_spkActor->GetGuid());
		}

		(*itor3).second->m_spkActor = NULL;
		SAFE_DELETE((*itor3).second);

		itor3 = m_kPgActorContainer.erase(itor3);
	}

	while( !m_kPgPetContainer.empty() )
	{
		ContPgPet::iterator itor4 = m_kPgPetContainer.begin();
		if( m_kPgPetContainer.end() != itor4 )
		{
			DeletePgPet(itor4->first);
		}
	}
}

bool PgUIModel::Initialize(POINT2 const& ptWndSize, bool bOrtho)
{
	//카메라 세팅
	m_spCamera = NiNew NiCamera;
	NiFrustum kFrustum = m_spCamera->GetViewFrustum();
	kFrustum.m_fNear = 0.0f;
	kFrustum.m_fFar = 1500.0f;
	if( bOrtho )
	{
		kFrustum.m_bOrtho = true;
		kFrustum.m_fLeft = -(ptWndSize.x * 0.5f);
		kFrustum.m_fRight = (ptWndSize.x * 0.5f);
		kFrustum.m_fTop = (ptWndSize.y * 0.5f);
		kFrustum.m_fBottom = -(ptWndSize.y * 0.5f);
	}
	m_spCamera->SetViewFrustum(kFrustum);
	m_spCamera->Update(0.0f);
	
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	if (NULL == pkRenderer)
	{
		return false;
	}
	
	NiTexture::FormatPrefs kFormat;
	kFormat.m_ePixelLayout= NiTexture::FormatPrefs::TRUE_COLOR_32;
	kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;

	m_spRenderTexture = NiRenderedTexture::Create(m_kWndSize.x, m_kWndSize.y, pkRenderer, kFormat);
	if(!m_spRenderTexture)
	{
		return	false;
	}
	m_pkRendererData = m_spRenderTexture->GetRendererData();
	m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderTexture->GetBuffer(), pkRenderer, false, true);

	m_pkModelScreenTexture = NiNew NiScreenTexture(m_spRenderTexture);
	m_pkModelScreenTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	return true;
}

int PgUIModel::GetAnimationSeqID(std::string const& strID, std::string const& StrAnimationName)
{
	TiXmlDocument* pkXmlDoc = PgXmlLoader::GetXmlDocumentByID(strID.c_str());
	if (NULL == pkXmlDoc)
	{
		return 0 ;
	}
	TiXmlNode *pkActorNode = pkXmlDoc->FirstChild("ACTOR");
	if(!pkActorNode)
	{
		return 0;
	}
	TiXmlNode* pkWorldObjectNode = pkActorNode->FirstChild("WORLDOBJECT");
	if(!pkWorldObjectNode)
	{
		return 0;
	}
	TiXmlNode *pkSlotNode = pkWorldObjectNode->FirstChild("SLOT");
	if( !pkSlotNode ) 
	{
		return 0;
	}
	TiXmlNode* pkItemNode = pkSlotNode->FirstChild("ITEM");
	if(!pkItemNode)
	{
		return 0;
	}
	TiXmlElement* pkElement = (TiXmlElement *)pkItemNode;
	while( pkElement )
	{		
		int const iType = pkElement->Type();
		if(TiXmlNode::ELEMENT ==iType)
		{
			char const* pcName = pkElement->Attribute("NAME");
			char const* pcAnimation = pkElement->Attribute("ANIMATION");
			if( !::strcmp(pcName, StrAnimationName.c_str() ) )
			{
				int const iSeqID = PgStringUtil::SafeAtoi(pcAnimation);
				return iSeqID;
			}
		}
		pkItemNode = pkItemNode->NextSibling();
		pkElement = (TiXmlElement*)pkItemNode;
	}

	return 0;
}

bool PgUIModel::AddActorByID(std::string const& strActorName, std::string const& strID, bool const bEnableGlowMap)
{
	TiXmlDocument* pkXmlDoc = PgXmlLoader::GetXmlDocumentByID(strID.c_str());
	if (NULL == pkXmlDoc)
	{
		return false ;
	}

	char const* kfmPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "KFMPATH");
	if (kfmPath)
	{
		AddActor(strActorName, g_kAMPool.LoadActorManager(kfmPath, PgIXmlObject::ID_NONE, false), false, true);
	}

	TiXmlNode* pkDoc = pkXmlDoc->FirstChild();
	VariTextureContainer kTextureContainer;	

	while(pkDoc)
	{
		int const iType = pkDoc->Type();
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlNode* pkChildNode = pkDoc->FirstChild();
				while(pkChildNode)
				{
					TiXmlElement* pkElement = (TiXmlElement *)pkChildNode;
					char const* pcTagName = pkElement->Value();
					if(strcmp(pcTagName, "TEXTURE") == 0)
					{
						TiXmlNode* pChildNode = pkElement->FirstChild();

						while(pChildNode)
						{
							TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
							while(pkAttr)
							{
								char const* pcAttrName = pkAttr->Name();
								char const* pcAttrValue = pkAttr->Value();

								if(strcmp(pcAttrName, "SRC") == 0)
								{
									char const* szChangeTextureName = pkElement->GetText();
									kTextureContainer.insert(std::make_pair(pcAttrValue, szChangeTextureName));
								}
								pkAttr = pkAttr->Next();
							}
							pChildNode = pChildNode->NextSibling();
							pkElement = (TiXmlElement*)pChildNode;
						}
					}
					pkChildNode = pkChildNode->NextSibling();
				}
			}
		}
		pkDoc = pkDoc->NextSibling();
	}
	
	NiActorManager* pkAM = GetActor(strActorName);
	if(pkAM && pkAM->GetNIFRoot())
	{
		PgRenderer::EnableGlowMap(pkAM->GetNIFRoot(), bEnableGlowMap);

		if(!kTextureContainer.empty())
		{
			ChangeTexture((NiNode*)pkAM->GetNIFRoot(), kTextureContainer);
		}
		return true;
	}
	else
	{
		NILOG(PGLOG_ERROR, "[%s] Nif Add Faild Name :%s ID : %s \n", __FUNCTIONW__, strActorName.c_str(), strID.c_str());
		return false;
	}	
}

bool PgUIModel::AddActor(std::string const& strActorName, NiActorManagerPtr spNiActor, bool const bClone, bool const bCameraReset, bool const bEnableGlowMap)
{
	if(strActorName.empty() || !spNiActor)
	{
		return false;
	}

	ContActorItor itor = m_kActorContainer.find(strActorName);
	NiActorManagerPtr pkActor = NULL;
	if(bClone)
	{
		pkActor = spNiActor->Clone();
	}
	else
	{
		pkActor = spNiActor;
	}

	if(pkActor && pkActor->GetNIFRoot())
	{
		PgRenderer::EnableGlowMap(pkActor->GetNIFRoot(), bEnableGlowMap);

		UpdateModel(pkActor->GetNIFRoot());
		pkActor->Update(0.0f);
		NiActorManager::SequenceID kSeqID = 1000001;
		pkActor->SetTargetAnimation(kSeqID);

		if(m_kActorContainer.end() == itor)
		{
			std::pair<ContActor::iterator, bool> kRet = m_kActorContainer.insert(std::make_pair(strActorName, pkActor));
			std::pair<ContActorSeqID::iterator, bool> kRet2 =  m_kActorSeqContainer.insert(std::make_pair(strActorName, kSeqID));		
		}
		else
		{
			(*itor).second = pkActor;

			ContActorSeqIDItor itor2 = m_kActorSeqContainer.find(strActorName);
			(*itor2).second = kSeqID;
		}

		return true;
	}
	else
	{
		NILOG(PGLOG_ERROR, "[%s] NiActorManager Add Faild %s\n", __FUNCTIONW__, strActorName.c_str());
		return false;
	}	
}
bool PgUIModel::AddNIF(std::string const& strNIFName, NiNodePtr pNiNode, bool const bClone, bool const bCameraReset, bool const bEnableGlowMap)
{
	if(strNIFName.empty() || !pNiNode)
	{
		return false;
	}

	ContNIFItor itor = m_kNIFContainer.find(strNIFName);

	NiNodePtr pkNode = NULL;
	if(bClone)
	{
		pkNode = static_cast<NiNode*>(pNiNode->Clone());
	}
	else
	{
		pkNode = pNiNode;
	}

	if(pkNode)
	{
		PgRenderer::EnableGlowMap(pkNode, bEnableGlowMap);	
		UpdateModel(pkNode, bCameraReset);
		NiTimeController::StartAnimations(pkNode, 0.0f);

		if(m_kNIFContainer.end() == itor)
		{
			std::pair<ContNIF::iterator, bool> kRet = m_kNIFContainer.insert(std::make_pair(strNIFName, pkNode));
		}
		else
		{
			(*itor).second = pkNode;
		}

		return true;
	}
	else
	{
		NILOG(PGLOG_ERROR, "[%s] Nif Add Faild %s\n", __FUNCTIONW__, strNIFName.c_str());
		return false;
	}
}

void PgUIModel::ResetNIFAnimation(std::string const& strNIFName)
{
	ContNIFItor itor = m_kNIFContainer.find(strNIFName);
	if(m_kNIFContainer.end() != itor)
	{
		NiTimeController::StartAnimations((*itor).second, 0.0f);
	}
}

void PgUIModel::SetEnableUpdate(bool const bEnableUpdate)
{
	m_bEnableUpdate = bEnableUpdate;
}

bool PgUIModel::AddNodeAlphaProperty(NiNode* pNiNode)
{
	unsigned int uiArrayCount = pNiNode->GetArrayCount();
	for (unsigned int i = 0; i < uiArrayCount; ++i)
	{
		NiAVObject* pkChild = pNiNode->GetAt(i);
		if(!pkChild)
		{
			continue;
		}
		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiAlphaProperty* pkProperty = static_cast<NiAlphaProperty*>(pkChild->GetProperty(NiAlphaProperty::GetType()));

			if(!pkProperty)
			{
				pkProperty = NiNew NiAlphaProperty;
				pkProperty->SetAlphaBlending(true);
				pkProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
				pkProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
				pkProperty->SetAlphaTesting(true);
				pkProperty->SetTestRef(10);
				pkProperty->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);

				pkChild->AttachProperty(pkProperty);
			}
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			AddNodeAlphaProperty((NiNode*)pkChild);
		}
	}

	return true;
}



bool PgUIModel::DeleteActor(std::string const& strActorName)
{
	ContActorItor itor = m_kActorContainer.find(strActorName);
	if(m_kActorContainer.end() != itor)
	{
		m_kActorContainer.erase(itor);
	}
	
	return true;
}
bool PgUIModel::DeleteNIF(std::string const& strNIFName)
{
	ContNIFItor itor = m_kNIFContainer.find(strNIFName);
	if(m_kNIFContainer.end() != itor)
	{
		m_kNIFContainer.erase(itor);
	}
	
	return true;
}

NiActorManager* PgUIModel::GetActor(std::string const& strActorName) const
{
	ContActor::const_iterator itor = m_kActorContainer.find(strActorName);
	if(m_kActorContainer.end() != itor)
	{
		return (*itor).second;
	}

	return NULL;
}
NiNode* PgUIModel::GetNIF(std::string const& strNIFName) const
{
	ContNIF::const_iterator itor = m_kNIFContainer.find(strNIFName);
	if(m_kNIFContainer.end() != itor)
	{
		return (*itor).second;
	}

	return NULL;
}
void PgUIModel::InvalidateRenderTarget()
{
	if(!CheckNeedInvlidateRenderTarget())
	{
		return;
	}

	NiScreenTexture::ScreenRect const &kRect = m_pkModelScreenTexture->GetScreenRect(0);

	RenderFrame(NiRenderer::GetRenderer(),POINT2(kRect.m_sPixLeft,kRect.m_sPixTop));
}
bool	PgUIModel::CheckNeedInvlidateRenderTarget()
{
	if(!m_pkModelScreenTexture || m_pkModelScreenTexture->GetNumScreenRects() == 0)
	{
		return	false;
	}

	NiTexture	*pkTexture = m_pkModelScreenTexture->GetTexture();
	if(!pkTexture)
	{
		return	false;
	}

	if(m_pkRendererData != pkTexture->GetRendererData())
	{
		m_pkRendererData = pkTexture->GetRendererData();
		return	true;
	}

	return false;
}
void PgUIModel::RenderFrame(NiRenderer* pkRenderer, POINT2 const& ptWndPos)
{
	if(!pkRenderer || !m_pkModelScreenTexture)
	{
		return;	
	}

	bool bWorldSpotLight = false;
	// 월드가 있고 스팟 라이트가 켜져 있을 경우
	if(!m_bEnableWorldSpotLight && g_pkWorld && g_pkWorld->GetSpotLightOn())
	{	
		bWorldSpotLight = true;
		g_pkWorld->TurnOnSpotLight(false);
	}

	float uiScreenWidth = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	float uiScreenHeight = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	m_pkModelScreenTexture->RemoveAllScreenRects();
	m_pkModelScreenTexture->AddNewScreenRect((short)ptWndPos.y, (short)ptWndPos.x, (unsigned short)m_kWndSize.x, (unsigned short)m_kWndSize.y, 0, 0);
	m_pkModelScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

	bool bInsideFrame = pkRenderer->GetInsideFrameState();

	if (!bInsideFrame)
	{
		pkRenderer->BeginFrame();
	}

	NiRenderTargetGroup *pkOldTarget = NULL;
	if(pkRenderer->IsRenderTargetGroupActive())
	{
		pkOldTarget = (NiRenderTargetGroup *) pkRenderer->GetCurrentRenderTargetGroup();
		pkRenderer->EndUsingRenderTargetGroup();
	}

	pkRenderer->SetBackgroundColor(DEF_BG_COLORA);
	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_ALL);	

	for(ContActorItor itor = m_kActorContainer.begin(); itor != m_kActorContainer.end(); ++itor)
	{
		ContNameItor itorAm = std::find(m_kDisableUpdateActorManagerContainer.begin(), m_kDisableUpdateActorManagerContainer.end(), (*itor).first);
		if(itorAm == m_kDisableUpdateActorManagerContainer.end())
		{
			NiVisibleArray kArray;
			NiCullingProcess kCuller(&kArray);
			NiDrawScene(m_spCamera, (*itor).second->GetNIFRoot(), kCuller);
		}
	}

	for(ContNIFItor itor2 = m_kNIFContainer.begin(); itor2 != m_kNIFContainer.end(); ++itor2)
	{
		ContNameItor itorNif = std::find(m_kDisableUpdateNIFContainer.begin(), m_kDisableUpdateNIFContainer.end(), (*itor2).first);
		if(itorNif == m_kDisableUpdateNIFContainer.end())
		{		
			NiVisibleArray kArray;
			NiCullingProcess kCuller(&kArray);
			NiDrawScene(m_spCamera, (*itor2).second, kCuller);
		}
	}

	for(ContPgActor::iterator itor3 = m_kPgActorContainer.begin(); itor3 != m_kPgActorContainer.end(); ++itor3)
	{
		(*itor3).second->m_spkActor->SetAppCulled(false);
		(*itor3).second->m_spkActor->SetHide(false);
		NiVisibleArray kArray;
		NiCullingProcess kCuller(&kArray);
		NiDrawScene(m_spCamera, (*itor3).second->m_spkActor, kCuller);
		(*itor3).second->m_spkActor->SetHide(true);
		(*itor3).second->m_spkActor->SetAppCulled(true);
	}

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	for(ContPgPet::iterator itor4 = m_kPgPetContainer.begin(); itor4 != m_kPgPetContainer.end(); ++itor4)
	{
		PgActorPet* pkPet = (*itor4).second->m_spkPet;
		(*itor4).second->m_spkPet->SetAppCulled(false);
		(*itor4).second->m_spkPet->SetHide(false);
		NiVisibleArray kArray;
		NiCullingProcess kCuller(&kArray);
		NiDrawScene(m_spCamera, (*itor4).second->m_spkPet, kCuller);

		NiVisibleArray kArrayP;
		(*itor4).second->m_spkPet->CullingProcessParticle(m_spCamera, &kArrayP);
		NiDrawVisibleArray(m_spCamera, kArrayP);

		pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_ZENABLE,false);
		(*itor4).second->m_spkPet->DrawNoZTest(PgRenderer::GetPgRenderer(), m_spCamera, g_pkApp->GetFrameCount());
		pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_ZENABLE,true);
		
		
		(*itor4).second->m_spkPet->SetHide(true);
		(*itor4).second->m_spkPet->SetAppCulled(true);
	}

	pkRenderer->EndUsingRenderTargetGroup();
	pkRenderer->SetBackgroundColor(DEF_BG_COLOR);
	
	if(pkOldTarget)
	{
		pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
	}

	if (!bInsideFrame)
	{
		pkRenderer->EndFrame();
		pkRenderer->DisplayFrame();
	}

	if(!m_bEnableWorldSpotLight && bWorldSpotLight && g_pkWorld)
	{
		g_pkWorld->TurnOnSpotLight(true);
	}
}

NiCamera* PgUIModel::GetCamera()
{
	return m_spCamera;
}

bool PgUIModel::Update(float fAccumTime, float fFrameTime)
{
	if(m_bEnableUpdate)
	{
		if(m_bAutoRotateCamera)
		{
			WorldRotate(m_kAutoDegreeToAxis.x, m_kAutoDegreeToAxis.y, m_kAutoDegreeToAxis.z);
		}

		for(ContActorItor itor = m_kActorContainer.begin(); itor != m_kActorContainer.end(); ++itor)
		{
			ContNameItor itorAm = std::find(m_kDisableUpdateActorManagerContainer.begin(), m_kDisableUpdateActorManagerContainer.end(), (*itor).first);
			if(itorAm == m_kDisableUpdateActorManagerContainer.end())
			{
				(*itor).second->GetNIFRoot()->Update(fAccumTime);
				(*itor).second->Update(fAccumTime);

			}
		}

		for(ContNIFItor itor2 = m_kNIFContainer.begin(); itor2 != m_kNIFContainer.end(); ++itor2)
		{
			ContNameItor itorNif = std::find(m_kDisableUpdateNIFContainer.begin(), m_kDisableUpdateNIFContainer.end(), (*itor2).first);
			if(itorNif == m_kDisableUpdateNIFContainer.end())
			{
				(*itor2).second->Update(fAccumTime);
			}
		}

		for(ContPgActor::iterator itor3 = m_kPgActorContainer.begin(); itor3 != m_kPgActorContainer.end(); ++itor3)
		{
			//g_pkWorld 에 붙어 있으므로 따로 업데이트 하지 않아도 된다.
			if((*itor3).second->m_bUpdate)
			{
				if((*itor3).second->m_spkActor)
				{
					if(g_pkWorld)
					{
						(*itor3).second->m_spkActor->NiNode::Update(g_pkWorld->GetAccumTime());
						(*itor3).second->m_spkActor->Update(g_pkWorld->GetAccumTime(), fFrameTime);
					}
					
				}
			}

		}

		for(ContPgPet::iterator itor4 = m_kPgPetContainer.begin(); itor4 != m_kPgPetContainer.end(); ++itor4)
		{
			//g_pkWorld 에 붙어 있으므로 따로 업데이트 하지 않아도 된다.
			if((*itor4).second->m_bUpdate)
			{
				if((*itor4).second->m_spkPet)
				{
					if(g_pkWorld)
					{
						(*itor4).second->m_spkPet->NiNode::Update(g_pkWorld->GetAccumTime());
						(*itor4).second->m_spkPet->Update(g_pkWorld->GetAccumTime(), fFrameTime);
					}
					
				}
			}
		}
	}

	return true;
}

void PgUIModel::Draw(PgRenderer* pkRenderer)
{
	if(pkRenderer && m_pkModelScreenTexture)
	{
		InvalidateRenderTarget();

		PgUIScene::Render_UIObject(pkRenderer, m_pkModelScreenTexture);
	}
}

bool PgUIModel::SetTargetAnimation(std::string const& strActorName, int const iID)
{
	ContActorItor itor = m_kActorContainer.find(strActorName);
	ContActorSeqIDItor itor2 = m_kActorSeqContainer.find(strActorName);

	if(	m_kActorContainer.end() != itor &&
		m_kActorSeqContainer.end() != itor2)
	{
		(*itor).second->Reset();
		(*itor).second->Update(0);
		(*itor).second->SetTargetAnimation(iID);

		(*itor2).second = iID;

		return true;
	}

	return false;
}

bool PgUIModel::IsAnimationDone(std::string const& strActorName)
{
	ContActorItor itor = m_kActorContainer.find(strActorName);
	ContActorSeqIDItor itor2 = m_kActorSeqContainer.find(strActorName);

	if(m_kActorContainer.end() != itor)
	{
		NiControllerSequence *pkSequence = (*itor).second->GetSequence((*itor2).second);

		if(!pkSequence || pkSequence->GetCycleType() == NiTimeController::LOOP)
		{
			return false;
		}

		float fTime = (*itor).second->GetNextEventTime(NiActorManager::END_OF_SEQUENCE, (*itor2).second);

		if((*itor).second->GetCurAnimation() == (*itor2).second && fTime == NiActorManager::INVALID_TIME)
		{
			return true;
		}
	}

	return false;
}

void PgUIModel::CameraReset(NiAVObject* pkModelRoot, SCameraResetInfo const& kResetInfo, float const fPerspective)
{
	if( !pkModelRoot || !m_spCamera )	
	{
		return;
	}

	m_kCameraResetInfo = kResetInfo;

	NiBound kBound;
	GetWorldBounds(pkModelRoot, kBound);
	float fRadius = kBound.GetRadius() + kResetInfo.fRadius; //xml에서 얻어온 증가값을 더해준다 (xml상 조절 가능)
	NiPoint3 kCenter = kBound.GetCenter();
	if(m_spCamera)
	{
		m_kCameraResetInfo.fRadius = fRadius;
		if( CalcOrthogonalFrustum(fRadius, m_kCameraResetInfo.fXCenter, m_kCameraResetInfo.fYCenter) )
		{
			NiPoint3 kTranslate(0.0f, -(3.0f * fRadius), 0.0f);
			switch( m_kCameraResetInfo.eHeightSet )
			{
			case SCameraResetInfo::EHS_FIX_MODEL_VALUE:
				{
					NiPoint3 kObjectPos = pkModelRoot->GetWorldTranslate();
					kTranslate.z = kObjectPos.z;
				}break;
			case SCameraResetInfo::EHS_USE_RADIAN_VALUE:
				{
					kTranslate.z = fRadius;
				}break;
			default:
				break;
			}
			
			m_spCamera->SetTranslate(kTranslate);
			m_spCamera->Update(0.0f);
		}

		if( CalcPerspectiveFrustum(pkModelRoot, m_kCameraResetInfo.fXCenter, m_kCameraResetInfo.fYCenter, fPerspective) )
		{
			NiPoint3 kTranslate(0.0f, 0.0f, 0.0f);

			m_spCamera->SetTranslate(NiPoint3(kCenter.x, kCenter.y - 2.0f * fRadius, kCenter.z));
			m_spCamera->Update(0.0f);
		}

		m_spCamera->LookAtWorldPoint(pkModelRoot->GetWorldTranslate(), NiPoint3::UNIT_Z);
		m_spCamera->Update(0.0f);
	}
	m_kTotalDegreeToAxis = NiPoint3::ZERO;
}

void PgUIModel::CameraReset(NiAVObject* pkModelRoot, bool const bUseHeightRadian, float const fXCenter, float const fYCenter, float const fPerspective)
{
	if( !pkModelRoot || !m_spCamera )	
	{
		return;
	}

	NiBound kBound;
	GetWorldBounds(pkModelRoot, kBound);
	float fRadius = kBound.GetRadius();
	NiPoint3 kCenter = kBound.GetCenter();
	if(m_spCamera)
	{
		if( CalcOrthogonalFrustum(fRadius, fXCenter, fYCenter) )
		{
			m_spCamera->SetTranslate(NiPoint3(0, -(2.0f * fRadius), (bUseHeightRadian)?(fRadius):(0.0f)));
			m_spCamera->Update(0.0f);
		}

		if( CalcPerspectiveFrustum(pkModelRoot, fXCenter, fYCenter, fPerspective) )
		{
			m_spCamera->SetTranslate(NiPoint3(kCenter.x, kCenter.y - 2.0f * fRadius, kCenter.z));
			m_spCamera->Update(0.0f);
		}

		m_spCamera->LookAtWorldPoint(pkModelRoot->GetWorldTranslate(), NiPoint3::UNIT_Z);
		m_spCamera->Update(0.0f);
	}
	m_kTotalDegreeToAxis = NiPoint3::ZERO;
}

void PgUIModel::CameraReset(NiAVObject* pkModelRoot, NiPoint3 const& kTranslate, NiPoint3 const& kAddLook)
{
	if( !pkModelRoot )
	{
		return;
	}

	NiBound kBound;
	GetWorldBounds(pkModelRoot, kBound);
	float fRadius = kBound.GetRadius();
	if(m_spCamera)
	{
		m_spCamera->SetTranslate(NiPoint3(fRadius * kTranslate.x, fRadius * kTranslate.y, fRadius * kTranslate.z));
		m_spCamera->Update(0.0f);

		NiPoint3 kLookPt = pkModelRoot->GetWorldTranslate() + kAddLook;
		m_spCamera->LookAtWorldPoint(kLookPt, NiPoint3::UNIT_Z);
		m_spCamera->Update(0.0f);
	}
	m_kTotalDegreeToAxis = NiPoint3::ZERO;
}

void PgUIModel::CameraZoom(float const fZoomDistance)
{
	m_fCurCameraZoom += fZoomDistance;

	m_fCurCameraZoom = NiClamp(m_fCurCameraZoom, m_fMinCameraZoom, m_fMaxCameraZoom);

	if(m_fCurCameraZoom == m_fMinCameraZoom || m_fCurCameraZoom == m_fMaxCameraZoom)
		return;

	if(m_spCamera)
	{
		NiPoint3 pos = m_spCamera->GetWorldTranslate();
		NiPoint3 dir = m_spCamera->GetWorldDirection();

		pos.x += dir.x * fZoomDistance;
		pos.y += dir.y * fZoomDistance;
		pos.z += dir.z * fZoomDistance;

		m_spCamera->SetTranslate(pos);
		m_spCamera->Update(0.0f);
	}
}

void PgUIModel::CameraZoomSubstitution(float const fZoomDistance)
{
	m_fCurCameraZoom = fZoomDistance;

	m_fCurCameraZoom = NiClamp(m_fCurCameraZoom, m_fMinCameraZoom, m_fMaxCameraZoom);
	if(m_spCamera)
	{
		NiPoint3 pos = m_spCamera->GetWorldTranslate();
		NiPoint3 dir = m_spCamera->GetWorldDirection();

		pos.x += dir.x * fZoomDistance;
		pos.y += dir.y * fZoomDistance;
		pos.z += dir.z * fZoomDistance;

		m_spCamera->SetTranslate(pos);
		m_spCamera->Update(0.0f);
	}
}

void PgUIModel::SetCameraZoomMinMax(float const fMin, float const fMax)
{
	m_fMinCameraZoom = fMin;
	m_fMaxCameraZoom = fMax;
}

void PgUIModel::LocalRotate(NiAVObject* pkObject, float const fXAxis, float const fYAxis, float const fZAxis)
{
	NiTransform kTrans = pkObject->GetLocalTransform();
	
	NiMatrix3 kRot;
	//NiQuaternion kQuat(1.f, 0.f, 0.f, 0.f);
	if( fXAxis )
	{
		//NiQuaternion kQuatX;
		kRot.MakeXRotation(NxMath::degToRad(fXAxis));
		//kQuatX.FromAngleAxisX(NxMath::degToRad(fXAxis));
		//kQuat = kQuat * kQuatX;
		NiTransform kTransRot;
		kTransRot.MakeIdentity();
		kTransRot.m_Rotate = kRot;
		//kQuat.ToRotation(kTransRot.m_Rotate);

		pkObject->SetLocalTransform(kTrans * kTransRot);
	}
	if( fYAxis )
	{
		//NiQuaternion kQuatY;
		kRot.MakeYRotation(NxMath::degToRad(fYAxis));
		//kQuatY.FromAngleAxisY(NxMath::degToRad(fYAxis));
		//kQuat = kQuat * kQuatY;
		NiTransform kTransRot;
		kTransRot.MakeIdentity();
		kTransRot.m_Rotate = kRot;
		//kQuat.ToRotation(kTransRot.m_Rotate);

		pkObject->SetLocalTransform(kTrans * kTransRot);
	}
	if( fZAxis )
	{
		//NiQuaternion kQuatZ;
		kRot.MakeZRotation(NxMath::degToRad(fZAxis));
		//kQuatZ.FromAngleAxisZ(NxMath::degToRad(fZAxis));
		//kQuat = kQuat * kQuatZ;
		NiTransform kTransRot;
		kTransRot.MakeIdentity();
		kTransRot.m_Rotate = kRot;
		//kQuat.ToRotation(kTransRot.m_Rotate);

		pkObject->SetLocalTransform(kTrans * kTransRot);
	}

	//NiTransform kTrans = pkObject->GetLocalTransform();
//	NiTransform kTransRot;
//	kTransRot.MakeIdentity();
//	kTransRot.m_Rotate = kRot;
	//kQuat.ToRotation(kTransRot.m_Rotate);

//	pkObject->SetLocalTransform(kTrans * kTransRot);
}

void PgUIModel::WorldRotate(float const fXAxis, float const fYAxis, float const fZAxis)
{
	NiMatrix3 kRot;
	if(fXAxis)
	{
		float fCalcAxis = fXAxis;
		if( m_kLimitDegreeToAxis.x != 0.0f )
		{
			m_kTotalDegreeToAxis.x += fCalcAxis;
			if( fCalcAxis < -m_kLimitDegreeToAxis.x )
			{ 
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.x + m_kLimitDegreeToAxis.x);
				m_kTotalDegreeToAxis.x = -m_kLimitDegreeToAxis.x;
			}
			if( fCalcAxis > m_kLimitDegreeToAxis.x )
			{ 
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.x - m_kLimitDegreeToAxis.x);
				m_kTotalDegreeToAxis.x = m_kLimitDegreeToAxis.x;
			}
		}
		kRot.MakeXRotation(NxMath::degToRad(fCalcAxis));
	}

	if(fYAxis)
	{
		float fCalcAxis = fYAxis;
		if( m_kLimitDegreeToAxis.y != 0.0f )
		{
			m_kTotalDegreeToAxis.y += fCalcAxis;
			if( m_kTotalDegreeToAxis.z < -m_kLimitDegreeToAxis.z )
			{ 
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.y + m_kLimitDegreeToAxis.y);
				m_kTotalDegreeToAxis.y = -m_kLimitDegreeToAxis.y;
			}
			if( m_kTotalDegreeToAxis.z > m_kLimitDegreeToAxis.z )
			{
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.y - m_kLimitDegreeToAxis.y);
				m_kTotalDegreeToAxis.y = m_kLimitDegreeToAxis.y;
			}
		}
		kRot.MakeYRotation(NxMath::degToRad(fCalcAxis));
	}

	if(fZAxis)
	{
		float fCalcAxis = fZAxis;
		if( m_kLimitDegreeToAxis.y != 0.0f )
		{
			m_kTotalDegreeToAxis.z += fCalcAxis;
			if( m_kTotalDegreeToAxis.z < -m_kLimitDegreeToAxis.z )
			{ 
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.z + m_kLimitDegreeToAxis.z);
				m_kTotalDegreeToAxis.z = -m_kLimitDegreeToAxis.z;
			}
			if( m_kTotalDegreeToAxis.z > m_kLimitDegreeToAxis.z )
			{
				fCalcAxis = fZAxis - (m_kTotalDegreeToAxis.z - m_kLimitDegreeToAxis.z);
				m_kTotalDegreeToAxis.z = m_kLimitDegreeToAxis.z;
			}
		}
		kRot.MakeZRotation(NxMath::degToRad(fCalcAxis));
	}

	for(ContActorItor itor = m_kActorContainer.begin(); itor != m_kActorContainer.end(); ++itor)
	{
		NiTransform kTrans = (*itor).second->GetNIFRoot()->GetWorldTransform();
		NiTransform kTransRot;
		kTransRot.MakeIdentity();
		kTransRot.m_Rotate = kRot;

		(*itor).second->GetNIFRoot()->SetLocalTransform(kTransRot * kTrans);
	}

	for(ContNIFItor itor2 = m_kNIFContainer.begin(); itor2 != m_kNIFContainer.end(); ++itor2)
	{
		NiTransform kTrans = (*itor2).second->GetWorldTransform();
		NiTransform kTransRot;
		kTransRot.MakeIdentity();
		kTransRot.m_Rotate = kRot;

		(*itor2).second->SetLocalTransform(kTransRot * kTrans);
	}

	for(ContPgActor::iterator itor3 = m_kPgActorContainer.begin(); itor3 != m_kPgActorContainer.end(); ++itor3)
	{
		NiQuaternion kQuat;
		kQuat.FromRotation(kRot);
		
		(*itor3).second->m_spkActor->SetRotation(kQuat);
	}

	for(ContPgPet::iterator itor4 = m_kPgPetContainer.begin(); itor4 != m_kPgPetContainer.end(); ++itor4)
	{
		NiQuaternion kQuat;
		kQuat.FromRotation(kRot);
		
		(*itor4).second->m_spkPet->SetRotation(kQuat);
	}
}

void PgUIModel::SetWorldLimitRotate(float const fXAxis, float const fYAxis, float const fZAxis)
{
	m_kLimitDegreeToAxis = NiPoint3(fXAxis, fYAxis, fZAxis);
}

void PgUIModel::SetAutoRotate(bool const bAutoRotate, float const fXAxis, float const fYAxis, float const fZAxis)
{
	if(m_bAutoRotateCamera == bAutoRotate && 
		m_kAutoDegreeToAxis.x == fXAxis &&
		m_kAutoDegreeToAxis.y == fYAxis &&
		m_kAutoDegreeToAxis.z == fZAxis
		)
		return;

	m_bAutoRotateCamera = bAutoRotate;

	m_kAutoDegreeToAxis.x = fXAxis;
	m_kAutoDegreeToAxis.y = fYAxis;
	m_kAutoDegreeToAxis.z = fZAxis;
}

void PgUIModel::GetWorldBounds(NiAVObject* pkObject, NiBound& kBound)
{
    if (NiIsKindOf(NiNode, pkObject))
    {
        GetWorldBoundsNode((NiNode *)pkObject, kBound);
    }
    else if (NiIsKindOf(NiTriBasedGeom, pkObject))
    {
        NiGeometry* pkGeom = (NiGeometry*) pkObject;
        NiPoint3* pkVerts = pkGeom->GetVertices();

        NiBound kChildBound;
        kChildBound.ComputeFromData(pkGeom->GetVertexCount(), pkVerts);

        NiSkinInstance* pkSkin = pkGeom->GetSkinInstance();
        if (NULL != pkSkin)
        {
            pkSkin->UpdateModelBound(kChildBound);
        }

        kBound.Update(kChildBound, pkGeom->GetWorldTransform());
    }
}

void PgUIModel::GetWorldBoundsNode(NiNode *pkObject, NiBound &kBound)
{
    NiBound kWorldBound;
    kWorldBound.SetRadius(0.0f);

    for (unsigned int i = 0; i < pkObject->GetArrayCount(); ++i)
    {
        NiAVObject* pkChild = pkObject->GetAt(i);
        if (pkChild)
        {
            NiBound kChildBound;
            GetWorldBounds(pkChild, kChildBound);
            
            if (0.0f < kChildBound.GetRadius())
            {
                if (0.0f == kWorldBound.GetRadius())
                {
                    kWorldBound = kChildBound;
                }
                else
                {
                    kWorldBound.Merge(&kChildBound);
                }
            }
        }
    }

    kBound = kWorldBound;
}

bool PgUIModel::UpdateModel(NiAVObject* pkModelRoot, bool const bCameraReset)
{
	if(pkModelRoot)
	{
		pkModelRoot->SetWorldTranslate(NiPoint3::ZERO);
		pkModelRoot->SetTranslate(NiPoint3::ZERO);
		pkModelRoot->SetRotate(NiQuaternion::IDENTITY);
		pkModelRoot->UpdateNodeBound();
		pkModelRoot->UpdateProperties();
		pkModelRoot->UpdateEffects();
		pkModelRoot->Update(0.0f);		
	}

	if(bCameraReset)
	{
		CameraReset(pkModelRoot, NiPoint3(0.5f, -1.5f, 1.0f), NiPoint3(0.0f, 0.0f, 0.0f));
	}

	return true;
}

bool PgUIModel::SetCameraByName(std::string const& strNIFName, char const* pcCameraName)
{
	NiNode* pkNode = GetNIF(strNIFName);
	if(pkNode)
	{
		NiAVObject* pkObj = pkNode->GetObjectByName(pcCameraName);

		if (NiIsKindOf(NiNode, pkObj))
		{
			NiNode* pkNode = NiDynamicCast(NiNode, pkObj);
			for (unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
			{
				if (NiIsKindOf(NiCamera, pkNode->GetAt(i)))
				{
					pkObj = pkNode->GetAt(i);
					break;
				}
			}
		}

		if (NiIsKindOf(NiCamera, pkObj))
		{
			m_spCamera = NiDynamicCast(NiCamera, pkObj);
			return true;
		}
	}
	return false;
}

NiCamera* FindCamera(NiAVObject* pObject)
{
	NiCamera* pkCamera = NULL;

	if(NiIsKindOf(NiCamera, pObject))
	{
		pkCamera = NiDynamicCast(NiCamera, pObject);
	}

	NiNode* pkNode = NiDynamicCast(NiNode, pObject);
	if(pkNode)
	{
		for(unsigned int i=0; i<pkNode->GetArrayCount(); ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				NiCamera* pkCameraChild;
				pkCameraChild = FindCamera(pkChild);
				if(!pkCamera)
				{
					pkCamera = pkCameraChild;
				}
			}
		}
	}

	return pkCamera;
}

bool PgUIModel::SetRecursiveCameraByName(std::string const& strNIFName, char const* pcCameraName)
{
	NiNode* pkNode = GetNIF(strNIFName);
	if(pkNode)
	{
		NiAVObject* pkObj = pkNode->GetObjectByName(pcCameraName);
		if( pkObj )
		{
			NiCamera* TempCamera = FindCamera(pkObj);
			if(TempCamera)
			{
				m_spCamera = TempCamera;
				return true;
			}
		}
	}
	return false;
}

void PgUIModel::SetOrthoZoom(float const fZoom, int const iMoveX, int const iMoveY)
{
	m_fOrthoZoom = fZoom;
	m_fOrthoZoom = NiClamp(m_fOrthoZoom, m_fMinCameraZoom, m_fMaxCameraZoom);

	float fRate = static_cast<float>(m_kWndSize.x) / static_cast<float>(m_kWndSize.y);
	float fHeight = m_kCameraResetInfo.fRadius * 2.2f;
	float fWidth = fHeight * fRate;

	NiFrustum kFrustum = m_spCamera->GetViewFrustum();
	kFrustum.m_fLeft = -((fWidth * m_kCameraResetInfo.fXCenter) * m_fOrthoZoom) + iMoveX;
	kFrustum.m_fRight = ((fWidth * (1.0f - m_kCameraResetInfo.fXCenter))* m_fOrthoZoom) + iMoveX;
	kFrustum.m_fTop = ((fHeight * m_kCameraResetInfo.fYCenter) * m_fOrthoZoom) + iMoveY;
	kFrustum.m_fBottom = -((fHeight * (1.0f - m_kCameraResetInfo.fYCenter)) * m_fOrthoZoom) + iMoveY;
	m_spCamera->SetViewFrustum(kFrustum);
	m_spCamera->Update(0.0f);
}

void PgUIModel::SetOrthoZoomOld(float const fZoom, int const iMoveX, int const iMoveY)
{
	m_fOrthoZoom = fZoom;
	m_fOrthoZoom = NiClamp(m_fOrthoZoom, m_fMinCameraZoom, m_fMaxCameraZoom);

	NiFrustum kFrustum = m_spCamera->GetViewFrustum();
	kFrustum.m_fLeft = -((m_spRenderTexture->GetWidth() * m_fOrthoZoom) * 0.5f) + iMoveX;
	kFrustum.m_fRight = ((m_spRenderTexture->GetWidth() * m_fOrthoZoom) * 0.5f) + iMoveX;
	kFrustum.m_fTop = ((m_spRenderTexture->GetHeight() * m_fOrthoZoom) * 0.5f) + iMoveY;
	kFrustum.m_fBottom = -((m_spRenderTexture->GetHeight() * m_fOrthoZoom) * 0.5f) + iMoveY;
	m_spCamera->SetViewFrustum(kFrustum);
	m_spCamera->Update(0.0f);
}

void PgUIModel::SetNIFEnableUpdate(std::string const& strNIFName, bool const bEnableUpdate)
{
	ContNameItor itor = std::find(m_kDisableUpdateNIFContainer.begin(), m_kDisableUpdateNIFContainer.end(), strNIFName);

	if(bEnableUpdate)
	{
		//Disable List에 존재하면 삭제
		if(itor != m_kDisableUpdateNIFContainer.end())
		{
			m_kDisableUpdateNIFContainer.erase(itor);
		}
	}
	else
	{
		//Disable List에 존재하지 않으면 추가
		if(itor == m_kDisableUpdateNIFContainer.end())
		{
			m_kDisableUpdateNIFContainer.push_back(strNIFName);
		}
	}
}

void PgUIModel::SetActorEnableUpdate(std::string const& strActorName, bool const bEnableUpdate)
{
	ContNameItor itor = std::find(m_kDisableUpdateActorManagerContainer.begin(), m_kDisableUpdateActorManagerContainer.end(), strActorName);

	if(bEnableUpdate)
	{
		//Disable List에 존재하면 삭제
		if(itor != m_kDisableUpdateActorManagerContainer.end())
		{
			m_kDisableUpdateActorManagerContainer.erase(itor);
		}
	}
	else
	{
		//Disable List에 존재하지 않으면 추가
		if(itor == m_kDisableUpdateActorManagerContainer.end())
		{
			m_kDisableUpdateActorManagerContainer.push_back(strActorName);
		}
	}
}

void PgUIModel::ChangeTexture(NiNode *pkRoot, VariTextureContainer &rkTextureContainer)
{
	if (NULL == g_pkWorld || NULL == pkRoot)
	{
		return;
	}

	// 텍스쳐가 있으면 텍스쳐를 바꾸어준다.
	NiObjectList kGeometries;
	g_pkWorld->GetAllGeometries(pkRoot, kGeometries);
	while(!kGeometries.IsEmpty())
	{
		NiGeometry *pkGeo = NiDynamicCast(NiGeometry, kGeometries.GetTail());
		kGeometries.RemoveTail();
		if (!pkGeo || !pkGeo->GetPropertyState() || !pkGeo->GetPropertyState()->GetTexturing())
		{
			continue;
		}

		NiTexturingProperty* pkTextureProp = pkGeo->GetPropertyState()->GetTexturing();
		if (pkTextureProp && pkTextureProp->GetBaseMap())
		{
			NiSourceTexture* pkSrc = NiDynamicCast(NiSourceTexture, pkTextureProp->GetBaseMap()->GetTexture());
			std::string strTexture = pkSrc->GetFilename();

			VariTextureContainer::iterator itr = rkTextureContainer.find(strTexture);
			if(itr == rkTextureContainer.end())
			{
				continue;
			}
			std::string strNewTexture = itr->second;

			NiSourceTexture* pTexture = g_kNifMan.GetTexture(strNewTexture);
			if (pTexture)
			{
				pkTextureProp->GetBaseMap()->SetTexture(pTexture);
			}
		}
	}

	pkRoot->UpdateProperties();
}

bool PgUIModel::CalcPerspectiveFrustum(NiAVObject* pkObject, float const fXCenter, float const fYCenter, float const fPerspective)
{
	if( m_spCamera && pkObject )
	{
		NiFrustum kFrustum = m_spCamera->GetViewFrustum();
		if( kFrustum.m_bOrtho )
		{
			return false;
		}

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = m_kWndSize.x;
		rect.bottom = m_kWndSize.y;

		float fSceneRadius = pkObject->GetWorldBound().GetRadius();
		float fRight, fTop;
		float fFar = fSceneRadius * 4.0f;
		fRight = fTop = 1.0f / NiSqrt(3.0f);

		NiPoint3 kCameraLocation = m_spCamera->GetTranslate();
		NiPoint3 kWorldCenter = pkObject->GetWorldBound().GetCenter();
		NiPoint3 kDistanceVector = kCameraLocation - kWorldCenter;

		float fDist = kDistanceVector.Length();

		fFar += fDist;

		fFar > 0.0f ? 0 : fFar = 5.0f;

		if ( rect.right > rect.bottom )
			fRight *= (float) rect.right / (float) rect.bottom;
		else
			fTop *= (float) rect.bottom / (float) rect.right;

		float fWidth = fRight * fPerspective;
		float fHeight = fTop * fPerspective;

		kFrustum.m_fLeft = -(fWidth * fXCenter);
		kFrustum.m_fRight = (fWidth * (1.0f - fXCenter));
		kFrustum.m_fTop = (fHeight * fYCenter);
		kFrustum.m_fBottom = -(fHeight * (1.0f - fYCenter));
		kFrustum.m_fNear = 0.01f;
		kFrustum.m_fFar =fFar; 
		kFrustum.m_bOrtho = false;

		m_spCamera->SetViewFrustum(kFrustum);
		m_spCamera->Update(0.0f);
		return true;
	}
	return false;
}

bool PgUIModel::CalcOrthogonalFrustum(float fRadian, float const fXCenter, float const fYCenter)
{
	if( m_spCamera )
	{
		NiFrustum kFrustum = m_spCamera->GetViewFrustum();
		if( !kFrustum.m_bOrtho )
		{
			return false;
		}

		float fRate = static_cast<float>(m_kWndSize.x) / static_cast<float>(m_kWndSize.y);
		float fHeight = fRadian * 2.2f;
		float fWidth = fHeight * fRate;

		kFrustum.m_fLeft = -(fWidth * fXCenter);
		kFrustum.m_fRight = (fWidth * (1.0f - fXCenter));
		kFrustum.m_fTop = (fHeight * fYCenter);
		kFrustum.m_fBottom = -(fHeight * (1.0f - fYCenter));
		kFrustum.m_fNear = 1.0f;
		kFrustum.m_fFar    = 10000.0f;

		m_spCamera->SetViewFrustum(kFrustum);
		return true;
	}
	return false;
}

bool PgUIModel::AddPgActor(std::string const& strActorName, PgActorPtr pkSrcActor, bool const bClone, bool const bCameraReset)
{
	UIModelPgActorInfo* pkTemp = NULL;
	std::pair<ContPgActor::iterator, bool> kRet = m_kPgActorContainer.insert(std::make_pair(strActorName, pkTemp));
	//새로 등록 되는 경우
	if(kRet.second)
	{
		kRet.first->second = new UIModelPgActorInfo;		
	}
	
	if(kRet.first->second)
	{
		UIModelPgActorInfo* pkInfo = kRet.first->second;

		if(bClone)
		{
			int const iClass = pkSrcActor->GetPilot()->GetAbil(AT_CLASS);
			int const iLevel = pkSrcActor->GetPilot()->GetAbil(AT_LEVEL);
			int const iGender = pkSrcActor->GetPilot()->GetAbil(AT_GENDER);

			BM::GUID kGuid;
			kGuid.Generate();
			PgPilot* pkPilot =  g_kPilotMan.NewPilot(kGuid, iClass, iGender);
			g_kPilotMan.InsertPilot(kGuid, pkPilot);

			pkPilot->SetUnit(kGuid, UT_PLAYER, iClass, iLevel, iGender);			

			PgPlayer* pkCpyPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			PgPlayer* pkSrcPlayer = dynamic_cast<PgPlayer*>(pkSrcActor->GetPilot()->GetUnit());

			if(	pkCpyPlayer  
			&&	pkSrcPlayer)
			{
				pkCpyPlayer->ForceSetCoupleColorGuid(pkSrcPlayer->CoupleColorGuid());
			}

			pkInfo->m_spkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			pkInfo->m_bUpdate = true;
			
			if(g_pkWorld)
			{
				g_pkWorld->AddObject(kGuid, pkInfo->m_spkActor, pkSrcActor->GetPos(), OGT_PLAYER);
				pkInfo->m_spkActor->SetHide(true);
				pkInfo->m_spkActor->SetHideShadow(true);
			}

			pkInfo->m_spkActor->ClearActionState();
			pkInfo->m_spkActor->LockBidirection(false);

			pkInfo->m_spkActor->CopyEquipItem(pkSrcActor);
			pkInfo->m_spkActor->SetCheckMeetFloor(false);
			pkInfo->m_spkActor->SetMyWeaponAnimFolderNum(pkSrcActor->GetMyWeaponAnimFolderNum());
			pkInfo->m_spkActor->TransitAction(ACTIONNAME_IDLE/*pkSrcActor->GetAction()->GetID().c_str()*/);
		}
		else
		{
			pkInfo->m_spkActor = pkSrcActor;
			pkInfo->m_bUpdate = false;
		}

		if(pkInfo->m_spkActor)
		{
			if(pkSrcActor->GetPilot() && pkSrcActor->GetPilot()->GetUnit())
			{
				PgPet* pkPet = dynamic_cast<PgPet*>(pkSrcActor->GetPilot()->GetUnit());
				if(pkPet)
				{
					PgActorPet* pkActorPet = NiDynamicCast(PgActorPet,pkInfo->m_spkActor);
					if(pkActorPet)
					{
						int const iColor = pkSrcActor->GetPilot()->GetAbil(AT_COLOR_INDEX);
						if(pkInfo->m_spkActor->GetPilot())
						{
							pkInfo->m_spkActor->GetPilot()->SetAbil(AT_COLOR_INDEX, iColor);
						}
					}
				}
			}

			pkInfo->m_spkActor->SetPosition(NiPoint3::ZERO);
			pkInfo->m_spkActor->SetFreeMove(true);
			UpdateModel(pkInfo->m_spkActor, bCameraReset);
		}
	}

	return true;
}
PgActor* PgUIModel::GetPgActor(std::string const& strActorName) const
{
	ContPgActor::const_iterator itor = m_kPgActorContainer.find(strActorName);

	if(m_kPgActorContainer.end() != itor)
	{
		return (*itor).second->m_spkActor;
	}

	return NULL;
}

bool PgUIModel::AddPgPet(std::string const& strActorName, PgActorPet* pkSrcPet, bool const bClone, bool const bCameraReset)
{
	UIModelPgPetInfo* pkTemp = NULL;
	std::pair<ContPgPet::iterator, bool> kRet = m_kPgPetContainer.insert(std::make_pair(strActorName, pkTemp));
	//새로 등록 되는 경우
	if(kRet.second)
	{
		kRet.first->second = new UIModelPgPetInfo;		
	}
	
	if(kRet.first->second)
	{
		UIModelPgPetInfo* pkInfo = kRet.first->second;

		if(bClone)
		{
			int const iClass = pkSrcPet->GetPilot()->GetAbil(AT_CLASS);
			int const iLevel = pkSrcPet->GetPilot()->GetAbil(AT_LEVEL);
			int const iGender = pkSrcPet->GetPilot()->GetAbil(AT_GENDER);

			BM::GUID kGuid;
			kGuid.Generate();
			PgPilot* pkPilot =  g_kPilotMan.NewPilot(kGuid, iClass, iGender, OBJ_TYPE_PET);
			g_kPilotMan.InsertPilot(kGuid, pkPilot);
			pkPilot->SetAbil(AT_LEVEL, iLevel);

			pkPilot->SetUnit(kGuid, UT_PET, iClass, iLevel, iGender);

			pkInfo->m_spkPet = dynamic_cast<PgActorPet*>(pkPilot->GetWorldObject());
			if(!pkInfo->m_spkPet)	
			{	
				g_kPilotMan.RemovePilot(kGuid);
				m_kPgPetContainer.erase(kRet.first);
				return false;
			}
			pkInfo->m_spkPet->SetUnderMyControl(true);
			pkInfo->m_bUpdate = false;
			pkInfo->m_spkPet->NoTrace(true);
			
			if(g_pkWorld)
			{
				g_pkWorld->AddObject(kGuid, pkInfo->m_spkPet, pkSrcPet->GetPos(), OGT_PET);
				pkInfo->m_spkPet->SetHide(true);
				pkInfo->m_spkPet->SetHideShadow(true);
			}

			pkInfo->m_spkPet->SetPetType(pkSrcPet->GetPetType());	//1차인지 2차인지
			pkInfo->m_spkPet->ClearActionState();
			pkInfo->m_spkPet->LockBidirection(false);

			pkInfo->m_spkPet->CopyEquipItem(pkSrcPet);
			pkInfo->m_spkPet->SetCheckMeetFloor(false);
			pkInfo->m_spkPet->TransitAction("a_PetUIIdle");
		}
		else
		{
			pkInfo->m_spkPet = pkSrcPet;
			pkInfo->m_bUpdate = false;
		}

		if(pkInfo->m_spkPet)
		{
			if(pkSrcPet->GetPilot() && pkSrcPet->GetPilot()->GetUnit())
			{
				PgPet* pkPet = dynamic_cast<PgPet*>(pkSrcPet->GetPilot()->GetUnit());
				if(pkPet)
				{
					int const iColor = pkSrcPet->GetPilot()->GetAbil(AT_COLOR_INDEX);
					if(pkInfo->m_spkPet->GetPilot())
					{
						pkInfo->m_spkPet->GetPilot()->SetAbil(AT_COLOR_INDEX, iColor);
					}
				}
			}

			pkInfo->m_spkPet->SetPosition(NiPoint3::ZERO);
			pkInfo->m_spkPet->SetFreeMove(true);
			UpdateModel(pkInfo->m_spkPet, bCameraReset);
		}
	}

	return true;
}

bool PgUIModel::DeletePgPet(std::string const& strActorName)
{
	ContPgPet::iterator itor = m_kPgPetContainer.find(strActorName);
	if( itor != m_kPgPetContainer.end() )
	{
		if(g_pkWorld)
		{
			g_pkWorld->RemoveObjectOnNextUpdate((*itor).second->m_spkPet->GetGuid());
		}
		(*itor).second->m_spkPet = NULL;
		SAFE_DELETE((*itor).second);

		m_kPgPetContainer.erase(itor);

		return true;
	}
	return false;
}

PgActorPet*	PgUIModel::GetPgPet(std::string const& strActorName) const
{
	ContPgPet::const_iterator itor = m_kPgPetContainer.find(strActorName);
	if( itor != m_kPgPetContainer.end() )
	{
		return (*itor).second->m_spkPet;
	}

	return NULL;
}

bool PgUIModel::ChangePetAction(std::string const& strActorName, std::string const& strActionName)
{
	ContPgPet::const_iterator itor = m_kPgPetContainer.find(strActorName);
	if( itor != m_kPgPetContainer.end() )
	{
		ContPgPet::mapped_type const& kPatInfo = itor->second;
		if( kPatInfo && kPatInfo->m_spkPet )
		{
			return  kPatInfo->m_spkPet->TransitAction(strActionName.c_str());
		}
	}
	return false;
}

bool PgUIModel::DeletePgActor(std::string const& strActorName)
{
	ContPgActor::iterator itor = m_kPgActorContainer.find(strActorName);

	if(m_kPgActorContainer.end() != itor)
	{
		if(g_pkWorld)
		{
			g_pkWorld->RemoveObjectOnNextUpdate((*itor).second->m_spkActor->GetGuid());
		}
		(*itor).second->m_spkActor = NULL;
		SAFE_DELETE((*itor).second);

		m_kPgActorContainer.erase(itor);

		return true;
	}

	return false;
}
