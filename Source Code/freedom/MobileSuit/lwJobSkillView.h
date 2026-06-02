#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLVIEW_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLVIEW_H
#include "lwUI.h"

namespace lwJobSkillView
{
	bool IsActivateMainWnd();
	void UpdateMainWnd();

	void RegisterWrapper(lua_State *pkState);
	void lwCallJobSkillViewUI();
	bool lwShowList();
	bool lwSetIcon( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm );
	bool lwSetExpertness( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm );
	void VisibleEmptyText(bool const bVisible);
	void SetTiredGauge(PgPlayer* pPlayer);
	void lwSetEnableSubSkill( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm );
	void lwCallToolTip_EnableSubTool( lwUIWnd kWnd, lwPoint2 &pt);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLVIEW_H