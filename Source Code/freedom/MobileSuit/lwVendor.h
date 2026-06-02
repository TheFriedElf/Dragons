#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_LWVENDOR_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_LWVENDOR_H

namespace lwVendor
{
	void RegisterWrapper(lua_State *pkState);
	void lwRequestMyVendorInfo(BM::GUID kOwnerGuid);
	void lwRequestVendorExit();
	void lwRequsetVendorCreate(std::wstring kVendorTitle);
	void lwRequsetVendorRename(std::wstring kVendorTitle);
	void lwRequsetVendorDelete();
	void lwRequsetVendorBuy(int const iBuyNum);
	void RecvMarket_Command(WORD const wPacketType, BM::Stream& rkPacket);

	void lwResetSelectArticle();
	void lwCallVendor();
	void SetUIIconToBaseItem(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, PgBase_Item const& kItem);
	__int64 lwGetPrice(bool bIsCash);
	void lwOnCallVendorBuyUI();
	void RequestBuySelectItem(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iItemCount);
	void CallVendorBuyUI(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iBuyCount);
	void SetUITextToItemCost(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kGold, std::wstring const& kCash, ERegCostType const Type, __int64 const CostValue);
	void SetUIText(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, std::wstring const& kText, bool bIsReduce = false);
	bool lwIsMyVendor();
	bool lwCheckVendorRemainTime();
	bool lwCheckVendorGround();
}

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_LWVENDOR_H