#include "stdafx.h"
#include "lwPlayTime.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include <shellapi.h>

TCHAR lwPlayTime::ms_szURL[MAX_PATH];

lwPlayTime::lwPlayTime()
:	m_dwNextWarnSec(0)
,	m_dwNextTimerUpdate(0)
,	m_iRate(0)
,	m_bViewClock(false)
{
	m_dtLastLogout.Clear();
}

lwPlayTime::~lwPlayTime()
{

}

void lwPlayTime::Activate(char *pszName)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( !pkPlayer )
	{
		return;
	}

	m_bViewClock = pkPlayer->IsViewClock();
	m_kMsg.Clear();
	m_dwNextWarnSec = 0;
	int iRate = 100;
	pkPlayer->CalcExpRate(iRate);
	if ( m_bViewClock )
	{
		SetRate(iRate);
		lwActivateUI(pszName);
	}
}

void GetTimeString(int const iSec, std::wstring & kTime)
{
	int iMin = iSec / 60;
	int iHour = iMin / 60;
	iMin = iMin % 60; 

	wchar_t szTemp[MAX_PATH] = {0,};
	if(iHour > 0)
	{
		int iLen = wcslen(szTemp);
		if(iMin > 0)
		{
			swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iHour, TTW(175).c_str());
		}
		else
		{//1시 0분일때 1시간으로 표시하기 위한 처리
			swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iHour, TTW(5026).c_str());
		}
	}
	if(iMin > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iMin, TTW(176).c_str());
	}

	kTime = szTemp;

	if(kTime.empty())
	{
		kTime = L"0";
		kTime += TTW(176);
	}
}

namespace lwPlayTimeUtil
{
	typedef struct tagChatColor
	{
		tagChatColor()
			: dwFontColor(0xFFFFFFFF), dwOutlineColor(0xFFFFFFFF)
		{}

		tagChatColor(DWORD const dwFont, DWORD const dwOutline)
			: dwFontColor(dwFont), dwOutlineColor(dwOutline)
		{}

		DWORD dwFontColor, dwOutlineColor;
	}SChatColor;

	const static SChatColor aColorSet[] = 
	{
		SChatColor(0xFFFFF954, 0xFF000000),//Normal
		SChatColor(0xFFFF2A2F, 0xFF000000),//Warning
	};
	const static int ColorSize = sizeof(aColorSet) / sizeof(SChatColor);

	void ShowNotice(BM::vstring const & vstrText, int const iType)
	{
		SChatColor kColor;

		if(ColorSize < iType)
		{
			kColor = aColorSet[0];
		}
		else
		{
			kColor = aColorSet[iType];
		}

		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(kChatLog, vstrText);

		XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate(L"WarnOut_PlayerPlayTime");
		if( pkWnd )
		{
			pkWnd->CalledTime( BM::GetTime32() );
			pkWnd->Text( vstrText );
			pkWnd->Size(pkWnd->Size().x, Pg2DString::CalculateOnlySize((XUI::CXUI_Style_String)pkWnd->StyleText()).y + 2);
			pkWnd->FontColor(kColor.dwFontColor);
			pkWnd->OutLineColor(kColor.dwOutlineColor);
			XUI::CXUI_Wnd* pkBgWnd = pkWnd->GetControl(_T("IMG_BG"));
			if( pkBgWnd )
			{
				pkBgWnd->Size(pkBgWnd->Size().x, pkWnd->Size().y);
				pkBgWnd->ImgSize(pkBgWnd->Size());
			}
		}
	}
}

bool lwPlayTime::CallPlayTimeWarn(DWORD const dwCurTime)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->IsUse() )
	{
		return false;
	}

	int const iAccConnSec = pkPlayer->GetAccConnSec_Member();

	if( m_dwNextWarnSec > iAccConnSec )
	{
		return false;
	}

	switch(m_kMsg.iStyle)
	{
	case 1:
		{
			int const iAccConnMin = iAccConnSec / 60;
			int const iPlayTimeMin = g_kDefPlayTime.PlayTimeMin();
			BM::vstring vstrText(TTW(m_kMsg.iNo)); 
			vstrText.Replace( L"#Min#", iPlayTimeMin-iAccConnMin );
			lwPlayTimeUtil::ShowNotice(vstrText, 0);
		}break;
	case 2:
		{
			BM::vstring vstrText(TTW(m_kMsg.iNo)); 
			vstrText.Replace( L"#Per#", m_iRate );
			lwPlayTimeUtil::ShowNotice(vstrText, 1);
		}break;
	case 3:
		{
			lwPlayTimeUtil::ShowNotice(BM::vstring(TTW(m_kMsg.iNo)), 0);
		}break;
	case 4:
		{
			lwPlayTimeUtil::ShowNotice(BM::vstring(TTW(m_kMsg.iNo)), 1);
		}break;
	case 100:
		{
			//{Name}님 누적 온라인 시간이 {ConnTime}이 됐습니다.
			//피로 시간까지는 {RemainTime}이 남아있습니다.
			//1.
			int iAccConnMin = iAccConnSec / 60;
			int iAccConnHour = iAccConnMin / 60;
			iAccConnMin = iAccConnMin % 60; 
	
			std::wstring kAccTime;
			GetTimeString(iAccConnSec, kAccTime);
			BM::vstring vstrText(TTW(80)); 
			vstrText.Replace( L"#Name#", pkPlayer->Name() );
			vstrText.Replace( L"#ConnTime#", kAccTime );
			Notice_Show( vstrText, EL_Normal );

			//2.
			int iPlayTimeSec = g_kDefPlayTime.PlayTimeSec() - iAccConnSec;
			if(iPlayTimeSec > 0)
			{
				std::wstring kRemainTime;
				GetTimeString(iPlayTimeSec, kRemainTime);
				BM::vstring kText2(TTW(799007)); 
				kText2.Replace( L"#RemainTime#", kRemainTime );
				Notice_Show( kText2, EL_Normal );
			}
		}break;
	case 101:
		{
			//{Name}님 누적 온라인 시간이 {ConnTime}이 됐습니다.
			//피로 게임 시간 중입니다.
			//{Name}님은 보상을 {Per}%만 받게 됩니다.
			//로그아웃하여 휴식을 취하시기 바랍니다.
			//1.
			int iAccConnMin = iAccConnSec / 60;
			int iAccConnHour = iAccConnMin / 60;
			iAccConnMin = iAccConnMin % 60; 
	
			std::wstring kAccTime;
			GetTimeString(iAccConnSec, kAccTime);
			BM::vstring vstrText(TTW(80)); 
			vstrText.Replace( L"#Name#", pkPlayer->Name() );
			vstrText.Replace( L"#ConnTime#", kAccTime );
			Notice_Show( vstrText, EL_Normal );

			//2.
			Notice_Show( TTW(799008), EL_Normal );

			//3.
			BM::vstring kText3(TTW(87)); 
			kText3.Replace( L"#Name#", pkPlayer->Name() );
			kText3.Replace( L"#Per#", m_iRate );
			Notice_Show( kText3, EL_Normal );

			//4.
			Notice_Show( TTW(799010), EL_Normal );
		}break;
	case 102:
		{
			//{Name}님 누적 온라인 시간이 5시간이 됐습니다.
			//불 건강 게임 시간 중입니다.
			//{Name}님은 보상을 {Per}%만 받게 됩니다.
			//로그아웃하여 휴식을 취하시기 바랍니다.

			//1.
			int iAccConnMin = iAccConnSec / 60;
			int iAccConnHour = iAccConnMin / 60;
			iAccConnMin = iAccConnMin % 60; 
	
			std::wstring kAccTime;
			GetTimeString(iAccConnSec, kAccTime);
			BM::vstring vstrText(TTW(80)); 
			vstrText.Replace( L"#Name#", pkPlayer->Name() );
			vstrText.Replace( L"#ConnTime#", kAccTime );
			Notice_Show( vstrText, EL_Warning );

			//2.
			Notice_Show( TTW(799009), EL_Warning );

			//3.
			BM::vstring kText3(TTW(87)); 
			kText3.Replace( L"#Name#", pkPlayer->Name() );
			kText3.Replace( L"#Per#", m_iRate );
			Notice_Show( kText3, EL_Warning );

			//4.
			Notice_Show( TTW(799010), EL_Warning );
		}break;
	case 0:
		{
		}break;
	default:
		{
			std::wstring wstrMsg;
			int const iAccConnHour = iAccConnSec / 3600;
			int iRate = pkPlayer->GetExpRate();
			switch( iAccConnHour )
			{
			case 0:
				{	
				}break;
			case 1:
			case 2:
				{
					if ( SetRate(iRate) )
					{
						WstringFormat( wstrMsg, MAX_PATH, TTW(82).c_str(), iRate );
					}
					else
					{
						WstringFormat( wstrMsg, MAX_PATH, TTW(80).c_str(), iAccConnHour );
					}
				}break;
			case 3:
			case 4:
				{
					if ( SetRate(iRate) )
					{
						WstringFormat( wstrMsg, MAX_PATH, TTW(82).c_str(), iRate );
					}
					else
					{
						WstringFormat( wstrMsg, MAX_PATH, TTW(81).c_str() );
					}
				}break;
			default:
				{
					SetRate(iRate);
					WstringFormat( wstrMsg, MAX_PATH, TTW(83).c_str() );
				}break;
			}

			if ( wstrMsg.size() )
			{
				Notice_Show( wstrMsg, EL_Warning );
			}
		}break;
	}

	m_kMsg.Clear();

	SetNextWarnMsgTime(iAccConnSec);
	return true;
}

void lwPlayTime::CallTimerToolTip(lwUIWnd UIWnd)
{
	PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if ( !pkMyPlayer )
	{
		return;
	}

	if ( UIWnd.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd =UIWnd.GetSelf();;
	if ( !pkWnd )
	{
		return;
	}

	POINT3I loc = pkWnd->Location();
	loc.x -= 215;
	loc.y += 20;
	
//	pkMyPlayer->Update_PlayTime(BM::GetTime32());

	//int const iThisConnMin = pkMyPlayer->GetThisConnSec_Character() / 60;	
	int iThisConnMin = 0;
	if( pkMyPlayer->GetThisConnSec_Character() > 0 )
	{
		iThisConnMin = (pkMyPlayer->GetThisConnSec_Character() + g_kNetwork.oldSelectCharacterSec()) / 60;
	}
	else
	{
		iThisConnMin = g_kNetwork.oldSelectCharacterSec() / 60;
	}
	int const iAccConnMin = pkMyPlayer->GetAccConnSec_Member() / 60;
	int const iAccDisMin = pkMyPlayer->GetAccDisConnSec_Member() / 60;
	std::wstring wstrText;
	std::wstring wstrTime;

	wstrTime = TTW(GetTimeTextValue());
	
	if( pkMyPlayer->IsUse() )
	{
		typedef PgDefPlayerPlayTimeImpl::EDefPlayTimeResetType EDPTT;
		
		if(EDPTT::EDPTRT_ACCDISCON == g_kDefPlayTime.ResetType())
		{
			WstringFormat( wstrText, MAX_PATH, TTW(90).c_str(),iThisConnMin/60,iThisConnMin%60, iAccConnMin/60, iAccConnMin%60,
				wstrTime.c_str(), m_iRate, iAccDisMin/60, iAccDisMin%60);
		}
		else
		{
			WstringFormat( wstrText, MAX_PATH, TTW(799012).c_str(),iThisConnMin/60,iThisConnMin%60, iAccConnMin/60, iAccConnMin%60,
				wstrTime.c_str(), m_iRate);
		}
	}
	else
	{
		WstringFormat( wstrText, MAX_PATH, TTW(990).c_str(),iThisConnMin/60,iThisConnMin%60 );		
	}
	lwCallToolTipByText(0, lwWString(wstrText.c_str()), lwPoint2(loc.x,loc.y), "ToolTip_Timer");
	return;
}

int lwPlayTime::GetTimeTextValue()
{
	int iTTW = 0;

	if(m_iRate>50)
	{
		iTTW = 991;
	}
	else if(m_iRate>0)
	{
		iTTW = 992;
	}
	else
	{
		iTTW = 993;
	}

	return iTTW;
}

void lwPlayTime::SetRateTime()
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( (NULL==pkPlayer)
	|| (false==pkPlayer->IsUse()) )
	{
		return;
	}

	SetRate( pkPlayer->GetExpRate() );
}

void lwPlayTime::SetNextWarnSec(int const iSec)
{
	m_kMsg.Clear();
	m_dwNextWarnSec = iSec;
}

void lwPlayTime::Update()
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get(L"BTN_TIMER");
	if ( !pkWnd )
	{
		return;
	}

	if ( !m_bViewClock )
	{
		pkWnd->Close();
		return;
	}	

	// 메시지 출력
	DWORD dwNowTime = BM::GetTime32();
	CallPlayTimeWarn(dwNowTime);

	SetRateTime();

	if ( dwNowTime >= m_dwNextTimerUpdate )
	{// 모래시계 UV좌표 바꾸기
		XUI::CXUI_Wnd *pkClockWnd = pkWnd->GetControl(_T("FRM_HOURGLASS"));
		if ( pkClockWnd )
		{
			const SUVInfo& kUVInfo = pkClockWnd->UVInfo();
			size_t iIndex = kUVInfo.Index + 1;
			if ( iIndex > (kUVInfo.U*kUVInfo.V) )
			{
				iIndex = 1;
			}
			if( m_wstrImgName.empty() )
			{
				SetImgName();
			}
			pkClockWnd->DefaultImgName(m_wstrImgName);
			pkClockWnd->UVUpdate(iIndex);
		}

		m_dwNextTimerUpdate = dwNowTime + 5000;//5초뒤에 Update
	}
}

void lwPlayTime::SetImgName()
{
	if( m_iRate > 50 )
	{
		m_wstrImgName = _T("../Data/6_ui/main/mnTimeG.tga");
	}
	else if( m_iRate > 0 )
	{
		m_wstrImgName = _T("../Data/6_ui/main/mnTimeY.tga");
	}
	else
	{
		m_wstrImgName = _T("../Data/6_ui/main/mnTimeR.tga");
	}
}

bool lwPlayTime::SetRate(int const iRate)
{
	if ( m_iRate == iRate )
	{
		return false;
	}
	m_iRate = iRate;
	SetImgName();
	return true;
}

void lwPlayTime::SetNextWarnMsgTime(int const iAccConnSec)
{	
	m_dwNextWarnSec = g_kDefPlayTime.GetNextMsgSec(iAccConnSec, m_kMsg);
}

bool lwPlayTime::IsPickUpItem()const
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkPlayer )
	{
		return pkPlayer->IsTakeUpItem();
	}
	return false;
}

bool lwPlayTime::CallFatigueUI()
{
	if( NULL != XUIMgr.Call(L"SFRM_FATIGUE", false))
	{
		return true;
	}

	return false;
}

void lwPlayTime::RunInternetBrowser()
{
	HINSTANCE hInstance = ::ShellExecute(NULL, _T("open") , ms_szURL, NULL, NULL, SW_SHOWNORMAL);
	if(hInstance)
	{
		::PostQuitMessage(0);
	}
}

void lwPlayTime::SetLastLogOutTime(BM::DBTIMESTAMP_EX const dtLastLogout)
{
	m_dtLastLogout = dtLastLogout;
}

BM::DBTIMESTAMP_EX lwPlayTime::GetLastLogOutTime()
{
	return m_dtLastLogout;
}