#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONLOADER_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONLOADER_H

#include "PgSkillOptionData.H"

class	PgSkillOptionLoader
{

public:

	static	bool	LoadFromFile(PgSkillOptionData *pkSkillOptionData,BM::GUID const &kPlayerGuid);

private:

	static	void	LoadSkillOptions(FILE *fp,DWORD const &dwSkillID,PgSkillOptionData *pkSkillOptionData);
	

};

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONLOADER_H