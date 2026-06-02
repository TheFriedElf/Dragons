
#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGWORLD_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGWORLD_H



#include "Variant/PgSmallAreaInfo.h"
#include "Variant/PgEmporia.h"
#include "PgIScene.h"
#include "PgIWorldObject.h"
#include "PgCameraMan.h"
#include "HDRScene.h"
#include "PgCameraModeFollow.h"
#include "PgOctree.H"
#include "PgBuilding.h"
#include "PgMultiThreadSceneGraphUpdater.H"
#include "PgWorldElementFog.H"
#include "PgLinearInterpolation.h"
#include "PgMinimapType.h"
#include "PgDungeonMinimap.h"

#include "NewWare/Scene/AssetManager.h"
#include "NewWare/Scene/SceneSpacePartition.h"
#include "NewWare/Renderer/DrawWorkflow.h"



namespace NewWare
{ 
    namespace Scene
	{ 
        class DomainNodeManager;
    } //namespace Scene
} //namespace NewWare

class PgInput;
class PgCameraMan;
class PgTrigger;
class PgShineStone;
class PgActor;
//class PgMood;
//class PgSeasonMan;
//class PgMoodMan;
class PgScreenBreak;
class PgDamageNumMan;
class PgOctree;
class PgWater;
class PgWorldActionMan;
class PgPuppet;
class PgHome;
class PgSelectStage;
class PgWOPriorityQueue;
class Pg2DString;
class PgAVObjectAlphaProcessManager;
class PgSpotLight;
class	PgRope;
class PgPickObjectNullChecker;
class	PgPSRoomGroup;

#define MAX_LOD_RANGE_NUM	3
#define PG_SET_MAPTEXTURE_DIR	"../Data/3_World/Map_Textures/"

#define MAX_LIGHT_FOR_OBJECT	5	//	Ambient Light 는 포함되지 않음
#define MAX_OBJ_CONT	2

#define DEFAULT_MINIMAPCAMERA	"default"


typedef std::set<int> SET_INT;
typedef std::vector<PgTrigger *> CONT_TRIGGER_INFO;

namespace WorldObjectGroupsUtil
{
	// ObjectGroups 에서 Child를 Attach
	struct AttachChild
	{
		AttachChild(NiAVObject* pkChildNode, bool const bFirstAvail = true);
		AttachChild(AttachChild const& rhs);
		bool operator() (NiNode* pkRootNode);
	private:
		NiAVObject* m_pkChildNode;
		bool m_bFirstAvail;
	};

	// ObjectGroups 에서 Child를 Detach
	struct DetachChild
	{
		DetachChild(NiAVObject* pkChildNode);
		DetachChild(DetachChild const& rhs);
		bool operator() (NiNode* pkRootNode);
	private:
		NiAVObject* m_pkChildNode;
	};
};

enum	WORLD_OBJECT_CONT_ID
{
	WOCID_BEFORE=0,
	WOCID_MAIN=1
};

typedef struct stFocusFilterInfo
{
	stFocusFilterInfo()
	{
		pTexture1 = NULL;
		pTexture2 = NULL;
		fAlphaStart = 0.0f;		
		fAlphaEnd = 0.0f;
		fStartTime = 0.0f;
		fTotalTime = 0.0f;
		fInterval = 0.0f;
		bAutoTurnOff = true;
		bRenderObject = true;
		bAlphaEndIsMaxValueRenderObject = true;
	}

	NiScreenTexturePtr pTexture1;
	NiScreenTexturePtr pTexture2;

	float fAlphaStart;
	float fAlphaEnd;
	float fStartTime;
	float fTotalTime;
	float fInterval;
	bool bAutoTurnOff;
	bool bRenderObject;
	bool bAlphaEndIsMaxValueRenderObject;// 알파가 맥스값(1.0)일때 오브젝트를 렌더링 할 것인가?
} FocusFilterInfo;

typedef struct tagPhysXShapeDesc
{
	tagPhysXShapeDesc()
	{
		pkShapeDesc = 0;
		usNumNxVerts = 0;
		pkNxVerts = 0;
		usNumNxTris = 0;
		pkNxTriData = 0;
	}
	NiPhysXShapeDesc *pkShapeDesc;
	unsigned short usNumNxVerts;
	NxVec3* pkNxVerts;
	unsigned short usNumNxTris;
	NxU32* pkNxTriData;

} SPhysXShapeDesc;

typedef struct tagMapMoveCompleteFade
{
	tagMapMoveCompleteFade():dwFadeColor(0),fFadeTime(0.f),bFadeIn(false){}

	void SetFadeColor(char const * szColor)
	{
		if(szColor)
		{
			int	iR=255,iG=255,iB=255;
			sscanf_s(szColor,"%d,%d,%d",&iR,&iG,&iB);
			dwFadeColor = (0xff000000 | iR<<16 | iG<<8 | iB);
		}	
	}

	DWORD dwFadeColor;
	float fFadeTime;
	bool bFadeIn;
} SMapMoveCompleteFade;

typedef enum
{
	ALPHA_PROCESS_CULL_FALSE		= 0,	//보이는 상태
	ALPHA_PROCESS_CULL_ALPHA_ADD,			//알파가 올라가는 상태
	ALPHA_PROCESS_CULL_ALPHA_SUB,			//알파가 내려가는 상태
	ALPHA_PROCESS_CULL_TRUE,				//보이지 않는 상태
	

} AlphaProcessCullState;


typedef	enum	
{
	AG_OBSTRUCTOR = -8,
	AG_EFFECT = 1,
	AG_SHADOW = 2,	
	AG_OBJECT = 3,

} AlphaGroup;

typedef	enum	
{
	SLMT_NONE		= 0,
	SLMT_LINEAR		= 1,	// 선형시간으로 업데이트가 이루어짐
} SlowMotionType;


typedef struct tagRemoteObject
{
	std::string kTargetObjectName;
	std::string kStartAniID;
	std::string kNextAniID;
	DWORD dwStartAniTime;
	bool bUseRandomStart;
	tagRemoteObject()
	{
		dwStartAniTime = 0;
		bUseRandomStart = false;
	}
}SREMOTE_OBJECT;
typedef std::map<std::string, SREMOTE_OBJECT> CONT_MAP_REMOTE_OBJECT;

typedef struct tagCurrentRemoteObjcet
{
	SREMOTE_OBJECT kObjectInfo;		//제어할 오브젝트 정보
	DWORD dwInitTime;				//초기화 된 시각
	DWORD dwRandomDelayTime;		//랜덤하게 시작할 경우, 딜레이 시간 계산용으로 사용 될 시각
	bool bIsPlay;					//플레이 한 오브젝트 파일
	tagCurrentRemoteObjcet()
	{
		dwInitTime = 0;
		dwRandomDelayTime = 0;
		bIsPlay = false;
	}
}SCURRENT_REMOTE_OBJECT;
typedef std::vector<SCURRENT_REMOTE_OBJECT> CONT_VEC_CURRENT_REMOTE_OBJECT;

class	PgUserRaycastReport :public NxUserRaycastReport
{

public:

	typedef	std::vector<NxRaycastHit> HitCont;

private:

	HitCont	m_kHitReports;

public:

	void	Clear()
	{
		m_kHitReports.clear();
	}

	HitCont	const&	GetHitReports()	const	{	return	m_kHitReports;	}

	virtual bool onHit(const NxRaycastHit& hits)
	{
		m_kHitReports.push_back(hits);

		return	true;
	}

};

class PgWorld
	: public PgIScene
{
public:

	enum	LIGHT_TYPE
	{
		LT_NONE=0,
		LT_DIRECTIONAL=(1<<0),
		LT_AMBIENT=(1<<1),
		LT_POINT=(1<<2),
		LT_ALL
	};

	typedef std::map<BM::GUID, PgIWorldObject *> ObjectContainer;
	typedef std::map<BM::GUID, NiAVObjectPtr> AVObjectContainer;
	typedef std::map< std::string, NiEntityInterface* > NiEntityInterfaceContainer;
	typedef std::map< std::string, NiNode* > NodeContainer;
	typedef std::map< std::string, NiLODNode* > OptimizationPolyCont;
	typedef std::list<PgIWorldObject*> ObjectContainerEx;
	typedef std::list<NiAVObject*> AVObjectList;
	typedef std::map<int,NiAVObjectPtr> AVObjectMap;

	typedef std::map<std::string, PgTrigger *> TriggerContainer;
	typedef std::map<BM::GUID, PgShineStone *> ShineStoneContainer;
	typedef std::map<unsigned int, std::string> InputContainer;
	
	typedef std::set<BM::GUID> RemoveObjectContainer;
	typedef std::list<BM::GUID> UnfreezeObjectContainer;

	typedef std::map<std::string, PgPuppet *> PuppetContainer;

	typedef std::map<int, std::string> CompleteBGContainer;
	typedef CompleteBGContainer::iterator CompleteBGContainerIter;

	typedef std::set< BM::GUID > ContActorFilter;
	typedef std::map< std::wstring, SPhysXShapeDesc > ScalePhysXCont;		// Unique Name, PhysXDesc struct

	typedef std::vector< NiFixedString >	StringContainer;
	typedef std::map<size_t,NiAVObject*>	ContPvPLamp;
	typedef StringContainer::iterator	StringIter;
	typedef std::map< DWORD , int > ShineStoneCountCont;
	typedef std::vector< NiAVObject* >	AVObjectVec;

	typedef std::vector<NiPhysXScenePtr>	PhysXSceneVec;

	// building
	typedef std::map< short, PgBuilding >				CONT_BUILDING;
	typedef std::map< std::string, SEmporiaGuildInfo >	CONT_EMPORIA_GATE_INFO;

	typedef std::list<int> IntList;

	typedef std::map<int, PROGRESSINFO> CONT_PROGRESS_INFO; //던전 미니맵 UI 좌표 정보

	// BgSound
	typedef std::map< std::string, float >				BgSoundContainer;
	
	struct	stMapObject
	{
		NiAVObjectPtr	m_spAVObject;
		PhysXSceneVec	m_kPhysXSceneCont;
		bool	m_bHasAnimation;
		bool	m_bNoPortalSystem;

		stMapObject():m_bHasAnimation(false)
		{
		};
		stMapObject(NiAVObject *pkAVObject,PhysXSceneVec const &kPhysXSceneCont,bool bHasAnimation,bool bNoPortalSystem)
			:m_spAVObject(pkAVObject),m_kPhysXSceneCont(kPhysXSceneCont),m_bHasAnimation(bHasAnimation),m_bNoPortalSystem(bNoPortalSystem)
		{
		};
	};

	typedef	std::map<std::string,stMapObject> MapObjectMap;

	struct	stWorldText	:	public	NiMemObject
	{
		NiPoint3	m_kPosition;
		Pg2DString	*m_pkText;
		float	m_fScale;
		NiColorA	m_kColorA;

		stWorldText(NiPoint3 const &kCenter,std::wstring const &kText,float fScale,const NiColorA &kColor,std::wstring const &kFontName);
		~stWorldText();

		void	Draw(PgRenderer *pkRenderer,NiCameraPtr spCamera);
		bool	operator ==(stWorldText const& rhs) const;
	};

	typedef	std::list<stWorldText*> WorldTextCont;
	WorldTextCont	m_WorldTextCont;

	struct	stWaveEffect
	{
		std::string	m_kEntityID,m_kStopEffectID,m_kMoveEffectID;
		NiAVObject	*m_pkEntityNode;
		stWaveEffect():m_pkEntityNode(NULL)
		{
		};
	};
	typedef std::vector<stWaveEffect> WaveEffectVec;
	WaveEffectVec	m_vWaveEffect;

	static	bool	m_bNowLoading;


	struct	stItemEquipInfo
	{
		BM::GUID	m_kUnitGUID;
		int	m_iItemNo;
		bool	m_bSetToDefaultItem;
		PgItemEx	*m_pkEquipItem;

		stItemEquipInfo()
			:m_iItemNo(0),m_bSetToDefaultItem(false),m_pkEquipItem(0)
		{
		};
		stItemEquipInfo(BM::GUID const &kGUID,int iItemNo,bool bSetToDefault,PgItemEx *pkEquipItem)
			:m_kUnitGUID(kGUID),m_iItemNo(iItemNo),m_bSetToDefaultItem(bSetToDefault),m_pkEquipItem(pkEquipItem)
		{
		};
	};

	enum EFFECT_BAR_BLINK_TYPE
	{
		EBT_NONE=0,
		EBT_BAR_TWINKLE=(1<<0),
		EBT_SCREEN_BLINK=(1<<1),
	};

	struct XUI::tagListItem;
	typedef struct tagEffectRemainTime
	{
		unsigned long		ulEndTime;
		wchar_t const*		szName;
		XUI::SListItem*		pkListItem;
		int					iDurationTime;
		EFFECT_BAR_BLINK_TYPE	eType;	//종료시점에서 해야 할 일
		tagEffectRemainTime() : ulEndTime(0), pkListItem(NULL), szName(NULL), iDurationTime(0), eType(EBT_NONE)
		{
		}

		tagEffectRemainTime(wchar_t const* pkName, unsigned long const ulTime, XUI::SListItem* pkItem)
		{
			Set(pkName, ulTime, pkItem);
			iDurationTime = 0;
			eType = EBT_NONE;
		}

		void Set(wchar_t const* pkName, unsigned long const ulTime, XUI::SListItem* pkItem = NULL);
		bool Update(unsigned long const ulAccumTime);
		void MakeText(unsigned long const ulRemainTime);
	} SEffectRemainTime;

	typedef std::multimap<unsigned long, SEffectRemainTime> ContEffectRemainTime;

	typedef	std::list<stItemEquipInfo> ItemEquipInfoList;
	ItemEquipInfoList	m_kItemEquipInfoList;
	ACE_RW_Thread_Mutex	m_kItemEquipInfoListMutex;

	typedef	std::map<std::string,NiCameraPtr> CONT_CAMERA;

public:
	PgWorld(void);
	virtual ~PgWorld(void);

public:
	//! 월드를 렌더링 한다.
	void Draw(PgRenderer *pkRenderer, float fFrameTime);

	void DrawReflectionWorldScene( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime );
	void DrawFocusFilter( PgRenderer* pkRenderer  );

	//!	프로젝션 맵을 업데이트 한다.
	void	DrawProjectionMap(PgRenderer *pkRenderer, NiCameraPtr spCamera,float fFrameTime);

	void DrawObjects(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime);
	void DrawImmediateActor(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime);
	void DrawNoZTest(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime);
	
	std::string& GetStartScript() { return m_kStartScript; }
	std::string& GetReadyScript() { return m_kReadyScript; }
	std::string& GetBossDieScript() { return m_kBossDie; }
	virtual void OnAddScript();
	virtual void OnRemoveScript();
	virtual void OnTerminateScript();

	static	bool	GetNowLoading()		{	return	m_bNowLoading;	}
	static	void	SetNowLoading(bool bLoading)	{	m_bNowLoading = bLoading;	}

	//! 월드를 최신 상태로 유지한다.
	bool Update(float fAccumTime, float fFrameTime);
	bool UpdateScene(float fAccumTime, float fFrameTime);	
	
	//! 새 월드 오브젝트를 생성해서 등록한다.
	bool AddObject(BM::GUID const &rkGuid, PgIWorldObject *pkObject, NiPoint3 &rkLoc, ObjectGroupType kGroup);

	//!	월드 텍스트 추가
	void	AddWorldText(NiPoint3 const &kCenterPos,std::wstring const &kText,float fScale=1.0f,const NiColorA &kColor=NiColorA(1,1,1,1),std::wstring const &kFontName=_T("FONT_NAME"));
	void	DelWorldText(NiPoint3 const &kCenterPos,std::wstring const &kText,float fScale=1.0f,const NiColorA &kColor=NiColorA(1,1,1,1),std::wstring const &kFontName=_T("FONT_NAME"));
	void	ClearAllWorldText();
	void	DrawWorldText(PgRenderer *pkRenderer,NiCameraPtr spCamera);
	
	PgIWorldObject* AddUnit(CUnit *pkUnit, PgPilot* pkPilot = NULL,float fPacketReceiveTime=0);
	static PgPilot* CreatePilot(CUnit* pkUnit, bool& bNew);

	//! 등록된 월드 오브젝트를 삭제한다.
	bool RemoveObject(BM::GUID const &rkGuid, bool bPreserve = false);

	//! 등록된 월드 오브젝트를 삭제한다. (다음 업데이트에서 삭제한다.)
	void RemoveObjectOnNextUpdate(BM::GUID const &rkGuid);

	//! 등록된 월드 오브젝트를 반환한다.
	PgIWorldObject *FindObject(BM::GUID const &rkGuid)const;

	//! 등록된 월드 오브젝트를 반환한다.
	PgIWorldObject *FindObjectByName(char const *pkObjectName)const;

	//!	오브젝트가 실제로 월드에 존재하는지 체크한다(에러 체크용)
	bool	CheckObjectExist(PgIWorldObject *pkObject)const;
	//!	트리거가 실제로 월드에 존재하는지 체크한다(에러 체크용)
	bool	CheckTriggerExist(PgTrigger *pkObject);
	//!	샤인스톤이 실제로 월드에 존재하는지 체크한다(에러 체크용)
	bool	CheckShineStoneExist(PgShineStone *pkObject);

	//! 등록된 모든 오브젝트들을 삭제한다.
	void ClearAllObjects(bool bExceptNPC = false);	// TODO: 같은맵에서 이동시 NPC를 안지우기 위한 땜빵.

	//! 새 꼭두각시를 추가한다.
	PgPuppet *AddPuppet(BM::GUID &rkGuid, char const *pcPuppetName, NiPoint3 kTranslate, NiQuaternion kRotate);
	PgPuppet *AddPuppet_ActorXML(std::string const &kName,std::string const &kXmlPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger);
	PgPuppet *AddPuppet_KFMPath(std::string const &kName,std::string const &kKFMPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger);

	PgRope*	AddRope(char const *strRopeName,NiPoint3 const &kStart,NiPoint3 const &kEnd,float fRange);

	void	AddWaveEffect(char const *strEntityID,char const *strStopEffectID,char const *strMoveEffectID);
	void	UpdateWaveEffect(PgIWorldObject *pkWorldObject,bool bMoveEffect = true);

	//! 새로운 ShineStone을 월드에 추가한다.
	bool AddShineStoneToWorld(BM::GUID const &kStoneGuid, POINT3 const &ptPos);

	//! 새 월드 이펙트 오브젝트를 추가
	void	AddEffectObject(PgIWorldObject *pkObject);
	//!	 등록된 이펙트 오브젝트를 다음 업데이트 시 삭제되도록 한다.
	void	RemoveEffectObjectOnNextUpdate(PgIWorldObject *pkObject);

	//! AddUnitQueue 에서 유닛을 제거한다.
	bool RemoveUnitFromAddUnitQueue(BM::GUID const &kGUID);

	//! XML 'STONES' 노드를 파싱한다.
	virtual bool ParseXmlForShineStone(TiXmlNode const *pkNode, void *pArg = 0);	

	//! XML 'COMPLETE_PATH' 노드를 파싱한다.
	virtual bool ParseXmlForCompleteBG(TiXmlNode const *pkNode, void *pArg = 0);

	//! XML 'NPC' 노드를 파싱한다.
	bool ParseXmlForNpc( TiXmlElement const * pkElement, BM::GUID &rkOutNpcGuid );

	//! XML 'TRIGGER' 노드를 파싱한다.
	virtual bool ParseXmlForTrigger(TiXmlNode const *pkNode, PgTrigger *&pkTrigger);

	//! XML 'PROGRESS' 노드를 파싱한다. (생성되는 오브젝트는 Trigger이지만 생성 방식은 다르다.)
	virtual bool ParseXmlForProgress(TiXmlElement const * pkElement);
	virtual bool ParseXmlForProgressWay(TiXmlElement const * pkElement);

	//! XML 'WORLD' 노드를 파싱한다.
	virtual bool ParseXml(TiXmlNode const *pkNode, void *pArg = 0, bool bUTF8 = false);
	
	//! 입력을 처리한다.
	virtual bool ProcessInput(PgInput *pkInput);

	//! 맵 로딩 처리 관련 함수
	bool CheckRootNode(NiNode *pkRootNode);

	//! 기본 노드 세팅.
	bool DefaultNodeSetting(NiNode *pkRootNode);

	//! 해당 PhysX 물체에 Scale을 먹인다.
	void SetScaleToPhysXObject(NiPhysXSceneDesc *pkDesc, float fScale, NxVec3 kOffset = NxVec3(0.0f, 0.0f, 0.0f));

	// LoadGsa2, Scale이 1인 오브젝트를 컨테이너에 추가한다.
	void InsertPhysXDefObject(NiPhysXSceneDesc* pkDesc, const std::string& rkName);

	//! LoadGsa2, 다른 스케일의 PhysXDesc를 만든다. (m_kScalePhysXCont에 원본을 복사해서)
	void MakePhysXScaleObject(NiPhysXSceneDesc* pkDesc, float fScale, const std::string& rkName);


	//! 해당 노드의 모든 Geometry를 찾는다.
	void GetAllGeometries(NiNode const* pkNode, NiObjectList& kGeomerties);

	//! 노드에 있는 오브젝트들의 텍스쳐에 접미자를 붙여 새로운 텍스쳐를 로드한다. 새로운 텍스쳐가 없을 경우 기본 텍스쳐를 씀.
	void SetPostFixTextureToTexture(std::string const &rkPostfixTextureName, NiTexturingProperty::Map *pkMap);
	void SetPostFixTextureToNode(std::string const &rkPostfixTextureName, NiNode *pkObjectNode);

	//! GSA 파일을 로드 한다.
	bool LoadGsa(char const *pcGsaPath);
	
	//! NIF 파일을 로드 한다.
	bool LoadNif(char const *pcNifPath);


	//!	프로젝션 맵을 생성,파괴한다.
	void	CreateProjectionMap();
	void	DestroyProjectionMap();
	//!	노드와 하위노드들에게 프로젝션 맵을 설정해준다.
	void	ApplyProjectionMap(NiAVObject	*pkObject);

	//! Kinemetic피직스 물체에서 연결 되어 있는 Ni물체를 찾는다. && Group을 설정한다.
	bool FindObjFromPhysXSrc(NiPhysXSrc *pkPhysXSrc, NiAVObject **pout_SrcObj, NxActor **pout_Actor, int iSetShapeGroup = -1);

	//! 트리거를 설정합니다.
	bool LoadTriggers();

	//! 샤이닝스톤을 설정합니다.
	bool LoadShineStones();

	//! 최상위 Scene을 반환한다.
	NiNode* GetSceneRoot()const;

	//! 사다리들의 루트 노드를 반환한다.
	NiNode *GetLadderRoot()const;

	//! 탈 수 있는 줄의 누트 노드를 반환한다.
	NiNode *GetRopeRoot()const;

	//! PhysX루트 노드를 반환한다.
	NiNode *GetPhysXRoot()const;

	NiNode *GetCameraWallsRoot()const;

	NiNode*	GetLightRoot()const;

	NiNode* GetSpawnRoot()const;

	//! 스폰 위치를 반환한다.
	bool FindSpawnLoc(char const *pcSpawnName, NiPoint3 &rkSpawn_out);

	//! 트리거 위치를 반환한다.
	bool FindTriggerLoc(char const *pcTriggerName, NiPoint3 &rkTrigger_out);

	//! 계절을 바꾼다.
//	bool ChangeSeason(char const *pcSeasonName);

	//! 분위기를 바꾼다. (밤/낮 전환)
//	bool TransitMood(char const *pcMoodName);

	//! 트리거를 돌려 준다. 나중에 빼자.xxxxxxxxxxxxxxxxxxxx
	PgTrigger * GetTriggerByIndex(int const iIndex)const;
	PgTrigger * GetTriggerByID(std::string const& strTriggerID )const;
	void PgWorld::GetTriggerByType(int const Type, CONT_TRIGGER_INFO & ContTrigger)const;
	PgTrigger * GetTriggerByIDWithIgnoreCase(char const * kTriggerID)const;
	
	//! xxxxxxxx 임시, 마우스 좌표에 있는 World Object를 반환한다.
	PgIWorldObject* PickObject(PgPickObjectNullChecker& Tester);

	PgIWorldObject* PickObject(ObjectGroupType iObjectGroupType = OGT_NONE);
	PgIWorldObject* PickObjectByKeyboard();

	//! 전체 광의 AffectedList에 등록한다.
	void LightObjectRecurse(NiAVObject* pkLightTree, NiNode* pkToBeLit,LIGHT_TYPE kLightType = (LIGHT_TYPE)((LT_DIRECTIONAL | LT_AMBIENT)));
	//!	전체 광의 AffectedList에서 제거한다.
	void RemoveLightObjectRecurse(NiAVObject* pkLightTree, NiNode* pkToBeLit,LIGHT_TYPE kLightType = (LIGHT_TYPE)((LT_DIRECTIONAL | LT_AMBIENT)));

	//!	최적화 노드를 설정한다.
	void SetOptimizationLevel(int iLevel);

	//!	LowPoly를 보여줄지 설정한다.
	void SetActiveLowPolygon(bool bActive, int iGlobalLOD);
	void SetWorldLODData(float first, float second, float third);

	NiPoint3	ThrowRay(NiPoint3 const& kStart,NiPoint3 const& kDir,float const fDistance, int const iShapeType=1, unsigned int const uiGroup=-1)const;	//	레이를 쏴서 충돌지점을 구한다.

	//
	void	AddItemEquipInfo(BM::GUID const &kUnitGUID,int iItemNo,bool bSetToDefaultItem,PgItemEx *pkEquipItem);

	//
	void	InitSmallAreaInfo(BM::Stream *pkPacket);
	void	UpdateSmallAreaIndex(NiPoint3 const &kPosition);

	//! 앞으로 fRange 내에서 제일 가까운 World Object를 반환한다.
	PgIWorldObject *RayCheckObject(NiPoint3 const &kStart, int iActionNo,NiPoint3 const &kDir, unsigned int uiObjectGroup, float fRange, PgIWorldObject *pkSelf);
	void RayCheckObjectWidely(PgWOPriorityQueue &kOut,int iActionNo,NiPoint3 const &vStart,NiPoint3 const &vDir, unsigned int uiGroup, float fRange,float fHorizRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication = true,PgActor *pkCasterActor = NULL);
	void RayCheckObjectFloorSphere(PgWOPriorityQueue &kOut,int iActionNo,NiPoint3 const &vStart,NiPoint3 const &vDir, unsigned int uiGroup, float fRadius,EAttackedUnitPos kTargetPosType,bool bNoDuplication = true,PgActor *pkCasterActor = NULL);
	void RayCheckObjectRange(PgWOPriorityQueue &kOut,int iActionNo,NiPoint3 const &vStart, unsigned int uiGroup, float fRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication = true,PgActor *pkCasterActor = NULL);
	void RayCheckObjectBar(PgWOPriorityQueue &kOut,int iActionNo,NiPoint3 const &vStart, NiPoint3 const &vDir,unsigned int uiGroup, float fEdgeLength,float fRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication = true,PgActor *pkCasterActor = NULL);
	int GetMiniMapHeight() const {return m_kDrawHeight.y;}

	int	overlapSphereShapes(
		const NxSphere& worldSphere, 
		NxShapesType shapeType, 
		NxU32 nbShapes, 
		NxShape** shapes, 
		NxUserEntityReport<NxShape*>* callback, 
		NxU32 activeGroups=0xffffffff, 
		const NxGroupsMask* groupsMask=NULL, 
		bool accurateCollision=false);
	NxU32 overlapCapsuleShapes(const NxCapsule& worldCapsule, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false);
	NxU32 overlapOBBShapes(const NxBox& worldBox, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false);

	NxShape* raycastClosestShape(const NxRay& worldRay, const NxShapesType shapeType, NxRaycastHit& hit, const NxU32 groups=0xffffffff, const NxReal maxDist=NX_MAX_F32, const NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL)const;
	NxU32  raycastAllShapes(const NxRay &worldRay, NxUserRaycastReport &report, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=NULL) const;

	void	LockPhysX(bool bLock);

	NiPoint3	FindActorFloorPos(NiPoint3 const &kStart,const NxU32 kGroup=1);	//	액터 놓을 위치를 계산한다.(떨어지지 않게)

	//! 해당 트리거의 피직스를 켜거너 끈다
	void SetTriggerPhysX(std::string const& kID, bool bOn);

	//! 오브젝트를 냉동시킨다.(소환 해제)
	void FreezeObject(BM::GUID &rkGuid);

	//! 오브젝트를 해동시킨다. (소환)
	void UnfreezeObject(BM::GUID const &rkGuid);

	//! 오브젝트의 애니메이션 관련.
	static void SetTimeToAniObj(NiObjectNET* pkObj, float &rfTime, bool bRandomTime = false);
	//static void IncTimeToAniObj(NiObjectNET* pkObj, float fTime);
	static void IncTimeToAniObj(NiObjectNET* pkObj, unsigned __int64 iMilliSec);
	static void SetAniType(NiObjectNET* pkObj, NiTimeController::AnimType eType);
	static void SetAniCycleType(NiObjectNET* pkObj, NiTimeController::CycleType eType);
	static void SetAniStop(NiObjectNET* pkObj);

	//!	보스의 GUID 를 반환
	const	BM::GUID&	GetBossGUID();
	void	SetBossGUID(BM::GUID const &GUID);

	//!	몬스터 타겟 설정
	void	SetMonsterTarget(BM::GUID const &MonGUID,BM::GUID const &TargetGUID);
	void	GetMonsterTarget(BM::GUID &MonGUID,BM::GUID &TargetGUID);

	//! NPC
	void GetShowNPC(CONT_MINIMAP_ICON_INFO & rkContIconInfo)const;

	//! 미니맵 관련 초기화
	bool	SetWorldDataForMiniMap(std::string strMiniMapName, POINT2 const& WndSize);
	bool	SetWorldDataForAllMiniMap();
	//! 미니맵 카메라 변경
	bool	SetMinimapCamera(const char* kCameraName=DEFAULT_MINIMAPCAMERA);
	void	ClearContMinimapCamera();

	//!	월드 업데이트 스피드 조절
	void	SetUpdateSpeed(float fSpeed);
	float	GetUpdateSpeed();
	void	SetSlowMotion(SlowMotionType const eType, float const fFrom, float const fTo, float const fTime);
	bool	UpdateSlowMotion(float const fDeltaTime);

	float	GetAccumTime()	{	return	m_fAccumTime;	}
	float	GetFrameTime()	{	return	m_fFrameTime;	}

	//	kUnitType 에 해당하는 모든 유닛을 kActionName 의 액션으로 ReserveTransitAction 시킨다.
	void	ReserveTransitActionToActors(char const *kActionName,EUnitType kUnitType);

//	bool AddPetToWorld(BM::GUID const &rkGuid, BM::GUID const &rkMasterGuid, NiPoint3 const &rkPos, int iID);

	PgWorldActionMan*	GetWorldActionMan()	{	return	m_pWorldActionMan;	}

	bool	FindPathNormal(NiPoint3 const &kPosition,NiPoint3 &kResult);

	void ResetFocusFilter();
	//! 화면 전체를 덮는 Texture Effect를 보여준다.
	void SetShowWorldFocusFilter(bool bShow, char* pTextureFile, float fAlpha);
	void SetShowWorldFocusFilter(char* pTextureFile, float fAlphaStart, float fAlphaEnd, float fTime, bool bAutoTurnOff = true, bool const bRenderObject = true, bool const bAlphaEndIsMaxValueRenderObject = false);
	void SetShowWorldFocusFilter(DWORD color, float fAlphaStart, float fAlphaEnd, float fTime, bool bAutoTurnOff = true, bool const bRenderObject = true, bool const bAlphaEndIsMaxValueRenderObject = false);
	void SetShowWorldFocusFilter(char* pTextureFile1, char* pTextureFile2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval);
	void SetShowWorldFocusFilter(DWORD color1, DWORD color2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval);
	//!	현재 포커스 필터의 알파값을 얻어온다.
	float	GetWorldFocusFilterAlpha();
	bool IsEndShowWorldFocusFilter();// 화면 전체를 덮는 Texture Effect가 끝났는가?

	PgCameraMan* GetCameraMan() { return &m_kCameraMan; }
	const CameraAdjustInfo& GetCameraAdjustInfo() { return m_kCameraAdjustInfo; }

	void RemoveAllObject(bool bExceptNPC = false); 	// TODO: 같은맵에서 이동시 NPC를 안지우기 위한 땜빵.
	void RemoveAllMonster();
	void RemoveAllMonster(int const iGrade);
	void RemoveAllObjectExceptMe();

	bool GetContainer(const PgIXmlObject::XmlObjectID eType, ObjectContainer &pkContainer);

	float GetViewDistanceRange(int iIndex) { if (iIndex >= 0 && iIndex < MAX_LOD_RANGE_NUM) return m_afViewDistanceRange[iIndex]; return 0; }
	float GetWorldQualityRange(int iIndex) { if (iIndex >= 0 && iIndex < MAX_LOD_RANGE_NUM) return m_afWorldQualityRange[iIndex]; return 0; }
	float GetWorldObjectLODRange(int iIndex) { if (iIndex >= 0 && iIndex < MAX_LOD_RANGE_NUM) return m_afWorldObjectLODRange[iIndex]; return 0; }

	bool GetCompleteBG(CompleteBGContainer& pkContainer);
	CompleteBGContainer& GetCompleteBG()	{	return m_CompleteBGContainer;	}

	void	UpdateSelected(float fTime,NiCamera *pkCamera,NiNode *pkSelectiveNodeRoot);

	//	GroundAttr
	void SetAttr( const T_GNDATTR kAttr ){	m_kAttribute = kAttr;	}
	T_GNDATTR GetAttr()const	{	return m_kAttribute;	}
	bool IsHaveAttr( const T_GNDATTR kCheckAttr );
	bool IsEqualAttr( const T_GNDATTR kCheckAttr ){	return m_kAttribute == kCheckAttr; }

	bool IsMineItemBox(PgGroundItemBox *pDropItemBox);

	unsigned int GetLockBidirection();

	int	GetDifficultyLevel()	const	{	return	m_iDifficultyLevel;	}
	void	SetDifficultyLevel(int iLevel)	{	m_iDifficultyLevel = iLevel;	}

	PgAVObjectAlphaProcessManager* GetAVObjectAlphaProcessManager();
	void CullAlphaProcessUpdate(float fAccumTime,float fFrameTIme);
	void AddAlphaProcessObject(NiNode* pNode, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool bOverride);

	// RandomAni를 사용하지 않는 오브젝트인지 검사한다.
	bool FindDisableRandomAniObject(const NiFixedString &strObjectName);
	// 월드에 붙어 있는 오브젝트를 이름으로 찾아 제거 한다.
	bool DetachObjectByName(char const* szObjectName);

	void ShineStoneCountContReset();
	void CountShineStone(DWORD stoneID);
	int	 GetShineStoneCount(DWORD stoneID);

	bool UpdatePortalObject( std::string const &kTriggerID, SEmporiaGuildInfo const &kGuildInfo );

	// home
	PgHome* GetHome() { return m_pkHome; }
	PgHome* CreateHome();
	void InitializeHome();
	void ReleaseHome();

	NiNode* GetDynamicNodeRoot() { return m_pkDynamicNodeRoot; }
	NiNode* GetStaticNodeRoot() { return m_pkStaticNodeRoot; }
	NiNode* GetSelectiveNodeRoot() { return m_pkSelectiveNodeRoot; }
	PgPSRoomGroup*	GetPSRoomGroupRoot()	{	return	m_pkRoomGroupRoot;	}
	
	bool CanCallDungeonExistUI() const { return m_bCallDungeonExistUI; }
private:
	//NiNode*	GetObjectGroup(ObjectGroupType kType);
	NiNodePtr m_aspObjectGroups[OGT_MAX];
	Loki::Mutex m_kObjectGroupsLock;
public:
	template< typename _T_FUNC >
	bool RunObjectGroupFunc(ObjectGroupType kType, _T_FUNC kFunc) // 함수 원형은 WorldObjecGroupUtil::AttachChild, ::DetachChild 참조
	{
		BM::CAutoMutex kLock(m_kObjectGroupsLock);
		if(kType >=OGT_MAX || 0>kType)
		{
			return false;
		}
		return kFunc(m_aspObjectGroups[kType]);
	}

	//! 캐릭터 위에 월드 오브젝트 화살표를 띄운다.
	void SetDirectionArrow( NiAVObject* pkDestObj );

	void DynamicGndAttr(EDynamicGroundAttr const eNew);
	
	void OnWorldEnvironmentStatusChanged(DWORD const dwNewValue,bool bChangeImmediately);

	void AllMonsterUpdateName();
	void AllPlayerUpdateName();
	void AllNpcUpdateQuest();

public:
	PgCameraMan m_kCameraMan;
	CameraAdjustInfo m_kCameraAdjustInfo;
	ShineStoneContainer m_kShineStoneContainer;
	NiActorManagerPtr m_spCameraAM;	
	int				m_iMapDisciptionNo;
	//! 캐릭터 머리위에 대미지같은 숫자 띄워주는 클래스
	PgDamageNumMan	*m_pkDamageNumMan;

	CLASS_DECLARATION_S_NO_SET(EDynamicGroundAttr, DynamicGndAttr);
	CLASS_DECLARATION_S(SGroundOwnerInfo,	GroundOwnerInfo);
	CLASS_DECLARATION_NO_SET(bool, m_bUseMiniMap, UseMiniMap);
	NiCamera* GetMinimapCamera() { return m_spMinimapCamera; }


    NiNode* GetSkyRoot() const { return m_spSkyRoot; };
    PgWater* GetWater() const { return m_pkWater; };
    PgSpotLight* GetSpotLightMgr() const { return m_pkSpotLightMan; };
    bool UseViewDistanceRange() const { return m_bUseViewDistanceRange; };
    bool UseFogEndFar() const { return m_bUseFogEndFar; };
    bool EnableFocusFilter() const { return (0 != m_spFocusFilter); };
	bool IsExistMiniMapData() const { return !m_kMiniMapImage.empty(); }
public:
	bool IsMapMoveCompleteFade() const { return 0 < m_kMapMoveCompleteFade.fFadeTime; }
	void MapMoveCompleteFade();

private:
	SMapMoveCompleteFade m_kMapMoveCompleteFade;

public:
	void SaveHideActor(int const iClassNo, bool const bHide);
	void SaveHideActor(EUnitType const eType, bool const bHide);
	bool IsSaveHideActor(PgActor * pkActor)const;
	void ClearSaveHideActor();

private:
	bool IsSaveHideActor(int const iClassNo)const;
	bool IsSaveHideActor(EUnitType const eType)const;

	typedef std::set<EUnitType> CONT_SAVEHIDE_UNITTYPE;
	SET_INT m_kContSaveHideActor;
	CONT_SAVEHIDE_UNITTYPE m_kContSaveHideByUnitType;

protected:
	PgHome *m_pkHome;

	ObjectContainer			m_kObjectContainer[MAX_OBJ_CONT];
	AVObjectContainer		m_kFrozenContainer;
	RemoveObjectContainer	m_kRemoveObjectContainer;
	UnfreezeObjectContainer m_kUnfreezeObjectList;
	TriggerContainer		m_kTriggerContainer;
	InputContainer			m_kInputContainer;
	PuppetContainer			m_kPuppetContainer;
	ObjectContainerEx*		m_pkPlayerObjContainer;	//	플레이어 컨테이너
	StringContainer			m_kDisableRandomAniObjNameContainer;	// World에 붙어있는 오브젝트들 중 RandomAni를 사용하지 말아야 하는 오브젝트 이름(시간 동기화도 사용하지 않음)
	ShineStoneCountCont	m_kShineStoneCountContainer;	// 맵의 샤인스톤 개수
	// BgSound
	BgSoundContainer		m_kBgSoundContainer;

	NiNodePtr m_spSpawnRoot;
	NiNodePtr m_spPermissionRoot;
	NiNodePtr m_spPathRoot;
	NiNodePtr m_spSkyRoot;
	NiNodePtr m_spPhysRoot;
	NiNodePtr m_spTriggerRoot;
	NiNodePtr m_spShineStoneRoot;
	NiNodePtr m_spLadderRoot;
	NiNodePtr m_spRopeRoot;
	NiNodePtr m_spLightRoot;
	NiNodePtr m_spCameraWalls;
	NiNodePtr m_spOptimization_0;
	NiNodePtr m_spOptimization_1;
	NiNodePtr m_spOptimization_2;
	NiNode* m_pkDynamicNodeRoot;
	NiNode* m_pkStaticNodeRoot;
	NiNode* m_pkSelectiveNodeRoot;
	PgPSRoomGroup	*m_pkRoomGroupRoot;
    NewWare::Scene::DomainNodeManager* m_pkDomainNodeManager;

	NiCameraPtr m_spMinimapCamera;
	std::string m_kMiniMapImage;
	CONT_CAMERA m_kContMinimapCamera;

	MapObjectMap	m_kMapObjectCont;

	PgOctree	*m_pOctree;
	DWORD	m_dwMainThreadID;

	//!	물 이펙트 처리 클래스
	PgWater	*m_pkWater;

	AVObjectMap	m_kWorldParticle;
	IntList	m_kWorldParticleDisposedIDList;
	int	m_iWorldParticleLastID;

	//!	월드 액션 관리자
	PgWorldActionMan	*m_pWorldActionMan;

	//!	보스의 GUID(보스전 월드일 경우 서버로부터 보스의 GUID를 받아 셋팅된다)
	BM::GUID	m_BossGUID;
	//! 몬스터 타겟 GUID
	BM::GUID	m_Attacker_Monster_GUID;
	BM::GUID	m_Monster_Target_GUID;

	NiPoint3 m_kRayStart;
	NiPoint3 m_kRayEnd;
	POINT m_kDrawHeight;

	NiPick m_kPick;
	
	std::string m_kScript;			// 월드 로딩하면 DoFile을 하는 스크립트 파일 이름.
	std::string m_kUpdateScript;	// 월드 업데이트 시마다 불리는 함수.
	std::string m_kPostScript;		// 월드가 로딩 되고 난 후
	std::string m_kStartScript;		// 인던등의 시작 시점에
	std::string m_kReadyScript;		// 서버에서 MAP_LOADED가 왔을 때
	std::string m_kBossDie;			// 보스가 죽었을 때
	std::string m_kAddScript;		// 월드가 RenderMan에 들어갈 때. 아래 3 이벤트는 RenderMan이 호출한다.
	std::string m_kRemoveScript;	// 월드가 RenderMan에서 지워질 때.
	std::string m_kTerminateScript;	// 월드가 삭제될 때

	PgSpotLight	*m_pkSpotLightMan;

	FocusFilterInfo m_kFocusFilterInfo;
	//NiScreenElementsPtr m_spFocusFilter;
	NiScreenTexturePtr m_spFocusFilter;
	//float m_fFocusFilterTimer;
	//bool m_bFocusFilterDelete;
	//float m_fTargetAlpha;
	//float m_fStartAlpha;
	//float m_fFocusFilterTimerStartTime;

	// Fog
	NiFogProperty *m_pkFogProperty;
    NiColor m_kFogColor;
    float m_fFogAlpha;
	bool m_bUseFogAtSkybox;
	bool m_bUseFogEndFar;
	float	m_fOriginalFogStart,m_fOriginalFogEnd,m_fOriginalFogDensity;

	// LOD
	float m_afWorldObjectLODRange[MAX_LOD_RANGE_NUM];
	float m_afViewDistanceRange[MAX_LOD_RANGE_NUM];
	float m_afWorldQualityRange[MAX_LOD_RANGE_NUM];	
	bool m_bUseWorldObjectLOD;
	bool m_bUseViewDistanceRange;
	bool m_bUseWorldQualityRange;

	OptimizationPolyCont m_kOptPolyCont;
	NodeContainer m_kOptimizeMidCont;
	NodeContainer m_kOptimizeLowCont;

	//	Projection	
	bool	m_bUseProjectionEffect;
	NiTextureEffectPtr	m_spProjectionEffect;
	NiRenderedTexturePtr	m_spProjectionTargetMap;
	NiRenderTargetGroupPtr	m_spProjectionTargetGroup;

	// Background Color
	NiColor m_kBGColor;

	// Prev. Draw-Mode
	NewWare::Renderer::DrawBound::DrawMode m_ePrevDrawMode;

	// PgAVObjectAlphaProcessManager
	typedef std::map<NiNode*, int>	CullContainter;	
	PgAVObjectAlphaProcessManager*	m_pkAVObjectAlphaPM;
	CullContainter					m_kCullContainter;

	typedef std::list<NiNode*>		CullNodeContainer;
	CullNodeContainer				m_kAppCulledToTrueMainTainList;
	CullNodeContainer				m_kAppCulledToFalseMainTainList;

	//	World	Accum	Time
	float	m_fAccumTime,m_fFrameTime;
	float	m_fUpdateSpeed;
	float	m_fLastUpdateTime;
	float	m_fSavedUpdateTime;

	// Random Ani Use
	bool	m_bUseRandomAni;

	//	Update Selected
	int	m_iUpdateSelectiveLODCounter[MAX_LOD_RANGE_NUM];

	NiExtraDataPtr	m_spExtraData;

	CLASS_DECLARATION_S(bool, IsActivateWorldMap);
	CLASS_DECLARATION_S(bool, IsUpdate);

	ContEffectRemainTime			m_kContEffectRemainTime;	//화면에 표시될 이펙트 남은시간
	DWORD							m_dwSaveBlinkTime;			//이펙트 남은시간이 다되갈때 화면이 빨간색으로 깜빡이는 걸 위해 시간을 저장할 변수
	bool	m_bCallDungeonExistUI;
protected:
	typedef std::map<NiGeometry*, std::list<NiGeometry*>> GeometryMatchContainter;

	GeometryMatchContainter m_kGeomContainer;
	unsigned int m_eUnlockBidirection;

//	GroundAttribute
	T_GNDATTR	m_kAttribute;

#ifndef EXTERNAL_RELEASE
	int m_iDraw; // 0 아무것도 안그리기, 1 update만, 2 draw만 3 update/draw모두
	float m_fObjectUpdateTime;
#endif

public:	
	void	AddSpotLightCaster(NiAVObject* pkCaster);
	void	RemoveSpotLightCaster(NiAVObject* pkCaster);
	void	TurnOnSpotLight(bool const bTurnOn);
	void	SetSpotLightBGColor(int const Red,int const Blue,int const Green);
	bool	GetSpotLightOn();
	bool	IsForceSpotlight() const { return m_bForceSpot; };

	void	SetBgColor(NiColor const kColor);

	void	SetDrawMode( NewWare::Renderer::DrawBound::DrawMode const eDrawMode );
	void	RestoreDrawMode();

	// 트리거 헨들러
protected:
	class PgPhysXTriggerReporter : public NxUserTriggerReport
	{
	private:
		void onTrigger(NxShape &rkTriggerShape, NxShape &rkOtherShape, NxTriggerFlag kFlag);
		bool CamTrgProcedure(PgTrigger* pkTrigger, PgActor* pkActor);
		//bool CompareCamTrgName(std::string const& kOldTrgName, std::string const& kNewTrgName);
	};
	// 카메라 트리거가 정상값이 아니면 메세지를 보냄
	void CheckCamTrg();
	//bool CheckCamTrgMark(PgTrigger* pkTrg);
	std::string CheckCamTrgType(PgTrigger* pkTrg);
	bool CheckCamTrgCondition(PgTrigger* pkTrg);

	// 충돌 콜백커
	class PgPhysXContactReporter : public NxUserContactReport
	{
		void onContactNotify(NxContactPair &rkPair, NxU32 kEvent);
	};

	void	InitSpotLightMan();
	void	TerminateSpotLightMan();

	void removePilot(ObjectContainer::iterator iter, bool bCheckWorld = true);
	void processRemoveObjectList(unsigned int iProcessCount = 0);
	void processUnfreezeObjectList(unsigned int iProcessCount = 0);
	void processAddUnitQueue(unsigned int iProcessCount = 0);
	PgPilot* addNpcToWorld(unsigned int iNpcNo, unsigned iNpcKID, BM::GUID const &rkGuid,
		char const* pcActorName, char const* pcScriptName, NiPoint3 const& rkPos, NiPoint3 const& rkDir,
		int iType, bool bHidden, int GiveEffectNo, bool bHideMiniMap = false );
	void optimizeWorldNode();

    NewWare::Scene::AssetManager m_kSharedAssets;
	
    void cullObjectByRange(NiCamera* pkCamera, NodeContainer& rkContainer, float fRange);
	void	UpdateVisibleObjectList();
	void	AddVisibleObjectListToVisibleArray(NiVisibleArray &kArray);
	void	AddVisibleObjectListToVisibleArray(NiVisibleArray &kArray,NiAVObject *pkObject);

	void	UpdateItemEquipInfo();
	void	ClearAllItemEquipInfo();

	void	UpdateFocusFilter(float fAccumTime,float fFrameTime);
	void	UpdateFocusFilterAlphaTransition(float fElapsedTime);
	void	UpdateFocusFilterTextureSwap(float fElapsedTime);
	void	ResetFocusFilterIfTimeIsOver(float fElapsedTime);
	void	SetFocusFilterAlpha(float fAlpha);
	float	CalcFocusFilterAlpha(float fElapsedTime)	const;
	float	CalcFocusFilterElapsedTime()	const;
	bool	GetFocusFilterActivated()	const;

	//! XML 'WORLD' 노드를 파싱한다.
	virtual TiXmlNode const* ParseWorldXml(TiXmlNode const *pkNode, void *pArg = 0);
	
	bool	ApplyFogElement(PgWorldElementFog const &kFogElement);

	// 카메라 트리거가 정상적인지 체크
	void CheckCameraTrigger();

	//! 디버그용 키입력를 처리하는 함수
	void processTemporaryInput();

	//! AddUnit Thread관련 처리
	typedef struct stAddUnitInfo
	{
		stAddUnitInfo() { pkUnit = NULL; pkPilot = NULL; fPacketReceiveTime=0;}
		CUnit* pkUnit;
		PgPilot* pkPilot;
		float fPacketReceiveTime;
	} AddUnitInfo;

	typedef std::list<AddUnitInfo> AddUnitQueue;

	//! GSA 내부에서 BaseObject를 로드한다.
	bool LoadBaseObject(NiScene* pkScene);

	//! GSA 내부에서 로드한 오브젝트를 특별한 노드에 넣는다.
	HRESULT AttachChildNode(NiEntityInterface const* pkEntity, NiNode* pkSceneRootNode, NiNode* pkChildNode,PhysXSceneVec const &kPhysXSceneCont);

public:
	// PhysX 환경을 꾸민다.
	bool LoadPhysX(bool bCreatedSnapshot = false);

	// 최상위 물리 Scene를 반환한다.
	NiPhysXScene *GetPhysXScene();

	// Actor 그룹 번호를 반환한다.
	static NxActorGroup GetPhysXTerrainGroup();
	static NxActorGroup GetPhysXTriggerGroup();
	static NxActorGroup GetPhysXActorGroup();

	static	void	SetPhysXGroup(NiPhysXScene *pkScene,int iPhysXGroup);

	//! 현재의 맵 넘버.
	CLASS_DECLARATION_S(int, MapNo);

public:
	// PhysX 관련
	NiPhysXScenePtr m_spPhysXScene;
	
	static NxActorGroup ms_kPhysXTerrainGroup;
	static NxActorGroup ms_kPhysXTriggerGroup;
	static NxActorGroup ms_kPhysXActorGroup;

	PgPhysXTriggerReporter m_kPhysXTriggerHandler;
	PgPhysXContactReporter m_kPhysXContactHandler;

	float m_fGravity;

	// GSA맵인지 NIF맵인지
	bool m_bLoadGsa;

	void UseFog(bool bUse, bool bUseAdjust);
	NiFogProperty* GetFogProperty() { return m_pkFogProperty; }
	static void DeleteAddUnitInfo(CUnit* pkUnit, PgPilot* pkPilot);
	void EnqueueAddUnitInfo(CUnit* pkUnit, PgPilot* pkPilot,float fPacketReceiveTime);
	void SetShowActorInfo(bool bShow) { m_bShowActorInfo = bShow; }
	void SetShowWorldInfo(bool bShow) { m_bShowWorldInfo = bShow; }

	int		AttachParticle(NiAVObject *pkParticle,NiPoint3 const &kPos,float const fAliveTime=0.f);
	void	DetachParticle(int iSlotID);
	void	DetachAllParticle();
	void	ProcessParticle(float fFrameTime);

	void	AttachSound(char const *pcSoundID, NiPoint3& kPos, float fVolume, float fMin, float fMax);
	void	DetachSound(char const* pcSoundID);

	void	ApplyFogOption();
	void	ApplyTextureResOption();
	void	ApplyTextureFilteringOption();
	void	ApplyWorldQualityOption();

	void	SetUseFogAdjust(bool bUse)	{	m_bUseFogAdjust = bUse;	}
	void	SetUseLODAdjust(bool bUse)	{	m_bUseLODAdjust = bUse;	}
	void	SetUseCameraAdjust(bool bUse)	{	m_bUseCameraAdjust = bUse;	}

	bool	GetUseFogAdjust()	{	return	m_bUseFogAdjust;	}
	bool	GetUseLODAdjust()	{	return	m_bUseLODAdjust;	}
	bool	GetUseCameraAdjust()	{	return	m_bUseCameraAdjust;	}

	PgMultiThreadSceneGraphUpdaterPtr	GetMultiThreadSceneGraphUpdater()	{	return	m_spMultiThreadSceneGraphUpdater;	}

    void	AdjustCameraAspectRatio(NiCamera *pkCamera);

	bool	IsNotCulling()const;

private:

	bool	GetGsaPath(TiXmlNode const *pkNode,std::string &kGsaPath);
	void	CreateEnvironmentSet();

	void	ApplyProjectionDecalTextureCoordinate(NiAVObject *pkAVObject,NiBound const &kBound,NiTexture *pkDecalTexture,NiCamera const &kCamera);
	void	UpdateProjectionDecal();
	void	ApplyProjectionDecal();

	//! for graphic support
	bool m_bUseCameraAdjust;
	bool m_bUseFogAdjust;
	bool m_bUseLODAdjust;
	DWORD m_kKeyPressedTIme[9]; //! fog adjust용
	AddUnitQueue m_kAddUnitQueue;
	Loki::Mutex m_kAddUnitQueueLock;
	bool m_bShowActorInfo;
	bool m_bShowWorldInfo;
	int m_iTriggerIndexOrder;

	//! DrawOption
	bool m_bDrawBG;
	bool m_bDrawWorldText;
    public:
    bool GetDrawBG();
    bool GetDrawWorldText();
    private:
	void SetDrawBG(bool const bDrawBG);
	void SetDrawWorldText(bool const bDrawWorldText);


	//! 강제스팟라이트 여부
	bool m_bForceSpot;

	int	m_iDifficultyLevel;	//	맵 난이도

	CompleteBGContainer m_CompleteBGContainer;	// 미션

	int	m_iEnvironmentStateSetID;

	bool LightHillLamp( size_t const iIndex=UINT_MAX );
	ContPvPLamp			m_kContHillLamp;		// PvP

	CONT_BUILDING			m_kContBuilding;
	CONT_EMPORIA_GATE_INFO	m_kContEmporiaGateInfo;
	PgSmallAreaInfo	m_kSmallAreaInfo;
	int	m_iSmallAreaIndex;

	NiRenderedTexturePtr	m_spRenderedTexture;
	NiRenderTargetGroupPtr	m_spRenderTargetGroup;
	NiCamera	m_kProjectionDecalCamera;

	PgMultiThreadSceneGraphUpdaterPtr	m_spMultiThreadSceneGraphUpdater;

	bool m_bNotCulling;

public:
    const ContActorFilter& GetDrawActorFilter() const { return m_kDrawActorFilter; };
protected:
	ContActorFilter m_kDrawActorFilter;//그릴 Actor Guid
	ScalePhysXCont m_kScalePhysXCont;		// To Share PhysX Desc cont

public:
	bool BuildingControl( CONT_BUILDING::key_type nBuildNo, SEmporiaFunction const &kFunc, bool const bBuild, bool const bMessage=false );

	void AddDrawActorFilter(BM::GUID const &rkActorGuid);
	bool IsDrawActor(BM::GUID const &rkActorGuid) const;
	void ClearDrawActorFilter();


	typedef struct tagHideObject
	{
		bool bAlphaEnable;
		float fAlphaValue;
	}SHideObject;
	typedef std::map<NiAVObject*, SHideObject> HideObjectContainer;
	HideObjectContainer m_kHideObjectContainer;// 숨기는 오브젝트들
protected:
	void CreatePhysXScene();
	void createPhysXObject(NiEntityInterface const *pkEntity, NiStream const &kUniqueStream);
	void CreateDefaultNode(NodeContainer &kNodeCont,bool bFindBase);
	void lightSetting(bool bFindBase, NodeContainer &kMapObjForLight, NiEntityInterfaceContainer &kMapLight);
	void loadNifObject(NiEntityInterface const *pkEntity, NiNodePtr const &spNifRoot, bool bUseLOD, NiStream const &kUniqueStream, NodeContainer &kNodeCont, NodeContainer &kMapObjForLight, bool bLoadGsa2);
	void loadOtherEntities(NiEntityInterface *pkEntity, NiEntityInterfaceContainer &kMapLight);

	bool	LoadEnvSoundEntity(NiEntityInterface *pkEntity);
	bool	LoadActorEntity(NiEntityInterface *pkEntity,NodeContainer &kMapObjForLight);


public:
	void AddHideObject(NiEntityComponentInterface const * _pkCompo, NiAVObject* _pkHideObject);
	bool IsHideObject(NiAVObject* _pkParentNode, float& _fAlphaValue);
	void ClearHideObject();

	void SetDetachGlowMap(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject);//!/ 맵로딩시 해당 엔터티의 글로우맵을 떼어낸다.
	void SetUseUpdateLodForLoadTime(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject);//!/ 맵로딩시 USE_UPDATE_LOD 를 안쓰게 셋팅한다.
	void ApplyComponentOcclusionCulling(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject);//!/ 오쿨루젼 컬링 그룹을 오브젝트 엑스트라 데이타에 기록한다.
	void ApplyComponentHiddenPortal(NiEntityComponentInterface const * _pkCompo, NiAVObject *_pkObject);

	void RecursiveDetachGlowMapByGndAttr(NiAVObject *pkObject);

	static TBL_DEF_MAP const* GetMapData(int const nMapNo);


	HideObjectContainer	const	&GetHideObjectCont()	const	{	return	m_kHideObjectContainer;	}

	//typedef std::map<std::string, NiActorManager*> ActorManagerContainer;
	//ActorManagerContainer m_kOriginalKfmObject;
	//ActorManagerContainer m_kSettedKfmObject;

	//NiActorManager *GetOriginalKfmObject(std::string strPath);
	//NiActorManager *GetSettedKfmObject(std::string strName, std::string strPath = "");
	//void MapKfmObjectClear();
	//void UpdateMapKfmObject(float fTime);
	//void DrawMapKfmObject(NiCamera* pkCamera);

	void Recv_PT_M_C_UNIT_POS_CHANGE( BM::Stream &kPacket );
	void AddEffectRemainTime(BM::GUID const& rkTargetGuid, int const iNum, unsigned long const ulEndTime, CEffectDef const* pkEffectDef = NULL);
	void UpdateEffectRemainTime();
	void DeleteEffectRemainTime(BM::GUID const& rkGuid);

	bool IsHiddenNpcEvent(BM::GUID const& rkNpcGuid, CUnit * pkUnit)const;
	void ProcessNpcEvent();

public:
    void DrawEmporiaGate( PgRenderer* pkRenderer, float fFrameTime );
    void DrawDebugInfo( PgRenderer* pkRenderer, float fFrameTime );
private:
	PgLinearInterpolation kSlowMotionInter;

private:
    //NewWare::Scene::SceneSpacePartition m_kSceneSpacePartition;
	
	class PgTimeEventLoader
	{
	public:
		PgTimeEventLoader();
		std::string const& GetID() const;
		void ReadFromXmlElement(TiXmlElement const *Element);
		void ClearID()	{m_ID.clear();}
	private:
		std::string FindID(TiXmlElement const *Element) const;
		void SetID(std::string const& ID);
		PgWorldTimeCondition	m_TimeCondition;
		std::string m_ID;
	};

	PgTimeEventLoader m_TimeEventLoader;
//------------MouseOver Actor --------------//
public:
	PgIWorldObject* const GetOldMouseOverObject() const { return m_pkOldMouseOverObject; }
	void SetOldMouseOverObject(PgIWorldObject *pkObject);
private:
	PgIWorldObject *m_pkOldMouseOverObject;

public:
	typedef struct tagEventScriptOnEnter
	{
		tagEventScriptOnEnter()
		{
			m_iEventScriptIDOnEnter = 0;
			m_iEventScriptIDOnEnterByPartyBreakIn = 0;
			m_iLimitLevelMin = 0;
			m_iLimitLevelMax = 0;
			m_bPlayOnce = false;
		}

		int	m_iEventScriptIDOnEnter;
		int	m_iEventScriptIDOnEnterByPartyBreakIn;
		std::list<int> m_kContLimit_Quest;
		std::list<int> m_kContLimit_Class;
		int m_iLimitLevelMin;
		int m_iLimitLevelMax;
		bool m_bPlayOnce;
	}EventScriptOnEnter;
	EventScriptOnEnter m_kEventScriptOnEnter;
//-------------------Remote Object---------------//
public:
	void SetCurrentRemoteObject(SREMOTE_OBJECT const kRemoteObject);
	bool IsHaveCurrentRemoteObject(std::string const kObjectName);
	bool IsHaveCurrentRemoteObject();
	void UpdateCurrentRemoteObjcet();
	bool IsPlayRemoteObject( std::string const kObjectName );
	void PlayRemoteObject( std::string const kObjectName, std::string const &rkAniName );
private:
	CONT_VEC_CURRENT_REMOTE_OBJECT m_kContCurrentObject;	//현재 플레이 되는 오브젝트 상태 컨테이너

//------------------Progress Map------------------
public:
	void InitDungeonProgressPosFromPartyMaster(PgPilot* pkPilot);
};

extern	PgWorld	*g_pkWorld;
extern	ACE_RW_Thread_Mutex g_kWorldLock;

class PgCharacterLevel
{
public:
	PgCharacterLevel(){}
	~PgCharacterLevel(){}

protected:
	int m_nCharacterLevel;// 맵 로딩시 캐릭터 정보가 없을때 캐릭터 레벨을 알 필요가 있어서 생성

public:
	void SetCharacterLevel();
	void SetCharacterLevel(int nLevel);
	int GetCharacterLevel(){ return m_nCharacterLevel; }


};

#define g_kChaLevel SINGLETON_CUSTOM(PgCharacterLevel, CreateUsingNiNew)
//////////////////////////////////////////////////////////////////////////////////////////
//

typedef struct tagSayItem
{
	int iActionType;
	int iTTW;
	float fUpTime;
	float fDelay;
	std::string kActionName;
	std::string kSoundID;
	std::string kFaceID;
	int iFaceAniID;
	int iMinLevel;
	int iMaxLevel;
	ContQuestID kCompleteQuest;
	ContQuestID kNotCompleteQuest;
	POINT3 kCamPos;

	tagSayItem();
	bool ParseXml(TiXmlElement const *pkSayNode);
	void Clear();
	bool operator == (const tagSayItem &rkRight) const;
} SSayItem;
typedef std::vector< SSayItem > ContSayItem;

typedef struct tagWorldNpc
{
	tagWorldNpc()
	{	
		uiNpcNo = 0;
		uiNpcKID = 0;
		kNpcGuid.Clear();
		kActorName.clear();
		kScriptName.clear();
		kPos = NiPoint3( 0, 0, 0 );
		kDir = NiPoint3( 0, 0, 0 );
		iType = E_NPC_TYPE_FIXED;
		bHidden = false;
		bIgnoreCameraCulling = 0;
		iParam = 0;
		uiDesctibID = 0;
		kVec.clear();
		bHideMiniMap = false;
		iMinimapIcon = 0;
		GiveEffectNo = 0;
	}
	
	unsigned int	uiNpcNo;
	unsigned int	uiNpcKID;
	BM::GUID		kNpcGuid;
	std::string		kActorName;
	std::string		kScriptName;
	NiPoint3		kPos;
	NiPoint3		kDir;
	int				iType;
	bool			bHidden;
	bool			bIgnoreCameraCulling;
	int				iParam;
	unsigned int	uiDesctibID;
	ContSayItem		kVec;
	int				iDescription;
	bool			bHideMiniMap;
	int				iMinimapIcon;
	int				GiveEffectNo;
} SWorldNpc;

class PgWorldNpcMgr
{
public:
	typedef std::map< BM::GUID, SWorldNpc > CONT_NPC;
	typedef std::map< std::string, CONT_NPC > CONT_WORLD_NPC;
	typedef std::map< int, CONT_NPC > CONT_WORLD_NPC_BY_MAPNO;

public:
	PgWorldNpcMgr(){ m_kContWorldNpc.clear(); m_kContNpc.clear(); m_kContWorldNpcByMapNo.clear(); }
	~PgWorldNpcMgr(){ m_kContWorldNpc.clear(); m_kContNpc.clear(); m_kContWorldNpcByMapNo.clear(); }

public:
	void LoadFile( const char* pcFilename );
	TiXmlNode const* ParseAllNpcXml( TiXmlNode const *pkNode, std::string& rkOutWorldId, int &rkOutMapNo );
	bool ParseXmlForWorldNpc( TiXmlElement const * pkElement, SWorldNpc& rkOutWorldNpc );
	bool AddNpc( const SWorldNpc& rkWorldNpc );
	bool AddWorldNpc( const std::string& kWorldId, const CONT_NPC& kContNpc, int const iMapNo = 0 );
	const CONT_WORLD_NPC& GetWorldNpc() { return m_kContWorldNpc; }
	CONT_WORLD_NPC_BY_MAPNO const& GetWorldNpcByMapNo() const { return m_kContWorldNpcByMapNo; }

	SWorldNpc FindNpc(BM::GUID const& rkGuid, int const iMapNo = 0) const;

private:
	SWorldNpc FindNpc(CONT_NPC const& rkCont, BM::GUID const& rkGuid) const;
	CONT_WORLD_NPC m_kContWorldNpc;
	CONT_NPC m_kContNpc;
	CONT_WORLD_NPC_BY_MAPNO m_kContWorldNpcByMapNo;
};
#define g_kWorldNpcMgr SINGLETON_STATIC(PgWorldNpcMgr)

class PgPickObjectNullChecker
{
public:
	PgPickObjectNullChecker() {};
	virtual ~PgPickObjectNullChecker() {};
	virtual bool Test(PgIWorldObject* pkObject) = 0;
	virtual float Distance(PgIWorldObject const* pkObject)const { return 0; }
};

//
class PgPickObjectMouseTester : public PgPickObjectNullChecker
{
public:
	PgPickObjectMouseTester(NiCamera& rkCamera, NiPick& rkNewPick);
	virtual ~PgPickObjectMouseTester() {};
	virtual bool Test(PgIWorldObject* pkObject);
	virtual float Distance(PgIWorldObject const* pkObject)const;

protected:
	bool PickTest(PgIWorldObject* pkObject);

private:
	NiPoint3 m_kOrgPt;
	NiPoint3 m_kRayDir;
	NiPick& m_kPick;
};

//
class PgPickObjectMouseTypeChecker : public PgPickObjectMouseTester
{
public:
	PgPickObjectMouseTypeChecker(NiCamera& rkCamera, NiPick& rkNewPick, ObjectGroupType iObjectGroupType = OGT_NONE);
	virtual ~PgPickObjectMouseTypeChecker() {};
	virtual bool Test(PgIWorldObject* pkObject);

private:
	ObjectGroupType m_iObjectGroupType;
};

//
class PgPickObjectSpaceCheker : public PgPickObjectNullChecker
{
public:
	PgPickObjectSpaceCheker(PgIWorldObject& rkPlayer);
	virtual ~PgPickObjectSpaceCheker() {};
	virtual bool Test(PgIWorldObject* pkObject);

private:
	PgIWorldObject& m_kPlayer;
	NiPoint3 m_kPlayerPos;

public:
	PgIWorldObject* m_pkClosestObject;
	float m_fClosestDistance;
};

namespace PgWorldUtil
{
	std::string const kCharacterSelectStateID("w_char_sel");

	inline bool IsCharacterSelectStage()
	{
		if( g_pkWorld )
		{
			return g_pkWorld->GetID() == kCharacterSelectStateID;
		}
		return false;
	}

	bool SetTriggerEnable(PgWorld* pkWorld, std::string const& rkTriggerID, bool const bEnable);
};

//카메라 트리거 관련 상수 문자열
//extern std::string const CAMTRG_IN_MARK;
//extern std::string const CAMTRG_OUT_MARK;
extern std::string const CAMTRG_OUT_TYPE;
extern std::string const CAMTRG_IN_TYPE;
extern std::string const CAMTRG_SINGLE_TYPE;
extern std::string const CAMTRG_TYPE_NAME;
extern std::string const CAMTRG_GROUP;
extern std::string const CAMTRG_EMPTY_MARK;
extern std::string const CAMTRG_CAMERA_ADJUST;
extern std::string const CAMTRG_WIDTH;
extern std::string const CAMTRG_ZOOM;
extern std::string const CAMTRG_UP;
extern std::string const CAMTRG_TARGET_X;
extern std::string const CAMTRG_TARGET_Y;
extern std::string const CAMTRG_TARGET_Z;

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGWORLD_H