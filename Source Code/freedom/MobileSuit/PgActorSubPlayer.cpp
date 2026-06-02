#include "StdAfx.h"
#include "lwActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
//#include "PgMobileSuit.h"
#include "PgStat.h"
#include "PgInterpolator.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "PgActorSubPlayer.h"

PgActorSubPlayer::PgActorSubPlayer()
{
}

PgActorSubPlayer::~PgActorSubPlayer()
{
}

float PgActorSubPlayer::TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate)
{
	bool bTracing = false;
	NiPoint3 const &rkCurPos = GetPos();
	NiPoint3 kTempTargetLoc = m_kTargetLoc;
	/*if(stFollowInfo::FS_MOVE_TO_STARTPOS==m_kFollowInfo.m_kFollowState)
	{
		if(m_kFollowInfo.m_kSavePoint != NiPoint3::ZERO)
		{
			kTempTargetLoc = m_kFollowInfo.m_kSavePoint;
		}
		fLimitDistance = 0.0f;
		fAccelateScale = 5.0f;
	}*/
	NiPoint3 kTempCurLoc = rkCurPos;
	kTempTargetLoc.z = 0;
	kTempCurLoc.z = 0;
	NiPoint3 kDiffPos = kTempTargetLoc - kTempCurLoc;
	float fDistance = kDiffPos.Length();
	kDiffPos.Unitize();
	NiPoint3 kNextPos = GetTranslate();
	SetFreeMove(false);
	bool bZDist = 130 < fabs(rkCurPos.z - m_kTargetLoc.z);
	
	bool bIsFloor = true;
	int iActionNo = 0;
	bool bIsRiding = false;
	PgPilot* pkPilot = GetPilot();
	if(pkPilot)
	{
		PgActor* pkActor = g_kPilotMan.FindActor(pkPilot->GetUnit()->Caller());
		if(pkActor)
		{
			bIsFloor = pkActor->IsMeetFloor();
			if(pkActor->GetAction())
			{
				iActionNo = pkActor->GetAction()->GetActionNo();
			}
			bIsRiding = pkActor->IsOnRidingObject();
		}
	}
	
	bool bTelePort = false;
	if(150<fDistance /*&& stFollowInfo::FS_MOVE_TO_STARTPOS!=m_kFollowInfo.m_kFollowState*/ || bZDist)
	{
		if(!bIsFloor)	
		{
			return 0.0f;
		}
		
		if( !bIsRiding
			&& !IsHide()
			)
		{
			kNextPos = m_kTargetLoc;
			bTelePort = true;
			NiAVObject *pkParticle = g_kParticleMan.GetParticle("ef_show_01_p_ef_heart",PgParticle::O_SCALE, 1.0f );
			if(pkParticle)
			{
				if(!AttachTo(1010, "p_ef_heart", pkParticle))
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
			}
		}
	}
	
	if( fLimitDistance < fDistance && !bZDist && !bTelePort)
	{
		// 열심히 쫒아가자
		kNextPos = rkCurPos + (kDiffPos * fSpeed * fFrameTime) + (kDiffPos * (fDistance-fLimitDistance) * fAccelateScale * fFrameTime);

		NiPoint3 const kTempDown = GetWorld()->ThrowRay(NiPoint3(kNextPos.x, kNextPos.y, kNextPos.z+50.0f), NiPoint3(0.0f,0.0f,-1.0f), 200.0f);
		NiPoint3 const kTempPos = GetWorld()->ThrowRay(NiPoint3(kNextPos.x, kNextPos.y, m_kTargetLoc.z+10), NiPoint3(0.0f,0.0f,-1.0f), 1000.0f);	// 주인이 어느정도 높이에 떠 있나?
		//if(-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z)	//바닥이 허공
		if((-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z) || 95.0f < rkCurPos.z - kTempPos.z)
		{
			NiPoint3 kFrontDir = kDiffPos;
			kFrontDir.Unitize();
			NiPoint3 const kTempFront = GetWorld()->ThrowRay(GetLastFloorPos(), kFrontDir, 40.0f);	//앞으로 레이
			if(-1==kTempFront.x && -1==kTempFront.y && -1==kTempFront.z)
			{
				SetTraceFlyTargetLoc(rkCurPos);//멈추자 일단
				fDistance = 0.0f;
				return fDistance;
			}
		}
		else
		{
			kNextPos.z = kTempPos.z + 25.0f;
		}

		if(-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z)	//바닥이 허공
		{
			if(!bIsFloor)
			{
				if(100005440 == iActionNo)	//텔레점프
				{
					SetFreeMove(true);//이놈은 진짜 대점프를 탄 상태일 때만 해 주자
					kNextPos.z = m_kTargetLoc.z;
				}
				else//바닥이 허공. 난 바닥. 주인은 허공
				{
					SetTraceFlyTargetLoc(rkCurPos);//멈추자 일단
					kNextPos = GetTranslate();
					fDistance = 0.0f;
				}
			}
		}
		else if(bIsFloor)
		{
			bTracing = true;
		}
		kNextPos.z = kTempPos.z + 25.0f;
	}

	if(GetTranslate() != kNextPos)
	{
		m_pkController->setPosition( NxExtendedVec3(kNextPos.x, kNextPos.y, kNextPos.z) );
		SetTranslate(kNextPos);
		SetWorldTranslate(kNextPos);

		if( bCanRotate )
		{
			SetMovingDir(kDiffPos);
			ConcilDirection(kDiffPos, true);
		}
		SetPositionChanged(true);
	}

	/*if(stFollowInfo::FS_MOVE_TO_STARTPOS==m_kFollowInfo.m_kFollowState)
	{
		if(1.0f>=fDistance)
		{
			m_kFollowInfo.m_kFollowState=stFollowInfo::FS_PROCESS_ACTION_QUEUE;
			m_kFollowInfo.m_kSavePoint = NiPoint3::ZERO;
		}
		else
		{
			fDistance = 61.0f;
		}
	}*/
	return fDistance;
}

void PgActorSubPlayer::ClearPetActionQueue()
{
	BM::CAutoMutex kLock(m_kPetMutex);
	size_t kSize = m_kPetActionQueue.size();
	for(size_t i = 0; i < kSize; ++i)
	{
		m_kPetActionQueue.pop();
	}
}

bool PgActorSubPlayer::MakePetActionQueue(int iGrade)
{
	ClearPetActionQueue();
	if(!GetPilot() || !g_pkWorld)	{return false;}
	//PgSubPlayer* pkPet = dynamic_cast<PgSubPlayer*>(GetPilot()->GetUnit());
	//if(!pkPet)						{return false;}
	//if(EPET_TYPE_1==m_ePetType)	//1차펫
	//{return false;}

	int iIdleSlot = 0;
	if( 3 > iGrade )
	{
		iIdleSlot = 0;
	}
	else if( 4 > iGrade )
	{
		iIdleSlot = 1;
	}
	else
	{
		iIdleSlot = 2;
	}

	float fAccum = g_pkWorld->GetAccumTime();
	ClearPetActionQueue();
	BM::CAutoMutex kLock(m_kPetMutex);
	for(int i = 0; i < 6; ++i)
	{
		fAccum+=10.0f;
		SPetActionEle kEle;
		kEle.fEndTime = fAccum;
		kEle.bIsLoop = false;	//아이들
		kEle.iSlot = iIdleSlot;
// 		if(0==i%2)
// 		{
// 			for(int k = 0; k < 3; ++k)	//어빌 3종류
// 			{
// 				int iTemp = iAbil + k;
// 				if(AT_MENTAL<iTemp)
// 				{
// 					iTemp = AT_HUNGER;
// 				}
// 				if(3000>pkPet->GetAbil(iTemp))
// 				{
// 					kEle.iSlot = 3+iTemp-AT_HUNGER;//기본+k가 되야 함
// 					kEle.bIsLoop = true;	//아이들
// 					break;	
// 				}
// 			}
// 			++iAbil;
// 		}

		m_kPetActionQueue.push(kEle);
	}

	return false==m_kPetActionQueue.empty();
}

int PgActorSubPlayer::UpdatePetActionQueue(float fAccumTime)
{
//	if(EPET_TYPE_1==m_ePetType)	{return 0;}
	BM::CAutoMutex kLock(m_kPetMutex);
	if(m_kPetActionQueue.empty())	
	{
		if(MakePetActionQueue())
		{
			return m_kPetActionQueue.front().iSlot;
		}
		return 0;
	}
	SPetActionEle const& rkEle = m_kPetActionQueue.front();
	int iSlot = rkEle.iSlot;
	if(rkEle.IsEnd(fAccumTime))
	{
		bool bAniDone = IsAnimationDone();
		if(rkEle.bIsLoop || bAniDone)
		{
			m_kPetActionQueue.pop();
			return UpdatePetActionQueue(fAccumTime);
		}
	}
	return iSlot;
}