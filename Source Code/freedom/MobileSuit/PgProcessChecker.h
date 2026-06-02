#ifndef FREEDOM_DRAGONICA_UTIL_PGPROCESSCHECKER_H
#define FREEDOM_DRAGONICA_UTIL_PGPROCESSCHECKER_H

#include "Utility/ProcessChecker.h"
#include "BM/PgTask.h"

#pragma pack(1)
typedef struct tagDummyMessage	
{
	int iDummy;
} SDummyMessage;

#pragma pack()


class PgProcessChecker
	:	public PgTask<SDummyMessage>,
		public ProcessChecker
{
public:
	PgProcessChecker()
	{
	}
	virtual ~PgProcessChecker()
	{
	}

public:
	virtual int StartSvc(int const iThreadCount = 1)
	{
		return activate(THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, 1, 1, THREAD_PRIORITY_BELOW_NORMAL);
	}
	void StopSvc()
	{
		m_kIsStop = true;
	}

protected:
	virtual bool PutMsg(MSG &rkMsg, unsigned long ulPriority = 0);
	virtual void HandleMessage(MSG *pkMsg);
	virtual int svc(void);

	
	ACE_Message_Queue<ACE_MT_SYNCH> msg_queue_;	
};
//옵저버로??.
#define g_kProcessChecker SINGLETON_STATIC(PgProcessChecker)
#endif // FREEDOM_DRAGONICA_UTIL_PGPROCESSCHECKER_H