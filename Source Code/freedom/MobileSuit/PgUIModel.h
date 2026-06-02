#ifndef FREEDOM_DRAGONICA_UI_PGUIMODEL_H
#define FREEDOM_DRAGONICA_UI_PGUIMODEL_H

#include "PgUIDrawObject.h"
#include "PgActor.h"
#include "PgActorPet.h"

typedef struct tagUIModelPgActorInfo
{
	bool		m_bUpdate;
	PgActorPtr	m_spkActor;
}UIModelPgActorInfo;

typedef struct tagUIModelPgPetInfo
{
	bool		m_bUpdate;
	PgActorPetPtr m_spkPet;
}UIModelPgPetInfo;

typedef struct tagCameraResetInfo
{
	typedef enum : BYTE
	{
		EHS_NONE = 0,
		EHS_FIX_MODEL_VALUE,
		EHS_USE_RADIAN_VALUE,
	}E_HEIGHT_SETTING;

	tagCameraResetInfo() : fXCenter(0.5f), fYCenter(0.5f), fRadius(0.0f), eHeightSet(EHS_NONE)
	{
	}

	float fXCenter;
	float fYCenter;
	float fRadius;
	E_HEIGHT_SETTING eHeightSet;
}SCameraResetInfo;

class PgRenderer;

class PgUIModel 
	: public PgUIDrawObject
{
public:
	static PgUIModel* Create(POINT2 const& ptWndSize, bool bUpdate = true, bool bOrtho = false);

	PgUIModel(POINT2 const& ptWndSize);
	virtual ~PgUIModel();

	void Clear();
	void Terminate();
	bool Initialize(POINT2 const& ptWndSize, bool bOrtho);

	//! UIModel이 가지고있는 ActorManager와 Nif들을 텍스쳐에 렌더링
	void RenderFrame(NiRenderer* pkRenderer, POINT2 const& pt);
	//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
	bool Update(float fAccumTime, float fFrameTime);
	//! pkRenderer를 이용해서 Draw
	void Draw(PgRenderer *pkRenderer);

	void	InvalidateRenderTarget();

	bool			AddActorByID(std::string const& strActorName, std::string const& strID, bool const bEnableGlowMap = false);

	bool			AddActor(std::string const& strActorName, NiActorManagerPtr spNiActor, bool const bClone = true, bool const bCameraReset = true, bool const bEnableGlowMap = false);
	bool			DeleteActor(std::string const& strActorName);
	NiActorManager*	GetActor(std::string const& strActorName) const ;
	
	bool			AddNIF(std::string const& strNIFName, NiNodePtr pNiNode, bool const bClone = true, bool  const bCameraReset = true, bool const bEnableGlowMap = false);
	bool			DeleteNIF(std::string const& strNIFName);
	NiNode*			GetNIF(std::string const& strNIFName) const ;
	
	void			ResetNIFAnimation(std::string const& strNIFName);

	bool			AddPgActor(std::string const& strActorName, PgActorPtr pkSrcActor, bool const bClone = true, bool  const bCameraReset = true);
	bool			DeletePgActor(std::string const& strActorName);
	PgActor*		GetPgActor(std::string const& strActorName) const;

	bool			AddPgPet(std::string const& strActorName, PgActorPet* pkSrcActor, bool const bClone = true, bool  const bCameraReset = true);
	bool			DeletePgPet(std::string const& strActorName);
	PgActorPet*		GetPgPet(std::string const& strActorName) const;
	
	bool			ChangePetAction(std::string const& strActorName, std::string const& strActionName);

	bool SetCameraByName(std::string const& strNIFName, char const* pcCameraName);
	bool SetRecursiveCameraByName(std::string const& strNIFName, char const* pcCameraName);
	//! 카메라를 가져온다.
	NiCamera* GetCamera();

	float GetOrthoZoom() const { return m_fOrthoZoom; };
	void SetOrthoZoom(float const fZoom, int const iMoveX = 0, int const iMoveY = 0);
	void SetOrthoZoomOld(float const fZoom, int const iMoveX=0, int const iMoveY=0);
	
	virtual PgUITexture* GetTex() { return m_pkModelScreenTexture; }
	
	//! 애니메이션 시퀀스를 변경한다.
	bool SetTargetAnimation(std::string const& strActorName, int const iID);
	//! 애니메이션 스퀀스가 끝났는지 얻어온다.
	bool IsAnimationDone(std::string const& strActorName);
	//! 애니메이션 이름의 시퀀스 번호를 리턴한다.
	int GetAnimationSeqID(std::string const& strID, std::string const& StrAnimationName);

	//! 카메라를 리셋
	void CameraReset(NiAVObject* pkModelRoot, SCameraResetInfo const& kResetInfo, float const fPerspective = 2.0f);
	void CameraReset(NiAVObject* pkModelRoot, bool const bUseHeightRadian, float const fXCenter = 0.5f, float const fYCenter = 0.5f, float const fPerspective = 2.0f);
	void CameraReset(NiAVObject* pkModelRoot, NiPoint3 const& kTranslate, NiPoint3 const& kAddLook);
	//! 카메라를 줌 인/아웃 한다.
	void CameraZoom(float const fZoomDistance); 
	//! 카메라의 줌 값을 그냥 대입
	void CameraZoomSubstitution(float const fZoomDistance); 
	//! 카메라의 줌 최소/최대를 설정한다.
	void SetCameraZoomMinMax(float const fMin, float const fMax);
	//! RenderModel이 가지고 있는 ActorManager / Nif를 World에 대해 회전 시킨다.
	void LocalRotate(NiAVObject *pkObject, float const fXAxis, float const fYAxis, float const fZAxis);
	//! ActorManager / Nif를 회전 시킨다.
	void WorldRotate(float const fXAxis, float const fYAxis, float const fZAxis);
	void SetWorldLimitRotate(float const fXAxis, float const fYAxis, float const fZAxis);
	//! RenderModel들을 자동으로 회전시켜준다.
	void SetAutoRotate(bool const bAutoRotate, float const fXAxis, float const fYAxis, float const fZAxis);
	//! Update를 할지 / 안할지 설정 한다.(원하는 시점에 업데이트 하기 위해서)
	void SetEnableUpdate(bool const bEnableUpdate);
	void SetNIFEnableUpdate(std::string const& strNIFName, bool const bEnableUpdate);
	void SetActorEnableUpdate(std::string const& strActorName, bool const bEnableUpdate);

	//!월드 에서 사용하는 스팟 라이트를 사용할 것인가?
	void EnableWorldSpotLight(bool bEnable) { m_bEnableWorldSpotLight = bEnable; }

protected:
	typedef std::map< std::string, NiActorManagerPtr > ContActor;
	typedef ContActor::iterator ContActorItor;

	typedef std::map< std::string, NiNodePtr > ContNIF;
	typedef ContNIF::iterator ContNIFItor;

	typedef std::map< std::string, UIModelPgActorInfo* > ContPgActor;
	typedef std::map< std::string, UIModelPgPetInfo* > ContPgPet;

	typedef std::map< std::string, NiActorManager::SequenceID > ContActorSeqID;
	typedef ContActorSeqID::iterator ContActorSeqIDItor;

	typedef	std::vector<std::string>	ContName;
	typedef ContName::iterator			ContNameItor;

	typedef std::map<std::string, std::string> VariTextureContainer;

	ContActor		m_kActorContainer;
	ContActorSeqID	m_kActorSeqContainer;
	ContPgActor		m_kPgActorContainer;
	ContPgPet		m_kPgPetContainer;

	ContNIF			m_kNIFContainer;
	
	NiCameraPtr				m_spCamera;

	NiScreenTexture*		m_pkModelScreenTexture;
	NiRenderTargetGroupPtr	m_spRenderTargetGroup;
	NiRenderedTexturePtr	m_spRenderTexture;
	NiTexture::RendererData *m_pkRendererData;

	POINT2 m_kWndSize;

	float		m_fMinCameraZoom;
	float		m_fMaxCameraZoom;
	float		m_fCurCameraZoom;
	float		m_fOrthoZoom;

	bool		m_bAutoRotateCamera;
	bool		m_bEnableUpdate;
	bool		m_bEnableWorldSpotLight; //월드에서 쓰는 스팟라이트를 사용할 것인가?(Default : false)

	NiPoint3	m_kAutoDegreeToAxis; //!x 축 / y축 / z축
	NiPoint3	m_kTotalDegreeToAxis;
	NiPoint3	m_kLimitDegreeToAxis;

	ContName	m_kDisableUpdateNIFContainer;
	ContName	m_kDisableUpdateActorManagerContainer;

	SCameraResetInfo	m_kCameraResetInfo;

protected:
	bool AddNodeAlphaProperty(NiNode* pNiNode);

	//! 모델의 바운드를 찾는다.
	void GetWorldBounds(NiAVObject* pkObject, NiBound& kBound);

	//! 노드의 바운드를 찾는다.
	void GetWorldBoundsNode(NiNode* pkObject, NiBound& kBound);

	//! 모델을 업데이트 한다.
	bool UpdateModel(NiAVObject* pkModelRoot, bool const bCameraReset = true);

	//! Texture 바리에이션이 있을 경우 텍스쳐를 변경 해준다.
	void ChangeTexture(NiNode* pkRoot, VariTextureContainer& rkTextureContainer);

	bool CalcPerspectiveFrustum(NiAVObject* pkObject, float const fXCenter = 0.5f, float const fYCenter = 0.5f, float const fPerspective = 2.0f);
	bool CalcOrthogonalFrustum(float fRadian, float const fXCenter = 0.5f, float const fYCenter = 0.5f);

	bool	CheckNeedInvlidateRenderTarget();
};
#endif // FREEDOM_DRAGONICA_UI_PGUIMODEL_H