#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCMGR_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCMGR_H

class	PgActor;
class	PgAction;
class	PgActionTargetList;

class	lwAction;
class	lwActor;
class	lwActionTargetList;
class	lwPacket;
class	lwActionResult;
class	lwActionTargetInfo;

class	PgActionFSM_Base
{
public:

	virtual	bool	OnEnter(lwActor actor,lwAction action)	const;
	virtual	bool	OnCheckCanEnter(lwActor actor,lwAction action)	const;
	virtual	bool	OnUpdate(lwActor actor,lwAction action,float accumTime,float frameTime)	const;
	virtual	bool	OnCleanUp(lwActor actor,lwAction action)	const;
	virtual	bool	OnLeave(lwActor actor,lwAction action,bool bCancel)	const;
	virtual	bool	OnTimer(lwActor actor,lwAction action,float fCallTime,int iTimerID)	const;
	virtual	int		OnFindTarget(lwActor actor,lwAction action,lwActionTargetList kTargetList);
	virtual	void	OnTargetListModified(lwActor actor,lwAction action,bool bIsBefore)	const;
	virtual	bool	OnEvent(lwActor actor,std::string kTextKey,int iSeqID)	const;
	virtual	void	OnCastingCompleted(lwActor actor,lwAction action)	const;
	virtual	void	OnOverridePacket(lwActor actor,lwAction action,lwPacket kPacket)	const;
};


class	PgActionFSMFuncMgr
{
private:

	typedef std::map<std::string,PgActionFSM_Base*> ActionFSMCont;

	ActionFSMCont	m_kFSMCont;

public:

	PgActionFSMFuncMgr();
	virtual	~PgActionFSMFuncMgr();

	PgActionFSM_Base* GetFSM(std::string const &kFSMName);

};

extern	PgActionFSMFuncMgr	g_kActionFSMFuncMgr;

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONFSMFUNCMGR_H