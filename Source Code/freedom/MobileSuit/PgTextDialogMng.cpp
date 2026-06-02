#include "stdafx.h"
#include "ServerLib.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuestMan.h"
#include "lwGuid.h"
#include "PgTextDialogMng.h"
#include "PgQuestFullScreenDialog.h"
#include "PgPilot.h"
#include "PgPilotMan.h"

PgTextDialogMng::PgTextDialogMng()
{
}

PgTextDialogMng::~PgTextDialogMng()
{
}

void PgTextDialogMng::Reload()
{
	m_kMap.clear();
	PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_TEXT_DIALOG);
	PgXmlLoader::CreateObject(XML_ELEMENT_TEXT_DIALOG);
}

STextDialg const* PgTextDialogMng::Get(int const iDialogID) const
{
	 const_iterator find_iter = m_kMap.find(iDialogID);
	 if( m_kMap.end() == find_iter )
	 {
		 return NULL;
	 }
	 return &(*find_iter).second;
}

//ContDialog m_kMap;

bool PgTextDialogMng::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	if( !pkNode )
	{
		return false;
	}

	TiXmlElement const *pkDialogNode = pkNode->FirstChildElement("DIALOG");
	while( pkDialogNode )
	{
		ContTextDialog::mapped_type kNewDialog;

		if( !kNewDialog.Parse(pkDialogNode, PgQuestInfoVerifyUtil::SQuestInfoError(PgQuestInfoVerifyUtil::ET_ParseError, std::string("TextDialogs.xml"), 0, NULL, 0, BM::vstring()) ) )
		{
		//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] Can't Parse Dialog :%d"), __FUNCTIONW__, __LINE__, kNewDialog.iDialogID);
			return false;
		}

		switch( kNewDialog.eType )
		{
		case QDT_Normal:
		case QDT_NpcTalk:
		case QDT_Talk_Menu:
			{
				//
				kNewDialog.eType = QDT_TextDialog;
			}break;;
		case QDT_Movie:
			{
				kNewDialog.eType = QDT_Movie;
			}break;
		default:
			{
			//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] Wrong Dialog Type[ID: %d] :%d"), __FUNCTIONW__, __LINE__, kNewDialog.iDialogID, (int)kNewDialog.eType);
			}break;
		}

		pkDialogNode->Attribute("TITLE", &kNewDialog.iTitleID);

		if( kNewDialog.kDialogText.empty()
		||	1 < kNewDialog.kDialogText.size() )
		{
#ifndef EXTERNAL_RELEASE
			_PgMessageBox("Error", "Can't Dialog[ID: %d] Body Text count: %u ........ support only 1 count", kNewDialog.iDialogID, kNewDialog.kDialogText.size());
#endif
			return false;
		}

		std::pair<ContTextDialog::iterator, bool> kRet = m_kMap.insert( std::make_pair(kNewDialog.iDialogID, kNewDialog) );
		if( !kRet.second )
		{
#ifndef EXTERNAL_RELEASE
			_PgMessageBox("Error", "Can't insert TextDialog ID: %d ........ maybe duplicate ID", kNewDialog.iDialogID);
#endif
			return false;
		}

		pkDialogNode = pkDialogNode->NextSiblingElement();
	}
	return true;
}



void lwTextDialog::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	//
	def(pkState, "CallTextDialog", CallTextDialog);
}

void lwTextDialog::CallTextDialog(int const iDialogID, lwGUID kNpcGuid)
{
	STextDialg const *pkDialog = g_kTextDialogMng.Get(iDialogID);
	if( !pkDialog )
	{
#ifndef EXTERNAL_RELEASE
		_PgMessageBox("Error", "Can't Find TextDialog ID: %d", iDialogID);
#endif
		return;
	}

	SFullScreenDialogInfo kInfo;
	kInfo.eType = pkDialog->eType;
	kInfo.kGuid = kNpcGuid();

	SQuestDialogText const &rkDialogText = pkDialog->kDialogText.front();
	if( rkDialogText.iStringNameID )
	{
		std::wstring const *pkDefString = NULL;
		if( ::GetDefString(rkDialogText.iStringNameID, pkDefString) )
		{
			kInfo.kObjectName = *pkDefString;
		}
	}
	else if( rkDialogText.iTextNameID )
	{
		kInfo.kObjectName = TTW(rkDialogText.iTextNameID);
	}
	else if( rkDialogText.iMovieNo )
	{
		kInfo.iMovieNo = rkDialogText.iMovieNo;
	}
	else
	{
		PgPilot *pkNpcPilot = g_kPilotMan.FindPilot( kNpcGuid() );
		if( !pkNpcPilot )
		{
#ifndef EXTERNAL_RELEASE
			_PgMessageBox("Error", "Can't Find NPC GUID: %s ............ Can't show Dialog Name", kNpcGuid.GetString());
			return;
#endif
		}
		kInfo.kObjectName = pkNpcPilot->GetName();
	}

	kInfo.kTitleText = TTW(pkDialog->iTitleID);
	kInfo.kDialogText = TTW(rkDialogText.iTextNo);
	kInfo.kFaceID = rkDialogText.kFaceID;
	kInfo.iFaceAniID = rkDialogText.iFaceAniID;	//Only 3d Model
	kInfo.kModelID = rkDialogText.kModelID;
	kInfo.kStitchImageID = rkDialogText.kStitchImageID;

	g_kQuestMan.CallFullScreenTalk(kInfo, NULL, pkDialog);
}