#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILL_LWDEATHSNATCHSKILLSCRIPTHELPFUNC_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILL_LWDEATHSNATCHSKILLSCRIPTHELPFUNC_H
class lwActor;
class lwAction;
class lwGUID;
class lwPoint3;
class lwActionTargetList;

namespace lwDeathSnatchSkillScriptHelpFunc
{
	extern void RegisterWrapper(lua_State *pkState);
	
	//Death Snatch
	
	extern lwGUID Skill_DeathSnatch_GetFirstTargetGuid(lwActionTargetList kTargetList);
	extern void Skill_DeathSnatch_SetAllSkillActorFlyingToStart(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_SetSkillActorFlyingToStart(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern void Skill_DeathSnatch_FindTargetAtSkillActor(lwActor kActor, lwAction kAction, lwActionTargetList kTargets,int iSkillActorIndex);
	extern void Skill_DeathSnatch_ProcessTextKeyEvent_End(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_ProcessTextKeyEvent_Hit(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_ProcessTextKeyEvent_Fire(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_LoadScriptParams(lwActor kActor, lwAction kAction);
	extern 	void Skill_DeathSnatch_LoadScriptParam_FlyAngle(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_LoadScriptParam_HandClassNo(lwActor kActor, lwAction kAction);

	extern void Skill_DeathSnatch_LoadScriptParam_HandCount(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_LoadScriptParam_HandGapAngle(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_LoadScriptParam_HideWeapon(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_LoadScriptParam_HideHandCreationAnimation(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_FireAllHand(lwActor kActor, lwAction kAction);
	extern bool Skill_DeathSnatch_FireHand(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern bool Skill_DeathSnatch_CheckTargetExist(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_AddAllTargetGuidToGuidContainer(lwActor kActor, lwAction kAction);
	extern bool Skill_DeathSnatch_CheckTarget(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_Update_WaitHandReachTargetActionState(lwActor kActor, lwAction kAction, float accumTime, float frameTime);
	extern bool Skill_DeathSnatch_IsAllSkillActorReachedTarget(lwActor kActor, lwAction kAction);
	extern bool Skill_DeathSnatch_IsSkillActorReachedTarget(lwActor kActor, lwAction kAction, int iSkillActorIndex);
	extern bool Skill_DeathSnatch_Update_WaitDrawAnimDoneActionState(lwActor kActor, lwAction kAction, float accumTime, float frameTime);
	extern void Skill_DeathSnatch_HideWeapon(lwActor kActor, lwAction kAction, bool bHide);
	extern lwAction Skill_DeathSnatch_GetSkillActorAction(lwActor kActor, lwAction kAction, int iSkillActorIndex);
	extern lwActor Skill_DeathSnatch_GetSkillActor(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern void Skill_DeathSnatch_DeleteAllSkillActor(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_DeleteSkillActor(lwGUID kActorGuid);
	extern void Skill_DeathSnatch_Fire(lwActor kActor, lwAction kAction);
	extern int Skill_DeathSnatch_GetSkillRange(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_CreateAllSkillActor(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_AddSkillActor(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern lwPoint3 Skill_DeathSnatch_GetHandInitialPosition(lwActor kActor, lwAction kAction,int iSkillActorIndex);

	extern void Skill_DeathSnatch_HideWeapon(lwActor kActor, lwAction kAction, bool bHide);
	extern lwAction Skill_DeathSnatch_GetSkillActorAction(lwActor kActor, lwAction kAction, int iSkillActorIndex);
	extern lwActor Skill_DeathSnatch_GetSkillActor(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern void Skill_DeathSnatch_DeleteAllSkillActor(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_DeleteSkillActor(lwGUID kActorGuid);
	extern void Skill_DeathSnatch_Fire(lwActor kActor, lwAction kAction);
	extern int Skill_DeathSnatch_GetSkillRange(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_CreateAllSkillActor(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_AddSkillActor(lwActor kActor, lwAction kAction,int iSkillActorIndex);
	extern lwPoint3 Skill_DeathSnatch_GetHandInitialPosition(lwActor kActor, lwAction kAction,int iSkillActorIndex);

	extern lwPoint3 Skill_DeathSnatch_GetHandFlyDirection(lwActor kActor, lwAction kAction, int iSkillActorIndex);
	extern float Skill_DeathSnatch_GetHandHorizontalRotationAngle(lwActor kActor, lwAction kAction, int iSkillActorIndex);
	extern float Skill_DeathSnatch_GetHandVerticalRotationAngle(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_InitAllSkillActorAction(lwActor kActor, lwAction kAction);
	extern void Skill_DeathSnatch_InitSkillActorAction(lwActor kActor,lwAction kAction,lwActor kSkillActor);
	extern void Skill_DeathSnatch_FindTarget_OnCurrentTargetPos(lwActor kActor,lwAction kAction,lwPoint3 kCurrentTargetPos,lwActionTargetList kTargets);

	extern void Skill_DeathSnatch_FindTarget_FromPrevTargetPosToCurrentTargetPos(lwActor kActor, lwAction kAction, lwPoint3 kCurrentTargetPos, lwPoint3 kPrevTargetPos, lwActionTargetList kTargets);
	extern int Skill_DeathSnatch_GetProjectileCollisionRadius(lwAction kAction);
	extern void Skill_DeathSnatch_SetActionState(lwAction kAction, int iState);
	extern int Skill_DeathSnatch_GetActionState(lwAction kAction);
	extern void Skill_DeathSnatch_SetTargetCount(lwAction kAction, int iTargetCount);
	extern int Skill_DeathSnatch_GetTargetCount(lwAction kAction);
	extern void Skill_DeathSnatch_SetFlyAngle(lwAction kAction, float fFlyAngle);
	extern float Skill_DeathSnatch_GetFlyAngle(lwAction kAction);
	extern void Skill_DeathSnatch_SetHandClassNo(lwAction kAction, int iHandClassNo);
	extern int Skill_DeathSnatch_GetHandClassNo(lwAction kAction);
	extern void Skill_DeathSnatch_SetHandCount(lwAction kAction,int iHandCount);
	extern int Skill_DeathSnatch_GetHandCount(lwAction kAction);
	extern void Skill_DeathSnatch_SetHandGapAngle(lwAction kAction,float fHandGapAngle);
	extern float Skill_DeathSnatch_GetHandGapAngle(lwAction kAction);
	extern void Skill_DeathSnatch_SetHideHandCreationAnimation(lwAction kAction, bool bHideHandCreationAnimation);
	extern bool Skill_DeathSnatch_GetHideHandCreationAnimation(lwAction kAction);
	extern void Skill_DeathSnatch_SetHideWeaponEnable(lwAction kAction, bool bHideWeaponEnable);
	extern bool Skill_DeathSnatch_GetHideWeaponEnable(lwAction kAction);
	extern void Skill_DeathSnatch_SetTargetGuid(lwAction kAction,lwGUID kTargetGuid);
	extern lwGUID Skill_DeathSnatch_GetTargetGuid(lwAction kAction);


	extern void Skill_DeathSnatch_AddSkillActorGuid(lwAction kAction, lwGUID kSkillActorGuid);
	extern lwGUID Skill_DeathSnatch_GetSkillActorGuid(lwAction kAction,int iSkillActorIndex);
	extern int Skill_DeathSnatch_GetSkillActorGuidSlotIndex(int iSkillActorIndex);
	extern int Skill_DeathSnatch_GetNumSkillActorGuid(lwAction kAction);
	extern void Skill_DeathSnatch_SetNumSkillActorGuid(lwAction kAction, int iNum);
	extern void Skill_DeathSnatch_SetWeaponHidden(lwAction kAction, bool bHidden);
	extern bool Skill_DeathSnatch_GetWeaponHidden(lwAction kAction);

	// Hand
	extern void Skill_DeathSnatchHandMove_SetParentActorGuid(lwAction kAction,lwGUID kParentActorGuid);
	extern void Skill_DeathSnatchHandMove_SetShakeCamera(lwAction kAction, bool bShake);
	extern void Skill_DeathSnatchHandMove_SetHideCreationAnimation(lwAction kAction, bool bHideCreationAnimation);
	extern int Skill_DeathSnatchHandMove_GetCurrentActionState(lwAction kAction);
	extern void Skill_DeathSnatchHandMove_StartFlyingToTargetState(lwActor kActor,lwAction kAction);
	extern void Skill_DeathSnatchHandMove_SetFlyDirection(lwAction kAction, lwPoint3 kDirection);
	extern void Skill_DeathSnatchHandMove_SetSkillRange(lwAction kAction, int iSkillRange);
	extern void Skill_DeathSnatchHandMove_SetNextActionState(lwAction kAction, int iState);
	extern lwPoint3 Skill_DeathSnatchHandMove_GetPrevActorPos(lwAction kAction);

	extern void Skill_DeathSnatchHandMove_SetCurrentActionState(lwAction kAction, int iState);
	extern int Skill_DeathSnatchHandMove_GetNextActionState(lwAction kAction, int iState);
	extern void Skill_DeathSnatchHandMove_SetFlyStartTime(lwAction kAction,float fTime);
	extern float Skill_DeathSnatchHandMove_GetFlyStartTime(lwAction kAction);
	extern lwPoint3 Skill_DeathSnatchHandMove_GetFlyDirection(lwAction kAction);
	extern void Skill_DeathSnatchHandMove_SetFlyStartPos(lwAction kAction, lwPoint3 kStartPos);
	extern lwPoint3 Skill_DeathSnatchHandMove_GetFlyStartPos(lwAction kAction);
	extern void Skill_DeathSnatchHandMove_SetFlyTargetPos(lwAction kAction,lwPoint3 kTargetPos);
	extern lwPoint3 Skill_DeathSnatchHandMove_GetFlyTargetPos(lwAction kAction);
	extern void Skill_DeathSnatchHandMove_SetPrevActorPos(lwAction kAction,lwPoint3 kPrevActorPosPos);
	extern void Skill_DeathSnatchHandMove_SetTargetCount(lwAction kAction,int iTargetCount);
	extern int Skill_DeathSnatchHandMove_GetTargetCount(lwAction kAction);
	extern void Skill_DeathSnatchHandMove_SetFirstTargetGuid(lwAction kAction, lwGUID kTargetGuid);
	extern lwGUID Skill_DeathSnatchHandMove_GetFirstTargetGuid(lwAction kAction);
	extern lwGUID Skill_DeathSnatchHandMove_GetParentActorGuid(lwAction kAction);
	extern int Skill_DeathSnatchHandMove_GetSkillRange(lwAction kAction);
	extern bool Skill_DeathSnatchHandMove_GetShakeCamera(lwAction kAction);
	extern 	bool Skill_DeathSnatchHandMove_GetHideCreationAnimation(lwAction kAction);

	extern void Skill_DeathSnatchHandMove_StartLoadingState(lwActor kActor,lwAction kAction);
	extern bool Skill_DeathSnatchHandMove_UpdateCurrentState(lwActor kActor,lwAction kAction,float accumTime);
	extern void Skill_DeathSnatchHandMove_UpdateNextState(lwActor kActor,lwAction kAction, float accumTime);
	extern void Skill_DeathSnatchHandMove_Finish(lwActor kActor,lwAction kAction);
	extern bool Skill_DeathSnatchHandMove_UpdateLoadingPosition(lwActor kActor,lwAction kAction);
	extern void Skill_DeathSnatchHandMove_UpdateFlyingPosition(lwActor kActor,lwAction kAction, float accumTime);
	extern void Skill_DeathSnatchHandMove_AttachedPosition(lwActor kActor,lwAction kAction, float accumTime);
	extern void Skill_DeathSnatchHandMove_FlyToTarget(lwActor kActor, lwAction kAction, float accumTime);
	extern void Skill_DeathSnatchHandMove_FlyBackToStart(lwActor kActor,lwAction kAction, float accumTime);
	extern void Skill_DeathSnatchHandMove_UpdateFlyingBackFinished(lwActor kActor,lwAction kAction);
	extern void Skill_DeathSnatchHandMove_UpdateFlyingBackPosition(lwActor kActor,lwAction kAction, float accumTime);
	extern lwActor Skill_DeathSnatchHandMove_GetParentActor(lwAction kAction);

	//Finish
	extern void Skill_DeathSnatchFinish_SetTargetsFromDeathSnatch(lwActor kActor, lwAction kAction, lwAction kDeathSnatchAction);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILL_LWDEATHSNATCHSKILLSCRIPTHELPFUNC_H