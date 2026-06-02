#include "StdAfx.h"
#include "PgError.h"
#include "PgMobileSuit.h"

int g_iWinMessageLogTab = 0;

void PgErrorFunc(char const *pcCaption, char const *pcMsg, ...)
{
#ifndef USE_INB
	va_list vl;
	va_start(vl, pcMsg);
	static char s_acBuf[512];

	vsnprintf_s(s_acBuf, sizeof(s_acBuf), _TRUNCATE, pcMsg, vl);
	NILOG(PGLOG_ERROR, "[PgError] %s - %s\n", pcCaption, s_acBuf);
	#ifndef EXTERNAL_RELEASE
		bool bShowMessageBox = (::GetPrivateProfileInt(TEXT("DEBUG"), TEXT("ShowMessageBox"), 1, g_pkApp->GetConfigFileName()) == 1);
		if (bShowMessageBox)
		{
 			NiMessageBox(s_acBuf, pcCaption);
		}
	#endif
#endif
}
Loki::Mutex g_kODSLock;
void PgOutputDebugString(char const *pcMsg, ...)
{
	g_kODSLock.Lock();
#ifndef USE_INB
	va_list vl;
	va_start(vl, pcMsg); 
	static char s_acBuf[1024];

	vsnprintf_s(s_acBuf, sizeof(s_acBuf), _TRUNCATE, pcMsg, vl);
	NILOG(PGLOG_LOG, "[PgOutputDebugString] %s", s_acBuf);
#ifndef EXTERNAL_RELEASE
	_PgOutputDebugStringLevel(s_acBuf);
#endif
#endif

	g_kODSLock.Unlock();
}

void PgOutputDebugStringLevel(int const iLevel, char const *pcMsg, ...)
{
#ifndef USE_INB
#ifndef EXTERNAL_RELEASE
	if (g_iOutPutDebugLevel != iLevel)
		return;
#endif
	
	va_list vl;
	va_start(vl, pcMsg); 
	static char s_acBuf[1024];

	vsnprintf_s(s_acBuf, sizeof(s_acBuf), _TRUNCATE, pcMsg, vl);
	NILOG(PGLOG_LOG, "[PgOutputDebugString] %s", s_acBuf);
#ifndef EXTERNAL_RELEASE
	OutputDebugStringA(s_acBuf);
#endif
#endif
}

void PgMessageBox(char const *pcCaption, char const *pcMsg, ...)	//	Debug,Release 모드에서만 동작한다
{
#ifndef USE_INB
#ifndef EXTERNAL_RELEASE

	va_list vl;
	va_start(vl, pcMsg); 
	static char s_acBuf[1024];

	vsnprintf_s(s_acBuf, sizeof(s_acBuf), _TRUNCATE, pcMsg, vl);

	NILOG(PGLOG_LOG, "[_PgMessageBox] [%s][%s]", pcCaption,s_acBuf);
	::MessageBox(NULL, UNI(s_acBuf),UNI(pcCaption), MB_OK);
#endif
#endif
}

void PgXmlErrorFunc(char const *pcCaption, const TiXmlNode *pkXml, char const *pcMsg, ...)
{
#ifndef USE_INB
	va_list vl;
	va_start(vl, pcMsg);
	static char s_acMsg[512];
	static char s_acInfo[512];
	TiXmlDocument *pkDoc = const_cast<TiXmlDocument *>(pkXml->GetDocument());
	char const *pcXmlPath = (char const *)(pkDoc->GetUserData());	// PgXmlLoader에서 에러 처리를 위해 경로를 설정해줌
	
	vsnprintf_s(s_acMsg, sizeof(s_acMsg), _TRUNCATE, pcMsg, vl);
	sprintf_s(s_acInfo, sizeof(s_acInfo), "[%s]  Line[%d] row[%d] :\n[%s]", pcXmlPath, pkXml->Row(), pkXml->Column(), s_acMsg);
	
	NILOG(PGLOG_ERROR, "[PgXmlError] %s - %s\n", pcCaption, s_acInfo);
#ifndef EXTERNAL_RELEASE
	bool bShowMessageBox = (::GetPrivateProfileInt(TEXT("DEBUG"), TEXT("ShowMessageBox"), 1, g_pkApp->GetConfigFileName()) == 1);
	if (bShowMessageBox)
		NiMessageBox(s_acInfo, pcCaption);
#endif
#endif
}

#ifndef EXTERNAL_RELEASE
void PgOutputDebugStringLevel(LPCSTR lpOutputString,int const iLevel)
{	
	if ( g_iOutPutDebugLevel == iLevel )
	{
		OutputDebugStringA(lpOutputString);
	}
}

void WriteToConsole(char const *pcMsg, ...)
{
	static int s_iLineNo = 0;
	static char s_acBuf[256] = {0,};

	if(!g_iUseDebugConsole)
	{
		return;
	}

	va_list vargs;
	va_start( vargs, pcMsg );
	::vsprintf_s(s_acBuf,sizeof(s_acBuf),pcMsg,vargs);
	va_end(vargs);

	NiString kStr;
	kStr.Format("%d : %s", ++s_iLineNo, s_acBuf);
    DWORD dwWrite;
    static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hOut != INVALID_HANDLE_VALUE)
	{
		WriteConsoleA(hOut, kStr, kStr.Length(), &dwWrite, 0);
		NILOG(PGLOG_CONSOLE, "%s", s_acBuf);
	}
}
#endif

void PgOutputDebugStringFromGameBryo(char const* pcOut)
{
#ifndef USE_INB
	if (pcOut)
	{
		NILOG(PGLOG_CONSOLE, "[GameBryo] %s", pcOut);
	}
#endif	
}

void CALLBACK PgOutputDebugStringFromXUI(const wchar_t* pcOut)
{
#ifndef EXTERNAL_RELEASE
	if (pcOut)
	{
		NILOG(PGLOG_WINMESSAGE, "%d [XUI] %s", g_iWinMessageLogTab, MB(std::wstring(pcOut)));
	}
#endif
}
