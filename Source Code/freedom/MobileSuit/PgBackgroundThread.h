#ifndef FREEDOM_DRAGONICA_THREAD_PGBACKGORUNDTHRAD_H
#define FREEDOM_DRAGONICA_THREAD_PGBACKGORUNDTHRAD_H

#ifdef PG_USE_BACKGROUND_THREAD

class PgBackgroundThread : public NiThreadProcedure
{
public:
	typedef enum
	{
		STAGE_NONE = -1,
		STAGE_0,
		STAGE_1,
		STAGE_2,
		STAGE_3,
		STAGE_4,
		STAGE_5,
		MAX_STAGE_NUM,
	} Stage;

	PgBackgroundThread();
	~PgBackgroundThread();
	// override
	virtual unsigned int ThreadProcedure(void* pvArg);

	// works	
	bool Initialize(HWND hWnd, bool bResume = true);
	void Terminate();
	void Suspend();
	void Resume();
	void SetThreadPriority(NiThread::Priority ePriority);
	void GetCurrentState(Stage& eStage, bool& bEnd);
	bool GetStageWorkDone(Stage eStage);
protected:
	bool doStage0Work();
	bool doStage1Work();
	bool doStage2Work();
	bool doStage3Work();
	bool doStage4Work();
	bool doStage5Work();
protected:
	NiThread* m_pThread;
	bool m_bTerminate;
	bool m_bStageWorkDone[MAX_STAGE_NUM];
	Stage m_eCurrentStage;
	int m_iCurrentWorkID;
	Stage m_eReservedStageChangeNum;
	Loki::Mutex m_kStageLock;
	HWND m_hMainWnd;
};

extern PgBackgroundThread g_kBackgroundThread;

#endif
#endif // FREEDOM_DRAGONICA_THREAD_PGBACKGORUNDTHRAD_H