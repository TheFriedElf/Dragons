#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMEVENTBOX_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMEVENTBOX_H

#include "PgScripting.h"
#include "lwGUID.h"
#include "lwWString.h"
#include "lwPacket.h"
#include "lwPoint2.h"
#include "lwPoint3.h"
#include "XUI/XUI_Manager.h"

class lwUIItemEventBox
{
public:
	lwUIItemEventBox(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void Clear();
	void SendReqEventBox();
	bool GetAgainSend();
	bool GetInProcess();
	size_t GetEventBoxItemCount();
	
protected:
	XUI::CXUI_Wnd *self;
};

namespace PgEventBoxUtil
{
	void EventBoxItemUpdateClear();
	void EventBoxItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos);
};

class PgItemEventBoxMgr
{
public:
	PgItemEventBoxMgr();
	virtual ~PgItemEventBoxMgr(){}

public:
	void Clear();
	bool SendReqEventBox();
	bool SetMaterialItem(SItemPos const& rkItemPos);
	void AgainSendUpdate();

	void SetEventBoxItemNo(int const& iEventBoxItemNo);
	size_t GetEventBoxItemCount();
protected:
	SItemPos m_kSrcItemPos;
	int m_iEventBoxItemNo;
	CLASS_DECLARATION_S(bool, AgainSend);
	CLASS_DECLARATION_S(bool, InProcess);
};

#define g_kEventBoxMgr SINGLETON_STATIC(PgItemEventBoxMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMEVENTBOX_H