#ifndef DRAGONICA_MOBILESUIT_PgCommonSkillActionFSM_H
#define DRAGONICA_MOBILESUIT_PgCommonSkillActionFSM_H

class	PgActor;
class	PgAction;
class	PgActionTargetList;

class	lwAction;
class	lwActor;
class	lwActionTargetList;
class	lwPacket;
class	PgActionFSM_Base;

class	PgCommonSkillAction
	: public	PgActionFSM_Base
{
public:
	PgCommonSkillAction();
	
	static	char	const*	GetID()	{	return	"CommonSkillAction";	}

	virtual	bool	OnCheckCanEnter(lwActor kActor, lwAction kAction)	const;
	
	virtual	bool	OnEnter(lwActor kActor, lwAction kAction)	const;
	virtual	void	OnCastingCompleted(lwActor kActor, lwAction kAction)	const;
	virtual	void	OnOverridePacket(lwActor kActor, lwAction kAction, lwPacket kPacket)	const;
	virtual	bool	OnUpdate(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)	const;
	virtual	bool	OnLeave(lwActor kActor, lwAction kAction, bool bCancel)	const;
	virtual	bool	OnCleanUp(lwActor kActor, lwAction kAction)	const;
		
	virtual	bool	OnEvent(lwActor kActor, std::string kTextKey, int iSeqID)	const;
	virtual	bool	OnTimer(lwActor kActor, lwAction kAction, float fCallTime, int iTimerID)	const;
	
	virtual	int		OnFindTarget(lwActor kActor, lwAction kAction, lwActionTargetList kTargetList);
	virtual	void	OnTargetListModified(lwActor kActor, lwAction kAction, bool bIsBefore)	const;
};

#endif