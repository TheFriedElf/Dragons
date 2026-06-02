#include "StdAfx.h"
#include "PgMobilesuit.h"
#include "PgCameraModeFollow.h"
#include "PgInterpolator.h"
#include "PgDamper.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "PgInput.H"
#include "PgRenderer.H"

#include "NewWare/Scene/ApplyTraversal.h"


CameraInfo PgCameraModeFollow::ms_kDefaultCamera;
CameraInfo PgCameraModeFollow::ms_kClassDefaultCamera[4];
FollowCameraInfo PgCameraModeFollow::ms_kFollowCameraInfo; // Static인 이유는.. 맵 이동시에도 유지되게..
CameraAreaInfo PgCameraModeFollow::ms_CameraAreaInfo;
CameraAreaInfo PgCameraModeFollow::ms_SavedCameraAreaInfo;

bool g_bCameraHeightAdjust = true; //! 점프시에 카메라가 캐릭터를 따라 올라가는 것을 막는다.
bool g_bUseCameraWall = true; //! 카메라 벽 체크 기능
bool g_bCameraSmoothMove = true;

NiQuaternion g_kSpeed;

#ifndef EXTERNAL_RELEASE

//#define PG_CAMERA_DEBUG

bool g_bCameraDebug = false;
#ifdef PG_CAMERA_DEBUG
FILE* fp;
#endif

#endif

void PgCameraModeFollow::UpdateMinMaxCameraZoomInfo()
{
	// calc min/max zoom delta;
	float fMinZoomPos = NiMin(m_kCameraAdjustInfo.fMinZoomPos, m_kCameraAdjustInfo.fMaxZoomPos);
	float fMaxZoomPos = NiMax(m_kCameraAdjustInfo.fMinZoomPos, m_kCameraAdjustInfo.fMaxZoomPos);
	float fMinZoomDelta = 0.0f;
	float fMaxZoomDelta = 0.0f;

	if (fMinZoomPos != 0.0f && fMinZoomPos > ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget)
	{
		fMinZoomDelta = (fMinZoomPos - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget) / (ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget);
	}
	else
	{
		fMinZoomDelta = m_kCameraAdjustInfo.fMinZoomDelta;
	}
	float const fMin = std::min<float>(1.0f,m_kCameraAdjustInfo.fMinZoomDelta);
	fMinZoomDelta = NiClamp(fMinZoomDelta, 0.0f, fMin);

	if (fMaxZoomPos != 0.0f && fMaxZoomPos < ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget)
	{
		fMaxZoomDelta = (fMaxZoomPos - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget) / (ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget);
	}
	else
	{
		fMaxZoomDelta = m_kCameraAdjustInfo.fMaxZoomDelta;
	}
	float const fMax = std::max<float>(1.0f,m_kCameraAdjustInfo.fMaxZoomDelta);
	fMaxZoomDelta = NiClamp(fMaxZoomDelta, 0.0f, fMax);

	m_kCameraAdjustInfo.fMinZoomDelta = NiMin(fMinZoomDelta, fMaxZoomDelta);
	m_kCameraAdjustInfo.fMaxZoomDelta = NiMax(fMinZoomDelta, fMaxZoomDelta);

	ms_kFollowCameraInfo.fZoomDelta = NiClamp(ms_kFollowCameraInfo.fZoomDelta, m_kCameraAdjustInfo.fMinZoomDelta, m_kCameraAdjustInfo.fMaxZoomDelta);
}

void PgCameraModeFollow::SetCameraZoom(float fZoom)
{
	ms_kFollowCameraInfo.fZoomDelta = fZoom;
}

float PgCameraModeFollow::GetCameraZoom()
{
	return ms_kFollowCameraInfo.fZoomDelta;
}

// default camera가 바뀔때마다 불러주면 된다.
void PgCameraModeFollow::SetDefaultCameraInfo()
{
	ms_kFollowCameraInfo.kCurrentPos = ms_kDefaultCamera;
	ms_kFollowCameraInfo.kCurrentEnd = ms_CameraAreaInfo.kZoomOutCamera;
	ms_kFollowCameraInfo.fZoomDelta = 1.0f;
	ms_kFollowCameraInfo.fUpDelta = 0.0f;

	if (NiAbs(ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget) > 10.0f)
	{
		ms_kFollowCameraInfo.fZoomDelta = (ms_kDefaultCamera.fDistanceFromTarget - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget) / (ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget - ms_CameraAreaInfo.kZoomInCamera.fDistanceFromTarget);
	}
}

void PgCameraModeFollow::SetDefaultCamera(int iClass)
{
	if (iClass >= 0 && iClass < 4)
	{
		ms_kDefaultCamera = ms_kClassDefaultCamera[iClass];
		SetDefaultCameraInfo();
	}
}

void PgCameraModeFollow::ResetCameraPos()
{
	SetDefaultCameraInfo();
	UpdateMinMaxCameraZoomInfo();
	// adjust zoom delta by min/max zoom delta
	ms_kFollowCameraInfo.fZoomDelta = NiClamp(ms_kFollowCameraInfo.fZoomDelta, m_kCameraAdjustInfo.fMinZoomDelta, m_kCameraAdjustInfo.fMaxZoomDelta);
	
	m_kCameraAdjustInfo.kCameraAdjust.fLookAtDepth = 0.0f;
	m_kCameraAdjustInfo.kCameraAdjust.fLookAtWidth = 0.0f;
	m_kCameraAdjustInfo.kCameraAdjust.fLookAtHeight = 0.0f;

	m_kCameraAdjustInfo.kCameraAdjust.fCameraWidth = 0.0f;
	m_kCameraAdjustInfo.kCameraAdjust.fCameraHeight = 0.0f;
	m_kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget = 0.0f;
}

void PgCameraModeFollow::SaveCameraPos()
{
	m_kSavedCameraInfo = ms_kFollowCameraInfo;
	ms_SavedCameraAreaInfo = ms_CameraAreaInfo;
}

void PgCameraModeFollow::RestoreCameraPos()
{
	ms_kFollowCameraInfo = m_kSavedCameraInfo;
	ms_CameraAreaInfo = ms_SavedCameraAreaInfo;
	//ms_kFollowCameraInfo.kCurrentEnd = ms_CameraAreaInfo.kZoomOutCamera;
	UpdateMinMaxCameraZoomInfo();
}

void PgCameraModeFollow::SetCameraZoomMax(float fValue)
{
	ms_CameraAreaInfo.kZoomOutCamera.fDistanceFromTarget = fValue;
	ms_kFollowCameraInfo.kCurrentEnd = ms_CameraAreaInfo.kZoomOutCamera;
	UpdateMinMaxCameraZoomInfo();
}

PgCameraModeFollow::PgCameraModeFollow(NiCamera *pkCamera, PgActor *pkActor)
: PgICameraMode(pkCamera)
{
	PG_ASSERT_LOG(pkActor);
	m_pkActor = pkActor;
	m_kCameraPrevTrn = NiPoint3::ZERO;
	m_kActorPrevTrn = NiPoint3::ZERO;
	m_fLastUpdateFrame = 0.0f;
	
	m_fNormalInterpolateTime = 1.0f;
	m_kTargetPathNormal = NiPoint3::ZERO;
	m_kCurrentPathNormal = NiPoint3::ZERO;
	m_kLastPathNormal = NiPoint3::ZERO;

	m_fTransInterpolateTime = 1.0f;
	m_kTargetTrans = NiPoint3::ZERO;
	m_kCurrentTrans = NiPoint3::ZERO;
	m_kLastTrans = NiPoint3::ZERO;

	m_kTargetFloorTrans = NiPoint3::ZERO;
	m_kTargetFloorTransOld = NiPoint3::ZERO;
	m_kTargetTransOld = NiPoint3::ZERO;

	m_fRotateInterpolateTime = 1.0f;
	m_kTargetRot = NiQuaternion(0, 0, 0, 0);
	m_kCurrentRot = NiQuaternion(0, 0, 0, 0);
	m_kLastRot = NiQuaternion(0, 0, 0, 0);	

	// w : Rot speed
	// x : Spring Const
	// y : Damp Const
	// z : Spring Length
	//g_kSpeed = NiQuaternion(5.0f, 2.5f, 10.0f, 0.2f); Original
	g_kSpeed = NiQuaternion(5.0f, 7.0f, 0.0f, 0.2f);

	// 아래 주석을 풀면, 맵마다 카메라 위치가 초기화 된다.
	//ms_kFollowCameraInfo.kCurrentPos = ms_kDefaultCamera;
	//ms_kFollowCameraInfo.kCurrentEnd = ms_CameraAreaInfo.kZoomOutCamera;
	m_bFixedCamera = false;
	m_eMoveType = MOVETYPE_DEPTH_FOLLOW_ACTOR;

	m_fMaxZoomDelta = 1.0f;
	m_fMinZoomDelta = 0.0f;
	m_fLastCameraZoomDelta = -1.0f;
	m_kSavedCameraInfo = ms_kFollowCameraInfo;
	
#ifdef PG_CAMERA_DEBUG
	fp = fopen("camera.txt", "wt");
	fputs("time, time(d), fTime, fCount, actor.x, actor.y, actor.z, actor(o).x, actor(o).y, actor(o).z, lastF.x, lastF.y, lastF.z, lastF.x(o), lastF.y(o), lastF.z(o), camera(o).x, camera(o).y, camera(o).z, camera(n).x, camera(n).y, camera(n).z", fp);
#endif
}

PgCameraModeFollow::~PgCameraModeFollow()
{
	m_kHiddenObjectContainerList.clear();
	releaseAllCamWalls();
#ifdef PG_CAMERA_DEBUG
	fclose(fp);
#endif
}

bool PgCameraModeFollow::Update(float fFrameTime)
{
	ms_kFollowCameraInfo.fZoomDelta = NiClamp(ms_kFollowCameraInfo.fZoomDelta, m_kCameraAdjustInfo.fMinZoomDelta, m_kCameraAdjustInfo.fMaxZoomDelta);

	if(ms_kFollowCameraInfo.fZoomDelta != m_fLastCameraZoomDelta)
	{
		ms_kFollowCameraInfo.kCurrentPos = PgCameraMan::Lerp(ms_CameraAreaInfo.kZoomInCamera,ms_kFollowCameraInfo.kCurrentEnd, ms_kFollowCameraInfo.fZoomDelta);

		float fActorRadius = 50.0f; // GetWorldBoudn.GetRadius()가 이상하게 나온다. Particle때문인것 같기도.
		if (fActorRadius + 20.0f + g_kFrustum.m_fNear > ms_kFollowCameraInfo.kCurrentPos.fDistanceFromTarget + m_kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget) // 캐릭터 위치보다 안쪽으로 들어가는 것을 막기 위해.
		{
			ms_kFollowCameraInfo.fZoomDelta = m_fLastCameraZoomDelta;
			ms_kFollowCameraInfo.kCurrentPos = PgCameraMan::Lerp(ms_CameraAreaInfo.kZoomInCamera,ms_kFollowCameraInfo.kCurrentEnd, ms_kFollowCameraInfo.fZoomDelta);			
		}
		m_fLastCameraZoomDelta = ms_kFollowCameraInfo.fZoomDelta;
	}
	
#ifdef PG_USE_NEW_CAMERAWALL
	m_kTargetTrans = adjustCameraByWall3(m_pkActor->GetPosition(), m_kTargetTransOld);
	m_kTargetTransOld = m_kTargetTrans;

	m_kTargetFloorTrans = adjustCameraByWall3(m_pkActor->GetLastFloorPos(), m_kTargetFloorTransOld);
	m_kTargetFloorTransOld = m_kTargetFloorTrans;
#endif
	bool bRetT = UpdateTranslate(fFrameTime);
	bool bRetR = UpdateRotate(fFrameTime);
	
	// 가리는 녀석을 투명하게
	static bool bHide = true;
	if (bHide) {
		//DWORD dwTime = BM::GetTime32();
		UpdateObstacle(g_pkApp->GetAccumTime(),fFrameTime);
		//UpdateHideCoveredObjects2();
		//UpdateHideObjectAlpha(fFrameTime);
		//_PgOutputDebugString("HideTime %d\n", BM::GetTime32() - dwTime);
	}
	
	if(bRetT == false && bRetR == false)
	{
		return false;
	}
	return true;
}
void	PgCameraModeFollow::UpdateObstacle(float fAccumTime,float fFrameTime)
{
	PgWorld::HideObjectContainer	kObstacleCont;

	//	Find Obstacle
	if(!m_pkActor || !m_pkActor->GetCharRoot() || NULL==g_pkWorld)
	{
		return;
	}

	PgWorld::HideObjectContainer	const	&kHideObjectCont = g_pkWorld->GetHideObjectCont();
	if(kHideObjectCont.size() == 0)
	{
		return;
	}

	NiAVObject* pkCenterDummy = m_pkActor->GetCharRoot()->GetObjectByName(ATTACH_POINT_CENTER);
	if(!pkCenterDummy)
	{
		return;
	}

	NiPoint3	const	&kStartPos = pkCenterDummy->GetWorldTranslate();
	NiPoint3	const	&kCamPos = m_pkCamera->GetWorldTranslate();
	NiPoint3	kRayDir = kCamPos - kStartPos;

	m_kHidePick.SetObserveAppCullFlag(true);
	m_kHidePick.SetSortType(NiPick::NO_SORT);
	m_kHidePick.SetFrontOnly(false);
	m_kHidePick.SetPickType(NiPick::FIND_ALL);

	for(PgWorld::HideObjectContainer::const_iterator itor = kHideObjectCont.begin(); itor != kHideObjectCont.end(); ++itor)
	{
		NiAVObject	*pkAVObject = itor->first;
		PgWorld::SHideObject	const	&kHideObjectInfo = itor->second;

		m_kHidePick.ClearResultsArray();
		m_kHidePick.SetTarget(pkAVObject);

		if(m_kHidePick.PickObjects(kStartPos, kRayDir))
		{
			NiPick::Results &rkRes = m_kHidePick.GetResults();
			for(unsigned int i = 0; i < rkRes.GetSize(); ++i)
			{
				NiPick::Record *pkRec = rkRes.GetAt(i);
				NiAVObject *pkObject = pkRec->GetAVObject();

				kObstacleCont.insert(std::make_pair(pkObject,kHideObjectInfo));
			}
			
		}
	}
	//	Set fade in obstables which is not really an obstacle anymore
	for(ObstacleInfoMap::iterator itor = m_kObstacleInfoCont.begin(); itor != m_kObstacleInfoCont.end(); ++itor)
	{
		NiAVObject	*pkAVObject = itor->first;
		stObstacleInfo	&kObstacleInfo = itor->second;

		if(kObstacleInfo.m_bRemove)
		{
			continue;
		}

		PgWorld::HideObjectContainer::iterator itor2 = kObstacleCont.find(pkAVObject);
		if(itor2 ==  kObstacleCont.end())	//	this does not exist in the container, this should be removed
		{
			NiMaterialProperty	*pkMaterialProp = NiDynamicCast(NiMaterialProperty, pkAVObject->GetProperty(NiMaterialProperty::GetType()));
			float	fStartAlpha = 0.0f;
			if(pkMaterialProp)
			{
				fStartAlpha = pkMaterialProp->GetAlpha();
			}
			float	fTargetAlpha = 1.0f;
			if(kObstacleInfo.m_spOriginalMaterialProperty)
			{
				fTargetAlpha = kObstacleInfo.m_spOriginalMaterialProperty->GetAlpha();
			}


			kObstacleInfo.m_bRemove = true;
			kObstacleInfo.m_fAlphaTransitStartTime = fAccumTime;
			kObstacleInfo.m_fStartAlpha = fStartAlpha;
			kObstacleInfo.m_fTargetAlpha = fTargetAlpha;
		}

	}

	//	Add obstables which is not already in the container
	for(PgWorld::HideObjectContainer::iterator itor = kObstacleCont.begin(); itor != kObstacleCont.end() ;++itor)
	{
		NiAVObject	*pkAVObject = itor->first;
		PgWorld::SHideObject	const	&kHideObjectInfo = itor->second;

		ObstacleInfoMap::iterator	itor2 = m_kObstacleInfoCont.find(pkAVObject);
		
		bool	bAdd = (itor2 == m_kObstacleInfoCont.end());	//	not exist in the container

		if(itor2 != m_kObstacleInfoCont.end())
		{
			stObstacleInfo	&kInfo = itor2->second;
			bAdd = kInfo.m_bRemove;	//	it is being removed
		}

		if(bAdd)
		{
			stObstacleInfo	kNewObstacle;

			NiAlphaProperty	*pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkAVObject->GetProperty(NiAlphaProperty::GetType()));
			NiMaterialProperty	*pkMaterialProp = NiDynamicCast(NiMaterialProperty, pkAVObject->GetProperty(NiMaterialProperty::GetType()));

			kNewObstacle.m_spOriginalAlphaProperty = pkAlphaProp;
			kNewObstacle.m_spOriginalMaterialProperty = pkMaterialProp;

			pkAVObject->DetachProperty(pkAlphaProp);
			pkAVObject->DetachProperty(pkMaterialProp);

			NiAlphaProperty	*pkNewAlphaProp = NULL;
			if(pkAlphaProp)
			{
				pkNewAlphaProp = NiDynamicCast(NiAlphaProperty, pkAlphaProp->Clone());
			}
			else
			{
				pkNewAlphaProp = NiNew NiAlphaProperty();
				pkNewAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
				pkNewAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
			}
			pkNewAlphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
			pkNewAlphaProp->SetAlphaBlending(true);
			pkNewAlphaProp->SetAlphaGroup(AG_OBSTRUCTOR);
			pkNewAlphaProp->SetNoSorter(false);

			float	fStartAlpha = 1.0f;
			NiMaterialProperty	*pkNewMaterialProp = 0;
			if(pkMaterialProp)
			{
				pkNewMaterialProp = NiDynamicCast(NiMaterialProperty,pkMaterialProp->Clone()); 
			}
			else
			{
				pkNewMaterialProp = NiNew NiMaterialProperty();
				pkNewMaterialProp->SetDiffuseColor(NiColor::WHITE);
				pkNewMaterialProp->SetSpecularColor(NiColor::BLACK);
				pkNewMaterialProp->SetAmbientColor(NiColor::WHITE);
				pkNewMaterialProp->SetEmittance(NiColor::BLACK);
			}

			pkAVObject->AttachProperty(pkNewAlphaProp);
			pkAVObject->AttachProperty(pkNewMaterialProp);
			pkAVObject->UpdateProperties();

			kNewObstacle.m_bRemove = false;
			kNewObstacle.m_fAlphaTestRef = pkNewAlphaProp->GetTestRef()/255.0f;
			kNewObstacle.m_fAlphaTransitStartTime = fAccumTime;
			kNewObstacle.m_fStartAlpha = pkNewMaterialProp->GetAlpha();
			kNewObstacle.m_fTargetAlpha = kHideObjectInfo.fAlphaValue;

			m_kObstacleInfoCont.insert(std::make_pair(pkAVObject,kNewObstacle));
		}
	}


	//	Update Alpha
	for(ObstacleInfoMap::iterator itor = m_kObstacleInfoCont.begin(); itor != m_kObstacleInfoCont.end(); )
	{
		NiAVObject	*pkAVObject = itor->first;
		stObstacleInfo	&kInfo = itor->second;


        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkAVObject, false );

		float	fNextAlpha = 1.0f;
		float	fAlphaTransitVector = 1.0f;
		float	fTime = fAccumTime - kInfo.m_fAlphaTransitStartTime;
		bool	bTransitionComplete = false;
		if(kInfo.m_fTargetAlpha<kInfo.m_fStartAlpha)
		{
			fNextAlpha = kInfo.m_fStartAlpha - fAlphaTransitVector*fTime;
			if(fNextAlpha<=kInfo.m_fTargetAlpha)
			{
				bTransitionComplete = true;
				fNextAlpha = kInfo.m_fTargetAlpha;
			}
		}
		else if(kInfo.m_fTargetAlpha>kInfo.m_fStartAlpha)
		{
			fNextAlpha = kInfo.m_fStartAlpha + fAlphaTransitVector*fTime;
			if(fNextAlpha>=kInfo.m_fTargetAlpha)
			{
				bTransitionComplete = true;
				fNextAlpha = kInfo.m_fTargetAlpha;
			}
		}
		else
		{
			bTransitionComplete = true;
		}


		if(bTransitionComplete && kInfo.m_bRemove)
		{
			NiAlphaProperty	*pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkAVObject->GetProperty(NiAlphaProperty::GetType()));
			NiMaterialProperty	*pkMaterialProp = NiDynamicCast(NiMaterialProperty, pkAVObject->GetProperty(NiMaterialProperty::GetType()));

			pkAVObject->DetachProperty(pkAlphaProp);
			pkAVObject->DetachProperty(pkMaterialProp);

			pkAVObject->AttachProperty(kInfo.m_spOriginalAlphaProperty);
			pkAVObject->AttachProperty(kInfo.m_spOriginalMaterialProperty);
			pkAVObject->UpdateProperties();

			itor = m_kObstacleInfoCont.erase(itor);
			continue;
		}

		NiMaterialProperty	*pkMaterialProp = NiDynamicCast(NiMaterialProperty, pkAVObject->GetProperty(NiMaterialProperty::GetType()));
		if(pkMaterialProp)
		{
			pkMaterialProp->SetAlpha(fNextAlpha);
		}

		NiAlphaProperty	*pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkAVObject->GetProperty(NiAlphaProperty::GetType()));
		float	fTestRef = pkAlphaProp->GetTestRef();
		if(fNextAlpha<fTestRef)
		{
			fTestRef = fNextAlpha;
		}
		else if(fNextAlpha>fTestRef)
		{
			fTestRef = fNextAlpha;
		}
		if(fTestRef>kInfo.m_fAlphaTestRef)
		{
			fTestRef = kInfo.m_fAlphaTestRef;
		}
		pkAlphaProp->SetTestRef(fTestRef*255);


		++itor;
	}
}
bool PgCameraModeFollow::UpdateTranslate(float fFrameTime)
{
	PG_ASSERT_LOG(m_pkCamera);
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkCamera == NULL || m_pkActor == NULL)
		return false;
	if(!g_pkWorld)
		return	false;

	NiPoint3 const &kCamTrn = m_pkCamera->GetTranslate();
	//NiPoint3 kActorTrn = m_pkActor->GetWorldTranslate();
	//NiPoint3 kActorTrn = m_pkActor->GetPosition();
	NiPoint3 kActorTrn = m_kTargetTrans;	
	NiPoint3 kActorLastFloorTrn = m_pkActor->GetLastFloorPos();
	NiPoint3 kNewCamTrn = NiPoint3::ZERO;
	float fActorRadius = m_pkActor->GetWorldBound().GetRadius();
	float fCurrentTime = g_pkWorld->GetAccumTime();
	float fActorDiff = (kActorTrn - m_kActorPrevTrn).Length();

#ifdef PG_CAMERA_DEBUG
	char buf[1024];
	if (g_bCameraDebug)
	{
		sprintf_s(buf, 1024, "\n%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,", 
			NiGetCurrentTimeInSec(), fCurrentTime - m_fLastUpdateFrame, fFrameTime, g_pkApp->GetFrameCount(),
			kActorTrn.x, kActorTrn.y, kActorTrn.z,
			m_kActorPrevTrn.x - kActorTrn.x, m_kActorPrevTrn.y - kActorTrn.y, m_kActorPrevTrn.z - kActorTrn.z,
			kActorLastFloorTrn.x, kActorLastFloorTrn.y, kActorLastFloorTrn.z,
			m_kTargetFloorTransOld.x, m_kTargetFloorTransOld.y, m_kTargetFloorTransOld.z,
			//m_kLastPathNormal.x, m_kLastPathNormal.y, m_kLastPathNormal.z,
			//m_kCurrentPathNormal.x, m_kCurrentPathNormal.y, m_kCurrentPathNormal.z,
			//m_fNormalInterpolateTime,
			//m_kTargetPathNormal.x, m_kTargetPathNormal.y, m_kTargetPathNormal.z,
			//m_pkActor->GetPathNormal().x, m_pkActor->GetPathNormal().y, m_pkActor->GetPathNormal().z,
			kCamTrn.x, kCamTrn.y, kCamTrn.z);
		fputs(buf, fp);
	}
#endif
	m_fLastUpdateFrame = fCurrentTime;

	// -- 패스 노멀을 보간 (부드러운 카메라 이동에 도움이 된다) --
	if(m_kLastPathNormal == NiPoint3::ZERO)
	{
		m_kCurrentPathNormal = m_kLastPathNormal = m_pkActor->GetPathNormal();
		m_kCameraPrevTrn = kCamTrn;
	}

	if(m_kTargetPathNormal != m_pkActor->GetPathNormal())
	{
		m_kLastPathNormal = m_kCurrentPathNormal;
		m_kTargetPathNormal = m_pkActor->GetPathNormal();
		if (g_bCameraSmoothMove)
			m_fNormalInterpolateTime = 0.0f;
		else
			m_kCurrentPathNormal = m_kTargetPathNormal;
	}
	if(m_fNormalInterpolateTime < 1.0f)
	{
		m_fNormalInterpolateTime += (fFrameTime / 2.0f); // * g_kSpeed.GetW();
		m_fNormalInterpolateTime = NiClamp(m_fNormalInterpolateTime, 0.0f, 1.0f);
		m_kCurrentPathNormal = PgInterpolator::Lerp(m_kLastPathNormal, m_kTargetPathNormal, m_fNormalInterpolateTime);
	}
	// -- 패스 노멀을 보간 --

	// -- 위치 수정 --
	if (m_eMoveType == MOVETYPE_DEPTH_FOLLOW_ACTOR)
	{
		// 아무런 조작 없음.
	}
	else if (m_eMoveType == MOVETYPE_DEPTH_FOLLOW_PATH)
	{
		//! 이 코드를 집어 넣으면 Path에 대한 상대위치가 되어버린다. (2007.09.06 강정욱)
		kActorTrn.x = m_pkActor->GetPathImpactPoint().x;
		kActorTrn.y = m_pkActor->GetPathImpactPoint().y;
	}

	float fHeight = m_pkActor->GetJumpAccumHeight() * 0.7f;
	if (g_bCameraHeightAdjust && kActorLastFloorTrn.z > INVALID_Z_HEIGHT)
	{
		if (kActorTrn.z - kActorLastFloorTrn.z >= 0 && kActorTrn.z - kActorLastFloorTrn.z <= PgCameraMan::GetJumpHeight()) //! 점프 중이라면..
		{
			kActorTrn.z = kActorLastFloorTrn.z; //! 캐릭터가 바닥에 있던 지점을 바라본다.
			fHeight = 0.0f;
		}
		else if (kActorTrn.z - kActorLastFloorTrn.z > PgCameraMan::GetJumpHeight())
		{
			//_PgOutputDebugString("camera z diff : %f\n", kActorTrn.z - kActorLastFloorTrn.z);
		}
	}
	
	// 바닥과 여백을 이용해 목표 위치를 구한다.
	kNewCamTrn = kActorTrn - (m_kCurrentPathNormal * (ms_kFollowCameraInfo.kCurrentPos.fDistanceFromTarget + m_kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget));
	kNewCamTrn.z += (ms_kFollowCameraInfo.kCurrentPos.fCameraHeight + m_kCameraAdjustInfo.kCameraAdjust.fCameraHeight);
	kNewCamTrn.x += (ms_kFollowCameraInfo.kCurrentPos.fCameraWidth + m_kCameraAdjustInfo.kCameraAdjust.fCameraWidth);

	// 점프시에 조정해줌.
	if(g_bCameraHeightAdjust)
	{
		kNewCamTrn.z -= fHeight;
	}

	// 위치의 변화가 없다면
	if(kNewCamTrn == kCamTrn)
	{
		return false;	// 업데이트 하지말라
	}
	
	//위에서 목표 위치를 한번만 구하고, 이부분에 있어서 speed (inch/second)나 보간 방법을 택해 해당 목표점에 대한 위치를 서서히 이동시킬수 있게끔 작업이 필요하다

	// 스프링 뎀핑한다.
	if (g_bCameraSmoothMove)
	{
		NiPoint3 kPos = PgDamper::SpringDamp(kCamTrn, kNewCamTrn, m_kCameraPrevTrn, fFrameTime, g_kSpeed.GetX(), g_kSpeed.GetY(), g_kSpeed.GetZ(), 8.0f, g_bCameraHeightAdjust);
		if (m_bFixedCamera)
		{

		}

		if(0.0f>=(kPos-kCamTrn).SqrLength())
		{
			m_pkCamera->SetTranslate(kNewCamTrn);	
		}
		else
		{
			m_pkCamera->SetTranslate(kPos);
			kNewCamTrn = kPos;
		}
	}
	else
	{
		m_pkCamera->SetTranslate(kNewCamTrn);
	}

#ifdef PG_USE_NEW_CAMERAWALL
	// 카메라는 카메라벽에 전혀 영향을 안받는다!!!
	//m_pkCamera->SetTranslate(adjustCameraByWall3(m_pkCamera->GetTranslate(), m_kCameraPrevTrn));
#else
	m_pkCamera->SetTranslate(adjustCameraByWall(m_pkCamera->GetTranslate()));
#endif

	m_pkCamera->Update(fFrameTime);

#ifdef PG_CAMERA_DEBUG
	if (g_bCameraDebug)
	{
		sprintf_s(buf, 1024, "NewCam(%f,%f,%f), PrevTrn(%f,%f,%f) Moved(%f,%f,%f) Diff1(%f,%f,%f)(%f), Diff2(%f,%f,%f), ", 
			kNewCamTrn.x, kNewCamTrn.y, kNewCamTrn.z,
			m_kCameraPrevTrn.x, m_kCameraPrevTrn.y, m_kCameraPrevTrn.z,
			m_pkCamera->GetTranslate().x, m_pkCamera->GetTranslate().y, m_pkCamera->GetTranslate().z,
			kNewCamTrn.x - m_kCameraPrevTrn.x, kNewCamTrn.y - m_kCameraPrevTrn.y, kNewCamTrn.z - m_kCameraPrevTrn.z, (kNewCamTrn.x - m_kCameraPrevTrn.x) / fFrameTime,
			m_pkCamera->GetTranslate().x - m_kCameraPrevTrn.x, m_pkCamera->GetTranslate().y - m_kCameraPrevTrn.y, m_pkCamera->GetTranslate().z - m_kCameraPrevTrn.z);
		fputs(buf, fp);
	}
#endif	

	m_kCameraPrevTrn = m_pkCamera->GetTranslate();
	m_kActorPrevTrn = m_pkActor->GetPosition();
	return true;
}

bool PgCameraModeFollow::UpdateRotate(float fFrameTime)
{
	PG_ASSERT_LOG(m_pkCamera);
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkCamera == NULL || m_pkActor == NULL)
		return false;

#ifdef PG_USE_NEW_CAMERAWALL
	NiPoint3 kTargetTrn = m_kTargetTrans;
	NiPoint3 kActorLastFloorTrn = m_kTargetFloorTrans;
#else
	//NiPoint3 kTargetTrn = adjustCameraByWall(m_pkActor->GetWorldTranslate());
	NiPoint3 kTargetTrn = adjustCameraByWall(m_pkActor->GetPosition());
	NiPoint3 kActorLastFloorTrn = adjustCameraByWall(m_pkActor->GetLastFloorPos());
#endif
	NiPoint3 kCamTrn = m_pkCamera->GetTranslate();
	NiQuaternion kNewCamRot;

//#ifdef PG_CAMERA_DEBUG
//	char buf[1024];
//	if (g_bCameraDebug)
//	{
//		sprintf_s(buf, 1024, "TargetTrn(%f,%f,%f), ", 
//			kTargetTrn.x, kTargetTrn.y, kTargetTrn.z);
//		fputs(buf, fp);
//	}
//#endif

	if (g_bCameraHeightAdjust && kActorLastFloorTrn.z > INVALID_Z_HEIGHT)		
	{
		if (kTargetTrn.z - kActorLastFloorTrn.z >= 0 && kTargetTrn.z - kActorLastFloorTrn.z <= PgCameraMan::GetJumpHeight()) //! 점프 중이라면..
			kTargetTrn.z = kActorLastFloorTrn.z; //! 캐릭터가 바닥에 있던 지점을 바라본다.
	}

	// 목표 지점을 구한다.
	kTargetTrn += ( m_kCurrentPathNormal * 100.0f) 
				  + ( NiPoint3::UNIT_Z * (ms_kFollowCameraInfo.kCurrentPos.fLookAtHeight + m_kCameraAdjustInfo.kCameraAdjust.fLookAtHeight))
				  + ( NiPoint3::UNIT_Y * (ms_kFollowCameraInfo.kCurrentPos.fLookAtDepth  + m_kCameraAdjustInfo.kCameraAdjust.fLookAtDepth) )
				  + ( NiPoint3::UNIT_X * (ms_kFollowCameraInfo.kCurrentPos.fLookAtWidth  + m_kCameraAdjustInfo.kCameraAdjust.fLookAtWidth) );

	//kTargetTrn += m_kCurrentPathNormal.UnitCross(NiPoint3::UNIT_Z) * g_kSpanRot.x;

	// 목표 지점을 바라보는 새로운 각도를 구한다.
	m_pkCamera->LookAtWorldPoint(kTargetTrn, NiPoint3::UNIT_Z);
	m_pkCamera->GetRotate(kNewCamRot);

	// 쿼터니언 축이 뒤틀리는 부분이면
	if(NiQuaternion::Dot(m_kCurrentRot, kNewCamRot) < 0)
	{
		// 축을 바로잡자
		NiPoint3 kAxis;
		float fAngle;
		m_kCurrentRot.ToAngleAxis(fAngle, kAxis);
		m_kCurrentRot.FromAngleAxis(-fAngle + NI_TWO_PI, -kAxis);
	}

	// 부드러운 회전을 위해서 보간한다.
	if(m_kTargetRot != kNewCamRot)
	{
		float fAngle = NiQuaternion::Dot(m_kTargetRot, m_kCurrentRot);
		//_PgOutputDebugString("[PgCameraModeFollow] Rot angle %f\n", fAngle);
		//if (fAngle != 1.0f)
		{
			//NiPoint3 axis(0, 0, 0);
			//float angle = 0.0f;
			//m_kTargetRot.ToAngleAxis(angle, axis);
			//NILOG(PGLOG_LOG, "[PgCameraModeFollow] Rot new target %f, %f, %f, %f\n", angle, axis.x, axis.y, axis.z);
			//m_kLastRot.ToAngleAxis(angle, axis);
			//NILOG(PGLOG_LOG, "[PgCameraModeFollow] Rot last target %f, %f, %f, %f\n", angle, axis.x, axis.y, axis.z);
			//m_kCurrentRot.ToAngleAxis(angle, axis);
			//NILOG(PGLOG_LOG, "[PgCameraModeFollow] Rot cur target %f, %f, %f, %f\n", angle, axis.x, axis.y, axis.z);
			m_kLastRot = m_kCurrentRot;
			m_kTargetRot = kNewCamRot;
			m_fRotateInterpolateTime = 0.0f;
		}
	}

	if(m_fRotateInterpolateTime < 1.0f)
	{
		if (g_bCameraSmoothMove)
			m_fRotateInterpolateTime += fFrameTime * g_kSpeed.GetW();
		else
			m_fRotateInterpolateTime = 1.0f;
		m_fRotateInterpolateTime = NiClamp(m_fRotateInterpolateTime, 0.0f, 1.0f);
		m_kCurrentRot = NiQuaternion::Slerp(m_fRotateInterpolateTime, m_kLastRot, m_kTargetRot);
		//m_kCurrentRot.Normalize();
		m_kCurrentRot.FastNormalize();
//#ifdef PG_CAMERA_DEBUG
//		char buf[1024];
//		if (g_bCameraDebug)
//		{
//			NiPoint3 axis(0, 0, 0);
//			float angle = 0.0f;
//			m_kLastRot.ToAngleAxis(angle, axis);
//			sprintf_s(buf, 1024, "Inter(%f), LastRot(%f,%f,%f,%f), ", m_fRotateInterpolateTime, axis.x, axis.y, axis.z, angle);
//			fputs(buf, fp);
//
//			m_kCurrentRot.ToAngleAxis(angle, axis);
//			sprintf_s(buf, 1024, "CurrentRot(%f,%f,%f,%f), ", axis.x, axis.y, axis.z, angle);
//			fputs(buf, fp);
//
//			m_kTargetRot.ToAngleAxis(angle, axis);
//			sprintf_s(buf, 1024, "TargetRot(%f,%f,%f,%f)", axis.x, axis.y, axis.z, angle);
//			fputs(buf, fp);			
//		}
//#endif		
		//NILOG(PGLOG_LOG, "[PgCameraModeFollow] Rot cur target %f, %f, %f, %f\n", angle, axis.x, axis.y, axis.z);
		//NILOG(PGLOG_LOG, "[PgCameraModeFollow] Interpolate time %f, %f\n", m_fRotateInterpolateTime, fFrameTime);
	}

	// 카메라 회전을 적용
	m_pkCamera->SetRotate(m_kCurrentRot);
	m_pkCamera->Update(fFrameTime);
	return true;
}


void PgCameraModeFollow::UpdateHideCoveredObjects2()
{
	if(NULL==m_pkCamera || NULL==m_pkActor || NULL==m_pkActor->GetWorld())
	{
		return;
	}
	// 참조 카운트가 없는 녀석들은 더 이상 가리지 않음
	UnsetHideObject();

	// 전 프레임에서 가리던 녀석들의 참조 카운트를 초기화한다.
	for(HiddenObjectContainerList::iterator itr = m_kHiddenObjectContainerList.begin();
		itr != m_kHiddenObjectContainerList.end() ;  ++itr)
	{
		//itr->second.bRef = false;
		itr->second.byStep = 2;
	}

	// 가리는 녀석들을 찾는다.
	kPick.ClearResultsArray();
	kPick.SetObserveAppCullFlag(true);
	kPick.SetSortType(NiPick::NO_SORT);
	kPick.SetFrontOnly(false);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetTarget(m_pkActor->GetWorld()->GetSceneRoot());

	//NiPoint3 kOrgPt = m_pkActor->GetWorldTranslate() + NiPoint3::UNIT_Z * 30.0f;
	//NiPoint3 kOrgPt = m_pkActor->GetPosition() + NiPoint3::UNIT_Z * 30.0f;
	//NiAVObject* pkHeadPos = m_pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
	NiAVObject* pkHeadPos = m_pkActor->GetNodePointStar();
	if(!pkHeadPos)
		return;

	NiPoint3 kOrgPt = pkHeadPos->GetTranslate();
	NiPoint3 kDir = m_pkCamera->GetTranslate() - kOrgPt;

	float fTotalDistance = kDir.Length();

	kDir.Unitize();

	// 중심점에서 하나더 찾는다.
	m_kHidePick.ClearResultsArray();
	m_kHidePick.SetObserveAppCullFlag(true);
	m_kHidePick.SetSortType(NiPick::NO_SORT);
	m_kHidePick.SetFrontOnly(false);
	m_kHidePick.SetPickType(NiPick::FIND_ALL);
	m_kHidePick.SetTarget(m_pkActor->GetWorld()->GetSceneRoot());

	NiAVObject* pkCenterPos = m_pkActor->GetCharRoot()->GetObjectByName(ATTACH_POINT_CENTER);
	if(!pkCenterPos)
		return;
	NiPoint3 kOrgPt2 = pkCenterPos->GetTranslate();
	NiPoint3 kDir2 = m_pkCamera->GetTranslate() - kOrgPt2;
	kDir2.Unitize();

	bool bSearch = false;
	if(m_kHidePick.PickObjects(kOrgPt2, kDir2))
	{
		NiPick::Results &rkRes = m_kHidePick.GetResults();

		for(unsigned int i = 0; i < rkRes.GetSize(); ++i)
		{
			NiPick::Record *pkRec = rkRes.GetAt(i);
			NiAVObject *pkObject = pkRec->GetAVObject();
			// 부모노드를 찾는다
			NiAVObject* pkObjectParent = FindHideObjectRoot(pkObject);
			if(pkObjectParent == NULL)
				continue;

			float fAlphaValue;
			// 숨기는 오브젝트 리스트에 있지는 판단
			if( g_pkWorld
				&& g_pkWorld->IsHideObject(pkObjectParent, fAlphaValue) )
			{
				bSearch = true;
				break;
			}
		}
	}

	if(bSearch && kPick.PickObjects(kOrgPt, kDir))
	{
		NiPick::Results &rkRes = kPick.GetResults();

		for(unsigned int i = 0; i < rkRes.GetSize(); ++i)
		{
			NiPick::Record *pkRec = rkRes.GetAt(i);
			NiAVObject *pkObject = pkRec->GetAVObject();
		
			float fDistance = pkRec->GetDistance();

			if(fDistance > fTotalDistance)
				continue;

			// 원래 가리던 녀석이면 참조 카운트만 올리고
			HiddenObjectContainerList::iterator itr = m_kHiddenObjectContainerList.find(pkObject);
			if(itr != m_kHiddenObjectContainerList.end())
			{
				//itr->second.bRef = true;
				itr->second.byStep = 1;
			}
			// 새로 들어온 녀석이면 숨기기 해준다.
			else
			{
				//// 알파 속성이 있으면 패스
				//NiAlphaPropertyPtr pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkObject->GetProperty(NiAlphaProperty::GetType()));
				//if(pkAlphaProp)
				//	continue;

				//// 바운드 크기가 작으면 패스
				//if(pkObject->GetWorldBound().GetRadius() < m_pkActor->GetNIFRoot()->GetWorldBound().GetRadius() )
				//	continue;

				// 부모노드를 찾는다
				NiAVObject* pkObjectParent = FindHideObjectRoot(pkObject);
				if(pkObjectParent == NULL)
					continue;

				// 숨기는 오브젝트 리스트에 있지는 판단
				float fAlphaValue;
				if( g_pkWorld
					&& !g_pkWorld->IsHideObject(pkObjectParent, fAlphaValue) )
					continue;

				// 이미 숨기고 있는건지 부모노드에서 찾는다.
				if(IsHideObjectSearchParentNode(pkObjectParent))
					continue;

				HiddenObjectPropertyList kProp;
				//kProp.bRef = true;
				kProp.byStep = 1;
				kProp.pParentNode = pkObjectParent;
				SetHideObject(pkObjectParent, fAlphaValue, kProp);
				m_kHiddenObjectContainerList.insert(std::make_pair(pkObject, kProp));
			}
		}
	}

	//// 참조 카운트가 없는 녀석들은 더 이상 가리지 않음
	//UnsetHideObject();
}



//void PgCameraModeFollow::UpdateHideCoveredObjects()
//{
//	// 전 프레임에서 가리던 녀석들의 참조 카운트를 초기화한다.
//	for(HiddenObjectContainer::iterator itr = m_kHiddenObjectContainer.begin();
//		itr != m_kHiddenObjectContainer.end();
//		++itr)
//	{
//		itr->second.cRefCount = 0;
//	}
//
//	// 가리는 녀석들을 찾는다.
//	kPick.ClearResultsArray();
//	kPick.SetObserveAppCullFlag(true);
//	kPick.SetSortType(NiPick::NO_SORT);
//	kPick.SetFrontOnly(false);
//	kPick.SetTarget(m_pkActor->GetWorld()->GetSceneRoot());
//
//	//NiPoint3 kOrgPt = m_pkActor->GetWorldTranslate() + NiPoint3::UNIT_Z * 30.0f;
//	NiPoint3 kOrgPt = m_pkActor->GetPosition() + NiPoint3::UNIT_Z * 30.0f;
//	NiPoint3 kDir = m_pkCamera->GetTranslate() - kOrgPt;
//	kDir.Unitize();
//	if(kPick.PickObjects(kOrgPt, kDir))
//	{
//		NiPick::Results &rkRes = kPick.GetResults();
//		for(unsigned int i = 0; i < rkRes.GetSize(); ++i)
//		{
//			NiPick::Record *pkRec = rkRes.GetAt(i);
//			NiAVObject *pkObject = pkRec->GetAVObject();
//			float fDistance = pkRec->GetDistance();
//			
//			// 원래 가리던 녀석이면 참조 카운트만 올리고
//			HiddenObjectContainer::iterator itr = m_kHiddenObjectContainer.find(pkObject);
//			if(itr != m_kHiddenObjectContainer.end())
//			{
//				itr->second.cRefCount = 1;
//			}
//			// 새로 들어온 녀석이면 숨기기 해준다.
//			else
//			{
//				HiddenObjectProperty kProp;
//				kProp.cRefCount = 1;
//
//				// 블렌딩 속성 장착
//				NiAlphaProperty *pkAlphaProp = NiNew NiAlphaProperty;
//				pkAlphaProp->SetAlphaBlending(true);
//				pkAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
//				pkAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
//				kProp.pkAlphaProperty = pkAlphaProp;
//
//				// 재질에서 Alpha 값 수정
//				NiGeometry *pkGeo = NiDynamicCast(NiGeometry, pkObject);
//				if(pkGeo)
//				{
//					kProp.fOldAlpha = pkGeo->GetPropertyState()->GetMaterial()->GetAlpha();
//					pkGeo->GetPropertyState()->GetMaterial()->SetAlpha(0.3f * (100-fDistance) / 100);
//				}
//
//				pkObject->AttachProperty(pkAlphaProp);
//				pkObject->UpdateProperties();
//				
//				m_kHiddenObjectContainer.insert(std::make_pair(pkObject, kProp));
//			}
//		}
//	}
//
//	// 참조 카운트가 없는 녀석들은 더 이상 가리지 않음
//	for(HiddenObjectContainer::iterator itr = m_kHiddenObjectContainer.begin();
//		itr != m_kHiddenObjectContainer.end(); )
//	{
//		if(itr->second.cRefCount == 0)
//		{
//			NiGeometry *pkGeo = NiDynamicCast(NiGeometry, itr->first);
//			if(pkGeo)
//			{
//				pkGeo->GetPropertyState()->GetMaterial()->SetAlpha(itr->second.fOldAlpha);
//			}
//
//			itr->first->DetachProperty(itr->second.pkAlphaProperty);
//			itr->first->UpdateProperties();
//			itr = m_kHiddenObjectContainer.erase(itr);
//			continue;
//		}
//		++itr;
//	}
//}

bool	PgCameraModeFollow::Input(PgInput *pkInput)
{
	if(!pkInput || !m_pkCamera || m_bFixedCamera || !g_pkLocalManager) return false;

	if(pkInput->Data()->GetDeviceID() != 1)
	{
		int const value = pkInput->Data()->GetDataValue();

		int xui_value = value;
		XUI::E_INPUT_EVENT_INDEX xui_event_type = XUI::IEI_NONE;

		POINT3I pt3Pos;

		unsigned int uiAppData = pkInput->Data()->GetAppData();

		NiPoint3	vCameraRight = m_pkCamera->GetWorldRightVector();
		NiPoint3	vCameraUp = m_pkCamera->GetWorldUpVector();
		NiPoint3	vCameraDir = m_pkCamera->GetWorldDirection();

		float	fDeltaValue = value - m_ptPrevMousePos.y;
		if(m_ptPrevMousePos.y == 0)
		{
			fDeltaValue = 0;
		}

		if(g_bUseDirectInput)
		{
			fDeltaValue = value;
		}

		

		bool bRightButtonDown = false;
		if(	g_pkLocalManager->GetInputSystem())
		{
			NiInputMouse* pkMouse = g_pkLocalManager->GetInputSystem()->GetMouse();
			if (pkMouse)
			{
				bRightButtonDown = pkMouse->ButtonIsDown(NiInputMouse::NIM_RIGHT);
			}
		}

		switch(uiAppData)
		{
			case NiAction::MOUSE_AXIS_X:
				break;
			case NiAction::MOUSE_AXIS_Y:
				m_ptPrevMousePos.y = value;
				if (bRightButtonDown)
				{
					ms_kFollowCameraInfo.fUpDelta += (float)fDeltaValue/20.0f * 0.1f;
					ms_kFollowCameraInfo.fUpDelta = NiClamp(ms_kFollowCameraInfo.fUpDelta, 0.0f, 1.0f);

					ms_kFollowCameraInfo.kCurrentEnd = PgCameraMan::Lerp(ms_CameraAreaInfo.kZoomOutCamera, ms_CameraAreaInfo.kUpCamera, ms_kFollowCameraInfo.fUpDelta);
					ms_kFollowCameraInfo.kCurrentPos = PgCameraMan::Lerp(ms_CameraAreaInfo.kZoomInCamera,ms_kFollowCameraInfo.kCurrentEnd, ms_kFollowCameraInfo.fZoomDelta);
				}
				break;
			case NiAction::MOUSE_AXIS_Z:				
				{
					ms_kFollowCameraInfo.fZoomDelta -= value/20 * 0.01f;
					ms_kFollowCameraInfo.fZoomDelta = NiClamp(ms_kFollowCameraInfo.fZoomDelta, m_kCameraAdjustInfo.fMinZoomDelta, m_kCameraAdjustInfo.fMaxZoomDelta);
					return	true;
				}
				break;
			case NiAction::MOUSE_BUTTON_LEFT:
				break;
			case NiAction::MOUSE_BUTTON_RIGHT:
				break;
			case NiAction::MOUSE_BUTTON_MIDDLE:
				break;
		};
		
	}
	else
	{
//		switch(pkInput->GetUKey()-1000)
//		{
//		};
	}

	return	false;
}

void PgCameraModeFollow::RefreshCameraPosition()
{
	PG_ASSERT_LOG(m_pkActor);
	if (m_pkActor == NULL)
		return;

	m_pkActor->FindPathNormal();
	NiPoint3 kActorPos = m_pkActor->GetPos();
#ifdef PG_USE_NEW_CAMERAWALL
	m_kTargetTrans = kActorPos;
#endif

	int bLoopCount = 0;
	bool bEndUpdate = false;
	bool oldHeightAdjust = g_bCameraHeightAdjust;
	bool oldSmoothMove = g_bCameraSmoothMove;
	bool oldUseCameraWall = g_bUseCameraWall;
	NiQuaternion oldCameraSpeed = g_kSpeed;

	g_kSpeed = g_kSpeed * 100.0f;
	g_bCameraSmoothMove = false;
	g_bCameraHeightAdjust = false;
	g_bUseCameraWall = false;

	do {
		bool bRetT = UpdateTranslate(0.1f);
		bool bRetR = UpdateRotate(0.1f);

		if (bRetT == false && bRetR == false)
			bEndUpdate = true;

		bLoopCount++;
	}
	while (bEndUpdate == false && bLoopCount < 100); // 카메라 코드를 잘 이해 못해서 땜빵으로 만들었음; !!!!

	g_bCameraHeightAdjust = oldHeightAdjust;
	g_bCameraSmoothMove = oldSmoothMove;
	g_bUseCameraWall = oldUseCameraWall;
	g_kSpeed = oldCameraSpeed;
	//NILOG(PGLOG_MINOR, "[PgCameraModeFollow] RefreshCameraPosition, after(%f,%f,%f) (%d)\n", m_pkCamera->GetWorldTranslate().x, m_pkCamera->GetWorldTranslate().y, m_pkCamera->GetWorldTranslate().z, bLoopCount);
}

void PgCameraModeFollow::SetActor(PgActor *pkActor)
{
	PG_ASSERT_LOG(pkActor);
	if (pkActor == NULL)
	{
		return;
	}

	NILOG(PGLOG_MINOR, "[PgCameraModeFollow] SetActor %s(%#X)\n", MB(pkActor->GetGuid().str()), this);
	m_pkActor = pkActor;
	m_pkActor->ResetLastFloorPos();

	m_kTargetTransOld = m_kActorPrevTrn = m_pkActor->GetPosition();
	m_kTargetFloorTrans = m_kTargetFloorTransOld = m_pkActor->GetLastFloorPos();

	PgWorld* pWorld = m_pkActor->GetWorld();
	if (pWorld)
	{
		releaseAllCamWalls();

		NiNode* walls = pWorld->GetCameraWallsRoot();
		recursiveProcessWalls(walls);
	}

	RefreshCameraPosition();
}

// adjustCameraByWall3(현좌표, 전좌표)
// 2007.11.02 강정욱
// 전 좌표에서 현 좌표까지 레이를 쏴서 카메라벽이 사이에 있다면.
// 현 좌표에서 카메라벽으로 레이를 다시 쏴서 거기서 (교점+전좌표쪽 향한 미약한 좌표)에
// 좌표를 지정한다.
NiPoint3 PgCameraModeFollow::adjustCameraByWall3(NiPoint3 const &rkPos, NiPoint3 const &rkPrevPos)
{
	if (g_bUseCameraWall == false)
		return rkPos;

	NiPoint3 kNewPos = rkPos;

	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);
	kPick.SetFrontOnly(false);
	kPick.SetPickType(NiPick::FIND_FIRST);

	PlaneMapContainer::iterator itr = m_kPlaneCont.begin();
	while(itr != m_kPlaneCont.end())
	{
		CameraWallInfo &kInfo = (*itr).second;
		NiPlane* pkPlane = kInfo.pkPlane;
		NiPoint3 const &rkNormal = pkPlane->GetNormal();
		NiPoint3 const kNormalEpsilon = rkNormal * PG_EPSILON;
		NiTriBasedGeom* pkTri = kInfo.pkTri;
		
		NiPoint3 const &rkCenter = pkTri->GetWorldBound().GetCenter();
		float fRadius = pkTri->GetWorldBound().GetRadius();
		if ((rkCenter-kNewPos).Length() > fRadius * 2)
		{
			++itr;
			continue;
		}
		kPick.SetTarget(pkTri);
		kPick.ClearResultsArray();

		kPick.PickObjects(rkPrevPos+rkNormal, -rkNormal, false);
		NiPick::Results& rkResults = kPick.GetResults();

		int result_old = pkPlane->WhichSide(rkPrevPos+rkNormal);
		int result_new = pkPlane->WhichSide(kNewPos);
		if (rkResults.GetSize() != 0 && result_old == NiPlane::POSITIVE_SIDE && result_new == NiPlane::NEGATIVE_SIDE)
		{
			NiPick::Record *pkOldRecord = rkResults.GetAt(0);
			float oldDistance = pkOldRecord->GetDistance();

			kPick.ClearResultsArray();
			kPick.PickObjects(kNewPos, rkNormal, true);
			NiPick::Results& rkNextResults = kPick.GetResults();
			if(rkNextResults.GetSize() != 0)
			{
				NiPick::Record *pkRecord = rkNextResults.GetAt(0);
				NiPoint3 kIntersection = (NiPoint3)pkRecord->GetIntersection();
				kNewPos = kIntersection;
			}
		}
		++itr;
	}

	return kNewPos;
}

NiPoint3 PgCameraModeFollow::adjustCameraByWall(NiPoint3 const &curPos)
{
	if (g_bUseCameraWall == false)
		return curPos;

	EnableWallByCharPos(curPos);

	for(PlaneMap::iterator itr = m_kPlanes.begin(); itr != m_kPlanes.end(); ++itr)
	{
		if (itr->second == NULL)
			continue;

		NiPlane* pPlane = itr->second;

		int result = pPlane->WhichSide(curPos);
		if (result == NiPlane::NEGATIVE_SIDE)
		{
			//! Find Collision Position in Plane
			float d = pPlane->GetNormal() * (curPos - (pPlane->GetNormal() * pPlane->GetConstant()));
			NiPoint3 pointOnPlane = curPos - (pPlane->GetNormal() * d);
			return pointOnPlane;
		}
	}
	return curPos;
}

NiPoint3 PgCameraModeFollow::adjustCameraByWall2(NiPoint3 const &actorPos)
{
	if (g_bUseCameraWall == false)
		return actorPos;

	//NiPoint3 actorPos = m_pkActor->GetWorldTranslate();
	PG_ASSERT_LOG(m_pkCamera);
	if (m_pkCamera == NULL || m_kPlanes.empty())
		return actorPos;

	EnableWallByCharPos(actorPos);

	D3DXMATRIX worldToCamMatrix(m_pkCamera->GetWorldToCameraMatrix());
	D3DXMATRIX camToWorldMatrix;
	if (D3DXMatrixInverse(&camToWorldMatrix, NULL, &worldToCamMatrix) == NULL)
		return actorPos;

	NiTransform kTransform;
	kTransform.m_Rotate = NiMatrix3(
		NiPoint3(camToWorldMatrix.m[0][0], camToWorldMatrix.m[0][1], camToWorldMatrix.m[0][2]),
		NiPoint3(camToWorldMatrix.m[1][0], camToWorldMatrix.m[1][1], camToWorldMatrix.m[1][2]),
		NiPoint3(camToWorldMatrix.m[2][0], camToWorldMatrix.m[2][1], camToWorldMatrix.m[2][2])
		);
	kTransform.m_Translate = NiPoint3(camToWorldMatrix.m[0][3], camToWorldMatrix.m[1][3], camToWorldMatrix.m[2][3]);
	for(PlaneMap::iterator itr = m_kPlanes.begin(); itr != m_kPlanes.end(); ++itr)
	{
		if (itr->second == NULL)
			continue;

		NiPlane* pPlane = itr->second;

		int result = pPlane->WhichSide(actorPos);
		if (result != NiPlane::NEGATIVE_SIDE)
		{
			//! Find Collision Position in Plane
			float d = pPlane->GetNormal() * (actorPos - (pPlane->GetNormal() * pPlane->GetConstant()));
			NiPoint3 pointOnPlane = actorPos - (pPlane->GetNormal() * d);
			//! Actor에서 가장 가까운 Plane의 점이 카메라에 보이면 안된다. 
			NiPoint3 screenPos;			
			if (m_pkCamera->WorldPtToScreenPt(pointOnPlane, screenPos.x, screenPos.y, screenPos.z) && (screenPos.x != 0.0f || screenPos.x != 1.0f))
			{				
				//! 어떻게 screenX와 screenY를 땡길것인가;
				screenPos.x = screenPos.x > 0.5f ? 1.0f : 0.0f;

				NiPoint3 targetPos = kTransform * screenPos;

				//NiPoint3 targetPos;
				//
				//float fW = screenPos.x * camToWorldMatrix.m[3][0] +
				//	screenPos.y * camToWorldMatrix.m[3][1] + screenPos.z * camToWorldMatrix.m[3][2] +
				//	camToWorldMatrix.m[3][3];

				//// Check to see if we're on the appropriate side of the camera.
				//if (fW > 0.00001f)
				//{
				//	float fInvW = 1.0f / fW;

				//	targetPos.x = screenPos.x * camToWorldMatrix.m[0][0] + screenPos.y * camToWorldMatrix.m[0][1] +
				//		screenPos.z * camToWorldMatrix.m[0][2] + camToWorldMatrix.m[0][3];
				//	targetPos.y = screenPos.x * camToWorldMatrix.m[1][0] + screenPos.y * camToWorldMatrix.m[1][1] +
				//		screenPos.z * camToWorldMatrix.m[1][2] + camToWorldMatrix.m[1][3];
				//	targetPos.z = screenPos.x * camToWorldMatrix.m[2][0] + screenPos.y * camToWorldMatrix.m[2][1] +
				//		screenPos.z * camToWorldMatrix.m[2][2] + camToWorldMatrix.m[2][3];

				//	targetPos = targetPos * fInvW;
				//}
				//else
				//{

				//}
				//NiPoint3 kOri;
				//NiPoint3 kDir;
				//m_pkCamera->ViewPointToRay(screenPos.x, screenPos.y, kOri, kDir);

				//kPick.ClearResultsArray();
				//kPick.SetObserveAppCullFlag(false);
				//kPick.SetSortType(NiPick::NO_SORT);
				//kPick.SetFrontOnly(true);
				//kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
				//kPick.SetPickType(NiPick::FIND_FIRST);
				//kPick.SetTarget(m_pkActor->GetWorld()->GetCameraWallsRoot());
				//if (kPick.PickObjects(kOri, kDir))
				//{
				//	NiPick::Results &rkRes = kPick.GetResults();
				//	for(unsigned int i = 0; i < rkRes.GetSize(); ++i)
				//	{
				//		NiPick::Record *pkRec = rkRes.GetAt(i);
				//		NiPoint3 targetPos = pkRec->GetIntersection();
				//	}
				//}

				//NILOG(PGLOG_MINOR, "[PgCameraModeFollow] adjustCameraByWall, camera is negative at (%f,%f,%f,%f) plane, prev(%f,%f,%f), adjust(%f,%f,%f), screen(%f,%f,%f) target(%f,%f,%f) cam(%f,%f,%f)\n",
				//	pPlane->GetNormal().x, pPlane->GetNormal().y, pPlane->GetNormal().z, pPlane->GetConstant(), actorPos.x, actorPos.y, actorPos.z, pointOnPlane.x, pointOnPlane.y, pointOnPlane.z,
				//	screenPos.x, screenPos.y, screenPos.z, targetPos.x, targetPos.y, targetPos.z, m_pkCamera->GetTranslate().x, m_pkCamera->GetTranslate().y, m_pkCamera->GetTranslate().z);
				return actorPos;// + NiPoint3(targetX * 100.0f, 0, 0);
			}
		}
		else
		{
			// TODO: 어떻게 커버하는게 좋을까?
			//PG_ASSERT_LOG(!"[PgCameraModeFollow] Can't come in Negative Side");
			/*NILOG(PGLOG_WARNING, "[PgCameraModeFollow] adjustCameraByWall, camera is negative at (%f,%f,%f,%f) plane, prev(%f,%f,%f)\n",
				pPlane->GetNormal().x, pPlane->GetNormal().y, pPlane->GetNormal().z, pPlane->GetConstant(), actorPos.x, actorPos.y, actorPos.z);*/
		}
	}
	return actorPos;
}

void PgCameraModeFollow::recursiveProcessWalls(NiAVObject* pkObject)
{
	if (pkObject == NULL)
		return;

	if (NiIsKindOf(NiTriBasedGeom, pkObject))
	{
		NiTriBasedGeom* triGeom = (NiTriBasedGeom*)pkObject;
		NiPoint3 tri1;
		NiPoint3 tri2;
		NiPoint3 tri3;
		NiPoint3* pTri1 = NULL;
		NiPoint3* pTri2 = NULL;
		NiPoint3* pTri3 = NULL;
		int triangleCount = triGeom->GetTriangleCount();
		triGeom->GetModelTriangle(0, pTri1, pTri2, pTri3);
		NiTransform kTransform = triGeom->GetWorldTransform();
		tri1 = kTransform * (*pTri1);
		tri2 = kTransform * (*pTri2);
		tri3 = kTransform * (*pTri3);
		NiPlane* pPlane = NiNew NiPlane(tri1, tri2, tri3);
		NiAVObject* pkParent = pkObject->GetParent();
		std::string kName = pkParent->GetName();	//TODO::map 말고 multimap으로 바꿀까?
		std::string kStaticName = kName.substr(0, 14);
		if (kStaticName == "c_wall_section" )
		{
			char const cNum = kName[16];
			int iNum = atoi(&cNum);
			if (iNum%2)
			{
				m_kPlanes.insert(std::make_pair( kName, pPlane));
			}
			else
			{
				m_kInactivePlanes.insert(std::make_pair( kName, pPlane));
			}
		}
		else
		{
#ifdef PG_USE_NEW_CAMERAWALL
			CameraWallInfo kInfo;
			kInfo.pkPlane = pPlane;
			kInfo.pkTri = triGeom;
			m_kPlaneCont.insert(std::make_pair( kName, kInfo ));
#endif
			m_kPlanes.insert(std::make_pair( kName, pPlane));
		}
	}

	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = (NiNode*)pkObject;
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if (pkChild)
			{
				recursiveProcessWalls(pkChild);
			}
		}
	}
}

NiPoint3 PgCameraModeFollow::GetActorPosition()
{
	if(!m_pkActor)
	{
		PG_ASSERT_LOG(m_pkActor);
		return NiPoint3::ZERO;
	}

	return m_pkActor->GetPosition();
}

bool PgCameraModeFollow::RemoveCameraWall(char const* szName)
{
	if(!szName)
	{
		PG_ASSERT_LOG(szName);
		return false;
	}

	if (m_kPlanes.empty())
	{
		return false;
	}

	PlaneMap::iterator it = m_kPlanes.find(std::string(szName));

	if (m_kPlanes.end() != it)
	{
		m_kRemovedPlanes.insert(std::make_pair(it->first, it->second));
		m_kPlanes.erase(it);
	}
	else	//InActive리스트에서도 찾아보자.
	{
		it = m_kInactivePlanes.find(std::string(szName));
		if (m_kInactivePlanes.end() != it)
		{
			m_kRemovedPlanes.insert(std::make_pair(it->first, it->second));
			m_kInactivePlanes.erase(it);
		}
	}

	return true;
}

bool PgCameraModeFollow::RecoverCameraWall(char const* szName)
{
	if(!szName)
	{
		PG_ASSERT_LOG(szName);
		return false;
	}

	if (m_kRemovedPlanes.empty())
	{
		return false;
	}

	PlaneMap::iterator it = m_kRemovedPlanes.find(std::string(szName));

	if (m_kRemovedPlanes.end() != it)
	{
		m_kPlanes.insert(std::make_pair(it->first, it->second));
		m_kRemovedPlanes.erase(it);
	}

	return true;
}

void PgCameraModeFollow::EnableWallByCharPos(NiPoint3 const &curPos)
{
	if (g_bUseCameraWall == false)
	{
		return;
	}

	for(PlaneMap::iterator itr = m_kInactivePlanes.begin(); itr != m_kInactivePlanes.end(); )
	{
		if (itr->second == NULL)
		{
			++itr;
			continue;
		}

		NiPlane* pPlane = itr->second;

		int result = pPlane->WhichSide(curPos);
		
		if (result == NiPlane::POSITIVE_SIDE)
		{
			m_kPlanes.insert(std::make_pair(itr->first, itr->second));
			itr = m_kInactivePlanes.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

void PgCameraModeFollow::releaseAllCamWalls()
{
	for(PlaneMap::iterator itr = m_kPlanes.begin(); itr != m_kPlanes.end(); ++itr)
		SAFE_DELETE_NI(itr->second);
	for(PlaneMap::iterator itr = m_kRemovedPlanes.begin(); itr != m_kRemovedPlanes.end(); ++itr)
		SAFE_DELETE_NI(itr->second);
	for(PlaneMap::iterator itr = m_kInactivePlanes.begin(); itr != m_kInactivePlanes.end(); ++itr)
		SAFE_DELETE_NI(itr->second);

	m_kPlanes.clear();
	m_kRemovedPlanes.clear();
	m_kInactivePlanes.clear();
#ifdef PG_USE_NEW_CAMERAWALL
	m_kPlaneCont.clear();
#endif
}

void PgCameraModeFollow::SetHideObject(NiAVObject* pkObject, float _fAlpha, HiddenObjectPropertyList& _kPropertyList)
{
 	float fAlpha = _fAlpha;
	if(fAlpha < 0)
		fAlpha = 0;

	// 재질에서 Alpha 값 수정
	//NiGeometry *pkGeo = NiDynamicCast(NiGeometry, pkObject);
	NiMaterialProperty* pProp = NiDynamicCast(NiMaterialProperty, pkObject->GetProperty(NiMaterialProperty::GetType()));
	if(_kPropertyList.byStep == 2)
	{
		_kPropertyList.byStep = 1;
	}
	else if(_kPropertyList.byStep == 3)
	{
	}
	// 새로 바꿀 알파값이 더 커야한다.
	// 동일한 메터리얼에는 한번만 셋팅한다.
	else if(pProp && fAlpha < pProp->GetAlpha() && (!FindHiddenObjectMeterial(_kPropertyList.kMeterialContainer, pProp->GetName()) ) )
	{
		AddHiddenObjectMeterial(_kPropertyList.kMeterialContainer, pProp->GetName());

		HiddenObjectProperty kProp;

		// 블렌딩 속성 장착
		NiAlphaProperty* pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkObject->GetProperty(NiAlphaProperty::GetType()));
		if(!pkAlphaProp)
		{
			kProp.fOldAlpha = pProp->GetAlpha();
			// 알파 속성이 없으므로 새로 만든다
			pkAlphaProp = NiNew NiAlphaProperty;

			pkAlphaProp->SetAlphaBlending(true);
			pkAlphaProp->SetAlphaTesting(true);
			pkAlphaProp->SetTestRef(0);
			pkAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
			pkAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
			pkAlphaProp->SetAlphaGroup(AG_OBSTRUCTOR);
			//pkAlphaProp->SetNoSorter(false);

			// 알파 속성이 없었다는것을 저장
			kProp.pkAlphaProperty = NULL;
			//kProp.fOldAlpha = pkGeo->GetPropertyState()->GetMaterial()->GetAlpha();
			pkObject->AttachProperty(pkAlphaProp);
		}
		else
		{
			kProp.fOldAlpha = pProp->GetAlpha();
			kProp.pkAlphaProperty = pkAlphaProp;
			//kProp.fOldAlpha = pkGeo->GetPropertyState()->GetMaterial()->GetAlpha();

			NiAlphaProperty	*pkAlphaNew = NiNew NiAlphaProperty();
			
			pkAlphaNew->SetAlphaBlending(true);
			pkAlphaNew->SetAlphaTesting(true);
			pkAlphaNew->SetTestRef(0);
			pkAlphaNew->SetTestMode(NiAlphaProperty::TEST_GREATER);
			pkAlphaNew->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
			pkAlphaNew->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
			pkAlphaNew->SetAlphaGroup(AG_OBSTRUCTOR);

			pkObject->DetachProperty(pkAlphaProp);
			pkObject->AttachProperty(pkAlphaNew);
			//pkAlphaProp->SetNoSorter(false);
		}

		pkObject->UpdateProperties();
		//pkGeo->GetPropertyState()->GetMaterial()->SetAlpha(0.3f);
		//pProp->SetAlpha(fAlpha);// UpdateHideObjectAlpha 함수를 사용하는 경우 주석 처리
		kProp.fNewAlpha = fAlpha;
		_kPropertyList.kObjectContainer.insert(std::make_pair(pkObject, kProp));
	}

	NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
	if(pkNode)
	{
		for(unsigned int un = 0; un < pkNode->GetArrayCount(); ++un)
		{
			NiAVObject* pkChild = pkNode->GetAt(un);
			if (pkChild)
				SetHideObject(pkChild, fAlpha, _kPropertyList);// 재귀
		}
	}
}

void PgCameraModeFollow::UnsetHideObject()
{
	for(HiddenObjectContainerList::iterator itrList = m_kHiddenObjectContainerList.begin();
		itrList != m_kHiddenObjectContainerList.end() ; )
	{
		// 참조 카운트가 없는 녀석들은 더 이상 가리지 않음
		//if(itrList->second.bRef == false)
		if(itrList->second.byStep == 3)
		{
			HiddenObjectContainer kHidden = itrList->second.kObjectContainer;

			for(HiddenObjectContainer::iterator itr = kHidden.begin() ; itr != kHidden.end() ; ++itr)
			{
				//NiGeometry *pkGeo = NiDynamicCast(NiGeometry, itr->first);
				NiMaterialProperty* pProp = NiDynamicCast(NiMaterialProperty, itr->first->GetProperty(NiMaterialProperty::GetType()));
				if(pProp && FindHiddenObjectMeterial(itrList->second.kMeterialContainer, pProp->GetName()))
				{
					DeleteHiddenObjectMeterial(itrList->second.kMeterialContainer, pProp->GetName());

					NiAlphaPropertyPtr pkAlphaProp = NiDynamicCast(NiAlphaProperty, itr->first->GetProperty(NiAlphaProperty::GetType()));
					if(pkAlphaProp)
					{
						itr->first->DetachProperty(pkAlphaProp);	//	임시로 붙인거 일단 떼고,

						if(itr->second.pkAlphaProperty)
						{
							// 원래의 알파속성을 복구
							itr->first->AttachProperty(itr->second.pkAlphaProperty);
							//pkAlphaProp->SetAlphaBlending(itr->second.pkAlphaProperty->GetAlphaBlending());
							//pkAlphaProp->SetSrcBlendMode(itr->second.pkAlphaProperty->GetSrcBlendMode());
							//pkAlphaProp->SetDestBlendMode(itr->second.pkAlphaProperty->GetDestBlendMode());
							//pkAlphaProp->SetAlphaTesting(itr->second.pkAlphaProperty->GetAlphaTesting());
							//pkAlphaProp->SetTestMode(itr->second.pkAlphaProperty->GetTestMode());
							//pkAlphaProp->SetTestRef(itr->second.pkAlphaProperty->GetTestRef());
							//pkAlphaProp->SetAlphaGroup(itr->second.pkAlphaProperty->GetAlphaGroup());
							itr->second.pkAlphaProperty = NULL;
						}
						else
						{
							// 원래 알파 속성이 없었다
							//	itr->first->DetachProperty(pkAlphaProp);
						}
						pkAlphaProp = NULL;

					}
					
					itr->first->UpdateProperties();
					pProp->SetAlpha(itr->second.fOldAlpha);
				}
			}
			
			kHidden.clear();
			m_kHiddenObjectContainerList.erase(itrList++);
			continue;
		}

		++itrList;
	}
}



NiAVObject* PgCameraModeFollow::FindHideObjectRoot(NiAVObject* pkObject)
{
	if(pkObject == NULL)
	{
		return NULL;
	}
	else
	{
		NiAVObject* pkParent = pkObject->GetParent();
		if(pkParent == NULL)
		{
			return NULL;
		}
		else
		{
			NiFixedString strName = pkParent->GetName();// 부모의 이름을 비교
			if(strName.Equals("SELECTIVE_NODE_ROOT") || strName.Equals("DYNAMIC_NODE_ROOT") || strName.Equals("STATIC_NODE_ROOT") || strName.Equals("ROOM_GROUP_ROOT"))
			{
				return pkObject;// 찾았다
			}
			else
			{
				return FindHideObjectRoot(pkParent);// 재귀
			}
		}
	}
}



bool PgCameraModeFollow::IsHideObjectSearchParentNode(NiAVObject* pkParent)
{
	HiddenObjectContainerList::iterator itr = m_kHiddenObjectContainerList.begin();
	for( ; itr != m_kHiddenObjectContainerList.end(); ++itr)
	{
		//if(itr->second.pParentNode == pkParent)
		if(itr->second.pParentNode->GetName() == pkParent->GetName())
			return true;
	}

	return false;
}
void PgCameraModeFollow::AddHiddenObjectMeterial( HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName)
{
	_kMeterial.push_back(_strName);
}
void PgCameraModeFollow::DeleteHiddenObjectMeterial( HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName)
{
	HiddenObjectMeterialContainer::iterator itr = _kMeterial.begin();
	for( ; itr != _kMeterial.end() ; ++itr )
	{
		if(_strName.Equals(*itr))
		{
			_kMeterial.erase(itr);
			return;
		}
	}
}
bool PgCameraModeFollow::FindHiddenObjectMeterial( HiddenObjectMeterialContainer& _kMeterial, NiFixedString _strName)
{
	HiddenObjectMeterialContainer::iterator itr = _kMeterial.begin();
	for( ; itr != _kMeterial.end() ; ++itr )
	{
		if(_strName.Equals(*itr))
			return true;
	}

	return false;
}

float const DECRESS_ALPHA_VALUE_PER_SECOND = 1.0f;
void PgCameraModeFollow::UpdateHideObjectAlpha(float _fDeltaTime)
{
	HiddenObjectContainerList::iterator itrList = m_kHiddenObjectContainerList.begin();
	for( ; itrList != m_kHiddenObjectContainerList.end() ; ++itrList)
	{
		HiddenObjectContainer::iterator itr = itrList->second.kObjectContainer.begin();
		for( ; itr != itrList->second.kObjectContainer.end(); ++itr)
		{
			NiMaterialProperty* pkMeterialProp = NiDynamicCast(NiMaterialProperty, itr->first->GetProperty(NiMaterialProperty::GetType()));
			if(pkMeterialProp)
			{
				float fAlpha = pkMeterialProp->GetAlpha();

				//if(itrList->second.bRef == true)
				if(itrList->second.byStep == 1)

				{
					if(fAlpha <= itr->second.fNewAlpha)
						continue;

					fAlpha = fAlpha - (_fDeltaTime / DECRESS_ALPHA_VALUE_PER_SECOND);
					if(fAlpha <= itr->second.fNewAlpha)
						fAlpha = itr->second.fNewAlpha;
				}
				else if(itrList->second.byStep == 2)
				{
					fAlpha = fAlpha + (_fDeltaTime / DECRESS_ALPHA_VALUE_PER_SECOND);
					if(fAlpha >= itr->second.fOldAlpha)
					{
						fAlpha = itr->second.fOldAlpha;
						itrList->second.byStep = 3;
					}
				}

				pkMeterialProp->SetAlpha(fAlpha);

				NiAVObject	*pkObject = itr->first;
				NiGeometry *pkGeom = NiDynamicCast(NiGeometry,pkObject);
				if(pkGeom)
				{
					pkGeom->SetNeedUpdateAllShaderConstant(true);
				}

				pkObject->UpdateProperties();
			}
		}
	}
}


void PgCameraModeFollow::SetCameraAdjustCameraInfo(NiPoint3 const kPosInfo, NiPoint3 const kLookAtInfo)
{// 위치값, Lookat을 타겟의 좌표와 합해 상대적인, 위치와 LookAt을 구한다
	CameraInfo& kCamAdInfo  = GetCameraAdjustInfo().kCameraAdjust;
	
	//위치
	kCamAdInfo.fCameraWidth		   = kPosInfo.x;	
	kCamAdInfo.fCameraHeight	   = kPosInfo.z;
	kCamAdInfo.fDistanceFromTarget = kPosInfo.y;	

	//LookAt
	kCamAdInfo.fLookAtWidth  = kLookAtInfo.x;
	kCamAdInfo.fLookAtHeight = kLookAtInfo.z;
	kCamAdInfo.fLookAtDepth  = kLookAtInfo.y;	
}
