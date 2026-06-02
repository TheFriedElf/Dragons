#include "stdafx.h"
#include "PgSkillOptionFactory.H"


void	PgSkillOptionFactory::RegisterPrototype(PgSkillOption *pkSkillOption)
{
	assert(pkSkillOption);
	if(!pkSkillOption)
	{
		return;
	}

	PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType = pkSkillOption->GetType();

	if(GetPrototype(kSkillOptionType))
	{
		return;
	}

	m_kSkillOptionPrototypeCont.insert(std::make_pair(kSkillOptionType,pkSkillOption));
}
PgSkillOption*	PgSkillOptionFactory::CreateSkillOption(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)
{
	PgSkillOption	*pkPrototype = GetPrototype(kSkillOptionType);
	if(!pkPrototype)
	{
		return	NULL;
	}

	return	pkPrototype->Clone();
}
PgSkillOption*	PgSkillOptionFactory::GetPrototype(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)
{

	SkillOptionPrototypeMap::iterator itor = m_kSkillOptionPrototypeCont.find(kSkillOptionType);
	if(itor == m_kSkillOptionPrototypeCont.end())
	{
		return	NULL;
	}

	PgSkillOption	*pkSkillOption = itor->second;
	return	pkSkillOption;
}