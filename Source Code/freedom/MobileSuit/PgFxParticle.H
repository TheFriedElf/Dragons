#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARITCLE_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARITCLE_H

#include "PgParticle.h"

NiSmartPointer(FxObject);
class PgFxParticle : public PgParticle
{
	NiDeclareRTTI;
	NiDeclareClone(PgFxParticle);

public:

	virtual	bool Create(NiAVObject *pkParticleRoot,std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo);
    virtual void ProcessClone(
        NiCloningProcess& kCloning);

private:

	virtual	void	CheckFinish(float fAccumTime,float fFrameTime);
	void	SetFxObjectCamera();

private:

	FxObjectPtr	m_spFxObject;
};

NiSmartPointer(PgFxParticle);

#endif //FREEDOM_DRAGONICA_RENDER_STREAM_PGFXPARITCLE_H