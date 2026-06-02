#ifndef FREEDOM_DRAGONICA_CONTENTS_MINIMAP_LWMINIMAP_SCRIPT_H
#define FREEDOM_DRAGONICA_CONTENTS_MINIMAP_LWMINIMAP_SCRIPT_H

#include "PgMinimap.h"

namespace lwMinimap
{
	void RegisterWrapper(lua_State *pkState);

	void OnOverMinimapIconItem(lwUIWnd kSelf);
	void OnClickMinimapIconItem(lwUIWnd kSelf);
	void ClearMinimapIconInfo();

	void CallMinimapObjectList(lwUIWnd kMainUI);
	bool UpdateNpcListUI(XUI::CXUI_Wnd* pkMainUI, CONT_MINIMAP_ICON_INFO& kMapIconList);
	void CloseMinimapObjectList();
	void SetMinimapObjectListItem(XUI::CXUI_Wnd* pkSlot, SMINIMAP_ICON_INFO const& kInfo);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_MINIMAP_LWMINIMAP_SCRIPT_H