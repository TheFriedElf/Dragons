#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILE_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILE_H

#include "PgActionTargetList.h"

class	PgActor;
class	PgTrailNode;
class	lwProjectile;

class	PgProjectile	:	public	PgIXmlObject, public NiRefObject
{
	friend class	lwProjectile;
public:
	enum	MovingType
	{
		MT_STRAIGHTLINE		= 0,	// 직선 이동(디폴트)
		MT_SIN_CURVELINE	= 1,	// 사인곡선 이동
		MT_BEZIER4_SPLINE	= 2,	// 베지어 곡선 이동
		MT_HOMMING			= 3,
		MT_STAY				= 4,	// 가만히 있는다. 
	};


	struct	stCollisionSphere
	{
		float	m_fRange;	//	반지름
		std::string	m_kTargetNode;	//	충돌구가 붙을 타겟 노드
		NiPoint3	m_kBeforePos;	//	충돌구의 이전 위치

		stCollisionSphere():
			m_fRange(0),
			m_kTargetNode(""),
			m_kBeforePos(0,0,0)
		{
		};
	};
	
	typedef struct SOptionInfo
	{
		bool bRemoveNoneTargetProjectile;	//타겟이 없는 발사체를 지울것인가
		SOptionInfo()
			:bRemoveNoneTargetProjectile(true)
		{}

		SOptionInfo(bool bRemoveNoneTargetProjectile_in)
			:bRemoveNoneTargetProjectile(bRemoveNoneTargetProjectile_in)
		{}
	}SOptionInfo;

	typedef	std::vector<stCollisionSphere> CollisionSphereCont;

protected:

	float	m_fOriginalSpeed,m_fOriginalAccelation;	//	XML 정의되어있는 속도와 가속도

	float	m_fSpeed;	//	이동 속력
	float	m_fAccelation;	//	이동 가속력
	NiPoint3	m_vVelocity;	//	이전 업데이트 시에 이동한 속도 혹은 초기이동속도(이 벡터가 다음 업데이트시에 관성의 방향 및 크기로 적용된다)
	NiPoint3	m_vTarget;	//	목표지점
	float	m_fMass;	//	질량(관성 계산에 영향을 준다)

	float	m_fAccumTime;

	NiPoint3	m_vFireStartPos;
	float	m_fFireStartTime;

	MovingType	m_MovingType;	//	발사체가 이동하는 타입 

	bool	m_bAutoDirection;	//	자동으로 타겟방향을 보도록 할 것인가?

	//	For MT_SIN_CURVELINE
	float	m_fSinPower,m_fSinRotateAngle;

	//	For MT_BEZIER4_SPLINE
	NiPoint3	m_kBezierControlPoint[4];

	//	충돌구 컨테이너
	CollisionSphereCont	m_CollisionSphereCont;

	bool	m_bCollIsOnyTargetLoc; // 무조껀 충돌지점까지 가서 충돌 체크 한다. 중간 충돌 체크는 무시

	enum	TargetType
	{
		TT_UNDEFINED=0,
		TT_OBJECT,
		TT_LOCATION
	};
	enum	State
	{
		S_UNDEFINED=0,
		S_LOADING,	//	장전되고 있는 상태
		S_DELAY_FIRING,	//	일정 시간 대기후 자동으로 날아간다
		S_FLYING,	//	타겟을 향해 날아가는 상태
		S_ARRIVED_AT_TARGET,	//	타겟에 도달한 상태
		S_REMOVING,	//	삭제중인 상태(파티클이 전부다 없어질때까지 기다린후, 삭제됨)
	};
	enum	ScriptCallEvent
	{
		SCE_NONE=0,
		SCE_LOADING_START=1<<0,
		SCE_LOADING=1<<1,
		SCE_FLYING_START=1<<2,
		SCE_FLYING=1<<3,
		SCE_ARRIVED_AT_TARGET=1<<4,
		SCE_ANIMATION=1<<5,
		SCE_LOST_TARGET=1<<6,
		SCE_UPDATE=1<<7,
		SCE_COLLISION=1<<8
	};

	std::string	m_kScriptName;
	std::string m_kTextureFolder;
	std::string	m_kID;

	TargetType	m_TargetType;
	State		m_State;
	
	bool	m_bMultipleAttack;
	bool	m_bDestroyMe;

	int		m_ScriptCallEvent;	
	
	PgActionTargetList	m_kActionTargetList;
	BM::GUID m_kParentGUID;		//	발사한 Pilot 

	NiNodePtr	m_spNif;

	typedef	std::map<std::string,std::string> ParamMap;

	ParamMap	m_ParamMap;

	int	m_iUID;	//	유니크 ID

	//	이 프로젝틸을 생성한 부모 액션정보
	int	m_iParentActionNo;
	int	m_iParentActionInstanceID;
	DWORD	m_dwParentActionTimeStamp;

	//	궤적 렌더링
	PgTrailNode	*m_pkTrail;
	std::string	m_kTrailTexturePath;
	int	m_iTrailTotalTime,m_iTrailBrightTime;
	float	m_fTrailExtendLength;
	std::string m_kTrailAttachNodeName;
	float	m_fTrailWidth;

	float	m_fDelayFireStartTime;
	float	m_fDelayFireTotalTime;

	bool	m_bHide;
	bool	m_bNoResetTargetLoc;

	//	충돌체크를 내 클라이언트에서 할 것인가?
	bool	m_bCollCheckByMe;

	//	타겟을 브로드캐스트 한 상태인가?
	bool	m_bTargetListBroadCasted;

	//	마지막으로 충돌한 시간
	float	m_fLastCollideTime;
	
	// 충돌 간격 시간
	float	m_fCollisionCheckSec;

	bool	m_bNoShadow;
	float	m_kAutoGround;
	bool	m_bIgnoreDefinedRange;	//Def에 정의된 Range를 무시할 것인가?

	float	m_fCollisionScale;	//충돌구 크기
public:

	//! 생성자
	PgProjectile();
	virtual	~PgProjectile()	{	Terminate();	}

	int const GetUID() const	{	return	m_iUID;	}
	void	SetUID(int ID)	{	m_iUID = ID;	}

	bool	IsCollCheckByMe()	{	return	m_bCollCheckByMe;	}

	void	SetParentActionInfo(int iActionNo,int iActionInstanceID,DWORD dwTimeStamp)
	{
		m_iParentActionNo = iActionNo;
		m_iParentActionInstanceID = iActionInstanceID;
		m_dwParentActionTimeStamp = dwTimeStamp;
	}
	int const	GetParentActionNo() const	{	return	m_iParentActionNo;	}
	int const	GetParentActionInstanceID() const	{	return	m_iParentActionInstanceID;	}
	DWORD	GetParentActionTimeStamp()	{	return	m_dwParentActionTimeStamp;	}

	void	SetParentPilot(BM::GUID const &kGUID)
	{
		m_kParentGUID = kGUID;
	}
	BM::GUID const &GetParentPilotGUID() const	{	return	m_kParentGUID;	}

	void SetCollIsOnyTargetLoc(bool const bCollIsOnyTargetLoc) { m_bCollIsOnyTargetLoc = bCollIsOnyTargetLoc; }
	bool GetCollIsOnyTargetLoc() const { return m_bCollIsOnyTargetLoc; }
	
	bool Update(float fAccumTime, float fFrameTime);
	void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	void Terminate();

	void	SetScriptName(std::string const &kScriptName)	{	m_kScriptName = kScriptName;	}
	const	std::string &GetScriptName()	{	return	m_kScriptName;	}
	void	SetParam(float fSpeed,float fAccel,float fMass);
	void	SetScale(float fScale);
	float	GetScale();
	NiPoint3 const& GetNodeWorldPos(char const* strNode)const;
	void	SetRotate(const NiQuaternion& rkQuat);
	void	SetTargetObject(PgActionTargetInfo &kTargetInfo);
	void	SetTargetObjectList(PgActionTargetList &kTargetInfoList,bool bNoSetTargetLoc=false);	//	리스트의 첫번째에 있는 놈이 목표위치가 된다.
	void	SetTargetLoc(NiPoint3 const	&vTargetLoc);
	void	Fire(bool bNoResetTargetLoc=false);
	void	DelayFire(float fDelayTime,bool bNoResetTargetLoc=false);	//	일정 시간 기다린 후에 자동으로 발사된다.
	
	void	LoadToPosition(NiPoint3 const &rkPos);
	void	LoadToWeapon(const PgActor *pkActor);	
	void	LoadToHelper(const PgActor *pkActor,char const* strHelperName);

	State	GetState()	{	return	m_State;	}
	TargetType	GetTargetType()	{	return	m_TargetType;	}

	std::string const& 	GetID()	const {	return	m_kID;	}
	void	SetID(std::string const &kID)	{	m_kID = kID;	}

	PgActionTargetInfo const*	GetActionTargetInfo(int const iIndex);
	int	GetActionTargetCount()	{	return	m_kActionTargetList.size();	}
	const	PgActionTargetList&	GetActionTargetList()	{	return	m_kActionTargetList;}
	const	NiPoint3	GetTargetLoc()	{	return	m_vTarget;	}

	void	ModifyTargetList(PgActionTargetList &kTargetList);

	PgProjectile*	CreateClone();

	bool	IsMultipleAttack()	{	return	m_bMultipleAttack;	}
	void	SetMultipleAttack(bool bIsMultipleAttack)	{	m_bMultipleAttack = bIsMultipleAttack;	}

	bool	IsDestroyMe()	{	return	m_bDestroyMe;	}

	NiPoint3	GetWorldPos();
	void	SetWorldPos(const	NiPoint3 &kPos);

	NiPoint3	GetFireStartPos()	{	return	m_vFireStartPos;	}

	float	GetCurrentSpeed();

	float	GetOriginalSpeed()	{	return	m_fOriginalSpeed;	}
	float	GetOriginalAccelation()	{	return	m_fOriginalAccelation;	}

	void	SetVelocity(NiPoint3 const &kVel);
	void	SetSpeed(float fSpeed)	{	m_fSpeed = fSpeed;	};
	float	GetSpeed() const { return m_fSpeed; };

	void	SetParam(char const*pParamName,char const*pParamValue);
	char const*	GetParam(char const*pParamName);

	void	SetMovingType(MovingType kMovingType);
	MovingType	GetMovingType()	{	return	m_MovingType;	}

	void	SetParam_SinCurveLineType(float fSinPower,float fSinRotateAngle);
	void	SetParam_Bezier4SplineType(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,NiPoint3 const &p4);
	void	SetHide(bool bHide)	{ m_bHide = bHide; }
	bool	GetHide()	{ return m_bHide; }

	void	SetAlpha(float	fAlpha);
	void	AttachSound(char const *pcSoundID, float fMin, float fMax);

	void	SetAutoDirection(bool bTrue)	{	m_bAutoDirection = bTrue;	}
	bool	GetAutoDirection()	{	return	m_bAutoDirection;	}

	void	SetTargetListBroadCasted(bool bBroadCasted)	{	m_bTargetListBroadCasted = bBroadCasted;	}
	bool	GetTargetListBroadCasted()	{	return	m_bTargetListBroadCasted;	}

	CLASS_DECLARATION_S(BM::GUID, TargetGuid);
	CLASS_DECLARATION_S(std::string, FireNode);
	CLASS_DECLARATION_S(std::string, TargetEndNode);
	CLASS_DECLARATION_S(float, HomingTime);
	CLASS_DECLARATION_S(float, HomingDelayTime);
	CLASS_DECLARATION_S(float, StayTime);
	CLASS_DECLARATION_S(float, AccumDistance);	//프로젝틸이 이동한 총 거리
	CLASS_DECLARATION_S(bool, ClearTargetList);
	CLASS_DECLARATION_S(int, PenetrationCount); // 관통 개체수	

	NiPoint3 GetDirection();
	NiPoint3 GetUp();
	NiPoint3 GetRight();
	float const	GetStartTime()	{ return m_fFireStartTime; }
	
	// 발사체가 작동하는데 필요한 추가 정보
	SOptionInfo GetOptionInfo() const {return m_kOptionInfo; }
	void SetOptionInfo(SOptionInfo const& kOptionInfo) { m_kOptionInfo = kOptionInfo; }
	
	// 충돌 체크 간격
	void SetCollisionCheckSec(float const fCheckTime) { m_fCollisionCheckSec = fCheckTime; }
	float GetCollisionCheckSec()  const { return m_fCollisionCheckSec; }
public:

	//	스크립트 호출
	void	OnLoadingStart();	//	장전시작
	void	OnLoading();	//	장전중
	void	OnFlyingStart();	//	발사 시작
	void	OnFlying();	//	날아가는 중
	void	OnArrivedAtTarget();	//	목표도달 완료
	void	OnTargetListModified();	//	서버로부터 타겟을 새로 받았다.
	void	OnAnimation(char const*pkEventName);	
	void	OnLostTarget();	//	타겟이 사라졌다
	void	OnUpdate();
	int		OnCollision(PgActor *pkCollideActor);	//	타겟과 충돌했다.
	void	OnRemove();

private:

	void	SetAlpha(float	fAlpha,NiAVObject *pkObject);


	void	UpdatePos(float fAccumTime, float fFrameTime);

	void	UpdatePos_StraightLineType(float fAccumTime, float fFrameTime);
	void	UpdatePos_SinCurveLineType(float fAccumTime, float fFrameTime);
	void	UpdatePos_Bezier4SplineType(float fAccumTime, float fFrameTime);
	void	UpdatePos_HommingType(float fAccumTime, float fFrameTime);
	void	UpdatePos_Stay(float const fAccumTime, float const fFrameTime);

	NiPoint3	GetLoc_SinCurveLineType(NiPoint3 &kStart,NiPoint3 &kEnd,float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd);
	NiPoint3	GetLoc_Bezier4SplineType(NiPoint3 &p1,NiPoint3 &p2,NiPoint3 &p3,NiPoint3 &p4,float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd);
	NiPoint3	GetLoc_HommingType(float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd);

	void	LookAtTarget(NiPoint3 &kStart,NiPoint3 &kTarget,float fAccumTime);

	void	StartRemove();

	bool	HasParticleNode(NiAVObject *pkObject);
	void	HideNoneParticleObject(NiAVObject *pkObject);
	bool	CheckAllParticleRemoved(NiAVObject *pkObject);

	void	InitNifData(NiAVObject *pkObject);

	void	CollisionCheck();
	void	ResetCollisionSphereBeforePos();
	void	UpdateCollisionSphereBeforePos();

	void	InitTrailNode();

	NiPoint3	ProcessorAutoGround(NiNodePtr const NodePtr, NiPoint3& PastPos, float const AccumTime);
	
private:
	SOptionInfo m_kOptionInfo;
}	
;

NiSmartPointer(PgProjectile);

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILE_H