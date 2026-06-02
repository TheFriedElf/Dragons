#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOT_H

#include "lwActor.h"
#include "lwPacket.h"
#include "lwDropBox.h"
#include "lwUnit.h"

class PgPilot;
class PgDropBox;
class	lwBreakableObject;
class	lwRidingObject;
class	lwObject;

class lwPilot
{
public:
	lwPilot(PgPilot *pkPilot);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	bool IsNil();

	lwGUID GetGuid();

	lwActor GetActor();
	lwObject GetObject();
	lwPoint3 GetPos();
	int	GetState();

	void	SetName(lwWString kName);
	lwWString GetName();

	//! HP를 설정한다.
	void SetHP(int Hp,int iTimeStamp);
	
	//! 능력을 설정한다.
	void SetAbil(int const iAbilType, int const iValue);
	int	GetAbil(int iAbilType);
	void SetAbil64(int AbilType, lwInt64 kInt64);
	lwInt64 lwPilot::GetAbil64(int iAbilType);

	//!	Base Class (전사,법사,궁수,도적) 을 얻어온다.
	int	GetBaseClassID();
	bool IsCorrectClass(int const iReqClassID,bool bNotIncludeSelf);	//	이 캐릭터가 iReqClassID 에 해당하는 캐릭터가 맞는지 체크한다.

	void SetUnit(lwGUID kGUID,int iUnitType,int iClassNo,int iLevel,int iGender);

	//! DropBox를 설정한다.
	void SetDropBox(lwDropBox kDropBox);

	//! 열고 있는 DropBox를 반환한다.
	lwDropBox GetDropBox();

	void SetInfo(lwGUID kMemberGuid, lwWString pcName, BYTE byGender, BYTE byClass);

	//! 자기 펫을 설정한다.
	void SetMyPet(lwGUID kPetGuid);

	//! 자기 펫 Guid를 알려준다.
	lwGUID GetMyPet();

	//! 펫 이름을 바꾼다.
	void ChangePetName(char const *pcName);

	void SetMonsterNo(int iMonNo);

	int GetMonsterNo();

	bool	IsPlayerPilot();	//	플레이어인가
	bool	IsMyPlayerPilot();	//	나의 플레이어인가.

	int	GetUpgradeCount();	//	전직 횟수 리턴
	int	GetUpgradeClass(int iGrade);	//	iGrade 에 해당하는 클래스 번호 리턴

	void	ReanalyseSkillTree();

	//! Gender를 설정
//	void SetGender(int iGender);

	void ChangeClass(BYTE byClass, short int sLevel);
	lwUnit GetUnit();

	PgPilot* operator()();

	// SetTarget , SetGoalPos
	void SetTarget(lwGUID kTarget);
	void SetGoalPos(lwPoint3 kPos);
	int UnitType();
	bool IsUnitType(int iType);
	int GetFrontDirection();
	bool IsAlive();	

	//!자기 커플 GUID반환
	lwGUID GetCoupleGuid();

	lwGUID GetTarget() const;

	bool IsHaveComboAction( int const iCurrentAction );
	int GetNextComboAction( int const iCurrentAction, bool const bSearchChargeAction, int &iNextAction_SC );
	bool IsHaveConnectComboAction(int const iWeaponType, int const iCurrentAction);
	int GetNextConnectComboAction(int const iWeaponType);
	bool GetNextComboList( int const iCurrentActionNO, std::list<int>& rkContNextCombo );
	bool IsUseComboAction( int const iActionNo );
	bool IsChargeCombo( int const iActionNo );
	void InsertInputKey(int const iUKey, float const fStartTime, bool const bPush);
	void ClearInputKey();
	void RefreshInputKey();
	void SetInputKeyState_Release();
	bool HaveInputKeyState_Release();
	bool IsInputAttackKey();
	int GetInputAttackKey();
	bool FindInputKey(int const iUiKey);

	char const* FindActionID(unsigned int uiUKey);
protected:
	PgPilot *m_pkPilot;
};

extern bool MakeCrash_SetAbilFromLuaScript(int const iAbilType);
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWPILOT_H