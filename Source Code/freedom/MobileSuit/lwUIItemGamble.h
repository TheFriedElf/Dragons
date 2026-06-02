#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMGAMBLE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMGAMBLE_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class lwUIItemGamble
{
public:
	lwUIItemGamble(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void Clear();
	void SendReqGamble();
	bool GetAgainSend();
	bool GetInProcess();
	size_t GetGambleItemCount();
	
protected:
	XUI::CXUI_Wnd *self;
};

namespace PgGambleUtil
{
	void GambleItemUpdateClear();
	void GambleItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos);
};

class PgItemGambleMgr
{
public:
	PgItemGambleMgr();
	virtual ~PgItemGambleMgr(){}

public:
	void Clear();
	bool SendReqGamble();
	bool SetMaterialItem(SItemPos const& rkItemPos);
	void AgainSendUpdate();

	void SetGambleItemNo(int const& iGambleItemNo);
	size_t GetGambleItemCount();
protected:
	SItemPos m_kSrcItemPos;
	int m_iGambleItemNo;
	CLASS_DECLARATION_S(bool, AgainSend);
	CLASS_DECLARATION_S(bool, InProcess);
};

#define g_kGambleMgr SINGLETON_STATIC(PgItemGambleMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMGAMBLE_H