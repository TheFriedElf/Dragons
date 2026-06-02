#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgCameraModeAni.h"
#include "PgWorld.h"

PgCameraModeAni::PgCameraModeAni(NiCamera *pkCamera):PgICameraMode(pkCamera)
{
	m_bUseActorPositionAsSoundListener = false;
	m_pkActorRoot = NULL;
}

PgCameraModeAni::~PgCameraModeAni()
{
	g_kFrustum = m_kOldFrustum;
	m_bUseActorPositionAsSoundListener = false;
	m_pkActorRoot = NULL;
}

bool PgCameraModeAni::Update(float fFrameTime)
{
	if(m_pkAniCamera && m_pkCamera)
	{
		m_pkCamera->SetTranslate(m_pkAniCamera->GetWorldTranslate());
		m_pkCamera->SetRotate(m_pkAniCamera->GetWorldRotate());
	}

	return true;
}

void PgCameraModeAni::SetAniCamera(NiCamera* pkAniCamera)
{
	if (pkAniCamera == NULL)
		return;

	m_pkAniCamera = pkAniCamera;
	m_kOldFrustum = g_kFrustum;
	g_kFrustum = m_pkAniCamera->GetViewFrustum();
	g_kFrustum.m_fFar = 100000.0f;
}
