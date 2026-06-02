#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_LWMARKET_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_LWMARKET_H

#include "PgMarketDef.h"

namespace lwMarket
{
	void RegisterWrapper(lua_State *pkState);

	void lwCallOpenMarket();
	//------------------------------------------------------------------
	// 상점 관리 동작
	//------------------------------------------------------------------
	void lwDrawShopHistoryPoint(lwUIWnd UISelf);
	void lwCallAddRegUI();
	void lwCloseAddRegUI(lwUIWnd UIParent);
	void lwAddRegItem(lwUIWnd UIParent);
	void lwDelRegListItem(lwUIWnd UITab);
	void lwClearShopRegUI(lwUIWnd UITab);
	void lwRequestMyShopInfo();
	void lwRequestShopAction(lwUIWnd UITab, lwUIWnd UISelf);
	void lwRequestMinimumCost(lwUIWnd UITab);
	void lwDrawShopOpenTime(lwUIWnd UISelf, lwUIWnd UITab, int Type);
	void lwRequestShopStateChange();
	void lwRequestShopInfoChange();
	void lwRequestShopUpdateOpenTime(lwUIWnd UITab);
	void lwRequestShopUpdateHP(int const ItemNo);
	void lwUpdateRegList(lwUIWnd UISelf);
	bool lwIsChangeShop(int const iType);
	void lwUpdataRegAction();
	void lwCloseMyShop();	


	bool AddRegistSellArticleInItem(XUI::CXUI_Wnd* pParent, SARTICLEINFO& kArticleInfo, SMarketItemInfoBase& kViewItemInfo);
	bool AddRegistSellArticleInMoney(XUI::CXUI_Wnd* pParent, int const iType, SARTICLEINFO& kArticleInfo, SMarketItemInfoBase& kViewItemInfo);


	bool UpdateRegList();
	void RegListClear();
	void AddRegListItemToPacketData(SMarketItemInfoBase const& ItemInfo);
	void SetMinimumCost(int const iGold, int const iSilver, int const iCopper);
	void RequestUseShopModifyItem(XUI::CXUI_Wnd* pWnd, std::wstring const& kShopName, E_MARKET_GRADE const eType, bool const bIsChange = false);
	//------------------------------------------------------------------
	// 재고 관리 동작
	//------------------------------------------------------------------
	void lwPrevSoldPageInfo();
	void lwNextSoldPageInfo();
	void lwDrawSoldPage();
	void lwDrawSoldPageInfo(lwUIWnd UISelf);
	void lwRequestSoldReceive(lwUIWnd UIGroup);

	void DrawSoldPage(int const iPage);
	void SetSoldListItem(lwUIWnd& UIItem, SMarketSoldItemInfo const& DrawInfo);

	//------------------------------------------------------------------
	// 공용
	//------------------------------------------------------------------
	void lwTabChangeState(int const TabID);
	void lwTabBtnChange(int const TabID);
	void lwDrawIconImage(lwUIWnd UISelf);
	void lwMarketAction(lwUIWnd UIAuc);
	void lwClearOpenMarket();
	void lwBuildItemCheckUpdate(lwUIWnd UISelf);
	void lwListItemCheckUpdate(lwUIWnd UISelf);
	void lwDrawMarketItemTooltip(lwUIWnd UIItem);
	void lwInOpenMarket();
	void lwOutOpenMarket();

	void SelectItemCheck(lwUIWnd& UISelf, bool const bIsClick);
	void SelectItemCheckT2(lwUIWnd& UISelf, bool const bIsClick);
	void AddItemListItem(lwUIWnd& UIList, SMarketItemInfoBase const& ItemInfo);
	int	 LimitLength(int Data, int Min, int Max);
	void TabChange(int const TabID);
	E_MARKET_GRADE GetShopGrade();
	bool ReturnResult(EMarketErrType const Rst);

	void RecvMarket_Command(WORD const wPacketType, BM::Stream& rkPacket);

	int  lwGetPrevMarketGrade();		
	int  lwGetCurrentMarketGrade();	
	void lwSetCurrentMarketGrade(int const iGrade);
	void lwRevertMarketGrade();
	void lwFixGradeRadioBtn();
	void lwMarketOpenConfirm();
	void lwMarketOpenCancel();
	void CallMarketOpenConfirm(BM::Stream kPacket);
	void lwMarKetGradeRadioBtnInit();

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market Shop 분리 2009. 4. 20
	//---------------------------------------------------------------------------------------------------------------------------------
	void lwClearSearchInfoUI(lwUIWnd UITab, int const DefaultLankTT, int const DefaultClassTT);
	void lwRequestShopList();
	void lwRequestEnterShop();
	void lwSelectItemType(int const Mode, int const Type);
	void lwSearchBtnState(lwUIWnd UISelf);
	void lwPrevShopPageInfo();
	void lwNextShopPageInfo();
	void lwDrawShopPageInfo(lwUIWnd UISelf);

	void DrawShopList();
	void ClearShopListItem(XUI::CXUI_Wnd* pItemWnd);
	void DrawShopListItem(XUI::CXUI_Wnd* pItemWnd, SMARKET_KEY const& kShopInfo);
	void DrawEnterShopItemList();
	void DrawEnterShopItem(XUI::CXUI_Wnd* pItemWnd, SUserMarketArticleInfo const& kItemInfo);
	void OnClickEnterShopBuyItem(lwUIWnd UITab);

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market Search 분리 2009. 4. 7
	//---------------------------------------------------------------------------------------------------------------------------------
	void lwSearchItemList(lwUIWnd UITab, lwUIWnd UISelf);
	void lwOnOverSearchListItem(lwUIWnd UISelf);
	void lwOnCallSearchListPopUpMenu(lwUIWnd UISelf);
	void lwPrevSearchPageInfo();
	void lwNextSearchPageInfo();
	void lwDrawSearchPageInfo(lwUIWnd UISelf);
	
	//private
	void RequestSearchItem(SPT_C_M_UM_REQ_MARKET_QUERY& kSearchKey);
	void RequestBuySelectItem(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iItemCount);
	
	void DrawSearchList();
	void ClearSearchListItem(XUI::CXUI_Wnd* pItemWnd);
	void DrawSearchListItem(XUI::CXUI_Wnd* pItemWnd, SMARKET_KEY const& kShopInfo, SUserMarketArticleInfo const& kItemInfo);
	
	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market RegItem 분리 2009. 7. 19
	//---------------------------------------------------------------------------------------------------------------------------------
	void lwOnClickChangeRegTabType(lwUIWnd kSelf, int const iType);
	void lwOnClickMoneyChangeRate(lwUIWnd kSelf);
	void lwOnTickCheckChangeMaxValue(lwUIWnd kSelf);


	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market SearchTab Common 분리 2009. 4. 7
	//---------------------------------------------------------------------------------------------------------------------------------
	void lwDrawShopListUpdate(lwUIWnd UISelf);
	void lwOnClickListItem(lwUIWnd UISelf);
	void lwSelectBuyOK(lwUIWnd UISelf);
	void lwSelectShopInOK(bool bSearch);
	void lwOnCallItemBuyUI(lwUIWnd UISelf);

	void lwDrawMarketItemIcon(lwUIWnd UISelf);
	void lwOnOverMarketItemToolTip(lwUIWnd UISelf);
	void CallItemBuyUI(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iBuyCount);
}

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Util 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
namespace lwMarketUtil
{
	void SetUIText(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, std::wstring const& kText, bool bIsReduce = false);
	void SetUITextToItemName(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, DWORD const dwItemNo, SEnchantInfo const& kEnchantInfo, bool bIsReduce = false, bool bIsCRLF = false);
	void SetUITextToItemCost(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kGold, std::wstring const& kCash, ERegCostType const Type, __int64 const CostValue);
	void SetUIIconToItemNo(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, DWORD const dwItemNo, SEnchantInfo const& kEnchantInfo, int const iItemCount, BM::PgPackedTime const& kGenTime);
	void SetUIIconToBaseItem(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, PgBase_Item const& kItem);
	void CalcMoneyChangeRate(int const iDest, int const iSour, float& fDest, float& fSour, float& fRate);
	std::wstring const FloatConvertToStr(float const fValue, int const iDecimalCount);
	bool SendMarketSearchText(std::wstring const & kSearchText);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_LWMARKET_H