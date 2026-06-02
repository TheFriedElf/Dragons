#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dbghelp.h"

#define __MAX_PATH	1024

class MiniDumper
{
protected:	
	static LONG WINAPI	Handler(struct _EXCEPTION_POINTERS *pExceptionInfo);

	_EXCEPTION_POINTERS	*m_pExceptionInfo;
	TCHAR				m_szDumpPath[__MAX_PATH];
	TCHAR				m_szAppPath[__MAX_PATH];
	TCHAR				m_szDumpTime[__MAX_PATH];
	TCHAR				m_szAppBaseName[__MAX_PATH];
	MINIDUMP_TYPE		m_eDumpType;

	virtual void VSetDumpFileName();
	virtual MINIDUMP_USER_STREAM_INFORMATION	*VGetUserStreamArray()	{	return NULL;	}

public:
	MiniDumper();
	TCHAR* GetAppBaseName() { return m_szAppBaseName; }
	TCHAR* GetDumpTime() { return m_szDumpTime; }
	void	SetMiniDumpType(MINIDUMP_TYPE eType) { m_eDumpType = eType; }
	static MiniDumper	*gpDumper;
	LONG				WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo);
};

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess,
			DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, 
			CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
			CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
			CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);