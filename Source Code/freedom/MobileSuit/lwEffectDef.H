#ifndef FREEDOM_DRAGONICA_SCRIPTING_DEF_LWEFFECTDEF_H
#define FREEDOM_DRAGONICA_SCRIPTING_DEF_LWEFFECTDEF_H
#include "variant/Effect.H"
class	lwEffectDef
{
public:

	lwEffectDef(const	CEffectDef	*pkEffectDef)	{	m_pkEffectDef = pkEffectDef;	};

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwEffectDef	GetEffectDef();
	int	GetAbil(int iAbilType);
	int	GetDurationTime();

	bool	IsNil();

private:

	const	CEffectDef	*m_pkEffectDef;

};
#endif // FREEDOM_DRAGONICA_SCRIPTING_DEF_LWEFFECTDEF_H