#include "stdafx.h"
#include "HBitmap.h"
#include "CTButton.h"

CCTButton::CCTButton(void)
	: m_hParent(NULL)
	, m_hWnd(NULL)
	, m_iCtlID(0)
	, m_wstrBtnText(L"")
	, m_hBitmap(NULL)
{
	m_PtPos.x = 0;
	m_PtPos.y = 0;
}

CCTButton::~CCTButton(void)
{
}

void	CCTButton::InitPush(HINSTANCE Inst, HWND Parent, int CtlID, POINT ptPos, std::wstring BtnText, CHBitmap* pBitmap, UINT Flag)
{
	m_wstrBtnText = BtnText;
	m_hParent = Parent;
	m_PtPos	= ptPos;
	m_iCtlID = CtlID;
	m_hBitmap = pBitmap;

	m_hWnd = ::CreateWindow(L"button", L"",
		WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_PUSHBUTTON | Flag,
		0, 0, 10, 10, Parent, (HMENU)CtlID, Inst, NULL);

	::SetWindowPos(m_hWnd, NULL, m_PtPos.x, m_PtPos.y, m_hBitmap->GetWidth(), m_hBitmap->GetHeight(), 0);
}

void	CCTButton::InitRadio(HINSTANCE Inst, HWND Parent, int CtlID, POINT ptPos, std::wstring BtnText, CHBitmap* pBitmap, UINT Flag)
{
	m_wstrBtnText = BtnText;
	m_hParent = Parent;
	m_PtPos	= ptPos;
	m_iCtlID = CtlID;
	m_hBitmap = pBitmap;

	m_hWnd = ::CreateWindow(L"button", L"",
		WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | BS_AUTORADIOBUTTON | Flag,
		0, 0, 10, 10, Parent, (HMENU)CtlID, Inst, NULL);

	::SetWindowPos(m_hWnd, NULL, m_PtPos.x, m_PtPos.y, m_hBitmap->GetWidth(), m_hBitmap->GetHeight(), 0);
}

void	CCTButton::OwnerDraw(LPDRAWITEMSTRUCT lpDIS)
{
	if( lpDIS->CtlID != m_iCtlID )
	{
		return;
	}

	HDC	MemDC = ::CreateCompatibleDC(lpDIS->hDC);
	HBITMAP	NewBitmap = ::CreateCompatibleBitmap(lpDIS->hDC, m_hBitmap->GetWidth(), m_hBitmap->GetHeight());
	HBITMAP OldBitmap = static_cast<HBITMAP>(::SelectObject(MemDC, NewBitmap));
	HFONT	NewFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
								DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
								DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Shell Dlg");
	HFONT	OldFont = static_cast<HFONT>(::SelectObject(MemDC, NewFont));
	::SetBkMode(MemDC, TRANSPARENT);

	if( lpDIS->itemState & ODS_SELECTED )
	{
		::SetTextColor(MemDC, RGB(125, 125, 125));
		m_hBitmap->DrawBitmap(MemDC, 0, 0, SRCCOPY);
	}
	else
	{
		m_hBitmap->DrawBitmap(MemDC, 0, 0, SRCCOPY);
	}


	RECT	rcDraw = { 0, 3, m_hBitmap->GetWidth(), m_hBitmap->GetHeight() };
	::DrawText(MemDC, m_wstrBtnText.c_str(), m_wstrBtnText.size(), &rcDraw, DT_CENTER | DT_VCENTER);
	::BitBlt(lpDIS->hDC, 0, 0, m_hBitmap->GetWidth(), m_hBitmap->GetHeight(), MemDC, 0, 0, SRCCOPY);

	::SelectObject(MemDC, OldFont);
	::DeleteObject(NewFont);
	::SelectObject(MemDC, OldBitmap);
	::DeleteDC(MemDC);
}