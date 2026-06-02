#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORPET_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORPET_H
#include "PgActor.h"

typedef struct tagPetActionEle
{
	float	fEndTime;
	bool	bIsLoop;
	int		iSlot;
	tagPetActionEle()				{Clear();}
	void Clear()					{fEndTime=0.0f; bIsLoop=false; iSlot=0;}
	bool IsEnd(float fAccumTime) const
	{
		return fEndTime<fAccumTime;
	}

}SPetActionEle;
typedef std::queue<SPetActionEle> PetActionQueue;

class PgActorPet : public PgActor
{
public:
	static PgInventory *ms_pkMyPetInventory;// 내 펫의 인벤토리(펫이 죽어 있을때만 셋팅되어 있다)
	static PgInventory *ms_pkTempPetInventory;// 펫 장착 패킷이 늦게와서 내 팻을 찾지 못했을 경우 임시 저장

public:
	PgActorPet();
	~PgActorPet();

	virtual bool Update(float fAccumTime, float fFrameTime);
	//virtual void UpdatePhysX(float fAccumTime, float fFrameTime);
	virtual	PgIWorldObject*	CreateCopy()	{	return	NULL;	}
	//virtual bool ProcessFollowingActor();
	//virtual void AddActionEntity(PgActionEntity& kActionEntity);
	virtual PgAction* CreateActionForTransitAction(char const *pcNextActionName, bool bEnable = true, NiPoint3 *pkActionStartPos = NULL, BYTE byDirection = DIR_NONE, int iActionNo = 0);
	virtual float TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate);
	void DoChangeColor();
	bool MakePetActionQueue(int iGrade = -1);	//-1이면 새로 얻어보자
	int UpdatePetActionQueue(float fAccumTime);

	virtual void CopyEquipItem(PgActorPet* pkPet);
	virtual bool EquipItem(PgItemEx *pkEquipItem, int iItemNo, bool bSetToDefault);		// 아이템을 즉시 장착
	virtual bool BeforeUse();
	virtual bool BeforeCleanUp();
	virtual	bool CreateCopyEx(PgActor *pkNewActor);
	void SetPetType(EPetType eType)	{m_ePetType = eType;}
	EPetType GetPetType() const		{return m_ePetType;}

	bool	SetMPBarValue( int const iBefore,int const iNew );
	PgActor* GetMasterActor(void);
	virtual NiNode* GetNodePointStar(void); //펫에 탑승했을땐 이름 노드가 달라진다.

	typedef enum ePetAttackState
	{
		E_ATK_IDLE,
		E_ATK_ATTACK,
		E_ATK_MACRO,	//매크로 공격 중
	}E_PET_ATK_STATE;

	E_PET_ATK_STATE GetAttackState() const
	{
		return m_kAttackState;
	}
	void SetAttackState(E_PET_ATK_STATE kValue)
	{
		m_kAttackState = kValue;
	}

	void ClearSkillContForAI()	{m_kSkillCont.clear();}
	CONT_SLOT const& GetSkillContForAI() const	{return m_kSkillCont;}
	void AddSkillToContForAI(int const iSkill)	
	{
		if(0<iSkill)
		{
			m_kSkillCont.push_back(iSkill);
		}
	}

protected:
	CLASS_DECLARATION(bool, m_bIsNoTrace, NoTrace);

	CONT_SLOT m_kSkillCont;

protected:
    virtual bool IsDrawable( PgRenderer* pkRenderer, NiCamera* pkCamera );

protected:
	virtual	void DoLoadingFinishWork();	//	로딩이 완료되었을때 처리
	void DoChangeItemTexture(int const iNo);
	int m_iLastOwnerActionNo;										//주인의 이전 액션
	//bool IsFollowingAction(PgActionEntity& rkActionEntity);	//따라 해야 되는 액션인가
	void ClearPetActionQueue();

	PetActionQueue	m_kPetActionQueue;
	mutable Loki::Mutex m_kPetMutex;	//펫 액션큐 동기화
	EPetType m_ePetType;	//저장해 놓자
	void MakePetType();
	E_PET_ATK_STATE m_kAttackState;
};

namespace PgActorPetAIUtil
{
	CUnit* FindEnemy(PgActor* pkActorPet);
	int UpdateDelay(float const fFrameTime, CUnit* pkUnit);
	int GetReservableSkill_Random(PgActorPet* pkActorPet, CUnit* pkUnit);
	void ChaseEnemy(PgActor* pkActor, CUnit* pkUnit, CSkillDef const *pkSkillDef = NULL);
	bool GetAroundVector(PgActor* pkActorPet, BM::GUID const& rkTargetGuid, float const fRange, bool const bNear, NiPoint3 &rkOut);
	void FireSkill(PgActor* pkActorPet, CUnit* pkUnit);
	bool SetValidGoalPos(PgActor* pkActorPet, CUnit* pkUnit, NiPoint3 const& rkEndPos, float const fHeight = AI_Z_LIMIT, bool bSetGoalPos = true);
	void SkillChooser(PgActor* pkActorPet, CUnit* pkUnit);
	int GetMinDistanceFromTarget(CUnit* pkUnit);
	bool IsCallerAlive(CUnit* pkUnit);
	void ChangeFSM_Idle(CUnit* pkUnit);
	bool IsCanUpdate(float fAccumTime, float fFrameTime, PgActorPet* pkActorPet, CUnit* pkUnit);
}

NiSmartPointer(PgActorPet);
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORPET_H