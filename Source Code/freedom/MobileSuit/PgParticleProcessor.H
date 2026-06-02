#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSOR_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSOR_H

#include <NiMain.H>

class	PgParticle;
class	PgParticleProcessor	:	public	NiObject
{

	NiDeclareRTTI;

public:

	enum	PARTICLE_PROCESSOR_ID
	{
		PPID_NONE=-1,
		PPID_ATTACH_TO_NODE=0,
		PPID_AUTO_GROUND,
		PPID_ROTATE_TO_TARGET,
		PPID_CAM_FRONT,
		PPID_SOUND,
		PPID_SPRITE_ANIMATION,
		PPID_FADE_IN,
		PPID_FADE_OUT,
		PPID_MAX,
	};

public:

	PgParticleProcessor()
		:m_bFinished(false)
	{
	}
	virtual	~PgParticleProcessor()	{};

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime) = 0;
	virtual	PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PPID_NONE;	}

	void	SetFinished(bool bFinished)	{	m_bFinished = bFinished;	}
	bool	GetFinished()	const	{	return	m_bFinished;	}

protected:

	void CopyMembers(PgParticleProcessor* pDest, NiCloningProcess& kCloning);

private:

	bool	m_bFinished;

};
NiSmartPointer(PgParticleProcessor);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSOR_H