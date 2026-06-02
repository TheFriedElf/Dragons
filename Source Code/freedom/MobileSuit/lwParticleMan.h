#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLEMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLEMAN_H

#include "lwParticle.h"

class PgParticleMan;

class lwParticleMan
{
public:
	lwParticleMan(PgParticleMan *pkParticleMan);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwParticle NewParticle(char const *pcEffectID);

protected:
	PgParticleMan *m_pkParticleMan;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLEMAN_H