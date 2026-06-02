#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORROTATETOTARGET_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORROTATETOTARGET_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorRotateToTarget	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorRotateToTarget);

public:

	PgParticleProcessorRotateToTarget()
	{
	};

	PgParticleProcessorRotateToTarget(NiAVObject *pkTargetForRotate,NiPoint3 const &kNormalDirForRotate)
		:m_spTargetForRotate(pkTargetForRotate),m_kNormalDirForRotate(kNormalDirForRotate)
	{
	};

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);

	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_ROTATE_TO_TARGET;	}

private:

	NiAVObjectPtr m_spTargetForRotate;
	NiPoint3 m_kNormalDirForRotate;
};

NiSmartPointer(PgParticleProcessorRotateToTarget);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORROTATETOTARGET_H