#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSOUNDMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSOUNDMAN_H

class	PgSoundMan;

class	lwSoundMan
{

public:

	lwSoundMan();

	static bool RegisterWrapper(lua_State *pkState);

	bool	IsNil();

	void	SetEnvSoundDebugRenderingOn();
	void	SetEnvSoundDebugRenderingOff();
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSOUNDMAN_H