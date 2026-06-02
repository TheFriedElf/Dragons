#include <stdafx.h>
#include "PgEventTimer.h"
#include "lwEventTimer.h"
#include "PgRenderMan.h"
#include "PgWorld.h"

using namespace lua_tinker;

lwEventTimer lwGetEventTimer()
{
	return lwEventTimer(&g_kEventTimer);
}

lwEventTimer::lwEventTimer(PgEventTimer* pkEventTimer)
{
	m_pkEventTimer = pkEventTimer;
}

bool lwEventTimer::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "GetEventTimer", lwGetEventTimer);

	class_<lwEventTimer>(pkState, "EventTimer")
		.def(pkState, constructor<PgEventTimer *>())
		.def(pkState, "Add", &lwEventTimer::Add)
		.def(pkState, "Del", &lwEventTimer::Del)
		.def(pkState, "Pause", &lwEventTimer::Pause)
		.def(pkState, "Resume", &lwEventTimer::Resume)
		.def(pkState, "Reset", &lwEventTimer::Reset)

		.def(pkState, "AddGlobalEvent", &lwEventTimer::AddGlobalEvent)
		.def(pkState, "DelGlobalEvent", &lwEventTimer::DelGlobalEvent)
		.def(pkState, "PauseGlobalEvent", &lwEventTimer::PauseGlobalEvent)
		.def(pkState, "ResumeGlobalEvent", &lwEventTimer::ResumeGlobalEvent)
		.def(pkState, "ResetGlobalEvent", &lwEventTimer::ResetGlobalEvent)
		;
	return true;
}

bool lwEventTimer::GetSceneID(std::string& rkOut)
{
	return g_kRenderMan.GetFirstTypeID<PgWorld>(rkOut);
}

bool lwEventTimer::Add(char const* szEventID, float const fTime, char const* szScript, int const iRunCount, bool const bImmediateRun)
{
	if( m_pkEventTimer )
	{
		STimerEvent kEvent;
		kEvent.Set(fTime, szScript, iRunCount);
		return m_pkEventTimer->AddLocal(szEventID, kEvent, bImmediateRun);
	}
	return false;
}

bool lwEventTimer::AddGlobalEvent(char const* szEventID, float const fTime, char const* szScript, int const iRunCount, bool const bImmediateRun)
{
	if( m_pkEventTimer )
	{
		STimerEvent kEvent;
		kEvent.Set(fTime, szScript, iRunCount);
		return m_pkEventTimer->AddGlobal(szEventID, kEvent, bImmediateRun);
	}
	return false;
}

bool lwEventTimer::Del(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		std::string kSceneID;
		bool const bFindScendID = GetSceneID(kSceneID);
		if( bFindScendID )
		{
			return m_pkEventTimer->Del(kSceneID.c_str(), szEventID);
		}
	}
	return false;
}

bool lwEventTimer::DelGlobalEvent(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		return m_pkEventTimer->Del("", szEventID);
	}
	return true;
}

bool lwEventTimer::Pause(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		std::string kSceneID;
		bool const bFindScendID = GetSceneID(kSceneID);
		if( bFindScendID )
		{
			return m_pkEventTimer->Pause(kSceneID.c_str(), szEventID);
		}
	}
	return false;
}

bool lwEventTimer::Resume(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		std::string kSceneID;
		bool const bFindScendID = GetSceneID(kSceneID);
		if( bFindScendID )
		{
			return m_pkEventTimer->Resume(kSceneID.c_str(), szEventID);
		}
	}
	return false;
}

bool lwEventTimer::Reset(char const* szEventID, int const iNewCount)
{
	if( m_pkEventTimer )
	{
		std::string kSceneID;
		bool const bFindScendID = GetSceneID(kSceneID);
		if( bFindScendID )
		{
			return m_pkEventTimer->Reset(kSceneID.c_str(), szEventID, iNewCount);
		}
	}
	return false;
}

bool lwEventTimer::PauseGlobalEvent(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		return m_pkEventTimer->Pause("", szEventID);
	}
	return false;
}

bool lwEventTimer::ResumeGlobalEvent(char const* szEventID)
{
	if( m_pkEventTimer )
	{
		return m_pkEventTimer->Resume("", szEventID);
	}
	return false;
}

bool lwEventTimer::ResetGlobalEvent(char const* szEventID, int const iNewCount)
{
	if( m_pkEventTimer )
	{
		return m_pkEventTimer->Reset("", szEventID, iNewCount);
	}
	return false;
}