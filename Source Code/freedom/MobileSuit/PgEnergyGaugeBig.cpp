#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgPilot.h"
#include "PgChatMgrClient.h"
#include "PgEnergyGaugeBig.h"
#include "lwUI.h"

std::wstring const	PgEnergyGaugeBig::ms_wstrBarName(_T("FRM_MAIN_GAUGEBAR"));
std::wstring const	PgEnergyGaugeBig::ms_wstrBarNameBoss(_T("FRM_BOSS_GAUGEBAR"));

PgEnergyGaugeBig* PgEnergyGaugeBig::ms_pkRendering = NULL;
bool PgEnergyGaugeBig::ms_bShow = true;

PgEnergyGaugeBig::PgEnergyGaugeBig( PgPilot const *pkPilot )
:	m_bBoss(false)
,	m_kBossNo(0)
,	m_ucCallWarning(0)
,	m_iIconNo(0)
,	m_bIsNoAliveTime(false)
{
	if( EGAUGE_ELITE == pkPilot->GetAbil(AT_HP_GAUGE_TYPE) )
	{
		m_iType = EGAUGE_ELITE;
	}
	else
	{
		m_iType = EGAUGE_BOSS;
	}
	ClearVisible();
	PgEnergyGaugeBig::Reset( pkPilot );
}

PgEnergyGaugeBig::~PgEnergyGaugeBig()
{
	ClearVisible();
}

void PgEnergyGaugeBig::Show( bool bShow )
{
	if ( ms_bShow != bShow )
	{
		ms_bShow = bShow;

		if ( ms_pkRendering )
		{
			ms_pkRendering->Show( -1 );
		}
	}
}

bool PgEnergyGaugeBig::Show( int const iValue )
{
	if ( ms_pkRendering && (this != ms_pkRendering) )
	{
		if ( !IsBoss() && ms_pkRendering->IsBoss() )
		{
			return false;
		}
	}
	ms_pkRendering = this;

	XUI::CXUI_Wnd* pWnd = NULL;
	if ( IsBoss() )
	{
		XUIMgr.Close( ms_wstrBarName );
		pWnd = XUIMgr.Activate( ms_wstrBarNameBoss, false );
	}
	else
	{
		pWnd = XUIMgr.Activate( ms_wstrBarName, false );
		if(pWnd)
		{
			pWnd->Text(m_wstrName);
		}
	}	

	if ( !pWnd )
	{
		NILOG(PGLOG_ERROR, "Can't find ID='%s'", ms_wstrBarName.c_str() );
		ClearVisible();
		return false;
	}

	if( m_bBoss )
	{
		if( m_bIsNoAliveTime )
		{
			pWnd->AliveTime(0);
		}
		else
		{
			DWORD const dwAliveTime = 4000;
			pWnd->AliveTime(dwAliveTime);
		}
	}
	else
	{
		pWnd->AliveTime(0);
	}

	if ( 0 < BossNo() )
	{
		if (XUI::CXUI_Wnd* pkImg = pWnd->GetControl(_T("ICN_BOSS")))
		{
			pkImg->SetCustomData(&m_iIconNo, sizeof(m_iIconNo));
		}
	}

	if( XUI::CXUI_AniBar* pBar = dynamic_cast<XUI::CXUI_AniBar*>(pWnd->GetControl(_T("BAR_HP"))) )
	{
		pBar->Max( m_iMaxValue );
		if ( iValue >= 0 )
		{
			pBar->Now( iValue );
		}

		// 경험치 표시를 위해 AniBar가 __int64로 변경되었음. 여기서 쓰는 어빌은 int 이므로 캐스팅 처리.
		// hp 어빌이 __int64로 변경된다면 여기도 변경 필요함.
		std::wstring wstrBarName;
		WstringFormat( wstrBarName, MAX_PATH, TTW(100).c_str(), m_wstrName.c_str(),(m_fNewHPRate*100.0f), static_cast<int>(pBar->Now()), static_cast<int>(pBar->Max()) );
		pBar->Text(wstrBarName);
	}
	m_fVisibleStartTime = 1.0f;

	if ( !ms_bShow )
	{
		XUIMgr.Close( ms_wstrBarName );
		XUIMgr.Close( ms_wstrBarNameBoss );
	}
	else
	{
		lua_tinker::call<void,bool,lwUIWnd,int>("UICallBossAttrUI", true, lwUIWnd(pWnd), BossNo());
	}
	return true;
}

void PgEnergyGaugeBig::Reset( PgPilot const *pkPilot )
{
	if(!g_pkWorld)
	{
		return;
	}
	if ( pkPilot )
	{
		if ( m_ucCallWarning < 2 )
		{
			switch( pkPilot->GetAbil(AT_GRADE) )
			{
			case EMGRADE_ELITE:	
				{
					switch( g_pkWorld->GetAttr() )
					{
					case GATTR_SUPER_GROUND:
					case GATTR_ELEMENT_GROUND:
						{
						}break;
					default:
						{
							if(pkPilot->IsShowWarning())
							{
								lwActivateUI("FRM_WARNING",false);
							}
						}break;
					}
				}break;
			case EMGRADE_BOSS:
				{
					m_bBoss = m_iType != EGAUGE_ELITE;
				}break;
			}
		}
		m_wstrName = pkPilot->GetName();
		BossNo( pkPilot->GetAbil(AT_CLASS) );
		m_iIconNo = pkPilot->GetAbil(AT_DEF_RES_NO);//임시
		SetBarValue( pkPilot->GetAbil(AT_C_MAX_HP), pkPilot->GetAbil(AT_HP), pkPilot->GetAbil(AT_HP) );

		if( 0 != pkPilot->GetAbil(AT_NO_BOSS_HP_HIDE) )
		{
			m_bIsNoAliveTime = true;
		}

		++m_ucCallWarning;

		lua_tinker::call<void,lwGUID,int,int>("SetElitePattenState", lwGUID(pkPilot->GetGuid()), pkPilot->GetAbil(AT_ELITEPATTEN_STATE), pkPilot->GetAbil(AT_GROGGY_MAX));
	}
}

void PgEnergyGaugeBig::ClearVisible()
{
	m_fVisibleStartTime = PgEnergyGauge::ms_fNonVisibleTime;
	if ( this == ms_pkRendering )
	{
		ms_pkRendering = NULL;
		XUIMgr.Close( ms_wstrBarName );
		XUIMgr.Close( ms_wstrBarNameBoss );
	}
}

void PgEnergyGaugeBig::SetBarValue( int iMax, int iBefore, int iNew )
{
	PgEnergyGauge::SetBarValue( iMax, iBefore, iNew );
	Show( iNew );
}

void PgEnergyGaugeBig::Update( float fAccumTime, float fFrameTime )
{
}