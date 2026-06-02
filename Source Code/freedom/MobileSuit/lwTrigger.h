#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWTRIGGER_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWTRIGGER_H

#include "lwWString.h"
#include "lwPoint3.h"

class PgTrigger;
class lwActor;

class lwTrigger
{
public:
	lwTrigger(PgTrigger *pkTrigger);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	char const * GetID()const;

	//! 파라메터를 반환한다.
	int GetParam();
	int GetParam2();
	int GetRagUI();

	lwWString GetParamAsString();

	void SetParam(int const iNewParam);
	void SetParam2(int const iNewParam);
	void SetParamAsString(char const* szNewParam);

	bool IsEnable();
	bool IsNil();
	void SetEnable(bool const bEnable = true);
	int GetIndex();

	char const*	GetParamFromParamMap(char const *strKeyString);
	int	GetParamIntFromParamMap(char const *strKeyString);
	bool	SetParamFromParamMap(char const* szKeyString, char const* szValueString);

	int	GetConditionType();
	char* GetConditionAction();

	bool OnAction(lwActor kActor);

	PgTrigger* GetSelf();

	bool IsHavePortalAccess( size_t const iIndex )const;
	bool IsPortalAccess( size_t const iIndex )const;
	int GetPortalAccessName( size_t const iIndex )const;
	
	int GetTriggerType();
	lwPoint3 GetTranslate()const;

	lwWString GetTeleportID();
	int GetErrorMsgID();

	int GetAddedType();
	bool GetDoOnLeaveScript();

	bool IsActiveTime(void) const;

	lwWString GetTriggerReactionSkillName(lwActor kActor);
	int GetTriggerReactionSkillNo(lwActor kActor);

	int GetTriggerSkillID(void) const;

	int GetProgressID(void) const;

	int GetLoadingImageID()const;
protected:
	PgTrigger* m_pkTrigger;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWTRIGGER_H