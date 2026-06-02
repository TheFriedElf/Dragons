#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOTMAN_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOTMAN_H

#include "PgIInputObserver.h"
#include "PgActionTargetList.h"

typedef enum ePlayerLockType // 숫자 변경하지 마세요(lua 에서 직접 호출 하고 있음)
{
	EPLT_None			= 0,
	EPLT_EventScript	= 1,	// EventScript System lock
	EPLT_QuestDialog	= 2,	// Quest Dialog lock
	EPLT_MapMoving		= 3,	// PT_M_C_NFY_MAP_MOVING 패킷 처리시
								// PT_M_C_NFY_FAILED_ENTER_INDUN
								// PT_M_C_NFY_MISSION_STATE
	EPLT_MissionResult	= 4,	// PT_N_C_NFY_MISSION_RESULT 나 미션 결과 처리시
	EPLT_PvpEnter		= 5,	// PT_PM_C_NFY_GAME_LOADING
	EPLT_BossLavalon	= 6,	// 라발론
	EPLT_BossParel		= 7,	// 파렐
	EPLT_BossSaphiere	= 8,	// 사파이어
	EPLT_WorldMap		= 9,	// 월드맵
	EPLT_MovieMgr		= 10,	// PgMovieMgr.cpp
	EPLT_CashShop		= 11,
	EPLT_PvpResult		= 12,
	EPLT_EventUI		= 13,
	EPLT_GuildEmporia	= 14,
	EPLT_SelectMap		= 15,
	EPLT_NpcPopup		= 16,
	EPLT_NamePopup		= 17,
	EPLT_SelectiveQuest	= 18,
	EPLT_RandomMap		= 19,
	EPLT_Trade			= 20,
	EPLT_ActionPortal	= 21,
	EPLT_OpenMarket		= 22,
	EPLT_Teleport01		= 23,	// 카오스/히든 포털
	EPLT_MacroCheck		= 24,
	EPLT_NextQuestTalk	= 25,	// 다음 퀘스트 대화 이어지는 락
	EPLT_BattleSquare	= 26,	// 배틀 스퀘어

	EPLT_ExpeditionEventScirpt = 27,	// 원정대 등 던전 진입시 나오는 Event Script System 락(서버에서 보내주면 인풋 락).

	EPLT_RaceEvent		= 28,

	EPLT_ManualOpening	= 30,	// 스킬 오프닝

	EPLT_StatusEffect1	= 100,	// 스킬 이펙트(안전거품 +)

	EPLT_ObserverMove	= 1000, // 옵저버 모드, PgObserverMode
} EPlayerLockType;
typedef std::list< int > ContLockType;

typedef struct tagReqPlayerLock
{
	inline tagReqPlayerLock(EPlayerLockType const LockType, bool ActionLock)
		:eLockType(LockType), bActionLock(ActionLock)
	{
	}

	inline tagReqPlayerLock(tagReqPlayerLock const& rhs)
		:eLockType(rhs.eLockType), bActionLock(rhs.bActionLock)
	{
	}

	~tagReqPlayerLock()
	{
	}

	bool operator == (tagReqPlayerLock const& rhs) const
	{
		return (eLockType == rhs.eLockType) && (bActionLock == rhs.bActionLock);
	}

	EPlayerLockType const eLockType;
	bool const bActionLock;
} SReqPlayerLock;

class PgPilot;
class PgInput;
class PgWorld;
class PgMobileSuit;
class PgAction;

class PgPilotMan 
	:	public PgIInputObserver
{
	friend	class	PgMobileSuit;


public:

	struct	stClassInfo
	{
		std::string	m_kActorID;
		std::string	m_kPilotPath;

		stClassInfo()
		{
		};

		stClassInfo(std::string const &kActorID,std::string const &kPilotPath)
			:m_kActorID(kActorID),m_kPilotPath(kPilotPath)
		{
		};
	};

	struct SJumpingCharEvent
	{
		SJumpingCharEvent(): iNo(0), iRemain(0), bUse(false), iClass(0) {}
		int BaseClassNo()const;

		int iNo;
		int iRemain;
		bool bUse;
		int iClass;
	};
private:

	typedef struct tagClassGenderKey
	{
		tagClassGenderKey()
		{
			Set(0,0);
		}

		tagClassGenderKey(int const iInGender,	int const iInClass)
		{
			Set(iInGender,	iInClass);
		}

		void Set(int const iInGender,	int const iInClass)
		{
			iGender = iInGender;
			iClass = iInClass;
		}

		int iGender;
		int iClass;
		bool operator < (const tagClassGenderKey &rhs)const
		{
			if(iGender < rhs.iGender) {return true;}
			if(iGender > rhs.iGender) {return false;}

			if(iClass < rhs.iClass) {return true;}
			if(iClass > rhs.iClass) {return false;}

			return false;
		}
	
		operator size_t const()		const	throw()
		{
			size_t temp = iGender ^ iClass;
			return temp ;
		}
	}SClassGenderKey;



	typedef std::map<SClassGenderKey, stClassInfo> ClassContainer;
	typedef std::map<BM::GUID, PgPilot *> Container;
	typedef std::vector<BM::Stream *> PacketBuffer;

	typedef struct tagReservedPilot
	{
		tagReservedPilot( EUnitType const _kUnitType=UT_NONETYPE )
			:	kUnitType(_kUnitType)
		{}

		void Release()
		{
			PacketBuffer::iterator packet_itr = kPacketBuffer.begin();
			for ( ; packet_itr != kPacketBuffer.end() ; ++ packet_itr )
			{
				SAFE_DELETE(*packet_itr);
			}
			kPacketBuffer.clear();
		}

		EUnitType		kUnitType;
		PacketBuffer	kPacketBuffer;
	}SReservedPilot;

	typedef std::map<BM::GUID, SReservedPilot> ReservedPilot;	
	typedef std::map<BYTE, unsigned int> DirectionSlot;
	typedef std::map<BM::GUID,float> PacketBufferTimeCont;
public:

public:
	PgPilotMan();
	virtual ~PgPilotMan();


	//! 입력 브로드-케스팅
	bool ProcessInput(PgInput *pkInput);

	//! 파일럿 컨테이닝
	PgIXmlObject::XmlObjectID GetObjectIDFromString(char const* pcType);
	PgPilot *NewPilot(BM::GUID const &rkGuid, int iClass, int iGender = 0, char const* pcObjectType = 0);

	bool InsertPilot(BM::GUID const &rkGuid, PgPilot* pkPilot);	
	PgPilot *FindPilot(BM::GUID const &rkGuid)const;	
	PgPilot *Unlocked_FindPilot(BM::GUID const &rkGuid)const;	
	PgPilot *FindPilotInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange);
	PgPilot *Unlocked_FindHouse(BM::GUID const &rkOnwerGuid);
	PgPilot *FindHouse(BM::GUID const &rkOnwerGuid);
	PgPilot *Unlocked_FindPilotInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange);
	PgPilot *FindPilot(std::wstring &kName, bool bFindPassPlayerPilot = false);	// bFindPassPlayerPilot == true 플레이어의 이름을 제외하고 이름을 찾는다.
	PgActor *FindActor(BM::GUID const &rkGuid);
	PgActor *FindActorByClassNo(int const iClassNo); // EventScript에서 쓰인다.
	PgActor *FindNpcActor(std::wstring const& kActorName) const;
	bool FindUnitInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange, UNIT_PTR_ARRAY& rkUnitArray);
	size_t Unlocked_FindUnitInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange, UNIT_PTR_ARRAY& rkUnitArray);
    bool FindUnit(EUnitType const eType, UNIT_PTR_ARRAY& rkUnitArray)const;

	bool RemovePilot(BM::GUID const &rkGuid);

	bool InsertReservedPilot(BM::GUID const &rkGuid,EUnitType const kUnitType);
	bool RemoveReservedPilot(BM::GUID const &rkGuid, bool bRemoveFromWorldAddUnitQueue = true);
	void ClearReservedPilot();
	bool IsReservedPilot( BM::GUID const &rkGuid, EUnitType *pOutUnitType=NULL )const;
	bool BufferReservedPilotPacket(BM::GUID const &rkGuid, BM::Stream* pPacket, unsigned int ukey);
	bool FlushReservedPilotPacket(BM::GUID const &rkGuid);

	float	GetFirstPacketBufferedTime(BM::GUID const &rkGuid);
	void	SetFirstPacketBufferedTime(BM::GUID const &rkGuid);

	void ClearPlayerInfo();
	bool SetPlayerPilot(BM::GUID const &rkGuid);
	PgPilot* GetPlayerPilot()const;
	PgPlayer* GetPlayerUnit()const;
	PgActor* GetPlayerActor()const;
	PgWorld* GetPlayerWorld()const;
	bool GetPlayerPilotGuid(BM::GUID& rkPilotGuid_out)const;
	bool IsMyPlayer(BM::GUID const &rkGuid)const;
	bool IsMySummoned(CUnit * pkUnit)const;
	bool IsMySummoned(BM::GUID const& rkGuid)const;

	bool Broadcast(PgPilot *pkPilot, PgAction *pkAction, bool bForceActionTermToZero);
	bool BroadcastDirection(PgPilot *pkPilot, BYTE byDirection);
	void BroadcastSimulate(PgPilot *pkPilot, bool bSimulate);


	void ReadClassID(char const *pcXmlPath);

	bool	GetClassInfo(int const iClass, int const iGender,stClassInfo &koutClassInfo);

	void ModifyActionResult(PgPilot *pkCaster, int iActionInstanceID, PgActionTargetList &rkTargetList, DWORD dwTimeStamp);

	void LockPlayerInput(SReqPlayerLock const& rkReqLock); // Lock/Unlock 시에 type 번호가 한쌍이어야 한다.
	void UnlockPlayerInput(SReqPlayerLock const& rkReqLock);
	bool IsLockMyInput()	{	return	m_bLockMyInput;	}

	//! 방향 슬롯을 초기화
	void ClearDirectionSlot(bool bReset);

	//! 방향 슬롯에 대한 정보를 반환.
	unsigned int GetDirectionSlot(BYTE byDirection);

	void	UpdateDirectionSlot(unsigned int uiUKey,bool bEnable);

	void SetJumpingEvent(int const iEventNo, int const iRemainRewardCount);
	void UseJumpingEvent(bool const bUse);
	int GetJumpingCreateCharCount()const;
	void SetJumpingSelectIdx(int const iIdx);
	bool IsCreateJumpingCharClass(int const iClassNo)const;
	int GetCreateJumpingCharClass(int const iIdx)const;
	int GetCreateJumpingCharLevel(int const iIdx)const;

	void SetHideBalloon(bool const bHide);
	bool IsHideBalloon()const;

protected:
	void SetLockMyInput(bool bLock);

	//	leesg213 2006-11-23
	//	Container 에 들어있는 파일럿들을 메모리 해제 시킨다.
	void	Destroy();


protected:
	DWORD m_dwLastActionBroadcastTime;
	unsigned	long	m_ulLast_S_KeyPressedTime;

	//! 방향 슬롯
	DirectionSlot m_kDirectionSlot;
	unsigned int m_uiStateCount;
	
	Container m_kContPilot;//실제 컨테이너.

	PacketBufferTimeCont	m_kFirstPacketBufferedTimeCont;
	ReservedPilot m_kReservedPilot;
	ClassContainer m_kClassContainer;	

	bool m_bLockMyInput;	//	내 인풋을 막는다.
	mutable ACE_RW_Thread_Mutex m_kReserveLock;
public:
	mutable ACE_RW_Thread_Mutex m_kRemoveLock;

protected:
	CLASS_DECLARATION_PTR(PgPilot*, m_pkPlayerPilot, PlayerPilot);
	CLASS_DECLARATION_PTR(PgPlayer*, m_pkPlayerUnit, PlayerUnit);
	CLASS_DECLARATION_PTR(PgActor*, m_pkPlayerActor, PlayerActor);
	CLASS_DECLARATION_PTR(PgWorld*, m_pkPlayerWorld, PlayerWorld);
	//CLASS_DECLARATION_PTR(PgPilot*, m_pkFindPilotCache, FindPilotCache);

	ContLockType m_kLockedType; // Lock (Can't move, Can skill/Z action, can't space key)
	ContLockType m_kActionLockedType; // Action Lock (Can't move, Can't skill/Z action, Can space key)

	int m_iCacheHitCount;
	bool m_bSet;
	bool m_bHideBalloon;

	CLASS_DECLARATION_S(BM::GUID, PlayerPilotGuid);
	CLASS_DECLARATION_S(SMemberPremiumData, MemberPremium);

	CLASS_DECLARATION_S(SJumpingCharEvent, JumpingCharEvent);
};

#define g_kPilotMan SINGLETON_STATIC(PgPilotMan)


//
namespace PgPilotManUtil
{
	void RemoveReservePilotUnit(BM::GUID const& rkCharacterGuid, char const* szFunc, size_t const iLine);
	void BufferReservedPilotPacket_NotUnitTypeRemove( BM::GUID const& rkCharacterGuid, EUnitType const kUnitType, BM::Stream* pkPacket, char const* szFunc, size_t const iLine );
	bool LockPlayerInput(ContLockType &rkLockCont, EPlayerLockType const eLockType);
	bool UnlockPlayerInput(ContLockType &rkLockCont, EPlayerLockType const eLockType);
	bool RemoveConnectionWithSubPlayer(BM::GUID const& rkPlayerGuid, BM::GUID& rkOutSubPlayer );
}
#endif// FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOTMAN_H