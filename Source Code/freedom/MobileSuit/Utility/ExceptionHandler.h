#ifndef __EXCEPTION_HANDLER_H_
#define __EXCEPTION_HANDLER_H_

//#define EH_USE_NO_EXCEPTION
#define EH_USE_SET_EXCEPTION
#define EH_WORK_AROUND_SYMGETLINEFROMADDR_BUG

#include <EH.h>
#include <new.h>
#include <dbghelp.h>

#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma comment(lib, "Rpcrt4.lib")
//#pragma warning(disable:4786)

#ifdef DM_USE_DEBUG_MEMORY
#include "DebugMemory.h"
#endif

class CSystemException
#ifdef USE_MFC
	: public CException
	#ifdef DM_USE_DEBUG_MEMORY
		, public CMemory::CNoTrackObject
	#endif
#elif defined(DM_USE_DEBUG_MEMORY)
	: public CMemory::CNoTrackObject
#endif
{
#ifdef USE_MFC
	DECLARE_DYNAMIC(CSystemException)
#endif

// Constructors
public:
	CSystemException(PEXCEPTION_POINTERS info):mExceptInfo(info) {}

// Attributes
//protected:
	PEXCEPTION_POINTERS mExceptInfo;

// Operations

// Implementation
public:
#ifdef USE_MFC
	virtual			~CSystemException() {}
	virtual BOOL GetErrorMessage(LPTSTR, UINT, PUINT pnHelpContext=NULL);
	virtual void WriteAdditionalInfo();
#else
	BOOL GetErrorMessage(LPTSTR, UINT, PUINT pnHelpContext=NULL);
	void WriteAdditionalInfo();
#endif
};

/*
#ifdef EH_USE_NO_EXCEPTION
	#define BEFORE			{
	#define _BEFORE			{
	#define AFTER			} if (0) {
	#define _AFTER			} if (0) {
	#define FIN				}
	#define _FIN			}
	#define AFTER_FIN		}	
	#define _AFTER_FIN		}
	#define CHECK_LOG
	#define ELOG			(1) ? 0 : _ELOG
	#define EASSERT(expr)
	#define ETHROW
	inline void _ELOG(LPCTSTR, ...) {}
#else // EH_USE_NO_EXCEPTION
	#define BEFORE			try {

#ifdef USE_MFC
	#define AFTER			} catch (CException* e) {																			\
								TCHAR szCause[255];																				\
								e->GetErrorMessage(szCause, sizeof(szCause));													\
								e->Delete();																					\
								ELOG(_T("%s,,,%s,%d,"), szCause, _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__);

#else // USE_MFC
	#define AFTER			} catch (CSystemException* e) {																		\
								TCHAR szCause[255];																				\
								e->GetErrorMessage(szCause, sizeof(szCause));													\
								SAFE_DELETE(e);																						\
								ELOG(_T("%s,,,%s,%d,"), szCause, _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__);

#endif // USE_MFC

	#define FIN				ETHROW; }
	#define AFTER_FIN		AFTER FIN
	#define CHECK_LOG		ELOG(_T("0,Check,%s,%d,"), _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__);
	#define _BEFORE			_se_translator_function tr_func_org = _set_se_translator(trans_func);								\
							try {

#ifdef USE_MFC
	#define _AFTER			} catch (CException* e) {																			\
								TCHAR szCause[255];																				\
								e->GetErrorMessage(szCause, sizeof(szCause));													\
								e->Delete();																					\
								ELOG(_T("%s,,,%s,%d,"), szCause, _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__);
#else // USE_MFC
	#define _AFTER			} catch (CSystemException* e) {																		\
								TCHAR szCause[255];																				\
								e->GetErrorMessage(szCause, sizeof(szCause));													\
								SAFE_DELETE(e);																						\
								ELOG(_T("%s,,,%s,%d,"), szCause, _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__);

#endif // USE_MFC

	#define _FIN 			} catch (...) {																						\
								ELOG(_T("0,Unhandled Exception,,,%s,%d,"), _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__); \
							}																									\
							_set_se_translator(tr_func_org);
	#define _AFTER_FIN		_AFTER _FIN

	#define EASSERT(expr)	if (!(expr)) ELOG(_T("0,Assertion Failed,,,%s,%d,"), _T(__FILE__) _T("[") _T(__TIMESTAMP__) _T("]"), __LINE__)
	#define ETHROW			throw new CSystemException(0)

	void 		ELOG(LPCTSTR, ...);
	void 		trans_func(unsigned int, PEXCEPTION_POINTERS);
#endif // EH_USE_NO_EXCEPTION
*/

//====================================================================================
class MSJExceptionHandler {
	friend void trans_func(unsigned int, PEXCEPTION_POINTERS);
	friend void purecall_handler(void);
	friend int _new_handler(size_t);
	friend void invalid_parameter_handler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
	friend void terminate_handler_func();
	friend void unexpected_handler_func();
	friend class CSystemException;
public:
	MSJExceptionHandler();
	~MSJExceptionHandler();

	static void InstallExceptionHandler(bool bSetException);
	static void UninstallExceptionHandler();
	
private:
	// entry point where control comes on an unhandled exception
	static LONG WINAPI		MSJUnhandledExceptionFilter(PEXCEPTION_POINTERS);

	// where report info is extracted and generated	
	static void				GenerateExceptionReport(PEXCEPTION_POINTERS);
	static void				WriteFaultReason(const HANDLE, PEXCEPTION_POINTERS);
	static void				WriteRegisters(const HANDLE, PEXCEPTION_POINTERS);

	// Helper functions
	static LPTSTR			GetExceptionString(DWORD);
	static BOOL				GetLogicalAddress(PVOID, PTSTR, DWORD, DWORD&, DWORD&);
	static void				IntelStackWalk(const HANDLE, PCONTEXT);
	static void				ImagehlpStackWalk(const HANDLE, PEXCEPTION_POINTERS);
	static int __cdecl		EWriteFile(HANDLE, LPCTSTR, ...);
	static BOOL				InitializeImagehlpFunctions(void);
	static BOOL				SymbolGetLineFromAddr(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
	static DWORD			GetFileVersion(TCHAR* fileName);
		
	// Variables used by the class
	static LPTOP_LEVEL_EXCEPTION_FILTER m_pPreviousFilter;
	static _se_translator_function mPrevTranslatorFunc;	
	static _purecall_handler mPrevPureHandler;	
	static _PNH mPrevNewHandler;
	static _invalid_parameter_handler mPrevInvalidParameterHandler;	
	static terminate_handler mPrevTerminateHandler;
	static unexpected_function mPrevUnexpectedHandler;

	// Make typedefs for some IMAGEHLP.DLL functions so that we can use them GetProcAddress
	typedef BOOL			(__stdcall *SYMINITIALIZE)			(HANDLE, LPSTR, BOOL);
	typedef BOOL			(__stdcall *SYMCLEANUP)				(HANDLE);
	typedef BOOL			(__stdcall *STACKWALK)				(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
	typedef LPVOID			(__stdcall *SYMFUNCTIONTABLEACCESS)	(HANDLE, DWORD);
	typedef DWORD			(__stdcall *SYMGETMODULEBASE)		(HANDLE, DWORD);
	typedef BOOL			(__stdcall *SYMGETSYMFROMADDR)		(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
	typedef BOOL			(__stdcall *SYMGETLINEFROMADDR)		(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
	typedef DWORD			(__stdcall *SYMGETOPTIONS)			(void);
	typedef DWORD			(__stdcall *SYMSETOPTIONS)			(DWORD);

	static HMODULE					m_hModImagehlp;
	static SYMINITIALIZE			_SymInitialize;
	static SYMCLEANUP				_SymCleanup;
	static STACKWALK				_StackWalk;
	static SYMFUNCTIONTABLEACCESS	_SymFunctionTableAccess;
	static SYMGETMODULEBASE			_SymGetModuleBase;
	static SYMGETSYMFROMADDR		_SymGetSymFromAddr;
	static SYMGETLINEFROMADDR		_SymGetLineFromAddr;
	static SYMGETOPTIONS			_SymGetOptions;
	static SYMSETOPTIONS			_SymSetOptions;
};

#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetAlertShow(bool bShow);
void ExceptionUserAlert();
void SetThreadName(DWORD dwThreadID, char* threadName);
#endif // __EXCEPTION_HANDLER_H_

