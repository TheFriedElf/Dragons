#include "stdafx.h"
#include "PgMobileSuit.h"
#include "ClientUtil.h"
#include "NMClass/NMFiles.cpp"
#pragma comment (lib, "NMClass/nmcogame.lib")


#define    _nmman			CNMManager::GetInstance()
#define    _nmco			CNMCOClientObject::GetInstance()

class PgAutoNexon
{
public:
	PgAutoNexon()
	{
		m_bIsInitSuccess = false;
	}

	~PgAutoNexon()
	{
		_nmco.LogoutAuth();
	}

	void SetInitSuccess(bool const bIsSuc)
	{
		m_bIsInitSuccess = bIsSuc;
	}
	
	bool IsInitSuccess()const
	{
		return m_bIsInitSuccess;
	}


protected:
	bool m_bIsInitSuccess;
};

PgAutoNexon g_kNexonDummy;

#define    WM_MESSENGER_NOTIFY	( WM_USER + 100 ) 

extern void CallLoginErrorMsgBox(int const iErrorNo, std::wstring const& strMsg);

BOOL	InitializeMessenger( int uGameCode, HWND hWnd )
{
	_nmman.Init();

	if ( !_nmco.SetLocale( kLocaleID_KR ) )
	{
		return FALSE;
	}

	if ( !_nmco.Initialize( uGameCode ) )
	{
		return FALSE;
	}

	if ( !_nmco.RegisterCallbackMessage( hWnd, WM_MESSENGER_NOTIFY ) )
	{
		return FALSE;
	}

	return TRUE;
}

#define	WM_NMEVENT	( WM_USER + 17 )

namespace Util
{
	CAtlString GameCodeToString( NMGameCode uGameCode )
	{
		switch ( uGameCode )
		{
		case kGameCode_lunia_jp:		return _T("kGameCode_lunia_jp");
		case kGameCode_luniatest_jp:	return _T("kGameCode_luniatest_jp");
		case kGameCode_ca_jp:			return _T("kGameCode_ca_jp");
		case kGameCode_tenvi:			return _T("kGameCode_tenvi");
		case kGameCode_atlantica:		return _T("kGameCode_atlantica");
		default:						return _T("kGameCode_Unknown");
		}
	}
}

void NexonInit()
{
	HWND hWnd = g_pkApp->GetWindowReference();
//	ClientUtil::Log::InitLog( hWndLog );

	BOOL bResult = TRUE;

	//
	//	[필수] 메신저 매니저 초기화
	//
	_nmman.Init();

	//
	//	[선택] 로케일과 게임코드 설정 로드
	//
	NMLOCALEID	uLocaleId = kLocaleID_JP;// ClientUtil::Settings::GetLocaleId();
	NMGameCode	uGameCode = 0x01008206;//ClientUtil::Settings::GetGameCode();
	UINT32		uRegionCode =  kRegionCode_JP6;//kRegionCode_JP6;//ClientUtil::Settings::GetRegionCode();

	ClientUtil::Log::LogTime( _T("Client begins...") );
	ClientUtil::Log::LogInfo( _T("    Locale ID: %s"), ClientUtil::Convert::LocaleIdToString( uLocaleId ) );
	ClientUtil::Log::LogInfo( _T("    Region Code: %d"), uRegionCode );
	ClientUtil::Log::LogInfo( _T("    Game Code: %s"), Util::GameCodeToString( uGameCode ) );

	//
	//	[선택] 메신저 모듈 패치 옵션 설정
	//
	//		- 기본은 메신저 모듈을 패치하도록 되어 있습니다.
	//		- 메신저 모듈을 패치하지 않을 경우, 게임 클라이언트가 있는 폴더에 있는 메신저 모듈을 사용하게 됩니다.
	//
//	BOOL bPatchOption = ClientUtil::Settings::GetPatchOption();
//	if ( bPatchOption == FALSE )
//	{
//		bResult = _nmco.SetPatchOption( bPatchOption );
//		ClientUtil::Log::LogTime( _T("_nmco.SetPatchOption( %s ): %s!"), ClientUtil::Convert::BOOLToString( bPatchOption ), ClientUtil::Convert::ResultToString( bResult ) );
//	}

	//
	//	[필수] 로케일 설정
	//
	if ( bResult )
	{
		bResult = _nmco.SetLocaleAndRegion( uLocaleId, (NMREGIONCODE)uRegionCode );
		ClientUtil::Log::LogTime( _T("_nmco.SetLocaleAndRegion( %s, %s ): %s!"), ClientUtil::Convert::LocaleIdToString( uLocaleId ), ClientUtil::Convert::RegionCodeToString( (NMREGIONCODE)uRegionCode ), ClientUtil::Convert::ResultToString( bResult ) );

		g_kNexonDummy.SetInitSuccess(bResult);
	}

	//
	//	[필수] 메신저 모듈 초기화
	//
	if ( bResult )
	{
		bResult = _nmco.Initialize( uGameCode );
		ClientUtil::Log::LogTime( _T("_nmco.Initialize( %s ): %s!"), Util::GameCodeToString( uGameCode ), ClientUtil::Convert::ResultToString( bResult ) );
	}

	//
	//	서버 주소 설정 로드
	//
	ATL::CString	strMessengerServerIp( ClientUtil::Settings::GetMessengerServerIp() );
	UINT16			uMessengerServerPort( ClientUtil::Settings::GetMessengerServerPort() );
	ATL::CString	strSessionServerIp( ClientUtil::Settings::GetSessionServerIp() );
	UINT16			uSessionServerPort( ClientUtil::Settings::GetSessionServerPort() );
	ATL::CString	strAuthServerIp( ClientUtil::Settings::GetAuthServerIp() );
	UINT16			uAuthServerPort( ClientUtil::Settings::GetAuthServerPort() );

	//
	//	서버 주소 세팅이 필요한지 체크
	//
	BOOL			bSetConnConfig = ( !strMessengerServerIp.IsEmpty() && uMessengerServerPort != 0 ) ||
									 ( !strSessionServerIp.IsEmpty() && uSessionServerPort != 0 ) ||
									 ( !strAuthServerIp.IsEmpty() && uAuthServerPort != 0 );

	//
	//	[선택] 서버 주소 세팅을 바꿉니다
	//
	//		-	테스트 서버에 접속하는 경우가 아니면 이 옵션은 사용하지 마시기 바랍니다
	//
	if ( bResult && bSetConnConfig )
	{
		CNMConnConfig connConfig( strMessengerServerIp, uMessengerServerPort, strSessionServerIp, uSessionServerPort, FALSE, strAuthServerIp, uAuthServerPort );
		bResult = _nmco.SetConnConfig( connConfig );
		ClientUtil::Log::LogTime( _T("_nmco.SetConnConfig( CNMConnConfig( \"%s\", %d, \"%s\", %d, FALSE, \"%s\", %d ) ): %s!"),
			connConfig.szLoginServerIp,
			connConfig.uLoginServerPort,
			connConfig.szStatServerIp,
			connConfig.uStatServerPort,
			connConfig.szLoginServerIp,
			connConfig.uAuthServerPort,
			ClientUtil::Convert::ResultToString( bResult ) );
	}

	//
	//	[선택] 클라이언트 패치
	//
	//		-	패치가 필요한지 여부는 메신저와 상관없이 게임에서 결정합니다.
	//		-	이 예제에서는 아래 bNeedPatch 값을 수정해서 패치 여부를 결정합니다.
	//
	BOOL bNeedPatch = FALSE;
/*	if ( bNeedPatch )
	{
		//
		//	NGM 설치 여부 확인
		//
		//		- NGM 을 이용해서 게임 클라이언트를 패치하는 게임의 경우 NGM 이 설치되어 있지 않으면 패치가 불가능합니다.
		//
		if ( bResult )
		{
			if ( NMCOHelpers::IsNGMInstalled() )
			{
				ClientUtil::Log::LogTime( _T("NMCOHelpers::IsNGMInstalled(): OK!") );
				bResult = TRUE;
			}
			else
			{
				ClientUtil::Log::LogTime( _T("NMCOHelpers::IsNGMInstalled(): Failed!") );
				bResult = FALSE;
			}
		}

		//
		//	패치 실행
		//
		//		-	szPatchURL 및 szPatchDir 은 게임팀에서 수정하여 테스트 하시기 바랍니다.
		//
		if ( bResult )
		{
			TCHAR*	szPatchURL	= _T( "patch url" );
			TCHAR*	szPatchDir	= _T( "patch dir" );
			TCHAR	szCmdLine[ 2048 ];

			::_tcscpy( szCmdLine , ::GetCommandLine() );

			if ( NMCOHelpers::ExecuteNGMPatcher( uGameCode, szPatchURL, szPatchDir, szCmdLine ) )
			{
				ClientUtil::Log::LogTime( _T("NMCOHelpers::ExecuteNGMPatcher(): OK!" ) );
				ClientUtil::Log::LogInfo( _T("    GameCode: %s" ), Util::GameCodeToString( uGameCode ) );
				ClientUtil::Log::LogInfo( _T("    PatchURL: %s" ), szPatchURL );
				ClientUtil::Log::LogInfo( _T("    PatchDir: %s" ), szPatchDir );
				ClientUtil::Log::LogInfo( _T("    CmdLine:  %s" ), szCmdLine );
				//
				//	게임 클라이언트 종료
				//
				//		-	실제 게임 클라이언트는 ExecuteNGMPatcher() 호출 후 종료해야 합니다.
				//			그래야 NGM이 게임 클라이언트 파일을 패치할 수 있기 때문입니다.
				//		-	단, 이 예제에서는 편의상 프로그램을 종료하지 않고 계속 진행합니다.
				//
				bResult = TRUE;
			}
			else
			{
				ClientUtil::Log::LogTime( _T("NMCOHelpers::ExecuteNGMPatcher(): Failed!") );
				ClientUtil::Log::LogInfo( _T("    GameCode: %s" ), Util::GameCodeToString( uGameCode ) );
				ClientUtil::Log::LogInfo( _T("    PatchURL: %s" ), szPatchURL );
				ClientUtil::Log::LogInfo( _T("    PatchDir: %s" ), szPatchDir );
				ClientUtil::Log::LogInfo( _T("    CmdLine:  %s" ), szCmdLine );
				bResult = FALSE;
			}
		}
	}
*/
	//
	//	[필수] 메신저 이벤트 핸들러 등록
	//
	//		-	메신저에서 발생하는 이벤트를 받기 위해 핸들러를 등록합니다
	//
	if ( bResult )
	{
		bResult = _nmco.RegisterCallbackMessage( hWnd, WM_NMEVENT );
		ClientUtil::Log::LogTime( _T("_nmco.RegisterCallbackMessage( 0x%08x, WM_NMEVENT ): %s!"), hWnd, ClientUtil::Convert::ResultToString( bResult ) );
	}
}

void NexonLoginError( const NMLoginAuthReplyCode resultAuth )
{
	// 로그인 실패, 결과 코드에 따라 메시지를 보여줘야 함.
	std::wstring strErrorMsg;
	switch ( resultAuth )
	{
	case kLoginAuth_ServerFailed:			{strErrorMsg=TTW(18002);}break;//	로그인 에러입니다. 다시 시도해 주세요.	// SSO 서버에 접근 자체가 실패한 경우, SSO 서버 내부적인 오류	
	case kLoginAuth_ServiceShutdown:		{strErrorMsg=TTW(18057);}break;//	서비스 점검 중입니다.					//	서비스이용 일시 중단 (점검)
	case kLoginAuth_BlockedByAdmin:			{strErrorMsg=TTW(18037);}break;//	사용중지된 계정입니다					//	운영자에 의해 블럭됨
	case kLoginAuth_InvalidPassport:		{strErrorMsg=TTW(18002);}break;//	로그인 에러입니다. 다시 시도해 주세요.	//	잘못된 패스포트
	case kLoginAuth_UserNotExists:			{strErrorMsg=TTW(18011);}break;//	잘못된 ID입니다							//	존재하지 않는 유저
	case kLoginAuth_WrongPassword:			{strErrorMsg=TTW(18012);}break;//	잘못된 패스워드 입니다					//	비밀번호 불일치
	case kLoginAuth_WithdrawnUser:			{strErrorMsg=TTW(18048);}break;//	이 서버에는 접속 할 수 없는 계정입니다.	//	탈퇴한 유저
	case kLoginAuth_ModuleNotInitialized:	{strErrorMsg=TTW(18046);}break;//	시스템에서 로그인을 막고 있습니다.		//	메신저모듈이 초기화되지 않았음
	case kLoginAuth_ModuleInitializeFailed:	{strErrorMsg=TTW(18046);}break;//	시스템에서 로그인을 막고 있습니다.		//	메신저모듈 초기화 실패
	default:								{strErrorMsg=TTW(18002);}break;//	로그인 에러입니다. 다시 시도해 주세요.
	}

	XUIMgr.Close( _T("SFRM_MSG_LOCK") );
	CallLoginErrorMsgBox(0, strErrorMsg);
}

bool NexonTryLogin(std::wstring const &szNexonID, std::wstring const &szPassword, std::wstring &strzOutString)
{
	static bool bTEMP = false;
	if(bTEMP)
	{
		_nmco.LogoutAuth();
	}
	else
	{
		bTEMP = true;
	}

	if(!g_kNexonDummy.IsInitSuccess())
	{
		XUIMgr.Close( _T("SFRM_MSG_LOCK") );
		CallLoginErrorMsgBox(0, TTW(18061));
		return false;
	}

//	strzOutString = L"Passport LoginString!!!!!!!!!!!!!!!!!!!!!1";
//	return true;

	NMLoginAuthReplyCode resultAuth = _nmco.LoginAuth( szNexonID.c_str(), szPassword.c_str() );

	if ( resultAuth == kLoginAuth_OK )
	{
		TCHAR szNexonPassport[ NXPASSPORT_SIZE ];
		_nmco.GetNexonPassport( szNexonPassport );
		strzOutString = szNexonPassport;
		return true;
	}
	else
	{	
		NexonLoginError( resultAuth );
		return false;
	}
}

bool NexonAttachAuth( const std::wstring& rkCmdNexonPassport, std::wstring& rkNexPassport )
{
	NMLoginAuthReplyCode resultAuth = _nmco.AttachAuth( rkCmdNexonPassport.c_str() );
	if ( resultAuth == kLoginAuth_OK )
	{
		TCHAR szNexonPassport[ NXPASSPORT_SIZE ];
		_nmco.GetNexonPassport( szNexonPassport );
		rkNexPassport = szNexonPassport;
		return true;
	}
	else
	{
		NexonLoginError( resultAuth );
		return false;
	}
}