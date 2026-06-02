#include <windows.h>
#include <tlhelp32.h>
#include <ntsecapi.h>
#include <string>
#include <vector>
#include <map>

//#pragma comment(lib, "ntdll.lib")

// headers from http://chpie.tistory.com/entry/Blocking-Global-message-hook
// and http://dual5651.hacktizen.com/tc/entry/Anti-Game-hacking-%C7%C1%B7%CE%B1%D7%B7%A5%C0%CC-%B0%AE%C3%DF%BE%EE%BE%DF%C7%D2-%B1%E2%B4%C9%B5%E9
#define CWINHOOKS       (WH_MAX - WH_MIN + 1)

typedef struct tagPROCESSINFO * PPROCESSINFO;
typedef struct tagDESKTOP * PDESKTOP;
typedef struct tagDESKTOPINFO * PDESKTOPINFO;
typedef struct tagTHREADINFO * PTHREADINFO;
typedef struct tagHOOK * PHOOK;
typedef HWND * PWND;

typedef struct _HEAD {
	//    KHANDLE h;
	DWORD   h;
	DWORD   cLockObj;
} HEAD, *PHEAD;

typedef struct _THROBJHEAD {
	HEAD a;
	PTHREADINFO pti;
} THROBJHEAD, *PTHROBJHEAD;

typedef struct _DESKHEAD {
	PDESKTOP rpdesk;
	//    KPBYTE   pSelf;
	PVOID    pSelf;
} DESKHEAD, *PDESKHEAD;

typedef struct _THRDESKHEAD {
	THROBJHEAD a;
	DESKHEAD b;
} THRDESKHEAD, *PTHRDESKHEAD;

typedef struct tagHOOK {   /* hk */
	THRDESKHEAD     head;
	PHOOK           phkNext;
	int             iHook;              // WH_xxx hook type
	PVOID           offPfn;
	UINT            flags;              // HF_xxx flags
	int             ihmod;
	PTHREADINFO     ptiHooked;          // Thread hooked.
	PDESKTOP        rpdesk;             // Global hook pdesk. Only used when 
	//  hook is locked and owner is destroyed
} HOOK;

typedef struct tagDESKTOPINFO {

	PVOID  pvDesktopBase;          // For handle validation
	PVOID  pvDesktopLimit;         // ???
	PWND          spwnd;                 // Desktop window
	DWORD         fsHooks;                // Deskop global hooks
	PHOOK         aphkStart[CWINHOOKS + 1];  // List of hooks
	PWND          spwndShell;            // Shell window
	PPROCESSINFO  ppiShellProcess;        // Shell Process
	PWND          spwndBkGnd;            // Shell background window
	PWND          spwndTaskman;          // Task-Manager window
	PWND          spwndProgman;          // Program-Manager window
	PVOID         pvwplShellHook;         // see (De)RegisterShellHookWindow
	int           cntMBox;                // ???
} DESKTOPINFO;

typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;

typedef struct tagTHREADINFO {
	struct W32THREAD // dt win32k!_W32THREAD -r
	{
		PVOID pEThread;
		unsigned long RefCount;
		PVOID ptlW32;
		PVOID pgdiDcattr;
		PVOID pgdiBrushAttr;
		PVOID pUMPDObjs;
		PVOID pUMPDHeaps;
		unsigned long dwEngAcquireCount;
		PVOID pSemTable;
		PVOID pUMPDObj;
	} W32THREAD;

	//***************************************** begin: USER specific fields

	PVOID             ptl;                // Listhead for thread lock list
	// end :: 0x2c :: start
	PPROCESSINFO    ppi;                // process info struct for this thread
	PVOID              pq;                 // keyboard and mouse input queue
	PVOID             spklActive;         // active keyboard layout for this thread
	PVOID             pcti;             // Info that must be visible from client
	PDESKTOP        rpdesk;
	PDESKTOPINFO    pDeskInfo;          // Desktop info visible to client

	// ....omitted....

} THREADINFO, *PTHREADINFO;

typedef struct _TEB
{
	NT_TIB Tib;                         // 00h
	PVOID EnvironmentPointer;           // 1Ch
	CLIENT_ID Cid;                      // 20h
	PVOID ActiveRpcInfo;                // 28h
	PVOID ThreadLocalStoragePointer;    // 2Ch
	PVOID Peb;                           // 30h
	DWORD LastErrorValue;               // 34h
	DWORD CountOfOwnedCriticalSections; // 38h
	PVOID CsrClientThread;              // 3Ch
	PTHREADINFO Win32ThreadInfo;        // 40h
	DWORD Win32ClientInfo[0x1F];        // 44h
	PVOID WOW32Reserved;                // C0h
	DWORD CurrentLocale;                // C4h
	DWORD FpSoftwareStatusRegister;     // C8h
	PVOID SystemReserved1[0x36];        // CCh
	PVOID Spare1;                       // 1A4h
	LONG ExceptionCode;                 // 1A8h
	DWORD SpareBytes1[0x28];            // 1ACh
	PVOID SystemReserved2[0xA];         // 1D4h
	//   GDI_TEB_BATCH GdiTebBatch;          // 1FCh
	DWORD gdiRgn;                       // 6DCh
	DWORD gdiPen;                       // 6E0h
	DWORD gdiBrush;                     // 6E4h
	CLIENT_ID RealClientId;             // 6E8h
	PVOID GdiCachedProcessHandle;       // 6F0h
	DWORD GdiClientPID;                 // 6F4h
	DWORD GdiClientTID;                 // 6F8h
	PVOID GdiThreadLocaleInfo;          // 6FCh
	PVOID UserReserved[5];              // 700h
	PVOID glDispatchTable[0x118];       // 714h
	DWORD glReserved1[0x1A];            // B74h
	PVOID glReserved2;                  // BDCh
	PVOID glSectionInfo;                // BE0h
	PVOID glSection;                    // BE4h
	PVOID glTable;                      // BE8h
	PVOID glCurrentRC;                  // BECh
	PVOID glContext;                    // BF0h
	DWORD LastStatusValue;           // BF4h
	UNICODE_STRING StaticUnicodeString; // BF8h
	WCHAR StaticUnicodeBuffer[0x105];   // C00h
	PVOID DeallocationStack;            // E0Ch
	PVOID TlsSlots[0x40];               // E10h
	LIST_ENTRY TlsLinks;                // F10h
	PVOID Vdm;                          // F18h
	PVOID ReservedForNtRpc;             // F1Ch
	PVOID DbgSsReserved[0x2];           // F20h
	DWORD HardErrorDisabled;            // F28h
	PVOID Instrumentation[0x10];        // F2Ch
	PVOID WinSockData;                  // F6Ch
	DWORD GdiBatchCount;                // F70h
	DWORD Spare2;                       // F74h
	DWORD Spare3;                       // F78h
	DWORD Spare4;                       // F7Ch
	PVOID ReservedForOle;               // F80h
	DWORD WaitingOnLoaderLock;          // F84h
} TEB, *PTEB;

typedef PTEB (NTAPI* FuncNtCurrentTeb)();

NTSYSAPI NTSTATUS NTAPI NtQueryInformationProcess( 
	IN HANDLE ProcessHandle, 
	IN PROCESS_INFORMATION_CLASS ProcessInformationClass, 
	OUT PVOID ProcessInformation, 
	IN ULONG ProcessInformationLength, 
	OUT PULONG ReturnLength 
	);

NTSYSAPI NTSTATUS NTAPI NtSetInformationThread ( // ZeSetInformationThread 함수를 호출 
												IN HANDLE ThreadHandle,
												IN THREAD_INFORMATION_CLASS ThreadInformationClass, // 값을 0x11로 만들고 API 호출하면 Debugger가 중료된다.
												IN PVOID ThreadInformation,
												IN ULONG ThreadInformationLength
												); 


class CModule
{
public:
	CModule()					{ };
	CModule( MODULEENTRY32* pEntry );
	virtual ~CModule();

	DWORD GetProcessID()			{ return m_dwProcessID;			}
	DWORD GetModuleID()				{ return m_dwModuleID;			}
	HANDLE GetModuleHandle()		{ return m_hModule;			}

	const wchar_t* GetName()			{ return m_dllName.c_str();		}
	const wchar_t* GetPath()			{ return m_dllPath.c_str();		}


private:
	DWORD	m_dwProcessID;
	DWORD	m_dwModuleID;
	HANDLE	m_hModule;
	std::wstring m_dllName;
	std::wstring m_dllPath;	
};

typedef std::vector<CModule *>	VEC_MODULE;
typedef std::map< std::wstring /*module*/, std::wstring /*module*/ > MAP_MODULENAME;

class CProcess
{
public:
	CProcess()						{ Clear(); };
	CProcess( PROCESSENTRY32* pEntry );
	virtual ~CProcess();

	DWORD GetParentProcessID()		{ return m_dwParentProcessID;	}
	DWORD GetProcessID()			{ return m_dwProcessID;			}
	const wchar_t* GetName()		{ return m_exeFile.c_str();		}
	const int GetModuleCount()		{ return m_count;				}
	VEC_MODULE* GetModule()			{ return &m_ModuleList;			}
	CProcess* GetParent()				{ return m_pParent; }
	void SetParent(CProcess* pProcess)	{ m_pParent = pProcess; }

	bool Snapshot(MAP_MODULENAME* pkBadModules);
	void Clear();

private:

	DWORD m_dwParentProcessID;
	DWORD m_dwProcessID;
	DWORD m_dwModuleID;
	std::wstring m_exeFile;
	int	m_count;
	CProcess* m_pParent;
	
	VEC_MODULE	m_ModuleList;

	//PROCESSENTRY32* m_pEntry;
};


typedef std::vector<CProcess *> VEC_PROCESS;
typedef std::map< std::wstring /*process*/, std::wstring /*process*/ > MAP_PROCESSNAME;

class ProcessChecker
{
public:
	ProcessChecker();
	virtual ~ProcessChecker();

	int GetTotalCheckCount()			{ return m_iTotalCheckCount; }
	void SetModuleCheck(bool bCheck)	{ m_bModuleCheck = bCheck; }
	void SetSelfKill(bool bUse)			{ m_bSelfKill = bUse; }
	void SetSelfCheck(bool bUse)		{ m_bSelfCheck = bUse; }
	void SetDelay( const DWORD time )	{ m_delay = time; }
	bool Snapshot();
	void Clear();
	void ClearProcessList();
	void Init();

	void CheckProcessDebugged(bool bKillMySelf = true);
	int CheckBadProcess(bool bKillProcess = true);
	bool ParseXml( TiXmlDocument const *pkDoc );
	bool KillProcess( DWORD dwProcessId );
	void GetMyTeb();
	void UnloadBadModule(CProcess* pProcess);

protected:	
	void setParent(CProcess* pProcess);
	VEC_PROCESS m_ProcessList;
	int m_count;
	MAP_PROCESSNAME m_badProcess;
	MAP_MODULENAME m_badModule;
	DWORD m_delay;
	DWORD m_prevTime;
	bool m_bModuleCheck;
	bool m_bSelfKill;
	bool m_bSelfCheck;
	DWORD m_dwGameProcessID;
	int m_iTotalCheckCount;
	CProcess* m_pGameProcess;
};

//extern ProcessChecker g_kProcessChecker;