#include "StdAfx.h"
#include "Utility\ExceptionHandler.h"
#include "Utility\ErrorReportFile.h"
#include "PgMobileSuit.h"
#include "PgProcessChecker.h"

bool PgProcessChecker::PutMsg(MSG &rkMsg, unsigned long ulPriority)
{
	// do nothing;
	return true;
}

void PgProcessChecker::HandleMessage(MSG *pkMsg)
{
	// do nothing;
}

int PgProcessChecker::svc(void)
{
	try {
	while( !IsStop() )//! stop 시그날 걸어서 꺼줄것.
	{
		ACE_Time_Value tv(0, m_delay * 1000);
		tv += ACE_OS::gettimeofday();
		ACE_Message_Block *pMsg;
		msg_queue_.dequeue(pMsg, &tv);
		//_PgOutputDebugString("check process start..... %f\n", NiGetCurrentTimeInSec());
		CheckBadProcess();
		//_PgOutputDebugString("check process end..... %f\n", NiGetCurrentTimeInSec());
	}
	//_PgOutputDebugString("check process end %f\n", NiGetCurrentTimeInSec());
	}
	catch (...)
	{
		Clear();
		return 0;
	}
	Clear();
	return 0;
}