#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgMath.h"
#include "PgUIScene.h"
#include "Variant/PgClassDefMgr.h"
#include "lwMonsterKillUIMgr.h"
#include "PgEventTimer.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgSoundMan.h"
#include "PgMissionComplete.h"

float const fTimeTotalInvenGetAniTime1 = 0.25f;
float const fTimeTotalInvenGetAniTime2 = 0.48f;
float const fTimeTotalInvenGetAniTime3 = 0.18f;
float const fTimeTotalInvenGetAniTime = fTimeTotalInvenGetAniTime1 + fTimeTotalInvenGetAniTime2 + fTimeTotalInvenGetAniTime3;
namespace PgDropBoxUtil
{
	extern float Bias(float x, float biasAmt);
}

namespace PgMonsterKillUIUtil
{
	std::wstring const kQuickMenuWndName(_T("QuickMenu"));
	std::wstring const kAnimatedIconWndName(_T("IMG_ANIMATED"));
	std::wstring const kKillCountWndName(_T("FRM_KILL_COUNT"));

	void MonsterKillUISetNumber(lwUIWnd& UIParent, char const* szFormName, bool const IsVisible, int const iNum)
	{
		lwUIWnd UINum = UIParent.GetControl(szFormName);
		if( UINum.IsNil() )
		{
			return;
		}
		UINum.Visible(IsVisible);

		if( UINum.IsVisible() )
		{
			UINum.SetUVIndex(iNum + 1);
		}
	}

	void MonsterKillUISetValue(lwUIWnd& UIParent, char const* szFormName, bool IsVisible, int iNum)
	{
		lwUIWnd UINumGroup = UIParent.GetControl(szFormName);
		UINumGroup.Visible(IsVisible);

		if( !UINumGroup.IsVisible() )
		{
			return;
		}

		MonsterKillUISetNumber(UINumGroup, "FRM_ONE", true, iNum % 10);

		if( iNum >= 10 )
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_TEN", true, iNum / 10 % 10);
		}
		else
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_TEN", false, 0);
		}

		if( iNum >= 100 )
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_HUNDRED", true, iNum / 100 % 10);
		}
		else
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_HUNDRED", false, 0);
		}

		if( iNum >= 1000 )
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_THOUSAND", true, iNum / 1000 % 10);
		}
		else
		{
			MonsterKillUISetNumber(UINumGroup, "FRM_THOUSAND", false, 0);
		}
	}

	void StartRewardEffect(int const iRewardItemNo)
	{
		CXUI_Wnd *pkTopWnd = XUIMgr.Get( kQuickMenuWndName );
		if( !pkTopWnd )
		{
			return;
		}

		PgUIUVSpriteObject *pkTextute = g_kUIScene.GetIconTexture( iRewardItemNo );
		if( !pkTextute )
		{
			return;
		}

		PgFormAnimatedMoveWnd *pkAniWnd = dynamic_cast<PgFormAnimatedMoveWnd *>(pkTopWnd->GetControl(kAnimatedIconWndName));
		if( !pkAniWnd )
		{
			return;
		}

		pkAniWnd->DefaultImg( pkTextute );
		pkAniWnd->UVInfo( pkTextute->GetUVInfo() );
		pkAniWnd->ImgIdx(-1);
		pkAniWnd->StartAni();

		STimerEvent kEvent;
		kEvent.Set(fTimeTotalInvenGetAniTime, "SetGetItemEffect()");
		g_kEventTimer.AddLocal("InvenGetEffect", kEvent);
	}

	bool FindNextReward(int& iKillNow, int &iKillMax, int &iRewardNo)
	{
		CONT_DEF_MONSTER_KILL_COUNT_REWARD const* pkRewardDef = NULL;
		g_kTblDataMgr.GetContDef(pkRewardDef);
		if( !pkRewardDef || pkRewardDef->empty() )
		{
			return false;
		}

		PgPlayer const *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		int iLimitMaxCount = 0;
		int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);
		//CONT_DEF_MONSTER_KILL_COUNT_REWARD::const_iterator iter = pkRewardDef->upper_bound(iKillNow);
		CONT_DEF_MONSTER_KILL_COUNT_REWARD::const_iterator iter = pkRewardDef->begin();
		while( pkRewardDef->end() != iter )
		{
			CONT_DEF_MONSTER_KILL_COUNT_REWARD::key_type const& rkKey = (*iter).first;
			CONT_DEF_MONSTER_KILL_COUNT_REWARD::mapped_type const& rkReward = (*iter).second;
			if( rkReward.CheckLevel(iPlayerLevel)
			&&	iKillMax < rkKey )
			{
				iKillMax = rkKey;
				iRewardNo = rkReward.iItemNo;
				return true;
			}

			iLimitMaxCount = std::max(iLimitMaxCount, rkKey.operator int());
			++iter;
		}

		if( iKillNow >= iLimitMaxCount )
		{
			iKillNow = iKillMax = iRewardNo = 0; // 리셋
			return FindNextReward(iKillNow, iKillMax, iRewardNo); // 재귀 호출
		}
		return false;
	}
}

PgMonsterKillUIMgr::PgMonsterKillUIMgr()
{
	Clear();
}

PgMonsterKillUIMgr::~PgMonsterKillUIMgr()
{
}

void PgMonsterKillUIMgr::Clear()
{
	m_iKillNow = m_iKillMax = m_iRewardKillNow = m_iRewardKillMax = m_iRewardNo = 0;
	m_fEffectEndTime = 0.f;
	m_bIsNowRewardEffect = false;
}

void PgMonsterKillUIMgr::Update(int const iNum)
{
	if( m_iKillNow == iNum )
	{
		return;
	}

	CONT_DEF_MONSTER_KILL_COUNT_REWARD const* pkRewardDef = NULL;
	g_kTblDataMgr.GetContDef(pkRewardDef);
	if( !pkRewardDef )
	{
		return;
	}

	PgPlayer const *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);
	CONT_DEF_MONSTER_KILL_COUNT_REWARD::const_iterator iter = pkRewardDef->find(iNum);
	while( pkRewardDef->end() != iter && (*iter).first == iNum )
	{
		CONT_DEF_MONSTER_KILL_COUNT_REWARD::key_type const& rkKey = (*iter).first;
		CONT_DEF_MONSTER_KILL_COUNT_REWARD::mapped_type const& rkReward = (*iter).second;
		if( rkReward.CheckLevel(iPlayerLevel)
		&&	rkKey == iNum ) // 해당 리워드
		{
			m_bIsNowRewardEffect = true;
			m_iRewardKillNow = m_iRewardKillMax = rkKey;
			m_fEffectEndTime = g_pkApp->GetAccumTime() + fTimeTotalInvenGetAniTime;

			PgMonsterKillUIUtil::StartRewardEffect(m_iRewardNo);
			break;
		}

		++iter;
	}

	//
	if( m_iKillNow > iNum )
	{
		m_iKillMax = 0; //초기화
	}
	m_iKillNow = iNum;

	//
	UpdateUI(false);
}

void PgMonsterKillUIMgr::UpdateUI(bool const bEndRewardEffect)
{
	if( !g_pkWorld )
	{
		return;
	}

	if( m_iKillNow >= m_iKillMax )
	{
		if( !PgMonsterKillUIUtil::FindNextReward(m_iKillNow, m_iKillMax, m_iRewardNo) )
		{
			return;
		}
	}

	CXUI_Wnd *pkTopWnd = XUIMgr.Get( PgMonsterKillUIUtil::kQuickMenuWndName );
	if( !pkTopWnd )
	{
		return;
	}

	CXUI_Wnd *pkKillCountWnd = pkTopWnd->GetControl( PgMonsterKillUIUtil::kKillCountWndName );
	if( !pkKillCountWnd )
	{
		return;
	}

	if( (g_pkApp->GetAccumTime() > m_fEffectEndTime) // 시간이 지났거나
	||	(bEndRewardEffect && m_bIsNowRewardEffect) ) // 끝났다고 통보가 오면
	{
		m_bIsNowRewardEffect = false; // 이펙트 그리기가 끝났다 원래대로 돌아간다.
	}
	
	// 특정 그라운드만 표시
	bool bCanKillCounter = (GATTR_DEFAULT == g_pkWorld->GetAttr()) || (0 == ((GATTR_FLAG_NO_KILLCOUNT_UI) & g_pkWorld->GetAttr()));
	if( true==bCanKillCounter )
	{
		GET_DEF(PgDefMapMgr, kDefMap);
		int const iValue = kDefMap.GetAbil(g_pkWorld->MapNo(), AT_NO_KILLCOUNT_MAP);
		bCanKillCounter = static_cast<bool>(!iValue);
	}
	pkKillCountWnd->Visible( bCanKillCounter );

	if( !bCanKillCounter )
	{
		return;
	}


	// Update
	bool const bIsRewardEffect = m_bIsNowRewardEffect;
	int const iCurKillCount = (bIsRewardEffect)? m_iRewardKillNow: m_iKillNow;
	int const iCurMaxCount = (bIsRewardEffect)? m_iRewardKillMax: m_iKillMax;

	lwUIWnd kTopWnd(pkKillCountWnd);
	PgMonsterKillUIUtil::MonsterKillUISetValue(kTopWnd, "FRM_COUNT", !bIsRewardEffect, iCurKillCount);		// 일반 카운터 감추고
	PgMonsterKillUIUtil::MonsterKillUISetValue(kTopWnd, "FRM_CRI_COUNT", bIsRewardEffect, iCurKillCount);	// 빨간 카운터 보이고
	PgMonsterKillUIUtil::MonsterKillUISetValue(kTopWnd, "FRM_TARGET_COUNT", true, iCurMaxCount);			// 목표 카운터 보이고

	CXUI_Wnd *pkRewardItemWnd = pkKillCountWnd->GetControl( std::wstring(_T("ICON_REWARD")) );
	if( pkRewardItemWnd )
	{
		int iPreRewardNo = 0;
		pkRewardItemWnd->GetCustomData(&iPreRewardNo, sizeof(int));

		if( iPreRewardNo == m_iRewardNo )
		{
			return;
		}

		PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(m_iRewardNo);
		if( !pkSprite )
		{
			pkRewardItemWnd->DefaultImgTexture(NULL);
			pkRewardItemWnd->SetInvalidate();
			return;
		}

		PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
		if( !pkUVSprite )
		{
			return;
		}

		pkRewardItemWnd->DefaultImgTexture(pkUVSprite);
		SUVInfo& rkUV = pkUVSprite->GetUVInfo();
		pkRewardItemWnd->UVInfo(rkUV);
		POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
		pkRewardItemWnd->ImgSize(kPoint);
		pkRewardItemWnd->SetInvalidate();
		pkRewardItemWnd->SetCustomData(&m_iRewardNo, sizeof(int));
	}
}

void lwDrawMonsterKillUI()
{
	g_kMonKillUIMgr.UpdateUI(true);
}


//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PgFormAnimatedMoveWnd::PgFormAnimatedMoveWnd()
{
	m_fStartAni = 0.f;
	m_bCanAni = m_bPlayMoveSound = false;
}

PgFormAnimatedMoveWnd::~PgFormAnimatedMoveWnd()
{
}

void PgFormAnimatedMoveWnd::StartAni()
{
	m_bCanAni = m_bPlayMoveSound = true;
	m_fStartAni = g_pkApp->GetAccumTime();
}

bool PgFormAnimatedMoveWnd::VDisplay()
{
	//NiPoint3 const kPoint1[3] = {
	//	NiPoint3(200.f, 110.f, 0.f),
	//	NiPoint3(200.f, 90.f, 0.f),
	//	NiPoint3(200.f, 0.f, 0.f)
	//};
	//NiPoint3 const kPoint2[3] = {
	//	NiPoint3(200.f, 0.f, 0.f),
	//	NiPoint3(70.f, 100.f, 0.f),
	//	NiPoint3(50.f, 201.f, 0.f)
	//};
	/*NiPoint3 const kPoint1[4] = {
		NiPoint3(200.f, 110.f, 0.f),
		NiPoint3(200.f, 90.f, 0.f),
		NiPoint3(100.f, 0.f, 0.f),
		NiPoint3(50.f, 201.f, 0.f)
	};*/
	/*NiPoint3 const kPoint1[4] = {
		NiPoint3(200.f, 110.f, 0.f),
		NiPoint3(200.f, 90.f, 0.f),
		NiPoint3(150.f, 0.f, 0.f),
		NiPoint3(180.f, 0.f, 0.f)
	};
	NiPoint3 const kPoint2[4] = {
		NiPoint3(180.f, 0.f, 0.f),
		NiPoint3(200.f, 90.f, 0.f),
		NiPoint3(100.f, 0.f, 0.f),
		NiPoint3(50.f, 201.f, 0.f)
	};*/
	NiPoint3 const kPoint1[4] = {
		NiPoint3(200.f, 110.f, 0.f),
		NiPoint3(200.f, 90.f, 0.f),
		NiPoint3(150.f, 1.f, 0.f),
		NiPoint3(130.f, 1.f, 0.f)
	};
	NiPoint3 const kPoint2[4] = {
		NiPoint3(130.f, 1.f, 0.f),
		NiPoint3(200.f, 90.f, 0.f),
		NiPoint3(100.f, 1.f, 0.f),
		NiPoint3(50.f, 201.f, 0.f)
	};

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
		//if( fTimeTotalInvenGetAniTime1 > fAniFrameTime )
		//{
		//	Bezier3(kPoint1[0], kPoint1[1], kPoint1[2], Bias(fAniFrameTime/fTimeTotalInvenGetAniTime1, 0.68f), kNow);
		//}
		//else
		//{
		//	Bezier3(kPoint2[0], kPoint2[1], kPoint2[2], Bias((fAniFrameTime-fTimeTotalInvenGetAniTime1)/fTimeTotalInvenGetAniTime2, 0.66f), kNow);
		//}

		float fMaxScale = 3.f;
		//float fScale = 1.f;
		if( fTimeTotalInvenGetAniTime1 > fAniFrameTime )
		{
			Bezier4(kPoint1[0], kPoint1[1], kPoint1[2], kPoint1[3], PgDropBoxUtil::Bias(fAniFrameTime/fTimeTotalInvenGetAniTime1, 0.48f), kNow);
			m_kScale = (fAniFrameTime/fTimeTotalInvenGetAniTime1 * fMaxScale);
			ImgPos( POINT2(kNow.x, kNow.y) );
		}
		else if( fTimeTotalInvenGetAniTime2 > fAniFrameTime )
		{
			//Bezier4(kPoint1[0], kPoint1[1], kPoint1[2], kPoint1[3], 1.f, kNow);
			//m_kScale = fMaxScale;
			ImgPos( POINT2(kPoint2[0].x, kPoint2[0].y) );
			m_kScale = fMaxScale;
		}
		else
		{
			if( m_bPlayMoveSound )
			{
				m_bPlayMoveSound = false;
				PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
				if( pkMyActor )
				{
					pkMyActor->PlayNewSound(NiAudioSource::TYPE_3D, "WoodenBox_Move", 0.f);
				}
			}

			float fEleapsedTime = __max(fAniFrameTime - fTimeTotalInvenGetAniTime1 - fTimeTotalInvenGetAniTime2, 0.f);
			Bezier4(kPoint2[0], kPoint2[1], kPoint2[2], kPoint2[3], PgDropBoxUtil::Bias(fEleapsedTime/fTimeTotalInvenGetAniTime3, 0.86f), kNow);
			m_kScale = __max(fMaxScale - (fEleapsedTime / fTimeTotalInvenGetAniTime2 * fMaxScale), 0.7f);
			ImgPos( POINT2(kNow.x, kNow.y) );
		}

		//Bezier4(kPoint1[0], kPoint1[1], kPoint1[2], kPoint1[3], Bias(fAniFrameTime/fTimeTotalInvenGetAniTime, 0.33f) + (fAniFrameTime*0.2), kNow);
		

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
			//kRenderInfo.kLoc = OffscreenLocation() + POINT3I(kNow.x, kNow.y, 0);
			kRenderInfo.fAlpha = 1.f;
		}
		else
		{
			kRenderInfo.kLoc = TotalLocation() + ImgPos();
			//kRenderInfo.kLoc = TotalLocation() + POINT3I(kNow.x, kNow.y, 0);
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
	}
	return true;
}




PgFormRouletteWnd::PgFormRouletteWnd()
{
}

PgFormRouletteWnd::~PgFormRouletteWnd()
{
}

void PgFormRouletteWnd::Clear()
{
	m_fTime = 0.f;
	m_fChangeTime = 0.f;
	m_kTempRoulette.clear();
	m_pkCurItem = NULL;
	m_bResultSound = false;
	m_iCurItem = 0;
	m_kResultItem.Clear();
	CXUI_Wnd::ClearCustomData();
	m_bStopKey = false;	
	m_bComplete = false;
}

void PgFormRouletteWnd::SetStopKey()
{
	if( m_bStopKey )
	{
		m_fTime = 9.0f;
	}
	else
	{
		g_kMissionComplete.StopResult();		
	}
}

void PgFormRouletteWnd::SetStopKeyState()
{
	m_bStopKey = true;
}

bool PgFormRouletteWnd::GetCompleteState()
{
	return m_bComplete;
}

bool PgFormRouletteWnd::VDisplay()
{
	if( m_kTempRoulette.empty() )
	{
		return true;
	}

	float const fAniFrameTime = g_pkApp->GetAccumTime();

	bool bDrawOffscreen = false;
	if( UseOffscreen() && Offscreen() && Invalidate() )
	{
		bDrawOffscreen = true;
	}

	float const fStopTime = 0.043;
	float const fCeta = 0.29;
	float const fTargetStartTime = 3.4f;
	float const fTargetSlowTime = 7.3f;
	float const fDelta = 1.5f;

	{
		//>>
		m_fTime += g_pkApp->GetFrameTime();
		m_fChangeTime += g_pkApp->GetFrameTime();

		if( fTargetStartTime + fDelta > m_fTime )
		{
			if( fStopTime < m_fChangeTime
			||	!m_pkCurItem )
			{
				//size_t const iCurItem = BM::Rand_Index(m_kTempRoulette.size());

				ContRouletteItem::value_type &rkCurElement = m_kTempRoulette.at(m_iCurItem);
				m_pkCurItem = &rkCurElement;
				++m_iCurItem;
				if( m_kTempRoulette.size() <= m_iCurItem )
				{
					m_iCurItem = 0;
				}

				//BM::vstring kCountStr = rkCurElement.iCount;
				//Text(kCountStr);
				m_fChangeTime = 0.f;

				PgActor *pkActor = g_kPilotMan.GetPlayerActor();
				if( pkActor )
				{
					pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Roulette_loop", 0.f);
				}
			}
		}
		else if( fTargetStartTime + fTargetSlowTime - fDelta > m_fTime )
		{
			float const fCurTime = m_fTime - fTargetStartTime;
			float fCur = 1 - NiCos(NiLog( (fTargetSlowTime+fCeta)/(fCurTime+fCeta)));
			fCur = __max(fCur, 0.f);
			fCur = __min(fCur, 1.f);

			if( fStopTime < m_fChangeTime )
			{
				size_t const iCurItem = fCur * (m_kTempRoulette.size() - 1);
				ContRouletteItem::value_type &rkCurElement = m_kTempRoulette.at(iCurItem);

				//
				if( m_pkCurItem != &rkCurElement )
				{
					m_pkCurItem = &rkCurElement;

					//BM::vstring kCountStr = rkCurElement.iCount;
					//Text(kCountStr);
					m_fChangeTime = 0.f;

					PgActor *pkActor = g_kPilotMan.GetPlayerActor();
					if( pkActor )
					{
						pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Roulette_loop", 0.f);
					}
				}
			}
		}
		else
		{
			ContRouletteItem::value_type &rkCurElement = (*m_kTempRoulette.begin());

			if( !m_bResultSound )
			{
				PgActor *pkActor = g_kPilotMan.GetPlayerActor();
				if( pkActor )
				{
					pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Roulette_Result", 0.f);
					if( NULL != g_pkWorld
						&& false == g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND) )
					{ // 원정대 던전이 아닐 때만 배경음을 여기서 동작시킴.
						g_kSoundMan.StopBgSound( -1 );
						g_kSoundMan.PlayBgSound( 1 );
					}
				}
				m_bResultSound = true;
				m_bComplete = true;

				BM::Stream kCustomData;
				m_kResultItem.WriteToPacket(kCustomData);
				CXUI_Wnd::SetCustomData(kCustomData.Data()); // 툴팁용
			}

			m_pkCurItem = &rkCurElement;

			//BM::vstring kCountStr = rkCurElement.iCount;
			//Text(kCountStr);
		}

		if( !m_pkCurItem )
		{
			return true;
		}
		SUVInfo const &rkUVInfo = m_pkCurItem->pkUVSprite->GetUVInfo();
		//<<

		void* pOffscreen = Offscreen();
		SRenderInfo kRenderInfo;
		SSizedScale &rSS = kRenderInfo.kSizedScale;

		kRenderInfo.bGrayScale = GrayScale();

		rSS.ptSrcSize = ImgSize();
		rSS.ptDrawSize = ImgSize()/POINT2((LONG)rkUVInfo.U, (LONG)rkUVInfo.V);
		kRenderInfo.kUVInfo = rkUVInfo;
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

		if( StyleText().Length() )
		{
			POINT3I const pt = TotalLocation() + TextPos();
			RenderText( pt, StyleText().GetOriginalString(), Font(), FontFlag() );
		}
		
		void *pkSprite = m_pkCurItem->pkUVSprite;
#ifdef XUI_USE_GENERAL_OFFSCREEN
		if (bDrawOffscreen)
		{
			m_spRenderer->RenderSprite( pkSprite, m_pkCurItem->iImgIndex, kRenderInfo, false);
			m_kOffscreenDrawList.push_back(std::make_pair(m_pkCurItem->pkUVSprite, Parent() == NULL));
		}
		else
#endif
		{
			m_spRenderer->RenderSprite( pkSprite, m_pkCurItem->iImgIndex, kRenderInfo);
		}
	}

	return true;
}

CXUI_Wnd* PgFormRouletteWnd::VClone()
{
	PgFormRouletteWnd *pWnd = new PgFormRouletteWnd;
	*pWnd = *this;
	return pWnd;
}

//bool PgFormRouletteWnd::SetRouletteBagItem(int const iItemNo, PgBase_Item const &rkResultItem, size_t iTempCount)
//{
//	if( !iItemNo )
//	{
//		return false;
//	}
//
//	GET_DEF(CItemDefMgr, kItemDefMgr);
//	CItemDef const* pkDef = kItemDefMgr.GetDef( iItemNo );
//	if( !pkDef )
//	{
//		return false;
//	}
//
//	int const iCustomeType = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
//	if( UICT_OPEN_CHEST != iCustomeType )
//	{
//		return false;
//	}
//	int const iCustomeValue1 = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
//
//	return SetRoulette(iCustomeValue1, rkResultItem, iTempCount);
//}

bool PgFormRouletteWnd::SetRoulette(int const iItemBagGroupNo, PgBase_Item const &rkResultItem, size_t iTempCount)
{
	if( !iItemBagGroupNo
	||	rkResultItem.IsEmpty() )
	{
		return false;
	}

	PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject *>( g_kUIScene.GetIconTexture(rkResultItem.ItemNo()) );
	if( !pkUVSprite )
	{
		return false;
	}

	Clear();
	int iLevel = 1;

	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkPlayer )
	{
		iLevel = pkPlayer->GetAbil( AT_LEVEL );
	}

	ContRouletteItem kVec;

	SRouletteItem kResultRouletteItem;
	kResultRouletteItem.iItemNo = rkResultItem.ItemNo();
	kResultRouletteItem.iCount = rkResultItem.Count();
	kResultRouletteItem.pkUVSprite = pkUVSprite;
	std::back_inserter(kVec) = kResultRouletteItem;
	m_kResultItem = rkResultItem;

	GET_DEF(CItemDefMgr, kItemDefMgr);
	GET_DEF(CItemBagMgr, kItemBagMgr);

	CONT_DEF_ITEM_BAG_GROUP const* pkDefBagGroup = NULL;
	CONT_DEF_ITEM_BAG const* pkDefItemBag = NULL;
	CONT_DEF_ITEM_BAG_ELEMENTS const* pkDefItemBagEle = NULL;
	g_kTblDataMgr.GetContDef(pkDefBagGroup);
	g_kTblDataMgr.GetContDef(pkDefItemBag);
	g_kTblDataMgr.GetContDef(pkDefItemBagEle);

	CONT_DEF_ITEM_BAG_GROUP::const_iterator groupbag_iter = pkDefBagGroup->find(iItemBagGroupNo);
	if( pkDefBagGroup->end() != groupbag_iter )
	{
		CONT_DEF_ITEM_BAG_GROUP::mapped_type const& rkBagGroup = (*groupbag_iter).second;
		size_t iCur = 0;
		PgItemBag kItemBag;
		for( ; MAX_ITEM_BAG_ELEMENT > iCur; ++iCur )
		{
			if ( S_OK == kItemBagMgr.GetItemBag( rkBagGroup.aiBagNo[iCur], static_cast<short>(iLevel), kItemBag ) )
			{
				for( int iBagIndex=0; iBagIndex < kItemBag.GetElementsCount(); ++iBagIndex)
				{
					int iItemNo = 0;
					size_t iItemCount = 1;
					if(S_OK == kItemBag.PopItemToIndex( iLevel, iItemNo, iItemCount, iBagIndex ))
					{
						if( kVec.end() == std::find(kVec.begin(), kVec.end(), SRouletteItemDiffer(iItemNo, kItemDefMgr)) )
						{
							PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject *>( g_kUIScene.GetIconTexture(iItemNo) );
							if( pkUVSprite )
							{
								ContRouletteItem::value_type kTempItem(iItemNo, 1, pkUVSprite);
								std::back_inserter(kVec) = kTempItem;
							}
						}
					}
				}
			}
		}
	}
	//else
	//{
	//	// 기존 방식
	//	PgItemBag kItemBag;
	//	GET_DEF(CItemBagMgr, kItemBagMgr);
	//	while( --iTempCount )
	//	{
	//		HRESULT const hBagRet = kItemBagMgr.GetItemBagByGrp(iItemBagGroupNo, kItemBag);
	//		if( S_OK != hBagRet )
	//		{
	//			return false;
	//		}

	//		ContRouletteItem::value_type kTempItem;
	//		if( S_OK == kItemBag.PopItem(kTempItem.iItemNo, kTempItem.iCount) )
	//		{
	//			ContRouletteItem::const_iterator find_iter = std::find(kVec.begin(), kVec.end(), kTempItem.iItemNo);
	//			if( find_iter != kVec.end() )
	//			{
	//				continue;
	//			}

	//			PgBase_Item kItem;
	//			if( S_OK == CreateSItem(kTempItem.iItemNo, kTempItem.iCount, GIOT_NONE, kItem) )
	//			{
	//				PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject *>( g_kUIScene.GetIconTexture(kTempItem.iItemNo) );
	//				if( pkUVSprite )
	//				{
	//					kTempItem.pkUVSprite = pkUVSprite;
	//					std::back_inserter(kVec) = kTempItem;
	//				}
	//			}
	//		}
	//	}
	//}

	m_kTempRoulette.swap(kVec);
	return true;
}