#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWWORLDACTIONMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWWORLDACTIONMAN_H

#include "PgWorldActionMan.h"
#include "lwWorldActionObject.h"

class lwWorldActionMan
{
public:
	//! 생성자
	lwWorldActionMan(PgWorldActionMan *pkWorldActionMan);

	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwWorldActionObject	CreateNewWorldActionObject(int iWorldActionID);

	bool	IsNil();

private:

	PgWorldActionMan	*m_pWorldActionMan;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWWORLDACTIONMAN_H