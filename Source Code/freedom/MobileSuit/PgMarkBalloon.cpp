#include "stdafx.h"
#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "PgMath.h"
#include "Pg2DString.h"
#include "FreedomPool.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgOption.h"
#include "PgUIScene.H"
#include "PgPilotMan.h"
#include "PgMarkBalloon.h"

std::string const PgMarkBalloon::ms_strMARK_Enemy = "../Data/6_UI/war/wrBlnR.tga";
std::string const PgMarkBalloon::ms_strMARK_MVP = "../Data/6_UI/war/wrBlnY.tga";

PgMarkBalloon::PgMarkBalloon()
{
	Init();
}

PgMarkBalloon::~PgMarkBalloon()
{
	Destroy();
}

void PgMarkBalloon::Init()
{
	m_pText = NULL;
	m_pkActor = NULL;
	m_spScreenTexture = NULL;
	m_eMarkType = T_MARK_NONE;
	m_bShow = true;
	m_iTextHeight = 0;
}

void PgMarkBalloon::Init(PgActor *pActor)
{
	Init();
	m_pkActor = pActor;
}

void PgMarkBalloon::Destroy()
{
	SAFE_DELETE(m_pText);
	m_pkActor = NULL;
	m_spScreenTexture = NULL;
}

void PgMarkBalloon::SetNewMarkBalloon( MARK_TYPE const eMarkType )
{
	LoadMarkResource( eMarkType );
	m_eMarkType = eMarkType;
}

void PgMarkBalloon::Release()
{
	m_spScreenTexture = NULL;
	SAFE_DELETE(m_pText);
	m_eMarkType = T_MARK_NONE;
}

void PgMarkBalloon::LoadMarkResource( MARK_TYPE const eMarkType )
{
	if ( m_eMarkType != eMarkType )
	{
		Release();

		DWORD dwFontColor = COLOR_BLACK;
		std::wstring wstrText;
		NiSourceTexturePtr pTexture = NULL;
		switch ( eMarkType )
		{
		case T_MARK_BATTLE_ENEMY:
			{
				pTexture = g_kNifMan.GetTexture(ms_strMARK_Enemy);
				wstrText = TTW(70098);
				m_iTextHeight = -8;
			}break;
		case T_MARK_BATTLE_MVP:
			{
				pTexture = g_kNifMan.GetTexture(ms_strMARK_MVP);
				wstrText = TTW(70099);
				m_iTextHeight = -8;
			}break;
		default:
			{
			}break;
		}

		if( pTexture )
		{
			m_spScreenTexture = NiNew NiScreenTexture(pTexture);

			XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(FONT_TEXT);
			assert(pFont);
			if( pFont )
			{
				CXUI_Style_String kSrcStyleString;
				kSrcStyleString.CreateStyleString( XUI::PgFontDef( pFont, dwFontColor ), wstrText );
				kSrcStyleString.SetOriginalString( wstrText );
				m_pText = new Pg2DString( kSrcStyleString, true, false, pTexture->GetWidth(), true );
			}
		}
	}
}

void PgMarkBalloon::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if ( m_spScreenTexture && m_pkActor && m_bShow )
	{
		float const fScreenWidth = (float)(pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0));
		float const fScreenHeight = (float)(pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0));

// 		NiPoint3 const kLengthPos = m_pkActor->GetPos() - pkCamera->GetTranslate();
// 		if( 1000.f < kLengthPos.Length() )
// 		{
// 			return;
// 		}

		//NiNodePtr spDummy = (NiNode*)m_pkActor->GetObjectByName(ATTACH_POINT_STAR);
		NiNodePtr spDummy = m_pkActor->GetNodePointStar();
		if( spDummy )
		{
			m_spScreenTexture->RemoveAllScreenRects();

			float fBx,fBy;
			if ( pkCamera->WorldPtToScreenPt( spDummy->GetWorldTranslate(), fBx, fBy ) )
			{
				NiPoint2 ptTextureSize( (float)(m_spScreenTexture->GetTexture()->GetWidth()), (float)(m_spScreenTexture->GetTexture()->GetHeight()));
				NiPoint2 ptTargetPos( fScreenWidth * fBx - (ptTextureSize.x / 2.0f), fScreenHeight * (1 - fBy) - ptTextureSize.y - 62.0f );

				NiPoint2 ptClippedAdjust;
				if ( Get_Clipped_Region(ptTargetPos, ptTextureSize, ptClippedAdjust, NiPoint2(0.0f,0.0f), NiPoint2(fScreenWidth,fScreenHeight) ) )
				{
					m_spScreenTexture->AddNewScreenRect(	(short)ptTargetPos.y, (short)ptTargetPos.x,
															(unsigned short)ptTextureSize.x, (unsigned short)ptTextureSize.y,
															(unsigned short)ptClippedAdjust.y, (unsigned short)ptClippedAdjust.x
														);
				}
				m_spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
				PgUIScene::Render_UIObject( pkRenderer, m_spScreenTexture );

				// Mark Name
				NiColorA const kTextColor(1.f, 1.f, 1.f, 1.f);
				NiColorA const kTextShadow(0.f, 0.f, 0.f, 1.f);

				int x = (int)ptTargetPos.x + (((int)ptTextureSize.x - m_pText->GetSize().x) / 2);
				int y = (int)ptTargetPos.y + (((int)ptTextureSize.y - m_pText->GetSize().y) / 2) + m_iTextHeight;
				m_pText->Draw( pkRenderer, x, y, kTextColor, kTextShadow, false);
			}
		}
	}
}

BM::GUID const &PgMarkBalloon::GetGuid()const
{
	return m_pkActor->GetPilotGuid();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class PgMarkBalloonMan
//////////////////////////////////////////////////////////////////////////////////////////////////////////

PgMarkBalloonMan::PgMarkBalloonMan()
{
	Init();
}

PgMarkBalloonMan::~PgMarkBalloonMan()
{

}

void PgMarkBalloonMan::Init()
{
	SetType(T_MARK_NONE);
}

void PgMarkBalloonMan::Clear()
{
	PgNodeMgr< PgMarkBalloon >::Clear();
	m_kContMarkPoint.clear();
	m_kMarkType = T_MARK_NONE;
}

void PgMarkBalloonMan::SetType( MARK_TYPE const kType )
{
	MARK_TYPE kAndType = kType & m_kMarkType;
	MARK_TYPE kRemoveType = kAndType ^ m_kMarkType;
	RemoveType( kRemoveType );
	m_kMarkType = kType;
}

void PgMarkBalloonMan::AddType( MARK_TYPE const kType )
{
	m_kMarkType |= kType;
}

void PgMarkBalloonMan::RemoveType( MARK_TYPE const kType )
{
	ContNode::iterator node_itr = m_kNodeList.begin();
	for ( ; node_itr!=m_kNodeList.end(); ++node_itr )
	{
		PgMarkBalloon *pkMarkBallon = *node_itr;
		if ( pkMarkBallon->GetType() & kType )
		{
			pkMarkBallon->Release();
		}
	}

	CONT_MARK_POINT::iterator mark_itr = m_kContMarkPoint.begin();
	while ( mark_itr!=m_kContMarkPoint.end() )
	{
		if ( mark_itr->second.kType & kType )
		{
			mark_itr = m_kContMarkPoint.erase( mark_itr );
		}
		else
		{
			++mark_itr;
		}
	}
}

void PgMarkBalloonMan::AddActor( PgActor *pkActor )
{
	CONT_MARK_POINT::const_iterator mark_itr = m_kContMarkPoint.find( pkActor->GetPilotGuid() );
	if ( mark_itr != m_kContMarkPoint.end() )
	{
		if ( mark_itr->second.IsDrawMark() )
		{
			pkActor->ShowMark( mark_itr->second.kType );
		}
	}
}

void PgMarkBalloonMan::AddMarkPoint( BM::GUID const &kGuid, MARK_TYPE const kMarkType, size_t iPoint )
{
	if ( kMarkType & m_kMarkType )
	{
		std::pair<CONT_MARK_POINT::iterator, bool> kPair = m_kContMarkPoint.insert( std::make_pair( kGuid, SMarkPoint() ) );
		SMarkPoint &kElement = kPair.first->second;
		if ( kPair.second )
		{
			kElement.kType = kMarkType;
			kElement.iPoint = iPoint;
		}
		else
		{
			if ( kElement.kType == kMarkType )
			{
				kElement.iPoint += iPoint;
			}
			else
			{
				kElement.kType = kMarkType;
				kElement.iPoint = iPoint;
			}
		}

		if ( kElement.IsDrawMark() )
		{
			PgActor *pkActor = g_kPilotMan.FindActor(kGuid);
			if ( pkActor )
			{
				pkActor->ShowMark( kMarkType );
			}
		}
	}
}

void PgMarkBalloonMan::RemoveMarkPoint( BM::GUID const &kGuid, MARK_TYPE const kMarkType )
{
	CONT_MARK_POINT::iterator mark_itr = m_kContMarkPoint.find( kGuid );
	if ( mark_itr != m_kContMarkPoint.end() )
	{
		if ( mark_itr->second.kType == kMarkType )
		{
			PgActor *pkActor = g_kPilotMan.FindActor(kGuid);
			if ( pkActor )
			{
				pkActor->ShowMark_Clear();
			}
		}
		m_kContMarkPoint.erase( mark_itr );
	}
}