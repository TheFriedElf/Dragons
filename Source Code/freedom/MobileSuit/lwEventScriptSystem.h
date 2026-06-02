#ifndef FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVNETSCRIPTSYSTEM_H
#define FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVNETSCRIPTSYSTEM_H
#include "PgScripting.h"
#include "PgEventScriptSystem.H"

LW_CLASS(PgEventScriptSystem, EventScriptSystem)

	bool	ActivateEvent(int iEventID,float fAccumTime,float fFrameTime);	//	이미 실행중인 이벤트를 다시 실행하는것은 안된다.
	bool	DeactivateEvent(int iEventID);	
	bool	DeactivateAll();	
	bool	IsNowActivated(int iEventID);
	bool	IsNowActivate();
	bool	IsNowUpdate();
	void	ReloadAll();
	void	Reload(int iEventID);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVNETSCRIPTSYSTEM_H