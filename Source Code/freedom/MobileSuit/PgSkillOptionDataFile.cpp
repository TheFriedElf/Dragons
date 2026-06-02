#include "stdafx.h"
#include "PgSkillOptionDataFile.H"
#include "PgHelpSystem.H"

const	std::wstring	SKILL_OPTION_DATA_FILE_NAME = std::wstring(_T("SkillOption.Dat"));

FILE*	PgSkillOptionDataFile::OpenForRead(BM::GUID const &kPlayerGuid)
{
	std::wstring kFilePath;
	if(!PgHelpSystem::ConvertGuidToSavePathName(kPlayerGuid,kFilePath))
	{
		return	NULL;
	}

	FILE	*fp = _wfopen((kFilePath+SKILL_OPTION_DATA_FILE_NAME).c_str(),_T("rb"));
	
	return	fp;
}
FILE*	PgSkillOptionDataFile::OpenForWrite(BM::GUID const &kPlayerGuid)
{
	std::wstring kFilePath;
	if(!PgHelpSystem::ConvertGuidToSavePathName(kPlayerGuid,kFilePath))
	{
		return	NULL;
	}

	FILE	*fp = _wfopen((kFilePath+SKILL_OPTION_DATA_FILE_NAME).c_str(),_T("wb"));
	
	return	fp;
}
