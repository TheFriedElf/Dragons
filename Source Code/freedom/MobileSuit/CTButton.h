#ifndef FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_CTBUTTON_H
#define FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_CTBUTTON_H

class CHBitmap;
class CCTButton
{
private:
	HWND	m_hParent;
	HWND	m_hWnd;
	int		m_iCtlID;
	POINT	m_PtPos;
	std::wstring	m_wstrBtnText;
	CHBitmap*		m_hBitmap;

public:
	void	InitPush(HINSTANCE Inst, HWND Parent, int CtlID, POINT ptPos, std::wstring BtnText, CHBitmap* pBitmap, UINT Flag = 0);
	void	InitRadio(HINSTANCE Inst, HWND Parent, int CtlID, POINT ptPos, std::wstring BtnText, CHBitmap* pBitmap, UINT Flag = 0);
	void	OwnerDraw(LPDRAWITEMSTRUCT lpDIS);
	
	bool	IsMyID(int ID) { return (m_iCtlID == ID)?(true):(false); };
	HWND	IsMyhWnd()	{ return m_hWnd; };
	
	CCTButton(void);
	virtual ~CCTButton(void);
};

#endif // FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_CTBUTTON_H