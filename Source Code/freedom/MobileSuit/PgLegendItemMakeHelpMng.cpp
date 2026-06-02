#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "ServerLib.h"
#include "lwUIQuest.h"
#include "PgQuestMan.h"
#include "PgChatMgrClient.h"
#include "lwUIbook.h"
#include "lwHelpSystem.h"

#include "PgLegendItemMakeHelpMng.h"

//
tagLegendItemHelpItem::tagLegendItemHelpItem()
	: iItemNo(0), iCount(0)
{
}
tagLegendItemHelpItem::tagLegendItemHelpItem(int const& riItemNo, int const& riCount)
	: iItemNo(riItemNo), iCount(riCount)
{
}
tagLegendItemHelpItem::tagLegendItemHelpItem(tagLegendItemHelpItem const& rhs)
	: iItemNo(rhs.iItemNo), iCount(rhs.iCount)
{
}
tagLegendItemHelpItem::~tagLegendItemHelpItem()
{
}
bool tagLegendItemHelpItem::ParseString(char const* szStr)
{
	if( NULL == szStr )
	{
		return false;
	}
	VEC_STRING kContStr;
	PgStringUtil::BreakSep(std::string(szStr), kContStr, "/");
	size_t const iRightCount = 2;
	if( iRightCount != kContStr.size() )
	{
		return false;
	}
	iItemNo = PgStringUtil::SafeAtoi(kContStr[0]);
	iCount = PgStringUtil::SafeAtoi(kContStr[1]);
	return true;
}

//
tagLegendItemMakeHelpStep::tagLegendItemMakeHelpStep()
	: iStepID(0), eType(LIMHST_TRADE), iExtVal(0), iExtVal2(0), kSrcItem1(), kSrcItem2(), kResultItem()
{
}
tagLegendItemMakeHelpStep::tagLegendItemMakeHelpStep(tagLegendItemMakeHelpStep const& rhs)
	: iStepID(rhs.iStepID), eType(rhs.eType), iExtVal(rhs.iExtVal), iExtVal2(rhs.iExtVal2), kSrcItem1(rhs.kSrcItem1), kSrcItem2(rhs.kSrcItem2), kResultItem(rhs.kResultItem)
{
}
tagLegendItemMakeHelpStep::~tagLegendItemMakeHelpStep()
{
}
bool tagLegendItemMakeHelpStep::ParseXml(TiXmlElement const* pkRoot)
{
	if( !pkRoot )
	{
		return false;
	}

	iStepID = PgStringUtil::SafeAtoi(pkRoot->Attribute("ID"));
	iExtVal = PgStringUtil::SafeAtoi(pkRoot->Attribute("EXT_VAL"));
	iExtVal2 = PgStringUtil::SafeAtoi(pkRoot->Attribute("EXT_VAL2"));
	char const* szType = pkRoot->Attribute("TYPE");
	if( 0 == strcmp(szType, "TRADE") )
	{
		eType = LIMHST_TRADE;
	}
	else if( 0 == strcmp(szType, "MAKE") )
	{
		eType = LIMHST_MAKE;
	}
	else
	{
		return false;
	}
	return kSrcItem1.ParseString(pkRoot->Attribute("SRC1"))
		&& kSrcItem2.ParseString(pkRoot->Attribute("SRC2"))
		&& kResultItem.ParseString(pkRoot->Attribute("RESULT"));
}

//
tagLegendItemMakeHelpInfo::tagLegendItemMakeHelpInfo()
	: iItemNo(0), iDescTextID(0), iTitleTextID(0), kContStep()
{
}
tagLegendItemMakeHelpInfo::tagLegendItemMakeHelpInfo(tagLegendItemMakeHelpInfo const& rhs)
	: iItemNo(rhs.iItemNo), iDescTextID(rhs.iDescTextID), iTitleTextID(rhs.iTitleTextID), kContStep(rhs.kContStep)
{
}
tagLegendItemMakeHelpInfo::~tagLegendItemMakeHelpInfo()
{
}
bool tagLegendItemMakeHelpInfo::ParseXml(TiXmlElement const* pkRoot)
{
	if( !pkRoot )
	{
		return false;
	}

	iItemNo = PgStringUtil::SafeAtoi(pkRoot->Attribute("NO"));
	iDescTextID = PgStringUtil::SafeAtoi(pkRoot->Attribute("DESC_TT"));
	iTitleTextID = PgStringUtil::SafeAtoi(pkRoot->Attribute("TITLE_TT"));

	bool bRet = true;
	TiXmlElement const* pkChild = pkRoot->FirstChildElement();
	while( pkChild )
	{
		char const* szName = pkChild->Value();
		char const* szValue = pkChild->GetText();
		if( 0 == strcmp("STEP", szName) )
		{
			CONT_LEGEND_ITEM_MAKE_HELP_STEP::mapped_type kNew;
			bRet = kNew.ParseXml(pkChild) && bRet;
			kContStep.insert(std::make_pair(kNew.iStepID,kNew));
		}
		else
		{
			bRet = false;
		}
		pkChild = pkChild->NextSiblingElement();
	}
	return bRet;
}

//
PgLegendItemMakeHelpMng::PgLegendItemMakeHelpMng()
{
}
PgLegendItemMakeHelpMng::~PgLegendItemMakeHelpMng()
{
}
bool PgLegendItemMakeHelpMng::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	if( !pkNode )
	{
		return false;
	}

	char const* szName = pkNode->Value();
	if( 0 != strcmp(XML_ELEMENT_LEGEND_ITEM_MAKE_HELP, szName) )
	{
		return false;
	}

	bool bRet = true;
	TiXmlElement const* pkSubNode = pkNode->FirstChildElement();
	while( pkSubNode )
	{
		char const* szName = pkSubNode->Value();
		if( 0 == strcmp(szName, "ITEM") )
		{
			CONT_LEGEND_ITEM_MAKE_HELP::mapped_type kNew;
			bRet = kNew.ParseXml(pkSubNode) && bRet;
			m_kContMakeHelp.insert( std::make_pair(kNew.iItemNo, kNew) );
		}
		else
		{
			bRet = false;
		}
		pkSubNode = pkSubNode->NextSiblingElement();
	}
	return bRet;
}

int PgLegendItemMakeHelpMng::GetStepSize(const int iItemNo)
{
	CONT_LEGEND_ITEM_MAKE_HELP::iterator find_itor = m_kContMakeHelp.find(iItemNo);
	
	if( find_itor != m_kContMakeHelp.end() )
	{
		CONT_LEGEND_ITEM_MAKE_HELP::mapped_type &rkElement = (*find_itor).second;
		return rkElement.kContStep.size();
	}

	return 0;
}

void PgLegendItemMakeHelpMng::SetMakeHelpUIData(XUI::CXUI_Wnd* pkTopWnd, const CONT_LEGEND_ITEM_MAKE_HELP_STEP::mapped_type& rkCont)
{
	if( !pkTopWnd )
	{
		return ;
	}

	// 단계
	XUI::CXUI_Wnd* pkStepWnd = pkTopWnd->GetControl(_T("FRM_STEP_TEXT"));
	if( pkStepWnd )
	{
		std::wstring kTemp;
		if( FormatTTW( kTemp, 362, rkCont.iStepID ) )
		{
			pkStepWnd->Text(kTemp);
		}
	}

	// 교환? 제조?
	XUI::CXUI_Button* pkHowToBtn = dynamic_cast<XUI::CXUI_Button*>(pkTopWnd->GetControl(_T("BTN_HOWTO")));
	XUI::CXUI_Wnd* pkHowToWnd = pkTopWnd->GetControl(_T("FRM_HOWTO_TEXT"));
	switch( rkCont.eType )
	{
	case LIMHST_TRADE:
		{
			if( pkHowToBtn ) { pkHowToBtn->Text(TTW(799117)); }
			if( pkHowToWnd ) { pkHowToWnd->Text(TTW(70050)); }
		}break;
	case LIMHST_MAKE:
		{
			if( pkHowToBtn ) { pkHowToBtn->Text(TTW(799118)); }
			if( pkHowToWnd ) { pkHowToWnd->Text(TTW(70051)); }			
		}break;
	default:
		{
		}break;
	}

	// 아이템 아이콘
	XUI::CXUI_Wnd* pkItemWnd1 = pkTopWnd->GetControl(_T("IMG_ITEM01"));
	XUI::CXUI_Wnd* pkItemWnd2 = pkTopWnd->GetControl(_T("IMG_ITEM02"));
	XUI::CXUI_Wnd* pkItemWnd3 = pkTopWnd->GetControl(_T("IMG_ITEM03"));

	if( pkItemWnd1 && pkItemWnd2 && pkItemWnd3 )
	{
		pkItemWnd1->SetCustomData( &rkCont.kSrcItem1.iItemNo, sizeof(rkCont.kSrcItem1.iItemNo) );
		pkItemWnd2->SetCustomData( &rkCont.kSrcItem2.iItemNo, sizeof(rkCont.kSrcItem2.iItemNo) );
		pkItemWnd3->SetCustomData( &rkCont.kResultItem.iItemNo, sizeof(rkCont.kResultItem.iItemNo) );

		SoulCraft::SetSimpleItemIcon(pkItemWnd1, rkCont.kSrcItem1.iItemNo);
		SoulCraft::SetSimpleItemIcon(pkItemWnd2, rkCont.kSrcItem2.iItemNo);
		SoulCraft::SetSimpleItemIcon(pkItemWnd3, rkCont.kResultItem.iItemNo);
	}
	
	XUI::CXUI_Wnd* pkItemMax1 = pkTopWnd->GetControl(_T("FRM_ITEM01_MAX"));
	XUI::CXUI_Wnd* pkItemMax2 = pkTopWnd->GetControl(_T("FRM_ITEM02_MAX"));
	XUI::CXUI_Wnd* pkItemMax3 = pkTopWnd->GetControl(_T("FRM_ITEM03_MAX"));

	if( pkItemMax1 && pkItemMax2 && pkItemMax3 )
	{
		BM::vstring vStr;

		vStr = rkCont.kSrcItem1.iCount;
		pkItemMax1->Text(vStr);

		vStr = rkCont.kSrcItem2.iCount;
		pkItemMax2->Text(vStr);

		vStr = rkCont.kResultItem.iCount;
		pkItemMax3->Text(vStr);
	}
	
	{// 현재 소지한 아이템 개수
		XUI::CXUI_Wnd* pkItemCur1 = pkTopWnd->GetControl(_T("FRM_ITEM01_CUR"));
		XUI::CXUI_Wnd* pkItemCur2 = pkTopWnd->GetControl(_T("FRM_ITEM02_CUR"));
		XUI::CXUI_Wnd* pkItemCur3 = pkTopWnd->GetControl(_T("FRM_ITEM03_CUR"));
		
		if( pkItemCur1 && pkItemCur2 && pkItemCur3 )
		{
			pkItemCur1->SetCustomData( &rkCont.kSrcItem1.iItemNo, sizeof(rkCont.kSrcItem1.iItemNo) );
			pkItemCur2->SetCustomData( &rkCont.kSrcItem2.iItemNo, sizeof(rkCont.kSrcItem2.iItemNo) );
			pkItemCur3->SetCustomData( &rkCont.kResultItem.iItemNo, sizeof(rkCont.kResultItem.iItemNo) );
		}
	}
}

void PgLegendItemMakeHelpMng::UpdateItemCount(lwUIWnd kWnd, const int iItemNo)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if(!pkWnd) 
	{
		return ;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return ;
	}
	
	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}
	
	BM::vstring vStr( pkInv->GetTotalCount(iItemNo, true) );
	pkWnd->Text(vStr);
}

bool PgLegendItemMakeHelpMng::CallMakeHelpUI(const int iItemNo)
{
	std::wstring const kCallUIName(_T("FRM_HELP_SUPER_GROUND_ITEM"));
	CONT_LEGEND_ITEM_MAKE_HELP::iterator find_itor = m_kContMakeHelp.find(iItemNo);
	if( find_itor == m_kContMakeHelp.end() )
	{
		XUIMgr.Close(kCallUIName);
		return false;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(kCallUIName);
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(kCallUIName); 
		if( !pkTopWnd )
		{
			return false;
		}
	}

	CONT_LEGEND_ITEM_MAKE_HELP::mapped_type &rkElement = (*find_itor).second;
	
	XUI::CXUI_Wnd* pkItemInfoWnd = pkTopWnd->GetControl(_T("SFRM_ITEM_INFO"));
	if( pkItemInfoWnd )
	{
		XUI::CXUI_Wnd* pkItemWnd = pkItemInfoWnd->GetControl(_T("SFRM_ITEM"));
		if( pkItemWnd )
		{
			XUI::CXUI_Icon* pkItemIcon = dynamic_cast<XUI::CXUI_Icon*>(pkItemWnd->GetControl(_T("ICON_ITEM")));
			if( pkItemIcon )
			{
				pkItemIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
			}
		}

		// 아이템 이름
		XUI::CXUI_Wnd* pkItemName = pkItemInfoWnd->GetControl(_T("FRM_ITEM_TEXT"));
		if( pkItemName )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
			if( !pItemDef )
			{
				return false;
			}

			const wchar_t *pName = NULL;
			if( GetDefString(pItemDef->NameNo(), pName) )
			{
				std::wstring kItemName(pName);
				PgStringUtil::EraseStr(kItemName, std::wstring(L"\n"), kItemName);
				pkItemName->Text(kItemName);
			}
		}

		XUI::CXUI_Wnd* pkItemDesc = pkItemInfoWnd->GetControl(_T("SFRM_ITEM_DESC"));
		if( pkItemDesc )
		{
			pkItemDesc->Text(TTW(rkElement.iDescTextID));
		}

		XUI::CXUI_Wnd* pkDescBtn = dynamic_cast< XUI::CXUI_Button* >(pkItemInfoWnd->GetControl(std::wstring(L"BTN_DETAIL_DESC")));
		if( pkDescBtn )
		{
			pkDescBtn->SetCustomData( &rkElement.iItemNo, sizeof(rkElement.iItemNo) );
		}
	}

	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopWnd->GetControl(_T("LIST_SUPER_ITEM")));
	if( !pkListWnd )
	{
		return false;
	}

	pkListWnd->ClearList();
	
	CONT_LEGEND_ITEM_MAKE_HELP_STEP::iterator loop_itor = rkElement.kContStep.begin();
	while( loop_itor != rkElement.kContStep.end() )
	{
		CONT_LEGEND_ITEM_MAKE_HELP_STEP::mapped_type const &rkHelpStep = (*loop_itor).second;

		XUI::SListItem* pkNewItem = pkListWnd->AddItem( std::wstring() );
		if( pkNewItem )
		{
			XUI::CXUI_Wnd* pkItemWnd = pkNewItem->m_pWnd;
			if( pkItemWnd )
			{
				SetMakeHelpUIData( pkItemWnd, rkHelpStep );
				BM::Stream kPacket;
				kPacket.Push( rkHelpStep.eType );
				kPacket.Push( rkHelpStep.iExtVal );
				kPacket.Push( rkHelpStep.iExtVal2 );
				pkItemWnd->SetCustomData( kPacket.Data() );
			}
		}

		++loop_itor;
	}
	return true;
}

void PgLegendItemMakeHelpMng::Clear()
{
	m_kContMakeHelp.clear();
}
bool PgLegendItemMakeHelpMng::CallMakeHelpUI(const int iItemNo, int iStepNo)
{
	CONT_LEGEND_ITEM_MAKE_HELP::iterator find_itor = m_kContMakeHelp.find(iItemNo);
	if( find_itor == m_kContMakeHelp.end() )
	{
		return false;
	}
	CONT_LEGEND_ITEM_MAKE_HELP::mapped_type const& rkElement = (*find_itor).second;
	if( rkElement.kContStep.empty() )
	{
		return false;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_IN_SUPER_GROUND_ITEM"));
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(_T("FRM_IN_SUPER_GROUND_ITEM"));
	}

	// 아이템 이름
	XUI::CXUI_Wnd* pkItemName = pkTopWnd->GetControl(_T("FRM_ITEM_TEXT"));
	if( pkItemName )
	{
		std::wstring kItemTitleName;
		if( 0 < rkElement.iTitleTextID )
		{
			kItemTitleName = TTW(rkElement.iTitleTextID);
		}
		else
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
			if( !pItemDef )
			{
				return false;
			}

			const wchar_t *pName = NULL;
			if( GetDefString(pItemDef->NameNo(), pName) )
			{
				kItemTitleName = pName;
			}
		}

		PgStringUtil::EraseStr(kItemTitleName, std::wstring(L"\n"), kItemTitleName);
		Quest::SetCutedTextLimitLength(pkItemName, kItemTitleName, _T("..."));
		pkItemName->SetCustomData(&iItemNo, sizeof(iItemNo));
	}

	CONT_LEGEND_ITEM_MAKE_HELP_STEP::const_iterator loop_itor = rkElement.kContStep.find(iStepNo);
	if( rkElement.kContStep.end() == rkElement.kContStep.find(iStepNo) )
	{
		if( 0 >= iStepNo )
		{
			loop_itor = --rkElement.kContStep.end();
		}
		else
		{
			loop_itor = rkElement.kContStep.begin();
		}
	}
	{
		CONT_LEGEND_ITEM_MAKE_HELP_STEP::mapped_type const &rkHelpStep = (*loop_itor).second;
		SetMakeHelpUIData( pkTopWnd, rkHelpStep );
		BM::Stream kPacket;
		kPacket.Push( iItemNo );
		kPacket.Push( rkHelpStep.iStepID );
		pkTopWnd->SetCustomData( kPacket.Data() );
	}
	return true;
}

void PgLegendItemMakeHelpMng::lwCallMakeHelpBigUI(const int iItemNo)
{
	g_kLegendItemMakeHelpMng.CallMakeHelpUI(iItemNo);
}

void PgLegendItemMakeHelpMng::lwCallMakeHelpSmallUI(const int iItemNo, const int iStepNo)
{
	g_kLegendItemMakeHelpMng.CallMakeHelpUI(iItemNo, iStepNo);
}

void PgLegendItemMakeHelpMng::lwUpdateItemCount(lwUIWnd kWnd, const int iItemNo)
{	
	g_kLegendItemMakeHelpMng.UpdateItemCount(kWnd,iItemNo);
}

void PgLegendItemMakeHelpMng::lwCallHowToCurrentStep(lwUIWnd kTop)
{
	XUI::CXUI_Wnd* pkTopWnd = kTop();
	if( pkTopWnd )
	{
		BM::Stream kPacket;
		pkTopWnd->GetCustomData(kPacket.Data());
		ELegendItemMakeHelpStepType eType = LIMHST_TRADE;
		int iExtVal = 0, iExtVal2 = 0;
		kPacket.PosAdjust();
		kPacket.Pop( eType );
		kPacket.Pop( iExtVal );
		kPacket.Pop( iExtVal2 );
		switch( eType )
		{
		case LIMHST_TRADE:
			{
				if( 0 < iExtVal )
				{
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if( pkPlayer )
					{
						PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
						if( pkMyQuest )
						{
							if( pkMyQuest->IsIngQuest(iExtVal) )
							{
								Quest::lwShow_IngQuestInterface(iExtVal);
							}
							else
							{
								if( 0 < iExtVal2 )
								{
									g_kChatMgrClient.LogMsgBox(iExtVal2);
								}
								else
								{
									PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iExtVal);
									if( pkQuestInfo )
									{
										std::wstring kTempSTr = TTW(799197);
										PgStringUtil::ReplaceStr(kTempSTr, std::wstring(L"$QUEST_NAME$"), TTW(pkQuestInfo->m_iTitleTextNo), kTempSTr);
										g_kChatMgrClient.LogMsgBox(kTempSTr);
									}
								}
							}
						}
					}
				}
			}break;
		case LIMHST_MAKE:
			{
				int const iHelpID = 11;
				OnClickDetailHelpView(iHelpID, true);
			}break;
		default:
			{
			}break;
		}
	}
}

void PgLegendItemMakeHelpMng::lwCallMakeHelpDetailDesc(lwUIWnd kWnd)
{
	lwUIBook::iForceRecommandItemNo = kWnd.GetCustomData<int>();
	lua_tinker::call< void, char const* >("DirectOpenUI", "OPEN_BOOK_RECOMMAND_ITEM");
}

void PgLegendItemMakeHelpMng::RegisterWrapper(lua_State* pkState)
{
	using namespace lua_tinker;
	def(pkState, "CallMakeHelpBigUI", lwCallMakeHelpBigUI);
	def(pkState, "CallMakeHelpSmallUI", lwCallMakeHelpSmallUI);
	def(pkState, "UpdateItemCount", lwUpdateItemCount);
	def(pkState, "CallMakeHelpHowToCurrentStep", lwCallHowToCurrentStep);
	def(pkState, "CallMakeHelpDetailDesc", lwCallMakeHelpDetailDesc);
}