#include "stdafx.h"
#include "lwBreakableObject.H"

using namespace lua_tinker;

void lwBreakableObject::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(BreakableObject)
		LW_REG_METHOD(BreakableObject, GetActor)
		LW_REG_METHOD(BreakableObject, GetParentGroupGUID)
		LW_REG_METHOD(BreakableObject, GetVerticalLocation)
		LW_REG_METHOD(BreakableObject, Break)
		;
}

lwActor lwBreakableObject::GetActor()
{
	PgActor	*pkActor = dynamic_cast<PgActor*>(m_pkBreakableObject);
	return	lwActor(pkActor);
}

lwGUID lwBreakableObject::GetParentGroupGUID()
{
	return	lwGUID(m_pkBreakableObject->GetParentGroupGUID());
}
int	lwBreakableObject::GetVerticalLocation()
{
	return	m_pkBreakableObject->GetVerticalLocation();
}
void	lwBreakableObject::Break()
{
	m_pkBreakableObject->Break();
}

