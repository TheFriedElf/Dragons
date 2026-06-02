#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_WORLDACTION_LWWORLDACTIONOBJECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_WORLDACTION_LWWORLDACTIONOBJECT_H

#include "PgWorldActionObject.h"

class	lwWorldActionObject
{
public:
	//! 생성자
	lwWorldActionObject(PgWorldActionObject *pkWorldActionObject);

	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	void	OnPacketReceive(lwPacket packet);

	void	Enter(int iElapsedTime);
	bool	IsNil();
	int		GetElapsedTime();
	int		GetCurrentStage();
	void	NextStage();
	DWORD	GetDuration();

	void	SetParamInt(int iID,int iValue);
	void	SetParamFloat(int iID,float fValue);
	void	SetParamString(int iID,char const* Value);
	void	SetParamGUID(int iID,lwGUID Value);
	void	SetParamStringPair(char const *pcKey, char const *pcValue);


	int	GetParamInt(int iID);
	float	GetParamFloat(int iID);
	char const* GetParamString(int iID);
	lwGUID GetParamGUID(int iID);
	char const *GetParamStringPair(char const *pcKey);


private:

	PgWorldActionObject	*m_pWorldActionObject;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_WORLDACTION_LWWORLDACTIONOBJECT_H