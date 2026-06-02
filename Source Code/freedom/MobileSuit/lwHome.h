#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHOME_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHOME_H

#include "lwFurniture.h"
#include "lwPoint3.h"
#include "lwGUID.h"

class PgHome;

class lwHome
{
public:
	lwHome(PgHome *pkHome);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! NULL 객체인가?
	bool IsNil();

	//! 진짜 객체를 반환한다.
	PgHome *lwHome::operator()();

	lwPoint3 GetCenterPos();
	lwGUID GetOwnerGuid();
	//void SetOwnerGuid(lwGUID guid);
	lwGUID GetHomeGuid();
	bool IsMyHome();

	// 배치모드로 돌입한다.
	void SetArrangeMode(bool bArrangeMode);

	// 배치모드인가?
	bool IsArrangeMode();

	// 배치할 가구를 설정한다.
	void SetArrangingFurniture(lwFurniture kFurniture);

	// 배치 중인 가구를 반환한다.
	lwFurniture GetArrangingFurniture();

	void AddWall(lwPoint3 kPos, int iRotate);
	void RemoveWall();

	lwFurniture AddFurniture(int iFurnitureNo, lwPoint3 kPos, int iRotate);
	void RemoveAllFurniture();

	bool SetRoomSize(int iX, int iY, int iZ);
	void SetMovingFurniture();

	void RemoveFurniture();
	
	bool IsHavePostbox();
	bool IsHaveSafebox();
	
	void RemoveAllFurnitureOnlyClient();
protected:
	PgHome *m_pkHome;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHOME_H