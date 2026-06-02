#ifndef FREEDOM_DRAGONICA_UI_PG2DSTRING_H
#define FREEDOM_DRAGONICA_UI_PG2DSTRING_H
#include "NiMain.h"
#include "XUI/XUI_Font.h"
#include "PgDynamicTextureMan.h"

extern	XUI::CXUI_2DString	*gCreate2DStringFunc(const XUI::CXUI_Style_String &kText);
extern unsigned int gCreateFontFunc(std::wstring const& wstrFontFileName, FT_Library &ftlib, FT_Face *pkFace, FT_Byte*& ptOutPtr);

class	PgRenderer;
class	PgMobileSuit;

class	Pg2DString	:	public	XUI::CXUI_2DString
{
public:
	Pg2DString(const XUI::PgFontDef &kFontDef, std::wstring const &Text)
		:	XUI::CXUI_2DString(kFontDef,Text), 
		m_bBold(false), 
		m_iCurrentCharNum(0), 
		m_fStartTime(-1.0f), 
		m_bDrawByTime(false), 
		m_fAlphaByChar(1.0f),
		m_fQuadScale(1.0f),
		m_bHaveAnimatedLetter(false),
		m_iTotalUsingTexture(0)
	{
		m_kBoldColor.sARGB = 0xf000;
		SetResource();
	};

	Pg2DString(const XUI::CXUI_Style_String &kText,
		bool bSetResource = true,
		bool bOnlySystemMem=false,
		int iLimitWidth=-1,
		bool bUseWordWrap = false)
		:	XUI::CXUI_2DString(kText), 
		m_bBold(false), 
		m_iCurrentCharNum(0), 
		m_fStartTime(-1.0f), 
		m_bDrawByTime(false), 
		m_fAlphaByChar(1.0f),
		m_fQuadScale(1.0f),
		m_bHaveAnimatedLetter(false),
		m_iTotalUsingTexture(0)
	{
		m_kBoldColor.sARGB = 0xf000;
		if(bSetResource)
		{
			SetResource(false,bOnlySystemMem,iLimitWidth,bUseWordWrap);
		}
	};

	virtual	~Pg2DString()	{	Destroy();	}

	virtual	void	Draw(int iScreenX,int iScreenY,float fR,float fG,float fB,float fA,bool bOutline = false) {}
	virtual	void	Draw(PgRenderer *pkRenderer,int iScreenX,int iScreenY,const NiColorA &kColor,const NiColorA &kOutlineColor=NiColorA(0,0,0,0),bool bOutline = false, DWORD const dwFontOption = 0);
	virtual	void	Draw(HDC dc,int iScreenWidth,int iScreenHeight,int iScreenX,int iScreenY,const NiColorA &kColor,const NiColorA &kOutlineColor=NiColorA(0,0,0,0),bool bOutline = false, DWORD const dwFontOption = 0);
	void	Draw_3DPos(PgRenderer *pkRenderer,NiPoint3 const &kTargetPos,NiCamera* pkCamera,const NiColorA &kColor,const NiColorA &kOutlineColor=NiColorA(0,0,0,0),bool bOutline = false,float fScale = 1.0f,bool bNoZWrite = false, DWORD const dwFontOption = 0);

	virtual	void	SetText(const XUI::PgFontDef &kFontDef,std::wstring const &wText,int iLimitWidth= -1,bool bUseWordWrap = false);
	virtual	void	SetText(const XUI::CXUI_Style_String &kText,int iLimitWidth= -1,bool bUseWordWrap = false);

	virtual void	SetDrawByTime(bool bValue) { m_bDrawByTime = bValue; }
	virtual void	ResetInitTime();
	virtual	bool	GetParsedCharAt(int iTargetX,int iTargetY,XUI::PgParsedChar &kfoundChar);

	void CalculateAlignLeft(DWORD const dwFontOption);

	virtual	POINT	GetSize()	{	return	m_ptSize;	}
	POINT	GetLinePosition(int iRequestedLineNum);


	CLASS_DECLARATION_S(bool, IsDrawingDone);	// 다 그렸는지

	static	POINT	CalculateOnlySize(XUI::CXUI_Style_String const &kText,int iLimitWidth = -1,bool bUseWordWrap = false);
	static	void	DestroyStatics()
	{
		m_spMaterialProperty = 0;
		m_spAlphaProperty = 0;
		m_spZBufferProperty = 0;
		m_spVertexColorProperty = 0;
	}
private:
	friend	class	PgMobileSuit;
	struct	stStringPiece
	{
		int	m_iLineNum;
		RECT	m_rtArea;
		NiPoint2	m_ptLastRenderPos;
		NiColorA	m_kColor;
		PgDynamicTexture	*m_pkDTexture;
		NiScreenTexturePtr	m_spScreenTexture;
		XUI::CXUI_Style_String	m_kString;

		bool	m_bHasAnimatedFont;

		stStringPiece()
			:m_iLineNum(0),
			m_kColor(0,0,0,0),
			m_pkDTexture(NULL),
			m_spScreenTexture(NULL),
			m_bHasAnimatedFont(false)
		{
			SetRectEmpty(&m_rtArea);
		}
		~stStringPiece()
		{
			m_spScreenTexture = 0;
			m_pkDTexture = 0;
		}
	};

	typedef std::list<stStringPiece*> STR_PIECE_LIST;
	typedef std::map< int, int > ContAlignLeft;

	int	m_iTotalUsingTexture;
	CONT_DYNAMIC_TEXTURE	m_Textures;
	STR_PIECE_LIST	m_StringPieceList;

	POINT	m_ptSize;

	NiBillboardNodePtr	m_spQuad;	//	3D 좌표로 렌더링할 때 사용된다.
	float	m_fQuadScale;	//	m_spQuad 를 생성할 때 사용된 Scale값

	bool m_bBold;
	_ARGB16	m_kBoldColor;

	int m_iCurrentCharNum;		// 현재 풀 알파로 출력 될 글자 번호
	float m_fStartTime;		// 글자가 처음 출력 되기 시작할 시간
	bool m_bDrawByTime;		// 글자를 시간차로 출력 할것인가
	float m_fAlphaByChar;	// 한 글자당 증가하는 알파치. 기본 1.0f

	ContAlignLeft m_kAlignLeft;

	bool m_bHaveAnimatedLetter;

	//	모든 Pg2DString 객체들이 렌더링시에 공통으로 사용하는 프로퍼티들
	static	NiMaterialPropertyPtr	m_spMaterialProperty;
	static	NiAlphaPropertyPtr	m_spAlphaProperty;
	static	NiZBufferPropertyPtr	m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

protected:
	void	Add_String_Piece(XUI::CXUI_Style_String &kString,PgDynamicTexture *pkTexture,const RECT &TextAreaInTexture,int iLineNum,DWORD dwFontColor=0xffffffff);
	void	WriteTextLineToTexture(const POINT &kLineStartPos, XUI::CXUI_Style_String &kText,void *pLockBuffer,int iLockPitch);
	void	WriteStringPieceToTexture(stStringPiece *pkStringPiece);

	void	Destroy();

	POINT	SetResource(bool bCalculateOnlySize=false,bool bOnlySystemMem =false,int iLimitWidth=-1,bool bUseWordWrap = false);	//	Return : 전체 텍스트의 실제 가로 세로 사이즈

	PgDynamicTexture*	CreateDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem = false);

	bool	UnlockAndCreateNewTexturePage(int iTexWidth,int iTexHeight,NiTexture::FormatPrefs &kFormat,PgDynamicTexture *&pkTexture,void *&pkLockBuff,int &iLockPitch,bool bOnlySystemMem=false);

	void	CreateQuad(float const fScale);
	NiTriShape*	CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);

	void	ProcessWordWrap(int iLimitWidth,bool bUseWordWrap);


};
#endif // FREEDOM_DRAGONICA_UI_PG2DSTRING_H