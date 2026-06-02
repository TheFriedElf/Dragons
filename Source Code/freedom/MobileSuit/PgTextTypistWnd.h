#ifndef FREEDOM_DRAGONICA_UI_PGTEXTTYPISTWND_H
#define FREEDOM_DRAGONICA_UI_PGTEXTTYPISTWND_H

class PgTextTypistWnd : public XUI::CXUI_Wnd
{
public:
	PgTextTypistWnd();
	virtual ~PgTextTypistWnd();

	virtual void VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue);
	virtual CXUI_Wnd* VCreate() const { return new PgTextTypistWnd; }

	virtual bool VDisplay();
	virtual bool VPeekEvent(XUI::E_INPUT_EVENT_INDEX const &rET, POINT3I const &rPT, DWORD const &dwValue);//이벤트를 처리 했다면 true 리턴
	virtual void UpWheal();
	virtual void DownWheal();

	virtual void SkipDrawInterval();
	virtual void Text(std::wstring const& szString);

	bool DrawFinish()const	{ return m_kStandByWord.empty(); }
private:
	void SetButtonVisible(bool const bVisible);

private:
	typedef std::list< XUI::PgParsedChar > kWordList;
	kWordList	m_kStandByWord;
	float		m_fIntervalTime;
	float		m_fPrevTime;

	size_t m_iLineCount;
	size_t m_iCurStartLine;
	std::wstring m_kUpBtnName;
	std::wstring m_kDownBtnName;
};
#endif // FREEDOM_DRAGONICA_UI_PGTEXTTYPISTWND_H