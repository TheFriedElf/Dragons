#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORAUTOGROUND_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORAUTOGROUND_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorAutoGround	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorAutoGround);

public:

	PgParticleProcessorAutoGround()
	{
	};

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);

	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_AUTO_GROUND;	}

};

NiSmartPointer(PgParticleProcessorAutoGround);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORAUTOGROUND_H