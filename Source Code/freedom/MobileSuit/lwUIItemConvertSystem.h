#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCONVERTSYSTEM_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCONVERTSYSTEM_H

#include "PgScripting.h"
#include "lwGUID.h"
#include "lwWString.h"
#include "lwPacket.h"
#include "lwPoint2.h"
#include "lwPoint3.h"
#include "lwActor.h"
#include "XUI/XUI_Manager.h"

class PgUIModel;

class lwUIItemConvertSystem
{
public:
	lwUIItemConvertSystem(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void Clear(bool const bClearAll);
	bool SetMaterialItem();
	bool InProgress();
	void SetInProgress();
	void OnItemConvertCallInit();
	void OnItemConvertInit(int iCount);
	void SendItemConvert();
	bool PreCheckItemConvert(int const iCount);
	int ItemConvertMaxCount();

protected:
	XUI::CXUI_Wnd *self;
};

class PgItemConvertSystemMgr
{
public:
	PgItemConvertSystemMgr();
	virtual ~PgItemConvertSystemMgr(){}

public:
	void Clear(bool const bAllClear = false);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd);
	bool SetMaterialItem(XUI::CXUI_Wnd* pWnd);
	void OnItemConvertInit(XUI::CXUI_Wnd* pWnd, int iCount, bool bInit);
	void UpdateUI_Init(XUI::CXUI_Wnd* pWnd, int const iFirstItemNo, int const iSecondItemNo);
	bool PreCheckItemConvert(int const iCount);
	int ItemConvertMaxCount();

	SItemPos const& GetSrcItemPos() const { return m_kSrcItemPos; }
	bool SetItem(SIconInfo const & rkInfo);
	int GetItemNo();
	void SendItemConvert();
	int GetItemFirstNo();
	int GetItemFirstCount();
	int GetItemSecondNo();
	int GetItemSecondCount();

	CLASS_DECLARATION_S(bool, InProgress);
	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
protected:
	PgBase_Item	m_kItem;
	SItemPos m_kSrcItemPos;
	int		iFirstItemNo;
	int		iSecondItemNo;

	int		m_iItemTotalSourceCount;
	int		m_iItemFirstCount;
	int		m_iItemSecondCount;
	int		m_iItemCount;

	bool SetSrcItem(const SItemPos &rkItemPos);
	bool SetMaterialItem(EKindUIIconGroup const kGroup, SItemPos const& rkItemPos);		
};

#define g_kItemConvertSystemMgr SINGLETON_STATIC(PgItemConvertSystemMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCONVERTSYSTEM_H