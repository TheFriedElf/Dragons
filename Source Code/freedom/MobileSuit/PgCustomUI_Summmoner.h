#ifndef FREEDOM_DRAGONICA_CUSTOMUI_SUMMONER_H
#define FREEDOM_DRAGONICA_CUSTOMUI_SUMMONER_H

class PgActor;
class PgAction;
class PgActionTargetList;
namespace XUI
{
	class CXUI_Wnd;
}

typedef std::pair<int, BM::GUID> PgSelectSummoned;
typedef std::vector<PgSelectSummoned> CONT_SELECT_SUMMONDED;

class PgCustomUI_Summmoner
{
public:
	PgCustomUI_Summmoner();
	~PgCustomUI_Summmoner();

	void Init(PgActor * pkActor);
	bool IsUse()const;
	void Clear();
	void UpdateSelectSummond();

	XUI::CXUI_Wnd* GetShowMenu()const;
	bool CallMenu();
	void ShowMenu(bool const bShow);
	void DoCancel();
	void SelectMenu(Direction const eDir);
	void MenuUpdatePos();

	PgAction* DoAction(char const * szActionName);
	bool IsAllSelectSummoned();

	int FindTargets(PgActionTargetList & rkFoundTargetList);

private:
	void ViewSelectArrow_All(bool const bShow);
	bool IsAble() const;

private:
	int m_iSelectSummonedIdx;
	PgSelectSummoned m_kSelectSummoned;
	CONT_SELECT_SUMMONDED m_kContSelectSummoned;

	Direction m_eSelectMenuDir;
	
	PgPilot * m_pkPilot;
	PgActor * m_pkActor;
	CUnit * m_pkUnit;
};

#define g_kUnitCustomUI SINGLETON_STATIC(PgCustomUI_Summmoner)

#endif //FREEDOM_DRAGONICA_CUSTOMUI_SUMMONER_H
