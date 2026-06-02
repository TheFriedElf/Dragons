#include "StdAfx.h"
#include "ServerLib.h"
#include "Lohengrin/PacketStruct.h"
#include "variant/PgMyHome.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "PgHomeTown.h"
#include "lwHomeUI_Script.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgCommandMgr.h"
#include "PgUIScene.h"
#include "PgHome.h"
#include "PgFriendMgr.h"
#include "PgChatMgrClient.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "lwUIItemRepair.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIItemSocketSystem.h"
#include "PgCoupleMgr.h"
#include "PgGuild.h"
#include "PgHomeRenew.h"

int const HOTOWN_MAX_HOUSE_COUNT = 30;
int const HOTOWN_LINE_HOUSE_COUNT = 1;
__int64 const i64OneWeekSec = CGameTime::OneWeek/CGameTime::SECOND;

namespace lwHomeUIUtil
{
	SYSTEMTIME const GetTimeGab(BM::PgPackedTime const& kTime)
	{
		__int64 kNowTime = g_kEventView.GetLocalSecTime();
		__int64 kTempTime = 0;

		CGameTime::SystemTime2SecTime(kTime, kTempTime);

		kTempTime = kTempTime - kNowTime;

		SYSTEMTIME RstTime;
		ZeroMemory(&RstTime, sizeof(RstTime));
		if( kTempTime > 0 )
		{
			RstTime.wSecond = kTempTime % 60;
			RstTime.wMinute = kTempTime / 60 % 60;
			RstTime.wHour = kTempTime / 60 / 60 % 24;
			RstTime.wDay = kTempTime / 60 / 60 / 24;
		}
		return RstTime;
	}

	void CheckConvTimeStr(int const iOption, E_TIME_CONV_OPTION const eType, int const iValue, int const iTTID, std::wstring& kOutText)
	{
		wchar_t szTemp[ MAX_PATH ] = {0,};

		if((iOption & eType) == eType
		&& (iValue > 0 || !kOutText.empty()))
		{
			::swprintf_s(szTemp, MAX_PATH, TTW(iTTID).c_str(), iValue);
			kOutText += szTemp;
			kOutText += L" ";
		}		
	}

	std::wstring const GetTimeGabString(BM::PgPackedTime const& kTime, int const iOption, bool const bNoGab)
	{
		SYSTEMTIME kCunvTime;
		if( !bNoGab )
		{
			kCunvTime = GetTimeGab(kTime);
		}
		else
		{
			kCunvTime = kTime;
		}

		wchar_t szTemp[ MAX_PATH ] = {0,};
		std::wstring kTimeStr;

		CheckConvTimeStr(iOption, ETCT_YEAR, kCunvTime.wYear, 311, kTimeStr);
		CheckConvTimeStr(iOption, ETCT_MONTH, kCunvTime.wMonth, 312, kTimeStr);
		CheckConvTimeStr(iOption, ETCT_DAY, kCunvTime.wDay, 313, kTimeStr);
		CheckConvTimeStr(iOption, ETCT_HOUR, kCunvTime.wHour, 314, kTimeStr);
		CheckConvTimeStr(iOption, ETCT_MIN, kCunvTime.wMinute, 315, kTimeStr);
		CheckConvTimeStr(iOption, ETCT_SEC, kCunvTime.wSecond, 316, kTimeStr);

		return kTimeStr;
	}

	int const GetStateTTID(SMYHOME const& kHome)
	{
		if( g_kPilotMan.IsMyPlayer(kHome.kOwnerGuid) )
		{
			return 201012;
		}

		switch( PgHomeUIUtil::CheckToHomeVisitPossible(kHome) )
		{
		case PgHomeUIUtil::EHCR_CLOSE:
			{
				return 201013;
			}break;
		}
		return 201012;
	}

	void CommonPageControl(XUI::CXUI_Wnd* pPageMainUI, PgPage const& kPage)
	{
		if( !pPageMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pPageMainUI->GetControl(L"BLD_PAGENUM"));
		if( !pBuild )
		{
			return;
		}

		int const NowPage = kPage.Now();
		int const StartPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage();
		int const MaxPage = kPage.Max();
		
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"CBTN_NUM");
			vStr += i;

			XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pPageMainUI->GetControl(vStr));
			if( pCheckBtn )
			{
				pCheckBtn->ClickLock((StartPage + i) == NowPage);
				pCheckBtn->Check(pCheckBtn->ClickLock());	//이놈은 현제 페이지
				pCheckBtn->Visible((StartPage + i) < MaxPage);	//이놈은 넘었다.
				vStr = (StartPage + i) + 1;						//0부터 시작이니까.
				pCheckBtn->Text(vStr);
			}
		}
	}

	std::wstring const GetTownName(short const iTownNo)
	{
		BM::vstring strText;
		switch( iTownNo )
		{
		case 1:
			{
				strText = TTW(201695);
			}break;
		case 2:
			{
				strText = TTW(201713);
			}break;
		case 3:
			{
				strText = TTW(201714);
			}break;
		case 4:
			{
				strText = TTW(201696);
			}break;
		case 5:
			{
				strText = TTW(201715);
			}break;
		case 6:
			{
				strText = TTW(201697);
			}break;
		}

		return strText;
		//CONT_TOWN2GROUND const* pkContTown2Ground = NULL;
		//g_kTblDataMgr.GetContDef(pkContTown2Ground);
		//if( pkContTown2Ground )
		//{
		//	CONT_TOWN2GROUND::const_iterator town_itor = pkContTown2Ground->find(iTownNo);
		//	if( town_itor != pkContTown2Ground->end() )
		//	{
		//		CONT_TOWN2GROUND::mapped_type const& kGroundNo = town_itor->second;

		//		std::wstring const* kMapName = NULL;
		//		if( GetDefString(kGroundNo, kMapName) )
		//		{
		//			return *kMapName;
		//		}
		//	}
		//}
		//return std::wstring();
	}
};


namespace lwHomeUI
{
	void lwTest_AddTownBoardItem(short const streetno, int const houseno, bool bNotCallBoard);

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		//HomeTownBoard
		def(pkState, "OnSendTown_HomeList", lwHomeUI::OnSendTown_HomeList);
		def(pkState, "OnCallHomeBiddingRequest", lwHomeUI::OnCallHomeBiddingRequest);
		def(pkState, "OnCallHomeDetailView", lwHomeUI::OnCallHomeDetailView);
		def(pkState, "OnClickHomeBiddingOK", lwHomeUI::OnClickHomeBiddingOK);
		def(pkState, "OnClickTownBoardTownNumber", lwHomeUI::OnClickTownBoardTownNumber);
		def(pkState, "OnClickTownBoardTownNumberItem", lwHomeUI::OnClickTownBoardTownNumberItem);
		def(pkState, "OnClickTownBoardListViewType", lwHomeUI::OnClickTownBoardListViewType);
		def(pkState, "OnClickTownBoardListViewTypeItem", lwHomeUI::OnClickTownBoardListViewTypeItem);

		def(pkState, "OnClickTownBoardBeginPage", lwHomeUI::OnClickTownBoardBeginPage);
		def(pkState, "OnClickTownBoardEndPage", lwHomeUI::OnClickTownBoardEndPage);
		def(pkState, "OnClickTownBoardJumpPrevPage", lwHomeUI::OnClickTownBoardJumpPrevPage);
		def(pkState, "OnClickTownBoardPrevPage", lwHomeUI::OnClickTownBoardPrevPage);
		def(pkState, "OnClickTownBoardNextPage", lwHomeUI::OnClickTownBoardNextPage);
		def(pkState, "OnClickTownBoardJumpNextPage", lwHomeUI::OnClickTownBoardJumpNextPage);
		def(pkState, "OnClickTownBoardPage", lwHomeUI::OnClickTownBoardPage);

		//MyHome Menu
		def(pkState, "OnClickMyHomeVisitHistory", lwHomeUI::OnClickMyHomeVisitHistory);
		def(pkState, "OnClickMyHomeInvitation", lwHomeUI::OnClickMyHomeInvitation);
		def(pkState, "OnCallOrtherHomeInvitationUI", lwHomeUI::OnCallOrtherHomeInvitationUI);
		def(pkState, "OnClickOrtherHomeInvitation", lwHomeUI::OnClickOrtherHomeInvitation);
		def(pkState, "OnClickOrtherHomeInvitationToGO", lwHomeUI::OnClickOrtherHomeInvitationToGO);
		def(pkState, "OnCallHomeInOwnerAndUserUI", lwHomeUI::OnCallHomeInOwnerAndUserUI);
		def(pkState, "OnUpdateHomeVisitInfo", lwHomeUI::OnUpdateHomeVisitInfo);
		def(pkState, "OnClickVisitListItem", lwHomeUI::OnClickVisitListItem);
		def(pkState, "OnClickVisitToFriendReg", lwHomeUI::OnClickVisitToFriendReg);
		def(pkState, "OnClickVisitListItemDelete", lwHomeUI::OnClickVisitListItemDelete);

		def(pkState, "OnClickInviteBeginPage", lwHomeUI::OnClickInviteBeginPage);
		def(pkState, "OnClickInviteEndPage", lwHomeUI::OnClickInviteEndPage);
		def(pkState, "OnClickInviteJumpPrevPage", lwHomeUI::OnClickInviteJumpPrevPage);
		def(pkState, "OnClickInviteJumpNextPage", lwHomeUI::OnClickInviteJumpNextPage);
		def(pkState, "OnClickInvitePage", lwHomeUI::OnClickInvitePage);

		def(pkState, "IsUseCashShop", lwHomeUI::IsUseCashShop);

		//Npc Menu
		def(pkState, "OnClickHomeAuctionRegist", lwHomeUI::OnClickHomeAuctionRegist);
		def(pkState, "OnCallOpenTownList", lwHomeUI::OnCallOpenTownList);
		def(pkState, "OnCallTownMinimap", lwHomeUI::OnCallTownMinimap);
		def(pkState, "OnClickHomeRegistOK", lwHomeUI::OnClickHomeRegistOK);
		def(pkState, "OnClickHomeRegistDayDrop", lwHomeUI::OnClickHomeRegistDayDrop);
		def(pkState, "OnClickHomeRegistDayDropItem", lwHomeUI::OnClickHomeRegistDayDropItem);
		def(pkState, "OnClickMyHomeInfoRequest", lwHomeUI::OnClickMyHomeInfoRequest);
		def(pkState, "OnClickHomeUnRegistAuction", lwHomeUI::OnClickHomeUnRegistAuction);
		def(pkState, "OnClickMoveToTown", lwHomeUI::OnClickMoveToTown);
		def(pkState, "OnClickHomePost", lwHomeUI::OnClickHomePost);
		def(pkState, "OnClickHomeSafeInv", lwHomeUI::OnClickHomeSafeInv);

		//MiniMap
		def(pkState, "OnClickMinimapTownNumPopup", lwHomeUI::OnClickMinimapTownNumPopup);
		def(pkState, "OnClickMinimapTownNumPopupItem", lwHomeUI::OnClickMinimapTownNumPopupItem);
		def(pkState, "OnClickMinimapHomeItem", lwHomeUI::OnClickMinimapHomeItem);
		def(pkState, "OnClickGoToMyHome", lwHomeUI::OnClickGoToMyHome);

		def(pkState, "ReserveMyHomeTaxPayEvent", lwHomeUI::ReserveMyHomeTaxPayEvent);
		def(pkState, "SendMyHomeTaxPay_Packet", lwHomeUI::SendMyHomeTaxPay_Packet);
		def(pkState, "CheckExistHaveHome", lwHomeUI::CheckExistHaveHome);
		def(pkState, "CheckMyHome", lwHomeUI::CheckMyHome);
		

		

		//홈 옵션(공개 여부)
		def(pkState, "ReserveMyHomeOptionEvent", lwHomeUI::ReserveMyHomeOptionEvent);
		def(pkState, "SendMyHomeOption_Packet", lwHomeUI::SendMyHomeOption_Packet);
		
		//데코레이션 부가 기능 사용(버프받기)
		def(pkState, "ReserveMyHomeUseDecoUIEvent", lwHomeUI::ReserveMyHomeUseDecoUIEvent);
		def(pkState, "SendMyHomeUseDeco_Packet", lwHomeUI::SendMyHomeUseDeco_Packet);
		
		// test -
		def(pkState, "OnCallUIHomePayTax", lwHomeUI::OnCallUIHomePayTax);
		def(pkState, "OnCallUIHomeAuctionInfo", lwHomeUI::OnCallUIHomeAuctionInfo);
		def(pkState, "OnCallUIHomeOption", lwHomeUI::OnCallUIHomeOption);
		def(pkState, "OnCallUIUseDecoUI", lwHomeUI::OnCallUIUseDecoUI);
		def(pkState, "IsHomeBidding", lwHomeUI::IsHomeBidding);
		def(pkState, "IsHomeBlock", lwHomeUI::IsHomeBlock);

		def(pkState, "SavePickedHouse", lwHomeUI::SavePickedHouse);

		// 마이홈 아르바이트
		def(pkState, "OnClickMyHomeSideJobOnMaid", lwHomeUI::OnClickMyHomeSideJobOnMaid);
		def(pkState, "OnClickMyHomeSideJobPeriodRadioButton", lwHomeUI::OnClickMyHomeSideJobPeriodRadioButton);
		def(pkState, "OnClickMyHomeSideJobOK", lwHomeUI::OnClickMyHomeSideJobOK);
		def(pkState, "OnClickMyHomeSideJobSettingCancel", lwHomeUI::OnClickMyHomeSideJobSettingCancel);
		def(pkState, "IsRepairService", lwHomeUI::IsRepairService);
		def(pkState, "IsEnchantService", lwHomeUI::IsEnchantService);
		def(pkState, "OnCheckSideJobList", lwHomeUI::OnCheckSideJobList);
		def(pkState, "OnClickRepairService", lwHomeUI::OnClickRepairService);
		def(pkState, "OnClickEnchantService", lwHomeUI::OnClickEnchantService);
		def(pkState, "OnCallMyHomeSideJobRemainText", lwHomeUI::OnCallMyHomeSideJobRemainText);
		def(pkState, "OnClickMyHomeSideJobNpcMove", lwHomeUI::OnClickMyHomeSideJobNpcMove);
		def(pkState, "OnClickMyHomeSideJobNpcRemove", lwHomeUI::OnClickMyHomeSideJobNpcRemove);
		def(pkState, "OnClickMyHomeSideJobSoulGold", lwHomeUI::OnClickMyHomeSideJobSoulGold);
		def(pkState, "OnClickMyHomeSideJobSoulCount", lwHomeUI::OnClickMyHomeSideJobSoulCount);
		def(pkState, "OnClickMyHomeSideJobSocketGold", lwHomeUI::OnClickMyHomeSideJobSocketGold);
		def(pkState, "OnClickMyHomeSideJobSocketRate", lwHomeUI::OnClickMyHomeSideJobSocketRate);
		def(pkState, "OnClickMyHomeSideJobEnchantRate", lwHomeUI::OnClickMyHomeSideJobEnchantRate);

		def(pkState, "OnSendSideJobBoardList", lwHomeUI::OnSendSideJobBoardList);
		def(pkState, "OnClickSideJobBoardTownNumber", lwHomeUI::OnClickSideJobBoardTownNumber);
		def(pkState, "OnClickSideJobBoardTownItem", lwHomeUI::OnClickSideJobBoardTownItem);
		def(pkState, "OnClickSideJobBoardListViewType", lwHomeUI::OnClickSideJobBoardListViewType);
		def(pkState, "OnClickSideJobBoardListViewTypeItem", lwHomeUI::OnClickSideJobBoardListViewTypeItem);
		def(pkState, "OnClickSideJobBoardBeginPage", lwHomeUI::OnClickSideJobBoardBeginPage);
		def(pkState, "OnClickSideJobBoardJumpPrevPage", lwHomeUI::OnClickSideJobBoardJumpPrevPage);
		def(pkState, "OnClickSideJobBoardJumpNextPage", lwHomeUI::OnClickSideJobBoardJumpNextPage);
		def(pkState, "OnClickSideJobBoardEndPage", lwHomeUI::OnClickSideJobBoardEndPage);
		def(pkState, "OnClickSideJobBoardPage", lwHomeUI::OnClickSideJobBoardPage);
		def(pkState, "OnClickSideJobBoardMoveHome", lwHomeUI::OnClickSideJobBoardMoveHome);

		//Test
		def(pkState, "Test_AddTownBoardItem", lwHomeUI::lwTest_AddTownBoardItem);
	}

	BM::GUID g_kPickedHouse;

	void SavePickedHouse(lwGUID kGuid)
	{
		g_kPickedHouse = kGuid();
	}

	//패킷
	bool RecvMyHome_Packet(WORD kPacketType, BM::Stream& rkPacket)
	{
		switch( kPacketType )
		{
		case PT_M_C_ANS_HOMETOWN_INFO://홈타운 보드
			{
				if( g_kHomeTownMgr.ReadTownBoardItem(rkPacket) )
				{
					BYTE kCallType = PgHomeUIUtil::ETLRT_OPEN_BOARD;
					rkPacket.Pop(kCallType);

					switch( kCallType )
					{
					case PgHomeUIUtil::ETLRT_OPEN_MYHOME_ENTER:
						{
							lwHomeRenew::OpenMyhomeEnterWnd();
						}break;
					case PgHomeUIUtil::ETLRT_OPEN_SIDE_JOB_BOARD:
						{//아르바이트 게시판
							OnCallSideJobBoard();
						}break;
					case PgHomeUIUtil::ETLRT_OPEN_BOARD:
						{//타운게시판 용도로 요청되었다
							OnCallHomeTownBoard();
						}break;
					case PgHomeUIUtil::ETLRT_OPEN_MINIMAP:
						{//미니맵 용도로 요청되었다.
							int iTownNo = 1;//디폴트다
							if( g_pkWorld
								&& g_pkWorld->IsHaveAttr(GATTR_FLAG_HOMETOWN) 
								)
							{
								int const iMapNo = g_pkWorld->MapNo();

								CONT_DEFMAP const * pContDefMap = NULL;
								g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
								CONT_DEFMAP::const_iterator map_it = pContDefMap->find(iMapNo);	//맵번호로 맵 데프에서 찾아서
								if (pContDefMap->end() != map_it)
								{
									CONT_DEFMAP::mapped_type const& kMapInfo = map_it->second;

									iTownNo = kMapInfo.sHometownNo;
								}
							}

							SHOMEADDR kAddr(iTownNo, 0);
							if(g_kPickedHouse.IsNotNull())
							{
								PgPilot* pkHousePilot = g_kPilotMan.FindPilot(g_kPickedHouse);
								if(pkHousePilot)
								{
									PgMyHome* pkMyHome = dynamic_cast<PgMyHome*>(pkHousePilot->GetUnit());
									if(pkMyHome)
									{
										kAddr = pkMyHome->HomeAddr();
									}
								}
							}

							OnCallHomeTownMinimap(kAddr.StreetNo(), kAddr.HouseNo());
						}break;
					case PgHomeUIUtil::ETLRT_OPEN_BUILDING_NAME_BOARD:
						{
							lwHomeRenew::SetBuildNameBoard();
						}break;
					}
				}
				else
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201502, true);
				}
			}break;
		case PT_M_C_ANS_MYHOME_AUCTION_REG://경매등록
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201401, true);
				}
			}break;
		case PT_M_C_ANS_MYHOME_BIDDING://입찰
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					short iStreetNo = 0;
					int iHouseNo = 0;
					__int64 iBiddingCost = 0;

					rkPacket.Pop(iStreetNo);
					rkPacket.Pop(iHouseNo);
					rkPacket.Pop(iBiddingCost);

					if( g_kHomeTownMgr.UpdateHomeInfo(iStreetNo, iHouseNo, iBiddingCost) )
					{
						SetHomeTownBoardList(XUIMgr.Get(L"FRM_HOME_TOWN_BOARD"));
					}

					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201047, true);
				}
			}break;
		case PT_M_C_ANS_MYHOME_AUCTION_UNREG://경매취소
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201402, true);
				}
			}break;
		case PT_M_C_ANS_MYHOME_POST_INVITATION_CARD://초청장 발급
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					std::wstring kName;
					rkPacket.Pop(kName);
					wchar_t szTemp[MAX_PATH] = {0,};
					swprintf_s(szTemp, MAX_PATH, TTW(201079).c_str(), kName.c_str());
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
				}
			}break;
		case PT_M_C_HOME_NOTI_INVITATION://초청장받음
			{
				std::wstring kPlayerName;
				rkPacket.Pop(kPlayerName);
				OnCallInvitatianNotice(kPlayerName);
			}break;
		case PT_M_C_ANS_MYHOME_VISITORS://방문자리스트
			{
				g_kHomeTownMgr.ReadHomeVisitorItem( rkPacket );
				OnCallMyHomeVisitHistory();
			}break;
		case PT_M_C_ANS_MYHOME_INVITATION_CARD:
			{
				g_kHomeTownMgr.ReadHomeInviteItem( rkPacket );
				OnCallOrtherHomeInvitationUI();
			}break;
		case PT_M_C_ANS_MYHOME_PAY_TEX://세금 지불
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{// 세금 납부 완료 메세지
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790601, true);
				}
			}break;
			case PT_M_C_ANS_HOME_VISITFLAG_MODIFY://공개 옵션 설정
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
				}
			}break;
		case PT_M_C_ANS_HOME_USE_ITEM_EFFECT://데코레이션 아이템 사용
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{// 다수개의 완료 메세지를 전달 받게됨
				}
			}break;
		case PT_M_C_HOME_NOTI_ATTACHMENT://세금 밀려 집 빼앗길때
			{
				short sStreetNo=0;
				int sHouseNo=0;
				__int64 i64Tax=0;
				rkPacket.Pop(sStreetNo);
				rkPacket.Pop(sHouseNo);
				rkPacket.Pop(i64Tax);
				OnCallUIDeprivedHome(sStreetNo, sHouseNo, i64Tax);
			}break;
		case PT_M_C_HOME_NOTI_TEX:
			{
				short sStreetNo=0;
				int sHouseNo=0;
				__int64 i64Tax=0;
				BM::PgPackedTime kPayTaxTime;
				rkPacket.Pop(sStreetNo);
				rkPacket.Pop(sHouseNo);
				rkPacket.Pop(i64Tax);
				rkPacket.Pop(kPayTaxTime);
				OnCallNotiDelayTax(sStreetNo, sHouseNo, i64Tax, kPayTaxTime);
			}break;
		case PT_M_C_HOME_NOTI_BIDDING:
			{// 낙찰 성공 알림
				short sStreetNo=0;
				int sHouseNo=0;
				rkPacket.Pop(sStreetNo);
				rkPacket.Pop(sHouseNo);
				{
					wchar_t szBuf[MAX_PATH]={0,};
					::swprintf_s(szBuf, MAX_PATH, TTW(405129).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo);
					SChatLog kChatLog(CT_NOTICE);
					g_kChatMgrClient.AddLogMessage(kChatLog, szBuf, true);
					PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
					if( pPilot )
					{
						std::wstring kMyName = pPilot->GetName();
						g_kHomeRenewMgr.SetBoardNameTag(sStreetNo, sHouseNo, kMyName);
					}
				}
			}break;
		case PT_M_C_ANS_MYHOME_INFO://내 홈정보
			{//홈정보 업데이트 되면
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( g_pkWorld
					&& CheckErrorMsg(kResult) 
					)
				{
					SMYHOME kHome;
					BYTE byEvtType;
					rkPacket.Pop(byEvtType);
					kHome.ReadFromPacket(rkPacket);
					// 패킷 보낼때 담아둔 이벤트를 수행한다.
					OnEventProcedure(static_cast<lwHomeUIUtil::E_HOME_EVT>(byEvtType), kHome);
				}
			}break;
		case PT_M_C_MYHOME_NOTI_MODIFY_OWNER:
			{
				SMYHOME kMyHome;
				kMyHome.ReadFromPacket(rkPacket);
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkPlayer )
				{
					if( g_kPilotMan.IsMyPlayer(kMyHome.kOwnerGuid) )
					{
						pkPlayer->HomeAddr(SHOMEADDR(kMyHome.siStreetNo, kMyHome.iHouseNo));
					}
					else
					{
						pkPlayer->HomeAddr(SHOMEADDR());
					}
				}
			}break;
		case PT_M_C_ANS_MYHOME_ENTER:
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
				}
			}break;
		case PT_M_C_HOME_NOTI_CHANGE_TOP_BIDDER:
			{// 최고 입찰자가 변경되었다
				short sStreetNo=0;
				int sHouseNo=0;
				rkPacket.Pop(sStreetNo);
				rkPacket.Pop(sHouseNo);
				{
					wchar_t szBuf[MAX_PATH]={0,};
					::swprintf_s(szBuf, MAX_PATH, TTW(201121).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo);
					SChatLog kChatLog(CT_NOTICE);
					g_kChatMgrClient.AddLogMessage(kChatLog, szBuf, true);
				}
			}break;
		case PT_M_C_HOME_NOTI_AUCTION_CANCELED:
			{//경매가 취소 되었다
				short sStreetNo=0;
				int sHouseNo=0;
				rkPacket.Pop(sStreetNo);
				rkPacket.Pop(sHouseNo);
				{
					wchar_t szBuf[MAX_PATH]={0,};
					::swprintf_s(szBuf, MAX_PATH, TTW(201122).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo);
					SChatLog kChatLog(CT_NOTICE);
					g_kChatMgrClient.AddLogMessage(kChatLog, szBuf, true);
				}
			}break;
		case PT_M_C_ANS_START_SIDE_JOB:
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					XUIMgr.Close(L"SFRM_HOME_SIDE_JOB");
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201611, true);
					SendMyHomeInfoRequest();
				}
			}break;
		case PT_M_C_ANS_CANCEL_SIDE_JOB:
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
					XUIMgr.Close(L"SFRM_HOME_SIDE_JOB");
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201610, true);
					SendMyHomeInfoRequest();
				}
			}break;
		case PT_M_C_ANS_ENTER_SIDE_JOB:
			{
				HRESULT kResult = EC_OK;
				rkPacket.Pop(kResult);
				if( CheckErrorMsg(kResult) )
				{
				}
			}break;
		default:
			{
				return false;
			}break;
		}

		return true;
	}

	//Error 체크
	bool CheckErrorMsg(HRESULT kResult, bool bShowMessageBox)
	{
		switch( kResult )
		{
		case EC_OK:
			{
				return true;
			}break;
		case E_MYHOME_IS_NOT_PAY_TEX_TIME:
			{// 세금 지급 기간이 아님
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405114, true);
			}break;
		case E_MYHOME_INVALID_COST_LIMIT:
			{// 경매 등록 실패. XX골드 이상으로 등록해야 합니다. (기본 값 30골드)
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201531, true);
			}break;
		case E_MYHOME_NOT_HAVE_INVATIONCARD:
			{// 초대장을 소지하고 있지 않아 이동할 수 없습니다.
				//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201726, true);
				Notice_Show( TTW(201726), EL_Warning );
			}break;
		case E_MYHOME_CANT_BUY_BIDDING:
			{// 경매 중인 마이 홈이 있으므로 경매가 끝난 이후에 마이 홈APT를 구입할 수 있습니다.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201727, true);
			}break;
		case E_MYHOME_NEED_PAY_TEX:
			{// 마이 홈 APT를 반납하기 위해서는 세금을 모두 납부하셔야 합니다. 현재 세금이 미납 중이므로 세금을 납부한 후 반납을 해주시기 바랍니다.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201735, true);
			}break;
		case E_MYHOME_ALREADY_CHAT_ROOM:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201783, true);
			}break;
		case E_MYHOME_NOT_FOUND_ROOM:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201784, true);
			}break;
		case E_MYHOME_THIS_ROOM_FULL:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201785, true);
			}break;
		case E_MYHOME_NOT_IN_CHAT_ROOM:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201786, true);
			}break;
		case E_MYHOME_ALL_CHAT_ROOM_OPENED:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201787, true);
			}break;
		case E_MYHOME_INVALID_GUEST_COUNT:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201788, true);
			}break;
		case E_MYHOME_NOT_IN_HOME:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201789, true);
			}break;
		case E_MYHOME_NOT_MATCH_PASSWORD:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201790, true);
			}break;
		case E_MYHOME_NOT_FOUND_TARGET:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201791, true);
			}break;
		case E_MYHOME_DISABLE_PRIVATEMSG:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201792, true);
			}break;
		case E_MYHOME_IS_KICKED_ROOM:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201811, true);
			}break;
		case E_FAIL:
			{// 현재는 돈이 부족할때 이메세지가 오며, 메세지는 무시한다 - 2009.09.10
				return false;
			}break;
		default:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201501 + (kResult - E_MYHOME_ALREADY_HAVE), true);
			}break;
		}
		return false;
	}

	//타운게시판 오픈
	void OnSendTown_HomeList()
	{
		BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_INFO);
		kPacket.Push((BYTE)PgHomeUIUtil::ETLRT_OPEN_BOARD);
		NETWORK_SEND(kPacket);
	}

	//입찰 신청
	void OnCallHomeBiddingRequest(lwUIWnd kRecver)
	{
		XUI::CXUI_Wnd* pRecver = kRecver.GetSelf();
		if( !pRecver )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pRecver->Parent();
		if( !pParent )
		{
			return;
		}

		SHOMEADDR kHomeAddr;
		pParent->GetCustomData(&kHomeAddr, sizeof(kHomeAddr));

		XUI::CXUI_Wnd* BiddingUI = XUIMgr.Call(L"SFRM_HOME_AUCTION_BIDDING");
		if( !BiddingUI )
		{
			return;
		}
		RegistUIAction(BiddingUI, L"CloseUI");
		BiddingUI->SetCustomData(&kHomeAddr, sizeof(kHomeAddr));

		SMYHOME kHomeInfo;
		if( !g_kHomeTownMgr.GetHomeInfo(kHomeAddr.StreetNo(), kHomeAddr.HouseNo(), kHomeInfo ) )
		{//없는 집인데?
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201054, true);
			BiddingUI->Close();
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);

		if( iPlayerLevel < kHomeInfo.iLvLimitMin
		&&  iPlayerLevel > kHomeInfo.iLvLimitMax )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201530, true);
			BiddingUI->Close();
			return;
		}

		wchar_t szTemp[MAX_PATH] = {0,};

		XUI::CXUI_Wnd* pTemp = BiddingUI->GetControl(L"SFRM_HOME_ADDRESS");
		if( pTemp )
		{
			swprintf_s(szTemp, MAX_PATH, TTW(201019).c_str(), lwHomeUIUtil::GetTownName(kHomeAddr.StreetNo()).c_str(), kHomeAddr.HouseNo());
			pTemp->Text(szTemp);

			pTemp = pTemp->GetControl(L"SFRM_NOW_BIDDING_COST");
			if( pTemp )
			{
				pTemp->Text(BM::vstring(kHomeInfo.i64LastBiddingCost / 10000).operator std::wstring const&());
			}
		}

		pTemp = BiddingUI->GetControl(L"FRM_MINIMUM_BIDDING_COST");
		if( pTemp )
		{
			__int64 const iLowLimitBiddingCost = (kHomeInfo.i64LastBiddingCost + (kHomeInfo.i64LastBiddingCost / 100 * NEXT_BIDDING_COST_RATE + UPTOGOLD)) / 10000;
			swprintf_s(szTemp, MAX_PATH, TTW(201022).c_str(), iLowLimitBiddingCost);
			pTemp->Text(szTemp);
		}

		XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(BiddingUI->GetControl(L"EDT_BIDDING_MONEY"));
		if( pEdit )
		{
			pEdit->EditText(L"");
		}
	}

	//집정보 보기
	void OnCallHomeDetailView(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		SHOMEADDR kAddr;
		pParent->GetCustomData(&kAddr, sizeof(kAddr));

		OnCallHomeTownMinimap(kAddr.StreetNo(), kAddr.HouseNo());
	}

	//입찰 시도
	void OnClickHomeBiddingOK(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		SHOMEADDR kHomeAddr;
		pParent->GetCustomData(&kHomeAddr, sizeof(kHomeAddr));

		SMYHOME kHomeInfo;
		if( !g_kHomeTownMgr.GetHomeInfo(kHomeAddr.StreetNo(), kHomeAddr.HouseNo(), kHomeInfo) )
		{//없는 집인데?
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201054, true);
			return;
		}

		XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(L"EDT_BIDDING_MONEY"));
		if( !pEdit )
		{
			return;
		}
		__int64 i64Value = ::_wtoi64(pEdit->EditText().c_str());
		__int64 iHaveMoney = pkPlayer->GetAbil64(AT_MONEY);

		if( (i64Value * 10000) > iHaveMoney )
		{//가진돈이 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201048, true);
			return;
		}

		__int64 iMinimumCost = (kHomeInfo.i64LastBiddingCost + (kHomeInfo.i64LastBiddingCost / 100 * NEXT_BIDDING_COST_RATE + UPTOGOLD)) / 10000;
		if( iMinimumCost > i64Value )
		{//최소 입찰금액보다 작다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201050, true);
			return;
		}
		pParent->Close();

		i64Value *= 10000;
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_BIDDING);
		kPacket.Push(kHomeAddr.StreetNo());
		kPacket.Push(kHomeAddr.HouseNo());
		kPacket.Push(i64Value);
		NETWORK_SEND(kPacket);
	}

	//타운 넘버 드롭 오픈
	void OnClickTownBoardTownNumber(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_TOWN_NUMBER_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;

		XUI::SListItem* pAllViewItem = pList->AddItem(L"");
		if( pAllViewItem && pAllViewItem->m_pWnd )
		{
			short TownNo = 0;
			pAllViewItem->m_pWnd->SetCustomData(&TownNo, sizeof(TownNo));
			pAllViewItem->m_pWnd->Text(TTW(201002));
			ItemHeight = pAllViewItem->m_pWnd->Size().y;
		}

		PgHomeTown::CONT_TOWNNO const& kTownNoList = g_kHomeTownMgr.TownNoList();
		PgHomeTown::CONT_TOWNNO::const_iterator townNo_itor = kTownNoList.begin();
		while( townNo_itor != kTownNoList.end() )
		{
			PgHomeTown::CONT_TOWNNO::value_type const& kTownNo = (*townNo_itor);

			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				pListItem->m_pWnd->SetCustomData(&kTownNo, sizeof(kTownNo));
				//swprintf_s(szTemp, MAX_PATH, TTW(201003).c_str(), kTownNo);
				pListItem->m_pWnd->Text(lwHomeUIUtil::GetTownName(kTownNo));
			}
			++townNo_itor;
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	//타운 넘버 선택
	void OnClickTownBoardTownNumberItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		short iTownNo = 0;
		pSelf->GetCustomData(&iTownNo, sizeof(iTownNo));

		XUI::CXUI_Wnd* pHomeTownBoardUI = XUIMgr.Get(L"FRM_HOME_TOWN_BOARD");
		if( pHomeTownBoardUI )
		{
			XUI::CXUI_Wnd* TargetUI = pHomeTownBoardUI->GetControl(L"SFRM_TOWN_ADDR");
			if( TargetUI )
			{
				TargetUI->Text(pSelf->Text());
			}
		}

		if( g_kHomeTownMgr.SetTownBoardDisplayTownNo(iTownNo) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_HOME_TOWN_BOARD");
			if( pMainUI )
			{
				SetHomeTownBoardList(pMainUI);
				SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
			}
		}
	}

	//타운 집 일부 보기 드롭 오픈
	void OnClickTownBoardListViewType(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_TOWN_HOME_STATE_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;
		for( int i = 0; i < PgHomeUIUtil::ETBSS_END; ++i )
		{
			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				int TTID = 0;
				switch( i )
				{
				case 0:{ TTID = 201032;	}break;
				case 1:{ TTID = 201012;	}break;
				case 2:{ TTID = 201013;	}break;
				case 3:{ TTID = 201014;	}break;
				case 4:{ TTID = 201015;	}break;
				}
				pListItem->m_pWnd->SetCustomData(&i, sizeof(i));
				pListItem->m_pWnd->Text(TTW(TTID));
				ItemHeight = pListItem->m_pWnd->Size().y;
			}
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	// 타운 집 일부 보기 선택
	void OnClickTownBoardListViewTypeItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		int iViewState = 0;
		pSelf->GetCustomData(&iViewState, sizeof(iViewState));
		
		XUI::CXUI_Wnd* pHomeTownBoardUI = XUIMgr.Get(L"FRM_HOME_TOWN_BOARD");
		if( pHomeTownBoardUI )
		{
			XUI::CXUI_Wnd* TargetUI = pHomeTownBoardUI->GetControl(L"SFRM_ALIGN_CATE");
			if( TargetUI )
			{
				TargetUI->Text(pSelf->Text());
			}
		}

		if( g_kHomeTownMgr.SetTownBoardDisplayHomeStateType(iViewState) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_HOME_TOWN_BOARD");
			if( pMainUI )
			{
				SetHomeTownBoardList(pMainUI);
				SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
			}
		}
	}

	void OnClickTownBoardBeginPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageBegin() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardEndPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageEnd() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardJumpPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrevJump() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrev() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNext() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardJumpNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNextJump() )
		{
			return;
		}

		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickTownBoardPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( kSelf.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pSelf->BuildIndex();

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}

		PgPage& kPage = g_kHomeTownMgr.GetBoardPage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetHomeTownBoardList(pMainUI);
		SetHomeTownBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	// 실제 타운 게시판 오픈
	void OnCallHomeTownBoard()
	{
		XUI::CXUI_Wnd* pBoardUI = XUIMgr.Get(L"FRM_HOME_TOWN_BOARD");
		if( !pBoardUI || pBoardUI->IsClosed() )
		{
			pBoardUI = XUIMgr.Call(L"FRM_HOME_TOWN_BOARD");
			if( !pBoardUI )
			{
				return;
			}
		}

		RegistUIAction(pBoardUI, L"CloseUI");

		XUI::CXUI_Wnd* pWnd = pBoardUI->GetControl(L"SFRM_TOWN_ADDR");
		if( pWnd )
		{
			short wTownNo = 0;
			pWnd->Text(TTW(201002));
			pWnd->SetCustomData(&wTownNo, sizeof(wTownNo));
		}

		pWnd = pBoardUI->GetControl(L"SFRM_ALIGN_CATE");
		if( pWnd )
		{
			int iSortType = PgHomeUIUtil::ETBSS_ALL_ITEM;
			pWnd->Text(TTW(201032));
			pWnd->SetCustomData(&iSortType, sizeof(iSortType));
		}

		SetHomeTownBoardList(pBoardUI);
		SetHomeTownBoardPageControl(pBoardUI->GetControl(L"FRM_PAGE"));
	}

	void SetHomeTownBoardList(XUI::CXUI_Wnd* pMainUI)
	{
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM_SLOT"));
		if( !pBuild )
		{
			return;
		}

		CONT_MYHOME kHomeList;
		g_kHomeTownMgr.GetTownBoardDisplayItem(kHomeList);

		CONT_MYHOME::const_iterator home_iter = kHomeList.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pHomeSlot = pMainUI->GetControl(vStr);
			if( pHomeSlot )
			{
				if( home_iter != kHomeList.end() )
				{
					CONT_MYHOME::mapped_type const& kHomeInfo = home_iter->second;
					SetHomeTownBoardHomeSlot(pHomeSlot, kHomeInfo);
					++home_iter;
				}
				else
				{
					SetHomeTownBoardHomeSlot(pHomeSlot, SMYHOME());
				}
			}
		}
	}

	void SetHomeTownBoardPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeTownMgr.GetBoardPage());
	}

	//보드 집 정보 슬롯
	void SetHomeTownBoardHomeSlot(XUI::CXUI_Wnd* pSlot, SMYHOME const& kHome)
	{
		if( !pSlot )
		{
			return;
		}

		if( 0 == kHome.siStreetNo && 0 == kHome.iHouseNo )
		{
			pSlot->Visible(false);
		}
		else
		{
			pSlot->Visible(true);
		}

		pSlot->OwnerGuid(kHome.kOwnerGuid);

		SHOMEADDR kPoint(kHome.siStreetNo, kHome.iHouseNo);
		pSlot->SetCustomData(&kPoint, sizeof(kPoint));

		wchar_t szTemp[MAX_PATH] = {0,};

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_LEVEL");
		if( pTemp )
		{
			//std::wstring wStrLevel;
			//FormatTTW(wStrLevel, 8007, kHome.iLvLimitMin, kHome.iLvLimitMax);
			//pTemp->Text(wStrLevel);
			BM::vstring strText(kHome.iLvLimitMin);
			strText += TTW(224);
			pTemp->Text(strText);
		}

		pTemp = pSlot->GetControl(L"SFRM_LOC");
		if( pTemp )
		{
			std::wstring kLocStr;
			//swprintf_s(szTemp, TTW(201003).c_str(), kHome.siStreetNo);
			kLocStr = lwHomeUIUtil::GetTownName(kHome.siStreetNo);
			kLocStr += L" ";
			swprintf_s(szTemp, TTW(201004).c_str(), kHome.iHouseNo);
			kLocStr += szTemp;
			pTemp->Text(kLocStr);
		}

		pTemp = pSlot->GetControl(L"SFRM_OWNER");
		if( pTemp )
		{
			pTemp->Text(kHome.kName);
		}


		bool bIsMyHome = g_kPilotMan.IsMyPlayer( kHome.kOwnerGuid );
		bool bIsBidding = kHome.bAuctionState == MAS_IS_BIDDING;

		pTemp = pSlot->GetControl(L"SFRM_STATE");
		if( pTemp )
		{
			pTemp->Visible(bIsMyHome || !bIsBidding);
			int iTTID = lwHomeUIUtil::GetStateTTID(kHome);
			if( bIsMyHome && bIsBidding )
			{
				iTTID = 201014;
			}
			pTemp->Text(TTW(iTTID));
		}

		pTemp = pSlot->GetControl(L"BTN_BIDDING");
		if( pTemp )
		{
			pTemp->Visible(!bIsMyHome && bIsBidding);
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(!pTemp->Visible());
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_COST");
		if( pTemp )
		{
			__int64 iMoney = kHome.i64LastBiddingCost / 10000;
			pTemp->Text((!bIsBidding)?(TTW(201016)):(BM::vstring(iMoney).operator std::wstring const&()));
		}

		pTemp = pSlot->GetControl(L"SFRM_CLOSE");
		if( pTemp )
		{
			pTemp->Text((!bIsBidding)?(TTW(201016)):(lwHomeUIUtil::GetTimeGabString(kHome.kAuctionTime)));
		}
	}

	void OnClickMyHomeVisitHistory(lwUIWnd kSelf)
	{
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			if( !pkHome->IsMyHome() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201112, true);
				return;
			}
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201502, true);
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_MYHOME_VISITORS);
		NETWORK_SEND(kPacket);
	}

	void OnClickMyHomeInvitation(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pMainUI = kParent.GetSelf();
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pMainUI->GetControl(L"EDT_INVITE_USER_NAME"));
		if( !pEdit )
		{
			return;
		}

		std::wstring kText = pEdit->EditText();
		if( kText.empty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 404951, true);
			return;
		}

		if( kText.size() > MAX_CHARACTER_NAME_LEN )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 404024, true);
			return;
		}

		if(g_kClientFS.Filter(kText, false, FST_ALL)
			|| !g_kUnicodeFilter.IsCorrect(UFFC_CHARACTER_NAME, kText)
			)
		{//욕설 등이 있으면
			pEdit->EditText(L"");
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50513, true);
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_MYHOME_POST_INVITATION_CARD);
		kPacket.Push(kText);
		NETWORK_SEND(kPacket);

		pMainUI->Close();
	}

	void OnClickOrtherHomeInvitation()
	{
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_INVITATION_CARD);
		NETWORK_SEND(kPacket);
	}

	void OnClickOrtherHomeInvitationToGO(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		SHOMEADDR kAddr;
		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( pParent )
		{
			pParent = pParent->Parent();
			if( pParent )
			{
				pParent->GetCustomData(&kAddr, sizeof(kAddr));
			}
		}

		BM::Stream kPacket(PT_C_M_REQ_MYHOME_ENTER);
		kPacket.Push(kAddr.StreetNo());
		kPacket.Push(kAddr.HouseNo());

		std::wstring kMessage = TTW(201109);
		if( g_kExpedition.ExpeditionGuid().IsNotNull() )
		{
			kMessage = TTW(720048) + std::wstring(L"\n") + kMessage;
		}
		else if( g_kParty.PartyGuid().IsNotNull() )
		{
			kMessage = TTW(201119) + std::wstring(L"\n") + kMessage;
		}
		CallCommonMsgYesNoBox(kMessage, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	}

	void OnCallHomeInOwnerAndUserUI()
	{
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"FRM_MYHOME_OWNER");
		if( !pMainUI )
		{
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		XUI::CXUI_Wnd* pOwnerName = pMainUI->GetControl(L"SFRM_NAME");
		if( pOwnerName )
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(201078).c_str(), pkHomeUnit->OwnerName().c_str());
			pOwnerName->Text(szTemp);
		}

		OnUpdateHomeVisitInfo(lwUIWnd(pMainUI->GetControl(L"SFRM_VISIT_INFO")));

		XUIMgr.Call(L"SFRM_MYHOME_MENU");
		XUIMgr.Activate(L"Bg_XP_Bar");
		XUIMgr.Close(L"FormHero");
		XUIMgr.Close(L"QuickInv");
		XUIMgr.Close(L"SFRM_SYSTEMMENU");
		XUIMgr.Close(L"QuickMenu");
		XUIMgr.Close(L"QuickInvEx");
		XUIMgr.Close(L"FRM_PARTY_STATE");
		g_kChatMgrClient.ChatStation(ECS_MYHOME);
		int const iLoadPreChatTab = -1;
		lwCallChatWindow(iLoadPreChatTab);
		//lwHomeRenew::SetChatHeaderName();
		//내집으로 돌아가기 버튼
		XUI::CXUI_Wnd* pGoToMyHomeWnd = pMainUI->GetControl(L"BTN_GO_MYHOME");
		if(!pGoToMyHomeWnd)
		{
			return;
		}
		XUI::CXUI_Button* pGoToMyHomeBtn = dynamic_cast<XUI::CXUI_Button*>(pGoToMyHomeWnd);
		if(!pGoToMyHomeBtn)
		{
			return;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		//내집이 없거나, 이미 내집안에 있으면 비활성화
		bool const bNotHaveMyHome = (pkPlayer->HomeAddr().IsNull() || pkHome->IsMyHome() );
		pGoToMyHomeBtn->Disable(bNotHaveMyHome);
	}

	void OnUpdateHomeVisitInfo(lwUIWnd UISelf)
	{
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		XUI::CXUI_Wnd* pToday = pSelf->GetControl(L"FRM_TODAY_NUM");
		if( pToday )
		{
			BM::vstring vStr(pkHomeUnit->GetAbil(AT_MYHOME_TODAYHIT));
			pToday->Text(vStr);
		}

		XUI::CXUI_Wnd* pTotal = pSelf->GetControl(L"FRM_TOTAL_NUM");
		if( pTotal )
		{
			BM::vstring vStr(pkHomeUnit->GetAbil(AT_MYHOME_TOTALHIT));
			pTotal->Text(vStr);
		}
	}

	void OnClickVisitListItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_ITEM"));
		if( !pBuild )
		{
			return;
		}

		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;
			XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
			if( pSlot )
			{
				SetMyHomeVisitHistorySlotClickInfo(pSlot, false);
			}
		}
		SetMyHomeVisitHistorySlotClickInfo(pSelf, true);
	}

	void OnClickVisitToFriendReg(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_ITEM"));
		if( !pBuild )
		{
			return;
		}

		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;
			XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
			if( pSlot && pSlot->Visible() )
			{
				XUI::CXUI_Wnd* pSelect = pSlot->GetControl(L"FRM_ITEM_SELECT");
				if( pSelect && pSelect->Visible() )
				{
					SFriendItem kItem;
					kItem.CharGuid( pSlot->OwnerGuid() );
					g_kFriendMgr.SendFriend_Command(FCT_ADD_BYGUID, kItem);
					return;
				}
			}
		}
		//여기 오면에러420
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 420, true);

	}

	void OnClickVisitListItemDelete(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_ITEM"));
		if( !pBuild )
		{
			return;
		}

		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;
			XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
			if( pSlot && pSlot->Visible() )
			{
				XUI::CXUI_Wnd* pSelect = pSlot->GetControl(L"FRM_ITEM_SELECT");
				if( pSelect && pSelect->Visible() )
				{
					g_kHomeTownMgr.DeleteHomeVisitorItem( pSelect->OwnerGuid() );
					OnCallMyHomeVisitHistory();
					//서버에 보내줘야 하는데?

					return;
				}
			}
		}
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 420, true);
	}

	void OnCallMyHomeVisitHistory()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_VISIT_HISTORY");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"SFRM_VISIT_HISTORY");
			if( !pMainUI )
			{
				return;
			}
		}

		SetMyHomeVisitHistory(pMainUI);
	}

	void SetMyHomeVisitHistory(XUI::CXUI_Wnd* pMainUI)
	{
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM"));
		if( !pBuild )
		{
			return;
		}

		//방문자 데이타를 받아와야 함
		CONT_VISITORINFO kVisitorList;
		g_kHomeTownMgr.GetHomeVisitorItem(kVisitorList);
		//! 방문 날짜를 기준으로 정렬해주자.
		kVisitorList.sort(lwHomeUI::SortVisitDay);

		CONT_VISITORINFO::iterator visit_itor = kVisitorList.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
			if( pSlot )
			{
				if( visit_itor != kVisitorList.end() )
				{
					CONT_VISITORINFO::value_type const& kVisitInfo = (*visit_itor);
					SetMyHomeVisitHistorySlotClickInfo(pSlot, false);
					SetMyHomeVisitHistorySlot(pSlot, kVisitInfo);
					++visit_itor;
				}
				else
				{
					SetMyHomeVisitHistorySlotClickInfo(pSlot, false);
					SetMyHomeVisitHistorySlot(pSlot, SVISITORINFO());
				}
			}
		}
	}

	bool SortVisitDay(SVISITORINFO const &rkLeft, SVISITORINFO const &rkRight)
	{
		if( rkLeft.kVistTime < rkRight.kVistTime )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	void SetMyHomeVisitHistorySlotClickInfo(XUI::CXUI_Wnd* pSlot, bool const bClick)
	{
		if( !pSlot )
		{
			return;
		}

		DWORD FontColor = (bClick)?(0xFFA0410D):(0xFF3D3413);

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_NAME");
		if( pTemp )
		{
			pTemp->FontColor(FontColor);
		}

		pTemp = pSlot->GetControl(L"SFRM_TIME");
		if( pTemp )
		{
			pTemp->FontColor(FontColor);
		}

		pTemp = pSlot->GetControl(L"FRM_ITEM_SELECT");
		if( pTemp )
		{
			pTemp->Visible(bClick);
		}
	}

	void SetMyHomeVisitHistorySlot(XUI::CXUI_Wnd* pSlot, SVISITORINFO const& kVisitInfo)
	{
		if( !pSlot )
		{
			return;
		}

		if( kVisitInfo.kOwnerGuid.IsNull() )
		{
			pSlot->Visible(false);
			return;
		}
		else
		{
			pSlot->Visible(true);
			pSlot->OwnerGuid(kVisitInfo.kOwnerGuid);
		}

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_NAME");
		if( pTemp )
		{
			pTemp->Text(kVisitInfo.kName);
		}

		pTemp = pSlot->GetControl(L"SFRM_TIME");
		if( pTemp )
		{
			int const eConvStrOption = lwHomeUIUtil::ETCT_DEF_TIME_COV | lwHomeUIUtil::ETCT_MONTH | lwHomeUIUtil::ETCT_YEAR;
			pTemp->Text(lwHomeUIUtil::GetTimeGabString(kVisitInfo.kVistTime, eConvStrOption, true));
		}

		pTemp = pSlot->GetControl(L"SFRM_HOME");
		if( pTemp )
		{
			bool bHaveHouse = !kVisitInfo.kAddr.IsNull();

			XUI::CXUI_Wnd* pHomeImg = pTemp->GetControl(L"BTN_VIEW_TO_GO");
			if( pHomeImg )
			{
				pHomeImg->Visible(bHaveHouse);
				if( !pHomeImg->Visible() )
				{
					pTemp->Text(TTW(420012));
				}
			}
		}
	}

	void OnClickInviteBeginPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetInvitePage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageBegin() )
		{
			return;
		}

		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickInviteEndPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetInvitePage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageEnd() )
		{
			return;
		}

		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickInviteJumpPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetInvitePage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrev() )
		{
			return;
		}

		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickInviteJumpNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetInvitePage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNext() )
		{
			return;
		}

		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickInvitePage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( kSelf.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pSelf->BuildIndex();

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}

		PgPage& kPage = g_kHomeTownMgr.GetInvitePage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	bool IsUseCashShop()
	{
		return g_kHomeTownMgr.UseCashShop();
	}

	void OnCallOrtherHomeInvitationUI()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_RECV_HOME_INVITE_LETTER");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"SFRM_RECV_HOME_INVITE_LETTER");
			if( !pMainUI )
			{
				return;
			}
		}

		SetOrtherHomeIvitationList(pMainUI);
		SetOrtherHomeIvitationPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void SetOrtherHomeIvitationList(XUI::CXUI_Wnd* pMainUI)
	{
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM"));
		if( !pBuild )
		{
			return;
		}

		//방문자 데이타를 받아와야 함
		CONT_INVITATIONCARD kInvitationList;
		g_kHomeTownMgr.GetHomeInviteItem(kInvitationList);

		CONT_INVITATIONCARD::iterator invite_itor = kInvitationList.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
			if( pSlot )
			{
				if( invite_itor != kInvitationList.end() )
				{
					CONT_INVITATIONCARD::value_type const& kInviteInfo = (*invite_itor);
					SetOrtherHomeIvitationListSlot(pSlot, kInviteInfo);
					++invite_itor;
				}
				else
				{
					SetOrtherHomeIvitationListSlot(pSlot, SINVITATIONCARD());
				}
			}
		}
	}

	void SetOrtherHomeIvitationListSlot(XUI::CXUI_Wnd* pSlot, SINVITATIONCARD const& kInviteInfo)
	{
		if( !pSlot )
		{
			return;
		}

		if( kInviteInfo.siStreetNo == 0 || kInviteInfo.iHouseNo == 0 )
		{
			pSlot->Visible(false);
			return;
		}
		else
		{
			pSlot->Visible(true);
		}

		SHOMEADDR	kAddr(kInviteInfo.siStreetNo, kInviteInfo.iHouseNo);
		pSlot->SetCustomData(&kAddr, sizeof(kAddr));

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_NAME");
		if( pTemp )
		{
			pTemp->Text(kInviteInfo.kHomeOwnerName);
		}

		pTemp = pSlot->GetControl(L"SFRM_ADDR");
		if( pTemp )
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			std::wstring kLocStr;
			//swprintf_s(szTemp, TTW(201003).c_str(), kInviteInfo.siStreetNo);
			kLocStr = lwHomeUIUtil::GetTownName(kInviteInfo.siStreetNo);
			kLocStr += L" ";
			swprintf_s(szTemp, TTW(201004).c_str(), kInviteInfo.iHouseNo);
			kLocStr += szTemp;
			if( g_kHomeRenewMgr.IsMyhomeApt(kInviteInfo.siStreetNo) )
			{
				pTemp->Text(TTW(201818));
			}
			else
			{
				pTemp->Text(kLocStr);
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_TIME");
		if( pTemp )
		{
			pTemp->Text(lwHomeUIUtil::GetTimeGabString(kInviteInfo.kLimitTime, lwHomeUIUtil::ETCT_DAY | lwHomeUIUtil::ETCT_HOUR));
		}
	}

	void SetOrtherHomeIvitationPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeTownMgr.GetInvitePage());
	}

	//경매 등록
	void OnClickHomeAuctionRegist()
	{
		if(false == CheckMyHome())
		{
			return;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( pkPlayer->HomeAddr().IsNull() )
		{//난 집없다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return;
		}

		OnSendHomeInfoRequest(pkPlayer->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_AUCTION_REG);
	}

	void OnCallOpenTownList()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_SELECT_HOMETOWN");
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pMainUI->GetControl(L"LST_TOWN"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		PgHomeUI::CONT_TOWNNO const& kCont = g_kHomeTownMgr.TownNoList();

		PgHomeUI::CONT_TOWNNO::const_iterator townno_itor = kCont.begin();
		while( townno_itor != kCont.end() )
		{
			PgHomeUI::CONT_TOWNNO::value_type const& kTownNo = (*townno_itor);
			XUI::SListItem* pItemWnd = pList->AddItem(L"");
			if( pItemWnd && pItemWnd->m_pWnd )
			{
				XUI::CXUI_Wnd* pText = pItemWnd->m_pWnd->GetControl(L"SFRM_TOWN_SELECT_ITEM");
				if( pText )
				{
					//wchar_t szTemp[MAX_PATH] = {0,};
					//swprintf_s(szTemp, TTW(201003).c_str(), kTownNo);
					pText->Text(lwHomeUIUtil::GetTownName(kTownNo));
				}				

				CXUI_Wnd* pkBtnWnd = pItemWnd->m_pWnd->GetControl(L"BTN_REG");			
				if( pkBtnWnd )
				{
					std::wstring wstrBtn = TTW(400084) + TTW( 2200 + (kTownNo-1 ) );
					pkBtnWnd->Text(wstrBtn);
				}

				pItemWnd->m_pWnd->SetCustomData(&kTownNo, sizeof(kTownNo));
			}

			++townno_itor;
		}
	}

	void OnCallTownMinimap()
	{//NPC를 이용한 호출은 무조건 고정 =_=;
		BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_INFO);
		kPacket.Push((BYTE)PgHomeUIUtil::ETLRT_OPEN_MINIMAP);
		NETWORK_SEND(kPacket);
		SavePickedHouse(lwGUID(""));
	}

	//경매 등록 시도
	void OnClickHomeRegistOK(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( pkPlayer->HomeAddr().IsNull() )
		{//난 집없다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return;
		}

		XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(L"EDT_START_MONEY"));
		if( !pEdit )
		{
			return;
		}
		__int64 i64Value = ::_wtoi64(pEdit->EditText().c_str());
		if(0 == i64Value)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790602, true);
			return;
		}

		XUI::CXUI_Wnd* pWnd = pParent->GetControl(L"FRM_SALE_DAY");
		if( !pWnd )
		{
			return;
		}
		int iRegTime = 0;
		pWnd->GetCustomData(&iRegTime, sizeof(iRegTime));
		pParent->Close();

		int iUITime = (iRegTime < MAT_1D)?(iRegTime):(iRegTime / MAT_1D);
		int TTID = (iRegTime < MAT_1D)?(201057):(201058);

		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, TTW(201056).c_str(), lwHomeUIUtil::GetTownName(pkPlayer->HomeAddr().StreetNo()).c_str(), pkPlayer->HomeAddr().HouseNo(), iUITime, TTW(TTID).c_str(), i64Value);
		i64Value *= 10000;
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_AUCTION_REG);
		kPacket.Push(i64Value);
		kPacket.Push(iRegTime);
		CallCommonMsgYesNoBox(szTemp, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	}

	//입찰 기간 드롭 오픈
	void OnClickHomeRegistDayDrop(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_TOWN_AUCTION_REG_TIME_TYPE");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int const MAX_TIME_TYPE = 5;
		int ItemHeight = 0;
		for( int i = 0; i < MAX_TIME_TYPE; ++i )
		{
			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				int iTime = 0;
				int TTID = 0;
				int TextTime = 0;
				switch( i )
				{
				case 0:{ iTime = MAT_6;		TTID = 201028;	TextTime = MAT_6;	}break;
				case 1:{ iTime = MAT_12;	TTID = 201028;	TextTime = MAT_12;	}break;
				case 2:{ iTime = MAT_18;	TTID = 201028;	TextTime = MAT_18;	}break;
				case 3:{ iTime = MAT_1D;	TTID = 201029;	TextTime = MAT_1D / MAT_1D;	}break;
				case 4:{ iTime = MAT_6D;	TTID = 201029;	TextTime = MAT_6D / MAT_1D;	}break;
				}
				pListItem->m_pWnd->SetCustomData(&iTime, sizeof(iTime));

				swprintf_s(szTemp, MAX_PATH, TTW(TTID).c_str(), TextTime);
				pListItem->m_pWnd->Text(szTemp);
				ItemHeight = pListItem->m_pWnd->Size().y;
			}
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	//입찰 기간 선택
	void OnClickHomeRegistDayDropItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		int iSelectTime = 0;
		pSelf->GetCustomData(&iSelectTime, sizeof(iSelectTime));

		XUI::CXUI_Wnd* pAuctionRegUI = XUIMgr.Get(L"SFRM_HOME_AUCTION_REGIST");
		if( pAuctionRegUI )
		{
			XUI::CXUI_Wnd* TargetUI = pAuctionRegUI->GetControl(L"FRM_SALE_DAY");
			if( TargetUI )
			{
				TargetUI->SetCustomData(&iSelectTime, sizeof(iSelectTime));
				TargetUI->Text(pSelf->Text());
			}
		}
	}

	void OnClickMyHomeInfoRequest()
	{
		if(false == CheckMyHome())
		{
			return;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer && !pkPlayer->HomeAddr().IsNull() )
		{
			OnSendHomeInfoRequest(pkPlayer->HomeAddr(),lwHomeUIUtil::EEVT_CALL_MYHOME_AUCTION_INFO);
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
		}
	}

	void OnCallHomeAuctionRegist()
	{
		XUI::CXUI_Wnd* AuctionRegUI = XUIMgr.Call(L"SFRM_HOME_AUCTION_REGIST");
		if( !AuctionRegUI )
		{
			return;
		}

		wchar_t szTemp[MAX_PATH] = {0,};
		XUI::CXUI_Wnd* pTemp = AuctionRegUI->GetControl(L"FRM_SALE_DAY");
		if( pTemp )
		{
			int iDefault = MAT_6;
			swprintf_s(szTemp, TTW(201028).c_str(), iDefault);
			pTemp->Text(szTemp);
			pTemp->SetCustomData(&iDefault, sizeof(iDefault));
		}

		XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(AuctionRegUI->GetControl(L"EDT_START_MONEY"));
		if( pEdit )
		{
			pEdit->EditText(L"");
		}
	}

	void OnSetMyHomeInfoUI()
	{
		SMYHOME kHome;
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_MYHOME_REG_INFO");
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"FRM_ADDRESS_VALUE");
		if( pTemp )
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			std::wstring kLocStr;
			//swprintf_s(szTemp, TTW(201003).c_str(), kHome.siStreetNo);
			kLocStr = lwHomeUIUtil::GetTownName(kHome.siStreetNo);
			kLocStr += L" ";
			swprintf_s(szTemp, TTW(201004).c_str(), kHome.iHouseNo);
			kLocStr += szTemp;
			pTemp->Text(kLocStr);
		}

		pTemp = pMainUI->GetControl(L"FRM_TIME_VALUE");
		if( pTemp )
		{
			pTemp->Text(lwHomeUIUtil::GetTimeGabString(kHome.kAuctionTime));
		}

		pTemp = pMainUI->GetControl(L"FRM_STARTCOST_VALUE");
		if( pTemp )
		{
			__int64 iMoney = kHome.i64FirstBiddingCost / 10000;
			pTemp->Text(BM::vstring(iMoney).operator std::wstring const&());
		}

		pTemp = pMainUI->GetControl(L"FRM_NOWCOST_VALUE");
		if( pTemp )
		{
			__int64 iMoney = kHome.i64LastBiddingCost / 10000;
			pTemp->Text(BM::vstring(iMoney).operator std::wstring const&());
		}
	}

	void OnSendHomeInfoRequest(SHOMEADDR const& kAddr,lwHomeUIUtil::E_HOME_EVT const& eEvent)
	{
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_INFO);
		kPacket.Push(kAddr.StreetNo());
		kPacket.Push(kAddr.HouseNo());
		// 홈 정보가 업데이트 될때 사용될 이벤트를 기록
		kPacket.Push(static_cast<BYTE>(eEvent));
		NETWORK_SEND(kPacket);
		//g_kHomeTownMgr.ReserveEvent(sEvent);
	}

	void OnClickHomeUnRegistAuction()
	{
		XUI::CXUI_Wnd* pkAuctionInfo = XUIMgr.Get(L"SFRM_MYHOME_REG_INFO");
		if(pkAuctionInfo)
		{
			XUI::CXUI_Wnd* pkStartCost = pkAuctionInfo->GetControl(L"FRM_STARTCOST_VALUE");
			if(pkStartCost)
			{
				__int64 const iNeedGold = ::_atoi64(MB(pkStartCost->Text())) *10000; //골드 단위로 맞춤
				g_kHomeTownMgr.MyHomeFirstBiddingCost(iNeedGold);
			}
		}
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_AUCTION_UNREG);
		CallCommonMsgYesNoBox(TTW(201113), 96, 97, lwPacket(&kPacket), true, MBT_ACCEPT_HOMEAUCTION_CANCEL, NULL);
	}

	void OnClickMoveToTown(lwUIWnd kSelf, const short siStreet)
	{
		XUI::CXUI_Wnd* pWnd = kSelf.GetSelf();
		if( !pWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pWnd->Parent();
		if( !pParent )
		{
			return;
		}

		short siStreetNo = 0;

		if( 0 != siStreet )
		{
			siStreetNo = siStreet;
		}
		else
		{			
			pParent->GetCustomData(&siStreetNo, sizeof(siStreetNo));
		}

		CONT_TOWN2GROUND const* kCont = NULL;
		g_kTblDataMgr.GetContDef(kCont);

		CONT_TOWN2GROUND::const_iterator s_iter = kCont->find(siStreetNo);
		if( s_iter != kCont->end() )
		{
			CONT_TOWN2GROUND::mapped_type const& kGroundNo = s_iter->second;
			BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_ENTER);
			kPacket.Push(kGroundNo);
			kPacket.Push((int)1);
			NETWORK_SEND(kPacket);

			while( pParent->Parent() )
			{
				pParent = pParent->Parent();
			}
			pParent->Close();
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201107, true);
		}
	}

	void OnClickHomePost()
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( !CheckExistHaveHome() || !CheckMyHome() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201112, true);
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		PgInventory* pkInv = pkHomeUnit->GetInven();
		if( pkInv )
		{
			CONT_HAVE_ITEM_DATA kItemCont;
			if( S_OK == pkInv->GetItems(IT_HOME, UICT_HOME_POSTBOX, kItemCont) )
			{	//기간 확인
				CONT_HAVE_ITEM_DATA::const_iterator item_it = kItemCont.begin();
				while(kItemCont.end()!=item_it)
				{
					PgBase_Item const& rkBaseItem = (*item_it).second;
					if(!rkBaseItem.IsEmpty())
					{
						if( rkBaseItem.GetUseAbleTime() >= 0 )// 기간이 지나지 않았으면
						{//우체통 연다.
							CallPostUI(lwGUID(pkHomeUnit->GetID()));				
							return;
						}
					}
					++item_it;
				}
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
			}
			else
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201117, true);
			}
			return;
		}
		//없어
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201117, true);
	}

	void OnClickHomeSafeInv()
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( !CheckExistHaveHome() || !CheckMyHome() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201112, true);
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		PgInventory* pkInv = pkHomeUnit->GetInven();
		if( pkInv )
		{
			CONT_HAVE_ITEM_DATA kItemCont;
			if( S_OK == pkInv->GetItems(IT_HOME, UICT_HOME_SAFEBOX, kItemCont) )
			{	//기간 확인
				CONT_HAVE_ITEM_DATA::const_iterator item_it = kItemCont.begin();
				while(kItemCont.end()!=item_it)
				{
					PgBase_Item const& rkBaseItem = (*item_it).second;
					if(!rkBaseItem.IsEmpty())
					{
						if( rkBaseItem.GetUseAbleTime() >= 0 )// 기간이 지나지 않았으면
						{//창고를 연다.
							PgSafeInventory::CallInvSafeUI(lwGUID(pkHomeUnit->GetID()));
							return;
						}
					}
					++item_it;
				}
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
			}
			else
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201118, true);
			}
			return;
		}
		//없어
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201118, true);
	}

	void OnClickMinimapTownNumPopup(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_TOWNMAP_TOWN_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;

		PgHomeUI::CONT_TOWNNO const& kTownNoList = g_kHomeTownMgr.TownNoList();
		PgHomeUI::CONT_TOWNNO::const_iterator townNo_itor = kTownNoList.begin();
		while( townNo_itor != kTownNoList.end() )
		{
			PgHomeUI::CONT_TOWNNO::value_type const& kTownNo = (*townNo_itor);

			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				pListItem->m_pWnd->SetCustomData(&kTownNo, sizeof(kTownNo));
				//swprintf_s(szTemp, MAX_PATH, TTW(201003).c_str(), kTownNo);
				pListItem->m_pWnd->Text(lwHomeUIUtil::GetTownName(kTownNo));
				ItemHeight = pListItem->m_pWnd->Size().y;
			}
			++townNo_itor;
		}

		if( !pList->GetTotalItemCount() )
		{
			pDropWnd->Close();
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	void OnClickMinimapTownNumPopupItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}
		short siStreetNo = 0;
		pSelf->GetCustomData(&siStreetNo, sizeof(siStreetNo));

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_TOWN_MAP_INFO");
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropTitle = pMainUI->GetControl(L"SFRM_TOWN_TEXT");
		if( !pDropTitle )
		{
			return;
		}
		SHOMEADDR kAddr;
		pDropTitle->GetCustomData(&kAddr, sizeof(kAddr));
		kAddr.StreetNo(siStreetNo);
		pDropTitle->SetCustomData(&kAddr, sizeof(kAddr));

		OnCallHomeTownMinimap(siStreetNo);
	}

	void OnClickMinimapHomeItem(lwUIWnd kParent, int const siHouseNo)
	{
		XUI::CXUI_Wnd* pMainUI = kParent.GetSelf();
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pTownUI = pMainUI->GetControl(L"SFRM_TOWN_TEXT");
		if( !pTownUI )
		{
			return;
		}

		SHOMEADDR kAddr;
		pTownUI->GetCustomData(&kAddr, sizeof(kAddr));

		BM::vstring kStr(L"BTN_HOME_ITEM");
		kStr += kAddr.HouseNo();

		XUI::CXUI_Wnd* pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(false);
			pWnd->TwinkleStartTime(0);
		}

		kStr = L"BTN_HOME_ITEM";
		kStr += siHouseNo;

		pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(true);
			pWnd->TwinkleInterTime(100);
		}

		kAddr.HouseNo(siHouseNo);
		pTownUI->SetCustomData(&kAddr, sizeof(kAddr));

		SMYHOME kHome;
		if( !g_kHomeTownMgr.GetHomeInfo(kAddr.StreetNo(), siHouseNo, kHome) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201054, true);
			OnSetSelectedHomeInfo(pMainUI, false, SMYHOME());
			return;
		}

		bool bIsMyHome = PgHomeUIUtil::CheckMyHome(kHome);
		OnSetSelectedHomeInfo(pMainUI, bIsMyHome, kHome);
	}

	void OnClickGoToMyHome()
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( 0 == (g_pkWorld->GetAttr() & GATTR_FLAG_MYHOME) )
		{
			if( 0 != (g_pkWorld->GetAttr() & GATTR_FLAG_CANT_HOMEWARP) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201510, true);
				return;
			}
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return;
		}
		if(pkPlayer->GetAbil(AT_DUEL) > 0) //결투중이면 마이홈 입장 불가
		{
			::Notice_Show( TTW(201209), EL_Warning );
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_MYHOME_ENTER);
		kPacket.Push(pkPlayer->HomeAddr().StreetNo());
		kPacket.Push(pkPlayer->HomeAddr().HouseNo());
		if( g_kExpedition.ExpeditionGuid().IsNotNull() )
		{
			std::wstring kMessage = TTW(720048) + std::wstring(L"\n") + TTW(201120);
			CallCommonMsgYesNoBox(kMessage, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
		}
		else if( g_kParty.PartyGuid().IsNotNull() )
		{
			std::wstring kMessage = TTW(201119) + std::wstring(L"\n") + TTW(201120);
			CallCommonMsgYesNoBox(kMessage, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
		}
		else
		{
			NETWORK_SEND(kPacket);
		}
	}

	void OnCallHomeTownMinimap(short const siStreetNo, int const iHouseNo)
	{
		if(!g_pkWorld)
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_TOWN_MAP_INFO");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"SFRM_TOWN_MAP_INFO");
			if( !pMainUI )
			{
				return;
			}
		}

		if( !g_pkWorld->IsHaveAttr(GATTR_FLAG_HOMETOWN) )
		{
			RegistUIAction(pMainUI, L"CloseUI");
		}

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"SFRM_TOWN_TEXT");
		if( !pTemp )
		{
			return;
		}
		SHOMEADDR kAddr;
		pTemp->GetCustomData(&kAddr, sizeof(kAddr));

		BM::vstring kStr(L"BTN_HOME_ITEM");
		kStr += kAddr.HouseNo();
		XUI::CXUI_Wnd* pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(false);
			pWnd->TwinkleStartTime(0);
		}

		kStr = L"BTN_HOME_ITEM";
		kStr += iHouseNo;
		pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(true);
			pWnd->TwinkleInterTime(100);
		}

		wchar_t szTemp[MAX_PATH] = {0,};
		//swprintf_s(szTemp, TTW(201003).c_str(), siStreetNo);
		pTemp->Text(lwHomeUIUtil::GetTownName(siStreetNo));
		kAddr.StreetNo(siStreetNo);
		kAddr.HouseNo(iHouseNo);
		pTemp->SetCustomData(&kAddr, sizeof(kAddr));

		XUI::CXUI_Wnd* pBackImg = pMainUI->GetControl(L"SFRM_TOWNMAP");
		if( pBackImg )
		{
			wchar_t szTemp[MAX_PATH]={0,};
			swprintf_s(szTemp, MAX_PATH, L"../Data/6_ui/myhome/mhMap%02d.tga", siStreetNo);
			pBackImg->DefaultImgName(szTemp);
		}

		CONT_MYHOME	kHomeList;
		g_kHomeTownMgr.GetTownHouseCont( siStreetNo, kHomeList );

		int iFirstHouse = iHouseNo;

		CONT_MYHOME::iterator home_iter = kHomeList.begin();
		for(int i = 1; i <= HOTOWN_MAX_HOUSE_COUNT; ++i)
		{
			BM::vstring vStr(L"BTN_HOME_ITEM");
			vStr += i;

			XUI::CXUI_Button* pMiniMapIcon = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(vStr));
			if( pMiniMapIcon )
			{
				if( home_iter != kHomeList.end() && home_iter->first.HouseNo() == i)
				{
					CONT_MYHOME::mapped_type const& kHome = home_iter->second;

					if( iFirstHouse == 0 )
					{
						iFirstHouse = kHome.iHouseNo;
					}

					NiScreenTexture::ScreenRect* pRect = NULL;
					int iRectCount = 0;
					{
						PgUISpriteObject *pkSpriteObj = (PgUISpriteObject *)pMiniMapIcon->BtnImg();
						if( pkSpriteObj )
						{
							NiScreenTexture *pkSprite = pkSpriteObj->GetTex();
							iRectCount = pkSprite->GetNumScreenRects();
							pRect = NiNew NiScreenTexture::ScreenRect[ iRectCount ];
							if( pRect )
							{
								for(int i = 0; i < iRectCount; ++i)
								{
									pRect[i] = pkSprite->GetScreenRect(i);
								}
							}
						}
					}

					if( g_kPilotMan.IsMyPlayer( kHome.kOwnerGuid ) )
					{
						pMiniMapIcon->ButtonChangeImage(L"../Data/6_ui/myhome/mhIcon04.tga");
					}
					else
					{
						if( MAS_IS_BIDDING == kHome.bAuctionState )
						{
							pMiniMapIcon->ButtonChangeImage(L"../Data/6_ui/myhome/mhIcon03.tga");
						}
						else
						{
							if( PgHomeUIUtil::EHCR_CLOSE == PgHomeUIUtil::CheckToHomeVisitPossible( kHome ) )
							{
								pMiniMapIcon->ButtonChangeImage(L"../Data/6_ui/myhome/mhIcon02.tga");
							}
							else
							{
								pMiniMapIcon->ButtonChangeImage(L"../Data/6_ui/myhome/mhIcon01.tga");
							}
						}
					}

					{
						PgUISpriteObject *pkSpriteObj = (PgUISpriteObject *)pMiniMapIcon->BtnImg();
						if( pkSpriteObj )
						{
							if( pRect )
							{
								NiScreenTexture *pkSprite = pkSpriteObj->GetTex();
								for(int i = 0; i < iRectCount; ++i)
								{
									pkSprite->AddNewScreenRect(pRect[i].m_sPixTop, pRect[i].m_sPixLeft, pRect[i].m_usPixWidth, pRect[i].m_usPixHeight,
										pRect[i].m_usTexTop, pRect[i].m_usTexLeft, pRect[i].m_kColor);
								}
								NiDelete [] pRect;
								pRect = NULL;
							}
						}
					}

					pMiniMapIcon->Visible(true);
					++home_iter;
				}
				else
				{
					pMiniMapIcon->Visible(false);
				}
			}
		}

		OnClickMinimapHomeItem(lwUIWnd(pMainUI), iFirstHouse);
	}

	void OnSetSelectedHomeInfo(XUI::CXUI_Wnd* pMainUI, bool const bIsMyHome, SMYHOME const& kHome)
	{
		if( !pMainUI )
		{
			return;
		}

		bool bIsClear = (kHome.siStreetNo == 0 || kHome.iHouseNo == 0);

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"FRM_NAME");
		if( pTemp )
		{
			pTemp->Text((bIsClear)?(L""):(kHome.kName));
		}

		pTemp = pMainUI->GetControl(L"FRM_ADDR");
		if( pTemp && !bIsClear)
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			std::wstring kLocStr;
			//swprintf_s(szTemp, TTW(201003).c_str(), kHome.siStreetNo);
			kLocStr = lwHomeUIUtil::GetTownName(kHome.siStreetNo);
			kLocStr += L" ";
			swprintf_s(szTemp, TTW(201004).c_str(), kHome.iHouseNo);
			kLocStr += szTemp;
			pTemp->Text(kLocStr);
		}
		else
		{
			pTemp->Text(L"");
		}

		pTemp = pMainUI->GetControl(L"FRM_STATE");
		if( pTemp && !bIsClear )
		{
			if( bIsMyHome )
			{
				pTemp->Text(TTW(201092));
			}
			else if( kHome.bAuctionState == MAS_IS_BIDDING )
			{
				pTemp->Text(TTW(201091));
			}
			else
			{
				switch( PgHomeUIUtil::CheckToHomeVisitPossible(kHome) )
				{
				case PgHomeUIUtil::EHCR_CLOSE:
					{
						pTemp->Text(TTW(201090));
					}break;
				default:
					{
						pTemp->Text(TTW(201089));
					}break;
				}
			}
		}
		else
		{
			pTemp->Text(L"");
		}
	}
	void OnCallInvitatianNotice(std::wstring const& kUserName)
	{
		wchar_t szTemp[MAX_PATH] = {0, };
		::swprintf_s(szTemp, MAX_PATH, TTW(201108).c_str(), kUserName.c_str());

		XUI::CXUI_Wnd* pNotice = XUIMgr.Activate(L"FRM_INVITE_NOTICE");
		if( pNotice )
		{
			pNotice->Text(szTemp);
			pNotice->AliveTime(6000);
			pNotice->TwinkleTime(6000);
			pNotice->TwinkleInterTime(500);
			pNotice->SetTwinkle(true);
		}

		XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"FRM_MINIMAP");
		if( pkMiniMap )
		{
			XUI::CXUI_Wnd* pkBtn = pkMiniMap->GetControl(L"BTN_HOME_INVITE_LIST");
			if( pkBtn )
			{
				pkBtn->TwinkleTime(UINT_MAX);
				pkBtn->TwinkleInterTime(500);
				pkBtn->SetTwinkle(true);				
			}
		}
	}

	bool CheckExistHaveHome()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		return !pkPlayer->HomeAddr().IsNull();
	}

	bool CheckMyHome()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if(!pkHome)
		{
			return false;
		}
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return false;
		}
		if(!g_kPilotMan.IsMyPlayer(pkHomeUnit->OwnerGuid()))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790600, true);
			return false;
		}

		return true;
	}

	// 세금 관련 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	bool OnCallUIHomePayTax(SMYHOME const& kHome)
	{
		__int64 i64Temp =0;
		CGameTime::SystemTime2SecTime(kHome.kPayTexTime, i64Temp);
		__int64 const& i64TexPayTimeSec = i64Temp;

		
		SYSTEMTIME kNowTime;
		g_kEventView.GetLocalTime(&kNowTime);
		__int64 i64NowTimeSec = 0;
		CGameTime::SystemTime2SecTime(kNowTime, i64NowTimeSec);
		
		__int64 const i64DistTimeSec = i64TexPayTimeSec - i64NowTimeSec;

		__int64 i64DelayWeek = 1;
		if(0 > i64DistTimeSec && lwHomeRenew::IsMyhomeApt() )
		{
			i64DelayWeek = SecToWeek(-i64DistTimeSec, false);
			if( i64DelayWeek >= 4 )
			{
				return lwHomeRenew::OnCallMyhomeAptRelive(kHome);
			}
			else
			{
				return OnCallUIHomePayTaxDetail(kHome);
			}
		}
		else
		{
			return OnCallUIHomePayTaxDetail(kHome);
		}
	}

	bool OnCallUIHomePayTaxDetail(SMYHOME const& kHome)
	{//세금 납부 UI에 값 세팅 부분
		//플레이어의 홈 정보를 얻어와
		XUI::CXUI_Wnd* pkTaxPay = XUIMgr.Call(WSTR_HOME_PAY_TAX_UI);
		if(pkTaxPay)
		{
			XUI::CXUI_Wnd* pkUI = pkTaxPay->GetControl(L"FRM_UI");
			if( pkUI )
			{
				XUI::CXUI_Wnd* pkNotice2 = pkUI->GetControl(L"FRM_NOTICE2");
				if( pkNotice2 )
				{
					if( lwHomeRenew::IsMyhomeApt() )
					{
						pkNotice2->Text(TTW(201810));
					}
					else
					{
						pkNotice2->Text(TTW(405108));
					}
				}
			}
			// 세금 납부 시간을 초로 변환
			__int64 i64Temp =0;
			CGameTime::SystemTime2SecTime(kHome.kPayTexTime, i64Temp);
			__int64 const& i64TexPayTimeSec = i64Temp;

			// 현재 시간을 초로 변환해서
			SYSTEMTIME kNowTime;
			g_kEventView.GetLocalTime(&kNowTime);
			__int64 i64NowTimeSec = 0;
			CGameTime::SystemTime2SecTime(kNowTime, i64NowTimeSec);
			// 연체 여부를 확인
			__int64 const i64DistTimeSec = i64TexPayTimeSec - i64NowTimeSec;

			XUI::CXUI_Wnd* pkPayDay = pkTaxPay->GetControl(_T("FRM_DAY_PAYMENT_VALUE"));
			if(pkPayDay)
			{// 납부일자 값 세팅
				SYSTEMTIME kPayDay = kHome.kPayTexTime;
				//CGameTime::SecTime2SystemTime(i64TexPayTimeSec, kPayDay);
				wchar_t szBuf[MAX_PATH]={0,};
				::swprintf_s(szBuf, MAX_PATH, TTW(405109).c_str(), kPayDay.wMonth, kPayDay.wDay, kPayDay.wHour, kPayDay.wMinute);
				pkPayDay->Text(szBuf);
			}

			XUI::CXUI_Wnd* pkDelay = pkTaxPay->GetControl(_T("FRM_DELAY_VALUE"));
			__int64 i64DelayWeek = 1;
			if(pkPayDay)
			{// 연체여부 값 세팅
				if(0 > i64DistTimeSec)
				{// 연체 되었다면, 연체된 주를 계산하고
					i64DelayWeek = SecToWeek(-i64DistTimeSec);
					wchar_t szBuf[MAX_PATH]={0,};
					::swprintf_s(szBuf, MAX_PATH, TTW(405111).c_str(), i64DelayWeek);
					pkDelay->Text(szBuf);
				}
				else
				{// 연체 되지 않았다는 메세지 보여주고
					pkDelay->Text(TTW(405116));
				}
			}
			{// 세금 값 세팅을 세팅하고(연체된 값 만큼)
				__int64 i64TaxValue = MYHOMEUTIL::CalcTex(kHome);
				pkTaxPay->SetCustomData(&i64TaxValue, sizeof(i64TaxValue));
				__int64 i64Gold = 0;
				__int64 i64Silver = 0;
				__int64 i64Copper = 0;
				__int64 i64Temp = i64TaxValue/10000;
				if (0<i64Temp)
				{// 골드
					i64Gold = i64Temp;
					i64TaxValue -= i64Temp*10000;
				}
				i64Temp = i64TaxValue/100;
				if (0<i64Temp)
				{// 실버
					i64Silver = i64Temp;
					i64TaxValue -= i64Temp*100;
				}
				if (0<i64TaxValue)
				{// 쿠퍼
					i64Copper = i64TaxValue;
				}
				XUI::CXUI_Wnd* pkTaxGold = pkTaxPay->GetControl(_T("FRM_COIN1"));
				if(pkTaxGold)
				{
					pkTaxGold->Text(BM::vstring(i64Gold));
				}
				XUI::CXUI_Wnd* pkTaxSilver = pkTaxPay->GetControl(_T("FRM_COIN2"));
				if(pkTaxSilver)
				{
					pkTaxSilver->Text(BM::vstring(i64Silver));
				}
				XUI::CXUI_Wnd* pkTaxCopper = pkTaxPay->GetControl(_T("FRM_COIN3"));
				if(pkTaxCopper)
				{
					pkTaxCopper->Text(BM::vstring(i64Copper));
				}
			}
			XUI::CXUI_Wnd* pkLimitPayDay = pkTaxPay->GetControl(_T("FRM_LIMIT_PAYMENT_VALUE"));
			if(pkLimitPayDay)
			{// 최종 납부 기한 세팅
				SYSTEMTIME kLimitPayDay;
				__int64 i64LimitPayTimeSec = (i64TexPayTimeSec+(i64OneWeekSec*4));
				CGameTime::SecTime2SystemTime(i64LimitPayTimeSec, kLimitPayDay);
				wchar_t szBuf[MAX_PATH]={0,};
				::swprintf_s(szBuf, MAX_PATH, TTW(405109).c_str(), kLimitPayDay.wMonth, kLimitPayDay.wDay, kLimitPayDay.wHour, kLimitPayDay.wMinute);
				pkLimitPayDay->Text(szBuf);
			}
		}
		return true;
	}

	bool ReserveMyHomeTaxPayEvent()
	{// 세금 납부창 호출 이벤트 예약
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer && !pkPlayer->HomeAddr().IsNull() )
		{
			OnSendHomeInfoRequest(pkPlayer->HomeAddr(), lwHomeUIUtil::EEVT_CALL_PAY_TAX_UI);
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return false;
		}
		return true;
	}

	bool SendMyHomeTaxPay_Packet()
	{// 세금 납부
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		if(pkPlayer->HomeAddr().IsNull())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return false;
		}

		// UI를 호출하면서 저장한 세금 값을 얻어와
		__int64 i64TaxValue = 0;
		XUI::CXUI_Wnd* pkTaxPay = XUIMgr.Get(WSTR_HOME_PAY_TAX_UI);
		if(!pkTaxPay)
		{
			return false;
		}
		pkTaxPay->GetCustomData(&i64TaxValue, sizeof(i64TaxValue));
		__int64 const& i64MyMoney = pkPlayer->GetInven()->Money();
		if(i64MyMoney < i64TaxValue)
		{// 소지금과 비교하여
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405115, true);
			return false;
		}
		// 패킷을 보낸다
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_PAY_TEX);
		NETWORK_SEND(kPacket);
		return true;
	}

	int	SecToWeek(__int64 const& i64Sec, bool const& bRoundOff) // 초를 주단위로 환산
	{		
		int iWeek = i64Sec/(i64OneWeekSec);
		int iDaySec = i64Sec%(i64OneWeekSec);
		if(bRoundOff && 0 < iDaySec)
		{//올림
			++iWeek;
		}
		return iWeek;
	}

	// 세금 밀려서 소유권 박탈 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	bool OnCallUIDeprivedHome(short const& sStreetNo, int const& sHouseNo, __int64 const& i64Tax)
	{// 소유권 박탈 UI 값 세팅 부분
		SMYHOME kHomeInfo;
		if(!g_kHomeTownMgr.GetHomeInfo(sStreetNo, sHouseNo, kHomeInfo))
		{
			return false;
		}
		XUI::CXUI_Wnd* pkHomeless = XUIMgr.Call(WSTR_HOME_DEPRIVED_HOME_UI);
		if(pkHomeless)
		{
			XUI::CXUI_Wnd* pkBG = pkHomeless->GetControl(_T("FRM_UI"));
			if(pkBG)
			{
				XUI::CXUI_Wnd* pkNotice = pkBG->GetControl(_T("FRM_NOTICE"));
				if(pkNotice)
				{// 집 위치, %s %d
					{
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405112).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(),sHouseNo);
						pkNotice->Text(szBuf);
					}
					{// 시스템 메세지 출력
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405118).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo);
						SChatLog kChatLog(CT_NOTICE);
						g_kChatMgrClient.AddLogMessage(kChatLog, szBuf, false);
					}
				}
			}
			{// 연체된 세금 값
				__int64 i64Gold = 0;
				__int64 i64Silver = 0;
				__int64 i64Copper = 0;

				__int64 i64TaxValue = i64Tax;
				__int64 i64Temp = i64TaxValue/10000;
				if (0<i64Temp)
				{// 골드
					i64Gold = i64Temp;
					i64TaxValue -= i64Temp*10000;
				}
				i64Temp = i64TaxValue/100;
				if (0<i64Temp)
				{// 실버
					i64Silver = i64Temp;
					i64TaxValue -= i64Temp*100;
				}
				if (0<i64TaxValue)
				{// 쿠퍼
					i64Copper = i64TaxValue;
				}
				XUI::CXUI_Wnd* pkTaxGold = pkHomeless->GetControl(_T("FRM_COIN1"));
				if(pkTaxGold)
				{
					pkTaxGold->Text(BM::vstring(i64Gold));
				}
				XUI::CXUI_Wnd* pkTaxSilver = pkHomeless->GetControl(_T("FRM_COIN2"));
				if(pkTaxSilver)
				{
					pkTaxSilver->Text(BM::vstring(i64Silver));
				}
				XUI::CXUI_Wnd* pkTaxCopper = pkHomeless->GetControl(_T("FRM_COIN3"));
				if(pkTaxCopper)
				{
					pkTaxCopper->Text(BM::vstring(i64Copper));
				}
			}
			return true;
		}
		return false;
	}
	
	bool OnCallNotiDelayTax(short const& sStreetNo, int const& sHouseNo, __int64 const& i64Tax, BM::PgPackedTime const& kPayTaxTime)
	{// 세금 연체 알림
		XUI::CXUI_Wnd* pkHomeless = XUIMgr.Call(WSTR_HOME_DEPRIVED_HOME_UI);
		if(pkHomeless)
		{
			XUI::CXUI_Wnd* pkBG = pkHomeless->GetControl(_T("FRM_UI"));
			if(pkBG)
			{
				XUI::CXUI_Wnd* pkNotice = pkBG->GetControl(_T("FRM_NOTICE"));
				__int64 i64DelayWeek = 0;
				{
					// 세금 납부 시간을 초로 변환
					__int64 i64Temp =0;
					CGameTime::SystemTime2SecTime(kPayTaxTime, i64Temp);
					__int64 const& i64TexPayTimeSec = i64Temp;
					// 현재 시간을 초로 변환해서
					SYSTEMTIME kNowTime;
					g_kEventView.GetLocalTime(&kNowTime);
					__int64 i64NowTimeSec = 0;
					CGameTime::SystemTime2SecTime(kNowTime, i64NowTimeSec);
					// 연체된 시간(초)을 구해서
					__int64 const i64DistTimeSec = i64TexPayTimeSec - i64NowTimeSec;
					// 연체된 주를 계산하고
					i64DelayWeek = SecToWeek(-i64DistTimeSec);	//올림 함
				}
				{// 연체여부 값 세팅
					if(pkNotice)
					{// 집 위치, 연체 세금을 표시한다 %s %d %d
						{
							wchar_t szBuf[MAX_PATH]={0,};
							if( g_kHomeRenewMgr.IsMyhomeApt(sStreetNo) )
							{
								::swprintf_s(szBuf, MAX_PATH, TTW(201816).c_str(), i64DelayWeek);
							}
							else
							{
								::swprintf_s(szBuf, MAX_PATH, TTW(405117).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo, i64DelayWeek);
							}
							pkNotice->Text(szBuf);
						}
						{// 시스템 메세지 출력
							wchar_t szBuf[MAX_PATH]={0,};
							if( g_kHomeRenewMgr.IsMyhomeApt(sStreetNo) )
							{
								::swprintf_s(szBuf, MAX_PATH, TTW(201817).c_str(), i64DelayWeek);
							}
							else
							{
								::swprintf_s(szBuf, MAX_PATH, TTW(405119).c_str(), lwHomeUIUtil::GetTownName(sStreetNo).c_str(), sHouseNo, i64DelayWeek);
							}
							SChatLog kChatLog(CT_NOTICE);
							g_kChatMgrClient.AddLogMessage(kChatLog, szBuf, false);
						}
						
					}
					{// 연체된 세금 값
						__int64 i64Gold = 0;
						__int64 i64Silver = 0;
						__int64 i64Copper = 0;

						__int64 i64TaxValue = i64Tax;
						__int64 i64Temp = i64TaxValue/10000;
						if (0<i64Temp)
						{// 골드
							i64Gold = i64Temp;
							i64TaxValue -= i64Temp*10000;
						}
						i64Temp = i64TaxValue/100;
						if (0<i64Temp)
						{// 실버
							i64Silver = i64Temp;
							i64TaxValue -= i64Temp*100;
						}
						if (0<i64TaxValue)
						{// 쿠퍼
							i64Copper = i64TaxValue;
						}
						XUI::CXUI_Wnd* pkTaxGold = pkHomeless->GetControl(_T("FRM_COIN1"));
						if(pkTaxGold)
						{
							pkTaxGold->Text(BM::vstring(i64Gold));
						}
						XUI::CXUI_Wnd* pkTaxSilver = pkHomeless->GetControl(_T("FRM_COIN2"));
						if(pkTaxSilver)
						{
							pkTaxSilver->Text(BM::vstring(i64Silver));
						}
						XUI::CXUI_Wnd* pkTaxCopper = pkHomeless->GetControl(_T("FRM_COIN3"));
						if(pkTaxCopper)
						{
							pkTaxCopper->Text(BM::vstring(i64Copper));
						}
					}
					return true;
				}
			}
		}
		return false;
	}

	bool IsHomeBidding()
	{
		if( g_pkWorld && g_pkWorld->GetHome() && g_pkWorld->GetHome()->GetHomeUnit() )
		{
			int iState = g_pkWorld->GetHome()->GetHomeUnit()->GetAbil(AT_MYHOME_STATE);
			if( MAS_IS_BIDDING == iState )
			{
				return true;
			}
		}
		//SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		//if( kHome.bAuctionState == MAS_IS_BIDDING )
		//{
		//	return true;
		//}

		return false;
	}

	bool IsHomeBlock()
	{
		if( g_pkWorld && g_pkWorld->GetHome() && g_pkWorld->GetHome()->GetHomeUnit() )
		{
			int iState = g_pkWorld->GetHome()->GetHomeUnit()->GetAbil(AT_MYHOME_STATE);
			if( MAS_IS_BLOCK == iState )
			{
				return true;
			}
		}
		//SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		//if( kHome.bAuctionState == MAS_IS_BLOCK)
		//{
		//	return true;
		//}

		return false;
	}

	// 내 소유 홈의 경매 정보  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	bool OnCallUIHomeAuctionInfo(SMYHOME const& kHome)
	{
		if( kHome.bAuctionState == MAS_IS_BIDDING )
		{
			if( g_kPilotMan.IsMyPlayer( kHome.kOwnerGuid ) )
			{
				XUI::CXUI_Wnd* pkAuctionInfo = XUIMgr.Call(L"SFRM_MYHOME_REG_INFO");
				if(pkAuctionInfo)
				{	
					XUI::CXUI_Wnd* pkAddress = pkAuctionInfo->GetControl(L"FRM_ADDRESS_VALUE");
					if(pkAddress)
					{
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405126).c_str(), lwHomeUIUtil::GetTownName(kHome.siStreetNo).c_str(), kHome.iHouseNo);
						pkAddress->Text(szBuf);
					}

					XUI::CXUI_Wnd* pkLimitTime = pkAuctionInfo->GetControl(L"FRM_TIME_VALUE");
					if(pkLimitTime)
					{
						SYSTEMTIME kTime =  kHome.kAuctionTime;
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405128).c_str(), kTime.wMonth, kTime.wDay, kTime.wHour, kTime.wMinute);
						pkLimitTime->Text(szBuf);
					}

					XUI::CXUI_Wnd* pkStartCost = pkAuctionInfo->GetControl(L"FRM_STARTCOST_VALUE");
					if(pkStartCost)
					{
						BM::vstring vStr(kHome.i64FirstBiddingCost / 10000);
						pkStartCost->Text(vStr);
					}

					XUI::CXUI_Wnd* pkLastCost = pkAuctionInfo->GetControl(L"FRM_NOWCOST_VALUE");
					if(pkLastCost)
					{
						BM::vstring vStr(kHome.i64LastBiddingCost / 10000);
						pkLastCost->Text(vStr);
					}
				}
			}
			else
			{
				XUI::CXUI_Wnd* pkAuctionInfo = XUIMgr.Call(WSTR_HOME_AUCTION_INFO_UI);
				if(pkAuctionInfo)
				{	
					XUI::CXUI_Wnd* pkBid = pkAuctionInfo->GetControl(_T("FRM_BIDDING_VALUE"));
					if(pkBid)
					{//입찰 가격	
						BM::vstring kBidValue(kHome.i64LastBiddingCost);
						pkBid->Text(kBidValue);
					}

					XUI::CXUI_Wnd* pkAddress = pkAuctionInfo->GetControl(_T("FRM_ADRESS_VALUE"));
					if(pkAddress)
					{//집 번지
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405126).c_str(), kHome.siStreetNo, kHome.iHouseNo);
						pkAddress->Text(szBuf);
					}
					//XUI::CXUI_Wnd* pkTime = pkAuctionInfo->GetControl(_T("FRM_BIDDING_TYPE_VALUE"));
					//if(pkTime)
					//{ // %d 시간
					//	wchar_t szBuf[MAX_PATH]={0,};
					//	//::swprintf_s(szBuf, MAX_PATH, TTW(405125).c_str(), );					
					//	//pkTime->Text(szBuf);
					//}
					XUI::CXUI_Wnd* pkLimitTime = pkAuctionInfo->GetControl(_T("FRM_LIMIT_TIME_VALUE"));
					if(pkLimitTime)
					{// 경매 마감 시간
						SYSTEMTIME kTime =  kHome.kAuctionTime;
						wchar_t szBuf[MAX_PATH]={0,};
						::swprintf_s(szBuf, MAX_PATH, TTW(405128).c_str(), kTime.wMonth, kTime.wDay, kTime.wHour, kTime.wMinute);
						pkLimitTime->Text(szBuf);
					}
				}
			}
		}
		else
		{//경매중이 아니다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201512, true);
		}
		return true;
	}

	//		/////////////////////////////////////////////////////////////////////////////////////////
	bool ReserveMyHomeOptionEvent()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if(!pkHome)
		{// 집이 없는 곳에서 옵션을 설정할수 없고
			return false;
		}
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if(!pkHomeUnit)
		{
			return false;
		}
		if(!g_kPilotMan.IsMyPlayer( pkHomeUnit->OwnerGuid()))
		{// 남의 집에서 옵션 UI를 열수없다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790600, true);
			return false;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer && !pkPlayer->HomeAddr().IsNull() )
		{
			OnSendHomeInfoRequest(pkPlayer->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_OPTION_UI);
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return false;
		}
		return true;
	}

	bool OnCallUIHomeOption(SMYHOME const& kHome)
	{
		//플레이어의 홈 정보를 얻어와
		XUI::CXUI_Wnd* pkOption = XUIMgr.Call(WSTR_HOME_OPTION_UI);
		if(pkOption)
		{// UI에 체크 표시를 설정함
			lwUIWnd klwOptionWnd(pkOption);
			char const* const pcLuaFunName = "MyHome_Option_ChkBtn_OnOff";
			{// 모두 공개 여부
				if( true ==  g_kHomeRenewMgr.IsMyhomeApt(kHome.siStreetNo) )
				{//마이홈 아파트면 전체 비공개
					lua_tinker::call<void, lwUIWnd, int, bool>(pcLuaFunName,
						klwOptionWnd, lwHomeUIUtil::EHOME_PUBILC_OPTION, false);
				}
				else
				{
					lua_tinker::call<void, lwUIWnd, int, bool>(pcLuaFunName,
						klwOptionWnd, lwHomeUIUtil::EHOME_PUBILC_OPTION, (MEV_ALL & kHome.bEnableVisitBit));
				}
			}
			{// 친구
				lua_tinker::call<void, lwUIWnd, int, bool>(pcLuaFunName,
					klwOptionWnd, lwHomeUIUtil::EFRIEND_PUBILC_OPTION, (MEV_FRIEND & kHome.bEnableVisitBit));
			}
			{// 커플
				lua_tinker::call<void, lwUIWnd, int, bool>(pcLuaFunName,
					klwOptionWnd, lwHomeUIUtil::ECOUPLE_PUBLIC_OPTION, (MEV_COUPLE & kHome.bEnableVisitBit));
			}
			{// 길드원
				lua_tinker::call<void, lwUIWnd, int, bool>(pcLuaFunName,
					klwOptionWnd, lwHomeUIUtil::EGUILD_PEOPLE_PUBILC_OPTION, (MEV_GUILD & kHome.bEnableVisitBit));
			}
			//pkOption->SetCustomData(&kHome, kHome.min_size());
		}
		return true;
	}

	bool SendMyHomeOption_Packet(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkOption = kWnd.GetSelf();
		if(pkOption)
		{
			BYTE byFlag = 0;
			XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkOption->GetControl(L"CBTN_PUBLIC_HOME_ON"));
			if(pkChkBtn)
			{// 모두에게 공개
				if(pkChkBtn->Check())
				{
					byFlag |= MEV_ALL;
				}
			}
			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkOption->GetControl(L"CBTN_PUBLIC_FRIEND_ON"));
			if(pkChkBtn)
			{// 친구에게 공개
				if(pkChkBtn->Check())
				{
					byFlag |= MEV_FRIEND;
				}
			}
			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkOption->GetControl(L"CBTN_PUBLIC_COUPLE_ON"));
			if(pkChkBtn)
			{// 커플에게 공개
				if(pkChkBtn->Check())
				{
					byFlag |= MEV_COUPLE;
				}
			}
			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkOption->GetControl(L"CBTN_PUBLIC_GUILD_ON"));
			if(pkChkBtn)
			{// 길드원에게 공개
				if(pkChkBtn->Check())
				{
					byFlag |= MEV_GUILD;
				}
			}
			SMYHOME kHome = g_kHomeTownMgr.GetMyHomeInfo();
			//pkOption->GetCustomData(&kHome, kHome.min_size());
			BM::Stream kPacket(PT_C_M_REQ_HOME_VISITFLAG_MODIFY);
			kPacket.Push(kHome.kHomeInSideGuid);
			kPacket.Push(byFlag);
			NETWORK_SEND(kPacket);
		}
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////////////////	
	// 데코레이션 부가기능 (버프 받기)
	bool ReserveMyHomeUseDecoUIEvent()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_USEDECO_UI);
		}
		else
		{// 집으로 들어왔는데 집이 없다?
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1923, true);
			return false;
		}
		return true;
	}

	bool OnCallUIUseDecoUI(SMYHOME const& kHome)
	{
		//플레이어의 홈 정보를 얻어와
		{// 홈 Unit에 장착된 데코레이션 아이템들을 얻어온다
			PgMyHome* pkMyHome = NULL;
			{// 내 홈의
				PgPilot* pkTemp = g_kPilotMan.FindPilot(kHome.kHomeInSideGuid);
				if(!pkTemp)
				{
					return false;
				}
				pkMyHome = dynamic_cast<PgMyHome*>(pkTemp->GetUnit());
				if(!pkMyHome)
				{
					return false;
				}
			}
			PgInventory* pkHomeInv = pkMyHome->GetInven();
			if(!pkHomeInv)
			{
				return false;
			}
			// 데코레이션 아이템에 들어있는
			CONT_HAVE_ITEM_NO kContHomeItem;
			pkHomeInv->GetItems(IT_HOME, kContHomeItem);
			CONT_HAVE_ITEM_NO::const_iterator Item_itor = kContHomeItem.begin();
			GET_DEF(CItemDefMgr, kItemDefMgr);
			XUI::CXUI_Wnd* pkUseDeco = XUIMgr.Call(WSTR_HOME_USE_DECO_UI);
			CONT_DEFEFFECT const *pkDefEffect = NULL;
			g_kTblDataMgr.GetContDef(pkDefEffect);
			if(pkUseDeco)
			{
				XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkUseDeco->GetControl(L"LIST_DECO_LIST"));
				if(!pkListWnd)
				{
					return false;
				}
				pkListWnd->ClearList();

				typedef struct tagHomeBuff
				{
					int iItemNo;
					int iType;
					int iLv;
					std::wstring strName;
				}SHomeBuff;
				typedef std::map<int,SHomeBuff> CONT_HOMEBUFF;
				CONT_HOMEBUFF contBuff; // 이펙트타입, 레벨

				while(kContHomeItem.end() != Item_itor)
				{
					int const& iItemNo = (*Item_itor);
					CItemDef const* pkDef = kItemDefMgr.GetDef(iItemNo);
					int const iEffectNo = pkDef->GetAbil(AT_EFFECTNUM1);
					CONT_DEFEFFECT::const_iterator Eff_itor = pkDefEffect->find(iEffectNo);
					if(pkDefEffect->end() != Eff_itor)
					{
						CONT_DEFEFFECT::mapped_type const& kElement = Eff_itor->second;
						wchar_t const* pcEffName=NULL;
						GetDefString(kElement.iName, pcEffName);
						bool bInsert = true;
						SHomeBuff kBuff;
						kBuff.iItemNo = iItemNo;
						kBuff.iLv = kElement.iName % 100;
						kBuff.iType = kElement.iName - kBuff.iLv;
						kBuff.strName = pcEffName;

						std::pair<CONT_HOMEBUFF::iterator, bool> Result = contBuff.insert(std::make_pair(kBuff.iType, kBuff));
						if( false == Result.second )
						{
							if( Result.first->second.iLv < kBuff.iLv )
							{
								Result.first->second = kBuff;
							}
						}
					}
					++Item_itor;
				}

				for( CONT_HOMEBUFF::iterator itr = contBuff.begin(); itr != contBuff.end(); ++itr )
				{
					SHomeBuff const& homeBuff = itr->second;

					XUI::SListItem* pkListItem = pkListWnd->AddItem(_T(""));
					if( pkListItem
						&&	pkListItem->m_pWnd )
					{//UI에 넣어주고
						XUI::CXUI_Wnd* pWnd = pkListItem->m_pWnd;
						XUI::CXUI_Wnd* pkEffName = pWnd->GetControl(L"FRM_BUFF_NAME");
						if(pkEffName)
						{
							pkEffName->Text(homeBuff.strName.c_str());
						}

						// 아이템 위치를 ListItem UI에 저장한다
						SItemPos kPos;
						pkHomeInv->GetFirstItem(IT_HOME, homeBuff.iItemNo, kPos);
						pWnd->SetCustomData(&kPos, sizeof(kPos));
					}
				}
			}
		}
		return true;
	}

	bool SendMyHomeUseDeco_Packet(lwUIWnd kWnd)
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if(kWnd.IsNil())
		{
			return false;
		}
		XUI::CXUI_Wnd* pkUseDeco = kWnd.GetSelf();
		XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkUseDeco->GetControl(L"LIST_DECO_LIST"));
		if(!pkListWnd)
		{
			return false;
		}
		int iUseCnt = 0;
		XUI::SListItem* pkListItem = pkListWnd->FirstItem();
		while(pkListItem)
		{
			XUI::CXUI_Wnd* pWnd = pkListItem->m_pWnd;
			XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(L"CBTN_CHECK"));
			if(true == pkChkBtn->Check())
			{// 아이템 버프를 사요 요청을 보내고, 체크된 버튼은 해제 시킴
				pkChkBtn->Check(false);
				SItemPos kPos;
				pWnd->GetCustomData(&kPos, sizeof(kPos));
				BM::Stream kPacket(PT_C_M_REQ_HOME_USE_ITEM_EFFECT);
				kPacket.Push(pkHomeUnit->GetID());
				kPacket.Push(kPos);
				NETWORK_SEND(kPacket);
				++iUseCnt;
			}
			pkListItem = pkListWnd->NextItem(pkListItem);
		}
		// 사용한 버프가 있다면 연출을 사용
		lua_tinker::call<void, int>("MyHome_DecoBuf_Particle", iUseCnt);
		return true;
	}

	bool OnEventProcedure(lwHomeUIUtil::E_HOME_EVT const& eEvent, SMYHOME const& kHome)
	{
		lwHomeRenew::OnMyhomeBuffDisplay();

		switch(eEvent)
		{
		case lwHomeUIUtil::EEVT_NONE:
			{
			}break;
		case lwHomeUIUtil::EEVT_CALL_PAY_TAX_UI:
			{// 세금 납부 UI 호출
				return OnCallUIHomePayTax(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_AUCTION_INFO:
			{// 집 정보 보기
				return OnCallUIHomeAuctionInfo(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_AUCTION_REG:
			{
				if( !g_kPilotMan.IsMyPlayer( kHome.kOwnerGuid ) )
				{//내집이 아닌데?
					return false;
				}
				if( kHome.bAuctionState == MAS_IS_BLOCK)
				{//블럭당했는데 뭔 경매등록이냐?
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201520, true);
					return false;
				}
				if( kHome.bAuctionState == MAS_IS_BIDDING )
				{//이미 등록 했는데?
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201505, true);
					return false;
				}

				OnCallHomeAuctionRegist();
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_OPTION_UI:
			{// 홈 공개 설정
				return OnCallUIHomeOption(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_USEDECO_UI:
			{// 데코레이션 사용(버프 받기)
				return OnCallUIUseDecoUI(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_UI:
			{// 아르바이트 설정 UI
				return OnClickMyHomeSideJobOnMaidResive(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_INFO:
			{// 마이홈 정보 저장
				SetMyHomeInfoEnterHome(kHome);
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOUL_GOLD:
			{// 아르바이트 소울 돈
				SetMyHomeInfoEnterHome(kHome);
				ExcuteMyHomeSideJobSoulGold();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOUL_COUNT:
			{// 아르바이트 소울 갯수
				SetMyHomeInfoEnterHome(kHome);
				ExcuteMyHomeSideJobSoulCount();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_GOLD:
			{// 아르바이트 소켓 돈
				SetMyHomeInfoEnterHome(kHome);
				ExcuteMyHomeSideJobSocketGold();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_RATE:
			{// 아르바이트 소켓 비율
				SetMyHomeInfoEnterHome(kHome);
				ExcuteMyHomeSideJobSocketRate();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_RATE:
			{// 아르바이트 인첸트 비율
				SetMyHomeInfoEnterHome(kHome);
				ExcuteMyHomeSideJobEnchantRate();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_GOLD:
			{// 아르바이트 인첸트 비율
				SetMyHomeInfoEnterHome(kHome);
				ExcuteEnchantService();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_REPAIR:
			{// 아르바이트 인첸트 비율
				SetMyHomeInfoEnterHome(kHome);
				ExcuteRepairService();
				return true;
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_BUFF:
			{// 마이홈 버프
				return lwHomeRenew::OnCallMyhomeVisitComment(kHome);
			}break;
		case lwHomeUIUtil::EEVT_CALL_MYHOME_APT_RELIVE:
			{// 마이홈 부활
				return lwHomeRenew::OnCallMyhomeAptRelive(kHome);
			}break;
		default:
			{
				return false;
			}break;
		}
		return true;
	}

	void OnClickMyHomeSideJobOnMaidSetDay(XUI::CXUI_Wnd* pkParentWnd)
	{
		XUI::CXUI_Wnd* pkBody = pkParentWnd->GetControl(_T("SFRM_BODY_BG_SD"));
		if( pkBody )
		{
			int iDay[] = {0,1,3,7,15,30};
			for( int i=1; i<=5; ++i )
			{
				BM::vstring kRadio("CBTN_RADIO");
				kRadio+=i;

				XUI::CXUI_Wnd* pkRadio = pkBody->GetControl(kRadio);
				if( pkRadio )
				{
					XUI::CXUI_Wnd* pkText = pkRadio->GetControl(_T("FRM_TEXT"));
					if( pkText )
					{
						BM::vstring kText(TTW(201608)); 
						kText.Replace( L"#MARK1#", iDay[i] );
						pkText->Text(kText);
					}
				}
			}
		}
	}

	void OnClickMyHomeSideJobOnMaidSetComment(XUI::CXUI_Wnd* pkParentWnd)
	{
		XUI::CXUI_Wnd* pkComment = pkParentWnd->GetControl(_T("SFRM_COMMENT"));
		if( pkComment )
		{
			CONT_DEFSIDEJOBRATE const *pkContDef = NULL;
			g_kTblDataMgr.GetContDef(pkContDef);
			if( pkContDef )
			{
				CONT_DEFSIDEJOBRATE::const_iterator rate_iter = pkContDef->find(MSJ_REPAIR);
				if(rate_iter != pkContDef->end())
				{
					BM::vstring kText(TTW(201601));
					int iPercent = rate_iter->second.iPayRate / 100;
					kText.Replace( L"#MARK1#", iPercent );
					pkComment->Text(kText);
				}
			}
		}
	}

	bool OnClickMyHomeSideJobOnMaid()
	{// 메이드에서 아르바이트 설정 눌렀을때
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return false;
		}

		int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
		if( MAS_NOT_BIDDING != iState )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
			return false;
		}

		OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_UI);

		return true;
	}

	bool OnClickMyHomeSideJobOnMaidResiveUnderComment(XUI::CXUI_Wnd* pkParentWnd, SMYHOME const& kHome, eMyHomeSideJob kSideJob)
	{
		XUI::CXUI_Wnd* pkComment = pkParentWnd->GetControl(_T("SFRM_BODY_BG_SD"));
		if( pkComment )
		{
			float fPercent = 0;

			CONT_DEFSIDEJOBRATE const *pkContDef = NULL;
			g_kTblDataMgr.GetContDef(pkContDef);
			if( pkContDef )
			{
				CONT_DEFSIDEJOBRATE::const_iterator rate_iter = pkContDef->find(kSideJob);
				if(rate_iter != pkContDef->end())
				{
					fPercent = static_cast<float>(kHome.iEquipItemCount) * (static_cast<float>(rate_iter->second.iCostRate) / 100.0f);
				}
			}

			//CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(MSJ_REPAIR);
			//if( itr != kHome.kContSideJob.end() )
			//{
			//	fPercent = static_cast<float>(itr->second.iJobRate) / 100.0f;
			//}
			BM::vstring kText(TTW(201609));
			kText.Replace( L"#MARK1#", kHome.iEquipItemCount );
			kText.Replace( L"#MARK2#", BM::vstring(fPercent, L"%.1f") );
			pkComment->Text(kText);
		}

		return true;
	}

	bool OnClickMyHomeSideJobOnMaidResive(SMYHOME const& kHome)
	{
		SetMyHomeInfoEnterHome(kHome);
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(WSTR_HOME_SIDE_JOB_UI);
		if( pkWnd )
		{
			OnClickMyHomeSideJobOnMaidSetComment(pkWnd);// 타이틀 밑에 나오는 설명문
			OnClickMyHomeSideJobOnMaidSetDay(pkWnd);// 1일 ~ 30일 에 숫자 채워 넣기
			XUI::CXUI_Wnd* pkBody = pkWnd->GetControl(_T("SFRM_BODY_BG_SD"));
			if( pkBody )
			{
				OnClickMyHomeSideJobPeriodRadioButton( pkBody, 1);// 1일이 선택되게 하기
			}
			//OnClickMyHomeSideJobOnMaidResiveUnderComment(pkWnd, kHome, MSJ_REPAIR);
		}
		return true;
	}

	bool OnClickMyHomeSideJobPeriodRadioButton(lwUIWnd kParentWnd, int iPos)
	{
		if(!g_pkWorld)
		{
			return false;
		}
		XUI::CXUI_Wnd* pkParentWnd = kParentWnd.GetSelf();
		if( !pkParentWnd)
		{
			return false;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}

		int iGrade = pkHome->GetGrade();
		int iGradeEnable[] = {0, 1, 3, 4, 5 };
		int iView = iGradeEnable[iGrade];
		for( int i=1; i<=5; ++i )
		{
			BM::vstring kRadio("CBTN_RADIO");
			kRadio+=i;

			XUI::CXUI_Wnd* pkRadio = pkParentWnd->GetControl(kRadio);
			if( pkRadio )
			{
				XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkRadio);
				if( pkChkBtn )
				{
					if( i > iView )
					{
						pkChkBtn->Visible(false);
						//pkChkBtn->Disable(true);
					}
					else
					{
						pkChkBtn->Visible(true);
						//pkChkBtn->Disable(false);
						if( i != iPos )
						{
							pkChkBtn->Check(false);
							pkChkBtn->ClickLock(false);
						}
					}
				}
			}
		}

		return true;
	}

	bool IsCheckStateSelf(XUI::CXUI_Wnd* pkSelf)
	{
		if( pkSelf )
		{
			XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkSelf);
			if( pkChkBtn )
			{
				return pkChkBtn->Check();
			}
		}

		return false;
	}

	int OnClickMyHomeSideJobGetRadioPos(XUI::CXUI_Wnd* pkParent)
	{
		if( !pkParent )
		{
			return 1;
		}

		for( int i=1; i<=5; ++i )
		{
			BM::vstring kRadio("CBTN_RADIO");
			kRadio+=i;

			XUI::CXUI_Wnd* pkRadio = pkParent->GetControl(kRadio);
			if( pkRadio )
			{
				XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkRadio);
				if( pkChkBtn )
				{
					if( pkChkBtn->Check() )
					{
						return i;
					}
				}
			}
		}

		return 1;
	}

	bool OnClickMyHomeSideJobOK(lwUIWnd kParentWnd)
	{
		XUI::CXUI_Wnd* pkParentWnd = kParentWnd.GetSelf();
		if( !pkParentWnd )
		{
			return false;
		}

		// 아르바이트 리스트 선택한것이 있는지 확인
		bool bRepair = false;
		bool bEnchant = false;
		XUI::CXUI_Wnd* pkBody = pkParentWnd->GetControl(_T("SFRM_BODY_BG_SD"));
		if( !pkBody )
		{
			return false;
		}
		XUI::CXUI_Wnd* pkRepair = pkBody->GetControl(_T("CBTN_REPAIR"));
		if( pkRepair )
		{
			if( IsCheckStateSelf(pkRepair) )
			{
				bRepair = true;
			}
		}
		XUI::CXUI_Wnd* pkEnchant= pkBody->GetControl(_T("CBTN_ENCHANT"));
		if( pkEnchant )
		{
			if( IsCheckStateSelf(pkEnchant) )
			{
				bEnchant = true;
			}
		}
		if( !(bRepair || bEnchant) )
		{
			// 리스트 선택하라는 메세지 박스
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201648, true);
			return true;
		}

		int iRadioPos = 1;
		iRadioPos = OnClickMyHomeSideJobGetRadioPos(pkBody);

		int iDay[] = {0,1,3,7,15,30};
		int iSecond = iDay[iRadioPos] * 24 * 60 * 60;

		if( bRepair )
		{
			BM::Stream kPacket(PT_C_M_REQ_START_SIDE_JOB);
			kPacket.Push(MSJ_REPAIR);
			kPacket.Push(iSecond);
			NETWORK_SEND(kPacket);
		}
		if( bEnchant )
		{
			BM::Stream kPacket(PT_C_M_REQ_START_SIDE_JOB);
			kPacket.Push(MSJ_ENCHANT);
			kPacket.Push(iSecond);
			NETWORK_SEND(kPacket);
		}

		return true;
	}

	bool OnClickMyHomeSideJobSettingCancel(lwUIWnd kParentWnd)
	{
		XUI::CXUI_Wnd* pkParentWnd = kParentWnd.GetSelf();
		if( !pkParentWnd )
		{
			return false;
		}

		// 아르바이트 리스트 선택한것이 있는지 확인
		bool bRepair = false;
		bool bEnchant = false;
		XUI::CXUI_Wnd* pkBody = pkParentWnd->GetControl(_T("SFRM_BODY_BG_SD"));
		if( !pkBody )
		{
			return false;
		}
		XUI::CXUI_Wnd* pkRepair = pkBody->GetControl(_T("CBTN_REPAIR"));
		if( pkRepair )
		{
			if( IsCheckStateSelf(pkRepair) )
			{
				bRepair = true;
			}
		}
		XUI::CXUI_Wnd* pkEnchant= pkBody->GetControl(_T("CBTN_ENCHANT"));
		if( pkEnchant )
		{
			if( IsCheckStateSelf(pkEnchant) )
			{
				bEnchant = true;
			}
		}
		if( !(bRepair || bEnchant) )
		{
			// 리스트 선택하라는 메세지 박스
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201648, true);
			return true;
		}

		if( bRepair )
		{
			if( !IsRepairService() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201653, true);
				return true;
			}
		}

		if( bEnchant )
		{
			if( !IsEnchantService() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201653, true);
				return false;
			}
		}

		if( bRepair )
		{
			BM::Stream kPacket(PT_C_M_REQ_CANCEL_SIDE_JOB);
			kPacket.Push(MSJ_REPAIR);
			NETWORK_SEND(kPacket);
		}
		if( bEnchant )
		{
			BM::Stream kPacket(PT_C_M_REQ_CANCEL_SIDE_JOB);
			kPacket.Push(MSJ_ENCHANT);
			NETWORK_SEND(kPacket);
		}

		return true;
	}

	void SetMyHomeInfoEnterHome(SMYHOME const& kHome)
	{
		g_kHomeTownMgr.SetMyHomeInfo(kHome);
	}

	void SendMyHomeInfoRequest()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_INFO);
		}
	}

	bool IsRepairService()
	{
		SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(MSJ_REPAIR);
		if( itr == kHome.kContSideJob.end() )
		{
			return false;
		}

		return true;
		//PgHome* pkHome = g_pkWorld->GetHome();
		//if( !pkHome )
		//{
		//	return false;
		//}
		//PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		//if( !pkHomeUnit )
		//{
		//	return false;
		//}

		//int iSideJob = pkHomeUnit->GetAbil(AT_HOME_SIDEJOB);

		//return ((iSideJob & MSJ_REPAIR) != 0);
	}

	bool IsEnchantService()
	{
		SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(MSJ_ENCHANT);
		if( itr == kHome.kContSideJob.end() )
		{
			return false;
		}

		if( MSJRT_GOLD != itr->second.kJobRateType )
		{
			return false;
		}

		return true;

		//PgHome* pkHome = g_pkWorld->GetHome();
		//if( !pkHome )
		//{
		//	return false;
		//}
		//PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		//if( !pkHomeUnit )
		//{
		//	return false;
		//}

		//int iSideJob = pkHomeUnit->GetAbil(AT_HOME_SIDEJOB);

		//return ((iSideJob & MSJ_ENCHANT) != 0);
	}

	bool OnCheckSideJobList(lwUIWnd lwSelf, int iCheck)
	{
		XUI::CXUI_Wnd* pkParent = lwSelf.GetSelf();
		if( !pkParent )
		{
			return false;
		}

		OnClickMyHomeSideJobOnMaidResiveUnderComment(pkParent, g_kHomeTownMgr.GetMyHomeInfo(), static_cast<eMyHomeSideJob>(iCheck));
		return true;
	}

	float GetMyHomeSideJobDiscountRate(eMyHomeSideJob kSideJob, eMyHomeSideJobRateType kRateType)
	{
		SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(kSideJob);
		if( itr == kHome.kContSideJob.end() )
		{
			return 0.0f;
		}

		if( itr->second.kJobRateType != kRateType )
		{
			return 0.0f;
		}
		return static_cast<float>(itr->second.iJobRate) / 100.0f * static_cast<float>(kHome.iEquipItemCount);
	}

	bool OnClickRepairService()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return false;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_REPAIR);
		}

		return true;
	}

	bool ExcuteRepairService()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_REPAIR, MSJRT_GOLD);

		BM::vstring kText(TTW(201614)); 
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_REPAIR, NULL);

		return true;
	}

	bool OpenMyHomeRepairService()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				g_kItemRepair.NpcGuid( pkHomeUnit->GetID() );
				std::wstring const kRepairUIName(_T("SFRM_REPAIR"));
				CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kRepairUIName );
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
					return true;
				}
			}
		}

		return false;
	}

	bool OnClickEnchantService()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return false;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_GOLD);
		}

		return true;
	}

	bool ExcuteEnchantService()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_ENCHANT, MSJRT_GOLD);

		BM::vstring kText(TTW(201616)); 
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_ENCHANT, NULL);

		return true;
	}

	bool OpenMyHomeEnchantService()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kItemPlusUIName(_T("SFRM_ITEM_PLUS_UPGRADE"));
				g_kItemPlusUpgradeMgr.NpcGuid( pkHomeUnit->GetID() );
				CXUI_Wnd* pkTopWnd = XUIMgr.Call(kItemPlusUIName);
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
				}
			}
		}

		return false;
	}

	bool OnCallMyHomeSideJobRemainText(lwUIWnd lwSelf, int iSelect)
	{
		XUI::CXUI_Wnd* pkSelf = lwSelf.GetSelf();
		if( !pkSelf )
		{
			return false;
		}

		pkSelf->Text(L"");
		BM::PgPackedTime kEndTime;
		if( 1 == iSelect )//MSJ_REPAIR
		{
			if( !IsRepairService() )
			{
				return true;
			}

			SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
			CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(MSJ_REPAIR);
			if( itr == kHome.kContSideJob.end() )
			{
				return true;
			}
			kEndTime = itr->second.kEndTime;
		}
		else if( 2 == iSelect )//MSJ_ENCHANT
		{
			if( !IsEnchantService() )
			{
				return true;
			}

			SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
			CONT_MYHOME_SIDE_JOB::const_iterator itr = kHome.kContSideJob.find(MSJ_ENCHANT);
			if( itr == kHome.kContSideJob.end() )
			{
				return true;
			}
			kEndTime = itr->second.kEndTime;
		}

		//BM::PgPackedTime		kEndTime;
		int iDay = kEndTime.Day();
		int iHour = kEndTime.Hour();
		int iMin = kEndTime.Min();
		BM::vstring kText(TTW(201652));
		kText.Replace( L"#MARK1#", iDay);
		kText.Replace( L"#MARK2#", iHour);
		kText.Replace( L"#MARK3#", iMin);
		pkSelf->Text(kText);

		return true;
	}

	void OnClickMyHomeSideJobNpcMove(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if (pkHome)
		{
			pkHome->SetMaidNpcMoving(kGuid());
		}
	}

	void OnClickMyHomeSideJobNpcRemove(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			pkHome->SendHomeMaidUnEquip(kGuid());
		}
	}

	void OnClickMyHomeSideJobSoulGold(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return;
			}
			if( !pkHome->IsUseAbleTimeMaidNPC(kGuid()) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOUL_GOLD);
		}
	}

	void OnClickMyHomeSideJobSoulCount(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return;
			}
			if( !pkHome->IsUseAbleTimeMaidNPC(kGuid()) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOUL_COUNT);
		}
	}

	void OnClickMyHomeSideJobSocketGold(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return;
			}
			if( !pkHome->IsUseAbleTimeMaidNPC(kGuid()) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_GOLD);
		}
	}

	void OnClickMyHomeSideJobSocketRate(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return;
			}
			if( !pkHome->IsUseAbleTimeMaidNPC(kGuid()) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_RATE);
		}
	}

	void OnClickMyHomeSideJobEnchantRate(lwGUID kGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
			if( MAS_NOT_BIDDING != iState )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
				return;
			}
			if( !pkHome->IsUseAbleTimeMaidNPC(kGuid()) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_RATE);
		}
	}

	void ExcuteMyHomeSideJobSoulGold()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_SOULCRAFT, MSJRT_GOLD);
		BM::vstring kText(TTW(201619)); 
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		kText += '\n';
		kText += TTW(201620);
		kText += '\n';
		kText += TTW(201621);
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_SOUL_GOLD, NULL);
	}

	void ExcuteMyHomeSideJobSoulCount()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_SOULCRAFT, MSJRT_SOUL);
		BM::vstring kText(TTW(201622));
		kText += '\n';
		kText += TTW(201620);
		kText += '\n';
		kText += TTW(201621);
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_SOUL_COUNT, NULL);
	}

	void ExcuteMyHomeSideJobSocketGold()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_SOCKET, MSJRT_GOLD);
		BM::vstring kText(TTW(201624));
		kText += '\n';
		kText += TTW(201625);
		kText += '\n';
		kText += TTW(201626);
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_SOCKET_GOLD, NULL);
	}

	void ExcuteMyHomeSideJobSocketRate()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_SOCKET, MSJRT_RATE);
		BM::vstring kText(TTW(201627));
		kText += '\n';
		kText += TTW(201625);
		kText += '\n';
		kText += TTW(201626);
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_SOCKET_RATE, NULL);
	}

	void ExcuteMyHomeSideJobEnchantRate()
	{
		float fPercent = GetMyHomeSideJobDiscountRate(MSJ_ENCHANT, MSJRT_RATE);
		BM::vstring kText(TTW(201629));
		kText += '\n';
		kText += TTW(201630);
		kText.Replace( L"#MARK1#", BM::vstring(fPercent, L"%.1f") );
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(), true, MBT_COMMON_YESNO_MYHOM_ENCHANT_RATE, NULL);
	}


	void OpenMyHomeSideJobSoulGold()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kRarityUpgradeSelectUIName(_T("ITEM_RARITY_UPGRADE_SELECT"));
				CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kRarityUpgradeSelectUIName );
				if( pkTopWnd )
				{
					BM::Stream kCustomData;
					kCustomData.Push( pkHomeUnit->GetID() );
					pkTopWnd->SetCustomData( kCustomData.Data() );
				}
			}
		}
	}

	void OpenMyHomeSideJobSoulCount()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kRarityUpgradeSelectUIName(_T("ITEM_RARITY_UPGRADE_SELECT"));
				CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kRarityUpgradeSelectUIName );
				if( pkTopWnd )
				{
					BM::Stream kCustomData;
					kCustomData.Push( pkHomeUnit->GetID() );
					pkTopWnd->SetCustomData( kCustomData.Data() );
				}
			}
		}
	}

	void OpenMyHomeSideJobSocketGold()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kSocketSystemUIName(_T("SFRM_SOCKET_SYSTEM"));
				g_kSocketSystemMgr.NpcGuid( pkHomeUnit->GetID() );
				CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kSocketSystemUIName );
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
				}
			}
		}
	}

	void OpenMyHomeSideJobSocketRate()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kSocketSystemUIName(_T("SFRM_SOCKET_SYSTEM"));
				g_kSocketSystemMgr.NpcGuid( pkHomeUnit->GetID() );
				CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kSocketSystemUIName );
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
				}
			}
		}
	}

	void OpenMyHomeSideJobEnchantRate()
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kItemPlusUIName(_T("SFRM_ITEM_PLUS_UPGRADE"));
				g_kItemPlusUpgradeMgr.NpcGuid( pkHomeUnit->GetID() );
				CXUI_Wnd* pkTopWnd = XUIMgr.Call(kItemPlusUIName);
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
				}
			}
		}
	}

	// 아르바이트 게시판 오픈
	void OnSendSideJobBoardList()
	{
		BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_INFO);
		kPacket.Push((BYTE)PgHomeUIUtil::ETLRT_OPEN_SIDE_JOB_BOARD);
		NETWORK_SEND(kPacket);
	}

	void SetSideJobBoardPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeTownMgr.GetSideJobBoardPage());
	}

	PgHomeUIUtil::E_SIDE_JOB_SORT_STATE GetSideJobType(eMyHomeSideJob kSideJob, eMyHomeSideJobRateType kType)
	{
		PgHomeUIUtil::E_SIDE_JOB_SORT_STATE kRtn = PgHomeUIUtil::ESJSS_ALL_ITEM;	
		switch( kSideJob )
		{
		case MSJ_REPAIR:
			{
				if( MSJRT_GOLD == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_REPAIR;
				}
			}break;
		case MSJ_ENCHANT:
			{
				if( MSJRT_GOLD == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_ENCHANT_GOLD;
				}
				else if( MSJRT_RATE == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_ENCHANT_RATE;
				}
			}break;
		case MSJ_SOULCRAFT:
			{
				if( MSJRT_GOLD == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_SOUL_GOLD;
				}
				else if( MSJRT_SOUL == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_SOUL_COUNT;
				}
			}break;
		case MSJ_SOCKET:
			{
				if( MSJRT_GOLD == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_SOCKET_GOLD;
				}
				else if( MSJRT_RATE == kType )
				{
					kRtn = PgHomeUIUtil::ESJSS_SOCKET_RATE;
				}
			}break;
		default:
			{
			}break;
		}

		return kRtn;
	}

	void SetSideJobText(XUI::CXUI_Wnd* pSelf, PgHomeUIUtil::E_SIDE_JOB_SORT_STATE kSortState)
	{
		if( pSelf )
		{
			switch( kSortState )
			{
			case PgHomeUIUtil::ESJSS_ALL_ITEM:
				{
					pSelf->Text(TTW(201032));
				}break;
			case PgHomeUIUtil::ESJSS_REPAIR:
				{
					pSelf->Text(TTW(201638));
				}break;
			case PgHomeUIUtil::ESJSS_ENCHANT_GOLD:
				{
					pSelf->Text(TTW(201639));
				}break;
			case PgHomeUIUtil::ESJSS_ENCHANT_RATE:
				{
					pSelf->Text(TTW(201640));
				}break;
			case PgHomeUIUtil::ESJSS_SOUL_GOLD:
				{
					pSelf->Text(TTW(201641));
				}break;
			case PgHomeUIUtil::ESJSS_SOUL_COUNT:
				{
					pSelf->Text(TTW(201642));
				}break;
			case PgHomeUIUtil::ESJSS_SOCKET_GOLD:
				{
					pSelf->Text(TTW(201643));
				}break;
			case PgHomeUIUtil::ESJSS_SOCKET_RATE:
				{
					pSelf->Text(TTW(201644));
				}break;
			default:
				{
					pSelf->Text(L"");
				}break;
			}
		}
	}
	//아르바이트 집 정보 슬롯
	void SetSideJobBoardHomeSlot(XUI::CXUI_Wnd* pSlot, SHOMESIDEJOB const& kSideJob)
	{
		if( !pSlot )
		{
			return;
		}

		SMYHOME kHome = kSideJob.kHome;

		if( 0 == kHome.siStreetNo && 0 == kHome.iHouseNo )
		{
			pSlot->Visible(false);
		}
		else
		{
			pSlot->Visible(true);
		}

		pSlot->OwnerGuid(kHome.kOwnerGuid);

		SHOMEADDR kPoint(kHome.siStreetNo, kHome.iHouseNo);
		pSlot->SetCustomData(&kPoint, sizeof(kPoint));

		wchar_t szTemp[MAX_PATH] = {0,};

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_TOWN");
		if( pTemp )
		{
			std::wstring kLocStr;
			kLocStr = lwHomeUIUtil::GetTownName(kHome.siStreetNo);
			pTemp->Text(kLocStr);
		}

		pTemp = pSlot->GetControl(L"SFRM_ADDRESS");
		if( pTemp )
		{
			std::wstring kLocStr;
			swprintf_s(szTemp, TTW(201004).c_str(), kHome.iHouseNo);
			kLocStr = szTemp;
			pTemp->Text(kLocStr);
		}

		PgHomeUIUtil::E_SIDE_JOB_SORT_STATE kSortState = GetSideJobType(kSideJob.kSideJob, kSideJob.kSideJobType);
		pTemp = pSlot->GetControl(L"SFRM_SIDE_JOB");
		SetSideJobText(pTemp, kSortState);

		pTemp = pSlot->GetControl(L"SFRM_DISCOUNT_RATE");
		if( pTemp )
		{
			BM::vstring kLocStr(kSideJob.fJobRate, L"%.1f");
			kLocStr += '%';
			pTemp->Text(kLocStr);
		}

		pTemp = pSlot->GetControl(L"BTN_MOVE");
		if( pTemp )
		{
			bool bIsDisable = true;
			if((MEV_ALL & kHome.bEnableVisitBit))
			{
				bIsDisable = false;
			}

			if( (kHome.bEnableVisitBit & MEV_ALL) == MEV_ALL )
			{
				bIsDisable = false;
			}

			if( (kHome.bEnableVisitBit & MEV_COUPLE) == MEV_COUPLE )
			{
				SCouple kMyCouple = g_kCoupleMgr.GetMyInfo();

				if( kMyCouple.CoupleGuid() != BM::GUID::NullData()
				&& kMyCouple.CoupleGuid() == kHome.kOwnerGuid )
				{
					bIsDisable = false;
				}
			}

			if( (kHome.bEnableVisitBit & MEV_GUILD) == MEV_GUILD )
			{
				SGuildMemberInfo kTemp;
				if( g_kGuildMgr.IamHaveGuild()
				&& g_kGuildMgr.GetMemberByGuid( kHome.kOwnerGuid, kTemp ) )
				{
					bIsDisable = false;
				}
			}

			if( (kHome.bEnableVisitBit & MEV_FRIEND) == MEV_FRIEND )
			{
				SFriendItem kTemp;
				if( g_kFriendMgr.Friend_Find_ByGuid( kHome.kOwnerGuid, kTemp ) )
				{
					bIsDisable = false;
				}
			}

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(bIsDisable);
				if( bIsDisable )
				{
					pButton->Text(L"");
				}
				else
				{
					pButton->Text(TTW(201602));
				}
			}
		}
	}

	void SetSideJobBoardList(XUI::CXUI_Wnd* pMainUI)
	{
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM_SLOT"));
		if( !pBuild )
		{
			return;
		}

		CONT_SIDEJOB kSideJobList;
		g_kHomeTownMgr.GetSideJobBoardDisplayItem(kSideJobList);

		CONT_SIDEJOB::const_iterator sidejob_itr = kSideJobList.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pHomeSlot = pMainUI->GetControl(vStr);
			if( pHomeSlot )
			{
				if( sidejob_itr != kSideJobList.end() )
				{
					SHOMESIDEJOB const& kSideJobInfo = *sidejob_itr;
					SetSideJobBoardHomeSlot(pHomeSlot, kSideJobInfo);
					++sidejob_itr;
				}
				else
				{
					SetSideJobBoardHomeSlot(pHomeSlot, SHOMESIDEJOB());
				}
			}
		}
	}

	void OnCallSideJobBoard()
	{
		std::wstring const strBoard(_T("SFRM_SIDE_JOB_BOARD"));
		XUI::CXUI_Wnd* pBoardUI = XUIMgr.Get(strBoard);
		if( !pBoardUI || pBoardUI->IsClosed() )
		{
			pBoardUI = XUIMgr.Call(strBoard);
			if( !pBoardUI )
			{
				return;
			}
		}

		RegistUIAction(pBoardUI, L"CloseUI");

		XUI::CXUI_Wnd* pWnd = pBoardUI->GetControl(L"SFRM_TOWN_ADDR");
		if( pWnd )
		{
			short wTownNo = 0;
			pWnd->Text(TTW(201002));
			pWnd->SetCustomData(&wTownNo, sizeof(wTownNo));
		}

		pWnd = pBoardUI->GetControl(L"SFRM_ALIGN_CATE");
		if( pWnd )
		{
			int iSortType = PgHomeUIUtil::ESJSS_ALL_ITEM;
			pWnd->Text(TTW(201032));
			pWnd->SetCustomData(&iSortType, sizeof(iSortType));
		}

		SetSideJobBoardList(pBoardUI);
		SetSideJobBoardPageControl(pBoardUI->GetControl(L"FRM_PAGE"));
	}

	//아르바이트 타운 주소 드롭 오픈
	void OnClickSideJobBoardTownNumber(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_SIDE_JOB_BOARD_TOWN_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;

		XUI::SListItem* pAllViewItem = pList->AddItem(L"");
		if( pAllViewItem && pAllViewItem->m_pWnd )
		{
			short TownNo = 0;
			pAllViewItem->m_pWnd->SetCustomData(&TownNo, sizeof(TownNo));
			pAllViewItem->m_pWnd->Text(TTW(201002));
			ItemHeight = pAllViewItem->m_pWnd->Size().y;
		}

		PgHomeTown::CONT_TOWNNO const& kTownNoList = g_kHomeTownMgr.TownNoList();
		PgHomeTown::CONT_TOWNNO::const_iterator townNo_itor = kTownNoList.begin();
		while( townNo_itor != kTownNoList.end() )
		{
			PgHomeTown::CONT_TOWNNO::value_type const& kTownNo = (*townNo_itor);

			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				pListItem->m_pWnd->SetCustomData(&kTownNo, sizeof(kTownNo));
				//swprintf_s(szTemp, MAX_PATH, TTW(201003).c_str(), kTownNo);
				pListItem->m_pWnd->Text(lwHomeUIUtil::GetTownName(kTownNo));
			}
			++townNo_itor;
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	//아르바이트 타운 선택
	void OnClickSideJobBoardTownItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		short iTownNo = 0;
		pSelf->GetCustomData(&iTownNo, sizeof(iTownNo));

		XUI::CXUI_Wnd* pHomeTownBoardUI = XUIMgr.Get(L"SFRM_SIDE_JOB_BOARD");
		if( pHomeTownBoardUI )
		{
			XUI::CXUI_Wnd* TargetUI = pHomeTownBoardUI->GetControl(L"SFRM_TOWN_ADDR");
			if( TargetUI )
			{
				TargetUI->Text(pSelf->Text());
			}
		}

		if( g_kHomeTownMgr.SetSideJobBoardDisplayTownNo(iTownNo) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_SIDE_JOB_BOARD");
			if( pMainUI )
			{
				SetSideJobBoardList(pMainUI);
				SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
			}
		}
	}

	//아르바이트 소트 드롭 오픈
	void OnClickSideJobBoardListViewType(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_SIDE_JOB_BOARD_SORT_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;
		for( int i = 0; i < PgHomeUIUtil::ESJSS_END; ++i )
		{
			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				SetSideJobText(pListItem->m_pWnd, static_cast<PgHomeUIUtil::E_SIDE_JOB_SORT_STATE>(i));
				pListItem->m_pWnd->SetCustomData(&i, sizeof(i));
				ItemHeight = pListItem->m_pWnd->Size().y;
			}
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	// 아르바이트 일부 보기 선택
	void OnClickSideJobBoardListViewTypeItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		int iViewState = 0;
		pSelf->GetCustomData(&iViewState, sizeof(iViewState));
		
		XUI::CXUI_Wnd* pHomeTownBoardUI = XUIMgr.Get(L"SFRM_SIDE_JOB_BOARD");
		if( pHomeTownBoardUI )
		{
			XUI::CXUI_Wnd* TargetUI = pHomeTownBoardUI->GetControl(L"SFRM_ALIGN_CATE");
			if( TargetUI )
			{
				TargetUI->Text(pSelf->Text());
			}
		}

		if( g_kHomeTownMgr.SetSideJobBoardDisplaySortType(iViewState) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_SIDE_JOB_BOARD");
			if( pMainUI )
			{
				SetSideJobBoardList(pMainUI);
				SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
			}
		}
	}

	void OnClickSideJobBoardBeginPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetSideJobBoardPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageBegin() )
		{
			return;
		}

		SetSideJobBoardList(pMainUI);
		SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickSideJobBoardJumpPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetSideJobBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrevJump() )
		{
			return;
		}

		SetSideJobBoardList(pMainUI);
		SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickSideJobBoardJumpNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetSideJobBoardPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNextJump() )
		{
			return;
		}

		SetSideJobBoardList(pMainUI);
		SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickSideJobBoardEndPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}
		
		PgPage& kPage = g_kHomeTownMgr.GetSideJobBoardPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageEnd() )
		{
			return;
		}

		SetSideJobBoardList(pMainUI);
		SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickSideJobBoardPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( kSelf.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pSelf->BuildIndex();

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		while( pMainUI->Parent() )
		{
			pMainUI = pMainUI->Parent();
		}

		PgPage& kPage = g_kHomeTownMgr.GetSideJobBoardPage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetSideJobBoardList(pMainUI);
		SetSideJobBoardPageControl(pMainUI->GetControl(L"FRM_PAGE"));
	}

	void OnClickSideJobBoardMoveHome(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pSlot = pSelf->Parent();
		if( !pSlot )
		{
			return;
		}

		SHOMEADDR kHomeAddr;
		pSlot->GetCustomData(&kHomeAddr, sizeof(kHomeAddr));

		if( kHomeAddr.StreetNo() == 0 || kHomeAddr.HouseNo() == 0 )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 99, true);
			return;
		}

		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( pkPC )
		{
			if( 50 > pkPC->GetAbil64(AT_MONEY) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201647, true);
				return;
			}
		}
		BM::vstring kText(TTW(201645)); 
		kText.Replace( L"#MARK1#", lwHomeUIUtil::GetTownName(kHomeAddr.StreetNo() ) );
		kText.Replace( L"#MARK2#", kHomeAddr.HouseNo() );
		kText.Replace( L"#MARK3#", GetMoneyString(g_iSideJobBoardMoveCost, true, false) );		
		kText += '\n';
		kText += TTW(201646);

		BM::Stream kPacket(PT_C_M_REQ_ENTER_SIDE_JOB);
		kPacket.Push(kHomeAddr.StreetNo());
		kPacket.Push(kHomeAddr.HouseNo());
		CallCommonMsgYesNoBox(kText, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void lwTest_AddTownBoardItem(short const streetno, int const houseno, bool bNotCallBoard)
	{
		SHOMEADDR kAddr(streetno,houseno);
		if( kAddr.IsNull() )
		{
			kAddr.StreetNo(BM::Rand_Range(SHRT_MAX,1));
			kAddr.HouseNo(BM::Rand_Range(SHRT_MAX,1));
		}
		g_kHomeTownMgr.Test_AddTownBoardItem(kAddr);
		!bNotCallBoard ? OnCallHomeTownBoard() : 0;
	}
}