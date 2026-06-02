#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <malloc.h>

#define max(a, b)  (((a) > (b)) ? (a) : (b))

#include "ErrorReportFile.h"
#include "ExceptionHandler.h"
#include "Lohengrin/BuildNumber.h"
#include "../PgError.h"
#include "../PgMobileSuit.h"
#include "MiniDump.h"

const unsigned int EXCEPTIONHANDLER_BUFFSIZE = (4096 * 4);
const unsigned int IMGAGEHLP_BUFFSIZE = (1024 * 63);

#ifndef EH_USE_NO_EXCEPTION
class CLogCritical {
	static CRITICAL_SECTION m_CriticalSection;
public:
	CLogCritical();
	~CLogCritical();

	static void Initialize(void)	{ InitializeCriticalSection(&m_CriticalSection); }
	static void Uninitialize(void)	{ DeleteCriticalSection(&m_CriticalSection); }
};

CRITICAL_SECTION CLogCritical::m_CriticalSection;

CLogCritical::CLogCritical() {
	EnterCriticalSection(&m_CriticalSection);
}

CLogCritical::~CLogCritical() {
	LeaveCriticalSection(&m_CriticalSection);
}

class CLogInitialize {
public:
	CLogInitialize()	{ CLogCritical::Initialize(); }
	~CLogInitialize()	{ CLogCritical::Uninitialize(); }
};

CLogInitialize g_LogInitialize;

//============================== Global Variables =============================

//
// Declare the static variables of the MSJExceptionHandler class
//
LPTOP_LEVEL_EXCEPTION_FILTER MSJExceptionHandler::m_pPreviousFilter = NULL;
_se_translator_function MSJExceptionHandler::mPrevTranslatorFunc = NULL;
_purecall_handler MSJExceptionHandler::mPrevPureHandler = NULL;	
_PNH MSJExceptionHandler::mPrevNewHandler = NULL;
_invalid_parameter_handler MSJExceptionHandler::mPrevInvalidParameterHandler = NULL;
terminate_handler MSJExceptionHandler::mPrevTerminateHandler;
unexpected_handler MSJExceptionHandler::mPrevUnexpectedHandler;

HMODULE MSJExceptionHandler::m_hModImagehlp = 0;
MSJExceptionHandler::SYMINITIALIZE				MSJExceptionHandler::_SymInitialize = 0;
MSJExceptionHandler::SYMCLEANUP					MSJExceptionHandler::_SymCleanup = 0;
MSJExceptionHandler::STACKWALK					MSJExceptionHandler::_StackWalk = 0;
MSJExceptionHandler::SYMFUNCTIONTABLEACCESS		MSJExceptionHandler::_SymFunctionTableAccess = 0;
MSJExceptionHandler::SYMGETMODULEBASE			MSJExceptionHandler::_SymGetModuleBase = 0;
MSJExceptionHandler::SYMGETSYMFROMADDR			MSJExceptionHandler::_SymGetSymFromAddr = 0;
MSJExceptionHandler::SYMGETLINEFROMADDR			MSJExceptionHandler::_SymGetLineFromAddr = 0;
MSJExceptionHandler::SYMGETOPTIONS				MSJExceptionHandler::_SymGetOptions = 0;
MSJExceptionHandler::SYMSETOPTIONS				MSJExceptionHandler::_SymSetOptions = 0;

// Declare global instance of class
MSJExceptionHandler g_MSJExceptionHandler;
MiniDumper g_kMiniDumper;

//============================== Class Methods =============================

//=============
// Constructor 
//=============
MSJExceptionHandler::MSJExceptionHandler()
{
}

//============
// Destructor 
//============
MSJExceptionHandler::~MSJExceptionHandler()
{
	UninstallExceptionHandler();
}

void MSJExceptionHandler::InstallExceptionHandler(bool bSetException)
{
	// Install the unhandled exception filter function
#ifdef EH_USE_SET_EXCEPTION
	if (bSetException)
		m_pPreviousFilter = SetUnhandledExceptionFilter(MSJUnhandledExceptionFilter);
#endif
	if (bSetException)
		mPrevTranslatorFunc = _set_se_translator(trans_func);
	mPrevPureHandler = _set_purecall_handler(purecall_handler);
	mPrevNewHandler= _set_new_handler(_new_handler);
	mPrevInvalidParameterHandler= _set_invalid_parameter_handler(invalid_parameter_handler);
	mPrevTerminateHandler = set_terminate(terminate_handler_func);
	mPrevUnexpectedHandler = set_unexpected(unexpected_handler_func);
}

void MSJExceptionHandler::UninstallExceptionHandler()
{
#ifdef EH_USE_SET_EXCEPTION
	if (m_pPreviousFilter)
	{
		SetUnhandledExceptionFilter(m_pPreviousFilter);
		m_pPreviousFilter = NULL;
	}
#endif
	if (mPrevTranslatorFunc)
	{
		_set_se_translator(mPrevTranslatorFunc);
		mPrevTranslatorFunc = NULL;
	}

	if (mPrevPureHandler)
	{
		_set_purecall_handler(mPrevPureHandler);
		mPrevPureHandler = NULL;
	}

	if (mPrevNewHandler)
	{
		_set_new_handler(mPrevNewHandler);
		mPrevNewHandler = NULL;
	}

	if (mPrevInvalidParameterHandler)
	{
		_set_invalid_parameter_handler(mPrevInvalidParameterHandler);
		mPrevInvalidParameterHandler = NULL;
	}

	if (mPrevTerminateHandler)
	{
		set_terminate(mPrevTerminateHandler);
		mPrevTerminateHandler = NULL;
	}

	if (mPrevUnexpectedHandler)
	{
		set_unexpected(mPrevUnexpectedHandler);
		mPrevUnexpectedHandler = NULL;
	}

	if (m_hModImagehlp)
	{
		FreeLibrary(m_hModImagehlp);
		m_hModImagehlp = 0;
	}
}


//===========================================================
// Entry point where control comes on an unhandled exception 
//===========================================================
LONG WINAPI MSJExceptionHandler::MSJUnhandledExceptionFilter(PEXCEPTION_POINTERS lpExceptionInfo)
{
	g_kCoreCenter.Close();
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
#endif
	GenerateExceptionReport(lpExceptionInfo);
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
/*
	if (m_pPreviousFilter)
		return m_pPreviousFilter(lpExceptionInfo);
	else
		return EXCEPTION_CONTINUE_SEARCH;
*/
	return EXCEPTION_EXECUTE_HANDLER;
}

//===========================================================================
// Open the report file, and write the desired information to it.  
// Called by MSJUnhandledExceptionFilter
//===========================================================================
void MSJExceptionHandler::GenerateExceptionReport(PEXCEPTION_POINTERS lpExceptionInfo)
{
//	assert(g_szLogPath);
	CLogCritical cs;

	SETDIR_ERROR_REPORT(PG_EXCEPTION_FOLDER);

	if(MiniDumper::gpDumper != NULL)
	{
		OutputDebugString(TEXT("GenerateExceptionReport::MakeMinidump\n"));
		MiniDumper::gpDumper->WriteMiniDump(lpExceptionInfo);
	}

	OutputDebugString(TEXT("GenerateExceptionReport::Start\n"));
	TCHAR fileName[1024] = { 0, };

	_tcscat(fileName, MiniDumper::gpDumper->GetAppBaseName());
	_tcscat(fileName, MiniDumper::gpDumper->GetDumpTime());

	ERROR_REPORT_PTR->SetFileName(fileName);
	HANDLE hFile = ERROR_REPORT_PTR->Create();
	if (INVALID_HANDLE_VALUE == hFile)
	{
		OutputDebugString(TEXT("GenerateExceptionReport::Create Failed\n"));
		return;
	}

#ifdef _UNICODE
	DWORD dwWritten;
	dwWritten = SetFilePointer(hFile, 0, 0, FILE_END);
	if (0 == dwWritten) {
		TCHAR mark = 0xFEFF;
		WriteFile(hFile, &mark, sizeof(TCHAR), &dwWritten, NULL);
	}
#else
	SetFilePointer(hFile, 0, 0, FILE_END);
#endif

	TCHAR szBuffer[EXCEPTIONHANDLER_BUFFSIZE];
	int nNumberOfBytes = 0;

	// Start out with a banner
	SYSTEMTIME st = {0, };
	::GetLocalTime(&st);
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes, _T("[Procedure Infomation]\r\n"));
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
			_T("Version=%s, %s-%s, %s\r\n")
			_T("ProcessID=%d\r\n")
			_T("ThreadID=%d\r\n")
			_T("Date=\"%04d/%02d/%02d %02d:%02d:%02d\"\r\n"),
			_T(VERSION_FILEVERSION_CLIENT_STR), _T(__DATE__), _T(__TIME__),
			_T("")
#ifdef EXTERNAL_RELEASE
			_T("E")
#endif
#ifdef USE_INB
			_T("B")
#endif
			,
			GetCurrentProcessId(), GetCurrentThreadId(),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	EWriteFile(hFile, szBuffer);
	OutputDebugString(szBuffer);

	// Show reasons
	WriteFaultReason(hFile, lpExceptionInfo);

	// Show registers
	WriteRegisters(hFile, lpExceptionInfo);

	if (InitializeImagehlpFunctions()) {
		HANDLE hProcess = GetCurrentProcess();
		if (_SymInitialize(hProcess, NULL, TRUE)) {
			ImagehlpStackWalk(hFile, lpExceptionInfo);

			_SymCleanup(hProcess);
		}
	}
	else {
		OutputDebugString(_T("IMAGEHLP.DLL or its exported procs not found"));

#ifdef _M_IX86  // Intel Only!
		// Walk the stack using x86 specific code
		IntelStackWalk(hFile, lpExceptionInfo->ContextRecord);
#endif
	}

	EWriteFile(hFile, _T("\r\n"));
	OutputDebugString(_T("\r\n"));

	ERROR_REPORT_PTR->Close();
}

//===========================================================================
void MSJExceptionHandler::WriteFaultReason(const HANDLE hFile, PEXCEPTION_POINTERS lpExceptionInfo) {
	PEXCEPTION_RECORD pExceptionRecord = lpExceptionInfo->ExceptionRecord;

	TCHAR szBuffer[EXCEPTIONHANDLER_BUFFSIZE];
	int nNumberOfBytes = 0;

	// First print information about the type of fault
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes, _T("\r\n[Exception Information]\r\n"));
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
			_T("ExceptionDescription=\"%s\"\r\n")
			_T("ExceptionCode=%08X\r\n"),
			GetExceptionString(pExceptionRecord->ExceptionCode),
			pExceptionRecord->ExceptionCode);

	// Now print information about where the fault occured
	TCHAR szModule[MAX_PATH];
	DWORD dwSection, dwOffset;
	GetLogicalAddress(pExceptionRecord->ExceptionAddress, szModule, sizeof(szModule), dwSection, dwOffset);

#ifdef _WIN64
	nNumberOfBytes += _stprintf(szBuffer + nNumberOfBytes,
			_T("ExceptionAddress=%016X\r\n")
			_T("ModuleName=%s\r\n"),
			pExceptionRecord->ExceptionAddress, szModule);
#else
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
			_T("ExceptionAddress=%04X:%08X\r\n")
			_T("ModuleName=%s\r\n"),
			lpExceptionInfo->ContextRecord->SegCs,
			(DWORD)pExceptionRecord->ExceptionAddress, szModule);
#endif

	if (STATUS_ACCESS_VIOLATION == pExceptionRecord->ExceptionCode && pExceptionRecord->NumberParameters >= 2) {
		nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
				_T("AccessViolationReason=%s\r\n")
				_T("AccessViolationlocation=%08X\r\n"),
				(pExceptionRecord->ExceptionInformation[0]) ? _T("Write") : _T("Read"),
				pExceptionRecord->ExceptionInformation[1]);
	}

	OutputDebugString(szBuffer);
	EWriteFile(hFile, szBuffer);
}

//===========================================================================
void MSJExceptionHandler::WriteRegisters(const HANDLE hFile, PEXCEPTION_POINTERS lpExceptionInfo) {
	PCONTEXT pContextRecord = lpExceptionInfo->ContextRecord;

	TCHAR szBuffer[EXCEPTIONHANDLER_BUFFSIZE];
	int nNumberOfBytes = 0;

#ifdef _M_IX86  // Intel Only!
	// This call puts 48 bytes on the stack, which could be a problem when the stack is blown.
	
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes, _T("\r\n[Register Information]\r\n"));
	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
			_T("EAX=%08X\r\n") _T("EBX=%08X\r\n") _T("ECX=%08X\r\n")
			_T("EDX=%08X\r\n") _T("ESI=%08X\r\n") _T("EDI=%08X\r\n")
			_T("EBP=%08X\r\n") _T("ESP=%08X\r\n") _T("EIP=%08X\r\n")
			_T("FLG=%08X\r\n") _T("CS=%04X\r\n") _T("DS=%04X\r\n")
			_T("SS=%04X\r\n") _T("ES=%04X\r\n") _T("FS=%04X\r\n")
			_T("GS=%04X\r\n"), 
			pContextRecord->Eax, pContextRecord->Ebx, pContextRecord->Ecx, 
			pContextRecord->Edx, pContextRecord->Esi, pContextRecord->Edi, 
			pContextRecord->Ebp, pContextRecord->Esp, pContextRecord->Eip, 
			pContextRecord->EFlags, pContextRecord->SegCs, pContextRecord->SegDs, 
			pContextRecord->SegSs, pContextRecord->SegEs, pContextRecord->SegFs, 
			pContextRecord->SegGs);
#endif
	//{
	//	nNumberOfBytes += _stprintf(szBuffer + nNumberOfBytes, _T("Bytes at CS:EIP: "));
	//	unsigned char *szCode = (unsigned char *)pContextRecord->Eip;
	//	for (int nCodeByte = 0; nCodeByte < 16; nCodeByte++) {
	//		if (!IsBadReadPtr(szCode+nCodeByte, 1))
	//			nNumberOfBytes += _stprintf(szBuffer + nNumberOfBytes, _T("%02x "), szCode[nCodeByte]);
	//		else
	//			nNumberOfBytes += _stprintf(szBuffer + nNumberOfBytes, _T("?? "));
	//	}
	//	nNumberOfBytes += _stprintf(szBuffer + nNumberOfBytes, _T("\r\n"));
	//}

	EWriteFile(hFile, szBuffer);
	OutputDebugString(szBuffer);
}

//======================================================================
// Given an exception code, returns a pointer to a static string with a 
// description of the exception										 
//======================================================================
LPTSTR MSJExceptionHandler::GetExceptionString(DWORD dwCode) {
#define EXCEPTION(x) case EXCEPTION_##x: _tcscat(buf, _T("EXCEPTION_")); _tcscat(buf, _T(#x)); return buf;
	TCHAR buf[64]{};

	switch(dwCode) {
		EXCEPTION(ACCESS_VIOLATION)
		EXCEPTION(DATATYPE_MISALIGNMENT)
		EXCEPTION(BREAKPOINT)
		EXCEPTION(SINGLE_STEP)
		EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
		EXCEPTION(FLT_DENORMAL_OPERAND)
		EXCEPTION(FLT_DIVIDE_BY_ZERO)
		EXCEPTION(FLT_INEXACT_RESULT)
		EXCEPTION(FLT_INVALID_OPERATION)
		EXCEPTION(FLT_OVERFLOW)
		EXCEPTION(FLT_STACK_CHECK)
		EXCEPTION(FLT_UNDERFLOW)
		EXCEPTION(INT_DIVIDE_BY_ZERO)
		EXCEPTION(INT_OVERFLOW)
		EXCEPTION(PRIV_INSTRUCTION)
		EXCEPTION(IN_PAGE_ERROR)
		EXCEPTION(ILLEGAL_INSTRUCTION)
		EXCEPTION(NONCONTINUABLE_EXCEPTION)
		EXCEPTION(STACK_OVERFLOW)
		EXCEPTION(INVALID_DISPOSITION)
		EXCEPTION(GUARD_PAGE)
		EXCEPTION(INVALID_HANDLE)
	}

	// If not one of the "known" exceptions, try to get the string from NTDLL.DLL's message table.
	static TCHAR szBuffer[512] = {0, };
	FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle(_T("NTDLL.DLL")), dwCode, 0, szBuffer, sizeof(szBuffer), 0);

	return szBuffer;
}

//==============================================================================
// Given a linear address, locates the module, section, and offset containing that address.															   
//																			 
// Note: the lpszModule paramater buffer is an output buffer of length specified by the dwLength parameter (in characters!)									   
//==============================================================================
BOOL MSJExceptionHandler::GetLogicalAddress(PVOID lpvAddress, LPTSTR lpszModule, DWORD dwLength, DWORD& dwSection, DWORD& dwOffset) {
	BOOL bResult = FALSE;

	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(lpvAddress, &mbi, sizeof(mbi))) {
		DWORD dwMod = (DWORD)mbi.AllocationBase;
		if (GetModuleFileName((HMODULE)dwMod, lpszModule, dwLength)) {
			// Point to the DOS header in memory
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)dwMod;

			// From the DOS header, find the NT (PE) header
			PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(dwMod + pDosHeader->e_lfanew);

			PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeader);

			DWORD rva = (DWORD)lpvAddress - dwMod; // RVA is offset from module load address

			// Iterate through the section table, looking for the one that encompasses the linear address.
			for (unsigned i=0; i<pNtHeader->FileHeader.NumberOfSections; i++, pSection++) {
				DWORD dwSectionStart = pSection->VirtualAddress;
				DWORD dwSectionEnd = dwSectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

				// Is the address in this section???
				if ((rva >= dwSectionStart) && (rva <= dwSectionEnd)) {
					// Yes, address is in the section.  
					// Calculate section and offset, 
					// and store in the "dwSection" & "dwOffset" params, which were passed by reference.
					dwSection = i + 1;
					dwOffset = rva - dwSectionStart;

					bResult = TRUE;
					break;
				}
			}
		}
	}

	return bResult;
}

//============================================================
// Walks the stack, and writes the results to the report file 
//============================================================
void MSJExceptionHandler::IntelStackWalk(const HANDLE hFile, PCONTEXT lpContext) {
	TCHAR szBuffer[EXCEPTIONHANDLER_BUFFSIZE];
	int nNumberOfBytes = 0;

	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes, _T("\r\n[Call Stack Infomation]\r\n"));

	DWORD	dwPc		= lpContext->Eip;
	PDWORD	pdwFrame	= (PDWORD)lpContext->Ebp;
	PDWORD	pdwPrevFrame;
	
	for(;;) {
		TCHAR szModule[MAX_PATH];
		DWORD dwSection = 0, dwOffset = 0;

		GetLogicalAddress((PVOID)dwPc, szModule, sizeof(szModule), dwSection, dwOffset);

		nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, EXCEPTIONHANDLER_BUFFSIZE-nNumberOfBytes,
				_T("%08X=%08X,%04X:%08X,%s\r\n"),
				dwPc, (DWORD)pdwFrame, dwSection, dwOffset, szModule);

		dwPc = pdwFrame[1];

		pdwPrevFrame = pdwFrame;

		pdwFrame = (PDWORD)pdwFrame[0]; // precede to next higher frame on stack

		if ((DWORD)pdwFrame & 3)	// Frame pointer must be aligned on a DWORD boundary.
			break;				  // Bail if not so.

		if (pdwFrame <= pdwPrevFrame)
			break;

		// Can two DWORDs be read from the supposed frame address?		  
		if (IsBadWritePtr(pdwFrame, sizeof(PVOID)*2))
			break;
	}

	EWriteFile(hFile, szBuffer);
	OutputDebugString(szBuffer);
}

//============================================================
// Walks the stack, and writes the results to the report file 
//============================================================
void MSJExceptionHandler::ImagehlpStackWalk(const HANDLE hFile, PEXCEPTION_POINTERS lpExceptionInfo) {
	TCHAR szBuffer[IMGAGEHLP_BUFFSIZE];
	int nNumberOfBytes = 0;

	nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T("\r\n[Call Stack Information]\r\n"));

	// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

	// Turn on line loading and deferred loading.
	_SymSetOptions(_SymGetOptions() | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

	STACKFRAME stStackFrame;
	ZeroMemory(&stStackFrame, sizeof(stStackFrame));

	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	PCONTEXT pContext		= lpExceptionInfo->ContextRecord;

	stStackFrame.AddrPC.Offset		= pContext->Eip;
	stStackFrame.AddrPC.Mode		= AddrModeFlat;
	stStackFrame.AddrStack.Offset	= pContext->Esp;
	stStackFrame.AddrStack.Mode		= AddrModeFlat;
	stStackFrame.AddrFrame.Offset	= pContext->Ebp;
	stStackFrame.AddrFrame.Mode		= AddrModeFlat;

#ifdef _WIN64
#define CH_MACHINE IMAGE_FILE_MACHINE_IA64
#else
#define CH_MACHINE IMAGE_FILE_MACHINE_I386
#endif
	for (;;) {
		HANDLE hProcess = GetCurrentProcess();
		if (!_StackWalk(CH_MACHINE, hProcess, GetCurrentThread(), &stStackFrame, pContext, NULL, _SymFunctionTableAccess, _SymGetModuleBase, NULL))
			break;

		if (0 == stStackFrame.AddrFrame.Offset) // Basic sanity check to make sure the frame is OK.
			break;								// Bail if not.

		nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T("%08X=%08X,"), stStackFrame.AddrPC.Offset, stStackFrame.AddrFrame.Offset);

		// IMAGEHLP is wacky, and requires you to pass in a pointer to a IMAGEHLP_SYMBOL structure.
		// The problem is that this structure is variable length.
		// That is, you determine how big the structure is at runtime.
		// This means that you can't use sizeof(struct).
		// So...make a buffer that's big enough, and make a pointer to the buffer.
		// We also need to initialize not one, but TWO members of the structure before it can be used.
		BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512];
		PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLength = 512;
						
		DWORD dwDisplacement;  // Displacement of the input address, relative to the start of the symbol

		if (_SymGetSymFromAddr(hProcess, stStackFrame.AddrPC.Offset, &dwDisplacement, pSymbol)) {
			nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T("%hs+%X"), pSymbol->Name, dwDisplacement);

			IMAGEHLP_LINE stImagehlpLine;
			ZeroMemory(&stImagehlpLine, sizeof(IMAGEHLP_LINE));
			stImagehlpLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);
			if (SymbolGetLineFromAddr(hProcess, stStackFrame.AddrPC.Offset, &dwDisplacement, &stImagehlpLine)) {
				if (dwDisplacement > 0) {
#ifdef _UNICODE
					nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T(",%S,%04d+%04d"), stImagehlpLine.FileName, stImagehlpLine.LineNumber, dwDisplacement);
#else
					nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T(",%s,%04d+%04d"), stImagehlpLine.FileName, stImagehlpLine.LineNumber, dwDisplacement);
#endif
				}
				else {
#ifdef _UNICODE
					nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T(",%S,%04d"), stImagehlpLine.FileName, stImagehlpLine.LineNumber);
#else
					nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T(",%s,%04d"), stImagehlpLine.FileName, stImagehlpLine.LineNumber);
#endif
				}
			}
		}
		else { // No symbol found.  Print out the logical address instead.
			TCHAR szModule[MAX_PATH];
			DWORD dwSection = 0, dwOffset = 0;

			GetLogicalAddress((PVOID)stStackFrame.AddrPC.Offset, szModule, sizeof(szModule), dwSection, dwOffset);

			nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T("%04X:%08X,%s"), dwSection, dwOffset, szModule);

			// If the symbol wasn't found, the source file and line number won't be found either
		}

		nNumberOfBytes += _stprintf_s(szBuffer+nNumberOfBytes, IMGAGEHLP_BUFFSIZE-nNumberOfBytes, _T("\r\n"));

		//! 이상하게 Call Stack이 loop를 돌면서 많아지는 경우가 있다.
		if (nNumberOfBytes >= IMGAGEHLP_BUFFSIZE - 256)
			break;
	}

	EWriteFile(hFile, szBuffer);
	OutputDebugString(szBuffer);
}

//============================================================================
// Helper function that writes to the report file, and allows the user to use 
// printf style formating													 
//============================================================================
int __cdecl MSJExceptionHandler::EWriteFile(HANDLE hFile, LPCTSTR lpszFormat, ...)
{
	TCHAR	szBuff[EXCEPTIONHANDLER_BUFFSIZE];
	int		nNumberOfBytesToWrite ;
	DWORD	dwNumberOfBytesWritten;
	va_list arg_ptr;
		  
	va_start(arg_ptr, lpszFormat);
	nNumberOfBytesToWrite = wvsprintf(szBuff, lpszFormat, arg_ptr);
	va_end(arg_ptr);

	WriteFile(hFile, szBuff, nNumberOfBytesToWrite * sizeof(TCHAR), &dwNumberOfBytesWritten, 0);

	return nNumberOfBytesToWrite;
}

DWORD MSJExceptionHandler::GetFileVersion(TCHAR* fileName)
{
/*
	DWORD dwLen, dwUseless;
	LPTSTR lpVI;
	UINT verMajor = 0;

	dwLen = GetFileVersionInfoSize((LPTSTR)fileName, &dwUseless);
	if (dwLen == 0)
		return 0;

	lpVI = (LPTSTR)_alloca(dwLen);
	//	lpVI = (LPTSTR) GlobalAlloc(GPTR, dwLen);
	if (lpVI)
	{
		DWORD dwBufSize;
		VS_FIXEDFILEINFO* lpFFI;

		GetFileVersionInfo((LPTSTR)fileName, NULL, dwLen, lpVI);

		if (VerQueryValue(lpVI, _T("\\"),
			(LPVOID *) &lpFFI, (UINT *) &dwBufSize))

		{
			//We now have the VS_FIXEDFILEINFO in lpFFI
			//verMajor = HIWORD(lpFFI->dwFileVersionMS);
			verMajor = lpFFI->dwFileVersionLS & 0xffff;
		}
	}
	return verMajor;
*/
	if (fileName == NULL)
		return 0;

	HINSTANCE module = GetModuleHandle(fileName);

	if (module == NULL)
		return 0;

	void *verinfo = NULL;
	HRSRC resrc;
	HGLOBAL verhdl;
	void *v = NULL;
	unsigned int size;

	resrc = FindResource(module, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (resrc == NULL)
		return 0;

	size = SizeofResource(module, resrc);
	verinfo = _alloca(size);

	verhdl = LoadResource(module, resrc);
	if (verhdl == NULL)
		return 0;
	v = LockResource(verhdl);
	if (v == NULL)
		return 0;

	//LockResource()한 것을 그대로 사용하면 VerQueryValue()에서
	//access violation exception이 올라온다.
	memcpy(verinfo, v, size);

	VS_FIXEDFILEINFO *ffi = NULL;

	if (!VerQueryValue(verinfo, TEXT("\\"), (void**)&ffi, &size))
		return 0;

	if (ffi == NULL)
		return 0;

	return ffi->dwFileVersionLS & 0xffff;
}

//=========================================================================
// Load IMAGEHLP.DLL and get the address of functions in it that we'll use 
//=========================================================================
BOOL MSJExceptionHandler::InitializeImagehlpFunctions(void) {
	BOOL bResult = FALSE;

	if (m_hModImagehlp)
	{
		bResult = TRUE;
	}
	else
	{
		//m_hModImagehlp = LoadLibrary(_T("dbgehlp.DLL"));
		m_hModImagehlp = LoadLibrary(_T("IMAGEHLP.DLL"));
		if (m_hModImagehlp)
		{
			if ((_SymInitialize					= (SYMINITIALIZE)GetProcAddress(m_hModImagehlp, "SymInitialize")) != NULL
					&& (_SymCleanup				= (SYMCLEANUP)GetProcAddress(m_hModImagehlp, "SymCleanup")) != NULL
					&& (_StackWalk				= (STACKWALK)GetProcAddress(m_hModImagehlp, "StackWalk")) != NULL
					&& (_SymFunctionTableAccess	= (SYMFUNCTIONTABLEACCESS)GetProcAddress(m_hModImagehlp, "SymFunctionTableAccess")) != NULL
					&& (_SymGetModuleBase		= (SYMGETMODULEBASE)GetProcAddress(m_hModImagehlp, "SymGetModuleBase")) != NULL
					&& (_SymGetSymFromAddr		= (SYMGETSYMFROMADDR)GetProcAddress(m_hModImagehlp, "SymGetSymFromAddr")) != NULL
					&& (_SymGetLineFromAddr		= (SYMGETLINEFROMADDR)GetProcAddress(m_hModImagehlp, "SymGetLineFromAddr")) != NULL
					&& (_SymGetOptions			= (SYMGETOPTIONS)GetProcAddress(m_hModImagehlp, "SymGetOptions")) != NULL
					&& (_SymSetOptions			= (SYMSETOPTIONS)GetProcAddress(m_hModImagehlp, "SymSetOptions")) != NULL)
			{
				bResult = TRUE;
			}
			else
			{
				FreeLibrary(m_hModImagehlp);
				m_hModImagehlp = 0;
			}
		}
	}

	return bResult;
}

#define BACKWARD 10000
//====================================================================================
BOOL MSJExceptionHandler::SymbolGetLineFromAddr(HANDLE hProcess, DWORD dwAddr, PDWORD lpdwDisplacement, PIMAGEHLP_LINE lpImagehlpLine) {
	BOOL bResult = TRUE;

#ifdef EH_WORK_AROUND_SYMGETLINEFROMADDR_BUG
	// The problem is that the symbol engine finds only those source line addresses (after the first lookup) that fall exactly on a zero displacement. 
	// I'll walk backward BACKWARD bytes to find the line and return the proper displacement.
	DWORD dwTempDisplacement = 0 ;
	while (FALSE == _SymGetLineFromAddr(hProcess, dwAddr - dwTempDisplacement, lpdwDisplacement, lpImagehlpLine)) 
	{
		dwTempDisplacement++;
		if (BACKWARD == dwTempDisplacement) 
		{
			bResult = FALSE;
			break;
		}
	}

	if (bResult) 
	{
		// I found the line, and the source line information is correct, 
		// so change the displacement if I had to search backward to find the source line.
		if (dwTempDisplacement) 
		{
			*lpdwDisplacement = dwTempDisplacement;
		}
	}
#else // EH_WORK_AROUND_SYMGETLINEFROMADDR_BUG
	bResult = SymGetLineFromAddr(hProcess, dwAddr, lpdwDisplacement, lpImagehlpLine);
#endif

	return bResult;
}
/*
//====================================================================================
void ELOG(LPCTSTR lpszFormat, ...)
{
//	assert(g_szLogPath);

	CLogCritical cs;

	TCHAR szBuffer[4096];

	va_list args;
	va_start(args, lpszFormat);
	_vsntprintf(szBuffer, 4096 * sizeof(TCHAR), lpszFormat, args);
	va_end(args);

	WRITE_ERROR_REPORT(szBuffer);

//	HANDLE hFile = ERROR_REPORT_PTR->Create();
//	if (INVALID_HANDLE_VALUE != hFile) {
//		DWORD dwWritten;
//#ifdef _UNICODE
//		dwWritten = SetFilePointer(hFile, 0, NULL, FILE_END);
//		if (0 == dwWritten) {
//			TCHAR mark = 0xFEFF;
//			WriteFile(hFile, &mark, sizeof(TCHAR), &dwWritten, NULL);
//		}
//#else
//		SetFilePointer(hFile, 0, NULL, FILE_END);
//#endif
//
//		TCHAR szBuffer[2048];
//		int nNumberOfBytes;
//
//		SYSTEMTIME st = {0, };
//		::GetLocalTime(&st);
//		nNumberOfBytes = _sntprintf(szBuffer, sizeof(szBuffer)/sizeof(TCHAR), _T("[PID:TID(%d:%d) %04d/%02d/%02d %02d:%02d:%02d]: "), GetCurrentProcessId(), GetCurrentThreadId(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
//		WriteFile(hFile, szBuffer, nNumberOfBytes * sizeof(TCHAR), &dwWritten, NULL);
//
//		va_list arg_ptr;
//		va_start(arg_ptr, lpszFormat);
//
//		nNumberOfBytes = _vsntprintf(szBuffer, sizeof(szBuffer)/sizeof(TCHAR)-1, lpszFormat, arg_ptr);
//		szBuffer[nNumberOfBytes] = 0;
//
//		va_end(arg_ptr);
//
//		LPTSTR pszBuffer;
//		for (pszBuffer = szBuffer; *pszBuffer;) {
//			if (*pszBuffer == '\n') {
//				WriteFile(hFile, _T("\r\n"), 2 * sizeof(TCHAR), &dwWritten, NULL);
//				pszBuffer++;
//				continue;
//			}
//			nNumberOfBytes = _tcscspn(pszBuffer, _T("\r\n"));
//			WriteFile(hFile, pszBuffer, nNumberOfBytes * sizeof(TCHAR), &dwWritten, NULL);
//			pszBuffer += nNumberOfBytes;
//		}
//
//		ERROR_REPORT_PTR->Close();
//	}
}
*/

//////////////////////////////////////////////////////////////////////////
// CSystemException class

#ifdef USE_MFC
IMPLEMENT_DYNAMIC(CSystemException, CException)
#endif

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

BOOL CSystemException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT)
{
	_sntprintf_s(lpszError, nMaxError, nMaxError-1, TEXT("%s"),
			MSJExceptionHandler::GetExceptionString(mExceptInfo->ExceptionRecord->ExceptionCode));
	return TRUE;
}

void CSystemException::WriteAdditionalInfo()
{
	MSJExceptionHandler::GenerateExceptionReport(mExceptInfo);
}

//====================================================================================
void trans_func(unsigned int u, PEXCEPTION_POINTERS lpExpPtr)
{
	//MSJExceptionHandler::GenerateExceptionReport(lpExpPtr);
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);	
#endif
	PG_FLUSH_LOG
	throw new CSystemException(lpExpPtr);
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
	exit(-1);
}

void purecall_handler()
{
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
	SetAlertShow(true);
#endif
	PG_TRY_BLOCK
	PG_FLUSH_LOG	
	::RaiseException(1, 0, 0, NULL);
	PG_CATCH_BLOCK
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
	exit(-1);
}

int _new_handler(size_t)
{
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
	SetAlertShow(true);
#endif
	PG_TRY_BLOCK
	PG_FLUSH_LOG
	::RaiseException(1, 0, 0, NULL);
	PG_CATCH_BLOCK
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
	exit(-1);
}

void invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t)
{
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
	SetAlertShow(true);
#endif
	PG_TRY_BLOCK
	_PgOutputDebugString("invliad_parameter_handler: %s, %s, %s, %d", expression ? MB(expression) : "NULL",  function ? MB(function) : "NULL", file ? MB(file) : "NULL", line);
	PG_FLUSH_LOG
	::RaiseException(1, 0, 0, NULL);
	PG_CATCH_BLOCK
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
	exit(-1);
}

void terminate_handler_func()
{
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
	SetAlertShow(true);
#endif
	PG_TRY_BLOCK
	PG_FLUSH_LOG
	::RaiseException(1, 0, 0, NULL);
	PG_CATCH_BLOCK
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
}

void unexpected_handler_func()
{
#ifndef USE_INB
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
	SetAlertShow(true);
#endif
	PG_TRY_BLOCK
	PG_FLUSH_LOG
	::RaiseException(1, 0, 0, NULL);
	PG_CATCH_BLOCK
	MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
}

bool g_bExceptionAlertShow = false;

void SetAlertShow(bool bShow)
{
	g_bExceptionAlertShow = bShow;
}

void ExceptionUserAlert()
{
#ifndef USE_INB
	if (g_bExceptionAlertShow)
	{
		std::string ClientName = GetClientName();
		ClientName += " Client";
		::MessageBoxA(NULL, "VERY IMPORTANT!!! Client crashed! \nContact developer or send exception log", ClientName.c_str(), MB_OK | MB_ICONWARNING);
	}
#endif
}

void SetThreadName(DWORD dwThreadID, char* threadName)
{
   Sleep(10);
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
	   ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}

#endif // EH_USE_NO_EXCEPTION
