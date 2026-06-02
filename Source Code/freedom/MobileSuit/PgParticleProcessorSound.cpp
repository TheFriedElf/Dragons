#include "stdafx.h"
#include "PgParticleProcessorSound.H"
#include "PgParticle.H"
#include "PgSoundMan.H"

NiImplementRTTI(PgParticleProcessorSound, PgParticleProcessor);

bool	PgParticleProcessorSound::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(pkParticle->IsFinished())
	{
		return false;
	}

	if(m_fSoundTime >= 0 && pkParticle->GetNow()>m_fSoundTime)
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,m_kSoundID.c_str(),0.0f,80,180,pkParticle);
		m_fSoundTime = -1.0f;
	}

	return	false;
}
NiImplementCreateClone(PgParticleProcessorSound);

void PgParticleProcessorSound::CopyMembers(PgParticleProcessorSound* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->m_fSoundTime = m_fSoundTime;
	pDest->m_kSoundID = m_kSoundID;
}
