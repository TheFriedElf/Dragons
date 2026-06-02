#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYUPGRADE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYUPGRADE_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class PgUIModel;

class lwUIItemRarityUpgrade
{
public:
	lwUIItemRarityUpgrade(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void DisplayNeedItemIcon(int iIndex);
	void DisplayResultItem();
	void ClearUpgradeData();
	bool SendReqRarityUpgrade(bool bIsTrueSend);
	int GetUpgradeNeedMoney();
	void CallComfirmMessageBox( const bool bIsModal = true );
	void Clear(bool const bClearAll);
	int CheckOK();	//클라에서 먼저 한번 체크해 주자
	int const GetNowNeedItemCount(int const iNeed) const;
	void OnDisplay();
	void OnTick(lwUIWnd kWnd);
	bool InProgress();
	void SetAttachElement(int iType);
	lwWString GetExplane();
	void SetLockSlot(int const iIndex);
	void SetMaterialItem(int iType, bool bNoBuyMsg = true);
	void ReSetUpgradeData();
	static void SetExplaneText();
	void CallCheckInsureItem();
	void ResultProcess();
protected:
	XUI::CXUI_Wnd *self;
};

typedef struct tagNeedItemRarityUpgrade
{
	int iIndex;
	int iItemNo;
	int iNeedNum;
	int iNowNum;
	tagNeedItemRarityUpgrade()
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

	bool IsOK(bool bOnlyRarity)	//재련 가능 상태?
	{
		bool bOk = false;
		switch(iIndex)
		{
		case 0:	//환혼
			{
				bOk = iNowNum >= iNeedNum;
			}break;
		case 2:	//넣을 속성
			{
				bOk = iNowNum >= iNeedNum || bOnlyRarity;
			}
			break;
		case 1:	//확률
		case 3:	//보험
			{
				bOk = true;
			}
		    break;
		}

		return bOk;
	}

}SNeedItemRarityUpgrade;


class PgItemRarityUpgradeMgr
{
public:
	enum E_RARITY_CHECK_ERR
	{
		RCE_OK		= 0,
		RCE_FALSE	= 1,
		RCE_SEAL,
		RCE_CURSE,
		RCE_GOD,
	};

	static HRESULT CheckRarityBundle(PgBase_Item const& kItem, EPropertyType const eAttachElement = E_PPTY_NONE);

	PgItemRarityUpgradeMgr();
	virtual ~PgItemRarityUpgradeMgr(){ m_pkWndUIModel = 0; m_pkWndUIModel_Result = 0;}

	typedef enum eRarityItemType
	{
		RIT_SOUL = 0,
		RIT_PROBABILITY,
		RIT_SET_ELEMENT,
		RIT_INSUR_ITEM,
	}ERarityItemType;

public:
	void Clear(bool const bAllClear = false);
	void InitUIModel();
	int CallComfirmMessageBox( const bool bIsModal = true );
	__int64 GetUpgradeNeedMoney();
	void DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd);

	bool SendReqRarityUpgrade(bool bIsTrueSend);
	int CheckNeedItem();
	int const GetNowNeedItemCount(int const iNeed) const;
	PgBase_Item const GetResultItem() const { return m_kResultItem; } 
	SItemPos const& GetSrcItemPos() const { return m_kSrcItemPos; }
	SItemPos const& GetInsureItemPos() const { return m_kInsureItemPos; };
	SItemPos const& GetProbabilityItemPos() const { return m_kProbabilityItemPos; };
	bool IsChangedGuid() const;
	//bool IsInProgress();
	void RunProgressEffect(bool bOn = true);
	void OnDisplay();
	void OnTick(POINT2 kPt);
	void SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo);
	void SetMaterialItem(XUI::CXUI_Wnd* pWnd, int iType, bool bNoBuyMsg = true);
	void SetMaterialItem(ERarityItemType const eType, SItemPos kItemPos, bool bIsSlotUpdate = false);
	void SetSrcMaterialBtnInit(XUI::CXUI_Wnd* pWnd, int const iType, bool const bVisible = true);
	void ResultProcess();

	BM::vstring GetExplane();	//설명

	CLASS_DECLARATION_S(bool, InProgress);
	CLASS_DECLARATION_S(float, StartTime);
	CLASS_DECLARATION_S(EPropertyType, AttachElement);	//붙일 속성
	CLASS_DECLARATION_S(EPropertyType, MyElement);		//붙일 속성
	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
	
	void RecentResult(EItemRarityUpgradeResult const & eResult);
	EItemRarityUpgradeResult const RecentResult() { return m_RecentResult; }

	CLASS_DECLARATION_S(DWORD, InsureItemNo);
	
	static E_ITEM_GRADE GetGradeOfModifiedOption(PgBase_Item const& pastItem, PgBase_Item const& nowItem);	//이전과 변경된 옵션 등급. 소울등급1부터 순서대로 검사함.
protected:
	BM::GUID m_guidSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	SItemPos m_kSrcItemPos;
	SItemPos m_kInsureItemPos;
	SItemPos m_kProbabilityItemPos;

	PgBase_Item	m_kResultItem;	
	PgBase_Item	m_kPastResultItem;		//소울하기 전의 옵션과 비교용
	PgBase_Item	m_kItem;

	PgUIModel	*m_pkWndUIModel;		//회오리 이펙트용
	PgUIModel	*m_pkWndUIModel_Result;	//결과 이펙트용
	EItemRarityUpgradeResult m_RecentResult;
	
	SNeedItemRarityUpgrade m_kItemArray[4];
	BM::vstring	m_kExplane;

	bool SetElementInfo(int const iIndex, PgBase_Item const & rkSItem, PgInventory *pInv, SNeedItemRarityUpgrade* pkArray);
	bool SetSrcItem(const SItemPos &rkItemPos);
	bool SetMaterialItem(EKindUIIconGroup const kGroup, SItemPos const& rkItemPos);
	void MakeExplane(SNeedItemRarityUpgrade const * pkArray );
	bool IsOnlyUseSoul()	{ return (AttachElement()==E_PPTY_NONE || AttachElement()==E_PPTY_CURSE); }
};

#define g_kItemRarityUpgradeMgr SINGLETON_STATIC(PgItemRarityUpgradeMgr)
#endif //FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYUPGRADE_H