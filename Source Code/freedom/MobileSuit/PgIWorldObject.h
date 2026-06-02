#ifndef FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECT_H
#define FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECT_H

#include <NiNode.h>
#include "PgIXmlObject.h"
#include "PgIWorldObjectBase.h"

class PgWorld;
class PgAction;
class PgActionSlot;
class PgPilot;
class PgRenderer;
class NiCamera;

enum ObjectGroupType
{// typedef NxU16 NxCollisionGroup;		// Must be < 32
	OGT_NONE=0,
	OGT_PLAYER=1,
	OGT_MONSTER=2,
	OGT_GROUNDBOX=3,
	OGT_PET=5,
	OGT_NPC=6,
	OGT_PUPPET=7,
	OGT_ENTITY=9,
	OGT_SHINESTONE=10,
	OGT_OBJECT=11,
	OGT_EFFECT=12,
	OGT_FURNITURE=13,
	OGT_MYHOME=14,
	OGT_SIMILAR_PLAYER=15,
	OGT_MAX,// 16개를 넘을수 없음. PhysX의 그룹 비트가 16비트밖에 지원하지 않음.
};

class PgIWorldObject : public NiNode, public PgIXmlObject
{
	NiDeclareRTTI;
public:	
	class EventCallbackObject : public NiActorManager::CallbackAVObject
	{
	public:
		EventCallbackObject(PgIWorldObject *pkObject);
		virtual ~EventCallbackObject();

		virtual	void	EventActivated(NiActorManager *pkManager, 
			NiActorManager::EventType kEventType,
			NiActorManager::SequenceID eSequenceID, float fCurrentTime, float fEventTime, 
			NiAnimationEvent *pkAnimationEvent);

	private:

		void	DoAudioEvent(NiAnimationEvent *pkEvent);
		void	DoEffectEvent(NiAnimationEvent	*pkEvent);
		void	DoFadeOutEffectEvent(NiAnimationEvent *pkEvent);
		void	DoParticleFadeOutWhenSequenceChanged();

	protected:

		PgIWorldObject *m_pkObject;
		float m_fEventTime;
	};

	//! 생성자
	PgIWorldObject();

	//! 소멸자
	virtual ~PgIWorldObject();

	virtual	PgIWorldObject*	CreateCopy();
	virtual	void	Reset();

	//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
	virtual bool Update(float fAccumTime, float fFrameTime);
	
	//	파티클 위치 업데이트
	void	UpdateParticlesTransform(float fAccumTime, float fFrameTime);

	//! pkRenderer를 이용해서 Draw
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) = 0;
	//! pkRenderer를 이용해서 Draw
	virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	//! Z 버퍼 테스트를 하지 않는다.
	virtual void DrawNoZTest(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };

	//! 그림자를 렌더링한다.
	virtual void RenderShadow(PgRenderer *pkRenderer, NiCamera *pkCamera){};

	//! PhysX를 초기화한다.
	virtual void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup) {}
	virtual void ResetABVToNewAM() {}

	virtual void SyncPhysX() {}

	//! PhysX를 해제한다.

	virtual bool BeforeUse();		// Main Thread에서 사용하기 직전에 호출하기 위한 함수들. 적당한 이름이 안 떠올라서 이렇게 해놨다.
	virtual bool BeforeCleanUp();	// Delete Thread에 넣어서 지우기전에 Main Thread에서 정리해야 할 작업들

	virtual void ReleasePhysX() {}
	virtual void ReleaseAllParticles();

	virtual void Terminate();

	virtual	void	OnAbilChanged(int iAbilType,int iValue) {};

	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false) = 0;

	virtual	bool GetCanBatchRender() const { return false; };

	//! Path Node를 설정한다.
    void SetPathRoot( NiNode* pkPathRoot ) { m_pkPathRoot = pkPathRoot; };

	//! 이 액터가 내 클라이언트의 액터인가? 
	bool IsMyActor();

    PgPilot* GetPilot() const { return m_pkPilot; };
    void SetPilot( PgPilot* pkPilot ) { m_pkPilot = pkPilot; };

	//! 최근 패스의 노멀 벡터를 반환/설정 한다.
    const NiPoint3& GetPathNormal() const { return m_kPathNormal; };
    void SetPathNormal( NiPoint3& rkPathNormal ) { m_kPathNormal = rkPathNormal; };

	//! 최근 패스와 캐릭터의 충돌 지점을 반환/선택 한다.
    const NiPoint3& GetPathImpactPoint() const { return m_kPathImpactPoint; };
    void SetPathImpactPoint(NiPoint3 &rkPathImpactPoint) { m_kPathImpactPoint = rkPathImpactPoint; };

	//! 픽업 엑션 스크립트를 설정한다.
	void SetPickupScript(char const *pcScript);

	//! 마우스오버 엑션 스크립트를 설정한다.
	void SetMouseOverScript(char const *pcScript);

	//! 마우스아웃 엑션 스크립트를 설정한다.
	void SetMouseOutScript(char const *pcScript);

	//! 마우스오버 엑션 스크립트를 반환한다.
	bool GetMouseOverScript(std::wstring &rkScript);

	//! 마우스아웃 엑션 스크립트를 반환한다.
	bool GetMouseOutScript(std::wstring &rkScript);

	//! Update스크립트를 설정한다.
	void SetUpdateScript(char const *pcScript);
	
	//! 픽업 엑션을 구동한다. (NPC는 상점 오픈, 유저는 거래 메뉴 오픈 등)
	bool Pickup();

	//! 마우스오버 액션을 구동한다. (1차적으로 캐릭터 선택창에서 사용)
	bool MouseOver(bool &rbWorking);

	//! 마우스아웃 액션을 구동한다. (1차적으로 캐릭터 선택창에서 사용)
	bool MouseOut();

	//! 월드를 반환한다.
    PgWorld* GetWorld() const;

	//! GUID를 반환/설정 한다.
    BM::GUID const& GetGuid() const;
    void SetGuid( BM::GUID const& rkGuid ) { m_kGuid = rkGuid; };

	//! 지정된 곳에 Node를 붙인다.
	virtual	bool AttachTo(int iSlot, char const *pcTargetName, NiAVObject *pkObject);
	virtual	bool AttachToPoint(int iSlot, NiPoint3 kLoc, NiAVObject *pkObject);
	void	SetParticleAlphaGroup(int iSlot,int iAlphaGroup);
	NiPoint3 GetParticleNodeWorldPos(int iSlot, char *strNodeName);
	NiAVObject* GetParticleNode(int iSlot, char const* strNodeName);
	NiAVObject*	GetParticleNodeWithTextKey(std::string const &kTextKey);
	void	FadeOutParticleWhenSequenceChanged();

	//! 지정된 곳의 Node를 뗀다.
	virtual	bool DetachFrom(int iSlot, bool bDefaultThreadDelete = false);
	bool RemoveAVObject(NiAVObject* pObject, bool bDefaultThreadDelete = false);

	//! 지정된 곳의 파티클을 멈춘다.
	bool ChangeParticleGeneration(int iSlot, bool bGenerate);

	//! RayCheckObject 임펙트 위치를 설정한다.
	void SetHitPoint(NiPoint3 const &rkPoint);

	//! RayCheckObject 임펙트 위치를 반환한다.
	NiPoint3 &GetHitPoint();

	//! Event Callback을 생성하고, 등록한다.
	void RegisterEventCallback();
	void UnregisterEventCallback();

	//! SubEvent Callback을 생성하고, 등록한다.
	void RegisterSubEventCallback();
	
	//! Clone Process
	void ProcessClone(NiCloningProcess& kCloning);

	//!	RayCheckObject 를 통해 찾아낸 충돌오브젝트의 중심좌표를 설정한다.
	void	SetHitObjectCenterPos(NiPoint3 const &kPoint);
	//!	RayCheckObject 를 통해 찾아낸 충돌오브젝트의 중심좌표를 반환한다.
	const	NiPoint3	&GetHitObjectCenterPos();

	void	SetHitObjectABVIndex(int iIndex);
	int	GetHitObjectABVIndex()	{	return	m_iHitObjectABVIndex;	}

	std::wstring const& GetPickupScript()const { return m_pkWorldObjectBase ? m_pkWorldObjectBase->GetPickupScript() : L""; }

	bool GetExistSubActorManager() { return m_pkWorldObjectBase != 0 && m_pkWorldObjectBase->GetExistSubActorManager(); }
	bool GetUseSubActorManager() { return m_pkWorldObjectBase != 0 && m_pkWorldObjectBase->GetUseSubActorManager(); }
	virtual void SetUseSubActorManager(bool bUse) { if (m_pkWorldObjectBase) m_pkWorldObjectBase->SetUseSubActorManager(bUse); }

	//! LOD 관련
	unsigned int GetLODCount() { if (m_pkWorldObjectBase) return m_pkWorldObjectBase->GetLODCount(); return 0; }
	void SetLOD(int iLODLevel) { if (m_pkWorldObjectBase) m_pkWorldObjectBase->SetLOD(iLODLevel); }
	int GetCurrentLOD() { if (m_pkWorldObjectBase) return m_pkWorldObjectBase->GetCurrentLOD(); return 0; }

	//! Object 그룹 번호를 반환한다.
	unsigned int GetGroupNo()
	{
		return m_uiGroupNo;
	}

	//! Object 그룹 번호를 설정한다.
	void SetGroupNo(unsigned int uiGroupNo)
	{
		m_uiGroupNo = uiGroupNo;
	}

	virtual NiAVObject* GetNIFRoot() const;
	virtual	NiAVObject*	GetMainNIFRoot()	const;
	virtual NiActorManager* GetActorManager() const;
	virtual PgActionSlot *GetActionSlot() const;
	virtual PgIWorldObjectBase::ABVShape *GetABVShape(int iIndex);
	virtual bool RegisterCallback(NiActorManager::SequenceID kSeqID);
	virtual void OnVisible(NiCullingProcess& kCuller);
	
	//!	ActorManager가 완전히 로딩이 끝났을때, 호출해주는 메소드(일종의 노티파이 메소드?)
	virtual	void	NfyActorManagerLoadingComplete()	{};
	virtual	void	NfyActorManagerChanged(bool bBefore)	{};

	std::wstring const &GetUpdateScript() const;
	std::wstring const &GetInitScript() const;
	float GetEffectScale() const;
	float GetScale() const;
	void RemoveSubIWorldObjectBase();
	
	void SetSubIWorldObjectBase(PgIWorldObjectBase* pkBase);
	void SetWorldObjectBase(PgIWorldObjectBase* pkWorldObjectBase);

	PgIWorldObjectBase::stWorldEventStateInfo const* GetWorldEventStateInfo() const
	{
		if(!m_pkWorldObjectBase)
		{
			return	NULL;
		}

		return	m_pkWorldObjectBase->GetWorldEventStateInfo();
	}

	std::string& GetDamageBlinkID()	const {	return m_pkWorldObjectBase->GetDamageBlinkID();	}

	void	ReserveKFMTransit(char const* strKFMPath,bool bChangeImmediately = false)	
	{	
		if(strKFMPath) 
		{ 
			m_kReservedKFMTransit = strKFMPath;
		}	
		if(bChangeImmediately)
		{
			DoKFMTransition();
		}
	}
	std::string const &GetReservedKFMTransit()	{	return	m_kReservedKFMTransit;	}
	void	DoKFMTransition();

	void SetUseLOD( bool bUse ) { m_bUseLOD = bUse; };
	bool GetUseLOD() const { return m_bUseLOD; };
	void SetIgnoreCameraCulling( bool bUse ) { m_bIgnoreCameraCulling = bUse; };
	bool GetIgnoreCameraCulling() const { return m_bIgnoreCameraCulling; };

	NiPhysXScene* GetPhysXScene() { return m_pkPhysXScene; }

	int	GetWorldEventStateID()	const	
	{	
		return	m_iWorldEventStateID;	
	}
	__int64	GetWorldEventStateChangeTime()	const
	{
		return	m_iWorldEventStateIDChangeTime;
	}
	virtual	void	SetWorldEventStateID(int iNewID,__int64 iChangeTime,bool bSetImmediate)	
	{	
		m_iWorldEventStateID = iNewID;	
		m_iWorldEventStateIDChangeTime = iChangeTime;
	}

	virtual bool AttachToSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax);

protected:
	friend class PgClientWorkerThread;
	friend class PgIWorldObjectBase;
	//! PgIWorldObject Base
	PgIWorldObjectBase* GetWorldObjectBase() { return m_pkWorldObjectBase; }
	PgIWorldObjectBase* m_pkWorldObjectBase;
	bool m_bIsVisibleInFrustum;
	virtual void AttachChild(NiAVObject* pkChild, bool bFirstAvail = true);

	int	m_iWorldEventStateID;
	__int64	m_iWorldEventStateIDChangeTime;

//public:	// todo
	typedef std::map<int, NiAVObjectPtr> AttachSlot;

	NiNode *m_pkPathRoot;

	NiPoint3 m_kPathNormal;
	NiPoint3 m_kPathImpactPoint;

	NiPhysXScene *m_pkPhysXScene;
	
	//! render
	NiWireframePropertyPtr m_spWireframe;
	NiBoneLODController *m_pkBoneLOD;

	PgPilot *m_pkPilot;

	BM::GUID m_kGuid;

	//! 부착 컨테이너(파티클)
	AttachSlot m_kAttachSlot;

	//! 부착 컨테이너(파티클,Z버퍼 무시)
	AttachSlot m_kAttachSlot_NoZTest;

	//! RayCheckObject 임펙트 위치
	NiPoint3 m_kHitPoint;

	//! Auto Event Callback
	EventCallbackObject *m_pkEventCallback;

	//! Sub Auto Event Callback
	EventCallbackObject *m_pkSubEventCallback;

	NiPoint3	m_kHitObjectCenterPos;
	int	m_iHitObjectABVIndex;

	//! SlotNumber
	int m_iLastSlot;

	//! Object Group No
	unsigned int m_uiGroupNo;

	//!	KFM 변경 예약
	std::string	m_kReservedKFMTransit;

	//! LOD 사용 여부
	bool m_bUseLOD;
	bool m_bIgnoreCameraCulling;
};

NiSmartPointer(PgIWorldObject);

class	PgWOPriorityQueueNode
{
public:
	float	m_fDistance;
	PgIWorldObject	*m_pkObject;

	PgWOPriorityQueueNode()	{};
	PgWOPriorityQueueNode(PgIWorldObject *pkObj,float fDistance)	{	m_pkObject = pkObj;	m_fDistance = fDistance;	}
};

class	PgWOPriorityQueue	//	거리가 작은 오브젝트가 앞에 오게 된다.
{
public:
	typedef	std::list<PgWOPriorityQueueNode> ObjList;

private:

	ObjList	m_ObjList;

public:
	
	void	Clear();
	void	AddObj(PgIWorldObject *pkObject,float fDistance);
	ObjList* GetList()	{	return	&m_ObjList;	}
	bool	CheckExist(PgIWorldObject *pkObject);
};
#endif // FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECT_H