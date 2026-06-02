#include "StdAfx.h"
#include "PgCameraMan.h"
#include "PgCameraModeFollow.h"
#include "PgCameraModeBox.h"
#include "PgCameraModeFollow2.h"
#include "PgCameraModeMovie.h"
#include "PgCameraModeCircle.h"
#include "PgCameraModeScript.H"
#include "PgCameraModeFree.H"
#include "PgCameraModeEvent.H"
#include "PgCameraModeAni.H"
#include "PgInput.h"
#include "PgOption.h"
#include "PgPilotMan.h"

float PgCameraMan::m_fJumpHeight = 100.0f;
QuakeCameraInfo g_kQuakeCameraInfo;

// 정보 출력용;
NiPoint3 g_kCameraTrn;
NiQuaternion g_kCameraRot;

CameraInfo PgCameraMan::Lerp(CameraInfo &rkPosA, CameraInfo &rkPosB, float fPerc)
{
	return CameraInfo(
		rkPosA.fDistanceFromTarget + fPerc * (rkPosB.fDistanceFromTarget - rkPosA.fDistanceFromTarget),
		rkPosA.fCameraHeight + fPerc * (rkPosB.fCameraHeight - rkPosA.fCameraHeight),
		rkPosA.fLookAtHeight + fPerc * (rkPosB.fLookAtHeight - rkPosA.fLookAtHeight)
		);
}

//! 한번만 부르자.
bool PgCameraMan::ParseXml(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);

	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'CAMERA'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	PG_ASSERT_LOG(strcmp(pkElement->Value(), "CAMERA") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if (strcmp(pcTagName, "ITEM") == 0)
		{
			CameraInfo kInfo;
			char const* pcType = NULL;

			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "TYPE") == 0)
				{
					pcType = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "CAMERA_DISTANCE") == 0)
				{
					kInfo.fDistanceFromTarget = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "CAMERA_Z") == 0)
				{
					kInfo.fCameraHeight= (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "TARGET_Z") == 0)
				{
					kInfo.fLookAtHeight = (float)atof(pcAttrValue);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			if(!pcType)
			{
				PgXmlError(pkElement, "Not Enough Camera Data");
			}
			else
			{
				if (_strnicmp(pcType, "DEFAULT", strlen("DEFAULT")) == 0)
				{
					PgCameraModeFollow::ms_kDefaultCamera = kInfo;
				}
				else if (_strnicmp(pcType, "FIGHTER", strlen("FIGHTER")) == 0)
				{
					PgCameraModeFollow::ms_kClassDefaultCamera[0] = kInfo;
				}
				else if (_strnicmp(pcType, "MAGICIAN", strlen("MAGICIAN")) == 0)
				{
					PgCameraModeFollow::ms_kClassDefaultCamera[1] = kInfo;
				}
				else if (_strnicmp(pcType, "ARCHER", strlen("ARCHER")) == 0)
				{
					PgCameraModeFollow::ms_kClassDefaultCamera[2] = kInfo;
				}
				else if (_strnicmp(pcType, "THIEF", strlen("THIEF")) == 0)
				{
					PgCameraModeFollow::ms_kClassDefaultCamera[3] = kInfo;
				}				
				else if (_strnicmp(pcType, "ZOOMOUT", strlen("ZOOMOUT")) == 0)
				{
					PgCameraModeFollow::ms_CameraAreaInfo.kZoomOutCamera = kInfo;
				}
				else if (_strnicmp(pcType, "UP", strlen("UP")) == 0)
				{
					PgCameraModeFollow::ms_CameraAreaInfo.kUpCamera = kInfo;
				}
				else if (_strnicmp(pcType, "ZOOMIN", strlen("ZOOMIN")) == 0)
				{
					PgCameraModeFollow::ms_CameraAreaInfo.kZoomInCamera = kInfo;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Camera type '%s'", pcType);
				}
			}
		}
		else if (strcmp(pcTagName, "VALUE") == 0)
		{
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "JUMP_HEIGHT") == 0)
				{
					m_fJumpHeight = (float)atof(pcAttrValue);
					PG_ASSERT_LOG(m_fJumpHeight > 0);
				}

				pkAttr = pkAttr->Next();
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	PgCameraModeFollow::SetDefaultCameraInfo();

	return true;
}

void PgCameraMan::ResetCameraPos()
{
	PG_ASSERT_LOG(m_pkCameraMode);
	if (m_pkCameraMode == NULL)
		return;

	m_pkCameraMode->ResetCameraPos();
}

void PgCameraMan::SaveCameraPos()
{
	PG_ASSERT_LOG(m_pkCameraMode);
	if (m_pkCameraMode == NULL)
		return;

	m_pkCameraMode->SaveCameraPos();
}

void PgCameraMan::RestoreCameraPos()
{
	PG_ASSERT_LOG(m_pkCameraMode);
	if (m_pkCameraMode == NULL)
		return;

	m_pkCameraMode->RestoreCameraPos();
}

void PgCameraMan::RefreshCamera()
{
	PG_ASSERT_LOG(m_pkCameraMode);
	if (m_pkCameraMode == NULL)
		return;

	if (GetCameraModeE() == CMODE_FOLLOW || GetCameraModeE() == CMODE_FOLLOW_DEPTH_PATH)
	{
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_pkCameraMode);
		if (pkCameraMode)
			pkCameraMode->RefreshCameraPosition();
	}
}

void PgCameraMan::NotifyActorRemove(PgActor* pkActor)
{
	if (m_pkCameraMode == NULL || pkActor == NULL)
		return;

	m_pkCameraMode->NotifyActorRemove(pkActor);
}

PgCameraMan::PgCameraMan()
{
	m_spCamera = 0;
	m_eCameraMode = CMODE_NONE;
	m_pkCameraMode = 0;
	
	m_vCamVelocity = NiPoint3(0,0,0);
	m_kReservedCameraPosition = NiPoint3(0,0,0);
	m_bQuakeCamera = false;
	LockInput(false);
}

PgCameraMan::~PgCameraMan()
{
	// 아래 두줄 leesg213 추가 2006-11-21
	// 카메라 객체 메모리 해제를 해주기 위함.
	SAFE_DELETE_NI(m_pkCameraMode);
}

NiCameraPtr PgCameraMan::GetCamera()
{
	return m_spCamera;
}

void PgCameraMan::SetCamera(NiCameraPtr spCamera)
{
	m_spCamera = spCamera;
	if(m_spCamera)
	{
		m_spCamera->UpdateProperties();
		m_spCamera->UpdateEffects();
		m_spCamera->Update(0);
	}
}

void PgCameraMan::SetCamera(NiString& rkCameraName)
{
	NiCameraPtr spCamera;

	if(m_kContainer.GetAt(rkCameraName, spCamera))
	{
		m_spCamera = spCamera;
	}
}

void PgCameraMan::AddCamera(NiString& rkCameraName, NiCameraPtr spCamera)
{
	if(spCamera)
	{
		spCamera->UpdateProperties();
		spCamera->UpdateEffects();
		spCamera->Update(0);
	}

	m_kContainer.SetAt(rkCameraName, spCamera);
}

void PgCameraMan::DeleteCamera(NiString& rkCameraName)
{
	m_kContainer.RemoveAt(rkCameraName);
}

void PgCameraMan::Update(float fFrameTime)
{
	// 카메라의 위치를 선택된 모드에 맞게 갱신한다.
	if (m_spCamera == NULL)
		return;

	NiPoint3 vCamWorldPos = m_spCamera->GetWorldLocation();
	m_vCamVelocity = NiPoint3::ZERO;

	if (m_pkCameraMode)
	{
		m_pkCameraMode->SetCamera(m_spCamera);	//	Set the camera every each time
		m_pkCameraMode->Update(fFrameTime);
	}

	m_spCamera->SetViewFrustum(g_kFrustum);
	
	m_spCamera->Update(fFrameTime);

	if(m_pkCameraMode && m_pkCameraMode->Update(fFrameTime))
	{
		m_vCamVelocity = m_spCamera->GetWorldLocation() - vCamWorldPos;

		//무비카메라일때만
		if(m_eCameraMode == CMODE_MOVIE && m_pkCameraMode)
		{
			PgCameraModeMovie* pkCamera = dynamic_cast<PgCameraModeMovie *>(m_pkCameraMode);

			//애니가 끝났으면 애니 시퀀스 초기화
			if( pkCamera->IsAnimationDone() )	
			{
				m_PastCameraAniID = NiActorManager::INVALID_SEQUENCE_ID;
				m_szNextScript.clear();
				
				//! 무비 종료되었으므로, HP 보이기 옵션 복구한다
				g_kGlobalOption.RestoreDisplayHP();
			}
		}

		// 사운드 리스너를 카메라에 맞게 갱신한다.	
		NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
		if(pkSS && pkSS->GetHWnd())
		{
			NiMilesListener *pkListener = pkSS->GetListener();

			pkListener->SetUpVector(m_spCamera->GetWorldUpVector());
			pkListener->SetDirectionVector(m_spCamera->GetWorldDirection());
			//pkListener->SetTranslate(m_spCamera->GetTranslate() + m_spCamera->GetWorldDirection() * 380.0f);
			
			// Listener의 위치는 신중하게 정해야 함.
			// Follow Mode일 때는 를 캐릭터위치에 두고, Movie Mode나 Free Mode일 때는 카메라에 위치시킨다.
			NiPoint3 kListenerPosition;
			NiPoint3 const kAdjustPos = m_spCamera->GetWorldDirection() * 20.0f;
			
			switch(m_eCameraMode)
			{
			case CMODE_FOLLOW:
				{
					kListenerPosition = ((PgCameraModeFollow *)m_pkCameraMode)->GetActorPosition() + kAdjustPos;
				}break;
			case CMODE_CIRCLE:
				{
					kListenerPosition = ((PgCameraModeCircle *)m_pkCameraMode)->GetActorPosition() + kAdjustPos;
				}break;
			case CMODE_BOX:
				{
					kListenerPosition = ((PgCameraModeBox *)m_pkCameraMode)->GetActorPosition() + kAdjustPos;
				}break;
			case CMODE_EVENT:
			case CMODE_FREE:
			case CMODE_MOVIE:
			case CMODE_SCRIPT:
				{
					kListenerPosition = m_spCamera->GetWorldTranslate() + m_spCamera->GetWorldDirection() * 380.0f;
				}break;
			case CMODE_ANIMATION:
				{
					if (((PgCameraModeAni*)m_pkCameraMode)->GetUseActorPositionAsSoundListener() && 
						((PgCameraModeAni*)m_pkCameraMode)->GetActorRoot() != NULL)
					{
						kListenerPosition = ((PgCameraModeAni*)m_pkCameraMode)->GetActorRoot()->GetWorldTranslate();
					}
					else
					{
						kListenerPosition = m_spCamera->GetWorldTranslate();
					}
				}break;
			case CMODE_FOLLOW2:		// 현재 FOLLOW2모드는 안씀
			default:
				// 카메라가 없으면 Listener는 0, 0, 0 ?
				break;
			}
			
			pkListener->SetWorldTranslate(kListenerPosition);
			pkListener->Update();
		}
	}
	
	switch(m_eCameraMode)
	{
	case CMODE_NONE:
	case CMODE_STOP:
		{// 사운드 리스너를 캐릭터에게 맞춘다.
			NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
			if(pkSS && pkSS->GetHWnd())
			{
				PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
				if( pkMyActor )
				{
					NiMilesListener *pkListener = pkSS->GetListener();
					if( pkListener )
					{
						pkListener->SetUpVector(m_spCamera->GetWorldUpVector());
						pkListener->SetDirectionVector(m_spCamera->GetWorldDirection());

						NiPoint3 kListenerPosition = pkMyActor->GetWorldTranslate();

						pkListener->SetWorldTranslate(kListenerPosition);
						pkListener->Update();
					}
				}
			}	
		}break;
	}
	
	// 지진이 있다면
	if (g_kQuakeCameraInfo.fQuakeTime == 0.0f && g_kQuakeCameraInfo.fQuakeDuring > 0.0f && m_bQuakeCamera == false && g_kQuakeCameraInfo.iQuakeType >= 0)
	{
		//! 지진의 시작
		m_kReservedCameraPosition = m_spCamera->GetTranslate();
		m_bQuakeCamera = true;
		//_PgOutputDebugString("Camera Reserved %f, %f, %f\n", m_kReservedCameraPosition.x, m_kReservedCameraPosition.y, m_kReservedCameraPosition.z);
	}
	if(g_kQuakeCameraInfo.fQuakeTime < g_kQuakeCameraInfo.fQuakeDuring)
	{
		// 지진을 적용한다.
		NiPoint3 kTrn = (((m_eCameraMode == CMODE_FOLLOW || m_eCameraMode == CMODE_BOX || m_eCameraMode == CMODE_CIRCLE || m_eCameraMode == CMODE_MOVIE)) ? m_spCamera->GetTranslate() : m_kReservedCameraPosition);
		
		float fRealFactor = g_kQuakeCameraInfo.fQuakeFactor;
		if(g_pkWorld && 0!=g_pkWorld->GetUpdateSpeed())
		{
			fRealFactor*=g_pkWorld->GetUpdateSpeed();//슬로우모션일때 보정
		}
		g_kQuakeCameraInfo.fQuakeTime += fFrameTime;

		NiPoint3 kTrnDiff = NiPoint3(0,0,0);
		float fMod = 0.0f;
		float sinValue = 0.0f;
		float diff = 0.0f;
		switch(g_kQuakeCameraInfo.iQuakeType)
		{
		case 4:
			{
				fMod = NiFmod((g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring) * g_kQuakeCameraInfo.iCycleCount, 1.0f);
				sinValue = (g_kQuakeCameraInfo.bPlus ? 1.0f : -1.0f) * sin(NI_PI * g_kQuakeCameraInfo.fFrequency * fMod);
				diff =  sinValue * fRealFactor;
				kTrnDiff.y += diff;
				//PgOutputPrint3("QuakeCamera y %f, %f, %f\n", fMod, sinValue, diff);
			}break;
		case 3:
			{
				fMod = NiFmod((g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring) * g_kQuakeCameraInfo.iCycleCount, 1.0f);
				sinValue = (g_kQuakeCameraInfo.bPlus ? 1.0f : -1.0f) * sin(NI_PI * g_kQuakeCameraInfo.fFrequency * fMod);
				diff =  sinValue * fRealFactor;
				kTrnDiff.z += diff;
				//PgOutputPrint3("QuakeCamera z %f, %f, %f\n", fMod, sinValue, diff);
			}break;
		case 2:
			{
				fMod = NiFmod((g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring) * g_kQuakeCameraInfo.iCycleCount, 1.0f);
				sinValue = (g_kQuakeCameraInfo.bPlus ? 1.0f : -1.0f) * sin(NI_PI * g_kQuakeCameraInfo.fFrequency * fMod);
				diff =  sinValue * fRealFactor;
				kTrnDiff.x += diff;
				//PgOutputPrint3("QuakeCamera x %f, %f, %f\n", fMod, sinValue, diff);
			}break;
		case 1:
		default:
			{
				kTrnDiff.x += sin(NI_PI * (NiRand() % 2 ? 1 : -1) * (g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring)) * fRealFactor;
				kTrnDiff.y += sin(NI_PI * (NiRand() % 2 ? 1 : -1) * (g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring)) * fRealFactor;
				kTrnDiff.z += sin(NI_PI * (NiRand() % 2 ? 1 : -1) * (g_kQuakeCameraInfo.fQuakeTime/g_kQuakeCameraInfo.fQuakeDuring)) * fRealFactor;
			}break;
		}		

		NiPoint3 xDir = m_spCamera->GetWorldRightVector();
		NiPoint3 yDir = m_spCamera->GetWorldDirection();
		NiPoint3 zDir = m_spCamera->GetWorldUpVector();
		xDir.Unitize();
		yDir.Unitize();
		zDir.Unitize();		
		m_spCamera->SetTranslate(kTrn + xDir * kTrnDiff.x + yDir * kTrnDiff.y + zDir * kTrnDiff.z);
	}
	else
	{
		ClearQuake();
	}
	
	// 정보 출력용으로 카메라의 위치를 저장해둔다.
	g_kCameraTrn = m_spCamera->GetTranslate();
	m_spCamera->GetRotate(g_kCameraRot);
}

void PgCameraMan::ClearQuake()
{
	if (m_bQuakeCamera)
	{
		//! 지진 끝
		g_kQuakeCameraInfo.fQuakeTime = 0.0f;
		g_kQuakeCameraInfo.fQuakeDuring = 0.0f;
		if (m_eCameraMode != CMODE_FOLLOW && m_eCameraMode != CMODE_BOX && m_eCameraMode != CMODE_CIRCLE && m_eCameraMode != CMODE_MOVIE)
			m_spCamera->SetTranslate(m_kReservedCameraPosition);
		//_PgOutputDebugString("Camera Reserved return %f, %f, %f\n", m_kReservedCameraPosition.x, m_kReservedCameraPosition.y, m_kReservedCameraPosition.z);
		m_bQuakeCamera = false;
	}
}

void	PgCameraMan::QuakeCamera(float fDuring, float fFactor)
{
	ClearQuake();

	g_kQuakeCameraInfo.fQuakeDuring = fDuring;
	g_kQuakeCameraInfo.fQuakeFactor = fFactor;
	g_kQuakeCameraInfo.fQuakeTime = 0.0f;
}

PgICameraMode *PgCameraMan::GetCameraMode()
{
	return m_pkCameraMode;
}

void PgCameraMan::SetCameraMode(ECameraMode eCameraMode, PgActor *pkActor, NiActorManagerPtr spAM)
{
	if(m_eCameraMode != CMODE_NONE)
	{
		if(CMODE_MOVIE==m_eCameraMode)
		{
			g_kGlobalOption.RestoreDisplayHP();
		}
		SAFE_DELETE_NI(m_pkCameraMode);
		m_pkCameraMode = 0;
	}
	
	switch(eCameraMode)
	{
	case CMODE_NONE:
		{
			m_pkCameraMode = 0;
			m_PastCameraAniID = NiActorManager::INVALID_SEQUENCE_ID;	//애니 시퀀스 초기화
			m_szNextScript.clear();
		}break;
	case CMODE_FOLLOW:
	case CMODE_FOLLOW_DEPTH_PATH:
		{
			PG_ASSERT_LOG(pkActor);
			m_pkCameraMode = NiNew PgCameraModeFollow(m_spCamera, pkActor);
			m_pkCameraMode->Update(0.0f);
			m_PastCameraAniID = NiActorManager::INVALID_SEQUENCE_ID;	//애니 시퀀스 초기화
			m_szNextScript.clear();
			if(pkActor != NULL)
				((PgCameraModeFollow*)m_pkCameraMode)->SetActor(pkActor);

			if (eCameraMode == CMODE_FOLLOW)
				((PgCameraModeFollow*)m_pkCameraMode)->SetCameraMoveType(PgCameraModeFollow::MOVETYPE_DEPTH_FOLLOW_ACTOR);
			else if (eCameraMode == CMODE_FOLLOW_DEPTH_PATH)
				((PgCameraModeFollow*)m_pkCameraMode)->SetCameraMoveType(PgCameraModeFollow::MOVETYPE_DEPTH_FOLLOW_PATH);

			((PgCameraModeFollow*)m_pkCameraMode)->UpdateMinMaxCameraZoomInfo();
		}break;

	case CMODE_BOX:
		{
			PG_ASSERT_LOG(pkActor);
			m_pkCameraMode = NiNew PgCameraModeBox(m_spCamera, pkActor);
			m_pkCameraMode->Update(0.0f);
			m_PastCameraAniID = NiActorManager::INVALID_SEQUENCE_ID;	//애니 시퀀스 초기화
			m_szNextScript.clear();
			if(pkActor != NULL)
				((PgCameraModeBox*)m_pkCameraMode)->SetActor(pkActor);
		}break;
	case CMODE_STOP:
	case CMODE_FOLLOW2:
		{
			PG_ASSERT_LOG(pkActor);
			m_pkCameraMode = NiNew PgCameraModeFollow2(m_spCamera, pkActor);
			m_pkCameraMode->Update(0.0f);
			m_PastCameraAniID = NiActorManager::INVALID_SEQUENCE_ID;	//애니 시퀀스 초기화
			m_szNextScript.clear();
		}break;
	case CMODE_FREE:
		{
			m_pkCameraMode = NiNew PgCameraModeFree(m_spCamera);
			m_pkCameraMode->Update(0.0f);
		}break;
	case CMODE_MOVIE:
		{
			PG_ASSERT_LOG(spAM);
			m_pkCameraMode = NiNew PgCameraModeMovie(m_spCamera, spAM);
			PgCameraModeMovie *pMovieMode = dynamic_cast<PgCameraModeMovie*>(m_pkCameraMode);
			pMovieMode->SetTargetAnimation(m_PastCameraAniID, m_szNextScript.c_str());
			pMovieMode->Update(0.0f);
			
			//! 무비 카메라는 HP Display Option을 끈다.
			g_kGlobalOption.OffDisplayHP();
		}break;
	case CMODE_CIRCLE:
		{
			m_pkCameraMode = NiNew PgCameraModeCircle(m_spCamera, pkActor);
			m_pkCameraMode->Update(0.0f);
		}break;
	case CMODE_SCRIPT:
		{
			m_pkCameraMode = NiNew PgCameraModeScript(m_spCamera);
			m_pkCameraMode->Update(0.0f);
		}break;
	case CMODE_EVENT:
		{
			m_pkCameraMode = NiNew PgCameraModeEvent(m_spCamera);
		}break;
	case CMODE_ANIMATION:
		{
			m_pkCameraMode = NiNew PgCameraModeAni(m_spCamera);
		}break;
	default:
		{
			NILOG(PGLOG_WARNING, "[PgCameraMan] SetCamera, invalid camed mode %d\n", eCameraMode);
		}break;
	}

	m_eCameraMode = eCameraMode;
}

bool PgCameraMan::SetCameraTargetAnimation(int const &iNum, char const *pcFuncName)
{
	if( iNum != NiActorManager::INVALID_SEQUENCE_ID )
	{
		if( m_PastCameraAniID != iNum)
		{
			m_PastCameraAniID = iNum;
			m_szNextScript = pcFuncName;
			return true;
		}
	}
	return false;
}

bool PgCameraMan::Input(PgInput *pkInput)
{
	PgICameraMode *pkCamMode = GetCameraMode();
	if(pkCamMode && !LockInput())
	{
		return pkCamMode->Input(pkInput);
	}

	return true;
}