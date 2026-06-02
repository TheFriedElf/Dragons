#include "StdAfx.h"
#include "PgCameraModeCircle.h"
#include "PgCameraModeFollow.h"
#include "PgInterpolator.h"
#include "PgDamper.h"
#include "PgWorld.h"
#include "PgActor.h"

extern NiQuaternion g_kSpeed;

PgCameraModeCircle::PgCameraModeCircle(NiCamera *pkCamera, PgActor *pkActor)
: PgICameraMode(pkCamera)
{
	m_pkActor = pkActor;
	m_kPrevTrn = NiPoint3::ZERO;

	// x : Spring Const
	// y : Damp Const
	// z : Spring Length
	// w : Rot speed
	g_kSpeed = NiQuaternion(4.5f, 2.0f, 4.0f, 20.0f);
}

PgCameraModeCircle::~PgCameraModeCircle()
{
}

bool PgCameraModeCircle::Update(float fFrameTime)
{
	bool bRetT = UpdateTranslate(fFrameTime);
	bool bRetR = UpdateRotate(fFrameTime);
	
	if(bRetT == false && bRetR == false)
	{
		return false;
	}

	return true;
}

bool PgCameraModeCircle::UpdateTranslate(float fFrameTime)
{
	PG_ASSERT_LOG(m_pkCamera);
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL || m_pkCamera == NULL)
		return false;

	PG_ASSERT_LOG(m_pkActor->GetWorld());	

	NiPoint3 const &kCamTrn = m_pkCamera->GetTranslate();
	NiPoint3 const &kActorTrn = m_pkActor->GetTranslate();
	NiPoint3 kNewCamTrn;

	// 바닥 좌표를 갱신
	// (높이는 바닥으로 다시 찾을 것이므로 갱신하지 말것!)
	m_kFloor.x = kActorTrn.x;
	m_kFloor.y = kActorTrn.y;

	// 바닥을 찾는다.
	NxScene *pkScene = m_pkActor->GetWorld()->GetPhysXScene()->GetPhysXScene();
	NxRaycastHit kHit;	
	if(g_pkWorld
		&& pkScene && g_pkWorld->raycastClosestShape(
		NxRay(NxVec3(kActorTrn.x, kActorTrn.y, kActorTrn.z + 10.0f), NxVec3(0, 0, -1)),
		NX_STATIC_SHAPES, kHit, -1, 300, NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT))
	{
		m_kFloor.z = kHit.worldImpact.z;
	}
	else
	{
		m_kFloor.z = kActorTrn.z;
	}
	
	NiPoint3 kCamDir = m_kFloor - NiPoint3(0,0,0);
	kCamDir.Unitize();

	kNewCamTrn = m_kFloor + kCamDir * (PgCameraModeFollow::GetCameraInfo().kCurrentPos.fDistanceFromTarget + m_kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget);
	kNewCamTrn.z += (PgCameraModeFollow::GetCameraInfo().kCurrentPos.fCameraHeight + m_kCameraAdjustInfo.kCameraAdjust.fCameraHeight);

	// 위치의 변화가 없다면
	if(kNewCamTrn == kCamTrn)
	{
		return false;	// 업데이트 하지말라
	}

	// 카메라 이동을 적용
	// fFrameTime = NiMin(fFrameTime * 0.4f, 0.1f);
	m_pkCamera->SetTranslate(PgDamper::SpringDamp(kCamTrn, kNewCamTrn, m_kPrevTrn, fFrameTime, g_kSpeed.GetX(), g_kSpeed.GetY(), g_kSpeed.GetZ()));
	// m_pkCamera->SetTranslate(kNewCamTrn);
	
	// 스프링 뎀퍼를 위해서 마지막 목표 위치를 저장한다.
	m_kPrevTrn = kNewCamTrn;

	return true;
}

bool PgCameraModeCircle::UpdateRotate(float fFrameTime)
{
	PG_ASSERT_LOG(m_pkCamera);
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkCamera == NULL || m_pkActor == NULL)
		return false;
	
	NiPoint3 kFloor;
	NiQuaternion kNewCamRot;
	NiQuaternion kCamRot;
	
	// 윗쪽 여백을 적용한다.
	kFloor = m_kFloor;
	kFloor.z += (PgCameraModeFollow::GetCameraInfo().kCurrentPos.fLookAtHeight + m_kCameraAdjustInfo.kCameraAdjust.fLookAtHeight);

	m_pkCamera->GetRotate(kCamRot);

	// 목표 지점을 바라보는 각도를 구한다.
	m_pkCamera->LookAtWorldPoint(kFloor, NiPoint3::UNIT_Z);

	m_pkCamera->GetRotate(kNewCamRot);
	// m_pkCamera->SetRotate(kCamRot);

	// 쿼터니언 축이 뒤틀리는 부분이면
	if(NiQuaternion::Dot(kCamRot, kNewCamRot) < 0)
	{
		// 축을 바로잡자
		NiPoint3 kAxis;
		float fAngle;

		kCamRot.ToAngleAxis(fAngle, kAxis);
		kCamRot.FromAngleAxis(-fAngle + NI_TWO_PI, -kAxis);
	}
	// 시간 기반으로 보간한다.
 	kNewCamRot = NiQuaternion::Slerp(NiMin(fFrameTime * g_kSpeed.GetW(), 1.0f), kCamRot, kNewCamRot);
	kNewCamRot.FastNormalize();

	// 각도의 변화가 없다면
	if(kNewCamRot == kCamRot)
	{
		return false;	// 업데이트 하지말라
	}

	// 카메라 회전을 적용
	m_pkCamera->SetRotate(kNewCamRot);

	return true;
}

NiPoint3 PgCameraModeCircle::GetActorPosition()
{
	if(!m_pkActor)
	{
		PG_ASSERT_LOG(m_pkActor);
		return NiPoint3::ZERO;
	}

	return m_pkActor->GetPosition();
}