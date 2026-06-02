#include "stdafx.h"
#include "ProcessChecker.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <mmsystem.h>

#pragma comment(lib,"winmm.lib")

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

CModule::CModule( MODULEENTRY32* pEntry )
{
	m_dwProcessID = pEntry->th32ProcessID;
	m_dwModuleID = pEntry->th32ModuleID;
	m_hModule = pEntry->hModule;

	m_dllName = pEntry->szModule;
	m_dllPath = pEntry->szExePath;
}

CModule::~CModule()
{
}

CProcess::~CProcess()
{
	Clear();
}

CProcess::CProcess( PROCESSENTRY32* pEntry )
	: m_count(0)
{
	Clear();

	m_dwParentProcessID = pEntry->th32ParentProcessID;
	m_dwProcessID = pEntry->th32ProcessID;
	m_dwModuleID = pEntry->th32ModuleID;

	TCHAR * pBuf = _tcsrchr( pEntry->szExeFile, _T('\\') );
	if ( pBuf != NULL )
		m_exeFile = &pBuf[1];
	else
		m_exeFile = pEntry->szExeFile;
}

void CProcess::Clear()
{
	VEC_MODULE::iterator itor;
	CModule* pModule = NULL;

	for ( itor = m_ModuleList.begin(); itor != m_ModuleList.end(); ++itor )
	{
		pModule = (CModule*)(*itor);
		SAFE_DELETE( pModule );
	}
	m_ModuleList.clear();

	m_count = 0;
	m_dwParentProcessID = 0;
	m_dwProcessID = 0;
	m_exeFile.clear();
	m_pParent = NULL;
}

bool CProcess::Snapshot(MAP_MODULENAME* pkBadModules)
{
	if (pkBadModules == NULL)
		return false;

	HANDLE hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, m_dwProcessID );
	if ( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		return true;
	}

	MODULEENTRY32 me32 = { sizeof(me32), };
	CModule* pModule;

	while ( Module32Next( hModuleSnap, &me32 ) )
	{
		std::wstring module_name = me32.szModule;
		std::transform( module_name.begin(), module_name.end(), module_name.begin(), toupper );
		MAP_MODULENAME::iterator module_itor = pkBadModules->find(module_name);
		if (module_itor != pkBadModules->end())
		{
			pModule = new CModule( &me32 );
			m_ModuleList.push_back( pModule );
			++m_count;
		}
	}

	CloseHandle( hModuleSnap );
	return true;
}

ProcessChecker::ProcessChecker() 
	: m_count(0), m_delay(1000), m_prevTime(0), m_bModuleCheck(false),
	m_dwGameProcessID(0), m_bSelfKill(false), m_bSelfCheck(false), m_iTotalCheckCount(0), m_pGameProcess(NULL)
{
}

ProcessChecker::~ProcessChecker()
{
	Clear();
}

void ProcessChecker::Init()
{
	m_dwGameProcessID = GetCurrentProcessId();
}

void ProcessChecker::Clear()
{
	ClearProcessList();
	m_delay = 0;
	m_prevTime = 0;
	m_badProcess.clear();
	m_badModule.clear();
	m_dwGameProcessID = 0;
	m_iTotalCheckCount = 0;
	m_pGameProcess = NULL;
}

void ProcessChecker::ClearProcessList()
{
	VEC_PROCESS::iterator itor;
	CProcess* pProcess;

	for ( itor = m_ProcessList.begin(); itor != m_ProcessList.end(); ++itor )
	{
		pProcess = (CProcess*)(*itor);
		SAFE_DELETE( pProcess );
	}

	m_ProcessList.clear();
	m_count = 0;
}

bool ProcessChecker::Snapshot()
{
	ClearProcessList();

	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	
	if ( hProcessSnap == INVALID_HANDLE_VALUE ) 
	{
		//throw std::runtime_error( "Can't snapshot process\n" );
	}

	PROCESSENTRY32 pe32 = { sizeof(pe32), };
	CProcess* pProcess;

	for ( BOOL ok = Process32First(hProcessSnap, &pe32); ok; ok = Process32Next(hProcessSnap, &pe32) )
	{
		pProcess = new CProcess( &pe32 );
		m_ProcessList.push_back( pProcess );
		if (pProcess->GetProcessID() == m_dwGameProcessID)
		{
			m_pGameProcess = pProcess;
		}

		if (m_bModuleCheck || pProcess->GetProcessID() == m_dwGameProcessID)
		{
			pProcess->Snapshot(&m_badModule);
		}

		++m_count;
	}

	CloseHandle( hProcessSnap );

	for (int i=0; i<m_count; ++i)
	{
		setParent(m_ProcessList[i]);
	}

	return true;
}

bool ProcessChecker::ParseXml(TiXmlDocument const *pkDoc)
{
	if (pkDoc == NULL)
		return false;

	TiXmlElement* root = const_cast<TiXmlElement*>(pkDoc->RootElement());
	TiXmlNode* child;

	std::string rootName = root->Value();
	
	if ( rootName == "BAD_PROCESS" )
	{
		std::string tag;
		std::wstring process_name;
		std::wstring module_name;

		for ( child=root->FirstChild(); child != NULL; child=child->NextSibling() )
		{
			tag = child->Value();
			if ( tag == "PROCESS" )
			{
				char const* pkProcessName = child->ToElement()->Attribute("NAME");

				if (pkProcessName == NULL)
					continue;

				process_name = UNI(std::string(pkProcessName));
				UPR(process_name);
				m_badProcess[process_name] = process_name;
			}
			else if (tag == "MODULE")
			{
				char const* pkModuleName = child->ToElement()->Attribute("NAME");
				if (pkModuleName == NULL)
					continue;

				module_name = UNI(std::string(pkModuleName));
				UPR(module_name);
				m_badModule[module_name] = module_name;
			}
		}
	}

	return true;
}


void ProcessChecker::CheckProcessDebugged(bool bKillMySelf)
{

	// reference  : http://www.securityfocus.com/comments/infocus/1893/959/threaded#959
	// reference2 : http://www.sysnet.pe.kr/Default.aspx?mode=2&sub=1&detail=1&wid=765
	//1st
	if (IsDebuggerPresent())
	{
#ifdef EXTERNAL_RELEASE
		goto __FOUND;
#else
		NILOG(PGLOG_WARNING, "Found Debugger 1\n");
#endif
	}

	BOOL bRet = FALSE;
  
	__asm
	{
		push EAX
	    MOV EAX, FS:[00000018H]
	    MOV EAX, DWORD PTR [EAX+030H]
	    MOVZX EAX, BYTE PTR [EAX+002H]
	    MOV bRet, EAX
		pop EAX
	}

	if (bRet)
	{
#ifdef EXTERNAL_RELEASE
		goto __FOUND;
#else
		NILOG(PGLOG_WARNING, "Found Debugger 2\n");
#endif
	}
	
//	bRet = FALSE;
//	CheckRemoteDebuggerPresent(GetCurrentProcess(), &bRet);
//	if (bRet)
//	{
//#ifdef EXTERNAL_RELEASE
//		goto __FOUND;
//#else
//		NILOG(PGLOG_WARNING, "Found Debugger 3\n");
//#endif
//	}

//	__asm
//	{
//		push eax
//		mov eax, fs:[30h] 
//		mov eax, [eax+2] 
//		test eax, eax 
//		jne __FOUND
//		pop eax
//	}
//#ifndef EXTERNAL_RELEASE
//	NILOG(PGLOG_WARNING, "Can't Found Debugger 4\n");
//#endif

	// ntdll.lib이 필요.
	//__asm
	//{
	//	push 0 
	//	push 0 
	//	push 11h ;ThreadHideFromDebugger 
	//	push -2 
	//	call NtSetInformationThread

	//}

__NOTFOUND:
	return;

__FOUND:
#ifdef EXTERNAL_RELEASE
	if (bKillMySelf)
		KillProcess(m_dwGameProcessID);
#else
	NILOG(PGLOG_WARNING, "Found Debugger 4\n");
#endif
	return;
}

int ProcessChecker::CheckBadProcess(bool bKillProcess)
{
	//// check delay time
	//DWORD dwCurrTime = BM::GetTime32();

	//if ( dwCurrTime > m_prevTime )
	//{		
	//	m_prevTime = dwCurrTime + (m_delay == 0 ? 1000 : m_delay );
		Snapshot();
	//}
	//else
	//{
	//	return -1;	// skip
	//}


	if (m_pGameProcess)
	{
		if (m_pGameProcess->GetModuleCount() > 0)
		{
			// 이건 아래서 걸림.
		}
		else
		{
			CProcess* pParent = m_pGameProcess->GetParent();
			while (pParent != NULL)
			{
				if (pParent->GetModuleCount() > 0)
				{
					break;
				}

				if (pParent->GetName())
				{
				}

				pParent = pParent->GetParent();
			}			
		}
	}

	int iCount = 0;
	for ( int i=0; i<m_count; ++i )
	{
		bool bChecked = false;
		if (m_ProcessList[i]->GetModuleCount() > 0)
		{
			bChecked = true;
		}
		else
		{
			std::wstring process_name = m_ProcessList[i]->GetName();
			
			std::transform( process_name.begin(), process_name.end(), process_name.begin(), toupper );

			MAP_PROCESSNAME::iterator itor = m_badProcess.find( process_name );
			
			if (itor != m_badProcess.end())
			{
				bChecked = true;
			}
		}

		if (bKillProcess && bChecked)
		{
			m_iTotalCheckCount++;
			if (m_ProcessList[i]->GetProcessID() == m_dwGameProcessID && m_bSelfCheck == false)
			{
				if (m_ProcessList[i]->GetModuleCount() > 0)
				{
					UnloadBadModule(m_ProcessList[i]);
				}
				continue;
			}

			NILOG(PGLOG_WARNING, "Check Bad Process %s.. trying to kill(%d)\n", MB(m_ProcessList[i]->GetName()), m_ProcessList[i]->GetModuleCount());
			//KillProcess(m_ProcessList[i]->GetProcessID());

			if (m_bSelfKill && m_dwGameProcessID > 0)
			{
				if (KillProcess(m_dwGameProcessID))
				{
					m_dwGameProcessID = 0;
				}
			}
		}
	}	
	
	return m_iTotalCheckCount;
}

bool ProcessChecker::KillProcess( const DWORD dwProcessId )
{
	if (dwProcessId == 0)
		return false;

	if (dwProcessId == m_dwGameProcessID)
	{
		if (m_bSelfKill == false)
			return false;

		m_dwGameProcessID = 0;
	}

	HANDLE hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessId );

	if ( hProcess != NULL )
	{
		if ( TerminateProcess( hProcess, 0 ) )
			return true;
	}

	return false;
}

void ProcessChecker::setParent(CProcess* pProcess)
{
	if (pProcess == NULL)
		return;
	
	for (int i=0; i<m_count; ++i)
	{
		if (m_ProcessList[i] == pProcess)
			continue;

		if (m_ProcessList[i]->GetProcessID() == pProcess->GetParentProcessID())
		{
			pProcess->SetParent(m_ProcessList[i]);
			return;
		}
	}
}


void ProcessChecker::GetMyTeb()
{
	FuncNtCurrentTeb ngt = (FuncNtCurrentTeb)GetProcAddress( GetModuleHandle( _T("ntdll.dll") ), "NtCurrentTeb" );
	PTEB pTeb = ngt();
	if (pTeb == NULL)
		return;

	PTHREADINFO pTi = pTeb->Win32ThreadInfo;
	if (pTi == NULL)
		return;

	PDESKTOPINFO pDeskInfo = pTi->pDeskInfo;
	if (pDeskInfo == NULL)
		return;	
}

void ProcessChecker::UnloadBadModule(CProcess* pProcess)
{
	if (pProcess == NULL)
		return;

	if (pProcess->GetModuleCount() <= 0)
		return;

	for (int i = 0; i < pProcess->GetModuleCount(); ++i)
	{
		CModule* pModule = pProcess->GetModule()->at(i);
		if (pModule)
		{
			HMODULE hModule = GetModuleHandle(pModule->GetName());
			if (FreeLibrary(hModule))
			{
				NILOG(PGLOG_WARNING, "%s module is unloaded from %s process\n", MB(pModule->GetName()), MB(pProcess->GetName()));
			}
		}
	}
}


/*
void zzzSetWindowsHookEx(
						  HANDLE hmod,
						  PUNICODE_STRING pstrLib,
						  PTHREADINFO ptiThread,
						  DWORD dwFlags)
{
	ACCESS_MASK amDesired;
	PHOOK phkNew;
	TL tlphkNew;
	PHOOK *pphkStart;
	PTHREADINFO ptiCurrent;

	ptiCurrent = PtiCurrent();

	if (ptiThread == NULL) {
		return;
	}

	return phkNew;
}

*/