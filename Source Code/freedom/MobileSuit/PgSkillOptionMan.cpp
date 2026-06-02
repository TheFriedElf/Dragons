#include "stdafx.h"
#include "PgSkillOptionMan.H"
#include "PgSkillOptionLoader.H"
#include "PgSkillOptionSaver.H"


void	PgSkillOptionMan::Initialize()
{
	CreateSkillOptionData();
}
void	PgSkillOptionMan::Terminate()
{
	ReleaseSkillOptionData();
}
bool	PgSkillOptionMan::LoadFromFile(BM::GUID const &kPlayerGuid)
{

	ClearAllSkillOption();

	return	PgSkillOptionLoader::LoadFromFile(m_spSkillOptionData,kPlayerGuid);
}

bool	PgSkillOptionMan::SaveToFile(BM::GUID const &kPlayerGuid)
{
	if(!m_spSkillOptionData)
	{
		return	false;
	}

	return	PgSkillOptionSaver::SaveToFile(m_spSkillOptionData,kPlayerGuid);
}

PgSkillOption*	PgSkillOptionMan::AddSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)
{
	if(!m_spSkillOptionData)
	{
		return	NULL;
	}

	return	m_spSkillOptionData->AddSkillOption(dwKeySkillID,kSkillOptionType);
}

PgSkillOption*	PgSkillOptionMan::GetSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType)
{
	if(!m_spSkillOptionData)
	{
		return	NULL;
	}

	return	m_spSkillOptionData->GetSkillOption(dwKeySkillID,kSkillOptionType);
}

void	PgSkillOptionMan::ClearAllSkillOption()
{
	if(m_spSkillOptionData)
	{
		m_spSkillOptionData->ReleaseAllSkillOption();
	}
}
void	PgSkillOptionMan::CreateSkillOptionData()
{
	m_spSkillOptionData = NiNew PgSkillOptionData();
}
void	PgSkillOptionMan::ReleaseSkillOptionData()
{
	m_spSkillOptionData = 0;
}