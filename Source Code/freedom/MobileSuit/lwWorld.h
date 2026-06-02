#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWORLD_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWORLD_H

#include "lwGUID.h"
#include "lwActor.h"
#include "lwParticle.h"
#include "lwPoint3.h"
#include "lwQuaternion.h"
#include "lwDropBox.h"
#include "lwWorldActionMan.h"
#include "lwPuppet.h"
#include "lwHome.h"
#include "lwBaseItemSet.h"
#include "lwSelectStage.h"
#include "lwTrigger.h"

class PgWorld;
class PgPvP;
class PgIWorldObject;
class lwUIWnd;
class	lwRope;
class	lwCameraModeEvent;

extern DWORD lwGetServerElapsedTime32(bool const bUseSavedTime = true);
extern void lwSetServerDateTimeToWnd(lwUIWnd wnd, bool bGameTime, bool bNoTime, bool bNoDate );

class lwWorld
{
public:
	//! 생성자
	lwWorld(PgWorld *pkWorld);

	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! 임시, 마우스 위치의 엑터를 반환한다.
	lwActor PickActor(int iObjectGroupType = 0);

	//! Actor를 추가한다.
	bool AddActor(lwGUID kGUID, lwActor kActor, lwPoint3 kLoc, unsigned int uiGroup);

	//! 객체를 제거한다.
	void RemoveObject(lwGUID kGUID);

	//! 오브젝트를 냉동시킨다.
	void FreezeObject(lwGUID kGuid);

	//! 오브젝트의 냉동을 푼다.
	void UnfreezeObject(lwGUID kGuid);

	//! DropBox를 추가한다.
	bool AddDropBox(lwGUID kGuid, lwDropBox kDropBox, lwPoint3 kLoc);

	//! Actor를 제거한다.(다음 업데이트에 제거됨, DIe 처리용)
	void RemoveActorOnNextUpdate(lwGUID kGuid);

	//! Spawn Point를 찾는다.
	lwPoint3 FindSpawnLoc(char const *pcSpawnName);

	//! Trigger Point를 찾는다.
	lwPoint3 FindTriggerLoc(char const *pcTriggerName);

	//! PilotGuid로 액터를 찾는다.
	lwActor FindActor(lwGUID kPilotGUID);

	lwPoint3	FindActorFloorPos(lwPoint3 kStart);	//	액터 놓을 위치를 계산한다.(떨어지지 않게)

	//! PilotGuid로 WorldObject를 찾는다.
	PgIWorldObject* FindObject(lwGUID kGUID)const;

	void	InitSmallAreaInfo(lwPacket kPacket);

	//! Guid를 반환한다.
	char const *GetID();

	//!	월드 업데이트 스피드 조절
	void	SetUpdateSpeed(float fSpeed);
	float	GetUpdateSpeed();

	float	GetAccumTime();

	//! 계절을 바꾼다.
	bool ChangeSeason(char const *pcSeasonName);

	//! 정예나 보스가 죽었을 때
	void OnBossDie();

	//! 엠포리아 포탈 테스트용 함수
	void RaiseEmporiaPortalUpdate(char const *pcTriggerName, BYTE byGulidLogoIndex, char const* pcGuildName);

	//! 분위기를 바꾼다.(밤/낮)
	bool TransitMood(char const *pcMood);

	lwWorldActionMan	GetWorldActionMan();

	bool SetPlayer(lwActor kActor);

	void SetCamera( char const *pcCameraName );
	bool SetCameraMode(int iMode, lwActor kActor);
	bool SetCameraModeByName( char const *pcSceneRoot, char const *pcCameraName );
	bool SetCameraModeByActorInObjectName(lwActor kActor, char const *pcCameraName, bool bAniReset = false, float fStartTime = 0.0f, bool bUseCameraTargetAsSoundListener = false);
	bool SetCameraModeInPlayer(int iMode);
	bool SetCameraModeMovie(int iMode, int iSeqID, char const *pcFuncName = 0);
	bool SetCameraModeMovie2(lwActor kActor, int iMode, int iSeqID, char const *pcScriptName);
	void SetCameraMovieTranslate(lwPoint3 kTrn);
	void SetCameraAdjustCameraInfo(lwPoint3 const kPosInfo, lwPoint3 const kLookAtInfo);

	lwCameraModeEvent	GetCameraModeEvent();
	
	bool SetMainCamera(char const* pcCamName);
	bool SetMainCameraStatus(char const* pcOtherCameName);

	bool SetCameraFixed( bool const bFixed );
	void SetCameraZoom( float const fZoom );
	float GetCameraZoom();
	void ResetCameraPos();
	void SaveCameraPos();
	void RestoreCameraPos();
	void RefreshCamera();
	void SetCameraZoomMax(float fValue);

	bool SetCameraPose(lwPoint3 kTrn, lwQuaternion kRot);

	void	SetBoxCameraDistance(float fDistance);
	void	SetFreeCamMoveSpeed(float	fSpeed);
	void	SetFreeCamMoveAccelSpeed(float	fSpeed);
	void	SetFreeCamRotateSpeed(float	fSpeed);
	void	SetFreeCamFOV(float	fFOV);

	void	EnableAlphaBlending(char const *strNodeName,bool bEnable);
	void	ResizeBonesPerPartition(char const *strNodeName,int iBonesPerPartition,int iVertsPerPartition);
	void	EnableMipMap(char const *strNodeName,bool bEnable);

	void	TurnOnSpotLight(bool bTurnOn);
	void	SetSpotLightBGColor(int Red,int Blue,int Green);
	
	void	TurnOnLight(bool bTurnOn);

	void	SetBgColor(int const iR, int const iG,int const iB);
	void	SetDrawMode(DWORD const dwDrawMode);
	void	RestoreDrawMode();

	void	HideParticle(bool bHide);
	void	HideNode(const char *strNodeName,bool bHide);
	void	SetNodeSUO(const char *strNodeName,bool bUse);
	void	CountSameTexture(const char *strNodeName);

	bool ToLadder(lwActor kCaster);

	lwPoint3	ThrowRay(lwPoint3 kStart,lwPoint3 kDir,float fDistance, int iShapeType, int iGroup=-1);	//	레이를 쏴서 충돌지점을 구한다.

	bool AddParticle(lwParticle kParticle, lwPoint3 kLoc, unsigned int uiGroup);
	

	int	AttachParticle(const char *strParticleID,lwPoint3 kLoc, float const fAliveTime);
	int	AttachParticleSWithRotate(const char *strParticleID,lwPoint3 kLoc, float fScale, lwPoint3 kRot, float const fAliveTime);
	bool AttachParticleToPointWithRotate(char const* pcParticle, lwPoint3 kPoint, lwQuaternion kQuat, float fScale, float const fAliveTime);
	void DetachParticle(int iSlotID);
	void DetachAllParticle();

	void AttachSound(char const *pcSoundID, lwPoint3 kLoc, float fVolume, float fMin, float fMax);
	void DetachSound(char const* pcSoundID);

	//! Particle의 방출을 제어한다.
	bool ChangeParticleGeneration(char const* pcObjName, bool bGenerate);

	//! Fog 기능을 사용한다.
	void	UseFog(bool bUse, bool bUseAdjust = false);

	void	SetFogAlpha(float fValue);
	void	SetFogColor(int R,int G,int B);

	//! Fog 값을 수정 한다.
	void	SetFogStart(float fValue);
	void	SetFogEnd(float fValue);
	//! Fog Adjust 기능을 사용한다.
	void	UseFogAdjust(bool bUse);
	void	UseLODAdjust(bool bUse);
	//! Camera Adjust 기능을 사용한다.
	void	UseCameraAdjust(bool bUse);
	//! Camera를 원래 대로 리셋하다.
	void ResetCamera();	
	void SetShowWorldInfo(bool bShow);
	void SetShowActorInfo(bool bShow);
	void SetShowPhysXStat(bool bShow);
	void SetBoneLODLevel(int iLODLevel, int iType = 0);
	void SetWorldLODLevel(int iLODLevel);
	void SetWorldLODData(float first = 0.0f, float second = 0.0f, float third = 0.0f);

	//! ShineStone를 리셋한다.
	void	ResetShineStone();

	//! Animation의 속성을 바꾼다.
	void SetAniType_AppInit(char const* pcName);
	void SetAniType_AppTime(char const* pcName);
	void SetAniCycleType_Loop(char const* pcName);
	void SetAniCycleType_Reverse(char const* pcName);
	void SetAniCycleType_Clamp(char const* pcName);

	//! Animation 시간을 바꾼다.
	void SetStartAnimation(float fStartTime = 0.0f);
	void SetStartAnimationByName(char const* pcName, float fStartTime = 0.0f);
	void SetStopAnimation();
	void SetStopAnimationByName(char const* pcName);

	void RestartAnimation(char const* pcName);

	int	GetEventScriptIDOnEnter()const;

	//! Animation 시간을 리턴한다.
	float GetAnimationTime( char const *pcSceneRoot );

	//! 캐릭터 위에 월드 오브젝트 화살표를 띄운다.
	void SetDirectionArrow( NiAVObject* pkDestObj );

	// xxxxxxxxxx 나중에 빼자
	lwTrigger GetTriggerByIndex(int iIndex);
	lwTrigger GetTriggerByID(char const *kTriggerID);

	int	GetDifficultyLevel();
	void	SetDifficultyLevel(int iLevel);


	//! 최적화 레벨을 설정한다.
	void SetOptimizationLevel(int iLevel);

	//! 로우 폴리를 보여줄지 설정.
	void SetActiveLowPolygon(bool bActive, int iGlobalLOD = 0);

	void SetDoFixedStep(bool bUse);
	void SetMaxIter(int iMaxIter);
	void SetPhysXDebug(bool bDebug);

	//! 월드에 붙어있는 특정 오브젝트를 이름으로 찾아 때어낸다.
	bool DetachObjectByName(char const* szObjectName);

	//	나에게 가입 요청을 한 사람이, 그 요청을 취소했다.
	//void	Party_Join_Request_Canceled();

	//	파티 분배 방식을 설정한다.
	//void	SetPartyOption(int Opt);

	//	가입 요청을 받은 사람에게, 가입 요청 받았음을 표시해주고, 수락,거부 응답을 기다린다.
	//void	Receive_Party_Join_Request(lwWString Master_Name,lwGUID PartyGuid,lwGUID Master_MemberGuid);

	//	파티 이름을 설정한다.
	//void	Set_Party_Name(lwWString &PartyName);

	//	파티 멤버를 초기화한다.
	//void	Clear_All_Party_Member();

	//	파티장을 설정한다.
	//void	Set_Party_Master(lwGUID lwkGuid);

	//	멤버를 파티에서 탈퇴시킨다.
	//void	Remove_Party_Member(lwGUID lwkGuid);

	//	멤버의 상태값을 설정한다.
	//void	Set_Party_Member_State(lwWString Member_Name,int iState);

	//	파티 관련 처리중에 유저에게 알려줄 내용을 표시한다.
	//	지정한 시간이 지나면 자동으로 사라지고(0일경우 무한대), 그 전에 확인 버튼을 눌러도 사라진다.
	//void	Show_Party_Message_Box(lwWString NotifyMessage,int iDisplayLimitTime);

	//	가입 요청을 보낸 클라이언트에 띄워주는, 응답 대기 메세지박스를 띄운다.
	//	지정한 시간이 지나면 자동으로 사라지면서, 가입이 취소된다.
	//void	Show_Party_Join_Answer_Wait_Box(bool bShow,char const* TargetUserName,int iDisplayLimitTime);

	// 월드의 속성을 반환한다.
	bool IsHaveWorldAttr( const T_GNDATTR kAttr );

	bool IsNil();

	//! 월드 번호를 저장한다.
	void SetMapNo(int iMapNo);

	//! 월드 번호를 반환.
	int GetMapNo();

	//! 맵의 타입을 반환.
	T_GNDATTR GetAttr();

	//! 맵의 타입을 세팅.
	void SetAttr(INT sAttr);
	int GetDynamicAttr() const;

	lwGUID	GetBossGUID();
	void	SetBossGUID(lwGUID GUID);

	//!	몬스터 타겟 설정
	void	SetMonsterTarget(lwGUID MonGUID,lwGUID TargetGUID);
	lwGUID	GetMonsterTarget_MonGUID();
	lwGUID	GetMonsterTarget_TargetGUID();

	NiActorManagerPtr GetCameraAM();
	void SetCameraAM(NiActorManager* pAM);
	void RemoveCameraAM();

	//! 새 퍼펫을 추가한다.
	lwPuppet AddPuppet(lwGUID kGuid, char const *pcPuppetName, lwPoint3 kTranslate, lwQuaternion kRotate);

	//! 홈 객체를 반환한다.
	lwHome GetHome();
	lwHome CreateHome();
	void ReleaseHome();

	void ClearReservedPilot();

	lwRope	GetRope(char const *strRopeName);

	lwSelectStage SetSelectStage(bool bTrue);

	void	ReloadTexture(const char *strNodeName,int iMipMapSkipLevel);
	void	ChangeShader(const char *strNodeName,char const *strShaderName);

	void	RecursiveDetachGlowMapByGndAttr();

	//	kUnitType 에 해당하는 모든 유닛을 kActionName 의 액션으로 ReserveTransitAction 시킨다.
	void	ReserveTransitActionToActors(char const *kActionName,int kUnitType);

	void ResetWorldFocusFilter();
	void SetShowWorldFocusFilter(bool bShow, char* pTextureFile, float fAlpha);
	void SetShowWorldFocusFilterTexAlpha(char* pTextureFile, float fAlphaStart, float fAlphaEnd, float fTime, bool bKeepFilter, bool bRenderObject=true );
	void SetShowWorldFocusFilterColorAlpha(DWORD color, float fAlphaStart, float fAlphaEnd, float fTime, bool bKeepFilter, bool bRenderObject=true );
	void SetShowWorldFocusFilterTexSwitch(char* pTextureFile1, char* pTextureFile2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval);
	void SetShowWorldFocusFilterColorSwitch(DWORD color1, DWORD color2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval);
	
	//!	현재 포커스 필터의 알파값을 얻어온다.
	float	GetWorldFocusFilterAlpha();
	bool	IsEndShowWorldFocusFilter();

	//! 오브젝트의 포지션을 반환한다.
	lwPoint3 GetObjectPosByName(char const *pcName);
	lwPoint3 GetObjectWorldPosByName(char const *pcName);

	lwPoint3 GetCameraPosByName(char const *pcName);

	void Recv_PT_M_C_NFY_AREA_DATA(lwPacket Packet);
	void Recv_PT_M_C_ADD_UNIT(lwPacket Packet);
	void Recv_PT_M_C_NFY_MAPLOADED(lwPacket Packet);
	void Recv_AddReadyCharacter(lwPacket Packet);
//	bool Recv_PT_M_C_NFY_PLAY_OPENING(lwPacket Packet);

	void RemoveAllObject(bool bExceptNPC = false);
	void RemoveAllMonster();
	void RemoveAllGradeMonster(int const iGrade);
	void RemoveAllObjectExceptMe();

	bool CheckPlayerInRange(lwActor kActor, float fRange);

	bool RemoveCameraWall(char const* pkName);
	bool RecoverCameraWall(char const* pkName);

	void SetActivePhysXGroup(const PgIXmlObject::XmlObjectID eType, int iGroup, bool bEnable);

	void AddDrawActorFilter(lwGUID kActorGuid);
	void ClearDrawActorFilter();

	void SetActivaingWorldMap(bool bActivating);
	void SetUpdateWorld(bool bUpdate);
	void SetSlowMotion(int const iType, float const fFrom, float const fTo, float const fTime);
	void AddShineStoneCount(unsigned long const dwStoneID);

	void SetCameraAdjustInfo(float fUp, float fZoom, float fTargetZ, float fMinZoom, float fMaxZoom);
	void RecoverCameraAdjustInfo();

	float	GetServerElapsedTime();
	
	int GetCameraMove_ID();

	lwGUID GetOldMouseOverObject();

	bool IsMapMoveCompleteFade() const;
	void MapMoveCompleteFade();
	void SaveHideActorClassNo(int const iClassNo, bool const bHide);
	void SaveHideActorType(int const iType, bool const bHide);
	void ClearSaveHideActor();

//	ShineStone Count

//	void AddPetToWorld(lwGUID kGUID, lwGUID kMasterGUID, lwPoint3 kPosition, int iID);

	bool CheckEventScriptOnEnter();
	bool IsExistMiniMapData() const;

	lwWString GetMissionTrigIDOnThisMap(void);

	void InitDmgNum(); 

public:
	void SetCurProgress(int iProgress);
	int GetCurProgress(void) const;

	lwPoint2 GetMapThumbnailImgSize(void);
	lwWString GetMapThumbnailImgName(void);

protected:
	DWORD m_dwMapState;
	PgWorld *m_pkWorld;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWWORLD_H