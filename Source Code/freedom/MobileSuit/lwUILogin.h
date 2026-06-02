#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUILOGIN_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUILOGIN_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"

class lwUILogin
{
public:
	lwUILogin(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void LoginToChannel();
	void ReqChannelInfo();
	void SendRealm();
	void BackToRealmSelect();
	void SendChannel();
	void SelectItem(lwUIWnd UIBtn, bool bIsRealm);
protected:
	XUI::CXUI_Wnd *self;
};

namespace LoginUtil
{
	void SendSelectCharacter(BM::GUID const &rkGuid, bool const bPassTutorial);
	void CALLBACK SendSelectCharacterAfterMovieFinished(std::wstring kMovieID);
	void CALLBACK SendSelectCharacterAfterMovieFinished_NotTutorial(std::wstring kMovieID);
	bool CanShowCharacterMovie();
	extern BM::GUID g_kSelectedCharacterGUID;

	extern int const LOGIN_FAIL_COUNT;
	extern int g_iLoginFailExitCount;
}

void lwCannotConnectLogin();
int const lwGetCurrentChennelName();
void NetCallChannelUI(BM::Stream &rkPacket);
void NetCallRealmUI(BM::Stream& rkPacket);
void RealmSaveCheck();
void CutByRealmNameAndFlag(std::wstring const& kString, std::wstring& RealmName, bool& bIsNew);
void NetCallChannelUI(short const nRealmNo, bool bChange);
void lwUIItemDeSelect(lwUIWnd UIParent, char const* szFormName, int const iMaxForm);
void lwUIItemDeSelect2(lwUIWnd UIParent, char const* szFormName, int const iMaxForm);
bool lwDisconnectLoginServer();
bool lwSendSelectCharacter(lwGUID kGuid);
void lwLoginIDSave(bool bSave, std::wstring const& Text);
void lwOnCallLoginDlg(lwUIWnd UIParent);
void lwOnCallOnLoginDlg( lwUIWnd kSelf );
void lwCloseServerTuringCard();
float lwGetTurnStartTime();
void NetClearChennelUI();


extern void NetCallChannelUI(short const nRealmNo, bool bChange);
extern bool g_bNeedManyRealmCardUI;
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUILOGIN_H