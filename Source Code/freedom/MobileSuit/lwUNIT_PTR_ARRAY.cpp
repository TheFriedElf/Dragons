#include "stdafx.h"
#include "lwUNIT_PTR_ARRAY.h"

///////////////////////////////////////////////////////////////////////
//	lwUNIT_PTR_ARRAY
///////////////////////////////////////////////////////////////////////

using namespace lua_tinker;

lwUNIT_PTR_ARRAY lwNewUNIT_PTR_ARRAY()
{
	UNIT_PTR_ARRAY* pkNew = new UNIT_PTR_ARRAY;
	return lwUNIT_PTR_ARRAY(pkNew);
}

void lwDeleteUNIT_PTR_ARRAY(lwUNIT_PTR_ARRAY kObject)
{
	PG_ASSERT_LOG(kObject());
	delete kObject();
}

void lwUNIT_PTR_ARRAY::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "NewUNIT_PTR_ARRAY", &lwNewUNIT_PTR_ARRAY);
	def(pkState, "DeleteUNIT_PTR_ARRAY", &lwDeleteUNIT_PTR_ARRAY);

	LW_REG_CLASS(UNIT_PTR_ARRAY)
		LW_REG_METHOD(UNIT_PTR_ARRAY, GetUnit)
		LW_REG_METHOD(UNIT_PTR_ARRAY, AddUnit)
		;
}

lwUnit lwUNIT_PTR_ARRAY::GetUnit(int iIndex)
{
	assert(m_pkUNIT_PTR_ARRAY);
	if (m_pkUNIT_PTR_ARRAY == NULL)
		return NULL;

	UNIT_PTR_ARRAY::const_iterator itor = m_pkUNIT_PTR_ARRAY->begin();
	int iCount = 0;
	while (itor != m_pkUNIT_PTR_ARRAY->end())
	{
		if (iCount == iIndex)
		{
			return lwUnit(itor->pkUnit);
		}
		++iCount;
		++itor;
	}
	return NULL;
}

void lwUNIT_PTR_ARRAY::AddUnit(lwUnit kUnit)
{
	assert(m_pkUNIT_PTR_ARRAY);

	m_pkUNIT_PTR_ARRAY->Add(kUnit());
}