#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMPLUSUPGRADE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMPLUSUPGRADE_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

typedef enum : BYTE
{
	ENEEDIO_DUST		= 0,
	ENEEDIO_INSURANCE	= 1,
	ENEEDIO_PROBABILITY,
	ENEEDIO_END,
}ENEEDITEM_ORDER;

class lwUIItemPlusUpgrade
{
public:
	lwUIItemPlusUpgrade(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void DisplayNeedItemIcon();
	void DisplayResultItem();
	void SetMaterialItem(bool bNoBuyMsg = true);
	void SetProbabilityMinus();
	void ClearUpgradeData();
	void ReSetUpgradeData();
	bool SendReqPlusUpgrade(bool bIsTrueSend);
	int GetUpgradeNeedMoney();
	void CallComfirmMessageBox( const bool bIsModal = true );
	void Clear(bool const bAllClear);
	int CheckOK();	//클라에서 먼저 한번 체크해 주자
	int const GetNowNeedItemCount(int const iNeed) const;
	void CallCheckInsureItem();
	void ResultProcess();
	
protected:
	XUI::CXUI_Wnd *self;
};

typedef struct tagNeedItemPlusUpgrade
{
	int iIndex;
	int iItemNo;
	int iNeedNum;
	int iNowNum;

	tagNeedItemPlusUpgrade();
	void Init();
	void SetInfo(int iIdx, int iItmNo, int iNeed, int iNow);
	bool IsOK();	//재련 가능 상태?
}SNeedItemPlusUpgrade;

class PgItemPlusUpgradeMgr
{
public:
	PgItemPlusUpgradeMgr();
	virtual ~PgItemPlusUpgradeMgr(){}

public:
	void Clear(bool const bAllClear = false);
	int CallComfirmMessageBox( const bool bIsModal = true );
	__int64 GetUpgradeNeedMoney() const;
	bool SetSrcItem(const SItemPos &rkItemPos);
	void DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd);
	void SetMaterialItem(XUI::CXUI_Wnd* pWnd, bool bNoBuyMsg = true);
	void SetMaterialItem(ENEEDITEM_ORDER const Type, SItemPos kItemPos, bool bIsSlotUpdate = false);
	void SetProbAbilityMinus();
	void SetSrcMaterialBtnInit(XUI::CXUI_Wnd* pWnd, bool const bVisible = true);
	void SetSrcMaterialBtnTrue(XUI::CXUI_Wnd* pWnd, bool const bVisible = true);
	const TBL_DEF_ITEMPLUSUPGRADE* GetPlusInfo(SItemPlusUpgradeKey const &rkKey);
	const TBL_DEF_ITEMPLUSUPGRADE* GetPlusInfo();//렙업 할 정보가 나온다.
	bool SendReqPlusUpgrade(bool bIsTrueSend);
	int CheckNeedItem();
	int const GetNowNeedItemCount(int const iNeed) const;
	PgBase_Item const GetResultItem() const { return m_kResultItem; } 
	SItemPos const& GetSrcItemPos() const { return m_kSrcItemPos; }
	bool IsChangedGuid() const;
	bool IsInProgress();
	void SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo);
	bool SetMaterialItem(EKindUIIconGroup const kGroup, const SItemPos& rkItemPos);
	void SetResultItem(PgBase_Item const & rkResultItem);

	SItemPos const& GetInsureItemPos() const { return m_kInsureItemPos; };
	SItemPos const& GetProbabilityItemPos() const { return m_kProbabilityItemPos; };
	int const& GetProbAbilityNo() const { return m_iProbAbilityNo; };

	int const GetUseProbabilityItemCount() const { return m_iProbabilityUseCount; };

	void RecentResult(EPlusItemUpgradeResult const & eResult) { m_RecentResult = eResult; }
	void ResultProcess();

	CLASS_DECLARATION_S(DWORD, InsureItemNo);
protected:
	BM::GUID m_guidSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	SItemPos m_kSrcItemPos;
	int m_iProbAbilityNo;
	SItemPos	m_kInsureItemPos;
	SItemPos	m_kProbabilityItemPos;
	short		m_iProbabilityUseCount;
	EPlusItemUpgradeResult m_RecentResult;

	PgBase_Item	m_kItem;
	PgBase_Item	m_kResultItem;	

	SNeedItemPlusUpgrade m_kItemArray[MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY+1];
	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
};

#define g_kItemPlusUpgradeMgr SINGLETON_STATIC(PgItemPlusUpgradeMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMPLUSUPGRADE_H