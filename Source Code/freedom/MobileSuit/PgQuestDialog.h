#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTDIALOG_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTDIALOG_H
//	퀘스트의 상세 정보를 보여주는 다이얼로그
class PgQUI_QuestDialog	: public PgQuestUI
{
public:
	PgQUI_QuestDialog();

	virtual CXUI_Wnd* VCreate()const{return new PgQUI_QuestDialog;}

	void Clear();

	void Show(bool const bInfoDialog, int const iQuestID, int const iDialogID);
	void UpdateControl();

	bool OnRewardItemSelected_From_QuestInfo(int iRewardType, unsigned int iSelectedNum);//고를 수 있는 보상 아이템 선택했을 때
	void OnGiveUpConfirmed();

protected:
	bool UpdateShare(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateTitle(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateDialog(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateObject(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateReward(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateRewardIcon(int const iSetNo, PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd *pTopWnd, int &iYLoc);
	bool UpdateSkillIcon(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateButtons(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);
	bool UpdateLevelIcon(PgQuestInfo const* pkQuestInfo, XUI::CXUI_Wnd* pTopWnd, int& iYLoc);

	bool UpdateState(int const iQuestID);

protected:
	bool m_bInfoDialog;
	int m_iQuestID;
	int m_iDialogID;
	SUserQuestState	m_kUserState;
};

extern bool BreakMoney_UInt(__int64 const iTotal, std::vector< unsigned int > &rkVec);

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTDIALOG_H