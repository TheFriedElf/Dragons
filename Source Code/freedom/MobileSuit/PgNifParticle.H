#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLE_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLE_H

#include "PgParticle.h"

class PgNifParticle : public PgParticle
{
	NiDeclareRTTI;
	NiDeclareClone(PgNifParticle);

public:

	PgNifParticle()
		:m_fEnd(0)
	{
	};

	virtual	bool Create(NiAVObject *pkParticleRoot,std::string const &kID,stParticleInfo const &kParticleInfo);

private:

	void Analyze(NiObjectNET* pkRoot);
	virtual	void	CheckFinish(float fAccumTime,float fFrameTime);

	void	SetEnd(float fEnd)	{	m_fEnd = fEnd;	}
	float	GetEnd()	const	{	return	m_fEnd;	}


protected:

	float	m_fEnd;

};

NiSmartPointer(PgNifParticle);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLE_H