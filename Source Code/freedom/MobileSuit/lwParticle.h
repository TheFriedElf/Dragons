#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLE_H

#include "lwPoint3.h"

class PgParticle;

class lwParticle
{
public:
	lwParticle(PgParticle *pkParticle);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	bool SetTrigger(char const *pcTriggerFunc, unsigned int uiGroup, unsigned uiTTL);

	PgParticle *lwParticle::operator()();

	void SetVelocity(lwPoint3 kVel);

protected:
	PgParticle *m_pkParticle;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWPARTICLE_H