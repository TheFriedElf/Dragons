#ifndef FREEDOM_DRAGONICA_UTIL_ERRORREPORTFILE_H
#define FREEDOM_DRAGONICA_UTIL_ERRORREPORTFILE_H

#define ERROR_REPORT_PTR ErrorReportFile::GetPtr()
#define DELETE_ERROR_REPORT ErrorReportFile::DeletePtr
#define SETDIR_ERROR_REPORT ErrorReportFile::GetPtr()->SetPath
#define FORMAT_ERROR_HEADER ErrorReportFile::GetPtr()->FormatHeader
#define FORMAT_ERROR_TAILER ErrorReportFile::GetPtr()->FormatTailer
#define WRITE_ERROR_INFO ErrorReportFile::GetPtr()->WriteInfo
#define WRITE_ERROR_REPORT ErrorReportFile::GetPtr()->WriteReport
#define FORMAT_ERROR_REPORT ErrorReportFile::GetPtr()->FormatReport
#define UPLOAD_ERROR_REPORT ErrorReportFile::GetPtr()->UploadReports
#define WAIT_ERROR_REPORT ErrorReportFile::GetPtr()->WaitForUploadComplete


class ErrorReportFile
{
// Constructors & Destructors
protected:
	ErrorReportFile();
public:
	~ErrorReportFile();

// Attributes
protected:
	static ErrorReportFile *mSingletonPtr;
	TCHAR *mErrorReportPath;
	TCHAR mReportFileName[1024];
	TCHAR mReportHeader[1024], mReportTailer[1024];
	HANDLE mFileHandle, mUploadHandle;
	TCHAR mAspPath[MAX_PATH];
	TCHAR mServerIp[MAX_PATH];

// Accessors & Queries
public:
	inline static ErrorReportFile *GetPtr();
	inline static void DeletePtr();
	void SetPath(const TCHAR *path);
	void AddHeader(const TCHAR *fmt, ...);
	void AddTailer(const TCHAR *fmt, ...);
	void FormatHeader(int appVer, int appCode);
	void FormatTailer();
	inline TCHAR* GetAspPath(){ return mAspPath; }
	inline TCHAR* GetServerIp(){ return mServerIp; }
protected:
	BOOL IsFileExist(LPCTSTR lpszFile);
	BOOL CreateLogPath(LPCTSTR lpszLogPath);

// Operations
public:	
	HANDLE Create();
	void Close();
	void SetFileName(TCHAR* pFileName);
	void WriteInfo(const TCHAR *fmt, ...);
	void WriteReport(const TCHAR *fmt, ...);
	void FormatReport(int errCode, const TCHAR *errText, const TCHAR *fileName, int lineNum);
//	void UploadReports(const TCHAR *serverIp, const TCHAR *aspName);
//	void WaitForUploadComplete();
//	static DWORD WINAPI UploadReportProc(LPVOID lpParameter);
};

inline ErrorReportFile *ErrorReportFile::GetPtr()
{
	if (mSingletonPtr == NULL)
		mSingletonPtr = new ErrorReportFile;
	return mSingletonPtr;
}

inline void ErrorReportFile::DeletePtr()
{
	SAFE_DELETE(mSingletonPtr);

}
#endif // FREEDOM_DRAGONICA_UTIL_ERRORREPORTFILE_H