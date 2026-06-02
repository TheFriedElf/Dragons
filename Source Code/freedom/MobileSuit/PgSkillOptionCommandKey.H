#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONCOMMANDKEY_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONCOMMANDKEY_H

#include "PgSkillOption.H"

class	PgSkillOptionCommandKey	:	public	PgSkillOption
{
	NiDeclareRTTI;

public:

	PgSkillOptionCommandKey(bool bEnableCommandKey):
		m_bEnableCommandKey(bEnableCommandKey)
	{
	};

	void	SetEnableCommandKey(bool const &bEnable)	{	m_bEnableCommandKey = bEnable;	}
	bool	GetEnableCommandKey()	const	{	return	m_bEnableCommandKey;	}

	virtual	void	SaveToFile(FILE *fp)	const;
	virtual	void	LoadFromFile(FILE *fp);

	virtual	SKILL_OPTION_TYPE	GetType()	const	{	return	PgSkillOptionCommandKey::GetTypeS();	}
	static	SKILL_OPTION_TYPE	GetTypeS()	{	return	PgSkillOption::SOT_ENABLE_COMMAND_KEY;	}

protected:

	virtual	PgSkillOption*	Clone()
	{
		return	static_cast<PgSkillOption*>(NiNew PgSkillOptionCommandKey(GetEnableCommandKey()));
	}
	
private:

	bool	m_bEnableCommandKey;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONCOMMANDKEY_H