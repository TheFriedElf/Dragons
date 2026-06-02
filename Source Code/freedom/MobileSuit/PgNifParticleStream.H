#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLESTREAM_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLESTREAM_H

#include "PgParticleStream.H"

class	PgNifParticleStream	:	public	PgParticleStream
{
	NiDeclareRTTI;

public:

	PgParticlePtr	Load(std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo);

	virtual	std::string	GetStreamID()	{	return	"NIF";	};

private:


	NiAVObjectPtr	LoadNif(std::string const &kPath);
};

NiSmartPointer(PgNifParticleStream);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGNIFPARTICLESTREAM_H