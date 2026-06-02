#ifndef FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVENTSCRIPTCMD_H
#define FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVENTSCRIPTCMD_H
#include "PgScripting.h"
#include "PgEventScriptCmd.H"

class	lwPoint3;

LW_CLASS(PgEventScriptCmd, EventScriptCmd)

	char	const*	GetType();

	char	const*	GetAttr(char	const*	strAttrName);
	int	GetAttrInt(char	const*	strAttrName);
	float	GetAttrFloat(char	const*	strAttrName);
	lwPoint3	GetAttrPoint3(char	const*	strAttrName);
	DWORD	GetAttrColor(char	const*	strAttrName);
	bool	GetAttrBool(char	const*	strAttrName);

	char	const*	GetParam(char	const*	strParamName);
	int	GetParamInt(char	const*	strParamName);
	float	GetParamFloat(char	const*	strParamName);
	lwPoint3	GetParamPoint3(char	const*	strParamName);
	DWORD	GetParamColor(char	const*	strParamName);

	void	SetParam(char	const*	strParamName,char	const*	strParamValue);
	void	SetParamInt(char	const*	strParamName,int iParamValue);
	void	SetParamFloat(char	const*	strParamName,float fParamValue);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_EVENTSCRIPTSYSTEM_LWEVENTSCRIPTCMD_H