#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGDAILYQUESTUI_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGDAILYQUESTUI_H
namespace DailyQuestUI
{
	static int iLastDailyQuestUITitleNo = 0;
	extern std::wstring const kDailyQuestWndName;

	void GetDailyQuestCustomData(XUI::CXUI_Wnd* pkItemWnd, BM::GUID& kNpcGuid, EQuestState &eState, int &iQuestID);
	EQuestState GetDailyQuestState(int const iQuestID, ContNpcQuestInfo const& kQuestVec, PgMyQuest const* pkMyQuest);
	bool RegisterWrapper(lua_State *pkState);
	void CallDailyQuestUI(lwGUID kNpcGuid, int const iTitle);
	void SendDailyQuestPacket();
	void AnsShowQuestDialog(BM::GUID const& rkNpcGuid, BYTE const cDialogType, int const iQuestID, EQuestType const eQuestType, int const iDialogID);
};

namespace RandomQuestUI
{
	extern int const iCantCompleteInMission;
	extern std::wstring const kRandomQuestUIName;
	extern std::wstring const kRandomQuestBtnName;

	bool RegisterWrapper(lua_State *pkState);
	void SendReqBuild();
	bool IsCanBuild();
	bool IsCanRun();
	void CallRandomQuestBtn(bool bTwinkle = false);
	void CallRandomQuestUI(bool const bAutoNext = false);
};

namespace RandomTacticsQuestUI
{	
	extern std::wstring const kRandomTacticsQuestBtnName;
	bool RegisterWrapper(lua_State *pkState);
	void SendReqBuild();
	bool IsCanBuild();
	bool IsCanRun();
	void CallRandomTacticsQuestBtn(bool bTwinkle = false);
	void CallRandomTacticsQuestUI(bool const bAutoNext = false);
};

namespace WantedQuestUI
{
	extern int iLastSelectIndex;
	extern std::wstring const kWantedQuestUIName;
	extern std::wstring const kWantedQuestBtnName;
	bool RegisterWrapper(lua_State *pkState);
	void SendReqBuild();
	bool IsCanBuild();
	bool IsCanRun();
	void CallWantedQuestBtn(bool bTwinkle = false);
	void CallWantedQuestUI();	
}

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGDAILYQUESTUI_H