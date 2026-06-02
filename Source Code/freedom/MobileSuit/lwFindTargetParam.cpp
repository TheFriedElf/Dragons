#include "stdafx.h"
#include "lwFindTargetParam.H"
#include "lwPoint3.H"

lwFindTargetParam::lwFindTargetParam()
{
	m_fRange = m_fWideAngle = m_fEffectRadius = 0;
	m_iMaxTargets = 0;
	m_bNoDuplication = false;
	m_iFindTargetOption = 0;
}

bool lwFindTargetParam::RegisterWrapper(lua_State *pkState)
{ 
	using namespace lua_tinker; 

	class_<lwFindTargetParam>(pkState, "FindTargetParam")
		.def(pkState, constructor<>())
		.def(pkState, "SetParam_1", &lwFindTargetParam::SetParam_1)
		.def(pkState, "SetParam_2", &lwFindTargetParam::SetParam_2)
		.def(pkState, "SetParam_3", &lwFindTargetParam::SetParam_3)
		;

	return true;
}
void	lwFindTargetParam::SetParam_1(lwPoint3 kStart,lwPoint3 kDir)
{
	m_kStart = kStart();
	m_kDir = kDir();
}
void	lwFindTargetParam::SetParam_2(float fRange,float fWideAngle,float fEffectRadius,int iMaxTargets)
{
	m_fRange = fRange;
	m_fWideAngle = fWideAngle;
	m_fEffectRadius = fEffectRadius;
	m_iMaxTargets = iMaxTargets;
}
void	lwFindTargetParam::SetParam_3(bool bNoDuplication,int iFindTargetOption)
{
	m_bNoDuplication = bNoDuplication;
	m_iFindTargetOption = iFindTargetOption;
}