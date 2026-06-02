#include "stdafx.h"
#include "lwGuid.h"
#include "lwUI.h"
#include "PgQuestMan.h"
#include "PgDailyQuestUI.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgChatMgrClient.h"
#include "lwUIQuest.h"
#include "ServerLib.h"
#include "PgTextDialogMng.h"
#include "Variant/PgStringUtil.h"
#include "PgUIModel.h"
#include "PgUIScene.h"
#include "PgNifMan.h"
#include "PgResourceIcon.h"
#include "PgWorldMapPopUpUI.h"

extern bool BreakMoney_WStr(__int64 const iTotal, VEC_WSTRING &rkVec);
extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);

namespace DailyQuestUI
{
	int const iHideTextID = 401027;

	std::wstring const kDailyQuestWndName(_T("FRM_ONEDAY_QUEST"));
	typedef std::vector< CXUI_Wnd* > ContXuiWnd;

	typedef struct tagFinderNpcQuest
	{
		inline explicit tagFinderNpcQuest(int const iQuestID)
			:m_iQuestID(iQuestID)
		{
		}

		inline tagFinderNpcQuest(tagFinderNpcQuest const& rhs)
			:m_iQuestID(rhs.m_iQuestID)
		{
		}

		inline bool operator () (ContNpcQuestInfo::value_type const& rkElement) const
		{
			return m_iQuestID == rkElement->iQuestID;
		}
	private:
		int const m_iQuestID;
	} SFinderNpcQuest;

	bool SortDailyQuest(int const lhs, int const rhs)
	{
		PgQuestInfo const* pkLhs = g_kQuestMan.GetQuest(lhs);
		PgQuestInfo const* pkRhs = g_kQuestMan.GetQuest(rhs);
		if( !pkLhs || !pkRhs )
		{
			return lhs < rhs; // 단순 ID로 정렬
		}
		return pkLhs->OrderIndex() < pkRhs->OrderIndex();
	}

	void SetDailyQuestCustomData(XUI::CXUI_Wnd* pkItemWnd, BM::GUID const& kNpcGuid, EQuestState const eState, int const iQuestID)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( kNpcGuid );
		kCustomData.Push( static_cast<int>(eState) );
		kCustomData.Push( iQuestID );
		pkItemWnd->SetCustomData( kCustomData.Data() );
	}

	void GetDailyQuestCustomData(XUI::CXUI_Wnd* pkItemWnd, BM::GUID& kNpcGuid, EQuestState &eState, int &iQuestID)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkItemWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();

		int iTempInt = 0;
		kCustomData.Pop( kNpcGuid );
		kCustomData.Pop( iTempInt );
		kCustomData.Pop( iQuestID );
		eState = static_cast<EQuestState>(iTempInt);
	}

	EQuestState GetDailyQuestState(int const iQuestID, ContNpcQuestInfo const& kQuestVec, PgMyQuest const* pkMyQuest)
	{
		if( !pkMyQuest )
		{
			return QS_None;
		}

		SUserQuestState const* pkUserState = pkMyQuest->Get(iQuestID);
		if( pkUserState )
		{
			return static_cast<EQuestState>(pkUserState->byQuestState);
		}

		EQuestState eState = QS_None;
		ContNpcQuestInfo::const_iterator find_iter = std::find_if(kQuestVec.begin(), kQuestVec.end(), SFinderNpcQuest(iQuestID));
		if( kQuestVec.end() != find_iter )
		{
			eState = static_cast<EQuestState>((*find_iter)->cState);
		}
		else if( pkMyQuest->IsEndedQuest(iQuestID) )
		{
			eState = QS_Finished;
		}
		else
		{
			SUserQuestState const* pkUserState = pkMyQuest->Get(iQuestID);
			if( pkUserState )
			{
				eState = static_cast<EQuestState>(pkUserState->byQuestState);
			}
		}
		return eState;
	}

	inline void GetDailyQuestCustomData(XUI::CXUI_Wnd* pkItemWnd, EQuestState &eState, int &iQuestID)
	{
		BM::GUID kTempNpcGuid;
		GetDailyQuestCustomData(pkItemWnd, kTempNpcGuid, eState, iQuestID);
	}

	bool IsCanOpenQuest(EQuestState const eState)
	{
		switch( eState )
		{
		case QS_Begin:
		case QS_Ing:
		case QS_End:
		case QS_Finished:
		case QS_Failed:
			{
			}break;
		case QS_Begin_NYet:
		default:
			{
				return false;
			}break;
		}
		return true;
	}

	inline bool IsCountedDailyQuestState(EQuestState const eState)
	{
		switch( eState )
		{
		case QS_Finished:
			{
				return true;
			}break;
		}
		return false;
	}

	void UpdateDailyItem(XUI::CXUI_Wnd* pkItemWnd, int const iQuestID, EQuestState const eState, bool const bIsSelected, bool const bOpened)
	{
		if( !pkItemWnd )
		{
			return;
		}

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
		if( !pkQuestInfo )
		{
			return;
		}

		int const iNormalUVIndex	= 1;
		int const iFocusedUVIndex	= 2;
		int const iSelectedUVIndex	= 3;
		int const iDisabledUVIndex	= 4;
		DWORD const dwUnSelectedFontColor		= 0xFF52322E;
		DWORD const dwSelectedFontColor			= 0xFFFFF1B2;
		DWORD const dwIngQuestFontColor			= 0xFF1B48A2;
		DWORD const dwEndQuestFontColor			= 0xFF1A6C01;
		DWORD const dwFailedQuesetFontColor		= 0xFFDD1900;

		std::wstring kTitleText = TTW(pkQuestInfo->m_iTitleTextNo);
		int iUVIndex = iNormalUVIndex;
		DWORD dwTextColor = dwUnSelectedFontColor;
		bool bGrayScale = false;
		switch( eState )
		{
		case QS_Begin:
			{
			}break;
		case QS_Ing:
			{
				kTitleText += _T(" ") + TTW(401028);
				dwTextColor = dwIngQuestFontColor;
			}break;
		case QS_End:
			{
				kTitleText += _T(" ") + TTW(401029);
				iUVIndex = iFocusedUVIndex;
				dwTextColor = dwEndQuestFontColor;
			}break;
		case QS_Failed:
			{
				kTitleText += _T(" ") + TTW(401030);
				dwTextColor = dwFailedQuesetFontColor;
			}break;
		case QS_Finished:
			{
				kTitleText += _T(" ") + TTW(401031);
			}break;
		case QS_Begin_NYet:
		default:
			{
				bGrayScale = true;
				iUVIndex = iDisabledUVIndex;
			}break;
		}

		std::wstring const& rkTitleText = (bOpened)? kTitleText: TTW(iHideTextID);
		pkItemWnd->Text( rkTitleText );

		SUVInfo kUVInfo = pkItemWnd->UVInfo();
		kUVInfo.Index = (bIsSelected)? iSelectedUVIndex: iUVIndex;
		pkItemWnd->UVInfo(kUVInfo);
		if( bIsSelected )
		{
			pkItemWnd->FontColor( dwSelectedFontColor );
			pkItemWnd->FontFlag( pkItemWnd->FontFlag() | XTF_OUTLINE );
		}
		else
		{
			pkItemWnd->FontColor( dwTextColor );
			pkItemWnd->FontFlag( pkItemWnd->FontFlag() & (~XTF_OUTLINE) );
		}
		pkItemWnd->GrayScale(bGrayScale);

		switch( pkQuestInfo->Type() )
		{
		case QT_Random:
		case QT_RandomTactics:
			{
			}break;
		default:
			{
				size_t const iQuestLevel = static_cast<size_t>(pkQuestInfo->Difficult());
				ContXuiWnd kVec;
				std::back_inserter(kVec) = pkItemWnd->GetControl(std::wstring(_T("IMG_STAR1")));
				std::back_inserter(kVec) = pkItemWnd->GetControl(std::wstring(_T("IMG_STAR2")));
				std::back_inserter(kVec) = pkItemWnd->GetControl(std::wstring(_T("IMG_STAR3")));
				std::back_inserter(kVec) = pkItemWnd->GetControl(std::wstring(_T("IMG_STAR4")));
				ContXuiWnd::iterator star_iter = kVec.begin();
				for( size_t iCur = 0; kVec.size() > iCur; ++iCur )
				{
					CXUI_Wnd *pkImgWnd = kVec.at(iCur);
					if( pkImgWnd )
					{
						bool const bVisible = (bOpened)? iQuestLevel > iCur: false;
						pkImgWnd->Visible( bVisible );
					}
				}
			}break;
		}
	}
	void UpdateSelectedQuest(PgPlayer const* pkPlayer, CXUI_Wnd* pkTopWnd, int const iSelectedQuestID, EQuestState const eSelectedQuestState, bool const bSelectedOpened, bool const bShowReward)
	{
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iSelectedQuestID);

		CXUI_Wnd* pkQuestInfoTitle = pkTopWnd->GetControl(_T("FRM_INFO_TITLE"));
		if( pkQuestInfoTitle )
		{
			std::wstring const kTitleText = (pkQuestInfo)? TTW(pkQuestInfo->m_iTitleTextNo): std::wstring();
			std::wstring const& rkTitleText = (bSelectedOpened)? kTitleText: TTW(iHideTextID);
			pkQuestInfoTitle->Text( rkTitleText );
		}

		CXUI_List* pkQuestInfoContents = dynamic_cast<CXUI_List*>(pkTopWnd->GetControl(_T("LIST_ONEDAY_QUEST_INFO")));
		if( pkQuestInfoContents )
		{
			pkQuestInfoContents->ClearList();

			SListItem* pkNewItem = pkQuestInfoContents->AddItem(std::wstring());
			if( pkNewItem
			&&	pkNewItem->m_pWnd )
			{
				CXUI_Wnd* pkContentsWnd = pkNewItem->m_pWnd;
				if( pkQuestInfo )
				{
					SQuestDialog const* pkInfoDialog = NULL;
					std::wstring kContentsText;
					if( bSelectedOpened )
					{
						pkQuestInfo->GetInfoDialog(pkInfoDialog);
					}
					else
					{
						pkQuestInfo->GetDialog(QRDID_DailyQuestClosedInfo, pkInfoDialog);
					}

					if( pkInfoDialog
					&&	0 != pkInfoDialog->kDialogText.front().iTextNo )
					{
						kContentsText = TTW(pkInfoDialog->kDialogText.front().iTextNo);
					}
					
					if( kContentsText.empty() )
					{
						kContentsText = TTW(iHideTextID);
					}

					{
						pkContentsWnd->Text( kContentsText );

						CXUI_Style_String kStyleText = pkContentsWnd->StyleText();
						POINT const kCalcSize = Pg2DString::CalculateOnlySize(kStyleText);
						pkContentsWnd->Size( pkContentsWnd->Size().x, kCalcSize.y+4 );
					}
				}
				else
				{
					pkContentsWnd->Text( std::wstring() );
				}
			}
			
			pkQuestInfoContents->AdjustMiddleBtnPos();
			pkQuestInfoContents->AdjustMiddleBarSize();
			pkQuestInfoContents->AdjustDownBtnPos();
		}

		CXUI_Wnd* pkQuestInfoReward = pkTopWnd->GetControl(_T("FRM_REWARD_TITLE"));
		if( pkQuestInfoReward )
		{
			BM::vstring kRewardText(TTW(401021));
			kRewardText << TTW(19931);
			if( pkQuestInfo )
			{
				std::wstring kTemp;
				std::wstring const kSpace(_T(" "));
				kRewardText << TTW(401022) << _T(" :");
				if( 0 != pkQuestInfo->m_kReward.iMoney )
				{
					if( FormatMoney(pkQuestInfo->m_kReward.iMoney, kTemp) )
					{
						kRewardText << kSpace << TTW(401111) << _T("[") << kTemp << _T("]"); // 골드
					}
				}
				int const iRewardExp = PgQuestInfoUtil::GetQuestRewardExp(pkQuestInfo, pkPlayer->GetMyQuest());
				if( 0 != iRewardExp )
				{
					if( FormatTTW(kTemp, 401013, iRewardExp) ) // 경험치
					{
						kRewardText << kSpace << kTemp;
					}
				}
				int const iRewardGuildExp = PgQuestInfoUtil::GetQuestRewardGuildExp(pkQuestInfo, pkPlayer->GetMyQuest());
				if( 0 != iRewardGuildExp )
				{
					if( FormatTTW(kTemp, 401014, iRewardGuildExp) ) // 길드 경험치
					{
						kRewardText << kSpace << kTemp;
					}
				}
				if( 0 != pkQuestInfo->m_kReward.iTacticsExp )
				{
					if( FormatTTW(kTemp, 401015, pkQuestInfo->m_kReward.iTacticsExp) ) // 용병 경험치
					{
						kRewardText << kSpace << kTemp;
					}
				}
			}
			std::wstring const& rkRewardText = (bShowReward)? kRewardText: TTW(iHideTextID);
			pkQuestInfoReward->Text(rkRewardText);
		}

		ContXuiWnd kRewardIconVec, kRewardIconBGVec;
		size_t const iMaxItemArray = 8;
		for( size_t iCur = 0; iMaxItemArray > iCur; ++iCur )
		{
			std::back_inserter(kRewardIconVec) = pkTopWnd->GetControl( BM::vstring(_T("REWARD_ITEM")) << iCur );
			std::back_inserter(kRewardIconBGVec) = pkTopWnd->GetControl( BM::vstring(_T("IMG_REWARD_BG")) << iCur );
		}
		ContRewardVec::value_type const* pkRewardSet = (pkQuestInfo)? pkQuestInfo->m_kReward.GetRewardSet1(pkPlayer->GetAbil(AT_GENDER), pkPlayer->GetAbil(AT_CLASS)): NULL;
		ContXuiWnd::iterator btn_iter = kRewardIconVec.begin();
		ContXuiWnd::iterator btnbg_iter = kRewardIconBGVec.begin();
		size_t iCurItem = 0;
		while( kRewardIconVec.end() != btn_iter && kRewardIconBGVec.end() != btnbg_iter )
		{
			CXUI_Wnd* pkRewardIcon = (*btn_iter);
			CXUI_Wnd* pkRewardIconBg = (*btnbg_iter);
			if( pkRewardIcon
			&&	pkRewardIconBg )
			{
				if( pkRewardSet
				&&	pkRewardSet->kItemList.size() > iCurItem )
				{
					ContQuestItem::value_type const& rkRewardItem = pkRewardSet->kItemList.at(iCurItem);
					pkRewardIcon->Visible( bShowReward );
					pkRewardIconBg->Visible( true );
					pkRewardIcon->SetCustomData(&rkRewardItem, sizeof(rkRewardItem));
					Quest::lwOnCallQuestRewardItemImage(lwUIWnd(pkRewardIcon));
				}
				else
				{
					pkRewardIcon->Visible(false);
					pkRewardIconBg->Visible(false);
					pkRewardIcon->ClearCustomData();
				}
			}
			++btn_iter;
			++btnbg_iter;
			++iCurItem;
		}

		CXUI_Button* pkFuncButton1 = dynamic_cast<CXUI_Button*>(pkTopWnd->GetControl(_T("BTN_FUNC1")));
		CXUI_Button* pkFuncButton2 = dynamic_cast<CXUI_Button*>(pkTopWnd->GetControl(_T("BTN_CLOSE")));
		if( pkFuncButton1
		&&	pkFuncButton2 )
		{
			bool bVisble = true;
			switch( eSelectedQuestState )
			{
			case QS_Begin: // 시작
				{
					bVisble = true;
					pkFuncButton1->Text( TTW(401018) + TTW(2200) );
				}break;
			case QS_Failed: // 포기
				{
					bVisble = false; //감춰
					//pkFuncButton1->Text( TTW(401020) );
				}break;
			case QS_End: // 완료
				{
					bVisble = true;
					pkFuncButton1->Text( TTW(401019) + TTW(2200) );
				}break;
			case QS_Ing:
			case QS_Finished: // 감춰
			default:
				{
					bVisble = false;
				}break;
			}

			bool const bVisibleFunc1Btn = bSelectedOpened && bVisble;
			pkFuncButton1->Visible( bVisibleFunc1Btn );

			int const iCloseBtnDefaultX = 247;
			int const iCloseBtnCenterX = (pkTopWnd->Size().x / 2) - (pkFuncButton2->Size().x / 2);
			POINT3I kBtn2Loc = pkFuncButton2->Location();
			kBtn2Loc.x = (bVisibleFunc1Btn)? iCloseBtnDefaultX: iCloseBtnCenterX;
			pkFuncButton2->Location(kBtn2Loc);
		}
	}

	void SelectDailyQuest(lwUIWnd kSelectWnd)
	{
		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kDailyQuestWndName);
		if( !pkTopWnd )
		{
			return;
		}

		if( !kSelectWnd() )
		{
			return;
		}
		
		BM::GUID kNpcGuid;
		EQuestState eState = QS_None;
		int iQuestID = 0;
		GetDailyQuestCustomData(kSelectWnd(), kNpcGuid, eState, iQuestID);
		SetDailyQuestCustomData(pkTopWnd, kNpcGuid, eState, iQuestID);

		CallDailyQuestUI(lwGUID(kNpcGuid), iLastDailyQuestUITitleNo);
	}
};

bool DailyQuestUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "CallDailyQuestUI", CallDailyQuestUI);
	def(pkState, "SendDailyQuestPacket", SendDailyQuestPacket);
	def(pkState, "SelectDailyQuest", SelectDailyQuest);
	return true;
}

void DailyQuestUI::SendDailyQuestPacket()
{
	CXUI_Wnd* pkTopWnd = XUIMgr.Get(kDailyQuestWndName);
	if( !pkTopWnd )
	{
		return;
	}

	BM::GUID kNpcGuid;
	int iSelectedQuestID = 0;
	EQuestState eSelectedQuestState = QS_None;
	GetDailyQuestCustomData(pkTopWnd, kNpcGuid, eSelectedQuestState, iSelectedQuestID); // 선택된것 로드

	if( !iSelectedQuestID )
	{
		return;
	}

	switch( eSelectedQuestState )
	{
	case QS_Begin: // 시작
		{
			pkTopWnd->Close();
			Quest::lwOnQuestItemSelected_From_SelectiveQuestList(iSelectedQuestID, kNpcGuid);
		}break;
	case QS_End: // 완료
		{
			pkTopWnd->Close();
			BM::Stream kPacket(PT_C_M_REQ_END_DAILYQUEST, kNpcGuid);
			kPacket.Push( iSelectedQuestID );
			NETWORK_SEND(kPacket)
		}break;
	case QS_Failed: // 포기
	case QS_Ing:
	case QS_Finished: // 없어
	default:
		{
		}break;
	}
}

void DailyQuestUI::CallDailyQuestUI(lwGUID kNpcGuid, int const iTitle)
{
	ContNpcQuestInfo kQuestVec;
	g_kQuestMan.PopNPCQuestInfo(kNpcGuid(), kQuestVec);

	ContQuestID kNpcQuestVec;
	if( 0 == g_kQuestMan.GetNpcDailyInfo(kNpcGuid(), kNpcQuestVec) )
	{
		return ;
	}
	std::sort(kNpcQuestVec.begin(), kNpcQuestVec.end(), SortDailyQuest);

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}
	
	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}


	CXUI_Wnd* pkTopWnd = XUIMgr.Get(kDailyQuestWndName);
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(kDailyQuestWndName);
		if( !pkTopWnd )
		{
			return;
		}
	}


	//
	int iSelectedQuestID = 0;
	EQuestState eSelectedQuestState = QS_None;
	GetDailyQuestCustomData(pkTopWnd, eSelectedQuestState, iSelectedQuestID); // 선택된것 로드

	if( 0 != iSelectedQuestID
	&&	kNpcQuestVec.end() == std::find(kNpcQuestVec.begin(), kNpcQuestVec.end(), iSelectedQuestID) ) // 현재 목록에 저장값이 없으면 초기화
	{
		iSelectedQuestID = 0;
		eSelectedQuestState = QS_None;
	}

	eSelectedQuestState = GetDailyQuestState(iSelectedQuestID, kQuestVec, pkMyQuest); // 상태값을 다시 로드


	//
	// 목록 갱신
	size_t iCountQuest = 0;
	CXUI_List* pkQuestList = dynamic_cast<CXUI_List*>(pkTopWnd->GetControl(_T("LIST_ONEDAY_QUEST")));
	if( pkQuestList )
	{
		ContQuestID::iterator loop_iter = kNpcQuestVec.begin();
		if( kNpcQuestVec.end() != loop_iter
		&&	0 == iSelectedQuestID
		&&	QS_None == eSelectedQuestState )
		{
			iSelectedQuestID = (*loop_iter); // 첫번째 퀘스트가
			eSelectedQuestState = GetDailyQuestState(iSelectedQuestID, kQuestVec, pkMyQuest);
		}


		// 있는것(재활용) / 모자르면(추가)
		SListItem* pkListItem = pkQuestList->FirstItem();
		while( kNpcQuestVec.end() != loop_iter )
		{
			int const iQuestID = (*loop_iter);
			EQuestState const eState = GetDailyQuestState(iQuestID, kQuestVec, pkMyQuest);

			SListItem* pkTempItem = (pkListItem)? pkListItem: pkQuestList->AddItem(std::wstring());
			if( pkTempItem )
			{
				UpdateDailyItem(pkTempItem->m_pWnd, iQuestID, eState, iQuestID == iSelectedQuestID, IsCanOpenQuest(eState));
				SetDailyQuestCustomData(pkTempItem->m_pWnd, kNpcGuid(), eState, iQuestID);
				if( IsCountedDailyQuestState(eState) )
				{
					++iCountQuest;
				}
			}

			pkListItem = (pkListItem)? pkQuestList->NextItem(pkListItem): NULL;
			++loop_iter;
		}

		// 남은것(삭제)
		while( pkListItem )
		{
			pkListItem = pkQuestList->DeleteItem(pkListItem);
		}
	}

	CXUI_Wnd* pkTitleWnd = pkTopWnd->GetControl(_T("FRM_TITLE"));
	if( pkTitleWnd )
	{
		pkTitleWnd->Text( TTW(iTitle) );
	}
	
	CXUI_Wnd* pkCountWnd = pkTopWnd->GetControl(_T("FRM_COUNT"));
	if( pkCountWnd )
	{
		BM::vstring kTemp(BM::vstring(TTW(401017)) << iCountQuest << TTW(401016) << _T("/") << kNpcQuestVec.size());
		pkCountWnd->Text( kTemp );
	}


	//
	// 선택된 퀘스트 정보 표시
	bool const bIsCanOpen = IsCanOpenQuest(eSelectedQuestState);
	UpdateSelectedQuest(pkPlayer, pkTopWnd, iSelectedQuestID, eSelectedQuestState, bIsCanOpen, bIsCanOpen);

	//
	SetDailyQuestCustomData(pkTopWnd, kNpcGuid(), eSelectedQuestState, iSelectedQuestID); // 선택된것 저장
	iLastDailyQuestUITitleNo = iTitle;
}

void DailyQuestUI::AnsShowQuestDialog(BM::GUID const& rkNpcGuid, BYTE const cDialogType, int const iQuestID, EQuestType const eQuestType, int const iDialogID)
{
	if( QT_Wanted != eQuestType )
	{
		switch( cDialogType )
		{
		case QSDT_ErrorDialog:
			{
				g_kQuestMan.CallQuestDialog(rkNpcGuid, iQuestID, 0); // 강제로 창 닫어
			}break;
		case QSDT_CompleteDialog:
			{
				g_kQuestMan.CallQuestDialog(rkNpcGuid, iQuestID, iDialogID);
			}break;
		case QSDT_BeginDialog:
		case QSDT_NormalDialog:
			{
				g_kQuestMan.CallQuestDialog(rkNpcGuid, iQuestID, iDialogID);
			}break;
		default:
			{
			}break;
		}
	}

	switch( iDialogID )
	{
	case QRDID_MaxQuestSlot:
		{
			g_kChatMgrClient.LogMsgBox(700401); // 퀘스트 슬롯 만땅
		}break;
	case QRDID_FullInven:
		{
			g_kChatMgrClient.LogMsgBox(3074); // 인벤 만땅
		}break;
	case QRDID_Accepted:
		{
			// 수락
		}break;
	case QRDID_Completed:
		{
			// 완료
		}break;
	default:
		{
		}break;
	}
}

/////////////////////////////////////////////////////////////////////////////
namespace RandomQuestUI
{
	int const iCantCompleteInMission = 400650;
	std::wstring const kRandomQuestUIName( L"FRM_RANDOM_QUEST" );
	std::wstring const kRandomQuestBtnName( L"BTN_DAILY_RANDOM_QUEST" );

	void SendReqBuild()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		//int const iLimitLevel = 24;
		//if( iLimitLevel > pkPlayer->GetAbil(AT_LEVEL) )
		//{
		//	return;
		//}
		if( true == pkPlayer->GetMyQuest()->BuildedRandomQuest()
		&&	!RandomQuest::IsHasRebuildItem(pkPlayer, QT_Random, UICT_REBUILD_RAND_QUEST)
		&&	!RandomQuest::IsHasRebuildItem(pkPlayer, QT_Random, UICT_REBUILD_RAND_QUEST_CONSUME) )
		{
			int const iNoticeLevel = 2;
			g_kChatMgrClient.AddMessage(799103, SChatLog(CT_EVENT_SYSTEM), true, iNoticeLevel);
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_RANDOMQUEST_BUILD);
		NETWORK_SEND(kPacket);
	}
	bool IsCanBuild()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		return !pkMyQuest->BuildedRandomQuest();
	}
	bool IsCanRun()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
		{
			return false;
		}
		return pkMyQuest->BuildedRandomQuest() && (pkMyQuest->RandomQuestCompleteCount() < MAX_QUEST_RANDOM_EXP_COUNT); // true(진행가능) / false(완료)
	}
	bool IsHaveTypeQuest(EQuestType const eType)
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		ContUserQuestState kContState;
		pkMyQuest->GetQuestList(kContState);
		ContUserQuestState::const_iterator iter = kContState.begin();
		while( kContState.end() != iter )
		{
			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest((*iter).iQuestID);
			if( pkQuestInfo )
			{
				if( eType == pkQuestInfo->Type()
				&&	QS_End != (*iter).byQuestState )
				{
					return true;
				}
			}
			++iter;
		}
		return false;
	}
	void CallRandomQuestBtn(bool bTwinkle)
	{
		if( IsCanRun()
		&&	!IsHaveTypeQuest(QT_Random) )
		{
			CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRandomQuestBtnName);
			if( !pkTopWnd )
			{
				pkTopWnd = XUIMgr.Call(kRandomQuestBtnName);
			}
			if( pkTopWnd
			&&	bTwinkle )
			{
				DWORD const dwTwinkleTime = 10000;
				DWORD const dwTwinkleInter = 100;
				pkTopWnd->SetTwinkle(true);
				pkTopWnd->TwinkleTime(dwTwinkleTime);
				pkTopWnd->TwinkleInterTime(dwTwinkleInter);
			}
		}
		else
		{
			XUIMgr.Close(kRandomQuestBtnName);
		}
	}

	//
	class PgRandomQuestUI
	{
	public:
		PgRandomQuestUI(ContQuestID const& rkContQuest, int const iRandomCompleteCount)
			: m_kContQuest(rkContQuest), m_iRandomCompleteCount(iRandomCompleteCount)
		{
		}
		~PgRandomQuestUI()
		{
		}

		static void SetRandomQuestCustomData(CXUI_Wnd* pkTopWnd, EQuestType const eQuestType, EQuestState const eState, int const iSelectedQuestID)
		{
			if( !pkTopWnd )
			{
				return;
			}

			BM::Stream kCustomData;
			kCustomData.Push( eQuestType );
			kCustomData.Push( eState );
			kCustomData.Push( iSelectedQuestID );
			pkTopWnd->SetCustomData( kCustomData.Data() );
		}
		static void GetRandomQuestCustomData(CXUI_Wnd* pkTopWnd, EQuestType& eQuestType, EQuestState& eState, int& iSelectedQuestID)
		{
			if( !pkTopWnd )
			{
				return;
			}
			BM::Stream kCustomData;
			if( !pkTopWnd->GetCustomData(kCustomData.Data()) )
			{
				return;
			}
			kCustomData.PosAdjust();
			kCustomData.Pop( eQuestType );
			kCustomData.Pop( eState );
			kCustomData.Pop( iSelectedQuestID );
		}
		static void SendReqRandomQuestFunc(lwUIWnd kTopWnd)
		{
			CXUI_Wnd* pkTopWnd = kTopWnd();
			if( !pkTopWnd )
			{
				return;
			}
			EQuestType eQuestType = QT_None;
			EQuestState eState = QS_None;
			int iQuestID = 0;
			PgRandomQuestUI::GetRandomQuestCustomData(pkTopWnd, eQuestType, eState, iQuestID);

			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{
				return;
			}
			if( QT_Random == pkQuestInfo->Type()
			||	QT_RandomTactics == pkQuestInfo->Type() )
			{
			}
			else
			{
				return;
			}

			switch( eState )
			{
			case QS_Begin:
				{
					pkTopWnd->Close();
					BM::Stream kPacket(PT_C_M_REQ_BEGIN_RANDOMQUEST, iQuestID);
					NETWORK_SEND(kPacket);
				}break;
			case QS_End:
				{
					if( g_pkWorld
						&& 0 != (g_pkWorld->GetAttr() & GATTR_FLAG_MISSION) 
						)
					{
						g_kChatMgrClient.AddMessage(iCantCompleteInMission, SChatLog(CT_EVENT_SYSTEM), true);
						return;
					}

					pkTopWnd->Close();
					BM::Stream kPacket(PT_C_M_REQ_END_RANDOMQUEST, iQuestID);
					NETWORK_SEND(kPacket);
				}break;
			default:
				{
				}break;
			}
		}
		static void SelectRandomQuest(lwUIWnd kSelectWnd)
		{
			CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRandomQuestUIName);
			if( !pkTopWnd )
			{
				return;
			}

			if( !kSelectWnd() )
			{
				return;
			}
			
			EQuestType eType = QT_None;
			EQuestState eState = QS_None;
			int iQuestID = 0;
			PgRandomQuestUI::GetRandomQuestCustomData(kSelectWnd(), eType, eState, iQuestID);
			PgRandomQuestUI::SetRandomQuestCustomData(pkTopWnd, eType, eState, iQuestID);

			switch( eType )
			{
			case QT_RandomTactics:
				{
					RandomTacticsQuestUI::CallRandomTacticsQuestUI(false);
				}break;
			default:
			case QT_Random:
				{
					CallRandomQuestUI(false);
				}break;
			}
		}

		void Call(EQuestType const eQuestType, bool const bAutoNext)
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( !pkMyQuest )
			{
				return;
			}

			CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRandomQuestUIName);
			if( !pkTopWnd )
			{
				pkTopWnd = XUIMgr.Call(kRandomQuestUIName);
				if( !pkTopWnd )
				{
					return;
				}
			}

			std::sort(m_kContQuest.begin(), m_kContQuest.end(), DailyQuestUI::SortDailyQuest);

			//
			EQuestType eTempType = QT_None;
			bool bSelectedOpen = false;
			int iSelectedQuestID = 0;
			int iSelectedQuestCount = 0;
			EQuestState eSelectedQuestState = QS_None;
			GetRandomQuestCustomData(pkTopWnd, eTempType, eSelectedQuestState, iSelectedQuestID); // 선택된것 로드

			if( 0 != iSelectedQuestID
			&&	m_kContQuest.end() == std::find(m_kContQuest.begin(), m_kContQuest.end(), iSelectedQuestID) ) // 현재 목록에 저장값이 없으면 초기화
			{
				iSelectedQuestID = 0;
				eSelectedQuestState = QS_None;
			}
			if( bAutoNext )
			{
				int iCountQuest = 0;
				ContQuestID::const_iterator loop_iter = m_kContQuest.begin();
				while( m_kContQuest.end() != loop_iter )
				{
					if( iCountQuest == m_iRandomCompleteCount )
					{
						iSelectedQuestID = (*loop_iter);
						break;
					}
					++iCountQuest;
					++loop_iter;
				}
			}
			eSelectedQuestState = GetRandomQuestStatus(pkPlayer, iSelectedQuestID); // 상태값을 다시 로드


			//
			// 목록 갱신
			int iCountQuest = 0;
			CXUI_List* pkQuestList = dynamic_cast<CXUI_List*>(pkTopWnd->GetControl(_T("LIST_RANDOM_QUEST")));
			if( pkQuestList )
			{
				ContQuestID::const_iterator loop_iter = m_kContQuest.begin();
				if( m_kContQuest.end() != loop_iter
				&&	0 == iSelectedQuestID
				&&	QS_None == eSelectedQuestState )
				{
					iSelectedQuestID = (*loop_iter); // 첫번째 퀘스트가
					eSelectedQuestState = GetRandomQuestStatus(pkPlayer, iSelectedQuestID);
				}

				// 있는것(재활용) / 모자르면(추가)
				SListItem* pkListItem = pkQuestList->FirstItem();
				while( m_kContQuest.end() != loop_iter )
				{
					int const iQuestID = (*loop_iter);
					EQuestState const eState = GetRandomQuestStatus(pkPlayer, iQuestID);

					bool const IsCanOpen = (iCountQuest <= m_iRandomCompleteCount);
					if( iQuestID == iSelectedQuestID ) // 선택된것이 오픈 될 수 있느냐?
					{
						bSelectedOpen = IsCanOpen;
						iSelectedQuestCount = iCountQuest;
					}

					SListItem* pkTempItem = (pkListItem)? pkListItem: pkQuestList->AddItem(std::wstring());
					if( pkTempItem )
					{
						DailyQuestUI::UpdateDailyItem(pkTempItem->m_pWnd, iQuestID, eState, iQuestID == iSelectedQuestID, IsCanOpen);
						SetRandomQuestCustomData(pkTempItem->m_pWnd, eQuestType, eState, iQuestID);

						if( pkTempItem->m_pWnd )
						{
							CXUI_Wnd* pkExpRateWnd = pkTempItem->m_pWnd->GetControl( std::wstring(L"FRM_EXP_RATE") );
							if( pkExpRateWnd )
							{
								std::wstring kTemp;
								if( 0 != iCountQuest )
								{
									kTemp = std::wstring(BM::vstring() + TTW(400420) + TTW(406121) + BM::vstring(iCountQuest+1));
								}
								else
								{
									kTemp = L"--";
								}
								pkExpRateWnd->Text( kTemp );
							}
						}
					}

					pkListItem = (pkListItem)? pkQuestList->NextItem(pkListItem): NULL;
					++loop_iter;
					++iCountQuest;
				}

				// 남은것(삭제)
				while( pkListItem )
				{
					pkListItem = pkQuestList->DeleteItem(pkListItem);
				}
			}

			CXUI_Wnd* pkCountWnd = pkTopWnd->GetControl(_T("FRM_COUNT"));
			if( pkCountWnd )
			{
				BM::vstring kTemp(BM::vstring(TTW(401017)) << m_iRandomCompleteCount << TTW(401016) << _T("/") << m_kContQuest.size());
				pkCountWnd->Text( kTemp );
			}

			CXUI_Wnd* pkTitleWnd = pkTopWnd->GetControl( L"FRM_TITLE" );
			if( pkTitleWnd )
			{
				switch( eQuestType )
				{
				case QT_RandomTactics:
					{
						pkTitleWnd->Text( TTW(406139) );
					}break;
				case QT_Random:
				default:
					{
						pkTitleWnd->Text( TTW(406110) );
					}break;
				}
			}

			//
			// 선택된 퀘스트 정보 표시 (보상 항상 표시)
			DailyQuestUI::UpdateSelectedQuest(pkPlayer, pkTopWnd, iSelectedQuestID, eSelectedQuestState, bSelectedOpen, true);

			//
			SetRandomQuestCustomData(pkTopWnd, eQuestType, eSelectedQuestState, iSelectedQuestID); // 선택된것 저장
		}

	public:
		static EQuestState GetRandomQuestStatus(PgPlayer* pkPlayer, int const iQuestID)
		{
			if( !pkPlayer )
			{
				return QS_None;
			}

			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( !pkMyQuest )
			{
				return QS_None;
			}

			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{
				return QS_None;
			}

			if( pkMyQuest->IsEndedQuest(iQuestID) )
			{
				return QS_Finished;
			}
			SUserQuestState const* pkUserState = pkMyQuest->Get(iQuestID);
			if( pkUserState )
			{
				return static_cast< EQuestState >(pkUserState->byQuestState);
			}

			EQuestBeginLimit const eRet = PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPlayer), iQuestID, pkQuestInfo);
			if( QBL_None == eRet )
			{
				return QS_Begin;
			}
			return QS_Begin_NYet;
		}


	private:
		ContQuestID m_kContQuest;
		int const m_iRandomCompleteCount;
	};

	void CallRandomQuestUI(bool const bAutoNext)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return;
		}

		PgRandomQuestUI kRandomQuestUI(pkMyQuest->ContRandomQuest(), pkMyQuest->RandomQuestCompleteCount());
		kRandomQuestUI.Call(QT_Random, bAutoNext);
	}

	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallRandomQuestUI", CallRandomQuestUI);
		def(pkState, "CallRandomQuestBtn", CallRandomQuestBtn);
		def(pkState, "IsCanBuildRandomQuest", IsCanBuild);
		def(pkState, "IsCanRunRandomQuest", IsCanRun);
		def(pkState, "SendReqBuildRandomQuest", SendReqBuild);
		def(pkState, "SendReqRandomQuestFunc", PgRandomQuestUI::SendReqRandomQuestFunc);
		def(pkState, "SelectRandomQuest", PgRandomQuestUI::SelectRandomQuest);
		return true;
	}
};

namespace RandomTacticsQuestUI
{
	std::wstring const kRandomTacticsQuestBtnName( L"BTN_DAILY_TACTICS_RANDOM_QUEST" );
	void CallRandomTacticsQuestBtn(bool bTwinkle)
	{
		if( IsCanRun()
		&&	!RandomQuestUI::IsHaveTypeQuest(QT_RandomTactics) )
		{
			CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRandomTacticsQuestBtnName);
			if( !pkTopWnd )
			{
				pkTopWnd = XUIMgr.Call(kRandomTacticsQuestBtnName);
			}
			if( pkTopWnd
			&&	bTwinkle )
			{
				DWORD const dwTwinkleTime = 10000;
				DWORD const dwTwinkleInter = 100;
				pkTopWnd->SetTwinkle(true);
				pkTopWnd->TwinkleTime(dwTwinkleTime);
				pkTopWnd->TwinkleInterTime(dwTwinkleInter);
			}
		}
		else
		{
			XUIMgr.Close(kRandomTacticsQuestBtnName);
		}
	}
	void CallRandomTacticsQuestUI(bool const bAutoNext)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return;
		}

		RandomQuestUI::PgRandomQuestUI kRandomQuestUI(pkMyQuest->ContTacticsQuest(), pkMyQuest->TacticsQuestCompleteCount());
		kRandomQuestUI.Call(QT_RandomTactics, bAutoNext);
	}

	bool IsCanBuild()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		return !pkMyQuest->BuildedTacticsQuest();
	}
	bool IsCanRun()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
		{
			return false;
		}
		if( BM::GUID::IsNull(pkPlayer->GuildGuid()) )
		{
			return false;
		}
		return pkMyQuest->BuildedTacticsQuest() && (pkMyQuest->TacticsQuestCompleteCount() < MAX_QUEST_RANDOM_EXP_COUNT); // true(진행가능) / false(완료)
	}
	void SendReqBuild()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( true == pkPlayer->GetMyQuest()->BuildedTacticsQuest()
		&&	!RandomQuest::IsHasRebuildItem(pkPlayer, QT_RandomTactics, UICT_REBUILD_RAND_QUEST)
		&&	!RandomQuest::IsHasRebuildItem(pkPlayer, QT_RandomTactics, UICT_REBUILD_RAND_QUEST_CONSUME) )
		{
			int const iNoticeLevel = 2;
			g_kChatMgrClient.AddMessage(799103, SChatLog(CT_EVENT_SYSTEM), true, iNoticeLevel);
			return;
		}
		BM::Stream kPacket(PT_C_M_REQ_RANDOMTACTICSQUEST_BUILD);
		NETWORK_SEND(kPacket);
	}

	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallRandomTacticsQuestUI", CallRandomTacticsQuestUI);
		def(pkState, "CallRandomTacticsQuestBtn", CallRandomTacticsQuestBtn);
		def(pkState, "IsCanBuildTacticsRandomQuest", IsCanBuild);
		def(pkState, "IsCanRunTacticsRandomQuest", IsCanRun);
		def(pkState, "SendReqBuildTacticsRandomQuest", SendReqBuild);
		return true;
	}
};


// 현상 수배 퀘스트
namespace WantedQuestUI
{
	int iLastSelectIndex = 0;
	std::wstring const kWantedQuestUIName( L"FRM_WANTED_QUEST" );
	std::wstring const kWantedQuestBtnName( L"BTN_WANTED_QUEST" );

	void RenderMonsterFace( const SQuestDialog* pkDialog )
	{
		if( !pkDialog )
		{
			return ;
		}

		char const *szWantedUIModelName = "QuestWanted";
		char const *szWantedActorName = "WantedActor";

		PgUIModel* pkUIModel = g_kUIScene.FindUIModel( szWantedUIModelName );
		if( !pkUIModel )
		{
			return;
		}
		
		if( pkUIModel->AddActorByID( szWantedActorName, MB( pkDialog->kDialogText.front().kFaceID ) ) )
		{
			if( pkDialog->kDialogText.front().iFaceAniID )
			{
				pkUIModel->SetTargetAnimation( szWantedActorName, pkDialog->kDialogText.front().iFaceAniID );
			}
			else
			{
				int iFaceAniID = 0;
				if( QDT_Wanted == pkDialog->eType )
				{
					iFaceAniID = pkUIModel->GetAnimationSeqID( MB(pkDialog->kDialogText.front().kFaceID), "attk_01_02");
					if( !iFaceAniID )
					{
						iFaceAniID = pkUIModel->GetAnimationSeqID( MB(pkDialog->kDialogText.front().kFaceID), "attk_02_02");
					}
					if( !iFaceAniID )
					{
						iFaceAniID = pkUIModel->GetAnimationSeqID( MB(pkDialog->kDialogText.front().kFaceID), "shot_01");
					}
				}
				else if( QDT_Wanted_Complete == pkDialog->eType )
				{
					iFaceAniID = pkUIModel->GetAnimationSeqID( MB(pkDialog->kDialogText.front().kFaceID), "die_01");
				}

				if( iFaceAniID )
				{
					pkUIModel->SetTargetAnimation( szWantedActorName, iFaceAniID );
				}
			}

			NiCamera *pkCamera = pkUIModel->GetCamera();
			if( pkCamera
				&&	POINT3() != pkDialog->kDialogText.front().kCamPos )
			{
				pkCamera->SetTranslate( pkDialog->kDialogText.front().kCamPos.x, pkDialog->kDialogText.front().kCamPos.y, pkDialog->kDialogText.front().kCamPos.z );
				pkCamera->Update( 0.f );
			}
			else
			{
				NiActorManager* pkAM = pkUIModel->GetActor( szWantedActorName );
				if( pkAM )
				{
					NiAVObject* pkRoot = pkAM->GetNIFRoot();
					if( pkRoot )
					{
						pkRoot->SetTranslate( NiPoint3( 0.f, 0.f, -25.f ) );
					}
				}
			}
		}
	}

	void GetWantedQuestCustomData( XUI::CXUI_Wnd* pkItemWnd, int& iBuildIndex, int &iQuestID )
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkItemWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();

		kCustomData.Pop( iBuildIndex );
		kCustomData.Pop( iQuestID );
	}

	void SendReqBuild()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_WANTEDQUEST_BUILD);
		NETWORK_SEND(kPacket);
	}

	void SendReqWantedQuestFunc()
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return ;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get( kWantedQuestUIName );
		if( !pkTopWnd )
		{
			return;
		}

		EQuestState eState = QS_None;
		int iQuestID = 0;
		int iBuildIndex = 0;
		
		BM::vstring vStr(_T("FRM_WANTED_QUEST_ITEM"));
		vStr += iLastSelectIndex;
		std::wstring wstrLastName = (std::wstring const&)vStr;
		CXUI_Wnd* pkBuildWnd = pkTopWnd->GetControl( wstrLastName );
		if( pkBuildWnd )
		{// Select Frm
			GetWantedQuestCustomData( pkBuildWnd, iBuildIndex, iQuestID );
		}

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( iQuestID );
		if( !pkQuestInfo )
		{
			return ;
		}

		if( QT_Wanted != pkQuestInfo->Type() )
		{
			return ;
		}

		eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );

		switch( eState )
		{
		case QS_Begin:
			{
				pkTopWnd->Close();
				BM::Stream kPacket( PT_C_M_REQ_BEGIN_WANTEDQUEST, iQuestID );
				NETWORK_SEND( kPacket );
			}break;
		case QS_End:
			{
				if( g_pkWorld
					&& 0 != (g_pkWorld->GetAttr() & GATTR_FLAG_MISSION ) 
					)
				{
					g_kChatMgrClient.AddMessage( 19936, SChatLog( CT_EVENT_SYSTEM ), true );
					return ;
				}
				
				pkTopWnd->Close();
				BM::Stream kPacket( PT_C_M_REQ_END_WANTEDQUEST, iQuestID );
				NETWORK_SEND( kPacket );
			}break;
		default:
			{
			}break;
		}
	}

	bool IsCanBuild()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}

		if( !pkMyQuest->ExistCanBuildWantedQuest() )
		{
			return false;
		}

		return !pkMyQuest->BuildedWantedQuest();
	}

	bool IsCanRun()
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}
		return pkMyQuest->BuildedWantedQuest();
	}

	void SetWantedQuestCustomData( XUI::CXUI_Wnd* pkItemWnd, const int iBuildIndex, int const iQuestID )
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( iBuildIndex );
		kCustomData.Push( iQuestID );
		pkItemWnd->SetCustomData( kCustomData.Data() );
	}

	void SelectWantedQuest( lwUIWnd kSelectWnd )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get( kWantedQuestUIName );
		if( !pkTopWnd )
		{
			return;
		}

		int iQuestID = 0;
		int iBuildIndex = 0;
		GetWantedQuestCustomData( kSelectWnd(), iBuildIndex, iQuestID );
		iLastSelectIndex = iBuildIndex; // 선택한 퀘스트의 빌드인덱스 저장

		// 수락 버튼
		CXUI_Button* pkBtnAccept = dynamic_cast<CXUI_Button*>(pkTopWnd->GetControl(_T("BTN_ACCEPT")));
		if( pkBtnAccept )
		{
			std::wstring wstrState;
			EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );
			switch( eState )
			{
			case QS_Begin:
				{// 수락[F1]
					wstrState += _T("{C=0xFF482E26/}") + TTW(20000) + _T("{C=0xFF9e2100/}") +_T("[F1]");
					pkBtnAccept->Text( wstrState );
				}break;
			case QS_Ing:
				{// 진행
					wstrState += _T("{C=0xFF482E26/}") + TTW(19999) + _T("{C=0xFF9e2100/}");
					pkBtnAccept->Text( wstrState );
				}break;
			case QS_End:
				{// 완료[F1]
					wstrState += _T("{C=0xFF482E26/}") + TTW(20001) + _T("{C=0xFF9e2100/}") +_T("[F1]");
					pkBtnAccept->Text( wstrState );					
				}break;
			case QS_Finished:
				{// 달성
					wstrState += _T("{C=0xFF482E26/}") + TTW(19998) + _T("{C=0xFF9e2100/}");
					pkBtnAccept->Text( wstrState );
				}
			default:
				{
				}break;
			}
		}

		CONT_DEF_QUEST_WANTED const *pkContWantedQuest = NULL;
		g_kTblDataMgr.GetContDef(pkContWantedQuest);
		if( pkContWantedQuest )
		{
			int iRewardClearCount = 0;
			int iItemNo = 0;

			CONT_DEF_QUEST_WANTED::const_iterator find_iter = pkContWantedQuest->find( iQuestID );
			if( pkContWantedQuest->end() != find_iter )
			{
				CONT_DEF_QUEST_WANTED::mapped_type const& rkQuestWanted = (*find_iter).second;
				iRewardClearCount = rkQuestWanted.iClearCount;

				iItemNo = rkQuestWanted.iItemNo;
				int const iBaseClass = pkPlayer->GetAbil(AT_BASE_CLASS);
				if( UCLASS_SHAMAN == iBaseClass ||
					UCLASS_DOUBLE_FIGHTER == iBaseClass )
				{
					iItemNo = rkQuestWanted.iDra_ItemNo;
				}

				CXUI_Wnd* pkParent = kSelectWnd.GetParent().GetSelf();
				if( !pkParent )
				{
					return ;
				}

				PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( iQuestID );

				{
					// Face
					std::wstring kContentsText;
					SQuestDialog const* pkInfoDialog = NULL;

					EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );
					int iDialogID = static_cast<int>(QRDID_Begin);
					if( QS_Finished == eState )
					{
						iDialogID = static_cast<int>(QRDID_CanComplete);
					}
					bool bFindDialog = pkQuestInfo->GetDialog( iDialogID, pkInfoDialog );
					if( bFindDialog )
					{
						RenderMonsterFace( pkInfoDialog );
					}

					// 체포 딱지
					CXUI_Wnd* pkMark = pkTopWnd->GetControl(_T("FRM_MARK"));
					if( pkMark )
					{	
						std::wstring wstrState;
						EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );

						if( QS_Finished == eState )
						{
							pkMark->Visible(true);
						}
						else
						{
							pkMark->Visible(false);
						}
					}

					// 퀘스트 목표
					bFindDialog = pkQuestInfo->GetDialog( static_cast<int>(QRDID_Info), pkInfoDialog );
					if( bFindDialog )
					{
						CXUI_Wnd* pkInfo = pkTopWnd->GetControl(_T("FRM_QUEST_INFO"));
						if( pkInfo )
						{
							pkInfo->Text( TTW( pkInfoDialog->kDialogText.front().iTextNo ) );
						}
					}
				}
				
				{// 경험치
					CXUI_Wnd* pkFrmExp = pkParent->GetControl(_T("FRM_EXP"));
					if( pkFrmExp )
					{
						CXUI_Wnd* pkFrmExpNum = pkFrmExp->GetControl(_T("FRM_EXP_NUM"));
						if( pkFrmExpNum )
						{
							int const iRewardExp = PgQuestInfoUtil::GetQuestRewardExp( pkQuestInfo, pkPlayer->GetMyQuest() );
							if( iRewardExp )
							{
								BM::vstring vstrExp(iRewardExp);
								pkFrmExpNum->Text(vstrExp);
							}
						}
					}
				}

				{// 골드
					CXUI_Wnd* pkFrmGold = pkParent->GetControl(_T("FRM_GOLD"));
					if( pkFrmGold )
					{
						CXUI_Wnd* pkGoldNum = pkFrmGold->GetControl(_T("FRM_GOLD_NUM"));
						CXUI_Wnd* pkSilverNum = pkFrmGold->GetControl(_T("FRM_SILVER_NUM"));
						CXUI_Wnd* pkBronzeNum = pkFrmGold->GetControl(_T("FRM_BRONZE_NUM"));

						int const iRewardMoney = pkQuestInfo->m_kReward.iMoney;
						if( iRewardMoney )
						{
							std::vector< std::wstring > kVec;
							BreakMoney_WStr(iRewardMoney, kVec);

							pkGoldNum->Text(kVec[0]);
							pkSilverNum->Text(kVec[1]);
							pkBronzeNum->Text(kVec[2]);
						}						
					}
				}

				{// 보상 아이템
					CXUI_Wnd* pkFrmItem = pkParent->GetControl(_T("FRM_ITEM"));
					if( pkFrmItem )
					{
						CXUI_Icon* pkItemIcon = dynamic_cast<CXUI_Icon*>(pkFrmItem->GetControl(_T("ICN_ITEM")));
						if( pkItemIcon )
						{
							pkItemIcon->SetCustomData( &iItemNo, sizeof(iItemNo) );
						}

						CXUI_Wnd* pkItemDesc = pkFrmItem->GetControl(_T("FRM_ITEM_DESC")); // 000번 클리어시 몬스터 카드 획득
						if( pkItemDesc )
						{
							std::wstring  kTemp;

							if( iRewardClearCount )
							{
								if( FormatTTW( kTemp, 406149, iRewardClearCount ) )
								{
									pkItemDesc->Text( kTemp );
								}
							}
							else
							{
								pkItemDesc->Text(TTW(406159));
							}							
						}
					}
				}

				{// 현재 클리어 횟수
					CXUI_Wnd* pkFrmClearCount = pkParent->GetControl(_T("FRM_CLEAR_COUNT"));
					if( pkFrmClearCount )
					{
						int iNowClearCount = pkPlayer->GetWantedQuestClearCount(iQuestID);
						std::wstring  kTemp;
						if( FormatTTW( kTemp, 406147, iNowClearCount ) )
						{
							pkFrmClearCount->Text( kTemp );
						}
					}
				}
			}
		}		
	}

	class PgWantedQuestUI
	{
	public: 
		PgWantedQuestUI( ContQuestID const& rkContQuest )
			: m_kContQuest( rkContQuest )
		{
			
		}
		~PgWantedQuestUI()
		{
		}

	public:
		bool IsExistBeginOrEnd()
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return false;
			}

			ContQuestID::const_iterator loop_iter = m_kContQuest.begin();			
			while( m_kContQuest.end() != loop_iter )
			{
				const int iQuestID = (*loop_iter);
				EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );

				if( QS_Begin == eState || QS_End == eState )
				{
					return true;
				}

				++loop_iter;
			}

			return false;
		}

		bool IsAllFinished()
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return false;
			}

			ContQuestID::const_iterator loop_iter = m_kContQuest.begin();			
			while( m_kContQuest.end() != loop_iter )
			{
				const int iQuestID = (*loop_iter);
				EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );

				if( QS_Finished != eState )
				{
					return false;
				}

				++loop_iter;
			}

			return true;
		}

		void Call()
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			CXUI_Wnd* pkTopWnd = XUIMgr.Get(kWantedQuestUIName);
			if( !pkTopWnd )
			{
				pkTopWnd = XUIMgr.Call(kWantedQuestUIName);
				if( !pkTopWnd )
				{
					return;
				}
			}

			std::sort( m_kContQuest.begin(), m_kContQuest.end(), DailyQuestUI::SortDailyQuest );

			int iBuildIndex = 0;
			ContQuestID::const_iterator loop_iter = m_kContQuest.begin();			
			while( m_kContQuest.end() != loop_iter )
			{
				BM::vstring vStr(_T("FRM_WANTED_QUEST_ITEM"));
				vStr += iBuildIndex;

				std::wstring wstrFrmName = (std::wstring const&)vStr;
				CXUI_Wnd* pkBuildWnd = pkTopWnd->GetControl( wstrFrmName );
				if( !pkBuildWnd )
				{
					return ;
				}

				const int iQuestID = (*loop_iter);
				PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( iQuestID );	// 퀘스트 정보를 얻어와.

				SetWantedQuestCustomData( pkBuildWnd, iBuildIndex, iQuestID );

				int iMonsterNo = 0;
				if( NULL != pkQuestInfo )
				{
					ContQuestMonster::const_iterator itor = pkQuestInfo->m_kDepend_Monster.begin();	// 현상수배 퀘스트는 몬스터 종류가 하나니까 처음만 꺼내도 돼.
					if( itor != pkQuestInfo->m_kDepend_Monster.end() )
					{
						iMonsterNo = (*itor).second.iClassNo;						
					}
				}

				GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
				CMonsterDef const *pMonsterDef = kMonsterDefMgr.GetDef(iMonsterNo);
				if( !pMonsterDef )
				{
					return ;
				}

				{// 몬스터 아이콘
					CXUI_Icon* pkMonsterIcon = dynamic_cast<CXUI_Icon*>(pkBuildWnd->GetControl(_T("ICN_MONSTER")));
					if( pkMonsterIcon )
					{
						PgPilotMan::stClassInfo kClassInfo;
						g_kPilotMan.GetClassInfo(iMonsterNo, 0, kClassInfo);
						char const* pcPath = PgXmlLoader::GetPathByID( kClassInfo.m_kActorID.c_str() );

						if( pcPath )
						{
							SResourceIcon kRscIcon;
							SIconImage kIconImage;
							std::wstring strPath = UNI(pcPath);
							PgWorldMapPopUpUI::FindMonsterFromXMLPath(kRscIcon, kIconImage, strPath);
							
							pkMonsterIcon->Visible(true);
							SIconInfo kIconInfo;
							kIconInfo.iIconGroup = KUIG_MONSTER;
							kIconInfo.iIconKey = iMonsterNo;
							kIconInfo.iIconResNumber = iMonsterNo;
							pkMonsterIcon->SetIconInfo(kIconInfo);
							pkMonsterIcon->UVInfo(SUVInfo(kIconImage.iMaxX, kIconImage.iMaxY, kRscIcon.iIdx));
							NiSourceTexture const* tex = g_kNifMan.GetTexture(MB(kIconImage.wstrPath));

							if (tex)
							{
								pkMonsterIcon->ImgSize(POINT2(tex->GetWidth(), tex->GetHeight()));
								pkMonsterIcon->DefaultImgName(kIconImage.wstrPath);		
							}
						}
					}
				}

				{// 몬스터이름
					wchar_t const * pkMonName = NULL;
					if( GetDefString( pMonsterDef->NameNo(), pkMonName ) )
					{
						CXUI_Wnd* pkMonsterName = pkBuildWnd->GetControl(_T("FRM_MONSTER_NAME"));
						if( pkMonsterName )
						{
							pkMonsterName->Text(pkMonName);
						}
					}
				}

				{// State
					CXUI_Wnd* pkState = pkBuildWnd->GetControl(_T("FRM_QUEST_STATE"));					
					if( pkState )
					{
						std::wstring wstrState;
						EQuestState eState = RandomQuestUI::PgRandomQuestUI::GetRandomQuestStatus( pkPlayer, iQuestID );
						switch( eState )
						{
						case QS_Begin:
							{
								bool bIsCoolTime = false;

								if( bIsCoolTime )
								{// 대기중
									wstrState += _T("{C=0xFFC7B299/}") + TTW(19996) + _T("{C=0xFFFFFFFF/}");
									pkState->Text( wstrState );
								}
								else
								{//시작 가능
									wstrState += _T("{C=0xFF00FF00/}") + TTW(19997) + _T("{C=0xFFFFFFFF/}");
									pkState->Text( wstrState );
								}
							}break;
						case QS_Ing:
							{//진행
								wstrState += _T("{C=0xFFFFFF00/}") + TTW(19999) + _T("{C=0xFFFFFFFF/}");
								pkState->Text( wstrState );
							}break;
						case QS_End:
							{//완료(1회)
								wstrState += _T("{C=0xFF00FFFF/}") + TTW(20001) + _T("{C=0xFFFFFFFF/}");								
								pkState->Text( wstrState );								
							}break;
						case QS_Finished:
							{// 목표 카운트 도달해서 진짜 완료
								wstrState += _T("{C=0xFF009CFF/}") + TTW(19998) + _T("{C=0xFFFFFFFF/}");								
								pkState->Text( wstrState );
							}break;
						case QS_Failed:
							{// 실패
								wstrState += _T("{C=0xFFFF0000/}") + TTW(20002) + _T("{C=0xFFFFFFFF/}");								
								pkState->Text( wstrState );
							}break;
						default:
							{
							}break;
						}
					}
				}

				{// 완료 횟수 표시
					CXUI_AniBar* pkAniBar = dynamic_cast<CXUI_AniBar*>(pkBuildWnd->GetControl(_T("ANB_PERCENT")));
					if( pkAniBar )
					{
						CONT_DEF_QUEST_WANTED const *pkContWantedQuest = NULL;
						g_kTblDataMgr.GetContDef(pkContWantedQuest);
						if( pkContWantedQuest )
						{
							int iRewardClearCount = 0;
							int iNowClearCount = 0;

							CONT_DEF_QUEST_WANTED::const_iterator find_iter = pkContWantedQuest->find( iQuestID );
							if( pkContWantedQuest->end() != find_iter )
							{
								CONT_DEF_QUEST_WANTED::mapped_type const& rkQuestWanted = (*find_iter).second;
								iRewardClearCount = rkQuestWanted.iClearCount;					// 목표 카운트
								iNowClearCount = pkPlayer->GetWantedQuestClearCount(iQuestID);	// 현재 카운트
								
								if( iRewardClearCount )
								{
									pkAniBar->Now(iNowClearCount);
									pkAniBar->Max(iRewardClearCount);

									std::wstring kTemp;
									if( FormatTTW( kTemp, 406152, iNowClearCount, iRewardClearCount ) )
									{
										pkAniBar->Text(kTemp);
									}
								}
							}
						}
					}
				}

				pkBuildWnd->Visible(true);

				++iBuildIndex;
				++loop_iter;
			}

			{// 첫번째 목록이 선택이 되도록
				iLastSelectIndex = 0;
				BM::vstring vStr(_T("FRM_WANTED_QUEST_ITEM"));
				vStr += iLastSelectIndex;
				std::wstring wstrLastName = (std::wstring const&)vStr;
				CXUI_Wnd* pkBuildWnd = pkTopWnd->GetControl( wstrLastName );
				if( pkBuildWnd )
				{
					SelectWantedQuest( pkBuildWnd );
				}
			}
		}

	private:
		ContQuestID m_kContQuest;
	};

	void CallWantedQuestUI()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return;
		}

		PgWantedQuestUI kWantedQuestUI( pkMyQuest->ContWantedQuest() );
		if( kWantedQuestUI.IsAllFinished() )
		{	
			return ;
		}

		kWantedQuestUI.Call();
	}

	void CallWantedQuestBtn(bool bTwinkle)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
		{
			return;
		}

		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return;
		}

		if( !pkMyQuest->BuildedWantedQuest() )
		{
			return ;
		}
		
		PgWantedQuestUI kWantedQuestUI( pkMyQuest->ContWantedQuest() );
		if( !kWantedQuestUI.IsExistBeginOrEnd() )
		{// 수락가능하거나 완료가능한 퀘스트가 있을 때만 하단 UI 호출
			XUIMgr.Close(kWantedQuestBtnName);
			return ;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get( kWantedQuestBtnName );
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call( kWantedQuestBtnName );
		}
		if( pkTopWnd
			&&	bTwinkle )
		{
			DWORD const dwTwinkleTime = 10000;
			DWORD const dwTwinkleInter = 100;
			pkTopWnd->SetTwinkle(true);
			pkTopWnd->TwinkleTime(dwTwinkleTime);
			pkTopWnd->TwinkleInterTime(dwTwinkleInter);
		}
	}

	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "SelectWantedQuest", SelectWantedQuest);
		def(pkState, "CallWantedQuestUI", CallWantedQuestUI);
		def(pkState, "CallWantedQuestBtn", CallWantedQuestBtn);
		def(pkState, "IsCanBuildWantedQuest", IsCanBuild);
		def(pkState, "IsCanRunWantedQuest", IsCanRun);
		def(pkState, "SendReqBuildWantedQuest", SendReqBuild);
		def(pkState, "SendReqWantedQuestFunc", SendReqWantedQuestFunc);
		return true;
	}	
};