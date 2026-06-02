#include "stdafx.h"
#include "PgSkillOptionLoader.H"
#include "PgSkillOptionDataFile.H"
	
bool	PgSkillOptionLoader::LoadFromFile(PgSkillOptionData* pkSkillOptionData,BM::GUID const &kPlayerGuid)
{
	FILE	*fp = PgSkillOptionDataFile::OpenForRead(kPlayerGuid);
	if(!fp)
	{
		return false;
	}

	unsigned	short	usVersion = 0;
	fread(&usVersion,sizeof(unsigned short),1,fp);

	unsigned	short	usTotalSkill = 0;
	fread(&usTotalSkill,sizeof(unsigned short),1,fp);

	for(unsigned int ui=0;ui<usTotalSkill;++ui)
	{
		DWORD	dwSkillID;
		fread(&dwSkillID,sizeof(DWORD),1,fp);

		LoadSkillOptions(fp,dwSkillID,pkSkillOptionData);
	}

	fclose(fp);

	return	true;
}
void	PgSkillOptionLoader::LoadSkillOptions(FILE *fp,DWORD const &dwSkillID,PgSkillOptionData* pkSkillOptionData)
{
	unsigned	short	usNumOptions = 0;
	fread(&usNumOptions,sizeof(unsigned short),1,fp);

	for(unsigned int ui=0;ui<usNumOptions;++ui)
	{
		PgSkillOption::SKILL_OPTION_TYPE kSkillOptionType;
		fread(&kSkillOptionType,sizeof(PgSkillOption::SKILL_OPTION_TYPE),1,fp);

		PgSkillOption	*pkSkillOption = pkSkillOptionData->GetSkillOption(dwSkillID,kSkillOptionType);
		if(!pkSkillOption)
		{
			pkSkillOption = pkSkillOptionData->AddSkillOption(dwSkillID,kSkillOptionType);
		}

		pkSkillOption->LoadFromFile(fp);
	}
}