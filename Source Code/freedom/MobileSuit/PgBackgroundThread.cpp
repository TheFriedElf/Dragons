#include "StdAfx.h"
#include "PgBackgroundThread.h"
#include "PgMobileSuit.h"
#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"

#ifdef PG_USE_BACKGROUND_THREAD
PgBackgroundThread g_kBackgroundThread;

PgBackgroundThread::PgBackgroundThread() :
	m_eCurrentStage(STAGE_NONE),
	m_eReservedStageChangeNum(STAGE_NONE),
	m_iCurrentWorkID(0),
	m_pThread(0),
	m_hMainWnd(0),
	m_bTerminate(false)
{
	for (int i = 0; i < MAX_STAGE_NUM; i++)
		m_bStageWorkDone[i] = false;
}

PgBackgroundThread::~PgBackgroundThread()
{
	if (m_pThread != NULL)
	{
		Terminate();
		SAFE_DELETE_NI(m_pThread);
	}
	m_pThread = NULL;
}

void PgBackgroundThread::SetThreadPriority(NiThread::Priority ePriority)
{
	if (m_pThread)
	{
		m_pThread->SetPriority(ePriority);
	}
}

unsigned int PgBackgroundThread::ThreadProcedure(void* pvArg)
{
	PG_TRY_BLOCK
	NILOG(PGLOG_THREAD, "[PgBackgroundThread] Start of PgWorkerThreaProc\n");
	m_eCurrentStage = STAGE_0;		
	while (!m_bTerminate)
	{
		bool bWorkDone = false;
		float workStartTime = NiGetCurrentTimeInSec();
		NILOG(PGLOG_THREAD, "[PgBackgroundThread] Start of Stage(%d)\n", m_eCurrentStage);
		switch(m_eCurrentStage)
		{
		case STAGE_0:
			bWorkDone = doStage0Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		case STAGE_1:
			bWorkDone = doStage1Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		case STAGE_2:
			bWorkDone = doStage2Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		case STAGE_3:
			bWorkDone = doStage3Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		case STAGE_4:
			bWorkDone = doStage4Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		case STAGE_5:
			bWorkDone = doStage5Work();
			m_bStageWorkDone[m_eCurrentStage] = bWorkDone;
			break;
		default:				
			bWorkDone = true;
			NiSleep(1000);
			//return true;
			; // do nothing. or increase sleep time?
		}

		//if (m_eCurrentStage < MAX_STAGE_NUM)
			NILOG(PGLOG_THREAD, "[PgBackgroundThread] End of Stage(%d,%d,time:%f)\n", m_eCurrentStage, bWorkDone, NiGetCurrentTimeInSec() - workStartTime);

		if (bWorkDone)
		{
			//! Stage의 일을 다 마쳤으니 MainThread에 신호를 보내고 다음 Stage로 간다.
			
			m_kStageLock.Lock();
			//! TODO: MainThread에 신호 보내기
			if (m_hMainWnd)
			{
				if (::PostMessage(m_hMainWnd, WM_PG_STAGEDONE_NOTIFY, m_eCurrentStage, 0) == FALSE)
				{
					NILOG(PGLOG_THREAD, "[PgBackgroundThread] Send Main Wnd Notify Failed(%d)\n", GetLastError());
				}
				else
				{
					NILOG(PGLOG_THREAD, "[PgBackgroundThread] Send Main Wnd Notify\n");
				}
			}
			m_eCurrentStage = (PgBackgroundThread::Stage)(m_eCurrentStage + 1);
			m_kStageLock.Unlock();
		}
		else
		{
			g_pkApp->QuitApplication();
			break;
		}

		NiSleep(1);
	}

	NILOG(PGLOG_THREAD, "[PgBackgroundThread] End of PgBackgroundThreaProc\n\n");
	PG_CATCH_BLOCK
	return true;
}

bool PgBackgroundThread::Initialize(HWND hWnd, bool bResume)
{
	if (m_pThread != NULL)
	{
		//! 이미 쓰레드가 있다면? 언놈이 만든게냐;;
		return false;
	}

	if (hWnd == NULL)
		return false;

	m_hMainWnd = hWnd;

	m_pThread = NiThread::Create(this);
	m_pThread->SetName("BackgroundThread");
	m_pThread->SetPriority(NiThread::BELOW_NORMAL);
	if (bResume)
		m_pThread->Resume();

	return true;
}

void PgBackgroundThread::Terminate()
{
	if (m_pThread != NULL)
	{
		m_bTerminate = true;
		if (m_pThread->GetStatus() == NiThread::RUNNING)
		{
			Stage eStage = STAGE_NONE;
			bool bEnd = false;
			GetCurrentState(eStage, bEnd);
			if (eStage < STAGE_5)
			{
				m_pThread->Suspend();
			}
			else
			{
				m_pThread->WaitForCompletion();
			}
		}
		SAFE_DELETE_NI(m_pThread);
		m_pThread = NULL;
	}
}

void PgBackgroundThread::Suspend()
{
	if (m_pThread != NULL)
		m_pThread->Suspend();
}

bool PgBackgroundThread::doStage0Work()
{
	if (g_pkApp)
		return g_pkApp->DoStage0Work();

	return false;
}

bool PgBackgroundThread::doStage1Work()
{
	if (g_pkApp)
		return g_pkApp->DoStage1Work();
	
	return false;
}

bool PgBackgroundThread::doStage2Work()
{
	if (g_pkApp)
		return g_pkApp->DoStage2Work();
	return true;
}

bool PgBackgroundThread::doStage3Work()
{
	if (g_pkApp)
		return g_pkApp->DoStage3Work();
	return true;
}

bool PgBackgroundThread::doStage4Work()
{
	if (g_pkApp)
		return g_pkApp->DoStage4Work();
	return true;
}

bool PgBackgroundThread::doStage5Work()
{
//	if (g_pkApp)
//		return g_pkApp->DoStage5Work();
	return true;
}

void PgBackgroundThread::GetCurrentState(Stage& eStage, bool& bEnd)
{
	BM::CAutoMutex kLock(m_kStageLock);
	eStage = m_eCurrentStage;
	if (m_eCurrentStage > STAGE_NONE && m_eCurrentStage < MAX_STAGE_NUM)
		bEnd = m_bStageWorkDone[m_eCurrentStage];
	else
		bEnd = false;
}

bool PgBackgroundThread::GetStageWorkDone(Stage eStage)
{
	BM::CAutoMutex kLock(m_kStageLock);
	if (eStage > STAGE_NONE && eStage < MAX_STAGE_NUM)
		return m_bStageWorkDone[eStage];
	return false;
}
#endif