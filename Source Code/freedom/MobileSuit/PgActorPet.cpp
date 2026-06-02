#include "StdAfx.h"
#include "PgActorPet.h"
#include "lwActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
//#include "PgMobileSuit.h"
#include "PgStat.h"
#include "PgInterpolator.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "PgEnergyGuage.H"
#include "PgCircleShadow.H"
PgInventory* PgActorPet::ms_pkMyPetInventory = NULL;
PgInventory* PgActorPet::ms_pkTempPetInventory = NULL;

const	char	*ACTIONNAME_TRACE = "a_trace_ground";
const	char	*ACTIONNAME_TRACE2 = "a_trace_ground2";
const	char	*ACTIONNAME_RUN_PET = "a_run_pet";
const	char	*ACTIONNAME_RIDING = "a_riding_ground";
int const DEFAULT_DETECT_RANGE = 200;

namespace PgPetUIUtil
{
	bool PetSkillToSkillTree(PgPlayer* pkPlayer);
	XUI::CXUI_Wnd* SetPetUIDefaultState(bool bEnableCover);
}

PgActorPet::PgActorPet(void) : m_iLastOwnerActionNo(0), m_bIsNoTrace(false), m_ePetType(EPET_TYPE_1), m_kAttackState(E_ATK_IDLE)
{	
	m_kSkillCont.reserve(MAX_PET_SKILLCOUNT);
}

PgActorPet::~PgActorPet(void)
{
}

bool PgActorPet::Update(float fAccumTime, float fFrameTime)
{
	PgActor::Update(fAccumTime, fFrameTime);
	if(!GetPilot() || !GetPilot()->GetUnit() || GetPilot()->GetUnit()->IsDead()) {return false;}	//유닛 없거나 죽었으면 돌지 말자
	
	if(false==PgActorPetAIUtil::IsCanUpdate(fAccumTime, fFrameTime, this, GetPilot()->GetUnit()))
	{
		return true;
	}

	EUnitState const eState = GetPilot()->GetUnit()->GetState();
	
	switch(eState)
	{
	case US_CHASE_ENEMY:
		{
			PgActorPetAIUtil::ChaseEnemy(this, GetPilot()->GetUnit());
		}break;	//선택한 스킬 거리만큼 타겟 앞으로 가자
	case US_SKILL_FIRE:
		{
			PgActorPetAIUtil::FireSkill(this, GetPilot()->GetUnit());
		}break;	//Chase에서 정한 위치까지 갔으면 공격하자
	case US_NONE:
	case US_IDLE:
		{
			if(E_ATK_MACRO==GetAttackState())	{break;}	//임시로 막자
			PgActorPetAIUtil::SkillChooser(this, GetPilot()->GetUnit());
		}break;	//공격이 끝이면 다시 a_trace_ground2로 바꾸던지 혹은 주인 근처로 가자
	}

	return true;
}

PgAction* PgActorPet::CreateActionForTransitAction(char const* pcNextActionName, bool bEnable, NiPoint3* pkActionStartPos, BYTE byDirection, int iActionNo)
{
	if( !m_bIsNoTrace && (0==strcmp(pcNextActionName, "a_idle") || 0==strcmp(pcNextActionName, "a_run")) )
	{
		pcNextActionName = ACTIONNAME_TRACE;
	}

	PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode(iActionNo);
	if(pkNode)
	{
		const CSkillDef* pkDef = pkNode->GetSkillDef();
		if(pkDef)
		{
			int const iLevel = pkDef->GetAbil(AT_LEVEL);
			iActionNo += iLevel - 1;
		}
	}

	return PgActor::CreateActionForTransitAction(pcNextActionName, bEnable, pkActionStartPos, byDirection, iActionNo);
}

float PgActorPet::TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate)
{
	//return 0.0f;
	bool bTracing = false;
	NiPoint3 const &rkCurPos = GetPos();
	NiPoint3 kTempTargetLoc = m_kTargetLoc;
	/*if(stFollowInfo::FS_MOVE_TO_STARTPOS==m_kFollowInfo.m_kFollowState)
	{
		if(m_kFollowInfo.m_kSavePoint != NiPoint3::ZERO)
		{
			kTempTargetLoc = m_kFollowInfo.m_kSavePoint;
		}
		fLimitDistance = 0.0f;
		fAccelateScale = 5.0f;
	}*/
	NiPoint3 kTempCurLoc = rkCurPos;
	kTempTargetLoc.z = 0;
	kTempCurLoc.z = 0;
	NiPoint3 kDiffPos = kTempTargetLoc - kTempCurLoc;
	float fDistance = kDiffPos.Length();
	kDiffPos.Unitize();
	NiPoint3 kNextPos = GetTranslate();
	SetFreeMove(false);
	bool bZDist = 130 < fabs(rkCurPos.z - m_kTargetLoc.z);
	
	bool bIsFloor = true;
	int iActionNo = 0;
	bool bIsRiding = false;
	PgPilot* pkPilot = GetPilot();
	if(pkPilot)
	{
		PgActor* pkActor = g_kPilotMan.FindActor(pkPilot->GetUnit()->Caller());
		if(pkActor)
		{
			bIsFloor = pkActor->IsMeetFloor();
			if(pkActor->GetAction())
			{
				iActionNo = pkActor->GetAction()->GetActionNo();
			}
			bIsRiding = pkActor->IsOnRidingObject();
		}
	}
	
	bool bTelePort = false;
	if(150<fDistance /*&& stFollowInfo::FS_MOVE_TO_STARTPOS!=m_kFollowInfo.m_kFollowState*/ || bZDist)
	{
		if(!bIsFloor)	{return 0.0f;}
		
		if(bIsFloor && !bIsRiding)	//주인이 바닥에 붙어 잇을 때만 펑하자
		{
			kNextPos = m_kTargetLoc;
			bTelePort = true;
			NiAVObject *pkParticle = g_kParticleMan.GetParticle("ef_show_01_p_ef_heart",PgParticle::O_SCALE, 1.0f );
			if(pkParticle)
			{
				if(!AttachTo(1010, "p_ef_heart", pkParticle))
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
			}
		}
	}
	
	if( fLimitDistance < fDistance && !bZDist && !bTelePort)
	{
		// 열심히 쫒아가자
		kNextPos = rkCurPos + (kDiffPos * fSpeed * fFrameTime) + (kDiffPos * (fDistance-fLimitDistance) * fAccelateScale * fFrameTime);

		NiPoint3 const kTempDown = GetWorld()->ThrowRay(NiPoint3(kNextPos.x, kNextPos.y, kNextPos.z+50.0f), NiPoint3(0.0f,0.0f,-1.0f), 200.0f);
		NiPoint3 const kTempPos = GetWorld()->ThrowRay(NiPoint3(kNextPos.x, kNextPos.y, m_kTargetLoc.z+10), NiPoint3(0.0f,0.0f,-1.0f), 1000.0f);	// 주인이 어느정도 높이에 떠 있나?
		//if(-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z)	//바닥이 허공
		if((-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z) || 95.0f < rkCurPos.z - kTempPos.z)
		{
			NiPoint3 kFrontDir = kDiffPos;
			kFrontDir.Unitize();
			NiPoint3 const kTempFront = GetWorld()->ThrowRay(GetLastFloorPos(), kFrontDir, 40.0f);	//앞으로 레이
			if(-1==kTempFront.x && -1==kTempFront.y && -1==kTempFront.z)
			{
				SetTraceFlyTargetLoc(rkCurPos);//멈추자 일단
				fDistance = 0.0f;
				return fDistance;
			}
		}
		else
		{
			kNextPos.z = kTempPos.z + 25.0f;
		}

		if(-1==kTempPos.x && -1==kTempPos.y && -1==kTempPos.z)	//바닥이 허공
		{
			if(!bIsFloor)
			{
				if(100005440 == iActionNo)	//텔레점프
				{
					SetFreeMove(true);//이놈은 진짜 대점프를 탄 상태일 때만 해 주자
					kNextPos.z = m_kTargetLoc.z;
				}
				else//바닥이 허공. 난 바닥. 주인은 허공
				{
					SetTraceFlyTargetLoc(rkCurPos);//멈추자 일단
					kNextPos = GetTranslate();
					fDistance = 0.0f;
				}
			}
		}
		else if(bIsFloor)
		{
			bTracing = true;
		}
		kNextPos.z = kTempPos.z + 25.0f;
	}

	if(GetTranslate() != kNextPos)
	{
		m_pkController->setPosition( NxExtendedVec3(kNextPos.x, kNextPos.y, kNextPos.z) );
		SetTranslate(kNextPos);
		SetWorldTranslate(kNextPos);

		if( bCanRotate )
		{
			SetMovingDir(kDiffPos);
			ConcilDirection(kDiffPos, true);
		}
		SetPositionChanged(true);
	}

	/*if(stFollowInfo::FS_MOVE_TO_STARTPOS==m_kFollowInfo.m_kFollowState)
	{
		if(1.0f>=fDistance)
		{
			m_kFollowInfo.m_kFollowState=stFollowInfo::FS_PROCESS_ACTION_QUEUE;
			m_kFollowInfo.m_kSavePoint = NiPoint3::ZERO;
		}
		else
		{
			fDistance = 61.0f;
		}
	}*/




	return fDistance;
}

void PgActorPet::DoChangeColor()
{
	PgPilot* pkPilot = GetPilot();
	if(!pkPilot)	{return;}

	bool b1stType = EPET_TYPE_1==m_ePetType;//1차펫이냐 2차 펫이냐
	int iColor = pkPilot->GetAbil(AT_COLOR_INDEX);
	bool bIs3drPhase = true;//1<pkPilot->GetAbil(AT_CLASS)%10;	//2차펫 3단계이냐?//인제 무조건 3차다
	if(0>=iColor)	
	{
		if(b1stType || !bIs3drPhase || m_VarTextureList.empty())	{return;}
		VariTextureContainer::const_iterator tex_it = m_VarTextureList.begin();
		if(m_VarTextureList.end() == tex_it)	{return;}
		std::string const &rkPath = (*tex_it).second;
		size_t kPos = rkPath.find(".dds");//확장자 앞의 두글자를 찾자
		if(2>kPos)	{return;}
		iColor = atoi(&rkPath.at(kPos-2));
		pkPilot->SetAbil(AT_COLOR_INDEX, iColor);
	}	//0은 무염색
	
	if(!b1stType && bIs3drPhase)
	{
		DoChangeItemTexture(iColor);
		return;
	}

	if(m_VarTextureList.empty())	{return;}

	BM::vstring kIndex;
	if(10>iColor)
	{
		kIndex+=L"0";
	}
	kIndex+=iColor;

	RestoreTexture();

	VariTextureContainer::iterator tex_it = m_VarTextureList.begin();
	while( m_VarTextureList.end() != tex_it )
	{
		std::string const &rkKey = (*tex_it).first;
		std::string &rkPath = (*tex_it).second;

		size_t kPos = rkPath.find(".dds");
		if(2<kPos)	//두글자가 바뀔거라 적어도 2보단 커야지
		{
			rkPath.replace(kPos-2, 2, MB(kIndex));
		}

		++tex_it;
	}

	ChangeTexture(this);
}

void PgActorPet::CopyEquipItem(PgActorPet* pkPet)
{
	if( !pkPet )
	{
		return;
	}

	PgPilot* pkPilot = pkPet->GetPilot();
	if( !pkPilot )
	{
		return;
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	if( !pkUnit )
	{
		return;
	}

	PgInventory *pkInven = pkUnit->GetInven();
	if( !pkInven )
	{
		return;
	}

	GET_DEF(PgClassPetDefMgr, kClassDefMgr);
	PgClassPetDef kPetDef;
	bool const bDef = kClassDefMgr.GetDef( SClassKey(pkUnit->GetAbil(AT_CLASS), pkUnit->GetAbil(AT_LEVEL)), &kPetDef );

	GET_DEF(CItemDefMgr, kItemDefMgr);
	for( int iCur = 0; EQUIP_POS_MAX > iCur ; ++iCur )
	{
		PgBase_Item kItem;

		SItemPos const kPetItemPos(IT_FIT, iCur);
		if( S_OK == pkInven->GetItem(kPetItemPos, kItem) && false == kItem.IsUseTimeOut() )	//&& !kOption.IsHideCashInvenPos(static_cast< EEquipPos >(iCur)) ) && !kOption.IsHideEquipInvenPos(static_cast< EEquipPos >(iCur)) )
		{//! 캐쉬 아이템 보임
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( pkItemDef )
			{
				eEquipLimit const eLimit = static_cast< eEquipLimit >(pkItemDef->GetAbil(AT_EQUIP_LIMIT));
				AddToDefaultItem(eLimit, kItem.ItemNo());
			}
		}
		else
		{
			if ( true == bDef )
			{
				eEquipLimit const kEquipLimit = static_cast<eEquipLimit>(0x00000001 << iCur);

				switch( kEquipLimit )
				{
				case EQUIP_LIMIT_PET_HEAD:{ AddToDefaultItem(kEquipLimit, kPetDef.GetDefaultHair()); }break;
				case EQUIP_LIMIT_PET_BODY:{ AddToDefaultItem(kEquipLimit, kPetDef.GetDefaultBody()); }break;
				case EQUIP_LIMIT_FACE:{ AddToDefaultItem(kEquipLimit, kPetDef.GetDefaultFace()); }break;
				default:
					{
						if( DelDefaultItem(kEquipLimit) )
						{
							EInvType const eType = IT_FIT;
							EEquipPos const ePos = static_cast< EEquipPos >(iCur);
							UnequipItem(eType, ePos, 0, PgItemEx::LOAD_TYPE_INSTANT);
						}
					}continue;
				}
			}
		}
	}

	EquipAllItem();
}

void PgActorPet::DoChangeItemTexture(int const iNo)
{
	PgItemEx::ApplyTextureChange(iNo, GetNIFRoot());
}

void PgActorPet::DoLoadingFinishWork()
{
	if(!m_iEquipCount && !m_bLoadingComplete)
	{
		DoChangeColor();
	}
	PgActor::DoLoadingFinishWork();
}

void PgActorPet::ClearPetActionQueue()
{
	BM::CAutoMutex kLock(m_kPetMutex);
	size_t kSize = m_kPetActionQueue.size();
	for(size_t i = 0; i < kSize; ++i)
	{
		m_kPetActionQueue.pop();
	}
}

bool PgActorPet::MakePetActionQueue(int iGrade)
{
	ClearPetActionQueue();
	if(!GetPilot() || !g_pkWorld)	{return false;}
	PgPet* pkPet = dynamic_cast<PgPet*>(GetPilot()->GetUnit());
	if(!pkPet)						{return false;}
	if(EPET_TYPE_1==m_ePetType)	//1차펫
	{return false;}

	int iIdleSlot = 0;
	if( 3 > iGrade )
	{
		iIdleSlot = 0;
	}
	else if( 4 > iGrade )
	{
		iIdleSlot = 1;
	}
	else
	{
		iIdleSlot = 2;
	}

	float fAccum = g_pkWorld->GetAccumTime();
	ClearPetActionQueue();
	BM::CAutoMutex kLock(m_kPetMutex);
	for(int i = 0; i < 6; ++i)
	{
		fAccum+=10.0f;
		SPetActionEle kEle;
		kEle.fEndTime = fAccum;
		kEle.bIsLoop = false;	//아이들
		kEle.iSlot = iIdleSlot;
// 		if(0==i%2)
// 		{
// 			for(int k = 0; k < 3; ++k)	//어빌 3종류
// 			{
// 				int iTemp = iAbil + k;
// 				if(AT_MENTAL<iTemp)
// 				{
// 					iTemp = AT_HUNGER;
// 				}
// 				if(3000>pkPet->GetAbil(iTemp))
// 				{
// 					kEle.iSlot = 3+iTemp-AT_HUNGER;//기본+k가 되야 함
// 					kEle.bIsLoop = true;	//아이들
// 					break;	
// 				}
// 			}
// 			++iAbil;
// 		}

		m_kPetActionQueue.push(kEle);
	}

	return false==m_kPetActionQueue.empty();
}

int PgActorPet::UpdatePetActionQueue(float fAccumTime)
{
	if(EPET_TYPE_1==m_ePetType)	{return 0;}
	BM::CAutoMutex kLock(m_kPetMutex);
	if(m_kPetActionQueue.empty())	
	{
		if(MakePetActionQueue())
		{
			return m_kPetActionQueue.front().iSlot;
		}
		return 0;
	}
	SPetActionEle const& rkEle = m_kPetActionQueue.front();
	int iSlot = rkEle.iSlot;
	if(rkEle.IsEnd(fAccumTime))
	{
		bool bAniDone = IsAnimationDone();
		if(rkEle.bIsLoop || bAniDone)
		{
			m_kPetActionQueue.pop();
			return UpdatePetActionQueue(fAccumTime);
		}
	}
	return iSlot;
}

bool PgActorPet::BeforeUse()
{
	MakePetType();
	if(IsMyPet())
	{
		PgPetUIUtil::SetPetUIDefaultState(false);
		PgPetUIUtil::PetSkillToSkillTree(g_kPilotMan.GetPlayerUnit());
	}
/*	if(GetPilot()->GetUnit()->GetAbil(815) && m_pkMountedRidingPet == NULL)
	{
		PgActor* pkActor = g_kPilotMan.FindActor(GetPilot()->GetUnit()->Caller());
		if(pkActor)
		{
			pkActor->MountPet();
		}
	}
*/
	return PgActor::BeforeUse();
}

bool PgActorPet::BeforeCleanUp()
{ //주인,펫 중 소멸 순서를 알수 없으므로 먼저 소멸되는 놈이 Unmount 해준다.
	if(m_pkMountedRidingPet && m_pkMountedRidingPet->IsRidingPet())
	{
		m_pkMountedRidingPet->UnmountPet();
	}

	return PgActor::BeforeCleanUp();
}

PgActor* PgActorPet::GetMasterActor(void)
{
	PgPilot* pkPilot = GetPilot();
	if(!pkPilot)
	{
		return NULL;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return NULL;
	}
	const BM::GUID kCallerID = pkUnit->Caller();
	if(kCallerID.IsNull())
	{
		return NULL;
	}

	return g_kPilotMan.FindActor(kCallerID);
}

NiNode* PgActorPet::GetNodePointStar(void)
{
	PgPilot* pkPilot = GetPilot();
	NiNode* pkNodeRet = NULL;
	if(!pkPilot)
	{
		return NULL;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return NULL;
	}

	if(!pkUnit->IsUnitType(UT_PET))
	{
		return NULL;
	}

	NiAVObject* pkRoot = GetNIFRoot();
	if(!pkRoot)
	{
		return NULL;
	}

	if(IsRidingPet())
	{ //탑승시 탑승 전용 노드 사용
		pkNodeRet = NiDynamicCast(NiNode, pkRoot->GetObjectByName(ATTACH_POINT_RIDENAME));
	}
	else
	{ //비탑승시 디폴트 노드 사용
		pkNodeRet = NiDynamicCast(NiNode, pkRoot->GetObjectByName(ATTACH_POINT_STAR));
	}
	return pkNodeRet;
}

bool PgActorPet::CreateCopyEx(PgActor *pkNewActor)
{
	bool bRet = PgActor::CreateCopyEx(pkNewActor);
	PgActorPet* pkActorPet = dynamic_cast<PgActorPet*>(pkNewActor);
	if(pkActorPet)
	{
		pkActorPet->m_ePetType = m_ePetType;
	}
	return bRet;
}

void PgActorPet::MakePetType()
{
	if(GetPilot())
	{
		PgPet const* pkPet = dynamic_cast<PgPet const*>(GetPilot()->GetUnit());
		if(pkPet)
		{
			m_ePetType = static_cast<EPetType>(pkPet->GetPetType());
		}
	}
}

bool PgActorPet::SetMPBarValue( int const iBefore,int const iNew )
{
	if ( !m_pHPGaugeBar )
	{		
		return false;
	}
	
	int const iMaxMP = GetPilot()->GetAbil(AT_C_MAX_MP);
	if(iBefore == iNew && m_pHPGaugeBar->MaxValue() == iMaxMP)
	{		
		return false;	
	}

	_PgOutputDebugString("[PgActor::SetMPBarValue] Actor:%s iMaxMP:%d Before MP : %d New MP: %d\n",MB(GetPilotGuid().str()),iMaxMP,iBefore,iNew);

	m_pHPGaugeBar->SetBarValue ( iMaxMP, iBefore,iNew );	

	return true;
}

bool PgActorPet::EquipItem(PgItemEx *pkEquipItem, int iItemNo, bool bSetToDefault)
{
	bool bRet = __super::EquipItem(pkEquipItem, iItemNo, bSetToDefault);
	DoChangeColor();
	return bRet;
}

bool PgActorPet::IsDrawable( PgRenderer* pkRenderer, NiCamera* pkCamera )
{
	PgPilot* pkPilot = GetPilot();
	if(pkPilot == NULL) { return false; }
	CUnit* pkUnit = GetUnit();
	if(pkUnit == NULL) { return false; }

	// 펫인 경우 펫의 주인의 상태에 따라서 숨기고 보이고가 결정 된다.
	PgActor* pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
	if(NULL == pkCallerActor) { return false; }
	PgPilot* pkCallerPilot = pkCallerActor->GetPilot();
	if(NULL == pkCallerPilot) { return false; }
	if(pkCallerPilot->GetGuid().IsNull() == true || pkCallerPilot->GetName().empty() == true)
	{ //펫 주인의 GUID나 이름이 비어있다면 정상적으로 로딩이 완료되지 않았을 것이다.
		return false;
	}

	if(false == pkCallerActor->IsCompleteLoadParts()) //내 주인액터의 데이터 로딩이 끝나지 않았으면 실행하지 않는다.
	{
		return false;
	}
	if(pkCallerPilot->IsHide())
	{
		return false;
	}
	if (pkCallerActor->GetInvisibleGrade() >= PgActor::INVISIBLE_FAR && false == pkCallerActor->GetIgnoreCameraCulling())
	{
		return false;
	}
	// This는 펫이고 펫의 주인과 나와의 관계를 비교한다.
	const bool bThisIsEnemyActor = pkCallerActor->IsEnemy( g_kPilotMan.GetPlayerActor() );
	if ( pkCallerActor->IsHide() || pkCallerActor->IsBlinkHide() ) // 숨거나 블링크 숨기 / !(적 & 숨었을 경우)
	{
		if( !bThisIsEnemyActor )
			PgCircleShadow::AddOnlyShadowToVisibleArrayRecursive(this,pkRenderer,pkCamera);	//	그림자만 그리기
		return false;
	}

	if ( pkCallerActor->IsInvisible() && bThisIsEnemyActor )
		return false;

    return true;
}


bool PgActorPetAIUtil::IsCallerAlive(CUnit* pkUnit)
{
	if(NULL==pkUnit)				{return false;}
	if(pkUnit->Caller().IsNull())	{return false;}
	
	PgPilot *pkPilot = g_kPilotMan.FindPilot(pkUnit->Caller());
	if(pkPilot && pkPilot->GetUnit())
	{
		return pkPilot->GetUnit()->IsAlive();
	}
	return false;
}

CUnit* PgActorPetAIUtil::FindEnemy(PgActor* pkActorPet)
{
	if(NULL==pkActorPet)	{return NULL;}
	
	PgAction* pkAction = pkActorPet->GetAction();
	if(NULL==pkAction)	{return NULL;}
	
	if(0==strcmp(pkAction->GetID().c_str(), ACTIONNAME_TRACE2))
	{
		if(strcmp(pkAction->GetParam(0), "STOP"))
		{
			return NULL;	//멈춰있는 상태가 아니면
		}
	}

	PgPilot* pkPilot = pkActorPet->GetPilot();
	if(NULL==pkPilot)	{return NULL;}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(NULL==pkUnit)	{return NULL;}
	PgActor* pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
	if(NULL==pkCallerActor)	{return NULL;}
	int iRange = pkUnit->GetAbil(AT_DETECT_RANGE);
	if (0>=iRange)
	{
		iRange = DEFAULT_DETECT_RANGE;
	}

	std::map<float, CUnit*>	SortedTargetArr;
	
	CUnit* pkMonster = NULL;
	UNIT_PTR_ARRAY kUnitArray;
	if(g_kPilotMan.FindUnitInRange(UT_MONSTER, pkCallerActor->GetPos(), iRange, kUnitArray)) //주인 중심
	{
		for(UNIT_PTR_ARRAY::iterator itor = kUnitArray.begin(); itor != kUnitArray.end(); ++itor)
		{
			pkMonster = (*itor).pkUnit;
			if(pkMonster && pkMonster->IsAlive())
			{
				PgActor* pkActorMon = g_kPilotMan.FindActor(pkMonster->GetID());
				if(pkActorMon && true==pkActorMon->GetCanHit() && false==pkActorMon->IsBlowUp())
				{
					SortedTargetArr.insert(std::make_pair((pkActorPet->GetPos()-pkActorMon->GetPos()).SqrLength(), pkMonster));
					//return pkMonster;	
				}
			}
		}

		if(false==SortedTargetArr.empty())
		{
			return (*SortedTargetArr.begin()).second;
		}
	}
	/*
	if(g_kPilotMan.FindPVPTargetInRange(pkUnit->GetID(), iRange, kUnitArray)) //펫 중심 PVP 타겟 찾기
	{
		for(UNIT_PTR_ARRAY::iterator itor = kUnitArray.begin(); itor != kUnitArray.end(); ++itor)
		{
			pkMonster = (*itor).pkUnit;
			if(pkMonster && pkMonster->IsAlive())
			{
				PgActor* pkActorMon = g_kPilotMan.FindActor(pkMonster->GetID());
				if(pkActorMon)
				{
					SortedTargetArr.insert(std::make_pair((pkActorPet->GetPos()-pkActorMon->GetPos()).SqrLength(), pkMonster));
				}
			}
		}
	}

	if(false==SortedTargetArr.empty())
	{
		return (*SortedTargetArr.begin()).second;
	}
	*/
	return NULL;
}

int PgActorPetAIUtil::GetReservableSkill_Random(PgActorPet* pkActorPet, CUnit* pkUnit)
{
	if(NULL==pkActorPet || NULL==pkUnit)	{return 0;}
	int iCount = 0;

	PgActor::CONT_SLOT const& rkSkillCont = pkActorPet->GetSkillContForAI();
	size_t const kCount = rkSkillCont.size();
	if(0>=kCount)		{return 0;}
	int iSkillNo = 0;
	PgActor::stSkillCoolTimeInfo const *pkInfo = pkActorPet->GetSkillCoolTimeInfo();
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	while (iCount++ < MAX_PET_SKILLCOUNT)
	{
		iSkillNo = rkSkillCont[BM::Rand_Index(kCount)];	//0은 기본 스킬로 하자
		
		if(iSkillNo)
		{
			CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
			if(NULL==pkSkillDef)											{iSkillNo = 0; continue;}
			if(EST_ACTIVE!=pkSkillDef->GetType())							{iSkillNo = 0; continue;}
			//if(ESTARGET_ENEMY!=pkSkillDef->GetTargetType())				{iSkillNo = 0; continue;}	//이건 생각 좀 해봐야 할 듯
			if(pkSkillDef->GetAbil(AT_NEED_MP) > pkUnit->GetAbil(AT_MP))	{iSkillNo = 0; continue;}	//마나 부족

			//PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = pkInfo->m_CoolTimeInfoMap.find(g_kSkillTree.GetKeySkillNo(iSkillNo));
			//if(itor==pkInfo->m_CoolTimeInfoMap.end())
			bool bIsGobalCoolTime = false;
			if(false==pkActorPet->IsInCoolTime(iSkillNo, bIsGobalCoolTime))
			{
				break;	//쿨타임 없는 놈을 고르자
			}
			else
			{
				iSkillNo = 0;
			}
		}
	}

	return iSkillNo;
}

int PgActorPetAIUtil::UpdateDelay(float const fFrameTime, CUnit* pkUnit)
{
	if(NULL==pkUnit)	{return 0;}
	
	int const iDelay = pkUnit->GetDelay();
	int const iNewDelay = std::max(static_cast<int>(iDelay - fFrameTime*1000.0f), 0);
	pkUnit->SetDelay(iNewDelay);
	return iNewDelay;
}

void PgActorPetAIUtil::ChaseEnemy(PgActor* pkActor, CUnit* pkUnit, CSkillDef const *pkSkillDef)
{
	if(!pkActor || !pkUnit || !pkUnit->GetSkill())	{return;}
	int const iSkillNo = pkUnit->GetSkill()->GetSkillNo();
	if(NULL==pkSkillDef)
	{
		pkSkillDef = pkUnit->GetSkill()->GetSkillDef();
		if(NULL==pkSkillDef)	//그래도 널이면
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
			if(NULL==pkSkillDef)	//그래도 널이면
			{
				return;
			}
		}
	}

	POINT3 ptGoalPos = pkUnit->GoalPos();
	NiPoint3 const& rkNowPos = pkActor->GetPos();

	bool bTransitWalkAction = false;

	if( ptGoalPos == POINT3::NullData() )
	{
		if(ESTARGET_CASTER & pkSkillDef->GetTargetType() || ESTARGET_SELF & pkSkillDef->GetTargetType())	//캐스터가 목표면 그냥 현재 위치에서 쓰자. 대신 스킬 범위를 엄청 넓게
		{
			ptGoalPos.x = rkNowPos.x;
			ptGoalPos.y = rkNowPos.y;
			ptGoalPos.z = rkNowPos.z;
			pkUnit->GoalPos(ptGoalPos);
		}
		else
		{
			NiPoint3 rkVec;
			int const iMin = GetMinDistanceFromTarget(pkUnit);
			if(GetAroundVector(pkActor, pkUnit->GetTarget(), iMin, true, rkVec))
			{
				bTransitWalkAction = SetValidGoalPos(pkActor, pkUnit, rkVec);	//목표까지 움직이도록 해야 함. 액션 만들까?
				if(false==bTransitWalkAction)
				{
					if(GetAroundVector(pkActor, pkUnit->GetTarget(), iMin, false, rkVec))//Near위치가 에러면 Far위치로 다시 검사 하자
					{
						bTransitWalkAction = SetValidGoalPos(pkActor, pkUnit, rkVec);
						if(false==bTransitWalkAction)
						{
							PgActorPetAIUtil::ChangeFSM_Idle(pkUnit);

							PgActor* pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
							if(pkCallerActor)	//주인이 있다면 주인 중심
							{
								pkActor->SetTraceFlyTargetLoc( pkCallerActor->GetPos() );
							}
							if(pkActor->GetAction() && pkActor->GetAction()->GetID()!=ACTIONNAME_TRACE2)
							{
								pkActor->ReserveTransitAction(ACTIONNAME_TRACE2);
							}
							return;
						}
					}
				}
			}
		}
		ptGoalPos = pkUnit->GoalPos();
	}	

	NiPoint3 kDist = rkNowPos - NiPoint3(ptGoalPos.x, ptGoalPos.y, ptGoalPos.z);
	if(kDist.SqrLength() < AI_GOALPOS_ARRIVE_DISTANCE_Q)
	{//목표 도착
		pkUnit->GoalPos(POINT3::NullData());
		pkUnit->SetState(US_SKILL_FIRE);
		return;
	}

	PgActor* pkTargetActor = g_kPilotMan.FindActor(pkUnit->GetTarget());

	bool bClearState = false;

	if(NULL!=pkTargetActor)
	{
		if(pkActor == pkTargetActor)
		{
			bClearState = true;
		}
		else
		{
			int iRange = pkUnit->GetAbil(AT_DETECT_RANGE);
			if (0>=iRange)
			{
				iRange = DEFAULT_DETECT_RANGE;
			}

			NiPoint3 kCenter = pkActor->GetPos();
			PgActor* pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
			if(pkCallerActor)	//주인이 있다면 주인 중심
			{
				kCenter = pkCallerActor->GetPos();
			}

			float fDist = (kCenter - pkTargetActor->GetPos()).Length();
			bClearState = (fDist > iRange);
		}
	}

	if(bClearState || NULL==pkTargetActor)
	{
		PgActorPetAIUtil::ChangeFSM_Idle(pkUnit);
		if(pkActor->GetAction() && pkActor->GetAction()->GetID()!=ACTIONNAME_TRACE2)
		{
			pkActor->ReserveTransitAction(ACTIONNAME_TRACE2);
		}
		return;
	}

	if(bTransitWalkAction || US_CHASE_ENEMY == pkUnit->GetState())	//움직여야 함
	{
		PgAction* pkAction = pkActor->ReserveTransitAction(ACTIONNAME_RUN_PET);
		if(pkAction)
		{
			pkAction->SetNextActionName(MB(std::wstring(pkSkillDef->GetActionName())));
			pkAction->SetParamAsPoint(0, NiPoint3(pkUnit->GoalPos().x, pkUnit->GoalPos().y, pkUnit->GoalPos().z));
		}
	}
}

bool PgActorPetAIUtil::GetAroundVector(PgActor* pkActorPet, BM::GUID const& rkTargetGuid, float const fRange, bool const bNear, NiPoint3 &rkOut)
{
	if(NULL==pkActorPet)	{return false;}
	PgActor* pkTargetActor = g_kPilotMan.FindActor(rkTargetGuid);
	if(NULL==pkTargetActor)	{return false;}

	NiPoint3 ptUnitPos = pkActorPet->GetPos();
	NiPoint3 ptTargetPos = pkTargetActor->GetPos();

	NiPoint3 kPath = pkTargetActor->GetPathNormal();

	kPath.Unitize();

	NiPoint3 kRightVec = kPath.Cross(NiPoint3(0.0f, 0.0f, 1.0f));

	NiPoint3 kFirst = ptTargetPos + kRightVec * fRange;
	NiPoint3 kSecond = ptTargetPos - kRightVec * fRange;

	NiPoint3 kNearTarget;
	NiPoint3 kFarTarget;

	if((kFirst - ptUnitPos).SqrLength() > (kSecond - ptUnitPos).SqrLength())
	{
		kFarTarget = kFirst;
		kNearTarget = kSecond;
	}
	else
	{
		kFarTarget = kSecond;
		kNearTarget = kFirst;
	}

	rkOut = bNear?kNearTarget : kFarTarget;
	return true;
}

void PgActorPetAIUtil::FireSkill(PgActor* pkActorPet, CUnit* pkUnit)
{
	if(NULL==pkActorPet || NULL==pkUnit)	{return;}
	PgActor* pkTargetActor = g_kPilotMan.FindActor(pkUnit->GetTarget());
	if(pkTargetActor)
	{
		if(pkUnit->GetSkill() && pkUnit->GetSkill()->GetSkillNo())
		{
			int const iSkillNo = pkUnit->GetSkill()->GetSkillNo();
			
			CSkillDef const *pkSkillDef = pkUnit->GetSkill()->GetSkillDef();
			if(NULL==pkSkillDef)															{return;}

			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(iSkillNo);
			if(1<kActionScriptName.size())
			{
				PgAction* pkAction = pkActorPet->ReserveTransitAction(MB(kActionScriptName));
				if(pkAction)
				{
					pkActorPet->LookAt(pkTargetActor->GetPos(), true);
					pkAction->SetNextActionName(ACTIONNAME_TRACE2);
					pkUnit->GetSkill()->Reserve(iSkillNo);

					static int const siDefaultAnimTime = 550;
					int iAnimTime = pkSkillDef->GetAbil(AT_ANIMATION_TIME);
					if(iAnimTime == 0)
					{
						iAnimTime = siDefaultAnimTime;
					}
					iAnimTime+=(pkSkillDef->GetAbil(AT_DEFAULT_ATTACK_DELAY) + pkSkillDef->GetAbil(AT_GLOBAL_COOLTIME));	//공격 딜레이+글로벌 쿨타임
					pkUnit->SetDelay(iAnimTime+100);	//클라에서는 딜레이를 안쓰니까 이걸로 자동 시전을 컨트롤 하자	//딜레이가 약간 있어야 함
				}
			}
		}
	}
	PgActorPetAIUtil::ChangeFSM_Idle(pkUnit);
}

bool PgActorPetAIUtil::SetValidGoalPos(PgActor* pkActorPet, CUnit* pkUnit, NiPoint3 const& rkEndPos, float const fHeight, bool bSetGoalPos)
{
	if(NULL==pkActorPet || NULL==pkUnit){return false;}
	//목표지점 바닥체크

	NiPoint3 const& rkNowPos = pkActorPet->GetPos();
	
	if(rkNowPos==rkEndPos)
	{
		if(bSetGoalPos)	
		{
			pkUnit->GoalPos(POINT3(rkEndPos.x, rkEndPos.y, rkEndPos.z));	
		}
		return true;
	}

	if(NULL==pkActorPet->GetWorld())	{return false;}

	static float const fRayLength = 170.0f;		// 아래로 쏘는 거리 (얼마가 적당할지 => 단차는 약 100)
	NiPoint3 kHit = pkActorPet->GetWorld()->ThrowRay(NiPoint3(rkEndPos.x, rkEndPos.y, rkEndPos.z+20), NiPoint3(0.0f, 0.0f, -1.0f), fRayLength);

	if(-1 == kHit.x && -1 == kHit.y && -1 == kHit.z)
	{
		if(bSetGoalPos)	
		{
			pkUnit->GoalPos(POINT3(0,0,0));
		}
		return false;
	}

	float const fGroundZ = kHit.z;
	NiPoint3 kRayDir = rkEndPos - rkNowPos;
	float fRayDirLength = kRayDir.Length();
	float fTangentRay = kRayDir.z / fRayDirLength;	// tan(direction)
	if (abs(fTangentRay) > 0.5f || abs(kRayDir.z) > AI_Z_LIMIT)	// 기울기와 Z축 높이차로 이동할 수 있는지 결정하자.
	{
		if(bSetGoalPos)	
		{
			pkUnit->GoalPos(POINT3(0,0,0));
		}
		return false;
	}
	kRayDir.Unitize();

	kHit = pkActorPet->GetWorld()->ThrowRay(NiPoint3(rkNowPos.x, rkNowPos.y, rkNowPos.z+fHeight), kRayDir, fRayDirLength + 5.0f);	//중간에 걸리는게 없다면
	if(-1 == kHit.x && -1 == kHit.y && -1 == kHit.z)
	{
		if(bSetGoalPos)	
		{
			pkUnit->GoalPos(POINT3(rkEndPos.x, rkEndPos.y, fGroundZ+25.0f));	//Ray를 쏘면 25 차이가 나므로
		}
		return true;
	}

	if(bSetGoalPos)	
	{
		pkUnit->GoalPos(POINT3(0,0,0));
	}

	return false;
}

void PgActorPetAIUtil::SkillChooser(PgActor* pkActorPet, CUnit* pkUnit)
{
	if(NULL==pkActorPet || NULL==pkUnit)												{return;}
	CUnit* pkTarget = PgActorPetAIUtil::FindEnemy(pkActorPet);
	if(NULL!=pkTarget)
	{
		int const iSkillNo = PgActorPetAIUtil::GetReservableSkill_Random(dynamic_cast<PgActorPet*>(pkActorPet), pkUnit);
		if(iSkillNo && pkTarget)
		{
			CSkill* pkSkill = pkUnit->GetSkill();
			if(NULL==pkSkill)																{return;}

			pkSkill->Reserve(iSkillNo);

			pkUnit->SetState(US_CHASE_ENEMY);//적을 찾았으니 쫒아가야 된다
			if(ESTARGET_CASTER & pkSkill->GetSkillDef()->GetTargetType())
			{
				pkUnit->SetTarget(pkUnit->Caller());
			}
			else if(ESTARGET_SELF & pkSkill->GetSkillDef()->GetTargetType())
			{
				pkUnit->SetTarget(pkUnit->GetID());
			}
			else
			{
				pkUnit->SetTarget(pkTarget->GetID());
			}
		}
	}
	else
	{
		pkUnit->SetTarget(BM::GUID::NullData());
		PgAction* pkCurAction = pkActorPet->GetAction();
		
		if(pkCurAction && pkCurAction->GetID()!=ACTIONNAME_TRACE2)	//2가 2차펫
		{
			pkActorPet->ReserveTransitAction(ACTIONNAME_TRACE2);
		}
	}
}

int PgActorPetAIUtil::GetMinDistanceFromTarget(CUnit* pkUnit)
{
	int iMinDistance = AI_MONSTER_MIN_DISTANCE_FROM_TARGET;
	if (pkUnit)
	{
		CSkill* pkSkill = pkUnit->GetSkill();
		if (pkSkill)
		{
			int iAbil = pkSkill->GetAbil(AT_MON_MIN_RANGE);
			if (0<iAbil)
			{
				iMinDistance = iAbil;
			}
			else
			{
				iAbil = pkSkill->GetAbil(AT_SKILL_MIN_RANGE);
				if (0<iAbil)
				{
					iMinDistance = iAbil;
				}
				else
				{
					iAbil = pkUnit->GetAbil(AT_MON_MIN_RANGE);
					if (0<iAbil)
					{
						iMinDistance = iAbil;
					}
				}
			}

			if(iMinDistance == AI_MONSTER_MIN_DISTANCE_FROM_TARGET)
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(pkSkill->GetSkillNo());
				if(pkSkillDef->m_sRange > AI_MONSTER_MIN_DISTANCE_FROM_TARGET)
				{
					short sSkillRange = pkSkillDef->m_sRange - AI_MONSTER_MIN_DISTANCE_FROM_TARGET;
					iMinDistance = sSkillRange > AI_MONSTER_MIN_DISTANCE_FROM_TARGET ? sSkillRange : AI_MONSTER_MIN_DISTANCE_FROM_TARGET;
				}
			}
		}
	}
	return iMinDistance;
}

void PgActorPetAIUtil::ChangeFSM_Idle(CUnit* pkUnit)
{
	if (pkUnit)
	{
		pkUnit->GoalPos(POINT3::NullData());
		pkUnit->SetState(US_IDLE);
		if(pkUnit->GetSkill())
		{
			pkUnit->GetSkill()->Reserve(0);
		}
	}
}

bool PgActorPetAIUtil::IsCanUpdate(float fAccumTime, float fFrameTime, PgActorPet* pkActorPet, CUnit* pkUnit)
{
	if(NULL==pkActorPet)													{return false;}
	if(false==pkActorPet->GetCallerIsMe())									{return false;}	//내 펫 아니면 돌지 말자
	if(pkActorPet->GetPetType()==EPET_TYPE_1)								{return false;}	//1차펫은 돌지 말자
	if(pkActorPet->GetPetType()==EPET_TYPE_3 && pkActorPet->IsRidingPet()) {return false;}
	//내 펫이 탑승용이고 현재 탑승 중이라면 돌지 말자
	
	EUnitState const eState = pkUnit->GetState();
	int const iDelay = PgActorPetAIUtil::UpdateDelay(fFrameTime, pkUnit);
	if(0<iDelay)															{return false;}	//아직 딜레이가 남아 있으면 돌지 말자

	if(false==PgActorPetAIUtil::IsCallerAlive(pkUnit))						{return false;}	//PC가 죽었다

	return (0!=pkUnit->GetAbil(AT_AUTO_PET_SKILL));//pkActorPet->GetAttackState()!=PgActorPet::E_ATK_IDLE;
}