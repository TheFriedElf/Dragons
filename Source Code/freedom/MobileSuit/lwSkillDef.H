#ifndef FREEDOM_DRAGONICA_SCRIPTING_DEF_LWSKILLDEF_H
#define FREEDOM_DRAGONICA_SCRIPTING_DEF_LWSKILLDEF_H

class	CSkillDef;
class	lwWString;
class	lwSkillDef
{
public:

	lwSkillDef(const	CSkillDef	*pkSkillDef)	{	m_pkSkillDef = pkSkillDef;	};

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwSkillDef	GetSkillDef();
	int	GetEffectNo();
	bool	GetSkillType(int iSkillType);
	int	GetAbil(int iAbilType);
	lwWString	GetActionName();
	bool IsSkillAtt(int iType) const;

	bool	IsNil();

private:

	const	CSkillDef	*m_pkSkillDef;

};
#endif // FREEDOM_DRAGONICA_SCRIPTING_DEF_LWSKILLDEF_H