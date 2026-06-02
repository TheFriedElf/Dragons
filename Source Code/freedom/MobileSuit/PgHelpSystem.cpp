#include "stdafx.h"
#include "PgHelpSystem.H"
#include "lwHelpSystem.H"
#include "lwHelpObject.H"
#include <shlobj.h>
#include "BM/filesupport.h"

#include "PgPilotMan.h"
#include "PgNetWork.h"
#include "PgPilot.h"
#include "lohengrin/PgRealmManager.h"
#include "PgMobileSuit.h"
#include "PgOption.h"

PgHelpSystem	g_kHelpSystem;

const	char	*HELP_CONDITION_TRIGGER="Trigger";
const	char	*HELP_CONDITION_LEVEL_UP="LevelUp";
const	char	*HELP_CONDITION_OPEN_UI="OpenUI";
const	char	*HELP_CONDITION_MAP_MOVE="MapMove";
const	char	*HELP_CONDITION_PARTY="Party";
const	char	*HELP_CONDITION_REPAIR="Repair";
const	char	*HELP_CONDITION_BLACKSMITH="BlackSmith";
const	char	*HELP_CONDITION_ITEMNO="ItemNo";
const	char	*HELP_CONDITION_CLASS_ITEM="Class_Item";
const	char	*HELP_CONDITION_MONSTER_KILL_COUNTER="MonsterKillCounter";
const	char	*HELP_CONDITION_CHECK_LV="Check_Lv";
const	char	*HELP_CONDITION_CURSEITEM="CurseItemGet";
const	char	*HELP_CONDITION_ACHIEVE="Achieve";
const	char	*HELP_CONDITION_MISSIONCOMPLETE="MissionComplete";
const	char	*HELP_CONDITION_CHAR_CARD="CharCard";

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgHelpObject
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PgHelpObject::PgHelpObject(int const iType, std::string const &kConditionParamValue, int const iMaxActivatingCount,
							int const iID, int const iTitleTTID, int const iMessageTTID, int const iHelperMsgTTID,
							std::string const &kRelatedUIName, int const iCallEventID, int const iMaxLevelLimit,
							int const iExTitleTTID, int const iExMessageTTID, std::string const& PreviewImgPath,
							int const iExBtnTTID, int const iExType)
		: m_iHelpUIType(iType), m_kConditionParamValue(kConditionParamValue), m_iMaxActivatingCount(iMaxActivatingCount),
		m_iID(iID), m_iTitleTTID(iTitleTTID), m_iMessageTTID(iMessageTTID), m_iHelperMsgTTID(iHelperMsgTTID),
		m_kRelatedUIName(kRelatedUIName), m_iCallEventID(iCallEventID), m_iMaxLevelLimit(iMaxLevelLimit),
		m_iCurrentActivatingCount(0), m_bDoNotActivateAgain(false), m_iExTitleTTID(iExTitleTTID), m_iExMessageTTID(iExMessageTTID), 
		m_kPreviewImgPath(PreviewImgPath), m_iExBtnTTID(iExBtnTTID), m_iExType(iExType)
{
}
PgHelpObject::~PgHelpObject()
{
}

bool	PgHelpObject::IsCorrectCondition(std::string const &kConditionParamValueNow)
{
	if(m_iMaxActivatingCount>0)
	{
		if(m_iCurrentActivatingCount>=m_iMaxActivatingCount)
		{
			return	false;
		}
	}

	if(GetDoNotActivateAgain())
	{
		return	false;
	}

	return	(stricmp(kConditionParamValueNow.c_str(),m_kConditionParamValue.c_str()) == 0);
}
void	PgHelpObject::Activate(bool bIncreaseActivatingCount)
{
	if(bIncreaseActivatingCount)
	{
		m_iCurrentActivatingCount++;

		PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
		if( pkPC
		&&	0 != m_iMaxLevelLimit )
		{
			if( pkPC->GetAbil(AT_LEVEL) > m_iMaxLevelLimit )
			{
				return;
			}
		}
	}

	g_kHelpSystem.SetActivatedObject(this);

	if( 0 != g_kGlobalOption.GetValue(XML_ELEMENT_ETC, std::string("OFF_HELP")) )
	{
		return;
	}

	lua_tinker::call<void,lwHelpSystem,lwHelpObject>("HelpSystem_OnActivateObject",lwHelpSystem(&g_kHelpSystem),lwHelpObject(this));

	_PgOutputDebugString("PgHelpObject::Activate ID[%d]\n",m_iID);
}
void	PgHelpObject::LoadFromFile(PgHelpObject *pkHelpObject,FILE *fp)
{
	if(!fp)
	{
		return;
	}

	bool	bDoNotActivateAgain = false;
	fread(&bDoNotActivateAgain,sizeof(bool),1,fp);

	//	ActivatingCount 로딩
	unsigned	long	ulActivatingCount = 0;
	fread(&ulActivatingCount,sizeof(unsigned long),1,fp);

	if(pkHelpObject)
	{
		pkHelpObject->SetDoNotActivateAgain(bDoNotActivateAgain);
		pkHelpObject->SetActivatingCount(ulActivatingCount);
	}
}
void	PgHelpObject::SaveToFile(PgHelpObject *pkHelpObject,FILE *fp)
{
	if(!fp)
	{
		return;
	}

	bool	bDoNotActivateAgain = false;
	if(pkHelpObject)
	{
		bDoNotActivateAgain = pkHelpObject->GetDoNotActivateAgain();
	}
	fwrite(&bDoNotActivateAgain,sizeof(bool),1,fp);

	//	ActivatingCount 로딩
	unsigned	long	ulActivatingCount = 0;
	if(pkHelpObject)
	{
		ulActivatingCount = static_cast<unsigned long>(pkHelpObject->GetActivatingCount());
	}
	fwrite(&ulActivatingCount,sizeof(unsigned long),1,fp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgHelpConditionUnit
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PgHelpConditionUnit::PgHelpConditionUnit(std::string const &kConditionName)
:m_kConditionName(kConditionName)
{
}
PgHelpConditionUnit::~PgHelpConditionUnit()
{
	ReleaseAllObject();
}

bool	PgHelpConditionUnit::AddObject(PgHelpObject *pkNewHelpObject)
{
	m_kHelpObjects.push_back(pkNewHelpObject);
	return	true;
}

void	PgHelpConditionUnit::ActivateObject(std::string const &kConditionParamValueNow)
{
	int	iTotal = m_kHelpObjects.size();

	for(int i=0;i<iTotal;i++)
	{
		if(	m_kHelpObjects[i]->IsCorrectCondition(kConditionParamValueNow))
		{
			m_kHelpObjects[i]->Activate();	
		}
	}
}
int		PgHelpConditionUnit::GetHelpID(std::string const &kConditionParamValueNow)
{
	int	iTotal = m_kHelpObjects.size();

	for(int i=0;i<iTotal;i++)
	{
		if(	m_kHelpObjects[i]->IsCorrectCondition(kConditionParamValueNow))
		{
			return m_kHelpObjects[i]->GetID();
		}
	}
	return 0;
}

void	PgHelpConditionUnit::ReleaseAllObject()
{
	m_kHelpObjects.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgHelpSystem
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PgHelpSystem::PgHelpSystem()
:m_pkActivatedObject(NULL), m_kHelperGuid(BM::GUID::NullData()), m_kRealmNo(0)
{
}
PgHelpSystem::~PgHelpSystem()
{
	TerminateSystem();
}

bool HelpObjectCompair(PgHelpObject const* lhs, PgHelpObject const* rhs)
{
	if(!lhs || !rhs)	{return false;}

	std::wstring const& kThisStr = TTW(lhs->GetTitleTTID());
	std::wstring const& krhsStr = TTW(rhs->GetTitleTTID());
	return kThisStr < krhsStr;  
}

void PgHelpSystem::SortHelpObjectList()
{
	m_kHelpObjectList.sort(HelpObjectCompair);
}

void	PgHelpSystem::InitSystem()
{
	ReleaseAllObject();
	ParseXML("HelpSystem.XML", m_kHelpObjects);

	ParseXML("HelpSystemMain.XML", m_kHelpObjectList, false);
	m_kLoadedHelpGuid = BM::GUID::NullData();
	m_kHelpPage.SetPageAttribute(10,5);
	m_kHelpPage.SetMaxItem(static_cast<int>(m_kHelpObjectList.size()));
}

void	PgHelpSystem::TerminateSystem()
{
	ReleaseAllObject();
}
void	PgHelpSystem::Clear()
{
	ResetActivatingCount();
	m_kLoadedHelpGuid = BM::GUID::NullData();
}
PgHelpObject*	PgHelpSystem::GetObject(int iObjectID)
{
	int	iTotal = m_kHelpObjects.size();
	for(int i=0;i<iTotal;i++)
	{
		if(m_kHelpObjects[i]->GetID() == iObjectID)
		{
			return	m_kHelpObjects[i];
		}
	}
	return	NULL;
}

PgHelpObject*	PgHelpSystem::GetObjectFromList(int iObjectID)
{
	HelpObjectList::iterator it = m_kHelpObjectList.begin();
	while(it!=m_kHelpObjectList.end())
	{
		if((*it) && (*it)->GetID() == iObjectID)
		{
			return (*it);
		}
			 
		++it;
	}

	return	NULL;
}

bool	PgHelpSystem::ConvertGuidToSavePathName(BM::GUID const &rkGuid, std::wstring &rkOutPathName)
{
	wchar_t UserInfoPath[MAX_PATH] = {0,};
	wchar_t UserDataPath[MAX_PATH]= {0,};

	if( S_OK != SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, UserDataPath) )
	{
		return false;
	}

	std::wstring kName = std::wstring();
	std::wstring kRealmNameTemp = std::wstring();
	const wchar_t* kCharName = 0;
	const wchar_t* kRealmName = 0;
	short const iRealmNo = g_kHelpSystem.RealmNo();
	PgPilot *pkPilot = g_kPilotMan.FindPilot(rkGuid);
	if (pkPilot)
	{
		CUnit* pkUnit = pkPilot->GetUnit();
		kName = pkPilot->GetName();
		kCharName = kName.c_str();
	}

	PgRealm	Realm;
	if( S_OK == g_kRealmMgr.GetRealm(iRealmNo, Realm) )
	{
		kRealmNameTemp = Realm.Name();
		kRealmName = kRealmNameTemp.c_str();
	}

	if( (kCharName == NULL) || (kRealmName == NULL) )
	{
		return false;
	}

	std::wstring kGUID(rkGuid.str());
#ifndef EXTERNAL_RELEASE
	if( g_pkApp->IsSingleMode() )
	{
		kGUID = _T("SingleModeCharacter");
	}
#endif

	swprintf_s(UserInfoPath, MAX_PATH, L"%s\\%s\\Users\\%s_%s\\", UserDataPath, GetClientNameW(), kCharName, kRealmName);
	BM::ReserveFolder(UserInfoPath);
	rkOutPathName = UserInfoPath;
	return true;
}

void	PgHelpSystem::LoadHelpInfoFile(BM::GUID const &rkGuid)
{
	if( BM::GUID::IsNull(rkGuid) )
	{
		return;
	}

	if( BM::GUID::IsNotNull(m_kLoadedHelpGuid) )
	{
		return;
	}

	std::wstring kSaveFilePath;
	if( !ConvertGuidToSavePathName(rkGuid, kSaveFilePath) )
	{
		return;
	}

	m_kLoadedHelpGuid = rkGuid;
	ResetActivatingCount();

	FILE	*fp = _wfopen((kSaveFilePath+_T("HelpInfo.Dat")).c_str(),_T("rb"));
	if(!fp)
	{
		return;
	}

	//	파일 버전 로드
	unsigned	short	usVersion = 0;
	fread(&usVersion,sizeof(unsigned short),1,fp);

	//	갯수 로드
	unsigned	short	usTotal = 0;
	fread(&usTotal,sizeof(unsigned short),1,fp);

	//	각 오브젝트 별 로딩
	for(int i=0;i<usTotal;i++)
	{
		//	ID 로딩
		unsigned	long	ulID = 0;
		fread(&ulID,sizeof(unsigned long),1,fp);

		PgHelpObject::LoadFromFile(GetObject(ulID),fp);
	}


	fclose(fp);
}
void	PgHelpSystem::SaveHelpInfoFile(BM::GUID const &rkGuid)
{
	if( BM::GUID::IsNull(rkGuid) )
	{
		return;
	}

	if( BM::GUID::IsNull(m_kLoadedHelpGuid) )
	{
		return;
	}

	std::wstring kSaveFilePath;
	if( !ConvertGuidToSavePathName(rkGuid, kSaveFilePath) )
	{
		return;
	}

	FILE	*fp = _wfopen((kSaveFilePath+_T("HelpInfo.Dat")).c_str(),_T("wb"));
	if(!fp)
	{
		return;
	}

	//	파일 버전 저장
	unsigned	short	usVersion = 0;
	fwrite(&usVersion,sizeof(unsigned short),1,fp);

	//	갯수 저장
	unsigned	short	usTotal = static_cast<unsigned short>(m_kHelpObjects.size());
	fwrite(&usTotal,sizeof(unsigned short),1,fp);

	//	각 오브젝트 별 저장
	for(int i=0;i<usTotal;i++)
	{
		//	ID 저장
		unsigned	long	ulID = static_cast<unsigned long>(m_kHelpObjects[i]->GetID());
		fwrite(&ulID,sizeof(unsigned long),1,fp);

		PgHelpObject::SaveToFile(m_kHelpObjects[i],fp);
	}

	fclose(fp);
}

PgHelpObject const& PgHelpSystem::GetHelpObject(int iObjectID, bool bFromList)
{
	static PgHelpObject kObject;
	PgHelpObject* pkObject = bFromList ?  GetObjectFromList(iObjectID) : GetObject(iObjectID);
	if( pkObject )
	{
		return (*pkObject);
	}
	return	kObject;
}

void	PgHelpSystem::ResetActivatingCount()
{
	int	iTotal = m_kHelpObjects.size();

	for(int i=0;i<iTotal;i++)
	{
		m_kHelpObjects[i]->SetActivatingCount(0);
	}
}

void	PgHelpSystem::ActivateByCondition(std::string const &kConditionName,int iConditionValue)
{
	char	str[100];
	sprintf_s(str,100,"%d",iConditionValue);

	ActivateByCondition(kConditionName,std::string(str));

}

void	PgHelpSystem::ActivateByHelpID(int const iHelpID)
{
	PgHelpObject* pkHelp = GetObject(iHelpID);
	if( pkHelp )
	{
		pkHelp->Activate(false);
	}
}

int		PgHelpSystem::ChangeConditionNameToHelpID(std::string const &kConditionName, std::string const& kConditionValue)
{
	PgHelpConditionUnit	*pkHelpConditionUnit = GetHelpConditionUnit(kConditionName);
	if(pkHelpConditionUnit)
	{
		return pkHelpConditionUnit->GetHelpID(kConditionValue);
	}
	return	0;
}

void	PgHelpSystem::ActivateByCondition(std::string const &kConditionName,std::string const kConditionValue)
{

	PgHelpConditionUnit	*pkHelpConditionUnit = GetHelpConditionUnit(kConditionName);
	if(pkHelpConditionUnit)
	{
		pkHelpConditionUnit->ActivateObject(kConditionValue);
	}
}
void	PgHelpSystem::ActivateNextObject()
{	
	if(!m_pkActivatedObject)
	{
		return;
	}

	int	iTotal = m_kHelpObjects.size();

	PgHelpObject	*pkPrevObject = m_kHelpObjects[iTotal - 1];
	for(int i=0;i<iTotal;i++)
	{
		if(pkPrevObject == m_pkActivatedObject)
		{
			m_kHelpObjects[i]->Activate(false);
			return;
		}

		pkPrevObject = m_kHelpObjects[i];
	}
}
void	PgHelpSystem::ActivatePrevObject()
{
	if(!m_pkActivatedObject)
	{
		return;
	}
	int	iTotal = m_kHelpObjects.size();

	PgHelpObject	*pkPrevObject = m_kHelpObjects[iTotal - 1];
	for(int i=0;i<iTotal;i++)
	{
		if(m_kHelpObjects[i] == m_pkActivatedObject)
		{
			pkPrevObject->Activate(false);
			return;
		}

		pkPrevObject = m_kHelpObjects[i];
	}
}


void	PgHelpSystem::ReleaseAllObject()
{
	for(HelpConditionUnitMap::iterator itor = m_kHelpConditionUnits.begin(); itor != m_kHelpConditionUnits.end(); itor++)
	{
		PgHelpConditionUnit	*pkHelpConditionUnit = itor->second;

		SAFE_DELETE(pkHelpConditionUnit);
	}
	m_kHelpConditionUnits.clear();

	int	iTotal = m_kHelpObjects.size();
	for(int i=0;i<iTotal;i++)
	{
		SAFE_DELETE(m_kHelpObjects[i]);
	}
	m_kHelpObjects.clear();
	m_kHelpObjectList.clear();

}
PgHelpConditionUnit*	PgHelpSystem::GetHelpConditionUnit(std::string const &kConditionName)
{
	HelpConditionUnitMap::iterator itor = m_kHelpConditionUnits.find(kConditionName);
	if( itor != m_kHelpConditionUnits.end() )
	{
		PgHelpConditionUnit	*pkHelpConditionUnit = itor->second;
		return	pkHelpConditionUnit;
	}
	return	NULL;
}
PgHelpConditionUnit*	PgHelpSystem::CreateHelpConditionUnit(std::string const &kConditionName)
{

	PgHelpConditionUnit *pkHelpConditionUnit = new PgHelpConditionUnit(kConditionName);

	m_kHelpConditionUnits.insert(std::make_pair(kConditionName,pkHelpConditionUnit));

	return	pkHelpConditionUnit;
}

template <typename T>
void	PgHelpSystem::ParseXML(char const *strXMLPath, T& kArray, bool const bMakeCondition)
{
	TiXmlDocument kXmlDoc(strXMLPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(strXMLPath)))
	{
		PgError1("Parse Failed [%s]", strXMLPath);
		return;
	}

	// Find Root
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName,"HELP_OBJECT")==0)
		{

			int	iType=0, iID=0,iTitleTTID=0,iMessageTTID=0,iMaxActivatingCount=0,iHelperMsgTTID=0,iCallEventID=0,iMaxLevelLimit=0;
			int iExTitleTTID=0, iExMessageTTID=0, iExBtnTTID=0, iExType=0;
			std::string kRelatedUIName,kConditionName,kConditionValue, kPreviewImgPath;

			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					iID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "UI_Type") == 0)
				{
					iType = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Title") == 0)
				{
					iTitleTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Message") == 0)
				{
					iMessageTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "HelperMsg") == 0)
				{
					iHelperMsgTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Max_Activating_Count") == 0)
				{
					iMaxActivatingCount = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Related_UI") == 0)
				{
					kRelatedUIName = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "Condition") == 0)
				{
					kConditionName = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "Value") == 0)
				{
					kConditionValue = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "CallEventID")==0)
				{
					iCallEventID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "MaxLevelLimit")==0)
				{
					iMaxLevelLimit = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Ex_Title")==0)
				{
					iExTitleTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Ex_Message")==0)
				{
					iExMessageTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Ex_Preview_ImgPath")==0)
				{
					kPreviewImgPath = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "Ex_BtnTTID")==0)
				{
					iExBtnTTID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Ex_Type")==0)
				{
					iExType = atoi(pcAttrValue);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			PgHelpObject	*pkHelpObject = new PgHelpObject(iType, kConditionValue, iMaxActivatingCount, iID, iTitleTTID, iMessageTTID, iHelperMsgTTID,
															kRelatedUIName, iCallEventID, iMaxLevelLimit, iExTitleTTID, iExMessageTTID, kPreviewImgPath,
															iExBtnTTID, iExType);

			if(pkHelpObject && pkHelpObject->GetID())
			{
				kArray.push_back(pkHelpObject);
			}

			if(bMakeCondition)
			{
				PgHelpConditionUnit	*pkHelpConditionUnit = GetHelpConditionUnit(kConditionName);
				if(!pkHelpConditionUnit)
				{
					pkHelpConditionUnit = CreateHelpConditionUnit(kConditionName);
				}

				if(pkHelpConditionUnit)
				{
					pkHelpConditionUnit->AddObject(pkHelpObject);
				}
			}

		}

		pkElement = pkElement->NextSiblingElement();
	}
}

void PgHelpSystem::GetHelpDisplayItem(HelpObjectList& rkObjectList)
{
	rkObjectList.clear();
	HelpObjectList::const_iterator itor = m_kHelpObjectList.begin();
	int const ignoreCount = m_kHelpPage.Now() * m_kHelpPage.GetMaxItemSlot();
	int iLoopCount = 0;

	while( itor != m_kHelpObjectList.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++itor;
			continue;
		}

		rkObjectList.push_back((*itor));
		++itor;

		if( (iLoopCount - ignoreCount) == m_kHelpPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}
