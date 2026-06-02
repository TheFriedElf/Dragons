#ifndef FREEDOM_DRAGONICA_RENDER_ACTION_PGACTIONEFFECT_H
#define FREEDOM_DRAGONICA_RENDER_ACTION_PGACTIONEFFECT_H

#include "PgIXmlObject.h"
class	PgActor;
class	PgAction;

class	PgActionEffect
{
private:

	PgActor	*m_pkParentActor;

	bool	m_bIsTemporaryEffect;
	DWORD	m_dwTimeStamp;
	int	m_iActionInstanceID;
	BM::GUID	m_kCasterGUID;
	int	m_iActionID;

	std::string	m_kActionID;
	bool	m_bDoNotTransitToLastAction;

public:

	PgActionEffect(PgActor *pkParentActor,BM::GUID const &kCasterGUID,int iActionInstanceID,int iActionID,bool bIsTemporaryEffect,DWORD dwTimeStamp);
	virtual	~PgActionEffect();

	void	OnTopLevelEnter();
	void	OnTopLevelLeave();
	void	OnEnter();
	void	OnLeave();
	bool	OnUpdate(float fAccumTime,float fFrameTime);

	void	SetToRealEffect(DWORD dwTimeStamp);

public:

	bool	DoNotTransitToLastAction()	{	return	m_bDoNotTransitToLastAction;	}
	void	DoNotTransitToLastAction(bool bDoNot)	{	m_bDoNotTransitToLastAction = bDoNot;	}

	BM::GUID& GetCasterGUID()	{	return	m_kCasterGUID;	}
	int	GetActionInstanceID()	{	return	m_iActionInstanceID;	}
	bool	IsTemporaryEffect()	{	return	m_bIsTemporaryEffect;	}
	DWORD	GetTimeStamp()	{	return	m_dwTimeStamp;	}
	int	GetActionID()	{	return	m_iActionID;	}

};

typedef std::list<PgActionEffect*>	ActionEffectList;

class	PgActionEffectStack
{
	
	ActionEffectList	m_kActionEffectStack;
	PgActor	*m_pkParentActor;
	PgAction	*m_pkSavedLastAction;

public:

	PgActionEffectStack(PgActor *pkParentActor);
	virtual	~PgActionEffectStack();

	void	AddNewEffect(BM::GUID const &kCasterGUID,int iActionInstanceID,bool bIsTemporaryEffect,int iEffectID,DWORD dwTimeStamp);

	void	Update(float fAccumTime,float fFrameTime);
	void	ClearAll();

	bool	IsTopLevelActionEffect(PgAction *pkAction);

	PgActionEffect*	GetActionEffect(BM::GUID const &kCasterGUID,int iActionInstanceID);
	PgActionEffect*	GetActionEffect(int iEffectID);

	bool	SaveLastAction(PgAction *pkAction);

	bool	IsEmpty()	{	return	m_kActionEffectStack.size()==0;	}
private:

	void	ReleaseEffect(PgActionEffect *pkActionEffect);
	void	SetToRealEffect(PgActionEffect	*pkActionEffect,DWORD dwTimeStamp);
	void	AddToTopLevel(PgActionEffect	*pkActionEffect);
	void	ReleaseAllEffect(int iEffectID,bool bDoSavedAction=true);	//	iEffectID 에 해당하는 모든 이펙트를 제거한다.

	void	DoSavedAction();
};

#endif // FREEDOM_DRAGONICA_RENDER_ACTION_PGACTIONEFFECT_H