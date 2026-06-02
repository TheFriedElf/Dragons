#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORCAMFRONTPARTICLE_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORCAMFRONTPARTICLE_H

#include "PgParticleProcessor.H"

typedef struct tagCameraFrontParticleInfo
{
	bool  bCameraFrontParticle;			//카메라 정면에 있을 파티클인가
	NiPoint3 kDistPos;				//카메라 정면으로 떨어질 거리
	tagCameraFrontParticleInfo()
		:bCameraFrontParticle(false)
		,kDistPos(0.0f,0.0f,0.0f)
	{}
}SCameraFrontParticleInfo;

class	PgParticleProcessorCamFrontParticle	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorCamFrontParticle);

public:

	PgParticleProcessorCamFrontParticle()
	{
	};
	PgParticleProcessorCamFrontParticle(NiPoint3 const &kDistPos)
	{
		m_kCamFront.kDistPos = kDistPos;
	};

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);

	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_CAM_FRONT;	}

private:

	//카메라 앞에 붙는 파티클 정보
	SCameraFrontParticleInfo m_kCamFront;
};

NiSmartPointer(PgParticleProcessorCamFrontParticle);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORCAMFRONTPARTICLE_H