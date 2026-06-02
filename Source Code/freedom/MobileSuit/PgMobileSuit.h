#ifndef FREEDOM_DRAGONICA_APP_PGMOBILESUIT_H
#define FREEDOM_DRAGONICA_APP_PGMOBILESUIT_H

#include "NiApplication/NiApplication.h"
#include "PgLocalManager.h"

namespace NewWare { namespace Renderer { 
    class DrawWorkflow;
} }

class PgRenderMan;
class ShaderHelper;
class PgStatGroup;

struct	stDebugInfo
{
	int	m_iTotalUpdateTime;	//	Update + Render
	int	m_iRenderingTime;
	int	m_iUpdateTime;

	int	m_iFPSMethod;
	int	m_iUPS;

	stDebugInfo()
	{
		m_iFPSMethod = 1;
		m_iUPS = 60;
	}
	
};

class PgMobileSuit 
	: public NiApplication
{
public:
	typedef std::map<std::string, std::string> CONT_PATH;		// < ID, Path >

public:
	//// Constructor & Destructor
	//
	PgMobileSuit();
	virtual ~PgMobileSuit();

	//// Overrides NiApplication
	//
	
	virtual bool Initialize();
	virtual void Terminate();
	virtual bool CreateRenderer();
	
	void	RecreateRenderer(bool bFullScreen, bool bBorderless, unsigned int uiNewWidth,unsigned int uiNewHeight,unsigned int uiRefreshRate,bool bVSync);
	void RendererChange(bool bFullScreen, NiDX9Renderer::PresentationInterval eFlag);

	virtual void OnIdle();

	virtual	void OnActivate(bool bActivate);
	virtual bool OnDefault(NiEventRef pEventRecord);
	virtual bool OnWindowResize(int iWidth, int iHeight, unsigned int uiSizeType, NiWindowRef pWnd);
	virtual bool OnWindowDestroy(NiWindowRef pWnd, bool bOption);
	void OnConfigChanged();
	
	TCHAR* GetConfigFileName() { return m_szConfigFile; }
	void stopAllThread();
	void terminateAllThread();

	void TakeScreenShot();
	unsigned long GetFrameCount();
	void IncFrameCount();
	bool IsActivated() { return m_bActivated; }
	bool IsInitialized() { return m_bInitialized; }
	void AddStringToWindowTitle(const wchar_t* pszTitle, int index);
	stDebugInfo	m_DebugInfo;

	DWORD GetEventViewElapsed() const { return m_dwEventViewElapsed; }

public :
	bool	m_bUsedEraseBackgroundMessage;
	typedef enum
	{
		INIT_WORK_ID_0,
		INIT_WORK_ID_1,
		INIT_WORK_ID_2,
		INIT_WORK_ID_3,
		INIT_WORK_ID_4,
		INIT_WORK_ID_5,
		INIT_WORK_ID_6,
		INIT_WORK_ID_7,
		INIT_WORK_ID_8,
		INIT_WORK_ID_9,
		INIT_WORK_ID_10,
		INIT_WORK_ID_11,
		INIT_WORK_ID_12,
		INIT_WORK_ID_13,
		INIT_WORK_ID_14,
		INIT_WORK_ID_15,
		INIT_WORK_ID_16,
		INIT_WORK_ID_17,
		INIT_WORK_ID_MAX,
	} InitializeWorkID;
protected:
/*
#ifdef PG_USE_THREAD_LOADING
	class InitializeTProc : public NiThreadProcedure
	{
	public:
		virtual unsigned int ThreadProcedure(void* pvArg);

		static NiFastCriticalSection ms_kCriticalSection;
		static float ms_fTime;
		static int ms_iInitializeWorkID;
		static bool ms_bTerminate;
	};

	InitializeTProc m_akInitializeTProcs[MAX_THREAD_NUM];
	NiThread* m_apkInitializeThreads[MAX_THREAD_NUM];
#endif
*/

	__int64 ReadPatchVer();

public:
	bool doInitializeWork(InitializeWorkID currentWorkID);
	void ToggleFrameRate();
	
	void readConfigFile();
	int GetConfigFromFileInt(LPCWSTR const& wstrFindSection, LPCWSTR const& wstrFindElement, BM::vstring& vstrResult, BM::vstring const& vstrDefault, std::wstring const& wstrFindSource);
	int GetConfigFromFileStr(LPCWSTR const& wstrFindSection, LPCWSTR const& wstrFindElement, LPCWSTR const& wstrDefault, LPWSTR& wstrResult, size_t iResultBufferSize, std::wstring const& wstrFindSource);

	void SetTitleByConfig();
	TCHAR* GetServerIP(){ return m_szServerIP; }
	WORD GetServerPort(){ return m_wServerPort; }
	void LoadServerInfo();

	std::wstring const getUserInfoPath()const;
	float GetWindowXScale() { return m_fWindowXScale; }
	float GetWindowYScale() { return m_fWindowYScale; }
	bool GetUseSmallWindow() { return m_bUseSmallWindow; }
	std::wstring& GetCashChargeURL() { return m_kCashChargeUrl; }

	void	SetVSync(bool bVSync)	{	m_bVSync = bVSync;	}
	bool	GetVSync()	const	{	return	m_bVSync;	}

	void	SetRefreshRate(unsigned int uiRefreshRate) { m_uiRefreshRate = uiRefreshRate;	}
	unsigned int GetRefreshRate()	const	{	return	m_uiRefreshRate;	}

	void ChangeStickyKeySetting();
	void RestoreStickyKeySetting();
	void SetSingleMode(bool const bSingleMode) { m_bSingleMode = bSingleMode; }
	bool IsSingleMode() const { return m_bSingleMode; }
	bool CheckLoginIP(std::wstring const& ServerIP)const;

protected:
	void initLogSystem();
	void writeConfigFile();
	bool createOutputModules();	
	bool checkDeviceCaps();
	void checkAdapter();
	void adjustGameOption();
	void adjustUrl();
	bool doInitScript();
	std::wstring const getScreenShotPath() const;
	void setMainThreadPriority(int iThreadPriority);
	void setBackgroundThreadPriority(int iThreadPriority, int iType);
	bool LoadAllDefStringXML(char const* pcFileName);
	void ParseLocalElement(TiXmlElement const* pkElement, CONT_PATH & rkContPath);
	//// Execution Parameters

	void AdjustWindowStyle(bool bFullScreen,int iWidth,int iHeight, bool bFitToScreen = false);
	//
	HANDLE m_hMutexHandle;
	TCHAR m_szConfigFile[MAX_PATH];
	bool m_bDebugMode;
	bool m_bSingleMode;
	unsigned long m_iFrameCount;
	unsigned int m_iMaxFrameRate;
	DWORD m_dwLastFrameTime;
	DWORD m_dwEventViewElapsed;
	unsigned int m_iWorkerThreadCount;
	unsigned int m_iMemoryWorkerThreadCount;
	int m_iFullScreenIni;
	bool m_bActivated;
	bool m_bInitialized;
	RECT m_kClientRect;
	RECT m_kWindowRect;
	POINT m_kWindowPos;
	bool m_bUseSmallWindow;
	float m_fWindowXScale;
	float m_fWindowYScale;
	bool	m_bVSync;
	unsigned	int	m_uiRefreshRate;
	std::map<int, std::wstring> m_kRankUrlMap;
	std::map<int, std::wstring> m_kCashChargeUrlMap;
	STICKYKEYS m_kStickyKeysBackup;
	FILTERKEYS m_kFilterKeysBackup;
	bool m_bKeySettingBackuped;

	CLASS_DECLARATION_S(int, UseGameGuard);
	CLASS_DECLARATION_S(bool, UseGSM);
	CLASS_DECLARATION_S(bool, UseWebLinkage);
	CLASS_DECLARATION_S(BM::VersionInfo, PatchVer);

	std::wstring m_kCashChargeUrl;
	TCHAR m_szServerIP[40];
	//int m_iServerPort;
	WORD m_wServerPort;

	//// Shader
	//
	ShaderHelper *m_pkShaderHelper;
	CLASS_DECLARATION(bool, m_bVisibleClassNo, VisibleClassNo);//PgActor에서 이름을 출력할때 이름옆에 ClassNo를 붙인다.
	CLASS_DECLARATION(bool, m_bVisibleTextDebug, VisibleTextDebug);
	CLASS_DECLARATION(bool, m_bVisibleQTextDebug, VisibleQTextDebug);

    // DrawWorkflow instance.
    NewWare::Renderer::DrawWorkflow* m_pkDrawWorkflow;

	POINT2 m_kStandardResolutionSize;	//창모드 일때, 작업표시줄로 인해 수정된 해상도 말고 모니터 해상도로 정해진 값(옵션에 해상도 목록에 나오는 해상도)
	void SetStandardResolutionSize(POINT2 const& rkSize);
	POINT2 GetStandardResolutionSize() const;

	MONITORINFO m_mi;

	void AdjustWindowRectForFullScreenToWindowMode(LPRECT lpWindowRect, LPRECT lpClientRect, unsigned int const iW, unsigned int const iH);
	void PreInitializeForWindowMode(unsigned int const iW, unsigned int const iH);

	typedef std::set< std::wstring > CONT_LOGIN_IP;

	CONT_LOGIN_IP m_LoginIPCont;

	void SetLoginIP();
	void SetInterLoginIP();
};

extern PgMobileSuit *g_pkApp;
PG_STAT_H(extern PgStatGroup g_kMobileSuitStatGroup);
PG_STAT_H(extern PgStatGroup g_kActorStatGroup);
PG_STAT_H(extern PgStatGroup g_kRenderManStatGroup);
PG_STAT_H(extern PgStatGroup g_kWorldStatGroup);
PG_STAT_H(extern PgStatGroup g_kUIStatGroup);
PG_STAT_H(extern PgStatGroup g_kScriptStatGroup);
PG_STAT_H(extern PgStatGroup g_kPilotStatGroup);
PG_STAT_H(extern PgStatGroup g_kLocalInputStatGroup);
PG_STAT_H(extern PgStatGroup g_kRemoteInputStatGroup);

inline unsigned long PgMobileSuit::GetFrameCount() { return m_iFrameCount; }

#endif // FREEDOM_DRAGONICA_APP_PGMOBILESUIT_H