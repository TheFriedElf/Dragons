#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWACTOR_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWACTOR_H

#include "lwGUID.h"
#include "lwPoint3.h"
#include "lwAction.h"
#include "lwAttackEffect.h"
#include "PgSkillTree.H"

class lwPilot;
class PgActor;
class lwSkillTargetMan;
class lwQuaternion;
class	lwTrigger;
class lwWString;

class lwActor
{
public:
	lwActor(PgActor *pkActor);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! 캐릭터를 왼쪽, 오른쪽으로 걷게 한다.
	bool Walk(int iDir, float fSpeed, float fFrameTime,bool bCorrectFinalPos);
	//!	빠르게 동기화 지점으로 이동한다.
	void	StartSyncMove( lwPoint3 kTargetPos, char const *pcActionName=NULL );
	bool	UpdateSyncMove(float fSpeed, float fFrameTime);
	//! 캐릭터의 속도를 0으로 한다. (Z는 Keep)
	void Stop();

	float TraceFly(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, float fLimitZ, float fFloatHeight, bool bCanRotate);
	float TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate);
	void SetTraceFlyTargetLoc(lwPoint3 kTargetLoc);
	lwPoint3 GetTraceFlyTargetLoc();
	
	bool IsOnRidingObject();
	void ConcilDirection(lwPoint3 kLookingDir, bool bRightAway = false);

	//! 뛴다
	void StartJump(float fHeight);

	//! 그만 뛴다.
	void StopJump();

	//! 캐릭터의 Scale 을 변화시킨다.
	void	SetTargetScale(float fScale,unsigned long ulTotalScaleChangeTime);

	//! 캐릭터를 민다.
	void	PushActor(bool bLeft,float fDistance,float fVelocity,float fAccel);
	void	PushActorDir(lwPoint3 kDir,float fDistance,float fVelocity,float fAccel);
	bool	GetNowPush();	//	현재 밀리는 중인가?

	//! 점프 중인지 체크
	bool IsJumping();
	
	//! 점프한 높이를  반환한다.
	float GetJumpAccumHeight();

	//!	점프한 시간 
	float	GetJumpTime();

	//! 점프한 높이를 리셋
	void ResetJumpAccumHeight();

	//! 캐릭터에 힘을 준다.
	void AddForce(lwPoint3 vForce);

	//! 캐릭터에 가속도를 준다.
	void AddVelocity(lwPoint3 kVel);

	//!	애니메이션 리셋
	void	ResetAnimation();

	bool	IsOnlyMoveAction();

	bool	IsUnitType(int iUnitType);

	//!	현재 트리거 반환
	lwTrigger	GetCurrentTrigger();

	//! 움직이는 물체에 관한 캐릭터 이동.
	bool ApplyMovingObject_OnEnter(lwTrigger kTrigger);
	bool ApplyMovingObject_OnUpdate(lwTrigger kTrigger);
	bool ApplyMovingObject_OnLeave(lwTrigger kTrigger);

	//! 캐릭터를 돌린다(Set Local Orientation)
	bool SetRotation(float fDegree, lwPoint3 kAxis);

	//! 이 캐릭터를 카메라 포거스 캐릭터로 만든다.
	void	SetCameraFocus();

	//! 노드를 숨긴다.
	void SetNodeHide(char const* strNodeName, bool bHide);
	//! 노드에 투명도를 설정한다.
	void SetNodeAlpha(char const* strNodeName, float fAlpha);
	void SetNodeAlphaChange(char const* strNodeName, float fAlphaStart, float fAlphaEnd, float fChangeTime);
	//! 특정 Nif 노드의 월드 좌표를 리턴한다.
	lwPoint3	GetNodeWorldPos(char const* strNodeName);
	lwPoint3	GetParticleNodeWorldPos(int iSlot, char *strNodeName);

	lwQuaternion	GetNodeWorldRotate(char const* strNodeName);
	bool SetRotationQuat(lwQuaternion kQuat);

	//! 변신 하기 전의 원래 액터를 설정한다.
	void	SetOriginalActor(lwActor Actor);
	lwActor	GetOriginalActor();

	//!	무기 궤적 그리기 시작
	void	StartWeaponTrail();
	//!	무기	궤적	그리기	종료
	void	EndWeaponTrail();
	//!	몸 궤적 그리기 시작
	void	StartBodyTrail(char const* strTexPath, int iTotalTime, int iBrightTime);
	//!	몸	궤적	그리기	종료
	void	EndBodyTrail();
	bool	StartTrail(int const iTrailType, char const* strTexPath, int iTotalTime, int iBrightTime);
	bool	EndTrail(int const iTrailType);

	//! 월드 좌표
	lwPoint3	GetPos();

	//! 이동방향을 반환해 준다.
	lwPoint3 GetMovingDir();
	void SetMovingDir(lwPoint3 kDir);
	void	SetMovingDelta(lwPoint3 kDelta);
	lwPoint3	GetMovingDelta();

	//! 재생중인 애니메이션이 끝났는지 알려준다.
	bool IsAnimationDone();

	//! 재생중인 애니메이션이 루프인지 알려준다.
	bool IsAnimationLoop() const;

	//! 지정한 애니메이션을 플레이 한다.
	void SetTargetAnimation(char const *pcAnimName);

	//!	애니메이션 플레이 시간을 리턴한다.
	float	GetAnimationLength(char const *rkAnimationName);

	//! 애니메이션 정보를 얻어온다.
	char const* GetAnimationInfo(char const* infoName,int iSeqID);

	char const* GetAnimationInfoFromAction(char const* infoName, lwAction pkNowAction, int iSeqID);

	lwWString GetAnimationInfoFromAniName(char const* szInfoName, char const* szAniName);

	//! 현재 재생중인 애니메이션에 찾고자 하는 텍스트키가 있는지 검사
	bool HaveAnimationTextKey(char const* szKey, char const* szAnimationName) const;

	//! Normal 공격의 종료 시간을 저장한다
	void	SetNormalAttackEndTime();

	//! 현재 시각이 연타 가능 입력 시간 구간내인지 체크한다.
	bool CanNowConnectToNextComboAttack(float fMaxTime);

	//! 이 액터가 내가 조종하는 액터인가?
	bool IsMyActor();

	//! 이 액터가 내가 조종하는 서브액터인가?
	bool IsMySubPlayer();

	//! 이 액터가 내 펫인가?
	bool IsMyPet();

	//! 이 액터가 펫인가?
	bool IsPet();

	//!	액터의 아이템 모델을 임시로 교체한다.
	void	ChangeItemModel(int iItemPos,char const *pkNewItemXMLPath);
	//!	액터의 무기 모델을 원래 것으로 돌려놓는다.
	void	RestoreItemModel(int iItemPos);

	//! 변신한다.
	void Transformation(char const *pcNewModel, char const *pcNextAction);
	bool IsTransformed();

	//! 원래대로 돌아옴
	void RestoreTransformation(char const *pcNextAction);

	//! UpdateScript를 설정한다.
	void SetUpdateScript(char const *pcUpdateScript);

	bool	IsTopLevelActionEffect(lwAction kAction);

	//!	줄 수 있는 퀘스트 정보 업데이트
	//void	ClearQuestSimpleInfoPool();
	//void	AddQuestSimpleInfo(short sQuestID,char byState);
	//void	PopSavedQuestSimpleInfo();	//	퀘스트 매니저가 저장하고 있는 퀘스트 정보를 가져온다.

	//!	진행중인 퀘스트 정보 업데이트
	//void	UpdateQuestUserInfo(short sQuestID,BYTE byQuestState,unsigned long ulParamArrayAddr);

	bool	HaveQuest();
	bool	HaveDailyQuest();
	bool	HaveActivate();
	bool	HaveTalk();
	bool	HaveWarning();

	void	StartSkillCoolTime(unsigned	long	ulSkillNo);
	void	CutSkillCoolTime(unsigned	long	ulSkillNo);
	void	SkillCastingConfirmed(unsigned	long	ulSkillNo,short sErrorCode);
	void	CutSkillCasting();
	
	bool GetSkillTreeNode(int const iQuickSlotIndex, PgSkillTree::stTreeNode *&pkNode);

	int	GetRemainSkillCoolTimeInQuickSlot(int iQuickSlotIndex);
	float	GetRemainSkillCoolTimeInQuickSlotFloat(int iQuickSlotIndex);
	float	GetSkillTotalCoolTimeInQuickSlot(int iQuickSlotIndex);

	int	GetCastingSkillNo();

	//! 장비하고 있는 무기의 타입번호를 리턴한다.
	int	GetEquippedWeaponType();

	//! S 버튼에 설정되는 액션 ID
	void SetNormalAttackActionID(char const* actionid);
	char const* GetNormalAttackActionID();

	//! 지정된 애니를 플레이 한다.
	bool ActivateAnimation();

	lwAction	ReserveTransitAction(char const *kNextAction, BYTE byDir);
	lwAction	ReserveTransitActionIgnoreCase(char const *kNextAction, BYTE byDir);
	lwAction	ReserveTransitActionByActionNo(int iActionNo, BYTE byDir);

	lwAction	GetReservedTransitAction();
	void		ClearReservedAction();

	lwGUID		CreateTempActionByActionNo(int iActionNo);
	lwAction	GetTempAction(lwGUID kActionGuid);
	void		RemoveTempAction(lwGUID kActionGuid);

	//!	현재 액션을 중단 시키고 pcNextActionName 으로 전이시킨다.
	void	CancelAction(int iActionID,int iActionInstanceID,char const *pcNextActionName);

	//! 지정한 Action을 해제한다.
	bool UntransitAction(char const *pcNextActionName);

	void	SetEventScriptIDOnDie(int iEventID);
	int		GetEventScriptIDOnDie();
	
	//! 파티클이 붙었는지 체크
	bool IsAttachParticleSlot(int const iSlot)const;

	//! 지정한 파티클을 붙인다.
	bool AttachParticle(int iSlot, char const *pcTarget, char const *pcParticle );
	bool AttachParticleS( int iSlot, char const *pcTarget, char const *pcParticle, float const fScale );
	//! 파티클을 붙이는데 회전은 적용시키지 않는다.1
	bool AttachParticleWithRotate(int iSlot, char const *pcTarget, const char *pcParticle, float fScale = 1.0f, bool NoFollowParentRotation = false );
	void ChangeParticleTexture(int const iSlot, char const* pcParticle, char const* TexNode, int const GeoNo, char const* pcTexture);


	//! 지정한 파티클을 지정한 위치에 붙인다.(월드 좌표계, 따라 움직이지 않음)
	bool AttachParticleToPoint(int iSlot, lwPoint3 kPoint, char const *pcParticle );
	bool AttachParticleToPointS( int iSlot, lwPoint3 kPoint, char const *pcParticle, float const fScale );

	//! 파티클을 지정한 위치에 돌려서 붙인다.
	bool AttachParticleToPointWithRotate(int iSlot, lwPoint3 kPoint, const char *pcParticle, lwQuaternion kQuat, float fScale);
	
	//! 파티클의 애니타입을 바꾸며 처음부터 플레이한다.
	bool lwActor::ParticleStartAnimation(int iSlot, const char *pcParticle);

	//! 지정한 파티클을 그림자 위치에 붙인다.(월드 좌표계, 따라 움직이지 않음)
	bool AttachParticleToShadow(char const *pcParticle);
	
	//! 카메라 앞에 파티클을 붙인다
	bool AttachParticleToCameraFront(int iSlot, lwPoint3 kPos, const char *pcParticle, float fScale);

	//! 지정한 파티클을 떼어 낸다. (확 떼어 낸다.)
	bool DetachFrom(int iSlot, bool bDefaultThreadDelete = false);

	//! 지정한 파티클을 더이상 나오지 않게 한다.
	bool ChangeParticleGeneration(int iSlot, bool bGenerate);

	//! 파티클의 Alpha Group 을 변경한다.
	void SetParticleAlphaGroup(int iSlot,int iAlphaGroup);

	//! 모든 파티클을 다 때어낸다.
	void ReleaseAllParticles();

	//!	몬스터가 인지한 공격목표를 설정한다.
	//void	SetAttackTarget(lwGUID kTargetGUID);

	//!	일정 시간동안 액터를 흔든다.
	void	SetShakeInPeriod(float fShakePower,int iPeriod);

	//! 현재 진행중인 액션을 가져 온다.
	lwAction GetAction();

	//! 현재 플레이어의 속도를 가져온다.
	lwPoint3 GetVelocity();

	//! 캐릭터 진행 방향에, 인자로 넘어온 방향이 포함되는지 알려준다.
	bool ContainsDirection(int iDir);

	//! 캐릭터가 진행 방향을 알려준다.
	int GetDirection();

	//! 캐릭터의 방향을 설정한다.
	void SetDirection(int iDirection);

	//! 다음 슬롯의 애니를 플레이 한다.
	bool PlayNext();
	
	//! 이전 슬롯의 애니를 플레이 한다.
	bool PlayPrev();

	//! 현재 슬롯의 애니를 플레이 한다.
	bool PlayCurrentSlot(bool bNoRandom);

	//! 앞 또는 뒤를 보게 한다.
	void SeeFront(bool bFront, bool bRightAway);

	void SeeLadder();

	//! 주체가 GameBryo인지 AgeiaPhysX인지 세팅
	void InitPhysical(bool bIsPhysical);

	//! 주체가 바뀜으로써.. 주체가 아닌것이 Update가 안되는것을 강제로 해버릴 수 있다.
	//! PhysX관련. Source 를 업데이트 한다.
	void UpdatePhysicalSrc(float fTime, bool bForce = false);

	//! PhysX관련. Destination 를 업데이트 한다.
	void UpdatePhysicalDest(float fTime, bool bForce = false);

	//! null 인지를 반환한다.
	bool IsNil();

	//! Actor를 조정하는 Pilot의 GUID를 반환한다.
	lwGUID GetPilotGuid();

	lwPilot	GetPilot();

	//! Actor를 조정하는 Pilot의 Guid를 설정한다.
	void SetPilotGuid(lwGUID kGuid);

	//! Path의 노멀 백터를 갱신한다.
	void FindPathNormal(bool const bDoNotConcil = false);

	//!	패스 노멀을 리턴한다.
	lwPoint3	GetPathNormal();

	//! 현재 위치를 반환
	lwPoint3 GetTranslate();

	//! 바닥 위치를 반환
	lwPoint3 GetFloorLoc();

	//! 마지막으로 바닥을 밟았던 위치를 반환한다.
	lwPoint3 GetLastFloorPos();

	bool	HasTarget();

	void	ActionToggleStateChange(int iActionNo,bool bOn);

	//! 위치 변경
	void SetTranslate(lwPoint3 loc,bool bDontUseController);

	void SetWorldTranslate(lwPoint3 loc);

	// 인자로 받는 Actor와 같은 행렬로 셋팅
	void SetClonePos(lwActor kActor);

	//! 대미지 숫자를 띄운다.
	void	ShowDamageNum(lwPoint3 attackerloc,lwPoint3 kTargetPos,int iDamage);
	//!	스킬 텍스트를 띄운다.
	void ShowSkillText(lwPoint3 kTargetPos,int iTextType,bool bUp);
	//!	간단 텍스트를 띄운다.
	void ShowSimpleText(lwPoint3 kTargetPos,int iTextType);
	//! 액터를 깜박거리게 한다.
	void Blink(bool bBlink, int iBlinkFreq);

	//! 현재 사용자가 입력한 마지막 방향키를 가져온다.
	bool GetLastKey();

	//! 현재 사용자가 입력한 마지막 방향키를 눌렀는지 뗐는지 가져온다.
	bool GetLastKeyPressed();

	//! 애니메이션 스피드를 조절
	void SetAnimSpeed(float fSpeed);
	//! 애니메이션 스피드 얻기
	float GetAnimSpeed();

	//!	일정 시간동안 애니메이션 스피드를 조정한다.
	void	SetAnimSpeedInPeriod(float fAnimSpeed,int iPeriod);
	void	SetAnimOriginalSpeed();

	void	SetUnderMyControl(bool bUnderMyControl);
	bool	IsUnderMyControl();

	bool	IsSendBlowStatus();
	void	SetSendBlowStatus(bool bSend, bool bFirstDown = false, bool bNoUseStandUpTime = false);

	bool	CheckStatusEffectExist(char const* strStatusEffectXMLID);
	bool	CheckStatusEffectTypeExist(BYTE byType);
	bool	CheckEffectExist(int const iEffectNo, bool const bInGroup);
	int		CheckSkillExist(int const iSkillNo);

	//!	다운상태로 만든다.
	void	SetDownState(bool bDownState);
	bool	IsDownState();

	void	SetTotalDownTime(float fTime);
	float	GetTotalDownTime();

	//! 액터를 움직인다.
	void MoveActor(float fX, float fY, float fZ);

	//! 캐릭터의 자유로운 이동
	void FreeMove(bool bFreeMove);

	//! 때릴 수 있는가?
	void SetCanHit(bool bCanHit);

	void StartGodTime(float fTotalGodTime);

	bool IsGodTime();

	//! 매달릴 수 있는가?
	bool HangItOn();

	//! 줄을 탈 수 있는가?
	bool HangOnRope();

	//! 사다리를 탈 수 있는가?
	bool ClimbUpLadder();

	//! Concil을 하지 않는다.
	void Concil(bool bConcil);

	//!	이녀석을 때릴수 있는가?
	bool	GetCanHit();
	
	//! 물리를 동기화할 것인지, 게임브리오에 동기화할 것인지 결정
//	void SwitchPhysical(bool bGoPhsyical, float fAccumTime, float fFrameTime);

	short	GetComboCount();
	void	SetComboCount(short sCount);

	void	StartNormalAttackFreeze();
	void	StopNormalAttackFreeze();
	int	GetNormalAttackFreezeElapsedTime();

	char*	GetEquippedWeaponProjectileID();

	//! 미리 정해진 말풍선 목록중에서 하나를 띄운다.
	bool	OnClickSay();
	//! 말풍선을 띄운다.
	void	ShowChatBalloon(char const *Text);
	//! 쿨타임 시간 설정
	void	SetTotalCoolTime(unsigned long ulCoolTime);
	unsigned	long	GetRemainCoolTime();

	lwPoint3 GetNodeTranslate(char const *pcNodeName);
	void	SetNodeScale(char const *pcNodeName,float fScale);
	lwPoint3 GetNodeRotateAxis(char *strNodeName, int iAxis = 0, bool bWorld = false);

	lwSkillTargetMan	GetSkillTargetMan();

	PgActor *lwActor::operator()();

	bool IsToLeft();

	void ToLeft(bool bLeft,bool bTurnRightAway);

	int GetAbil(int iAbilType);
	void SetAbil(int iAbilType, int iValue);

	lwPoint3	GetABVShapeWorldPos(int iIndex);

	char const *GetID();

	void PhysXSync();
	void ReleasePhysX();
	void ReleaseABVShapes();

	void SetHide(bool const bHide);
	void SetHideBalloon(bool const bHide);
	bool IsHide()const;
	void SetHideShadow(bool const bHide);

	void SetColor(float fR, float fG, float fB);

	bool IsMeetFloor();

	bool IsCheckMeetFloor();

	bool IsMeetSide();

	bool IsSlide();

	void SetUseSmoothShow(bool bSmoothShow);

	void SetMeetFloor(bool bFloor);

	bool AttachSound(int iSlot, char const *pcSndID, float fVolume, float fDistMin = 80, float fDistMax = 100);
	bool AttachSoundToPoint(int iSlot, char const *pcSndID, lwPoint3 kPos, float fVolume, float fDistMin = 80, float fDistMax = 100);

	void BackMoving(bool bBack);

	void SetGravity(float fGravity);

	void UseSkipUpdateWhenNotVisible(bool bUse);

	void SetActiveGrp(int iGroup, bool bTrue);
	
	void ResetActiveGrp();

	void SetForceSync(bool bSync);

	void Throw();

	void	SetThrowStart();
	lwPoint3	CheckCollWithCamera();	

	void EquipAllItem();

	unsigned int GetActionState(char const *pcActionID);

	void ClearActionState();

	void ClearDirectionSlot(bool bReset);

	// 슬롯에 지정된 사운드를 플레이한다.
	bool PlaySlotSound(char const *pcSlotName);

	void SetPickupScript(char const *pcScript);

	void SetMouseOverScript(char const *pcScript);
	
	void SetMouseOutScript(char const *pcScript);

	//void FollowActor(lwGUID kMasterGuid, float fMoveSpeed = 1.0f);

	void Teleport();

	void UpdateModel(char const *pcActorName);

	bool OutOfSight(lwGUID kTargetGuid, float fDistance, bool bConsiderZAxis = false);

	int CompareActorPosition(lwGUID kTargetGuid, lwPoint3 kAxis, float fRange = 20.0f);

	bool WillBeFall(bool bDown, float fDistance);

	void SetSpeedScale(float fScale);

	float GetSpeedScale();

	lwGUID GetPetMaster();

	lwActor GetMyPet();

	bool IsRidingPet(void);
	lwActor GetMountTargetPet(void) const;
	bool UnmountPet(void);
	bool MountPet(void);

	bool LookAt(lwPoint3 kNormal,bool bTurnRightAway,bool bBidirection,bool bNoCheckSameDir);
	void LookAtBidirection(lwPoint3 kNormal, bool bTurnRightAway);

	lwPoint3 GetLookingDir();

	BYTE GetLastDirection();

	void DoDropItems(int iReqCount,float fJumpHeight);

	void IncRotate(float fRadian);

	void DetachChild(char const *pcName);

	lwPoint3 GetHitPoint();
	lwPoint3	GetHitABVCenterPos();
	int	GetHitObjectABVIndex();

	void AddToDefaultItem(int iItemPos, int iItemNo);

	void SetDefaultItem(int iItemPos, int iItemNo);

	bool EquipItem(int iItemNo, bool bSetToDefaultItem);

	void PlayWeaponSound(int iActionType, lwActor kPeer, char const *pcActionID, float fVolume, lwActionTargetInfo kTargetInfo);

	int GetDefaultItem(int iPos);

	void SetItemColor(int iItemPos, int iItemNo);

	void SetHeadScale(float f);
	
	bool HideParts(int iItemPos, bool bHide);
	void	HideNode(char const *strNodeName,bool bHide);
	int GetPartsHideCnt(int iItemPos);

	void Talk(int i, int iUpTime);

	//! Boss Object ID를 설정한다.
	void SetBoss();

	//! 인자로 들어오는 Actor와의 거리를 반환한다.
	float GetDistance(lwActor kActor);

	//! Effect를 추가한다. (도트)
	bool AddEffect(int iEffectNo, int iEffectValue, lwGUID kCasterGUID, int iActionInstanceID, DWORD dwTimeStamp, bool bIsTemporaryEffect);

	//! 액터를 회전시킨다(빙글 빙글)
	//void TwistActor(int fTwistTimes, float fSpeed);

	//! 돌렸던 액터를 원상복귀
	void RestoreTwistedActor();

	//! 파마메터를 돌려준다.
	char const *GetParam(char const *pcKey);

	//! 파라메터를 설정한다.
	void SetParam(char const *pcKey, char const *pcVal);

	//! 목표를 향해 점프한다.
	float StartTeleJump(lwPoint3 kTargetLoc, float fHeight);

	//! 떨어지지 않는 가능한 좌표로 강제 이동 할지 안할지 설정한다.
	void SetAdjustValidPos(bool bAble);

	//! 타겟 포지션으로 움직이록 설정한 것을 해제한다.
	void SetNoWalkingTarget(bool bDoNextAction);

	//! WalkingToTarget 인지 아닌지 리턴.
	bool GetWalkingToTarget();

	//! Walking Target Direction을 얻는다.
	lwPoint3 GetWalkingTargetDir();
	
	//! 목표 타겟으로 움직이록 설정한다.
	void SetWalkingTarget(lwGUID kGuid, bool const bForceToTarget , char const *pcNextAction, bool bCliffCheck);

	//! 목표 지점으로 움직이록 설정한다.
	void SetWalkingTargetLoc(lwPoint3 kTargetLoc, bool const bForceToTarget , char const *pcNextAction, bool bCliffCheck);

	//! 목표 지점을 반환한다.
	lwPoint3 GetWalkingTargetLoc();

	//! 두 방향만을 보게 하거나, 8방향 모두 보게 한다.
	void LockBidirection(bool bLock);
	bool IsLockBidirection()const;

	//! 월드에 셋팅되어 있는 방향 셋팅으로 되돌린다.
	void RestoreLockBidirection();

	//! Layer Animation
	bool ActivateLayerAnimation(char const *pcAnimationName);
	
	bool DeactivateLayerAnimation(char const *pcAnimationName);

	//! Target Alpha 설정
	void SetTargetAlpha(float fStartAlpha, float fTargetAlpha, float fTransitionTime, bool bIgnoreLoadingFisnishInit);

	//! Alpha Transit이 끝났는가?
	bool IsAlphaTransitDone();

	//! Actor의 LastFloorPos를 초기화 시킨다.
	void ResetLastFloorPos();

	//!	죽을 때 뿌려줄 이펙트 파티클 ID 를 얻어온다.
	char const* GetDieParticleID();
	char const* GetDieParticleNode();
	float	GetDieParticleScale();
	char const* GetDieSoundID();

	bool GetExistDieAnimation();
	bool GetUseDieAnimation();
	void SetUseSubAnimation(bool bUse);

	float	GetEffectScale();


	//!	무시할 이펙트 리스트에 추가한다.
	void	AddIgnoreEffect(int iEffectID);
	void	RemoveIgnoreEffect(int iEffectID);
	void	ClearIgnoreEffectList();

	bool	IsBlowUp();
	float	GetBlowUpStartTime();
	void	SetBlowUp(bool bBlowUp);

	void	ClearAllActionEffect();

	//!	수직 아래로 레이를 쏘아서 바닥으로부터 이 액터가 얼마나 떠 있는지를 반환한다.
	float	GetFloatHeight(float fRayLength);

	//! 다른 액터를 이 액터의 어느 부위에 붙인다.
	void AttachToObject(lwActor Actor, char const *pcNodeName);
	void DetachFromObject(char const *pcNodeName);	// 모든 자식을 떼버린다.

	//! 이 액터를 자신의 부모로 부터 땐다.
	void DetachFromParent();

	//! Transfomation(Translate, Scale, Rotation 등을 초기화.)
	void ResetTransfomation();
	
	BYTE GetDirFromMovingVector(lwPoint3 kPoint);
	BYTE GetDirFromMovingVector8Way(lwPoint3 kPoint);

	lwPoint3 GetDirectionVector(int iDir);

	void SetLookingDirection(BYTE byDir,bool bTurnRightAway);

	void	AddHeadBuffIcon(int iEffectNo);
	void	RemoveHeadBuffIcon(int iEffectNo);

	void	BeforeUse();
	void	SetInstallTimerGauge(float fTotalTime);
	void	DestroyInstallTimerGauge();

	bool	FindFollowingMeActor(lwGUID kActorGUID);
	bool	IsNowFollowing();
	lwGUID  GetFollowingTargetGUID();
	void	SetStartFollow(lwGUID kTargetActorGUID);
	void	AddFollowingMeActor(lwGUID kGUID);
	void	FollowActor(lwActor kTarget);

	bool	FollowingMeActorCheck(lwGUID kTargetActorGUID);
	void	ResponseFollowActor(lwGUID kTargetActorGUID,int kMode);	
	void	RequestFollowActor(lwGUID kTargetActorGUID,int kMode, bool bForce = false);	
	bool	CheckCanFollow(lwGUID kTargetActorGUID, bool const bMsg=false)const;

	void	ChangeKFM(char const *strKFMPath,bool bChangeImmediately);

	//! Actor의 현재 회전된 쿼터니온 값을 얻어온다.
	lwQuaternion GetRotateQuaternion();

	//! 아이템 숨기기
	void HideEquipItem(int iEquipPos, bool bHide);

	//! Update PhysX에 사용되는 FrameTime지정
	void SetUpdatePhysXFrameTime(float fFrameTime);

	//! (몬스터의 경우에만) 타겟을 바라보게 한다.
	void SetLookTarget(bool bLook);

	bool IsEqualObjectName(char *szName);
	
	bool	IsStun();
	int		GetSpecificIdle();

	void	SetSpotLightColor(int Red,int Green,int Blue);

	void	CopyEquipItem(lwActor kActor);

	void	SetAutoDeleteActorTimer(float fTime);

	void	SetUseLOD(bool bUse);

	bool	AttachAttackEffect(char const* szActionName, int const iSlot);
	lwAttackEffect GetAttackEffect(char const* szActionName);
	char const*	GetDamageEffectID(bool bIsCri = false);
	
	bool const IsInCoolTime(int iSkillNo, bool bIsGobalCoolTime) const;

	lwActor GetSubPlayer();
	lwActor GetCallerActor() const;

	//!	현재 애니메이션 시퀀스 번호를 리턴한다.
	int	GetAniSequenceID();

	void	SetDefaultHeadSize(float const fDefaultHeadSize);
	float	GetDefaultHeadSize() const;
	void	SetTargetHeadSize(float const fTargetHeadSize, float const fTransitSpeed);

	void	SetAlpha(float fAlpha);
	float	GetAlpha()const;

	void	SetHideNameTitle(bool const bHide);
	bool	IsHideNameTitle() const;
public:
	lwWString GetStatusEffectParam(int const iEffectID, char const* szKey)const;
	void SetStatusEffectParam(int const iEffectID, char const* szKey, char const* szValue);

	char const* GetStartParamID(char const *kStr);
	int const GetStartEffectSave(char const *kStr);

	lwWString GetNpcMenuStr();

	void SetAnimationStartTime(float fTime);
	float GetAnimationStartTime();

	lwGUID GetCallerGuid()const;

	void IncTimeToAniObj(int const iMilliSec = 0);
	
	void MakePetActionQueue(int iGrade);
	int UpdatePetActionQueue(float fAccumTime);

	short lwActor::GetHomeAddrTown();
	int lwActor::GetHomeAddrHouse();

	void AddSummonUnit(lwGUID kGuid, int iClassNo, bool bUniqueClass);
	int GetSummonUnitCount();
	int GetMaxSummonUnitCount();
	bool DeleteSummonUnit(lwGUID kSummonUnitGuid);
	lwGUID GetSummonUnit(int iIndex);
	void ClearSummonUnit();
	void PickUpNearItem(float const fPickRange, int const iCount);	//절대 펫 이외에 부르면 안됨
	void SetAttackState(int iValue);
	int GetAttackState();

	bool GetActionToggleState(int iActionNo);

	lwGUID FollowingHeadGuid();
	int GetGroggyRecoveryValue()const;

	float GetAnimationTime(char const* strActionName);

	void SetDuelWinnerTitle(void);

	void	SetComboCharge(float const fChargingTime);
	void	CallComboCharge();
	void	CutComboCharge();

	bool CheckCanRidingAttack(void);
protected:
	PgActor *m_pkActor;
};
#endif //FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTOR_LWACTOR_H