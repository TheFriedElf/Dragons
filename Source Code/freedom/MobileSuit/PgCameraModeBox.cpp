#include "StdAfx.h"
#include "PgMobilesuit.h"
#include "PgCameraModeBox.h"
#include "PgInterpolator.h"
#include "PgDamper.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "PgInput.H"

PgCameraModeBox::PgCameraModeBox(NiCamera *pkCamera, PgActor *pkActor)
: PgICameraMode(pkCamera)
{
	m_fDistanceFromActor = 470.0f;
	m_fCameraMoveWidth = 140.0f;
	m_pkActor = pkActor;
	m_kPrevTrn = NiPoint3::ZERO;

	m_bCameraSmoothMove = true;

	kZoomOutCameraPosition = NiPoint3(0, -1335, 224);
	kZoomOutCameraRotation = NiQuaternion(0.5368f, 0.4599f, 0.5356f, 0.4599f);

	kZoomInCameraPosition = NiPoint3(0, -800, 125);
	kZoomInCameraRotation = NiQuaternion(0.5100f, 0.4906f, 0.5075f, 0.4894f);

	m_fNormalInterpolateTime = 1.0f;
	m_kTargetPathNormal = NiPoint3::ZERO;
	m_kCurrentPathNormal = NiPoint3::ZERO;
	m_kLastPathNormal = NiPoint3::ZERO;

	// w : Rot speed
	// x : Spring Const
	// y : Damp Const
	// z : Spring Length
	m_kSpeed = NiQuaternion(5.0f, 7.0f, 0.0f, 1.0f);
}

PgCameraModeBox::~PgCameraModeBox()
{
}

bool PgCameraModeBox::Update(float fFrameTime)
{
	PG_ASSERT_LOG(m_pkCamera);
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkCamera == NULL || m_pkActor == NULL)
		return false;

	NiPoint3 const &kCamTrn = m_pkCamera->GetWorldTranslate();
	NiPoint3 kActorTrn = m_pkActor->GetWorldTranslate();
	NiPoint3 kNewCamTrn = NiPoint3::ZERO;

	// -- 패스 노멀을 보간 (부드러운 카메라 이동에 도움이 된다) --
	if(m_kLastPathNormal == NiPoint3::ZERO)
	{
		m_kCurrentPathNormal = m_kLastPathNormal = m_pkActor->GetPathNormal();
		m_kPrevTrn = kCamTrn;
	}

	if(m_kTargetPathNormal != m_pkActor->GetPathNormal())
	{
		m_kLastPathNormal = m_kCurrentPathNormal;
		m_kTargetPathNormal = m_pkActor->GetPathNormal();
		if (m_bCameraSmoothMove)
			m_fNormalInterpolateTime = 0.0f;
		else
			m_kCurrentPathNormal = m_kTargetPathNormal;
	}

	if(m_fNormalInterpolateTime < 1.0f)
	{
		m_fNormalInterpolateTime += fFrameTime; // * m_kSpeed.GetW();
		m_fNormalInterpolateTime = NiClamp(m_fNormalInterpolateTime, 0.0f, 1.0f);
		m_kCurrentPathNormal = PgInterpolator::Lerp(m_kLastPathNormal, m_kTargetPathNormal, m_fNormalInterpolateTime);
	}
	// -- 패스 노멀을 보간 --

	// 바닥과 여백을 이용해 목표 위치를 구한다.
	kNewCamTrn = kActorTrn - (m_kCurrentPathNormal * m_fDistanceFromActor);
	float fCameraX = kNewCamTrn.x;
	float fTargetDelta = kNewCamTrn.y;
	float fMin = NiMin(kZoomOutCameraPosition.y, kZoomInCameraPosition.y);
	float fMax = NiMax(kZoomOutCameraPosition.y, kZoomInCameraPosition.y);

	if (fMin == fMax)
	{
		PG_ASSERT_LOG(0);
		return false;
	}

	fCameraX = NiClamp(fCameraX, -1.0f * m_fCameraMoveWidth, m_fCameraMoveWidth);
	fTargetDelta = NiClamp(fTargetDelta, fMin, fMax);	

	fTargetDelta = (fTargetDelta - fMin) / (fMax - fMin); // normalize

	NiPoint3 kTargetTrn;

	if (fMin == kZoomOutCameraPosition.y)
	{
		kTargetTrn = PgInterpolator::Lerp(kZoomOutCameraPosition, kZoomInCameraPosition, fTargetDelta);
	}
	else
	{
		kTargetTrn = PgInterpolator::Lerp(kZoomInCameraPosition, kZoomOutCameraPosition, fTargetDelta);
	}

	// 위치의 변화가 없다면
	if(kTargetTrn == kCamTrn)
	{
		return false;	// 업데이트 하지말라
	}

	kNewCamTrn = kTargetTrn;
	kNewCamTrn.x = fCameraX;

	// 스프링 뎀핑한다.
	if (m_bCameraSmoothMove)
	{
		kNewCamTrn = PgDamper::SpringDamp(kCamTrn, kNewCamTrn, m_kPrevTrn, fFrameTime, m_kSpeed.GetX(), m_kSpeed.GetY(), m_kSpeed.GetZ());
		m_pkCamera->SetTranslate(kNewCamTrn);
	}
	else
		m_pkCamera->SetTranslate(kNewCamTrn);

	float fCurrentDelta = (kNewCamTrn.y - fMin) / (fMax - fMin); // normalize
	NiQuaternion fCurrentRot;
	if (fMin == kZoomOutCameraPosition.y)
	{
		fCurrentRot = NiQuaternion::Slerp(fCurrentDelta, kZoomOutCameraRotation, kZoomInCameraRotation);
	}
	else
	{
		fCurrentRot = NiQuaternion::Slerp(fCurrentDelta, kZoomInCameraRotation, kZoomOutCameraRotation);
	}
	fCurrentRot.FastNormalize();
	m_pkCamera->SetRotate(fCurrentRot);

	m_kPrevTrn = kNewCamTrn;
	return true;
}

void PgCameraModeBox::SetActor(PgActor *pkActor)
{
	NILOG(PGLOG_MINOR, "[PgCameraModeBox] SetActor\n");
	PG_ASSERT_LOG(pkActor);
	if (pkActor == NULL)
		return;

	m_pkActor = pkActor;
	pkActor->ResetLastFloorPos();
}

NiPoint3 PgCameraModeBox::GetActorPosition()
{
	PG_ASSERT_LOG(m_pkActor);
	if(!m_pkActor)
	{
		return NiPoint3::ZERO;
	}

	return m_pkActor->GetPosition();
}