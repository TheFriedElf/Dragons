#ifndef FREEDOM_DRAGONICA_THREAD_PGAMPOOL_H
#define FREEDOM_DRAGONICA_THREAD_PGAMPOOL_H

//#define PG_AMPOOL_VER1
#define PG_AMPOOL_VER2

#ifdef PG_AMPOOL_VER2
#define MAX_TASK_QUEUE_COUNT	5
#endif

typedef std::list<NiActorManagerPtr> AMPtrList;

struct stAMPoolInfo
{
	PgIXmlObject::XmlObjectID eObjectID;
	unsigned int iReservedCount;
	unsigned int iMinCount;
	unsigned int iMaxCount;
	unsigned int iCurCount; // 현재 유지되고 있는 Pool 갯수. size()로 합칠까?
	unsigned int iTotalCount; // 지금까지 Clone되어서 나간 녀석들 + Pool에 있는 녀석들의 합
	AMPtrList kAMList;
	std::string strKFMPath; // AMList에 아무것도 없으면 이걸 이용해서 Clonning을 한다.
	Loki::Mutex kPoolLock;
	DWORD dwRequestTime; // 요청이 들어온 시간 기록. 요청을 처리하고 return할때 0이 됨.
	float fLastWorkTime; // 마지막으로 BackGround Clonning한 시간.
	float fLastServedTime; // 마지막으로 요청을 받은 시간.
	float fLastReturnTime; // 마지막으로 돌려 받은 시간.
	int iRequestCount; // 현재 waiting되고 있는 RequestCount;
	bool bLoadFailed;
};

#include "PgAMPool2.h"

class	PgMobileSuit;
/*
class PgAMPool
{
#if defined (PG_AMPOOL_VER1) || defined(PG_AMPOOL_VER2)
	typedef std::map<std::string, stAMPoolInfo*> AMCacheContainer;
	typedef std::list<stAMPoolInfo*> AMCloneTaskQueue;
#else
	typedef std::map<std::string, AMPtrList> AMCacheContainer;
#endif	

	friend	class	PgMobileSuit;

public:
	PgAMPool();
	~PgAMPool();


	NiActorManagerPtr LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID, bool bPrepareOnly = false);
	void RemoveClonnedActorManager(NiActorManagerPtr pActorManager);

	NiLoopedThread* GetClonningThread() { return m_pkClonningThread; }
	void SuspendClonningThread();
	void ResumeClonningThread();
	void Update(float fAccumTime);
	void SetThreadPriority(NiThread::Priority ePriority);
	NiThread::Priority GetThreadPriority();

	AMCacheContainer m_kCacheContainer;
protected:

	void Destroy();

protected:
#if defined (PG_AMPOOL_VER1)
	NiActorManagerPtr GetClonnedActorManager(AMCacheContainer::iterator iter, bool bFirst);
#elif defined (PG_AMPOOL_VER2)
	NiActorManagerPtr GetClonnedActorManager(stAMPoolInfo* pkAmPoolInfo);
#else
	NiActorManagerPtr GetClonnedActorManager(char const *pcKFMPath);
#endif

	class ClonningThreadProc : public NiLoopedThreadProcedure
	{		
	public:
		// override
		virtual bool LoopedProcedure(void* pvArg);
		bool m_bTerminate;

#if defined (PG_AMPOOL_VER2)
		int InvalidateTaskQueue();
		Loki::Mutex m_kTaskQueueLock;
		AMCloneTaskQueue m_kTaskQueue[MAX_TASK_QUEUE_COUNT];
		AMCacheContainer* m_pkCacheContainer;
		ACE_RW_Thread_Mutex* m_pkCacheLock;
#endif
	};

	//char m_kPathBackup[MAX_PATH];
	NiLoopedThread* m_pkClonningThread;
	ClonningThreadProc m_kClonningThreadProc;
#ifdef PG_AMPOOL_VER2
	ACE_RW_Thread_Mutex m_kCacheLock;
#else
	Loki::Mutex m_kClonningLock;
#endif
	DWORD m_fLastLoopTime;
};
*/
//#define g_kAMPool SINGLETON_STATIC(PgAMPool)
#define g_kAMPool SINGLETON_STATIC(PgAMPool2)
#endif // FREEDOM_DRAGONICA_THREAD_PGAMPOOL_H