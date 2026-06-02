#include "stdafx.h"
#include "PgEventScriptData.H"

unsigned	long	PgEventScriptData::m_ulDataFileVersion = 0;


PgEventScriptData::PgEventScriptData() : m_iEventID(-1),m_iInstanceCount(0)
{
}

PgEventScriptData::~PgEventScriptData()
{
	ReleaseAll();
}

void	PgEventScriptData::ReleaseAll()
{
	int	iTotal = m_kEventScriptCmdCont.size();
	for(int i=0;i<iTotal;i++)
	{
		SAFE_DELETE(m_kEventScriptCmdCont[i]);
	}
	m_kEventScriptCmdCont.clear();
}

bool	PgEventScriptData::CheckLoadFromXML()	const	
{
	bool	bLoadFromXml = false;

	int	const	iEventID = GetEventID();

	char	xmlfilepath[512];
	char	binfilepath[512];

	sprintf_s(xmlfilepath,512,"../XML/EventScript/Event%d.XML",iEventID);
	sprintf_s(binfilepath,512,"../XML/EventScript/Event%d.BIN",iEventID);

	//	pack 을 사용하는 상태라면 XML PACK 파일 생성시간과 비교한다. 
	//	의문 : pack 파일이 만약 이미 열려진 상태라면, FindFirstFile 로 다시 핸들을 얻어 올수 있는가?
	if(g_bUsePackData)
	{
		strcpy_s(xmlfilepath,512,PgXmlLoader::GetPackFilePath());
	}

	HANDLE	hXMLFile = CreateFile(UNI(xmlfilepath),FILE_READ_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	HANDLE	hBINFile = CreateFile(UNI(binfilepath),FILE_READ_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if(hXMLFile != INVALID_HANDLE_VALUE && hBINFile != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION	kXMLFileInfo,kBINFileInfo;

		GetFileInformationByHandle(hXMLFile,&kXMLFileInfo);
		GetFileInformationByHandle(hBINFile,&kBINFileInfo);

		//	CompareFileTime : 1 = First file time is later than second file time
		bLoadFromXml = (CompareFileTime(&kXMLFileInfo.ftLastWriteTime,&kBINFileInfo.ftLastWriteTime) == 1);	
	}
	else if(hXMLFile != INVALID_HANDLE_VALUE && hBINFile == INVALID_HANDLE_VALUE)
	{
		bLoadFromXml = true;
	}

	if(hXMLFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hXMLFile);
	}
	if(hBINFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hBINFile);
	}

	return	bLoadFromXml;
}
bool	PgEventScriptData::LoadFromFile(int const iEventID)
{
	SetEventID(iEventID);

	//	File 이름 생성
	char	xmlfilepath[512];
	char	binfilepath[512];

	sprintf_s(xmlfilepath,512,"EventScript/Event%d.XML",iEventID);
	sprintf_s(binfilepath,512,"../XML/EventScript/Event%d.BIN",iEventID);

	//	Binary FILE과 XML FILE과의 최종 수정시간을 비교하여, xml 로부터 로딩해야할지 bin 에서 로딩해야할지 체크한다.
	if(CheckLoadFromXML())	
	{
		if(LoadFromXml(xmlfilepath))
		{
			//	Binary 파일로 저장해두자.
			SaveToBinary(binfilepath);
			return	true;
		}
		return	false;
	}

	//	Binary 파일에서 로딩하자.
	if(!LoadFromBinary(binfilepath))
	{
		//	Binary 에서 로딩 실패 할 경우, XML 에서 로딩 시도해본다.
		if(LoadFromXml(xmlfilepath))
		{
			//	Binary 파일로 저장해두자.
			SaveToBinary(binfilepath);
			return	true;
		}
		return	false;
	}

	return	true;
}

bool	PgEventScriptData::LoadFromXml(char const *pkDataFilePath)
{
	TiXmlDocument kXmlDoc(pkDataFilePath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pkDataFilePath)))
	{
		PgError1("PgEventScriptData::LoadFromXml LoadFile() Failed [%s]", pkDataFilePath);
		return false;
	}

	// Find Root
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	pkElement = pkElement->FirstChildElement();

	int	iCount = 0;
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(stricmp(pcTagName,"CMD")==0)
		{
			AddNewCmd(PgEventScriptCmd::Create(pkElement->FirstAttribute(),GetEventID(),iCount++));
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return	true;
}

PgEventScriptCmd*	PgEventScriptData::AddNewCmd(PgEventScriptCmd *pkCmd)
{
	PG_ASSERT_LOG(pkCmd);
	if(!pkCmd)
	{
		return	NULL;
	}

	m_kEventScriptCmdCont.push_back(pkCmd);

	return	pkCmd;
}

bool	PgEventScriptData::LoadFromBinary(char const *pkDataFilePath)
{
	ReleaseAll();

	FILE	*fp = fopen(pkDataFilePath,"rb");
	if(!fp)
	{
		return	false;
	}

	//	Load version
	unsigned	long	ulVersion = 0;
	fread(&ulVersion,sizeof(ulVersion),1,fp);

	if(ulVersion != m_ulDataFileVersion)	//	버전이 안맞으면 로딩 실패
	{
		fclose(fp);
		return	false;
	}

	//	Load Count
	unsigned	short	usCount = 0;
	fread(&usCount,sizeof(usCount),1,fp);

	//	Load each commands
	for(unsigned short i =0;i<usCount;i++)
	{
		if(AddNewCmd(PgEventScriptCmd::Create(fp,GetEventID(),i)) == NULL)	//	로딩 실패
		{
			fclose(fp);
			return	false;
		}
	}

	fclose(fp);

	return	true;
}

void	PgEventScriptData::SaveToBinary(char const *pkDataFilePath)	const
{
	FILE	*fp = fopen(pkDataFilePath,"wb");
	if(!fp)
	{
		return;
	}

	//	Save version
	fwrite(&m_ulDataFileVersion,sizeof(m_ulDataFileVersion),1,fp);

	//	Save Count
	unsigned	short	usCount = m_kEventScriptCmdCont.size();
	fwrite(&usCount,sizeof(usCount),1,fp);

	//	Save each commands
	for(unsigned short i =0;i<usCount;i++)
	{
		PgEventScriptCmd	*pkCmd = m_kEventScriptCmdCont[i];
		pkCmd->SaveToBinary(fp);
	}

	fclose(fp);
}


bool	PgEventScriptData::Reload()
{
	if(GetInstanceCount() != 0)	//	이 데이터를 사용하는 인스턴스가 하나도 없어야 리로딩이 가능하다.
	{
		_PgMessageBox("PgEventScriptData::Reload Failed","InstanceCount of Event %d should be 0. current : %d",GetEventID(),GetInstanceCount());
		return	false;
	}

	ReleaseAll();

	return	LoadFromFile(GetEventID());
}