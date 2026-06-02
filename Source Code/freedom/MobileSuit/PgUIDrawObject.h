#ifndef FREEDOM_DRAGONICA_UI_PGUIDRAWOBJECT_H
#define FREEDOM_DRAGONICA_UI_PGUIDRAWOBJECT_H

#include <string>
#include <map>
#include <list>
#include "XUI/XUI_Font.h"
#include "Pg2DString.h"

using namespace XUI;
class CXUI_FontManager;
class PgRenderer;

enum eUIDrawObject
{
	E_UDO_NONE = 0,
	E_UDO_TEXT = 1,
	E_UDO_SPRITE = 2,
	E_UDO_POOL_SPRITE = 3,
	E_UDO_POOL_TEXT = 4,
	E_UDO_STATIC_FORM = 5,
};

typedef NiScreenTexture		PgUITexture;

class PgUIDrawObject
{
public:
	PgUIDrawObject();
	virtual ~PgUIDrawObject(){	Destroy();	}
	virtual void Draw(PgRenderer *pkRenderer)=0;
	virtual PgUITexture* GetTex() = 0;

	virtual int Type(){return E_UDO_NONE;}
public:
	void SetRect(const RECT &rcIn){m_rcDrawable = rcIn;}
	const RECT& GetRect()const{return m_rcDrawable;}
	void SetDrawRect(const RECT &rcIn){m_rcDraw = rcIn;}
	const RECT& GetDrawRect()const{return m_rcDraw;}
	void SetDestPos(const D3DXVECTOR3 &rcIn){m_destPos = rcIn;}
	const D3DXVECTOR3& GetDestPos()const{return m_destPos;}
	void SetColor(const D3DCOLOR &rcIn){m_color = rcIn;}
	const D3DCOLOR& GetColor()const{return m_color;}
	bool	IsGrayScale();
	void	SetGrayScale(bool bGrayScale);
	bool	IsHighLightScale();
	void	SetHighLightScale(bool bHighLightScale);
	void	SetDeleteLock(bool bLock);
	bool	GetDeleteLock();
	virtual void Release(){};

	CLASS_DECLARATION_S(float, Scale);
	
protected:
	CLASS_DECLARATION_S(float, Alpha);

protected:

	void	Destroy();

protected:

	RECT m_rcDrawable;
	RECT m_rcDraw;
	D3DXVECTOR3 m_destPos;
	D3DCOLOR m_color;
	bool	m_bGrayScale;
	bool	m_bHighLightScale;
	bool	m_bDeleteLock;	//	이것이 true 일때, 삭제되면 안된다.
};

typedef std::list<PgUIDrawObject*> DrawList;//UIDrawObject의 리스트

struct	stUIOffscreen	:	public PgUIDrawObject
{
	POINT	m_kPos;
	POINT	m_kSize;
	NiRenderedTexturePtr	m_spRenderedTexture;
	NiScreenTexturePtr	m_spScreenTexture;

public:

	stUIOffscreen()
		:m_spScreenTexture(0),
		m_spRenderedTexture(0)
	{
	};

	virtual PgUITexture* GetTex()
	{
		return	NULL;
	}
	virtual void Draw(PgRenderer *pkRenderer);

};


class PgUISpriteObject
	:	public PgUIDrawObject
{
public:
	PgUISpriteObject(PgUITexture *pkSprite)
		:	m_pkTex(pkSprite)
		,	m_fRotationDeg(0.0f)
	{
	}

	virtual ~PgUISpriteObject()
	{
		m_pkTex = 0;
	}
	virtual void Draw(PgRenderer *pkRenderer);
	virtual PgUITexture* GetTex();

	virtual int Type(){return E_UDO_SPRITE;}
	std::wstring m_TexName;

	CLASS_DECLARATION_S(POINT2F, ScaleCenter);

	CLASS_DECLARATION( float, m_fRotationDeg, RotationDeg );
	CLASS_DECLARATION_S(POINT2F, RotationCenter);
protected:
	PgUITexture* m_pkTex;
};
class PgUIStaticFormDrawObject
	:	public PgUIDrawObject
{
public:
	PgUIStaticFormDrawObject(CXUI_StaticForm *pkStaticForm)
		:	m_pkStaticForm(pkStaticForm)
	{
		m_bScreenTextureUpdated = false;
	}

	virtual ~PgUIStaticFormDrawObject()
	{
		m_pkStaticForm = 0;
	}
	void	SetRenderInfo(const SRenderInfo& kRenderInfo)
	{
		m_kRenderInfo = kRenderInfo;
	}
	virtual void Draw(PgRenderer *pkRenderer);
	virtual PgUITexture* GetTex() {	return	0;	}
	virtual int Type(){return E_UDO_STATIC_FORM;}
	std::wstring m_TexName;

	void	UpdateScreenTexture();
	void	UpdateScreenTexturePos();

protected:
	bool	m_bScreenTextureUpdated;
	NiPoint2	m_kLastTargetPos;
	CXUI_StaticForm* m_pkStaticForm;
	SRenderInfo	m_kRenderInfo;

};

class PgUIRenderedSpriteObject
	:	public PgUISpriteObject
{
public:
	PgUIRenderedSpriteObject(PgUITexture *pkSprite, POINT2 ptSize, int staticNum)
		: PgUISpriteObject(pkSprite), m_bDrawn(false), m_ptSize(ptSize), m_iStaticNum(staticNum)
	{
	}

	virtual ~PgUIRenderedSpriteObject()
	{
	}

	virtual void Draw(PgRenderer *pkRenderer);
	bool GetDrawn() { return m_bDrawn; }
	void SetDrawn(bool bDrawn) { m_bDrawn = bDrawn; }
	bool IsSameRenderedSprite(const POINT2 &ptSize, int staticNum)
	{
		if (ptSize.x == m_ptSize.x && ptSize.y == m_ptSize.y && staticNum == m_iStaticNum)
			return true;
		return false;
	}
protected:
	bool m_bDrawn;
	POINT2 m_ptSize;
	int m_iStaticNum;
};

class PgUIUVSpriteObject
	:	public PgUISpriteObject
{
public:
	PgUIUVSpriteObject(PgUITexture *pkSprite, const SUVInfo &rkUVInfo)
		:PgUISpriteObject(pkSprite), m_kUVInfo(rkUVInfo)
	{
	}
	virtual ~PgUIUVSpriteObject(){}

	void SetUVInfo(size_t const InU, size_t const InV, size_t const InIndex)
	{
		m_kUVInfo.Set(InU, InV, InIndex);
	}

	SUVInfo& GetUVInfo()
	{
		return m_kUVInfo;
	}

private:
	SUVInfo m_kUVInfo;
};

/////////풀에 담긴 스프라이트
class PgUISpritePoolObject
	:	public PgUISpriteObject
{
public:
	typedef std::list< PgUISpritePoolObject* > SprObjlist;
public:
	PgUISpritePoolObject(PgUITexture *pkSprite, SprObjlist *pPool)
		:	PgUISpriteObject(pkSprite)
		,	m_rPool(*pPool)
	{
	}
	virtual ~PgUISpritePoolObject()
	{
	}
public:
	virtual int Type(){return E_UDO_POOL_SPRITE;}
	virtual void Release()
	{
		m_rPool.push_back(this);
	}

	SprObjlist &m_rPool;
};

class PgUITextObject
	:	public PgUIDrawObject
{
public:
	PgUITextObject(XUI::CXUI_FontManager *pFontManager);
	virtual ~PgUITextObject();
	virtual void Draw(PgRenderer *pkRenderer);
	virtual PgUITexture* GetTex() { return NULL; }
	
	void SetData(const POINT2 &ptPos, std::wstring const &wstrText, unsigned long ulDiffuseColor, unsigned long ulOutLineColor, XUI::CXUI_2DString *p2DString);
	void SetFont(std::wstring const &rkFontKey);
	virtual int Type(){return E_UDO_TEXT;}
	
	CLASS_DECLARATION_S(DWORD, TextFlag);

protected:
	POINT2 m_kptPos;
	std::wstring m_kText;
	unsigned long m_ulDiffuseColor;
	unsigned long m_ulOutLineColor;
	std::wstring m_kFontName;
	XUI::CXUI_2DString	*m_p2DString;
	Pg2DString	m_k2DString;

	XUI::CXUI_FontManager * const m_pFontManager;	//	leesg213 2006.12.01 폰트 방식 수정
};

class PgUITextPoolObject
	:	public PgUITextObject
{
public:
	typedef std::list< PgUITextPoolObject* > CONT_TEXT_POOL;
public:
	PgUITextPoolObject(XUI::CXUI_FontManager	*pFontManager, CONT_TEXT_POOL *pPool)
		:	PgUITextObject(pFontManager)
		,	m_rPool(*pPool)
	{
	}
	virtual ~PgUITextPoolObject()
	{
	}
public:
	virtual int Type(){return E_UDO_POOL_TEXT;}
	virtual void Release()
	{
		m_rPool.push_back(this);
	}

	CONT_TEXT_POOL &m_rPool;
};

extern bool IsNullRect(const RECT &rc);
#endif // FREEDOM_DRAGONICA_UI_PGUIDRAWOBJECT_H