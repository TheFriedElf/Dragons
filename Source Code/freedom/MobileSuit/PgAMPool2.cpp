#include "StdAfx.h"
#include "PgAMPool2.h"
#include "PgMobileSuit.h"
#include "PgWorkerThread.h"
#include "PgActorManager.H"
#include "PgNiFile.h"
#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"
#include "PgRenderer.H"

PgAMPool2::PgAMPool2()
	: m_kUpdateTimer(5000), m_kDeleteTimer(300), m_bInitialized(true)
{
}

PgAMPool2::~PgAMPool2()
{
}

void PgAMPool2::Destroy()
{
	m_bInitialized = false;
{
	BM::CAutoMutex kLock(m_kCacheLock, true);

	int	iTotalAM = m_kCacheContainer.size();

//	_PgOutputDebugString("Total AM In Cache : %d \n",iTotalAM);
	int	iCount =0 ;
	for(AMCacheContainer::iterator itr = m_kCacheContainer.begin();itr != m_kCacheContainer.end();++itr)
	{
//		_PgOutputDebugString("Current AM Index : %d \n",iCount);
		iCount++;
		if (itr->second == NULL)
		{
//			_PgOutputDebugString("Current AM Pass\n");
			continue;
		}

		AMPtrList	&kAMList = itr->second->kAMList;
		kAMList.clear(); // SmartPointer로 바꾸면서 clear만 하면 된다.
		SAFE_DELETE(itr->second);
//		_PgOutputDebugString("Current AM OK\n");
	}
	m_kCacheContainer.clear();
}
//	_PgOutputDebugString("------ CacheLock::UnLockWriter1\n");
	m_kDeleteAMList.clear();	// Smart Pointer;
}

void PgAMPool2::Update(float fAccumTime)
{
#define PG_USE_AMPOOL_CLEANUP
#ifdef PG_USE_AMPOOL_CLEANUP
	if (m_kUpdateTimer.Tick() && m_bInitialized)
	{
		float fCurTime = NiGetCurrentTimeInSec();
		// AMPool 정리. 오랬동안 안 쓰는건 좀 지우자.
//		_PgOutputDebugString("------ CacheLock::LockWriter2\n");
		BM::CAutoMutex kLock(m_kCacheLock, true);
		for(AMCacheContainer::iterator itr = m_kCacheContainer.begin();
			itr != m_kCacheContainer.end();
			++itr)
		{
			stAMPoolInfo* pkAMPoolInfo = itr->second;
			if (pkAMPoolInfo == NULL)
				continue;

			if (pkAMPoolInfo->dwRequestTime == 0
				&& (pkAMPoolInfo->fLastReturnTime == 0 || pkAMPoolInfo->fLastReturnTime > (fCurTime - 180.0f))
				&& (pkAMPoolInfo->fLastServedTime == 0 || pkAMPoolInfo->fLastServedTime > (fCurTime - 300.0f)))
			{
				NILOG(PGLOG_MINOR, "[PgAMPool] %s pool(%d) info last return:%f, last served:%f, pool size: %d\n",pkAMPoolInfo->strKFMPath.c_str(), pkAMPoolInfo->eObjectID, pkAMPoolInfo->fLastReturnTime, pkAMPoolInfo->fLastServedTime, pkAMPoolInfo->kAMList.size());
				
				BM::CAutoMutex kLock(pkAMPoolInfo->kPoolLock);
				while (pkAMPoolInfo->kAMList.size() > pkAMPoolInfo->iMaxCount)
				{
					m_kDeleteAMList.push_back(pkAMPoolInfo->kAMList.back()); // 지우는건 Lock을 풀고 지운다.
					pkAMPoolInfo->kAMList.pop_back();
				}
			}			
		}
//		_PgOutputDebugString("------ CacheLock::UnLockWriter2\n");
	}

	if (m_kDeleteAMList.size() && m_kDeleteTimer.Tick() && m_bInitialized)
	{
		int iSize = m_kDeleteAMList.size();
		float fTime = NiGetCurrentTimeInSec();
		// Thread로 지우는게 더 좋을까?
		m_kDeleteAMList.pop_back();
		NILOG(PGLOG_THREAD, "[PgAMPool] %d AM waiting for delete, one am deleted (%f time elapsed)\n", iSize - 1, NiGetCurrentTimeInSec() - fTime);
	}
#endif
}

bool PgAMPool2::ReturnActorManager(NiActorManagerPtr pAM)
{
	if (pAM == NULL)
		return false;

	g_bUseAMReuse = (::GetPrivateProfileInt(_T("Debug"), _T("UseAMReuse"), 1, g_pkApp->GetConfigFileName()) == 1);
	if (g_bUseAMReuse == false)
	{
		pAM = NULL;
		return true;
	}

	std::string filePath;
	if (pAM->GetKFMTool() == NULL)
		return false;
	
	filePath = pAM->GetKFMFileName();
	if (filePath.size() == 0)
		return false;

	pAM->Reset();
	pAM->Update(0);
	pAM->SetCallbackObject(0);
	pAM->SetCallbackAVObject(0);
	pAM->ClearAllRegisteredCallbacks();
	if (pAM->GetNIFRoot())
	{
		pAM->GetNIFRoot()->Update(0.0f);
		pAM->GetNIFRoot()->SetAppCulled(true);
	}

	NILOG(PGLOG_THREAD, "[PgAMPool2]\tReturnActorManager 0x%0X, %s\n", pAM, filePath.c_str());

	if (m_bInitialized == false)
	{
		NILOG(PGLOG_WARNING, "[PgAMPool2]\tReturnActorManager 0x%0X, %s after destroy\n", pAM, filePath.c_str());
		pAM = NULL;
		return true;
	}

	bool bFound = false;
	stAMPoolInfo* pPoolInfo = NULL;
	{
		BM::CAutoMutex kLock(m_kCacheLock);
		AMCacheContainer::iterator iter = m_kCacheContainer.find(filePath);
		bFound = (iter != m_kCacheContainer.end());
		pPoolInfo = iter->second;
	}

	if (bFound)
	{
		unsigned int iOldListSize = 0;
		unsigned int iNewListSize = 0;
		
		PG_ASSERT_LOG(pPoolInfo);

		{
			BM::CAutoMutex kLock(pPoolInfo->kPoolLock);
			iNewListSize = iOldListSize = pPoolInfo->kAMList.size();
			AMPtrList::const_iterator iter = std::find(pPoolInfo->kAMList.begin(), pPoolInfo->kAMList.end(), pAM);
			if (iter != pPoolInfo->kAMList.end())
			{
				NILOG(PGLOG_ERROR, "[PgAMPool2]\tReturnActorManager(%s) try to insert duplicate am pointer\n", filePath.c_str());
				pAM = NULL;
				return false;
			}

			if (iOldListSize < pPoolInfo->iMaxCount)
			{
				pPoolInfo->kAMList.push_back(pAM);
				iNewListSize++;
				pPoolInfo->fLastReturnTime = NiGetCurrentTimeInSec();
				NILOG(PGLOG_THREAD, "[PgAMPool2]\tReturnActorManager(%s) succeeded(%d, %d)\n", filePath.c_str(), iOldListSize, iNewListSize);
			}
			return true;
		}

		pAM = 0;
		return true;
	}

	NILOG(PGLOG_THREAD, "[PgAMPool2]\tReturnActorManager(%s) failed\n", filePath.c_str());
	pAM = 0;
	return false;
}

NiActorManagerPtr PgAMPool2::LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID, bool bPrepareOnly)
{
	NILOG(PGLOG_THREAD, "[PgAMPool2]\tLoadActorManager %s,%d\n", pcKFMPath, eObjectID);

	if (m_bInitialized == false)
	{
		NILOG(PGLOG_WARNING, "[PgAMPool2]\tLoadActorManager %s,%d after destroy\n", pcKFMPath, eObjectID);
		return NULL;
	}

	stAMPoolInfo* pkAmPoolInfo = NULL;
	{//아래에 WriteLock 있으므로 풀지말것.
		BM::CAutoMutex kLock(m_kCacheLock);
		AMCacheContainer::iterator itr = m_kCacheContainer.find(pcKFMPath);
		if(itr != m_kCacheContainer.end())
		{
			pkAmPoolInfo = itr->second;
		}
	}

	if(!pkAmPoolInfo)
	{
		NILOG(PGLOG_THREAD, "[PgAMPool2]\tFirst Loading AM of %s\n", pcKFMPath);
		unsigned int iMinCount = 0;
		unsigned int iMaxCount = 0;

		switch(eObjectID)
		{
		case PgIXmlObject::ID_PC:
			{
				iMinCount = 2;
				iMaxCount = 5;
			}break;
		case PgIXmlObject::ID_NPC:
			{
				iMinCount = 1;
				iMaxCount = 1;
			}break;
		case PgIXmlObject::ID_MONSTER:
			{
				iMinCount = 1;
				iMaxCount = 5;
			}break;
		case PgIXmlObject::ID_PET:
			{
				iMinCount = 1;
				iMaxCount = 3;
			}break;
		case PgIXmlObject::ID_BOSS:
			{
				iMinCount = 0;
				iMaxCount = 0;
			}break;
		case PgIXmlObject::ID_FURNITURE:
			{
				iMinCount = 0;
				iMaxCount = 0;
			}break;
		case PgIXmlObject::ID_DROPBOX:
			{
				iMinCount = 2;
				iMaxCount = 10;
			}break;
		case PgIXmlObject::ID_ITEM:
			{
				iMinCount = 2;
				iMaxCount = 5;
			}break;
		default:
			{
				NILOG(PGLOG_WARNING, "[PgAMPool2] Unknown ObjectID(%d, %s)\n", eObjectID, pcKFMPath);
				iMinCount = 0;
				iMaxCount = 0;
			}break;
		}

		PgNiFile::SetSilentLoading(true);
		NiActorManagerPtr pkAM = PgActorManager::CreatePG(pcKFMPath);
		PgNiFile::SetSilentLoading(false);
		if (iMinCount < 1 || pkAM == NULL)
			return pkAM;

		stAMPoolInfo* pAMInfo = new stAMPoolInfo;
		PG_ASSERT_LOG(pAMInfo);

		pAMInfo->iMinCount = iMinCount;
		pAMInfo->iMaxCount = iMaxCount;
		pAMInfo->eObjectID = eObjectID;
		pAMInfo->iTotalCount = 0;
		pAMInfo->iCurCount = 0;
		pAMInfo->iReservedCount = 0;
		pAMInfo->iRequestCount = 0;
		pAMInfo->strKFMPath = pcKFMPath;
		pAMInfo->bLoadFailed = false;
		pAMInfo->dwRequestTime = BM::GetTime32();
		pAMInfo->fLastServedTime = 0.0f;
		pAMInfo->fLastReturnTime = 0.0f;
		pAMInfo->fLastWorkTime = 0.0f;
		pAMInfo->kAMList.push_back(pkAM);

		std::pair<AMCacheContainer::iterator, bool> ret;

		BM::CAutoMutex kLock(m_kCacheLock, true);
		ret = m_kCacheContainer.insert(std::make_pair(pcKFMPath, pAMInfo));
		pkAmPoolInfo = ret.first->second;
	}

	if (bPrepareOnly)
	{
		return NULL;
	}

	NILOG(PGLOG_THREAD, "[PgAMPool2]\t Before GetClonnedActorManager(%d)\n", pkAmPoolInfo->kAMList.size());
	return GetClonnedActorManager(pkAmPoolInfo);
}

NiActorManagerPtr PgAMPool2::GetClonnedActorManager(stAMPoolInfo* pkAmPoolInfo)
{
	float fStartTime = NiGetCurrentTimeInSec();
	NiActorManagerPtr pOriginal = NULL;
	NiActorManagerPtr pClonned = NULL;
	unsigned int iOldListSize = 0;
	unsigned int iNewListSize = 0;

	stAMPoolInfo* pkPoolInfo = pkAmPoolInfo;
{
	BM::CAutoMutex kLock(pkPoolInfo->kPoolLock);
	iNewListSize = iOldListSize = pkPoolInfo->kAMList.size();
	if (iOldListSize > 2)
	{
		pClonned = pkPoolInfo->kAMList.back();
		PG_ASSERT_LOG(pClonned);
		pkPoolInfo->kAMList.pop_back();
		iNewListSize--;
	}
	else
	{
		pOriginal = pkPoolInfo->kAMList.front();		
	}
	pkPoolInfo->dwRequestTime = 0;

}

	if (pClonned == NULL)
	{
		pClonned = pOriginal->Clone();
	}

	if (pClonned && pClonned->GetNIFRoot())
	{
		pClonned->GetNIFRoot()->SetAppCulled(false);
	}

	if (iNewListSize < pkPoolInfo->iMaxCount)
	{
		g_kMemoryWorkerThread.DoCloneActor(pkAmPoolInfo, pkPoolInfo->iMaxCount - iNewListSize);
	}

	pkPoolInfo->fLastServedTime = NiGetCurrentTimeInSec();
	NILOG(PGLOG_THREAD, "[PgAMPool2]\t GetClonnedActorManager returned(%d, %d)\n", iOldListSize, iNewListSize, pkPoolInfo->fLastServedTime - fStartTime);
	return pClonned;
}
