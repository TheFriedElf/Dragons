#ifndef FREEDOM_DRAGONICA_UI_PGPAGE_H
#define FREEDOM_DRAGONICA_UI_PGPAGE_H

class PgPage
{
public:
	void SetPageAttribute(int const MAX_SLOT, int const MAX_PAGE);
	void SetMaxItem(int const iMaxItem, bool bResetNowPage = true);
	bool PageCheck(int const iPageIdx) const;
	int PageSet(int const iPageIdx) const;
	int PagePrev() const;
	int PageNext() const;
	int PagePrevJump() const;
	int PageNextJump() const;
	int PageBegin() const;
	int PageEnd() const;

	int const GetMaxItemSlot() const { return M_MAX_SLOT; }
	int const GetMaxViewPage() const { return M_MAX_PAGE; }
	int const Now() const { return m_kNow; }

	PgPage();
	~PgPage();

protected:
	CLASS_DECLARATION_S_NO_SET(int, MaxItem);
	CLASS_DECLARATION_S_NO_SET(int, Max);
	int M_MAX_SLOT;
	int M_MAX_PAGE;
	mutable int m_kNow;
};

namespace UIPageUtil
{
	void PageControl(XUI::CXUI_Wnd* pPageMainUI, PgPage const& kPage);
}
#endif // FREEDOM_DRAGONICA_UI_PGPAGE_H