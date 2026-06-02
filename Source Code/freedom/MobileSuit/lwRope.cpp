#include "stdafx.h"
#include "lwRope.H"
#include "PgRope.H"
#include "lwPoint3.H"

using namespace lua_tinker;

void lwRope::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(Rope)
		LW_REG_METHOD(Rope, GetStartPos)
		LW_REG_METHOD(Rope, GetEndPos)
		;
}

lwPoint3	lwRope::GetStartPos()
{
	return	lwPoint3(m_pkRope->GetStartPos());
}
lwPoint3	lwRope::GetEndPos()
{
	return	lwPoint3(m_pkRope->GetEndPos());
}
