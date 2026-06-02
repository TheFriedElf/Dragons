#include "StdAfx.h"
#include "PgICameraMode.h"
#include "PgPilotMan.h"

PgICameraMode::PgICameraMode(NiCamera *pkCamera)
{
	m_pkCamera = pkCamera;
}

PgICameraMode::~PgICameraMode(void)
{
}

NiCamera *PgICameraMode::GetCamera()
{
	return m_pkCamera;
}
	
void PgICameraMode::SetCamera(NiCamera *pkCamera)
{
	m_pkCamera = pkCamera;
}

void PgICameraMode::NotifyActorRemove(PgActor* pkActor)
{
	if (m_pkActor != NULL && pkActor == m_pkActor)
	{
		if (g_kPilotMan.GetPlayerActor() == m_pkActor)
		{
			// 내꺼가 없어지면 어쩌지? -_-
			PG_ASSERT_LOG(0);
		}
		else
		{
			SetActor(g_kPilotMan.GetPlayerActor());
		}
	}
}