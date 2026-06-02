#include "stdafx.h"
#include "PgSkillOptionCommandKey.H"

NiImplementRTTI(PgSkillOptionCommandKey, PgSkillOption);

void	PgSkillOptionCommandKey::SaveToFile(FILE *fp)	const
{
	PgSkillOption::SaveToFile(fp);

	fwrite(&m_bEnableCommandKey,sizeof(bool),1,fp);
}
void	PgSkillOptionCommandKey::LoadFromFile(FILE *fp)
{
	PgSkillOption::LoadFromFile(fp);

	fread(&m_bEnableCommandKey,sizeof(bool),1,fp);
}
