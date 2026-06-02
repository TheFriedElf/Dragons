#include "stdafx.h"
#include "lwObject.H"

using namespace lua_tinker;

void lwObject::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(Object)
		LW_REG_METHOD(Object, GetAttactedObjectPos)
		LW_REG_METHOD(Object, SetAttachedObject)
		LW_REG_METHOD(Object, GetActor)
		LW_REG_METHOD(Object, GetParentGroupGUID)
		LW_REG_METHOD(Object, GetVerticalLocation)
		LW_REG_METHOD(Object, GetRidingObjectPosition)
		LW_REG_METHOD(Object, Break)
		;
}
lwPoint3	lwObject::GetRidingObjectPosition(float fElapsedTime)
{
	return	lwPoint3(m_pkObject->GetRidingObjectPosition(fElapsedTime));
}
lwPoint3	lwObject::GetAttactedObjectPos()
{
	return	lwPoint3(m_pkObject->GetAttactedObjectPos());
}
void	lwObject::SetAttachedObject(char const *objname)
{
	m_pkObject->SetAttachedObject(objname);
}

lwActor lwObject::GetActor()
{
	PgActor	*pkActor = dynamic_cast<PgActor*>(m_pkObject);
	return	lwActor(pkActor);
}

lwGUID lwObject::GetParentGroupGUID()
{
	return	lwGUID(m_pkObject->GetParentGroupGUID());
}
int	lwObject::GetVerticalLocation()
{
	return	m_pkObject->GetVerticalLocation();
}
void	lwObject::Break()
{
	m_pkObject->Break();
}

