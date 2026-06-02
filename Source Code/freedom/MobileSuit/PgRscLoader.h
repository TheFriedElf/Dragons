#ifndef FREEDOM_DRAGONICA_PGRSCLOADER_H
#define FREEDOM_DRAGONICA_PGRSCLOADER_H
#include "PgRscContainer.h"

class PgRscLoader//ThreadBase
	:	public PgTask< tagRscMessage >,
		public BM::CObserver< tagRscMessage >
{
	typedef tagRscMessage T_MSG;
public:
	PgRscLoader();
	virtual ~PgRscLoader();

public:
	virtual void VUpdate( BM::CSubject< T_MSG > *const pChangedSubject, T_MSG rkNfy );
	virtual void HandleMessage(T_MSG *pkMsg);
};
//可历滚肺??.

#define g_kRscLoader SINGLETON_STATIC(PgRscLoader)





#pragma pack(1)

typedef struct tagLoadMessage
	:	public BM::Stream
{
	tagLoadMessage(int const iInPriType=0)
	{
		PriType(iInPriType);
		m_kOrderGuid.Generate();
	}

	void Set(int const iInPriType)
	{
		PriType(iInPriType);
	}

	CLASS_DECLARATION_S(int, PriType);
	CLASS_DECLARATION_S(BM::GUID, OrderGuid);
}SLoadMessage;

#pragma pack()

class PgLoader//ThreadBase
	:	public PgTask<SLoadMessage>
{
public:
	PgLoader()
	{
		IsComplete(false);
		IsMainComplete(false);
		IsLogoComplete(false);
	}
	virtual ~PgLoader()
	{
	}

public:
	void MainCompleteSignal();
	void LogoCompleteSignal();

	virtual bool PutMsg(MSG &rkMsg);
	virtual void HandleMessage(MSG *pkMsg);
	bool WaitLoad();
	virtual int StartSvc(int const iThreadCount = 3)
	{
		if(iThreadCount)
		{
			return activate( THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, iThreadCount, 1, THREAD_PRIORITY_ABOVE_NORMAL );
		}
		return 0;
	}
	
	
	ACE_Message_Queue<ACE_MT_SYNCH> msg_queue_;
	
	std::set<BM::GUID> m_kLoadOrder;
	
	CLASS_DECLARATION_S(bool, IsComplete);
	CLASS_DECLARATION_S(bool, IsMainComplete);
	CLASS_DECLARATION_S(bool, IsLogoComplete);

	Loki::Mutex m_kMutex;
};
//可历滚肺??.
#define g_kLoader SINGLETON_STATIC(PgLoader)

#endif // FREEDOM_DRAGONICA_PGRSCLOADER_H