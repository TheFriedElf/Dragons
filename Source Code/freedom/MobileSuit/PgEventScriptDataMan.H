#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATAMAN_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATAMAN_H

#include "PgEventScriptData.H"

typedef	std::vector<PgEventScriptData*> EventScriptDataCont;

class	PgEventScriptDataMan
{
	EventScriptDataCont	m_kEventScriptDataCont;

public:

	void	Init();
	void	Terminate();

public:

	PgEventScriptData*	GetEventScriptData(int iEventID);

	void	ReloadAll();	//	실행중인 스크립트는 재로딩 할 수 없다.
	void	Reload(int iEventID);
	void	ReleaseAll();	//	모든 스크립트를 메모리 해제시킨다.
};

extern	PgEventScriptDataMan	g_kEventScriptDataMan;

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATAMAN_H