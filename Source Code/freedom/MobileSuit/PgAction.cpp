#include "stdafx.h"
#include "Variant/PgControlDefMgr.h"
#include "variant/PgActionResult.H"
#include "variant/Global.h"
#include "PgActor.H"
#include "PgXmlLoader.h"
#include "PgInput.h"
#include "PgAction.h"
#include "PgActor.h"
#include "lwActor.h"
#include "lwAction.h"
#include "PgLocalManager.h"
#include "PgSkillTargetMan.h"
#include "PgParticleMan.h"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "PgWorld.H"
#include "lwActionTargetList.H"
#include "PgMobileSuit.H"
#include "PgActionEffect.H"
#include "PgClientParty.H"
#include "PgActionPool.H"
#include "PgChatMgrClient.h"
#include "PgActionTargetList.H"
#include "PgActionFSMFuncMgr.H"

#include "lwActionResult.H"
#include "lwUnit.h"
#include "lwUnit_ptr_array.h"
#include "lwPilot.H"
#include "PgObject.H"
#include "PgEnergyGuage.H"
#include "PgPOTParticle.h"

char const* ACTIONNAME_IDLE = "a_idle";
char const* ACTIONNAME_BIDLE = "a_battle_idle";
char const* ACTIONNAME_RUN	=	"a_run";
char const* ACTIONNAME_JUMP = "a_jump";
char const* ACTIONNAME_AWAKE_CHARGE = "a_AwakeCharge";
char const* ACTIONNAME_RP_IDLE = "a_rp_idle";
char const* ACTIONNAME_RP_WALK = "a_rp_walk";
char const* ACTIONNAME_RP_JUMP = "a_rp_jump";
char const* ACTIONNAME_BLOCK = "a_block";

char const* ACTIONTYPE_JOBSKILL = "JOB_SKILL";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgAction
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int	PgAction::ms_iActionInstanceIDGenerator = 0;

PgAction::PgAction() 
{ 
	Clear();
}

PgAction::PgAction(PgAction *pkSrcAction)
{
	Init(pkSrcAction);
}

PgAction::~PgAction()
{
	DeleteParamPacket();
	Clear();
}
void PgAction::CopyFrom(PgAction *pkSrcAction)
{
	m_ActionTargetList = *pkSrcAction->GetTargetList();
	m_iActionInstanceID = pkSrcAction->GetActionInstanceID();
	m_byActionParam = pkSrcAction->GetActionParam();
	m_iActionNo = pkSrcAction->GetActionNo();
	m_iEffectNo = pkSrcAction->GetEffectNo();
	m_SkillType = pkSrcAction->GetSkillType();
	m_dwTimeStamp = pkSrcAction->GetTimeStamp();
	m_kStartRandomSeedCallCounter = pkSrcAction->StartRandomSeedCallCounter();	
	m_fActionEnterTime = pkSrcAction->GetActionEnterTime();
	m_bActionFinished = pkSrcAction->GetActionFinished();
	m_kNextActionName = pkSrcAction->GetNextActionName();
	m_pkNextActionInputSlotInfo = const_cast<PgInputSlotInfo*>(pkSrcAction->GetNextActionInputSlotInfo());
	m_GuidContainer = *pkSrcAction->GetGUIDContainer();
	m_kActionType = pkSrcAction->GetActionType();
	m_kSlotContainer = pkSrcAction->m_kSlotContainer;
	m_kParamContainer = pkSrcAction->m_kParamContainer;
	m_kPointContainer = pkSrcAction->m_kPointContainer;
	m_kFSMScript = pkSrcAction->GetFSMScript();
	m_ScriptParamMap = pkSrcAction->m_ScriptParamMap;
	m_bEnable = pkSrcAction->GetEnable();
	m_bChangeToNextActionOnNextUpdate = pkSrcAction->IsChangeToNextActionOnNextUpdate();
	m_uiCurrentSlot = pkSrcAction->GetCurrentSlot();
	m_ActionOption = pkSrcAction->GetActionOption();
	m_pkInputSlotInfo = const_cast<PgInputSlotInfo*>(pkSrcAction->GetInputSlotInfo());
	m_kActionStartPos = pkSrcAction->GetActionStartPos();
	m_kSyncStartPos = pkSrcAction->GetSyncStartPos();
	m_bSyncRunEnabled = pkSrcAction->IsSyncRunEnabled();
	m_byMovingDirection = pkSrcAction->GetDirection();
	m_bRecord = pkSrcAction->IsRecord();
	m_bAlreadySync = pkSrcAction->AlreadySync();
	m_dwBirthTime = pkSrcAction->GetBirthTime();
	m_dwActionTerm = pkSrcAction->GetActionTerm();
	m_iStage = pkSrcAction->GetCurrentStage();
	m_iProjectileUID = 0;
	m_pkSkillDef = pkSrcAction->GetSkillDef();
	m_pkFSMFunc = pkSrcAction->GetFSMFunc();
	m_iTranslateSkillCnt = pkSrcAction->m_iTranslateSkillCnt;

	DeleteParamPacket();

	if(pkSrcAction->GetParamAsPacket())
	{
		BM::Stream *pkExtraPacket = new BM::Stream();
		pkExtraPacket->Push(*pkSrcAction->GetParamAsPacket());
		SetParamAsPacket(pkExtraPacket);
	}

}

bool PgAction::Init(const PgAction *pkSrcAction)
{
	Clear();
	m_iActionNo = pkSrcAction->GetActionNo();
	m_iEffectNo = pkSrcAction->GetEffectNo();
	m_kID = pkSrcAction->m_kID;
	m_kNextActionName = pkSrcAction->m_kNextActionName;
	m_GuidContainer = pkSrcAction->m_GuidContainer;
	m_kActionType = pkSrcAction->m_kActionType;
	m_kSlotContainer = pkSrcAction->m_kSlotContainer;
	m_kParamContainer = pkSrcAction->m_kParamContainer;
	m_kFSMScript = pkSrcAction->m_kFSMScript;
	m_pkFSMFunc = pkSrcAction->m_pkFSMFunc;

	SetActionOption(pkSrcAction->GetActionOption());

	m_bEnable = pkSrcAction->m_bEnable;

	m_uiCurrentSlot = pkSrcAction->m_uiCurrentSlot;
	m_ScriptParamMap = pkSrcAction->m_ScriptParamMap;
	m_SkillType = pkSrcAction->m_SkillType;
	m_pkSkillDef = pkSrcAction->m_pkSkillDef;

	SetActionOption(AO_AUTO_BROADCAST_WHEN_CASTING_COMPLETED,true);
	SetActionOption(AO_NO_PLAY_ANIMATION,false);
	SetActionOption(AO_NO_BROADCAST,false);
	SetActionOption(AO_NO_BROADCAST_TARGETLIST,false);

	m_kActionStartPos = NiPoint3::ZERO;
	m_byMovingDirection = DIR_NONE;
	m_bRecord = pkSrcAction->m_bRecord;
	m_dwBirthTime = pkSrcAction->m_dwBirthTime;
	m_dwActionTerm = pkSrcAction->m_dwActionTerm;
	m_bAlreadySync = pkSrcAction->m_bAlreadySync;
	m_iStage = pkSrcAction->m_iStage;

	m_iProjectileUID = 0;
	m_iTranslateSkillCnt = pkSrcAction->m_iTranslateSkillCnt;

	return false;
}

void PgAction::Clear()
{ 

	m_kID = "";

	m_ActionTargetList.clear();
	m_ActionTargetList.SetActionEffectApplied(false);

	m_iActionInstanceID = 0;
	m_byActionParam = 0;
	m_iActionNo = 0;
	m_iEffectNo = 0;
	m_pkParamPacket = 0;

	m_dwBirthTime = 0;
	m_dwActionTerm = 0;
	
	m_iStage = 0;
	m_dwTimeStamp = 0;
	m_kStartRandomSeedCallCounter = 0;
	m_fActionEnterTime = 0.f;
	SetActionFinished(false);

	m_bAddToActionEntity = false;

	ChangeToNextActionOnNextUpdate(false);

	m_pkInputSlotInfo = NULL;

	m_kNextActionName = "";
	m_pkNextActionInputSlotInfo = NULL;

	m_GuidContainer.clear();
	m_kActionType = "Unspecified";
	m_kSlotContainer.clear();
	m_kParamContainer.clear();
	m_kPointContainer.clear();

	m_kFSMScript = "";
	m_pkFSMFunc = NULL;
	m_ScriptParamMap.clear();

	SetActionOption(AO_NONE);
	m_pkSkillDef = NULL;

	m_bEnable = false;

	m_uiCurrentSlot = 0;				//! 현재 플레이 중인 애니메이션의 슬롯 번호

	m_iProjectileUID = 0;
	
	m_kActionStartPos = NiPoint3::ZERO;

	m_byMovingDirection = DIR_NONE;
	m_bAlreadySync = false;
	m_bRecord = false;

	ClearTimer();

	EnableSyncRun(false);

	bRayStatus = false;

	m_iSkillCoolTime = 0;
	m_fSkillCoolTimeRate = 0.f;
	m_iSkillCastingTime = 0;
	m_iTranslateSkillCnt = 0;
}

void PgAction::SetActionInstanceID()
{
	m_iActionInstanceID = GenerateNextActionInstanceID();
}
void PgAction::SetActionInstanceID(int const iInstanceID)
{
	m_iActionInstanceID = iInstanceID;
}
int	PgAction::Call_Skill_Result_Func(PgPilot* pkCasterPilot,UNIT_PTR_ARRAY *pkTargetArray,PgActionResultVector *pkActionResultVec)
{
	//	일단 결과 전부 생성하자.

	lwUNIT_PTR_ARRAY	kArray(pkTargetArray);
	int	iTotalTarget = pkTargetArray->size();
	for(int i=0;i<iTotalTarget;++i)
	{
		lwUnit	kUnit = kArray.GetUnit(i);
		pkActionResultVec->GetResult(kUnit.GetGuid()(),true)->SetInvalid(false);
	}

	if(pkCasterPilot)
	{		
		//return lua_tinker::call<int, lwPilot,lwUNIT_PTR_ARRAY,int, lwActionResultVector>(funcname,lwPilot(pkCasterPilot),lwUNIT_PTR_ARRAY(pkTargetArray),GetActionNo(),lwActionResultVector(pkActionResultVec));
		CUnit* pkCasterUnit = pkCasterPilot->GetUnit();
		if (pkCasterUnit != NULL)
		{
			bool IsPlayer = pkCasterUnit->IsUnitType(UT_PLAYER) || pkCasterUnit->IsUnitType(UT_PET) || pkCasterUnit->IsUnitType(UT_SUB_PLAYER);
			if(false==IsPlayer)
			{
				if(pkCasterUnit->IsUnitType(UT_ENTITY))
				{
					BM::GUID kPlayerGuid;
					if( g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid) )
					{
						IsPlayer = pkCasterUnit->Caller() == kPlayerGuid;
					}
				}
			}

			char funcname[100];
			if(IsPlayer)
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				sprintf_s(funcname,100,"GetSkillResult%u", kSkillDefMgr.GetCallSkillNum(GetActionNo()));
			}
			else
			{
				sprintf_s(funcname,100,"GetSkillResult%u", GetActionNo());
			}

			return lua_tinker::call<int, lwUnit,lwUNIT_PTR_ARRAY,int, lwActionResultVector>(funcname, lwUnit(pkCasterUnit),lwUNIT_PTR_ARRAY(pkTargetArray),GetActionNo(),lwActionResultVector(pkActionResultVec));
		}
		else
		{
			return 0;
		}
	}
	return 0;
}
bool PgAction::CheckTargetExist(PgActor* pkActor,char const *kActionName)
{
	PgAction* pkTempAction = g_kActionPool.CreateAction(kActionName);

	PgActionTargetList	kNewTargetList;
	kNewTargetList.SetActionInfo(pkActor->GetPilotGuid(),pkTempAction->GetActionInstanceID(),pkTempAction->GetActionNo(),0);

	int	iFound = 0;
	if(GetFSMFunc())
	{
		iFound = GetFSMFunc()->OnFindTarget(lwActor(pkActor),lwAction(pkTempAction),lwActionTargetList(&kNewTargetList));
	}
	else
	{
//#ifndef EXTERNAL_RELEASE
//		if( lua_tinker::call<bool>("GetUseScript"))
//#else
		if(!GetActionOptionEnable(AO_USE_CFUNC_ONTARGET))
//#endif
		{
			iFound = lua_tinker::call<int, lwActor, lwAction,lwActionTargetList>((pkTempAction->GetFSMScript() + "_OnFindTarget").c_str(), lwActor(pkActor),lwAction(pkTempAction),lwActionTargetList(&kNewTargetList));
		}
		else
		{
			iFound = OnFindTargetFunc(pkActor, pkTempAction, kNewTargetList);
		}
	}

	g_kActionPool.ReleaseAction(pkTempAction);

	return iFound>0;
}
bool PgAction::CheckCanEnter(PgActor* pkActor,char const *kActionName,bool const bShowFailMsg)
{
	if(NULL == pkActor)
	{
		return	false;
	}

	PgAction* pkTempAction = g_kActionPool.CreateAction(kActionName);
	if(NULL == pkTempAction)
	{
		return	false;
	}

	// 나 일 경우 실제 사용해야하는 액션 레벨로 세팅한다.
	if(pkActor->IsUnderMyControl())
	{
		int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(pkTempAction);
		PgSkillTree::stTreeNode const* pkNode = g_kSkillTree.GetNode(iKeySkillNo);
		if(pkNode)
		{
			CSkillDef const* pkDef = pkNode->GetSkillDef();
			if(pkDef)
			{
				int const iLevel = pkDef->GetAbil(AT_LEVEL);
				int iActionNo = pkTempAction->GetActionNo() + iLevel - 1;

				// 아이템에 의해서 스킬이 오버레벨 되는 경우
				if(pkActor->GetPilot() && pkActor->GetPilot()->GetUnit())
				{
					PgPlayer* pkPlayer = static_cast<PgPlayer*>(pkActor->GetPilot()->GetUnit());
					int const iLearnedSkill = pkPlayer->GetMySkill()->GetLearnedSkill(iActionNo, true);
					if(iLearnedSkill > iActionNo)
					{
						iActionNo = iLearnedSkill;
					}
				}

				pkTempAction->SetActionNo(iActionNo);
			}
		}
		else
		{
			//트리에 없어도 사용가능한 스킬이 있다.
			int const iNeedSkillNo = pkTempAction->GetAbil(AT_NEED_SKILL_01);
			PgSkillTree::stTreeNode *pkNode2 = g_kSkillTree.GetNode(iNeedSkillNo);
			if(pkNode2)
			{
				int iLevel = pkNode2->GetOriginalSkillLevel();
				// 아이템에 의해서 스킬이 오버레벨 되는 경우
				if(pkActor->GetPilot() && pkActor->GetPilot()->GetUnit())
				{
					PgPlayer const* pkPlayer = static_cast<PgPlayer const*>(pkActor->GetPilot()->GetUnit());
					int const iLearnedSkill = pkPlayer->GetMySkill()->GetLearnedSkill(pkNode2->GetOriginalSkillNo(), true);
					if(iLearnedSkill > pkNode2->GetOriginalSkillNo())
					{
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkNextSkillDef = kSkillDefMgr.GetDef(iLearnedSkill);
						if(pkNextSkillDef)
						{
							iLevel = pkNextSkillDef->GetAbil(AT_LEVEL);
						}
					}
				}

				CSkillDef const* pkDef = pkNode2->GetSkillDef();
				if(pkDef)
				{
					if(EST_PASSIVE == pkDef->GetType())
					{						
						int const iNextLevelSkillNo = pkTempAction->GetActionNo() + iLevel - 1;
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkNextSkillDef = kSkillDefMgr.GetDef(iNextLevelSkillNo);
						//다음 레벨에 해당되는 스킬이 있을 경우 세팅 없다면 1레벨짜리 스킬이다.
						if(NULL != pkNextSkillDef && iNextLevelSkillNo > pkTempAction->GetActionNo())
						{
							pkTempAction->SetActionNo(iNextLevelSkillNo);
						}
					}
				}
			}
		}
	}

	if(false == pkActor->CheckRequirementForAction(pkTempAction,bShowFailMsg))
	{
		g_kActionPool.ReleaseAction(pkTempAction);
		return false;
	}

	if(false == pkTempAction->CheckCanEnter(pkActor,pkTempAction))
	{
		g_kActionPool.ReleaseAction(pkTempAction);
		return false;
	}

	g_kActionPool.ReleaseAction(pkTempAction);
	return true;
}

void PgAction::CreateActionTargetList(PgActor* pkActor,PgAction* pkAction,bool const bNoCalcActionResult, bool const bForcedCreation)
{
	if( (!pkActor || false == pkActor->IsUnderMyControl())
		&& false == bForcedCreation)
	{
		return;
	}

	if(pkAction == NULL)
	{
		pkAction = this;
	}

	ClearTargetList();	//	현재 타겟리스크 클리어

	PgActionTargetList	kNewTargetList;
	kNewTargetList.SetActionInfo(pkActor->GetPilotGuid(),pkAction->GetActionInstanceID(),pkAction->GetActionNo(),0);

	int	iFound = 0;
	if(GetActionOptionEnable(AO_USE_CFUNC_ONTARGET))
	{
		iFound = OnFindTargetFunc(pkActor, pkAction, kNewTargetList);
	}
	else
	{
		if(GetFSMFunc())
		{
			iFound = GetFSMFunc()->OnFindTarget(lwActor(pkActor),lwAction(pkAction),lwActionTargetList(&kNewTargetList));
		}
		else
		{
			iFound = lua_tinker::call<int, lwActor, lwAction,lwActionTargetList>((m_kFSMScript + "_OnFindTarget").c_str(), lwActor(pkActor),lwAction(pkAction),lwActionTargetList(&kNewTargetList));
		}
	}

	PgActionTargetInfo* pkTargetInfo = NULL;
	PgPilot*			pkTargetPilot= NULL;
	PgActor*			pkTargetActor= NULL;

	UNIT_PTR_ARRAY kUnitPtrArray;
	PgActionResultVector	kActionResultVec;
		
	for(ActionTargetList::iterator itor = kNewTargetList.begin();itor != kNewTargetList.end();++itor)
	{		
		PgActionResult* pkActionResult = NULL;
			
		pkTargetInfo = &(*itor);
		pkTargetPilot = g_kPilotMan.FindPilot(pkTargetInfo->GetTargetPilotGUID());
		if(pkTargetPilot)
		{
			pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
			if(pkTargetActor)
			{
				kUnitPtrArray.Add(pkTargetPilot->GetUnit());
				if(bNoCalcActionResult)
				{
					AddTarget(pkTargetInfo->GetTargetPilotGUID(),pkTargetActor->GetHitObjectABVIndex(),NULL);
				}
				else
				{
					pkActionResult = kActionResultVec.GetResult(pkTargetInfo->GetTargetPilotGUID(),true);
				}
			}
		}
	}

	if(false == bNoCalcActionResult)
	{
		if(pkActor->GetPilot() && pkActor->GetPilot()->GetUnit())
		{
			StartRandomSeedCallCounter(pkActor->GetPilot()->GetUnit()->RandomSeedCallCounter());
		}

		if(!kUnitPtrArray.empty())
		{
			//서버에서도 GetSkillResult는 한번만 호출된다. 최종 엔트리에 한번만 적용 시켜주어야 한다.
			Call_Skill_Result_Func(pkActor->GetPilot(),&kUnitPtrArray,&kActionResultVec);

			for(UNIT_PTR_ARRAY::iterator itor2 = kUnitPtrArray.begin();itor2 != kUnitPtrArray.end(); ++itor2)
			{
				PgActionResult* pkActionResult = kActionResultVec.GetResult((*itor2).pkUnit->GetID());
				pkTargetPilot = g_kPilotMan.FindPilot((*itor2).pkUnit->GetID());
				if(pkTargetPilot)
				{
					pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
					if(pkTargetActor)
					{
						if(pkActionResult && false == pkActionResult->GetInvalid())
						{
							AddTarget((*itor2).pkUnit->GetID(), pkTargetActor->GetHitObjectABVIndex(),pkActionResult);
						}
					}
				}
			}
		}
	}
}
bool PgAction::StartTimer(float const fTotalDuration, float const fInterval, int const iTimerID)
{
	if(!g_pkWorld)
	{
		return false;
	}

	//	동일한 ID의 타이머가 있다면 삭제한다.
	DeleteTimer(iTimerID);

	stActionTimer	kNewTimer;
	kNewTimer.m_fInterval = fInterval;
	kNewTimer.m_fStartTime = g_pkWorld->GetAccumTime();
	kNewTimer.m_fTotalDuration = fTotalDuration;
	kNewTimer.m_iTimerID = iTimerID;

	m_TimerCont.push_back(kNewTimer);

	return true;
}
void PgAction::ClearTimer()
{
	m_TimerCont.clear();
}
void PgAction::DeleteTimer(int const iTimerID)
{
	stActionTimer* pkTimer = NULL;

	for(TimerCont::iterator itor = m_TimerCont.begin(); itor != m_TimerCont.end(); ++itor)
	{
		pkTimer = &(*itor);
		if(iTimerID == pkTimer->m_iTimerID)
		{
			m_TimerCont.erase(itor);
			return;
		}
	}
}
void PgAction::UpdateTimer(PgActor* pkActor,float fAccumTime)
{
	stActionTimer	*pkTimer = NULL;
	float	fNowTime = fAccumTime;
	float	fLastCallTime=0.0f;
	float	fCallTime=0.0f;	
	float	fElapsedTime=0.0f;
	int		iNeedCallTime=0;

	bool bContinue = false;
	for(TimerCont::iterator itor = m_TimerCont.begin(); itor != m_TimerCont.end();)
	{
		pkTimer = &(*itor);

		fElapsedTime = fNowTime-pkTimer->m_fStartTime;
		if(fElapsedTime>pkTimer->m_fTotalDuration)
		{
			fElapsedTime = pkTimer->m_fTotalDuration;
		}

		iNeedCallTime = (int)(fElapsedTime/pkTimer->m_fInterval) - pkTimer->m_iLastCallTime;

		fLastCallTime = pkTimer->m_fStartTime+pkTimer->m_iLastCallTime*pkTimer->m_fInterval;
		
		bContinue = false;
		for(int i = 0; i < iNeedCallTime; ++i)
		{
			fCallTime = fLastCallTime + (i + 1) * pkTimer->m_fInterval;
			if (m_kFSMScript.size() == 0)
			{
				NILOG(PGLOG_ERROR, "[PgAction] CurrentActionSript is not set, but timer is called\n");
			}

			if(m_kFSMScript.size() != 0)
			{
				bool bResult = false;
				if(GetFSMFunc())
				{
					bResult = GetFSMFunc()->OnTimer(lwActor(pkActor), lwAction(this),fCallTime,pkTimer->m_iTimerID);
				}
				else
				{
					bResult = lua_tinker::call<bool, lwActor, float, lwAction,int >((m_kFSMScript + "_OnTimer").c_str(), lwActor(pkActor), fCallTime,lwAction(this),pkTimer->m_iTimerID);
				}

				if(!bResult)
				{
					itor = m_TimerCont.erase(itor);
					bContinue = true;
					break;
				}
			}
			
		}
		if(bContinue)
		{
			continue;
		}

		if(fElapsedTime == pkTimer->m_fTotalDuration)
		{
			itor = m_TimerCont.erase(itor);
			continue;
		}
		
		++itor;

		pkTimer->m_iLastCallTime += iNeedCallTime;
	}
}
void PgAction::SetInputSlotInfo(PgInputSlotInfo *pkInputSlotInfo)
{
	m_pkInputSlotInfo = pkInputSlotInfo;
}
PgInputSlotInfo const* PgAction::GetInputSlotInfo()	const
{
	return m_pkInputSlotInfo;
}

int	PgAction::GetActionInstanceID() const
{
	return m_iActionInstanceID;
}

bool PgAction::AlreadySync()
{
	return m_bAlreadySync;
}

void PgAction::AlreadySync(bool bSync)
{
	m_bAlreadySync = bSync;
}

void PgAction::ClearTargetList()
{
	m_ActionTargetList.clear();
	m_ActionTargetList.SetActionEffectApplied(false);
}

bool PgAction::IsCorrectTargetWithTargetType(int iActionNo,PgActor *pkTargetActor,PgActor *pkCasterActor)
{
	if(!pkTargetActor)
	{
		return false;
	}

	PgActor* pkActor = NULL;
	if(pkCasterActor)
	{
		pkActor = pkCasterActor;
	}
	else
	{
		pkActor  = g_kPilotMan.GetPlayerActor();
	}
	if(!pkActor)
	{
		return false;
	}

	PgPilot* pkPilot = pkActor->GetPilot();
	if(!pkPilot)
	{
		return false;
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return false;
	}

	PgPilot* pkTargetPilot = pkTargetActor->GetPilot();
	if(!pkTargetPilot)
	{
		return false;
	}

	CUnit* pkTargetUnit = pkTargetPilot->GetUnit();
	if(!pkTargetUnit)
	{
		return false;
	}

	if(UT_OBJECT == pkTargetUnit->UnitType())
	{
		PgObject* pkObject = dynamic_cast<PgObject*>(pkTargetActor);
		if(pkObject && pkObject->HasAttribute(EObj_Attr_Breakable) == false)	//	부술수 있는 오브젝트가 아니라면 return false!
		{
			return false;
		}

		int ObjectTeam = pkTargetUnit->GetAbil(AT_TEAM);
		if( ObjectTeam > 0 )
		{
			if( pkUnit->GetAbil(AT_TEAM) == ObjectTeam )
			{	// 같은 팀 어빌을 가진 오브젝트는 return false!
				return false;
			}
		}
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iActionNo);
	if(!pkSkillDef)
	{
		NILOG(PGLOG_ERROR, "[PgAction] %d action no, can't find SkillDef\n", iActionNo);
		return false;
	}

	int const iTargetType = pkSkillDef->GetTargetType();
	bool bIsCorrectTarget = false;

	//	ESTARGET_SELF
	if(pkTargetPilot == pkPilot && (iTargetType & ESTARGET_SELF) )
	{
		bIsCorrectTarget = true;
	}

	if(iTargetType & ESTARGET_SUMMONED)
	{
		bIsCorrectTarget = pkUnit->IsSummonUnit(pkTargetUnit->GetID());
	}
	
	//	ESTARGET_ENEMY
	//	적인가 아닌가?
	bool bIsEnemy = pkActor->IsEnemy(pkTargetActor);

#ifndef EXTERNAL_RELEASE
	//	싱글모드에서는 무조건 몬스터이다.
	bool bIsSingleMode = g_pkApp->IsSingleMode();
	if(bIsSingleMode && pkTargetPilot != pkPilot)
	{
		bIsEnemy = true;
	}
#endif

	if(bIsEnemy && (iTargetType & ESTARGET_ENEMY))
	{
		bIsCorrectTarget = true;
	}

	if(!bIsEnemy && (iTargetType & ESTARGET_ALLIES) && pkTargetPilot != pkPilot)
	{
		bIsCorrectTarget = true;
	}
	//	TT_PARTY
	bool bIsParty = true;
	if( !pkActor->IsMyActor()  || false == pkTargetUnit->IsUnitType(UT_PLAYER) || !g_kParty.IsMember(pkTargetPilot->GetGuid()) )
	{
		//Pvp에서는 스킬 타겟 타입이 파티일 경우
		if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP))
		{
			if(!bIsEnemy)
			{
				bIsParty = true;
			}
			else
			{
				bIsParty = false;
			}
		}
		else
		{
			bIsParty = false;
		}		
	}

	if(bIsParty && (iTargetType & ESTARGET_PARTY))
	{
		if(pkPilot == pkTargetPilot) //나도 파티원	
		{
			// ME 옵션이 없으면 타겟에서는 제외 되어야 한다.
			if(iTargetType & ESTARGET_SELF)
			{
				bIsCorrectTarget = true;
			}
			else
			{
				bIsCorrectTarget = false;
			}
		}
		else
		{
			bIsCorrectTarget = true;
		}
	}
	if( (iTargetType & ESTARGET_ENEMY) && pkUnit->GetAbil(AT_DUEL) > 0 &&
		pkUnit != pkTargetUnit && pkUnit->GetAbil(AT_DUEL) == pkTargetUnit->GetAbil(AT_DUEL))
	{
		bool bTargetIsNotMyCaller = true;
		bool bImNotTargetsCaller = true;
		bool bDiffCaller = true;
		if(pkUnit->Caller().IsNotNull() && pkUnit->Caller() == pkTargetUnit->GetID())
		{
			bTargetIsNotMyCaller = false; //타겟이 내 주인이다.
		}
		if(pkTargetUnit->Caller().IsNotNull() && pkTargetUnit->Caller() == pkUnit->GetID())
		{
			bImNotTargetsCaller = false; //내가 타겟의 주인이다.
		}
		if(pkUnit->Caller().IsNotNull() && pkTargetUnit->Caller().IsNotNull() && pkUnit->Caller() == pkTargetUnit->Caller())
		{
			bDiffCaller = false; //내 주인과 타겟의 주인이 같다.
		}
		if(bTargetIsNotMyCaller && bImNotTargetsCaller && bDiffCaller)
		{
			bIsCorrectTarget = true;
		}

		//if(pkUnit->Caller() != pkTargetUnit->GetID() && pkTargetUnit->Caller() != pkUnit->GetID()) //소환체가 주인을 때릴수 없고 PC가 자기 소환체를 때릴수 없다
		//{
		//	bIsCorrectTarget = true;
		//}
	}

	//	TT_HIDDEN
	if(pkTargetActor->IsInvisible() && !(iTargetType & ESTARGET_HIDDEN))
	{
		bIsCorrectTarget = false;
	}

	//	TT_DEAD
	if(!(iTargetType & ESTARGET_DEAD) && pkTargetActor->GetActorDead())
	{
		bIsCorrectTarget = false;
	}

	// 죽은 대상에 쓰는 스킬 & 살아 있는 액터
	if(iTargetType & ESTARGET_DEAD && !pkTargetActor->GetActorDead())
	{
		bIsCorrectTarget = false;
	}

	if(iTargetType & ESTARGET_COUPLE)
	{
		bIsCorrectTarget = false;

		PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);

		if(pkPilot == pkTargetPilot) //나도 커플
		{
			if(iTargetType & ESTARGET_SELF) // ME 옵션이 없으면 타겟에서는 제외
			{
				if(BM::GUID::IsNotNull(pkPlayer->CoupleGuid())) //커플 아이디가 있으면
				{
					bIsCorrectTarget = true;
				}
			}
		}
		else
		{
			if(pkPlayer)
			{			
				if(pkPlayer->CoupleGuid() == pkTargetActor->GetGuid())
				{
					bIsCorrectTarget = true;
				}
			}
		}
	}

	//길드원한테 사용 하는 타입
	if(iTargetType & ESTARGET_GUILD)
	{
		bIsCorrectTarget = false;

		PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		PgPlayer *pkTargetPlayer = dynamic_cast<PgPlayer*>(pkTargetUnit);

		if(pkPilot == pkTargetPilot) //나도 길드원
		{	
			if(iTargetType & ESTARGET_SELF) //ESTARGET_SELF 가 없으면 나는 길드원에서 제외
			{
				//Guild Guid가 있으면 길드가 있음
				if(BM::GUID::IsNotNull(pkPlayer->GuildGuid()))
				{
					bIsCorrectTarget = true;
				}
			}
		}
		else
		{
			if(pkPlayer && pkTargetPlayer)
			{
				if(pkPlayer->GuildGuid() == pkTargetPlayer->GuildGuid())
				{
					bIsCorrectTarget = true;
				}
			}
		}		
	}

	return bIsCorrectTarget;
}

int	PgAction::FindTargets(int iActionNo, TargetAcquireType kTargetType, NiPoint3 kStart, NiPoint3 kDir, float fRange, float fWideAngle, float fEffectRadius, PgActionTargetList *pkIgnoreTargets,int iMaxTargets,bool bNoDuplication, PgActionTargetList &kout_FoundTargets, PgPilot *pkCasterPilot,  bool const bAddDisplayInfo)
{
	PgPilot* pkPilot = NULL;

	if(pkCasterPilot)
	{
		pkPilot = pkCasterPilot;
	}
	else
	{
		pkPilot = g_kPilotMan.GetPlayerPilot();
	}

	if(!pkPilot) {return 0;}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(!pkActor) {return 0;}

	PgWorld	*pkWorld = pkActor->GetWorld();
	if(!pkWorld) {return 0;}

	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)	{return 0;}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iActionNo);
	if(!pkSkillDef)
	{
		NILOG(PGLOG_ERROR, "[PgAction] %d action no, can't find SkillDef\n", iActionNo);
		return 0;
	}

	bool bIsSingleMode = 
#ifndef EXTERNAL_RELEASE
		g_pkApp->IsSingleMode();
#else
		false;
#endif

	if(0 == iMaxTargets)
	{
		iMaxTargets = pkSkillDef->GetAbil(AT_MAX_TARGETNUM);
	}

	if(0 == iMaxTargets)
	{
		_PgOutputDebugString("[Warnning] [PgAction::FindTargets] iActionNo:%d MaxTarget is 0\n",iActionNo);
		return 0;
	}

	bool bCheckTargetWithoutEffect = pkSkillDef->GetAbil(AT_SKILL_CHECK_NONE_EFFECT) != 0;
	int iSkillEffectNo = pkSkillDef->GetEffectNo();
	
	// 스킬 제한 이펙트가 걸려있는지 확인해 범위를 제어한다
	//TAT_RAY_DIR:	iAttackRange, 30, 0				(
	//TAT_SPHERE:	0			, 30, iAttackRange
	//TAT_BAR:		iAttackRange, 00, 0

	fWideAngle = PgAdjustSkillFunc::CalcAdjustSkillValue(EASCT_RANGE2,iActionNo, pkUnit, fWideAngle);
	fRange = PgAdjustSkillFunc::CalcAdjustSkillValue(EASCT_RANGE, iActionNo, pkUnit, fRange);
	fEffectRadius= PgAdjustSkillFunc::CalcAdjustSkillValue(EASCT_RANGE, iActionNo, pkUnit, fEffectRadius);

	PgWOPriorityQueue kIgnoreFound;

	int iIgnoreTargetSize = 0;

	if(pkIgnoreTargets)
	{
		iIgnoreTargetSize = pkIgnoreTargets->size();
		for(ActionTargetList::iterator itor = pkIgnoreTargets->begin(); itor != pkIgnoreTargets->end(); ++itor)
		{//Ignore를 돌면서. 
			PgPilot* pkTargetPilot = g_kPilotMan.FindPilot((*itor).GetTargetPilotGUID());
			if(pkTargetPilot)
			{//타겟을 찾아서.
				PgActor* pkTargetActor = dynamic_cast<PgActor *>(pkTargetPilot->GetWorldObject());
				if(pkTargetActor)
				{//엑터를 받아와서. -> 찾은놈이라고 해서 넣기.
					kIgnoreFound.AddObj(pkTargetActor,(pkTargetActor->GetWorldTranslate()-pkActor->GetWorldTranslate()).Length());
				}
			}
		}
	}

	int const iTargetType = pkSkillDef->GetTargetType();
	if(ESTARGET_NONE == iTargetType)
	{
		return 0;
	}

	if(ESTARGET_SELF == iTargetType)
	{
		if(false == kIgnoreFound.CheckExist(pkActor))
		{
			kout_FoundTargets.GetList().push_back(PgActionTargetInfo(pkPilot->GetGuid(),pkActor->GetHitObjectABVIndex()));
		}
		return kout_FoundTargets.size();
	}

	if(ESTARGET_CASTER == iTargetType)
	{
		PgActor* pkCasterActor = g_kPilotMan.GetPlayerActor();
		PgPilot* pkCasterPilot = g_kPilotMan.GetPlayerPilot();
		CUnit* pkCasterUnit = g_kPilotMan.GetPlayerUnit();

		if(pkCasterUnit && pkCasterPilot && pkCasterUnit)
		{
			if(pkUnit->Caller() == pkCasterUnit->GetID())
			{
				if(false == kIgnoreFound.CheckExist(pkCasterActor))
				{
					if(bCheckTargetWithoutEffect == false || pkCasterUnit->GetEffect(iSkillEffectNo, true) == NULL)
					{ //대상에게 이펙트 유무 여부를 체크
						kout_FoundTargets.GetList().push_back(PgActionTargetInfo(pkCasterPilot->GetGuid(),pkCasterActor->GetHitObjectABVIndex()));					
					}
				}

				return kout_FoundTargets.size();
			}
		}
	}

	unsigned int uiGroup = 0;
	if(bIsSingleMode)
	{
		uiGroup |= 1<<(OGT_PLAYER+1);
		uiGroup |= 1<<(OGT_MONSTER+1);
		uiGroup |= 1<<(OGT_OBJECT+1);
	}

	if(iTargetType & ESTARGET_ENEMY)
	{
		if(g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP_ABLE) || 0 != g_pkWorld->DynamicGndAttr() & DGATTR_FLAG_FREEPVP) )
		{
			uiGroup |= 1<<(OGT_PLAYER+1);
			uiGroup |= 1<<(OGT_MONSTER+1);
			uiGroup |= 1<<(OGT_OBJECT+1);
			uiGroup |= 1<<(OGT_SIMILAR_PLAYER+1);
		}
		else if( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION))
			&& ( pkUnit->IsUnitType(UT_PLAYER) || pkUnit->IsUnitType(UT_SUB_PLAYER) )
			&& (0 < pkUnit->GetAbil(AT_TEAM)) 
			)
		{
			uiGroup |= 1<<(OGT_MONSTER+1);
		}
		else
		{
			if(pkUnit->IsUnitType(UT_PLAYER) ||
				pkUnit->IsUnitType(UT_PET) ||
				pkUnit->IsUnitType(UT_SUB_PLAYER)
				)	//	플레이어/소환물/펫 이면 몬스터가 적
			{
				uiGroup |= 1<<(OGT_MONSTER+1);
				uiGroup |= 1<<(OGT_OBJECT+1);
				if(pkUnit->GetAbil(AT_DUEL) > 0 || pkUnit->GetAbil(AT_TEAM) > 0)
				{
					uiGroup |= 1<<(OGT_PLAYER+1);
					uiGroup |= 1<<(OGT_SIMILAR_PLAYER+1);
				}
			}
			else if(pkUnit->IsUnitType(UT_MONSTER))	//	몬스터라면 플레이어가 적
			{
				uiGroup |= 1<<(OGT_PLAYER+1);
				uiGroup |= 1<<(OGT_SIMILAR_PLAYER+1);
			}
			else if(pkUnit->IsUnitType(UT_ENTITY) ||
				pkUnit->IsUnitType(UT_SUMMONED)) //	소환체라면 몬스터/플레이어가 적이 되지 않을까?
			{
				uiGroup |= 1<<(OGT_PLAYER+1);
				uiGroup |= 1<<(OGT_MONSTER+1);
				uiGroup |= 1<<(OGT_OBJECT+1);
			}
		}
	}

	if(iTargetType & ESTARGET_PARTY || iTargetType & ESTARGET_SELF || iTargetType & ESTARGET_ALLIES)
	{
		if(pkUnit->IsUnitType(UT_PLAYER) || 
			pkUnit->IsUnitType(UT_ENTITY) || 
			pkUnit->IsUnitType(UT_PET) ||
			pkUnit->IsUnitType(UT_SUB_PLAYER)
			)	//	플레이어라면 
		{
			uiGroup |= 1<<(OGT_PLAYER+1);
		}
		else if(pkUnit->IsUnitType(UT_MONSTER))	//	몬스터라면 
		{
			uiGroup |= 1<<(OGT_MONSTER+1);
		}
	}

	if((iTargetType & ESTARGET_COUPLE) || (iTargetType & ESTARGET_GUILD))
	{
		uiGroup |= 1<<(OGT_PLAYER+1);
	}

	if(ESTARGET_SUMMONED & iTargetType)
	{
		uiGroup |= 1<<(OGT_SIMILAR_PLAYER+1);
	}


	if(uiGroup == 0)
	{
		_PgOutputDebugString("[Warnning] [PgAction::FindTargets] iActionNo:%d uiGroup is 0\n",iActionNo);
		return 0;
	}

	EAttackedUnitPos kTargetUnitPosType = static_cast<EAttackedUnitPos>(pkSkillDef->GetAbil(AT_ATTACK_UNIT_POS));
	if(kTargetUnitPosType == EAttacked_Pos_None)
	{
		kTargetUnitPosType = EAttacked_Pos_Normal;
	}

	//if(kOption&FTO_BLOWUP) kTargetUnitPosType= static_cast<EAttackedUnitPos>((((int)kTargetUnitPosType)|EAttacked_Pos_Blowup));


	switch(kTargetType)//찾기 타입이.
	{
	case PgAction::TAT_RAY_DIR:
		{
			pkWorld->RayCheckObjectWidely(kIgnoreFound,iActionNo,kStart,kDir,uiGroup,fRange,fWideAngle,kTargetUnitPosType,bNoDuplication,pkActor);
		}break;
	case PgAction::TAT_RAY_DIR_COLL_PATH:
		{
			pkWorld->RayCheckObjectFloorSphere(kIgnoreFound,iActionNo,kStart,kDir,uiGroup,fEffectRadius,kTargetUnitPosType,bNoDuplication,pkActor);
		}break;
	case PgAction::TAT_SPHERE:
		{
			pkWorld->RayCheckObjectRange(kIgnoreFound,iActionNo,kStart,uiGroup,fEffectRadius,kTargetUnitPosType,bNoDuplication,pkActor);
		}break;
	case PgAction::TAT_BAR:
		{
			pkWorld->RayCheckObjectBar(kIgnoreFound,iActionNo,kStart,kDir,uiGroup,fWideAngle,fRange,kTargetUnitPosType,bNoDuplication,pkActor);
		}
		break;
	}
	if(iTargetType & ESTARGET_SELF)
	{
		if(false == kIgnoreFound.CheckExist(pkActor))
		{
			kIgnoreFound.AddObj(pkActor,0);
		}
	}
	
	int iSkip = 0;
	PgWOPriorityQueue::ObjList *pkIFList = kIgnoreFound.GetList();
	PgWOPriorityQueue::ObjList::iterator itor = pkIFList->begin();

	PgPilot* pkTargetPilot = NULL;

	if(bNoDuplication)
	{
		int	iAdded = 0;
		for(;itor != pkIFList->end(); ++itor)
		{
			pkTargetPilot = (*itor).m_pkObject->GetPilot();
			if(!pkTargetPilot)
			{
				continue;
			}

			if(0 < pkTargetPilot->GetAbil(AT_CANNOT_DAMAGE)							// 데미지를 받을수 없는 대상은 제외하고
				|| kout_FoundTargets.GetTargetByGUID(pkTargetPilot->GetGuid())
				)
			{
				continue;
			}

			CUnit* pkUnit = pkTargetPilot->GetUnit();
			if(pkUnit && bCheckTargetWithoutEffect == true && pkUnit->GetEffect(iSkillEffectNo, true) != NULL )
			{
				continue;
			}

			kout_FoundTargets.GetList().push_back(PgActionTargetInfo(pkTargetPilot->GetGuid(),(*itor).m_pkObject->GetHitObjectABVIndex()));

			++iAdded;
			if(iAdded == iMaxTargets)
			{
				break;
			}
		}
	}
	else
	{
		for(int i=0; i<iMaxTargets; ++i)
		{
			if(itor == pkIFList->end())
			{
				break;
			}
			
			pkTargetPilot = (*itor).m_pkObject->GetPilot();

			if( (pkTargetPilot && 0 < pkTargetPilot->GetAbil(AT_CANNOT_DAMAGE))
				|| !pkTargetPilot
				)
			{// 데미지를 받을수 없는 대상은 제외하고
				++itor;
				continue;
			}
			CUnit* pkUnit = pkTargetPilot->GetUnit();
			if(pkUnit && bCheckTargetWithoutEffect == true && pkUnit->GetEffect(iSkillEffectNo, true) != NULL )
			{
				continue;
			}

			kout_FoundTargets.GetList().push_back(PgActionTargetInfo(pkTargetPilot->GetGuid(),(*itor).m_pkObject->GetHitObjectABVIndex()));

			++itor;
		}
	}
	
#ifndef EXTERNAL_RELEASE
		if(bAddDisplayInfo)
		{
			switch(pkUnit->UnitType())
			{
			case UT_PLAYER:
			case UT_SUB_PLAYER:
			case UT_ENTITY:
			case UT_SUMMONED:
			case UT_PET:
				{
					g_kSkillTargetMan.EditSkillRangeParam(pkUnit->GetID(), iActionNo, kTargetType, kDir, kStart, fRange, fWideAngle, fEffectRadius);
				}break;
			}
		}
#endif

	return kout_FoundTargets.GetList().size();
}

int PgAction::OnFindTarget(PgActor* pkActor, PgActionTargetList* pkTargetList)
{
	NiPoint3 pkCasterPos = pkActor->GetPos();
	NiPoint3 pkCasterDir = pkActor->GetLookingDir();
	float const fBackDistance = 10.0f;

	pkCasterDir *= fBackDistance;
	pkCasterPos += pkCasterDir;


	int const iAttackRange = lwAction(this).GetSkillRange(GetActionNo(), lwActor(pkActor));

	TargetAcquireType const kTargetType = TAT_RAY_DIR;

	int iTargets = 0;

	PgPilot* pkPilot = NULL;
	if(pkActor)
	{
		pkPilot = pkActor->GetPilot();
	}

	switch(kTargetType)//찾기 타입이.
	{
	case TAT_RAY_DIR:
		{
			iTargets = FindTargets(GetActionNo(), kTargetType, pkCasterPos, pkCasterDir, iAttackRange, 30, 0, pkTargetList, 0, true, *pkTargetList, pkPilot);
		}break;
	case TAT_RAY_DIR_COLL_PATH:
		{

		}break;
	case TAT_SPHERE:
		{
			iTargets = FindTargets(GetActionNo(), kTargetType, pkCasterPos, pkCasterDir, 0, 30, iAttackRange, pkTargetList, 0, true, *pkTargetList, pkPilot);
		}break;
	case TAT_BAR:
		{
			iTargets = FindTargets(GetActionNo(), kTargetType, pkCasterPos, pkCasterDir, iAttackRange, 00, 0, pkTargetList, 0, true, *pkTargetList, pkPilot);
		}break;
	default :
		{
		}break;
	}
	
	return iTargets;
		
}

void PgAction::AddTarget(BM::GUID const &kGUID,int iABVIndex,PgActionResult *pkActionResult)
{
	PgActionTargetInfo	kActionInfo(kGUID,iABVIndex);
	if(pkActionResult)
	{
		kActionInfo.SetActionResult(*pkActionResult);
		if(pkActionResult->IsValidHP())
		{
			kActionInfo.SetRemainHP(pkActionResult->GetRemainHP());
		}
	}
	m_ActionTargetList.GetList().push_back(kActionInfo);
}

void PgAction::SetTargetActionResult(PgActionResultVector *pkActionResult,bool bDoNotSetEffect)
{
	int iCount = 0;
	for(ActionTargetList::iterator itor = m_ActionTargetList.begin(); itor != m_ActionTargetList.end(); ++itor)
	{
		PgActionResult* pkResult = pkActionResult->GetResult(itor->GetTargetPilotGUID(), false);
		if (pkResult != NULL && pkResult->GetInvalid() == false)
		{
			if(bDoNotSetEffect)
			{
				(*itor).GetActionResult().SetBlocked(pkResult->GetBlocked());
				(*itor).GetActionResult().SetMissed(pkResult->GetMissed());
				(*itor).GetActionResult().SetValue(pkResult->GetValue());
			}
			else
			{
				(*itor).SetActionResult(*pkResult);
			}
		}
	}
}

void PgAction::SetTargetList(PgActionTargetList &kTargetList)
{
	ClearTargetList();
	for(ActionTargetList::iterator itor = kTargetList.begin(); itor != kTargetList.end(); ++itor)
	{
		AddTarget((*itor).GetTargetPilotGUID(),(*itor).GetSphereIndex(),&(*itor).GetActionResult());
	}
	m_ActionTargetList.SetActionInfo(kTargetList.m_kCasterGUID,kTargetList.m_iActionInstanceID,kTargetList.GetActionNo(),kTargetList.m_dwTimeStamp);
}

void PgAction::SetActionParam(BYTE byParam)
{
	m_byActionParam = byParam;
}

void PgAction::BroadCastTargetListModify(PgPilot *pkPilot, bool bisTargetEmptyToBroadCast)	//	타겟 리스트가 바뀌었음을 서버에 알린다.
{
	// bisTargetEmptyToBroadCast == false 일 때 타겟리스트가 비어있으면 보내지 않는다
	if(false == bisTargetEmptyToBroadCast)
	{
		if(0 >= GetTargetList()->size())
		{
			return ;
		}
	}

	int	iAPBackup = GetActionParam();	
	SetActionParam(ESS_TARGETLISTMODIFY);	//	잠시 바꾼다.
	g_kPilotMan.Broadcast(pkPilot, this, true);
	SetActionParam(iAPBackup);	//	원상 복구!
}

void PgAction::BroadCast(PgPilot *pkPilot)	//	서버로 전송하여 다른 유저에게 브로드캐스트 하도록 한다.
{
	PG_ASSERT_LOG(pkPilot);
	if (pkPilot == NULL)
	{
		return;
	}

	NILOG(PGLOG_NETWORK, "[PgAction] BroadCast Action(%d,%d,%d,%d)(%d)\n", GetActionNo(), GetActionInstanceID(), GetActionParam(), GetTargetList()->size(), g_pkApp->GetFrameCount());
//	g_kPilotMan.Broadcast(pkPilot,GetActionNo(),GetActionInstanceID(),GetActionParam(),GetTargetList(),true,true);
	SetEnable(true);
	g_kPilotMan.Broadcast(pkPilot, this, true);
}
int	PgAction::GetAbil(int const iAbilType) const
{
	if(0 == m_pkSkillDef)
	{
		return 0;
	}

	switch(iAbilType)
	{
	case ATS_COOL_TIME:
		{
			int iCoolTime = m_pkSkillDef->GetAbil(ATS_COOL_TIME);
			iCoolTime += m_iSkillCoolTime;
			iCoolTime += std::max<int>(iCoolTime*m_fSkillCoolTimeRate,0);
			return __max(0, iCoolTime);			
		}break;
	case AT_CAST_TIME:
		{
			int iCastTime = m_pkSkillDef->GetAbil(AT_CAST_TIME);
			iCastTime += m_iSkillCastingTime;
			return __max(0, iCastTime);			
		}break;
	default:
		{
			return m_pkSkillDef->GetAbil(iAbilType);
		}break;	
	}
	
	return 0;
}

//! GUID 컨테이너를 클리어한다.
void PgAction::ClearGUIDContainer()
{
	m_GuidContainer.clear();
}
//! GUID 컨테이너에 GUID 를 추가한다.
void PgAction::AddNewGUID(BM::GUID const &newGUID)
{
	if(CheckGUIDExist(newGUID)) return;

	m_GuidContainer.push_back(newGUID);
}

//! GUID 컨테이너에서 하나를 삭제한다.
void PgAction::RemoveGUID(BM::GUID const &newGUID)
{
	m_GuidContainer.remove(newGUID);
}
//! GUID 컨테이너에 GUID 가 존재하는지 체크한다.
bool PgAction::CheckGUIDExist(BM::GUID const &GUID)
{
	for(GUIDContainer::iterator itor = m_GuidContainer.begin(); itor != m_GuidContainer.end(); ++itor)
	{
		if( (*itor) == GUID) return true;
	}
	return false;
}
int	PgAction::GetTotalGUIDInContainer() const
{
	return m_GuidContainer.size();
}
BM::GUID const	*PgAction::GetGUIDInContainer(int iIndex)
{
	int i=0;
	for(GUIDContainer::iterator itor = m_GuidContainer.begin(); itor != m_GuidContainer.end(); ++itor)
	{
		if(i == iIndex)
		{
			return &(*itor);
		}
		++i;
	}
	return NULL;
}

bool PgAction::SetParam(unsigned int uiIndex, char const *pcParam)
{
	if(!pcParam)
	{
		PG_ASSERT_LOG(!"SetParam : Parameter is null!");
		return false;
	}

	m_kParamContainer[uiIndex] = std::string(pcParam);
	
	return true;
}

char const* PgAction::GetParam(unsigned int const uiIndex) const
{ 
	ParamContainer::const_iterator itr = m_kParamContainer.find(uiIndex);
	if(itr == m_kParamContainer.end())
	{
		//! 예전에는 0번에 "null" string을 넣었음.
		//return m_kParamContainer.find(0)->second.c_str();
		return "null";
	}

	return itr->second.c_str();
}

void PgAction::OverridePacket(PgActor* pkActor, BM::Stream *pkPacket)
{
	if(GetFSMFunc())
	{
		GetFSMFunc()->OnOverridePacket(lwActor(pkActor), lwAction(this), lwPacket(pkPacket));
	}
	else
	{
		lua_tinker::call<void, lwActor, lwAction, lwPacket>(std::string(GetFSMScript() + "_OnOverridePacket").c_str(), lwActor(pkActor), lwAction(this), lwPacket(pkPacket));
	}
}
void PgAction::SetNextActionName(char const *pcActionName)
{
	if( !pcActionName )
	{
		_PgOutputDebugString("PgAction::SetNextActionName curActionName : %s, pcActionName is NULL\n",GetID().c_str());
		m_kNextActionName = std::string(ACTIONNAME_IDLE);
		return;
	}
	_PgOutputDebugString("PgAction::SetNextActionName curActionName : %s, pcActionName:%s\n",GetID().c_str(), pcActionName);

	m_kNextActionName = std::string(pcActionName);
}
void PgAction::SetNextActionInputSlotInfo(PgInputSlotInfo *pkInputSklotInfo)
{
	m_pkNextActionInputSlotInfo = pkInputSklotInfo;
}
PgInputSlotInfo const* PgAction::GetNextActionInputSlotInfo() const
{
	return m_pkNextActionInputSlotInfo;
}

bool PgAction::IsRecord()
{
	return m_bRecord;
}

void PgAction::SetRecord(bool bRecord)
{
	m_bRecord = bRecord;
}

BYTE PgAction::GetDirection() const
{
	return m_byMovingDirection;
}

void PgAction::SetDirection(BYTE byDir)
{
	m_byMovingDirection = byDir;
}

void PgAction::SetParamAsPacket(BM::Stream *pkPacket, bool const bPrevPacketDelete)
{
	if(bPrevPacketDelete)
	{
		DeleteParamPacket();
	}

	m_pkParamPacket = pkPacket;
}

BM::Stream* PgAction::GetParamAsPacket() const
{
	return m_pkParamPacket;
}

void PgAction::DeleteParamPacket()
{
	SAFE_DELETE(m_pkParamPacket);
}

int PgAction::GetCurrentSlot() const
{
	return m_uiCurrentSlot;
}

int PgAction::GetSlotCount() const
{
	return m_kSlotContainer.size();
}

bool PgAction::SetSlot(unsigned int iSlot)
{
	if(m_kSlotContainer.size() <= iSlot)
	{
		return false;
	}
	m_uiCurrentSlot = iSlot;
	return true;
}

bool PgAction::NextSlot()
{
	return SetSlot(m_uiCurrentSlot+1);
}

bool PgAction::PrevSlot()
{
	return SetSlot(m_uiCurrentSlot-1);
}

bool PgAction::GetActionName(std::string& rkActionName_out,int iSlotNum)
{
	PG_ASSERT_LOG(iSlotNum >= -1);
	if(iSlotNum == -1)
	{
		iSlotNum = m_uiCurrentSlot;
	}

	if((int)m_kSlotContainer.size() <= iSlotNum)
	{
		return false;
	} 
	rkActionName_out = m_kSlotContainer.at(iSlotNum).m_kSlotName;
	return true;
}

bool PgAction::IsSlotLoop()const
{
	return m_kSlotContainer.at(m_uiCurrentSlot).m_bLoop;
}
bool PgAction::IsSlotEventScript()const
{
	return m_kSlotContainer.at(m_uiCurrentSlot).m_bEventScript;
}
bool PgAction::GetAnimationInfo(int const iSlotNum, std::string const& rkInfoName, std::string & rkOut)const
{
	if(m_kSlotContainer.size() <= iSlotNum)
	{
		return false;
	}

	PgSlot::CONT_ANIMATIONINFO const& kContAniInfo = m_kSlotContainer.at(m_uiCurrentSlot).m_kContAnimationInfo;
	PgSlot::CONT_ANIMATIONINFO::const_iterator itor = kContAniInfo.find(rkInfoName);
	if(itor != kContAniInfo.end())
	{
		rkOut = (*itor).second;
		return true;
	}
	return false;
}
char const*	PgAction::GetScriptParam(std::string const& kParamName) const
{
	ScriptParamMap::const_iterator itor = m_ScriptParamMap.find(kParamName);
	if(itor != m_ScriptParamMap.end())
	{
		return itor->second.c_str();
	}
	return "";
}

std::string &PgAction::GetNextActionName()
{
	return m_kNextActionName;
}

std::string &PgAction::GetActionType()
{
	return m_kActionType;
}
int PgAction::GetCount()
{
	return m_kSlotContainer.size();
}

std::string &PgAction::GetFSMScript()
{
	return m_kFSMScript;
}

bool PgAction::GetEnable()
{
	return m_bEnable;
}

void PgAction::SetEnable(bool bEnable)
{
	m_bEnable = bEnable;
}
void PgAction::SetActionNo(int iActionNo)	
{	
	if(m_iActionNo == iActionNo) return;

	m_iActionNo = iActionNo;	

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iActionNo);
	if (pkSkillDef == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgAction] %d action no, can't find SkillDef\n", iActionNo);
		return;
	}
	m_pkSkillDef = pkSkillDef;
	m_SkillType = static_cast<ESkillType>(m_pkSkillDef->GetType());
	m_iEffectNo = m_pkSkillDef->GetEffectNo();	// 스킬과 연결되어있는 이펙트 번호
}

#ifdef PG_SYNC_ENTIRE_TIME
void PgAction::SetBirthTime(DWORD dwBirthTime)
{
	m_dwBirthTime = dwBirthTime;
}

DWORD PgAction::GetBirthTime()
{
	return m_dwBirthTime;
}
#endif

void PgAction::CopyParamTo(PgAction *pkTarget)
{
	pkTarget->m_kParamContainer = m_kParamContainer;
	pkTarget->m_kPointContainer = m_kPointContainer;
	pkTarget->m_GuidContainer = m_GuidContainer;
}

DWORD PgAction::GetElapsedTime() const
{
	return (DWORD)(BM::GetTime32() - m_fActionEnterTime * 1000.0f);
}

int PgAction::GetCurrentStage() const
{
	return m_iStage;
}

void PgAction::NextStage()
{
	++m_iStage;
}

//!	캐스팅이 끝났을 경우 호출된다.
void PgAction::OnCastingCompleted(PgActor* pkActor,PgAction* pkAction)
{
	if (GetID().empty() || m_kFSMScript.empty() || NULL == pkActor|| NULL == pkAction)
	{
		return;
	}

	if(pkAction->GetSkillDef()->GetType() == EST_TOGGLE)
	{
		pkAction->SetActionParam(ESS_TOGGLE_ON);
	}
	else
	{
		pkAction->SetActionParam(ESS_FIRE);
	}

	if(GetFSMFunc())
	{
		GetFSMFunc()->OnCastingCompleted(lwActor(pkActor), lwAction(pkAction));
	}
	else
	{
		lua_tinker::call<void, lwActor, lwAction>((m_kFSMScript + "_OnCastingCompleted").c_str(), lwActor(pkActor), lwAction(pkAction));
	}

	if(pkActor->IsUnderMyControl())
	{
		bool bToggle = false;
		CSkillDef const* pkSkillDef = GetSkillDef();
		if(pkSkillDef)
		{
			if(EST_TOGGLE == pkSkillDef->GetType())
			{
				bToggle = true;
			}
		}

		// 토클의 쿨타임은 스킬을 해제한 후 부터 시작한다.
		if(bToggle)
		{
			pkActor->StartSkillToggle(pkAction->GetActionNo());
		}
		else
		{
			pkActor->StartSkillCoolTime(pkAction->GetActionNo());	//	쿨타임 시작
		}
		
		pkAction->SetActionInstanceID();
		pkAction->GetTargetList()->m_iActionInstanceID = pkAction->GetActionInstanceID();
		if(GetActionOptionEnable(AO_AUTO_BROADCAST_WHEN_CASTING_COMPLETED))
		{
			pkAction->BroadCast(pkActor->GetPilot());
		}
	}
}

//!	이 액션으로 진입할 수 있는지 체크한다.
bool PgAction::CheckCanEnter(PgActor* pkActor, PgAction* pkAction)
{
	if (GetID().empty() || m_kFSMScript.empty() || pkActor == NULL || pkAction == NULL)
	{
		return false;
	}

	PgAction* pkCurAction = pkActor->GetAction();
	PgPilot* pkPilot = pkActor->GetPilot();
	CUnit* pkUnit = NULL;
	if(pkPilot)
	{
		pkUnit = pkPilot->GetUnit();
	}
	if(pkCurAction && !pkCurAction->GetActionFinished())
	{
		if(pkUnit && pkUnit->IsUnitType(UT_MONSTER))	//플레이어가 아니고
		{
			if(pkAction->GetActionType()=="IDLE" && pkCurAction->GetActionType() == "EFFECT")
			{
				return false;	//띄워 올렸을 때 몬스터가 튕기는 현상 방지
			}
		}
	}

	if(!g_pkWorld)
	{
		return false;
	}
	if(!GetActionOptionEnable(AO_CALL_CHECK_CAN_ENTER))
	{
		return true;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	//NILOG(PGLOG_LOG, "EnterFMS(%s)\n", m_kFSMScript.c_str());
	SetActionFinished(false);
	m_fActionEnterTime = g_pkWorld->GetAccumTime();

	bool bResult = false;
	if(GetFSMFunc())
	{
		bResult = GetFSMFunc()->OnCheckCanEnter(lwActor(pkActor), lwAction(pkAction));
	}
	else
	{
		bResult = lua_tinker::call<bool, lwActor, lwAction>((m_kFSMScript + "_OnCheckCanEnter").c_str(), lwActor(pkActor), lwAction(pkAction));
	}
	
	NIMETRICS_ENDTIMER(a);
	if(!pkActor->IsUnderMyControl() && m_kFSMScript!="Act_Die") 
	{
		return true;
	}
	return bResult;
}

bool PgAction::EnterFSM(PgActor* pkActor, PgAction* pkAction)
{
	if (GetID().empty() || m_kFSMScript.empty() || pkActor == NULL || pkAction == NULL)
	{
		return false;
	}

	if(!g_pkWorld)
	{
		return false;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	//NILOG(PGLOG_LOG, "EnterFMS(%s)\n", m_kFSMScript.c_str());

	m_fActionEnterTime = g_pkWorld->GetAccumTime();

	bool bResult = false;
	if(GetFSMFunc())
	{
		bResult = GetFSMFunc()->OnEnter(lwActor(pkActor), lwAction(pkAction));
	}
	else
	{
		if (m_kFSMScript.empty() == false)
		{
			bResult = lua_tinker::call<bool, lwActor, lwAction>((m_kFSMScript + "_OnEnter").c_str(), lwActor(pkActor), lwAction(pkAction));
		}
	}
	NIMETRICS_ENDTIMER(a);
	if(!pkActor->IsUnderMyControl())// && !pkActor->IsMyPet()) 
	{
		bResult = true;
	}

	if(bResult)
	{
		SetActionFinished(false);
	}
	
	if(0 < GetAbil(AT_POT_PARTICLE_ACTION))
	{
		PgPOTParticleUtile::AttachParticle(pkActor->GetPOTParticleInfo(), PgPOTParticle::E_APOT_MELEE_ATK, *pkActor);
	}

	return bResult;
}

bool PgAction::LeaveFSM(PgActor* pkActor, PgAction* pkAction, bool bCancel)
{
	if (GetID().empty() || m_kFSMScript.empty() || pkActor == NULL || pkAction == NULL)
	{
		return true;
	}

	PgAction	*pkCurAction = pkActor->GetAction();
	if(pkCurAction && pkCurAction->IsChangeToNextActionOnNextUpdate())
	{
		return true;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	//NILOG(PGLOG_LOG, "LeaveFMS(%s)\n", m_kFSMScript.c_str());

	bool bResult = false;
	if(GetFSMFunc())
	{
		bResult = GetFSMFunc()->OnLeave(lwActor(pkActor), lwAction(pkAction),bCancel);
	}
	else
	{
		bResult = lua_tinker::call<bool, lwActor, lwAction,bool>((m_kFSMScript + "_OnLeave").c_str(), lwActor(pkActor), lwAction(pkAction),bCancel);
	}

	NIMETRICS_ENDTIMER(a);
	if(!pkActor->IsUnderMyControl())// && !pkActor->IsMyPet())) 
	{
		return true;
	}
	return bResult;
}

void PgAction::ModifyTargetList(PgActionTargetList& kTargetList)
{
	_PgOutputDebugString("[PgAction::ModifyTargetList] ActionInstanceID : %d Action Result Applied : %d\n", GetActionInstanceID(),m_ActionTargetList.IsActionEffectApplied());

	PgActionTargetInfo	*pkTargetInfo=NULL,*pkTargetInfo2=NULL;
	for(ActionTargetList::iterator itor = kTargetList.begin();itor != kTargetList.end(); )
	{

		pkTargetInfo = &(*itor);
		pkTargetInfo2 = m_ActionTargetList.GetTargetByGUID(pkTargetInfo->GetTargetPilotGUID());

		if(!pkTargetInfo2)
		{
			++itor;
			continue;
		}

		pkTargetInfo2->CopyFrom(*pkTargetInfo);

		if(false == m_ActionTargetList.IsActionEffectApplied())
		{
			itor = kTargetList.GetList().erase(itor);
		}
		else
		{
			++itor;
		}
	}

	m_ActionTargetList.SetTimeStamp(kTargetList.GetTimeStamp());
	m_ActionTargetList.SetEffectReal(true);
	SetTimeStamp(kTargetList.GetTimeStamp());
	
}

bool PgAction::ModifyActionResult(PgActor* pkActor, PgActionTargetList& kTargetList,int iTimeStamp)
{
	if(GetID().empty())
	{
		return false;
	}

	_PgOutputDebugString("PgAction::ModifyActionResult pkActor : %s %s ActionInstanceID : %d m_ActionTargetList Size : %d TargetList Size : %d\n",MB(pkActor->GetPilot()->GetGuid().str()),MB(pkActor->GetPilot()->GetName()),kTargetList.m_iActionInstanceID,m_ActionTargetList.size(),kTargetList.size());

	if(!kTargetList.CopyActionResultTo(m_ActionTargetList))
	{
		return true;
	}

	m_ActionTargetList.SetTimeStamp(iTimeStamp);
	kTargetList.SetTimeStamp(iTimeStamp);
	m_ActionTargetList.SetEffectReal(true);
	SetTimeStamp(iTimeStamp);

	if(false == m_ActionTargetList.IsActionEffectApplied())
	{
		return false;
	}

	return true;
}
void PgAction::CleanUpFSM(PgActor* pkActor, PgAction* pkAction)	//! 액션에 사용되었던 리소스들을 모두 해제시킨다
{
	if (GetID().empty() || m_kFSMScript.empty() || NULL == pkActor )
	{
		return;
	}

	PgPilot* pkPilot = pkActor->GetPilot();
	if(!pkPilot)
	{
		return;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	
	if(0 < GetAbil(AT_POT_PARTICLE_ACTION))
	{// 액션 종료 후 세트아이템 파티클 마무리
		PgPOTParticleUtile::FinishUpWhenActionEnd(pkActor->GetPOTParticleInfo(), *pkActor);
	}

	lwActor kActor(pkActor);
	lwAction kAction(pkAction);

	if(GetFSMFunc())
	{
		GetFSMFunc()->OnCleanUp(kActor, kAction);
	}
	else
	{
		lua_tinker::call<void, lwActor, lwAction>((m_kFSMScript + "_OnCleanUp").c_str(), kActor, kAction);
	}
/* - !!!!!!!!!!!!!!!!!!!!!!이하에 this의 멤버에 접근하는 코드를 작성하지 말것!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		OnCleanUp 스크립트에서 GetStatusEffectMan():RemoveStatusEffectFromActor가 호출되면 this가 제거되어버리고 다른 액션이 되고
		this가 제거되어버린 상황에서 이곳 함수로 돌아와 남은것을 실행할때 제거된 객체에 접근하기 때문에 크래시 나게됨
		(외부로 중요한 처리를 모두 빼다보니 생긴 문제)
*/

	if( 0 < pkPilot->GetAbil(AT_SKILL_CUSTOM_VALUE_STORAGE) )
	{// 스킬을 사용한후에 스킬 커스텀 값이 존재한다면
	 // 초기화 시켜준다(한스킬당 한번만 쓰는 값이고, 스킬마다 달라질수 있으므로)
		pkPilot->SetAbil(AT_SKILL_CUSTOM_VALUE_STORAGE, 0);	// 이값은 비동기 값이며, 서버에서 보내주고 클라에서 사용후 폐기한다.(서버는 보내는 동작만 해야함)
	}

	if (g_pkApp->IsSingleMode() == false )
	{
		int const iDieSkill = pkPilot->GetAbil(AT_SKILL_ON_DIE);
		if ( 0 < iDieSkill
			&& iDieSkill == GetActionNo()
			&& 1 >= pkPilot->GetAbil(AT_HP)
			)
		{
			BM::GUID const & rkGuid = pkActor->GetGuid();
			//pkActor->ClearActionQueue();
			pkActor->AddEffect(ACTIONEFFECT_DIE, 0, 0, rkGuid, 0, 0, true);
			if(pkActor->GetHPGaugeBar())
			{
				pkActor->GetHPGaugeBar()->SetBarValue(pkPilot->GetAbil(AT_C_MAX_HP),1,0);
			}
		}
	}

//	pkActor->CutSkillToggle(pkAction->GetActionNo());
	
	NIMETRICS_ENDTIMER(a);
}

void PgAction::FindTargetFSM(PgActor* pkActor, PgAction* pkAction,PgActionTargetList &kTargetList)	//! 타겟을 찾아서 kTargetList에 집어넣는 스크립트를 호출한다.
{
	kTargetList.clear();

	if (GetID().empty() || m_kFSMScript.empty() || pkActor == NULL)
	{
		return;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);

	if(GetFSMFunc())
	{
		GetFSMFunc()->OnFindTarget(lwActor(pkActor),lwAction(this),lwActionTargetList(&kTargetList));
	}
	else
	{
//#ifndef EXTERNAL_RELEASE
//		if( lua_tinker::call<bool>("GetUseScript"))
//#else
		if(!GetActionOptionEnable(AO_USE_CFUNC_ONTARGET))
//#endif
		{
			lua_tinker::call<void, lwActor, lwAction,lwActionTargetList>((m_kFSMScript + "_OnFindTarget").c_str(), lwActor(pkActor),lwAction(this),lwActionTargetList(&kTargetList));
		}
		else
		{
			OnFindTargetFunc(pkActor, pkAction, kTargetList);
		}
	}
	
	NIMETRICS_ENDTIMER(a);
}

void PgAction::ChangeToNextActionOnNextUpdate(bool bChange)	//	다음업데이트시에 자동으로 NextAction  으로 전이합니다.
{
	m_bChangeToNextActionOnNextUpdate = bChange;
}

bool PgAction::UpdateFSM(PgActor* pkActor, float fAccumTime, float fFrameTime)
{
	UpdateTimer(pkActor,fAccumTime);

	if(IsChangeToNextActionOnNextUpdate())
	{
		if(!m_kNextActionName.empty())
		{
			std::string	kNextActionName = m_kNextActionName;

			PgAction	*pkNextAction = pkActor->CreateActionForTransitAction(kNextActionName.c_str(),true,NULL,pkActor->GetDirection());
			if(!pkNextAction)
			{
				ChangeToNextActionOnNextUpdate(false);
			}
			else
			{
				pkNextAction->SetInputSlotInfo(const_cast<PgInputSlotInfo*>(GetNextActionInputSlotInfo()));
				if(!pkActor->ProcessAction(pkNextAction,pkActor->IsMyActor()))
				{
					if(pkActor->GetReservedTransitAction() == NULL && kNextActionName == m_kNextActionName)
					{
						ChangeToNextActionOnNextUpdate(false);
					}
				}
			}
		}
		return false;	
	}

	if (GetID().empty() || m_kFSMScript.empty() || pkActor == NULL)
	{
		return false;
	}

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);

	bool bReturn = false;
	if(GetFSMFunc())
	{
		bReturn = GetFSMFunc()->OnUpdate(lwActor(pkActor), lwAction(this),fAccumTime, fFrameTime);
	}
	else
	{
		bReturn = lua_tinker::call<bool, lwActor, float, float, lwAction>((m_kFSMScript + "_OnUpdate").c_str(), lwActor(pkActor), fAccumTime, fFrameTime,lwAction(this));
	}

	NIMETRICS_ENDTIMER(a);
	SetActionFinished(!bReturn);
	if(!bReturn)
	{
		if(pkActor->IsUnderMyControl() && pkActor->GetSkillCastingInfo()->m_ulSkillNo == GetActionNo())
		{
			pkActor->CutSkillCasting(GetActionNo());
		}

		if(pkActor->GetReservedTransitAction() == NULL)	//	스크립트에서 액터를 트랜짓 액션 하지 않았을 경우, NextAction 으로 전이한다.
		{
			if(!m_kNextActionName.empty())
			{
				if(m_kNextActionName == ACTIONNAME_IDLE || m_kNextActionName == ACTIONNAME_BIDLE)
				{
					PgPilot* pPilot = pkActor->GetPilot();
					if( pPilot )
					{
						CUnit* pUnit = pPilot->GetUnit();
						if( pUnit )
						{
							int const iNoIdle = pUnit->GetAbil(AT_SKILL_SPECIFIC_IDLE);
							switch( iNoIdle )
							{
							case ESIT_NONE:	{}break;
							case ESIT_BOSS_MONSTER_IDLE:
								{
									SetNextActionName("a_SpecificIdle");
								}break;
							default:
								{
									GET_DEF(CSkillDefMgr, kSkillDefMgr);
									std::wstring kActionID = kSkillDefMgr.GetActionName(iNoIdle);
									SetNextActionName(MB(kActionID));
								}break;
							}
						}
					}
				}

				PgAction	*pkNextAction = pkActor->CreateActionForTransitAction(m_kNextActionName.c_str(),true,NULL,pkActor->GetDirection());
				if(pkNextAction)
				{
					pkNextAction->SetInputSlotInfo(const_cast<PgInputSlotInfo*>(GetNextActionInputSlotInfo()));
					pkActor->ProcessAction(pkNextAction,pkActor->IsMyActor());
				}
			}
			return false;
		}
	}

	return true;
}


bool PgAction::EventFSM(PgActor* pkActor, char const *pcTextKey,NiActorManager::SequenceID seqID)
{
	if(GetActionOptionEnable(AO_USE_EVENT) && pkActor != NULL && pcTextKey != NULL)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		//NILOG(PGLOG_LOG, "EventFSM(%s) %s Event called\n", m_kFSMScript.c_str(), pcTextKey);

		bool bResult = false;
		if(GetFSMFunc())
		{
			bResult = GetFSMFunc()->OnEvent(lwActor(pkActor), pcTextKey,(int)seqID);
		}
		else
		{
			bResult = lua_tinker::call<bool, lwActor, char const *,int>((m_kFSMScript + "_OnEvent").c_str(), lwActor(pkActor), pcTextKey,(int)seqID);
		}
		NIMETRICS_ENDTIMER(a);

		if(0 < GetAbil(AT_POT_PARTICLE_ACTION))
		{
			PgPOTParticleUtile::AttachParticle(pkActor->GetPOTParticleInfo(), PgPOTParticle::E_APOT_EVENT_HITKEY, *pkActor, pcTextKey);
		}

		return bResult;
	}
	return false;
}
void PgAction::OnTargetListModified(PgActor* pkActor,bool bIsBefore)
{
	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	//NILOG(PGLOG_LOG, "EventFSM(%s) %s Event called\n", m_kFSMScript.c_str(), pcTextKey);

	if(GetFSMFunc())
	{
		GetFSMFunc()->OnTargetListModified(lwActor(pkActor), lwAction(this),bIsBefore);
	}
	else
	{
		lua_tinker::call<void, lwActor,lwAction,bool>((m_kFSMScript + "_OnTargetListModified").c_str(), lwActor(pkActor), lwAction(this),bIsBefore);
	}
	NIMETRICS_ENDTIMER(a);
}

NiPoint3 const& PgAction::GetParamAsPoint(int const iIndex) const
{
	PointContainer::const_iterator itr = m_kPointContainer.find(iIndex);
	if(itr == m_kPointContainer.end())
	{
		return NiPoint3::ZERO;
	}
	
	return itr->second;
}

void PgAction::SetParamAsPoint(int const iIndex, NiPoint3 &rkPoint)
{
	PointContainer::iterator itr = m_kPointContainer.find(iIndex);
	if(itr == m_kPointContainer.end())
	{
		m_kPointContainer.insert(std::make_pair(iIndex, rkPoint));	
		return;
	}
	itr->second = rkPoint;
}

void PgAction::SetSkillCoolTime(int const iAddCoolTime)
{
	m_iSkillCoolTime = iAddCoolTime;
}
void PgAction::SetSkillCoolTimeRate(float const fAddCoolTimeRate)
{
	m_fSkillCoolTimeRate = fAddCoolTimeRate;
}
void PgAction::SetSkillCastingTime(int const iAddCastingTime)
{
	m_iSkillCastingTime = iAddCastingTime;
}

bool PgAction::ParseXml(TiXmlNode const* pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement const* pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const* pcTagName = pkElement->Value();

			if(0 == strcmp(pcTagName, "ACTION"))
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == strcmp(pcName, "ID"))
					{
						SetID(pcValue);
						m_iActionNo = kSkillDefMgr.GetSkillNoFromActionName(std::wstring(UNI(pcValue)));
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iActionNo);
						if (pkSkillDef == NULL)
						{
							NILOG(PGLOG_ERROR, "[PgAction] %d action no, can't find SkillDef\n", m_iActionNo);
						}

						m_pkSkillDef = pkSkillDef;

						if(pkSkillDef)
						{
							m_SkillType = (ESkillType)pkSkillDef->GetType();
							m_iEffectNo = pkSkillDef->GetEffectNo();
						}
					}
					else if(0 == _stricmp(pcName,"TYPE"))
					{
						m_kActionType = pcValue;
					}
					else if(0 == strcmp(pcName, "NEXT"))
					{
						m_kNextActionName = pcValue;
					}
					else if(0 == strcmp(pcName, "LOOP"))
					{
						SetActionOption(AO_LOOP, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "USE_CFUNC"))
					{
						SetActionOption(AO_USE_CFUNC, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "SMALLL_AREA_CHECK"))
					{
						SetActionOption(AO_DISABLE_SMALLAREACHECK, (0 == strcmp(pcValue, "FALSE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "USE-EVENT"))
					{
						SetActionOption(AO_USE_EVENT, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "BREAK_FALL"))
					{
						SetActionOption(AO_BREAKFALL, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "CAN_FOLLOW"))
					{
						SetActionOption(AO_CAN_FOLLOW, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"CHECK_CAN_ENTER"))
					{
						SetActionOption(AO_CALL_CHECK_CAN_ENTER, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "MONITOR"))
					{
						SetActionOption(AO_DO_MONITOR, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"CAN_CHANGE_ACTOR_POS"))
					{
						SetActionOption(AO_CAN_CHANGE_ACTOR_POS, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"IGNORE_DMG_EFFECT"))
					{
						SetActionOption(AO_IGNORE_DMG_EFFECT, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "ALWAYS_IGNORE_DMG_EFFECT"))
					{
						SetActionOption(AO_ALWAYS_IGNORE_DMG_EFFECT, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"NO_RANDOM_ANIMATION"))
					{
						SetActionOption(AO_NO_RANDOM_ANIMATION, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName, "OVERRIDE_PACKET"))
					{
						SetActionOption(AO_OVERRIDE_PACKET, (0 == strcmp(pcValue, "TRUE") ? true : false));
					}
					else if(0 == strcmp(pcName, "DONT_LOOK_TARGET"))
					{
						SetActionOption(AO_DONT_LOOK_TARGET, (0 == strcmp(pcValue, "TRUE") ? true : false));
					}
					else if(0 == strcmp(pcName, "BIDIRECTION"))
					{
						SetActionOption(AO_BIDIRECTION, (0 == strcmp(pcValue, "TRUE") ? true : false));
					}
					else if(0 == strcmp(pcName,"NO_SET_WALKING_TARGET_LOC"))
					{
						SetActionOption(AO_NO_SET_WALKING_TARGET_LOC, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"USE_CFUNC_ONTARGET"))	//타겟팅을 CFUNC로 할 것인가?
					{
						SetActionOption(AO_USE_CFUNC_ONTARGET, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"NO_CHANGE_TELEPORT_ACTOR_POS"))
					{
						SetActionOption(AO_NO_CHANGE_TELEPORT_ACTOR_POS, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"IGNORE_PHYSX"))
					{
						SetActionOption(AO_IGNORE_PHYSX, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"ALWAYS_UPDATE"))
					{
						SetActionOption(AO_ALWAYS_UPDATE, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else if(0 == strcmp(pcName,"KIND_OF_TRIGGER_ACTION"))
					{
						SetActionOption(AO_KIND_OF_TRIGGER_ACTION, (0 == strcmp(pcValue, "TRUE")) ? true : false);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
					}

					pkAttr = pkAttr->Next();
				}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode const* pkChildNode = pkElement->FirstChild();
				if(0 != pkChildNode)
				{

					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(0 == strcmp(pcTagName, "SLOT"))
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				PgSlot kSlot;

				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == strcmp(pcName, "NAME"))
					{
						kSlot.m_kSlotName = pcValue;
						LWR(kSlot.m_kSlotName);
					}
					else if(0 == strcmp(pcName, "LOOP"))
					{
						kSlot.m_bLoop = (0 == strcmp(pcValue, "TRUE")) ? true : false;
					}
					else if(0 == strcmp(pcName, "USE_EVENTSCRIPT"))
					{
						kSlot.m_bEventScript = (0 == strcmp(pcValue, "TRUE")) ? true : false;
					}
					else if(0 == strcmp(pcName, "FIRE_EFFECT_NODE")
						|| 0 == strcmp(pcName, "FIRE_EFFECT")
						|| 0 == strcmp(pcName, "FIRE_EFFECT_AUTO_SCALE"))
					{
						kSlot.m_kContAnimationInfo.insert(std::make_pair(pcName,pcValue));
					}
					else if(STR_COMMON_EFFECT_ID.compare(pcName) == 0
						|| STR_COMMON_EFFECT_NODE.compare(pcName) == 0
						|| STR_COMMON_EFFECT_SCALE.compare(pcName) == 0
						|| STR_COMMON_EFFECT_TARGET.compare(pcName) == 0
						|| STR_COMMON_EFFECT_DETACH_SKIP.compare(pcName) == 0)
					{
						kSlot.m_kContAnimationInfo.insert(std::make_pair(pcName,pcValue));
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
						break;
					}
					pkAttr = pkAttr->Next();
				}

				m_kSlotContainer.push_back(kSlot);
			}
			else if(0 == strcmp(pcTagName, "FSM"))
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();

				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == strcmp(pcName, "SCRIPT"))
					{
						m_kFSMScript = pcValue;

						if(GetActionOptionEnable(AO_USE_CFUNC))
						{
							m_pkFSMFunc = g_kActionFSMFuncMgr.GetFSM(m_kFSMScript);
						}
					}
					else
					{
						m_ScriptParamMap.insert(std::make_pair(std::string(pcName),std::string(pcValue)));
					}

					pkAttr = pkAttr->Next();
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}
		break;

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	TiXmlNode const* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	return true;
}

int		PgAction::OnFindTargetFunc(PgActor* pkActor, PgAction* pkAction, PgActionTargetList &rkTargetList)	//스크립트는 루아를 쓰지만 타겟찾는건 소스코드에서 할때
{
	if ( !pkAction )
	{
		pkAction = this;
	}

	CSkillDef const* pkSkillDef = pkAction->GetSkillDef();

	if ( !pkSkillDef )
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pkSkillDef = kSkillDefMgr.GetDef(GetActionNo());
	}

	if(!pkSkillDef)
	{
		return 0;
	}

	float fSkillRange = static_cast<float>(lwAction::GetSkillRangeEx(pkSkillDef,pkActor));
	float fWideAngle = static_cast<float>(pkSkillDef->GetAbil(AT_1ST_AREA_PARAM2));
	float fEffectRadius = fWideAngle;
	NiPoint3	kStart = pkActor->GetTranslate(), kDir = pkActor->GetLookingDir();
	kStart -= (kDir*pkSkillDef->GetAbil(AT_SKILL_BACK_DISTANCE));

	switch(pkSkillDef->GetAbil(AT_1ST_AREA_TYPE))
	{
	case ESArea_Sphere:
		{
			fEffectRadius = fSkillRange;
		}break;
	case ESArea_Front_Sphere:
		{
			kDir *= fSkillRange;
			kStart += kDir;
			if(0 >= fWideAngle)
			{
				fEffectRadius = fWideAngle = fSkillRange;
			}
		}break;
	case ESArea_Cube:
	case ESArea_Cone:
		{
			if(0 == fWideAngle)
			{
				fWideAngle = 40.0f;
			}
		}break;
	}

	return FindTargets(pkAction->GetActionNo(), ServerTypeToTargetAcquireType(static_cast<ESkillArea>(pkSkillDef->GetAbil(AT_1ST_AREA_TYPE))), kStart, kDir, fSkillRange, fWideAngle, fEffectRadius, &rkTargetList, 0, true, rkTargetList, g_kPilotMan.FindPilot(pkActor->GetPilotGuid()));
}

PgAction::TargetAcquireType PgAction::ServerTypeToTargetAcquireType(ESkillArea eType)
{
	TargetAcquireType kType = TAT_SPHERE;
	switch(eType)
	{
	case ESArea_Sphere:
	case ESArea_Front_Sphere:
		{
			kType = TAT_SPHERE;
		}break;
	case ESArea_Cube:
	case ESArea_Cone:
		{
			kType = TAT_BAR;
		}break;
	}

	return kType;
}

int PgAction::GetTranslateSkillCnt()
{// 전이할수 있는 스킬들의 갯수(4차스킬 중 커맨드 입력으로, 나갈수 있는 스킬들의 갯수)
	if(m_iTranslateSkillCnt)
	{
		return m_iTranslateSkillCnt;
	}
	int iCnt = 0;
	for(int i = AT_TRANSLATE_SKILL01; i <= AT_TRANSLATE_SKILL10; ++i)
	{// 10개
		int const iTranslateSkillNo = GetAbil(i);
		if(iTranslateSkillNo)
		{
			++iCnt;
		}
		else
		{// 빈자리 없이 입력 되어야 한다
			break;
		}
	}
	SetTranslageSkillCnt(iCnt);
	return iCnt;
}