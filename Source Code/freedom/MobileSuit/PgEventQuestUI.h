#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_EVENTQUEST_PGEVENTQUESTUI_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_EVENTQUEST_PGEVENTQUESTUI_H
class PgEventQuest;

namespace PgEventQuestUI
{
	void ClearInfo();
	void ReadFromPacket(BM::Stream& rkPacket);

	void UpdateUI();
	void CloseUI();
	void CloseMini();
	void CallMini();
	bool IsNowEvent();
	void CheckNowEvent();

	void Complete();
	void CheckTargetItem();
	bool IsInTargetItem(int const iItemNo);

	//
	bool RegisterWrapper(lua_State *pkState);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_EVENTQUEST_PGEVENTQUESTUI_H