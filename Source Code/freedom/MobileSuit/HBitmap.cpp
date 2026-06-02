#include "stdafx.h"
#include "HBitmap.h"

CHBitmap::CHBitmap(void)
	: m_hBitmap(NULL)
	, m_iWidth(0)
	, m_iHeight(0)
{
}

CHBitmap::~CHBitmap(void)
{
	if( m_hBitmap )
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
}

bool	CHBitmap::LoadBitmapToDefault(std::wstring const& FileName, int iFx, int iFy)
{
	m_hBitmap = static_cast<HBITMAP>(::LoadImage(NULL, FileName.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
	if( m_hBitmap )
	{
		BITMAP	Info;
		::GetObject(m_hBitmap, sizeof(BITMAP), (BITMAP*)&Info);
		m_iWidth = Info.bmWidth;
		m_iHeight = Info.bmHeight;
		m_iFrameX = iFx;
		m_iFrameY = iFy;

		return	true;
	}

	return	false;		
}

bool	CHBitmap::LoadBitmapToCustum(std::wstring const& FileName, int iFx, int iFy)
{
	BITMAPFILEHEADER FileHeader;
	BITMAPINFO* BmInfo;
	DWORD dwRead;
	void* pData;

	HANDLE	hFile = CreateFile(FileName.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	ReadFile(hFile, &FileHeader, sizeof(BITMAPFILEHEADER), &dwRead, NULL);
	DWORD dwInfoSize = FileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);
	BmInfo = reinterpret_cast<BITMAPINFO*>(new BYTE[dwInfoSize]);
	ReadFile(hFile, BmInfo, dwInfoSize, &dwRead, NULL);

	m_iWidth = BmInfo->bmiHeader.biWidth;
	m_iHeight = BmInfo->bmiHeader.biHeight;
	m_iFrameX = iFx;
	m_iFrameY = iFy;
	m_hBitmap = CreateDIBSection(NULL, BmInfo, DIB_RGB_COLORS, &pData, NULL, 0);
	ReadFile(hFile, pData, BmInfo->bmiHeader.biSizeImage, &dwRead, NULL);

	delete [] BmInfo;
	CloseHandle(hFile);
	return true;	
}

bool	CHBitmap::LoadBitmapToMemory(std::vector<char> const& kData, int iFx, int iFy)
{
	if( kData.size() )
	{
		BITMAPFILEHEADER FileHeader;
		BITMAPINFO* BmInfo;
		void* pData;

		::memcpy(&FileHeader, &kData.at(0), sizeof(BITMAPFILEHEADER));

		DWORD dwInfoSize = FileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);
		BmInfo	= reinterpret_cast<BITMAPINFO*>(new BYTE[dwInfoSize]);
		::memcpy(BmInfo, &kData.at(sizeof(BITMAPFILEHEADER)), dwInfoSize);

		m_iWidth = BmInfo->bmiHeader.biWidth;
		m_iHeight = BmInfo->bmiHeader.biHeight;
		m_iFrameX = iFx;
		m_iFrameY = iFy;
		m_hBitmap = CreateDIBSection(NULL, BmInfo, DIB_RGB_COLORS, &pData, NULL, 0);
		::memcpy(pData, &kData.at(FileHeader.bfOffBits), BmInfo->bmiHeader.biSizeImage);

		delete [] BmInfo;
		return true;	
	}
	else
	{
		return false;
	}
}

bool	CHBitmap::LoadToResource(HINSTANCE hInst, UINT ID, int iFx, int iFy)
{
	m_hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(ID));
	if( m_hBitmap )
	{
		BITMAP	Info;
		::GetObject(m_hBitmap, sizeof(BITMAP), (BITMAP*)&Info);
		m_iWidth = Info.bmWidth;
		m_iHeight = Info.bmHeight;
		m_iFrameX = iFx;
		m_iFrameY = iFy;
		return true;
	}

	return false;
}

void	CHBitmap::DrawBitmap(HDC hDC, int iX, int iY, UINT BltFlag)
{
	HDC	MemDC = CreateCompatibleDC(NULL);
	HBITMAP OldBit = static_cast<HBITMAP>(::SelectObject(MemDC, m_hBitmap));
	BitBlt(hDC, iX, iY, m_iWidth, m_iHeight, MemDC, 0, 0, BltFlag);
	SelectObject(MemDC, OldBit);
	DeleteDC(MemDC);
}

void	CHBitmap::DrawBitmap(HDC hDC, int iX, int iY, float fPercent, UINT BltFlag)
{
	int	iWidth = (int)(m_iWidth * fPercent);

	HDC	MemDC = CreateCompatibleDC(NULL);
	HBITMAP OldBit = static_cast<HBITMAP>(::SelectObject(MemDC, m_hBitmap));
	BitBlt(hDC, iX, iY, iWidth, m_iHeight, MemDC, 0, 0, BltFlag);
	SelectObject(MemDC, OldBit);
	DeleteDC(MemDC);
}

void	CHBitmap::DrawFrame(HDC hDC, int iX, int iY, int iFrame, UINT BltFlag)
{
	int iWidth = (iFrame % m_iFrameX) * GetFWidth();
	int iHeight = (iFrame / m_iFrameX) * GetFHeight();

	HDC	MemDC = CreateCompatibleDC(NULL);
	HBITMAP OldBit = static_cast<HBITMAP>(::SelectObject(MemDC, m_hBitmap));
	BitBlt(hDC, iX, iY, GetFWidth(), GetFHeight(), MemDC, iWidth, iHeight, BltFlag);
	SelectObject(MemDC, OldBit);
	DeleteDC(MemDC);
}

void	CHBitmap::DrawStretch(HDC hDC, int iX, int iY, int iDWidth, int iDHeight, UINT BltFlag, bool bIsFrame, int iFrame)
{
	HDC	MemDC = CreateCompatibleDC(NULL);
	HBITMAP OldBit = static_cast<HBITMAP>(::SelectObject(MemDC, m_hBitmap));
	if(bIsFrame)
	{
		int iWidth = (iFrame % m_iFrameX) * GetFWidth();
		int iHeight = (iFrame / m_iFrameX) * GetFHeight();

		StretchBlt(hDC, iX, iY, iDWidth, iDHeight, MemDC, iWidth, iHeight, GetFWidth(), GetFHeight(), BltFlag);
	}
	else
	{
		StretchBlt(hDC, iX, iY, iDWidth, iDHeight, MemDC, 0, 0, m_iWidth, m_iHeight, BltFlag);
	}
	SelectObject(MemDC, OldBit);
	DeleteDC(MemDC);
}

void	CHBitmap::DrawPattern(HDC hDC, int iX, int iY, int iWidth, int iHeight, UINT BltFlag, bool bIsFrame, int iFrame)
{
	if( bIsFrame )
	{
		int iFrameWidth = GetFWidth();
		int iFrameHeight = GetFHeight();

		DrawStretch(hDC, iX, iY, iWidth, iHeight, BltFlag, bIsFrame, iFrame);
	}
	else
	{
		DrawStretch(hDC, iX, iY, iWidth, iHeight, BltFlag);
	}
}