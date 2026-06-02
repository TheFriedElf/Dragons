#ifndef FREEDOM_DRAGONICA_RENDER_TRIGGER_PGTRIGGER_H
#define FREEDOM_DRAGONICA_RENDER_TRIGGER_PGTRIGGER_H

#include "Variant/PgPortalAccess.h"
#include "Variant/PgSimpleTime.h"
#include "PgIWorldObject.h"


class PgActor;
class SPortalAccessInfo;

class PgTrigger : public PgIWorldObject
{
public:
	typedef enum
	{
		TRIGGER_TYPE_NONE = -1,
		TRIGGER_TYPE_PORTAL,
		TRIGGER_TYPE_MISSION,
		TRIGGER_TYPE_JUMP,
		TRIGGER_TYPE_TELEJUMP,
		TRIGGER_TYPE_ROPE_RIDING,
		TRIGGER_TYPE_LOCATION,
		TRIGGER_TYPE_EVENT,
		TRIGGER_TYPE_KEYEVENT,
		TRIGGER_TYPE_CAMERA,
		TRIGGER_TYPE_BATTLEAREA,
		TRIGGER_TYPE_PORTAL_EMPORIA,
		TRIGGER_TYPE_MISSION_NPC,
		TRIGGER_TYPE_ENTER_HOME,
		TRIGGER_TYPE_MISSION_EVENT_NPC,
		TRIGGER_TYPE_TRANSTOWER,
		TRIGGER_TYPE_MISSION_EASY,
		TRIGGER_TYPE_HIDDEN_PORTAL,
		TRIGGER_TYPE_TELEPORT,
		TRIGGER_TYPE_SUPERGROUND,
		TRIGGER_TYPE_PARTYPORTAL,
		TRIGGER_TYPE_PARTY_MEMBER_PORTAL,
		TRIGGER_TYPE_JOB_SKILL,
		TRIGGER_TYPE_DOUBLE_UP,
		TRIGGER_TYPE_ROADSIGN,
		TRIGGER_TYPE_CHANNELPORTAL,
		TRIGGER_TYPE_KING_OF_HILL,
		TRIGGER_TYPE_LOVE_FENCE,
		TRIGGER_TYPE_SCORE,
		TRIGGER_TYPE_RADAR,
		TRIGGER_TYPE_MISSION_EVENT_NOT_HAVE_ARCADE,
		TRIGGER_TYPE_PROGRESS_SENSOR,
		TRIGGER_TYPE_RAG_PARTY_MEMBER_PORTAL,
		TRIGGER_TYPE_RACE_CHECK_POINT,
		TRIGGER_TYPE_MISSION_UNUSED_GADACOIN,
		MAX_NUM_TRIGGER_TYPE,
	} TriggerType;

	typedef enum
	{
		CT_TOUCH,
		CT_ACTION,
		CT_SCRIPT,
	} ConditionType;

	typedef enum
	{
		ADD_NONE,
		ADD_RADAR,
	} AddedType;

	typedef std::map<std::string,std::string> ParamMap;
	typedef std::map< float, float > CONT_TIME;
	typedef std::map<std::string, float> CONT_CAMTRG_INFO;
	typedef std::set< BM::GUID > CONT_DISABLE_IN_ACTOR;
public:
	PgTrigger();
	~PgTrigger();

	bool Initialize();

	//! 스크립트를 호출한다.
	bool OnEnter(PgActor *pkActor);
	bool OnUpdate(PgActor *pkActor);
	bool OnLeave(PgActor *pkActor, PgTrigger *pNextTrigger);
	bool OnAction(PgActor *pkActor);

	//! PgIWorldObject 재정의
	virtual bool Update(float fAccumTime, float fFrameTime);
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false){	return true; }

	//! XML을 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	std::string &GetConditionAction();
	ConditionType	GetConditionType()	{	return	m_eConditionType;	}
	NiAVObject* GetTriggerObject() { return m_spTriggerObject; }
	void SetTriggerObject(NiAVObject* pObj) { m_spTriggerObject = pObj; }
	NiPoint3 GetTriggerObjectPos() { return (m_spTriggerObject ? m_spTriggerObject->GetWorldTranslate() : NiPoint3::ZERO); }

	TriggerType GetTriggerType() { return m_eTriggerType; }
	void SetTriggerType(TriggerType eType) { m_eTriggerType = eType; }

	CLASS_DECLARATION_S_NO_SET(bool, Enable);
	void Enable(bool const bNewValue);
	char const*	GetParamFromParamMap(char const *strKeyString);
	bool	SetParamFromParamMap(char const* szKeyString, char const* szValueString);

	int	GetTriggerTitleTextID()	{	return	m_iTriggerTitleTextID;	}
	const	NiPoint3&	GetTitleTextAdjust()	{	return	m_ptTitleTextAdjust;	}
	float	GetTriggerTitleTextScale()	{	return	m_kTriggerTitleTextScale;	}
	const	NiColorA	&GetTriggerTitleTextColor()	{	return	m_kTriggerTitleTextColor;	}
	const	std::wstring& GetTriggerTitleTextFontName()	{	return	m_kTitleTextFontName;	}
	int		GetIndex();
	void	SetIndex(int iIndex);
	std::wstring GetTriggerReactionSkillName(CUnit* pkTargetUnit);
	int GetTriggerReactionSkillNo(CUnit* pkTargetUnit);

	NiPhysXRigidBodySrc* GetPhysXRigidBodySrc() { return m_spPhysXRigidBodySrc; }
	NiAVObject* GetSrcObject() { return m_spSrcObject; }

	void	SetRotation(double	fRotation)	{	m_fRotation	= fRotation;	}
	double	GetRotation()	const	{	return	m_fRotation;	}

	// Portal Access
	bool IsHavePortalAccess( size_t const iIndex )const{return m_kContPortalAccess.size()>iIndex;}
	int GetPortalAccessName( size_t const iIndex )const;
	bool IsPortalAccess( size_t const iIndex )const;
	void IsProtalAccess();
	void IsAccessInfo(size_t const iIndex, SPortalAccessInfo & rkAccessInfo)const;
	
	// 카메라 트리거 정보 얻기
	float const GetCamTrgInfoHeight()		const;
	float const GetCamTrgInfoWidth()		const;
	float const GetCamTrgInfoZoom()			const;
	float const GetCamTrgInfoTargetHeight() const;
	float const GetCamTrgInfoTargetDepth()	const;
	float const GetCamTrgInfoTargetWidth()	const;
	int   const GetCamTrgInfoGroup()		const;

	std::string const& GetScript() const { return m_kScript; }

	bool IsActiveTime(void) const;

protected:
	typedef std::vector<SSIMPLETIMELIMIT> CONT_TIME_EVENT;
	CONT_TIME_EVENT m_kContTimeEvent;

	//! 애니메이션이 있는 물체라면 m_kPhysXActiveTimeCont 를 체크하여 활성화 되어있는지 리턴.
	bool IsPhysXActiveTime();

	CLASS_DECLARATION(int, m_iParam, Param);
	CLASS_DECLARATION(int, m_iParam2, Param2);
	CLASS_DECLARATION(std::wstring, m_wstrParam, ParamString);
	CLASS_DECLARATION(int, m_iTriggerSkillID, TriggerSkillID);

	// 트리거의 부가 속성( 예 : 레이더 )
	CLASS_DECLARATION(AddedType, m_eAddedType, EAddedType);

	// 트리거에서 트리거로 이동할 때 이전 트리거의 OnLeave의 스크립트를 사용할 것인가 체크하기 위한 bool 변수
	CLASS_DECLARATION(bool, m_bDoOnLeaveScript, DoOnLeaveScript);

	CLASS_DECLARATION(int, m_iEntityTeam, EntityTeam);
	CLASS_DECLARATION(int, m_iObjectNo, ObjectNo);
	CLASS_DECLARATION(int, m_iObjectTeam, ObjectTeam);
	// 트리거 위치에 있는 파괴오브젝트의 상태를 저장. true : 살아있음, flase : 파괴됨
	CLASS_DECLARATION(bool, m_bLoveFenceLife, LoveFenceLife); // 러브러브 모드의 울타리 오브젝트 상태

	std::vector<std::string> m_ContEntityLinkName;

	CLASS_DECLARATION_NO_SET(bool, m_bMinimapHide, MinimapHide);
	CLASS_DECLARATION_NO_SET(bool, m_bUseWorldEventScript, UseWorldEventScript);

	CLASS_DECLARATION(int, m_LoadingImageID, LoadingImageID);
	CLASS_DECLARATION(int, m_RagUI, RagUI);

	// Camera Trigger Info 최소한의 필요한 값이 왔는지 체크하고, 없다면 디폴트 값을 세팅
	bool CheckCamTrgInfoAndFix();

private:

	void	SetSourceObjects(NiPhysXRigidBodySrc *pkPhysXRigidBodySrc,NiAVObject *pkSourceObj)
	{
		m_spPhysXRigidBodySrc = pkPhysXRigidBodySrc;
		m_spSrcObject = pkSourceObj;
	}


protected:
	int	m_iTriggerTitleTextID;
	NiColorA	m_kTriggerTitleTextColor;
	float	m_kTriggerTitleTextScale;
	NiPoint3	m_ptTitleTextAdjust;
	std::wstring	m_kTitleTextFontName;
	NiAVObjectPtr m_spTriggerObject;

	ConditionType m_eConditionType;
	std::string m_kConditionAction;
	std::string m_kScript;
	int m_iIndex;

	double	m_fRotation;

	ParamMap	m_ParamMap;
	TriggerType m_eTriggerType;

	// PhysX Regid body Src
	NiPhysXRigidBodySrcPtr m_spPhysXRigidBodySrc;
	NiAVObjectPtr m_spSrcObject;

	// PhysX Active
	float m_fPhysXActiveTotalTime;
	CONT_TIME m_kPhysXActiveTimeCont;

	// Particle Emit 관련
	bool m_bIsEmitting;
	CONT_TIME m_kParticleEmitTimeCont;
	float m_fParticleEmitTotalTime;

	//	Enter 시에 플레이 할 SoundID
	std::string	m_kSoundIDOnEnter;

	// Portal Access
	CONT_PORTAL_ACCESS	m_kContPortalAccess;
	
	// 카메라 트리거 정보
	CONT_CAMTRG_INFO	m_kContCamTrgInfo;

	CONT_DISABLE_IN_ACTOR m_kContDisableInActor;
public:
	// 퀘스트 존재할 때 이동 할 위치 ID
	typedef struct tagQuestTeleport
	{
		tagQuestTeleport()
		{
			iQuestID = 0;
		}

		int iQuestID;
		std::wstring kTeleportID;
	}SQuestTeleport;
	std::wstring FindQuestTeleport();
private:
	std::list<SQuestTeleport> m_kContQuestTeleport;
	CLASS_DECLARATION(int, m_iErrorMsgID, ErrorMsgID );
//////////////////////접촉 시 애니 연출 기능 - 시작 - //////////////////////
public:
	void ParseXML_RemoteObject(const TiXmlElement *pkElement);
	void SetRemoteObject();
	bool GetRemoteObject( std::string const &rkObjectName, CONT_MAP_REMOTE_OBJECT::mapped_type &rkObject );
private:
	CONT_MAP_REMOTE_OBJECT m_kContRemoteObject;				//xml으로 부터 파싱한 오브젝트 목록
//////////////////////접촉 시 애니 연출 기능 - 끝 - //////////////////////

protected:
	int m_iProgressID;
	void SetProgressID(int iProgressID) { m_iProgressID = iProgressID; }
public:
	int GetProgressID(void) const { return m_iProgressID; }
};
#endif // FREEDOM_DRAGONICA_RENDER_TRIGGER_PGTRIGGER_H