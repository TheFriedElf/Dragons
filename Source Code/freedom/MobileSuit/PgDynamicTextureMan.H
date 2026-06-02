#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGDYNAMICTEXTRUEMAN_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGDYNAMICTEXTRUEMAN_H
#include "NiMain.h"

class	PgDynamicTextureMan;
class	PgDynamicTexture	:	public	NiMemObject
{
public:
	PgDynamicTexture();
	PgDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem = false);

	virtual	~PgDynamicTexture();

	NiSourceTexturePtr	GetTexture()	{	return	m_spDynamicTexture;	}

	void*	Lock(int &iPitch,bool bClearMem = false);
	void	UnLock();

	BYTE*	GetCopyData();

	int	GetWidth()	{	return	m_iWidth;	}
	int	GetHeight()	{	return	m_iHeight;	}
	bool SetChanged(bool bChanged = true) { m_bDirty = true; }

protected:
	void	RecreateByCopyData();

	friend class	PgDynamicTextureMan;

	NiPixelDataPtr	m_spCopyData;	
	NiSourceTexturePtr	m_spDynamicTexture;

	int	m_iWidth,m_iHeight;
	bool m_bDirty;
};

typedef std::list<PgDynamicTexture*> CONT_DYNAMIC_TEXTURE;

class	PgMobileSuit;
class	PgDynamicTextureMan
{
public:

	PgDynamicTextureMan():m_bAlive(true)
	{
	};

	PgDynamicTexture*	CreateDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem=false);
	void	ReleaseDynamicTexture(PgDynamicTexture *pTexture);
	void	RecreateAllTextureByCopyData();
	
	bool	GetAlive()	const	{	return	m_bAlive;	}

protected:
	void	Destroy();
	friend	class	PgMobileSuit;

	CONT_DYNAMIC_TEXTURE	m_Textures;

	bool	m_bAlive;
};

extern	PgDynamicTextureMan	g_DynamicTextureMan;

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGDYNAMICTEXTRUEMAN_H