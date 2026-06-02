#ifndef FREEDOM_DRAGONICA_THREAD_PGSCREENUPDATETHREAD_H
#define FREEDOM_DRAGONICA_THREAD_PGSCREENUPDATETHREAD_H

#define PG_USE_SCREEN_UPDATE_TRHEAD

class PgScreenUpdateThread : public NiThreadProcedure
{	
protected:

public:
	PgScreenUpdateThread();
	~PgScreenUpdateThread();

	bool Initialize();
	void Terminate();
	void SetThreadPriority(NiThread::Priority ePriority);
	bool IsRunning() { return !m_bStopWorkerThread; }
	DWORD GetInitTime()	{ return m_dwInitTime; }
	DWORD GetPassedTime() { return BM::GetTime32() - m_dwInitTime; }

	// override
	virtual unsigned int ThreadProcedure(void* pvArg);
protected:
	//NiThread* m_pThread;
	bool m_bStopWorkerThread;		//! 모든 일을 멈추게 된다. 당연히 Update에서 DoLoop도 호출 안된다.
	DWORD m_dwInitTime;
};

extern PgScreenUpdateThread g_kScreenUpdateThread;

#ifdef PG_USE_SCREEN_UPDATE_TRHEAD
#define PG_START_SCREEN_UPDATE_THREAD	g_kScreenUpdateThread.Initialize();
#define PG_END_SCREEN_UPDATE_THREAD	g_kScreenUpdateThread.Terminate();
#else
#define PG_START_SCREEN_UPDATE_THREAD
#define PG_END_SCREEN_UPDATE_THREAD
#endif

#endif // FREEDOM_DRAGONICA_THREAD_PGSCREENUPDATETHREAD_H