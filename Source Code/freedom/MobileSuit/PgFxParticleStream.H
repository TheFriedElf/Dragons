#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARTICLESTREAM_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARTICLESTREAM_H

#include "PgParticleStream.H"

class	PgFxParticleStream	:	public	PgParticleStream
{
	NiDeclareRTTI;

public:

	PgParticlePtr	Load(std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo);

	virtual	std::string	GetStreamID()	{	return	"FX";	};

private:

	NiCamera*	GetWorldCamera();
};

NiSmartPointer(PgFxParticleStream);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARTICLESTREAM_H