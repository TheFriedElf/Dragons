#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWRENDERMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWRENDERMAN_H

#include "lwGUID.h"

class PgRenderMan;

class lwRenderMan
{
public:
	lwRenderMan(void*);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);
	
	//! 씬을 추가한다.
	void *AddScene(char const *pcID);
	void *AddSceneByNo(int const iMapNo);
	void CleanUpScene(char const *pcID);

	//! 씬을 만든다. (만들기만 한다.)
	void *CreateScene(char const *pcID);

	//! 씬을 삭제한다.
	void RemoveScene(char const *pcWorldID);
	void RemoveAllScene(char const *pcExceptWorld);

	void InvalidateSceneContainer();

	//! 씬을 미리 로딩해 둔다.
	//void PreloadScene(char const *pcID);

	//! 씬을 찾는다.
	void *FindScene(char const *pcID);

	//! 강제로 한번 그린다.
	void Draw();

	void SetGraphicOption(char const* pcOptionName, int iOptionValue);
	void SetUseAlphaGroup(bool bUse);
	void SetFrameDebug(bool bDebug);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWRENDERMAN_H