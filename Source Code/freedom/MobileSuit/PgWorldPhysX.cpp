#include "stdafx.h"
#include "PgWorld.h"
#include "PgTrigger.h"
#include "PgShineStone.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgAction.h"
#include "PgNifMan.h"
#include "PgMobileSuit.h"
#include "PgStat.h"

NxActorGroup PgWorld::ms_kPhysXTerrainGroup = 10;
NxActorGroup PgWorld::ms_kPhysXTriggerGroup = 11;
NxActorGroup PgWorld::ms_kPhysXActorGroup = 16;


std::string const CAMTRG_LAST_ID("LastCameraTriggerID");
std::string const CAMTRG_LAST_TYPE("LastCameraTriggerType");
std::string const CAMTRG_LAST_GROUPNO("LAST_CAMTRG_GROUP");

NiPhysXScene *PgWorld::GetPhysXScene()
{
	return m_spPhysXScene;
}

NxActorGroup PgWorld::GetPhysXTerrainGroup()
{
	return ms_kPhysXTerrainGroup;
}

NxActorGroup PgWorld::GetPhysXTriggerGroup()
{
	return ms_kPhysXTriggerGroup;
}
void	PgWorld::SetPhysXGroup(NiPhysXScene *pkScene,int iPhysXGroup)
{
	if(!pkScene)
	{
		return;
	}

	NxScene	*pkNxScene = pkScene->GetPhysXScene();
	if(!pkNxScene)
	{
		return;
	}

	NxU32	uiTotalActor = pkNxScene->getNbActors();
	for(NxU32 ui = 0;ui <uiTotalActor; ++ui)
	{
		NxActor	*pkActor = *(pkNxScene->getActors()+ui);
		if(!pkActor)
		{
			continue;
		}

		NxU32	uiTotalShape = pkActor->getNbShapes();
		for(NxU32 uj = 0;uj <uiTotalShape; ++uj)
		{
			NxShape	*pkShape = *(pkActor->getShapes()+uj);
			if(!pkShape)
			{
				continue;
			}

			pkShape->setGroup(iPhysXGroup);
		}

		pkActor->setGroup(iPhysXGroup);
	}

}
NxActorGroup PgWorld::GetPhysXActorGroup()
{
	return ms_kPhysXActorGroup;
}
void	PgWorld::LockPhysX(bool bLock)
{
	if(!m_spPhysXScene)
	{
		return;
	}
	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if(bLock)
	{
		pkPhysXManager->WaitSDKLock();
		while(m_spPhysXScene->GetInSimFetch()) 
		{
			NiSleep(1);
		}
	}
	else
	{
		pkPhysXManager->ReleaseSDKLock();
	}
}
bool PgWorld::LoadPhysX(bool bCreatedSnapshot)
{
	// NiPhysXScene(겜브리오 레핑 PhysX)를 생성
	if(!m_spPhysXScene)
	{
		PG_ASSERT_LOG(!"NiPhysXScene not found");
		return false;
	}

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();

	PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
	PG_STAT(timerA.Start());
	LockPhysX(true);
	PG_STAT(timerA.Stop());

	if (!bCreatedSnapshot)
	{
		m_spPhysXScene->CreateSceneFromSnapshot(0);
	}

	// 물리 마스터 씬을 설정한다.
	NxScene *pkScene = m_spPhysXScene->GetPhysXScene();
	pkScene->setGravity(NxVec3(0.0f, 0.0f, m_fGravity));
	pkScene->setUserTriggerReport(&m_kPhysXTriggerHandler);
	pkScene->setUserContactReport(&m_kPhysXContactHandler);

	// 엑터들의 충돌 그룹을 설정한다.
	pkScene->setActorGroupPairFlags(2, 3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);

	LockPhysX(false);
	bool bUseFixedStep = false;
	float fTimeStep = 60;
	unsigned int iMaxIter = 1;
#if !defined(USE_INB) && !defined(EXTERNAL_RELEASE)
	bUseFixedStep = ((::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.FixedStep"), 0, g_pkApp->GetConfigFileName()))?true:false);
	fTimeStep = ::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.TimeStep"), 60, g_pkApp->GetConfigFileName());
	iMaxIter = ::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.MaxIter"), 1, g_pkApp->GetConfigFileName());
#endif
	if (fTimeStep > 0.0f)
		fTimeStep = 1.0f / fTimeStep;

	// 업데이트 정보를 설정한다.
	m_spPhysXScene->SetFetchTime(0.0f);
	m_spPhysXScene->SetUpdateSrc(true);
    m_spPhysXScene->SetUpdateDest(true);
	m_spPhysXScene->SetDoFixedStep(bUseFixedStep);
    m_spPhysXScene->SetTimestep(fTimeStep);
	m_spPhysXScene->GetPhysXScene()->setTiming(fTimeStep, iMaxIter, bUseFixedStep ? NX_TIMESTEP_FIXED : NX_TIMESTEP_VARIABLE);

	// PhysX 디버그 정보를 표시한다.
#ifndef USE_INB
	if( g_bUseDebugInfo )
	{
		m_spPhysXScene->SetDebugRender(true, m_spSceneRoot);
	}
	else
#endif
	{
		m_spPhysXScene->SetDebugRender(false, m_spSceneRoot);
	}
	return true;
}

bool PgWorld::FindObjFromPhysXSrc(NiPhysXSrc *pkPhysXSrc, NiAVObject **pout_SrcObj, NxActor **pout_Actor, int iSetShapeGroup)
{//맵 로딩이 끝난 후 피직스 오브젝트에 트리거 속성을 부여하느냐 마느냐 결정
	if(NiIsKindOf(NiPhysXRigidBodySrc, pkPhysXSrc))
	{
		NiPhysXRigidBodySrc *pkSrc = (NiPhysXRigidBodySrc *)pkPhysXSrc;
		*pout_SrcObj = pkSrc->GetSource();
		*pout_Actor = pkSrc->GetTarget();
		if (*pout_Actor)
		{
			(*pout_Actor)->setMass(1);
			NxShape *pkShape = *(*pout_Actor)->getShapes();
			if (pkShape)
			{
				if (iSetShapeGroup != -1)
				{
					int iGroup = pkShape->getGroup();
					if (iGroup == 0)
						pkShape->setGroup(iSetShapeGroup);
				}
				if(PG_PHYSX_GROUP_OBJECT == pkShape->getGroup())
				{
					return false;
				}
				else if(PG_PHYSX_GROUP_PUPPET == pkShape->getGroup())
				{
					return false;
				}
				pkShape->setFlag(NX_TRIGGER_ENABLE, true); 
			}
		}

		return true;
	}

	return false;
}

bool PgWorld::LoadTriggers()
{
	for(unsigned int i = 0;
		i < m_spPhysXScene->GetSourcesCount();
		i++)
	{
		NiPhysXSrc *pkPhysXSrc = m_spPhysXScene->GetSourceAt(i);
		NiAVObject *pkSrcObj = 0;
		NxActor *pkActor = 0;

		if (!FindObjFromPhysXSrc(pkPhysXSrc, &pkSrcObj, &pkActor, PG_PHYSX_GROUP_TRIGGER))
		{
			continue;
		}

		if(pkSrcObj && pkActor && pkSrcObj->GetName())
		{
			TriggerContainer::iterator itr = m_kTriggerContainer.find((char const *)pkSrcObj->GetName());
			
			if(itr == m_kTriggerContainer.end())
			{
				continue;
			}

			pkActor->userData = (void *)itr->second;
		}
	}

	return true;
}

bool PgWorld::LoadShineStones()
{
	bool bSingleMode = g_pkApp->IsSingleMode();
	if (bSingleMode)
	{
		int iIdx = 1;
		bool bFirst = true;

		ShineStoneContainer::iterator itr;
		for (itr = m_kShineStoneContainer.begin() ;
			itr != m_kShineStoneContainer.end() ;
			++itr, ++iIdx)
		{
			PgShineStone* pkStone = (PgShineStone*)itr->second;
			if (!pkStone)
			{
				continue;
			}

			BM::GUID guid;
			guid.Generate();
			AddObject(guid ,pkStone, pkStone->GetPos(), OGT_SHINESTONE);

			// PhysX 를 PhysXRoot에 추가해주자. slave
			NiPhysXScenePtr spPhysXObject = pkStone->GetPhysXObj();
			if (!spPhysXObject)
			{
				continue;
			}

			if (spPhysXObject->GetSnapshot())
			{
				NiPhysXSceneDesc* pkDesc = spPhysXObject->GetSnapshot();

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
							NiString strDescName = MB(pkStone->GetGuid().str());
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

				NxMat34 kSlaveMat;
				NiMatrix3 kPhysXRotMat;
				kPhysXRotMat.MakeIdentity();
				NiPoint3 kPhysXTranslation = pkStone->GetPos();
				NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);

				spPhysXObject->SetSlaved(m_spPhysXScene, kSlaveMat);
				{
					bFirst = false;
					spPhysXObject->CreateSceneFromSnapshot(0);
				}
			}

			// Insert PhysX Kinetic Data(Trigger Data)
			for (unsigned int iSrcCount=0 ; iSrcCount<spPhysXObject->GetSourcesCount() ; iSrcCount++)
			{
				NiPhysXSrc *pkPhysXSrc = spPhysXObject->GetSourceAt(iSrcCount);
				if(NiIsKindOf(NiPhysXRigidBodySrc, pkPhysXSrc))
				{
					NiPhysXRigidBodySrc *pkBodySrc = (NiPhysXRigidBodySrc *)pkPhysXSrc;
					NiAVObject *pkSrcObj = pkBodySrc->GetSource();
					pkSrcObj->SetName(MB(pkStone->GetGuid().str()));
				}
				m_spPhysXScene->AddSource(pkPhysXSrc);

				// Add to container
				NiAVObject *pkSrcObj = 0;
				NxActor *pkActor = 0;
				if (!FindObjFromPhysXSrc(pkPhysXSrc, &pkSrcObj, &pkActor, PG_PHYSX_GROUP_SHINESTONE))
				{
				}
				if(pkSrcObj && pkActor && pkSrcObj->GetName())
				{
					ShineStoneContainer::iterator itr = m_kShineStoneContainer.find(pkStone->GetGuid());
					if(itr == m_kShineStoneContainer.end())
					{
						continue;
					}

					pkActor->userData = (void *)itr->second;
				}
			}
		}
	}

	return true;

}

void PgWorld::PgPhysXContactReporter::onContactNotify(NxContactPair& pair, NxU32 events)
{
	//NxActor *pkActorA = pair.actors[0];
	//NxActor *pkActorB = pair.actors[1];

	//if((pkActorA->getGroup() == 2 && pkActorB->getGroup() == 3)
	//|| (pkActorA->getGroup() == 3 && pkActorB->getGroup() == 2))
	//{
	//	NxActor *pkPlayer = 0;
	//	
	//	if(pkActorA->getGroup() == 2)
	//	{
	//		pkPlayer = pkActorA;
	//	}
	//	else
	//	{
	//		pkPlayer = pkActorB;
	//	}

	//	if(pkPlayer)
	//	{
	//		PgActor *pkPlayerActor = (PgActor *)pkPlayer->userData;

	//		if(pkPlayerActor->GetAction()->GetID() != "a_touch_dmg")
	//		{
	//			pkPlayerActor->TransitAction("a_touch_dmg");
	//		}
	//	}
	//}
}

void PgWorld::PgPhysXTriggerReporter::onTrigger(NxShape &rkTriggerShape, NxShape &rkOtherShape, NxTriggerFlag kFlag)
{
	if(!g_pkWorld) return;

	NxActor &kTriggerActor = rkTriggerShape.getActor();
	NxActor &kPearActor = rkOtherShape.getActor();

	if(NULL == kTriggerActor.userData)
	{
		return;
	}

	if(NULL == kPearActor.userData)
	{
		return;
	}

	NxCollisionGroup kGroup = rkTriggerShape.getGroup();

	switch(kGroup)
	{
	case	PG_PHYSX_GROUP_TRIGGER:
		{
			if(!g_pkWorld)
			{
				return;
			}
			PgTrigger *pkTrigger = (PgTrigger *)kTriggerActor.userData;
			PgActor *pkActor = (PgActor *)kPearActor.userData;

			//	액터가 실제로 존재하는 액터인지 체크하자.
			if(g_pkWorld->CheckObjectExist(pkActor) == false)    { return; }
			//	트리거가 실제로 존재하는 액터인지 체크하자.
			if(g_pkWorld->CheckTriggerExist(pkTrigger) == false) { return; }

			if( NX_TRIGGER_ON_ENTER == kFlag )
			{//내 액터인지 여부와 관계없이 트리거에 닿았을 경우 오브젝트 연출 있으면 해주자( 연출에 대한 클라이언트 간에 동기화는 하지 않는다.(불필요)
				pkTrigger->SetRemoteObject();
			}

			BM::GUID kGuid;
			if(!pkActor || !pkActor->GetPilot())
			{
				return;
			}
			if(g_kPilotMan.IsMyPlayer(pkActor->GetPilot()->GetGuid()) )
			{
				if(pkActor->IsRidingPet()) //탑승 중일 땐, 펫으로만 충돌체크를 해야한다.
				{
					pkActor = pkActor->GetMountTargetPet();
				}
			}
			else if(pkActor->GetUnit() && pkActor->GetUnit()->IsUnitType(UT_PET) && pkActor->IsRidingPet())
			{ //충돌된 대상이 라이딩펫이고, 현재 PC가 탑승 중이면
				pkActor = pkActor->GetMountTargetPet(); //대상을 PC로 치환
			}
			else
			{
				return;
			}
			if(pkTrigger)
			{				
				char const* const pcCamType = pkTrigger->GetParamFromParamMap(CAMTRG_TYPE_NAME.c_str());
				if(pcCamType)
				{// 트리거가 CAMERA_TYPE이면
					if( false == CamTrgProcedure(pkTrigger, pkActor) )
					{// 처리 실패
						//_PgMessageBox("CameraTrigger Execute Error", "CameraTrigger Execute Error");
					}
				}
				else
				{// 트리거가 CAMERA_TYPE이 아니라면
					switch(kFlag)
					{
					case NX_TRIGGER_ON_ENTER:
						{// 트리거 진입시
							if(pkActor)
							{
								PgTrigger* pkActorCurTrigger = pkActor->GetCurrentTrigger();
								if(pkActorCurTrigger
									&& pkTrigger->GetID() != pkActorCurTrigger->GetID()
									)
								{// 현재 들어와있는 트리거와 다른 트리거에 진입했다면
									pkActorCurTrigger->OnLeave( pkActor, pkTrigger );	// 이전 트리거를 종료해주고
								}
							}
							pkTrigger->OnEnter(pkActor);	// 현재 트리거를 실행 하고
							
						}break;

					case NX_TRIGGER_ON_STAY:
						{
							pkTrigger->OnUpdate(pkActor);
						}break;

					case NX_TRIGGER_ON_LEAVE:
						{
							pkTrigger->OnLeave(pkActor, NULL);
						}break;
					}
				}
			}
		}break;
	case	PG_PHYSX_GROUP_SHINESTONE:
		{			
			if(!g_pkWorld)
			{
				return;
			}
			PgShineStone *pkStone = (PgShineStone *)kTriggerActor.userData;
			PgActor *pkActor = (PgActor *)kPearActor.userData;			

			//	액터가 실제로 존재하는 액터인지 체크하자.
			if(g_pkWorld->CheckObjectExist(pkActor) == false) return;
			//	샤인스톤이 실제로 존재하는 액터인지 체크하자.
			if(g_pkWorld->CheckShineStoneExist(pkStone) == false) return;

			BM::GUID kGuid;
			if(!pkActor || !pkActor->GetPilot() || !g_kPilotMan.IsMyPlayer(pkActor->GetPilot()->GetGuid()))
			{
				return;
			}

			if(pkStone)
			{
				switch(kFlag)
				{
				case NX_TRIGGER_ON_ENTER:
					{
						if (pkStone->OnEnter(pkActor))
						{
							unsigned int iCount = kTriggerActor.getNbShapes();
							NxShape* const* kShapeArray = kTriggerActor.getShapes();

							if (kShapeArray)
							{
								for (unsigned i = 0; i < iCount; i++)
								{
									NxShape* pkShape = kShapeArray[i];
									if (pkShape != NULL)
									{
										if (pkShape->getFlag(NX_TRIGGER_ENABLE))
										{
											pkShape->setFlag(NX_TRIGGER_ENABLE, false);
										}
									}
								}
							}
						}
					}
					break;
				case NX_TRIGGER_ON_STAY:{pkStone->OnUpdate(pkActor);}break;
				case NX_TRIGGER_ON_LEAVE:{pkStone->OnLeave(pkActor);}break;
				}
			}
		}break;
	}

}

//
//bool PgWorld::PgPhysXTriggerReporter::CompareCamTrgName(std::string const& kOldTrgName, std::string const& kNewTrgName)
//{// 이름에서 MARK를 제거하고, 같은 종류의 카메라 트리거 인지 확인한다
//	std::string kLhs = kOldTrgName;
//	std::string kRhs = kNewTrgName;
//	std::transform( kLhs.begin(), kLhs.end(), kLhs.begin(), toupper );	
//	std::transform( kRhs.begin(), kRhs.end(), kRhs.begin(), toupper );
//
//	std::string::size_type LhsIndex = kLhs.rfind(CAMTRG_IN_MARK);
//	std::string::size_type RhsIndex = 0;
//
//	if( std::string::npos != LhsIndex )
//	{// CAMTRG_IN_MARK가 존재한다면, 다른쪽에서 CAMTRG_OUT_MARK를 찾아보고
//		RhsIndex = kRhs.rfind(CAMTRG_OUT_MARK);
//		if( std::string::npos == RhsIndex )
//		{// 없으면 둘은 같은 종류가 아님
//			return false;
//		}
//	}
//	else
//	{// 혹시 반대로 입력 했다면, 
//		LhsIndex = kLhs.rfind(CAMTRG_OUT_MARK);
//		if( std::string::npos != LhsIndex )
//		{// CAMRGR_OUT_MARK을 찾아보고, 다른쪽에서 CAMTRG_IN_MARK를 찾아보고
//			RhsIndex = kRhs.rfind(CAMTRG_IN_MARK);
//			if( std::string::npos == RhsIndex )
//			{// 없으면 둘은 같은 종류가 아님
//				return false;
//			}
//		}
//		else
//		{// CAMRGR_IN_MARK, CAMRGR_OUT_MARK 둘다 없다면, 같은 종류가 아님
//			return false;
//		}
//	}
//
//	std::string const kFilteredLhsCamName (kLhs, 0, LhsIndex);
//	std::string const kFilteredRhsCamName (kRhs, 0, RhsIndex);
//
//	if( kFilteredLhsCamName == kFilteredRhsCamName ) 
//	{// MARK를 제외한 문장이 같다면, 둘은 같은 종류!
//		return true;
//	}
//
//	return false;
//}


bool PgWorld::PgPhysXTriggerReporter::CamTrgProcedure(PgTrigger* pkTrigger, PgActor* pkActor)
{// 리턴 값이 true이면 정상, false이면 비정상인 상황
	if( NULL == pkTrigger )
	{
		return false;
	}

	if( NULL ==  pkActor )
	{
		return false;
	}

	char const* const pcCamType = pkTrigger->GetParamFromParamMap(CAMTRG_TYPE_NAME.c_str());
	if( NULL == pcCamType )
	{//CAMERA_TYPE이 아니라면 종료
		return false;
	}

	// 카메라 트리거 그룹 No 얻음
	int const iCamTrgGroupNo = pkTrigger->GetCamTrgInfoGroup();

	std::string const  kNewTrgCamType(pcCamType);
	std::string const& kNewTrgID = pkTrigger->GetID();
	
	char const* const pcLastCamTrgID = pkActor->GetParam(CAMTRG_LAST_ID.c_str());
	if( NULL == pcLastCamTrgID || 0 == ::strcmp(CAMTRG_EMPTY_MARK.c_str(), pcLastCamTrgID) )
	{// 처음 실행되는 CAMERA_TYPE 트리거 라면,

		if( CAMTRG_OUT_TYPE == kNewTrgCamType )
		{// 첫 실행이래도, IN을 만나기 전에 실행 될 수 없다.
			return true;
		}

		if(false == kNewTrgID.empty())
		{// 처음 실행되는 CAMERA_TYPE 트리거 ID를 기억하고, 카메라 세팅
			if( NULL == g_pkWorld ) 
			{
				return false;
			}
			PgCameraMan* pkCameraMan = g_pkWorld->GetCameraMan();
			if( NULL == pkCameraMan )
			{
				return false;
			}
			PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(pkCameraMan->GetCameraMode());
			if( NULL == pkCameraMode )
			{
				return false;
			}

			{
				NiPoint3 const kRelativePos(pkTrigger->GetCamTrgInfoWidth(), pkTrigger->GetCamTrgInfoZoom(), pkTrigger->GetCamTrgInfoHeight());
				NiPoint3 const kRelativeLookAt(pkTrigger->GetCamTrgInfoTargetWidth(), pkTrigger->GetCamTrgInfoTargetDepth(), pkTrigger->GetCamTrgInfoTargetHeight());
				pkCameraMode->SetCameraAdjustCameraInfo(kRelativePos, kRelativeLookAt);
			}
			{// in, out 타입의 트리거라면 최근에 실행된 카메라 트리거 이름과 타입을 기억하고
				pkActor->SetParam(CAMTRG_LAST_ID.c_str(),	kNewTrgID.c_str());
				pkActor->SetParam(CAMTRG_LAST_TYPE.c_str(),	kNewTrgCamType.c_str());
				{
					char szBuf[MAX_PATH] ={0, };
					::sprintf(szBuf,"%d",iCamTrgGroupNo);
					pkActor->SetParam(CAMTRG_LAST_GROUPNO.c_str(), szBuf);
				}
			}
		}
		else
		{// 트리거 ID가 없는 경우는 있을수 없음!
			_PgMessageBox("CameraTrigger Execute Error", "Something is wrong. check camera trigger ID" );
			return false;
		}
	}
	else
	{// 처음 실행되는 CAMERA_TYPE 트리거가 아니고,
		std::string const kLastCamTrgID(pcLastCamTrgID);
		char const* pcLastCamTrgType = pkActor->GetParam(CAMTRG_LAST_TYPE.c_str());

		int iLastCamTrgGroupNo = 0;
		{
			char const* const pcLastCamTrgGroupNo = pkActor->GetParam(CAMTRG_LAST_GROUPNO.c_str());
			if(pcLastCamTrgGroupNo)
			{
				iLastCamTrgGroupNo = ::atoi(pcLastCamTrgGroupNo);
			}
		}

		if( NULL != pcLastCamTrgType )
		{
			std::string const kLastCamTrgType(pcLastCamTrgType);
			if( kLastCamTrgType != kNewTrgCamType )
			{// 서로 타입이 다르고
				
				if(iLastCamTrgGroupNo != iCamTrgGroupNo)
				{// 같은 그룹의 카메라 트리거가 아니면 사용할 수 없고
					return false;
				}

				if( CAMTRG_OUT_TYPE == kNewTrgCamType )
				{// IN이 OUT을 만났을때, 리셋하고, 카메라 세팅한다!

					if( NULL == g_pkWorld ) 
					{
						return false;
					}
					PgCameraMan* pkCameraMan = g_pkWorld->GetCameraMan();
					if( NULL == pkCameraMan )
					{
						return false;
					}
					PgCameraModeFollow* pkCameraMode = 
						dynamic_cast<PgCameraModeFollow*>(pkCameraMan->GetCameraMode());
					if( NULL == pkCameraMode )
					{
						return false;
					}
					{
						NiPoint3 const kRelativePos(pkTrigger->GetCamTrgInfoWidth(), pkTrigger->GetCamTrgInfoZoom(), pkTrigger->GetCamTrgInfoHeight());
						NiPoint3 const kRelativeLookAt(pkTrigger->GetCamTrgInfoTargetWidth(), pkTrigger->GetCamTrgInfoTargetDepth(), pkTrigger->GetCamTrgInfoTargetHeight());
						pkCameraMode->SetCameraAdjustCameraInfo(kRelativePos, kRelativeLookAt);
					}
					pkActor->SetParam(CAMTRG_LAST_ID.c_str(),	CAMTRG_EMPTY_MARK.c_str());
					pkActor->SetParam(CAMTRG_LAST_TYPE.c_str(), CAMTRG_EMPTY_MARK.c_str());
				}
				else
				{// OUT이 실행되며, IN을 만나는것은 있을수 없음!
					//_PgMessageBox("CameraTrigger Execute Error", "Something is wrong. check camera trigger");
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
#if !defined(USE_INB) && !defined(EXTERNAL_RELEASE)
	WriteToConsole("%s 실행\n", pkTrigger->GetID().c_str() );
#endif
	// 스크립트와 연결 안되므로, CamTrg라면 이하는 실행될 필요없음.
	return true;
}