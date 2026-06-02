#include "stdafx.h"
#include "PgParticleProcessorCamFrontParticle.H"
#include "PgParticle.H"
#include "PgWorld.H"


NiImplementRTTI(PgParticleProcessorCamFrontParticle, PgParticleProcessor);

bool	PgParticleProcessorCamFrontParticle::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(!g_pkWorld)
	{
		return	false;
	}
	PgCameraMan* pkCameraMan = g_pkWorld->GetCameraMan();
	if(!pkCameraMan)
	{
		return	false;
	}
	NiCamera* pkCurCamera = pkCameraMan->GetCamera();
	if(!pkCurCamera)
	{
		return	false;
	}

	//위치 구함
	NiPoint3 kPos = pkCurCamera->GetWorldDirection() * m_kCamFront.kDistPos.y;
	kPos += pkCurCamera->GetWorldRightVector() * m_kCamFront.kDistPos.x;
	kPos += pkCurCamera->GetWorldUpVector() * m_kCamFront.kDistPos.z;
	kPos += pkCurCamera->GetTranslate();
	pkParticle->SetTranslate(kPos);

	NiPoint3 kDir =  -pkCurCamera->GetWorldDirection();
	NiPoint3 kUpVec(0,0,1);
	NiPoint3 kRightVec = kUpVec.UnitCross(kDir);
	NiMatrix3 kMat(kRightVec, kDir, kUpVec);
	pkParticle->SetRotate(kMat);

	return	true;
}
NiImplementCreateClone(PgParticleProcessorCamFrontParticle);

void PgParticleProcessorCamFrontParticle::CopyMembers(PgParticleProcessorCamFrontParticle* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->m_kCamFront = m_kCamFront;
}
