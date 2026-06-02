#ifndef FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECTBASE_H
#define FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECTBASE_H

class PgActionSlot;
class PgIWorldObject;

class PgIWorldObjectBase : public NiMemObject
{
public:
	class ABVShape
	{
	public:
		typedef enum eShapeType
		{
			ST_NONE,
			ST_CAPSULE,
			ST_BOX,
			ST_SPHERE
		} EShapeType;

		NiFixedString m_kTo;
		EShapeType m_eType;
		NxCapsuleShapeDesc kCapsuleDesc;
		NxBoxShapeDesc kBoxDesc;
		NxSphereShapeDesc kSphereDesc;
		NxMat34 m_kMat;

		ABVShape()
		{
			m_eType = ST_NONE;
		}

		bool IsValid()
		{
			return m_eType != ST_NONE;
		}

		NxShapeDesc *GetPhysXShapeDesc()
		{
			if(m_eType == ST_CAPSULE)
			{
				return &kCapsuleDesc;
			}
			else if(m_eType == ST_SPHERE)
			{
				return &kSphereDesc;
			}
			return &kBoxDesc;
		}
	};

	struct	stEffectAttachInfo
	{
		std::string	m_kEffectID,m_kTo;
		int	m_iSlot;

		stEffectAttachInfo(std::string const &kEffectID,std::string &kTo,int iSlot)
			:m_kEffectID(kEffectID),m_kTo(kTo),m_iSlot(iSlot)
		{
		};
	};

	struct stCameraAnimationInfo
	{
		stCameraAnimationInfo(char const* pcCameraName, char const* pcCameraPath)
		{
			m_kCameraName = pcCameraName;
			m_kCameraAniFilePath = pcCameraPath;
			PG_ASSERT_LOG(!m_kCameraName.empty());
			PG_ASSERT_LOG(!m_kCameraAniFilePath.empty());
		};
		std::string m_kCameraName;
		std::string m_kCameraAniFilePath;
	};

	struct	stWorldEventStateInfo
	{
		struct	stState
		{
			int	m_iStateID;
			std::string	m_kSlot;

			stState()
			{
			};
			stState(int iStateID,std::string const &kSlotID)
				:m_iStateID(iStateID),
				m_kSlot(kSlotID)
			{
			};
		};
		struct	stTransit
		{
			int	m_iFromID,m_iToID;
			std::string	m_kSlot;

			stTransit()
			{
			};
			stTransit(int iFromID,int iToID,std::string const &kSlotID)
				:m_iFromID(iFromID),m_iToID(iToID),m_kSlot(kSlotID)
			{
			};
		};

		typedef	std::vector<stState> StateCont;
		typedef	std::vector<stTransit> TransitCont;

		StateCont	m_kStateCont;
		TransitCont	m_kTransitCont;

		bool GetStateSlot(int const iStateID,std::string &kout_Slot)	const
		{
			for(StateCont::size_type i = 0; i < m_kStateCont.size(); ++i)
			{
				if(m_kStateCont[i].m_iStateID == iStateID)
				{
					kout_Slot = m_kStateCont[i].m_kSlot;
					return	true;
				}
			}
			return	false;
		}

		bool GetTransitSlot(int iFrom,int iTo,std::string &kout_Slot)	const
		{
			for(TransitCont::size_type i = 0; i < m_kTransitCont.size(); ++i)
			{
				if(m_kTransitCont[i].m_iFromID == iFrom &&
					m_kTransitCont[i].m_iToID == iTo
					)
				{
					kout_Slot = m_kTransitCont[i].m_kSlot;
					return	true;
				}
			}
			return	false;
		}

	};

	typedef	std::list<stEffectAttachInfo> EffectAttachInfoList;
	typedef std::map<std::string, stCameraAnimationInfo> CameraAnimationInfoMap;
public:
	
	//! Constructor
	PgIWorldObjectBase();

	//! Destructor
	virtual ~PgIWorldObjectBase();

	//! NifRoot를 반환
	NiAVObject* GetNIFRoot() const;

	NiAVObject*	GetMainNIFRoot()	const;

	//! ActorManager를 반환
	NiActorManager* GetActorManager() const;

	//! SubActorManager를 반환
	NiActorManager* GetSubActorManager() const;

	//! Actio Slot을 반환
	PgActionSlot *GetActionSlot() const;

	//! Update Script를 반환.
	std::wstring const &GetUpdateScript() const;

	//! Event Script를 반환
	std::wstring const &GetEventScript() const;

	//! Pickup Script를 반환.
	std::wstring const &GetPickupScript() const;

	//! MouseOver Script를 반환.
	std::wstring const &GetMouseOverScript() const;

	//! MouseOut Script를 반환.
	std::wstring const &GetMouseOutScript() const;

	//! Init Script를 반환.
	std::wstring const &GetInitScript() const;

	//! Pickup Script 설정
	void SetPickupScript(std::wstring kScriptName);

	//! MouseOver Script 설정
	void SetMouseOverScript(std::wstring kScriptName);

	//! MouseOut Script 설정
	void SetMouseOutScript(std::wstring kScriptName);

	//! Update Script를 설정.
	void SetUpdateScript(std::wstring kScriptName);
	
	//! Effect Scale을 반환
	float GetEffectScale() const;

	//!	캐릭터 Scale 반환
	float	GetScale()	const	{	return	m_fScale;	}
	void	SetScale(float	fScale)	{	m_fScale = fScale;	}
	
	//! Index에 해당하는 ABVShape을 리턴한다.
	ABVShape* GetABVShape(int iIndex);
	
	//! 두 번째 WorldObjectBase를 셋팅한다.
	void SetSubIWorldObjectBase(PgIWorldObjectBase *pkSub);
	
	//! 두 번째 WorldObjectBase를 반환.
	PgIWorldObjectBase *GetSubIWorldObjectBase();

	//! IconID 를 반환
	std::wstring GetIconID() const { m_wstrIconID; };

	//! PgIWorldObjectBase를 만들기 위해 Xml을 파싱
	bool ParseXml(const TiXmlNode *pkNode, PgIWorldObject *pkWorldObject, bool bBeSubBase = false);

	//! 두 번째 WorldObjectBase를 삭제.
	void RemoveSubIWorldObjectBase();

	//! 오브젝트를 클론
	PgIWorldObjectBase *Clone();

	PgIWorldObjectBase*	CreateCopy(PgIWorldObject *pkWorldObject,bool bBeSubBase = false);

	//! Sub AM을 사용하도록 세팅
	bool GetExistSubActorManager() { return m_spSubAM != 0; }
	void SetUseSubActorManager(bool bUse);
	bool GetUseSubActorManager() { return m_bUseSubActorManager; }
	unsigned int GetLODCount();
	bool SetLOD(int iLODLevel);
	int GetCurrentLOD();
	bool SetActorManager(NiActorManagerPtr pkAM, PgIWorldObject *pkWorldObject, float fScale, bool bSubAM = false, const NiQuaternion &rkQuat = NiQuaternion(0.0f, 0.0f, 0.0f, 0.0f));
	void	ChangeActorManager(std::string const &kKFMPath,PgIWorldObject *pkWorldObject);

	// DemageBlink
	void SetDamageBlinkID( char const *pcID )	{	if ( pcID )	m_strDamageBlinkID = pcID;	}
	std::string& GetDamageBlinkID() {	return m_strDamageBlinkID;	}

	void AttachEffects(PgIWorldObject *pkWorldObject);
	void DetachEffects(PgIWorldObject *pkWorldObject);

	stWorldEventStateInfo const	*GetWorldEventStateInfo() const
	{
		return	&m_kWorldEventStateInfo;
	}
private:

	void	ParseWorldEventStateInfo(const TiXmlNode *pkNode);

private:

	//! Main ActorManager
	NiActorManagerPtr m_spAM;

	//! Sub ActorManager
	NiActorManagerPtr m_spSubAM;
	bool m_bUseSubActorManager;

	//! Action Slot
	PgActionSlot *m_pkActionSlot;

	EffectAttachInfoList	m_kEffectAttachInfoList;

	//! 기본 이펙트 스케일
	float m_fEffectScale;

	//! 캐릭터 Scale
	float m_fScale;

	// 캐릭터의 Damege Blink
	std::string m_strDamageBlinkID;
	
	//! 클릭 했을 때, 실행될 스크립트
	std::wstring m_kPickupScript;
	
	//! 매번 업데이트 시에 실행되는 스크립트
	std::wstring m_kUpdateScript;

	//! 마우스 오버 시에 실행되는 스크립트
	std::wstring m_kMouseOverScript;

	//! 마우스 아웃 시에 실행되는 스크립트
	std::wstring m_kMouseOutScript;

	//! Event Script Name
	std::wstring m_kEventScript;

	//! Init Script Name
	std::wstring m_kInitScript;	//	처음 생성된 직후에 실행되는 스크립트

	//! ABV Shape
	ABVShape m_kABVShapes[PG_MAX_NB_ABV_SHAPES];

	//! Sub WorldObject Base
	PgIWorldObjectBase *m_pkSubIWorldObjectBase;

	//! Icon ID
	std::wstring m_wstrIconID;

	CameraAnimationInfoMap m_kCameraAniMap;

	stWorldEventStateInfo	m_kWorldEventStateInfo;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGIWORLDOBJECTBASE_H