#include "stdafx.h"
#include "Variant/DefAbilType.h"
#include "Variant/PgStringUtil.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "PgMobileSuit.h"
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

namespace lwDeathSnatchSkillScriptHelpFunc
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		
		def(pkState, "Skill_DeathSnatch_GetFirstTargetGuid", Skill_DeathSnatch_GetFirstTargetGuid);
		def(pkState, "Skill_DeathSnatch_SetAllSkillActorFlyingToStart", Skill_DeathSnatch_SetAllSkillActorFlyingToStart);
		def(pkState, "Skill_DeathSnatch_SetSkillActorFlyingToStart", Skill_DeathSnatch_SetSkillActorFlyingToStart);
		def(pkState, "Skill_DeathSnatch_FindTargetAtSkillActor", Skill_DeathSnatch_FindTargetAtSkillActor);
		def(pkState, "Skill_DeathSnatch_ProcessTextKeyEvent_End", Skill_DeathSnatch_ProcessTextKeyEvent_End);
		def(pkState, "Skill_DeathSnatch_ProcessTextKeyEvent_Hit", Skill_DeathSnatch_ProcessTextKeyEvent_Hit);
		def(pkState, "Skill_DeathSnatch_ProcessTextKeyEvent_Fire", Skill_DeathSnatch_ProcessTextKeyEvent_Fire);
		def(pkState, "Skill_DeathSnatch_LoadScriptParams", Skill_DeathSnatch_LoadScriptParams);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_FlyAngle", Skill_DeathSnatch_LoadScriptParam_FlyAngle);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_HandClassNo", Skill_DeathSnatch_LoadScriptParam_HandClassNo);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_HandCount", Skill_DeathSnatch_LoadScriptParam_HandCount);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_HandGapAngle", Skill_DeathSnatch_LoadScriptParam_HandGapAngle);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_HideWeapon", Skill_DeathSnatch_LoadScriptParam_HideWeapon);
		def(pkState, "Skill_DeathSnatch_LoadScriptParam_HideHandCreationAnimation", Skill_DeathSnatch_LoadScriptParam_HideHandCreationAnimation);
		def(pkState, "Skill_DeathSnatch_FireAllHand", Skill_DeathSnatch_FireAllHand);
		def(pkState, "Skill_DeathSnatch_FireHand", Skill_DeathSnatch_FireHand);
		def(pkState, "Skill_DeathSnatch_CheckTargetExist", Skill_DeathSnatch_CheckTargetExist);
		def(pkState, "Skill_DeathSnatch_AddAllTargetGuidToGuidContainer", Skill_DeathSnatch_AddAllTargetGuidToGuidContainer);
		def(pkState, "Skill_DeathSnatch_CheckTarget", Skill_DeathSnatch_CheckTarget);
		def(pkState, "Skill_DeathSnatch_Update_WaitHandReachTargetActionState", Skill_DeathSnatch_Update_WaitHandReachTargetActionState);
		def(pkState, "Skill_DeathSnatch_IsAllSkillActorReachedTarget", Skill_DeathSnatch_IsAllSkillActorReachedTarget);
		def(pkState, "Skill_DeathSnatch_IsSkillActorReachedTarget", Skill_DeathSnatch_IsSkillActorReachedTarget);
		def(pkState, "Skill_DeathSnatch_Update_WaitDrawAnimDoneActionState", Skill_DeathSnatch_Update_WaitDrawAnimDoneActionState);
		def(pkState, "Skill_DeathSnatch_HideWeapon", Skill_DeathSnatch_HideWeapon);
		def(pkState, "Skill_DeathSnatch_GetSkillActorAction", Skill_DeathSnatch_GetSkillActorAction);
		def(pkState, "Skill_DeathSnatch_GetSkillActor", Skill_DeathSnatch_GetSkillActor);
		def(pkState, "Skill_DeathSnatch_DeleteAllSkillActor", Skill_DeathSnatch_DeleteAllSkillActor);
		def(pkState, "Skill_DeathSnatch_DeleteSkillActor", Skill_DeathSnatch_DeleteSkillActor);
		def(pkState, "Skill_DeathSnatch_Fire", Skill_DeathSnatch_Fire);
		def(pkState, "Skill_DeathSnatch_GetSkillRange", Skill_DeathSnatch_GetSkillRange);
		def(pkState, "Skill_DeathSnatch_CreateAllSkillActor", Skill_DeathSnatch_CreateAllSkillActor);
		def(pkState, "Skill_DeathSnatch_AddSkillActor", Skill_DeathSnatch_AddSkillActor);
		def(pkState, "Skill_DeathSnatch_GetHandInitialPosition", Skill_DeathSnatch_GetHandInitialPosition);
		def(pkState, "Skill_DeathSnatch_HideWeapon", Skill_DeathSnatch_HideWeapon);
		def(pkState, "Skill_DeathSnatch_GetSkillActorAction", Skill_DeathSnatch_GetSkillActorAction);
		def(pkState, "Skill_DeathSnatch_GetSkillActor", Skill_DeathSnatch_GetSkillActor);
		def(pkState, "Skill_DeathSnatch_DeleteAllSkillActor", Skill_DeathSnatch_DeleteAllSkillActor);
		def(pkState, "Skill_DeathSnatch_DeleteSkillActor", Skill_DeathSnatch_DeleteSkillActor);
		def(pkState, "Skill_DeathSnatch_Fire", Skill_DeathSnatch_Fire);
		def(pkState, "Skill_DeathSnatch_GetSkillRange", Skill_DeathSnatch_GetSkillRange);
		def(pkState, "Skill_DeathSnatch_CreateAllSkillActor", Skill_DeathSnatch_CreateAllSkillActor);
		def(pkState, "Skill_DeathSnatch_AddSkillActor", Skill_DeathSnatch_AddSkillActor);
		def(pkState, "Skill_DeathSnatch_GetHandInitialPosition", Skill_DeathSnatch_GetHandInitialPosition);
		def(pkState, "Skill_DeathSnatch_GetHandFlyDirection", Skill_DeathSnatch_GetHandFlyDirection);
		def(pkState, "Skill_DeathSnatch_GetHandHorizontalRotationAngle", Skill_DeathSnatch_GetHandHorizontalRotationAngle);
		def(pkState, "Skill_DeathSnatch_GetHandVerticalRotationAngle", Skill_DeathSnatch_GetHandVerticalRotationAngle);
		def(pkState, "Skill_DeathSnatch_InitAllSkillActorAction", Skill_DeathSnatch_InitAllSkillActorAction);
		def(pkState, "Skill_DeathSnatch_InitSkillActorAction", Skill_DeathSnatch_InitSkillActorAction);
		def(pkState, "Skill_DeathSnatch_FindTarget_OnCurrentTargetPos", Skill_DeathSnatch_FindTarget_OnCurrentTargetPos);
		def(pkState, "Skill_DeathSnatch_FindTarget_FromPrevTargetPosToCurrentTargetPos", Skill_DeathSnatch_FindTarget_FromPrevTargetPosToCurrentTargetPos);
		def(pkState, "Skill_DeathSnatch_GetProjectileCollisionRadius", Skill_DeathSnatch_GetProjectileCollisionRadius);
		def(pkState, "Skill_DeathSnatch_SetActionState", Skill_DeathSnatch_SetActionState);
		def(pkState, "Skill_DeathSnatch_GetActionState", Skill_DeathSnatch_GetActionState);
		def(pkState, "Skill_DeathSnatch_SetTargetCount", Skill_DeathSnatch_SetTargetCount);
		def(pkState, "Skill_DeathSnatch_GetTargetCount", Skill_DeathSnatch_GetTargetCount);
		def(pkState, "Skill_DeathSnatch_SetFlyAngle", Skill_DeathSnatch_DeleteSkillActor);
		def(pkState, "Skill_DeathSnatch_GetFlyAngle", Skill_DeathSnatch_GetFlyAngle);
		def(pkState, "Skill_DeathSnatch_SetHandClassNo", Skill_DeathSnatch_SetHandClassNo);
		def(pkState, "Skill_DeathSnatch_GetHandClassNo", Skill_DeathSnatch_GetHandClassNo);
		def(pkState, "Skill_DeathSnatch_SetHandCount", Skill_DeathSnatch_SetHandCount);
		def(pkState, "Skill_DeathSnatch_SetHandGapAngle", Skill_DeathSnatch_SetHandGapAngle);
		def(pkState, "Skill_DeathSnatch_GetHandGapAngle", Skill_DeathSnatch_GetHandGapAngle);
		def(pkState, "Skill_DeathSnatch_SetHideHandCreationAnimation", Skill_DeathSnatch_SetHideHandCreationAnimation);
		def(pkState, "Skill_DeathSnatch_GetHideHandCreationAnimation", Skill_DeathSnatch_GetHideHandCreationAnimation);
		def(pkState, "Skill_DeathSnatch_SetHideWeaponEnable", Skill_DeathSnatch_SetHideWeaponEnable);
		def(pkState, "Skill_DeathSnatch_GetHideWeaponEnable", Skill_DeathSnatch_GetHideWeaponEnable);
		def(pkState, "Skill_DeathSnatch_SetTargetGuid", Skill_DeathSnatch_SetTargetGuid);
		def(pkState, "Skill_DeathSnatch_GetTargetGuid", Skill_DeathSnatch_GetTargetGuid);
		def(pkState, "Skill_DeathSnatch_AddSkillActorGuid", Skill_DeathSnatch_AddSkillActorGuid);
		def(pkState, "Skill_DeathSnatch_GetSkillActorGuid", Skill_DeathSnatch_GetSkillActorGuid);
		def(pkState, "Skill_DeathSnatch_GetSkillActorGuidSlotIndex", Skill_DeathSnatch_GetSkillActorGuidSlotIndex);
		def(pkState, "Skill_DeathSnatch_GetNumSkillActorGuid", Skill_DeathSnatch_GetNumSkillActorGuid);
		def(pkState, "Skill_DeathSnatch_SetNumSkillActorGuid", Skill_DeathSnatch_SetNumSkillActorGuid);
		def(pkState, "Skill_DeathSnatch_SetWeaponHidden", Skill_DeathSnatch_SetWeaponHidden);
		def(pkState, "Skill_DeathSnatch_GetWeaponHidden", Skill_DeathSnatch_GetWeaponHidden);
		
		// Hand
		def(pkState, "Skill_DeathSnatchHandMove_SetParentActorGuid", Skill_DeathSnatchHandMove_SetParentActorGuid);
		def(pkState, "Skill_DeathSnatchHandMove_SetShakeCamera", Skill_DeathSnatchHandMove_SetShakeCamera);
		def(pkState, "Skill_DeathSnatchHandMove_SetHideCreationAnimation", Skill_DeathSnatchHandMove_SetHideCreationAnimation);
		def(pkState, "Skill_DeathSnatchHandMove_GetCurrentActionState", Skill_DeathSnatchHandMove_GetCurrentActionState);
		def(pkState, "Skill_DeathSnatchHandMove_StartFlyingToTargetState", Skill_DeathSnatchHandMove_StartFlyingToTargetState);
		def(pkState, "Skill_DeathSnatchHandMove_SetFlyDirection", Skill_DeathSnatchHandMove_SetFlyDirection);
		def(pkState, "Skill_DeathSnatchHandMove_SetSkillRange", Skill_DeathSnatchHandMove_SetSkillRange);
		def(pkState, "Skill_DeathSnatchHandMove_SetNextActionState", Skill_DeathSnatchHandMove_SetNextActionState);
		def(pkState, "Skill_DeathSnatchHandMove_GetPrevActorPos", Skill_DeathSnatchHandMove_GetPrevActorPos);
		def(pkState, "Skill_DeathSnatchHandMove_SetCurrentActionState", Skill_DeathSnatchHandMove_SetCurrentActionState);
		def(pkState, "Skill_DeathSnatchHandMove_GetNextActionState", Skill_DeathSnatchHandMove_GetNextActionState);
		def(pkState, "Skill_DeathSnatchHandMove_SetFlyStartTime", Skill_DeathSnatchHandMove_SetFlyStartTime);
		def(pkState, "Skill_DeathSnatchHandMove_GetFlyStartTime", Skill_DeathSnatchHandMove_GetFlyStartTime);
		def(pkState, "Skill_DeathSnatchHandMove_GetFlyDirection", Skill_DeathSnatchHandMove_GetFlyDirection);
		def(pkState, "Skill_DeathSnatchHandMove_SetFlyStartPos", Skill_DeathSnatchHandMove_SetFlyStartPos);
		def(pkState, "Skill_DeathSnatchHandMove_GetFlyStartPos", Skill_DeathSnatchHandMove_GetFlyStartPos);
		def(pkState, "Skill_DeathSnatchHandMove_SetFlyTargetPos", Skill_DeathSnatchHandMove_SetFlyTargetPos);
		def(pkState, "Skill_DeathSnatchHandMove_GetFlyTargetPos", Skill_DeathSnatchHandMove_GetFlyTargetPos);
		def(pkState, "Skill_DeathSnatchHandMove_SetPrevActorPos", Skill_DeathSnatchHandMove_SetPrevActorPos);
		def(pkState, "Skill_DeathSnatchHandMove_SetTargetCount", Skill_DeathSnatchHandMove_SetTargetCount);
		def(pkState, "Skill_DeathSnatchHandMove_GetTargetCount", Skill_DeathSnatchHandMove_GetTargetCount);
		def(pkState, "Skill_DeathSnatchHandMove_SetFirstTargetGuid", Skill_DeathSnatchHandMove_SetFirstTargetGuid);
		def(pkState, "Skill_DeathSnatchHandMove_GetFirstTargetGuid", Skill_DeathSnatchHandMove_GetFirstTargetGuid);
		def(pkState, "Skill_DeathSnatchHandMove_GetParentActorGuid", Skill_DeathSnatchHandMove_GetParentActorGuid);
		def(pkState, "Skill_DeathSnatchHandMove_GetSkillRange", Skill_DeathSnatchHandMove_GetSkillRange);
		def(pkState, "Skill_DeathSnatchHandMove_GetShakeCamera", Skill_DeathSnatchHandMove_GetShakeCamera);
		def(pkState, "Skill_DeathSnatchHandMove_GetHideCreationAnimation", Skill_DeathSnatchHandMove_GetHideCreationAnimation);
		def(pkState, "Skill_DeathSnatchHandMove_StartLoadingState", Skill_DeathSnatchHandMove_StartLoadingState);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateCurrentState", Skill_DeathSnatchHandMove_UpdateCurrentState);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateNextState", Skill_DeathSnatchHandMove_UpdateNextState);
		def(pkState, "Skill_DeathSnatchHandMove_Finish", Skill_DeathSnatchHandMove_Finish);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateLoadingPosition", Skill_DeathSnatchHandMove_UpdateLoadingPosition);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateFlyingPosition", Skill_DeathSnatchHandMove_UpdateFlyingPosition);
		def(pkState, "Skill_DeathSnatchHandMove_AttachedPosition", Skill_DeathSnatchHandMove_AttachedPosition);
		def(pkState, "Skill_DeathSnatchHandMove_FlyToTarget", Skill_DeathSnatchHandMove_FlyToTarget);
		def(pkState, "Skill_DeathSnatchHandMove_FlyBackToStart", Skill_DeathSnatchHandMove_FlyBackToStart);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateFlyingBackFinished", Skill_DeathSnatchHandMove_UpdateFlyingBackFinished);
		def(pkState, "Skill_DeathSnatchHandMove_UpdateFlyingBackPosition", Skill_DeathSnatchHandMove_UpdateFlyingBackPosition);

		// Finish
		def(pkState, "Skill_DeathSnatchFinish_SetTargetsFromDeathSnatch", Skill_DeathSnatchFinish_SetTargetsFromDeathSnatch);
	}

	//------------------------------------------------
	// Definitions
	//------------------------------------------------
	// Param Definition
	// 0[int] : Action State (See Action State Definition)
	// 1[int] : Is the weapon hidden currently? ( 0 : Not Hidden 1 : Hidden)
	// 2[int] : Target Actor Count
	// 3[string] : Target Actor GUID
	// 4[float] : Fly Angle
	// 5[int] : Hand Class No
	// 6[int] : Hand Count
	// 7[float] : Hand Gap Angle
	// 8[int] : Hide Hand During Creation Animation ( 0 : Not Hide 1 : Hide)
	// 9[int] : use weapon hide || not
	// 10[int] : Num of Hand Skill Actor Guid 
	// 11 ~ 11+HandCount [int] : Hand Skill Actor Guid

	// Point Param Definition

	// GUID Container : Target Actor's GUID
	
	// Action State
	int const DEATHSNATCH_ACTIONSTATE_WAIT_FIRE_TEXTKEY = 0;
	int const DEATHSNATCH_ACTIONSTATE_WAIT_HAND_REACH_TARGET = 1;
	int const DEATHSNATCH_ACTIONSTATE_WAIT_TARGETLISTMODIFY_PACKET = 2;
	int const DEATHSNATCH_ACTIONSTATE_WAIT_DRAW_ANIM_DONE = 3;
	int const DEATHSNATCH_ACTIONSTATE_FINISHED = 4;
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_LOADING = 0;			//-- 0 : Loading
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_TARGET = 1;	//-- 1 : Flying to the Target
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_ATTACHED_TO_TARGET = 2;	//-- 2 : Attached To The Target
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_START = 3;//-- 3 : Flying back to Start Pos
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_WAIT = 4;	//-- 4 : Wait
	int const DEATHSNATCHHANDMOVE_ACTIONSTATE_FINISHED = 5;	//-- 5 : Finished

	// Script Param
	// "FLY_ANGLE" : Angle which the hand fly. In Degree (Default : 45)
	// "HAND_CLASS_NO" : class no of the hand skill actor
	// "HAND_COUNT" : How many hands are going to be fired?(Default : 1)
	// "HAND_GAP_ANGLE" : Angle of gap between each hands(Default : 30 degree)
	// "HIDE_HAND_CREATION_ANIMATION" : "TRUE" || "FALSE", do not show hand during creation animation is on going(Default : FALSE)
	// "HIDE_WEAPON" : "TRUE" || "FALSE", hide weapon || not


	/*
	//----------------------------------------------
	// Implementation Functions
	------------------------------------------------
	*/

	lwGUID Skill_DeathSnatch_GetFirstTargetGuid(lwActionTargetList kTargetList)
	{
		int const iTargetCount = kTargetList.size();
		if(0 == iTargetCount)
		{
			return	lwGUID(NULL);
		}
		lwActionTargetInfo kTargetInfo = kTargetList.GetTargetInfo(0);
		return	kTargetInfo.GetTargetGUID();
	}

	void Skill_DeathSnatch_SetAllSkillActorFlyingToStart(lwActor kActor, lwAction kAction)
	{
		int iNumSkillActor = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int i=0;

		while(i<iNumSkillActor)
		{
			Skill_DeathSnatch_SetSkillActorFlyingToStart(kActor, kAction, i);
			++i;
		}
	}

	void Skill_DeathSnatch_SetSkillActorFlyingToStart(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		lwAction kSkillActorAction = Skill_DeathSnatch_GetSkillActorAction(kActor, kAction, iSkillActorIndex);
		if(kSkillActorAction.IsNil())
		{
			return;
		}
		Skill_DeathSnatchHandMove_SetNextActionState(kSkillActorAction,DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_START);
	}

	void Skill_DeathSnatch_FindTargetAtSkillActor(lwActor kActor, lwAction kAction, lwActionTargetList kTargets, int iSkillActorIndex)
	{
		lwActor kSkillActor = Skill_DeathSnatch_GetSkillActor(kActor, kAction, iSkillActorIndex);
		if(kSkillActor.IsNil())
		{
			return;
		}

		lwPoint3 kCurrentTargetPos (kSkillActor.GetPos().GetX(), kSkillActor.GetPos().GetY(), kSkillActor.GetPos().GetZ());
		lwPoint3 kPrevTargetPos = Skill_DeathSnatchHandMove_GetPrevActorPos(kAction);

		Skill_DeathSnatch_FindTarget_OnCurrentTargetPos(kActor, kAction, kCurrentTargetPos, kTargets);
		Skill_DeathSnatch_FindTarget_FromPrevTargetPosToCurrentTargetPos(kActor, kAction, kCurrentTargetPos, kPrevTargetPos, kTargets);
	}

	void Skill_DeathSnatch_ProcessTextKeyEvent_End(lwActor kActor, lwAction kAction)
	{
		Skill_DeathSnatch_ProcessTextKeyEvent_Fire(kActor, kAction);
	}

	void Skill_DeathSnatch_ProcessTextKeyEvent_Hit(lwActor kActor, lwAction kAction)
	{
		Skill_DeathSnatch_ProcessTextKeyEvent_Fire(kActor, kAction);
	}

	void Skill_DeathSnatch_ProcessTextKeyEvent_Fire(lwActor kActor, lwAction kAction)
	{
		int iActionState = Skill_DeathSnatch_GetActionState(kAction);
		if(DEATHSNATCH_ACTIONSTATE_WAIT_FIRE_TEXTKEY != iActionState)
		{
			return;
		}
		Skill_DeathSnatch_FireAllHand(kActor, kAction);
	}

	void Skill_DeathSnatch_LoadScriptParams(lwActor kActor, lwAction kAction)
	{
		Skill_DeathSnatch_LoadScriptParam_FlyAngle(kActor, kAction);
		Skill_DeathSnatch_LoadScriptParam_HandClassNo(kActor, kAction);
		Skill_DeathSnatch_LoadScriptParam_HandCount(kActor, kAction);
		Skill_DeathSnatch_LoadScriptParam_HandGapAngle(kActor, kAction);
		Skill_DeathSnatch_LoadScriptParam_HideHandCreationAnimation(kActor, kAction);
		Skill_DeathSnatch_LoadScriptParam_HideWeapon(kActor, kAction);
	}

	void Skill_DeathSnatch_LoadScriptParam_FlyAngle(lwActor kActor, lwAction kAction)
	{
		std::string kFlyAngle = kAction.GetScriptParam("FLY_ANGLE");

		float fFlyAngle = PgStringUtil::SafeAtof(kFlyAngle);
		//if(0.0f == fFlyAngle )	{	fFlyAngle = 45.0f;	}
		Skill_DeathSnatch_SetFlyAngle(kAction,fFlyAngle);
	}

	void Skill_DeathSnatch_LoadScriptParam_HandClassNo(lwActor kActor, lwAction kAction)
	{
		std::string	kHandClassNo = kAction.GetScriptParam("HAND_CLASS_NO");
		int iHandClassNo = PgStringUtil::SafeAtoi(kHandClassNo);	
		if(0 == iHandClassNo)
		{
			_PgMessageBox("Death Snatch Script Error","Hand Class No Is Missed.");
		}
		Skill_DeathSnatch_SetHandClassNo(kAction, iHandClassNo);
	}


	//// 여기까지 등록
	void Skill_DeathSnatch_LoadScriptParam_HandCount(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		std::string kHandCount = kAction.GetScriptParam("HAND_COUNT");
		int iHandCount = PgStringUtil::SafeAtoi(kHandCount);
		if(0 == iHandCount)
		{
			iHandCount = 1;
		}

		Skill_DeathSnatch_SetHandCount(kAction, iHandCount);
	}

	void Skill_DeathSnatch_LoadScriptParam_HandGapAngle(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		std::string kHandGapAngle = kAction.GetScriptParam("HAND_GAP_ANGLE");
		float 	fHandGapAngle = PgStringUtil::SafeAtof(kHandGapAngle);
		if(fHandGapAngle == 0.0f )
		{
			fHandGapAngle = 30.0f;
		}
		Skill_DeathSnatch_SetHandGapAngle(kAction,fHandGapAngle);
	}

	void Skill_DeathSnatch_LoadScriptParam_HideWeapon(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		std::string kHideWeapon = kAction.GetScriptParam("HIDE_WEAPON");
		Skill_DeathSnatch_SetHideWeaponEnable(kAction, (kHideWeapon == "TRUE") );

	}

	void Skill_DeathSnatch_LoadScriptParam_HideHandCreationAnimation(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		std::string kHideHandCreationAnimation = kAction.GetScriptParam("HIDE_HAND_CREATION_ANIMATION");
		Skill_DeathSnatch_SetHideHandCreationAnimation(kAction, (kHideHandCreationAnimation == "TRUE") );
	}

	void Skill_DeathSnatch_FireAllHand(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int	iNumSkillActor = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int	i = 0;

		while (i<iNumSkillActor)
		{

			Skill_DeathSnatch_FireHand(kActor, kAction, i);
			++i;
		}

		Skill_DeathSnatch_SetActionState(kAction,DEATHSNATCH_ACTIONSTATE_WAIT_HAND_REACH_TARGET);
		kActor.PlayNext();
	}

	bool Skill_DeathSnatch_FireHand(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		lwActor	kSkillActor = Skill_DeathSnatch_GetSkillActor(kActor, kAction, iSkillActorIndex);
		if(kSkillActor.IsNil() == false )
		{
			lwAction kSkillActorAction = kSkillActor.GetAction();
			if(kSkillActorAction.IsNil())
			{
				return false;
			}
			Skill_DeathSnatchHandMove_StartFlyingToTargetState(kSkillActor, kSkillActorAction);
			Skill_DeathSnatchHandMove_SetFlyDirection(kSkillActorAction,Skill_DeathSnatch_GetHandFlyDirection(kActor, kAction, iSkillActorIndex));
			Skill_DeathSnatchHandMove_SetSkillRange(kSkillActorAction,Skill_DeathSnatch_GetSkillRange(kActor, kAction));
		}
		return	true;
	}

	bool Skill_DeathSnatch_CheckTargetExist(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		kAction.CreateActionTargetList(kActor, false);
		int iTargetCount = kAction.GetTargetCount();
		return	(0<iTargetCount);
	}

	void Skill_DeathSnatch_AddAllTargetGuidToGuidContainer(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iTargetCount = kAction.GetTargetCount();
		int	i = 0;

		kAction.ClearGUIDContainer();
		while (i<iTargetCount)
		{
			lwGUID kTargetGuid = kAction.GetTargetGUID(i);
			kAction.AddNewGUID(kTargetGuid);
			++i;
		}
	}

	bool Skill_DeathSnatch_CheckTarget(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		if(false == kActor.IsMyActor() )
		{
			return false;
		}

		if(Skill_DeathSnatch_CheckTargetExist(kActor, kAction) == true )
		{
			Skill_DeathSnatch_AddAllTargetGuidToGuidContainer(kActor, kAction);

			kAction.BroadCastTargetListModify(kActor.GetPilot(),true);
			Skill_DeathSnatch_SetActionState(kAction,DEATHSNATCH_ACTIONSTATE_WAIT_TARGETLISTMODIFY_PACKET);

			return	true;
		}
		return	false;
	}

	void Skill_DeathSnatch_Update_WaitHandReachTargetActionState(lwActor kActor, lwAction kAction, float accumTime, float frameTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		bool bCheckTargetResult = Skill_DeathSnatch_CheckTarget(kActor, kAction);

		if(false == Skill_DeathSnatch_IsAllSkillActorReachedTarget(kActor, kAction) )
		{
			return;
		}

		if(false == bCheckTargetResult)
		{
			Skill_DeathSnatch_SetActionState(kAction, DEATHSNATCH_ACTIONSTATE_WAIT_TARGETLISTMODIFY_PACKET);
			if(kActor.IsMyActor() )
			{
				if(g_pkApp->IsSingleMode())
				{
					lua_tinker::call<void, lwActor,lwAction,bool>("Skill_DeathSnatch_OnTargetListModified", kActor, kAction,false);
				}
				else
				{
					kAction.BroadCastTargetListModify(kActor.GetPilot(),true);
				}
			}
		}
	}

	bool Skill_DeathSnatch_IsAllSkillActorReachedTarget(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		int iNumSkillActor = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int	i=0;

		while (i<iNumSkillActor)
		{
			if(Skill_DeathSnatch_IsSkillActorReachedTarget(kActor, kAction, i) == false)
			{
				return	false;
			}
			++i;
		}
		return	true;
	}

	bool Skill_DeathSnatch_IsSkillActorReachedTarget(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		lwAction kSkillActorAction = Skill_DeathSnatch_GetSkillActorAction(kActor, kAction, iSkillActorIndex);
		int iSkillActorState = Skill_DeathSnatchHandMove_GetCurrentActionState(kSkillActorAction);
		bool bIsReachedToTarget = (iSkillActorState != DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_TARGET);
		return bIsReachedToTarget;
	}

	bool Skill_DeathSnatch_Update_WaitDrawAnimDoneActionState(lwActor kActor, lwAction kAction, float accumTime, float frameTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		bool bIsAnimDone = kActor.IsAnimationDone();

		if(bIsAnimDone == true )
		{
			Skill_DeathSnatch_SetActionState(kAction,DEATHSNATCH_ACTIONSTATE_FINISHED);
			return	false;

		}

		return	true;
	}


	void Skill_DeathSnatch_HideWeapon(lwActor kActor, lwAction kAction, bool bHide)
	{
		if(false == Skill_DeathSnatch_GetHideWeaponEnable(kAction))
		{
			return;
		}

		if(Skill_DeathSnatch_GetWeaponHidden(kAction) == bHide )
		{
			return;
		}

		kActor.HideParts(EQUIP_POS_WEAPON, bHide);
		Skill_DeathSnatch_SetWeaponHidden(kAction, bHide);
	}

	lwAction Skill_DeathSnatch_GetSkillActorAction(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		lwActor kSkillActor = Skill_DeathSnatch_GetSkillActor(kActor, kAction, iSkillActorIndex);
		return	kSkillActor.GetAction();
	}


	lwActor Skill_DeathSnatch_GetSkillActor(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		if(!g_pkWorld)
		{
			return lwActor(NULL);
		}
		lwGUID kActorGuid = Skill_DeathSnatch_GetSkillActorGuid(kAction, iSkillActorIndex);
		lwActor kFindActor(static_cast<PgActor*>( g_pkWorld->FindObject(kActorGuid()) ) );
		return kFindActor;
	}

	void Skill_DeathSnatch_DeleteAllSkillActor(lwActor kActor, lwAction kAction)
	{
		int iNumSkillActor = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int i = 0;

		while(i<iNumSkillActor)
		{
			lwGUID	kActorGuid = Skill_DeathSnatch_GetSkillActorGuid(kAction, i);
			Skill_DeathSnatch_DeleteSkillActor(kActorGuid);
			++i;
		}
	}

	void Skill_DeathSnatch_DeleteSkillActor(lwGUID kActorGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->RemoveObjectOnNextUpdate( kActorGuid() );
	}

	void Skill_DeathSnatch_Fire(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		kAction.SetSlot(kAction.GetCurrentSlot()+1);
		kActor.PlayCurrentSlot(false);

		std::string kFireEffectID = kAction.GetScriptParam("FIRE_CASTER_EFFECT_ID");
		std::string kFireEffectTargetNodeID = kAction.GetScriptParam("FIRE_CASTER_EFFECT_TARGET_NODE");
		std::string kFireEffectTargetNodeIDToPos = kAction.GetScriptParam("FIRE_CASTER_EFFECT_TARGET_NODE_TO_WORLD_POS");
		std::string kFireEffectTargetNodeIDToPosWithRotate = kAction.GetScriptParam("FIRE_CASTER_EFFECT_TARGET_NODE_TO_WORLD_POS_WITH_ROTATE");

		if (kFireEffectID != "" 
			&& (kFireEffectTargetNodeID != "" || kFireEffectTargetNodeIDToPos != ""  || kFireEffectTargetNodeIDToPosWithRotate != "" )
			)
		{
			if (kFireEffectTargetNodeID == "__FRONT_ACTOR" )
			{
				lwPoint3 kDir = kActor.GetLookingDir();
				float fDistance = 30.0f;
				kDir.Multiply(fDistance);
				kDir.Add(kActor.GetPos());
				kDir.SetZ(kDir.GetZ()-28);
				kActor.AttachParticleToPoint(7212, kDir, kFireEffectID.c_str());
			}
			else if(kFireEffectTargetNodeID != "" )
			{
				kActor.AttachParticle(7212, kFireEffectTargetNodeID.c_str(), kFireEffectID.c_str());
			}
			else if(kFireEffectTargetNodeIDToPos != "" )
			{
				kActor.AttachParticleToPoint(7212, kActor.GetNodeWorldPos(kFireEffectTargetNodeIDToPos.c_str()), kFireEffectID.c_str());
			}
			else
			{
				lwQuaternion kQuat =  kActor.GetRotateQuaternion();
				kActor.AttachParticleToPointWithRotate(7212, kActor.GetNodeWorldPos(kFireEffectTargetNodeIDToPosWithRotate.c_str()), kFireEffectID.c_str(), kQuat, 0);
			}
		}

	}

	int Skill_DeathSnatch_GetSkillRange(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iAttackRange = kAction.GetSkillRange(0, kActor);
		if(iAttackRange == 0 )
		{
			iAttackRange = 100;
		}

		return	iAttackRange;
	}

	void Skill_DeathSnatch_CreateAllSkillActor(lwActor kActor, lwAction kAction)
	{
		int iNumSkillActor = Skill_DeathSnatch_GetHandCount(kAction);
		int i = 0;
		while (i<iNumSkillActor)
		{
			Skill_DeathSnatch_AddSkillActor(kActor, kAction, i);
			++i;
		}
	}

	void Skill_DeathSnatch_AddSkillActor(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{	
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if(!g_pkWorld)
		{
			return;
		}

		lwGUID guid("123");
		guid.Generate();
		Skill_DeathSnatch_AddSkillActorGuid(kAction,guid);

		lwPoint3 kGenPoint = Skill_DeathSnatch_GetHandInitialPosition(kActor, kAction, iSkillActorIndex);

		lwPilot pilot( g_kPilotMan.NewPilot(guid(), Skill_DeathSnatch_GetHandClassNo(kAction), 0) );
		if(pilot.IsNil() == false)
		{
			lwActor kSkillActor = pilot.GetActor();
			kSkillActor.SetUseSmoothShow(false);
			g_pkWorld->AddObject(guid(), kSkillActor(), kGenPoint(), OGT_NPC);
		}
		else
		{
			_PgMessageBox("Death Snatch Script Error","Create Skill Actor Failed.");
		}
	}

	lwPoint3 Skill_DeathSnatch_GetHandInitialPosition(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		int	iNumSkillActor = Skill_DeathSnatch_GetHandCount(kAction);
		lwPoint3 kCenterPoint = kActor.GetNodeWorldPos("char_root");

		return	kCenterPoint;
	}

	lwPoint3 Skill_DeathSnatch_GetHandFlyDirection(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		float fVerticalRotationAngle = Skill_DeathSnatch_GetHandVerticalRotationAngle(kActor, kAction);
		float fHorizontalRotationAngle = Skill_DeathSnatch_GetHandHorizontalRotationAngle(kActor, kAction, iSkillActorIndex);

		lwPoint3	kUpDirection(0.0f, 0.0f, 1.0f);
		lwPoint3	kActorLookingDirection = kActor.GetLookingDir();
		lwPoint3	kFlyDirection = kActor.GetLookingDir();
		lwPoint3	kActorSideDirection = kActorLookingDirection.Cross(kUpDirection);

		kFlyDirection.Rotate(kActorSideDirection,fVerticalRotationAngle*3.141592f/180.0f);
		kFlyDirection.Rotate(kUpDirection,fHorizontalRotationAngle*3.141592f/180.0f);

		return	kFlyDirection;
	}

	float Skill_DeathSnatch_GetHandHorizontalRotationAngle(lwActor kActor, lwAction kAction, int iSkillActorIndex)
	{
		int   iNumSkillActor = Skill_DeathSnatch_GetHandCount(kAction);
		float fGapAngle = Skill_DeathSnatch_GetHandGapAngle(kAction);

		float	fRotationAngle = fGapAngle*(iNumSkillActor-1)*0.5f - iSkillActorIndex*fGapAngle;
		return	fRotationAngle;
	}

	float Skill_DeathSnatch_GetHandVerticalRotationAngle(lwActor kActor, lwAction kAction)
	{
		float fRotateAngle = Skill_DeathSnatch_GetFlyAngle(kAction);
		return	fRotateAngle;
	}

	void Skill_DeathSnatch_InitAllSkillActorAction(lwActor kActor, lwAction kAction)
	{
		int	iNumSkillActors = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int	i = 0;

		while(i<iNumSkillActors)
		{
			lwActor kSkillActor = Skill_DeathSnatch_GetSkillActor(kActor, kAction, i);
			Skill_DeathSnatch_InitSkillActorAction(kActor, kAction, kSkillActor);
			++i;
		}
	}

	void Skill_DeathSnatch_InitSkillActorAction(lwActor kActor,lwAction kAction,lwActor kSkillActor)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if( kSkillActor.IsNil() )	{ lwSetBreak(); }

		lwAction kHandAction = kSkillActor.ReserveTransitAction("a_Death Snatch_Hand_Move", DIR_NONE);

		std::string kTexture;
		char* pkTexture = kAction.GetScriptParam("TRAIL_TEXTURE");
		if(pkTexture)
		{
			kTexture = pkTexture;
		}

		if(!kTexture.empty())
		{
			char* pkTotalTime = kAction.GetScriptParam("TRAIL_TOTAL_TIME");
			int iTotalTime = PgStringUtil::SafeAtoi(pkTotalTime);

			char* pkBrightTime = kAction.GetScriptParam("TRAIL_BRIGHT_TIME");
			int iBrightTime =  PgStringUtil::SafeAtoi(pkBrightTime);

			//char* pkExtendLength = kAction.GetScriptParam("TRAIL_EXTEND_LENGTH");
			//float fExtendLength = PgStringUtil::SafeAtof(pkExtendLength);

			//ODS("kTexture : ".. kTexture .. " iTotalTime : ".. iTotalTime.. " iBrightTime : ".. iBrightTime .. " iExtendLength : " ..iExtendLength.."\n" ,false, 1509)

			kSkillActor.StartBodyTrail(pkTexture, iTotalTime, iBrightTime);
		}

		Skill_DeathSnatchHandMove_SetParentActorGuid(kHandAction, kActor.GetPilotGuid());

		if (kActor.IsMyActor())
		{
			Skill_DeathSnatchHandMove_SetShakeCamera(kHandAction, true);
		}

		Skill_DeathSnatchHandMove_SetHideCreationAnimation(kHandAction, Skill_DeathSnatch_GetHideHandCreationAnimation(kAction));

	}

	void Skill_DeathSnatch_FindTarget_OnCurrentTargetPos(lwActor kActor,lwAction kAction,lwPoint3 kCurrentTargetPos,lwActionTargetList kTargets)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		float fWideAngle = static_cast<float>(Skill_DeathSnatch_GetProjectileCollisionRadius(kAction));

		lwFindTargetParam kParam;
		kParam.SetParam_1(kCurrentTargetPos, lwPoint3(0.0f, 0.0f, 0.0f));
		kParam.SetParam_2(0.0f, 0.0f, fWideAngle, 0);
		kParam.SetParam_3(true,PgAction::FTO_BLOWUP);
		kAction.FindTargets(PgAction::TAT_SPHERE, kParam, kTargets, kTargets);
	}

	void Skill_DeathSnatch_FindTarget_FromPrevTargetPosToCurrentTargetPos(lwActor kActor, lwAction kAction, lwPoint3 kCurrentTargetPos, lwPoint3 kPrevTargetPos, lwActionTargetList kTargets)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		if (kPrevTargetPos.IsZero())
		{
			return;
		}

		lwPoint3 kDirection = kCurrentTargetPos.Subtract2(kPrevTargetPos);
		float fDistance = kDirection.Length();
		kDirection.Unitize();
		float fWideAngle = static_cast<float>(Skill_DeathSnatch_GetProjectileCollisionRadius(kAction));

		lwFindTargetParam kParam;
		kParam.SetParam_1(kPrevTargetPos, kDirection);
		kParam.SetParam_2(fDistance, fWideAngle, 0 ,0);
		kParam.SetParam_3(true, PgAction::FTO_BLOWUP);

		kAction.FindTargets(PgAction::TAT_BAR, kParam, kTargets, kTargets);
	}

	int Skill_DeathSnatch_GetProjectileCollisionRadius(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iParam2 = kAction.GetAbil(AT_2ND_AREA_PARAM2);
		if(0 == iParam2)
		{
			iParam2 = 50;
		}
		return	iParam2;
	}

	//------------------------------------------------
	//// Parameter Set/Get Functions
	//------------------------------------------------

	void Skill_DeathSnatch_SetActionState(lwAction kAction, int iState)
	{
		kAction.SetParamInt(0, iState);
	}

	int Skill_DeathSnatch_GetActionState(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamInt(0);
	}

	void Skill_DeathSnatch_SetTargetCount(lwAction kAction, int iTargetCount)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamInt(2, iTargetCount);
	}

	int Skill_DeathSnatch_GetTargetCount(lwAction kAction)
	{
		return	kAction.GetParamInt(2);
	}

	void Skill_DeathSnatch_SetFlyAngle(lwAction kAction, float fFlyAngle)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamFloat(4, fFlyAngle);
	}

	float Skill_DeathSnatch_GetFlyAngle(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamFloat(4);
	}

	void Skill_DeathSnatch_SetHandClassNo(lwAction kAction, int iHandClassNo)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamInt(5, iHandClassNo);
	}

	int Skill_DeathSnatch_GetHandClassNo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamInt(5);
	}

	void Skill_DeathSnatch_SetHandCount(lwAction kAction, int iHandCount)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamInt(6, iHandCount);
	}

	int Skill_DeathSnatch_GetHandCount(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamInt(6);
	}

	void Skill_DeathSnatch_SetHandGapAngle(lwAction kAction,float fHandGapAngle)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamFloat(7, fHandGapAngle);
	}

	float Skill_DeathSnatch_GetHandGapAngle(lwAction kAction)
	{	
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamFloat(7);
	}

	void Skill_DeathSnatch_SetHideHandCreationAnimation(lwAction kAction, bool bHideHandCreationAnimation)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if (bHideHandCreationAnimation)
		{
			kAction.SetParamInt(8,1);
		}
		else
		{
			kAction.SetParamInt(8,0);
		}
	}

	bool Skill_DeathSnatch_GetHideHandCreationAnimation(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	(kAction.GetParamInt(8) == 1);
	}

	void Skill_DeathSnatch_SetHideWeaponEnable(lwAction kAction, bool bHideWeaponEnable)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if(bHideWeaponEnable)
		{
			kAction.SetParamInt(9,1);
		}
		else
		{
			kAction.SetParamInt(9,0);
		}
	}

	bool Skill_DeathSnatch_GetHideWeaponEnable(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	(kAction.GetParamInt(9) == 1);
	}

	void Skill_DeathSnatch_SetTargetGuid(lwAction kAction,lwGUID kTargetGuid)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParam(3, kTargetGuid.GetString());
	}

	lwGUID Skill_DeathSnatch_GetTargetGuid(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	lwGUID(kAction.GetParam(3));
	}

	void Skill_DeathSnatch_AddSkillActorGuid(lwAction kAction, lwGUID kSkillActorGuid)
	{	
		if( kAction.IsNil() )	{ lwSetBreak(); }
		int iCurrentNum = Skill_DeathSnatch_GetNumSkillActorGuid(kAction);
		int iSlotIndex = Skill_DeathSnatch_GetSkillActorGuidSlotIndex(iCurrentNum);

		kAction.SetParam(iSlotIndex, kSkillActorGuid.GetString());

		Skill_DeathSnatch_SetNumSkillActorGuid(kAction, iCurrentNum+1);
	}

	lwGUID Skill_DeathSnatch_GetSkillActorGuid(lwAction kAction, int iSkillActorIndex)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		int	iSlotIndex = Skill_DeathSnatch_GetSkillActorGuidSlotIndex(iSkillActorIndex);
		return	lwGUID(kAction.GetParam(iSlotIndex));

	}

	int Skill_DeathSnatch_GetSkillActorGuidSlotIndex(int iSkillActorIndex)
	{
		return	11+iSkillActorIndex;
	}

	int Skill_DeathSnatch_GetNumSkillActorGuid(lwAction kAction)
	{
		if( kAction.IsNil() )
		{ 
			lwSetBreak();
		}
		return	kAction.GetParamInt(10);
	}

	void Skill_DeathSnatch_SetNumSkillActorGuid(lwAction kAction, int iNum)
	{	
		if( kAction.IsNil() )	
		{ 
			lwSetBreak();
		}
		kAction.SetParamInt(10, iNum);
	}

	void Skill_DeathSnatch_SetWeaponHidden(lwAction kAction, bool bHidden)
	{
		if( kAction.IsNil() )	
		{ 
			lwSetBreak();	
		}

		if( bHidden )
		{
			kAction.SetParamInt(1,1);
		}
		else
		{
			kAction.SetParamInt(1,0);
		}
	}

	bool Skill_DeathSnatch_GetWeaponHidden(lwAction kAction)
	{
		if( kAction.IsNil() )	
		{ 
			lwSetBreak();
		}
		return	(kAction.GetParamInt(1) == 1);
	}

	// Hand
	void Skill_DeathSnatchHandMove_SetParentActorGuid(lwAction kAction,lwGUID kParentActorGuid)
	{
		if( kAction.IsNil() )	
		{ 
			lwSetBreak();
		}
		kAction.SetParam(7, kParentActorGuid.GetString());
	}

	void Skill_DeathSnatchHandMove_SetShakeCamera(lwAction kAction, bool bShake)
	{
		if( kAction.IsNil() )	
		{ 
			lwSetBreak();
		}
		if(bShake)
		{
			kAction.SetParamInt(6,1);
		}
		else
		{
			kAction.SetParamInt(6,0);
		}
	}

	void Skill_DeathSnatchHandMove_SetHideCreationAnimation(lwAction kAction, bool bHideCreationAnimation)
	{
		if( kAction.IsNil() )
		{ 
			lwSetBreak();
		}

		if(bHideCreationAnimation)
		{
			kAction.SetParamInt(8,1);
		}
		else
		{
			kAction.SetParamInt(8,0);
		}
	}

	int Skill_DeathSnatchHandMove_GetCurrentActionState(lwAction kAction)
	{	
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		return kAction.GetParamInt(0);
	}

	void Skill_DeathSnatchHandMove_StartFlyingToTargetState(lwActor kActor,lwAction kAction)
	{
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}

		Skill_DeathSnatchHandMove_SetNextActionState(kAction, DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_TARGET);
		kActor.SetHide(false);
		kActor.SetHideShadow(false);
	}

	void Skill_DeathSnatchHandMove_SetFlyDirection(lwAction kAction, lwPoint3 kDirection)
	{
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		kAction.SetParamAsPoint(0, kDirection);
	}

	void  Skill_DeathSnatchHandMove_SetSkillRange(lwAction kAction, int iSkillRange)
	{
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		kAction.SetParamInt(5, iSkillRange);
	}

	void Skill_DeathSnatchHandMove_SetNextActionState(lwAction kAction, int iState)
	{
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		kAction.SetParamInt(1, iState);
	}

	lwPoint3 Skill_DeathSnatchHandMove_GetPrevActorPos(lwAction kAction)
	{
		if( kAction.IsNil() )
		{
			lwSetBreak();
		}
		return kAction.GetParamAsPoint(3);
	}

	//------------------------------------------------
	//-- Parameter Set/Get Functions
	//------------------------------------------------
	void Skill_DeathSnatchHandMove_SetCurrentActionState(lwAction kAction, int iState)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamInt(0,iState);
	}

	int Skill_DeathSnatchHandMove_GetNextActionState(lwAction kAction, int iState)
	{
		return kAction.GetParamInt(1);
	}

	void Skill_DeathSnatchHandMove_SetFlyStartTime(lwAction kAction,float fTime)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamFloat(2,fTime);
	}

	float Skill_DeathSnatchHandMove_GetFlyStartTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return kAction.GetParamFloat(2);
	}

	lwPoint3 Skill_DeathSnatchHandMove_GetFlyDirection(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return kAction.GetParamAsPoint(0);
	}

	void Skill_DeathSnatchHandMove_SetFlyStartPos(lwAction kAction, lwPoint3 kStartPos)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamAsPoint(1,kStartPos);
	}

	lwPoint3 Skill_DeathSnatchHandMove_GetFlyStartPos(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return kAction.GetParamAsPoint(1);
	}

	void Skill_DeathSnatchHandMove_SetFlyTargetPos(lwAction kAction, lwPoint3 kTargetPos)
	{

		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamAsPoint(2,kTargetPos);
	}

	lwPoint3 Skill_DeathSnatchHandMove_GetFlyTargetPos(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return kAction.GetParamAsPoint(2);
	}

	void Skill_DeathSnatchHandMove_SetPrevActorPos(lwAction kAction, lwPoint3 kPrevActorPosPos)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamAsPoint(3,kPrevActorPosPos);
	}

	void Skill_DeathSnatchHandMove_SetTargetCount(lwAction kAction, int iTargetCount)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParamInt(3,iTargetCount);
	}

	int Skill_DeathSnatchHandMove_GetTargetCount(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return kAction.GetParamInt(3);
	}

	void Skill_DeathSnatchHandMove_SetFirstTargetGuid(lwAction kAction, lwGUID kTargetGuid)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		kAction.SetParam(4,kTargetGuid.GetString());
	}

	lwGUID Skill_DeathSnatchHandMove_GetFirstTargetGuid(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	lwGUID(kAction.GetParam(4));

	}

	lwGUID Skill_DeathSnatchHandMove_GetParentActorGuid(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	lwGUID(kAction.GetParam(7));
	}

	int Skill_DeathSnatchHandMove_GetSkillRange(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	kAction.GetParamInt(5);
	}

	bool Skill_DeathSnatchHandMove_GetShakeCamera(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	(kAction.GetParamInt(6) == 1);

	}

	bool Skill_DeathSnatchHandMove_GetHideCreationAnimation(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwSetBreak(); }
		return	(kAction.GetParamInt(8) == 1);
	}

	//------------------------------------------------
	//-- Implementation Functions
	//------------------------------------------------

	void Skill_DeathSnatchHandMove_StartLoadingState(lwActor kActor,lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		Skill_DeathSnatchHandMove_SetCurrentActionState(kAction,DEATHSNATCHHANDMOVE_ACTIONSTATE_LOADING);
		Skill_DeathSnatchHandMove_SetNextActionState(kAction,DEATHSNATCHHANDMOVE_ACTIONSTATE_LOADING);

		if( Skill_DeathSnatchHandMove_GetHideCreationAnimation(kAction) )
		{
			kActor.SetHide(true);
			kActor.SetHideShadow(true);
		}
	}

	bool Skill_DeathSnatchHandMove_UpdateCurrentState(lwActor kActor,lwAction kAction,float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iCurrentState = Skill_DeathSnatchHandMove_GetCurrentActionState(kAction);
		//bool animDone = kActor.IsAnimationDone();
		//char const* nextActionName = kAction.GetNextActionName();

		switch(iCurrentState)
		{
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_LOADING:
			{
				if( Skill_DeathSnatchHandMove_UpdateLoadingPosition(kActor,kAction) == false )
				{
					return	false;
				}
			}break;
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_TARGET:
			{
				Skill_DeathSnatchHandMove_UpdateFlyingPosition(kActor,kAction,accumTime);
			}break;
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_ATTACHED_TO_TARGET:
			{
				Skill_DeathSnatchHandMove_AttachedPosition(kActor,kAction,accumTime);
			}break;
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_START:
			{
				Skill_DeathSnatchHandMove_UpdateFlyingBackPosition(kActor,kAction,accumTime);
			}break;
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_WAIT:
			{//-- nothing to do. Just wait.
			}break;
		case DEATHSNATCHHANDMOVE_ACTIONSTATE_FINISHED:
			{
				return	false;
			}break;
		}
		return	true;
	}

	void Skill_DeathSnatchHandMove_UpdateNextState(lwActor kActor,lwAction kAction, float accumTime)
	{	
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iNextState = Skill_DeathSnatchHandMove_GetNextActionState(kAction, 0);
		if(iNextState == DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_TARGET )
		{
			Skill_DeathSnatchHandMove_FlyToTarget(kActor,kAction,accumTime);
		}
		if(iNextState == DEATHSNATCHHANDMOVE_ACTIONSTATE_ATTACHED_TO_TARGET )
		{
			kActor.PlayNext();		
		}	
		if(iNextState == DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_START )
		{
			Skill_DeathSnatchHandMove_FlyBackToStart(kActor,kAction,accumTime);
		}

		if(iNextState != -1)
		{
			Skill_DeathSnatchHandMove_SetCurrentActionState(kAction,iNextState);
		}
		Skill_DeathSnatchHandMove_SetNextActionState(kAction,-1);
	}

	void Skill_DeathSnatchHandMove_Finish(lwActor kActor,lwAction kAction)
	{
		Skill_DeathSnatchHandMove_SetCurrentActionState(kAction,DEATHSNATCHHANDMOVE_ACTIONSTATE_FINISHED);
	}

	bool Skill_DeathSnatchHandMove_UpdateLoadingPosition(lwActor kActor,lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		lwActor kParentActor = Skill_DeathSnatchHandMove_GetParentActor(kAction);
		if( kParentActor.IsNil() )
		{
			Skill_DeathSnatchHandMove_Finish(kActor,kAction);
			return	false;
		}

		lwPoint3 kPos = kParentActor.GetNodeWorldPos("char_root");
		lwQuaternion kQuat = kParentActor.GetNodeWorldRotate("char_root");

		kPos.SetZ(kPos.GetZ()+10);

		kActor.SetTranslate(kPos, false);
		kActor.SetRotationQuat(kQuat);
		return true;
	}

	void Skill_DeathSnatchHandMove_UpdateFlyingPosition(lwActor kActor,lwAction kAction, float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		
		lwPoint3	vStartPos = Skill_DeathSnatchHandMove_GetFlyStartPos(kAction);
		
		float	fStartTime = Skill_DeathSnatchHandMove_GetFlyStartTime(kAction);

		int	iSkillRange = Skill_DeathSnatchHandMove_GetSkillRange(kAction);
		lwPoint3	vDir = Skill_DeathSnatchHandMove_GetFlyDirection(kAction);
		lwPoint3	vTargetPos = vStartPos.Add2(vDir.Multiply2(static_cast<float>(iSkillRange)));

		float	fElapsedTime = accumTime - fStartTime;
		float	fTotalDistance = vTargetPos.Distance(vStartPos);
		float	fTotalMoveTime = 0.1f;
		float	fTimeRate = fElapsedTime / fTotalMoveTime;
		if( fTimeRate>1.0f )
		{
			fTimeRate = 1.0f;
			if( Skill_DeathSnatchHandMove_GetNextActionState(kAction, 0) != DEATHSNATCHHANDMOVE_ACTIONSTATE_FLYING_TO_START )
			{
				Skill_DeathSnatchHandMove_SetNextActionState(kAction,DEATHSNATCHHANDMOVE_ACTIONSTATE_ATTACHED_TO_TARGET);
			}
			kActor.EndBodyTrail();
		}

		float	fCurrentDistance = fTotalDistance * fTimeRate;
		lwPoint3 vNextPos = vStartPos.Add2(vDir.Multiply2(fCurrentDistance));
		Skill_DeathSnatchHandMove_SetPrevActorPos(kAction,kActor.GetPos());
		kActor.SetTranslate(vNextPos, false);
	}

	void Skill_DeathSnatchHandMove_AttachedPosition(lwActor kActor,lwAction kAction, float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if(!g_pkWorld)
		{
			return;
		}
		lwGUID	kTargetGUID = Skill_DeathSnatchHandMove_GetFirstTargetGuid(kAction);
		lwActor	kTargetActor(static_cast<PgActor*>( g_pkWorld->FindObject(kTargetGUID()) ) );

		if( kTargetActor.IsNil() == false )
		{
			kActor.SetTranslate(kTargetActor.GetPos(), false);
		}
	}


	void Skill_DeathSnatchHandMove_FlyToTarget(lwActor kActor, lwAction kAction, float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		Skill_DeathSnatchHandMove_SetFlyStartPos(kAction, kActor.GetPos());
		Skill_DeathSnatchHandMove_SetFlyStartTime(kAction, accumTime);
		kActor.PlayNext();
		//--actor.StartBodyTrail("../Data/5_Effect/9_Tex/swift_01.tga",500,0,0);
	}

	void Skill_DeathSnatchHandMove_FlyBackToStart(lwActor kActor,lwAction kAction, float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		int iCurrentSlot = kAction.GetCurrentSlot();
		if (iCurrentSlot != 2 )
		{
			kAction.SetSlot(2);
			kActor.PlayCurrentSlot(false);
		}

		lwPoint3 vStartPos = Skill_DeathSnatchHandMove_GetFlyStartPos(kAction);
		Skill_DeathSnatchHandMove_SetFlyStartTime(kAction,accumTime);

		Skill_DeathSnatchHandMove_SetFlyStartPos(kAction,kActor.GetPos());
		Skill_DeathSnatchHandMove_SetFlyTargetPos(kAction,vStartPos);
	}

	void Skill_DeathSnatchHandMove_UpdateFlyingBackFinished(lwActor kActor,lwAction kAction)
	{
		Skill_DeathSnatchHandMove_SetNextActionState(kAction, DEATHSNATCHHANDMOVE_ACTIONSTATE_WAIT);

		if(Skill_DeathSnatchHandMove_GetShakeCamera(kAction) )
		{
			lwQuakeCamera(0.2f, 7.0f, 3.0f, 1.0f, 1.0f);
		}
	}

	void Skill_DeathSnatchHandMove_UpdateFlyingBackPosition(lwActor kActor,lwAction kAction, float accumTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }

		lwPoint3	vStartPos = Skill_DeathSnatchHandMove_GetFlyStartPos(kAction);
		lwPoint3	vTargetPos = Skill_DeathSnatchHandMove_GetFlyTargetPos(kAction);
		float	fStartTime = Skill_DeathSnatchHandMove_GetFlyStartTime(kAction);

		lwPoint3	vDir = vTargetPos.Subtract2(vStartPos);
		vDir.Unitize();

		float	fElapsedTime = accumTime - fStartTime;
		float	fTotalDistance = vTargetPos.Distance(vStartPos);
		float	fTotalMoveTime = 0.05f;
		float	fTimeRate = fElapsedTime / fTotalMoveTime;

		if( fTimeRate > 1.0f )
		{
			fTimeRate = 1.0f;
			Skill_DeathSnatchHandMove_UpdateFlyingBackFinished(kActor,kAction);
		}
		float fCurrentDistance = fTotalDistance * fTimeRate;
		lwPoint3 vNextPos = vStartPos.Add2(vDir.Multiply2(fCurrentDistance));
		kActor.SetTranslate(vNextPos, false);
	}

	lwActor Skill_DeathSnatchHandMove_GetParentActor(lwAction kAction)
	{
		if(!g_pkWorld)
		{
			return lwActor(NULL);
		}
		lwGUID	kActorGuid = Skill_DeathSnatchHandMove_GetParentActorGuid(kAction);
		if( kActorGuid.IsNil() )
		{
			return	lwActor(NULL);
		}
		lwActor	kActor(static_cast<PgActor*>( g_pkWorld->FindObject(kActorGuid()) ) );

		return	kActor;
	}

	//Finish 
	//------------------------------------------------
	//-- Implementation Functions
	//------------------------------------------------
	void Skill_DeathSnatchFinish_SetTargetsFromDeathSnatch(lwActor kActor, lwAction kAction, lwAction kDeathSnatchAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); }
		if( kAction.IsNil() )	{ lwSetBreak(); }
		if(!g_pkWorld)
		{
			return;
		}

		int	iTargetCount = kDeathSnatchAction.GetTotalGUIDInContainer();
		int	i = 0;

		kAction.ClearGUIDContainer();
		while (i<iTargetCount)
		{
			lwGUID kTargetGuid = kDeathSnatchAction.GetGUIDInContainer(i);
			lwActor kFindActor(static_cast<PgActor*>( g_pkWorld->FindObject(kTargetGuid()) ) );

			bool const bActorIsExist = (kFindActor.IsNil() == false);

			if(true == bActorIsExist)
			{
				kAction.AddNewGUID(kTargetGuid);
			}
			++i;
		}

	}

};
