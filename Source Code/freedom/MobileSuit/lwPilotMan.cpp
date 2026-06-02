#include "StdAfx.h"
#include "lwPilotMan.h"
#include "PgPilotMan.h"
#include "PgXmlLoader.h"

lwPilotMan::lwPilotMan(PgPilotMan *pkPilotMan)
{
	m_pkPilotMan = pkPilotMan;
}

bool lwPilotMan::RegisterWrapper(lua_State *pkState)
{ 
	using namespace lua_tinker; 

	class_<lwPilotMan>(pkState, "PilotMan")
		.def(pkState, constructor<PgPilotMan *>())
		.def(pkState, "NewPilot", &lwPilotMan::NewPilot)
		.def(pkState, "FindPilot", &lwPilotMan::FindPilot)
		.def(pkState, "FindPilotByName", &lwPilotMan::FindPilotByName)
		.def(pkState, "FindPilotInRange", &lwPilotMan::FindPilotInRange)
		.def(pkState, "FindActorByClassNo", &lwPilotMan::FindActorByClassNo)
		.def(pkState, "IsReservedPilot", &lwPilotMan::IsReservedPilot)
		.def(pkState, "RemovePilot", &lwPilotMan::RemovePilot)
		.def(pkState, "RemoveReservedPilot", &lwPilotMan::RemoveReservedPilot)
		.def(pkState, "GetPlayerPilot", &lwPilotMan::GetPlayerPilot)
		.def(pkState, "GetPlayerUnit", &lwPilotMan::GetPlayerUnit)
		.def(pkState, "SetPlayerPilotGuid", &lwPilotMan::SetPlayerPilotGuid)
		.def(pkState, "GetPlayerPilotGuid", &lwPilotMan::GetPlayerPilotGuid)
		.def(pkState, "InsertPilot",&lwPilotMan::InsertPilot)
		//.def(pkState, "SetLockMyInput", &lwPilotMan::SetLockMyInput)
		.def(pkState, "GetPlayerActor", &lwPilotMan::GetPlayerActor)
		.def(pkState, "IsMyPlayer", &lwPilotMan::IsMyPlayer)
		.def(pkState, "IsLockMyInput", &lwPilotMan::IsLockMyInput)		
		.def(pkState, "GetHyperJumpGuid", &lwPilotMan::GetHyperJumpGuid)
		.def(pkState, "ClearHyperJumpGuid", &lwPilotMan::ClearHyperJumpInfo)
		.def(pkState, "BroadcastDirection", &lwPilotMan::BroadcastDirection)
		.def(pkState, "IsPremiumService", &lwPilotMan::IsPremiumService)
		.def(pkState, "GetPremiumGrade", &lwPilotMan::GetPremiumGrade)
		.def(pkState, "IsJumpingEvent", &lwPilotMan::IsJumpingEvent)
		.def(pkState, "GetJumpingCreateCharCount", &lwPilotMan::GetJumpingCreateCharCount)
		.def(pkState, "GetJumpingRemainCount", &lwPilotMan::GetJumpingRemainCount)
		.def(pkState, "IsCreateJumpingCharClass", &lwPilotMan::IsCreateJumpingCharClass)
		.def(pkState, "GetCreateJumpingCharClass", &lwPilotMan::GetCreateJumpingCharClass)
		.def(pkState, "GetCreateJumpingCharLevel", &lwPilotMan::GetCreateJumpingCharLevel)
		.def(pkState, "SetJumpingSelectIdx", &lwPilotMan::SetJumpingSelectIdx)
		.def(pkState, "NotUseJumpingEvent", &lwPilotMan::NotUseJumpingEvent)
		.def(pkState, "SetHideBalloon", &lwPilotMan::SetHideBalloon)
		;

	return true;
}

lwPilot	lwPilotMan::GetPlayerPilot()
{
	return	lwPilot(g_kPilotMan.GetPlayerPilot());
}

lwActor lwPilotMan::GetPlayerActor()
{
	return lwActor(g_kPilotMan.GetPlayerActor());
}

lwUnit lwPilotMan::GetPlayerUnit(void)
{
	return lwUnit( g_kPilotMan.GetPlayerUnit() );
}

bool lwPilotMan::IsReservedPilot(lwGUID kGuid)
{
	return	g_kPilotMan.IsReservedPilot(kGuid());
}

void	lwPilotMan::SetPlayerPilotGuid(lwGUID kGuid)
{
	g_kPilotMan.SetPlayerPilot(kGuid());
}
lwGUID	lwPilotMan::GetPlayerPilotGuid()
{
	BM::GUID kPlayerPilotGuid;
	bool const bRet = g_kPilotMan.GetPlayerPilotGuid(kPlayerPilotGuid);
	if( bRet )
	{
		return (lwGUID)kPlayerPilotGuid;//성공
	}
	assert(NULL);
	return (lwGUID)BM::GUID::NullData();//실패
}

bool lwPilotMan::IsMyPlayer( lwGUID kGuid )
{
	return g_kPilotMan.IsMyPlayer( kGuid.GetGUID() );
}

lwPilot lwPilotMan::NewPilot(lwGUID kGuid, int iClass, int iGender,char const* pcObjectType)
{
	return (lwPilot)g_kPilotMan.NewPilot(kGuid(), iClass, iGender,pcObjectType);
}
bool lwPilotMan::InsertPilot(lwGUID kGuid, lwPilot kPilot)
{
	return g_kPilotMan.InsertPilot(kGuid(), kPilot());
}
lwPilot lwPilotMan::FindPilotByName(lwWString kName, bool bFindPassPlayerPilot)
{
	return (lwPilot)g_kPilotMan.FindPilot(kName(), bFindPassPlayerPilot);

}
lwPilot lwPilotMan::FindPilot(lwGUID kGuid)
{
	return (lwPilot)g_kPilotMan.FindPilot(kGuid());
}

lwPilot lwPilotMan::FindPilotInRange(int iUnitType, lwPoint3 kPos, int iRange)
{
	return (lwPilot)g_kPilotMan.FindPilotInRange(static_cast<EUnitType>(iUnitType), kPos(), iRange);
}
	
bool lwPilotMan::RemovePilot(lwGUID kGuid)
{
	return g_kPilotMan.RemovePilot(kGuid());
}

bool lwPilotMan::RemoveReservedPilot(lwGUID kGuid)
{
	return g_kPilotMan.RemoveReservedPilot(kGuid());
}

//void lwPilotMan::SetLockMyInput(bool bLock)
//{
//	g_kPilotMan.SetLockMyInput(bLock);
//}

lwActor lwPilotMan::FindActorByClassNo(int iClassNo)
{
	return lwActor(g_kPilotMan.FindActorByClassNo(iClassNo));	
}

bool lwPilotMan::IsLockMyInput()
{
	return g_kPilotMan.IsLockMyInput();
}

lwGUID lwPilotMan::GetHyperJumpGuid()
{
	if(m_pkPilotMan)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(m_pkPilotMan->GetPlayerUnit());
		if(pkPlayer)
		{
			return lwGUID(pkPlayer->GetHyperJumpGuid());
		}
	}

	return lwGUID("");
}

void lwPilotMan::ClearHyperJumpInfo()
{
	if(m_pkPilotMan)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(m_pkPilotMan->GetPlayerUnit());
		if(pkPlayer)
		{
			pkPlayer->ClearHyperJumpInfo();
		}
	}
}

bool lwPilotMan::BroadcastDirection(lwPilot kPilot, int iDir)
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->BroadcastDirection(kPilot(), static_cast<Direction>(iDir));
	}
	return false;
}

bool lwPilotMan::IsPremiumService()const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->MemberPremium().iServiceNo;
	}
	return false;
}

int lwPilotMan::GetPremiumGrade()const
{
	if(m_pkPilotMan)
	{
		GET_DEF(PgDefPremiumMgr, kDefPremium);
		SPremiumData const* pkDefPremium = kDefPremium.GetDef(m_pkPilotMan->MemberPremium().iServiceNo);
		if(pkDefPremium)
		{
			return pkDefPremium->byGrade;
		}
	}
	return 0;
}

bool lwPilotMan::IsJumpingEvent()const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->JumpingCharEvent().iNo>0;
	}
	return false;
}

int lwPilotMan::GetJumpingCreateCharCount()const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->GetJumpingCreateCharCount();
	}
	return 0;
}

int lwPilotMan::GetJumpingRemainCount()const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->JumpingCharEvent().iRemain;
	}
	return 0;
}

bool lwPilotMan::IsCreateJumpingCharClass(int const iClassNo)const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->IsCreateJumpingCharClass(iClassNo);
	}
	return 0;
}

int lwPilotMan::GetCreateJumpingCharClass(int const iIdx)const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->GetCreateJumpingCharClass(iIdx);
	}
	return 0;
}

int lwPilotMan::GetCreateJumpingCharLevel(int const iIdx)const
{
	if(m_pkPilotMan)
	{
		return m_pkPilotMan->GetCreateJumpingCharLevel(iIdx);
	}
	return 0;
}

void lwPilotMan::SetJumpingSelectIdx(int const iIdx)
{
	if(m_pkPilotMan)
	{
		m_pkPilotMan->SetJumpingSelectIdx(iIdx);
	}
}

void lwPilotMan::NotUseJumpingEvent()
{
	if(m_pkPilotMan)
	{
		m_pkPilotMan->UseJumpingEvent(false);
	}
}

void lwPilotMan::SetHideBalloon(bool const bHide)
{
	if(m_pkPilotMan)
	{
		m_pkPilotMan->SetHideBalloon(bHide);
	}
}