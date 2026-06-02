#include "stdafx.h"
#include "PgCameraModeEvent.H"
#include "PgWorld.H"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "PgMath.H"

PgCameraModeEvent::PgCameraModeEvent(NiCamera *pkCamera)
:m_fMoveStartTime(0),
m_fTotalMoveTime(0),
m_fCamDistance(0),
m_fCamMoveUpDistance(0),
m_fTotalMoveDistance(0),
m_fHorizRotAngle(0),
m_fVertRotAngle(0),
m_kState(S_STOPPED),
m_fA(0),
m_fB(0),
PgICameraMode(pkCamera)
{
}
bool PgCameraModeEvent::Update(float fFrameTime)
{

	switch(m_kState)
	{
	case	S_MOVE_TO_TARGET_POS:
		{
			UpdateMoveToTargetPos(fFrameTime);
		}
		break;
	}

	return	true;
}

void	PgCameraModeEvent::UpdateMoveToTargetPos(float fFrameTime)
{
	if(!g_pkWorld)
	{
		return;
	}

	float	fNowTime = g_pkWorld->GetAccumTime();

	float	fElapsedTime = fNowTime - m_fMoveStartTime;
	float	fRate = 1;
	
	if(m_fTotalMoveTime>0)
	{
		float	fMoveDistance = m_fA*(fElapsedTime*fElapsedTime*fElapsedTime) + m_fB*(fElapsedTime*fElapsedTime);	//	y = ax^3+bx^2
		fRate = fMoveDistance / m_fTotalMoveDistance;
	}

	if(fRate>1 || fElapsedTime>=m_fTotalMoveTime)
	{
		fRate = 1;
	}

	NiPoint3	const	kNextPos = m_kStartCamPos + (m_kCamTargetPos - m_kStartCamPos)*fRate;
	GetCamera()->SetTranslate(kNextPos);
	GetCamera()->Update(fNowTime);
	
	NiPoint3	kLookAt1 = m_kStartCamPos + m_kStartPathNormal*1000;
	NiPoint3	kLookAt2 = m_kLookTargetPos - m_kCamTargetPos;
	kLookAt2.Unitize();
	kLookAt2 = m_kCamTargetPos + kLookAt2*1000;

	NiPoint3	const	kNextLookAt = kLookAt1 + (kLookAt2 - kLookAt1)*fRate;
	GetCamera()->LookAtWorldPoint(kNextLookAt,NiPoint3::UNIT_Z);

	if(fRate == 1)
	{
		m_kState = S_STOPPED;
	}
}

void	PgCameraModeEvent::StartMoveCamera(float fMoveTime,float fCamDistance,float fCamMoveUpDistance,float fHorizRotAngle,float fVertRotAngle)
{
	if(!g_pkWorld)
	{
		return;
	}

	m_fMoveStartTime = g_pkWorld->GetAccumTime();
	m_fTotalMoveTime = fMoveTime;

	m_fCamDistance = fCamDistance;
	m_fCamMoveUpDistance= fCamMoveUpDistance;
	m_fHorizRotAngle = fHorizRotAngle;
	m_fVertRotAngle = fVertRotAngle;

	m_kStartCamPos = GetCamera()->GetWorldTranslate();
	m_kStartPathNormal = GetCamera()->GetWorldDirection();

	NiPoint3	kPathNormal;
	if(g_pkWorld->FindPathNormal(m_kTargetPos,kPathNormal))
	{
		m_kTargetPathNormal = kPathNormal;
	}
	else
	{
		m_kTargetPathNormal = m_kStartPathNormal;
	}

	//	수직 상승 적용(m_fCamMoveUpDistance)
	m_kLookTargetPos = m_kTargetPos + m_kTargetPathNormal * 100.0f + NiPoint3::UNIT_Z * m_fCamMoveUpDistance;

	//	수직각 적용(m_fVertRotAngle),타겟으로부터의 카메라 거리 적용(m_fCamDistance)
	float	fTangent = tan(m_fVertRotAngle*3.141592/180.0);

	float	const	fCamDistanceHoriz = sqrt( (m_fCamDistance*m_fCamDistance)/(fTangent*fTangent+1) );
	float	const	fCamDistanceVert = fTangent * fCamDistanceHoriz;

	m_kCamTargetPos = -m_kTargetPathNormal * fCamDistanceHoriz;
	m_kCamTargetPos.z += fCamDistanceVert;

	//	수평각 적용(m_fHorizRotAngle)
	NiQuaternion	kQuat(m_fHorizRotAngle*3.141592f/180.0f,NiPoint3::UNIT_Z);
	NiMatrix3	kRotMat;

	kQuat.ToRotation(kRotMat);
	m_kCamTargetPos = m_kCamTargetPos*kRotMat + m_kTargetPos;

	//	카메라 가속 이동 관련 값 구하기
	if(m_fTotalMoveTime>0)
	{
		m_fTotalMoveDistance = (m_kCamTargetPos - m_kStartCamPos).Length();
		if(m_fTotalMoveDistance == 0)
		{
			return;
		}

		m_fA = -(2*m_fTotalMoveDistance/(m_fTotalMoveTime*m_fTotalMoveTime*m_fTotalMoveTime));
		m_fB = 3*m_fTotalMoveDistance/(m_fTotalMoveTime*m_fTotalMoveTime);
	}

	m_kState = S_MOVE_TO_TARGET_POS;
}

void	PgCameraModeEvent::SetTargetActor(BM::GUID const &kTargetActor)
{
	PgActor	*pkActor = g_kPilotMan.FindActor(kTargetActor);
	if(!pkActor)
	{
		return;
	}

	NiPoint3	kPos = pkActor->GetPosition();
	if(pkActor->GetPilot() && pkActor->GetPilot()->GetAbil(AT_MONSTER_TYPE)==EMONTYPE_FLYING)
	{
	}
	else
	{
		kPos.z = pkActor->GetLastFloorPos().z;
	}
	
	SetTargetPos(kPos);
}
void	PgCameraModeEvent::SetTargetPos(NiPoint3 const &kTargetPos)
{
	m_kTargetPos = kTargetPos;
}
