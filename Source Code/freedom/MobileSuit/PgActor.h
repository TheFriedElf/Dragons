#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTOR_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTOR_H

#include "PgIWorldObject.h"
#include "BM/ClassSupport.h"
#include "PgItemEx.h"
#include "PgWeaponSoundMan.h"
#include "PgTrail.H"
#include "PgStatusEffect.H"
#include "Variant/PgQuestInfo.h"
#include "Lohengrin/PacketStruct4Map.H"
#include "PgGuildMark.h"
#include "PgGuardianMark.h"
#include "PgMarkBalloon.h"
#include "PgTextObject.H"
#include "PgAchievementTitle.h"
#include "PgMyhomeMark.h"
#include "PgVendorBalloon.h"
#include "PgPOTParticle.h"
#include "PgDuelTitle.h"
#include "PgEffectCountDown.h"

class PgInput;
class PgAction;
class PgParticle;
class PgTrigger;
class PgChainAttack;
class PgTextBalloon;
class Pg2DString;
class PgSkillTargetMan;
class ShadowGeometry;
class PgActionEffect;
class PgActionEffectStack;
class PgEnergyGauge;
class PgDropBox;
class PgHeadBuffIconList;
class PgPartyBalloon;
class PgExpeditionBalloon;
class PgVendorBalloon;
class ShadowObject;
class PgSpotLight;
class PgObject;
class PgActionTargetInfo;

#define PG_USE_CAPSULE_CONTROLLER

#define PG_ROTATION_INTERPOL_SPEED	10.0f			// 2x (0.5 sec)
#define PG_PATHWALL_DIST_LIMIT		300.0f			// 패스(벽)을 찾는 최대거리
#define PG_PATHWALL_SPAN_MAX		200.0f			// 패스(벽)과 캐릭터 사이의 최대 공간
#define PG_PATHWALL_SPAN_MIN		45.0f			// 패스(벽)과 캐릭터 사이의 최소 공간
#define PG_LIMITED_ANGLE			45.0f

#define	PG_SYNC_DIST_WARP	15
#define	PG_SYNC_DIST_SLIDE	150

typedef enum
{
	PG_PHYSX_GROUP_BASE_FLOOR = 0,
	PG_PHYSX_GROUP_BASE_WALL = 1,
	PG_PHYSX_GROUP_PLAYER_WALL = 9,
	PG_PHYSX_GROUP_MONSTER_WALL = 10,
	PG_PHYSX_GROUP_OBSTACLE = 4,
	PG_PHYSX_GROUP_FILTERING_WALL = 5,
	PG_PHYSX_GROUP_PUPPET = (OGT_PUPPET+1),
	PG_PHYSX_GROUP_OBJECT = (OGT_OBJECT+1),
	PG_PHYSX_GROUP_FURNITURE = (OGT_FURNITURE+1),
	PG_PHYSX_GROUP_TRIGGER,
	PG_PHYSX_GROUP_SHINESTONE,
	PG_PHYSX_GROUP_MONSTER
} PhysXGroup;

const static NxU32 DEFAULT_ACTIVE_GRP = (1 << PG_PHYSX_GROUP_BASE_FLOOR) | (1 << PG_PHYSX_GROUP_BASE_WALL) | 
(1 << PG_PHYSX_GROUP_OBJECT) | (1<<PG_PHYSX_GROUP_PLAYER_WALL) |
(1 << PG_PHYSX_GROUP_PUPPET) | (1 << PG_PHYSX_GROUP_FURNITURE | (1 << PG_PHYSX_GROUP_MONSTER));	// 마그마에서 떨어지는 벽 그룹 번호.

const static NxU32 MONSTER_ACTIVE_GRP = (1 << PG_PHYSX_GROUP_BASE_FLOOR) | (1 << PG_PHYSX_GROUP_BASE_WALL) | 
(1 << PG_PHYSX_GROUP_OBJECT) | (1 << PG_PHYSX_GROUP_MONSTER_WALL) | 
(1 << PG_PHYSX_GROUP_PUPPET) | (1 << PG_PHYSX_GROUP_FURNITURE);	// 마그마에서 떨어지는 벽 그룹 번호.

//class PgIActionEntity
//{
//public:
//	virtual ~PgIActionEntity();
//	virtual DWORD GetActionTerm() = 0;
//	virtual void ProcessAction(PgActor *pkActor) = 0;
//};
//
//class PgActionEntity : public PgIActionEntity
//{
//public:
//	PgActionEntity(PgAction *pkAction);
//	virtual ~PgActionEntity();
//	
//
//	void ProcessAction(PgActor *pkActor);
//	DWORD GetActionTerm();
//
//protected:
//	PgAction *m_pkAction;
//};
//
//class PgDirectionEntity : public PgIActionEntity
//{
//public:
//	PgDirectionEntity(BYTE byDirection, DWORD dwActionTerm);
//	virtual ~PgDirectionEntity();
//	
//	void ProcessAction(PgActor *pkActor);
//	DWORD GetActionTerm();
//	
//protected:
//	BYTE m_byDirection;
//	DWORD m_dwActionTerm;
//};

std::string const kSAT_Idle = "IDLE";
std::string const kSAT_Click = "CLICK";
std::string const kSAT_Talk = "TALK";
std::string const kSAT_Warning = "WARNING";
std::string const kSAT_TextDialogs = "TEXT_DIALOGS";
typedef enum eSayActionStatus
{
	SAS_None = 0,
	SAS_Wait = 1,
	SAS_Run,
	SAS_Delay,
} ESayActionStatus;
typedef enum eSayActionType
{
	SAT_None = 0,
	SAT_Idle,
	SAT_Click,
	SAT_Talk,
	SAT_Warning,
	SAT_TextDialogs,
} ESayActionType;

//
class PgSayItemChecker
{
public:
	PgSayItemChecker(int const iPlayerLevel, const PgMyQuest *pkMyQuest);
	~PgSayItemChecker();

	bool operator ()(const ContSayItem::value_type& rkItem) const;

private:
	int const m_iPlayerLevel;
	const PgMyQuest *m_pkMyQuest;
};

//
class PgActionSay
{
public:
	PgActionSay() {Clear();};
	~PgActionSay() {};

	void Clear();

	bool AddSay(const SSayItem &rkItem);
	bool GetCur(SSayItem &kItem) const;

	ESayActionStatus Update(float fAccumTime, float fFrameTime);
	ESayActionStatus OnClick();
	ESayActionStatus OnTalk();
	ESayActionStatus OnWarning();

	size_t const TalkCount() const;
	size_t const WarningCount() const;

protected:
	bool GetRandomSay(ContSayItem const & rkCont, SSayItem const *pkPrev, const SSayItem* &pkOut)const;

	void SayFilter(ContSayItem& rkCont) const;

	//
	CLASS_DECLARATION(float, m_kfUpTime, UpTime);
	CLASS_DECLARATION_NO_SET(ESayActionStatus, m_kCurSayStatus, CurSayStatus);
	CLASS_DECLARATION_NO_SET(int, m_iTextDialogsID, TextDialogsID);
protected:
	//말풍선
	ContSayItem m_kIdleSayList;
	ContSayItem m_kClickSayList;

	//2D Face 대화창
	ContSayItem m_kTalkSayList;

	//입간판
	ContSayItem m_kWarningList;

	const SSayItem* m_pkCurSayItem;
};

//
class PgActionEntity
{
public:
	PgActionEntity(PgAction *pkAction, BYTE byDirection);
	PgAction *GetAction() const;
	void	SetAction(PgAction *pkAction)
	{
		m_pkAction = pkAction;
	}
	
	DWORD GetActionTerm();
	
	BYTE GetDirection()	;
	void SetDirectionTerm(DWORD dwTerm);
	void SetDirectionStartPos(NiPoint3 &rkPos);
	NiPoint3 &GetDirectionStartPos();

	PgActionEntity	CreateCopy();

private:
	PgAction *m_pkAction;
	BYTE m_byDirection;
	DWORD m_dwDirectionTerm;
	NiPoint3 m_kDirectionStartPos;
};

struct	stActorDropItemInfo
{
	BM::GUID	m_kItemGUID;
	int	m_iActionInstanceID;
};
typedef std::list<stActorDropItemInfo> ActorDropItemInfoList;

struct	stActorItemPickupInfo
{
	BM::GUID	m_kItemGUID;
	float	m_fItemPickUpTime;
};
typedef std::map<BM::GUID,stActorItemPickupInfo> ActorPickUpInfoCont;

class	PgActorAppearanceMan;


class	PgSweepHitCallBack	:	public	NxUserEntityReport<NxSweepQueryHit>
{
public:
	virtual bool onEvent(NxU32 nbEntities, NxSweepQueryHit* entities);
};

class	PgControllerHitCallBack	:	public	NxUserControllerHitReport
{
	PgActor	*m_pkParentActor;

public:

	PgControllerHitCallBack():m_pkParentActor(0)
	{
	};
	PgControllerHitCallBack(PgActor *pkActor):m_pkParentActor(pkActor)
	{
	};
	void	SetParentActor(PgActor *pkActor)	{	m_pkParentActor = pkActor;	}

	virtual NxControllerAction  onShapeHit(const NxControllerShapeHit& hit);
	virtual NxControllerAction  onControllerHit(const NxControllersHit& hit);
};

typedef struct tagUIModelOpt
{
	tagUIModelOpt() : fDrawHeight(0.0f), fIncreaseCamRad(0.0f) {}
	void Clear()
	{
		fDrawHeight = 0.0f;
		fIncreaseCamRad = 0.0f;
	}
	float fDrawHeight;
	float fIncreaseCamRad;
}SUIMODEL_OPT;

typedef enum eItemDivideReservedType
{
	EIDRT_NONE					= 0,
	EIDRT_JOBSKILL3_CREATE		= 1,
} EItemDivideReservedType;

typedef enum eSkillSetResult
{
	ESSR_NONE = 0,
	ESSR_NOT_DEFSKILLSET,
	ESSR_NOT_SKILLSET,
	ESSR_NOT_FIND_DEFSKILLSET,
	ESSR_NOT_CLASS,
	ESSR_NOT_USE_ITEM,
	ESSR_ETC,
}ESkillSetResult;

class PgSkillSetAction
{
public:
	typedef enum eSTATE : BYTE
	{
		ES_NONE			= 0,
		ES_NEXTACTION	= 1,
		ES_READACTION	= 2,
	}E_STATE;

	typedef std::vector<std::wstring> CONT_SKILLTEXT;

	typedef struct tagReserveInfo
	{
		int iSkillNo;
		BYTE byDelay;
		float fAccumDelay;
		tagReserveInfo(int const skillno=0, BYTE const delay=0):iSkillNo(skillno),byDelay(delay),fAccumDelay(0.f) {}
	} SReserveInfo;

	PgSkillSetAction();

	ESkillSetResult CheckCanSkillSetDoAction(PgPlayer const * pkPlayer, BYTE const iSetNo);
	bool IsReservedNextAction(bool const bIgnoreTime=false);
	bool IsReserveActionEmpty();
	bool GetReservedAction(std::wstring & rkNextActionName, PgActor* pkActor, bool const bIgnoreTime=false);
	bool GetReservedAction(int & iNextActionNo, PgActor* pkActor, bool const bIgnoreTime=false);
	bool NextReservedAction(PgActor* pkActor);
	bool DoReservedAction(PgActor* pkActor);
	bool ReserveAction(BYTE const iSetNo);
	bool ReserveActionCancel();
	void ContSkillSet(CONT_USER_SKILLSET const & kContSkillSet);
	bool GetMaxCoolTime(PgActor * pkActor, int const iSetNo, DWORD & dwMaxRemainTime);
	bool GetSkillText(int const iSetNo, CONT_SKILLTEXT & rkContSkillText);
	CONT_USER_SKILLSET::mapped_type const * find(BYTE const bySetNo);

private:
	typedef std::deque<SReserveInfo> CONT_RESERVSKILL;
	BYTE m_byReserveSetNo;
	E_STATE m_eState;
	CONT_RESERVSKILL m_kContReserveSkill;
	CONT_USER_SKILLSET m_kContSkillSet;
	CONT_USER_SKILLSET m_kContBasicSkillSet;//기초스킬세트
public:
	void ContBasicSkillSet(CONT_USER_SKILLSET const & kContBasicSkillSet);
	CONT_USER_SKILLSET& GetBasicSkillSet();
	CONT_USER_SKILLSET& GetSkillSet();
	void ApplyBasicSkillSetToSkillSet();
};

enum eEquipShowPriority
{
	EESP_NONE = 0,
	EESP_DEFAULT = 1,
	EESP_NORMAL_FIT = 2,
	EESP_CASH_FIT = 3,
	EESP_CASH_SET_EFFECT_FIT = 4,
	EESP_NORMAL_SET_EFFECT_FIT = 5,
	EESP_EFFECT_FIT = 6,
};


class PgActor : public PgIWorldObject
{
	typedef std::vector<NiColor> CONT_NI_COLOR;
	NiDeclareRTTI;
protected:
	//! Call back object
	class ActorCallbackObject : public NiActorManager::CallbackObject
	{
	public:
		virtual void AnimActivated(NiActorManager* pkManager,
			NiActorManager::SequenceID eSequenceID,float fCurrentTime,
			float fEventTime){}
		virtual void AnimDeactivated(NiActorManager* pkManager,
			NiActorManager::SequenceID eSequenceID,float fCurrentTime,
			float fEventTime){}
		virtual void TextKeyEvent(NiActorManager* pkManager,
			NiActorManager::SequenceID eSequenceID, 
			const NiFixedString& kTextKey,
			const NiTextKeyMatch* pkMatchObject,
			float fCurrentTime, float fEventTime);
		virtual void EndOfSequence(NiActorManager* pkManager,
			NiActorManager::SequenceID eSequenceID,
			float fCurrentTime,
			float fEventTime){}
		virtual ~ActorCallbackObject();

		PgIWorldObject *m_pkWorldObject;
		std::string m_kScriptName;

		NiActorManager::SequenceID	m_kLastEventSequenceID;
		float m_fLastEventTime;
		int	m_iLastEventIndex;
		std::string m_kLastEventName;
	};

	typedef struct _AttachItemInfo
	{
		int iItemNo;
		bool bSetDefaultItem;
		int iClass;
		int iGender;
		PgItemEx* pItem;
		_AttachItemInfo() { iItemNo = 0; bSetDefaultItem = false; iClass = 0; iGender = 0; pItem = NULL; }
	} AttachItemInfo;

	typedef struct _TextureInfo
	{
		NiTexturingProperty	*pTexturing;
		NiTexturingProperty::MapEnum kMapEnum;
		NiTexturePtr spTexture;

		_TextureInfo()
			:kMapEnum(NiTexturingProperty::BASE_INDEX),
			pTexturing(NULL),
			spTexture(NULL)
		{
		}
	} TextureInfo;

	class PgControllerHitReport : public NxUserControllerHitReport
	{
	public:
		virtual NxControllerAction  onShapeHit(const NxControllerShapeHit& hit) { return NX_ACTION_NONE; }
		virtual NxControllerAction  onControllerHit(const NxControllersHit& hit);
	};

	typedef	std::list<BM::GUID> GUIDCont;
	typedef std::map<BM::GUID, BM::Stream> CONT_ITEM_DIVIDERESERVED;
	typedef std::map<BM::GUID, PgAction*> CONT_TEMP_ACTION;

	struct	stFollowInfo
	{

		enum	FollowState
		{
			FS_NONE=0,
			FS_MOVE_TO_STARTPOS,
			FS_WAIT,
			FS_PROCESS_ACTION_QUEUE,
		};

		FollowState	m_kFollowState;

		bool	m_bTargetLost;

		BM::GUID m_kFollowTargetActor;	//	내가 쫒아가는 타겟액터
		GUIDCont	m_kFollowingMeActorCont;	//	나를 쫒아 오는 액터들

		bool	m_bFollowFirstActionAdded;
		DWORD	m_dwSavedActionTerm;

		Direction	m_kSavedDirection;

		//NiPoint3	m_kSavePoint;			//펫용. 특정 액션시작지점을 저장해서 써야할 때.(점프)

		float	m_fTargetLostStartTime;

		int	m_iRequestCount;

		stFollowInfo():
			m_dwSavedActionTerm(0),
			m_kSavedDirection(DIR_NONE),
			m_bFollowFirstActionAdded(false),
			m_kFollowState(FS_NONE),
			m_fTargetLostStartTime(0),
			m_bTargetLost(false),
			m_iRequestCount(0)
		{
		}
		~stFollowInfo()
		{
		}

		bool	IsTargetLost()	{	return	m_bTargetLost;	}
		void	SetTargetLost(bool bLost)	{	m_bTargetLost = bLost;	}

		float	GetTargetLostStartTime()	{	return	m_fTargetLostStartTime;	}
		void	SetTargetLostStartTime(float fTime)	{	m_fTargetLostStartTime=fTime;	}

		void	SaveDirection(Direction kDirection)	{	m_kSavedDirection = kDirection;	}
		Direction GetSavedDirection()	{	return	m_kSavedDirection;	}

		void	SetStartFollow(BM::GUID const &kTargetActor)
		{
			m_dwSavedActionTerm = 0;
			m_kFollowTargetActor = kTargetActor;
			m_bFollowFirstActionAdded = false;
			m_kFollowState = stFollowInfo::FS_NONE;

			SetTargetLost(false);
		}
		void	SetStopFollow()
		{
			m_kFollowTargetActor.Clear();
			m_kFollowState = stFollowInfo::FS_NONE;
		}

		bool	FindFollowingMeActor(const	BM::GUID &kActorGUID)
		{
			for(GUIDCont::iterator itor = m_kFollowingMeActorCont.begin();
				itor != m_kFollowingMeActorCont.end(); 
				itor++)
			{
				BM::GUID kGUID = *itor;

				if(kGUID == kActorGUID)
				{
					return true;
				}
			}
			return false;
		}

		void	AddFollowingMeActor(const	BM::GUID &kActorGUID)	//	나를 따라오는 액터를 추가한다. FollowActor() 에서 자동으로 호출된다.
		{
			//	이미 있는 놈이면 그냥 리턴한다.
			for(GUIDCont::iterator itor = m_kFollowingMeActorCont.begin();
				itor != m_kFollowingMeActorCont.end(); 
				itor++)
			{
				BM::GUID kGUID = *itor;

				if(kGUID == kActorGUID)
				{
					return;
				}
			}


			m_kFollowingMeActorCont.push_back(kActorGUID);
		}
		void	RemoveFollowingMeActor(const	BM::GUID &kActorGUID)	//	나를 따라오는 액터를 제거한다. StopFollowActor() 에서 자동으로 호출된다.
		{

			for(GUIDCont::iterator itor = m_kFollowingMeActorCont.begin(); 
				itor != m_kFollowingMeActorCont.end(); 
				itor++)
			{
				BM::GUID kGUID = *itor;

				if(kGUID == kActorGUID)
				{
					m_kFollowingMeActorCont.erase(itor);
					return;
				}
			}
		}

		FollowState	GetFollowState() const { return m_kFollowState; }
		void		SetFollowState(FollowState kState) { m_kFollowState = kState; }

		void	SaveActionTerm(int iActionTerm)
		{
			m_dwSavedActionTerm += iActionTerm;
		}
		void	SetSavedActionTerm(int iNewActionTerm)	{	m_dwSavedActionTerm = iNewActionTerm;	}
		DWORD	GetSavedActionTerm()	{	return	m_dwSavedActionTerm;	}


	};

	class ItemDesc
	{
	public:
		ItemDesc(eEquipLimit kItemPos);
		ItemDesc(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const& kEnchantInfo);

		eEquipLimit m_kItemPos;
		int m_iItemNo;
		SEnchantInfo m_kEnchantInfo;

		bool operator ==(const ItemDesc &rhs);
	};

	class AMPair
	{
	public:
		AMPair(eEquipLimit kItemPos, NiActorManagerPtr spAM,PgItemEx::stCustomAniIDChangeSetting *pstCustomAniIDChangeSetting = NULL);

		eEquipLimit m_kItemPos;
		NiActorManagerPtr m_spAM;
		PgItemEx::stCustomAniIDChangeSetting m_stCustomAniIDChangeSetting;

		bool operator ==(const AMPair &rhs);
	};

	struct	stPush
	{
		bool	m_bActivated;
		bool	m_bLeft;
		float	m_fDistance;
		float	m_fVelocity;
		float	m_fAccel;
		float	m_fOriginDistance;
		NiPoint3	m_kDir;
		stPush()
		{
			memset(this,0,sizeof(stPush));
		}
	};

	typedef struct tagNpcMarkInfo
	{
		bool IsUse;
		std::string kEffectID;
		std::string kTargetNodeName;
		tagNpcMarkInfo():
		 IsUse(false)
		,kTargetNodeName("char_root")
		{}
	}SNpcMarkInfo;

public:

	struct	stAlphaTransitInfo
	{
		//! 알파를 바꾸는 시간
		float m_fAlphaTransitionTime;

		//! 알파를 바꿀 떄 쓰이는 AccumTime;
		float m_fAlphaAccumTime;

		//! Target Alpha
		float m_fTargetAlpha;

		//! Prev Alpha
		float m_fPrevAlpha;

		//!	Current Alpha
		float	m_fCurrentAlpha;
		//! 
		bool bUseLoadingFinishInit;

		stAlphaTransitInfo():
		m_fAlphaTransitionTime(0.0f)
		,m_fAlphaAccumTime(0.0f)
		,m_fTargetAlpha(0.0f)
		,m_fPrevAlpha(0.0f)
		,m_fCurrentAlpha(0.0f)
		,bUseLoadingFinishInit(true)
		{}
	};
	struct	stColorTransitInfo
	{
		float m_fTransitionTime;
		float m_fAccumTime;

		NiColor	m_kTarget,m_kPrev,m_kCurrent;

		stColorTransitInfo()
		{
			memset(this,0,sizeof(stColorTransitInfo));
		}
	};

	struct	stSkillCastingInfo
	{
		unsigned	long	m_ulSkillNo;
		unsigned	long	m_ulCastStartTime;
		unsigned	long	m_ulTotalCastTime;
		stSkillCastingInfo()
		{
			m_ulSkillNo = 0;
		}
	};

	struct	stSkillCoolTimeInfo
	{
		struct	stCoolTimeInfoNode
		{
			unsigned	long	m_ulSkillNo;
			unsigned	long	m_ulCoolStartTime;
			unsigned	long	m_ulTotalCoolTime;
			stCoolTimeInfoNode(unsigned long ulSkillNo,unsigned long ulCoolStartTime,unsigned long ulTotalCoolTime)
				:m_ulSkillNo(ulSkillNo),
				m_ulCoolStartTime(ulCoolStartTime),
				m_ulTotalCoolTime(ulTotalCoolTime)

			{
			}
		};
		typedef	std::map<unsigned long,stCoolTimeInfoNode> CoolTimeInfoMap;

		CoolTimeInfoMap	m_CoolTimeInfoMap;

		void	Reset()
		{
			m_CoolTimeInfoMap.clear();
		}
	};

	struct	stDamageBlinkInfo
	{
		bool	bEnable;
		float	m_fLastTime;
		float	m_fPeriod;
		float	m_fStartTime,m_fTotalTime;
		bool	m_bToggle;

		stDamageBlinkInfo()
		{
			Reset();
		}
		void	Reset()
		{
			memset(this,0,sizeof(stDamageBlinkInfo));
		}
	};

	struct	stGodTimeInfo
	{
		bool	m_bGodTime;
		float	m_fTotalTime;
		float	m_fStartTime;
		float	m_fBlinkTime;
		stGodTimeInfo()
		{
			Reset();
		}
		void	Reset()
		{
			memset(this,0,sizeof(stGodTimeInfo));
		}
	};

	typedef enum ETransformEffectAttachInfoState
	{
		E_NONE_INIT = -1,
	}ETransformEffectAttachInfoState;

	typedef struct  STransformEffectAttachInfo
	{
		int iTransformEffectID;
		PgIWorldObjectBase::EffectAttachInfoList kContInfoList;
		STransformEffectAttachInfo():iTransformEffectID(E_NONE_INIT)
		{}
	}STransformEffectAttachInfo;

//	ABVShape m_kABVShapes[PG_MAX_NB_ABV_SHAPES];

	typedef enum
	{
		VISIBLE	= 0,
		INVISIBIE_NONE = VISIBLE,
		INVISIBLE_NEAR,
		INVISIBLE_MIDDLE,
		INVISIBLE_FAR,
		INVISIBLE_FARFAR,
		MAX_NUM_INVISIBLE,
	} InvisibleGrade;

	typedef enum
	{
		RA_IDLE = 0,
		RA_OPENING = 1,
		RA_INTRO_IDLE = 2
	} ReservedActionType;
	
	enum eTrailAttachType
	{
		ETAT_NONE	= -1,
		ETAT_BODY	= 0,
		ETAT_R_HAND = 1,
		ETAT_L_HAND = 2,
		ETAT_R_FOOT = 3,
		ETAT_L_FOOT = 4,
		ETAT_CENTER = 5,
		ETAT_MAX
	};
	
	enum eFreezeStatus
	{
		EFS_NONE	= 0,
		EFS_FREEZED = 1/*<<0*/,
		EFS_ALLOW_DMG_ACTION = 1<<1,
	};

	class ColorSet
	{
	public:
		ColorSet(const NiColor &rkAmbient, const NiColor &rkEmissive,const NiColor &rkSpecular,const NiColor &rkDiffuse);

		NiColor m_kAmbient;
		NiColor m_kEmissive;
		NiColor	m_kSpecular;
		NiColor	m_kDiffuse;
	};	

	struct SEffectCountDown
	{
		SEffectCountDown():iEffectNo(0), fCountDown(0.f) {}

		int iEffectNo;
		float fCountDown;
	};

	typedef	std::map<eEquipLimit,int> IntMap;
	typedef std::vector<ItemDesc> DefaultItemContainer;
	typedef std::list<AttachItemInfo> AttachItemContainer;
	typedef std::vector<AMPair> AMContainer;
	typedef std::map<eEquipLimit, PgItemEx* > PartsAttachInfo;
	typedef std::map<std::string, std::string> ScriptParamContainer;
	typedef std::vector<NiMaterialPropertyPtr> MaterialContainer;
	typedef std::map<NiSpecularPropertyPtr, bool> SpecularEnableContainer;
	typedef std::map<NiMaterialPropertyPtr, ColorSet> MaterialColorContainer;
	typedef std::map<NiAlphaPropertyPtr, int> AlphaPropContainer;
	typedef std::list<PgActionEntity> ActionQueue;
	//typedef std::list<PgIActionEntity *> ActionQueue;
	typedef std::list<PgAction *> ActionList;
	typedef std::map<int, std::string> ReservedActionTable;
	typedef std::list<int> IntList;
	typedef std::map<std::string, std::string> VariTextureContainer;
	typedef std::vector<TextureInfo> OrgTextureContainer;
	typedef std::vector<int> CONT_SLOT;
	typedef std::map< int, CONT_SLOT >	CONT_SET_ITEM_SLOT;
	typedef std::set<int>	TOGGLE_SKILL_SET;
	typedef std::map<int,bool> ItemEquipInfo;
	typedef std::list<STransformEffectAttachInfo> CONT_TRANSFORM_EFFECT_ATTACH_INFO;
	typedef std::map<int, int> EQUIP_ITEM_CONT; // 외형만 바꾸어 장착되는 아이템 리스트(ItemPos, ItemNo)
	typedef std::vector<PgTrailNodePtr> CONT_TRAIL;
	typedef std::map<std::string, std::string> CONT_CHANGE_ACTION;
	typedef std::pair<BM::GUID, int> SCommonEffectSlotInfo;	//first: CharGuid, second: AttachSlotNo
	typedef std::vector<SCommonEffectSlotInfo> CONT_COMMON_EFFECT;
	typedef std::set<int> CONT_FIXED_SLOT_LIST;
	//현재 트리거안에 있는가?
	
	struct SAppearanceChangeInfo
	{
		int iItemNo;
		EInvType eInvType;

		SAppearanceChangeInfo():
			iItemNo(0),
			eInvType(IT_NONE)
		{}
		
		SAppearanceChangeInfo(EInvType eType, int ItemNo):
			iItemNo(ItemNo),
			eInvType(eType)
		{}
	};
	typedef std::map<int, SAppearanceChangeInfo> CONT_APPEARANCE_CHANGE_INFO;
public:
	PgActor();
	virtual ~PgActor();
	virtual void Terminate();

	CUnit* GetUnit() const;

	virtual	PgIWorldObject*	CreateCopy();
	virtual	bool CreateCopyEx(PgActor *pkNewActor);

	int	GetABVShapeIndex(char const* strShapeName);
	NiPoint3 GetABVShapeWorldPos(int iIndex);

	//! Render interface
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	//! pkRenderer를 이용해서 Draw
	virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual void DrawNoZTest(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	void CullingProcessParticle(NiCamera *pkCamera, NiVisibleArray *pkArray, PgRenderer *pkRenderer = NULL);

	//! 그림자를 렌더링한다.
	//virtual void RenderShadow(PgRenderer *pkRenderer, NiCamera *pkCamera);

	//Update Actor name
	virtual bool UpdateName(std::wstring const& rkName = L"");
	virtual bool UpdateCustomCount(int const iCount = 0, bool bUpdate = true);
	virtual	bool CanSee();	//	이런 저런 Hide 되는 조건들을 모두 체크해서, 최종적으로 보이는지 안보이는지를 리턴.
	PgEnergyGauge* GetHPGaugeBar() const;
	virtual void UpdateQuestDepend() {};

	//! 이동오브젝트 위에 있을 동안 사용할 가속 설정
	void SetVelocity(NiPoint3 kVelo) { m_kVelocity = kVelo; };
	NiPoint3 GetVelocity() { return m_kVelocity; };

	//! Update Actors
	virtual bool Update(float fAccumTime, float fFrameTime);
	NxVec3 GetAdjustValidDeltaLoc(NxVec3 kTargetDeltaLoc);
	NxVec3 GetAdjustValidDetailDeltaLoc(NxVec3 kTargetDeltaLoc, NxVec3 kFindLoc, float fDegree);
	virtual void UpdatePhysX(float fAccumTime, float fFrameTime);
	virtual void UpdateDownwardPass(float fTime, bool bUpdateControllers);
	//! Parsing Xml
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	void CheckRidingObject();

	bool	IsOnRidingObject()	{	return	(m_pkMountedRidingObject!=NULL);	}


	bool IsVisible() { return m_bVisible;}
	InvisibleGrade GetInvisibleGrade() const { return m_eInvisibleGrade; }
	NiPoint3 GetNormalizedActorPosByCamera() { return m_kNormalizedActorPosByCamera; }

	//! Transformation / Restore
	virtual void Transformation(char const *pcNewModel, char const *pcFirstAction, int const iTransformEffectID=0);
	void RestoreTransformation(char const *pcFirstAction);
	bool IsTransformed()	{	return	m_bTransformed;	}
	const std::string&	GetTransformedActorID()	{	return	m_kTransformedActorID;	}

	/// 애니메이션
	//! 오브젝트에 애니메이션을 지정해준다. 단 즉시 플레이 할 수도 있고 그렇지 않을 수도 있다.
	bool SetTargetAnimation(std::string const& rkAnimationName, bool const bActivate=true, bool const bNoRandom=false);

	//!	현재 애니메이션 시퀀스 번호를 리턴한다.
	int	GetAniSequenceID()	{	return	m_kSeqID;	}

	//! 레이어 애니메이션을 활성화 한다.
	//bool ActivateLayerAnimation(char const *pcAnimationName);

	//! 레이어 애니메이션을 비활성화 한다.
	//bool DeactivateLayerAnimation(char const *pcAnimationName);

	//!	애니메이션 플레이 시간을 리턴한다.
	float GetAnimationLength(std::string &rkAnimationName);
	bool GetAnimationInfo(std::string &rkInfoName, int iSeqID, std::string &rkInfoOut, PgAction* pkAction = NULL);

	//! 현재 설정된 애니메이션을 활성화 한다.
	bool ActivateAnimation(bool bAllowRepeat = false);

	//!	애니메이션 리셋!
	void ResetAnimation();

	//! 현재 재생중인 애니메이션이 끝났는지 알려준다.
	bool IsAnimationDone();

	//! 현재 재생중인 애니메이션가 루프인지 알려준다.
	bool IsAnimationLoop() const;

	//! 애니메이션의 스피드를 조절한다.
	void SetAnimSpeed(float fSpeed);

	//! 애니메이션 스피드 얻기
	float GetAnimSpeed() const;

	//! 현재 재생중인 애니메이션에 찾고자 하는 텍스트키가 있는지 검사
	bool HaveAnimationTextKey(char const* szKey, char const* szAnimationName=NULL) const;

	//! 죽어서 날아가던 액터가 카메라와 부딛혔는지 체크한다.
	bool CheckCollWithCamera(float &fCollScreenX,float &fCollScreenY);

	//! Normal 공격의 종료 시간을 저장한다
	void SetNormalAttackEndTime();

	//! 최근 Normal 공격의 종료시간을 리턴한다.
	unsigned long GetNormalAttackEndTime() const;

	//! 현재 시각이 연타 가능 입력 시간 구간내인지 체크한다.
	bool CanNowConnectToNextComboAttack(float const fMaxTime) const;

	//! S 버튼에 설정되는 액션 ID
	void SetNormalAttackActionID(std::string const actionid);
	char const* GetNormalAttackActionID();

	bool	AddDropItem(PgDropBox *pkItemBox);	//	추가할 수 있으면 true 아니면 false
	void	DoDropItems(int iActionInstanceID=-1,int iReqCount = 0,float fJumpHeight = 0);	//	iActionInstanceID 에 해당하는 아이템들을 떨군다. -1 일 경우 모든 아이템을 떨군다. 


	//! 다른 액터의 장비를 그대로 복사해서 세팅
	virtual void CopyEquipItem(PgActor* pkActor);
	
	//! 다른 액터의 장비를 그대로 복사해서 세팅
	void EquipAllItem_SubPlayer(PgActor* pkCallerActor);
	bool CopyEquipItemFromMainPlayer(PgActor* pkActor, EInvType kInvType, EEquipPos kItemPos);
	
	//! 인벤토리가 변경될때 처리되는 내용들
	void OnChangeInventory();

	void	OnAbilChanged(int iAbilType,int iValue);

	//! 매달릴 수 있는가?
	//bool HangItOn();

	//! 줄을 탈 수 있는가?
	//bool HangOnRope();

	//! 사다리를 탈 수 있는가?
	bool ClimbUpLadder();

	//! 사다리를 바라본다.
	void SeeLadder();

	bool	SyncActionOnAddUnit(PgPlayer* pkPlayer);
	//!	SyncMove
	bool	StartSyncMove(PgAction *pkSyncMoveNextAction);
	bool	UpdateSyncMove(float fSpeed,float fFrameTime);

	/// 액션
	//! 기본 행동을 한다.
	void DoReservedAction(ReservedActionType eType,bool bTransitRightAway = false);

	//! 예약된 액션을 반환한다.
	char const *GetReservedAction(ReservedActionType eType);

	//! Action Layer에 추가한다.
	//bool AddToActionLayer(PgAction *pkAction);

	//----------------------------------------------
	// Action Queuing
	//----------------------------------------------
	//! Action Queue를 처리함.
	virtual bool ProcessActionQueue();
	
	//! Action Queue를 정리.
	void ClearActionQueue();

	//! Action의 초기 위치를 맞춘다.
	virtual bool BeginSync(PgAction *pkAction, DWORD dwOvertime);

	//! 초기 위치로 좌표 계산해서 맞춤.
	virtual bool UpdateSync(float fFrameTime);

	//! 좌표를 Sync중인가.
	bool IsSync();
	//---------------------------------------------

	//! 예측된 속도를 반환
	//float GetEstimatedVelocity(float fPredictedDist, float fRealDist, float fVelocity);

	//! Cubic Spline method로 계산한 좌표를 반환.
	//NiPoint3 GetCubicSplines(NiPoint3 &rkStartPos, NiPoint3 &rkEndPos, NiPoint3 &rkVelocity);

	//! Action을 처리
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false);

	//! 모니터링 해야 할 방향을 널리 알린다.
	void BroadcastMonitoredDirection();

	//! 지정한 액션의 상태로 Transit을 시도한다.
	bool TransitAction(char const *pcNextActionName, bool bEnable = true, NiPoint3 *pkActionStartPos = NULL, BYTE byDirection = DIR_NONE,bool bForceToTransit = false);

	//!	TransitAction 을 하기위한 PgAction 을 생성한다.
	virtual PgAction* CreateActionForTransitAction(char const *pcNextActionName, bool bEnable = true, NiPoint3 *pkActionStartPos = NULL, BYTE byDirection = DIR_NONE, int iActionNo = 0);

	//!	현재 액션을 중단 시키고 pcNextActionName 으로 전이시킨다.
	void CancelAction(int iActionID,int iActionInstanceID,char const *pcNextActionName, bool bToggleCancel = false);

	//! 현재 액션에 지정된 Slot번호의 Animation을 플레이 한다.
	bool PlayCurrentSlot(bool bNoRandom=false);

	//! 다음 지정된 애니메이션을 플레이 한다.
	bool PlayNext();

	//! 다음 지정된 애니메이션을 플레이 한다.
	bool PlayPrev();

	//! 현재 플레이 중인 Action을 리턴한다.
	PgAction *GetAction() const;

	void	ClearAllActionEffect();

	//!	토클 액션의 토클상태를 변환한다.
	void	ActionToggleStateChange(int const iActionNo, bool const bOn);
	bool	GetActionToggleState(int const iActionNo) const;	//	true : Activated  false : Deactivated

	//! PhysX를 초기화한다.
	virtual	void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual	void InitRidingInfo(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void UpdateRidingInfo(float fAccumTime, float fFrameTime);
	virtual void ResetABVToNewAM();
	//! 주체가 Gamebryo인지 AgeiaPhysX 인지 정한다.
	void InitPhysical(bool bIsPhysical = true);

	//! PhysX를 해제한다.
	virtual void ReleasePhysX();
	virtual void ReleaseABVShapes();
	virtual void ReleaseAllParticles();
	virtual bool BeforeUse();
	virtual bool BeforeCleanUp();

	void	PickUpNearItem(bool bCheckCaller = false, float const fPickRange = 30.0f, int const iPickCount = INT_MAX);	//	근처의 아이템을 집는다.

	//!	월드 좌표
	NiPoint3 GetPos();

	//! 바닥 좌표
	NiPoint3 GetFloorLoc();

	//! 물리를 동기화할 것인지, 게임브리오에 동기화할 것인지 
//	void SwitchPhysical(bool bGoPhysical, float fAccumTime, float fFrameTime);

	/// 패스
	//! 가장 가까운 패스의 노멀 벡터를 계산한다.
	bool FindPathNormal(bool const bDoNotConcil = false);

	//! 캐릭터의 회전을 패스와 방향에 맞게 조절한다.
	virtual void ConcilDirection(NiPoint3 &rkLookingDir, bool const bRightAway = false);
	
	//! Path의 최상위 노드를 설정한다.
	void SetPathRoot(NiNode *pkPathRoot);

	/// 물리
	//! 물리 시스템의 Actor를 반환한다.
	NxActor *GetPhysXActor() const;

	//! 현재 바라보고 있는 방향을 알려준다.
	NiPoint3 const &GetLookingDir()const;

	//! 현재 진행 방향을 알려준다.
	NiPoint3 const& GetMovingDir()const;
	void SetMovingDir(NiPoint3 const kMovingDir);

	//! 캐릭터 볼륨을 변경한다.
	//bool UpdateCharacterExtents(float fRadius, float fHeight);

	//! 캐릭터를 Quternion으로 돌림
	bool SetRotation(float m_fDegree, NiPoint3 kAxis, bool bForce = false);
	void SetRotation(const NiQuaternion &kQuat, bool bForce = false);
	void SetRotation(const NxQuat &kQuat, bool bForce = false);

	bool EnableRotation();

	//! 현재 진행 방향이 왼쪽인지 오른쪽인지 알려준다.
//	bool IsToLeft();
	
	//! 진행 방향을 설정한다.
//	void ToLeft(bool const bLeft, bool bTurnRightAway = false);		// bTurnRightAway : 바로 해당 방향을 바라보도록 만든다.

	//! 눌러진 키에 따라 방향을 갱신한다.
	void InvalidateDirection();

	//! 현재 방향이 인자로 들어오는 Direction을 포함하는지 체크
	bool ContainsDirection(BYTE byDir);

	//! 마지막 바라본 방향
	BYTE GetLastDirection();

	//! 상하좌우 4방향 중 어디를 보고 있는지 리턴한다.
	BYTE GetDirection();

	static	Direction	GetDirectionInverse(Direction kDirection);	//	kDirection 의 역방향을 구한다.

	//! Moving Vector로 부터 우리 방식의 Direction을 가져온다.
	BYTE GetDirFromMovingVector(NiPoint3 const& rkMovingVector); // 좌우 2가지 방향을 얻어온다.

	//! Moving Vector로 부터 우리 방식의 Direction을 가져온다.
	BYTE GetDirFromMovingVector8Way(NiPoint3 const& rkMovingVector); // 8방향을 얻어온다.

	//! 상하좌우 4방향 중 어디를 볼지 설정한다.
	void SetDirection(BYTE byDirection);

	//! 방향을 예약.
	void ReserveDirection(BYTE byDirection, DWORD dwDirectionTerm, NiPoint3 &rkCurPos);

	//! 보는 방향을 설정
	void SetLookingDirection(BYTE byDirection, bool bRightAway = false);
	void SetLookingTarget(NiPoint3 const &rkTarget, bool bRightAway = false);

	void	ReserveDieByActioin(bool bDie = true);
	bool	IsReserveDieByAction();

	//! 
	void AddChangeAction(std::string const& rkFromAction, std::string const& rkToAction);
	void DelChangeAction(std::string const& rkActionName);
	char const* GetChangeAction(char const* pcActionName)const;
	
	//! 미리사용될 애니메이션(kf)파일을 읽어 둔다. 
	void PreLoadAllAnimation();
public:
	//! Effect를 등록한다.
	//	leesg213 파라메터 추가
	bool AddEffect(int const iEffectNo, int const iEffectValue,float const fElapsedTime = 0, BM::GUID const &kCasterGUID = BM::GUID(), int const iActionInstanceID = 0, DWORD const dwTimeStamp = 0, bool const bIsTemporaryEffect = 0, CSkillDef const* pkSkillDef = NULL);
	bool AddEffectToAction(PgPilot	*pkTargetPilot,int iEffectNo,int iEffectValue,int iActionInstanceID);
	
	//!	무시할 이펙트 리스트에 추가한다.
	void	AddIgnoreEffect(int iEffectID);
	void	RemoveIgnoreEffect(int iEffectID);
	void	ClearIgnoreEffectList();
	bool	IsIgnoreEffect(int iEffectID);

	//! Effect를 등록해제 한다.
	void RemoveEffectFromAction(PgPilot	*pkTargetPilot,int iEffectNo);

	//!	대미지 블링크
	void	StartDamageBlink(bool bStart);
	
	//! 현재 설정되어 있는 트리거를 반환한다.
	PgTrigger *GetCurrentTrigger();

	//! 현재 트리거를 설정한다.
	void SetCurrentTrigger(PgTrigger *pkTrigger);

	//! 트리거 안에 있는지 여부 확인
	bool GetIsInTrigger() const;

	//! 캐릭터를 깜박이게 한다.
	void BlinkThis(bool const bBlink, int const iBlinkFreq);

	//! 캐릭터를 움직인다. (변위가 아닌 절대 수치로.)
	void MoveActorAbsolute(NxVec3 const &kMoveAbs);

	//! 캐릭터를 움직인다.
	void MoveActor(NxVec3 kDelta);

	//! Chain Attack Count 를 1 증가시킨다. 단 최근 증가된 시간에서 3초 이상 흘렀을 경우 1 로리셋.
	void IncreaseChainAttackCount(int const iSkillNo);

	//! 파츠를 액터에 붙인다.(붙을 곳을 자동으로 찾아가 붙는다)
	int AttachSkinningParts(PgItemEx* pkParts);
	
	//! 파츠를 액터의 특정 더미에 붙인다.(이때의 파츠는 스키닝을 하지 않는 파츠다)
	bool AttachNoSkinningParts(PgItemEx* pkParts,char const *pcTargetDummy = 0);
	
	//! 파츠를 액터에서 떼어낸다.
	bool DetachParts(eEquipLimit kItemPos);
	bool DetachParts(PgItemEx* pkParts);

	//! 파츠가 이미 붙어있는지 체크한다.
	bool IsExistParts(eEquipLimit kItemPos);

	//! 모든 파츠를 제거한다.
	bool DetachAllParts();

	//모든 파츠의 로딩이 완료 되었는지를 검사.라이딩 펫 전용이며, 내 캐릭을 대상으로는 쓰면 안된다.
	bool IsCompleteLoadParts(void)
	{
		//return (m_bBeginLoadParts && m_kAttachItemContainer.empty());
		return m_bLoadingComplete;
	}

	//! DFS로 스키닝 데이터를 찾는다.
	int SkinningParts(PgItemEx* pkParts, NiNode *pkNode, bool bAttach);

	//! 이 액터가 내PC의 펫인가?
	//bool IsMyPet();

	//! 이 액터가 펫인가?
	//bool IsPet();

	//! 걷는다.
	virtual	bool Walk(BYTE byDir, float fSpeed, float fFrameTime, bool bCorrectFinalPos = false);

	//! 날아서 따라간다
	float TraceFly(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, float fLimitZ, float fFloatHeight, bool bCanRotate);
	//! 바닥으로 달린다
	virtual float TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate);
	void SetTraceFlyTargetLoc(NiPoint3 const &rkTargetLoc)		{ m_kTargetLoc = rkTargetLoc; }
	NiPoint3 GetTraceFlyTargetLoc() const { return m_kTargetLoc; }

	//! Direction으로 부터 Unitize Vector를 얻는다.
	NiPoint3 GetDirectionVector(BYTE byDirection);

	//! Walking Target의 Direction을 얻는다.
	NiPoint3 GetWalkingTargetDir();

	//! 타겟 포지션으로 움직이록 설정한 것을 해제한다.
	void SetNoWalkingTarget(bool bDoNextAction);

	//! 목표 타겟으로 움직이록 설정한다.
	void SetWalkingTarget(BM::GUID &rkGuid, bool const bForceToTarget = false, char const *pcNextAction = 0, bool bCheckCliff = false,float fSkillRange = 0);

	//! 목표 지점으로 움직이도록 설정한다.
	void SetWalkingTargetLoc(NiPoint3 const &kTargetLoc, bool const bForceToTarget = false, char const *pcNextAction = 0, bool bCheckCliff = false,float fSkillRange = 0);

	//! 목표 지점을 반환.
	NiPoint3 const &GetWalkingTargetLoc();

	//! 목표 지점으로 움직이도록 설정 되어 있는지 리턴.
	bool GetWalkingToTarget();

	//! 패스로부터의 거리를 얻는다.
	float GetDistanceFromPath(NiPoint3 const &kPos);

	//! 캐릭터를 민다.
	void	PushActor(bool bLeft,float fDistance,float fVelocity,float fAccel);
	void	PushActor(NiPoint3 const &rkDir,float fDistance,float fVelocity,float fAccel);
	bool	GetNowPush()
	{
		return	m_Push.m_bActivated;
	}

	//! 선다.
	void Stop();
	
	//! 점프 한다.
	void StartJump(float const fHeight);

	//! 목표 지점까지 점프한다.
	float StartTeleJump(NiPoint3 const &kTargetPoint, float const fHeight);

	//! 떨어질수 있는 곳일때 떨어질수 없게 보정 할 수 있는지 없는지 세팅
	void SetAdjustValidPos(bool bAble)	{	m_bAdjustValidPos = bAble;	}

	//! 떨어질수 있는 곳일때 떨어질수 없게 보정 할 수 있는지 없는지 여부 반환 : m_bAdjustValidPos
	bool GetAdjustValidPos()	const	{	return m_bAdjustValidPos;	}

	//! 강제 좌표 이동이 가능한가
	bool GetForceSync();

	//! 강제 좌표 이동할 것인지 설정
	void SetForceSync(bool bSync);

	//! 점프를 중단 한다.
	void StopJump();

	//! 점프중인지를 True/False로 반환한다.
	bool GetJump()	const;
	void	SetJump(bool bJump)	{	m_bJump = bJump;	}

	//! 점프한 높이를  반환한다.
	float GetJumpAccumHeight();

	//!	점프한 시간 
	float	GetJumpTime()	const;
	void	SetJumpTime(float fTime)	{	m_fJumpTime = fTime;	}

	//! 점프한 높이 리셋
	void ResetJumpAccumHeight();

	//! 마지막으로 바닥을 밟았던 위치를 리셋.
	void ResetLastFloorPos();
	
	//! 마지막으로 바닥을 밟았던 위치를 반환한다.
	NiPoint3 GetLastFloorPos();

	//! 모니터링 하고 있는 액션이 눌러졌는지, 떼어졌는지 알려준다.
	unsigned int GetActionState(char const *pcActionID);

	//! 모니터링 액션 슬롯을 초기화 한다.
	void ClearActionState();

	//! 자유로운 이동(중력이 없다)
	void SetFreeMove(bool bFreeMove);
	bool	GetFreeMove()	const	{	return	m_bFreeMove;	}

	void	AddActionEntity(PgAction *pkAction,Direction kDirection);
	void	AddActionEntity(PgActionEntity& kActionEntity);
	void	AddActionEntityToFollowers(PgActionEntity& kActionEntity);

	//! 동기화 보간 변위를 설정한다.
//	void SetSyncInterpolDelta(NxVec3 &rkDelta);

	//! 동기화 좌표를 설정한다.
//	void SetSyncPosition(NiPoint3 &rkSyncPos);
	
	//! 동기화 보간 시간을 설정한다. (0 = 시작, PG_SYNC_INTERPOL_TIME = 끝)
//	void SetSyncInterpolTime(float fTime);

	//! 파라메터를 설정
	void SetParam(char const *pcKey, char const *pcVal);
	
	//! 파마메터를 가져옴
	char const*	GetParam(char const *pcParamName);

	StatusEffectInstanceList&			GetStatusEffectInstanceList()	{	return	m_StatusEffectInstanceList;	}
	StatusEffectInstanceList::iterator	RemoveStatusEffectInstance(PgStatusEffectInstance const& kInstance);

	void	AddStatusEffectInstance(PgStatusEffectInstance* pkInstance);	
	bool	CheckStatusEffectExist(char const* strStatusEffectXMLID);
	bool	CheckStatusEffectTypeExist(BYTE byType);
	
	bool	CheckEffectExist(int const iEffectNo, bool const bInGroup = false);
	int		CheckSkillExist(int const iSkillNo);

public:
	char const* GetStartParamID(char const *kStr);
	int const GetStartEffectSave(char const *kStr);
	std::string GetStatusEffectParam(int const iEffectID, std::string const& kKey)const;
	void SetStatusEffectParam(int const iEffectID, std::string const& kKey, std::string const& kValue);

	//! 변신 하기 전의 원래 액터를 설정한다.
	void	SetOriginalActorGUID(BM::GUID guid);
	BM::GUID	GetOriginalActorGUID();

	//!	줄 수 있는 퀘스트 정보 업데이트
	virtual void	ClearQuestInfo() {};
	virtual void	PopSavedQuestSimpleInfo() {};	//	퀘스트 매니저가 저장하고 있는 퀘스트 정보를 가져온다.

	//!	진행중인 퀘스트 정보 업데이트
	//void UpdateQuestUserInfo(const SUserQuestState& rkUpdatedState);

	//! 캐릭터가 죽어서 날아갈때
	void SetThrowStart();

	//! 캐릭터의 Scale 을 변화시킨다.
	void	SetTargetScale(float fScale,unsigned long ulTotalScaleChangeTime = 1000.0f);

	//! 알파 트랜짓이 끝났는지 체크.
	bool IsAlphaTransitDone();

	//! 캐릭터의 Alphat를 변화시킨다.
	void SetTargetAlpha(float const fStartAlpha, float const fTargetAlpha, float const fTransitionTime, bool const bIgnoreLoadingFisnishInit = false);
	
	void SetTargetColor(const NiColor &kColor, float const fTransitionTime);
	void SetTargetSpecular(const NiColor &kColor, float const fTransitionTime);

	void TransitActorSpecular(float fFrameTime);

	//! 캐릭터의 Opacity를 변화시킨다.
	//void SetTargetOpacity(float fOpacity, float fTransitionTime);

	//! 대미지 숫자를 캐릭터 머리 위에 띄워준다.
	void ShowDamageNum(NiPoint3 vAttackerPos,NiPoint3 kTargetPos,int iDamage,bool const bClamScreen = false, bool const bCritical = false, BYTE const btColor = 5, int const iEnchantLevel = 0, int const iExceptAbil = 0); //5 가 흰색 cpp 참조
	//! 경험치 숫자를 캐릭터 머리 위에 띄워준다.
	void ShowExpNum(int const iExp);
	//! 자동으로 회복되는 HP/MP 회복량을 머리 위에 띄워 준다.
	void ShowHpMpNum(int iValue, bool bHp = true);

	//!	스킬 텍스트를 띄운다.
	void ShowSkillText(NiPoint3 kTargetPos,int iTextType,bool bUp);

	//!	간단 메세지를 띄운다.
	void ShowSimpleText(NiPoint3 kTargetPos,int iTextType);

	//! 장비하고 있는 무기의 타입번호를 리턴한다.
	int	GetEquippedWeaponType();

	char* GetEquippedWeaponProjectileID();
	PgItemEx*	GetEquippedWeapon() const;

	//! 기본 아이템을 추가한다.
	void AddToDefaultItem(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const* pEnchant = NULL);

	//! 기본 아이템을 속성을 셋팅한다.
	void SetDefaultItem(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const* pEnchant = NULL);

	//! 기본 아이템을 추출한다.
	int GetDefaultItem(eEquipLimit kItemPos);
	bool GetDefaultItemEnchantInfo(eEquipLimit kItemPos, SEnchantInfo& kInfo);
	
	//! 기본 아이템을 삭제한다.
	bool DelDefaultItem(eEquipLimit kItemPos);

	void	RenderShadowObject();
	void	SetSpotLightColor(int Red,int Green,int Blue);

	//! 로딩할 아이템 갯수를 올린다.
	void IncEquipCount();

	//! 로딩할 아이템 갯수를 줄인다.
	void DecEquipCount();

	//! 아이템 착용	(SetToDefaultItem을 True로 하면, 착용한 아이템을 기본 아이템으로 해버린다)
	bool AddEquipItem(int iItemNo, bool bSetToDefaultItem = false, PgItemEx::ItemLoadType eLoadType = PgItemEx::LOAD_TYPE_DEFAULT, bool bReal=true);				// 스레드를 구동해서 Item을 장착
	bool EquipItemProc(int iItemNo, bool bSetToDefaultItem = false, PgItemEx* pkEquipItem = NULL, PgItemEx::ItemLoadType eLoadType = PgItemEx::LOAD_TYPE_DEFAULT);			// 스레드에서 사용하는 Load 함수
	virtual bool EquipItem(PgItemEx* pkEquipItem, int iItemNo, bool bSetToDefault);		// 아이템을 즉시 장착

	void RefreshCustomItemColor();

	int GetAdjustedItemNo( CUnit *pkPlayer, EEquipPos const kEquipPos );//	공성전 등에서 장착 아이템 번호를 임의로 조정해야 할 때 사용된다.
	void SetAdjustedItem(EQUIP_ITEM_CONT const& kEquipItemCont, int const iOtherEquipItemReturnValue = 0);
	bool IsEquipItemList()const;
	
	int GetCashItemChanger( EEquipPos const kEquipPos );
	int GetNormalItemChanger( EEquipPos const kEquipPos );

	void AddCashItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont);
	void RemoveCashItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont);
	
	void AddNormalItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont);
	void RemoveNormalItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont);

	//  아이템 장착 위치(훈장/풍선) - 훈장 아이템이 착용위치 : 훈장 -> 보이는데 상관이 없다. 장착해제시 캐쉬 아이템을 계속 유지 / 풍선 -> 풍선 아이템 해제시 훈장 아이템이 보이는 루틴 안타야 된다.
	bool IsEquipMedalPos(EEquipPos kItemPos);
	//! 아이템을 착용 위치로 착용
	virtual bool EquipItemByPos(EInvType kInvType,EEquipPos eEquipPos, bool bReal=true);

	//! 아이템 착용 해제
	bool UnequipItem( EInvType kInvType,EEquipPos kItemPos, int iItemNo, PgItemEx::ItemLoadType eLoadType = PgItemEx::LOAD_TYPE_INSTANT, bool const bClear = false , bool bReal=true);


	//! 아이템 색깔 바꿈
	bool SetItemColor(eEquipLimit kItemPos, int iItemNo, bool bDefaultItem = true);

	virtual	bool	GetCanBatchRender()	const;

	void TransitColor(float fTime);
	void	SetColor(const NiColor& kColor);

	//! Actor에 알파를 변화시킨다.
	void TransitAlpha(float fTime);
	void	SetAlpha(float fAlpha);
	float	GetAlpha()const;
	void SetParticleAlpha(NiAVObject* pkRoot, float fAlpha);

	void	NeedToUpdateMaterialProp(bool bNeed)	{	m_bMaterialCached = !bNeed;	}

	//! Material Property를 찾는다.
	bool FindMaterialProp(NiNode *pkRoot, bool bCheckNoChange = true, bool bTraverseAllNode = false);

	//! Material Color를 저장해 둔다.
	bool StoreDefaultMaterialColor(NiNode *pkNode, bool bForce);

	//! Alpha Property를 얻어온다.
	bool GetAlphaProperty();

	//!	액터의 스페큘러를 켠다
	virtual	void	TurnOnSpecular();
	//!	액터의 스페큘러를 원상복구시킨다.
	virtual	void	RestoreSpecular();

	//! 모든 아이템 착용
	void EquipAllItem();

	//! 착용 아이템 숨기기
	void HideEquipItem(int iEquipPos, bool bHide);

	//! 새로운 월드에 갔다 붙인다.
	void AttachToWorld(PgWorld *pkWorld);

	//! 기존 월드에서 떼어 낸다.
	void ReleaseFromWorld();

	//!	죽을 때 뿌려줄 이펙트 파티클 ID 를 얻어온다.
	std::string const &GetDieParticleID() const		{	return	m_kDieParticleID;	}
	std::string const &GetDieParticleNode() const	{	return	m_kDieParticleNode;	}
	std::string const &GetDieSoundID() const		{	return	m_kDieSoundID;		}
	const float &GetDieParticleScale() const		{	return	m_fDieParticleScale;}

	//! 말풍선을 띄운다.
	void	ShowChatBalloon(const EChatType eChatType, char const *Text, int const iUpTime = 4000, bool const bFake = false);
	void	ShowChatBalloon(const EChatType eChatType, std::wstring const &Text, int iUpTime = 4000, bool const bFake = false);
	void	ShowChatBalloon_Clear();

	// Mark를 띄운다.
	void	ShowMark( MARK_TYPE const eMarkType );
	void	ShowMark_Clear();

	//! 클릭당했을 때 말풍선을 띄워준다.(클라이언트 Only)
	virtual bool OnClickSay() {return false;};
	virtual bool OnClickTalk() {return false;};
	virtual bool OnClickWarning() {return false;};

	virtual bool IsActivate(void)const{return true;}
	virtual bool IsHaveTalk(void)const {return false;}
	virtual bool IsHaveWarning(void)const {return false;}

	//! 지정된 슬롯의 사운드를 플레이한다.
	bool PlaySlotSound(std::string const& rkSlotName);
	const CMonsterDef *m_pkMonsterDef;

	//! 이 캐릭터를 카메라 포거스 캐릭터로 만든다.
	void	SetCameraFocus();

	void	SetMovingDelta(NxVec3 const &kDelta);
	NxVec3	const	&GetMovingDelta();

	//bool	IsSyncCrashed()	{	return	m_bSyncCrashed;	}
	//void	SetSyncCrashed(bool bCrashed)	{	m_bSyncCrashed = bCrashed;	}

//	void	SetSyncTargetPos(NiPoint3 &kTargetPos);
//	void	SetSyncStartPos(NiPoint3 &kStartPos);
//	bool	IsSyncPosProcessing();
//	void	ProcessSyncPosition(float fFrameTime, float fSyncRate);
//	void	StartSyncPosition();
public:
	//! 어타치 된 노드 리스트에 추가한다.
	void AddToAttachedList(PgItemEx* pkParts, NiNode *pkNode);

	//! 어타치 된 노드 리스트에서 제거한다.
	void RemoveFromAttachedList(PgItemEx* pkParts, NiNode *pkNode);

	//!	무기 궤적 그리기 시작
	void	StartWeaponTrail();
	//!	무기	궤적	그리기	종료
	void	EndWeaponTrail();
	//!	몸 궤적 그리기 시작
	void	StartBodyTrail(char const* strTexPath, int iTotalTime, int iBrightTime);
	//!	몸	궤적	그리기	종료
	void	EndBodyTrail();
	
	bool	StartTrail(eTrailAttachType const eTrailType, std::string const kTexPath, int iTotalTime, int iBrightTime);
	bool	EndTrail(eTrailAttachType const eTrailType);

	void	RefreshHPGaugeBar(int iBeforeHP,int iNewHP,PgActor *pkAttacker, bool const bSetAbil = false);

	bool	IsSendBlowStatus()	{	return	m_bSendBlowStatus;	}
	void	SetSendBlowStatus(bool bSend, bool bFirstDown = false, bool bNoUseStandUpTime = false);

	// Actor를 자동삭제할 타이머
	void	SetAutoDeleteActorTimer(float fTime);

	// Following 
	void	RequestFollowActor(BM::GUID const& kTargetActorGUID, EPlayer_Follow_Mode const kMode,bool const bForce = false);	//	kTargetActorGUID를 따라갈것을 서버에 요청한다.
	void	ResponseFollowActor(BM::GUID const& kTargetActorGUID, EPlayer_Follow_Mode const kMode);	//	kTargetActorGUID를 따라갈것을 서버에 요청한다.

	bool	CheckCanFollow(const	BM::GUID &kTargetActorGUID, bool const bMsg=true);	//	kTargetActorGUID 를 따라갈 수 있는지 체크하고 메세지를 보여준다.
	bool	FollowActor(const	BM::GUID &kTargetActorGUID);	//	kTargetActorGUID 를 따라가도록 한다.
	void	StopFollowActor();
	bool	ProcessFollowingActor();
	int		GetPosChangeActionCount(const ActionQueue &kQueue);	//	큐에 들어있는 액션 중에, 캐릭터의 위치를 이동시키는 액션이 몇개나 있는가?
	
	bool	IsNowFollowingMeActor()	{ return m_kFollowInfo.m_kFollowingMeActorCont.size(); }
	bool	IsNowFollowing()	{	return	m_kFollowInfo.m_kFollowTargetActor != BM::GUID::NullData();	}	//	현재 누군가를 따라가는 중인가?
	BM::GUID const &GetFollowingTargetGUID()	{	return	m_kFollowInfo.m_kFollowTargetActor;	}
	
	bool	FindFollowingMeActor(const	BM::GUID &kActorGUID);
	void	AddFollowingMeActor(const	BM::GUID &kActorGUID);	//	나를 따라오는 액터를 추가한다. FollowActor() 에서 자동으로 호출된다.
	void	RemoveFollowingMeActor(const	BM::GUID &kActorGUID);	//	나를 따라오는 액터를 제거한다. StopFollowActor() 에서 자동으로 호출된다.
	void	SetFollowTargetActor(const	BM::GUID &kActorGUID);


	//! TargetGuid객체와 Distance거리 내에 있으면 True, 그렇지 않으면 false를 리턴(Target객체가 없어도 False)
	//bool OutOfSight(BM::GUID &rkTargetGuid, float fDistance, bool bConsiderZAxis = false);

	//! TargetGuid객체를 향해서 뛰어간다. 어떤 행동 중이거나, [주인이 없으면(펫이 아니면) false를 리턴]
	//bool FollowActor(BM::GUID &rkTargetGuid, float fMoveSpeed);

	//! 자신이 TargetGuid객체보다 아래에 있는지를 알려준다. (+Z축 기준) [주인이 없으면(펫이 아니면) false를 리턴]
	//int CompareActorPosition(BM::GUID &rkTargetGuid, NiPoint3 kAxis, float fRange = 20.0f);

	//! fDistance 앞이 절벽인지 아닌지 알아낸다.
	//bool WillBeFall(bool bDown, float fDistance);

	//!	일정 시간동안 애니메이션 스피드를 조정한다.
	void	SetAnimSpeedInPeriod(float const fAnimSpeed, int const iPeriod);

	//! 애니메이션 스피드를 원래대로 돌린다.
	void	SetAnimOriginalSpeed();

	//!	일정 시간동안 액터를 흔든다.
	void	SetShakeInPeriod(float const fShakePower,int const iPeriod);

	//! Actor를 빙글빙글 돌린다.
	//void TwistActor(int const iTwistTimes, float const fSpeed);

	//! 돌렸던 Actor를 원상복귀 시킨다(보간 없다)
	void RestoreTwistedActor();

	//!	액터의 아이템 모델을 임시로 교체한다.
	void	ChangeItemModel(eEquipLimit kItemPos,char const *pkNewItemXMLPath);
	//!	액터의 무기 모델을 원래 것으로 돌려놓는다.
	void	RestoreItemModel(eEquipLimit kItemPos);

	//!	다운상태로 만든다.
	void	SetDownState(bool bDownState);
	bool	IsDownState()	{	return	m_bDownState;	}

	void	SetTotalDownTime(float fTime)	{	m_fTotalDownTime = fTime;	}
	float	GetTotalDownTime()	{	return	m_fTotalDownTime;	}

	void SetTraceUpdate(bool bTrace) { m_bTraceUpdate = bTrace; }
	bool GetTraceUpdate() { return m_bTraceUpdate; }

	PgAction*	ReserveTransitAction(char const *kNextAction, BYTE byDirection = DIR_NONE);
	PgAction*	ReserveTransitAction(int iActionNo, BYTE byDirection = DIR_NONE);
	PgAction*	ReserveTransitAction(PgAction *pkNextAction);

	PgAction*	GetReservedTransitAction();
	void	ClearReservedAction();
	bool	DoReservedTransitAction();

	BM::GUID	CreateTempAction(int const iActionNo);
	PgAction*	GetTempAction(BM::GUID const& kActionGuid);
	void		RemoveTempAction(BM::GUID const& kActionGuid);
	void		ClearTempAction();

	//! Pet의 주인 Guid를 설정한다.
	//void SetPetMaster(BM::GUID const &rkGuid);

	//! Pet의 주인 Guid를 가져온다.
	//BM::GUID &GetPetMaster();

	//! 펫을 탄다.
	//bool RideMyPet(bool bRide);

	//! 걷는 속도(점프 했을 때에도)의 Scale을 조절한다.
	void SetSpeedScale(float fScale);

	//! 걷는 속도를 반환한다.
	float GetSpeedScale();

	bool	IsEnemy(PgActor *pkTarget);	//	나의 적인가?

	//! 지정한 곳을 봐라
	bool LookAt(NiPoint3 const& rkTarget,bool bTurnRightAway=false,bool bBidirection=false,bool bNotCheckSameDir=false);
//	bool	LookAtBidirection(NiPoint3 &rkTarget);


	//! 지금 각도에서 추가로 회전
	void IncRotate(float fRadian);

	//! 움직이는 물체에 관한 캐릭터 이동.
	bool ApplyMovingObject_OnEnter(PgTrigger* pkTrigger);
	bool ApplyMovingObject_OnUpdate(PgTrigger* pkTrigger);
	bool ApplyMovingObject_OnLeave(PgTrigger* pkTrigger);

	//! 장착한 아이템 중, 원하는 것을 숨긴다.
	bool HideParts(const eEquipLimit kEquipLimit, bool const bHide);
	void ApplyHideParts(const eEquipLimit kEquipLimit);
	void ApplyHidePartsAll();
	bool GetPartsHideCnt(eEquipLimit const kEquipLimit, int& iCnt_out) const;
	//HideCount를 무시하고 숨김옵션을 동작한다.(신종족 격투가 보조무기용)
	bool HideParts_IgnoreHideCnt(const eEquipLimit kEquipLimit, bool const bHide);

	//! 액터 NIF의 노드를 숨기거나 보이게 한다.
	void HideNode(char const *strNodeName,bool bHide);

	//! 점프시 받는 중력을 조절한다.
	void SetGravity(float fGravity)	{	ms_fGravity = fGravity;	}
	float	GetGravity()	const	{	return	ms_fGravity;	}

	//! 두 방향만을 보게 하거나, 방향을 풀거나 한다.
	void LockBidirection(bool bLock);
	bool IsLockBidirection()const	{	return	m_bLockBidirection;	}

	void	SetUnderMyControl(bool bUnderMyControl);
	bool	IsUnderMyControl();
	bool	IsMyPet();
	bool	IsMySubPlayer();

	bool	GetCallerIsMe();

	BM::GUID const &GetPilotGuid();
	
	short	GetComboCount()	{	return	m_sComboCount;	}
	void	SetComboCount(short sCount)	{	m_sComboCount = sCount;	}

	void	StartNormalAttackFreeze();
	void	StopNormalAttackFreeze();
	int	GetNormalAttackFreezeElapsedTime();

	void	SetTargetHeadSize(float const fTargetHeadSize, float const fTransitSpeed = 1.0f);
	void	SetDefaultHeadSize(float const fDefaultHeadSize) { m_fDefaultHeadSize = fDefaultHeadSize; }
	float	GetDefaultHeadSize() const {return m_fDefaultHeadSize;}

	void	SetInvisible(bool const bTrue);
	bool	IsInvisible() const	{ return	m_bInvisible || m_bHide;	}
	void	SetStun(bool const bTrue);
	bool	IsStun() const { return	m_bStun; }
	void	SetHide(bool const bHide);
	void	SetHideBalloon(bool const bHide);
	bool	IsHide() const { return	m_bHide; }
	void	SetBlinkHide(bool const bHide);
	bool	IsBlinkHide() const { return m_bBlinkHide; }
	void	SetHideNameTitle(bool const bHide) { m_bHideNameTitle = bHide; }
	bool	IsHideNameTitle() const { return m_bHideNameTitle; }
	void	SetFreezed(bool const bTrue, bool const bSetAni=true, bool const bDoDmgAction = false);
	bool	GetFreezed() const;
	bool	CanDmgActionOnFreezed() const;
	void	SetNotActionShift(bool const bTrue);
	bool	IsActionShift(PgAction const* pkAction=NULL) const;
	void	SetOnlyMoveAction(bool bTrue)	{	m_bOnlyMoveAction = bTrue;	}
	void	SetOnlyDefaultAttack(bool bTrue)	{	m_bOnlyDefaultAttack = bTrue;	}
	bool	IsOnlyMoveAction()	const	{	return	m_bOnlyMoveAction;	}
	bool	IsOnlyDefaultAttack()	const	{	return	m_bOnlyDefaultAttack;	}
	bool	IsBlowUp()	const	{	return	m_bBlowUp;	}
	void	SetBlowUp(bool const bBlowUp);
	float	GetBlowUpStartTime()	const	{	return	m_fBlowUpStartTime;	}
	bool	IsMeetFloor() const	{ return m_bFloor; }
	void	SetMeetFloor(bool bMeetFloor) 
	{	
		m_bFloor = bMeetFloor;	
		if(bMeetFloor)
		{
			m_fBlowUpStartTime = 0;
		}
	}
	bool	IsCheckMeetFloor() const	{ return m_bCheckMeetFloor; }
	void	SetCheckMeetFloor(bool bCheckMeetFloor) {	m_bCheckMeetFloor = bCheckMeetFloor;	}
	bool	IsMeetSide() const	{ return m_bSide; }
	void	SetMeetSide(bool bMeetSide) {	m_bSide = bMeetSide;	}
	
	bool	GetSlide() const	{ return m_bSlide; }
	void	SetSlide(bool bSlide)	{	m_bSlide = bSlide;	}

	void	SetBackMoving(bool bBackMoving)	{ m_bBackMoving = bBackMoving;	}
	void	SetNoConcil(bool bNoConcil)	{ m_bNoConcil = bNoConcil;	}
	void	SetNoFindPathNormal(bool bNoFindPathNormal)	{ m_bNoFindPathNormal = bNoFindPathNormal;	}
	CLASS_DECLARATION_S(float, UseSmoothShow);	

	PgActionEffectStack*	GetActionEffectStack()	{	return	m_pkActionEffectStack;	}

	bool	IsShowWarning()const { return m_bShowWarning; }

public:
	//! Model을 업데이트 해야 하는지 알려준다.
	NiAVObject *GetUIModelUpdate();
	
	//! UIModel의 업데이트 여부를 설정한다.
	void SetNeedUIModelUpdate(bool bFlag);

	void	SetDefaultMaterialNeedsUpdateFlag(bool bFlag);

	int GetAttachSlotNo();
	bool AttachTo(int iSlot, char const *pcTargetName, NiAVObject *pkObject);
	bool AttachToPoint(int iSlot, NiPoint3 kLoc, NiAVObject *pkObject);
	NiPoint3 GetParticleNodeWorldPos(int iSlot, char *strNodeName);
	NiAVObject* GetParticleNode(int iSlot, char const* strNodeName);
	bool IsAttachParticleSlot(int const iSlot)const;

	bool DetachFrom(int iSlot, bool bDefaultThreadDelete = false);

	virtual bool AttachToSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax);

	//! 무기 사운드(피격, 방어)를 플레이한다.
	void PlayWeaponSound(PgWeaponSoundManager::EWeaponSoundType eType, PgActor *pkPeer, char const *pcActionID, float fVolume = 0.0f, PgActionTargetInfo* pkTargetInfo = NULL);
	bool PlayNewSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin = 80, float fDistMax = 180);
	bool AddNewParticle(char const *szParticleID, int const iSlot, char const *szTargetName, float const fScale,
						bool const bLoop=false, bool const bZTest=true, float const fSoundTime=0.f, bool const bAutoGround=false, bool const bUseAppAccumTime=false, bool bNoFollowParentRotation =  false);

	stSkillCastingInfo*		GetSkillCastingInfo()	{	return	&m_SkillCastingInfo;	}
	stSkillCoolTimeInfo*	GetSkillCoolTimeInfo()	{	return	&m_SkillCoolTimeInfo;	}
	TOGGLE_SKILL_SET const *	GetSkillToggleInfo()	{	return	&m_SkillToggleInfo;		}
	PgSkillSetAction & SkillSetAction() { return m_SkillSetAction; }

	bool const IsInCoolTime(unsigned long ulSkillNo, bool& rbIsGobalCoolTime) const;

	virtual void	StartSkillCoolTime(unsigned	long const ulSkillNo);
	void			ReCalcCoolTime(int const iCoolTimeRate);

	void	CutSkillCoolTime(unsigned long const ulSkillNo);
	void	StartSkillCasting(unsigned long const ulSkillNo);
	void	SkillCastingConfirmed(unsigned long	const ulSkillNo, short const sErrorCode);
	void	CutSkillCasting(unsigned long const ulSkillNo);

	void	StartSkillToggle(unsigned long const ulSkillNo);
	void	CutSkillToggle(unsigned long const ulSkillNo);

	void	SetComboCharge(float const fChargingTime);
	void	CallComboCharge();
	void	CutComboCharge();

	void	SetHideShadow(bool const bHide);
	void	SetHideMiniMap(bool const bHide);
	void	SetCanHit(bool const bCanHit);
	bool	GetCanHit() const;

	void	StartGodTime(float fTotalGodTime);
	bool	IsGodTime() const	{	return	m_kGodTimeInfo.m_bGodTime;	}

	void	AbilChangedByEffect(BM::GUID const &kCaterGUID, WORD wAbilID, int iAbilValue, int iDelta);

	void UseSkipUpdateWhenNotVisible(bool bUse)	{	m_bUseSkipUpdateWhenNotVisible = bUse;	}

	//!	ActorManager가 완전히 로딩이 끝났을때, 호출해주는 메소드(일종의 노티파이 메소드?)
	virtual	void	NfyActorManagerLoadingComplete();
	virtual	void	NfyActorManagerChanged(bool bBefore);

	void	CheckTouchDmg();	//	다른 몬스터와 충돌체크

	NiPoint3 GetPosition(bool bDebugPos = false);
	virtual bool SetPosition(NiPoint3 const &rkTranslate);

	void RestoreTexture();
	void ChangeTexture(NiNode *pkRoot);		// 바리에이션 텍스쳐가 있으면 텍스쳐를 바꾸어준다.

	void	SetControllerShapeHit(NxControllerShapeHit const &kHit)	{	m_kControllerShapeHit = kHit;	}
	NxControllerShapeHit	const&	GetControllerShapeHit()	{	return	m_kControllerShapeHit;	}

//	void SetActorDead(bool bDead) { m_bDead = bDead; }
//	bool GetActorDead() { return m_bDead; }
	bool GetActorDead();

	void	DetachActorAlphaProperty(NiAVObject *pkAVObject);
	void	AttachActorAlphaProperty(NiAVObject *pkAVObject);
	virtual void SetUseSubActorManager(bool bUse);

	const	PartsAttachInfo&	GetPartsAttachInfo()		const	{	return	m_kPartsAttachInfo;	}

	static void SetNameVisible(bool bVisible)	{	m_stbNameVisible = bVisible; }
	static bool GetNameVisible()				{	return m_stbNameVisible; }

	static void SetVisiblePCName(bool const bVisible)		{ m_bDrawNamePC = bVisible; };
	static void SetVisibleNPCName(bool const bVisible)		{ m_bDrawNameNPC = bVisible; };

	static	void	SetAutoGetItemMoney(bool const bEnable)	{	m_bAutoGetItemMoney = bEnable;	}
	static	void	SetAutoGetItemEquip(bool const bEnable)	{	m_bAutoGetItemEquip = bEnable;	}
	static	void	SetAutoGetItemConsume(bool const bEnable)	{	m_bAutoGetItemConsume = bEnable;	}
	static	void	SetAutoGetItemETC(bool const bEnable)	{	m_bAutoGetItemETC = bEnable;	}

	void SetActiveGrp(int iGroup, bool bUse);

	bool GetNameColor(std::wstring &rkOut);
	void GetNameEmoticon(std::wstring &rkOut);
	bool GetGuildNameColor(NiColorA &kColor);
	void GetEnchantPrefixName(std::wstring& rkOut, std::wstring const& rkNameFont, std::wstring const& rkNameColor);

	void	SetEventScriptIDOnDie(int iEventID)	{	m_iEventScriptIDOnDie = iEventID;	}
	int		GetEventScriptIDOnDie()	const {	return	m_iEventScriptIDOnDie;	}

#ifdef PG_SYNC_ENTIRE_TIME
	static DWORD GetSynchronizedTime();
	static DWORD GetLastSentTime();
	static DWORD GetAverageLatency();

	static void SyncEntireTime(DWORD dwTime);
	static void SetLastSentTime(DWORD dwLastSentTime);
	static void SetAverageLatency(DWORD dwTimeDiff);
#endif

	void ResetActiveGrp();
	void PrintItemInfo();

	void RestoreLockBidirection();

	PgHeadBuffIconList*	GetHeadBuffIconList()	{	return	m_pkHeadBuffIconList;	}
	void AddHeadBuffIcon(int const iEffectID);
	void RemoveHeadBuffIcon(int const iEffectID);

	void	SetInstallTimerGauge(float fInstallTotalTime = 10.0f);
	void	DestroyInstallTimerGauge();

	void	SetAliveTimeGauge(float fAliveTotalTime);
	void	DestroyAliveTimeGauge();

	bool	CheckRequirementForAction(PgAction *pkNextAction,bool const bShowFailMsg=false);

	void SetUpdatePhysXFrameTime(float fFrameTime);

	void	ResetSkillCoolTimeFromUnit();

	void SetNodeHide(char const* strNodeName, bool bHide);
	void SetNodeAlpha(char const* strNodeName, float fAlpha);
	void SetNodeAlphaRecursive(NiAVObject* pkObject, float fAlpha);
	void SetNodeAlphaChange(char const* strNodeName, float fAlphaStart, float fAlphaEnd, float fChangeTime);
	void UpdateNodeAlpha();

	//! 가지고 있는 무기의 번호
	unsigned int GetMyWeaponNo() const { return m_uiMyWeaponNo; }
	void SetMyWeaponNo(unsigned int const uiNo) { m_uiMyWeaponNo = uiNo; }

	bool AttachAttackEffect(char const* szActionName, int const iSlot = 0);

	void SetAnimationStartTime(float fTime) { m_fAnimationStartTime = fTime; }
	float GetAnimationStartTime() const { return m_fAnimationStartTime; }

	bool IsUseBattleIdle()	{ return m_bUseBattleIdle; } const
	void SetUseBattleIdle(bool bValue)	{ m_bUseBattleIdle = bValue; }

	BYTE	GetMyWeaponAnimFolderNum() { return m_byMyWeaponAnimFolderNum; }
	void 	SetMyWeaponAnimFolderNum(BYTE byMyWeaponAnimFolderNum) { m_byMyWeaponAnimFolderNum = byMyWeaponAnimFolderNum; }

	void AddTransformEffectAttachInfo(int const iTransformEffectID, PgIWorldObjectBase::stEffectAttachInfo const & kInfo);
	bool RemoveTransformEffectAttachInfo(int const iTransformEffectID);

	PgPOTParticle GetPOTParticleInfo() const;

	//void DetachPOTParticle(EInvType kInvType,EEquipPos kItemPos,int iItemNo);
	void RemoveCompletedItemSet(int const iSetNo );
	
	void AddDivideReservedTransit(EItemDivideReservedType const eType, SItemPos const& kItemPos, int const iCalcValue, BM::Stream & kAddonPacket);
	void DoDivideReservedTransit(BM::GUID const& kSourceGuid, BM::GUID const& kDivideGuid);

	void SetState(EUnitState const eState);
	
	int GetCurOriginSeqID() const;
	bool IsRidingPet(void) const { return (m_pkMountedRidingPet != NULL); }
	PgActor* GetMountTargetPet(void) const { return m_pkMountedRidingPet; }
	void SyncMountPet(void);
	bool MountPet(void);
	bool UnmountPet(void);
	PgActor* GetPetActor(void);
	virtual NiNode* GetNodePointStar(void); //펫에 탑승했을땐 이름 노드가 달라진다.

	float GetAnimationTime(std::string const& strActionName);
	float GetAnimationSpeed(std::string const& strActionName);

	void SetLeaveExpedition(bool const& bValue);
	bool const GetLeaveExpedition() const { return m_bLeaveExpedition; }
	bool CheckOutLobby(PgTrigger * pTrigger);	// 원정대 로비에서 나갈 것인지 체크(PgTrigger::OnAction에서 함)

	NiAVObjectPtr GetCharRoot(void);

	void SetDuelWinnerTitle(void)
	{
		if(GetUnit())
		{
			m_spDuelTitle->Set(GetUnit()->GetAbil(AT_DUEL_WINS));
		}
	}

	void SetEffectCountDown(int const iEffectNo, int const iCountDown);
	WORD GetEffectCountDownSec()const;
	void UpdateEffectCountDonw(float const fFrameTime);
	void DelEffectCountDown(int const iEffectNo);

	void IgnoreSlide(bool const bIgnore) { m_bIgnoreSlide = bIgnore; }
	bool IgnoreSlide() const { return m_bIgnoreSlide; }
	
	int CheckItemSetComplete(EInvType eInvType, int const iEquipItemNo, bool& rbOutIsComplete);
protected:

	void	UpdateBottomRayHit(NxVec3 const &kNewPosition);

	virtual void AttachChild(NiAVObject* pkChild, bool bFirstAvail = true);
	//! PgInput에 따른 행동을 취한다.
	bool DoAction(PgAction *pkAction,bool bForceToTransit = false);
	virtual void onHPChanged();

	virtual bool checkVisible();
	virtual	void DoLoadingFinishWork();	//	로딩이 완료되었을때 처리

	void	CheckInvalidSpace();	//	있을 수 없는 공간에 캐릭터가 위치하고 있는지 체크한다.

	void	SetPositionChanged(bool bChanged)	{	m_bPositionChanged = bChanged;	}
	bool	GetPositionChanged()	{	return	m_bPositionChanged;	}	


	float	GetRotationInterpolTime()	const	{	return	m_fRotationInterpolTime;	}
	void	SetRotationInterpolTime(float fTime)	{	m_fRotationInterpolTime = fTime;	}

	float	GetInitialVelocity()	const	{	return	m_fInitialVelocity;	}
	void	SetInitialVelocity(float fVelocity)	{	m_fInitialVelocity = fVelocity;	}

	float	GetSlideStartTime()	const	{	return	m_fSlideStartTime;	}
	void	SetSlideStartTime(float fTime)	{	m_fSlideStartTime = fTime;	}

	void UpdateStatusEffect(float fAccumTime, float fFrameTime);

	void MoveParticlesToTarget(PgActor* pkTarget);
	
	void Update_IsAbleSlide(float const fAccumTime, float const fFrameTime, bool& bDoNotSlide);
	void InitControllerShapeHit();
private:
	void AddItemEffect(int const iItemNo);
	void RemoveItemEffect(int const iItemNo);

	int GetAppearanceItemNo(EEquipPos const eEquipPos, int& riType,
		bool const bCheckEffect=true, 
		bool const bCheckCashItemSet=true, bool const bCheckNormalItemSet=true,
		bool const bCheckCash=true		 , bool const bCheckNormal=true );

	void	DetachNameNodes(NiAVObject* pkNIFRoot, char const* const szTargetNameNode = ATTACH_POINT_STAR);
	void	AttachNameNodes(NiAVObject* pkNIFRoot, char const* const szTargetNameNode = ATTACH_POINT_STAR);

	bool	SetHPBarValue( int const iBefore,int const iNew );	
	
	void	CheckItemUseTime();

	inline	void	UpdateHeadSize(float fFrameTime,float fAccumTime);	//	머리크기 업데이트
	inline	void	UpdateGodTime(float fAccumTime);	//	무적시간 업데이트
	inline	void	UpdateDamageBlink(float fAccumTime);	//	대미지 블링크 업데이트
	inline	void	UpdatePush(float fFrameTime = 1.0f);	//	밀기 업데이트
	inline	void	UpdateScale();	//	스케일 업데이트
	inline	void	UpdateTwist();	//	회전처리
	inline	void	UpdateBlink(float fModifiedFrameTime);	//	깜빡임 처리
	inline	void	UpdateItemEquip();
	void	UpdateActorManager(float fAnimationAccumTime);	//	액터 매니저 업데이트
	inline	void	UpdateTrigger();	//	트리거 업데이트
	inline	float	CalcAnimationAccumTime(float fAccumTime,float fFrameTime);
	void	UpdateWaveEffect(float fAccumTime);
	

	void	OnTargetListModified(PgAction *pkNextAction);
	void	OnCastingCompleted(PgAction *pkNextAction);
	bool	ProcessToggleAction(PgAction *pkNextAction);
	bool	ProcessLeaveCurrentAction(PgAction *pkNextAction);
	void	SetActionParam(PgAction	*pkNextAction);
	void	SetIgonreDamageEffect(PgAction	*pkNextAction);
	void	PlayAnimation(PgAction *pkNextAction);
	void	SetSeeFrontAttribute();

	void	UpdateShakeActor();
	void	UpdateHPGaugeBarPosition(NiCamera *pkCamera);

	void	DrawParticle(PgRenderer *pkRenderer,bool bOnlyZTestEnable);
	void	DrawHeadBuffIconList(PgRenderer *pkRenderer,NiCamera *pkCamera);

	int GetNextSetEffectSlotIndex(); //세트 이펙트가 붙을 이펙트 슬롯 인덱스를 돌려준다.

	void	ApplyHairType(PgItemEx* pkHair,PgItemEx* pkHelm);

	bool GetCoupleItemColor(CONT_NI_COLOR &kContColor);

	PgTrailNodePtr GetTrailNode(eTrailAttachType const eTrailType);
	bool			SetNodeTrail(eTrailAttachType const eTrailType, PgTrailNodePtr& rkNodeTrail);
	
	//int CheckItemSetComplete(EInvType eInvType, int const iEquipItemNo, bool& rbOutIsComplete);
	void AddCompletedItemSet(int const iSetNo);	// 현재 입고 있는 세트 아이템 번호를 추가
protected:
    virtual bool IsDrawable( PgRenderer* pkRenderer, NiCamera* pkCamera );
	virtual void	DrawNameText(PgRenderer *pkRenderer,NiCamera *pkCamera);

protected:
	NiQuaternion m_kToRotation;
	float m_fRotationInterpolTime;

	NxControllerShapeHit	m_kControllerShapeHit;
	PgControllerHitCallBack	m_kControllerHitCallBack;

	NxRaycastHit	m_kBottomRayHit;

	bool m_bFloor;
	bool m_bCheckMeetFloor;
	bool m_bSlide;
	bool m_bSide;
	bool m_bDoSimulateOnServer;
	DWORD m_dwLastSimulatedTime;
	
	NiPoint3 m_kSourcePos;

	NiPoint3 m_kVelocity;

	bool m_bBackMoving;

	bool m_bNoConcil;

	bool m_bNoFindPathNormal;

	bool m_bFreeMove;

	bool m_bFalling;

	bool m_bLockBidirection;

	bool m_bRiding;

	bool m_bTransformation;

	bool	m_bCheckCamColl;
	float	m_fCheckCam_FirstDValue;

	float	m_fStartScale,m_fTargetScale;
	unsigned	long	m_ulScaleChangeStartTime;
	unsigned	long	m_ulTotalScaleChangeTime;

	NiAVObject* m_pAlphaNode;
	float m_fStartAlpha;
	float m_fEndAlpha;
	unsigned long m_ulAlphaStartTime;
	unsigned long m_ulAlphaChangeTime;

	bool m_bLoadingComplete;
	//! 마지막에 바닥에 닫아있던 좌표.
	NiPoint3	m_kLastFloorPos;

	//! 이전의 캐릭터 월드 좌표(죽어서 날아가는 캐릭터의 카메라 부딛힘 처리에 사용된다.)
	NiPoint3	m_kPrevWorldPos;

	//!	캐릭터의 이전 컨트롤러 좌표
	NxVec3	m_vPrevControllerPos;


	float	m_fLastCheckItemUseTime;

	//! Alpha Property
	NiAlphaPropertyPtr m_spAlphaProperty;
	
	//! 파츠 부착 목록
	PartsAttachInfo	m_kPartsAttachInfo;	

	ItemEquipInfo	m_kItemEquipInfo;	//	어떤 아이템이 장착상태인지,장착되지 않은 상태인지(컨테이너 내에 있다면 장착, 그렇지 않다면 비장착)

	//! Current Animation Sequence ID
	NiActorManager::SequenceID m_kSeqID;

	//! Action
	PgAction *m_pkAction;

	//! Action Queue
	ActionQueue m_kActionQueue;

	//! 캐릭터가 바라보는 방향
	NiPoint3 m_kLookingDir;

	//! 캐릭터 이동 진행 방향
	NiPoint3 m_kMovingDir;

	//! 캐릭터의 이동 방향을 Path를 기준으로 8방향으로 알려 줌
	BYTE m_byMovingDirection;

	//! 캐릭터의 이동 방향중 이전 방향을 기록해 둔다. DIR_NONE가 아닌 것으로만
	BYTE m_byLastMovingDirection;

	//!	캐릭터가 마지막으로 어느 방향을 보았는가
	BYTE m_byLastDirection;

	//!	현재 무기 궤적을 그리는 중인가?
	bool m_bNowDrawWeaponTrail;

	//!	현재 몸 궤적을 그리는 중인가.
	bool m_bNowDrawBodyTrail;

	//! 최근 Normal Attack 의 종료 시간
	unsigned long m_ulNormalAttackEndTime;

	//! 머리위에 텍스트 풍선
	PgTextBalloon* m_pTextBalloon;

	//! 머리위에 마크
	PgMarkBalloon* m_pMarkBalloon;

	//! 머리위에 파티 이름
	PgPartyBalloon* m_pPartyBalloon;

	//! 머리위에 원정대 이름
	PgExpeditionBalloon* m_pExpeditionBalloon;

	//! 머리 위에 노점 이름
	PgVendorBalloon* m_pVendorBalloon;

	//! 무기 궤적 그려주는 클래스
	PgTrailNodePtr m_spWeaponTrailNode;
	//!	몸 궤적 그려주기
	PgTrailNodePtr m_spBodyTrailNode;

	CONT_TRAIL m_kContTrail;
	//!	원형 그림자
//	PgCircleShadow *m_pCircleShadow;

	//! 이름 렌더링
	PgTextObjectPtr m_spTitleName;
	PgTextObjectPtr	m_spNameText;
	PgTextObjectPtr	m_spGuildNameText;
	PgAchievementTitlePtr m_spAchievementTitle;
	PgAchievementTitlePtr m_spGIFTitle;

	PgTextObjectPtr m_spCustomCountText;
	
	PgGuildMarkPtr m_spGuildMark;

	PgGuardianMarkPtr m_spGuardianMark;

	PgMyhomeMarkPtr m_spMyhomeMark;

	PgDuelTitlePtr m_spDuelTitle;
	PgEffectCountDownPtr m_spEffectCountDown;

	//! 참이면 캐릭터가 깜박거리며 없어진다.
	bool m_bBlink;
	bool m_bBlinkHide;	//	블링크에 의해 순간적으로 안보이는 상태인가.

	//! 한 번 깜박하는데 필요한 시간
	float m_fBlinkAccumTime;

	//! 깜박거리는 주기
	float m_fBlinkFreq;

	bool m_bSendBlowStatus;	//	띄워져서 날아가는 상태에 있을때, 현재 좌표를 서버로 보낼것인가.

	//!	몬스터가 타겟으로 인지한 시간
	float	m_fLastAttackTargetedTime;
	//!	몬스터가 타겟으로 인지한 타겟 GUID
	BM::GUID m_kLastAttackTargetGUID;

	//! Callback Object
	ActorCallbackObject *m_pkActorCallback;

	//! 현재 트리거
	PgTrigger	*m_pkCurrentTrigger;
	bool		m_bCurrentTriggerAct;

	

	//! PhysX 관련
	NiPhysXKinematicSrc *m_pkPhysXSrc;
	NiPhysXTransformDest *m_pkPhysXDest;
	NxActor *m_pkPhysXActor;

	NxActor *m_apkPhysXCollisionActors[PG_MAX_NB_ABV_SHAPES];
	NiPhysXKinematicSrc *m_apkPhysXCollisionSrcs[PG_MAX_NB_ABV_SHAPES];
	bool m_bIsOptimizeSleep;	//OptimizeSleep기능을 킬 것인가. 몬스터가 회전안되는 문제 해결

	//! 동기화 보간 정보
	float m_fSyncInterpolTime;
	NxVec3 m_kSyncInterpolDelta;
	NiPoint3 m_kSyncPosition;
	
	//! 움직이는 속도 조절
	float m_fMovingSpeedScale;

	//! 가지고 있는 무기의 종류
	unsigned int m_uiMyWeaponType;

	//!	가지고 있는 무기의 장착제한
	eEquipLimit	m_kMyWeaponEquipLimit;	
	
	//!	가지고 있는 무기의 애니메이션 폴더 번호
	BYTE m_byMyWeaponAnimFolderNum;

	//!	현재 액션을 시작할때 착용하고 있었던 무기의 애니메이션 폴더 번호
	BYTE m_byWeaponAnimFolderNumAtActionStart;

	//!	가지고 있는 무기의 번호
	unsigned int m_uiMyWeaponNo;

	//! 변신 하기 전의 원래 액터 
	BM::GUID m_OriginalActorGUID;

	//!	유저에게 줄 수 있는 퀘스트 목록
	//PgQuestSimpleInfoPool *m_pkQuestSimpleInfoPool;

	void UpdateSkillInfos();

	//! 펫이라면, 주인의 Pilot Guid를 가지고 있다, xxxxxxxxxxxxxx 닦을것.
//	BM::GUID m_kMasterGuid;
#ifdef PG_USE_CAPSULE_CONTROLLER
	NxCapsuleController *m_pkController;
#else
	NxBoxController *m_pkController;
#endif

	//! 캐릭터가 이동할 좌표. (변위가 아닌 절대)
	NxVec3 m_kMovingAbsolute;

	//! 이전 캐릭터가 이동한 변위
	NxVec3 m_kPrevMovingDelta;

	//! 캐릭터가 이동할 변위
	NxVec3 m_kMovingDelta;

	//! 미끄러질 방향
	NxVec3 m_kSlideVector;

	//! ActionQueue Mutex
	Loki::Mutex m_kActionQueueMutex;

	//! Charactor Controller 의 Active Collision Group
	NxU32 m_uiActiveGrp;

	//! 보여줄까 말까.
	bool m_bHide;	//	나를 포함한 모든 사람에게 보이지 않게된다.
	bool m_bInvisible;	//	적에게만 보이지 않는다. 아군에게는 반투명상태로 보이게된다.
	bool	m_bShadowHide;	//	나의 그림자를 그리지 않는다.
	bool m_bMiniMapHide;	//	캐릭터 이름, 미니맵, NPC 리스트에 출력하지 않는다.
	bool m_bHideNameTitle;

	//! 때릴 수 있느냐 - Target으로 잡힐것이냐 말것이냐.
	bool m_bCanHit;

	//!	무적상태인가
	stGodTimeInfo	m_kGodTimeInfo;


	bool m_bVisible;
	InvisibleGrade m_eInvisibleGrade;
	static float const ms_kUpdateIntervalByInvisibleGrade[MAX_NUM_INVISIBLE];
	float m_kLastUpdateTimeByInvisibleGrade[MAX_NUM_INVISIBLE];
	bool m_kCanUpdate[MAX_NUM_INVISIBLE];
	NiPoint3 m_kNormalizedActorPosByCamera;

	//float m_fLastDownwardPassUpdateTime;
	//float m_fLastUpdateTime;
	//float m_fStridenUpdateTime;

	//! 채팅 입력도중 인가?
	bool m_bInputNow;

	//!	현재 몇번째 콤보 공격을 하고 있는가?
	short m_sComboCount;

	//!	콤보를 다 한후 기본 공격이 잠시 경직되는 시간.
	unsigned	long	m_ulNormalAttackFreezeStartTime;

	//! 점프중인가?
	bool m_bJump;

	//! Actor의 좌표를 가능한 좌표로 수정할수 있는가?
	bool m_bAdjustValidPos;

	//! 캐릭터 좌표를 강제 이동 시도(Sync)를 할 수 있는가?
	bool m_bForceSync;

	//! 모델을 업데이트 해야 하는가?
	bool m_bNeedToUpdateUIModel;

	//! 점프 한 시간
	float m_fJumpTime;

	//! 점프 한 높이
	float m_fJumpAccumHeight;

	//! 몇 바퀴 돌릴 것인가?
	//int m_iTwistTimes;

	//! Twist할 속도
	//float m_fTwistSpeed;

	//! Twstr할 각도
	//float m_fTwistDegree;

	//! Twist할 때 쓸 Local Transform정보
	NiTransform m_kLocalTransform;

	//! 애니메이션 속도 조절 총 시간
	float	m_fTotalAnimSpeedControlTime;
	float	m_fAnimSpeedControlStartTime;
	float	m_fAnimSpeedControlValue;
	float	m_fOriginalAnimSpeed;
	float	m_fAccumTimeAdjust;
	float	m_fBeforeAccumTime;

	float	m_fLastDownwardPassUpdateTime;
	
	//! 캐릭터 흔들기
	int	m_iTotalShakeTime;
	int	m_iShakeStartTime;
	float	m_fShakeValue;
	NiPoint3	m_kOriginalPos;

	//! 점프 초기 속도
	float m_fInitialVelocity;	

	stPush	m_Push;	//	캐릭터 밀기에 사용됨.

	float	m_fLastItemPickUpTime;
	NiPoint3	m_kLastItemPickUpPos;
	ActorPickUpInfoCont	m_kActorPickUpInfoCont;

	//! UserController Hit Report
	PgControllerHitReport m_kControllerHitReport;	

	NiPick *m_pkPick;

	stSkillCoolTimeInfo	m_SkillCoolTimeInfo;
	stSkillCastingInfo	m_SkillCastingInfo;
	TOGGLE_SKILL_SET	m_SkillToggleInfo;
	PgSkillSetAction	m_SkillSetAction;

	//! 현재 입고 있는 세트 아이템 번호(세트 아이템을 입게 되면 추가 되며, 세트 아이템을 벗을시에 사라진다)
	CONT_SET_ITEM_SLOT	m_kContCurSetNo; 
	int					m_kGenerateSetEffectSlotIndex;
	//! 아이템을 착용하지 않았을 때 입는 기본 아이템
	DefaultItemContainer m_kDefaultItem;

	//! Attach될 아이템을 임시로 보관하는 Conatiner
	AttachItemContainer m_kAttachItemContainer;
	Loki::Mutex m_kAttachItemLock;

	//! 아이템에 붙은 애니를 관리하는 AM Container
	AMContainer m_kSupplementAMContainer;

	//! 스크립트에서 사용하는 파라메터 저장소
	ScriptParamContainer m_kScriptParamContainer;

	//! Material Property를 이미 다 찾았는가?
	bool m_bMaterialCached;

	//! Material Color Property를 이미 다 찾았는가?
	bool m_bMaterialColorCached;
	
	//! Material Container;
	MaterialContainer m_kMaterialContainer;

	//!	Specular	Container
	SpecularEnableContainer	m_kSpecularContainer;

	//! Material Color Container
	MaterialColorContainer m_kMaterialColorContainer;

	//! Material Currnet Color Container
	MaterialColorContainer m_kMaterialCurrentColorContainer;

	//! Material to Alpha(has alpha testing) Container
	AlphaPropContainer m_kAlphaPropContainer;

	stAlphaTransitInfo	m_AlphaTransitInfo;
	stColorTransitInfo	m_ColorTransitInfo;
	stColorTransitInfo	m_SpecularTransitInfo;

	bool	m_bSpecularOn;

	// 지정한 곳으로 가게 할 정보들
	NiPoint3 m_kTargetDir;
	NiPoint3 m_kTargetLoc;
	NiPoint3 m_kStartLoc;
	float	m_fWalkToTargetLocSkillRange;
	float m_fDiffLength;
	bool m_bWalkingToTargetForce;
	bool m_bWalkingToTarget;
	bool m_bCheckCliff;
	std::string m_kTargetWalkingNextAction;

	//!	SyncMove
	NiPoint3	m_kSyncMoveStartPos;
	PgAction	*m_pkSyncMoveNextAction;
	float	m_fSyncMoveStartTime;

	bool	m_bBlowUp;	//	공중에 띄워져 있는 상태인가
	float	m_fBlowUpStartTime;

	//!	머리 크기(상태이상에 의해 사이즈가 변할 수 있다)
	float	m_fTargetHeadSize;
	float	m_fCurrentHeadSize;
	float	m_fDefaultHeadSize;
	float	m_fHeadSizeTransitSpeed;

	//!	스턴된 상태인가.
	bool	m_bStun;

	//!	얼어붙은 상태인가?(애니메이션이 멈추고, 다른 액션으로의 전이가 강제로 막히게 된다)
	int	m_iFreezeStatus;

	//! 액션전이를 하지 않아야 하는 경우
	bool	m_bNotActionShift;

	//!	오로지 이동 액션만 가능한 상태인가(박스등으로 변신했을 때)
	bool	m_bOnlyMoveAction;
	//!	오로지 기본 공격만 가능한 상태인가.
	bool	m_bOnlyDefaultAttack;
	//! Actor가 점프시에 영향을 받는 중력
	static float ms_fGravity;
	//float m_fLocalUpDownSpeed; //! 따로 ReverseGravity에 쓰려고 넣었다.;;
	//bool m_bUseLocalUpDownSpeed;

	//! Default Ambient Color
	static NiColor ms_kDefaultAmbient;

	//! Default Emissive Color
	static NiColor ms_kDefaultEmissive;

	//!	Default	Specular	Color
	static NiColor ms_kDefaultSpecular;

	//!	Default	Specular	Color
	static NiColor ms_kDefaultDiffuse;
	//!	Action Effect
	PgActionEffectStack	*m_pkActionEffectStack;

	//CLASS_DECLARATION_PTR(void*, m_pInfoUI, InfoUI);

	//! Don't Display Name
	bool m_bNoName;

	bool m_bDownState;
	float m_fTotalDownTime;

	bool m_bTraceUpdate; //! true면.. update되는 위치를 로그로 남긴다 디버그용.
	NxExtendedVec3 m_kLastFramePos;

	// 처음 Update를 탈 때 EquipCount 가 0이면 로딩이 완료된 것이다.
	LONG m_iEquipCount;

	static bool m_stbNameVisible;

	//!	액터가 화면 밖에 있을때 업데이트를 스킵하는 처리를 해줄것인가.
	bool	m_bUseSkipUpdateWhenNotVisible;

	//	대미지 입었을 때 빨간색으로 블링크 시키는 기능
	stDamageBlinkInfo	m_kDamageBlinkInfo;
	
	//! 초기 액션
	std::wstring m_kDefaultAction;

	bool m_bTransformed;
	std::string	m_kTransformedActorID;
#ifndef EXTERNAL_RELEASE
	DWORD m_dwLastTransitTime;
	DWORD m_dwLastTransitFrame;
#endif

	//! 마지막으로 액션한 시간.
	DWORD m_dwLastActionTime;
	DWORD m_dwLastFrameTime;
	DWORD m_dwAccumedOverTime;

	//!	마지막 위치 싱크한 이후로 Jump 액션이 실행된 적이 있는가?
	bool	m_bSyncCrashed;

	//! 속도 보정 할 때 쓸 값
	float m_fVelocityRate;
	bool m_bSyncVelocity;

	//!	현재 위치 싱크 중인가.
	bool m_bSync;
	float m_fElapsedSyncTime;
	NiPoint3 m_kSyncPositionTarget;
	NiPoint3 m_kSyncPositionStart;
	Direction	m_kSyncStartDir;

	//! 보간 후, 처리할 액션의 Latency를 가진다.
	DWORD m_dwActionLatency;

	DWORD m_dwLastUpdateFrame; // 테스트용 변수
	float m_fLastUpdateFrameTime; // 테스트용 변수
	int m_iUpdateCount;

	float	m_fLastWaveEffectUpdateTime;

	bool	m_bLeaveExpedition;		//	원정대 로비를 떠날 것인지 확인하기 위한 변수

	bool	m_bDieReservedByAction;	//	액션 패킷에 의해 죽음이 예약된 상태이다. SetAbil HP==0 이 와도 죽지 않게 한다.
	
	float	m_fAutoDeleteActorStartTime;
	float	m_fAutoDeleteActorTime;	// 세팅되어 있으면 일정 시간뒤 actor는 자동 삭제 된다.

#ifdef PG_SYNC_ENTIRE_TIME
	//! 서버와 맞춰 놓은 시간
	static DWORD ms_dwSyncTime;
	static DWORD ms_dwLastSentTime;
	static DWORD ms_dwLocalSyncTime;
	static DWORD ms_dwAverageLatency;
#endif

	bool m_bIsUnderMyControl;	//	내 액터는 아니지만, 내가 컨트롤하는 액터인가?(소환물,트랩등에 사용된다)

	ReservedActionTable m_kReservedAction;//! 예약된 액션 리스트
	ActionList	m_kReservedTransitAction;	//!	액션 전환 예약
	StatusEffectInstanceList	m_StatusEffectInstanceList;//!	상태이상 리스트
	StatusEffectUpdateList	m_StatusEffectInstanceListForUpdate;//!	틱 돌아야 되는 상태이상 리스트
	IntList	m_IgnoreEffectList;//!	무시할 이펙트 리스트
	VariTextureContainer m_VarTextureList;//! 바리에이션 텍스쳐 리스트
	OrgTextureContainer m_OrgTextureList;//! 소스 텍스쳐 리스트
	std::string	m_kDieParticleID;//!	죽을때 뿌려줄 이펙트 ID
	std::string m_kDieParticleNode;//!	죽을때 뿌려줄 이펙트가 붙을 node이름
	std::string m_kDieSoundID;//!	죽을때 뿌려줄 이펙트가 붙을 node이름
	float m_fDieParticleScale;//!	죽을때 뿌려줄 이펙트 스케일
	IntList	m_ActionToggleState;//!	액션 토클 상태 관리
	int	m_iEventScriptIDOnDie;	//	죽을 때 실행해줄 이벤트 스크립트 ID
	bool m_bUseBattleIdle;		//	정예 이상일때도 베틀아이들을 사용할 것인가

	ActorDropItemInfoList	m_ActorDropItemInfoList;	//	이 액터가 드랍하는 아이템 정보

	PgHeadBuffIconList	*m_pkHeadBuffIconList;	//	머리위에 버프아이콘을 띄워주는 클래스

	mutable Loki::Mutex m_kMutex;

	// 액터가 죽은 경우 일때
	bool m_bDead;

	float m_fSpecifiedFrameTime;
	float m_fLoadingStartTime;

	float	m_fSlideStartTime;

	static bool m_bDrawNamePC;
	static bool m_bDrawNameNPC;

	static	bool	m_bAutoGetItemMoney,m_bAutoGetItemEquip,m_bAutoGetItemConsume,m_bAutoGetItemETC;

	ShadowObject	*m_pkShadow;
	NiAVObjectPtr	m_spSpotLightGeom;

	PgEnergyGauge	*m_pHPGaugeBar;//!	HP 게이지바
	PgObject	*m_pkMountedRidingObject;	//	내가 타고 있는 라이딩 오브젝트
	PgActor		*m_pkMountedRidingPet; //탑승 중일 때, 주인과 펫은 상호 참조 한다.
	NiPoint3	m_kSceneRootPos; //펫 탑승 전 PC의 SceneRoot 로컬위치를 저장하고 탑승해제할 때 복구
	float		m_fScaleOrig; //펫탑승 전 PC의 원본 스케일값

	stFollowInfo	m_kFollowInfo;

	PgActorAppearanceMan	*m_pkActorAppearanceMan;	//	외형 관리자

	bool	m_bPositionChanged;
	std::string	m_kLoadingCompleteInitFunc; //! 로딩 후 초기화 할 것이 있는지 여부 lua 함수 호출
	
	int	m_iGodTimeStatusEffectInstanceID;
	int	m_iDamageBlinkStatusEffectInstanceID;

	IntMap	m_kPartsHideInfo;

	float m_fAnimationStartTime; // 값이 있으면 업데이트를 해당 시간부터 한다.
	SNpcMarkInfo m_kNpcMarkInfo;

	CONT_TRANSFORM_EFFECT_ATTACH_INFO m_kContTransformEffAttachInfo;

	EQUIP_ITEM_CONT	m_kEquipItemCont;// 장착시 대신 보여줘야하는 아이템
	int				m_iOtherEquipItemReturnValue; //m_kEquipItemCont이 있을 경우 이 외에 아이템은 어떻게 처리 할 것인가?
	
	CONT_APPEARANCE_CHANGE_INFO m_kCashChangeItem;
	CONT_APPEARANCE_CHANGE_INFO m_kNormalChangeItem;

	//통과 못하는 피직
	typedef std::vector<NiPhysXKinematicSrc*> NiPhysXKinematicSrcCont;
	typedef std::vector<NiPhysXScenePtr> NiPhysXSceneCont;

	NiPhysXSceneCont	m_kPhysXSceneObjCont;
	NiPhysXKinematicSrcCont	m_vKinematicSrcCont;

	bool	m_bCanRide;
	bool	m_bShowWarning;
	int		m_iOldStrategicPoint;
	PgPOTParticle m_kPOTParticle;
	CLASS_DECLARATION_S(std::string, IdleEffectName);
	CLASS_DECLARATION_S(std::string, IdleEffectNode);

	CONT_ITEM_DIVIDERESERVED m_kContItemDivideReserved;

	CONT_CHANGE_ACTION	m_kContChangeAction;

	SEffectCountDown m_kEffectCountDown;

	bool m_bIgnoreSlide;	// 경사각에서 미끄러질수 있는가? (현재 UT_PLAYER만 이기능을 설정)
	int m_iAttachSlotNo;
	CONT_COMMON_EFFECT m_kContCommonEffect;	//CommonEffect를 Actor에 붙인 정보
	

	CONT_FIXED_SLOT_LIST m_kFixedParticleList; //MoveParticlesToTarget()함수에 의해 옮겨지지 않는 파티클 슬롯 번호의 목록

protected:
	CONT_TEMP_ACTION m_kContTempAction;

public :
	CLASS_DECLARATION(bool, m_bEquipDefaultItem, EquipDefaultItem); // default 옷을 한벌 입었는가?
	CLASS_DECLARATION_S(bool, AlwaysGlowMap); //글로우 맵을 항상 켜줄 것인가?	
	CLASS_DECLARATION_S_NO_SET(SUIMODEL_OPT, UIModelOpt);

/////////////////// PgCustomUI_Summmoner
public:
	void ViewSelectArrow(bool const bShow);

};

NiSmartPointer(PgActor);

bool CheckHPMPForAction(CSkillDef const* pkSkillDef, CUnit const* pkUnit, bool const bShowFailMsg = false, bool const bShowNILog = false);

namespace PgActorUtil
{
	void ExpressAwakeMaxState(PgActor* pkActor);
	PgActor* GetSubPlayerActor(PgActor* pkActor);
	int const GetCalcAniSeqID(int iSeqID, int const iBaseClassID, int const iWeaponAnimFolderNum);	// iSeqID가 1000000 이하일 경우, 계산된 SeqID를 반환해준다
	int const GetOrigAniSeqID(int iSeqID, int const iBaseClassID, int const iWeaponAnimFolderNum);	// iSeqID가 1000000 이상일 경우, 원래 SeqID를 반환해준다	void AdjustParticleScaleByUnitScaleAbil(CUnit* pkUnit, PgParticle* pkParticle);		// 파티클 붙일때 유닛 스케일로 인한
	void AdjustParticleScaleByUnitScaleAbil(CUnit* pkUnit, PgParticle* pkParticle);		// 파티클 붙일때 유닛 스케일로 인한
};

extern int GetTotalSummonedSupply(CUnit const* pkCaller);

#endif //FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTOR_H