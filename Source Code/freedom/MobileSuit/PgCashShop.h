#ifndef FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_PGCASHSHOP_H
#define FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_PGCASHSHOP_H

#include "variant/ItemDefMgr.h"
#include "lohengrin/dbtables.h"

typedef enum eCashShopResult
{
	ECSR_OK = 0,
	ECSR_EXIST_REGIST_ITEM,
	ECSR_NOT_EXIST_ARTICLE_IDX,
}E_CASHSHOP_RESULT;

typedef enum eCashItemDisplayType
{
	CIDT_NON	= 0,
	CIDT_NEW	= 1,
	CIDT_COOL	= 2,
	CIDT_SALE	= 3,
	CIDT_EVENT	= 4,
	CIDT_HOT	= 5,	
	CIDT_BEST	= 6,
	CIDT_RECOMMAND = 7,
} ECashItemDisplayType;

typedef enum eCashSaleType
{
	ECST_PERIOD = 0,
	ECST_ETERNAL= 1,
	ECST_CONSUME,
	ECST_PACKEGE,
}ECashSaleType;

typedef struct tagModelActorInfo
{
	explicit tagModelActorInfo(std::string const& kModelName, std::string const& kObjectName)
		: kPgUIModelID(kModelName), kObjectID(kObjectName)
	{}
	std::string kPgUIModelID;
	std::string kObjectID;
}SModelActorInfo;

typedef struct tagPreviewItemInfo
{
	tagPreviewItemInfo() : dwItemNo(0), iArticleNo(0), iEquipPos(0), bStyle(false), byTimeType(0), siUseTime(0) {}
	DWORD	dwItemNo;
	int		iArticleNo;
	int		iEquipPos;
	bool	bStyle;

	//캐시 가차용 미리보기 정보
	BYTE	byTimeType;
	short	siUseTime;
}SPreviewItemInfo;

typedef enum eCashShopItemType
{
	ECSIT_BEGIN = 0,
	ECSIT_INSURANCE	= 0,
	ECSIT_RUNSTONE,
	ECSIT_DESTROY_SOULSTONE,
	ECSIT_HAMMER,
	ECSIT_DESTROY_STONE,
	ECSIT_INV_EXTENDER,
	ECSIT_RESURRECTION,
	ECSIT_GADA_COIN,
	ECSIT_SHOP_GRADE,
	ECSIT_SHOP_POINT,
	ECSIT_MEGAPHONE,
	ECSIT_SKILL_RESET,
	ECSIT_LUCKY_STONE,
	ECSIT_MISSION_CARD,
	ECSIT_SAFE_INV_EXTEND,
	ECSIT_END,
} E_CASHSHOP_ITEM_TYPE;

typedef std::vector<TBL_DEF_CASH_SHOP_CATEGORY>	CONT_DEF_CASH_SHOP_VEC;
typedef std::list< TBL_DEF_CASH_SHOP_ARTICLE > EQUIP_PREVIEW_CONT;
typedef std::list< SPreviewItemInfo > RECV_UI_EQUIP_PREVIEW_CONT;
typedef std::map< DWORD, DWORD > PREV_EQUIP_ITEM_CONT;
typedef std::vector< TBL_DEF_CASH_SHOP_ARTICLE > CONT_SHOPPING_BASKET;
typedef std::vector< int > CONT_SHOPPING_BASKET_SLOT_IDX;
typedef std::map< int, int > CONT_CS_SUB_TYPE_TO_ITEMNO;

typedef struct tagCashShopItemTypeToValue
{
	tagCashShopItemTypeToValue() : iTTW(0) { kCont.clear(); }
	int iTTW;
	CONT_CS_SUB_TYPE_TO_ITEMNO kCont;
}E_CASHSHOP_ITEM_TYPE_TO_VALUE;
typedef std::vector< E_CASHSHOP_ITEM_TYPE_TO_VALUE > CONT_CS_ITEMTYPE_TO_ITEMNO;

int const CASH_ITEM_PER_PAGE = 10;
int const CASH_GIFT_PER_PAGE = 5;
int const CASH_LIMITED_PER_PAGE = 4;
int const MAX_EQUIP_SLOT = 14;
int const MAX_PAGE_SLOT = 5;

class PgFurniture;
class PgCashShop
{
public:
	typedef enum tagECashShopType
	{
		ECASH_SHOP		= 0,
		EMILEAGE_SHOP	= 1,
		ECASHSHOP_GACHA	= 2,
		ECOSTUME_MIX	= 3,
	}ECashShopType;

	typedef enum tagECashShopCategory
	{
		ECSCATEGORY_NORMAL		= 0,
		ECSCATEGORY_GIFT		= 7,
	}ECashShopCategory;
	typedef enum tagECashShopCategoryNum
	{
		ECSCATEGORYNUM_BEGIN	= ECSCATEGORY_NORMAL,
		ECSCATEGORYNUM_END		= ECSCATEGORY_GIFT,
	}ECashShopCategoryNum;
	typedef enum tagECashShopForm
	{
		ECSFORM_COMMON		= ECSCATEGORY_NORMAL,
		ECSFORM_GIFT		= ECSCATEGORY_GIFT,
		ECSFORM_LIMITED,
		ECSFORM_UNKNOWN,
	}ECashShopForm;

public:
	PgCashShop();
	virtual ~PgCashShop();

	static bool IsPackegeArticle(TBL_DEF_CASH_SHOP_ARTICLE const& kArticle);
	static bool IsStyleItem(CItemDef const*& pDef);

	void OnBuild(BM::GUID const &kValueKey);
	void ParseXml();
	void Init();
	void InitMileage();
	// 대분류 탭을 눌렀을 때
	bool GetCategory(int const iCategory, TBL_DEF_CASH_SHOP_CATEGORY &rkCate);
	// 중분류 탭을 눌렀을 때
	bool GetSubategory(int const iCategory, TBL_DEF_CASH_SHOP_CATEGORY &rkCate);
	bool RecvCashShop_Command(WORD const wPacketType, BM::Stream &rkPacket);
	size_t GetItemByDispType(EQUIP_PREVIEW_CONT& rkOutputMap, ECashItemDisplayType const eType = CIDT_BEST);
	void InitPageArticle()	{m_kRecentPageArticle.clear();}
	void AddToPageArticle(TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle);
	bool FindPageArticle(int const iIndex, TBL_DEF_CASH_SHOP_ARTICLE &rkArticle);
	bool FindArticle(DWORD const dwItemNo, int& Idx);
	bool FindArticle(DWORD const dwItemNo, int& Idx, TBL_DEF_CASH_SHOP_ARTICLE& rkArticle) const;
	bool FindArticleByKey(TBL_KEY_INT kKey, TBL_DEF_CASH_SHOP_ARTICLE& rkArticleOut);
	bool SetLimitedArticle(const CONT_CASH_SHOP_ITEM_LIMITSELL& rContLimitedArticle);


	void SetGiftCont(CONT_CASHGIFTINFO const& rkCont);
	void GetGiftContRecv(CONT_CASHGIFTINFO& rkCont, int const iPage = 0) const;
	void GetGiftContSend(CONT_CASHGIFTINFO& rkCont, int const iPage = 0) const;
	size_t GetGiftRecvCount() const { return m_kGiftContRecv.size(); };
	size_t GetGiftSendCount() const { return m_kGiftContSend.size(); };
	void DeleteGift(BM::GUID const& rkGuid);
	void SetRankCont(CONT_CASHRANKINFO const& rkCont);
	bool GetItem(int const iIndex, TBL_DEF_CASH_SHOP_ARTICLE & rkArticle);
	bool GetLimitedInfo(CONT_CASH_SHOP_ITEM_LIMITSELL& rContLimitedOut) const;
	size_t GetLimitedSize(void) const { return m_kDefCashItemLimitSell.size(); }
	bool InitPreviewActor();
	int SetPreviewEquipItem(int const iItemNo) const;

	//장바구니
	E_CASHSHOP_RESULT AddNewArticleToBasket(int const iArticleIDX);
	void DelArticleToBasket(int const iSlotIDX, int const iArticleIDX);
	bool GetShoppingBasket(CONT_SHOPPING_BASKET& kBasket);
	bool GetShoppingBasket(CONT_SHOPPING_BASKET_SLOT_IDX& kIDX, EQUIP_PREVIEW_CONT& kItems);
	void UpDateShoppingBasketItemCost(int const iIdx, int const iArticleNo, TBL_DEF_CASH_SHOP_ITEM_PRICE const& kCost);
	bool ChangeShoppingBasketSlotIDXToArticleIDX(int const iSlotIDX, TBL_DEF_CASH_SHOP_ARTICLE &rkArticle);
	void ClearShoppingBasket();
	int  ShoppingBasketSize();
	__int64 const GetBasketItemTotalCost();
	void UpdateValidShoppingBasketList();

	int GetItemTypeToArticleNo(int const iItemType, int const SubKey);
	int GetItemTypeToItemNo(int const iItemType, int const SubKey);
	int GetItemTypeToSubKey(int const iItemType, int const CustomValue);
	int GetItemTypeToConfirmTTID(int const iItemType);

	//Actor
	void SetPreviewActor(SModelActorInfo const& kInfo){ m_kActorModel = kInfo; }
	PgActor* GetPreviewActor() const;

	void ClearPreviewEquip();
	__int64 GetPreviewItemTotalCost();
	void DelPreviewEquip(int const iItemNo);
	void GetPreviewItems(EQUIP_PREVIEW_CONT& kItems) const;
	void UpDatePreiewItemCost(int const iArticleNo, TBL_DEF_CASH_SHOP_ITEM_PRICE const& kCost);
	bool SetPreviewEquip(TBL_DEF_CASH_SHOP_ARTICLE const& Item, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont);
	bool ReSetPreviewEquip(int const iArticleNo, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont);

	//Object
	bool SetPreviewFurniture(TBL_DEF_CASH_SHOP_ARTICLE const& kArticle);
	void ClearPreviewFurniture();
	PgFurniture* GetPreviewFurniture(){ return m_pkItemFurniture; };

	int GetRecentTapCate()const{ return m_iRecentTopCategory; };
	int GetRecentSubCate()const{ return m_iRecentSubCategory; };
	
	bool isUseMileage() const { return m_bUseMileage;}
	void SetUseMileage(bool const bUse, XUI::CXUI_Wnd* const pkWnd);
	
	void SetRememberTotalPrice(__int64 i64TotalPrice) { m_i64TotalPrice = i64TotalPrice; }
	__int64 GetRememberTotalPrice() const { return m_i64TotalPrice; }

	bool SetShopType(ECashShopType const eType);
	ECashShopType GetShopType() const { return m_eShopType; }

	bool SetBuyTypeUI(ECashShopType const eType);
	ECashShopType GetBuyTypeUI() const { return m_eBuyTypeUI; }	

	bool SetShopSubType(ECashShopForm const eFormType); //캐쉬샵/마일리지샵의 하위 타입. 현재는 일반, 한정판매, 선물함이 있음.
	ECashShopForm GetShopSubType() const { return m_eShopSubType;}

	void SetRememberItemMileage(__int64 i64ItemMileage) { m_i64ItemMileage = i64ItemMileage; }
	__int64 GetRememberItemMileage() const { return m_i64ItemMileage; }
	
	__int64 const CalcAddBonus(__int64 const i64Cost,int const iMileage) const;
	__int64 const CalcLocalAddBonus(__int64 const i64Cost);

	static bool BestItem_Greater(TBL_DEF_CASH_SHOP_ARTICLE const& pLeft, TBL_DEF_CASH_SHOP_ARTICLE const& pRight);
	static bool ArticleCostDeleteToOneSave(CONT_CASH_SHOP_ITEM_PRICE& kPrice);

	int GetItemPrice(int const iItemNo, BYTE bTimeType, int iUseTime, bool bDiscount);
protected:
	bool SetPreviewFurnitureModelProperty(NiNode* pkNode);
	bool SetPreviewFurnitureModelPropertyReal(NiNode* pkNode);
	bool DetachPreviewFurnitureModelPhysics(NiNode* pNode);
	void GetPreviewEquipItemList(bool const bSetterIsCall, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont) const;

private:
	void SortShopArticlesByCategory(CONT_DEF_CASH_SHOP& kContShop, CONT_DEF_CASH_SHOP_VEC& kContShopData_out);

private:
	CONT_DEF_CASH_SHOP m_kDefCashShop;
	//CONT_DEF_CASH_SHOP const *m_pkDefMileageShop;
	CONT_DEF_CASH_SHOP_ARTICLE m_kDefCashShopArticle;
	CONT_CASH_SHOP_ITEM_LIMITSELL m_kDefCashItemLimitSell;
	
	CONT_DEF_CASH_SHOP_VEC m_kCashShopData;
	CONT_DEF_CASH_SHOP_VEC m_kMileageShopData;
	CONT_DEF_CASH_SHOP_VEC m_kCSLimitedData; //캐쉬샵 전용 아티클을 담는다.

	int m_iRecentTopCategory;	//최근 열어본 최상위 분류
	int m_iRecentSubCategory;	//최근 열어본 중간 분류

	EQUIP_PREVIEW_CONT m_kPreviewEqItem;	//입어보기 선택한 목록들 - 위치가 같은 곳은 이전걸 빼고
	EQUIP_PREVIEW_CONT m_kBestItem;

	TBL_DEF_CASH_SHOP_CATEGORY m_kRecentTopCategory;	//매번 얻어오지말고 복사해 놓고 쓰자
	TBL_DEF_CASH_SHOP_CATEGORY m_kRecentSubCategory;
	
	CONT_DEF_CASH_SHOP_ARTICLE m_kRecentPageArticle;	//현재 페이지에 보여지고 있는 물품들 모으자
	CONT_CASHGIFTINFO m_kGiftContRecv;
	CONT_CASHGIFTINFO m_kGiftContSend;
	CONT_CASHRANKINFO m_kRankCont;
	PREV_EQUIP_ITEM_CONT	m_kDefaultItemCont;
	PREV_EQUIP_ITEM_CONT	m_kStyleItemCont;
	CONT_SHOPPING_BASKET	m_kShoppingBasket;

	CLASS_DECLARATION_S(SItemPos, RenewItemPos);
	CLASS_DECLARATION_S(BM::GUID, ValueKey);

	bool m_bUseMileage;
	ECashShopType m_eShopType;
	ECashShopType m_eBuyTypeUI;
	ECashShopForm m_eShopSubType;

	__int64 m_i64ItemMileage;
	__int64 m_i64TotalPrice;
	TBL_DEF_CASH_SHOP_ARTICLE m_kPreviewFurniture;
	PgFurniture* m_pkItemFurniture;

	SModelActorInfo	m_kActorModel;

	static CONT_CS_ITEMTYPE_TO_ITEMNO m_kItemTypeToItemNo;

	CLASS_DECLARATION_S(int, StaticItemType)
	CLASS_DECLARATION_S(int, StaticItemSubKey)
	CLASS_DECLARATION_S(int, StaticItemNo)
};

#define	g_kCashShopMgr	SINGLETON_STATIC(PgCashShop)

#endif // FREEDOM_DRAGONICA_CONTENTS_CASHSHOP_PGCASHSHOP_H