#include "StdAfx.h"
#include "./NiApplication/NiDX9Select.h"

#include "PgXmlLoader.h"
#include "PgIXmlObject.h"
// 생성 가능한 오브젝트들
#include "PgWorld.h"
#include "PgUIScene.h"
#include "PgActor.h"
#include "PgActorNpc.h"
#include "PgActorMonster.h"
#include "PgActorPet.h"
#include "PgLocalManager.h"
#include "PgRemoteManager.h"
#include "PgAction.h"
#include "PgTrigger.h"
#include "PgShineStone.h"
#include "PgWorldMapUI.h"
#include "PgPilot.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgQuest.h"
#include "PgQuestMan.h"
#include "PgWorldActionMan.h"
#include "PgPuppet.h"
#include "PgFurniture.h"
#include "PgProjectile.h"
#include "PgSkillTree.h"
#include "BM/PgFilterString.h"
#include "PgCommandMgr.h"
#include "PgChatMgrClient.h"
#include "PgOption.h"
#include "PgActionPool.h"
#include "FreedomPool.h"
#include "PgStatusEffect.H"
#include "PgResourceIcon.h"
#include "PgEntityZone.h"
#include "PgErrorCodeTable.h"
#include "PgNiFile.h"
#include "PgObject.H"
#include "PgStitchImageMng.h"
#include "PgTextDialogMng.h"
#include "lwCashItem.h"
#include "PgHouse.h"
#include "PgLegendItemMakeHelpMng.h"
#include "PgSuperGroundUI.h"
#include "PgRagnarokGroundUI.h"
#include "PgTextBalloon.h"
#include "PgActorSubPlayer.h"
#include "PgActionXmlData.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace XmlLoaderUtil
{
	bool PrepareXmlDocumentAfterWork(int const iType, TiXmlDocument* pkDocument)
	{
		if( !pkDocument )
		{
			return false;
		}

		bool bRet = false;
		switch( iType )
		{
		case PXDAW_ParseQuest:
			{
				bRet = g_kQuestMan.ParseXml( pkDocument->FirstChildElement() );
			}break;
		default:
			{
			}break;
		}

		return bRet;
	}
};


extern bool g_bFindFirstLocal;

#define USE_XML_WORKER_THREAD

PgXmlLoader *PgXmlLoader::ms_pkSelf = 0;
Loki::Mutex PgXmlLoader::m_kCreateLock;

PgXmlLoader::PgXmlLoader(void)
{
}

PgXmlLoader::~PgXmlLoader(void)
{
	m_kCacheLock.Lock();
	for(XmlCacheContainer::iterator itr = m_kXmlCacheContainer.begin();
		itr != m_kXmlCacheContainer.end();
		itr++)
	{
		SAFE_DELETE(itr->second);
	}
	//m_kXmlCacheContainer.clear();
	m_kCacheLock.Unlock();
}

void PgXmlLoader::DestroySingleton()
{
	SAFE_DELETE_NI(ms_pkSelf);
}

bool PgXmlLoader::Initialize()
{
	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
	if(!LoadPath("path.xml"))
	{
		return false;
	}

	if(g_bUseImportActionData)
	{//#XML_TO_CODE
		g_kActionXmlData.Init();
	}

	return true;
}

bool PgXmlLoader::LoadPath(char const *pcPathXml)
{
	TiXmlDocument kXmlDoc;
	if(!LoadFile(kXmlDoc, UNI(pcPathXml)))
	{
		PG_ASSERT_LOG(!"failed to loading path.xml");
		return false;
	}

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = kXmlDoc.FirstChild();
	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", pcPathXml);
		return false;
	}

	TiXmlElement const* pkElement = pkRootNode->FirstChildElement();
	while( pkElement )
	{
		ParserXml_Path( pkElement, m_kXmlPathContainer, m_kXmlTypeContainer );
		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

bool PgXmlLoader::ParserXml_Path(TiXmlElement const* pkElement, XmlPathContainer & rkXmlPathContainer, XmlTypeContainer & rkXmlTypeContainer)
{
	if(pkElement)
	{
		if( ::strcmp(pkElement->Value(), "LOCAL") == 0 )
		{
			bool const bNationCode = PgStringUtil::SafeAtob(pkElement->Attribute("NATION_CODE"), false);
			PgXmlLocalUtil::RESULT_NODE kResultNode = PgXmlLocalUtil::FindInLocalNode(g_kLocal, pkElement, "LoadPath", bNationCode);
			if(NULL == kResultNode.first)
			{
				kResultNode.first = kResultNode.second;
				kResultNode.second = NULL;
			}

			XmlPathContainer kContFindPath;
			XmlTypeContainer kContFindType;
			XmlPathContainer kContDefaultPath;
			XmlTypeContainer kContDefaultType;

			if( TiXmlNode const* pkFindLocalNode = kResultNode.first )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				while( pkResultNode )
				{
					ParserXml_Path(pkResultNode, kContFindPath, kContFindType);
					pkResultNode = pkResultNode->NextSiblingElement();
				}
			}

			if( TiXmlNode const* pkFindLocalNode = kResultNode.second )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				while( pkResultNode )
				{
					ParserXml_Path(pkResultNode, kContDefaultPath, kContDefaultType);
					pkResultNode = pkResultNode->NextSiblingElement();
				}
			}

			XmlPathContainer::const_iterator path_it = kContFindPath.begin();
			while(path_it != kContFindPath.end())
			{
				if(IsFile(path_it->second.c_str()))
				{
					rkXmlPathContainer.insert( std::make_pair(path_it->first, path_it->second) );

					XmlPathContainer::const_iterator type_it = kContFindType.find((*path_it).first);
					if(type_it != kContFindType.end())
					{
						rkXmlTypeContainer.insert( std::make_pair(type_it->first, type_it->second) );
					}
				}
				else
				{
					XmlPathContainer::const_iterator path2_it = kContDefaultPath.find((*path_it).first);
					if(path2_it != kContDefaultPath.end())
					{
						rkXmlPathContainer.insert( std::make_pair(path2_it->first, path2_it->second) );
					}
					
					XmlPathContainer::const_iterator type_it = kContDefaultType.find((*path_it).first);
					if(type_it != kContDefaultType.end())
					{
						rkXmlTypeContainer.insert( std::make_pair(type_it->first, type_it->second) );
					}
				}
				++path_it;
			}
		}
		else
		{
			if(g_bUseImportActionData)
			{//#XML_TO_CODE
				if( ::strcmp(pkElement->Value(), "ACTION") == 0 )
				{// Action이면 넘어가고
					return false;
				}
			}

			char const *pcID = 0;
			char const *pcType = 0;

			// GUID를 알아낸다.
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();
				
				if(strcmp(pcAttrName, "ID") == 0)
				{
					pcID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "TYPE") == 0)
				{
					pcType = pcAttrValue;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			char const* pcPath = pkElement->GetText();

			if (pcID != NULL && pcPath != NULL && strlen(pcID) > 0 && strlen(pcPath) > 0)
			{
				// 경로 컨테이너에 추가한다.
				std::string strPath = pcPath;
				std::string strID = pcID;
				std::string strType;
				UPR(strPath);
				UPR(strID);
				if (!rkXmlPathContainer.insert(std::make_pair(strID, strPath)).second)
				{
					//PG_ASSERT_LOG(!"xml path adding failed");
					NILOG(PGLOG_ERROR, "[PgXmlLoader] LoadPath(%s,%s) failed\n", pcID, pcPath);
				}

				if(!pcType)
				{
					pcType = pkElement->Value();
				}
				strType = pcType;
				UPR(strType);

				rkXmlTypeContainer.insert(std::make_pair(strID, strType));
			}
		}
	}

	return true;
}

void PgXmlLoader::LoadAllDocumentToCache()
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	XmlPathContainer::iterator itr = pkSelf->m_kXmlPathContainer.begin();

	unsigned int iCount = 0;
	unsigned int iPrepareCount = 0;
	float fTime = NiGetCurrentTimeInSec();
	float fPrepareTime = 0.0f;
	while (itr != pkSelf->m_kXmlPathContainer.end())
	{
#ifdef USE_XML_WORKER_THREAD
		WorkData kWorkData;
		kWorkData.strWorkData = itr->second;
		kWorkData.strWorkData2 = itr->first;

		g_kLoadWorkerThread.DoLoadResource(pkSelf, kWorkData);
#else
		TiXmlDocument* pkDoc = GetXmlDocumentInCacheByPath(itr->second.c_str());
		PG_ASSERT_LOG(pkDoc);

		if (pkDoc)
		{
			float fTemp = NiGetCurrentTimeInSec();
			if (PrepareObject(itr->first.c_str()))
			{
				iPrepareCount++;
				fPrepareTime += (NiGetCurrentTimeInSec() - fTemp);
			}
		}
#endif
		++itr;
		iCount++;
	}
	fTime = NiGetCurrentTimeInSec() - fTime;
	NILOG(PGLOG_STAT, "[PgXmlLoader] LoadAllDocumentToCache %f time %d count, prepare %f, %d \n", fTime, iCount, fPrepareTime, iPrepareCount);
}

PgXmlLoader *PgXmlLoader::Get()
{
	BM::CAutoMutex kLock(m_kCreateLock);

	if(!ms_pkSelf)
	{
		ms_pkSelf = NiNew PgXmlLoader();
		
		if(!ms_pkSelf)
		{
			PG_ASSERT_LOG(!"PgXmlLoader instancing failed");
			return 0;
		}

		if(!ms_pkSelf->Initialize())
		{
			PG_ASSERT_LOG(!"PgXmlLoader initialing failed");
			SAFE_DELETE_NI(ms_pkSelf);
			return 0;
		}
	}

	return ms_pkSelf;
}
//BM::TObjectPool< PgProjectile, CreateArrayUsingNiNew > g_kProjectilePool(50);

PgIXmlObject *PgXmlLoader::CreateInstance(char const *pcClassName, char const *pcObjectType)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	// TODO : 해싱 및 캐싱 적용으로 보다 빠르게
	if(strcmp(pcClassName, "WORLD") == 0)
	{
		return new PgWorld;//g_kWorldPool.New();
	}
	else if(strcmp(pcClassName, "UI") == 0)
	{
		return &g_kUIScene;//UI는 싱글
	}
	else if(strcmp(pcClassName, "ACTOR") == 0)
	{
		if(pcObjectType)
		{
			if (_strnicmp(pcObjectType, OBJ_TYPE_ACTOR, strlen(OBJ_TYPE_ACTOR)) == 0)
			{
				return NiNew PgActor;
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_NPC, strlen(OBJ_TYPE_NPC)) == 0)
			{
				return NiNew PgActorNpc;
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_MONSTER, strlen(OBJ_TYPE_MONSTER)) == 0)
			{
				return NiNew PgActorMonster;
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_PET, strlen(OBJ_TYPE_PET)) == 0)
			{
				return NiNew PgActorPet;
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_OBJECT, strlen(OBJ_TYPE_OBJECT)) == 0)
			{
				return NiNew PgObject();
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_MYHOME, strlen(OBJ_TYPE_MYHOME)) == 0)
			{
				return NiNew PgHouse;	//일단임시
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_SUMMONED, strlen(OBJ_TYPE_SUMMONED)) == 0)
			{
				return NiNew PgActorMonster;	//일단임시
			}
			else if (_strnicmp(pcObjectType, OBJ_TYPE_SUB_PLAYER, strlen(OBJ_TYPE_SUB_PLAYER)) == 0)
			{
				return NiNew PgActorSubPlayer;
			}
			else
			{
				NILOG(PGLOG_ERROR, "[PgXmlLoader] Can't find correct Type of actor(%s)\n", pcObjectType);
			}
		}
		else
		{
			return NiNew PgActor;
		}	
	}
	else if(strcmp(pcClassName, "INPUT") == 0)
	{
		return new PgLocalManager;
	}
	else if(strcmp(pcClassName, "REMOTEINPUT") == 0)
	{
		return new PgRemoteManager;
	}
	else if(strcmp(pcClassName, "WORLDMAP") == 0)
	{
		return &g_kWorldMapUI;
	}
	else if(strcmp(pcClassName, "RESOURCE_ICON") == 0)
	{
		return &g_kResourceIcon;
	}
	else if(strcmp(pcClassName, "ACTION") == 0)
	{
		return g_kActionPool.CreateAction(NULL, false);
	}
	else if(strcmp(pcClassName,"STATUS_EFFECT") == 0)
	{
		return	new PgStatusEffect();
	}
	else if(strcmp(pcClassName,"ENTITY") == 0)
	{
		return NiNew PgEntityZone;
	}
	else if(strcmp(pcClassName,XML_ELEMENT_TEXT_TABLE) == 0)
	{
		if( !g_pkTT )
		{
			return new PgTextTable;//생성
		}
		return g_pkTT;//
	}
	else if(strcmp(pcClassName,XML_ELEMENT_QUEST_TEXT_TABLE) == 0)
	{
		return g_pkTT;
	}
	else if(strcmp(pcClassName,XML_ELEMENT_MAP_TEXT_TABLE) == 0)
	{
		return g_pkTT;
	}
	else if(strcmp(pcClassName,XML_ELEMENT_ERROR_CODE_TABLE) == 0)
	{
		return	new PgErrorCodeTable;
	}
	else if(strcmp(pcClassName, "TRIGGER") == 0)
	{
		return NiNew PgTrigger;
	}
	else if(strcmp(pcClassName, "STONE") == 0)
	{
		return NiNew PgShineStone;
	}
	else if(strcmp(pcClassName,"WORLD_ACTION_SETTING") == 0)
	{
		return new PgWorldActionMan;
	}
	else if(strcmp(pcClassName,"PROJECTILE")==0)
	{
		return NiNew PgProjectile; //g_kProjectilePool.New();
	}
	//else if(strcmp(pcClassName, "MOOD") == 0)
	//{
	//	return NiNew PgMoodMan;
	//}
	//else if(strcmp(pcClassName, "SEASON") == 0)
	//{
	//	return NiNew PgSeasonMan;
	//}
	else if(strcmp(pcClassName, "PILOT") == 0)
	{
		return new PgPilot;//g_kPilotPool.New();
	}
	else if(strcmp(pcClassName, "ITEM") == 0)
	{
		return NiNew PgItemEx;
	}
	else if(strcmp(pcClassName, "DROPBOX") == 0)
	{
		return NiNew PgDropBox;
	}
	else if(strcmp(pcClassName, "PUPPET") == 0)
	{
		return NiNew PgPuppet;
	}
	else if(strcmp(pcClassName, "FURNITURE") == 0)
	{
		return NiNew PgFurniture;
	}
	else if(strcmp(pcClassName, "BAD_WORD") == 0)
	{
		return &g_kClientFS;
	}
	else if(strcmp(pcClassName, "IGNORE_WORD") == 0)
	{
		return &g_kClientFS;
	}
	else if(strcmp(pcClassName, "GOOD_WORD") == 0)
	{
		return &g_kClientFS;
	}
	else if(strcmp(pcClassName, "BLOCK_NICK_NAME") == 0)
	{
		return &g_kClientFS;
	}
	else if(strcmp(pcClassName, "LINE_COMMAND") == 0)
	{
		return &g_kPgEmotiMgr;
	}
	else if(strcmp(pcClassName, "CHAT_COMMAND") == 0)
	{
		return &g_kChatMgrClient;
	}
	else if(strcmp(pcClassName, "CONFIG") == 0)
	{
		return &g_kGlobalOption;
	}
	else if(strcmp(pcClassName, "KEYSET_TABLE") == 0)
	{
		return &g_kGlobalOption;
	}
	else if(strcmp(pcClassName, "QUEST") == 0)
	{
		return &g_kQuestMan;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_BALLOON_FORM) == 0)
	{
		return &g_kBalloonImageMng;
	}
	else if(strcmp(pcClassName, "REALM_QUEST_EVENT") == 0)
	{
		return &g_kQuestMan;
	}
	else if(strcmp(pcClassName, "IMPORTANCE_QUEST") == 0)
	{
		return &g_kQuestMan;
	}
	else if(strcmp(pcClassName, "CUSTOM_QUEST_MARKER") == 0)
	{
		return &g_kQuestMan;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_STITCH_IMAGE) == 0)
	{
		return &g_kStitchImageMng;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_LEGEND_ITEM_MAKE_HELP) == 0)
	{
		return &g_kLegendItemMakeHelpMng;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_SUPER_GROUND_ENTER_SETTING) == 0)
	{
		return &g_kSuperGroundEnterUIMgr;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING) == 0)
	{
		return &g_kRagnarokGroundEnterUIMgr;
	}
	else if(strcmp(pcClassName, XML_ELEMENT_TEXT_DIALOG) == 0)
	{
		return &g_kTextDialogMng;
	}

	PG_ASSERT_LOG(!"xml object instancing failed");
	return 0;
}

bool PgXmlLoader::PrepareObject(char const *pcID)
{
	if (pcID == NULL)
		return false;

	TiXmlDocument* pkXmlDoc = NULL;
	pkXmlDoc = GetXmlDocumentByID(pcID);

	if (pkXmlDoc == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader::PrepareObject] Get %s xml document failed!\n", pcID);
		return false;
	}

	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	if (pkSelf == NULL)
		return NULL;

	std::string strID = pcID;
	UPR(strID);

	XmlTypeContainer::iterator typeIter = pkSelf->m_kXmlTypeContainer.find(strID);
	if (typeIter == pkSelf->m_kXmlTypeContainer.end())
	{
		// 이럴순 없지.
		NILOG(PGLOG_ERROR, "[PgXmlLoader::PrepareObject] Get type of %s xml failed!\n", pcID);
		return false;
	}

	XmlPathContainer::iterator pathiter = pkSelf->m_kXmlPathContainer.find(strID);
	if(pathiter == pkSelf->m_kXmlPathContainer.end())
	{
		// 이럴순 없지.
		NILOG(PGLOG_ERROR, "[PgXmlLoader::PrepareObject] Get type of %s xml failed!\n", pcID);
		return false;
	}

	if (strcmp(typeIter->second.c_str(), "ACTOR") == 0)
	{
		//char const* kfmPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "KFMPATH");
		//if (kfmPath)
		//{
		//	//g_kAMPool.LoadActorManager(kfmPath, PgIXmlObject::ID_PC, true);
		//}
		char const* pkActorName = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "ACTOR", "NAME");
		char const* pkIconPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "ICON", "ID");
		if (pkActorName && pkIconPath)
		{
			return g_kResourceIcon.AddIconID(UNI(pkActorName), UNI(pathiter->second), UNI(pkIconPath));
		}
	}
	/*else if (strcmp(typeIter->second.c_str(), "WORLD") == 0)
	{
		TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
		while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
		{
			pkRootNode = pkRootNode->NextSibling();
		}
		if (pkRootNode)
		{
			return g_kWorldMapUI.ParseXml_WorldNpc(pkRootNode, pkXmlDoc->Value());
		}
	}*/
	//else if (strcmp(typeIter->second.c_str(), "PROJECTILE") == 0)
	//{
	//	char const* texPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "TEX_PATH");
	//	char const* nifPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "NIF_PATH");
	//	if (nifPath)
	//	{
	//		bool bPathChanged = false;
	//		if (texPath && strlen(texPath) > 0)
	//		{
	//			NiImageConverter::SetPlatformSpecificSubdirectory(texPath, TEMPORARY_PLATFORM_SUB_DIR_SLOT);
	//			bPathChanged = true;
	//		}
	//		g_kNifMan.GetNif(nifPath);
	//		
	//		if (bPathChanged)
	//		{
	//			NiImageConverter::SetPlatformSpecificSubdirectory(NULL, TEMPORARY_PLATFORM_SUB_DIR_SLOT);
	//		}
	//	}

	//	char const* trailTexPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "TRAIL", "TRAIL_TEXTURE");
	//	if (trailTexPath)
	//	{
	//		g_kNifMan.GetTexture(trailTexPath);
	//	}
	//}
	//else if (strcmp(typeIter->second.c_str(), "DROPBOX") == 0)
	//{
	//	char const* kfmPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "KFMPATH");
	//	if (kfmPath)
	//	{
	//		g_kAMPool.LoadActorManager(kfmPath, PgIXmlObject::ID_DROPBOX, true);
	//	}
	//}
	else
	{

	}

	return false;
}

bool PgXmlLoader::PrepareXmlDocument(char const *pcPath, int  const iAfterWork, bool const bUseThreadLoading)
{
	PG_ASSERT_LOG(pcPath);
	if (pcPath == NULL)
	{
		return false;
	}

	if (strcmp(pcPath, ".") == 0)
	{
		return false;
	}

	if(0 != ::_tcsicmp(_T(".xml"), PathFindExtension(UNI(pcPath))))
	{
		return false;
	}

#ifdef USE_XML_WORKER_THREAD
	if( bUseThreadLoading )
	{
		WorkData kWorkData;
		kWorkData.strWorkData = pcPath;
		kWorkData.iWorkData = iAfterWork;

		g_kLoadWorkerThread.DoLoadResource(PgXmlLoader::Get(), kWorkData);
	}
	else
#endif
	{
		TiXmlDocument* pkDoc = GetXmlDocumentInCacheByPath(pcPath);
		if( pkDoc == NULL )
		{
			return false;
		}
		XmlLoaderUtil::PrepareXmlDocumentAfterWork(iAfterWork, pkDoc);
	}


	return true;
}

PgIXmlObject *PgXmlLoader::CreateObject(char const *pcID, void *pArg, PgIXmlObject *pkObject, char const* pcObjectType, int const iDataType)
{
	PgXmlLoader *pkSelf = NULL;
{
	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

	pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	if (pkSelf == NULL)
	{
		return NULL;
	}
}
	PG_ASSERT_LOG(pcID);
	if (pcID == NULL)
	{
		return NULL;
	}

	if (strlen(pcID) == 0)
	{
		return NULL;
	}

	std::string strID = pcID;

	if(strID == "0")
	{
		return NULL;
	}
	UPR(strID);

	std::string kXmlPath;
	if( g_bUseImportActionData
		&& 1 == iDataType
		)//#XML_TO_CODE
	{// Action은 별도의 컨테이너에서 가지고 오는데 
		if( false == g_kActionXmlData.GetXmlPath(strID, kXmlPath) )
		{// 등록되지 않은 ID이면 실패
			return NULL;
		}
	}
	else
	{
		XmlPathContainer::iterator itr = pkSelf->m_kXmlPathContainer.find(strID);
		{
			PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

			if(itr == pkSelf->m_kXmlPathContainer.end())
			{
				PgError1("[PgXmlLoader] CreateObject(%s) failed, can't find xml path\n", pcID);
				//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[PgXmlLoader] CreateObject(%s) failed, can't find xml path\n"), UNI(pcID));
				//	::MessageBox(NULL,UNI(pcID),UNI("Cannot Find the Actor in Path.XML"),MB_OK);
				return 0;
			}
		}
		kXmlPath = itr->second;
	}

	
	TiXmlDocument *pkXmlDoc = NULL;
//**********************************************************************************************************************************************************************
{
	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
	pkSelf->m_kCacheLock.Lock();
	XmlCacheContainer::iterator itrCache = pkSelf->m_kXmlCacheContainer.find(kXmlPath);	

	if(itrCache != pkSelf->m_kXmlCacheContainer.end())
	{
		pkXmlDoc = itrCache->second;
	}
	else 
	{
		pkSelf->m_kCacheLock.Unlock();
		pkXmlDoc = new TiXmlDocument;

		switch(iDataType)
		{
		case 1:
			{
				if(g_bUseImportActionData)
				{//#XML_TO_CODE
					std::string kData;
					if( !g_kActionXmlData.GetXmlData_ByPath(kXmlPath, kData) 
						|| kData.empty() 
						)
					{
						PgError2("[PgXmlLoader] CreateObject(%s) failed, %s CODE xml parse failed\n", pcID, kXmlPath.c_str());
						delete pkXmlDoc;
						return 0;
					}
					pkXmlDoc->Parse( &kData.at(0) );
					break;
				}
			}// break; 의도적 제거
		default:
			{
				if(!LoadFile(*pkXmlDoc, UNI(kXmlPath)))
				{
					PgError2("[PgXmlLoader] CreateObject(%s) failed, %s xml parse failed\n", pcID, kXmlPath.c_str());
					//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[PgXmlLoader] CreateObject(%s) failed, %s xml parse failed"), UNI(pcID), kXmlPath.c_str());
					delete pkXmlDoc;
					return 0;
				}
			}break;
		}
		
		if(pkXmlDoc->Error())
		{
			NILOG(PGLOG_ERROR, "[PgXmlLoader] failed create From Buffer %s(%s)(%d, %d, %d, %s)", pcID, kXmlPath.c_str(), pkXmlDoc->ErrorRow(), pkXmlDoc->ErrorCol(), pkXmlDoc->ErrorId(), pkXmlDoc->ErrorDesc());
			delete pkXmlDoc;
			return 0;
		}

		pkSelf->m_kCacheLock.Lock();
		std::pair<XmlCacheContainer::iterator, bool> ret = pkSelf->m_kXmlCacheContainer.insert(std::make_pair(kXmlPath, pkXmlDoc));
		if (ret.second == false)
		{
			NILOG(PGLOG_WARNING, "[PgXmlLoader] %s xml inserted already\n", kXmlPath.c_str());
			delete pkXmlDoc;
			pkXmlDoc = ret.first->second;
		}
	}
	pkSelf->m_kCacheLock.Unlock();

	if (pkXmlDoc == NULL)
		return NULL;
}
	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
	bool bUTF8 = false;
{
	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		int const iType = pkRootNode->Type();
		if (iType == TiXmlNode::DECLARATION)
		{				
			TiXmlDeclaration  const* pDecl = dynamic_cast<TiXmlDeclaration const*>(pkRootNode);
			
			if (NULL != pDecl)
			{
				std::string encoding = pDecl->Encoding();
				UPR(encoding);

				if (encoding.compare(std::string("UTF-8")) == 0 || encoding.compare(std::string("UTF8")) == 0)
				{
					bUTF8 = true;
				}
			}
		}
		pkRootNode = pkRootNode->NextSibling();
	}
}
	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", kXmlPath.c_str());
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("No [%s] Top Class Node"));
		return 0;
	}
{
	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
	// 최상위 노드를 바탕으로 객체를 생성한다.
	pkObject = pkSelf->CreateObject(pkRootNode, pArg, pkObject, pcObjectType, bUTF8);
	if(!pkObject)
	{
		PgError1("creating %s PgXmlObject failed", pcID);
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("creating PgXmlObject failed"));
		return 0;
	}
}
	// 객체의 알맞은 ID를 부여한다.
	pkObject->SetID(pcID);

	return pkObject;
}
void PgXmlLoader::CreateObjectByType(char const *pcType)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	CONT_XML_ID kList;
	GetXmlIDByType(pkSelf, kList, pcType);

	CONT_XML_ID::const_iterator loop_iter = kList.begin();
	while( kList.end() != loop_iter )
	{
		CreateObject((*loop_iter).c_str());
		++loop_iter;
	}
}
PgIXmlObject *PgXmlLoader::CreateObjectFromBuffer(char const *pcPath, char const *pcBuffer, void *pArg, PgIXmlObject *pkObject)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	
	TiXmlDocument *pkXmlDoc = NULL;

	std::string strPath = pcPath;
	UPR(strPath);

	pkSelf->m_kCacheLock.Lock();
	XmlCacheContainer::iterator itrCache = pkSelf->m_kXmlCacheContainer.find(strPath);

	if(itrCache != pkSelf->m_kXmlCacheContainer.end())
	{
		pkXmlDoc = itrCache->second;
	}
	else
	{
		pkSelf->m_kCacheLock.Unlock();
		pkXmlDoc = new TiXmlDocument(strPath.c_str());
		PG_ASSERT_LOG(pkXmlDoc);
		if (pkXmlDoc == NULL)
			return NULL;

		pkXmlDoc->Parse(pcBuffer);
		if(pkXmlDoc->Error())
		{
			NILOG(PGLOG_ERROR, "[PgXmlLoader] failed create From Buffer %s(%d, %d, %d, %s)", pcPath, pkXmlDoc->ErrorRow(), pkXmlDoc->ErrorCol(), pkXmlDoc->ErrorId(), pkXmlDoc->ErrorDesc());
			delete pkXmlDoc;
			return 0;
		}
		
		pkSelf->m_kCacheLock.Lock();
		std::pair<XmlCacheContainer::iterator, bool> ret = pkSelf->m_kXmlCacheContainer.insert(std::make_pair(strPath, pkXmlDoc));

		if (ret.second == false)
		{
			NILOG(PGLOG_WARNING, "[PgXmlLoader] %s xml inserted already\n", strPath.c_str());
			delete pkXmlDoc;
			pkXmlDoc = ret.first->second;
		}
	}
	pkSelf->m_kCacheLock.Unlock();

	if (pkXmlDoc == NULL)
		return NULL;

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
	bool bUTF8 = false;

	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		int const iType = pkRootNode->Type();
		if (iType == TiXmlNode::DECLARATION)
		{				
			TiXmlDeclaration  const* pDecl = dynamic_cast<TiXmlDeclaration const*>(pkRootNode);
			
			if (NULL != pDecl)
			{
				std::string encoding = pDecl->Encoding();
				UPR(encoding);

				if (encoding.compare(std::string("UTF-8")) == 0 || encoding.compare(std::string("UTF8")) == 0)
				{
					bUTF8 = true;
				}
			}
		}
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", pcPath);
		return 0;
	}

	// 최상위 노드를 바탕으로 객체를 생성한다.
	pkObject = pkSelf->CreateObject(pkRootNode, pArg, pkObject, NULL, bUTF8);
	if(!pkObject)
	{
		PG_ASSERT_LOG(!"creating PgXmlObject from buffer failed");
		return 0;
	}
	
	/*
	// 객체의 알맞은 GUID를 부여한다.
	pkObject->SetIDSetGuid(BM::GUID());
	*/
	return pkObject;
}

TiXmlDocument *PgXmlLoader::GetXmlDocumentByID(char const *pcID)
{
	if (pcID == NULL)
		return NULL;

	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	if (pkSelf == NULL)
		return NULL;

	std::string strID = pcID;
	UPR(strID);

	XmlPathContainer::iterator itr = pkSelf->m_kXmlPathContainer.find(strID);
	if(itr == pkSelf->m_kXmlPathContainer.end())
	{
		NILOG(PGLOG_ERROR,"[PgXmlLoader] GetXmlDocumentById Can't find %s ID\n", pcID);
		return 0;
	}

	return GetXmlDocumentInCacheByPath(itr->second.c_str(), pcID);
}


TiXmlDocument *PgXmlLoader::GetXmlDocumentInCacheByPath(char const *pcPath, char const* pcID)
{
	if (pcPath == NULL)
		return NULL;

	if (strlen(pcPath) == 1 && pcPath[0] == '.')
		return NULL;

	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	if (pkSelf == NULL)
		return NULL;

	TiXmlDocument *pkXmlDoc = 0;

	std::string strPath = pcPath;
	UPR(strPath);

	pkSelf->m_kCacheLock.Lock();
	XmlCacheContainer::iterator itrCache = pkSelf->m_kXmlCacheContainer.find(strPath);
	if(itrCache != pkSelf->m_kXmlCacheContainer.end())
	{
		pkSelf->m_kCacheLock.Unlock();
		pkXmlDoc = itrCache->second;

		if(pkXmlDoc && pkXmlDoc->Error())
		{
			NILOG(PGLOG_ERROR, "[PgXmlLoader] GetXmlDocumentInCacheByPath %s(%d, %d, %d, %s)", pcPath, pkXmlDoc->ErrorRow(), pkXmlDoc->ErrorCol(), pkXmlDoc->ErrorId(), pkXmlDoc->ErrorDesc());
		}

		return pkXmlDoc;
	}
	pkSelf->m_kCacheLock.Unlock();

	pkXmlDoc = new TiXmlDocument(strPath.c_str());
	if(!pkXmlDoc)
	{
		return 0;
	}

	if(!LoadFile(*pkXmlDoc, UNI(pcPath)))
	{
		NILOG(PGLOG_ERROR,"[PgXmlLoader] GetXmlDocumentInCacheByPath Load Failed '%s'\n", pcPath);
		delete pkXmlDoc;
		return 0;
	}

	if(pkXmlDoc->Error())
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader] GetXmlDocumentInCache %s(%d, %d, %d, %s)", pcPath, pkXmlDoc->ErrorRow(), pkXmlDoc->ErrorCol(), pkXmlDoc->ErrorId(), pkXmlDoc->ErrorDesc());
		delete pkXmlDoc;
		return 0;
	}

	pkSelf->m_kCacheLock.Lock();	// for Insert
	std::pair<XmlCacheContainer::iterator, bool> ret = pkSelf->m_kXmlCacheContainer.insert(std::make_pair(strPath, pkXmlDoc));
	pkSelf->m_kCacheLock.Unlock();

	if (ret.second == false)
	{
		NILOG(PGLOG_WARNING, "[PgXmlLoader] %s xml inserted already\n", strPath.c_str());
		delete pkXmlDoc;
		pkXmlDoc = ret.first->second;
	}
	else
	{
		if (pcID != NULL && pcID[0] != '\0')
			PrepareObject(pcID);
	}

	return pkXmlDoc;
}

void	PgXmlLoader::ReleaseXmlDocumentInCacheByPath(char const *pcPath)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	std::string strPath = pcPath;
	UPR(strPath);

	pkSelf->m_kCacheLock.Lock();
	XmlCacheContainer::iterator itrCache = pkSelf->m_kXmlCacheContainer.find(strPath);
	if(itrCache != pkSelf->m_kXmlCacheContainer.end())
	{
		TiXmlDocument *pkXmlDoc;
		pkXmlDoc = itrCache->second;
		pkSelf->m_kXmlCacheContainer.erase(itrCache);

		SAFE_DELETE(pkXmlDoc);
	}
	pkSelf->m_kCacheLock.Unlock();
}

void	PgXmlLoader::ReleaseXmlDocumentInCacheByID(char const *pcID)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	std::string strID = pcID;
	UPR(strID);

	XmlPathContainer::iterator itr = pkSelf->m_kXmlPathContainer.find(strID);
	if(itr == pkSelf->m_kXmlPathContainer.end())
	{
		return;
	}

	char const* name = itr->second.c_str();

	pkSelf->m_kCacheLock.Lock();
	XmlCacheContainer::iterator itrCache = pkSelf->m_kXmlCacheContainer.find(name);
	if(itrCache != pkSelf->m_kXmlCacheContainer.end())
	{
		TiXmlDocument *pkXmlDoc;
		pkXmlDoc = itrCache->second;
		pkSelf->m_kXmlCacheContainer.erase(itrCache);

		SAFE_DELETE(pkXmlDoc);
	}
	pkSelf->m_kCacheLock.Unlock();
}
void PgXmlLoader::ReleaseXmlDocumentInCacheByType(char const *pcType)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	CONT_XML_ID kList;
	GetXmlIDByType(pkSelf, kList, pcType);

	CONT_XML_ID::const_iterator iter = kList.begin();
	while( kList.end() != iter )
	{
		ReleaseXmlDocumentInCacheByID((*iter).c_str());
		++iter;
	}
}

PgIXmlObject *PgXmlLoader::CreateObjectFromFile(char const *pcPath, void *pArg, PgIXmlObject *pkObject)
{
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	
	TiXmlDocument *pkXmlDoc = GetXmlDocumentInCacheByPath(pcPath);
	if (pkXmlDoc == NULL)
	{
		NILOG(PGLOG_ERROR,"[PgXmlLoader] CreateObjectFromFile: Load Failed '%s'\n", pcPath);
		return NULL;
	}

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
	bool bUTF8 = false;

	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		int const iType = pkRootNode->Type();
		if (iType == TiXmlNode::DECLARATION)
		{				
			TiXmlDeclaration  const* pDecl = dynamic_cast<TiXmlDeclaration const*>(pkRootNode);
			
			if (NULL != pDecl)
			{
				std::string encoding = pDecl->Encoding();
				UPR(encoding);

				if (encoding.compare(std::string("UTF-8")) == 0 || encoding.compare(std::string("UTF8")) == 0)
				{
					bUTF8 = true;
				}
			}
		}
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", pcPath);
		return 0;
	}

	// 최상위 노드를 바탕으로 객체를 생성한다.
	pkObject = pkSelf->CreateObject(pkRootNode, pArg, pkObject, NULL, bUTF8);
	if(!pkObject)
	{
		PG_ASSERT_LOG(!"creating PgXmlObject failed");
		return 0;
	}
	
	/*
	// 객체의 알맞은 GUID를 부여한다.
	pkObject->SetIDSetGuid(BM::GUID());
	*/
	return pkObject;
}

PgIXmlObject *PgXmlLoader::CreateObject(const TiXmlNode *pkNode, void *pArg, PgIXmlObject *pkObject, char const* pcObjectType, bool bUTF8)
{
	PG_ASSERT_LOG(pkNode);
	
	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);

	char const *szTagName = pkNode->Value();
	if(!szTagName)
	{
		PG_ASSERT_LOG(!"is root name blanked?");
		return 0;
	}

	if(!pkObject)
	{
		pkObject = pkSelf->CreateInstance(szTagName, pcObjectType);
		if(!pkObject)
		{
			NILOG(PGLOG_ERROR, "XmlObject %s instancing failed", szTagName);
			return 0;
		}
	}

	if(!pkObject->ParseXml(pkNode, pArg, bUTF8))
	{
		PG_ASSERT_LOG(!"XmlObject parsing failed");
		SAFE_DELETE_NI(pkObject);
		return 0;
	}

	return pkObject;
}
char	const*	PgXmlLoader::GetPackFilePath()
{
	return	"../xml.dat";
}

bool PgXmlLoader::IsFile(char const * szFileName)
{
	if(NULL == szFileName)
	{
		return false;
	}

	bool bFindFirstLocal = false;
#ifndef EXTERNAL_RELEASE
	bFindFirstLocal = g_bFindFirstLocal;
#endif

	if(false==g_bUsePackData || bFindFirstLocal)
	{
		std::string strPath = "../XML/";
		strPath += szFileName;
		if(0 == access(strPath.c_str(), 0))
		{
			return true;
		}
	}

	std::vector< char > data(0);
	if(!BM::PgDataPackManager::LoadFromPack(UNI(GetPackFilePath()), (std::wstring)(_T("./"))+UNI(szFileName), data))
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader] LoadFromPack failed, %s xml parse failed\n", szFileName);
		return false;
	}

	return (false == data.empty());
}

bool PgXmlLoader::LoadFile(TiXmlDocument &rkXmlDoc, std::wstring const &rkFileName)
{//	TiXmlDocument 이 객체를 받아서.. 사용을 하는게야.
//	rkXmlDoc.SetUserData(MB(rkFileName));
	if (rkFileName.empty())
	{
		NILOG(PGLOG_WARNING, "[PgXmlLoader] filename is empty\n");
		return false;
	}

	bool bFindFirstLocal = false;
#ifndef EXTERNAL_RELEASE
	bFindFirstLocal = g_bFindFirstLocal;
#endif

	if(false==g_bUsePackData || bFindFirstLocal)
	{
		std::string strPath = "../XML/";
		strPath += MB(rkFileName);
		if (rkXmlDoc.LoadFile(strPath.c_str()))
		{
			return true;
		}
		char pszPath[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, pszPath);
		PgError3("[PgXmlLoader] %s, %s %s\n", rkXmlDoc.ErrorDesc(), pszPath, strPath.c_str());
	}

	std::vector< char > data(0);

	PG_STAT(PgStatTimerF timerA(PgNiFile::ms_kFileIOStatGroup.GetStatInfo("LoadFromPack"), g_pkApp->GetFrameCount()));
	PG_STAT(timerA.Start())
	if(!BM::PgDataPackManager::LoadFromPack(UNI(GetPackFilePath()), (std::wstring)(_T("./"))+rkFileName, data))
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader] LoadFromPack failed, %s xml parse failed\n",MB(rkFileName));
		return false;
	}
	PG_STAT(timerA.Stop());
	
	if(!data.size())
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader] failed loading from Packed Data.\n");
		return false;
	}

	int iDataSize = data.size();
	data.push_back('\0');	// 파싱이 제대로 끝나는 것을 보장하기 위해서.

	try {
		rkXmlDoc.Parse(&data.at(0));
	}
	catch (...)
	{
		PgError6("[PgXmlLoader] exception raised while parsing %s(%d)(%d, %d, %d, %s)\n", MB(rkFileName), iDataSize, rkXmlDoc.ErrorRow(), rkXmlDoc.ErrorCol(), rkXmlDoc.ErrorId(), rkXmlDoc.ErrorDesc());
	}

	if(rkXmlDoc.Error())
	{
		NILOG(PGLOG_ERROR, "[PgXmlLoader] failed to loading %s(%d,%d)(%d, %d, %d, %s)\n", MB(rkFileName), iDataSize, data.size(), rkXmlDoc.ErrorRow(), rkXmlDoc.ErrorCol(), rkXmlDoc.ErrorId(), rkXmlDoc.ErrorDesc());
		return false;
	}
	return true;
}

char const* PgXmlLoader::GetXmlTagValue(const TiXmlNode* pkRootNode, char const* pcTargetTagName, char const* pcTargetAttributeName)
{
	if (pkRootNode == NULL || pcTargetTagName == NULL)
		return NULL;

	char const* retValue = NULL;
	const TiXmlNode *pkNode = pkRootNode;
	while (pkNode)
	{
		if (pkNode->Type() == TiXmlNode::ELEMENT)
		{
			TiXmlElement *pkElement = (TiXmlElement*)pkNode;

			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, pcTargetTagName) == 0)
			{
				// 찾았다.
				if (pcTargetAttributeName == NULL)
					return pkElement->GetText();

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();
					if (strcmp(pcName, pcTargetAttributeName) == 0)
					{
						return pcValue;
					}					
					pkAttr = pkAttr->Next();
				}

			}
			else
			{
				if (pkNode->FirstChild())
				{
					retValue = GetXmlTagValue(pkNode->FirstChild(), pcTargetTagName, pcTargetAttributeName);
					if (retValue)
						return retValue;
				}
			}
		}
		pkNode = pkNode->NextSibling();
	}

	return retValue;
}

char const* PgXmlLoader::GetPathByID(char const *pcID)
{
	if (pcID == NULL)
		return NULL;

	PgXmlLoader *pkSelf = PgXmlLoader::Get();
	PG_ASSERT_LOG(pkSelf);
	if (pkSelf == NULL)
		return NULL;

	std::string strID = pcID;
	UPR(strID);

	XmlPathContainer::iterator itr = pkSelf->m_kXmlPathContainer.find(strID);
	if(itr == pkSelf->m_kXmlPathContainer.end())
	{
		NILOG(PGLOG_ERROR,"[PgXmlLoader] GetXmlDocumentById Can't find %s ID\n", pcID);
		return 0;
	}

	return itr->second.c_str();
}

bool PgXmlLoader::DoClientWork(WorkData& rkWorkData)
{
	TiXmlDocument* pkDoc = GetXmlDocumentInCacheByPath(rkWorkData.strWorkData.c_str(), rkWorkData.strWorkData2.c_str());
	//PG_ASSERT_LOG(pkDoc);

	XmlLoaderUtil::PrepareXmlDocumentAfterWork( rkWorkData.iWorkData, pkDoc );

	/*if (pkDoc && rkWorkData.strWorkData2.empty() == false)
	{
		NILOG(PGLOG_WARNING,"[PgXmlLoader] PrepareObject of %s failed\n", rkWorkData.strWorkData.c_str());
		PrepareObject(rkWorkData.strWorkData2.c_str());
	}*/
	return true;
}
void PgXmlLoader::GetXmlIDByType(PgXmlLoader* pkSelf, CONT_XML_ID& rkList, char const* pcType)
{
	if( !pkSelf || !pcType )
	{
		return;
	}
	std::string kType(pcType);
	UPR(kType);

	pkSelf->m_kCacheLock.Lock();
	XmlTypeContainer::const_iterator iter = pkSelf->m_kXmlTypeContainer.begin();
	while( pkSelf->m_kXmlTypeContainer.end() != iter )
	{
		if( (*iter).second == kType )
		{
			std::back_inserter(rkList) = (*iter).first;
		}
		++iter;
	}
	pkSelf->m_kCacheLock.Unlock();
}