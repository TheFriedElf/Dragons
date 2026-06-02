#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKETDEF_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKETDEF_H

#include "lohengrin/packetstruct.h"

#define MAX_STOCK_SLOT 8
#define MAX_LIST_ITEM 20
#define MAX_SHOP_LIST_ITEM 10
#define DEF_SEC_TIME 60
#define DEF_MIN_TIME 60
#define DEF_HOUR_TIME 24

namespace PgMarketUtil
{
	bool RequestResult(EUserMarketResult const Result);
};

typedef enum : BYTE
{
	RCT_GOLD = 0,
	RCT_CASH,
}ERegCostType;

typedef enum : BYTE
{
	ETS_SHOPLIST	= 0,
	ETS_SEARCHLIST	= 1,
	ETS_SHOPINLIST	= 2,
	ETS_REG,
	ETS_STOCK,
}EMarketTabState;

typedef enum : BYTE
{
	RTT_ONLINE = 0,
	RTT_OFFLINE,
}ERegTimeType;

typedef enum
{
	RON_TYPE1 = 0,
	RON_TYPE2 = 259200,
}ERegOnlineOpenTime;

typedef enum
{
	ROFF_TYPE1 = 0,
	ROFF_TYPE2 = 36000,
}ERegOfflineTime;

typedef enum : BYTE
{
	RSS_TYPE1 = 5,
	RSS_TYPE2 = 10,
	RSS_TYPE3 = 20,
}ERegShopSlot;

typedef enum : BYTE
{
	MET_SUCCESS = 0,
	MET_FAIL,
	MET_REG_ALREADY_ITEM,
	MET_REG_MAX_SLOT,
	MET_REG_CONTRACT_INS,
	MET_REG_SHOP_STARTED,
	MET_REG_LIST_NOTHING,
	MET_NOT_EXIST_RECORD,
	MET_NOT_COST_VALUE,
	MET_NOT_ITEM_CNT_VALUE,
	MET_REG_NON_TIME_ADD,
	MET_NOT_ADD_ITEM,
	MET_EXIST_NO_INV_POS,
	MET_DEALING_MAX_COUNT,
}EMarketErrType;

typedef	enum : BYTE
{
	ST_ITEM = 0,
	ST_SELLER,
}ESearchType;

typedef enum
{
	ECC_NORMAL	= 5,
	ECC_GOOD	= 3,
	ECC_HISTORY	= 2,
}ECostCharge;

bool bExistEmptyInv(DWORD const dwItemNo);

typedef struct tagMarketIconInfo
{
	tagMarketIconInfo() : dwItemNum(0), iCnt(0), cItemGroup(0), cInvPos(0) {};
	tagMarketIconInfo(DWORD const ItemNum, int const Cnt, BYTE const ItemGroup, BYTE const InvPos)
	{
		dwItemNum = ItemNum;
		iCnt = Cnt;
		cItemGroup = ItemGroup;
		cInvPos = InvPos;
	};

	DWORD	dwItemNum;
	int		iCnt;
	BYTE	cItemGroup;
	BYTE	cInvPos;
}SMarketIconInfo;

typedef struct tagMarketItemInfoBase
{
	tagMarketItemInfoBase()
		: Guid()
		, eCostType(RCT_GOLD)
		, i64CostValue(0)
		, kItemType(UMAT_ITEM)
		, SellMoney(0)
	{
	};

	BM::GUID	Guid;
	ERegCostType eCostType;
	__int64		i64CostValue;
	eUserMarketArticleType kItemType;
	short		SellMoney;
	PgBase_Item	kItem;
}SMarketItemInfoBase;
typedef std::map< BM::GUID, SMarketItemInfoBase > kBaseItemContainer;

typedef struct tagMarketSoldItemInfo 
	: public tagMarketItemInfoBase
{
	tagMarketSoldItemInfo() : tagMarketItemInfoBase(), SaveTime(0), iRate(0), wstrBuyerName()
	{
	};
	int SaveTime;
	int iRate;
	std::wstring wstrBuyerName;
}SMarketSoldItemInfo;
typedef std::map< BM::GUID, SMarketSoldItemInfo > kSoldItemContainer;

typedef struct tagMyShopData
	: public tagMARKET_KEY
{
	tagMyShopData()
		: tagMARKET_KEY()
		, iOnTime(0)
		, iOffTime(0)
	{
	};
	__int64	iOnTime;
	__int64 iOffTime;
}SMyShopData;

typedef struct tagShopItemInfo
{
	tagShopItemInfo()
	{
		Clear();
	};

	bool ConvertItemList(CONT_USER_MARKET_ARTICLEINFO_LIST const& ItemList);
	bool AddItem(SMarketItemInfoBase const& ItemInfo);
	bool FindItem(BM::GUID const& Guid, SMarketItemInfoBase& Base);
	bool UpdateInfo(BM::GUID const& Guid, SMarketItemInfoBase const& Base);
	virtual bool DelItem(BM::GUID const& Guid);
	virtual void Clear()
	{
		m_kItemCont.clear();
	}

	CLASS_DECLARATION_S_NO_SET(kBaseItemContainer, ItemCont);
}SShopItemInfo;

//------------------------------------------------------------------
// 상점 관리
//------------------------------------------------------------------
typedef std::map< BM::GUID, SARTICLEINFO > kRegItemContainer;
typedef struct tagMyShopInfo
	: public SShopItemInfo
{
	tagMyShopInfo();

	bool AddRegItem(BM::GUID const& Guid, SARTICLEINFO const& RegInfo);
	bool DelItem(BM::GUID const& Guid);
	bool FindRegItem(BM::GUID const& Guid, SARTICLEINFO& RegInfo);
	void SetShopInfo(SMyShopData const& ShopInfo)
	{
		m_kShopInfo = ShopInfo;
	}
	void UpdateTimeInfo(__int64 const OnTime, __int64 const OffTime);
	void FlowTime(__int64 const TickTime);
	void UpdateHP(int const Hp);
	void Clear()
	{
		m_kTempRegList.clear();
		SShopItemInfo::Clear();
	}

	CLASS_DECLARATION_S_NO_SET(SMyShopData, ShopInfo);
	CLASS_DECLARATION_S_NO_SET(kRegItemContainer, TempRegList);
}SMyShopInfo;

//------------------------------------------------------------------
// 재고 관리 동작
//------------------------------------------------------------------
typedef std::map< BM::GUID, SMarketSoldItemInfo > kSoldItemContainer;
typedef std::list< SMarketSoldItemInfo > kSoldList;

typedef struct tagSoldItemInfo
{
	tagSoldItemInfo()
	{
		Clear();
	};

	bool ConvertSoldList(CONT_USER_MARKET_DEALINGINFO_LIST const& SoldList, __int64 const iSaveTime);
	bool DelSoldItem(BM::GUID const& Guid);
	bool GetSoldPageItem(size_t& PageNum, kSoldList& kList);
	bool FindItem(BM::GUID const& Guid, tagMarketItemInfoBase& Info);
	int Size() const { return m_kSoldCont.size(); };
	void Clear(){ m_kSoldCont.clear(); };
private:
	kSoldItemContainer	m_kSoldCont;
}SSoldItemInfo;

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_PGMARKETDEF_H