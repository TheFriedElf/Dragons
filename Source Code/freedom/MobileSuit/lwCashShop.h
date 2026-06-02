#ifndef FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_LWCASHSHOP_H
#define FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_LWCASHSHOP_H

#include "PgCashShop.h"
#include "lwUI.h"

namespace lwCashShop
{
	typedef enum etagLimitedStateType
	{
		LIMITEDSTATE_NA = 0,
		LIMITEDSTATE_FINISH = 1,
		LIMITEDSTATE_WAIT,
		LIMITEDSTATE_SOLDOUT,
		LIMITEDSTATE_TIME,
		LIMITEDSTATE_IMPEND,
		LIMITEDSTATE_HOT,
		LIMITEDSTATE_SALE,
	}ELimitedStateType;

	typedef enum eCashItemBuyType
	{
		CIBT_NONE	= 0,
		CIBT_BUY	= 1,
		CIBT_BUY_REAL,
		CIBT_RENEW,
		CIBT_RENEW_REAL,
		CIBT_GIFT,
		CIBT_GIFT_REAL,
		CIBT_IMPORTUNE,
		CIBT_IMPORTUNE_REAL,
		CIBT_PV_ALL,
		CIBT_PV_ALL_REAL,
		CIBT_BASKET,
		CIBT_BASKET_REAL,
		CIBT_BASKET_PART,
		CIBT_BASKET_PART_REAL,
		CIBT_STATIC_ITEM_BUY,
		CIBT_STATIC_ITEM_BUY_REAL,
		CIBT_STATIC_ITEM_RENEW,
		CIBT_STATIC_ITEM_RENEW_REAL,
	}ECashItemBuyType;

	typedef enum eLimitedType
	{
		LT_TIME			= 2,
		LT_TIMELESS		= 3,
		LT_PACKAGE		= 4,
	}ELimitedType;

	typedef enum ePageBtnType
	{
		PBT_NORMAL	= 0,
		PBT_JUMP	= 1,
		PBT_LAST,
	}EPageBtnType;

	typedef struct tagCategoryInfo
	{
		tagCategoryInfo():iTab(0), iSub(0){};
		int iTab;
		int iSub;
	}SCateInfo;
	int const LIMITED_ITEM_NAME_MAX_LENGTH = 145;
	int const CASH_ITEM_IMG_SIZE = 70;
	//! 스크립팅 시스템에 등록한다.
	bool RegisterWrapper(lua_State *pkState);

	void lwCallCashShop();
	void TopCategoryOnClick(lwUIWnd kWnd, int const iCategory);
	void SubCategoryOnClick(lwUIWnd kWnd, int const iCategory);
	void SetCategoryPageClick(lwUIWnd kWnd, int const iBtnType);
	void SetShopPage(XUI::CXUI_Wnd* pkWnd, CONT_DEF_CASH_SHOP_ARTICLE const & rkCate, int const iPage);
	void SetGiftPage(XUI::CXUI_Wnd* pkWnd, CONT_CASHGIFTINFO const& rkCate, bool bRecv = true);
	void SetLimitedPage(XUI::CXUI_Wnd* pkWnd, CONT_DEF_CASH_SHOP_ARTICLE const & rkCate, CONT_CASH_SHOP_ITEM_LIMITSELL const & rContLimited, int iPage);
	XUI::CXUI_Wnd* SetShopForm(XUI::CXUI_Wnd* pkWndParent, PgCashShop::ECashShopForm eFormType); //
	void SetMainForm(XUI::CXUI_Wnd* pkWndParent, PgCashShop::ECashShopType const eCashShopType);
	void lwDayBtnOnClick(lwUIWnd kWnd, int const iBuyType);
	void lwBuyCashItemOnClick(lwUIWnd kWnd, int const iBuyType, int const iIndexFrom = 0);
	void lwGiftCashItemOnClick(lwUIWnd kWnd, bool bRealGift = false);
	void lwRecvGift(lwUIWnd kWnd);
	void lwDoActionCashItem(lwUIWnd kWnd);
	void lwExitCashShop();
	void lwDrawPreviewEquipIcon(lwUIWnd UISelf);
	void lwCallPreviewEquipItemToolTip(lwUIWnd UISelf);
	void lwClearPreviewEquipIcon();
	void lwSendRegMyLank(bool const bIsVisible);
	void lwRotatePreviewActor(float const fAngle);
	void lwSelectCashInvType(lwUIWnd UISelf, int const iType);
	void lwOnClickChangePage(lwUIWnd UISelf);
	void lwSelectItemMoveToInv(lwUIWnd UIParent);
	void lwSelectItemReNew(lwUIWnd UIParent);
	bool lwExistPosInItem(lwUIWnd UIIcon, bool const bIsBasket);
	void lwSetUseMileage(bool const bUse, lwUIWnd kWnd);
	void lwOpenCashChargeURL();
	bool lwSetShopType(int const i);
	int	 lwGetShopType();
	void lwUpdateTotalCashExpression(lwUIWnd kWnd);
	void lwAddPreviewActor(char const* UIModel, char const* UIActorName);
	void lwRemovePreviewActor(char const* UIModel, char const* UIActorName);
	void lwClearCashShopInfo();
	int  lwCurrentBuyType();
	void lwClearPreviewFurnitureModel();
	void lwOnClickUnRegPreviewItem(lwUIWnd kSelf);
	void lwConfirmBasketRegItem(lwUIWnd kSelf);
	void lwConfirmBasketDeRegItem(lwUIWnd kSelf);
	void lwDeleteSelectBasketItem(bool const bIsAllDelete);
	void RecvCashShop_Command(WORD const wPacketType, BM::Stream &rkPacket);
	void lwOnCallStaticCashItemBuy(int const ItemType, int const Subkey = 0);
	void lwOnCallStaticCashItemRenew(int const iIconGroup, int const iIconKey);
	void lwOnCloseStaticCashItemBuy();

	//private

	void UpdateLimitedPage(XUI::CXUI_Wnd* pkWnd);
	bool IsLimitedCategory(const TBL_DEF_CASH_SHOP_CATEGORY& rkSubCate);
	std::wstring MakeTimeStrW(const BM::PgPackedTime& rkTime);
	std::wstring const MakeUseTimeStr(BYTE const bSaleType, int const UseValue, EUseItemType TimeType = UIT_DAY);
	std::wstring const MakeBuyValueStr(TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, int const iBuyType = CIBT_BUY);
	void SetDayBtnInitialize(XUI::CXUI_Wnd* pWnd, TBL_DEF_CASH_SHOP_ARTICLE const& kArticle, int const SelectCost = -1, int const iBuyType = CIBT_BUY);
	void MakeUseTimeStr(BYTE const bSaleType, TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, BM::vstring& rkStr, int const iItemCount = 0, int const iBuyType = CIBT_BUY);
	XUI::CXUI_Wnd* AddPriceToList(BYTE const bSaleType, TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, XUI::CXUI_List* pkList, int const iItemCount = 0, int const iBuyType = CIBT_BUY);
	void WriteToUI(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle);
	void CashMiniIcon(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle);
	void CashNoEquip(XUI::CXUI_Wnd* pWnd,int const iIndexFrom, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle,bool MainUINoEquip = false);
	void WriteToGiftUI(XUI::CXUI_Wnd* pkWnd, SCASHGIFTINFO const & rkArticle, bool bRecv = true);
	void WriteToLimitedUI(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle, TBL_DEF_CASH_SHOP_ITEM_LIMITSELL const& rkLimitedInfo);
	void SetLimitedStateUI(XUI::CXUI_Wnd* pkWnd, ELimitedStateType eStateType);
	void SetLimitedState(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle, TBL_DEF_CASH_SHOP_ITEM_LIMITSELL const& rkLimitedInfo);
	void InitUI(XUI::CXUI_Wnd* pkWnd);
	void InitLimitedUI(XUI::CXUI_Wnd* pkWnd);
	void InitPageUI(XUI::CXUI_Wnd* pkWnd, int const iMaxItems, const PgCashShop::ECashShopForm eFormType = PgCashShop::ECSFORM_COMMON);
	void SetUIPageInfo(XUI::CXUI_Wnd* pkWnd, int const iMaxPage, int const iStartPage = 0, bool const bIsReset = false);
	void DrawCashItem(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const &rkArticle, float const fScale = 1.0f, int const iSize = CASH_ITEM_IMG_SIZE);
	void SetCashNum(XUI::CXUI_Wnd* pkWnd);
	void SetPreviewEquipIcon(RECV_UI_EQUIP_PREVIEW_CONT const& ItemCont);
	void SetPreviewFurnitureItem();
	void ClearPreviewEquipIcon();
	void SetLankingData(XUI::CXUI_Wnd* pkWnd, CONT_CASHRANKINFO const& rkData);
	void SetBestItemInfo(XUI::CXUI_Wnd* pkWnd);
	void SelectedMoveBtnClear(XUI::CXUI_Wnd* pParent);
	void SelectCashTypeClear(XUI::CXUI_Wnd* pParent);
	bool ReturnResult(int const iError, int const iReqLevel=0);
	void OffAllMileageWnd(bool const bOff, XUI::CXUI_Wnd* const pkBuyWnd);
	bool IsBuyingCashItem();
	bool PageUIVisibleCheck(int const iType, int& iMaxPage);
	void SetCashInvViewSlot(XUI::CXUI_Wnd* pParent, int const iType, int const iPage);
	void ViewChangeModel(bool const bIsEquipView);
	
	void UpdateShoppingBasketItemSlot();
	void SetShoppingBasketItemSlot(XUI::CXUI_Wnd* pkParent);
	void CashItemDrawToScaling(XUI::CXUI_Wnd* pkSlot, TBL_DEF_CASH_SHOP_ARTICLE const &rkArticle, float const fScale = 1.0f, int const IconSize = CASH_ITEM_IMG_SIZE);

	void SetVisibleCashShopUI(PgCashShop::ECashShopType const eCashShopType);
	void RegShoppingBasketItem(int const iArticleIDX);
	void UnRegShoppingBasketItem(int const iSlotIDX, int const iArticleIDX);
	void CheckSelectedSlotItem(CONT_SHOPPING_BASKET_SLOT_IDX& kIDX, EQUIP_PREVIEW_CONT& kItems);
	__int64 const GetSelectedSlotItemTotalCost();

	int lwGetStaticBuyType(lwUIWnd kMainUI);
	void CallStaticCashItemBuyUI(int const iArticleIDX, bool const bRenew = false);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_LWCASHSHOP_H