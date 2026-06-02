#include "stdafx.h"
#include "lwActionResult.h"

using namespace lua_tinker;

void lwActionResult::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(ActionResult)
		LW_REG_METHOD(ActionResult, SetMissed)
		LW_REG_METHOD(ActionResult, GetMissed)
		LW_REG_METHOD(ActionResult, SetValue)
		LW_REG_METHOD(ActionResult, GetValue)
		LW_REG_METHOD(ActionResult, AddEffect)
		LW_REG_METHOD(ActionResult, GetEffectNum)
		LW_REG_METHOD(ActionResult, GetEffect)
		LW_REG_METHOD(ActionResult, Init)
		LW_REG_METHOD(ActionResult, SetBlocked)
		LW_REG_METHOD(ActionResult, GetBlocked)
		LW_REG_METHOD(ActionResult, SetComic)
		LW_REG_METHOD(ActionResult, GetComic)
		LW_REG_METHOD(ActionResult, SetDead)
		LW_REG_METHOD(ActionResult, GetDead)
		LW_REG_METHOD(ActionResult, SetInvalid)
		LW_REG_METHOD(ActionResult, GetInvalid)
		LW_REG_METHOD(ActionResult, SetDodged)
		LW_REG_METHOD(ActionResult, GetDodged)
		LW_REG_METHOD(ActionResult, SetCritical)
		LW_REG_METHOD(ActionResult, GetCritical)
		LW_REG_METHOD(ActionResult, SetRemainHP)
		LW_REG_METHOD(ActionResult, IsMissed)
		LW_REG_METHOD(ActionResult, SetAbil)
		LW_REG_METHOD(ActionResult, GetAbil)
		;
}

bool lwActionResult::IsMissed()// GetMissed()와의 차이 : IsMissed() = GetMissed() or GetBlocked() or GetDodged() / 즉 어떤 이유로든 안맞았으면 true를 리턴하는 함수이다.
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return GetMissed() || GetDodged() || (PgActionResult::EBT_NONE_REACTION == GetBlocked());
}
void lwActionResult::SetMissed(bool const bMissed)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetMissed(bMissed);
}

bool lwActionResult::GetMissed()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetMissed();
}

void lwActionResult::SetValue(int const iValue)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetValue(iValue);
}

int lwActionResult::GetValue()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return 0;
	}
	return m_pkActionResult->GetValue();
}

void lwActionResult::AddEffect(int const iEffect)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->AddEffect(iEffect);
}

int lwActionResult::GetEffectNum()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return 0;
	}
	return m_pkActionResult->GetEffectNum();
}

int lwActionResult::GetEffect(int const iEffectIndex)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return 0;
	}
	return m_pkActionResult->GetEffect(iEffectIndex);
}

void lwActionResult::Init()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->Init();
}

void lwActionResult::SetBlocked(BYTE const byBlocked)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetBlocked(byBlocked);
}

BYTE lwActionResult::GetBlocked()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetBlocked();
}

void lwActionResult::SetComic(bool const bComic)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetComic(bComic);
}

bool lwActionResult::GetComic()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetComic();
}

void lwActionResult::SetDead(bool const bDead)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetDead(bDead);
}

bool lwActionResult::GetDead()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetDead();
}

void lwActionResult::SetInvalid(bool const bInvalid)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetInvalid(bInvalid);
}

bool lwActionResult::GetInvalid()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetInvalid();
}

void lwActionResult::SetDodged(bool const bDodged)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetDodged(bDodged);
}

bool lwActionResult::GetDodged()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetDodged();
}

void lwActionResult::SetCritical(bool const bCritical)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetCritical(bCritical);
}

bool lwActionResult::GetCritical()
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return false;
	}
	return m_pkActionResult->GetCritical();
}
void lwActionResult::SetRemainHP(int const iHP)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	return m_pkActionResult->SetRemainHP(iHP);
}

void lwActionResult::SetAbil(WORD wAbil, int iValue)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return ;
	}
	m_pkActionResult->SetAbil(wAbil, iValue);
}

int lwActionResult::GetAbil(WORD wAbil)
{
	if( !m_pkActionResult )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkActionResult is NULL"), __FUNCTIONW__);
		return 0;
	}
	return m_pkActionResult->GetAbil(wAbil);
}

/////////////////////////////////////////////////////////////////////////////
//	PgActionResultVector
/////////////////////////////////////////////////////////////////////////////

lwActionResultVector lwNewActionResultVector()
{
	PgActionResultVector* pkNew = new PgActionResultVector;
	return lwActionResultVector(pkNew);
}

void lwDeleteActionResultVector(lwActionResultVector kObject)
{
	POINTER_VERIFY_INFO_LOG(kObject());
	delete kObject();
}

void lwActionResultVector::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "NewActionResultVector", &lwNewActionResultVector);
	def(pkState, "DeleteActionResultVector", &lwDeleteActionResultVector);
	
	LW_REG_CLASS(ActionResultVector)
		LW_REG_METHOD(ActionResultVector, GetResult)
		;
}

lwActionResult lwActionResultVector::GetResult(lwGUID kGuid, bool bMake)
{
	return lwActionResult(m_pkActionResultVector->GetResult(kGuid(), bMake));
}

