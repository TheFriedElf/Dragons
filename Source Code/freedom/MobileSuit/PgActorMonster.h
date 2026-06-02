#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMONSTER_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMONSTER_H

#include "Variant/PgMonster.h"

NiSmartPointer(PgElementMark);

class PgActorMonster : public PgActor
{
	NiDeclareRTTI;

public:
	PgActorMonster();

	void SetMonsterType(EMonsterType eType) { m_eMonsterType = eType; }
	EMonsterType GetMonsterType() { return m_eMonsterType; }
	void SetLookTarget(bool bLook);

	virtual	PgIWorldObject*	CreateCopy();


	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void UpdatePhysX(float fAccumTime, float fFrameTime);
	virtual void ConcilDirection(NiPoint3 &rkLookingDir, bool const bRightAway = false);
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection = false,bool bForceToTransit = false);

	virtual bool UpdateName(std::wstring const& rkName = L"");

	void ResetActiveGrp();
	
	//!	몬스터가 인지한 공격목표를 설정한다.
	void SetAttackTarget(BM::GUID const &kTargetGUID);

	//! ActionQueue에 있는 Action을 처리함.
	virtual bool ProcessActionQueue();

	//! Sync를 시작.
	virtual bool BeginSync(PgAction *pkAction, DWORD dwOverTime);

	//! Sync를 수행.
	virtual bool UpdateSync(float fFrameTime);

	//! 공중형을 위해 Z값을 얼마나 올려야 되는지 확인.
	float AdjustToFly(NiPoint3 const &kCurPos, float fFrameTime);

	bool DoSpeech(EUnitState const eUnitState);

	//! PhysX를 초기화한다.
	virtual	void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);

	virtual bool BeforeCleanUp();

private:
	void SetMovingDeltaZCheck();	

	PgElementMarkPtr m_spElementMark;

protected:
	virtual void	DrawNameText(PgRenderer *pkRenderer,NiCamera *pkCamera);
	EMonsterType m_eMonsterType;
	bool m_bLookFocusTarget;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMONSTER_H