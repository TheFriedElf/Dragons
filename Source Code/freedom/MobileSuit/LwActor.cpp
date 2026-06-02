#include "stdafx.h"
#include "PgActorPet.h"
#include "lwActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNifMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgInput.h"
#include "PgSoundMan.h"
#include "mmsystem.h"
#include "Variant/PgPlayer.h"
#include "lwSkillTargetMan.h"
#include "lwQuaternion.h"
#include "lwPilot.H"
#include "PgActionEffect.H"
#include "PgWorkerThread.h"
#include "lwTrigger.H"
#include "PgWorld.H"
#include "PgQuestMan.h"
#include "PgMobileSuit.h"
#include "PgStat.h"
#include "PgHeadBuffIcon.H"
#include "PgActorMonster.h"
#include "PgSpotLight.H"
#include "PgDailyQuestUI.h"
#include "PgAction.h"
#include "lwActionTargetInfo.h"
#include "PgParticleProcessorCamFrontParticle.H"
#include "PgActionSlot.h"
#include "PgActorSubPlayer.h"

lwActor lwToActor(PgIWorldObject *pkObject)
{
	PgActor* pkActor = dynamic_cast<PgActor*>(pkObject);
	if (pkActor == NULL)
	{
		PG_RAISE_EXCEPTION;
	}
	return (lwActor)(pkActor);
}

lwGUID lwGetPilotGUID(PgIWorldObject *pkObject)
{
	if (pkObject && pkObject->GetPilot())
	{
		return (lwGUID)pkObject->GetPilot()->GetGuid();
	}
	else
	{
		return (lwGUID)BM::GUID::NullData();
	}
}

lwActor::lwActor(PgActor *pkActor)
{
	m_pkActor = pkActor;
}

lwActor lwGetMyActor()
{
	return lwActor(g_kPilotMan.GetPlayerActor());
}

lwActor lwGetActorByName(lwWString kPilotName)
{
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kPilotName());
	if(pkPilot)
	{
		PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
		return lwActor(pkActor);
	}
	return lwActor(NULL);		
}

int lwGetMyActorValue(int iAbilType)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		return pkPlayer->GetAbil(iAbilType);
	}
	return 0;		
}

void lwSetActorNameVisible(bool const bVisible)
{
	PgActor::SetNameVisible(bVisible);
}

bool lwGetActorNameVisible()
{
	return PgActor::GetNameVisible();
}


bool lwActor::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "GetMyActor", lwGetMyActor);
	def(pkState, "GetActorByName",lwGetActorByName);
	def(pkState, "GetMyActorValue", lwGetMyActorValue);
	def(pkState, "SetActorNameVisible", lwSetActorNameVisible);
	def(pkState, "GetActorNameVisible", lwGetActorNameVisible);

	class_<lwActor>(pkState, "Actor")
		.def(pkState, constructor<PgActor *>())
		.def(pkState, "Walk", &lwActor::Walk)
		.def(pkState, "StartSyncMove", &lwActor::StartSyncMove)
		.def(pkState, "UpdateSyncMove", &lwActor::UpdateSyncMove)
		.def(pkState, "Stop", &lwActor::Stop)
		.def(pkState, "StartJump", &lwActor::StartJump)
		.def(pkState, "StopJump", &lwActor::StopJump)
		.def(pkState, "AddForce", &lwActor::AddForce)
		.def(pkState, "AddVelocity", &lwActor::AddVelocity)
		.def(pkState, "GetVelocity", &lwActor::GetVelocity)
		.def(pkState, "GetMovingDir", &lwActor::GetMovingDir)
		.def(pkState, "SetMovingDir", &lwActor::SetMovingDir)
		.def(pkState, "GetAction", &lwActor::GetAction)
		.def(pkState, "IsAnimationDone", &lwActor::IsAnimationDone)
		.def(pkState, "IsAnimationLoop", &lwActor::IsAnimationLoop)
		.def(pkState, "HaveAnimationTextKey", &lwActor::HaveAnimationTextKey)
		.def(pkState, "CancelAction", &lwActor::CancelAction)
		.def(pkState, "IsAttachParticleSlot", &lwActor::IsAttachParticleSlot)
		.def(pkState, "AttachParticle", &lwActor::AttachParticle)
		.def(pkState, "AttachParticleS", &lwActor::AttachParticleS)
		.def(pkState, "AttachParticleWithRotate", &lwActor::AttachParticleWithRotate)
		.def(pkState, "AttachParticleToPoint", &lwActor::AttachParticleToPoint)
		.def(pkState, "AttachParticleToPointS", &lwActor::AttachParticleToPointS)
		.def(pkState, "AttachParticleToPointWithRotate", &lwActor::AttachParticleToPointWithRotate)
		.def(pkState, "ParticleStartAnimation", &lwActor::ParticleStartAnimation)
		.def(pkState, "AttachParticleToCameraFront", &lwActor::AttachParticleToCameraFront)		
		.def(pkState, "DetachFrom", &lwActor::DetachFrom)
		.def(pkState, "ChangeParticleGeneration", &lwActor::ChangeParticleGeneration)
		.def(pkState, "ReleaseAllParticles", &lwActor::ReleaseAllParticles)
		.def(pkState, "PlayNext", &lwActor::PlayNext)
		.def(pkState, "PlayPrev", &lwActor::PlayPrev)
		.def(pkState, "PlayCurrentSlot", &lwActor::PlayCurrentSlot)
		.def(pkState, "SeeFront", &lwActor::SeeFront)
		.def(pkState, "SeeLadder", &lwActor::SeeLadder)
		.def(pkState, "InitPhysical", &lwActor::InitPhysical)
		.def(pkState, "UpdatePhysicalSrc", &lwActor::UpdatePhysicalSrc)
		.def(pkState, "UpdatePhysicalDest", &lwActor::UpdatePhysicalDest)
		.def(pkState, "IsNil", &lwActor::IsNil)
		.def(pkState, "GetPilotGuid", &lwActor::GetPilotGuid)
		.def(pkState, "GetPilot", &lwActor::GetPilot)
		.def(pkState, "SetPilotGuid", &lwActor::SetPilotGuid)
		.def(pkState, "FindPathNormal", &lwActor::FindPathNormal)
		.def(pkState, "GetPathNormal", &lwActor::GetPathNormal)
		.def(pkState, "GetTranslate", &lwActor::GetTranslate)
		.def(pkState, "SetTranslate", &lwActor::SetTranslate)
		.def(pkState, "SetWorldTranslate", &lwActor::SetWorldTranslate)		
		.def(pkState, "SetClonePos", &lwActor::SetClonePos)
		.def(pkState, "GetAbil", &lwActor::GetAbil)
		.def(pkState, "SetAbil", &lwActor::SetAbil)		
		.def(pkState, "ActivateAnimation", &lwActor::ActivateAnimation)
		.def(pkState, "ClearAllActionEffect", &lwActor::ClearAllActionEffect)
		.def(pkState, "GetID", &lwActor::GetID)
		.def(pkState, "PhysXSync", &lwActor::PhysXSync)
		.def(pkState, "GetNodeTranslate", &lwActor::GetNodeTranslate)
		.def(pkState, "SetNodeScale", &lwActor::SetNodeScale)
		.def(pkState, "SetHide", &lwActor::SetHide)
		.def(pkState, "SetHideBalloon", &lwActor::SetHideBalloon)
		.def(pkState, "IsHide", &lwActor::IsHide)
		.def(pkState, "SetHideShadow", &lwActor::SetHideShadow)
		.def(pkState, "SetColor", &lwActor::SetColor)
		.def(pkState, "SetCanHit", &lwActor::SetCanHit)
		.def(pkState, "IsToLeft", &lwActor::IsToLeft)
		.def(pkState, "ToLeft", &lwActor::ToLeft)
		.def(pkState, "GetDirection", &lwActor::GetDirection)
		.def(pkState, "GetDirectionVector", &lwActor::GetDirectionVector)
		.def(pkState, "SetDirection", &lwActor::SetDirection)
		.def(pkState, "ContainsDirection", &lwActor::ContainsDirection)
		.def(pkState, "GetLastDirection", &lwActor::GetLastDirection)
		.def(pkState, "GetLookingDir", &lwActor::GetLookingDir)
		.def(pkState, "Blink", &lwActor::Blink)
		.def(pkState, "IsMeetFloor", &lwActor::IsMeetFloor)
		.def(pkState, "IsCheckMeetFloor", &lwActor::IsCheckMeetFloor)		
		.def(pkState, "IsSlide", &lwActor::IsSlide)
		.def(pkState, "SetUseSmoothShow", &lwActor::SetUseSmoothShow)		
		.def(pkState, "IsMeetSide", &lwActor::IsMeetSide)
		.def(pkState, "SetMeetFloor", &lwActor::SetMeetFloor)
		.def(pkState, "AttachSound", &lwActor::AttachSound)
		.def(pkState, "AttachSoundToPoint", &lwActor::AttachSoundToPoint)
		.def(pkState, "BackMoving", &lwActor::BackMoving)
		.def(pkState, "Throw", &lwActor::Throw)
		.def(pkState, "SetThrowStart", &lwActor::SetThrowStart)
		.def(pkState, "CheckCollWithCamera", &lwActor::CheckCollWithCamera)
		.def(pkState, "SetAnimSpeed", &lwActor::SetAnimSpeed)
		.def(pkState, "GetAnimSpeed", &lwActor::GetAnimSpeed)	
		.def(pkState, "GetActionState", &lwActor::GetActionState)
		.def(pkState, "ClearActionState", &lwActor::ClearActionState)
		.def(pkState, "ClearDirectionSlot", &lwActor::ClearDirectionSlot)
		.def(pkState, "MoveActor", &lwActor::MoveActor)
		.def(pkState, "FreeMove", &lwActor::FreeMove)
		.def(pkState, "SetNormalAttackEndTime", &lwActor::SetNormalAttackEndTime)
		.def(pkState, "CanNowConnectToNextComboAttack", &lwActor::CanNowConnectToNextComboAttack)
		.def(pkState, "IsMyActor", &lwActor::IsMyActor)
		//.def(pkState, "IsMyPet", &lwActor::IsMyPet)
		.def(pkState, "SetNormalAttackActionID", &lwActor::SetNormalAttackActionID)
		.def(pkState, "GetNormalAttackActionID", &lwActor::GetNormalAttackActionID)
		.def(pkState, "SetParticleAlphaGroup", &lwActor::SetParticleAlphaGroup)
		.def(pkState, "ShowDamageNum", &lwActor::ShowDamageNum)
		.def(pkState, "ShowSkillText", &lwActor::ShowSkillText)
		.def(pkState, "ShowSimpleText", &lwActor::ShowSimpleText)
		.def(pkState, "EquipAllItem", &lwActor::EquipAllItem)
		.def(pkState, "ShowChatBalloon", &lwActor::ShowChatBalloon)
		.def(pkState, "OnClickSay", &lwActor::OnClickSay)
		.def(pkState, "PlaySlotSound", &lwActor::PlaySlotSound)
		.def(pkState, "PushActor", &lwActor::PushActor)
		.def(pkState, "PushActorDir", &lwActor::PushActorDir)
		.def(pkState, "GetNowPush", &lwActor::GetNowPush)
		.def(pkState, "SetTargetScale", &lwActor::SetTargetScale)
		.def(pkState, "SetCameraFocus", &lwActor::SetCameraFocus)
		.def(pkState, "SetOriginalActor", &lwActor::SetOriginalActor)
		.def(pkState, "GetOriginalActor", &lwActor::GetOriginalActor)
		.def(pkState, "GetNodeWorldRotate", &lwActor::GetNodeWorldRotate)
		.def(pkState, "SetRotationQuat", &lwActor::SetRotationQuat)
		.def(pkState, "GetEquippedWeaponType", &lwActor::GetEquippedWeaponType)
		.def(pkState, "SetRotation", &lwActor::SetRotation)
		.def(pkState, "SetTargetAnimation", &lwActor::SetTargetAnimation)
//		.def(pkState, "HangItOn", &lwActor::HangItOn)
//		.def(pkState, "HangOnRope", &lwActor::HangOnRope)
		.def(pkState, "ClimbUpLadder", &lwActor::ClimbUpLadder)
//		.def(pkState, "SwitchPhysical", &lwActor::SwitchPhysical)
		.def(pkState, "Concil", &lwActor::Concil)
		.def(pkState, "GetCanHit", &lwActor::GetCanHit)
		.def(pkState, "SetPickupScript", &lwActor::SetPickupScript)
		.def(pkState, "SetMouseOverScript", &lwActor::SetMouseOverScript)
		.def(pkState, "SetMouseOutScript", &lwActor::SetMouseOutScript)
		.def(pkState, "StartWeaponTrail", &lwActor::StartWeaponTrail)
		.def(pkState, "EndWeaponTrail", &lwActor::EndWeaponTrail)
		.def(pkState, "StartBodyTrail", &lwActor::StartBodyTrail)
		.def(pkState, "EndBodyTrail", &lwActor::EndBodyTrail)
		.def(pkState, "StartTrail", &lwActor::StartTrail)
		.def(pkState, "EndTrail", &lwActor::EndTrail)
		//.def(pkState, "ClearQuestSimpleInfoPool", &lwActor::ClearQuestSimpleInfoPool)
		//.def(pkState, "AddQuestSimpleInfo", &lwActor::AddQuestSimpleInfo)
		//.def(pkState, "UpdateQuestUserInfo", &lwActor::UpdateQuestUserInfo)
		.def(pkState, "HaveQuest", &lwActor::HaveQuest)
		.def(pkState, "HaveDailyQuest", &lwActor::HaveDailyQuest)
		.def(pkState, "HaveActivate", &lwActor::HaveActivate)
		.def(pkState, "HaveTalk", &lwActor::HaveTalk)
		.def(pkState, "HaveWarning", &lwActor::HaveWarning)
//		.def(pkState, "Teleport", &lwActor::Teleport)
		.def(pkState, "UpdateModel", &lwActor::UpdateModel)
//		.def(pkState, "OutOfSight", &lwActor::OutOfSight)
//		.def(pkState, "FollowActor", &lwActor::FollowActor)
//		.def(pkState, "CompareActorPosition", &lwActor::CompareActorPosition)
		.def(pkState, "SetSpeedScale", &lwActor::SetSpeedScale)
		.def(pkState, "GetSpeedScale", &lwActor::GetSpeedScale)
//		.def(pkState, "WillBeFall", &lwActor::WillBeFall)
//		.def(pkState, "GetPetMaster", &lwActor::GetPetMaster)
		.def(pkState, "UntransitAction", &lwActor::UntransitAction)
		//.def(pkState, "IsPet", &lwActor::IsPet)
		//.def(pkState, "GetMyPet", &lwActor::GetMyPet)
		//.def(pkState, "RideMyPet", &lwActor::RideMyPet)
		//.def(pkState, "IsRiding", &lwActor::IsRiding)
		.def(pkState, "LookAt", &lwActor::LookAt)
		.def(pkState, "LookAtBidirection", &lwActor::LookAtBidirection)
		.def(pkState, "IncRotate", &lwActor::IncRotate)
		.def(pkState, "IsJumping", &lwActor::IsJumping)
		.def(pkState, "GetJumpAccumHeight", &lwActor::GetJumpAccumHeight)
		.def(pkState, "GetJumpTime", &lwActor::GetJumpTime)
		.def(pkState, "ResetJumpAccumHeight", &lwActor::ResetJumpAccumHeight)
		.def(pkState, "DetachChild", &lwActor::DetachChild)
		.def(pkState, "GetHitPoint", &lwActor::GetHitPoint)
		.def(pkState, "GetComboCount", &lwActor::GetComboCount)
		.def(pkState, "SetComboCount", &lwActor::SetComboCount)
		.def(pkState, "StartNormalAttackFreeze", &lwActor::StartNormalAttackFreeze)
		.def(pkState, "StopNormalAttackFreeze", &lwActor::StopNormalAttackFreeze)
		.def(pkState, "GetNormalAttackFreezeElapsedTime", &lwActor::GetNormalAttackFreezeElapsedTime)
		.def(pkState, "GetPos", &lwActor::GetPos)
		//.def(pkState, "PopSavedQuestSimpleInfo", &lwActor::PopSavedQuestSimpleInfo)
		.def(pkState, "GetEquippedWeaponProjectileID", &lwActor::GetEquippedWeaponProjectileID)
		.def(pkState, "AddToDefaultItem", &lwActor::AddToDefaultItem)
		.def(pkState, "SetDefaultItem", &lwActor::SetDefaultItem)
		.def(pkState, "ReleasePhysX", &lwActor::ReleasePhysX)
		.def(pkState, "ReleaseABVShapes", &lwActor::ReleaseABVShapes)
		.def(pkState, "EquipItem", &lwActor::EquipItem)
		.def(pkState, "PlayWeaponSound", &lwActor::PlayWeaponSound)
		.def(pkState, "StartSkillCoolTime", &lwActor::StartSkillCoolTime)
		.def(pkState, "CutSkillCoolTime", &lwActor::CutSkillCoolTime)
		.def(pkState, "SkillCastingConfirmed", &lwActor::SkillCastingConfirmed)
		.def(pkState, "ResetAnimation", &lwActor::ResetAnimation)
		.def(pkState, "SetItemColor", &lwActor::SetItemColor)
		.def(pkState, "GetDefaultItem", &lwActor::GetDefaultItem)
		.def(pkState, "GetCastingSkillNo", &lwActor::GetCastingSkillNo)
		.def(pkState, "SetHeadScale", &lwActor::SetHeadScale)
		.def(pkState, "GetRemainSkillCoolTimeInQuickSlot", &lwActor::GetRemainSkillCoolTimeInQuickSlot)
		.def(pkState, "GetRemainSkillCoolTimeInQuickSlotFloat", &lwActor::GetRemainSkillCoolTimeInQuickSlotFloat)
		.def(pkState, "GetSkillTotalCoolTimeInQuickSlot", &lwActor::GetSkillTotalCoolTimeInQuickSlot)
		.def(pkState, "GetNodeWorldPos", &lwActor::GetNodeWorldPos)
		.def(pkState, "GetParticleNodeWorldPos", &lwActor::GetParticleNodeWorldPos)		
		.def(pkState, "SetNodeHide", &lwActor::SetNodeHide)
		.def(pkState, "SetNodeAlpha", &lwActor::SetNodeAlpha)
		.def(pkState, "SetNodeAlphaChange", &lwActor::SetNodeAlphaChange)
		.def(pkState, "HideParts", &lwActor::HideParts)
		.def(pkState, "HideNode", &lwActor::HideNode)
		.def(pkState, "GetPartsHideCnt", &lwActor::GetPartsHideCnt)
		.def(pkState, "Talk", &lwActor::Talk)
		.def(pkState, "GetABVShapeWorldPos", &lwActor::GetABVShapeWorldPos)
		.def(pkState, "SetBoss", &lwActor::SetBoss)
		.def(pkState, "GetHitABVCenterPos", &lwActor::GetHitABVCenterPos)
		.def(pkState, "SetGravity", &lwActor::SetGravity)
		.def(pkState, "SetMovingDelta", &lwActor::SetMovingDelta)
		.def(pkState, "GetMovingDelta", &lwActor::GetMovingDelta)
		.def(pkState, "SetEventScriptIDOnDie", &lwActor::SetEventScriptIDOnDie)
		.def(pkState, "GetEventScriptIDOnDie", &lwActor::GetEventScriptIDOnDie)
		.def(pkState, "IsSendBlowStatus", &lwActor::IsSendBlowStatus)
		.def(pkState, "SetSendBlowStatus", &lwActor::SetSendBlowStatus)
		.def(pkState, "SetAnimSpeedInPeriod", &lwActor::SetAnimSpeedInPeriod)
		.def(pkState, "SetAnimOriginalSpeed", &lwActor::SetAnimOriginalSpeed)
		.def(pkState, "SetShakeInPeriod", &lwActor::SetShakeInPeriod)
		.def(pkState, "GetDistance", &lwActor::GetDistance)
		.def(pkState, "AddEffect", &lwActor::AddEffect)
		.def(pkState, "GetHitObjectABVIndex", &lwActor::GetHitObjectABVIndex)
		//.def(pkState, "TwistActor", &lwActor::TwistActor)
		.def(pkState, "RestoreTwistedActor", &lwActor::RestoreTwistedActor)
		.def(pkState, "SetParam", &lwActor::SetParam)
		.def(pkState, "GetParam", &lwActor::GetParam)
		.def(pkState, "StartTeleJump", &lwActor::StartTeleJump)
		.def(pkState, "SetAdjustValidPos", &lwActor::SetAdjustValidPos)
		.def(pkState, "GetWalkingToTarget", &lwActor::GetWalkingToTarget)
		.def(pkState, "SetWalkingTarget", &lwActor::SetWalkingTarget)
		.def(pkState, "GetWalkingTargetLoc", &lwActor::GetWalkingTargetLoc)
		.def(pkState, "SetWalkingTargetLoc", &lwActor::SetWalkingTargetLoc)
		.def(pkState, "SetNoWalkingTarget", &lwActor::SetNoWalkingTarget)
		.def(pkState, "LockBidirection", &lwActor::LockBidirection)
		.def(pkState, "IsLockBidirection", &lwActor::IsLockBidirection)
		.def(pkState, "SetDownState", &lwActor::SetDownState)
		.def(pkState, "IsDownState", &lwActor::IsDownState)
		.def(pkState, "SetTotalDownTime", &lwActor::SetTotalDownTime)
		.def(pkState, "GetTotalDownTime", &lwActor::GetTotalDownTime)
		.def(pkState, "GetAnimationLength", &lwActor::GetAnimationLength)
//		.def(pkState, "ActivateLayerAnimation", &lwActor::ActivateLayerAnimation)
//		.def(pkState, "DeactivateLayerAnimation", &lwActor::DeactivateLayerAnimation)
		.def(pkState, "SetTargetAlpha", &lwActor::SetTargetAlpha)
		.def(pkState, "ResetLastFloorPos", &lwActor::ResetLastFloorPos)
		.def(pkState, "GetExistDieAnimation", &lwActor::GetExistDieAnimation)
		.def(pkState, "GetUseDieAnimation", &lwActor::GetUseDieAnimation)
		.def(pkState, "SetUseSubAnimation", &lwActor::SetUseSubAnimation)
		.def(pkState, "GetAnimationInfo", &lwActor::GetAnimationInfo)
		.def(pkState, "GetFloorLoc", &lwActor::GetFloorLoc)
		.def(pkState, "GetLastFloorPos", &lwActor::GetLastFloorPos)		
		.def(pkState, "UseSkipUpdateWhenNotVisible", &lwActor::UseSkipUpdateWhenNotVisible)
		.def(pkState, "SetActiveGrp", &lwActor::SetActiveGrp)
		.def(pkState, "SetForcePos", &lwActor::SetForceSync)
		.def(pkState, "ChangeItemModel", &lwActor::ChangeItemModel)
		.def(pkState, "RestoreItemModel", &lwActor::RestoreItemModel)
		.def(pkState, "Transformation", &lwActor::Transformation)
		.def(pkState, "RestoreTransformation", &lwActor::RestoreTransformation)
		.def(pkState, "SetUpdateScript", &lwActor::SetUpdateScript)
		.def(pkState, "ReserveTransitAction", &lwActor::ReserveTransitAction)
		.def(pkState, "ReserveTransitActionIgnoreCase", &lwActor::ReserveTransitActionIgnoreCase)
		.def(pkState, "ReserveTransitActionByActionNo", &lwActor::ReserveTransitActionByActionNo)
		.def(pkState, "GetReservedTransitAction", &lwActor::GetReservedTransitAction)
		.def(pkState, "StartGodTime", &lwActor::StartGodTime)
		.def(pkState, "IsGodTime", &lwActor::IsGodTime)		
		.def(pkState, "IsOnlyMoveAction", &lwActor::IsOnlyMoveAction)
		.def(pkState, "IsUnitType", &lwActor::IsUnitType)
		.def(pkState, "AddIgnoreEffect", &lwActor::AddIgnoreEffect)
		.def(pkState, "RemoveIgnoreEffect", &lwActor::RemoveIgnoreEffect)
		.def(pkState, "ClearIgnoreEffectList", &lwActor::ClearIgnoreEffectList)
		.def(pkState, "ResetActiveGrp", &lwActor::ResetActiveGrp)
		.def(pkState, "GetCurrentTrigger", &lwActor::GetCurrentTrigger)
		.def(pkState, "ApplyMovingObject_OnEnter", &lwActor::ApplyMovingObject_OnEnter)
		.def(pkState, "ApplyMovingObject_OnUpdate", &lwActor::ApplyMovingObject_OnUpdate)
		.def(pkState, "ApplyMovingObject_OnLeave", &lwActor::ApplyMovingObject_OnLeave)
		.def(pkState, "IsBlowUp", &lwActor::IsBlowUp)
		.def(pkState, "GetBlowUpStartTime", &lwActor::GetBlowUpStartTime)
		.def(pkState, "SetBlowUp", &lwActor::SetBlowUp)
		.def(pkState, "GetFloatHeight", &lwActor::GetFloatHeight)
		.def(pkState, "GetDieParticleID", &lwActor::GetDieParticleID)
		.def(pkState, "GetDieParticleNode", &lwActor::GetDieParticleNode)
		.def(pkState, "GetDieParticleScale", &lwActor::GetDieParticleScale)
		.def(pkState, "GetDieSoundID", &lwActor::GetDieSoundID)
//		.def(pkState, "SetAttackTarget", &lwActor::SetAttackTarget)
		.def(pkState, "AttachToObject", &lwActor::AttachToObject)
		.def(pkState, "DetachFromObject", &lwActor::DetachFromObject)
		.def(pkState, "DetachFromParent", &lwActor::DetachFromParent)
		.def(pkState, "ResetTransfomation", &lwActor::ResetTransfomation)
		.def(pkState, "IsTransformed", &lwActor::IsTransformed)
		.def(pkState, "IsAlphaTransitDone", &lwActor::IsAlphaTransitDone)
		.def(pkState, "ClearReservedAction", &lwActor::ClearReservedAction)
		.def(pkState, "CreateTempActionByActionNo", &lwActor::CreateTempActionByActionNo)
		.def(pkState, "GetTempAction", &lwActor::GetTempAction)
		.def(pkState, "RemoveTempAction", &lwActor::RemoveTempAction)
		.def(pkState, "CheckStatusEffectExist", &lwActor::CheckStatusEffectExist)
		.def(pkState, "CheckStatusEffectTypeExist", &lwActor::CheckStatusEffectTypeExist)
		.def(pkState, "CheckEffectExist", &lwActor::CheckEffectExist)
		.def(pkState, "CheckSkillExist", &lwActor::CheckSkillExist)		
		.def(pkState, "DoDropItems", &lwActor::DoDropItems)
		.def(pkState, "CutSkillCasting", &lwActor::CutSkillCasting)
		.def(pkState, "SetUnderMyControl", &lwActor::SetUnderMyControl)
		.def(pkState, "IsUnderMyControl", &lwActor::IsUnderMyControl)
		.def(pkState, "ActionToggleStateChange", &lwActor::ActionToggleStateChange)
		.def(pkState, "RestoreLockBidirection", &lwActor::RestoreLockBidirection)
		.def(pkState, "GetDirFromMovingVector", &lwActor::GetDirFromMovingVector)
		.def(pkState, "GetDirFromMovingVector8Way", &lwActor::GetDirFromMovingVector8Way)
		.def(pkState, "SetLookingDirection", &lwActor::SetLookingDirection)
		.def(pkState, "GetWalkingTargetDir", &lwActor::GetWalkingTargetDir)
		.def(pkState, "AddHeadBuffIcon", &lwActor::AddHeadBuffIcon)
		.def(pkState, "RemoveHeadBuffIcon", &lwActor::RemoveHeadBuffIcon)
		.def(pkState, "BeforeUse", &lwActor::BeforeUse)
		.def(pkState, "SetInstallTimerGauge", &lwActor::SetInstallTimerGauge)
		.def(pkState, "DestroyInstallTimerGauge", &lwActor::DestroyInstallTimerGauge)
		.def(pkState, "GetRotateQuaternion", &lwActor::GetRotateQuaternion)
		.def(pkState, "HideEquipItem", &lwActor::HideEquipItem)
		.def(pkState, "SetUpdatePhysXFrameTime", &lwActor::SetUpdatePhysXFrameTime)
		.def(pkState, "SetLookTarget", &lwActor::SetLookTarget)
		.def(pkState, "IsEqualObjectName", &lwActor::IsEqualObjectName)
		.def(pkState, "IsStun", &lwActor::IsStun)
		.def(pkState, "GetSpecificIdle", &lwActor::GetSpecificIdle)
		.def(pkState, "SetSpotLightColor", &lwActor::SetSpotLightColor)
		.def(pkState, "FindFollowingMeActor", &lwActor::FindFollowingMeActor)
		.def(pkState, "IsNowFollowing", &lwActor::IsNowFollowing)
		.def(pkState, "FollowActor", &lwActor::FollowActor)
		.def(pkState, "GetNodeRotateAxis", &lwActor::GetNodeRotateAxis)
		.def(pkState, "GetAnimationInfoFromAction", &lwActor::GetAnimationInfoFromAction)
		.def(pkState, "GetAnimationInfoFromAniName", &lwActor::GetAnimationInfoFromAniName)
		.def(pkState, "RequestFollowActor", &lwActor::RequestFollowActor)
		.def(pkState, "CheckCanFollow", &lwActor::CheckCanFollow)
		.def(pkState, "ResponseFollowActor", &lwActor::ResponseFollowActor)
		.def(pkState, "CopyEquipItem", &lwActor::CopyEquipItem)
		.def(pkState, "SetAutoDeleteActorTimer", &lwActor::SetAutoDeleteActorTimer)
		.def(pkState, "HasTarget", &lwActor::HasTarget)
		.def(pkState, "TraceFly", &lwActor::TraceFly)
		.def(pkState, "TraceGround", &lwActor::TraceGround)
		.def(pkState, "SetTraceFlyTargetLoc", &lwActor::SetTraceFlyTargetLoc)
		.def(pkState, "GetTraceFlyTargetLoc", &lwActor::GetTraceFlyTargetLoc)
		.def(pkState, "IsOnRidingObject", &lwActor::IsOnRidingObject)
		.def(pkState, "ChangeKFM", &lwActor::ChangeKFM)
		.def(pkState, "GetFollowingTargetGUID", &lwActor::GetFollowingTargetGUID)
		.def(pkState, "SetStartFollow", &lwActor::SetStartFollow)		
		.def(pkState, "AddFollowingMeActor", &lwActor::AddFollowingMeActor)
		.def(pkState, "SetUseLOD", &lwActor::SetUseLOD)
		.def(pkState, "AttachAttackEffect", &lwActor::AttachAttackEffect)
		.def(pkState, "GetDamageEffectID", &lwActor::GetDamageEffectID)
		.def(pkState, "GetAttackEffect", &lwActor::GetAttackEffect)
		.def(pkState, "GetStartParamID", &lwActor::GetStartParamID)
		.def(pkState, "GetStartEffectSave", &lwActor::GetStartEffectSave)
		.def(pkState, "GetEffectScale", &lwActor::GetEffectScale)
		.def(pkState, "GetStatusEffectParam", &lwActor::GetStatusEffectParam)
		.def(pkState, "SetStatusEffectParam", &lwActor::SetStatusEffectParam)
		.def(pkState, "ChangeParticleTexture", &lwActor::ChangeParticleTexture)
		.def(pkState, "GetNpcMenuStr", &lwActor::GetNpcMenuStr)
		.def(pkState, "GetAnimationStartTime", &lwActor::GetAnimationStartTime)
		.def(pkState, "SetAnimationStartTime", &lwActor::SetAnimationStartTime)
		.def(pkState, "GetCallerGuid", &lwActor::GetCallerGuid)
		.def(pkState, "IncTimeToAniObj", &lwActor::IncTimeToAniObj)
		.def(pkState, "MakePetActionQueue", &lwActor::MakePetActionQueue)
		.def(pkState, "UpdatePetActionQueue", &lwActor::UpdatePetActionQueue)
		.def(pkState, "ConcilDirection", &lwActor::ConcilDirection)
		.def(pkState, "GetHomeAddrTown", &lwActor::GetHomeAddrTown)
		.def(pkState, "GetHomeAddrHouse", &lwActor::GetHomeAddrHouse)
		.def(pkState, "AddSummonUnit", &lwActor::AddSummonUnit)
		.def(pkState, "GetSummonUnitCount", &lwActor::GetSummonUnitCount)
		.def(pkState, "GetMaxSummonUnitCount", &lwActor::GetMaxSummonUnitCount)
		.def(pkState, "DeleteSummonUnit", &lwActor::DeleteSummonUnit)
		.def(pkState, "GetSummonUnit", &lwActor::GetSummonUnit)
		.def(pkState, "ClearSummonUnit", &lwActor::ClearSummonUnit)		
		.def(pkState, "PickUpNearItem", &lwActor::PickUpNearItem)		
		.def(pkState, "SetAttackState", &lwActor::SetAttackState)		
		.def(pkState, "GetAttackState", &lwActor::GetAttackState)		
		.def(pkState, "GetActionToggleState", &lwActor::GetActionToggleState)
		.def(pkState, "FollowingHeadGuid", &lwActor::FollowingHeadGuid)
		.def(pkState, "GetGroggyRecoveryValue", &lwActor::GetGroggyRecoveryValue)
		.def(pkState, "IsInCoolTime", &lwActor::IsInCoolTime)
		.def(pkState, "GetSubPlayer", &lwActor::GetSubPlayer)
		.def(pkState, "GetCallerActor", &lwActor::GetCallerActor)
		.def(pkState, "GetAniSequenceID", &lwActor::GetAniSequenceID)
		.def(pkState, "GetMountTargetPet", &lwActor::GetMountTargetPet)
		.def(pkState, "IsRidingPet", &lwActor::IsRidingPet)
		.def(pkState, "MountPet", &lwActor::MountPet)
		.def(pkState, "UnmountPet", &lwActor::UnmountPet)

		.def(pkState, "GetAnimationTime", &lwActor::GetAnimationTime)
		.def(pkState, "GetDefaultHeadSize", &lwActor::GetDefaultHeadSize)
		.def(pkState, "SetDefaultHeadSize", &lwActor::SetDefaultHeadSize)
		.def(pkState, "SetTargetHeadSize", &lwActor::SetTargetHeadSize)
		.def(pkState, "SetDuelWinnerTitle", &lwActor::SetDuelWinnerTitle)
		.def(pkState, "SetAlpha", &lwActor::SetAlpha)
		.def(pkState, "GetAlpha", &lwActor::GetAlpha)
		.def(pkState, "IsHideNameTitle", &lwActor::IsHideNameTitle)
		.def(pkState, "SetHideNameTitle", &lwActor::SetHideNameTitle)
		.def(pkState, "CheckCanRidingAttack", &lwActor::CheckCanRidingAttack)
		;

	def(pkState, "ToActor", lwToActor);
	def(pkState, "GetPilotGUID", lwGetPilotGUID);

	return true;
}

short lwActor::GetHomeAddrTown()
{
	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot){return 0;}

	PgPlayer	*pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if(!pkPlayer){return 0;}

	return pkPlayer->HomeAddr().StreetNo();
}

int lwActor::GetHomeAddrHouse()
{
	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot){return 0;}

	PgPlayer	*pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if(!pkPlayer){return 0;}

	return pkPlayer->HomeAddr().HouseNo();
}

float	lwActor::GetEffectScale()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return m_pkActor->GetEffectScale();
}

void	lwActor::ChangeKFM(char const *strKFMPath,bool bChangeImmediately)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ReserveKFMTransit(strKFMPath,bChangeImmediately);
}

void	lwActor::RequestFollowActor(lwGUID kTargetActorGUID,int kMode, bool bForce)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->RequestFollowActor(kTargetActorGUID(),static_cast<EPlayer_Follow_Mode>(kMode), bForce);
}

bool	lwActor::CheckCanFollow(lwGUID kTargetActorGUID,bool const bMsg)const
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->CheckCanFollow(kTargetActorGUID(),bMsg);
}

void	lwActor::ResponseFollowActor(lwGUID kTargetActorGUID,int kMode)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ResponseFollowActor(kTargetActorGUID(),static_cast<EPlayer_Follow_Mode>(kMode));
}

void	lwActor::FollowActor(lwActor kTarget)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->FollowActor(kTarget()->GetPilotGuid());
}
bool	lwActor::FindFollowingMeActor(lwGUID kActorGUID)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->FindFollowingMeActor(kActorGUID());
}
bool	lwActor::IsNowFollowing()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsNowFollowing();
}
lwGUID lwActor::GetFollowingTargetGUID()
{
	if(!m_pkActor)
	{
		return lwGUID(NULL);
	}
	return (lwGUID)m_pkActor->GetFollowingTargetGUID();
}
void	lwActor::SetStartFollow(lwGUID kTargetActorGUID)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetFollowTargetActor(kTargetActorGUID());
}
void	lwActor::AddFollowingMeActor(lwGUID kGUID)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->AddFollowingMeActor(kGUID());
}
void	lwActor::SetSpotLightColor(int Red,int Green,int Blue)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetSpotLightColor(Red,Green,Blue);
}
bool	lwActor::IsStun()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsStun();
}

int		lwActor::GetSpecificIdle()
{
	if(!m_pkActor)
	{
		return 0;
	}
	PgPilot* pPilot = m_pkActor->GetPilot();
	if( !pPilot )
	{
		return 0;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return 0;
	}

	int const iNoIdle = pUnit->GetAbil(AT_SKILL_SPECIFIC_IDLE);
	
	return iNoIdle;
}

void	lwActor::SetInstallTimerGauge(float fTotalTime)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetInstallTimerGauge(fTotalTime);
}

void	lwActor::DestroyInstallTimerGauge()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->DestroyInstallTimerGauge();
}

void	lwActor::BeforeUse()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->BeforeUse();
}
void	lwActor::AddHeadBuffIcon(int iEffectNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->AddHeadBuffIcon(iEffectNo);
}
void	lwActor::RemoveHeadBuffIcon(int iEffectNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->RemoveHeadBuffIcon(iEffectNo);
}
bool	lwActor::HasTarget()
{
	if(!m_pkActor)
	{
		return false;
	}
	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot)	return	false;

	CUnit	*pkUnit = pkPilot->GetUnit();
	if(!pkUnit)	return	false;

	if(pkUnit->GetTarget() == BM::GUID::NullData())	return	false;

	return	true;

}
void	lwActor::CutSkillCasting()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->CutSkillCasting(0);
}
//!	수직 아래로 레이를 쏘아서 바닥으로부터 이 액터가 얼마나 떠 있는지를 반환한다.
float	lwActor::GetFloatHeight(float fRayLength)
{
	if(!g_pkWorld
		|| !m_pkActor
		) 
	{
		return 99999.0f;
	}
	PG_ASSERT_LOG(fRayLength > 0.0f);

	NiPoint3	kWorldPos = m_pkActor->GetPosition();

	NxExtendedVec3 kCharPos = NxExtendedVec3(kWorldPos.x,kWorldPos.y,kWorldPos.z);
	NxVec3 kCharactorPosition((NxReal)kCharPos.x, (NxReal)kCharPos.y, (NxReal)kCharPos.z);
	NxRay kRay(kCharactorPosition, NxVec3(0.0f, 0.0f, -1.0f));
	NxRaycastHit kHit;
	NxShape *pkHitShape = g_pkWorld->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, NiMax(fRayLength, 0.1f), NX_RAYCAST_SHAPE|NX_RAYCAST_DISTANCE);
	if(!pkHitShape) return	99999.0;

	return	kHit.distance;
}
void lwActor::DoDropItems(int iReqCount,float fJumpHeight)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->DoDropItems(-1,iReqCount,fJumpHeight);
}
bool	lwActor::IsBlowUp()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsBlowUp();
}
float	lwActor::GetBlowUpStartTime()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->GetBlowUpStartTime();
}
void	lwActor::SetBlowUp(bool bBlowUp)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetBlowUp(bBlowUp);
}
void	lwActor::ClearAllActionEffect()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ClearAllActionEffect();
}
//!	일정 시간동안 액터를 흔든다.
void	lwActor::SetShakeInPeriod(float fShakePower,int iPeriod)
{
	//	기획팀장님의 요청에 따라 캐릭 흔드는 것은 일단 막는다. by leesg213

	//	m_pkActor->SetShakeInPeriod(fShakePower,iPeriod);
}

bool lwActor::IsSendBlowStatus()
{
	if(m_pkActor)
	{
		return	m_pkActor->IsSendBlowStatus();
	}

	return false;
}
void lwActor::SetSendBlowStatus(bool bSend, bool bFirstDown, bool bNoUseStandUpTime)
{
	if(m_pkActor)
	{
		m_pkActor->SetSendBlowStatus(bSend, bFirstDown, bNoUseStandUpTime);
	}
}
bool lwActor::CheckStatusEffectExist(char const *strStatusEffectXMLID)
{
	if(m_pkActor)
	{
		return	m_pkActor->CheckStatusEffectExist(strStatusEffectXMLID);
	}

	return false;
}

bool lwActor::CheckEffectExist(int const iEffectNo, bool const bInGroup)
{
	if(m_pkActor)
	{
		return m_pkActor->CheckEffectExist(iEffectNo, bInGroup);
	}

	return false;
}

int lwActor::CheckSkillExist(int const iSkillNo)
{
	if(m_pkActor)
	{
		return m_pkActor->CheckSkillExist(iSkillNo);
	}

	return 0;
}

void	lwActor::StartSkillCoolTime(unsigned	long	ulSkillNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StartSkillCoolTime(ulSkillNo);
}
void	lwActor::CutSkillCoolTime(unsigned	long	ulSkillNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->CutSkillCoolTime(ulSkillNo);
}
void	lwActor::SkillCastingConfirmed(unsigned	long	ulSkillNo,short sErrorCode)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SkillCastingConfirmed(ulSkillNo,sErrorCode);
}

bool lwActor::GetSkillTreeNode(int const iQuickSlotIndex, PgSkillTree::stTreeNode *&pkNode)
{
	if(!m_pkActor)
	{
		return false;
	}
	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot){return false;}

	PgPlayer	*pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if(!pkPlayer){return false;}

	PgQuickInventory *pkQInven = pkPlayer->GetQInven();
	if(!pkQInven){return false;}

	SQuickInvInfo kQuickInvInfo;
	const HRESULT hRet = pkQInven->GetItem(pkQInven->AdjustSlotIDX(iQuickSlotIndex), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다

	if(S_OK != hRet
	|| ((kQuickInvInfo.Grp() != KUIG_SKILLTREE) && (kQuickInvInfo.Grp() != KUIG_SKILLTREE_PET)))
	{
		return false;
	}

	int const iSkillNo = kQuickInvInfo.ID();
	int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(iSkillNo);
	
	pkNode = g_kSkillTree.GetNode(iKeySkillNo, kQuickInvInfo.Grp() == KUIG_SKILLTREE_PET);
	if(!pkNode){return false;}
	
	return true;
}

int	lwActor::GetRemainSkillCoolTimeInQuickSlot(int iQuickSlotIndex)
{
	PG_STAT(PgStatTimerF timer(g_kActorStatGroup.GetStatInfo("lwActor.GetRemainSkillCoolTimeInQuickSlot"), g_pkApp->GetFrameCount()));
	PgSkillTree::stTreeNode *pkNode = NULL;
	if (!m_pkActor)
	{
		return 0;
	}

	PgActor::stSkillCoolTimeInfo *pkInfo = m_pkActor->GetSkillCoolTimeInfo();
	if (!pkInfo)
	{
		return 0;
	}

	if (pkInfo->m_CoolTimeInfoMap.empty())
	{
		return 0;
	}

	if(GetSkillTreeNode(iQuickSlotIndex, pkNode))
	{
		PgActor::stSkillCoolTimeInfo *pkInfo = m_pkActor->GetSkillCoolTimeInfo();
		int iSkillNo = pkNode->m_ulKeySkillNo;

		PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor = pkInfo->m_CoolTimeInfoMap.find(iSkillNo);
		if(itor != pkInfo->m_CoolTimeInfoMap.end())
		{
			int iRemainCoolTime = itor->second.m_ulTotalCoolTime - (BM::GetTime32() - itor->second.m_ulCoolStartTime);
			if(iRemainCoolTime<0) iRemainCoolTime = 0;

			int	iRemainCoolTimeInSec = iRemainCoolTime/1000;
			if(iRemainCoolTime%1000 !=0 ) iRemainCoolTimeInSec++;

			return	iRemainCoolTimeInSec;
		}
	}
	return	0;
}

float	lwActor::GetRemainSkillCoolTimeInQuickSlotFloat(int iQuickSlotIndex)
{
	PG_STAT(PgStatTimerF timer(g_kActorStatGroup.GetStatInfo("lwActor.GetRemainSkillCoolTimeInQuickSlotFloat"), g_pkApp->GetFrameCount()));
	PgSkillTree::stTreeNode *pkNode = NULL;

	if (!m_pkActor)
	{
		return 0;
	}

	PgActor::stSkillCoolTimeInfo *pkInfo = m_pkActor->GetSkillCoolTimeInfo();
	if (!pkInfo)
	{
		return 0;
	}

	if (pkInfo->m_CoolTimeInfoMap.empty())
	{
		return 0;
	}

	if(GetSkillTreeNode(iQuickSlotIndex, pkNode))
	{
		int iSkillNo = pkNode->m_ulKeySkillNo;

		PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor = pkInfo->m_CoolTimeInfoMap.find(iSkillNo);
		if(itor != pkInfo->m_CoolTimeInfoMap.end())
		{
			int iRemainCoolTime = itor->second.m_ulTotalCoolTime - (BM::GetTime32() - itor->second.m_ulCoolStartTime);
			if(iRemainCoolTime<0)
			{
				iRemainCoolTime = 0;
			}

			float	fRemainCoolTimeInFloat = (float)(iRemainCoolTime)/1000.0f;

			return	fRemainCoolTimeInFloat;
		}
	}

	return 0;
}

float	lwActor::GetSkillTotalCoolTimeInQuickSlot(int iQuickSlotIndex)
{
	PG_STAT(PgStatTimerF timer(g_kActorStatGroup.GetStatInfo("lwActor.GetSkillTotalCoolTimeInQuickSlot"), g_pkApp->GetFrameCount()));
	PgSkillTree::stTreeNode *pkNode = NULL;

	if (!m_pkActor)
	{
		return 0;
	}

	PgActor::stSkillCoolTimeInfo *pkInfo = m_pkActor->GetSkillCoolTimeInfo();
	if (!pkInfo)
	{
		return 0;
	}

	if (pkInfo->m_CoolTimeInfoMap.empty())
	{
		return 0;
	}

	if(GetSkillTreeNode(iQuickSlotIndex, pkNode))
	{
		int iSkillNo = pkNode->m_ulKeySkillNo;

		PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor = pkInfo->m_CoolTimeInfoMap.find(iSkillNo);
		if(itor != pkInfo->m_CoolTimeInfoMap.end())
		{
			return itor->second.m_ulTotalCoolTime/1000.0f;
		}
	}

	return 0;
}

int	lwActor::GetCastingSkillNo()
{
	if(!m_pkActor)
	{
		return 0;
	}
	PgActor::stSkillCastingInfo *pInfo = m_pkActor->GetSkillCastingInfo();
	return	pInfo->m_ulSkillNo;
}

char*	lwActor::GetEquippedWeaponProjectileID()
{
	if(!m_pkActor)
	{
		return "";
	}
	return	m_pkActor->GetEquippedWeaponProjectileID();
}

bool lwActor::Walk(int iDir, float fSpeed, float fFrameTime,bool bCorrectFinalPos)
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->Walk((Direction)iDir, fSpeed, fFrameTime,bCorrectFinalPos);
}

float lwActor::TraceFly(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, float fLimitZ, float fFloatHeight, bool bCanRotate)
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->TraceFly(fSpeed, fFrameTime, fLimitDistance, fAccelateScale, fLimitZ, fFloatHeight, bCanRotate);
}

float lwActor::TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate)
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->TraceGround(fSpeed, fFrameTime, fLimitDistance, fAccelateScale, bCanRotate);
}

void lwActor::SetTraceFlyTargetLoc(lwPoint3 kTargetLoc)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetTraceFlyTargetLoc( kTargetLoc() );
}

lwPoint3 lwActor::GetTraceFlyTargetLoc()
{
	if(m_pkActor)
	{
		return lwPoint3(m_pkActor->GetTraceFlyTargetLoc());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

//! 캐릭터를 민다.
void	lwActor::PushActor(bool bLeft,float fDistance,float fVelocity,float fAccel)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->PushActor(bLeft,fDistance,fVelocity,fAccel);
}

void	lwActor::PushActorDir(lwPoint3 kDir,float fDistance,float fVelocity,float fAccel)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->PushActor(kDir(),fDistance,fVelocity,fAccel);
}

bool	lwActor::GetNowPush()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->GetNowPush();
}

bool lwActor::IsJumping()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->GetJump();
}

//! 점프한 높이를  반환한다.
float lwActor::GetJumpAccumHeight()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->GetJumpAccumHeight();
}
//!	점프한 시간 
float	lwActor::GetJumpTime()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->GetJumpTime();
}
void lwActor::ResetJumpAccumHeight()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ResetJumpAccumHeight();
}

void lwActor::Stop()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->Stop();
}
void	lwActor::StartNormalAttackFreeze()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StartNormalAttackFreeze();
}
void	lwActor::StopNormalAttackFreeze()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StopNormalAttackFreeze();
}
int	lwActor::GetNormalAttackFreezeElapsedTime()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return	m_pkActor->GetNormalAttackFreezeElapsedTime();
}
short	lwActor::GetComboCount()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return	m_pkActor->GetComboCount();
}
void	lwActor::SetComboCount(short sCount)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetComboCount(sCount);
}

//! 변신 하기 전의 원래 액터를 설정한다.
void	lwActor::SetOriginalActor(lwActor Actor)
{
	if(Actor.m_pkActor && Actor.m_pkActor->GetPilot())
		m_pkActor->SetOriginalActorGUID(Actor.m_pkActor->GetPilot()->GetGuid());
}
lwActor	lwActor::GetOriginalActor()
{
	if(!m_pkActor)
	{
		return lwActor(NULL);
	}
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(m_pkActor->GetOriginalActorGUID());
	if(!pkPilot || !pkPilot->GetWorldObject())	return	lwActor(0);
		
	return	lwActor(dynamic_cast<PgActor *>(pkPilot->GetWorldObject()));
}
//!	무기 궤적 그리기 시작
void	lwActor::StartWeaponTrail()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StartWeaponTrail();
}
//!	무기	궤적	그리기	종료
void	lwActor::EndWeaponTrail()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->EndWeaponTrail();
}
void	lwActor::StartBodyTrail(char const* strTexPath, int iTotalTime, int iBrightTime)
{
	if(m_pkActor)
	{
		m_pkActor->StartBodyTrail(strTexPath, iTotalTime, iBrightTime);
	}
}

void	lwActor::EndBodyTrail()
{
	if(m_pkActor)
	{
		m_pkActor->EndBodyTrail();
	}
}

bool lwActor::StartTrail(int const iTrailType, char const* strTexPath, int iTotalTime, int iBrightTime)
{
	if(m_pkActor
		&& NULL != strTexPath
		)
	{
		return m_pkActor->StartTrail( static_cast<PgActor::eTrailAttachType>(iTrailType), strTexPath, iTotalTime, iBrightTime);
	}
	return false;
}
bool lwActor::EndTrail(int const iTrailType)
{
	if(m_pkActor)
	{
		return m_pkActor->EndTrail( static_cast<PgActor::eTrailAttachType>(iTrailType) );
	}
	return false;
}

//! 장비하고 있는 무기의 타입번호를 리턴한다.
int	lwActor::GetEquippedWeaponType()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return	m_pkActor->GetEquippedWeaponType();
}

void lwActor::StartJump(float fHeight)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetComboCount(0);

	m_pkActor->StartJump(fHeight);
}

void lwActor::StopJump()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StopJump();
}
//! 캐릭터의 Scale 을 변화시킨다.
void	lwActor::SetTargetScale(float fScale,unsigned long ulTotalScaleChangeTime)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetTargetScale(fScale,ulTotalScaleChangeTime);
}
float	lwActor::GetAnimationLength(char const*rkAnimationName)
{
	if(!m_pkActor)
	{
		return 0.0f;
	}

	if(rkAnimationName)
	{
		return	m_pkActor->GetAnimationLength(std::string(rkAnimationName));
	}
	return 0.0f;
}

char const* lwActor::GetAnimationInfo(char const* infoName,int iSeqID)
{
	if (infoName == NULL || m_pkActor == NULL)
		return NULL;

	if(iSeqID == 0)
	{
		iSeqID = m_pkActor->GetAniSequenceID();
	}

	std::string info;

	if (m_pkActor->GetAnimationInfo(std::string(infoName), iSeqID, info) == false)
	{
		return NULL;
	}

	// TODO: PgActionSlot안에 있는 AnimationInfo에서 String을 가져오는데, return 하면서 local value가 사라져서 일단 이렇게 땜빵을;;
	static std::string returnString;
	returnString = info;
	return returnString.c_str();
}

char const* lwActor::GetAnimationInfoFromAction(char const* infoName, lwAction pkNowAction, int iSeqID)
{
	if (infoName == NULL || pkNowAction.IsNil() )
	{
		return NULL;
	}
	
	if(0==iSeqID)
	{
		iSeqID = m_pkActor->GetAniSequenceID();
	}

	std::string info;

	if (m_pkActor->GetAnimationInfo(std::string(infoName), iSeqID, info, pkNowAction()) == false)
	{
		return NULL;
	}

	// TODO: PgActionSlot안에 있는 AnimationInfo에서 String을 가져오는데, return 하면서 local value가 사라져서 일단 이렇게 땜빵을;;
	static std::string returnString;
	returnString = info;
	return returnString.c_str();
}

lwWString lwActor::GetAnimationInfoFromAniName(char const* szInfoName, char const* szAniName)
{
	if(!m_pkActor)
	{
		return lwWString("");
	}

	if(szInfoName == NULL || szAniName == NULL)
	{
		return lwWString("");
	}

	std::string info;

	if(m_pkActor->GetActionSlot()->GetAnimationInfo(std::string(szAniName), 0,std::string(szInfoName), info))
	{
		return lwWString(info.c_str());
	}
	return lwWString("");
}

void lwActor::SetTargetAnimation(const char *pcAnimName)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetTargetAnimation(std::string(pcAnimName), true,false);
}

void lwActor::AddForce(lwPoint3 vForce)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->GetPhysXActor()->addForce(NxVec3(vForce.GetX(), vForce.GetY(), vForce.GetZ()));
}

void lwActor::SetNodeHide(char const* strNodeName, bool bHide)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetNodeHide(strNodeName, bHide);
}

void lwActor::SetNodeAlpha(char const* strNodeName, float fAlpha)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetNodeAlpha(strNodeName, fAlpha);
}

void lwActor::SetNodeAlphaChange(char const* strNodeName, float fAlphaStart, float fAlphaEnd, float fChangeTime)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetNodeAlphaChange(strNodeName, fAlphaStart, fAlphaEnd, fChangeTime);
}

//! 특정 Nif 노드의 월드 좌표를 리턴한다.
lwPoint3	lwActor::GetNodeWorldPos(char const* strNodeName)
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	NiAVObjectPtr	spNode = m_pkActor->GetCharRoot()->GetObjectByName(strNodeName);
	if(spNode == NULL)
		return	lwPoint3(0.0f,0.0f,0.0f);
	return	lwPoint3(spNode->GetWorldTranslate());
}
lwQuaternion	lwActor::GetNodeWorldRotate(char const* strNodeName)
{
	if(!m_pkActor)
	{
		return lwQuaternion(0.0f,0.0f,0.0f,0.0f);
	}
	NiAVObjectPtr	spNode = m_pkActor->GetCharRoot()->GetObjectByName(strNodeName);
	if(spNode == NULL)
	{
		return	lwQuaternion(0.0f,0.0f,0.0f,0.0f);
	}

	NiMatrix3	const &kRotateMat = spNode->GetWorldRotate();
	NiQuaternion	kQuat;
	kQuat.FromRotation(kRotateMat);

	return	lwQuaternion(kQuat);
}
bool lwActor::SetRotationQuat(lwQuaternion kQuat)
{
	if(!m_pkActor)
	{
		return false;
	}
	m_pkActor->SetRotation(kQuat());
	return	true;
}
lwPoint3	lwActor::GetParticleNodeWorldPos(int iSlot, char *strNodeName)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return lwPoint3(0.0f,0.0f,0.0f);

	return lwPoint3(m_pkActor->GetParticleNodeWorldPos(iSlot, strNodeName));
}

void lwActor::ChangeParticleTexture(int const iSlot, char const* pcParticle, char const* TexNode, int const GeoNo, char const* pcTexture)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return;

	NiAVObject* pkParticle = m_pkActor->GetParticleNode(iSlot, pcParticle);
	if(!pkParticle)
		return;

	NiNode* pNode = NiDynamicCast(NiNode, pkParticle->GetObjectByName(TexNode));
	if( !pNode )
		return;

	NiSourceTexturePtr spChangeTargetTex = g_kNifMan.GetTexture(pcTexture);
	if( !spChangeTargetTex )
		return;

	NiGeometry* pkTexGeom = NiDynamicCast(NiGeometry, pNode->GetAt(GeoNo));
	if( !pkTexGeom )
		return;

	NiPropertyStatePtr pkPropertyState = pkTexGeom->GetPropertyState();
	if( !pkPropertyState )
		return;

	NiTexturingProperty* pkTexProperty = pkPropertyState->GetTexturing();
	if( !pkTexProperty )
		return;

	pkTexProperty->SetBaseTexture( spChangeTargetTex );
	pkParticle->UpdateProperties();
	pkParticle->UpdateEffects();
	pkParticle->Update(0.0f);
}

lwPoint3	lwActor::GetNodeRotateAxis(char *strNodeName, int iAxis, bool bWorld)
{
	if(strNodeName == NULL
		|| !m_pkActor
		)
	{
		return	lwPoint3(0,0,0);
	}
	NiAVObjectPtr	spNode = m_pkActor->GetNIFRoot()->GetObjectByName(strNodeName);
	if(spNode == NULL)
	{
		return	lwPoint3(0,0,0);
	}

	iAxis = __min(iAxis, 2);
	iAxis = __max(iAxis, 0);

	NiPoint3 kCol(0,0,0);

	if (bWorld)
	{
		spNode->GetWorldRotate().GetRow(iAxis, kCol);
	}
	else
	{
		spNode->GetRotate().GetRow(iAxis, kCol);
	}

	kCol.Unitize();

	return lwPoint3(kCol);
}

//! 이 캐릭터를 카메라 포거스 캐릭터로 만든다.
void	lwActor::SetCameraFocus()
{
	if(m_pkActor) m_pkActor->SetCameraFocus();
}
void	lwActor::ResetAnimation()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ResetAnimation();
}
bool	lwActor::IsOnlyMoveAction()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsOnlyMoveAction();
}
bool	lwActor::IsUnitType(int iUnitType)
{
	if(!m_pkActor)
	{
		return false;
	}
	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot)
	{
		return	false;
	}

	CUnit	*pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return	false;
	}

	return	pkUnit->IsUnitType(static_cast<EUnitType>(iUnitType));
}
//!	현재 트리거 반환
lwTrigger	lwActor::GetCurrentTrigger()
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return	lwTrigger(m_pkActor->GetCurrentTrigger());
}

//! 움직이는 물체에 관한 캐릭터 이동.
bool lwActor::ApplyMovingObject_OnEnter(lwTrigger kTrigger)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ApplyMovingObject_OnEnter(kTrigger.GetSelf());
}
bool lwActor::ApplyMovingObject_OnUpdate(lwTrigger kTrigger)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ApplyMovingObject_OnUpdate(kTrigger.GetSelf());
}
bool lwActor::ApplyMovingObject_OnLeave(lwTrigger kTrigger)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ApplyMovingObject_OnLeave(kTrigger.GetSelf());
}

void lwActor::AddVelocity(lwPoint3 kVel)
{
	if(!m_pkActor)
	{
		return;
	}
	NxActor *pkPhysXActor = m_pkActor->GetPhysXActor();
	PG_ASSERT_LOG(pkPhysXActor);

	NxVec3 kCurVel = pkPhysXActor->getLinearVelocity();
	
	// xxxxxxxxxxxx TODO : 보다 졍교한 로직
	// 게임적 연출을 위해서 기존의 z축 가속도를 0으로 하고 더하자
	kCurVel.z = 0;

	NxVec3 kAddVel;
	NiPhysXTypes::NiPoint3ToNxVec3(kVel(), kAddVel);

//	pkPhysXActor->setLinearVelocity(kCurVel + kAddVel);
	m_pkActor->StartJump(kVel.GetZ());
}
lwPoint3	lwActor::GetPos()
{
	if(m_pkActor)
	{
		return	lwPoint3(m_pkActor->GetPos());
	}
	return lwPoint3(0,0,0);
}

lwPoint3 lwActor::GetMovingDir()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f, 0.0f, 0.0f);
	}
	return (lwPoint3)m_pkActor->GetMovingDir();
}
void lwActor::SetMovingDir(lwPoint3 kDir)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetMovingDir(kDir());
}
void	lwActor::SetMovingDelta(lwPoint3 kDelta)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetMovingDelta( NxVec3(kDelta.GetX(), kDelta.GetY(), kDelta.GetZ()) );
}
lwPoint3	lwActor::GetMovingDelta()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f, 0.0f, 0.0f);
	}
	return	lwPoint3( m_pkActor->GetMovingDelta().x, m_pkActor->GetMovingDelta().y, m_pkActor->GetMovingDelta().z );
}
bool lwActor::IsAnimationDone()
{
	if(m_pkActor)
	{
		return m_pkActor->IsAnimationDone();
	}

	return false;
}
bool lwActor::IsAnimationLoop() const
{
	if(m_pkActor)
	{
		return m_pkActor->IsAnimationLoop();
	}

	return false;
}
bool lwActor::HaveAnimationTextKey(char const* szKey, char const* szAnimationName) const
{
	if(m_pkActor)
	{
		return m_pkActor->HaveAnimationTextKey(szKey, szAnimationName);
	}

	return false;
}
bool lwActor::IsMyActor()
{
	if(m_pkActor)
	{
		return m_pkActor->IsMyActor();
	}

	return false;
}

bool lwActor::IsMySubPlayer()
{
	if(m_pkActor)
	{
		return m_pkActor->IsMySubPlayer();
	}

	return false;
}
//bool lwActor::IsMyPet()
//{
//	return m_pkActor->IsMyPet();
//}
//!	줄 수 있는 퀘스트 정보 업데이트
//void	lwActor::ClearQuestSimpleInfoPool()
//{
//	m_pkActor->ClearQuestInfo();
//}
//void	lwActor::AddQuestSimpleInfo(short sQuestID,char byState)
//{
//	m_pkActor->AddQuestSimpleInfo(sQuestID,(BYTE)byState);
//}
//void	lwActor::PopSavedQuestSimpleInfo()	//	퀘스트 매니저가 저장하고 있는 퀘스트 정보를 가져온다.
//{
//	m_pkActor->PopSavedQuestSimpleInfo();
//}

//!	진행중인 퀘스트 정보 업데이트
//void	lwActor::UpdateQuestUserInfo(short sQuestID,BYTE byQuestState,unsigned long ulParamArrayAddr)
//{
//	BYTE	*pParamArray = (BYTE *)ulParamArrayAddr;
//	m_pkActor->UpdateQuestUserInfo(sQuestID,byQuestState,pParamArray);
//
//	SAFE_DELETE_ARRAY(pParamArray);
//}
bool lwActor::HaveQuest()
{
	if( !m_pkActor )
	{
		return false;
	}
	BM::GUID const& rkGuid = m_pkActor->GetGuid();
	size_t const iCountRet = g_kQuestMan.CanNPCQuestInfo(rkGuid);
	if( !iCountRet )
	{
		return false;
	}
	return true;
}

bool lwActor::HaveDailyQuest()
{
	if( !m_pkActor )
	{
		return false;
	}

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}

	BM::GUID const& rkGuid = m_pkActor->GetGuid();
	ContQuestID kDailyVec;
	g_kQuestMan.GetNpcDailyInfo(rkGuid, kDailyVec);
	if( kDailyVec.empty() )
	{
		return false;
	}

	ContNpcQuestInfo kNpcQuestVec;
	g_kQuestMan.PopNPCQuestInfo(rkGuid, kNpcQuestVec);

	size_t iDailyQuestCount = 0;
	ContQuestID::const_iterator loop_iter = kDailyVec.begin();
	while( kDailyVec.end() != loop_iter )
	{
		int const iQuestID = (*loop_iter);
		EQuestState const eState = DailyQuestUI::GetDailyQuestState(iQuestID, kNpcQuestVec, pkMyQuest);
		switch( eState )
		{
		case QS_Begin:
		case QS_End:
		case QS_Ing:
		case QS_Finished:
			{
				++iDailyQuestCount;
			}break;
		default:
			{
			}break;
		}
		++loop_iter;
	}
	return 0 != iDailyQuestCount;
}

bool lwActor::HaveActivate()
{
	if ( m_pkActor )
	{
		return m_pkActor->IsActivate();
	}
	return false;
}

bool lwActor::HaveTalk()
{
	if(m_pkActor)
	{
		return m_pkActor->IsHaveTalk();
	}

	return false;
}

bool lwActor::HaveWarning()
{
	if(m_pkActor)
	{
		return m_pkActor->IsHaveWarning();
	}

	return false;
}

void lwActor::SetNormalAttackActionID(char const* actionid)
{
	if(m_pkActor)
	{
		m_pkActor->SetNormalAttackActionID(actionid);
	}
}
char const* lwActor::GetNormalAttackActionID()
{
	if(!m_pkActor)
	{
		return "";
	}
	return (char*)m_pkActor->GetNormalAttackActionID();
}
//! Normal 공격의 종료 시간을 저장한다
void	lwActor::SetNormalAttackEndTime()
{
	if(m_pkActor)
	{
		return m_pkActor->SetNormalAttackEndTime();
	}
}

//! 현재 시각이 연타 가능 입력 시간 구간내인지 체크한다.
bool lwActor::CanNowConnectToNextComboAttack(float fMaxTime)
{
	if(m_pkActor)
	{
		return m_pkActor->CanNowConnectToNextComboAttack(fMaxTime);
	}

	return false;
}

lwAction	lwActor::ReserveTransitAction(char const *kNextAction, BYTE byDir)
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return	lwAction(m_pkActor->ReserveTransitAction(kNextAction, byDir));
}

lwAction	lwActor::ReserveTransitActionIgnoreCase(char const *kNextAction, BYTE byDir)
{
	if(!m_pkActor)
	{
		return NULL; 
	}
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	int const iActionNo = kSkillDefMgr.GetSkillNoFromActionName(UNI(kNextAction), true);
	return	lwAction(m_pkActor->ReserveTransitAction(iActionNo, byDir));
}

lwAction	lwActor::ReserveTransitActionByActionNo(int iActionNo, BYTE byDir)
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return	lwAction(m_pkActor->ReserveTransitAction(iActionNo, byDir));
}

lwAction	lwActor::GetReservedTransitAction()
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return	lwAction(m_pkActor->GetReservedTransitAction());
}

void	lwActor::ClearReservedAction()
{
	if(!m_pkActor)
	{
		return;
	}
	return	m_pkActor->ClearReservedAction();
}

lwGUID lwActor::CreateTempActionByActionNo(int iActionNo)
{
	return m_pkActor ? lwGUID(m_pkActor->CreateTempAction(iActionNo)) : lwGUID("");
}

lwAction lwActor::GetTempAction(lwGUID kActionGuid)
{
	return m_pkActor ? m_pkActor->GetTempAction(kActionGuid()) : lwAction(NULL);
}

void lwActor::RemoveTempAction(lwGUID kActionGuid)
{
	if(m_pkActor)
	{
		m_pkActor->RemoveTempAction(kActionGuid());
	}
}

//!	현재 액션을 중단 시키고 pcNextActionName 으로 전이시킨다.
void	lwActor::CancelAction(int iActionID,int iActionInstanceID,char const *pcNextActionName)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->CancelAction(iActionID,iActionInstanceID,pcNextActionName, true);
}

void	lwActor::SetEventScriptIDOnDie(int iEventID)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetEventScriptIDOnDie(iEventID);
}
int		lwActor::GetEventScriptIDOnDie()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return	m_pkActor->GetEventScriptIDOnDie();
}

bool lwActor::UntransitAction(char const *pcNextActionName)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->TransitAction(pcNextActionName, false);
}

void	lwActor::ShowChatBalloon(char const *Text)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ShowChatBalloon(CT_NORMAL, Text);
}
bool	lwActor::OnClickSay()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->OnClickSay();
}

bool lwActor::IsAttachParticleSlot(int const iSlot)const
{
	return m_pkActor ? m_pkActor->IsAttachParticleSlot(iSlot) : false;
}

bool lwActor::AttachParticle(int iSlot, char const *pcTarget, char const *pcParticle )
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return false;

	return AttachParticleS( iSlot, pcTarget, pcParticle, m_pkActor->GetEffectScale() );
}

bool lwActor::AttachParticleS(int iSlot, char const *pcTarget, char const *pcParticle, float const fScale )
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return false;

	if(NULL == pcParticle || NULL == pcTarget || 0 == strlen(pcParticle) || 0 == strlen(pcTarget) )
		return	false;

	NiAVObject *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale );
	if(!pkParticle)
		return false;

	if(!m_pkActor->AttachTo(iSlot, pcTarget, (NiAVObject *)pkParticle))
	{
		THREAD_DELETE_PARTICLE(pkParticle);
		return	false;
	}

	return true;
}

bool lwActor::AttachParticleWithRotate(int iSlot, char const *pcTarget, const char *pcParticle, float fScale, bool NoFollowParentRotation)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
	{
		return false;
	}

	if (0>=fScale)
	{
		fScale = m_pkActor->GetEffectScale();
	}

	NiAVObject *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale);
	if (!pkParticle)
	{
		return false;
	}

	PgParticle* pkParticle2 = dynamic_cast<PgParticle*>(pkParticle);
	if(pkParticle2)
	{
		pkParticle2->SetNoFollowParentRotation(NoFollowParentRotation);
	}

	if(!m_pkActor->AttachTo(iSlot, pcTarget, (NiAVObject *)pkParticle))
	{
		THREAD_DELETE_PARTICLE(pkParticle);
	}

	return	true;
}

bool lwActor::AttachParticleToPointWithRotate(int iSlot, lwPoint3 kPoint, const char *pcParticle, lwQuaternion kQuat, float fScale)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
	{
		return false;
	}

	if (0>=fScale)
	{
		fScale = m_pkActor->GetEffectScale();
	}


	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale);
	if (!pkParticle)
	{
		return false;
	}
	
	pkParticle->SetRotate(kQuat());
	
	if(pkParticle->IsAutoGround())
	{
		pkParticle->SetOriginalRotate(pkParticle->GetRotate());
	}

	if(!m_pkActor->AttachToPoint(iSlot, kPoint(), (NiAVObject *)pkParticle))
	{
		THREAD_DELETE_OBJECT(pkParticle);
	}
	return true;
}

bool lwActor::ParticleStartAnimation(int iSlot, const char *pcParticle)
{
	if (m_pkActor == NULL || !g_pkWorld)
	{
		return false;
	}
	NiAVObject* pkParticle = m_pkActor->GetParticleNode(iSlot, pcParticle);
	if (!pkParticle )
	{
		return false;
	}

	PgWorld::SetAniType(pkParticle, NiTimeController::APP_INIT);
	NiTimeController::StartAnimations(pkParticle, g_pkWorld->GetAccumTime());

	return true;
}

bool lwActor::AttachParticleToPoint(int iSlot, lwPoint3 kPoint, char const *pcParticle )
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return false;

	return AttachParticleToPointS( iSlot, kPoint, pcParticle, m_pkActor->GetEffectScale() );
}

bool lwActor::AttachParticleToPointS(int iSlot, lwPoint3 kPoint, char const *pcParticle, float fScale )
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return false;

	NiAVObject *pkParticle = g_kParticleMan.GetParticle(pcParticle,PgParticle::O_SCALE, fScale );

	if (!pkParticle)
		return false;

	if(!m_pkActor->AttachToPoint(iSlot, kPoint(), (NiAVObject *)pkParticle))
	{
		THREAD_DELETE_OBJECT(pkParticle);
		return	false;
	}

	return true;
}

bool lwActor::AttachParticleToCameraFront(int iSlot, lwPoint3 kPos, const char *pcParticle, float fScale)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
	{
		return false;
	}

	if (0>fScale)
	{
		fScale = 0;
	}

	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle,PgParticle::O_SCALE, fScale);
	if (!pkParticle)
	{
		return false;
	}
	
	pkParticle->SetParticleProcessor(NiNew PgParticleProcessorCamFrontParticle(kPos()));
	if(!m_pkActor->AttachToPoint(iSlot, NiPoint3(0,0,0), (NiAVObject *)pkParticle))
	{
		THREAD_DELETE_OBJECT(pkParticle);
		return false;
	}
	return true;
}

char const* lwActor::GetStartParamID(char const *kStr)
{
	if(!m_pkActor)
	{
		return "";
	}
	return m_pkActor->GetStartParamID(kStr);
}

int const lwActor::GetStartEffectSave(char const *kStr)
{
	if(!m_pkActor)
	{
		return 0;
	}
	return m_pkActor->GetStartEffectSave(kStr);
}

bool lwActor::DetachFrom(int iSlot, bool bDefaultThreadDelete)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->DetachFrom(iSlot, bDefaultThreadDelete);
}

bool lwActor::ChangeParticleGeneration(int iSlot, bool bGenerate)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ChangeParticleGeneration(iSlot, bGenerate);
}
void lwActor::SetParticleAlphaGroup(int iSlot,int iAlphaGroup)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetParticleAlphaGroup(iSlot,iAlphaGroup);
}

//!	몬스터가 인지한 공격목표를 설정한다.
//void lwActor::SetAttackTarget(lwGUID kTargetGUID)
//{
//	m_pkActor->SetAttackTarget(kTargetGUID());
//}

lwPoint3 lwActor::GetVelocity()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f, 0.0f, 0.0f);
	}
	if(m_pkActor->GetPhysXActor() == NULL)
	{
		return	lwPoint3(0.0f,0.0f,0.0f);
	}

	NiPoint3 kVelocity;
	NiPhysXTypes::NxVec3ToNiPoint3(m_pkActor->GetPhysXActor()->getLinearVelocity(), kVelocity);

	return lwPoint3(kVelocity);
}

lwAction lwActor::GetAction()
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return lwAction(m_pkActor->GetAction());
}

bool lwActor::PlayNext()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->PlayNext();
}

bool lwActor::PlayPrev()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->PlayPrev();
}

bool lwActor::PlayCurrentSlot(bool bNoRandom)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->PlayCurrentSlot(bNoRandom);
}

void lwActor::SeeFront(bool bFront, bool bRightAway)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetLookingDirection((bFront ? DIR_DOWN : DIR_UP), bRightAway);
}
void lwActor::SeeLadder()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SeeLadder();
}

//! 주체를 AgeiaPhysX/Gamebryo 중 하나로 바꾼다.
void lwActor::InitPhysical(bool bIsPhysical)
{
	if (m_pkActor)
	{
		m_pkActor->InitPhysical(bIsPhysical);
	}
}

//! 주체가 바뀜으로써.. 주체가 아닌것이 Update가 안되는것을 강제로 해버릴 수 있다.
//! Source 를 업데이트 한다. (GameBryo 물체 업데이트)
void lwActor::UpdatePhysicalSrc(float fTime, bool bForce)
{
	if (m_pkActor && m_pkActor->GetPhysXScene())
	{
		m_pkActor->GetPhysXScene()->UpdateSources(fTime, bForce);
	}
}

//! Destination 를 업데이트 한다. (PhysX 물체 업데이트)
void lwActor::UpdatePhysicalDest(float fTime, bool bForce)
{
	if (m_pkActor && m_pkActor->GetPhysXScene())
	{
		m_pkActor->GetPhysXScene()->UpdateDestinations(fTime, bForce);
	}
}

bool lwActor::IsNil()
{
	return (m_pkActor == 0 ? true : false);
}

lwPilot	lwActor::GetPilot()
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return lwPilot(NULL);

	return	lwPilot(m_pkActor->GetPilot());
}

lwGUID lwActor::GetPilotGuid()
{
	if(!m_pkActor)
	{
		return NULL;
	}
	if(m_pkActor->GetPilot())
	{
		return (lwGUID)m_pkActor->GetPilot()->GetGuid();
	}

	return (lwGUID)BM::GUID();
}

void lwActor::SetPilotGuid(lwGUID kGuid)
{
	if(!m_pkActor)
	{
		return;
	}
	if(m_pkActor->GetPilot())
	{
		m_pkActor->GetPilot()->SetGuid(kGuid());
	}
}
//!	패스 노멀을 리턴한다.
lwPoint3	lwActor::GetPathNormal()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return	m_pkActor->GetPathNormal();
}
void lwActor::FindPathNormal(bool const bDoNotConcil)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->FindPathNormal(bDoNotConcil);
}

PgActor *lwActor::operator()()
{
	return m_pkActor;
}

bool lwActor::IsToLeft()
{
	if(!m_pkActor)
	{
		return false;
	}
	NiPoint3	kLookingDir = m_pkActor->GetLookingDir();
	return ((m_pkActor->GetPathNormal().Cross(kLookingDir).z>0) ? true : false);
}

void lwActor::ToLeft(bool bLeft,bool bTurnRightAway)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetLookingDirection((bLeft ? DIR_LEFT : DIR_RIGHT), bTurnRightAway);
}

bool lwActor::ContainsDirection(int iDir)
{
	if(!m_pkActor)
	{
		return false;
	}
	return (m_pkActor->GetDirection() & iDir) != 0;
}

int lwActor::GetDirection()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return m_pkActor->GetDirection();
}

void lwActor::SetDirection(int iDirection)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetDirection((Direction)iDirection);
}

lwPoint3 lwActor::GetDirectionVector(int iDirection)
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return lwPoint3(m_pkActor->GetDirectionVector(iDirection));
}

lwPoint3 lwActor::GetTranslate()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return lwPoint3(m_pkActor->GetTranslate());
}

lwPoint3 lwActor::GetFloorLoc()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return lwPoint3(m_pkActor->GetFloorLoc());
}

lwPoint3 lwActor::GetLastFloorPos()
{
	if(m_pkActor)
	{
		return lwPoint3(m_pkActor->GetLastFloorPos());
	}

	return lwPoint3(NiPoint3::ZERO);
}

void	lwActor::ActionToggleStateChange(int iActionNo,bool bOn)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ActionToggleStateChange(iActionNo,bOn);
}
void lwActor::SetTranslate(lwPoint3 loc,bool bDontUseController)
{
	if (m_pkActor)
	{
		NILOG(PGLOG_LOG, "[lwActor] SetTranslate, (%f, %f, %f) bDontUseController:%d\n", loc.GetX(), loc.GetY(), loc.GetZ(),bDontUseController);

		if(bDontUseController)
		{
			m_pkActor->SetTranslate(loc());
		}
		else
		{
			m_pkActor->SetPosition(loc());
		}
	}
}

void lwActor::SetWorldTranslate(lwPoint3 loc)
{
	if(m_pkActor)
	{
		m_pkActor->SetWorldTranslate(loc());
	}
}

void lwActor::SetClonePos(lwActor kActor)
{
	if ( m_pkActor && false == kActor.IsNil() )
	{
		NiQuaternion kQuat;
		kActor()->GetRotate(kQuat);
		m_pkActor->SetRotation(kQuat);
		m_pkActor->SetPosition(kActor()->GetTranslate());
	}
}

//! 대미지 숫자를 띄운다.
void	lwActor::ShowDamageNum(lwPoint3 attackerloc,lwPoint3 kTargetPos,int iDamage)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ShowDamageNum(attackerloc(),kTargetPos(),iDamage);
}

void lwActor::ShowSkillText(lwPoint3 kTargetPos,int iTextType,bool bUp)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ShowSkillText(kTargetPos(),iTextType,bUp);
}
void lwActor::ShowSimpleText(lwPoint3 kTargetPos,int iTextType)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ShowSimpleText(kTargetPos(),iTextType);
}
lwPoint3 lwActor::GetNodeTranslate(char const *pcNodeName)
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	NiAVObject *pkObj = m_pkActor->GetObjectByName(pcNodeName);

	if(!pkObj)
	{
		return (lwPoint3)m_pkActor->GetWorldTranslate();
	}

	return (lwPoint3)pkObj->GetWorldTranslate();
}
void	lwActor::SetNodeScale(char const *pcNodeName,float fScale)
{
	if(!m_pkActor)
	{
		return;
	}
	NiAVObject *pkObj = m_pkActor->GetObjectByName(pcNodeName);

	if(pkObj)
	{
		pkObj->SetScale(fScale);
	}
}

lwPoint3	lwActor::GetABVShapeWorldPos(int iIndex)
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return	lwPoint3(m_pkActor->GetABVShapeWorldPos(iIndex));
}
void	lwActor::SetDownState(bool bDownState)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetDownState(bDownState);
}
bool	lwActor::IsDownState()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsDownState();
}
void	lwActor::SetTotalDownTime(float fTime)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetTotalDownTime(fTime);
}
float	lwActor::GetTotalDownTime()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->GetTotalDownTime();
}

extern int g_iEventNo;
extern SEventRaceData g_kRaceEventInfo;
int lwActor::GetAbil(int iAbilType)
{
	if (m_pkActor == NULL)
		return 0;

	if(NULL != g_pkWorld && iAbilType == AT_C_MOVESPEED && g_pkWorld->GetAttr() == GATTR_RACE_GROUND)
	{ //달리기 이벤트 맵일 때..
		if(g_iEventNo != 0)
		{
			return g_kRaceEventInfo.MoveSpeed;
		}
	}

	int iValue = 0;
	if(m_pkActor->m_pkMonsterDef)
	{
		iValue = m_pkActor->m_pkMonsterDef->GetAbil(iAbilType);
	}

	if (iValue != 0)
	{
		return iValue;
	}

	if(m_pkActor->GetPilot())
	{
		iValue =  m_pkActor->GetPilot()->GetAbil(iAbilType);
		return iValue;
	}

	return	0;
}

void lwActor::SetAbil(int iAbilType, int iValue)
{
	if (m_pkActor == NULL)
		return ;
	PgPilot* pkPilot = m_pkActor->GetPilot();
	if(pkPilot)
	{
#ifdef EXTERNAL_RELEASE
		if( false == g_pkApp->IsSingleMode() )
		{
			if( m_pkActor->IsMyActor()
				|| m_pkActor->IsMyPet()
				)
			{
				MakeCrash_SetAbilFromLuaScript(iAbilType);
			}
		}
#endif
		pkPilot->SetAbil(iAbilType, iValue);
	}
}

bool lwActor::CheckCanRidingAttack(void)
{
	return (GetAbil(AT_PET_CANNOT_RIDING_ATTACK) == 0);
}

bool lwActor::ActivateAnimation()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ActivateAnimation(true);
}

char const *lwActor::GetID()
{
	if(!m_pkActor)
	{
		return NULL;
	}
	return ((PgIXmlObject *)m_pkActor)->GetID().c_str();
}
void lwActor::ReleasePhysX()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ReleasePhysX();
}
void lwActor::ReleaseABVShapes()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ReleaseABVShapes();
}
void lwActor::PhysXSync()
{
	if(!m_pkActor)
	{
		return;
	}
	/*
	NiAVObject *pkCharRoot = m_pkActor->GetObjectByName("char_root");
	NiPoint3 kLoc = pkCharRoot->GetWorldTranslate();

	NxVec3 kToLoc;
	NiPhysXTypes::NiPoint3ToNxVec3(kLoc, kToLoc);

	m_pkActor->GetPhysXActor()->setGlobalPosition(kToLoc);
	*/
}

void lwActor::SetHide(bool const bHide)
{
	if(!m_pkActor)
	{
		return;
	}
	PG_ASSERT_LOG(m_pkActor);
	m_pkActor->SetHide(bHide);
}
void lwActor::SetHideBalloon(bool const bHide)
{
	if(m_pkActor)
	{
		m_pkActor->SetHideBalloon(bHide);
	}
}
bool lwActor::IsHide()const
{
	if(m_pkActor)
	{
		return m_pkActor->IsHide();
	}
	return false;
}

void lwActor::SetHideShadow(bool const bHide)
{
	if(!m_pkActor)
	{
		return;
	}
	PG_ASSERT_LOG(m_pkActor);
	m_pkActor->SetHideShadow(bHide);
}

void lwActor::SetColor(float fR, float fG, float fB)
{
	if(!m_pkActor)
	{
		return;
	}
	PG_ASSERT_LOG(m_pkActor);
	NiColor kColor(fR, fG, fB);
	m_pkActor->SetColor(kColor);
}

void lwActor::SetCanHit(bool bCanHit)
{
	if(!m_pkActor)
	{
		return;
	}
	PG_ASSERT_LOG(m_pkActor);
	m_pkActor->SetCanHit(bCanHit);
}
void	lwActor::StartGodTime(float fTotalGodTime)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->StartGodTime(fTotalGodTime);
}
bool lwActor::IsGodTime()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->IsGodTime();
}
void lwActor::Blink(bool bBlink, int iBlinkFreq)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->BlinkThis(bBlink, iBlinkFreq);
}

bool lwActor::IsMeetFloor()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->IsMeetFloor();
}

bool lwActor::IsCheckMeetFloor()
{
	if(m_pkActor)
	{
		return m_pkActor->IsCheckMeetFloor();
	}

	return true;
}

bool lwActor::IsMeetSide()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->IsMeetSide();
}

bool lwActor::IsSlide()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->GetSlide();
}

void lwActor::SetUseSmoothShow(bool bSmoothShow)
{
	if (m_pkActor)
	{
		m_pkActor->UseSmoothShow( (true == bSmoothShow ? 1.0f : 0.0f) );
	}
}

void lwActor::SetMeetFloor(bool bFloor)
{
	if( m_pkActor )
	{
		m_pkActor->SetMeetFloor(bFloor);
	}
}

bool lwActor::AttachSound(int iSlot, char const *pcSndID, float fVolume, float fDistMin, float fDistMax)
{
	if( m_pkActor )
	{
		m_pkActor->AttachToSound(NiAudioSource::TYPE_3D, pcSndID, fVolume, fDistMin, fDistMax);
	}
	return	true;
}

bool lwActor::AttachSoundToPoint(int iSlot, char const *pcSndID, lwPoint3 kPos, float fVolume, float fDistMin, float fDistMax)
{
	if(g_pkWorld)
	{
		return NULL!=g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, pcSndID, fVolume, fDistMin, fDistMax, g_pkWorld->GetDynamicNodeRoot(), &NiPoint3(kPos.GetX(), kPos.GetY(), kPos.GetZ()));
	}
	return false;
}

void lwActor::BackMoving(bool bBack)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetBackMoving(bBack);
}

void lwActor::SetGravity(float fGravity)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetGravity(fGravity);
}
void	lwActor::UseSkipUpdateWhenNotVisible(bool bUse)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->UseSkipUpdateWhenNotVisible(bUse);
}
void	lwActor::SetThrowStart()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetThrowStart();
}
lwPoint3	lwActor::CheckCollWithCamera()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	NiPoint3	kCollPoint(-1.0f,-1.0f,-1.0f);
	if(m_pkActor->CheckCollWithCamera(kCollPoint.x,kCollPoint.y))
	{
		return	lwPoint3(kCollPoint);
	}
	return	lwPoint3(-1.0f,-1.0f,-1.0f);
}
void lwActor::Throw()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetNoFindPathNormal(true);
	m_pkActor->GetPhysXActor()->getShapes()[0]->setGroup(10);
	
	NxVec3 kVel;
	NiPhysXTypes::NiPoint3ToNxVec3(m_pkActor->GetPathNormal(), kVel);
	
	kVel *= -1.0f;
	kVel.z = 1.0f;
	kVel.normalize();

	float fAdjust[3] = {0,};
	fAdjust[0] = (float)(BM::Rand_Index(100));
	fAdjust[1] = (float)(BM::Rand_Index(100));
	fAdjust[2] = (float)(BM::Rand_Index(100));

	if(BM::Rand_Range(1)){fAdjust[0] *= -1;}
	if(BM::Rand_Range(1)){fAdjust[1] *= -1;}

	kVel *= 500.0f;
	kVel.x+=fAdjust[0];
	kVel.y+=fAdjust[1];
	kVel.z+=fAdjust[2];

	m_pkActor->GetPhysXActor()->clearBodyFlag(NX_BF_KINEMATIC);
	m_pkActor->GetPhysXActor()->setLinearVelocity(kVel);
	m_pkActor->GetPhysXActor()->setAngularVelocity(NxVec3(200 + BM::Rand_Index(400), 0.0f, 0.0f));	

	m_pkActor->SetThrowStart();
}

void lwActor::SetAnimSpeed(float fSpeed)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetAnimSpeed(fSpeed);
}
//! 애니메이션 스피드 얻기
float	lwActor::GetAnimSpeed()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return	m_pkActor->GetAnimSpeed();
}

//!	일정 시간동안 애니메이션 스피드를 조정한다.
void	lwActor::SetAnimSpeedInPeriod(float fAnimSpeed,int iPeriod)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetAnimSpeedInPeriod(fAnimSpeed,iPeriod);
}

void	lwActor::SetAnimOriginalSpeed()
{
	if(!m_pkActor)
	{
		return;
	}
	PG_ASSERT_LOG(m_pkActor);
	m_pkActor->SetAnimOriginalSpeed();
}
void	lwActor::SetUnderMyControl(bool bUnderMyControl)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetUnderMyControl(bUnderMyControl);
}
bool	lwActor::IsUnderMyControl()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->IsUnderMyControl();
}

unsigned int lwActor::GetActionState(char const *pcActionID)
{
	if(!m_pkActor)
	{
		return 0;
	}
	return m_pkActor->GetActionState(pcActionID);
}

void lwActor::ClearActionState()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ClearActionState();
}

void lwActor::ClearDirectionSlot(bool bReset)
{
	if(!m_pkActor)
	{
		return;
	}
#ifdef PG_USE_DO_SYNC
	m_pkActor->ClearDirectionSlot(bReset);
#endif
}

void lwActor::MoveActor(float fX, float fY, float fZ)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->MoveActor(NxVec3(fX, fY, fZ));
}

void lwActor::FreeMove(bool bFreeMove)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetFreeMove(bFreeMove);
}

void lwActor::EquipAllItem()
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->EquipAllItem();
}

bool lwActor::PlaySlotSound(char const *pcSlotName)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->PlaySlotSound(std::string(pcSlotName));
}

bool lwActor::SetRotation(float fDegree, lwPoint3 kAxis)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->SetRotation(fDegree, kAxis());
}

bool lwActor::IsOnRidingObject()
{
	if(m_pkActor)
	{
		return m_pkActor->IsOnRidingObject();
	}

	return false;
}

void lwActor::ConcilDirection(lwPoint3 kLookingDir, bool bRightAway)
{
	if(m_pkActor)
	{
		m_pkActor->ConcilDirection(kLookingDir(), bRightAway);
	}
}

//bool lwActor::HangItOn()
//{
//	return m_pkActor->HangItOn();
//}
//
//bool lwActor::HangOnRope()
//{
//	return m_pkActor->HangOnRope();
//}

bool lwActor::ClimbUpLadder()
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->ClimbUpLadder();
}

//void lwActor::SwitchPhysical(bool bGoPhysical, float fAccumTime, float fFrameTime)
//{
//	m_pkActor->SwitchPhysical(bGoPhysical, fAccumTime, fFrameTime);
//}

void lwActor::Concil(bool bConcil)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetNoConcil(!bConcil);
}
//!	이녀석을 때릴수 있는가?
bool	lwActor::GetCanHit()
{
	if(!m_pkActor)
	{
		return false;
	}
	return	m_pkActor->GetCanHit();
}

void lwActor::SetPickupScript(char const *pcScript)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetPickupScript(pcScript);
}

void lwActor::SetMouseOverScript(char const *pcScript)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetMouseOverScript(pcScript);
}

void lwActor::SetMouseOutScript(char const *pcScript)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetMouseOutScript(pcScript);
}

//void lwActor::FollowActor(lwGUID kMasterGuid, float fMoveSpeed)
//{
//	m_pkActor->FollowActor(kMasterGuid(), fMoveSpeed);
//}

//void lwActor::Teleport()
//{
//	BM::GUID const &rkMasterGuid = m_pkActor->GetPetMaster();
//	if(rkMasterGuid != BM::GUID())
//	{
//		PgPilot *pkPilot = g_kPilotMan.FindPilot(rkMasterGuid);
//		if(pkPilot)
//		{
//			PgActor *pkMasterActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
//			if(pkMasterActor)
//			{
//				m_pkActor->SetPosition(pkMasterActor->GetPosition());
//			}
//		}
//	}
//}

void lwActor::UpdateModel(char const *pcActorName)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->Transformation(pcActorName, false);
}

//bool lwActor::OutOfSight(lwGUID kTargetGuid, float fDistance, bool bConsiderZAxis)
//{
//	return m_pkActor->OutOfSight(kTargetGuid(), fDistance, bConsiderZAxis);
//}
//
//int lwActor::CompareActorPosition(lwGUID kTargetGuid, lwPoint3 kAxis, float fRange)
//{
//	return m_pkActor->CompareActorPosition(kTargetGuid(), kAxis(), fRange);
//}
//
//bool lwActor::WillBeFall(bool bDown, float fDistance)
//{
//	return m_pkActor->WillBeFall(bDown, fDistance);
//}

void lwActor::SetSpeedScale(float fScale)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetSpeedScale(fScale);
}

float lwActor::GetSpeedScale()
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return m_pkActor->GetSpeedScale();
}

//lwGUID lwActor::GetPetMaster()
//{
//	BM::GUID kGuid = m_pkActor->GetPetMaster();
//	if(kGuid == BM::GUID())
//	{
//		return lwGUID(BM::GUID());
//	}
//	
//	return lwGUID(kGuid);
//}
//
//lwActor lwActor::GetMyPet()
//{
//	PgPilot *pkPilot = m_pkActor->GetPilot();
//	if(!pkPilot)
//	{
//		return lwActor(0);
//	}
//
//	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkPilot->GetUnit());
//	if(!pkPlayer)
//	{
//		return lwActor(0);
//	}
//
//	BM::GUID kGuid = pkPlayer->PetGuid();
//	pkPilot = g_kPilotMan.FindPilot(kGuid);
//	if(!pkPilot)
//	{
//		return lwActor(0);
//	}
//
//	PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
//	if(!pkActor)
//	{
//		return lwActor(0);
//	}
//
//	return lwActor(pkActor);
//}
//
//bool lwActor::IsPet()
//{
//	return m_pkActor->IsPet();
//}
//
//bool lwActor::RideMyPet(bool bRide)
//{
//	return m_pkActor->RideMyPet(bRide);
//}
//
//bool lwActor::IsRiding()
//{
//	return m_pkActor->m_bRiding;
//}


lwActor lwActor::GetMountTargetPet(void)const
{
	if(!m_pkActor)
	{
		return lwActor(NULL);
	}

	PgActor* pTargetPet = m_pkActor->GetMountTargetPet();
	if(!pTargetPet)
	{
		return lwActor(NULL);
	}

	return lwActor(pTargetPet);
}

bool lwActor::IsRidingPet(void)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->IsRidingPet();
}

bool lwActor::MountPet(void)
{
	if(!m_pkActor)
	{
		return false;
	}
	if(!m_pkActor->IsCompleteLoadParts()) { return false; }
	return m_pkActor->MountPet();
}

bool lwActor::UnmountPet(void)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->UnmountPet();
}

bool lwActor::CheckStatusEffectTypeExist(BYTE byType)
{ //디버프에 걸린 상태인가? (현재는 펫에 탑승할 수 있는지 판단하는 용도로만 쓰인다)
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->CheckStatusEffectTypeExist(byType);
}



// 방향 관련.
bool lwActor::LookAt(lwPoint3 kTarget, bool bTurnRightAway,bool bBidirection,bool bNoCheckSameDir)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->LookAt(kTarget(),bTurnRightAway,bBidirection,bNoCheckSameDir);
}

void lwActor::LookAtBidirection(lwPoint3 kTarget, bool bTurnRightAway)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetLookingTarget(kTarget(), bTurnRightAway);
}

lwPoint3 lwActor::GetLookingDir()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return lwPoint3(m_pkActor->GetLookingDir());
}

BYTE lwActor::GetLastDirection()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return m_pkActor->GetLastDirection();
}

void lwActor::IncRotate(float fRadian)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->IncRotate(fRadian);
}

void lwActor::DetachChild(char const *pcName)
{
	if(!m_pkActor)
	{
		return;
	}
	NiAVObject *pkChild = m_pkActor->GetObjectByName(pcName);
	if(pkChild)
	{
		NiNode *pkParent = pkChild->GetParent();
		if (pkParent)
		{
			THREAD_DELETE_OBJECT(pkParent->DetachChild(pkChild));
			pkParent->Update(0.0f);
		}		
	}
}

lwPoint3 lwActor::GetHitPoint()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return (lwPoint3)m_pkActor->GetHitPoint();
}

lwPoint3 lwActor::GetHitABVCenterPos()
{
	if(!m_pkActor)
	{
		return lwPoint3(0.0f,0.0f,0.0f);
	}
	return (lwPoint3)m_pkActor->GetHitObjectCenterPos();
}

int	lwActor::GetHitObjectABVIndex()
{
	if(!m_pkActor)
	{
		return 0;
	}
	return	m_pkActor->GetHitObjectABVIndex();
}

void lwActor::AddToDefaultItem(int iItemPos, int iItemNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->AddToDefaultItem(static_cast<eEquipLimit>(1 << iItemPos), iItemNo);
}

void lwActor::SetDefaultItem(int iItemPos, int iItemNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetDefaultItem(static_cast<eEquipLimit>(1 << iItemPos), iItemNo);
}

bool lwActor::EquipItem(int iItemNo, bool bSetToDefaultItem)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->AddEquipItem(iItemNo, bSetToDefaultItem);
}

void lwActor::PlayWeaponSound(int iActionType, lwActor kPeer, char const *pcActionID, float fVolume, lwActionTargetInfo kTargetInfo)
{
	if(m_pkActor)
	{
		m_pkActor->PlayWeaponSound((PgWeaponSoundManager::EWeaponSoundType)iActionType, kPeer(), pcActionID, fVolume, kTargetInfo());
	}
}

int lwActor::GetDefaultItem(int iPos)
{
	if(!m_pkActor)
	{
		return 0;
	}
	return m_pkActor->GetDefaultItem(static_cast<eEquipLimit>(iPos));
}

void lwActor::SetItemColor(int iItemPos, int iItemNo)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetItemColor(static_cast<eEquipLimit>(iItemPos), iItemNo);
}

void lwActor::SetHeadScale(float f)
{
	if(!m_pkActor)
	{
		return;
	}

	NiNodePtr spHair = m_pkActor->GetPartsAttachInfo().find(EQUIP_LIMIT_HAIR)->second->GetMeshRoot();
	NiNodePtr spFace = m_pkActor->GetPartsAttachInfo().find(EQUIP_LIMIT_FACE)->second->GetMeshRoot();

	spHair->SetTranslate(spHair->GetTranslate() + NiPoint3(2 * (f - 1.0f), 0, 0));
	spFace->SetTranslate(spFace->GetTranslate() + NiPoint3(2 * (f - 1.0f), 0, 0));

	spHair->SetScale(f);
	spFace->SetScale(f);
}
//!	액터의 아이템 모델을 임시로 교체한다.
void	lwActor::ChangeItemModel(int iItemPos,char const *pkNewItemXMLPath)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->ChangeItemModel(static_cast<eEquipLimit>(iItemPos),pkNewItemXMLPath);
}
//!	액터의 무기 모델을 원래 것으로 돌려놓는다.
void	lwActor::RestoreItemModel(int iItemPos)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->RestoreItemModel(static_cast<eEquipLimit>(iItemPos));
}
bool	lwActor::IsTopLevelActionEffect(lwAction kAction)
{
	if(!m_pkActor)
	{
		return false;
	}
	if(m_pkActor->GetActionEffectStack())
	{
		return m_pkActor->GetActionEffectStack()->IsTopLevelActionEffect(kAction());
	}
	return	false;
}

bool lwActor::HideParts(int iItemPos, bool bHide)
{
	if(!m_pkActor)
	{
		return false;
	}
/*	if(m_pkActor->GetPilot())
	{
		_PgOutputDebugString("[lwActor::HideParts] Actor:%s iItemPos:%d bHide:%d\n",
			MB(m_pkActor->GetPilot()->GetGuid().str()),iItemPos,bHide);
	}*/

	return m_pkActor->HideParts(static_cast<eEquipLimit>(1 << iItemPos), bHide);
}

void	lwActor::HideNode(char const *strNodeName,bool bHide)
{
	if(m_pkActor)
	{
		m_pkActor->HideNode(strNodeName,bHide);
	}
}

int lwActor::GetPartsHideCnt(int iItemPos)
{
	if(!m_pkActor)
	{
		return 0;
	}
	int iCnt = 0;
	m_pkActor->GetPartsHideCnt(static_cast<eEquipLimit>(1 << iItemPos), iCnt);
	return iCnt;
}

void lwActor::Talk(int i, int iUpTime)
{
	if(!m_pkActor)
	{
		return;
	}
	if( iUpTime )
	{
		m_pkActor->ShowChatBalloon(CT_NORMAL, TTW(i), iUpTime);
	}
	else
	{
		m_pkActor->ShowChatBalloon(CT_NORMAL, TTW(i));
	}	
}
void lwActor::SetBoss()
{
	if (m_pkActor)
		m_pkActor->SetObjectID(PgIXmlObject::ID_BOSS);
}

float lwActor::GetDistance(lwActor kActor)
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	NiPoint3 kDist = m_pkActor->GetTranslate() - kActor()->GetTranslate();
	return kDist.Length();
}

bool lwActor::AddEffect(int iEffectNo, int iEffectValue,lwGUID kCasterGUID, int iActionInstanceID, DWORD dwTimeStamp, bool bIsTemporaryEffect)
{
	if(!m_pkActor)
	{
		return false;
	}
	return m_pkActor->AddEffect(iEffectNo, iEffectValue, 0, kCasterGUID(), iActionInstanceID, dwTimeStamp, bIsTemporaryEffect);
}

//void lwActor::TwistActor(int fTwistTimes, float fSpeed)
//{
//	m_pkActor->TwistActor(fTwistTimes, fSpeed);
//}

void lwActor::RestoreTwistedActor()
{
	if (m_pkActor)
	{
		m_pkActor->RestoreTwistedActor();
	}
}

char const *lwActor::GetParam(char const *pcKey)
{
	if(m_pkActor)
	{
		return m_pkActor->GetParam(pcKey);
	}

	return NULL;
}

void lwActor::SetParam(char const *pcKey, char const *pcVal)
{
	if(NULL != pcVal)
	{
		if (m_pkActor)
		{
			m_pkActor->SetParam(pcKey, pcVal);
		}
	}
}

float lwActor::StartTeleJump(lwPoint3 kTargetLoc, float fDefaultHeight)
{
	if(!m_pkActor)
	{
		return 0.0f;
	}
	return m_pkActor->StartTeleJump(kTargetLoc(), fDefaultHeight);
}

void lwActor::SetAdjustValidPos(bool bAble)
{
	if (m_pkActor)
	{
		m_pkActor->SetAdjustValidPos(bAble);
	}
}

bool lwActor::IsLockBidirection()const
{
	if ( m_pkActor )
	{
		return m_pkActor->IsLockBidirection();
	}
	return false;
}

void lwActor::LockBidirection(bool bLock)
{
	if (m_pkActor)
	{
		m_pkActor->LockBidirection(bLock);
	}
}

void lwActor::StartSyncMove(lwPoint3 kTargetPos, char const *pcActionName )
{
	if(!m_pkActor)
	{
		return;
	}
	PgAction* pkAction = NULL;
	if ( !pcActionName )
	{
		pkAction = m_pkActor->GetPilot()->CreateAction(ACTIONNAME_IDLE);
	}
	else
	{
		pkAction = m_pkActor->GetPilot()->CreateAction(pcActionName);
	}

	if(pkAction)
	{
		pkAction->SetActionStartPos(kTargetPos());
		m_pkActor->StartSyncMove(pkAction);
	}
}
//!	빠르게 동기화 지점으로 이동한다.
bool lwActor::UpdateSyncMove(float fSpeed, float fFrameTime)
{
	if(m_pkActor)
	{
		return m_pkActor->UpdateSyncMove(fSpeed,fFrameTime);
	}

	return false;
}

bool lwActor::GetWalkingToTarget()
{
	if(m_pkActor)
	{
		return m_pkActor->GetWalkingToTarget();
	}

	return false;
}

lwPoint3 lwActor::GetWalkingTargetDir()
{
	if(!m_pkActor)
	{
		return lwPoint3(0,0,0);
	}

	return lwPoint3(m_pkActor->GetWalkingTargetDir());
}

void lwActor::SetWalkingTargetLoc(lwPoint3 kTargetLoc, bool bForceToTarget, char const *pcNextAction, bool bCliffCheck)
{
	if(m_pkActor)
	{
		m_pkActor->SetWalkingTargetLoc(kTargetLoc(), bForceToTarget, pcNextAction, bCliffCheck,-1);
	}
}

void lwActor::SetWalkingTarget(lwGUID kGuid, bool bForceToTarget, char const *pcNextAction, bool bCliffCheck)
{
	if(m_pkActor)
	{
		m_pkActor->SetWalkingTarget(kGuid(), bForceToTarget, pcNextAction, bCliffCheck,-1);
	}
}

void lwActor::SetNoWalkingTarget(bool bDoNextAction)
{
	if(m_pkActor)
	{
		m_pkActor->SetNoWalkingTarget(bDoNextAction);
	}
}

lwPoint3 lwActor::GetWalkingTargetLoc()
{
	if(m_pkActor)
	{
		return lwPoint3(m_pkActor->GetWalkingTargetLoc());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

//bool lwActor::ActivateLayerAnimation(char const *pcAnimationName)
//{
//	return m_pkActor->ActivateLayerAnimation(pcAnimationName);
//}
//
//bool lwActor::DeactivateLayerAnimation(char const *pcAnimationName)
//{
//	return m_pkActor->DeactivateLayerAnimation(pcAnimationName);
//}

bool lwActor::IsAlphaTransitDone()
{
	if(m_pkActor)
	{
		return m_pkActor->IsAlphaTransitDone();
	}

	return false;
}

void lwActor::SetTargetAlpha(float fStartAlpha, float fTargetAlpha, float fTransitionTime, bool bIgnoreLoadingFisnishInit)
{
	if(!m_pkActor)
	{
		return;
	}
	m_pkActor->SetTargetAlpha(fStartAlpha, fTargetAlpha, fTransitionTime, bIgnoreLoadingFisnishInit);
}
char const* lwActor::GetDieParticleID()
{
	if (!m_pkActor)
	{
		return "";
	}

	return (char*)m_pkActor->GetDieParticleID().c_str();
}

char const* lwActor::GetDieSoundID()
{
	if (!m_pkActor)
	{
		return "";
	}
	return	(char*)m_pkActor->GetDieSoundID().c_str();
}

char const* lwActor::GetDieParticleNode()
{
	if (!m_pkActor)
	{
		return "";
	}
	return	(char*)m_pkActor->GetDieParticleNode().c_str();
}

float lwActor::GetDieParticleScale()
{
	if (!m_pkActor)
	{
		return 0.0f;
	}
	return m_pkActor->GetDieParticleScale();
}

void lwActor::ResetLastFloorPos()
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor)
	{
		m_pkActor->ResetLastFloorPos();
	}
}

bool lwActor::GetExistDieAnimation()
{
	PG_ASSERT_LOG(m_pkActor);
	if(m_pkActor)
	{
		return m_pkActor->GetExistSubActorManager();
	}

	return false;
}

bool lwActor::GetUseDieAnimation()
{
	PG_ASSERT_LOG(m_pkActor);
	if(m_pkActor)
	{
		return m_pkActor->GetUseSubActorManager();
	}

	return false;
}

void lwActor::SetUseSubAnimation(bool bUse)
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor)
	{
		m_pkActor->SetUseSubActorManager(bUse);
	}
}
//!	무시할 이펙트 리스트에 추가한다.
void lwActor::AddIgnoreEffect(int iEffectID)
{
	if(m_pkActor)
	{
		m_pkActor->AddIgnoreEffect(iEffectID);
	}
}
void lwActor::RemoveIgnoreEffect(int iEffectID)
{
	if(m_pkActor)
	{
		m_pkActor->RemoveIgnoreEffect(iEffectID);
	}
}
void lwActor::ClearIgnoreEffectList()
{
	if(m_pkActor)
	{
		m_pkActor->ClearIgnoreEffectList();
	}
}

void lwActor::SetActiveGrp(int iGroup, bool bTrue)
{
	if(m_pkActor)
	{
		m_pkActor->SetActiveGrp(iGroup, bTrue);
	}
}

void lwActor::SetForceSync(bool bSync)
{
	if(m_pkActor)
	{
		m_pkActor->SetForceSync(bSync);
	}
}

void lwActor::Transformation(char const *pcNewModel, char const *pcNextAction)
{
	if(m_pkActor)
	{
		m_pkActor->Transformation(pcNewModel, pcNextAction);
	}
}

void lwActor::RestoreTransformation(char const *pcNextAction)
{
	if(m_pkActor)
	{
		m_pkActor->RestoreTransformation(pcNextAction);
	}
}

void lwActor::SetUpdateScript(char const *pcUpdateScript)
{
	if(pcUpdateScript)
	{
		m_pkActor->SetUpdateScript(std::string(pcUpdateScript).c_str());
	}
}

void lwActor::ResetActiveGrp()
{
	if(m_pkActor)
	{
		m_pkActor->ResetActiveGrp();
	}
}

void lwActor::AttachToObject(lwActor Actor, char const* pcNodeName)
{
	if(m_pkActor)
	{
		NiAVObject* pkObject = m_pkActor->GetObjectByName(pcNodeName);
		PgActor* pkActor = Actor();
		if (NiIsKindOf(NiNode, pkObject) && pkActor)
		{
			NiNode* pkNode = (NiNode*)pkObject;
			pkNode->AttachChild(pkActor, true);
			//((NiNode*)pkActor)->IncRefCount();	// 이렇게 하면 pkActor 가 삭제되지 않아 Memory Leak 발생.
		}
	}
}

void lwActor::DetachFromObject(char const* pcNodeName)
{
	if(m_pkActor)
	{
		NiAVObject* pkObject = m_pkActor->GetObjectByName(pcNodeName);
		if (NiIsKindOf(NiNode, pkObject))
		{
			NiNode* pkNode = (NiNode*)pkObject;
			for (unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
			{                                                                                                                                    
				NiAVObjectPtr	spChild = pkNode->DetachChildAt(i);
				PgActor* pkActor = NiDynamicCast(PgActor,spChild);
				if(pkActor && g_pkWorld && g_pkWorld->GetSceneRoot())
				{
					g_pkWorld->RunObjectGroupFunc(OGT_PLAYER, WorldObjectGroupsUtil::AttachChild(pkActor, true));
				}
			}
		}
	}
}

void lwActor::DetachFromParent()
{
	if (NULL == m_pkActor)
	{
		return;
	}

	NiNode* pkNode = NiDynamicCast(NiNode, m_pkActor);
	if(pkNode)
	{
		//NiNode* pkParent = pkNode->GetParent();
		//if (pkParent)
		//	pkParent->DetachChild(pkNode);

		pkNode->DetachParent();
	}
}

void lwActor::ResetTransfomation()
{
	if(m_pkActor)
	{
		NiNode* pkNode = NiDynamicCast(NiNode, m_pkActor);
		if (pkNode)
		{
			pkNode->SetTranslate(NiPoint3(0,0,0));
			pkNode->SetScale(1.0f);
			NiMatrix3 kMat;
			kMat.MakeIdentity();
			pkNode->SetRotate(kMat);
		}
	}
}

bool lwActor::IsTransformed()
{
	if(m_pkActor)
	{
		return m_pkActor->IsTransformed();
	}
	return false;
}
void lwActor::RestoreLockBidirection()
{
	if(!m_pkActor)
	{
		return;
	}

	m_pkActor->RestoreLockBidirection();
}

BYTE lwActor::GetDirFromMovingVector(lwPoint3 kPoint)
{
	if(!m_pkActor)
	{
		return 0;
	}

	return m_pkActor->GetDirFromMovingVector(kPoint());
}

BYTE lwActor::GetDirFromMovingVector8Way(lwPoint3 kPoint)
{
	if(!m_pkActor)
	{
		return 0;
	}

	return m_pkActor->GetDirFromMovingVector8Way(kPoint());
}

void lwActor::SetLookingDirection(BYTE byDir,bool bTurnRightAway)
{
	if(!m_pkActor)
	{
		return;
	}

	m_pkActor->SetLookingDirection(byDir,bTurnRightAway);
}

lwQuaternion lwActor::GetRotateQuaternion()
{
	if(!m_pkActor)
	{
		lwQuaternion kQuaternion(0,0,0,1);
		return kQuaternion;
	}

	NiQuaternion kQuaternion;
	m_pkActor->GetRotate(kQuaternion);

	lwQuaternion kQuat(kQuaternion);

	return kQuat;
}

void lwActor::HideEquipItem(int iEquipPos, bool bHide)
{
	if(!m_pkActor)
	{
		return;
	}

	m_pkActor->HideEquipItem(iEquipPos, bHide);
}

void lwActor::SetUpdatePhysXFrameTime(float fFrameTime)
{
	if(!m_pkActor)
	{
		return;
	}

	m_pkActor->SetUpdatePhysXFrameTime(fFrameTime);
}

void lwActor::SetLookTarget(bool bLook)
{
	if(!m_pkActor)
	{
		return;
	}	

	PgActorMonster *pkMonster = dynamic_cast<PgActorMonster *>(m_pkActor);
	if(!pkMonster)
	{
		return;
	}

	pkMonster->SetLookTarget(bLook);
}

bool lwActor::IsEqualObjectName(char *szName)
{
	if ( m_pkActor )
	{
		const NiFixedString kName = m_pkActor->GetName();
		return kName.Equals(szName);
	}
	return false;
}

void lwActor::CopyEquipItem(lwActor kActor)
{
	if ( m_pkActor )
	{
		m_pkActor->CopyEquipItem(kActor());
	}
}

void lwActor::SetAutoDeleteActorTimer(float fTime)
{
	if ( m_pkActor )
	{
		m_pkActor->SetAutoDeleteActorTimer(fTime);
	}
}

void lwActor::ReleaseAllParticles()
{
	if ( m_pkActor )
	{
		m_pkActor->ReleaseAllParticles();
	}
}

void lwActor::SetUseLOD(bool bUse)
{
	if (m_pkActor)
	{
		m_pkActor->SetUseLOD(bUse);
	}
}

bool lwActor::AttachAttackEffect(char const* szActionName, int const iSlot)
{
	if (m_pkActor)
	{
		return m_pkActor->AttachAttackEffect(szActionName, iSlot);
	}

	return false;
}

char const* lwActor::GetDamageEffectID(bool bIsCri)
{
	if (m_pkActor)
	{
		PgItemEx* pkItemEx = m_pkActor->GetEquippedWeapon();
		if(pkItemEx)
		{
			return pkItemEx->GetDamageEffect(bIsCri).c_str();
		}
	}
	return "";
}

lwAttackEffect lwActor::GetAttackEffect(char const* szActionName)
{
	if (m_pkActor)
	{
		PgItemEx* pkItemEx = m_pkActor->GetEquippedWeapon();
		if(pkItemEx)
		{
			PgItemEx::SAttackEffect const* pkEff = pkItemEx->FindAttackEffect(szActionName);
			if(pkEff)
			{
				return lwAttackEffect(pkEff);
			}
		}
	}

	return lwAttackEffect(NULL);
}

lwWString lwActor::GetStatusEffectParam(int const iEffectID, char const* szKey)const
{
	std::string kValue;
	if( m_pkActor && szKey)
	{
		kValue = m_pkActor->GetStatusEffectParam(iEffectID, szKey);
	}
	return lwWString(kValue.c_str());
}

void lwActor::SetStatusEffectParam(int const iEffectID, char const* szKey, char const* szValue)
{
	if( m_pkActor && szKey && szValue)
	{
		m_pkActor->SetStatusEffectParam(iEffectID, szKey, szValue);
	}
}

lwWString lwActor::GetNpcMenuStr()
{
	if( m_pkActor )
	{
		PgPilot* pkPilot = m_pkActor->GetPilot();
		if( pkPilot )
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if( pkUnit )
			{
				PgNpc* pkNpc = dynamic_cast< PgNpc* >(pkUnit);
				if( pkNpc )
				{
					return pkNpc->MenuStr();
				}
			}
		}
	}
	return lwWString(std::wstring());
}

void lwActor::SetAnimationStartTime(float fTime)
{
	if(m_pkActor)
	{
		m_pkActor->SetAnimationStartTime(fTime);
	}
}

float lwActor::GetAnimationStartTime()
{
	if(m_pkActor)
	{		
		return m_pkActor->GetAnimationStartTime();
	}
	return (0.0f);
}

lwGUID lwActor::GetCallerGuid()const
{
	if ( m_pkActor )
	{
		if ( m_pkActor->GetPilot() )
		{
			CUnit *pkUnit = m_pkActor->GetPilot()->GetUnit();
			if ( pkUnit )
			{
				return lwGUID( pkUnit->Caller() );
			}
		}
	}
	return lwGUID(NULL);
}


void lwActor::IncTimeToAniObj(int const iMilliSec)
{
	PgWorld::IncTimeToAniObj(m_pkActor->GetNIFRoot(), static_cast<unsigned __int64>(iMilliSec));
}

void lwActor::MakePetActionQueue(int iGrade)
{
	PgActorPet* pkPet = dynamic_cast<PgActorPet*>(m_pkActor);
	if(pkPet)
	{
		pkPet->MakePetActionQueue(iGrade);
	}
}

int lwActor::UpdatePetActionQueue(float fAccumTime)
{
	if(!m_pkActor)
	{
		return 0;
	}
	PgPilot* pkPilot = m_pkActor->GetPilot();
	if(!pkPilot)	{ return 0; }
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)		{ return 0; }
	switch( pkUnit->UnitType() )
	{
	case UT_PET:
		{
			PgActorPet* pkActorPet = static_cast<PgActorPet*>(m_pkActor);
			if(pkActorPet)
			{
				return pkActorPet->UpdatePetActionQueue(fAccumTime);
			}
		}break;
	case UT_SUB_PLAYER:
		{
			PgActorSubPlayer* pkActorSubPlayer = static_cast<PgActorSubPlayer*>(m_pkActor);
			if(pkActorSubPlayer)
			{
				return pkActorSubPlayer->UpdatePetActionQueue(fAccumTime);
			}
		}break;
	}
	return 0;
}
void lwActor::AddSummonUnit(lwGUID kGuid, int iClassNo, bool bUniqueClass)
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			int iSummonType = ESO_NONE;
			if(bUniqueClass)
			{
				iSummonType |= ESO_UNIQUE_CLASS;
			}
			m_pkActor->GetPilot()->GetUnit()->AddSummonUnit(kGuid(), iClassNo, static_cast<SummonOptionType>(iSummonType));
		}
	}
}

int lwActor::GetSummonUnitCount()
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			return m_pkActor->GetPilot()->GetUnit()->GetSummonUnitCount();
		}
	}

	return 0;
}

int lwActor::GetMaxSummonUnitCount()
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			return m_pkActor->GetPilot()->GetUnit()->GetMaxSummonUnitCount();
		}
	}

	return 0;
}

bool lwActor::DeleteSummonUnit(lwGUID kSummonUnitGuid)
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			return m_pkActor->GetPilot()->GetUnit()->DeleteSummonUnit(kSummonUnitGuid());
		}
	}

	return false;
}

lwGUID lwActor::GetSummonUnit(int iIndex)
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			return lwGUID(m_pkActor->GetPilot()->GetUnit()->GetSummonUnit(iIndex));
		}
	}

	BM::GUID kGuid;
	return lwGUID(kGuid);
}

void lwActor::ClearSummonUnit()
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			m_pkActor->GetPilot()->GetUnit()->ClearSummonUnit();
		}
	}
}

void lwActor::PickUpNearItem(float const fPickRange, int const iCount)
{
	if(m_pkActor)
	{
		if(m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
		{
			if(UT_PLAYER!=m_pkActor->GetPilot()->GetUnit()->UnitType())
			{
				m_pkActor->PickUpNearItem(true, fPickRange, iCount);	//플레이어가 아닐 때만 호출하자. PgActor의 업데이트에서 이미 호출되기 때문
			}
		}
	}
}

void lwActor::SetAttackState(int iValue)
{
	PgActorPet* pkPet = dynamic_cast<PgActorPet*>(m_pkActor);
	if(pkPet)
	{
		pkPet->SetAttackState(static_cast<PgActorPet::E_PET_ATK_STATE>(iValue));
	}
}

int lwActor::GetAttackState()
{
	PgActorPet* pkPet = dynamic_cast<PgActorPet*>(m_pkActor);
	if(pkPet)
	{
		return static_cast<int>(pkPet->GetAttackState());
	}

	return 0;
}

bool lwActor::GetActionToggleState(int iActionNo)
{
	if(m_pkActor)
	{
		if(0==iActionNo && m_pkActor->GetAction())
		{
			iActionNo = m_pkActor->GetAction()->GetActionNo();
		}

		if(iActionNo)
		{
			return m_pkActor->GetActionToggleState(iActionNo);
		}
	}

	return false;
}

lwGUID lwActor::FollowingHeadGuid()
{
	if( m_pkActor )
	{
		PgPilot * pkPilot = m_pkActor->GetPilot();
		if ( pkPilot )
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if( pkPlayer )
			{
				return lwGUID(pkPlayer->FollowingHeadGuid());
			}
		}
	}
	return lwGUID(BM::GUID::NullData());
}

int lwActor::GetGroggyRecoveryValue()const
{
	if(!m_pkActor)
	{
		return 0;
	}
	PgPilot * pkPilot = m_pkActor->GetPilot();
	if ( pkPilot && pkPilot->GetUnit() )
	{
		return pkPilot->GetUnit()->GetGroggyRecoveryValue();
	}

	return 0;
}

bool const lwActor::IsInCoolTime(int iSkillNo, bool bIsGobalCoolTime) const
{
	if( m_pkActor )
	{
		return m_pkActor->IsInCoolTime(iSkillNo, bIsGobalCoolTime);
	}
	return false;
}

lwActor lwActor::GetSubPlayer()
{	
	return lwActor( PgActorUtil::GetSubPlayerActor(m_pkActor) );
}

lwActor lwActor::GetCallerActor() const
{
	if ( m_pkActor )
	{
		if ( m_pkActor->GetPilot() )
		{
			CUnit *pkUnit = m_pkActor->GetPilot()->GetUnit();
			if ( pkUnit )
			{
				return lwActor( g_kPilotMan.FindActor(pkUnit->Caller()) );
			}
		}
	}
	return lwActor(NULL);
}

int lwActor::GetAniSequenceID()
{
	if ( m_pkActor )
	{
		return m_pkActor->GetAniSequenceID();
	}
	return 0;
}

float lwActor::GetAnimationTime(char const* strActionName)
{
	if ( m_pkActor )
	{
		return m_pkActor->GetAnimationTime(strActionName);
	}
	return 0;
}

void lwActor::SetDefaultHeadSize(float const fDefaultHeadSize)
{
	if ( m_pkActor )
	{
		return m_pkActor->SetDefaultHeadSize(fDefaultHeadSize);
	}
}
float lwActor::GetDefaultHeadSize() const
{
	if ( m_pkActor )
	{
		return m_pkActor->GetDefaultHeadSize();
	}
	return 0.0f;
}

void lwActor::SetTargetHeadSize(float const fTargetHeadSize, float const fTransitSpeed)
{
	if ( m_pkActor )
	{
		m_pkActor->SetTargetHeadSize(fTargetHeadSize, fTransitSpeed);
	}
}

void lwActor::SetDuelWinnerTitle(void)
{
	if ( m_pkActor )
	{
		m_pkActor->SetDuelWinnerTitle();
	}
}

void lwActor::SetAlpha(float fAlpha)
{
	if ( m_pkActor )
	{
		m_pkActor->SetAlpha(fAlpha);
	}
}
float lwActor::GetAlpha()const
{
	if ( m_pkActor )
	{
		return m_pkActor->GetAlpha();
	}
	return 0.0f;
}

void lwActor::SetHideNameTitle(bool const bHide)
{ 
	if ( m_pkActor )
	{
		m_pkActor->SetHideNameTitle(bHide);
	}
}
bool lwActor::IsHideNameTitle() const 
{ 
	if ( m_pkActor )
	{
		return m_pkActor->IsHideNameTitle();
	}
	return false;
}

void lwActor::SetComboCharge(float const fChargingTime)
{
	if ( m_pkActor )
	{
		m_pkActor->SetComboCharge(fChargingTime);
	}
}
void lwActor::CallComboCharge()
{
	if ( m_pkActor )
	{
		m_pkActor->CallComboCharge();
	}
}
void lwActor::CutComboCharge()
{
	if ( m_pkActor )
	{
		m_pkActor->CutComboCharge();
	}
}