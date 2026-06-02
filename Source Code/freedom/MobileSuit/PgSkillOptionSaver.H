#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONSAVER_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONSAVER_H

#include "PgSkillOptionData.H"

class	PgSkillOptionSaver
{
public:

	static	bool	SaveToFile(PgSkillOptionData* pkSkillOptionData,BM::GUID const &kPlayerGuid);

private:

	static	void	Save_Type_SkillOption_Cont(FILE *fp,PgSkillOptionData::Type_SkillOption_Map const	*pkType_SkillOption_Cont);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILLOPTIONSYSTEM_PGSKILLOPTIONSAVER_H