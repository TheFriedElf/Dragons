#include "stdafx.h"
#include "PgUIDrawObject.h"
#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "Pg2DString.h"
#include "PgOption.h"
#include "PgUIScene.H"
#include "xui/xui_renderer_base.h"
#include "PgMath.H"

const unsigned int iBoldColorNum = 4;

bool IsNullRect(const RECT &rc)
{
	if( rc.bottom == 0
	&&	rc.left== 0
	&&	rc.right== 0
	&&	rc.top== 0)
	{
		return true;
	}

	if( rc.bottom < 0 
	&&	rc.left< 0
	&&	rc.right< 0
	&&	rc.top< 0)
	{
		assert(0);
		return true;
	}

	if(	rc.top > rc.bottom
	||	rc.left > rc.right	)
	{
		assert(0);
		return true;
	}

	return false;
}

void	PgUIDrawObject::Destroy()
{
	if(m_bDeleteLock)
	{
		//	DrawList 에 들어있는 상태이므로 DrawList 에서 제거하자.
		g_kUIScene.RemoveFromDrawList(this);
	}
	m_bDeleteLock = false;

}
PgUIDrawObject::PgUIDrawObject()
{
	::SetRect(&m_rcDrawable,0,0,0,0);
	::SetRect(&m_rcDraw,0,0,0,0);
	::memset(&m_destPos,0,sizeof(m_destPos));
	SetColor(0);
	SetGrayScale(false);
	SetHighLightScale(false);
	m_bDeleteLock = false;
	Scale(1.0f);
	//	Alpha(1);
}
bool	PgUIDrawObject::IsGrayScale()
{	
	return	m_bGrayScale;	
}
void	PgUIDrawObject::SetGrayScale(bool bGrayScale)	
{	
	m_bGrayScale = bGrayScale;	
}
bool	PgUIDrawObject::IsHighLightScale()
{	
	return	m_bHighLightScale;	
}
void	PgUIDrawObject::SetHighLightScale(bool bHighLightScale)	
{	
	m_bHighLightScale = bHighLightScale;	
}
void	PgUIDrawObject::SetDeleteLock(bool bLock)
{
	m_bDeleteLock = bLock;
}
bool	PgUIDrawObject::GetDeleteLock()
{
	return	m_bDeleteLock;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//	class	stUIOffscreen
//////////////////////////////////////////////////////////////////////////////////////////////

void stUIOffscreen::Draw(PgRenderer *pkRenderer)
{
	NiRenderedTexturePtr	spRenderedTexture = m_spRenderedTexture;
	if(!spRenderedTexture)
	{
		return;
	}

	NiScreenTexturePtr spScreenTexture = m_spScreenTexture;
	if(!spScreenTexture)
	{
		spScreenTexture = NiNew NiScreenTexture(m_spRenderedTexture);
		spScreenTexture->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);

		spScreenTexture->AddNewScreenRect(m_kPos.y,m_kPos.x,
			spRenderedTexture->GetWidth(),spRenderedTexture->GetHeight(),
			0,0,NiColorA::WHITE);

		m_spScreenTexture = spScreenTexture;
	}
	else
	{
		spScreenTexture->SetTexture(spRenderedTexture);
	}

	NiScreenTexture::ScreenRect const & kRect = spScreenTexture->GetScreenRect(0);

	if(kRect.m_sPixLeft != m_kPos.x ||
		kRect.m_sPixTop != m_kPos.y ||
		kRect.m_usPixWidth != m_kSize.x ||
		kRect.m_usPixHeight != m_kSize.y)
	{
		spScreenTexture->RemoveAllScreenRects();

		spScreenTexture->AddNewScreenRect(m_kPos.y,m_kPos.x,
			m_kSize.x,m_kSize.y,
			0,0,NiColorA::WHITE);

		spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}

	PgUIScene::Render_UIObject(pkRenderer,spScreenTexture);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgUISpriteObject
//////////////////////////////////////////////////////////////////////////////////////////////

void PgUISpriteObject::Draw(PgRenderer *pkRenderer)
{
	DWORD dwType = PgRenderer::E_PS_NONE;
	if (IsGrayScale())
	{
		dwType = dwType | PgRenderer::E_PS_GRAY;
	}

	if (IsHighLightScale())
	{
		dwType = dwType | PgRenderer::E_PS_HIGHLIGHT;
	}

	stRenderOption kOption = stRenderOption(NiColorA(0,0,0,0),false);
	kOption.m_fScale = Scale();
	kOption.m_kScaleCenter = ScaleCenter();

	kOption.m_fRotateAngle = RotationDeg();
	kOption.m_kRotationCenter = RotationCenter();

	PgUIScene::Render_UIObject(pkRenderer,(NiAVObject*)m_pkTex,dwType,
		kOption);
}

PgUITexture* PgUISpriteObject::GetTex()
{
	return m_pkTex;
}
void	PgUIStaticFormDrawObject::UpdateScreenTexture()
{

	/*
	PgUISpriteObject	*pkSpriteObj = (PgUISpriteObject*)m_pkStaticForm->GetBodyImg();
	if(!pkSpriteObj)
	{
		return;
	}

	PgUITexture	*pkTex = pkSpriteObj->GetTex();
	if(!pkTex || !pkTex->GetTexture())
	{
		return;
	}

	int	iScreenWidth = NiRenderer::GetRenderer()->GetDefaultRenderTargetGroup()->GetWidth(0);
	int	iScreenHeight = NiRenderer::GetRenderer()->GetDefaultRenderTargetGroup()->GetHeight(0);

	pkTex->RemoveAllScreenRects();


	const	RECT&	kBodyRect = m_pkStaticForm->GetRectBody();
	const	RECT&	kTBRect = m_pkStaticForm->GetRectTB();
	const	RECT&	kLRRect = m_pkStaticForm->GetRectLR();
	const	RECT&	kCornerRect = m_pkStaticForm->GetRectCorner();

	const	POINT2	&kFormSize = m_pkStaticForm->Size();

	bool	bBodyExist = (kBodyRect.right-kBodyRect.left>0 && kBodyRect.bottom-kBodyRect.top>0);
	bool	bTBExist = (kTBRect.right-kTBRect.left>0 && kTBRect.bottom-kTBRect.top>0);
	bool	bLRExist = (kLRRect.right-kLRRect.left>0 && kLRRect.bottom-kLRRect.top>0);
	bool	bCornerExist = (kCornerRect.right-kCornerRect.left>0 && kCornerRect.bottom-kCornerRect.top>0);

	NiPoint2	kBodySize(kBodyRect.right-kBodyRect.left,kBodyRect.bottom-kBodyRect.top);
	NiPoint2	kTBSize(kTBRect.right-kTBRect.left,kTBRect.bottom-kTBRect.top);
	NiPoint2	kLRSize(kLRRect.right-kLRRect.left,kLRRect.bottom-kLRRect.top);
	NiPoint2	kCornerSize(kCornerRect.right-kCornerRect.left,kCornerRect.bottom-kCornerRect.top);

	//	가운데부터
	NiPoint2	kTargetPos(0,0);

	if(bBodyExist)
	{

		NiPoint2	kBodyFormSize;

		kBodyFormSize.x = kFormSize.x-kLRSize.x*2;
		kBodyFormSize.y = kFormSize.y-kTBSize.y*2;

		kTargetPos.x = kLRSize.x;
		kTargetPos.y = kTBSize.y;

		if(kBodyFormSize.x>0 && kBodyFormSize.y>0)
		{
			int	iHorizCount = (int)(kBodyFormSize.x/kBodySize.x);
			int	iVertCount = (int)(kBodyFormSize.y/kBodySize.y);

			int	iRemainHorizLength = kBodyFormSize.x-iHorizCount*kBodySize.x;
			int	iRemainVertLength = kBodyFormSize.y-iVertCount*kBodySize.y;

			for(int i=0;i<iVertCount;i++)
			{
				for(int j=0;j<iHorizCount;j++)
				{
					pkTex->AddNewScreenRect(kTargetPos.y+i*kBodySize.y,kTargetPos.x+j*kBodySize.x,
						kBodySize.x,kBodySize.y,kBodyRect.top,kBodyRect.left);
				}
			}

			if(iRemainVertLength>0)
			{
				for(int j=0;j<iHorizCount;j++)
				{
					pkTex->AddNewScreenRect(kTargetPos.y+iVertCount*kBodySize.y,kTargetPos.x+j*kBodySize.x,
						kBodySize.x,iRemainVertLength,kBodyRect.top,kBodyRect.left);
				}
			}

			if(iRemainHorizLength>0)
			{
				for(int i=0;i<iVertCount;i++)
				{
					pkTex->AddNewScreenRect(kTargetPos.y+i*kBodySize.y,kTargetPos.x+iHorizCount*kBodySize.x,
						iRemainHorizLength,kBodySize.y,kBodyRect.top,kBodyRect.left);
				}
			}

			if(iRemainVertLength>0 && iRemainHorizLength>0)
			{
				pkTex->AddNewScreenRect(kTargetPos.y+iVertCount*kBodySize.y,kTargetPos.x+iHorizCount*kBodySize.x,
					iRemainHorizLength,iRemainVertLength,kBodyRect.top,kBodyRect.left);
			}
		}

	}



	//	위,아래 라인
	if(kTBRect.right-kTBRect.left>0 && kTBRect.bottom-kTBRect.top>0)
	{
		kTargetPos.x = 0;
		kTargetPos.y = 0;

		NiPoint2	kBodySrcSize(kTBRect.right-kTBRect.left,kTBRect.bottom-kTBRect.top);
		NiPoint2	kBodyFormSize(kFormSize.x-kTargetPos.x*2,kBodySrcSize.y);

		if(kBodyFormSize.x>0 && kBodyFormSize.y>0)
		{
			int	iHorizCount = (int)(kBodyFormSize.x/kBodySrcSize.x);
			int	iRemainHorizLength = kBodyFormSize.x-iHorizCount*kBodySrcSize.x;

			for(int j=0;j<iHorizCount;j++)
			{
				pkTex->AddNewScreenRect(kTargetPos.y+kBodySrcSize.y,kTargetPos.x+j*kBodySrcSize.x,
					kBodySrcSize.x,kBodySrcSize.y,kTBRect.top,kTBRect.left);
				pkTex->AddNewScreenRect(kTargetPos.y+kFormSize.y-kBodySrcSize.y,kTargetPos.x+j*kBodySrcSize.x,
					kBodySrcSize.x,kBodySrcSize.y,kTBRect.top,kTBRect.left);
			}

			if(iRemainHorizLength>0)
			{
				pkTex->AddNewScreenRect(kTargetPos.y+kBodySrcSize.y,kTargetPos.x+iHorizCount*kBodySrcSize.x,
					iRemainHorizLength,kBodySrcSize.y,kTBRect.top,kTBRect.left);
				pkTex->AddNewScreenRect(kTargetPos.y+kFormSize.y-kBodySrcSize.y,kTargetPos.x+iHorizCount*kBodySrcSize.x,
					iRemainHorizLength,kBodySrcSize.y,kTBRect.top,kTBRect.left);
			}
		}
	}

	//	좌,우 라인
	if(bLRExist)
	{


		NiPoint2	kLRFormSize;

		kLRFormSize.x = kLRSize.x;
		kLRFormSize.y = kFormSize.y - kTBSize.y*2;

		kTargetPos.x = 0;
		kTargetPos.y = kTBSize.y;

		if(kLRFormSize.x>0 && kLRFormSize.y>0)
		{
			int	iVertCount = (int)(kLRFormSize.y/kLRSize.y);
			int	iRemainVertLength = kLRFormSize.y-iVertCount*kLRSize.y;

			for(int j=0;j<iVertCount;j++)
			{
				pkTex->AddNewScreenRect(kTargetPos.y+j*kLRSize.y,kTargetPos.x,
					kLRSize.x,kLRSize.y,kLRRect.top,kLRRect.left);
				pkTex->AddNewScreenRect(kTargetPos.y+j*kLRSize.y,kFormSize.x-kLRSize.x,
					kLRSize.x,kLRSize.y,kLRRect.top,kLRRect.left);
			}

			if(iRemainVertLength>0)
			{
				pkTex->AddNewScreenRect(kTargetPos.y+iVertCount*kLRSize.y,kTargetPos.x,
					kLRSize.x,iRemainVertLength,kLRRect.top,kLRRect.left);
				pkTex->AddNewScreenRect(kTargetPos.y+iVertCount*kLRSize.y,kFormSize.x-kLRSize.x,
					kLRSize.x,iRemainVertLength,kLRRect.top,kLRRect.left);
			}
		}
	}*/

	m_bScreenTextureUpdated = true;
}
void	PgUIStaticFormDrawObject::UpdateScreenTexturePos()
{
	/*NiPoint2	kOriginPos(m_pkStaticForm->TotalLocation().x,m_pkStaticForm->TotalLocation().y);

	if(m_kLastTargetPos == kOriginPos)
	{
		return;
	}

	NiPoint2	kDiff = kOriginPos-m_kLastTargetPos;

	PgUISpriteObject	*pkSpriteObj = (PgUISpriteObject*)m_pkStaticForm->GetBodyImg();
	if(!pkSpriteObj)
	{
		return;
	}

	PgUITexture	*pkTex = pkSpriteObj->GetTex();
	if(!pkTex || !pkTex->GetTexture())
	{
		return;
	}

	int	iTotalRect = pkTex->GetNumScreenRects();
	for(int i=0;i<iTotalRect;i++)
	{
		NiScreenTexture::ScreenRect &kRect = pkTex->GetScreenRect(i);

		kRect.m_sPixLeft += kDiff.x;
		kRect.m_sPixTop += kDiff.y;
	}

	pkTex->MarkAsChanged(NiScreenTexture::VERTEX_MASK);

	m_kLastTargetPos = kOriginPos;*/
	
}
void PgUIStaticFormDrawObject::Draw(PgRenderer *pkRenderer)
{
/*	DWORD dwType = PgRenderer::E_PS_NONE;
	if (IsGrayScale())
	{
		dwType = dwType | PgRenderer::E_PS_GRAY;
	}

	if (IsHighLightScale())
	{
		dwType = dwType | PgRenderer::E_PS_HIGHLIGHT;
	}

	if(!m_bScreenTextureUpdated)
	{
		UpdateScreenTexture();
	}

	UpdateScreenTexturePos();

	PgUISpriteObject	*pkSpriteObj = (PgUISpriteObject*)m_pkStaticForm->GetBodyImg();
	if(!pkSpriteObj)
	{
		return;
	}

	PgUITexture	*pkTex = pkSpriteObj->GetTex();
	if(!pkTex || !pkTex->GetTexture())
	{
		return;
	}

	PgUIScene::Render_UIObject((NiAVObject*)pkTex,dwType,stRenderOption(NiColorA(0,0,0,0),false));*/

}

void PgUIRenderedSpriteObject::Draw(PgRenderer *pkRenderer)
{
	DWORD dwType = PgRenderer::E_PS_NONE;
	if (IsGrayScale())
	{
		dwType = dwType | PgRenderer::E_PS_GRAY;
	}
	if (IsHighLightScale())
	{
		dwType = dwType | PgRenderer::E_PS_HIGHLIGHT;
	}
	PgUIScene::Render_UIObject(pkRenderer,(NiAVObject*)m_pkTex,dwType,stRenderOption(NiColorA(0,0,0,0),false));
}

//
PgUITextObject::PgUITextObject(XUI::CXUI_FontManager *pFontManager)
		:m_pFontManager(pFontManager), m_k2DString(pFontManager->GetFont(FONT_TEXT), _T("")), m_p2DString(NULL)
{
	m_ulDiffuseColor = 0xFFFFFFFF;
	m_ulOutLineColor = 0xFF000000;
}

PgUITextObject::~PgUITextObject()
{
}

void PgUITextObject::SetData(const POINT2 &ptPos, std::wstring const &wstrText, unsigned long ulDiffuseColor, unsigned long ulOutLineColor, XUI::CXUI_2DString *p2DString)
{
	m_kptPos = ptPos;
	m_kText = wstrText;
	m_ulDiffuseColor = ulDiffuseColor;
	m_ulOutLineColor = ulOutLineColor;
	m_p2DString = p2DString;
}

void PgUITextObject::SetFont(std::wstring const &rkFontKey)
{
	m_kFontName = rkFontKey;
}

void PgUITextObject::Draw(PgRenderer *pkRenderer)
{
	DWORD const dw0x = 0xFF;
	float const fDiffA = ((m_ulDiffuseColor >> 24) & dw0x) / 255.f;
	float const fDiffR = ((m_ulDiffuseColor >> 16) & dw0x) / 255.f;
	float const fDiffG = ((m_ulDiffuseColor >> 8) & dw0x) / 255.f;
	float const fDiffB = ((m_ulDiffuseColor ) & dw0x) / 255.f;

	float const fOutA = ((m_ulOutLineColor >> 24) & dw0x) / 255.f;
	float const fOutR = ((m_ulOutLineColor >> 16) & dw0x) / 255.f;
	float const fOutG = ((m_ulOutLineColor >> 8) & dw0x) / 255.f;
	float const fOutB = ((m_ulOutLineColor ) & dw0x) / 255.f;

	if(!m_p2DString)
	{
		CXUI_Font	*pkFont = g_kFontMgr.GetFont(m_kFontName);
		if(!pkFont)
		{
			return;
		}
		m_k2DString.SetText( XUI::PgFontDef(pkFont, m_ulDiffuseColor), m_kText);
		m_p2DString = &m_k2DString;
	}

	Pg2DString *pText = dynamic_cast<Pg2DString*>(m_p2DString);
	if(pText == NULL)
		return;
	
	int iXAdjust= 0;
	if( XUI::XTF_ALIGN_CENTER & TextFlag() )
	{
		iXAdjust = -pText->GetSize().x/2; 
	}
	if( XUI::XTF_ALIGN_RIGHT & TextFlag() )
	{
		iXAdjust = -pText->GetSize().x;
	}

	bool const bOutline = (0 != (XUI::XTF_OUTLINE & TextFlag()));
	//float const fColor[4] = {0, 0, 0, 1.0};
	const NiColorA kDiffuse(fDiffR, fDiffG, fDiffB, Alpha());
	const NiColorA kOutLine(fOutR, fOutG, fOutB, fOutA);
	pText->Draw(pkRenderer,m_kptPos.x+iXAdjust,m_kptPos.y, kDiffuse, kOutLine, bOutline, TextFlag());
}