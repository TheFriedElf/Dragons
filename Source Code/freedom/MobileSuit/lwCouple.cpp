#include "stdafx.h"
#include "lwGuid.h"
#include "lwWString.h"
#include "lwUI.h"
#include "PgCoupleMgr.h"
#include "lwCouple.h"
#include "PgNetwork.h"
#include "Lohengrin/GameTime.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "PgChatMgrClient.h"
#include "PgActorNpc.h"

std::wstring TimeToWStr(BM::PgPackedTime const &rkTime)
{
	TCHAR szTemp[512] = {0, };
	BM::PgPackedTime const kNullTime;
	if( kNullTime == rkTime )
	{
// 		switch(g_kLocal.ServiceRegion())
// 		{
// 		case LOCAL_MGR::NC_USA:
// 			{// mm/dd/yyyy/tt
// 				_stprintf_s(szTemp, _T("-%s -%s -%s -%s"), TTW(173).c_str(), TTW(174).c_str(),TTW(172).c_str(), TTW(175).c_str() );
// 			}break;
// 		default:
// 			{// yyyy/mm/dd/tt
// 				//_stprintf_s(szTemp, _T("-%s -%s -%s, -%s -%s -%s"), TTW(172).c_str(), TTW(173).c_str(), TTW(174).c_str(), TTW(175).c_str(), TTW(8).c_str(), TTW(9).c_str() );
// 				_stprintf_s(szTemp, _T("-%s -%s -%s -%s"), TTW(172).c_str(), TTW(173).c_str(), TTW(174).c_str(), TTW(175).c_str() );
// 			};
// 		}

		_stprintf_s(szTemp, _T("-%s -%s -%s -%s"), TTW(172).c_str(), TTW(173).c_str(), TTW(174).c_str(), TTW(175).c_str() );
	}
	else
	{
// 		switch(g_kLocal.ServiceRegion())
// 		{
// 		case LOCAL_MGR::NC_USA:
// 			{// mm/dd/yyyy/tt
// 				_stprintf_s(szTemp, _T("%d%s %d%s %d%s %d%s"),
// 					rkTime.Month(), TTW(173).c_str(),
// 					rkTime.Day(), TTW(174).c_str(),
// 					BM::PgPackedTime::BASE_YEAR+rkTime.Year(), TTW(172).c_str(),
// 					rkTime.Hour(), TTW(175).c_str());
// 			}break;
// 		default:
// 			{// yyyy/mm/dd/tt
// 				//_stprintf_s(szTemp, _T("%d%s %d%s %d%s, %d%s %d%s %d%s"),
// 				_stprintf_s(szTemp, _T("%d%s %d%s %d%s %d%s"),
// 					BM::PgPackedTime::BASE_YEAR+rkTime.Year(), TTW(172).c_str(),
// 					rkTime.Month(), TTW(173).c_str(), 
// 					rkTime.Day(), TTW(174).c_str(),
// 					rkTime.Hour(), TTW(175).c_str());
// 				//rkTime.Min(), TTW(8).c_str(), 
// 				//rkTime.Sec(), TTW(9).c_str() );
// 			}break;
// 
// 		}

		_stprintf_s(szTemp, _T("%d%s %d%s %d%s %d%s"),
			BM::PgPackedTime::BASE_YEAR+rkTime.Year(), TTW(172).c_str(),
			rkTime.Month(), TTW(173).c_str(), 
			rkTime.Day(), TTW(174).c_str(),
			rkTime.Hour(), TTW(175).c_str());
	}

	return std::wstring(szTemp);
}

void lwCouple::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "CoupleDrawItem", DrawItem);
	def(pkState, "Couple_FindNextPage", Couple_FindNextPage);
	def(pkState, "Couple_FindPrevPage", Couple_FindPrevPage);
	def(pkState, "IamHaveCouple", IamHaveCouple);
	def(pkState, "ReqBreakCouple", ReqBreakCouple);
	def(pkState, "ReqWarpCouple", ReqWarpCouple);
	def(pkState, "Send_AnsCouple", Send_AnsCouple);
	def(pkState, "Send_BreakCouple", Send_BreakCouple);
	def(pkState, "Send_ReqCoupleByGuid", Send_ReqCoupleByGuid);
	def(pkState, "Send_ReqCoupleByName", Send_ReqCoupleByName);
	def(pkState, "Send_ReqFind", Send_ReqFind);
	def(pkState, "Send_WarpCouple", Send_WarpCouple);
	def(pkState, "UpdateCoupleUI", UpdateCoupleUI);
	def(pkState, "CallCoupleWarpItemToolTip", CallCoupleWarpItemToolTip);
	def(pkState, "GetMyCoupleMapNo", GetMyCoupleMapNo);
	def(pkState, "Send_ReqSweetHeartQuestByGuid", Send_ReqSweetHeartQuestByGuid);
	def(pkState, "Send_AnsSweetHeartQuest", Send_AnsSweetHeartQuest);
	def(pkState, "GetMyCouplePlayTime", GetMyCouplePlayTime);
	def(pkState, "CheckSweetHeartQuestTime", CheckSweetHeartQuestTime);
	def(pkState, "CoupleSweetHeartUI", CoupleSweetHeartUI);
	def(pkState, "SweetHeartQuestCompleteBtn", SweetHeartQuestCompleteBtn);
	def(pkState, "OnSelectSendMarry", OnSelectSendMarry);
	def(pkState, "OnSelectSendMoney", OnSelectSendMoney);
	def(pkState, "SetMarryState", SetMarryState);
	def(pkState, "SetMarryTotalMoney", SetMarryTotalMoney);
	def(pkState, "SweetHeartNPC_TalkCall", SweetHeartNPC_TalkCall);
	def(pkState, "SweetHeartVillageNPC_TalkCall", SweetHeartVillageNPC_TalkCall);
	def(pkState, "IsSweetHeart", IsSweetHeart);
}

void lwCouple::Send_ReqFind()
{
	if( IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox(450035);
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Find);
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_ReqCoupleByGuid(lwGUID kGuid, bool bInstance)
{
	if( IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox(450035);
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Couple);
	kPacket.Push( false );		// By Guid
	kPacket.Push( bInstance );
	kPacket.Push( kGuid() );
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_ReqCoupleByName(lwWString kName, bool bInstance)
{
	if( IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox(450035);
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Couple);
	kPacket.Push( true );		// By Name
	kPacket.Push( bInstance );
	kPacket.Push( kName() );
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_AnsCouple(lwGUID kGuid, bool bSayYes, bool bInstance)
{
	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Ans_Couple);
	kPacket.Push( kGuid() );
	kPacket.Push( bSayYes );
	kPacket.Push( bInstance );
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_BreakCouple()
{
	if( !IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox( 450045 );
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Break);
	NETWORK_SEND( kPacket )
}

void lwCouple::ReqBreakCouple()
{
	if( !IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox( 450045 );
		return ;
	}

	SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();

	std::wstring const &rkHimName = rkMyCoupleInfo.CharName();
	std::wstring kLogMessage;
	if( FormatTTW(kLogMessage, 450034, rkHimName.c_str()) )
	{
		CallYesNoMsgBox(kLogMessage, BM::GUID::NullData(), MBT_CONFIRM_BREAK_COUPLE);
	}
}

void lwCouple::ReqWarpCouple()
{
	if( !IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox( 450045 );
		return ;
	}

	if ( g_pkWorld )
	{
		switch( g_pkWorld->GetAttr() )
		{
		case GATTR_PVP:
		case GATTR_EMPORIABATTLE:
		case GATTR_MISSION:
		//case GATTR_CHAOS_F:
		case GATTR_CHAOS_MISSION:
		//case GATTR_HIDDEN_F:
		case GATTR_BOSS:
			{
				g_kChatMgrClient.LogMsgBox( 460007 );
				return;
			}break;
		default:
			{
			}break;
		}
	}

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( pkPlayer->IsDead() )
	{
		g_kChatMgrClient.LogMsgBox( 10410 );
		return;
	}

	SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();

	CONT_DEFMAP const* pkDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkDefMap);
	if( !pkDefMap )
	{
		return;
	}

	CONT_DEFMAP::const_iterator find_iter = pkDefMap->find(rkMyCoupleInfo.GroundNo());
	if( pkDefMap->end() == find_iter )
	{
		g_kChatMgrClient.LogMsgBox( 460007 );
		return;
	}

	if( 0 != ((*find_iter).second.iAttr&GATTR_FLAG_CANT_PARTYWARP) )
	{
		g_kChatMgrClient.LogMsgBox( 1930 );
		return;
	}

	std::wstring const &rkHimName = rkMyCoupleInfo.CharName();
	std::wstring kLogMessage;
	if( FormatTTW(kLogMessage, 450068, rkHimName.c_str()) )
	{
		CallYesNoMsgBox(kLogMessage, BM::GUID::NullData(), MBT_CONFIRM_WARP_COUPLE);
	}
}

bool lwCouple::IamHaveCouple()
{
	return g_kCoupleMgr.Have();
}

bool lwCouple::IsSweetHeart()
{
	return g_kCoupleMgr.IsSweetHeart();
}

void lwCouple::Send_ReqInfo()
{
	if( !IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox( 450045 );
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Info);
	NETWORK_SEND(kPacket)
}

void lwCouple::Send_WarpCouple()
{
	if( !IamHaveCouple() )
	{
		g_kChatMgrClient.LogMsgBox( 450045 );
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_Warp);
	kPacket.Push( COUPLE_WARP_ITEM_NO );
	NETWORK_SEND(kPacket)
}

void lwCouple::UpdateCoupleUI(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd *pkCoupleTopWnd = kWnd();
	if( !pkCoupleTopWnd )
	{
		XUI::CXUI_Wnd *pkCommunityTopWnd = XUIMgr.Get( _T("SFRM_COMMUNITY") );
		if( !pkCommunityTopWnd )
		{
			return;
		}

		pkCoupleTopWnd = pkCommunityTopWnd->GetControl( _T("FRM_COUPLE") );
		if( !pkCoupleTopWnd )
		{
			return;
		}
	}

	bool const bHaveCouple = IamHaveCouple();

	XUI::CXUI_Wnd *pkNameWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_NAME") );
	XUI::CXUI_Wnd *pkStartTimeWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_TIME1") );
	XUI::CXUI_Wnd *pkEndTimeWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_TIME2") );
	XUI::CXUI_Icon *pkCoupleSkillIcon = dynamic_cast<XUI::CXUI_Icon*>(pkCoupleTopWnd->GetControl( _T("SkillIcon") ));
	XUI::CXUI_Icon *pkCoupleSkillIconArea = dynamic_cast<XUI::CXUI_Icon*>(pkCoupleTopWnd->GetControl( _T("SkillIconArea") ));
	XUI::CXUI_Wnd *pkHelpUIWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_HELP") );
	//XUI::CXUI_Image *pkCoupleItemIcon1 = dynamic_cast<XUI::CXUI_Image*>(pkCoupleTopWnd->GetControl( _T("ICON_Item1") ));

	XUI::CXUI_Wnd *pkOnlineWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_ONLINE") );
	XUI::CXUI_Wnd *pkLocationWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_LOCATION") );
	XUI::CXUI_Wnd *pkSweetHeartBtnWnd = pkCoupleTopWnd->GetControl( _T("BTN_SWEETHEART_IF") );
	XUI::CXUI_Wnd *pkPanaltyWnd = pkCoupleTopWnd->GetControl( _T("FRM_PANALTY") );
	XUI::CXUI_Wnd* pkMyhomeWnd = pkCoupleTopWnd->GetControl( _T("SFRM_COUPLE_MYHOME") );

	SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
	if( pkMyhomeWnd )
	{
		XUI::CXUI_Wnd* pkMyHomeGoWnd = pkMyhomeWnd->GetControl( L"BTN_MYHOME_GO_NONE" );
		if( !pkMyHomeGoWnd )
		{
			return;
		}
		XUI::CXUI_Button* pkMyHomeGoBtn = dynamic_cast<XUI::CXUI_Button*>( pkMyHomeGoWnd );
		if( !pkMyHomeGoBtn )
		{
			return;
		}
		pkMyHomeGoBtn->Disable( (!bHaveCouple) || (rkMyCoupleInfo.HomeAddr().IsNull()) );

		XUI::CXUI_Button* pkMyhomeInvite = dynamic_cast<XUI::CXUI_Button*>(pkMyhomeWnd->GetControl( _T("BTN_MYHOME_INVITE") ));
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( (!pkMyhomeInvite) || (!pkPlayer) )
		{
			return;
		}
		bool const bNotHaveMyHome = (pkPlayer->HomeAddr().IsNull());
		pkMyhomeInvite->Disable( bNotHaveMyHome );
	}

	if( pkNameWnd )
	{
		if( bHaveCouple )
		{
			std::wstring kCoupleName = _T("");

			kCoupleName = rkMyCoupleInfo.CharName();

			if( !(rkMyCoupleInfo.LimitDate() == BM::PgPackedTime()) )
			{
				kCoupleName += TTW(550012);
			}
			pkNameWnd->Text( kCoupleName );
		}
		else
		{
			pkNameWnd->Text( TTW(50106) );
		}
		pkNameWnd->Visible( bHaveCouple );
	}
	if( pkStartTimeWnd )
	{
		std::wstring const kStartDateText = TimeToWStr( rkMyCoupleInfo.StartDate() );
		pkStartTimeWnd->Text( kStartDateText );
		pkStartTimeWnd->Visible( bHaveCouple );
	}
	
	if( pkEndTimeWnd )
	{
		BM::vstring kTemp(rkMyCoupleInfo.DDay());
		kTemp += TTW(174);		
		pkEndTimeWnd->Text( (std::wstring)kTemp );
		pkEndTimeWnd->Visible( bHaveCouple );
	}

	if( pkOnlineWnd )
	{
		std::wstring kCoupleName = _T("");
		kCoupleName = rkMyCoupleInfo.CharName();

		if( bHaveCouple
		&&	!kCoupleName.empty() )
		{
			std::wstring const &rkStatusName = GetCoupleLinkStatusName( rkMyCoupleInfo.LinkStatus() );
			
			pkOnlineWnd->Text( rkStatusName );
			
			XUI::CXUI_Wnd *pkImg_StatusWnd = pkOnlineWnd->GetControl( _T("IMG_STATUS") );
			if( pkImg_StatusWnd )
			{
				pkImg_StatusWnd->Visible( true );

				SUVInfo kUVInfo = pkImg_StatusWnd->UVInfo();
				kUVInfo.Index = rkMyCoupleInfo.LinkStatus();
				pkImg_StatusWnd->UVInfo(kUVInfo);
				pkImg_StatusWnd->SetInvalidate();
			}
		}
		else
		{
			pkOnlineWnd->Text( _T("") );
			XUI::CXUI_Wnd *pkImg_StatusWnd = pkOnlineWnd->GetControl( _T("IMG_STATUS") );
			if( pkImg_StatusWnd )
			{
				pkImg_StatusWnd->Visible( false );
			}
		}
	}

	if( pkLocationWnd )
	{
		std::wstring const &rkMapName = (rkMyCoupleInfo.GroundNo() && rkMyCoupleInfo.LinkStatus() != FLS_OFFLINE ) ? GetMapName(rkMyCoupleInfo.GroundNo()) : std::wstring();
		pkLocationWnd->Text( ((bHaveCouple)? rkMapName: std::wstring()) );
		if( bHaveCouple && (BM::GUID::IsNull(rkMyCoupleInfo.CoupleGuid())) && (rkMapName != _T("")) )
		{
			Send_ReqInfo();
		}
	}

	if( pkCoupleSkillIcon )
	{
		SIconInfo kIconInfo = pkCoupleSkillIcon->IconInfo();
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		const CSkillDef* pkSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
		if( pkSkillDef )
		{
			kIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
			pkCoupleSkillIcon->SetIconInfo(kIconInfo);
		}
		pkCoupleSkillIcon->GrayScale( !bHaveCouple );
	}

	if( pkCoupleSkillIconArea )
	{
		SIconInfo kIconInfo = pkCoupleSkillIconArea->IconInfo();
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		const CSkillDef* pkSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
		if( pkSkillDef )
		{
			kIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
			pkCoupleSkillIconArea->SetIconInfo(kIconInfo);
		}
		pkCoupleSkillIconArea->GrayScale( !bHaveCouple );
	}

	XUI::CXUI_Builder *pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkCoupleTopWnd->GetControl(L"BLD_EMOTION"));
	if(pkBuilder)
	{
		int const iTotal = pkBuilder->CountX()*pkBuilder->CountY();
		for(int i = 0; i < iTotal; ++i)
		{
			BM::vstring kBGTitle(L"FRM_EMOTION");
			kBGTitle+=i;
			XUI::CXUI_Wnd* pkBg = pkCoupleTopWnd->GetControl((std::wstring const&)kBGTitle);
			if(pkBg)
			{
				XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkBg->GetControl(L"ICN_SKILL"));
				if(pkIcon)
				{
					pkIcon->GrayScale( !bHaveCouple );
				}
			}
		}
	}

	if( pkHelpUIWnd )
	{
		pkHelpUIWnd->Visible( !bHaveCouple );
	}

	if( pkSweetHeartBtnWnd )
	{
		pkSweetHeartBtnWnd->Visible(false);

		if( rkMyCoupleInfo.cStatus() == (CoupleS_Normal) )
		{
			pkSweetHeartBtnWnd->Visible( bHaveCouple );
		}
	}

	if( pkPanaltyWnd )
	{
		pkPanaltyWnd->Visible(false);

		if( rkMyCoupleInfo.cStatus() != (CoupleS_Normal | CoupleS_SweetHeart) )
		{
			BM::PgPackedTime const kNullTime;
			if( !(kNullTime == rkMyCoupleInfo.CouplePanaltyLimitDate()) )
			{
				SYSTEMTIME kNowTime;
				g_kEventView.GetLocalTime(&kNowTime);	

				BM::PgPackedTime kNow(kNowTime);
				bool const bLimited = kNow < rkMyCoupleInfo.CouplePanaltyLimitDate();
				if( bLimited )
				{
					pkPanaltyWnd->Visible(true);

					BM::PgPackedTime const rkTime(rkMyCoupleInfo.CouplePanaltyLimitDate());

					TCHAR szTemp[512] = {0, };
					_stprintf_s(szTemp, _T("%d%s %d%s %d%s %d%s %d%s"),
						BM::PgPackedTime::BASE_YEAR+rkTime.Year(), TTW(172).c_str(),
						rkTime.Month(), TTW(173).c_str(), 
						rkTime.Day(), TTW(174).c_str(),
						rkTime.Hour(), TTW(175).c_str(),
						rkTime.Min(), TTW(8).c_str());

					pkPanaltyWnd->Text( TTW(450102) + szTemp );
				}
			}
		}
	}



	//if( pkCoupleItemIcon1 )
	//{
	//	int const iCoupleWarpItemNo = COUPLE_WARP_ITEM_NO;
	//	pkCoupleItemIcon1->SetCustomData( &iCoupleWarpItemNo, sizeof(iCoupleWarpItemNo) );

	//	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	//	if( pkPlayer )
	//	{
	//		PgInventory *pkInven = pkPlayer->GetInven();
	//		if( pkInven )
	//		{
	//			int const iItemCount = (int)pkInven->GetTotalCount(iCoupleWarpItemNo);
	//			BM::vstring const kCountText = iItemCount;
	//			pkCoupleItemIcon1->Text( (std::wstring)kCountText );
	//			pkCoupleItemIcon1->GrayScale( 0 == iItemCount );
	//		}
	//	}
	//}
}

std::wstring lwCouple::GetCoupleLinkStatusName(BYTE iLinkStatus)
{
	return TTW(iLinkStatus + 20);
}

void lwCouple::Couple_FindNextPage()
{
	g_kCoupleMgr.CallFindNext();
}

void lwCouple::Couple_FindPrevPage()
{
	g_kCoupleMgr.CallFindPrev();
}

void lwCouple::DrawItem(lwUIWnd kImgWnd, int const iItemNo, bool const bSetItemCount)
{
	if( !kImgWnd() )
	{
		return;
	}
	XUI::CXUI_Wnd *pkImageWnd = kImgWnd();
	//int const iCoupleWarpItemNo = COUPLE_WARP_ITEM_NO;

	PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(iItemNo);
	if (!pkSprite) 
	{ 
		pkImageWnd->DefaultImgTexture(NULL);
		pkImageWnd->SetInvalidate();
		return; 
	}
	PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
	if(!pkUVSprite) { return; }
	pkImageWnd->DefaultImgTexture(pkUVSprite);

	SUVInfo &rkUV = pkUVSprite->GetUVInfo();
	pkImageWnd->UVInfo(rkUV);
	POINT2 kPoint(40*rkUV.U, 40*rkUV.V);//이미지 원본 사이즈 복구
	pkImageWnd->ImgSize(kPoint);
	pkImageWnd->SetInvalidate();
	pkImageWnd->SetCustomData(&iItemNo, sizeof(iItemNo));

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory *pkInven = pkPlayer->GetInven();
		if( pkInven )
		{
			int const iItemCount = (int)pkInven->GetTotalCount(iItemNo);
			BM::vstring const kCountText(iItemCount);
			pkImageWnd->Text( ((bSetItemCount)? (std::wstring)kCountText: std::wstring()) );
			bool const bEnableUse = !IamHaveCouple() || (0 == iItemCount);
			pkImageWnd->GrayScale( bEnableUse );
		}
	}
}

void lwCouple::CallCoupleWarpItemToolTip(lwUIWnd kControl)
{
	XUI::CXUI_Wnd *pkWnd = kControl();
	if( pkWnd )
	{
		int iItemNo = 0;;
		pkWnd->GetCustomData(&iItemNo, sizeof(iItemNo));
		CallToolTip_ItemNo(iItemNo, kControl.GetLocation());
	}
	else
	{
		NILOG(PGLOG_ERROR, "kControl is NULL"); assert(0);
	}
}

int const lwCouple::GetMyCoupleMapNo()
{
	if( IamHaveCouple() )
	{
		SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
		return rkMyCoupleInfo.GroundNo();
	}

	return 0;
}

void lwCouple::Send_ReqSweetHeartQuestByGuid(int const iQuestID)
{
	if( !IamHaveCouple() )
	{
		//g_kChatMgrClient.LogMsgBox(450035);
		return;
	}

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_SweetHeart_Quest);
	kPacket.Push( pkPlayer->CoupleGuid() );
	kPacket.Push( iQuestID );
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_AnsSweetHeartQuest(lwGUID kGuid, bool bSayYes, int iQuestID)
{
	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Ans_SweetHeart_Quest);
	kPacket.Push( kGuid() );
	kPacket.Push( iQuestID );
	kPacket.Push( bSayYes );	
	NETWORK_SEND( kPacket )
}

int const lwCouple::GetMyCouplePlayTime()
{
	if( IamHaveCouple() )
	{
		SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
		return static_cast<int>(rkMyCoupleInfo.SweetHeartQuestTime());
	}

	return 0;
}

void lwCouple::CheckSweetHeartQuestTime()
{
	g_kCoupleMgr.CheckSweetHeartQuestTime();
}

void lwCouple::CoupleSweetHeartUI()
{
	g_kCoupleMgr.CoupleSweetHeartUI();
}

void lwCouple::SweetHeartQuestCompleteBtn(int const iQuestID)
{
	if( !IamHaveCouple() )
	{
		//g_kChatMgrClient.LogMsgBox(450035);
		return;
	}

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Req_SweetHeart_Complete);
	kPacket.Push( pkPlayer->CoupleGuid() );
	kPacket.Push( iQuestID );
	NETWORK_SEND( kPacket )
}

void lwCouple::Send_AnsSweetHeartComplete(lwGUID kGuid, bool bSayYes, int iQuestID)
{
	BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND, (BYTE)CC_Ans_SweetHeart_Complete);
	kPacket.Push( kGuid() );
	kPacket.Push( bSayYes );
	kPacket.Push( iQuestID );
	NETWORK_SEND( kPacket )
}

void lwCouple::OnSelectSendMarry(int const iGoldMoney)
{
	/*if( !IamHaveCouple() )
	{
		//g_kChatMgrClient.LogMsgBox(450035);
		return;
	}*/

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	// 내돈 Gold가 소지하고 있는지 없는지 체크
	if( 0 < iGoldMoney )
	{
		BM::Stream kPacket(PT_C_M_REQ_MARRY_COMMAND, (BYTE)MC_ReqMarry);
		kPacket.Push( iGoldMoney );
		NETWORK_SEND( kPacket )
	}
}

void lwCouple::OnSelectSendMoney(int const iGoldMoney)
{
	// 내돈 Gold가 소지하고 있는지 없는지 체크
	if( 0 < iGoldMoney )
	{
		BM::Stream kPacket(PT_C_M_REQ_MARRY_COMMAND, (BYTE)MC_ReqSendMoney);
		kPacket.Push( iGoldMoney );
		NETWORK_SEND( kPacket )
	}
}

void lwCouple::SetMarryState(int const iType)
{
	BM::Stream kPacket(PT_C_M_REQ_MARRY_COMMAND, (BYTE)MC_SetActionState);
	kPacket.Push( iType );
	NETWORK_SEND( kPacket )
}

void lwCouple::SetMarryTotalMoney()
{	
	BM::Stream kPacket(PT_C_M_REQ_MARRY_COMMAND, (BYTE)MC_TotalMoney);
	NETWORK_SEND( kPacket )
}

void lwCouple::SweetHeartNPC_TalkCall(int const iTextNo, char const *kNpcGuidString)
{
	if(!g_pkWorld)
	{
		return;
	}
	SChatLog kChatLog(CT_EVENT);
	std::wstring kLog = TTW(iTextNo);
	kChatLog.kCharName = TTW(450138);
	g_kChatMgrClient.AddLogMessage(kChatLog, kLog);

	BM::GUID kNpcGuidTalk;
	std::wstring kNpcString = UNI(kNpcGuidString);
	kNpcGuidTalk.Set( kNpcString );
	PgActorNpc* pkNpcActor = dynamic_cast<PgActorNpc*>( g_pkWorld->FindObject(kNpcGuidTalk) );
	if( pkNpcActor )
	{
		pkNpcActor->ShowChatBalloon(CT_EVENT, kLog);
	}
}

void lwCouple::SweetHeartVillageNPC_TalkCall(char const *kCharName, char const *kCoupleName, char const *kNpcGuidString)
{
	if(!g_pkWorld)
	{
		return;
	}
	std::wstring const rkCharName = UNI(kCharName);
	std::wstring const rkCoupleName = UNI(kCoupleName);

	if( !rkCharName.empty() && !rkCoupleName.empty() )
	{
		std::wstring kLogMessage;
		if( FormatTTW(kLogMessage, 450140, rkCharName.c_str(), rkCoupleName.c_str()) )
		{
			BM::GUID kNpcGuidTalk;
			std::wstring kNpcString = UNI(kNpcGuidString);
			kNpcGuidTalk.Set( kNpcString );
			PgActorNpc* pkNpcActor = dynamic_cast<PgActorNpc*>( g_pkWorld->FindObject(kNpcGuidTalk) );
			if( pkNpcActor )
			{
				pkNpcActor->ShowChatBalloon(CT_EVENT, kLogMessage);
			}			
		}
	}

}