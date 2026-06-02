#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgQuestInfo.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestSelectiveList.h"
#include "PgQuestMan.h"
#include "lwGuid.h"
#include "lwUI.h"
#include "lwUIQuest.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgQUI_SelectiveQuestList
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void PgQUI_SelectiveQuestList::Show(BM::GUID const &rkQuestObjectGUID)
{
	OwnerGuid(rkQuestObjectGUID);
	UpdateControl();
}

void PgQUI_SelectiveQuestList::UpdateControl()
{
	PgPilot *pPilot = g_kPilotMan.FindPilot(OwnerGuid());
	if( !pPilot )
	{
		return;
	}

	PgActor	*pActor = dynamic_cast<PgActor *>(pPilot->GetWorldObject());
	if( !pActor )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTitleForm = GetControl(_T("SFRM_SELECTIVE_QUEST"));
	if( pkTitleForm )
	{
		XUI::CXUI_Wnd* pkTitleBtn = pkTitleForm->GetControl(_T("CBTN_TITLE"));
		if( pkTitleBtn )
		{
			std::wstring kName = pPilot->GetName();
			PgStringUtil::EraseStr<std::wstring>(kName, _T("\n"), kName);//케리지 리턴 삭제
			pkTitleBtn->Text(kName);
		}
	}

	ContNpcQuestInfo kNpcQuest;
	size_t iQuestCount = g_kQuestMan.PopNPCQuestInfo(OwnerGuid(), kNpcQuest);
	if( !iQuestCount )
	{
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(GetControl(_T("QUEST_LIST")));
	if( !pkList )
	{
		return;
	}
	
	pkList->ClearList();
	lua_tinker::call<void, lwGUID>("OnInit_SelectiveQuestList", lwGUID(OwnerGuid()));

	DWORD dwColor = COLOR_WHITE;
	ContNpcQuestInfo::iterator quest_iter = kNpcQuest.begin();
	int iCur = 0;
	while(kNpcQuest.end() != quest_iter)
	{
		const ContNpcQuestInfo::value_type pkElement = (*quest_iter);

		const PgQuestInfo* pkQuestInfo = g_kQuestMan.GetQuest(pkElement->iQuestID);
		bool const bCanQuest = (pkElement->cState == QS_Begin) || (pkElement->cState == QS_Ing) || (pkElement->cState == QS_End);
		bool const bIsDailyQuest = PgQuestInfoUtil::IsDailyUIQuest(pkQuestInfo); // 1일 퀘스트는 UI가 따로다
		if( pkQuestInfo
		&&	bCanQuest
		&&	!bIsDailyQuest
		&&	QT_Scroll != pkQuestInfo->Type() ) // 퀘스트 스크롤로만 퀘스트를 받을 수 있다
		{
			int const iQuestID = pkElement->iQuestID;
			std::wstring ItemTitle = TTW(pkQuestInfo->m_iTitleTextNo);
			XUI::SListItem* pItem = pkList->AddItem(ItemTitle);
			if( !pItem )
			{
				NILOG(PGLOG_ERROR, "Can't add item"); assert(0);
				continue;
			}

			XUI::CXUI_Wnd* pSelectiveWnd = pItem->m_pWnd;
			if( pSelectiveWnd )
			{
				lua_tinker::call<void, int, int>("OnAdd_SelectiveQuestList", iCur, iQuestID);

				pSelectiveWnd->SetCustomData(&iQuestID, sizeof(iQuestID));
				pSelectiveWnd->OwnerGuid( OwnerGuid() );

				std::wstring kQuestTypeString = _T("");
				if( QT_Couple == pkQuestInfo->Type() )
				{
					kQuestTypeString = _T("{C=0xFFF10FD9/}[") + TTW(450025) + _T("]{C=0xFF4D3413/}");
				}
				else if( QT_SweetHeart == pkQuestInfo->Type() )
				{
					kQuestTypeString = _T("{C=0xFFF10FD9/}[") + TTW(450145) + _T("]{C=0xFF4D3413/}");
				}
				else if( QT_Scenario == pkQuestInfo->Type() )
				{
					kQuestTypeString = _T("{C=0xFFEE070D/}[") + TTW(3031) + _T("]{C=0xFF4D3413/}");
				}
				else
				{
					kQuestTypeString = _T("");
				}

				std::wstring kAddString;
				std::wstring kTextColor;
				std::wstring kTextDefaultColor = _T("{C=0xFF4D3413/}");
				if( QS_End == pkElement->cState )
				{
					kTextColor = _T("{C=0xFF00A336/}");					
					kAddString = kTextColor + _T("[") + TTW(20001) + _T("]");
					pSelectiveWnd->FontFlag( pSelectiveWnd->FontFlag() | XTF_BOLD );
				}
				else if( QS_Ing == pkElement->cState )
				{
					kTextColor = _T("{C=0xFF0071BB/}");
					kAddString = kTextColor + TTW(20008);
					pSelectiveWnd->FontFlag( pSelectiveWnd->FontFlag() | XTF_BOLD );
				}
				else
				{
					kTextColor = kTextDefaultColor;
				}

				int iWidth = pSelectiveWnd->Width();
				if( !kAddString.empty() )
				{
					pSelectiveWnd->Text(kAddString);
					XUI::CXUI_Style_String kStyleString = pSelectiveWnd->StyleText();
					POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
					iWidth -= (kTextSize.x + 50); // 강제로 50만큼 더 빼준다.
				}
				Quest::SetCutedTextLimitLength(pSelectiveWnd, ItemTitle, _T("..."), iWidth);
				ItemTitle = kQuestTypeString + kTextColor+ pSelectiveWnd->Text() + kAddString + kTextDefaultColor;
				pSelectiveWnd->Text( ItemTitle );
			}
			++iCur;
		}

		++quest_iter;
	}

	lua_tinker::call<void, int>("Move_SelectiveQuestList", 0);
}