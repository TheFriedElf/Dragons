#include "stdafx.h"
#include "PgMessageUtil.h"

namespace PgMessageUtil
{
	size_t GetFromMessageDotIni(TCHAR const* szApp, TCHAR const* szKey, std::wstring& rkOut, TCHAR const* szDefaultMsg)
	{
		return GetFromIni(L"./message.ini", szApp, szKey, rkOut, szDefaultMsg);
	}

	size_t GetFromIni(TCHAR const* szIniFile, TCHAR const* szApp, TCHAR const* szKey, std::wstring& rkOut, TCHAR const* szDefaultMsg)
	{
		int const iMaxCharSize = 1024;
		TCHAR szErrorMsg[iMaxCharSize] = {0,};
		::GetPrivateProfileString(szApp, szKey, szDefaultMsg, szErrorMsg, iMaxCharSize, szIniFile);
		rkOut = szErrorMsg;
		return rkOut.size();
	}
};