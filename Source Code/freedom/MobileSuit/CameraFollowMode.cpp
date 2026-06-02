#include "StdAfx.h"
#include "CameraFollowMode.h"
#include "Actor.h"

CCamFollowMode::CCamFollowMode(NiCamera *pkCamera, CActor *pkActor)
{
	m_pkCamera = pkCamera;
	m_pkActor = pkActor;
}

CCamFollowMode::~CCamFollowMode(void)
{
}

bool CCamFollowMode::Update(float fTime)
{
	UpdateTranslate(fTime);
	m_pkCamera->Update(fTime);

	return true;
}

bool CCamFollowMode::UpdateTranslate(float fTime)
{
	assert(m_pkCamera);
	assert(m_pkActor);

	const NiPoint3 &kCamTrn = m_pkCamera->GetTranslate();
	const NiPoint3 &kActorTrn = m_pkActor->GetTranslate();
	NiPoint3 kNewCamTrn;

	kNewCamTrn = -m_pkActor->GetPathNormal() * 800 + kActorTrn;

	if(kNewCamTrn == kCamTrn)
	{
		return false;	// 업데이트 필요 없음
	}

	m_pkCamera->GetParent()->SetTranslate(kNewCamTrn);
	m_pkCamera->LookAtWorldPoint(kActorTrn, NiPoint3::UNIT_Z);

	return true;
}

bool CCamFollowMode::UpdateRotate(float fTime)
{
	assert(m_pkCamera);
	assert(m_pkActor);

	return true;
}