#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTION_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTION_H

#include "lwGUID.h"

class PgAction;
class lwPoint3;
class lwPilot;
class lwActor;
class lwActionResultVector;
class lwActionResult;
class lwActionTargetList;
class lwActionTargetInfo;
class lwFindTargetParam;
class lwUNIT_PTR_ARRAY;
class lwPacket;
class	PgActor;
class	lwInputSlotInfo;
class lwAction
{
public:
	
	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! 생성자
	lwAction(PgAction *pkAction);

	PgAction* operator()();

	//! 액션의 이름을 반환한다.
	char const* GetID() const;

	//! 액션의 이름을 설정한다.
	void SetID(char const* pcName);

	//! 액션에 대해서 탈출이 시도된다(키가 떨어졌을 때)
	bool GetEnable() const;

	//! 지정된 다음 엑션을 반환한다.
	char const *GetNextActionName() const;

	//! 다음 엑션을 설정한다.
	void SetNextActionName(char const *pcActionName);
	void SetNextActionInputSlotInfo(lwInputSlotInfo kInputSklotInfo);

	//! 지정한 애니메이션 슬롯 번호를 플레이 한다.
	bool SetSlot(int const iSlot);

	//! Get Ability
	int	GetAbil(int const iAbilType) const;

	void	BroadCast(lwPilot kPilot);	//	서버로 전송하여 다른 유저에게 브로드캐스트 하도록 한다.
	void	BroadCastTargetListModify(lwPilot kPilot, bool bisTargetEmptyToBroadCast);	//	타겟 리스트가 바뀌었음을 서버에 알린다.

	//!	브로드캐스팅 하지 않을것인가?
	void	SetDoNotBroadCast(bool const bDoNotBroadCast);
	bool	GetDoNotBroadCast() const;
	void	SetDoNotPlayAnimation(bool const constbDoNotPlayAnimation);
	bool	GetDoNotPlayAnimation();
	void	ChangeToNextActionOnNextUpdate(bool const bDoNotBroadCastCurrentAction, bool const bDoNotPlayCurrentAnimation);	//	다음업데이트시에 자동으로 NextAction  으로 전이합니다.
	bool	IsChangeToNextActionOnNextUpdate();

	void	CreateActionTargetList(lwActor kActor, bool const bNoCalcActionResult, bool const bForcedCreation = false);

	//! 다음 애니메이션 슬롯을 플레이 한다.
	bool	NextSlot();
	void	PrevSlot();
	char*	GetCurrentSlotAnimName();
	char*	GetSlotAnimName(int const iSlotNum);

	bool	CanBreakFall() const;
	void	SetThrowRayStatus(bool const bStatus);

	//! 현재 플레이 중인 애니메이션 슬롯 번호를 가져온다.
	int GetCurrentSlot() const;

	//! Action에 등록되어 있는 슬롯의 개수를 반환한다.
	int GetSlotCount() const;

	//!	ActionType 에 해당하는 액션의 NiKeyboardInput 번호를 리턴한다.
	int	GetActionKeyNo(int const iActionType);	//	iActionType : 0 : ATTACK 1 : JUMP 2 : USE

	//	액션이 시작된 시간을 리턴한다.
	float	GetActionEnterTime() const;

	char const *GetActionType() const;

	//! GUID 컨테이너를 클리어한다.
	void	ClearGUIDContainer();
	//! GUID 컨테이너에 GUID 를 추가한다.
	void	AddNewGUID(lwGUID newGUID);
	//! GUID 컨테이너에서 하나를 삭제한다.
	void	RemoveGUID(lwGUID newGUID);
	//! GUID 컨테이너에 GUID 가 존재하는지 체크한다.
	bool	CheckGUIDExist(lwGUID GUID);
	int		GetTotalGUIDInContainer() const;
	lwGUID	GetGUIDInContainer(int const iIndex) const;

	//! 현재 액션의 이름을 리턴한다. 
	char const* GetActionName();

	int		GetActionParam() const;
	void	SetActionParam(int const iParam);

	int	GetSkillLevel() const;

	int	FindTargets(int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets);
	int	FindTargetsEx(int iActionNo, int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets);
	int	FindTargetsEx2(int iActionNo, int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets,lwPilot kCasterPilot);

	int	Call_Skill_Result_Func(lwPilot kCasterPilot,lwUNIT_PTR_ARRAY kTargetArray,lwActionResultVector kActionResult);

	//! TODO : 스택이 없으므로 임시로 파라메터를 n개로 쓸 수 있음.

	//! 파라메터를 설정한다.
	bool SetParam(unsigned int const uiIndex, char const *pcParam);

	//! 파라메터를 반환한다.
	char const* GetParam(unsigned int const constuiIndex) const;

	//! 파라메터를 설정한다.
	bool SetParamFloat(unsigned int const uiIndex, float const fFloat);

	//! 파라메터를 반환한다.
	float GetParamFloat(unsigned int const uiIndex) const;

	//! 파라메터를 설정한다.
	bool SetParamInt(unsigned int const uiIndex, int const iNum);

	//! 파라메터를 반환한다.
	int GetParamInt(unsigned int const uiIndex) const;

	lwGUID	GetEffectCasterGuid()	const;

	void		SetParamAsPacket(lwPacket kPacket);
	lwPacket	GetParamAsPacket() const ;
	void		DeleteParamPacket();

	void		SetParamAsPoint(int iIndex, lwPoint3 kPoint);
	lwPoint3	GetParamAsPoint(int iIndex) const;

	//	Timer
	bool	StartTimer(float const fTotalDuration, float const fInterval, int const iTimerID);
	void	DeleteTimer(int const iTimerID);

	//! Action이 Loop인가?
	bool IsLoop();

	//! Action에서 SlotEventScript를 사용할 것인가?
	bool IsSlotEventScript()const;

	//! Action이 Null인가?
	bool IsNil();

	char*	GetScriptParam(char const* strParamName) const;
	int		GetScriptParamAsInt(char const* strParamName) const;
	float	GetScriptParamAsFloat(char const* strParamName) const;

	int		GetTimeStamp() const;

	void	SetAutoBroadCastWhenCastingCompleted(bool const bTrue);

	void	SetNoBroadCastTargetList(bool bNoBroadCast);
	void	SetCanChangeActorPos(bool const bTrue);
	void	SetIgnoreNotActionShift(bool const bTrue);
	
	int	GetActionNoByActionID(char const* pkActionID) const;
	int	GetActionNo() const;
	int	GetEffectNo() const;

	bool CheckTargetExist(lwActor kActor, char const* pkActionID);
	bool CheckCanEnter(lwActor kActor, char const* pkActionID, bool const bShowFailMsg);

	int					GetTargetCount() const;
	lwGUID				GetTargetGUID(int const iIndex) const;
	lwPoint3			GetTargetSpherePos(int const iIndex) const;
	lwActionResult		GetTargetActionResult(int const iIndex);
	lwActionTargetInfo	GetTargetInfo(int const iIndex);
	int					GetTargetABVShapeIndex(int const iIndex) const;

	void ClearTargetList();

	int			GetSkillRange(int const iActionNo,lwActor kActor, bool const bCalcAdjustSkillEffect = false);
	static	int	GetSkillRangeEx(CSkillDef const* pkSkillDef, lwActor kActor);

	void				AddTarget(lwGUID kGuid, int const iABVIndex,lwActionResult kActionResult);
	void				SetTargetList(lwActionTargetList kActionTargetList);
	lwActionTargetList	GetTargetList() const;

	void				SetTargetActionResult(lwActionResultVector kActionResult, bool bDoNotSetEffect);

	lwInputSlotInfo	GetInputSlotInfo() const;

	int	 GetActionInstanceID() const;
	void SetActionInstanceID();

	lwPoint3	GetSyncStartPos() const;
	lwPoint3	GetActionStartPos() const;

	void EnableSyncRun(bool bEnable);
	bool IsSyncRunEnabled() const;

	bool	IsLeft();

	BYTE	GetDirection() const;
	void SetDirection(BYTE byDir);
	int		GetCurrentStage() const;
	void	NextStage();
	DWORD	GetElapsedTime() const;
	void	SetActionTerm(int const iTerm);
	lwActor	GetEffectCaster();

	static bool CheckLearnedPrecedentSkill(char const* skillName);
	
	int GetTranslateSkillCnt();

	bool IsSlotLoop();
	int GetSkillNo() const;
	int GetParentSkillNo() const;

	bool IsSkillFireType(int const iType)const;
private:
	PgAction *m_pkAction;
};

extern lwAction lwGetDummyAction();
extern void lwInitActionXML();
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTION_H