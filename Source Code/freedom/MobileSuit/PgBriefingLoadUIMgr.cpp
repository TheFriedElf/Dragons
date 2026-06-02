#include "StdAfx.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgGuild.h"
#include "PgBriefingLoadUIMgr.h"

extern POINT2 GetDefaultLocation(POINT2 const & kPos);

int const MAX_CALLED_UI_COUNT = 100;
int const TT_NO_NEXT = 92;
//----------------------------------------------------------------------------------------------------//
//																									  //
//----------------------------------------------------------------------------------------------------//
void lwUIEmBattleLoad::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "SetBriefingLoadUI", lwSetBriefingLoadUI);
	def(pkState, "CloseBriefingOBJUI", lwCloseBriefingOBJUI);
}

void lwUIEmBattleLoad::lwCloseBriefingOBJUI()
{
	XUI::CXUI_Wnd* pkBriefing = XUIMgr.Get(_T("FRM_BRIEFING_SCREEN"));
	if( pkBriefing )
	{
		pkBriefing->Close();
	}
	g_kBriefingLoadUIMgr.ClearAllOBJUI();
	g_kBriefingLoadUIMgr.Idx(0);
}

bool lwUIEmBattleLoad::lwSetBriefingLoadUI(int const iMapNo, int const iProgress)
{
	switch( iProgress )
	{
	case -1:
		{
			PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pPlayer )
			{
				return false;
			}
			
			SGuildEmporiaInfo const& EmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
			if( EmporiaInfo.byType != EMPORIA_KEY_NONE )
			{
				g_kBriefingLoadUIMgr.Attacker( EmporiaInfo.byType == EMPORIA_KEY_BATTLE_ATTACK );
			}
			else
			{
				switch( EmporiaInfo.byMercenaryFlag )
				{
				case EMPORIA_MERCENARY_NONE:
					{
						return false;
					}break;
				default:
					{
						g_kBriefingLoadUIMgr.Attacker( EmporiaInfo.byMercenaryFlag == EMPORIA_MERCENARY_ATK );
					}break;
				}
			}
		}return false;
	case 0:
		{
			lwCloseBriefingOBJUI();
			XUI::CXUI_Wnd* pkLoadingUI = XUIMgr.Get(L"FRM_LOADING_IMG");
			if( pkLoadingUI )
			{
				XUIMgr.Close(L"LOADING_IMG_EMBATTLE");

				char const* pFormName = g_kUIScene.FindLoadingImg(iMapNo, GATTR_EMPORIABATTLE);
				if( pFormName )
				{
					XUI::CXUI_Wnd* pkImg = pkLoadingUI->GetControl(UNI(pFormName));
					if( pkImg )
					{
						pkImg->Visible(true);
					}
				}
			}

			XUI::CXUI_Wnd* pBriefingPageBtn = XUIMgr.Get(L"FRM_LOADING_HELP_BTN_BAR");
			if( pBriefingPageBtn )
			{
				XUI::CXUI_Button* pkNexBtn = dynamic_cast<XUI::CXUI_Button*>(pBriefingPageBtn->GetControl(L"BTN_NEXT"));
				XUI::CXUI_Button* pkPreBtn = dynamic_cast<XUI::CXUI_Button*>(pBriefingPageBtn->GetControl(L"BTN_PREV"));
				if( pkNexBtn && pkPreBtn )
				{
					pkNexBtn->Disable(false);
					pkPreBtn->Disable(true);

					pkNexBtn->ClearCustomData();
					pkNexBtn->Text(TTW(TT_NO_NEXT));
					pkNexBtn->TwinkleTime(UINT_MAX);
					pkNexBtn->TwinkleInterTime(500);
					pkNexBtn->SetTwinkle(true);
				}
			}
		}return true;
	default:
		{
			XUI::CXUI_Wnd* pkLoadingUI = XUIMgr.Get(L"FRM_LOADING_IMG");
			if( pkLoadingUI )
			{
				XUIMgr.Activate(L"LOADING_IMG_EMBATTLE");

				char const* pFormName = g_kUIScene.FindLoadingImg(iMapNo, GATTR_EMPORIABATTLE);
				if( pFormName )
				{
					XUI::CXUI_Wnd* pkImg = pkLoadingUI->GetControl(UNI(pFormName));
					if( pkImg )
					{
						pkImg->Visible(false);
					}
				}
			}
		}break;
	}

	SBriefingMapInfo kInfo;
	if( !g_kBriefingLoadUIMgr.GetBriefingInfo(iMapNo, kInfo) )
	{
		return false; 
	}

	KCONT_BRIEFING_TICK_ITEM::const_iterator c_iter = kInfo.kTickCont.find(iProgress);
	if( c_iter != kInfo.kTickCont.end() )
	{
		KCONT_BRIEFING_TICK_ITEM::mapped_type const& kTickInfo = c_iter->second;

		XUI::CXUI_Wnd* pBriefingPageBtn = XUIMgr.Get(L"FRM_LOADING_HELP_BTN_BAR");
		if( pBriefingPageBtn )
		{
			XUI::CXUI_Button* pkNexBtn = dynamic_cast<XUI::CXUI_Button*>(pBriefingPageBtn->GetControl(L"BTN_NEXT"));
			XUI::CXUI_Button* pkPreBtn = dynamic_cast<XUI::CXUI_Button*>(pBriefingPageBtn->GetControl(L"BTN_PREV"));
			if( pkNexBtn && pkPreBtn )
			{
				int NexP = iProgress + 1;
				int PreP = iProgress - 1;
				KCONT_BRIEFING_TICK_ITEM::const_iterator Nex_iter = kInfo.kTickCont.find(NexP);
				KCONT_BRIEFING_TICK_ITEM::const_iterator Pre_iter = kInfo.kTickCont.find(PreP);

				int iNo = TT_NO_NEXT;
				int iLastPage = 0;
				if(Nex_iter == kInfo.kTickCont.end())
				{
					iNo = 76015;
					iLastPage = 1;

					pkNexBtn->TwinkleTime(UINT_MAX);
					pkNexBtn->TwinkleInterTime(330);
					pkNexBtn->SetTwinkle(true);
				}
				pkNexBtn->Text(TTW(iNo));
				pkNexBtn->SetCustomData(&iLastPage, sizeof(iLastPage));

				pkPreBtn->Disable(Pre_iter == kInfo.kTickCont.end());
			}
		}

		XUI::CXUI_Wnd* pkBriefing = XUIMgr.Activate(_T("FRM_BRIEFING_SCREEN"));
		if( pkBriefing )
		{
			XUI::CXUI_Wnd* pkTemp = pkBriefing->GetControl(_T("FRM_NUMBER"));
			if( pkTemp )
			{
				pkTemp->Text((std::wstring const&)BM::vstring(kTickInfo.iIdx));
			}

			pkTemp = pkBriefing->GetControl(_T("FRM_TITLE"));
			if( pkTemp )
			{
				pkTemp->Text(TTW(kTickInfo.iTitleTTW));
			}

			pkTemp = pkBriefing->GetControl(_T("FRM_CONTENT"));
			if( pkTemp )
			{
				pkTemp->Text(TTW(kTickInfo.iContentTTW));
			}

			pkTemp = pkBriefing->GetControl(_T("FRM_MINIMAP"));
			if( pkTemp )
			{
				pkTemp->DefaultImgName(kInfo.kImgPath);
				pkTemp->ImgSize(kInfo.kSize);
			}
		}

		KCONT_BRIEFING_POP_UI::const_iterator ui_iter = kTickInfo.kUIList.begin();
		while( ui_iter != kTickInfo.kUIList.end() )
		{
			XUI::CXUI_Wnd* pUITemp = g_kBriefingLoadUIMgr.GetOBJUI(ui_iter->kUIName, ui_iter->bCopy);
			if( pUITemp )
			{
				pUITemp->Location( GetDefaultLocation(ui_iter->kPos) );
			}
			++ui_iter;
		}
	}
	else
	{
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------//
//																									  //
//----------------------------------------------------------------------------------------------------//
PgBriefingLoadUIMgr::PgBriefingLoadUIMgr(void)
	: m_kAttacker(false)
	, m_kIdx(0)
{
}

PgBriefingLoadUIMgr::~PgBriefingLoadUIMgr(void)
{
	ClearAllOBJUI();
	m_kBriefingAtkCont.clear();
	m_kBriefingDefCont.clear();
}

bool PgBriefingLoadUIMgr::ParseXML(char const* pcXmlPath)
{
//	NILOG(PGLOG_LOG, "[PgEMBattleLoadMgr::ParseXml] Start\n");
	TiXmlDocument kXmlDoc(pcXmlPath);
	if( !PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)) )
	{
		return false;
	}

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	char const* pcTagName = pkElement->Value();
	if( strcmp(pcTagName, "BATTLE_MAP_LOAD_BRIEFING_TABLE") == 0 )
	{
		TiXmlElement const* pkSubElem = pkElement->FirstChildElement();
		
		while( pkSubElem )
		{
			char const* pcTagName = pkSubElem->Value();
			bool bResult = false;
			if( strcmp(pcTagName, "ATTACKER") == 0 )
			{
				bResult = RecursiveParseNode(pkSubElem->FirstChildElement(), &m_kBriefingAtkCont);
			}
			else
			{
				if( strcmp(pcTagName, "DEFENDER") == 0 )
				{
					bResult = RecursiveParseNode(pkSubElem->FirstChildElement(), &m_kBriefingDefCont);
				}
			}
			
			if( !bResult )
			{
				//error 파싱이 실패했거나 어디에도 해당하지 않는다
			}
			pkSubElem = pkSubElem->NextSiblingElement();
		}
		return true;
	}
	return false;
}

bool PgBriefingLoadUIMgr::RecursiveParseNode(TiXmlElement const* pkElement, KCONT_BRIEFING_MAP_INFO* pkMapCont, KCONT_BRIEFING_TICK_ITEM* pkTickCont, KCONT_BRIEFING_POP_UI* pkUICont)
{
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();
		if( strcmp(pcTagName, "BATTLE_MAP") == 0 )
		{
			int iMapNo = 0;
			SBriefingMapInfo	kMapInfo;
			_ParseMapAttr(pkElement->FirstAttribute(), kMapInfo);
			if( RecursiveParseNode(pkElement->FirstChildElement(), NULL, &kMapInfo.kTickCont) )
			{
				if( pkMapCont )
				{
					std::pair<KCONT_BRIEFING_MAP_INFO::iterator, bool> bResult = pkMapCont->insert(std::make_pair(kMapInfo.iMapNo, kMapInfo));
					if( !bResult.second ){ assert(0); }
				}
			}
		}
		else if( strcmp(pcTagName, "BRIEFING") == 0 )
		{
			SBriefingTickItem	kTickItem;
			_ParseTickAttr(pkElement->FirstAttribute(), kTickItem);
			if( RecursiveParseNode(pkElement->FirstChildElement(), NULL, NULL, &kTickItem.kUIList) )
			{
				if( pkTickCont )
				{
					std::pair<KCONT_BRIEFING_TICK_ITEM::iterator, bool> TickResult = pkTickCont->insert(std::make_pair(kTickItem.iTick, kTickItem));
					if( !TickResult.second ){ assert(0); }
				}
			}
		}
		else if( strcmp(pcTagName, "POP_UI") == 0 )
		{
			SBriefingPopUIInfo kUIInfo;
			_ParsePopUIAttr(pkElement->FirstAttribute(), kUIInfo);
			if( pkUICont )
			{
				pkUICont->push_back(kUIInfo);
			}
		}
		else
		{

		}

		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

bool PgBriefingLoadUIMgr::_ParseMapAttr(TiXmlAttribute const* pkAttr, SBriefingMapInfo& kMapInfo)
{
	while( pkAttr )
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();
		if( strcmp(pcAttrName, "MAPNO") == 0 )
		{
			kMapInfo.iMapNo = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "IMG_PATH") == 0 )
		{
			kMapInfo.kImgPath = UNI(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "IMG_W") == 0 )
		{
			kMapInfo.kSize.x = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "IMG_H") == 0 )
		{
			kMapInfo.kSize.y = atoi(pcAttrValue);
		}
		else
		{
			//??
		}
		pkAttr = pkAttr->Next();
	}
	return true;
}

bool PgBriefingLoadUIMgr::_ParseTickAttr(TiXmlAttribute const* pkAttr, SBriefingTickItem& kTickItem)
{
	while( pkAttr )
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if( strcmp(pcAttrName, "TICK") == 0 )
		{
			kTickItem.iTick = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "IDX") == 0 )
		{
			kTickItem.iIdx = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "TITLE_TTW") == 0 )
		{
			kTickItem.iTitleTTW = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "CONTENT_TTW") == 0 )
		{
			kTickItem.iContentTTW = atoi(pcAttrValue);
		}
		else
		{
			//??
		}
		pkAttr = pkAttr->Next();
	}
	return true;
}

bool PgBriefingLoadUIMgr::_ParsePopUIAttr(TiXmlAttribute const* pkAttr, SBriefingPopUIInfo& kUIInfo)
{
	while( pkAttr )
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if( strcmp(pcAttrName, "NAME") == 0 )
		{
			kUIInfo.kUIName = UNI(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "X") == 0 )
		{
			kUIInfo.kPos.x = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "Y") == 0 )
		{
			kUIInfo.kPos.y = atoi(pcAttrValue);
		}
		else if( strcmp(pcAttrName, "IS_COPY") == 0 )
		{
			kUIInfo.bCopy = ( atoi(pcAttrValue) != 0 )?(true):(false);
		}
		else
		{
			//??
		}
		pkAttr = pkAttr->Next();
	}
	return true;
}

bool PgBriefingLoadUIMgr::GetBriefingInfo(int const iMapNo, SBriefingMapInfo& kInfo)
{
	if( Attacker() )
	{
		KCONT_BRIEFING_MAP_INFO::iterator	iter = m_kBriefingAtkCont.find(iMapNo);
		if( iter != m_kBriefingAtkCont.end() )
		{
			kInfo = iter->second;
			return true;
		}
	}
	else
	{
		KCONT_BRIEFING_MAP_INFO::iterator	iter = m_kBriefingDefCont.find(iMapNo);
		if( iter != m_kBriefingDefCont.end() )
		{
			kInfo = iter->second;
			return true;
		}
	}
	return false;
}

void PgBriefingLoadUIMgr::ClearAllOBJUI()
{
	if( !m_kUICont.empty() )
	{
		KCONT_BRIEFING_CALLED_UI::iterator	iter = m_kUICont.begin();
		while( iter != m_kUICont.end() )
		{
			(*iter)->Close();
			iter = m_kUICont.erase(iter);
		}
	}
}

XUI::CXUI_Wnd* PgBriefingLoadUIMgr::GetOBJUI(std::wstring const& kUIName, bool const bIsCopyUI)
{
	if( !m_kUICont.empty() )
	{
		KCONT_BRIEFING_CALLED_UI::iterator	iter = m_kUICont.begin();
		while( iter != m_kUICont.end() )
		{
			if( (*iter)->IsClosed() )
			{
				iter = m_kUICont.erase(iter);
				continue;
			}
			++iter;
		}
	}

	if( bIsCopyUI )
	{
		if( m_kUICont.size() >= MAX_CALLED_UI_COUNT )
		{
			return NULL;
		}

		for(int i = 0; i < MAX_CALLED_UI_COUNT; ++i)
		{
			BM::vstring	vStr("FRM_BRIEFING_OBJ");
			vStr += i;

			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(vStr);
			if( !pWnd || pWnd->IsClosed() )
			{
				pWnd = XUIMgr.Activate(kUIName, false, vStr);
				if( !pWnd )
				{ 
					assert(0); 
					return NULL;
				}
			}
			else
			{
				continue;
			}

			KCONT_BRIEFING_CALLED_UI::iterator iter = m_kUICont.insert(m_kUICont.end(), pWnd);
			if( iter != m_kUICont.end() )
			{
				return pWnd;
			}
		}
	}
	else
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kUIName);
		if( !pWnd && pWnd->IsClosed() )
		{
			pWnd = XUIMgr.Activate(kUIName);
			if( !pWnd )
			{ 
				assert(0); 
				return NULL;
			}
		}
		return pWnd;
	}
	return NULL;
}