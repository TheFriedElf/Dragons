#ifndef FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_HBITMAP_H
#define FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_HBITMAP_H

#include	<windows.h>
#include	<vector>
#include	<string>

class CHBitmap
{
protected:
	HBITMAP	m_hBitmap;
	int		m_iWidth;
	int		m_iHeight;
	int		m_iFrameX;
	int		m_iFrameY;

public:
	bool	LoadBitmapToDefault(std::wstring const& FileName, int iFx = 1, int iFy = 1);
	bool	LoadBitmapToCustum(std::wstring const& FileName, int iFx = 1, int iFy = 1);
	bool	LoadBitmapToMemory(std::vector<char> const& kData, int iFx = 1, int iFy = 1);
	bool	LoadToResource(HINSTANCE hInst, UINT ID, int iFx = 1, int iFy = 1);

	void	DrawBitmap(HDC hDC, int iX, int iY, UINT BltFlag);
	void	DrawBitmap(HDC hDC, int iX, int iY, float fPercent, UINT BltFlag);
	void	DrawFrame(HDC hDC, int iX, int iY, int iFrame, UINT BltFlag);
	void	DrawStretch(HDC hDC, int iX, int iY, int iDWidth, int iDHeight, UINT BltFlag, bool bIsFrame = false, int iFrame = 0);
	void	DrawPattern(HDC hDC, int iX, int iY, int iWidth, int iHeight, UINT BltFlag, bool bIsFrame = false, int iFrame = 0);

	int	GetWidth() const { return m_iWidth; }
	int GetHeight() const { return m_iHeight; }
	int	GetFWidth() const { return (m_iWidth > 1)?(m_iWidth / m_iFrameX):(m_iWidth); }
	int GetFHeight() const { return (m_iHeight > 1)?(m_iHeight / m_iFrameY):(m_iHeight); }

	CHBitmap(void);
	virtual ~CHBitmap(void);
};
#endif // FREEDOM_DRAGONICA_UI_OUTCHATWINDOW_HBITMAP_H