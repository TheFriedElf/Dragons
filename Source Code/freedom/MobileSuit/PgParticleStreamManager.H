#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAMMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAMMANAGER_H

#include <map>
#include <string>

#include "PgParticleStream.H"

class	PgParticleStreamManager	:	public	NiObject
{
public:

	typedef	std::map<std::string,PgParticleStreamPtr> ParticleStreamMap;

public:

	virtual	~PgParticleStreamManager()	{};

	void	RegisterStreams();

	PgParticlePtr	LoadParticle(std::string const &kStreamID,std::string const &kParticleID,PgParticle::stParticleInfo const &kParticleInfo);

private:

	void	RegisterStream(PgParticleStream *pkStream);
	PgParticleStream*	FindStream(std::string const &kStreamID);

private:

	ParticleStreamMap	m_kParticleStreamCont;
};

NiSmartPointer(PgParticleStreamManager);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAMMANAGER_H