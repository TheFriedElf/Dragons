#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOTMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOTMAN_H
#include "lwPilot.h"
#include "lwUnit.h"

class PgPilotMan;

class lwPilotMan
{
public:
	lwPilotMan(PgPilotMan *pkPilotMan);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwPilot NewPilot(lwGUID kGuid, int iClass, int iGender,char const* pcObjectType);
	
	lwPilot FindPilot(lwGUID kGuid);
	lwPilot FindPilotByName(lwWString kName, bool bFindPassPlayerPilot = false);
	lwPilot FindPilotInRange(int iUnitType, lwPoint3 kTargetLoc, int iRange);
	lwActor FindActorByClassNo(int iClassNo);
	bool IsReservedPilot(lwGUID kGuid);
	lwPilot	GetPlayerPilot();
	lwUnit GetPlayerUnit(void);
	bool IsMyPlayer( lwGUID kGuid );

	bool InsertPilot(lwGUID rkGuid, lwPilot kPilot);

	bool RemovePilot(lwGUID kGuid);
	bool RemoveReservedPilot(lwGUID kGuid);

	void SetPlayerPilotGuid(lwGUID kGuid);
	lwGUID GetPlayerPilotGuid();

	//void SetLockMyInput(bool bLock);
	lwActor GetPlayerActor();
	bool IsLockMyInput();
	lwGUID GetHyperJumpGuid();
	void ClearHyperJumpInfo();
	
	bool BroadcastDirection(lwPilot kPilot, int iDir);
	bool IsPremiumService()const;
	int GetPremiumGrade()const;

	bool IsJumpingEvent()const;
	int GetJumpingCreateCharCount()const;
	int GetJumpingRemainCount()const;
	bool IsCreateJumpingCharClass(int const iClassNo)const;
	int GetCreateJumpingCharClass(int const iIdx)const;
	int GetCreateJumpingCharLevel(int const iIdx)const;
	void SetJumpingSelectIdx(int const iIdx);
	void NotUseJumpingEvent();
	void SetHideBalloon(bool const bHide);
protected:
	PgPilotMan *m_pkPilotMan;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOTMAN_H