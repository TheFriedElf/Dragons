#ifndef FREEDOM_DRAGONICA_CONTENTS_STATUSEFFECT_PGSTATUSEFFECT_H
#define FREEDOM_DRAGONICA_CONTENTS_STATUSEFFECT_PGSTATUSEFFECT_H

#include "NiMain.H"
#include "PgIXmlObject.H"
#include "PgParticle.h"

class PgPilot;
class PgActor;

typedef std::map<std::string, std::string>	CONT_PARAM;

struct stStatusEffectParticle
{
	std::string 	m_kParticleID;
	std::string		m_kAttachTargetNodeID;
	std::string		m_kAttachTargetNodeIDToWorld;
	std::string		m_kFollowRotationTargetNodeID;
	bool			m_bAttachToPoint;
	bool			m_bAttachToWorld;
	bool			m_bNoFollowParentRotation;
	bool			m_bIsAttached; // 실제로 Actor에 붙은 파티클인가?
	BYTE			m_byShowOption;	// 0 : All, 1 : Me, 2 : My Team, 3 : Enemy	
	float			m_fScale;
	int				m_iInstanceID;
	float			m_fPhase;
	NiPoint3		m_kRandomOffsetMin;
	NiPoint3		m_kRandomOffsetMax;
	bool			m_bSeeFront;
	bool			m_bAutoScale;
	bool			m_bNotDetachParticle;
	std::string		m_kParticleSoundID;

	enum eSHOW_OPTION
	{		
		SO_ME = 0,		// 나한테만 보인다.
		SO_MY_TEAM = 1,	// 나와 같은 팀이면 보인다.
		SO_ENEMY = 2,	// 적 한테만 보인다.
		SO_ALL = 7,		// 모두 보여준다.
	};

	stStatusEffectParticle() 
		: m_bAttachToPoint(false), m_bAttachToWorld(false), m_bNoFollowParentRotation(false), m_bIsAttached(false), m_byShowOption(SO_ALL), m_fScale(0.0f), m_fPhase(0.0f)
		,m_kRandomOffsetMin(NiPoint3(0.0f,0.0f,0.0f))
		,m_kRandomOffsetMax(NiPoint3(0.0f,0.0f,0.0f))
		,m_bSeeFront(false)
		,m_bAutoScale(false)
		,m_bNotDetachParticle(false)
	{
		static	int	iInstanceIDGenerator = 0;
		m_iInstanceID = iInstanceIDGenerator++;
	}
};

struct stStatusEffectSkillText
{
	enum eSTATUS_EFFECT_SKILL_TEXT_TYPE
	{
		SESTT_SKILL_TEXT = 0,
		SESTT_SIMPLE_TEXT = 1,
	};

	BYTE m_bySkillTextType;
	int	 m_iSkillTextType;
	bool m_bIsUp;

	stStatusEffectSkillText() :
		m_bySkillTextType(SESTT_SKILL_TEXT),
		m_iSkillTextType(-1),
		m_bIsUp(true)
	{}
};

typedef enum eStatrActionOptionType : BYTE
{
	SAOT_NONE			= 0,
	SAOT_ALL			= 0,
	SAOT_ME				= 1,
	SAOT_NOT_OVERLAP	= 2, //액션 중복 유무
} EStartActionOptionType;

class PgStatusEffect : public PgIXmlObject
{
public:
	enum ITEM_TYPE
	{
		IT_NONE=0,
		IT_COLOR=(1<<0),
		IT_ALPHA=(1<<1),
		IT_HEAD_SIZE=(1<<2),
		IT_PARTICLE=(1<<3),
		IT_INVISIBLE=(1<<4),
		IT_HIDE_PARTS=(1<<5),
		IT_TRANSFORM=(1<<6),
		IT_FREEZED=(1<<7),
		IT_ONLY_MOVE_ACTION=(1<<8),
		IT_SKILL_TEXT=(1<<9),
		IT_ONLY_DEFAULT_ATTACK=(1<<10),
		IT_CHANGE_DEFAULT_ATTACK=(1<<11),
		IT_HIDE=(1<<12),
		IT_STUN=(1<<13),
		IT_PLAY_SOUND=(1<<14),
		IT_INPUT_SLOT=(1<<15),
		IT_DETACH_ACTION=(1<<16),
		IT_START_ACTION=(1<<17),
		IT_INVINCIBLE=(1<<18),
		IT_BODY_SIZE=(1<<19),
		IT_DARK_SIGHT=(1<<20),
		IT_TEXTURE_CHANGE=(1<<21),
		IT_DETACH_PARTICLE=(1<<22),
		IT_WORLD_FOCUS_FILTER=(1<<23),
		IT_INPUTDIR_REVERSE=(1<<24),
		IT_EQUIP_ITEM=(1<<25),
		IT_WHITE_OUT=(1<<26),
		IT_NOT_ACTION_SHIFT=(1<<27),
		IT_HIDE_SHADOW=(1<<28),
		IT_EVENT_SCRIPT=(1<<29),
		IT_HIDE_NODE=(1<<30),
		IT_CHANGE_ACTION=(1<<31),
	};
	typedef enum eHELMET_CHANGE_TYPE
	{
		E_NONE_HELMET_CHANGE_TYPE	   = 0,
		E_HELMET_CHANGE_TYPE		   = 1,
		E_SHORTTERM_HELMET_CHANGE_TYPE = 2,
	}EHELMET_CHANGE_TYPE;

	struct	stInputSlotInfo
	{
		int	m_iSlotID;
		bool m_bEnable;

		stInputSlotInfo(int iSlotID,bool bEnable)	:m_iSlotID(iSlotID),m_bEnable(bEnable)
		{
		}

	};

	typedef struct SFocusFilter
	{
		DWORD dwColor;
		float fAlphaStart;
		float fAlphaEnd;
		float fStartTime;
		float fClearTime;
		SFocusFilter():dwColor(-1), fAlphaStart(0.0f), fAlphaEnd(0.0f)
			,fStartTime(0.0f), fClearTime(0.0f)
		{}
	}SFocusFilter;

	typedef	std::list<stStatusEffectParticle*> ParticleCont;
	typedef	std::list<int> IntCont;
	typedef	std::list<stStatusEffectSkillText*> SkillTextCont;

	typedef	std::map<int, std::string>		RedirectionMap;
	typedef std::vector<stInputSlotInfo>	VInputSlotInfo;
	typedef std::map<int, int>				EQUIP_ITEM_CONT; // 외형만 바꾸어 장착되는 아이템 리스트(ItemPos, ItemNo)
	typedef CAbilObject::DYN_ABIL			CONT_ADD_ABIL;			
	typedef std::map<std::string, std::string>	CONT_CHANGE_ACTION;

public:

	PgStatusEffect()	{ Create(); };
	~PgStatusEffect()	{ Destroy(); };

	virtual bool ParseXml(TiXmlNode const* pkNode, void* pArg = 0, bool bUTF8 = false);

	float	GetHeadSize()	const { return m_fHeadSize; }
	float	GetBodySize()	const { return m_fBodySize; }
	float	GetBodyChangeTime()	const { return m_fBodyChangeTime; }
	int		GetValidItems()	const { return m_ValidItems; }

	NiColorA const&	GetColorA()		const { return m_kColorAlpha;	}
	NiColor const&	GetSpecular()	const { return m_kSpecular; }
	float	GetAlphaStartTransitionTime()	const { return m_fAlphaStartTransitionTime; }
	float	GetAlphaEndTransitionTime()	const { return m_fAlphaEndTransitionTime; }
	float	GetAlphaEnd()	const { return m_fAlphaEnd; }

	int		GetGIFTitleEmoticonID()	const { return m_iGIFTitleEmoticonID; }

	int		GetActivateEventScript()	const { return m_iActivateEventScript; }
	int		GetDeactivateEventScript()	const { return m_iDeactivateEventScript; }

	float	GetColorTransitTime()	const { return m_fColorTransitTime; }
	float	GetColorTransitSpeed()	const { return m_fColorTransitSpeed; }

	std::string const& GetTransformID()				const { return m_kTransformID; }
	std::string const& GetTransformIDForMale()		const { return m_kTransformIDForMale; }
	std::string const& GetTransformIDForFemale()	const { return m_kTransformIDForFemale; }
	std::string const& GetTransformNextAction()		const { return m_kTransformNextAction; }
	int const GetTransformNameNo()					const { return m_iTransformNameNo; }

	char const* GetRedirectionXMLID(int const iEffectValue) const;

	std::string	GetType()	const { return m_kType; }	

	std::string const& GetNewDefaultAttack() const		{ return m_kNewDefaultAttack; }
	std::string const& GetOriginalDefaultAttack() const { return m_kOriginalDefaultAttack; }

	std::string const& GetSoundID() const					{ return m_kSoundID; }
	int const GetLoopCount() const							{ return m_iLoopCount; }

	SkillTextCont	const& GetSkillTextContainer() const	{ return m_kSkillTextContainer; }
	IntCont			const& GetHidePartContainer() const		{ return m_kHidePartContainer; }
	ParticleCont	const& GetParticleContainer() const		{ return m_kParticleContainer; }
	ParticleCont	const& GetDetachParticleContainer() const		{ return m_kDetachParticleContainer; }

	VInputSlotInfo const& GetInputSlotInfo() const			{ return m_vInputSlotInfo; }

	std::string const& GetDetachActionID() const			{ return m_kDetachActionID; }
	std::string const& GetStartActionID() const				{ return m_kStartActionID; }
	EStartActionOptionType const GetStartActionOption() const		{ return m_eStartActionOption; }
	int const GetParamIndex() const							{ return m_kParamIndex; }
	std::string const& GetParamID() const					{ return m_kParamID; }
	int const GetEffectSave() const							{ return m_kEffectSave; }
	
	int GetHideEnable() const								{ return m_iHideEnable; }
	bool GetNotDetachOnDie() const							{ return m_bNotDetachOnDie; }

	typedef std::vector<std::string> TEXTURE_PATH_LIST;
	TEXTURE_PATH_LIST const& GetChangedTextureList() const	{return m_kChangedTextureList;}
	TEXTURE_PATH_LIST const& GetOriginTextureList() const	{return m_kOriginTextureList;}
	TEXTURE_PATH_LIST const& GetContWhiteOutTexture() const	{return m_kContWhiteOutTexture;}
	size_t const GetChangeTextureCount() const { return std::min(m_kChangedTextureList.size(), m_kOriginTextureList.size()); }	//혹여나. 이래야 안전함
	
	void	StartWorldFocusFilter(bool const bMyPlayer) const;
	void	ClearWorldFocusFilter(bool const bMyPlayer) const;

	EQUIP_ITEM_CONT const& GetEquipItemList() const;
	int const GetOtherEquipItemReturnValue() const { return m_iOtherEquipItemReturnValue; }

	std::string const& GetScriptName() const				{ return m_kScriptName; }
	
	CONT_ADD_ABIL const &GetAddAbil()const{return m_kContAddAbil;}
	CONT_PARAM const &GetContParam()const{return m_kContParam;}

	std::string const& GetHideNodeName()const				{ return m_kHideNodeName; }
	
	bool IsDoDmgActionOnFreezed() const { return m_bDoDmgActionOnFreezed; }
	CONT_CHANGE_ACTION const& GetContChangeAction()const	{ return m_kContChangeAction; }
	bool IsOnlyAttachCaster() const { return m_bOnlyCaster; }
	void SetOnlyAttachCaster(bool const bOnlyCaster) { m_bOnlyCaster = bOnlyCaster; }
	

	void SetAttackScaleByAbil(float AttackScaleByAbil) { m_AttackScaleByAbil = AttackScaleByAbil;}
	float GetAttackScaleByAbil() const { return m_AttackScaleByAbil;} 

private:	
	void	Create();
	void	Destroy();

private:

	std::string	m_kType;

	int	m_ValidItems;

	NiColorA	m_kColorAlpha;
	NiColor		m_kSpecular;
	float		m_fAlphaStartTransitionTime;
	float		m_fAlphaEndTransitionTime;
	float		m_fAlphaEnd;

	int			m_iGIFTitleEmoticonID;

	int			m_iActivateEventScript;
	int			m_iDeactivateEventScript;

	float	m_fHeadSize;
	float	m_fBodySize;
	float	m_fBodyChangeTime;

	float	m_fColorTransitSpeed, m_fColorTransitTime;

	ParticleCont	m_kParticleContainer;
	ParticleCont	m_kDetachParticleContainer;
	IntCont			m_kHidePartContainer;
	SkillTextCont	m_kSkillTextContainer;

	std::string	m_kTransformID;
	std::string	m_kTransformIDForMale;
	std::string	m_kTransformIDForFemale;
	std::string m_kTransformNextAction;
	int			m_iTransformNameNo;

	std::string m_kNewDefaultAttack, m_kOriginalDefaultAttack;
	std::string	m_kSoundID;
	int			m_iLoopCount;	

	std::string	m_kDetachActionID;
	std::string	m_kStartActionID;
	EStartActionOptionType		m_eStartActionOption;
	int			m_kParamIndex;
	std::string	m_kParamID;
	int			m_kEffectSave;

	RedirectionMap	m_RedirectionMap;

	VInputSlotInfo m_vInputSlotInfo;

	TEXTURE_PATH_LIST	m_kChangedTextureList;
	TEXTURE_PATH_LIST	m_kOriginTextureList;
	TEXTURE_PATH_LIST	m_kContWhiteOutTexture;

	int m_iHideEnable;
	bool m_bNotDetachOnDie;	//죽을때도 안떨어질것

	SFocusFilter	m_kFocusFilter;	

	EQUIP_ITEM_CONT		m_kEquipItemCont;			// 새롭게 장착되는 아이템 리스트
	int					m_iOtherEquipItemReturnValue; //  장착 리스트에 없는 경우 리턴 값

	std::string	m_kScriptName;	//틱마다 실행될 함수 머리

	CONT_ADD_ABIL		m_kContAddAbil;
	CONT_PARAM			m_kContParam;

	std::string	m_kHideNodeName;
	bool m_bDoDmgActionOnFreezed;
	float m_AttackScaleByAbil;//공격 스킬 크기를 저장하는 변수

	CONT_CHANGE_ACTION	m_kContChangeAction;
	bool m_bOnlyCaster; // 캐스터에게만 붙는 StatusEffect
	
};

class PgStatusEffectInstance	
{
public:

	PgStatusEffectInstance(PgStatusEffect const* pkStatusEffect, bool bMadeByItem = false) 
		: m_bMadeByItem(bMadeByItem),
		m_pkStatusEffect(pkStatusEffect),
		m_kVisualState(EVS_VISIBLE),
		m_ulStartTime(0),
		m_iEffectID(0),
		m_iEffectKey(0),
		m_iEffectValue(0),
		m_ulTickTime(0),
		m_fDetachAlpha(1.f)
	{
		static	int	iInstanceIDGenerator = 0;
		m_iInstanceID = iInstanceIDGenerator++;
		if(pkStatusEffect)
		{
			m_kContParam = pkStatusEffect->GetContParam();
		}
	}

	~PgStatusEffectInstance();

	int		GetInstanceID()	const { return m_iInstanceID; }

	void	SetEffectID(int iEffectID)	{ m_iEffectID = iEffectID; }
	int		GetEffectID() const			{ return m_iEffectID; }

	void	SetEffectKey(int const iEffectKey)	{m_iEffectKey = iEffectKey;}
	int		GetEffectKey()const					{return m_iEffectKey;}

	void	SetEffectValue(int iEffectValue)	{ m_iEffectValue = iEffectValue; }
	int		GetEffectValue() const				{ return m_iEffectValue; }

	void			SetStartTime(unsigned long ulStartTime)	{ m_ulStartTime = ulStartTime; }
	unsigned long	GetStartTime() const { return m_ulStartTime; }

	void			SetTickTime(unsigned long ulTickTime)	{ m_ulTickTime = ulTickTime; }
	unsigned long	GetTickTime() const { return m_ulTickTime; }

	void				SetVisualState(EffectVisualState kVisualState)	{ m_kVisualState = kVisualState; }
	EffectVisualState	GetVisualState() const							{ return m_kVisualState; }

	PgStatusEffect const*	GetStatusEffect() const { return m_pkStatusEffect; }

	bool GetMadeByItem() const { return m_bMadeByItem; }

	//
	void AttachEffect(PgPilot* pkTargetPilot, bool const bNowAdded = false);
	void DetachEffect(PgPilot* pkTargetPilot);

	void AttachEffectAbil(PgPilot *pkTargetPilot);
	void DetachEffectAbil(PgPilot *pkTargetPilot);

	void StartEffect(PgPilot* pkTargetPilot);

	void ShowSkillTexts(PgPilot* pkTargetPilot);

	void PlaySound(PgActor* pkTarget);
	void StopSound();

	void SetTransformation(PgActor* pkTarget);
	void RestoreTransformation(PgActor* pkTarget);
	void AttachAdjustedItem(PgActor* pkTarget);
	void DetachAdjustedItem(PgActor* pkTarget);

	std::string GetParam(std::string const& kKey)const;
	void SetParam(std::string const& kKey, std::string const& kValue);
private :
	//
	void AttachParticles(PgPilot* pkTargetPilot, bool const bNowAdded = false);
	void AttachParticles(PgPilot* pkTargetPilot, PgStatusEffect::ParticleCont const& rkParticleContainer, bool const bNowAdded = false);
	void DetachParticles(PgPilot* pkTargetPilot);

	void HideParts(PgPilot* pkTargetPilot);
	void ShowParts(PgPilot* pkTargetPilot);
	void ShowPartsIfOnlyHided(PgPilot* pkTargetPilot);
	void HidePartsIfOnlyShowed(PgPilot* pkTargetPilot);

	void ChangeDefaultAttack(PgPilot* pkTargetPilot);
	void RestoreDefaultAttack(PgPilot* pkTargetPilot);

	void DoInputSlotIDEnable();
	void UndoInputSlotIDEnable();	

	void ChangeTexture(PgPilot* pkTargetPilot, bool const bRecover = false);
	void ChangeTextureRecursiveProcess(NiAVObject* pkObject, bool const bRestore = false);

	void AttachWhiteOut(PgPilot* pkTargetPilot);
	void DetachWhiteOut(PgPilot* pkTargetPilot);
	
	float StatusEffectRangeScaleByAbil(float fSize)const;

private :
	int	m_iInstanceID;

	PgStatusEffect const*	m_pkStatusEffect;

	EffectVisualState	m_kVisualState;
	unsigned long		m_ulStartTime;
	unsigned long		m_ulTickTime;

	float				m_fDetachAlpha;

	int	m_iEffectID, m_iEffectValue;
	int m_iEffectKey;

	bool m_bMadeByItem;	//	아이템 장착에 의해서 생겨난 것인가?

	typedef std::map<int, int> ContIDMaped;

	ContIDMaped m_kAttachedStatusEffectParticleContainer;

	CONT_PARAM			m_kContParam;
};

class PgStatusEffectMan;
namespace PgStatusEffectManUtil
{
	typedef struct tagReservedEffectKey
	{
		tagReservedEffectKey()
			: kPilotGuid(), iEffectID(0)
		{
		}

		tagReservedEffectKey(tagReservedEffectKey const& rhs)
			: kPilotGuid(rhs.kPilotGuid), iEffectID(rhs.iEffectID)
		{
		}

		tagReservedEffectKey(BM::GUID const& rkPilotGuid, int const iEffectID = 0)
			: kPilotGuid(rkPilotGuid), iEffectID(iEffectID)
		{
		}

		bool operator == (tagReservedEffectKey const& rhs) const
		{
			return (kPilotGuid == rhs.kPilotGuid) && (iEffectID == rhs.iEffectID);
		}

		bool operator == (BM::GUID const& rhs) const
		{
			return kPilotGuid == rhs;
		}

		BM::GUID kPilotGuid; // Character GUID
		int iEffectID;
	} SReservedEffectKey;

	typedef struct tagReservedEffect : public SReservedEffectKey
	{
		tagReservedEffect()
			: SReservedEffectKey(), kCasterGuid(), iEffectKey(0), iActionInstanceID(0), iValue(0), ulStartTime(0), ulEndTime(0)
		{
		}

		tagReservedEffect(tagReservedEffect const& rhs)
			: SReservedEffectKey(rhs), kCasterGuid(rhs.kCasterGuid), iEffectKey(rhs.iEffectKey),
			iActionInstanceID(rhs.iActionInstanceID), iValue(rhs.iValue), ulStartTime(rhs.ulStartTime), ulEndTime(rhs.ulEndTime)
			,	kExpireTime(rhs.kExpireTime)
		{
		}

		void ReadFromPacket(BM::Stream& rkPacket)
		{
			rkPacket.Pop( kPilotGuid );
			rkPacket.Pop( iEffectKey );
			rkPacket.Pop( iEffectID );
			rkPacket.Pop( kCasterGuid );
			rkPacket.Pop( iActionInstanceID );
			rkPacket.Pop( iValue );
			rkPacket.Pop( ulStartTime );
			rkPacket.Pop( ulEndTime );
			rkPacket.Pop( kExpireTime );
		}

		bool operator == (tagReservedEffect const& rhs) const
		{
			return SReservedEffectKey::operator ==(rhs) && (kCasterGuid == rhs.kCasterGuid) 
				&& (iEffectKey == rhs.iEffectKey) && (iActionInstanceID == rhs.iActionInstanceID) && (iValue == rhs.iValue);
		}

		bool operator == (SReservedEffectKey const& rhs) const
		{
			return SReservedEffectKey::operator ==(rhs);
		}

		bool operator == (BM::GUID const& rhs) const
		{
			return SReservedEffectKey::operator ==(rhs);
		}

		BM::GUID kCasterGuid; // 시전자
		int iEffectKey; // EffectKey (EffectID or ItemID)
		int iActionInstanceID;
		int iValue;
		unsigned long ulStartTime;
		unsigned long ulEndTime;
		BM::DBTIMESTAMP_EX kExpireTime;
	} SReservedEffect;

	typedef struct tagEffectUpdateInfo
	{
		tagEffectUpdateInfo(PgStatusEffectInstance* pkInstance = NULL, float fTick = 0.0f) : m_pkInstance(pkInstance), m_kTickTime(fTick), m_kLastUpdateTime(0.0f)
		{
		}

		bool Update(float const fAccumTime);

		PgStatusEffectInstance *m_pkInstance;
		float	m_kTickTime;
		float	m_kLastUpdateTime;
	}SEffectUpdateInfo;

	typedef std::list< SReservedEffect > ContReserveEffect;
	typedef std::list< SReservedEffectKey > ContReserveEffectKey;
	typedef std::map< BM::GUID, ContReserveEffect > ContAddReserveEffect;
	typedef std::map< BM::GUID, ContReserveEffectKey > ContDelReserveEffect;

	void AddEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, BM::GUID const& rkCasterGuid, int const iEffectID, 
		int const iActionInstanceID, int const iValue, unsigned long const ulStartTime = 0, unsigned long const ulEndTime = 0, int const iEffectKey=0, BM::DBTIMESTAMP_EX const * const pkExpireTime = NULL );
//	void AddEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, SReservedEffect const& rkEffect);	
	void DelEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, int const iEffectID);
	bool AddReservedEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, SReservedEffect const& rkEffect);
	bool DelReservedEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, int const iEffectID);
	bool AddReservedEffect(PgStatusEffectMan& rkEffectMan, SReservedEffect const& rkEffect);
	bool DelReservedEffect(PgStatusEffectMan& rkEffectMan, BM::GUID const& rkPilotGuid, int const iEffectID);
}

typedef	std::list<PgStatusEffectInstance*> StatusEffectInstanceList;
typedef	std::list<PgStatusEffectManUtil::SEffectUpdateInfo> StatusEffectUpdateList;

class PgMobileSuit;

class PgStatusEffectMan
{
public:
	friend class PgMobileSuit;

	struct	stCasterInfo
	{
		PgPilot* m_pkTargetPilot;
		BM::GUID m_kCasterPilotGUID;
		int	m_iEffectID;
		stCasterInfo(BM::GUID const& kCasterPilotGUID, PgPilot* pkTargetPilot, int iEffectID)
		{
			m_kCasterPilotGUID = kCasterPilotGUID;
			m_pkTargetPilot = pkTargetPilot;
			m_iEffectID = iEffectID;
		}
	};

	typedef	std::map<std::string, PgStatusEffect*> ContStatusEffect;

	typedef	std::list<stCasterInfo*> CasterInfoList;

public:

	PgStatusEffectMan()	{ Create(); }
	~PgStatusEffectMan(){ Destroy(); }

	void	RemoveStatusEffectAfterAction(PgPilot* pkTargetPilot, int const iEffectID);

	void	AddStatusEffect(PgPilot* pkTargetPilot, PgPilot* pkCasterPilot, int const iActionInstanceID, int const iEffectID, int const iValue, bool const bAddEvenExist = false,
		bool const bNowAdded = false, DWORD const dwElpasedTime = 0, unsigned long const ulStartTime = 0, unsigned long const ulEndTime = 0, int iEffectKey = 0, BM::DBTIMESTAMP_EX const * const pkExpireTime = NULL );
	
	//void	AddStatusEffect(PgPilot* pkTargetPilot, PgPilot* pkCasterPilot, PgStatusEffectManUtil::SReservedEffect const& rkEffect, bool const bAddEvenExist = false,
	//	bool const bNowAdded = false, DWORD const dwElpasedTime = 0);

	void	RemoveStatusEffect(PgPilot* pkTargetPilot, int const iEffectID);

	void	ReAddEveryEffect(PgPilot* pkTargetPilot);

	int		AddStatusEffectToActor(PgPilot* pkTargetPilot, PgPilot* pkCaster, std::string kEffectXMLID, int const iEffectID, int const iEffectKey, int const iEffectValue, bool const bNowAdded, bool const bMadeByItem);
	void	RemoveStatusEffectFromActor(PgPilot* pkTargetPilot, int const iEffectID);	//	this uses  iEffectID to search the instance.
	void	RemoveStatusEffectFromActor2(PgPilot* pkTargetPilot, int const iInstanceID);	//	this uses iInstanceID to search the instance.

	void	RemoveAllStatusEffect(PgPilot* pkTargetPilot, bool const bIncludeItemStatusEffect = false);

	void AddReserveEffect(PgStatusEffectManUtil::SReservedEffect const& rkReserveEffect);
	void DelReserveEffect(BM::GUID const& rkReservePilotGuid, int const iEffectKey);
	void ClearReserveEffect(BM::GUID const& rkReservePilotGuid);
	void ClearReserveEffectAll();
	void DoReserved(PgPilot* pkPilot);	
	size_t DisplayHeadTransformEffect(PgActor* const pkActor, bool const bShow=true);
	bool NoticeMsgWhenEffectApplyToPlayer(int iEffectID, PgPilot* pkTargetPilot);	// 적용된 버프, 디버프를 공지 메세지로 알림
protected:
	void	Destroy();
	void TryReservedDoEffect(BM::GUID const& rkReservePilotGuid); // 추가/삭제 예약이후 Pilot이 있나 제 확인
	template< typename _T_KEY, typename _T_LIST_VALUE >
	void AddReserve(std::map< _T_KEY, std::list< _T_LIST_VALUE > >& rkCont, _T_KEY const& rkKey, _T_LIST_VALUE const& rkValue);

private:
	void	Create();
	void	UpdateEffect(PgPilot* pkTargetPilot);
	PgStatusEffect* GetStatusEffect(std::string const& kID, int const iEffectValue);
	PgStatusEffect* GetStatusEffectEx(std::string const& kID);
	bool	TemporaryHideSameTypeEffect(PgPilot* pkTargetPilot, PgStatusEffectInstance const & kSrcStatusEffectInstance);
	void	ShowTemporaryHidedEffect(PgPilot* pkTargetPilot, std::string kEffectType);
	
	bool ToggleStateChange(int const iEffectID, PgPilot* pkTargetPilot, bool const bOn);

private:
	Loki::Mutex m_kMutex;
	ContStatusEffect	m_ContStatusEffect;

	PgStatusEffectManUtil::ContAddReserveEffect m_kAddReserve;
	PgStatusEffectManUtil::ContDelReserveEffect m_kDelReserve;	
};

extern	PgStatusEffectMan	g_kStatusEffectMan;

#endif // FREEDOM_DRAGONICA_CONTENTS_STATUSEFFECT_PGSTATUSEFFECT_H