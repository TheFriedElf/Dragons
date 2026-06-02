#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMSOCKETSYSTEM_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMSOCKETSYSTEM_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class PgUIModel;

class lwUIItemSocketSystem
{
public:
	lwUIItemSocketSystem(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void DisplayNeedItemIcon(int iIndex);
	void DisplayResultItem();
	void ClearUpgradeData();
	bool SendReqSocketSystem(bool bIsTrueSend);
	int GetUpgradeCreateNeedMoney();
	int GetUpgradeRemoveNeedMoney();
	int GetUpgradeResetNeedMoney();
	int GetUpgradeExtractionNeedMoney();
	void CallComfirmMessageBox( bool const bIsModal = true );
	void Clear(bool const bClearAll);
	int CheckOK();	//클라에서 먼저 한번 체크해 주자
	int const GetNowNeedItemCount(int const iNeed) const;
	void OnDisplay();
	void OnTick(lwUIWnd kWnd);
	bool InProgress();
	void SetAttachElement(int iType);
	lwWString GetExplane();
	void SetLockSlot(int const iIndex);
	void SetMaterialItem(int iType, bool bCreate, bool bNoBuyMsg = true);
	int	GetSoulCreateCount();
	int GetSoulRemoveCount();	
	int GetSoulResetCount();
	int GetExtractionCount();
	bool GetSocketSystemEmpty();
	bool IsSocketItemEmpty() const;
	bool IsInsureItemPos() const;
	bool IsProbabilityItemPos() const;
	void CallSocketItem();
	void SetMenuType(int iMenu);
	int GetMenuType();
	void CheckMenuSocketItem();
	int GetMenuTypeOrder();
	int GetSelectOrderMenu();
	void SetSelectOrderMenu(int iSelectOrderMenu);
	bool GetSelectOrderMenuCheck(int iSelectMenu);
	bool IsSelectOrderMenuIndex(int iIndex);
	void DisplaySocketRemoveText(int iIndex);
	void SetSocketItemMenuUIUpdate();

	void lwSetMateriaItemExtraction(int iCount,int iCashBtnRegCount);
	int GetHaveCount();
	void SetSoulText();
	
protected:
	XUI::CXUI_Wnd *self;
};

typedef struct tagNeedItemSocketSystem
{
	int iIndex;
	int iItemNo;
	int iNeedNum;
	int iNowNum;
	tagNeedItemSocketSystem()
	{
		Init();
	}

	void Init()
	{
		iIndex = 0; 
		iItemNo = 0;
		iNeedNum = 0;
		iNowNum = 0;
	}

	void SetInfo(int iIdx, int iItmNo, int iNeed, int iNow)
	{
		iIndex = iIdx; 
		iItemNo = iItmNo;
		iNeedNum = iNeed;
		iNowNum = iNow;
	}

	bool IsOK()
	{
		bool bOk = false;
		switch(iIndex)
		{
		case 0:	//소울
			{
				bOk = iNowNum >= iNeedNum;
			}break;
		case 2:
			{
				bOk = true;
			}
			break;
		case 1:	//확률
			{
				bOk = iNowNum >= iNeedNum;
			}break;
		case 3:	//파괴석
			{
				bOk = iNowNum >= iNeedNum;
			}
		    break;
		}

		return bOk;
	}

}SNeedItemSocketSystem;

namespace PgMonsterCardUtil
{
	typedef enum eMonsterCardItemType
	{
		MCIT_NONE = 0,
		MCIT_SUCCESS = 1,
		MCIT_FAIL = 2,
		MCIT_EQUIP_LIMIT = 3,
	}EMonsterCardItemType;

	bool IsMonsterCardSocketState(SEnchantInfo const kEhtInfo, int const iOrderIndex);
	EMonsterCardItemType ItemMonsterCardAdd(XUI::CXUI_Wnd* pWnd, PgBase_Item const* pkItem, SItemPos rkPos, int const iEquipLimit, int const iOrderIndex, int const iItemLevelMin, int const iItemLevelMax);
	void MonsterCardListUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos);
};

class PgItemSocketSystemMgr
{
public:
	enum E_SOCKET_CHECK_ERR
	{
		SCE_OK		= 0,
		SCE_FALSE	= 1,
		SCE_CURSE,
		SCE_SEAL,
		SCE_GOD,
	};

	static HRESULT CheckSocketBundle(PgBase_Item const& kItem);

	PgItemSocketSystemMgr();
	virtual ~PgItemSocketSystemMgr(){ m_pkWndUIModel = 0; m_pkWndUIModel_Result = 0;}

	typedef enum eSocketItemType
	{
		SIT_SOUL = 0,
		SIT_PROBABILITY,
		SIT_SET_ELEMENT,
		SIT_INSUR_ITEM,
		SIT_EXTRACTION,
	}ESocketItemType;

	typedef enum eSocketItemMenu
	{
		SIM_CREATE = 1,
		SIM_REMOVE,
		SIM_RESET,
		SIM_EXTRACTION,
	}ESocketItemMenu;

public:
	void Clear(bool const bAllClear = false);
	void InitUIModel();
	int CallComfirmMessageBox( bool const bIsModal = true );
	int GetCreateSocketItemOrder();
	int GetRemoveSocketItemOrder();
	int GetResetSocketItemOrder();
	int GetExtractionSocketItemOrder();
	bool GetSelectOrderMenuCheck(int iSelectMenu);
	__int64 GetUpgradeCreateNeedMoney();
	__int64 GetUpgradeRemoveNeedMoney();
	__int64 GetUpgradeResetNeedMoney();
	__int64 GetUpgradeExtractionNeedMoney();
	bool GetSocketSystemEmpty();
	void DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd);

	bool SendReqSocketSystem(bool bIsTrueSend);
	int CheckNeedItem();
	int const GetNowNeedItemCount(int const iNeed) const;
	PgBase_Item const GetResultItem() const { return m_kResultItem; } 
	SItemPos const& GetSrcItemPos() const { return m_kSrcItemPos; }
	bool IsChangedGuid() const;
	//bool IsInProgress();
	void RunProgressEffect(bool bOn = true);
	void OnDisplay();
	void OnTick(POINT2 kPt);
	void SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo);
	void SetMaterialItem(int iType, SItemPos kItemPos, bool bCreate);
	void SetMaterialItem(XUI::CXUI_Wnd* pWnd, int iType, bool bCreate, bool bNoBuyMsg = true);
	void SetSrcMaterialBtnInit(XUI::CXUI_Wnd* pWnd, int const iType, bool const bVisible = true);
	int GetSocketNeedItemNo();
	int GetMenuTypeOrder();

	BM::vstring GetExplane();	//설명
	int	GetSoulCreateCount();
	int GetSoulRemoveCount();
	int GetSoulResetCount();
	int GetExtractionCount();

	CLASS_DECLARATION_S(bool, InProgress);
	CLASS_DECLARATION_S(float, StartTime);
	CLASS_DECLARATION_S(EPropertyType, AttachElement);	//붙일 속성
	CLASS_DECLARATION_S(EPropertyType, MyElement);		//붙일 속성
	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
	
	void RecentResult(EItemSocketSystemResult const & eResult);
	EItemSocketSystemResult const RecentResult() { return m_RecentResult; }

	int GetSocketSystemErrorMsg(HRESULT hResult);
	bool IsSocketItemEmpty() const;
	bool IsInsureItemPos() const;
	bool IsProbabilityItemPos() const;
	void UpdateSoulItemView();
	void CheckMenuSocketItem();
	void SetMenuType(int iMenu);
	int GetMenuType();
	bool DisplayNeedItemText(int const dwItemNo);
	void SetSelectOrderMenu(int iSelectOrderMenu);
	int GetSelectOrderMenu();
	bool IsSelectOrderMenuIndex(int iIndex);
	void DisplaySocketRemoveText(int iIndex);
	void SetSocketItemMenuUIUpdate();

	void ExtractionNeedItem();//소켓추출 인게임 아이템 필요한 값 셋팅
	void SetExtractionCashItem(int iHaveCashitemCountPlus,int iCashBtnRegCount);
	void SetMaterialExtractionBtnInit(XUI::CXUI_Wnd* pWnd,int iCount,int iCashBtnRegCount);
	int ExtractionCheckCard(); //소켓 추출 몬스터카드 번호 체크
	int GetHaveCount();
	void SetSoulText();
	
	
protected:
	BM::GUID m_guidSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	SItemPos m_kSrcItemPos;
	SItemPos m_kInsureItemPos;
	SItemPos m_kProbabilityItemPos;

	PgBase_Item	m_kResultItem;	
	PgBase_Item	m_kItem;

	PgUIModel	*m_pkWndUIModel;		//회오리 이펙트용
	PgUIModel	*m_pkWndUIModel_Result;	//결과 이펙트용
	EItemSocketSystemResult m_RecentResult;
	
	SNeedItemSocketSystem m_kItemArray[4];
	BM::vstring	m_kExplane;
	int		m_iSoulCreateCount;
	int		m_iSoulRemoveCount;
	int		m_iSoulResetCount;
	int		m_iSocket_CreateOrder;
	int		m_iSocket_RemoveOrder;
	int		m_iSocket_ResetOrder;
	int		m_iSocket_ExtractionOrder;
	int		m_iMenu;
	int		m_iSelectOrderMenu;
	int		m_iExtractionInGameItemNameNum; //필요한 추출 ingame 필요한 아이템 번호
	int   	m_iNeedItemExtractionCount; //필요한 추출 아이템 수량
	int		m_iNeedItemExtractionCountTemp;
	int   	m_iNeedCashItemExtractionCount; //필요한 추출 캐쉬 아이템 수량
	int		m_iHaveCashItemExtractionNum;//필요한 추출 캐쉬 아이템 번호
	int		m_iUserHaveCashItemCount; //유저가 누른 캐쉬 아이템 횟수 0부터 시작~
	int		m_iHaveItemCount;
	int		m_iHaveIngameItemCount;
	int		m_iCardItemNo;	//몬스터 카드 번호.

	bool SetElementInfo(int const iIndex, PgBase_Item const & rkSItem, PgInventory *pInv, SNeedItemSocketSystem* pkArray);
	bool SetSrcItem(const SItemPos &rkItemPos);
	bool SetMaterialItem(EKindUIIconGroup const kGroup, SItemPos const& rkItemPos);
	void MakeExplane(SNeedItemSocketSystem const * pkArray );
	bool IsOnlyUseSoul()	{ return (AttachElement()==E_PPTY_NONE || AttachElement()==E_PPTY_CURSE); }	
};

#define g_kSocketSystemMgr SINGLETON_STATIC(PgItemSocketSystemMgr)

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMSOCKETSYSTEM_H