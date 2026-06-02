#include "stdafx.h"
#include "NiApplication/NiDX9Select.h"
#include <NiApplicationMetrics.h>
#include <NiDx9RendererMetrics.h>
#include <NiMetricsOutput.h>
#include <shlobj.h>
#include <ShellAPI.h>

#include "Lohengrin/BuildNumber.h"
#include "BuildNumber.h"
#include "PgMobileSuit.h"
#include "PgMessageUtil.h"
#include "ShaderHelper.h"
#include "PgRenderMan.h" 
#include "PgScripting.h"
#include "PgNetwork.h"  
#include "PgWorld.h"    
#include "PgActor.h" 
#include "PgActorNpc.h"
#include "PgLocalManager.h"
#include "PgRemoteManager.h" 
#include "ServerLib.h"
#include "PgParticleMan.h"     
#include "PgParticle.h"   
#include "PgXmlLoader.h"
#include "PgSoundMan.h"     
#include "PgPilotMan.h"   
#include "PgUIScene.h" 
#include "PgAMPool.h"
#include "PgNifMan.h"
#include "Pg2DString.h"
#include "PgUISound.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuest.h"
#include "PgQuestMan.h"
#include "PgActionPool.h"
#include "PgDynamicTextureMan.h"
#include "PgProjectileMan.h"
#include "PgSkillTree.h"
#include "PgWeaponSoundMan.h"
#include "PgWorkerThread.h"
#include "PgTrail.H"
#include "PgOption.h"
#include "PgChatMgrClient.h"
#include "PgNiFile.h"
#include "PgStatusEffect.h"
#include "BM/filesupport.h"
#include "BM/PgFilterString.h"
#include "PgStat.h"
#include "PgResourceMonitor.h"
#include "PgScreenUpdateThread.h"
#include "PgItemMan.h"
#include "PgNiEntityStreamingAscii.h"
#include "PgAppProtect.h"
#include "resource.h"
#include "PgComboAdvisor.H"
#include "PgMissionComplete.h"
#include "PgErrorCodeTable.h"
#include "lwUI.h"
#include "PgLinkedPlane.H"
#include "PgEventTimer.h"
#include "PgMovieMgr.h"
#include "BugTrap.h"
#include "PgHeadBuffIcon.H"
#include "PgRscLoader.h"
#include "Utility/ComputerInfo.h"
#include "Utility/ErrorReportFile.h"
#include "Utility/ExceptionHandler.h"
#include "FreedomPool.h"
#include "PgPartyBalloon.h"
#include "PgExpeditionBalloon.h"
#include "PgTextBalloon.h"
#include "PgMarkBalloon.h"
#include "PgDuelTitle.h"
#include "PgInputSystem.H"
#include "PgWorldMapUI.h"
#include "PgActorManager.H"
#include "PgChatWindow.H"
#include "PgSelectStage.h"
#include "SimpEnc/SimpEnc.h"
#include "PgHelpSystem.H"
#include "PgCmdlineParse.h"
#include "Ranking/PgRankingMgr.h"
#include "BrightBloom.H"
#include "PgPostProcessMan.H"
#include "PgControllerSequenceManager.H"
#include "PgWorldObjectPool.H"
#include "PgGuildMark.h"
#include "PgGuardianMark.h"
#include "PgCashShop.h"
#include "DataPack/PgDataPackManager.h"
#include "PgBriefingLoadUIMgr.h"
#include "PgAchieveNfyMgr.h"
#include "lwUIShineStone.h"
#include "PgBalloonEmoticonMgr.h"
#include "lwUIBook.h"
#include "PgZoneDrawing.h"
#include "PgHomeTown.h"
#include "Variant/PgBattleSquare.h"
#include "PgBattleSquare.h"
#include "PgSkillOptionMan.H"
#include "PgEnvironmentMan.H"
#include "PgEnvElementFactory.H"
#include "lwPlayTime.h"
#include "PgFxStudio.H"
#include "PgQuestMarkHelp.h"
#include "PgMinimap.h"
#include "PgScreenBreak.h"
#include "PgChainAttack.h"
#include "PgSkillTargetMan.h"
#include "PgHomeRenew.h"
#include "PgWebLogin.h"
#include "PgElementMark.h"
#include "PgActionXmlData.h"
#include "PgConstellation.h"
#include "Variant/Global.h"

#include "NewWare/Renderer/DrawWorkflow.h"


//#undef EXTERNAL_RELEASE
//
//#ifndef EXTERNAL_RELEASE
#undef PG_TRY_BLOCK
#undef PG_CATCH_BLOCK
#undef PG_CATCH_BLOCK_NO_EXIT
#define PG_TRY_BLOCK
#define PG_CATCH_BLOCK
#define PG_CATCH_BLOCK_NO_EXIT
//#endif

#define PG_MIN_FRAME_RATE	1.0f / 10.0f
#define MAX_SITE_NUM	50

int g_iUseBugTrap = 0;
TCHAR g_szDumpAddr[256];
int g_nDumpPort = 9999;
TCHAR g_szDumpAppName[256];
int g_iBugTrapScreenShot = 0;
Loki::Mutex g_kSoundInitMutex;
PgMobileSuit *g_pkApp = 0;
bool g_bFindFirstLocal = false;

PG_STAT_H(PgStatGroup g_kMobileSuitStatGroup("MobileSuit"));
PG_STAT_H(PgStatGroup g_kActorStatGroup("PgActor"));
PG_STAT_H(PgStatGroup g_kRenderManStatGroup("PgRenderMan"));
PG_STAT_H(PgStatGroup g_kWorldStatGroup("PgWorld"));
PG_STAT_H(PgStatGroup g_kUIStatGroup("PgUIScene"));
PG_STAT_H(PgStatGroup g_kScriptStatGroup("PgScript"));
PG_STAT_H(PgStatGroup g_kPilotStatGroup("PgPilot"));
PG_STAT_H(PgStatGroup g_kLocalInputStatGroup("PgLocalInput"));
PG_STAT_H(PgStatGroup g_kRemoteInputStatGroup("PgRemoteInput"));

extern CPgLuaDebugger *g_pkScriptDebug;

extern ControllerManager g_kControllerManager;
extern void SkillOnTick(float const fAccumTime, float const fFrameTime);//xxx todo

extern bool g_bDisconnectFlush;
extern void OnDisconnectFlush();
extern TCHAR g_szSuperGirlURL[512];
extern void NexonInit();

char	*g_pkCacheBuffer = NULL;
DWORD	g_dwCacheBufferSize = 1024*1024*50;	//	50MB

void	CacheFileInFolder(const char *pkFolder,const char *pkExtension)
{
	TCHAR fileName[MAX_PATH];
	WIN32_FIND_DATA findData;
	_stprintf(fileName, _T("%s/*.*"), UNI(pkFolder));
	HANDLE findHandle = FindFirstFile(fileName, &findData);
	if (findHandle ==  INVALID_HANDLE_VALUE)
		return ;
	do
	{
		if( (findData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) !=0 )
		{
			continue;
		}

		if (wcscmp(findData.cFileName,_T(".")) == 0 || 
			wcscmp(findData.cFileName,_T("..")) == 0)
		{
			continue;
		}

		if((findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			TCHAR	kNextFolderPath[MAX_PATH];
			swprintf_s(kNextFolderPath, MAX_PATH, _T("%s/%s"), UNI(pkFolder), findData.cFileName );
			std::wstring	kNextPath(kNextFolderPath);
			CacheFileInFolder(MB(kNextPath),pkExtension);
		}

		TCHAR	kFoundPath[MAX_PATH];
		swprintf_s(kFoundPath,MAX_PATH,_T("%s/%s"),UNI(pkFolder),findData.cFileName);

		FILE	*fp = _wfopen(kFoundPath,_T("rb"));

		if(fp)
		{
			fseek(fp,0,SEEK_END);
			long	ulSize = ftell(fp);
			if(ulSize>0)
			{
				if(ulSize>g_dwCacheBufferSize)
				{
					delete []g_pkCacheBuffer;
					g_dwCacheBufferSize = ulSize;
					g_pkCacheBuffer = new char[ulSize];
				}
				fseek(fp,0,SEEK_SET);

				fread(g_pkCacheBuffer,ulSize,1,fp);
			}

			fclose(fp);
		}

	} while (FindNextFile(findHandle, &findData));

	FindClose(findHandle);
}

//void CacheFile()
//{
//	g_pkCacheBuffer = new char[g_dwCacheBufferSize];
//	DWORD	dwStartTime = BM::GetTime32();
//	CacheFileInFolder("../Data/1_Cha/9_SSkill","*.nif");
//	CacheFileInFolder("../Data/5_Effect","*.nif");
//	CacheFileInFolder("../Data/4_Item","*.nif");
//	CacheFileInFolder("../Sound","*.nif");
//	CacheFileInFolder("../XML","*.nif");
//
//	delete []g_pkCacheBuffer;
//	g_pkCacheBuffer = NULL;
//
//	DWORD	dwEndTime = BM::GetTime32() - dwStartTime;
//	_PgOutputDebugString("Cache Time : %d\n",dwEndTime);
//}

//void bugtraptest(void)
//{
//	_PgMessageBox("bugtrap", "test");
//}

static void SetupExceptionHandler()
{
	/*BT_InstallSehFilter();
	BT_SetAppName(g_szDumpAppName);
	std::string strVersion((char const*)VERSION_FILEVERSION_CLIENT_STR);
	strVersion = strVersion + " (" + __DATE__ + ")";
	BT_SetAppVersion(UNI(strVersion));
	BT_SetSupportEMail(_T(""));
	//BT_SetActivityType(BTA_SENDREPORT);// 다이얼로그 없이 바로 보내기
	BT_SetDumpType( MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory );
	DWORD dwFlag = BTF_DETAILEDMODE | BTF_ATTACHREPORT;
	if(g_iBugTrapScreenShot)
	{
		dwFlag = dwFlag | BTF_SCREENCAPTURE;
	}
	BT_SetFlags(dwFlag);
	BT_SetSupportServer(g_szDumpAddr, g_nDumpPort);
	BT_SetSupportURL(g_szDumpAddr);
	BT_SetSupportPort(g_nDumpPort);
	//BT_SetCallBack((PFCALLBACK)bugtraptest);

	BT_SetCallBack(reinterpret_cast<PFCALLBACK>(_Antihack_UnhandledExceptionHandler));//! 예외시 핵실드 종료 루틴 호출*/
}

static void SetupExceptionHandlerLogVideoDriver()
{
	/*BTTrace trace(NULL, BTLF_TEXT);
	trace.Clear();
	trace.InsertF(_T("video card: %s, Ver(%s)"), 
		UNI(PgComputerInfo::GetGPUInfo().strName), UNI(PgComputerInfo::GetGPUInfo().strVersion));
	BT_AddLogFile(BT_GetLogFileName(trace.GetHandle()));
	trace.Close();*/
}

void setThreadNames()
{
	HANDLE hMainThread = GetCurrentThread();

	SetThreadName((DWORD)hMainThread, "Dragonica");

	ACE_Thread_Manager* pManager = NULL;
	ACE_hthread_t kThreadIdArray[20];
	ssize_t count = 0;
	char buf[64];
	pManager = ACE_Thread_Manager::instance();

	if (pManager)
	{
		count = pManager->hthread_list(&g_kLoader, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
		for (int i = 0; i < count; i++)
		{
			sprintf_s(buf, 64, "LoadingThread%d", i);
			SetThreadName((DWORD)kThreadIdArray[i], buf);
		}

		count = pManager->hthread_list(&g_kAddUnitThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
		for (int i = 0; i < count; i++)
		{
			sprintf_s(buf, 64, "AddUnitThread%d", i);
			SetThreadName((DWORD)kThreadIdArray[i], buf);
		}

		count = pManager->hthread_list(&g_kMemoryWorkerThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
		for (int i = 0; i < count; i++)
		{
			sprintf_s(buf, 64, "MemoryWorker%d", i);
			SetThreadName((DWORD)kThreadIdArray[i], buf);
		}

		count = pManager->hthread_list(&g_kLoadWorkerThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
		for (int i = 0; i < count; i++)
		{
			sprintf_s(buf, 64, "LoadWorkerThread%d", i);
			SetThreadName((DWORD)kThreadIdArray[i], buf);
		}
	}	
}

// Instancing application
NiApplication* NiApplication::Create()
{
	BM::g_bCalcSHA2CheckSum = true;
	BM::g_bUseFormattedFile = true;
	g_pkApp = NiNew PgMobileSuit;
	return g_pkApp;
}

char const* GetClientName()
{
	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_KOREA:
	case LOCAL_MGR::NC_SINGAPORE:
		return "Dragonsaga";
	case LOCAL_MGR::NC_NOT_SET:
		return PG_CLIENT_NAME;
	}
	return "Dragonica";
}

wchar_t const* GetClientNameW()
{
	static std::wstring kName;
	kName = UNI(GetClientName());
	return kName.c_str();
}

char const* GameTitle()
{
	static std::string kTitle;
	kTitle = GetClientName();
	
#if defined(USE_INB)

#elif defined(EXTERNAL_RELEASE)
	kTitle += " (ver ";
	kTitle += VERSION_FILEVERSION_CLIENT_STR;
	kTitle += ")";
#elif defined(NDEBUG)
	kTitle += " (INTERNAL ver ";
	kTitle += VERSION_FILEVERSION_CLIENT_STR;
	kTitle += ")";
#elif defined(_DEBUG)
	kTitle += " (DEBUG ver ";
	kTitle += VERSION_FILEVERSION_CLIENT_STR;
	kTitle += ")";
#endif
	return kTitle.c_str();
}


PgMobileSuit::PgMobileSuit()
    : NiApplication(GameTitle(), XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y), 
      m_pkDrawWorkflow(NULL)
{
#if defined(_DEBUG)
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_ALLOC_MEM_DF;
	//flag |= _CRTDBG_DELAY_FREE_MEM_DF;
	//flag |= _CRTDBG_CHECK_ALWAYS_DF;
	//flag |= _CRTDBG_CHECK_CRT_DF;
	flag |= _CRTDBG_LEAK_CHECK_DF;
	//flag |= _CRTDBG_CHECK_EVERY_128_DF;
	_CrtSetDbgFlag(flag);

	m_bDebugMode = true;
#else
	m_bDebugMode = false;
#endif 

	SetVSync(false);
	m_hMutexHandle = NULL;
	m_fVTMemoryMax = 150000.0f; 
	m_fVTPerformanceMax = 360.0f;
	m_pkShaderHelper = 0;
	m_bFrameRateEnabled = false;
	m_iMaxFrameRate = 60;
	m_bSingleMode = false;
	m_iWorkerThreadCount = 1;
	m_iMemoryWorkerThreadCount = 1;
	m_bVisibleClassNo = false;
	m_bVisibleTextDebug = false;
	m_bVisibleQTextDebug = false;
	m_bActivated = false;
	m_bInitialized = false;
	m_bKeySettingBackuped = false;
	m_wServerPort = 0;
	m_uiRefreshRate = NiDX9Renderer::REFRESHRATE_DEFAULT;

	m_bUsedEraseBackgroundMessage = true;

	UseGameGuard(0);
	UseGSM(false);
	UseWebLinkage(false);
	m_iFullScreenIni = -1;

	m_iFrameCount = 0;
	m_dwLastFrameTime = 0;
	m_dwEventViewElapsed = 0;

	memset(&m_kWindowRect, 0, sizeof(RECT));
	memset(&m_kClientRect, 0, sizeof(RECT));
	memset(&m_kWindowPos, 0, sizeof(POINT));
	m_bUseSmallWindow = false;

	char pcInitConfig[MAX_PATH] = "../Config.ini";

	PG_ASSERT_LOG(ms_pkCommand);
	if (ms_pkCommand)
	{
		ms_pkCommand->String("config", pcInitConfig, sizeof(pcInitConfig));
	}

	m_bMultiThread = true;

#if defined(_UNICODE) || defined(UNICODE)
	MultiByteToWideChar(CP_ACP, 0, pcInitConfig, MAX_PATH, m_szConfigFile, MAX_PATH);	
#else
	SAFE_STRNCPY(m_szConfigFile, pcInitConfig);
#endif

#ifdef USE_INB
	const wchar_t* defaultLocale = _T("kor");	
	if(!g_kLocal.LoadFromINB(_T("../Config.inb"), defaultLocale))
#else
	const wchar_t* defaultLocale = _T("kor");
	if(!g_kLocal.LoadFromINI(m_szConfigFile, defaultLocale))
#endif
	{
		PgError("NiApplication::Initialize Locale Failed");
	}

	SetStandardResolutionSize(POINT2(0,0));

	SetLoginIP();
#ifndef USE_INB
	SetInterLoginIP();
#endif
	readConfigFile();

	SYSTEMTIME kLocalTime;
	::GetLocalTime(&kLocalTime);
	SYSTEMTIME kGameTime = kLocalTime;
	g_kEventView.SetGameTime( &kLocalTime, &kGameTime );
}

PgMobileSuit::~PgMobileSuit()
{
    delete m_pkDrawWorkflow;

	NILOG(PGLOG_LOG, "PgMobileSuit Destructor");
	PG_FLUSH_LOG;
}

bool CheckExistFile(std::string strFilenName)
{
	HANDLE hFile = CreateFileA(strFilenName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER iFileSize;
	BOOL bsize = GetFileSizeEx(hFile, &iFileSize);
	if(!bsize || iFileSize.QuadPart == 0)
	{
		CloseHandle(hFile);	
		return false;
	}

	CloseHandle(hFile);	

	return true;
}
bool PgMobileSuit::Initialize()
{
	//	DUMMY_LOG	INIT_LOG(); // 로그 테스트 할사람만 켜세요 

	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		ACE::init();
	}
	std::wstring mutexName = GetClientNameW();
	mutexName += TEXT("_Client");

	m_kPatchVer.iVersion = ReadPatchVer();

#ifndef EXTERNAL_RELEASE
	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		if( ::OpenClipboard(NULL) )//클립보드 초기화
		{
			::EmptyClipboard();
			::CloseClipboard();
		}
	}
#endif
	PG_TRY_BLOCK

#ifdef USE_INB	//USE_INB 일때에만 클라이언트 한개
	HANDLE mutex;
	mutex = CreateMutex(NULL, TRUE, mutexName.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		std::wstring kMsg;
		PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("DuplicateError"), kMsg, _T("another client is already running."));
		::MessageBox(m_pkAppWindow->GetRenderWindowReference(), kMsg.c_str(), GetClientNameW(), MB_OK);
		return false;
	}
	m_hMutexHandle = mutex;
#endif	

#ifdef USE_INB
	if (g_kLocal.ServiceRegion() == LOCAL_MGR::NC_EU)
	{
		if (false == g_kCmdLineParse.LaunchByLauncher())
		{
			std::wstring kMsg;
			PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("DirectExecuteError"), kMsg, _T("Dragonica Client must be launched by Launcher."));
			::MessageBox(m_pkAppWindow->GetRenderWindowReference(), kMsg.c_str(), GetClientNameW(), MB_OK);
			return false;
		}
	}
	else if (g_kLocal.ServiceRegion() == LOCAL_MGR::NC_KOREA)
	{
		if( false == g_kWebLogin.IsCorrectArg() )
		{
			std::wstring kMsg;
			PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("DirectExecuteError"), kMsg, _T("Dragonica Client must be launched by Launcher."));
			::MessageBox(m_pkAppWindow->GetRenderWindowReference(), kMsg.c_str(), GetClientNameW(), MB_OK);
			return false;
		}
	}
#endif
	SetClassLong(GetWindowReference(), GCL_HICON, (LONG)LoadIcon(GetInstanceReference(), MAKEINTRESOURCE(IDI_MOBILESUIT)));
	SetTitleByConfig();
	adjustUrl();

	std::wstring kTempMsg;
	PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("GameTitle"), kTempMsg, GetClientNameW());
	AddStringToWindowTitle(kTempMsg.c_str(), 0);

	TIMECAPS caps;
	if (timeGetDevCaps(&caps, sizeof(caps)) == TIMERR_NOERROR)
	{
		timeBeginPeriod(caps.wPeriodMin);
	}	
	PgComputerInfo::InitMemStatus();


#ifdef PG_EXCEPTION_ENABLED
	if(g_iUseBugTrap)
	{
		SetupExceptionHandler();// 버그트랩을 이용한 덤프전송
	}
	MSJExceptionHandler::InstallExceptionHandler(g_iUseBugTrap == 0);
#endif


	std::wstring kStrTemp = _T("TestString");
	MB(kStrTemp);


	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		initLogSystem();
		if (GetCommand() && GetCommand()->GetCmdLine()) 
		{
			NILOG(PGLOG_LOG, "Cmd line: %s\n", GetCommand()->GetCmdLine());
		}
		NILOG(PGLOG_LOG, "Start Initialization\n");	
		NiSourceTexture::SetDestroyAppDataFlag(true);
		NiImageConverter::SetPlatformSpecificSubdirectory("../Data/3_World/Map_Textures", 0);		
		NiImageConverter::SetPlatformSpecificSubdirectory("../Data/5_Effect/9_Tex", 1);
		NiImageConverter::SetPlatformSpecificSubdirectory("../Data/3_World/97_Myhome/09_Texture", 2);
		//NiImageConverter::SetPlatformSpecificSubdirectory("../Data/3_World/Map_Textures", 3);
		PgNiFile::SetFileReadMode(false, false, g_bUsePackData);
	}

	if (g_bUsePackData)
	{
		bool bCheckFile = true;
		TCHAR szCheckedFileName[2048] = { 0, };

		if(!CheckExistFile("../SCRIPT.DAT"))
		{
			_tcscat(szCheckedFileName, _T("SCRIPT.DAT "));
			bCheckFile = false;
		}
		else
		{
			BM::PgDataPackManager::GetPackFileCache(L"../SCRIPT.DAT", true);
		}

		if(!CheckExistFile("../XML.DAT"))
		{
			_tcscat(szCheckedFileName, _T("XML.DAT "));
			bCheckFile = false;
		}
		else
		{
			BM::PgDataPackManager::GetPackFileCache(L"../XML.DAT", true);
		}

		if (g_bUseExtraPackData)
		{
			if(!CheckExistFile("../Data/1_CHA.dat"))
			{
				_tcscat(szCheckedFileName, _T("1_CHA.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/2_MON.dat"))
			{
				_tcscat(szCheckedFileName, _T("2_MON.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/2_MON2.dat"))
			{
				_tcscat(szCheckedFileName, _T("2_MON2.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/3_World.dat"))
			{
				_tcscat(szCheckedFileName, _T("3_WORLD.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/4_ITEM.dat"))
			{
				_tcscat(szCheckedFileName, _T("4_ITEM.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/5_EFFECT.dat"))
			{
				_tcscat(szCheckedFileName, _T("5_EFFECT.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../Data/6_UI.dat"))
			{
				_tcscat(szCheckedFileName, _T("6_UI.DAT "));
				bCheckFile = false;
			}
			if(!CheckExistFile("../BGM.DAT"))
			{
				_tcscat(szCheckedFileName, _T("BGM.DAT "));
				bCheckFile = false;
			}
		}

		if (bCheckFile == false)
		{
			std::wstring kTempMsg;
			PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("DataFileError"), kTempMsg, _T("file is not exist or corrupted. Reinstall client, please."));
			_tcscat(szCheckedFileName, kTempMsg.c_str());
			::MessageBox(m_pkAppWindow->GetRenderWindowReference(), szCheckedFileName, GetClientNameW(), MB_OK);
			return false;
		}

#ifdef USE_INB	// 마무리 될때까지는 막자.
		//! 여기서 Pack의 버젼을 검사한다. 버젼이 다르면 false, false, false!!!
		bool bCheckVersion = true;
		TCHAR szVersionCheckedFileName[2048] = { 0, };

		if(!BM::PgDataPackManager::CheckPackVersion(L"../script.dat", m_kPatchVer))
		{
			_tcscat(szVersionCheckedFileName, _T("SCRIPT.DAT "));
			bCheckVersion = false;
		}
		if(!BM::PgDataPackManager::CheckPackVersion(UNI(PgXmlLoader::GetPackFilePath()), m_kPatchVer))
		{
			_tcscat(szVersionCheckedFileName, _T("XML.DAT "));
			bCheckVersion = false;
		}

		if (g_bUseExtraPackData)
		{
			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/3_World.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("3_WORLD.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/1_CHA.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("1_CHA.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/2_MON.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("2_MON.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/2_MON2.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("2_MON2.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/4_ITEM.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("4_ITEM.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/5_EFFECT.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("5_EFFECT.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../Data/6_UI.dat", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("6_UI.DAT "));
				bCheckVersion = false;
			}

			if(!BM::PgDataPackManager::CheckPackVersion(L"../BGM.DAT", m_kPatchVer))
			{
				_tcscat(szVersionCheckedFileName, _T("BGM.DAT "));
				bCheckVersion = false;
			}
		}

		if (bCheckVersion == false)
		{
			std::wstring kTempMsg;
			PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("DataFileVersionError"), kTempMsg, _T("file's version is not correct. Reinstall client, please"));
			_tcscat(szVersionCheckedFileName, kTempMsg.c_str());
			::MessageBox(m_pkAppWindow->GetRenderWindowReference(), szVersionCheckedFileName, GetClientNameW(), MB_OK);
			return false;
		}
#endif
		//! 여기서 Pack쓸 파일들은 지정해줘야 한다. script.dat/xml.dat는 알아서 되므로 Pass!
		if (g_bUseExtraPackData)
		{
			PgNiFile::RegisterPackFile("/Data/3_World/", "../Data/3_World.dat");
			PgNiFile::RegisterPackFile("/Data/1_Cha/", "../Data/1_CHA.dat");
			PgNiFile::RegisterPackFile("/Data/2_Mon/", "../Data/2_MON.dat");
			PgNiFile::RegisterPackFile("/Data/2_Mon2/", "../Data/2_MON2.dat");
			PgNiFile::RegisterPackFile("/Data/4_Item/", "../Data/4_ITEM.dat");
			PgNiFile::RegisterPackFile("/Data/5_Effect/", "../Data/5_EFFECT.dat");
			PgNiFile::RegisterPackFile("/Data/6_UI/", "../Data/6_UI.dat");		
			PgNiFile::RegisterPackFile("/BGM/", "../BGM.DAT");
			if( false == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_TAIWAN ) )
			{
				PgNiFile::RegisterPackFile("/Font/", "../font.dat");
			}
		}
	}

	PgNiEntityStreamingAscii::_SDMInit();
	NiFactoryRegisterStreaming(PgNiEntityStreamingAscii);
	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		PgXmlLoader::CreateObject("CONFIG");//Load Default Config xml
	}
	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		g_kGlobalOption.Load();
		adjustGameOption();
	}

	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		NILOG(PGLOG_LOG, "NiApplication::Initialize\n");
		
		if(!m_bFullscreen && !m_bBorderless)
		{
			PreInitializeForWindowMode(XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y);
		}
		if(!NiApplication::Initialize())
		{
			PgError("NiApplication::Initialize Failed");
			return false;
		}

		// Recreate the renderer to address incorrect UI resolution
		RecreateRenderer((g_kGlobalOption.GetValue("GAME", "FULL_SCREEN") == 1),
			(g_kGlobalOption.GetValue("GAME", "BORDERLESS") == 1),
			g_kGlobalOption.GetValue(XML_ELEMENT_GAME, STR_GAME_RESOLUTION_WIDTH),
			g_kGlobalOption.GetValue(XML_ELEMENT_GAME, STR_GAME_RESOLUTION_HEIGHT),
			0,	// RefreshRate 기본값 0
			(g_kGlobalOption.GetCurrentGraphicOption("REFRESH_RATE_SYNC") == 1));

#ifdef PG_EXCEPTION_ENABLED
		if(g_iUseBugTrap)
		{
			SetupExceptionHandlerLogVideoDriver();
		}
#endif
	}

    // NiRenderer를 만들고 난 다음 DrawWorkflow를 만듬.
    m_pkDrawWorkflow = new NewWare::Renderer::DrawWorkflow( NiRenderer::GetRenderer() );
    assert( m_pkDrawWorkflow );

#ifndef EXTERNAL_RELEASE
	// Console 띄우기
	if(g_iUseDebugConsole)
	{
		AllocConsole();
	}
#endif
	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		//Pool Init
		InitFreedomPool();
	}

	{
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

		NILOG(PGLOG_LOG, "Start TEXT_TABLE Initialization\n");
		g_pkTT = dynamic_cast<PgTextTable *>(PgXmlLoader::CreateObject(XML_ELEMENT_TEXT_TABLE));
		NILOG(PGLOG_LOG, "Start ERROR_CODE_TABLE Initialization\n");
		g_pkErrorCT = dynamic_cast<PgErrorCodeTable *>(PgXmlLoader::CreateObject(XML_ELEMENT_ERROR_CODE_TABLE));

		SetMaxFrameRate((float)m_iMaxFrameRate);
	}

	{
		PgXmlLoader::CreateObject(XML_ELEMENT_CUSTOM_QUEST_MARKER);
		PgXmlLoader::CreateObject(XML_ELEMENT_IMPORTANCE_QUEST);
		PgXmlLoader::CreateObject(XML_ELEMENT_STITCH_IMAGE);
		PgXmlLoader::CreateObject(XML_ELEMENT_TEXT_DIALOG);
	}

	//g_kVersionCheck.PatcherCall(false);

	//if(g_kLocal.ServiceRegion() == LOCAL_MGR::NC_CHINA)
	//{
	//	LPVOID lpRtn = NULL;
	//	//bool bRtn = g_kVersionCheck.Compare(_T("125.131.115.181"), 6390, _T("ap"), _T("ap1234"), lpRtn);
	//	bool bRtn = g_kVersionCheck.Compare(g_kVersionCheck.ChinaPatchAddr().c_str(), g_kVersionCheck.ChinaPatchPort(), _T(""), _T(""), lpRtn);
	//	if(bRtn == false)
	//	{
	//		g_kVersionCheck.PatcherCall(true);
	//		return false;
	//	}
	//}

#ifdef PG_USE_THREAD_LOADING
	{

		//CacheFile(); // 테스트용이라고 들었음! (함수내부 return에서 쓰이는 곳에 주석 처리로 바꿈)
		ChangeStickyKeySetting();

		setMainThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
		g_kLoadWorkerThread.activate(THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, PgComputerInfo::GetCPUInfo().iNumProcess, 0, THREAD_PRIORITY_BELOW_NORMAL);
		while ( !g_kLoadWorkerThread.IsWorking() )
		{
			unsigned int const iSleepTime = 10;
			NiSleep(iSleepTime);
		}

		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		doInitializeWork(InitializeWorkID(0));			
		doInitScript();
		for( int i = 1; INIT_WORK_ID_MAX >i; i++)
		{
			SLoadMessage kMsg(i);
			g_kLoader.PutMsg(kMsg);
		}

		AdjustWindowStyle(m_bFullscreen || (g_kGlobalOption.GetValue("GAME", "BORDERLESS") == 1), XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y, m_bFullscreen ? false : true );

		g_kLoader.StartSvc(5);
		setBackgroundThreadPriority(THREAD_PRIORITY_NORMAL, 0);
		setBackgroundThreadPriority(THREAD_PRIORITY_NORMAL, 3);
		g_kLoader.WaitLoad();
	}
#else // #ifdef PG_USE_THREAD_LOADING
	{
		PgTimeCheck kTimeCheck(g_kLogWorker, LT_CAUTION, __FUNCTIONW__, __LINE__);

		for (InitializeWorkID eID = INIT_WORK_ID_0; eID < INIT_WORK_ID_MAX; eID = (InitializeWorkID)(eID + 1))
		{
			if (doInitializeWork(eID) == false)
			{
				return false;
			}
		}
	}
#endif // #ifdef PG_USE_THREAD_LOADING	

	if (m_bDestroying == false)
	{
		//// 백그라운드 워커 스레드를 구동한다.
		{
			PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
			setMainThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);

			g_kAddUnitThread.activate(THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, m_iWorkerThreadCount > 0 ? m_iWorkerThreadCount : PgComputerInfo::GetCPUInfo().iNumProcess, 0, THREAD_PRIORITY_BELOW_NORMAL);
			g_kEquipThread.activate(THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, PgComputerInfo::GetCPUInfo().iNumProcess + 1, 0, THREAD_PRIORITY_BELOW_NORMAL);		
			g_kMemoryWorkerThread.activate(THR_NEW_LWP| THR_JOINABLE | THR_INHERIT_SCHED, m_iMemoryWorkerThreadCount > 0 ? m_iMemoryWorkerThreadCount : PgComputerInfo::GetCPUInfo().iNumProcess, 0, THREAD_PRIORITY_NORMAL);

			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 1);
			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 2);
			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 3);
			PgClientWorkerThread::SetWorkerSleepControl(true);
		}
#ifndef USE_INB
		{
			PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
			if (m_bFrameRateEnabled && m_pkFrameRate == NULL)
			{
				m_pkFrameRate = NiNew NiFrameRate();
				if (m_pkFrameRate)
				{
					m_pkFrameRate->Init(true);
				}
			}
		}
#endif	
		CONT_DEF_FILTER_UNICODE const* pkFilterUnicode = NULL;
		g_kTblDataMgr.GetContDef(pkFilterUnicode);
		CONT_DEF_FILTER_UNICODE::const_iterator filter_iter = pkFilterUnicode->begin();
		while( pkFilterUnicode->end() != filter_iter )
		{// 유니코드 필터링 범위 얻음
			CONT_DEF_FILTER_UNICODE::value_type const& rkFilter = (*filter_iter);
			g_kUnicodeFilter.AddRange(rkFilter.iFuncCode, rkFilter.bFilterType, rkFilter.cStart, rkFilter.cEnd);
			++filter_iter;
		}
		PgScripting::DoString("UI_endLogo()");
		int iOpening = g_kGlobalOption.GetValue(XML_ELEMENT_MOVIE, XML_ATTR_MOVIE_OPENING);
		if(!iOpening)
		{
			g_kGlobalOption.SetConfig(XML_ELEMENT_MOVIE, XML_ATTR_MOVIE_OPENING, 1, NULL);
			g_kGlobalOption.ApplyConfig();
			g_kGlobalOption.Save();
			g_kMovieMgr.Play("opening");
		}
		m_bInitialized = true;
	}
	setThreadNames();
	NILOG(PGLOG_LOG, "End Initialization\n");
	PG_CATCH_BLOCK
	return true;
}

void	OutputGeometryInfo(NiAVObject *pkObject,int iTab)
{
	if(!pkObject)
	{
		return;
	}

	if(iTab == 0)
	{
		_PgOutputDebugString("----------------------\n");
	}

	char	str[1000];
	str[0]='\0';

	for(int i=0;i<iTab;i++)
	{
		strcat(str,"\t");
	}

	char	str2[1000];
	sprintf_s(str2,1000,"[%s]\n",pkObject->GetName());

	strcat(str,str2);

	_PgOutputDebugString(str);

	if(pkObject->GetParent())
	{
		OutputGeometryInfo(pkObject->GetParent(),iTab+1);
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(
//	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
//{
//	return NULL;
//}
//LONG WINAPI MyDummySetUnhandledExceptionFilter(PEXCEPTION_POINTERS lpExceptionInfo)
//{
//	return NULL;
//}
//
//BOOL PreventSetUnhandledExceptionFilter()
//{
//	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
//	if(hKernel32 == NULL)
//	{
//		return FALSE;
//	}
//	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
//	if(pOrgEntry == NULL)
//	{
//		return FALSE;
//	}
//	unsigned char newJump[100];
//	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
//	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
//	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
//	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
//	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
//
//	newJump[ 0 ] = 0xE9;  // JMP absolute
//	memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));
//	SIZE_T bytesWritten;
//	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
//		pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
//	return bRet;
//}

void trans_func_s(unsigned int u, PEXCEPTION_POINTERS lpExpPtr)
{
	throw new CSystemException(lpExpPtr);
	//exit(-1);
}


void PgMobileSuit::Terminate()
{
#ifdef USE_INB
	try {
#ifdef PG_EXCEPTION_ENABLED
		if(g_iUseBugTrap)
		{
			//BT_UninstallSehFilter();
		}
		MSJExceptionHandler::UninstallExceptionHandler();
		_set_se_translator(trans_func_s);
#endif
#endif// USE_INB

#ifndef USE_INB
		PG_TRY_BLOCK
#endif
		PG_FLUSH_LOG;
		RestoreStickyKeySetting();
		//g_pkMemoryTrack->DisplayState(g_aLog[LT_INFO]);	

		//if( m_bInitialized )
		//{
		//	// 중복 실행시 Config 날라가지 않도록
		//	g_kGlobalOption.ApplyConfig();
		//	g_kGlobalOption.Save();
		//}

		g_kLogWorker.VDeactivate();
		g_kCoreCenter.Close();

		// 설문조사
		if(!g_strFinishLinkURL.empty())
		{
			HINSTANCE hInstance = ShellExecute(NULL, _T("open") , g_strFinishLinkURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}

		m_bInitialized = false;
		g_kSoundMan.StopBgSound();

#ifndef EXTERNAL_RELEASE
		if(g_iUseDebugConsole)
		{
			FreeConsole();
		}
#endif
		PgNiEntityStreamingAscii::_SDMShutdown();
		NILOG(PGLOG_LOG, "XUIMgr CloseAll\n");

		XUIMgr.ClearMediaPlayer();//사운드용 static point NULL

		XUIMgr.CloseAll();

		g_kSkillTree.Terminate();
		g_kSkillOptionMan.Terminate();

		g_kChatMgrClient.Active(false);
		g_kQuestMan.SaveMiniQuest();
		NILOG(PGLOG_LOG, "Terminate Application\n");
		stopAllThread();
		PG_STAT(g_kMobileSuitStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kActorStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kRenderManStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kWorldStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kUIStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kScriptStatGroup.PrintStatGroupForCSV());
		PG_STAT(g_kPilotStatGroup.PrintStatGroupForCSV());	
		PG_STAT(g_kLocalInputStatGroup.PrintStatGroupForCSV());	
		PG_STAT(g_kRemoteInputStatGroup.PrintStatGroupForCSV());	
		PG_STAT(PgNiFile::ms_kFileIOStatGroup.PrintStatGroupForCSV());
		PG_STAT(PgNiFile::ms_kFileStatGroup.PrintStatGroupForCSV());
		PG_RESOURCE_MONITOR(g_kResourceMonitor.PrintResourceCountersForCSV());
		//if (g_pkRemoteManager) // 나가기 전의 마지막 위치를 줄 필요가 있는가?
		//	g_pkRemoteManager->SendPing();
		NILOG(PGLOG_LOG, "g_kWorldObjectPool Terminate\n");
		g_kWorldObjectPool.Terminate();
		NILOG(PGLOG_LOG, "g_kControllerSequenceManager Terminate\n");
		g_kControllerSequenceManager.Terminate();
		NILOG(PGLOG_LOG, "Pg2DString::DestroyStatics\n");
		Pg2DString::DestroyStatics();
		NILOG(PGLOG_LOG, "g_kWorldMapUI Destroy\n");
		g_kWorldMapUI.Destroy();
		NILOG(PGLOG_LOG, "g_kChatWindow.TerminateWindow\n");
		g_kChatWindow.TerminateWindow();

#ifndef USE_INB
		NILOG(PGLOG_LOG, "g_kSkillTargetMan Destroy\n");
		g_kSkillTargetMan.Destroy();
		g_kMonSkillTargetMan.Destroy();
		NILOG(PGLOG_LOG, "g_kZoneDrawer Destroy\n");
		g_kZoneDrawer.Destroy();
#endif//USE_INB
		NILOG(PGLOG_LOG, "g_kPartyBalloonMgr Destroy\n");
		g_kPartyBalloonMgr.Destroy();
		NILOG(PGLOG_LOG, "g_kExpeditionBalloonMgr Destroy\n");
		g_kExpeditionBalloonMgr.Destroy();
		NILOG(PGLOG_LOG, "g_kVendorBalloonMgr Destroy\n");
		g_kVendorBalloonMgr.Destroy();
		NILOG(PGLOG_LOG, "g_kBalloonMan2D Destroy\n");
		g_kBalloonMan2D.Destroy();
		NILOG(PGLOG_LOG, "g_kBalloonMan3D Destroy\n");
		g_kBalloonMan3D.Destroy();
		NILOG(PGLOG_LOG, "PgBoardBalloon Static Destroy\n");
		PgBoardBalloon::DestoryStatic();
		NILOG(PGLOG_LOG, "PgGuildMark Static Destroy\n");
		PgGuildMark::DestoryStatic();
		NILOG(PGLOG_LOG, "PgGuardianMark Static Destroy\n");
		PgGuardianMark::DestoryStatic();
		NILOG(PGLOG_LOG, "g_kMarkBalloonMan Destroy\n");
		g_kMarkBalloonMan.Destroy();
		NILOG(PGLOG_LOG, "g_kQuestMarkHelpMng Destroy\n");
		g_kQuestMarkHelpMng.Destroy();
		NILOG(PGLOG_LOG, "g_kScreenBreak Destroy\n");
		g_kScreenBreak.Destroy();
		NILOG(PGLOG_LOG, "g_kChainAttack Destroy\n");
		g_kChainAttack.Destroy();
		NILOG(PGLOG_LOG, "g_kMissionComplete Destroy\n");
		g_kMissionComplete.Destroy();
		NILOG(PGLOG_LOG, "g_kBossComplete Destroy\n");
		g_kBossComplete.Destroy();
		NILOG(PGLOG_LOG, "g_kExpeditionComplete Destroy\n");
		g_kExpeditionComplete.Destroy();
		NILOG(PGLOG_LOG, "g_kNifMan Destroy\n");
		g_kNifMan.Destroy();
		NILOG(PGLOG_LOG, "g_kQuestMan Destroy\n");
		g_kQuestMan.Destroy();
		NILOG(PGLOG_LOG, "g_DynamicTextureMan Destroy\n");
		g_DynamicTextureMan.Destroy();
		NILOG(PGLOG_LOG, "g_kLinkedPlaneGroupMan Destroy\n");
		g_kLinkedPlaneGroupMan.Destroy();
		NILOG(PGLOG_LOG, "g_kHeadBuffIconListMgr Destroy\n");
		g_kHeadBuffIconListMgr.Terminate();
		NILOG(PGLOG_LOG, "g_kBalloonEmoticonMgr Destroy\n");
		g_kBalloonEmoticonMgr.Terminate();
		NILOG(PGLOG_LOG, "PgElementMark Static Destroy\n");
		PgElementMark::DestoryStatic();
		NILOG(PGLOG_LOG, "PgDuelTitle Static Destroy\n");
		PgDuelTitle::DestoryStatic();
		g_kSelectStage.Terminate();

		g_kEnvironmentMan.Terminate();

		NILOG(PGLOG_LOG, "g_kUIScene Destroy\n");
		g_kUIScene.Terminate();
		PgMiniMapBase::ReleaseAll();

		NILOG(PGLOG_LOG, "XUIMgr Destroy\n");
		XUIMgr.Destroy();

		NILOG(PGLOG_LOG, "PgLocalManager DestroySingleton\n");
		PgLocalManager::DestroySingleton();	

		NILOG(PGLOG_LOG, "g_pkRemoteManager Destroy\n");
		SAFE_DELETE_NI(g_pkRemoteManager);


		NILOG(PGLOG_LOG, "g_kProjectileMan Destroy\n");
		g_kProjectileMan.Destroy();

		PG_FLUSH_LOG;
		NILOG(PGLOG_LOG, "g_kRenderMan Destroy\n");
		g_kRenderMan.Terminate();
		NILOG(PGLOG_LOG, "g_kPilotMan Destroy\n");
		g_kPilotMan.Destroy();
		PG_FLUSH_LOG;

		NILOG(PGLOG_LOG, "g_kStatusEffectMan Destroy\n");
		g_kStatusEffectMan.Destroy();

		NILOG(PGLOG_LOG, "g_kHelpSystem.TerminateSystem()\n");
		g_kHelpSystem.TerminateSystem();

		NILOG(PGLOG_LOG, "g_kAMPool Destroy\n");
		g_kAMPool.Destroy();
		NILOG(PGLOG_LOG, "g_kActionPool Destroy\n");
		g_kActionPool.Destroy();
		NILOG(PGLOG_LOG, "g_kItemMan Destroy\n");
		g_kItemMan.ClearAllCache();
		NILOG(PGLOG_LOG, "g_kComboAdvisor Destroy\n");
		g_kComboAdvisor.Destroy();
		NILOG(PGLOG_LOG, "g_kParticleMan Destroy\n");
		g_kParticleMan.Terminate();
		g_kFxStudio.Terminate();

		NILOG(PGLOG_LOG, "g_kControllerManager purgeControllers\n");
		g_kControllerManager.purgeControllers();

		NILOG(PGLOG_LOG, "terminateAllThread\n");
		terminateAllThread();

		NILOG(PGLOG_LOG, "g_kUISound Destroy\n");
		g_kUISound.Destroy();
		NILOG(PGLOG_LOG, "g_kWeaponSoundMan Destroy\n");
		g_kWeaponSoundMan.Destroy();
		NILOG(PGLOG_LOG, "g_kSoundMan Destroy\n");
		g_kSoundMan.Destroy();
		NILOG(PGLOG_LOG, "PgAchievementTitle DestroyStatic\n");
		PgAchievementTitle::DestroyStatic();

		NILOG(PGLOG_LOG, "PgScripting Destroy\n");
		PgScripting::Terminate();
		NILOG(PGLOG_LOG, "PgXmlLoader DestroySingleton\n");
		PgXmlLoader::DestroySingleton();

		NILOG(PGLOG_LOG, "NiPhysXManager::GetPhysXManager Shutdown\n");
		NiPhysXManager::GetPhysXManager()->Shutdown();

        SAFE_DELETE(m_pkDrawWorkflow);

		if(m_spRenderer)
		{
			m_spRenderer->SetSorter( NULL );
		}

		if(m_pkShaderHelper)
		{
			NILOG(PGLOG_LOG, "m_pkShaderHelper Destroy\n");
			SAFE_DELETE_NI(m_pkShaderHelper);
		}

		NILOG(PGLOG_LOG, "g_pkTT Destroy\n");
		SAFE_DELETE_NI(g_pkTT);
		NILOG(PGLOG_LOG, "g_pkErrorCT Destroy\n");
		SAFE_DELETE(g_pkErrorCT);

		//	안해주어도 된다.
		// 	NILOG(PGLOG_LOG, "g_kTblDataMgr Destroy\n");
		// 	g_kTblDataMgr.Clear();

		if (m_hMutexHandle)
		{
			NILOG(PGLOG_LOG, "ReleaseMutex\n");
			ReleaseMutex(m_hMutexHandle);
			m_hMutexHandle = NULL;
		}

		NILOG(PGLOG_LOG, "TerminateFreedomPool\n");
		TerminateFreedomPool();

		NILOG(PGLOG_LOG, "g_kFontMgr Destroy\n");
		g_kFontMgr.Destroy();
		g_kActionTargetTransferInfoMan.Destroy();
#ifndef USE_INB
		PG_CATCH_BLOCK
#ifdef PG_EXCEPTION_ENABLED
			if(g_iUseBugTrap)
			{
				//BT_UninstallSehFilter();
			}
			MSJExceptionHandler::UninstallExceptionHandler();
#endif	
#endif //USE_INB

#ifndef EXTERNAL_RELEASE
			//if (PgActorManager::GetObjectCount() > 0)
			//{
			//	int iCount = 0;
			//	PgActorManager* pkAM = PgActorManager::GetHead();
			//	while (pkAM != NULL)
			//	{
			//		const NiFixedString& rkName = pkAM->GetKFMFileName();
			//		_PgOutputDebugString("[PgMobileSuit] Terminate - %d PgActorManager's name %s\n", iCount, rkName);
			//		pkAM = pkAM->GetNext();
			//		iCount++;
			//	}
			//}
			//if (NiGeometry::GetObjectCount() > 0)
			//{
			//	int iCount = 0;
			//	NiGeometry* pkGeom = NiGeometry::GetHead();
			//	while (pkGeom != NULL)
			//	{
			//		const NiFixedString& rkName = pkGeom->GetName();
			//		NILOG(PGLOG_ERROR, "[PgMobileSuit] Terminate - %d Geometry's name %s\n", iCount, rkName);
			//		OutputGeometryInfo(pkGeom,0);
			//		pkGeom = pkGeom->GetNext();
			//		iCount++;
			//	}
			//}
#endif

			NILOG(PGLOG_LOG, "NiApplication Terminate\n");
			NiApplication::Terminate();

			NILOG(PGLOG_LOG, "End of Terminate Application\n");
			/*
			_PgOutputDebugString("Remain Object: %d\n",NiGeometry::GetObjectCount());

			NiGeometry	*pkRemain = NiGeometry::GetHead();
			while(pkRemain)
			{
			_PgOutputDebugString("Remain Geometry : %s\n",pkRemain->GetName());

			pkRemain = pkRemain->GetNext();
			}
			*/
			TIMECAPS caps;
			if (timeGetDevCaps(&caps, sizeof(caps)) == TIMERR_NOERROR)
			{
				timeEndPeriod(caps.wPeriodMin);
			}
#ifdef USE_INB
	} catch (CSystemException* e) {
		SAFE_DELETE(e);
		::RaiseException(1, 0, 0, NULL);
	}
#endif
	g_kAppProtect.UnInit();
	BM::PgDataPackManager::Clear();
    //PROFILER_UPDATE(); // Update all profiles
    //PROFILER_OUTPUT( "profile.txt" ); // print to file.
}

bool G_D3D_Device_Reset_Notify(bool bBeforeReset,void *pvData)
{
	if(bBeforeReset)
	{
		PgRenderer	*pkPgRenderer = PgRenderer::GetPgRenderer();
		if(pkPgRenderer && pkPgRenderer->m_pkPostProcessMan)
		{
			BrightBloom	*pkBloom = (BrightBloom*)pkPgRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
			if(pkBloom)
			{
				pkBloom->ShutDown();
			}

		}
	}
	else
	{
		//	Dynamic Texture들을 유효하게 업데이트 시켜줘야한다.
		g_DynamicTextureMan.RecreateAllTextureByCopyData();
		g_kUIScene.RecreateOffscreen();

		PgRenderer	*pkPgRenderer = PgRenderer::GetPgRenderer();
		if(pkPgRenderer && pkPgRenderer->m_pkPostProcessMan)
		{
			BrightBloom	*pkBloom = (BrightBloom*)pkPgRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
			if(pkBloom)
			{
				pkBloom->Restart();
			}

		}
	}
	return true;
}

void	PgMobileSuit::RecreateRenderer(bool bFullScreen, bool bBorderless, unsigned int uiNewWidth,unsigned int uiNewHeight,unsigned int uiNewRefreshRate,bool bVSync)
{
	if ( 0 >= uiNewWidth || 0 >= uiNewHeight )
	{
		return ;
	}

	POINT2 const kPastResolution(XUIMgr.GetResolutionSize());

	if (bBorderless)
	{
		uiNewWidth = GetSystemMetrics(SM_CXSCREEN);
		uiNewHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	bool bIsResolutionChanged = !(GetStandardResolutionSize().x == uiNewWidth && GetStandardResolutionSize().y == uiNewHeight);

	if(GetFullscreen() == bFullScreen && GetVSync() == bVSync && false==bIsResolutionChanged )
	{
		return;
	}

	bool bIsFullScreenChanged = (GetFullscreen() != bFullScreen) || (GetBorderless() != bBorderless);

	if( uiNewWidth != XUI::EXV_DEFAULT_SCREEN_WIDTH || uiNewHeight != XUI::EXV_DEFAULT_SCREEN_HEIGHT )
	{
		XUIMgr.SetIsDefaultResolution(false);
	}
	
	NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(!pDX9Renderer)
	{
		return;
	}

	if(!GetAppWindow())
	{
		return;
	}

	if(uiNewWidth == 0)
	{
		uiNewWidth = GetAppWindow()->GetWidth();
		uiNewHeight = GetAppWindow()->GetHeight();
	}

	unsigned	int	uiWidth=0,uiHeight = 0,uiUseFlags = 0;
	NiWindowRef	kWndDevice=0,kWndFocus = 0;
	unsigned	int	uiAdapter = 0;
	NiDX9Renderer::DeviceDesc eDesc;
	NiDX9Renderer::FrameBufferFormat eFBFormat;
	NiDX9Renderer::DepthStencilFormat eDSFormat;
	NiDX9Renderer::PresentationInterval ePresentationInterval;
	NiDX9Renderer::SwapEffect eSwapEffect;
	unsigned	int	uiFBMode=0,uiBackBufferCount=0,uiRefreshRate = 0;


	pDX9Renderer->GetCreationParameters(uiWidth,uiHeight,uiUseFlags,kWndDevice,kWndFocus,uiAdapter,eDesc,eFBFormat,
		eDSFormat,ePresentationInterval,eSwapEffect,uiFBMode,uiBackBufferCount,uiRefreshRate);

	if(uiNewWidth != 0)
	{
		uiWidth = uiNewWidth;
		uiHeight = uiNewHeight;
	}
	if(uiNewRefreshRate != 0)
	{
		uiRefreshRate = uiNewRefreshRate;
	}

	ePresentationInterval = bVSync ? NiDX9Renderer::PRESENT_INTERVAL_ONE : NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;

	if(bFullScreen)
	{
		HWND hWnd = g_kChatWindow.GethWnd();
		if ( hWnd )
		{
			::SendMessage( hWnd, WM_DESTROY, 0, 0 );
		}
		uiUseFlags |= NiDX9Renderer::USE_FULLSCREEN;
	}
	else
	{
		uiUseFlags &= ~NiDX9Renderer::USE_FULLSCREEN;
	}

	bool bRecreated = false;
	if(bIsFullScreenChanged)
	{
		XUIMgr.SetResolutionSize( POINT2(uiWidth, uiHeight) );
		if(!bFullScreen && !bBorderless)	//먼저 한번 바꿔보고
		{
			bRecreated = true;

			AdjustWindowRectForFullScreenToWindowMode(&m_kWindowRect, &m_kClientRect, XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y);
		}
		else
		{
			GetMonitorInfo(MonitorFromWindow(GetWindowReference(), MONITOR_DEFAULTTONEAREST), &m_mi);
			//XUIMgr.SetResolutionSize( POINT2(uiWidth, uiHeight) );	//AdjustWindowStyle안에서 XUIMgr.GetResolutionSize를 사용하기 때문에 한번 저장해 주자
			AdjustWindowStyle(bFullScreen || bBorderless,uiWidth,uiHeight,!bFullScreen);
		}
	}

	if(!bFullScreen && !bBorderless)
	{
		if(false==bIsFullScreenChanged)
		{
			XUIMgr.SetResolutionSize( POINT2(uiWidth, uiHeight) );
			AdjustWindowStyle(bFullScreen,uiWidth,uiHeight,bFullScreen ? false : true);
		}
		
		// The rectangle calculation is wrong, causing the wrong UI size
		//uiWidth = m_kClientRect.right - m_kClientRect.left;
		//uiHeight = m_kClientRect.bottom - m_kClientRect.top;

		if(EXV_DEFAULT_SCREEN_WIDTH > uiWidth || EXV_DEFAULT_SCREEN_HEIGHT > uiHeight)
		{//기본해상도 보다 작으면 최소한으로 맞춰줌
			uiWidth = EXV_DEFAULT_SCREEN_WIDTH;
			uiHeight = EXV_DEFAULT_SCREEN_HEIGHT;
		}
	}
	else
	{
		SetStandardResolutionSize(POINT2(uiWidth, uiHeight));
	}

	POINT2 ptGab;
	ptGab.x = -1 * ( kPastResolution.x - uiWidth );
	ptGab.y = -1 * ( kPastResolution.y - uiHeight );	

	XUIMgr.SetResolutionSize( POINT2(uiWidth, uiHeight) );
	XUIMgr.CorrectionLocationByResolution( ptGab );// UI 위치 보정

	
	NiDX9Renderer::RecreateStatus eStatus = pDX9Renderer->Recreate(uiWidth,uiHeight,uiUseFlags,GetWindowReference(),
		eFBFormat,eDSFormat,ePresentationInterval,eSwapEffect,uiFBMode,uiBackBufferCount,uiRefreshRate);
	
	if(true==bRecreated)
	{
		AdjustWindowStyle(bFullScreen || bBorderless,uiWidth,uiHeight,!bFullScreen);
	}

	//해상도가 바뀌면 GetEffect해서 뽀샤시를 ReCreate하도록 하자!!
	if(bIsResolutionChanged || bIsFullScreenChanged)
	{
		PgRenderer* pkRenderer = g_kRenderMan.GetRenderer();
		if(pkRenderer)
		{
			BrightBloom* pkBloom = dynamic_cast<BrightBloom*>(pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM));
			if(pkBloom)
			{
				pkBloom->CleanupEffect();
				pkBloom->CreateEffect();
			}
		}

		AdjustLowHPWarnningByResolution();
	}

	SetFullscreen(bFullScreen);
	SetBorderless(bBorderless);
	SetVSync(bVSync);
	SetRefreshRate(uiRefreshRate);

}
void PgMobileSuit::RendererChange(bool bFullScreen, NiDX9Renderer::PresentationInterval eFlag)
{
	NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(!pDX9Renderer)
	{
		return;
	}
	if(!GetAppWindow())
	{
		return;
	}

	unsigned int uiNewWidth = GetAppWindow()->GetWidth();
	unsigned int uiNewHeight = GetAppWindow()->GetHeight();

	unsigned	int	uiWidth=0,uiHeight = 0,uiUseFlags = 0;
	NiWindowRef	kWndDevice=0,kWndFocus = 0;
	unsigned	int	uiAdapter = 0;
	NiDX9Renderer::DeviceDesc eDesc;
	NiDX9Renderer::FrameBufferFormat eFBFormat;
	NiDX9Renderer::DepthStencilFormat eDSFormat;
	NiDX9Renderer::PresentationInterval ePresentationInterval;
	NiDX9Renderer::SwapEffect eSwapEffect;
	unsigned	int	uiFBMode=0,uiBackBufferCount=0,uiRefreshRate = 0;


	pDX9Renderer->GetCreationParameters(uiWidth,uiHeight,uiUseFlags,kWndDevice,kWndFocus,uiAdapter,eDesc,eFBFormat,
		eDSFormat,ePresentationInterval,eSwapEffect,uiFBMode,uiBackBufferCount,uiRefreshRate);

	if(uiNewWidth != 0)
	{
		uiWidth = uiNewWidth;
		uiHeight = uiNewHeight;
	}

	if(bFullScreen)
	{
		uiUseFlags |= NiDX9Renderer::USE_FULLSCREEN;
		ePresentationInterval = eFlag;
	}
	else
	{
		uiUseFlags &= ~NiDX9Renderer::USE_FULLSCREEN;
		ePresentationInterval = eFlag;
	}

	NiDX9Renderer::RecreateStatus eStatus = pDX9Renderer->Recreate(uiWidth,uiHeight,uiUseFlags,GetWindowReference(),
		eFBFormat,eDSFormat,ePresentationInterval,eSwapEffect,uiFBMode,uiBackBufferCount,uiRefreshRate);

	AdjustWindowStyle(bFullScreen,uiWidth,uiHeight,bFullScreen ? false : true);

	SetFullscreen(bFullScreen);
	SetBorderless(false);
}
void PgMobileSuit::AdjustWindowStyle(bool bFullScreen,int iWidth,int iHeight, bool bFitToScreen)
{
	m_kWindowRect.left = m_kWindowRect.top = 0;
	m_kWindowRect.right = iWidth; 
	m_kWindowRect.bottom = iHeight;
	SetStandardResolutionSize(POINT2(iWidth, iHeight));
	
	if(!bFullScreen)
	{
		DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU;
		bool bDesktopIsSmall = false;		
		AdjustWindowRect(&m_kWindowRect, &m_kClientRect, dwWindowStyle, bDesktopIsSmall, bFitToScreen);

		SetWindowLong( GetWindowReference(), GWL_STYLE, dwWindowStyle );

		SetWindowPos(GetWindowReference(), HWND_NOTOPMOST,
			//bDesktopIsSmall ? m_kWindowRect.left : (GetSystemMetrics(SM_CXSCREEN)-m_kWindowRect.right+m_kWindowRect.left)/2,
			//bDesktopIsSmall ? m_kWindowRect.top : (GetSystemMetrics(SM_CYSCREEN)-m_kWindowRect.bottom+m_kWindowRect.top)/2,
			m_kWindowRect.left, m_kWindowRect.top,
			m_kWindowRect.right-m_kWindowRect.left,m_kWindowRect.bottom-m_kWindowRect.top,
			SWP_FRAMECHANGED);
	}
	else
	{
		DWORD	dwWindowStyle = WS_POPUP | WS_VISIBLE;
		SetWindowLong( GetWindowReference(), GWL_STYLE, dwWindowStyle );

		bool bDesktopIsSmall = false;		
		AdjustWindowRect(&m_kWindowRect, &m_kClientRect,dwWindowStyle,bDesktopIsSmall);
		SetWindowPos(GetWindowReference(),HWND_TOP,
			(GetSystemMetrics(SM_CXSCREEN)-m_kWindowRect.right+m_kWindowRect.left)/2,
			(GetSystemMetrics(SM_CYSCREEN)-m_kWindowRect.bottom+m_kWindowRect.top)/2,
			m_kWindowRect.right-m_kWindowRect.left,m_kWindowRect.bottom-m_kWindowRect.top,
			SWP_FRAMECHANGED);
	}

// 	if (m_kClientRect.right - m_kClientRect.left < XUI::EXV_DEFAULT_SCREEN_WIDTH || m_kClientRect.bottom - m_kClientRect.top < XUI::EXV_DEFAULT_SCREEN_HEIGHT)
// 	{
// 		m_bUseSmallWindow = true;
// 		m_fWindowXScale = (float)XUI::EXV_DEFAULT_SCREEN_WIDTH / (float)(m_kClientRect.right - m_kClientRect.left);
// 		m_fWindowYScale = (float)XUI::EXV_DEFAULT_SCREEN_HEIGHT / (float)(m_kClientRect.bottom - m_kClientRect.top);
// 	}
	if (m_kClientRect.right - m_kClientRect.left < XUIMgr.GetResolutionSize().x || m_kClientRect.bottom - m_kClientRect.top < XUIMgr.GetResolutionSize().y)
	{
		m_bUseSmallWindow = true;
		m_fWindowXScale = (float)XUIMgr.GetResolutionSize().x / (float)(m_kClientRect.right - m_kClientRect.left);
		m_fWindowYScale = (float)XUIMgr.GetResolutionSize().y / (float)(m_kClientRect.bottom - m_kClientRect.top);
	}
	else
	{
		m_bUseSmallWindow = false;
		m_fWindowXScale = 1.0f;
		m_fWindowYScale = 1.0f;
	}
}
bool PgMobileSuit::CreateRenderer()
{
	m_mi.cbSize = sizeof(m_mi);
	GetMonitorInfo(MonitorFromWindow(GetWindowReference(), MONITOR_DEFAULTTONEAREST), &m_mi);//랜더러가 생성되기전 바탕화면 크기 검사
	char const* pcWorkingDir = "../Shader/Generated";
	NiMaterial::SetDefaultWorkingDirectory(pcWorkingDir);

	NiWindowRef pWnd;
	if (m_bFullscreen)
		pWnd = m_pkAppWindow->GetWindowReference();
	else
		pWnd = m_pkAppWindow->GetRenderWindowReference();

	//char acErrorStr[512];

	checkAdapter();

	if (m_bFullscreen)
		m_uiBitDepth = 32;
	else
		m_uiBitDepth = 0;

	m_spRenderer = NiDX9Select::CreateRenderer(
		m_pkAppWindow->GetWindowReference(), 
		m_pkAppWindow->GetRenderWindowReference(), m_bRendererDialog, 
		m_uiBitDepth, 
		XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y,
		NiDX9Renderer::REFRESHRATE_DEFAULT,
		m_bStencil,
		m_bMultiThread, m_bNVPerfHUD ? true : m_bRefRast, m_bSWVertex, m_bNVPerfHUD,
		m_bFullscreen,m_bVSync);

	if (m_spRenderer == 0)
	{
		PgError1("DX9 Renderer Creation Failed(%s)", NiRenderer::GetLastErrorString());

		::MessageBox(NULL,_T("DX9 Renderer Initialization has been failed. You may reboot this computer, and try again."),_T("Renderer Initialization Failed."),MB_OK);
		return false;
	}
	else 
	{ 
		m_spRenderer->SetBackgroundColor(DEF_BG_COLOR);
	}

	NiDX9RendererPtr  pDX9Renderer= NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
	if(pDX9Renderer)
	{ 
		pDX9Renderer->AddResetNotificationFunc(G_D3D_Device_Reset_Notify,0);
		pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,TRUE);
	}

	//m_spRenderer->UseLegacyPipelineAsDefaultMaterial();	

	NILOG(PGLOG_LOG, "[PgMobileSuit] RendererCreated pal(%d) hwskin(%d) np2ct(%d) np2(%d) aart(%d)\n", pDX9Renderer->GetFlags() & NiRenderer::CAPS_PALETTIZED, pDX9Renderer->GetFlags() & NiRenderer::CAPS_HARDWARESKINNING, pDX9Renderer->GetFlags() & NiRenderer::CAPS_NONPOW2_CONDITIONAL_TEXT, pDX9Renderer->GetFlags() & NiRenderer::CAPS_NONPOW2_TEXT, pDX9Renderer->GetFlags() & NiRenderer::CAPS_AA_RENDERED_TEXTURES);

	return checkDeviceCaps();
}

bool PgMobileSuit::OnDefault(NiEventRef pEventRecord)
{
	PG_TRY_BLOCK
		//	PG_STAT(PgStatTimerF timer(g_kMobileSuitStatGroup.GetStatInfo("OnDefault"), GetFrameCount()));
		if (pEventRecord == NULL)
		{
			return false;
		}

		if(	PgLocalManager::Get() 
			&&	PgLocalManager::Get()->HandleWindowMessage(pEventRecord->uiMsg,pEventRecord->wParam,pEventRecord->lParam))
		{
			return	true;
		}

		//if( PgLocalManager::Get() 
		//	&& PgLocalManager::Get()->HandleGamePad(pEventRecord->uiMsg,pEventRecord->wParam,pEventRecord->lParam))
		//{
		//	return true;
		//}

		switch(pEventRecord->uiMsg)
		{
		case WM_SYSKEYDOWN:
			{
				// Alt-F4는 허용한다.
				if (pEventRecord->wParam == VK_F4)
				{
					return false;
				}
				// Alt키를 눌러서 나오는 메뉴를 땜빵으로 막는다;
				if (pEventRecord->wParam == VK_F10)
				{
					return true;
				}

				if (pEventRecord->lParam & KF_ALTDOWN << 16)
				{
					return true;
				}
			}
			break;
		case WM_SYSKEYUP:
			{
				if (pEventRecord->lParam & KF_ALTDOWN << 16)
				{
					return true;
				}
			}
			break;
		case WM_ACTIVATE:
			{
				//_PgOutputDebugString("OnDefault::WM_ACTIVATE(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);
				if (LOWORD(pEventRecord->wParam) != WA_INACTIVE)
				{
					OnActivate(true);
				}
				else
				{
					XUIMgr.ResetModifier();
				}
				return false;
			}
			break;
		case WM_ACTIVATEAPP:
			{
				//_PgOutputDebugString("OnDefault::WM_ACTIVATEAPP(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);			
				return false;
			}
			break;
		case WM_NCACTIVATE:
			{
				//_PgOutputDebugString("OnDefault::WM_NCACTIVATE(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);
				if (pEventRecord->wParam == FALSE)
				{
					OnActivate(false);
				}
				return false;
			}
			break;		
		case WM_MOVE:
			{
				WORD iX = LOWORD(pEventRecord->lParam);
				WORD iY = HIWORD(pEventRecord->lParam);
				m_kWindowPos.x = iX;
				m_kWindowPos.y = iY;
				if( g_kChatMgrClient.CheckChatOut() )
				{
					if( g_kChatWindow.IsSnap() )
					{
						RECT	rcWinP;
						GetWindowRect(g_pkApp->GetWindowReference(), &rcWinP);
						SetWindowPos(g_kChatWindow.GethWnd(), NULL, rcWinP.right, rcWinP.top, 0, 0, SWP_NOSIZE);
					}
				}
			}
			break;
		case WM_CLOSE:
			//! 이렇게 하면 FULL Screen일때 창 사이즈가 줄어들지 않는댄다.
			{
				SetForegroundWindow(GetDesktopWindow());
			}
			break;
		case WM_KEYUP:
			{
				switch(pEventRecord->wParam)
				{
				case VK_SNAPSHOT:
					{
						TakeScreenShot();
					}break;
				}
			}break;
		case WM_PG_REQUEST_INIT_SCRIPT:
			{	//// Scripting 엔진을 초기화한다.
				//doInitScript();
			}break;
		case WM_ERASEBKGND:
			{
				//화면 깜박임 방지
				if(m_bUsedEraseBackgroundMessage)
				{
					return false;
				}
				else
				{
					return true;
				}
			}break;
		case WM_KILLFOCUS:
			{
				if (::IsChild(GetWindowReference(), (HWND)(pEventRecord->wParam)))
				{
					SetFocus(GetWindowReference());
				}
			}
			break;
		}
		PG_CATCH_BLOCK
			return false;
}

void PgMobileSuit::OnActivate(bool bActivate)
{
	m_bActivated = bActivate;
	if (bActivate)
	{
		g_kSoundMan.SetBGMVolume(0.f, true);
		g_kSoundMan.SetEffectVolume(0.f, true);
		HidePointer();
		if (m_bInitialized)
		{
			setMainThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
			setBackgroundThreadPriority(THREAD_PRIORITY_NORMAL, 0);
			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 1);
			setBackgroundThreadPriority(THREAD_PRIORITY_NORMAL, 2);
			//XUIMgr.SetImeOpenStatus(false);
		}
		NILOG(PGLOG_LOG, "Application Activated\n");
	}
	else
	{
		if(PgLocalManager::Get())
		{
			PgInputSystem	*pkInputSystem = NiDynamicCast(PgInputSystem,PgLocalManager::Get()->GetInputSystem());
			if(pkInputSystem)
			{
				pkInputSystem->ReleaseAllKey();
			}
		}

		g_kSoundMan.SetBGMVolume(0.0f);
		g_kSoundMan.SetEffectVolume(0.0f);
		ShowPointer();
		if (m_bInitialized)
		{
			setMainThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 0);
			setBackgroundThreadPriority(THREAD_PRIORITY_LOWEST, 1);
			setBackgroundThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 2);
			//XUIMgr.SetImeOpenStatus(false);
		}
		NILOG(PGLOG_LOG, "Application Deactivated\n");
	}
}

bool PgMobileSuit::OnWindowResize(int iWidth, int iHeight, unsigned int uiSizeType, NiWindowRef pWnd)
{
	NiApplication::OnWindowResize(iWidth, iHeight, uiSizeType, pWnd);

	if (GetMinimized() && m_bInitialized)
	{
		setMainThreadPriority(THREAD_PRIORITY_IDLE);
		setBackgroundThreadPriority(THREAD_PRIORITY_IDLE, 0);
		setBackgroundThreadPriority(THREAD_PRIORITY_IDLE, 1);
		setBackgroundThreadPriority(THREAD_PRIORITY_IDLE, 2);
	}

	return true;
}

bool PgMobileSuit::OnWindowDestroy(NiWindowRef pWnd, bool bOption)
{
	NiApplication::OnWindowDestroy(pWnd, bOption);
	stopAllThread();
	return true;
}

void PgMobileSuit::IncFrameCount()
{
	m_iFrameCount++;
	m_iFrameCount = m_iFrameCount % 10000;	
	m_dwLastFrameTime = BM::GetTime32();
}

void PgMobileSuit::OnIdle()
{
    PROFILE_FUNC();

	PG_TRY_BLOCK

	g_kChatWindow.OnIdle();

	PG_STAT(PgStatTimerF timerBefore(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.BeforeStart"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerBefore1(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.BeforeStart1"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerBefore2(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.BeforeStart2"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerBefore3(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.BeforeStart3"), GetFrameCount()));
	PG_STAT(timerBefore.Start());
	if (!MeasureTime())
	{
		return;
	}

	{
		int const iNowTime = static_cast<int>( g_kEventView.GetLocalSecTime( CGameTime::MILLISECOND ) );
		static int iOldTime = iNowTime;
		m_dwEventViewElapsed = iNowTime - iOldTime;
		iOldTime = iNowTime;
	}

	PG_STAT(timerBefore1.Start());
	NiApplication::UpdateInput();
	PG_STAT(timerBefore1.Stop());

	ResetFrameTimings();

	PG_STAT(timerBefore2.Start());

	if(g_pkLocalManager->GetInputSystem())
	{
		PgInputSystem* pkPgInput = NiDynamicCast(PgInputSystem, PgLocalManager::Get()->GetInputSystem());
		if(NULL != pkPgInput)
		{
			if(NULL != pkPgInput->GetGamePad(0))
			{
				pkPgInput->HandleGamePad(pkPgInput->GetGamePad(0));
			}
		}

		NiInputKeyboard* pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
		if (pkKeyboard)
		{
#ifndef USE_INB
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_PAUSE))
			{
				if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_SCROLL))
					::RaiseException(1,0,0,NULL);
			}
#endif
#ifdef PG_STAT_ENABLED
			static bool bUseInterval = false;
			static float fLastIntervalTime = 0.0f;
			static float fIntervalTime = 0.5f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_F7))
			{
				bUseInterval = !bUseInterval;
			}

			if (bUseInterval)
			{
				if (g_kMobileSuitStatGroup.GetRecording())
				{
					NILOG(PGLOG_STAT, "------- End of Stat Recording(%d) -------\n", GetFrameCount());
					g_kMobileSuitStatGroup.StopRecording();
					g_kMobileSuitStatGroup.PrintStatGroup();
					g_kMobileSuitStatGroup.ResetStatGroup();
					g_kRenderManStatGroup.StopRecording();
					g_kRenderManStatGroup.PrintStatGroup();
					g_kRenderManStatGroup.ResetStatGroup();
					g_kWorldStatGroup.StopRecording();
					g_kWorldStatGroup.PrintStatGroup();
					g_kWorldStatGroup.ResetStatGroup();
					g_kPilotStatGroup.StopRecording();
					g_kPilotStatGroup.PrintStatGroup();
					g_kPilotStatGroup.ResetStatGroup();
					g_kActorStatGroup.StopRecording();
					g_kActorStatGroup.PrintStatGroup();
					g_kActorStatGroup.ResetStatGroup();
					g_kUIStatGroup.StopRecording();
					g_kUIStatGroup.PrintStatGroup();
					g_kUIStatGroup.ResetStatGroup();
					g_kScriptStatGroup.StopRecording();
					g_kScriptStatGroup.PrintStatGroup();
					g_kScriptStatGroup.ResetStatGroup();
					g_kLocalInputStatGroup.StopRecording();
					g_kLocalInputStatGroup.PrintStatGroup();
					g_kLocalInputStatGroup.ResetStatGroup();
					g_kRemoteInputStatGroup.StopRecording();
					g_kRemoteInputStatGroup.PrintStatGroup();
					g_kRemoteInputStatGroup.ResetStatGroup();
					PgNiFile::ms_kFileIOStatGroup.StopRecording();
					PgNiFile::ms_kFileIOStatGroup.PrintStatGroup();
					PgNiFile::ms_kFileIOStatGroup.ResetStatGroup();
					NILOG(PGLOG_STAT, "------- End of Stat Print -------\n");
				}
				else if (NiGetCurrentTimeInSec() + fIntervalTime > fLastIntervalTime)
				{
					fLastIntervalTime = NiGetCurrentTimeInSec();
					NILOG(PGLOG_STAT, "------- Start of Stat Recording(%d) -------\n", GetFrameCount());
					g_kMobileSuitStatGroup.StartRecording();
					g_kActorStatGroup.StartRecording();
					g_kRenderManStatGroup.StartRecording();
					g_kWorldStatGroup.StartRecording();
					g_kUIStatGroup.StartRecording();
					g_kScriptStatGroup.StartRecording();
					g_kPilotStatGroup.StartRecording();
					g_kLocalInputStatGroup.StartRecording();
					g_kRemoteInputStatGroup.StartRecording();
					PgNiFile::ms_kFileIOStatGroup.StartRecording();
				}				
			}
#endif
		}
	}
	PG_STAT(timerBefore2.Stop());

#ifndef USE_INB	
	PG_STAT(timerBefore3.Start());
	if (m_bFrameRateEnabled && m_pkFrameRate)
	{
		m_pkFrameRate->TakeSample();
		m_pkFrameRate->Update();
	}
	PG_STAT(timerBefore3.Stop());
#endif

	m_fFrameRate = 1.0f / GetFrameTime();

#if	defined(PG_STAT_ENABLED) && 0
	if (BM::GetTime32() - m_dwLastFrameTime > 1000)
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] Last Frame(%d) Freezed! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		NILOG(PGLOG_STAT, "[PgMobileSuit] Last Frame(%d) Freezed! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		PG_STAT(g_kMobileSuitStatGroup.PrintStatGroup());
		PG_STAT(g_kUIStatGroup.PrintStatGroup());
		PG_STAT(g_kRenderManStatGroup.PrintStatGroup());
		PG_STAT(g_kWorldStatGroup.PrintStatGroup());
		PG_STAT(g_kActorStatGroup.PrintStatGroup());
	}
	else if (BM::GetTime32() - m_dwLastFrameTime > 300)
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] Last Frame(%d) Laged! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		NILOG(PGLOG_STAT, "[PgMobileSuit] Last Frame(%d) Laged! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		PG_STAT(g_kMobileSuitStatGroup.PrintStatGroup());
		PG_STAT(g_kUIStatGroup.PrintStatGroup());
		PG_STAT(g_kRenderManStatGroup.PrintStatGroup());
		PG_STAT(g_kWorldStatGroup.PrintStatGroup());
		PG_STAT(g_kActorStatGroup.PrintStatGroup());
	}
	else if (BM::GetTime32() - m_dwLastFrameTime > 50)
	{
		//NILOG(PGLOG_LOG, "[PgMobileSuit] Last Frame(%d) FrameDown! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		//NILOG(PGLOG_STAT, "[PgMobileSuit] Last Frame(%d) FrameDown! %d\n", m_iFrameCount, BM::GetTime32() - m_dwLastFrameTime);
		PG_STAT(g_kMobileSuitStatGroup.PrintStatGroup());
		PG_STAT(g_kUIStatGroup.PrintStatGroup());
		PG_STAT(g_kRenderManStatGroup.PrintStatGroup());
		PG_STAT(g_kWorldStatGroup.PrintStatGroup());
		PG_STAT(g_kActorStatGroup.PrintStatGroup());
	}
#endif

	IncFrameCount();

	PG_STAT(timerBefore.Stop());

	PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.SkillOnTick"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerB(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.UpdateInput(L)"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerC(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.UpdateInput(R)"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerD(g_kMobileSuitStatGroup.GetStatInfo("OnIdle"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerE(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.RenderUpdate"), GetFrameCount()));
	PG_STAT(PgStatTimerF timerF(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.RenderDraw"), GetFrameCount()));	

	// 갱신
	BeginUpdate();
	{
		PG_STAT(timerA.Start());
		SkillOnTick(m_fAccumTime, m_fFrameTime);//xxx todo
		PG_STAT(timerA.Stop());

		//환경 사운드 업데이트
		g_kSoundMan.UpdateEnvSound(m_fFrameTime);

		PG_STAT(timerC.Start());
		if (g_pkRemoteManager)
		{
			g_pkRemoteManager->UpdateInput();
		}
		PG_STAT(timerC.Stop());
		PG_STAT(timerB.Start());
		PgIInputManager::InvalidateInputObserverContainer();
		if (g_pkLocalManager)
		{
			g_pkLocalManager->UpdateInput();
		}
		PG_STAT(timerB.Stop());
		//PG_STAT(timerD.Start());
		//g_kPilotMan.Update();
		//PG_STAT(timerD.Stop());

		// 최소 프레임 보다 떨어지면
		// 분할해서 Update을 한다. (벽 뚫기 등 방지)
		PG_STAT(timerE.Start());
#ifdef USE_PARTITION_UPDATE
		if(m_fFrameTime > PG_MIN_FRAME_RATE)
		{
			// 백업
			float fOldAccumTime = m_fAccumTime;
			float fOldFrameTime = m_fFrameTime;

			// 분할 Updating
			m_fAccumTime -= m_fFrameTime;		
			while(m_fFrameTime > PG_MIN_FRAME_RATE)
			{
				m_fFrameTime -= PG_MIN_FRAME_RATE;
				m_fAccumTime += PG_MIN_FRAME_RATE;
				g_kRenderMan.Update(m_fAccumTime, PG_MIN_FRAME_RATE);				
			}
			g_kRenderMan.Update(m_fAccumTime + m_fFrameTime, m_fFrameTime);			

			// 복구
			m_fAccumTime = fOldAccumTime;
			m_fFrameTime = fOldFrameTime;
		}
		else
#endif
		{
			g_kRenderMan.Update(m_fAccumTime, m_fFrameTime);
		}
		g_kAMPool.Update(m_fAccumTime);

		PG_STAT(timerE.Stop());
		//PgScripting::Update();
		g_kMovieMgr.Update();
		g_kHomeRenewMgr.Update(m_fAccumTime, m_fFrameTime);
		g_kChatMgrClient.Update();
		g_kBattleSquareMng.Update();
		g_kAchieveNfyMgr.Update(m_fAccumTime);
		g_klwPlayTime.Update();
	}
	EndUpdate();

	BeginRender();
	{
		PG_STAT(timerF.Start());
		if(!g_kMovieMgr.IsRenderFrame() )
		{
			g_kRenderMan.Draw(m_fAccumTime, m_fFrameTime);
		}
		PG_STAT(timerF.Stop());
		PgScripting::Update();
	}
	EndRender();

	m_DebugInfo.m_iUpdateTime = (int)(m_fUpdateTime*1000);
	m_DebugInfo.m_iRenderingTime = (int)(m_fRenderTime*1000);
	m_DebugInfo.m_iTotalUpdateTime=m_DebugInfo.m_iUpdateTime + m_DebugInfo.m_iRenderingTime;

	PG_CATCH_BLOCK
		//	_PgOutputDebugString("m_iUpdateTime : %2.5f m_iRenderingTime : %2.5f\n",m_fUpdateTime,m_fRenderTime);

		PG_TRY_BLOCK
		// 사운드
		PG_STAT(PgStatTimerF timerG(g_kMobileSuitStatGroup.GetStatInfo("OnIdle.Sound"), GetFrameCount()));
	PG_STAT(timerG.Start());
	NiMilesAudioSystem* pkSS = NiDynamicCast(NiMilesAudioSystem, NiAudioSystem::GetAudioSystem());	
	if(g_bUseSound && pkSS && pkSS->GetHWnd())
	{
		pkSS->Update(m_fAccumTime, true);
	}
	PG_STAT(timerG.Stop());

	//Global Timer Event
	g_kEventTimer.Update(m_fAccumTime, m_fFrameTime);

	// Lua Debug
	if (g_pkScriptDebug)
		g_pkScriptDebug->Dofile();

	//Disconnected From Server
	if( g_bDisconnectFlush )
	{
		OnDisconnectFlush();
	}

	if( !g_kAppProtect.CheckProcess() )
	{
		if(0 != g_pkApp->UseGameGuard())
		{
			OnDisconnectFlush();
			ExitApp();
		}
	}

#if NIMETRIC == 1
	UpdateMetrics();
#endif
	PG_CATCH_BLOCK_NO_EXIT
}

bool PgMobileSuit::createOutputModules()
{
#if NIMETRICS == 1
	// 1) Create default XML output

	// number of frames to aggregate metrics over
	const unsigned int uiFramePeriod = 1;
	// filename to save out to
	std::string Filename = GetClientName();
	Filename += "_XMLOutput.xml";

	// Add XML output module to metrics layer
	NiXMLMetricsOutput* pkXML = NiNew NiXMLMetricsOutput(uiFramePeriod, 
		Filename.c_str());
	NiMetricsLayer::AddOutputModule(pkXML);

	/*
	// 2) Create NiVisualTracker output
	float const fLeftBorder = 0.05f;
	float const fTopBorder = 0.025f;
	float const fRegionHeight = 0.25f;
	float const fRegionWidth = 0.90f;

	NiRect<float> kWindowRect1;
	kWindowRect1.m_left   = fLeftBorder;
	kWindowRect1.m_right  = kWindowRect1.m_left + fRegionWidth;
	kWindowRect1.m_top    = 0.15f;
	kWindowRect1.m_bottom = kWindowRect1.m_top + fRegionHeight;

	NiRect<float> kWindowRect2;
	kWindowRect2.m_left   = fLeftBorder;
	kWindowRect2.m_right  = kWindowRect2.m_left + fRegionWidth;
	kWindowRect2.m_top    = kWindowRect1.m_bottom + fTopBorder;
	kWindowRect2.m_bottom = kWindowRect2.m_top + fRegionHeight;

	NiRect<float> kWindowRect3;
	kWindowRect3.m_left   = fLeftBorder;
	kWindowRect3.m_right  = kWindowRect3.m_left + fRegionWidth;
	kWindowRect3.m_top    = kWindowRect2.m_bottom + fTopBorder;
	kWindowRect3.m_bottom = kWindowRect3.m_top + fRegionHeight;

	NiColor blue(0.0f, 0.0f, 1.0f);
	NiColor yellow(1.0f, 1.0f, 0.0f);
	NiColor red(1.0f, 0.0f, 0.0f);

	NiVisualTracker* pkTracker = NULL;

	// First Tracker (FPS vs Objects drawn)
	pkTracker = NiNew NiVisualTracker(m_fVTPerformanceMax ,0, kWindowRect1, 
	"Performance", true, 2);
	pkTracker->SetShow(false);

	pkTracker->AddGraph(NiNew FrameRateUpdate(&m_fFrameRate), 
	"Frame-Rate", red, 100, 0.1f, true);
	pkTracker->AddGraph(NiNew VisibleArrayUpdate(m_kVisible),
	"Vis Objects", yellow, 100, 0.1f, true);

	m_kTrackers.Add(pkTracker);

	// Second Tracker (Timing)
	pkTracker =  NiNew NiVisualTracker(m_fVTTimeMax ,0 , kWindowRect2, 
	"Time (ms)", true, 3);
	pkTracker->SetShow(false);

	pkTracker->AddGraph(NiNew GenericFloatUpdate(0.001f, &m_fUpdateTime), 
	"Update", red, 100, 0.1f, true);
	pkTracker->AddGraph(NiNew GenericFloatUpdate(0.001f, &m_fCullTime), 
	"Cull", yellow, 100, 0.1f, true);
	pkTracker->AddGraph(NiNew GenericFloatUpdate(0.001f, &m_fRenderTime), 
	"Render", blue, 100, 0.1f, true);

	m_kTrackers.Add(pkTracker);

	#ifdef NI_MEMORY_DEBUGGER
	// Third Tracker (memory)
	NiMemTracker *pkMemTracker = NiMemTracker::Get();
	if (pkMemTracker)
	{
	pkTracker = NiNew NiVisualTracker(m_fVTMemoryMax ,0 , kWindowRect3, 
	"Memory (kb)", true, 3);
	pkTracker->SetShow(false);

	pkTracker->AddGraph(NiNew MemHighWaterMarkUpdate(
	1024.0f, pkMemTracker), "High Watermark",
	red, 100, 0.1f, true);
	pkTracker->AddGraph(NiNew MemCurrentUpdate(
	1024.0f, pkMemTracker), "Current",
	yellow, 100, 0.1f, true);
	pkTracker->AddGraph(NiNew MemCurrentAllocCountUpdate(
	1.0f, pkMemTracker), "Num Allocs",
	blue, 100, 0.1f, true);

	m_kTrackers.Add(pkTracker);
	}
	#endif


	const unsigned int uiVTFramePeriod = 10;
	NiVisualTrackerOutput* pkVTOutput = NiNew NiVisualTrackerOutput(
	uiVTFramePeriod);
	NiMetricsLayer::AddOutputModule(pkVTOutput);

	// Make sure there is at least one tracker
	if (m_kTrackers.GetSize() < 1)
	return false;

	m_kTrackers.GetAt(0)->RemoveAll();

	// Add visual tracker graphs

	pkVTOutput->AddGraph(m_kTrackers.GetAt(0), 
	NiParticleMetrics::ms_acNames[
	NiParticleMetrics::UPDATED_PARTICLES],
	NiVisualTrackerOutput::FUNC_SUM_PER_FRAME, NiColor(0.0f, 1.0f, 0.0f),
	100, 0.1f, true, 1.0f, false, "Particles");

	pkVTOutput->AddGraph(m_kTrackers.GetAt(0), 
	NiApplicationMetrics::ms_acNames[
	NiApplicationMetrics::FRAMERATE],
	NiVisualTrackerOutput::FUNC_MEAN, NiColor(1.0f, 1.0f, 0.0f),
	100, 0.1f, true, 1.0f, false, "FPS");

	// Number of draw primitive calls
	pkVTOutput->AddGraph(m_kTrackers.GetAt(0), 
	NiDx9RendererMetrics::ms_acNames[
	NiDx9RendererMetrics::DRAW_PRIMITIVE],
	NiVisualTrackerOutput::FUNC_SUM_PER_FRAME, NiColor(1.0f, 0.0f, 0.0f),
	100, 0.1f, true, 2.0f, false, "DrawCalls/2");
	*/

#endif //#if NIMETRICS == 1

	return true;
}

int PgMobileSuit::GetConfigFromFileInt(LPCWSTR const& wstrFindSection, LPCWSTR const& wstrFindElement, BM::vstring& vstrResult, BM::vstring const& vstrDefault, std::wstring const& wstrFindSource)
{
#ifdef USE_INB	//INB
	return BM::GetFromINB(wstrFindSection, wstrFindElement, vstrResult, vstrDefault, wstrFindSource);
#else	//INI
	if( 0 != vstrResult.size())
	{
		vstrResult = _T("");//이전에 썼으면 비우기
	}
	return ::GetPrivateProfileInt(wstrFindSection, wstrFindElement, vstrDefault, m_szConfigFile);
#endif
	//error
	NILOG(PGLOG_ERROR, "[PgMobileSuit] Faild From GetConfigFromFileInt() \n");
	return 0;
}

int PgMobileSuit::GetConfigFromFileStr(LPCWSTR const& wstrFindSection, LPCWSTR const& wstrFindElement, LPCWSTR const& wstrDefault, LPWSTR& wstrResult, size_t iResultBufferSize, std::wstring const& wstrFindSource)
{
#ifdef USE_INB	//INB
	::memset(wstrResult, 0, iResultBufferSize);
	BM::vstring vstrResult;
	BM::GetFromINB(wstrFindSection, wstrFindElement, vstrResult, BM::vstring(wstrDefault), wstrFindSource);
	if(vstrResult.size())
	{
		::wcsncpy_s(wstrResult, iResultBufferSize, (wchar_t const*)vstrResult, __min(vstrResult.size(), iResultBufferSize - 1));
	}
	return vstrResult.size();
#else	//INI
	return ::GetPrivateProfileString(wstrFindSection, wstrFindElement, wstrDefault, wstrResult, iResultBufferSize, m_szConfigFile);
#endif
	//error
	NILOG(PGLOG_ERROR, "[PgMobileSuit] Faild From GetConfigFromFileStr() \n");
	return 0;
}

void PgMobileSuit::readConfigFile()
{
	int const iDefaultUsePack = 
#ifdef EXTERNAL_RELEASE
		1;
#else
		0;
#endif
	int const iDefaultUseGSM = 
#if defined(USE_INB)
		1;
#else
		0;
#endif
	int const iDefaultUseGameGuard = iDefaultUseGSM;
	std::wstring const kDefaultInitGameGuardLib(L"Dragonica");
	bool bUseModuleCheck = false;
	bool bUseSelfKill = false;

	std::vector<char> vecConfigData;
	std::string strConfig;

#ifdef USE_INB	//INB
	BM::DecLoad(_T("../Config.inb"), vecConfigData);

	if(vecConfigData.size())
	{//INB 사용
		vecConfigData.push_back('\0');//바이너리 받아올 때 널터미네이트가 안붙음
		strConfig = std::string(&vecConfigData.at(0));
	}
	else
	{
		NILOG(PGLOG_ERROR, "[PgMobileSuit] Can't load Config.inb\n");
		return;
	}
#endif

	std::wstring const kCfgData(UNI(strConfig.c_str()));
	BM::vstring vstrResult;


#ifdef USE_INB	//INB
	UseGameGuard(1);
#else
	//INI
	UseGameGuard(::GetPrivateProfileInt(_T("Game"), _T("UseGameGuard"), iDefaultUseGameGuard, m_szConfigFile));
	m_iFullScreenIni =			GetConfigFromFileInt(_T("GAME"), _T("FullScreen"), vstrResult, BM::vstring(-1), kCfgData);
	m_bRendererDialog =			GetConfigFromFileInt(_T("GAME"), _T("SelectDialog"), vstrResult, BM::vstring(0), kCfgData);
	m_bStencil =				GetConfigFromFileInt(_T("GAME"), _T("Stencil"), vstrResult, BM::vstring(1), kCfgData);
	g_bUseLoadGsa2 =			GetConfigFromFileInt(_T("GAME"), _T("UseLoadGsa2"), vstrResult, BM::vstring(0), kCfgData);	
	g_bFindFirstLocal =			GetConfigFromFileInt(_T("GAME"), _T("UseFindFirstLocal"), vstrResult, BM::vstring(0), kCfgData);	
#endif
	g_bUsePackData =			GetConfigFromFileInt(_T("GAME"), _T("UsePack"), vstrResult, BM::vstring(iDefaultUsePack), kCfgData);
	g_bUseExtraPackData =		GetConfigFromFileInt(_T("GAME"), _T("UseExtraPack"), vstrResult, BM::vstring(iDefaultUsePack), kCfgData);
	g_bUseVariableActorUpdate =	GetConfigFromFileInt(_T("GAME"), _T("UseVariableActorUpdate"), vstrResult, BM::vstring(0), kCfgData);
	g_bUseSound =				GetConfigFromFileInt(_T("GAME"), _T("Sound"), vstrResult, BM::vstring(1), kCfgData);
	bUseModuleCheck =			GetConfigFromFileInt(_T("GAME"), _T("UseModuleCheck"), vstrResult, BM::vstring(1), kCfgData);
	bUseSelfKill =				GetConfigFromFileInt(_T("GAME"), _T("UseSelfKill"), vstrResult, BM::vstring(1), kCfgData);
	g_iSyncPosSpeed =			GetConfigFromFileInt(_T("GAME"), _T("SyncPosSpeed"), vstrResult, BM::vstring(150), kCfgData);
	g_bUseAMPool =				GetConfigFromFileInt(_T("GAME"), _T("UseAMPool"), vstrResult, BM::vstring(1), kCfgData);
	g_iUseAddUnitThread =		GetConfigFromFileInt(_T("GAME"), _T("UseAddUnitThread"), vstrResult, BM::vstring(1), kCfgData);
	m_iMaxFrameRate =			GetConfigFromFileInt(_T("GAME"), _T("MaxFrameRate"), vstrResult, BM::vstring(60), kCfgData);
	g_bUseDirectInput =			GetConfigFromFileInt(_T("GAME"), _T("UseDirectInput"), vstrResult, BM::vstring(0), kCfgData);
	g_bUseNewClone =			GetConfigFromFileInt(_T("GAME"), _T("UseNewClone"), vstrResult,BM::vstring(1), kCfgData);
	g_bSendSmallAreaInfo =		GetConfigFromFileInt(_T("GAME"), _T("SendSmallArea"), vstrResult,BM::vstring(1), kCfgData);
	g_bDrawCircleShadow =		GetConfigFromFileInt(_T("GAME"), _T("DrawCircleShadow"), vstrResult,BM::vstring(1), kCfgData);
	g_bUseBatchRender =			GetConfigFromFileInt(_T("GAME"), _T("UseBatchRender"), vstrResult,BM::vstring(1), kCfgData);
	g_bUseMTSGU =				GetConfigFromFileInt(_T("GAME"), _T("UseMTSGU"), vstrResult,BM::vstring(0), kCfgData);
	g_bUseColorShadow = ( 0 == GetConfigFromFileInt(_T("GAME"), _T("ColorShadowOff"), vstrResult, BM::vstring(0), kCfgData));
	g_bUseActorControllerUpdateOptimize = GetConfigFromFileInt(_T("GAME"), _T("UseActorControllerUpdateOptimize"), vstrResult,BM::vstring(1), kCfgData);
	g_bUseProjectionShadow = GetConfigFromFileInt(_T("GAME"), _T("UseProjectionShadow"), vstrResult,BM::vstring(0), kCfgData);
	g_bUseProjectionLight =	GetConfigFromFileInt(_T("GAME"), _T("UseProjectionLight"), vstrResult,BM::vstring(0), kCfgData);
	g_bUseUIOffscreenRendering = GetConfigFromFileInt(_T("Game"), _T("UseUIOffscreenRendering"), vstrResult, BM::vstring(1), kCfgData);
	g_bUseCoupleUI = GetConfigFromFileInt(_T("Game"), _T("UseCoupleUI"), vstrResult, BM::vstring(1), kCfgData);
	g_iSideJobBoardMoveCost =	GetConfigFromFileInt(_T("Game"), _T("SideJobBoardMoveCost"), vstrResult, BM::vstring(50), kCfgData);
	g_bUseLoginFailExit = GetConfigFromFileInt(_T("Game"), _T("UseLoginFailExit"), vstrResult, BM::vstring(1), kCfgData);
	int F7DamageHideStage = 0;
	F7DamageHideStage = GetConfigFromFileInt(_T("Game"), _T("F7DamageHideStage"), vstrResult, BM::vstring(0), kCfgData);
#ifndef EXTERNAL_RELEASE
	g_bUseImportActionData =  GetConfigFromFileInt(_T("Game"), _T("UseImportActionData"), vstrResult, BM::vstring(0), kCfgData);
#endif
	if( F7DamageHideStage )
	{ // F7 스테이지1의 번호가 0부터 시작하기 때문에 -1을 해준다.
		g_iF7DamageHideStage = F7DamageHideStage - 1;
	}

	WCHAR wszFinishLinkURL[1024] = {0,};
	LPWSTR lpwstrFinishLinkURL = wszFinishLinkURL;
	GetConfigFromFileStr(_T("GAME"), _T("FinishLinkURL"), _T(""),	lpwstrFinishLinkURL, sizeof(wszFinishLinkURL), kCfgData);
	g_strFinishLinkURL = (std::wstring)lpwstrFinishLinkURL;

	WCHAR wszCashChargeUrl[1024] = {0,};
	LPWSTR lpwszCashChargeUrl = wszCashChargeUrl;
	GetConfigFromFileStr(_T("GAME"), _T("CashChargeURL"), _T(""),	lpwszCashChargeUrl, sizeof(wszCashChargeUrl), kCfgData);
	m_kCashChargeUrl = (std::wstring)lpwszCashChargeUrl;

	for (int i = 0; i < MAX_SITE_NUM; i++)
	{
		WCHAR szSiteCashChargeUrl[1024] = {0, };
		LPWSTR lpwszSiteCashChargeUrl = szSiteCashChargeUrl;
		BM::vstring	vSiteCashChargeUrl(L"CashChargeURL_");
		vSiteCashChargeUrl += i;

		GetConfigFromFileStr( _T("GAME"), vSiteCashChargeUrl, _T(""), lpwszSiteCashChargeUrl, sizeof(szSiteCashChargeUrl), kCfgData);

		if (szSiteCashChargeUrl[0] != L'\0')
		{
			m_kCashChargeUrlMap.insert(std::make_pair(i, std::wstring(lpwszSiteCashChargeUrl)));
		}
	}

	for (int i = 0; i < MAX_SITE_NUM; i++)
	{
		WCHAR szSiteRankUrl[1024] = {0, };
		LPWSTR lpwszSiteRankUrl = szSiteRankUrl;
		BM::vstring	vSiteRankUrl(L"SITE_RANK_URL_");
		vSiteRankUrl += i;

		GetConfigFromFileStr( _T("SERVER"), vSiteRankUrl, _T(""), lpwszSiteRankUrl, sizeof(szSiteRankUrl), kCfgData);

		if (szSiteRankUrl[0] != L'\0')
		{
			m_kRankUrlMap.insert(std::make_pair(i, std::wstring(lpwszSiteRankUrl)));
		}
	}	

	g_iUseBugTrap =				GetConfigFromFileInt(_T("GAME"), _T("UseBugTrap"), vstrResult, BM::vstring(0), kCfgData);
	g_nDumpPort =				GetConfigFromFileInt(_T("GAME"), _T("BugTrapPort"), vstrResult, BM::vstring(9999), kCfgData);
	LPWSTR lpszDumpAddr = g_szDumpAddr;
	GetConfigFromFileStr(_T("GAME"), _T("BugTrapIP"), _T("125.131.115.181"),	lpszDumpAddr, sizeof(g_szDumpAddr), kCfgData);
	_tcsncpy(g_szDumpAddr, std::wstring(lpszDumpAddr).c_str(), 40);
	LPWSTR lpszDumpAppNamer = g_szDumpAppName;
	GetConfigFromFileStr(_T("GAME"), _T("BugTrapAppName"), _T("Dragonica_Default"), lpszDumpAppNamer, sizeof(g_szDumpAppName), kCfgData);
	_tcsncpy(g_szDumpAppName, std::wstring(lpszDumpAppNamer).c_str(), 40);
	g_iBugTrapScreenShot =		GetConfigFromFileInt(_T("GAME"), _T("BugTrapScreenShot"), vstrResult, BM::vstring(0), kCfgData);

	UseGSM((GetConfigFromFileInt(_T("Game"), _T("UseGSM"), vstrResult, BM::vstring(iDefaultUseGSM), kCfgData)?(true):(false)));
	UseWebLinkage((GetConfigFromFileInt(_T("Game"), _T("UseWebLinkage"), vstrResult, BM::vstring(0), kCfgData)));

	g_fWeaponSize =	GetConfigFromFileInt(_T("GAME"), _T("WeaponSize"), vstrResult, BM::vstring(ABILITY_RATE_VALUE_FLOAT), kCfgData) / ABILITY_RATE_VALUE_FLOAT;

	WCHAR szInitGameGuardLib[1024] = {0, };
	LPWSTR lpszInitGameGuardLib = szInitGameGuardLib;
	GetConfigFromFileStr( _T("Game"), _T("InitGameGuardLib"), kDefaultInitGameGuardLib.c_str(), lpszInitGameGuardLib, sizeof(szInitGameGuardLib), kCfgData);
	g_kAppProtect.ServiceLib( lpszInitGameGuardLib );

	WCHAR szServerIP[40] = {0,};
	LPWSTR lpszServerIP = szServerIP;
	GetConfigFromFileStr(_T("SERVER"), _T("IP"), _T(""), lpszServerIP, sizeof(szServerIP), kCfgData);
	_tcsncpy(m_szServerIP, std::wstring(lpszServerIP).c_str(), 40);
	m_wServerPort =				GetConfigFromFileInt( _T("SERVER"), _T("PORT"), vstrResult, BM::vstring(0), kCfgData);

	WCHAR szServerRankURL[260] = {0,};
	LPWSTR lpszServerRankURL = szServerRankURL;
	GetConfigFromFileStr( _T("SERVER"), _T("RANK_URL"), _T(""), lpszServerRankURL, sizeof(PgRankingMgr::ms_szURL), kCfgData);
	_tcscpy_s(PgRankingMgr::ms_szURL, MAX_PATH - 1, lpszServerRankURL);

	WCHAR szSupergirlURL[512] = {0,};
	LPWSTR lpszSupergirlURL = szSupergirlURL;
	GetConfigFromFileStr(_T("SERVER"), _T("SUPERGIRL_URL"), _T(""), lpszSupergirlURL, sizeof(szSupergirlURL), kCfgData);
	_tcscpy_s(g_szSuperGirlURL, 512 - 1, lpszSupergirlURL);

	WCHAR szMemberPassportURL[260] = {0,};
	LPWSTR lpszMemberPassportURL = szMemberPassportURL;
	GetConfigFromFileStr( _T("SERVER"), _T("PASSPORT_URL"), _T(""), lpszMemberPassportURL, sizeof(lwPlayTime::ms_szURL), kCfgData);
	_tcscpy_s(lwPlayTime::ms_szURL, MAX_PATH - 1, lpszMemberPassportURL);

	WCHAR szLocalInfoServiceRegion[1024] = {0,};
	LPWSTR lpszLocalInfoServiceRegion = szLocalInfoServiceRegion;
	GetConfigFromFileStr(_T("LOCAL_INFO"), _T("SERVICE_REGION"), _T(""), lpszLocalInfoServiceRegion, sizeof(szLocalInfoServiceRegion), kCfgData);

	std::wstring strResult = (std::wstring)lpszLocalInfoServiceRegion;
	UPR(strResult);
	XUIMgr.LocalName(strResult);

	for (int i = 0; i < MAX_SITE_NUM; i++)
	{
		TCHAR szSiteRankUrl[1024] = {0, };
		BM::vstring	vSiteRankUrl(L"SITE_RANK_URL_");
		vSiteRankUrl += i;

		::GetPrivateProfileString( _T("SERVER"), vSiteRankUrl, _T(""), szSiteRankUrl, sizeof(szSiteRankUrl) - 1, m_szConfigFile);

		if (szSiteRankUrl[0] != L'\0')
		{
			m_kRankUrlMap.insert(std::make_pair(i, std::wstring(szSiteRankUrl)));
		}
	}

#ifndef USE_INB	//INB
	m_bFrameRateEnabled =		GetConfigFromFileInt(_T("DEBUG"), _T("FrameRate"), vstrResult, BM::vstring(0), kCfgData);
	m_bNVPerfHUD =				GetConfigFromFileInt(_T("DEBUG"), _T("NVPerfHUD"), vstrResult, BM::vstring(0), kCfgData);
	m_bVisibleClassNo			= GetConfigFromFileInt(_T("Debug"), _T("VisibleClassNo"), vstrResult, BM::vstring(0), kCfgData);
	g_iOutPutDebugLevel			= ::GetPrivateProfileInt(_T("Debug"), _T("OutPutDebugLevel"), 0, m_szConfigFile);
	m_bVisibleTextDebug			= (::GetPrivateProfileInt(_T("Debug"), _T("TextDebug"), 0, m_szConfigFile) != 0) ? true: false;
	m_bVisibleQTextDebug		= (::GetPrivateProfileInt(_T("Debug"), _T("QuestTextDebug"), 0, m_szConfigFile) != 0) ? true: false;
	m_bMultiThread =			GetConfigFromFileInt(_T("DEBUG"), _T("DXMultiThread"), vstrResult, BM::vstring(1), kCfgData);
	m_bSingleMode				= GetConfigFromFileInt(_T("Debug"), _T("SingleMode"), vstrResult, BM::vstring(0), kCfgData);
	g_bUseDebugInfo				= GetConfigFromFileInt(_T("Debug"), _T("PhysXDebug"), vstrResult, BM::vstring(0), kCfgData);
	g_bShowCharacterPosition	= GetConfigFromFileInt(_T("Debug"), _T("ShowCharacterPosition"), vstrResult, BM::vstring(0), kCfgData);
	g_bShowChannelInfo			= GetConfigFromFileInt(_T("Debug"), _T("ShowChannel"), vstrResult, BM::vstring(0), kCfgData);
	m_iWorkerThreadCount		= GetConfigFromFileInt(_T("Debug"), _T("WorkerThreadCount"), vstrResult, BM::vstring(1), kCfgData);
	m_iMemoryWorkerThreadCount	= GetConfigFromFileInt(_T("Debug"), _T("MemoryWorkerThreadCount"), vstrResult, BM::vstring(1), kCfgData);
	g_iUseDebugConsole			= GetConfigFromFileInt(_T("Debug"), _T("UseDebugConsole"), vstrResult, BM::vstring(0), kCfgData);
#endif	// USE_INB	//INB
	PgActorManager::SetUseNewClone(g_bUseNewClone);
}

void PgMobileSuit::SetInterLoginIP()
{
	//internal
	m_LoginIPCont.insert(L"172.31.208.58");
	m_LoginIPCont.insert(L"172.31.208.31");
	m_LoginIPCont.insert(L"172.31.208.32");
	m_LoginIPCont.insert(L"172.31.208.33");
	m_LoginIPCont.insert(L"172.31.208.34");
	m_LoginIPCont.insert(L"172.31.208.201");
	m_LoginIPCont.insert(L"172.31.208.202");
	m_LoginIPCont.insert(L"172.31.208.203");
	m_LoginIPCont.insert(L"172.31.208.204");
	m_LoginIPCont.insert(L"172.31.208.205");
	m_LoginIPCont.insert(L"172.31.208.206");
	m_LoginIPCont.insert(L"172.31.208.207");
	m_LoginIPCont.insert(L"172.31.208.208");
	m_LoginIPCont.insert(L"172.31.208.184");
	m_LoginIPCont.insert(L"172.31.208.185");
	m_LoginIPCont.insert(L"127.0.0.1");
	m_LoginIPCont.insert(L"192.168.200.34");
	m_LoginIPCont.insert(L"192.168.200.35");
	m_LoginIPCont.insert(L"112.136.175.51");
	m_LoginIPCont.insert(L"112.136.175.5");
	m_LoginIPCont.insert(L"112.136.175.34");
	m_LoginIPCont.insert(L"172.31.208.58");
}

void PgMobileSuit::SetLoginIP()
{
	LOCAL_MGR::NATION_CODE const code = (LOCAL_MGR::NATION_CODE)g_kLocal.ServiceRegion();

	switch(code)
	{
	case LOCAL_MGR::NC_KOREA:
		{
			SetInterLoginIP();
		}break;
	case LOCAL_MGR::NC_CHINA:
		{
			SetInterLoginIP();
			//심의
			m_LoginIPCont.insert(L"118.144.76.222");
			m_LoginIPCont.insert(L"118.144.76.223");
			//TEST
			m_LoginIPCont.insert(L"118.144.76.215");
			m_LoginIPCont.insert(L"118.144.76.217");
			m_LoginIPCont.insert(L"118.144.76.218");
			m_LoginIPCont.insert(L"118.144.76.225");
			m_LoginIPCont.insert(L"125.32.54.71");

			//CBT
			m_LoginIPCont.insert(L"118.145.28.171");
			m_LoginIPCont.insert(L"118.145.28.172");
			m_LoginIPCont.insert(L"118.145.28.173");
			m_LoginIPCont.insert(L"118.145.28.174");
			m_LoginIPCont.insert(L"118.145.28.175");
			m_LoginIPCont.insert(L"118.145.28.176");
			m_LoginIPCont.insert(L"118.145.28.177");
			m_LoginIPCont.insert(L"118.145.28.178");
			m_LoginIPCont.insert(L"118.145.28.179");
			m_LoginIPCont.insert(L"118.145.28.180");
			m_LoginIPCont.insert(L"118.145.28.181");
			m_LoginIPCont.insert(L"118.145.28.182");

			m_LoginIPCont.insert(L"58.215.50.111");
			m_LoginIPCont.insert(L"58.215.50.112");
			m_LoginIPCont.insert(L"58.215.50.113");
			m_LoginIPCont.insert(L"58.215.50.114");
			m_LoginIPCont.insert(L"58.215.50.115");

			m_LoginIPCont.insert(L"117.34.10.15");
			m_LoginIPCont.insert(L"117.34.10.16");
			m_LoginIPCont.insert(L"117.34.10.17");
			m_LoginIPCont.insert(L"117.34.10.18");
			m_LoginIPCont.insert(L"117.34.10.19");

			m_LoginIPCont.insert(L"119.167.247.177");
			m_LoginIPCont.insert(L"119.167.247.178");
			m_LoginIPCont.insert(L"119.167.247.179");
			m_LoginIPCont.insert(L"119.167.247.180");
			m_LoginIPCont.insert(L"119.167.247.181");
			m_LoginIPCont.insert(L"114.111.162.163");	//test
		}break;
	case LOCAL_MGR::NC_TAIWAN:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"210.242.203.3");		//tw
			m_LoginIPCont.insert(L"63.216.196.244");	//hk
			m_LoginIPCont.insert(L"210.242.203.5");		//test
			m_LoginIPCont.insert(L"210.242.203.9");		//test

			//추가,변경
			m_LoginIPCont.insert(L"192.168.45.91");		//test
			m_LoginIPCont.insert(L"218.32.164.57");		//test
			m_LoginIPCont.insert(L"192.168.45.1");		//test
			m_LoginIPCont.insert(L"218.32.164.63");		//test
			m_LoginIPCont.insert(L"192.168.45.11");		//r1
			m_LoginIPCont.insert(L"218.32.164.62");		//r1
		}break;
	case LOCAL_MGR::NC_THAILAND:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"61.91.123.197");	//live
			m_LoginIPCont.insert(L"61.91.123.227");	//test
			m_LoginIPCont.insert(L"61.91.123.230"); //test
			m_LoginIPCont.insert(L"61.91.123.232");
		}break;
	case LOCAL_MGR::NC_USA:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"172.16.0.12");	//live
			m_LoginIPCont.insert(L"128.241.92.225");	//test
		}break;
	case LOCAL_MGR::NC_JAPAN:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"59.128.94.75");	//live
			m_LoginIPCont.insert(L"59.128.94.95");	//old test
			m_LoginIPCont.insert(L"59.128.94.68");	//new origin test
			//가상화서버 IP
			m_LoginIPCont.insert(L"59.128.93.75");	//live
			m_LoginIPCont.insert(L"59.128.93.95");	//old test
			m_LoginIPCont.insert(L"59.128.93.68");	//new origin test
		}break;
	case LOCAL_MGR::NC_SINGAPORE:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"122.50.2.133");	//live
			m_LoginIPCont.insert(L"122.50.2.170");	//test

			//싱가폴에서 말레이시아 관리 함
			m_LoginIPCont.insert(L"112.137.175.71");	//live
			m_LoginIPCont.insert(L"112.137.175.111");	//test
		}break;
	case LOCAL_MGR::NC_INDONESIA:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"202.158.252.5");		//old live
			m_LoginIPCont.insert(L"202.158.252.62");	//test
			m_LoginIPCont.insert(L"202.158.252.60");		//test2
		}break;
	case LOCAL_MGR::NC_VIETNAM:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"205.252.216.33");	//live
			m_LoginIPCont.insert(L"180.148.138.114");	//test
			//리런칭용
			m_LoginIPCont.insert(L"111.65.253.1");	//live
		}break;
	case LOCAL_MGR::NC_RUSSIA:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"217.197.124.170");	//live
			m_LoginIPCont.insert(L"195.128.95.42");		//test
			m_LoginIPCont.insert(L"217.197.124.168");	//test2
		}break;
	case LOCAL_MGR::NC_ENGLAND:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"195.59.139.50");		//live
			m_LoginIPCont.insert(L"87.198.181.223");	//test
			m_LoginIPCont.insert(L"87.198.181.221");	//test
		}break;
	case LOCAL_MGR::NC_GERMANY:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"195.59.139.100");	//live
			m_LoginIPCont.insert(L"87.198.181.222");	//test
			m_LoginIPCont.insert(L"87.198.181.221");	//test
		}break;
	case LOCAL_MGR::NC_FRANCE:
		{
			SetInterLoginIP();
			m_LoginIPCont.insert(L"195.59.139.70");		//live
			m_LoginIPCont.insert(L"87.198.181.224");	//test
			m_LoginIPCont.insert(L"87.198.181.221");	//test
		}break;
	case LOCAL_MGR::NC_EU:
		{
			SetInterLoginIP();
			// 영국
			m_LoginIPCont.insert(L"195.59.139.50");		//live
			m_LoginIPCont.insert(L"87.198.181.223");	//test
			// 독일
			m_LoginIPCont.insert(L"195.59.139.100");	//live
			m_LoginIPCont.insert(L"87.198.181.222");	//test
			// 프랑스
			m_LoginIPCont.insert(L"195.59.139.70");		//live
			m_LoginIPCont.insert(L"87.198.181.224");	//test

			m_LoginIPCont.insert(L"87.198.181.225");	//new origin test
			m_LoginIPCont.insert(L"87.198.181.221");
		}break;
	default:
		{
		}break;
	}
}

bool PgMobileSuit::CheckLoginIP(std::wstring const& ServerIP)const
{
	if(m_LoginIPCont.end() == m_LoginIPCont.find(ServerIP))
	{
		ExitApp();	
		return false;
	}
	return true;
}

void PgMobileSuit::SetTitleByConfig()
{
	wchar_t title[MAX_PATH];
	GetWindowTextW(GetWindowReference(), title, MAX_PATH);
	//AddStringToWindowTitle(title, 0);
#ifndef EXTERNAL_RELEASE
	std::wstring wstrPacketVer = L"; PacketVer(";
	wstrPacketVer += PACKET_VERSION_C;
	wstrPacketVer += L"/";
	wstrPacketVer += PACKET_VERSION_S;
	wstrPacketVer += L")";
	AddStringToWindowTitle(wstrPacketVer.c_str(), 4);

	BM::vstring windowTitle;

	if (g_bUsePackData && g_bUseExtraPackData)
	{
		windowTitle += TEXT(" UsePack+ ");
	}
	else if (g_bUsePackData)
	{
		windowTitle += TEXT(" UsePack ");
	}

	if (g_iUseAddUnitThread > 0)
	{
		g_bUseAMPool = false;
	}

	if (m_bSingleMode)
	{
		g_iUseAddUnitThread = 0;
		windowTitle += TEXT(" Single ");
	}

	m_iWorkerThreadCount = __max(1, m_iWorkerThreadCount);
	m_iMemoryWorkerThreadCount = __max(1, m_iMemoryWorkerThreadCount);

	if (windowTitle.size() > 0)
	{
		AddStringToWindowTitle(windowTitle, 1);
	}

	BM::vstring ip_port;
	ip_port += TEXT(" ");
	ip_port += m_szServerIP;
	ip_port += TEXT(":");
	ip_port += m_wServerPort;
	ip_port += TEXT(" ");

	AddStringToWindowTitle(ip_port, 2);
#endif	// #ifndef EXTERNAL_RELEASE
	std::wstring kVerString;
	PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("ClientVersionString"), kVerString, _T("Client Ver : "));
	BM::vstring	vPatchVer(L" (");
	vPatchVer += kVerString;
	vPatchVer += L" ";
	vPatchVer += m_kPatchVer.Version.i16Major;
	vPatchVer += L".";
	vPatchVer += m_kPatchVer.Version.i16Minor;
	vPatchVer += L".";
	vPatchVer += m_kPatchVer.Version.i32Tiny;
	vPatchVer += L") ";
	AddStringToWindowTitle(vPatchVer, 5);

	if (g_kCmdLineParse.SiteName().size() >= 1)
	{
		std::wstring kSiteString;
		PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("SiteNameString"), kSiteString, _T("Site : "));
		BM::vstring	vSiteName(L" (");
		vSiteName += kSiteString;
		vSiteName += L" ";
		vSiteName += g_kCmdLineParse.SiteName();
		vSiteName += L") ";

		AddStringToWindowTitle(vSiteName, 6);
	}
}

void PgMobileSuit::writeConfigFile()
{

}

void PgMobileSuit::checkAdapter()
{
	const NiDX9SystemDesc* pkInfo = NiDX9Renderer::GetSystemDesc();
	if (pkInfo == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgMobileSuit] Can't Get System Description\n");
		return;
	}

	unsigned int uiAdapterCount = pkInfo->GetAdapterCount();
	NILOG(PGLOG_MINOR, "[PgMobileSuit] AdapterCount : %d\n", uiAdapterCount);

	bool bAdaptersExist = false;
	for (unsigned int i = 0; i < uiAdapterCount; i++)
	{
		const NiDX9AdapterDesc* pkAdapter = pkInfo->GetAdapter(i);
		if (pkAdapter == NULL)
		{
			continue;
		}

		NILOG(PGLOG_MINOR, "[PgMobileSuit] %d adpater - %s\n", i, pkAdapter->GetStringDescription());
		const NiDX9DeviceDesc* pkDevice = pkAdapter->GetDevice(D3DDEVTYPE_HAL);
		if (!pkDevice)
		{
			NILOG(PGLOG_ERROR, "[PgMobileSuit] Can't Get HAL Device from %d adpater, \n", i);
			continue;
		}

		// we skip fullscreen - only devices unless we are looking
		// at fullscreen mode.  Also, we skip SW devices for now.
		// Later, we will add just the primary device as a SW device
		if (!m_bFullscreen)
		{
			// test if device can render to window in the current display mode
			if (!pkAdapter->CanDeviceRenderWindowed(D3DDEVTYPE_HAL))
			{
				NILOG(PGLOG_ERROR, "[PgMobileSuit] Can't Render Windowed at %d adpater, \n", i);
				continue;
			}
		}
		else
		{
			unsigned int uiModeCount = pkAdapter->GetModeCount();
			NILOG(PGLOG_MINOR, "[PgMobileSuit] %d Adapter has %d mode, \n", i, uiModeCount);

			int i16Default = -1;
			int i32Default = -1;
			bool bResolutionsExist = false;

			for (unsigned int j = 0; j < uiModeCount; j++)
			{
				const NiDX9AdapterDesc::ModeDesc* pkMode = pkAdapter->GetMode(j);

 				//if(pkMode == NULL || (pkMode->m_uiWidth != XUI::EXV_DEFAULT_SCREEN_WIDTH && pkMode->m_uiHeight != XUI::EXV_DEFAULT_SCREEN_HEIGHT))
 				//{
 				//	continue;
 				//}

				if(pkMode == NULL || (pkMode->m_uiWidth != XUIMgr.GetResolutionSize().x && pkMode->m_uiHeight != XUIMgr.GetResolutionSize().y))
				{
					continue;
				}

				if(pkMode == NULL)
				{
					continue;
				}

				NILOG(PGLOG_MINOR, "[PgMobileSuit] %d Adapter %d mode: %u x %u x %u bpp \n", i, j, pkMode->m_uiWidth, pkMode->m_uiHeight, pkMode->m_uiBPP);
				bResolutionsExist = true;
			}
		}


		bAdaptersExist = true;
	}
}

bool PgMobileSuit::checkDeviceCaps()
{
	NiDX9Renderer* renderer = NiDX9Renderer::GetRenderer();
	if (renderer == NULL)
		return false;

	NILOG(PGLOG_LOG, "Start CheckComputer\n");
	PgComputerInfo::CheckComputerInfo(renderer->GetDirect3D());
	NILOG(PGLOG_LOG, "CPU : %dMHz(%d), RAM : %dMB, Graphic %s(%s), VRAM %dMB\n", PgComputerInfo::GetCPUInfo().iMhz, PgComputerInfo::GetCPUInfo().iNumProcess, PgComputerInfo::GetCPUInfo().iRam, PgComputerInfo::GetGPUInfo().strName, PgComputerInfo::GetGPUInfo().strVersion, PgComputerInfo::GetGPUInfo().iVRam);
	NILOG(PGLOG_LOG, "End CheckComputer Initialization\n");

	int iGraphicModeSet = 0;
	if( -1 == g_kCmdLineParse.GraphicMode() )
	{
		/*if( g_kGlobalOption.IsConfigCreated() )
		{
			if( PgComputerInfo::GetGpuGrade() >= DEVICE_GRADE_HIGH )
			{
				iGraphicModeSet = 1; // High Set
			}
			else if( PgComputerInfo::GetGpuGrade() >= DEVICE_GRADE_MEDIUM )
			{
				iGraphicModeSet = 3; // custom setting을 이용 (Middle 즈음...)
			}
			else
			{
				iGraphicModeSet = 2; // Low Set
			}
		}
		else*/
		{
			iGraphicModeSet = g_kGlobalOption.GetValue(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET);
		}
	}
	else
	{
		iGraphicModeSet = g_kCmdLineParse.GraphicMode() + 1;
	}

	if( 0 != iGraphicModeSet )
	{
		g_kGlobalOption.SetConfig(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET, iGraphicModeSet, NULL);
		g_kGlobalOption.ApplyConfig();
	}

	bool result = true;
	bool bShaderGood = false;
	const D3DCAPS9* caps = renderer->GetDeviceCaps();
	if (caps == NULL)
		return false;

	if ((caps->TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) == 0)
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card doesn't support NONPOW2CONDTEXT\n");
		result = false;
	}

	if (caps->VertexShaderVersion < D3DVS_VERSION(1,1))
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card doesn't support vertexShader 1.1\n");
		result = false;
	}
	NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card support %d.%d vertexShader\n", D3DSHADER_VERSION_MAJOR(caps->VertexShaderVersion), D3DSHADER_VERSION_MINOR(caps->VertexShaderVersion));

	if (caps->PixelShaderVersion < D3DPS_VERSION(1,1))
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card doesn't support pixelShader 1.1\n");
		result = false;	
	}
	NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card support %d.%d pixelShader\n", D3DSHADER_VERSION_MAJOR(caps->PixelShaderVersion), D3DSHADER_VERSION_MINOR(caps->PixelShaderVersion));

	//if (caps->VertexShaderVersion < D3DVS_VERSION(3,0) || caps->PixelShaderVersion < D3DPS_VERSION(3,0))		
	//{
	//	NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card doesn't support vertex/pixelShader 3.0\n");
	//	g_bUseFog = false;	//! 일단 상황이 안 좋으니 막자.
	//}

	if (caps->VertexShaderVersion < D3DVS_VERSION(2,0) || caps->PixelShaderVersion < D3DPS_VERSION(2,0))
	{
		NILOG(PGLOG_LOG, "[PgMobileSuit] this graphic card doesn't support vertex/pixelShader 2.0\n");
		bShaderGood = false;
	}
	else
	{

		int	iBonesPerPartitions = (caps->MaxVertexShaderConst-70)/3;
		if(iBonesPerPartitions>30)
		{
			iBonesPerPartitions = 30;
		}
		if(iBonesPerPartitions<4)
		{
			iBonesPerPartitions = 4;
		}
		PgRenderer::SetBonesPerPartition(iBonesPerPartitions);

		bShaderGood = true;
	}

	//! Check fog caps
	if ((caps->PrimitiveMiscCaps & D3DPMISCCAPS_FOGANDSPECULARALPHA) == 0)
		result = false;

	if ((caps->PrimitiveMiscCaps & D3DPMISCCAPS_FOGVERTEXCLAMPED) == 0)
		result = false;

	if ((caps->LineCaps & D3DLINECAPS_FOG) == 0)
		result = false;

	if ((caps->RasterCaps & D3DPRASTERCAPS_FOGVERTEX) == 0)
		result = false;

	if ((caps->RasterCaps & D3DPRASTERCAPS_FOGTABLE) == 0)
		result = false;

	if ((caps->RasterCaps & D3DPRASTERCAPS_FOGRANGE) == 0)
		result = false;

	if ((caps->RasterCaps & D3DPRASTERCAPS_WFOG) == 0)
		result = false;

	if ((caps->RasterCaps & D3DPRASTERCAPS_ZFOG) == 0)
		result = false;

	return true;
}

void PgMobileSuit::stopAllThread()
{
	PG_END_SCREEN_UPDATE_THREAD;
	g_kAddUnitThread.IsStop(true);
	g_kAddUnitThread.ClearAllWorks(true);
	g_kEquipThread.IsStop(true);
	g_kEquipThread.ClearAllWorks(true);
	g_kLoadWorkerThread.IsStop(true);
	g_kLoadWorkerThread.ClearAllWorks(true);	
	g_kMemoryWorkerThread.IsStop(true);
	g_kMemoryWorkerThread.ClearAllWorks(true);
	g_kLoader.VDeactivate();
}

void PgMobileSuit::terminateAllThread()
{
	g_kAddUnitThread.IsStop(true);
	g_kAddUnitThread.PrintStatInfo();
	g_kEquipThread.IsStop(true);
	g_kEquipThread.PrintStatInfo();
	g_kLoadWorkerThread.IsStop(true);
	g_kLoadWorkerThread.PrintStatInfo();
	g_kMemoryWorkerThread.IsStop(true);
	g_kMemoryWorkerThread.PrintStatInfo();
	g_kLoader.VDeactivate();
}

void PgMobileSuit::initLogSystem()
{
	//! Log Option and Log File Setting
#if defined(PG_LOG_ENABLED) && !defined(NI_LOGGER_DISABLE) && !defined(USE_INB)
	TCHAR szLogLevelOption[64] = TEXT("");
	int iLogLevel = 0;
	bool bWriteLogDirect = false;
	bool bTreatWarningAsError = false;

	iLogLevel = 0;

	::GetPrivateProfileString(TEXT("Log"), TEXT("LogLevel"), TEXT("NO_LOG"), szLogLevelOption, sizeof(szLogLevelOption), m_szConfigFile);
	bWriteLogDirect = ::GetPrivateProfileInt(TEXT("Log"), TEXT("DirectMode"), 0, m_szConfigFile);
	bTreatWarningAsError = ::GetPrivateProfileInt(TEXT("Log"), TEXT("TreatWarningAsError"), 0, m_szConfigFile);
	int const iInfoLogLevel = ::GetPrivateProfileInt(TEXT("Log"), TEXT("InfoLogLevel"), 9, m_szConfigFile);
	SetLogLeveLimit(static_cast<BM::E_LOG_LEVEL>(iInfoLogLevel));

	if (_tcsncmp(szLogLevelOption, TEXT("WIN_LOG"), MAX_PATH) == 0)
		iLogLevel = 8;
	else if (_tcsncmp(szLogLevelOption, TEXT("FULL_LOG2"), MAX_PATH) == 0)
		iLogLevel = 6;
	else if (_tcsncmp(szLogLevelOption, TEXT("FULL_LOG"), MAX_PATH) == 0)
		iLogLevel = 5;
	else if (_tcsncmp(szLogLevelOption, TEXT("ERROR_LOG"), MAX_PATH) == 0)
		iLogLevel = 1;
	else if (_tcsncmp(szLogLevelOption, TEXT("NO_LOG"), MAX_PATH) == 0)
		iLogLevel = 0;

	switch(iLogLevel)
	{
	case 6:
		NiLogger::SetPrependTimestamp(PGLOG_THREAD, true);
		NiLogger::SetPrependTimestamp(PGLOG_STAT, true);
		NiLogger::SetPrependTimestamp(PGLOG_MINOR, true);
		NiLogger::SetPrependTimestamp(PGLOG_MEMIO, true);
		NiLogger::SetPrependTimestamp(PGLOG_CONSOLE, true);
		NiLogger::SetPrependTimestamp(PGLOG_MESSAGE, true);		
	case 5:
		NiLogger::SetPrependTimestamp(PGLOG_SCRIPT, true);
		NiLogger::SetPrependTimestamp(PGLOG_NETWORK, true);
		NiLogger::SetPrependTimestamp(PGLOG_LOGIC, true);
		NiLogger::SetPrependTimestamp(PGLOG_LOG, true);	
	case 1:
		if (iLogLevel >= 5 || bTreatWarningAsError)
			NiLogger::SetPrependTimestamp(PGLOG_WARNING, true);
		NiLogger::SetPrependTimestamp(PGLOG_ASSERT, true);
		NiLogger::SetPrependTimestamp(PGLOG_ERROR, true);
	case 0:
	default:
		break;
	case 8:
		NiLogger::SetPrependTimestamp(PGLOG_WINMESSAGE, true);
		break;
	}

	if (iLogLevel > 0)
	{
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		char szFileName[128] = {0,};
		char szErrorFileName[128] = {0,};
		char szWarningFileName[128] = {0,};
		char szNetworkFileName[128] = {0,};
		char szThreadFileName[128] = {0,};
		char szStatFileName[128] = {0,};
		char szMemIOFileName[128] = {0,};
		char szConsoleFileName[128] = {0,};
		char szMessageFileName[128] = {0,};
		char szWinMessageFileName[128] = {0,};
		TCHAR szInfoLogFileName[128] = {0,};

		sprintf_s(szFileName, "./Log/%04u%02u%02u%02u%02u%02u_client_log.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szErrorFileName, "./Log/%04u%02u%02u%02u%02u%02u_error_log.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szWarningFileName, "./Log/%04u%02u%02u%02u%02u%02u_warning_log.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szNetworkFileName, "./Log/%04u%02u%02u%02u%02u%02u_newtork_log.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szThreadFileName, "./Log/%04u%02u%02u%02u%02u%02u_thread.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szStatFileName, "./Log/%04u%02u%02u%02u%02u%02u_stat.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szMemIOFileName, "./Log/%04u%02u%02u%02u%02u%02u_memio.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szConsoleFileName, "./Log/%04u%02u%02u%02u%02u%02u_console.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szMessageFileName, "./Log/%04u%02u%02u%02u%02u%02u_message.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		sprintf_s(szWinMessageFileName, "./Log/%04u%02u%02u%02u%02u%02u_winmessage.txt",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		_stprintf_s(szInfoLogFileName, 128, TEXT("%04u%02u%02u%02u%02u%02u_info_log.txt"),
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

		int logid;
		int errorlogid;
		int warninglogid;
		int networklogid;
		int threadlogid;
		int statlogid;
		int memiologid;
		int consolelogid;
		int messagelogid;
		int winmessagelogid;

		BM::ReserveFolder(UNI(szFileName));

		switch(iLogLevel)
		{
		case 6:
			{
				threadlogid = NiLogger::OpenLog(szThreadFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				statlogid = NiLogger::OpenLog(szStatFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				memiologid = NiLogger::OpenLog(szMemIOFileName, NiLogger::OPEN_OVERWRITE, false, true);
				consolelogid = NiLogger::OpenLog(szConsoleFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				messagelogid = NiLogger::OpenLog(szMessageFileName, NiLogger::OPEN_OVERWRITE, false, true);
			}
		case 5:
			{
				networklogid = NiLogger::OpenLog(szNetworkFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				logid = NiLogger::OpenLog(szFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				if (!bTreatWarningAsError)
				{
					warninglogid = NiLogger::OpenLog(szWarningFileName, NiLogger::OPEN_OVERWRITE, bWriteLogDirect, true);
				}
			}
		case 1:
			{
				errorlogid = NiLogger::OpenLog(szErrorFileName, NiLogger::OPEN_OVERWRITE, true, true);
				std::wstring kLogFolder;
				InitLog(kLogFolder, L"Log", BM::OUTPUT_JUST_TRACE, BM::LOG_LV9, L"./");
				INFO_LOG_LEVEL((BM::E_LOG_LEVEL)iInfoLogLevel);
			}
		case 0:
		default:
			{
			}break;
		case 8:
			winmessagelogid = NiLogger::OpenLog(szWinMessageFileName, NiLogger::OPEN_OVERWRITE, false, true);
			break;
		}

		switch(iLogLevel)
		{
		case 6:
			{
				NiLogger::SetOutputToLogFile(PGLOG_THREAD, threadlogid);
				NiLogger::SetOutputToLogFile(PGLOG_STAT, statlogid);
				NiLogger::SetOutputToLogFile(PGLOG_MINOR, logid);
				NiLogger::SetOutputToLogFile(PGLOG_MEMIO, memiologid);
				NiLogger::SetOutputToLogFile(PGLOG_CONSOLE, consolelogid);
				NiLogger::SetOutputToLogFile(PGLOG_MESSAGE, messagelogid);
			}
		case 5:
			{
				NiLogger::SetOutputToLogFile(PGLOG_SCRIPT, logid);
				if (!bTreatWarningAsError)
				{
					NiLogger::SetOutputToLogFile(PGLOG_WARNING, warninglogid);
				} 
				NiLogger::SetOutputToLogFile(PGLOG_NETWORK, networklogid);
				NiLogger::SetOutputToLogFile(PGLOG_LOGIC, logid);
				NiLogger::SetOutputToLogFile(PGLOG_LOG, logid);	
			}
		case 1:
			{
				if (bTreatWarningAsError)
				{	
					NiLogger::SetOutputToLogFile(PGLOG_WARNING, errorlogid);
				}
				NiLogger::SetOutputToLogFile(PGLOG_ASSERT, errorlogid);
				NiLogger::SetOutputToLogFile(PGLOG_ERROR, errorlogid);
			}
		case 0:			
		default:
			{
			}break;
		case 8:
			NiLogger::SetOutputToLogFile(PGLOG_WINMESSAGE, winmessagelogid);
			break;
		}
	}
#ifdef EXTERNAL_RELEASE
	NiSetOutputDebugStringCallback(PgOutputDebugStringFromGameBryo);
#else
	NiSetOutputDebugStringCallback(NULL);
#endif


#endif // #if defined(PG_LOG_ENABLED) && !defined(NI_LOGGER_DISABLE)
}

void PgMobileSuit::TakeScreenShot()
{
	static DWORD s_count = 0;

	__time64_t ltime = 0;
	struct tm today = {0,};

	_time64( &ltime );
	_localtime64_s( &today, &ltime );

	wchar_t fmt[24] = {0,};
	wcsftime(fmt, 24, L"%y%m%d%H%M%S", &today);

	wchar_t FilePath[MAX_PATH] = {0,};
	swprintf(FilePath, L"%s%s%s%02d.jpg", getScreenShotPath().c_str(), GetClientNameW(), fmt, s_count++);
	

	NiDX9Renderer *pkRenderer = NiDX9Renderer::GetRenderer();
	if (pkRenderer == NULL)
		return;

	LPDIRECT3DDEVICE9 pkDevice = pkRenderer->GetD3DDevice();
	if (pkDevice == NULL)
		return;

	int x, y;
	RECT rcWindow;
	LPDIRECT3DSURFACE9 lpBack;
	// get the screen resolution. If it's a windowed application get the whole
	// screen size. If it's a fullscreen application you might have somewhere 
	// your defines as: #define SCREEN_WIDTH 800
	if (m_bFullscreen)
	{
		//x = XUI::EXV_DEFAULT_SCREEN_WIDTH;
		//y = XUI::EXV_DEFAULT_SCREEN_HEIGHT;

		x = XUIMgr.GetResolutionSize().x;
		y = XUIMgr.GetResolutionSize().y;
	}
	else
	{
		x = GetSystemMetrics(SM_CXSCREEN);
		y = GetSystemMetrics(SM_CYSCREEN);

		// to get the window sizes
		GetWindowRect(GetWindowReference(), &rcWindow);
	}

	pkDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &lpBack);

	HBITMAP bitmap;
	HDC hdc1 = NULL, hdc2 = NULL;
	if (!FAILED(lpBack->GetDC(&hdc1)))
	{			
		if (NULL != (hdc2 = CreateCompatibleDC(hdc1)))
		{		
			D3DSURFACE_DESC dsc;
			lpBack->GetDesc(&dsc);
			if (NULL != (bitmap = CreateCompatibleBitmap(hdc1, dsc.Width, dsc.Height)))
			{
				if (SelectObject(hdc2, bitmap))
				{
					if (BitBlt(hdc2, 0, 0, dsc.Width, dsc.Height, hdc1, 0, 0, SRCCOPY))
					{
						CxImage	*pkImage = new CxImage();
						BOOL bRet = pkImage->CreateFromHBITMAP(bitmap);
						if (bRet && pkImage) 
						{
							FILE *fp = _tfopen(FilePath, _T("wb"));
							if (fp && pkImage->Encode(fp, CXIMAGE_FORMAT_JPG))
							{
								//lwAddWarnDataStr(FilePath, 1);//스크린샷. 메세지.								
								SChatLog kChatLog(CT_EVENT_SYSTEM);
								std::wstring kLog = FilePath;
								g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
								fclose(fp);
							}							
						}
						SAFE_DELETE(pkImage);
					}
				}
				DeleteObject(bitmap);
			}
			DeleteDC(hdc2);
		}
		lpBack->ReleaseDC(hdc1);
	}
	else
	{
		if(D3D_OK == D3DXSaveSurfaceToFile(FilePath, D3DXIFF_JPG, lpBack, NULL, NULL))
		{
			//lwAddWarnDataStr(FilePath, 1);//스크린샷. 메세지.
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			std::wstring kLog = FilePath;
			g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
		}
	}
	SAFE_RELEASE(lpBack);	
}

std::wstring const PgMobileSuit::getUserInfoPath()const
{
	wchar_t UserInfoPath[MAX_PATH] = {0,};
	wchar_t userDataPath[MAX_PATH]= {0,};

	if(S_OK == SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, userDataPath))
	{
		swprintf_s(UserInfoPath, MAX_PATH, L"%s\\%s\\Users\\", userDataPath, GetClientNameW());
		BM::ReserveFolder(UserInfoPath);	
		return UserInfoPath;
	}
	else
	{
		::GetCurrentDirectory(MAX_PATH, UserInfoPath);
		std::wstring kParent = BM::GetParentFolder(UserInfoPath);

		kParent += L"\\Users\\";
		BM::ReserveFolder(kParent);	
		return kParent;
	}
}

std::wstring const PgMobileSuit::getScreenShotPath()const
{
	wchar_t screenShotPath[MAX_PATH] = {0,};
	wchar_t userDataPath[MAX_PATH]= {0,};

	if(S_OK == SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, userDataPath))
	{
		swprintf_s(screenShotPath, MAX_PATH, L"%s\\%s\\ScreenShots\\", userDataPath, GetClientNameW());
		BM::ReserveFolder(screenShotPath);	
		return screenShotPath;
	}
	else
	{
		::GetCurrentDirectory(MAX_PATH, screenShotPath);
		std::wstring kParent = BM::GetParentFolder(screenShotPath);

		kParent += L"\\ScreenShots\\";
		BM::ReserveFolder(kParent);	
		return kParent;
	}
}

//int PgMobileSuit::getEncoderClsid(const WCHAR* format, CLSID* pClsid)
//{
//	unsigned int  num = 0;    // number of image encoders
//	unsigned int  size = 0;   // size of the image encoder array in bytes
//
//	Gdiplus::GetImageEncodersSize(&num, &size);
//	if(size == 0)return -1;
//
//	Gdiplus::ImageCodecInfo* imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
//	Gdiplus::GetImageEncoders(num, size, imageCodecInfo);
//
//	for(unsigned int i = 0; i < num; ++i)
//	{
//		if( wcscmp(imageCodecInfo[i].MimeType, format) == 0 )
//		{
//			*pClsid = imageCodecInfo[i].Clsid;
//			delete[] imageCodecInfo;
//			return i;
//		}    
//	}
//	delete[] imageCodecInfo;
//	return -1;
//}

bool PgMobileSuit::doInitializeWork(InitializeWorkID currentWorkID)
{
	switch(currentWorkID)
	{
	case INIT_WORK_ID_0:
		{
			PG_MEM_LOG
				////// RenderMan 생성
				NILOG(PGLOG_LOG, "Start Render Initialization\n");
			if(!g_kRenderMan.SetRenderer(NiRenderer::GetRenderer()))
			{
				PgError("RenderManager Init Failed");
				return false;
			}
			//			g_pkRenderMan = PgRenderMan::Create(NiRenderer::GetRenderer());
			//			m_pkRenderMan = g_pkRenderMan;
			//			if(!m_pkRenderMan)
			//			{
			//				PgError("RenderManager Init Failed");
			//				return false;
			//			}
			NILOG(PGLOG_LOG, "End Render Initialization\n");
			if( false == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_TAIWAN ) )
			{
				g_kFontMgr.Init(gCreate2DStringFunc, gCreateFontFunc);
			}
			else
			{
				g_kFontMgr.Init(gCreate2DStringFunc);
			}
			g_kFontMgr.SetCalculateOnlySizeFunc(CalculateOnlySize);

			NILOG(PGLOG_LOG, "End FontMgr Initialization\n");		
			PG_MEM_LOG

				g_kControllerSequenceManager.Init();
			g_kWorldObjectPool.Init();
			g_kQuestMan.Init();	//	빈 함수이지만, 반드시 호출해줘야 싱글톤 객체가 생성된다. 절대 지우지 말것!
			g_kShineStoneUpgrade.Clear();

			PG_MEM_LOG
				NILOG(PGLOG_LOG, "Start LoadTBData Initialization\n");

			LoadAllDefStringXML("DefStringPath.xml");

			if(!LoadTBData(_T("../Table/") ))
			{
				PgError("Table Data Load Failed");
				return false;
			}
			NILOG(PGLOG_LOG, "End LoadTBData Initialization\n");

			NILOG(PGLOG_LOG, "Start UI Initialization\n");
			if(!g_kUIScene.Initialize())
			{
				PgError("UI System Init Failed.");
				return false;
			}

			g_kUIScene.EnableOffScreenRendering(g_bUseUIOffscreenRendering);
			g_kEnvironmentMan.Init();
			g_kHomeTownMgr.Build();
			g_kHomeRenewMgr.Build();

			NILOG(PGLOG_LOG, "End UI Initialization\n");
			PG_MEM_LOG
		}break;
	case INIT_WORK_ID_1:
		{
			PG_MEM_LOG			
			PgXmlLoader::CreateObject(XML_ELEMENT_MAP_TEXT_TABLE);
			PgXmlLoader::CreateObjectByType(XML_ELEMENT_QUEST_TEXT_TABLE);
			//PgXmlLoader::CreateObject(XML_ELEMENT_ERROR_CODE_TABLE);
			PgXmlLoader::CreateObject("KEYSET_TABLE");//Load Default Key Funciont Map
			PgXmlLoader::CreateObjectByType("BAD_WORD");
			switch(g_kLocal.ServiceRegion())
			{
			case LOCAL_MGR::NC_USA:
			case LOCAL_MGR::NC_EU:
				{// 유럽, 미국은 IGNORE_WORD를 사용한다
					PgXmlLoader::CreateObject("IGNORE_WORD");
				}break;
			case LOCAL_MGR::NC_JAPAN:
				{
					NexonInit();
				}break;
			}
			PgXmlLoader::CreateObject("GOOD_WORD");
			PgXmlLoader::CreateObject("BLOCK_NICK_NAME");
			PgXmlLoader::CreateObject("LINE_COMMAND");
			PgXmlLoader::CreateObject("CHAT_COMMAND");
			PgXmlLoader::CreateObject("WORLDMAP");
			PgXmlLoader::CreateObject("RESOURCE_ICON");
			PgXmlLoader::CreateObject("REALM_QUEST_EVENT");
			PgXmlLoader::CreateObject(XML_ELEMENT_LEGEND_ITEM_MAKE_HELP);
			PgXmlLoader::CreateObject(XML_ELEMENT_SUPER_GROUND_ENTER_SETTING);
			PgXmlLoader::CreateObject(XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING);
			PgXmlLoader::CreateObject(XML_ELEMENT_BALLOON_FORM);
			PG_MEM_LOG

				//// 로컬/리모트 인풋 관리자 생성 및 등록
				// TODO : Network으로 부터 Stream받아서 Singleton만들기
				NILOG(PGLOG_LOG, "Start Input Initialization\n");
			if(!PgLocalManager::Create("s_input_local", NiRenderer::GetRenderer() ))
			{
				PgError("Local Input Manager Init Failed");
				return false;
			}

			g_kGlobalOption.ApplyKeySet_ToSystem();//Set User Config Keyboard setting or Default Keyboard setting

			g_pkRemoteManager = dynamic_cast<PgRemoteManager *>(PgXmlLoader::CreateObject("s_input_remote"));
			if(!g_pkRemoteManager)
			{
				PgError("Remote Manager Init Failed");
				return false;
			}

			PgIInputManager::AddInputObserver(&g_kPilotMan);
			NILOG(PGLOG_LOG, "End Input Initialization\n");
			PG_MEM_LOG

				PgXmlLoader::LoadAllDocumentToCache();
		}break;
	case INIT_WORK_ID_2:
		{
			PG_MEM_LOG
				//// 네트워크 엔진을 초기화한다.
				//
				NILOG(PGLOG_LOG, "Start Network Initialization\n");
			if(!g_kNetwork.Init())
			{
				PgError("Network Init Faild");
				return false;
			}
			NILOG(PGLOG_LOG, "End Network Initialization\n");
			//AfterLoadTBData();
			PG_MEM_LOG
		}break;
	case INIT_WORK_ID_3:
		{
			PG_MEM_LOG
				//// PhysX 물리 엔진을 초기화한다.
				//
				NILOG(PGLOG_LOG, "Start Physics Initialization\n");
			NiPhysXManager* pkManager = NiPhysXManager::GetPhysXManager();
			if (!pkManager->Initialize())
			{
				PgError("PhysX Init Faild");
				return false;
			}

			// Countinous Collision Detection를 사용한다.
			// 이 설정을 빼면, 캐릭터가 충돌면을 통과하는 현상이 발생한다.
			//	pkManager->m_pkPhysXSDK->setParameter(NX_CONTINUOUS_CD, 1.0f);
			//	pkManager->m_pkPhysXSDK->setParameter(NX_CCD_EPSILON, 0.1f);
			// 디버깅 정보를 표시하도록 설정한다.
#ifndef USE_INB
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 15.0f);
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 1.0f);
			//pkManager->m_pkPhysXSDK->setParameter(NX_TRIGGER_TRIGGER_CALLBACK, false);
			//pkManager->m_pkPhysXSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425);
#endif
			//	pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_COLLISION_SKELETONS, 1);
			//	pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_COLLISION_CCD, 1);
			NILOG(PGLOG_LOG, "End Physics Initialization\n");
			PG_MEM_LOG
		}break;
	case INIT_WORK_ID_4:
		{
			PG_MEM_LOG
				NILOG(PGLOG_LOG, "Start Shader Initialization\n");
			char const* pcWorkingDir = "../Shader/Generated";
			NiMaterial::SetDefaultWorkingDirectory(pcWorkingDir);
			g_pkApp->m_pkShaderHelper = NiNew ShaderHelper;


			if(g_pkApp->m_pkShaderHelper)
			{
				char* apcProgramDirectories[1] = 
				{
					"../Shader/DX9/"
				};

				char* apcShaderDirectories[1] =
				{
					"../Shader/"
				};

				NILOG(PGLOG_LOG, "Start SetupShaderSystem Initialization\n");
				if(!g_pkApp->m_pkShaderHelper->SetupShaderSystem(apcProgramDirectories, 1, apcShaderDirectories, 1))
				{
					PgError("Shader Init Failed");
					return false;
				}
			}
			PG_MEM_LOG

				if(!g_kRenderMan.SetupPostProcessShaders())
				{
					return false;
				}


#ifndef USE_INB
				//개발용 기능
				g_kSkillTargetMan.Init();
				g_kMonSkillTargetMan.Init();
				g_kZoneDrawer.Init();
#endif//USE_INB

				NILOG(PGLOG_LOG, "End Shader Initialization\n");
				PG_MEM_LOG
		}break;
	case INIT_WORK_ID_5:
		{
			//			g_kCashShopMgr.OnBuild();
			g_kCashShopMgr.ParseXml();
			g_kAchieveNfyMgr.ParseXml("Setting.xml");
			g_kAchieveNfyMgr.OnBuild();
			g_kWorldMapUI.OnBuild();			

			PG_MEM_LOG
		}break;
	case INIT_WORK_ID_6:
		{
			PG_MEM_LOG
				NILOG(PGLOG_LOG, "Start Particle Initialization\n");
			//// ParticleMan 생성
			//
			g_kFxStudio.Initialize();
			if(!g_kParticleMan.ParseXml("effect.xml"))
			{
				PgError("Can't Load Particle Info");
			}

			g_kItemMan.ParseXml_SetItemEffectTable(_T("SetEffectTable.xml"), _T("SetItemEffectTable.xml"));
			//g_kItemMan.ParseXML_ItemPOTParticleInfo(L"ItemPOTParticle.xml");
			PG_MEM_LOG
		}break;
	case INIT_WORK_ID_7:
		{
#ifndef EXTERNAL_RELEASE
			g_pkApp->createOutputModules();
#endif
			g_kWorldNpcMgr.LoadFile("All_Npc.xml"); // Npc
			g_kConstellationEnterUIMgr.LoadFile("ConstellationSetting.xml");
			g_kMovieMgr.ParseXml("movie.xml");
			g_kMovieMgr.ParseScenarioXml("ScenarioMovie.xml");
			g_kMovieMgr.ParseQuestXml("QuestMovie.xml");
			PgCameraMan::ParseXml("camera.xml");
			g_kBriefingLoadUIMgr.ParseXML("BriefingLoadTable.xml");

			g_kHelpSystem.InitSystem();
			g_kSkillOptionMan.Initialize();

			g_kProjectileMan.Init();
			g_kScreenBreak.Init();
			g_kChainAttack.Init();
			g_kPartyBalloonMgr.Clear();
			g_kExpeditionBalloonMgr.Clear();
			g_kVendorBalloonMgr.Clear();
			g_kBalloonMan2D.Clear();
			g_kBalloonMan3D.Clear();
			g_kMarkBalloonMan.Clear();
			g_kQuestMarkHelpMng.Clear();
			g_kComboAdvisor.Init();
			g_kHeadBuffIconListMgr.Init();
			g_kMissionComplete.CleanUp();
			g_kBossComplete.CleanUp();
			g_kExpeditionComplete.CleanUp();
			g_kBalloonEmoticonMgr.Initialize();
			g_kBookActorCameraMgr.Init();
			g_kSkillTree.ParseXml_BasicCombo();

			g_kNifMan.GetNif("../Data/5_Effect/0_Common/CircleShadowGeom.nif");
			g_kNifMan.GetTexture("../Data/5_effect/9_Tex/CircleShadow.dds");
			g_kNifMan.GetNif("../Data/5_Effect/0_Common/e_target_01.nif");
			g_kNifMan.GetNif("../Data/6_UI/Quad.Nif");
			g_kNifMan.GetNif("../Data/6_ui/Combo/combo.nif");
			g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum.nif");
		}break;
	case INIT_WORK_ID_8:
		{
			BM::CAutoMutex kLock(g_kSoundInitMutex);
			PG_MEM_LOG
				NILOG(PGLOG_LOG, "Start Sound Loading\n");
			if(g_bUseSound && !g_kSoundMan.ParseXml("sound.xml"))
			{
				PgError("Sound Loading Faild");
			}
			NILOG(PGLOG_LOG, "Start WeaponSound Loading\n");
			//// 무기 사운드 로드
			//
			PG_MEM_LOG
				if(g_bUseSound && !g_kWeaponSoundMan.Initialize())
				{
					PgError("Weapon Sound Load Failed");
				}
				NILOG(PGLOG_LOG, "End WeaponSound Loading\n");
				PG_MEM_LOG
		}break;

		// TODO: HardCoding된 것을 따로 외부로 빼는 것이 좋겠다.
	case INIT_WORK_ID_9:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/1_Male/01_Fig/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_10:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/2_Female/01_Fig/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_11:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/1_Male/02_Mag/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_12:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/2_Female/02_Mag/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_13:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/1_Male/03_Arc/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_14:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/2_Female/03_Arc/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_15:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/1_Male/04_Thi/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_16:
		g_kAMPool.LoadActorManager("../Data/1_Cha/1_PC/2_Female/04_Thi/00_Base.kfm", PgIXmlObject::ID_PC, true);
		break;
	case INIT_WORK_ID_17:
		g_kSelectStage.PrepareBaseItemFullSet();
		//g_kActionPool.LoadActionBeforeUse();
		break;
	default:
		break;
	}

	return true;
}

void PgMobileSuit::AddStringToWindowTitle(const wchar_t* pszTitle, int index)
{
	//#ifndef EXTERNAL_RELEASE
#define MAX_WINDOW_TITLE_SLOT	8
	if (index < 0 || index >= MAX_WINDOW_TITLE_SLOT || m_bFullscreen)
		return;

	static wchar_t winTitles[MAX_WINDOW_TITLE_SLOT][MAX_PATH] = { { 0, }, };

	if (pszTitle)
	{
		wcsncpy_s(winTitles[index], MAX_PATH, pszTitle, MAX_PATH - 1);
	}
	else
	{
		winTitles[index][0] = TEXT('\0');
	}

	wchar_t title[MAX_PATH * MAX_WINDOW_TITLE_SLOT] = { 0, };
	for (int i = 0; i < MAX_WINDOW_TITLE_SLOT; i++)
	{
		wcscat_s(title, winTitles[i]);
	}

	SetWindowTextW(GetWindowReference(), title);
	//#endif
}

void PgMobileSuit::adjustGameOption()
{
	// full screen, low graphic, shader option 조정.
	int iLowGraphics = 1;
	int iShader = 0;

	enum
	{
		iFullScreenOption = 1,
		iWindowScreenOption = 2,
	};

	// read options from Command (if exists)
	if( g_kCmdLineParse.FullMode() == -1 )
	{
		// read options from Config.xml
		m_bFullscreen = (iFullScreenOption == g_kGlobalOption.GetValue("GAME", "FULL_SCREEN")); // 1이면 FullScreen, 2면 윈도우 모드이다.

#ifndef USE_INB
		if( m_iFullScreenIni != -1 )
		{
			m_bFullscreen = (m_iFullScreenIni == iFullScreenOption); // from ini
		}
#endif
	}
	else
	{
		m_bFullscreen = (bool)g_kCmdLineParse.FullMode(); // from command line
	}

	SetVSync((g_kGlobalOption.GetCurrentGraphicOption("REFRESH_RATE_SYNC") == 1));

	g_kGlobalOption.SetConfig(XML_ELEMENT_GAME, STR_GAME_FULL_SCREEN, ((m_bFullscreen)? iFullScreenOption: iWindowScreenOption), NULL);
	g_kGlobalOption.ApplyConfig();
}

bool PgMobileSuit::doInitScript()
{
	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

		NILOG(PGLOG_LOG, "Start Script Initialization\n");
		if(!PgScripting::Initialize())
		{
			PgError("Script System Init Failed");
			return false;
		}
	}

	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		BM::CAutoMutex kLock(g_kSoundInitMutex);
		NILOG(PGLOG_LOG, "Start Sound Initialization\n");
		if(g_bUseSound && !g_kSoundMan.Initialize())
		{
			PgError("Sound System Init Faild");
		}
		g_kSoundMan.SetBGMVolume(0.f, true);
		g_kSoundMan.SetEffectVolume(0.f, true);
		NILOG(PGLOG_LOG, "End Sound Initialization\n");
	}

	char pcInitScript[128] = "init_basic.lua";
	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		NILOG(PGLOG_LOG, "Start InitScript\n");
		if (m_bSingleMode)
		{
			strcpy_s(pcInitScript, 128, "init_single.lua");
		}
	}
	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		if (ms_pkCommand)
		{
			ms_pkCommand->String("initscript", pcInitScript, sizeof(pcInitScript));
		}
	}

	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);

		PgScripting::DoFile(pcInitScript);
		NILOG(PGLOG_LOG, "End InitScript\n");

		if(g_bUseImportActionData)
		{//#XML_TO_CODE
			PgActionXmlData::CONT_STR const& rkContLuaData = g_kActionXmlData.GetContLuaData();
			PgActionXmlData::CONT_STR::const_iterator kItor = rkContLuaData.begin();
			while(rkContLuaData.end() != kItor)
			{
				std::string const& rkElem = (*kItor);
				PgScripting::DoString( rkElem.c_str() );
				++kItor;
			}
		}
		
		if(!lua_tinker::call<bool>("LoadingChecker"))
		{
			PgError1("%s Load Failed", pcInitScript);
			return false;
		}
	}

	{		
		PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
		if(!XUIMgr.InitIME(GetWindowReference(), GetInstanceReference()))
		{
			PgError("XUI Ime Init Failed");
			QuitApplication();
			return true;
		}
	}
	g_kLoader.MainCompleteSignal();

	return true;
}

void PgMobileSuit::setMainThreadPriority(int iThreadPriority)
{
#ifndef EXTERNAL_RELEASE
	if (iThreadPriority > THREAD_PRIORITY_NORMAL)
		iThreadPriority = THREAD_PRIORITY_NORMAL;
#endif

	DWORD dwProcessAffinity = 0xFFFFFFFF;
	DWORD dwSystemAffinity = 0xFFFFFFFF;
	if (::GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinity, &dwSystemAffinity))
	{
		NILOG(PGLOG_THREAD, "Process Affinity %#x, System Affinity %#x\n", dwProcessAffinity, dwSystemAffinity);
	}

	HANDLE hMainThread = GetCurrentThread();
	HANDLE hMainProcess = GetCurrentProcess();
	::SetThreadAffinityMask(hMainThread, 0xFFFFFFFF & dwProcessAffinity);
	::SetThreadIdealProcessor(hMainThread, PgComputerInfo::GetIdealProcessorForThread(0));
	::SetThreadPriority(hMainThread, iThreadPriority);
}

void PgMobileSuit::setBackgroundThreadPriority(int iThreadPriority, int iType)
{
#ifndef EXTERNAL_RELEASE
	if (iThreadPriority > THREAD_PRIORITY_BELOW_NORMAL)
		iThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
#endif
	DWORD dwProcessAffinity = 0xFFFFFFFF;
	DWORD dwSystemAffinity = 0xFFFFFFFF;
	if (::GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinity, &dwSystemAffinity))
	{
		NILOG(PGLOG_THREAD, "Process Affinity %#x, System Affinity %#x\n", dwProcessAffinity, dwSystemAffinity);
	}

	ACE_Thread_Manager* pManager = NULL;
	ACE_hthread_t kThreadIdArray[20];
	ssize_t count = 0;

	if (iType == 0)
	{
		pManager = ACE_Thread_Manager::instance();
		if (pManager)
			count = pManager->hthread_list(&g_kLoader, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
	}
	else if (iType == 1)
	{
		pManager = ACE_Thread_Manager::instance();
		if (pManager)
		{
			ACE_hthread_t kThreadIdArray2[10] ={0,};
			count = pManager->hthread_list(&g_kAddUnitThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
			ssize_t count2 = pManager->hthread_list(&g_kEquipThread, kThreadIdArray2, sizeof(kThreadIdArray2)/sizeof(ACE_hthread_t));

			for (int i = count; i < count + count2 && i < 20; i++)
			{
				kThreadIdArray[i] = kThreadIdArray2[i - count];
			}
			count += count2;
		}
	}
	else if (iType == 2)
	{
		pManager = ACE_Thread_Manager::instance();
		if (pManager)
			count = pManager->hthread_list(&g_kMemoryWorkerThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
	}
	else if (iType == 3)
	{
		pManager = ACE_Thread_Manager::instance();
		if (pManager)
			count = pManager->hthread_list(&g_kLoadWorkerThread, kThreadIdArray, sizeof(kThreadIdArray)/sizeof(ACE_hthread_t));
	}

	if (pManager)
	{
		for (int i = 0; i < count; i++)
		{
			::SetThreadAffinityMask(kThreadIdArray[i], 0xFFFFFFFF & dwProcessAffinity);
			::SetThreadIdealProcessor(kThreadIdArray[i], PgComputerInfo::GetIdealProcessorForThread(i + 1));
			::SetThreadPriority(kThreadIdArray[i], iThreadPriority);

			DWORD dwPriorityClass;
			int	iThreadPriority;
			BOOL bThreadPriorityBoost;

			dwPriorityClass = ::GetPriorityClass(kThreadIdArray[i]);
			iThreadPriority = ::GetThreadPriority(kThreadIdArray[i]);
			::GetThreadPriorityBoost(kThreadIdArray[i], &bThreadPriorityBoost);

			NILOG(PGLOG_THREAD, "Loader Thread %d(%d), %d class, %d prio, %d boost\n", i, kThreadIdArray[i], dwPriorityClass, iThreadPriority, bThreadPriorityBoost);
		}
	}
}

void PgMobileSuit::ParseLocalElement(TiXmlElement const* pkElement, PgMobileSuit::CONT_PATH & rkContPath)
{
	while(pkElement)
	{
		assert(pkElement);

		char const* pcTagName = pkElement->Value();
		if(0 == strcmp(pcTagName, "LOCAL"))
		{
			PgXmlLocalUtil::RESULT_NODE kResultNode = PgXmlLocalUtil::FindInLocalNode(g_kLocal, pkElement, "DefStringPath.XML", true);
			if(NULL == kResultNode.first)
			{
				kResultNode.first = kResultNode.second;
				kResultNode.second = NULL;
			}

			CONT_PATH kContFindPath;
			CONT_PATH kContDefaultPath;

			if( TiXmlNode const* pkFindLocalNode = kResultNode.first )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				while( pkResultNode )
				{
					ParseLocalElement(pkResultNode, kContFindPath);
					pkResultNode = pkResultNode->NextSiblingElement();
				}
			}

			if( TiXmlNode const* pkFindLocalNode = kResultNode.second )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				while( pkResultNode )
				{
					ParseLocalElement(pkResultNode, kContDefaultPath);
					pkResultNode = pkResultNode->NextSiblingElement();
				}
			}

			CONT_PATH::const_iterator path_it = kContFindPath.begin();
			while(path_it != kContFindPath.end())
			{
				if(PgXmlLoader::IsFile(path_it->second.c_str()))
				{
					rkContPath.insert(std::make_pair(path_it->first, path_it->second));
				}
				else
				{
					CONT_PATH::const_iterator path2_it = kContDefaultPath.find( path_it->first );
					if(path2_it != kContDefaultPath.end())
					{
						rkContPath.insert(std::make_pair(path2_it->first, path2_it->second));
					}
				}
				++path_it;
			}
		}
		else if(0 == strcmp(pcTagName, "DEF_STRING"))
		{
			char const * pcID = pkElement->Attribute("ID");
			if(pcID && pkElement->GetText())
			{
				std::string strID = pcID;
				std::string strPath = pkElement->GetText();
				UPR(strID);
				UPR(strPath);
				rkContPath.insert(std::make_pair(strID, strPath));
			}
		}

		pkElement = pkElement->NextSiblingElement();
	}
}

bool PgMobileSuit::LoadAllDefStringXML(char const* pcFileName)
{
	TiXmlDocument kXmlDoc;
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcFileName)))
	{
		PG_ASSERT_LOG(!"failed to loading DefStringsPath.xml");
		return false;
	}

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = kXmlDoc.FirstChild();
	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", pcFileName);
		return false;
	}

	CONT_PATH kContPath;
	TiXmlElement* pkElement = pkRootNode->FirstChildElement();
	while(pkElement)
	{
		ParseLocalElement(pkElement, kContPath);
		pkElement = pkElement->NextSiblingElement();
	}

	CONT_PATH::const_iterator path_it = kContPath.begin();
	while(path_it != kContPath.end())
	{
		TiXmlDocument kDoc;
		if(!PgXmlLoader::LoadFile(kDoc, UNI(path_it->second)))
		{
			_PgMessageBox("Error", "[%s] can't read", path_it->second.c_str());
			INFO_LOG( BM::LOG_LV1, __FL__ << path_it->second.c_str()
				<< L" XML Load Error : " << UNI(kDoc.ErrorDesc())
				<< L" Row : " << kDoc.ErrorRow() 
				<< L" Col : " << kDoc.ErrorCol() );
		}
		if(!g_kTblDataMgr.ParseDefStringXML(kDoc.FirstChild()))
		{
			PG_ASSERT_LOG(NULL);
			PgXmlError1(pkElement, "Can't Load XML:[%s]", path_it->second.c_str());
		}
		++path_it;
	}

	return true;
}

void PgMobileSuit::ToggleFrameRate()
{
#ifndef USE_INB
	m_bFrameRateEnabled = !m_bFrameRateEnabled;
	if (m_bFrameRateEnabled && m_pkFrameRate == NULL)
	{
		m_pkFrameRate = NiNew NiFrameRate();
		if (m_pkFrameRate)
		{
			m_pkFrameRate->Init(true);
		}
	}
#endif
}

void PgMobileSuit::LoadServerInfo()
{
	::GetPrivateProfileString( _T("SERVER"), _T("IP"),	_T(""), m_szServerIP, sizeof(m_szServerIP) - 1, m_szConfigFile);
	m_wServerPort = (WORD)::GetPrivateProfileInt( _T("SERVER"),	_T("PORT"),	0, m_szConfigFile);
}

__int64 PgMobileSuit::ReadPatchVer()
{
	typedef	std::list<std::wstring>	CUT_STRING;

	std::vector< char >	vecKey;

	BM::FileToMem(L"..\\Patch.ID", vecKey);
	BM::SimpleDecrypt(vecKey, vecKey);
	vecKey.push_back('\n');

	std::string	strKey;
	strKey.resize(vecKey.size());
	::memcpy(&strKey.at(0), &vecKey.at(0), vecKey.size());

	if(!strKey.size()){ return 0; }

	CUT_STRING	kNumberList;
	std::wstring	wstrKey = UNI(strKey);
	BM::vstring::CutTextByKey<std::wstring>(wstrKey, L".", kNumberList);

	if(kNumberList.size() != 3){ return 0; }

	CUT_STRING::iterator	iter = kNumberList.begin();

	BM::VersionInfo	Info;
	Info.Version.i16Major = (WORD)_wtoi(iter->c_str());	++iter;
	Info.Version.i16Minor = (WORD)_wtoi(iter->c_str()); ++iter;
	Info.Version.i32Tiny = _wtoi(iter->c_str());
	return Info.iVersion;
}

void PgMobileSuit::OnConfigChanged()
{
	RecreateRenderer((g_kGlobalOption.GetValue("GAME", "FULL_SCREEN") == 1),
		(g_kGlobalOption.GetValue("GAME", "BORDERLESS") == 1),
		g_kGlobalOption.GetValue( XML_ELEMENT_GAME, STR_GAME_RESOLUTION_WIDTH ),
		g_kGlobalOption.GetValue( XML_ELEMENT_GAME, STR_GAME_RESOLUTION_HEIGHT ),
		0,	// RefreshRate 기본값 0
		(g_kGlobalOption.GetCurrentGraphicOption("REFRESH_RATE_SYNC") == 1));

	if (g_pkWorld)
	{
		PgWorld::ObjectContainer kContainer;
		g_pkWorld->GetContainer(PgIXmlObject::MAX_XML_OBJECT_ID, kContainer);

		int iLODLevel = 0;
		if (GET_OPTION_WORLD_QUALITY == false)
		{
			iLODLevel = 1;
		}
		PgWorld::ObjectContainer::const_iterator itr = kContainer.begin();
		while (kContainer.end() != itr)
		{
			PgIWorldObject* pkObject = itr->second;
			if (pkObject)
			{
				int iLODCount = pkObject->GetLODCount();
				if (iLODCount > 0)
				{
					//pkObject->SetLOD(iLODLevel);
				}
			}
			++itr;
		}
	}
}

void PgMobileSuit::adjustUrl()
{
	if (g_kCmdLineParse.SiteIndex() != -1)
	{
		std::map<int, std::wstring>::iterator iter = m_kRankUrlMap.find(g_kCmdLineParse.SiteIndex());
		if (iter != m_kRankUrlMap.end())
		{
			_sntprintf(PgRankingMgr::ms_szURL, MAX_PATH, _T("%s"), iter->second.c_str());
		}

		std::map<int, std::wstring>::iterator iter2 = m_kCashChargeUrlMap.find(g_kCmdLineParse.SiteIndex());
		if (iter2 != m_kCashChargeUrlMap.end())
		{
			m_kCashChargeUrl = iter2->second;
		}
	}
}

void PgMobileSuit::ChangeStickyKeySetting()
{
	// 고정키 막기 
	// 백업 
	STICKYKEYS sStickKeys;
	sStickKeys.cbSize = sizeof(STICKYKEYS);
	SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sStickKeys, 0 );
	m_kStickyKeysBackup = sStickKeys;
	// 설정
	sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sStickKeys, 0 );
	m_bKeySettingBackuped = true;
}

void PgMobileSuit::RestoreStickyKeySetting()
{
	if (m_bKeySettingBackuped)
	{
		// 고정키 복구
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_kStickyKeysBackup, 0 );
	}
}


void PgMobileSuit::SetStandardResolutionSize(POINT2 const& rkSize)
{
	m_kStandardResolutionSize.x = rkSize.x;
	m_kStandardResolutionSize.y = rkSize.y;
}

POINT2 PgMobileSuit::GetStandardResolutionSize() const
{
	return m_kStandardResolutionSize;
}

void PgMobileSuit::AdjustWindowRectForFullScreenToWindowMode(LPRECT lpWindowRect, LPRECT lpClientRect, unsigned int const iW, unsigned int const iH)
{
	unsigned int iCaptionBarHeight = ::GetSystemMetrics(SM_CYCAPTION);
	unsigned int iFrameWidth = ::GetSystemMetrics(SM_CXFIXEDFRAME);
	unsigned int iFrameHeight = ::GetSystemMetrics(SM_CYFIXEDFRAME);

	SetRect(lpWindowRect, 0, 0, iW, iH);
	SetRect(lpClientRect, 0, 0, iW, iH);

	lpWindowRect->right = lpWindowRect->right + iFrameWidth * 2;
	lpWindowRect->bottom = lpWindowRect->bottom + iFrameHeight * 2 + iCaptionBarHeight;

	unsigned int iNeedWidth = iW + iFrameWidth * 2;
	unsigned int iNeedHeight = iH + iFrameHeight * 2 + iCaptionBarHeight;

	bool bDesktopIsSmall = false;
	if (m_mi.rcWork.bottom - m_mi.rcWork.top <= iNeedHeight || m_mi.rcWork.right - m_mi.rcWork.left <= iNeedWidth)
	{
		bDesktopIsSmall = true;
	}
	else
	{
		bDesktopIsSmall = false;
	}

	//저장된 mi를 이용하여 랜더러 크기를 다시 계산해야 함

	unsigned int iMaxWidth = m_mi.rcWork.right - m_mi.rcWork.left;
	unsigned int iMaxHeight = m_mi.rcWork.bottom - m_mi.rcWork.top;	
	unsigned int iFitWidth = 0;
	unsigned int iFitHeight = 0;
	bool bCenterX = false;
	bool bCenterY = false;

	if( lpClientRect->right - lpClientRect->left == m_mi.rcWork.right - m_mi.rcWork.left
		&& lpClientRect->bottom - lpClientRect->top == m_mi.rcWork.bottom - m_mi.rcWork.top)
	{	//게임화면 크기가 모니터 해상도와 같으면 AdjustWindowRect( 함수 안에서 창모드 전체화면을 해 
		//줄 것이기 때문에 윈도우 크기를 모니터 크기로 맞추자. 대략 좋지 않음.
		SetRect(lpWindowRect, 0, 0, iMaxWidth, iMaxHeight);
		return;
	}

	if(false==bDesktopIsSmall)
	{
		int x = iMaxWidth/2 - (lpWindowRect->right - lpWindowRect->left)/2;
		int y = iMaxHeight/2 - (lpWindowRect->bottom - lpWindowRect->top)/2;

		SetRect(lpWindowRect, x, y, x + iNeedWidth, y + iNeedHeight);
	}
	else
	{
		// 최대 size찾기
		unsigned int iMaxWidthByMaxHeight = (unsigned int)((iMaxHeight - iFrameHeight * 2 - iCaptionBarHeight) * iW / iH);
		unsigned int iMaxHeightByMaxWidth = (unsigned int)((iMaxWidth - iFrameWidth * 2) * iH / iW);		

		if (iMaxHeightByMaxWidth <= iMaxHeight)
		{
			iFitWidth = iMaxWidth;
			iFitHeight = iMaxHeightByMaxWidth;
			bCenterY = true;
		}
		else if (iMaxWidthByMaxHeight <= iMaxWidth)
		{
			iFitWidth = iMaxWidthByMaxHeight;
			iFitHeight = iMaxHeight;
			bCenterX = true;
		}
		else
		{
			PG_ASSERT_LOG(0);
		}

		// 가운데 정렬
		if (bCenterX)
		{
			lpWindowRect->left = (unsigned int)((iMaxWidth - iFitWidth) / 2.0f) - iFrameWidth;
			lpWindowRect->right = lpWindowRect->left + iFitWidth + iFrameWidth * 2;
		}
		else
		{
			lpWindowRect->left = 0;
			lpWindowRect->right = lpWindowRect->left + iFitWidth ;
		}		
		lpClientRect->left = lpWindowRect->left + iFrameWidth;
		lpClientRect->right = lpWindowRect->right - iFrameWidth;

		if (bCenterY)
		{
			lpWindowRect->top = (unsigned int)((iMaxHeight - iFitHeight + iFrameHeight * 2 + iCaptionBarHeight) / 2.0f);
			lpWindowRect->bottom = lpWindowRect->top + iFitHeight + iFrameHeight * 2 + iCaptionBarHeight;
		}
		else
		{
			lpWindowRect->top = 0;
			lpWindowRect->bottom = lpWindowRect->top + iFitHeight;
		}
		lpClientRect->top = lpWindowRect->top + iFrameHeight + iCaptionBarHeight;
		lpClientRect->bottom = lpWindowRect->bottom - iFrameHeight;	
	}
}

void PgMobileSuit::PreInitializeForWindowMode(unsigned int const iW, unsigned int const iH)
{
	m_mi.cbSize = sizeof(m_mi);
	GetMonitorInfo(MonitorFromWindow(GetWindowReference(), MONITOR_DEFAULTTONEAREST), &m_mi);

	//랜더러가 생성될 때, 주어진 해상도가 아니라 윈도우 정보를 받아오기 때문에
	DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU;
	bool bDesktopIsSmall = false;
	RECT kClientRect;
	RECT kWindowRect = {0,0,iW, iH};;	//창 틀까지 포함한 길이
	AdjustWindowRect(&kWindowRect, &kClientRect, dwWindowStyle, bDesktopIsSmall, true, true);

	SetWindowLong( GetWindowReference(), GWL_STYLE, dwWindowStyle );

	unsigned int uiWidth = 0, uiHeight = 0;
	uiWidth = kClientRect.right - kClientRect.left;
	uiHeight = kClientRect.bottom - kClientRect.top;

	if(EXV_DEFAULT_SCREEN_WIDTH > uiWidth || EXV_DEFAULT_SCREEN_HEIGHT > uiHeight)
	{//기본해상도 보다 작으면 최소한으로 맞춰줌
		uiWidth = EXV_DEFAULT_SCREEN_WIDTH;
		uiHeight = EXV_DEFAULT_SCREEN_HEIGHT;
	}
	XUIMgr.SetResolutionSize(POINT2(uiWidth,uiHeight));

	uiWidth += (kWindowRect.right-kWindowRect.left) - (kClientRect.right-kClientRect.left);
	uiHeight += (kWindowRect.bottom-kWindowRect.top) - (kClientRect.bottom-kClientRect.top);
	SetWindowPos(GetWindowReference(), HWND_NOTOPMOST,
		kWindowRect.left, kWindowRect.top,
		uiWidth,uiHeight,SWP_FRAMECHANGED);
}