#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGEMSTORE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGEMSTORE_H
#include "lwUI.h"
#include "lwGUID.h"
#include "lwPacket.h"
#include "PgScripting.h"
#include "PgPage.h"
#include <deque>

namespace GEM_STORE
{
	class SDefGems;
}

class PgGemStore
{
public:
	enum E_CHECK_ITEM_RESULT
	{
		EIR_FAIL = -1,
		EIR_OK = 0,
		EIR_LACK_CP,
		EIR_LACK_JEWEL,
		EIR_LACK_HERO_MARK,
	};

	typedef struct tagDefGemItemInfo
	{
		int				iItemNo;
		int				iOrderIndex;
		tagDefGemItemInfo()
		{
			Init();
		}
		void Init()
		{
			iItemNo = 0;
			iOrderIndex = 0;
		}
	}SDefGemItemInfo;	
	typedef std::vector<SDefGemItemInfo> VEC_GEMITEM_INFO;

	PgGemStore();
	virtual ~PgGemStore();
		
	void SendReqGemStoreInfo(BM::GUID& rkNpcGuid);	// 상점정보를 요청하고	
	void SendReqEventStoreInfo(BM::GUID& rkNpcGuid);	// 상점정보를 요청하고	
	void RecvGemstoreInfo(BM::Stream& rkPacket);	// 상정정보를 받아 UI를 구성하고
	void ClickSlotItem(lwUIWnd& kListItem);			// UI내의 아이템을 클릭하면, 해당 표현을 하고	
	bool SendReqGemTrade();							// 선택한 아이템의 교환 정보를 보낸 후
	bool SendReqGemTradeTree();
	void RecvGemTradeResult(BM::Stream& rkPacket); // 에러 결과가 있다면 처리 한다

	void BeginPage(lwUIWnd kSelf);
	void EndPage(lwUIWnd kSelf);
	void JumpPrevPage(lwUIWnd kSelf);
	void JumpNextPage(lwUIWnd kSelf);
	void Page(lwUIWnd kSelf);

	void ItemTreeClear();
	void Clear();
	void DrawIconImage(lwUIWnd& UISelf);
	void DrawItemTooltip(lwUIWnd& UISelf);

	PgPage const& GetPage() const { return m_kPage; }
	int GetSelectedItemNo() const { return m_iSelectedItemNo; }
	int GetOrderIndex() const { return m_iOrderIndex; }
	
	void ClearNeedItemImg(XUI::CXUI_Wnd* pMainWnd);
	bool IsItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex)const;
	void SetItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex);
	void UpdateItemTree();



	void SendReqDefGemStoreInfo(BM::GUID& rkNpcGuid, int const iMenu, bool const bCheckClass);	// 상점정보를 요청하고	
	void RecvDefGemstoreInfo(BM::Stream& rkPacket);	
	bool IsOtherMode() {return (EGT_OTHER == m_eGemStoreType);};
	bool IsEventMode() {return (EGT_EVENT == m_eGemStoreType);};
	void SetGemStoreType(E_GEMSTORE_TYPE const eType) { m_eGemStoreType = eType; }
	E_GEMSTORE_TYPE GetGemStoreType()const { return m_eGemStoreType; }
	void ClickDefSlotItem(lwUIWnd& kListItem);
	void RecvDefGemTradeResult(BM::Stream& rkPacket); // 에러 결과가 있다면 처리 한다
	int GetGemStoreTitleMenu();
	void ResetCursor(lwUIWnd& kSelf);

protected:
	XUI::CXUI_Wnd* CallGemStoreUIandGetListWnd();
	XUI::CXUI_Wnd* CallCPStoreUIandGetListWnd();
	bool CheckMyClassItem(int const iItemNo);
	bool SetStoreSlot(XUI::CXUI_Wnd* pkMain);
	bool SetSlotItem(XUI::CXUI_Wnd* pkIcon, int const iItemNo);
	int GetAmountItemPlayerHave(int const iItemNo);
	int GetItemPlayerHave(int const iItemNo);
	E_CHECK_ITEM_RESULT isEnoughTrade(int const iSelectedItemNo,int const iOrderIndex);
	bool SendReqGemTrade(int const iSelectedItemNo,int const iOrderIndex);


	bool SetDefStoreSlot(XUI::CXUI_Wnd* pkMain);
	bool SetDefSlotItem(XUI::CXUI_Wnd* pkIcon, int const iItemNo);
	E_CHECK_ITEM_RESULT isDefEnoughTrade(int const iSelectedItemNo,int const iOrderIndex);


	/////////////////////////////////////////아이템 트리
	void ItemTreeSetCustomData(XUI::CXUI_Wnd* pJewelWnd,XUI::CXUI_Icon* pIconJewel,int iItemNo,int iNeedItemNo, int const iOrderIndex);
	void DrawItemTree(XUI::CXUI_Wnd* pMainWnd, int const Row, GEM_STORE::SDefGems const& kSlotItem);
	void UpdateItemTreeHistory(XUI::CXUI_Wnd * pMainWnd);

protected:
	CONT_GEMSTORE_ARTICLE m_kContAtricle;
	int m_iEnoughCnt;
	BM::GUID m_kNpcGuid;
	int m_iSelectedItemNo;
	PgPage m_kPage;
	bool m_bIsCPStore;
	int m_iOrderIndex;
	bool m_bCheckClass;

	// 아이템 트리 ////////////////////////////////////////////////////////////////////////////////
	typedef std::deque<std::pair<int,int> > CONT_TREE_HISTORY;	//first: ItemNo, second: OrderIndex

	int m_iTreeItemNo;
	int m_iTreeOrderIndex;
	int m_iConstellationSlotCount;
	int m_iConstellationSlotNo;
	std::vector<int> m_vConstellationSlotOrderIndex;
	std::vector<int> m_vConstellationSlotNo;
	int m_iUpTreeSelectedItemNo;
	int m_iUpTreeSeletedOrederIndex;
	CONT_TREE_HISTORY m_kContTreeHistory;
	///////////////////////////////////////////////////////////////////////////////////////////////

	mutable Loki::Mutex m_kMutex;
	E_GEMSTORE_TYPE	m_eGemStoreType;
	CONT_DEFGEMSTORE_ARTICLE m_kDefContAtricle;
};

#define g_kGemStore SINGLETON_STATIC(PgGemStore)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGEMSTORE_H