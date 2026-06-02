
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>

//#include "TypeDefinition.h"
//#include "HttpFileUploader.h"
#include "ErrorReportfile.h"
#include <io.h>


//////////////////////////////////////////////////////////////////////////
// ErrorReportFile class

ErrorReportFile *ErrorReportFile::mSingletonPtr = NULL;

//////////////////////////////////////////////////////////////////////////
// Contruction/Destruction
//////////////////////////////////////////////////////////////////////////

ErrorReportFile::ErrorReportFile(void)
{
	mErrorReportPath = NULL;
	mFileHandle = NULL;
	mUploadHandle = NULL;
	memset(mReportFileName, 0, sizeof(mReportHeader));
	memset(mReportHeader, 0, sizeof(mReportHeader));
	memset(mReportTailer, 0, sizeof(mReportTailer));
	mAspPath[0] = '\0';
	mServerIp[0] = '\0';
}

ErrorReportFile::~ErrorReportFile(void)
{
//	WaitForUploadComplete();
	SAFE_DELETE(mErrorReportPath)
	
	if (mFileHandle !=  NULL)
	{
		CloseHandle(mFileHandle);
	}
}

//////////////////////////////////////////////////////////////////////////
// Accesses/Queries
//////////////////////////////////////////////////////////////////////////

void ErrorReportFile::SetPath(const TCHAR *path)
{
	SAFE_DELETE(mErrorReportPath);

	unsigned int iSize = _tcslen(path) + 1;
	mErrorReportPath = new TCHAR[iSize];
	_tcscpy_s(mErrorReportPath, iSize, path);

	int nLength = (int)_tcslen(mErrorReportPath);
	if (mErrorReportPath[nLength - 1] ==  '\\')
		mErrorReportPath[nLength - 1] = 0;

	OutputDebugString(TEXT("ErrorReportFile::SetPath - "));
	OutputDebugString(mErrorReportPath);
	OutputDebugString(TEXT("\n"));
	if (!IsFileExist(mErrorReportPath))
		CreateLogPath(mErrorReportPath);

	// set program database directory
	//{
	//	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	//	TCHAR filename[_MAX_PATH], path[_MAX_PATH];

	//	GetModuleFileName(NULL, filename, _MAX_PATH);
	//	_tsplitpath(filename, drive, dir, fname, ext);
	//	_stprintf(path, _T("%s%s"), drive, dir);

	//	SetCurrentDirectory(path);
	//}
}

BOOL ErrorReportFile::IsFileExist(LPCTSTR lpszFile)
{
	BOOL bResult;

#ifdef _UNICODE
	struct _wfinddata_t structFile;
#else
	struct _finddata_t structFile;
#endif

	long hFile = (long)_tfindfirst((LPTSTR)lpszFile, &structFile);
	bResult = (-1 ==  hFile) ? FALSE : TRUE;
	_findclose(hFile);

	return bResult;
}

BOOL ErrorReportFile::CreateLogPath(LPCTSTR lpszLogPath)
{
	BOOL bResult = TRUE;

	_TCHAR szDelimiter[] = _T("\\");

	_TCHAR szSrcLogPath[MAX_PATH];
	_tcscpy_s(szSrcLogPath, MAX_PATH, lpszLogPath);

	_TCHAR szDstLogPath[MAX_PATH] = {0, };

	_TCHAR* pNextToken = NULL;
	_TCHAR* pToken = _tcstok_s(szSrcLogPath, szDelimiter, &pNextToken);
	while (pToken) {
		_tcscat_s(szDstLogPath, MAX_PATH, pToken);
		if (2 ==  _tcslen(szDstLogPath)) {
			// just only drive?
		}
		else {
			// folder?
/*
			if (!IsFileExist(szDstLogPath)) {
				if (!CreateDirectory(szDstLogPath, NULL)) {
					OutputDebugString(TEXT("CreateLogPath Failed - "));
					OutputDebugString(szDstLogPath);
					OutputDebugString(TEXT("\n"));
					bResult = FALSE;
					break;
				}
			}
*/
			if (!NiFile::DirectoryExists(MB(szDstLogPath)) && 
				!NiFile::CreateDirectoryRecursive(MB(szDstLogPath)))
			{
				OutputDebugString(TEXT("CreateLogPath Failed - "));
				OutputDebugString(szDstLogPath);
				OutputDebugString(TEXT("\n"));
				bResult = FALSE;
				break;
			}
		}
		_tcscat_s(szDstLogPath, MAX_PATH, _T("\\"));

		pToken = _tcstok_s(NULL, szDelimiter, &pNextToken);
	}

	return bResult;
}

void ErrorReportFile::AddHeader(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int s = sizeof(mReportHeader), l = (int)_tcslen(mReportHeader), n = 0;
	if (l < s - 1)
		n = _vsntprintf_s(mReportHeader + l, s - l, s - l, fmt, args);
	
	if (l + n >=  s)
		mReportHeader[s - 1] = '\0';
	va_end(args);
}

void ErrorReportFile::AddTailer(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int s = sizeof(mReportTailer), l = (int)_tcslen(mReportTailer), n = 0;
	if (l < s - 1)
		n = _vsntprintf_s(mReportTailer + l, s - l, s - l, fmt, args);
	if (l + n >=  s)
		mReportTailer[s - 1] = '\0';
	va_end(args);
}

void ErrorReportFile::FormatHeader(int appVer, int appCode)
{
	AddHeader(TEXT("[General Information]\r\n"));
	AddHeader(TEXT("GameVer=%d\r\nServiceCode=0x%04X\r\n"), appVer, appCode);
}

void ErrorReportFile::FormatTailer()
{
	//AddTailer("ID=%s\r\n" "UID=%d\r\n" "IP=%s\r\n" "OS=%d\r\n", id, uid, ip, osVer);
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

HANDLE ErrorReportFile::Create()
{
	if (mReportFileName[0] ==  _T('\0'))
	{
		UUID uuid;
		UuidCreate(&uuid);
		unsigned short* uuidStr;
		if (UuidToString(&uuid, &uuidStr) ==  RPC_S_OUT_OF_MEMORY)
			return NULL;
		unsigned int iSize = _tcslen((const TCHAR *)uuidStr) + 1;
		_tcscpy_s(mReportFileName, 1024, (const TCHAR *)uuidStr);
		RpcStringFree(&uuidStr);
	}

	if (mFileHandle !=  NULL)
		return mFileHandle;

	TCHAR fileName[MAX_PATH] = {0, };
	//SYSTEMTIME st = {0, };
	//::GetLocalTime(&st);
	//_stprintf(fileName, _T("%s\\%04d_%02d_%02d.log"), g_szLogPath, st.wYear, st.wMonth, st.wDay);
	_stprintf_s(fileName, MAX_PATH, _T("%s\\%s.log"), mErrorReportPath, mReportFileName);
	mFileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0);

	SetFilePointer(mFileHandle, 0, NULL, FILE_END);

	DWORD dwWrite;
	wchar_t szBuf[6];
	wchar_t wc = 0xFEFF;
	swprintf_s(szBuf, 6, L"%c", wc);
	WriteFile(mFileHandle, szBuf, 2, &dwWrite, NULL);
	
	return mFileHandle;
}

void ErrorReportFile::Close()
{
	CloseHandle(mFileHandle);
	mFileHandle = NULL;
}

void ErrorReportFile::SetFileName(TCHAR* pFileName)
{
	if (pFileName == NULL)
		return;

	_tcscpy_s(mReportFileName, 1024, pFileName);
}

void ErrorReportFile::WriteInfo(const TCHAR *fmt, ...)
{
	Create();

	if (mFileHandle ==  INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(mFileHandle, 0, NULL, FILE_END);

	va_list args;
	va_start(args, fmt);
	int nNumberOfChars = _vsctprintf(fmt, args);
	int iSize = nNumberOfChars + 1;
	TCHAR *szBuffer = new TCHAR[iSize];
	_vstprintf_s(szBuffer, iSize, fmt, args);
	va_end(args);

	DWORD dwWritten;
	WriteFile(mFileHandle, szBuffer, nNumberOfChars, &dwWritten, NULL);

	SAFE_DELETE(szBuffer);

	Close();
}

void ErrorReportFile::WriteReport(const TCHAR *fmt, ...)
{
	Create();

	if (mFileHandle ==  INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(mFileHandle, 0, NULL, FILE_END);

	va_list args;
	va_start(args, fmt);
	int nNumberOfChars = _vsctprintf(fmt, args);
	unsigned int iSize = nNumberOfChars + 1;
	TCHAR *szBuffer = new TCHAR[iSize];
	_vstprintf_s(szBuffer, iSize, fmt, args);
	va_end(args);

	DWORD dwWritten;
	WriteFile(mFileHandle, mReportHeader, (DWORD)_tcslen(mReportHeader), &dwWritten, NULL);
	WriteFile(mFileHandle, mReportTailer, (DWORD)_tcslen(mReportTailer), &dwWritten, NULL);
	WriteFile(mFileHandle, szBuffer, nNumberOfChars, &dwWritten, NULL);

	SAFE_DELETE(szBuffer);

	Close();
}

void ErrorReportFile::FormatReport(int errCode, const TCHAR *errText, const TCHAR *fileName, int lineNum)
{
	WriteReport(TEXT("\r\n[Error Infomation] \r\n ErrorCode=%08X\r\n ErrorType=%s \r\n FileName=%s \r\n LineNumber=%d \r\n\r\n"), errCode, errText, fileName, lineNum);
}
/*
void ErrorReportFile::UploadReports(const TCHAR *serverIp, const TCHAR* aspPath)
{
	if (mUploadHandle !=  NULL)
		return;

	if (aspPath)
		lstrcpy(mAspPath, aspPath);

	if (serverIp)
		lstrcpy(mServerIp, serverIp);

	DWORD threadId;
	mUploadHandle = CreateThread(NULL, 0, UploadReportProc, this, 0, &threadId);
}

void ErrorReportFile::WaitForUploadComplete()
{
	if (mUploadHandle ==  NULL)
		return;

	WaitForSingleObject(mUploadHandle, 30000);
	CloseHandle(mUploadHandle);
	mUploadHandle = NULL;
}

DWORD ErrorReportFile::UploadReportProc(LPVOID lpParameter)
{
	ErrorReportFile *errorReport = (ErrorReportFile *)lpParameter;

	if (errorReport->mErrorReportPath ==  NULL
			|| strlen(errorReport->mErrorReportPath) < 1)
		return -1;

	try
	{
		TCHAR fileName[MAX_PATH];
		WIN32_FIND_DATA findData;
		_stprintf(fileName, _T("%s\\*.log"), ERROR_REPORT_PTR->mErrorReportPath);
		HANDLE findHandle = FindFirstFile(fileName, &findData);
		if (findHandle ==  INVALID_HANDLE_VALUE)
			return -1;

		do
		{
			_stprintf(fileName, _T("%s\\%s"), ERROR_REPORT_PTR->mErrorReportPath, findData.cFileName);
			HTTP_UPLOADER_PTR->Init(errorReport->GetServerIp(),errorReport->GetAspPath());
			if (HTTP_UPLOADER_PTR->Upload(fileName, findData.cFileName))
			{
				while (!HTTP_UPLOADER_PTR->IsComplete())
					Sleep(0);
			}
			HTTP_UPLOADER_PTR->Clear();
			DeleteFile(fileName);
		} while (FindNextFile(findHandle, &findData));
		FindClose(findHandle);

		DELETE_FILE_UPLOADER();
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

*/