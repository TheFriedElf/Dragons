#include "stdafx.h"
#include "lwSkillDef.h"
#include "lwWString.H"

lwSkillDef	lwGetSkillDef(int iSkillNo)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	const	CSkillDef	*pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	return	lwSkillDef(pkSkillDef);
}

/*
void	lwReleaseSkillDef(lwSkillDef kSkillDef)
{
}
*/

//! 스크립팅 시스템에 등록한다.
bool lwSkillDef::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "GetSkillDef", &lwGetSkillDef);
	//def(pkState, "ReleaseSkillDef", &lwReleaseSkillDef);

	class_<lwSkillDef>(pkState, "SkillDef")
		.def(pkState, constructor<CSkillDef*>())
		.def(pkState, "GetSkillDef", &lwSkillDef::GetSkillDef)
		.def(pkState, "IsNil", &lwSkillDef::IsNil)
		.def(pkState, "GetEffectNo", &lwSkillDef::GetEffectNo)
		.def(pkState, "GetSkillType", &lwSkillDef::GetSkillType)
		.def(pkState, "GetAbil", &lwSkillDef::GetAbil)
		.def(pkState, "GetActionName", &lwSkillDef::GetActionName)
		.def(pkState, "IsSkillAtt", &lwSkillDef::IsSkillAtt)
		;

	return true;
}
bool	lwSkillDef::GetSkillType(int iSkillType)
{
	return ((m_pkSkillDef->GetAbil(AT_SKILL_ATT) & iSkillType) == 0) ? false : true;
}
int	lwSkillDef::GetAbil(int iAbilType)
{
	return	m_pkSkillDef->GetAbil(iAbilType);
}
lwWString	lwSkillDef::GetActionName()
{
	std::wstring kActionName(m_pkSkillDef->GetActionName());
	return	lwWString(kActionName);
}

lwSkillDef	lwSkillDef::GetSkillDef()
{
	return	lwSkillDef(m_pkSkillDef);
}
int	lwSkillDef::GetEffectNo()
{
	return	m_pkSkillDef->GetEffectNo();
}

bool	lwSkillDef::IsNil()
{
	return	m_pkSkillDef==NULL;
}

bool 	lwSkillDef::IsSkillAtt(int iType) const
{
	if (m_pkSkillDef)
	{
		return m_pkSkillDef->IsSkillAtt((ESkillAttribute)iType);
	}
	return false;
	
}