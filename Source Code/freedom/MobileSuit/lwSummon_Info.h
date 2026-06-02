#ifndef FREEDOM_DRAGONICA_SUMMON_INFO_H
#define FREEDOM_DRAGONICA_SUMMON_INFO_H

#include "PgCustomUI_Summmoner.h"
#include "lwUI.h"

namespace lwSummon_Info
{
	void RegisterWrapper(lua_State *pkState);
	void lwShowSummonList();
	void ReSizeUI();
	void lwSummonList_Minimize(lwUIWnd kSelf);
	void AddAllSummoned();
	void lwAddSummon(BM::GUID const kSummonGuid);

	void lwSummonList_Tick();
	bool lwUpdateItem(lwUIWnd kSelf);

	void SetSelectSummonAll(bool const bShow);
	void ShowSelectSummon(BM::GUID const& rkSummonGuid);
	void CallSummonToolTip(lwGUID kGuid, lwPoint2 &pt);
};

#endif