#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWATTACKEFFECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWATTACKEFFECT_H

#include "PgItemEx.h"

class lwAttackEffect
{
public:
	lwAttackEffect(PgItemEx::SAttackEffect const *pkEff);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	bool IsNil() const;
	char const* GetActionID() const;
	char const* GetNodeName() const;
	char const* GetEffectName() const;
	float const GetScale() const;

protected:
	PgItemEx::SAttackEffect const *m_pkEff;
};

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWATTACKEFFECT_H