#include "StdAfx.h"
#include "lwTrigger.h"
#include "PgTrigger.h"
#include "lwActor.h"
#include "PgPilotMan.h"
#include "PgPilot.h"

lwTrigger::lwTrigger(PgTrigger *pkTrigger)
{
	m_pkTrigger = pkTrigger;
	if (m_pkTrigger)
	{
		m_pkTrigger->Enable(true);
	}
}

bool lwTrigger::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwTrigger>(pkState, "Trigger")
		.def(pkState, constructor<PgTrigger *>())
		.def(pkState, "GetID", &lwTrigger::GetID)
		.def(pkState, "GetParam", &lwTrigger::GetParam)
		.def(pkState, "GetParam2", &lwTrigger::GetParam2)
		.def(pkState, "GetRagUI", &lwTrigger::GetRagUI)
		.def(pkState, "GetParamAsString", &lwTrigger::GetParamAsString)
		.def(pkState, "SetParam", &lwTrigger::SetParam)
		.def(pkState, "SetParam2", &lwTrigger::SetParam2)
		.def(pkState, "SetParamAsString", &lwTrigger::SetParamAsString)
		.def(pkState, "IsEnable", &lwTrigger::IsEnable)
		.def(pkState, "IsNil", &lwTrigger::IsNil)
		.def(pkState, "SetEnable", &lwTrigger::SetEnable)
		.def(pkState, "GetParamFromParamMap", &lwTrigger::GetParamFromParamMap)
		.def(pkState, "GetParamIntFromParamMap", &lwTrigger::GetParamIntFromParamMap)
		.def(pkState, "GetConditionType", &lwTrigger::GetConditionType)
		.def(pkState, "GetConditionAction", &lwTrigger::GetConditionAction)
		.def(pkState, "GetIndex", &lwTrigger::GetIndex)
		.def(pkState, "IsHavePortalAccess", &lwTrigger::IsHavePortalAccess)
		.def(pkState, "IsPortalAccess", &lwTrigger::IsPortalAccess )
		.def(pkState, "GetPortalAccessName", &lwTrigger::GetPortalAccessName)
		.def(pkState, "OnAction", &lwTrigger::OnAction)
		.def(pkState, "GetTriggerType", &lwTrigger::GetTriggerType)
		.def(pkState, "GetTranslate", &lwTrigger::GetTranslate)
		.def(pkState, "GetTeleportID", &lwTrigger::GetTeleportID)
		.def(pkState, "GetErrorMsgID", &lwTrigger::GetErrorMsgID)
		.def(pkState, "GetAddedType", &lwTrigger::GetAddedType)
		.def(pkState, "GetDoOnLeaveScript", &lwTrigger::GetDoOnLeaveScript)
		.def(pkState, "IsActiveTime", &lwTrigger::IsActiveTime)
		.def(pkState, "GetTriggerReactionSkillName", &lwTrigger::GetTriggerReactionSkillName)
		.def(pkState, "GetTriggerReactionSkillNo", &lwTrigger::GetTriggerReactionSkillNo)
		.def(pkState, "GetTriggerSkillID", &lwTrigger::GetTriggerSkillID)
		.def(pkState, "GetProgressID", &lwTrigger::GetProgressID)
		.def(pkState, "GetLoadingImageID", &lwTrigger::GetLoadingImageID)
		;

	return true;
}
PgTrigger* lwTrigger::GetSelf()
{
	return m_pkTrigger;
}

char* lwTrigger::GetConditionAction()
{
	return	(char*)m_pkTrigger->GetConditionAction().c_str();
}
int	lwTrigger::GetConditionType()
{
	return	(int)m_pkTrigger->GetConditionType();
}
char const*	lwTrigger::GetParamFromParamMap(char const *strKeyString)
{
	return	m_pkTrigger->GetParamFromParamMap(strKeyString);
}
int	lwTrigger::GetParamIntFromParamMap(char const *strKeyString)
{
	char	const *pkValue = GetParamFromParamMap(strKeyString);
	if(pkValue == NULL)
	{
		return	-1;
	}
	return	atoi(pkValue);
}

bool	lwTrigger::SetParamFromParamMap(char const* szKeyString, char const* szValueString)
{
	return m_pkTrigger->SetParamFromParamMap(szKeyString, szValueString);
}

char const * lwTrigger::GetID()const
{
	if ( m_pkTrigger )
	{
		return m_pkTrigger->GetID().c_str();
	}

	return NULL;
}

int lwTrigger::GetParam()
{
	return m_pkTrigger->Param();
}

int lwTrigger::GetParam2()
{
	return m_pkTrigger->Param2();
}

int lwTrigger::GetRagUI()
{
	return m_pkTrigger->RagUI();
}

void lwTrigger::SetParam(int const iNewParam)
{
	m_pkTrigger->Param(iNewParam);
}

void lwTrigger::SetParam2(int const iNewParam)
{
	m_pkTrigger->Param2(iNewParam);
}

lwWString lwTrigger::GetParamAsString()
{
	return m_pkTrigger->ParamString().c_str();
}

void lwTrigger::SetParamAsString(char const* szNewParam)
{
	if(szNewParam) {return;}
	m_pkTrigger->ParamString(UNI(szNewParam));
}

bool lwTrigger::IsNil()
{
	return m_pkTrigger==NULL;
}

bool lwTrigger::IsEnable()
{
	return m_pkTrigger->Enable();
}
void lwTrigger::SetEnable(bool const bEnable)
{
	m_pkTrigger->Enable(bEnable);
}

int lwTrigger::GetIndex()
{
	return m_pkTrigger->GetIndex();
}

bool lwTrigger::IsHavePortalAccess( size_t const iIndex )const
{
	if ( m_pkTrigger )
	{
		return m_pkTrigger->IsHavePortalAccess(iIndex);
	}
	return false;
}

bool lwTrigger::IsPortalAccess( size_t const iIndex )const
{
	if ( m_pkTrigger )
	{
		return m_pkTrigger->IsPortalAccess( iIndex );
	}
	return false;
}

int lwTrigger::GetPortalAccessName( size_t const iIndex )const
{
	if ( m_pkTrigger )
	{
		return m_pkTrigger->GetPortalAccessName(iIndex);
	}
	return 0;
}

bool lwTrigger::OnAction(lwActor kActor)
{
	if ( m_pkTrigger )
	{
		return m_pkTrigger->OnAction(kActor());
	}
	return false;
}
int lwTrigger::GetTriggerType()
{
	if ( m_pkTrigger )
	{
		return static_cast<int>(m_pkTrigger->GetTriggerType());
	}
	return static_cast<int>(PgTrigger::TRIGGER_TYPE_NONE);
}

lwPoint3 lwTrigger::GetTranslate()const
{
	return lwPoint3(m_pkTrigger->GetTriggerObjectPos());
}

lwWString lwTrigger::GetTeleportID()
{
	std::wstring kQuestTeleportID = m_pkTrigger->FindQuestTeleport();
	if( kQuestTeleportID.size() )
	{
		return lwWString( kQuestTeleportID );
	}
	return m_pkTrigger->ParamString().c_str();
}

int lwTrigger::GetErrorMsgID()
{
	return m_pkTrigger->ErrorMsgID();
}

int lwTrigger::GetAddedType()
{
	return static_cast<int>(m_pkTrigger->EAddedType());
}

bool lwTrigger::GetDoOnLeaveScript()
{
	return m_pkTrigger->DoOnLeaveScript();
}

bool lwTrigger::IsActiveTime(void) const
{
	return m_pkTrigger->IsActiveTime();
}

int lwTrigger::GetLoadingImageID()const
{
	if( m_pkTrigger )
	{
		return m_pkTrigger->LoadingImageID();
	}
	return 0;
}lwWString lwTrigger::GetTriggerReactionSkillName(lwActor kActor)
{
	if(kActor.IsNil() || kActor.GetPilotGuid().IsNil() || kActor.GetPilotGuid().GetGUID().IsNull())
	{
		return lwWString("");
	}

	BM::GUID kGuid = kActor.GetPilotGuid().GetGUID();
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	if(!pkPilot)
	{
		return lwWString("");
	}

	CUnit* pkTargetUnit = pkPilot->GetUnit();
	if(!pkTargetUnit || !m_pkTrigger)
	{
		return lwWString("");
	}

	std::wstring strReturn = m_pkTrigger->GetTriggerReactionSkillName(pkTargetUnit);
	if(strReturn.empty())
	{
		return lwWString("");
	}

	return lwWString(strReturn.c_str());
	

}

int lwTrigger::GetTriggerReactionSkillNo(lwActor kActor)
{
	if(kActor.IsNil() || kActor.GetPilotGuid().IsNil() || kActor.GetPilotGuid().GetGUID().IsNull())
	{
		return 0;
	}

	BM::GUID kGuid = kActor.GetPilotGuid().GetGUID();
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	if(!pkPilot)
	{
		return 0;
	}

	CUnit* pkTargetUnit = pkPilot->GetUnit();
	if(!pkTargetUnit || !m_pkTrigger)
	{
		return 0;
	}

	return m_pkTrigger->GetTriggerReactionSkillNo(pkTargetUnit);
}

int lwTrigger::GetTriggerSkillID(void) const
{
	if( m_pkTrigger )
	{
		return m_pkTrigger->TriggerSkillID();
	}
	return 0;
}

int lwTrigger::GetProgressID(void) const
{
	if( m_pkTrigger )
	{
		return m_pkTrigger->GetProgressID();
	}
	return 0;
}
