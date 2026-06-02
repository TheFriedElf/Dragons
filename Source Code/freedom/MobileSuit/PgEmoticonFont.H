#ifndef FREEDOM_DRAGONICA_UI_PGEMOTICONFONT_H
#define FREEDOM_DRAGONICA_UI_PGEMOTICONFONT_H

#include "xui/XUI_Font.H"
#include "cximage/ximage.h"

class	PgEmoticonFont	:	public	XUI::CXUI_Font
{

	struct	stImageCache
	{
		int	m_iWidth,m_iHeight;
		short	*m_sColorBuffer;

		stImageCache():
			m_iWidth(0),
			m_iHeight(0),
			m_sColorBuffer(0)
		{
		};

		~stImageCache()
		{
			m_iWidth = 0;
			m_iHeight = 0;
			SAFE_DELETE_ARRAY(m_sColorBuffer);
		}
	};

	struct	stEmoticonData
	{
		int	m_iID;
		std::string	m_kPath;
		CxImage	*m_pkImage;

		int	m_iTotalFrame;
		stImageCache	**m_pkImageCache;

		stEmoticonData()
			:m_iID(0),m_pkImage(NULL),m_iTotalFrame(0),m_pkImageCache(NULL)

		{
		}
		stEmoticonData(bool const bUsePack, int const iID, char const *strPath);
		~stEmoticonData()
		{
			for(int i = 0;i < m_iTotalFrame; ++i)
			{
				stImageCache	*pkImageCache = *(m_pkImageCache+i);
				SAFE_DELETE(pkImageCache);
			}

			SAFE_DELETE_ARRAY(m_pkImageCache);
			SAFE_DELETE(m_pkImage);
		}
	};

	typedef std::map<int, stEmoticonData*>	EmoticonCont;

	EmoticonCont	m_kEmoticonCont;

public:

	PgEmoticonFont(std::wstring const &wFontKey,char const *strXMLPath);
	virtual ~PgEmoticonFont();

	virtual	int GetWidth(const TCHAR code) const;
	virtual	int GetHeight(const TCHAR code) const;
	virtual	int GetHeight() const;
	virtual	int MaxHeight(std::wstring const &text) const;

	virtual	int Draw(_ARGB16 * ptr, int const iMaxBuffSize, int const dx, int const dy, int const pitch, unsigned short const code, _ARGB16 const &color);

	virtual	bool	IsNeedAnimation(const TCHAR code) const;
	virtual int IsEmoticon()const								{ return true; }
private:

	void	ParseXML(char const *strXMLPath);
	void	ClearAllData();

	stImageCache*	GetCachedImage(stEmoticonData const* pkEmoticonData,int const iFrameNum) const;
	stImageCache*	CacheImage(stEmoticonData* pkEmoticonData,int const iFrameNum, CxImage *pkImage);

	stEmoticonData*	GetEmoticonData(int const iEmoticonID)	const;
	CxImage*	GetEmoticonImage(int const iEmoticonID)	const;
};
#endif // FREEDOM_DRAGONICA_UI_PGEMOTICONFONT_H