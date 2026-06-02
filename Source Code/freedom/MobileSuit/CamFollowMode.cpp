#include "StdAfx.h"
#include "CamFollowMode.h"

CCamFollowMode::CCamFollowMode(NiCameraPtr spCamera, CActorPtr spActor)
{
	m_spCamera = spCamera;
	m_spActor = spActor;
}

CCamFollowMode::~CCamFollowMode(void)
{
	m_spCamera = 0;
	m_spActor = 0;
}

bool CCamFollowMode::GetTranslate(float fAccumTime, NiPoint3& m_rkLocOut)
{
	NiPoint3 kLocLast = m_spCamera->GetTranslate();
	// NiPoint3 kLocActor = m_spTarget->GetTranslate();

	return true;
}

bool CCamFollowMode::GetRotate(float fAccumTime, NiQuaternion& m_rkRotOut)
{
	return true;
}