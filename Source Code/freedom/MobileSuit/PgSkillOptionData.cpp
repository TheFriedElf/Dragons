#include "stdafx.h"
#include "PgSkillOptionData.H"
#include "PgSkillOptionCommandKey.H"

PgSkillOptionData::PgSkillOptionData()
{
	RegisterPrototypesToFactory();
}
void	PgSkillOptionData::RegisterPrototypesToFactory()
{
	GetFactory().RegisterPrototype(NiNew PgSkillOptionCommandKey(true));
}

PgSkillOptionData::~PgSkillOptionData()
{
	ReleaseAllSkillOption();
}
void	PgSkillOptionData::ReleaseAllSkillOption()
{
	m_kSkillOptionDataCont.clear();
}
PgSkillOption*	PgSkillOptionData::GetSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)	const
{
	Type_SkillOption_Map	*pkType_SkillOption_Cont= Find_Type_SkillOption_Cont(dwKeySkillID);
	PgSkillOption	*pkSkillOption = FindSkillOption(kSkillOptionType,pkType_SkillOption_Cont);

	return	pkSkillOption;
}
PgSkillOptionData::Type_SkillOption_Map*	PgSkillOptionData::Find_Type_SkillOption_Cont(DWORD dwKeySkillID)	const
{
	SkillID_SkillOption_Map::const_iterator itor = m_kSkillOptionDataCont.find(dwKeySkillID);
	if(itor == m_kSkillOptionDataCont.end())
	{
		return	NULL;
	}

	Type_SkillOption_Map	*pkFound = const_cast<Type_SkillOption_Map*>(&(itor->second));
	return	pkFound;
}

PgSkillOption*	PgSkillOptionData::FindSkillOption(PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType,Type_SkillOption_Map *pkType_SkillOption_Cont)	const
{
	if(!pkType_SkillOption_Cont)
	{
		return	NULL;
	}

	Type_SkillOption_Map::const_iterator	itor = pkType_SkillOption_Cont->find(kSkillOptionType);
	if( itor == pkType_SkillOption_Cont->end())
	{
		return	NULL;
	}

	PgSkillOption	*pkSkillOption = itor->second;

	return	pkSkillOption;
}

PgSkillOption*	PgSkillOptionData::AddSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)
{
	Type_SkillOption_Map	*pkType_SkillOption_Cont = Find_Type_SkillOption_Cont(dwKeySkillID);
	if(pkType_SkillOption_Cont == NULL)
	{
		pkType_SkillOption_Cont = Add_Type_SkillOption_Cont(dwKeySkillID);
	}
	
	PgSkillOption	*pkSkillOption = FindSkillOption(kSkillOptionType,pkType_SkillOption_Cont);
	if(!pkSkillOption)
	{
		pkSkillOption = GetFactory().CreateSkillOption(kSkillOptionType);
		pkType_SkillOption_Cont->insert(std::make_pair(kSkillOptionType,pkSkillOption));
	}

	return	pkSkillOption;
}
PgSkillOptionData::Type_SkillOption_Map*	PgSkillOptionData::Add_Type_SkillOption_Cont(DWORD dwKeySkillID)
{
	m_kSkillOptionDataCont.insert(std::make_pair(dwKeySkillID,Type_SkillOption_Map()));
	return	Find_Type_SkillOption_Cont(dwKeySkillID);
}
