#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIQUEST_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIQUEST_H
class lwWString;
class lwGUID;

namespace EventTaiwanEffectQuest
{
	void UpdateUI();
	void RegisterWrapper(lua_State *pkState);
};

namespace RealmQuest
{
	void RegisterWrapper(lua_State *pkState);
}

namespace Quest
{
	extern int const iQuestMiniListSnapGroupID;
	int const iIconWidth = 40;
	int const iIconHeight = 40;

	bool RegisterWrapper(lua_State *pkState);

	void lwOnCallQuestRewardItemImage(lwUIWnd IconControl);
	void lwOnCallQuestRewardItemToolTip(lwUIWnd kControl);
	void lwOnCallQuestRewardLevelToolTip(lwUIWnd kControl);
	void lwQuestTextParser(std::wstring &szText, const PgQuestInfo* pkQuestInfo = NULL);
	void lwOnQuestItemSelected_From_MyQuestList(lwUIWnd UIWnd);
	void lwOnQuestItemSelected_From_SelectiveQuestList(int const iQuestID, lwGUID kGuid, bool const bRemoteAccept = false);
	void lwOnItemSelected_From_QuestInfo(lwUIWnd kQuestInfoWnd, int const iSelectionID);
	void lwOnGiveUpQuestConfirmed();
	void lwOnCheckBoxClick_From_MyQuestList(lwUIWnd UIWnd);
	void lwOnShowMiniQuestList();
	unsigned int lwGet_MiniQuestList_ShowCount();
	void lwOnRewardItemSelected_From_QuestInfo(lwUIWnd UIWnd, int iRewardType, int iSelectNum);
	void lwShow_IngQuestInterface(int const iSelectedQuestID);
	bool lwIsCanShowIngQuestList(int const iQuestID);
	void lwSaveMiniIngQuestSnapInfo();
	void lwLoadMiniIngQuestSnapInfo();
	void lwSetCutedTextLimitLength(lwUIWnd UIWnd, lwWString kText,  lwWString kTail, long const iCustomTargetWidth);

	void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);

	void ReqShareQuest(lwUIWnd kBtn);
	void ClearRecvShareQuest();
	void RecvShareQuest(BM::GUID const& rkOrgGuid, int const iShareQuestID);
	void SendAnsShareQuest(BM::GUID const& rkOrgGuid, int const iShareQuestID, bool const bSayYes);
	void SendReqRemoteCompleteQuest(lwUIWnd kWnd);
}

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIQUEST_H