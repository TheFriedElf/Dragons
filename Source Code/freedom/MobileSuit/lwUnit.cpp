#include "stdafx.h"
#include "lwUnit.h"
#include "lwEffect.h"

using namespace lua_tinker;

void lwUnit::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(Unit)
		LW_REG_METHOD(Unit, GetGuid)
		LW_REG_METHOD(Unit, GetPartyGuid)
		LW_REG_METHOD(Unit, GetState)
		LW_REG_METHOD(Unit, GetUnitType)
		LW_REG_METHOD(Unit, SetState)
		LW_REG_METHOD(Unit, GetAbil)
		LW_REG_METHOD(Unit, GetFrontDirection)
		LW_REG_METHOD(Unit, FindEffect)
		LW_REG_METHOD(Unit, GetEffect)
		LW_REG_METHOD(Unit, DeleteEffect)
		LW_REG_METHOD(Unit, GetGuildGuid)
		LW_REG_METHOD(Unit, IsEffect)
		LW_REG_METHOD(Unit, SetCaller)
		LW_REG_METHOD(Unit, GetCaller)
		LW_REG_METHOD(Unit, GetSelectedPet)
		LW_REG_METHOD(Unit, IsDead)
		;
}

lwGUID lwUnit::GetGuid()
{
	if (m_pkUnit == NULL)
		return lwGUID(BM::GUID::NullData());

	return lwGUID(m_pkUnit->GetID());
}

lwGUID lwUnit::GetPartyGuid()
{
	if( m_pkUnit )
	{
		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(m_pkUnit);
		if( pkPC )
		{
			return lwGUID(pkPC->PartyGuid());
		}
	}
	return lwGUID(BM::GUID::NullData());
}

lwGUID lwUnit::GetGuildGuid()
{
	if ( m_pkUnit )
	{
		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(m_pkUnit);
		if( pkPC )
		{
			return lwGUID(pkPC->GuildGuid());
		}
	}

	return lwGUID(BM::GUID::NullData());
}

int lwUnit::GetState()
{
	if ( m_pkUnit == NULL )
		return 0 ;

	return (int)m_pkUnit->GetState();
}

int lwUnit::GetUnitType()
{
	if ( m_pkUnit == NULL )
		return 0 ;

	return (int)m_pkUnit->UnitType();
}

void lwUnit::SetState(int const iState)
{
	assert(m_pkUnit);
	if ( m_pkUnit == NULL )
		return ;
	m_pkUnit->SetState((EUnitState)iState);
}

int lwUnit::GetAbil(WORD wType)
{
	assert(m_pkUnit);
	if ( m_pkUnit == NULL )
		return 0 ;

	return m_pkUnit->GetAbil(wType);
}

int lwUnit::GetFrontDirection()
{
	if( !m_pkUnit )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkUnit is NULL"), __FUNCTIONW__);
		return 0;
	}
	return m_pkUnit->FrontDirection();
}

bool lwUnit::IsEffect(int iEffectKey, bool bInGroup)
{
	if( !m_pkUnit )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkUnit is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkUnit->IsItemEffect(iEffectKey, bInGroup);
}

lwEffect lwUnit::FindEffect(int iEffectNo)
{
	CEffect const * pkEffect = NULL;
	if( m_pkUnit )
	{
		pkEffect = m_pkUnit->FindEffect(iEffectNo);
	}
	return lwEffect(const_cast<CEffect*>(pkEffect));
}

lwEffect lwUnit::GetEffect(int iEffectKey, bool bInGroup)
{
	if( !m_pkUnit )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkUnit is NULL"), __FUNCTIONW__);
		return lwEffect(NULL);
	}
	CEffect* pkEffect = m_pkUnit->GetEffect(iEffectKey, bInGroup);
	return lwEffect(pkEffect);
}

bool lwUnit::DeleteEffect(int iEffectNo)
{
	if( !m_pkUnit )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkUnit is NULL"), __FUNCTIONW__);
		return false;
	}
	// Client에서는 아무것도 하지 말아라..
	return true;
}

void lwUnit::SetCaller(lwGUID kActorGUID)
{
	if( m_pkUnit )
	{
		m_pkUnit->Caller(kActorGUID());
	}
}

lwGUID lwUnit::GetCaller() const
{
	if( m_pkUnit )
	{
		return lwGUID(m_pkUnit->Caller());
	}

	return lwGUID(BM::GUID::NullData());
}

lwGUID lwUnit::GetSelectedPet() const
{
	if( m_pkUnit )
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(m_pkUnit);
		if(pkPlayer)
		{
			return lwGUID(pkPlayer->SelectedPetID());
		}
	}

	return lwGUID(BM::GUID::NullData());
}

bool lwUnit::IsDead()const
{
	if ( m_pkUnit )
	{
		return m_pkUnit->IsDead();
	}
	return true;
}
