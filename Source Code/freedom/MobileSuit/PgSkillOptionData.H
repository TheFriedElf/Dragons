#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONDATA_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONDATA_H

#include "PgSkillOption.H"
#include "PgSkillOptionFactory.H"

class	PgSkillOptionData	:	public	NiObject
{

	friend	class	PgSkillOptionSaver;

public:

	typedef	std::map<PgSkillOption::SKILL_OPTION_TYPE,PgSkillOptionPtr> Type_SkillOption_Map;
	typedef	std::map<DWORD,Type_SkillOption_Map> SkillID_SkillOption_Map;

public:

	PgSkillOptionData();
	virtual	~PgSkillOptionData();

	PgSkillOption*	GetSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)	const;
	PgSkillOption*	AddSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType);

	void	ReleaseAllSkillOption();

protected:

	SkillID_SkillOption_Map	const&	GetSkillOptionContainerForSaveToFile()	const	{	return	m_kSkillOptionDataCont;	}

private:

	Type_SkillOption_Map*	Find_Type_SkillOption_Cont(DWORD dwKeySkillID)	const;
	PgSkillOption*	FindSkillOption(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType,Type_SkillOption_Map *pkType_SkillOption_Cont)	const;

	Type_SkillOption_Map*	Add_Type_SkillOption_Cont(DWORD dwKeySkillID);
	
	PgSkillOptionFactory&	GetFactory()	{	return	m_kSkillOptionFactory;	}

	void	RegisterPrototypesToFactory();

private:

	SkillID_SkillOption_Map	m_kSkillOptionDataCont;
	PgSkillOptionFactory	m_kSkillOptionFactory;
};

NiSmartPointer(PgSkillOptionData);

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONDATA_H