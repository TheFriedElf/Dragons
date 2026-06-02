#ifndef FREEDOM_DRAGONICA_RENDER_ACTION_PGACTION_H
#define FREEDOM_DRAGONICA_RENDER_ACTION_PGACTION_H

#include "PgIXmlObject.h"
#include "PgActionTargetList.H"

class PgActor;
class PgInputSlotInfo;
class PgActionFSM_Base;

int const ACTIONKEY_ATTACK			=	1045;
int const ACTIONKEY_JUMP			=	1046;
int const ACTIONKEY_USE				=	1044;
int const ACTIONKEY_LEFT			=	1123;
int const ACTIONKEY_RIGHT			=	1124;
int const ACTIONKEY_UP				=	1121;
int const ACTIONKEY_DOWN			=	1126;
int const ACTIONKEY_DOWN_ATTACK		=	3004;
int const ACTIONKEY_UP_ATTACK		=	3003;
int const ACTIONKEY_LEFT_ATTACK		=	3009;
int const ACTIONKEY_RIGHT_ATTACK	=	3008;
int const ACTIONKEY_CHARGE			=	1057;

extern	char const* ACTIONNAME_IDLE;
extern	char const* ACTIONNAME_BIDLE;
extern	char const* ACTIONNAME_RUN;
extern	char const* ACTIONNAME_JUMP;
extern	char const* ACTIONNAME_AWAKE_CHARGE;
extern	char const*ACTIONNAME_RP_IDLE;
extern	char const*ACTIONNAME_RP_WALK;
extern	char const*ACTIONNAME_RP_JUMP;
extern	char const*ACTIONNAME_BLOCK;

extern	char const* ACTIONTYPE_JOBSKILL;

class PgAction : public PgIXmlObject
{
private:
	static	int	ms_iActionInstanceIDGenerator;
	typedef	std::map<std::string,std::string> ScriptParamMap;

public:
	enum TargetAcquireType
	{
		TAT_RAY_DIR=0,	//	vStart 에서 vDir 방향으로 레이를 쏴서 걸리를 놈들을 찾아낸다.
		TAT_RAY_DIR_COLL_PATH=1, // vStart 에서 vDir 방향으로 레이를 쏴서 패스에 충돌하는 위치로부터 일정 영역 안에 있는 놈들을 찾아낸다.
		TAT_SPHERE=2,	//	vStart 에서 반지름 fEffectRadius 크기의 구 안에 있는 놈들을 찾아낸다.
		TAT_BAR,	//	사각통 안에 있는 놈들을 찾아낸다.
	};

	enum	FindTargetOption
	{
		FTO_NONE=0,
		FTO_NORMAL=(1<<0),
		FTO_DOWN=(1<<1),
		FTO_BLOWUP=(1<<2),
		FTO_OBSTACLE=(1<<3),
	};

	class PgSlot
	{
	public:
		typedef std::map<std::string,std::string> CONT_ANIMATIONINFO;
		PgSlot() : m_kSlotName(""), m_bLoop(0), m_bEventScript(0) {}

		std::string m_kSlotName;
		bool m_bLoop;
		bool m_bEventScript;
		CONT_ANIMATIONINFO m_kContAnimationInfo;
	};

	enum	ActionOption
	{
		AO_NONE=0,
		AO_LOOP=(1<<0),	//! 액션을 반복하여 플레이 할지 결정하는 기준
		AO_USE_EVENT=(1<<1),//! 이 액션에 대해 Event 핸들러를 쓸지 안쓸지 체크
		AO_DO_MONITOR=(1<<2),	//!	이 액션에 대한, 입력을 모니터링 할지 안할지
		AO_CAN_CHANGE_ACTOR_POS=(1<<3),	//!	이 액션이 액터의 위치를 변화시키는가
		AO_NO_BROADCAST=(1<<4),	//!	이 액션을 브로드캐스팅 하지 않을것인가
		AO_NO_PLAY_ANIMATION=(1<<5),	//!	이 액션의 애니메이션을 플레이하지 않을것인가
		AO_CALL_CHECK_CAN_ENTER=(1<<6),	//!	액션으로 Enter 할 수 있는지 체크하는 스크립트 함수를 호출 할 것인가?
		AO_IGNORE_DMG_EFFECT=(1<<7), //	이 액션을 플레이하는 중에 대미지를 입더라도 대미지 이펙트를 하지 않을것인가?	
		AO_NO_RANDOM_ANIMATION=(1<<8),	//	랜덤 애니 사용여부
		AO_OVERRIDE_PACKET=(1<<9),
		AO_AUTO_BROADCAST_WHEN_CASTING_COMPLETED=(1<<10),
		AO_DONT_LOOK_TARGET=(1<<11),	// 타겟이 있을 경우, 안쳐다보게 함.
		AO_BIDIRECTION=(1<<12),	// 액션을 실행할 때 좌우만 바라보게 함(일단은 몬스터만 적용),
		AO_BREAKFALL=(1<<13),	//	낙법 사용이 가능한 액션인가
		AO_CAN_FOLLOW=(1<<14),	//	따라가기로 따라할수 있는 액션인가
		AO_NO_SET_WALKING_TARGET_LOC=(1<<15),	// 목표점을 향해 움직일건가
		AO_USE_CFUNC=(1<<16),	//	lua 대신 c 언어 스크립트를 사용할 것인가
		AO_ALWAYS_IGNORE_DMG_EFFECT=(1<<17), //	항상 데미지 이펙트를 하지 않을 것인가? (AO_IGNORE_DMG_EFFECT는 FIRE 상태에서만 Ignore 한다)
		AO_DISABLE_SMALLAREACHECK=(1<<18),	//	액션 중에 SmallArea 체크를 할 것인가
		AO_USE_CFUNC_ONTARGET=(1<<19),	//	타겟팅을 c언어 함수에서 할 것인가
		AO_NO_CHANGE_TELEPORT_ACTOR_POS=(1<<20),	// 위치가 안맞을때 순간이동으로 위치를 맞추는것을 안할것인가
		AO_IGNORE_PHYSX=(1<<21),	// 이동시 PhysX 벽을 무시한다.
		AO_NO_BROADCAST_TARGETLIST=(1<<22),	//	BroadCast 시에 TargetList 는 제외한다.
		AO_ALWAYS_UPDATE=(1<<23),	//	카메라 밖에 있어도 업데이트를 정상적으로 한다.
		AO_KIND_OF_TRIGGER_ACTION=(1<<24),	//	맵 트리거용 액션이다
		AO_IGNORE_NOTACTIONSHIFT=(1<<25),	//	액션 전이 막는 어빌이 걸려도 액션이 진행될 수 있도록 한다.
	};

	struct	stActionTimer
	{
		float	m_fStartTime;
		float	m_fTotalDuration;
		float	m_fInterval;
		int	m_iLastCallTime;
		int	m_iTimerID;
		stActionTimer()
		{
			memset(this,0,sizeof(stActionTimer));
		}
	};

	typedef std::vector<PgSlot> SlotContainer;
	typedef std::map<unsigned int, std::string> ParamContainer;
	typedef std::list<BM::GUID> GUIDContainer;
	typedef std::map<int, NiPoint3> PointContainer;
	typedef std::list<stActionTimer> TimerCont;
	
	//! 생성자
public:
	PgAction();
	PgAction(PgAction *pkSrcAction);
	virtual ~PgAction();
	bool Init(const PgAction *pkSrcAction);
	void Clear();

	void	CopyFrom(PgAction *pkSrcAction);
	
	PgActionTargetList*	GetTargetList()	{	return	&m_ActionTargetList;	}

	static	int	GenerateNextActionInstanceID()	{	return	ms_iActionInstanceIDGenerator++;	}
	void	ClearTargetList();
	void	AddTarget(BM::GUID const &kGUID,int iABVIndex, PgActionResult* pkActionResult);
	void	SetTargetActionResult(PgActionResultVector* pkActionResult, bool bDoNotSetEffect);
	void	AddNullTarget(BM::GUID const &kGUID,int iABVIndex);
	void	SetTargetList(PgActionTargetList& kTargetList);
	void	SetActionParam(BYTE byParam);
	BYTE	GetActionParam()const{	return	m_byActionParam;	}
	int	GetActionNo() const {	return	m_iActionNo;	}
	int GetEffectNo() const {	return	m_iEffectNo;	}

	ESkillType GetSkillType()	{	return	m_SkillType;	}
	void	SetActionNo(int iActionNo);

	bool	GetActionFinished()	const	{	return	m_bActionFinished;	}
	void	SetActionFinished(bool bFinished)	{	m_bActionFinished = bFinished;	}

	void	CopyParamTo(PgAction *pkTarget);

	void	SetInputSlotInfo(PgInputSlotInfo *pkInputSlotInfo);
	const	PgInputSlotInfo *GetInputSlotInfo()	const;

	void	SetTimeStamp(DWORD dwTimeStamp)	{	m_dwTimeStamp	=	dwTimeStamp;	}
	DWORD	GetTimeStamp()	{	return	m_dwTimeStamp;	}

	bool IsRecord();
	void SetRecord(bool bRecord);

	BYTE GetDirection() const;
	void SetDirection(BYTE byDir);

	static	int	FindTargets(int iActionNo,TargetAcquireType kType,NiPoint3 kStart,NiPoint3 kDir,float fRange,float fWideAngle,float fEffectRadius, PgActionTargetList* pkIgnoreTargets,int iMaxTargets, bool bNoDuplication, PgActionTargetList& kout_FoundTargets, PgPilot* pkCasterPilot = NULL, bool const bAddDisplayInfo = true);
	static	bool	IsCorrectTargetWithTargetType(int iActionNo,PgActor* pkTargetActor, PgActor* pkCasterActor = NULL);

	int		OnFindTarget(PgActor* pkActor, PgActionTargetList* pkTargetList);

	bool	CheckTargetExist(PgActor* pkActor,char const* kActionName);
	static	bool CheckCanEnter(PgActor* pkActor,char const* kActionName, bool const bShowFailMsg);

	void	CreateActionTargetList(PgActor* pkActor,PgAction* pkAction=NULL,bool const bNoCalcActionResult = false, bool const bForcedCreation = false);
	int	Call_Skill_Result_Func(PgPilot* pkCasterPilot,UNIT_PTR_ARRAY *pkTargetArray,PgActionResultVector *pkActionResultVec);

	//	Timer
	bool	StartTimer(float const fTotalDuration, float const fInterval, int const iTimerID);
	void	DeleteTimer(int const iTimerID);
	void	UpdateTimer(PgActor* pkActor,float fAccumTime);
	void	ClearTimer();

	//	Action Instance ID
	void	SetActionInstanceID();
	void	SetActionInstanceID(int const iInstanceID);
	int		GetActionInstanceID() const;

	float	GetActionEnterTime() const	{	return	m_fActionEnterTime;	}//	액션이 시작된 시간을 리턴한다.
	DWORD	GetElapsedTime() const;

	//	XML Parsing
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);//! Xml을 파싱한다.

	//	Script Call Functions
	bool CheckCanEnter		(PgActor* pkActor, PgAction* pkAction);//!	이 액션으로 진입할 수 있는지 체크한다.
	void OnCastingCompleted	(PgActor* pkActor, PgAction* pkAction);//!	캐스팅이 끝났을 경우 호출된다.
	bool EnterFSM			(PgActor* pkActor, PgAction* pkAction);//! FSM으로 진입한다.
	bool LeaveFSM			(PgActor* pkActor, PgAction* pkAction,bool bCancel=false);//! FSM에서 탈출한다.
	bool UpdateFSM			(PgActor* pkActor, float fAccumTime, float fFrameTime);//! FSM 업데이트 한다.
	void FindTargetFSM		(PgActor* pkActor, PgAction* pkAction, PgActionTargetList& kTargetList);	//! 타겟을 찾아서 kTargetList에 집어넣는 스크립트를 호출한다.
	void CleanUpFSM			(PgActor* pkActor, PgAction* pkAction);	//! 액션에 사용되었던 리소스들을 모두 해제시킨다
	bool EventFSM			(PgActor* pkActor, char const* pcTextKey, NiActorManager::SequenceID seqID);
	void OnTargetListModified(PgActor* pkActor,bool bIsBefore);

	std::string &GetNextActionName();//! 지정된 다음 엑션 이름을 반환한다.
	void SetNextActionName(char const* pcActionName);//! 다음 엑션 이름을 지정한다.

	void ModifyTargetList(PgActionTargetList& kTargetList);
	bool ModifyActionResult(PgActor* pkActor, PgActionTargetList& kTargetList, int iTimeStamp);//! FSM으로 Animation Event를 넘긴다.
	
	void SetNextActionInputSlotInfo(PgInputSlotInfo* pkInputSklotInfo);
	PgInputSlotInfo const* GetNextActionInputSlotInfo() const;

	void	ChangeToNextActionOnNextUpdate(bool bChange = true);	//	다음업데이트시에 자동으로 NextAction  으로 전이합니다.
	bool	IsChangeToNextActionOnNextUpdate()	const	{	return	m_bChangeToNextActionOnNextUpdate;	}	//	다음업데이트시에 자동으로 NextAction  으로 전이합니다.

	std::string &GetActionType();

	int GetCount();//! Action의 개수를 반환한다.
	
	bool GetActionName(std::string& rkActionName_out,int iSlotNum = -1);//! Get Action Name

	bool IsSlotLoop()const;//! Get Looping for Slot
	bool IsSlotEventScript()const;
	bool GetAnimationInfo(int const iSlotNum, std::string const& rkInfoName, std::string & rkOut)const;
	
	std::string &GetFSMScript();//! Get FSM Scripting Function's Prefix
	PgActionFSM_Base*	GetFSMFunc()	{	return	m_pkFSMFunc;	}

	
	bool GetEnable();//! 이 액션에 대해서, 키가 눌러졌는지 떼어졌는지 체크 할 수 있다.
	void SetEnable(bool bEnable);//! 이 액션에 대해서 키가 눌러졌는지 떼어 졌는지 설정할 수 있다.
	
	//	Action Slot
	int GetCurrentSlot() const;//! 현재 진행중인 Action의 슬롯 번호를 반환한다.
	bool SetSlot(unsigned int iSlot);//! 다음 진행할 Action의 슬롯 번호를 셋팅한다.
	int GetSlotCount() const;//! Action의 슬롯 개수를 가져온다.
	bool NextSlot();//! Action의 다음 Slot에 대한 진행을 시도한다.
	bool PrevSlot();//! Action의 다음 Slot에 대한 진행을 시도한다.

	//	Action Param
	char const* GetScriptParam(std::string const& kParamName) const;
	bool SetParam(unsigned int uiIndex, char const* pcParam);	//! 파라메터를 설정
	char const* GetParam(unsigned int uiIndex) const;//! 파라메터를 가져옴
	BM::Stream* GetParamAsPacket() const;	// 파라메터로 설정된 패킷을 가져온다.
	void DeleteParamPacket();
	void SetParamAsPacket(BM::Stream* pkPacket, bool const bPrevPacketDelete = true);	// 패킷을 파라메터로 설정한다.
	NiPoint3 const &GetParamAsPoint(int const iIndex) const;
	void SetParamAsPoint(int const iIndex, NiPoint3 &kPoint);

	PgActor *FindEnemy(float fRange);//! 앞의 적 타겟을 찾는다.

	//	GUID Container
	void	ClearGUIDContainer();//! GUID 컨테이너를 클리어한다.
	void	AddNewGUID(BM::GUID const &newGUID);//! GUID 컨테이너에 GUID 를 추가한다.
	void	RemoveGUID(BM::GUID const  &newGUID);//! GUID 컨테이너에서 하나를 삭제한다.
	bool	CheckGUIDExist(BM::GUID const &GUID);//! GUID 컨테이너에 GUID 가 존재하는지 체크한다.
	int	GetTotalGUIDInContainer() const;
	BM::GUID const	*GetGUIDInContainer(int iIndex);
	GUIDContainer*	GetGUIDContainer()	{	return	&m_GuidContainer;	}

	//	BroadCast
	void	BroadCast(PgPilot *pkPilot);	//	서버로 전송하여 다른 유저에게 브로드캐스트 하도록 한다.
	void	BroadCastTargetListModify(PgPilot *pkPilot, bool bisTargetEmptyToBroadCast = false);	//	타겟 리스트가 바뀌었음을 서버에 알린다. // bisTargetEmptyToBroadCast 타겟이 비어 있어도 보내도록

	void	SetActionStartPos(const	NiPoint3 &kPos)	{	m_kActionStartPos = kPos;	}
	const	NiPoint3	&GetActionStartPos() const	{	return	m_kActionStartPos;	}

	void	SetSyncStartPos(NiPoint3 &kPos)	{	m_kSyncStartPos = kPos;	}
	NiPoint3&	GetSyncStartPos()			{	return	m_kSyncStartPos;	}

	void	EnableSyncRun(bool const bEnable)	{	m_bSyncRunEnabled = bEnable;	}
	bool	IsSyncRunEnabled()	const			{	return	m_bSyncRunEnabled;	}

	void SetActionTerm(DWORD dwActionTerm)	{ m_dwActionTerm = dwActionTerm; }
	DWORD GetActionTerm()	{ return m_dwActionTerm; }

	const	CSkillDef*	GetSkillDef() const {	return	m_pkSkillDef;	}
	int	GetAbil(int const iAbilType) const;

	int	GetNextProjectileUID()	{	return	m_iProjectileUID++;	}

	int		GetCurrentStage() const;
	void	NextStage();
	void	OverridePacket(PgActor* pkActor, BM::Stream* pkPacket);
	
#ifdef PG_SYNC_ENTIRE_TIME
	void SetBirthTime(DWORD dwBirthTime);
	DWORD GetBirthTime();
#endif

	bool AlreadySync();
	void AlreadySync(bool bSync);

	void	SetAddToActionEntity(bool bAdd)	{	m_bAddToActionEntity = bAdd;	}
	bool	GetAddToActionEntity()	{	return	m_bAddToActionEntity;	}

	void	SetActionOption(ActionOption kOption,bool bEnable)
	{
		int	iActionOption = (int)m_ActionOption;
		bEnable ? iActionOption|=kOption : iActionOption &= ~kOption;
		m_ActionOption = (ActionOption)iActionOption;
		
	}
	bool	GetActionOptionEnable(ActionOption kOption)	const
	{
		int	iActionOption = (int)m_ActionOption;
		return	(iActionOption&kOption)!=0;
	}

	ActionOption	GetActionOption()	const	{	return	m_ActionOption;	}
	void	SetActionOption(ActionOption	kOption)	{	m_ActionOption = kOption;	}

	// 스킬의 추가적인 쿨 타임 / 캐스팅 타임
	void SetSkillCoolTime(int const iAddCoolTime);
	void SetSkillCoolTimeRate(float const fAddCoolTimeRate);
	void SetSkillCastingTime(int const iAddCastingTime);

	int	 GetSkillCoolTime() const { return m_iSkillCoolTime; }
	float GetSkillCoolTimeRate() const { return m_fSkillCoolTimeRate; }
	int  GetSkillCastingTime() const { return m_iSkillCastingTime; }

	void	SetEffectCasterGuid(BM::GUID const &kGuid)	{	m_kEffectCasterGuid = kGuid;	}
	BM::GUID	GetEffectCasterGuid()	const	{	return	m_kEffectCasterGuid;	}

	void SetThrowRayStatus(bool bStatus) {bRayStatus = bStatus;}
	bool GetThrowRayStatus() const { return bRayStatus; }
	std::string const GetActionType() const { return m_kActionType; }

	static PgAction::TargetAcquireType ServerTypeToTargetAcquireType(ESkillArea eType);
	CLASS_DECLARATION_S(DWORD, StartRandomSeedCallCounter);

	int GetTranslateSkillCnt(); // 전이할수 있는 스킬들의 갯수(4차스킬 중 커맨드 입력으로, 나갈수 있는 스킬들의 갯수)

protected:
	int		OnFindTargetFunc(PgActor* pkActor, PgAction* pkAction, PgActionTargetList& rkTargetList);	//스크립트는 루아를 쓰지만 타겟찾는건 소스코드에서 할때
	void SetTranslageSkillCnt(int const iCnt) { m_iTranslateSkillCnt = iCnt; }

protected:
	PgActionTargetList m_ActionTargetList;

	int	m_iActionInstanceID;
	BYTE m_byActionParam;
	int	m_iActionNo;
	int m_iEffectNo;

	BM::GUID	m_kEffectCasterGuid;	//	이 액션이 ActionEffect 일 경우, Caster 의 Guid
	ESkillType m_SkillType;

	DWORD m_dwTimeStamp;

	float m_fActionEnterTime;	//	OnEnter 스크립트가 호출된 시간.(즉 액션이 시작된 시간, 단위 sec)

	bool m_bActionFinished;

	std::string m_kNextActionName;//! 다음 액션 이름(현재 액션이 종료 되면, 이 액션으로 자동 전이한다)
	PgInputSlotInfo	*m_pkNextActionInputSlotInfo;

	GUIDContainer m_GuidContainer;//!	guid 를 저장한다.
	std::string m_kActionType;//! 액션 타입
	SlotContainer m_kSlotContainer;//! Slot Container (Ani에 대한 Slot이 들어있다)
	ParamContainer m_kParamContainer;//! Parameter Container
	PointContainer m_kPointContainer;

	PgActionFSM_Base*	m_pkFSMFunc;	//	lua 대신 C 형태의FSM 함수
	std::string m_kFSMScript;//! Script
	ScriptParamMap	m_ScriptParamMap;	//	스크립트 실행시 참조될 파라메터들

	bool m_bEnable;//! 이 액션에 대해 진입인지 탈출인지 알려준다.

	bool m_bChangeToNextActionOnNextUpdate;
	unsigned int m_uiCurrentSlot;//! 현재 플레이 중인 애니메이션의 슬롯 번호

	ActionOption	m_ActionOption;

	PgInputSlotInfo	*m_pkInputSlotInfo;	//	이 액션이 키입력에 의해 생성된 액션일 경우, 입력키 정보

	NiPoint3 m_kActionStartPos;	//	이 액션을 플레이할 시작위치
	NiPoint3 m_kSyncStartPos;	
	bool m_bSyncRunEnabled;
	BM::Stream *m_pkParamPacket;
	BYTE m_byMovingDirection;
	bool m_bRecord;
	bool m_bAlreadySync;
	DWORD m_dwBirthTime;
	DWORD m_dwActionTerm;
	int m_iStage;

	bool	m_bAddToActionEntity;

	int	m_iProjectileUID;	//	이 액션에 의해 생성되는 발사체들에게 할당되는 발사체 Unique ID

	TimerCont	m_TimerCont;

	CSkillDef const* m_pkSkillDef;
	int m_iSkillCoolTime;		//스킬의 추가적인 쿨 타임
	float m_fSkillCoolTimeRate; //스킬의 추가적인 쿨 타임(상대값)
	int m_iSkillCastingTime;	//스킬의 추가적인 캐스팅 타임

	bool bRayStatus;
	
	int m_iTranslateSkillCnt;	// 전이할수 있는 스킬들의 갯수(4차스킬 중 커맨드 입력으로, 나갈수 있는 스킬들의 갯수)
};

#endif // FREEDOM_DRAGONICA_RENDER_ACTION_PGACTION_H