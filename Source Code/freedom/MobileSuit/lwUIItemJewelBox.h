#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMJEWELBOX_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMJEWELBOX_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class lwUIItemJewelBox
{
public:
	lwUIItemJewelBox(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void Clear();
	void SendReqJewelBox();
	
protected:
	XUI::CXUI_Wnd *self;
};

namespace PgJewelBoxUtil
{
	void JewelBoxItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos);
};

class PgItemJewelBoxMgr
{
public:
	PgItemJewelBoxMgr();
	virtual ~PgItemJewelBoxMgr(){}

public:
	void Clear();
	bool SendReqJewelBox();
	bool SetMaterialItem(int const iType, SItemPos const& rkItemPos);

protected:
	SItemPos m_kSrcItemPos;
	SItemPos m_kKeyItemPos;
};

#define g_kJewelBoxMgr SINGLETON_STATIC(PgItemJewelBoxMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMJEWELBOX_H