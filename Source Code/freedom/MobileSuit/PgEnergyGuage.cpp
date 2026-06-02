#include "stdafx.h"
#include "PgEnergyGuage.H"
#include "PgNifMan.H"
#include "PgMobileSuit.H"
#include "PgRenderer.H"
#include "PgPilot.h"
#include "PgEnergyGaugeBig.h"
#include "PgUIScene.H"
#include "Variant/PgMonster.H"
#include "PgOption.h"
#include "PgPilotMan.h"

PgEnergyGaugeMan	g_kEnergyGaugeMan;

bool PgEnergyGauge::ms_bDrawEnergyGaugeBar = false;
float const PgEnergyGauge::ms_fUseTime = 5.0f;
float const	PgEnergyGauge::ms_fNonVisibleTime = -1.0f;
bool PgEnergyGauge::ms_bDrawEnergyGaugeBarHide = true; // 게이지 보기/안보기
bool PgEnergyGauge::ms_bDrawPetMPGaugeBar = false;

PgEnergyGauge::PgEnergyGauge()
:	m_fVisibleStartTime(ms_fNonVisibleTime)
,	m_fTotalVisibleTime(ms_fUseTime)
,	m_iMaxValue(0)
//,	m_iBeforeValue(0)
//,	m_iNewValue(0)
,	m_bVisible(true)
,	m_bDraw(true)
,	m_fBeforeHPRate(1.0f)
,	m_fNewHPRate(1.0f)
,	m_fCurrentRate(ms_fNonVisibleTime)
,	m_fRateChangeSpeed(0.1f)
,	m_fStandardWidth(0.0f)
,	m_spGaugeOut(NULL)
,	m_spGaugeIn(NULL)
,	m_spPartGauge(NULL)
,	m_iType(EGAUGE_DEFAULT)
,	m_iOption(EGOT_DEFAULT)
,	m_bAutoScale(false)
,	m_fScale(1.0f)
,	m_iCurIndex(0)
,	m_bIsMyPet(false)
{

}

PgEnergyGauge::PgEnergyGauge( PgPilot const *pkPilot )
:	m_fVisibleStartTime(ms_fNonVisibleTime)
,	m_fTotalVisibleTime(ms_fUseTime)
,	m_iMaxValue(0)
//,	m_iBeforeValue(0)
//,	m_iNewValue(0)
,	m_bVisible(true)
,	m_bDraw(true)
,	m_fBeforeHPRate(1.0f)
,	m_fNewHPRate(1.0f)
,	m_fCurrentRate(0.0f)
,	m_fRateChangeSpeed(0.1f)
,	m_fStandardWidth(0.0f)
,	m_spGaugeOut(NULL)
,	m_spGaugeIn(NULL)
,	m_spPartGauge(NULL)
,	m_iType(EGAUGE_DEFAULT)
,	m_iOption(EGOT_DEFAULT)
,	m_bAutoScale(false)
,	m_fScale(1.0f)
,	m_iCurIndex(0)
,	m_bIsMyPet(false)
{
	Init( pkPilot );
}

PgEnergyGauge::~PgEnergyGauge()
{
	m_spGaugeOut = NULL;
	m_spGaugeIn = NULL;
	m_spPartGauge = NULL;

	m_kExternTexture.clear();
}

void PgEnergyGauge::Init(PgPilot const* pkPilot)
{
	if( pkPilot )
	{
		if(pkPilot->GetUnit() && pkPilot->GetUnit()->IsUnitType(UT_PET))
		{
			GET_DEF(PgClassPetDefMgr, kClassDefMgr);
			PgClassPetDef kPetDef;
			bool const bDef = kClassDefMgr.GetDef( SClassKey(pkPilot->GetUnit()->GetAbil(AT_CLASS), pkPilot->GetUnit()->GetAbil(AT_LEVEL)), &kPetDef );

			Init(kPetDef.GetAbil(AT_HP_GAUGE_TYPE));
			// 펫은 HP가 아닌 MP 값을 표시해 주어야 한다.

			int const iMP = pkPilot->GetAbil(AT_MP);
			SetBarValue( pkPilot->GetAbil(AT_C_MAX_MP), iMP, iMP );
			BM::GUID kGuid;
			if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
			{
				if(pkPilot->GetUnit()->Caller()==kGuid)
				{
					m_bIsMyPet = true;	
					return;
				}
			}
		}
		else
		{
			Init( pkPilot->GetAbil(AT_HP_GAUGE_TYPE), pkPilot->GetAbil(AT_OPTION_HP_GAGE) );
			int const iHP = pkPilot->GetAbil(AT_HP);
			SetBarValue( pkPilot->GetAbil(AT_C_MAX_HP), iHP, iHP );
		}
		
		m_fVisibleStartTime = ms_fNonVisibleTime;
	}
}

void PgEnergyGauge::Init(int const iGuageType, int const iOption)
{
	m_iType = iGuageType;
	m_iOption = iOption;
	switch ( iGuageType )
	{
	case EGAUGE_DEFAULT:
		{
			NiSourceTexturePtr	spTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnMstEbar.dds");
			PG_ASSERT_LOG(spTex);
			if(spTex)
			{
				m_spGaugeOut = NiNew NiScreenTexture(spTex);
				m_spGaugeIn = NiNew NiScreenTexture(spTex);

				m_spPartGauge = NiNew NiScreenTexture(spTex);
				m_spPartGauge->AddNewScreenRect(0,0,90,12,0,0);

				m_spGaugeOut->AddNewScreenRect(0,0, 96, 18, 12, 0 );
				m_spGaugeIn->AddNewScreenRect(3,3,90,12,0,0);

				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_fStandardWidth = 90.0f;
			}

			m_kExternTexture.clear();
		}break;
	case EGAUGE_CORE:
		{
			NiSourceTexturePtr spTexFrame = g_kNifMan.GetTexture("../Data/6_UI/war/wrCrEngBg.tga");
			PG_ASSERT_LOG(spTexFrame);
			if ( spTexFrame )
			{
				m_spGaugeOut = NiNew NiScreenTexture(spTexFrame);
				m_spGaugeOut->AddNewScreenRect( 0, 0, spTexFrame->GetWidth(), spTexFrame->GetHeight(), 0, 0 );
				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			}

			NiSourceTexturePtr spTexBar = g_kNifMan.GetTexture("../Data/6_UI/war/wrCrEngBar.tga");
			PG_ASSERT_LOG( spTexBar );
			if ( spTexBar )
			{
				m_spGaugeIn = NiNew NiScreenTexture(spTexBar);
				m_spPartGauge =NiNew NiScreenTexture(spTexBar);

				m_spGaugeIn->AddNewScreenRect( 0, 0, spTexBar->GetWidth(), spTexBar->GetHeight() , 0, 0 );
				m_spPartGauge->AddNewScreenRect( 0, 0, spTexBar->GetWidth(), spTexBar->GetHeight() , 0, 0 );

				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_fStandardWidth = (float)(spTexBar->GetWidth());
			}

			m_kExternTexture.clear();

			m_bAutoScale = true;
		}break;
	case EGAUGE_CHARGING1:
		{
			NiSourceTexturePtr spText = g_kNifMan.GetTexture("../Data/6_UI/main/mnBarRBbg.tga");
			PG_ASSERT_LOG( spText );
			if( spText )
			{
				//
				m_spGaugeOut = NiNew NiScreenTexture(spText);
				m_spGaugeOut->AddNewScreenRect( 0, 0, spText->GetWidth(), spText->GetHeight()/2, 0, 0);
				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			
				//
				m_spGaugeIn = NiNew NiScreenTexture(spText);
				m_spGaugeIn->AddNewScreenRect( 0, 0, spText->GetWidth(), spText->GetHeight()/2, spText->GetHeight()/2, 0);
				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				
				m_spPartGauge = NiNew NiScreenTexture(spText);
				m_spPartGauge->AddNewScreenRect( 0, 0, spText->GetWidth(), spText->GetHeight()/2, spText->GetHeight()/2, 0);
				m_spPartGauge->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);


				m_fStandardWidth = static_cast< float >(spText->GetWidth());
			}

			NiSourceTexturePtr spExtra1 = g_kNifMan.GetTexture("../Data/6_UI/main/mnBarRBcc.tga");
			PG_ASSERT_LOG( spExtra1 );
			if( spExtra1 )
			{
				NiScreenTexturePtr spExtraTexture = NULL;

				spExtraTexture = NiNew NiScreenTexture(spExtra1);
				spExtraTexture->AddNewScreenRect( 0, 0, spExtra1->GetWidth(), spExtra1->GetHeight()/2, spExtra1->GetHeight()/2, 0);
				spExtraTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				m_kExternTexture.push_back( spExtraTexture );

				spExtraTexture = NiNew NiScreenTexture(spExtra1);
				spExtraTexture->AddNewScreenRect( 0, 0, spExtra1->GetWidth(), spExtra1->GetHeight()/2, 0, 0);
				spExtraTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				m_kExternTexture.push_back( spExtraTexture );
			}
		}break;
	case EGAUGE_CHARGING2:
		{
			NiSourceTexturePtr spText = g_kNifMan.GetTexture("../Data/6_UI/main/mnRBbarbg.tga");
			PG_ASSERT_LOG( spText );
			if( spText )
			{
				m_spGaugeOut = NiNew NiScreenTexture(spText);
				m_spGaugeOut->AddNewScreenRect( 0, 0, spText->GetWidth(), spText->GetHeight(), 0, 0);
				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			}

			NiSourceTexturePtr spBarText = g_kNifMan.GetTexture("../Data/6_UI/main/mnRBbar.tga");
			PG_ASSERT_LOG( spBarText );
			if( spBarText )
			{
				m_spGaugeIn = NiNew NiScreenTexture(spBarText);
				m_spGaugeIn->AddNewScreenRect( 0, 0, spBarText->GetWidth(), spBarText->GetHeight()/2, spBarText->GetHeight()/2, 0);
				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_spPartGauge = NiNew NiScreenTexture(spBarText);
				m_spPartGauge->AddNewScreenRect( 0, 0, spBarText->GetWidth(), spBarText->GetHeight()/2, spBarText->GetHeight()/2, 0);
				m_spPartGauge->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_fStandardWidth = static_cast< float >(spBarText->GetWidth());
			}

			m_kExternTexture.clear();
		}break;
	case EGAUGE_PET:
	case EGAUGE_ALIVETIME:
		{
			NiSourceTexturePtr	spTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnMstEbar2.dds");
			PG_ASSERT_LOG(spTex);
			if(spTex)
			{
				m_spGaugeOut = NiNew NiScreenTexture(spTex);
				m_spGaugeIn = NiNew NiScreenTexture(spTex);

				m_spPartGauge = NiNew NiScreenTexture(spTex);
				m_spPartGauge->AddNewScreenRect(0,0,90,6,0,0);

				m_spGaugeOut->AddNewScreenRect(0,0, 96, 12, 6, 0 );
				m_spGaugeIn->AddNewScreenRect(3,3,90,6,0,0);

				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_fStandardWidth = 90.0f;
			}

			m_kExternTexture.clear();
		}break;
	case EGAUGE_SUMMONED:
		{
			NiSourceTexturePtr	spTex = g_kNifMan.GetTexture("../Data/6_UI/skill/skEbar.dds");
			PG_ASSERT_LOG(spTex);
			if(spTex)
			{
				m_spGaugeOut = NiNew NiScreenTexture(spTex);
				m_spGaugeIn = NiNew NiScreenTexture(spTex);

				m_spPartGauge = NiNew NiScreenTexture(spTex);
				m_spPartGauge->AddNewScreenRect(0,0,90,8,0,0);// dds 사이즈 보고 rect 정하기

				m_spGaugeOut->AddNewScreenRect(0,0, 96, 14, 8, 0 );
				m_spGaugeIn->AddNewScreenRect(3,3,90,8,0,0);

				m_spGaugeOut->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
				m_spGaugeIn->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

				m_fStandardWidth = 90.0f;
			}

			m_kExternTexture.clear();
		}break;
	}
	Blink(0.0f);
	m_iCurIndex = 0;
}

void	PgEnergyGauge::AddNewHPPart(const NiPoint2	&kStartPos,float const fHPStartPos,float	fHPLength)
{
	stHPPart	kNewPart;

	kNewPart.kStartPos = kNewPart.kCurrentPos = kStartPos;
	kNewPart.fStartTime = g_pkApp->GetAccumTime();
	kNewPart.fHPLength = fHPLength;
	kNewPart.fAlpha = 1.0f;
	kNewPart.fHPStartPos = fHPStartPos;
	
	m_HPPartCont.push_back(kNewPart);

}
void	PgEnergyGauge::Blink(float fTotalTime)
{
	m_bBlink = fTotalTime>0;

	m_bBlinkToggle = false;
	m_fLastBlinkTime = 0;
	m_fBlinkDuration = 0.05f;
	m_fBlinkStartTime = g_pkApp->GetAccumTime();
	m_fBlinkTotalTime = fTotalTime;
}

void	PgEnergyGauge::UpdateHPParts(float fAccumTime,float fFrameTime)
{
	float const fMaxMoveTime = 0.3f;
	float fElapsedTime = 0.0f;

	stHPPart* pkPartsInfo = NULL;

	PartCont::iterator itor = m_HPPartCont.begin();
	while( itor != m_HPPartCont.end() )
	{
		pkPartsInfo = &(*itor);

		fElapsedTime = fAccumTime - pkPartsInfo->fStartTime;

		if( fElapsedTime>fMaxMoveTime )
		{
			itor = m_HPPartCont.erase(itor);
		}
		else
		{
			float	fHeight = pkPartsInfo->kStartPos.y-100.0f*fElapsedTime+1200.0f*fElapsedTime*fElapsedTime;
			pkPartsInfo->kCurrentPos.y = fHeight;
			pkPartsInfo->kCurrentPos.x = pkPartsInfo->kStartPos.x+fElapsedTime*100.0f;
			pkPartsInfo->fAlpha = 1.0f - (fElapsedTime/fMaxMoveTime);
			if(pkPartsInfo->fAlpha<0)
			{
				pkPartsInfo->fAlpha = 0.0f;
			}
			pkPartsInfo->fScale = 2.0f - (fElapsedTime/fMaxMoveTime)*2;
			if(pkPartsInfo->fScale<0)
			{
				pkPartsInfo->fScale = 0.0f;
			}

			pkPartsInfo->fRotate = fElapsedTime*360.0f;	//	1초에 10바퀴
			++itor;
		}
	}
	
}
void	PgEnergyGauge::ClearHPParts()
{
	m_HPPartCont.clear();
}
void	PgEnergyGauge::DrawHPParts(PgRenderer *pkRenderer)
{
	if(!m_spPartGauge || m_spPartGauge->GetNumScreenRects() == 0)
	{
		return;
	}

	stHPPart* pkPartsInfo = NULL;

	for(PartCont::iterator itor = m_HPPartCont.begin();
		itor != m_HPPartCont.end(); ++itor)
	{
		pkPartsInfo = &(*itor);

		NiScreenTexture::ScreenRect &kRect = m_spPartGauge->GetScreenRect(0);
		kRect.m_sPixLeft = (short)pkPartsInfo->kCurrentPos.x;
		kRect.m_sPixTop = (short)pkPartsInfo->kCurrentPos.y;
		kRect.m_usPixWidth = (unsigned short)( m_fStandardWidth * pkPartsInfo->fHPLength );
		kRect.m_kColor.a = pkPartsInfo->fAlpha;
		kRect.m_usTexLeft = (unsigned short)( m_fStandardWidth * pkPartsInfo->fHPStartPos );

		m_spPartGauge->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
		PgUIScene::Render_UIObject(pkRenderer,
			m_spPartGauge,
			PgRenderer::E_PS_NONE,
			stRenderOption( NiColorA(0,0,0,0), false, pkPartsInfo->fScale*m_fScale, pkPartsInfo->fRotate ));

	}
}
void	PgEnergyGauge::SetBarValue(int iMax,int iBefore,int iNew)
{
	m_iMaxValue = iMax;
//	m_iBeforeValue = iBefore;
//	m_iNewValue = iNew;
	ValueChanged( static_cast<float>(iBefore), static_cast<float>(iNew) );
}

void	PgEnergyGauge::ValueChanged( float const fBefore, float const fNew )
{
	m_fBeforeHPRate = ((m_iMaxValue > 0) ? (fBefore / static_cast<float>(m_iMaxValue)) : 0.0f);
	m_fNewHPRate = ((m_iMaxValue > 0) ? (fNew / static_cast<float>(m_iMaxValue)) : 0.0f);

	switch( m_iType ) // 값 변화시
	{
	case EGAUGE_CHARGING1:
		{
			m_fBeforeHPRate = 1.f - m_fBeforeHPRate;
			m_fNewHPRate = 1.f - m_fNewHPRate;
		}break;
	case EGAUGE_CHARGING2:
	case EGAUGE_DEFAULT:
	case EGAUGE_CORE:
	case EGAUGE_BOSS:
	case EGAUGE_PET:
	default:
		{
		}break;
	}

	if(m_fBeforeHPRate<0.0f) { m_fBeforeHPRate = 0.0f; }
	if(m_fBeforeHPRate>1.0f) { m_fBeforeHPRate = 1.0f; }

	if(m_fNewHPRate<0.0f) { m_fNewHPRate = 0.0f; }
	if(m_fNewHPRate>1.0f) { m_fNewHPRate = 1.0f; }

	if(m_fCurrentRate == -1.0f)	//	최초로 값이 셋팅될 때
	{
		m_fCurrentRate = m_fBeforeHPRate;
	}

	if( m_fNewHPRate < m_fBeforeHPRate )	//	HP 가 줄어들때
	{
		switch( m_iType )
		{
		case EGAUGE_CHARGING1:
		case EGAUGE_CHARGING2:
			{
				Blink(0.3f);
			}break;
		case EGAUGE_DEFAULT:
		case EGAUGE_CORE:
		case EGAUGE_BOSS:
		case EGAUGE_PET:
		default:
			{
				if( m_spGaugeIn && m_spGaugeIn->GetNumScreenRects()>0 )
				{
					NiPoint2	kStartPos;
					NiScreenTexture::ScreenRect &kRect2 = m_spGaugeIn->GetScreenRect(0);
					kStartPos.x = (float)(kRect2.m_sPixLeft+(short)( m_fStandardWidth * m_fNewHPRate ));
					kStartPos.y = (float)(kRect2.m_sPixTop);
					AddNewHPPart( kStartPos, m_fNewHPRate, m_fBeforeHPRate - m_fNewHPRate );
				}
				Blink(EGAUGE_PET==m_iType ? 0.0f : 0.3f); // HP는 변화가 생기면 깜박거리고 게이지가 변하도록
			}break;
		}
	}

	m_fCurrentRate = m_fNewHPRate;
	SetBarLength(m_fCurrentRate);
}

void PgEnergyGauge::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
}
bool	PgEnergyGauge::GetVisible()const
{
	return	m_bVisible;
}
void	PgEnergyGauge::ResetVisibleStartTime()
{
	m_fVisibleStartTime = g_pkApp->GetAccumTime();
}

void	PgEnergyGauge::SetBarLength(const	float fLengthRate)
{
	if(!m_spGaugeIn || 0 >= m_spGaugeIn->GetNumScreenRects()) 
	{
		return;
	}
	NiScreenTexture::ScreenRect &kRect2 = m_spGaugeIn->GetScreenRect(0);
	kRect2.m_usPixWidth = (unsigned short)( m_fStandardWidth * fLengthRate );

	m_spGaugeIn->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

	DoClipping();
}
void PgEnergyGauge::Update(float fAccumTime,float fFrameTime)
{
	UpdateHPParts(fAccumTime,fFrameTime);
	UpdateBlink(fAccumTime);

	if( m_iMaxValue > 0 )
	{
		if(m_fCurrentRate < m_fNewHPRate)
		{
			float	fAmount = (m_fNewHPRate-m_fBeforeHPRate)*m_fRateChangeSpeed;
			m_fCurrentRate+=fAmount;

			if(m_fCurrentRate>=m_fNewHPRate)
			{
				m_fCurrentRate = m_fNewHPRate;
			}
			SetBarLength(m_fCurrentRate);
		}

		switch( m_iType )
		{
		case EGAUGE_CHARGING1:
			{
				++m_iCurIndex;
				if( m_kExternTexture.size()-1 < m_iCurIndex )
				{
					m_iCurIndex = 0;
				}
			}break;
		default:
			{
			}break;
		}

		if(m_fVisibleStartTime == ms_fNonVisibleTime) return;

		if(m_fCurrentRate == 0)
		{
			m_fVisibleStartTime = ms_fNonVisibleTime;
		}


		float	fElapsedTime = fAccumTime - m_fVisibleStartTime;
		if(fElapsedTime>=m_fTotalVisibleTime)
		{
			m_fVisibleStartTime = ms_fNonVisibleTime;
		}
	}
}

void	PgEnergyGauge::UpdateBlink(float fAccumTime)
{
	if(!m_bBlink)
	{
		return;
	}

	float	fTotalElapsedTime = fAccumTime - m_fBlinkStartTime;

	if(fTotalElapsedTime>m_fBlinkTotalTime)
	{
		Blink(0);
		return;
	}

	float	fElapsedTime = fAccumTime - m_fLastBlinkTime;
	if(fElapsedTime>m_fBlinkDuration)
	{
		m_bBlinkToggle = !m_bBlinkToggle;
		m_fLastBlinkTime = fAccumTime;
	}
}

bool PgEnergyGauge::IsDefaultView()const
{
	if( ms_bDrawEnergyGaugeBarHide &&
		GetVisible() &&
		m_bDraw &&
		(m_fCurrentRate > 0.0f) )
	{
		return true;
	}
	return false;
}

bool PgEnergyGauge::IsAlwaysView()const
{
	if( (m_iOption&EGOT_ALWAYSVIEW) &&
		IsDefaultView() )
	{
		return true;
	}
	return false;
}

void PgEnergyGauge::DrawImmediate(PgRenderer *pkRenderer)
{
	bool bDrawGauge = true;

	if(m_fVisibleStartTime == ms_fNonVisibleTime) { bDrawGauge = false; }

	if( !bDrawGauge )
	{
		if ( !PgWorldUtil::IsCharacterSelectStage() )
		{

			CXUI_Edit *pkEdit = (CXUI_Edit*)XUIMgr.GetFocusedEdit();
			if( pkEdit == NULL )
			{
				if( g_pkLocalManager )
				{
					NiInputKeyboard* pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
					if( pkKeyboard )
					{
						bDrawGauge = pkKeyboard->KeyIsDown(static_cast<NiInputKeyboard::KeyCode>(g_kGlobalOption.GetUKeyToKey(NiInputKeyboard::KEY_GRAVE+PgInput::UR_LOCAL_BEGIN)));
					}
				}

				if(EGAUGE_PET==m_iType)	//펫 게이지 일 경우에는
				{
					if(true==ms_bDrawPetMPGaugeBar)
					{
						bDrawGauge = !bDrawGauge;
					}
				}
				else if ( ms_bDrawEnergyGaugeBar )
				{
				//	ALT키가 눌러있을때는 옵션과 반대로 되게 해야 한다.
					bDrawGauge = !bDrawGauge;
				}
			}			
		}
	}

	//Draw 유무 검사
	bool bRealDraw = false;
	if( true == bDrawGauge )
	{
		if( EGAUGE_PET==m_iType )
		{
			if(false==m_bIsMyPet)
			{
				return;
			}
		}
		bRealDraw = IsDefaultView();
	}

	//실제 그림
	if( bRealDraw || IsAlwaysView() )
	{
		NiColorA kSpecular = NiColorA::BLACK;

		if(m_bBlinkToggle)
		{
			//kSpecular = NiColor::WHITE;

			float fR = lua_tinker::call<float>("GetHpGaugeBlinkColorR");
			float fG = lua_tinker::call<float>("GetHpGaugeBlinkColorG");
			float fB = lua_tinker::call<float>("GetHpGaugeBlinkColorB");
			float fA = lua_tinker::call<float>("GetHpGaugeBlinkColorA");

			kSpecular = NiColorA(fR,fG,fB,fA);
		}

		PgUIScene::Render_UIObject( pkRenderer, m_spGaugeOut, PgRenderer::E_PS_NONE, stRenderOption( NiColorA( 0.0f, 0.0f, 0.0f, 0.0f), false, m_fScale, 0, kSpecular, POINT2F(0.5f,0.5f)));
		PgUIScene::Render_UIObject( pkRenderer, m_spGaugeIn, PgRenderer::E_PS_NONE, stRenderOption( NiColorA( 0.0f, 0.0f, 0.0f, 0.0f), false, m_fScale, 0, kSpecular));
		if( !m_kExternTexture.empty() )
		{
			switch( m_iType )
			{
			case EGAUGE_CHARGING1:
				{
					PgUIScene::Render_UIObject( pkRenderer, m_kExternTexture.at(m_iCurIndex), PgRenderer::E_PS_NONE, stRenderOption( NiColorA( 0.0f, 0.0f, 0.0f, 0.0f), false, m_fScale, 0, kSpecular));
				}break;
			default:
				{
					ContExternTexture::iterator iter = m_kExternTexture.begin();
					while( m_kExternTexture.end() != iter )
					{
						PgUIScene::Render_UIObject( pkRenderer, (*iter), PgRenderer::E_PS_NONE, stRenderOption( NiColorA( 0.0f, 0.0f, 0.0f, 0.0f), false, m_fScale, 0, kSpecular));
						++iter;
					}
				}break;
			}
		}
		DrawHPParts(pkRenderer);
	}
}

void	PgEnergyGauge::SetPosition(NiPoint3 const& kTargetPos,const NiCamera* pkCamera)
{
	if ( m_spGaugeOut && m_spGaugeOut->GetNumScreenRects()>0 && pkCamera)
	{
		NiScreenTexture::ScreenRect &kRect1 = m_spGaugeOut->GetScreenRect(0);

		float const fCorrect = static_cast<float>(kRect1.m_usPixHeight) / 2.8f;
		m_kTargetPos = kTargetPos - (pkCamera->GetWorldUpVector() * fCorrect);

		float	fBx=0.0f,fBy=0.0f;

		if( m_spGaugeIn && m_spGaugeIn->GetNumScreenRects()>0 && pkCamera->WorldPtToScreenPt( m_kTargetPos, fBx, fBy ) )
		{
			SetVisible(true);

			NiRenderer *pkRenderer = NiRenderer::GetRenderer();
			int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
			int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

			int const iCenterX = (int)(iScreenWidth * fBx);
			int const iCenterY = (int)(iScreenHeight * (1-fBy));

			kRect1.m_sPixLeft = iCenterX - kRect1.m_usPixWidth/2;
			kRect1.m_sPixTop = iCenterY - kRect1.m_usPixHeight/2;

			NiScreenTexture::ScreenRect &kRect2 = m_spGaugeIn->GetScreenRect(0);
			unsigned int const iWidht2 = m_spGaugeIn->GetTexture()->GetWidth();
			unsigned int const iHeight2 = m_spGaugeIn->GetTexture()->GetHeight();

			switch ( m_iType )
			{
			case EGAUGE_CHARGING1:
				{
					kRect2.m_sPixLeft = kRect1.m_sPixLeft;
					kRect2.m_sPixTop = kRect1.m_sPixTop;
				}break;
			case EGAUGE_CHARGING2:
			case EGAUGE_DEFAULT:
			case EGAUGE_SUMMONED:
			case EGAUGE_PET:
			case EGAUGE_ALIVETIME:
				{
					kRect2.m_sPixLeft = kRect1.m_sPixLeft + 3;
					kRect2.m_sPixTop = kRect1.m_sPixTop + 3;
				}break;
			default:
				{
					kRect2.m_sPixLeft = kRect1.m_sPixLeft + ((m_spGaugeOut->GetTexture()->GetWidth() - iWidht2) / 2);
					kRect2.m_sPixTop = kRect1.m_sPixTop + ((m_spGaugeOut->GetTexture()->GetHeight() - iHeight2) / 2);
				}break;
			}

			if ( m_bAutoScale )
			{
				float const fBaseTarget = PgCameraModeFollow::GetCameraInfo().kCurrentEnd.fDistanceFromTarget * PgCameraModeFollow::GetCameraInfo().kCurrentEnd.fDistanceFromTarget;

				NiPoint3 const pt3 = m_kTargetPos - pkCamera->GetWorldLocation();
				float const fSqrLength = pt3.SqrLength();
				if ( fSqrLength > fBaseTarget )
				{
					m_fScale = 1.0f - (((fSqrLength - fBaseTarget) / fBaseTarget) * 0.1f);
					if ( m_fScale < 0.2f )
					{
						m_fScale = 0.2f;
					}

					kRect2.m_sPixLeft += (static_cast<float>(iWidht2/2) * ( 1.0f - m_fScale ));
					kRect2.m_sPixTop += (static_cast<float>(iHeight2/2) * ( 1.0f - m_fScale ));
				}
				else
				{
					m_fScale = 1.0f;
				}
			}
			
			m_spGaugeOut->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			m_spGaugeIn->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

			if( !m_kExternTexture.empty() )
			{
				ContExternTexture::iterator iter = m_kExternTexture.begin();
				while( m_kExternTexture.end() != iter )
				{
					if( (*iter) && 0 < (*iter)->GetNumScreenRects()  )
					{
						NiScreenTexture::ScreenRect &kRect3 = (*iter)->GetScreenRect(0);
						switch( m_iType )
						{
						case EGAUGE_CHARGING1:
							{
								kRect3.m_sPixLeft = kRect1.m_sPixLeft + 124;
								kRect3.m_sPixTop = kRect1.m_sPixTop - 15;
							}break;
						default:
							{
								kRect3.m_sPixLeft = kRect1.m_sPixLeft;
								kRect3.m_sPixTop = kRect1.m_sPixTop;
							}break;
						}
						(*iter)->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
					}
					++iter;
				}
			}

			DoClipping();
		}
		else
		{
			SetVisible(false);
		}
	}
}
void	PgEnergyGauge::DoClipping()
{
	if(m_iMaxValue == 0) return;
	if(m_fVisibleStartTime == ms_fNonVisibleTime) return;

	SetVisible(true);

	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	if(!m_spGaugeOut || 0>=m_spGaugeOut->GetNumScreenRects())
	{
		return;
	}

	NiScreenTexture::ScreenRect &kRect1 = m_spGaugeOut->GetScreenRect(0);

	switch ( m_iType )
	{
	case EGAUGE_DEFAULT:
		{
			kRect1.m_usPixWidth = 96;
			kRect1.m_usPixHeight = 18;
			kRect1.m_usTexLeft = 0;
			kRect1.m_usTexTop = 12;
		}break;
	case EGAUGE_CHARGING1:
		{
			kRect1.m_usPixWidth = m_spGaugeOut->GetTexture()->GetWidth();
			kRect1.m_usPixHeight = m_spGaugeOut->GetTexture()->GetHeight() / 2;
			kRect1.m_usTexLeft = 0;
			kRect1.m_usTexTop = 0;
		}break;	
	case EGAUGE_PET:
	case EGAUGE_ALIVETIME:
		{
			kRect1.m_usPixWidth = 96;
			kRect1.m_usPixHeight = 12;
			kRect1.m_usTexLeft = 0;
			kRect1.m_usTexTop = 6;
		}break;
	case EGAUGE_SUMMONED:
		{
			kRect1.m_usPixWidth = 96;
			kRect1.m_usPixHeight = 14;
			kRect1.m_usTexLeft = 0;
			kRect1.m_usTexTop = 8;
		}break;
	default:
		{
			kRect1.m_usPixWidth = m_spGaugeOut->GetTexture()->GetWidth();
			kRect1.m_usPixHeight = m_spGaugeOut->GetTexture()->GetHeight();
			kRect1.m_usTexLeft = 0;
			kRect1.m_usTexTop = 0;
		}break;
	}

	if(kRect1.m_sPixLeft>=iScreenWidth || kRect1.m_sPixLeft+kRect1.m_usPixWidth<=0 || kRect1.m_sPixTop>=iScreenHeight || kRect1.m_sPixTop+kRect1.m_usPixHeight<=0)
	{
		SetVisible(false);
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgEnergyGaugeMan
///////////////////////////////////////////////////////////////////////////////////////////////////
void	PgEnergyGaugeMan::Destroy()
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_NodeList.clear();
}

PgEnergyGauge*	PgEnergyGaugeMan::CreateNewGauge( PgPilot const* pkPilot )
{
	if(!pkPilot)
	{
		return NULL;
	}

	if(0 < pkPilot->GetAbil(AT_HIDE_HP_GAGE))
	{// HP 게이지를 붙이지 않음
		return NULL;
	}

	PgEnergyGauge	*pkNewGauge;

	if ( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP) )
	{
		pkNewGauge = new PgEnergyGauge( pkPilot );
	}
	else
	{
		switch( pkPilot->GetAbil(AT_GRADE) )
		{
		case EMGRADE_ELITE:
		case EMGRADE_BOSS:
			{
				if ( pkPilot->GetUnit()->IsUnitType(UT_MONSTER) )
				{// 정예몬스터
					pkNewGauge = new PgEnergyGaugeBig( pkPilot );
					break;
				}
			} // No break
		default:
			{
				if(pkPilot->GetAbil(AT_USE_ENERGY_GAUGE_BIG))
				{
					pkNewGauge = new PgEnergyGaugeBig( pkPilot );
				}
				else
				{
					pkNewGauge = new PgEnergyGauge( pkPilot );
				}
			}break;
		}
	}

	m_NodeList.push_back(pkNewGauge);
	return	pkNewGauge;
}

PgEnergyGauge*	PgEnergyGaugeMan::CreateInstallNewGauge(float fTotalTime) //설치류 타이머 게이지
{
	PgInstallTimerGauge	*pkNewGauge = new PgInstallTimerGauge;
	PG_ASSERT_LOG( pkNewGauge );
	if ( pkNewGauge )
	{
		pkNewGauge->SetTotalTime(fTotalTime);
		m_NodeList.push_back(pkNewGauge);	
	}
	return	pkNewGauge;
}

PgEnergyGauge*	PgEnergyGaugeMan::CreateAliveTimeNewGauge(float fTotalTime) //설치류 타이머 게이지
{
	PgAliveTimeGauge	*pkNewGauge = new PgAliveTimeGauge;
	PG_ASSERT_LOG( pkNewGauge );
	if ( pkNewGauge )
	{
		pkNewGauge->SetTotalTime(fTotalTime);
		m_NodeList.push_back(pkNewGauge);
	}
	return	pkNewGauge;
}

void	PgEnergyGaugeMan::DestroyGauge(PgEnergyGauge *pkGauge)
{
	if(!pkGauge) return;

	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); ++itor)
	{
		if(*itor == pkGauge)
		{
			SAFE_DELETE(pkGauge);
			m_NodeList.erase(itor);
			return;
		}	
	}

	NILOG(PGLOG_ASSERT,"PgEnergyGaugeMan::DestroyGauge() Not Found Requested Gauge Object\n");
}

void PgEnergyGaugeMan::Update(float fAccumTime,float fFrameTime)
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); ++itor)
	{
		(*itor)->Update(fAccumTime,fFrameTime);
	}
}
void PgEnergyGaugeMan::DrawImmediate(PgRenderer *pkRenderer)
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); ++itor)
	{
		(*itor)->DrawImmediate(pkRenderer);
	}
}


PgInstallTimerGauge::PgInstallTimerGauge()
{
	Init( EGAUGE_DEFAULT );
	m_bUpdatePosition = false;
	m_fStartAccumTime = 0.0f;
	m_fTotalTime = 0.0f;
	m_fCurTime = 0.0f;
}

PgInstallTimerGauge::~PgInstallTimerGauge()
{
}

void PgInstallTimerGauge::Update(float fAccumTime,float fFrameTime)
{
	m_fCurTime = fAccumTime;

	float fRate = (m_fTotalTime - m_fCurTime) / (m_fTotalTime - m_fStartAccumTime);

	fRate = NiClamp(fRate, 0.0f, 1.0f);

	SetBarLength(fRate);

}
void PgInstallTimerGauge::DrawImmediate(PgRenderer *pkRenderer)
{
	if(m_bUpdatePosition)
	{
		if(m_spGaugeOut)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spGaugeOut);
		}
		if(m_spGaugeIn)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spGaugeIn);
		}
	}
}
void PgInstallTimerGauge::SetBarValue(int iMax,int iBefore,int iNew)
{
	// 아무일도 하지 않음
}
void PgInstallTimerGauge::SetTotalTime(float fTotalTime)
{
	m_fStartAccumTime = g_pkApp->GetAccumTime();
	m_fTotalTime = m_fStartAccumTime + fTotalTime;
}

void PgInstallTimerGauge::SetPosition(NiPoint3 const &kTargetPos,const NiCamera *pkCamera)
{
	if(!m_bUpdatePosition)
	{
		m_fTotalTime -= g_pkApp->GetAccumTime() - m_fStartAccumTime;
	}

	m_bUpdatePosition = true;	
	NiPoint3 kNewPos = kTargetPos;
	kNewPos.y += 15.0f;
	PgEnergyGauge::SetPosition(kNewPos, pkCamera);
}


//
// 살아있는 시간 표시 게이지
//
PgAliveTimeGauge::PgAliveTimeGauge()
{
	Init( EGAUGE_ALIVETIME );
	m_bUpdatePosition = false;
	m_fStartAccumTime = 0.0f;
	m_fTotalTime = 0.0f;
	m_fCurTime = 0.0f;
}

PgAliveTimeGauge::~PgAliveTimeGauge()
{
}

void PgAliveTimeGauge::Update(float fAccumTime,float fFrameTime)
{
	m_fCurTime = fAccumTime;

	float fRate = (m_fTotalTime - m_fCurTime) / (m_fTotalTime - m_fStartAccumTime);

	fRate = NiClamp(fRate, 0.0f, 1.0f);

	SetBarLength(fRate);
}

void PgAliveTimeGauge::DrawImmediate(PgRenderer *pkRenderer)
{
	if(m_bUpdatePosition && GetVisible())
	{
		if(m_spGaugeOut)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spGaugeOut);
		}
		if(m_spGaugeIn)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spGaugeIn);
		}
	}
}
void PgAliveTimeGauge::SetBarValue(int iMax,int iBefore,int iNew)
{
	// 아무일도 하지 않음
}
void PgAliveTimeGauge::SetTotalTime(float fTotalTime)
{
	m_fStartAccumTime = g_pkApp->GetAccumTime();
	m_fTotalTime = m_fStartAccumTime + fTotalTime;
}

void PgAliveTimeGauge::SetPosition(NiPoint3 const &kTargetPos,const NiCamera *pkCamera)
{
	if(!m_bUpdatePosition)
	{
		m_fTotalTime -= g_pkApp->GetAccumTime() - m_fStartAccumTime;
	}

	m_bUpdatePosition = true;
	NiPoint3 kNewPos = kTargetPos;
	kNewPos.y -= 5.0f;
	PgEnergyGauge::SetPosition(kNewPos, pkCamera);
}