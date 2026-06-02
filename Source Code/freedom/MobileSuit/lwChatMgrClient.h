#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_LWCHATMGRCLIENT_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_LWCHATMGRCLIENT_H

#include "PgChatMgrClient.h"

class	lwStyleString;
class lwChatMgrClient
{
public:	
	lwChatMgrClient(PgChatMgrClient* pkChatMgrClient);
	
	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);
	static void CallMegaPhone(int const Type);

	//
	bool SendChatW(lwWString lwkChat, bool const bCheckSpamChat);//기본 채팅(나머지는 임의적으로 필요할때만 사용)
	bool SendChatStyleString(lwStyleString lwkChat, bool const bCheckSpamChat);//기본 채팅(나머지는 임의적으로 필요할때만 사용)

	bool SendChat_InputNow(bool bPopup);

	bool RecvChat(lwPacket &rkPacket);
	bool RecvChat_InputNow(lwPacket &rkPacket);
	bool Self_InputNow(bool bPopup, int iChatMode);

	bool Character_ChatBaloon_Set(lwGUID lwkGuid, lwWString lwkChat, int iChatType);

	//void Set_IsGodCommand();

	void Notice_Show(lwWString kString, int const iLevel);
	void Notice_Show_ByDefStringNo(int const iDefStringNo);
	void Notice_Show_ByTextTableNo(int const iTextTableNo);
	void Notice_Show_ByTextTableNo_CheckEqualMessage(int const iTextTableNo);

	int ChatMode_Get();
	int ChatMode_Set(int iChatType);

	void ChatFilter_ModeClear();//채팅모드로 필터링 초기화
	void ChatFilter_ModeAdd(int iFilterSet, int iChatMode, char const *szXuiListID);//채팅모드로 필더링 추가
	void ChatFilter_ModeDel(int iFilterSet, int iChatMode, char const *szXuiListID);//채팅모드로 필더링 삭제

	void NoticeLevelColor_Add(int const iLevel, const unsigned int dwFontColor, const unsigned int dwOutlineColor, bool const bShowBG);//경고 레벨 컬러를 추가한다.
	void NoticeLevelColor_Clear();//경고 레벨 컬러를 초기화 한다.

	//void Chat_Refresh();
	
	void SetSpamChkFlag(int const iChkFlag);
	void SetMaxInputLog(int const iMaxInputLog);
	void SetMaxBlockTime(float const fMaxBlockTime);
	void SetBlockSpamChat_ChainInput(float const fWatchT, float const fBlockTime);
	void SetBlockSpamChat_LooseChainInput(float const fWatchT, int const iCnt, float const fBlockTime);
	void SetBlockSpamChat_MacroInput(int const iCnt, float const fRangeTime, float const fBlockTime);
	void SetBlockSpamChat_SameInput(float const fWatchT, int iCnt, float const fBlockTime);

	void AddEventMessage(int iEventMessageNo, bool bNotice = false, int iLevel = 0, int iChatType = CT_EVENT);
	void AddLogMessage(lwWString kContents, bool bNotice = false, int iLevel = 0, int iChatType = CT_EVENT);

	/*
	bool SendChat_Party_Inputnow();
	bool SendChat_Party_Claer();
	*/
	void InputNow_TimeMinimum(int nInterval);
	bool CheckChatOut();
	void ClearNotice();
	bool OnFocus_ChatEdit();

	void SetChatStation(int const iType);
	bool SetWhisperDlg(lwUIWnd Wnd);
	lwWString SetWhisperByTab();
	lwWString ConvertEmoticonCommandToHex(lwWString Str);
	unsigned int GetFontColor();
	void SetFontColor(unsigned int iColor);
	bool CheckChatCommand(lwWString Text);
	lwWString Name2Guid_Find_ByGuid(lwGUID Guid);

	void ChatAram(lwWString Text, bool bShowNow = false);
	void ClearChatBalloon();
	void SetChatOutDefaultPos(lwUIWnd Wnd);
	void SetSysChatOutDefaultPos(lwUIWnd Wnd);

	bool ToggleConsecutiveChat(void);
	bool GetToggleConsecutiveChat(void);

	void ToggleConsecutiveChatUI(void);
	void SetExistWhisperList(bool bSet);
	bool GetExistWhisperList(void);

	void SetSysChatOutSnap(bool bSnap);
	bool GetSysChatOutSnap();

	void CheckSnapPos(lwPoint2 const& rkPos);

	void ShowNotifyConnectInfoUI(void);
	void PopNotifyList(void);

	lwGUID GetNotifyGuid(void);
	lwGUID Name2Guid_Find_ByName(lwWString kName);

	void SetSysOutHide(bool bHide);
	bool GetSysOutHide(void);

	bool CheckChatTag(lwWString Text);
	void Regist_ChatBlockUser(lwWString & Name);
	void UnRegist_ChatBlockUser(lwWString & Name);
	void Modify_ChatBlockMode(lwWString & Name, BYTE BlockMode);
	void UpdateChatBlockList(lwUIWnd BlockWnd);

protected:
	PgChatMgrClient *m_pkChatMgrClient;

	CLASS_DECLARATION(DWORD, m_dwInputNow_TimeMin, InputNow_TimeMin);
	CLASS_DECLARATION(DWORD, m_dwInputNow_Time, InputNow_Time);
	CLASS_DECLARATION(bool, m_bInputNow, InputNow);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_LWCHATMGRCLIENT_H