#include "stdafx.h"
#include "lwActionTargetInfo.h"
#include "PgAction.H"
#include "lwGUID.H"
#include "lwPoint3.H"
#include "lwActionResult.H"

using namespace lua_tinker;

void lwActionTargetInfo::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(ActionTargetInfo)
		LW_REG_METHOD(ActionTargetInfo, GetTargetGUID)
		LW_REG_METHOD(ActionTargetInfo, GetABVIndex)
		LW_REG_METHOD(ActionTargetInfo, GetRemainHP)
		LW_REG_METHOD(ActionTargetInfo, GetActionResult)
		LW_REG_METHOD(ActionTargetInfo, SetActionResult)
		;

}

lwGUID	lwActionTargetInfo::GetTargetGUID()
{
	return	lwGUID(m_pkActionTargetInfo ? m_pkActionTargetInfo->GetTargetPilotGUID() : BM::GUID::NullData());
}
int	lwActionTargetInfo::GetRemainHP()
{
	return	m_pkActionTargetInfo->GetRemainHP();
}
void	lwActionTargetInfo::SetActionResult(lwActionResult kActionResult)
{
	m_pkActionTargetInfo->SetActionResult(*(kActionResult()));
}

int lwActionTargetInfo::GetABVIndex()
{
	return	m_pkActionTargetInfo->GetSphereIndex();
}
lwActionResult lwActionTargetInfo::GetActionResult()
{
	return	lwActionResult(&m_pkActionTargetInfo->GetActionResult());
}
