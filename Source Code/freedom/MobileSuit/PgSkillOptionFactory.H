#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONFACTORY_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONFACTORY_H

#include "PgSkillOption.H"

class	PgSkillOptionFactory	
{
	
	typedef	std::map<PgSkillOption::SKILL_OPTION_TYPE,PgSkillOptionPtr>	SkillOptionPrototypeMap;

public:

	void	RegisterPrototype(PgSkillOption *pkSkillOption);

public:

	PgSkillOption*	CreateSkillOption(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType);

private:

	PgSkillOption*	GetPrototype(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType);

private:

	SkillOptionPrototypeMap	m_kSkillOptionPrototypeCont;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONFACTORY_H