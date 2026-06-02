#include "StdAfx.h"
#include "lwParticleMan.h"
#include "PgParticleMan.h"

lwParticleMan::lwParticleMan(PgParticleMan *pkParticleMan)
{
	m_pkParticleMan = pkParticleMan;
}

bool lwParticleMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwParticleMan>(pkState, "ParticleMan")
		.def(pkState, constructor<PgParticleMan *>())
		.def(pkState, "NewParticle", &lwParticleMan::NewParticle)
		;

	return true;
}

lwParticle lwParticleMan::NewParticle(char const *pcEffectID)
{
	PgParticle *pkParticle = m_pkParticleMan->GetParticle(pcEffectID);

	return (lwParticle)pkParticle;
}
