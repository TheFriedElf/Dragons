#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSTYLESTRING_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSTYLESTRING_H
#include "PgScripting.h"
#include "xui/xui_font.H"

class	lwWString;


class lwStyleString
{
public:
	lwStyleString()
	{
	};
	lwStyleString( lwStyleString const &kStyleString);
	lwStyleString(XUI::CXUI_Style_String const &kStyleString);

	// 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

public:

	lwWString	GetNormalText();
	bool IsNil();			//== ""

	XUI::CXUI_Style_String	const &operator()()	const	{	return	m_kStyleString;	}
	XUI::CXUI_Style_String	const	&GetStyleString()	const	{	return	m_kStyleString;	}

protected:
	XUI::CXUI_Style_String m_kStyleString;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSTYLESTRING_H