#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "PgMobileSuit.h"
#include "PgActionFSMFuncs.h"
#include "PgAction.H"
#include "PgActor.H"
#include "lwActor.H"
#include "lwAction.H"
#include "lwPilot.H"
#include "PgPilot.H"
#include "lwComboAdvisor.H"
#include "PgWorld.H"
#include "lwInputSlotInfo.H"
#include "lwBase.H"
#include "lwCommonSkillUtilFunc.h"
#include "PgPilotMan.h"
#include "lwActionResult.h"
#include "lwActionTargetInfo.h"
#include "lwActionTargetList.h"
#include "lwFindTargetParam.h"
#include "PgMath.h"
#include "PgOption.h"

int const SLOTNO_IDLE_EFFECT = 20110620;
float const g_fEvasionStartTime = 0.3f;
float const g_fLandingThreshold = 70.0f;
std::string const STR_HIJUMP("HiJump");
////////////////////////////////////////////////////////////////////////////////////////////////
//	Common Functions 
////////////////////////////////////////////////////////////////////////////////////////////////
extern	bool lwKeyIsDown(int iKeyNum, bool bIsNotUKey);
extern	float	lwGetAccumTime();
extern	lwComboAdvisor	lwGetComboAdvisor();

bool Act_Melee_IsToUpAttack(lwActor actor,lwAction action)
{
	if (action.IsNil())
	{
		return	false;
	}
	
	if	(lwKeyIsDown(ACTIONKEY_UP,false))
	{
		return	true;
	}
	
	lwInputSlotInfo	kInputSlotInfo = action.GetInputSlotInfo();
	if (kInputSlotInfo.IsNil() == false)
	{
		if	(kInputSlotInfo.GetUKey() == ACTIONKEY_UP_ATTACK)
		{
			return	true;
		}
	}
	
	return	false;
	
}

bool Act_Melee_IsToDownAttack(lwActor actor,lwAction action)
{
	if (action.IsNil())
	{
		return	false;
	}
	
	if	(lwKeyIsDown(ACTIONKEY_DOWN,false))
	{
		return	true;
	}
	
	lwInputSlotInfo	kInputSlotInfo = action.GetInputSlotInfo();
	if (kInputSlotInfo.IsNil() == false)
	{
	
		if	(kInputSlotInfo.GetUKey() == ACTIONKEY_DOWN_ATTACK)
		{
			return	true;
		}
	
	}
	
	return	false;
	
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Act_Idle
////////////////////////////////////////////////////////////////////////////////////////////////
bool	PgActionFSM_Act_Idle::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string actorID = actor.GetID();

	//펫 탑승 중이라면 탑승 동작으로 변경
	if(actor.IsRidingPet())
	{
		action.SetSlot(3);
		actor.PlayCurrentSlot(true);
		actor.ReserveTransitAction(ACTIONNAME_RP_IDLE, action.GetDirection());
		return true;
	}

	//	만약 stun 상태라면, stun 액션으로 전이시킨다.
	if(actor.IsStun())
	{
		actor.ReserveTransitAction("a_stun",DIR_NONE);
		return	false;
	}
	
	int const iSpecificIdleActionNo = actor.GetSpecificIdle();
	if(iSpecificIdleActionNo)
	{
		action.SetParamInt(100, 1);		// SpecificIdle
		switch(iSpecificIdleActionNo)
		{
		case ESIT_BOSS_MONSTER_IDLE:
			{
				actor.ResetAnimation();
				actor.ReserveTransitAction("a_SpecificIdle", DIR_NONE);
				return false;
			}break;
		case ESIT_NONE:
			{
			}break;
		default:
			{
				if(iSpecificIdleActionNo != action.GetActionNo())
				{
					actor.ReserveTransitActionByActionNo(actor.GetSpecificIdle(), actor.GetDirection());
					return false;
				}
			}break;
		}
	}
	if( actor.IsUnitType(UT_SUB_PLAYER) )
	{// 보조 캐릭터에 맞는 액션으로 바꿔주고(메인 캐릭터를 따라가는)
		actor.ReserveTransitAction("a_twin_sub_trace_ground", DIR_NONE);
		return false;
	}

	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() && actor.IsMyActor())
		{
			if (actor.IsUnitType(UT_PLAYER) || actor.IsUnitType(UT_PET))
			{
				BYTE	byDir = actor.GetDirection();
				if (byDir != DIR_NONE)
				{
					actor.ReserveTransitAction(ACTIONNAME_RUN,byDir);
					return false;
				}
			}
		}
	}


	if(std::string(action.GetID()) == ACTIONNAME_BIDLE && actor.IsUnitType(UT_PLAYER))
	{
		action.SetSlot(1);
	}

	actor.Stop();

	if(EMGRADE_UPGRADED >= actor.GetAbil(AT_GRADE) || actor()->IsUseBattleIdle())
	{
		if(actor.IsUnitType(UT_MONSTER) && actor.HasTarget())
		{
			action.SetSlot(2);
		}
	}

	if(actor.IsUnitType(UT_PLAYER))
	{
		action.SetParamFloat(2,15);
	}                                           
	else if(actor.IsUnitType(UT_MONSTER))
	{
		action.SetParamFloat(2,4);
	}
	else if(actor.IsUnitType(UT_PET))
	{
		action.SetParamFloat(2,8);
	}

	action.SetParamInt(4, 4 + BM::Rand_Index(4));
	action.SetParamInt(5, 0);
	action.SetParamInt(6, 0);
	action.SetParamFloat(13,-1);

	if(false==actor()->IdleEffectName().empty())
	{
		actor.AttachParticle(SLOTNO_IDLE_EFFECT, actor()->IdleEffectNode().c_str(), actor()->IdleEffectName().c_str());
	}
	return	true;
}


float	g_fLastAutoFireTime = 0;
void DoAutoFire(lwActor actor)
{
	lwCheckNil(actor.IsNil());

	if (actor.IsMyActor())
	{
		if (lwKeyIsDown(ACTIONKEY_ATTACK,false))
		{
			int	const	iBaseActorType = actor.GetPilot().GetBaseClassID();
			float	fAutoFireDelayTime = lua_tinker::call<float,int>("GetAutoFireDelayTime",iBaseActorType);
			float	fNowTime = lwGetAccumTime();
			float	fElapsedTime = fNowTime - g_fLastAutoFireTime;
			if (fElapsedTime>fAutoFireDelayTime)
			{
				char const*	pkAutoFireActionID = actor.GetNormalAttackActionID();

				if(PgAction::CheckCanEnter(actor(),pkAutoFireActionID,false) == false)
				{
					return;
				}

				g_fLastAutoFireTime = fNowTime;
				actor.ReserveTransitAction(pkAutoFireActionID,DIR_NONE);
			}
		}
	}
}
bool	PgActionFSM_Act_Idle::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int	currentSlot = action.GetCurrentSlot();
	//std::string param = action.GetParam(0);
	int	iIdleType = action.GetParamInt(6);
	
	if (actor.IsCheckMeetFloor())
	{
		if(actor.IsMeetFloor() == false && actor.IsMyActor())
		{
			lwAction kAction = actor.ReserveTransitAction(ACTIONNAME_JUMP,0);
			kAction.SetSlot(2);
			kAction.SetDoNotBroadCast(true);
			return false;
		}
	}

	if (action.GetParamFloat(13) == -1)
	{
		action.SetParamFloat(13,accumTime);
	}

	if (actor.IsMyActor() == false)
	{
		BYTE dir = actor.GetDirection();
		if (dir != DIR_NONE)
		{
			actor.ReserveTransitAction(ACTIONNAME_RUN, dir);
			return true;
		}
	}
	
	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() && actor.IsMyActor())
		{
			if (actor.IsUnitType(UT_PLAYER) || actor.IsUnitType(UT_PET))
			{
				BYTE	byDir = actor.GetDirection();
				if (byDir != DIR_NONE)
				{
					actor.ReserveTransitAction(ACTIONNAME_RUN,byDir);
					return true;
				}
			}
		}
	}

	DoAutoFire(actor);

	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() == false 
			&& actor.GetAbil(AT_MONSTER_TYPE) != 1 
			&& actor.IsUnitType(UT_OBJECT) == false)
		{
			if (strcmp(action.GetParam(119),"jump_trap") == 0 || actor.GetVelocity().GetZ() < 0 )
			{
				action.SetNextActionName(ACTIONNAME_JUMP);
				action.SetParam(3, "fall_down");
				return false;
			}
		}
	}

	if (actor.IsAnimationDone() == true)
	{
		actor.ResetAnimation();
	
		if (iIdleType == 0)
		{
		
			int iBaseIdleLoopNum = action.GetParamInt(4);
			int iBaseIdleLoopCurNum = action.GetParamInt(5);
			
			iBaseIdleLoopCurNum=iBaseIdleLoopCurNum+1;
			
			if (iBaseIdleLoopCurNum>= iBaseIdleLoopNum)
			{
				action.SetParamInt(6,1);
				actor.PlayCurrentSlot(false);
			}
			else
			{
				action.SetParamInt(5,iBaseIdleLoopCurNum);
				actor.PlayCurrentSlot(true);	
			}
		}
		else if( iIdleType == 1 )
		{
			
			action.SetParamInt(6, 0);
			action.SetParamInt(4, 4 + BM::Rand_Index(4));
			action.SetParamInt(5, 0);
			
			actor.PlayCurrentSlot(true);			
		}
	

		return true;
	}

	if (currentSlot == 1)
	{
		if (accumTime - action.GetParamFloat(13) > 3.0)
		{
			action.SetNextActionName(ACTIONNAME_IDLE);
			return false;
		}
	}
	else if(actor.GetPilot().GetAbil(AT_IDLEACTION_TYPE) != 101 && accumTime - action.GetParamFloat(13) > 3.0 )
	{
		action.SetParamFloat(13,accumTime);
	}
	
	int const iSpecificIdleActionNo = actor.GetSpecificIdle();
	switch(iSpecificIdleActionNo)
	{
	case ESIT_BOSS_MONSTER_IDLE:
		{
			actor.ResetAnimation();
			actor.ReserveTransitAction("a_SpecificIdle", DIR_NONE);
			return false;
		}break;
	case ESIT_NONE:
		{
			if(0 < action.GetParamInt(100))
			{
				actor.ReserveTransitAction(ACTIONNAME_IDLE, actor.GetDirection());
				return false;
			}
		}break;
	default:
		{
			if(iSpecificIdleActionNo != action.GetActionNo())
			{
				actor.ReserveTransitActionByActionNo(actor.GetSpecificIdle(), actor.GetDirection());
				return false;
			}
		}break;
	}

	return true;
}
bool	PgActionFSM_Act_Idle::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	lwCheckNil(actor.GetAction().IsNil());

	if (strcmp(action.GetID(),ACTIONNAME_JUMP)==0 && strcmp(actor.GetAction().GetParam(3),"fall_down") == 0 )
	{
		// 절벽에서 떨어질 때는, ActionPacket을 보내지 않는다.
		action.SetSlot(2);
		action.SetDoNotBroadCast(true);
	}

	if(false==actor()->IdleEffectName().empty())
	{
		actor.DetachFrom(SLOTNO_IDLE_EFFECT);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Act_Walk
////////////////////////////////////////////////////////////////////////////////////////////////
bool	PgActionFSM_Act_Walk::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	//펫 탑승 중이라면 탑승 동작으로 변경
	if(actor.IsRidingPet())
	{
		actor.ReserveTransitAction(ACTIONNAME_RP_WALK, action.GetDirection());
		return true;
	}

	lwAction prevAction = actor.GetAction();
	if (prevAction.IsNil() == false)
	{
		std::string prevActionID = prevAction.GetID();
		if (prevActionID == ACTIONNAME_JUMP)
		{
			// 현재 액션이 점프이고 슬롯이 3번이 아니거나,
			// 바닥이 아니면, 전이 불가능
			int prevSlot = prevAction.GetCurrentSlot();
			if (prevSlot != 3 ||
				actor.IsMeetFloor() == false)
			{
				action.SetDoNotBroadCast(true);
				return false;
			}
		}
		else if(prevActionID == "a_walk_left" ||
			prevActionID == "a_walk_right" ||
			prevActionID == "a_walk_up" ||
			prevActionID == "a_walk_down")
		{
			action.SetDoNotBroadCast(true);
			return false;
		}
		// 그 외에는 전이 가능
	}

	actor.FindPathNormal();
	if (strcmp(action.GetID(),"a_walk_left") == 0)
	{
		actor.ToLeft(true,true);
	}
	else if(strcmp(action.GetID(),"a_walk_right") == 0)
	{
		actor.ToLeft(false,true);
	}
	
	//actor.ResetAnimation();
	
	return true;
}
bool	PgActionFSM_Act_Walk::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string actorID = actor.GetID();
	
	float fOriginalMoveSpeed = static_cast<float>(actor.GetAbil(AT_MOVESPEED));
	float movingSpeed = static_cast<float>(actor.GetAbil(AT_C_MOVESPEED));
	float fCorrectedSpeed = static_cast<float>(actor.GetAbil(AT_ADD_MOVESPEED_BY_DELAY));

	if(g_pkWorld)
	{
		if(g_pkWorld->GetAttr() & GATTR_VILLAGE)
		{
			fOriginalMoveSpeed += static_cast<float>(actor.GetAbil(AT_VILLAGE_MOVESPEED));
			movingSpeed += static_cast<float>(actor.GetAbil(AT_C_VILLAGE_MOVESPEED));
		}
	}
	
	if(0<fCorrectedSpeed)
	{
		movingSpeed = fCorrectedSpeed;	//보정된 값
	}

	if (fOriginalMoveSpeed == 0)
	{
		fOriginalMoveSpeed = movingSpeed;
	}
		
	float	fAnimSpeed = 0.0f;
	
	if (0 < fOriginalMoveSpeed)
	{
		fAnimSpeed = movingSpeed/fOriginalMoveSpeed;
	}
	
	char const* pkAutoSpeed = actor.GetAnimationInfo("USE_AUTO_ANI_SPEED",0);
	if(NULL==pkAutoSpeed || 0!=strcmp(pkAutoSpeed,"FALSE"))
	{
		actor.SetAnimSpeed(fAnimSpeed);
	}
	
	if (actor.IsUnitType(UT_PLAYER))
	{
		movingSpeed = movingSpeed * 0.6f;
	}

#ifndef EXTERNAL_RELEASE
	if (g_pkApp->IsSingleMode() == true)
	{
		movingSpeed = 120.0f * 0.6f;
	}
#endif

	BYTE dir = actor.GetDirection() ;

    //ODS("Act_Walk_OnUpdate actor."..actorID.." GUID."..actor.GetPilotGuid().GetString().." movingSpeed."..movingSpeed.."\n");
	if (actor.Walk(dir, movingSpeed, frameTime,false) == false)
	{
	    return  false;
	}

	lwPoint3 vel = actor.GetVelocity();
	float z = vel.GetZ();

	// 뛰어가다가 발이 땅에서 떨어졌을 경우
	// 올라가는 점프를 해야 할지, 내려오는 점프를 해야 할지 결정
	if (actor.IsMeetFloor() == false)
	{
		if (z < -2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_down");
			return false;
		}
		else if (z > 2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_up");
			return false;
		}
	}


	return true;
}
bool	PgActionFSM_Act_Walk::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	lwAction curAction = actor.GetAction();

	if (curAction.IsNil() == false && (strcmp(action.GetID(),ACTIONNAME_JUMP) ==0) )
	{
		std::string const param = curAction.GetParam(2);
		if (param == "fall_down")
		{
			action.SetSlot(2);
		}
		else if (param == "fall_up" || param == "null")
		{
			action.SetSlot(1);
		}
	}

	return true;
}
bool	PgActionFSM_Act_Walk::OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const
{
	lwCheckNil(actor.IsNil());

	if (kTextKey == "start")
	{
		actor.AttachParticle(BM::Rand_Range(-200, -100),"char_root", "e_run");
	}

	return	true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Act_Run
////////////////////////////////////////////////////////////////////////////////////////////////

bool	PgActionFSM_Act_Run::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

//	if action.GetEnable() == false then
		// 음.. 일단 주석처리..
//		local up = actor.GetActionState("a_run_up")
//		local down = actor.GetActionState("a_run_down")
//		local right = actor.GetActionState("a_run_right")
//		local left = actor.GetActionState("a_run_left")		
	
//		if up ~=0 or down ~=0 or right ~=0 or left ~=0 then
//			return false
//		end
//	end
	
	lwAction curAction = actor.GetAction();
	if(false==curAction.IsNil())
	{
		std::string curActionID = curAction.GetID();
		if (curActionID == ACTIONNAME_RUN && actor.IsNowFollowing() == false)
		{
			_PgOutputDebugString("Current Action is \"a_run\" . transit failed!\n");
			return false ;
		}
		int const iSpecificActionNo = actor.GetAbil(AT_SKILL_SPECIFIC_RUN);
		if(0 < iSpecificActionNo
			//&& iSpecificActionNo != curAction.GetActionNo()
			)
		{
			actor.ReserveTransitActionByActionNo(iSpecificActionNo, actor.GetDirection());
			return false;
		}
	}
	
	return true;
}
bool	PgActionFSM_Act_Run::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if( actor.IsUnitType(UT_MONSTER) && actor.GetAnimationLength(action.GetSlotAnimName(0)) == 0 )
	{
		action.SetSlot(1);	//run이 없으면 walk모션으로 보임
		actor.PlayCurrentSlot(true);
	}

	if( actor.IsMyActor() )
	{//동기화 맞지 않는 문제 일부 수정 : 이동 하기 전에 캐릭터 방향을 다시 받도록 수정
		NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
		if( pkKeyboard)
		{
			int const iDirKeySet[4] = {ACTIONKEY_LEFT, ACTIONKEY_RIGHT, ACTIONKEY_UP, ACTIONKEY_DOWN};
			BYTE const byDirSet[4] = {DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN};
			for( int iCount = 0; iCount<4; ++iCount )
			{
				int const iKeyNum = g_kGlobalOption.GetUKeyToKey(iDirKeySet[iCount]);
				NiInputKeyboard::KeyCode kKeycode = static_cast<NiInputKeyboard::KeyCode>(iKeyNum);
				bool const bKeyDown = pkKeyboard->KeyIsDown( kKeycode );
				if( bKeyDown )
				{
					g_kPilotMan.UpdateDirectionSlot(iDirKeySet[iCount], true);
				}
			}
		}
	}

	lwAction prevAction = actor.GetAction();

	//if (actor.IsMyActor())
	//{
	//	_PgOutputDebugString("Run is Entered PrevAction . "..actor.GetAction().GetID().." \n");
	//}

	//펫 탑승 중이라면 탑승 동작으로 변경
	if(actor.IsRidingPet())
	{
		actor.ReserveTransitAction(ACTIONNAME_RP_WALK, action.GetDirection());
		return true;
	}

	if (prevAction.IsNil() == false)
	{
		std::string prevActionID = prevAction.GetID();
		int prevSlot = prevAction.GetCurrentSlot();
		if (prevActionID == ACTIONNAME_JUMP)
		{
			// 현재 액션이 점프이고 슬롯이 3번이 아니거나,
			// 바닥이 아니면, 전이 불가능
			if (prevSlot != 3 || actor.IsMeetFloor() == false)
			{
				action.SetDoNotBroadCast(true);
				//ODS("__________________return false on run action _________________\n")
				return false;
			}
		}
		else if (prevActionID == ACTIONNAME_RUN)
		{
			return false;
		}
		// 그 외에는 전이 가능
	}

	actor.UseSkipUpdateWhenNotVisible(false);
	action.SetParamInt(1,0);

	lwPoint3	kTargetPos = action.GetParamAsPoint(0);
	if (kTargetPos.IsZero() == false )
	{
		action.SetParamInt(3,1);
		
		lwPoint3	kMoveDirection = kTargetPos.Subtract2(actor.GetPos());
		kMoveDirection.Unitize();
		action.SetParamAsPoint(1,kMoveDirection);
		action.SetParamAsPoint(2,actor.GetPos());
		
		actor.SetMovingDir(kMoveDirection);
		actor.LookAt(kTargetPos,true,true,false);
		actor.ConcilDirection(kMoveDirection, true);
	}

	if (action.GetParamInt(5) == 1)
	{
		actor.BackMoving(true);
	}

	//시작 위치와 시작 시간을 기록
	action.SetParamAsPoint(7, actor.GetPos());
	action.SetParamFloat(8, static_cast<float>(g_kEventView.GetServerElapsedTime()));
	
	//lwCommonSkillUtilFunc::TryMustChangeSubPlayerAction(actor, "a_twin_sub_trace_ground", prevAction.GetDirection());

	return true;
}
bool	PgActionFSM_Act_Run::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string actorID = actor.GetID();
	float movingSpeed = 0.0f;
	bool	bMoveToPos = (action.GetParamInt(3) == 1);
	float	fCustomSpeed = static_cast<float>(action.GetParamInt(4));

#ifndef EXTERNAL_RELEASE
	if (g_pkApp->IsSingleMode() == true)
	{
		movingSpeed = 120.0f ;
	}
	else
#endif
	{
		movingSpeed = static_cast<float>(actor.GetAbil(AT_C_MOVESPEED));
	}

	if(0.0f != fCustomSpeed)
	{
		movingSpeed = fCustomSpeed;
	}
	
	float fOriginalMoveSpeed = static_cast<float>(actor.GetAbil(AT_MOVESPEED));
	if (0.0f == fOriginalMoveSpeed)
	{
		fOriginalMoveSpeed = movingSpeed;
	}

	if(g_pkWorld)
	{
		if(g_pkWorld->GetAttr() & GATTR_VILLAGE)
		{
			movingSpeed += static_cast<float>(actor.GetAbil(AT_C_VILLAGE_MOVESPEED));
		}
	}
	
	float fAnimSpeed = 0.0f;
	
	if (0.0f < fOriginalMoveSpeed)
	{
		fAnimSpeed = movingSpeed/fOriginalMoveSpeed;
	}
	
	char const* pkAutoSpeed = actor.GetAnimationInfo("USE_AUTO_ANI_SPEED",0);
	if(NULL==pkAutoSpeed || 0!=strcmp(pkAutoSpeed,"FALSE"))
	{
		actor.SetAnimSpeed(fAnimSpeed);
	}
	
	DoAutoFire(actor);
	
	if (actor.IsMyActor() && action.GetParamInt(1) == 0 )
	{
		if (accumTime - action.GetActionEnterTime() > 0.1f)
		{
			actor.SetComboCount(0);
			action.SetParamInt(1,1);
		}
	}
	
	if (bMoveToPos)
	{
		lwPoint3	kMoveTargetPos = action.GetParamAsPoint(0);
		lwPoint3	kMoveDirection = action.GetParamAsPoint(1);	
		lwPoint3	kMoveStartPos = action.GetParamAsPoint(2);
		
		lwPoint3	kDir1 = actor.GetPos().Subtract2(kMoveTargetPos);
		kDir1.Unitize();
		lwPoint3	kDir2 = kMoveStartPos.Subtract2(kMoveTargetPos);
		kDir2.Unitize();
		
		if (0 > kDir1.Dot(kDir2) || 5 > actor.GetPos().Distance(kMoveTargetPos))
		{
			actor.SetTranslate(kMoveTargetPos,false);
			return	false;
		}
		
		kMoveDirection.Multiply(movingSpeed);
		actor.SetMovingDelta(kMoveDirection);
	
		return	true;
	}
	
	BYTE dir = actor.GetDirection();
	
	if (dir == DIR_NONE)
	{
		if (actor.GetWalkingToTarget() == false)
		{
			//ODS("[Act_Run_OnUpdate] Direction is None . transit Next Action\n")
			//--actor.FindPathNormal()
			return false ;
		}
	}
	
	if (movingSpeed == 0)
	{
	    return  false;
	}

	//NiPoint3 kNewPos = actor.GetPos()();
	//NiPoint3 kOldPos = action.GetParamAsPoint(7)();
	//kOldPos.z = kNewPos.z = 0.0f; //Z를 무시하고 계산한다. 떨어지는 것은 중력에 의한 것

	//float const fDistance = (kNewPos - kOldPos).Length();

	////이동한 거리가 있을 경우
	//if(0.0f < fDistance)
	//{
	//	NiPoint3 kDirection = kNewPos - kOldPos;
	//	kDirection.Unitize();
	//	kDirection *= movingSpeed * frameTime;
	//	kDirection += kNewPos;

	//	//지금까지 이동한 거리
	//	float const fDistance2 = (kDirection - kOldPos).Length();

	//	//시뮬레이션
	//	float const fElapsedTime = (static_cast<float>(g_kEventView.GetServerElapsedTime()) - action.GetParamFloat(8)) / 1000.0f;
	//	float const fMovingSpeed2 = static_cast<float>(actor.GetAbil(AT_MOVESPEED)) * fElapsedTime;
	//	NiPoint3 kDirection2 = kNewPos - kOldPos;
	//	kDirection2.Unitize();
	//	kDirection2 *= fMovingSpeed2;
	//	kDirection2 += kNewPos;

	//	float const fDistance3 = (kDirection2 - kOldPos).Length();

	//	if(fDistance2 > fDistance3)
	//	{
	//		int i = 0;
	//		movingSpeed += movingSpeed * (fElapsedTime-frameTime);
	//	}
	//}

	actor.Walk(dir, movingSpeed,0,false);

	lwPoint3 vel = actor.GetVelocity();
	float z = vel.GetZ();

	// 뛰어가다가 발이 땅에서 떨어졌을 경우
	// 올라가는 점프를 해야 할지, 내려오는 점프를 해야 할지 결정
	if (actor.IsMeetFloor() == false)
	{
		if (z < -2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_down");
			//ODS("Jump(fall down)\n")
			return false;
		}
		else if (z > 2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_up");
			//ODS("Jump(fall up)\n")
			return false;
		}
	}

	// 사다리 체크
	if (actor.ContainsDirection(DIR_UP) && 
		actor.IsMyActor() &&
		actor.ClimbUpLadder())
	{
		action.SetNextActionName("a_ladder_idle");
		return false;
	}
	
	return true;
}
bool	PgActionFSM_Act_Run::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	lwAction curAction = actor.GetAction();

	lwCheckNil(curAction.IsNil());
	std::string kActionID = action.GetID();
	//ODS("Act_Run_OnLeave nextid . "..action.GetID().."\n");

	if (kActionID == ACTIONNAME_JUMP)
	{
		std::string const param = curAction.GetParam(2);
		if (param == "fall_down")
		{
			action.SetSlot(2);
			action.SetDoNotBroadCast(true);
		}
		else if (param == "fall_up")
		{
			action.SetSlot(1);
			action.SetDoNotBroadCast(true);
		}
		else if (param == "null")
		{
			if(actor.IsMeetFloor())
				action.SetSlot(1);
			else
				action.SetSlot(2);
		}
	}
	else if (kActionID == "a_ladder_idle" ||
		kActionID == "a_ladder_down" ||
		kActionID == "a_ladder_up" )
	{
		actor.HideParts(6, true);
		actor.SetParam("LADDER_WEAPON_HIDE","TRUE");
	}
	else if (kActionID == ACTIONNAME_IDLE)
	{
		//action.SetDoNotBroadCast(true)
		return true;
	}

	actor.UseSkipUpdateWhenNotVisible(true);

	return true;
}
bool	PgActionFSM_Act_Run::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	actor.BackMoving(false);
	return	true;
}

bool	PgActionFSM_Act_Run::OnEvent(lwActor actor,std::string textKey,int iSeqID)	const
{
	lwCheckNil(actor.IsNil());
	if (textKey == "start")
	{
		actor.AttachParticle(BM::Rand_Range(-200, -100), "char_root", "e_run");
	}
	return	true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Act_Dash
////////////////////////////////////////////////////////////////////////////////////////////////

bool	PgActionFSM_Act_Dash::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	//펫 탑승 중이라면 대쉬 불가
	if(actor.IsRidingPet())
	{
		return false;
	}
	lwAction prevAction = actor.GetAction();
	if( false == prevAction.IsNil() )
	{
		int const iSpecificActionNo = actor.GetAbil(AT_SKILL_SPECIFIC_DASH);
		if(0 < iSpecificActionNo
			//&& iSpecificActionNo != prevAction.GetActionNo()
			)
		{
			actor.ReserveTransitActionByActionNo(iSpecificActionNo, actor.GetDirection());
			return false;
		}
	}

	// 기획팀장님이 공중에서 대쉬 할 수 있게 해달랍니다;;(단 한 번만)
	if (actor.IsMeetFloor() == false)
	{
		if (actor.GetJumpAccumHeight()<50)
		{
			return false;
		}
	}
	
	if (actor.IsOnlyMoveAction())
	{
		return false;
	}
#ifndef EXTERNAL_RELEASE
	if (g_pkApp->IsSingleMode() == false)
#endif
	{
		//	이동 속도가  0이면 안된다.
		if (actor.GetAbil(AT_C_MOVESPEED) == 0)
		{
			return	false;
		}
	}

	return true;
}
bool	PgActionFSM_Act_Dash::OnEnter(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	//현재 사용 하지 않는다
	//if (actor.IsMyActor())
	//{
	//	if (actor.GetPilot().IsCorrectClass(UCLASS_ASSASSIN,false))
	//	{
	//		if (action.CheckCanEnter(actor,"a_ass_attk_dash",false))
	//		{
	//		
	//			if (action.GetDirection() != DIR_LEFT &&
	//				action.GetDirection() != DIR_RIGHT)
	//			{
	//		
	//				action.SetNextActionName("a_ass_attk_dash");
	//				action.ChangeToNextActionOnNextUpdate(true, true);
	//				return	true;
	//			
	//			}
	//		}
	//	}
	//}


	if (actor.IsMyActor() == false)
	{
//		ODS("======================== Other actor's dash Begin =====================\n")
		actor.ClearReservedAction();
	}

	float	fDashSpeed = lua_tinker::call<float>("GetDashSpeed");

	action.SetParamFloat(0, g_pkWorld->GetAccumTime());	// Start Time
	action.SetParamFloat(1, fDashSpeed);	// Start Velocity
	action.SetParam(2,"");

	action.SetParamAsPoint(7, actor.GetPos());
	action.SetParamInt(8, static_cast<int>(g_kEventView.GetServerElapsedTime()));
	
	actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
	
	actor.FindPathNormal();
	lwPoint3	pt = actor.GetTranslate();
	pt.SetZ(pt.GetZ()-30);
	//actor.AttachParticleToPoint(201, pt, "e_special_transform")	
	actor.SetComboCount(0);
	
	if (actor.IsMyActor() == false)
	{
		actor.SetTranslate(action.GetActionStartPos(),false);
	}
	
	actor.UseSkipUpdateWhenNotVisible(false);
	action.SetParamInt(4,0);
	
	if (actor.IsMyActor())
	{
	
        lwGetComboAdvisor().OnNewActionEnter(action.GetID());	
        lwGetComboAdvisor().OnNewActionEnter(action.GetID());	
        
		std::string kNormalAttackActionID = actor.GetNormalAttackActionID();
        lwGetComboAdvisor().AddNextAction("a_dash_attack");    
        lwGetComboAdvisor().AddNextAction("a_dash_jump");    
        lwGetComboAdvisor().AddNextAction("a_dash_blowup");    
        lwGetComboAdvisor().AddNextAction("a_clown_sliding_tackle");    
	}
	
	// actor.SetDirection(action.GetDirection());

	return true;
}
bool	PgActionFSM_Act_Dash::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int	iState = action.GetParamInt(4);

	std::string jumping = "TRUE";
	if (actor.IsJumping() == false)
	{
		jumping = "FALSE";
	}

	//ODS("__________Jumping . " .. jumping .. "________\n")

	
	if (std::string(action.GetParam(2)) == "" && 
	
		( std::string(action.GetNextActionName()) == "a_dash_attack"
		|| std::string(action.GetNextActionName()) == "a_magician_dash_attack"
		|| std::string(action.GetNextActionName()) == "a_thi_dash_attack"
		|| std::string(action.GetNextActionName()) == "a_archer_dash_attack"
		|| std::string(action.GetNextActionName()) == "a_Sum_dash_attack"
		|| std::string(action.GetNextActionName()) == "a_twin_dash_attack"
		)
	  )
	{
		
	    action.SetNextActionName(ACTIONNAME_IDLE);
	    
	}
	
	if (std::string(action.GetParam(2)) == "ToDashAttack!")
	{
	    action.SetParam(2,"end");
		return false;
	};
	
	if (iState == 0)
	{
		actor.FindPathNormal();		
		
		float fAccel = -1000 * frameTime;
		float fVelocity = action.GetParamFloat(1);
		
//		local kMovingDir = actor.GetLookingDir()
//		kMovingDir.Multiply(fVelocity)
		
		BYTE dir = action.GetDirection();

		//ODS("______________Direction . " .. dir .. "\n")

		
		//현재는 일단 막아 둔다.
		//최대 대쉬 거리보다 많이 가는 경우가 생길 수 있으므로 시뮬레이션 후 최대값 이상 가지 못하도록 한다.
		NiPoint3 kNewPos = actor.GetPos()();
		NiPoint3 kOldPos = action.GetParamAsPoint(7)();
		kOldPos.z = kNewPos.z = 0.0f; //Z를 무시하고 계산한다. 떨어지는 것은 중력에 의한 것

		float const fDistance = (kNewPos - kOldPos).Length();

		//이동한 거리가 있을 경우
		if(0.0f < fDistance)
		{
			float const fMaxDistance = 130.0f;
			//남은 이동 거리
			float const fDistance2 = fMaxDistance - fDistance;

			//두 벡터를 이용하여 방향 벡터를 구하고
			//방향 벡터를 이용하여 실제로 이동하는 곳의 위치를 구한다.
			NiPoint3 kDirection = kNewPos - kOldPos;
			kDirection.Unitize();
			kDirection *= fVelocity * frameTime;
			kDirection += kNewPos;

			// 이번 프레임에 이동 하는 거리
			float const fDistance3 = (kDirection - kOldPos).Length();

			//이미 최대 대쉬 이동거리 거리를 벗어난 경우
			if(fMaxDistance - fDistance3 < 0)
			{
				//fVelocity값을 다시 세팅해야 한다.				
				fVelocity = fDistance2 / frameTime;
			}
		}

		actor.Walk(dir, fVelocity,0,false);
		
		fVelocity = fVelocity + fAccel;
		fVelocity = __max(0, fVelocity);
		action.SetParamFloat(1, fVelocity);
		
		float fElapsedTime = accumTime - action.GetParamFloat(0);
		if (0.3f < fElapsedTime)
		{
			// TODO . ElapsedTime이 0.3에서 얼마나 벗어났는지 확인 후에, 그만큼 뒤로 당겨 주어야 한다.
			action.SetParamInt(4,1);
			action.SetParamFloat(5, accumTime);
		}
	}
	else
	{
		float fElapsedTime = accumTime - action.GetParamFloat(5);
		float	fDashFreezeTime = lua_tinker::call<float>("GetDashFreezeTime");
		if (fElapsedTime>=fDashFreezeTime)
		{
			action.SetParam(2,"end");
			if (actor.IsMeetFloor() == false)
			{
				action.SetNextActionName(ACTIONNAME_JUMP);
			}
			return false;
		}
	}
	
	if (std::string(action.GetParam(3)) == "EndNow")
	{
		return false;
	}
		
	return	true;
}
bool	PgActionFSM_Act_Dash::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	actor.UseSkipUpdateWhenNotVisible(true);
	actor.EndBodyTrail();

	int iStartTime = actor.GetAction().GetParamInt(8);
	int iEndTime = static_cast<int>(g_kEventView.GetServerElapsedTime());

	_PgOutputDebugString("a_Dash Moving Time : %d\n", iEndTime - iStartTime);
	NILOG(PGLOG_WARNING, "a_Dash Moving Time : %d\n", iEndTime - iStartTime);

	return true;
}
bool	PgActionFSM_Act_Dash::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string kNextActionID = action.GetID();
	if("a_rest" == kNextActionID)
	{
		return true;
	}
	
	lwAction kCurAction = actor.GetAction();

	lwCheckNil(kCurAction.IsNil());
	std::string newActionID = action.GetID();

	lwCheckNil(actor.GetPilot().IsNil());
    int	iBaseActorType = actor.GetPilot().GetBaseClassID();
	int const iEquipWeaponType = actor.GetEquippedWeaponType();
    
    if (newActionID == ACTIONNAME_JUMP)
	{
		//action.SetDoNotBroadCast(true)
		
		action.SetSlot(2);
	}

	if (std::string(action.GetActionType())=="EFFECT")
	{
		return true;
	};
	
	int weapontype = actor.GetEquippedWeaponType();
	
	if( newActionID == "a_dash_attack" ||
		newActionID == "a_thi_dash_attack" ||
		newActionID == "a_nj_dash_attack" ||
		newActionID == "a_archer_dash_attack" ||
		newActionID == "a_magician_dash_attack" ||
		newActionID == "a_clown_sliding_tackle" ||
		newActionID == "a_kni_dashdownblow" ||
		newActionID == "a_dash_blowup" ||
		newActionID == "a_trap"||
		newActionID == "a_Sum_dash_attack"
		|| "a_twin_dash_attack" == newActionID
		|| "a_twin_screw_drop_kick" == newActionID
		)
	{
		action.SetParamFloat(1,kCurAction.GetParamFloat(1));
		action.SetParamAsPoint(7, kCurAction.GetParamAsPoint(7)); // 대쉬의 시작 위치를 세팅
		action.SetParamFloat(8,130.0f); //Max이동 거리
	
		return true;
	}
	
	if ((newActionID.substr(0,7) == "a_melee" || 
		newActionID.substr(0,11) == "a_thi_melee" || 
		newActionID.substr(0,10) == "a_nj_melee" ||
		newActionID == "a_archer_shot_01" || 
		newActionID == "a_MagicianShot_01" || 
		newActionID == "a_Sum_Rifle_Shot"
		|| "a_twin_melee_01_m" == newActionID
		) 
		&& weapontype!=0)
	{
		
		// Dash Attack!
		
		
		lwPilot	kPilot = actor.GetPilot();
		lwCheckNil(kPilot.IsNil());

		std::string	kNextActionName = "";
		
		if (kPilot.IsCorrectClass(UCLASS_CLOWN,false) &&
			Act_Melee_IsToDownAttack(actor,action))
		{
			
			kNextActionName = "a_clown_sliding_tackle";
		}
		else if (kPilot.IsCorrectClass(UCLASS_KNIGHT,false) &&
			 actor.IsMeetFloor() == false)
		{
			kNextActionName = "a_kni_dashdownblow";
		}
		else if (kPilot.IsCorrectClass(UCLASS_WARRIOR,false) && 
			Act_Melee_IsToUpAttack(actor,action))
		{
			kNextActionName = "a_dash_blowup";
		}
		else if (kPilot.IsCorrectClass(UCLASS_MIRAGE,false) && 
			actor.IsMeetFloor() == false)
		{
			kNextActionName = "a_twin_screw_drop_kick";
		}
		else
		{
			
			if (actor.CheckStatusEffectExist("se_transform_to_metamorphosis"))
			{
				kNextActionName = "a_nj_dash_attack";
			}
			else if (iBaseActorType == UCLASS_FIGHTER)
			{
				kNextActionName = "a_dash_attack";
				//if( 1 == g_kGlobalOption.GetValue("ETC", "USE_OLD_COMBO") )
				//{
				//	kNextActionName = "a_dash_attack";
				//}
				//else
				//{
				//	if( PgItemEx::IT_SWORD == iEquipWeaponType )
				//	{//한손검
				//		kNextActionName = "a_ohs_combo_k_a";
				//	}
				//	else if( PgItemEx::IT_BTS == iEquipWeaponType )
				//	{//양손검
				//		kNextActionName = "a_ths_combo_k_a";
				//	}
				//}
				actor.ResetAnimation();
			}
			else if (iBaseActorType == UCLASS_MAGICIAN)
			{
				kNextActionName = "a_magician_dash_attack";
			}
			else if (iBaseActorType == UCLASS_ARCHER)
			{
				kNextActionName = "a_archer_dash_attack";
			}
			else if (iBaseActorType == UCLASS_THIEF)
			{
				kNextActionName = "a_thi_dash_attack";
			}
			else if (iBaseActorType == UCLASS_SHAMAN)
			{
				kNextActionName = "a_Sum_dash_attack";
			}
			else if (UCLASS_DOUBLE_FIGHTER == iBaseActorType )
			{
				kNextActionName = "a_twin_dash_attack";
			}
		}
		
		if (kNextActionName !="" && action.CheckCanEnter(actor,kNextActionName.c_str(),true))
		{
			kCurAction.SetParam(2,"ToDashAttack!");
			kCurAction.SetNextActionName(kNextActionName.c_str());
		}
		return false;
	}

	if (std::string(kCurAction.GetParam(2)) == "end")
	{
		if (newActionID == ACTIONNAME_JUMP)
		{
			action.SetSlot(2);
		}
		return	true;
	}
	else if (newActionID == ACTIONNAME_JUMP && actor.IsJumping() == false)
	{
		if (std::string(action.GetParam(5)) == "HiJump" ) // hiJump면 그냥 jump로 넘어감.
		{
			action.SetSlot(1);
			kCurAction.SetParam(3, "EndNow");
			return true;
		}

		if(action.CheckCanEnter(actor,"a_dash_jump",false) == false)
		{
			return	false;
		}
		
		//ODS("____________To Dash Jump______________\n")
		kCurAction.SetNextActionName("a_dash_jump");
		kCurAction.SetParam(3, "EndNow");
		return false ;
	}
	else if(newActionID == "a_dash_jump")
	{
		return	action.CheckCanEnter(actor,newActionID.c_str(),false);
	}
	else if( newActionID == "a_telejump" ||
		newActionID == "a_lock_move" ||
		newActionID == "a_trap" ||
		newActionID == "a_teleport"
		|| newActionID == "a_teleport_rocket"
		)
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Act_Dash_Jump
////////////////////////////////////////////////////////////////////////////////////////////////
bool	PgActionFSM_Act_Dash_Jump::OnEnter(lwActor actor,lwAction action)	const
{

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if (actor.IsMyActor() == false)
	{
		action.SetSlot(0);
	}
	//else if( actionName != "a_dash" && actionName != "a_ass_attk_dash" )
	//{
	//	return false;
	//}

	float	jumpForce = lua_tinker::call<float>("GetJumpForce");

	actor.StartJump(jumpForce);
	actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
	
//	if actor.IsMyActor() == false then
//		actor.SetTranslate(action.GetActionStartPos());
//	end
	
	
	actor.UseSkipUpdateWhenNotVisible(false);

	if (actor.IsMyActor())
	{
        lwGetComboAdvisor().OnNewActionEnter(action.GetID());	
	}
	
	//if( IsClass_OwnSubPlayer(actor.GetAbil(AT_CLASS)) )
	//{// 쌍둥이이면
	//	lwActor kSubActor = actor.GetSubPlayer();
	//	if( !kSubActor.IsNil() )
	//	{
	//		lwAction kSubActorCurAction = kSubActor.GetAction();
	//		if( !kSubActorCurAction.IsNil() )
	//		{
	//			kSubActorCurAction.SetDirection( actor.GetAction().GetDirection() );
	//			kSubActorCurAction.SetNextActionName("a_dash_jump");
	//			kSubActorCurAction.ChangeToNextActionOnNextUpdate(true,true);
	//		}
	//	}
	//}
	
	lwAction prevAction = actor.GetAction();
//	lwCheckNil(prevAction.IsNil());
	if( false == prevAction.IsNil() )
	{
		int iDir = prevAction.GetDirection();
		if( actor.IsUnitType(UT_SUB_PLAYER) )
		{
			lwActor kCaller = actor.GetCallerActor();
			if( !kCaller.IsNil() )
			{
				lwAction kCallerAction = kCaller.GetAction();
				if( !kCallerAction.IsNil() )
				{
					iDir = kCallerAction.GetDirection();
				}
			}
		}
		action.SetParamInt(911, iDir);

		lwActor kSubActor = actor.GetSubPlayer();
		if( false == kSubActor.IsNil() )
		{
			lwAction kSubAction = kSubActor.GetAction();
			if( false == kSubAction.IsNil() )
			{
				if( 0 != strcmp( "BlowUp_Down", kSubAction.GetActionName() ) 
					&& 0 != strcmp( "BlowUp_Loop", kSubAction.GetActionName() )
					&& 0 != strcmp( "BlowUp_Start", kSubAction.GetActionName() ) )
				{//서브캐릭터 상태가 다운 상태면 강제 전이 시키지 않도록 하자.
					lwCommonSkillUtilFunc::TryMustChangeSubPlayerAction( actor, action.GetID(), iDir, 0, true );
				}
			}
		}
	}

	return true;
}
bool	PgActionFSM_Act_Dash_Jump::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const
{
	lwCheckNil(action.IsNil());
	lwCheckNil(actor.IsNil());
	int curAnimSlot = action.GetCurrentSlot(); // Current Animation Slot

	float	fDashJumpSpeed = lua_tinker::call<float>("GetDashJumpSpeed");
	float movingSpeed = fDashJumpSpeed;

	bool IsAnimDone = actor.IsAnimationDone();
	BYTE dir = action.GetParamInt(911);
	
	if (curAnimSlot != 2)
	{
		actor.Walk(dir, movingSpeed,0,false);
	}

	if (curAnimSlot == 0)
	{
		if (IsAnimDone == true )
		{
			actor.PlayNext();
		}
	}
	else if (curAnimSlot == 1)
	{
		if (actor.IsMeetFloor())
		{
			dir = actor.GetDirection();
			if (dir == DIR_NONE)
			{
				actor.PlayNext();
			}
			else
			{
				action.SetNextActionName(ACTIONNAME_RUN);
				action.SetParam(4808, "end");
			}
		}
	}
	else if (curAnimSlot == 2)
	{
		if (IsAnimDone == true)
		{
			action.SetNextActionName(ACTIONNAME_IDLE);
			action.SetParam(4808, "end");
		}
		else if (actor.GetDirection() != DIR_NONE)
		{
			action.SetNextActionName(ACTIONNAME_RUN);
			action.SetParam(4808, "end");
		}
	}

	if ( std::string(action.GetParam(4808)) == "end" )
	{
		if( actor.IsUnitType( UT_SUB_PLAYER ) )
		{
			lwCommonSkillUtilFunc::TryMustChangeActorAction(actor, "a_twin_sub_trace_ground");
		}
		return false;
	}

	return true;
}
bool	PgActionFSM_Act_Dash_Jump::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	actor.UseSkipUpdateWhenNotVisible(true);
	actor.EndBodyTrail();

	return	true;
}
bool	PgActionFSM_Act_Dash_Jump::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	lwAction kCurAction = actor.GetAction();
	lwCheckNil(kCurAction.IsNil());

	std::string newActionID = action.GetID();
	int iCurrnetSlot = kCurAction.GetCurrentSlot();

	//ODS("__________Dash_Jump's NextAction . " .. newActionID .. "\n")
	
	if (actor.IsMyActor() == false || 
		std::string(action.GetActionType()) == "EFFECT")
	{
		return true;
	}
	
	if ( std::string(kCurAction.GetParam(4808)) == "end" &&
		(iCurrnetSlot == 1 ||
		iCurrnetSlot == 2))
	{
		if (newActionID == ACTIONNAME_RUN ||
			newActionID == ACTIONNAME_IDLE)
		{
			return true;
		}
	}
	else if (newActionID == "a_telejump" ||
		newActionID == "a_lock_move" ||
		newActionID == "a_trap" || 
		newActionID == "a_teleport"
		|| newActionID == "a_teleport_rocket"
		)
	{
		return true;
	}
	else if (newActionID == ACTIONNAME_JUMP)
	{
		if (std::string(action.GetParam(5)) == "HiJump") // hiJump면 그냥 jump로 넘어감.
		{
			kCurAction.SetParam(4808, "EndNow");
			return true;
		}
	}
	
	return false;
}

bool PgActionFSM_Act_Jump::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	
	lwAction prevAction = actor.GetAction();
	//lwCheckNil(prevAction.IsNil());
	
	if( prevAction.IsNil() ) 
	{
		return	true;
	}
	int const iSpecificActionNo = actor.GetAbil(AT_SKILL_SPECIFIC_JUMP);
	if(0 < iSpecificActionNo
		//&& iSpecificActionNo != prevAction.GetActionNo()
		)
	{
		actor.ReserveTransitActionByActionNo(iSpecificActionNo, actor.GetDirection());
		return false;
	}

	lwPacket kPacket = action.GetParamAsPacket();
	if( kPacket.IsNil() == false ) 
	{
		action.SetParamFloat(4,kPacket.PopFloat());
		action.SetParamInt(7,kPacket.PopInt());
	}
		
	std::string actionName = prevAction.GetID();
	
	//	낙법 체크
	CheckBreakFall(actor);

	if( lwIsSingleMode() == false && actor.GetAbil(AT_C_MOVESPEED) == 0 ) 
	{ // 0 이면 점프 할 수 없는 상태
		return false;
	}
	
	if( IsFloatEvasion(actor,action) )
	{
		actor.ClearAllActionEffect();
		return	true;
	}
	
    if( actor.IsMeetFloor() == false ) 
	{
        if( actionName == "a_ladder_down" ||
		    actionName == "a_ladder_idle" || 
		    actionName == "a_ladder_up" ||
		    actionName == "a_ladder_dash" ||
		    actionName == "a_magician_down_shot" ||
		    actionName == "a_archer_down_shot" ||
		    actionName.substr(0,19) == "a_MagicianFloatShot" ||
		    actionName.substr(0,13) == "a_float_melee" ||
		    actionName.substr(0,16) == "a_nj_float_melee" ||
		    actionName == "a_thief_float_melee_01" ||
		    actionName == "a_down_attack_fly" ||
		    actionName == "a_mag_down_attack_fly" ||
		    actionName == "a_thi_down_attack_fly" ||
		    actionName == "a_arc_down_attack_fly" ||
			actionName == ACTIONNAME_RUN ||
			actionName == "a_storm blade" ||
			actionName == "a_mega_stormblade" ||
			actionName == "a_AscensionDragon" ||
			actionName == ACTIONNAME_IDLE ||
			actionName == "a_archer_shot_01" ||
			actionName == ACTIONNAME_JUMP ||
			actionName == "a_dash" ||
			actionName == "a_dash_jump" ||
			actionName == "a_melee_drop" ||
			actionName == "a_ran_floatdownshot" ||
			actionName == "a_fig_floatdownblow1" ||
			actionName == "a_fig_floatdownblow2" ||
			actionName == "a_Judge Dread" ||
			actionName == "a_Sky Pirce01" ||			
			actionName == "a_Screw Upper" ||
			actionName == "a_mage_down_shot"||
			actionName ==  "a_MP-Zero Trap" ||
			actionName ==  "a_Splash Trap" ||
			actionName ==  "a_Freezing Trap" ||
			actionName ==  "a_Silence Trap" ||			
			actionName ==  "a_tesla_coil" ||
			actionName.substr(0,15) == "a_magefloatshot" ||
			actionName.substr(0,18) == "a_twin_float_melee" ||
			actionName == "a_Sum_Rifle_Jump_Shot_01"
			|| "a_twin_tornado_spin" == actionName
			|| "a_twin_dragon_upper" == actionName
			|| "a_Air_Smash" == actionName
		    ) {
			//ODS("Act_Jump_OnCheckCanEnter - true . "..actionName.."\n", false, 3851)
		    return  true;
		}
        return  false;
    }
	return	true;
}
bool PgActionFSM_Act_Jump::OnEnter(lwActor actor,lwAction action) const
{
		// check prveious action
	// 아직까지는 actor의 Action이 이전 액션이다.
	// OnEnter에서 true를 되돌려 주면, 이전의 액션은 
	// 완전히 삭제된다.

//	if( action.GetParamFloat(123456) == 0 ) {
//		action.SetParamFloat(123456, timeGetTime())
//	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if( actor.IsMyActor() )
	{//동기화 맞지 않는 문제 일부 수정 : 이동 하기 전에 캐릭터 방향을 다시 받도록 수정
		NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
		if( pkKeyboard)
		{
			int const iDirKeySet[4] = {ACTIONKEY_LEFT, ACTIONKEY_RIGHT, ACTIONKEY_UP, ACTIONKEY_DOWN};
			BYTE const byDirSet[4] = {DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN};
			for( int iCount = 0; iCount<4; ++iCount )
			{
				int const iKeyNum = g_kGlobalOption.GetUKeyToKey(iDirKeySet[iCount]);
				NiInputKeyboard::KeyCode kKeycode = static_cast<NiInputKeyboard::KeyCode>(iKeyNum);
				bool const bKeyDown = pkKeyboard->KeyIsDown( kKeycode );
				if( bKeyDown )
				{
					g_kPilotMan.UpdateDirectionSlot(iDirKeySet[iCount], true);
				}
			}
		}
	}

	lwAction prevAction = actor.GetAction();
//	lwCheckNil(prevAction.IsNil());

	lwPilot kPilot = actor.GetPilot();
	if( true == kPilot.IsNil() )
	{
		return false;
	}
	
	if(	prevAction.IsNil() )
	{
		return	true;
	}
	
	std::string actionName = prevAction.GetID();

	//ODS("Act_Jump_OnEnter actionName."..actionName.."\n");
	
	float	fJumpForce = lua_tinker::get<float>(*lua_wrapper_user(g_kLuaTinker),"jumpForce");
	float	bIsFloatEvasion = (action.GetParamInt(7) == 1);
	
	if( strcmp(action.GetParam(4), "null") != 0 ) 
	{
		fJumpForce = PgStringUtil::SafeAtof(action.GetParam(4));
		if( fJumpForce == 0.0f)
		{
			fJumpForce = lua_tinker::get<float>(*lua_wrapper_user(g_kLuaTinker),"jumpForce");
		}
	}
	
	if( bIsFloatEvasion ) 
	{
		fJumpForce = 30;
	}
		
	action.SetParamFloat(4,fJumpForce);
	
	if( STR_HIJUMP == action.GetParam(5) )
	{
		//UseCameraHeightAdjust(false)
	}
	
	if( bIsFloatEvasion 
		|| ESS_CASTTIME == prevAction.GetActionParam()	// 캐스팅중 캔슬이면
		) 
	{
		actor.StartJump(fJumpForce);
		return	true;
	}

	if( actionName == ACTIONNAME_RUN ||
		actionName == "a_walk_left" ||
		actionName == "a_walk_right" ||
		actionName == "a_walk_up" ||
		actionName == "a_walk_down" ||
		actionName == "a_ladder_down" ||
		actionName == "a_ladder_idle" || 
		actionName == "a_ladder_up" ||
		actionName == "a_ladder_dash" ||
		actionName == "a_dash" || 
		actionName == "a_dash_jump" ||
		STR_HIJUMP == action.GetParam(5)
		) 
	{
		if( action.GetCurrentSlot() == 1 ) 
		{
			if( strcmp(prevAction.GetParam(2), "fall_up") != 0 ) 
			{
				SetComboAdvisor(actor,action);
				actor.StartJump(fJumpForce);
			}
			lwPoint3	pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
		}
		else if( action.GetCurrentSlot() == 2 ) 
		{
			if( actor.IsJumping() == false ) 
			{
				actor.StartJump(0);
			}
		}
		return true;
	}
	else if( actionName == ACTIONNAME_JUMP ) 
	{
		if( strcmp(action.GetParam(1), "jump_again") == 0 )
		{
			actor.StartJump(fJumpForce/2);
			action.SetSlot(4);
			return true;
		}
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false 
			) 
		{
			action.SetSlot(1);
			actor.StartJump(fJumpForce);
			SetComboAdvisor(actor,action);
			return true;
		}
		else
		{
			action.SetSlot(2);
			//ODS("Act_Jump_OnEnter Failed. actionname is a_jump\n");
			return false;
		}
	}
	else if( actionName == ACTIONNAME_IDLE ||
		actionName == ACTIONNAME_BIDLE ||
		actionName == "a_hang" || 
		actionName == "a_rope" ||
		actionName == "a_magician_charge_attack" ||
		actionName == "a_archer_jump_shot" ||
		actionName.substr(0,19) == "a_MagicianFloatShot" ||
		actionName.substr(0,13) == "a_float_melee" ||
		actionName.substr(0,16) == "a_nj_float_melee" ||
		actionName == "a_thief_float_melee_01" ||
		actionName == "a_down_attack_fly" ||
		actionName == "a_mag_down_attack_fly" ||
		actionName == "a_thi_down_attack_fly" ||
		actionName == "a_arc_down_attack_fly" ||
		actionName == "a_archer_shot_01" ||
		actionName == "a_melee_drop" ||
		actionName == "a_melee_blow_up" ||
		actionName == "a_fig_floatdownblow1" ||
		actionName == "a_fig_floatdownblow2" ||
		actionName == "a_magician_down_shot" ||
		actionName == "a_ran_floatdownshot" ||
		actionName == "a_storm blade" ||
		actionName == "a_mega_stormblade" ||
		actionName == "a_AscensionDragon" ||
		actionName == "a_archer_down_shot" ||
		actionName == "a_Sky Pirce01" ||		
		actionName == "a_Screw Upper" ||
		actionName == "a_mage_down_shot" ||
		actionName ==  "a_MP-Zero Trap" ||
		actionName ==  "a_Splash Trap" ||
		actionName ==  "a_Freezing Trap" ||
		actionName ==  "a_Silence Trap" ||
		actionName ==  "a_tesla_coil" || 
		actionName ==  "a_Sum_Rifle_Jump_Shot_01" ||
		actionName.substr(0,15) == "a_magefloatshot" ||
		actionName.substr(0,18) == "a_twin_float_melee"
		|| "a_twin_tornado_spin" == actionName
		|| "a_twin_dragon_upper" == actionName
		|| "a_Air_Smash" == actionName
		|| "a_domination" == actionName
		)
	{
		if( actor.IsMeetFloor() == false && 
			action.GetCurrentSlot() != 2 
			) 
		{
			//ODS("Act_Jump_OnEnter Failed action.GetCurrentSlot() != 2 / actionName . "..actionName.."\n", false, 3851);
			return	false;
		}
		//ODS("Act_Jump_OnEnter - true . "..actionName.."\n", false, 3851)

		return true;
	}
	else if( kPilot.IsHaveComboAction(prevAction.GetActionNo()) )
	{
		action.SetSlot(1);
		actor.StartJump(fJumpForce);
		return true;
	}

	//ODS("Act_Jump_OnEnter Failed actionName . "..actionName.."\n", false, 3851);
	return false;
}
void PgActionFSM_Act_Jump::OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const
{
	lwCheckNil(action.IsNil());
	lwCheckNil(kPacket.IsNil());

	kPacket.PushFloat(action.GetParamFloat(4));
	kPacket.PushInt(action.GetParamInt(7));	//	Is this Float Evasion ?
}
bool PgActionFSM_Act_Jump::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime) const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int curAnimSlot = action.GetCurrentSlot();
	float movingSpeed = 0.0f;
	float	fJumpForce = action.GetParamFloat(4);
		
	if( lwIsSingleMode() == true ) 
	{
		movingSpeed = 150;
	}
	else
	{
		movingSpeed = static_cast<float>( actor.GetAbil(AT_C_MOVESPEED) );
	}

	if( NULL != g_pkWorld ) 
	{
		if( g_pkWorld->GetAttr() == GATTR_VILLAGE ) 
		{
			movingSpeed = movingSpeed + actor.GetAbil(AT_C_VILLAGE_MOVESPEED);
		}
	}

	DoAutoFire(actor);
	
	lwPoint3 vel = actor.GetVelocity(); 	// Current Velocity
	float z = vel.GetZ(); 				// Gravity

	//std::string param = action.GetParam(0);

	// Gravity값이 깔끔하게 0이 나오지 않는 것에 주의.
	bool IsAnimDone = actor.IsAnimationDone();

	std::string nextAction = ACTIONNAME_IDLE;
	int dir = actor.GetDirection();

	////ODS("Current Slot = " .. curAnimSlot .. "\n")

	// 사다리 체크
	if( actor.ContainsDirection(DIR_UP) && 
		actor.IsMyActor() &&
		actor.ClimbUpLadder() 
		)
	{
		action.SetNextActionName("a_ladder_idle");
		return false;
	}

	//if( actor.IsMyActor() ) 
	//{
	//	ODS("[Jump] Current Dir = " .. dir .. "\n");
	//}

	if( dir != DIR_NONE ) 
	{
		actor.Walk(dir, movingSpeed, 0.0f, false);
		nextAction = ACTIONNAME_RUN;
	}
	else if( actor.GetWalkingToTarget() == true ) 
	{
		actor.Walk(dir, movingSpeed, 0.0f, false);
//	else
//		WriteToConsole("________공중에서 움직인 시간 . " .. timeGetTime() - action.GetParamFloat(123456) .. "AccumHeight . " .. actor.GetJumpAccumHeight() .. " JumpTime . " .. actor.GetJumpTime() .. " Moving Speed . " .. movingSpeed .. "\n")
	}

	if( curAnimSlot == 0 ) 
	{
		if( IsAnimDone == true ) 
		{
			actor.StartJump(fJumpForce);
            SetComboAdvisor(actor,action);
			
			lwPoint3 pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
			actor.PlayNext();
			return true;
		}
		else if( dir != DIR_NONE ) 
		{
			actor.PlayNext();
			actor.StartJump(fJumpForce);
            SetComboAdvisor(actor,action);
			
			lwPoint3	pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
		}
		else if( z > g_fLandingThreshold ) 
		{
			actor.PlayNext();
		}
	}	
	else if( curAnimSlot == 1 )
	{
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false &&
			actor.IsJumping() == false 
			)
		{
			if( dir == DIR_NONE ) 
			{
				actor.Stop();
				action.SetSlot(3);
				actor.PlayCurrentSlot(false);
			}
			else
			{
				// Jump가 3번 슬롯이어야 run으로 Enter가능
				// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
				if( nextAction != ACTIONNAME_IDLE ) 
				{
					action.SetSlot(3);
					action.SetNextActionName(nextAction.c_str());
					return false;
				}
				else if( strcmp(action.GetParam(1), "jump_again") != 0 )
				{
					action.SetNextActionName(ACTIONNAME_JUMP);
					return false;
				}
			}
		}
		else if( IsAnimDone == true ) 
		{
			actor.PlayNext();
		}
	}
	else if( curAnimSlot == 2 ) 
	{
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false ) 
		{
			if( dir == DIR_NONE ) 
			{
				actor.Stop();
				actor.PlayNext();
			}
			else
			{
				// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
				if( nextAction != ACTIONNAME_IDLE ) 
				{
					action.SetSlot(3);
					action.SetNextActionName(nextAction.c_str());
					return false;
				}
				else if( strcmp(action.GetParam(1), "jump_again") != 0 ) 
				{
					// param이 run이 아니고,
					// 이단 점프를 한 것이 아니면, 이단 점프를 한다.
					action.SetNextActionName(ACTIONNAME_JUMP);
					return false;
				}
			}
		}
	}
	else if( curAnimSlot == 3 ) 
	{ 
		if( IsAnimDone == true ) 
		{
			return false;
		}
		else
		{
			if( nextAction == ACTIONNAME_JUMP ) 
			{
				action.SetNextActionName(nextAction.c_str());
				return false;
			}
		}
	}
	else if( curAnimSlot == 4 ) 
	{ 
		if( actor.IsMeetFloor() == true ) 
		{
			// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
			if( nextAction != ACTIONNAME_IDLE ) 
			{
				action.SetSlot(3);
				action.SetNextActionName(nextAction.c_str());
				return false;
			}
			else
			{
				action.SetSlot(3);
				actor.PlayCurrentSlot(false);
			}
		}
		else if( IsAnimDone == true ) 
		{
			action.SetSlot(2);
			actor.PlayCurrentSlot(false);
		}
	}

	// 매달리기 처리
	//if( action.GetParam(10) != "just_hang" &&
		//(curAnimSlot == 1 || curAnimSlot == 2 || curAnimSlot == 4) &&
		//actor.IsMeetFloor() == false ) {
		//hang = actor.GetActionState("a_hang")
		//if( hang != 0 && 
			//actor.HangItOn() == true ) {
			//action.SetParam(5, "Done")
			//action.SetNextActionName("a_hang")
			//return false
		//}
	//}

	return true;
}
bool PgActionFSM_Act_Jump::OnCleanUp(lwActor actor,lwAction action) const
{
	lwCheckNil(actor.IsNil());
	lwAction curAction = actor.GetAction();
	lwCheckNil(curAction.IsNil());
	
	if( curAction.IsNil() == false && STR_HIJUMP == curAction.GetParam(5) ) 
	{
		lwUseCameraHeightAdjust(true);
	}
	return true;
}
bool PgActionFSM_Act_Jump::OnLeave(lwActor actor,lwAction action,bool bCancel) const
{
	lwCheckNil(actor.IsNil());

	lwAction curAction = actor.GetAction();
	lwCheckNil(curAction.IsNil());

	std::string nextActionName = action.GetID();
	lwCheckNil(action.IsNil());
	
	//ODS("Act_Jump_OnLeave nextActionName."..nextActionName.."\n");

	// 새로운 액션이 들어왔을 때
	if( action.GetEnable() == true ) 
	{
		if( nextActionName == ACTIONNAME_JUMP && 
			strcmp(curAction.GetParam(1), "jump_again") != 0 ) 
		{
			if( actor.GetAbil(AT_DOUBLE_JUMP_USE) == 1 ) 
			{
				if( curAction.GetCurrentSlot() == 1 || 
					curAction.GetCurrentSlot() == 2 ) 
				{
					if( 0 == PgStringUtil::SafeStrcmp("TRUE", actor.GetParam("DOUBLE_JUMP")) ) 
					{
						action.SetParam(1, "jump_again");
					}
				}
				return true;
			}
		}
		else if( nextActionName == ACTIONNAME_RUN ||
			actor.IsMeetFloor() && 
			actor.IsMeetSide() == false 
			)
		{
			//action.SetDoNotBroadCast(true)
			return true;
		}
		else if( nextActionName == "a_ladder_down" || 
			nextActionName == "a_ladder_idle" ||
			nextActionName == "a_ladder_up" 
			)
		{
			actor.HideParts(6, true);
			actor.SetParam("LADDER_WEAPON_HIDE","TRUE");
			return true;
		}
		else if( nextActionName == ACTIONNAME_IDLE )
		{
			curAction.SetParam(0, "null");
			actor.DetachFrom(2);
		}
		else if( nextActionName == "a_hang" &&
			strcmp(curAction.GetParam(5), "Done") != 0
			)
		{
			return false ;
		}
		return true;
	}
	return false;
}

void PgActionFSM_Act_Jump::CheckBreakFall(lwActor actor) const
{
	lwCheckNil(actor.IsNil());
	if(!g_pkWorld)
	{
		return;
	}

	if(actor.IsMyActor())	//	낙법 처리
	{
		lwAction action = actor.GetAction();
		lwCheckNil(action.IsNil());

		if(action.IsNil() == false 
			&& action.CanBreakFall()
			)
		{
			float fLastPressTime = action.GetParamFloat(6);
			if(0 ==  fLastPressTime)
			{
				fLastPressTime = g_pkWorld->GetAccumTime();
				action.SetParamFloat(6,fLastPressTime);
			}
		}
	
		if( actor.IsDownState())
		{
			float fDownTime = actor.GetTotalDownTime();
			fDownTime = fDownTime - 0.5f;
			if( fDownTime < 0)
			{
				fDownTime = 0;
			}
			actor.SetTotalDownTime(fDownTime);
		}
	}
}

bool PgActionFSM_Act_Jump::IsFloatEvasion(lwActor actor, lwAction action) const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if(!g_pkWorld)
	{
		return false;
	}

	if( action.GetParamInt(7) == 1 ) 
	{
		return	true;
	}
	
	if( actor.IsMyActor() == false ) 
	{
		return	false;
	}
	
	if( actor.IsMeetFloor() )
	{
		return	false;
	}

	lwAction	kCurAction = actor.GetAction();
	if( kCurAction.IsNil() ) 
	{
		return	false;
	}
	
	lwInputSlotInfo kInputSlotInfo = action.GetInputSlotInfo();
	if( kInputSlotInfo.IsNil() ) 
	{
		//ODS("Act_Jump_IsFloatEvasion kInputSlotInfo.IsNil()\n");
		return	false;
	}
	
	if( kInputSlotInfo.GetUKey() != 3023 ) 
	{
		//ODS("Act_Jump_IsFloatEvasion kInputSlotInfo.GetUKey() ~= 3023\n");
		return	false;
	}
	
	if( 0 == PgStringUtil::SafeStrcmp(actor.GetParam("FLOAT_EVASION"), "FALSE") ) 
	{
		return	false;
	}

	if( actor.IsBlowUp() == false ) 
	{
		//ODS("Act_Jump_IsFloatEvasion actor.IsBlowUp() == false\n");
		return	false;
	}
	
	float fActionStartTime = kCurAction.GetActionEnterTime();
	float fCurrentTime = g_pkWorld->GetAccumTime();
	
	if( (fCurrentTime - fActionStartTime) < g_fEvasionStartTime ) 
	{
		return	false;
	}
	
	action.SetParamInt(7,1);
	actor.SetParam("FLOAT_EVASION","FALSE");
	actor.SetCanHit(false);
	
	return	true;
}

void PgActionFSM_Act_Jump::SetComboAdvisor(lwActor actor, lwAction action) const
{
	lwCheckNil(actor.IsNil());

	if( actor.IsMyActor() ) 
	{
		lwCheckNil(action.IsNil());

		lwGetComboAdvisor().OnNewActionEnter(action.GetID());

		lwPilot kPilot = actor.GetPilot();
		lwCheckNil(kPilot.IsNil());

		int iBaseClassID = kPilot.GetBaseClassID();

		if( iBaseClassID == UCLASS_FIGHTER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_melee_drop");
			lwGetComboAdvisor().AddNextAction("a_float_melee_01");
		}
		else if( iBaseClassID == UCLASS_THIEF ) 
		{
			lwGetComboAdvisor().AddNextAction("a_thi_melee_drop");
			lwGetComboAdvisor().AddNextAction("a_thief_float_melee_01");
		}
		else if( iBaseClassID == UCLASS_MAGICIAN ) 
		{
			lwGetComboAdvisor().AddNextAction("a_magician_down_shot");
			lwGetComboAdvisor().AddNextAction("a_MagicianFloatShot_01");
		}
		else if( iBaseClassID == UCLASS_ARCHER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_archer_down_shot");
			lwGetComboAdvisor().AddNextAction("a_MagicianFloatShot_01");
		}
		else if( iBaseClassID == UCLASS_DOUBLE_FIGHTER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_twin_float_melee_01");
			lwGetComboAdvisor().AddNextAction("a_twin_float_melee_01");
		}
	}
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Skill
bool	PgActionFSM_Skill_Blitz_Play::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	if(!g_pkWorld)
	{
		return false;
	}
	
	//	시작 시간 기록
	action.SetParamFloat(0,g_pkWorld->GetAccumTime());
	
	//	State
	action.SetParamInt(4,0);
	
	//	Hit Count
	action.SetParamInt(5,0);
	
	//	Last Hit Time
	action.SetParamFloat(6,0);

	//  boom Time
	action.SetParamFloat(7,g_pkWorld->GetAccumTime() + 0.3f);
	//  boom Count
	action.SetParamInt(8,0);
	
	actor.SetMovingDelta(lwPoint3(0,0,0));
	actor.StopJump();
	actor.FreeMove(true);

	float	fStartHeight = 150.0f;
	
	lwPoint3 kPos = actor.GetPos();
	kPos.SetZ(kPos.GetZ()+fStartHeight);
	actor.AttachParticleToPoint(1,kPos,"ef_bang");
	actor.SetTranslate(kPos, false);

	action.SetParamFloat(33,kPos.GetX()); // 처음 시작 위치
	action.SetParamFloat(34,kPos.GetY());
	action.SetParamFloat(35,kPos.GetZ());		

	actor.SetTargetScale(0.9f, 10);
	
	//lwCommonSkillUtilFunc::InitMaxHitCnt(action);
	lwCommonSkillUtilFunc::SetMaxHitCnt(action, 3);
	return true;
}
bool	PgActionFSM_Skill_Blitz_Play::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	bool animDone = actor.IsAnimationDone();
	int iSlotNum = action.GetCurrentSlot();
	float fMoveSpeed  = 600.0f;
	float fMaxMoveTime = 0.4f;
	int	iState = action.GetParamInt(4);

	if (iSlotNum == 0 ) 
	{
		actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
		action.SetParamFloat(0,fAccumTime);	//	이동 시작 시간 기록

		lwPoint3 kPos = actor.GetPos();
		// 현재 위치 기록
		action.SetParamFloat(1,kPos.GetX());
		action.SetParamFloat(2,kPos.GetY());
		action.SetParamFloat(3,kPos.GetZ());

		actor.PlayNext();

	}
	else if (iSlotNum == 1 ) 
	{	
		float	fElapsedTime = fAccumTime - action.GetParamFloat(0);
		float	fMoveDistance = fElapsedTime * fMoveSpeed;
		float	fRate = fElapsedTime/fMaxMoveTime;
		if (fRate>1 ) 
		{
			fRate = 1;
		}

		float	fMoveDistance2 = fMoveSpeed * fRate;

		lwPoint3 kStartPos(action.GetParamFloat(33),action.GetParamFloat(34),action.GetParamFloat(35));
		lwPoint3 kTargetPos(action.GetParamFloat(30),action.GetParamFloat(31),action.GetParamFloat(32));
		lwPoint3 kLookDir = kTargetPos.Subtract2(kStartPos);
		kLookDir.Unitize();
		kLookDir.Multiply(fMoveDistance);

		kStartPos.Add(kLookDir);
		actor.SetTranslate(kStartPos, false);

		if( fRate == 1.0f ) 
		{
			actor.EndBodyTrail();
			actor.PlayNext();
			action.SetParamInt(4,1);//	Start Hit
			OldActionLuaFunc::HitOneTime(actor,action);
			return true;
		}
	}

	if (iState == 1)
	{
		float	fElapsedTime = fAccumTime - action.GetParamFloat(6);
		if (fElapsedTime >= 0.15 ) 
		{
			OldActionLuaFunc::HitOneTime(actor,action);
		}
	}
	else if (iState == 2)
	{

		if ( animDone && fAccumTime - action.GetParamFloat(6)>0.5 )
		{
			action.SetParamFloat(6, g_pkWorld->GetAccumTime());
			action.SetParamInt(4,3);
			actor.SetTargetAlpha(actor.GetAlpha(), 0.0f, 0.7f, false);
		}
	}
	else if ( iState == 3 ) 
	{
		if ( fAccumTime - action.GetParamFloat(6) > 1.2 )
		{
			return false;
		}
	}

	float fBoomElapsedTime = fAccumTime - action.GetParamFloat(7);
	if ( fBoomElapsedTime >= 0.05 && action.GetParamInt(8) < 3 ) 
	{
		lwPoint3 ptcl = g_pkWorld->ThrowRay(actor.GetPos()(), NiPoint3(0,0,-1), 100);
		if ( ptcl.GetX() == -1 && ptcl.GetY() == -1 && ptcl.GetZ() == -1 ) 
		{
			return true;
		}
		ptcl.SetZ(ptcl.GetZ()+5);
		actor.AttachParticleToPoint(100 + action.GetParamInt(8), ptcl, "ef_boom_01");
		action.SetParamFloat(7,g_pkWorld->GetAccumTime());
		action.SetParamInt(8,action.GetParamInt(8) + 1);
		g_pkWorld->AttachSound("Missile", actor.GetPos()(), 0,0,0);
	}

	return true;
 }

bool	PgActionFSM_Skill_Blitz_Play::OnCleanUp(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return true;
	}
	lwCheckNil(actor.IsNil());
		
	actor.EndBodyTrail();
	g_pkWorld->RemoveObjectOnNextUpdate(actor.GetPilotGuid()());
	return true;
	
}
bool	PgActionFSM_Skill_Blitz_Play::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	return true;
}
bool	PgActionFSM_Skill_Blitz_Play::OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const
{
	return	true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool PgActionFSM_Skill_Wolf_Rush_Play::OnEnter(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
	std::string actorID = actor.GetID();
	std::string actionID = action.GetID();
	
	//	시작 시간 기록
	action.SetParamFloat(0,g_pkWorld->GetAccumTime());
	
	//	State
	action.SetParamInt(4,0);
	
	//	Hit Count
	action.SetParamInt(5,0);
	
	//	Last Hit Time
	action.SetParamFloat(6,0);
	
	lwPoint3 kPos = actor.GetPos();
	kPos.SetZ(kPos.GetZ()-30);
	actor.AttachParticleToPoint(1,kPos,"ef_bang");
	
	actor.SetMovingDelta(lwPoint3(0,0,0));
	actor.StopJump();
	actor.FreeMove(true);
	actor.SetTargetScale(1.3f, 180);
	
	//lwCommonSkillUtilFunc::InitMaxHitCnt(action);
	lwCommonSkillUtilFunc::SetMaxHitCnt(action, 5);
	return true;
}

bool PgActionFSM_Skill_Wolf_Rush_Play::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
	bool animDone = actor.IsAnimationDone();
//	local actionID = action.GetID();
	int iSlotNum = action.GetCurrentSlot();
	float fMoveSpeed  = 700;
	int fMaxMoveDistance = action.GetParamInt(10)+10;
	int	iState = action.GetParamInt(4);
	
	if( iSlotNum == 0 ) 
	{
		float fElapsedTime = fAccumTime - action.GetParamFloat(0);
		if( fElapsedTime>=0.3f ) 
		{
			actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
			action.SetParamFloat(0,fAccumTime);	//	이동 시작 시간 기록
			
			lwPoint3	kPos = actor.GetPos();
			// 현재 위치 기록
			action.SetParamFloat(1,kPos.GetX());
			action.SetParamFloat(2,kPos.GetY());
			action.SetParamFloat(3,kPos.GetZ());
			actor.PlayNext();
		}
	}
	else if( iSlotNum == 1 )
	{
		float	fElapsedTime = fAccumTime - action.GetParamFloat(0);
		float	fMoveDistance = fElapsedTime * fMoveSpeed;
		
		if( fMoveDistance>=fMaxMoveDistance ) 
		{
			actor.EndBodyTrail();
			actor.PlayNext();
		}
		else
		{
			lwPoint3 kMovingDir = lwPoint3(action.GetParamFloat(11),action.GetParamFloat(12),action.GetParamFloat(13));
			kMovingDir.Multiply(fMoveDistance);
			lwPoint3 kNextPos = lwPoint3(action.GetParamFloat(1),action.GetParamFloat(2),action.GetParamFloat(3));
			kNextPos.Add(kMovingDir);
			actor.SetTranslate(kNextPos, false);
		}
	}
	
	if( iState == 1 )
	{
		float	fElapsedTime = fAccumTime - action.GetParamFloat(6);
		if( fElapsedTime >= 0.15f ) 
		{
			OldActionLuaFunc::HitOneTime(actor,action);
		}
	}
	else if( iState == 2 ) 
	{
		if( animDone && fAccumTime - action.GetParamFloat(6)>0.5f ) 
		{
			action.SetParamFloat(6,g_pkWorld->GetAccumTime());
			action.SetParamInt(4,3);
			actor.SetTargetAlpha(actor.GetAlpha(),0.0f, 0.8f, false);
		}
	}
	else if( iState == 3 )
	{
		if( fAccumTime - action.GetParamFloat(6) > 1.2f )
		{
			return false;
		}
	}	

	return true;
}
bool PgActionFSM_Skill_Wolf_Rush_Play::OnCleanUp(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
	actor.EndBodyTrail();
	g_pkWorld->RemoveObjectOnNextUpdate(actor.GetPilotGuid()());
	return true;
}
bool PgActionFSM_Skill_Wolf_Rush_Play::OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const
{
	lwCheckNil(actor.IsNil());
		
	lwAction kAction = actor.GetAction();
	lwCheckNil(kAction.IsNil());
	

	if( kAction.GetActionParam() == ESS_CASTTIME ) 
	{
		return true;
	}
	
	if( kAction.GetParamInt(4) == 0 
		&& (kTextKey == "hit" || kTextKey == "end")
		)
	{
		kAction.SetParamInt(4,1); //	Start Hit
		OldActionLuaFunc::HitOneTime(actor,kAction);
	}
	return true;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool PgActionFSM_Skill_Falcon_Beat_Play::OnEnter(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	//	시작 시간 기록
	action.SetParamFloat(0,g_pkWorld->GetAccumTime());
	
	//	State
	action.SetParamInt(4,0);
	
	//	Hit Count
	action.SetParamInt(5,0);
	lwCommonSkillUtilFunc::SetMaxHitCnt(action, 3);

	//	Last Hit Time
	action.SetParamFloat(6,0);

	//  boom Time
	action.SetParamFloat(7, g_pkWorld->GetAccumTime() + 0.48f);
	//  boom Count
	action.SetParamInt(8,0);
	
	float	fStartHeight = 150;
	
	lwPoint3 kPos = actor.GetPos();
	kPos.SetZ(kPos.GetZ()+fStartHeight);
	actor.AttachParticleToPoint(1,kPos,"ef_bang");
	
	actor.SetMovingDelta(lwPoint3(0,0,0));
	actor.StopJump();
	actor.FreeMove(true);

	//	베지어 곡선을 그리기 위한 4개의 포인트를 찾아보자.
	lwPoint3 kMovingDir(action.GetParamFloat(11),action.GetParamFloat(12),action.GetParamFloat(13));		
	kPos = actor.GetPos();
	
	kPos.SetZ(kPos.GetZ()+fStartHeight);	
	
	//	첫번째좌표는 캐릭터의 현재 좌표
	action.SetParamFloat(14,kPos.GetX());
	action.SetParamFloat(15,kPos.GetY());
	action.SetParamFloat(16,kPos.GetZ());
	
	//	마지막 좌표는 캐릭터로부터 사정거리까지의 좌표
	float fMaxMoveDistance = static_cast<float>( action.GetParamInt(10)+180 );
	
	lwPoint3	kTargetPos = kMovingDir.Multiply2(fMaxMoveDistance);
	kTargetPos.Add(actor.GetPos());
	kTargetPos.SetZ(kTargetPos.GetZ() + 5);
	action.SetParamFloat(23,kTargetPos.GetX());
	action.SetParamFloat(24,kTargetPos.GetY());
	action.SetParamFloat(25,kTargetPos.GetZ());	
	
	// 두번째 좌표
	lwPoint3	kSecPos(kPos.GetX(),kPos.GetY(),kPos.GetZ() + 20);	
	kSecPos.SetZ(kSecPos.GetZ()+(kTargetPos.GetZ()-kSecPos.GetZ())*0.9f);
	action.SetParamFloat(17,kSecPos.GetX());
	action.SetParamFloat(18,kSecPos.GetY());
	action.SetParamFloat(19,kSecPos.GetZ());
		
	// 세번째 좌표
	lwPoint3	kThirdPos = kMovingDir.Multiply2(fMaxMoveDistance*0.2f);
	kThirdPos.Add(kSecPos);
	action.SetParamFloat(20,kThirdPos.GetX());
	action.SetParamFloat(21,kThirdPos.GetY());
	action.SetParamFloat(22,kThirdPos.GetZ());

	actor.SetTargetScale(0.9f, 10);
	return true;
}
bool PgActionFSM_Skill_Falcon_Beat_Play::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	if( NULL == g_pkWorld )
	{
		return false;	
	}
	
	bool animDone = actor.IsAnimationDone();
//	local actionID = action.GetID();
	int iSlotNum = action.GetCurrentSlot();
	float fMoveSpeed  = 600;
	float fMaxMoveTime = 0.8f;
	int	iState = action.GetParamInt(4);
	
	if( iSlotNum == 0 )
	{
		actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
		
		action.SetParamFloat(0,fAccumTime);	//	이동 시작 시간 기록
		
		lwPoint3	kPos = actor.GetPos();
		// 현재 위치 기록
		action.SetParamFloat(1,kPos.GetX());
		action.SetParamFloat(2,kPos.GetY());
		action.SetParamFloat(3,kPos.GetZ());
	
		actor.PlayNext();
	}
	
	if( iSlotNum == 1 ) 
	{
		float	fElapsedTime = fAccumTime - action.GetParamFloat(0);
		float	fMoveDistance = fElapsedTime * fMoveSpeed;
		float	fRate = fElapsedTime/fMaxMoveTime;
		if( fRate>1 ) 
		{
			fRate = 1;
		}
		lwPoint3 kMovingDir(action.GetParamFloat(11),action.GetParamFloat(12),action.GetParamFloat(13));
		
		lwPoint3	kP1(action.GetParamFloat(14),action.GetParamFloat(15),action.GetParamFloat(16));
		lwPoint3	kP2(action.GetParamFloat(17),action.GetParamFloat(18),action.GetParamFloat(19));
		lwPoint3	kP3(action.GetParamFloat(20),action.GetParamFloat(21),action.GetParamFloat(22));
		lwPoint3	kP4(action.GetParamFloat(23),action.GetParamFloat(24),action.GetParamFloat(25));
		
		NiPoint3 kTempPos;
		Bezier4(kP1(),kP2(),kP3(),kP4(),fRate, kTempPos);
		lwPoint3 kNextPos(kTempPos); 
		actor.SetTranslate(kNextPos, false);
		
		actor.LookAt(kNextPos,true,false, false);
	
		if( fRate == 1 ) 
		{
			actor.EndBodyTrail();
			actor.PlayNext();
			
			action.SetParamInt(4,1);	//	Start Hit
			OldActionLuaFunc::HitOneTime(actor,action);
			return true;
		}
	}
	
	if( iState == 1 ) 
	{
		float	fElapsedTime = fAccumTime - action.GetParamFloat(6);
		if( fElapsedTime >= 0.15 ) 
		{
			OldActionLuaFunc::HitOneTime(actor,action);
		}
	}
	else if( iState == 2 ) 
	{
		if( animDone && fAccumTime - action.GetParamFloat(6)>0.5 ) {
			action.SetParamFloat(6,g_pkWorld->GetAccumTime());
			action.SetParamInt(4,3);
			actor.SetTargetAlpha(actor.GetAlpha(),0.0f, 0.7f, false);
		}
	}
	else if( iState == 3 ) 
	{
		if( fAccumTime - action.GetParamFloat(6) > 1.2 ) 
		{
			return false;
		}
	}

	float fBoomElapsedTime = fAccumTime - action.GetParamFloat(7);
	if( fBoomElapsedTime >= 0.05 && action.GetParamInt(8) < 5 )
	{
		lwPoint3 ptcl( g_pkWorld->ThrowRay(actor.GetPos()(), NiPoint3(0,0,-1), 100) );
		if( ptcl.GetX() == -1 && ptcl.GetY() == -1 && ptcl.GetZ() == -1 ) 
		{
			return true;
		}
		ptcl.SetZ(ptcl.GetZ()+5);
		actor.AttachParticleToPoint(100 + action.GetParamInt(8), ptcl, "ef_boom_01");
		action.SetParamFloat(7,g_pkWorld->GetAccumTime());
		action.SetParamInt(8,action.GetParamInt(8) + 1);
		g_pkWorld->AttachSound("Missile", actor.GetPos()(), 0.0f,0.0f,0.0f);
	}

	return true;
}
bool PgActionFSM_Skill_Falcon_Beat_Play::OnCleanUp(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}
	lwCheckNil(actor.IsNil());		
	actor.EndBodyTrail();
	g_pkWorld->RemoveObjectOnNextUpdate(actor.GetPilotGuid()());
	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool PgActionFSM_Skill_Bro_Spin_Fire::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if( NULL == g_pkWorld )
	{
		return false;	
	}

	if( actor.IsUnderMyControl() ) 
	{
		action.StartTimer(1.5f,0.3f,0);
	}

	action.SetParamInt(10,0); // count
	int iMaxHit = action.GetAbil(AT_COUNT);
	if( 0 == iMaxHit ) 
	{
		iMaxHit = 4;
	}
	action.SetParamInt(11, iMaxHit); // MaxCount

	action.SetDoNotBroadCast(true);

	//	시작 시간 기록
	action.SetParamFloat(0,g_pkWorld->GetAccumTime());
	action.SetParamFloat(1,g_pkWorld->GetAccumTime());

	action.SetParamFloat(13,actor.GetPos().GetX());
	action.SetParamFloat(14,actor.GetPos().GetY());
	action.SetParamFloat(15,actor.GetPos().GetZ());

	//action.SetSlot(action.GetCurrentSlot()+1);
	actor.PlayCurrentSlot(false);
	action.SetParamInt(9,0);

	actor.StopJump();
	actor.FreeMove(true);

	lwPoint3 pt = actor.GetNodeTranslate("char_root");
	actor.AttachParticleToPoint(581,pt,"ef_Bro_Spin_char_root");

	std::string	kSoundID = action.GetScriptParam("FIRE_SOUND_ID");
	if( kSoundID.empty() )
	{
		actor.AttachSound(2783,kSoundID.c_str(), 0);
	}
	return true;
}
bool PgActionFSM_Skill_Bro_Spin_Fire::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	//std::string actorID = actor.GetID();
	int iActionState = action.GetParamInt(9);
	//공중으로 올라가게 
	actor.SetMovingDelta(lwPoint3(0.0f,0.0f,12.0f));
	//회전
	float DegToRadUnit = 3.141592f/180.0f;
	float SpinScalar = 4.0f*360.0f;

	float fElapsedTime = accumTime - action.GetParamFloat(0);

	actor.IncRotate(frameTime*SpinScalar*DegToRadUnit);

	if( fElapsedTime> 1.0f ) 
	{
		if( iActionState == 0 ) 
		{
			actor.SetTargetAlpha(actor.GetAlpha(),0.0f, 0.7f, false);
			action.SetParamInt(9,1);
			actor.DetachFrom(581);
		}
	}
	return true;
}

bool PgActionFSM_Skill_Bro_Spin_Fire::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	actor.DetachFrom(581);
	return true;
}
bool PgActionFSM_Skill_Bro_Spin_Fire::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	if( NULL == g_pkWorld )
	{
		return false;	
	}
	g_pkWorld->RemoveObjectOnNextUpdate( actor.GetPilotGuid()() );
	return true;
}

bool PgActionFSM_Skill_Bro_Spin_Fire::OnTimer(lwActor actor,lwAction action,float fCallTime,int iTimerID)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int iHitCount = action.GetParamInt(10);
	int iTotalHit = action.GetParamInt(11);

	if( iHitCount == iTotalHit ) 
	{
		return false;
	}

	action.CreateActionTargetList(actor, false);

	if( lwIsSingleMode() ) 
	{
		OnTargetListModified(actor,action,false);
		return true;
	}
	else
	{
		lwActionTargetList kTargetList = action.GetTargetList();
		int iTargetCount = kTargetList.size();
		if( 0 < iTargetCount ) 
		{
			action.BroadCastTargetListModify(actor.GetPilot(), false);
			action.ClearTargetList();
		}
	}

	iHitCount = iHitCount + 1;
	action.SetParamInt(10,iHitCount);

	if( iHitCount == iTotalHit ) 
	{
		return	true;
	}

	return	true;
}
int PgActionFSM_Skill_Bro_Spin_Fire::OnFindTarget(lwActor actor,lwAction action,lwActionTargetList kTargetList)
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	lwPoint3 kPos(action.GetParamFloat(13),action.GetParamFloat(14),action.GetParamFloat(15) - 30);
	lwPoint3 kDir = actor.GetLookingDir();

	lwFindTargetParam kParam;
	kParam.SetParam_1(kPos,lwPoint3(0.0f,0.0f,1.0f));
	kParam.SetParam_2(400.0f, static_cast<float>( action.GetSkillRange(0,actor)*2 ),0,0);
	kParam.SetParam_3(true,PgAction::FTO_BLOWUP + PgAction::FTO_NORMAL);

	int iFound = action.FindTargetsEx(action.GetActionNo(),PgAction::TAT_BAR,kParam,kTargetList,kTargetList);
	return iFound;
}

void PgActionFSM_Skill_Bro_Spin_Fire::OnTargetListModified(lwActor actor,lwAction action,bool bIsBefore)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if( bIsBefore == false ) 
	{
		lwCommonSkillUtilFunc::DefaultHitOneTime(actor,action, false);
		action.GetTargetList().ApplyActionEffects(false, false, false);
	}
}

///
bool	PgActionFSM_Act_IdleBear::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
	std::string actorID = actor.GetID();

	//펫 탑승 중이라면 탑승 동작으로 변경
	if(actor.IsRidingPet())
	{
		action.SetSlot(3);
		actor.PlayCurrentSlot(true);
		actor.ReserveTransitAction(ACTIONNAME_RP_IDLE, action.GetDirection());
		return true;
	}

	//	만약 stun 상태라면, stun 액션으로 전이시킨다.
	if(actor.IsStun())
	{
		actor.ReserveTransitAction("a_stun",DIR_NONE);
		return	false;
	}
	
	int const iSpecificIdleActionNo = actor.GetSpecificIdle();
	if(iSpecificIdleActionNo)
	{
		action.SetParamInt(100, 1);		// SpecificIdle
		switch(iSpecificIdleActionNo)
		{
		case ESIT_BOSS_MONSTER_IDLE:
			{
				actor.ResetAnimation();
				actor.ReserveTransitAction("a_SpecificIdle", DIR_NONE);
				return false;
			}break;
		case ESIT_NONE:
			{
			}break;
		default:
			{
				if(iSpecificIdleActionNo != action.GetActionNo())
				{
					actor.ReserveTransitActionByActionNo(actor.GetSpecificIdle(), actor.GetDirection());
					return false;
				}
			}break;
		}
	}
	if( actor.IsUnitType(UT_SUB_PLAYER) )
	{// 보조 캐릭터에 맞는 액션으로 바꿔주고(메인 캐릭터를 따라가는)
		actor.ReserveTransitAction("a_twin_sub_trace_ground", DIR_NONE);
		return false;
	}

	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() && actor.IsMyActor())
		{
			if (actor.IsUnitType(UT_PLAYER) || actor.IsUnitType(UT_PET))
			{
				BYTE	byDir = actor.GetDirection();
				if (byDir != DIR_NONE)
				{
					actor.ReserveTransitAction(ACTIONNAME_RUN,byDir);
					return false;
				}
			}
		}
	}


	if(std::string(action.GetID()) == ACTIONNAME_BIDLE && actor.IsUnitType(UT_PLAYER))
	{
		action.SetSlot(1);
	}

	actor.Stop();

	if(EMGRADE_UPGRADED >= actor.GetAbil(AT_GRADE) || actor()->IsUseBattleIdle())
	{
		if(actor.IsUnitType(UT_MONSTER) && actor.HasTarget())
		{
			action.SetSlot(2);
		}
	}

	if(actor.IsUnitType(UT_PLAYER))
	{
		action.SetParamFloat(2,15);
	}                                           
	else if(actor.IsUnitType(UT_MONSTER))
	{
		action.SetParamFloat(2,4);
	}
	else if(actor.IsUnitType(UT_PET))
	{
		action.SetParamFloat(2,8);
	}

	action.SetParamInt(4, 4 + BM::Rand_Index(4));
	action.SetParamInt(5, 0);
	action.SetParamInt(6, 0);
	action.SetParamFloat(13,-1);
	
	if(false==actor()->IdleEffectName().empty())
	{
		actor.AttachParticle(SLOTNO_IDLE_EFFECT, actor()->IdleEffectNode().c_str(), actor()->IdleEffectName().c_str());
	}
	lwCommonSkillUtilFunc::InitIsBanSubPlayerAction(actor, action);	// SC 스킬을 쓰면 안되는 스킬인가
	return	true;
}

bool	PgActionFSM_Act_IdleBear::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int	currentSlot = action.GetCurrentSlot();
	//std::string param = action.GetParam(0);
	int	iIdleType = action.GetParamInt(6);
	
	if (actor.IsCheckMeetFloor())
	{
		if(actor.IsMeetFloor() == false && actor.IsMyActor())
		{
			lwAction kAction = actor.ReserveTransitAction(ACTIONNAME_JUMP,0);
			kAction.SetSlot(2);
			kAction.SetDoNotBroadCast(true);
			return false;
		}
	}

	if (action.GetParamFloat(13) == -1)
	{
		action.SetParamFloat(13,accumTime);
	}

	//if (actor.IsMyActor() == false)
	//{
		//BYTE dir = actor.GetDirection();
		//if (dir != DIR_NONE)
		//{
		//	actor.ReserveTransitAction(ACTIONNAME_RUN, dir);
		//	return true;
		//}
	//}
	
	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() && actor.IsMyActor())
		{
			if (actor.IsUnitType(UT_PLAYER) || actor.IsUnitType(UT_PET))
			{
				BYTE	byDir = actor.GetDirection();
				if (byDir != DIR_NONE)
				{
					actor.ReserveTransitAction(ACTIONNAME_RUN,byDir);
					return true;
				}
			}
		}
	}

	DoAutoFire(actor);

	if (actor.IsCheckMeetFloor())
	{
		if (actor.IsMeetFloor() == false 
			&& actor.GetAbil(AT_MONSTER_TYPE) != 1 
			&& actor.IsUnitType(UT_OBJECT) == false)
		{
			if (strcmp(action.GetParam(119),"jump_trap") == 0 || actor.GetVelocity().GetZ() < 0 )
			{
				action.SetNextActionName(ACTIONNAME_JUMP);
				action.SetParam(3, "fall_down");
				return false;
			}
		}
	}

	if (actor.IsAnimationDone() == true)
	{
		actor.ResetAnimation();
	
		if (iIdleType == 0)
		{
		
			int iBaseIdleLoopNum = action.GetParamInt(4);
			int iBaseIdleLoopCurNum = action.GetParamInt(5);
			
			iBaseIdleLoopCurNum=iBaseIdleLoopCurNum+1;
			
			if (iBaseIdleLoopCurNum>= iBaseIdleLoopNum)
			{
				action.SetParamInt(6,1);
				actor.PlayCurrentSlot(false);
			}
			else
			{
				action.SetParamInt(5,iBaseIdleLoopCurNum);
				actor.PlayCurrentSlot(true);	
			}
		}
		else if( iIdleType == 1 )
		{
			
			action.SetParamInt(6, 0);
			action.SetParamInt(4, 4 + BM::Rand_Index(4));
			action.SetParamInt(5, 0);
			
			actor.PlayCurrentSlot(true);			
		}
	

		return true;
	}

	if (currentSlot == 1)
	{
		if (accumTime - action.GetParamFloat(13) > 3.0)
		{
			action.SetNextActionName(ACTIONNAME_IDLE);
			return false;
		}
	}
	else if(actor.GetPilot().GetAbil(AT_IDLEACTION_TYPE) != 101 && accumTime - action.GetParamFloat(13) > 3.0 )
	{
		action.SetParamFloat(13,accumTime);
	}
	
	int const iSpecificIdleActionNo = actor.GetSpecificIdle();
	switch(iSpecificIdleActionNo)
	{
	case ESIT_BOSS_MONSTER_IDLE:
		{
			actor.ResetAnimation();
			actor.ReserveTransitAction("a_SpecificIdle", DIR_NONE);
			return false;
		}break;
	case ESIT_NONE:
		{
			if(0 < action.GetParamInt(100))
			{
				actor.ReserveTransitAction(ACTIONNAME_IDLE, actor.GetDirection());
				return false;
			}
		}break;
	default:
		{
			if(iSpecificIdleActionNo != action.GetActionNo())
			{
				actor.ReserveTransitActionByActionNo(actor.GetSpecificIdle(), actor.GetDirection());
				return false;
			}
		}break;
	}

	return true;
}
bool	PgActionFSM_Act_IdleBear::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	lwCheckNil(actor.GetAction().IsNil());

	if (strcmp(action.GetID(),ACTIONNAME_JUMP)==0 && strcmp(actor.GetAction().GetParam(3),"fall_down") == 0 )
	{
		// 절벽에서 떨어질 때는, ActionPacket을 보내지 않는다.
		action.SetSlot(2);
		action.SetDoNotBroadCast(true);
	}

	if(false==actor()->IdleEffectName().empty())
	{
		actor.DetachFrom(SLOTNO_IDLE_EFFECT);
	}
	return true;
}
///
bool	PgActionFSM_Act_RunBear::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
//	if action.GetEnable() == false then
		// 음.. 일단 주석처리..
//		local up = actor.GetActionState("a_run_up")
//		local down = actor.GetActionState("a_run_down")
//		local right = actor.GetActionState("a_run_right")
//		local left = actor.GetActionState("a_run_left")		
	
//		if up ~=0 or down ~=0 or right ~=0 or left ~=0 then
//			return false
//		end
//	end
	
	lwAction curAction = actor.GetAction();
	if(false==curAction.IsNil())
	{
		std::string curActionID = curAction.GetID();
		if (curActionID == ACTIONNAME_RUN && actor.IsNowFollowing() == false)
		{
			_PgOutputDebugString("Current Action is \"a_run\" . transit failed!\n");
			return false ;
		}
	}

	return true;
}
bool	PgActionFSM_Act_RunBear::OnEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	lwAction prevAction = actor.GetAction();
	
	if (prevAction.IsNil() == false)
	{
		std::string prevActionID = prevAction.GetID();
		int prevSlot = prevAction.GetCurrentSlot();
		if (prevActionID.substr(0,6) == ACTIONNAME_JUMP)
		{
			// 현재 액션이 점프이고 슬롯이 3번이 아니거나,
			// 바닥이 아니면, 전이 불가능
			if (prevSlot != 3 || actor.IsMeetFloor() == false)
			{
				action.SetDoNotBroadCast(true);
				return false;
			}
		}
		else if (prevActionID.substr(0,5) == ACTIONNAME_RUN)
		{
			return false;
		}
		// 그 외에는 전이 가능
	}

	actor.UseSkipUpdateWhenNotVisible(false);
	action.SetParamInt(1,0);

	lwPoint3	kTargetPos = action.GetParamAsPoint(0);
	if (kTargetPos.IsZero() == false )
	{
		action.SetParamInt(3,1);
		
		lwPoint3	kMoveDirection = kTargetPos.Subtract2(actor.GetPos());
		kMoveDirection.Unitize();
		action.SetParamAsPoint(1,kMoveDirection);
		action.SetParamAsPoint(2,actor.GetPos());
		
		actor.SetMovingDir(kMoveDirection);
		actor.LookAt(kTargetPos,true,true,false);
		actor.ConcilDirection(kMoveDirection, true);
	}

	if (action.GetParamInt(5) == 1)
	{
		actor.BackMoving(true);
	}

	//시작 위치와 시작 시간을 기록
	action.SetParamAsPoint(7, actor.GetPos());
	action.SetParamFloat(8, static_cast<float>(g_kEventView.GetServerElapsedTime()));
	lwCommonSkillUtilFunc::InitIsBanSubPlayerAction(actor, action);	// SC 스킬을 쓰면 안되는 스킬인가
	return true;
}
bool	PgActionFSM_Act_RunBear::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string actorID = actor.GetID();
	float movingSpeed = 0.0f;
	bool	bMoveToPos = (action.GetParamInt(3) == 1);
	float	fCustomSpeed = static_cast<float>(action.GetParamInt(4));

#ifndef EXTERNAL_RELEASE
	if (g_pkApp->IsSingleMode() == true)
	{
		movingSpeed = 120.0f ;
	}
	else
#endif
	{
		movingSpeed = static_cast<float>(actor.GetAbil(AT_C_MOVESPEED));
	}

	if(0.0f != fCustomSpeed)
	{
		movingSpeed = fCustomSpeed;
	}
	
	float fOriginalMoveSpeed = static_cast<float>(actor.GetAbil(AT_MOVESPEED));
	if (0.0f == fOriginalMoveSpeed)
	{
		fOriginalMoveSpeed = movingSpeed;
	}

	if(g_pkWorld)
	{
		if(g_pkWorld->GetAttr() & GATTR_VILLAGE)
		{
			movingSpeed += static_cast<float>(actor.GetAbil(AT_C_VILLAGE_MOVESPEED));
		}
	}
	
	float fAnimSpeed = 0.0f;
	
	if (0.0f < fOriginalMoveSpeed)
	{
		fAnimSpeed = movingSpeed/fOriginalMoveSpeed;
	}
	
	char const* pkAutoSpeed = actor.GetAnimationInfo("USE_AUTO_ANI_SPEED",0);
	if(NULL==pkAutoSpeed || 0!=strcmp(pkAutoSpeed,"FALSE"))
	{
		actor.SetAnimSpeed(fAnimSpeed);
	}
	
	DoAutoFire(actor);
	
	if (actor.IsMyActor() && action.GetParamInt(1) == 0 )
	{
		if (accumTime - action.GetActionEnterTime() > 0.1f)
		{
			actor.SetComboCount(0);
			action.SetParamInt(1,1);
		}
	}
	
	if (bMoveToPos)
	{
		lwPoint3	kMoveTargetPos = action.GetParamAsPoint(0);
		lwPoint3	kMoveDirection = action.GetParamAsPoint(1);	
		lwPoint3	kMoveStartPos = action.GetParamAsPoint(2);
		
		lwPoint3	kDir1 = actor.GetPos().Subtract2(kMoveTargetPos);
		kDir1.Unitize();
		lwPoint3	kDir2 = kMoveStartPos.Subtract2(kMoveTargetPos);
		kDir2.Unitize();
		
		if (0 > kDir1.Dot(kDir2) || 5 > actor.GetPos().Distance(kMoveTargetPos))
		{
			actor.SetTranslate(kMoveTargetPos,false);
			return	false;
		}
		
		kMoveDirection.Multiply(movingSpeed);
		actor.SetMovingDelta(kMoveDirection);
	
		return	true;
	}
	
	BYTE dir = actor.GetDirection();
	
	if (dir == DIR_NONE)
	{
		if (actor.GetWalkingToTarget() == false)
		{
			return false ;
		}
	}
	
	if (movingSpeed == 0)
	{
	    return  false;
	}
	actor.Walk(dir, movingSpeed,0,false);

	lwPoint3 vel = actor.GetVelocity();
	float z = vel.GetZ();

	// 뛰어가다가 발이 땅에서 떨어졌을 경우
	// 올라가는 점프를 해야 할지, 내려오는 점프를 해야 할지 결정
	if (actor.IsMeetFloor() == false)
	{
		if (z < -2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_down");
			return false;
		}
		else if (z > 2)
		{
			action.SetNextActionName(ACTIONNAME_JUMP);
			action.SetParam(2, "fall_up");
			return false;
		}
	}

	// 사다리 체크
	if (actor.ContainsDirection(DIR_UP) && 
		actor.IsMyActor() &&
		actor.ClimbUpLadder())
	{
		action.SetNextActionName("a_ladder_idle");
		return false;
	}
	
	return true;
}
bool	PgActionFSM_Act_RunBear::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	lwAction curAction = actor.GetAction();

	lwCheckNil(curAction.IsNil());
	std::string kActionID = action.GetID();

	if (kActionID.substr(0,6) == ACTIONNAME_JUMP)
	{
		std::string param = curAction.GetParam(2);
		if (param == "fall_down")
		{
			action.SetSlot(2);
			action.SetDoNotBroadCast(true);
		}
		else if (param == "fall_up")
		{
			action.SetSlot(1);
			action.SetDoNotBroadCast(true);
		}
		else if (param == "null")
		{
			if(actor.IsMeetFloor())
			{
				action.SetSlot(1);
			}
			else
			{
				action.SetSlot(2);
			}
		}
	}
	else if (kActionID == "a_ladder_idle" ||
		kActionID == "a_ladder_down" ||
		kActionID == "a_ladder_up" )
	{
		actor.HideParts(6, true);
		actor.SetParam("LADDER_WEAPON_HIDE","TRUE");
	}
	else if (kActionID.substr(0,6) == ACTIONNAME_IDLE)
	{
		//action.SetDoNotBroadCast(true)
		return true;
	}

	actor.UseSkipUpdateWhenNotVisible(true);

	return true;
}
bool	PgActionFSM_Act_RunBear::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	actor.BackMoving(false);
	return	true;
}

bool	PgActionFSM_Act_RunBear::OnEvent(lwActor actor,std::string textKey,int iSeqID)	const
{
	lwCheckNil(actor.IsNil());
	if (textKey == "start")
	{
		actor.AttachParticle(BM::Rand_Range(-200, -100), "char_root", "e_run");
	}
	return	true;
}
///
bool	PgActionFSM_Act_DashBear::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	//펫 탑승 중이라면 대쉬 불가
	if(actor.IsRidingPet())
	{
		return false;
	}

	// 기획팀장님이 공중에서 대쉬 할 수 있게 해달랍니다;;(단 한 번만)
	if (actor.IsMeetFloor() == false)
	{
		if (actor.GetJumpAccumHeight()<50)
		{
			return false;
		}
	}
	
	if (actor.IsOnlyMoveAction())
	{
		return false;
	}
#ifndef EXTERNAL_RELEASE
	if (g_pkApp->IsSingleMode() == false)
#endif
	{
		//	이동 속도가  0이면 안된다.
		if (actor.GetAbil(AT_C_MOVESPEED) == 0)
		{
			return	false;
		}
	}

	return true;
}
bool	PgActionFSM_Act_DashBear::OnEnter(lwActor actor,lwAction action)	const
{
	if(!g_pkWorld)
	{
		return false;
	}

	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if (actor.IsMyActor() == false)
	{
//		ODS("======================== Other actor's dash Begin =====================\n")
		actor.ClearReservedAction();
	}

	float	fDashSpeed = lua_tinker::call<float>("GetDashSpeed");

	action.SetParamFloat(0, g_pkWorld->GetAccumTime());	// Start Time
	action.SetParamFloat(1, fDashSpeed);	// Start Velocity
	action.SetParam(2,"");

	action.SetParamAsPoint(7, actor.GetPos());
	action.SetParamInt(8, static_cast<int>(g_kEventView.GetServerElapsedTime()));
	
	actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0);
	
	actor.FindPathNormal();
	lwPoint3	pt = actor.GetTranslate();
	pt.SetZ(pt.GetZ()-30);
	//actor.AttachParticleToPoint(201, pt, "e_special_transform")	
	actor.SetComboCount(0);
	
	if (actor.IsMyActor() == false)
	{
		actor.SetTranslate(action.GetActionStartPos(),false);
	}
	
	actor.UseSkipUpdateWhenNotVisible(false);
	action.SetParamInt(4,0);	
	// actor.SetDirection(action.GetDirection());
	lwCommonSkillUtilFunc::InitIsBanSubPlayerAction(actor, action);	// SC 스킬을 쓰면 안되는 스킬인가
	return true;
}
bool	PgActionFSM_Act_DashBear::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int	iState = action.GetParamInt(4);

	std::string jumping = "TRUE";
	if (actor.IsJumping() == false)
	{
		jumping = "FALSE";
	}

	//ODS("__________Jumping . " .. jumping .. "________\n")
	if (iState == 0)
	{
		actor.FindPathNormal();		
		
		float fAccel = -1000 * frameTime;
		float fVelocity = action.GetParamFloat(1);
		
//		local kMovingDir = actor.GetLookingDir()
//		kMovingDir.Multiply(fVelocity)
		
		BYTE dir = action.GetDirection();

		//ODS("______________Direction . " .. dir .. "\n")

		
		//현재는 일단 막아 둔다.
		//최대 대쉬 거리보다 많이 가는 경우가 생길 수 있으므로 시뮬레이션 후 최대값 이상 가지 못하도록 한다.
		NiPoint3 kNewPos = actor.GetPos()();
		NiPoint3 kOldPos = action.GetParamAsPoint(7)();
		kOldPos.z = kNewPos.z = 0.0f; //Z를 무시하고 계산한다. 떨어지는 것은 중력에 의한 것

		float const fDistance = (kNewPos - kOldPos).Length();

		//이동한 거리가 있을 경우
		if(0.0f < fDistance)
		{
			float const fMaxDistance = 130.0f;
			//남은 이동 거리
			float const fDistance2 = fMaxDistance - fDistance;

			//두 벡터를 이용하여 방향 벡터를 구하고
			//방향 벡터를 이용하여 실제로 이동하는 곳의 위치를 구한다.
			NiPoint3 kDirection = kNewPos - kOldPos;
			kDirection.Unitize();
			kDirection *= fVelocity * frameTime;
			kDirection += kNewPos;

			// 이번 프레임에 이동 하는 거리
			float const fDistance3 = (kDirection - kOldPos).Length();

			//이미 최대 대쉬 이동거리 거리를 벗어난 경우
			if(fMaxDistance - fDistance3 < 0)
			{
				//fVelocity값을 다시 세팅해야 한다.				
				fVelocity = fDistance2 / frameTime;
			}
		}

		actor.Walk(dir, fVelocity,0,false);
		
		fVelocity = fVelocity + fAccel;
		fVelocity = __max(0, fVelocity);
		action.SetParamFloat(1, fVelocity);
		
		float fElapsedTime = accumTime - action.GetParamFloat(0);
		if (0.3f < fElapsedTime)
		{
			// TODO . ElapsedTime이 0.3에서 얼마나 벗어났는지 확인 후에, 그만큼 뒤로 당겨 주어야 한다.
			action.SetParamInt(4,1);
			action.SetParamFloat(5, accumTime);
		}
	}
	else
	{
		float fElapsedTime = accumTime - action.GetParamFloat(5);
		float	fDashFreezeTime = lua_tinker::call<float>("GetDashFreezeTime");
		if (fElapsedTime>=fDashFreezeTime)
		{
			action.SetParam(2,"end");
			if (actor.IsMeetFloor() == false)
			{
				action.SetNextActionName(ACTIONNAME_JUMP);
			}
			return false;
		}
	}
	
	if (std::string(action.GetParam(3)) == "EndNow")
	{
		return false;
	}
		
	return	true;
}
bool	PgActionFSM_Act_DashBear::OnCleanUp(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());

	actor.UseSkipUpdateWhenNotVisible(true);
	actor.EndBodyTrail();
	return true;
}
bool	PgActionFSM_Act_DashBear::OnLeave(lwActor actor,lwAction action,bool bCancel)	const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	std::string kNextActionID = action.GetID();	
	lwAction kCurAction = actor.GetAction();

	lwCheckNil(kCurAction.IsNil());
	std::string newActionID = action.GetID();

	lwCheckNil(actor.GetPilot().IsNil());
    int	iBaseActorType = actor.GetPilot().GetBaseClassID();
    
	if (newActionID.substr(0,6) == ACTIONNAME_JUMP)
	{
		//action.SetDoNotBroadCast(true)
		
		action.SetSlot(2);
	}

	if (std::string(action.GetActionType())=="EFFECT")
	{
		return true;
	};
	
	if (std::string(kCurAction.GetParam(2)) == "end")
	{
		if (newActionID.substr(0,6) == ACTIONNAME_JUMP)
		{
			action.SetSlot(2);
		}
		return	true;
	}
	else if (newActionID.substr(0,6) == ACTIONNAME_JUMP && actor.IsJumping() == false)
	{
		if (std::string(action.GetParam(5)) == "HiJump" ) // hiJump면 그냥 jump로 넘어감.
		{
			action.SetSlot(1);
			kCurAction.SetParam(3, "EndNow");
			return true;
		}
	}
	else if( newActionID == "a_telejump" ||
		newActionID == "a_lock_move" ||
//		newActionID == "a_trap" ||
		newActionID == "a_teleport"
		|| newActionID == "a_teleport_rocket"
		)
	{
		return true;
	}

	return false;
}

///
bool PgActionFSM_Act_JumpBear::OnCheckCanEnter(lwActor actor,lwAction action)	const
{
	lwCheckNil(actor.IsNil());
	
	lwAction prevAction = actor.GetAction();
	lwCheckNil(prevAction.IsNil());
	
	if( prevAction.IsNil() ) 
	{
		return	true;
	}


	lwPacket kPacket = action.GetParamAsPacket();
	if( kPacket.IsNil() == false ) 
	{
		action.SetParamFloat(4,kPacket.PopFloat());
		action.SetParamInt(7,kPacket.PopInt());
	}
		
	std::string actionName = prevAction.GetID();
	
	//	낙법 체크
	//CheckBreakFall(actor);

	if( lwIsSingleMode() == false && actor.GetAbil(AT_C_MOVESPEED) == 0 ) 
	{ // 0 이면 점프 할 수 없는 상태
		return false;
	}
	
	if( IsFloatEvasion(actor,action) )
	{
		actor.ClearAllActionEffect();
		return	true;
	}
	
    if( actor.IsMeetFloor() == false ) 
	{
        if( actionName == "a_ladder_down" ||
		    actionName == "a_ladder_idle" || 
		    actionName == "a_ladder_up" ||
		    actionName == "a_ladder_dash" ||
			actionName.substr(0,5) == ACTIONNAME_RUN ||
			actionName.substr(0,6) == ACTIONNAME_IDLE ||
			actionName.substr(0,6) == ACTIONNAME_JUMP ||
			actionName.substr(0,6) == "a_dash"
			)
		{
			return true;
		}
		return false;
	}
	return true;
}
bool PgActionFSM_Act_JumpBear::OnEnter(lwActor actor,lwAction action) const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());
	
	lwAction prevAction = actor.GetAction();
	
	if(	prevAction.IsNil() )
	{
		return	true;
	}
	lwCommonSkillUtilFunc::InitIsBanSubPlayerAction(actor, action);	// SC 스킬을 쓰면 안되는 스킬인가
	std::string actionName = prevAction.GetID();

	float	fJumpForce = lua_tinker::get<float>(*lua_wrapper_user(g_kLuaTinker),"jumpForce");
	float	bIsFloatEvasion = (action.GetParamInt(7) == 1);
	
	if( strcmp(action.GetParam(4), "null") != 0 ) 
	{
		fJumpForce = PgStringUtil::SafeAtof(action.GetParam(4));
		if( fJumpForce == 0.0f)
		{
			fJumpForce = lua_tinker::get<float>(*lua_wrapper_user(g_kLuaTinker),"jumpForce");
		}
	}
	
	if( bIsFloatEvasion ) 
	{
		fJumpForce = 30;
	}
		
	action.SetParamFloat(4,fJumpForce);
	
	if( STR_HIJUMP == action.GetParam(5) )
	{
		//UseCameraHeightAdjust(false)
	}
	
	if( bIsFloatEvasion 
		|| ESS_CASTTIME == prevAction.GetActionParam()	// 캐스팅중 캔슬이면
		) 
	{
		actor.StartJump(fJumpForce);
		return	true;
	}

	if( actionName.substr(0,5) == ACTIONNAME_RUN ||
		actionName == "a_walk_left" ||
		actionName == "a_walk_right" ||
		actionName == "a_walk_up" ||
		actionName == "a_walk_down" ||
		actionName == "a_ladder_down" ||
		actionName == "a_ladder_idle" || 
		actionName == "a_ladder_up" ||
		actionName == "a_ladder_dash" ||
		actionName.substr(0,6) == "a_dash" || 
		STR_HIJUMP == action.GetParam(5)
		) 
	{
		if( action.GetCurrentSlot() == 1 ) 
		{
			if( strcmp(prevAction.GetParam(2), "fall_up") != 0 ) 
			{
				SetComboAdvisor(actor,action);
				actor.StartJump(fJumpForce);
			}
			lwPoint3	pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
		}
		else if( action.GetCurrentSlot() == 2 ) 
		{
			if( actor.IsJumping() == false ) 
			{
				actor.StartJump(0);
			}
		}
		return true;
	}
	else if( actionName.substr(0,6) == ACTIONNAME_JUMP ) 
	{
		if( strcmp(action.GetParam(1), "jump_again") == 0 )
		{
			actor.StartJump(fJumpForce/2);
			action.SetSlot(4);
			return true;
		}
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false 
			) 
		{
			action.SetSlot(1);
			actor.StartJump(fJumpForce);
			SetComboAdvisor(actor,action);
			return true;
		}
		else
		{
			action.SetSlot(2);
			return false;
		}
	}
	else if( actionName.substr(0,6) == ACTIONNAME_IDLE 
		|| actionName == "a_hang" 
		|| actionName == "a_rope" 
		)
	{
		if( actor.IsMeetFloor() == false && 
			action.GetCurrentSlot() != 2
			) 
		{
			return	false;
		}
		return true;
	}

	return false;
}
void PgActionFSM_Act_JumpBear::OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const
{
	lwCheckNil(action.IsNil());
	lwCheckNil(kPacket.IsNil());

	kPacket.PushFloat(action.GetParamFloat(4));
	kPacket.PushInt(action.GetParamInt(7));	//	Is this Float Evasion ?
}
bool PgActionFSM_Act_JumpBear::OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime) const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	int curAnimSlot = action.GetCurrentSlot();
	float movingSpeed = 0.0f;
	float	fJumpForce = action.GetParamFloat(4);

	if( lwIsSingleMode() == true ) 
	{
		movingSpeed = 150;
	}
	else
	{
		movingSpeed = static_cast<float>( actor.GetAbil(AT_C_MOVESPEED) );
	}

	if( NULL != g_pkWorld ) 
	{
		if( g_pkWorld->GetAttr() == GATTR_VILLAGE ) 
		{
			movingSpeed = movingSpeed + actor.GetAbil(AT_C_VILLAGE_MOVESPEED);
		}
	}

	DoAutoFire(actor);
	
	lwPoint3 vel = actor.GetVelocity(); 	// Current Velocity
	float z = vel.GetZ(); 				// Gravity

	//std::string param = action.GetParam(0);

	// Gravity값이 깔끔하게 0이 나오지 않는 것에 주의.
	bool IsAnimDone = actor.IsAnimationDone();

	std::string nextAction = ACTIONNAME_IDLE;
	int dir = actor.GetDirection();

	////ODS("Current Slot = " .. curAnimSlot .. "\n")

	// 사다리 체크
	if( actor.ContainsDirection(DIR_UP) && 
		actor.IsMyActor() &&
		actor.ClimbUpLadder() 
		)
	{
		action.SetNextActionName("a_ladder_idle");
		return false;
	}

	if( dir != DIR_NONE ) 
	{
		actor.Walk(dir, movingSpeed, 0.0f, false);
		nextAction = ACTIONNAME_RUN;
	}
	else if( actor.GetWalkingToTarget() == true ) 
	{
		actor.Walk(dir, movingSpeed, 0.0f, false);
	}

	if( curAnimSlot == 0 ) 
	{
		if( IsAnimDone == true ) 
		{
			actor.StartJump(fJumpForce);
            SetComboAdvisor(actor,action);
			
			lwPoint3 pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
			actor.PlayNext();
			return true;
		}
		else if( dir != DIR_NONE ) 
		{
			actor.PlayNext();
			actor.StartJump(fJumpForce);
            SetComboAdvisor(actor,action);
			
			lwPoint3	pt = actor.GetTranslate();
			pt.SetZ(pt.GetZ() - 30);
			actor.AttachParticleToPoint(2, pt, "e_jump");
		}
		else if( z > g_fLandingThreshold ) 
		{
			actor.PlayNext();
		}
	}	
	else if( curAnimSlot == 1 )
	{
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false &&
			actor.IsJumping() == false 
			)
		{
			if( dir == DIR_NONE ) 
			{
				actor.Stop();
				action.SetSlot(3);
				actor.PlayCurrentSlot(false);
			}
			else
			{
				// Jump가 3번 슬롯이어야 run으로 Enter가능
				// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
				if( nextAction != ACTIONNAME_IDLE ) 
				{
					action.SetSlot(3);
					action.SetNextActionName(nextAction.c_str());
					return false;
				}
				else if( strcmp(action.GetParam(1), "jump_again") != 0 )
				{
					action.SetNextActionName(ACTIONNAME_JUMP);
					return false;
				}
			}
		}
		else if( IsAnimDone == true ) 
		{
			actor.PlayNext();
		}
	}
	else if( curAnimSlot == 2 ) 
	{
		if( actor.IsMeetFloor() == true &&
			actor.IsSlide() == false ) 
		{
			if( dir == DIR_NONE ) 
			{
				actor.Stop();
				actor.PlayNext();
			}
			else
			{
				// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
				if( nextAction != ACTIONNAME_IDLE ) 
				{
					action.SetSlot(3);
					action.SetNextActionName(nextAction.c_str());
					return false;
				}
				else if( strcmp(action.GetParam(1), "jump_again") != 0 ) 
				{
					// param이 run이 아니고,
					// 이단 점프를 한 것이 아니면, 이단 점프를 한다.
					action.SetNextActionName(ACTIONNAME_JUMP);
					return false;
				}
			}
		}
	}
	else if( curAnimSlot == 3 ) 
	{ 
		if( IsAnimDone == true ) 
		{
			return false;
		}
		else
		{
			if( nextAction == ACTIONNAME_JUMP ) 
			{
				action.SetNextActionName(nextAction.c_str());
				return false;
			}
		}
	}
	else if( curAnimSlot == 4 ) 
	{ 
		if( actor.IsMeetFloor() == true ) 
		{
			// param으로 run이 들어오면 착지후에 달리는 애니를 주겠다는 뜻
			if( nextAction != ACTIONNAME_IDLE ) 
			{
				action.SetSlot(3);
				action.SetNextActionName(nextAction.c_str());
				return false;
			}
			else
			{
				action.SetSlot(3);
				actor.PlayCurrentSlot(false);
			}
		}
		else if( IsAnimDone == true ) 
		{
			action.SetSlot(2);
			actor.PlayCurrentSlot(false);
		}
	}

	return true;
}
bool PgActionFSM_Act_JumpBear::OnLeave(lwActor actor,lwAction action,bool bCancel) const
{
	lwCheckNil(actor.IsNil());

	lwAction curAction = actor.GetAction();
	lwCheckNil(curAction.IsNil());

	std::string nextActionName = action.GetID();
	lwCheckNil(action.IsNil());
	
	
	// 새로운 액션이 들어왔을 때
	if( action.GetEnable() == true ) 
	{
		if( nextActionName == ACTIONNAME_JUMP && 
			strcmp(curAction.GetParam(1), "jump_again") != 0 ) 
		{
			if( actor.GetAbil(AT_DOUBLE_JUMP_USE) == 1 ) 
			{
				if( curAction.GetCurrentSlot() == 1 || 
					curAction.GetCurrentSlot() == 2 ) 
				{
					if( 0 == PgStringUtil::SafeStrcmp("TRUE", actor.GetParam("DOUBLE_JUMP")) ) 
					{
						action.SetParam(1, "jump_again");
					}
				}
				return true;
			}
		}
		else if( nextActionName == ACTIONNAME_RUN ||
			actor.IsMeetFloor() && 
			actor.IsMeetSide() == false 
			)
		{
			//action.SetDoNotBroadCast(true)
			return true;
		}
		else if( nextActionName == "a_ladder_down" || 
			nextActionName == "a_ladder_idle" ||
			nextActionName == "a_ladder_up" 
			)
		{
			actor.HideParts(6, true);
			actor.SetParam("LADDER_WEAPON_HIDE","TRUE");
			return true;
		}
		else if( nextActionName == ACTIONNAME_IDLE )
		{
			curAction.SetParam(0, "null");
			actor.DetachFrom(2);
		}
		else if( nextActionName == "a_hang" &&
			strcmp(curAction.GetParam(5), "Done") != 0
			)
		{
			return false ;
		}
		return true;
	}
	return false;
}
bool PgActionFSM_Act_JumpBear::OnCleanUp(lwActor actor,lwAction action) const
{
	lwCheckNil(actor.IsNil());
	lwAction curAction = actor.GetAction();
	lwCheckNil(curAction.IsNil());
	
	if( curAction.IsNil() == false && STR_HIJUMP == curAction.GetParam(5) ) 
	{
		lwUseCameraHeightAdjust(true);
	}
	return true;
}


//void PgActionFSM_Act_JumpBear::CheckBreakFall(lwActor actor) const
//{
//	lwCheckNil(actor.IsNil());
//	if(!g_pkWorld)
//	{
//		return;
//	}
//
//	if(actor.IsMyActor())	//	낙법 처리
//	{
//		lwAction action = actor.GetAction();
//		lwCheckNil(action.IsNil());
//
//		if(action.IsNil() == false 
//			&& action.CanBreakFall()
//			)
//		{
//			float fLastPressTime = action.GetParamFloat(6);
//			if(0 ==  fLastPressTime)
//			{
//				fLastPressTime = g_pkWorld->GetAccumTime();
//				action.SetParamFloat(6,fLastPressTime);
//			}
//		}
//	
//		if( actor.IsDownState())
//		{
//			float fDownTime = actor.GetTotalDownTime();
//			fDownTime = fDownTime - 0.5f;
//			if( fDownTime < 0)
//			{
//				fDownTime = 0;
//			}
//			actor.SetTotalDownTime(fDownTime);
//		}
//	}
//}

bool PgActionFSM_Act_JumpBear::IsFloatEvasion(lwActor actor, lwAction action) const
{
	lwCheckNil(actor.IsNil());
	lwCheckNil(action.IsNil());

	if(!g_pkWorld)
	{
		return false;
	}

	if( action.GetParamInt(7) == 1 ) 
	{
		return	true;
	}
	
	if( actor.IsMyActor() == false ) 
	{
		return	false;
	}
	
	if( actor.IsMeetFloor() )
	{
		return	false;
	}

	lwAction	kCurAction = actor.GetAction();
	if( kCurAction.IsNil() ) 
	{
		return	false;
	}
	
	lwInputSlotInfo kInputSlotInfo = action.GetInputSlotInfo();
	if( kInputSlotInfo.IsNil() ) 
	{
		//ODS("Act_Jump_IsFloatEvasion kInputSlotInfo.IsNil()\n");
		return	false;
	}
	
	if( kInputSlotInfo.GetUKey() != 3023 ) 
	{
		//ODS("Act_Jump_IsFloatEvasion kInputSlotInfo.GetUKey() ~= 3023\n");
		return	false;
	}
	
	if( 0 == PgStringUtil::SafeStrcmp(actor.GetParam("FLOAT_EVASION"), "FALSE") ) 
	{
		return	false;
	}

	if( actor.IsBlowUp() == false ) 
	{
		//ODS("Act_Jump_IsFloatEvasion actor.IsBlowUp() == false\n");
		return	false;
	}
	
	float fActionStartTime = kCurAction.GetActionEnterTime();
	float fCurrentTime = g_pkWorld->GetAccumTime();
	
	if( (fCurrentTime - fActionStartTime) < g_fEvasionStartTime ) 
	{
		return	false;
	}
	
	action.SetParamInt(7,1);
	actor.SetParam("FLOAT_EVASION","FALSE");
	actor.SetCanHit(false);
	
	return	true;
}

void PgActionFSM_Act_JumpBear::SetComboAdvisor(lwActor actor, lwAction action) const
{
	lwCheckNil(actor.IsNil());

	if( actor.IsMyActor() ) 
	{
		lwCheckNil(action.IsNil());

		lwGetComboAdvisor().OnNewActionEnter(action.GetID());

		lwPilot kPilot = actor.GetPilot();
		lwCheckNil(kPilot.IsNil());

		int iBaseClassID = kPilot.GetBaseClassID();

		if( iBaseClassID == UCLASS_FIGHTER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_melee_drop");
			lwGetComboAdvisor().AddNextAction("a_float_melee_01");
		}
		else if( iBaseClassID == UCLASS_THIEF ) 
		{
			lwGetComboAdvisor().AddNextAction("a_thi_melee_drop");
			lwGetComboAdvisor().AddNextAction("a_thief_float_melee_01");
		}
		else if( iBaseClassID == UCLASS_MAGICIAN ) 
		{
			lwGetComboAdvisor().AddNextAction("a_magician_down_shot");
			lwGetComboAdvisor().AddNextAction("a_MagicianFloatShot_01");
		}
		else if( iBaseClassID == UCLASS_ARCHER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_archer_down_shot");
			lwGetComboAdvisor().AddNextAction("a_MagicianFloatShot_01");
		}
		else if( iBaseClassID == UCLASS_DOUBLE_FIGHTER ) 
		{
			lwGetComboAdvisor().AddNextAction("a_twin_float_melee_01");
			lwGetComboAdvisor().AddNextAction("a_twin_float_melee_01");
		}
	}
}
///
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
namespace OldActionLuaFunc
{// lua에 있던 함수들을 C로 옮기는중 겹치는 내용을 함수화 한것들
	bool HitOneTime(lwActor actor,lwAction action)
	{
		if( lwCheckNil(actor.IsNil()) )
		{
			return false;
		}
		if( lwCheckNil(action.IsNil()) )
		{
			return false;
		}
		if( NULL == g_pkWorld )
		{
			return false;	
		}

		action.SetParamFloat(6,g_pkWorld->GetAccumTime());

		int	iHitCount = action.GetParamInt(5);

		//lwGUID kOriginalActorGUID = action.GetGUIDInContainer(0);

		//if ( kOriginalActorGUID.IsNil() == true )
		//{
		//	return true;
		//}

		//lwPilot	kPilot( g_kPilotMan.FindPilot(kOriginalActorGUID()) );
		//if ( kPilot.IsNil() == true) 
		//{
		//	return true;
		//}	
		//lwActor kOriginalActor = kPilot.GetActor();
		
		int iTotalHit = lwCommonSkillUtilFunc::GetMaxHitCnt(action);

		if ( iHitCount == iTotalHit-1 ) 
		{
			int iTargetCount = action.GetTargetCount();
			int i =0;
			if ( iTargetCount>0 ) 
			{
				while (i<iTargetCount)
				{
					lwActionResult  actionResult = action.GetTargetActionResult(i);
					if ( actionResult.IsNil() == false )
					{
						action.SetParamInt(15, actionResult.GetValue() /iTotalHit );
						int	iOneDmg = action.GetParamInt(15);

						actionResult.SetValue( actionResult.GetValue() - iOneDmg*(iTotalHit-1) );

						lwGUID kTargetGUID = action.GetTargetGUID(i);
						lwPilot kTargetPilot = g_kPilotMan.FindPilot(kTargetGUID());
						if ( kTargetPilot.IsNil() == false ) {

							lwActor actorTarget = kTargetPilot.GetActor();

							lwActionTargetInfo kInfo = action.GetTargetInfo(i);
							OldActionLuaFunc::DoDamage(actor,actorTarget,actionResult, kInfo);
						}
					}
					++i;
				}
			}
			action.GetTargetList().ApplyActionEffects(false, false, false);
		}
		else
		{
			action.GetTargetList().ApplyOnlyDamage(iTotalHit, false, false);
			int iTargetCount = action.GetTargetCount();
			int i =0;
			if ( iTargetCount>0 ) 
			{
				while(i<iTargetCount)
				{
					lwActionResult actionResult = action.GetTargetActionResult(i);
					if ( actionResult.IsNil() == false ) 
					{
						lwGUID kTargetGUID = action.GetTargetGUID(i);
						lwPilot kTargetPilot( g_kPilotMan.FindPilot(kTargetGUID()) );
						if ( kTargetPilot.IsNil() == false ) 
						{
							lwActor actorTarget = kTargetPilot.GetActor();
							OldActionLuaFunc::DoDamage(actor,actorTarget,actionResult,action.GetTargetInfo(i));
						}
					}
					++i;
				}
			}
		}


		++iHitCount;

		if ( iHitCount == iTotalHit ) 
		{
			action.SetParamInt(4,2);	//	
			return	true;
		}

		action.SetParamInt(5,iHitCount);

		return	true;
	}

	void DoDamage(lwActor actor, lwActor actorTarget,lwActionResult kActionResult, lwActionTargetInfo kActionTargetInfo)
	{
		//if( lwCheckNil(actor.IsNil()) ) 
		//{
		//	return;
		//}
		if( lwCheckNil(actorTarget.IsNil()) )
		{
			return;
		}
		//lwAction kAction = actor.GetAction();
		//if( lwCheckNil(kAction.IsNil()) )
		//{
		//	return;
		//}

		int	iSphereIndex = kActionTargetInfo.GetABVIndex();
		lwPoint3 pt = actorTarget.GetABVShapeWorldPos(iSphereIndex);

		if (kActionResult.GetCritical() ) 
		{
			actorTarget.AttachParticleToPoint(12, pt, "e_dmg_cri");
		}
		else
		{
			actorTarget.AttachParticleToPoint(12, pt, "e_dmg");
		}

		// 충격 효과 적용
		actorTarget.SetShakeInPeriod(5, 72);
	}
}