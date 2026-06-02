#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTINGLIST_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTINGLIST_H

//	진행중인 퀘스트 목록
class PgQUI_IngQuestList	:	public	PgQuestUI
{
public:
	PgQUI_IngQuestList();
	virtual	~PgQUI_IngQuestList();

	void Show(int const iSelectedQuestID);

	virtual CXUI_Wnd* VCreate()const{return new PgQUI_IngQuestList;}

	void OnQuestItemSelected_From_MyQuestList(int const iQuestID);
	void OnCheckBoxClick_From_MyQuestList(XUI::CXUI_Wnd *pItem);
	void UpdateControl();
	
private:
	void UpdateItem(XUI::CXUI_Wnd *pkNewQuestWnd, SUserQuestState const &rkQuestState, PgQuestInfo const *pkQuestInfo);

	CLASS_DECLARATION_S_NO_SET(int, SelectQuestID);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTINGLIST_H