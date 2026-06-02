#ifndef FREEDOM_DRAGONICA_THREAD_PGAMPOOL2_H
#define FREEDOM_DRAGONICA_THREAD_PGAMPOOL2_H

#include "PgAMPool.h"
#include "Utility/SimpleTimer.h"

#define PG_USE_ACTORMANAGER_REUSE

class PgAMPool2
{
	typedef std::map<std::string, stAMPoolInfo*> AMCacheContainer;

public:
	PgAMPool2();
	~PgAMPool2();

	void Destroy();
	NiActorManagerPtr LoadActorManager(char const *pcKFMPath, PgIXmlObject::XmlObjectID eObjectID, bool bPrepareOnly = false);
	bool ReturnActorManager(NiActorManagerPtr pAM);
	void Update(float fAccumTime);
	void SetThreadPriority(NiThread::Priority ePriority) {}
	NiThread::Priority GetThreadPriority() { return NiThread::IDLE; }

	AMCacheContainer m_kCacheContainer;
protected:
	NiActorManagerPtr GetClonnedActorManager(stAMPoolInfo* pkAmPoolInfo);
	ACE_RW_Thread_Mutex m_kCacheLock;
	SimpleTimer m_kUpdateTimer;
	SimpleTimer m_kDeleteTimer;
	AMPtrList m_kDeleteAMList;
	bool m_bInitialized;
};
#endif // FREEDOM_DRAGONICA_THREAD_PGAMPOOL2_H