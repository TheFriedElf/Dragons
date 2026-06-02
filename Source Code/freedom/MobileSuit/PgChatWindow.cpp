#include "stdafx.h"
#include "CTButton.h"
#include "PgChatWindow.H"
#include "./NiApplication/NiDX9Select.h"
#include "PgRenderer.H"
#include "PgUIScene.H"
#include "XUI/XUI_Manager.H"
#include "PgUISound.h"
#include "PgError.H"
#include "Pg2DString.H"
#include "lwUI.h"
#include "PgMobileSuit.h"
#include "bm/filesupport.h"
#include "PgChatMgrClient.h"
#include "PgSoundMan.h"

using namespace XUI;

HINSTANCE	PgChatWindow::m_hInstance = 0;
int const	MAX_CHAT_TEXT_LEN = 100;
PgChatWindow	g_kChatWindow;

stChatListElement::~stChatListElement()
{
	SAFE_DELETE(m_pk2DString);
	if(m_hBitmap)
	{
		DeleteObject(m_hBitmap);
	}
	m_hBitmap = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
//	struct	stChatTab
//////////////////////////////////////////////////////////////////////////////////////////
stChatTab::stChatTab()
{
	m_hListControl = 0;
	m_hTabBtn = 0;
	m_bIsSelect = false;
	m_kListControlSize.x=m_kListControlSize.y=0;
}
stChatTab::~stChatTab()
{
	ClearAllChatElement();
}
void	stChatTab::ClearAllChatElement()
{
	for(ChatListElementCont::iterator itor = m_kChatListElementCont.begin();
		itor != m_kChatListElementCont.end(); itor++)
	{
		stChatListElement	*pkElem = *itor;
		SAFE_DELETE(pkElem);
	}
	m_kChatListElementCont.clear();
}

stChatListElement*	stChatTab::AddChatString(CXUI_Style_String const& kString, int const iLimitWidth)
{
	stChatListElement	*pkNewElem = new stChatListElement();

	pkNewElem->m_pk2DString = new Pg2DString(kString,true,true,iLimitWidth);
	pkNewElem->m_iWidth = pkNewElem->m_pk2DString->GetSize().x;
	pkNewElem->m_iHeight = pkNewElem->m_pk2DString->GetSize().y;
	pkNewElem->m_iLimitWidth = iLimitWidth;
	pkNewElem->m_kString = kString;

	m_kChatListElementCont.push_back(pkNewElem);

	return	pkNewElem;
}

stChatListElement*	stChatTab::GetChatString(int const iIndex) const
{
	int const iMaxSize = static_cast<int>(m_kChatListElementCont.size());

	if(iIndex >= iMaxSize)
	{
		return NULL;
	}

	int iCount = 0;
	ChatListElementCont::const_iterator itor = m_kChatListElementCont.begin();

	while(iCount != iIndex)
	{
		++iCount;
		++itor;
	}

	return (*itor);
}
int	stChatTab::GetElementSize() const
{
	return static_cast<int>(m_kChatListElementCont.size());
}

void	stChatTab::RebuildElements()
{
	int	iScreenWidth = m_kListControlSize.x;
	
	::SendMessage(m_hListControl,LB_RESETCONTENT,0,0);

	for(ChatListElementCont::iterator itor = m_kChatListElementCont.begin(); itor != m_kChatListElementCont.end(); itor++)
	{

		stChatListElement	*pkElem = *itor;

		//	문자열들의 가로폭을 다시 맞춘다.
		Pg2DString	*pkNewString = new Pg2DString(pkElem->m_kString,true,true,iScreenWidth-32);
		SAFE_DELETE(pkElem->m_pk2DString);
		pkElem->m_pk2DString = pkNewString;
		DeleteObject(pkElem->m_hBitmap);
		pkElem->m_hBitmap = 0;
		pkElem->m_iWidth = pkElem->m_pk2DString->GetSize().x;
		pkElem->m_iHeight = pkElem->m_pk2DString->GetSize().y;
		pkElem->m_iLimitWidth = iScreenWidth;

		::SendMessage(m_hListControl,LB_ADDSTRING,0,(LPARAM)pkElem);

	}

	int	iTotal = m_kChatListElementCont.size();
	::SendMessage(m_hListControl,LB_SETCARETINDEX,iTotal-1,false);
}
//////////////////////////////////////////////////////////////////////////////////////////
//	class	PgChatWindow
//////////////////////////////////////////////////////////////////////////////////////////


PgChatWindow::PgChatWindow()
{
	m_hWnd = 0;
	m_hInstance = 0;
	//m_iCurrentTab = -1;
	//m_iCurrentTab2 = -1;
	m_iCurrentTab = CTT_NORMAL;
	m_iCurrentTab2 = CTT_SYSTEM;

	m_kCursor = 0;
	m_hCursorSizeHoriz = 0;
	m_hCursorSizeVert = 0;

	m_kMousePos.x=m_kMousePos.y=0;
	m_kWindowSizing.x=m_kWindowSizing.y=0;

//	m_spRenderTargetTexture = 0;
//	m_spRenderTargetGroup = 0;

	m_kWindowSize.x = m_kWindowSize.y=0;

	m_hDivideChatRectBtn = NULL;
	m_hChangeWinDesignBtn = NULL;
	m_hSplitBar = NULL;
	m_hClose	= NULL;
	m_bIsDivide = false;
	m_bIsDesigned = false;
	m_bClickSplit = false;

	for(int i=0; i<CTT_MAX_WINDOW_TAB; ++i)
	{
		m_hTabBtn2[i] = NULL;
	}

	m_bSnap = true;
/*	if(m_pkDynamicTexture)
	{
		m_pkDynamicTexture->Release();
	}
	m_pkDynamicTexture = NULL;*/
}

PgChatWindow::~PgChatWindow()
{
	TerminateWindow();
}

void	PgChatWindow::RegisterWindowClass(HINSTANCE hInstance,WNDCLASSEX &kWC)
{
	std::wstring	kClassName = UNI(GetClassName());

	m_hInstance = hInstance;

    kWC.lpfnWndProc   = PgChatWindow::WinProc;
    kWC.hCursor       = LoadCursor(0, IDC_ARROW);
	kWC.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    kWC.lpszClassName = kClassName.c_str();

	RegisterClassEx(&kWC);
}

void	PgChatWindow::InitWindow(HWND hParentWnd)
{
	if(!m_hInstance){ return; }
	if( m_hWnd )	{ return; }	


	if (g_bUsePackData && g_bUseExtraPackData)
	{
		std::vector<char>	kData;

		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/box_CN.bmp", kData);
		m_kBitmap[BT_CORNER].LoadBitmapToMemory(kData, 2, 2);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/Box_LR.bmp", kData);
		m_kBitmap[BT_LR].LoadBitmapToMemory(kData, 2, 1);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/Box_TB.bmp", kData);
		m_kBitmap[BT_TB].LoadBitmapToMemory(kData, 1, 2);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/Box_Body.bmp", kData);
		m_kBitmap[BT_BODY].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/Tab_LR.bmp", kData);
		m_kBitmap[BT_CAP_LR].LoadBitmapToMemory(kData, 2, 1);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/Tab_Body.bmp", kData);
		m_kBitmap[BT_CAP_CENTER].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/LineBg.bmp", kData);
		m_kBitmap[BT_DIV_LINE].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/TopBG.bmp", kData);
		m_kBitmap[BT_TOP_BG].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/CheckB.bmp", kData);
		m_kBitmap[BT_RDO].LoadBitmapToMemory(kData, 2, 1);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/CloseB.bmp", kData);
		m_kBitmap[BT_BTN_CLOSE].LoadBitmapToMemory(kData, 2, 2);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/B01.bmp", kData);
		m_kBitmap[BT_BTN_DIV].LoadBitmapToMemory(kData, 2, 2);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/B02.bmp", kData);
		m_kBitmap[BT_BTN_CHANGE].LoadBitmapToMemory(kData, 2, 2);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/chatBg.bmp", kData);
		m_kBitmap[BT_CHAT_LT].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/chatBg2.bmp", kData);
		m_kBitmap[BT_CHAT_CT].LoadBitmapToMemory(kData);
		BM::PgDataPackManager::LoadFromPack(L"../data/6_ui.dat", L"./chat/chatBg3.bmp", kData);
		m_kBitmap[BT_CHAT_RT].LoadBitmapToMemory(kData);
	}
	else
	{
		std::vector<char>	kData;
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\box_CN.bmp", kData);
		m_kBitmap[BT_CORNER].LoadBitmapToMemory(kData, 2, 2);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\Box_LR.bmp", kData);
		m_kBitmap[BT_LR].LoadBitmapToMemory(kData, 2, 1);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\Box_TB.bmp", kData);
		m_kBitmap[BT_TB].LoadBitmapToMemory(kData, 1, 2);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\Box_Body.bmp", kData);
		m_kBitmap[BT_BODY].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\Tab_LR.bmp", kData);
		m_kBitmap[BT_CAP_LR].LoadBitmapToMemory(kData, 2, 1);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\Tab_Body.bmp", kData);
		m_kBitmap[BT_CAP_CENTER].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\LineBg.bmp", kData);
		m_kBitmap[BT_DIV_LINE].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\TopBG.bmp", kData);
		m_kBitmap[BT_TOP_BG].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\CheckB.bmp", kData);
		m_kBitmap[BT_RDO].LoadBitmapToMemory(kData, 2, 1);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\CloseB.bmp", kData);
		m_kBitmap[BT_BTN_CLOSE].LoadBitmapToMemory(kData, 2, 2);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\B01.bmp", kData);
		m_kBitmap[BT_BTN_DIV].LoadBitmapToMemory(kData, 2, 2);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\B02.bmp", kData);
		m_kBitmap[BT_BTN_CHANGE].LoadBitmapToMemory(kData, 2, 2);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\chatBg.bmp", kData);
		m_kBitmap[BT_CHAT_LT].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\chatBg2.bmp", kData);
		m_kBitmap[BT_CHAT_CT].LoadBitmapToMemory(kData);
		BM::FileToMem(L"..\\Data\\6_UI\\chat\\chatBg3.bmp", kData);
		m_kBitmap[BT_CHAT_RT].LoadBitmapToMemory(kData);
	}

	RECT	kRect;
	GetWindowRect(hParentWnd,&kRect);

	m_kWindowSize.x = MIN_CHAT_SIZE;
	m_kWindowSize.y = kRect.bottom-kRect.top;

	std::string ClientName = GetClientName();
	ClientName += " Chatting Extension";

	m_hWnd = CreateWindowEx(
		WS_EX_LAYERED,
		UNI(GetClassName()),
		UNI(ClientName),
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
		kRect.right,
		kRect.top,
		m_kWindowSize.x,m_kWindowSize.y,hParentWnd,
		NULL,
		m_hInstance,
		0L);

	m_hParentWnd = hParentWnd;

	m_kCursor = LoadCursor(m_hInstance,MAKEINTRESOURCE(IDC_ARROW));
	m_hCursorSizeHoriz = LoadCursor(m_hInstance,MAKEINTRESOURCE(IDC_SIZEWE));
	m_hCursorSizeVert = LoadCursor(m_hInstance,MAKEINTRESOURCE(IDC_SIZENS));

	SetLayeredWindowAttributes(m_hWnd, RGB(255, 0, 255), 255, LWA_COLORKEY | LWA_ALPHA);//
	SetFocus(hParentWnd);

}

void	PgChatWindow::ClearText(int iTab)
{
	PG_ASSERT_LOG(iTab>=0 && iTab<CTT_MAX_WINDOW_TAB);

	SendMessage(m_kChatTab[iTab].m_hListControl,LB_RESETCONTENT,0,0);
	m_kChatTab[iTab].ClearAllChatElement();

}
void	PgChatWindow::GetTextFromChatUI(int iTab)
{
	PG_ASSERT_LOG(iTab>=0 && iTab<CTT_MAX_WINDOW_TAB);

	ClearText(iTab);

	CXUI_Wnd	*pkChatOut = XUIMgr.Get(_T("ChatOut"));
	if(!pkChatOut)
	{
		return;
	}

	char	*strControlID[] = { "LIST_CHAT_NORMAL", "LIST_CHAT_GUILD", "LIST_CHAT_PARTY", "LIST_CHAT_TRADE", "LIST_CHAT_SYSTEM" };

	XUI::CXUI_List *pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkChatOut->GetControl(UNI(strControlID[iTab])));
	if(!pkListWnd)
	{
		return;
	}

	int	iTotal = 0;

	stChatListElement	*pkElem = NULL;
	SListItem	*pkItem = pkListWnd->FirstItem();
	while(pkItem)
	{

		XUI::CXUI_Wnd *pkWnd = pkItem->m_pWnd;
		if(pkWnd)
		{
			pkElem = m_kChatTab[iTab].AddChatString(pkWnd->StyleText());
			SendMessage(m_kChatTab[iTab].m_hListControl,LB_ADDSTRING,0,(LPARAM)pkElem);
			iTotal++;
		}

		pkItem = pkListWnd->NextItem(pkItem);
	}

	SendMessage(m_kChatTab[iTab].m_hListControl,LB_SETCARETINDEX,iTotal,false);

}
void	PgChatWindow::SetTab(int iTab)
{
	PG_ASSERT_LOG(iTab>=0 && iTab<CTT_MAX_WINDOW_TAB);

	if( m_bIsDivide && (m_iCurrentTab2 == iTab ) )
	{
		return;
	}

	if(m_iCurrentTab != iTab)
	{
		ShowWindow(m_kChatTab[m_iCurrentTab].m_hListControl,SW_HIDE);

		m_iCurrentTab = iTab;

		int	iTotal = m_kChatTab[iTab].m_kChatListElementCont.size();
		SendMessage(m_kChatTab[iTab].m_hListControl,LB_SETCARETINDEX,iTotal-1,false);
	}
	ShowWindow(m_kChatTab[iTab].m_hListControl,SW_SHOW);
	SetControlPosition();
}

void	PgChatWindow::SetTab2(int iTab)
{
	PG_ASSERT_LOG(iTab>=0 && iTab<CTT_MAX_WINDOW_TAB);

	if( m_iCurrentTab == iTab )
	{
		return;
	}

	if(m_iCurrentTab2 != iTab)
	{
		ShowWindow(m_kChatTab[m_iCurrentTab2].m_hListControl,SW_HIDE);

		m_iCurrentTab2 = iTab;

		int	iTotal = m_kChatTab[iTab].m_kChatListElementCont.size();
		SendMessage(m_kChatTab[iTab].m_hListControl,LB_SETCARETINDEX,iTotal-1,false);
	}
	ShowWindow(m_kChatTab[iTab].m_hListControl,SW_SHOW);
	SetControlPosition();
}

void PgChatWindow::SetDivideTab()
{
	if( m_bIsDivide )
	{
		ShowWindow(m_hSplitBar, SW_SHOW);
		//m_iCurrentTab = CTT_SYSTEM;
		//m_iCurrentTab2 = CTT_NORMAL;
		std::swap(m_iCurrentTab, m_iCurrentTab2);
		if( m_iCurrentTab == m_iCurrentTab2 )
		{
			if( CTT_SYSTEM == m_iCurrentTab2 )
			{
				m_iCurrentTab = CTT_NORMAL;
			}
			else
			{
				m_iCurrentTab = CTT_SYSTEM;
			}
		}

		int i = 0;
		for( i=0; i<CTT_GUILD; ++i)
		{
			ShowWindow(m_hTabBtn2[i],SW_SHOW);
		}
		for( i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
		{
			if( i != m_iCurrentTab && i != m_iCurrentTab2 )
			{
				ShowWindow(m_kChatTab[i].m_hListControl,SW_HIDE);
				continue;
			}
			int	iTotal = m_kChatTab[i].m_kChatListElementCont.size();
			SendMessage(m_kChatTab[i].m_hListControl,LB_SETCARETINDEX,iTotal-1,false);
			ShowWindow(m_kChatTab[i].m_hListControl,SW_SHOW);
		}

		for( i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
		{
			if ( m_kChatTab[i].m_hTabBtn )
			{
				InvalidateRect( m_kChatTab[i].m_hTabBtn, NULL, TRUE );
			}

			if( m_hTabBtn2[i] )
			{
				InvalidateRect( m_hTabBtn2[i], NULL, TRUE );
			}
		}
	}
	else
	{
		std::swap(m_iCurrentTab, m_iCurrentTab2);
		ShowWindow(m_hSplitBar, SW_HIDE);
		int i=0;
		for( i=0; i<CTT_GUILD; ++i)
		{
			ShowWindow(m_hTabBtn2[i],SW_HIDE);
		}
		for( i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
		{
			EnableWindow(m_kChatTab[i].m_hTabBtn, true);
			ShowWindow(m_kChatTab[i].m_hListControl,SW_HIDE);
			if( i == m_iCurrentTab )
			{
				int	iTotal = m_kChatTab[i].m_kChatListElementCont.size();
				SendMessage(m_kChatTab[i].m_hListControl,LB_SETCARETINDEX,iTotal-1,false);
				ShowWindow(m_kChatTab[i].m_hListControl,SW_SHOW);				
			}
		}

		for( i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
		{
			if ( m_kChatTab[i].m_hTabBtn )
			{
				InvalidateRect( m_kChatTab[i].m_hTabBtn, NULL, TRUE );
			}
		}
	}
}

void	PgChatWindow::OnCreate()
{
	HFONT	hFont = CreateFont(12,0,0,0,0,0,0,0,DEFAULT_CHARSET,3,2,1,VARIABLE_PITCH | FF_ROMAN,_T("MS PGothic"));

	for(int i = 0;i<CTT_GUILD;i++)
	{
		int iBtnStyle = WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|BS_AUTORADIOBUTTON|BS_OWNERDRAW;
		if(i==CTT_SYSTEM)
		{
			iBtnStyle|=WS_GROUP;
		}

		m_kChatTab[i].m_hTabBtn = CreateWindow(_T("button"),
			TTW(400159+i).c_str(),
			iBtnStyle, 0, 0, 10, 10, m_hWnd, (HMENU)(i+1), m_hInstance, NULL);

		SendMessage(m_kChatTab[i].m_hTabBtn,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(false,0));
		SetClassLong(m_kChatTab[i].m_hTabBtn,GCL_HBRBACKGROUND,(LONG)(GetStockObject(NULL_BRUSH)));

		m_kChatTab[i].m_hListControl = CreateWindow(_T("listbox"),
			NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|LBS_NOINTEGRALHEIGHT|LBS_NOTIFY|LBS_OWNERDRAWVARIABLE|LBS_NOSEL,
			0, 0, MIN_CHAT_SIZE, 300, m_hWnd,	NULL, m_hInstance, NULL);

		ShowWindow(m_kChatTab[i].m_hListControl,SW_HIDE);

		m_hTabBtn2[i] = CreateWindow(_T("button"),
			TTW(400159+i).c_str(),
			iBtnStyle, 0, 0, 10, 10, m_hWnd, (HMENU)(i+1+CTT_MAX_WINDOW_TAB), m_hInstance, NULL);

		SendMessage(m_hTabBtn2[i],WM_SETFONT,(WPARAM)hFont,MAKELPARAM(false,0));
		SetClassLong(m_hTabBtn2[i],GCL_HBRBACKGROUND,(LONG)(GetStockObject(NULL_BRUSH)));
		ShowWindow(m_hTabBtn2[i],SW_HIDE);
	}

	m_hDivideChatRectBtn = CreateWindow(_T("button"), 
		_T(""), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
		0, 0, 21, 21, m_hWnd, (HMENU)(1001), m_hInstance, NULL);

	//m_hChangeWinDesignBtn = CreateWindow(_T("button"), 
	//	_T(""),	WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
	//	0, 0, 21, 21, m_hWnd, (HMENU)(1002), m_hInstance, NULL);

	m_hClose = CreateWindow(_T("button"), 
		_T(""), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
		0, 0, 24, 24, m_hWnd, (HMENU)(1003), m_hInstance, NULL);

	SetRect(&m_rcSplitRect, 3, (m_kWindowSize.y / 2) - 2 - (m_kBitmap[BT_TOP_BG].GetHeight()/2), m_kWindowSize.x - 3, (m_kWindowSize.y / 2) + 2 - (m_kBitmap[BT_TOP_BG].GetHeight()/2));

	m_hSplitBar = CreateWindowEx(WS_EX_TOPMOST, _T("button"), 
		_T(""),	WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
		m_rcSplitRect.left, m_rcSplitRect.top, m_rcSplitRect.right - m_rcSplitRect.left, 
		m_rcSplitRect.bottom - m_rcSplitRect.top, m_hWnd, (HMENU)(1004), m_hInstance, NULL);
	ShowWindow(m_hSplitBar, SW_HIDE);

	m_hEdit = CreateWindow(_T("edit"), 
		_T(""), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | ES_AUTOHSCROLL,
		0, 0, 14, 13, m_hWnd, (HMENU)(1005), m_hInstance, NULL);

	HFONT	NewFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
								DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
								DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Shell Dlg");
	SendMessage(m_hEdit, WM_SETFONT, (WPARAM)NewFont, (LPARAM)TRUE);
	SendMessage(m_hEdit, EM_LIMITTEXT, (WPARAM)MAX_CHAT_TEXT_LEN, 0);

	m_kOldProc = (WNDPROC)GetWindowLong(m_hSplitBar, GWL_WNDPROC);
	SetWindowLong(m_hSplitBar, GWL_WNDPROC, (LONG)SubProc);
	m_kEditProc = (WNDPROC)GetWindowLong(m_hEdit, GWL_WNDPROC);
	SetWindowLong(m_hEdit, GWL_WNDPROC, (LONG)EditProc);

	m_hBrush = CreateSolidBrush(RGB(255, 255, 0));

	CheckRadioButton(m_hWnd, 1, CTT_MAX_WINDOW_TAB, 1);

	//if( m_bIsDivide )
	//{
	m_bIsDivide =  true;
	SetDivideTab();
	//}
	//else
	//{
	//	SetTab(m_iCurrentTab);
	//}

	for( int ii=0; ii<CTT_GUILD; ++ii)
	{
		GetTextFromChatUI(ii);
	}
	SetControlPosition();
	ReBuildListContents();
}
void	PgChatWindow::AddNewString(int iTab,const XUI::CXUI_Style_String &kString, bool const bCheckEqualMessage)
{
	PG_ASSERT_LOG(iTab>=0 && iTab<CTT_MAX_WINDOW_TAB);

	if(bCheckEqualMessage)
	{
		// 가장 최근 스트링을 얻어 온다.
		int const iSize = m_kChatTab[iTab].GetElementSize();
		if(0 < iSize)
		{
			stChatListElement* pkElem = m_kChatTab[iTab].GetChatString(iSize - 1);
			if(pkElem)
			{
				// 가장 최근 스트링과 같으면 추가 하지 않음
				if(pkElem->m_kString == kString)
				{
					return;
				}
			}
		}
	}

	stChatListElement* pkElem = m_kChatTab[iTab].AddChatString(kString,m_kChatTab[iTab].m_kListControlSize.x-32);
	SendMessage(m_kChatTab[iTab].m_hListControl,LB_ADDSTRING,0,(LPARAM)pkElem);

	int	iTotal = m_kChatTab[iTab].m_kChatListElementCont.size();
	if(MAX_CHAT_LINE < iTotal)
	{
		ChatListElementCont::iterator itor = m_kChatTab[iTab].m_kChatListElementCont.begin();

		stChatListElement* pkElem = (*itor);
		SAFE_DELETE(pkElem);

		m_kChatTab[iTab].m_kChatListElementCont.erase(itor);

		SendMessage(m_kChatTab[iTab].m_hListControl,LB_DELETESTRING,0,0);
		--iTotal;
	}

	int iItemHeight = 0;
	ChatListElementCont::reverse_iterator r_iter = m_kChatTab[iTab].m_kChatListElementCont.rbegin();
	while( r_iter != m_kChatTab[iTab].m_kChatListElementCont.rend() )
	{
		iItemHeight += (*r_iter)->m_iHeight;
		if( m_kChatTab[iTab].m_kListControlSize.y < iItemHeight )
		{
			SendMessage(m_kChatTab[iTab].m_hListControl,LB_SETTOPINDEX, iTotal, 0);
			break;
		}
		--iTotal;
		++r_iter;
	}



	//SendMessage(m_kChatTab[iTab].m_hListControl,LB_SETCARETINDEX, iTotal-1,false);
}
void	PgChatWindow::OnCommand(WPARAM wParam,LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case 1001: 
		{ 
			m_bIsDivide = (m_bIsDivide)?(false):(true); 
			SetDivideTab();
			SetControlPosition();
		}
		break;
	case 1002: 
		{
			SChatLog	rkChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(66, rkChatLog);
		}
		break;
	case 1003:
		{
			TerminateWindow();
			AlignClientCenter();
		}
		break;
	case 1:
	case 2:
	case 3:
		{
			SetTab(LOWORD(wParam) - 1);
			for(int i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
			{
				if ( m_kChatTab[i].m_hTabBtn )
				{
					InvalidateRect( m_kChatTab[i].m_hTabBtn, NULL, TRUE );
				}
				
			}
		}break;
	case CTT_MAX_WINDOW_TAB+1:
	case CTT_MAX_WINDOW_TAB+2:
	case CTT_MAX_WINDOW_TAB+3:
		{
			int iPos = LOWORD(wParam) - CTT_MAX_WINDOW_TAB - 1;
			SetTab2(iPos);
			for(int i = 0; i < CTT_MAX_WINDOW_TAB; ++i)
			{
				if ( m_hTabBtn2[i] )
				{
					InvalidateRect( m_hTabBtn2[i], NULL, TRUE );
				}
				
			}
		}break;
	}
}

void	PgChatWindow::OnDrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	switch( lpDIS->CtlType )
	{
	case ODT_BUTTON:
		{
			switch( lpDIS->CtlID )
			{
			case 1: 
			case 2: 
			case 3:
				{
					SetBkMode(lpDIS->hDC, TRANSPARENT);

					SetTextColor(lpDIS->hDC, RGB(255, 255, 255));
					if( (lpDIS->CtlID - 1) == m_iCurrentTab )
					{
						m_kBitmap[BT_RDO].DrawFrame(lpDIS->hDC, 0, 0, 1, SRCCOPY);
					}
					else
					{
						m_kBitmap[BT_RDO].DrawFrame(lpDIS->hDC, 0, 0, 0, SRCCOPY);
					}

					TextOut(lpDIS->hDC, 24, 4, TTW(400159 + (lpDIS->CtlID - 1)).c_str(), TTW(400159 + (lpDIS->CtlID - 1)).size());
				}break;
			case CTT_MAX_WINDOW_TAB+1:
			case CTT_MAX_WINDOW_TAB+2:
			case CTT_MAX_WINDOW_TAB+3:
				{
					SetBkMode(lpDIS->hDC, TRANSPARENT);

					int iPos = lpDIS->CtlID - CTT_MAX_WINDOW_TAB - 1;
					SetTextColor(lpDIS->hDC, RGB(255, 255, 255));
					if( m_iCurrentTab2 == iPos)
					{
						m_kBitmap[BT_RDO].DrawFrame(lpDIS->hDC, 0, 0, 1, SRCCOPY);
					}
					else
					{
						m_kBitmap[BT_RDO].DrawFrame(lpDIS->hDC, 0, 0, 0, SRCCOPY);
					}
					TextOut(lpDIS->hDC, 24, 4, TTW(400159 + iPos).c_str(), TTW(400159 + iPos).size());
				}break;
			case 1001: 
			case 1002: 
			case 1003:
				{
					if(lpDIS->itemState & ODS_SELECTED)
					{
						m_kBitmap[BT_BTN_DIV + (lpDIS->CtlID - 1001)].DrawFrame(lpDIS->hDC, 0, 0, 2, SRCCOPY);
					}
					else 
					{ 
						m_kBitmap[BT_BTN_DIV + (lpDIS->CtlID - 1001)].DrawFrame(lpDIS->hDC, 0, 0, 0, SRCCOPY); 
					}
				}break;
			case 1004:
				{
					m_kBitmap[BT_DIV_LINE].DrawStretch(lpDIS->hDC, 0, 0, lpDIS->rcItem.right, lpDIS->rcItem.bottom, SRCCOPY);
				}break;
			}
		}
		break;
	case ODT_LISTBOX:
		{
			if(lpDIS->itemAction != 1)
			{
				return;
			}
			stChatListElement	*pkElem = (stChatListElement*)lpDIS->itemData;
			if(!pkElem)
			{
				return;
			}

			bool	bDraw = false;
			HDC	hMemDC = CreateCompatibleDC(lpDIS->hDC);

			HBITMAP	hBitmap = pkElem->m_hBitmap;
			if(hBitmap == NULL)	//	Bitmap 이 없다면 생성하자.
			{
				bDraw = true;
				hBitmap = pkElem->m_hBitmap = CreateBitmap(pkElem->m_iWidth,pkElem->m_iHeight,1,32,NULL);
			}
			HBITMAP	hOldBitmap = (HBITMAP)SelectObject(hMemDC,hBitmap);

			if(bDraw)
			{
				pkElem->m_pk2DString->Draw(hMemDC,pkElem->m_iWidth+10,pkElem->m_iHeight+10,0,0,NiColorA::WHITE);
			}

			BitBlt(lpDIS->hDC,lpDIS->rcItem.left,lpDIS->rcItem.top,pkElem->m_iWidth,pkElem->m_iHeight,hMemDC,0,0,SRCCOPY);

			SelectObject(hMemDC,hOldBitmap);
			DeleteDC(hMemDC);
		}
		break;
	default:
		return;
	}
}

void	PgChatWindow::MouseInSplit(bool bRet)
{
	if( m_bIsDivide )
	{
		if( bRet )
		{
			m_bClickSplit = true;
		}
		else
		{
			m_bClickSplit = false;
			POINT	MousePt;
			GetCursorPos(&MousePt);
			ScreenToClient(m_hWnd, &MousePt);

			int iUpLimit = m_kBitmap[BT_CAP_CENTER].GetHeight() + m_kBitmap[BT_TOP_BG].GetHeight() + m_kBitmap[BT_CORNER].GetHeight() + MINIMUM_AREA;
			int iDownLimit = m_kWindowSize.y - m_kBitmap[BT_CORNER].GetHeight() - m_kBitmap[BT_CHAT_CT].GetHeight() - MINIMUM_AREA;

			MousePt.y = (MousePt.y < iUpLimit)?(iUpLimit):(MousePt.y);
			MousePt.y = (MousePt.y > iDownLimit)?(iDownLimit):(MousePt.y);

			OffsetRect(&m_rcSplitRect, 0, MousePt.y - 2 - m_rcSplitRect.top);

			SetControlPosition();
			SetWindowPos(m_hSplitBar, NULL, m_rcSplitRect.left, m_rcSplitRect.top,
				m_rcSplitRect.right - m_rcSplitRect.left, m_rcSplitRect.bottom - m_rcSplitRect.top, 0);
		}
	}
}

void	PgChatWindow::OnMeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	stChatListElement	*pkElem = (stChatListElement*)lpMIS->itemData;
	if(!pkElem)
	{
		return;
	}

	lpMIS->itemWidth = pkElem->m_iWidth;
	lpMIS->itemHeight = pkElem->m_iHeight;
}
void	PgChatWindow::ReBuildListContents()
{
	for(int i=0;i<CTT_MAX_WINDOW_TAB;i++)
	{
		m_kChatTab[i].RebuildElements();
	}
}
void	PgChatWindow::OnEraseBackground(HDC dc)
{
	RECT	kRect;

	kRect.left = m_kBitmap[BT_CORNER].GetWidth() / 2;
	kRect.top = m_kBitmap[BT_CORNER].GetHeight() / 2;
	kRect.right = kRect.left + m_kWindowSize.x - m_kBitmap[BT_CORNER].GetWidth();
	kRect.bottom = kRect.top + m_kWindowSize.y - m_kBitmap[BT_CORNER].GetHeight();

	FillRect(dc,&kRect,(HBRUSH)GetStockObject(DKGRAY_BRUSH));
}

void	PgChatWindow::SetControlPosition()
{
	int	iBtnWidth = 21;
	int	iBtnHeight = 21;
	int nWidth = m_kBitmap[BT_CORNER].GetWidth() / 2;
	int nHeight = m_kBitmap[BT_CORNER].GetHeight() / 2;

	SetRect(&m_rcSplitRect, 3, m_rcSplitRect.top, m_kWindowSize.x - 3, m_rcSplitRect.bottom);
	SetRect(&m_rcCaption, 3, 3, m_kWindowSize.x - 3, m_kBitmap[BT_CAP_CENTER].GetHeight());

	SetWindowPos(m_hDivideChatRectBtn, 
		NULL,
		m_kWindowSize.x - 8 - iBtnWidth, 
		7 + m_kBitmap[BT_CAP_CENTER].GetHeight(),
		iBtnWidth, 
		iBtnHeight, 
		0);

	SetWindowPos(m_hClose, NULL, m_rcCaption.right - 28, m_rcCaption.top + 4, 0, 0, SWP_NOSIZE);

	SetWindowPos(m_hSplitBar, 
		NULL, 
		m_rcSplitRect.left,	m_rcSplitRect.top, 
		m_rcSplitRect.right - m_rcSplitRect.left, 
		m_rcSplitRect.bottom - m_rcSplitRect.top, 
		0);

	SetWindowPos(m_hEdit, NULL, 2 + m_kBitmap[BT_CHAT_LT].GetWidth(), 
		m_kWindowSize.y + 7 - m_kBitmap[BT_CHAT_CT].GetHeight(), 
		m_kWindowSize.x - 2 - m_kBitmap[BT_CHAT_LT].GetWidth() - 2 - m_kBitmap[BT_CHAT_RT].GetWidth(), 
		13, 0);

	iBtnWidth = 60;
	iBtnHeight = 20;

	for(int i=0;i<CTT_MAX_WINDOW_TAB;i++)
	{
		SetWindowPos(m_kChatTab[i].m_hTabBtn, 
			NULL, 
			nWidth + (iBtnWidth + 5) * i, 
			7 + m_kBitmap[BT_CAP_CENTER].GetHeight(), 
			iBtnWidth, 
			iBtnHeight, 
			0);

		SetWindowPos(m_hTabBtn2[i], 
			NULL, 
			nWidth + (iBtnWidth + 5) * i, 
			7 + m_rcSplitRect.top,
			iBtnWidth, 
			iBtnHeight, 
			0);

		m_kChatTab[i].m_kListControlSize.x = m_kWindowSize.x - m_kBitmap[BT_CORNER].GetWidth();
		int iStartY = m_kBitmap[BT_CAP_CENTER].GetHeight() + m_kBitmap[BT_TOP_BG].GetHeight() + nHeight;
		if( m_bIsDivide )
		{
			if( i == m_iCurrentTab )
			{
				m_kChatTab[i].m_kListControlSize.y = m_rcSplitRect.top  - iStartY - nHeight;
			}
			else if( i == m_iCurrentTab2 )
			{
				iStartY = m_rcSplitRect.bottom + nHeight + m_kBitmap[BT_TOP_BG].GetHeight();
				m_kChatTab[i].m_kListControlSize.y = m_kWindowSize.y - m_rcSplitRect.bottom - m_kBitmap[BT_CORNER].GetHeight() - m_kBitmap[BT_CHAT_LT].GetHeight() - m_kBitmap[BT_TOP_BG].GetHeight();
			}
		}
		else
		{
			m_kChatTab[i].m_kListControlSize.y = m_kWindowSize.y - m_kBitmap[BT_CAP_CENTER].GetHeight() - m_kBitmap[BT_TOP_BG].GetHeight() - m_kBitmap[BT_CORNER].GetHeight() - m_kBitmap[BT_CHAT_LT].GetHeight();
		}

		SetWindowPos(m_kChatTab[i].m_hListControl, 
			NULL, 
			nWidth, 
			iStartY, 
			m_kChatTab[i].m_kListControlSize.x, 
			m_kChatTab[i].m_kListControlSize.y, 
			0);
	}

	SetRect(&m_rcSizeRestored[SR_L], 0, BORDER_SIZE, BORDER_SIZE, m_kWindowSize.y - BORDER_SIZE );
	SetRect(&m_rcSizeRestored[SR_T], BORDER_SIZE, 0, m_kWindowSize.x - BORDER_SIZE, BORDER_SIZE);
	SetRect(&m_rcSizeRestored[SR_R], m_kWindowSize.x - BORDER_SIZE, BORDER_SIZE, m_kWindowSize.x, m_kWindowSize.y - BORDER_SIZE);
	SetRect(&m_rcSizeRestored[SR_B], BORDER_SIZE, m_kWindowSize.y - BORDER_SIZE, m_kWindowSize.x - BORDER_SIZE, m_kWindowSize.y);
	SetRect(&m_rcSizeRestored[SR_LT], 0, 0, nWidth, nHeight);
	SetRect(&m_rcSizeRestored[SR_TR], m_kWindowSize.x - nWidth, 0, m_kWindowSize.x, nHeight);
	SetRect(&m_rcSizeRestored[SR_RB], m_kWindowSize.x - nWidth, m_kWindowSize.y - nHeight, m_kWindowSize.x, m_kWindowSize.y);
	SetRect(&m_rcSizeRestored[SR_BL], 0, m_kWindowSize.y - nHeight, nWidth, m_kWindowSize.y);
}
void	PgChatWindow::TerminateWindow()
{
	for (int i = 0; i < CTT_MAX_WINDOW_TAB; i++)
	{
		m_kChatTab[i].ClearAllChatElement();
	}

	if(m_hWnd)
	{
		DestroyWindow(m_hWnd);
	}
	m_hWnd = 0;
}

void	PgChatWindow::AlignClientCenter()
{
	if (g_pkApp && g_pkApp->IsInitialized())
	{
		RECT	rcSize;
		GetWindowRect(g_pkApp->GetWindowReference(), &rcSize);
		SetWindowPos(g_pkApp->GetWindowReference(), NULL, 
			(GetSystemMetrics(SM_CXSCREEN) - rcSize.right + rcSize.left) / 2,
			(GetSystemMetrics(SM_CYSCREEN) - rcSize.bottom + rcSize.top) / 2,
			0, 0, SWP_NOSIZE);

		lwCallChatWindow(1);
	}
}

void	PgChatWindow::OnSize(int iWidth,int iHeight)
{
	m_kWindowSizing.x = iWidth;
	m_kWindowSizing.y = iHeight;

	InvalidateRect(m_hWnd, NULL, false);
}
void	PgChatWindow::OnExitSizeMove()
{
	bool	bWidthChanged = (m_kWindowSize.x != m_kWindowSizing.x);

	m_kWindowSize = m_kWindowSizing;

	if((m_kWindowSize.x < MIN_CHAT_SIZE) || (m_kWindowSize.y < MIN_CHAT_SIZE))
	{
		m_kWindowSize.x = (m_kWindowSize.x < MIN_CHAT_SIZE)?(MIN_CHAT_SIZE):(m_kWindowSize.x);
		m_kWindowSize.y = (m_kWindowSize.y < MIN_CHAT_SIZE)?(MIN_CHAT_SIZE):(m_kWindowSize.y);
		SetWindowPos(m_hWnd, NULL, 0, 0, m_kWindowSize.x, m_kWindowSize.y, SWP_NOMOVE);
	}
	AreaCheckToReSet();
	SetControlPosition();

	if(bWidthChanged)
	{
		ReBuildListContents();
	}
}

void	PgChatWindow::OnPaint()
{
	PAINTSTRUCT	ps;
	HDC	hDC = BeginPaint(m_hWnd,&ps);
	EndPaint(m_hWnd,&ps);
}

void	PgChatWindow::OnMouseMove(int iX,int iY)
{
	m_kMousePos.x = iX;
	m_kMousePos.y = iY;
}

void	PgChatWindow::OnSetCursor(WORD wHitTestCode)
{

	POINT ptMouse;

	GetCursorPos(&ptMouse);

	HWND	hWnd = WindowFromPoint(ptMouse);
	if(!hWnd)
	{
		return;
	}
	HWND	hParent = 0;
	while(hParent = GetParent(hWnd))
	{
		hWnd = hParent;

		if(hWnd == m_hWnd)
		{
			switch(wHitTestCode)
			{
			case HTBOTTOM:
			case HTTOP:
				{
					SetCursor(m_hCursorSizeVert);
				}
				break;
			case HTLEFT:
			case HTRIGHT:
				{
					SetCursor(m_hCursorSizeHoriz);
				}
				break;
			default:
				{
					SetCursor(m_kCursor);
				}
				break;
			}
			break;
		}
	}
}

LRESULT	CALLBACK	PgChatWindow::WinProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam)
{

	switch(uiMsg)
	{
	case WM_MOUSEMOVE:
		{
			g_kChatWindow.OnMouseMove(LOWORD(lParam),HIWORD(lParam));
		}
		return	0;
	case WM_CREATE:
		{
			g_kChatWindow.SetHWnd(hWnd);
			g_kChatWindow.OnCreate();
		}
		return	0;
	case WM_SIZE:
		{
			g_kChatWindow.OnSize(LOWORD(lParam),HIWORD(lParam));
		}
		break;
	case WM_EXITSIZEMOVE:
		{
			g_kChatWindow.OnExitSizeMove();
		}
		break;
	case WM_ERASEBKGND:
		{
			HDC	hDC = GetDC(hWnd);
			g_kChatWindow.DrawWindow(hDC);
			ReleaseDC(hWnd, hDC);
			return 0;
		}
		break;
	case WM_PAINT:
		{
			g_kChatWindow.OnPaint();
		}
		break;
	case WM_DRAWITEM:
		{
			g_kChatWindow.OnDrawItem((LPDRAWITEMSTRUCT)lParam);
			return	true;
		}	
		break;
	case WM_MEASUREITEM:
		{
			g_kChatWindow.OnMeasureItem((LPMEASUREITEMSTRUCT)lParam);
			return	true;
		}
		break;
	case WM_CTLCOLOREDIT:
		{
			g_kChatWindow.OnEditPaint(wParam,lParam);
		}return TRUE;
	case WM_CTLCOLORLISTBOX:
		{
			return	(LRESULT)GetStockObject(BLACK_BRUSH);
		}
		return	true;
	case WM_CTLCOLORSTATIC :
		{
			return	(LRESULT)GetStockObject(WHITE_BRUSH);
		}
	case WM_CTLCOLORBTN:
		{
			if( g_kChatWindow.GetSplitState() )
			{
			}
		}
	case WM_NCHITTEST:
		{
			return g_kChatWindow.OnNCHit(uiMsg, wParam, lParam);
		}
		break;
	case WM_COMMAND:
		{
			g_kChatWindow.OnCommand(wParam,lParam);
		}
		return	0;
	case WM_CLOSE:
		{
			g_kChatWindow.SetHWnd(0);
		}
		break;
	case WM_SETCURSOR:
		{
			g_kChatWindow.OnSetCursor(LOWORD(lParam));
		}
		return	false;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		{
			SendMessage(g_kChatWindow.GetParentHWnd(),uiMsg,wParam,lParam);
		}
		return	true;
	case WM_ACTIVATE:
		{
			//_PgOutputDebugString("OnDefault::WM_ACTIVATE(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);
			if ( LOWORD(wParam) != WA_INACTIVE )
			{
				g_kSoundMan.SetBGMVolume(0.f, true);
				g_kSoundMan.SetEffectVolume(0.f, true);
			}
		}break;
	case WM_NCACTIVATE:
		{
			//_PgOutputDebugString("OnDefault::WM_NCACTIVATE(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);
			if ( wParam == FALSE )
			{
				g_kSoundMan.SetBGMVolume(0.0f);
				g_kSoundMan.SetEffectVolume(0.0f);
			}
		}
		break;
	case WM_WINDOWPOSCHANGING:
		{
			if( g_pkApp )
			{
				LPWINDOWPOS pWindowPos = (LPWINDOWPOS)lParam;
				if( pWindowPos->flags & SWP_NOMOVE )
				{
					break;
				}

				RECT	rcWinP;
				GetWindowRect(g_pkApp->GetWindowReference(), &rcWinP);

				int iTPx = rcWinP.right;
				int iTPy = rcWinP.top;

				int const DELTA = 30;
				if( iTPx - pWindowPos->x > DELTA || iTPx - pWindowPos->x < -DELTA
					|| iTPy - pWindowPos->y > DELTA || iTPy - pWindowPos->y < -DELTA )
				{
					g_kChatWindow.SetSnap(false);
				}
				else
				{
					pWindowPos->x = iTPx;
					pWindowPos->y = iTPy;
					g_kChatWindow.SetSnap(true);
				}
			}
		}break;
	}

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

HBRUSH PgChatWindow::OnEditPaint(WPARAM wParam, LPARAM lParam)
{
	HDC		hDC = (HDC)wParam;
	HWND	hWnd = (HWND)lParam;

	SetBkColor(hDC, RGB(247, 234, 150));
	SetTextColor(hDC, m_hEditFontColor);
	return m_hBrush;
}

void PgChatWindow::OnIdle()
{
	if(!m_hWnd)
	{
		return;
	}

//	Draw();
}

void PgChatWindow::AreaCheckToReSet()
{
	if( m_kWindowSize.y < (m_rcSplitRect.bottom + m_kBitmap[BT_CORNER].GetHeight() + MINIMUM_AREA) )
	{
		SetRect(&m_rcSplitRect, 
			m_rcSplitRect.left, m_kWindowSize.y - MINIMUM_AREA - m_kBitmap[BT_CORNER].GetHeight(), 
			m_kWindowSize.x - 3, m_kWindowSize.y - MINIMUM_AREA - m_kBitmap[BT_CORNER].GetHeight() + 4);
	}
}

void PgChatWindow::DrawWindow(HDC hDC)
{
	if( !m_hWnd ){ return; }

	RECT	rcWin;
	GetClientRect(m_hWnd, &rcWin);
	int	WinX = rcWin.right - rcWin.left;
	int WinY = rcWin.bottom - rcWin.top;

	HDC	MemDC = CreateCompatibleDC(hDC);
	HBITMAP NewBit = CreateCompatibleBitmap(hDC, WinX, WinY);
	HBITMAP OldBit = static_cast<HBITMAP>(SelectObject(MemDC, NewBit));

	DrawFormBitmap(MemDC, 0, 0, WinX, WinY);

	BitBlt(hDC, 0, 0, WinX, WinY, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBit);
	DeleteObject(NewBit);
	DeleteDC(MemDC);
}

void PgChatWindow::DrawFormBitmap(HDC hDC, int iStartX, int iStartY, int iWinWidth, int iWinHeight)
{
	//	윈도우
	m_kBitmap[BT_BODY].DrawPattern(hDC, iStartX, iStartY, iWinWidth, iWinHeight, SRCCOPY, true, 0);
	m_kBitmap[BT_LR].DrawPattern(hDC, iStartX, iStartY, m_kBitmap[BT_LR].GetFWidth(), iWinHeight, SRCCOPY, true, 0);
	m_kBitmap[BT_LR].DrawPattern(hDC, iWinWidth - m_kBitmap[BT_LR].GetFWidth(), iStartY, m_kBitmap[BT_LR].GetFWidth(), iWinHeight, SRCCOPY, true, 1);
	m_kBitmap[BT_TB].DrawPattern(hDC, iStartX, iStartY, iWinWidth, m_kBitmap[BT_TB].GetFHeight(), SRCCOPY, true, 0);
	m_kBitmap[BT_TB].DrawPattern(hDC, iStartX, iWinHeight - m_kBitmap[BT_TB].GetFHeight(), iWinWidth, m_kBitmap[BT_TB].GetFHeight(), SRCCOPY, true, 1);
	m_kBitmap[BT_CORNER].DrawFrame(hDC, iStartX, iStartY, 0, SRCCOPY);
	m_kBitmap[BT_CORNER].DrawFrame(hDC, iWinWidth - m_kBitmap[BT_CORNER].GetFWidth(), iStartY, 1, SRCCOPY);
	m_kBitmap[BT_CORNER].DrawFrame(hDC, iStartX, iWinHeight - m_kBitmap[BT_CORNER].GetFHeight(), 2, SRCCOPY);
	m_kBitmap[BT_CORNER].DrawFrame(hDC, iWinWidth - m_kBitmap[BT_CORNER].GetFWidth(), iWinHeight - m_kBitmap[BT_CORNER].GetFHeight(), 3, SRCCOPY);

	//	캡션
	m_kBitmap[BT_CAP_CENTER].DrawPattern(hDC, iStartX + 3, iStartY + 3, iWinWidth - 6, m_kBitmap[BT_CAP_CENTER].GetFHeight(), SRCCOPY);
	m_kBitmap[BT_CAP_LR].DrawFrame(hDC, iStartX + 3, iStartY + 3, 0, SRCCOPY);
	m_kBitmap[BT_CAP_LR].DrawFrame(hDC, iWinWidth - 3 - m_kBitmap[BT_CAP_LR].GetFWidth(), iStartY + 3, 1, SRCCOPY);
	
	//	메뉴
	m_kBitmap[BT_TOP_BG].DrawPattern(hDC, iStartX + 3, iStartY + 3 + m_kBitmap[BT_CAP_CENTER].GetHeight(), iWinWidth - 6, m_kBitmap[BT_TOP_BG].GetFHeight(), SRCCOPY);

	//	Edit
	m_kBitmap[BT_CHAT_CT].DrawPattern(hDC, iStartX + 2,	iWinHeight - 2 - m_kBitmap[BT_CHAT_CT].GetHeight(), iWinWidth - 4, m_kBitmap[BT_CHAT_CT].GetHeight(), SRCCOPY);
	m_kBitmap[BT_CHAT_LT].DrawPattern(hDC, iStartX + 2, iWinHeight - 2 - m_kBitmap[BT_CHAT_LT].GetHeight(), m_kBitmap[BT_CHAT_LT].GetWidth(), m_kBitmap[BT_CHAT_LT].GetHeight(), SRCCOPY);
	m_kBitmap[BT_CHAT_RT].DrawPattern(hDC, iWinWidth - 2 - m_kBitmap[BT_CHAT_RT].GetWidth(), iWinHeight - 2 - m_kBitmap[BT_CHAT_RT].GetHeight(), m_kBitmap[BT_CHAT_RT].GetWidth(), m_kBitmap[BT_CHAT_RT].GetHeight(), SRCCOPY);

	//	DrawText
	HFONT	hFont = CreateFont(20,0,0,0,0,0,0,0,DEFAULT_CHARSET,3,2,1,VARIABLE_PITCH | FF_SWISS,_T("arial"));
	HFONT	OldFont = (HFONT)SelectObject(hDC, hFont);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkMode(hDC, TRANSPARENT);
	TextOut(hDC, m_rcCaption.left + 8, m_rcCaption.top + 4, TTW(73).c_str(), TTW(73).size());
	
	RECT	rc = { iStartX + 2, iWinHeight + 6 - m_kBitmap[BT_CHAT_CT].GetHeight(), iStartX + 2 + m_kBitmap[BT_CHAT_LT].GetWidth(), iWinHeight - 2 }; 
	SelectObject(hDC, OldFont);
	DeleteObject(hFont);



	if( !m_bIsDivide ){ return; }

	//	Split
	m_kBitmap[BT_DIV_LINE].DrawStretch(hDC, m_rcSplitRect.left, m_rcSplitRect.top, 
		m_rcSplitRect.right - m_rcSplitRect.left, m_rcSplitRect.bottom - m_rcSplitRect.top, SRCCOPY);
	
	m_kBitmap[BT_TOP_BG].DrawPattern(hDC, iStartX + 3, iStartY + 3 + m_rcSplitRect.top, iWinWidth - 6, m_kBitmap[BT_TOP_BG].GetFHeight(), SRCCOPY);
}

LRESULT	CALLBACK	PgChatWindow::SubProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
	case WM_MOUSEMOVE:
		{
			if( g_kChatWindow.GetSplitState() )
			{
				g_kChatWindow.MoveSplit();
			}
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			g_kChatWindow.MouseInSplit(true);
			SetCapture(hWnd);
		}
		break;
	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			g_kChatWindow.MouseInSplit(false);
		}
		break;
	}

	return CallWindowProc(g_kChatWindow.GetOldProc(), hWnd, uiMsg, wParam, lParam);
}

void	PgChatWindow::MoveSplit()
{
	if( m_bClickSplit )
	{
		POINT	MousePt;
		GetCursorPos(&MousePt);
		ScreenToClient(m_hWnd, &MousePt);

		int iUpLimit = m_kBitmap[BT_CAP_CENTER].GetHeight() + m_kBitmap[BT_TOP_BG].GetHeight() + m_kBitmap[BT_CORNER].GetHeight() + MINIMUM_AREA;
		int iDownLimit = m_kWindowSize.y - m_kBitmap[BT_CORNER].GetHeight() - m_kBitmap[BT_CHAT_CT].GetHeight() - MINIMUM_AREA;

		MousePt.y = (MousePt.y < iUpLimit)?(iUpLimit):(MousePt.y);
		MousePt.y = (MousePt.y > iDownLimit)?(iDownLimit):(MousePt.y);

		SetWindowPos(m_hSplitBar, HWND_TOP, m_rcSplitRect.left, MousePt.y -2,
			m_rcSplitRect.right - m_rcSplitRect.left, m_rcSplitRect.bottom - m_rcSplitRect.top, 0);
	}
}

int PgChatWindow::OnNCHit(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	int nHit = DefWindowProc(m_hWnd, uiMsg, wParam, lParam);
	if( nHit == HTCLIENT )
	{
		POINT	MousePt = { LOWORD(lParam), HIWORD(lParam) };
		ScreenToClient(m_hWnd, &MousePt);

		if( PtInRect(&m_rcCaption, MousePt) )
		{
			return HTCAPTION;
		}

		for(int i = 0; i < SR_END; ++i)
		{
			if(PtInRect(&m_rcSizeRestored[i], MousePt))
			{
				switch( i )
				{
				case SR_L: { return HTLEFT;  }break;
				case SR_R: { return HTRIGHT; }break;
				case SR_T: { return HTTOP; }break;
				case SR_B: { return HTBOTTOM; }break;
				case SR_LT: { return HTTOPLEFT; }break;
				case SR_RB: { return HTBOTTOMRIGHT; }break;
				case SR_TR: { return HTTOPRIGHT; }break;
				case SR_BL: { return HTBOTTOMLEFT; }break;
				}
			}
		}
	}
	return nHit;
}

/*
void	PgChatWindow::Draw()
{
	UpdateRenderTarget();

	HDC	hDC = GetDC(m_hWnd);
	HDC	hMemDC = CreateCompatibleDC(hDC);

	BITMAP	bit;

	HBITMAP	kBitmap = this->CreateBitmap(hDC);
	HBITMAP	kOldBitmap = (HBITMAP)SelectObject(hMemDC,kBitmap);

	GetObject(kBitmap,sizeof(BITMAP),&bit);

	BitBlt(hDC,0,0,bit.bmWidth,bit.bmHeight,hMemDC,0,0,SRCCOPY);
	
	SelectObject(hMemDC,kOldBitmap);
	DeleteObject(kBitmap);

	DeleteDC(hMemDC);
	ReleaseDC(m_hWnd,hDC);
}
HBITMAP	PgChatWindow::CreateBitmap(HDC hDC)
{
	NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());

	HBITMAP	hBitmap = 0;
	BITMAP	kBitInfo;

	const	int	iWidth = m_spRenderTargetTexture->GetWidth();
	const	int	iHeight = m_spRenderTargetTexture->GetHeight();

	Ni2DBuffer	*pkSrcBuffer = m_spRenderTargetTexture->GetBuffer();
	
	hBitmap = CreateCompatibleBitmap(hDC,iWidth,iHeight);

	GetObject(hBitmap,sizeof(BITMAP),&kBitInfo);


	NiDX9SourceTextureData* pkSourceTexData = static_cast<NiDX9SourceTextureData *>(m_spRenderTargetTexture->GetRendererData());
	if(!pkSourceTexData)
	{
		return NULL;
	}
	
	LPDIRECT3DTEXTURE9 pkD3DTexture = (LPDIRECT3DTEXTURE9)pkSourceTexData->GetD3DTexture();
	if(!pkD3DTexture)
	{
		return NULL;
	}

	IDirect3DSurface9	*pkSrcSurface = NULL,*pkTargetSurface=NULL;
	pkD3DTexture->GetSurfaceLevel(0,&pkSrcSurface);
	m_pkDynamicTexture->GetSurfaceLevel(0,&pkTargetSurface);

	pkRenderer->GetD3DDevice()->GetRenderTargetData(pkSrcSurface,pkTargetSurface);

    // Lock the surface
    D3DLOCKED_RECT kLock;

    if (FAILED(pkTargetSurface->LockRect(&kLock, NULL, D3DLOCK_READONLY)))
        return NULL;

	::SetBitmapBits(hBitmap,iWidth*iHeight*4,kLock.pBits);

	pkTargetSurface->UnlockRect();

	return	hBitmap;
}
void	PgChatWindow::UpdateMemoryDC(HDC kMemDC)
{

}

void	PgChatWindow::CreateRenderTarget()
{
	NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	NiTexture::FormatPrefs kPrefs,kPrefs2;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
	kPrefs.m_eMipMapped = NiTexture::FormatPrefs::NO;
	m_spRenderTargetTexture = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer,kPrefs);
    m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderTargetTexture->GetBuffer(),pkRenderer,true);

	HRESULT	hr = pkRenderer->GetD3DDevice()->CreateTexture(uiWidth,
		uiHeight,1,0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&m_pkDynamicTexture,NULL);

}
void	PgChatWindow::UpdateRenderTarget()
{
	if(m_spRenderTargetTexture == 0)
	{
		CreateRenderTarget();
		if(m_spRenderTargetTexture == 0)
		{
			return;
		}
	}

	PgRenderer	*pkRenderer = PgRenderer::GetPgRenderer();
	if(!pkRenderer)
	{
		return;
	}

	pkRenderer->SetBackgroundColor(NiColor::BLACK);
	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup,NiRenderer::CLEAR_ALL);

	CXUI_Wnd	*pkWnd = XUIMgr.Get(_T("SKILL_WINDOW"));
	if(pkWnd)
	{
		pkWnd->Visible(true);
		pkWnd->TotalLocation(POINT3I(0,0,0));
		pkWnd->Size(m_kWindowSize.x,m_kWindowSize.y);
		g_kUIScene.Draw(pkRenderer,_T("SKILL_WINDOW"),0);
	}

	pkRenderer->EndUsingRenderTargetGroup();

}

*/

LRESULT	CALLBACK PgChatWindow::EditProc(HWND hWnd, UINT uiMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
	case WM_ERASEBKGND:
		{
			HDC	hDC = GetDC(hWnd);
			g_kChatWindow.DrawWindow(hDC);
			ReleaseDC(hWnd, hDC);
		}break;
	case WM_KEYDOWN:
		{
			switch(LOWORD(wParam))
			{
			case VK_RETURN:
				{
					int const iMaxChatLimit = 1024;
					int iSize = ::GetWindowTextLength(hWnd);
					if( iSize
					&&	iMaxChatLimit > iSize )
					{
						wchar_t szTemp[iMaxChatLimit] = {0, };
						GetWindowText(hWnd, szTemp, iMaxChatLimit);
						g_kChatMgrClient.SendChatDlg(std::wstring(szTemp));
						SetWindowText(hWnd, _T(""));
					}
					SetFocus(g_pkApp->GetWindowReference());
				}return TRUE;
			case VK_ESCAPE:
				{
				}break;
			}
		}break;
	}

	return CallWindowProc(g_kChatWindow.GetEditProc(), hWnd, uiMsg, wParam, lParam);
}

void PgChatWindow::SetChatControl(EChatType const Type, std::wstring const& kHead, DWORD const Color)
{
	if( m_hEdit )
	{
		m_hEditFontColor = Color;
		HDC	hDC = GetDC(m_hEdit);
		SendMessage(m_hWnd, WM_CTLCOLOREDIT, (WPARAM)hDC, (LPARAM)m_hEdit);
		SetWindowText(m_hEdit, kHead.c_str());
		ReleaseDC(m_hEdit, hDC);
		SetChatOutFocus();
	}
}

bool PgChatWindow::SetChatOutFocus()
{
	SetFocus(m_hEdit);
	::SendMessage(m_hEdit, WM_KEYDOWN, VK_END, NULL);
	return true;
}