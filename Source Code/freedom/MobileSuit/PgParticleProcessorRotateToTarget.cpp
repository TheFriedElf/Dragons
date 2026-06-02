#include "stdafx.h"
#include "PgParticleProcessorRotateToTarget.H"
#include "PgParticle.H"
#include "PgWorld.H"
#include "PgPilotMan.H"
#include "PgPilot.H"
#include "PgActor.H"
#include "PgParticleMan.H"

NiImplementRTTI(PgParticleProcessorRotateToTarget, PgParticleProcessor);

bool	PgParticleProcessorRotateToTarget::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{

	if(!g_pkWorld)
	{
		return	false;
	}

	const NiTransform &kTransform = pkParticle->GetLocalTransform();

	NiPoint3 kLength = NiPoint3(0,0,80);
	const NiAVObject* pkTarget = m_spTargetForRotate;
	NiPoint3 const &rkNormalDir = m_kNormalDirForRotate;
	NiPoint3 const &rkSrcPos = (pkParticle->GetWorldTranslate() + kLength);
	NiPoint3 const &rkDestPos = pkTarget->GetWorldTranslate();
	NiPoint3 kDir = rkDestPos - rkSrcPos;
	kDir.Unitize();

	NiPoint3 const &rkLookDir = g_kPilotMan.GetPlayerActor()->GetLookingDir();

	NiQuaternion kQuat2 = PgParticleMan::GetBetweenQuaternion(rkNormalDir, kDir);
	NiQuaternion kRetQuat = kQuat2;
	
	pkParticle->SetRotate(kRetQuat);
	pkParticle->SetTranslate(kTransform.m_Translate + kLength);

	return	true;		

}
NiImplementCreateClone(PgParticleProcessorRotateToTarget);

void PgParticleProcessorRotateToTarget::CopyMembers(PgParticleProcessorRotateToTarget* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->m_spTargetForRotate = m_spTargetForRotate;
	pDest->m_kNormalDirForRotate = m_kNormalDirForRotate;
}
