#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPSYSTEM_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPSYSTEM_H
#include "PgScripting.h"
#include "PgHelpSystem.H"

class	lwGUID;
class	lwHelpObject;
class	lwUIWnd;

LW_CLASS(PgHelpSystem, HelpSystem)

	//	조건에 의해 헬프 오브젝트를 활성화 시킨다.
	void	ActivateByConditionString(char const *kConditionName,char const* kConditionValue);
	void	ActivateByConditionInt(char const *kConditionName,int iConditionValue);

	//	다음,이전 헬프 오브젝트를 활성화 시킨다.
	void	ActivateNextObject();	
	void	ActivatePrevObject();

	//	현재 활성화 되어있는 오브젝트를 얻는다.
	lwHelpObject	GetActivatedObject();

	void	LoadHelpInfoFile(lwGUID kCharGUID);
	void	SaveHelpInfoFile(lwGUID kCharGUID);
	void	ResetActivatingCount();

	void	InitHelpMain(lwUIWnd kWnd);
	void	OnClickHelpBeginPage(lwUIWnd kSelf);
	void	OnClickHelpEndPage(lwUIWnd kSelf);
	void	OnClickHelpNextPage(lwUIWnd kSelf, bool bPrev = false);

	void	OnClickHelpPage(lwUIWnd kSelf);

	void	SetHelpList(XUI::CXUI_Wnd* pMainUI);

LW_CLASS_END;

namespace Helper
{
	void	RegistHelperGuid(lwGUID kGuid);
	void	RemoveHelperGuid();
	lwGUID	GetHelperGuid();
}
void OnClickDetailHelpView(int const iHelpID, bool bFromMainList = false);
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPSYSTEM_H