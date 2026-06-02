
#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgNetwork.h"
#include "Pg2DString.h"
#include "lwUI.h"
#include "lwLuckyStarUI.h"
#include "PgChatMgrClient.h"
#include <deque>

namespace lwLuckyStarUI
{
	typedef struct tagEventInfo
	{
		BM::GUID kEventGuid;
		int iCostMoney;
		int iEventCount;
		int iDelayTime;
		int iLimitTime;
		int iNowStar;
		int iBeforStar;
		int iLuckyUser;
		int iTotalUser;
		bool bMyReturnResult;
		int iMyLuckyStep;
		int iMyLuckyStar;
		int iLuckyPoint;

		tagEventInfo()
		{
			Clear();
		}
		void Clear()
		{
			kEventGuid.Clear();
			iCostMoney = 0;
			iEventCount = 0;
			iDelayTime = 0;
			iLimitTime = 0;
			iNowStar = 0;
			iBeforStar = 0;
			iLuckyUser = 0;
			iTotalUser = 0;
			bMyReturnResult = false;
			iMyLuckyStep = 0;
			iMyLuckyStar = 0;
			iLuckyPoint = 0;
		}
	}SEVENTINFO;

	typedef struct tagPopupEventInfo
	{
		ELUCKYSTAR_DLG_STATE kID;
		SEVENTINFO kEvent;

		tagPopupEventInfo() : kID(LSDS_NONE) {}
		tagPopupEventInfo(ELUCKYSTAR_DLG_STATE kId, SEVENTINFO const & kInfo)
			: kID(kId), kEvent(kInfo)
		{
		}
	}SPOPUPEVENTINFO;

	typedef std::deque<SPOPUPEVENTINFO> CONT_POPUPINFO;
	CONT_POPUPINFO	kContPopup;
	SEVENTINFO		kCurEvent;
	DWORD			dwNextEventTime = 0;
	DWORD			dwOldEventTick = 0;
	bool bEventJoin = false;
	bool bAutoPopup = true;	

	Loki::Mutex kMutex;


	std::wstring GetStarName(int iStar)
	{
		wchar_t szTemp[MAX_PATH] = {0,};
		if(0<iStar && iStar<13)
		{
			swprintf_s(szTemp, MAX_PATH, L"%s", TTW(2358+iStar).c_str());
		}
		
		return szTemp;
	}

	void AutoPopup()
	{
		if(false==PgWorld::GetNowLoading() && true==bAutoPopup)
		{
			lwLuckyStarPopupOpen();
		}
	}
}

void lwLuckyStarUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "LuckyStarEventInit", &lwLuckyStarUI::lwLuckyStarEventInit);
	def(pkState, "LuckyStarEventUI", &lwLuckyStarUI::lwLuckyStarEventUI);
	def(pkState, "GetLuckyStarEventGuid", &lwLuckyStarUI::lwGetLuckyStarEventGuid);
	def(pkState, "IsLuckyStarAutoPopup", &lwLuckyStarUI::lwIsLuckyStarAutoPopup);	
	def(pkState, "LuckyStarAutoPopup", &lwLuckyStarUI::lwLuckyStarAutoPopup);	
	def(pkState, "LuckyStarPopupOpen", &lwLuckyStarUI::lwLuckyStarPopupOpen);
	def(pkState, "LuckyStarPopupClose", &lwLuckyStarUI::lwLuckyStarPopupClose);
	def(pkState, "LuckyStarResultDisplay", &lwLuckyStarUI::lwLuckyStarResultDisplay);
	def(pkState, "LuckyStarResultCustomDisplay", &lwLuckyStarUI::lwLuckyStarResultCustomDisplay);
	def(pkState, "LuckyStarInfoDisplay", &lwLuckyStarUI::lwLuckyStarInfoDisplay);
	def(pkState, "LuckyStarNextEventTime", &lwLuckyStarUI::lwLuckyStarNextEventTime);
}

void lwLuckyStarUI::lwLuckyStarEventInit()
{
	BM::CAutoMutex kLock(kMutex);

	kCurEvent.Clear();
	kContPopup.clear();
	bEventJoin = false;
	bAutoPopup = true;
	dwNextEventTime = 0;
	dwOldEventTick = 0;
	lua_tinker::call<void>("LuckyStarUI_Clear");
}

bool lwLuckyStarUI::lwIsLuckyStarAutoPopup()
{
	return bAutoPopup;
}

void lwLuckyStarUI::lwLuckyStarAutoPopup(bool bState)
{
	bAutoPopup = bState;
}

lwWString lwLuckyStarUI::lwLuckyStarNextEventTime()
{
	BM::CAutoMutex kLock(kMutex);

	if(dwNextEventTime < 1)
	{
		return L"";
	}

	int iDiffTime = dwNextEventTime - (BM::GetTime32() - dwOldEventTick);
	if(0 < iDiffTime)
	{
		dwNextEventTime = iDiffTime;
		dwOldEventTick = BM::GetTime32();
	}
	else
	{
		dwNextEventTime = 0;
		dwOldEventTick = 0;
		return L"";
	}

	int iSec  = dwNextEventTime / 1000;
	int iMin  = iSec  / 60;
	int iHour = iMin  / 60;
	int iDay  = iHour / 24;
	iSec  = iSec % 60;
	iMin  = iMin % 60;
	iHour = iHour % 24;

	wchar_t szTemp[MAX_PATH] = {0,};
	if(iDay > 0)
	{
		swprintf_s(szTemp, MAX_PATH, L"%d%s ", iDay, TTW(174).c_str());
	}

	if(iHour > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iHour, TTW(175).c_str());
	}

	if(iMin > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iMin, TTW(176).c_str());
	}

	if(iSec > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iSec, TTW(177).c_str());
	}

	return szTemp;
}

void lwLuckyStarUI::lwLuckyStarResultDisplay(lwUIWnd UIWnd)
{
	BM::CAutoMutex kLock(kMutex);

	if (UIWnd.IsNil()) { return; }
	UIWnd.Visible(true);

	if(true == kContPopup.empty()) { return; }
	SEVENTINFO const & kEvent = kContPopup[0].kEvent;

	lwUIWnd kColorBox = UIWnd.GetControl("SFRM_COLORBOX");
	lwUIWnd kLineBox  = UIWnd.GetControl("SFRM_LINEBOX");
	if(kColorBox.IsNil() || kLineBox.IsNil()) { return; }

	lwUIWnd kText = kColorBox.GetControl("FRM_INFO_TEXT");
	lwUIWnd kLuckyUser = kColorBox.GetControl("SFRM_LS_LUCKYUSER_NUM");
	lwUIWnd kTotalUser = kColorBox.GetControl("SFRM_LS_TOTALUSER_NUM");
	if(kText.IsNil() || kLuckyUser.IsNil() || kTotalUser.IsNil()) { return; }		

	wchar_t szTemp[MAX_PATH] = {0,};
	swprintf_s(szTemp, MAX_PATH, TTW(2347).c_str(), 
		kEvent.iEventCount, GetStarName(kEvent.iNowStar).c_str());
	kText.SetStaticText( MB(szTemp) );
	if( _itow(kEvent.iLuckyUser, szTemp, 10) )
	{
		kLuckyUser.SetStaticText( MB(szTemp) );
	}

	if( _itow(kEvent.iTotalUser, szTemp, 10) )
	{
		kTotalUser.SetStaticText( MB(szTemp) );
	}	
	
	if(0 == kEvent.iBeforStar)
	{
		lwUIWnd kImg1 = kLineBox.GetControl("FRM_I1");
		if(kImg1.IsNil()) { return; }

		char szFileName[MAX_PATH] = {0,};
		sprintf_s(szFileName, MAX_PATH, "../Data/6_ui/event/evtLsB%02d.tga", kEvent.iNowStar);
		kImg1.ChangeImage( szFileName, false );
		kImg1.SetCustomData<int>(kEvent.iNowStar);
	}
	else
	{
		lwUIWnd kImg1 = kLineBox.GetControl("FRM_I1");
		lwUIWnd kImg2 = kLineBox.GetControl("FRM_I2");
		if(kImg1.IsNil() || kImg2.IsNil()) { return; }

		char szFileName[MAX_PATH] = {0,};
		sprintf_s(szFileName, MAX_PATH, "../Data/6_ui/event/evtLsB%02d.tga", kEvent.iNowStar);
		kImg1.ChangeImage( szFileName, false );
		kImg1.SetCustomData<int>(kEvent.iNowStar);

		sprintf_s(szFileName, MAX_PATH, "../Data/6_ui/event/evtLsB%02d.tga", kEvent.iBeforStar);
		kImg2.ChangeImage( szFileName, false );
		kImg2.SetCustomData<int>(kEvent.iBeforStar);
	}
}

void lwLuckyStarUI::lwLuckyStarResultCustomDisplay(lwUIWnd UIWnd)
{
	BM::CAutoMutex kLock(kMutex);

	if (UIWnd.IsNil()) { return; }
	UIWnd.Visible(true);

	if(true == kContPopup.empty()) { return; }
	SEVENTINFO const & kEvent = kContPopup[0].kEvent;

	lwUIWnd kColorBox = UIWnd.GetControl("SFRM_COLORBOX");
	if(kColorBox.IsNil()) { return; }

	lwUIWnd kColorBox2_1 = kColorBox.GetControl("SFRM_COLORBOX2_1");
	lwUIWnd kColorBox2_2 = kColorBox.GetControl("SFRM_COLORBOX2_2");
	if(kColorBox2_1.IsNil() || kColorBox2_1.IsNil()) { return; }

	lwUIWnd kText1 = kColorBox.GetControl("FRM_INFO_TEXT");
	lwUIWnd kLuckyPoint = kColorBox2_1.GetControl("SFRM_COLORBOX3");
	lwUIWnd kMyPoint = kColorBox2_2.GetControl("SFRM_COLORBOX3");
	if(kText1.IsNil() || kLuckyPoint.IsNil() || kMyPoint.IsNil()) 
	{ 
		return; 
	}

	int iMsg = 0;
	if(true==kEvent.bMyReturnResult)
	{
		iMsg = 2348;
	}
	else
	{
		iMsg = 2349;
	}

	wchar_t szTemp[MAX_PATH] = {0,};

	swprintf_s(szTemp, MAX_PATH, TTW(iMsg).c_str(), kEvent.iEventCount);
	kText1.SetStaticText( MB(szTemp) );

	swprintf_s(szTemp, MAX_PATH, L"+%d", kEvent.iLuckyPoint);
	kLuckyPoint.SetStaticText( MB(szTemp) );

	swprintf_s(szTemp, MAX_PATH, L"%d", kEvent.iMyLuckyStep);
	kMyPoint.SetStaticText( MB(szTemp) );
}

void lwLuckyStarUI::lwLuckyStarInfoDisplay(lwUIWnd UIWnd)
{
	if (UIWnd.IsNil()) { return;  }
	UIWnd.Visible(true);

	lwUIWnd kColorBox = UIWnd.GetControl("SFRM_COLORBOX");
	if(kColorBox.IsNil()) { return; }
	lwUIWnd kText = kColorBox .GetControl("FRM_INFO_TEXT");
	if(kText.IsNil()) { return; }

	wchar_t szTemp[MAX_PATH] = {0,};
	swprintf_s(szTemp, MAX_PATH, TTW(2352).c_str(), kCurEvent.iEventCount, GetStarName(kCurEvent.iMyLuckyStar).c_str());

	kText.SetStaticText(MB(szTemp));
}

lwGUID lwLuckyStarUI::lwGetLuckyStarEventGuid()
{
	return kCurEvent.kEventGuid;
}

void lwLuckyStarUI::lwLuckyStarPopupOpen()
{
	BM::CAutoMutex kLock(kMutex);
	
	if(false==lua_tinker::call<bool>("LuckyStarUI_Visible"))
	{
		return;
	}

	ELUCKYSTAR_DLG_STATE kState = LSDS_NONE;

	if(true==kContPopup.empty())
	{
		if(true==bEventJoin)
		{
			kState = LSDS_INFOPOP;
		}
		else
		{
			kState = LSDS_SELECTPOP;
		}
	}
	else
	{
		kState = kContPopup[0].kID;
	}

	XUI::CXUI_Wnd* pkWnd = NULL;
	switch(kState)
	{
	case LSDS_SELECTPOP:
		{
			if(false == kContPopup.empty())
			{
				pkWnd = XUIMgr.Activate(L"SFRM_LS_SELECTPOP");
				if(pkWnd)
				{
					XUI::CXUI_Wnd* pCostWnd = pkWnd->GetControl(L"SFRM_LS_COSTMONEY");
					if(pCostWnd)
					{
						pCostWnd->SetCustomData(&kContPopup[0].kEvent.iCostMoney, sizeof(int));
					}
				}
			}
			else if(false == kCurEvent.kEventGuid.IsNull())
			{
				pkWnd = XUIMgr.Activate(L"SFRM_LS_SELECTPOP");
				if(pkWnd)
				{
					XUI::CXUI_Wnd* pCostWnd = pkWnd->GetControl(L"SFRM_LS_COSTMONEY");
					if(pCostWnd)
					{
						pCostWnd->SetCustomData(&kCurEvent.iCostMoney, sizeof(int));
					}
				}
			}
		}break;
	case LSDS_RESULTPOP:
		{
			if(false==kContPopup.empty())
			{
				SEVENTINFO const & kEvent = kContPopup[0].kEvent;
				if(kEvent.iBeforStar==0)
				{
					pkWnd = XUIMgr.Activate(L"SFRM_LS_RESULT_1");
				}
				else
				{
					pkWnd = XUIMgr.Activate(L"SFRM_LS_RESULT_2");
				}
			}			
		}break;
	case LSDS_RESULTCUSTOMPOP:
		{
			pkWnd = XUIMgr.Activate(L"SFRM_LS_RESULT_CUSTOM");
		}break;
	case LSDS_INFOPOP:
		{
			pkWnd = XUIMgr.Activate(L"SFRM_LS_INFOPOP");
		}break;
	case LSDS_CHANGEPOP:
		{
			pkWnd = XUIMgr.Activate(L"SFRM_LS_CHANGEPOP");
		}break;
	default:
		{
			XUIMgr.Close(L"BTN_LUCKYSTAR_ICON");
		}break;
	}
}

void lwLuckyStarUI::lwLuckyStarPopupClose(lwUIWnd UIWnd)
{
	BM::CAutoMutex kLock(kMutex);

	if (false == UIWnd.IsNil())
	{
		UIWnd.Close();
	}

	if(false==kContPopup.empty())
	{
		kContPopup.pop_front();
	}
	
	XUIMgr.Activate(L"BTN_LUCKYSTAR_ICON");
	//Popup할 Window가 없으면 Icon제거
	if(kCurEvent.kEventGuid.IsNull() && true==kContPopup.empty() && false==bEventJoin)
	{		
		XUIMgr.Close(L"BTN_LUCKYSTAR_ICON");
	}

	//Popup할 Window가 있으면서 AutoPopup이 설정된 경우 다음 팝업창을 띄움
	if(false==kContPopup.empty() && lwIsLuckyStarAutoPopup())
	{
		lwLuckyStarPopupOpen();
	}
}

void lwLuckyStarUI::lwLuckyStarEventUI()
{
	if(true==lua_tinker::call<bool>("LuckyStarUI_Visible") &&
		(false==kContPopup.empty() || bEventJoin))
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"BTN_LUCKYSTAR_ICON");

		if(false==kContPopup.empty() && lwIsLuckyStarAutoPopup())
		{
			lwLuckyStarPopupOpen();
		}
	}
	else
	{
		XUIMgr.Close(L"BTN_LUCKYSTAR_ICON");
	}
}

void lwLuckyStarUI::LuckyStarEventClose()
{
	BM::CAutoMutex kLock(kMutex);

	bEventJoin = false;
	kCurEvent.Clear();
	
	bool bClose = false;
	switch( kContPopup.size() )
	{
	case 0:
		{
			bClose = true;
		}break;
	case 1:
		{
			if( LSDS_SELECTPOP == kContPopup[0].kID )
			{
				bClose = true;
			}
		}break;
	}

	if( bClose )
	{
		XUIMgr.Close(L"BTN_LUCKYSTAR_ICON");
	}
}

void lwLuckyStarUI::RecvLuckyStar_Command(WORD const wPacketType, BM::Stream& rkPacket)
{
	BM::CAutoMutex kLock(kMutex);

	switch( wPacketType )
	{
	case PT_M_C_REQ_LUCKYSTAR_JOIN:
		{
			bool bJoinedUser = false;
			kContPopup.clear();
			kCurEvent.Clear();
			rkPacket.Pop(kCurEvent.kEventGuid);
			rkPacket.Pop(kCurEvent.iEventCount);
			rkPacket.Pop(kCurEvent.iDelayTime);
			rkPacket.Pop(kCurEvent.iLimitTime);
			rkPacket.Pop(kCurEvent.iCostMoney);
			rkPacket.Pop(kCurEvent.iLuckyPoint);
			rkPacket.Pop(kCurEvent.iLuckyPoint);
			rkPacket.Pop(bJoinedUser);
			if( kCurEvent.kEventGuid.IsNull() )
			{
				return;
			}

			kContPopup.push_back( SPOPUPEVENTINFO(LSDS_SELECTPOP,kCurEvent));
			if(false == bJoinedUser)
			{
				AutoPopup();
			}
		}break;
	case PT_M_C_ANS_LUCKYSTAR_ENTER:
		{
			ELUCKYSTAR_EVENT_RESULT iErrorType = LSER_SUCCESS;
			rkPacket.Pop(iErrorType);

			if( LuckyStarResult(iErrorType) )
			{
				bEventJoin = true;
				Notice_Show(TTW(2356), EL_Normal);
			}
		}break;
	case PT_M_C_ANS_LUCKYSTAR_CHANGE_STAR:
		{
			ELUCKYSTAR_EVENT_RESULT iErrorType = LSER_SUCCESS;
			rkPacket.Pop(iErrorType);
			if( LuckyStarResult(iErrorType) )
			{
				int iBeforStar = kCurEvent.iMyLuckyStar;
				rkPacket.Pop(kCurEvent.iMyLuckyStar);

				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf_s(szTemp, MAX_PATH, TTW(2355).c_str(), GetStarName(iBeforStar).c_str(), GetStarName(kCurEvent.iMyLuckyStar).c_str());
				Notice_Show(std::wstring(szTemp), EL_Normal);
			}
		}break;
	case PT_M_C_ANS_LUCKYSTAR_USERINFO:
		{
			kCurEvent.Clear();
			rkPacket.Pop(kCurEvent.kEventGuid);
			rkPacket.Pop(kCurEvent.iEventCount);
			rkPacket.Pop(kCurEvent.iDelayTime);
			rkPacket.Pop(kCurEvent.iLimitTime);
			rkPacket.Pop(kCurEvent.iLuckyPoint);
			rkPacket.Pop(dwNextEventTime);
			rkPacket.Pop(kCurEvent.iMyLuckyStar);
			rkPacket.Pop(kCurEvent.iMyLuckyStep);
			
			dwOldEventTick = BM::GetTime32();
			bEventJoin = true;
		}break;
	case PT_M_C_LUCKYSTAR_RESULT:
		{
			rkPacket.Pop(kCurEvent.iEventCount);
			rkPacket.Pop(kCurEvent.iBeforStar);
			rkPacket.Pop(kCurEvent.iNowStar);
			rkPacket.Pop(kCurEvent.iLuckyUser);
			rkPacket.Pop(kCurEvent.iTotalUser);
			rkPacket.Pop(dwNextEventTime);

			dwOldEventTick = BM::GetTime32();

			//제 ??차 행운성은 게자리 입니다.
			//당첨자 / 참여자 : ? / ?
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(2375).c_str(), 
				kCurEvent.iEventCount, GetStarName(kCurEvent.iNowStar).c_str());
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), std::wstring(szTemp));

			swprintf_s(szTemp, MAX_PATH, L"%s / %s : %d / %d", 
				TTW(2371).c_str(), TTW(2372).c_str(), kCurEvent.iLuckyUser, kCurEvent.iTotalUser);
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), std::wstring(szTemp));

			//
			kContPopup.push_back( SPOPUPEVENTINFO(LSDS_RESULTPOP,kCurEvent));
			++kCurEvent.iEventCount;
			AutoPopup();
		}break;
	case PT_M_C_LUCKYSTAR_RESULT_CUSTOM:
		{
			rkPacket.Pop(kCurEvent.bMyReturnResult);
			rkPacket.Pop(kCurEvent.iEventCount);
			rkPacket.Pop(kCurEvent.iMyLuckyStep);

			//축하 or 안타 ?차 행운성에..
			//추가된 행운성 지수 : ?
			//누적된 행운성 지수 : ?
			int iMsg = 0;
			if(true==kCurEvent.bMyReturnResult)
			{
				iMsg = 2376;
			}
			else
			{
				iMsg = 2377;
			}

			wchar_t szTemp[MAX_PATH] = {0,};

			swprintf_s(szTemp, MAX_PATH, TTW(iMsg).c_str(), kCurEvent.iEventCount);
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), std::wstring(szTemp));

			swprintf_s(szTemp, MAX_PATH, L"%s : +%d", TTW(2350).c_str(), kCurEvent.iLuckyPoint);
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), std::wstring(szTemp));

			swprintf_s(szTemp, MAX_PATH, L"%s : %d", TTW(2351).c_str(), kCurEvent.iMyLuckyStep);
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), std::wstring(szTemp));

			//
			kContPopup.push_back( SPOPUPEVENTINFO(LSDS_RESULTCUSTOMPOP,kCurEvent));
			++kCurEvent.iEventCount;
			AutoPopup();
		}break;
	case PT_M_C_LUCKYSTAR_CLOSE:
		{
			BM::GUID kEventGuid;
			std::wstring kEventNotiClose;
			rkPacket.Pop(kEventGuid);
			rkPacket.Pop(kEventNotiClose);

			LuckyStarEventClose();

			if( kEventNotiClose.empty() )
			{
				g_kChatMgrClient.AddMessage(2340, SChatLog(CT_EVENT));
			}
			else
			{
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), kEventNotiClose);
			}
		}break;
	default:
		{
			//None Def Packet Type
		}break;
	}

	if(kCurEvent.iEventCount < 1)
	{
		kCurEvent.iEventCount = 0;
	}

	if((false==kContPopup.empty() || bEventJoin) &&
		true==lua_tinker::call<bool>("LuckyStarUI_Visible") &&
		false==lua_tinker::call<bool>("LuckyStarUI_IsOpenPopup"))
	{
		XUIMgr.Activate(L"BTN_LUCKYSTAR_ICON");
	}

	lua_tinker::call<void>("LuckyStarUI_Animation");
}

bool lwLuckyStarUI::LuckyStarResult(ELUCKYSTAR_EVENT_RESULT const iErrorType)
{
	switch( iErrorType )
	{
	case LSER_SUCCESS:
		{
			return true;
		}
	case LSER_RESULT_MIN_AGO:
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(2358).c_str(), kCurEvent.iLimitTime/(60000)); //분
			Notice_Show(std::wstring(szTemp), EL_Warning);
		}break;
	//case LSER_NOT_FOUND_EVENT:
	//case LSER_NOT_SELECT_STAR:
	//case LSER_NOT_OPENED:
	//case LSER_ALREADY_JOINED:
	//case LSER_NOT_JOINED_EVENT:
	//case LSER_NOT_FIND_USERINFO:
	//case LSER_PLAYER_FULL:
	//case LSER_HAVE_NOT_ENOUGH_MONEY:
	default:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2329 + iErrorType, true);
			g_kChatMgrClient.AddMessage(2329 + iErrorType, SChatLog(CT_EVENT));
		}break;
	}
	return false;
}


