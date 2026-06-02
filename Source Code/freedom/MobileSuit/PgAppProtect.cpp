#include "StdAfx.h"
#include "Variant/Global.h"
#include "PgMobileSuit.h"
#include "PgMessageUtil.h"
#include "PgAppProtect.h"
#include "PgNetwork.h"
#include "BuildNumber.h"
#include "Lohengrin/BuildNumber.h"

#ifdef EXTERNAL_RELEASE
#ifdef USE_GAMEGUARD
	#pragma comment(lib, "./NProtect/NPGameLib_DE_MTDLL.lib")

	PgAntiBeamShield::PgAntiBeamShield(void)
	{
		m_pkNProtect = NULL;
		m_dwTime = 0;
	}

	PgAntiBeamShield::~PgAntiBeamShield(void)
	{
		UnInit();
	}

	void PgAntiBeamShield::UnInit()
	{
		SAFE_DELETE(m_pkNProtect);
	}

	HRESULT PgAntiBeamShield::Init(const HWND hWnd)
	{
		if(0 == g_pkApp->UseGameGuard())
		{
			return S_OK;
		}
		
		m_pkNProtect = new CNPGameLib(m_kServiceLib.c_str());

		DWORD const dwRet = m_pkNProtect->Init();

		if(dwRet != NPGAMEMON_SUCCESS)
		{
			//const wchar_t *lpszMsg = NULL;
			std::wstring kTempStr;
			switch(dwRet)
			{
			case NPGAMEMON_ERROR_EXIST:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0001", kTempStr, L"게임가드가 실행 중 입니다. 잠시 후나 재부팅 후에 다시 실행해보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_GAME_EXIST:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0002", kTempStr, L"게임이 중복 실행되었거나 게임가드가 이미 실행 중 입니다. 게임 종료 후 다시 실행해보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_INIT:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0003", kTempStr, L"게임가드 초기화 에러입니다. 재부팅 후 다시 실행해보거나 충돌할 수 있는 다른 프로그램들을 종료한 후 실행해 보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_AUTH_GAMEGUARD:
			case NPGAMEMON_ERROR_NFOUND_GG:
			case NPGAMEMON_ERROR_AUTH_INI:
			case NPGAMEMON_ERROR_NFOUND_INI:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0004", kTempStr, L"게임가드 파일이 없거나 변조되었습니다. 게임가드 셋업 파일을 설치해보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_CRYPTOAPI:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0005", kTempStr, L"윈도우의 일부 시스템 파일이 손상되었습니다. 인터넷 익스플로러(IE)를 다시 설치해보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_EXECUTE:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0006", kTempStr, L"게임가드 실행에 실패했습니다. 게임가드 셋업 파일을 다시 설치해보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_ILLEGAL_PRG:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0007", kTempStr, L"불법 프로그램이 발견되었습니다. 불필요한 프로그램을 종료한 후 다시 실행해보시기 바랍니다.");
				}break;
			case NPGMUP_ERROR_ABORT:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0008", kTempStr, L"게임가드 업데이트를 취소하셨습니다. 접속이 계속 되지 않을 경우 인터넷 및 개인 방화벽 설정을 조정해 보시기 바랍니다.");
				}break;
			case NPGMUP_ERROR_CONNECT:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0009", kTempStr, L"게임가드 업데이트 서버 접속에 실패하였습니다. 잠시 후 다시 접속하거나, 네트웍 상태를 점검해봅니다.");
				}break;
			case NPGAMEMON_ERROR_GAMEGUARD:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0010", kTempStr, L"게임가드 초기화 에러 또는 구버젼의 게임가드 파일입니다. 게임가드 셋업파일을 다시 설치하고 게임을 실행해봅니다.");
				}break;
			case NPGMUP_ERROR_PARAM:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0011", kTempStr, L"ini 파일이 없거나 변조되었습니다. 게임가드 셋업 파일을 설치하면 해결할 수 있습니다.");
				}break;
			case NPGMUP_ERROR_INIT:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0012", kTempStr, L"npgmup.des 초기화 에러입니다. 게임가드폴더를 삭제후 다시 게임실행을 해봅니다.");
				}break;
			case NPGMUP_ERROR_DOWNCFG:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0013", kTempStr, L"게임가드 업데이트 서버 접속에 실패하였습니다. 잠시 후 재시도 해보거나, 개인 방화벽이 있다면 설정을 조정해 보시기 바랍니다.");
				}break;
			case NPGMUP_ERROR_AUTH:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0014", kTempStr, L"게임가드 업데이트를 완료하지 못 했습니다. 바이러스 백신을 일시 중시 시킨 후 재시도 해보시거나, PC 관리 프로그램을 사용하시면 설정을 조정해 보시기 바랍니다.");
				}break;
			case NPGAMEMON_ERROR_NPSCAN:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0015", kTempStr, L"바이러스 및 해킹툴 검사 모듈 로딩에 실패 했습니다. 메모리 부족이거나 바이러스에 의한 감염일 수 있습니다.");
				}break;
			case NPGG_ERROR_COLLISION:
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0016", kTempStr, L"게임가드와 충돌 프로그램이 발견되었습니다.");
				}break; 
			default:
			// 적절한 종료 메시지 출력
				{
					PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0017", kTempStr, L"게임가드 실행 중 에러가 발생하였습니다. 게임 폴더 안의 GameGuard 폴더에 있는 *.erl 파일들을 Game1@inca.co.kr로 첨부하여 메일 보내주시기 바랍니다.");
				}break;
			}
			::MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
			return E_FAIL;
		}
		
		m_pkNProtect->SetHwnd(hWnd);
		return S_OK;
	}

	void PgAntiBeamShield::OnLogin(std::wstring const &wstrID)
	{
		if(0 == g_pkApp->UseGameGuard())
		{
			return;
		}
		m_pkNProtect->Send(wstrID.c_str());
	}

	bool PgAntiBeamShield::CheckProcess()
	{
		if(0 == g_pkApp->UseGameGuard())
		{
			return false;
		}

		if(BM::TimeCheck(m_dwTime, 5000))
		{
			if(m_pkNProtect->Check() != NPGAMEMON_SUCCESS)
			{//겜 종료.
				return false;
			}
		}

		return true;
	}

	BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg)
	{
		if(0 == g_pkApp->UseGameGuard())
		{
			return TRUE;
		}

		switch( g_kLocal.ServiceRegion() )
		{
			case LOCAL_MGR::NC_CHINA:
				{
					switch(dwMsg)
					{
					case NPGAMEMON_CHECK_CSAUTH2:
						{
							GG_AUTH_DATA kAuthData;
							memcpy(&kAuthData, (PVOID)dwArg, sizeof(GG_AUTH_DATA));
							// 서버로 인증 패킷을 전송. 이때 Callback 함수는 게임가드 프로세서가 호출하므로 동기화가 필수

							BM::Stream kPacket;
							if ( BM::GUID::IsNotNull( g_kAppProtect.GuidKey() ) )
							{
								kPacket.Push( PT_C_S_TRY_ACCESS_SWITCH_SECOND );
								kPacket.Push( g_kAppProtect.GuidKey() );

								g_kAppProtect.GuidKey( BM::GUID::NullData());
							}
							else
							{
								kPacket.Push( PT_C_S_ANS_GAME_GUARD_CHECK );
							}

							kPacket.Push( static_cast<unsigned long>(ERROR_SUCCESS) );
							kPacket.Push(kAuthData);
							NETWORK_SEND(kPacket)
						}break;
					case NPGAMEMON_COMM_ERROR:
					//case NPGAMEMON_COMM_CLOSE:
					case NPGAMEMON_SPEEDHACK:
					case NPGAMEMON_GAMEHACK_KILLED:
					case NPGAMEMON_GAMEHACK_DETECT:
					case NPGAMEMON_GAMEHACK_DOUBT:
					case NPGAMEMON_INIT_ERROR:
						{
							return FALSE;
						}break;
					}
				}break;
			default:
				{
					switch(dwMsg)
					{
					case NPGAMEMON_CHECK_CSAUTH2:
						{
							GG_AUTH_DATA kAuthData;
							memcpy(&kAuthData, (PVOID)dwArg, sizeof(GG_AUTH_DATA));
							// 서버로 인증 패킷을 전송. 이때 Callback 함수는 게임가드 프로세서가 호출하므로 동기화가 필수

							BM::Stream kPacket;
							if ( BM::GUID::IsNotNull( g_kAppProtect.GuidKey() ) )
							{
								kPacket.Push( PT_C_S_TRY_ACCESS_SWITCH_SECOND );
								kPacket.Push( g_kAppProtect.GuidKey() );

								g_kAppProtect.GuidKey( BM::GUID::NullData());
							}
							else
							{
								kPacket.Push( PT_C_S_ANS_GAME_GUARD_CHECK );
							}

							kPacket.Push( static_cast<unsigned long>(ERROR_SUCCESS) );
							kPacket.Push(kAuthData);
							NETWORK_SEND(kPacket)
						}break;
					case NPGAMEMON_GAMEHACK_DETECT:
						{
							//wsprintf(g_szHackMsg, TEXT(“게임핵이 발견되었습니다.\r\n%s”), npgl.GetInfo());
							BM::vstring kTempStr;
							if( g_kAppProtect.GetInfo() )
							{//해킹 정보 남김.
								kTempStr = g_kAppProtect.GetInfo();
							}
							{
								BM::Stream kPacket(PT_C_M_NFY_HACKSHIELD_CALLBACK);
								kPacket.Push( static_cast< long >(dwMsg) );
								kPacket.Push( kTempStr.operator const std::wstring &() );
								NETWORK_SEND(kPacket);
							}
							{//오류 메세지 호출
								std::wstring kMsgStr;
								//PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("InitAppProtectFailed"), kMsgStr, _T("AppProtect luanch failed"));
								PgMessageUtil::GetFromMessageDotIni(L"GUARD", L"GG0007", kMsgStr, L"불법 프로그램이 발견되었습니다. 불필요한 프로그램을 종료한 후 다시 실행해보시기 바랍니다.");
								BM::vstring kCaptionStr(kMsgStr);
								kCaptionStr += L" : ";
								kCaptionStr += dwMsg;
								MessageBox(NULL, kCaptionStr, GetClientNameW(), MB_OK);
							}
							{
								g_bWM_CLOSE = true;
								if(g_pkApp && g_pkApp->GetAppWindow())
								{
									::PostMessage(g_pkApp->GetAppWindow()->GetWindowReference(), WM_CLOSE, 0, 0);
								}
								else
								{
									::PostQuitMessage(0);
								}
							}
						}break;
			/*		case NPGAMEMON_COMM_ERROR:
					case NPGAMEMON_COMM_CLOSE:
						bAppExit = true; // 종료 코드
						return false;
					case NPGAMEMON_INIT_ERROR:
						wsprintf(g_szHackMsg, TEXT("게임가드 초기화 에러 : %lu"), dwArg);
						bAppExit = true; // 종료 코드
						return false;
					case NPGAMEMON_SPEEDHACK:
						wsprintf(g_szHackMsg, TEXT(“스피드핵이 감지되었습니다.”));
						bAppExit = true; // 종료 코드
						return false;
					case NPGAMEMON_GAMEHACK_KILLED:
				//		wsprintf(g_szHackMsg, TEXT( “게임핵이 발견되었습니다.\r\n%s”), npgl.GetInfo());
				//		bAppExit = true; // 종료 코드
				//		return false;
					case NPGAMEMON_GAMEHACK_DOUBT:
						wsprintf(g_szHackMsg, TEXT(“게임이나 게임가드가 변조되었습니다.”));
						bAppExit = true; // 종료 코드
						return false;
					case NPGAMEMON_GAMEHACK_REPORT:
						{
							DWORD dwHackInfoSize = 0;
							LPBYTE pHackInfo = NULL;
							pHackInfo = GetHackInfoFromGameMon(&dwHackInfoSize);
							// pHackInfo = npgl.GetHackInfo(&dwHackInfoSize); // C++ 일 경우.
							if (pHackInfo && dwHackInfoSize > 0)
							{
								// 아래 함수는 게임가드에서 제공하는 함수가 아닙니다. 
								SendToHackLog(pHackInfo, dwHackInfoSize); // 서버로 데이터 전송.
							}
						}
						*/
					}
				}break;
		}

		return TRUE; // 계속 진행
	}

	char const* PgAntiBeamShield::GetInfo()const
	{
		if( m_pkNProtect )
		{
			return m_pkNProtect->GetInfo();
		}
		return NULL;
	}
	
	void PgAntiBeamShield::CheckState( BM::Stream &rkPacket, BM::GUID const &guidSwitchKey )
	{
		if(0 == g_pkApp->UseGameGuard())
		{
			return;
		}

		m_guidKey = guidSwitchKey;

		GG_AUTH_DATA kAuthData;
		rkPacket.Pop(kAuthData);
		DWORD const dwRet = m_pkNProtect->Auth2(&kAuthData);
	}
#endif //#ifdef USE_GAMEGUARD

#ifdef USE_HACKSHIELD

#ifdef HACKSHIELD_UNAUTO	//오토 미허용(기본)
	#pragma comment(lib, "./AhnLabHackShield_UNAUTO/HShield_MD.lib")
	#pragma comment(lib, "./AhnLabHackShield_UNAUTO/HSUpChk.lib")
	#pragma comment(lib, "./AhnLabHackShield_UNAUTO/HsUserUtil.lib")
#endif

#ifdef HACKSHIELD_AUTO	//오토 허용(중국/일본)
	#pragma comment(lib, "./AhnLabHackShield_AUTO/HShield_MD.lib")
	#pragma comment(lib, "./AhnLabHackShield_AUTO/HSUpChk.lib")
	#pragma comment(lib, "./AhnLabHackShield_AUTO/HsUserUtil.lib")
#endif

	DWORD PgAntiBeamShield::m_dwThreadID = 0;

	PgAntiBeamShield::PgAntiBeamShield(void)
	{
		m_dwTime = 0;
	}

	PgAntiBeamShield::~PgAntiBeamShield(void)
	{
		UnInit();
	}

	void PgAntiBeamShield::UnInit()
	{
		{
			int iRet = _AhnHS_StopService();
			if( HS_ERR_OK != iRet )
			{
				//
			}
		}
		{
			int iRet = _AhnHS_Uninitialize();
			if( HS_ERR_OK != iRet )
			{
				//
			}
		}
	}

	HRESULT PgAntiBeamShield::Init(const HWND hWnd)
	{
		// g_pkApp->UseGameGuard()
		//		0, Disable All
		//		1, All
		//		2, Disable UserMode
		//		3, DIaable UserMode, No Update
		//		4, For Debug (No User, No Update)

		if(0 == g_pkApp->UseGameGuard())
		{
			return S_OK;
		}

		TCHAR	*pEnd = NULL;
		TCHAR	szFullFileName[MAX_PATH] = {0, };

		if( 0 == GetModuleFileName(NULL, szFullFileName, MAX_PATH) )
		{
			return E_FAIL;
		}

		pEnd = _tcsrchr(szFullFileName, _T('\\')) + 1;
		if( !pEnd )
		{
			return E_FAIL;
		}
		*pEnd = _T('\0');

		TCHAR	szHShieldPath[MAX_PATH] = {0, };
		TCHAR	szIniPath[MAX_PATH] = {0, };

		_stprintf(szIniPath, _T("%s"), szFullFileName);
		_stprintf(szHShieldPath, _T("%s\\HShield"), szFullFileName);	// HShield-Path 세팅
		_tcscat(szFullFileName, _T("HShield\\EhSvc.dll"));				// EHSVC.DLL =Path 세팅.

		// 핵실드 모니터링 서버 사용
		// 아이디나 모니터링 주소, 게임버전등은 암호화나 인코딩 하여 보관하였다가 실제 아래 함수에 입력시
		// 복호화나 디코딩 하여 입력하는 것이 좋다.
		AHNHS_EXT_ERRORINFO HsExtError = {0, };
		//ZeroMemory( HsExtError, sizeof(HsExtError) );
		bool bUseServerMoniterring = false;
		std::wstring kSerial(L""); // Default Nothing
		int iGameCode = 0;
		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_CHINA:
			{
				//swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"아이피 받아서 넣을 것" );		// 모니터링 서버주소
				//bUseServerMoniterring = false;
				//kSerial = L"A53B3DC3290E8B9D00B5F97C";
				//iGameCode = 5661;
			}break;
		case LOCAL_MGR::NC_SINGAPORE:
			{
				swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"122.50.2.25" );		// 모니터링 서버주소
				bUseServerMoniterring = true;
				kSerial = L"0B69589C138FC2767F5A3244";
				iGameCode = 5220;
			}break;
		case LOCAL_MGR::NC_USA:
			{
				//swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"아이피 받아서 넣을 것" );		// 모니터링 서버주소
				//bUseServerMoniterring = false;
			}break;
		case LOCAL_MGR::NC_THAILAND:
			{
				//swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"122.50.1.151" );		// 모니터링 서버주소
				//bUseServerMoniterring = false;
				kSerial = L"62E3F99B06EA59222C967809";
				iGameCode = 5230;
			}break;
		case LOCAL_MGR::NC_INDONESIA:
			{
				//swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"122.50.1.151" );		// 모니터링 서버주소
				//bUseServerMoniterring = false;
				kSerial = L"EBD2FDBDB4DBC06B54BB219F";
				iGameCode = 5235;
			}break;
		case LOCAL_MGR::NC_JAPAN:
			{
				//swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"아이피 받아서 넣을 것" );		// 모니터링 서버주소
				//bUseServerMoniterring = false;
				kSerial = L"C2B60B2951C63C64FC563981";
				iGameCode = 5210;
			}break;
		case LOCAL_MGR::NC_KOREA:
			{
				swprintf_s(  HsExtError.szServer, MAX_PATH, L"%s", L"112.175.133.5:7999" );		// 모니터링 서버주소
				bUseServerMoniterring = true;
				kSerial = L"5F4D9B75ACD5193DF421F6B1";
				iGameCode = 6111;
			}break;
		case LOCAL_MGR::NC_PHILIPPINES:
			{
				kSerial = L"0F7D5D5B8087D84FFD3E2FCA";
				iGameCode = 5236;
			}break;
		case LOCAL_MGR::NC_VIETNAM:
			{
				kSerial = L"24BE37ED437512918FFF54DF";
				iGameCode = 5232;
			}break;
		case LOCAL_MGR::NC_BRAZIL:
			{//브라질
				kSerial = L"772DC30BC21496879A654F0C";
				iGameCode = 5219;
			}break;
		}
		//sprintf( HsExtError.szUserId, "%s", "GameUser" );				// 유저 아이디
		// _AhnHS_Initialize시점에 아이디를 얻을 수 없는 경우에는 널 스트링을 입력한다.
		// 아이디를 얻는 시점에 _AhnHS_SetUserId를 호출하여 아이디를 전송한다.
		//sprintf( (char*)HsExtError.szUserId, "%s", "" ); 
		if( true == bUseServerMoniterring )
		{
			wnsprintf( HsExtError.szGameVersion, MAX_PATH, L"% (%s) %d.%d.%d", VERSION_PRODUCTVERSION_STR, __DATE__, g_pkApp->PatchVer().Version.i16Major, g_pkApp->PatchVer().Version.i16Minor, g_pkApp->PatchVer().Version.i32Tiny);		// 게임 버전
			int iRet = _AhnHS_StartMonitor(HsExtError, szFullFileName);
			if( HS_ERR_OK != iRet )
			{
				return E_FAIL;	
			}
		}

		// 비관리자 계정 핵실드 실행 연동
		bool bUseUserMode = true;
#ifndef USB_INB
		bUseUserMode = 2 > g_pkApp->UseGameGuard();
#endif
		if( bUseUserMode )
		{
			if( true == _AhnHSUserUtil_IsAdmin() )
			{
				if( false == _AhnHsUserUtil_IsEnableHSAdminRights() )
				{
					_AhnHsUserUtil_CreateUser();
#if defined(HACKSHIELD_AUTO)
					char szTempPath[MAX_PATH] = {0, };
					strcat_s(szTempPath, MB(szHShieldPath));
					_AhnHsUserUtil_SetFolderPermission(szTempPath);
#endif
#if defined(HACKSHIELD_UNAUTO)
					_AhnHsUserUtil_SetFolderPermission( szHShieldPath );
#endif
				}
			}
		}
		// 핵실드 업데이트
#ifndef USB_INB
		if( 3 > g_pkApp->UseGameGuard() )
#endif
		{

			DWORD dwResult = 0x00000000;
#if defined(HACKSHIELD_AUTO)
			dwResult = _AhnHS_HSUpdate(szHShieldPath, 1000 * 600);
#endif
#if defined(HACKSHIELD_UNAUTO)
			dwResult = _AhnHS_HSUpdateEx(szHShieldPath, 1000 * 600, iGameCode, AHNHSUPDATE_CHKOPT_GAMECODE, HsExtError);
#endif

			std::wstring kTempStr;
			LPTSTR szErrorStr = _T("");
			switch(dwResult)
			{
			case 0x00000000:
				{
					// 성공
				}break;
			case 0x30000010:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP001", kTempStr, L"업데이트 실패: HSUpdate.env 파일을 읽을 수 없습니다.");
				}break;
			case 0x30000020:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP002", kTempStr, L"업데이트 실패: HSUpdate.env 파일을 쓸 수 없습니다.");
				}break;
			case 0x30000030:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP003", kTempStr, L"업데이트 실패: 업데이트 서버에 연결할 수 없습니다.");
				}break;
			case 0x30000050:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP004", kTempStr, L"업데이트 실패: 네트워크 결과 입력 중에 오류가 발생하였습니다.");
				}break;
			case 0x30000060:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP005", kTempStr, L"업데이트 실패: 업데이트 프로그램 관련 파일을 찾을 수 없습니다.");
				}break;
			case 0x30000070:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP006", kTempStr, L"업데이트 실패: HSUpdate.pt 인증 파일을 찾을 수 없습니다.");
				}break;
			case 0x30000080:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP007", kTempStr, L"업데이트 실패: 업데이트 파일이 잘못 되었습니다.");
				}break;
			case 0x30000090:
				{
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP008", kTempStr, L"업데이트 실패: 업데이트 전송 시간을 초과하였습니다.");
				}break;
			default :
				{
					wchar_t szTempStr[MAX_PATH] = {0, };
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HUP009", kTempStr, L"업데이트 실패: 정의되지 않은 오류가 발생 하였습니다. (Ret=%x).");
					_stprintf(szTempStr, kTempStr.c_str(), dwResult);
					kTempStr = szTempStr;
				}break;
			}
			if( HACKSHIELD_ERROR_SUCCESS != dwResult )
			{
				MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
				return E_FAIL;
			}
			//TRACE("%s[%x] = AhnHS_HSUpdate(%s, %d)", szErrorStr, dwResult, szHShieldPath, 0);
		}

		{
#ifdef USB_INB
				DWORD const dwOption =	AHNHS_CHKOPT_ALL
#else
				// 디버그 가능
				DWORD const dwCanDebug = (AHNHS_CHKOPT_SPEEDHACK |AHNHS_CHKOPT_AUTOMOUSE |AHNHS_CHKOPT_PROCESSSCAN |AHNHS_USE_LOG_FILE);
				DWORD const dwOption =	((5 == g_pkApp->UseGameGuard())? dwCanDebug: AHNHS_CHKOPT_ALL)
#endif
										| AHNHS_ALLOW_SVCHOST_OPENPROCESS
										| AHNHS_ALLOW_LSASS_OPENPROCESS
										| AHNHS_ALLOW_CSRSS_OPENPROCESS
										| AHNHS_DONOT_TERMINATE_PROCESS
										| AHNHS_CHKOPT_LOCAL_MEMORY_PROTECTION
										| AHNHS_CHKOPT_SELF_DESTRUCTION
										//| AHNHS_CHKOPT_ANTIFREESERVER
										| AHNHS_CHKOPT_STANDALONE
										| AHNHS_ALLOW_SWITCH_WINDOW
										| AHNHS_CHKOPT_UPDATED_FILE_CHECK;

			int iRet = _AhnHS_Initialize(szFullFileName, _AhnHS_CallBack,
				iGameCode, kSerial.c_str(),
				dwOption,
				AHNHS_SPEEDHACK_SENSING_RATIO_NORMAL);
			assert(nRet != HS_ERR_INVALID_PARAM);
			assert(nRet != HS_ERR_INVALID_LICENSE);
			assert(nRet != HS_ERR_ALREADY_INITIALIZED);

			if( HS_ERR_OK != iRet )
			{
				std::wstring kTempStr;
				switch( iRet )
				{
				case HS_ERR_ANOTHER_SERVICE_RUNNING:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS001", kTempStr, L"다른 게임이 실행중입니다.\n프로그램을 종료합니다.");
					}break;
				case HS_ERR_INVALID_FILES:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS002", kTempStr, L"잘못된 파일 설치되었습니다.\n프로그램을 재설치하시기 바랍니다.");
					}break;
				case HS_ERR_DEBUGGER_DETECT:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS003", kTempStr, L"컴퓨터에서 디버거 실행이 감지되었습니다.\n디버거의 실행을 중지시킨 뒤에 다시 실행시켜주시기바랍니다.");
					}break;
				case HS_ERR_NEED_ADMIN_RIGHTS:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS004", kTempStr, L"Admin 권한으로 실행되어야 합니다.\n프로그램을 종료합니다.");
					}break;
				case HS_ERR_COMPATIBILITY_MODE_RUNNING:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS005", kTempStr, L"호환성 모드로 프로그램이 실행중입니다.\n프로그램을 종료합니다.");
					}break;
				case HS_ERR_ALREADY_GAME_STARTED:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS007", kTempStr, L"게임이 이미 실행중입니다.\n프로그램을 종료합니다.");
					}break;
				default:
					{
						PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS006", kTempStr, L"해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다.");
						wchar_t szMsg[MAX_PATH] = {0, };
						_stprintf(szMsg, kTempStr.c_str(), iRet);
						kTempStr = szMsg;
					}break;
				}

				if( !kTempStr.empty() )
				{
					MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
				}
				return E_FAIL;
			}
		}

		{
			int iRet = _AhnHS_StartService();
			assert(nRet != HS_ERR_NOT_INITIALIZED);
			assert(nRet != HS_ERR_ALREADY_SERVICE_RUNNING);
			if( HS_ERR_OK != iRet )
			{
				return E_FAIL;
			}
		}
		{	// DLL 모듈 LMP 보호
			#define HS_USER_ERR_BASE			0xF0000000
			std::wstring const aryDllFile[] = { std::wstring(L".\\NxCharacter.dll")
											,	std::wstring(L".\\NxCooking.dll")
											,	std::wstring(L".\\NxExtensions.dll")
											,	std::wstring(L".\\PhysXCore.dll")
											,	std::wstring(L".\\PhysXLoader.dll") };
			size_t iCount = 0;
			std::wstring const* pkBegin = aryDllFile;
			while( PgArrayUtil::IsInArray(pkBegin, aryDllFile) )
			{
				++iCount;
				if( TRUE != _AhnHS_IsModuleSecureW( (*pkBegin).c_str() ) )
				{
					std::wstring kTempStr;
					PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HS006", kTempStr, L"해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다.");
					wchar_t szMsg[MAX_PATH] = {0, };
					_stprintf(szMsg, kTempStr.c_str(), HS_USER_ERR_BASE+iCount);
					kTempStr = szMsg;
					MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
					return E_FAIL;
				}
				++pkBegin;
			}
		}

		m_dwThreadID = GetCurrentThreadId();

		return S_OK;
	}

	void PgAntiBeamShield::OnLogin(std::wstring const &wstrID)
	{
		//핵실드 모니터링 서버 사용시
		_AhnHS_SetUserId(wstrID.c_str());
	}

	bool PgAntiBeamShield::CheckProcess()
	{
		return true;
	}

	void PgAntiBeamShield::CheckState( BM::Stream &rkPacket, BM::GUID const &guidSwitchKey )
	{
		AHNHS_TRANS_BUFFER kRecvBuffer;
		AHNHS_TRANS_BUFFER kResponBuffer;
		::memset( &kResponBuffer, 0, sizeof(AHNHS_TRANS_BUFFER) );

		rkPacket.Pop( kRecvBuffer.nLength );
		rkPacket.PopMemory( kRecvBuffer.byBuffer, static_cast<size_t>(kRecvBuffer.nLength) );

		unsigned long const ulRet = ::_AhnHS_MakeResponse( kRecvBuffer.byBuffer, static_cast<unsigned long>(kRecvBuffer.nLength), &kResponBuffer );

		BM::Stream kAnsPacket;
		if ( BM::GUID::IsNotNull( guidSwitchKey ) )
		{
			kAnsPacket.Push( PT_C_S_TRY_ACCESS_SWITCH_SECOND );
			kAnsPacket.Push( guidSwitchKey );
		}
		else
		{
			kAnsPacket.Push( PT_C_S_ANS_GAME_GUARD_CHECK );
		}

		kAnsPacket.Push( ulRet );
		kAnsPacket.Push( kResponBuffer.nLength );
		kAnsPacket.Push( kResponBuffer.byBuffer, static_cast<size_t>(kResponBuffer.nLength) );
		NETWORK_SEND( kAnsPacket );
	}

	int __stdcall _AhnHS_CallBack(long lCode, long lParamSize, void* pParam)
	{
		TCHAR szMsg[MAX_PATH] = {0, };
		std::wstring kTempStr;
		bool bForceTerminate = false;

		switch(lCode)
		{
			//Engine Callback
		case AHNHS_ENGINE_DETECT_GAME_HACK:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS001", kTempStr, L"다음 위치에서 해킹툴이 발견되어 프로그램을 종료시켰습니다.\n%s");
				_stprintf(szMsg, kTempStr.c_str(), UNI((char*)pParam));
				kTempStr = szMsg;
				bForceTerminate = true;
			}break;
			//창모드 해킹툴 감지
		case AHNHS_ENGINE_DETECT_WINDOWED_HACK:
			{
				//_stprintf(szMsg, _T("프로그램에 대하여 창모드 해킹툴이 감지되었습니다."));
				//bForceTerminate = true;
				return 1;
			}break;
			//Speed 관련
		case AHNHS_ACTAPC_DETECT_SPEEDHACK:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS002", kTempStr, L"현재 이 PC에서 SpeedHack으로 의심되는 동작이 감지되었습니다.");
				bForceTerminate = true;
			}break;

			//디버깅 방지
		case AHNHS_ACTAPC_DETECT_KDTRACE:	
		case AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS003", kTempStr, L"프로그램에 대하여 디버깅 시도가 발생하였습니다. (Code = %x)\n프로그램을 종료합니다.");
				_stprintf(szMsg, kTempStr.c_str(), lCode);
				kTempStr = szMsg;
				bForceTerminate = true;
			}break;

		case AHNHS_ACTAPC_DETECT_AUTOMACRO:
			{
				/*PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS004", kTempStr, L"매크로 기능으로 의심되는 동작이 감지되었습니다.. (Code = %x)\n프로그램을 종료합니다.");
				_stprintf(szMsg, kTempStr.c_str(), lCode);
				kTempStr = szMsg;
				bForceTerminate = true;*/
				return 1;
			}break;

			// 코드 패치 감지
		case AHNHS_ACTAPC_DETECT_ABNORMAL_MEMORY_ACCESS:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS005", kTempStr, L"다음 위치에서 메모리 접근이 감지되어 프로그램을 종료시켰습니다.\n%s");
				_stprintf(szMsg, kTempStr.c_str(), UNI((char*)pParam));
				kTempStr = szMsg;
				bForceTerminate = true;
			}break;
		case AHNHS_ACTAPC_DETECT_MEM_MODIFY_FROM_LMP:
		case AHNHS_ACTAPC_DETECT_LMP_FAILED:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS006", kTempStr, L"모듈에서 메모리 변조가 감지되었습니다.");
				_stprintf(szMsg, kTempStr.c_str());
				kTempStr = szMsg;
				bForceTerminate = true;
			}break;

			//그외 해킹 방지 기능 이상 
		case AHNHS_ACTAPC_DETECT_AUTOMOUSE:
		case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
		case AHNHS_ACTAPC_DETECT_HOOKFUNCTION:
		case AHNHS_ACTAPC_DETECT_MESSAGEHOOK:
		case AHNHS_ACTAPC_DETECT_MODULE_CHANGE:
		case AHNHS_ACTAPC_DETECT_ENGINEFAILED:
		case AHNHS_ACTAPC_DETECT_CODEMISMATCH:
		case AHNHS_ACTAPC_DETECT_PROTECTSCREENFAILED:
		case AHNHS_ACTAPC_DETECT_ABNORMAL_HACKSHIELD_STATUS:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS007", kTempStr, L"해킹 방어 기능에 이상이 발생하였습니다. (Code = %x)\n프로그램을 종료합니다.");
				_stprintf(szMsg, kTempStr.c_str(), lCode);
				kTempStr = szMsg;
				bForceTerminate = true;
			}break;
		default:
			{
				PgMessageUtil::GetFromMessageDotIni(L"SHIELD", L"HSS008", kTempStr, L"정의 되지 않은 해킹 종류가 감지 되었습니다. (Code = %x)");
				_stprintf(szMsg, kTempStr.c_str(), lCode);
				kTempStr = szMsg;
			}break;
		}

		{
			BM::Stream kPacket(PT_C_M_NFY_HACKSHIELD_CALLBACK);
			kPacket.Push( lCode );
			kPacket.Push( kTempStr );
			NETWORK_SEND(kPacket);
		}

		if( bForceTerminate )
		{
			PostThreadMessage(PgAntiBeamShield::m_dwThreadID, WM_QUIT, 0, 0);
		}

		if( !kTempStr.empty() )
		{
			MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
		}
		return 1;
	}
#endif //#ifdef USE_HACKSHIELD
#endif //#ifdef EXTERNAL_RELEASE
	
	void CALLBACK _Antihack_UnhandledExceptionHandler(void)
	{
		g_kAppProtect.UnInit();
	}

#if (!defined(USE_GAMEGUARD) && !defined(USE_HACKSHIELD)) || !defined(EXTERNAL_RELEASE)
	// 더미 AntiHack 연동

	PgAntiBeamShield::PgAntiBeamShield(void)
	{
		m_dwTime = 0;
	}

	PgAntiBeamShield::~PgAntiBeamShield(void)
	{
	}

	void PgAntiBeamShield::UnInit()
	{
	}

	HRESULT PgAntiBeamShield::Init(const HWND hWnd)
	{
		return S_OK;
	}

	void PgAntiBeamShield::OnLogin(std::wstring const &wstrID)
	{
	}

	bool PgAntiBeamShield::CheckProcess()
	{
		return true;
	}

	void PgAntiBeamShield::CheckState( BM::Stream &rkPacket, BM::GUID const &guidSwitchKey )
	{
		unsigned long const ulRet = 0;

		BM::Stream kAnsPacket;
		if ( BM::GUID::IsNotNull( guidSwitchKey ) )
		{
			kAnsPacket.Push( PT_C_S_TRY_ACCESS_SWITCH_SECOND );
			kAnsPacket.Push( guidSwitchKey );
		}
		else
		{
			kAnsPacket.Push( PT_C_S_ANS_GAME_GUARD_CHECK );
		}

		kAnsPacket.Push( ulRet );
		NETWORK_SEND( kAnsPacket );
	}
#endif