#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKET_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKET_H

#include "PgMarketDef.h"

//---------------------------------------------------------------------------------------------------------------------------------
//	Market PageUtil Class 분리 2009. 4. 20
//---------------------------------------------------------------------------------------------------------------------------------
template< int PAGE_MAX_ITEM_SLOT >
class PgMaketPage
{
public:
	std::wstring const GetPageString()
	{
		int iMax = TotalItem();

		int iPageFirst = Page() * PAGE_MAX_ITEM_SLOT;
		int iPageFinal = 0;
		int NextPage = Page() + 1;
		if( NextPage >= TotalPage() )
		{
			iPageFinal = iMax;
		}
		else
		{
			iPageFinal = NextPage * PAGE_MAX_ITEM_SLOT;
		}
		
		if( 0 != iMax )
		{
			iPageFirst += 1;
		}

		wchar_t	szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, TTW(403056).c_str(), iPageFirst, iPageFinal, iMax);
		return std::wstring(szTemp);
	};

	virtual bool PrevPage()
	{
		int const iPrevPage = Page() - 1;
		if( 0 > iPrevPage )
		{
			return false;
		}
		Page(iPrevPage);
		return true;
	};

	virtual bool NextPage()
	{
		int const iNextPage = Page() + 1;
		if( TotalPage() <= iNextPage )
		{
			return false;
		}
		Page(iNextPage);
		return true;
	};

	int const GetPageSlotMax() const
	{ 
		return PAGE_MAX_ITEM_SLOT; 
	};

	PgMaketPage() : m_kTotalItem(0), m_kPage(0), m_kTotalPage(0){};
	virtual ~PgMaketPage(){};

protected:
	CLASS_DECLARATION_S(int, TotalItem);
	CLASS_DECLARATION_S(int, Page);
	CLASS_DECLARATION_S(int, TotalPage);
};

//---------------------------------------------------------------------------------------------------------------------------------
//	기존 마켓 클래스 2008. ?. ?
//---------------------------------------------------------------------------------------------------------------------------------
class PgMarket
{
public:
	//	내물건 등록
	void ViewMessage(int Type);

	PgMarket(void);
	virtual ~PgMarket(void);
	//------------------------------------------------------------------
	// 상점 관리 동작
	//------------------------------------------------------------------
	int RequestAddRegItem(SARTICLEINFO const& kRegInfo, SMarketItemInfoBase const& kItemInfo);
	int RequestDelRegItem(BM::GUID const& Guid);
	int RequestOpenShop(std::wstring const& ShopName, E_MARKET_GRADE const Type);
	int RequestMinimumCost(SItemPos kPos, BM::GUID const& Guid);
	void ResultAddRegItem(BM::Stream& kPacket);
	void ResultMinimumCost(BM::Stream& kPacket);
	void ResultRegShopItemInfo(BM::Stream& kPacket);
	void ResultUpdateOpenTime(BM::Stream& kPacket);
	void ResultUpdateShopState(BM::Stream& kPacket);
	void ResultOpenShop(BM::Stream& kPacket);
	void ResultDelRegItem(BM::Stream& kPacket);
	void ResultCloseMyShop(BM::Stream& kPacket);
	void ClearMyShopInfo() { m_kMyShopInfo.Clear(); };
	void FlowTime(float const TickTime);
	bool FindRegTabItem(BM::GUID const& Guid, SMarketItemInfoBase& Info);
	void ResultRegShopItemDealingInfo(BM::Stream& kPacket);

	//------------------------------------------------------------------
	// 재고 관리 동작
	//------------------------------------------------------------------
	bool GetSoldPageItem(size_t PageNum, kSoldList& List);
	void ClearStockInfo() { m_kSoldInfo.Clear(); };
	void ResultDealingRead(BM::Stream& kPacket);
	bool FindStockTabItem(BM::GUID const& Guid, SMarketItemInfoBase& Info);

	//------------------------------------------------------------------
	// 기타 동작
	//------------------------------------------------------------------
	int ClearOpenMarket();
	int GetMaxOnlineOpenTime(E_MARKET_GRADE const Type);
	int GetMaxOfflineOpenTime(E_MARKET_GRADE const Type);
	__int64 GetClassLimit(int const TypeID);
	bool UpdateTime();
	void TempRegItemInfo(XUI::CXUI_Wnd* pParent, SItemPos const& Pos);
	bool RequestResult(EUserMarketResult Result);
	int GetMaxItemSlot(E_MARKET_GRADE const Type);
	bool DeleteTempItem(EMarketTabState const Type);
	E_MARKET_GRADE GetPrevMarketGrade() const;	
	E_MARKET_GRADE GetCurrentMarketGrade() const;
	void SetCurrentMarketGrade(int const iGrade);
	void SetPrevMarketGrade(int const iGrade);
	void RevertMarketGrade();


protected:
	//------------------------------------------------------------------
	// 상점 관리
	//------------------------------------------------------------------
	CLASS_DECLARATION_S_NO_SET(SMyShopInfo, MyShopInfo);

	CLASS_DECLARATION_S_NO_SET(E_MARKET_STATE, ShopState);
	CLASS_DECLARATION_S_NO_SET(__int64, ListMaxCount);
	//------------------------------------------------------------------
	// 재고 관리
	//------------------------------------------------------------------
	CLASS_DECLARATION_S_NO_SET(SSoldItemInfo, SoldInfo);

	CLASS_DECLARATION_S(int, StockPage);
	CLASS_DECLARATION_S(int, SoldPage);
	//------------------------------------------------------------------
	// 구매 관리
	//------------------------------------------------------------------
	CLASS_DECLARATION_S(bool, bIsPushSearchBtn);
	CLASS_DECLARATION_S(float, UpdateTimeSearchBtn);
	//------------------------------------------------------------------
	// 기타
	//------------------------------------------------------------------
	CLASS_DECLARATION_S(EMarketTabState, TabState);
	CLASS_DECLARATION_S(float, OnTimeUpdate);
	CLASS_DECLARATION_S(bool, IsSendOK);
	float m_fPrevTime;
	float m_fOnlineOpenTime;
	float m_fOfflineOpenTime;
	
	CLASS_DECLARATION_S(BM::GUID, TempDeleteItem);
	CONT_MARKET_KEY_LIST m_kContMarketKeyBuffer;
		
	E_MARKET_GRADE m_ePrevMarketGrade;
	E_MARKET_GRADE m_eCurrentMarketGrade;

	CLASS_DECLARATION_S(SUserMarketDealingList, ContDealing);
	CLASS_DECLARATION_S(int, DealingMax);	
};

#define	g_kMarketMgr	SINGLETON_STATIC(PgMarket)

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Shop 분리 2009. 4. 20
//---------------------------------------------------------------------------------------------------------------------------------
class PgMarketShopMgr
	: public PgMaketPage< MARET_ARTICLE_ONE_PAGE >
{
public:
	typedef SUserMarketArticleInfo		ITEM_VAL;
	typedef std::set< SMARKET_KEY >		CONT_SHOP_ITEM;
	typedef std::list< ITEM_VAL >		CONT_ENTER_SHOP_ITEM;

	void RecvShopItem(BM::Stream& rkPacket);
	void GetPageInfo(CONT_SHOP_ITEM& kList) const;
	bool RecvEnterShopItem(BM::Stream& rkPacket);
	bool GetEnterShopItemInfo(BM::GUID const& ItemGuid, ITEM_VAL& kItem) const;
	bool RecvBuyEnterShopItem(BM::Stream& rkPacket);

	CONT_ENTER_SHOP_ITEM const& GetEnterShopItemList() const { return m_kEnterShopItemContainer; };
	
	void Clear();

	PgMarketShopMgr();
	~PgMarketShopMgr();

protected:
	void DeleteShop(BM::GUID const& Guid);
	void DeleteEnterShopItem(BM::GUID const& Guid);

protected:
	CONT_SHOP_ITEM			m_kShopContainer;
	CONT_ENTER_SHOP_ITEM	m_kEnterShopItemContainer;
	CLASS_DECLARATION_S(BM::GUID, kEnterShopGuid);
};

#define g_kMKTShopMgr SINGLETON_STATIC(PgMarketShopMgr)

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Search 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
class PgMarKetSearchMgr
	: public PgMaketPage< MARET_ARTICLE_ONE_PAGE >
{
public:
	typedef SMARKET_KEY					ITEM_KEY;
	typedef SUserMarketArticleInfo		ITEM_VAL;
	typedef CONT_MARKET_SEARCH_RESULT	CONT_SERACH_ITEM;
	typedef SPT_C_M_UM_REQ_MARKET_QUERY	SEARCH_KEY;

	void RecvSearchItem(BM::Stream& rkPacket);
	void GetPageInfo(CONT_SERACH_ITEM& kList) const;
	bool GetItemInfo(BM::GUID const& ShopGuid, BM::GUID const& ItemGuid, ITEM_VAL& kItem) const;
	void DeleteItemInfo(BM::GUID const& ShopGuid, BM::GUID const& ItemGuid);
	bool RecvBuyEnterShopItem(BM::GUID const& ShopGuid, BM::GUID const& ItemGuid, int const iLeftItemNum);

	void Clear();
	
	PgMarKetSearchMgr();
	~PgMarKetSearchMgr();

protected:
	CLASS_DECLARATION_S(int, Type);
	CLASS_DECLARATION_S(int, TypeNode);
	CLASS_DECLARATION_S(SEARCH_KEY, SearchKey);

private:
	CONT_SERACH_ITEM	m_kItemContainer;
};

bool CheckMarketRegItem(PgPlayer const* pkPlayer, PgBase_Item const& kItem, CItemDef const* pDef, bool const bMsg);

#define g_kMKTSearchMgr SINGLETON_STATIC(PgMarKetSearchMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKET_H