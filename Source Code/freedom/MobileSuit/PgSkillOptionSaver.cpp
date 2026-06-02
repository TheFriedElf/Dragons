#include "stdafx.h"
#include "PgSkillOptionSaver.H"
#include "PgSkillOptionDataFile.H"
	
bool	PgSkillOptionSaver::SaveToFile(PgSkillOptionData* pkSkillOptionData,BM::GUID const &kPlayerGuid)
{
	if(!pkSkillOptionData)
	{
		return	false;
	}

	FILE	*fp = PgSkillOptionDataFile::OpenForWrite(kPlayerGuid);
	if(!fp)
	{
		return false;
	}

	unsigned	short	usVersion = 0;
	fwrite(&usVersion,sizeof(unsigned short),1,fp);

	PgSkillOptionData::SkillID_SkillOption_Map	const	&kSkillOptionMap = pkSkillOptionData->GetSkillOptionContainerForSaveToFile();
	unsigned	short	usTotalSkill = kSkillOptionMap.size();
	fwrite(&usTotalSkill,sizeof(unsigned short),1,fp);

	for(PgSkillOptionData::SkillID_SkillOption_Map::const_iterator itor = kSkillOptionMap.begin(); itor != kSkillOptionMap.end(); ++itor)
	{
		DWORD	dwSkillID = itor->first;
		PgSkillOptionData::Type_SkillOption_Map const	*pkType_SkillOption_Cont = &itor->second;

		fwrite(&dwSkillID,sizeof(DWORD),1,fp);
		Save_Type_SkillOption_Cont(fp,pkType_SkillOption_Cont);
	}

	fclose(fp);

	return	true;
}
void	PgSkillOptionSaver::Save_Type_SkillOption_Cont(FILE *fp,PgSkillOptionData::Type_SkillOption_Map const	*pkType_SkillOption_Cont)
{
	unsigned	short	usNumOptions = pkType_SkillOption_Cont->size();
	fwrite(&usNumOptions,sizeof(unsigned short),1,fp);

	for(PgSkillOptionData::Type_SkillOption_Map::const_iterator itor = pkType_SkillOption_Cont->begin(); itor != pkType_SkillOption_Cont->end(); ++itor)
	{
		PgSkillOption	const	*pkSkillOption = itor->second;
		PgSkillOption::SKILL_OPTION_TYPE	kSkillOptionType = pkSkillOption->GetType();
		fwrite(&kSkillOptionType,sizeof(PgSkillOption::SKILL_OPTION_TYPE),1,fp);
		pkSkillOption->SaveToFile(fp);
	}
}