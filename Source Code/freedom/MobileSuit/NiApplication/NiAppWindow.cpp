// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2006 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net

// Precompiled Header
#include "stdafx.h"

#include <NiOS.h>

// status window and menu support
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "NiAppWindow.h"
#include "NiApplication.h"
#include "../PgAppProtect.h"

char NiAppWindow::ms_acWindowClassName[] = "Gamebryo Application";

//---------------------------------------------------------------------------
NiAppWindow::NiAppWindow(char const* pcWindowCaption, unsigned int uiWidth, 
    unsigned int uiHeight, unsigned int uiNumStatusPanes, 
    unsigned long ulWindowStyle)
{
    // text for window caption
    if (pcWindowCaption && pcWindowCaption[0])
    {
        unsigned int uiLen = strlen(pcWindowCaption) + 1;
        m_pcWindowCaption = NiAlloc(char, uiLen);
        NiStrcpy(m_pcWindowCaption, uiLen, pcWindowCaption);
    }
    else
    {
        unsigned int uiLen = strlen(ms_acWindowClassName) + 1;
        m_pcWindowCaption = NiAlloc(char, uiLen);
        NiStrcpy(m_pcWindowCaption, uiLen, ms_acWindowClassName);
    }

    // window references
    m_pWnd = 0;
    m_pRenderWnd = 0;
    m_ppStatusWnd = 0;
    
    // client window dimensions
    m_uiWidth = m_uiParentWidth = uiWidth;
    m_uiHeight = m_uiParentHeight = uiHeight;

    // client window style
    m_ulWindowStyle = ulWindowStyle;

    // number of panes in status bar
    m_uiNumStatusPanes = uiNumStatusPanes;
    m_bStatusPanes = false;
}
//---------------------------------------------------------------------------
NiAppWindow::~NiAppWindow()
{
    if (GetStatusPanesExist())
    {
        if (IsWindow(m_pWnd))
        {
            StatusWindowDestroy();
        }
        if (m_ppStatusWnd)
            NiFree(m_ppStatusWnd);
    }
    NiFree(m_pcWindowCaption);

    m_pWnd = 0;
    m_pRenderWnd = 0;
}
//---------------------------------------------------------------------------
void NiAppWindow::SetWindowCaption(char* pcCaption)
{
    NiFree(m_pcWindowCaption);
	if(NULL == pcCaption)
	{
		pcCaption = "";
	}
	const unsigned int uiStrLength = strlen(pcCaption) + 1;
    m_pcWindowCaption = NiAlloc(char, uiStrLength);
    NiStrcpy(m_pcWindowCaption, uiStrLength, pcCaption);

    // Call the appropriate function to set the window title
    SetWindowText(m_pWnd, UNI(m_pcWindowCaption));
}
//---------------------------------------------------------------------------
void NiAppWindow::StatusWindowCreate()
{

    // Make sure we can create a status window
    if (NiApplication::ms_pkApplication->GetFullscreen())
        m_uiNumStatusPanes = 0;

    if (m_uiNumStatusPanes == 0 || m_bStatusPanes)
        return;

    RECT kStatusRect;
    RECT kRect;
    int iWidth =0;
    int iHeight =0;
    INITCOMMONCONTROLSEX kICC;

	memset(&kStatusRect,0,sizeof(RECT));
	memset(&kRect,0,sizeof(RECT));
	memset(&kICC,0,sizeof(INITCOMMONCONTROLSEX));

    kICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    kICC.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&kICC);
    
    SetStatusWindowReference(CreateWindow(
        STATUSCLASSNAME, 
        L"", 
        WS_CHILD | WS_VISIBLE, 
        0, 0, 0, 0, 
        m_pWnd, 
        0, 
        NiApplication::GetInstanceReference(), 
        0));
    
    // increase parent window height by status bar height
    GetWindowRect(GetStatusWindowReference(), &kStatusRect);
    GetWindowRect(GetWindowReference(), &kRect);

    m_bStatusPanes = true;    

    iWidth = kRect.right - kRect.left;
    iHeight = kRect.bottom - kRect.top + kStatusRect.bottom 
        - kStatusRect.top;
    MoveWindow(GetWindowReference(), kRect.left, kRect.top, iWidth, iHeight,
        TRUE);
}
//---------------------------------------------------------------------------
void NiAppWindow::WriteToStatusWindow(const unsigned int uiPane, char* pcMsg) 
    const
{
    if (uiPane >= m_uiNumStatusPanes || !m_bStatusPanes)
        return;
    SendMessage(GetStatusWindowReference(), SB_SETTEXT, (WPARAM) uiPane, 
        (LPARAM) pcMsg);
    SendMessage(GetStatusWindowReference(), WM_PAINT, 0, 0); 
}
//---------------------------------------------------------------------------
void NiAppWindow::StatusWindowDestroy()
{
    if (m_uiNumStatusPanes == 0 || !m_bStatusPanes)
        return;

    RECT kStatusRect;
    RECT kRect;
    int iWidth = 0;
    int iHeight = 0;
	memset(&kStatusRect,0,sizeof(RECT));
	memset(&kRect,0,sizeof(RECT));

    GetWindowRect(GetStatusWindowReference(), &kStatusRect);
    GetWindowRect(GetWindowReference(), &kRect);

    DestroyWindow(GetStatusWindowReference());
    SetStatusWindowReference(0);
    
    m_bStatusPanes = false;

    // decrease parent window height by status bar height
    iWidth = kRect.right - kRect.left;
    iHeight = kRect.bottom - kRect.top - 
        (kStatusRect.bottom - kStatusRect.top);
    MoveWindow(GetWindowReference(), kRect.left, kRect.top, iWidth, iHeight, 
        TRUE);
}
//---------------------------------------------------------------------------
NiWindowRef NiAppWindow::CreateMainWindow(int iWinMode, NiWindowRef hWnd)
{
    NiWindowRef pWnd = 0;
    // require the renderer window to have the specified client area
	DWORD	dwVisible = WS_VISIBLE;
#if defined(USE_GAMEGUARD) || defined(USE_HACKSHIELD) // 게임가드는 게임가드가 init 된 후에 창을 보여준다.
	dwVisible = 0;
#endif

	DWORD	dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | dwVisible;
	RECT	rc = {0,0,GetWidth(),GetHeight()};
	RECT	rc2 = {0,0,0,0};
	bool	bFullscreen = NiApplication::ms_pkApplication->GetFullscreen();
	bool	bBorderless = NiApplication::ms_pkApplication->GetBorderless();

	if (bFullscreen || bBorderless)
	{
		dwWindowStyle = WS_POPUP | dwVisible;
	}

	bool bDesktopIsSmall = false;
	AdjustWindowRect(&rc, &rc2, dwWindowStyle, bDesktopIsSmall, false); //bFullscreen ? false : true);

	if (bDesktopIsSmall && bFullscreen == false)
	{
		dwWindowStyle = WS_POPUP | dwVisible;
		//rc.left = 0;
		//rc.right = GetWidth();
		//rc.top = 0;
		//rc.bottom = GetHeight();
	}

	pWnd = CreateWindow(
		UNI(GetWindowClassName()), 
        UNI(GetWindowCaption()),        
		dwWindowStyle,
		rc.left,//bDesktopIsSmall ? rc.left : (GetSystemMetrics(SM_CXSCREEN)-rc.right+rc.left)/2,
		rc.top,//bDesktopIsSmall ? rc.top :  (GetSystemMetrics(SM_CYSCREEN)-rc.bottom+rc.top)/2,
		(rc.right-rc.left),(rc.bottom-rc.top),0L,
		NULL,
		NiApplication::GetInstanceReference(),
		0L);

    SetWindowReference(pWnd);
    SetRenderWindowReference(pWnd);

	NILOG(PGLOG_WINMESSAGE, "MainWindow handle is %#X\n", pWnd);
	return pWnd;

/*

	RECT kRect = { 0, 0, GetWidth(), GetHeight() };
    AdjustWindowRect(&kRect, GetWindowStyle(), 
        NiApplication::ms_pkApplication->GetMenuID());

    unsigned int uiFlags = WS_CHILD;

    // Create window
    if (NiApplication::ms_pkApplication->GetFullscreen())
    {
        // Create window
        pWnd = CreateWindow
        (
            UNI(GetWindowClassName()), 
            UNI(GetWindowCaption()),         
            WS_POPUP | WS_CLIPCHILDREN,       
            0,                      // initial x position
            0,                      // initial y position
            kRect.right - kRect.left, // initial width
            kRect.bottom - kRect.top, // initial height
            hWnd,                   // parent window handle
            0,                   // window menu handle
            NiApplication::GetInstanceReference(), // program instance handle
            0                    // creation parameters
       );
    }
    else
    {
        uiFlags |= WS_VISIBLE;

        pWnd = CreateWindow
        (
            UNI(GetWindowClassName()), 
            UNI(GetWindowCaption()),         
            GetWindowStyle() | WS_CLIPCHILDREN,
            0,                      // initial x position
            0,                      // initial y position
            kRect.right - kRect.left, // initial width
            kRect.bottom - kRect.top, // initial height
            hWnd,                   // parent window handle
            0,                   // window menu handle
            NiApplication::GetInstanceReference(), // program instance handle
            0                    // creation parameters
       );
    }
    SetWindowReference(pWnd);

    // create status window (if requested)
    if (GetNumStatusPanes())
        StatusWindowCreate();

    // Create render window
    NiWindowRef pRenderWnd = CreateWindow
    (
    
        UNI(GetWindowClassName()), 
        0, 
        uiFlags | WS_CLIPCHILDREN, 
        0,                      // initial x position
        0,                      // initial y position
        GetWidth(),             // initial width
        GetHeight(),            // initial height
        pWnd,                   // parent window handle
        0,                   // window menu handle
        NiApplication::GetInstanceReference(), // program instance handle
        0                    // creation parameters
   );

    SetRenderWindowReference(pRenderWnd);

//	ShowWindow(GetWindowReference(), iWinMode);
//	UpdateWindow(GetWindowReference());

    return pWnd;*/
}
//---------------------------------------------------------------------------

void NiAppWindow::AdjustWindowRect(LPRECT lpWindowRect, LPRECT lpClientRect, DWORD &dwStyle, bool& bDesktopIsSmall, bool bFitToScreen, bool bOnlyCalcRect)
{
	if (lpWindowRect == NULL)
		return;

	if(false==bOnlyCalcRect)
	{
		SetWidth(lpWindowRect->right-lpWindowRect->left);
		SetHeight(lpWindowRect->bottom-lpWindowRect->top);
	}

	::AdjustWindowRect(lpWindowRect, dwStyle, false);

	HWND hWndDesktop = GetDesktopWindow();
	RECT rcDesktop;
	memset(&rcDesktop,0,sizeof(RECT));
	::GetWindowRect(hWndDesktop, &rcDesktop);
	DWORD dwPopup = dwStyle & WS_POPUP;
	
	if (dwPopup == WS_POPUP) // 전체화면 모드
	{
		*lpWindowRect = rcDesktop;
		*lpClientRect = rcDesktop;
		bDesktopIsSmall = false;
		return;
	}

	int iCaptionBarHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int iFrameWidth = ::GetSystemMetrics(SM_CXFIXEDFRAME);
	int iFrameHeight = ::GetSystemMetrics(SM_CYFIXEDFRAME);

	lpClientRect->left = lpWindowRect->left + iFrameWidth;
	lpClientRect->right = lpWindowRect->right - iFrameWidth;
	lpClientRect->top = lpWindowRect->top + iFrameHeight + iCaptionBarHeight;
	lpClientRect->bottom = lpWindowRect->bottom - iFrameHeight;

	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(MonitorFromWindow(GetWindowReference(), MONITOR_DEFAULTTONEAREST), &mi);

	unsigned int iNeedWidth = XUIMgr.GetResolutionSize().x + iFrameWidth * 2;
	unsigned int iNeedHeight = XUIMgr.GetResolutionSize().y + iFrameHeight * 2 + iCaptionBarHeight;

	bool bFullWindow = false;

	if (mi.rcWork.bottom - mi.rcWork.top <= iNeedHeight || mi.rcWork.right - mi.rcWork.left <= iNeedWidth)
	{
		bDesktopIsSmall = true;

		if( lpClientRect->right - lpClientRect->left == mi.rcWork.right - mi.rcWork.left
			&& lpClientRect->bottom - lpClientRect->top == mi.rcWork.bottom - mi.rcWork.top)
		{//게임화면 크기가 모니터 해상도와 같으면 
			bDesktopIsSmall = false;
			bFullWindow = true;
		}
	}
	else
	{
		bDesktopIsSmall = false;
	}

	if (bFitToScreen)
	{
		int iMaxWidth = mi.rcWork.right - mi.rcWork.left;
		int iMaxHeight = mi.rcWork.bottom - mi.rcWork.top;	
		int iFitWidth = 0;
		int iFitHeight = 0;
		bool bCenterX = false;
		bool bCenterY = false;

		if(false==bDesktopIsSmall)
		{
			int x = iMaxWidth/2 - (lpWindowRect->right - lpWindowRect->left)/2;
			int y = iMaxHeight/2 - (lpWindowRect->bottom - lpWindowRect->top)/2;
			if (bFullWindow)
			{
				x = 0;
				y = 0;
				iNeedWidth = mi.rcWork.right - mi.rcWork.left;
				iNeedHeight = mi.rcWork.bottom - mi.rcWork.top;
				dwStyle = (WS_OVERLAPPED | WS_VISIBLE | WS_BORDER);
			}

			SetRect(lpWindowRect, x, y, x + iNeedWidth, y + iNeedHeight);
			return;			
		}

		// 최대 size찾기
		unsigned int iMaxWidthByMaxHeight = (unsigned int)((iMaxHeight - iFrameHeight * 2 - iCaptionBarHeight) * XUIMgr.GetResolutionSize().x / XUIMgr.GetResolutionSize().y);
		unsigned int iMaxHeightByMaxWidth = (unsigned int)((iMaxWidth - iFrameWidth * 2) * XUIMgr.GetResolutionSize().y / XUIMgr.GetResolutionSize().x);		

		if (iMaxHeightByMaxWidth <= iMaxHeight)
		{
			iFitWidth = iMaxWidth;
			iFitHeight = iMaxHeightByMaxWidth;
			bCenterY = true;
		}
		else if (iMaxWidthByMaxHeight <= iMaxWidth)
		{
			iFitWidth = iMaxWidthByMaxHeight;
			iFitHeight = iMaxHeight;
			bCenterX = true;
		}
		else
		{
			PG_ASSERT_LOG(0);
		}

		// 가운데 정렬
		if (bCenterX)
		{
			lpWindowRect->left = ((iMaxWidth - iFitWidth) / 2.0f) - iFrameWidth;
			lpWindowRect->right = lpWindowRect->left + iFitWidth + iFrameWidth * 2;
		}
		else
		{
			lpWindowRect->left = 0;
			lpWindowRect->right = lpWindowRect->left + iFitWidth ;
		}		
		lpClientRect->left = lpWindowRect->left + iFrameWidth;
		lpClientRect->right = lpWindowRect->right - iFrameWidth;

		if (bCenterY)
		{
			lpWindowRect->top = (int)((iMaxHeight - (iFitHeight + iFrameHeight * 2 + iCaptionBarHeight)) / 2.0f);
			lpWindowRect->bottom = lpWindowRect->top + iFitHeight + iFrameHeight * 2 + iCaptionBarHeight;
		}
		else
		{
			lpWindowRect->top = 0;
			lpWindowRect->bottom = lpWindowRect->top + iFitHeight;
		}
		lpClientRect->top = lpWindowRect->top + iFrameHeight + iCaptionBarHeight;
		lpClientRect->bottom = lpWindowRect->bottom - iFrameHeight;
	}

	return;
}