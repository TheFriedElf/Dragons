#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DROPBOX_PGDROPBOX_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DROPBOX_PGDROPBOX_H
#include "PgIWorldObject.h"
#include "Pg2DString.h"
#include "PgPilot.h"
#include "PgActor.h"

class Pg2DString;
class PgAction;

class PgDropBox 
	: public PgIWorldObject
{
	NiDeclareRTTI;
private:
	typedef std::vector< PgBase_Item > CONT_ITEM_ARRAY;
	typedef struct tagDropTransform
	{
		int				m_iID;
		NiQuaternion	m_kQuat;
		float			m_fRotSpeed;
		float			m_fScale;
		NiPoint3		m_kPos;
		NiPoint3		m_kOriginPos;
		std::wstring	m_wstrEffectName;
		std::string	m_strDropSoundName;
		std::string	m_strCatchSoundName;
		float			m_fUpDown;
		float			m_fUpDownSpeed;
		tagDropTransform()
		{
			Clear();
		}
		void Clear()
		{
			m_iID = 0;
			m_kQuat = NiQuaternion::IDENTITY;
			m_fRotSpeed = 0.0f;
			m_fScale = 1.0f;
			m_kPos = NiPoint3::ZERO;
			m_kOriginPos = NiPoint3::ZERO;
			m_wstrEffectName = _T("ef_Drop_Item_01");
			m_strDropSoundName.clear();
			m_strCatchSoundName.clear();
			m_fUpDown = 5.0f;
			m_fUpDownSpeed = 13.0f;
		}
	}SDropTransform;
	
	typedef std::map<int, SDropTransform> DropTransformList;
public:
	typedef enum eboxstate
	{
		E_BOX_NONE = 0,
		E_BOX_HIDE,
		E_BOX_JUMP,
		E_BOX_IDLE,
		E_BOX_EAT,
	}E_BOX_STATE ;

	PgDropBox();
	virtual ~PgDropBox();

	//! PgIWorldObject 재정의
	virtual bool Update(float fAccumTime, float fFrameTime);

	//! PgIWorldObject 재정의
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };

	//! Action
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false);

	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	void SetRotation(const NiQuaternion &kQuat);

	//! PgIWorldObject 재정의
	virtual void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);

	//! PhysX를 해제한다.
	virtual void ReleasePhysX();
	virtual void ReleaseABVShapes();
	virtual void CreateABVShapes();

	bool SetPosition(NiPoint3 const &rkTranslate);

	//! PgIXmlObject 재정의
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	//! 아이템을 추가한다.
	HRESULT AddItem(PgBase_Item const &rkItem);
	
	//! 아이템의 갯수를 반환한다.
	size_t GetCount()const;
	
	//! 아이템을 뽑는다.
	HRESULT PopItem(BM::GUID const &rkItemGuid, PgBase_Item &rkOutItem);

	//! 아이템을 뽑는다.
	HRESULT PopItem(size_t const szPos, PgBase_Item &rkOutItem);

	//! 아이템을 반환한다.
	PgBase_Item *GetItem(size_t szPos);

	//! 픽업 가능한 아이템인지 체크한다.
	bool CheckPickup();

	//! 엑션을 구동한다. (열기, 닫기, 제거 등)
	bool TransitAction(char const *pcAction);

	//! 주인의 GUID를 설정한다.
	void SetOwnerGuid(BM::GUID const &rkGuid);

	//! 제공자의 GUID를 설정한다.
	bool SetOfferer(BM::GUID const &rkGuid, NiPoint3 const &rkOffererPos, float const fJumpHeight = 0);

	//! 주인의 GUID를 반환한다.
	BM::GUID& GetOwnerGuid();

	//! 제공자의 GUID를 반환한다.
	BM::GUID& GetOffererGuid();

	//! 박스가 가지고 있는 아이템 개수
	int ItemCount();

	bool	IsMoney();
	bool	IsEquip();
	bool	IsConsume();
	bool	IsETC();

	void SetName(std::wstring const &wName);
	void SetItemNum(int const iNum);
	void SetItemNum(PgBase_Item const &rkItem);
	void SetNameColor(DWORD dwColor);
	void ChaseOwner();

	void	SetHide(bool bHide);
	bool	GetHide();

	virtual NiActorManager* GetActorManager() const;

	E_BOX_STATE GetBoxState() const { return m_eBoxState; }

protected:
	void StartJump(float fHeight);

	CLASS_DECLARATION_S(NiPoint3, PastPos);

protected:


	E_BOX_STATE			m_eBoxState;
	CONT_ITEM_ARRAY		m_kItemArray;				//! 상자에 든 아이템들
	NiAudioSourcePtr	m_pkCurrentAudioSource;		//! 재생 중인 사운드 소스
	BM::GUID			m_kOwnerGuid;				//! 주인의 GUID
	BM::GUID			m_kOffererGuid;				//! 상자를 주는 놈의 GUID
	NiPoint3			m_kOffererPos;				//
	Pg2DString			*m_pNameText;				//! 이름 렌더링
	float				m_fLifeTime;				//! 필드 위에 남아 있는 시간(초)
	NiColorA			m_kNameColor;

	std::wstring		m_kName;

	bool				m_bChaseOwner;
	std::string			m_strOwnerEffect;			// 획득 완료시 오너한테 붙을 이펙트
	std::string			m_strOwnerEffect_TargetName;// 획득 완료시 오너한테 붙을 이펙트의 위치
	float				m_fChaseStartTime;
	float				m_fSinProgress;
	NiPoint3			m_kptOriginPos;

	bool				m_bLove_BaseItem;			// 베이스기지 용 아이템인가

	bool				m_bJump;					//! 점프중인가?
	float				m_fJumpTime;				//! 점프 한 시간
//	float				m_fJumpAccumHeight;			//! 점프 한 높이
	bool				m_bFloor;					//! 바닥인가
	float				m_fInitialVelocity;			//! 점프 초기 속도
	static float		ms_fGravity;				//! Actor가 점프시에 영향을 받는 중력

#ifdef PG_USE_DROPBOX_EAT_PATTERN1
	PgPilot				*m_pkOwnerPilot;
	PgActor				*m_pkOwnerActor;
#endif

	NxRaycastHit		m_kBottomRayHit;

//	NiNodePtr			m_spParticleNode;	// Detail의 파티클.				//! 파티클들만 모아논 노드 리스트

	static DropTransformList	ms_kDropTransformList;
	static bool ms_bLoaded;

	NiActorManager*		m_pkAM;
	PgItemEx*			m_pkItemEX;

#ifdef PG_USE_CAPSULE_CONTROLLER
	NxCapsuleController *m_pkController;
#else
	NxBoxController *m_pkController;
#endif
	//! PhysX 관련
	NiPhysXKinematicSrc *m_pkPhysXSrc;
	NiPhysXTransformDest *m_pkPhysXDest;
	NxActor *m_pkPhysXActor;

	NxActor *m_apkPhysXCollisionActors[PG_MAX_NB_ABV_SHAPES];
	NiPhysXKinematicSrc *m_apkPhysXCollisionSrcs[PG_MAX_NB_ABV_SHAPES];


	CLASS_DECLARATION_S(bool, RemoveReserve);
	CLASS_DECLARATION_S(bool, IsGetDropItemRes);
	CLASS_DECLARATION_S(int, ItemNum);
	CLASS_DECLARATION_S(float, RotDegree);
	CLASS_DECLARATION_S(bool, IsMine);
	CLASS_DECLARATION_S(std::string, PickupSound);
	CLASS_DECLARATION_S_NO_SET(float, CreateTime);

	CLASS_DECLARATION_S(WORD, RemainTime);

	SDropTransform		m_kNowDropTransform;

	std::string	m_ParticleName;
	std::string	m_ParticleNode;

	bool LoadTransformXml(char const* szFile);
	virtual bool ParseTransformXml(const TiXmlNode *pkNode, void *pArg = 0);
	int const GetAbil(WORD const wAbil = AT_DROPITEMRES);
	void InitDropBox();
	void SetOriginMesh();
	//bool CalcIsMine();
	bool SetTotalAlpha(NiNode *pkRoot, float const fAlpha);
	bool PlaySound(char const *szPath);

	PgItemEx*	GetItemEx()	{	return	m_pkItemEX;	}

private:

	PgTrailNode	*m_pkTrail;
	bool m_bMeshLoaded;

	void	StartTrail();
	void	StopTrail();

	bool	m_bContactGround;	//땅에 처음 닫았는가?
	void	AttachCircleShadow(NiPoint3 const& rkPos);
	void	RemoveNextUpdate();
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DROPBOX_PGDROPBOX_H