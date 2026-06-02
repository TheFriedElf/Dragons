#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONTARGETLIST_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONTARGETLIST_H

#include "PgIXmlObject.h"
#include "variant/PgActionResult.H"
#include "PgActor.H"

class PgActor;
class PgInput;
class PgPilot;
class PgAction;
class lwActor;

#define INVALID_REMAIN_HP (-1)
class PgActionTargetInfo
{
private:

	BM::GUID	m_TargetPilotGUID;
	BYTE		m_bySphereIndex;
	NiPoint3	m_kTargetPos;
	int			m_iRemainHP;

	PgActionResult	m_kActionResult;

public:

	PgActionTargetInfo() {};
	PgActionTargetInfo(BM::GUID const& kGUID, BYTE const byABVIndex)
	{
		m_TargetPilotGUID = kGUID;
		m_bySphereIndex = byABVIndex;
		m_iRemainHP = INVALID_REMAIN_HP;
	}

	void	CopyFrom(PgActionTargetInfo &kSrc)	//	RemainHP,ActionResult 만 복사함.
	{
		SetRemainHP(kSrc.GetRemainHP());
		kSrc.GetActionResult().CopyEffectTo(GetActionResult());
		m_kActionResult.SetBlocked(kSrc.GetActionResult().GetBlocked());
		m_kActionResult.SetCollision(kSrc.GetActionResult().GetCollision());
		m_kActionResult.SetComic(kSrc.GetActionResult().GetComic());
		m_kActionResult.SetDead(kSrc.GetActionResult().GetDead());
		m_kActionResult.SetDodged(kSrc.GetActionResult().GetDodged());
		m_kActionResult.SetMissed(kSrc.GetActionResult().GetMissed());
		m_kActionResult.SetInvalid(kSrc.GetActionResult().GetInvalid());
		m_kActionResult.SetRemainHP(kSrc.GetActionResult().GetRemainHP());
		m_kActionResult.SetValue(kSrc.GetActionResult().GetValue());
		m_kActionResult.SetAbsorbValue(kSrc.GetActionResult().GetAbsorbValue());
		m_kActionResult.SetRestore(kSrc.GetActionResult().GetRestore());
	}

	BM::GUID const& GetTargetPilotGUID()	const		{ return	m_TargetPilotGUID;	}
	void	SetTargetPilotGUID(BM::GUID const& kGUID)	{ m_TargetPilotGUID = kGUID;	}

	BYTE	GetSphereIndex()	const			{	return	m_bySphereIndex;	}
	void	SetSphereIndex(BYTE const byIndex)	{	m_bySphereIndex = byIndex;	}

	const	NiPoint3&	GetTargetPos()	const	{	return	m_kTargetPos;	}

	int	 GetRemainHP() const				{ return m_iRemainHP; }
	void SetRemainHP(int const iRemainHP)	{ m_iRemainHP = iRemainHP; }

	PgActionResult&	GetActionResult() { return m_kActionResult; }
	void			SetActionResult(PgActionResult const& kActionResult) { m_kActionResult = kActionResult;	}

};

typedef	std::list<PgActionTargetInfo> ActionTargetList;

class	PgActionTargetList
{
private :
	ActionTargetList		m_kActionTargetList;

	//!	액션 이펙트가 적용되었는가?
	int	m_bActionEffectApplied;

public:

	//!	Caster GUID
	BM::GUID	m_kCasterGUID;
	int			m_iActionInstanceID;
	DWORD		m_dwTimeStamp;
	int			m_iActionNo;

public:

	PgActionTargetList() : 
		m_iActionInstanceID(-1), 
		m_iActionNo(-1), 
		m_bActionEffectApplied(false)
	{}

	~PgActionTargetList() {}

	void	SetActionInfo(BM::GUID const& kCasterGUID, int const iActionInstanceID, int const iActionNo, DWORD const dwTimeStamp)
	{
		m_kCasterGUID = kCasterGUID;
		m_iActionInstanceID = iActionInstanceID;
		m_iActionNo = iActionNo;
		SetTimeStamp(dwTimeStamp);
	}
	void	SetTimeStamp(DWORD const dwTimeStamp)
	{
		m_dwTimeStamp = dwTimeStamp;
	}
	DWORD	GetTimeStamp() const {	return	m_dwTimeStamp;	}

	int	const					size() const	{	return	m_kActionTargetList.size();	}
	ActionTargetList::iterator	begin()			{	return	m_kActionTargetList.begin();	}
	ActionTargetList::iterator	end()			{	return	m_kActionTargetList.end();	}
	void						clear()			{	m_kActionTargetList.clear();	}

	ActionTargetList&	GetList()	{	return	m_kActionTargetList;	}

	PgActionTargetInfo*	GetTargetByGUID(BM::GUID const& kGUID);
	BM::GUID const& GetCasterGUID()	const {	return	m_kCasterGUID;	}

	int		GetActionNo() const			{	return	m_iActionNo;	}
	int		GetActionInstanceID() const	{	return	m_iActionInstanceID;	}

	bool	CopyActionResultTo(PgActionTargetList &kTarget);	//	이펙트만 복사한다.
	void	SetEffectReal(bool const bReal);

	bool	IsActionEffectApplied()	const { return m_bActionEffectApplied != 0;	}
	void	SetActionEffectApplied(bool const bApplied)	{	m_bActionEffectApplied = (int)bApplied;	}
	void	ApplyActionEffects(bool const bOnlyDieEffect = false, bool const bNoShowDamageNum = false, float const fRandomPosRange = 0.0f, bool bReleaseAction = false);	//	bOnlyDieEffect : true : 다른 이펙트는 패스하고 죽는 이펙트만 적용한다.
	void	ApplyOnlyDamage(int const iDivide = 1, bool const bApplyEffects = false, float const fRandomPosRange = 0.0f);
	void	ApplyActionEffects(BM::GUID const& kTargetGUID);
	void	ApplyElementalDmgEffect(CUnit* pkCaster, lwActor kTargetActor, PgActionResult& rkActionResult);
private:
	void	ApplyDieEffectToStillLoadingPilot(BM::GUID const& kTargetGUID,PgActionResult* pkActionResult);
	void	ApplyDieEffectFromNotExistingPilot(PgActionResult* pkActionResult);

	void	ShowActionResultText(PgActionResult* pkActionResult, PgActor* pkCaster, PgActor* pkTarget);

	void	ShowDamageNum(PgActor* pkCaster,PgActor* pkTarget,int iDamage, float fRandomPosRange = 0, bool bCritical = false, PgActionResult* pkActionResult = NULL);
	void	ShowReflectDamageNum(PgActor* pkCaster,PgActor* pkTarget,int iReflectDamage,PgActionResult* pkActionResult);
};

struct	stActionTargetTransferInfo
{
	BM::GUID	m_kOriginalActorGUID;
	int	m_iOriginalActionNo;
	int	m_iOrigianlActionInstanceID;

	BM::GUID	m_kTransferredActorGUID;
	int	m_iTransferredProjectileUID;

	stActionTargetTransferInfo() :
		m_iOriginalActionNo(0),
		m_iOrigianlActionInstanceID(0),
		m_iTransferredProjectileUID(-1)
	{}

	stActionTargetTransferInfo(BM::GUID const& kOriginalActor, int const iOriginalActionNo,int const iOriginalActionInstanceID, BM::GUID const& kTransferredActorGUID) :
		m_kOriginalActorGUID(kOriginalActor),
		m_iOriginalActionNo(iOriginalActionNo),
		m_iOrigianlActionInstanceID(iOriginalActionInstanceID),
		m_kTransferredActorGUID(kTransferredActorGUID),
		m_iTransferredProjectileUID(-1)
	{}

	stActionTargetTransferInfo(BM::GUID const& kOriginalActor, int const iOriginalActionNo,int const iOriginalActionInstanceID, int const iProjectileUID) :
		m_kOriginalActorGUID(kOriginalActor),
		m_iOriginalActionNo(iOriginalActionNo),
		m_iOrigianlActionInstanceID(iOriginalActionInstanceID),
		m_iTransferredProjectileUID(iProjectileUID)
	{}
};

class	PgActionTargetTransferInfoMan
{
public:

	typedef	std::list<stActionTargetTransferInfo> ActionTargetListTransferInfoCont;

private:

	ActionTargetListTransferInfoCont	m_Cont;

public:

	PgActionTargetTransferInfoMan()		{	Create();	}
	~PgActionTargetTransferInfoMan()	{	Destroy();	}

	void	Create();
	void	Destroy();

	void	AddTransferInfo(BM::GUID const& kOriginalActor,
		int const iOriginalActionNo,int const iOriginalActionInstanceID,
		BM::GUID const& kTransferredActorGUID);
	void	AddTransferInfo(BM::GUID const& kOriginalActor,
		int const iOriginalActionNo,int const iOriginalActionInstanceID,
		int const iProjectileUID);

	void	DeleteTransferInfo(BM::GUID const& kOriginalActor,
		int const iOriginalActionNo,int const iOriginalActionInstanceID,
		BM::GUID const& kTransferredActorGUID);

	void	DeleteTransferInfo(BM::GUID const& kOriginalActor,
		int const iOriginalActionNo,int const iOriginalActionInstanceID,
		int const iProjectileUID);

	void	ModifyTargetList(PgActionTargetList &kTargetList);

};

extern	PgActionTargetTransferInfoMan	g_kActionTargetTransferInfoMan;

namespace PgActionTargetListUtil
{
	extern bool CheckDoDamageAction(PgPilot* pkPilot, PgActionResult* pkActionResult);
}

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONTARGETLIST_H