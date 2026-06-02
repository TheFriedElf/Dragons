#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORATTACHTONODE_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORATTACHTONODE_H

#include "PgParticleProcessor.H"


class	PgParticleProcessorAttachToNode	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorAttachToNode);

public:

	PgParticleProcessorAttachToNode()
	{
	};
	PgParticleProcessorAttachToNode(NiAVObject *pkAttachNode)
		:m_spAttachNode(pkAttachNode)
	{
	}

	virtual	bool	DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime);
	
	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_ATTACH_TO_NODE;	}

private:

	NiAVObjectPtr	m_spAttachNode;

};

NiSmartPointer(PgParticleProcessorAttachToNode);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PROCESSOR_PGPARTICLEPROCESSORATTACHTONODE_H