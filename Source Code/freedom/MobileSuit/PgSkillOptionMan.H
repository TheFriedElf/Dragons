#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONMAN_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONMAN_H

#include <NiMain.H>
#include "CreateUsingNiNew.inl"
#include "PgSkillOptionData.H"

class	PgSkillOptionMan	:	public	NiObject
{

	friend	struct CreateUsingNiNew<PgSkillOptionMan>;

protected:

	PgSkillOptionMan()	{};
	virtual	~PgSkillOptionMan()	{};

public:

	void	Initialize();
	void	Terminate();

public:

	bool	LoadFromFile(BM::GUID const &kPlayerGuid);
	bool	SaveToFile(BM::GUID const &kPlayerGuid);

public:

	PgSkillOption*	AddSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType);
	PgSkillOption*	GetSkillOption(DWORD dwKeySkillID,PgSkillOption::SKILL_OPTION_TYPE const &kSkillOptionType);

	void	ClearAllSkillOption();

private:

	void	CreateSkillOptionData();
	void	ReleaseSkillOptionData();

private:

	PgSkillOptionDataPtr	m_spSkillOptionData;

};
#define g_kSkillOptionMan SINGLETON_CUSTOM(PgSkillOptionMan, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONMAN_H