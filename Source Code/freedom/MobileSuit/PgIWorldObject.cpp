#include "stdafx.h"
#include "PgIWorldObject.h"
#include "PgIWorldObjectBase.h"
#include "PgAVObjectAlphaProcessManager.h"

// xxx 의존 제거
#include "PgActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgWorld.h"
#include "PgParticle.h"
#include "PgMobileSuit.h"
#include "PgRenderer.H"

#include "PgSoundMan.h"
#include "PgParticleMan.h"
#include "PgWorkerThread.h"
#include "PgParticleProcessorAttachToNode.H"
#include "PgParticleProcessorFadeIn.H"
#include "PgParticleProcessorFadeOut.H"
#include "NiAnimationEventSound.H"
#include "NiAnimationEventEffect.H"
#include "NiAnimationEventTextKey.H"
#include "NiAnimationEventFadeOutEffect.H"

#include "NewWare/Scene/ApplyTraversal.h"


std::string EXTRA_DATA_NAME_PARTICLE_TEXT_KEY = "PARTICLE_TEXT_KEY_NAME"; 
std::string EXTRA_DATA_NAME_PARTICLE_FADEOUT_WHEN_SEQUENCECHANGED= "EXTRA_DATA_NAME_PARTICLE_FADEOUT_WHEN_SEQUENCECHANGED"; 

NiImplementRTTI(PgIWorldObject, NiNode);

PgIWorldObject::PgIWorldObject() :
	m_pkWorldObjectBase(0),
	m_spWireframe(0),
	m_pkBoneLOD(0),
	m_pkPilot(0),
	m_pkPathRoot(0),
	m_kPathNormal(NiPoint3::ZERO),
	m_pkPhysXScene(0),
	m_bIsVisibleInFrustum(false),
	m_iLastSlot(1000000),
	m_pkEventCallback(0),
	m_pkSubEventCallback(0),
	m_bUseLOD(true),
	m_bIgnoreCameraCulling(false),
	m_iWorldEventStateID(-1),
	m_iWorldEventStateIDChangeTime(0),
	m_iHitObjectABVIndex(0)
{
}

PgIWorldObject::~PgIWorldObject()
{
	Terminate();
}

PgIWorldObject* PgIWorldObject::CreateCopy()
{
	return	NULL;
}
void	PgIWorldObject::Reset()
{
}

bool PgIWorldObject::BeforeUse()
{
	return true;
}

bool PgIWorldObject::BeforeCleanUp()
{
	
	ReleasePhysX();
	ReleaseAllParticles();
	g_kSoundMan.RecursiveDetachAllSoundObject(this);	//	사운드 오브젝트 떼어내기

	return true;
}

void PgIWorldObject::Terminate()
{
	SAFE_DELETE_NI(m_pkEventCallback);
	SAFE_DELETE_NI(m_pkSubEventCallback);
	m_pkPathRoot = 0;

	SAFE_DELETE_NI(m_pkWorldObjectBase);
}

void PgIWorldObject::ReleaseAllParticles()
{
	//	파티클 지우기
	// Attach 녀석들 정리
	for(AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); )
    {
		RemoveAVObject(itr->second, true);
		itr = m_kAttachSlot.erase(itr);
    }
	m_kAttachSlot.clear();

	// Attach_NoZTest 녀석들 정리
	for(AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); )
	{
		RemoveAVObject(itr->second, true);
		itr = m_kAttachSlot_NoZTest.erase(itr);
	}
	m_kAttachSlot_NoZTest.clear();
}


PgWorld* PgIWorldObject::GetWorld() const
{
	if( NULL == g_pkWorld )
	{
		PG_ASSERT_LOG(g_pkWorld);
		return NULL;
	}
    return g_pkWorld;
}

BM::GUID const& PgIWorldObject::GetGuid() const
{
    return (NULL != GetPilot())? GetPilot()->GetGuid(): m_kGuid;
}


bool PgIWorldObject::Pickup()
{
	if( !m_pkWorldObjectBase )
	{return false;}

	std::wstring kPickupScript = m_pkWorldObjectBase->GetPickupScript();
	if( !kPickupScript.length() )
	{
		return false;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);

	PgIWorldObject* pkTargetActor = this;
	if(GetPilot() && GetPilot()->GetUnit() &&
		GetPilot()->GetUnit()->IsUnitType(UT_PET))
	{ //펫이고 탑승 중이면 PC의 메뉴를 호출해야 한다.
		PgActor* pkThis = dynamic_cast<PgActor*>(this);
		if(pkThis && pkThis->GetMountTargetPet())
		{
			pkTargetActor = pkThis->GetMountTargetPet();
		}
	}	

	const bool bLuaRet = lua_tinker::call<bool, void *>(MB(kPickupScript), pkTargetActor );
	NIMETRICS_ENDTIMER(a);
	return bLuaRet;
}

bool PgIWorldObject::MouseOver(bool &rbWorking)
{
	rbWorking = false;
	if( !m_pkWorldObjectBase )
	{return false;}

	std::wstring kMouseOverScript = m_pkWorldObjectBase->GetMouseOverScript();
	if( !kMouseOverScript.length() )
	{
		return false;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	const bool bLuaRet = lua_tinker::call<bool, void *>(MB(kMouseOverScript), this);
	NIMETRICS_ENDTIMER(a);
	rbWorking = true;
	return bLuaRet;
}

bool PgIWorldObject::MouseOut()
{
	if( !m_pkWorldObjectBase )
	{return false;}

	std::wstring kMouseOutScript = m_pkWorldObjectBase->GetMouseOutScript();
	if( !kMouseOutScript.length() )
	{
		return false;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	const bool bLuaRet = lua_tinker::call<bool, void *>(MB(kMouseOutScript), this);
	NIMETRICS_ENDTIMER(a);
	return bLuaRet;
}

bool PgIWorldObject::IsMyActor()
{
	if (GetPilot() && g_kPilotMan.IsMyPlayer(GetPilot()->GetGuid()))
		return true;

	return false;
}

void*	Particle_Terminate_CallBackFunc(NiAVObject *pkAVObject)
{
	__asm	int	3;
}

bool PgIWorldObject::AttachTo(int iSlot, char const *pcTargetName, NiAVObject *pkObject)
{
	PG_ASSERT_LOG(g_pkWorld);
	PG_ASSERT_LOG(pcTargetName);
	PG_ASSERT_LOG(pkObject);

	if (pkObject == NULL)
	{
		return false;
	}

	if (g_pkWorld == NULL || pcTargetName == NULL)
	{
		return false;
	}

	NiAVObject* pkCurRoot = GetNIFRoot();
	if(!pkCurRoot)
	{
		return false;
	}

	NiNode *pkTarget = (NiNode *)pkCurRoot->GetObjectByName(pcTargetName);
	if(!pkTarget)
	{
		return false;
	}

	// 이 슬롯에 있는 이전 파티클을 삭제한다.
	DetachFrom( iSlot );

	pkObject->SetAppCulled( true );

	if ( g_pkWorld && g_pkWorld->GetSceneRoot() )
	{
		g_pkWorld->RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(pkObject, true));
	}

	PgParticle *pkParticle = NiDynamicCast(PgParticle, pkObject);
	if(pkParticle)
	{
		pkParticle->SetParticleProcessor(NiNew PgParticleProcessorAttachToNode(pkTarget));
	}

	m_kAttachSlot.insert(std::make_pair(iSlot, pkObject));

    return true;
}

bool PgIWorldObject::AttachToPoint(int iSlot, NiPoint3 kLoc, NiAVObject *pkObject)
{
	PG_ASSERT_LOG(g_pkWorld);
	PG_ASSERT_LOG(pkObject);

	if (pkObject == NULL)
	{
		return false;
	}

	PgPilot	*pkPilot = g_kPilotMan.GetPlayerPilot();
	
	if (g_pkWorld == NULL || pkPilot == NULL)
	{
		return false;
	}

	pkObject->SetTranslate(kLoc);
	pkObject->SetWorldTranslate(kLoc);
	pkObject->Update(g_pkWorld->GetAccumTime(), false);

	_PgOutputDebugString("AttachToPoint kLoc:(%f,%f,%f) ParticleID : %s\n",kLoc.x,kLoc.y,kLoc.z,pkObject->GetName());

	//pkObject->UpdateNodeBound();
	//pkObject->UpdateProperties();
	//pkObject->UpdateEffects();
	// 이 슬롯에 있는 이전 파티클을 삭제한다.
	DetachFrom(iSlot);
	// 새 객체를 붙인다.
	g_pkWorld->RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(pkObject, true));

	pkObject->SetAppCulled(true);

	float fActorDistance = 1;
	float fMainActorDistance = 0;

	if(g_pkWorld && g_pkWorld->m_kCameraMan.GetCamera())
	{
		PgActor	*pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
		NiPoint3	ptMainActorLoc(pkActor->GetPosition());
	
		NiCameraPtr	spCurrentCam = g_pkWorld->m_kCameraMan.GetCamera();

		NiPoint3	vCamPos = spCurrentCam->GetWorldLocation();
		NiPoint3	vCamPos2 = vCamPos+spCurrentCam->GetWorldRightVector();
		NiPoint3	vCamPos3 = vCamPos2+spCurrentCam->GetWorldUpVector();

		NiPlane	pkCamPlane(vCamPos,vCamPos2,vCamPos3);

		//	카레라 평면과 메인 액터와의 거리를 구한다.
		fMainActorDistance = NiAbs(pkCamPlane.Distance(ptMainActorLoc));

		//	카메라 평면과 현재 액터와의 거리를 구한다.
		fActorDistance = NiAbs(pkCamPlane.Distance(GetWorldTranslate()));
	}

	//	현재 액터의 카메라와의 거리가, 메인 액터의 카메라와의 거리보다, 일정 거리 이상 더 멀다면
	if(fActorDistance>fMainActorDistance+30)
	{
		//	현재 파티클을 Z버퍼 테스트하도록 만든다.
		// 슬롯에 정보를 넣는다.
		PgParticle	*pkParticle = NiDynamicCast(PgParticle,pkObject);

		if(!pkParticle || pkParticle->GetZTest())
			m_kAttachSlot.insert(std::make_pair(iSlot, pkObject));
		else
			m_kAttachSlot_NoZTest.insert(std::make_pair(iSlot, pkObject));

    }
    else
    {
		PgParticle	*pkParticle = NiDynamicCast(PgParticle,pkObject);
		if(!pkParticle || pkParticle->GetZTest() == false)
		//	그렇지 않다면, Z 테스트를 하지 않도록 만든다.
			m_kAttachSlot_NoZTest.insert(std::make_pair(iSlot, pkObject));
		else
			m_kAttachSlot.insert(std::make_pair(iSlot, pkObject));

	}

    return true;
}

void PgIWorldObject::SetParticleAlphaGroup(int iSlot,int iAlphaGroup)
{
	AttachSlot::iterator itr = m_kAttachSlot.find(iSlot);
	if(itr != m_kAttachSlot.end())	//	일반 슬롯에 없으면
	{
		NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( itr->second, iAlphaGroup );
		return;
    }

	itr = m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != m_kAttachSlot_NoZTest.end())
    {
		NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( itr->second, iAlphaGroup );
		return;
	}	
}

NiPoint3 PgIWorldObject::GetParticleNodeWorldPos(int iSlot, char *strNodeName)
{
	AttachSlot::iterator itr = m_kAttachSlot.find(iSlot);
	if(itr != m_kAttachSlot.end())	//	일반 슬롯에 없으면
	{
		NiAVObjectPtr spNode = itr->second->GetObjectByName(strNodeName);
		if(spNode == NULL)
		{
			return	NiPoint3(0,0,0);
		}
		return spNode->GetWorldTranslate();
	}

	itr = m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != m_kAttachSlot_NoZTest.end())
	{
		NiAVObjectPtr spNode = itr->second->GetObjectByName(strNodeName);
		if(spNode == NULL)
		{
			return	NiPoint3(0,0,0);
		}
		return spNode->GetWorldTranslate();
	}	

	return NiPoint3(0,0,0);
}

NiAVObject* PgIWorldObject::GetParticleNode( int iSlot, char const* strNodeName )
{
	AttachSlot::iterator iter = m_kAttachSlot.find(iSlot);
	if( iter != m_kAttachSlot.end() )
	{
		if( strcmp(iter->second->GetName(), strNodeName) )
		{
			return iter->second;
		}
	}

	iter = m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(iter != m_kAttachSlot_NoZTest.end())
	{
		if( strcmp(iter->second->GetName(), strNodeName) )
		{
			return iter->second;
		}
	}	
	return NULL;
}

NiAVObject* PgIWorldObject::GetParticleNodeWithTextKey( std::string const& kTextKey )
{
	for(AttachSlot::iterator itor = m_kAttachSlot.begin(); itor != m_kAttachSlot.end() ;++itor)
	{
		NiAVObject	*pkAVObject = itor->second;
		if(!pkAVObject)
		{
			continue;
		}
		NiStringExtraData *pkExtraData = NiDynamicCast(NiStringExtraData,pkAVObject->GetExtraData(EXTRA_DATA_NAME_PARTICLE_TEXT_KEY.c_str()));
		if(!pkExtraData)
		{
			continue;
		}
		if(pkExtraData->GetValue() == kTextKey.c_str())
		{
			return	pkAVObject;
		}
	}

	for(AttachSlot::iterator itor = m_kAttachSlot_NoZTest.begin(); itor != m_kAttachSlot_NoZTest.end() ;++itor)
	{
		NiAVObject	*pkAVObject = itor->second;
		if(!pkAVObject)
		{
			continue;
		}
		NiStringExtraData *pkExtraData = NiDynamicCast(NiStringExtraData,pkAVObject->GetExtraData(EXTRA_DATA_NAME_PARTICLE_TEXT_KEY.c_str()));
		if(!pkExtraData)
		{
			continue;
		}
		if(pkExtraData->GetValue() == kTextKey.c_str())
		{
			return	pkAVObject;
		}
	}
	return NULL;
}
void PgIWorldObject::FadeOutParticleWhenSequenceChanged()
{
	for(AttachSlot::iterator itor = m_kAttachSlot.begin(); itor != m_kAttachSlot.end() ;++itor)
	{
		PgParticle	*pkParticle = NiDynamicCast(PgParticle,itor->second);
		if(!pkParticle)
		{
			continue;
		}
		NiFloatExtraData *pkExtraData = NiDynamicCast(NiFloatExtraData,pkParticle->GetExtraData(EXTRA_DATA_NAME_PARTICLE_FADEOUT_WHEN_SEQUENCECHANGED.c_str()));
		if(!pkExtraData)
		{
			continue;
		}

		if(pkParticle->GetParticleProcessor(PgParticleProcessor::PPID_FADE_OUT))
		{
			continue;
		}

		pkParticle->SetParticleProcessor(NiNew PgParticleProcessorFadeOut(pkExtraData->GetValue()));
}

	for(AttachSlot::iterator itor = m_kAttachSlot_NoZTest.begin(); itor != m_kAttachSlot_NoZTest.end() ;++itor)
{
		PgParticle	*pkParticle = NiDynamicCast(PgParticle,itor->second);
		if(!pkParticle)
    {
			continue;
		}
		NiFloatExtraData *pkExtraData = NiDynamicCast(NiFloatExtraData,pkParticle->GetExtraData(EXTRA_DATA_NAME_PARTICLE_FADEOUT_WHEN_SEQUENCECHANGED.c_str()));
		if(!pkExtraData)
        {
			continue;
        }
		
		if(pkParticle->GetParticleProcessor(PgParticleProcessor::PPID_FADE_OUT))
    {
			continue;
        }

		pkParticle->SetParticleProcessor(NiNew PgParticleProcessorFadeOut(pkExtraData->GetValue()));
    }
}

bool PgIWorldObject::RemoveAVObject(NiAVObject* pObject, bool bDefaultThreadDelete)
{
	if (pObject)	
	{
		NiNode *pParent = pObject->GetParent();

		if (pParent != NULL)
		{
			//if (GetObjectID() == PgIXmlObject::ID_BOSS)
			//{
			//	NILOG(PGLOG_ERROR, "RemoveAVObject 0x%0X object(%s) has 0x%0X parent(%s)\n", pObject, pObject->GetName(), pParent, pParent->GetName());
			//}

			if(NiIsKindOf(NiAudioSource,pObject))	//	When pObject is an audio source, It must be deleted in the main thread.
			{
				pParent->DetachChild(pObject);
				return true;
			}

			if(bDefaultThreadDelete)
			{
				THREAD_DELETE_OBJECT(pParent->DetachChild(pObject));
			}
			else
			{
				PgWorld* pkWorld = GetWorld();
				if(pkWorld)
				{
					PgAVObjectAlphaProcessManager* pkAVObjectPM = pkWorld->GetAVObjectAlphaProcessManager();
					if(pkAVObjectPM)
					{					
						pkAVObjectPM->AddAVObject(pObject, 0.4f, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_DELETE, false, true);
					}
				}
			}
			//pParent->UpdateEffects();
			//pParent->UpdateProperties();
			//pParent->UpdateNodeBound();
		}
		else
		{
			if(NiIsKindOf(NiAudioSource,pObject))	//	When pObject is an audio source, It must be deleted in the main thread.
			{
				NiDelete pObject;
				return true;
			}

			if(bDefaultThreadDelete)
			{
				THREAD_DELETE_OBJECT(pObject);
			}
			else
			{
				PgWorld* pkWorld = GetWorld();
				if(pkWorld)
				{
					PgAVObjectAlphaProcessManager* pkAVObjectPM = pkWorld->GetAVObjectAlphaProcessManager();
					if(pkAVObjectPM)
					{
						pkAVObjectPM->AddAVObject(pObject, 0.4f, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_DELETE, false, true);
					}
				}
			}

			//if (GetObjectID() == PgIXmlObject::ID_BOSS)
			//{
			//	NILOG(PGLOG_ERROR, "RemoveAVObject 0x%0X object(%s) has no parent\n", pObject, pObject->GetName());
			//}
		}
	}
	return true;
}

bool PgIWorldObject::DetachFrom( int iSlot, bool bDefaultThreadDelete )
{
	NiAVObject*	pkObj = 0;
	AttachSlot::iterator itr = m_kAttachSlot.find(iSlot);
	if(itr != m_kAttachSlot.end())	//	일반 슬롯에 없으면
	{
		pkObj = itr->second;
		RemoveAVObject(pkObj, bDefaultThreadDelete);

		m_kAttachSlot.erase(itr);
		//NILOG(PGLOG_ERROR, "DetachFrom %d slot 0x%0X object(%s)\n", iSlot, pkObj, pkObj->GetName());
	}

	itr = m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != m_kAttachSlot_NoZTest.end())
    {
    	pkObj = itr->second;
		RemoveAVObject(pkObj, bDefaultThreadDelete);

		m_kAttachSlot_NoZTest.erase(itr);
		//NILOG(PGLOG_ERROR, "DetachFrom %d slot 0x%0X object(%s)\n", iSlot, pkObj, pkObj->GetName());
    }
	
    return true;
}

bool PgIWorldObject::ChangeParticleGeneration( int iSlot, bool bGenerate )
{
	NiAVObject*	pkObj = 0;
	AttachSlot::iterator itr = m_kAttachSlot.find(iSlot);
	if(itr != m_kAttachSlot.end())	//	일반 슬롯에 없으면
	{
		pkObj = itr->second;
		PgParticleMan::ChangeParticleGeneration(pkObj, bGenerate);

		m_kAttachSlot.erase(itr);
	}

	itr = m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != m_kAttachSlot_NoZTest.end())
	{
		pkObj = itr->second;
		PgParticleMan::ChangeParticleGeneration(pkObj, bGenerate);

		m_kAttachSlot_NoZTest.erase(itr);
	}
	
	return true;
}

void	OutputAVObjectInfo(NiAVObject *pkObject,int iTab)
{
	char	strTab[100];
	strTab[0]='\0';

	for(int j=0;j<iTab;j++)
	{
		strcat(strTab,"\t");
	}
	_PgOutputDebugString("%sOutputAVObjectInfo ObjectName : %s Local : (%f,%f,%f) World : (%f,%f,%f)\n", strTab, pkObject->GetName(), pkObject->GetTranslate().x, pkObject->GetTranslate().y, pkObject->GetTranslate().z, pkObject->GetWorldTranslate().x, pkObject->GetWorldTranslate().y, pkObject->GetWorldTranslate().z);

	if(NiIsKindOf(NiNode,pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
		if(pkNode)
		{
			int	iChild = pkNode->GetArrayCount();
			for(int i=0;i<iChild;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					OutputAVObjectInfo(pkChild,iTab+1);
				}
			}
		}
	}
}

void	PgIWorldObject::UpdateParticlesTransform(float fAccumTime, float fFrameTime)
{
	// Attach 녀석들 정리
	int iCount =0 ;
	for(AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); itr++)
	{
		// 파티클 정리
		PgParticle *pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle)
		{
			pkParticle->Update(fAccumTime,fFrameTime);
		}

		iCount++;
	}

	iCount =0 ;
	for(AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); itr++)
	{
		// 파티클 정리
		PgParticle *pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle)
		{
			pkParticle->Update(fAccumTime,fFrameTime);
		}
		iCount++;
	}
}
bool PgIWorldObject::Update( float fAccumTime, float fFrameTime )
{
	// Attach 녀석들 정리
	for(AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); )
	{
		// 파티클 정리
		PgParticle *pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle)
		{
			if(pkParticle->IsFinished())
			{// 실제로 이렇게 해서 파티클이 제대로 마무리 되지 않는다. PgActor의 Detach부분이 실행되어야 한다.
				RemoveAVObject(pkParticle);
				itr = m_kAttachSlot.erase(itr);
				continue;
			}
		}

		++itr;
	}

	// Attach_NoZTest 녀석들 정리
	for(AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); )
	{
		// 파티클 정리
		PgParticle *pkParticle = NiDynamicCast(PgParticle, itr->second);
		if(pkParticle)
		{
			if(pkParticle->IsFinished())
			{
				RemoveAVObject(pkParticle);
				itr = m_kAttachSlot_NoZTest.erase(itr);
				continue;
			}
		}
		++itr;
	}
	m_bIsVisibleInFrustum = false;	// OnVisible이 불려야만 true가 된다.
	return true;
}

void PgIWorldObject::OnVisible(NiCullingProcess& kCuller)
{
	m_bIsVisibleInFrustum = true;
	NiNode::OnVisible(kCuller);
}

void PgIWorldObject::SetHitPoint(NiPoint3 const &rkPoint)
{
	m_kHitPoint = rkPoint;
}

NiPoint3 &PgIWorldObject::GetHitPoint()
{
	return m_kHitPoint;
}

void PgIWorldObject::ProcessClone(NiCloningProcess& kCloning)
{
	NiNode::ProcessClone(kCloning);

	NiObject *pkClone = 0;
	
	bool bCloned = kCloning.m_pkCloneMap->GetAt(this, pkClone);
	PgIWorldObject *pkDest = 0;

	if(bCloned)
	{
		pkDest = (PgIWorldObject *) pkClone;
		if(m_pkWorldObjectBase)
		{
			pkDest->SetWorldObjectBase(m_pkWorldObjectBase->Clone());
		}
	}
	else
	{
		pkDest = this;
	}

	pkDest->SetID(GetID().c_str());
}

//---------------------------------------------------------------------------
void PgIWorldObject::RegisterEventCallback()
{
	PG_ASSERT_LOG(GetActorManager());
	PG_ASSERT_LOG(GetActorManager()->GetCallbackAVObject() == NULL);

	if (GetActorManager() == NULL)
		return;

	m_pkEventCallback = NiNew EventCallbackObject(this);
	GetActorManager()->SetCallbackAVObject(m_pkEventCallback);
}
void PgIWorldObject::UnregisterEventCallback()
{
	if(GetActorManager())
	{
		GetActorManager()->SetCallbackAVObject(NULL);
		SAFE_DELETE_NI(m_pkEventCallback);
	}
}

void PgIWorldObject::RegisterSubEventCallback()
{
	m_pkSubEventCallback = NiNew EventCallbackObject(this);
	GetWorldObjectBase()->GetSubActorManager()->SetCallbackAVObject(m_pkSubEventCallback);

}
//---------------------------------------------------------------------------
PgIWorldObject::EventCallbackObject::EventCallbackObject(PgIWorldObject *pkObject) : 
	m_pkObject(pkObject)
{
}
//---------------------------------------------------------------------------
PgIWorldObject::EventCallbackObject::~EventCallbackObject()
{
}
//--------------------------------------------------------------------------
void	PgIWorldObject::EventCallbackObject::EventActivated(NiActorManager *pkManager, 
															NiActorManager::EventType kEventType,
															NiActorManager::SequenceID eSequenceID, 
															float fCurrentTime, 
															float fEventTime, 
	NiAnimationEvent *pkAnimationEvent)
{
	if (pkManager == NULL || m_pkObject == NULL)
	{
		return;
	}

	switch(kEventType)
	{
	case	NiActorManager::ANIM_ACTIVATED:
		{
			DoParticleFadeOutWhenSequenceChanged();
		}
		break;
	case	NiActorManager::AUDIO_EVENT:
		{
			DoAudioEvent(pkAnimationEvent);
		}
		break;
	case	NiActorManager::FADEOUTEFFECT_EVENT:
		{
			DoFadeOutEffectEvent(pkAnimationEvent);
		}	
		break;
	case	NiActorManager::EFFECT_EVENT:
		{
			DoEffectEvent(pkAnimationEvent);
		}
		break;
	}


}
void	PgIWorldObject::EventCallbackObject::DoParticleFadeOutWhenSequenceChanged()
{
	m_pkObject->FadeOutParticleWhenSequenceChanged();
}
void	PgIWorldObject::EventCallbackObject::DoAudioEvent(NiAnimationEvent *pkEvent)
{
	NiAnimationEventSound	*pkSoundEvent = NiDynamicCast(NiAnimationEventSound,pkEvent);
	if(!pkSoundEvent)
	{
		return;
	}
	m_pkObject->AttachToSound(NiAudioSource::TYPE_3D,
								(char const*)pkSoundEvent->GetName(),
								pkSoundEvent->GetVolume(),
								pkSoundEvent->GetMinDist(),
								pkSoundEvent->GetMaxDist());
}
void	PgIWorldObject::EventCallbackObject::DoEffectEvent(NiAnimationEvent	*pkEvent)
{
	NiAnimationEventEffect	*pkEffectEvent = NiDynamicCast(NiAnimationEventEffect,pkEvent);
	if(!pkEffectEvent)
	{
		return;
	}

	PgParticle *pkParticle = g_kParticleMan.GetParticle(pkEffectEvent->GetEffectName(), 
		PgParticle::O_SCALE,
		pkEffectEvent->GetScale() * m_pkObject->GetEffectScale());

	if(!pkParticle)
	{
		return;
	}

	bool bRet = false;
	if(false==pkEffectEvent->GetAttachToPos())
	{
		bRet = m_pkObject->AttachTo(m_pkObject->m_iLastSlot++, pkEffectEvent->GetAttachPointName(), (NiAVObject *)pkParticle);
	}
	else
	{
		NiNode *pkTarget = (NiNode *)m_pkObject->GetObjectByName(pkEffectEvent->GetAttachPointName());
		
		if(NULL!=pkTarget)
		{
			pkParticle->SetRotate(pkTarget->GetWorldRotate());
			bRet = m_pkObject->AttachToPoint(m_pkObject->m_iLastSlot++, pkTarget->GetWorldTranslate(), (NiAVObject *)pkParticle);			
		}
		else
		{
			_PgOutputDebugString("DoEffectEvent Cannot Find Node Name %s ParticleID : %s\n", pkEffectEvent->GetAttachPointName(),pkEffectEvent->GetEffectName());
		}		
	}
	if(false==bRet)
	{
		THREAD_DELETE_PARTICLE(pkParticle);
	}

	if(pkEffectEvent->GetUseFadeIn() && pkEffectEvent->GetFadeInTime()>0)
	{
		pkParticle->SetParticleProcessor(NiNew PgParticleProcessorFadeIn(pkEffectEvent->GetFadeInTime()));
	}
	if(pkEffectEvent->GetUseFadeOutWhenSequenceChanged())
	{
		pkParticle->AddExtraData(EXTRA_DATA_NAME_PARTICLE_FADEOUT_WHEN_SEQUENCECHANGED.c_str(),NiNew NiFloatExtraData(pkEffectEvent->GetFadeOutTimeWhenSequenceChanged()));
	}

	pkParticle->AddExtraData(EXTRA_DATA_NAME_PARTICLE_TEXT_KEY.c_str(),NiNew NiStringExtraData(pkEffectEvent->GetTextKey()->GetText()));

}
void	PgIWorldObject::EventCallbackObject::DoFadeOutEffectEvent(NiAnimationEvent *pkEvent)
{
	NiAnimationEventFadeOutEffect	*pkFadeOutEffectEvent = NiDynamicCast(NiAnimationEventFadeOutEffect,pkEvent);
	if(!pkFadeOutEffectEvent)
	{
		return;
	}

	PgParticle	*pkParticle = NiDynamicCast(PgParticle,m_pkObject->GetParticleNodeWithTextKey((char const*)pkFadeOutEffectEvent->GetEffectName()));
	if(pkParticle)
	{
		pkParticle->SetParticleProcessor(NiNew PgParticleProcessorFadeOut(pkFadeOutEffectEvent->GetFadeOutTime()));
	}}

//--------------------------------------------------------------------------
//!	RayCheckObject 를 통해 찾아낸 충돌오브젝트의 중심좌표를 설정한다.
void	PgIWorldObject::SetHitObjectCenterPos(NiPoint3 const &kPoint)
{
	m_kHitObjectCenterPos = kPoint;
}

//!	RayCheckObject 를 통해 찾아낸 충돌오브젝트의 중심좌표를 반환한다.
const	NiPoint3	&PgIWorldObject::GetHitObjectCenterPos()
{
	return	m_kHitObjectCenterPos;
}
void	PgIWorldObject::SetHitObjectABVIndex(int iIndex)
{
	m_iHitObjectABVIndex = iIndex;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgWOPriorityQueue
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void	PgWOPriorityQueue::Clear()
{
	m_ObjList.clear();
}
bool	PgWOPriorityQueue::CheckExist(PgIWorldObject *pkObject)
{

	PgWOPriorityQueueNode	*pkNode;
	for(ObjList::iterator itor = m_ObjList.begin(); itor != m_ObjList.end(); itor++)
	{
		pkNode= &(*itor);
		if(pkNode->m_pkObject == pkObject)
		{
			return true;
		}
	}
	return	false;
}

void	PgWOPriorityQueue::AddObj(PgIWorldObject *pkObject,float fDistance)
{
	if(!pkObject) return;
	if(m_ObjList.size() == 0)
	{
		m_ObjList.push_back(PgWOPriorityQueueNode(pkObject,fDistance));
		return;
	}
	PgWOPriorityQueueNode	*pkNode;
	for(ObjList::iterator itor = m_ObjList.begin(); itor != m_ObjList.end(); itor++)
	{
		pkNode= &(*itor);
		if(fDistance<pkNode->m_fDistance)
		{
			m_ObjList.insert(itor,PgWOPriorityQueueNode(pkObject,fDistance));
			return;
		}
	}
	m_ObjList.push_back(PgWOPriorityQueueNode(pkObject,fDistance));
}

NiAVObject* PgIWorldObject::GetNIFRoot() const
{
	if (m_pkWorldObjectBase == NULL)
		return NULL;
	return m_pkWorldObjectBase->GetNIFRoot();
}

NiAVObject* PgIWorldObject::GetMainNIFRoot() const
{
	if (NULL == m_pkWorldObjectBase)
	{
		return NULL;
	}
	return m_pkWorldObjectBase->GetMainNIFRoot();
}

NiActorManager* PgIWorldObject::GetActorManager() const
{
	if (m_pkWorldObjectBase == NULL)
		return NULL;
	return m_pkWorldObjectBase->GetActorManager();
}

PgIWorldObjectBase::ABVShape *PgIWorldObject::GetABVShape(int iIndex)
{
	if (m_pkWorldObjectBase == NULL)
		return NULL;
	return m_pkWorldObjectBase->GetABVShape(iIndex);
}

const std::wstring &PgIWorldObject::GetUpdateScript() const
{
	if (m_pkWorldObjectBase == NULL)
	{
		static const std::wstring Temp;
		return Temp;		// TODO: 꼭 고치자. warning C4172: returning address of local variable or temporary
	}
	return m_pkWorldObjectBase->GetUpdateScript();
}

const std::wstring &PgIWorldObject::GetInitScript() const
{
	if (m_pkWorldObjectBase == NULL)
	{
		static const std::wstring Temp;
		return Temp;		// TODO: 꼭 고치자. warning C4172: returning address of local variable or temporary
	}
	return m_pkWorldObjectBase->GetInitScript();
}

PgActionSlot *PgIWorldObject::GetActionSlot() const
{
	if (m_pkWorldObjectBase == NULL)
		return NULL;
	return m_pkWorldObjectBase->GetActionSlot();
}

void PgIWorldObject::SetWorldObjectBase(PgIWorldObjectBase *pkWorldObjectBase)
{
	SAFE_DELETE_NI(m_pkWorldObjectBase);
	m_pkWorldObjectBase = pkWorldObjectBase;
}

void PgIWorldObject::SetPickupScript(char const *pcScript)
{
	if (m_pkWorldObjectBase == NULL)
		return;
	m_pkWorldObjectBase->SetPickupScript(UNI(std::string(pcScript)));
}

void PgIWorldObject::SetMouseOverScript(char const *pcScript)
{
	if (m_pkWorldObjectBase == NULL)
	{
		return;
	}
	m_pkWorldObjectBase->SetMouseOverScript(UNI(std::string(pcScript)));
}

void PgIWorldObject::SetMouseOutScript(char const *pcScript)
{
	if (m_pkWorldObjectBase == NULL)
	{
		return;
	}
	m_pkWorldObjectBase->SetMouseOutScript(UNI(std::string(pcScript)));
}

bool PgIWorldObject::GetMouseOverScript(std::wstring &rkScript)
{
	if (m_pkWorldObjectBase == NULL)
	{
		return false;
	}
	rkScript = m_pkWorldObjectBase->GetMouseOverScript();
	if( 0 == rkScript.length() )
	{
		return false;
	}
	return true;
}

bool PgIWorldObject::GetMouseOutScript(std::wstring &rkScript)
{
	if (m_pkWorldObjectBase == NULL)
	{
		return false;
	}
	rkScript = m_pkWorldObjectBase->GetMouseOutScript();
	if( 0 == rkScript.length() )
	{
		return false;
	}
	return true;
}

void PgIWorldObject::SetUpdateScript(char const *pcScript)
{
	if (m_pkWorldObjectBase == NULL)
		return;
	m_pkWorldObjectBase->SetUpdateScript(UNI(std::string(pcScript)));
}

float PgIWorldObject::GetEffectScale() const
{
	if (m_pkWorldObjectBase == NULL)
		return 1.0f;
	return m_pkWorldObjectBase->GetEffectScale();
}
float PgIWorldObject::GetScale() const
{
	if (m_pkWorldObjectBase == NULL)
		return 1.0f;
	return m_pkWorldObjectBase->GetScale();
}
void PgIWorldObject::SetSubIWorldObjectBase(PgIWorldObjectBase *pkBase)
{
	if (m_pkWorldObjectBase == NULL)
		return;
	m_pkWorldObjectBase->SetSubIWorldObjectBase(pkBase);
}

void PgIWorldObject::RemoveSubIWorldObjectBase()
{
	if (m_pkWorldObjectBase == NULL)
		return;
	m_pkWorldObjectBase->RemoveSubIWorldObjectBase();
}
void	PgIWorldObject::DoKFMTransition()	//	Thread Loading 중에 ChangeActor를 해버리면 어찌되는가?
{
	if(m_kReservedKFMTransit.size() == 0)
	{
		return;
	}
	if (m_pkWorldObjectBase == NULL)
		return;
	m_pkWorldObjectBase->ChangeActorManager(m_kReservedKFMTransit,this);

	m_kReservedKFMTransit = "";

}

void PgIWorldObject::AttachChild(NiAVObject* pkChild, bool bFirstAvail)
{
	NiNode::AttachChild(pkChild, bFirstAvail);
}

bool PgIWorldObject::RegisterCallback(NiActorManager::SequenceID kSeqID)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.RegisterCallback"), g_pkApp->GetFrameCount()));
	if(!GetActorManager())
	{
		PG_ASSERT_LOG(!"Actor Manager doens't initialized");
		return false;
	}
	
	GetActorManager()->ClearAllRegisteredCallbacks();
	
	NiControllerSequence *pkCS = GetActorManager()->GetSequence(kSeqID);
	if(!pkCS)
	{
		return false;
	}

	NiTextKeyExtraData *pkTextKeys = pkCS->GetTextKeys();
	unsigned int uiTextKeyCount;
	NiTextKey *pkTextKey = pkTextKeys->GetKeys(uiTextKeyCount);

	for(unsigned int uiTextKeyIdx = 0; uiTextKeyIdx < uiTextKeyCount; ++uiTextKeyIdx)
	{
		NiFixedString kTextKeyName = pkTextKey[uiTextKeyIdx].GetText();
		if(!GetActorManager()->RegisterCallback(NiActorManager::TEXT_KEY_EVENT, kSeqID, kTextKeyName))
		{
			PG_ASSERT_LOG(!"Register Callback fail!!");
		}
	}

	GetActorManager()->RefreshAdditionalEvent(kSeqID, pkCS);

	return true;
}

bool PgIWorldObject::AttachToSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax)
{
	g_kSoundMan.PlayAudioSourceByID(uiType, pcID, fVolume, fDistMin, fDistMax, this);
	return true;
}