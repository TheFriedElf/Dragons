#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWFINDTARGETPARAM_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWFINDTARGETPARAM_H

class	lwPoint3;
class	lwFindTargetParam
{
public:

	lwFindTargetParam();

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	void	SetParam_1(lwPoint3 kStart,lwPoint3 kDir);
	void	SetParam_2(float fRange,float fWideAngle,float fEffectRadius,int iMaxTargets);
	void	SetParam_3(bool bNoDuplication,int iFindTargetOption);

public:

	NiPoint3	m_kStart,m_kDir;
	float	m_fRange,m_fWideAngle,m_fEffectRadius;
	int	m_iMaxTargets;
	bool	m_bNoDuplication;
	int	m_iFindTargetOption;
};

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWFINDTARGETPARAM_H