#ifndef FREEDOM_DRAGONICA_CONTENTS_MYHOME_LWHOMEUI_SCRIPT_H
#define FREEDOM_DRAGONICA_CONTENTS_MYHOME_LWHOMEUI_SCRIPT_H

#include "PgHomeUI.h"

class PgPage;
namespace lwHomeUIUtil
{
	typedef enum
	{
		EHOME_PUBILC_OPTION			= 1,
		EFRIEND_PUBILC_OPTION		= 2,
		ECOUPLE_PUBLIC_OPTION		= 3,
		EGUILD_PEOPLE_PUBILC_OPTION = 4,
	}E_HOME_OPTION;
	
	typedef enum
	{// 0~ 255 까지만 사용할것
		EEVT_NONE					  = 0,
		EEVT_CALL_PAY_TAX_UI		  = 1,	//세금 납부 UI
		EEVT_CALL_MYHOME_AUCTION_INFO = 2,
		EEVT_CALL_MYHOME_AUCTION_REG  = 3,
		EEVT_CALL_MYHOME_OPTION_UI	  = 4,	//홈 공개 여부 UI
		EEVT_CALL_MYHOME_USEDECO_UI	  = 5,	//데코레이션 부가기능 사용 UI
		EEVT_CALL_MYHOME_SIDE_JOB_UI  = 6,	//아르바이트 설정 UI
		EEVT_CALL_MYHOME_INFO		  = 7,	//마이홈 정보를 저장한다.
		EEVT_CALL_MYHOME_SIDE_JOB_SOUL_GOLD		= 8,	//아르바이트 소울 돈
		EEVT_CALL_MYHOME_SIDE_JOB_SOUL_COUNT	= 9,	//아르바이트 소울 갯수
		EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_GOLD	= 10,	//아르바이트 소켓 돈
		EEVT_CALL_MYHOME_SIDE_JOB_SOCKET_RATE	= 11,	//아르바이트 소켓 비율
		EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_RATE	= 12,	//아르바이트 인첸트 비율
		EEVT_CALL_MYHOME_SIDE_JOB_ENCHANT_GOLD	= 13,	//아르바이트 인첸트 돈
		EEVT_CALL_MYHOME_SIDE_JOB_REPAIR		= 14,	//아르바이트 수리
		EEVT_CALL_MYHOME_BUFF		  = 15,	//마이홈 버프
		EEVT_CALL_MYHOME_APT_RELIVE	  = 16,	//마이홈 아파트 부활
	}E_HOME_EVT;

	typedef enum
	{
		ETCT_SEC	= 0x01 << 1,
		ETCT_MIN	= 0x01 << 2,
		ETCT_HOUR	= 0x01 << 3,
		ETCT_DAY	= 0x01 << 4,
		ETCT_MONTH	= 0x01 << 5,
		ETCT_YEAR	= 0x01 << 6,

		ETCT_DEF_TIME_COV = ETCT_DAY | ETCT_HOUR | ETCT_MIN,
	}E_TIME_CONV_OPTION;

	SYSTEMTIME const GetTimeGab(BM::PgPackedTime const& kTime);
	void CheckConvTimeStr(int const iOption, E_TIME_CONV_OPTION const eType, int const iValue, int const iTTID, std::wstring& kOutText);
	std::wstring const GetTimeGabString(BM::PgPackedTime const& kTime, int const iOption = ETCT_DEF_TIME_COV, bool const bNoGab = false);
	int const GetStateTTID(SMYHOME const& kHome);
	void CommonPageControl(XUI::CXUI_Wnd* pPageMainUI, PgPage const& kPage);
	std::wstring const GetTownName(short const iTownNo);
};

namespace lwHomeUI
{
	void RegisterWrapper(lua_State *pkState);
	//Packet
	bool RecvMyHome_Packet(WORD kPacketType, BM::Stream& rkPacket);
	bool CheckErrorMsg(HRESULT kResult, bool bShowMessageBox = true);

	//HomeTownBoard
	void OnSendTown_HomeList();
	void OnCallHomeBiddingRequest(lwUIWnd kRecver);
	void OnCallHomeDetailView(lwUIWnd kSelf);
	void OnClickHomeBiddingOK(lwUIWnd kParent);
	void OnClickTownBoardTownNumber(lwUIWnd kSelf);
	void OnClickTownBoardTownNumberItem(lwUIWnd kSelf);
	void OnClickTownBoardListViewType(lwUIWnd kSelf);
	void OnClickTownBoardListViewTypeItem(lwUIWnd kSelf);

	void OnClickTownBoardBeginPage(lwUIWnd kSelf);
	void OnClickTownBoardEndPage(lwUIWnd kSelf);
	void OnClickTownBoardJumpPrevPage(lwUIWnd kSelf);
	void OnClickTownBoardPrevPage(lwUIWnd kSelf);
	void OnClickTownBoardNextPage(lwUIWnd kSelf);
	void OnClickTownBoardJumpNextPage(lwUIWnd kSelf);
	void OnClickTownBoardPage(lwUIWnd kSelf);

	void OnCallHomeTownBoard();
	void SetHomeTownBoardList(XUI::CXUI_Wnd* pMainUI);
	void SetHomeTownBoardPageControl(XUI::CXUI_Wnd* pPageMainUI);
	void SetHomeTownBoardHomeSlot(XUI::CXUI_Wnd* pSlot, SMYHOME const& kHome);

	//MyHome
	void OnClickMyHomeVisitHistory(lwUIWnd kSelf);
	void OnClickMyHomeInvitation(lwUIWnd kParent);
	void OnClickOrtherHomeInvitation();
	void OnClickOrtherHomeInvitationToGO(lwUIWnd kSelf);
	void OnCallHomeInOwnerAndUserUI();
	void OnUpdateHomeVisitInfo(lwUIWnd UISelf);
	void OnClickVisitListItem(lwUIWnd kSelf);
	void OnClickVisitToFriendReg(lwUIWnd kSelf);
	void OnClickVisitListItemDelete(lwUIWnd kSelf);

	void OnClickInviteBeginPage(lwUIWnd kSelf);
	void OnClickInviteEndPage(lwUIWnd kSelf);
	void OnClickInviteJumpPrevPage(lwUIWnd kSelf);
	void OnClickInviteJumpNextPage(lwUIWnd kSelf);
	void OnClickInvitePage(lwUIWnd kSelf);

	bool IsUseCashShop();

	void OnCallMyHomeVisitHistory();
	void SetMyHomeVisitHistory(XUI::CXUI_Wnd* pMainUI);
	void SetMyHomeVisitHistorySlotClickInfo(XUI::CXUI_Wnd* pSlot, bool const bClick);
	void SetMyHomeVisitHistorySlot(XUI::CXUI_Wnd* pSlot, SVISITORINFO const& kVisitInfo);
	bool SortVisitDay(SVISITORINFO const &rkLeft, SVISITORINFO const &rkRight);

	void OnCallOrtherHomeInvitationUI();
	void SetOrtherHomeIvitationList(XUI::CXUI_Wnd* pMainUI);
	void SetOrtherHomeIvitationListSlot(XUI::CXUI_Wnd* pSlot, SINVITATIONCARD const& kInviteInfo);
	void SetOrtherHomeIvitationPageControl(XUI::CXUI_Wnd* pPageMainUI);

	//Npc Menu
	void OnClickHomeAuctionRegist();
	void OnCallOpenTownList();
	void OnCallTownMinimap();
	void OnClickHomeRegistOK(lwUIWnd kParent);
	void OnClickHomeRegistDayDrop(lwUIWnd kSelf);
	void OnClickHomeRegistDayDropItem(lwUIWnd kSelf);
	void OnClickMyHomeInfoRequest();
	void OnClickHomeUnRegistAuction();
	void OnClickMoveToTown(lwUIWnd kSelf, const short siStreet = 0);
	void OnClickHomePost();
	void OnClickHomeSafeInv();
	
	void OnCallHomeAuctionRegist();
	void OnSetMyHomeInfoUI();
	void OnSendHomeInfoRequest(SHOMEADDR const& kAddr,lwHomeUIUtil::E_HOME_EVT const& eEvent=lwHomeUIUtil::EEVT_NONE);

	//MiniMap
	void OnClickMinimapTownNumPopup(lwUIWnd kSelf);
	void OnClickMinimapTownNumPopupItem(lwUIWnd kSelf);
	void OnClickMinimapHomeItem(lwUIWnd kParent, int const siHouseNo);
	void OnClickGoToMyHome();

	void OnCallHomeTownMinimap(short const iStreetNo, int const iHouseNo = 0);
	void OnSetSelectedHomeInfo(XUI::CXUI_Wnd* pMainUI, bool const bIsMyHome, SMYHOME const& kHome);

	//etc
	void OnCallInvitatianNotice(std::wstring const& kUserName);
	bool CheckExistHaveHome();
	bool CheckMyHome();

	// 세금
	bool SendMyHomeTaxPay_Packet();													// 납부
	int	SecToWeek(__int64 const& i64Sec, bool const& bRoundOff=true);				// 초를 주단위로 환산
	bool ReserveMyHomeTaxPayEvent();												// 세금 UI 호출 이벤트 예약
	bool OnCallUIHomePayTax(SMYHOME const& kHome);														// 세금 납부 UI 호출
	bool OnCallUIHomePayTaxDetail(SMYHOME const& kHome);
	
	// 소유권 박탈(세금 밀린경우)
	bool OnCallUIDeprivedHome(short const& sStreetNo, int const& sHouseNo, __int64 const& i64Tax);
	// 세금 연체 알림
	bool OnCallNotiDelayTax(short const& sStreetNo, int const& sHouseNo, __int64 const& i64Tax, BM::PgPackedTime const& kPayTaxTime);

	// 경매에 올린 내집 정보
	//	bool ReserveMyHomeAuctionInfo();
	bool OnCallUIHomeAuctionInfo(SMYHOME const& kHome);	//경매 정보
	bool IsHomeBidding();
	bool IsHomeBlock();
	
	// 마이홈 공개 설정(옵션)
	bool ReserveMyHomeOptionEvent();
	bool OnCallUIHomeOption(SMYHOME const& kHome);
	bool SendMyHomeOption_Packet(lwUIWnd kWnd);
	
	// 데코레이션 부가기능 (버프 받기)
	bool ReserveMyHomeUseDecoUIEvent();
	bool OnCallUIUseDecoUI(SMYHOME const& kHome);
	bool SendMyHomeUseDeco_Packet(lwUIWnd kWnd);

	// 예약된 이벤트 처리	
	bool OnEventProcedure(lwHomeUIUtil::E_HOME_EVT const& eEvent, SMYHOME const& kHome);
	void SavePickedHouse(lwGUID kGuid);

	// 마이홈 아르바이트
	bool OnClickMyHomeSideJobOnMaid();
	bool OnClickMyHomeSideJobPeriodRadioButton(lwUIWnd kParentWnd, int iPos);
	bool OnClickMyHomeSideJobOK(lwUIWnd kParentWnd);
	bool OnClickMyHomeSideJobSettingCancel(lwUIWnd kParentWnd);
	bool OnClickMyHomeSideJobOnMaidResive(SMYHOME const& kHome);
	void SetMyHomeInfoEnterHome(SMYHOME const& kHome);
	void SendMyHomeInfoRequest();
	bool IsRepairService();
	bool IsEnchantService();
	bool OnCheckSideJobList(lwUIWnd lwSelf, int iCheck);
	bool OpenMyHomeRepairService();
	bool OpenMyHomeEnchantService();
	bool OnClickRepairService();
	bool ExcuteRepairService();
	bool OnClickEnchantService();
	bool ExcuteEnchantService();
	bool OnCallMyHomeSideJobRemainText(lwUIWnd lwSelf, int iSelect);
	float GetMyHomeSideJobDiscountRate(eMyHomeSideJob kSideJob, eMyHomeSideJobRateType kRateType);
	void OnClickMyHomeSideJobNpcMove(lwGUID kGuid);
	void OnClickMyHomeSideJobNpcRemove(lwGUID kGuid);
	void OnClickMyHomeSideJobSoulGold(lwGUID kGuid);
	void OnClickMyHomeSideJobSoulCount(lwGUID kGuid);
	void OnClickMyHomeSideJobSocketGold(lwGUID kGuid);
	void OnClickMyHomeSideJobSocketRate(lwGUID kGuid);
	void OnClickMyHomeSideJobEnchantRate(lwGUID kGuid);
	void ExcuteMyHomeSideJobSoulGold();
	void ExcuteMyHomeSideJobSoulCount();
	void ExcuteMyHomeSideJobSocketGold();
	void ExcuteMyHomeSideJobSocketRate();
	void ExcuteMyHomeSideJobEnchantRate();
	void OpenMyHomeSideJobSoulGold();
	void OpenMyHomeSideJobSoulCount();
	void OpenMyHomeSideJobSocketGold();
	void OpenMyHomeSideJobSocketRate();
	void OpenMyHomeSideJobEnchantRate();

	void OnSendSideJobBoardList();
	void OnCallSideJobBoard();
	PgHomeUIUtil::E_SIDE_JOB_SORT_STATE GetSideJobType(eMyHomeSideJob kSideJob, eMyHomeSideJobRateType kType);
	void OnClickSideJobBoardTownNumber(lwUIWnd kSelf);
	void OnClickSideJobBoardTownItem(lwUIWnd kSelf);
	void OnClickSideJobBoardListViewType(lwUIWnd kSelf);
	void OnClickSideJobBoardListViewTypeItem(lwUIWnd kSelf);
	void OnClickSideJobBoardBeginPage(lwUIWnd kSelf);
	void OnClickSideJobBoardJumpPrevPage(lwUIWnd kSelf);
	void OnClickSideJobBoardJumpNextPage(lwUIWnd kSelf);
	void OnClickSideJobBoardEndPage(lwUIWnd kSelf);
	void OnClickSideJobBoardPage(lwUIWnd kSelf);
	void OnClickSideJobBoardMoveHome(lwUIWnd kSelf);
}

std::wstring const WSTR_HOME_PAY_TAX_UI(L"SFRM_HOME_TAX_PAYMENT");
std::wstring const WSTR_HOME_DEPRIVED_HOME_UI(L"SFRM_HOME_TAX_INFO");
std::wstring const WSTR_HOME_AUCTION_INFO_UI(L"SFRM_HOME_AUCTION_INFO");
std::wstring const WSTR_HOME_OPTION_UI(L"SFRM_HOME_MAN_OPTION");
std::wstring const WSTR_HOME_USE_DECO_UI(L"SFRM_HOME_USE_DECO");
std::wstring const WSTR_HOME_SIDE_JOB_UI(L"SFRM_HOME_SIDE_JOB");

#endif // FREEDOM_DRAGONICA_CONTENTS_MYHOME_LWHOMEUI_SCRIPT_H