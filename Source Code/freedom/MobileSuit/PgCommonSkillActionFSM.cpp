#include "stdafx.h"
#include "Variant/DefAbilType.h"
#include "Variant/PgStringUtil.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "lwBase.h"
#include "lwDeathSnatchSkillScriptHelpFunc.h"
#include "PgPilotMan.h"
#include "lwPilot.h"
#include "lwPilotMan.h"
#include "lwActor.h"
#include "lwAction.h"
#include "lwGuid.h"
#include "lwPoint3.h"
#include "lwFindTargetParam.h"
#include "lwQuaternion.h"
#include "lwActionTargetInfo.h"
#include "lwActionTargetList.h"
#include "lwPacket.H"
#include "PgActionFSMFuncMgr.H"
#include "PgCommonSkillActionFSM.h"
#include "lwCommonSkillUtilFunc.h"
#include "lwBase.h"

using lwCommonSkillUtilFunc::ATTACK_TIMER_ID;

PgCommonSkillAction::PgCommonSkillAction()
{}
	
bool PgCommonSkillAction::OnCheckCanEnter(lwActor kActor, lwAction kAction)	const
{
	return lwCommonSkillUtilFunc::OnCheckCanEnter(kActor, kAction);
}

bool PgCommonSkillAction::OnEnter(lwActor kActor, lwAction kAction)	const
{
	if( lwCommonSkillUtilFunc::OnEnter(kActor, kAction) )
	{
		if( ESS_CASTTIME != kAction.GetActionParam() )
		{//캐스팅이 완료된 경우에만 OnCastingCompleted실행
			OnCastingCompleted(kActor, kAction);
		}
		return true;
	}
	return false;
}

void PgCommonSkillAction::OnCastingCompleted(lwActor kActor, lwAction kAction)	const
{
	lwCommonSkillUtilFunc::OnCastingCompleted(kActor, kAction);
}

void PgCommonSkillAction::OnOverridePacket(lwActor kActor, lwAction kAction, lwPacket kPacket)	const
{
	if( kActor.IsNil() )	{ lwSetBreak(); }
	if( kAction.IsNil() )	{ lwSetBreak(); }
	lwCommonSkillUtilFunc::OnOverridePacket(kActor, kAction, kPacket);
}

bool PgCommonSkillAction::OnUpdate(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)	const
{
	return lwCommonSkillUtilFunc::OnUpdate(kActor, kAction, fAccumTime, fFrameTime);
}


bool PgCommonSkillAction::OnLeave(lwActor kActor, lwAction kNextAction, bool bCancel)	const
{
	return lwCommonSkillUtilFunc::OnLeave(kActor, kNextAction, bCancel);
}

bool PgCommonSkillAction::OnCleanUp(lwActor kActor, lwAction kAction)	const
{
	return lwCommonSkillUtilFunc::OnCleanUp(kActor, kAction);
}

bool PgCommonSkillAction::OnEvent(lwActor kActor, std::string kTextKey, int iSeqID)	const
{
	return lwCommonSkillUtilFunc::OnEvent(kActor, kTextKey.c_str(), iSeqID);
}

bool PgCommonSkillAction::OnTimer(lwActor kActor, lwAction kAction, float fCallTime, int iTimerID)	const
{
	return lwCommonSkillUtilFunc::OnTimer(kActor, kAction, fCallTime, iTimerID);
}

int PgCommonSkillAction::OnFindTarget(lwActor kActor, lwAction kAction, lwActionTargetList kTargetList)
{// 일반적으로 USE_CFUNC_ONTARGET="TRUE"로 사용하게끔하고 아주 특수한 경우 커스텀하게 타겟을 찾아야할때만 이것을 사용
	return lwCommonSkillUtilFunc::OnFindTarget(kActor, kAction, kTargetList);
}

void PgCommonSkillAction::OnTargetListModified(lwActor kActor, lwAction kAction, bool bIsBefore)	const
{
	lwCommonSkillUtilFunc::OnTargetListModified(kActor, kAction, bIsBefore);
}
/*
SkillFire 부분일때 -= 화면을 어두어지게 하는 부분이 있다면, // 이것도 시점이 시작, Fire 부분등 나뉘어 질수 있지만 보통은 시작이니까 일단은 시작으로 해두자
	//if actor:IsMyActor() then
	//-- 화면 어두워지게 처리
	//	g_world:SetShowWorldFocusFilterColorAlpha(0x000000, 0.0, 1.0, 0.4,true,true);
	//	g_world:AddDrawActorFilter(actor:GetPilotGuid());
	//end

*/