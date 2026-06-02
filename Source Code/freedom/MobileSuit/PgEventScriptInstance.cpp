#include "stdafx.h"
#include "PgEventScriptInstance.H"
#include "PgEventScriptData.H"
#include "PgEventScriptCmd.H"
#include "PgEventScriptDataMan.H"

PgEventScriptInstance* PgEventScriptInstance::Create(int iEventID)
{
	PgEventScriptData	*pkData = g_kEventScriptDataMan.GetEventScriptData(iEventID);
	if(!pkData)
	{
		return	NULL;
	}

	return new PgEventScriptInstance(pkData);
}

PgEventScriptInstance::PgEventScriptInstance(PgEventScriptData *pkData)	: m_pkData(pkData),m_iNextCmdIndex(0)
{
	if(m_pkData)
	{
		m_pkData->IncInstanceCount();
	}
}

PgEventScriptInstance::~PgEventScriptInstance()
{
	if(m_pkData)
	{
		m_pkData->DecInstanceCount();
	}
}

bool	PgEventScriptInstance::Update(float fAccumTime,float fFrameTime)
{
	if(!m_pkData)
	{
		return	true;
	}
	bool	bResult = true;

	int	const	iStartIndex = m_iNextCmdIndex;
	int	const	iFinalIndex = m_pkData->GetCmdCount()-1;

	int	i = iStartIndex;
	for(; i<=iFinalIndex;i++)
	{
		PgEventScriptCmd	*pkNextCmd = m_pkData->GetCmd(i);
		if(!pkNextCmd->Update(fAccumTime,fFrameTime,m_kCmdCustomParamMap))
		{
			break;
		}
		m_kCmdCustomParamMap.clear();
	}

	m_iNextCmdIndex = i;

	return	(m_iNextCmdIndex>iFinalIndex);
}