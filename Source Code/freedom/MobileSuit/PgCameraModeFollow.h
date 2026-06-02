#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW_H

#include "PgICameraMode.h"
#include "PgCameraMan.h"

#define PG_USE_NEW_CAMERAWALL

#ifdef PG_USE_NEW_CAMERAWALL
typedef struct tagCameraWallInfo
{
	NiPlane* pkPlane;
	NiTriBasedGeom* pkTri;
} CameraWallInfo;
#endif

typedef struct stFollowCameraInfo
{
	stFollowCameraInfo() 
	{
		fZoomDelta = 0.0f;	//! 0.0이 최대로 줌 아웃 된 상태 1.0이 최대로 줌 인된 상태. 1.0이 Default
		fUpDelta = 0.0f;	//! 0.0이 기본 카메라, 1.0이 최대로 올라간 카메라.
	}
	CameraInfo kCurrentPos;
	CameraInfo kCurrentEnd;	// 최대로 줌 아웃 했을때의 위치
	float fZoomDelta;
	float fUpDelta;
} FollowCameraInfo;

typedef struct stCameraAreaInfo
{
	CameraInfo kZoomOutCamera;
	CameraInfo kZoomInCamera;
	CameraInfo kUpCamera;
} CameraAreaInfo;

class PgInput;

class PgCameraModeFollow : public PgICameraMode
{
protected:
	typedef struct tagHiddenObjectProperty
	{
		NiAlphaPropertyPtr pkAlphaProperty;// 기존 알파속성 저장
		//char cRefCount;// 안쓴다, 나중에 삭제
		float fNewAlpha;// 바뀔 알파값 저장
		float fOldAlpha;// 기존 알파값 저장
	} HiddenObjectProperty;

	struct	stObstacleInfo
	{
		NiAlphaPropertyPtr	m_spOriginalAlphaProperty;
		NiMaterialPropertyPtr	m_spOriginalMaterialProperty;

		float	m_fAlphaTransitStartTime;
		float	m_fStartAlpha,m_fTargetAlpha;
		float	m_fAlphaTestRef;
		bool	m_bRemove;

		stObstacleInfo()
			:m_fAlphaTransitStartTime(0),m_fStartAlpha(0),m_fTargetAlpha(0),m_bRemove(false),m_fAlphaTestRef(0)
		{
		};

	};

	typedef std::map<NiAVObject *, HiddenObjectProperty> HiddenObjectContainer;
	typedef std::map<NiAVObject *, stObstacleInfo> ObstacleInfoMap;

	typedef std::list<NiFixedString> HiddenObjectMeterialContainer;

	typedef struct tagHiddenObjectPropertyList
	{
		//bool bRef;
		BYTE byStep;// 1. 하이드 2. 언하이드, 3.복구
		NiAVObject* pParentNode;// 해당 매쉬가 속해있는 오브젝트의 노드
		HiddenObjectContainer kObjectContainer;// 메쉬별로 저장
		HiddenObjectMeterialContainer kMeterialContainer;// 공유 메터리얼 판별에 사용
	} HiddenObjectPropertyList;

	typedef std::map<NiAVObject *, HiddenObjectPropertyList> HiddenObjectContainerList;	

public:
	typedef enum
	{
		MOVETYPE_DEPTH_FOLLOW_ACTOR,
		MOVETYPE_DEPTH_FOLLOW_PATH,
	} EMoveType;

	PgCameraModeFollow(NiCamera *pkCamera, PgActor *pkActor);
	virtual ~PgCameraModeFollow();

	virtual void SetActor(PgActor *pkActor);
	virtual bool Update(float fFrameTime);
	bool Input(PgInput *pkInput);
	void ResetCameraPos();
	void SaveCameraPos();
	void RestoreCameraPos();

	//! Refresh? Attach된 액터앞으로 바로 간다. Damping이고 Interpolation이고 없다. 
	void RefreshCameraPosition();
	NiPoint3 GetActorPosition();

	bool RemoveCameraWall(char const* szName);	//카메라 벽 제거
	bool RecoverCameraWall(char const* szName);	//제거된 벽 복구
	void EnableWallByCharPos(NiPoint3 const &curPos);

	void SetCameraMoveType(EMoveType eType) { m_eMoveType = eType; }
	EMoveType GetCameraMoveType() { return m_eMoveType; }

	void SetCameraFixed(bool bFixed) { m_bFixedCamera = bFixed; }
	bool GetCameraFixed() { return m_bFixedCamera; }
	
	void UpdateMinMaxCameraZoomInfo();
	void SetCameraAdjustInfo(CameraAdjustInfo const& kAdjustInfo) { m_kCameraAdjustInfo = kAdjustInfo; UpdateMinMaxCameraZoomInfo(); }
	void SetCameraAdjustCameraInfo(NiPoint3 const kPosInfo, NiPoint3 const kLookAtInfo);

	CameraAdjustInfo& GetCameraAdjustInfo() { return m_kCameraAdjustInfo; }
	static FollowCameraInfo& GetCameraInfo() { return ms_kFollowCameraInfo; }
	static void SetCameraZoom(float fZoom);
	static float GetCameraZoom();
	static void SetDefaultCameraInfo();
	static void SetDefaultCamera(int iClass);
	void SetCameraZoomMax(float fValue);
	

protected:
	bool UpdateTranslate(float fTime);
	bool UpdateRotate(float fTime);

	void	UpdateObstacle(float fAccumTime,float fFrameTime);

	//void UpdateHideCoveredObjects();// 안쓸예정, 조만간 주석처리
	void UpdateHideCoveredObjects2();// 캐릭터를 가리는 물체에 대한 업데이트 함수
	void UpdateHideObjectAlpha(float _fDeltaTime);// 알파값을 시간에 따라 조정
	NiAVObject* FindHideObjectRoot(NiAVObject* pkObject);// 숨길 오브젝트의 루트를 찾아낸다
	bool IsHideObjectSearchParentNode(NiAVObject* pkParent);// 이미 숨기고 있는건지 부모노드에서 찾는다, 재귀
	void SetHideObject(NiAVObject* _pkObject, float _f, HiddenObjectPropertyList& _kPropertyList);// 알파를 셋팅한다, 재귀
	void UnsetHideObject();// 목록에서 찾아서 알파속성을 복구한다.

	NiPoint3 adjustCameraByWall(NiPoint3 const &curPos);
	NiPoint3 adjustCameraByWall2(NiPoint3 const &curPos);
	NiPoint3 adjustCameraByWall3(NiPoint3 const &rkPos, NiPoint3 const &rkPrevPos);
	void recursiveProcessWalls(NiAVObject* pkObject);
	void releaseAllCamWalls();

	void AddHiddenObjectMeterial(HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName);
	void DeleteHiddenObjectMeterial(HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName);
	bool FindHiddenObjectMeterial(HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName);

protected:
	NiPoint3 m_kCameraPrevTrn;
	NiPoint3 m_kActorPrevTrn;
	float m_fLastUpdateFrame;

	float m_fNormalInterpolateTime;
	NiPoint3 m_kTargetPathNormal;
	NiPoint3 m_kCurrentPathNormal;
	NiPoint3 m_kLastPathNormal;

	float m_fTransInterpolateTime;
	NiPoint3 m_kTargetTrans;
	NiPoint3 m_kCurrentTrans;
	NiPoint3 m_kLastTrans;

	NiPoint3 m_kTargetFloorTrans;
	NiPoint3 m_kTargetFloorTransOld;
	NiPoint3 m_kTargetTransOld;

	float m_fRotateInterpolateTime;
	NiQuaternion m_kTargetRot;
	NiQuaternion m_kCurrentRot;
	NiQuaternion m_kLastRot;

	NiPick kPick;
	NiPick m_kHidePick;
	//HiddenObjectContainer m_kHiddenObjectContainer;// 안쓸예정, 조만간 주석처리
	HiddenObjectContainerList m_kHiddenObjectContainerList;// 캐릭터를 가리는 물체들을 들고 있는 컨테이너

	ObstacleInfoMap	m_kObstacleInfoCont;

	typedef std::vector< NiPlane* > PlaneVector;
	typedef std::map< std::string, NiPlane* > PlaneMap;
#ifdef PG_USE_NEW_CAMERAWALL
	typedef std::map< std::string, CameraWallInfo > PlaneMapContainer;
	PlaneMapContainer m_kPlaneCont;
#endif
	PlaneMap m_kPlanes;
	PlaneMap m_kRemovedPlanes;	//제거된 카메라 벽들
	PlaneMap m_kInactivePlanes;	//숨겨야 하는 벽들	
	
	bool m_bFixedCamera;
	EMoveType m_eMoveType;

	// 맵마다 각각 세팅되는 Min/Max Zoom Pos/Delta.
	float m_fMaxZoomDelta;
	float m_fMinZoomDelta;
	CameraAdjustInfo m_kCameraAdjustInfo;
	float m_fLastCameraZoomDelta;

	friend class PgCameraMan;
	static CameraInfo ms_kDefaultCamera;
	static CameraInfo ms_kClassDefaultCamera[4];

	static FollowCameraInfo ms_kFollowCameraInfo;
	static CameraAreaInfo ms_CameraAreaInfo;
	FollowCameraInfo m_kSavedCameraInfo;
	static CameraAreaInfo ms_SavedCameraAreaInfo;

	NiPoint2	m_ptPrevMousePos;
};

//#ifndef EXTERNAL_RELEASE
extern bool g_bCameraDebug;
//#endif

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW_H