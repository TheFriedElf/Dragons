#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PGACTORSUBPLAYER_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PGACTORSUBPLAYER_H
#include "PgActorPet.h"

class PgActorSubPlayer
	: public PgActor
{
public:
	public:
	PgActorSubPlayer();
	virtual ~PgActorSubPlayer();

	virtual float TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate);
	
	bool MakePetActionQueue(int iGrade = -1);	//-1이면 새로 얻어보자
	int UpdatePetActionQueue(float fAccumTime);
	void ClearPetActionQueue();

protected:

	PetActionQueue	m_kPetActionQueue;
	mutable Loki::Mutex m_kPetMutex;	//펫 액션큐 동기화

	CLASS_DECLARATION(bool, m_bIsNoTrace, NoTrace);

protected:
	//virtual	void DoLoadingFinishWork();	//	로딩이 완료되었을때 처리
	//void DoChangeItemTexture(int const iNo);
	//int m_iLastOwnerActionNo;										//주인의 이전 액션
	////bool IsFollowingAction(PgActionEntity& rkActionEntity);	//따라 해야 되는 액션인가
	//void ClearPetActionQueue();

	//PetActionQueue	m_kPetActionQueue;
	//mutable Loki::Mutex m_kPetMutex;	//펫 액션큐 동기화
	//EPetType m_ePetType;	//저장해 놓자
	//void MakePetType();
	//E_PET_ATK_STATE m_kAttackState;
};

namespace PgActorSubPlayerUtil
{

}

NiSmartPointer(PgActorSubPlayer);

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PGACTORSUBPLAYER_H