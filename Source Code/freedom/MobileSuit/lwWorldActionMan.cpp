#include "stdafx.h"
#include "lwWorldActionMan.h"
lwWorldActionMan::lwWorldActionMan(PgWorldActionMan *pkWorldActionMan)
{
	m_pWorldActionMan = pkWorldActionMan;
}
bool	lwWorldActionMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwWorldActionMan>(pkState, "WorldActionMan")
		.def(pkState, constructor<PgWorldActionMan *>())
		.def(pkState, "CreateNewWorldActionObject",&lwWorldActionMan::CreateNewWorldActionObject)
		.def(pkState, "IsNil",&lwWorldActionMan::IsNil)
		;
	return true;
}
lwWorldActionObject	lwWorldActionMan::CreateNewWorldActionObject(int iWorldActionID)
{
	PgWorldActionObject	*pObj = m_pWorldActionMan->CreateNewWorldActionObject(iWorldActionID);
	assert(pObj);
	return	lwWorldActionObject(pObj);
}
bool	lwWorldActionMan::IsNil()
{
	if(m_pWorldActionMan==0) return true;
	return false;
}
