#include "StdAfx.h"
#include "lwDateTime.h"

using namespace lua_tinker;

lwDateTime::lwDateTime(void)
{
}

lwDateTime::lwDateTime(BM::DBTIMESTAMP_EX & kDateTime)
{
	m_kDateTime = kDateTime;
}

bool lwDateTime::RegisterWrapper(lua_State *pkState)
{
	// 등록한다.
	class_<lwDateTime>(pkState, "DATETIME")
		.def(pkState, constructor<void>())
		;

	return true;
}
