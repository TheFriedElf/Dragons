#include "stdafx.h"
#include "lwAttackEffect.h"

bool lwAttackEffect::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	class_<lwAttackEffect>(pkState, "AttackEffect")
		.def(pkState, constructor<PgItemEx::SAttackEffect *>())
		.def(pkState, "IsNil", &lwAttackEffect::IsNil)
		.def(pkState, "GetActionID", &lwAttackEffect::GetActionID)
		.def(pkState, "GetNodeName", &lwAttackEffect::GetNodeName)
		.def(pkState, "GetEffectName", &lwAttackEffect::GetEffectName)
		.def(pkState, "GetScale", &lwAttackEffect::GetScale)
		;

	return true;
}

lwAttackEffect::lwAttackEffect(PgItemEx::SAttackEffect const *pkEff)
{
	m_pkEff = pkEff;
}

bool lwAttackEffect::IsNil() const
{
	return NULL==m_pkEff;
}

char const* lwAttackEffect::GetActionID() const
{
	if(m_pkEff)
	{
		return m_pkEff->m_kActionID.c_str();
	}
	return NULL;
}

char const* lwAttackEffect::GetNodeName() const
{
	if(m_pkEff)
	{
		return m_pkEff->m_kNodeID.c_str();
	}
	return NULL;
}

char const* lwAttackEffect::GetEffectName() const
{
	if(m_pkEff)
	{
		return m_pkEff->m_kEffectID.c_str();
	}
	return NULL;
}

float const lwAttackEffect::GetScale() const
{
	if(m_pkEff)
	{
		return m_pkEff->m_fScale;
	}
	return 1.0f;
}