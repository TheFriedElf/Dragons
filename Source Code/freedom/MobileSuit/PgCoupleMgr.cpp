#include "stdafx.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "PgCoupleMgr.h"
#include "lwUI.h"
#include "lwCouple.h"
#include "PgWorld.h"
#include "lwUIQuest.h"
#include "PgOption.h"

extern std::wstring const GetMapName(int const iMapNo);
extern void CallMapMoveCause( BYTE const byCause );

std::wstring const COUPLE_AUTO_MATCH_WND_NAME = _T("SFRM_COUPLE_AUTO_MATCH");
std::wstring const COUPLE_FIND_WND_NAME = _T("SFRM_COUPLE_FIND");


namespace PgCoupleMgrUtil
{
	int const iCoupleWarpEffectSlotID = 8385;
	char const* szCoupleWarpEffectID = "ef_couple_heart_warp_01";
	char const* szCoupleWarpSoundID = "Couple_Warf";
	char const* szCoupleWarpEffectNodeID = "char_root";

	bool AttachCoupleWarpEffect(PgActor* pkActor)
	{
		if( !pkActor )
		{
			return false;
		}
		pkActor->AddNewParticle(szCoupleWarpEffectID, iCoupleWarpEffectSlotID, szCoupleWarpEffectNodeID, pkActor->GetEffectScale(),false,true,0.f,false,false,true);
		pkActor->PlayNewSound(NiAudioSource::TYPE_3D, szCoupleWarpSoundID, 0.0f); // Sound
		return true;
	}
}


PgCoupleMgr::PgCoupleMgr()
{
	Clear();
}

PgCoupleMgr::~PgCoupleMgr()
{
}

void PgCoupleMgr::Breaked(BM::GUID const &rkCharGuid)
{
	BM::GUID kCoupleGuid;
	bool const bIsMyPlayer = g_kPilotMan.IsMyPlayer(rkCharGuid);
	{
		if( g_pkWorld )
		{
			PgActor* pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject( rkCharGuid ));
			if( pkActor )
			{
				PgPilot *pkPilot = pkActor->GetPilot();
				if( pkPilot )
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>( pkPilot->GetUnit() );
					if( pkPlayer )
					{
						kCoupleGuid = pkPlayer->CoupleGuid();
						pkPlayer->CoupleGuid( BM::GUID::NullData() );
						pkActor->UpdateName();

						if( bIsMyPlayer )
						{
							PgMySkill *pkMySkill = pkPlayer->GetMySkill();
							if( pkMySkill )
							{
								pkMySkill->EraseSkill( SDT_Couple );
								g_kSkillTree.CreateSkillTree( pkPlayer);
							}
						}
					}
				}
			}
		}
	}

	{
		if( g_pkWorld )
		{
			PgActor* pkCoupleActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject( kCoupleGuid ));
			if( pkCoupleActor )
			{
				PgPilot *pkCouplePilot = pkCoupleActor->GetPilot();
				if( pkCouplePilot )
				{
					PgPlayer *pkCouplePlayer = dynamic_cast<PgPlayer*>( pkCouplePilot->GetUnit() );
					if( pkCouplePlayer )
					{
						pkCouplePlayer->CoupleGuid( BM::GUID::NullData() );
						pkCoupleActor->UpdateName();
					}
				}
			}
		}
	}

	if( bIsMyPlayer )
	{
		//g_kChatMgrClient.Name2Guid_Remove( m_kMyCouple.CoupleGuid() );
		m_kMyCouple.Clear();
	}
}

void PgCoupleMgr::Clear()
{
	m_kMyCouple.Clear();
	m_kFindResult.clear();
	m_kCurPage = 0;
}

void PgCoupleMgr::UpdateBtnWnd(XUI::CXUI_Button *pkBtnWnd, SContentsUser const &rkUser) const
{
	if( !pkBtnWnd )
	{
		return;
	}

	pkBtnWnd->OwnerGuid( rkUser.kCharGuid );
	pkBtnWnd->Visible( !rkUser.Empty() );
}

void PgCoupleMgr::UpdateItemWnd(XUI::CXUI_Wnd *pkItemWnd, SContentsUser const &rkUser) const
{
	if( !pkItemWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkLocationWnd = pkItemWnd->GetControl(_T("SFRM_LOCATION"));
	XUI::CXUI_Wnd *pkNameWnd = pkItemWnd->GetControl(_T("SFRM_NAME"));
	XUI::CXUI_Wnd *pkLevelWnd = pkItemWnd->GetControl(_T("SFRM_LEVEL"));

	BM::vstring const kLevel(rkUser.sLevel);

	std::wstring const &rkMapName = (!rkUser.Empty())? GetMapName(rkUser.kGndKey.GroundNo()): std::wstring();
	std::wstring const &rkName = (!rkUser.Empty())? rkUser.kName: std::wstring();
	std::wstring const &rkLevel = (!rkUser.Empty())? (std::wstring)kLevel: std::wstring();

	if( pkLocationWnd )
	{// 지역 이름이 너무 길면 말줄임표를 사용
		//pkLocationWnd->Text( rkMapName ); 
		int const iWidth = pkLocationWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pkLocationWnd, rkMapName, WSTR_UI_REGULATIONSTR, iWidth);
	}
	if( pkNameWnd )
	{ // 캐릭터 이름이 너무 길면 말줄임표를 사용
		//pkNameWnd->Text( rkName ); 
		int const iWidth = pkNameWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pkNameWnd, rkName, WSTR_UI_REGULATIONSTR, iWidth);
	}
	if( pkLevelWnd )		{ pkLevelWnd->Text( rkLevel ); }
}

void PgCoupleMgr::CallAutoMatch(ContContentsUser const &rkVec)const
{
	if( false == g_bUseCoupleUI )
	{
		return;
	}
	XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Get(COUPLE_AUTO_MATCH_WND_NAME);
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(COUPLE_AUTO_MATCH_WND_NAME);
		if( !pkTopWnd )
		{
			return;
		}
	}

	size_t iMax = 5;
	ContContentsUser::const_iterator match_iter = rkVec.begin();
	for( size_t iCur = 0; iMax > iCur; ++iCur )
	{
		BM::vstring kWndName(_T("FRM_INFO_"));
		BM::vstring kBtnName(_T("BTN_INFO_BTN_"));
		kWndName += (int)iCur;
		kBtnName += (int)iCur;
		XUI::CXUI_Wnd *pkItemWnd = pkTopWnd->GetControl((std::wstring)kWndName);
		XUI::CXUI_Button *pkBtnWnd = dynamic_cast<XUI::CXUI_Button*>( pkTopWnd->GetControl((std::wstring)kBtnName) );

		if( rkVec.size() > iCur )
		{
			ContContentsUser::value_type const &rkUser = rkVec.at(iCur);
			UpdateItemWnd(pkItemWnd, rkUser);
			UpdateBtnWnd(pkBtnWnd, rkUser);
		}
		else
		{
			UpdateItemWnd(pkItemWnd, SContentsUser());
			UpdateBtnWnd(pkBtnWnd, SContentsUser());
		}
	}
}

void PgCoupleMgr::CallFind(ContContentsUser const &rkVec, size_t const iPage) const
{
	if( false == g_bUseCoupleUI )
	{
		return;
	}
	XUI::CXUI_Wnd *pkFindTopWnd = XUIMgr.Get( COUPLE_FIND_WND_NAME );
	if( !pkFindTopWnd )
	{
		pkFindTopWnd = XUIMgr.Call( COUPLE_FIND_WND_NAME );
		if( !pkFindTopWnd )
		{
			return;
		}
	}

	size_t const iStartIdx = iPage * iFindOnePageItems;
	ContContentsUser::const_iterator match_iter = rkVec.begin();
	for( size_t iCur = 0; iFindOnePageItems > iCur; ++iCur )
	{
		BM::vstring kWndName(_T("FRM_INFO_"));
		BM::vstring kBtnName(_T("BTN_INFO_BTN_"));
		kWndName += (int)iCur;
		kBtnName += (int)iCur;
		XUI::CXUI_Wnd *pkItemWnd = pkFindTopWnd->GetControl((std::wstring)kWndName);
		XUI::CXUI_Button *pkBtnWnd = dynamic_cast<XUI::CXUI_Button*>( pkFindTopWnd->GetControl((std::wstring)kBtnName) );

		if( rkVec.size() > (iStartIdx + iCur) )
		{
			ContContentsUser::value_type const &rkUser = rkVec.at(iStartIdx + iCur);
			UpdateItemWnd(pkItemWnd, rkUser);
			UpdateBtnWnd(pkBtnWnd, rkUser);
		}
		else
		{
			UpdateItemWnd(pkItemWnd, SContentsUser());
			UpdateBtnWnd(pkBtnWnd, SContentsUser());
		}
	}

	XUI::CXUI_Wnd *pkPageNumWnd = pkFindTopWnd->GetControl( _T("SFRM_PAGE_NO") );
	if( pkPageNumWnd )
	{
		size_t const iTotalPage = (rkVec.size() / iFindOnePageItems) + 1;
		TCHAR szTemp[512] = {0, };
		_stprintf_s(szTemp, _T("{C=0xFFA0410D/}%d{C=0xFF4D3413/}/%d"), iPage+1, iTotalPage);
		pkPageNumWnd->Text(szTemp);
	}
}

bool PgCoupleMgr::ProcessPacket(BM::Stream &rkPacket)
{
	BYTE cCmdType = 0;
	BYTE cCmdResult = 0;

	rkPacket.Pop(cCmdType);
	rkPacket.Pop(cCmdResult);

	switch( cCmdType )
	{
	case CC_Auto_Match:			{ RecvAutoMatch(cCmdResult, rkPacket); }break;
	case CC_Nfy_AddSkill:		{ RecvAddSkill(cCmdResult, rkPacket); }break;
	case CC_Nfy_TimeLimit:		{ RecvTimeLimit(cCmdResult, rkPacket); }break;
	case CC_Req_Break:			{ RecvReqBreak(cCmdResult, rkPacket); }break;
	case CC_Req_Couple:			{ RecvReqCouple(cCmdResult, rkPacket); }break;
	case CC_Ans_Couple:			{ RecvAnsCouple(cCmdResult, rkPacket); }break;
	case CC_Req_Find:			{ RecvReqFind(cCmdResult, rkPacket); }break;
	case CC_Req_Info:			{ RecvReqInfo(cCmdResult, rkPacket); }break;
	case CC_Req_Warp:			{ RecvReqWarp(cCmdResult, rkPacket); }break;
	case CC_Status:				{ RecvReqStatus(cCmdResult, rkPacket); }break;
	case CC_Req_SweetHeart_Quest:			{ RecvReqSweetHeartQuest(cCmdResult, rkPacket); }break;
	case CC_Ans_SweetHeart_Quest:			{ RecvAnsSweetHeartQuest(cCmdResult, rkPacket); }break;
	case CC_SweetHeartQuestTimeInfo:		{ RecvSweetHeartQuestTimeInfo(cCmdResult, rkPacket); }break;
	case CC_Req_SweetHeart_Complete:		{ RecvReqSweetHeartComplete(cCmdResult, rkPacket); }break;
	case CC_Ans_SweetHeart_Complete:		{ RecvAnsSweetHeartComplete(cCmdResult, rkPacket); }break;
	case CC_SendGndCoupleStatus:			{ RecvGndCoupleStatus(cCmdResult, rkPacket); }break;
	case CC_CoupleInit:						{ RecvCoupleInit(cCmdResult, rkPacket); }break;
	default:
		{
			return false;
		}break;
	}

	lwCouple::UpdateCoupleUI(NULL);
	SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();

	return true;
}

void PgCoupleMgr::RecvAutoMatch(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	ContContentsUser kVec;
	PU::TLoadArray_M(rkPacket, kVec);

	if( !kVec.empty() )
	{
		CallAutoMatch(kVec);
	}
}

void PgCoupleMgr::RecvReqSweetHeartComplete(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;

	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			std::wstring kHimName;

			rkPacket.Pop( kHimName );

			// 연인 조건 신청을 성공 했어
			int const iTTW = 450100;
			std::wstring kTempText;
			if( FormatTTW(kTempText, iTTW, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_Notify:
		{
			BM::GUID kHimGuid;
			std::wstring kHimName;
			int iQuestID = 0;

			rkPacket.Pop( kHimGuid );
			rkPacket.Pop( kHimName );
			rkPacket.Pop( iQuestID ); 

			// 연인 조건 신청
			int const iTTW = 450097;
			SChatLog kChatLog(CT_EVENT);
			if( FormatTTW(kChatLog.kContents, iTTW, kHimName.c_str()) )
			{
				CallYesNoMsgBox(kChatLog.kContents, kHimGuid, MBT_CONFIRM_SWEETHEART_COMPLETE, iQuestID);
			}
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 실패
			// 상대방을 못찾았어
			iTTW = 700013;
		}break;
	}
}

void PgCoupleMgr::RecvGndCoupleStatus(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			BM::GUID kCharGuid;
			BYTE cStatus = 0;
			BM::GUID kColorGuid;

			rkPacket.Pop( kCharGuid );
			rkPacket.Pop( kColorGuid );
			rkPacket.Pop( cStatus );

			if( g_kPilotMan.IsMyPlayer( kCharGuid ) )
			{
				PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
				if( pkPC )
				{
					pkPC->ForceSetCoupleColorGuid( kColorGuid );
					pkPC->SetCoupleStatus( cStatus );
				}
			}
			else
			{
				PgPilot* pkPilot = g_kPilotMan.FindPilot(kCharGuid);
				if( pkPilot )
				{
					PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
					if( pkPC )
					{
						pkPC->ForceSetCoupleColorGuid( kColorGuid );
						pkPC->SetCoupleStatus( cStatus );
					}
				}
			}
		}break;
	default:
		{
		}break;
	}
}

void PgCoupleMgr::RecvAnsSweetHeartComplete(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			std::wstring kNameLeft = _T("");
			int iLeftGender = 0;
			std::wstring kNameRight = _T("");
			int iRightGender = 0;

			rkPacket.Pop(kNameLeft);
			rkPacket.Pop(iLeftGender);
			rkPacket.Pop(kNameRight);
			rkPacket.Pop(iRightGender);

			// 연인 연출 시작하자 //
			lwCloseUI("SFRM_SWEETHEART_IF");

			char *pszText = "FRM_SWEETHEART_PLAY";

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;
			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				lwCallUI(pszText, false);

				lwUIWnd	UIAuction = lwGetUIWnd("FRM_SWEETHEART_PLAY");
				if( UIAuction.IsNil() )
				{
					return;
				}

				lwUIWnd UILeft = UIAuction.GetControl("FRM_SWEETHEART_TEXT1");
				if( UILeft.IsNil() )
				{
					return;
				}
				lwUIWnd UIRight = UIAuction.GetControl("FRM_SWEETHEART_TEXT2");
				if( UIRight.IsNil() )
				{
					return;
				}

				char const *wstr1 = MB(kNameLeft);
				char const *wstr2 = MB(kNameRight);

				if( 1 == iLeftGender )
				{
					UILeft.SetStaticText(wstr1);
					UIRight.SetStaticText(wstr2);
				}
				else
				{
					UILeft.SetStaticText(wstr2);
					UIRight.SetStaticText(wstr1);
				}
			}
		}break;
	case CoupleCR_NotGender:
		{
			// 동성 간에는 커플 안되
			iTTW = 450046;
		}break;
	case CoupleCR_Reject:
		{
			// 거절 당했어
			std::wstring kHimName;

			rkPacket.Pop( kHimName );

			std::wstring kTempText;
			if( FormatTTW(kTempText, 450099, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 응답 상대를 못찾았어
			// 응답 신청을 실패 했어
			iTTW = 700013;
		}break;
	}

	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

void PgCoupleMgr::RecvReqSweetHeartQuest(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_ReqWait:
		{
			// 커플을 신청한 상태입니다. 수락 대기상태입니다.
			iTTW = 450054;
		}break;
	case CoupleCR_Pre:
		{
			// 먼저 커플 깨고서 신청해
			iTTW = 450035;
		}break;
	case CoupleCR_NotGender:
		{
			// 동성 간에는 커플 안되
			iTTW = 450046;
		}break;
	case CoupleCR_NotSingle:
		{
			// 상대방은 임자 있어
			iTTW = 450041;
		}break;
	case CoupleCR_Success:
		{
			std::wstring kHimName;

			rkPacket.Pop( kHimName );

			// 연인 조건 신청을 성공 했어
			int const iTTW = 450078;
			std::wstring kTempText;
			if( FormatTTW(kTempText, iTTW, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_Notify:
		{
			BM::GUID kHimGuid;
			std::wstring kHimName;
			int iQuestID = 0;

			rkPacket.Pop( kHimGuid );
			rkPacket.Pop( kHimName );
			rkPacket.Pop( iQuestID ); 

			// 연인 조건 신청
			int const iTTW = 450075;
			SChatLog kChatLog(CT_EVENT);
			if( FormatTTW(kChatLog.kContents, iTTW, kHimName.c_str()) )
			{
				CallYesNoMsgBox(kChatLog.kContents, kHimGuid, MBT_CONFIRM_SWEETHEART_QUEST, iQuestID);
			}
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 실패
			// 상대방을 못찾았어
			iTTW = 700013;
		}break;
	}
	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

void PgCoupleMgr::RecvSweetHeartQuestTimeInfo(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	__int64 iQuestPlayTime = 0;

	rkPacket.Pop( iQuestPlayTime );

	char *pszText = "FRM_SWEETHEART_PLAYTIME";
	//char *pszTextBtn = "FRM_SWEETHEART_PLAYTIME_COMPLETE";
	char *pszTextIF = "SFRM_SWEETHEART_IF";

	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			m_kMyCouple.SweetHeartQuestTime( iQuestPlayTime );
			SetSweetHeartQuestTimeUsed( true );

			lwCloseUI(pszTextIF);
			//lwCloseUI(pszTextBtn);

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;
			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				lwCallUI(pszText, false);
			}
		}break;
	case CoupleCR_None:
		{
			m_kMyCouple.SweetHeartQuestTime( iQuestPlayTime );
			SetSweetHeartQuestTimeUsed( false );

			lwCloseUI(pszTextIF);
			lwCloseUI(pszText);
			//lwCloseUI(pszTextBtn);
		}break;
	case CoupleCR_Failed:
		{
			m_kMyCouple.SweetHeartQuestTime( iQuestPlayTime );
			SetSweetHeartQuestTimeUsed( false );

			//lwCloseUI(pszTextBtn);

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;
			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				lwCallUI(pszText, false);
			}
		}break;
	case CoupleCR_QuestComplete:
		{
			m_kMyCouple.SweetHeartQuestTime( iQuestPlayTime );
			SetSweetHeartQuestTimeUsed( false );

			lwCloseUI(pszTextIF);
			lwCloseUI(pszText);

			//std::wstring const wstr = UNI(pszTextBtn);
			//XUI::CXUI_Wnd* pWnd = NULL;
			//if(!XUIMgr.IsActivate(wstr, pWnd))
			//{
			//	lwCallUI(pszTextBtn, false);
			//}
		}break;
	default:
		{
		}break;
	}
}

void PgCoupleMgr::RecvAnsSweetHeartQuest(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			// 당신 연인 퀘스트 수락 됬어

			BM::GUID kMineGuid;
			BM::GUID kHimGuid;
			std::wstring kMineName;
			std::wstring kHimName;

			rkPacket.Pop( kMineGuid );
			rkPacket.Pop( kHimGuid );
			rkPacket.Pop( kMineName );
			rkPacket.Pop( kHimName );

			//
			bool const bIsMine = g_kPilotMan.IsMyPlayer(kMineGuid);
			if( bIsMine )
			{
				int const iCoupleTTW = 450076;
				std::wstring kTempText;
				if( FormatTTW(kTempText, iCoupleTTW, kHimName.c_str()) )
				{
					g_kChatMgrClient.LogMsgBox(kTempText);
				}
			}
		}break;
	case CoupleCR_NotGender:
		{
			// 동성 간에는 커플 안되
			iTTW = 450046;
		}break;
	case CoupleCR_Reject:
		{
			// 거절 당했어
			std::wstring kHimName;

			rkPacket.Pop( kHimName );

			std::wstring kTempText;
			if( FormatTTW(kTempText, 450077, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 응답 상대를 못찾았어
			// 응답 신청을 실패 했어
			iTTW = 700013;
		}break;
	}

	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

void PgCoupleMgr::RecvReqCouple(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_ReqWait:
		{
			// 커플을 신청한 상태입니다. 수락 대기상태입니다.
			iTTW = 450054;
		}break;
	case CoupleCR_Pre:
		{
			// 먼저 커플 깨고서 신청해
			iTTW = 450035;
		}break;
	case CoupleCR_NotGender:
		{
			// 동성 간에는 커플 안되
			iTTW = 450046;
		}break;
	case CoupleCR_NotSingle:
		{
			// 상대방은 임자 있어
			iTTW = 450041;
		}break;
	case CoupleCR_Success:
		{
			std::wstring kHimName;
			bool bInstance = false;

			rkPacket.Pop( kHimName );
			rkPacket.Pop( bInstance );

			// 커플 신청을 성공 했어
			int const iTTW = (bInstance)? 450059: 450040;
			std::wstring kTempText;
			if( FormatTTW(kTempText, iTTW, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_Notify:
		{
			BM::GUID kHimGuid;
			std::wstring kHimName;
			bool bInstance = false;

			rkPacket.Pop( kHimGuid );
			rkPacket.Pop( kHimName );
			rkPacket.Pop( bInstance );

			// 누가 커플 신챙 했어
			int const iTTW = (bInstance)? 450060: 450030;
			SChatLog kChatLog(CT_EVENT);
			if( FormatTTW(kChatLog.kContents, iTTW, kHimName.c_str()) )
			{
				EMsgBoxType const eType = (bInstance)? MBT_CONFIRM_INSTANCE_COUPLE: MBT_CONFIRM_COUPLE;
				CallYesNoMsgBox(kChatLog.kContents, kHimGuid, eType);
			}
			else
			{
				lwCouple::Send_AnsCouple(kHimGuid, false, bInstance); // 이런일이 있겠어 --;;
			}
		}break;
	case CoupleCR_SweetHeart_Panalty:
		{	// 패널티 중이다.
			iTTW = 450101;
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 실패
			// 상대방을 못찾았어
			iTTW = 700013;
		}break;
	}
	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

void PgCoupleMgr::RecvAnsCouple(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			// 당신 커플 됬어

			BM::GUID kMineGuid;
			BM::GUID kHimGuid;
			std::wstring kMineName;
			std::wstring kHimName;
			bool bFirstPacket = false;
			bool bInstance = false;

			rkPacket.Pop( kMineGuid );
			rkPacket.Pop( kHimGuid );
			std::list<int> kSkillStack;
			PU::TLoadArray_A(rkPacket, kSkillStack);

			rkPacket.Pop( bFirstPacket );
			rkPacket.Pop( kMineName );
			rkPacket.Pop( kHimName );
			rkPacket.Pop( bInstance );

			if( bFirstPacket )
			{
				MakeCouple(kMineGuid, kHimGuid);
				MakeCouple(kHimGuid, kMineGuid);
			}

			//
			bool const bIsMine = g_kPilotMan.IsMyPlayer(kMineGuid);
			if( bIsMine )
			{
				int const iCoupleTTW = (bInstance)? 450061: 450031;
				std::wstring kTempText;
				if( FormatTTW(kTempText, iCoupleTTW, kHimName.c_str()) )
				{
					g_kChatMgrClient.LogMsgBox(kTempText);
				}
				PgPlayer *pkMinePlayer = g_kPilotMan.GetPlayerUnit();
				if( pkMinePlayer )
				{
					std::list<int>::const_iterator it = kSkillStack.begin();
					while(kSkillStack.end() != it)
					{
						AddSkill((*it));
						++it;
					}
				}

				lwCouple::Send_ReqInfo();						// 서버로 커플 정보 요청
				XUIMgr.Close( COUPLE_AUTO_MATCH_WND_NAME );		// 성공 하면 찾기창 닫는다
				XUIMgr.Close( COUPLE_FIND_WND_NAME );
			}
		}break;
	case CoupleCR_NotGender:
		{
			// 동성 간에는 커플 안되
			iTTW = 450046;
		}break;
	case CoupleCR_Reject:
		{
			// 거절 당했어
			std::wstring kHimName;

			rkPacket.Pop( kHimName );

			std::wstring kTempText;
			if( FormatTTW(kTempText, 450033, kHimName.c_str()) )
			{
				g_kChatMgrClient.LogMsgBox(kTempText);
			}
		}break;
	case CoupleCR_SweetHeart_Panalty:
		{	// 패널티 중이다.
			iTTW = 450101;
		}break;
	case CoupleCR_NotFind:
	case CoupleCR_Failed:
	default:
		{
			// 응답 상대를 못찾았어
			// 응답 신청을 실패 했어
			iTTW = 700013;
		}break;
	}

	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

void PgCoupleMgr::RecvReqBreak(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_Success:
	case CoupleCR_Notify:
		{
			// 커플 깨기 성공
			BM::GUID kBreakerGuid;

			rkPacket.Pop( kBreakerGuid );

			Breaked(kBreakerGuid);

			bool const bIsMine = g_kPilotMan.IsMyPlayer(kBreakerGuid);
			if( bIsMine )
			{
				if( CoupleCR_Notify == cCmdResult )
				{
					// 당신의 커플이 떠났어
					std::wstring kBreakerName;
					rkPacket.Pop( kBreakerName );

					if( !kBreakerName.empty() )
					{
						std::wstring kTempText;
						if( FormatTTW(kTempText, 450044, kBreakerName.c_str()) )
						{
							g_kChatMgrClient.LogMsgBox(kTempText);
						}
					}
					else
					{
						iTTW = 450047;
					}
				}
				else
				{
					iTTW = 450042;
				}
			}
		}break;
	case CoupleCR_None:
		{
			// 커플이 없는데 뭘 깨려고
			iTTW = 450045;
		}break;
	case CoupleCR_Failed:
	default:
		{
			// 아무튼 실패 했어
			iTTW = 700013;
		}break;
	}

	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox(iTTW);
	}
}

bool PgCoupleMgr::Have()
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		bool const bHaveCouple = BM::GUID::NullData() != pkPlayer->CoupleGuid();
		return bHaveCouple;
	}
	return false;
}

bool PgCoupleMgr::IsSweetHeart()
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
		bool const bSweetHeart = (rkMyCoupleInfo.cStatus() == (CoupleS_Normal | CoupleS_SweetHeart));
		return bSweetHeart;
	}
	return false;
}

void PgCoupleMgr::RecvReqFind(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			m_kCurPage = 0;
			m_kFindResult.clear();

			PU::TLoadArray_M(rkPacket, m_kFindResult);

			CallFind(m_kFindResult, CurPage());
		}break;
	default:
		{
			g_kChatMgrClient.LogMsgBox(450063);
		}break;
	}
}

void PgCoupleMgr::RecvCoupleInit(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			BM::PgPackedTime kNow;

			rkPacket.Pop( kNow );
			m_kMyCouple.SetPanaltyLimitDate( kNow );
		}break;
	default:
		{
		}break;
	}
}

void PgCoupleMgr::RecvReqInfo(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			// 커플 정보
			SCouple kTempCouple;
			kTempCouple.ReadFromPacket( rkPacket );
			BYTE cLinkStatus = 0;
			int iGroundNo = 0;
			cLinkStatus = m_kMyCouple.LinkStatus();
			iGroundNo = m_kMyCouple.GroundNo();

			m_kMyCouple = kTempCouple;
			m_kMyCouple.LinkStatus(cLinkStatus);
			m_kMyCouple.GroundNo(iGroundNo);

			PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
			if( g_pkWorld
				&& pkPC 
				)
			{
				pkPC->ForceSetCoupleColorGuid( m_kMyCouple.ColorGuid() );
				pkPC->SetCoupleStatus( m_kMyCouple.cStatus() );
				
				PgActor *pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(pkPC->GetID()));
				if(pkActor)
				{
					pkActor->UpdateName();
				}
			}

			//g_kChatMgrClient.Name2Guid_Add( CT_FRIEND, m_kMyCouple.CharName(), m_kMyCouple.CoupleGuid() );
		}break;
	case CoupleCR_Failed:
	case CoupleCR_None:
	default:
		{
			BM::GUID kPlayerGuid;
			if( g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid) )
			{
				Breaked(kPlayerGuid);
			}
		}break;
	}
}

void PgCoupleMgr::RecvReqStatus(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	BYTE cLinkStatus = 0;
	int iGroundNo = 0;

	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			rkPacket.Pop( cLinkStatus );
			rkPacket.Pop( iGroundNo );

			if( 0 != g_kGlobalOption.GetValue(XML_ELEMENT_ETC, "NOTIFY_COUPLE") )
			{				
				if( (0 == m_kMyCouple.GroundNo()) && !(m_kMyCouple.CharName().empty()) )
				{
					g_kChatMgrClient.ShowNotifyConnectInfo(NCI_COUPLE, m_kMyCouple.CharName(), 0, SUVInfo(), m_kMyCouple.CoupleGuid());
				}
			}

			m_kMyCouple.LinkStatus(cLinkStatus);
			m_kMyCouple.GroundNo(iGroundNo);

			lua_tinker::call<void, lwUIWnd, int>("SetCoupleIcon", lwUIWnd(NULL), iGroundNo);	//월드맵에서 커플 위치 업데이트
		}break;
	case CoupleCR_Failed:
	case CoupleCR_None:
	default:
		{
		}break;
	}
}

void PgCoupleMgr::RecvAddSkill(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
			int iCoupleSkillNo = 0;

			rkPacket.Pop( iCoupleSkillNo );

			AddSkill(iCoupleSkillNo);
		}break;
	default:
		{
			// 뭥미?
		}break;
	}
}

void PgCoupleMgr::RecvTimeLimit(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	switch( cCmdResult )
	{
	case CoupleCR_Success:
	default:
		{
			BM::GUID kCharGuid;

			rkPacket.Pop(kCharGuid);

			Breaked(kCharGuid);
			
			bool const bIsMine = g_kPilotMan.IsMyPlayer(kCharGuid);
			if( bIsMine )
			{
				g_kChatMgrClient.LogMsgBox(450058);
			}
		}break;
	}
}

void PgCoupleMgr::CallFindNext()
{
	size_t const iTestNext = (CurPage()+1) * iFindOnePageItems;
	if( m_kFindResult.size() <= iTestNext )
	{
		g_kChatMgrClient.LogMsgBox(450064);
		return;
	}
	
	++m_kCurPage;
	CallFind(m_kFindResult, CurPage());
}

void PgCoupleMgr::CallFindPrev()
{
	if( 0 == CurPage() )
	{
		g_kChatMgrClient.LogMsgBox(450065);
		return;
	}

	--m_kCurPage;
	CallFind(m_kFindResult, CurPage());
}

void PgCoupleMgr::AddSkill(int const iSkillNo)
{
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( pkPC )
	{
		PgMySkill *pkMySkill = pkPC->GetMySkill();
		if( pkMySkill )
		{
			pkMySkill->LearnNewSkill(iSkillNo);
			g_kSkillTree.CreateSkillTree( pkPC );
		}
	}
}

void PgCoupleMgr::RecvReqWarp(BYTE const cCmdResult, BM::Stream &rkPacket)
{
	int iTTW = 0;
	switch( cCmdResult )
	{
	case CoupleCR_Success:
		{
		}break;
	case CoupleCR_None:
		{
			iTTW = 450066;	// 아이템이 부족해
		}break;
	case CoupleCR_Notify:
		{
			iTTW = 450048;	// 커플이 워프 아웃 합니다.
		}break;
	case CoupleCR_Failed:
		{
			iTTW = 450045;	// 커플이나 맺고서 해
		}break;
	case CoupleCR_Reject:
		{
			iTTW = 450067;	// 상대편에게 워프 할 수 없습니다. (인던이나, PVP 중)
		}break;
	case CoupleCR_Same:
		{
			iTTW = 450070;	// 같은 그라운드에서는 못써
		}break;
	case CoupleCR_NotChannel:
		{
			iTTW = 460006;	// 다른 캐널이라서 워프 할수 없다.
		}break;
	case CoupleCR_NotMapMove:
		{
			iTTW = 10410;	// 이동할수 없는 상태
		}break;
	case CoupleCR_NotFind:
	default:
		{
			iTTW = 450069;	// 커플이 접속해 있지 않거나 다른 채널이야
		}break;
	}

	if( iTTW )
	{
		g_kChatMgrClient.LogMsgBox( iTTW );
	}
}

void PgCoupleMgr::MakeCouple(BM::GUID const &rkCharGuid, BM::GUID const &rkCoupleGuid)
{
	if( g_pkWorld )
	{
		PgActor* pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject( rkCharGuid ));
		if( pkActor )
		{
			PgPilot *pkPilot = pkActor->GetPilot();
			if( pkPilot )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>( pkPilot->GetUnit() );
				if( pkPlayer )
				{
					//bool const bSetCouple = pkPlayer->CoupleGuid() != rkCoupleGuid;
					pkPlayer->CoupleGuid(rkCoupleGuid);
					pkActor->UpdateName();

					//if( bSetCouple )
					//{
						pkActor->AddNewParticle("ef_couple_heart_begin_01", 8384, "char_root", pkActor->GetEffectScale());
						pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Couple_Make", 0.0f); // Sound
					//}
				}
			}
		}
	}
}

bool PgCoupleMgr::GetPlayTimeUse()
{
	return bPlayTimeUse;
}

void PgCoupleMgr::SetSweetHeartQuestTimeUsed(bool bUse)
{
	if( true == bUse )
	{
		m_kMyCouple.SweetHeartQuestTimeGap( g_kEventView.GetLocalSecTime(CGameTime::SECOND) );
	}
	else
	{
		m_kMyCouple.SweetHeartQuestTimeGap( 0i64 );
	}
	bPlayTimeUse = bUse;
}

void PgCoupleMgr::SweetHeartQuestTimeUpdate(__int64 const iNowTime, __int64 const PlayTimeGap)
{
	if( false == bPlayTimeUse ) 
	{
		return;
	}

	if( false == Have() )
	{
		return;
	}

	if( BM::GUID::IsNotNull( m_kMyCouple.CoupleGuid() ) )
	{
		__int64 const iPlayTimeGap = std::max( PlayTimeGap, 0i64 );
		m_kMyCouple.SweetHeartQuestTime( std::max((m_kMyCouple.SweetHeartQuestTime() - iPlayTimeGap), 0i64) );
		m_kMyCouple.SweetHeartQuestTimeGap( iNowTime );
	}
}

void PgCoupleMgr::CheckSweetHeartQuestTime()
{
	if( false == bPlayTimeUse ) 
	{
		return;
	}

	if( false == Have() )
	{
		return;
	}

	__int64 const iNowTime = g_kEventView.GetLocalSecTime(CGameTime::SECOND);
	__int64 const iPlayTime = m_kMyCouple.SweetHeartQuestTimeGap();
	if( 0 < iPlayTime )
	{
		__int64 const PlayTimeGap = std::max( iNowTime - iPlayTime, 0i64 );
		if( 0 < PlayTimeGap )
		{
			SweetHeartQuestTimeUpdate( iNowTime, PlayTimeGap );
		}
	}
}

void PgCoupleMgr::CoupleSweetHeartUI()
{
	char *pszText = "FRM_SWEETHEART_PLAYTIME";
	//char *pszTextBtn = "FRM_SWEETHEART_PLAYTIME_COMPLETE";

	lwCloseUI("SFRM_SWEETHEART_IF");

	if( 0 > m_kMyCouple.SweetHeartQuestTime() )
	{
		lwCloseUI(pszText);
		
		//std::wstring const wstr = UNI(pszTextBtn);
		//XUI::CXUI_Wnd* pWnd = NULL;
		//if(!XUIMgr.IsActivate(wstr, pWnd))
		//{
		//	lwCallUI(pszTextBtn, false);
		//}
	}
	else if( 0 < m_kMyCouple.SweetHeartQuestTime() )
	{
		//lwCloseUI(pszTextBtn);
		
		std::wstring const wstr = UNI(pszText);
		XUI::CXUI_Wnd* pWnd = NULL;
		if(!XUIMgr.IsActivate(wstr, pWnd))
		{
			lwCallUI(pszText, false);
		}
	}
}

//////////////////////////////////////////
PgMapMoveCompleteEventMgr::PgMapMoveCompleteEventMgr()
{
	Clear();
}

PgMapMoveCompleteEventMgr::~PgMapMoveCompleteEventMgr()
{
}

void PgMapMoveCompleteEventMgr::Clear()
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kEventMap.clear();
}

void PgMapMoveCompleteEventMgr::Push(key_type const &rkKey, mapped_type const &rkVal)
{
	BM::CAutoMutex kLock(m_kMutex);

	std::pair<ContEvent::iterator, bool> kRet = m_kEventMap.insert( std::make_pair(rkKey, rkVal) );
	if( !kRet.second )
	{
		iterator iter = kRet.first;
		if( m_kEventMap.end() != iter )
		{
			(*iter).second = rkVal;
		}
	}
}

bool PgMapMoveCompleteEventMgr::Pop(key_type const &rkKey)
{
	BM::CAutoMutex kLock(m_kMutex);
	if( Process(rkKey) )
	{
		m_kEventMap.erase(rkKey);
		return true;
	}
	return false;
}

bool PgMapMoveCompleteEventMgr::Process(key_type const &rkKey)
{
	BM::CAutoMutex kLock(m_kMutex);
	if(!g_pkWorld)
	{
		return	false;
	}

	iterator iter = m_kEventMap.find(rkKey);
	if( m_kEventMap.end() == iter )
	{
		return false;
	}

	mapped_type const &rkMapMoveCause = (*iter).second;

	//
	PgActor *pkActor = dynamic_cast<PgActor*>( g_pkWorld->FindObject(rkKey) );
	if( !pkActor )
	{
		return false;
	}
	PgPilot *pkPilot = pkActor->GetPilot();
	if( !pkPilot )
	{
		return false;
	}
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>( pkPilot->GetUnit() );
	if( !pkPlayer )
	{
		return false;
	}

	// 어떤 이유로 맵이동을 해서 완료를 했는가?
	// 나던 // 타인이던 상관 없이
	switch( rkMapMoveCause )
	{
	case MMET_None: //
		{
		}break;
	case MMET_Normal: // 보통의 맵이동
		{
		}break;
	case MMET_PartyWarp: // 파티원 소환 아이템
		{
		}break;
	case MMET_CoupleWarp: // 커플 워프 아이템
		{
			if( BM::GUID::IsNotNull(pkPlayer->CoupleGuid()) )
			{
				if( g_pkWorld )
				{
					PgActor *pkCoupleActor = dynamic_cast<PgActor*>( g_pkWorld->FindObject(pkPlayer->CoupleGuid()) );
					if( pkActor
					&&	pkCoupleActor ) // 둘다 있어야 처리
					{
						PgCoupleMgrUtil::AttachCoupleWarpEffect(pkActor);
						PgCoupleMgrUtil::AttachCoupleWarpEffect(pkCoupleActor);
					}
					else
					{
						if( g_kPilotMan.IsReservedPilot(pkPlayer->CoupleGuid()) )
						{
							// 상대편이 쓰레드 로딩 중이면 지연
							Push( pkPlayer->CoupleGuid(), rkMapMoveCause );
						}
						else
						{
							// 아니면 있는 사람만 처리
							PgCoupleMgrUtil::AttachCoupleWarpEffect(pkActor);
							PgCoupleMgrUtil::AttachCoupleWarpEffect(pkCoupleActor);
						}
					}
				}
			}
		}break;
	default:
		{
		}break;
	}

	
	if( g_kPilotMan.IsMyPlayer(rkKey) )
	{// 나일 때만(타인은 처리 안함)
		CallMapMoveCause( rkMapMoveCause );
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgCoupleMgr::ProcessMarryPacket(BM::Stream &rkPacket)
{
	BYTE cCmdType = 0;

	rkPacket.Pop(cCmdType);

	switch( cCmdType )
	{
	case MC_AnsMarry:
		{
			bool bRet = false;

			rkPacket.Pop( bRet );

			if( true == bRet )
			{
				g_kChatMgrClient.LogMsgBox(450123);
			}
		}break;
	case MC_AnsSendMoney:
		{
			bool bRet = false;

			rkPacket.Pop( bRet );

			if( true == bRet )
			{
				g_kChatMgrClient.LogMsgBox(450122);
			}
		}break;
	case MC_ActionPlay:
		{
			std::wstring kNameLeft = _T("");
			int iLeftGender = 0;
			std::wstring kNameRight = _T("");
			int iRightGender = 0;

			rkPacket.Pop(kNameLeft);
			rkPacket.Pop(iLeftGender);
			rkPacket.Pop(kNameRight);
			rkPacket.Pop(iRightGender);

			// 연인 연출 시작하자 //
			lwCloseUI("SFRM_SWEETHEART_IF");

			char *pszText = "FRM_SWEETHEART_PLAY";

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;
			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				lwCallUI(pszText, false);

				lwUIWnd	UIAuction = lwGetUIWnd("FRM_SWEETHEART_PLAY");
				if( UIAuction.IsNil() )
				{
					break;
				}

				lwUIWnd UILeft = UIAuction.GetControl("FRM_SWEETHEART_TEXT1");
				if( UILeft.IsNil() )
				{
					break;
				}
				lwUIWnd UIRight = UIAuction.GetControl("FRM_SWEETHEART_TEXT2");
				if( UIRight.IsNil() )
				{
					break;
				}

				std::string wstr1 = MB(kNameLeft);
				std::string wstr2 = MB(kNameRight);

				if( 1 == iLeftGender )
				{
					UILeft.SetStaticText(wstr1.c_str());
					UIRight.SetStaticText(wstr2.c_str());
				}
				else
				{
					UILeft.SetStaticText(wstr2.c_str());
					UIRight.SetStaticText(wstr1.c_str());
				}
			}

			lua_tinker::call<void>("WeddingFanfare");
		}break;
	case MC_ActionPlayEnd:
		{
			lwCloseUI("SFRM_SWEETHEART_IF");

			char *pszText = "FRM_SWEETHEART_PLAYEND";

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;
			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				lwCallUI(pszText, false);
			}

			lua_tinker::call<void>("WeddingBalloon");
		}break;
	case MC_TotalMoney:
		{
			__int64 const iGoldToBronze = 10000;
			__int64 i64TotalMoney = 0;

			rkPacket.Pop( i64TotalMoney );

			int iGoldMoney = static_cast<int>(i64TotalMoney / iGoldToBronze);

			lua_tinker::call<void, int>("SweetHeart_TotalMoney", (int)iGoldMoney);
		}break;
	case MC_SetActionState:
		{
			int iMsgNo = 0;

			rkPacket.Pop( iMsgNo );

			g_kChatMgrClient.LogMsgBox(iMsgNo);
		}break;
	case MC_NpcTalk:
		{
			std::wstring kCharName;
			std::wstring kCoupleName;

			rkPacket.Pop(kCharName);
			rkPacket.Pop(kCoupleName);

			lua_tinker::call<void, char const*, char const*>("SweetHeartVillageNPC_Talk", MB(kCharName), MB(kCoupleName));
		}break;
	case MC_Init:
		{			
			std::wstring kNameLeft = _T("");
			std::wstring kNameRight = _T("");

			rkPacket.Pop(kNameLeft);
			rkPacket.Pop(kNameRight);

			std::wstring kLogMessage;
			if( FormatTTW(kLogMessage, 450142, kNameLeft.c_str(), kNameRight.c_str()) )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, kLogMessage, true);
			}
		}break;
	case MC_End:
		{			
			std::wstring kNameLeft = _T("");
			std::wstring kNameRight = _T("");

			rkPacket.Pop(kNameLeft);
			rkPacket.Pop(kNameRight);

			std::wstring kLogMessage;
			if( FormatTTW(kLogMessage, 450143, kNameLeft.c_str(), kNameRight.c_str()) )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, kLogMessage, true);
			}
		}break;
	default:
		{
			return false;
		}break;
	}

	return true;
}