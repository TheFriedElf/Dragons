#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgPlayer.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestMan.h"
#include "PgPilotMan.h"
#include "PgQuestIngList.h"
#include "PgQuestDialog.h"
#include "lwWString.h"
#include "lwGuid.h"
#include "lwUI.h"
#include "lwQuestMan.h"
#include "lwUIQuest.h"

#include "Pg2dString.h"
#include "PgChatMgrClient.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgQUI_IngQuestList
///////////////////////////////////////////////////////////////////////////////////////////////////////////

PgQUI_IngQuestList::PgQUI_IngQuestList()
{
	m_kSelectQuestID = 0;
}

PgQUI_IngQuestList::~PgQUI_IngQuestList()
{
}

void PgQUI_IngQuestList::Show(int const iSelectedQuestID)
{
	m_kSelectQuestID = iSelectedQuestID;
	UpdateControl();
}

void PgQUI_IngQuestList::UpdateControl()
{
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	if( !pkMyQuest->IsIngQuest( m_kSelectQuestID ) )
	{
		m_kSelectQuestID = 0;
	}


	//
	if( !m_kSelectQuestID ) // 퀘스트 목록창이 불리우면서, 퀘스트를 지정 하지 않으면
	{
		m_kSelectQuestID = g_kQuestMan.GetRecentBeginQuest();
		if( !m_kSelectQuestID ) // 최근에 받은 퀘스트가 없으면
		{
			// 내 퀘스트 목록중에서 시나리오 퀘스트 또는 일반 퀘스트를 아무거나 선택해준다
			ContUserQuestState kMyQuestState;
			bool const bHaveIngQuest = pkMyQuest->GetQuestList(kMyQuestState);

			if( bHaveIngQuest )
			{
				ContQuestID kIngScenarioQuest, kIngNormalQuest;

				ContUserQuestState::const_iterator iter = kMyQuestState.begin();
				while( kMyQuestState.end() != iter )
				{
					ContUserQuestState::value_type const &rkElement = (*iter);
					PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest( rkElement.iQuestID );
					if( pkQuestInfo )
					{
						switch( pkQuestInfo->Type() )
						{
						case QT_Scenario:
							{
								std::back_inserter(kIngScenarioQuest) = pkQuestInfo->ID();
							}break;
						case QT_Soul:
							{
							}break;
						default:
							{
								std::back_inserter(kIngNormalQuest) = pkQuestInfo->ID();
							}break;
						}
					}
					++iter;
				}

				//
				if( !kIngScenarioQuest.empty() )
				{
					m_kSelectQuestID = kIngScenarioQuest.front();
				}
				else if( !kIngNormalQuest.empty() )
				{
					m_kSelectQuestID = kIngNormalQuest.front();
				}
			}
		}
	}
	//

	XUI::CXUI_Wnd* pkListForm = GetControl(_T("SFRM_MY_QUEST_LIST"));
	if( !pkListForm )
	{return;}

	XUI::CXUI_List* pkQuestNormalList = dynamic_cast<XUI::CXUI_List*>(pkListForm->GetControl(_T("QUEST_LIST")));
	if( !pkQuestNormalList )
	{return;}

	XUI::CXUI_List* pkQuestScenarioList = dynamic_cast<XUI::CXUI_List*>(pkListForm->GetControl(_T("QUEST_SCENARIO_LIST")));
	if( !pkQuestScenarioList )
	{return;}

	XUI::CXUI_Wnd* pkScenarioCountWnd = pkListForm->GetControl(_T("SFRM_TITLE_SCENARIO_COUNT"));
	XUI::CXUI_Wnd* pkNormalCountWnd = pkListForm->GetControl(_T("SFRM_TITLE_NORMAL_COUNT"));
	size_t iScenarioQuestCount = 0, iNormalQuestCount = 0;

	//내 퀘스트 목록을 Mananger로 부터 얻는다.
	ContUserQuestState kMyQuestState;
	bool const bHaveIngQuest = pkMyQuest->GetQuestList(kMyQuestState);

	//있던 없던 초기화
	pkQuestNormalList->ClearList();
	pkQuestScenarioList->ClearList();

	//
	PgQUI_QuestDialog* pkQuestInfoWnd = dynamic_cast<PgQUI_QuestDialog*>( GetControl(WSTR_MY_QUEST_INFO) );
	if( pkQuestInfoWnd
	&&	!bHaveIngQuest )
	{
		pkQuestInfoWnd->Clear();
	}

	if( bHaveIngQuest )
	{
		ContUserQuestState::iterator myquest_iter = kMyQuestState.begin();
		for(; kMyQuestState.end() != myquest_iter; ++myquest_iter)
		{
			const SUserQuestState& rkQuestState = (*myquest_iter);
			int const iQuestID = rkQuestState.iQuestID;//퀘스트 정보를 가져온다.
			const PgQuestInfo* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{continue;}

			//Quest Title
			std::wstring const kQuestTitle = TTW(pkQuestInfo->m_iTitleTextNo);

			SUserQuestState const *pkMyQuestState = pkMyQuest->Get(iQuestID);//내 상태
			if( !pkMyQuestState )
			{continue;}

			switch( pkQuestInfo->Type() )
			{
			case QT_Scenario:
				{
					XUI::SListItem *pkNewItem = pkQuestScenarioList->AddItem(kQuestTitle);
					if( !pkNewItem )			{ continue; }
					if( !pkNewItem->m_pWnd )	{ continue; }
					UpdateItem(pkNewItem->m_pWnd, rkQuestState, pkQuestInfo);
					++iScenarioQuestCount;
				}break;
			case QT_Soul:
				{
					continue;
				}break;
			default:
				{
					//그룹을 찾고 업으면 생성, 있으면 진행
					std::wstring const kQuestGroupName = TTW(pkQuestInfo->m_iGroupTextNo);
					XUI::SListItem* pkGroupItem = pkQuestNormalList->FindItem(kQuestGroupName);
					if( !pkGroupItem )
					{
						pkGroupItem = pkQuestNormalList->AddItem(kQuestGroupName);
						if( !pkGroupItem )	{ continue; }//완전 실패

						XUI::CXUI_Tree* pkGroupItemWnd = dynamic_cast<XUI::CXUI_Tree*>( pkGroupItem->m_pWnd );
						if( pkGroupItemWnd )
						{
							CXUI_Wnd* pkGroupTitleWnd = pkGroupItemWnd->GetControl(_T("SFRM_QUEST_GROUP"));
							if( pkGroupTitleWnd )
							{
								pkGroupTitleWnd->Text(kQuestGroupName);
							}
							pkGroupItemWnd->DeleteAllItem();
						}
					}

					XUI::CXUI_Tree* pkGroupTreeWnd = dynamic_cast<XUI::CXUI_Tree*>(pkGroupItem->m_pWnd);
					if( !pkGroupTreeWnd )	{ continue; }

					XUI::SListItem* pkNewQuestItem = pkGroupTreeWnd->AddItem(kQuestTitle);
					if( !pkNewQuestItem )	{ continue; }

					++iNormalQuestCount;

					UpdateItem(pkNewQuestItem->m_pWnd, rkQuestState, pkQuestInfo);

					pkGroupTreeWnd->CheckState(true);
				}break;
			}
		}

		if( pkQuestInfoWnd
		&&	m_kSelectQuestID )
		{
			pkQuestInfoWnd->Show(true, m_kSelectQuestID, 0);
		}
	}

	if( pkScenarioCountWnd )
	{
		TCHAR szTemp[512] = {0, };
		_stprintf_s(szTemp, _T("%d / %d"), iScenarioQuestCount, MAX_ING_SCENARIO_QUEST);
		pkScenarioCountWnd->Text( std::wstring(szTemp) );
	}
	if( pkNormalCountWnd )
	{
		TCHAR szTemp[512] = {0, };
		_stprintf_s(szTemp, _T("%d / %d"), iNormalQuestCount, MAX_ING_QUESTNUM);
		pkNormalCountWnd->Text( std::wstring(szTemp) );
	}
}

void PgQUI_IngQuestList::UpdateItem(XUI::CXUI_Wnd *pkNewQuestWnd, SUserQuestState const &rkQuestState, PgQuestInfo const *pkQuestInfo)
{
	if( !pkNewQuestWnd
	||	!pkQuestInfo )
	{
		return;
	}

	std::wstring kQuestTitle = TTW(pkQuestInfo->m_iTitleTextNo);

#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kQuestID(pkQuestInfo->ID());
			kQuestTitle += _T(" ") + (std::wstring)kQuestID;
		}
#endif

	switch(rkQuestState.byQuestState)
	{
	case QS_End:
		{
			pkNewQuestWnd->FontColor(0xFF00A336);//완료는 색깔 바꿈
			kQuestTitle = _T("[") + TTW(20001) + _T("] ") + kQuestTitle;
			pkNewQuestWnd->FontFlag( pkNewQuestWnd->FontFlag() | XTF_BOLD );
		}break;	
	case QS_Failed:
		{
			pkNewQuestWnd->FontColor(0xFFC0C0C0);//실패도 색깔 바꿈
			kQuestTitle = _T("[") + TTW(20002) + _T("] ") + kQuestTitle;
			pkNewQuestWnd->FontFlag( pkNewQuestWnd->FontFlag() | XTF_BOLD );
		}break;
	default:
		{
			if( pkQuestInfo->m_kLimit.iMinLevel )//Min Level
			{
				BM::vstring kStartLevel(pkQuestInfo->m_kLimit.iMinLevel);
				kQuestTitle = _T("[") + (std::wstring)kStartLevel + _T("] ") + kQuestTitle;
			}
			pkNewQuestWnd->FontColor(0xFF4D3413);
		}break;
	}

	XUI::CXUI_Button* pSelectedButton = dynamic_cast<XUI::CXUI_Button*>(pkNewQuestWnd->GetControl(_T("BTN_SELECT")));
	if( pSelectedButton )
	{
		if( m_kSelectQuestID == rkQuestState.iQuestID ) //선택 된거면
		{			
			pSelectedButton->Visible(true);
		}
		else
		{			
			pSelectedButton->Visible(false);
		}
	}

	pkNewQuestWnd->SetCustomData(&rkQuestState.iQuestID, sizeof(rkQuestState.iQuestID));
	Quest::SetCutedTextLimitLength(pkNewQuestWnd, kQuestTitle, _T("..."), 174);

	//
	XUI::CXUI_CheckButton* pkCheckBtnWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkNewQuestWnd->GetControl(_T("CHKBTN_CHK")));
	if( pkCheckBtnWnd )
	{
		pkCheckBtnWnd->SetCustomData(&rkQuestState.iQuestID, sizeof(rkQuestState.iQuestID));

		bool const bShowMiniList = g_kQuestMan.IsMiniQuestList(rkQuestState.iQuestID);
		pkCheckBtnWnd->Check(bShowMiniList);
	}
}

void PgQUI_IngQuestList::OnCheckBoxClick_From_MyQuestList(XUI::CXUI_Wnd *pItem)
{	
	XUI::CXUI_CheckButton* pButton = dynamic_cast<XUI::CXUI_CheckButton*>(pItem);
	if( !pButton )
	{
		return;
	}

	int	iQuestID = 0;
	pButton->GetCustomData(&iQuestID, sizeof(iQuestID));

	bool bRet = false;
	if( pButton->Check() )
	{
		bRet = g_kQuestMan.RemoveMiniQuestList(iQuestID);
	}
	else
	{
		bRet = g_kQuestMan.AddMiniQuestList(iQuestID);
		if( !bRet )
		{
			g_kChatMgrClient.AddMessage(450267, SChatLog(CT_EVENT_SYSTEM), true, 1);
		}
	}

	if( bRet )
	{
		pButton->Check(!pButton->Check());//체크 반전
	}
}

void PgQUI_IngQuestList::OnQuestItemSelected_From_MyQuestList(int const iQuestID)
{
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	PgQUI_QuestDialog* pkQuestInfoWnd = dynamic_cast<PgQUI_QuestDialog*>( GetControl(WSTR_MY_QUEST_INFO) );
	if( !pkQuestInfoWnd )
	{
		return;
	}

	if( m_kSelectQuestID
	&&	m_kSelectQuestID == iQuestID )
	{
		return;
	}

	pkQuestInfoWnd->Show(true, iQuestID, 0);

	m_kSelectQuestID = iQuestID;

	XUI::CXUI_Wnd* pkListForm = GetControl(_T("SFRM_MY_QUEST_LIST"));
	if( !pkListForm )
	{return;}

	XUI::CXUI_List* pkQuestNormalList = dynamic_cast<XUI::CXUI_List*>(pkListForm->GetControl(_T("QUEST_LIST")));
	if( !pkQuestNormalList )
	{return;}
	XUI::CXUI_List* pkQuestScenarioList = dynamic_cast<XUI::CXUI_List*>(pkListForm->GetControl(_T("QUEST_SCENARIO_LIST")));
	if( !pkQuestScenarioList )
	{return;}

	{//Scenario
		SListItem *pkCurItem = pkQuestScenarioList->FirstItem();
		while( pkCurItem )
		{
			XUI::CXUI_Wnd *pkItemWnd = pkCurItem->m_pWnd;
			if( pkItemWnd )
			{
				int iQuestID = 0;
				pkItemWnd->GetCustomData(&iQuestID, sizeof(iQuestID));
				PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);

				SUserQuestState const *pkUserState = pkMyQuest->Get(iQuestID);

				if( pkQuestInfo && pkUserState )
				{
					UpdateItem(pkItemWnd, *pkUserState, pkQuestInfo);
				}
			}
			pkCurItem = pkQuestScenarioList->NextItem(pkCurItem);
		}
	}

	{//Normal
		SListItem *pkCurItem = pkQuestNormalList->FirstItem();
		while( pkCurItem )
		{
			XUI::CXUI_Tree *pkTreeWnd = dynamic_cast<XUI::CXUI_Tree*>(pkCurItem->m_pWnd);
			if( pkTreeWnd )
			{
				SListItem *pkSubCurItem = pkTreeWnd->FirstItem();
				while( pkSubCurItem )
				{
					XUI::CXUI_Wnd *pkItemWnd = pkSubCurItem->m_pWnd;
					if( pkItemWnd )
					{	
						int iQuestID = 0;
						pkItemWnd->GetCustomData(&iQuestID, sizeof(iQuestID));
						PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);

						SUserQuestState const *pkUserState = pkMyQuest->Get(iQuestID);

						if( pkQuestInfo && pkUserState )
						{
							UpdateItem(pkItemWnd, *pkUserState, pkQuestInfo);
						}
					}

					pkSubCurItem = pkTreeWnd->NextItem(pkSubCurItem);
				}
			}
			pkCurItem = pkQuestNormalList->NextItem(pkCurItem);
		}
	}

}
