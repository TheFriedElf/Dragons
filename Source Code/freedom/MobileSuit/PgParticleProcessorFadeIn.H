#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEIN_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEIN_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorFadeIn	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorFadeIn);

public:

	PgParticleProcessorFadeIn()
		:m_fFadeInTime(0)
	{
	};
	PgParticleProcessorFadeIn(float fFadeInTime)
		:m_fFadeInTime(fFadeInTime)
	{
	}

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);
	
	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_FADE_IN;	}

private:

	float	GetFadeInTime()	const	{	return	m_fFadeInTime;	}
	void	SetFadeInTime(float fTime)	{	m_fFadeInTime = fTime;	}

private:

	float	m_fFadeInTime;

};

NiSmartPointer(PgParticleProcessorFadeIn);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEIN_H