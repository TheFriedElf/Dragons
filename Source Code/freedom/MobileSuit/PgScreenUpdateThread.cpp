#include "StdAfx.h"
#include "PgScreenUpdateThread.h"
#include "PgMobileSuit.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgUIScene.h"
#include "PgRscLoader.h"
#include "PgAMPool.h"
#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"

PgScreenUpdateThread g_kScreenUpdateThread;

PgScreenUpdateThread::PgScreenUpdateThread() : m_dwInitTime(0)
{
	//m_pThread = NULL;
	m_bStopWorkerThread = true; //! Init하기 전에는 멈춰 있음.
}

PgScreenUpdateThread::~PgScreenUpdateThread()
{
	Terminate();
}

void PgScreenUpdateThread::SetThreadPriority(NiThread::Priority ePriority)
{
	//if (m_pThread)
	//{
	//	m_pThread->SetPriority(ePriority);
	//}
}

bool PgScreenUpdateThread::Initialize()
{
	//if (m_pThread != NULL)
	//{
	//	//! 이미 쓰레드가 있다면? 언놈이 만든게냐;;
	//	return false;
	//}

	//m_pThread = NiThread::Create(this);
	//m_pThread->SetName("ScreenUpdaterThread");
	//m_pThread->SetPriority(NiThread::LOWEST);
	//m_pThread->Resume();
	m_bStopWorkerThread = false;
	m_dwInitTime = BM::GetTime32();
	return true;
}

void PgScreenUpdateThread::Terminate()
{
	//if (m_pThread != NULL)
	//{
	//	m_bStopWorkerThread = true;
	//	while (m_pThread->GetStatus() != NiThread::COMPLETE)
	//		NiSleep(10);
	//	SAFE_DELETE_NI(m_pThread);
	//}
	m_bStopWorkerThread = true;
	m_dwInitTime = 0;
}

unsigned int PgScreenUpdateThread::ThreadProcedure(void* pvArg)
{
	PG_TRY_BLOCK
	//NILOG(PGLOG_THREAD, "[PgScreenUpdateThread] Start of PgScreenUpdateThreaProc\n");
	PgRenderer* pRenderer = g_kRenderMan.GetRenderer();
	if (!m_bStopWorkerThread)
	{
		g_kAMPool.Update(0.0f);		
		g_kUIScene.Update(g_pkApp->GetAccumTime(), 0.06f);
		if (g_pkApp->GetFullscreen() == false || (g_pkApp->GetFullscreen() && g_pkApp->IsActivated()))
		{
			pRenderer->BeginFrame();
			pRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ALL);
			pRenderer->EndUsingRenderTargetGroup();
			g_kUIScene.Draw(pRenderer, 0.06f);
			pRenderer->EndFrame();
			pRenderer->DisplayFrame();
		}
		g_pkApp->IncFrameCount();
		//if (!m_bStopWorkerThread)
		//	NiSleep(50);

		if (GetInitTime() > 0 && GetPassedTime() > 10000)
		{
			g_kLoader.LogoCompleteSignal();
			m_dwInitTime = 0;
		}
	}
	PG_CATCH_BLOCK

	//NILOG(PGLOG_THREAD, "[PgScreenUpdateThread] End of PgScreenUpdateThreaProc\n");
	return 0;
}
