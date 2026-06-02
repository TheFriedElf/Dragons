#include "stdafx.h"
#include "lwRidingObject.H"

using namespace lua_tinker;

void lwRidingObject::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(RidingObject)
		LW_REG_METHOD(RidingObject, GetAttactedObjectPos)
		LW_REG_METHOD(RidingObject, SetAttachedObject)
		;
}

lwPoint3	lwRidingObject::GetAttactedObjectPos()
{
	return	lwPoint3(m_pkRidingObject->GetAttactedObjectPos());
}
void	lwRidingObject::SetAttachedObject(char const *objname)
{
	m_pkRidingObject->SetAttachedObject(objname);
}

