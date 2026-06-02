#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSOUND_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSOUND_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorSound	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorSound);

public:

	PgParticleProcessorSound()
		:m_fSoundTime(0)
	{
	};

	PgParticleProcessorSound(float fSoundTime,std::string const &kSoundID)
		:m_fSoundTime(fSoundTime),m_kSoundID(kSoundID)
	{
	}

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);

	virtual	PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PPID_SOUND;	}

private:

	float	m_fSoundTime;
	std::string m_kSoundID;
};

NiSmartPointer(PgParticleProcessorSound);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSOUND_H