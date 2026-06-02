#include "stdafx.h"
#include "PgEventScriptCmd.H"
#include "lwEventScriptCmd.H"

const std::string	NULL_STRING;

///////////////////////////////////////////////////////////////////////////////////
//	class	PgEventScriptCmd
///////////////////////////////////////////////////////////////////////////////////

PgEventScriptCmd::PgEventScriptCmd(int iEventID,int iCommandNumber) 
: m_pkCustomParamMap(NULL),m_iCommandNumber(iCommandNumber),m_iEventID(iEventID)
{
}

PgEventScriptCmd*	PgEventScriptCmd::Create(TiXmlAttribute const *pkAttribute,int iEventID,int iCommandNumber)
{
	PgEventScriptCmd	*pkNewCmd = new PgEventScriptCmd(iEventID,iCommandNumber);
	if(!pkNewCmd)
	{
		return	NULL;
	}

	if(!pkNewCmd->LoadFromXMLAttribute(pkAttribute))
	{
		SAFE_DELETE(pkNewCmd);
		return	NULL;
	}

	return	pkNewCmd;
}

PgEventScriptCmd*	PgEventScriptCmd::Create(FILE *fp,int iEventID,int iCommandNumber)
{

	PgEventScriptCmd	*pkNewCmd = new PgEventScriptCmd(iEventID,iCommandNumber);
	if(!pkNewCmd)
	{
		return	NULL;
	}

	if(!pkNewCmd->LoadFromBinary(fp))
	{
		SAFE_DELETE(pkNewCmd);
		return	NULL;
	}

	return	pkNewCmd;

}

PgEventScriptCmd::~PgEventScriptCmd()
{
}

bool	PgEventScriptCmd::Update(float fAccumTime,float fFrameTime,ParamMap &kCustomParamMap)
{
	m_pkCustomParamMap = &kCustomParamMap;

	std::string	kLuaFuncName = std::string("EventScript_")+m_kType + "_OnUpdate";

	bool	bResult = lua_tinker::call<bool,lwEventScriptCmd,float,float>(kLuaFuncName.c_str(),lwEventScriptCmd(this),fAccumTime,fFrameTime);

	m_pkCustomParamMap  = NULL;

	return	bResult;
}

bool	PgEventScriptCmd::LoadFromXMLAttribute(TiXmlAttribute const *pkAttribute)
{

	while(pkAttribute)
	{
		char const *pcAttrName = pkAttribute->Name();
		char const *pcAttrValue = pkAttribute->Value();

		NiString kString = pcAttrName;
		kString.ToUpper();

		pcAttrName = (char const*)kString;

		NiString kString2 = pcAttrValue;
		if(kString != "FUNC" && kString != "FACEID" && kString != "OBJECTID" && kString != "CAMERAID" && kString != "COMMAND")
		{
			kString2.ToUpper();
		}

		pcAttrValue = (char const*)kString2;

		if(stricmp(pcAttrName, "TYPE") == 0)
		{
			m_kType = pcAttrValue;
		}
		else 
		{
			m_kAttributes.insert(std::make_pair(std::string(pcAttrName),std::string(pcAttrValue)));
		}

		pkAttribute = pkAttribute->Next();
	}

	return	true;
}

bool	PgEventScriptCmd::LoadFromBinary(FILE *fp)
{
	BYTE	byLen = 0;

	//	Read Type
	char	strType[256];
	fread(&byLen,sizeof(byLen),1,fp);
	fread(strType,1,byLen,fp);
	strType[byLen] = '\0';

	SetType(strType);

	//	Read Attribute Count
	unsigned	short	usAttrCount = 0;
	fread(&usAttrCount,sizeof(usAttrCount),1,fp);

	for(unsigned short i=0;i<usAttrCount;i++)
	{
		char	strAttrType[256];
		char	strAttrValue[256];
		//	Attr Type
		fread(&byLen,sizeof(BYTE),1,fp);
		fread(strAttrType,1,byLen,fp);
		strAttrType[byLen] = '\0';
		
		//	Attr Value
		fread(&byLen,sizeof(BYTE),1,fp);
		fread(strAttrValue,1,byLen,fp);
		strAttrValue[byLen] = '\0';
		
		m_kAttributes.insert(std::make_pair(std::string(strAttrType),std::string(strAttrValue)));
	}


	return	true;
}

void	PgEventScriptCmd::SaveToBinary(FILE *fp)
{
	BYTE	byLen = 0;

	//	Write Type
	char	strType[256];
	strcpy_s(strType,256,GetType().c_str());
	byLen = strlen(strType);
	fwrite(&byLen,sizeof(byLen),1,fp);
	fwrite(strType,1,byLen,fp);

	//	Write Attribute Count
	unsigned	short	usAttrCount = m_kAttributes.size();
	fwrite(&usAttrCount,sizeof(usAttrCount),1,fp);

	for(ParamMap::iterator itor = m_kAttributes.begin(); itor != m_kAttributes.end(); itor++)
	{
		char	strAttrType[256];
		char	strAttrValue[256];

		strcpy_s(strAttrType,256,itor->first.c_str());
		strcpy_s(strAttrValue,256,itor->second.c_str());

		//	Attr Type
		byLen = strlen(strAttrType);
		fwrite(&byLen,sizeof(BYTE),1,fp);
		fwrite(strAttrType,1,byLen,fp);
		
		//	Attr Value
		byLen = strlen(strAttrValue);
		fwrite(&byLen,sizeof(BYTE),1,fp);
		fwrite(strAttrValue,1,byLen,fp);
	}
}

std::string const&	PgEventScriptCmd::GetAttr(std::string const &kAttrName)
{
	NiString	kString = kAttrName.c_str();
	kString.ToUpper();

	std::string	kNewAttrName((char const*)kString);

	ParamMap::iterator itor = m_kAttributes.find(kNewAttrName);
	if(itor == m_kAttributes.end())
	{
		return	NULL_STRING;
	}

	return	itor->second;
}
int	PgEventScriptCmd::GetAttrInt(std::string const &kAttrName)
{
	std::string	kValue = GetAttr(kAttrName);
	int	iValue = atoi(kValue.c_str());
	return	iValue;
}
float	PgEventScriptCmd::GetAttrFloat(std::string const &kAttrName)
{
	std::string	kValue = GetAttr(kAttrName);
	float	fValue = static_cast<float>(atof(kValue.c_str()));
	return	fValue;
}

std::string const&	PgEventScriptCmd::GetParam(std::string const &kParamName)
{
	if(!m_pkCustomParamMap)
	{
		return	NULL_STRING;
	}

	ParamMap::iterator itor = m_pkCustomParamMap->find(kParamName);
	if(itor == m_pkCustomParamMap->end())
	{
		return	NULL_STRING;
	}

	return	itor->second;
}
int	PgEventScriptCmd::GetParamInt(std::string const &kParamName)
{
	std::string	kValue = GetParam(kParamName);
	int	iValue = atoi(kValue.c_str());
	return	iValue;
}
float	PgEventScriptCmd::GetParamFloat(std::string const &kParamName)
{
	std::string	kValue = GetParam(kParamName);
	float	fValue = static_cast<float>(atof(kValue.c_str()));
	return	fValue;
}

void	PgEventScriptCmd::SetParam(std::string const &kParamName,std::string const &kParamValue)
{
	if(!m_pkCustomParamMap)
	{
		return;
	}

	ParamMap::iterator itor = m_pkCustomParamMap->find(kParamName);
	if(itor == m_pkCustomParamMap->end())
	{
		m_pkCustomParamMap->insert(std::make_pair(kParamName,kParamValue));
		return;
	}

	itor->second = kParamValue;
}
void	PgEventScriptCmd::SetParamInt(std::string const &kParamName,int iParamValue)
{
	char	strValue[256];
	sprintf_s(strValue,"%d",iParamValue);
	SetParam(kParamName,strValue);
}
void	PgEventScriptCmd::SetParamFloat(std::string const &kParamName,float fParamValue)
{
	char	strValue[256];
	sprintf_s(strValue,"%f",fParamValue);
	SetParam(kParamName,strValue);
}

