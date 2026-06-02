#include "stdafx.h"
#include "lwInputSlotInfo.h"
#include "PgPilot.h"

using namespace lua_tinker;

void lwInputSlotInfo::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(InputSlotInfo)
		LW_REG_METHOD(InputSlotInfo, GetActionID)
		LW_REG_METHOD(InputSlotInfo, GetUKey)
		LW_REG_METHOD(InputSlotInfo, IsRecord)
		LW_REG_METHOD(InputSlotInfo, GetUserDataAsString)
		;
}
const	char*	lwInputSlotInfo::GetActionID()const
{
	return	m_pkInputSlotInfo->GetActionID().c_str();
}
unsigned	int	lwInputSlotInfo::GetUKey()
{
	return	m_pkInputSlotInfo->GetUKey();
}
bool	lwInputSlotInfo::IsRecord()
{
	return	m_pkInputSlotInfo->IsRecord();
}
char const*	lwInputSlotInfo::GetUserDataAsString()const
{
	return	(char const*)(m_pkInputSlotInfo->GetUserData());
}