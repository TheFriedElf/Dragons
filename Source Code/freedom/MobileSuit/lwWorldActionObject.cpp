#include "stdafx.h"
#include "lwWorldActionObject.h"

lwWorldActionObject::lwWorldActionObject(PgWorldActionObject *pkWorldActionObject)
{
	m_pWorldActionObject = pkWorldActionObject;
}
bool	lwWorldActionObject::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwWorldActionObject>(pkState, "WorldActionObject")
		.def(pkState, constructor<PgWorldActionObject *>())
		.def(pkState, "OnPacketReceive", &lwWorldActionObject::OnPacketReceive)
		.def(pkState, "Enter", &lwWorldActionObject::Enter)
		.def(pkState, "IsNil", &lwWorldActionObject::IsNil)
		.def(pkState, "GetElapsedTime",&lwWorldActionObject::GetElapsedTime)
		.def(pkState, "GetCurrentStage", &lwWorldActionObject::GetCurrentStage)
		.def(pkState, "NextStage", &lwWorldActionObject::NextStage)
		.def(pkState, "GetDuration", &lwWorldActionObject::GetDuration)

		.def(pkState, "SetParamInt",&lwWorldActionObject::SetParamInt)
		.def(pkState, "SetParamFloat",&lwWorldActionObject::SetParamFloat)
		.def(pkState, "SetParamString",&lwWorldActionObject::SetParamString)
		.def(pkState, "SetParamGUID",&lwWorldActionObject::SetParamGUID)
		.def(pkState, "SetParamStringPair", &lwWorldActionObject::SetParamStringPair)

		.def(pkState, "GetParamInt",&lwWorldActionObject::GetParamInt)
		.def(pkState, "GetParamFloat",&lwWorldActionObject::GetParamFloat)
		.def(pkState, "GetParamString",&lwWorldActionObject::GetParamString)
		.def(pkState, "GetParamGUID",&lwWorldActionObject::GetParamGUID)
		.def(pkState, "GetParamStringPair", &lwWorldActionObject::GetParamStringPair)

		;
	return true;
}
void	lwWorldActionObject::OnPacketReceive(lwPacket packet)
{
	m_pWorldActionObject->OnPacketReceive(packet);
}
void	lwWorldActionObject::Enter(int ulElapsedTime)
{
	m_pWorldActionObject->OnEnter((unsigned long)ulElapsedTime);
}

int		lwWorldActionObject::GetCurrentStage()
{
	return m_pWorldActionObject->GetCurrentStage();
}

void	lwWorldActionObject::NextStage()
{
	m_pWorldActionObject->NextStage();
}

DWORD	lwWorldActionObject::GetDuration()
{
	return m_pWorldActionObject->GetDuration();
}

void	lwWorldActionObject::SetParamInt(int iID,int iValue)
{
	m_pWorldActionObject->SetParam(iID,iValue);
}
void	lwWorldActionObject::SetParamFloat(int iID,float fValue)
{
	m_pWorldActionObject->SetParam(iID,fValue);
}
void	lwWorldActionObject::SetParamString(int iID,char const* Value)
{
	m_pWorldActionObject->SetParam(iID,std::string(Value));
}
void	lwWorldActionObject::SetParamGUID(int iID,lwGUID Value)
{
	m_pWorldActionObject->SetParam(iID,Value());
}

void	lwWorldActionObject::SetParamStringPair(char const *pcKey, char const *pcValue)
{
	m_pWorldActionObject->SetParam(pcKey, pcValue);
}

int	lwWorldActionObject::GetParamInt(int iID)
{
	return m_pWorldActionObject->GetParamInt(iID);
}
float	lwWorldActionObject::GetParamFloat(int iID)
{
	return	m_pWorldActionObject->GetParamFloat(iID);
}
char const* lwWorldActionObject::GetParamString(int iID)
{
	return	m_pWorldActionObject->GetParamString(iID).c_str();
}

char const *lwWorldActionObject::GetParamStringPair(char const *pcKey)
{
	return m_pWorldActionObject->GetParam(pcKey);
}

lwGUID lwWorldActionObject::GetParamGUID(int iID)
{
	return	lwGUID(m_pWorldActionObject->GetParamGUID(iID));
}
bool	lwWorldActionObject::IsNil()
{
	if(m_pWorldActionObject == 0) return true;
	return false;
}
int		lwWorldActionObject::GetElapsedTime()
{
	//return	m_pWorldActionObject->GetElapsedTime();
	return 0;
}

