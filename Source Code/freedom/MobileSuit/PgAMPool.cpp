#include "StdAfx.h"
#include "PgAMPool.h"
#include "PgMobileSuit.h"
#include "PgActorManager.H"

#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"

#define PG_MAX_NB_CLONES		5	// 최대로 Clonning할 수
/*
PgAMPool::PgAMPool()
#ifdef PG_AMPOOL_VER2
#endif
{
	m_pkClonningThread = 0;	
	m_fLastLoopTime = 0;
	m_kClonningThreadProc.m_bTerminate = true;
#ifdef PG_AMPOOL_VER2
	m_kClonningThreadProc.m_pkCacheContainer = &m_kCacheContainer;
	m_kClonningThreadProc.m_pkCacheLock = &m_kCacheLock;
#endif
}

PgAMPool::~PgAMPool()
{
}

void PgAMPool::Destroy()
{
	if(m_pkClonningThread)
	{
		m_kClonningThreadProc.m_bTerminate = true;
		m_pkClonningThread->Shutdown();
	}

	for(AMCacheContainer::iterator itr = m_kCacheContainer.begin();
		itr != m_kCacheContainer.end();
		itr++)
	{
#if defined (PG_AMPOOL_VER1) || defined (PG_AMPOOL_VER2)
		if (itr->second == NULL)
			continue;

		for(AMPtrList::iterator itrL = itr->second->kAMList.begin(); itrL != itr->second->kAMList.end(); ++itrL)
#else
		for(AMPtrList::iterator itrL = itr->second.begin(); itrL != itr->second.end(); ++itrL)
#endif
		{
			SAFE_DELETE_NI(*itrL);
		}
#if defined (PG_AMPOOL_VER1) || defined (PG_AMPOOL_VER2)
		SAFE_DELETE(itr->second);
#endif
	}
}

void PgAMPool::Update(float fAccumTime)
{
#if defined (PG_AMPOOL_VER2)
	//! 주기적으로 TaskQueue가 남아 있으면 DoLoop를 부른다.
	if (m_pkClonningThread != NULL && m_kClonningThreadProc.m_bTerminate)
	{
		if (m_kClonningThreadProc.m_kTaskQueue[0].size() > 0 || 
			m_kClonningThreadProc.m_kTaskQueue[1].size() > 0 || 
			BM::GetTime32() - m_fLastLoopTime > 1000)
		{
			m_pkClonningThread->DoLoop();
			m_fLastLoopTime = BM::GetTime32();
		}
	}
#endif
}

void PgAMPool::SuspendClonningThread()
{
	if (m_pkClonningThread)
	{
		if (m_pkClonningThread->GetStatus() == NiThread::RUNNING)
			m_pkClonningThread->Suspend();
	}
}

void PgAMPool::ResumeClonningThread()
{
	if (m_pkClonningThread)
	{
		if (m_pkClonningThread->GetStatus() == NiThread::SUSPENDED)
			m_pkClonningThread->Resume();
	}
}

void PgAMPool::RemoveClonnedActorManager(NiActorManagerPtr pActorManager)
{
	//! 어느 kfmPath인지 어떻게 알아낼 것인가?
}

#if defined (PG_AMPOOL_VER1)
NiActorManagerPtr PgAMPool::LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID)
{
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoadActorManager %s,%d\n", pcKFMPath, eObjectID);
	if(!m_pkClonningThread)
	{
		m_pkClonningThread = NiLoopedThread::Create(&m_kClonningThreadProc);
		m_pkClonningThread->SetPriority(NiThread::BELOW_NORMAL);
		NiProcessorAffinity kAffinity(NiProcessorAffinity::PROCESSOR_1, NiProcessorAffinity::PROCESSOR_DONT_CARE);
		m_pkClonningThread->SetThreadAffinity(kAffinity);
		m_pkClonningThread->Resume();
		m_pkClonningThread->DoLoop();
	}

	bool bFirst = false;
	m_kClonningLock.Lock();
	AMCacheContainer::iterator itr = m_kCacheContainer.find(pcKFMPath);
	if(itr == m_kCacheContainer.end())
	{
		NILOG(PGLOG_THREAD, "[PgAMPool]\tFirst Loading AM of %s\n", pcKFMPath);
		bFirst = true;
		m_kClonningLock.Unlock();
		NiActorManagerPtr pkAM = NiActorManager::Create(pcKFMPath);

		if (pkAM == NULL)
			return 0;

		stAMPoolInfo* pAMInfo = new stAMPoolInfo;
		PG_ASSERT_LOG(pAMInfo);

		pAMInfo->eObjectID = eObjectID;
		switch(eObjectID)
		{
		case PgIXmlObject::ID_PC:
			pAMInfo->iMinCount = 5;
			break;
		case PgIXmlObject::ID_NPC:
			pAMInfo->iMinCount = 0;
			break;
		case PgIXmlObject::ID_MONSTER:
			pAMInfo->iMinCount = 5;
			break;
		case PgIXmlObject::ID_PET:
			pAMInfo->iMinCount = 5;
			break;
		case PgIXmlObject::ID_BOSS:
			pAMInfo->iMinCount = 0;
			break;
		case PgIXmlObject::ID_FURNITURE:
			pAMInfo->iMinCount = 0;
			break;
		case PgIXmlObject::ID_DROPBOX:
			pAMInfo->iMinCount = 5;
			break;
		case PgIXmlObject::ID_ITEM:
			pAMInfo->iMinCount = 2;
			break;
		default:
			pAMInfo->iMinCount = 2;
			break;
		}

		if (pAMInfo->iMinCount <= 1)
		{
			SAFE_DELETE(pAMInfo);
			return pkAM;
		}

		pAMInfo->eObjectID = eObjectID;
		pAMInfo->kAMList.push_back(pkAM);
		pAMInfo->iTotalCount = 1;
		pAMInfo->iCurCount = 0;
		pAMInfo->strKFMPath = pcKFMPath;
		std::pair<AMCacheContainer::iterator, bool> ret;
		m_kClonningLock.Lock();
		ret = m_kCacheContainer.insert(std::make_pair(pcKFMPath, pAMInfo));

		if (ret.second == false)
		{
			SAFE_DELETE_NI(pkAM);
			SAFE_DELETE(pAMInfo);
			m_kClonningLock.Unlock();
			return 0;
		}

		itr = ret.first;
	}
	m_kClonningLock.Unlock();

	NILOG(PGLOG_THREAD, "[PgAMPool]\t Before GetClonnedActorManager(%d)\n", itr->second->kAMList.size());
	return GetClonnedActorManager(itr, bFirst);
}

NiActorManagerPtr PgAMPool::GetClonnedActorManager(AMCacheContainer::iterator iter, bool bFirst)
{
	static DWORD dwTotalWaitingTime = 0;
	// Caching 일어날 때 까지 대기한다.
	bool bFirstLog = true;
	int iWaitingCount = 0;
	DWORD dwWaitingTime = 0;

	static DWORD dwWaitingTicketCount = 0;
	while (iter->second->kAMList.size() < 2)
	{
		if (m_kClonningThreadProc.m_bTerminate)
		{
			m_pkClonningThread->DoLoop();
		}
		if (bFirstLog)
		{
			NILOG(PGLOG_THREAD, "[PgAMPool]\t Waiting inside GetClonnedActorManager(%d)- ReqNum: %d\n", iter->second->kAMList.size(), dwWaitingTicketCount);
			bFirstLog = false;
			dwWaitingTime = BM::GetTime32();
			dwWaitingTicketCount++;
		}
		iWaitingCount++;
		NiSleep(0);
	}

	if (iWaitingCount > 0)
	{
		dwTotalWaitingTime += (BM::GetTime32() - dwWaitingTime);
		NILOG(PGLOG_THREAD, "[PgAMPool]\tWaiting for %s inside %d count, %d tick(accum %d)\n", iter->second->strKFMPath.c_str(), iWaitingCount, BM::GetTime32() - dwWaitingTime, dwTotalWaitingTime);
	}

	// 제일 끝에 것을 뽑자
	iter->second->kPoolLock.Lock();
	NiActorManagerPtr pkAM = iter->second->kAMList.back();
	iter->second->kAMList.pop_back();
	iter->second->kPoolLock.Unlock();

	NILOG(PGLOG_THREAD, "[PgAMPool]\t GetClonnedActorManager returned(%d)\n", iter->second->kAMList.size());
	PG_ASSERT_LOG(pkAM);
	return pkAM;
}

bool PgAMPool::ClonningThreadProc::LoopedProcedure(void* pvArg)
{
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoopProdecure Started\n");
	m_bTerminate = false;
	bool bRemain = true;

	PG_TRY_BLOCK
	do
	{
		bRemain = false;
		AMCacheContainer *pkCacheContainer = &g_kAMPool.m_kCacheContainer;

		g_kAMPool.m_kClonningLock.Lock();
		for(AMCacheContainer::iterator itr = pkCacheContainer->begin();
			itr != pkCacheContainer->end();
			itr++)
		{
			//! 외부에서 강제 종료 시키는 것 체크
			if (m_bTerminate)
				break;

			unsigned int iClonnedAMCount = itr->second->kAMList.size();
			if(iClonnedAMCount < itr->second->iMinCount)
			{
				g_kAMPool.m_kClonningLock.Unlock();
				itr->second->kPoolLock.Lock();

				//NiActorManagerPtr pkOri = itr->second->kAMList.front();
				NiActorManagerPtr pkOri = NULL;
				pkOri = itr->second->kAMList.front();

				NiActorManagerPtr pkAM = NULL;
				if (pkOri)
				{
					pkAM = pkOri->Clone();
					itr->second->kPoolLock.Unlock();
				}
				else
				{
					itr->second->kPoolLock.Unlock();
					continue;
				}

				NILOG(PGLOG_THREAD, "[PgAMPool]\t%s AM Clonned\n", itr->second->strKFMPath.c_str());
				if (iClonnedAMCount > 2)
				{
					NiSleep(0); //! 중간에 잠깐 쉬려고 하는데 이러면 어떻게 될까 모르겠네..
					bRemain = true;
				}
				g_kAMPool.m_kClonningLock.Lock();
				itr->second->kAMList.push_back(pkAM);
				NILOG(PGLOG_THREAD, "[PgAMPool]\t%s AM added(%d)\n", itr->second->strKFMPath.c_str(), itr->second->kAMList.size());				
			}
		}
		g_kAMPool.m_kClonningLock.Unlock();
		NILOG(PGLOG_THREAD, "[PgAMPool]\tOneLoopProdecure Done, waiting\n");
		NiSleep(0);
	} while (bRemain && m_bTerminate == false);
	PG_CATCH_BLOCK

	m_bTerminate = true;
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoopProdecure Ended\n");
	return true;
}

#elif defined (PG_AMPOOL_VER2)

NiActorManagerPtr PgAMPool::LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID, bool bPrepareOnly)
{
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoadActorManager %s,%d\n", pcKFMPath, eObjectID);
	if(!m_pkClonningThread)
	{
		m_pkClonningThread = NiLoopedThread::Create(&m_kClonningThreadProc);
		m_pkClonningThread->SetPriority(NiThread::LOWEST);
		NiProcessorAffinity kAffinity(NiProcessorAffinity::PROCESSOR_1, NiProcessorAffinity::PROCESSOR_DONT_CARE);
		m_pkClonningThread->SetThreadAffinity(kAffinity);
		m_pkClonningThread->Resume();		
	}

	stAMPoolInfo* pkAmPoolInfo = NULL;

{//아래에 WriteLock 있으므로 풀지말것.
	BM::CAutoMutex kLock(m_kCacheLock, true);//이터레이터를 계속 사용하기 때문에 WriteLock
	AMCacheContainer::iterator itr = m_kCacheContainer.find(pcKFMPath);
	if(itr != m_kCacheContainer.end())
	{
		pkAmPoolInfo = itr->second;
	}
}
	if(!pkAmPoolInfo)
	{
		NILOG(PGLOG_THREAD, "[PgAMPool]\tFirst Loading AM of %s\n", pcKFMPath);
		unsigned int iMinCount = 0;

		switch(eObjectID)
		{
		case PgIXmlObject::ID_PC:
			iMinCount = 2;
			break;
		case PgIXmlObject::ID_NPC:
			iMinCount = 1;
			break;
		case PgIXmlObject::ID_MONSTER:
			iMinCount = 1;
			break;
		case PgIXmlObject::ID_PET:
			iMinCount = 2;
			break;
		case PgIXmlObject::ID_BOSS:
			iMinCount = 0;
			break;
		case PgIXmlObject::ID_FURNITURE:
			iMinCount = 0;
			break;
		case PgIXmlObject::ID_DROPBOX:
			iMinCount = 2;
			break;
		case PgIXmlObject::ID_ITEM:
			iMinCount = 2;
			break;
		default:
			NILOG(PGLOG_WARNING, "[PgAMPool] Unknown ObjectID(%d, %s)\n", eObjectID, pcKFMPath);
			iMinCount = 2;
			break;
		}

		if (iMinCount < 1)
		{
			NiActorManagerPtr pkAM = PgActorManager::CreatePG(pcKFMPath);
			return pkAM;
		}		

		stAMPoolInfo* pAMInfo = new stAMPoolInfo;
		PG_ASSERT_LOG(pAMInfo);

		pAMInfo->iMinCount = iMinCount;
		pAMInfo->eObjectID = eObjectID;
		pAMInfo->iTotalCount = 0;
		pAMInfo->iCurCount = 0;
		pAMInfo->iReservedCount = 0;
		pAMInfo->iRequestCount = 0;
		pAMInfo->fLastServedTime = 0.0f;
		pAMInfo->fLastReturnTime = 0.0f;
		pAMInfo->fLastWorkTime = 0.0f;
		pAMInfo->strKFMPath = pcKFMPath;
		pAMInfo->bLoadFailed = false;

		if (bPrepareOnly)
			pAMInfo->dwRequestTime = 0; // prepare only면.. requestTime을 0으로 만들어서 뒤로 가게 만든다.
		else
			pAMInfo->dwRequestTime = BM::GetTime32();
		std::pair<AMCacheContainer::iterator, bool> ret;

		BM::CAutoMutex kLock(m_kCacheLock, true);
		ret = m_kCacheContainer.insert(std::make_pair(pcKFMPath, pAMInfo));

		if (ret.second == false)
		{
			NILOG(PGLOG_ERROR, "[PgAMPool] AMInfo insert failed\n");
			SAFE_DELETE(pAMInfo);			
			return NULL;
		}

		pkAmPoolInfo = ret.first->second;
	}

	if (bPrepareOnly)
	{
		return NULL;
	}

	NILOG(PGLOG_THREAD, "[PgAMPool]\t Before GetClonnedActorManager(%d)\n", pkAmPoolInfo->kAMList.size());
	return GetClonnedActorManager(pkAmPoolInfo);
}

NiActorManagerPtr PgAMPool::GetClonnedActorManager(stAMPoolInfo* pkAmPoolInfo)
{
	static DWORD dwTotalWaitingTime = 0;
	// Caching 일어날 때 까지 대기한다.
	bool bFirst = true;
	unsigned int iWaitingCount = 0;
	DWORD dwWaitingTime = 0;

	static DWORD dwWaitingTicketCount = 0;
	//! Size를 읽어올때 Lock을 걸어야 할까?
	while (pkAmPoolInfo->kAMList.size() < (size_t)NiMin(2, pkAmPoolInfo->iMinCount) && pkAmPoolInfo->bLoadFailed == false)
	{
		if (m_kClonningThreadProc.m_bTerminate)
		{
			m_pkClonningThread->DoLoop();
			m_fLastLoopTime = BM::GetTime32();
		}

		if (bFirst)
		{
			NILOG(PGLOG_THREAD, "[PgAMPool]\t Waiting inside GetClonnedActorManager(%d) : ReqNum : %d\n", pkAmPoolInfo->kAMList.size(), dwWaitingTicketCount);
			pkAmPoolInfo->iRequestCount++;
			pkAmPoolInfo->dwRequestTime = BM::GetTime32();
			dwWaitingTime = BM::GetTime32();
			bFirst = false;
			dwWaitingTicketCount++;
		}		
		NiSleep(NiMin(iWaitingCount, 30));
		iWaitingCount++;
	}	

	if (iWaitingCount > 0)
	{
		dwTotalWaitingTime += (BM::GetTime32() - dwWaitingTime);
		NILOG(PGLOG_THREAD, "[PgAMPool]\t Waiting for %s inside %d count, %d tick(accum %d)\n", pkAmPoolInfo->strKFMPath.c_str(), iWaitingCount, BM::GetTime32() - dwWaitingTime, dwTotalWaitingTime);
	}

	if (pkAmPoolInfo->bLoadFailed)
		return NULL;	

	pkAmPoolInfo->dwRequestTime = 0;
	pkAmPoolInfo->fLastServedTime = NiGetCurrentTimeInSec();
	// 제일 끝에 것을 뽑자
	pkAmPoolInfo->kPoolLock.Lock();
	NiActorManagerPtr pkAM = pkAmPoolInfo->kAMList.back();
	pkAmPoolInfo->kAMList.pop_back();
	pkAmPoolInfo->kPoolLock.Unlock();

	NILOG(PGLOG_THREAD, "[PgAMPool]\t GetClonnedActorManager returned(%d)\n", pkAmPoolInfo->kAMList.size());
	return pkAM;
}

bool PgAMPool::ClonningThreadProc::LoopedProcedure(void* pvArg)
{
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoopProdecure Started\n");
	m_bTerminate = false;
	PG_TRY_BLOCK
	int iProcessingQueue = InvalidateTaskQueue();

	//! Task를 소비한다.	

	//for(AMCloneTaskQueue::iterator itr = m_kTaskQueue.begin(); itr != m_kTaskQueue.end(); ++itr)	
	if (iProcessingQueue >= 0 && iProcessingQueue < MAX_TASK_QUEUE_COUNT)
	{
		stAMPoolInfo* pPoolInfo = m_kTaskQueue[iProcessingQueue].front();

		if (pPoolInfo != NULL)
		{
			NILOG(PGLOG_THREAD, "[PgAMPool]\t%s AM Clonning Start\n", pPoolInfo->strKFMPath.c_str());
			NiActorManagerPtr pClonnedAM = NULL;
			if (pPoolInfo->kAMList.size() <= 0)
			{
				pClonnedAM = NiActorManager::Create(pPoolInfo->strKFMPath.c_str());				
			}
			else
			{
				pPoolInfo->kPoolLock.Lock();
				NiActorManagerPtr pkOri = pPoolInfo->kAMList.front();
				pClonnedAM = pkOri->Clone();
				pPoolInfo->kPoolLock.Unlock();
			}

			PG_ASSERT_LOG(pClonnedAM);
			if (pClonnedAM != NULL)
			{
				NILOG(PGLOG_THREAD, "[PgAMPool]\t%s AM Clonned\n", pPoolInfo->strKFMPath.c_str());
				NiSleep(0); //! 중간에 잠깐 쉬려고 하는데 이러면 어떻게 될까 모르겠네..

				pPoolInfo->kPoolLock.Lock();
				pPoolInfo->kAMList.push_back(pClonnedAM);
				pPoolInfo->kPoolLock.Unlock();
				pPoolInfo->iCurCount++;
				pPoolInfo->iTotalCount++;
				pPoolInfo->iReservedCount--;
				pPoolInfo->bLoadFailed = false;
				NILOG(PGLOG_THREAD, "[PgAMPool]\t%s AM added(%d)\n", pPoolInfo->strKFMPath.c_str(), pPoolInfo->kAMList.size());
			}
			else
			{
				pPoolInfo->bLoadFailed = true;
			}
		}
		m_kTaskQueue[iProcessingQueue].pop_front();
	}
	PG_CATCH_BLOCK

	m_bTerminate = true;
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoopProdecure Ended\n");
	return true;
}

int PgAMPool::ClonningThreadProc::InvalidateTaskQueue()
{
	//! InvalidateTaskQueue가 LoopedProcedure안에서 불리는 동안에는 TaskQueueLock을 걸 필요가 없다.
	PG_ASSERT_LOG(m_pkCacheContainer);
	PG_ASSERT_LOG(m_pkCacheLock);
	unsigned int i = 0;
	int iProcessingQueue = -1; //! -1은 아무것도 안한다는 것임.

#ifdef PG_LOG_ENABLED
	unsigned int iFrontAddCount[MAX_TASK_QUEUE_COUNT];
	unsigned int iBackAddCount[MAX_TASK_QUEUE_COUNT];
	unsigned int iCurrentCount[MAX_TASK_QUEUE_COUNT];

	for (i = 0; i < MAX_TASK_QUEUE_COUNT; i++)
	{
		iCurrentCount[i] = m_kTaskQueue[i].size();
		iFrontAddCount[i] = 0;
		iBackAddCount[i] = 0;
	}
#endif

	{
	//! Task를 채운다.
	BM::CAutoMutex kLock(*m_pkCacheLock);

	for(AMCacheContainer::iterator itr = m_pkCacheContainer->begin(); itr != m_pkCacheContainer->end(); ++itr)
	{
		itr->second->kPoolLock.Lock();
		unsigned int iPoolCount = itr->second->kAMList.size();
		itr->second->kPoolLock.Unlock();

		unsigned int iPriority = NiMin(iPoolCount, MAX_TASK_QUEUE_COUNT - 1); //! Pool에 차 있는 많큼 시작 Priority가 낮아진다.

		int iRemains = itr->second->iMinCount - iPoolCount - itr->second->iReservedCount;
#ifdef PG_LOG_ENABLED
		NILOG(PGLOG_THREAD, "[PgAMPool] %s (min:%d, cur:%d, total:%d, reserv:%d, req:%d, reqTime:%d, remain:%d, prio:%d)\n", itr->second->strKFMPath.c_str(),
			itr->second->iMinCount, itr->second->iCurCount, itr->second->iTotalCount, itr->second->iReservedCount, itr->second->iRequestCount, itr->second->dwRequestTime,
			iRemains, iPriority);
#endif
		if (iRemains <= 0)
		{
			if (itr->second->iRequestCount > 0)
			{
				//! Request가 들어왔기 때문에 이미 priority가 낮은 쪽에 다 Reserve는 되어있어서 Clonning이 늦어지는 경우가 있기 때문에
				//! 여기에서 Priority를 조정하거나, 일단 priority가 높은 곳에 더 넣어주는게 좋겠다. 임시로 넣고 나서는 iRequestCount를 수정해야 한다.
				PG_ASSERT_LOG(itr->second->iRequestCount <= 5);
				m_kTaskQueue[0].push_front(itr->second);
#ifdef PG_LOG_ENABLED
				iFrontAddCount[0]++;
#endif
				itr->second->iRequestCount--;
				if (itr->second->iRequestCount < 0)
					itr->second->iRequestCount = 0;
			}
			continue;
		}

		if (iPoolCount < 2)
		{
			//! TODO : Priority 1번/2번에 넣을 때에는, 
			//!			iMinCount와 dwRequestTime을 이용해서 insert를 하는게 Performance가 더 좋다.
			for (unsigned int i = 0; i < 2 - iPoolCount; i++)
			{
				if (itr->second->dwRequestTime != 0)
				{
					int count = 0;
					bool bInserted = false;
					for (AMCloneTaskQueue::iterator iter = m_kTaskQueue[iPriority].begin(); iter != m_kTaskQueue[iPriority].end(); ++iter)
					{
						if ((*iter)->dwRequestTime != 0 && (*iter)->dwRequestTime < itr->second->dwRequestTime)
						{
							count++;
							continue;
						}

						m_kTaskQueue[iPriority].insert(iter, itr->second);
						bInserted = true;
						NILOG(PGLOG_THREAD, "[PgAMPool] Insert %d queue %d position %d reqTime\n", iPriority, count, itr->second->dwRequestTime);
						break;
					}
					if (bInserted == false)
						m_kTaskQueue[iPriority].push_back(itr->second);
#ifdef PG_LOG_ENABLED
					iFrontAddCount[iPriority]++;
#endif
				}
				else
				{
					m_kTaskQueue[iPriority].push_back(itr->second);
#ifdef PG_LOG_ENABLED
					iBackAddCount[iPriority]++;
#endif
				}

				iRemains--;
				
				itr->second->iReservedCount++;
				iPriority = NiMin(MAX_TASK_QUEUE_COUNT - 1, iPriority + 1);
			}
		}

		if (iRemains > 0)
		{
			for (int i = 0; i < iRemains; i++)
			{
				m_kTaskQueue[iPriority].push_back(itr->second);
#ifdef PG_LOG_ENABLED
				iBackAddCount[iPriority]++;
#endif
				itr->second->iReservedCount++;
				iPriority = NiMin(MAX_TASK_QUEUE_COUNT - 1, iPriority + 1);
			}
		}		
	}
	}

	for (i = 0; i < MAX_TASK_QUEUE_COUNT; i++)
	{
#ifdef PG_LOG_ENABLED
		if (iCurrentCount[i] > 0 || iFrontAddCount[i] > 0 || iBackAddCount[i] > 0 || m_kTaskQueue[i].size() > 0)
		{
			NILOG(PGLOG_THREAD, "[PgAMPool] InvalidateTaskQueue[%d](%d, %d, %d, %d)\n", i,
				iCurrentCount[i], iFrontAddCount[i], iBackAddCount[i], m_kTaskQueue[i].size());
			int j = 0;
			for (AMCloneTaskQueue::iterator iter = m_kTaskQueue[i].begin(); iter != m_kTaskQueue[i].end(); ++iter)
			{
				NILOG(PGLOG_THREAD, "[PgAMPool] \tTaskQueue[%d](%d, %s)\n", i, j, (*iter)->strKFMPath.c_str());
				j++;
			}
		}
#endif

		if (iProcessingQueue < 0 && m_kTaskQueue[i].size() > 0)
			iProcessingQueue = i;
	}

	return iProcessingQueue;
}

#else //! Original Version

NiActorManagerPtr PgAMPool::LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID)
{
#ifdef PG_USE_WORKER_THREAD
	NILOG(PGLOG_THREAD, "[PgAMPool]\tLoadActorManager %s,%d\n", pcKFMPath, eObjectID);
	if(!m_pkClonningThread)
	{
		m_pkClonningThread = NiLoopedThread::Create(&m_kClonningThreadProc);
		m_pkClonningThread->SetPriority(NiThread::BELOW_NORMAL);
		NiProcessorAffinity kAffinity(NiProcessorAffinity::PROCESSOR_1, NiProcessorAffinity::PROCESSOR_DONT_CARE);
		m_pkClonningThread->SetThreadAffinity(kAffinity);
		m_pkClonningThread->Resume();
		m_pkClonningThread->DoLoop();
	}

	AMCacheContainer::iterator itr = m_kCacheContainer.find(pcKFMPath);
	if(itr == m_kCacheContainer.end())
	{
		BM::CAutoMutex kLock(m_kClonningLock);
#endif
		NiActorManagerPtr pkAM = NiActorManager::Create(pcKFMPath);

		if(!pkAM)
		{
			PG_ASSERT_LOG(0);
			return 0;
		}
#ifdef PG_USE_WORKER_THREAD
		itr = m_kCacheContainer.insert(
			std::make_pair(pcKFMPath, AMPtrList())).first;
		itr->second.push_back(pkAM);
	}

	return GetClonnedActorManager(pcKFMPath);
#else
	return pkAM;
#endif
}

NiActorManagerPtr PgAMPool::GetClonnedActorManager(char const *pcKFMPath)
{
	//	NILOG(PGLOG_THREAD, "[PgAMPool]\tRequest of Clonned AM of %s\n", pcKFMPath);

	if(m_kClonningThreadProc.m_bTerminate)
	{
		m_pkClonningThread->DoLoop();
	}

	g_kAMPool.m_kClonningLock.Lock();
	AMCacheContainer::iterator itr = m_kCacheContainer.find(pcKFMPath);
	g_kAMPool.m_kClonningLock.Unlock();

	// 없을 수 없지만, 그래도 에러 처리를 해주자
	if(itr == m_kCacheContainer.end())
	{
		return 0;
	}

	static DWORD dwTotalWaitingTime = 0;
	DWORD dwWaitingTime = BM::GetTime32();

	// Caching 일어날 때 까지 대기한다.
	//while (itr->second.size() < 2)
	//{
	//	NiSleep(0);
	//}
	NiActorManagerPtr pkAM = NULL;
	while (pkAM == NULL)
	{
		if (itr->second.size() < 2)
			NiSleep(0);
		else
		{
			// 제일 끝에 것을 뽑자
			m_kClonningLock.Lock();
			if (itr->second.size() >= 2)
			{
				pkAM = itr->second.back();
				itr->second.pop_back();
			}
			m_kClonningLock.Unlock();
		}
	}

	dwTotalWaitingTime += (BM::GetTime32() - dwWaitingTime);
	NILOG(PGLOG_THREAD, "[PgAMPool]\tWaiting for %s, %d tick(accum %d)\n", pcKFMPath, BM::GetTime32() - dwWaitingTime, dwTotalWaitingTime);
	// 제일 끝에 것을 뽑자
	//m_kClonningLock.Lock();
	//NiActorManagerPtr pkAM = itr->second.back();
	//itr->second.pop_back();
	//m_kClonningLock.Unlock();

	PG_ASSERT_LOG(pkAM);
	return pkAM;
}

bool PgAMPool::ClonningThreadProc::LoopedProcedure(void* pvArg)
{
	m_bTerminate = false;
	bool bRemain = true;
	PG_TRY_BLOCK
	do
	{
		bRemain = false;
		AMCacheContainer *pkCacheContainer = &g_kAMPool.m_kCacheContainer;

		g_kAMPool.m_kClonningLock.Lock();
		for(AMCacheContainer::iterator itr = pkCacheContainer->begin();
			itr != pkCacheContainer->end();
			itr++)
		{
			if(itr->second.size() < PG_MAX_NB_CLONES)
			{
				g_kAMPool.m_kClonningLock.Unlock();
				NiActorManagerPtr pkOri = itr->second.front();
				NiActorManagerPtr pkAM = NULL;
				if (pkOri)
					pkAM = pkOri->Clone();
				else
					continue;

				g_kAMPool.m_kClonningLock.Lock();
				itr->second.push_back(pkAM);

				bRemain = true;
			}
		}
		g_kAMPool.m_kClonningLock.Unlock();

		NiSleep(0);
	} while(bRemain && !m_bTerminate);
	PG_CATCH_BLOCK

	m_bTerminate = true;
	return true;
}
#endif

void PgAMPool::SetThreadPriority(NiThread::Priority ePriority)
{
	if (m_pkClonningThread)
	{
		m_pkClonningThread->SetPriority(ePriority);
	}
}

NiThread::Priority PgAMPool::GetThreadPriority()
{
	if (m_pkClonningThread)
	{
		return m_pkClonningThread->GetPriority();
	}

	return NiThread::IDLE;
}
*/