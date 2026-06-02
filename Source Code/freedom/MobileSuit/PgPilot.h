#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOT_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOT_H

#include "PgIXmlObject.h"
#include "Variant/Unit.h"
#include "Variant/Inventory.H"
#include "PgActionTargetList.h"

class PgIWorldObject;
class PgInput;
class PgDropBox;
class PgAction;

class PgRemoteInput
{
friend class PgPilot;

public:
	PgRemoteInput(SActionInfo& rkActionInfo, PgActionTargetList& rkTargetList, DWORD dwSyncTime);
	
private:
	SActionInfo	m_kActionInfo;
	PgActionTargetList m_kTargetList;

#ifdef PG_SYNC_ENTIRE_TIME
	DWORD m_dwSyncTime;
#endif
};

class PgInputSlotInfo
{
private:
	std::string		m_kActionID;
	unsigned int	m_uiKey;
	void*	m_pkUserData;
	bool	m_bRecord;
	bool	m_bEnable;
	int		m_iTargetType;	// 명령을 적용시킬 대상
	int		m_iWeaponType;	// 사용 가능한 무기 타입
public:
	enum eTargetType
	{// 명령을 적용시킬 대상
		ETT_SELF=0,	// 자신
		ETT_SUB_PLAYER=1, // 보조캐릭터(쌍둥이와 같은)
	};
public:
	PgInputSlotInfo(char const* pcActionID, unsigned int uiKey, void* pkUserData = 0, bool bRecord = false,bool bEnable = true, int const iTargetType = 0, int const iWeaponType = 0);

	// getter
	std::string const& GetActionID()const;
	unsigned int	GetUKey() const;
	void*			GetUserData() const;
	bool			IsRecord() const;
	bool			GetEnable()	const { return m_bEnable; }
	int				GetTargetType() const { return m_iTargetType; }
	int				GetWeaponType() const { return m_iWeaponType; }
	// setter
	void SetActionID(std::string const& rkActionID);

	// operator
	bool operator == (PgInputSlotInfo const& rhs)const;
};

//----------연계콤보--------------
enum eUKeyState
{
	EUKS_NONE		= 0,	//초기화
	EUKS_PRESS		= 1,	//눌렀을 때
	EUKS_RELEASE	= 2,	//떼었을 때
};
struct tagKeyInfo
{
	int iUKeyNo;			//키입력 정보
	eUKeyState eKeyState;	//키 상태 체크 - 기본 EUKS_NONE(초기화)
	float fKeyTime;			//키 눌렸던 시간 - 기본 0.0f
};
typedef std::vector<tagKeyInfo> CONT_VEC_INPUT_KEY;

struct tagComboKeyData
{
	int iNextSkillNo;		//전이될 스킬번호
	int iNextSkillNo_SC;	//보조캐릭터가 사용할 스킬번호
	int iNextSkillNo_Fusion;//격투가 퓨전 시, 캐릭터가 사용할 스킬번호
	bool bComboConnect;	//콤보 연결 스킬이냐
	int	iWeaponType;	//무기 제한
	CONT_VEC_INPUT_KEY kContKeyInfo;
};
typedef std::vector<tagComboKeyData> CONT_VEC_COMBO_KEY_DATA;
typedef std::map<int, CONT_VEC_COMBO_KEY_DATA> CONT_MAP_COMBO_ACTION_DATA;

//----------연계콤보 끝--------------
//맵<현재스킬, 키입력정보>
//키입력정보 : 

class PgPilot 
	:	public PgIXmlObject
	,	public BM::CObserver< BM::Stream* >
	
{
public:
	PgPilot();
	virtual ~PgPilot();

public:
	typedef std::list<PgRemoteInput*>	RemoteActionContainer;
	typedef std::list<PgInputSlotInfo>	InputSlotContainer;
	typedef std::vector<std::string>	DamageBlinkContainer;

	//! Create PgPilot
	static PgPilot* Create(BM::GUID& rkPilotGuid);

	PgPilot* CreateCopy();

	//! Process Action
	bool ProcessAction(PgAction* pkAction, bool bFromServer = false);

	//! Parse XML
	virtual bool ParseXml(TiXmlNode const* pkNode, void* pArg = 0, bool bUTF8 = false);

	void	ChangeXml(int iNewClassNo);

	//! Set Direciton
	void SetDirection(BYTE byDirection, DWORD dwDirectionTerm, NiPoint3& rkCurPos);
	
	//! Create Action
	PgAction *CreateAction(std::string const &kActionName);
	PgAction *CreateAction(PgInput *pkInput);
	PgAction *CreateAction(PgRemoteInput *pkRemoteInput);
	bool	CanExcuteByCommandKey(PgAction *pkAction);

	//! U-Key에 해당하는 SlotInfo를 되돌려 준다.
	PgInputSlotInfo* FindAction(unsigned int uiUKey);

	//! U-Key에 해당하는 Action-ID를 찾아준다.
	char const* FindActionID(unsigned int uiUKey)const;
	
	//! U-Key에 해당하는 Action-ID를 설정한다(U-Key 가 없다면 추가한다.)
	void SetKeyActionID(unsigned int uiUKey, std::string const& ActionID, int const iWeaponType);
//	void SetKeyActionID_By_QuickSlotDB();

	//! Abil을 셋팅한다.
	void SetAbil(int AbilType, int Value);
	void SetAbil64(int AbilType, __int64 Value);

	//! Abil을 가져온다.
	int	GetAbil(int iAbilType) const;
	__int64	GetAbil64(int iAbilType) const;
	
	//! U-Key를 컨테이너에서 제거한다.
	void RemoveActionKey(unsigned int uiUKey);

	//! 키보드로 조종이 가능한가?
	bool IsControllable();

	//! Set Object
	void SetWorldObject(PgIWorldObject* pkObject);

	//! Set Object
	PgIWorldObject* GetWorldObject();

	//! Get Pilot Guid
	BM::GUID const& GetGuid() const;
	void SetGuid(BM::GUID const& rkGuid);

	//!	Base Class (전사,법사,궁수,도적) 을 얻어온다.
	int	GetBaseClassID(int iReqClassID=-1);
	bool IsCorrectClass(int const iReqClassID, bool bNotIncludeSelf=false);	//	이 캐릭터가 iReqClassID 에 해당하는 캐릭터가 맞는지 체크한다.
	bool IsOverClass(int const iReqClassID);	//	이 캐릭터가 iReqClassID 의 하위 클래스 캐릭터인가?

	void SetName(std::wstring const& wName);
	std::wstring const GetName() const;

	bool IsShowWarning()const;
	bool IsHide()const;

	CUnit* GetUnit() const { return m_pkUnit; }
	void SetUnit(CUnit* pkUnit);
	void SetUnit(BM::GUID const& kGUID,int iUnitType,int iClassNo,int iLevel,int iGender);

	//! Pilot을 Frozen상태를 설정한다.
	void SetFreeze(bool bFreeze);

	//! Pilot이 Frozen상태인지 반환한다.
	bool IsFrozen();

	//! 들어오는 패킷을 처리하는 Method들
	void RecvNfyAction(SActionInfo& rkActionInfo, BM::Stream* pkPacket);

	void SetLastHPSetTimeStamp(DWORD dwTimeStamp)	{	m_dwLastHPSetTimeStamp = dwTimeStamp;	}
	DWORD GetLastHPSetTimeStamp()	{	return	m_dwLastHPSetTimeStamp;	}

	InputSlotContainer const& GetInputSlotContainer()	const	{	return	m_kInputSlotContainer;	}

	virtual void VUpdate(BM::CSubject< BM::Stream* > *const pChangedSubject, BM::Stream* pkNfy);

	static BM::GUID const& PlayerPilotGuid() {return ms_kPlayerPilotGuid;}
	static void PlayerPilotGuid(BM::GUID const& rkInGuid) {ms_kPlayerPilotGuid = rkInGuid;}

	void	ActivateExtendedSlot();
	
	void SetRidingPet(bool bRideOn); //펫에 탑승/하차 설정(이 값에 따라 액션이 달라져야 한다)
	bool IsRidingPet(void);
//----연계콤보   ----
	void ParseComboData(TiXmlNode const *pkNode);
	bool InsertComboData(TiXmlNode const *pkNode, int const iCurrentSkillNo, CONT_MAP_COMBO_ACTION_DATA &rkDestContComboActionData, CONT_VEC_COMBO_KEY_DATA &rkComboData);
	CONT_MAP_COMBO_ACTION_DATA const& GetComboActionData()	const	{	return	m_kContComboActionData;	}
	CONT_MAP_COMBO_ACTION_DATA const& GetConnectComboActionData()	const	{	return	m_kContConnectComboActionData;	}
	bool IsHaveComboAction( int const iCurrentAction );
	int GetNextComboAction( int const iCurrentAction, bool const bSearchChargeAction, int &iNextAction_SC );
	bool IsHaveConnectComboAction(int const iWeaponType, int const iCurrentAction);
	int GetNextConnectComboAction(int const iWeaponType);
	bool GetNextComboList( int const iCurrentActionNO, std::list<int>& rkContNextCombo );
	bool IsUseComboAction( int const iActionNo );
	bool IsChargeCombo(int const iActionNo);
	bool GetChargeResultAction(int const iActionNo, std::list<int> &rkContAction);
	//키 입력
	bool IsInputAttackKey();
	int GetInputAttackKey();
	void InsertInputKey(int const iUKey, float const fStartTime, bool const bPush);
	void ClearInputKey();
	void RefreshInputKey();
	void SetInputKeyState_Release();
	bool HaveInputKeyState_Release();
	bool FindInputKey(int const iUiKey);
	//입력한 키 반환 
	CONT_VEC_INPUT_KEY GetInputKey() const { return m_kContInputKey; }
//----연계콤보 끝----
private:
	void	ReloadXml();	
	//size_t DisplayHeadTransformEffect(bool const bShow);

protected:
	static BM::GUID ms_kPlayerPilotGuid;

	unsigned long m_ulLastRemoteActionTime;
	RemoteActionContainer m_kRemoteActionList;

	bool m_bFrozen;
	std::string m_kActorID;
		

	CUnit* m_pkUnit;
	PgIWorldObject* m_pkWorldObject;
	InputSlotContainer m_kInputSlotContainer;
	InputSlotContainer m_kRidingInputSlotCont;
	bool m_bRiding;

	DWORD m_dwLastHPSetTimeStamp;

	CONT_MAP_COMBO_ACTION_DATA m_kContComboActionData;
	CONT_MAP_COMBO_ACTION_DATA m_kContConnectComboActionData;	//콤보 중계 스킬
	CONT_VEC_INPUT_KEY m_kContInputKey;
};
#endif// FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGPILOT_H