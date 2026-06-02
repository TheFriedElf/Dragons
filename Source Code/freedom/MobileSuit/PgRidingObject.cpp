#include "stdafx.h"
#include "PgRidingObject.H"
#include "PgPilotMan.H"
#include "PgWorld.H"

NiImplementRTTI(PgRidingObject, PgActor);

///////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgRidingObject
///////////////////////////////////////////////////////////////////////////////////////////////
PgRidingObject::PgRidingObject()
{
	m_pkAttachedObject = NULL;
}
void PgRidingObject::ReleasePhysX()
{

	if(m_pkPhysXScene)
	{
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
		if (pkPhysXManager)
		{
			pkPhysXManager->WaitSDKLock();
		}

		int	iTotalSrc = m_vKinematicSrcCont.size();
		for(int i=0;i<iTotalSrc;i++)
		{
			m_pkPhysXScene->DeleteSource(m_vKinematicSrcCont[i]);
		}
		
		if (pkPhysXManager)
		{
			pkPhysXManager->ReleaseSDKLock();
		}
	}

	PgActor::ReleasePhysX();

	m_kPhysXSceneObjCont.clear();
	m_vKinematicSrcCont.clear();
}
bool PgRidingObject::BeforeCleanUp()
{
	m_kMountedActorMap.clear();
	m_pkAttachedObject = NULL;

	return	PgActor::BeforeCleanUp();
}
void	PgRidingObject::SetAttachedObject(char const *pkObjectName)
{
	if(!m_pkWorld)
	{
		return;
	}

	NiNode	*pkSceneRoot = m_pkWorld->GetSceneRoot();
	if(!pkSceneRoot)
	{
		return;
	}

	NiAVObject	*pkObject = pkSceneRoot->GetObjectByName(pkObjectName);
	if(pkObject)
	{
		pkObject->SetAppCulled(true);
		SetAttachedObject(pkObject);
	}
}
NiPoint3	const&	PgRidingObject::GetAttactedObjectPos()
{
	if(!m_pkAttachedObject)
	{
		return	NiPoint3(-1,-1,-1);
	}

	return	m_pkAttachedObject->GetWorldTranslate();
}

void	PgRidingObject::SetTransfromByAttachedObject()
{
	if(!m_pkAttachedObject)
	{
		return;
	}

	NiPoint3	const &kPosition = m_pkAttachedObject->GetWorldTranslate();
	SetPosition(kPosition);
}
void	PgRidingObject::MountActor(PgActor *pkActor)
{
	if(!pkActor)
	{
		return;
	}

	BM::GUID kPilotGuid = pkActor->GetPilotGuid();
	MountedActorCont::iterator itor = m_kMountedActorMap.find(kPilotGuid);
	if(itor != m_kMountedActorMap.end())
	{
		return;
	}

	m_kMountedActorMap.insert(std::make_pair(kPilotGuid,MountedActor(kPilotGuid)));

	_PgOutputDebugString("PgRidingObject::MountActor Guid:%s\n",MB(kPilotGuid.str()));

}
void	PgRidingObject::DemountActor(PgActor *pkActor)
{
	if(!pkActor)
	{
		return;
	}

	_PgOutputDebugString("DemountActor\n");

	BM::GUID kPilotGuid = pkActor->GetPilotGuid();
	MountedActorCont::iterator itor = m_kMountedActorMap.find(kPilotGuid);
	if(itor != m_kMountedActorMap.end())
	{
		m_kMountedActorMap.erase(itor);
		return;
	}
}
void PgRidingObject::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{

	NiPoint3	kMovedPosition = GetWorldTranslate();
	NxVec3	kMoveAmount = NxVec3(kMovedPosition.x - m_kPositionSaved.x,
								kMovedPosition.y - m_kPositionSaved.y,
								kMovedPosition.z - m_kPositionSaved.z);

	for(MountedActorCont::iterator itor = m_kMountedActorMap.begin(); itor != m_kMountedActorMap.end(); )
	{
		MountedActor &kMountedActor = itor->second;

		PgActor *pkMountedActor = g_kPilotMan.FindActor(kMountedActor.m_kUnitGUID);
		if(!pkMountedActor)
		{
			itor = m_kMountedActorMap.erase(itor);
			continue;
		}

		//_PgOutputDebugString("PgRidingObject pkMountedActor->MoveActorAbsolute[%f,%f,%f] Cont[%f,%f,%f]\n",kMoveAmount.x,kMoveAmount.y,kMoveAmount.z,kContMoveAmount.x,kContMoveAmount.y,kContMoveAmount.z);
		pkMountedActor->MoveActorAbsolute(kMoveAmount);

		itor++;
	}

	//SetTranslate(m_kPositionSaved);
	//SetWorldTranslate(m_kPositionSaved);
	//NiNode::Update(g_pkWorld->GetAccumTime());

	PgActor::Draw(pkRenderer,pkCamera,fFrameTime);

	//SetTranslate(kMovedPosition);
	//SetWorldTranslate(kMovedPosition);
	//NiNode::Update(g_pkWorld->GetAccumTime());

}
bool PgRidingObject::Update(float fAccumTime, float fFrameTime)
{
	m_kPositionSaved = GetWorldTranslate();

	
	bool bResult =	PgActor::Update(fAccumTime,fFrameTime);

	NiNode::UpdateDownwardPass(fAccumTime,true);

	return	bResult;
}
void PgRidingObject::UpdatePhysX(float fAccumTime, float fFrameTime)
{
}
void PgRidingObject::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);
}
void PgRidingObject::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{

	//SetAttachedObject("SD_SkyCastle_Elevator1001_01 01");

	PgActor::InitPhysX(pkPhysXScene,uiGroup);

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (pkPhysXScene == NULL || pkPhysXManager == NULL)
		return;

	if (g_iUseAddUnitThread == 1)
	{
		pkPhysXManager->WaitSDKLock();
	}

	NiActorManager	*pkAM = GetActorManager();
	if(!pkAM)
	{
		return;
	}

	NiKFMTool	*pkKFMTool = pkAM->GetKFMTool();
	if(!pkKFMTool)
	{
		return;
	}

	char	const	*pkNIFPath = pkKFMTool->GetFullModelPath();
	if(!pkNIFPath)
	{
		return;
	}

	NiStream kStream;
	if(kStream.Load(pkNIFPath))
	{
		int iCount = kStream.GetObjectCount();
		for(int i=1;i<iCount;i++)
		{

			NiObject *pkObject = kStream.GetObjectAt(i);
			if(NiIsKindOf(NiPhysXScene,pkObject))
			{
				NiPhysXScenePtr spPhysXSceneObj = NiDynamicCast(NiPhysXScene,pkObject);
				m_kPhysXSceneObjCont.push_back(spPhysXSceneObj);

				if(spPhysXSceneObj->GetSnapshot())
				{
					NiPhysXSceneDesc* pkDesc = spPhysXSceneObj->GetSnapshot();

					// 중복 네임이 있으면 피직스가 잘못 먹히기 때문에 강제로 이름을 바꾸어 준다.
					int iActorTotal = pkDesc->GetActorCount();
					for (int iActorCount=0 ; iActorCount<iActorTotal ; ++iActorCount)
					{
						NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
						int iShapeTotal = pkActorDesc->GetActorShapes().GetSize();
						for (int iShapeCount=0 ; iShapeCount<iShapeTotal ; iShapeCount++)
						{
							NiPhysXShapeDesc *pkShapeDesc =
								pkActorDesc->GetActorShapes().GetAt(iShapeCount);

							// Rename PhysX Object
							if (pkShapeDesc->GetMeshDesc())
							{
								NiString strDescName = GetID().c_str();
								strDescName += "_";
								char szCount[256];
								_itoa_s(iActorCount, szCount, 10);
								strDescName += szCount;
								strDescName += "_";
								_itoa_s(iShapeCount, szCount, 10);
								strDescName += szCount;
								strDescName += "_";
								//strDescName += strPath.c_str();
								NiFixedString strDescName_ = strDescName.MakeExternalCopy();
								pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
							}
						}
					}
				}

				NxMat34 kSlaveMat;
				NiMatrix3 kPhysXRotMat;
				kPhysXRotMat.IDENTITY;
				NiPoint3 kPhysXTranslation = GetTranslate();
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
						continue;
					}
					NiAVObject	*pkGBSource = GetObjectByName(pkPhysXKinematicSrcOrg->GetSource()->GetName());
					if(!pkGBSource)
					{
						continue;
					}

					NxActor	*pkTarget = pkPhysXKinematicSrcOrg->GetTarget();
					if(!pkTarget)
					{
						continue;
					}

					NxShape *const *pkShapes = pkTarget->getShapes();
					int	iNumShapes = pkTarget->getNbShapes();

					for(int k=0;k<iNumShapes;k++)
					{

						NxShape	*pkShape = *pkShapes;

						pkShape->setGroup(uiGroup+1);
						pkShape->userData = this;

						pkShapes++;
					}

					pkTarget->setGroup(uiGroup+1);
					pkTarget->userData = this;

					NiPhysXKinematicSrc *pkPhysXKinematicSrc = NiNew NiPhysXKinematicSrc(pkGBSource, pkTarget);
					pkPhysXKinematicSrc->SetActive(true);
					pkPhysXKinematicSrc->SetInterpolate(false);
					pkPhysXScene->AddSource(pkPhysXKinematicSrc);

					m_vKinematicSrcCont.push_back(pkPhysXKinematicSrc);
				}
			}
		}
	}

	if (g_iUseAddUnitThread == 1)
	{
		pkPhysXManager->ReleaseSDKLock();
	}
}

