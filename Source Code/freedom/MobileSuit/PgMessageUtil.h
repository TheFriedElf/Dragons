#ifndef FREEDOM_DRAGONICA_UTIL_PGMESSAGEUTIL_H
#define FREEDOM_DRAGONICA_UTIL_PGMESSAGEUTIL_H

namespace PgMessageUtil
{
	size_t GetFromMessageDotIni(TCHAR const* szApp, TCHAR const* szKey, std::wstring& rkOut, TCHAR const* szDefaultMsg);
	size_t GetFromIni(TCHAR const* szIniFile, TCHAR const* szApp, TCHAR const* szKey, std::wstring& rkOut, TCHAR const* szDefaultMsg = L"Unknown Message");
};
#endif // FREEDOM_DRAGONICA_UTIL_PGMESSAGEUTIL_H