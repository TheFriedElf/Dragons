#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAM_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAM_H

#include <NiMain.H>
#include "PgParticle.H"

class	PgParticleStream	:	public	NiObject
{
	NiDeclareRTTI;

public:

	virtual	~PgParticleStream()	{};

public:

	virtual	PgParticlePtr	Load(std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo) = 0;
	virtual	std::string	GetStreamID()	=	0;

};

NiSmartPointer(PgParticleStream);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLESTREAM_H