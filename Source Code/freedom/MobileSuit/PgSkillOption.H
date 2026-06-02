#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTION_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTION_H

#include <NiMain.H>

class	PgSkillOption	:	public	NiObject
{
	friend	class	PgSkillOptionFactory;
	NiDeclareRTTI;

public:

	enum	SKILL_OPTION_TYPE
	{
		SOT_UNKNOWN=0,
		SOT_ENABLE_COMMAND_KEY,
	};

public:

	virtual	SKILL_OPTION_TYPE	GetType()	const	= 0;

	virtual	void	SaveToFile(FILE *fp)	const	{};
	virtual	void	LoadFromFile(FILE *fp)	{};

protected:

	virtual	PgSkillOption*	Clone() = 0;

};

NiSmartPointer(PgSkillOption);

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTION_H