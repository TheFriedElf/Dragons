#include "stdafx.h"

#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "PgPilot.h"
#include "PgMath.h"
#include "Pg2DString.h"
#include "FreedomPool.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgOption.h"
#include "PgUIScene.H"
#include "PgInput.h"
#include "PgQuestMan.h"

#include "PgVendorBalloon.h"
#include "PgPilotMan.h"
#include "lwGUID.h"
#include "PgClientParty.H"
#include "PgVendor.h"
#include "lwUI.h"

std::string const kChatBalloonVendor = "../Data/6_UI/shop/shnoTitlebg.dds";
std::string const kCharBalloonIcon = "../Data/6_UI/shop/shnoTitleImg.tga";

extern unsigned int MakeTextureScreenRect(NiScreenTexturePtr& rkTexture, NiPoint2 kTargetPos, NiPoint2 kSize, NiPoint2 const kTexPos, NiPoint2 const kLeftTop, NiPoint2 const kRightBottom);

PgVendorBalloon::PgVendorBalloon()
{
	Init();
}

PgVendorBalloon::~PgVendorBalloon()
{
	Destroy();
}

void PgVendorBalloon::Init(PgActor* pkActor)
{
	Init();
	m_pkActor = pkActor;
}

void PgVendorBalloon::Init()
{
	m_spScreenTexture = 0;
	m_spIconTexture = 0;
	m_kOrgText.clear();
	m_pkTitle = 0;
	//Updated(false);
	m_kOwnerGuid.Clear();
	::SetRect(&m_kRect, 0, 0, 0, 0);
	m_pkActor = NULL;
}

void PgVendorBalloon::Destroy()
{
	SAFE_DELETE(m_pkTitle);
	m_spScreenTexture = 0;
}

bool PgVendorBalloon::SetNewTitle(std::wstring const &rkTitle, BM::GUID const &rkOwnerGuid)
{
	LoadResource();
	if( m_pkTitle
	&&	m_kOrgText == rkTitle
	&&	m_kOwnerGuid == rkOwnerGuid )
	{
		return false;
	}

	m_kOrgText = rkTitle;
	m_kOwnerGuid = rkOwnerGuid;

	if( m_kOrgText.empty() )
	{
		return false;
	}

	XUI::CXUI_Font* pkFont = g_kFontMgr.GetFont(FONT_CHAT);
	PG_ASSERT_LOG(pkFont);
	if( !pkFont ) return false;

	XUI::PgFontDef kFontDef(pkFont, COLOR_WHITE);
	if( !m_pkTitle )
	{
		m_pkTitle = new Pg2DString(kFontDef, m_kOrgText);
	}
	else
	{
		m_pkTitle->SetText(kFontDef, m_kOrgText);
	}
	//Updated(true);
	return true;
}

void PgVendorBalloon::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if( m_kOrgText.empty() ) return;

	if( !m_spScreenTexture
	||	!m_spIconTexture )
	{
		//PG_ASSERT_LOG(m_spScreenTexture && "Null ScreenTexture");
		return;
	}
	if( !pkRenderer ) return;
	if( !pkRenderer->GetDefaultRenderTargetGroup() ) return;
	if( !pkCamera ) return;
	if( !m_pkTitle ) return;
	if( !m_pkActor ) return;
	//NiNodePtr spDummy = (NiNode*)m_pkActor->GetObjectByName(ATTACH_POINT_STAR);
	NiNodePtr spDummy = m_pkActor->GetNodePointStar();
	if( !spDummy ) return;

	PgPilot *pkPilot = g_kPilotMan.FindPilot(m_pkActor->GetGuid());
	if( pkPilot )
	{
		if( pkPilot->IsHide() )
		{
			return;
		}
	}

	//
	const NiFrustum &rkCameraFrustum = pkCamera->GetViewFrustum();
	const NiPoint3 kLengthPos = m_pkActor->GetPos() - pkCamera->GetTranslate();
	if( 700.f < kLengthPos.Length() )
	{
		return;
	}

	//
	NiPoint3 const kStarPos = spDummy->GetWorldTranslate() + NiPoint3(0.f, 0.f, 12.f);
	float fX = 0.f, fY = 0.f;
	bool const bRet = pkCamera->WorldPtToScreenPt(kStarPos, fX, fY);
	if( !bRet ) return;

	//if( Updated() )
	//{
	//	Updated(false);//
		//Border
		int const iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		int const iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

		NiPoint2 kTargetPos(iScreenWidth * fX, iScreenHeight * (1 - fY) );
		const POINT kTextSize = m_pkTitle->GetSize();

		kTargetPos.x -= (kTextSize.x + 56 + 40) / 2;
		kTargetPos.y -= (kTextSize.y + 57);

		m_spScreenTexture->RemoveAllScreenRects();
		m_spIconTexture->RemoveAllScreenRects();

		NiPoint2 kPos, kLeftTop(0.f, 0.f), kRightBottom((float)iScreenWidth, (float)iScreenHeight);
		//좌측
		kPos = kTargetPos;
		MakeTextureScreenRect(m_spScreenTexture, kPos, NiPoint2(28.f, 57.f), NiPoint2(0.f, 0.f), kLeftTop, kRightBottom);
		//가운데
		for(int iCur = 0; kTextSize.x + 40 > iCur; ++iCur)
		{
			kPos = kTargetPos + NiPoint2(28.f + (float)iCur, 0.f);
			MakeTextureScreenRect(m_spScreenTexture, kPos, NiPoint2(1.f, 57.f), NiPoint2(28.f, 0.f), kLeftTop, kRightBottom);
		}
		//우측
		kPos = kTargetPos + NiPoint2(28.f + (float)kTextSize.x + 40, 0.f);
		MakeTextureScreenRect(m_spScreenTexture, kPos, NiPoint2(28.f, 57.f), NiPoint2(29.f, 0.f), kLeftTop, kRightBottom);

		m_spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

		PgUIScene::Render_UIObject(pkRenderer,m_spScreenTexture);
		::SetRect(&m_kRect, (int)kTargetPos.x, (int)kTargetPos.y, (int)(kPos.x + 28.f), (int)(kTargetPos.y + 57.f));//UI 클릭 영역 계산

		//Icon
		kPos = kTargetPos + NiPoint2(26.f, 12.f);
		MakeTextureScreenRect(m_spIconTexture, kPos, NiPoint2(35.f, 33.f), NiPoint2(0.f, 0.f), kLeftTop, kRightBottom);
		m_spIconTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
		//PgUIScene::Render_UIObject(pkRenderer,m_spIconTexture);
		PgUIScene::Render_UIObject(pkRenderer,m_spIconTexture);
	//}

	//XUI::CXUI_Font* pkFont = g_kFontMgr.GetFont(FONT_CHAT);
	//PG_ASSERT_LOG(pkFont);
	//if( !pkFont ) { return; }
	//g_kFontMgr.SetFont(FONT_CHAT);

	const NiColorA kTextColor(1.f, 1.f, 1.f, 1.f);
	const NiColorA kTextShadow(0.f, 0.f, 0.f, 1.f);
	NiPoint2 kTextPos = kTargetPos + NiPoint2(28.f + 39.f, 20);
	m_pkTitle->Draw(pkRenderer,(int)kTextPos.x, (int)kTextPos.y, kTextColor, kTextShadow, false);
}

bool PgVendorBalloon::ProcessInput(PgInput *pkInput)
{
	if( m_kOrgText.empty() )
	{
		return false;
	}

	switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
	{
	case 1001://Click Left(DOWN)
	//case 1002://Click Left(UP)
		{
			const POINT3I& rkMousePos = XUIMgr.MousePos();
			POINT kPoint;
			kPoint.x = rkMousePos.x;
			kPoint.y = rkMousePos.y;
			if( 0 != PtInRect(&m_kRect, kPoint) )
			{
				PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkMyPlayer )
				{
					return false;
				}
				if( pkMyPlayer->HaveParty() )
				{//파티중이면 불가
					lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 799444, true);
					return false;
				}
				if( m_kOwnerGuid != pkMyPlayer->GetID() && false == pkMyPlayer->OpenVendor())
				{//방문 수락 메세지
					PgPilot* pkPilot = m_pkActor->GetPilot();
					if(!pkPilot)
					{
						return false;
					}
					std::wstring kCharName = pkPilot->GetName();
					std::wstring const kFormStr(TTW(799443));
					wchar_t szBuf[200] ={0,};
					wsprintfW(szBuf, kFormStr.c_str(), kCharName.c_str());
					kCharName = szBuf;
					g_kVendorMgr.OwnerGuid( m_kOwnerGuid );
					lwCallCommonMsgYesNoBox(MB(kCharName), lwPacket(), true, MBT_CONFIRM_ENTER_VENDOR);
					return true;
				}
			}
		}break;
	}
	return false;
}

void PgVendorBalloon::LoadResource()
{
	if( !m_spScreenTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(kChatBalloonVendor);
		if( pTexture )
		{
			m_spScreenTexture = NiNew NiScreenTexture(pTexture);
		}
	}

	if( !m_spIconTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(kCharBalloonIcon);
		if( pTexture )
		{
			m_spIconTexture = NiNew NiScreenTexture(pTexture);
		}
	}
}

//////////////////////////
void PgVendorBalloonMgr::Init()
{
	//Pre Loading Balloon Texture
	g_kNifMan.GetTexture(kChatBalloonVendor);
	g_kNifMan.GetTexture(kCharBalloonIcon);
}

bool PgVendorBalloonMgr::ProcessInput(PgInput *pkInput)
{
	ContNode::iterator iter = m_kNodeList.begin();
	while(m_kNodeList.end() != iter)
	{
		bool const bRet = (*iter)->ProcessInput(pkInput);
		if( bRet )
		{
			return true;
		}
		++iter;
	}
	return false;
}

void PgVendorBalloonMgr::DrawImmediate(PgRenderer* pkRenderer, float fFrameTime)
{
	if( !g_pkWorld )
	{
		return;
	}

	if( g_kQuestMan.IsQuestDialog() )
	{
		return;
	}

	// 안그리는 그라운드들
	bool bCantDraw = g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION|GATTR_FLAG_BOSS|GATTR_SUPER_GROUND|GATTR_ELEMENT_GROUND);
	if( bCantDraw )
	{
		return; 
	}

	nodemgr_type::DrawImmediate(pkRenderer, fFrameTime);
}
