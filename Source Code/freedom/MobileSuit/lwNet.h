#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWNET_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWNET_H

class lwNet
{
public:
	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWNET_H