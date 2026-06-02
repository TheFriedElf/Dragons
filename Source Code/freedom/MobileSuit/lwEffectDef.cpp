#include "stdafx.h"
#include "lwEffectDef.h"

lwEffectDef	lwGetEffectDef(int iEffectNo)
{
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef	*pkEffectDef = kEffectDefMgr.GetDef(iEffectNo);
	return	lwEffectDef(pkEffectDef);
}

/*
void	lwReleaseEffectDef(lwEffectDef kEffectDef)
{
}
*/
//! 스크립팅 시스템에 등록한다.
bool lwEffectDef::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "GetEffectDef", &lwGetEffectDef);
	//def(pkState, "ReleaseEffectDef", &lwReleaseEffectDef);

	class_<lwEffectDef>(pkState, "EffectDef")
		.def(pkState, constructor<CEffectDef*>())
		.def(pkState, "GetEffectDef", &lwEffectDef::GetEffectDef)
		.def(pkState, "GetAbil", &lwEffectDef::GetAbil)
		.def(pkState, "GetDurationTime", &lwEffectDef::GetDurationTime)
		.def(pkState, "IsNil", &lwEffectDef::IsNil)
		;

	return true;
}

lwEffectDef	lwEffectDef::GetEffectDef()
{
	return	lwEffectDef(m_pkEffectDef);
}
int	lwEffectDef::GetAbil(int iAbilType)
{
	return	m_pkEffectDef->GetAbil(iAbilType);
}
int	lwEffectDef::GetDurationTime()
{
	return	m_pkEffectDef->GetDurationTime();
}
bool	lwEffectDef::IsNil()
{
	return	m_pkEffectDef==NULL;
}
