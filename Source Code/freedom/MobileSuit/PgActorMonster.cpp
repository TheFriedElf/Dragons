#include "Stdafx.h"
#include "Variant/PgRareMonsterSpeech.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgActorMonster.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "lwActor.h"
#include "lwAction.h"
#include "PgMobileSuit.h"
#include "PgInterpolator.h"
#include "PgStat.h"
#include "PgPilotMan.h"
#include "PgActionEffect.h"
#include "PgChatMgrClient.h"
#include "ServerLib.h"
#include "PgActionSlot.h"
#include "PgElementMark.h"
#include "PgActorUtil.h"

NiImplementRTTI(PgActorMonster, PgActor);

PgActorMonster::PgActorMonster()
:m_eMonsterType(EMONTYPE_NORMAL),
m_bLookFocusTarget(true), m_spElementMark(0)
{
	m_uiActiveGrp = MONSTER_ACTIVE_GRP;
}

bool PgActorMonster::Update(float fAccumTime, float fFrameTime)
{
	PgActor::Update(fAccumTime, fFrameTime);
	if(true==m_bCanRide)
	{
		UpdateRidingInfo(fAccumTime, fFrameTime);
	}
	
	PgPilot* pkPilot = GetPilot();
	if(pkPilot && pkPilot->GetUnit())
	{
		int iDelay = pkPilot->GetUnit()->GetDelay();
		pkPilot->GetUnit()->SetDelay(std::max((int)(iDelay - (fFrameTime*1000)), 0));
	}
	
	return true;
}

//랙으로 인해 fFrameTime이 커져 z축 값이 너무 커지는 경우 방지
void PgActorMonster::SetMovingDeltaZCheck()
{
	bool const bFlyingMonster = m_eMonsterType == EMONTYPE_FLYING;
	if(!m_bFreeMove && !bFlyingMonster)
	{
		float const fControllerHalfHeight = m_pkController->getHeight() * 0.5f;
		if( m_kMovingDelta.z < -fControllerHalfHeight)
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,-fControllerHalfHeight));
		}
	}
}

void PgActorMonster::UpdatePhysX(float fAccumTime, float fFrameTime)
{
	PG_ASSERT_LOG(m_pkPhysXActor);
	PG_ASSERT_LOG(m_pkController);

	if (m_pkPhysXActor == NULL || m_pkController == NULL)
	{
		return;	//	leesg213 2006.12.06
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdatePhysX"), g_pkApp->GetFrameCount()));

#ifdef PG_USE_ACTOR_TRACE
	NxExtendedVec3 beforeUpdatePos = m_pkController->getPosition();
	NxExtendedVec3 afterUpdatePos = m_pkController->getPosition();	
	NxExtendedVec3 afterMove1Pos = m_pkController->getPosition();
	NxExtendedVec3 beforeMove2Pos = m_pkController->getPosition();
	NxExtendedVec3 afterMove2Pos = m_pkController->getPosition();
#endif
	NxExtendedVec3 beforeMove1Pos = m_pkController->getPosition();

	if(m_pkAction)
	{
		if (1.0f>m_fRotationInterpolTime || GetNowPush())
		{
		}
		else if(IsMeetFloor() && m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) == false)
		{
			return;
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		GetWorld()->LockPhysX(true);
		PG_STAT(timerA.Stop());
	}

	bool bFlyingMonster = false;
	float fDeltaZ = 0.0f;
	float fTargetZPos = 0.0f;

	if (m_eMonsterType == EMONTYPE_FLYING)
	{
		bFlyingMonster = true;
		fTargetZPos = static_cast<float>(GetPilot()->GetAbil(AT_HEIGHT));
		PG_ASSERT_LOG(fTargetZPos > 0.0f);
		
		if(m_pkAction && m_pkAction->GetID() != "a_die")
		{
			m_bJump = false;
		}
		else if(m_bJump == false)
		{
			m_bFalling = true;
			m_bJump = true;
			m_fInitialVelocity = 0.0f;
			m_fJumpTime = 0.0f;
		}

		//if (GetAction() && GetAction()->GetID() == "a_fly")
		//	bFlyingMonster = true;
	}

	if(m_bJump)
	{
		// 점프 중이라면 점프 높이를 계산하자.
		// h = (v0 * (t+ delta t) + 0.5 * g(t + delta t)^2) - (v0*t + 0.5 * gt^2)
		//   = (v0 * (JumpTime + FrameTime) + 0.5 * g(JumpTime + FrameTime)^2) - (v0*t + 0.5 * gt^2)
		//   = ...
		//   = v0 + (g * (0.5 * JumpTime + FrameTime))
		float fJumpHeight = m_fInitialVelocity + ms_fGravity * (0.5f * fFrameTime +  m_fJumpTime);
		m_fJumpTime += fFrameTime;
		SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,m_kMovingDelta.z+fJumpHeight));
	}
	else
	{
		m_fJumpAccumHeight = 0.0f;
		if(!m_bFreeMove)
		{
			if(!bFlyingMonster)
			{
				SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,-98.0f));
			}
			else if(m_pkAction && m_pkAction->GetActionType() != "EFFECT")
			{
				NxExtendedVec3 kPos = m_pkController->getDebugPosition();
				fDeltaZ = AdjustToFly(NiPoint3((float)kPos.x, (float)kPos.y, (float)kPos.z), fFrameTime);
			}
		}
	}

	SetMovingDelta(m_kMovingDelta * fFrameTime);
	SetMovingDeltaZCheck();
	SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,m_kMovingDelta.z+fDeltaZ));

	//NILOG(PGLOG_MINOR, "Moving Delta Frame: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);

#ifdef PG_USE_CAPSULE_CONTROLLER
	float fControllerHalfHeight = m_pkController->getHeight() * 0.5f;
	float fControllerRadius = m_pkController->getRadius();
#else
	float fControllerHalfHeight = m_pkController->getExtents().y * 0.5f;
	float fControllerRadius = m_pkController->getExtents().x;
#endif

	// NxController로 캐릭터를 움직인다.
	NxU32 collisionFlags = 0;

	// TODO : 아래 코드를 Walk로 빼도 되는가?
	// Set Walking Location을 쓸 때, 다음 움직일 곳이 떨어지는 곳이면 못가게 하는 것
	if(m_bCheckCliff)
	{
		NxExtendedVec3 kCharPos = m_pkController->getPosition();
		NxExtendedVec3 kBeforePos = kCharPos;
		kBeforePos += m_kMovingDelta;

		NxRaycastHit kHit;
		if(!GetWorld()->raycastClosestShape(
			NxRay(NxVec3((NxReal)kBeforePos.x, (NxReal)kBeforePos.y, (NxReal)kBeforePos.z + 10.0f), NxVec3(0, 0, -1)),
			NX_STATIC_SHAPES, kHit, -1, NiMax(fTargetZPos + 50.0f, 0.1f), NX_RAYCAST_SHAPE))
		{
			SetMovingDelta(NxVec3(0,0,m_kMovingDelta.z));
			m_kTargetDir = NiPoint3::ZERO;
			m_bWalkingToTargetForce = false;
			if(m_kTargetWalkingNextAction.length() != 0)
			{
				TransitAction(m_kTargetWalkingNextAction.c_str());
			}
		}
	}

	beforeMove1Pos = m_pkController->getPosition();

	// 얼어 있으면, Z값 외에는 움직이지 않는다.
	if( GetFreezed() )
	{
		SetMovingDelta(NxVec3(0,0,m_kMovingDelta.z));
	}

#ifdef PG_USE_ACTOR_AUTOMOVE_EDGE
	if (m_bAdjustValidPos)
	{
		// 다음 좌표가 떨어지지 않게 보정해서 좌표를 리턴한다.
		float fDeltaZ = m_kMovingDelta.z;
		if(!m_bJump)
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,0));
		}

		SetMovingDelta(GetAdjustValidDeltaLoc(m_kMovingDelta));
		if(!m_bJump)
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,fDeltaZ));
		}
	}
#endif

	NxVec3	kFinalMove = m_kMovingDelta;
	NxExtendedVec3	kBeforeMove = m_pkController->getPosition();

	if(m_pkAction)
	{
		if(m_pkAction->GetActionOptionEnable(PgAction::AO_IGNORE_PHYSX))
		{
			m_pkController->move(m_kMovingDelta , 0, 0.001f, collisionFlags, 1.0f);
		}
		else
		{
			m_pkController->move(m_kMovingDelta , m_uiActiveGrp, 0.001f, collisionFlags, 1.0f);			
		}
	}

	m_kMovingAbsolute.zero();
	NxExtendedVec3	kAfterMove = m_pkController->getDebugPosition();
	SetPositionChanged( (kAfterMove.x != kBeforeMove.x) || (kAfterMove.y != kBeforeMove.y) || (kAfterMove.z != kBeforeMove.z) );

	NxVec3 kCharPos(static_cast<NxReal>(kAfterMove.x), static_cast<NxReal>(kAfterMove.y), static_cast<NxReal>(kAfterMove.z));
	UpdateBottomRayHit(kCharPos);

#ifdef PG_USE_ACTOR_TRACE
	afterMove2Pos = afterMove1Pos = m_pkController->getPosition();
	//_PgOutputDebugString("Cur Loc : %.1f, %.1f, %.1f\n", afterMove1Pos.x, afterMove1Pos.y, afterMove1Pos.z);
#endif

	bool bPreviousFloor = m_bFloor;
	m_bFloor = (collisionFlags & NXCC_COLLISION_DOWN ? true : false);
	m_bSide = (collisionFlags & NXCC_COLLISION_SIDES ? true : false);

	if(!m_bFreeMove)
	{
		if(m_bFloor) 
		{
			StopJump();
			m_fInitialVelocity = 0.0f;
			m_fJumpTime = 0.0f;
		}

		// Actor가 천장에 부딪혔을 때
		if((NXCC_COLLISION_UP & collisionFlags)	&& (0.0f != m_fInitialVelocity))
		{
			m_fInitialVelocity = 0.0f;
			m_fJumpTime = 0.0f;
		}
	}

	if (GetWorld() && 1 == g_iUseAddUnitThread)
	{
		GetWorld()->LockPhysX(false);
	}

	if(m_bJump)
	{
		// 반드시 getDebugPosition을 써야 한다. move를 한 후 Update가 되기 전에는 getPosition으로 가져오면 좌표가 그대로다.
		NxExtendedVec3 kCurPos = m_pkController->getDebugPosition();
		float fRealJumpHeight = static_cast<float>(kCurPos.z - beforeMove1Pos.z);
		m_fJumpAccumHeight += fRealJumpHeight;
	}

	SetMovingDelta(NxVec3(0,0,0));

	if (m_pkAction && m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
	{
		FindPathNormal();
	}

	if (m_bFloor)
	{
		NxExtendedVec3 curPos = m_pkController->getDebugPosition();
		m_kLastFloorPos.x = (float)curPos.x;
		m_kLastFloorPos.y = (float)curPos.y;
		m_kLastFloorPos.z = (float)curPos.z;
	}

	if(bPreviousFloor != m_bFloor)	//	바닥에서 떨어졌거나 혹은 바닥에 닿았을때 스크립트 호출한다.
	{
		lua_tinker::call<void, lwActor, lwAction,bool>("Actor_OnMeetFloor", lwActor(this), lwAction(GetAction()),m_bFloor);
	}

	// 회전을 보간한다.
	if(1.0f > m_fRotationInterpolTime)
	{
		m_fRotationInterpolTime = NiMin(m_fRotationInterpolTime + fFrameTime * PG_ROTATION_INTERPOL_SPEED, 1.0f);

		NxQuat kToRot;
		NiPhysXTypes::NiQuaternionToNxQuat(m_kToRotation, kToRot);
		NxQuat kCurRot = m_pkPhysXActor->getGlobalOrientationQuat();

		NxQuat kNewRot;
		kNewRot.slerp(m_fRotationInterpolTime, kCurRot, kToRot);
		kNewRot.normalize();
		SetRotation(kNewRot);
	}

#ifdef PG_USE_ACTOR_TRACE
	afterUpdatePos = m_pkController->getPosition();

	if (m_bTraceUpdate)
	{
		if (m_kLastFramePos.z != beforeUpdatePos.z)
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form last updatePhysx(%f,%f,%f) to (%f,%f,%f)\n", MB(GetGuid().str()), 
			m_kLastFramePos.x, m_kLastFramePos.y, m_kLastFramePos.z, beforeUpdatePos.x, beforeUpdatePos.y, beforeUpdatePos.z);

		if (beforeMove1Pos.z != afterMove1Pos.z)
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form before move1(%f,%f,%f) to after move1(%f,%f,%f)\n", MB(GetGuid().str()), 
			beforeMove1Pos.x, beforeMove1Pos.y, beforeMove1Pos.z, afterMove1Pos.x, afterMove1Pos.y, afterMove1Pos.z);

		if (beforeMove2Pos.z != afterMove2Pos.z)
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form before Move2(%f,%f,%f) to after Move2(%f,%f,%f)\n", MB(GetGuid().str()), 
			beforeMove2Pos.x, beforeMove2Pos.y, beforeMove2Pos.z, afterMove2Pos.x, afterMove2Pos.y, afterMove2Pos.z);
	}

	if (m_kLastFramePos.z > 0.0f && afterUpdatePos.z < 0.0f)
	{
		NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos z go underground(%f -> %f)\n", MB(GetGuid().str()), m_kLastFramePos.z, afterUpdatePos.z);
		//m_bTraceUpdate = true;
	}
	else if (m_kLastFramePos.z < 0.0f && afterUpdatePos.z > 0.0f)
	{
		NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos z go upperground(%f -> %f)\n", MB(GetGuid().str()), m_kLastFramePos.z, afterUpdatePos.z);
		//m_bTraceUpdate = false;
	}
	else if (m_kLastFramePos.z > 0.0f && afterUpdatePos.z > 0.0f)
	{
		//m_bTraceUpdate = false;
	}

	m_kLastFramePos = afterUpdatePos;
#endif
	m_kLastFramePos = m_pkController->getPosition();
}

float PgActorMonster::AdjustToFly(NiPoint3 const &kCurPos, float fFrameTime)
{
	// z좌표를 보간하기 위한 작업을 한다.		
	static float const fZMoveSpeed = 40.0f;

	bool bMoveUp = true;
	float fDeltaZ = 0.0f;
	float fTargetZPos = static_cast<float>(GetPilot()->GetAbil(AT_HEIGHT));
	NiPoint3 kTargetPos = kCurPos;

	NxRay kRay(NxVec3(kCurPos.x, kCurPos.y, kCurPos.z + 1.0f), NxVec3(0, 0, -1.0f));
	NxRaycastHit kHit;
	if(GetWorld()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, 1000.0f, NX_RAYCAST_IMPACT))
	{
		NiPhysXTypes::NxVec3ToNiPoint3(kHit.worldImpact, kTargetPos);
		kTargetPos.z = kTargetPos.z + fTargetZPos + PG_CHARACTER_Z_ADJUST;
	}

	//kTargetPos = PgInterpolator::Lerp(kCurPos, kTargetPos, fFrameTime);
	if (NiAbs(kTargetPos.z - kCurPos.z) > 0.01f)
	{
		fDeltaZ = (kTargetPos.z - kCurPos.z);
	}

	return fDeltaZ;
}

void PgActorMonster::SetAttackTarget(BM::GUID const& kTargetGUID)
{
	if(!g_pkWorld)
	{
		return;
	}

	//!	이전에 인지한 공격 목표와 같은 타겟일 경우
	if(kTargetGUID == BM::GUID::NullData() || 
		(m_kLastAttackTargetGUID == kTargetGUID && g_pkWorld->GetAccumTime() - m_fLastAttackTargetedTime < 5.0))
	{
		//	시간이 5초 이상 흘렀을 경우에만 인지 이펙트를 띄운다.
		return;
	}

	//!	인지 이펙트 띄우기
	lwActor(this).AttachParticle(79021,"p_ef_head","n_oh_p_ef_head");

	m_kLastAttackTargetGUID = kTargetGUID;
	m_fLastAttackTargetedTime = g_pkWorld->GetAccumTime();
}

void PgActorMonster::ConcilDirection(NiPoint3 &rkLookingDir, bool const bTurnRightAway)
{
	if( GetFreezed() )
	{
		return;
	}

	CUnit* pkUnit = m_pkPilot->GetUnit();
	BM::GUID kTargetGuid = pkUnit->GetTarget();
	if(m_bLookFocusTarget && kTargetGuid != BM::GUID::NullData())
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot(kTargetGuid);
		if(pkPilot)
		{
			PgActor* pkTargetActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(pkTargetActor)
			{
				NiPoint3 kDir = pkTargetActor->GetPosition() - GetPosition();
				rkLookingDir = kDir;
			}
		}
	}

	rkLookingDir.Unitize();

	PgActor::ConcilDirection(rkLookingDir, bTurnRightAway);
	
	//m_kLookingDir = rkLookingDir;
	//rkLookingDir.z = 0;
	//rkLookingDir.Unitize();	//	leesg213 추가함

	//PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ConcilDirection"), g_pkApp->GetFrameCount()));
	//
	//// 뒤로 밀리는 경우는 이동 방향을 거꾸로
	//NiPoint3 kLookingDir = rkLookingDir * (m_bBackMoving ? -1.0f : 1.0f);
	//NiPoint3 kCross = kLookingDir.UnitCross(NiPoint3::UNIT_Y);
	////WriteToConsole("___________Back Moving : %s rkLookingDir : %.4f, %.4f, %.4f\n", (m_bBackMoving ? "TRUE" : "FALSE"), rkLookingDir.x, rkLookingDir.y, rkLookingDir.z);

	//if(kCross.SqrLength() < 0.0001f)
	//{
	//	// LookingDir이 Y축과 딱 맞으면, UnitCross는 (0,0,0)을 리턴한다.
	//	// 이 경우 MovingDir의 y 값을 가지고 방향을 정해준다.
	//	kCross = (kLookingDir.y > 0 ? NiPoint3::UNIT_Z : -NiPoint3::UNIT_Z);
	//}
	//
	////_PgOutputDebugString("ConcilDirection Actor(%s) rkLookingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),kLookingDir.x,kLookingDir.y,kLookingDir.z);

	//// 방향에 맞도록 회전을 갱신
	//NiQuaternion kRot(NiACos(kLookingDir.Dot(-NiPoint3::UNIT_Y)), kCross);
	//if(m_kToRotation != kRot)
	//{
	//	m_kToRotation = kRot;
	//	if(bTurnRightAway)
	//	{
	//		SetRotation(kRot);
	//		m_fRotationInterpolTime = 1.0f;
	//	}
	//	else
	//	{
	//		m_fRotationInterpolTime = 0.0f;
	//	}
	//}	

	//BYTE byLastDirection = GetDirFromMovingVector(kLookingDir);
	//if(m_bLockBidirection && (byLastDirection & DIR_HORIZONTAL) != byLastDirection)
	//{
	//	return;
	//}

	//// WriteToConsole("Set Looking Direction : %.1f, %.1f, %.1f\n", rkLookingDir.x, rkLookingDir.y, rkLookingDir.z);

	//m_kLookingDir = kLookingDir;
	//m_byLastDirection = byLastDirection;
}

void PgActorMonster::SetLookTarget(bool bLook)
{
	m_bLookFocusTarget = bLook;
}

void PgActorMonster::ResetActiveGrp()
{
	m_uiActiveGrp = MONSTER_ACTIVE_GRP;
}
PgIWorldObject*	PgActorMonster::CreateCopy()
{
	PgActorMonster* pkNewActor = NiNew PgActorMonster();
	PgActor::CreateCopyEx(pkNewActor);
	return	pkNewActor;
}
bool PgActorMonster::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	if(!pkAction)
	{
		NILOG(PGLOG_LOG, "[PgActorMonster::ProcessAction] pkAction(input) is Null");
		return false;
	}

	//NiPoint3 kStartTrn = pkAction->GetActionStartPos();
	//if(kStartTrn != NiPoint3::ZERO)
	//{
	//	// 바닥을 체크 해본다.
	//	kStartTrn = g_pkWorld->FindActorFloorPos(kStartTrn);
	//	pkAction->SetActionStartPos(kStartTrn);
	//}

	// 일반 액션이 들어오고, Action Effect가 수행중 이라면 액션을 버린다.
	pkAction->SetActionTerm(0);
	//if(pkAction->GetActionType() == "EFFECT")
	//{
	//	ClearActionQueue();
	//}
	//else if(!m_pkActionEffectStack->IsEmpty())
	//{
	//	// Action Effect가 수행 중이라면, 액션을 버림.
	//	return true;
	//}

	//if(!pkAction->AlreadySync() && pkAction->GetActionType() != "EFFECT" && pkAction->GetActionTerm() != 0)
	//{
	//	WriteToConsole("________[PgActorMonster::ProcessAction] ActionTerm : %d, Add To ActionQueue : %s\n", pkAction->GetActionTerm(), pkAction->GetID().c_str());
	//	BM::CAutoMutex kLock(m_kActionQueueMutex);
	//	m_kActionQueue.push_back(PgActionEntity(pkAction, DIR_NONE));
	//	return true;
	//}

	// 자체적으로 Transitaion할 때는 StartTrn이 Zero이다. 그럴 때는 좌표 보정을 하면 안된다.

	//std::wstring kActionName = g_SkillDefMgr.GetActionName(pkAction->GetActionNo());
	//if(kActionName == _T("a_walk_left") || kActionName == _T("a_walk_right") || kActionName == _T("a_fly"))
	if( GetFreezed() )
	{
		SetLookTarget(false);				
	}
	else
	{
		bool bLookTerget = !pkAction->GetActionOptionEnable(PgAction::AO_DONT_LOOK_TARGET);
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(pkAction->GetActionNo());
		if(pkSkillDef && bLookTerget)
		{
			if(pkAction->GetActionParam() == ESS_FIRE)	//공격시에만 적용하자 이놈은
			{
				bLookTerget = !(pkSkillDef->GetAbil(AT_FIRE_TYPE) & EFireType_KeepVisionVector);
			}
		}
		
		SetLookTarget(bLookTerget);//!pkAction->GetActionOptionEnable(PgAction::AO_DONT_LOOK_TARGET));

		if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) && !pkAction->GetActionOptionEnable(PgAction::AO_NO_SET_WALKING_TARGET_LOC))
		{
			PgPilot* pkPilot = GetPilot();
			if(!pkPilot)
			{
				return false;
			}
			CUnit* pkUnit = pkPilot->GetUnit();
			if(!pkUnit)
			{
				return false;
			}

			POINT3 kGoalPos = pkUnit->GoalPos();
			NiPoint3 kLoc(kGoalPos.x, kGoalPos.y, kGoalPos.z);
			if(kLoc != NiPoint3::ZERO)
			{
				SetWalkingTargetLoc(kLoc, true, ACTIONNAME_IDLE, true);
			}
		}

		// 몬스터의 경우 타겟리스트의 첫번째 놈을 인지하도록한다.
		if(pkAction->GetTargetList() && pkAction->GetTargetList()->size() > 0)
		{
			PgActionTargetList *pkTargetList = pkAction->GetTargetList();
			PgActionTargetInfo kTargetInfo = *pkTargetList->begin();
			SetAttackTarget(kTargetInfo.GetTargetPilotGUID());
		}		
	}

	//std::wstring kActionName = g_SkillDefMgr.GetActionName(pkAction->GetActionNo());
	//char chBalloon[1024];
	//sprintf_s(chBalloon, 1024, "[%s] Skill : %s [%s]", MB(GetPilot()->GetName().c_str()), MB(kActionName.c_str()), (pkAction->GetActionType() == "EFFECT" ? "EFF" : "NON-EFF"));
	//ShowChatBalloon(CT_NORMAL, chBalloon, 10000);

	bool bResult = PgActor::ProcessAction(pkAction,IsMyActor(),bForceToTransit);
	if( bResult && m_bLookFocusTarget && pkAction->GetActionType() != "IDLE" && false == GetFreezed() )
	{
		ConcilDirection(m_kLookingDir, true);
	}
	
	return bResult;
}

bool PgActorMonster::BeginSync(PgAction *pkAction, DWORD dwOverTime)
{
	if(!pkAction)
	{
		return false;
	}

	if(pkAction->AlreadySync())
	{
		return true;
	}

	PG_STAT(PgStatTimerF timerD(g_kActorStatGroup.GetStatInfo("PgActor.BeginSync"), g_pkApp->GetFrameCount()));
	pkAction->AlreadySync(true);

	// Action Start Pos가 (0, 0, 0)이면 Sync를 할 필요가 없다는 거다.
	NiPoint3 kActionStartPos = pkAction->GetActionStartPos();
	if(kActionStartPos == NiPoint3::ZERO)
	{
		return false;
	}

	if(true == pkAction->GetActionOptionEnable(PgAction::AO_NO_CHANGE_TELEPORT_ACTOR_POS))
	{
		return false;
	}

	//std::wstring kActionName = g_SkillDefMgr.GetActionName(pkAction->GetActionNo());
	//char chBalloon[1024];

	NiPoint3 kCurPos = GetPosition();
	NiPoint3 kDiff = kCurPos - kActionStartPos;
	kDiff.z = 0.0f;
	float fDistance = kDiff.Length();
	//PG_ASSERT_LOG(fDistance > 0.0f)
	if(fDistance > PG_SYNC_DIST_SLIDE || fDistance < PG_SYNC_DIST_WARP)
	{
		if (m_eMonsterType == EMONTYPE_FLYING)
		{
			// 공중형 몬스터 처리
			kActionStartPos.z += static_cast<float>(GetPilot()->GetAbil(AT_HEIGHT));
		}

		SetPosition(kActionStartPos);
		if(fDistance > PG_SYNC_DIST_SLIDE)
		{
			// 먼 곳에서 순간이동 할 때는, Alpha를 뺀다.
			m_AlphaTransitInfo.m_fTargetAlpha = 0.0f;
			SetTargetAlpha(1.0f, 1.0f, 0.15f);
		}
		
		//sprintf_s(chBalloon, 1024, "[%s] Warp(%.4f): %s [%s]", MB(GetPilot()->GetName().c_str()), fDistance, MB(kActionName.c_str()), (pkAction->GetActionType() == "EFFECT" ? "EFFECT" : "NON-EFFECT"));
		//ShowChatBalloon(CT_NORMAL, chBalloon, 10000);
		//WriteToConsole("Action : %s [Distance : %.1f => Warp] [%s]\n", pkAction->GetID().c_str(), fDistance, MB(GetGuid().str()));
		SetMovingDelta(NxVec3(0,0,0));
		m_bSync = false;
		return true;
	}

	// Sync 시작 시간을 적어 둔다.
	//WriteToConsole("Start Interpolating [%s, %s]\n", pkAction->GetID().c_str(), MB(GetGuid().str()));
	//sprintf_s(chBalloon, 1024, "[%s] Start Sync : %s [%s]", MB(GetPilot()->GetName().c_str()), MB(kActionName.c_str()), (pkAction->GetActionType() == "EFFECT" ? "EFFECT" : "NON-EFFECT"));
	//ShowChatBalloon(CT_NORMAL, chBalloon, 10000);

	m_bSync = true;
	m_fElapsedSyncTime = 0.0f;
	m_kSyncPositionStart = kCurPos;
	m_kSyncPositionTarget = kActionStartPos;

	NiPoint3 kMovingDir = (kCurPos - kActionStartPos);
	kMovingDir.Unitize();

	if(m_bFloor && m_pkAction && m_pkAction->GetActionType() != "EFFECT")
	{
		//	뛰기 모션으로 바꾸자
		NiActorManager *pkAM = GetActorManager();
		if(pkAM)
		{
			NiActorManager::SequenceID kSeqID;
			
			pkAM->GetCurAnimation();
			GetActionSlot()->GetAnimation(std::string("run"), kSeqID);
			if(pkAM->GetCurAnimation() != kSeqID)
			{
				if(SetTargetAnimation(std::string("run")) == false)
				{
					SetTargetAnimation(std::string("walk"));
				}
			}
		}
	}

//	NILOG(PGLOG_LOG,"Push To Action Stack For Sync Position  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()),
//		pkAction->GetID().c_str(), pkAction->GetActionNo(), pkAction->GetActionInstanceID(),
//		pkAction->GetTargetList()->size());
	
	// 보간할 때 속도는 지연시간에 의해 결정된다.
//	DWORD dwActionBirthTime = pkAction->GetBirthTime();
//	m_dwActionLatency = (dwActionBirthTime == 0 ? 0 : PgActor::GetSynchronizedTime() - dwActionBirthTime);

	return false;
}

bool PgActorMonster::UpdateSync(float fFrameTime)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateInitPosition"), g_pkApp->GetFrameCount()));
	
	EUnitType eUType= GetPilot()->GetUnit()->UnitType();

	// Sync 시작, 끝 좌표를 설정.
	NiPoint3 kStartPos = m_kSyncPositionStart;
	NiPoint3 kSyncTargetPos = m_kSyncPositionTarget;
	NiPoint3 kMovVector = (kSyncTargetPos - kStartPos);
	kMovVector.Unitize();
	
	// 아무리 느려도 0.5초 안에 보간 되게 하자
	static float fCriticalTime = 0.75f;
	float fDistance1 = (kStartPos - kSyncTargetPos).Length();
	m_fElapsedSyncTime += fFrameTime;
	
	float fSpeed = static_cast<float>(GetPilot()->GetAbil(AT_C_MOVESPEED));
	if(g_pkWorld)
	{
		if(g_pkWorld->GetAttr() & GATTR_VILLAGE)
		{
			fSpeed += static_cast<float>(GetPilot()->GetAbil(AT_C_VILLAGE_MOVESPEED));
		}
	}
	
	// 이동 속도 계산
	float fMovingSpeed = NiMax(fSpeed * 2.0f, fDistance1 / fCriticalTime);
	
	float fUnitizedDiffHeight = kMovVector.z;

	kMovVector.z = 0.0f;
	kMovVector.Unitize();
	kMovVector = kMovVector * fMovingSpeed * fFrameTime;
	kMovVector.z += fUnitizedDiffHeight * fMovingSpeed * fFrameTime;
	NiPoint3 kNextPos = GetPosition() + kMovVector;

	float fDistance2 = (kStartPos - kNextPos).Length();

	//WriteToConsole("Now Interpolating : %.1f, %.1f, %.1f\tAdjusted Speed : %.1f\tDistance %.4f[%s]\n", kMovVector.x, kMovVector.y, kMovVector.z, fMovingSpeed, fDistance2, MB(GetGuid().str()));
	
	// TODO : 장애물 때문에 대각선이나 한 층 아래로 걷게 되었다면, 당연히 튄다 => 순간 이동?
	// Z축에 대한 값을 어떻게 조절 할 것인가. 
	
	// 시작위치에서 다음 좌표까지의 거리가, 시작위치에서 목표좌표까지의 거리보다 크다면, 지나친것이다.
	// 보간 시간이 지났으면 순간이동 시키고 바로 보간을 끝낸다.
	if(fDistance1 <= fDistance2 || (kMovVector == NiPoint3::ZERO) || m_fElapsedSyncTime >= fCriticalTime)
	{
		// 목표 지점 도달, 해당 액션을 한다.
		SetPosition(kSyncTargetPos);
		m_fElapsedSyncTime = 0.0f;
		m_bSync = false;

		ProcessActionQueue();
		return true;
	}

	float fDeltaZ = 0.0f;
	if (m_eMonsterType == EMONTYPE_FLYING)
	{
		NxExtendedVec3 kCurPos = m_pkController->getDebugPosition();
		fDeltaZ = AdjustToFly(NiPoint3((float)kCurPos.x, (float)kCurPos.y, (float)kCurPos.z), 1.0f);
		m_bJump = false;
	}	

	// TODO : move로 움직이는 것이 나을지, SetPosition으로 움직이는게 나을지.
	NxVec3 kMovingDelta;
	NxU32 collisionFlag = 0;
	NiPhysXTypes::NiPoint3ToNxVec3(kMovVector, kMovingDelta);
	kMovingDelta.z += fDeltaZ;
	m_pkController->move(kMovingDelta, m_uiActiveGrp, 0.001f, collisionFlag);

	//WriteToConsole("[%s]'s interpolated pos : [%.1f, %.1f, %.1f]\n", m_pkAction->GetID().c_str(), m_pkController->getDebugPosition().x, m_pkController->getDebugPosition().y, m_pkController->getDebugPosition().z);
	//_PgOutputDebugString("[Set m_kMovingDir 3] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);

	return false;
}

bool PgActorMonster::ProcessActionQueue()
{
	if(m_bSync)
	{
		return true;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActorMonster.ProcessActionQueue"), g_pkApp->GetFrameCount()));

	// 새로온 패킷의 속도가, 이전 패킷의 속도보다 빠를 때만 그만큼 시간을 늦게 두어서 패킷을 처리함.
	BM::CAutoMutex kLock(m_kActionQueueMutex);

	bool bReturn = true;
	ActionQueue::iterator itr = m_kActionQueue.begin();
	while(itr != m_kActionQueue.end())
	{
		PgActionEntity &rkActionEntity = *itr;
		
		DWORD dwNow = BM::GetTime32();
		if(dwNow <= m_dwLastActionTime)
		{
			// LastActionTime이 수정되면, 여기에 걸릴 수 있으나, 이제 수정될 일이 없음.
			//WriteToConsole("Now[%u] <= LastAction[%u]\n", dwNow, m_dwLastActionTime);
			return true;
		}
		
//		PG_ASSERT_LOG(dwNow > m_dwLastActionTime);

		DWORD dwActionTerm = rkActionEntity.GetActionTerm();
		DWORD dwElapsedTime = (m_dwLastActionTime != 0 ? dwNow - m_dwLastActionTime : dwActionTerm);
//		WriteToConsole("ElapsedTime[%u] = Now[%u] - LastAction[%u] (Action Term : %u)\n", 
//						dwElapsedTime, dwNow, m_dwLastActionTime, dwActionTerm);

		bool bForceToProcessAction = false;
		if(dwElapsedTime < dwActionTerm)
		{
			if(m_dwAccumedOverTime > 0 && m_pkAction && m_pkAction->AlreadySync())
			{
				if(m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) || 
					m_pkAction->GetActionOptionEnable(PgAction::AO_LOOP))
				{
					bForceToProcessAction = true;
				}
			}
			else
			{
				// 아직 액션을 할 때가 아니다.
				//char szBuff[1024];
				//if(rkActionEntity.GetAction())
				//{
				//	sprintf(szBuff, ", NextAction : %s\n", rkActionEntity.GetAction()->GetID().c_str());
				//}
				//else
				//{
				//	sprintf(szBuff, ", NextDirection : %u\n", rkActionEntity.GetDirection());
				//}
				//WriteToConsole("[Wait For Current Action Term] Current Action : %s%s", m_pkAction->GetID().c_str(), szBuff);
				return true;
			}
		}

		DWORD dwSyncTime = 0;
		DWORD dwOverTime = dwElapsedTime - dwActionTerm;
		//WriteToConsole("[ProcessNextAction] Action's OverTime : %u (%u - %u)\t", dwOverTime, dwElapsedTime, dwActionTerm);

		PgAction *pkAction = const_cast<PgAction*>(rkActionEntity.GetAction());
		if(pkAction)
		{
			if(bForceToProcessAction)
			{
				DWORD dwRemainTime = dwActionTerm - dwElapsedTime;
				DWORD dwNewActionTerm = 0;
				PG_ASSERT_LOG(dwRemainTime);
				
				//WriteToConsole("[Reward Synctime 2.1] RemainTime[%u] = ActionTerm[%u] - ElapsedTime[%u]\n", dwRemainTime, dwActionTerm, dwElapsedTime);
				if(m_dwAccumedOverTime <= dwRemainTime)
				{
					m_dwAccumedOverTime = 0;
					pkAction->SetActionTerm(dwRemainTime - m_dwAccumedOverTime);
					//WriteToConsole("[Reward Synctime 2.1.2] SetActionTerm[%u]\n", pkAction->GetActionTerm());
					return true;
				}
				else
				{
					m_dwAccumedOverTime -= dwRemainTime;
					pkAction->SetActionTerm(0);
					dwOverTime = 0;
					//WriteToConsole("[Reward Synctime 2.1.3] Remaind AccummedTime : %u\n", m_dwAccumedOverTime);
				}
			}

			if(!BeginSync(pkAction, dwOverTime))
			{
				//WriteToConsole("[BeginSync] LastAction Time [%u] : return false\n", m_dwLastActionTime);
				PG_ASSERT_LOG(m_bSync);
				return true;
			}
			
			ProcessAction(pkAction,IsMyActor());
			bReturn = false;
			m_dwLastActionTime = dwNow;
			//WriteToConsole("[ProcessAction] Action : %s, LastActionTime : %u\n", pkAction->GetID().c_str(), m_dwLastActionTime);
		}
		else
		{
			SetDirection(rkActionEntity.GetDirection());
			SetPosition(rkActionEntity.GetDirectionStartPos());
			//WriteToConsole("[ProcessDirection] Direction : %u\n", rkActionEntity.GetDirection());
		}

		// 매 틱당 OverTime이 발생하는데, 이것을 합산해서 Now에서 빼주어야 한다.
		m_dwAccumedOverTime += dwOverTime;
		itr = m_kActionQueue.erase(itr);
	}

	return bReturn;
}

bool PgActorMonster::DoSpeech(EUnitState const eUnitState)
{
	if( !m_pkPilot )
	{
		return false;
	}

	//	말은 30%확률로 한다
	if( BM::Rand_Index(100) > 10 )
	{
		return false;
	}

	int iMonsterNo = m_pkPilot->GetAbil(AT_CLASS);

	int iTalkNo = 0;
	switch( eUnitState )
	{
	case US_IDLE:	{ iTalkNo = m_pkPilot->GetAbil(AT_TALK_IDLE);	}break;
	case US_ATTACK: { iTalkNo = m_pkPilot->GetAbil(AT_TALK_ATTACK); }break;
	case US_MOVE:	{ iTalkNo = m_pkPilot->GetAbil(AT_TALK_MOVE);	}break;
	case US_DAMAGEDELAY:
	case US_PUSHED_BY_DAMAGE:
	case US_DAMAGEDELAY_NOCHANGE:
		{
			iTalkNo = m_pkPilot->GetAbil(AT_TALK_HIT);
		}break;
	default:
		{
			return false;
		}
	}
	//AT_TALK_BIDLE			= 6021,		// 전투 기본 상태

	int iSpeechNo = 0;
	if( PgRareMonsterSpeech::GetSpeech(iTalkNo, iSpeechNo) )
	{
		wchar_t const* pkSpeech = NULL;
		if( true == GetDefString(iSpeechNo, pkSpeech) )
		{
			SChatLog	kChatLog(CT_NORMAL);
			kChatLog.kCharName = m_pkPilot->GetName();
			kChatLog.kContents = pkSpeech;
			g_kChatMgrClient.AddLogMessage(kChatLog, kChatLog.kContents, false, 0, true, false, true);
			ShowChatBalloon(CT_NORMAL, pkSpeech);
		}
	}

	return true;
}

//! PhysX를 초기화한다.
void PgActorMonster::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	__super::InitPhysX(pkPhysXScene, uiGroup);

	if(m_bCanRide == false)
	{
		return;
	}

	InitRidingInfo(pkPhysXScene, PG_PHYSX_GROUP_MONSTER-1);
}

bool PgActorMonster::UpdateName(std::wstring const& rkName)
{
	__super::UpdateName(rkName);
	PgPilot	*pPilot = GetPilot();
	if( NULL==pPilot )	
	{
		return false;
	}
	CUnit* pkUnit = pPilot->GetUnit();
	if( NULL==pkUnit )
	{
		return false;
	}

	int iEle = pkUnit->GetAbil(AT_ELEMENT_MONSTER);
	if(0>=iEle)
	{
		return true;
	}

	// 속성 마크
	if( !m_spElementMark )
	{
		NiNode* pkMarkTargetNode = NiDynamicCast(NiNode, GetObjectByName(ATTACH_POINT_STAR));
		if( pkMarkTargetNode )
		{
			m_spElementMark = NiNew PgElementMark();
			m_spElementMark->Init(STR_DEFAULT_ELEMENT_MARK_IMG, NiPoint2(32.0f, 32.0f));//생성자에서 왠만하면 함수 호출 금지
			pkMarkTargetNode->AttachChild(m_spElementMark, true);
		}
	}
	if( m_spElementMark 
		&& m_spNameText
		)
	{
		float fNameWidth = static_cast<float>(m_spNameText->GetTextWidth());
		m_spElementMark->Set( iEle-1, fNameWidth + PgActorUtil::fNameWidthGap);//0베이스
		NiPoint3 kPos = m_spElementMark->GetTranslate();
		kPos.z = PgActorUtil::FindNamePosZ(this) + PgActorUtil::fAddedGuildMarkZ;
		m_spElementMark->SetTranslate(kPos);
	}

	return true;
}

bool PgActorMonster::BeforeCleanUp()
{
	bool bRet = __super::BeforeCleanUp();
	m_spElementMark = 0;
	return bRet;
}

void PgActorMonster::DrawNameText(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	PG_ASSERT_LOG(pkCamera);

	__super::DrawNameText(pkRenderer, pkCamera);

#ifndef EXTERNAL_RELEASE
	if(lua_tinker::call<bool>("DrawNameText"))
#endif
	{
		if(m_spElementMark)
		{
			m_spElementMark->SetAppCulled(true);//일단 숨김
			if(true==m_bDrawNameNPC && GetNameVisible() && !m_bNoName)
			{
				m_spElementMark->SetAppCulled(false);
			}
		}
	}
}

//bool PgActorMonster::UpdateSync(float fFrameTime)
//{	
//	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateInitPosition"), g_pkApp->GetFrameCount()));
//	
//	// Sync 시작, 끝 좌표를 설정.
//	NiPoint3 kStartPos = m_kSyncPositionStart;
//	NiPoint3 kSyncTargetPos = m_kSyncPositionTarget;
//	NiPoint3 kMovVector = (kSyncTargetPos - kStartPos);
//	kMovVector.Unitize();
//	
//	// 아무리 느려도 0.1초 안에 보간 되게 하자
//	static float const fCriticalTime = 0.1f;
//	float fDistance1 = (kStartPos - kSyncTargetPos).Length();
//
//	m_fElapsedSyncTime += fFrameTime;
//	
//	// 이동 속도 계산
//	float fMovingSpeed = NiMax(GetPilot()->GetAbil(AT_C_MOVESPEED) * 2.0f, fDistance1 / fCriticalTime);
//	float fUnitizedDiffHeight = kMovVector.z;
//
//	kMovVector.z = 0.0f;
//	kMovVector.Unitize();
//	kMovVector = kMovVector * fMovingSpeed * fFrameTime;
//	kMovVector.z += fUnitizedDiffHeight * fMovingSpeed * fFrameTime;
//	NiPoint3 kNextPos = GetPosition() + kMovVector;
//
//	float fDistance2 = (kStartPos - kNextPos).Length();
//
//	//WriteToConsole("Now Interpolating : %.1f, %.1f, %.1f\tAdjusted Speed : %.1f\n", kMovVector.x, kMovVector.y, kMovVector.z, fMovingSpeed);
//	
//	// TODO : 장애물 때문에 대각선이나 한 층 아래로 걷게 되었다면, 당연히 튄다 => 순간 이동?
//	// Z축에 대한 값을 어떻게 조절 할 것인가. 
//	
//	// 시작위치에서 다음 좌표까지의 거리가, 시작위치에서 목표좌표까지의 거리보다 크다면, 지나친것이다.
//	// 보간 시간이 지났으면 순간이동 시키고 바로 보간을 끝낸다.
//	if(fDistance1 <= fDistance2 || (kMovVector == NiPoint3::ZERO) || m_fElapsedSyncTime >= fCriticalTime)
//	{
//		// 목표 지점 도달, 해당 액션을 한다.
//		SetPosition(kSyncTargetPos);
//		m_fElapsedSyncTime = 0.0f;
//		m_bSync = false;
//		//WriteToConsole("Arrived [%s]'s Start Pos Now [%.1f, %.1f, %.1f]\n", m_pkAction->GetID().c_str(), kSyncTargetPos.x, kSyncTargetPos.y, kSyncTargetPos.z);
//
//		ProcessActionQueue();
//		return true;
//	}
//	
//	// TODO : move로 움직이는 것이 나을지, SetPosition으로 움직이는게 나을지.
//	NxVec3 kMovingDelta;
//	NxU32 collisionFlag = 0;
//	NiPhysXTypes::NiPoint3ToNxVec3(kMovVector, kMovingDelta);
//	m_pkController->move(kMovingDelta, m_uiActiveGrp, 0.001f, collisionFlag);
//
//	//WriteToConsole("[%s]'s interpolated pos : [%.1f, %.1f, %.1f]\n", m_pkAction->GetID().c_str(), m_pkController->getDebugPosition().x, m_pkController->getDebugPosition().y, m_pkController->getDebugPosition().z);
//	kMovVector.Unitize();
//	m_kMovingDir = kMovVector;
//	m_kMovingDir.z = 0;
//	_PgOutputDebugString("[Set m_kMovingDir 3] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);
//}