#ifndef FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_PGCHATWINDOW_H
#define FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_PGCHATWINDOW_H
#include <windows.h>
#include <d3d9.h>
#include "xui/xui_font.h"
#include "HBitmap.h"

//#define	MAX_CHAT_WINDOW_TAB	3
class	Pg2DString;
#define	MAX_CHAT_LINE	500
#define	MIN_CHAT_SIZE	250
#define BORDER_SIZE 3
#define MINIMUM_AREA	50
int const WINSIZE_MIN_X = 300;
int const WINSIZE_MIN_Y = 200;

struct	stChatListElement
{
	XUI::CXUI_Style_String m_kString;
	Pg2DString	*m_pk2DString;
	int	m_iWidth,m_iHeight;

	int	m_iLimitWidth;

	HBITMAP	m_hBitmap;

	stChatListElement()
	{
		m_iLimitWidth = 0;
		m_hBitmap = 0;
		m_pk2DString = NULL;
	}
	~stChatListElement();

};

typedef enum	eChatType_Tab
{
	CTT_ALL,
	CTT_NORMAL,
	CTT_SYSTEM,
	CTT_GUILD,
	CTT_PARTY,
	CTT_TRADE,
	CCT_WHISPER,
	CTT_MAX_WINDOW_TAB,
}EChatType_Tab;

typedef	enum	eBitmap_Type	
{ 
	BT_LR, 
	BT_TB, 
	BT_CORNER,
	BT_CAP_LR,
	BT_CAP_CENTER,
	BT_DIV_LINE,
	BT_TOP_BG,
	BT_RDO,
	BT_BTN_DIV,
	BT_BTN_CHANGE,
	BT_BTN_CLOSE,
	BT_BODY,
	BT_CHAT_LT,
	BT_CHAT_CT,
	BT_CHAT_RT,
	BT_END, 
}EBitmapType;

typedef enum	eSize_RestoredPosition
{ 
	SR_L, 
	SR_LT, 
	SR_T, 
	SR_TR, 
	SR_R, 
	SR_RB, 
	SR_B, 
	SR_BL, 
	SR_END, 
}ESizeRestoredPos;

typedef	std::list<stChatListElement*> ChatListElementCont;
struct	stChatTab
{
	HWND	m_hListControl;
	HWND	m_hTabBtn;

	POINT	m_kListControlSize;
	bool	m_bIsSelect;

	ChatListElementCont	m_kChatListElementCont;

	stChatTab();
	~stChatTab();

	void	ClearAllChatElement();
	stChatListElement*	AddChatString(XUI::CXUI_Style_String const& kString, int const iLimitWidth =- 1);
	stChatListElement*	GetChatString(int const iIndex) const;
	int	GetElementSize() const;

	void	RebuildElements();
};

class	PgChatWindow
{
//	NiRenderedTexturePtr	m_spRenderTargetTexture;
//	NiRenderTargetGroupPtr	m_spRenderTargetGroup;
//	IDirect3DTexture9	*m_pkDynamicTexture;

	HWND	m_hParentWnd;
	HWND	m_hWnd;
	static	HINSTANCE	m_hInstance;

	POINT	m_kWindowSize;
	POINT	m_kWindowSizing;

	stChatTab	m_kChatTab[CTT_MAX_WINDOW_TAB];
	HWND	m_hTabBtn2[CTT_MAX_WINDOW_TAB];
	int	m_iCurrentTab;
	int	m_iCurrentTab2;

	POINT	m_kMousePos;

	HCURSOR	m_kCursor,m_hCursorSizeHoriz,m_hCursorSizeVert;

	CHBitmap	m_kBitmap[BT_END];

	RECT	m_rcSizeRestored[SR_END];
	RECT	m_rcCaption;
	RECT	m_rcClose;
	RECT	m_rcSplitRect;

	HWND	m_hDivideChatRectBtn;
	HWND	m_hChangeWinDesignBtn;
	HWND	m_hSplitBar;
	HWND	m_hClose;
	HWND	m_hEdit;
	DWORD	m_hEditFontColor;

	bool	m_bIsDivide;
	bool	m_bClickSplit;
	bool	m_bIsDesigned;

	WNDPROC	m_kOldProc;
	WNDPROC	m_kEditProc;

	HBRUSH	m_hBrush;

	bool m_bSnap;
public:

	PgChatWindow();
	~PgChatWindow();

	void	InitWindow(HWND hParentWnd);
	void	TerminateWindow();
	void	AlignClientCenter();

	void	OnIdle();

	static	LRESULT	CALLBACK	WinProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam);

	static	void	RegisterWindowClass(HINSTANCE hInstance,WNDCLASSEX &kWC);
	static	const	char*	GetClassName()	{	return	"ChatWindow";	}

	void	ClearText(int iTab);
	void	GetTextFromChatUI(int iTab);

	void	SetTab(int iTab);
	void	SetTab2(int iTab);

	void	AddNewString(int iTab,const XUI::CXUI_Style_String &kString, bool const bCheckEqualMessage = false);

	void	OnSize(int iWidth,int iHeight);
	void	OnExitSizeMove();
	void	OnCreate();
	void	OnDrawItem(LPDRAWITEMSTRUCT lpDIS);
	void	OnMeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	void	OnPaint();
	void	OnEraseBackground(HDC dc);
	void	OnCommand(WPARAM wParam,LPARAM lParam);
	void	OnSetCursor(WORD wHitTestCode);

	void	SetHWnd(HWND hWnd)	{	m_hWnd = hWnd;	}

	void	SetControlPosition();
	HWND	GetParentHWnd()	{	return	m_hParentWnd;	}
	void	OnMouseMove(int iX,int iY);

	void	SetDivideTab();
	bool	GetSplitState() { return m_bClickSplit; }
	void	MouseInSplit(bool bRet);
	WNDPROC	GetOldProc() { return m_kOldProc; }
	WNDPROC GetEditProc() { return m_kEditProc; }
	static	LRESULT	CALLBACK	SubProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam);
	static	LRESULT	CALLBACK	EditProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam);
	void	MoveSplit();
	HWND	GethWnd() const { return m_hWnd; };
	HBRUSH	OnEditPaint(WPARAM wParam, LPARAM lParam);
	void	SetChatControl(EChatType const Type, std::wstring const& kHead, DWORD const Color);
	bool	SetChatOutFocus();
	void	SetSnap(bool bSnap) { m_bSnap = bSnap; }
	bool	IsSnap() { return m_bSnap; }

private:

	void	ReBuildListContents();

	int		OnNCHit(UINT uiMsg, WPARAM wParam, LPARAM lParam);
	void	DrawWindow(HDC hDC);
	void	DrawFormBitmap(HDC hDC, int iStartX, int iStartY, int iWidth, int iHeight);
	void	AreaCheckToReSet();
/*
	void	Draw();
	void	CreateRenderTarget();
	void	UpdateRenderTarget();
	void	UpdateMemoryDC(HDC kMemDC);
	HBITMAP	CreateBitmap(HDC hDC);*/
};

extern	PgChatWindow	g_kChatWindow;
#endif // FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_PGCHATWINDOW_H