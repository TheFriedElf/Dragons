#include "StdAfx.h"
#include "PgAppProtect.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgAction.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "PgQuest.h"
#include "PgRemoteManager.h"
#include "PgNifMan.h"
#include "PgClientParty.h"
#include "PgRenderMan.h"
#include "lwUILogin.h"
#include "lohengrin/PgRealmManager.h"
#include "PgWebLogin.h"
#include "lohengrin/packettype.h"
#include "PgCmdlineParse.h"
#include "PgCameraModeFollow.h"
#include "PgMovieMgr.h"
#include "PgMail.h"
#include "FreedomDef.h"
#include "PgOption.h"
#include "PgHelpSystem.h"
#include "PgSelectStage.h"
#include "Variant/PgStringUtil.h"

int const LIMIT_FULL = 100;
int const LIMIT_FINE = 70;
int const LIMIT_NICE = 30;
int const CHANNEL_SLOT_MAX = 10;
float g_svOnlyTurnStartTime = 0.0f;
BM::GUID LoginUtil::g_kSelectedCharacterGUID = BM::GUID::NullData();
int const LoginUtil::LOGIN_FAIL_COUNT = 3;
int LoginUtil::g_iLoginFailExitCount = 0;

extern bool g_bDisconnectFlush;
extern bool g_bDisconnectByUser;
extern bool g_bNeedManyRealmCardUI = true;

extern void DisplayRealmUI(CONT_REALM const& kContRealm, short const nSaveRealmNo, bool bRemoveLocation=true);
extern void WstringFormat( std::wstring& rkOutMsg, size_t const iSize, const wchar_t* szMsg, ... );

bool CheckRealm(short & nRealmNo, bool const bMsg);

void LoginUtil::SendSelectCharacter(BM::GUID const &rkGuid, bool const bPassTutorial)
{
	g_kMailMgr.Init();	//Mail initialization
	g_kNetwork.SelectCharGuid(rkGuid);

	BM::Stream kPacket(PT_C_S_REQ_SELECT_CHARACTER);
	kPacket.Push(rkGuid);
	kPacket.Push(bPassTutorial);
	NETWORK_SEND_TO_SWITCH( kPacket );
}

void CALLBACK LoginUtil::SendSelectCharacterAfterMovieFinished(std::wstring kMovieID)
{
	SendSelectCharacter(g_kSelectedCharacterGUID, false);
}

void CALLBACK LoginUtil::SendSelectCharacterAfterMovieFinished_NotTutorial(std::wstring kMovieID)
{
	SendSelectCharacter(g_kSelectedCharacterGUID, true);
}

bool LoginUtil::CanShowCharacterMovie()
{
	switch (g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_SINGAPORE:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_VIETNAM:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_JAPAN:
		{
			return true;
		}
	}
	
	return false;
}


void lwOnCallOnLoginDlg( lwUIWnd kSelf )
{
	XUI::CXUI_Wnd *pkWnd = kSelf.GetSelf();
	if(pkWnd)
	{
		bool bTryLogin = false;

		if( g_kCmdLineParse.IsUseForceAccount() )
		{
			bTryLogin = true;
		}

		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_KOREA:
		case LOCAL_MGR::NC_JAPAN:
			{
				if( !g_kLocal.IsAbleServiceType( LOCAL_MGR::ST_DEVELOP ) 
				&&	g_pkApp->UseWebLinkage() )
				{
					pkWnd->Close();

					if( g_kWebLogin.IsCorrectArg() 
						&& !g_kWebLogin.IsDisconnectServer() )
					{
						bTryLogin = true;
					}
				}
			}break;
		default:
			{
			}break;
		}

		if( bTryLogin )
		{
			lwTryLogin(_T(""), _T("") );
		}
	}
}

void lwOnCallLoginDlg(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd*	pkWnd = UIParent.GetSelf();
	if ( !pkWnd )
	{
		return;
	}

	if ( true == g_kNetwork.IsAutoLogin() )
	{
		pkWnd->Close();
		if ( g_kNetwork.NowChannelNo() == g_kNetwork.TryChannelNo() )
		{
			lua_tinker::call< void, int, short >( "UI_ChannelChanging", 157, -1 );
			lua_tinker::call< void, bool>( "NoticeRestExp", true);
		}
		else
		{
			lua_tinker::call< void, int, short >( "UI_ChannelChanging", 154, g_kNetwork.TryChannelNo() );
		}
		return;
	}
	else
	{
		lua_tinker::call< void, bool>( "NoticeRestExp", true);
	}

	XUI::CXUI_CheckButton*	pkCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(_T("CBTN_ID_SAVE")));
	if(pkCBtn)
	{
		bool const bSave = (bool)g_kGlobalOption.GetValue(STR_SAVE_INFO, STR_ID_AUTO_SAVE);
		pkCBtn->Check(bSave);

		XUI::CXUI_Edit*	pkEditID = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("ID")));
		if(pkEditID)
		{
			int iFocus = 0;
			if(!bSave)
			{
				pkEditID->EditText(L"");
				pkEditID->SetEditFocus(true);
			}
			else
			{
				std::wstring wstrID = UNI(g_kGlobalOption.GetText(STR_SAVE_INFO, STR_ID_AUTO_SAVE));
				pkEditID->EditText(wstrID);
				iFocus = 1;
			}
			pkEditID->SetCustomData(&iFocus, sizeof(iFocus));
		}
		XUI::CXUI_Edit*	pkEditPW = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("PW")));
		if(pkEditPW)
		{
			int iFocus = 0;
			if(bSave)
			{
				pkEditPW->SetEditFocus(true);
				iFocus = 1;
			}
			pkEditPW->SetCustomData(&iFocus, sizeof(iFocus));
		}
	}
}

void lwLoginIDSave(bool bSave, std::wstring const& Text)
{
	std::wstring SaveText = Text;
	if( !bSave )
	{
		SaveText = L"";
	}
	if(!g_kGlobalOption.SetConfig(STR_SAVE_INFO, STR_ID_AUTO_SAVE, (int)bSave, MB(SaveText)))
	{
		//assasa
		assert(0);
	}
	g_kGlobalOption.ApplyConfig();
	g_kGlobalOption.Save();
	g_kNetwork.bSaveID(bSave);
}

extern void ClearBuffInfo();
bool lwSendSelectCharacter(lwGUID kGuid)
{
	ClearBuffInfo();
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid.GetGUID());

	bool bIsNewbiePlayer = false;

	if( pkPilot )
	{
		g_kChaLevel.SetCharacterLevel(pkPilot->GetAbil(AT_LEVEL));
		PgCameraModeFollow::SetDefaultCamera(pkPilot->GetBaseClassID());

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>( pkPilot->GetUnit() );
		if( !pkPlayer )
		{
			return false;
		}

		bIsNewbiePlayer = PgPlayerUtil::IsNewbiePlayer(pkPlayer);
		
		int const iClass = pkPlayer->GetAbil(AT_BASE_CLASS);
		if( UCLASS_SHAMAN == iClass ||UCLASS_DOUBLE_FIGHTER == iClass)
		{//For new people, check out the tutorial
			bool const bPassTutorial = true;
			if (LoginUtil::CanShowCharacterMovie())
			{
				g_kMovieMgr.SetMovieFinishedCallBack(&LoginUtil::SendSelectCharacterAfterMovieFinished_NotTutorial);
				LoginUtil::g_kSelectedCharacterGUID = kGuid();
				g_kMovieMgr.PlayOpeningMoveFromGUID(kGuid);
			}
			else
			{
				LoginUtil::SendSelectCharacter(kGuid(), bPassTutorial);
			}
			return true;
		}

		// One or more tutorials required Skipping tutorials
		bool const bCanPassTutorial = (1 <= g_kSelectStage.GetNotNewbiePlayerCount());
		if( bIsNewbiePlayer
		&&	bCanPassTutorial )
		{
			CallYesNoMsgBox(TTW(400636), kGuid(), MBT_CONFIRM_ENTERTUTORIAL);
			return true;
		}

		if ( bIsNewbiePlayer && LoginUtil::CanShowCharacterMovie() )
		{
			g_kMovieMgr.SetMovieFinishedCallBack(&LoginUtil::SendSelectCharacterAfterMovieFinished);
			LoginUtil::g_kSelectedCharacterGUID = kGuid();
			g_kMovieMgr.PlayOpeningMoveFromGUID(kGuid);
			return true;
		}
	}
	else
	{
		if(	kGuid.IsNil() )
		{
			NILOG(PGLOG_WARNING, "Can't find Selected Character Pilot[Guid: %s] Can't pass tutorial\n", kGuid.GetString());
			lwAddWarnDataTT(600014);
			return false;
		}

		// There may be no pkPilot (do not do AddScene in AutoLogin)
	}

	bool const bPassTutorial = false;
	LoginUtil::SendSelectCharacter(kGuid(), bPassTutorial);
	return true;
}

bool lwTryLogin(lwWString lwID, lwWString lwPW, bool bSave, bool bCheck)
{
	PgStringUtil::TrimAll<std::wstring>( lwID(), L" ", lwID() );

	if(lwID().size())
	{
		g_kNetwork.LoginID(lwID());
	}
	if(lwPW().size())
	{
		g_kNetwork.LoginPW(lwPW());
	}

#ifndef EXTERNAL_RELEASE
	g_pkApp->LoadServerInfo();
#endif
	TCHAR szAddr[100];
	int iPort = g_pkApp->GetServerPort();

	if( g_pkApp->UseGSM()
	&& g_kWebLogin.IsCorrectArg() )
	{
		_tcscpy(szAddr, g_kWebLogin.ServerAddr().c_str());
	}
	else if(g_kCmdLineParse.IsUseForceAccount())
	{
		_tcscpy(szAddr, g_kCmdLineParse.ServerAddr().c_str());
		if(g_kCmdLineParse.IsLoginFuncOff())
		{
		//	return false; If you solve this, you will not be able to catch the pier.
		}
	}
	else
	{
		_tcscpy(szAddr, g_pkApp->GetServerIP());
	}

	if(	g_kLocal.ServiceRegion() == LOCAL_MGR::NC_JAPAN
	&&	!g_kLocal.IsAbleServiceType(LOCAL_MGR::ST_DEVELOP) )
	{
		if( g_pkApp->UseWebLinkage() ) // Using the web launcher
		{
			std::wstring rkNexPassport;
			extern bool NexonAttachAuth( const std::wstring& rkCmdNexonPassport, std::wstring& rkNexPassport );
			bool bRet = NexonAttachAuth( g_kWebLogin.NexonPassport(), rkNexPassport );
			g_kNetwork.AddonMsg(rkNexPassport);
			if(!bRet)
			{
				return false;
			}
		}
		else
		{
			std::wstring strOutString;
			extern bool NexonTryLogin(std::wstring const &szNexonID, std::wstring const &szPassword, std::wstring &strOutString);
			bool bRet = NexonTryLogin(lwID(), lwPW(), strOutString);
			g_kNetwork.AddonMsg(strOutString);

			if(!bRet)
			{
				return false;
			}
		}
	}	

	if(g_kLocal.ServiceRegion() == LOCAL_MGR::NC_CHINA 
	|| g_kLocal.ServiceRegion() == LOCAL_MGR::NC_TAIWAN)
	{
		if(g_kCmdLineParse.ServerIP().empty() || g_kCmdLineParse.ServerIP().size() < 7)
		{
#ifdef USE_INB
			_PgMessageBox("Connect Error", "Check Server Connect information.");
			return false;
#endif
		}
		else
		{
			_tcscpy(szAddr, g_kCmdLineParse.ServerIP().c_str());
		}

		if (0 != g_kCmdLineParse.ServerPort())
		{
			iPort = g_kCmdLineParse.ServerPort();
		}
	}
#ifndef EXTERNAL_RELEASE
	BM::vstring title;
	title += TEXT(" ");
	title += szAddr;
	title += TEXT(":");
	title += iPort;
	title += TEXT(" ");
	g_pkApp->AddStringToWindowTitle(title, 2);
#endif
	/*if( false == g_pkApp->CheckLoginIP(szAddr) )
	{
		return false;
	}*/

	g_kNetwork.ServerIP(szAddr);
	g_kNetwork.ServerPort(iPort);

	bool const bRet = g_kNetwork.ConnectLoginServer(CEL::ADDR_INFO(szAddr, iPort));
	if( bRet || bSave )
	{
		lwLoginIDSave(bCheck, lwID());
	}
	return bRet;
}

bool lwClearAutoLogin()
{
	if ( true == g_kNetwork.IsAutoLogin() )
	{
		BYTE const kConnectionState = ( g_kNetwork.ConnectionState() & (~EConnect_AutoLogin) );
		g_kNetwork.ConnectionState(kConnectionState);

		if ( NS_NONE == g_kNetwork.NetState() )
		{
			g_kNetwork.ClearAuthData();	
			XUIMgr.Activate( _T("LoginDlg") );
		}
		else
		{
			NetCallChannelUI( g_kNetwork.TryRealmNo(), false );
		}

		XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( _T("FRM_CHANNEL_CHANGING") );
		if ( pkWnd )
		{
			pkWnd->ClearCustomData();
			pkWnd->Close();
		}
		return true;
	}
	return false;
}

lwWString lwGetAccountID()
{
	return static_cast<lwWString>(g_kNetwork.LoginID());
}

void lwSetDisConnectFlush()
{
	g_bDisconnectFlush = true;
	g_bDisconnectByUser = true;
}

bool lwDisconnectLoginServer()
{
	return g_kNetwork.DisConnectLoginServer() || g_kNetwork.DisConnectSwitchServer();
}

lwUILogin::lwUILogin(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

void lwUIItemDeSelect(lwUIWnd UIParent, char const* szFormName, int const iMaxForm)
{
	if( !szFormName )
	{
		return;
	}

	XUI::CXUI_Wnd* pUIParent = UIParent.GetSelf();
	for(int i = 0; i < iMaxForm; ++i)
	{
		BM::vstring	vStr(UNI(szFormName));
		vStr += i;
		XUI::CXUI_Wnd* pUIChild = pUIParent->GetControl(vStr);
		if( pUIChild && (pUIChild->UVInfo().Index == 3) )
		{
			pUIChild->UVUpdate(1);
			pUIChild->FontColor(0xFF4D2F0E);
			break;
		}
	}
}

void lwUIItemDeSelect2(lwUIWnd UIParent, char const* szFormName, int const iMaxForm)
{
	if( !szFormName )
	{
		return;
	}

	XUI::CXUI_Wnd* pUIParent = UIParent.GetSelf();
	for(int i = 0; i < iMaxForm; ++i)
	{
		BM::vstring	vStr(UNI(szFormName));
		vStr += i;
		XUI::CXUI_Wnd* pUIChild = pUIParent->GetControl(vStr);
		if( pUIChild )
		{
			pUIChild = pUIChild->GetControl(_T("FRM_CHECK_BTN"));
			if( pUIChild && (pUIChild->UVInfo().Index == 3) )
			{
				pUIChild->UVUpdate(1);
				break;
			}
		}
	}
}

void lwCloseServerTuringCard()
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_DEFAULT_CHANNEL"));
	if( !pWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pTurnCard = pWnd->GetControl(_T("FRM_SV_TURN_CARD"));
	XUI::CXUI_Wnd* pCard = pWnd->GetControl(_T("FRM_CARD"));
	if( !pTurnCard || !pCard )
	{
		return;
	}

	pTurnCard->Visible(false);
	pCard->Visible(true);
}

float lwGetTurnStartTime()
{
	return g_svOnlyTurnStartTime;
}

bool lwUILogin::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	class_<lwUILogin>(pkState, "ChannelWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "LoginToChannel", &lwUILogin::LoginToChannel)
		.def(pkState, "ReqChannelInfo", &lwUILogin::ReqChannelInfo)
		.def(pkState, "SendRealm", &lwUILogin::SendRealm)
		.def(pkState, "SendChannel", &lwUILogin::SendChannel)
		.def(pkState, "SelectItem", &lwUILogin::SelectItem)
		.def(pkState, "BackToRealmSelect", &lwUILogin::BackToRealmSelect)
	;
	
	def(pkState, "OnCallOnLoginDlg", lwOnCallOnLoginDlg);
	def(pkState, "SendSelectCharacter", lwSendSelectCharacter);
	def(pkState, "LoginIDSave", lwLoginIDSave);
	def(pkState, "OnCallLoginDlg", lwOnCallLoginDlg);
	def(pkState, "UIItemDeSelect", lwUIItemDeSelect);
	def(pkState, "UIItemDeSelect2", lwUIItemDeSelect2);
	def(pkState, "CloseServerTuringCard", lwCloseServerTuringCard);
	def(pkState, "GetTurnStartTime", lwGetTurnStartTime);
	return true;
}

void lwUILogin::SelectItem(lwUIWnd UIBtn, bool bIsRealm)
{
	if( !self || UIBtn.IsNil() )
	{
 		return;
	}

	short nData = 0;
	self->GetCustomData(&nData, sizeof(nData));
	UIBtn.ClearCustomData();
	UIBtn.SetCustomData<short>(nData);

	if( bIsRealm )
	{
		g_kNetwork.TryRealmNo(nData);
	}
	else
	{
		g_kNetwork.TryChannelNo(nData);
	}
}

void lwUILogin::SendRealm()
{
	if( self && self->Visible() )
	{
		short nRealmNo = 0;
		self->GetCustomData(&nRealmNo, sizeof(nRealmNo));
		if( !nRealmNo )
		{
			return;
		}

		//Check if you can enter the selection realm
		if(false == CheckRealm(nRealmNo, true) )
		{
			return;
		}

		// Packet Blowing place
		g_kNetwork.TryRealmNo(nRealmNo);
		BM::Stream kPacket(PT_C_L_REQ_CHANNEL_LIST, g_kNetwork.TryRealmNo());
		NETWORK_SEND_TO_LOGIN(kPacket)
	}
}

void lwUILogin::BackToRealmSelect()
{
	BM::Stream kPacket(PT_C_L_REQ_REALM_LIST);
	NETWORK_SEND_TO_LOGIN(kPacket);
}

void lwUILogin::SendChannel()
{
	if( self && self->Visible() )
	{
		short nChannelNo = 0;
		self->GetCustomData(&nChannelNo, sizeof(nChannelNo));

		CONT_CHANNEL::mapped_type	kChannelInfo;
		if( S_OK == g_kRealmMgr.GetChannel(g_kNetwork.TryRealmNo(), nChannelNo, kChannelInfo) )
		{
			if( !kChannelInfo.IsAlive() )
			{
				return;
			}
		
			// Packet Blowing place
			BM::Stream kPacket(PT_C_L_TRY_LOGIN);
			kPacket.Push(g_kNetwork.TryRealmNo());
			kPacket.Push(g_kNetwork.TryChannelNo());

			NETWORK_SEND_TO_LOGIN(kPacket);

			if( bSendRet )
			{
				RealmSaveCheck();
			}
			else
			{
				// LoginServer If the transmission fails, it is a channel change request.
				if ( g_kNetwork.NowChannelNo() == g_kNetwork.TryChannelNo() )
				{
					// Current channel.
					Notice_Show( TTW(17), EL_Warning );
				}
				else
				{
					kPacket.Push(true);
					NETWORK_SEND_TO_SWITCH(kPacket);
				}				
			}
		}
	}
}

void lwUILogin::LoginToChannel()
{//connecttoswitch
	if (NULL == self)
	{
		return;
	}

	short nRealmNo = 0;
	short nChannelNo = 0;
{//Get the values recorded in the UI.
	BM::Stream kPacket;
	kPacket.Push( int(0) );//RealmNo, ChannelNo

	self->GetCustomData(&kPacket.Data().at(0), kPacket.Data().size());
	
	kPacket.PosAdjust();

	kPacket.Pop(nRealmNo);
	kPacket.Pop(nChannelNo);
}
	g_kNetwork.TryRealmNo(nRealmNo);
	g_kNetwork.TryChannelNo(nChannelNo);
//
//	Let's send TRY_LOGIN packet here.
//	If you click it several times, you must also work to prevent it.

	BM::Stream kPacket(PT_C_L_TRY_LOGIN);
	kPacket.Push(g_kNetwork.TryRealmNo());
	kPacket.Push(g_kNetwork.TryChannelNo());
	
	NETWORK_SEND_TO_LOGIN(kPacket);
}

void NetCallChannelUI(short const nRealmNo, bool bChange)
{
	XUIMgr.Close(_T("LoginBg"));
	XUIMgr.Activate(_T("FRM_SERVER_BG"));
	XUIMgr.Activate(_T("FRM_SV_CH_SELECT_TITLE"));
	XUIMgr.Activate(_T("FRM_TRADE_CHANNEL_TIP"));
	
	CONT_REALM	kContRealm;
	g_kRealmMgr.GetRealmCont(kContRealm);
	if(/*bChange &&*/ kContRealm.size() )
	{
		switch(g_kLocal.ServiceRegion())
		{// Realm Card UI Settings
		default:
			{
				DisplayRealmUI(kContRealm, g_kNetwork.TryRealmNo(), true);
			}break;
		}
	}
	g_kNetwork.TryRealmNo(nRealmNo);

	BM::Stream kPacket(PT_C_L_REQ_CHANNEL_LIST, g_kNetwork.TryRealmNo());
	NETWORK_SEND_TO_LOGIN(kPacket);
}

void lwUILogin::ReqChannelInfo()
{
	short nRealm = 0;
	self->GetCustomData(&nRealm, sizeof(nRealm));
//	BM::Stream kPacket(PT_C_L_REQ_CHANNEL_LIST, usRealm);
//	NETWORK_SEND(kPacket)
//	SetRealmNo(usRealm);	// We are saving the selected channel number.

	NetCallChannelUI( nRealm, false );
}

// void lwUILogin::SetRealmNo(int iNo)
// {
// 	unsigned short usRealm = iNo;
// 	if(0==iNo)
// 	{
// 		self->GetCustomData(&usRealm, sizeof(usRealm));
// 	}
// 	iNo = __max(1, usRealm);
// }

void CutByRealmNameAndFlag(std::wstring const& kString, std::wstring& RealmName, bool& bIsNew)
{
	typedef	std::list<std::wstring>	CUT_STRING;

	CUT_STRING	kList;
	BM::vstring::CutTextByKey<std::wstring>(kString, L"|", kList);

	if( kList.size() )
	{
		CUT_STRING::iterator	iter = kList.begin();
		RealmName = (*iter++);
		while(iter != kList.end())
		{
			if(iter->compare(_T("N")) == 0)
			{
				bIsNew = true;
			}
			++iter;
		}
	}
}

void RealmSaveCheck()
{
	bool bIsSave = false;

	XUI::CXUI_Wnd*	pParent = XUIMgr.Get(_T("FRM_SV_CH_SELECT_TITLE"));
	if( pParent )
	{
		XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(_T("CBTN_SAVE_SERVER")));
		if( pBtn )
		{
			bIsSave = pBtn->Check();
		}
	}

	char szTemp[MAX_PATH] = {0,};
	itoa(g_kNetwork.TryRealmNo(), szTemp, 10);
	if(!g_kGlobalOption.SetConfig(STR_SAVE_INFO, STR_CONNECT_SERVER_SAVE, 1, szTemp))
	{
		//assasa
		assert(0);
	}
	g_kGlobalOption.ApplyConfig();
	g_kGlobalOption.Save();
}

void NetCallRealmUI(BM::Stream& rkPacket)
{
	//Server information came with authentication success.
	g_kNetwork.NetState(NS_AUTH_COMPLETE);
	g_kRealmMgr.Clear();
	g_kRealmMgr.ReadFromPacket(rkPacket);

	LoginUtil::g_iLoginFailExitCount = 0;

	CONT_REALM	kContRealm;
	g_kRealmMgr.GetRealmCont(kContRealm);
	if( kContRealm.size() )
	{
		if( g_kNetwork.IsAutoLogin() )
		{
			BM::Stream kPacket(PT_C_L_REQ_CHANNEL_LIST, g_kNetwork.TryRealmNo());
			NETWORK_SEND_TO_LOGIN(kPacket);
			return;
		}

		XUIMgr.Close(_T("LoginDlg"));
		XUIMgr.Close(_T("LoginBg"));
		XUIMgr.Activate(_T("FRM_SERVER_BG"));
		XUI::CXUI_Wnd* pUITitle = XUIMgr.Activate(_T("FRM_SV_CH_SELECT_TITLE"));
		XUI::CXUI_Wnd* pUITip = XUIMgr.Activate(_T("FRM_TRADE_CHANNEL_TIP"));

		short nSaveRealmNo = (short)atoi(g_kGlobalOption.GetText(STR_SAVE_INFO, STR_CONNECT_SERVER_SAVE));
		if( 0 == nSaveRealmNo )
		{
			nSaveRealmNo = g_kRealmMgr.RealmFirstNo();
		}
		if (LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion())
		{
			nSaveRealmNo = 1;
		}
		CheckRealm(nSaveRealmNo, false);

		switch(g_kLocal.ServiceRegion())
		{//Realm Card UI Settings
		default:
			{
				DisplayRealmUI(kContRealm, nSaveRealmNo, true);
			}break;
		}

		NetClearChennelUI();
		if( nSaveRealmNo != 0 )
		{
			g_kNetwork.TryRealmNo(nSaveRealmNo);
			BM::Stream kPacket(PT_C_L_REQ_CHANNEL_LIST, g_kNetwork.TryRealmNo());
			NETWORK_SEND_TO_LOGIN(kPacket);
		}
	}
}

void NetClearChennelUI()
{
	XUI::CXUI_Wnd* pUIParent = XUIMgr.Activate(_T("FRM_DEFAULT_CHANNEL"));
	if( pUIParent )
	{
		XUI::CXUI_Wnd*	pDisBtn = pUIParent->GetControl(_T("BTN_DISCONNECT"));
		XUI::CXUI_Wnd*	pExitBtn = pUIParent->GetControl(_T("BTN_EXIT"));
		XUI::CXUI_Wnd*	pConnect = pUIParent->GetControl(_T("BTN_CONNECT"));
		if( pDisBtn && pExitBtn && pConnect )
		{
			pDisBtn->Visible(true);
			pExitBtn->Visible(false);
		}

		XUI::CXUI_Wnd* pTemp = pUIParent->GetControl(_T("FRM_SV_TURN_CARD"));
		if( pTemp )
		{
			pTemp->Visible(false);
		}

		pTemp = pUIParent->GetControl(_T("FRM_CARD_BG"));
		if( pTemp )
		{
			pTemp->Visible(true);
		}

		pTemp = pUIParent->GetControl(_T("FRM_CARD"));
		if( pTemp )
		{
			pTemp->Visible(false);
		}

		for( int i = 0; i < CHANNEL_SLOT_MAX; ++i )
		{
			short iChannelNo = i + 1;

			BM::vstring	vStr(_T("FRM_CHANNEL_ITEM"));
			vStr += (iChannelNo - 1);
			XUI::CXUI_Wnd*	pChannelSlot = pUIParent->GetControl(vStr);
			if( pChannelSlot )
			{
				XUI::CXUI_AniBar* pAni = dynamic_cast<XUI::CXUI_AniBar*>(pChannelSlot->GetControl(_T("BAR_CHENNEL")));;
				XUI::CXUI_Wnd*	pText = pChannelSlot->GetControl(_T("FRM_CHANNEL_NAME"));
				XUI::CXUI_Wnd*	pText2 = pChannelSlot->GetControl(_T("FRM_CHANNEL_STATE"));
				XUI::CXUI_Wnd*	pText3 = pChannelSlot->GetControl(_T("FRM_CHANNEL_LEVEL"));
				XUI::CXUI_Wnd* pSelect = pChannelSlot->GetControl(_T("FRM_CHECK_BTN"));
				if( !pAni || !pText || !pText2 || !pText3 || !pSelect )
				{
					continue;
				}

				pChannelSlot->ClearCustomData();
				pText->Text(_T(""));
				pText2->Text(_T(""));
				pText3->Text(_T(""));
				pAni->Max(1);
				pAni->Now(0);
				pSelect->UVUpdate(4);
			}
		}
	}	
}

void NetCallChannelUI(BM::Stream &rkPacket)
{
	//Channel information came in due to successful authentication.
	g_kNetwork.NetState(NS_AUTH_COMPLETE);
	g_kRealmMgr.ReadFromPacket(rkPacket);

	PgRealm	Realm;
	CONT_CHANNEL	kContChannel;
	if( S_OK == g_kRealmMgr.GetRealm(g_kNetwork.TryRealmNo(), Realm) )
	{
		Realm.GetChannelCont(kContChannel);
	}

	if( kContChannel.size() )
	{
		if(g_kNetwork.IsAutoLogin() || lwIgnoreChannelUI())
		{
			lwIgnoreChannelUI(true,false);
			BM::Stream kPacket(PT_C_L_TRY_LOGIN);
			kPacket.Push(g_kNetwork.TryRealmNo());
			kPacket.Push(g_kNetwork.TryChannelNo());
			NETWORK_SEND_TO_LOGIN(kPacket);
			return;
		}

		bool bOnlyChannel = false;

		XUI::CXUI_Wnd*	pUIRealm = NULL;
		if(g_bNeedManyRealmCardUI)
		{
			pUIRealm = XUIMgr.Get(_T("FRM_MANY_REALM"));
		}
		else
		{
			pUIRealm = XUIMgr.Get(_T("FRM_DEFAULT_REALM"));
		}
		
		if( !pUIRealm )
		{
			bOnlyChannel = true;
		}

		g_kHelpSystem.RealmNo(g_kNetwork.TryRealmNo());
		XUI::CXUI_Wnd* pUIParent = XUIMgr.Activate(_T("FRM_DEFAULT_CHANNEL"));
		if( pUIParent )
		{
			XUI::CXUI_Wnd*	pDisBtn = pUIParent->GetControl(_T("BTN_DISCONNECT"));
			XUI::CXUI_Wnd*	pExitBtn = pUIParent->GetControl(_T("BTN_EXIT"));
			XUI::CXUI_Wnd*	pConnect = pUIParent->GetControl(_T("BTN_CONNECT"));
			if( pDisBtn && pExitBtn && pConnect )
			{
				pDisBtn->Visible(true);
				pExitBtn->Visible(false);

				if( bOnlyChannel )
				{
					POINT3I PtPos = pUIParent->Location();
					PtPos.y = 250;
					pUIParent->Location(PtPos);

					pDisBtn->Visible(false);
					pExitBtn->Visible(true);
				}
			}

			XUI::CXUI_Wnd* pUITurnCard = pUIParent->GetControl(_T("FRM_SV_TURN_CARD"));
			if( pUITurnCard )
			{
				pUITurnCard->Visible(true);
				g_svOnlyTurnStartTime = g_pkApp->GetAccumTime();
			}

			XUI::CXUI_Wnd* pUITurnBg = pUIParent->GetControl(_T("FRM_CARD_BG"));
			if( pUITurnBg )
			{
				pUITurnBg->Visible(false);
			}

			XUI::CXUI_Wnd* pUIServer = pUIParent->GetControl(_T("FRM_CARD"));
			if( pUIServer )
			{
				//SUVInfo	Info = pUIServer->UVInfo();
				//Info.Index = g_kNetwork.TryRealmNo();
				//pUIServer->UVInfo(Info);
				lua_tinker::call<void, lwUIWnd, int>("ChangeRealmImgInChannelUI", pUIServer, g_kNetwork.TryRealmNo());

				bool bTemp = false;
				std::wstring wStr;
				CutByRealmNameAndFlag(Realm.Name(), wStr, bTemp);
			//	wStr += TTW(400661);
				pUIServer->Text(wStr);
				pUIServer->Visible(false);
			}

			XUI::CXUI_Wnd * pSelect_Default = NULL;
			int iTempUserCount = INT_MAX;

			for( int i = 0; i < CHANNEL_SLOT_MAX; ++i )
			{
				short const iChannelNo = static_cast<short>(i + 1);

				BM::vstring	vStr(_T("FRM_CHANNEL_ITEM"));
				vStr += (i);				

				XUI::CXUI_Wnd*	pChannelSlot = pUIParent->GetControl(vStr);				
				if( pChannelSlot )
				{
					XUI::CXUI_AniBar* pAni = dynamic_cast<XUI::CXUI_AniBar*>(pChannelSlot->GetControl(_T("BAR_CHENNEL")));;
					XUI::CXUI_Wnd*	pText = pChannelSlot->GetControl(_T("FRM_CHANNEL_NAME"));
					XUI::CXUI_Wnd*	pText2 = pChannelSlot->GetControl(_T("FRM_CHANNEL_STATE"));
					XUI::CXUI_Wnd*	pText3 = pChannelSlot->GetControl(_T("FRM_CHANNEL_LEVEL"));
					XUI::CXUI_Wnd* pSelect = pChannelSlot->GetControl(_T("FRM_CHECK_BTN"));
					if( !pAni || !pText || !pText2 || !pText3 || !pSelect )
					{
						continue;
					}

					CONT_CHANNEL::const_iterator	channel_iter = kContChannel.find(iChannelNo);
					if( channel_iter != kContChannel.end() )
					{
						pChannelSlot->SetCustomData(&iChannelNo, sizeof(iChannelNo));

						int iPercent = static_cast<int>(std::max(0, channel_iter->second.CurrentUser()) / static_cast<float>(std::max(1, channel_iter->second.MaxUser())) * 100);
						pAni->Max(channel_iter->second.MaxUser());
						pAni->Now(channel_iter->second.CurrentUser());

						pText->Text(channel_iter->second.ChannelName());
						pText3->Text(channel_iter->second.Notice_ChnList() );

						if( channel_iter->second.IsAlive() )
						{
							pSelect->UVUpdate(1);

							if ( iChannelNo != g_kNetwork.NowChannelNo() )
							{
								if ( iTempUserCount == channel_iter->second.CurrentUser() )
								{
									if ( i == BM::Rand_Index(CHANNEL_SLOT_MAX) )
									{
										// When the number of users is the same, random is applied.UseWebLinkage() )
										++iTempUserCount;
									}
								}

								if ( channel_iter->second.CurrentUser() < iTempUserCount )
								{
									pSelect_Default = pSelect;
									iTempUserCount = channel_iter->second.CurrentUser();
									g_kNetwork.TryChannelNo(iChannelNo);
								}
							}

							{
								BM::vstring vStr;
								vStr  << iPercent << _T(" Players Online");
								pText2->Text(vStr);
								pText2->FontColor(0xFF85FF22);
							}
						}
						else
						{
							pText2->Text(TTW(400655));
							pSelect->UVUpdate(4);
						}
					}
					else
					{
						pAni->Max(0);
						pAni->Now(0);
						pText->Text(L"");						
						pText2->Text(TTW(400655));
						pText3->Text(L"");
						pSelect->UVUpdate(4);
						
						switch( g_kLocal.ServiceRegion() )
						{
						case LOCAL_MGR::NC_CHINA:
							{
								pChannelSlot->Visible(false);
							}break;
						}
					}
				}				
			}

			if( pSelect_Default )
			{ 
				pSelect_Default->UVUpdate(3);
				short nChannelNo = g_kNetwork.TryChannelNo();
				pConnect->SetCustomData(&nChannelNo, sizeof(nChannelNo));	
			}
		}
	}	 
}

int const lwGetCurrentChennelName()
{
	return static_cast<int const>(g_kNetwork.NowChannelNo());
}

void lwCannotConnectLogin()
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get(std::wstring(L"LoginDlg"));

	if(pkWnd)
	{
		pkWnd->GetControl(std::wstring(L"TRY_LOGIN"))->IsClosed(false);
		lua_tinker::call<void, char const*, bool, int>("CommonMsgBox", MB(TTW(72)), true, 0);
	}
	else
	{
		if( g_pkApp->UseWebLinkage() )
		{
			lua_tinker::call<void, char const*, bool >("QuitMsgBox_NC", MB(TTW(72)), true);//The argument is strange.
		}
		else
		{
			lua_tinker::call<void, char const*, bool, int>("CommonMsgBox", MB(TTW(72)), true, 0);
		}
	}
}

void DisplayRealmUI(CONT_REALM const& kContRealm, short const nSaveRealmNo, bool bRemoveLocation)
{
	int const iTotalRealmCnt = static_cast<int>(kContRealm.size()) -1;
	XUI::CXUI_Wnd*	pUIParent =NULL;
	{
		g_bNeedManyRealmCardUI = lua_tinker::call<bool, int>("IsNeedManyRealmUI", iTotalRealmCnt);
		if(g_bNeedManyRealmCardUI)
		{
			pUIParent = XUIMgr.Call(_T("FRM_MANY_REALM"));
		}
		else
		{
			pUIParent = XUIMgr.Call(_T("FRM_DEFAULT_REALM"));
		}
	}
	
	if( !pUIParent )
	{
		return;
	}
	
	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pUIParent->GetControl(L"BLD_SERVER_ITEM"));
	if( !pBuild )
	{
		return;
	}

	int iRealmDiplayLimit =	lua_tinker::call<int, int>("CheckRealmDisplayLimit", iTotalRealmCnt);
	if(0 == iRealmDiplayLimit)
	{// If there are no functions in Lua, set the number of basic realm cards to 6
		iRealmDiplayLimit = 6;
	}	
	
	// Determine the realm background size at FRM_DEFAULT_REALM
	if( !g_bNeedManyRealmCardUI )
	{			
		XUI::CXUI_Wnd* pCard = pUIParent->GetControl(L"FRM_SERVER_ITEM0");
		if( pCard )
		{			
			// Realm card background
			POINT2 kBgSize = pUIParent->Size();
			kBgSize.x = ( iRealmDiplayLimit * pCard->Width() );
			pUIParent->Size( kBgSize );
			POINT2 kBgLocation = pUIParent->Location();			
			kBgLocation.x = (XUIMgr.GetResolutionSize().x / 2) - ( kBgSize.x / 2 );
			pUIParent->Location( kBgLocation );
			
			// line
			XUI::CXUI_Wnd* pBg = pUIParent->GetControl(L"FRM_BACKGROUND");
			if( pBg )
			{
				XUI::CXUI_Wnd* pLine = pBg->GetControl(L"IMG_LINE1");
				if( pLine )
				{
					POINT2 kLineSize = pLine->Size();
					kLineSize.x = kBgSize.x * 0.95;
					pLine->Size( kLineSize );
					pLine->ImgSize(kLineSize);
				}
			}

			// Return button
			XUI::CXUI_Wnd* pBtn = pUIParent->GetControl(L"BTN_BACK");
			if( pBtn )
			{
				POINT2 kBtnPos = pBtn->Location();
				kBtnPos.x = ( ( kBgSize.x / 2 ) - ( pBtn->Size().x / 2 ) );
				pBtn->Location(kBtnPos);
			}
		}
	}

	int const SERVER_SLOT_MAX = pBuild->CountX() * pBuild->CountY();
	// Find the location of the first Realm card
	int const HalfParentWidth = pUIParent->Width()/2;
	int const HalfGabX = pBuild->GabX()/2;	

	int const iFirstCardPosX = HalfParentWidth - HalfGabX * iRealmDiplayLimit-7;
	int iRow = 1;
	if(0 == iTotalRealmCnt%iRealmDiplayLimit)
	{// If the number of realm card rows falls off completely
		iRow = iTotalRealmCnt/iRealmDiplayLimit;
	}
	else
	{// If the count of the number of realm card rows does not fall exactly
		iRow = iTotalRealmCnt/iRealmDiplayLimit + 1;
	}
	
	int const iShowRealmLimit = iRealmDiplayLimit*iRow;
	
	int iFirstCardPosY=0;
	for( int i = 0; i < SERVER_SLOT_MAX; ++i)
	{
		short const iRealmNo = i + 1;
		BM::vstring	vStr(_T("FRM_SERVER_ITEM"));
		vStr += i;

		XUI::CXUI_Wnd* pRealmSlot = pUIParent->GetControl(vStr);
		if( pRealmSlot )
		{
			if(0 == i)
			{
				iFirstCardPosY = pRealmSlot->Location().y;
			}
			if(iShowRealmLimit <= i)
			{
				pRealmSlot->Visible(false);
				continue;
			}
			pRealmSlot->Visible(true);
			
			XUI::CXUI_Wnd* pPrepare = pRealmSlot->GetControl(_T("FRM_PREPARE"));
			pRealmSlot->SetCustomData(&iRealmNo, sizeof(iRealmNo));
			if(bRemoveLocation)
			{// The iRealmNo value must be in pRealmSlot to set the position and picture settings
				lua_tinker::call<void, lwUIWnd, int>("OnBuild_Realm", lwUIWnd(pRealmSlot), iTotalRealmCnt);
				int const iPosX = iFirstCardPosX					
					+ (pBuild->GabX()*(i%iRealmDiplayLimit));	//+ (pBuild->GabX()*(i%pBuild->CountX()));
				int const ModifyPosY = i/pBuild->CountX();
				int const iPosY =  iFirstCardPosY				//pRealmSlot->Location().y + (pBuild->GabY()*(i/pBuild->CountX()));					
					+ (pBuild->GabY()*(i/iRealmDiplayLimit));	//+ (pBuild->GabY()*(i/iRealmDiplayLimit))- (ModifyPosY*pBuild->GabY());					
				pRealmSlot->Location(iPosX, iPosY);
			}

			CONT_REALM::const_iterator	realm_iter = kContRealm.find(iRealmNo);
			if( realm_iter != kContRealm.end() )
			{
				std::wstring wName;
				bool bNew = false;
				{
					PgRealm::EREALM_STATE const eRealmState = static_cast<PgRealm::EREALM_STATE>(g_kRealmMgr.GetRealmState(iRealmNo));

					// For service outage, service outage Contents_Config.ini Whether to follow the information (login.lua)
					bool const bUseContentsConfig = lua_tinker::call<bool>("UseContentsConfig");
					if( (eRealmState == PgRealm::EREALM_STATE_NONE)
						&& bUseContentsConfig
						)
					{// If you follow the information in Contents_Config.ini, which is the service stop status
						if( pPrepare )
						{
							pPrepare->Visible(true);
							pRealmSlot->FontColor(0xFF9FA7B4);
							pRealmSlot->OutLineColor(0xFF070303);
							pRealmSlot->UVUpdate(4);
							pRealmSlot->Text(_T(""));
							pRealmSlot->Enable(false);
						}
					}
					else 
					{// If it is in service or does not follow the information in Contents_Config.ini
						if( (eRealmState & PgRealm::EREALM_STATE_NORMAL)
							|| (!bUseContentsConfig)
							)
						{// Normal state (operational state)
							if( pPrepare )
							{
								pPrepare->Visible(false);
							}
							CutByRealmNameAndFlag(realm_iter->second.Name(), wName, bNew);	
							pRealmSlot->Text(wName);

							if(nSaveRealmNo == iRealmNo)
							{
								pRealmSlot->FontColor(0xFF790000);
								pRealmSlot->UVUpdate(3);
								XUI::CXUI_Wnd* pUIBtn = pUIParent->GetControl(_T("BTN_SELECT"));
								lwUILogin	Login(pRealmSlot);
								Login.SelectItem(pUIBtn, true);
							}
							else
							{
								pRealmSlot->FontColor(0xFF4D2F0E);
								pRealmSlot->UVUpdate(1);
							}
							pRealmSlot->OutLineColor(0xFFFFE1BA);
							pRealmSlot->Enable(true);
						}
						else
						{
							if( pPrepare )
							{
								pPrepare->Visible(true);
							}
							pRealmSlot->FontColor(0xFF9FA7B4);
							pRealmSlot->OutLineColor(0xFF070303);
							pRealmSlot->UVUpdate(4);
							pRealmSlot->Text(_T(""));
							pRealmSlot->Enable(false);
						}
						
						{
							XUI::CXUI_Wnd* pkNewMark = pRealmSlot->GetControl(_T("IMG_NEW"));
							if(pkNewMark)
							{
								if(PgRealm::EREALM_STATE_NEW & eRealmState)
								{// New Realm
									pkNewMark->Visible(true);
								}
								else
								{
									pkNewMark->Visible(false);
								}
							}
						}

						{
							XUI::CXUI_Wnd* pkEvtMark = pRealmSlot->GetControl(_T("IMG_EVENT"));
							if(pkEvtMark)
							{
								if(PgRealm::EREALM_STATE_EVENT & eRealmState)
								{// Event in progress
									pkEvtMark->Visible(true);
								}
								else
								{
									pkEvtMark->Visible(false);
								}
							}
						}
						
						{
							XUI::CXUI_Wnd* pkRecmMark = pRealmSlot->GetControl(_T("IMG_RECMD"));
							if(pkRecmMark)
							{
								if(PgRealm::EREALM_STATE_RECOMMAND & eRealmState)
								{// Featured Realms
									pkRecmMark->Visible(true);
								}
								else
								{
									pkRecmMark->Visible(false);
								}
							}
						}

					}
				}
			}
			else
			{
				if( pPrepare )
				{
					pPrepare->Visible(true);
					pRealmSlot->FontColor(0xFF9FA7B4);
					pRealmSlot->OutLineColor(0xFF070303);
				}
			}
		}
	}
}

bool CheckRealm(short & nRealmNo, bool const bMsg)
{
	bool bCheck = false;

	/*if(g_kLocal.IsServiceRegion(LOCAL_MGR::NC_SINGAPORE) && g_spPurpleClient)
	{
		switch( g_spPurpleClient->NationCode() )
		{
		case LOCAL_MGR::NC_USA:
		case LOCAL_MGR::NC_SINGAPORE:
			{
				bCheck = (1 == nRealmNo);
				nRealmNo = 1;
			}break;
		case LOCAL_MGR::NC_CHINA:
			{
				bCheck = (2 == nRealmNo);
				nRealmNo = 2;
			}break;
		}
	}
	else*/
	{
		bCheck = PgRealm::EREALM_STATE_NONE != static_cast<PgRealm::EREALM_STATE>(g_kRealmMgr.GetRealmState(nRealmNo));
	}

	if(false==bCheck && bMsg)
	{
		PgRealm	Realm;
		g_kRealmMgr.GetRealm(nRealmNo, Realm);

		std::wstring wstrText;
		WstringFormat( wstrText, MAX_PATH, TTW(182).c_str(), Realm.Name().c_str());

		lwAddWarnDataStr(lwWString(wstrText), EL_Normal);
	}
	return bCheck;
}