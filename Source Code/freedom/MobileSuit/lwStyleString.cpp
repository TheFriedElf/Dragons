#include "stdafx.h"
#include "lwStyleString.H"
#include "lwWString.H"

using namespace lua_tinker;


lwStyleString::lwStyleString(lwStyleString const &kStyleString)
{
	m_kStyleString = kStyleString.GetStyleString();
}

lwStyleString::lwStyleString(XUI::CXUI_Style_String const &kStyleString)
{
	m_kStyleString = kStyleString;
}

bool lwStyleString::RegisterWrapper(lua_State *pkState)
{
	class_<lwStyleString>(pkState, "StyleString")
		.def(pkState, constructor<>())
		.def(pkState, "GetNormalText", &lwStyleString::GetNormalText)
		.def(pkState, "IsNil", &lwStyleString::IsNil)
		;

	return	true;
}

bool	lwStyleString::IsNil()
{
	return	false;
}
lwWString	lwStyleString::GetNormalText()
{
	return	lwWString(m_kStyleString.GetNormalString());
}

