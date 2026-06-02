#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCS_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCS_H

#include "PgActionFSMFuncMgr.H"

extern	void DoAutoFire(lwActor actor);

// Action
class	PgActionFSM_Act_Idle
	: public PgActionFSM_Base
{

public:
	static	char	const*	GetID()	{	return	"Act_Idle";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
};

class	PgActionFSM_Act_Walk
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Walk";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
};

class	PgActionFSM_Act_Run
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Run";	}

	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
};

class	PgActionFSM_Act_Dash
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Dash";	}

	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
};

class	PgActionFSM_Act_Dash_Jump
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Dash_Jump";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
};

class PgActionFSM_Act_Jump
	: public PgActionFSM_Base
{
public:
	static char const* GetID() { return "Act_Jump"; }
	
	bool OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool OnEnter(lwActor actor,lwAction action) const;
	void OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const;
	bool OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime) const;
	bool OnCleanUp(lwActor actor,lwAction action) const;
	bool OnLeave(lwActor actor,lwAction action,bool bCancel) const;

	void CheckBreakFall(lwActor actor) const;
	bool IsFloatEvasion(lwActor actor, lwAction action) const;
	void SetComboAdvisor(lwActor actor, lwAction action) const;
};

class	PgActionFSM_Act_IdleBear
	: public PgActionFSM_Base
{

public:
	static	char	const*	GetID()	{	return	"Act_Idle_Bear";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
};

class	PgActionFSM_Act_RunBear
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Run_Bear";	}

	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
};

class	PgActionFSM_Act_DashBear
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Act_Dash_Bear";	}

	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
};

class PgActionFSM_Act_JumpBear
	: public PgActionFSM_Base
{
public:
	static char const* GetID() { return "Act_Jump_Bear"; }
	
	bool OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool OnEnter(lwActor actor,lwAction action) const;
	void OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const;
	bool OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime) const;
	bool OnCleanUp(lwActor actor,lwAction action) const;
	bool OnLeave(lwActor actor,lwAction action,bool bCancel) const;

//	void CheckBreakFall(lwActor actor) const;
	bool IsFloatEvasion(lwActor actor, lwAction action) const;
	void SetComboAdvisor(lwActor actor, lwAction action) const;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Skill
class	PgActionFSM_Skill_Blitz_Play
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Skill_Blitz_Play";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
	
};


class	PgActionFSM_Skill_Wolf_Rush_Play
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Skill_Wolf_Rush_Play";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
};

class	PgActionFSM_Skill_Falcon_Beat_Play
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Skill_Falcon_Beat_Play";	}

	bool	OnEnter(lwActor actor,lwAction action)	const;
	bool	OnUpdate(lwActor actor,lwAction action,float fAccumTime,float fFrameTime)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;
};

class	PgActionFSM_Skill_Bro_Spin_Fire
	: public PgActionFSM_Base
{
public:

	static	char	const*	GetID()	{	return	"Skill_Bro_Spin_Fire";	}
	
//	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	bool	OnEnter(lwActor actor,lwAction action)	const;
//	void	OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const;
//	void	OnCastingCompleted(lwActor actor,lwAction action)	const;
	
	bool	OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const;

	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	bool	OnCleanUp(lwActor actor,lwAction action)	const;

	bool	OnTimer(lwActor actor,lwAction action,float fCallTime,int iTimerID)	const;
	int		OnFindTarget(lwActor actor,lwAction action,lwActionTargetList kTargetList);
	void	OnTargetListModified(lwActor actor,lwAction action,bool bIsBefore)	const;
//	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
};

namespace OldActionLuaFunc
{// lua에 있던 함수들을 C로 옮기는중 겹치는 내용을 함수화 한것들
	bool HitOneTime(lwActor actor,lwAction action);
	void DoDamage(lwActor actor, lwActor actorTarget,lwActionResult kActionResult, lwActionTargetInfo kActionTargetInfo);
}

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCS_H