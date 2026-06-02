#include "stdafx.h"
#include "PgWorldActionMan.h"
#include "PgWorldActionObject.h"
#include "PgWorld.H"

#include "lwWorldActionObject.h"
#include "PgMobileSuit.h"
//#include "lwWorldActionMan.h"
PgWorldActionObject::PgWorldActionObject(PgWorldActionMan *pMan, int iWorldActionID) :
	m_iCurrentStage(0)
{
	Init(pMan, iWorldActionID);
}

int	PgWorldActionObject::GetElapsedTime(float fAccumTime)
{
	return (int)((fAccumTime - m_fStartTime)*1000.0f + m_iElapsedTimeAtStart);
}

//	스크립트 호출
bool	PgWorldActionObject::OnUpdate(float fFrameTime,float fAccumTime)	//	return이 false일 경우, Leave 스크립트를 실행하고, 이 오브젝트를 제거시킨다.
{
	if (m_pWorldActionMan == NULL)
		return false;

	if (m_pWorldActionMan->GetWorldActionScript(m_iWorldActionID).empty())
	{
		return false;
	}

	bool bReturn= lua_tinker::call<bool, lwWorldActionObject,int>((m_pWorldActionMan->GetWorldActionScript(m_iWorldActionID) + "_OnUpdate").c_str(), lwWorldActionObject(this),GetElapsedTime(fAccumTime));
	return bReturn;
}
void	PgWorldActionObject::OnEnter(int iElapsedTimeAtStart)	//서버에서 TimeGetTime으로 시간 들어옴
{
	if(!g_pkWorld)
	{
		return;
	}
	m_fStartTime = g_pkWorld->GetAccumTime();//BM::GetTime32();
	m_iElapsedTimeAtStart = iElapsedTimeAtStart;
	lua_tinker::call<void, lwWorldActionObject,int >((m_pWorldActionMan->GetWorldActionScript(m_iWorldActionID) + "_OnEnter").c_str(), lwWorldActionObject(this),m_iElapsedTimeAtStart);
}
void	PgWorldActionObject::OnLeave()
{
	lua_tinker::call<void, lwWorldActionObject>((m_pWorldActionMan->GetWorldActionScript(m_iWorldActionID) + "_OnLeave").c_str(), lwWorldActionObject(this));
}
void	PgWorldActionObject::OnPacketReceive(lwPacket &Packet)
{
	lua_tinker::call<void, lwWorldActionObject,lwPacket>((m_pWorldActionMan->GetWorldActionScript(m_iWorldActionID) + "_OnReceivePacket").c_str(), lwWorldActionObject(this),Packet);
}

DWORD PgWorldActionObject::GetDuration()
{
	return m_pWorldActionMan->GetWorldActionDuration(m_iWorldActionID);
}

int	PgWorldActionObject::GetCurrentStage()
{
	// 현재는 단순히 표기를 하기 위함일 뿐 아무런 기능도 없음!
	return m_iCurrentStage;	
}

void PgWorldActionObject::NextStage()
{
	++m_iCurrentStage;
}

//	파라메터 추가
void	PgWorldActionObject::SetParam(int iID,int iValue)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type != WAPT_INT) 
		{
			SAFE_DELETE(pData);
			m_Params.erase(itr);
		}
	}
	stWAPT_INT *pIntData = (stWAPT_INT*)pData;
	if(pIntData == 0)
	{
		pIntData = new stWAPT_INT();
		m_Params[iID] = pIntData;
	}

	pIntData->m_iValue = iValue;
}
void	PgWorldActionObject::SetParam(int iID,float fValue)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type != WAPT_FLOAT) 
		{
			SAFE_DELETE(pData);
			pData = 0;
			m_Params.erase(itr);
		}
	}
	stWAPT_FLOAT *pFLOATData = (stWAPT_FLOAT*)pData;
	if(pData == 0)
	{
		pFLOATData = new stWAPT_FLOAT();
		m_Params[iID] = pFLOATData;
	}

	pFLOATData->m_fValue = fValue;
}
void	PgWorldActionObject::SetParam(int iID,std::string Value)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type != WAPT_STRING) 
		{
			SAFE_DELETE(pData);
			m_Params.erase(itr);
		}
	}
	stWAPT_STRING *pSTRINGData = (stWAPT_STRING*)pData;
	if(pSTRINGData == 0)
	{
		pSTRINGData = new stWAPT_STRING();
		m_Params[iID] = pSTRINGData;
	}

	pSTRINGData->m_Value = Value;
}
void	PgWorldActionObject::SetParam(int iID,BM::GUID Value)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type != WAPT_GUID) 
		{
			SAFE_DELETE(pData);
			m_Params.erase(itr);
		}
	}
	stWAPT_GUID *pGUIDData = (stWAPT_GUID*)pData;
	if(pGUIDData == 0)
	{
		pGUIDData = new stWAPT_GUID();
		m_Params[iID] = pGUIDData;
	}

	pGUIDData->m_Value = Value;
}

void PgWorldActionObject::SetParam(char const *pcKey, char const *pcValue)
{
	if(!pcKey || !pcValue)
	{
		return;
	}

	// 무조건 추가되거나, 갱신됨.
	m_kStringParamContainer[std::string(pcKey)] = std::string(pcValue);
}

char const *PgWorldActionObject::GetParam(char const *pcKey)
{
	StringParamContainer::iterator itr = m_kStringParamContainer.find(pcKey);
	if(itr == m_kStringParamContainer.end())
	{
		return 0;
	}

	return itr->second.c_str();
}

int	PgWorldActionObject::GetParamInt(int iID)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type == WAPT_INT) 
		{
			return ((stWAPT_INT*)pData)->m_iValue;
		}
		else
		{
			//::MessageBox(0,_T("Parameter ID 'Inputted' and 'Stored' are not match"),_T("GetParamInt Failed"),MB_OK);
			_PgMessageBox("GetParamInt Failed", "Parameter ID 'Inputted' and 'Stored' are not match");
		}
	}
	return 0;
}
float	PgWorldActionObject::GetParamFloat(int iID)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type == WAPT_FLOAT) 
		{
			return ((stWAPT_FLOAT*)pData)->m_fValue;
		}
		else
		{
			//::MessageBox(0,_T("Parameter ID 'Inputted' and 'Stored' are not match"),_T("GetParamFLOAT Failed"),MB_OK);
			_PgMessageBox("GetParamFLOAT Failed", "Parameter ID 'Inputted' and 'Stored' are not match");
		}
	}
	return 0;
}
std::string const &PgWorldActionObject::GetParamString(int iID)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type == WAPT_STRING) 
		{
			return ((stWAPT_STRING*)pData)->m_Value;
		}
		else
		{
			//::MessageBox(0,_T("Parameter ID 'Inputted' and 'Stored' are not match"),_T("GetParamSTRING Failed"),MB_OK);
			_PgMessageBox("GetParamSTRING Failed", "Parameter ID 'Inputted' and 'Stored' are not match");
		}
	}
	static std::string const kNuillString;
	return kNuillString;
}
BM::GUID const &PgWorldActionObject::GetParamGUID(int iID)
{
	stWorldActionParam	*pData = 0;
	ParamContainer::iterator itr = m_Params.find(iID);
	if(itr != m_Params.end())
	{
		pData = itr->second;
		if(pData->Type == WAPT_GUID) 
		{
			return ((stWAPT_GUID*)pData)->m_Value;
		}
		else
		{
			//::MessageBox(0,_T("Parameter ID 'Inputted' and 'Stored' are not match"),_T("GetParamGUID Failed"),MB_OK);
			_PgMessageBox("GetParamGUID Failed", "Parameter ID 'Inputted' and 'Stored' are not match");
		}
	}
	static BM::GUID const kNullGUID;
	return kNullGUID;
}

void	PgWorldActionObject::Init(PgWorldActionMan *pMan,int iWorldActionID)
{
	m_pWorldActionMan = pMan;
	m_iWorldActionID = iWorldActionID;
	m_iElapsedTimeAtStart = 0;
	m_ulStartTime = 0;
	m_fStartTime = 0.0f;
}

void	PgWorldActionObject::Destroy()
{
	for(ParamContainer::iterator itr = m_Params.begin();
		itr != m_Params.end();
		++itr)
	{
		SAFE_DELETE(itr->second);
	}
	m_Params.clear();
}