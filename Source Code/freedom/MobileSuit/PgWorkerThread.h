#ifndef FREEDOM_DRAGONICA_THREAD_PGWORKERTHREAD_H
#define FREEDOM_DRAGONICA_THREAD_PGWORKERTHREAD_H

#include "ace/Task_ex_t.h"
#include "ace/Message_Queue_t.h"
#include "BM/GUID.h"
#include "PgStat.h"
#include "PgAMPool.h"
#include "PgItemMan.h"

class PgActor;
class PgWorld;

typedef enum eWorkerThreadMessageType
{
	E_WTMT_NONE = -1,
	E_WTMT_DELETE,
	E_WTMT_ITEM_EQUIP,
	E_WTMT_ADDUNIT,
	E_WTMT_CLONE_ACTOR,
	//E_WTMT_CLONE_ACTOR2,
	//E_WTMT_SCREEN_SHOT,
	E_WTMT_LOAD_XML,
	E_WTMT_LOAD_SOUND,
	E_WTMT_LOAD_EFFECT,
	MAX_WTMT_NUM,
}EWorkerThreadMessageType;

typedef enum eWorkPriority
{
	E_PRIORITY_NONE = -10,
	E_PRIORITY_TAIL = -3,
	E_PRIORITY_LOWEST,
	E_PRIORITY_LOW,
	E_PRIORITY_NORMAL,
	E_PRIORITY_HIGH,
	E_PRIORITY_HIGHEST,	
	E_PRIORITY_HEAD,
	MAX_PRIORITY_NUM,
} EWorkPriority;

typedef enum eBasePriority
{
	E_BASE_PRIORITY_NONE = -1, 
	E_BASE_PRIORITY_DELETE = 2,		// 2부터 시작하는 이유는.. -2가 될 수 있기 때문에.
	E_BASE_PRIORITY_LOAD,
	E_BASE_PRIORITY_CLONE,
	E_BASE_PRIORITY_ITEM,
	E_BASE_PRIORITY_ADDUNIT,
	MAX_BASE_PRIORITY_NUM,
} EBasePriority;

#pragma pack(1)
typedef struct _ItemEquipInfo
{
	_ItemEquipInfo()
	{
		Clear();
	}

	void Clear()
	{
		iItemNo = 0;
		iGender = 0;
		iClass = 0;
		bSetToDefault = false;
	}
	int iItemNo;
	int iGender;
	int iClass;
	bool bSetToDefault;
}ItemEquipInfo;

typedef struct _WorkData
{
	_WorkData()
	{
		Clear();
	}

	void Clear()
	{
		pWorkData = NULL;
		bWorkData = false;
		fWorkData = 0.0f;
		iWorkData = 0;
		strWorkData = "";
		strWorkData2 = "";
	}
	// SetAbil/GetAbil 처럼 만들면 좋을까;;
	void* pWorkData;
	bool bWorkData;
	float fWorkData;
	int iWorkData;
	std::string strWorkData;
	std::string strWorkData2;
} WorkData;
#pragma pack()

#pragma pack(1)
typedef struct tagWorkerThreadMessage
{
	tagWorkerThreadMessage()
	{
		Clear();
	}

	void Clear()
	{
		iType = 0;
		fWorkQueueTime = 0.0f;
		pObject = NULL;
		pWorker = NULL;
		kGuid.Clear();//엑터GUID
		kInfo.Clear();
		kWorkData.Clear();
	}

	int iType;
	NiAVObjectPtr pObject;
	BM::GUID kGuid;//엑터GUID
	ItemEquipInfo kInfo;
	float fWorkQueueTime;
	void* pWorker;
	WorkData kWorkData;
} SWorkerThreadMessage;
#pragma pack()

class PgIClientWorker
{
public:
	virtual bool DoClientWork(WorkData& rkWorkData) = 0;
};

class PgClientWorkerThread
	:	public	ACE_Task_Ex< ACE_MT_SYNCH , SWorkerThreadMessage >
{
	typedef enum eValue
	{
		E_WATER_MARK	= sizeof(SWorkerThreadMessage) * 5000,//! 2000개의 메세지를 버텨준다.
	}E_VALUE;

public:
	PgClientWorkerThread();
	virtual ~PgClientWorkerThread();

	bool ClearAllWorks(bool bDeleteAll = false);
	bool DoDeleteObject(NiAVObjectPtr spObject, EWorkPriority ePriority = E_PRIORITY_NORMAL);	
	bool DoDeleteParticle(NiAVObjectPtr spObject, EWorkPriority ePriority = E_PRIORITY_NORMAL);

	bool DoEquipItem(PgActor* pkActor, BM::GUID const &kActorGuid, int const iItemPos, bool const bSetToDefaultItem, int iGender, int iClass, EWorkPriority ePriority = E_PRIORITY_NORMAL);
	bool DoAddUnit(CUnit* pkUnit, PgWorld* pkWorld, EWorkPriority ePriority = E_PRIORITY_NORMAL);
	bool DoCloneActor(stAMPoolInfo* info, int iCount, EWorkPriority ePriority = E_PRIORITY_NORMAL);
	bool DoLoadResource(PgIClientWorker* pkWorker, WorkData& rkWorkData, EWorkPriority ePriority = E_PRIORITY_NORMAL);
	//bool DoCloneActor2(std::string& rkKFMPath, BM::GUID& rkGuid, PgIWorldObject* pkActor, float fScale, bool bSubAM, EWorkPriority ePriority = E_PRIORITY_NORMAL);
	//bool DoSaveScreenShot(LPDIRECT3DSURFACE9 screenSurf, const wchar_t* pcFileName, EWorkPriority ePriority = E_PRIORITY_LOW);
	void PrintStatInfo();
	static void SetWorkerSleepControl(bool bUse);

protected:
	virtual int svc(void);
	EBasePriority adjustPriority(EBasePriority eOriginalPrio, EWorkPriority eWorkPrio);

	CLASS_DECLARATION_S( bool, IsStop );
	CLASS_DECLARATION_NO_SET( bool, m_bIsWorking, IsWorking );
	CLASS_DECLARATION_S( bool, IsPause );
	typedef BM::TObjectPool< SWorkerThreadMessage > MSG_POOL;
	MSG_POOL m_kMsgPool;
	DWORD m_dwLastWorkTime;
	DWORD m_dwLastWorkFrame;
	LONG m_iCurrentWorkingTask;

	StatInfoF m_kThreadWorkStat[MAX_WTMT_NUM];

	static bool ms_bUseSleepControl;
};

extern PgClientWorkerThread g_kAddUnitThread;
extern PgClientWorkerThread g_kEquipThread;
extern PgClientWorkerThread g_kMemoryWorkerThread;
extern PgClientWorkerThread g_kLoadWorkerThread;


#ifdef PG_USE_WORKER_THREAD

#define THREAD_DELETE_OBJECT(obj)	g_kMemoryWorkerThread.DoDeleteObject(obj)
#define THREAD_DELETE_ACTOR(obj)	g_kMemoryWorkerThread.DoDeleteObject(obj)
#define THREAD_DELETE_OBJECT_PRIORITY(obj, prio)	g_kMemoryWorkerThread.DoDeleteObject(obj, prio)
#define THREAD_DELETE_ACTOR_PRIORITY(obj, prio)	g_kMemoryWorkerThread.DoDeleteObject(obj, prio)
#define TRHEAD_LOAD_XML(worker, workData)	g_kLoadWorkerThread.DoLoadResource(worker, workData)

#ifdef PG_USE_ITEM_REUSE
#define THREAD_DELETE_ITEM(obj)		g_kItemMan.CacheItem(obj)
#else
#define THREAD_DELETE_ITEM(obj)	g_kMemoryWorkerThread.DoDeleteObject(obj)
#define THREAD_DELETE_ITEM_PRIORITY(obj, prio)	g_kMemoryWorkerThread.DoDeleteObject(obj, prio)
#endif

#define THREAD_DELETE_PARTICLE(obj)	g_kMemoryWorkerThread.DoDeleteObject(obj)
#define THREAD_DELETE_PARTICLE_PRIORITY(obj, prio)	g_kMemoryWorkerThread.DoDeleteObject(obj, prio)

#else

#define THREAD_DELETE_OBJECT(obj) do { NiAVObjectPtr objectTemp = obj; objectTemp = 0; } while (0)
#define THREAD_DELETE_OBJECT_PRIORITY(obj, prio) do { NiAVObjectPtr objectTemp = obj; objectTemp = 0; } while (0)

#endif
#endif // FREEDOM_DRAGONICA_THREAD_PGWORKERTHREAD_H