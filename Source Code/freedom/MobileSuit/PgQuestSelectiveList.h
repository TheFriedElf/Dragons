#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTSELECTIVELIST_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTSELECTIVELIST_H

//	NPC 등에게서 여러개의 퀘스트를 받을 수 있을 때, 그 목록을 보여주는 인터페이스
class PgQUI_SelectiveQuestList	:	public	PgQuestUI
{

public:

	PgQUI_SelectiveQuestList()	{}
	virtual	~PgQUI_SelectiveQuestList()	{}

	void Show(BM::GUID const &QuestObjectGUID);

	virtual CXUI_Wnd* VCreate()const{return new PgQUI_SelectiveQuestList;}

	void UpdateControl();

private:
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTSELECTIVELIST_H