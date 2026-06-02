#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDOR_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDOR_H

#include "PgMarketDef.h"

class PgVendor
{
public:
	PgVendor(void);
	virtual ~PgVendor(void);

	void Clear();

	//노점 정보 받기
	bool SetVendorInfo(BM::Stream rkPacket);

	void RequestMyVendorInfo(BM::GUID kOwnerGuid);
	void RequestVendorEnter();
	void RequestVendorExit();
	void RequsetVendorCreate(std::wstring kVendorTitle);
	void RequsetVendorRename(std::wstring kVendorTitle);
	void RequsetVendorDelete();
	void RequestVendorRefresh();

	void RecvMarket_Command(WORD const wPacketType, BM::Stream& rkPacket);

	bool IsMyVendor();
	bool IsMyVendor_Guid(BM::GUID const &rkOwnerGuid);
	void ResetSelectArticle();
	__int64 GetPrice(bool bIsCash);
	bool SetPrice(PgBase_Item const& kItem);
	void OnCallVendorBuyUI();

	//노점 열었을 때/ 정보 갱신될 때, 오픈마켓에 아이템이 있는지 확인
	bool IsEmptyArticle();
	
private:
	CLASS_DECLARATION_S(BM::GUID, OwnerGuid);
	CLASS_DECLARATION_S_NO_SET(SMyShopInfo, VendorInfo);
	CLASS_DECLARATION_S(SMarketItemInfoBase, SelectArticleInfo);
};

#define	g_kVendorMgr	SINGLETON_STATIC(PgVendor)

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDOR_H