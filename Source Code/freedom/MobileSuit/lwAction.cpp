#include "stdafx.h"
#include "variant/Global.h"
#include "lwAction.h"
#include "PgAction.h"
#include "lwGUID.h"
#include "lwPoint3.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "lwPilot.H"
#include "lwActor.H"
#include "lwActionResult.H"
#include "variant/PgActionResult.H"
#include "lwActionTargetInfo.H"
#include "lwActionTargetList.H"
#include "lwFindTargetParam.H"
#include "PgOption.H"
#include "Variant/constant.h"
#include "lwUNIT_PTR_ARRAY.h"
#include "lwInputSlotInfo.H"
#include "PgActionFSMFuncs.H"
#include "PgActionPool.h"

lwAction::lwAction(PgAction *pkAction)
{
	if (pkAction == NULL)
	{
		PG_WARNING_LOG(!"pkAction is NULL");
	}

	m_pkAction = pkAction;
}

lwAction lwGetDummyAction()
{
	return lwAction(0);
}

void	lwAct_Idle_DoAutoFire(lwActor kActor)
{
	DoAutoFire(kActor);
}
bool lwAction::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "Act_Idle_DoAutoFire", lwAct_Idle_DoAutoFire);
	def(pkState, "CheckLearnedPrecedentSkill", lwAction::CheckLearnedPrecedentSkill);
	
	class_<lwAction>(pkState, "Action")
		.def(pkState, "CheckTargetExist", &lwAction::CheckTargetExist)
		.def(pkState, "GetID", &lwAction::GetID)
		.def(pkState, "SetID", &lwAction::SetID)
		.def(pkState, "GetEnable", &lwAction::GetEnable)
		.def(pkState, "GetNextActionName", &lwAction::GetNextActionName)
		.def(pkState, "SetNextActionName", &lwAction::SetNextActionName)
		.def(pkState, "SetSlot", &lwAction::SetSlot)
		.def(pkState, "NextSlot", &lwAction::NextSlot)
		.def(pkState, "GetCurrentSlot", &lwAction::GetCurrentSlot)
		.def(pkState, "GetParam", &lwAction::GetParam)
		.def(pkState, "SetParam", &lwAction::SetParam)
		.def(pkState, "GetParamFloat", &lwAction::GetParamFloat)
		.def(pkState, "SetParamFloat", &lwAction::SetParamFloat)
		.def(pkState, "GetParamInt", &lwAction::GetParamInt)
		.def(pkState, "SetParamInt", &lwAction::SetParamInt)
		.def(pkState, "GetParamAsPacket", &lwAction::GetParamAsPacket)
		.def(pkState, "SetParamAsPacket", &lwAction::SetParamAsPacket)
		.def(pkState, "DeleteParamPacket", &lwAction::DeleteParamPacket)
		.def(pkState, "GetParamAsPoint", &lwAction::GetParamAsPoint)
		.def(pkState, "SetParamAsPoint", &lwAction::SetParamAsPoint)
		.def(pkState, "GetScriptParam", &lwAction::GetScriptParam)
		.def(pkState, "GetScriptParamAsInt", &lwAction::GetScriptParamAsInt)
		.def(pkState, "GetScriptParamAsFloat", &lwAction::GetScriptParamAsFloat)		
		.def(pkState, "GetActionParam", &lwAction::GetActionParam)
		.def(pkState, "SetActionParam", &lwAction::SetActionParam)
		.def(pkState, "GetSlotCount", &lwAction::GetSlotCount)
		.def(pkState, "StartTimer", &lwAction::StartTimer)
		.def(pkState, "DeleteTimer", &lwAction::DeleteTimer)
		.def(pkState, "IsLoop", &lwAction::IsLoop)
		.def(pkState, "IsSlotEventScript", &lwAction::IsSlotEventScript)
		.def(pkState, "IsNil", &lwAction::IsNil)
		.def(pkState, "GetActionName", &lwAction::GetActionName)
		.def(pkState, "GetActionType", &lwAction::GetActionType)
		.def(pkState, "ClearGUIDContainer", &lwAction::ClearGUIDContainer)
		.def(pkState, "AddNewGUID", &lwAction::AddNewGUID)
		.def(pkState, "RemoveGUID", &lwAction::RemoveGUID)
		.def(pkState, "CheckGUIDExist", &lwAction::CheckGUIDExist)
		.def(pkState, "GetAbil", &lwAction::GetAbil)
		.def(pkState, "GetTotalGUIDInContainer", &lwAction::GetTotalGUIDInContainer)
		.def(pkState, "GetGUIDInContainer", &lwAction::GetGUIDInContainer)
		.def(pkState, "GetTargetCount", &lwAction::GetTargetCount)
		.def(pkState, "ClearTargetList", &lwAction::ClearTargetList)
		.def(pkState, "GetTargetGUID", &lwAction::GetTargetGUID)
		.def(pkState, "GetTargetSpherePos", &lwAction::GetTargetSpherePos)
		.def(pkState, "GetActionInstanceID", &lwAction::GetActionInstanceID)
		.def(pkState, "GetActionNoByActionID", &lwAction::GetActionNoByActionID)
		.def(pkState, "GetActionNo", &lwAction::GetActionNo)
		.def(pkState, "GetEffectNo", &lwAction::GetEffectNo)
		.def(pkState, "BroadCast", &lwAction::BroadCast)
		.def(pkState, "BroadCastTargetListModify", &lwAction::BroadCastTargetListModify)
		.def(pkState, "GetTargetABVShapeIndex", &lwAction::GetTargetABVShapeIndex)
		.def(pkState, "GetTargetActionResult", &lwAction::GetTargetActionResult)
		.def(pkState, "SetTargetActionResult", &lwAction::SetTargetActionResult)
		.def(pkState, "SetActionInstanceID", &lwAction::SetActionInstanceID)
		.def(pkState, "GetDirection", &lwAction::GetDirection)
		.def(pkState, "SetDirection", &lwAction::SetDirection)
		.def(pkState, "GetSkillLevel", &lwAction::GetSkillLevel)
		.def(pkState, "GetSkillRange", &lwAction::GetSkillRange)
		.def(pkState, "FindTargets", &lwAction::FindTargets)
		.def(pkState, "FindTargetsEx", &lwAction::FindTargetsEx)
		.def(pkState, "FindTargetsEx2", &lwAction::FindTargetsEx2)
		.def(pkState, "AddTarget", &lwAction::AddTarget)
		.def(pkState, "SetTargetList", &lwAction::SetTargetList)
		.def(pkState, "GetTargetList", &lwAction::GetTargetList)
		.def(pkState, "SetDoNotBroadCast", &lwAction::SetDoNotBroadCast)
		.def(pkState, "GetDoNotBroadCast", &lwAction::GetDoNotBroadCast)
		.def(pkState, "SetDoNotPlayAnimation", &lwAction::SetDoNotPlayAnimation)
		.def(pkState, "GetDoNotPlayAnimation", &lwAction::GetDoNotPlayAnimation)
		.def(pkState, "ChangeToNextActionOnNextUpdate", &lwAction::ChangeToNextActionOnNextUpdate)
		.def(pkState, "NextSlot", &lwAction::NextSlot)
		.def(pkState, "PrevSlot", &lwAction::PrevSlot)
		.def(pkState, "GetSlotAnimName", &lwAction::GetSlotAnimName)
		.def(pkState, "GetCurrentSlotAnimName", &lwAction::GetCurrentSlotAnimName)
		.def(pkState, "GetTimeStamp", &lwAction::GetTimeStamp)
		.def(pkState, "GetTargetInfo", &lwAction::GetTargetInfo)
		.def(pkState, "GetActionEnterTime", &lwAction::GetActionEnterTime)
		.def(pkState, "Call_Skill_Result_Func", &lwAction::Call_Skill_Result_Func)
		.def(pkState, "GetActionKeyNo", &lwAction::GetActionKeyNo)
		.def(pkState, "GetEffectCasterGuid", &lwAction::GetEffectCasterGuid)
		.def(pkState, "SetAutoBroadCastWhenCastingCompleted", &lwAction::SetAutoBroadCastWhenCastingCompleted)
		.def(pkState, "SetNoBroadCastTargetList", &lwAction::SetNoBroadCastTargetList)
		.def(pkState, "SetCanChangeActorPos", &lwAction::SetCanChangeActorPos)
		.def(pkState, "SetIgnoreNotActionShift", &lwAction::SetIgnoreNotActionShift)
		.def(pkState, "GetSyncStartPos", &lwAction::GetSyncStartPos)
		.def(pkState, "EnableSyncRun", &lwAction::EnableSyncRun)
		.def(pkState, "IsSyncRunEnabled", &lwAction::IsSyncRunEnabled)
		.def(pkState, "GetActionStartPos", &lwAction::GetActionStartPos)
		.def(pkState, "IsLeft", &lwAction::IsLeft)
		.def(pkState, "IsChangeToNextActionOnNextUpdate", &lwAction::IsChangeToNextActionOnNextUpdate)
		.def(pkState, "GetCurrentStage", &lwAction::GetCurrentStage)
		.def(pkState, "NextStage", &lwAction::NextStage)
		.def(pkState, "GetElapsedTime", &lwAction::GetElapsedTime)
		.def(pkState, "CreateActionTargetList", &lwAction::CreateActionTargetList)
		.def(pkState, "SetActionTerm", &lwAction::SetActionTerm)
		.def(pkState, "GetInputSlotInfo", &lwAction::GetInputSlotInfo)
		.def(pkState, "SetNextActionInputSlotInfo", &lwAction::SetNextActionInputSlotInfo)
		.def(pkState, "CheckCanEnter", &lwAction::CheckCanEnter)
		.def(pkState, "CanBreakFall", &lwAction::CanBreakFall)
		.def(pkState, "SetThrowRayStatus", &lwAction::SetThrowRayStatus)
		.def(pkState, "GetEffectCaster", &lwAction::GetEffectCaster)
		.def(pkState, "GetTranslateSkillCnt", &lwAction::GetTranslateSkillCnt)
		.def(pkState, "IsSlotLoop", &lwAction::IsSlotLoop)
		.def(pkState, "GetSkillNo", &lwAction::GetSkillNo)
		.def(pkState, "GetParentSkillNo", &lwAction::GetParentSkillNo)
		.def(pkState, "IsSkillFireType", &lwAction::IsSkillFireType)
		;

	def(pkState, "GetDummyAction", &lwGetDummyAction);
	def(pkState, "InitActionXML", &lwInitActionXML);
	
	return true;
}

void lwAction::SetNextActionInputSlotInfo(lwInputSlotInfo kInputSklotInfo)
{
	if(m_pkAction)
	{
		m_pkAction->SetNextActionInputSlotInfo(kInputSklotInfo());
	}
}

bool lwAction::CanBreakFall() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionOptionEnable(PgAction::AO_BREAKFALL);
	}

	return false;
}

lwInputSlotInfo	lwAction::GetInputSlotInfo() const
{
	if(m_pkAction)
	{
		return	lwInputSlotInfo(const_cast<PgInputSlotInfo*>(m_pkAction->GetInputSlotInfo()));
	}

	return	lwInputSlotInfo(0);
}

bool	lwAction::IsLeft()
{
	//return	m_pkAction->IsLeft();
	return true;
}

lwPoint3 lwAction::GetActionStartPos() const
{
	if(m_pkAction)
	{
		return lwPoint3(m_pkAction->GetActionStartPos());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

lwPoint3 lwAction::GetSyncStartPos() const
{
	if(m_pkAction)
	{
		return lwPoint3(m_pkAction->GetSyncStartPos());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

void lwAction::EnableSyncRun(bool const bEnable)
{
	if(m_pkAction)
	{
		m_pkAction->EnableSyncRun(bEnable);
	}
}

bool lwAction::IsSyncRunEnabled() const
{
	if(m_pkAction)
	{
		return m_pkAction->IsSyncRunEnabled();
	}

	return false;
}

int	lwAction::FindTargets(int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets)
{
	if(m_pkAction == NULL)
	{
		return 0;
	}

	PgActionTargetList	*pkIgnoreList= kIgnoreTargets(),*pkFoundTargetList = kout_FoundTargets();
	
	return	PgAction::FindTargets(m_pkAction->GetActionNo(),(PgAction::TargetAcquireType)iTargetAcquireType,
		kParam.m_kStart, kParam.m_kDir, kParam.m_fRange, kParam.m_fWideAngle, kParam.m_fEffectRadius, 
		pkIgnoreList, kParam.m_iMaxTargets, kParam.m_bNoDuplication, *pkFoundTargetList, NULL, true);
}
int	lwAction::FindTargetsEx(int iActionNo,int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets)
{
	return	FindTargetsEx2(
		iActionNo,
		iTargetAcquireType,
		kParam,
		kIgnoreTargets,
		kout_FoundTargets,
		lwPilot(NULL));
}
int	lwAction::FindTargetsEx2(int iActionNo,int iTargetAcquireType,lwFindTargetParam kParam,lwActionTargetList kIgnoreTargets,lwActionTargetList kout_FoundTargets,lwPilot kCasterPilot)
{
	PgActionTargetList	*pkIgnoreList= kIgnoreTargets(),*pkFoundTargetList = kout_FoundTargets();
	
	return	PgAction::FindTargets(iActionNo,(PgAction::TargetAcquireType)iTargetAcquireType,
		kParam.m_kStart, kParam.m_kDir, kParam.m_fRange, kParam.m_fWideAngle, kParam.m_fEffectRadius,
		pkIgnoreList, kParam.m_iMaxTargets, kParam.m_bNoDuplication, *pkFoundTargetList,kCasterPilot(), true);
}

void lwAction::SetTargetList(lwActionTargetList kActionTargetList)
{
	if(m_pkAction)
	{
		return	m_pkAction->SetTargetList(*(kActionTargetList()));
	}
}

lwActionTargetList lwAction::GetTargetList() const
{
	if(m_pkAction)
	{
		return	lwActionTargetList(m_pkAction->GetTargetList());
	}

	return lwActionTargetList(0);
}

void lwAction::SetAutoBroadCastWhenCastingCompleted(bool const bTrue)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_AUTO_BROADCAST_WHEN_CASTING_COMPLETED,bTrue);
	}
}
void lwAction::SetNoBroadCastTargetList(bool bNoBroadCast)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_NO_BROADCAST_TARGETLIST,bNoBroadCast);
	}
}
void lwAction::SetCanChangeActorPos(bool const bTrue)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_CAN_CHANGE_ACTOR_POS,bTrue);
	}
}
void lwAction::SetIgnoreNotActionShift(bool const bTrue)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_IGNORE_NOTACTIONSHIFT,bTrue);
	}
}

int	lwAction::Call_Skill_Result_Func(lwPilot kCasterPilot,lwUNIT_PTR_ARRAY kTargetArray,lwActionResultVector kActionResult)
{
	if(m_pkAction)
	{
		return	m_pkAction->Call_Skill_Result_Func(kCasterPilot(),kTargetArray(),kActionResult());
	}

	return 0;
}

void lwAction::SetActionInstanceID()
{
	if(m_pkAction)
	{
		m_pkAction->SetActionInstanceID();
	}
}
PgAction* lwAction::operator()()
{
	if(m_pkAction)
	{
		return	m_pkAction;
	}

	return NULL;
}
void lwAction::BroadCast(lwPilot kPilot)	//	서버로 전송하여 다른 유저에게 브로드캐스트 하도록 한다.
{
	if(m_pkAction)
	{
		m_pkAction->BroadCast(kPilot());
	}
}
void lwAction::BroadCastTargetListModify(lwPilot kPilot, bool bisTargetEmptyToBroadCast)	//	타겟 리스트가 바뀌었음을 서버에 알린다.
{
	if(m_pkAction)
	{
		m_pkAction->BroadCastTargetListModify(kPilot(), bisTargetEmptyToBroadCast);
	}
}

void lwAction::SetDoNotPlayAnimation(bool const bDoNotPlayAnimation)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_NO_PLAY_ANIMATION, bDoNotPlayAnimation);
	}
}

void lwAction::SetThrowRayStatus(bool const bStatus)
{
	if(m_pkAction)
	{
		m_pkAction->SetThrowRayStatus(bStatus);
	}
}

bool lwAction::GetDoNotPlayAnimation()
{
	if(m_pkAction)
	{
		return m_pkAction->GetActionOptionEnable(PgAction::AO_NO_PLAY_ANIMATION);
	}

	return false;
}

bool lwAction::CheckCanEnter(lwActor kActor, char const* pkActionID, bool const bShowFailMsg)
{
	if(m_pkAction)
	{
		return m_pkAction->CheckCanEnter(kActor(),pkActionID,bShowFailMsg);
	}

	return false;
}

bool lwAction::CheckTargetExist(lwActor kActor, char const* pkActionID)
{
	if(m_pkAction)
	{
		return	m_pkAction->CheckTargetExist(kActor(), pkActionID);
	}

	return false;
}

void lwAction::CreateActionTargetList(lwActor kActor, bool const bNoCalcActionResult, bool const bForcedCreation)
{
	if(m_pkAction)
	{
		m_pkAction->CreateActionTargetList(kActor(), m_pkAction, bNoCalcActionResult, bForcedCreation);
	}
}

void lwAction::ChangeToNextActionOnNextUpdate(bool const bDoNotBroadCastCurrentAction, bool const bDoNotPlayCurrentAnimation)	//	다음업데이트시에 자동으로 NextAction  으로 전이합니다.
{
	if(m_pkAction)
	{
		m_pkAction->ChangeToNextActionOnNextUpdate();
		m_pkAction->SetActionOption(PgAction::AO_NO_BROADCAST, bDoNotBroadCastCurrentAction);
		m_pkAction->SetActionOption(PgAction::AO_NO_PLAY_ANIMATION, bDoNotPlayCurrentAnimation);
	}
}
bool lwAction::IsChangeToNextActionOnNextUpdate()
{
	if(m_pkAction)
	{
		return	m_pkAction->IsChangeToNextActionOnNextUpdate();
	}

	return false;
}

//!	브로드캐스팅 하지 않을것인가?
void lwAction::SetDoNotBroadCast(bool const bDoNotBroadCast)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionOption(PgAction::AO_NO_BROADCAST,bDoNotBroadCast);
	}
}

bool lwAction::GetDoNotBroadCast() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionOptionEnable(PgAction::AO_NO_BROADCAST);
	}

	return false;
}

int	lwAction::GetActionNo() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionNo();
	}

	return 0;
}

int	lwAction::GetEffectNo() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetEffectNo();
	}

	return 0;
}

int	lwAction::GetSkillLevel() const
{
	if(m_pkAction == NULL)
	{
		return 1;
	}

	CSkillDef const* pkSkillDef = m_pkAction->GetSkillDef();
	if(pkSkillDef)
	{
		return pkSkillDef->m_byLv;
	}

	return 1;
}

int	lwAction::GetTimeStamp() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetTimeStamp();	
	}

	return 0;
}

int	lwAction::GetActionNoByActionID(char const* pkActionID) const
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	int	iSkillID = kSkillDefMgr.GetSkillNoFromActionName(std::wstring(UNI(pkActionID)));
	return iSkillID;
}

char const* lwAction::GetID() const
{
	if(!m_pkAction)
	{
		return "Null_Action";
	}

	return m_pkAction->GetID().c_str();
}

void lwAction::SetID(char const* pcName)
{
	if(m_pkAction)
	{
		m_pkAction->SetID(pcName);
	}
}

bool lwAction::GetEnable() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetEnable();
	}

	return false;
}

char const* lwAction::GetNextActionName() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetNextActionName().c_str();
	}

	return "";
}

char const* lwAction::GetActionType() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetActionType().c_str();
	}

	return "";
}

void lwAction::SetNextActionName(char const* pcActionName)
{
	if(m_pkAction)
	{
		m_pkAction->SetNextActionName(pcActionName);
	}
}

bool lwAction::SetSlot(int const iSlot)
{
	if(m_pkAction)
	{
		return m_pkAction->SetSlot(iSlot);
	}
	return false;
}
//! Get Ability
int	lwAction::GetAbil(int const iAbilType) const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetAbil(iAbilType);
	}

	return 0;
}

bool lwAction::NextSlot()
{
	if(m_pkAction)
	{
		return m_pkAction->NextSlot();
	}

	return 0;
}
float lwAction::GetActionEnterTime() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionEnterTime();
	}

	return 0.0f;
}

int lwAction::GetSlotCount() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetSlotCount();
	}

	return 0;
}
//!	ActionType 에 해당하는 액션의 NiKeyboardInput 번호를 리턴한다.
int	lwAction::GetActionKeyNo(int const iActionType)	//	iActionType : 0 : ATTACK 1 : JUMP 2 : USE
{
	static int iUKey[] = {ACTIONKEY_ATTACK, ACTIONKEY_JUMP, ACTIONKEY_USE};

	return g_kGlobalOption.GetUKeyToKey(iUKey[iActionType]);
}

int	lwAction::GetActionParam() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionParam();
	}

	return 0;
}
void lwAction::SetActionParam(int const iParam)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionParam(iParam);
	}
}

char const* lwAction::GetActionName()
{
	static std::string ActionName;
	if(m_pkAction)
	{
		if(m_pkAction->GetActionName(ActionName) == false)
		{
			return "";
		}
	}

	return ActionName.c_str();
}

int lwAction::GetCurrentSlot() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetCurrentSlot();
	}

	return 0;
}
//! GUID 컨테이너를 클리어한다.
void lwAction::ClearGUIDContainer()
{
	if(m_pkAction)
	{
		m_pkAction->ClearGUIDContainer();
	}
}
//! GUID 컨테이너에 GUID 를 추가한다.
void lwAction::AddNewGUID(lwGUID newGUID)
{
	if(m_pkAction)
	{
		m_pkAction->AddNewGUID(newGUID.GetGUID());
	}
}

//! GUID 컨테이너에서 하나를 삭제한다.
void lwAction::RemoveGUID(lwGUID newGUID)
{
	if(m_pkAction)
	{
		m_pkAction->RemoveGUID(newGUID.GetGUID());
	}
}

//! GUID 컨테이너에 GUID 가 존재하는지 체크한다.
bool lwAction::CheckGUIDExist(lwGUID GUID)
{
	if(m_pkAction)
	{
		return	m_pkAction->CheckGUIDExist(GUID.GetGUID());
	}

	return false;
}
int	lwAction::GetTotalGUIDInContainer() const
{
	if(m_pkAction)
	{
		return m_pkAction->GetTotalGUIDInContainer();
	}

	return 0;
}
lwGUID lwAction::GetGUIDInContainer(int const iIndex) const
{
	if(m_pkAction)
	{
		BM::GUID const* pGUID = m_pkAction->GetGUIDInContainer(iIndex);
		if(pGUID)
		{
			return lwGUID(*pGUID);
		}
	}

	return	lwGUID(NULL);
}
lwGUID	lwAction::GetEffectCasterGuid()	const
{
	if(m_pkAction)
	{
		return lwGUID(m_pkAction->GetEffectCasterGuid());
	}

	return	lwGUID(NULL);
}
bool lwAction::SetParam(unsigned int const uiIndex, char const* pcParam)
{
	if(m_pkAction)
	{
		return m_pkAction->SetParam(uiIndex, pcParam);
	}
	return false;
}

char const* lwAction::GetParam(unsigned int const uiIndex) const
{
	if(m_pkAction)
	{
		return m_pkAction->GetParam(uiIndex);
	}

	return "";
}

bool lwAction::SetParamFloat(unsigned int const uiIndex, float const fParam)
{	
	if(m_pkAction)
	{
		static char szBuff[100];
		NiSprintf(szBuff, 100, "%f", fParam);
		return m_pkAction->SetParam(uiIndex, szBuff);
	}

	return false;
}

float lwAction::GetParamFloat(unsigned int const uiIndex) const
{
	char const* pcParam = NULL;
	if(m_pkAction)
	{		
		if(strcmp((pcParam = m_pkAction->GetParam(uiIndex)), "null") == 0)
		{
			return 0.0f;
		}
	}

	return (float)atof(pcParam);
}
//! 파라메터를 설정한다.
bool lwAction::SetParamInt(unsigned int const uiIndex, int const iNum)
{
	if(!m_pkAction)
	{
		return false;
	}
	static char szBuff[32];
	NiSprintf(szBuff, 31, "%d", iNum);
	return	m_pkAction->SetParam(uiIndex, szBuff);
}

//! 파라메터를 반환한다.
int lwAction::GetParamInt(unsigned int const uiIndex) const
{
	char const* pcParam = NULL;
	if(m_pkAction)
	{
		if(!(pcParam = m_pkAction->GetParam(uiIndex)))
		{
			return 0;
		}
	}

	return atoi(pcParam);
}

lwPoint3 lwAction::GetParamAsPoint(int const iIndex) const
{
	if(m_pkAction)
	{
		return lwPoint3(m_pkAction->GetParamAsPoint(iIndex));
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

void lwAction::SetParamAsPoint(int const iIndex, lwPoint3 kPoint)
{
	if(m_pkAction)
	{
		m_pkAction->SetParamAsPoint(iIndex, kPoint());
	}
}

void lwAction::SetParamAsPacket(lwPacket kPacket)
{
	if(m_pkAction)
	{
		m_pkAction->SetParamAsPacket(kPacket());
	}
}

lwPacket lwAction::GetParamAsPacket() const
{
	if(m_pkAction)
	{
		return lwPacket(m_pkAction->GetParamAsPacket());
	}

	return lwPacket(0);
}

void lwAction::DeleteParamPacket()
{
	if(m_pkAction)
	{
		m_pkAction->DeleteParamPacket();
	}
}
bool lwAction::StartTimer(float const fTotalDuration, float const fInterval, int const iTimerID)
{
	if(m_pkAction)
	{
		return	m_pkAction->StartTimer(fTotalDuration,fInterval,iTimerID);
	}

	return false;
}
void lwAction::DeleteTimer(int const iTimerID)
{
	if(m_pkAction)
	{
		m_pkAction->DeleteTimer(iTimerID);
	}
}

bool lwAction::IsLoop()
{
	if(m_pkAction)
	{
		return m_pkAction->GetActionOptionEnable(PgAction::AO_LOOP);
	}

	return false;
}

bool lwAction::IsSlotEventScript()const
{
	if(m_pkAction)
	{
		return m_pkAction->IsSlotEventScript();
	}

	return false;
}

char* lwAction::GetScriptParam(char const* strParamName) const
{
	if(m_pkAction)
	{
		return	(char*)m_pkAction->GetScriptParam(std::string(strParamName));
	}

	return "";
}

int	lwAction::GetScriptParamAsInt(char const* strParamName) const
{
	if(m_pkAction)
	{
		char* strParam = GetScriptParam(strParamName);
		return	atoi(strParam);
	}

	return 0;
}

float lwAction::GetScriptParamAsFloat(char const* strParamName) const
{
	if(m_pkAction)
	{
		char* strParam = GetScriptParam(strParamName);
		return	atof(strParam);
	}

	return 0.0f;
}


void lwAction::PrevSlot()
{
	if(m_pkAction)
	{
		m_pkAction->PrevSlot();
	}
}
char* lwAction::GetCurrentSlotAnimName()
{
	if(m_pkAction)
	{
		static std::string kActionName;
		if(m_pkAction->GetActionName(kActionName))
		{
			return (char*)kActionName.c_str();
		}
	}

	return "";
}
char* lwAction::GetSlotAnimName(int const iSlotNum)
{
	if(m_pkAction)
	{
		static	std::string kActionName;
		if(m_pkAction->GetActionName(kActionName,iSlotNum))
		{
			return	(char*)kActionName.c_str();
		}
	}

	return "";
}

bool lwAction::IsNil()
{
	return (m_pkAction == 0 ? true : false);
}

int	lwAction::GetSkillRange(int const iActionNo,lwActor kActor, bool const bCalcAdjustSkillEffect)
{
	if(m_pkAction)
	{
		CSkillDef const* pSkillDef = NULL;

		if (iActionNo == 0 )
		{
			pSkillDef = m_pkAction->GetSkillDef();
		}
		else
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			pSkillDef = kSkillDefMgr.GetDef(iActionNo);
		}

		if(!pSkillDef)
		{
			return 0;
		}

		int iAttackRange = GetSkillRangeEx(pSkillDef,kActor);
		if(bCalcAdjustSkillEffect	// 스킬 조정 이펙트를 고려해서 스킬을 얻어온다면
			&& !kActor.IsNil())
		{// 이펙트가 걸려있는지 확인한 후 범위 다시 계산
			PgPilot* pkPilot =kActor()->GetPilot();
			if(pkPilot)
			{//파일럿
				CUnit* pkUnit = pkPilot->GetUnit();
				if(pkUnit)
				{ //유닛
					if(UT_ENTITY == pkUnit->UnitType())
					{// 스킬 제한 이펙트가
						PgPilot* pkCallerPilot= g_kPilotMan.FindPilot(pkUnit->Caller());
						if(pkCallerPilot)
						{// 실제 소환자의 이펙트중 스킬 제한 이펙트가 있으면, Entity의 스킬 범위에도 적용해준다.
							CUnit* pkCallerUnit = pkCallerPilot->GetUnit();
							if(pkCallerUnit)
							{
								iAttackRange = PgAdjustSkillFunc::CalcAdjustSkillValue(EASCT_RANGE, GetActionNo(), pkCallerUnit, iAttackRange);
							}
						}
					}
				}
			}
		}
		return iAttackRange;
	}

	return 0;
}


int	lwAction::GetSkillRangeEx(CSkillDef const* pkSkillDef, lwActor kActor)
{
	if(kActor.IsNil())
	{
		return 0;
	}

	PgPilot* pkPilot = kActor()->GetPilot();
	if(!pkPilot)
	{//파일럿
		return 0;
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return 0;
	}

	return PgAdjustSkillFunc::GetAttackRange(pkUnit, pkSkillDef);
}

int	lwAction::GetTargetCount() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetTargetList()->size();
	}

	return 0;
}
void lwAction::ClearTargetList()
{
	if(m_pkAction)
	{
		m_pkAction->ClearTargetList();
	}
}
void lwAction::AddTarget(lwGUID kGuid,int const iABVIndex,lwActionResult kActionResult)
{
	if(m_pkAction)
	{
		m_pkAction->AddTarget(kGuid(),iABVIndex,kActionResult());
	}
}

void lwAction::SetTargetActionResult(lwActionResultVector kActionResult, bool bDoNotSetEffect)
{
	if(m_pkAction)
	{
		m_pkAction->SetTargetActionResult(kActionResult(),bDoNotSetEffect);
	}
}

lwGUID lwAction::GetTargetGUID(int const iIndex) const
{
	if(m_pkAction)
	{
		PgActionTargetList* pTargetList = m_pkAction->GetTargetList();

		int iCount = 0;
		for(ActionTargetList::const_iterator itor = pTargetList->begin(); itor != pTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				return	lwGUID((*itor).GetTargetPilotGUID());
			}
			++iCount;
		}
	}
	return	lwGUID(NULL);
}
int	lwAction::GetTargetABVShapeIndex(int const iIndex) const
{
	if(m_pkAction)
	{
		PgActionTargetList* pTargetList = m_pkAction->GetTargetList();

		int iCount = 0;
		for(ActionTargetList::const_iterator itor = pTargetList->begin(); itor != pTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				return	(*itor).GetSphereIndex();
			}
			++iCount;
		}
	}
	return	0;
}
lwActionTargetInfo lwAction::GetTargetInfo(int const iIndex)
{
	if(m_pkAction)
	{
		PgActionTargetList* pTargetList = m_pkAction->GetTargetList();

		int iCount = 0;
		for(ActionTargetList::iterator itor = pTargetList->begin(); itor != pTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				return lwActionTargetInfo(&(*itor));
			}
			++iCount;
		}
	}
	return	lwActionTargetInfo(NULL);	
}

lwActionResult lwAction::GetTargetActionResult(int const iIndex)
{
	if(m_pkAction)
	{
		PgActionTargetList* pTargetList = m_pkAction->GetTargetList();

		int iCount = 0;
		for(ActionTargetList::iterator itor = pTargetList->begin(); itor != pTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				return	lwActionResult(&(*itor).GetActionResult());
			}
			++iCount;
		}
	}
	return	lwActionResult(NULL);	
}

lwPoint3 lwAction::GetTargetSpherePos(int const iIndex) const
{
	if(m_pkAction)
	{
		PgActionTargetList* pTargetList = m_pkAction->GetTargetList();

		int iCount = 0;
		for(ActionTargetList::const_iterator itor = pTargetList->begin(); itor != pTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				PgActionTargetInfo const*  pTarget = &(*itor);

				PgPilot* pkTargetPilot = g_kPilotMan.FindPilot(pTarget->GetTargetPilotGUID());
				if(pkTargetPilot)
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
					if(pkActor)
					{
						PgIWorldObjectBase::ABVShape *pShape = pkActor->GetABVShape(pTarget->GetSphereIndex());
						if(pShape)
						{
							NiNodePtr spNode = (NiNode*)pkActor->GetNIFRoot()->GetObjectByName(pShape->m_kTo);
							if(spNode)
								return	lwPoint3(spNode->GetWorldTranslate());
						}
					}
				}
				return	lwPoint3(0,0,0);

			}
			++iCount;
		}
	}
	return	lwPoint3(0,0,0);
}
int	lwAction::GetActionInstanceID() const
{
	if(m_pkAction)
	{
		return	m_pkAction->GetActionInstanceID();
	}

	return 0;
}

BYTE lwAction::GetDirection() const
{
	if(!m_pkAction)
	{
		return DIR_NONE;
	}

	return m_pkAction->GetDirection();
}

void lwAction::SetDirection(BYTE byDir)
{
	if(!m_pkAction)
	{
		return;
	}
	m_pkAction->SetDirection(byDir);
}

int lwAction::GetCurrentStage() const
{
	if(!m_pkAction)
	{
		return 0;
	}

	return m_pkAction->GetCurrentStage();
}

void lwAction::NextStage()
{
	if(!m_pkAction)
	{
		return;
	}

	m_pkAction->NextStage();
}

DWORD lwAction::GetElapsedTime() const
{
	if(!m_pkAction)
	{
		return 0;
	}

	return m_pkAction->GetElapsedTime();
}

void lwAction::SetActionTerm(int const iTerm)
{
	if(m_pkAction)
	{
		m_pkAction->SetActionTerm(iTerm);
	}
}


lwActor	lwAction::GetEffectCaster()
{
	if(!m_pkAction)	
	{
		return lwActor(NULL);
	}

	return lwActor(g_kPilotMan.FindActor(m_pkAction->GetEffectCasterGuid()));
}

bool lwAction::CheckLearnedPrecedentSkill(char const* skillName)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	int const iSkillNo = kSkillDefMgr.GetSkillNoFromActionName( UNI(skillName) );
	if( 0 != iSkillNo )//스킬있고
	{
		CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if(pkSkillDef)
		{
			for (int i = 0; i < LEAD_SKILL_COUNT; ++i)
			{
				int const iNo = pkSkillDef->GetAbil(AT_NEED_SKILL_01+i);
				if (0!=iNo)
				{
					int const iKey = g_kSkillTree.GetKeySkillNo(iNo);
					PgSkillTree::stTreeNode* pNode = g_kSkillTree.GetNode(iKey);
					if (pNode)
					{
						if( !pNode->IsLearned() && !pNode->m_bTemporaryLearned )
						{//안배웠고, 임시로도 안배웠으면
							return false;
						}
					}
				}
			}

			return true;
		}
	}
	return false;
}

int lwAction::GetTranslateSkillCnt()
{
	if(!m_pkAction)	
	{
		return 0;
	}
	return m_pkAction->GetTranslateSkillCnt();
}

bool lwAction::IsSlotLoop()
{
	if(!m_pkAction)
	{
		return false;
	}
	return m_pkAction->IsSlotLoop();
}

void lwInitActionXML()
{
#ifndef EXTERNAL_RELEASE // 익스터널 버전에서는 보이지 않음
	g_kActionPool.Destroy();
#endif
}

int	lwAction::GetSkillNo() const
{
	if(m_pkAction == NULL)
	{
		return 0;
	}

	CSkillDef const* pkSkillDef = m_pkAction->GetSkillDef();
	if(pkSkillDef)
	{
		return pkSkillDef->No();
	}

	return 0;
}


int	lwAction::GetParentSkillNo() const
{
	if(m_pkAction == NULL)
	{
		return 0;
	}

	CSkillDef const* pkSkillDef = m_pkAction->GetSkillDef();
	if(pkSkillDef)
	{		
		return pkSkillDef->GetParentSkill();
	}

	return 0;
}

bool lwAction::IsSkillFireType(int const iType)const
{
	return ((GetAbil(AT_FIRE_TYPE) & iType) == iType) ? true : false;
}
