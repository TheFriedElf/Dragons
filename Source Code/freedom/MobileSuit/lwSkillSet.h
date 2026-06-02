#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWSKILLSET_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWSKILLSET_H
#include "lwUI.h"

int const JUMPSKILLNO = 100001001;


namespace lwSkillSet
{
	void RegisterWrapper(lua_State *pkState);
	void lwSkillSetInit(lwUIWnd kWnd);
	void lwSkillSetClose();
	void lwSkillSetSave();
	int lwGetClassLevel(int const iClass);
	void lwSkillSetDoAction(BYTE const iSetNo);
	void lwCallSkillSetToolTip(BYTE const iSetNo);
	void lwChangeSaveState(bool const bEnable);
	void lwSetMinTime(lwUIWnd kWnd, int const iSkillNo);
	//Basic SkillSet
	void lwClearBasicSkillSet();
	void lwCallSkillSetToolTip_Basic(BYTE const iSetNo);
	bool lwSetBasicSkillSet(int const iSetNo, int const iSkillNo, int const iCheckSkillNo, int const iDelay, int const iCount);
	void lwApplyBasicSkillSetToSkillSet();
	bool lwCheckUseSkillSetWnd( lwUIWnd kWnd, bool const bCheckAll );
	bool lwCheckUseSkillSetNo( int const iSetNo, bool const bCheckAll );
	bool lwCallHelpBasicSkillSet( int const iSetNo, bool const bCheckAll );
	void lwSetQuickInvBasicSkillSet(int const iQuickInvSlot, int const iSetNo );
	int lwGetHelpSkillSetNo(lwUIWnd* UISelf);
	void lwCallHelpSkillSetToolTip(int const iSetNo, lwPoint2& pt);
}
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWSKILLSET_H