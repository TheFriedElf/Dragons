
#include "stdafx.h"
#include "PgActionFSMFuncMgr.H"
#include "PgActionFSMFuncs.h"
#include "PgCommonSkillActionFSM.h"

#include "lwActor.H"
#include "lwAction.H"
#include "lwActionTargetList.H"
#include "lwPacket.H"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSM_Base
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	PgActionFSM_Base::OnEnter(lwActor actor,lwAction action)	const	{		return	true;	}
bool	PgActionFSM_Base::OnCheckCanEnter(lwActor actor,lwAction action)	const	{		return	true;	}
bool	PgActionFSM_Base::OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const	{		return	true;	}
bool	PgActionFSM_Base::OnCleanUp(lwActor actor,lwAction action)	const	{		return	true;	}
bool	PgActionFSM_Base::OnLeave(lwActor actor,lwAction action,bool bCancel)	const	{		return	true;	}
bool	PgActionFSM_Base::OnTimer(lwActor actor,lwAction action,float fCallTime,int iTimerID)	const	{		return	true;	}
int		PgActionFSM_Base::OnFindTarget(lwActor actor,lwAction action,lwActionTargetList kTargetList)	{	return	0;	}
void	PgActionFSM_Base::OnTargetListModified(lwActor actor,lwAction action,bool bIsBefore)	const	{	}
bool	PgActionFSM_Base::OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const	{		return	true;	}
void	PgActionFSM_Base::OnCastingCompleted(lwActor actor,lwAction action)	const	{	}
void	PgActionFSM_Base::OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const	{	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionFSMFuncMgr
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgActionFSMFuncMgr	g_kActionFSMFuncMgr;

PgActionFSMFuncMgr::PgActionFSMFuncMgr()
{
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Idle::GetID(),new PgActionFSM_Act_Idle()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Walk::GetID(),new PgActionFSM_Act_Walk()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Run::GetID(),new PgActionFSM_Act_Run()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Dash::GetID(),new PgActionFSM_Act_Dash()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Dash_Jump::GetID(),new PgActionFSM_Act_Dash_Jump()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_Jump::GetID(), new PgActionFSM_Act_Jump()));
	
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Skill_Blitz_Play::GetID(), new PgActionFSM_Skill_Blitz_Play()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Skill_Wolf_Rush_Play::GetID(), new PgActionFSM_Skill_Wolf_Rush_Play()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Skill_Falcon_Beat_Play::GetID(), new PgActionFSM_Skill_Falcon_Beat_Play()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Skill_Bro_Spin_Fire::GetID(), new PgActionFSM_Skill_Bro_Spin_Fire()));

	m_kFSMCont.insert(std::make_pair(PgCommonSkillAction::GetID(), new PgCommonSkillAction()));

	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_IdleBear::GetID(), new PgActionFSM_Act_IdleBear()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_RunBear::GetID(), new PgActionFSM_Act_RunBear()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_DashBear::GetID(), new PgActionFSM_Act_DashBear()));
	m_kFSMCont.insert(std::make_pair(PgActionFSM_Act_JumpBear::GetID(), new PgActionFSM_Act_JumpBear()));

}
PgActionFSMFuncMgr::~PgActionFSMFuncMgr()
{
	for(ActionFSMCont::iterator itor = m_kFSMCont.begin(); itor != m_kFSMCont.end(); itor++)
	{
		PgActionFSM_Base	*pkFunc = itor->second;
		SAFE_DELETE(pkFunc);
	}
	m_kFSMCont.clear();
}
PgActionFSM_Base* 	PgActionFSMFuncMgr::GetFSM(std::string const &kFuncName)
{
	ActionFSMCont::iterator itor = m_kFSMCont.find(kFuncName);
	if(itor == m_kFSMCont.end())
	{
		return	NULL;
	}

	PgActionFSM_Base	*pkFunc = (itor->second);

	return	pkFunc;
}
