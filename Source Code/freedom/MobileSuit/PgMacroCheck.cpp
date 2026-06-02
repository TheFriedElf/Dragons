#include "stdafx.h"
#include "PgPilotMan.h"
#include "PgMacroCheck.h"

PgMacroCheck::PgMacroCheck()
{
	Clear();
}

PgMacroCheck::~PgMacroCheck()
{
	Clear();
}
	
bool PgMacroCheck::IsComplete() const
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer) 
	{// 플레이어가 없으면 서버에서 매크로 방지 답을 기다리는지 알수 없다
		_PgMessageBox("PgMacroCheck::IsComplete()", "Can't find Player");
		return true;
	}
	bool bComplate =!pkMyPlayer->MacroWaitAns();

	return bComplate;
}

void PgMacroCheck::RememberCheckInfo(wchar_t const wcKey, int const iIteration, short const sType,  int const iLimitTime)
{
	m_wcKey = wcKey;
	m_iIteration = iIteration;
	m_sType = sType;
	m_iCurLimitTime=m_iLimitTime = iLimitTime;
}

void PgMacroCheck::Clear()
{
	m_wcKey = 0;
	m_iIteration = 0;
	m_sType  = 0;
	m_iLimitTime = 0;
	m_iCurLimitTime=0;
}