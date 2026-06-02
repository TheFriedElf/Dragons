#include "stdafx.h"
#include "Variant/Global.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgQuestInfo.h"
#include "XUI/XUI_Wnd.h"
#include "XUI/XUI_Button.h"
#include "PgUIScene.h"
#include "PgNifMan.h"
#include "PgOption.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgWorldMapUI.h"
#include "PgPilotMan.h"
#include "PgQuestMan.h"
#include "ServerLib.h"

#define PG_WORLDMAPUI_STRING_WORLD				_T("WORLD")
#define PG_WORLDMAPUI_STRING_REGION				_T("REGION")
#define PG_WORLDMAPUI_STRING_MAP				_T("MAP")

PgWorldMapUI::PgWorldMapUI() :
	m_pkForm(0)
	, m_pkSubForm(0)
	, m_kTargetOrgPt(0,0,0)
	, m_kTargetDeltaPt(0,0,0)	
{
	g_kUIScene.SetWorldMapUI(this);
	PlayerWorldMapNo(0);
	m_bTargetMoveUp = true;
	m_pkTargetForm = 0;
	m_fArrowStartAccumTime = 0;
	m_fArrowAccumTime = 0;
	m_kMiniMapPath.clear();
	m_kMapDescriptionIDCont.clear();
}

PgWorldMapUI::~PgWorldMapUI()
{
	{
		WorldMapCont::iterator itr = m_kWorldMapCont.begin();
		while(itr != m_kWorldMapCont.end())
		{
			SWorldMap& rkMap = (*itr).second;

			CONT_WM_SHARD::iterator shard_itr = rkMap.kShardCont.begin();
			while(shard_itr != rkMap.kShardCont.end())
			{
				SWorldMapShard &rkUI = ((*shard_itr).second);
				rkUI.pkWnd = 0;
				++shard_itr;
			}
			rkMap.kShardCont.clear();

			CONT_WM_BGFORM::iterator bgform_itr = rkMap.kBGFormCont.begin();
			while(bgform_itr != rkMap.kBGFormCont.end())
			{
				SWorldMapBGForm &rkUI = ((*bgform_itr).second);
				rkUI.pkWnd = 0;
				++bgform_itr;
			}
			rkMap.kBGFormCont.clear();
			++itr;
		}
	}
	m_kWorldMapCont.clear();

	{
		XUIFormCont::iterator itr = m_kUICont.begin();
		while(itr != m_kUICont.end())
		{
			((*itr).second) = 0;
			++itr;
		}
	}
	m_kUICont.clear();
}

bool PgWorldMapUI::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "WORLDMAP") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "WORLD") == 0)
			{
				SWorldMap kWorldMap;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						kWorldMap.wstrID = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "TYPE") == 0)
					{
						kWorldMap.wstrType = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "PARENT") == 0)
					{
						kWorldMap.wstrParent = UNI(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				// 안으로 한번더 파싱.
				std::pair<WorldMapCont::iterator, bool> kRet = 
					m_kWorldMapCont.insert(std::make_pair(kWorldMap.wstrID, kWorldMap));
				if (kRet.second)
				{
					SWorldMap& rkkWorldMap = ((*(kRet.first)).second);
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if(pkChildNode != 0)
					{
						if(!ParseXml_Shard(pkChildNode, pArg, rkkWorldMap))
						{
							return false;
						}
					}
				}
			}
			else
			{
				NILOG(PGLOG_WARNING, "[PgWorld] unknown %s node in %s world\n", pcTagName, GetID().c_str());
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	// xml 파싱이 성공적으로 끝났다면,
	if(strcmp(pkNode->Value(), "WORLDMAP") == 0)
	{
		// ?
	}

	return true;
}

bool PgWorldMapUI::ParseXml_Shard(const TiXmlNode *pkNode, void *pArg, SWorldMap& rkWorldMap)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "SHARD") == 0)
			{
				SWorldMapShard kShard;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						std::vector<std::string> kVec;
						PgStringUtil::BreakSep(pkAttr->Value(), kVec, "/");
						kShard.kVecId = kVec;
					}
					else if(strcmp(pkAttr->Name(), "IMG") == 0)
					{
						kShard.wstrImg = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "OVER_IMG") == 0)
					{
						kShard.wstrOverImg = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_X") == 0)
					{
						kShard.ptPos.x = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_Y") == 0)
					{
						kShard.ptPos.y = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_Z") == 0)
					{
						kShard.ptPos.z = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_WIDTH") == 0)
					{
						kShard.ptSize.x = (unsigned int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_HEIGHT") == 0)
					{
						kShard.ptSize.y = (unsigned int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "CHILD") == 0)
					{
						kShard.wstrChild = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "TYPE") == 0)
					{
						if(strcmp(pkAttr->Value(), "FIELD") == 0)
						{
							kShard.eShardType = SWorldMapShard::EShardType_Field;
						}
						else if(strcmp(pkAttr->Value(), "TOWN") == 0)
						{
							kShard.eShardType = SWorldMapShard::EShardType_Town;
						}
						else if(strcmp(pkAttr->Value(), "DUNGEON") == 0)
						{
							kShard.eShardType = SWorldMapShard::EShardType_Dungeon;
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pkAttr->Value());
						}
					}
					else if(strcmp(pkAttr->Name(), "NAME_DEFSTR") == 0)
					{
						kShard.wstrNameDefString = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "LOCATION_TTW") == 0)
					{
						kShard.iLocationTTW = (unsigned int)atoi(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				if (kShard.kVecId.size() > 0)
				{
					rkWorldMap.kShardCont.insert(std::make_pair(UNI(kShard.kVecId[0]), kShard));
				}
			}
			else if(strcmp(pcTagName, "BGFORM") == 0)
			{
				SWorldMapBGForm kBGForm;

				// 배경 폼.
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						kBGForm.kVecId.push_back(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG") == 0)
					{
						kBGForm.wstrImg = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_X") == 0)
					{
						kBGForm.ptPos.x = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_Y") == 0)
					{
						kBGForm.ptPos.y = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_Z") == 0)
					{
						kBGForm.ptPos.z = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_WIDTH") == 0)
					{
						kBGForm.ptSize.x = (unsigned int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_HEIGHT") == 0)
					{
						kBGForm.ptSize.y = (unsigned int)atoi(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				if (kBGForm.kVecId.size() > 0)
				{
					rkWorldMap.kBGFormCont.insert(std::make_pair(UNI(kBGForm.kVecId[0]), kBGForm));
				}
			}
			else if(strcmp(pcTagName, "FORM") == 0)
			{
			}
			else if(strcmp(pcTagName, "FONT") == 0)
			{
				SWorldMapFont kFontForm;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "TTW") == 0)
					{
						kFontForm.iTTW = atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "POS_X") == 0)
					{
						kFontForm.ptPos.x = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "POS_Y") == 0)
					{
						kFontForm.ptPos.y = (float)atof(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "POS_Z") == 0)
					{
						kFontForm.ptPos.z = (float)atof(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				if (kFontForm.iTTW)
				{
					std::wstring wstrID = (std::wstring)(BM::vstring(kFontForm.iTTW));
					rkWorldMap.kFontCont.insert(std::make_pair(wstrID, kFontForm));
				}
			}
		}
	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml_Shard(pkNextNode, pArg, rkWorldMap))
		{
			return false;
		}
	}
	return true;
}

bool PgWorldMapUI::ParseXml_WorldNpc(const TiXmlNode *pkNode, char const *pkXMLPath, int iMapNo)
{
	MapNpcSet kNpcSet;
	ParseXml_WorldNpc(pkNode, kNpcSet, pkXMLPath);

	/*m_kNpcLock.Lock();
	m_kMapNpcCont.insert(std::make_pair(UNI(pkXMLPath), kNpcSet));
	
	if(0<iMapNo)
	{
		MapNpcSet::const_iterator npc_it = kNpcSet.begin();
		while(npc_it!=kNpcSet.end())
		{
			m_kNpcGuidToMapNo.insert(std::make_pair(npc_it->kGuid, iMapNo));

			std::wstring kName = (*npc_it).wstrName;
			UPR(kName);
			m_kNpcFromMapNo.insert(std::make_pair(kName, iMapNo));
			++npc_it;
		}
	}

	m_kNpcLock.Unlock();*/

	return true;
}

bool PgWorldMapUI::ParseXml_WorldNpc(const TiXmlNode *pkNode, MapNpcSet& rkNpcSet, char const *pkXMLPath)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			std::string kXMLPath = pkXMLPath;
			UPR(kXMLPath);

			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "LOCAL") == 0)
			{
				TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgWorld");
				if( pkFindLocalNode )
				{
					TiXmlNode const* pkResultNode = pkFindLocalNode->FirstChildElement();
					if( pkResultNode )
					{
						if(false==ParseXml_WorldNpc(pkResultNode, rkNpcSet, pkXMLPath))
						{
							return false;
						}
					}
				}
			}
			else if(strcmp(pcTagName, "WORLD") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				int iDesctibID = 0;
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if (_stricmp(pcAttrName, "DESCRIPTION") == 0)
					{
						iDesctibID = (unsigned int)atoi(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}
				
				if(0<iDesctibID)
				{
					m_kMapDescriptionIDCont.insert(std::make_pair(UNI(kXMLPath), iDesctibID));
				}

				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml_WorldNpc(pkChildNode, rkNpcSet, pkXMLPath))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "NPC") == 0)
			{
				SNpcCreateInfo kNpcInfo;

				unsigned int iNpcNo = 0;
				unsigned int iNpcKID = 0;
				unsigned int iDesctibID = 0;
				BM::GUID kGuid;
				char const* pcActorName = 0;
				char const* pcScriptName = 0;
				NiPoint3 kPos = NiPoint3(0,0,0);
				NiPoint3 kDir = NiPoint3(0,0,0);
				int iType = 1;
				bool bHideMiniMap = false;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if (_stricmp(pcAttrName, "NAME") == 0)
					{
						iNpcNo = (unsigned int)atoi(pcAttrValue);
					}
					else if (_stricmp(pcAttrName, "ACTOR") == 0)
					{
						pcActorName = pcAttrValue;
					}
					else if (_stricmp(pcAttrName, "SCRIPT") == 0)
					{
						pcScriptName = pcAttrValue;
					}
					else if (_stricmp(pcAttrName, "GUID") == 0)
					{
						kGuid.Set(pcAttrValue);
					}
					else if (_stricmp(pcAttrName, "KID") == 0)
					{
						iNpcKID = (unsigned int)atoi(pcAttrValue);
					}
					else if (_stricmp(pcAttrName, "LOCATION") == 0)
					{
						sscanf_s(pcAttrValue, "%f, %f, %f", &kPos.x, &kPos.y, &kPos.z);						
					}
					else if (_stricmp(pcAttrName, "DIRECTION") == 0)
					{
						sscanf_s(pcAttrValue, "%f, %f, %f", &kDir.x, &kDir.y, &kDir.z);						
					}
					else if (_stricmp(pcAttrName, "TYPE") == 0)
					{
						if (_stricmp(pcAttrValue, "FIXED") == 0)
							iType = 1;
						else if (_stricmp(pcAttrValue, "DYNAMIC") == 0)
							iType = 2;
						else if (_stricmp(pcAttrValue, "CREATURE") == 0)
							iType = 3;
						else
						{
							NILOG(PGLOG_WARNING, "[PgWorld] %s NPC has unknown type(%s)\n", pcActorName, pcAttrValue);
						}
					}
					else if (_stricmp(pcAttrName, "DESCRIPTION") == 0)
					{
						iDesctibID = (unsigned int)atoi(pcAttrValue);
					}
					else if ( _stricmp(pcAttrName, "HIDE_MINIMAP") == 0 )
					{
						int iHideMiniMap = atoi( pcAttrValue );
						bHideMiniMap = (iHideMiniMap ? true : false);
					}

					pkAttr = pkAttr->Next();
				}

				std::wstring const* pkString = NULL;
				if( iNpcNo
				&&	::GetDefString(iNpcNo, pkString) )
				{
					kNpcInfo.kGuid = kGuid;
					kNpcInfo.kPosition.x = kPos.x;
					kNpcInfo.kPosition.y = kPos.y;
					kNpcInfo.kPosition.z = kPos.z;
					kNpcInfo.iID = iNpcKID;
					kNpcInfo.eType = (ENpcType)iType;
					kNpcInfo.wstrName = *pkString;
					kNpcInfo.wstrActor = UNI(pcActorName);
					kNpcInfo.wstrScriptName = UNI(pcScriptName);
					kNpcInfo.iDescriptionID = iDesctibID;
					kNpcInfo.bHideMiniMap = bHideMiniMap;
					g_kQuestMan.AddNpcName(kNpcInfo.kGuid, kNpcInfo.wstrName);
					ParseNpcName(kNpcInfo.wstrName.c_str(), kNpcInfo.wstrName);
					rkNpcSet.push_back(kNpcInfo);
				}

			}
			else if(strcmp(pcTagName, "MINIMAP") == 0)
			{
				std::string kMiniMapImage = pkElement->GetText();
				if (!kMiniMapImage.empty())
				{
#ifndef EXTERNAL_RELEASE
					NILOG(PGLOG_LOG, "[PgWorldMapUI::ParseXml_WorldNpc] MapXML : %s MinimapPath : %s\n", kXMLPath.c_str(), kMiniMapImage.c_str());
#endif
					m_kMiniMapPath.insert(std::make_pair(UNI(kXMLPath), UNI(kMiniMapImage)));
				}
			}
		}
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml_WorldNpc(pkNextNode, rkNpcSet, pkXMLPath))
		{
			return false;
		}
	}

	// xml 파싱이 성공적으로 끝났다면, 암것도 안함....
	//if(strcmp(pkNode->Value(), "NPC") == 0)
	//{
	//}

	return true;
}
bool PgWorldMapUI::Initialize(XUI::CXUI_Form* pkForm)
{
	if( !pkForm )
	{
		PG_ASSERT_LOG(pkForm);
		return false;
	}
	m_pkForm = pkForm;

	m_pkSubForm = m_pkForm->GetControl(_T("FRM_SUB_WORLDMAP"));
	if( !m_pkSubForm )
	{
		PG_ASSERT_LOG(m_pkSubForm);
		return false;
	}

	// Test용.
	m_wstrCurrentWorldID = L"Island01";
	ChangeWorldMap(m_wstrCurrentWorldID);

	m_fArrowAccumTime = m_fArrowStartAccumTime = g_pkApp->GetAccumTime();
	m_kMapDescriptionIDCont.clear();
	m_kNpcFromMapNo.clear();
	m_kNpcGuidToMapNo.clear();

	return true;
}

void PgWorldMapUI::Terminate()
{
	ReleaseControl();
}

void PgWorldMapUI::ReleaseControl()
{
	if( m_pkSubForm )
	{
		m_pkSubForm->RemoveAllControls();
		m_pkSubForm = NULL;
	}

	m_kUICont.clear();
	//XUIFormCont::iterator wnd_itr = m_kUICont.begin();
	//// 기존 UI들을 Visible을 꺼준다.
	//while(wnd_itr != m_kUICont.end())
	//{
	//	XUI::CXUI_Form* pkForm = ((*wnd_itr).second);
	//	if (pkForm)
	//	{
	//		m_pkSubForm()->RemoveControl(((*wnd_itr).first));
	//	}
	//	++wnd_itr;
	//}
	//m_kUICont.clear();
}

void PgWorldMapUI::AddIconToParent(lwUIWnd kParent, XUI::CXUI_Form* pkChildForm, std::wstring wstrID, const lwPoint3& ptPos, const POINT2& ptSize, std::string strImg)
{
	if (!pkChildForm)
	{
		return;
	}
	lwUIWnd kChild = lwUIWnd(pkChildForm);
	kChild.SetID(wstrID);
	kChild.SetLocation3(ptPos);
	kChild.ChangeImage( (char*)strImg.c_str(), false );
	kChild.SetSize(lwPoint2(ptSize));
	kChild.SetImgSize(lwPoint2(ptSize));
	pkChildForm->Visible(true);
	kChild.SetParent(kParent);
}

void PgWorldMapUI::RefreshForm(SWorldMap& rkWorldMap)
{
	if( !m_pkForm || !m_pkSubForm )
	{
		PG_ASSERT_LOG(m_pkForm);
		PG_ASSERT_LOG(m_pkSubForm);
		return;
	}

	XUI::CXUI_Form *pkIconWnd = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Call(_T("FRM_WORLDMAP_ICON_STATE")));
	XUI::CXUI_Form *pkFontWnd = dynamic_cast<XUI::CXUI_Form*>(m_pkForm->GetControl(_T("FRM_TEXT")));
	if( !pkIconWnd || !pkFontWnd)
	{
		PG_ASSERT_LOG(pkIconWnd);
		PG_ASSERT_LOG(pkFontWnd);
		return;
	}
	pkFontWnd->Text(L"");

	XUIFormCont::iterator wnd_itr = m_kUICont.begin();
	// 기존 UI들을 Visible을 꺼준다.
	while(wnd_itr != m_kUICont.end())
	{
		XUI::CXUI_Form* pkForm = ((*wnd_itr).second);
		if (pkForm)
		{
			pkForm->Visible(false);
			pkForm->Text(L"");
			pkForm->Visible(false);
		}
		++wnd_itr;
	}
	wnd_itr = m_kUICont.begin();

	// FRM_WORLDMAP, 메인 WorldMap
	lwUIWnd kWnd = m_pkSubForm;

	// BGForm
	CONT_WM_BGFORM& rkBGFormCont = rkWorldMap.kBGFormCont;
	CONT_WM_BGFORM::iterator bg_itr = rkBGFormCont.begin();
	while(bg_itr != rkBGFormCont.end())
	{
		SWorldMapBGForm &rkUI = ((*bg_itr).second);
		std::wstring wstrUIName = BM::GUID::Create().str();
		XUI::CXUI_Form* pkCloneForm = 0;
		{
			pkCloneForm = (XUI::CXUI_Form*)pkIconWnd->VClone();
			m_kUICont.insert(std::make_pair(wstrUIName, pkCloneForm));
		}
		
		if (!pkCloneForm)
		{
			++bg_itr;
			continue;
		}

		// Data
		//POINT2 ptPos = rkUI.ptPos;
		lwPoint3 kPt3 = lwPoint3((float)rkUI.ptPos.x, (float)rkUI.ptPos.y, (float)rkUI.ptPos.z);
		std::string strImage = MB(rkUI.wstrImg);
		NiSourceTexture* tex = g_kNifMan.GetTexture(strImage);
		if (!tex)
		{
			++bg_itr;
			continue;
		}
		if (!rkUI.ptSize.x && !rkUI.ptSize.y)
		{
			rkUI.ptSize.x = tex->GetWidth();
			rkUI.ptSize.y = tex->GetHeight();
		}

		rkUI.pkWnd = pkCloneForm;
		AddIconToParent(kWnd, pkCloneForm, wstrUIName, kPt3, static_cast<POINT2>(rkUI.ptSize), strImage);

		++bg_itr;
	}

	// Shard Child(Icon)
	CONT_WM_SHARD& rkShardCont = rkWorldMap.kShardCont;
	CONT_WM_SHARD::iterator shard_itr = rkShardCont.begin();
	while(shard_itr != rkShardCont.end())
	{
		SWorldMapShard &rkUI = ((*shard_itr).second);
		std::wstring wstrUIName = BM::GUID::Create().str();
		XUI::CXUI_Form* pkCloneForm = 0;
		{
			pkCloneForm = (XUI::CXUI_Form*)pkIconWnd->VClone();
			m_kUICont.insert(std::make_pair(wstrUIName, pkCloneForm));
		}
		
		if (!pkCloneForm)
		{
			++shard_itr;
			continue;
		}

		// Data
		std::string strImage = MB(rkUI.wstrImg);
		NiSourceTexture* tex = g_kNifMan.GetTexture(strImage);
		
		if (!tex)
		{
			++shard_itr;
			continue;
		}
		if (!rkUI.ptSize.x && !rkUI.ptSize.y)
		{
			rkUI.ptSize.x = tex->GetWidth();
			rkUI.ptSize.y = tex->GetHeight();
		}

		lwPoint3 kPt3 = lwPoint3((float)rkUI.ptPos.x, (float)rkUI.ptPos.y, (float)rkUI.ptPos.z);
		POINT2 ptSize(rkUI.ptSize);

		rkUI.pkWnd = pkCloneForm;
		AddIconToParent(kWnd, pkCloneForm, wstrUIName, kPt3, ptSize, strImage);

		int iMapNo = 0;
		if (rkUI.wstrNameDefString.size() != 0)
		{
			iMapNo = _wtoi(rkUI.wstrNameDefString.c_str());
		}
		else
		{
			iMapNo = _wtoi((*shard_itr).first.c_str());
		}

		// 텍스트. 이름
		const wchar_t *pText = NULL;
		if(iMapNo != 0 && GetDefString(iMapNo, pText))
		{
			lwUIWnd kCloneWnd(pkCloneForm);
			if (!kCloneWnd.IsNil())
			{
				std::wstring wstrText;	//폰트 초기화
				wstrText += pText;
				kCloneWnd()->Text(wstrText);
				kCloneWnd()->Visible(true);

				// 좌표.
				POINT2 ptText;
				ptText.x = (LONG)(tex->GetWidth()*0.5f);
				ptText.y = tex->GetHeight()-10;
				kCloneWnd()->TextPos(ptText);
			}
		}
		//

		++shard_itr;
	}

	// Font
	CONT_WM_FONT& rkFontCont = rkWorldMap.kFontCont;
	CONT_WM_FONT::iterator font_itr = rkFontCont.begin();
	while(font_itr != rkFontCont.end())
	{
		SWorldMapFont& rkFont = (*font_itr).second;
		pkFontWnd->Text(TTW(rkFont.iTTW));
		pkFontWnd->Location((int)rkFont.ptPos.x, (int)rkFont.ptPos.y, (int)rkFont.ptPos.z);
		++font_itr;
	}

	// Target
	SWorldMapShard kMarkShard;
	if (FindCurShardFromMapNo(kMarkShard, PlayerWorldMapNo(), rkWorldMap.wstrID))
	{
		std::string strUIName = MB(BM::GUID::Create().str());
		kMarkShard.kVecId[0] = strUIName.c_str();
		kMarkShard.kVecId[0] += "_target";
		XUI::CXUI_Form* pkCloneForm = 0;
		{
			pkCloneForm = (XUI::CXUI_Form*)pkIconWnd->VClone();
			m_kUICont.insert(std::make_pair(UNI(kMarkShard.kVecId[0]), pkCloneForm));
		}
		if (pkCloneForm)
		{
			std::string strImg = "../Data/6_ui/map/mapArrowSimple.tga";
			NiSourceTexture* tex = g_kNifMan.GetTexture(strImg);
			POINT2 ptTexSize;
			ptTexSize.x = tex->GetWidth();
			ptTexSize.y = tex->GetHeight();

			POINT2 ptTarget;
			ptTarget.x = (LONG)kMarkShard.ptPos.x + (int)(kMarkShard.ptSize.x * 0.5f);
			ptTarget.y = (LONG)kMarkShard.ptPos.y + (int)(kMarkShard.ptSize.y * 0.5f);
			ptTarget.x = (LONG)(ptTarget.x - (ptTexSize.x * 0.5f));
			ptTarget.y = ptTarget.y - (ptTexSize.y);

			lwPoint3 ptTarget3 = lwPoint3((float)ptTarget.x, (float)ptTarget.y, 1);
			m_kTargetOrgPt = ptTarget3;
			m_pkTargetForm = pkCloneForm;
			AddIconToParent(kWnd, pkCloneForm, UNI(kMarkShard.kVecId[0]), ptTarget3, ptTexSize, strImg);
		}
	}

	pkIconWnd->Close();

	m_pkForm->SortChildGreater();

	return;
}

bool PgWorldMapUI::ChangeWorldMap(std::wstring wstrWorldID)
{
	WorldMapCont::iterator itr = m_kWorldMapCont.find(wstrWorldID);
	if (itr != m_kWorldMapCont.end())
	{
		SWorldMap& rkMap = (*itr).second;
		// 월드 맵 으로 가려 할 때는 불가능.
		if (rkMap.wstrType == PG_WORLDMAPUI_STRING_WORLD)
		{
			return false;
		}

		RefreshForm(rkMap);
		return true;
	}

	return false;
}

bool PgWorldMapUI::FindCurShardFromMapNo(SWorldMapShard& rkRetShard, int iMapNo, std::wstring wstrCurWorldID)
{
	if (!iMapNo)
	{
		return false;
	}
	char szMapNo[32];
	_itoa_s(iMapNo, szMapNo, 10);
	std::wstring wstrMapNo = UNI(szMapNo);

	// 맵번호에 해당하는 Shard를 찾아준다.
	WorldMapCont::iterator find_world_itr = m_kWorldMapCont.end();
	CONT_WM_SHARD::iterator find_shard_itr;
	WorldMapCont::iterator world_itr = m_kWorldMapCont.begin();
	while(world_itr != m_kWorldMapCont.end())
	{
		SWorldMap& rkWorldMap = (*world_itr).second;
		if (rkWorldMap.wstrType != PG_WORLDMAPUI_STRING_MAP)
		{
			++world_itr;
			continue;
		}

		CONT_WM_SHARD::iterator shard_itr = rkWorldMap.kShardCont.begin();
		while(shard_itr != rkWorldMap.kShardCont.end())
		{
			const SWorldMapShard &rkShard = ((*shard_itr).second);
			for (unsigned int i=0 ; i<rkShard.kVecId.size() ; i++)
			{
				if (rkShard.kVecId[i] == MB(wstrMapNo))
				{
					find_world_itr = world_itr;
					find_shard_itr = shard_itr;
					break;
				}
			}
			++shard_itr;
		}

		++world_itr;
	}

	// ID를 체크한다. 계속 Parent로 올라가면서 체크 해본다.
	if (find_world_itr == m_kWorldMapCont.end())
	{
		return false;
	}
	// 현제 위치의 World와 찾은 World의 ID가 같다면 그냥 SHard 리턴.
	SWorldMap& rkFindWorldMap = (*find_world_itr).second;
	if (rkFindWorldMap.wstrID == wstrCurWorldID)
	{
		rkRetShard = (*find_shard_itr).second;
		return true;
	}
	std::wstring wstrFindID = rkFindWorldMap.wstrID;
	std::wstring wstrFindParent = rkFindWorldMap.wstrParent;
	//std::wstring wstrParent = wstrFindParent;
	
	// 현재의 World
	WorldMapCont::iterator cur_world_itr = m_kWorldMapCont.find(wstrCurWorldID);
	if (cur_world_itr == m_kWorldMapCont.end())
	{
		return false;
	}
	const SWorldMap& rkCurWorldMap = (*cur_world_itr).second;
	//std::wstring wstrCurID = rkCurWorldMap.wstrID;
	//std::wstring wstrParent = rkCurWorldMap.wstrParent;

	// 똑같은 월드는 아닌데.. 타입이 같을 경우.
	if (rkFindWorldMap.wstrType == rkCurWorldMap.wstrType)
	{
		return false;
	}

	// Parent로 계속 올라 가면서 Shard를 찾는다.
	bool bLoop = true;
	while(bLoop)
	{
		find_world_itr = m_kWorldMapCont.find(wstrFindParent);
		if (find_world_itr == m_kWorldMapCont.end())
		{
			return false;
		}
		SWorldMap& rkParentWorldMap = (*find_world_itr).second;

		// TODO : Parent로 계속 올라가서 찾고 있는데.. 전혀 다른곳에.. 현재 있는 장소(샤드)가 있다면
		// 표시 되지 말아야 함(return false)
		CONT_WM_SHARD::iterator shard_itr = rkParentWorldMap.kShardCont.begin();
		while(shard_itr != rkParentWorldMap.kShardCont.end())
		{
			SWorldMapShard &rkShard = ((*shard_itr).second);
			if (rkShard.wstrChild == wstrFindID)
			{
				rkRetShard = rkShard;
				return true;
			}
			++shard_itr;
		}

		wstrFindID = rkParentWorldMap.wstrID;
		wstrFindParent = rkParentWorldMap.wstrParent;
	}
	
	return false;
}

void PgWorldMapUI::RenderWorldMap()
{
}

void PgWorldMapUI::PulseWorldMap()
{
	float const fAccumTime = g_pkApp->GetAccumTime();
	float const fTotalTime = 0.3f;
	float const fTotalLength = 20.0f;
	float fPercent = (fAccumTime - m_fArrowAccumTime) / fTotalTime;
	if (fPercent > 1.0f)
	{
		fPercent = 1.0f;
	}

	if (m_bTargetMoveUp)
	{
		float fLength = fTotalLength * fPercent;
		m_kTargetDeltaPt.SetY(fLength);
	}
	else
	{
		float fLength = fTotalLength * fPercent;
		m_kTargetDeltaPt.SetY(fTotalLength - fLength);
	}

	if (fPercent >= 1.0f)
	{
		m_bTargetMoveUp = !m_bTargetMoveUp;
		m_fArrowAccumTime = fAccumTime;
	}

	// Set
	if (m_pkTargetForm)
	{
		lwUIWnd kWnd = lwUIWnd(m_pkTargetForm);
		lwPoint3 kResultPt = lwPoint3(m_kTargetOrgPt() + m_kTargetDeltaPt());
		kWnd.SetLocation3(kResultPt);
	}
}

void PgWorldMapUI::InSideWorldMap()
{
}

void PgWorldMapUI::OutSideWorldMap()
{
	WorldMapCont::const_iterator world_itr = m_kWorldMapCont.find(m_wstrCurrentWorldID);
	if(world_itr == m_kWorldMapCont.end())
	{
		return;
	}
	const SWorldMap& rkMap = (*world_itr).second;

	WorldMapCont::const_iterator parent_itr = m_kWorldMapCont.find(rkMap.wstrParent);
	if (parent_itr != m_kWorldMapCont.end())
	{
		const SWorldMap& rkParentMap = (*parent_itr).second;
		if (ChangeWorldMap(rkParentMap.wstrID))
		{
			m_wstrCurrentWorldID = rkParentMap.wstrID;
		}
	}
}

void PgWorldMapUI::MouseClickWorldMap(const POINT2& pt)
{
	WorldMapCont::const_iterator world_itr = m_kWorldMapCont.find(m_wstrCurrentWorldID);
	if(world_itr == m_kWorldMapCont.end())
	{
		return;
	}

	// 마우스 좌표로 계산.
	const SWorldMap& rkMap = (*world_itr).second;
	CONT_WM_SHARD::const_iterator shard_itr = rkMap.kShardCont.begin();

	bool bClickInRect = false;
	while(shard_itr != rkMap.kShardCont.end())
	{
		const SWorldMapShard &rkShard = ((*shard_itr).second);

		RECT rtRect;
		rtRect.left = (LONG)rkShard.ptPos.x;
		rtRect.top = (LONG)rkShard.ptPos.y;
		rtRect.right = (LONG)(rkShard.ptPos.x + rkShard.ptSize.x);
		rtRect.bottom = (LONG)(rkShard.ptPos.y + rkShard.ptSize.y);

		if (rkShard.pkWnd->Visible() && PtInRect(&rtRect, pt))
		{
			bClickInRect = true;
			if (ChangeWorldMap(rkShard.wstrChild))
			{
				m_wstrCurrentWorldID = rkShard.wstrChild;
			}
			else
			{
				int iMapNo = _wtoi((*shard_itr).first.c_str());
				if (iMapNo)
				{
					// 맵 이동
					//rkShard.kVecId[0];
					//Net_ReqMapMove(, 1);
					ClickedMapNo(iMapNo);
					XUI::CXUI_Form *pkIconWnd = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Call(_T("FRM_WORLDMAP_INFORMATION")));
					// 맵
					//lwCallToolTipByText(0, MB(rkShard.wstrID), lwPoint2(pt));
				}
			}
			break;
		}
		++shard_itr;
	}

	if (!bClickInRect)
	{
		//lwCloseToolTip();
	}
}

void PgWorldMapUI::MouseOverWorldMap(const POINT2& pt)const
{
	WorldMapCont::const_iterator world_itr = m_kWorldMapCont.find(m_wstrCurrentWorldID);
	if(world_itr == m_kWorldMapCont.end())
	{
		return;
	}

	// 마우스 좌표로 계산.
	const SWorldMap& rkMap = (*world_itr).second;
	CONT_WM_SHARD::const_iterator shard_itr = rkMap.kShardCont.begin();

	bool bClickInRect = false;
	while(shard_itr != rkMap.kShardCont.end())
	{
		const SWorldMapShard &rkShard = ((*shard_itr).second);
		XUI::CXUI_Wnd* pkOverImg = rkShard.pkWnd->GetControl(L"OVER_IMG");
		RECT rtRect;
		rtRect.left = (LONG)rkShard.ptPos.x;
		rtRect.top = (LONG)rkShard.ptPos.y;
		rtRect.right = (LONG)(rkShard.ptPos.x + rkShard.ptSize.x);
		rtRect.bottom = (LONG)(rkShard.ptPos.y + rkShard.ptSize.y);

		if (!bClickInRect && rkShard.wstrOverImg.size() && rkShard.pkWnd->Visible() && PtInRect(&rtRect, pt))
		{
			bClickInRect = true;
			if (pkOverImg)
			{
				lwUIWnd kWnd(pkOverImg);
				kWnd.ChangeImage( (char*)MB(rkShard.wstrOverImg), false );
				//lwPoint3 kPt3 = lwPoint3((float)rkShard.ptPos.x, (float)rkShard.ptPos.y, 1);
				//kWnd.SetLocation3(kPt3);
				//kChild.ChangeImage((char*)strImg.c_str());
				kWnd.SetSize(lwPoint2(static_cast<POINT2>(rkShard.ptSize)));
				kWnd.SetImgSize(lwPoint2(static_cast<POINT2>(rkShard.ptSize)));
				pkOverImg->PassEvent(true);
				pkOverImg->Visible(true);
			}
		}
		else
		{
			if (pkOverImg->DefaultImgName() != rkShard.wstrImg)
			{
				//pkOverImg->DefaultImgName(rkShard.wstrImg);
				pkOverImg->Visible(false);
			}
		}
		++shard_itr;
	}

	if (!bClickInRect)
	{
		//lwCloseToolTip();
	}
}

bool PgWorldMapUI::GetImgFromShardId(std::wstring& rkOutImg, std::wstring wstrID)const
{
	WorldMapCont::const_iterator itr = m_kWorldMapCont.begin();
	while(itr != m_kWorldMapCont.end())
	{
		const SWorldMap& rkMap = (*itr).second;
		//CONT_WM_SHARD kShardCont;
		CONT_WM_SHARD::const_iterator shard_itr = rkMap.kShardCont.begin();
		while(shard_itr != rkMap.kShardCont.end())
		{
			const SWorldMapShard &rkShard = ((*shard_itr).second);
			for (unsigned int i=0 ; i<rkShard.kVecId.size() ; i++)
			{
				if (rkShard.kVecId[i] == MB(wstrID))
				{
					rkOutImg = rkShard.wstrImg;

					return true;
				}
			}
			++shard_itr;
		}
		++itr;
	}

	return false;
}

bool PgWorldMapUI::GetShardFromId(SWorldMapShard& rkOutShard, std::wstring wstrID)const
{
	WorldMapCont::const_iterator itr = m_kWorldMapCont.begin();
	while(itr != m_kWorldMapCont.end())
	{
		const SWorldMap& rkMap = (*itr).second;
		CONT_WM_SHARD::const_iterator shard_itr = rkMap.kShardCont.begin();
		while(shard_itr != rkMap.kShardCont.end())
		{
			const SWorldMapShard &rkShard = ((*shard_itr).second);
			for (unsigned int i=0 ; i<rkShard.kVecId.size() ; i++)
			{
				if (rkShard.kVecId[i] == MB(wstrID))
				{
					rkOutShard = rkShard;
					return true;
				}
			}
			++shard_itr;
		}
		++itr;
	}

	return false;
}

void PgWorldMapUI::OnBuild()
{
	const CONT_DEFMAP* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	CONT_DEFMAP::const_iterator map_it = pContDefMap->begin();
#ifndef EXTERNAL_RELEASE
	NILOG(PGLOG_LOG, "[PgWorldMapUI::OnBuild] start mapCount = %d\n", pContDefMap->size());
#endif

	PgWorldNpcMgr::CONT_WORLD_NPC const &cont_NPC = g_kWorldNpcMgr.GetWorldNpc();

	PgWorldNpcMgr::CONT_WORLD_NPC::const_iterator it = cont_NPC.begin();

	while(cont_NPC.end()!=it)
	{
		PgWorldNpcMgr::CONT_WORLD_NPC::mapped_type const &rkNpcCont = (*it).second;
		PgWorldNpcMgr::CONT_WORLD_NPC::mapped_type::const_iterator npc_it = rkNpcCont.begin();
		if(rkNpcCont.end()!=npc_it)
		{

		}
		while(rkNpcCont.end()!=npc_it)
		{
			PgWorldNpcMgr::CONT_WORLD_NPC::mapped_type::mapped_type const &rkNpc = (*npc_it).second;

			++npc_it;
		}
		++it;
	}

	while(pContDefMap->end() != map_it)
	{
		CONT_DEFMAP::mapped_type const & rkMap = (*map_it).second;
		TiXmlDocument* pkDoc = PgXmlLoader::GetXmlDocumentInCacheByPath(MB(rkMap.strXMLPath));
		if(pkDoc)
		{
			TiXmlNode *pkRootNode = pkDoc->FirstChild();
			while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
			{
				pkRootNode = pkRootNode->NextSibling();
			}
			if (pkRootNode)
			{
				ParseXml_WorldNpc(pkRootNode, MB(rkMap.strXMLPath), rkMap.iMapNo);
			}
		}
		++map_it;
	}
#ifndef EXTERNAL_RELEASE
	NILOG(PGLOG_LOG, "[PgWorldMapUI::OnBuild] end\n");
#endif
}

void PgWorldMapUI::ConvertSNpcCreateInfoToSWorldNpc(SWorldNpc const& rkWorldNpc, SNpcCreateInfo& rkCreateInfo)
{
	rkCreateInfo.iDescriptionID = rkWorldNpc.iDescription;
	rkCreateInfo.iID = rkWorldNpc.uiNpcNo;
	rkCreateInfo.eType = static_cast<ENpcType>(rkWorldNpc.iType);
	rkCreateInfo.kGuid = rkWorldNpc.kNpcGuid;
	rkCreateInfo.wstrActor = UNI(rkWorldNpc.kActorName);
	rkCreateInfo.wstrScriptName = UNI(rkWorldNpc.kScriptName);
	rkCreateInfo.kPosition.Set(rkWorldNpc.kPos.x, rkWorldNpc.kPos.y, rkWorldNpc.kPos.z);
}

bool PgWorldMapUI::GetNpc(BM::GUID const& rkGuid, SNpcCreateInfo& kInfo) const
{
	SWorldNpc world_npc = g_kWorldNpcMgr.FindNpc(rkGuid);

	if(0 < world_npc.uiNpcNo)
	{
		PgWorldMapUI::ConvertSNpcCreateInfoToSWorldNpc(world_npc, kInfo);
		
		return true;

	}

	return false;
}

bool PgWorldMapUI::GetNpcSet(std::wstring kXMLPath, MapNpcSet& rkNpcSet)const
{
	/*MapNpcCont::const_iterator itr = m_kMapNpcCont.find(kXMLPath);
	if (itr != m_kMapNpcCont.end())
	{
		rkNpcSet = (*itr).second;
		return true;
	}

	UPR(kXMLPath);
	itr = m_kMapNpcCont.begin();
	while(itr != m_kMapNpcCont.end())
	{
		std::wstring rkXml = (*itr).first;
		UPR(rkXml);
		int iCur = rkXml.find(kXMLPath);
		if (iCur != -1)
		{
			rkNpcSet = (*itr).second;
			return true;
		}
		++itr;
	}*/

	return false;
}

bool PgWorldMapUI::GetNpcSet(int iMapNo, MapNpcSet& rkNpcSet)const
{
	PgWorldNpcMgr::CONT_WORLD_NPC_BY_MAPNO const &cont_NPC = g_kWorldNpcMgr.GetWorldNpcByMapNo();
	PgWorldNpcMgr::CONT_WORLD_NPC_BY_MAPNO::const_iterator cont_it = cont_NPC.find(iMapNo);
	if(cont_NPC.end()==cont_it)
	{
		return false;
	}

	PgWorldNpcMgr::CONT_WORLD_NPC_BY_MAPNO::mapped_type const& kNpcSet = (*cont_it).second;
	PgWorldNpcMgr::CONT_WORLD_NPC_BY_MAPNO::mapped_type::const_iterator npc_itr = kNpcSet.begin();

	MapNpcSet::value_type kCreateInfo;
	while(kNpcSet.end()!=npc_itr)
	{
		PgWorldNpcMgr::CONT_WORLD_NPC_BY_MAPNO::mapped_type::mapped_type const& rkNpcInfo = (*npc_itr).second;
		PgWorldMapUI::ConvertSNpcCreateInfoToSWorldNpc(rkNpcInfo, kCreateInfo);

		rkNpcSet.push_back(kCreateInfo);

		++npc_itr;
	}

	return !rkNpcSet.empty();
}

bool PgWorldMapUI::GetMiniMapPath(int iMapNo, std::wstring& rkPath )const
{
	// 테이블 데이터에서 XML Path 빼온다.
	const CONT_DEFMAP* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);
	CONT_DEFMAP::const_iterator itr = pContDefMap->find(iMapNo);
	if(itr == pContDefMap->end())
	{
		return false;
	}

	const TBL_DEF_MAP& rkDefMap = (*itr).second;
	std::wstring kstrPath = rkDefMap.strXMLPath;
	UPR(kstrPath);

	MiniMapPathCont::const_iterator path_it = m_kMiniMapPath.find(kstrPath);
	if(m_kMiniMapPath.end() != path_it)
	{
		rkPath = (*path_it).second;
		return true;
	}

	return false;
}

bool PgWorldMapUI::GetMapDiscriptionID(int iMapNo, std::wstring& kStr, std::wstring const& rkPath)const
{
	std::wstring kstrPath;
	if(rkPath.empty() || 2>rkPath.size())	//길이가 이상해도
	{
		// 테이블 데이터에서 XML Path 빼온다.
		const CONT_DEFMAP* pContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pContDefMap);
		CONT_DEFMAP::const_iterator itr = pContDefMap->find(iMapNo);
		if(itr == pContDefMap->end())
		{
			return false;
		}

		const TBL_DEF_MAP& rkDefMap = (*itr).second;

		kstrPath+=rkDefMap.strXMLPath;
	}
	else
	{
		kstrPath+=rkPath;
	}
	UPR(kstrPath);

	MapDescriptionIDCont::const_iterator disc_it = m_kMapDescriptionIDCont.find(kstrPath);
	if(m_kMapDescriptionIDCont.end() != disc_it)
	{
		if(0 < (*disc_it).second)
		{
			kStr = TTW((*disc_it).second);
			return !kStr.empty();
		}
	}

	return false;
}

int PgWorldMapUI::GetAlphaFromImg(char const* szPath, int iX, int iY)
{
	if (!szPath)
	{
		return 0;
	}
	std::wstring const kPath = UNI(szPath);
	WorldMapTextureCont::iterator texture_it = m_kWorldMapTextureCont.find(kPath);

	NiSourceTexture* tex = NULL;
	if (m_kWorldMapTextureCont.end() == texture_it) //못찾았으면
	{
		tex = g_kNifMan.GetTexture(std::string(szPath));
		if (tex)
		{ 
			tex->LoadPixelDataFromFile();
			m_kWorldMapTextureCont.insert(std::make_pair(kPath, tex));
		}
		else
		{
			return 0;
		}
	}
	else
	{
		tex = (*texture_it).second;
	}

	if (NULL == tex)
	{
		return 0;
	}

	const NiPixelData* pkPixelData = tex->GetSourcePixelData();
	if (pkPixelData)
	{
		const NiPixelFormat &rkFormat = pkPixelData->GetPixelFormat();
		const unsigned int* pkAddress = (unsigned int*)pkPixelData->operator ()(iX, iY);
		NiDevImageConverter::PixelBits kSrcBits(rkFormat);

		unsigned char ucA = NiDevImageConverter::UnpackAlphaChannel(*pkAddress, kSrcBits);
		return ucA;
	}
	
	return 0;
}

void PgWorldMapUI::Destroy()
{
	WorldMapTextureCont::iterator texture_it = m_kWorldMapTextureCont.begin();
	while (m_kWorldMapTextureCont.end() != texture_it)
	{
		NiSourceTexture* tex = (*texture_it).second;
		if (tex)
		{
			tex->DestroyAppPixelData();
		}
		++texture_it;
	}

	m_kWorldMapTextureCont.clear();
}

int const PgWorldMapUI::GetNpcMapNo(std::wstring const& rkName) const
{
	if(!g_pkWorld)
	{
		return 0;
	}
	std::wstring kUpr = rkName;
	UPR(kUpr);
	NpcFromMapNo::const_iterator map_it = m_kNpcFromMapNo.find(kUpr);
	if(map_it!=m_kNpcFromMapNo.end())
	{
		NpcFromMapNo kTempCont;
		int iFirstFind = (*map_it).second;
		size_t const tCount = m_kNpcFromMapNo.count(rkName);
		typedef std::map<int, std::wstring> FoundNpc;
		FoundNpc kFoundNpc;
		for(size_t i = 0; i<tCount; ++i)
		{
			if( lua_tinker::call<bool, int >("IsInGroundPosTable", (*map_it).second) )
			{
				//return (*map_it).second;
				kFoundNpc.insert(std::make_pair((*map_it).second, (*map_it).first));
			}

			++map_it;
			if(map_it==m_kNpcFromMapNo.end())
			{
				//return iFirstFind;
				break;
			}
		}
		//유효한 후보들을 수집한 후

		FoundNpc::const_iterator npc_it = kFoundNpc.find(g_pkWorld->MapNo());
		if(kFoundNpc.end() != npc_it)	//내가 있는 맵 우선
		{
			iFirstFind = (*npc_it).first;
		}
		else	//없으면 내가 가 본 맵 중에서 찾는다
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if ( pkPlayer )
			{
				FoundNpc::const_iterator npc_it2 = kFoundNpc.begin();
				while(kFoundNpc.end() != npc_it2)
				{
					if(SUCCEEDED(pkPlayer->IsOpenWorldMap((*npc_it2).first)))
					{
						iFirstFind = (*npc_it2).first;
					}
					++npc_it2;
				}
			}
		}

		return iFirstFind;
	}

	return 0;
}

bool const PgWorldMapUI::GetNpcGuidToMapNo(BM::GUID const& rkGuid, std::vector< int >& rkMapNoArray) const
{
	NpcGuidToMapNo::const_iterator map_it = m_kNpcGuidToMapNo.find(rkGuid);
	if(map_it != m_kNpcGuidToMapNo.end())
	{
		size_t const iCount = m_kNpcGuidToMapNo.count(rkGuid);
		for( size_t i = 0; i < iCount; ++i )
		{
			NpcGuidToMapNo::mapped_type const& kMapNo = map_it->second;
			++map_it;
			if( lua_tinker::call<bool, int>("IsInGroundPosTable", kMapNo) )
			{
				rkMapNoArray.push_back(kMapNo);
			}
		}
	}

	return !rkMapNoArray.empty();
}

extern bool ParseNpcName(const wchar_t *pName, std::wstring& rkOut)	//이름에서 < > 안에 있는 스트링만 구하기
{
	std::wstring const kName(pName);
	size_t const kFirst = kName.find(L'[')+1;
	size_t const kLast = kName.find(L']');

	if(kFirst < 0 || kLast < 0 || kName.size() < kFirst || kName.size() <kLast)
	{
		return false;
	}

	rkOut = kName.substr(kFirst, kLast-kFirst);

	return true;
}