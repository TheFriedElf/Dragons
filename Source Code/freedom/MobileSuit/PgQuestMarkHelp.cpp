#include "stdafx.h"
#include "Pg2DString.h"
#include "PgSoundMan.h"
#include "PgQuestMan.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgNifMan.h"
#include "PgUiScene.h"
#include "PgAction.h"
#include "PgOption.h"
#include "PgNetwork.h"
#include "PgQuestMarkHelp.h"



extern unsigned int MakeTextureScreenRect(NiScreenTexturePtr& rkTexture, NiPoint2 kTargetPos, NiPoint2 kSize, NiPoint2 const kTexPos, NiPoint2 const kLeftTop, NiPoint2 const kRightBottom);
extern bool g_kHelpQuestMark;

/////
PgQuestMarkHelp::PgQuestMarkHelp()
	: m_pkHelpBGTexture(0), m_pkHelpText(0), m_pkActorPos(0.f, 0.f, 0.f), m_bHide(false), m_fTextAddX(0.f)
{
}
PgQuestMarkHelp::~PgQuestMarkHelp()
{
	Clear();
}

void PgQuestMarkHelp::Clear()
{
	m_pkHelpBGTexture = NULL;
	SAFE_DELETE(m_pkHelpText);
	m_pkHelpText = 0;
	m_pkActorPos = NiPoint3(0.f, 0.f, 0.f);
	m_bHide = false;
	m_fTextAddX = 0.f;
}

void PgQuestMarkHelp::SetHelpText(std::wstring const &rkHelpText, std::string const& rkBGTexture, float const fTextAddX)
{
	if( rkHelpText.empty() )
	{
		return;
	}

	XUI::CXUI_Font* pkFont = g_kFontMgr.GetFont(FONT_FTCTS_16);
	PG_ASSERT_LOG(pkFont);
	if( pkFont )
	{
		XUI::PgFontDef kFontDef(pkFont, COLOR_WHITE);
		if( !m_pkHelpText )
		{
			m_pkHelpText = new Pg2DString(kFontDef, rkHelpText);
		}
		else
		{
			m_pkHelpText->SetText(kFontDef, rkHelpText);
		}
	}

	if( m_pkHelpBGTexture )
	{
		return;
	}

	NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(rkBGTexture);
	if( pTexture )
	{
		m_pkHelpBGTexture = NiNew NiScreenTexture(pTexture);
	}
	m_fTextAddX = fTextAddX;
}

void PgQuestMarkHelp::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if( !g_kHelpQuestMark )		{ return; }
	if( !pkRenderer->GetDefaultRenderTargetGroup() )	{ return; }
	if( !pkCamera )				{ return; }
	if( !m_pkHelpBGTexture )	{ return; }
	if( !m_pkHelpText )			{ return; }
	if( m_bHide )				{ return; }

	NiPoint3 const kLengthPos = m_pkActorPos - pkCamera->GetTranslate();
	if( 700.f < kLengthPos.Length() )
	{
		return;
	}

	const NiPoint3 kStarPos = m_pkActorPos + NiPoint3(0.f, 0.f, 40.f);
	float fX = 0.f, fY = 0.f;
	bool const bRet = pkCamera->WorldPtToScreenPt(kStarPos, fX, fY);
	if( !bRet ) return;

	int const iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int const iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	NiPoint2 kTargetPos(iScreenWidth * fX, iScreenHeight * (1 - fY) );
	POINT const kTextSize = m_pkHelpText->GetSize();

	m_pkHelpBGTexture->RemoveAllScreenRects();

	NiPoint2 const kLeftTop(0.f, 0.f), kRightBottom((float)iScreenWidth, (float)iScreenHeight);
	NiPoint2 kPos;

	float const fImageWidth = static_cast< float >(m_pkHelpBGTexture->GetTexture()->GetWidth());
	float const fImageHeight = static_cast< float >(m_pkHelpBGTexture->GetTexture()->GetHeight());

	kPos = NiPoint2(kTargetPos.x - (fImageWidth/2.f), kTargetPos.y);
	MakeTextureScreenRect(m_pkHelpBGTexture, kPos, NiPoint2(fImageWidth, fImageHeight), NiPoint2(0.f, 0.f), kLeftTop, kRightBottom);

	m_pkHelpBGTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	PgUIScene::Render_UIObject(pkRenderer, m_pkHelpBGTexture);

	const NiColorA kTextColor(0.95f, 0.92f, 0.78f, 1.f);
	const NiColorA kTextShadow(0.f, 0.f, 0.f, 1.f);
	kPos = NiPoint2(kTargetPos.x - (kTextSize.x/2.f) + m_fTextAddX, kTargetPos.y+8.f);
	m_pkHelpText->Draw(pkRenderer, (int)kPos.x, (int)kPos.y, kTextColor, kTextShadow, true);
}

PgQuestMarkMng::PgQuestMarkMng()
{
	m_fAccumTime = 0.f;
	m_bVisible = true;
}

PgQuestMarkMng::~PgQuestMarkMng()
{
}

void PgQuestMarkMng::DrawImmediate(PgRenderer* pkRenderer, float fFrameTime)
{
	float const fVisibleTime = 3.f;
	float const fInvisibleTime = 1.7f;

	m_fAccumTime += fFrameTime;
	if( m_bVisible
	&&	fVisibleTime < m_fAccumTime )
	{
		m_bVisible = !m_bVisible;
		m_fAccumTime = 0.f;
	}
	else if( fInvisibleTime < m_fAccumTime )
	{
		m_bVisible = !m_bVisible;
		m_fAccumTime = 0.f;
	}
	
	if( !m_bVisible )
	{
		return;
	}
	PgNodeMgr::DrawImmediate(pkRenderer, fFrameTime);
}