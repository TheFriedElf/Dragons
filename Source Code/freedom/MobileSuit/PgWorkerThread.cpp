#include "StdAfx.h"
#include "PgWorld.h"
#include "PgWorkerThread.h"
#include "PgActor.h"
#include "PgMobileSuit.h"
#include "PgAMPool.h"
#include "PgParticle.h"
#include "PgNiFile.h"
#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgDropBox.h"
#include "PgSelectStage.h"

PgClientWorkerThread g_kAddUnitThread;
PgClientWorkerThread g_kEquipThread;
PgClientWorkerThread g_kLoadWorkerThread;
PgClientWorkerThread g_kMemoryWorkerThread;

bool PgClientWorkerThread::ms_bUseSleepControl = false;

void PgClientWorkerThread::SetWorkerSleepControl(bool bUse)
{
	ms_bUseSleepControl = bUse;
}

PgClientWorkerThread::PgClientWorkerThread()
{
	IsStop(false);
	IsPause(false);
	msg_queue()->high_water_mark(E_WATER_MARK);
	m_dwLastWorkTime = BM::GetTime32();
	m_iCurrentWorkingTask = 0;
}

PgClientWorkerThread::~PgClientWorkerThread()
{
}

bool PgClientWorkerThread::ClearAllWorks(bool bDeleteAll)
{
	IsPause(true);
	unsigned int iCount = msg_queue()->message_count();
	unsigned int iClearCount = 0;

	NILOG(PGLOG_THREAD, "[PgWorkerThread] ClearAllWorks start(%d) %d task is exist\n", g_pkApp->GetFrameCount(), iCount);

	ACE_Time_Value tv(1, 1000);	tv += ACE_OS::gettimeofday();
	for (unsigned int i = 0; i < iCount; i++)
	{
		SWorkerThreadMessage *pkMsg = NULL;
		bool bClearMsg = false;
		if ( -1 != msg_queue()->dequeue( pkMsg, &tv ))
		{
			PG_ASSERT_LOG(pkMsg);
			switch(pkMsg->iType)
			{
			case E_WTMT_ITEM_EQUIP:
				{
					NILOG(PGLOG_THREAD, "[PgWorkerThread] %d work : item %d for %s\n", i, pkMsg->kInfo.iItemNo, MB(pkMsg->kGuid.str()));
					bClearMsg = true;
				}break;
			case E_WTMT_ADDUNIT:
				{
					CUnit* pUnit = (CUnit*)pkMsg->kWorkData.pWorkData;
					PG_ASSERT_LOG(pUnit);
					if (pUnit)
					{
						NILOG(PGLOG_THREAD, "[PgWorkerThread] %d work : add unit %s\n", i, MB(pUnit->GetID().str()));
						g_kPilotMan.RemoveReservedPilot(pUnit->GetID());
						g_kTotalObjMgr.UnRegistUnit(pUnit);
						g_kTotalObjMgr.ReleaseUnit(pUnit);
					}

					bClearMsg = true;
				}break;
			// do nothing. 빼서 맨뒤에 다시 넣는다;
			case E_WTMT_DELETE:
			case E_WTMT_CLONE_ACTOR:
			case E_WTMT_LOAD_XML:
			case E_WTMT_LOAD_SOUND:
			case E_WTMT_LOAD_EFFECT:
				{
					if (bDeleteAll)
					{
						bClearMsg = true;
					}
					else
					{
						if (-1 == msg_queue()->enqueue_tail(pkMsg, 0))
						{
							PG_ASSERT_LOG(!"Enqueue failed");
						}
					}
				}
				break;
			default:
				{
				}break;
			}

			if (bClearMsg)
			{
				SAFE_DELETE(pkMsg);
				iClearCount++;
			}
		}
	}

	iCount = msg_queue()->message_count();
	NILOG(PGLOG_THREAD, "[PgWorkerThread] ClearAllWorks end(%d) %d task cleared, %d task is exist\n", g_pkApp->GetFrameCount(), iClearCount, iCount);

	while (bDeleteAll && m_iCurrentWorkingTask > 0 && m_bIsWorking)
	{
		NILOG(PGLOG_WARNING, "[PgWorkerThread] %d working task exist, ClearAllWorks end(%d) %d task cleared, %d task is exist\n", m_iCurrentWorkingTask, g_pkApp->GetFrameCount(), iClearCount, iCount);
		NiSleep(10);
	}

	IsPause(false);
	return true;
}

bool PgClientWorkerThread::DoDeleteObject(NiAVObjectPtr spObject, EWorkPriority ePriority)
{
	if (spObject == NULL)
	{
		return false;
	}

	g_bUseThreadDelete = (::GetPrivateProfileInt(_T("Debug"), _T("UseThreadDelete"), 1, g_pkApp->GetConfigFileName()) == 1);
	if (g_bUseThreadDelete == false)
	{
		spObject = NULL;
		return true;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoDeleteObject(0x%08X,%d,%s,%s)(%d)\n", spObject, spObject->GetRefCount(), spObject->GetRTTI()->GetName(), spObject->GetName(), msg_queue()->message_count());
	if (IsStop() || IsPause())
	{
		spObject = 0;
		return true;
	}

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();

	pkMsg->iType = E_WTMT_DELETE;
	pkMsg->pObject = spObject;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();

	if(spObject->GetParent())
	{
		if (spObject->GetRefCount() == 1)
		{
			NILOG(PGLOG_WARNING, "[PgWorkerThread] DoDeleteObject(0x%08X,%d,%s,%s) refcount==1\n", spObject, spObject->GetRefCount(), spObject->GetRTTI()->GetName(), spObject->GetName());
		}
		spObject->GetParent()->DetachChild(spObject);	//	일단 부모와의 연결을 끊는다(신그래프에서 제거함)
	}

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_DELETE, ePriority));
		}break;
	}
	
	if(-1 != iResult)
	{
		return true;
	}
	PG_ASSERT_LOG(!"Can't enqueue DoDeleteObject to workerThread\n");
	return false;
}

bool PgClientWorkerThread::DoDeleteParticle(NiAVObjectPtr spObject, EWorkPriority ePriority)
{
	if (spObject == NULL || NiIsKindOf(PgParticle, spObject) == false)
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoDeleteParticle(0x%08X,%d,%s,%s)(%s)\n", spObject, spObject->GetRefCount(), spObject->GetRTTI()->GetName(), spObject->GetName(), msg_queue()->message_count());
	if (IsStop() || IsPause())
	{
		spObject = 0;
		return true;
	}

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();

	pkMsg->iType = E_WTMT_DELETE;
	pkMsg->pObject = spObject;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_DELETE, ePriority));
		}break;
	}

	if(-1 != iResult)
	{
		return true;
	}
	PG_ASSERT_LOG(!"Can't enqueue DoDeleteObject to workerThread\n");
	return false;
}

bool PgClientWorkerThread::DoEquipItem(PgActor* pkActor, BM::GUID const &kActorGuid, int const iItemNo, bool const bSetToDefaultItem, int iGender, int iClass, EWorkPriority ePriority)
{
	PG_ASSERT_LOG(kActorGuid != BM::GUID::NullData());
	if (kActorGuid == BM::GUID::NullData() || pkActor == NULL)
	{
		return false;
	}

	if (IsStop() || IsPause())
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] Insert EquipItem(%s, %d, %d, %d, %d)(%d)\n", MB(kActorGuid.str()), iItemNo, bSetToDefaultItem, iGender, iClass, msg_queue()->message_count());

	ItemEquipInfo info;
	info.iItemNo = iItemNo;
	info.iGender = iGender;
	info.iClass = iClass;
	info.bSetToDefault = bSetToDefaultItem;

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();
	pkMsg->iType = E_WTMT_ITEM_EQUIP;
	pkMsg->kGuid = kActorGuid;
	pkMsg->kInfo = info;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_ITEM, ePriority));
		}break;
	}

	if(-1 != iResult)
	{
		pkActor->IncEquipCount();
		return true;
	}

	PG_ASSERT_LOG(!"Can't enqueue DoEquipItem to workerThread\n");
	return false;
}

bool PgClientWorkerThread::DoAddUnit(CUnit* pkUnit, PgWorld* pkWorld, EWorkPriority ePriority)
{
	if (pkUnit == NULL || pkWorld == NULL)
	{
		return false;
	}

	if (IsStop() || IsPause())
	{
		NILOG(PGLOG_ERROR, "[PgWorkerThread] DoAddUnit(%s, %d) while s(%d),p(%d)\n", MB(pkUnit->GetID().str()), pkUnit->UnitType(), IsStop(), IsPause());
		return false;	// TODO: 아무래도 IsPause일때도 Insert를 해야 하지 않을까?
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoAddUnit(%s, %d)(%d)\n", MB(pkUnit->GetID().str()), pkUnit->UnitType(), msg_queue()->message_count());

	if ( !g_kPilotMan.InsertReservedPilot( pkUnit->GetID(), pkUnit->UnitType() ) )
	{
		return false;
	}

	bool bMyBox = false;

	switch(pkUnit->UnitType())
	{
	case UT_ENTITY: // 일단 Entity는 Thread를 안타게 해놨지만 혹시나 해서.		
		{
			NILOG(PGLOG_WARNING, "[PgWorkerThread] DoAddUnit %d type added\n", pkUnit->UnitType());
			ePriority = E_PRIORITY_HIGHEST;
		}break;
	case UT_GROUNDBOX:
		{
			PgGroundItemBox *pBox = dynamic_cast<PgGroundItemBox*>(pkUnit);
			if (pBox &&	g_pkWorld && g_pkWorld->IsMineItemBox(pBox))
			{
				bMyBox = true;
				ePriority = E_PRIORITY_HIGHEST;
			}
			else
			{
				ePriority = E_PRIORITY_LOWEST;
			}
		}
		break;
	case UT_PLAYER:
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>( pkUnit );
			if(!pkPlayer)	{ break; }
			BM::GUID const& rkGuid = pkPlayer->SubPlayerID();
			if(rkGuid.IsNull())	{ break; }
			PgPilot* pkSubPlayerPilot = g_kPilotMan.FindPilot(rkGuid);
			if(!pkSubPlayerPilot)	{ break; }
			CUnit* pkSubPlayerUnit = pkSubPlayerPilot->GetUnit();
			if(!pkSubPlayerUnit)	{ break; }
			PgSubPlayer* pkSubPlayer = dynamic_cast<PgSubPlayer*>(pkSubPlayerUnit);
			if(!pkSubPlayer) { break; }
			//pkSubPlayer->Caller(pkPlayer->GetID()); // 서버에서 하고 패킷으로 쏘기 떄문에 이부분은 필요 없음
			pkSubPlayer->SetCallerUnit(pkPlayer);
			PgActor* pkPlayerActor = g_kPilotMan.FindActor( pkPlayer->GetID() );
			if(pkPlayerActor)
			{// 각 파츠 얻어와서 동일하게 구성
				//PgActor* pkSubPlayerActor = g_kPilotMan.FindActor(rkGuid);
				//if( pkSubPlayerActor )
				//{
				//	pkSubPlayerActor->CopyEquipItem(pkPlayerActor);
				//	pkSubPlayerActor->DoReservedAction(PgActor::RA_OPENING, true);
				//	pkSubPlayerActor->LockBidirection(false);//8방향 풀기. 안풀면 움직일 때 방향이 자꾸 바뀌어 이상하게 보임
				//}
			}
		}break;
	case UT_SUB_PLAYER:
		{// Player보다 먼저 추가됨
		}break;
	case UT_MONSTER:
		{
			ePriority = E_PRIORITY_HIGH;
		}break;
	case UT_PET:
		{ //펫이 항상 주인을 참조해야 함
			PgPilot* pkCallerPilot = g_kPilotMan.FindPilot(pkUnit->Caller());
			if(pkCallerPilot)
			{
				CUnit* pkCallerUnit = pkCallerPilot->GetUnit();
				PgPet* pkPet = dynamic_cast<PgPet*>(pkUnit);
				if(pkPet)
				{
					pkPet->SetCallerUnit(pkCallerUnit);
				}
			}
		}break;
	case UT_NPC:
		{
			NILOG(PGLOG_WARNING, "[PgWorkerThread] DoAddUnit %d type added\n", pkUnit->UnitType());
		}
		break;
	case UT_BOSSMONSTER:
		// NPC/BOSS는 이리로 안들어온다.
		{
			NILOG(PGLOG_WARNING, "[PgWorkerThread] DoAddUnit %d type added\n", pkUnit->UnitType());
		}
		break;
	// 안 쓰임
	//case UT_CALLERTYPE:
	//	break;
	//case UT_UNIT_AI:
	//	break;
	default:
		break;
	}

	DUMMY_LOG	CAUTION_LOG( BM::LOG_LV1, L"ADD USER ["<<pkUnit->GetID()<<L":"<<pkUnit->Name()<<L"]");

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;

	pkMsg->iType = E_WTMT_ADDUNIT;
	pkMsg->kWorkData.pWorkData = (void*)pkUnit;
	pkMsg->pWorker = (void*)pkWorld;
	pkMsg->kWorkData.bWorkData = bMyBox;	// UNIT이 UT_GROUNDBOX일때만 쓴다.
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();	

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_ADDUNIT, ePriority));
		}break;
	}

	if(-1 != iResult)
	{
		return true;
	}
	return false;
}

bool PgClientWorkerThread::DoCloneActor(stAMPoolInfo* info, int iCount, EWorkPriority ePriority)
{
	if (info == NULL || IsStop() || IsPause())
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoCloneActor(%s, %d)(%d)\n", info->strKFMPath.c_str(), info->kAMList.size(), msg_queue()->message_count());

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();

	pkMsg->iType = E_WTMT_CLONE_ACTOR;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();
	pkMsg->kWorkData.pWorkData = (void*)info;

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_CLONE, ePriority));
		}break;
	}

	if(-1 != iResult)
	{
		return true;
	}
	return false;
}

bool PgClientWorkerThread::DoLoadResource(PgIClientWorker* pkWorker, WorkData& rkWorkData, EWorkPriority ePriority)
{
	if (pkWorker == NULL || m_bIsWorking == false || IsStop())
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoLoadResource(0x%08X worker, 0x%08X, %d, %d, %f, %s)(%d)\n", pkWorker, rkWorkData.pWorkData, rkWorkData.bWorkData, rkWorkData.iWorkData, rkWorkData.fWorkData, rkWorkData.strWorkData.c_str(), msg_queue()->message_count());

	if (IsPause())
	{
		pkWorker->DoClientWork(rkWorkData);
		return true;
	}

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;

	pkMsg->iType = E_WTMT_LOAD_XML;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();
	pkMsg->kWorkData = rkWorkData;
	pkMsg->pWorker = pkWorker;

	int iResult = 0;
	switch (ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_CLONE, ePriority));
		}break;
	}

	if(-1 != iResult)
	{
		return true;
	}

	return false;
}

/*
bool PgClientWorkerThread::DoCloneActor2(std::string& rkKFMPath, BM::GUID& rkGuid, PgIWorldObject* pkActor, float fScale, bool bSubAM, EWorkPriority ePriority)
{
	if (pkActor == NULL || IsStop() || IsPause() || rkGuid == BM::GUID::NullData() || g_iUseAddUnitThread != 2 || rkKFMPath.empty())
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoCloneActor2(%s, %s, %d, %d)\n", rkKFMPath.c_str(), MB(rkGuid.str()), fScale, bSubAM);

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();

	pkMsg->iType = E_WTMT_CLONE_ACTOR2;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();
	pkMsg->pWorker= (void*)pkActor;
	pkMsg->pWorkData = (void*)pkActor;
	pkMsg->kGuid = rkGuid;
	pkMsg->strWorkString = UNI(rkKFMPath);
	pkMsg->bWorkData = bSubAM;
	pkMsg->fWorkData = fScale;

	int iResult = 0;
	switch(ePriority)
	{
	case E_PRIORITY_HEAD:
		{
			iResult = msg_queue()->enqueue_head(pkMsg, 0);
		}break;
	case E_PRIORITY_TAIL:
		{
			iResult = msg_queue()->enqueue_tail(pkMsg, 0);
		}break;
	default:
		{
			iResult = msg_queue()->enqueue_prio(pkMsg, 0, adjustPriority(E_BASE_PRIORITY_CLONE, ePriority));
		}break;
	}

	if (-1 != iResult)
	{
		return true;
	}

	return false;
}

bool PgClientWorkerThread::DoSaveScreenShot(LPDIRECT3DSURFACE9 screenSurf, const wchar_t* pcFileName, EWorkPriority ePriority)
{
	if (screenSurf == NULL || pcFileName == NULL)
	{
		return false;
	}

	NILOG(PGLOG_THREAD, "[PgWorkerThread] DoSaveScreenShot\n");

	SWorkerThreadMessage *pkMsg = new SWorkerThreadMessage;
//	pkMsg->Clear();

	pkMsg->iType = E_WTMT_SCREEN_SHOT;
	pkMsg->fWorkQueueTime = NiGetCurrentTimeInSec();
	pkMsg->pWorkData = (void*)screenSurf;
	pkMsg->strWorkString = pcFileName;

	int iResult = msg_queue()->enqueue_prio(pkMsg, 0, E_BASE_PRIORITY_DELETE);
	if(-1 != iResult)
	{
		return true;
	}

	return false;
}
*/

int PgClientWorkerThread::svc(void)
{
	m_bIsWorking = true;
	PG_TRY_BLOCK
	
	while( !IsStop() )//! stop 시그날 걸어서 꺼줄것.
	{
		SWorkerThreadMessage *pkMsg = NULL;

		// TODO: IsPaused 체크해서 쉴것.
		ACE_Time_Value tv(1, 300);	tv += ACE_OS::gettimeofday();
		if( -1 != msg_queue()->dequeue( pkMsg, &tv ) )
		{
			::InterlockedIncrement(&m_iCurrentWorkingTask);
			NILOG(PGLOG_THREAD, "[PgWorkerThread] %d work start, %d task remains\n", pkMsg->iType, msg_queue()->message_count());
			float fTime = 0;
			fTime = NiGetCurrentTimeInSec();
			int iSleepCount = 0;

			if (ms_bUseSleepControl)
			{
				bool bTimeCheck = (BM::GetTime32() - m_dwLastWorkTime <= 10);
				bool bFrameCheck = (m_dwLastWorkFrame == g_pkApp->GetFrameCount());
				int iSleepTime = 0;
				
				if(bFrameCheck)
				{
					iSleepTime = 20;
				}
				else if (bTimeCheck)
				{
					iSleepTime = 5;
				}else
				{
					iSleepTime = 0;	// 아무일 없어도 5ms쯤 주면 더 좋을까?
				}

				if (iSleepTime == 0 || IsStop()) //IsStop()이 켜지면 쉬지말고 일한다;
				{
				//	break;
				}
				else
				{
					iSleepCount++;
					NILOG(PGLOG_THREAD, "[PgWorkerThread](%d) 0x%0X thread sleep start(%d,%d,%d,%d)\n", g_pkApp->GetFrameCount(), this, bTimeCheck, bFrameCheck, iSleepTime, iSleepCount);
					//NiSleep(iSleepTime);
					bool bResult = false; //SwitchToThread();
					NILOG(PGLOG_THREAD, "[PgWorkerThread](%d) 0x%0X thread sleep end(%d)\n", g_pkApp->GetFrameCount(), this, bResult);
				}
			}
			
			bool bDefaultStatCheck = true;
			switch(pkMsg->iType)
			{
			case E_WTMT_DELETE:
				{
					bool bCheckTime = false;
					char statString[256] = { 0, };
					bDefaultStatCheck = false;
					if (pkMsg->pObject->GetRefCount() > 1)
					{
						NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) DeleteObject(0x%08X,%d,%s,%s)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), pkMsg->pObject, pkMsg->pObject->GetRefCount(), pkMsg->pObject->GetRTTI()->GetName(), pkMsg->pObject->GetName());
						//PG_ASSERT_LOG(pkMsg->pObject->GetRefCount() == 1);
					}
					else
					{
						sprintf_s(statString, 256, "[PgWorkerThread](%d,%d) DeleteObject(0x%08X,%d,%s,%s)", g_pkApp->GetFrameCount(), GetCurrentThreadId(),
							pkMsg->pObject, pkMsg->pObject->GetRefCount(), pkMsg->pObject->GetRTTI()->GetName(), pkMsg->pObject->GetName());
						bCheckTime = true;
					}

					fTime = NiGetCurrentTimeInSec();
					pkMsg->pObject = NULL;
					fTime = NiGetCurrentTimeInSec() - fTime;
					if (bCheckTime)
					{
						UpdateStatInfo(m_kThreadWorkStat[pkMsg->iType], fTime);
						NILOG(PGLOG_STAT, "%s - %f, %f\n", statString, fTime, NiGetCurrentTimeInSec() - pkMsg->fWorkQueueTime);
					}
				}break;
			case E_WTMT_ITEM_EQUIP:
				{
					BM::GUID const &rkGuid = pkMsg->kGuid;
					const ItemEquipInfo &rkInfo = pkMsg->kInfo;

					NILOG(PGLOG_THREAD, "[PgWorkerThread] Fetch [%s,%d,%d]\n", MB(rkGuid.str()), rkInfo.iItemNo, rkInfo.bSetToDefault);
					
					fTime = NiGetCurrentTimeInSec();
					bDefaultStatCheck = false;
					// 엑터 가지고 검색을 해야됨. 포인터로 받지마라.

					// ItemDefNo로 부터 PgItemEx 생성부터 하고 Actor 찾자. Actor없으면 그냥 날려 버리면 된다.;
					//! 생성에 실패해도 actor는 한번 찾아야 한다. ItemCount를 decrease하기 위해서
					PgNiFile::SetSilentLoading(true);
					PgItemEx *pkEquipItem = PgItemEx::GetItemFromDef(rkInfo.iItemNo, rkInfo.iGender, rkInfo.iClass);
					PgNiFile::SetSilentLoading(false);
					//PgItemEx *pkEquipItem = NULL;
					
					if(g_pkWorld)
					{
						g_pkWorld->AddItemEquipInfo(rkGuid,rkInfo.iItemNo,rkInfo.bSetToDefault,pkEquipItem);
					}

					fTime = NiGetCurrentTimeInSec() - fTime;
					UpdateStatInfo(m_kThreadWorkStat[pkMsg->iType], fTime);
					if (fTime > 0.1f)
					{
						NILOG(PGLOG_STAT, "[PgWorkerThread] Fetch [%s,%d,%d,%f]\n", MB(rkGuid.str()), rkInfo.iItemNo, rkInfo.bSetToDefault, fTime);
					}
				}break;
			case E_WTMT_ADDUNIT:
				{
					PgWorld* pWorld = (PgWorld*)pkMsg->pWorker;
					CUnit* pUnit = (CUnit*)pkMsg->kWorkData.pWorkData;
					bool bDeleteUnit = false;
					PG_ASSERT_LOG(pUnit);

					// 현재 world와 pWorld를 비교. Addunit이 마무리 되기 전에 world가 바뀔 수 있다.
					if (pWorld != NULL && g_pkWorld != NULL && g_pkWorld == pWorld &&
						pUnit != NULL && g_kPilotMan.IsReservedPilot(pUnit->GetID()) && g_kPilotMan.FindPilot(pUnit->GetID()) == NULL)
					{
						NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) StartAddUnit (%s,%d)(time : %f, %f)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), MB(pUnit->GetID().str()), pUnit->UnitType(), NiGetCurrentTimeInSec() - pkMsg->fWorkQueueTime, NiGetCurrentTimeInSec() - fTime);
						
						bool bNew = true;
						PgNiFile::SetSilentLoading(true);
						PgPilot* pkPilot = PgWorld::CreatePilot(pUnit, bNew);
						if (pkPilot)
						{
							//if (pUnit && pUnit->UnitType() == UT_PLAYER)
							//{
							//	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
							//	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pUnit);

							//	if(pkPlayer && pkActor && pkPlayer->GetMemberGUID() != BM::GUID::NullData())
							//	{
							//		PLAYER_ABIL kInfo;
							//		pkPlayer->GetPlayerAbil(kInfo);

							//		g_kSelectStage.EquipBaseItemFullSet(pkActor, kInfo);

							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR_COLOR, kInfo.iHairColor);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_FACE, kInfo.iFace);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR, kInfo.iHairStyle);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_SHIRTS, kInfo.iJacket);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_PANTS, kInfo.iPants);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_BOOTS, kInfo.iShoes);
							//		pkActor->AddToDefaultItem(EQUIP_LIMIT_GLOVE, kInfo.iGloves);
							//		pkActor->EquipAllItem();
							//	}
							//}
							//else 
							if (pUnit && pUnit->UnitType() == UT_GROUNDBOX)
							{
								PgDropBox *pkBox = dynamic_cast<PgDropBox *>(pkPilot->GetWorldObject());
								PG_ASSERT_LOG(pkBox);
								PgGroundItemBox	*pkBoxUnit = dynamic_cast<PgGroundItemBox*>(pUnit);
								if (pkBox && pkBoxUnit)
								{
									pkBox->RemainTime(pkBoxUnit->CollectRemainTime() / 1000);
									pkBox->IsMine(pkMsg->kWorkData.bWorkData);
									PgBase_Item kItem;
									if ( S_OK == pkBoxUnit->PopItem(kItem, true) )
									{
										pkBox->SetItemNum(kItem);
									}
								}
							}
						}
						PgNiFile::SetSilentLoading(false);
						if (pkPilot)
						{
							NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) AddUnit (%s,%d)(time : %f, %f)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), MB(pUnit->GetID().str()), pUnit->UnitType(), NiGetCurrentTimeInSec() - pkMsg->fWorkQueueTime, NiGetCurrentTimeInSec() - fTime);
							BM::CAutoMutex kLock(g_kWorldLock);
							if (NULL != g_pkWorld 
								&& g_pkWorld == pWorld)
							{
								NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) AddUnit Enqueued(%s,%d)(time : %f, %f)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), MB(pUnit->GetID().str()), pUnit->UnitType(), NiGetCurrentTimeInSec() - pkMsg->fWorkQueueTime, NiGetCurrentTimeInSec() - fTime);
								pWorld->EnqueueAddUnitInfo(pUnit, pkPilot,pkMsg->fWorkQueueTime);
							}
							else
							{
								PgWorld::DeleteAddUnitInfo(pUnit, pkPilot);
							}
						}
						else
						{
							NILOG(PGLOG_THREAD, "[PgWorkerThread] Reserve Delete Unit because CreatePilot Failed\n");
							bDeleteUnit = true;
						}
					}
					else
					{
						// 월드가 바뀌었거나 ReservedPilot이 아니거나, 이미 들어 있는 GUID라면, 그냥 지우고 넘어간다.
						NILOG(PGLOG_THREAD, "[PgWorkerThread] Reserve Delete Unit because not correct status(%d, %d, %d, %d, %d, %d)\n", pWorld == NULL, g_pkWorld == NULL, g_pkWorld != pWorld, pUnit == NULL, g_kPilotMan.IsReservedPilot(pUnit->GetID()) == false, g_kPilotMan.FindPilot(pUnit->GetID()) != NULL);
						bDeleteUnit = true;
					}

					if (pUnit && bDeleteUnit)
					{
						NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) Delete Unit(%s,%d)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), MB(pUnit->GetID().str()), pUnit->UnitType());
						g_kPilotMan.RemoveReservedPilot(pUnit->GetID());
						g_kTotalObjMgr.UnRegistUnit(pUnit);
						g_kTotalObjMgr.ReleaseUnit(pUnit);
					}
				}break;
			case E_WTMT_CLONE_ACTOR:
				{
					stAMPoolInfo* pAMPoolInfo = (stAMPoolInfo*)pkMsg->kWorkData.pWorkData;
					if (pAMPoolInfo && pAMPoolInfo->kAMList.size() < pAMPoolInfo->iMaxCount)
					{
						NILOG(PGLOG_THREAD, "[PgWorkerThread](%d,%d) Clone Actor(%s,%d)\n", g_pkApp->GetFrameCount(), GetCurrentThreadId(), pAMPoolInfo->strKFMPath.c_str(), pAMPoolInfo->kAMList.size());
						BM::CAutoMutex kLock(pAMPoolInfo->kPoolLock);
						NiActorManager* pOriginal = NULL;
						NiActorManager* pClonned = NULL;
						if (pAMPoolInfo->kAMList.size() > 0)
						{
							pOriginal = pAMPoolInfo->kAMList.front();
							if (pOriginal)
							{
								pClonned = pOriginal->Clone();
							}
						}
						else
						{
							// 이러면 안된다; Original은 있어야지.
							PG_ASSERT_LOG(0);
						}
						if (pClonned)
						{
							pAMPoolInfo->kAMList.push_back(pClonned);
						}
						pAMPoolInfo->fLastWorkTime = NiGetCurrentTimeInSec();
					}
				}break;
			case E_WTMT_LOAD_XML:
			case E_WTMT_LOAD_SOUND:
			case E_WTMT_LOAD_EFFECT:
				{
					if (pkMsg->pWorker != NULL)
					{
						PgIClientWorker* pkWorker = (PgIClientWorker*)pkMsg->pWorker;
						pkWorker->DoClientWork(pkMsg->kWorkData);
					}
				}
				break;
			default:
				{
					PG_ASSERT_LOG( 0 && "THREAD Asserted Unknown MessageType");
				}break;
			}

			if (bDefaultStatCheck && pkMsg->iType > E_WTMT_NONE && pkMsg->iType < MAX_WTMT_NUM)
			{
				fTime = NiGetCurrentTimeInSec() - fTime;
				UpdateStatInfo(m_kThreadWorkStat[pkMsg->iType], fTime);
			}
			SAFE_DELETE(pkMsg);
			m_dwLastWorkTime = BM::GetTime32();
			m_dwLastWorkFrame = g_pkApp->GetFrameCount();
			::InterlockedDecrement(&m_iCurrentWorkingTask);
		}
	}
	PG_CATCH_BLOCK	

	if (m_iCurrentWorkingTask > 0)
	{
		NILOG(PGLOG_WARNING, "[PgWorkerThread] when thread stop, task count is %d\n", m_iCurrentWorkingTask);
	}
	m_iCurrentWorkingTask = 0;
	m_bIsWorking = false;
	return 1;
}

void PgClientWorkerThread::PrintStatInfo()
{
	NILOG(PGLOG_STAT, "[PgWorkerThread] WorkerThread Stat -----\n");
	for (int i = 0; i < MAX_WTMT_NUM; i++)
	{
		float avg1 = m_kThreadWorkStat[i].iCount > 0 ? m_kThreadWorkStat[i].fTotalTime / m_kThreadWorkStat[i].iCount : 0.0f;
		float avg2 = m_kThreadWorkStat[i].iCount > 2 ? (m_kThreadWorkStat[i].fTotalTime - m_kThreadWorkStat[i].fMinTime - m_kThreadWorkStat[i].fMaxTime) / (m_kThreadWorkStat[i].iCount - 2) : 0.0f; 
		NILOG(PGLOG_STAT, "\t\t %d Work %d times called, call time %f min %f, max %f, avg1 %f, avg2 %f\n", i, m_kThreadWorkStat[i].iCount, m_kThreadWorkStat[i].fTotalTime, m_kThreadWorkStat[i].fMinTime, m_kThreadWorkStat[i].fMaxTime, avg1, avg2);
	}
}

EBasePriority PgClientWorkerThread::adjustPriority(EBasePriority eOriginalPrio, EWorkPriority eWorkPrio)
{
	switch(eWorkPrio)
	{
	case E_PRIORITY_TAIL:
	case E_PRIORITY_HEAD:
	case E_PRIORITY_NORMAL:
		{
			return eOriginalPrio;
		}break;
	case E_PRIORITY_LOWEST:
	case E_PRIORITY_LOW:
	case E_PRIORITY_HIGH:
	case E_PRIORITY_HIGHEST:
		{
			return (EBasePriority)(eOriginalPrio + (int)eWorkPrio);
		}break;
	default:
		{
			PG_ASSERT_LOG(0);
		}break;
	}
	return eOriginalPrio;
}
