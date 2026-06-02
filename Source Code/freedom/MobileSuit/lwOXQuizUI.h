#ifndef FREEDOM_DRAGONICA_CONTENTS_LWOXQUIZUI_H
#define FREEDOM_DRAGONICA_CONTENTS_LWOXQUIZUI_H

namespace lwOXQuizUI
{
	typedef enum eOXDialogState
	{
		OXDS_START = 0,
		OXDS_QUEST = 1,
		OXDS_NOTICE,
	}EOX_DLG_STATE;

	void RegisterWrapper(lua_State *pkState);

	void lwSelectOXAnswer(lwUIWnd kWnd, bool const bOK);
	void lwUpdateOXLight(lwUIWnd kWnd);
	bool lwTickUpdateTimer(lwUIWnd kWnd);
	void lwRequestOXAnswer(lwUIWnd kWnd);

	void InitOXDialogUI(XUI::CXUI_Wnd* pkWnd);
	void CallOXInvitation(BM::Stream& rkPacket);
	void CallOXQuestionDialog(BM::Stream& rkPacket);
	void CallOXNoticeDialog(std::wstring const& kText, std::wstring const& kBtnText);

	void SetOXSwitchLight(XUI::CXUI_Wnd* pkWnd, bool const bIsOK);
	int GetOXSwitchLight(XUI::CXUI_Wnd* pkWnd);
	void SetMiddleText(XUI::CXUI_Wnd* pMiddle, std::wstring const& kText);
	void SetTimeLimit(XUI::CXUI_Wnd* pParent, float fLimitTime);

	void RecvOXQuiz_Command(WORD const wPacketType, BM::Stream& rkPacket);
	bool OxQuizResult(EOXQUIZ_EVENT_RESULT const iErrorType);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_LWOXQUIZUI_H