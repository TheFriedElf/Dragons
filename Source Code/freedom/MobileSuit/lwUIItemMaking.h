#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMMAKING_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMMAKING_H

#include "XUI/XUI_Manager.h"
#include "Variant/ItemMakingDefMgr.h"

class lwUIItemMaking
{
public:
	lwUIItemMaking(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

	void RefreshTodoList();
	void DisplaySrcItem();
	void DisplayDestItem();
	void DisplaySrcList();
	void DisplayDestList();	
	void Clear();
	void ClearItemCount();
	bool SetItemCount(lwWString kItemCount);
	void SelectTodoList();
	void SelectDestList();
	int GetNeedMoney();
	void MakingItem();
//	int GetCurTitleTextNo();

protected:
	XUI::CXUI_Wnd *self;
};

class PgItemMaking
{
public:
	typedef struct tagItemMaking_ItemData
	{
		tagItemMaking_ItemData()
		{
			iBuildIndex = 0;
			iItemCount = 0;
			kItemPos.Clear();
		}
		int iBuildIndex;
		int iItemCount;
		SItemPos kItemPos;
		PgBase_Item kItem;
	}SItemMaking_ItemData;

public:
	typedef std::map< int, SItemMaking_ItemData >	CONT_ITEMMAKING_ITEM;

protected:
	PgBase_Item const* GetMouseOverItem(XUI::CXUI_Icon *pIcon, const CONT_ITEMMAKING_ITEM& rkItemPos)const;

public:
	PgItemMaking();
	virtual ~PgItemMaking();

	void Clear();
	HRESULT PgItemMaking::IsCorrectItem(SItemPos const& rkItemPos, PgBase_Item &kOutItem)const;
	bool SetSrcItem(const SItemPos &rkItemPos, int iBuildIndex);
	void DisplayItem(XUI::CXUI_Wnd *pWnd, const CONT_ITEMMAKING_ITEM& rkItemPos)const;
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd)const;
	void DisplayDestItem(XUI::CXUI_Wnd *pWnd)const;
	void DisplaySrcList(XUI::CXUI_Wnd *pWnd)const;
	void DisplayDestList(XUI::CXUI_Wnd *pWnd)const;
	void ClearItemCount();
	bool SetItemCount(XUI::CXUI_Wnd *pWnd, int iItemCount);
	void SelectTodoList(int iMakingNo);
	void SelectDestList(XUI::CXUI_Wnd *pWnd);
	void MakingItem()const;
	int GetNeedMoney()const;
	//int GetCurTitleTextNo()const;
	PgBase_Item const* GetMouseOverSrcItem(XUI::CXUI_Icon *pIcon)const;
	PgBase_Item const* GetMouseOverDestItem(XUI::CXUI_Icon *pIcon)const;

	// 모든 공통 Result
	void Recv_PT_M_C_NFY_ITEM_MAKING_RESULT(BM::Stream* pkPacket);

protected:
	// 현재 제조에 관한..
	CLASS_DECLARATION_S( int, CurMakingNo );
	SDefItemMakingData m_kCurMakingData;

	// Slot에 관한..
	CONT_ITEMMAKING_ITEM m_kSrcItemPos;				// 직접적으로 아이템을 올림.
	CONT_ITEMMAKING_ITEM m_kDestItemPos;			// 아웃풋될 아이템을 보여줌. (관상용)
	SItemMaking_ItemData* m_pkCurItemData;
	CLASS_DECLARATION_S( int, CurDestListIndex );
};

#define g_kItemMaking SINGLETON_STATIC(PgItemMaking)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMMAKING_H