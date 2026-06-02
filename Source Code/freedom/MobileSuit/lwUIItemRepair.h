#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMREPAIR_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMREPAIR_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class lwUIItemRepair
{
public:
	lwUIItemRepair(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void Clear(bool const bAllClear);
	void SendReqRepair(bool const bIsAllRepair);
	int GetRepairNeedMoney();
	void SetRepairAllInfo();
	int CheckOK();
protected:
	XUI::CXUI_Wnd *self;
};

class PgItemRepairMgr
{
public:
	PgItemRepairMgr();
	virtual ~PgItemRepairMgr();

public:
	void Clear(bool const bAllClear = false);

	void SetSrcName(std::wstring const &kStr);
	bool SetSrcItem(const SItemPos &rkItemPos);
	
	bool SetRepairAllItem();//모두 수리 한다고 생각.
	
	void DisplaySrcIcon(XUI::CXUI_Wnd *pWnd);

	void SendReqRepair(bool const bIsAllRepair);

	int CheckOK();

	SItemPos const& GetSrcPos()const{return m_kSrcItemPos;}
protected:
	BM::GUID m_guidSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	SItemPos m_kSrcItemPos;

	CONT_REQ_ITEM_REPAIR m_kContRepair;
	
	CLASS_DECLARATION_S(int, NeedMoney);
	CLASS_DECLARATION_S(bool, RepairAll);
	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
};

#define g_kItemRepair SINGLETON_STATIC(PgItemRepairMgr)

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMREPAIR_H