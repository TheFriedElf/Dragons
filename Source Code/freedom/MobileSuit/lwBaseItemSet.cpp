#include "stdafx.h"
#include "lwBaseItemSet.h"
#include "PgBaseItemSet.h"

lwBaseItemSet::lwBaseItemSet(PgBaseItemSet *pkBaseItemSet)
{
	m_pkBaseItemSet = pkBaseItemSet;
}

bool lwBaseItemSet::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwBaseItemSet>(pkState, "BaseItemSet")
		.def(pkState, "IsNil", &lwBaseItemSet::IsNil)
		.def(pkState, "AddItem", &lwBaseItemSet::AddItem)
		.def(pkState, "AddItemInfo", &lwBaseItemSet::AddItemInfo)
		.def(pkState, "SetExposedSlot", &lwBaseItemSet::SetExposedSlot)
		;

	return true;
}

bool lwBaseItemSet::IsNil()
{
	return (m_pkBaseItemSet == 0);
}

int lwBaseItemSet::AddItem(int iItemNo, char const *pcIconPath)
{
	PgBaseItemSet::PgBaseItem *pkItem = new PgBaseItemSet::PgBaseItem(std::string(pcIconPath));
	m_pkBaseItemSet->AddItem(pkItem);
	return m_pkBaseItemSet->GetSize()-1;
}

void lwBaseItemSet::AddItemInfo(int iIndex, int iItemNo)
{
	PgBaseItemSet::PgBaseItem *pkItem = m_pkBaseItemSet->GetItem(iIndex);
	if(!pkItem)
	{
		return;
	}

	pkItem->AddItem(iItemNo);
}

void lwBaseItemSet::SetExposedSlot(int iNbExposedSlot)
{
	m_pkBaseItemSet->SetExposedSlot(iNbExposedSlot);
}

PgBaseItemSet *lwBaseItemSet::operator()()
{
	return m_pkBaseItemSet;
}