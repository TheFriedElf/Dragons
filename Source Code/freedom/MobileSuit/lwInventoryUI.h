#ifndef FREEDOM_DRAGONICA_CONTENTS_INVENTORY_LWINVENTORYUI_H
#define FREEDOM_DRAGONICA_CONTENTS_INVENTORY_LWINVENTORYUI_H

#include "PgInventoryUI.h"

namespace lwInventory
{
	void ClearCompInv();
	void InitCompInv(PgInventory* m_kOrgInv);
	void lwModifyCompInv(EInvType const eInvType, PgInventory* m_kOrgInv);
	void RegisterWrapper(lua_State *pkState);
	void lwApplyCurrentInv(EInvType const eInvType);
	void lwCallNewInvItem_Tab(EInvType const eInvType);
	bool lwCallNewInvItem(EInvType const eInvType);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_INVENTORY_LWINVENTORYUI_H