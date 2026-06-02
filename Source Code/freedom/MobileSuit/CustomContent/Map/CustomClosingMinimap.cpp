#include "StdAfx.h"
#include "CustomClosingMinimap.h"

namespace
{
	bool g_bDungeonMinimapAutoShowEnabled = true;
}

bool CustomClosingMinimap::IsDungeonMinimapAutoShowEnabled()
{
	return g_bDungeonMinimapAutoShowEnabled;
}

void CustomClosingMinimap::SetDungeonMinimapAutoShowEnabled(bool bEnabled)
{
	g_bDungeonMinimapAutoShowEnabled = bEnabled;
}

void CustomClosingMinimap::TryHideMinimapIfAlreadyOpened()
{
	XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"SFRM_PROGRESS_MAP");
	if (NULL != pkMiniMap)
	{
		CustomClosingMinimap::SetDungeonMinimapAutoShowEnabled(false);
		pkMiniMap->Close();
	}
}