#include "stdafx.h"
#include "lwEventScriptCmd.H"
#include "lwPoint3.H"

using namespace lua_tinker;

void lwEventScriptCmd::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(EventScriptCmd)
		LW_REG_METHOD(EventScriptCmd, GetType)

		LW_REG_METHOD(EventScriptCmd, GetAttr)
		LW_REG_METHOD(EventScriptCmd, GetAttrInt)
		LW_REG_METHOD(EventScriptCmd, GetAttrFloat)
		LW_REG_METHOD(EventScriptCmd, GetAttrPoint3)
		LW_REG_METHOD(EventScriptCmd, GetAttrColor)
		LW_REG_METHOD(EventScriptCmd, GetAttrBool)

		LW_REG_METHOD(EventScriptCmd, GetParam)
		LW_REG_METHOD(EventScriptCmd, GetParamInt)
		LW_REG_METHOD(EventScriptCmd, GetParamFloat)
		LW_REG_METHOD(EventScriptCmd, GetParamPoint3)
		LW_REG_METHOD(EventScriptCmd, GetParamColor)

		LW_REG_METHOD(EventScriptCmd, SetParam)
		LW_REG_METHOD(EventScriptCmd, SetParamInt)
		LW_REG_METHOD(EventScriptCmd, SetParamFloat)
		;
}

char	const*	lwEventScriptCmd::GetType()
{
	return	m_pkEventScriptCmd->GetType().c_str();
}

char	const*	lwEventScriptCmd::GetAttr(char	const*	strAttrName)
{
	return	m_pkEventScriptCmd->GetAttr(strAttrName).c_str();
}
int	lwEventScriptCmd::GetAttrInt(char	const*	strAttrName)
{
	return	m_pkEventScriptCmd->GetAttrInt(strAttrName);
}
float	lwEventScriptCmd::GetAttrFloat(char	const*	strAttrName)
{
	return	m_pkEventScriptCmd->GetAttrFloat(strAttrName);
}
lwPoint3	lwEventScriptCmd::GetAttrPoint3(char	const*	strAttrName)
{
	char	const	*pkAttr = GetAttr(strAttrName);
	NiPoint3	kPosition;

	sscanf_s(pkAttr,"%f,%f,%f",&kPosition.x,&kPosition.y,&kPosition.z);

	return	lwPoint3(kPosition);
}
DWORD	lwEventScriptCmd::GetAttrColor(char	const*	strAttrName)
{
	char	const	*pkAttr = GetAttr(strAttrName);
	int	iR=255,iG=255,iB=255;

	sscanf_s(pkAttr,"%d,%d,%d",&iR,&iG,&iB);

	return (0xff000000 | iR<<16 | iG<<8 | iB);
}
bool	lwEventScriptCmd::GetAttrBool(char	const*	strAttrName)
{
	char	const	*pkAttr = GetAttr(strAttrName);
	if(strlen(pkAttr) == 0)
	{
		return	false;
	}

	if(stricmp(pkAttr,"true") == 0)
	{
		return	true;
	}
	if(stricmp(pkAttr,"false") == 0)
	{
		return	false;
	}

	_PgMessageBox("EventScriptCommand Bool Parameter Error","[Event %d CmdNum %d] Invalid Bool Value Attr:%s Value:%s",m_pkEventScriptCmd->GetEventID(),m_pkEventScriptCmd->GetCommandNumber(),strAttrName,pkAttr);
	return	false;
}

char	const*	lwEventScriptCmd::GetParam(char	const*	strParamName)
{
	return	m_pkEventScriptCmd->GetParam(strParamName).c_str();
}
int	lwEventScriptCmd::GetParamInt(char	const*	strParamName)
{
	return	m_pkEventScriptCmd->GetParamInt(strParamName);
}
float	lwEventScriptCmd::GetParamFloat(char	const*	strParamName)
{
	return	m_pkEventScriptCmd->GetParamFloat(strParamName);
}
lwPoint3	lwEventScriptCmd::GetParamPoint3(char	const*	strParamName)
{

	char	const	*pkParam = GetParam(strParamName);
	NiPoint3	kPosition;

	sscanf_s(pkParam,"%f,%f,%f",&kPosition.x,&kPosition.y,&kPosition.z);

	return	lwPoint3(kPosition);
}
DWORD	lwEventScriptCmd::GetParamColor(char	const*	strParamName)
{
	char	const	*pkParam = GetParam(strParamName);
	int	iR=255,iG=255,iB=255;

	sscanf_s(pkParam,"%d,%d,%d",&iR,&iG,&iB);

	return (0xff000000 | iR<<16 | iG<<8 | iB);
}

void	lwEventScriptCmd::SetParam(char	const*	strParamName,char	const*	strParamValue)
{
	m_pkEventScriptCmd->SetParam(strParamName,strParamValue);
}
void	lwEventScriptCmd::SetParamInt(char	const*	strParamName,int iParamValue)
{
	m_pkEventScriptCmd->SetParamInt(strParamName,iParamValue);
}
void	lwEventScriptCmd::SetParamFloat(char	const*	strParamName,float fParamValue)
{
	m_pkEventScriptCmd->SetParamFloat(strParamName,fParamValue);
}