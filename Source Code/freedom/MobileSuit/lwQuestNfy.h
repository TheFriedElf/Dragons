#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_LWQUESTNFY_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_LWQUESTNFY_H

namespace lwQuestNfy
{
	enum E_QUEST_NFY_CALLTYPE
	{
		EQNCT_LEVELUP		= 0,
		EQNCT_QUEST_CLEAR	= 1,
		EQNCT_ALL	= 2,
	};

	bool RegisterWrapper(lua_State *pkState);

	void lwCheckNewQuestNfy(int const CallType, int const iQuestID = 0);
	void lwCallQuestNfyUI(int const CallType, int const QuestID, bool const bIsScenario);
	void lwOnOverQuestNfyListItem(lwUIWnd kWnd);
	void lwOnClickQuestNfyListItem(lwUIWnd kWnd);
	void lwOnClickDetailQuestView(lwUIWnd kWnd);
	void lwOnTickChangeTextToQuestNameAndQuestGroup(lwUIWnd kItemWnd, float fTickTime);
	bool lwOnTickCheckQuestModify(int const CallType, int const QuestID, bool const bIsScenario);

	bool CheckNewQuestToLevelUP(ContQuestInfo& kQuestList);
	bool CheckNewQuestToQuestClear(int const ClearQuestID, ContQuestInfo& kQuestList);
	bool CheckNewQuestAll(ContQuestInfo& kQuestList);

	bool CheckRemoteAcceptQuest(int const CallType, int const QuestID, bool const bIsScenario);
	void CallToolTip_NearQuestIcon(lwUIWnd kWnd);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_LWQUESTNFY_H