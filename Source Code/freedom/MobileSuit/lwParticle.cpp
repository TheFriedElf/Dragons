#include "StdAfx.h"
#include "lwParticle.h"
#include "PgParticle.h"

lwParticle::lwParticle(PgParticle *pkParticle)
{
	m_pkParticle = pkParticle;
}

bool lwParticle::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwParticle>(pkState, "Particle")
		.def(pkState, constructor<PgParticle *>())
		.def(pkState, "SetTrigger", &lwParticle::SetTrigger)
		.def(pkState, "SetVelocity", &lwParticle::SetVelocity)
		;

	return true;
}

bool lwParticle::SetTrigger(char const *pcTriggerFunc, unsigned int uiGroup, unsigned uiTTL)
{
	return	true;
}

PgParticle *lwParticle::operator()()
{
	return m_pkParticle;
}

void lwParticle::SetVelocity(lwPoint3 kVel)
{
}
