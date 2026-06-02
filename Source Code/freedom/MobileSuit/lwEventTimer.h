#ifndef FREEDOM_DRAGONICA_UTIL_TIMER_LWEVENTIMER_H
#define FREEDOM_DRAGONICA_UTIL_TIMER_LWEVENTIMER_H

class PgEventTimer;

class lwEventTimer
{
public:
	lwEventTimer(PgEventTimer* pkEventTimer);

	static bool RegisterWrapper(lua_State *pkState);

	bool Add(char const* szEventID, float const fTime, char const* szScript, int const iRunCount = 1, bool const bImmediateRun = false);
	bool Del(char const* szEventID);
	bool Pause(char const* szEventID);
	bool Resume(char const* szEventID);
	bool Reset(char const* szEventID, int const iNewCount = 0);


	bool AddGlobalEvent(char const* szEventID, float const fTime, char const* szScript, int const iRunCount = 1, bool const bImmediateRun = false);
	bool DelGlobalEvent(char const* szEventID);
	bool PauseGlobalEvent(char const* szEventID);
	bool ResumeGlobalEvent(char const* szEventID);
	bool ResetGlobalEvent(char const* szEventID, int const iNewCount = 0);

protected:
	bool GetSceneID(std::string& rkOut);

protected:
	PgEventTimer* m_pkEventTimer;
};
#endif // FREEDOM_DRAGONICA_UTIL_TIMER_LWEVENTIMER_H