
#include "stdafx.h"
#include <tchar.h>
#include "dbghelp.h"
#include "MiniDump.h"
#include "../PgAppProtect.h"

#define __MAX_PATH	1024

MiniDumper *MiniDumper::gpDumper = NULL;

MiniDumper::MiniDumper()
{
	//MiniDumper가 또 있는지 점검
	_ASSERT(!gpDumper && "gpDumper 없음!!");
	if(!gpDumper)
	{
//		::SetUnhandledExceptionFilter(Handler);
		gpDumper = this;
	}
	m_eDumpType = MiniDumpNormal;
}

LONG MiniDumper::Handler(_EXCEPTION_POINTERS *pExceptionInfo)
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	if(!gpDumper)
	{
		return retval;
	}
	return gpDumper->WriteMiniDump(pExceptionInfo);
}

LONG MiniDumper::WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo)
{
	g_kAppProtect.UnInit();

	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	//정확한 dbghelp.dll을 찾아야 한다.
	//System32에 있는 것이 오래된 것(Win2K)일 수 있으므로, exe가 있는 
	//디렉토리를 먼저 뒤져야 한다. 즉, 배포본에는 따로 DBGHELP.DLL을 
	//포함시켜 디버깅할 수 있다는 얘기.

	HMODULE hDll = NULL;
	TCHAR szDbgHelpPath[__MAX_PATH];

	if(GetModuleFileName(NULL, m_szAppPath, __MAX_PATH))
	{
		TCHAR *pSlash = _tcsrchr(m_szAppPath, '\\');
		if(pSlash)
		{
			_tcscpy(m_szAppBaseName, pSlash+1);
			*(pSlash+1) = 0;
		}

		_tcscpy(szDbgHelpPath, m_szAppPath);
		_tcscat(szDbgHelpPath, _T("DBGHELP.DLL"));
		hDll = ::LoadLibrary(szDbgHelpPath);
	}
	if(hDll == NULL)
	{
		//못 찾았음. 기본 DLL 로드한다.
		hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
	}

	LPCTSTR szResult = NULL;

	if(hDll)
	{
		MINIDUMPWRITEDUMP pMiniDumpWriteDump = 
			(MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");

		if(pMiniDumpWriteDump)
		{
			TCHAR szScratch[__MAX_PATH];

			VSetDumpFileName();

			//사용자에게 덤프 파일을 저장할 것인지 묻는다. 디버그 버전일 경우 무조건 수행.
//#ifdef _DEBUG
//			if(::MessageBox(NULL, _T("에러 발생! 덤프 파일을 저장하시겠습니까?"), NULL, MB_YESNO)==IDYES)
//#endif
			{
				//덤프 파일 생성
				HANDLE hFile = ::CreateFile(m_szDumpPath, 
											GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
											CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if(hFile != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					//덤프 기록
					BOOL bOK = pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
													hFile, m_eDumpType, &ExInfo, 
													VGetUserStreamArray(), NULL);

					if(bOK)
					{
						szResult = NULL;
						retval = EXCEPTION_EXECUTE_HANDLER;
						OutputDebugString(TEXT("MiniDump::Create success\n"));
					}
					else
					{
						LPVOID	pMsg;

						::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsg, 0, NULL);

						wsprintf(szScratch, _T("덤프 파일 '%s' 저장 실패(에러 %s)"), m_szDumpPath, pMsg);
						szResult = szScratch;
					}
					::CloseHandle(hFile);
				}
				else
				{
					LPVOID	pMsg;

					::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsg, 0, NULL);

					wsprintf(szScratch, _T("덤프 파일 '%s' 생성 실패(에러 %s)"), m_szDumpPath, pMsg);
					szResult = szScratch;
				}
			}
		}
		else
		{
			szResult = _T("DBGHELP.DLL이 구버전입니다");
		}
	}
	else
	{
		szResult = _T("DBGHELP.DLL을 찾지 못했습니다");
	}
	if(szResult)
	{
#ifndef EXTERNAL_RELEASE
		::MessageBox(NULL, szResult, NULL, MB_OK);
#endif
		OutputDebugString(szResult);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	return retval;
}

void MiniDumper::VSetDumpFileName()
{
	SYSTEMTIME st = {0, };
	::GetLocalTime(&st);

	_stprintf_s(m_szDumpTime, __MAX_PATH, _T("_%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	_tcscpy(m_szDumpPath, m_szAppPath);
	_tcscat(m_szDumpPath, PG_EXCEPTION_FOLDER TEXT("/"));
	_tcscat(m_szDumpPath, m_szAppBaseName);
	_tcscat(m_szDumpPath, m_szDumpTime);
	_tcscat(m_szDumpPath, _T(".dmp"));
}