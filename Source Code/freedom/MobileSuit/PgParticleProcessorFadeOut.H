#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEOUT_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEOUT_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorFadeOut	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorFadeOut);

public:

	PgParticleProcessorFadeOut()
		:m_fFadeOutTime(0),m_fFadeStartTime(-1)
	{
	};
	PgParticleProcessorFadeOut(float fFadeOutTime)
		:m_fFadeOutTime(fFadeOutTime),m_fFadeStartTime(-1)
	{
	}

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);
	
	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_FADE_OUT;	}

private:

	float	GetFadeOutTime()	const	{	return	m_fFadeOutTime;	}
	void	SetFadeOutTime(float fTime)	{	m_fFadeOutTime = fTime;	}

	void	UpdateElapsedTime(float fAccumTime)
	{
		if(m_fFadeStartTime == -1)
		{
			m_fFadeStartTime = fAccumTime;
		}

		m_fElapsedTime = fAccumTime - m_fFadeStartTime;
	}

	float	GetElapsedTime()	const	{	return	m_fElapsedTime;	}

private:

	float	m_fFadeOutTime;
	float	m_fFadeStartTime;
	float	m_fElapsedTime;

};

NiSmartPointer(PgParticleProcessorFadeOut);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORFADEOUT_H