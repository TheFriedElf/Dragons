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

#include "PgExpeditionBalloon.h"
#include "PgPilotMan.h"
#include "lwGUID.h"
#include "PgClientParty.H"
#include "PgClientExpedition.h"

std::string const kChatBalloonExpedition = "../Data/6_UI/expedition/epTitlebg.dds";
std::string const kCharBalloonEpIcon = "../Data/6_UI/expedition/epTitleImg.tga";

PgExpeditionBalloon::PgExpeditionBalloon()
{
	Init();
}

PgExpeditionBalloon::~PgExpeditionBalloon()
{
	Destroy();
}

void PgExpeditionBalloon::Init(PgActor* pkActor)
{
	Init();
	m_pkActor = pkActor;
}

void PgExpeditionBalloon::Init()
{
	m_spScreenTexture = 0;
	m_spIconTexture = 0;
	m_kOrgText.clear();
	m_pkTitle = 0;
	//Updated(false);
	m_kExpeditionGuid.Clear();
	::SetRect(&m_kRect, 0, 0, 0, 0);
	m_bMaxMember = false;
	m_pkActor = NULL;
}

void PgExpeditionBalloon::Destroy()
{
	SAFE_DELETE(m_pkTitle);
	m_spScreenTexture = 0;
}

void PgExpeditionBalloon::SetMaxMember(bool bMember)
{
	m_bMaxMember = bMember;
}

bool PgExpeditionBalloon::GetMaxMember()
{
	return m_bMaxMember;
}

bool PgExpeditionBalloon::GetTitlePublic()
{
	return m_bTitlePublic;
}

bool PgExpeditionBalloon::SetNewTitle(std::wstring const &rkTitle, BM::GUID const &rkExpeditionGuid, bool const rkTitlePublic)
{
	LoadResource();
	if( m_pkTitle
	&&	m_kOrgText == rkTitle
	&&	m_kExpeditionGuid == rkExpeditionGuid 
	&&  m_bTitlePublic == rkTitlePublic )
	{
		return false;
	}

	m_kOrgText = rkTitle;
	m_kExpeditionGuid = rkExpeditionGuid;
	m_bTitlePublic = rkTitlePublic;

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

void PgExpeditionBalloon::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if( m_kOrgText.empty() ) return;

	if( !GetTitlePublic() ) return;

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
		kPos = kTargetPos + NiPoint2(26.f, 17.f);
		MakeTextureScreenRect(m_spIconTexture, kPos, NiPoint2(42.f, 30.f), NiPoint2(0.f, 0.f), kLeftTop, kRightBottom);
		m_spIconTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
		//PgUIScene::Render_UIObject(pkRenderer,m_spIconTexture);
		(GetMaxMember()) ? PgUIScene::Render_UIObject(pkRenderer,m_spIconTexture, pkRenderer->E_PS_GRAY) : PgUIScene::Render_UIObject(pkRenderer,m_spIconTexture);
	//}

	//XUI::CXUI_Font* pkFont = g_kFontMgr.GetFont(FONT_CHAT);
	//PG_ASSERT_LOG(pkFont);
	//if( !pkFont ) { return; }
	//g_kFontMgr.SetFont(FONT_CHAT);

	const NiColorA kTextColor(1.f, 1.f, 1.f, 1.f);
	const NiColorA kTextShadow(0.f, 0.f, 0.f, 1.f);
	NiPoint2 kTextPos = kTargetPos + NiPoint2(28.f + 39.f, 24);
	m_pkTitle->Draw(pkRenderer,(int)kTextPos.x, (int)kTextPos.y, kTextColor, kTextShadow, false);
}

bool PgExpeditionBalloon::ProcessInput(PgInput *pkInput)
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

				if( BM::GUID::NullData() == pkMyPlayer->ExpeditionGuid() )	// 원정대에 속해있지 않을 때에만 가능
				{
					SExpeditionInfo ExpeditionInfo;
					// 벌룬의 원정대GUID로 원정대정보를 찾아 Expedition에 넣어준다.
					bool const bRet = g_kExpedition.GetExpedition(m_kExpeditionGuid, ExpeditionInfo);
					if( bRet )	// 성공하면
					{
						if( ExpeditionInfo.cCurMember >= ExpeditionInfo.cMaxMember )	// 원정대 인원 초과시 참가신청 불가
						{
							return false;
						}
						if( ExpeditionInfo.ExpeditionOption.GetOptionPublicTitle() == EOT_Public )	// 원정대가 공개중이면
						{
							// 원정대장에게 가입요청해달라고 서버에 요청하기
							PgClientExpeditionUtil::Send_C_N_REQ_JOIN_EXPEDITION(lwGUID(ExpeditionInfo.ExpeditionGuid));
							return true;
						}
					}
				}
			}
		}break;
	}
	return false;
}

void PgExpeditionBalloon::LoadResource()
{
	if( !m_spScreenTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(kChatBalloonExpedition);
		if( pTexture )
		{
			m_spScreenTexture = NiNew NiScreenTexture(pTexture);
		}
	}

	if( !m_spIconTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(kCharBalloonEpIcon);
		if( pTexture )
		{
			m_spIconTexture = NiNew NiScreenTexture(pTexture);
		}
	}
}

//////////////////////////
void PgExpeditionBalloonMgr::Init()
{
	//Pre Loading Balloon Texture
	g_kNifMan.GetTexture(kChatBalloonExpedition);
	g_kNifMan.GetTexture(kCharBalloonEpIcon);
}

bool PgExpeditionBalloonMgr::ProcessInput(PgInput *pkInput)
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

void PgExpeditionBalloonMgr::DrawImmediate(PgRenderer* pkRenderer, float fFrameTime)
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
	bool bCantDraw = g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION|GATTR_FLAG_BOSS|GATTR_SUPER_GROUND|GATTR_ELEMENT_GROUND|GATTR_EVENT_GROUND);
	if( bCantDraw )
	{
		return; 
	}

	nodemgr_type::DrawImmediate(pkRenderer, fFrameTime);
}
