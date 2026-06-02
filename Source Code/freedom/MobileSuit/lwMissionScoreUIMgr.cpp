#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgMath.h"
#include "PgUIScene.h"
#include "Variant/PgClassDefMgr.h"
#include "lwMissionScoreUIMgr.h"
#include "PgEventTimer.h"
#include "PgPilotMan.h"
#include "PgActor.h"

float const fTimeTotalInvenGetAniTime1 = 0.25f;
float const fTimeTotalInvenGetAniTime2 = 0.48f;
float const fTimeTotalInvenGetAniTime3 = 0.54f;
float const fTimeTotalInvenGetAniTime4 = 0.18f;
float const fTimeTotalInvenGetAniTime = fTimeTotalInvenGetAniTime1 + fTimeTotalInvenGetAniTime2 + fTimeTotalInvenGetAniTime3 + fTimeTotalInvenGetAniTime4;
namespace PgDropBoxUtil
{
	extern float Bias(float x, float biasAmt);
}

PgMissionScoreUIMgr::PgMissionScoreUIMgr()
{
}

PgMissionScoreUIMgr::~PgMissionScoreUIMgr()
{
}

void PgMissionScoreUIMgr::Start(E_MissionScoreType e_Type)
{
	CXUI_Wnd *pkTopWnd = XUIMgr.Get( std::wstring(_T("FRM_MISSION_SCORE")) );
	if( !pkTopWnd )
	{
		return;
	}

	PgMissionFormAnimatedMoveWnd *pkAniWnd = NULL;

	if( e_Type == MS_SENSE )
	{
		pkAniWnd = dynamic_cast<PgMissionFormAnimatedMoveWnd *>(pkTopWnd->GetControl(std::wstring(_T("FRM_ADD_SENSE"))));
	}
	else if( e_Type == MS_ABILITY )
	{
		pkAniWnd = dynamic_cast<PgMissionFormAnimatedMoveWnd *>(pkTopWnd->GetControl(std::wstring(_T("FRM_ADD_ABILITY"))));
	}

	if( !pkAniWnd )
	{
		return;
	}
	
	pkAniWnd->m_eMissionScoreType = e_Type;
	pkAniWnd->StartAni();
}

void PgMissionScoreUIMgr::MissionScoreUIUpdate(E_MissionScoreType e_Type)
{
	CXUI_Wnd *pkTopWnd = XUIMgr.Get( std::wstring(_T("FRM_MISSION_SCORE")) );
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkAniWnd = NULL;

	if( e_Type == MS_SENSE )
	{
		pkAniWnd = pkTopWnd->GetControl(std::wstring(_T("IMG_TIME_NOSCORE")));
	}
	else if( e_Type == MS_ABILITY )
	{
		pkAniWnd = pkTopWnd->GetControl(std::wstring(_T("IMG_HIT_NOSCORE")));
	}

	if( !pkAniWnd )
	{
		return;
	}

	pkAniWnd->Visible(true);
}

void lwMissionScoreTypeClear()
{
	//g_kMissionScoreUIMgr.m_eMissionScoreType = MS_NONE;
	g_kMissionScoreUIMgr.m_bScoreViewSense = false;
	g_kMissionScoreUIMgr.m_bScoreViewAbility = false;
}

void lwUpdateMissionScoreStart(int e_Type)
{
	//g_kMissionScoreUIMgr.m_eMissionScoreType = (E_MissionScoreType)e_Type;	
	g_kMissionScoreUIMgr.Start((E_MissionScoreType)e_Type);
}

void lwUpdateMissionScoreUI()
{
	g_kMissionScoreUIMgr.bUpdate(true);
	lwDrawMissionScoreUI();
}

void lwDrawMissionScoreUI()
{
	if(!g_pkWorld)
	{
		return;
	}

	CXUI_Wnd *pkTopWnd = XUIMgr.Get( std::wstring(L"FRM_MISSION_SCORE") );
	if( !pkTopWnd )
	{
		return;
	}
	
	if( GATTR_MISSION != g_pkWorld->GetAttr() )
	{
		return;
	}

	if( !g_kMissionScoreUIMgr.bUpdate() )
	{
		return;
	}

	return;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PgMissionFormAnimatedMoveWnd::PgMissionFormAnimatedMoveWnd()
{
	m_fStartAni = 0.f;
	m_bCanAni = m_bPlayMoveSound = false;	
}

PgMissionFormAnimatedMoveWnd::~PgMissionFormAnimatedMoveWnd()
{
}

void PgMissionFormAnimatedMoveWnd::StartAni()
{
	m_bCanAni = m_bPlayMoveSound = true;
	m_fStartAni = g_pkApp->GetAccumTime();	
}

bool PgMissionFormAnimatedMoveWnd::VDisplay()
{
	NiPoint3 kPoint1[4];
	NiPoint3 kPoint2[4];

	NiPoint3 kPointTemp[8];

	if( m_eMissionScoreType == MS_SENSE )
	{
		/*if( g_kMissionScoreUIMgr.m_bScoreViewSense ) 
		{
			return true;
		}*/
		kPointTemp[0] = NiPoint3(220.f, -5.f, 0.f);
		kPointTemp[1] = NiPoint3(180.f, 0.f, 0.f);
		kPointTemp[2] = NiPoint3(140.f, 0.f, 0.f);
		kPointTemp[3] = NiPoint3(110.f, 0.f, 0.f);

		kPointTemp[4] = NiPoint3(90.f, 20.f, 0.f);
		kPointTemp[5] = NiPoint3(70.f, 60.f, 0.f);
		kPointTemp[6] = NiPoint3(50.f, 110.f, 0.f);
		kPointTemp[7] = NiPoint3(88.f, 150.f, 0.f);
	}
	else if( m_eMissionScoreType == MS_ABILITY )
	{
		/*if( g_kMissionScoreUIMgr.m_bScoreViewAbility ) 
		{
			return true;
		}*/
		kPointTemp[0] = NiPoint3(220.f, 10.f, 0.f);
		kPointTemp[1] = NiPoint3(180.f, 15.f, 0.f);
		kPointTemp[2] = NiPoint3(140.f, 15.f, 0.f);
		kPointTemp[3] = NiPoint3(110.f, 15.f, 0.f);
								
		kPointTemp[4] = NiPoint3(90.f, 20.f, 0.f);
		kPointTemp[5] = NiPoint3(70.f, 40.f, 0.f);
		kPointTemp[6] = NiPoint3(50.f, 70.f, 0.f);
		kPointTemp[7] = NiPoint3(88.f, 90.f, 0.f);
	}
	else
	{
		return true;
	}

	for(int i=0;i<4;i++)
	{
		kPoint1[i] = kPointTemp[i];
		kPoint2[i] = kPointTemp[i+4];
	}

	if( !m_bCanAni )
	{
		return true;
	}

	float const fAniFrameTime = g_pkApp->GetAccumTime() - m_fStartAni;
	bool bDrawOffscreen = false;
	if( UseOffscreen() && Offscreen() && Invalidate() )
	{
		bDrawOffscreen = true;
	}

	//CXUI_Wnd::VDisplay();
	void *pImg = DefaultImg();
	if( pImg )
	{
		NiPoint3 kNow;

		float fMaxScale = 1.f;
		//float fScale = 1.f;
		if( fTimeTotalInvenGetAniTime1 > fAniFrameTime )
		{
			Bezier4(kPoint1[0], kPoint1[1], kPoint1[2], kPoint1[3], PgDropBoxUtil::Bias(fAniFrameTime/fTimeTotalInvenGetAniTime1, 0.48f), kNow);
			//m_kScale = (fAniFrameTime/fTimeTotalInvenGetAniTime1 * fMaxScale);
			m_kScale = 0.4f;
			ImgPos( POINT2(kNow.x, kNow.y) );

			if( m_bPlayMoveSound )
			{
				m_bPlayMoveSound = false;
				PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
				if( pkMyActor )
				{
					pkMyActor->PlayNewSound(NiAudioSource::TYPE_3D, "Mission_RankUI", 0.f);
				}
			}
		}
		else if( fTimeTotalInvenGetAniTime2 > fAniFrameTime )
		{
			m_bPlayMoveSound = true;
			ImgPos( POINT2(kPoint2[0].x, kPoint2[0].y) );
			m_kScale = fMaxScale;
			//m_kScale = 0.7f;
		}
		else if( fTimeTotalInvenGetAniTime3 > fAniFrameTime )
		{
			float fEleapsedTime = __max(fAniFrameTime - fTimeTotalInvenGetAniTime1 - fTimeTotalInvenGetAniTime2, 0.f);
			Bezier4(kPoint2[0], kPoint2[1], kPoint2[2], kPoint2[3], PgDropBoxUtil::Bias(fEleapsedTime/fTimeTotalInvenGetAniTime3, 0.86f), kNow);
			//m_kScale = __max(fMaxScale - (fEleapsedTime / fTimeTotalInvenGetAniTime2 * fMaxScale), 1.f);
			//m_kScale = 1.f;
			m_kScale = 0.5f;
			//ImgPos( POINT2(kNow.x, kNow.y) );
			ImgPos( POINT2(101, 31) );			
		}
		else
		{
			/*if( m_bPlayMoveSound )
			{
				m_bPlayMoveSound = false;
				PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
				if( pkMyActor )
				{
					pkMyActor->PlayNewSound(NiAudioSource::TYPE_3D, "Mission_Timeout", 0.f);
				}
			}*/
			/*int m_iRand_x = BM::Rand_Range(3) * (BM::Rand_Range(1) ? 1 : -1);
			int m_iRand_y = BM::Rand_Range(3) * (BM::Rand_Range(1) ? 1 : -1) + 14;
			ImgPos( POINT2(kPoint2[3].x+m_iRand_x, kPoint2[3].y+m_iRand_y) );*/
			m_kScale = 0.f;
			//m_kScale = (fTimeTotalInvenGetAniTime4/fAniFrameTime*1.5f);
		}

		void* pOffscreen = Offscreen();
		SRenderInfo kRenderInfo;
		SSizedScale &rSS = kRenderInfo.kSizedScale;

		kRenderInfo.bGrayScale = GrayScale();

		rSS.ptSrcSize = ImgSize();
		rSS.ptDrawSize = ImgSize()/POINT2((LONG)UVInfo().U, (LONG)UVInfo().V);
		kRenderInfo.kUVInfo = UVInfo();
		GetParentDrawRect(kRenderInfo.rcDrawable);
		if( bDrawOffscreen )
		{
			kRenderInfo.kLoc = OffscreenLocation() + ImgPos();
			kRenderInfo.fAlpha = 1.f;
		}
		else
		{
			kRenderInfo.kLoc = TotalLocation() + ImgPos();
			kRenderInfo.fAlpha = 1.f;
		}

		if( GetClip() )
		{//클리핑을 위한 렉텡글 조절
			POINT3I pt = TotalLocation();
			if(	pt.x < 0 )
			{
				// offscreen에 대해서 뭔가 해줘야 할게 있을까?
				kRenderInfo.rcClip.left = abs(pt.x);
				kRenderInfo.kLoc.x = 0;
			}
			if(	pt.y < 0 )
			{
				kRenderInfo.rcClip.top = abs(pt.y);
				kRenderInfo.kLoc.y = 0;
			}
		}
		else
		{
			SetRect(&kRenderInfo.rcClip, 0,0,0,0);
		}

		kRenderInfo.bTwinkle = IsTwinkle() && NowTwinkleOn() && !IgnoreTwinkle();
		kRenderInfo.fScale = Scale();
		
#ifdef XUI_USE_GENERAL_OFFSCREEN
		if (bDrawOffscreen)
		{
			m_spRenderer->RenderSprite( pImg, m_ImgIdx, kRenderInfo, false);
			m_kOffscreenDrawList.push_back(std::make_pair(pImg, Parent() == NULL));
		}
		else
#endif
		{
			m_spRenderer->RenderSprite( pImg, m_ImgIdx, kRenderInfo);
		}
	}

	if( fTimeTotalInvenGetAniTime < fAniFrameTime )
	{
		m_bCanAni = false;

		/*if( m_eMissionScoreType == MS_SENSE ) 
		{
			g_kMissionScoreUIMgr.m_bScoreViewSense = true;
		}
		else if( m_eMissionScoreType == MS_ABILITY ) 
		{
			g_kMissionScoreUIMgr.m_bScoreViewAbility = true;
		}
		g_kMissionScoreUIMgr.MissionScoreUIUpdate(m_eMissionScoreType);*/
	}
	return true;
}
