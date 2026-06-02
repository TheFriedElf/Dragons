#include "stdafx.h"
#include "lwEffect.h"

using namespace lua_tinker;


void lwEffect::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(Effect)
		LW_REG_METHOD(Effect, GetValue)
		LW_REG_METHOD(Effect, SetValue)
		LW_REG_METHOD(Effect, GetCaster)
		LW_REG_METHOD(Effect, SetCaster)
		LW_REG_METHOD(Effect, GetAbil)
		LW_REG_METHOD(Effect, GetEffectNo)
		LW_REG_METHOD(Effect, GetEndTime)
		LW_REG_METHOD(Effect, GetDurationTime)
		;
}

int lwEffect::GetValue()
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return m_pkEffect->GetValue();
}

void lwEffect::SetValue(int iValue)
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	m_pkEffect->SetValue(iValue);
}

void lwEffect::SetCaster(lwGUID kGuid)
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	m_pkEffect->SetCaster(kGuid());
}


lwGUID lwEffect::GetCaster()
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return lwGUID(m_pkEffect->GetCaster());
}

int lwEffect::GetAbil(int iAbil)
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return m_pkEffect->GetAbil((WORD)iAbil);
}

int lwEffect::GetEffectNo()
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return m_pkEffect->GetEffectNo();
}

DWORD lwEffect::GetEndTime()const
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return m_pkEffect->GetEndTime();
}

int lwEffect::GetDurationTime()const
{
	POINTER_VERIFY_INFO_LOG(m_pkEffect);
	return m_pkEffect->GetDurationTime();
}
