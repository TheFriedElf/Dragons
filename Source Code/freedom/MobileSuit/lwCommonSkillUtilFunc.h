#ifndef DRAGONICA_MOBILESUIT_lwCommonSkillUtilFunc_H
#define DRAGONICA_MOBILESUIT_lwCommonSkillUtilFunc_H

class lwActor;
class lwAction;
class lwGUID;
class lwPoint3;
class lwActionTargetList;
class lwProjectile;

namespace lwCommonSkillUtilFunc
{
	typedef std::vector<int> CONT_INT;

	enum EMovingType
	{// 캐릭터 위치 이동을 시키는 타입
		EMT_NONE						= 0,
		EMT_INTERPOLATION			= 1,	// 위치보간 하기
		EMT_VELOCITY					= 2,	// 속도 넣어주기
		EMT_VELOCITY_TO_GOAL_POS		= 3, // 속도로 위치 찾아가기
		EMT_VELOCITY_TO_GOAL_POS2	= 4, // 속도로 위치 찾아가기(타겟이 없으면, 이동하지 않음)
		EMT_TELEPORT					= 5, // 위치로 순간이동
	};

	enum EActionTime
	{// 액션 진행중 특정 시점
		EAT_NONE				= 0,
		EAT_CAST_COMPLETE	= 1,
		EAT_ON_CLEANUP		= 2,
		EAT_ANI_COMPLETE		= 3,
		EAT_MSG_FIRE			= 4,
		EAT_ON_ENTER			= 5,
		EAT_MSG_HIT			= 6,
		EAT_ANI_SLOT_TIME	= 7,
		EAT_MSG_SC_START = 8,
		EAT_MSG_QUAKE_CAM = 9,
		EAT_MSG_EFFECT = 10,
		EAT_MSG_ALPHA = 11,
		EAT_MSG_CHANGE_CAMERA = 12,
		EAT_HIT_TARGET = 13, 
		EAT_ON_CLEANUP_COMBO_CANCEL		= 14,
		EAT_MSG_QUAKE_MY_CAM = 15,
		EAT_MSG_UI	= 16,
		EAT_MSG_MOVE_START	= 17,
		EAT_MSG_MOVE_END	= 18,

		EAT_ANI_SLOT0 = 100,	// 애니슬롯 100개 까지 허용(그이상도 가능하지만 그리 쓸일은 없을듯)
		EAT_ANI_SLOT_MAX = 200,
	};
	
	enum ENextAniIncompleteReason
	{// 다음 슬롯의 애니를 시작 하려는데 실패한 이유
		ENAIR_NONE				= 0,
		ENAIR_ON_FLOATING		= 1<<1,	// 공중에 떠있음
		ENAIR_REMAIN_ANI_REPEAT	= 1<<2, // 애니 반복이 남아있음
	};
	
	enum EActionOverridePacketType : int
	{// 액션의 Override된 패킷의 타입
		EAOPT_SYNC_DIR = 1,
	};
	
	enum EActionCancelType
	{// 현재 액션사용중 캔슬할수 있는 조건 설정
		EACT_NONE = 0,					// 캔슬 불가(데미지 액션이 들어올땐 제외)
		EACT_ALLOW_ALL = 1,				// 모든 액션 캔슬 가능
		EACT_ALLOW_SOME_ACTION = 2,		// 몇몇 액션만 허용
		EACT_REJECT_SOME_ACTION = 3,	// 몇몇 액션만 허용안함
		EACT_SPECIFY_TOGGLE_ACTION = 4, // 토글스킬로 때릴때, 마지막 종료 애니할때는 캔슬할수 있게 함
		EACT_MAX
	};

	enum EActionTransitCheckType
	{// 다음 액션중 이전 액션의 이름을 체크하는 조건 설정
		EATCT_ALLOW_ALL = 0,			// 모든 액션 전이 가능
		EATCT_ALLOW_SOME_ACTION = 1,	// 몇몇 이전 액션 이름들이 같다면 허용
		EATCT_REJECT_SOME_ACTION = 2,	// 몇몇 이전 액션 이름들이 다르다면 액션만 허용
		EATCT_MAX
	};
	
	enum EProjectileArriveType
	{// 발사체가 도착하는 타입
		EPAT_NONE		= 0,
		EPAT_TARGET_POS	= 1, // 타겟의 위치를 도착지점으로
		EPAT_SET_DIRECTION	= 2, // 설정 방향으로
	};
	
	enum EProjectileRotateType
	{// 발사체 발사 방향 회전 타입
		EPRT_NONE		= 0,
		EPRT_UP_DOWN		= 1, //위아래
	};

	enum ETrailType
	{// 트레일을 붙이는 위치
		ETT_NONE = 0,
		ETT_BODY_TRAIL	 = 1,
		ETT_R_HAND_TRAIL = 2,
		ETT_L_HAND_TRAIL = 3,
		ETAT_R_FOOT_TRAIL= 4,
		ETAT_L_FOOT_TRAIL= 5,
		ETT_MAX
	};
	
	enum ENextTrainsitActionCheckType
	{// 다음 액션을 정할때 체크해야 할것이 있는가
		ENTACT_NONE = 0,		/// 액션을 종료할 때
		ENTACT_FIND_TARGET = 1,	//					타겟을 잡아보니 있는가?
		ENTACT_IS_PLAYER = 10,	//					유닛 타입이 플레이어 인가?
	};

	enum EOrderActionTarget
	{
		EOAT_NONE = 0,
		EOAT_SUB_PLAYER = 1,	// SC (Sub Character, 쌍둥이 보조캐릭터류)
	};
	
	enum EWFFColorAlphaTarget
	{// GetWorldFoucsFilterColor 적용시 나에게만 적용하는가, 모두에게 적용하는가
		EWFFCAT_NONE = 0,
		EWFFCAT_ME = 1,
		EWFFCAT_ALL = 2,
	};

	enum EAttachParticleType
	{
		APT_NODE = 0,
		APT_NODE_POS = 1,
		APT_NODE_POS_WITH_ROTATE = 2,
		APT_CALLER_ROTATE = 3,
		APT_ACTOR_POS = 4,
		APT_ATTACH_TO_WORLD = 5,
		APT_ATTACH_TO_WORLD_WITH_ROTATE = 6,
		APT_ATTACH_TO_RAY_DIR_COLL_PATH = 7,

		// 내 소유하의 Actor들에게만 파티클을 붙이는 타입	
		APT_NODE_ONLY_MINE							 = APT_NODE + 10000,									// 10000
		APT_NODE_POS_ONLY_MINE						 = APT_NODE_POS + APT_NODE_ONLY_MINE,					// 10001
		APT_NODE_POS_WITH_ROTATE_ONLY_MINE			 = APT_NODE_POS_WITH_ROTATE + APT_NODE_ONLY_MINE,		// 10002
		APT_CALLER_ROTATE_ONLY_MINE					 = APT_CALLER_ROTATE + APT_NODE_ONLY_MINE,				// 10003
		APT_ACTOR_POS_ONLY_MINE						 = APT_ACTOR_POS + APT_NODE_ONLY_MINE,					// 10004
		APT_ATTACH_TO_WORLD_ONLY_MINE				 = APT_ATTACH_TO_WORLD + APT_NODE_ONLY_MINE,			// 10005
		APT_ATTACH_TO_WORLD_WITH_ROTATE_ONLY_MINE	 = APT_ATTACH_TO_WORLD_WITH_ROTATE + APT_NODE_ONLY_MINE,// 10006

	};

	enum ECamearaMode
	{// 액션중 카메라 조정
		ECM_NONE = 0,
		ECM_STOP = 1,
		ECM_USE_CAMERA_NAME_IN_ACTOR = 10,

		ECM_STOP_ONLY_MINE = ECM_STOP + 10000,
		ECM_USE_CAMERA_NAME_IN_ACTOR_ONLY_MINE = ECM_USE_CAMERA_NAME_IN_ACTOR+ 10000,
	};
	
	enum EDecorationActorType
	{// 연출용으로 붙이는 액터 조정
		EDAT_NONE = 0,
		// 내 소유하의 Actor들에게 파티클을 붙이는 타입
		EDAT_ONLY_MINE = 10001,
	};

	enum EWorldFoucsFilterColorAlphaKeepType
	{// 포커스 필터 (배경 검어지는등)
		EWFFCAKT_NONE = 0,
		EWFFCAKT_KEEP_OTHER_ACTION		= 1,	// 특별히 제거해주지 않으면 계속 유지
		EWFFCAKT_KEEP_UNTIL_CLEAN_UP	= 2,	// 액션이 끝날때까지 유지
	};
	
	enum EUIControlType
	{// UI 끄고 키는 조정
		EUICT_NONE = 0,
		EUICT_CLOSE_ALL_UI = 1,		// 액션 시작시 모든 UI 제거
		EUICT_CALL_BASIC_UI = 2,	// 액션 중 기분 UI를 호출함(메인UI들). EUICT_CLOSE_ALL_UI와 짝을 이루어야함
		EUICT_CALL_UI	= 3,		// 액션 중 UI를 호출함
	};
	extern int const IDX_PLAY_HIT_SOUND;
	extern int const IDX_PLAY_HIT_GENDER_SOUND;

	extern int const ATTACK_TIMER_ID;						// 때릴때 사용하는 Timer의 아이디

	extern int const IDX_SPECIFY_PROCESS_ANI_SLOT_PARAM;	// AniSlot일때 특정 동작을 하기 위해서 param을 저장함, 100개씩 저장하며, 10개의 슬롯까지 가능(20100~21000)
		extern int const SUB_IDX_MEET_FOOLR_THEN_NEXT_ANI_SLOT;	// (일부러 들여쓰기함)

	extern int const IDX_PARTICLE_SLOT_AT_CAST_COMPLETE;	// 캐스팅 완료후 파티클 붙을 idx			(100000)
	extern int const IDX_PARTICLE_SLOT_AT_ON_CLEANUP;		// 액션 종료시 파티클 붙을 idx				(100100)
	extern int const IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG;		// fire 이벤트 메세지시에 파티클 붙을 idx	(100200)
	extern int const IDX_PARTICLE_SLOT_AT_ON_HIT_MSG;		// hit 이벤트 메세지시에 파티클 붙을 idx	(100300)
	extern int const IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG;	// effect 이벤트 메세지시에 파티클 붙을 idx (100400)
	extern int const IDX_PARTICLE_SLOT_AT_ANI_START;		// 애니메이션 시작시붙을 idx;				(200000)
	extern int const IDX_PARTICLE_SLOT_AT_MOVE_START;		// 캐릭터 이동 시작시붙을 idx;				(300000)
	extern int const IDX_PARTICLE_SLOT_AT_MOVE_END;		// 캐릭터 이동 시작시붙을 idx;					(400000)
	
	extern std::string const STR_DEFALT_TRAIL_TEX;
	
	extern std::string const STR_ENTER;
	extern std::string const STR_CLEANUP;

	//--------------------------------------------------------
	// OnEvent에서 사용하는 이벤트 키 문자
	extern std::string const STR_EVENT_MSG_HIT;
	extern std::string const STR_EVENT_MSG_FIRE;
	extern std::string const STR_EVENT_MSG_MOVE_START;
	extern std::string const STR_EVENT_MSG_MOVE_STOP;
	extern std::string const STR_EVENT_MSG_STOP_JUMP_START;
	extern std::string const STR_EVENT_MSG_STOP_JUMP_END;
	//extern std::string const STR_EVENT_MSG_ORDER_ACTION;
	extern std::string const STR_EVENT_MSG_SC_START;
	extern std::string const STR_EVENT_MSG_ALPHA_START;
	extern std::string const STR_EVENT_MSG_QUAKE_CAM;
	extern std::string const STR_EVENT_MSG_QUAKE_MY_CAM;
	extern std::string const STR_EVENT_MSG_EFFECT;
	extern std::string const STR_EVENT_MSG_CHANGE_CAM;
	extern std::string const STR_EVENT_MSG_TAT_START;
	extern std::string const STR_EVENT_MSG_TAT_END;
	extern std::string const STR_EVENT_MSG_COMBO_DELAY_CANCEL;

	extern std::string const STR_CAMERA_ZOOM_WHEN_Z_CAM_MSG;
	extern std::string const STR_UI_MSG;
	//--------------------------------------------------------

	extern std::string const STR_EFFECT_CNT_WHEN_ANI_SLOT;
	extern std::string const STR_EFFECT_ID_WHEN_ANI_SLOT;
	extern std::string const STR_EFFECT_TYPE_WHEN_ANI_SLOT;
	extern std::string const STR_EFFECT_SCALE_WHEN_ANI_SLOT;
	extern std::string const STR_EFFECT_TARGET_NODE_WHEN_ANI_SLOT;
	extern std::string const STR_EFFECT_DETACH_TIME_ANI_SLOT;

	extern std::string const STR_TRUE;
	extern std::string const STR_FALSE;

	//extern std::string const STR_FORWARD;
	extern std::string const STR_REAR;
	extern std::string const STR_CALLER;
	extern std::string const STR_SELF;
	extern std::string const STR_TARGET;
	extern std::string const STR_SAVED_TARGET;
	extern std::string const STR_USER_INPUT_DIR;
	extern std::string const STR_SC;
	extern std::string const STR_ANI_SLOT;

	extern std::string const STR_FIRE_PROJECTILE_ID;
	extern std::string const STR_FIRE_PROJECTILE_START_NODE_POS;
	extern std::string const STR_FIRE_PROJECTILE_MOVE_TYPE;
	extern std::string const STR_FIRE_PROJECTILE_ARRIVE_POS_TYPE;
	extern std::string const STR_FIRE_PROJECTILE_NUM;
	extern std::string const STR_FIRE_PROJECTILE_NUM_FROM_PARENT_SKILL_NO;
	extern std::string const STR_FIRE_PROJECTILE_START_POS_ADD_X;
	extern std::string const STR_FIRE_PROJECTILE_START_POS_ADD_Y;
	extern std::string const STR_FIRE_PROJECTILE_START_POS_ADD_Z;
	//extern std::string const STR_HIT_TARGET_EFFECT_TARGET_NODE;
	//extern std::string const STR_HIT_TARGET_EFFECT_ID;
	extern std::string const STR_FIRE_PROJECTILE_SIN_POWER;
	extern std::string const STR_FIRE_PROJECTILE_SIN_ANGLE;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG1;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_VERT_DEG1;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_DIST1;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG2;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_VERT_DEG2;
	extern std::string const STR_FIRE_PROJECTILE_BEZIER_DIST2;
	
	extern std::string const STR_WHEN_START_DIRECTION;
	
	extern std::string const STR_C_CAM_MSG_KEEP;
	extern std::string const STR_C_CAM_MSG_TYPE;
	extern std::string const STR_C_CAM_MSG_CAMERA_NAME;

	extern lwPoint3 const UP_VEC;

	extern void RegisterWrapper(lua_State *pkState);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//- - - - - - - - - - - - - - - - - -//
	// lwCommonSkillUtilFunc_FSM.cpp     //
	//- - - - - - - - - - - - - - - - - -//
	// FSM 계열 함수
	extern bool OnCheckCanEnter(lwActor kActor, lwAction kAction);
	extern bool OnEnter(lwActor kActor, lwAction kAction);
	extern void OnOverridePacket(lwActor kActor, lwAction kAction, lwPacket kPacket);
	extern bool OnCastingCompleted(lwActor kActor, lwAction kAction);
	extern bool OnUpdate(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime);
	extern bool OnLeave(lwActor kActor, lwAction kNextAction, bool bCancel);
	extern bool OnCleanUp(lwActor kActor, lwAction kAction);
	extern bool	OnEvent(lwActor kActor, char const* pcText, int iSeqID);
	extern bool	OnTimer(lwActor kActor, lwAction kAction, float fCallTime, int iTimerID);
	extern int OnFindTarget(lwActor kActor, lwAction kAction, lwActionTargetList kTargetList);
	extern void OnTargetListModified(lwActor kActor, lwAction kAction, bool bIsBefore);
	extern void OnMoveControl(lwActor kActor, lwAction kAction);
	extern int FindComboAction(lwActor kActor, lwAction kAction, bool const bCharge, int &iNextAction_SC);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	extern void ProcessEtc(lwActor kActor, lwAction kAction, EActionTime const eActionTime);

	extern bool ProcessCameraZoomRestore(lwActor kActor, lwAction kAction, EActionTime const eActionTime );

	// 캐스팅 시간의 변화를 계산하는 함수
	extern int CalcTotalCastTime(int const iOrigCastTime, int const iAddCastTime, int const iDecCastTimeRate);

	// SubPlayer에 액션 전이를 시도
	extern bool TryMustChangeSubPlayerAction(lwActor kOwnerActor, char const* pcActionName, BYTE byDir, int const iCheckCoolTimeSkillNo = 0, bool const bDoSameAction=false);

	// Actor에게 해당 액션으로 전이 시도
	extern bool TryMustChangeActorAction(lwActor kActor, char const* pcActionName);

	// 액션을 시작할때 방향 초기화
	extern bool InitDirection(lwActor kActor, lwAction kAction, char const* pcScriptParam);

	// 현재 애니메이션 슬롯의 Speed 파라메터가 있다면 얻어온다(XML/PC/ 경로에 XML에 정의된 애니 SPEED)
	extern float GetAniSlotSpeed(lwActor kActor, lwAction kAction, int const iSlot);

	// 타격 이펙트 사용 유무
	extern bool InitUseDmgEffect(lwAction kAction);

	// 무기 감추기/보이기 (반드시 감추기 보이기가 짝이 맞아야함)
	extern bool HideWeapon(lwActor kActor, lwAction kAction, bool const bHide);

	// 액션을 자동으로 브로드캐스팅 하지 않음
	extern void InitUseAutoBroadCast(lwAction kAction);

	// 캐스팅이 완료된 시점에서 자동으로 서버로 브로드 캐스팅 하는가
	extern void InitUseAutoBroadCastWhenCastingCompleted(lwAction kAction);
	
	// OverridePacket을 처리하는 부분(대부분 동기화에 쓰임
	extern bool ProcessOverridePacket(lwActor kActor, lwAction kAction);
	
	extern void ProcessOnAniSlotTime(lwActor kActor, lwAction kAction, int const iCurAniSlot);// 각 애니 슬롯이 되었을때의 처리

	extern bool ProcessPlaySound(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	
	extern bool ProcessQuakeCamera(lwAction kAction, EActionTime const eActionTime);
	//// 공격 할수 있는가
	//extern bool IsCanAttack(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 파티클 관련
	extern bool DetachAniSlotParticle(lwActor kActor, lwAction kAction, int const iAniSlot, int const iActionTimeType);
	extern bool AttachParticle(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	extern bool SubFuncAttachParticle(lwActor kActor, lwAction kAction,int const iSlot, char const* pckParticleScriptParam
		, char const* pcTargetNodeScriptParam, char const* pcScaleScriptParam, char const* pcAttachTypeScriptParam
		, char const* pcHorzDeg, char const* pcVertDeg, char const* pcDist 
		);
	extern void AttachParticleOnAniSlotTime(lwActor kActor, lwAction kAction, int const iAniSlot);
	extern bool ProcessEffectMsg(lwActor kActor, lwAction kAction);

	// event 메세지 처리
	extern bool ProcessHitMsg(lwActor kActor, lwAction kAction);
	extern bool ProcessFireMsg(lwActor kActor, lwAction kAction);
	extern bool ProcessAlphaMsg(lwActor kActor, lwAction kAction);

	// 다음 애니메이션 슬롯으로 넘어갈수 있는지 체크하고, (불가능한 이유를 리턴)
	extern int IsAbleToNextAniSlot(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime);
	// 불가능 하다면 그에 대한 처리
	extern void ProcessNextAniIncompleteReason(int const iIncompleteReason, lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime);
	
	// 일반적인 타격 이펙트 처리
	extern void DefaultHitOneTime(lwActor kActor, lwAction kAction, bool bApplyDmg);
	extern void DoDividedDamage(lwActor kActor, lwAction kAction, int iHitCount);
	// 때릴때 사운드
	extern void PlayHitSound(lwActor kActor, lwAction kAction);
	// 타격시 무기 소리를 재생하는 부분
	extern void PlayWeaponSound(lwActor kActor, lwAction kAction, bool const bCritical, lwActor kTargetActor, lwActionTargetInfo kTargetInfo);
	// 일반적인 타격 이펙트
	extern void DefaultDoDamage(lwActor kActor, lwActor kActorTarget,lwActionResult kActionResult,lwActionTargetInfo kActionTargetInfo);
	extern void SubDefaultDoDamageEffect(lwActor kActor, lwActor kActorTarget,lwActionResult kActionResult,lwActionTargetInfo kActionTargetInfo);

	//콤보
	extern bool IsComboDelay(lwActor kActor);
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 10000 - 액션 사용 중간에 타겟을 다시 잡아 때리는가? on/off 체크값
	extern void SetUseTLM(lwAction kAction, bool const bUseTLM);
	extern bool IsUseTLM(lwAction kAction);
	
	// 10001 - 점프중에 멈춰야 하는 액션인가
	extern void SetStopJump(lwActor kActor, lwAction kAction, bool const bStop);
	extern bool IsStopJump(lwAction kAction);
	
	// 10002 - 초당 회전 각도 값
	extern bool InitSpinDegPerSec(lwAction kAction);
	extern void SetSpinDegPerSec(lwAction kAction, float const fRotateDegPerSec);
	extern float GetSpinDegPerSec(lwAction kAction);
	
	// 10003 - 바닥에 닿아야만 다음 애니메이션을 할수 있는가
	extern void SetMeetFloorThenPlayNextAni(lwAction kAction, bool const bOn);
	extern bool GetMeetFloorThenPlayNextAni(lwAction kAction);
	
	// 10004 - 기본적인 타격 이펙트를 사용하는가
	extern void SetUseDmgEffect(lwAction kAction, bool const bOn);
	extern bool GetUseDmgEffect(lwAction kAction);
	
	// 10005 - 사용자의 입력에 의한 방향을 사용하는가
	extern void InitSyncInputDir(lwActor kActor, lwAction kAction);
	extern void UseSyncInputDir(lwAction kAction, bool const bUse);
	extern bool IsSyncInputDir( lwAction kAction);

	// 10006 - 몇번째 발사체가 발사 되는가
	extern int IncFireProjectileCnt(lwAction kAction);
	extern int GetFireProjectileCnt(lwAction kAction);

	// 10007 - 캐스팅 시간 감소에 따른 애니 스피드 조정
	extern void AdjustAniSpeedOnCastingTime(lwActor kActor, lwAction kAction);
	extern void ApplyAniSpeedOnUpdate(lwActor kActor, lwAction kAction);				//update시에 적용시켜줌 
	
	// 10008 - 최대 몇번이나 Create Target을 할수 있는가 (공격을 몇회 하는가)
	extern void InitMaxHitCnt(lwAction kAction);	
	extern void SetMaxHitCnt(lwAction kAction, int const iMaxHitCnt);
	extern int GetMaxHitCnt(lwAction kAction);
	// 10009 - 현재 몇번이나 CreateTarget을 했는가 (공격을 몇회 했는가)
	extern int IncHitCnt(lwAction kAction);
	extern int GetHitCnt(lwAction kAction);
	extern bool IsOverHitCnt(lwAction kAction);		// 최대 CreateTarget을 넘었는가

	// 10010 - 액션 사용중 캔슬할수 있는 상태(EActionCancelType)
	extern void InitCancelOption(lwAction kAction);
	extern int GetCancelOption(lwAction kAction);
	extern bool CheckCancelAction(lwActor kActor, lwAction kAction, lwAction kNextAction);
	
	// 10011~10110 - 현재 액션 사용중 캔슬과 연관된 액션 네임들
	extern void SetCheckActionName(lwAction kAction, int const iIdx, char const* pcActionName);
	extern char const* GetCheckActionName(lwAction kAction, int const iIdx);
	
	// 10120 - 다음 액션 전이 조건을 체크 하는 
	extern void InitTransitCheck(lwAction kAction);
	extern int GetTransitCheckType(lwAction kAction);
	extern bool CheckTransitAction(lwActor kActor, lwAction kAction, lwAction kPrevAction);
	// 10121~10220 - 다음 액션 전이를 위해 현재 액션 네임 체크
	extern void SetTrasitCheckPrevActionName(lwAction kAction, int const iIdx, char const* pcActionName);
	extern char const* GetTrasitCheckPrevActionName(lwAction kAction, int const iIdx);

	// 10221 ~ 10320 - 현재 액션을 캔슬하기 위해 현재 이펙트 체크
	extern void SetCancelActionCheckEffect(lwAction kAction, int const iIdx, int iCheckEffectNo);
	extern int GetCancelActionEffect(lwAction kAction, int const iIdx);
	
	// 10400 - 다음 액션 전이 조건 중 캐스팅 완료 상태로 이 액션에 넘어왔는지 확인 하는 변수
	extern void SetTransitCheck_CheckCastComplete(lwAction kAction, bool const bCastComplete);
	extern bool GetTransitCheck_CheckCastComplete(lwAction kAction);

	
	// 10500 - 애니메이션 중 QuakeCamera 메세지인 'cam' Event Msg가 몇번 들어왔는가?
	extern int IncMsgCamCnt(lwAction kAction);
	extern int GetMsgCamCnt(lwAction kAction);

	// 10800 - Trail 을 사용하는가
	extern void InitUseTrail(lwAction kAction);
	extern bool IsUseTrail(lwAction kAction);
	// 10801 - Trail 어떤때 Trail을 쓰는가
	extern void SetTrailUseFlag(lwAction kAction, int const iFlag);
	extern int GetTrailUseFlag(lwAction kAction);
	extern void ProcessTrail(lwActor kActor, lwAction kAction, EActionTime const eActionTime);

	// 연계 포인트를 사용 하는 스킬이라면, 발동 이펙트 등을 붙여줄수 있게 처리
	extern void AttachLinkageUseSkillEffect(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	// 10802 스킬 체크 이펙트를 이미 한번 사용했는가
	extern void SetCheckLinkageUseSkillEffect(lwAction kAction, bool const bUsed);
	extern bool GetCheckLinkageUseSkillEffect(lwAction kAction);
	
	// 10810 정면 바라보기 설정
	extern void InitSeeFrontInfo(lwActor kActor, lwAction kAction);
	extern void SetSeeFront(lwAction kAction, bool const bSeeFront);
	extern bool IsSeeFront(lwAction kAction);
	
	// 10811 alpha_start 메세지가 몇번이나 들어왔는가
	extern int IncMsgAlphaCnt(lwAction kAction);
	extern int GetMsgAlphaCnt(lwAction kAction);
	
	// 특정 액터만 화면에 표시 하는것을 사용한다면 정보를 읽어옴
	extern void InitDrawActorFilter(lwAction kAction);
	// 10812 특정 액터만 화면에 표시 하는것을 사용하는가?
	extern bool IsUseDrawActorFilter(lwAction kAction);
	// 10813 CleanUp시에 DrawActorFilter를 제거 하는가?
	extern bool IsClearDrawActorFilter_OnCleanUp(lwAction kAction);
	extern void ProcessDrawActorFilter(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	
	// 10814 kAction중 SC스킬을 쓸수 있는가?
	extern void InitIsBanSubPlayerAction(lwActor kActor, lwAction kAction);
	extern bool IsBanSubPlayerAction(lwAction kAction);

	// ----------------------------------------------------------------------------------------------------
	// 화면을 어두워 지게 하거나 색을 바꾸기
	extern void InitWorldFocusFilterColorAlpha(lwActor kActor, lwAction kAction);
	// 10821 대상이 누구일때(나, 모두)
	extern void SetWorldFoucsFilterColorAlpha_Target(lwAction kAction, int const iTargetType);
	extern int GetWorldFoucsFilterColorAlpha_Target(lwAction kAction);
	// 10822 무슨 색으로
	extern void SetWorldFoucsFilterColorAlpha_Color(lwAction kAction, int const iColor);
	extern int GetWorldFoucsFilterColorAlpha_Color(lwAction kAction);
	// 10823 alpha 몇에서
	extern void SetWorldFoucsFilterColorAlpha_AlphaStart(lwAction kAction, float const fAlphaStart);
	extern float GetWorldFoucsFilterColorAlpha_AlphaStart(lwAction kAction);
	// 10824 alpha 몇으로
	extern void SetWorldFoucsFilterColorAlpha_AlphaEnd(lwAction kAction, float const fAlphaEnd);
	extern float GetWorldFoucsFilterColorAlpha_AlphaEnd(lwAction kAction);
	// 10825 몇초에 걸쳐서
	extern void SetWorldFoucsFilterColorAlpha_TotalTime(lwAction kAction, float const fTotalTime);
	extern float GetWorldFoucsFilterColorAlpha_TotalTime(lwAction kAction);
	// 10826 어느 시점에
	extern void SetWorldFoucsFilterColorAlpha_ActionTime(lwAction kAction, int const iActionTime);
	extern int GetWorldFoucsFilterColorAlpha_ActionTime(lwAction kAction);
	// 10827 배경 색바꾼것(필터)를 유지 할것인가, 자동적으로 사라질것인가
	extern void SetWorldFoucsFilterColorAlpha_Keep(lwAction kAction, int const iKeep);
	extern int GetWorldFoucsFilterColorAlpha_Keep(lwAction kAction);

	extern void ProcessWorldFocusFilterColorAlpha(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	// ----------------------------------------------------------------------------------------------------
	// 10900  - 애니메이션 중 카메라 변경 메세지인 'c_cam' Event Msg가 몇번 들어왔는가?
	extern int IncMsgChangeCamCnt(lwAction kAction);
	extern int GetMsgChangeCamCnt(lwAction kAction);
	
	// 10901 현재 카메라 모드를 저장한다
	extern void SetCurCamMode(lwAction kAction, int const iCamMode);
	extern int GetCurCamMode(lwAction kAction);

	// 10902 현재 카메라 모드를 액션이 종료해도 유지 하는가?
	extern void SetKeepCamMode(lwAction kAction, bool const bKeepCamMode);
	extern bool IsKeepCamMode(lwAction kAction);
	extern void InitKeepCamMode(lwAction kAction);
	
	extern void ProcessChangeCam(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	// ----------------------------------------------------------------------------------------------------
	// UI 조정 EUIControlType
	//extern void InitUIControlInfo(lwActor kActor, lwAction kAction);

	//// 10950 UI 조정 타입
	//extern void SetUIControlType(lwAction kAction, int const iUIControlType);
	//extern int GetUIControlType(lwAction kAction);
	//// 10951 UI 조정 시작할때
	//extern void SetUIControlActionTime(lwAction kAction, int const iActionTime);
	//extern int GetUIControlActionTime(lwAction kAction);	
	
	// 10960 UI
	extern int IncUIControlMsgCnt(lwAction kAction);
	extern int GetUIControlMsgCnt(lwAction kAction);
	extern void ProcessUIMsg(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	
	extern void InitHideNameTitle(lwActor kActor, lwAction kAction);	// 언제 감출지 결정할 필요가 없다고 생각해서 Init에서 바로 처리함
	extern void ReleaseHideNameTitle(lwActor kActor, lwAction kAction);
	
	extern void InitHideSubActor(lwActor kActor, lwAction kAction);	// 언제 감출지 결정할 필요가 없다고 생각해서 Init에서 바로 처리함
	extern void ReleaseHideSubActor(lwActor kActor, lwAction kAction);
	// 위치 보간 관련 시작 ----------------------------------------------------------------------------------------------------------------------------------------------------------
	//- - - - - - - - - - - - - - - - - -//
	// lwCommonSkillUtilFunc_Moving.cpp  //
	//- - - - - - - - - - - - - - - - - -//
	// 11000 - 이동시작 on/off
	extern void SetMoving(lwActor kActor, lwAction kAction, bool const bMove);
	extern bool IsMoving(lwAction kAction);
	
	// 11008 - 이동 타입을 설정
	extern void SetMoveType(lwAction kAction, int const iMoveType);
	extern int GetMoveType(lwAction kAction);

	// 누구로부터(콜러/나/etc), 방향으로(각도), 만큼(인치) 이동할것인가를 읽어서 세팅
	extern bool InitMovingInfo(lwActor kActor, lwAction kAction, lwWString kTextKey, float fAcctime);			// 알아서 읽어서 세팅
	// 11001 - 이동할 목표점을 저장
	extern void SetMoveGoalPos(lwAction kAction, lwPoint3 kGoalPos);
	extern lwPoint3 GetMoveGoalPos(lwAction kAction);

	// 11002 - 이동 시작 위치를 저장
	extern void SetMoveStartPos(lwAction kAction,lwPoint3 kStartPos);
	extern lwPoint3 GetMoveStartPos(lwAction kAction);

	// 11003 - 애니메이션 Text Key에서, 몇번째 "move_start"가 들어왔는가
	extern int IncMoveStartCnt(lwAction kAction);
	extern int GetMoveStartCnt(lwAction kAction);

	// 11004 - 이동이 시작되는 시간(보간 완료시간)
	extern void SetMoveStartTime(lwAction kAction, float fStartTime);
	extern float GetMoveStartTime(lwAction kAction);

	// 11005 - 이동이 완료될 시간(보간 완료시간)
	extern void SetMoveArriveTime(lwAction kAction, float fArriveTime);
	extern float GetMoveArriveTime(lwAction kAction);
	
	// 11006 - 이동 보간중에 StopJump를 해야 하는가
	extern void SetStopJumpWhenMoving(lwAction kAction, bool const bStopJump);
	extern bool IsStopJumpWhenMoving(lwAction kAction);
	
	// 11007 - 이동 보간중에 바닥을 만났을때 멈춰야 하는가
	extern void SetWhenMeetFloorThenStopMoving(lwAction kAction, bool const bStopMoving);
	extern bool IsStopMovingWhenMeetFloor(lwAction kAction);

	// 11009 - MovingDelta값 세팅
	//extern bool InitMovingDelta(lwAction kAction);
	extern void SetMovingDelta(lwAction kAction, lwPoint3 const kVelocity);
	extern lwPoint3 GetMovingDelta(lwAction kAction);
	
	// 11010 - GoalPos를 계속해서 갱신 할것인가
	extern void UseMovingGoalPosUpdate(lwAction kAction, bool const bUseUpdate);
	extern bool IsMovingGoalPosUpdate(lwAction kAction);
	
	// 11011 - GoalPos를 계산 하는데 사용된 대상의 TargetGuid
	extern void SetMovingTargetGuid(lwAction kAction, lwGUID kTagrGetGuid);
	extern lwGUID GetMovingTargetGuid(lwAction kAction);

	// 11012 - Target의 위치로부터 더해줄 거리벡터의 방향
	extern void SetMovingDistVecRotate(lwAction kAction, lwPoint3 kDistVec);
	extern lwPoint3 GetMovingDistVecRotate(lwAction kAction);
	
	// 11013 - Target의 위치로부터 더해줄 거리벡터의 크기(위치보간 - 거리 / 속도 - Speed )
	extern void SetMovingDistVecLength(lwAction kAction, float const fLegnth);
	extern float GetMovingDistVecLength(lwAction kAction);
	
	// 11014 - 이동중 방향 업데이트를 사용하는가
	extern void UseMovingDirectionUpdate(lwAction kAction, bool const bUseUpdate);
	extern bool IsUseMovingDirectionUpdate(lwAction kAction);
	
	// 11015 - 목표점계산시 기준 위치에 더해줄 이격 방향
	extern void SetDistVecDir(lwAction kAction, lwPoint3 kDir);
	extern lwPoint3 GetDistVecDir(lwAction kAction);
	
	// 11016 - 속도를 통한 이동시, 속력을 저장
	extern void SetMovingSpeed(lwAction kAction, float const fSpeed);
	extern float GetMovingSpeed(lwAction kAction);
	//------------------------------------------------------------------------------------------------------------------------------
	// 11020 - 몇번째 다른 대상에 액션을 시키는(ORDER_ACTION)이 들어왔는가
	extern int IncOrderActionCnt(lwAction kAction);
	extern int GetOrderActionCnt(lwAction kAction);
	// 11021 - 11030 어떤 다른 대상인가를 저장(ORDER_ACTION_TARGET)
	extern bool SetOrderActionTargetType(lwAction kAction, int const iIdx, char const* pcTargetType);
	extern int GetOrderActionTargetType(lwAction kAction, int const iIdx);
	// 11031 - 11040 어떤 ActionTime에 액션을 하라고 전달 할것인가
	extern bool SetOrderActionTime(lwAction kAction, int const iIdx, char const* pcActionTime);
	extern int GetOrderActionTime(lwAction kAction, int const iIdx);
	// 11041 - 11050 ORDER_ACTION_TARGET 대상에게 ActionTime에 시킬 액션 이름은 무엇인가
	extern bool SetOrderActionName(lwAction kAction, int const iIdx, char const* pcActionName);
	extern char const* GetOrderActionName(lwAction kAction, int const iIdx);

	extern void InitOrderActionInfo(lwAction kAction);
	extern void ProcessOrderAction(lwActor kActor, lwAction kAction, EActionTime const eActionTime);
	//------------------------------------------------------------------------------------------------------------------------------
	// 11100 - 미리 타게팅해 잡아놓은 대상이 몇개인가
	extern void SetMaxSavedTargetCnt(lwAction kAction, int const iMaxCnt);
	extern int GetMaxSavedTargetCnt(lwAction kAction);	
	// 11101 - 미리 타게팅해 잡아놓은 대상중 몇번째 대상을 사용하고 있는가
	extern void SetCurUseSavedTargetIdx(lwAction kAction, int iIdx);
	extern int GetCurUseSavedTargetIdx(lwAction kAction);
	// 11200 ~ 11300 - 미리 타게팅해 잡아놓은 대상의 Guid를 저장
	extern void SetSavedTargetGuid(lwAction kAction, int const iIdx, lwGUID kTargetGuid);
	extern lwGUID GetSavedTargetGuid(lwAction kAction, int const iIdx);

	extern void ProcessMoving(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime);
	
	// 위치 보간 관련 끝-------------------------------------------------------------------------------------------------------------------------------------------------------------

	// 12000 - Timer
	extern bool InitAttackTimer(lwAction kAction);
	extern bool IfAbleAttackTimerThenStart(lwAction kAction, EActionTime const eActionTime);
	extern bool IsUseAttackTimer(lwAction kAction);
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	// 20000 특정 애니슬롯에서 해야할 특별한 처리가 있는가
	extern bool InitSpecifyProcessAnislot(lwAction kAction);
	extern void SetCurSpecifyProcessAniSlot(lwAction kAction, int const iAniSlot);
	extern int GetCurSpecifyProcessAniSlot(lwAction kAction);
	
	// 20100~21000 - AniSlot일때 특정 동작을 하기 위해서 param을 저장함, 100개씩 저장하며, 10개의 슬롯까지 가능
	
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 21100 ~ 22000 애니 슬롯이 몇번까지 반복되어야 하는가
	extern void InitRepeatAniSlotBySkillLevel(lwAction kAction);
	// 21100+ (0*10)	 : 0번 애니 슬롯이 몇번 반복 되어야 하는가
	extern void SetMaxRepeatAniSlot(lwAction kAction, int const iAniSlot, int const iRepeatCnt);
	extern int  GetMaxRepeatAniSlot(lwAction kAction, int const iAniSlot);
	// 21100+ (0*10) + 1 : 0번 애니 슬롯이 몇번 반복 했는가
	extern void SetCntRepeatAniSlot(lwAction kAction, int const iAniSlot, int const iRepeatCnt);
	extern int  GetCntRepeatAniSlot(lwAction kAction, int const iAniSlot);

	extern int  IncCntRepeatAniSlot(lwAction kAction, int const iAniSlot);
	extern bool IsDoneRepeatAniSlot(lwAction kAction, int const iAniSlot);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
	// Enter시에 클라에서 추가해 쓰는 연출용 액터의 모든 정보를 읽어온다
	extern void InitDecorationActorInfo(lwActor kCallerActor, lwAction kCallerAction);
	// 30000 DecorationClassNo 
	extern void SetDecorationActor_ClassNo(lwAction kAction, int const iDecoActorClassNo);
	extern int GetDecorationActor_ClassNo(lwAction kAction);
	// 30001 소환자 위치와 이 벡터가 합해져 연출용 액터가 나타날 지점을 정한다
	extern void SetDecorationActor_DistVec(lwAction kAction, lwPoint3 const kDistVec);
	extern lwPoint3 GetDecorationActor_DistVec(lwAction kAction);
	// 30002 연출용 액터가 시작할 액션 DecorationActor Action
	extern void SetDecorationActor_StartActionName(lwAction kAction, char const* pcActionName);
	extern char const* GetDecorationActor_StartActionName(lwAction kAction);
	// 30003 어느 액션 타임에 연출용 액터를 추가 할 것인지 설정한다
	extern void SetAddDecorationActor_ActionTime(lwAction kAction, int const iActionTime);
	extern int GetAddDecorationActor_ActionTime(lwAction kAction);
	//// 30004 시작시에 alpha값을 설정(안보이게 라든지 약간 투명하게 라든지)
	//extern void SetDecorationActor_StartAlpha(lwAction kAction, float const fAlpha);
	//extern float GetDecorationActor_StartAlpha(lwAction kAction);

	// 30005 타입 설정(나한테만 보이게 등)
	extern void SetDecorationActor_Type(lwAction kAction, int const iType);
	extern int GetDecorationActor_Type(lwAction kAction);

	extern bool ProcessDecorationActor(lwActor kActor, lwAction kAction, EActionTime const eActionTime);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
	// 연계콤보
	// 40000 연계콤보 사용 여부
	extern void SetUseCombo(lwAction kAction, bool const bUseCombo);
	extern bool IsUseCombo(lwAction kAction);
	// 40001 연계콤보 키입력
	extern void SetEnableComboKeyEnter(lwAction kAction, bool const bEnterComboKey);
	extern bool IsEnableComboKeyEnter(lwAction kAction);
	// 40002 연계콤보에서 다음 콤보 연계가 확정 되었는지
	extern void SetStartNextCombo(lwAction kAction, bool const bEnterComboKey);
	extern bool IsStartNextCombo(lwAction kAction);
	// 40003 차지 연계콤보 사용 여부
	extern void SetUseChargeCombo(lwAction kAction, bool const bCharge);
	extern bool IsUseChargeCombo(lwAction kAction);
	// 40004 콤보 딜레이 캔슬 여부
	extern void SetComboDelayCancle(lwAction kAction, bool const bCharge);
	extern bool IsComboDelayCancle(lwAction kAction);
	// 40005 카메라 줌인 설정
	extern void ResetZoomCameraCount(lwAction kAction);
	extern void IncreaseZoomCameraCount(lwAction kAction);
	extern int GetZoomCameraCount(lwAction kAction);
	// 40006 스킬 중계 설정
	extern void SetConnectCombo(lwAction kAction, int const iActionNo);
	extern int GetConnectCombo(lwAction kAction);
	// 40007 단축키를 이용한 기본 스킬 사용
	extern void SetSkillUse(lwAction kAction, int const iActionNo);
	extern int GetSkillUse(lwAction kAction);
	//40008 스킬 사용 중 캐릭터 이동 가능(방향전환X)
	extern void SetMoveControl(lwAction kAction, bool const bCharge);
	extern bool IsMoveControl(lwAction kAction);
	//40009 보조 캐릭터 스킬 예약
	extern void SetNextSubPlayerAction(lwAction kAction, int const iNextActionNo);
	extern int  GetNextSubPlayerAction(lwAction kAction);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
	// 100000~110000
	// 100000 CastingComplete 되었을때 붙는 파티클
	// 100100 OnCleanUp때 붙는 파티클

	// 100200 fire 이벤트 메세지 왔을때 붙는 파티클	
	int IncMsgFireCnt(lwAction kAction);// 100299 - 애니메이션 중 hit 메세지가 몇번 들어왔는가? int const IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG	 = 100200;
	int GetMsgFireCnt(lwAction kAction);
	
	// 100300 hit 이벤트 메세지 왔을때 붙는 파티클	
	int IncMsgHitCnt(lwAction kAction); // 100399 - 애니메이션 중 hit 메세지가 몇번 들어왔는가? int const IDX_PARTICLE_SLOT_AT_ON_HIT_MSG	 = 100300;
	int GetMsgHitCnt(lwAction kAction);

	// 100400~100500 effect 이벤트 메세지 들어왔을때 붙는 파티클	
	int IncMsgEffectCnt(lwAction kAction);	// 100499 - 애니메이션 중 effpos 메세지가 몇번 들어왔는가? IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG = 100400	
	int GetMsgEffectCnt(lwAction kAction);
	// 200000~210000 애니슬롯당 파티클 붙이는 연출을 위해 사용

	// ------------------------------------------------------------------
	// 210100~210200: 특수한 피격에 대한 리액션 여부
	// 210100: 트랩 스킬에 공격 받아 리액션하나?
	extern void SetReActionByTrapSkill(lwAction kAction, bool bSetOn);
	extern bool IsReActionByTrapSkill(lwAction kAction);

	
	// luascript에 있는 비효율적인 함수 코드로 옮겨 수정
	char const* GetSummonedHitSound(lwActor kActor);
	void Actor_Mob_Base_DoDefaultMeleeAttack(lwActor kActor,lwAction kAction);
	void Act_Mon_Melee_DoDamage(lwActor kActor,lwAction kAction);
	bool ProjectileHitOneTime(lwProjectile kProjectile);
};
#endif