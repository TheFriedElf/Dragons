#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMAN_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMAN_H
class PgICameraMode;
class PgActor;
class PgInput;

typedef struct stCameraInfo
{
	stCameraInfo()
	{
		fDistanceFromTarget = 0.0f;
		fCameraHeight = 0.0f;
		fLookAtHeight = 0.0f;
		fLookAtDepth  = 0.0f;
		fLookAtWidth  = 0.0f;
		fCameraWidth  = 0.0f;
	};

	stCameraInfo(float distance, float height, float lookatZ)
	{
		fDistanceFromTarget = distance;
		fCameraHeight = height;
		fLookAtHeight = lookatZ;
		fLookAtDepth  = 0.0f;
		fLookAtWidth  = 0.0f;
		fCameraWidth  = 0.0f;
	}

	// 타겟부터의 거리
	float fDistanceFromTarget;

	// 타겟부터의 높이
	float fCameraHeight;
	float fCameraWidth;
	// 타겟의 어느 위치를 볼 것인가.
	float fLookAtHeight;
	float fLookAtDepth;
	float fLookAtWidth;
} CameraInfo;

typedef struct stCameraAdjustInfo
{
	stCameraAdjustInfo() 
	{
		fMinZoomPos = 0.0f;
		fMaxZoomPos = 0.0f;
		fMinZoomDelta = 0.0f;
		fMaxZoomDelta = 1.0f;
	}
	CameraInfo kCameraAdjust;
	float fMinZoomPos;	// 0은 처리 안함.
	float fMaxZoomPos;	// 0은 처리 안함.
	float fMinZoomDelta;
	float fMaxZoomDelta;
} CameraAdjustInfo;

typedef struct stQuakeCameraInfo
{
	stQuakeCameraInfo()
	{
		fQuakeDuring = 0.0f;
		fQuakeFactor = 0.0f;
		fQuakeTime = 0.0f;
		iCycleCount = 3;
		fFrequency = 2.0f;
		bPlus = true;
		iQuakeType = 1;
	};

	// 전체 시간
	float fQuakeDuring;

	// 진폭
	float fQuakeFactor;

	// 시작 시간
	float fQuakeTime;

	// frequency를 몇번 반복할 것인가?
	int iCycleCount;

	// frequency * PI 만큼의 Frequency를 씀.
	float fFrequency;

	// true면 +, false면 -
	bool bPlus;
	
	// Type 1 : 원래 위치를 중심으로 반지름 r 사이에서 랜덤하게
	// Type 2 : 원래 위치를 중심으로 좌우 r 사이에서 랜덤하게?
	// Type 3 : 원래 위치를 중심으로 위아래 r 사이에서 랜덤하게?
	// Type 3 : 원래 위치를 중심으로 앞뒤로 r 사이에서 랜덤하게?
	int iQuakeType;	
} QuakeCameraInfo;

class PgCameraMan : public NiMemObject
{
	typedef NiTStringPointerMap<NiCameraPtr> Container;

public:
	typedef enum
	{
		CMODE_NONE,
		CMODE_FOLLOW,
		CMODE_MOVIE,
		CMODE_CIRCLE,
		CMODE_SCRIPT,
		CMODE_FREE,
		CMODE_FOLLOW2,
		CMODE_FOLLOW_DEPTH_PATH,		// 캐릭터 따라 깊이 들어가지 않고, 패스벽 기준으로 있는다.
		CMODE_BOX,						// 마그마 드레이크에서 쓰이는 카메라.
		CMODE_EVENT,
		CMODE_ANIMATION,				// Actor/Nif 안에 들어있는 애니메이션이 있는 카메라 모드
		CMODE_STOP,
	} ECameraMode;

public:
	//// Constructor & Destructor
	//
	PgCameraMan(void);
	~PgCameraMan(void);

	//// Getter
	//
	NiCameraPtr GetCamera();
	void SetCamera(NiCameraPtr spCamera);
	void SetCamera(NiString& rkCameraName);

	//// Container
	//
	void AddCamera(NiString& rkCameraName, NiCameraPtr spCamera);
	void DeleteCamera(NiString& rkCameraName);

	//// Update
	//
	void Update(float fFrameTime);

	//	Quake Camera
	void	QuakeCamera(float fDuring, float fFactor);

	//// Camera Mode
	//
	PgICameraMode *GetCameraMode();
	ECameraMode	GetCameraModeE()	{	return	m_eCameraMode;	}
	void SetCameraMode(ECameraMode eCameraMode, PgActor *pkActor = 0, NiActorManagerPtr spAM = 0);
	bool SetCameraTargetAnimation(int const &iNum, char const *pcFuncName = 0);

	NiPoint3	GetVelocity()	{	return	m_vCamVelocity;	}

	static bool ParseXml(char const* pcXmlPath);
	void ResetCameraPos();
	void SaveCameraPos();
	void RestoreCameraPos();
	void RefreshCamera();
	void NotifyActorRemove(PgActor* pkActor);

	CLASS_DECLARATION_S(bool, LockInput);
	void ClearQuake();

	bool Input(PgInput *pkInput);

	static float GetJumpHeight() { return m_fJumpHeight; } 
	static CameraInfo Lerp(CameraInfo &rkPosA, CameraInfo &rkPosB, float fPerc);

protected:
	NiCameraPtr m_spCamera;				// Current Camera
	Container m_kContainer;				// Camera Container
	NiPoint3 m_kReservedCameraPosition;
	bool m_bQuakeCamera;

	NiPoint3	m_vCamVelocity;	//	leesg213 카메라의 현재 위치 - 이전 프레임 위치(즉, 이동 속도)

	//// Camera Mode
	PgICameraMode				*m_pkCameraMode;
	ECameraMode					m_eCameraMode;
	NiActorManager::SequenceID	m_PastCameraAniID;
	std::string					m_szNextScript;				//카메라 애니가 끝나고 실행할 스크립트	

	static float m_fJumpHeight;
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMAN_H