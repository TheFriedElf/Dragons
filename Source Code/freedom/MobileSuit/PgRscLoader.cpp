#include "StdAfx.h"
#include "Utility\ExceptionHandler.h"
#include "Utility\ErrorReportFile.h"
#include "PgRscLoader.h"
#include "PgScreenUpdateThread.h"
#include "PgMobileSuit.h"

PgRscLoader::PgRscLoader()
{
}

PgRscLoader::~PgRscLoader()
{
}
	
void PgRscLoader::VUpdate( BM::CSubject< PgRscLoader::T_MSG > *const pChangedSubject, PgRscLoader::T_MSG rkNfy )
{
	if(rkNfy.IsImmidiate())
	{//즉시
		HandleMessage(&rkNfy);
	}
	else
	{
		PutMsg(rkNfy);
	}
}

void PgRscLoader::HandleMessage(PgRscLoader::T_MSG *pkMsg)
{//로드.
//	PgIResourceLoadController* pkController = dynamic_cast< PgIResourceLoadController* >(pkMsg->pkOrder);
	PgRscContainer* pkController = pkMsg->m_pkSubject;
	
	assert(pkController);
	if(pkController)
	{
		void *pkOutRsc = NULL;
		if( S_OK == pkController->LoadResource(*pkMsg, pkOutRsc))
		{
			pkController->LoadComplete(*pkMsg, pkOutRsc);
		}
		return;
	}
}











bool IsLogoImgComplete()
{
	if( XUI::CXUI_Wnd * pkWnd = XUIMgr.Get(L"FRM_BARUNSON_LOGO") )
	{
		return (BM::GetTime32() - pkWnd->CalledTime()) > 3000;
	}
	return false;
}

void PgLoader::LogoCompleteSignal()
{
	BM::CAutoMutex kLock(m_kMutex);
	IsLogoComplete(true);
}

void PgLoader::MainCompleteSignal()
{
	BM::CAutoMutex kLock(m_kMutex);
	IsMainComplete(true);
}

bool PgLoader::PutMsg(MSG &rkMsg)
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kLoadOrder.insert(rkMsg.OrderGuid());

	return PgTask<SLoadMessage>::PutMsg(rkMsg);
}

void PgLoader::HandleMessage(MSG *pkMsg)
{
	PG_TRY_BLOCK
	float fStartTime = NiGetCurrentTimeInSec();
	_PgOutputDebugString("HandleMessage %d start\n", (PgMobileSuit::InitializeWorkID)pkMsg->PriType());
	if(g_pkApp->doInitializeWork((PgMobileSuit::InitializeWorkID)pkMsg->PriType()))
	{
		_PgOutputDebugString("HandleMessage %d end(%f time elasped)\n", (PgMobileSuit::InitializeWorkID)pkMsg->PriType(), NiGetCurrentTimeInSec() - fStartTime);
		BM::CAutoMutex kLock(m_kMutex);
		m_kLoadOrder.erase(pkMsg->OrderGuid());

		if(m_kLoadOrder.empty())
		{
			IsComplete(true);
			msg_queue_.pulse();
		}
	}
	else
	{
		g_pkApp->QuitApplication();
	}
	PG_CATCH_BLOCK
}

bool PgLoader::WaitLoad()
{
	::MSG kMsg;
	g_kScreenUpdateThread.Initialize();
	while((!IsComplete() || !IsMainComplete() || !IsLogoComplete() || !IsLogoImgComplete()) && IsStop() == false)
	{//메인과 스레드쪽이 끝나면 끝.
		//_PgOutputDebugString("WaitLoad Start\n");
		if(PeekMessage(&kMsg, 0, 0, 0, PM_REMOVE))
		{
			//_PgOutputDebugString("WaitLoad Processing %u message\n", kMsg.message);
			if (kMsg.message == WM_QUIT)
			{
				return false;
			}

			if(!TranslateAccelerator(g_pkApp->GetAppWindow()->GetWindowReference(), g_pkApp->GetAcceleratorReference(), &kMsg))
			{
				TranslateMessage(&kMsg);
				DispatchMessage(&kMsg);
			}
		}

		//_PgOutputDebugString("WaitLoad End\n");
		ACE_Time_Value tv(0, 50000);	tv += ACE_OS::gettimeofday();
		ACE_Message_Block *pMsg;
		msg_queue_.dequeue(pMsg, &tv);
		g_kScreenUpdateThread.ThreadProcedure(NULL);
	}
	g_kScreenUpdateThread.Terminate();

	PgTask<SLoadMessage>::VDeactivate();//스레드 끝.
	
	return true;
}