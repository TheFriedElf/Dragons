#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGLETTERDIALOG_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGLETTERDIALOG_H

//
class PgLetterDialog
{
public:
	PgLetterDialog();
	~PgLetterDialog();

	bool CallNewLetter(int const iTTW);

	void NextLetter();
	void PrevLetter();

private:
	void UpdateLetter();
	
	CLASS_DECLARATION_NO_SET(size_t, m_iNowLetterStep, NowLetterStep);
	CLASS_DECLARATION_NO_SET(size_t, m_iNowLetterMaxStep, NowLetterMaxStep);
private:
	ContDialogStep m_kContLetterStep;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGLETTERDIALOG_H