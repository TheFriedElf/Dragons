#include "stdafx.h"
#include "lwUI.h"
#include "PgChatMgrClient.h"
#include "PgMToMChat.h"
#include "PgFriendMgr.h"
#include "PgGuild.h"
#include "lwMToMChat.h"
#include "PgEventTimer.h"
#include "PgRenderMan.h"
#include "PgEmporiaMgr.h"
#include "PgNetwork.h"

char const * EVENT_EM_NOTICE_REMAINTIME = "EVENT_EMPORIABATTLE_NOTICE_REMAINTIME";

void SetEmporiaBattleBodyText(XUI::CXUI_Wnd * pkWnd)
{
	if( !pkWnd ){ return; }
	
	XUI::CXUI_Wnd *pkNameControl = pkWnd->GetControl( L"FRM_NAME_TEXT" );
	if( !pkNameControl ){ return; }
	
	__int64 const i64BattleTime = g_kGuildMgr.GetEmporiaInfo().i64BattleTime;
	__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
	DWORD const iTime = std::max<DWORD>(0, (i64BattleTime-i64NowTime)/CGameTime::SECOND);
	std::wstring kRemainTime;
	TimeToString(iTime, kRemainTime);

	std::wstring kTimeText(TTW(73099));
	kTimeText += L"\n\n";
	if( !kRemainTime.empty() )
	{
		BM::vstring vTime(TTW(73100));
		vTime.Replace(L"#TIME#", kRemainTime);
		kTimeText += static_cast<std::wstring>(vTime);
	}
	else
	{
		kTimeText += TTW(73101);
	}

	pkNameControl->Text( kTimeText );
}

void SetEmporiaBattleAlram(bool bClear=false)
{
	if(bClear)
	{
		std::string kScendID;
		if( g_kRenderMan.GetFirstTypeID<PgWorld>(kScendID) )
		{
			g_kEventTimer.Del(kScendID.c_str(), EVENT_EM_NOTICE_REMAINTIME);
			return;
		}
	}

	__int64 const i64BattleTime = g_kGuildMgr.GetEmporiaInfo().i64BattleTime;
	__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
	int const iRemainTimeMin = std::max<int>(0,static_cast<int>((i64BattleTime-i64NowTime) / CGameTime::MINUTE));

	int iMin = 0;
	if(iRemainTimeMin >= 30)		{ iMin = 30; }
	else if(iRemainTimeMin >= 10)	{ iMin = 10; }
	else if(iRemainTimeMin >= 5)	{ iMin =  5; }
	else if(iRemainTimeMin >= 3)	{ iMin =  3; }
	else if(iRemainTimeMin >= 1)	{ iMin =  1; }

	if(iMin > 0)
	{
		std::wstring wstrNotice;
		WstringFormat( wstrNotice, MAX_PATH, TTW(71018).c_str(), iMin );
		
		float fRemainTime = iRemainTimeMin * 60 * 1.f;
		int const iLevel = 1;
		char szEventScript[MAX_PATH] = {0, };
		sprintf_s(szEventScript, MAX_PATH, "NoticeEmporiaBattleRemainTime(WideString('%s'))", MB(wstrNotice));

		STimerEvent kTimerEvent;
		kTimerEvent.Set(fRemainTime, szEventScript);
		g_kEventTimer.AddLocal(EVENT_EM_NOTICE_REMAINTIME, kTimerEvent);
	}
}

void lwMToMChatClose(lwUIWnd UIParent)
{
	lwGUID	Guid = UIParent.GetOwnerGuid();
	if( Guid.IsNil() )
	{
		return;
	}

	g_kMToMMgr.DelID(Guid());
	g_kMToMMgr.Del(Guid());
	UIParent.Close();
}

void lwMToMMapMoveReCheck()
{
	kMToMChatUserContainer	kCont;
	if( g_kMToMMgr.GetChatDlgList(kCont) )
	{
		kMToMChatUserContainer::const_iterator	iter = kCont.begin();
		for( ; iter != kCont.end() ; ++iter )
		{
			if( g_kMToMMgr.IsChatDlgState( iter->kGuid ) )
			{
				//	확대
				switch ( iter->kType )
				{
				case MTM_MSG_EMPORIABATTLE:
					{
						if ( g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE) )
						{
							if ( g_kGuildMgr.GetEmporiaInfo().IsNowBattle() )
							{
								g_kMToMMgr.SetChatDlgState( iter->kGuid, true);
								XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( szDefault_QuestionName, false );
								if ( pkWnd )
								{
									pkWnd->OwnerGuid( iter->kGuid );
									pkWnd->SetCustomData( &(iter->kType), sizeof(iter->kType) );

									SetEmporiaBattleBodyText( pkWnd );
								}

								SetEmporiaBattleAlram();
							}
							else
							{
								g_kMToMMgr.DelID( iter->kGuid );
								g_kMToMMgr.Del( iter->kGuid );
							}
						}	
					}break;
				case MTM_MSG_DEFAULT:
				default:
					{
						MToMChatNewDlg( lwGUID(iter->kGuid), iter->kType );
						g_kMToMMgr.AddAllList(iter->kGuid);
					}break;
				}
			}
			else
			{
				//	축소
				MToMChatNewDlgMini( lwGUID(iter->kGuid), iter->kType );
			}
		}
	}
}

void lwMToMChatDlgStateChange(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd *pkWnd = UIParent();
	if ( pkWnd )
	{
		BM::GUID const kGuid = pkWnd->OwnerGuid();
		if ( BM::GUID::IsNotNull( kGuid ) )
		{
			BYTE kType = MTM_MSG_DEFAULT;
			pkWnd->GetCustomData( &kType, sizeof(kType) );

			g_kMToMMgr.DelID( kGuid );

			if ( MTM_MSG_EMPORIABATTLE == kType )
			{
				if ( !g_kGuildMgr.GetEmporiaInfo().IsNowBattle() )
				{
					pkWnd->Close();
					g_kMToMMgr.DelID( kGuid );
					g_kMToMMgr.Del( kGuid );
					return;
				}
			}
			else if ( MTM_MSG_EM_ADMERCENARY == kType )
			{
				if( !XUIMgr.Get(L"FRM_GUILDWAR") )
				{
					g_kEmporiaMgr.SetViewADMercenaryType(ADMT_WINDOW);
					XUIMgr.Call(L"FRM_GUILDWAR");
				}

				lwMToMChatClose(pkWnd);
				return;
			}

			if( g_kMToMMgr.IsChatDlgState(kGuid) )
			{//	축소
				g_kMToMMgr.SetArriveNew(kGuid, false);
				MToMChatNewDlgMini( kGuid, kType );
			}
			else
			{//	확대
				switch ( kType )
				{
				case MTM_MSG_EMPORIABATTLE:
					{
						g_kMToMMgr.SetChatDlgState( kGuid, true);
						XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( szDefault_QuestionName, false );
						if ( pkWnd )
						{
							pkWnd->OwnerGuid( kGuid );
							pkWnd->SetCustomData( &kType, sizeof(kType) );

							SetEmporiaBattleBodyText( pkWnd );
							SetEmporiaBattleAlram();
						}
					}break;
				case MTM_MSG_DEFAULT:
				default:
					{
						MToMChatNewDlg( lwGUID(kGuid), kType );
						g_kMToMMgr.AddAllList(kGuid);
					}break;
				}
			}
		}

		pkWnd->Close();
	}
}

void MToMChatUIListClear(lwUIWnd& UIParent)
{
	lwUIWnd UIList = UIParent.GetControl("LIST_CHAT_MSG");
	if( UIList.IsNil() )
	{
		return;
	}
	UIList.ClearAllListItem();
}

void MToMChatUIListAdd(lwUIWnd& UIParent, BM::GUID const& Guid, SMtoMChatData const& ChatData)
{
	lwUIWnd UIList = UIParent.GetControl("LIST_CHAT_MSG");
	if( UIList.IsNil() )
	{
		return;
	}

	lwUIListItem lwItem = UIList.AddNewListItem(_T(""));
	if( lwItem.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* pItem = lwItem.GetWnd().GetSelf();

	std::wstring wstrText = _T("[") + ChatData.kName + _T("] : ") + ChatData.kContants;
	pItem->FontColor(ChatData.dwColor);
	pItem->Text(wstrText);
	pItem->Size(pItem->Size().x, pItem->TextRect().y + 1);

	while( UIList.GetListItemCount() > MAX_MToM_CHAT_LIST )
	{
		UIList.ListDeleteItem(UIList.ListFirstItem());
	}

	XUI::CXUI_List*	pList = dynamic_cast<XUI::CXUI_List*>(UIList.GetSelf());
	if( pList )
	{
		pList->DisplayStartPos(pList->GetTotalItemLen() - pList->Height());
		UIList.AdjustMiddleBtnPos();
	}
}

void lwMToMMiniToolTip(lwUIWnd UIParent, int const iTextNo, lwPoint2 Pt)
{
	XUI::CXUI_Wnd *pWnd = UIParent();
	if( !pWnd ){ return; }

	BYTE kType = 0;
	pWnd->GetCustomData( &kType, sizeof(kType) );

	std::wstring wstrName;
	switch(kType)
	{
	case MTM_MSG_EMPORIABATTLE:
		{
			__int64 const i64BattleTime = g_kGuildMgr.GetEmporiaInfo().i64BattleTime;
			__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
			DWORD const iTime = (i64BattleTime-i64NowTime) / CGameTime::SECOND;
			if(iTime > 0)
			{
				std::wstring kRemainTime;
				TimeToString(iTime, kRemainTime);
				BM::vstring vTime(TTW(74018));
				vTime.Replace(L"#TIME#", kRemainTime);

				wstrName = std::wstring(vTime);
			}
			else
			{
				wstrName = TTW(73101);
			}
		}break;
	case MTM_MSG_EM_ADMERCENARY:
		{
			wstrName = TTW(70090);
		}break;
	default:
		{
			lwGUID Guid = UIParent.GetOwnerGuid();			
			g_kMToMMgr.GetName(Guid(), wstrName);
		}break;
	}

	lwCallMutableToolTipByText(lwWString(wstrName), Pt);
}

void lwMToMMiniFlash(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd *pkWnd = UIParent();
	if ( pkWnd )
	{
		if( g_kMToMMgr.IsArriveNew( pkWnd->OwnerGuid() ) )
		{
			size_t iIndex = pkWnd->UVInfo().Index;
			if ( iIndex % 2 )
			{// 홀수면 증가
				++iIndex;
			}
			else
			{
				--iIndex;
			}

			pkWnd->UVUpdate( iIndex );
		}
	}
}

void lwMToMRefreshUI(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd *pkWnd = UIParent();
	if( !pkWnd ){ return; }

	BYTE kType = 0;
	pkWnd->GetCustomData( &kType, sizeof(kType) );

	switch(kType)
	{
	case MTM_MSG_EMPORIABATTLE:
		{
			if( g_kGuildMgr.GetEmporiaInfo().IsNowBattle() )
			{
				SetEmporiaBattleBodyText( pkWnd );
			}	
		}break;
	case MTM_MSG_DEFAULT:
	default:
		{
		}break;
	}
}

void lwMToMSendChatLog(lwUIWnd UIParent)
{
	lwUIWnd UIEdit = UIParent.GetControl("EDT_CHAT_INPUT");
	if( UIEdit.IsNil() )
	{
		return;
	}

	lwUIWnd UIName = UIParent.GetControl("FRM_NAME_TEXT");
	if( UIName.IsNil() )
	{
		return;
	}

	lwGUID Guid = UIParent.GetOwnerGuid();
	std::wstring wstrName = UIName.GetStaticText()();

	SMtoMChatData	ChatData;
	ChatData.kContants = UIEdit.GetEditText()();
	if( !ChatData.kContants.size() )
	{
		//lwClearEditFocus();
		return;
	}

	if( g_kChatMgrClient.SendMToMChat(ChatData.kContants, ChatData.dwColor, wstrName) )
	{
		UIEdit.SetEditText("");
		//g_kMToMMgr.Add(Guid(), wstrName, ChatData);
	}
}

void MToMChatNewDlg( lwGUID Guid, BYTE const kType )
{
	std::wstring kName;
	if( !g_kChatMgrClient.Name2Guid_Find_ByGuid(Guid(), kName) )
	{
		return;
	}

	g_kMToMMgr.Add( Guid(), kName, kType );
	g_kMToMMgr.SetChatDlgState(Guid(), true);

	std::wstring kID;
	if( !g_kMToMMgr.FindID(Guid(), kID) )
	{
		if( !g_kMToMMgr.AddID(Guid(), kID) )
		{
			return;
		}
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( szDefault_BigFormName, false, kID );
	if ( pkWnd )
	{
		pkWnd->OwnerGuid( Guid() );
		if ( kType )
		{
			pkWnd->SetCustomData( &kType, sizeof(kType) );
		}
		XUI::CXUI_Wnd *pkNameControl = pkWnd->GetControl( L"FRM_NAME_TEXT" );
		if ( pkNameControl )
		{
			pkNameControl->Text( kName );
		}
	}
}

void lwMToMChatNewDlg(lwGUID Guid)
{
	SFriendItem	rkFriend;
	if( g_kFriendMgr.Friend_Find_ByGuid(Guid(), rkFriend) )
	{
		if(!(rkFriend.ChatStatus() & FCS_BLOCK_NO_SEND)
		&&  (rkFriend.ChatStatus() & FCS_ADD_ALLOW)
		&& !(rkFriend.ChatStatus() & FCS_BLOCK_NO_RECV))
		{
			MToMChatNewDlg( Guid, 0 );
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 551002, true);
		}
	}
	else
	{
		MToMChatNewDlg( Guid, 0 );
	}
}

XUI::CXUI_Wnd* MToMChatNewDlgMini( lwGUID Guid, BYTE const kType )
{
	if ( MTM_MSG_EMPORIABATTLE == kType )
	{
		T_GNDATTR const kGndAttr = g_pkWorld ? g_pkWorld->GetAttr() : GATTR_DEFAULT;
		T_GNDATTR const kFlag = GATTR_FLAG_MYHOME | GATTR_FLAG_EMPORIABATTLE;
		if( kGndAttr & kFlag  )
		{
			return NULL;
		}

		if( !g_kGuildMgr.GetEmporiaInfo().IsNowBattle() )
		{
			g_kMToMMgr.DelID( Guid() );
			g_kMToMMgr.Del( Guid() );
			return NULL;
		}
	}
	else if( MTM_MSG_EM_ADMERCENARY == kType )
	{
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
		{
			return NULL;
		}
	}

	g_kMToMMgr.SetChatDlgState(Guid(), false);

	std::wstring kID;
	if( !g_kMToMMgr.FindID(Guid(), kID) )
	{
		if( !g_kMToMMgr.AddID(Guid(), kID) )
		{
			return NULL;
		}
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( szDefault_SmallFormName, false, kID );
	if ( pkWnd )
	{
		pkWnd->OwnerGuid( Guid() );
		if ( kType )
		{
			pkWnd->UVUpdate( static_cast<size_t>(kType) );
			pkWnd->SetCustomData( &kType, sizeof(kType) );
		}
		MToMAlignMiniDlg();
	}
	return pkWnd;
}

void MToMAlignMiniDlg()
{
	int DEFAULT_X = 634;
	int DEFAULT_Y = 660;
	int const LINE_MAX_ITEM = 7;

	if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
	{
		DEFAULT_X = 424;
		DEFAULT_Y = 730;
	}

	kIDContainer	kIDCont;
	g_kMToMMgr.GetIDList(false, kIDCont);

	if( !kIDCont.size() )
	{
		return;
	}

	kIDContainer::iterator	iter = kIDCont.begin();
	int iIndex = 0;
	while( iter != kIDCont.end() )
	{
		XUI::CXUI_Wnd * pkWnd = XUIMgr.Get( *iter );
		if ( pkWnd )
		{
			POINT2 const &ptSize = pkWnd->Size();
			POINT2 ptPos( DEFAULT_X + (ptSize.x * (iIndex % LINE_MAX_ITEM)), DEFAULT_Y - (ptSize.y * (iIndex / LINE_MAX_ITEM)) );
			CalcAbosolute(ptPos,true,false);
			pkWnd->Location( ptPos );
			++iIndex;
		}
		++iter;
	}
}
