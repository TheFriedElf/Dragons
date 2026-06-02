#include "StdAfx.h"
#include "lwUI.h"
#include "variant/PgQuestInfo.h"
#include "PgQuestMan.h"
#include "lwQuestNfy.h"
#include "PgPilotMan.h"
#include "PgWorldMapUI.h"
#include "PgResourceIcon.h"
#include "PgWorldMapPopUpUI.h"
#include "lwUIBook.h"
#include "PgOption.h"

extern bool lwIsEndedQuest(unsigned int sQuestID);
extern bool lwIsIngQuest(unsigned short sQuestID);

bool SetIconInfo(XUI::CXUI_Wnd* pkWnd, SResourceIcon const& rkRscIcon, SIconImage const& rkIconImage, int const iNo);
int const IGNORE_NOTIFY_QUEST_ID_1 = 901;
int const IGNORE_NOTIFY_QUEST_ID_2 = 902;
int const IGNORE_NOTIFY_QUEST_ID_3 = 903;
namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}
namespace lwQuestNfy
{
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CheckNewQuestNfy", lwQuestNfy::lwCheckNewQuestNfy);
		def(pkState, "CallQuestNfyUI", lwQuestNfy::lwCallQuestNfyUI);
		def(pkState, "OnOverQuestNfyListItem", lwQuestNfy::lwOnOverQuestNfyListItem);
		def(pkState, "OnClickQuestNfyListItem", lwQuestNfy::lwOnClickQuestNfyListItem);
		def(pkState, "OnClickDetailQuestView", lwQuestNfy::lwOnClickDetailQuestView);
		def(pkState, "OnTickChangeTextToQuestNameAndQuestGroup", lwQuestNfy::lwOnTickChangeTextToQuestNameAndQuestGroup);
		def(pkState, "OnTickCheckQuestModify", lwQuestNfy::lwOnTickCheckQuestModify);
		def(pkState, "CheckRemoteAcceptQuest", lwQuestNfy::CheckRemoteAcceptQuest);
		def(pkState, "CallToolTip_NearQuestIcon", lwQuestNfy::CallToolTip_NearQuestIcon);

		return true;
	}

	void lwCheckNewQuestNfy(int const CallType, int const iQuestID)
	{
		ContQuestInfo	kQuestList;
		g_kQuestMan.GetQuest(kQuestList, QT_None);

		switch( CallType )
		{
		case EQNCT_LEVELUP:
			{//레벨업한거다
				if( !CheckNewQuestToLevelUP(kQuestList) )
				{
					return;
				}
			}break;
		case EQNCT_QUEST_CLEAR:
			{//퀘스트를 클리어 한거다
				if( !CheckNewQuestToQuestClear(iQuestID, kQuestList) )
				{
					return;
				}
			}break;
		case EQNCT_ALL:
		default:
			{//모든 퀘스트 검사
				if( !CheckNewQuestAll(kQuestList) )
				{
					return;
				}
			}break;
		}

		bool bIsScenario = false;
		bool bIsNormal = false;
		ContQuestInfo::iterator	quest_itor = kQuestList.begin();
		while( quest_itor != kQuestList.end() )
		{
			ContQuestInfo::mapped_type const& pkQuestInfo = quest_itor->second;

			if( QT_Scenario == pkQuestInfo->Type() )
			{
				bIsScenario = true;
			}
			else
			{
				bIsNormal = true;
			}
			++quest_itor;
		}

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Activate(L"FRM_NEW_QUEST_NOTICE");
		if( pMainUI )
		{
			pMainUI->SetCustomData(&CallType, sizeof(CallType));
			XUI::CXUI_Builder* pBuilder = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_BTN"));
			if( pBuilder )
			{
				int const iTotalIDX = pBuilder->CountX() * pBuilder->CountY();
				for(int i = 0; i < iTotalIDX; ++i)
				{
					BM::vstring	vStr(L"FRM_ITEM");
					vStr += i;
					XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
					if( pItem )
					{
						pItem->SetCustomData(&iQuestID, sizeof(iQuestID));
						if( ( i == 0 && bIsScenario ) || ( i == 1 && bIsNormal ) )
						{
							pItem->Visible(true);
						}
						else
						{
							pItem->Visible(false);
						}
					}
				}
			}
		}
	}

	void lwCallQuestNfyUI(int const CallType, int const QuestID, bool const bIsScenario)
	{
		ContQuestInfo	kQuestList;

		g_kQuestMan.GetQuest(kQuestList, (bIsScenario)?(QT_Scenario):(QT_None));

		switch( CallType )
		{
		case EQNCT_LEVELUP:
			{//레벨업한거다
				if( !CheckNewQuestToLevelUP(kQuestList) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 401061, true);
					return;
				}
			}break;
		case EQNCT_QUEST_CLEAR:
			{//퀘스트를 클리어 한거다
				if( !CheckNewQuestToQuestClear(QuestID, kQuestList) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 401061, true);
					return;
				}
			}break;
		case EQNCT_ALL:
		default:
			{//퀘스트 전체
				if( !CheckNewQuestAll(kQuestList) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 401061, true);
					return;
				}
			}break;
		}

		if( !bIsScenario )
		{
			ContQuestInfo::iterator	quest_itor = kQuestList.begin();
			while( quest_itor != kQuestList.end() )
			{
				ContQuestInfo::mapped_type const& pkQuestInfo = quest_itor->second;

				if( QT_Scenario == pkQuestInfo->Type() )
				{
					quest_itor = kQuestList.erase(quest_itor);
					continue;
				}
				++quest_itor;
			}
		}

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_NEW_QUEST");
		if( pMainUI )
		{
			pMainUI->SetCustomData(&bIsScenario, sizeof(bIsScenario));
			XUI::CXUI_List* pUIList = dynamic_cast<XUI::CXUI_List*>(pMainUI->GetControl(L"QUEST_LIST"));
			if( !pUIList )
			{
				return;
			}
			
			CONT_DEFMAP const * pContDefMap = NULL;
			g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
			if( NULL == pContDefMap )
			{
				return;
			}
			XUIListUtil::SetMaxItemCount(pUIList, kQuestList.size());
			XUI::SListItem*	pListItem = pUIList->FirstItem();
			ContQuestInfo::iterator	quest_itor = kQuestList.begin();
			while( quest_itor != kQuestList.end() )
			{
				ContQuestInfo::key_type const& iQuestID = quest_itor->first;
				ContQuestInfo::mapped_type const& pkQuestInfo = quest_itor->second;

				if( pListItem && pListItem->m_pWnd )
				{
					XUI::CXUI_Wnd* pkQuestName = pListItem->m_pWnd->GetControl(L"FRM_NAME");
					XUI::CXUI_Wnd* pkGroupName = pListItem->m_pWnd->GetControl(L"FRM_GROUP_NAME");
					if( pkQuestName && pkGroupName )
					{
						float fNowTime = (g_pkApp != NULL)?(g_pkApp->GetAccumTime()):(0);
						pkQuestName->Visible(true);
						pkGroupName->Visible(false);
						Quest::SetCutedTextLimitLength(pkQuestName, TTW(pkQuestInfo->m_iTitleTextNo), L"...",140);
						pkQuestName->SetCustomData(&fNowTime, sizeof(fNowTime));
						pkGroupName->Text(L"");
						pkGroupName->ClearCustomData();
						ContImportanceQuest const& QuestList = g_kQuestMan.GetImportanceQuestList();
						if( !QuestList.empty() )
						{
							ContImportanceQuest::const_iterator group_itor = QuestList.begin();
							while(QuestList.end() != group_itor)
							{
								ContImportanceQuest::mapped_type const& kGroupInfo = (*group_itor).second;

								if( kGroupInfo.Find( iQuestID ) )
								{
									Quest::SetCutedTextLimitLength(pkGroupName, TTW(kGroupInfo.iGroupNameNo), L"...");
									pkGroupName->SetCustomData(&kGroupInfo.iGroupNameNo, sizeof(kGroupInfo.iGroupNameNo));
									break;
								}
								++group_itor;
							}
						}
					}
					pListItem->m_pWnd->SetCustomData(&iQuestID, sizeof(iQuestID));

					XUI::CXUI_Wnd* pUICheck = pListItem->m_pWnd->GetControl(L"FRM_CHECK");
					if( pUICheck )
					{
						pUICheck->UVUpdate(1);
						XUI::CXUI_Wnd* pUICheckIcn = pUICheck->GetControl(L"FRM_CHECK_ICON");
						if( NULL != pUICheckIcn )
						{
							pUICheckIcn->Visible(false);
						}
					}

					XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pListItem->m_pWnd->GetControl(L"ICON_NPC"));
					if( pIcon )
					{
						pIcon->Visible(false);
						if (!pkQuestInfo->m_kNpc_Client.empty())
						{
							SQuestNpc const& rkNPC = (*pkQuestInfo->m_kNpc_Client.begin());
							SNpcCreateInfo kInfo;
							if (g_kWorldMapUI.GetNpc(rkNPC.kNpcGuid, kInfo))
							{
								SResourceIcon kRscIcon;
								SIconImage kIconImage;
								if (PgWorldMapPopUpUI::FindNPCFromActorName(kRscIcon, kIconImage, kInfo.wstrActor))
								{
									SetIconInfo(pIcon, kRscIcon, kIconImage, kInfo.iID);
									pIcon->Visible(true);
								}
							}
						}
					}
					XUI::CXUI_Wnd* pkRemoteAccept = pListItem->m_pWnd->GetControl(L"BTN_REMOTE_ACCEPT");
					if( NULL != pkRemoteAccept )
					{
						pkRemoteAccept->GrayScale( !pkQuestInfo->IsCanRemoteAccept() );
					}
					XUI::CXUI_Wnd* pkIsNearQuest = pListItem->m_pWnd->GetControl(L"ICON_NEAR_QUEST");
					if( NULL != pkIsNearQuest )
					{
						bool bGray = true;
						if( NULL != g_pkWorld )
						{
							int const iMapNo = g_pkWorld->MapNo();
							const TBL_DEF_MAP *pkDefMap = PgWorld::GetMapData(iMapNo);
							if ( pkDefMap )
							{
								int const iDefGroupTextNo = pkDefMap->iGroupTextNo;
								int const iQuestGroupTextNo = pkQuestInfo->GroupTextNo();
								if( iQuestGroupTextNo == iDefGroupTextNo )
								{
									bGray = false;
									pkIsNearQuest->SetCustomData( &iMapNo, sizeof(iMapNo) );
								}
								else
								{
									CONT_DEFMAP::const_iterator map_itor = pContDefMap->begin();
									while( pContDefMap->end() != map_itor )
									{
										if( map_itor->second.iGroupTextNo == iQuestGroupTextNo )
										{
											int const iMapNo = map_itor->second.iMapNo;
											pkIsNearQuest->SetCustomData( &iMapNo, sizeof( iMapNo ) );
											break;
										}
										++map_itor;
									}
									bGray = true;
								}
							}
						}
						pkIsNearQuest->GrayScale(bGray);
					}
					pListItem = pUIList->NextItem(pListItem);
				}
				++quest_itor;
			}
		}
	}

	void lwOnOverQuestNfyListItem(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pSelf = kWnd.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
		if( pList )
		{
			XUI::SListItem*	pListItem = pList->FirstItem();
			while( pListItem && pListItem->m_pWnd )
			{
				XUI::CXUI_Wnd* pUICheck = pListItem->m_pWnd->GetControl(L"FRM_CHECK");
				if( pUICheck && pUICheck->UVInfo().Index != 3)
				{
					pUICheck->UVUpdate(1);
					XUI::CXUI_Wnd* pUICheckIcn = pUICheck->GetControl(L"FRM_CHECK_ICON");
					if( NULL != pUICheckIcn )
					{
						pUICheckIcn->Visible(false);
					}
				}
				pListItem = pList->NextItem(pListItem);
			}
		}

		XUI::CXUI_Wnd* pUICheck = pSelf->GetControl(L"FRM_CHECK");
		if( pUICheck && pUICheck->UVInfo().Index != 3 )
		{
			pUICheck->UVUpdate(2);
		}
	}

	void lwOnClickQuestNfyListItem(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pSelf = kWnd.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
		if( pList )
		{
			XUI::SListItem*	pListItem = pList->FirstItem();
			while( pListItem && pListItem->m_pWnd )
			{
				XUI::CXUI_Wnd* pUICheck = pListItem->m_pWnd->GetControl(L"FRM_CHECK");
				if( pUICheck )
				{
					pUICheck->UVUpdate(1);
					XUI::CXUI_Wnd* pUICheckIcn = pUICheck->GetControl(L"FRM_CHECK_ICON");
					if( NULL != pUICheckIcn )
					{
						pUICheckIcn->Visible(false);
					}
				}
				pListItem = pList->NextItem(pListItem);
			}
		}

		XUI::CXUI_Wnd* pUICheck = pSelf->GetControl(L"FRM_CHECK");
		if( pUICheck )
		{
			pUICheck->UVUpdate(3);
			XUI::CXUI_Wnd* pUICheckIcn = pUICheck->GetControl(L"FRM_CHECK_ICON");
			if( NULL != pUICheckIcn )
			{
				pUICheckIcn->Visible(true);
			}
		}

		int iQuestID = 0;
		pSelf->GetCustomData(&iQuestID, sizeof(iQuestID));

		XUI::CXUI_Wnd* pMainUI = pList->Parent();
		if( pMainUI )
		{
			XUI::CXUI_Wnd* pDetail = pMainUI->GetControl(L"BTN_DETAIL");
			if( pDetail )
			{
				pDetail->SetCustomData(&iQuestID, sizeof(iQuestID));
			}
		}
	}

	void lwOnClickDetailQuestView(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pSelf = kWnd.GetSelf();
		if( !pSelf )
		{
			return;
		}

		int iQuestID = 0;
		pSelf->GetCustomData(&iQuestID, sizeof(iQuestID));

		while( pSelf->Parent() )
		{
			pSelf = pSelf->Parent();
		}
		pSelf->Close();

		lwUIWnd kBookUI = lua_tinker::call<lwUIWnd, int>("OpenBookPage", 3);
		if(kBookUI.IsNil())
		{
			return;
		}
		XUI::CXUI_Wnd* pkBookUI = kBookUI();
		if( !pkBookUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pkQuestPage = pkBookUI->GetControl(L"CBTN_TAB3");
		if( !pkQuestPage )
		{
			return;
		}

		XUI::CXUI_Wnd* pkPage = pkBookUI->GetControl(L"FRM_PAGE2");
		if( !pkPage )
		{
			return;
		}

		XUI::CXUI_List* pkQuestList = dynamic_cast<XUI::CXUI_List*>(pkPage->GetControl(L"LST_QUEST"));
		if( !pkQuestList )
		{
			return;
		}

		lua_tinker::call<void, lwUIWnd>("OnClickQuestViewListChange", 
			lwUIWnd(pkPage->GetControl(L"BTN_QUEST_TYPE_CHANGE1")));

		XUI::SListItem* pkListItem = pkQuestList->FirstItem();
		int idx = 0;
		while( pkListItem && pkListItem->m_pWnd )
		{
			int iFindID = 0;
			XUI::CXUI_Wnd* pkQuest =  pkListItem->m_pWnd->GetControl(L"BTN_QUEST");
			if( pkQuest )
			{
				pkQuest->GetCustomData(&iFindID, sizeof(iFindID));
				if( iQuestID == iFindID)
				{
					lwUIBook::OnClickBookQuest( lwUIWnd(pkQuest) );
					pkQuestList->DisplayStartPos(pkListItem->m_pWnd->Height()*idx);
					return;
				}
			}
			pkListItem = pkQuestList->NextItem(pkListItem);
			++idx;
		}

	}

	void lwOnTickChangeTextToQuestNameAndQuestGroup(lwUIWnd kItemWnd, float fTickTime)
	{
		XUI::CXUI_Wnd* pSelf = kItemWnd.GetSelf();
		if( pSelf )
		{
			XUI::CXUI_Wnd* pkGroupName = pSelf->GetControl(L"FRM_GROUP_NAME");
			if( pkGroupName )
			{
				int iGroupNameNo = 0;
				pkGroupName->GetCustomData(&iGroupNameNo, sizeof(iGroupNameNo));
				if( iGroupNameNo == 0 )
				{
					return;
				}

				XUI::CXUI_Wnd* pkQuestName = pSelf->GetControl(L"FRM_NAME");
				if( !pkQuestName )
				{
					return;
				}

				float fPrevTime = 0;
				pkQuestName->GetCustomData(&fPrevTime, sizeof(fPrevTime));

				float const fNowTime = (g_pkApp != NULL)?(g_pkApp->GetAccumTime()):(0);
				if( 0 == fTickTime || ((fNowTime - fPrevTime) < fTickTime) )
				{
					return;
				}
				pkQuestName->SetCustomData(&fNowTime, sizeof(fNowTime));
				pkQuestName->Visible(!pkQuestName->Visible());
				pkGroupName->Visible(!pkQuestName->Visible());
			}
		}
	}

	bool lwOnTickCheckQuestModify(int const CallType, int const QuestID, bool const bIsScenario)
	{
		ContQuestInfo	kQuestList;
		g_kQuestMan.GetQuest(kQuestList, (bIsScenario)?(QT_Scenario):(QT_None));

		switch( CallType )
		{
		case EQNCT_LEVELUP:
			{//레벨업한거다
				return  CheckNewQuestToLevelUP(kQuestList);
			}break;
		case EQNCT_QUEST_CLEAR:
			{//퀘스트를 클리어 한거다
				 return CheckNewQuestToQuestClear(QuestID, kQuestList);
			}break;
		case EQNCT_ALL:
		default:
			{//퀘스트 전체
				 return CheckNewQuestAll(kQuestList);
			}break;
		}
		return false;
	}

	bool CheckNewQuestToLevelUP(ContQuestInfo& kQuestList)
	{
		PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		ContQuestInfo::iterator	iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			if( QBL_None != PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPlayer), kQuestID, pkQuestInfo) )
			{
				iter = kQuestList.erase(iter);
				continue;
			}
			if(	   IGNORE_NOTIFY_QUEST_ID_1 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_2 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_3 == (*iter).first )
			{
				kQuestList.erase(iter++);
				continue;
			}
			++iter;
		}

		iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			if( pkQuestInfo->m_kLimit.iMinLevel < pkPlayer->GetAbil(AT_LEVEL) )
			{
				iter = kQuestList.erase(iter);
				continue;
			}
			++iter;
		}

		return !kQuestList.empty();
	}

	bool CheckNewQuestToQuestClear(int const ClearQuestID, ContQuestInfo& kQuestList)
	{
		PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		ContQuestInfo::iterator	iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			bool bIsLive = true;

			if(	   IGNORE_NOTIFY_QUEST_ID_1 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_2 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_3 == (*iter).first )
			{
				kQuestList.erase(iter++);
				continue;
			}
			//And
			ContQuestID::iterator and_itor = pkQuestInfo->m_kLimit_PreQuestAnd.begin();
			while( and_itor != pkQuestInfo->m_kLimit_PreQuestAnd.end() )
			{
				ContQuestID::value_type const& kQuestID = (*and_itor);

				if( kQuestID != ClearQuestID )
				{
					bIsLive = false;
					break;
				}
				++and_itor;
			}
			//Or
			if( bIsLive && 0 != pkQuestInfo->m_kLimit_PreQuestOr.size() )
			{
				bIsLive = false;
				ContQuestID::iterator or_itor = pkQuestInfo->m_kLimit_PreQuestOr.begin();
				while( or_itor != pkQuestInfo->m_kLimit_PreQuestOr.end() )
				{
					ContQuestID::value_type const& kQuestID = (*or_itor);

					if( kQuestID == ClearQuestID )
					{
						bIsLive = true;
						break;
					}
					++or_itor;
				}
			}

			if( bIsLive )
			{
				++iter;
				continue;
			}

			iter = kQuestList.erase(iter);
		}

		iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			if( QBL_None != PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPlayer), kQuestID, pkQuestInfo) )
			{
				iter = kQuestList.erase(iter);
				continue;
			}
			++iter;
		}

		return !kQuestList.empty();
	}
	bool CheckNewQuestAll(ContQuestInfo& kQuestList)
	{
		PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		ContQuestInfo::iterator	iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			bool bIsLive = true;

			if(	   IGNORE_NOTIFY_QUEST_ID_1 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_2 == (*iter).first
				|| IGNORE_NOTIFY_QUEST_ID_3 == (*iter).first )
			{
				kQuestList.erase(iter++);
				continue;
			}
			//And
			ContQuestID::iterator and_itor = pkQuestInfo->m_kLimit_PreQuestAnd.begin();
			while( and_itor != pkQuestInfo->m_kLimit_PreQuestAnd.end() )
			{
				ContQuestID::value_type const& kQuestID = (*and_itor);
				if( false == lwIsEndedQuest(kQuestID) 
					&& false == lwIsIngQuest(kQuestID) )
				{
					bIsLive = false;
					break;
				}
				++and_itor;
			}
			//Or
			if( bIsLive && 0 != pkQuestInfo->m_kLimit_PreQuestOr.size() )
			{
				bIsLive = false;
				ContQuestID::iterator or_itor = pkQuestInfo->m_kLimit_PreQuestOr.begin();
				while( or_itor != pkQuestInfo->m_kLimit_PreQuestOr.end() )
				{
					ContQuestID::value_type const& kQuestID = (*or_itor);
					if( true == lwIsEndedQuest(kQuestID)
					|| true == lwIsIngQuest(kQuestID)  )
					{
						bIsLive = true;
						break;
					}
					++or_itor;
				}
			}

			if( bIsLive )
			{
				++iter;
				continue;
			}

			iter = kQuestList.erase(iter);
		}

		int const iShowAllQuest = g_kGlobalOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_SHOW_ALL_QUEST);
		int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);
		iter = kQuestList.begin();
		while( iter != kQuestList.end() )
		{
			ContQuestInfo::key_type const& kQuestID = iter->first;
			ContQuestInfo::mapped_type const& pkQuestInfo = iter->second;

			if( QBL_None != PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPlayer), kQuestID, pkQuestInfo) )
			{
				iter = kQuestList.erase(iter);
				continue;
			}

			if( 0 == iShowAllQuest )
			{//10레벨 이상 차이나는 시작가능한 퀘스트 표시 생략
				if( iPlayerLevel > pkQuestInfo->m_kLimit.iMaxLevel )
				{
					iter = kQuestList.erase(iter);
					continue;
				}
			}

			if( (pkQuestInfo->Type() == QT_Random)
			 || (pkQuestInfo->Type() == QT_RandomTactics)
			 || (pkQuestInfo->Type() == QT_Wanted) )
			{
				iter = kQuestList.erase(iter);
				continue;
			}

			++iter;
		}

		return !kQuestList.empty();
	}

	bool CheckRemoteAcceptQuest(int const CallType, int const QuestID, bool const bIsScenario)
	{
		ContQuestInfo	kQuestList;
		g_kQuestMan.GetQuest(kQuestList, (bIsScenario)?(QT_Scenario):(QT_None));
		switch( CallType )
		{
		case EQNCT_LEVELUP:
			{//레벨업한거다
				if( false == CheckNewQuestToLevelUP(kQuestList) )
				{
					return false;
				}
			}break;
		case EQNCT_QUEST_CLEAR:
			{//퀘스트를 클리어 한거다
				if( false == CheckNewQuestToQuestClear(QuestID, kQuestList) )
				{
					return false;
				}
			}break;
		case EQNCT_ALL:
		default:
			{//퀘스트 전체
				if( false == CheckNewQuestAll(kQuestList) )
				{
					return false;
				}
			}break;
		}
		ContQuestInfo::iterator itor_quest = kQuestList.begin();
		while( kQuestList.end() != itor_quest )
		{
			ContQuestInfo::mapped_type const& pkQuestInfo = itor_quest->second;
			if( NULL == pkQuestInfo )
			{
				return false;
			}
			if( true == pkQuestInfo->IsCanRemoteAccept() )
			{
				return true;
			}
			++itor_quest;
		}
		return false;
	}
	void CallToolTip_NearQuestIcon(lwUIWnd kWnd)
	{
		bool bGray = kWnd.IsGrayScale();
		int const iMapNameNo = kWnd.GetCustomData<int>();
		if( 0 == iMapNameNo )
		{
			return;
		}
		BM::vstring kToolTip;
		if( true == bGray )
		{
			kToolTip = TTW(75008);
		}
		else
		{
			kToolTip = TTW(75007);
		}
		kToolTip.Replace(L"#DEF_STRING_MAP#", GetMapName(iMapNameNo));
		lwCallToolTipByText(0, UNI(kToolTip), kWnd.GetLocation());
	}
}