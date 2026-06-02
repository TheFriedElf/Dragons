#include "StdAfx.h"
#include "PgPuppet.h"
#include "PgRenderer.h"
#include "PgAMPool.h"
#include "PgActionSlot.h"
#include "PgSoundMan.H"
#include "PgParticle.h"
#include "PgActor.H"
#include "PgPhysXUtil.H"
#include "PgWorld.h"

NiImplementRTTI(PgPuppet, PgIWorldObject);

PgPuppet::PgPuppet()
:m_bUsePhysX(false),
m_pkCurrentAudioSource(0),
m_bLoadingFinish(false),
m_bNowWorldEventStateTransit(false),
m_kSeqID(NiActorManager::INVALID_SEQUENCE_ID),
m_bUseTrigger(false),
m_bHasActorXML(false),
m_kRenamedPhysXObject(),
m_kOriginalNameOfRenamedPhysXObject()
{
	
	m_kPhysXTransform = NiTransform();
}

PgPuppet::~PgPuppet()
{
	if(g_pkWorld)
	{
		g_pkWorld->RemoveLightObjectRecurse(g_pkWorld->GetLightRoot(), this,PgWorld::LT_ALL);
	}
}

PgPuppet*	PgPuppet::CreatePuppet(std::string const &kName,std::string const &kKFMPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger)
{

	NiActorManagerPtr spAM = g_kAMPool.LoadActorManager(kKFMPath.c_str(), PgIXmlObject::ID_PC);
	if(!spAM)
	{
		return	NULL;
	}

	PgPuppet	*pkPuppet = NiNew PgPuppet();
	pkPuppet->SetUsePhysX(bUsePhysX);
	pkPuppet->SetUseTrigger(bUseTrigger);
	pkPuppet->SetID(kName.c_str());
	pkPuppet->SetName(kName.c_str());
	pkPuppet->SetObjectID(PgIXmlObject::ID_PUPPET);

	PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase;

	pkPuppet->SetWorldObjectBase(pkNewBase);

	NiQuaternion kQuat;
	kQuat.FromRotation(kTransform.m_Rotate);

	pkNewBase->SetScale(kTransform.m_fScale);
	pkNewBase->SetActorManager(spAM,pkPuppet,kTransform.m_fScale,false,kQuat);

	spAM->Reset();
	spAM->Update(0);
	spAM->SetTargetAnimation(kActivateSeqID);

	return	pkPuppet;
}

bool PgPuppet::Update(float fAccumTime, float fFrameTime)
{
	PgIWorldObject::Update(fAccumTime, fFrameTime);

	UpdateWorldEventStateTransition();

	NiActorManager *pkAM = GetActorManager();
	if (pkAM)
	{
		pkAM->Update(fAccumTime);
	}

	if(!m_bLoadingFinish)
	{
		DoLoadingFinishWork();
	}
	return true;
}

void PgPuppet::DoLoadingFinishWork()
{
	
	m_bLoadingFinish = true;
}

bool PgPuppet::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	return true;
}

bool PgPuppet::BeforeCleanUp()
{
	size_t iCur = 0;
	for( ; m_kRenamedPhysXObject.size() > iCur; ++iCur )
	{
		m_kRenamedPhysXObject.at(iCur)->SetName( m_kOriginalNameOfRenamedPhysXObject.at(iCur) );
	}
	m_kRenamedPhysXObject.clear();
	m_kOriginalNameOfRenamedPhysXObject.clear();
	return PgIWorldObject::BeforeCleanUp();
}

void	PgPuppet::UpdateWorldEventStateTransition()
{
	if(!m_bNowWorldEventStateTransit)
	{
		return;
	}

	if(IsAnimationDone() == false)
	{
		return;
	}

	m_bNowWorldEventStateTransit = false;

	PgIWorldObjectBase::stWorldEventStateInfo	const *pkStateInfo = GetWorldEventStateInfo();
	if(!pkStateInfo)
	{
		return;
	}

	std::string kSlot;
	if(!pkStateInfo->GetStateSlot(GetWorldEventStateID(),kSlot))
	{
		return;
	}

	PlaySlot(kSlot);
}

void	PgPuppet::SetWorldEventStateID(int iNewID,__int64 iChangeTime,bool bSetImmediate)
{

	if(GetWorldEventStateID() == -1)
	{
		bSetImmediate = true;
	}

	TransitToWorldEventState(GetWorldEventStateID(),iNewID,bSetImmediate);

	PgIWorldObject::SetWorldEventStateID(iNewID,iChangeTime,bSetImmediate);
}
void	PgPuppet::TransitToWorldEventState(int iFrom,int iTo,bool bSetImmediate)
{

	PgIWorldObjectBase::stWorldEventStateInfo	const *pkStateInfo = GetWorldEventStateInfo();
	if(!pkStateInfo)
	{
		_PgMessageBox(GetClientName(), "stWorldEventStateInfo is NULL, From[%d]->To[%d] Immediate[%d] In Puppet[%s]", iFrom, iTo, static_cast< int >(bSetImmediate), GetName());
		return;
	}

	std::string kSlot;
	if(bSetImmediate)
	{
		if(false == pkStateInfo->GetStateSlot(iTo,kSlot))
		{
			_PgMessageBox(GetClientName(), "Can't GetStateSlot From[%d]->To[%d] Immediate[%d] In Puppet[%s]", iFrom, iTo, static_cast< int >(bSetImmediate), GetName());
			return;
		}
	}
	else
	{
		if(false == pkStateInfo->GetTransitSlot(iFrom,iTo,kSlot))
		{
			if( iFrom != iTo )
			{
				_PgMessageBox(GetClientName(), "Can't find GetTransitSlot From[%d]->To[%d] Immediate[%d] In Puppet[%s]", iFrom, iTo, static_cast< int >(bSetImmediate), GetName());
			}
			return;
		}
	}

	if(!PlaySlot(kSlot))
	{
		_PgMessageBox(GetClientName(), "Can PlaySlot From[%d]->To[%d] Immediate[%d] Node[%s]", iFrom, iTo, static_cast< int >(bSetImmediate), GetName());
		return;
	}

	m_bNowWorldEventStateTransit = !bSetImmediate;
}
bool	PgPuppet::IsAnimationDone()	const
{
	PG_ASSERT_LOG(GetActorManager());
	if(!GetActorManager())	return	true;

	NiControllerSequence *pkSequence = GetActorManager()->GetSequence(m_kSeqID);

	if(!pkSequence || pkSequence->GetCycleType() == NiTimeController::LOOP)
	{
		return false;
	}

	if(GetActorManager()->GetCurAnimation() != m_kSeqID)
	{
		return false;
	}

	float fTime = GetActorManager()->GetNextEventTime(NiActorManager::END_OF_SEQUENCE, m_kSeqID);

	if(fTime == NiActorManager::INVALID_TIME)
	{
		return true;
	}

	return false;
}
void PgPuppet::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	if(!GetUsePhysX())
	{
		return;
	}

	if(GetUseTrigger())
	{
		uiGroup = PG_PHYSX_GROUP_TRIGGER - 1;
	}

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (pkPhysXScene == NULL || pkPhysXManager == NULL)
	{
		return;
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] WaitSDKLock\n",GetID().c_str());
		GetWorld()->LockPhysX(true);
	}

	NiActorManager	*pkAM = GetActorManager();
	if(!pkAM)
	{
		_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] No AM\n",GetID().c_str());
		return;
	}

	NiKFMTool	*pkKFMTool = pkAM->GetKFMTool();
	if(!pkKFMTool)
	{
		_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] No KFM Tool\n",GetID().c_str());
		return;
	}

	char	const	*pkNIFPath = pkKFMTool->GetFullModelPath();
	if(!pkNIFPath)
	{
		_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] No NIFPath\n",GetID().c_str());
		return;
	}

	NiStream kStream;
	if(kStream.Load(pkNIFPath))
	{
		int iCount = kStream.GetObjectCount();
		for(int i=0;i<iCount;i++)
		{

			NiObject *pkObject = kStream.GetObjectAt(i);
			if(NiIsKindOf(NiPhysXScene,pkObject))
			{
				NiPhysXScenePtr spPhysXSceneObj = NiDynamicCast(NiPhysXScene,pkObject);
				m_kPhysXSceneCont.push_back(spPhysXSceneObj);

				PgPhysXUtil::MakeStaticMeshNameUnique(spPhysXSceneObj,(char const*)pkNIFPath,GetID(),m_kPhysXTransform.m_fScale);
				PgPhysXUtil::ChangeScale(spPhysXSceneObj,m_kPhysXTransform.m_fScale);

				NxMat34 kSlaveMat;
				NiMatrix3 kPhysXRotMat;
				kPhysXRotMat.MakeIdentity();
				if(false == m_bHasActorXML)
				{
					kPhysXRotMat = m_kPhysXTransform.m_Rotate;
				}
				NiPoint3 kPhysXTranslation = m_kPhysXTransform.m_Translate;
				NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);

				spPhysXSceneObj->SetSlaved(pkPhysXScene, kSlaveMat);
				spPhysXSceneObj->CreateSceneFromSnapshot(0);

				unsigned	int	iSourceNum = spPhysXSceneObj->GetSourcesCount();
				for (unsigned int iSrcCount=0 ; iSrcCount< iSourceNum ; iSrcCount++)
				{
					NiPhysXSrc *pkPhysXSrc = spPhysXSceneObj->GetSourceAt(iSrcCount);

					NiPhysXKinematicSrc *pkPhysXKinematicSrcOrg = NiDynamicCast(NiPhysXKinematicSrc,pkPhysXSrc);
					if(!pkPhysXKinematicSrcOrg)
					{
						_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] No pkPhysXKinematicSrcOrg\n",GetID().c_str());
						continue;
					}
					NiAVObject	*pkGBSource = GetObjectByName(pkPhysXKinematicSrcOrg->GetSource()->GetName());
					if(!pkGBSource)
					{
						_PgMessageBox(GetClientName(), "PgPuppet::InitPhysX Name[%s] No pkGBSource\n", pkPhysXKinematicSrcOrg->GetSource()->GetName());
						continue;
					}

					NxActor	*pkTarget = pkPhysXKinematicSrcOrg->GetTarget();
					if(!pkTarget)
					{
						_PgOutputDebugString("PgPuppet::InitPhysX Name[%s] No pkTarget\n",GetID().c_str());
						continue;
					}

					NxShape *const *pkShapes = pkTarget->getShapes();
					int	iNumShapes = pkTarget->getNbShapes();

					for(int k=0;k<iNumShapes;k++)
					{

						NxShape	*pkShape = *pkShapes;

						pkShape->setGroup(uiGroup+1);
						pkShapes++;
					}

					pkTarget->setGroup(uiGroup+1);

					m_kRenamedPhysXObject.push_back(pkGBSource);
					m_kOriginalNameOfRenamedPhysXObject.push_back(pkGBSource->GetName());
					pkGBSource->SetName(GetName());

					NiPhysXKinematicSrc *pkPhysXKinematicSrc = NiNew NiPhysXKinematicSrc(pkGBSource, pkTarget);
					pkPhysXKinematicSrc->SetActive(true);
					pkPhysXKinematicSrc->SetInterpolate(false);
					
					pkPhysXScene->AddSource(pkPhysXKinematicSrc);

					m_kPhysXSrcCont.push_back(pkPhysXKinematicSrc);
				}
			}
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}
}

void PgPuppet::ReleasePhysX()
{

	if(m_pkPhysXScene)
	{
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
		if (GetWorld())
		{
			GetWorld()->LockPhysX(true);
		}

		int	const iTotalScene = m_kPhysXSceneCont.size();
		for(int i=0;i<iTotalScene;i++)
		{
			m_pkPhysXScene->RemoveSlave(m_kPhysXSceneCont[i]);
		}

		int	const iTotalSrc = m_kPhysXSrcCont.size();
		for(int i=0;i<iTotalSrc;i++)
		{
			m_pkPhysXScene->DeleteSource(m_kPhysXSrcCont[i]);
		}
		
		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}

	m_kPhysXSrcCont.clear();
	m_kPhysXSceneCont.clear();
}

void PgPuppet::Draw( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
	if ( pkRenderer == NULL || pkCamera == NULL )
		return;

	NiVisibleArray kArray;

	//	Z Test 를 하는 파티클 렌더링
	PgParticle	*pkParticle = NULL;
	for(AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); )
	{
		pkParticle = NiDynamicCast(PgParticle,itr->second);
		// 파티클 정리
		if(pkParticle && pkParticle->GetZTest() == true)
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->CullingProcess_Deprecated(pkCamera, pkParticle, &kArray, false);
			pkParticle->SetAppCulled(true);
		}

		++itr;
	}
    NiDrawVisibleArrayAppend( kArray );
}

void PgPuppet::DrawNoZTest( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
	PgParticle* pkParticle = NULL;
	for ( AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle,itr->second);
		if ( pkParticle && pkParticle->GetZTest() == false )
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->PartialRenderClick_Deprecated(pkParticle);
			pkParticle->SetAppCulled(true);
		}
	}

	for ( AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle, itr->second);
		if ( pkParticle )
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->PartialRenderClick_Deprecated(pkParticle);
			pkParticle->SetAppCulled(true);
		}
	}
}

void PgPuppet::ReloadNif()
{
	NiActorManager *pkAM = GetActorManager();
	if(!pkAM)
	{
		return;	
	}

//	pkAM->Reset();
//	pkAM->ChangeNIFRoot(pkAM->GetNIFRoot());
	pkAM->ReloadNIFFile();
}
bool	PgPuppet::PlaySlot(std::string const &kSlot)
{
	PgActionSlot *pkActionSlot = GetActionSlot();
	if(!pkActionSlot)
	{
		return false;
	}

	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(kSlot, kSeqID))
	{
		return false;
	}

	return PlayAnimation(kSeqID);
}
bool	PgPuppet::PlayAnimation(int iAnimationID)
{
	NiActorManager *pkAM = GetActorManager();
	if (pkAM == NULL)
	{ 
		PG_ASSERT_LOG(!"[PgPuppet::PlayAnimation]ActorManager doesn't exist!");
		return false;
	}

	if(!pkAM->GetSequence(iAnimationID))
	{
		return false;
	}

	pkAM->Reset();
	pkAM->SetTargetAnimation(iAnimationID);

	m_kSeqID = iAnimationID;

	return	true;
}
bool PgPuppet::TransitAction(char const *pcAction)
{
	NiActorManager *pkAM = GetActorManager();
	PgActionSlot *pkActionSlot = GetActionSlot();
	if (pkAM == NULL || pkActionSlot == NULL)
		return false;

	// 엑션에 지정된 에니를 플레이한다
	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(std::string(pcAction), kSeqID))
	{
		return false;
	}

	if(kSeqID == pkAM->GetCurAnimation())
	{
		pkAM->Reset();
		pkAM->Update(0);
		pkAM->SetTargetAnimation(NiActorManager::INVALID_SEQUENCE_ID);
		pkAM->Update(0.0f);
	}

	pkAM->SetTargetAnimation(kSeqID);
	NiControllerSequence *pkSeq = pkAM->GetSequence(kSeqID);

	// 예전에 플레이한 사운드를 정지한다.
	/*if(m_pkCurrentAudioSource)
	{
		if(m_pkCurrentAudioSource->GetStatus() == NiAudioSource::PLAYING)
		{
			m_pkCurrentAudioSource->Stop();
			m_pkCurrentAudioSource = 0;
		}
	}*/

	PgActionSlot::stSoundInfo kSoundInfo;
	if(pkActionSlot->GetSound(std::string(pcAction), kSoundInfo))
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,kSoundInfo.m_kSoundID.c_str(),kSoundInfo.m_fVolume,kSoundInfo.m_fMinDist,kSoundInfo.m_fMaxDist,this);
	}
	
	// 엑션에 지정된 사운드를 플레이한다.
	/*NiAudioSourcePtr pkAudioSource;
	if(pkActionSlot->GetSound(std::string(pcAction), pkAudioSource))
	{
		if(pkAudioSource->GetStatus() == NiAudioSource::PLAYING)
		{
			pkAudioSource->Stop();
		}
		pkAudioSource->Play();
		AttachChild(pkAudioSource);
		m_pkCurrentAudioSource = pkAudioSource;
	}*/

	return true;
}

NiObject* PgPuppet::CreateClone(NiCloningProcess& kCloning)
{
    PgPuppet* pkClone = NiNew PgPuppet;
    CopyMembers(pkClone, kCloning);
    return pkClone;
}

void PgPuppet::ProcessClone(NiCloningProcess& kCloning)
{
	PgIWorldObject::ProcessClone(kCloning);

	NiObject *pkClone = 0;
	
	bool bCloned = kCloning.m_pkCloneMap->GetAt(this, pkClone);
	PgPuppet *pkDest = 0;

	if(bCloned)
	{
		pkDest = (PgPuppet *) pkClone;
	}
	else
	{
		pkDest = this;
	}

	if(pkDest)
	{
		if (pkDest->GetActorManager())
		{
			pkDest->GetActorManager()->ReloadNIFFile();
			pkDest->GetActorManager()->Reset();
			pkDest->GetActorManager()->Update(0.0f);
		}
		pkDest->AttachChild(pkDest->GetNIFRoot(), true);
	}
}

bool PgPuppet::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			std::string kTagName(pkElement->Value());

			if(kTagName == "PUPPET")
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(kTagName == "WORLDOBJECT")
			{
				PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase;
				if(pkNewBase->ParseXml(pkNode, this))
				{
					SetObjectID(PgIXmlObject::ID_PUPPET);
				}
				else
				{
					SAFE_DELETE_NI(pkNewBase);
					return false;
				}

			}
			else if(kTagName == "WORLD_EVENT_STATE_INFO")
			{
			}

			//else if(strcmp(pcTagName, "KFMPATH") == 0)
			//{
			//	// KFM 경로를 알아낸다.
			//	PgIXmlObject::XmlObjectID eObjectID = PgIXmlObject::ID_NONE;
			//	if (pArg)
			//	{
			//		//! 확인 차 비교
			//		eObjectID = *((PgIXmlObject::XmlObjectID*)pArg);
			//		PG_ASSERT_LOG(eObjectID == PgIXmlObject::ID_PET);
			//		if (eObjectID != PgIXmlObject::ID_PET)
			//			NILOG(PGLOG_LOG, "[PgPuppet] Incorrect XmlObjectID for pet\n");
			//	}
			//	m_spAM = g_kAMPool.LoadActorManager(pkElement->GetText(), PgIXmlObject::ID_PET);
			//	SetObjectID(PgIXmlObject::ID_PET);

			//	if(!m_spAM)
			//	{
			//		PG_ASSERT_LOG(!"fatal : KFM Loading Failed.");
			//		return false;
			//	}

			//	const TiXmlAttribute *pAttr = pkElement->FirstAttribute(); 
			//	
			//	while(pAttr)
			//	{
			//		char const *pcAttrName = pAttr->Name();
			//		char const *pcAttrValue = pAttr->Value();

				
			//		if(strcmp(pcAttrName, "SCALE") == 0)
			//		{
			//			SetScale((float)atof(pcAttrValue));
			//		}
			//		else
			//		{
			//			PG_ASSERT_LOG(!"invalid attribute");
			//		}

			//		pAttr = pAttr->Next();
			//	}
			//}
			else if(kTagName == "POSITION")
			{
				NiPoint3 kTranslate(0.0f,0.0f,0.0f);
				const TiXmlAttribute *pAttr = pkElement->FirstAttribute(); 
				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();

					if(strcmp(pcAttrName, "X") == 0)
					{
						kTranslate.x = ((float)atof(pcAttrValue));
					}
					else if(strcmp(pcAttrName, "Y") == 0)
					{
						kTranslate.y = ((float)atof(pcAttrValue));
					}
					else if(strcmp(pcAttrName, "Z") == 0)
					{
						kTranslate.z = ((float)atof(pcAttrValue));
					}
					else
					{
						PG_ASSERT_LOG(!"invalid attribute");
					}

					pAttr = pAttr->Next();
				}

				SetTranslate(kTranslate);
			}
			//else if(strcmp(pcTagName, "SLOT") == 0)
			//{
			//	m_pkActionSlot = NiNew PgActionSlot;
			//	
			//	PG_ASSERT_LOG(m_pkActionSlot);				
			//	if(m_pkActionSlot == NULL || !m_pkActionSlot->ParseXml(pkNode, this))
			//	{
			//		SAFE_DELETE(m_pkActionSlot);
			//		PG_ASSERT_LOG(!"faild to loading Actor's Slot");
			//		return false;
			//	}
			//}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", kTagName.c_str());
				break;
			}
		}

	default:
		break;
	}


	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	// 모든 파싱이 끝났다면 DropBox를 초기화한다.
	if(strcmp(pkNode->Value(), "PUPPET") == 0)
	{
		NiActorManager *pkAM = GetActorManager();
		if (pkAM)
			pkAM->Update(0.0f);
		
		// TODO : Puppet처럼 Clone해서 쓰는 애들은 AttachChild해주면 안되는데
		// WorldObject에서 Attach해버리기 때문에 여기서 때주자.
		//DetachChild(GetNIFRoot());
	}

	return true;
}

int PgPuppet::GetCurAnimation()
{	
	return GetActorManager()->GetCurAnimation();
}