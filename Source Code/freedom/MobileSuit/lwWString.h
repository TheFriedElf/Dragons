#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWSTRING_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWSTRING_H

class lwWString
{
public:
	lwWString(char const *pcWString);
	lwWString(const wchar_t *pcWString, size_t const szLen = 0);
	lwWString( lwWString &rkWString);
	lwWString(std::wstring const &wstr);

	// 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

public:
	//! WString를 변경한다.
	bool Set(char const *pcWString);

	// string을 추가한다.
	void Add(char const *pcText);

	//! 진짜 WString를 반환한다.
	std::wstring &operator()();

	static	lwWString	GetAppendTextW(lwWString Text1,lwWString Text2);	//	Text1 과 Text2 를 연결한 새로운 텍스트를 리턴한다.

	//! 같은 WString인지 비교한다.
	bool IsEqual(lwWString kWString);

	void Generate();

	int Length();

	char const*	GetStr();

	std::wstring& GetWString()	{	return	m_kWString;	}

	bool IsNil();			//== ""

	void EraseCRLF();		//Erase \r\n

	bool FilterString(int const iMode, bool const bConvert);//1 == Badword, 2 == BlockNickName, 3 == All

	template< typename T >
	void Replace(char const* pkFindStr, T const& kData);
	void ReplaceInt(char const* pkFindStr, int const kData);
	void ReplaceStr(char const* pkFindStr, char const* kData);

protected:
	std::wstring m_kWString;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWSTRING_H