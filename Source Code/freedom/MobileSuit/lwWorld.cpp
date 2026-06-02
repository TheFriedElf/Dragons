#include "StdAfx.h"
#include "Lohengrin/PgRealm.h"
#include "Lohengrin/PgRealmManager.h"
#include "Variant/PgWorldEvent.h"
#include "Variant/PgBattleSquare.h"
#include "variant/PgTotalObjectMgr.H"
#include "Variant/PgPlayer.h"
#include "Variant/PgMonster.h"
#include "Variant/PgNpc.h"
#include "Variant/PgEntity.h"
#include "Variant/PgEventView.h"
#include "PgNetwork.h"
#include "lwWorld.h"
#include "PgWorld.h"
#include "PgHome.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgBaseItemSet.h"
#include "PgParticleMan.h"
#include "PgParticle.h"
#include "PgPilotMan.h"
#include "PgDropBox.h"
#include "PgShineStone.h"
#include "PgUIScene.h"
#include "PgQuest.h"
#include "PgClientParty.h"
#include "lwUI.h"
#include "lwPilot.h"
#include "PgCameraModeBox.h"
#include "lwActor.h"
#include "PgCameraModeMovie.h"
#include "PgCameraModeFollow.h"
#include "PgCameraModeAni.h"
#include "PgBaseItemSet.h"
#include "PgSelectStage.h"
#include "ServerLib.h"
#include "PgCameraModeFree.H"
#include "PgAction.h"
#include "PgActionEffect.h"
#include "PgAMPool.h"
#include "PgWorkerThread.h"
#include "PgChatMgrClient.h"
#include "PgWorldMapUI.h"
#include "PgActorMonster.h"
#include "PgEnergyGaugeBig.h"
#include "lwCameraModeEvent.H"
#include "PgTrigger.h"
#include "PgContentsBase.h"
#include "PgEmporiaBattleClient.h"
#include "lwRope.H"
#include "PgWorldEventClientMgr.h"
#include "PgWEClientObjectMgr.h"
#include "PgBattleSquare.h"
#include "PgEventQuestUI.h"
#include "PgQuestMan.h"
#include "PgDailyQuestUI.h"
#include "HardCoreDungeon.h"
#include "AlramMissionClient.h"
#include "PgSuperGroundUI.h"
#include "PgOption.h"
#include "lwInventoryUI.h"
#include "NewWare/Scene/ApplyTraversal.h"
#include "PgDungeonMinimap.h"


extern NiPoint3 g_kAdjustTrn;
extern SGroundKey g_kNowGroundKey;
extern NiCamera* FindCamera(NiAVObject* pObject);
DWORD g_dwLastTime = 0;

lwWorld lwToWorld(void *pkObj)
{
	assert(pkObj);
	// xxxxx TODO pkObj가 PgWorld의 것이 맞는지 검사
	if (pkObj)
	{
		//((PgWorld *)pkObj)->SetWorldDataForMiniMap();
	}
	return lwWorld((PgWorld *)pkObj);
}

lwWorld::lwWorld(PgWorld *pkWorld)
	: m_dwMapState(0)
{
	m_pkWorld = pkWorld;
}

DWORD g_dwSavedTime = 0;
DWORD lwGetServerElapsedTime32(bool const bUseSavedTime)
{
	// 간혹 g_kEventView.GetServerElapsedTime32() 결과 같이 같은 시간이 나올 수 있다.
	// 같은 값이 되는 것을 방지 하기 위해 아래와 같이 처리 하도록 한다.
	static Loki::Mutex k_lwGetServerElapsedTime32_Mutex;
	BM::CAutoMutex kLock(k_lwGetServerElapsedTime32_Mutex);
	DWORD const dwNewTime = bUseSavedTime ? g_dwSavedTime : g_kEventView.GetServerElapsedTime();
	if ( dwNewTime <= g_dwLastTime )
	{
		return ++g_dwLastTime;
	}
	g_dwLastTime = dwNewTime;
	return dwNewTime;
}

extern void SaveServerElapsedTime()
{
	g_dwSavedTime = g_kEventView.GetServerElapsedTime();
}

void lwSetServerDateTimeToWnd(lwUIWnd wnd, bool bGameTime, bool bNoTime, bool bNoDate )
{
	if( wnd.IsNil() )
	{
		return;
	}
	SYSTEMTIME kNowTime;
	if ( bGameTime )
	{
		g_kEventView.GetGameTime( &kNowTime );
	}
	else
	{
		g_kEventView.GetLocalTime( &kNowTime );
	}

	std::wstring wstrTime;
	if ( !bNoDate )
	{
		MakeStringForDateTT( wstrTime, kNowTime, 275 );
	}

	if ( !bNoTime )
	{
		if ( !bNoDate )
		{
			wstrTime += L" ";
		}
		MakeStringForTimeTT( wstrTime, kNowTime, 276 );
	}

	if ( !wstrTime.empty() )
	{
		wnd.SetStaticTextW(wstrTime.c_str());
	}
}

bool lwWorld::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "ToWorld", &lwToWorld);
	def(pkState, "GetServerElapsedTime32", &lwGetServerElapsedTime32);
	def(pkState, "SetServerDateTimeToWnd", &lwSetServerDateTimeToWnd);

	class_<lwWorld>(pkState, "World")
		.def(pkState, constructor<PgWorld *>())
		.def(pkState, "IsNil", &lwWorld::IsNil)
		.def(pkState, "IsHaveWorldAttr", &lwWorld::IsHaveWorldAttr)
		.def(pkState, "SetMapNo", &lwWorld::SetMapNo)
		.def(pkState, "GetMapNo", &lwWorld::GetMapNo)
		.def(pkState, "SetAttr", &lwWorld::SetAttr)
		.def(pkState, "GetAttr", &lwWorld::GetAttr)
		.def(pkState, "GetDynamicAttr", &lwWorld::GetDynamicAttr)
		.def(pkState, "PickActor", &lwWorld::PickActor)
		.def(pkState, "AddActor", &lwWorld::AddActor)
		.def(pkState, "RemoveActor", &lwWorld::RemoveObject)		// todo xxx RemoveObject로 바꿨으나.. 몇몇 스크립트에서 사용하므로 남겨둠
		.def(pkState, "RemoveObject", &lwWorld::RemoveObject)
		.def(pkState, "FreezeObject", &lwWorld::FreezeObject)
		.def(pkState, "UnfreezeObject", &lwWorld::UnfreezeObject)
		.def(pkState, "AddDropBox", &lwWorld::AddDropBox)
		.def(pkState, "RemoveActorOnNextUpdate", &lwWorld::RemoveActorOnNextUpdate)
		.def(pkState, "FindSpawnLoc", &lwWorld::FindSpawnLoc)
		.def(pkState, "FindTriggerLoc", &lwWorld::FindTriggerLoc)
		.def(pkState, "FindActorFloorPos", &lwWorld::FindActorFloorPos)
		.def(pkState, "FindActor", &lwWorld::FindActor)
		.def(pkState, "FindObject", &lwWorld::FindObject)
		.def(pkState, "ThrowRay", &lwWorld::ThrowRay)
		.def(pkState, "GetID", &lwWorld::GetID)
		.def(pkState, "SetUpdateSpeed", &lwWorld::SetUpdateSpeed)
		.def(pkState, "GetUpdateSpeed", &lwWorld::GetUpdateSpeed)
		.def(pkState, "SetPlayer", &lwWorld::SetPlayer)
		.def(pkState, "SetCamera", &lwWorld::SetCamera)
		.def(pkState, "SetCameraMode", &lwWorld::SetCameraMode)
		.def(pkState, "GetCameraModeEvent", &lwWorld::GetCameraModeEvent)
		.def(pkState, "SetCameraModeByName", &lwWorld::SetCameraModeByName)		
		.def(pkState, "SetCameraModeByActorInObjectName", &lwWorld::SetCameraModeByActorInObjectName)
		.def(pkState, "SetCameraModeInPlayer", &lwWorld::SetCameraModeInPlayer)
		.def(pkState, "SetCameraAdjustCameraInfo", &lwWorld::SetCameraAdjustCameraInfo)
		.def(pkState, "SetBoxCameraDistance", &lwWorld::SetBoxCameraDistance)
		.def(pkState, "SetFreeCamMoveSpeed", &lwWorld::SetFreeCamMoveSpeed)
		.def(pkState, "SetFreeCamMoveAccelSpeed", &lwWorld::SetFreeCamMoveAccelSpeed)
		.def(pkState, "SetFreeCamRotateSpeed", &lwWorld::SetFreeCamRotateSpeed)
		.def(pkState, "SetFreeCamFOV", &lwWorld::SetFreeCamFOV)
		.def(pkState, "SetCameraPose", &lwWorld::SetCameraPose)
		//.def(pkState, "ChangeSeason", &lwWorld::ChangeSeason)
		//.def(pkState, "TransitMood", &lwWorld::TransitMood)
		.def(pkState, "ToLadder", &lwWorld::ToLadder)
		.def(pkState, "AddParticle", &lwWorld::AddParticle)
		.def(pkState, "ChangeParticleGeneration", &lwWorld::ChangeParticleGeneration)
		.def(pkState, "GetWorldActionMan", &lwWorld::GetWorldActionMan)
		.def(pkState, "ReserveTransitActionToActors", &lwWorld::ReserveTransitActionToActors)
		.def(pkState, "OnBossDie", &lwWorld::OnBossDie)
		//.def(pkState, "Receive_Party_Join_Request", &lwWorld::Receive_Party_Join_Request)
		//.def(pkState, "Clear_All_Party_Member", &lwWorld::Clear_All_Party_Member)
		//.def(pkState, "Set_Party_Master", &lwWorld::Set_Party_Master)
		//.def(pkState, "Remove_Party_Member", &lwWorld::Remove_Party_Member)
		//.def(pkState, "Set_Party_Member_State", &lwWorld::Set_Party_Member_State)
		//.def(pkState, "Show_Party_Message_Box", &lwWorld::Show_Party_Message_Box)
		//.def(pkState, "Show_Party_Join_Answer_Wait_Box", &lwWorld::Show_Party_Join_Answer_Wait_Box)
		//.def(pkState, "Set_Party_Name", &lwWorld::Set_Party_Name)
		//.def(pkState, "Party_Join_Request_Canceled", &lwWorld::Party_Join_Request_Canceled)
		//.def(pkState, "SetPartyOption", &lwWorld::SetPartyOption)
		.def(pkState, "SetCameraModeMovie", &lwWorld::SetCameraModeMovie)
		.def(pkState, "SetCameraModeMovie2", &lwWorld::SetCameraModeMovie2)
		.def(pkState, "RemoveCameraAM", &lwWorld::RemoveCameraAM)
		.def(pkState, "SetCameraMovieTranslate", &lwWorld::SetCameraMovieTranslate)
		.def(pkState, "SetMainCamera", &lwWorld::SetMainCamera)
		.def(pkState, "SetMainCameraStatus", &lwWorld::SetMainCameraStatus)
		.def(pkState, "SetCameraFixed", &lwWorld::SetCameraFixed)
		.def(pkState, "SetCameraZoom", &lwWorld::SetCameraZoom)
		.def(pkState, "GetCameraZoom", &lwWorld::GetCameraZoom)
		.def(pkState, "SaveCameraPos", &lwWorld::SaveCameraPos)
		.def(pkState, "RestoreCameraPos", &lwWorld::RestoreCameraPos)
		.def(pkState, "ResetCameraPos", &lwWorld::ResetCameraPos)
		.def(pkState, "RefreshCamera", &lwWorld::RefreshCamera)
		.def(pkState, "SetCameraZoomMax", &lwWorld::SetCameraZoomMax)
		.def(pkState, "InitSmallAreaInfo", &lwWorld::InitSmallAreaInfo)
		.def(pkState, "GetBossGUID", &lwWorld::GetBossGUID)
		.def(pkState, "SetBossGUID", &lwWorld::SetBossGUID)
		.def(pkState, "GetDifficultyLevel", &lwWorld::GetDifficultyLevel)
		.def(pkState, "SetDifficultyLevel", &lwWorld::SetDifficultyLevel)
		.def(pkState, "SetMonsterTarget", &lwWorld::SetMonsterTarget)
		.def(pkState, "GetMonsterTarget_MonGUID", &lwWorld::GetMonsterTarget_MonGUID)
		.def(pkState, "GetMonsterTarget_TargetGUID", &lwWorld::GetMonsterTarget_TargetGUID)
		.def(pkState, "ResetWorldFocusFilter", &lwWorld::ResetWorldFocusFilter)
		.def(pkState, "SetShowWorldFocusFilter", &lwWorld::SetShowWorldFocusFilter)
		.def(pkState, "SetShowWorldFocusFilterTexAlpha", &lwWorld::SetShowWorldFocusFilterTexAlpha)
		.def(pkState, "SetShowWorldFocusFilterColorAlpha", &lwWorld::SetShowWorldFocusFilterColorAlpha)
		.def(pkState, "SetShowWorldFocusFilterTexSwitch", &lwWorld::SetShowWorldFocusFilterTexSwitch)
		.def(pkState, "SetShowWorldFocusFilterColorSwitch", &lwWorld::SetShowWorldFocusFilterColorSwitch)
		.def(pkState, "GetWorldFocusFilterAlpha", &lwWorld::GetWorldFocusFilterAlpha)
		.def(pkState, "IsEndShowWorldFocusFilter", &lwWorld::IsEndShowWorldFocusFilter)
		.def(pkState, "GetObjectPosByName", &lwWorld::GetObjectPosByName)
		.def(pkState, "GetObjectWorldPosByName", &lwWorld::GetObjectWorldPosByName)
		.def(pkState, "GetCameraPosByName", &lwWorld::GetCameraPosByName)
		.def(pkState, "SetAniType_AppInit", &lwWorld::SetAniType_AppInit)
		.def(pkState, "SetAniType_AppTime", &lwWorld::SetAniType_AppTime)
		.def(pkState, "SetAniCycleType_Loop", &lwWorld::SetAniCycleType_Loop)
		.def(pkState, "SetAniCycleType_Reverse", &lwWorld::SetAniCycleType_Reverse)
		.def(pkState, "SetAniCycleType_Clamp", &lwWorld::SetAniCycleType_Clamp)
		.def(pkState, "SetStartAnimation", &lwWorld::SetStartAnimation)
		.def(pkState, "SetStartAnimationByName", &lwWorld::SetStartAnimationByName)
		.def(pkState, "SetStopAnimation", &lwWorld::SetStopAnimation)
		.def(pkState, "SetStopAnimationByName", &lwWorld::SetStopAnimationByName)
		.def(pkState, "GetAnimationTime", &lwWorld::GetAnimationTime)
		.def(pkState, "GetAccumTime", &lwWorld::GetAccumTime)
		.def(pkState, "DetachObjectByName", &lwWorld::DetachObjectByName)		
		.def(pkState, "GetTriggerByIndex", &lwWorld::GetTriggerByIndex)
		.def(pkState, "GetTriggerByID", &lwWorld::GetTriggerByID)
		.def(pkState, "TurnOnSpotLight", &lwWorld::TurnOnSpotLight)
		.def(pkState, "SetBGColor", &lwWorld::SetBgColor)
		.def(pkState, "SetDrawMode", &lwWorld::SetDrawMode)
		.def(pkState, "RestoreDrawMode", &lwWorld::RestoreDrawMode)		
		.def(pkState, "HideParticle", &lwWorld::HideParticle)
		.def(pkState, "CountSameTexture", &lwWorld::CountSameTexture)
		.def(pkState, "HideNode", &lwWorld::HideNode)
		.def(pkState, "SetNodeSUO", &lwWorld::SetNodeSUO)
		.def(pkState, "TurnOnLight", &lwWorld::TurnOnLight)
		.def(pkState, "ReloadTexture", &lwWorld::ReloadTexture)
		.def(pkState, "ChangeShader", &lwWorld::ChangeShader)
		.def(pkState, "ResizeBonesPerPartition", &lwWorld::ResizeBonesPerPartition)
		.def(pkState, "EnableAlphaBlending", &lwWorld::EnableAlphaBlending)
		.def(pkState, "RestartAnimation", &lwWorld::RestartAnimation)
		.def(pkState, "GetEventScriptIDOnEnter", &lwWorld::GetEventScriptIDOnEnter)
		.def(pkState, "CheckEventScriptOnEnter", &lwWorld::CheckEventScriptOnEnter)
		.def(pkState, "EnableMipMap", &lwWorld::EnableMipMap)
		.def(pkState, "AttachParticle", &lwWorld::AttachParticle)
		.def(pkState, "AttachParticleSWithRotate", &lwWorld::AttachParticleSWithRotate)
		.def(pkState, "DetachParticle", &lwWorld::DetachParticle)
		.def(pkState, "DetachAllParticle", &lwWorld::DetachAllParticle)
		.def(pkState, "AttachSound", &lwWorld::AttachSound)
		.def(pkState, "DetachSound", &lwWorld::DetachSound)
		.def(pkState, "RecursiveDetachGlowMapByGndAttr", &lwWorld::RecursiveDetachGlowMapByGndAttr)
		.def(pkState, "GetCameraMove_ID", &lwWorld::GetCameraMove_ID)
		.def(pkState, "GetOldMouseOverObject", &lwWorld::GetOldMouseOverObject)
		.def(pkState, "IsMapMoveCompleteFade", &lwWorld::IsMapMoveCompleteFade)
		.def(pkState, "MapMoveCompleteFade", &lwWorld::MapMoveCompleteFade)
		.def(pkState, "SaveHideActorClassNo", &lwWorld::SaveHideActorClassNo)
		.def(pkState, "SaveHideActorType", &lwWorld::SaveHideActorType)
		.def(pkState, "ClearSaveHideActor", &lwWorld::ClearSaveHideActor)

//#ifndef EXTERNAL_RELEASE
		.def(pkState, "UseCameraAdjust", &lwWorld::UseCameraAdjust)
		.def(pkState, "UseFogAdjust", &lwWorld::UseFogAdjust)
		.def(pkState, "UseLODAdjust", &lwWorld::UseLODAdjust)
		.def(pkState, "UseFog", &lwWorld::UseFog)
		.def(pkState, "SetFogStart", &lwWorld::SetFogStart)
		.def(pkState, "SetFogEnd", &lwWorld::SetFogEnd)
		.def(pkState, "SetFogAlpha", &lwWorld::SetFogAlpha)
		.def(pkState, "SetFogColor", &lwWorld::SetFogColor)
		.def(pkState, "ResetCamera", &lwWorld::ResetCamera)
		.def(pkState, "ResetShineStone", &lwWorld::ResetShineStone)		
		.def(pkState, "SetShowWorldInfo", &lwWorld::SetShowWorldInfo)
		.def(pkState, "SetShowActorInfo", &lwWorld::SetShowActorInfo)
		.def(pkState, "SetShowPhysXStat", &lwWorld::SetShowPhysXStat)
		.def(pkState, "SetBoneLODLevel", &lwWorld::SetBoneLODLevel)
		.def(pkState, "SetDoFixedStep", &lwWorld::SetDoFixedStep)
		.def(pkState, "SetPhysXDebug", &lwWorld::SetPhysXDebug)
		.def(pkState, "RaisePortalUpdate", &lwWorld::RaiseEmporiaPortalUpdate)
		.def(pkState, "SetMaxIter", &lwWorld::SetMaxIter)
		.def(pkState, "SetOptimizationLevel", &lwWorld::SetOptimizationLevel)
		.def(pkState, "SetActiveLowPolygon", &lwWorld::SetActiveLowPolygon)
		.def(pkState, "SetWorldLODLevel", &lwWorld::SetWorldLODLevel)
		.def(pkState, "SetWorldLODData", &lwWorld::SetWorldLODData)
//#endif		
		.def(pkState, "Recv_AddReadyCharacter", &lwWorld::Recv_AddReadyCharacter)
		// about puppet
		.def(pkState, "AddPuppet", &lwWorld::AddPuppet)

		// about home
		.def(pkState, "GetHome", &lwWorld::GetHome)
		.def(pkState, "CreateHome", &lwWorld::CreateHome)
		.def(pkState, "ReleaseHome", &lwWorld::CreateHome)
		.def(pkState, "ClearReservedPilot", &lwWorld::ClearReservedPilot)

		// about character create
		.def(pkState, "SetSelectStage", &lwWorld::SetSelectStage)

		.def(pkState, "Recv_PT_M_C_NFY_AREA_DATA", &lwWorld::Recv_PT_M_C_NFY_AREA_DATA)
		.def(pkState, "Recv_PT_M_C_ADD_UNIT", &lwWorld::Recv_PT_M_C_ADD_UNIT)
		.def(pkState, "Recv_PT_M_C_NFY_MAPLOADED", &lwWorld::Recv_PT_M_C_NFY_MAPLOADED)
		.def(pkState, "GetServerElapsedTime", &lwWorld::GetServerElapsedTime)
		.def(pkState, "RemoveAllObject",	&lwWorld::RemoveAllObject)
		.def(pkState, "RemoveAllObjectExceptMe",	&lwWorld::RemoveAllObjectExceptMe)
		.def(pkState, "RemoveAllMonster", &lwWorld::RemoveAllMonster)
		.def(pkState, "RemoveAllGradeMonster", &lwWorld::RemoveAllGradeMonster)
		.def(pkState, "CheckPlayerInRange", &lwWorld::CheckPlayerInRange)
		.def(pkState, "RemoveCameraWall", &lwWorld::RemoveCameraWall)
		.def(pkState, "RecoverCameraWall", &lwWorld::RecoverCameraWall)

		.def(pkState, "SetActivePhysXGroup", &lwWorld::SetActivePhysXGroup)
		
		.def(pkState, "AddDrawActorFilter", &lwWorld::AddDrawActorFilter)
		.def(pkState, "ClearDrawActorFilter", &lwWorld::ClearDrawActorFilter)

		.def(pkState, "SetActivaingWorldMap", &lwWorld::SetActivaingWorldMap)
		.def(pkState, "SetUpdateWorld", &lwWorld::SetUpdateWorld)
		.def(pkState, "SetSlowMotion", &lwWorld::SetSlowMotion)

		.def(pkState, "SetCameraAdjustInfo", &lwWorld::SetCameraAdjustInfo)
		.def(pkState, "RecoverCameraAdjustInfo", &lwWorld::RecoverCameraAdjustInfo)		

		.def(pkState, "GetRope", &lwWorld::GetRope)

//		.def(pkState, "Recv_PT_M_C_NFY_PLAY_OPENING", &lwWorld::Recv_PT_M_C_NFY_PLAY_OPENING)
		
		//.def(pkState, "AddPetToWorld", &lwWorld::AddPetToWorld)
		
		.def(pkState, "GetMissionTrigIDOnThisMap", &lwWorld::GetMissionTrigIDOnThisMap)
		
		.def(pkState, "InitDmgNum", &lwWorld::InitDmgNum)
		.def(pkState, "IsExistMiniMapData", &lwWorld::IsExistMiniMapData)		
		.def(pkState, "SetCurProgress", &lwWorld::SetCurProgress)
		.def(pkState, "GetCurProgress", &lwWorld::GetCurProgress)

		.def(pkState, "GetMapThumbnailImgSize", &lwWorld::GetMapThumbnailImgSize)
		.def(pkState, "GetMapThumbnailImgName", &lwWorld::GetMapThumbnailImgName)
		;
	return true;
}
lwRope	lwWorld::GetRope(char const *strRopeName)
{
	PgRope	*pkRope = NULL;

	NiAVObject	*pkObject = m_pkWorld->GetSceneRoot()->GetObjectByName(strRopeName);
	if(pkObject)
	{
		pkRope = NiDynamicCast(PgRope,pkObject);
	}	

	return	lwRope(pkRope);
}
void	lwWorld::TurnOnLight(bool bTurnOn)
{
	NiNode	*pkLightRoot = m_pkWorld->GetLightRoot();
	if(pkLightRoot)
	{
		int	iLightCount = pkLightRoot->GetArrayCount();
		for(int i=0;i<iLightCount;i++)
		{
			NiLight	*pkLight = (NiLight*)pkLightRoot->GetAt(i);
			if(pkLight)
			{
				pkLight->SetSwitch(bTurnOn);
			}
		}
	}

	m_pkWorld->GetSceneRoot()->UpdateEffects();
	m_pkWorld->GetSceneRoot()->Update(0);
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_pkWorld->GetSceneRoot(), false );
}

bool lwWorld::IsNil()
{
	return m_pkWorld == 0;
}

void lwWorld::SetMapNo(int iMapNo)
{
	PG_ASSERT_LOG(iMapNo > 0);
	if (m_pkWorld)
	{
		m_pkWorld->MapNo(iMapNo);
	}

	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->PlayerWorldMapNo(iMapNo);
	}
}

//! 맵의 타입을 반환.
T_GNDATTR lwWorld::GetAttr()
{
	if (m_pkWorld)
	{
		return m_pkWorld->GetAttr();
	}
	return 0;
}
void	lwWorld::RecursiveDetachGlowMapByGndAttr()
{
	if(m_pkWorld)
	{
		m_pkWorld->RecursiveDetachGlowMapByGndAttr(m_pkWorld->GetSceneRoot());
	}
}
//! 맵의 타입을 세팅.
void lwWorld::SetAttr(INT sAttr)
{
	if (m_pkWorld)
	{
		m_pkWorld->SetAttr(sAttr);
	}
}

int lwWorld::GetDynamicAttr() const
{
	if( m_pkWorld )
	{
		return static_cast< int >(m_pkWorld->DynamicGndAttr());
	}
	return 0;
}


int lwWorld::GetMapNo()
{
	if (m_pkWorld)
	{
		return m_pkWorld->MapNo();
	}
	return 0;
}

lwActor lwWorld::PickActor(int iObjectGroupType)
{
	PgIWorldObject* pkObject = m_pkWorld->PickObject((ObjectGroupType)iObjectGroupType);
	return lwActor(dynamic_cast<PgActor *>(pkObject));
}

bool lwWorld::AddActor(lwGUID kGUID, lwActor kActor, lwPoint3 kLoc, unsigned int uiGroup)
{
	OutputDebugString(L"+");
	OutputDebugString(kGUID().str().c_str());
	OutputDebugString(L"\n");
	return m_pkWorld->AddObject(kGUID(), kActor(), kLoc(), (ObjectGroupType)uiGroup);
}

bool lwWorld::AddDropBox(lwGUID kGuid, lwDropBox kDropBox, lwPoint3 kLoc)
{	
	OutputDebugString(L"+");
	OutputDebugString(kGuid().str().c_str());
	OutputDebugString(L"\n");
	return m_pkWorld->AddObject(kGuid(), kDropBox(), kLoc(), OGT_GROUNDBOX);
}

void lwWorld::RemoveObject(lwGUID kGuid)
{
	OutputDebugString(L"-");
	OutputDebugString(kGuid().str().c_str());
	OutputDebugString(L"\n");
	m_pkWorld->RemoveObject(kGuid());
}

void lwWorld::FreezeObject(lwGUID kGuid)
{
	m_pkWorld->FreezeObject(kGuid());
}
lwPoint3	lwWorld::ThrowRay(lwPoint3 kStart,lwPoint3 kDir,float fDistance, int iShapeType, int iGroup)
{
	if(0==iGroup)
	{
		iGroup = -1;	//원래 ThrowRay함수 내부에서는 -1이 기본임.
	}
	return	lwPoint3(m_pkWorld->ThrowRay(kStart(),kDir(),fDistance,iShapeType,iGroup));
}

void lwWorld::UnfreezeObject(lwGUID kGuid)
{
	m_pkWorld->UnfreezeObject(kGuid());
}

void lwWorld::RemoveActorOnNextUpdate(lwGUID kGUID)
{
	m_pkWorld->RemoveObjectOnNextUpdate(kGUID());
	//g_kPilotMan.RemovePilot(kGUID());
}
int	lwWorld::AttachParticle(const char *strParticleID,lwPoint3 kLoc,float const fAliveTime)
{
	NiAVObject *pkParticle = g_kParticleMan.GetParticle(strParticleID);
	if(!pkParticle)
	{
		return -1;
	}

	return m_pkWorld->AttachParticle(pkParticle,kLoc(),fAliveTime);
}

int	lwWorld::AttachParticleSWithRotate(const char *strParticleID,lwPoint3 kLoc, float fScale, lwPoint3 kRot, float const fAliveTime)
{
	NiAVObject *pkParticle = g_kParticleMan.GetParticle(strParticleID);
	if(!pkParticle)
	{
		return -1;
	}

	NiPoint3 kRotate = kRot();
	NiMatrix3 kmatRotX;
	NiMatrix3 kmatRotY;
	NiMatrix3 kmatRotZ;
	NiMatrix3 kmatRot;
	kmatRot.MakeIdentity();
	kmatRotX.MakeIdentity();
	kmatRotY.MakeIdentity();
	kmatRotZ.MakeIdentity();
	if(0.0f != kRotate.x)
	{
		kmatRotX.MakeXRotation(kRotate.x);
	}
	if(0.0f != kRotate.y)
	{
		kmatRotY.MakeYRotation(kRotate.y);
	}
	if(0.0f != kRotate.z)
	{
		kmatRotZ.MakeZRotation(kRotate.z);
	}

	kmatRot = kmatRotX * kmatRotY * kmatRotZ;

	if(0.0f < fScale)
	{
		pkParticle->SetScale(fScale);
	}

	pkParticle->SetRotate(kmatRot);

	return m_pkWorld->AttachParticle(pkParticle,kLoc(),fAliveTime);
}

bool lwWorld::AttachParticleToPointWithRotate(char const* pcParticle, lwPoint3 kPoint, lwQuaternion kQuat, float fScale, float const fAliveTime)
{
	if (0>=fScale)
	{
		fScale = 1.0f;
	}

	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale);
	if (!pkParticle)
	{
		return false;
	}
	
	pkParticle->SetRotate(kQuat());
	
	if(pkParticle->IsAutoGround())
	{
		pkParticle->SetOriginalRotate(pkParticle->GetRotate());
	}

	return m_pkWorld->AttachParticle(pkParticle,kPoint(),fAliveTime);
}

void lwWorld::DetachParticle(int iSlotID)
{
	m_pkWorld->DetachParticle(iSlotID);
}

void lwWorld::DetachAllParticle()
{
	m_pkWorld->DetachAllParticle();
}
void lwWorld::AttachSound(char const *pcSoundID, lwPoint3 kLoc, float fVolume, float fMin, float fMax)
{ 
	m_pkWorld->AttachSound(pcSoundID, kLoc(), fVolume, fMin, fMax);
	
}

void lwWorld::DetachSound(char const* pcSoundID)
{
	m_pkWorld->DetachSound(pcSoundID);
}

bool lwWorld::AddParticle(lwParticle kParticle, lwPoint3 kLoc, unsigned int uiGroup)
{
	return	true;
}
bool lwWorld::ChangeParticleGeneration(char const* pcObjName, bool bGenerate)
{
	if (!m_pkWorld->GetSceneRoot())
	{
		return false;
	}
	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcObjName)
	{
		pkTarget = pkTarget->GetObjectByName(pcObjName);
	}
	if (pkTarget)
	{
		PgParticleMan::ChangeParticleGeneration(pkTarget, bGenerate);
	}

	return true;
}
lwPoint3 lwWorld::FindSpawnLoc(char const *pcSpawnName)
{
	NiPoint3 kLoc;

	m_pkWorld->FindSpawnLoc(pcSpawnName, kLoc);

	return lwPoint3(kLoc);
}
lwPoint3 lwWorld::FindTriggerLoc(char const *pcTriggerName)
{
	NiPoint3 kLoc;

	m_pkWorld->FindTriggerLoc(pcTriggerName, kLoc);

	return lwPoint3(kLoc);
}
lwPoint3	lwWorld::FindActorFloorPos(lwPoint3 kStart)	//	액터 놓을 위치를 계산한다.(떨어지지 않게)
{
	return	lwPoint3(m_pkWorld->FindActorFloorPos(kStart()));
}

lwActor lwWorld::FindActor(lwGUID kGUID)
{
	return lwActor((PgActor *)m_pkWorld->FindObject(kGUID()));
}

void	lwWorld::InitSmallAreaInfo(lwPacket kPacket)
{
	if(!m_pkWorld || !g_pkWorld)
	{
		return;
	}

	m_pkWorld->InitSmallAreaInfo(kPacket());

}

PgIWorldObject* lwWorld::FindObject(lwGUID kGUID)const
{
	return m_pkWorld->FindObject(kGUID());
}

char const *lwWorld::GetID()
{
	return m_pkWorld->GetID().c_str();
}
//!	월드 업데이트 스피드 조절
void	lwWorld::SetUpdateSpeed(float fSpeed)
{
	m_pkWorld->SetUpdateSpeed(fSpeed);
}
float	lwWorld::GetUpdateSpeed()
{
	return	m_pkWorld->GetUpdateSpeed();
}
float	lwWorld::GetAccumTime()
{
	return	m_pkWorld->GetAccumTime();
}
bool lwWorld::SetPlayer(lwActor kActor)
{
	return g_kPilotMan.SetPlayerPilot(kActor.GetPilotGuid()());
}

void lwWorld::SetCamera( char const *pcCameraName )
{
	m_pkWorld->m_kCameraMan.SetCamera( NiString( pcCameraName ) );
}

bool lwWorld::SetCameraMode(int iMode, lwActor kActor)
{
	PgActor* pActor(kActor());
	if(!pActor && (PgCameraMan::CMODE_FOLLOW == iMode))
	{
		return false;
	}

	if (m_pkWorld && m_pkWorld->GetCameraMan())
	{
		m_pkWorld->GetCameraMan()->SetCameraMode((PgCameraMan::ECameraMode)iMode, kActor());
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_pkWorld->GetCameraMan()->GetCameraMode());
		if (pkCameraMode)
		{
			pkCameraMode->SetCameraAdjustInfo(m_pkWorld->GetCameraAdjustInfo());
		}
		return true;
	}
	return false;
}
void lwWorld::SetCameraAdjustCameraInfo(lwPoint3 const kPosInfo, lwPoint3 const kLookAtInfo)
{
	PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_pkWorld->GetCameraMan()->GetCameraMode());
	if (pkCameraMode)
	{
		NiPoint3 const kPos(kPosInfo.GetX(),kPosInfo.GetY(),kPosInfo.GetZ());
		NiPoint3 const kLook(kLookAtInfo.GetX(),kLookAtInfo.GetY(),kLookAtInfo.GetZ());
		pkCameraMode->SetCameraAdjustCameraInfo(kPos, kLook);
	}
}
void	lwWorld::SetFogAlpha(float fValue)
{
	if (m_pkWorld)
	{
		NiFogProperty* pkFogProp = m_pkWorld->GetFogProperty();
		if (pkFogProp)
		{
			pkFogProp->SetFogAlpha(fValue);
		}
	}
}
void	lwWorld::SetFogColor(int R,int G,int B)
{
	if (m_pkWorld)
	{
		NiFogProperty* pkFogProp = m_pkWorld->GetFogProperty();
		if (pkFogProp)
		{
			pkFogProp->SetFogColor(NiColor(R/255.0f,G/255.0f,B/255.0f));
		}
	}
}

bool lwWorld::SetCameraModeInPlayer(int iMode)
{
	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if (pkActor)
	{
		m_pkWorld->m_kCameraMan.SetCameraMode((PgCameraMan::ECameraMode)iMode, pkActor);
	}

	return true;
}

void	lwWorld::SetFreeCamMoveAccelSpeed(float	fSpeed)
{
	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FREE)
	{
		PgCameraModeFree	*pkFreeCam = (PgCameraModeFree*)m_pkWorld->m_kCameraMan.GetCameraMode();
		if(pkFreeCam)
		{
			pkFreeCam->SetMoveAccelSpeed(fSpeed);
		}
	}
}

void	lwWorld::SetBoxCameraDistance(float fDistance)
{
	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_BOX)
	{
		PgCameraModeBox *pkBoxCam= (PgCameraModeBox*)m_pkWorld->m_kCameraMan.GetCameraMode();
		if (pkBoxCam)
		{
			pkBoxCam->SetCameraDistance(fDistance);
		}
	}
}

void	lwWorld::SetFreeCamMoveSpeed(float	fSpeed)
{
	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FREE)
	{
		PgCameraModeFree	*pkFreeCam = (PgCameraModeFree*)m_pkWorld->m_kCameraMan.GetCameraMode();
		if(pkFreeCam)
		{
			pkFreeCam->SetMoveSpeed(fSpeed);
		}
	}
}
void	lwWorld::SetFreeCamRotateSpeed(float	fSpeed)
{
	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FREE)
	{
		PgCameraModeFree	*pkFreeCam = (PgCameraModeFree*)m_pkWorld->m_kCameraMan.GetCameraMode();
		if(pkFreeCam)
		{
			pkFreeCam->SetRotateSpeed(fSpeed);
		}
	}
}
void	lwWorld::HideParticle(bool bHide)
{
    NewWare::Scene::ApplyTraversal::Geometry::RunParticleGeneration( m_pkWorld->GetSceneRoot(), bHide );
    NewWare::Scene::ApplyTraversal::Geometry::HideParticleSystems( m_pkWorld->GetSceneRoot(), bHide );
}
void	lwWorld::SetNodeSUO(const char *strNodeName,bool bUse)
{
	NiAVObject	*pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(strNodeName);
	if(pkNode)
	{
        NewWare::Scene::ApplyTraversal::Geometry::SetShaderConstantUpdateOptimizeFlag( pkNode, bUse, true );
	}

}
void	lwWorld::CountSameTexture(const char *strNodeName)
{
	NiAVObject	*pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(strNodeName);
	if(pkNode)
	{
		stSameTextureCounter kCounter;
		PgRenderer::CountSameTexture(pkNode,kCounter);
		kCounter.OutputResult();
	}
}
void	lwWorld::HideNode(const char *strNodeName,bool bHide)
{
	NiAVObject	*pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(strNodeName);
	if(pkNode)
	{
		pkNode->SetAppCulled(bHide);
	}
}
void	lwWorld::TurnOnSpotLight(bool bTurnOn)
{
	m_pkWorld->TurnOnSpotLight(bTurnOn);
}
void	lwWorld::SetSpotLightBGColor(int Red,int Blue,int Green)
{
	m_pkWorld->SetSpotLightBGColor(Red,Blue,Green);
}

void lwWorld::SetBgColor(int const iR, int const iG,int const iB)
{
	if(m_pkWorld)
	{
		NiColor kColor(iR/255.0f, iG/255.0f, iB/255.0f);
		m_pkWorld->SetBgColor(kColor);
	}
}

void lwWorld::SetDrawMode(DWORD const dwDrawMode)
{
	if ( m_pkWorld )
	{
		m_pkWorld->SetDrawMode( static_cast<NewWare::Renderer::DrawBound::DrawMode>(dwDrawMode) );
	}
}

void lwWorld::RestoreDrawMode()
{
	if(m_pkWorld)
	{
		m_pkWorld->RestoreDrawMode();
	}
}

void	lwWorld::SetFreeCamFOV(float	fFOV)
{
	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FREE)
	{
		PgCameraModeFree	*pkFreeCam = (PgCameraModeFree*)m_pkWorld->m_kCameraMan.GetCameraMode();
		if(pkFreeCam)
		{
			pkFreeCam->SetFOV(fFOV);
		}
	}
}

bool lwWorld::SetCameraModeMovie(int iMode, int iSeqID, char const *pcScriptName)
{
	if( GetCameraAM() && m_pkWorld->m_kCameraMan.SetCameraTargetAnimation(iSeqID, pcScriptName))
	{
		m_pkWorld->m_kCameraMan.SetCameraMode((PgCameraMan::ECameraMode)iMode, 0, GetCameraAM());
		return true;
	}
	return false;
}

bool lwWorld::SetCameraModeMovie2(lwActor kActor, int iMode, int iSeqID, char const *pcScriptName)
{
	PgActor* pActor(kActor());
	if(!pActor)
	{
		return false;
	}

	//PgWorld::SetAniType(pActor->GetNIFRoot(), NiTimeController::APP_INIT);

	//NiTimeController::StartAnimations(pActor->GetNIFRoot());

	SetCameraAM(pActor->GetActorManager());

	if( GetCameraAM() && m_pkWorld->m_kCameraMan.SetCameraTargetAnimation(iSeqID, pcScriptName))
	{
		m_pkWorld->m_kCameraMan.SetCameraMode((PgCameraMan::ECameraMode)iMode, 0, GetCameraAM());
		if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_MOVIE)
		{
			PgCameraModeMovie *pkMovieCam = dynamic_cast<PgCameraModeMovie*>(m_pkWorld->m_kCameraMan.GetCameraMode());
			if(pkMovieCam)
			{
				pkMovieCam->SetAMUpdate(false);
				return true;
			}
		}
	}
	return false;
}

void lwWorld::SetCameraMovieTranslate(lwPoint3 kTrn)
{
	if(PgCameraMan::CMODE_MOVIE == m_pkWorld->m_kCameraMan.GetCameraModeE())
	{
		PgCameraModeMovie *pkMovieCam = dynamic_cast<PgCameraModeMovie*>(m_pkWorld->m_kCameraMan.GetCameraMode());
		if(pkMovieCam)
		{
			pkMovieCam->SetTranslate(kTrn());
		}
	}
}

bool lwWorld::SetMainCamera(char const* pcCamName)
{
	if(!m_pkWorld || !m_pkWorld->GetSceneRoot())
	{
		return false;
	}

	NiAVObject* pkCam = m_pkWorld->GetSceneRoot()->GetObjectByName(pcCamName);
	if (!pkCam)
	{
		return false;
	}

	NiCameraPtr spCamera = 0;
	if (NiIsKindOf(NiCamera, pkCam))
	{
		spCamera = (NiCamera *)pkCam;
		m_pkWorld->m_kCameraMan.SetCamera(spCamera);
		m_pkWorld->m_kCameraMan.AddCamera((NiString)(pcCamName), spCamera);
		m_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_NONE, 0);
		return true;
	}

	return false;
}

bool lwWorld::SetMainCameraStatus(char const* pcOtherCameName)
{
	if(!m_pkWorld || !m_pkWorld->GetSceneRoot())
	{
		return false;
	}

	NiAVObject* pkCam = m_pkWorld->GetSceneRoot()->GetObjectByName(pcOtherCameName);
	if (!pkCam)
	{
		return false;
	}

	NiCameraPtr spCamera = 0;
	if (NiIsKindOf(NiCamera, pkCam))
	{
		spCamera = (NiCamera *)pkCam;
		NiCameraPtr spMainCamera = NiDynamicCast(NiCamera, spCamera->CreateDeepCopy());
		spMainCamera->Update(0);
		m_pkWorld->m_kCameraMan.SetCamera(spMainCamera);
		g_kFrustum = spMainCamera->GetViewFrustum();

		return true;
	}

	return false;
}

bool lwWorld::SetCameraFixed( bool const bFixed )
{
	PgCameraModeFollow *pkMode = dynamic_cast<PgCameraModeFollow*>(m_pkWorld->m_kCameraMan.GetCameraMode());
	if ( !pkMode )	return false;

	pkMode->SetCameraFixed( bFixed );
	return true;
}

void lwWorld::SetCameraZoom( float const fZoom )
{
	PgCameraModeFollow::SetCameraZoom(fZoom);
}

float lwWorld::GetCameraZoom()
{
	return PgCameraModeFollow::GetCameraZoom();
}

void lwWorld::ResetCameraPos()
{
	if (m_pkWorld == NULL)
		return;

	m_pkWorld->m_kCameraMan.ResetCameraPos();
}

void lwWorld::SaveCameraPos()
{
	if (m_pkWorld == NULL)
		return;

	m_pkWorld->m_kCameraMan.SaveCameraPos();
}

void lwWorld::RestoreCameraPos()
{
	if (m_pkWorld == NULL)
		return;

	m_pkWorld->m_kCameraMan.RestoreCameraPos();
}

void lwWorld::RefreshCamera()
{
	if (m_pkWorld == NULL)
		return;

	m_pkWorld->m_kCameraMan.RefreshCamera();
}

bool lwWorld::SetCameraPose(lwPoint3 kTrn, lwQuaternion kRot)
{
	if (m_pkWorld == NULL)
		return false;

	if (m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FREE)
		return true;

	m_pkWorld->m_kCameraMan.GetCamera()->SetTranslate(kTrn());
	m_pkWorld->m_kCameraMan.GetCamera()->SetRotate(kRot());
	m_pkWorld->m_kCameraMan.GetCamera()->Update(0.0f);
	return true;
}

void lwWorld::SetCameraZoomMax(float fValue)
{
	if (m_pkWorld == NULL)
		return;

	if (m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW)
	{
		PgCameraModeFollow *pkFollowCam = (PgCameraModeFollow*)m_pkWorld->m_kCameraMan.GetCameraMode();

		if (pkFollowCam)
		{
			return pkFollowCam->SetCameraZoomMax(fValue);
		}
	}
}
void	lwWorld::EnableAlphaBlending(char const *strNodeName,bool bEnable)
{
	NiAVObject	*pkAVObject = m_pkWorld->GetSceneRoot()->GetObjectByName( strNodeName );
	if(pkAVObject)
	{
		PgRenderer::EnableAlphaBlending(pkAVObject,bEnable);
	}	
}
void	lwWorld::EnableMipMap(char const *strNodeName,bool bEnable)
{
	NiAVObject	*pkAVObject = m_pkWorld->GetSceneRoot()->GetObjectByName( strNodeName );
	if(pkAVObject)
	{
		PgRenderer::EnableMipMap(pkAVObject,bEnable);
	}
}

void	lwWorld::ResizeBonesPerPartition(char const *strNodeName,int iBonesPerPartition,int iVertsPerPartition)
{
	NiAVObject	*pkAVObject = m_pkWorld->GetSceneRoot()->GetObjectByName( strNodeName );
	if(pkAVObject)
	{
        NewWare::Scene::ApplyTraversal::Geometry::OptimizeActorSkins( pkAVObject, iBonesPerPartition );
	}
}
lwCameraModeEvent	lwWorld::GetCameraModeEvent()
{
	if (m_pkWorld->m_kCameraMan.GetCameraModeE() != PgCameraMan::CMODE_EVENT)
	{
		return	lwCameraModeEvent(NULL);
	}

	PgCameraModeEvent	*pkCamMode = (PgCameraModeEvent*)m_pkWorld->m_kCameraMan.GetCameraMode();

	return	lwCameraModeEvent(pkCamMode);
}
bool lwWorld::SetCameraModeByName( char const *pcSceneRoot, char const *pcCameraName )	//루아에서 반드시 g_world:SetCameraMode(1, GetMyActor()) 이걸로 되돌려야 한다
{
	NiAVObject *pkSceneRoot = m_pkWorld->GetSceneRoot()->GetObjectByName( NiFixedString(pcSceneRoot) );
	if ( pkSceneRoot )
	{
		NiAVObject* pkCameraNode = pkSceneRoot->GetObjectByName(pcCameraName);
		NiCamera* pkCamera = NULL;

		if (pkCameraNode)
		{
			pkCamera = FindCamera(pkCameraNode);
		}

		if (pkCamera)
		{
			m_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_ANIMATION);
			PgCameraModeAni* pkCameraMode = dynamic_cast<PgCameraModeAni*>(m_pkWorld->m_kCameraMan.GetCameraMode());
			if(pkCameraMode)
			{
				pkCameraMode->SetAniCamera(pkCamera);
			}
			//m_pkWorld->m_kCameraMan.SetCamera( NiDynamicCast(NiCamera, pkCamera) );
			return true;
		}
	}
	return false;
}

bool lwWorld::SetCameraModeByActorInObjectName(lwActor kActor, char const *pcCameraName, bool bAniReset, float fStartTime, bool bUseActorPositionAsSoundListener)
{	
	if (pcCameraName == NULL)
		return false;

	if (kActor() == NULL || kActor()->GetNIFRoot() == NULL)
		return false;

	NiAVObject* pkCameraNode = NULL;
	NiCamera* pkCamera = NULL;

	pkCameraNode = kActor()->GetNIFRoot()->GetObjectByName(NiFixedString(pcCameraName));	

	if (pkCameraNode)
	{
		pkCamera = FindCamera(pkCameraNode);
	}

	if (pkCamera)
	{
		std::string cameraTarget = pcCameraName;
		cameraTarget += ".Target";
		NiAVObject *pkCameraTarget = kActor()->GetNIFRoot()->GetObjectByName(cameraTarget.c_str());

		m_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_ANIMATION);
		PgCameraModeAni* pkCameraMode = dynamic_cast<PgCameraModeAni*>(m_pkWorld->m_kCameraMan.GetCameraMode());
		if(pkCameraMode)
		{
			pkCameraMode->SetAniCamera(pkCamera);
			pkCameraMode->SetUseActorPositionAsSoundListener(bUseActorPositionAsSoundListener);
			pkCameraMode->SetActorRoot(kActor()->GetNIFRoot());
		}
		
		if(bAniReset)
		{
			PgWorld::SetAniType(pkCameraNode, NiTimeController::APP_TIME);
			PgWorld::SetAniCycleType(pkCameraNode, NiTimeController::CLAMP);
			PgRenderer::RestartAnimation(pkCameraNode, m_pkWorld->GetAccumTime());
			if (pkCameraTarget)
			{
				PgWorld::SetAniType(pkCameraTarget, NiTimeController::APP_TIME);
				PgWorld::SetAniCycleType(pkCameraTarget, NiTimeController::CLAMP);
				PgRenderer::RestartAnimation(pkCameraTarget, m_pkWorld->GetAccumTime());
			}
		}
		return true;
	}

	return false;
}

//bool lwWorld::ChangeSeason(char const *pcSeasonName)
//{
//	return m_pkWorld->ChangeSeason(pcSeasonName);
//}

//bool lwWorld::TransitMood(char const *pcMood)
//{
//	return m_pkWorld->TransitMood(pcMood);
//}

void lwWorld::OnBossDie()
{
	if(false == m_pkWorld->GetBossDieScript().empty())
	{
		lua_tinker::call<void>(m_pkWorld->GetBossDieScript().c_str());
	}
}

void lwWorld::RaiseEmporiaPortalUpdate(char const *pcTriggerName, BYTE byGulidLogoIndex, char const* pcGuildName)
{
	SEmporiaGuildInfo kGuildInfo;
	kGuildInfo.kGuildID = BM::GUID::Create();
	kGuildInfo.nEmblem = byGulidLogoIndex;
	kGuildInfo.wstrName = UNI(pcGuildName);
	m_pkWorld->UpdatePortalObject(pcTriggerName, kGuildInfo);
}

lwWorldActionMan	lwWorld::GetWorldActionMan()
{
	return	lwWorldActionMan(m_pkWorld->GetWorldActionMan());
}
bool lwWorld::ToLadder(lwActor kCaster)
{
	bool bRet = false;
	if(m_pkWorld->GetLadderRoot() != 0)
	{
		NiPick kPick;
		kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
		kPick.SetPickType(NiPick::FIND_FIRST);
		kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
		kPick.SetTarget(m_pkWorld->GetLadderRoot());

		if(kPick.PickObjects(kCaster()->GetTranslate() + NiPoint3::UNIT_Z * 15.0f, kCaster()->GetPathNormal()))
		{
			bRet = true;
		}
	}

	return bRet;
}

//! Fog Adjust 기능을 사용한다.
void	lwWorld::UseFogAdjust(bool bUse)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetUseFogAdjust(bUse);
}

void	lwWorld::UseLODAdjust(bool bUse)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetUseLODAdjust(bUse);
}

void	lwWorld::UseFog(bool bUse, bool bUseAdjust)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->UseFog(bUse, bUseAdjust);
}

void	lwWorld::SetFogStart(float fValue)
{
	if (m_pkWorld)
	{
		NiFogProperty* pkFogProp = m_pkWorld->GetFogProperty();
		if (pkFogProp)
		{
			pkFogProp->SetFogStart(fValue);
		}
	}
}

void	lwWorld::SetFogEnd(float fValue)
{
	if (m_pkWorld)
	{
		NiFogProperty* pkFogProp = m_pkWorld->GetFogProperty();
		if (pkFogProp)
		{
			pkFogProp->SetFogEnd(fValue);
		}
	}
}

//! Camera Adjust 기능을 사용한다.
void	lwWorld::UseCameraAdjust(bool bUse)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetUseCameraAdjust(bUse);
}

//! Camera를 원래 대로 리셋하다.
void	lwWorld::ResetCamera()
{
	if (m_pkWorld && m_pkWorld->GetCameraMan())
	{
		PgICameraMode* cameraMode= (PgICameraMode*)m_pkWorld->GetCameraMan()->GetCameraMode();
		if (cameraMode)
		{
			cameraMode->ResetCameraPos();
		}
	}	
}

//! ShineStone를 리셋한다.
void lwWorld::ResetShineStone()
{
	PgWorld::ShineStoneContainer::iterator itr = m_pkWorld->m_kShineStoneContainer.begin();
	for (itr = m_pkWorld->m_kShineStoneContainer.begin() ;
		itr != m_pkWorld->m_kShineStoneContainer.end() ;
		++itr)
	{
		PgShineStone* pkStone = (PgShineStone*)itr->second;
		pkStone->Initialize();
		pkStone->Enable(true);
		pkStone->SetAppCulled(false);
	}
}

void lwWorld::SetAniType_AppInit(char const* pcName)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcName)
		pkTarget = pkTarget->GetObjectByName(pcName);
	if (pkTarget)
		PgWorld::SetAniType(pkTarget, NiTimeController::APP_INIT);
}

void lwWorld::SetAniType_AppTime(char const* pcName)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcName)
		pkTarget = pkTarget->GetObjectByName(pcName);
	if (pkTarget)
		PgWorld::SetAniType(pkTarget, NiTimeController::APP_TIME);
}

void lwWorld::SetAniCycleType_Loop(char const* pcName)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcName)
		pkTarget = pkTarget->GetObjectByName(pcName);
	if (pkTarget)
		PgWorld::SetAniCycleType(pkTarget, NiTimeController::LOOP);
}

void lwWorld::SetAniCycleType_Reverse(char const* pcName)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcName)
		pkTarget = pkTarget->GetObjectByName(pcName);
	if (pkTarget)
		PgWorld::SetAniCycleType(pkTarget, NiTimeController::REVERSE);
}

void lwWorld::SetAniCycleType_Clamp(char const* pcName)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot();
	if (pcName)
		pkTarget = pkTarget->GetObjectByName(pcName);
	if (pkTarget)
		PgWorld::SetAniCycleType(pkTarget, NiTimeController::CLAMP);
}

//! Scene Root의 StartAnimation 시간을 바꾼다.
void lwWorld::SetStartAnimation(float fStartTime)
{
	if (!m_pkWorld->GetSceneRoot())
		return ;
	NiTimeController::StartAnimations(m_pkWorld->GetSceneRoot(), fStartTime);
}

//! 해당 오브젝트의 StartAnimation 시간을 바꾼다.
void lwWorld::SetStartAnimationByName(char const *pcName, float fStartTime)
{
	if (!m_pkWorld->GetSceneRoot() || !pcName)
		return ;	

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot()->GetObjectByName(pcName);
	if (pkTarget)
	{
		if (fStartTime == 0.0f)
			NiTimeController::StartAnimations(pkTarget);
		else
			NiTimeController::StartAnimations(pkTarget, fStartTime);
	}
}
int	lwWorld::GetEventScriptIDOnEnter()const
{
	int const INDUN_STATE_PLAY		= 0x08;	// 인던의 유저들이 놀고 있는 중이다.
	if(INDUN_STATE_PLAY & m_dwMapState)
	{//파티난입으로 간주
		return	m_pkWorld->m_kEventScriptOnEnter.m_iEventScriptIDOnEnterByPartyBreakIn;
	}
	return m_pkWorld->m_kEventScriptOnEnter.m_iEventScriptIDOnEnter;
}
//! Scene Root을 Stop한다.
void lwWorld::SetStopAnimation()
{
	if (!m_pkWorld->GetSceneRoot())
		return ;
	NiTimeController::StopAnimations(m_pkWorld->GetSceneRoot());
}


void lwWorld::RestartAnimation(char const* pcName)
{
	NiAVObject	*pkObject = m_pkWorld->GetSceneRoot()->GetObjectByName(pcName);
	if(!pkObject)
	{
		return;
	}

	PgRenderer::RestartAnimation(pkObject,m_pkWorld->GetAccumTime());

}
//! 해당 오브젝트를 Stop한다..
void lwWorld::SetStopAnimationByName(char const *pcName)
{
	if (!m_pkWorld->GetSceneRoot() || !pcName)
		return ;

	NiAVObject* pkTarget = m_pkWorld->GetSceneRoot()->GetObjectByName(pcName);
	if (pkTarget)
	{
		NiTimeController::StopAnimations(pkTarget);
	}
}

//! 최적화 레벨을 설정한다.
void lwWorld::SetOptimizationLevel(int iLevel)
{
	if (m_pkWorld)
	{
		m_pkWorld->SetOptimizationLevel(iLevel);
	}
}

//! 로우 폴리를 보여줄지 설정.
void lwWorld::SetActiveLowPolygon(bool bActive, int iGlobalLOD)
{
	if (m_pkWorld)
	{
		m_pkWorld->SetActiveLowPolygon(bActive, iGlobalLOD);
	}
}

void lwWorld::SetShowWorldInfo(bool bShow)
{
	if (m_pkWorld)
		m_pkWorld->SetShowWorldInfo(bShow);
}

void lwWorld::SetShowActorInfo(bool bShow)
{
	if (m_pkWorld)
		m_pkWorld->SetShowActorInfo(bShow);
}

void lwWorld::SetShowPhysXStat(bool bShow)
{
	//if (m_pkWorld)
	//	m_pkWorld->SetShowPhysXStat(bShow);
}

float lwWorld::GetAnimationTime( char const *pcSceneRoot )
{
	NiObjectNET* pkRoot = NiDynamicCast( NiObjectNET, m_pkWorld->GetSceneRoot()->GetObjectByName( NiFixedString(pcSceneRoot) ) );

	if ( !pkRoot )
	{
		return 0.0f;
	}

	float fBeginTime = 0.0f;
	float fEndTime = 0.0f;

	NiTimeController* pkController = pkRoot->GetControllers();
	while ( pkController )
	{
		if ( fBeginTime > pkController->GetBeginKeyTime() )
		{
			fBeginTime = pkController->GetBeginKeyTime();
		}

		if ( fEndTime < pkController->GetEndKeyTime() )
		{
			fEndTime = pkController->GetEndKeyTime();
		}

		pkController = pkController->GetNext();
	}

	return ( fEndTime - fBeginTime );
}

void lwWorld::SetDirectionArrow( NiAVObject* pkDestObj )
{
	// DestObj가 null이면 화살표를 없앤다.
	if (m_pkWorld)
	{
		m_pkWorld->SetDirectionArrow( pkDestObj );
	}
}

//	나에게 가입 요청을 한 사람이, 그 요청을 취소했다.
//void	lwWorld::Party_Join_Request_Canceled()
//{
//	g_kParty.Party_Join_Request_Canceled();
//}

//	파티 분배 방식을 설정한다.
//void	lwWorld::SetPartyOption(int Opt)
//{
//	g_kParty.SetPartyOption(Opt);
//}

//	가입 요청을 받은 사람에게, 가입 요청 받았음을 표시해주고, 수락,거부 응답을 기다린다.
//void	lwWorld::Receive_Party_Join_Request(lwWString Master_Name,lwGUID PartyGuid,lwGUID Master_MemberGuid)
//{
//	g_kParty.Receive_Party_Join_Request(Master_Name(),PartyGuid(),Master_MemberGuid());
//}
//	파티 이름을 설정한다.
//void	lwWorld::Set_Party_Name(lwWString &PartyName)
//{
//	g_kParty.Set_Party_Name(PartyName(),true);
//}

//	파티 멤버를 초기화한다.
//void	lwWorld::Clear_All_Party_Member()
//{
//	
//}

//	파티장을 설정한다.
//void	lwWorld::Set_Party_Master(lwGUID lwkGuid)
//{
//	
//}

////	멤버를 파티에서 탈퇴시킨다.
//void	lwWorld::Remove_Party_Member(lwGUID lwkGuid)
//{
//	
//}

////	멤버의 상태값을 설정한다.
//void	lwWorld::Set_Party_Member_State(lwWString Member_Name,int iState)
//{
//	g_kParty.Set_Party_Member_State(Member_Name(),(EPartyMemberState)iState);
//}

//	파티 관련 처리중에 유저에게 알려줄 내용을 표시한다.
//	지정한 시간이 지나면 자동으로 사라지고(0일경우 무한대), 그 전에 확인 버튼을 눌러도 사라진다.
//void	lwWorld::Show_Party_Message_Box(lwWString NotifyMessage,int iDisplayLimitTime)
//{
//	lwAddWarnDataStr(NotifyMessage,0);
//	g_kParty.Show_Party_Message_Box(NotifyMessage(),iDisplayLimitTime);
//}

//	가입 요청을 보낸 클라이언트에 띄워주는, 응답 대기 메세지박스를 띄운다.
//	지정한 시간이 지나면 자동으로 사라지면서, 가입이 취소된다.
//void	lwWorld::Show_Party_Join_Answer_Wait_Box(bool bShow,char const* TargetUserName,int iDisplayLimitTime)
//{
//	g_kParty.Show_Party_Join_Answer_Wait_Box(bShow,std::wstring(UNI(TargetUserName)),iDisplayLimitTime);
//}

bool lwWorld::IsHaveWorldAttr( const T_GNDATTR kAttr )
{
	return m_pkWorld->IsHaveAttr( kAttr );
}

NiActorManagerPtr lwWorld::GetCameraAM()
{
	return m_pkWorld->m_spCameraAM;
}

void lwWorld::SetCameraAM(NiActorManager* pAM)
{
	m_pkWorld->m_spCameraAM = pAM;
}

void lwWorld::RemoveCameraAM()
{
	m_pkWorld->m_spCameraAM = 0;
}

lwPuppet lwWorld::AddPuppet(lwGUID kGuid, char const *pcPuppetName, lwPoint3 kTranslate, lwQuaternion kRotate)
{
	return (lwPuppet)m_pkWorld->AddPuppet(kGuid(), pcPuppetName, kTranslate(), kRotate());
}
lwGUID	lwWorld::GetBossGUID()
{
	return	lwGUID(m_pkWorld->GetBossGUID());
}
void	lwWorld::SetBossGUID(lwGUID GUID)
{
	m_pkWorld->SetBossGUID(GUID());
}
//!	몬스터 타겟 설정
void	lwWorld::SetMonsterTarget(lwGUID MonGUID,lwGUID TargetGUID)
{
	m_pkWorld->SetMonsterTarget(MonGUID(),TargetGUID());
}
lwGUID	lwWorld::GetMonsterTarget_MonGUID()
{
	BM::GUID monguid,targetguid;
	m_pkWorld->GetMonsterTarget(monguid,targetguid);
	return	lwGUID(monguid);
}
lwGUID	lwWorld::GetMonsterTarget_TargetGUID()
{
	BM::GUID monguid,targetguid;
	m_pkWorld->GetMonsterTarget(monguid,targetguid);
	return	lwGUID(targetguid);
}

lwHome lwWorld::GetHome()
{
	if (m_pkWorld)
		return (lwHome)m_pkWorld->GetHome();
	
	return lwHome(NULL);
}

lwHome lwWorld::CreateHome()
{
	if (m_pkWorld)
		return (lwHome)m_pkWorld->CreateHome();

	return lwHome(NULL);
}

void lwWorld::ReleaseHome()
{
	if (m_pkWorld)
		m_pkWorld->ReleaseHome();
}

void lwWorld::ClearReservedPilot()
{
	g_kPilotMan.ClearReservedPilot();
}

//	kUnitType 에 해당하는 모든 유닛을 kActionName 의 액션으로 ReserveTransitAction 시킨다.
void	lwWorld::ReserveTransitActionToActors(char const *kActionName,int kUnitType)
{
	m_pkWorld->ReserveTransitActionToActors(kActionName,(EUnitType)kUnitType);
}
void	lwWorld::ChangeShader(const char *strNodeName,char const *strShaderName)
{
	NiAVObject	*pkAVObject = m_pkWorld->GetSceneRoot()->GetObjectByName(strNodeName);
	if(pkAVObject)
	{
		PgRenderer::ChangeShader(pkAVObject,strShaderName);
	}

}
void	lwWorld::ReloadTexture(const char *strNodeName,int iMipMapSkipLevel)
{
	NiAVObject	*pkAVObject = m_pkWorld->GetSceneRoot()->GetObjectByName(strNodeName);
	if(pkAVObject)
	{
		PgRenderer::ReloadTexture(pkAVObject,iMipMapSkipLevel);
	}
}
lwSelectStage lwWorld::SetSelectStage(bool bTrue)
{
	PgSelectStage* pkStage = &g_kSelectStage;
	if(!bTrue)
	{
		if (pkStage)
		{
			pkStage->Clear();
		}
	}

	//PgSelectStage *pkStage = new PgSelectStage(m_pkWorld);
	//m_pkWorld->m_pkSelectStage = pkStage;	
	return lwSelectStage(pkStage);
}

// 단순히 show/hide만 하는 interface
void lwWorld::ResetWorldFocusFilter()
{
	if (m_pkWorld)
		m_pkWorld->ResetFocusFilter();
}

void lwWorld::SetShowWorldFocusFilter(bool bShow, char* pTextureFile, float fAlpha)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetShowWorldFocusFilter(bShow, pTextureFile, fAlpha);
}

// 단순히 서서히 변하는 interface, keepfilter로 받는 이유는.. default가 false이기 때문이다.
void lwWorld::SetShowWorldFocusFilterTexAlpha(char* pTextureFile, float fAlphaStart, float fAlphaEnd, float fTime, bool bKeepFilter, bool bRenderObject )
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetShowWorldFocusFilter(pTextureFile, fAlphaStart, fAlphaEnd, fTime, bKeepFilter == false, bRenderObject );
}

// 단순히 서서히 변하는 interface (RGB)
void lwWorld::SetShowWorldFocusFilterColorAlpha(DWORD color, float fAlphaStart, float fAlphaEnd, float fTime, bool bKeepFilter, bool bRenderObject)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetShowWorldFocusFilter(color, fAlphaStart, fAlphaEnd, fTime, bKeepFilter == false, bRenderObject );
}

// 반복되는 interface, pTexture2가 없으면 꺼졌다 켜졌다 한다.
void lwWorld::SetShowWorldFocusFilterTexSwitch(char* pTextureFile1, char* pTextureFile2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetShowWorldFocusFilter(pTextureFile1, pTextureFile2, fAlpha1, fAlpha2, fTotalTime, fInterval);
}

// 반복되는 interface, pTexture2가 없으면 꺼졌다 켜졌다 한다.
void lwWorld::SetShowWorldFocusFilterColorSwitch(DWORD color1, DWORD color2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval)
{
	PG_ASSERT_LOG(m_pkWorld);
	if (m_pkWorld)
		m_pkWorld->SetShowWorldFocusFilter(color1, color2, fAlpha1, fAlpha2, fTotalTime, fInterval);
}
//!	현재 포커스 필터의 알파값을 얻어온다.
float	lwWorld::GetWorldFocusFilterAlpha()
{
	if(!m_pkWorld) return 0;

	return	m_pkWorld->GetWorldFocusFilterAlpha();
}

bool lwWorld::IsEndShowWorldFocusFilter()
{
	if(!m_pkWorld)	return true;
	return m_pkWorld->IsEndShowWorldFocusFilter();
}

lwPoint3 lwWorld::GetObjectPosByName(char const *pcName)
{
	NiAVObject *pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(pcName);
	if(!pkNode)
	{
		return lwPoint3(0, 0, 0);
	}
	return lwPoint3(pkNode->GetTranslate());
}

lwPoint3 lwWorld::GetObjectWorldPosByName(char const *pcName)
{
	NiAVObject *pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(pcName);
	if(!pkNode)
	{
		return lwPoint3(0, 0, 0);
	}
	return lwPoint3(pkNode->GetWorldTranslate());
}

lwPoint3 lwWorld::GetCameraPosByName(char const *pcName)
{
	NiCameraPtr spCamera = m_pkWorld->m_kCameraMan.GetCamera();
	if( spCamera )
	{
		//return lwPoint3(spCamera->GetWorldLocation());
		return lwPoint3(spCamera->GetTranslate());
	}
	return lwPoint3(0, 0, 0);
}
void lwWorld::Recv_PT_M_C_NFY_AREA_DATA(lwPacket Packet)
{
	assert(NULL);
//	Packet.
}

void lwWorld::Recv_PT_M_C_ADD_UNIT(lwPacket Packet)
{
	if (NULL == m_pkWorld || NULL == g_pkWorld)
		return;

	BM::Stream *pPacket = Packet();

	UNIT_PTR_ARRAY kUnitArray;

	kUnitArray.ReadFromPacket(*pPacket);

	NILOG(PGLOG_LOG, "[lwWorld] Recv_PT_M_C_ADD_UNIT (%d unit)\n", kUnitArray.size());
	UNIT_PTR_ARRAY::iterator unit_itor = kUnitArray.begin();

	int iCount = -1;

	static StatInfoF kAddUnitInfo;
	bool bFoundPlayer = false;
	while(unit_itor != kUnitArray.end())
	{//CUnit WriteToPacket 과 맞춘다
		CUnit *pkUnit = (*unit_itor).pkUnit;

		iCount++;
		if (pkUnit == NULL)
		{
			PG_ASSERT_LOG(pkUnit);
			++unit_itor;
			continue;
		}

		NILOG(PGLOG_LOG,"[lwWorld] Try to Add New Unit GUID:%s\n",MB(pkUnit->GetID().str()));

		if ( pkUnit->GetAbil(AT_HP) <= 0 )
		{
			EUnitType const kUnitType = pkUnit->UnitType();
			switch ( kUnitType )
			{
			case UT_MONSTER:
			case UT_BOSSMONSTER:
			case UT_OBJECT:
			case UT_SUMMONED:
				{
					NILOG( PGLOG_LOG, "Unit HP is 0. Add Unit Skipped %s\n", pkUnit->Name().c_str() );
					++unit_itor;
					continue;
				}break;
			default:
				{
				}break;
			}
		}

		if (g_pkWorld
			&& g_iUseAddUnitThread == 1)
		{// 유닛을 추가하는데
			bool bSuccess = false;
			PgPilot* pkPilot = NULL;
			pkPilot = g_kPilotMan.FindPilot(pkUnit->GetID());

			if(pkPilot != NULL 
			/*|| pkUnit->UnitType() == UT_BOSSMONSTER*/
			|| pkUnit->UnitType() == UT_ENTITY
			|| pkUnit->GetID() == g_kPilotMan.PlayerPilotGuid() )	
			/*|| pkUnit->GetAbil(AT_GRADE) >= EMGRADE_ELITE*/ 
			{// 이미 존재하거나, Entity거나, 자신의 Player일 이면 유닛을 생성하지 않고 추가하고
				bSuccess = (g_pkWorld->AddUnit(pkUnit) != NULL);
			}
			else
			{// 존재하지 않는다면, 유닛을 생성해서 추가 하는데
				bSuccess = g_kAddUnitThread.DoAddUnit(pkUnit, g_pkWorld);
			}

			if (bSuccess)
			{
				unit_itor->bAutoRemove = false;
				unit_itor = kUnitArray.erase(unit_itor);
				NILOG(PGLOG_LOG, "[lwWorld] DoAddUnit %d queued Guid[%s]\n", iCount, MB(pkUnit->GetID().str()));
			}
			else
			{
				++unit_itor;
				if (pkPilot != NULL || pkUnit->UnitType() == UT_BOSSMONSTER)
				{
					NILOG(PGLOG_ERROR, "[lwWorld] DoAddUnit %d failed Guid[%s]\n", iCount, MB(pkUnit->GetID().str()));
				}
			}
		}
		else
		{
			float fTime = NiGetCurrentTimeInSec();
			if(g_pkWorld
				&& g_pkWorld->AddUnit(pkUnit)
				)
			{
				unit_itor->bAutoRemove = false;
				unit_itor = kUnitArray.erase(unit_itor);
				NILOG(PGLOG_LOG, "[lwWorld] g_pkWorld->ADDUNIT %d success Guid[%s]\n", iCount, MB(pkUnit->GetID().str()));
			}
			else
			{
				++unit_itor;
				NILOG(PGLOG_ERROR, "[lwWorld] g_pkWorld->AddUnit %d failed Guid[%s]\n", iCount, MB(pkUnit->GetID().str()));
			}
			fTime = NiGetCurrentTimeInSec() - fTime;
			UpdateStatInfo(kAddUnitInfo, fTime);
			PrintStatInfo(kAddUnitInfo, "[lwWorld] AddUnit");
		}

		if(!bFoundPlayer 
			&& g_kPilotMan.IsMyPlayer(pkUnit->GetID())
			)
		{// 플레이어 정보
			bFoundPlayer = true;
			lwShowRestExpBar();	//휴식 경험치 UI 설정
		}
	}
	
	{//인벤개선-신규획득아이템 표시기능용 초기화코드
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		PgInventory* pkInv = pkPlayer->GetInven();
		if(pkInv && !pkInv->IsEmpty())
		{
			lwInventory::InitCompInv( pkInv );
		}
	}

	//g_kAMPool.SetThreadPriority(NiThread::LOWEST);
/*
	-- 월드에 캐릭터를 추가한다.
	local pilot = g_kPilotMan.NewPilot(charInfo.guidPilot, Pilot_FindXmlPath(charInfo.gender, charInfo.class), "pc")
	if pilot:IsNil() == true then
		pilot = g_pilotMan:FindPilot(charInfo.guidPilot)
		pilot:ReadPacket(charInfo.guidPilot, packet, 4,charInfo.class)	-- My Character
		return false
	end
*/
}

#define PG_USE_OLD_CHASEL
void lwWorld::Recv_AddReadyCharacter(lwPacket Packet)
{
	g_kNowGroundKey.Clear();
	BM::Stream *pPacket = Packet();

	//CUnit WriteToPacket 과 맞춘다
	UNIT_PTR_ARRAY kUnitArray;
	UNIT_PTR_ARRAY kInsertUnitArray;	//적용 완료된 유닛 컨테이너
	CONT_DB_ITEM_LIST kItemList;

	kUnitArray.ReadFromPacket(*pPacket);
	pPacket->Pop(kItemList);

	short iFlag = SOF_NONE;
	pPacket->Pop(iFlag);
	g_kGlobalOption.SetSendOptionFlag(iFlag);

	//프리미엄 서비스
	SMemberPremiumData kPremiumData;
	bool bPremium = false;
	pPacket->Pop(bPremium);
	if(bPremium)
	{
		pPacket->Pop(kPremiumData.iServiceNo);
		pPacket->Pop(kPremiumData.kStartDate);
		pPacket->Pop(kPremiumData.kEndDate);
	}
	g_kPilotMan.MemberPremium(kPremiumData);

	PgPremiumMgr kPremiumMgr;
	kPremiumMgr.SetService(kPremiumData.iServiceNo, BM::Stream());
	kPremiumMgr.EndDate(kPremiumData.kEndDate);
	lua_tinker::call<void,float>("PremiumReminAlram", kPremiumMgr.GetEndSecTime());

	//점핑 캐릭터 이벤트
	bool bJumpingEvent = false;
	int iJumping_EventNo = 0;
	int iJumping_RemainRewardCount = 0;
	pPacket->Pop(bJumpingEvent);
	if(bJumpingEvent)
	{
		pPacket->Pop(iJumping_EventNo);
		pPacket->Pop(iJumping_RemainRewardCount);
	}
	g_kPilotMan.SetJumpingEvent(iJumping_EventNo, iJumping_RemainRewardCount);

	PgSelectStage *pkSelectStage = &g_kSelectStage;//m_pkWorld->m_pkSelectStage;
	pkSelectStage->ResetSpawnSlot();//스폰에 있던것 다 지워.

	UNIT_PTR_ARRAY::iterator unit_itor = kUnitArray.begin();
	while(unit_itor != kUnitArray.end())
	{//넣는거 따로.
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>((*unit_itor).pkUnit);
		BM::GUID const &guidPilot = pkPlayer->GetID();

		if(pkPlayer)
		{
			BYTE const kState = pkPlayer->GetDBPlayerState();
			bool bDeleteWait = (( kState & CIDBS_ReserveDelete) == CIDBS_ReserveDelete)?(true):(false);
			if( !bDeleteWait )
			{
				bDeleteWait = (( kState & CIDBS_NeedRename) == CIDBS_NeedRename)?(true):(false);
			}
			short const iLevel = static_cast<short>(pkPlayer->GetAbil(AT_LEVEL));
			int iSlot = pkPlayer->GetAbil(AT_CHARACTOR_SLOT);

			if(0 < iSlot && iSlot < 9)
			{//원래 만들어진 캐릭터 먼저 넣어주자
				if(pkSelectStage->AddToSlot(SSelectCharKey(guidPilot, pkPlayer->BirthDate(), iLevel, iSlot-1), bDeleteWait))
				{
					kInsertUnitArray.push_back( (*unit_itor) );
					unit_itor->bAutoRemove = false;
					unit_itor = kUnitArray.erase(unit_itor);
					continue;
				}
			}
		}
		++unit_itor;
	}	

	unit_itor = kUnitArray.begin();
	while(unit_itor != kUnitArray.end())
	{//겹치는 슬롯의 경우 차례대로 넣어주자
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>((*unit_itor).pkUnit);
		BM::GUID const &guidPilot = pkPlayer->GetID();

		if(pkPlayer)
		{
			BYTE const kState = pkPlayer->GetDBPlayerState();
			bool bDeleteWait = (( kState & CIDBS_ReserveDelete) == CIDBS_ReserveDelete)?(true):(false);
			if( !bDeleteWait )
			{
				bDeleteWait = (( kState & CIDBS_NeedRename) == CIDBS_NeedRename)?(true):(false);
			}
			short const iLevel = static_cast<short>(pkPlayer->GetAbil(AT_LEVEL));
			if( -1 == pkSelectStage->GetSpawnSlot(guidPilot) )
			{
				int iSlot = 1;
				if( bDeleteWait )
				{
					while( BM::GUID::NullData() != pkSelectStage->GetDeleteActor(iSlot) )
					{
						++iSlot;
					}
				}
				else
				{
					while( BM::GUID::NullData() != pkSelectStage->GetSpawnActor(iSlot) )
					{
						++iSlot;
					}
				}
				if(pkSelectStage->AddToSlot(SSelectCharKey(guidPilot, pkPlayer->BirthDate(), iLevel, iSlot-1), bDeleteWait))
				{
					kInsertUnitArray.push_back( (*unit_itor) );
					unit_itor->bAutoRemove = false;
					unit_itor = kUnitArray.erase(unit_itor);
					continue;
				}
				else
				{
					BM::Stream kPacket(PT_C_S_REQ_SAVE_CHARACTOR_SLOT);
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(iSlot);
					NETWORK_SEND_TO_SWITCH(kPacket);

					pkSelectStage->SetSelectedActor(iSlot-1);
				}
			}
		}
		++unit_itor;
	}
	
	int const iSelectSlot = pkSelectStage->GetSelectedSlot();
	BM::GUID kSelectGuid = pkSelectStage->GetSpawnActor(iSelectSlot+1);
	if( kSelectGuid.IsNull() )
	{
		pkSelectStage->SelectDefaultCharacter();
	}

	unit_itor = kInsertUnitArray.begin();
	while(unit_itor != kInsertUnitArray.end())
	{//넣은거 가공 따로.
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>((*unit_itor).pkUnit);
		if(	pkPlayer )
		{
			BM::GUID const &guidPilot = pkPlayer->GetID();
			// 이미 추가되어 있는 캐릭터는 넣지 않는다.
			if(pkSelectStage->GetSpawnSlot(guidPilot) != -1 || pkSelectStage->CheckDeleteWaitSlot(guidPilot))//캐릭 있음.
			{
				int iSpawnSlot = pkSelectStage->GetSpawnSlot(guidPilot);
				NiPoint3 kSpawnLoc = pkSelectStage->GetSpawnPoint(iSpawnSlot);
				pkPlayer->SetPos(POINT3(kSpawnLoc.x, kSpawnLoc.y, kSpawnLoc.z));
				PgActor *pkActor = dynamic_cast<PgActor *>(m_pkWorld->AddUnit(pkPlayer));

				if(pkActor)
				{
					pkActor->SetFreeMove(true);
					kSpawnLoc.z += PG_CHARACTER_Z_ADJUST;
					pkActor->SetPosition(kSpawnLoc);
					pkActor->SetGroupNo(OGT_PLAYER);

					PgInventory *pkInv = pkPlayer->GetInven();
					CONT_DB_ITEM_LIST::const_iterator item_Itor = kItemList.begin();
					while(item_Itor != kItemList.end())
					{	
						if(pkPlayer->GetID() == (*item_Itor).OwnerGuid)
						{
							SItemPos const kItemPos((*item_Itor).InvType, (*item_Itor).InvPos);

							PgBase_Item kItem;
							kItem.Guid((*item_Itor).ItemGuid);
							kItem.ItemNo((*item_Itor).ItemNo);
							kItem.Count((*item_Itor).Count);

							SEnchantInfo kEnInfo;
							kEnInfo.Field_1((*item_Itor).Enchant_01);
							kEnInfo.Field_2((*item_Itor).Enchant_02);
							kEnInfo.Field_3((*item_Itor).Enchant_03);
							kEnInfo.Field_4((*item_Itor).Enchant_04);

							kItem.EnchantInfo(kEnInfo);
							kItem.State((*item_Itor).State);

							pkInv->Modify(kItemPos, kItem);

							bool const bEquipRet = pkActor->EquipItemByPos(static_cast<EInvType>(kItemPos.x),static_cast<EEquipPos>(kItemPos.y));
						}
						++item_Itor;
					}
					pkActor->UpdateName();	// 아이템이 적용된 상태에서 업적 붙여줄수 있게함
				}
				unit_itor->bAutoRemove = false;
				unit_itor = kInsertUnitArray.erase(unit_itor);
			}
			else
			{
				lwPilot kPilot(g_kPilotMan.FindPilot(guidPilot));
				if( !kPilot.IsNil() )
				{
					lwActor kActor = kPilot.GetActor();
					kActor.SeeFront(true, true);
					PgActor* pkActor = kActor();

					if (pkActor)
					{
						pkActor->DoReservedAction(PgActor::RA_INTRO_IDLE);
						PgAction *pkAction = pkActor->GetAction();
						if(pkAction)
						{
							pkAction->SetSlot(0);
						}
						pkActor->PlayCurrentSlot();
					}
				}
				++unit_itor;
			}
		}
	}
}

extern lwUIWnd lwCallMissionRetryUI();
extern void Recv_PT_M_C_NFY_EMPORIA_FUNCTION( PgWorld *pkWorld, BM::Stream &rkPacket );

float	lwWorld::GetServerElapsedTime()
{
	DWORD const dwElapsedTime = g_kEventView.GetServerElapsedTime();	// 1000 = 1s

	float	fElapsedTime = dwElapsedTime/1000.0f;
	return	fElapsedTime;
}
//__int64 g_iMapAccTime = 0;
void lwWorld::Recv_PT_M_C_NFY_MAPLOADED(lwPacket Packet)
{
	static __int64 iElapsedTime = 0;

	BM::Stream *pkPacket = Packet();

	m_pkWorld->ShineStoneCountContReset();

	CONT_RESERVED_STONE_LIST::mapped_type kContMyStone;
	pkPacket->Pop(m_dwMapState);
	pkPacket->Pop(kContMyStone);
	
	CONT_RESERVED_STONE_LIST::mapped_type::iterator my_stone_itor = kContMyStone.begin();
	while(my_stone_itor != kContMyStone.end())
	{
		m_pkWorld->AddShineStoneToWorld(my_stone_itor->first, my_stone_itor->second);
		++my_stone_itor;
	}

	// Ready World
	if ( !m_pkWorld->GetReadyScript().empty() )
	{
		if(true == m_pkWorld->IsHaveAttr(GATTR_FLAG_BOSS))
		{
			PgEnergyGauge::ms_bDrawEnergyGaugeBar = false;
		}
		lua_tinker::call<void, lwWorld>(m_pkWorld->GetReadyScript().c_str(), lwWorld(m_pkWorld) );
	}

	// World의 애니 물체의 시간 동기화.
	NiNode* pkWorldRoot = m_pkWorld->GetSceneRoot();
	if (pkWorldRoot)
	{
		//SYSTEMTIME* pkSystemTime;
		//SYSTEMTIME* pkSystemTime2;
		//g_kEventView.GetLocalTime(pkSystemTime);
		//g_kEventView.GetGameTime(pkSystemTime2);

		unsigned __int64 iElapsedTime = g_kEventView.GetServerElapsedTime();	// 1000 = 1s
//		iElapsedTime = iElapsedTime;
//
//		// Calc Accum Time
//		float fDeltaTime = NiGetCurrentTimeInSec() - g_pkApp->GetLastTime();
//		if (fDeltaTime < 0.0f)
//			fDeltaTime = 0.0f;
//		float fAccumTime = g_pkApp->GetAccumTime() + fDeltaTime;
////		g_iMapAccTime = (float)fAccumTime;
//
//		iElapsedTime -= (unsigned __int64)(fAccumTime * 1000);
		PgWorld::IncTimeToAniObj(pkWorldRoot, iElapsedTime);
	}

	PgActor* pkActor = g_kPilotMan.PlayerActor();
	if (pkActor)
	{
		if (pkActor->GetAction())
		{
			std::string strName = pkActor->GetAction()->GetID();
			if (strName == "a_die")
			{
				PgPilot* pkPilot = g_kPilotMan.PlayerPilot();
				if (pkPilot != NULL)	// 살려야되잖아~
				{
					pkActor->AddEffect(ACTIONEFFECT_REVIVE, 0, 0, pkPilot->GetGuid(), 0, 0, true );
				}
			}
		}

		pkActor->ResetSkillCoolTimeFromUnit();
		pkActor->DoReservedAction(PgActor::RA_IDLE,true);
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		PgActor* pPet = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
		if(pPet)
		{
			pPet->ResetSkillCoolTimeFromUnit();
		}
	}

	SGroundOwnerInfo kGroundOwnerInfo;
	kGroundOwnerInfo.ReadFromPacket( *pkPacket );
	m_pkWorld->GroundOwnerInfo( kGroundOwnerInfo );

	// Big Area 유닛의 정보가 뒤에 포함되어 온다.
	Recv_PT_M_C_ADD_UNIT( Packet );

	// 월드 이벤트를 싱크한다.
	g_kWorldEventClientMgr.SyncFromServer( *pkPacket );
	g_kWEClientObjectMgr.SyncFromServer( *pkPacket );

	m_pkWorld->DynamicGndAttr( static_cast< EDynamicGroundAttr >(Packet.PopInt()) );

	{
		size_t iCount = 0;
		std::string kTriggerID;
		bool bEnable = false;
		pkPacket->Pop( iCount );
		while( iCount-- )
		{
			pkPacket->Pop( kTriggerID );
			pkPacket->Pop( bEnable );
			PgWorldUtil::SetTriggerEnable(m_pkWorld, kTriggerID, bEnable);
		}
	}
	DWORD dwWorldEnvStatus = 0;
	pkPacket->Pop( dwWorldEnvStatus );	m_pkWorld->OnWorldEnvironmentStatusChanged(dwWorldEnvStatus,true);
	{
		CONT_REALM_QUEST_INFO kContRealmQuest;
		PU::TLoadTable_AM(*pkPacket, kContRealmQuest);
		CONT_REALM_QUEST_INFO::const_iterator iter = kContRealmQuest.begin();
		while( kContRealmQuest.end() != iter )
		{
			g_kQuestMan.UpdateRealmQuestCount((*iter).second);
			++iter;
		}
	}

	// Emporia Portal 정보를 로드한다.
	Recv_PT_M_C_NFY_EMPORIA_FUNCTION( m_pkWorld, *pkPacket );

	// 후처리
	//
	PgEventQuestUI::CheckNowEvent(); // 이벤트 퀘스트 UI 갱신

	// 현재 채널의 권장레벨을 시스템 메세지로 출력
	if( g_kNetwork.IsFirstConnect() )	// 채널 입장 최초에 한번
	{
		PgRealm	Realm;
		CONT_CHANNEL kContChannel;
		if( S_OK == g_kRealmMgr.GetRealm(g_kNetwork.NowRealmNo(), Realm) )
		{
			Realm.GetChannelCont(kContChannel);
		}

		if( kContChannel.size() )
		{
			const short iChannelNo = static_cast<short>( g_kNetwork.NowChannelNo() );
			CONT_CHANNEL::const_iterator	channel_iter = kContChannel.find(iChannelNo);
			if( channel_iter != kContChannel.end() )
			{
				std::wstring kStrMsg;
				kStrMsg.clear();
				kStrMsg = channel_iter->second.Notice_InGame();

				if( kStrMsg.empty() || kStrMsg.size() < 1 )
				{
					g_kNetwork.SetFirstConnect(false);
					return ;
				}

				SChatLog kSystemChat( CT_EVENT_GAME );
				g_kChatMgrClient.AddLogMessage( kSystemChat, kStrMsg );

				g_kNetwork.SetFirstConnect(false);
			}
		}	
	}

	switch ( m_pkWorld->GetAttr() )
	{
	case GATTR_HARDCORE_DUNGEON:
		{
			__int64 i64EndTime = 0i64;
			if ( true == pkPacket->Pop( i64EndTime ) )
			{
				if ( true == PgHardCoreDungeon::UpdateRemainTime( i64EndTime ) )
				{
					BM::GUID	kPartyGuid;
					pkPacket->Pop( kPartyGuid );
					pkPacket->Pop( i64EndTime );
					PgHardCoreDungeon::UpdateBossGndInfo( kPartyGuid, i64EndTime );
				}
			}
		}break;
	case GATTR_BATTLESQUARE:
		{
			g_kBattleSquareMng.ReadFromPacket( *pkPacket );
			g_kBattleSquareMng.CallTeamUI();
			g_kBattleSquareMng.Init();
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, true)); // 움직임 금지
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, false));
			lua_tinker::call< void >("OnCallBattleSquareMiniMap");
		}break;
	case GATTR_SUPER_GROUND:
		{
			PgSuperGroundUI::ReadFromMapMove(*pkPacket);
		}break;
	case GATTR_ELEMENT_GROUND:
		{
			PgSuperGroundUI::ReadFromMapMove(*pkPacket);
		}break;
	default:
		{
			g_kProgressMap.CallUI(false);
			g_kBattleSquareMng.Clear();
			PgSuperGroundUI::Clear();
		}break;
	}
	if(m_pkWorld
		&& m_pkWorld->CanCallDungeonExistUI() 
		)
	{
		lwActivateUI("FRM_ID_EXIT");
	}
	
	lua_tinker::call< void,int >("CheckNewQuestNfy",2);	// 퀘스트 알림아이콘 체크

	PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_BEGIN>()(*pkPacket);

	switch ( m_pkWorld->GetAttr() )
	{
	case GATTR_CONSTELLATION_GROUND:
		{
			lua_tinker::call<void>("CallConstellationStartUI");
			lua_tinker::call<void>("CallConstellationUI");
		}break;
	case GATTR_CONSTELLATION_BOSS:
		{
			lua_tinker::call<void>("CallConstellationUI");
		}break;
	}

	// 맵 로딩이 완료 됐다.
	BM::Stream NfyPacket(PT_C_T_NFY_MAPMOVE_TO_PARTYMGR);
	NETWORK_SEND(NfyPacket);
}

lwWString lwWorld::GetMissionTrigIDOnThisMap(void)
{ //현재 맵에서 미션트리거가 1개 존재한다면 아이디를 반환 해준다. 트리거가 2개 이상 존재한다면 무시.
	CONT_TRIGGER_INFO kContTrigInfo;
	m_pkWorld->GetTriggerByType(PgTrigger::TRIGGER_TYPE_MISSION, kContTrigInfo);
	if(kContTrigInfo.size() == 1)
	{
		PgTrigger* pkTrigger = *(kContTrigInfo.begin());
		std::wstring kTemp = UNI(pkTrigger->GetID());
		return lwWString(kTemp);
	}
	else if(kContTrigInfo.size() > 1)
	{ //미션 트리거가 여러개 존재 한다면 같은 미션 트리거인지 검사하고 같은 미션 트리거가 여러개라면 첫번째것을 사용
		CONT_TRIGGER_INFO::const_iterator iterTrig = kContTrigInfo.begin();
		int const iMissionNum = (*iterTrig)->Param();
		bool bSuccess = true;
		for( ; iterTrig != kContTrigInfo.end(); ++iterTrig)
		{
			PgTrigger* pkTrigger = *iterTrig;
			if(iMissionNum != pkTrigger->Param())
			{
				bSuccess = false;
			}
		}

		if(bSuccess)
		{
			PgTrigger* pkTrigger = *(kContTrigInfo.begin());
			std::wstring kTemp = UNI(pkTrigger->GetID());
			return lwWString(kTemp);
		}
	}
	return lwWString(L"");
}

void lwWorld::RemoveAllObject(bool bExceptNPC)
{
	m_pkWorld->RemoveAllObject(bExceptNPC);
}
void lwWorld::RemoveAllObjectExceptMe()
{
	m_pkWorld->RemoveAllObjectExceptMe();
}
void lwWorld::RemoveAllMonster()
{ 
	m_pkWorld->RemoveAllMonster(); 
}

void lwWorld::RemoveAllGradeMonster(int const iGrade)
{ 
	m_pkWorld->RemoveAllMonster(iGrade); 
}

bool lwWorld::CheckPlayerInRange(lwActor kActor, float fRange)
{
	if (kActor.IsNil())
	{
		return false;
	}
	NiPoint3 kCenter = kActor.GetPos()();

	PgWorld::ObjectContainer kContainer;
	m_pkWorld->GetContainer(PgIXmlObject::ID_PC, kContainer);
	if (kContainer.empty())
	{
		return false;
	}
	PgWorld::ObjectContainer::const_iterator itr = kContainer.begin();
	while (kContainer.end() != itr)
	{
		PgIWorldObject* pkOther = itr->second;
		if (pkOther)
		{
			const PgPilot* pkPilot = pkOther->GetPilot();
			if (pkPilot)
			{
				POINT3 kOtherVec = pkPilot->GetUnit()->GetPos();
				POINT3 kCenterVec(kCenter.x, kCenter.y, kCenter.z);
				float const fDist = kOtherVec.Distance(kOtherVec, kCenterVec);
				if (fDist > fRange)
				{
					return false;
				}
			}
		}
		++itr;
	}

	return true;
}

bool lwWorld::RemoveCameraWall(char const* pkName)
{
	PG_ASSERT_LOG(pkName);

	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW)
	{
		PgCameraModeFollow *pkFollowCam = (PgCameraModeFollow*)m_pkWorld->m_kCameraMan.GetCameraMode(); //TODO:위험!!!!!!

		if (pkFollowCam)
		{
			return pkFollowCam->RemoveCameraWall(pkName);
		}
	}
	
	return false;
}

bool lwWorld::RecoverCameraWall(char const* pkName)
{
	PG_ASSERT_LOG(pkName);

	if(m_pkWorld->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW)
	{
		PgCameraModeFollow *pkFollowCam = (PgCameraModeFollow*)m_pkWorld->m_kCameraMan.GetCameraMode(); //TODO:위험!!!!!!

		if (pkFollowCam)
		{
			return pkFollowCam->RecoverCameraWall(pkName);
		}
	}

	return false;
}

void lwWorld::SetBoneLODLevel(int iLODLevel, int iType)
{
	if (m_pkWorld)
	{
		PgWorld::ObjectContainer kContainer;
		if (iType == 0)
			m_pkWorld->GetContainer(PgIXmlObject::MAX_XML_OBJECT_ID, kContainer);
		else if (iType == 1)
			m_pkWorld->GetContainer(PgIXmlObject::ID_PC, kContainer);
		else if (iType == 2)
			m_pkWorld->GetContainer(PgIXmlObject::ID_MONSTER, kContainer);
		else if (iType == 3)
			m_pkWorld->GetContainer(PgIXmlObject::ID_NPC, kContainer);
			
		if (kContainer.empty())
		{
			return;
		}

		PgWorld::ObjectContainer::const_iterator itr = kContainer.begin();
		while (kContainer.end() != itr)
		{
			PgIWorldObject* pkObject = itr->second;
			if (pkObject)
			{
				int iLODCount = pkObject->GetLODCount();
				if (iLODCount > iLODLevel)
				{
					pkObject->SetLOD(iLODLevel);
				}
			}
			++itr;
		}
	}
}

void lwWorld::SetWorldLODLevel(int iLODLevel)
{
	NiLODNode::SetGlobalLOD(iLODLevel);
}

void lwWorld::SetWorldLODData(float first, float second, float third)
{
	if (m_pkWorld)
	{
		m_pkWorld->SetWorldLODData(first, second, third);
	}
}

void lwWorld::SetDoFixedStep(bool bUse)
{
	if (m_pkWorld)
	{
		m_pkWorld->GetPhysXScene()->SetDoFixedStep(bUse);
		m_pkWorld->GetPhysXScene()->GetPhysXScene()->setTiming(m_pkWorld->GetPhysXScene()->GetTimestep(), 1, bUse ? NX_TIMESTEP_FIXED : NX_TIMESTEP_VARIABLE);
	}
}

void lwWorld::SetPhysXDebug(bool bDebug)
{
	g_bUseDebugInfo = bDebug;
	if (m_pkWorld)
	{
		NiPhysXManager* pkManager = NiPhysXManager::GetPhysXManager();
		if(bDebug)
		{
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
		}
		else
		{
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZATION_SCALE, 0.0f);
			pkManager->m_pkPhysXSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 0.0f);
		}

		m_pkWorld->GetPhysXScene()->SetDebugRender(bDebug,m_pkWorld->GetSceneRoot());
	}
}

bool lwWorld::DetachObjectByName(char const* szObjectName)
{
	if (m_pkWorld)
	{
		return m_pkWorld->DetachObjectByName(szObjectName);
	}

	return false;
}

void lwWorld::SetMaxIter(int iMaxIter)
{
	if (m_pkWorld)
	{
		m_pkWorld->GetPhysXScene()->GetPhysXScene()->setTiming(m_pkWorld->GetPhysXScene()->GetTimestep(), iMaxIter, m_pkWorld->GetPhysXScene()->GetDoFixedStep() ? NX_TIMESTEP_FIXED : NX_TIMESTEP_VARIABLE);
	}
}

void lwWorld::SetActivePhysXGroup(const PgIXmlObject::XmlObjectID eType, int iGroup, bool bEnable)
{
	PgWorld::ObjectContainer kObjContainer;
	if(!m_pkWorld || !m_pkWorld->GetContainer(eType, kObjContainer))
	{
		return;
	}
	
	for(PgWorld::ObjectContainer::iterator itr = kObjContainer.begin();
		itr != kObjContainer.end();
		++itr)
	{
		if(eType == PgIXmlObject::ID_PC || eType == PgIXmlObject::ID_NPC || eType == PgIXmlObject::ID_MONSTER)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(itr->second);
			if(!pkActor)
			{
				continue;
			}

			pkActor->SetActiveGrp(iGroup, bEnable);
		}
	}
}
int	lwWorld::GetDifficultyLevel()
{
	return	m_pkWorld->GetDifficultyLevel();
}
void	lwWorld::SetDifficultyLevel(int iLevel)
{
	if(m_pkWorld)
	{
		m_pkWorld->SetDifficultyLevel(iLevel);
		lua_tinker::call<void, int>("SetMissionDifficultyLevel", iLevel );
	}
}

lwTrigger lwWorld::GetTriggerByID(char const *kTriggerID)
{
	return	lwTrigger(m_pkWorld->GetTriggerByIDWithIgnoreCase(kTriggerID));
}
lwTrigger lwWorld::GetTriggerByIndex(int iIndex)
{
	return lwTrigger(m_pkWorld->GetTriggerByIndex(iIndex));
}

void lwWorld::AddDrawActorFilter(lwGUID kActorGuid)
{
	if(m_pkWorld)
	{
		m_pkWorld->AddDrawActorFilter(kActorGuid());
	}
}

void lwWorld::ClearDrawActorFilter()
{
	if(m_pkWorld)
	{
		m_pkWorld->ClearDrawActorFilter();
	}
}

void lwWorld::SetActivaingWorldMap(bool bActivating)
{
	if(m_pkWorld)
	{
		m_pkWorld->IsActivateWorldMap(bActivating);
	}
}

void lwWorld::SetUpdateWorld( bool bUpdate )
{
	if ( m_pkWorld )
	{
		m_pkWorld->IsUpdate( bUpdate );
	}
}

void lwWorld::SetSlowMotion(int const iType, float const fFrom, float const fTo, float const fTime)
{
	if ( m_pkWorld )
	{
		m_pkWorld->SetSlowMotion( static_cast<SlowMotionType>(iType), fFrom, fTo, fTime );
	}
}

void lwWorld::SetCameraAdjustInfo(float fUp, float fZoom, float fTargetZ, float fMinZoom, float fMaxZoom)
{
	if ( m_pkWorld )
	{
		CameraAdjustInfo kCameraAdjustInfo(m_pkWorld->m_kCameraAdjustInfo);
		kCameraAdjustInfo.kCameraAdjust.fCameraHeight = fUp;
		kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget = -1.0f * fZoom;
		kCameraAdjustInfo.kCameraAdjust.fLookAtHeight = fTargetZ;
		kCameraAdjustInfo.fMinZoomPos = fMinZoom;
		kCameraAdjustInfo.fMaxZoomPos = fMaxZoom;
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_pkWorld->m_kCameraMan.GetCameraMode());
		if (pkCameraMode)
		{
			pkCameraMode->SetCameraAdjustInfo(kCameraAdjustInfo);
		}
	}
}

void lwWorld::RecoverCameraAdjustInfo()
{
	if ( m_pkWorld )
	{
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_pkWorld->m_kCameraMan.GetCameraMode());
		if (pkCameraMode)
		{
			pkCameraMode->SetCameraAdjustInfo(m_pkWorld->m_kCameraAdjustInfo);
		}
	}
}

void lwWorld::AddShineStoneCount(unsigned long const dwStoneID)
{
	
}


//void lwWorld::AddPetToWorld(lwGUID kGUID, lwGUID kMasterGUID, lwPoint3 kPosition, int iID)
//{
//	m_pkWorld->AddPetToWorld(kGUID(), kMasterGUID(), kPosition(), iID);
//}
	
int lwWorld::GetCameraMove_ID()
{
	PgCameraModeMovie *pkMovieCam = dynamic_cast<PgCameraModeMovie*>(m_pkWorld->m_kCameraMan.GetCameraMode());
	if(pkMovieCam)
	{
		return pkMovieCam->GetSequenceID();
	}

	return 0;
}

lwGUID lwWorld::GetOldMouseOverObject()
{
	if(!g_pkWorld)
	{
		return 0;
	}
	PgIWorldObject* pObject = g_pkWorld->GetOldMouseOverObject();
	if(pObject)
	{
		return lwGUID(pObject->GetGuid());
	}
	return 0;
}

bool lwWorld::IsMapMoveCompleteFade() const
{
	if(m_pkWorld)
	{
		return m_pkWorld->IsMapMoveCompleteFade();
	}
	return false;
}

void lwWorld::MapMoveCompleteFade()
{
	if(m_pkWorld)
	{
		m_pkWorld->MapMoveCompleteFade();
	}
}

void lwWorld::SaveHideActorClassNo(int const iClassNo, bool const bHide)
{
	if(m_pkWorld && iClassNo)
	{
		m_pkWorld->SaveHideActor(iClassNo, bHide);
	}
}

void lwWorld::SaveHideActorType(int const iType, bool const bHide)
{
	if(m_pkWorld && iType)
	{
		m_pkWorld->SaveHideActor(static_cast<EUnitType>(iType), bHide);
	}
}

void lwWorld::ClearSaveHideActor()
{
	if(m_pkWorld)
	{
		m_pkWorld->ClearSaveHideActor();
	}
}

bool lwWorld::CheckEventScriptOnEnter()
{
	bool bResult = true;
	if( m_pkWorld->m_kEventScriptOnEnter.m_iEventScriptIDOnEnter )
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(pPlayer)
		{
			if( m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Class.size() )
			{//클래스 체크
				bResult = false;
				int const iClass = pPlayer->GetAbil(AT_CLASS);
				std::list<int>::const_iterator iter_Class = m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Class.begin();
				while( m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Class.end() != iter_Class )
				{
					if( (*iter_Class) == iClass )
					{
						bResult= true;
						break;
					}
					++iter_Class;
				}
			}
			if( bResult && m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Quest.size() )
			{// 퀘스트 체크
				bResult = false;
				std::list<int>::const_iterator iter_Quest = m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Quest.begin();
				while( m_pkWorld->m_kEventScriptOnEnter.m_kContLimit_Quest.end() != iter_Quest )
				{
					if( pPlayer->GetQuestState( (*iter_Quest) ) )
					{
						bResult= true;
						break;
					}
					++iter_Quest;
				}
			}
			if( bResult && m_pkWorld->m_kEventScriptOnEnter.m_iLimitLevelMax )
			{// 최대레벨제한 체크
				bResult = false;
				int const iLevel = pPlayer->GetAbil(AT_LEVEL);
				if( m_pkWorld->m_kEventScriptOnEnter.m_iLimitLevelMax >= iLevel )
				{
					bResult= true;
				}
			}
			if( bResult && m_pkWorld->m_kEventScriptOnEnter.m_iLimitLevelMin )
			{// 최소레벨제한 체크
				bResult = false;
				int const iLevel = pPlayer->GetAbil(AT_LEVEL);
				if( m_pkWorld->m_kEventScriptOnEnter.m_iLimitLevelMin <= iLevel )
				{
					bResult= true;
				}
			}
			if( bResult && m_pkWorld->m_kEventScriptOnEnter.m_bPlayOnce )
			{//캐릭 생성 후 한번만 띄워 주는지 체크
				//맵 한번이라도 들어갔는지 확인하고 한번도 안들어갔으면 bResult=true 해주면 됨
				//월드맵에서 들어간 맵은 색깔이 다른데 그거 체크하는 부분 확인해서 작업하면 됨.
				bResult = false;	//한번은 해줘야 함...
				bResult = lua_tinker::call<bool>("GetVisitFirstMap");
			}
		}
	}
	return bResult;
}

void lwWorld::InitDmgNum()
{
	if(m_pkWorld)
	{
		if( m_pkWorld->m_pkDamageNumMan )
		{
			m_pkWorld->m_pkDamageNumMan->Init();
		}
	}
}

bool lwWorld::IsExistMiniMapData() const
{
	if(m_pkWorld)
	{
		return m_pkWorld->IsExistMiniMapData();
	}
	return false;
}

void lwWorld::SetCurProgress(int iProgress)
{
	g_kProgressMap.SetCurPos(iProgress);

	BM::GUID kMyGuid;
	g_kPilotMan.GetPlayerPilotGuid(kMyGuid);

	BM::Stream kPacket(PT_C_M_NFY_PROGRESS_POS, iProgress);
	NETWORK_SEND(kPacket); //현재 내 미니맵 위치를 서버에 알리고 저장
}

int lwWorld::GetCurProgress(void) const
{
	return g_kProgressMap.GetCurPos();
}


lwPoint2 lwWorld::GetMapThumbnailImgSize(void)
{
	PROGRESS_MAP_UI_INFO const& rkMapInfo = g_kProgressMap.GetInfo();
	return lwPoint2(rkMapInfo.iImgW, rkMapInfo.iImgH);
}


lwWString lwWorld::GetMapThumbnailImgName(void)
{
	PROGRESS_MAP_UI_INFO const& rkMapInfo = g_kProgressMap.GetInfo();
	if(rkMapInfo.wstrImgPath.empty())
	{
		return lwWString(_T(""));
	}

	std::wstring wsThumbnailImageName = rkMapInfo.wstrImgPath;
	std::wstring::size_type iFindPos = wsThumbnailImageName.find_last_of(_T(".tga"));
	if(iFindPos != 0)
	{
		wsThumbnailImageName.insert(iFindPos - 3, _T("_a"));
	}
	return lwWString(wsThumbnailImageName.c_str());
}

