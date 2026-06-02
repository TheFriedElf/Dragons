#include "stdafx.h"
#include <mmsystem.h>
#include "Lohengrin/PacketStruct2.h"
#include "Variant/Constant.H"
#include "Variant/PgStringUtil.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgMonster.H"
#include "Variant/PgEventView.h"
#include "Variant/PgMyQuest.h"
#include "Variant/PgControlDefMgr.h"
#include "Variant/PgQuestInfo.h"
#include "PgOption.h"
#include "PgXmlLoader.h"
#include "PgActionSlot.h"
#include "PgLocalManager.h"
#include "PgRenderer.h"
#include "PgAction.h"
#include "PgActionPool.h"
#include "PgParticleMan.h"
#include "PgTrigger.h"
#include "ServerLib.h"
#include "PgParticle.h"
#include "PgAMPool.h"
#include "PgWorld.h"
#include "PgPilot.h"
#include "lwActor.h"
#include "lwAction.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "PgUISound.h"
#include "PgItemEx.h"
#include "PgCameraModeFollow.h"
#include "PgTrail.h"
#include "PgScripting.h"
#include "PgQuestMan.h"
#include "PgNifMan.h"
#include "Pg2DString.h"
#include "PgSkillTargetMan.h"
#include "PgWorkerThread.h"
#include "lwUI.H"
#include "lwWString.H"
#include "ShadowGeometry.h"
#include "PgActionEffect.H"
#include "PgMobileSuit.h"
#include "FreedomPool.h"
#include "Utility/ExceptionHandler.h"
#include "Utility/ErrorReportFile.h"
#include "PgStatusEffect.H"
#include "PgEnergyGuage.H"
#include "PgMath.h"
#include "PgGuild.h"
#include "PgDropBox.H"
#include "PgSoundMan.h"
#include "PgHeadBuffIcon.H"
#include "PgCircleShadow.H"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "ShadowObject.H"
#include "PgPartyBalloon.h"
#include "PgExpeditionBalloon.h"
#include "PgTextBalloon.h"
#include "PgActor.h"
#include "PgSpotLight.H"
#include "PgHelpSystem.H"
#include "PgFriendMgr.H"
#include "PgCoupleMgr.h"
#include "PgObject.H"
#include "PgEventScriptSystem.H"
#include "PgChatMgrClient.h"
#include "PgActorMonster.H"
#include "BM/FileSupport.h"
#include "PgActorAppearanceMan.H"
#include "PgContentsBase.h"
#include "PgActorUtil.h"
#include "PgNetwork.h"
#include "PgSafeFoamMgr.h"
#include "PgFontSwitchTable.h"
#include "lwUIPet.h"
#include "PgParticleProcessorAttachToNode.H"
#include "Variant/PgBattleSquare.h"
#include "PgBattleSquare.h"
#include "PgChainAttack.h"
#include "PgPhysXUtil.H"
#include "lwJobSkillItem.h"
#include "PgCustomUI_Summmoner.h"
#include "lwSummon_Info.h"
#include "lwCommonSkillUtilFunc.h"
#include "PgRaceEvent.h"

#include "NewWare/Scene/ApplyTraversal.h"
#include "NewWare/Renderer/DrawWorkflow.h"

#ifndef EXTERNAL_RELEASE
//#define PG_USE_ACTOR_TRACE
#endif

#define PG_ITEM_SEQUENCIAL_LOAD

#ifdef PG_SYNC_ENTIRE_TIME
	DWORD PgActor::ms_dwSyncTime = 0;
	DWORD PgActor::ms_dwLocalSyncTime = 0;
	DWORD PgActor::ms_dwLastSentTime = 0;
	DWORD PgActor::ms_dwAverageLatency = 0;
#endif

extern const	char	*ACTIONNAME_RP_IDLE;
extern const	char	*ACTIONNAME_RP_WALK;
extern const	char	*ACTIONNAME_RP_JUMP;
extern const	char	*ACTIONNAME_TRACE;
extern const	char	*ACTIONNAME_TRACE2;
extern const	char	*ACTIONNAME_RUN_PET;
extern const	char	*ACTIONNAME_RIDING;

float PgActor::ms_fGravity = -800.0f;				// 캐릭터 점프시에만 적용되는 중력

bool PgActor::m_stbNameVisible = true;
bool PgActor::m_bDrawNamePC = true;
bool PgActor::m_bDrawNameNPC = true;

bool PgActor::m_bAutoGetItemMoney = true;
bool PgActor::m_bAutoGetItemEquip = true;
bool PgActor::m_bAutoGetItemConsume = true;
bool PgActor::m_bAutoGetItemETC = true;

NiColor PgActor::ms_kDefaultAmbient = NiColor(0.3137f, 0.3137f, 0.3137f);
NiColor PgActor::ms_kDefaultEmissive = NiColor(0.5098f, 0.5098f, 0.5098f);
NiColor PgActor::ms_kDefaultSpecular = NiColor(0, 0, 0);
NiColor PgActor::ms_kDefaultDiffuse = NiColor(0, 0, 0);
extern void NET_C_M_REQ_TRIGGER(int const iObjectType, lwGUID kGuid, int const iActionType);
extern void Send_PT_C_M_REQ_ITEM_DIVIDE(const SItemPos &kItemPos, int const iItemNo, BM::GUID const &kItemGuid, int const iCount);

float const PgActor::ms_kUpdateIntervalByInvisibleGrade[PgActor::MAX_NUM_INVISIBLE] = 
{
	1.0f / 60.0f, 1.0f / 20.0f, 1.0f / 6.0f, 1.0f / 3.0f, 1.0f / 2.0f
};

enum eATTACHTO_PARTICLE_SLOT
{
	EAPS_NONE					= 0,
	EAPS_STRATEGIC_POINT		= 191918,
	EAPS_CUSTOMUI_SUMMONED		= 200001,
	EAPS_GET_START				= 900000,
	EAPS_GET_END				= 999999,
};

ControllerManager g_kControllerManager;

NiImplementRTTI(PgActor, PgIWorldObject);

extern const	char	*ACTIONNAME_RUN_PET;

int GetTotalSummonedCount(CUnit const* pkCaller);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgControllerHitCallBack
/////////////////////////////////////////////////////////////////////////////////////////////////////////
NxControllerAction  PgControllerHitCallBack::onShapeHit(const NxControllerShapeHit& hit)
{
	if(m_pkParentActor 
		&& NX_NEGATIVE_UNIT_Z == hit.dir
		)
	{
		m_pkParentActor->SetControllerShapeHit(hit);
	}
	return	NX_ACTION_PUSH;
}
NxControllerAction  PgControllerHitCallBack::onControllerHit(const NxControllersHit& hit)
{
	return	NX_ACTION_PUSH;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgSweepHitCallBack
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgSweepHitCallBack::onEvent(NxU32 nbEntities, NxSweepQueryHit* entities)
{

	_PgOutputDebugString("PgSweepHitCallBack::onEvent t:%f n:(%f,%f,%f)\n",entities->t,entities->normal.x,entities->normal.y,entities->normal.z);
	return	true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActor
/////////////////////////////////////////////////////////////////////////////////////////////////////////

PgActor::PgActor()
:m_bLockBidirection(true),
m_bWalkingToTarget(false),
m_bWalkingToTargetForce(false),
m_pkAction(NULL),
m_kSeqID(-1),
m_kTargetDir(NiPoint3::ZERO),
m_kTargetLoc(NiPoint3::ZERO),
m_kMovingDir(NiPoint3::ZERO),
m_kLastFloorPos(NiPoint3::ZERO),
m_kPrevWorldPos(NiPoint3::ZERO),
m_kSourcePos(NiPoint3::ZERO),
m_pkCurrentTrigger(0),
m_bCurrentTriggerAct(false),
m_pkActorCallback ( 0),
m_kLookingDir ( NiPoint3::ZERO),
m_byMovingDirection ( DIR_NONE),
m_bPositionChanged ( false),
m_fLastItemPickUpTime ( 0.0f),
m_fLastAttackTargetedTime ( 0.0f),
m_pTextBalloon (NULL),
m_pMarkBalloon(NULL),
m_pPartyBalloon (NULL),
m_pExpeditionBalloon(NULL),
m_pVendorBalloon (NULL),
m_pHPGaugeBar(NULL),
m_pkHeadBuffIconList ( NULL),
m_fLastDownwardPassUpdateTime ( 0),
m_pkShadow ( NULL),
m_bSpecularOn(false),
// PhysX 관련
m_pkPhysXActor ( 0),
m_pkPhysXSrc ( 0),
m_pkPhysXDest ( 0),
m_pkController ( 0),
m_pkMonsterDef ( 0),
m_bIsUnderMyControl ( false),
m_pkSyncMoveNextAction ( NULL),
m_pkMountedRidingObject ( NULL),
m_pkMountedRidingPet ( NULL ),
m_fScaleOrig(1.0f),
m_bShadowHide ( false),
m_bMiniMapHide ( false),
m_spSpotLightGeom ( NULL),
m_bIsOptimizeSleep ( true),
// 물리 관련..
m_bJump ( false),
m_bAdjustValidPos ( true),
m_bForceSync ( true),
m_fJumpTime ( 0.0f),
m_fJumpAccumHeight ( 0.0f),
m_fInitialVelocity ( 0.0f),
m_bFreeMove ( false),
m_bFalling ( false),
m_bFloor ( false),
m_bCheckMeetFloor( true),
m_bSlide ( false),
//m_fLocalUpDownSpeed ( 0.0f),
//m_bUseLocalUpDownSpeed ( false),
m_dwLastUpdateFrame (0),
m_fLastUpdateFrameTime ( 0.0f),
m_iUpdateCount(0),
m_bHide ( false),
m_bHideNameTitle(false),
m_bCanHit ( true),
m_bBlink ( false),
m_bBlinkHide ( false),
m_bTransformed ( false),

m_fRotationInterpolTime ( 1.0f),
m_fSyncInterpolTime ( 0.0f),
m_fMovingSpeedScale ( 1.0f),
m_bSide ( false),
m_bCheckCamColl ( false),
m_bCheckCliff ( false),
m_bTransformation ( false),
m_bDead ( false),
m_bBackMoving ( false),
m_bNoConcil ( false),
m_bNoFindPathNormal ( false),
m_bBlowUp ( false),
m_fLastWaveEffectUpdateTime(0),
m_bLeaveExpedition ( false),
m_bDieReservedByAction ( false),
m_ulNormalAttackEndTime ( 0),
m_uiMyWeaponType ( PgItemEx::IT_FST),
m_kMyWeaponEquipLimit ( EQUIP_LIMIT_WEAPON),
m_byMyWeaponAnimFolderNum ( 1),
m_byWeaponAnimFolderNumAtActionStart(0),
m_fTargetScale ( 0),
m_fStartScale ( 0),
m_ulScaleChangeStartTime ( 0),
m_ulTotalScaleChangeTime ( 0),
m_pAlphaNode(NULL),
m_fStartAlpha(0),
m_fEndAlpha(0),
m_ulAlphaStartTime(0),
m_ulAlphaChangeTime(0),
m_bNowDrawWeaponTrail ( false),
m_bNowDrawBodyTrail ( false),
m_bRiding ( false),
m_bNeedToUpdateUIModel ( true),
m_kTargetWalkingNextAction ( ""),
m_spNameText(0),
m_spTitleName(0),
m_spGuildNameText(0),
m_spGuildMark(0),
m_spGuardianMark(0),
m_spMyhomeMark(0),
m_spAchievementTitle(0),
m_spGIFTitle(0),
m_spCustomCountText(0),
m_spDuelTitle(0),
m_spEffectCountDown(0),
m_sComboCount(0),
m_ulNormalAttackFreezeStartTime ( 0),
m_bMaterialColorCached ( false),
m_kAlwaysGlowMap(false),
m_iGodTimeStatusEffectInstanceID(-1),
m_iDamageBlinkStatusEffectInstanceID(-1),
m_bSendBlowStatus ( false),

m_fTotalAnimSpeedControlTime ( 0),
m_fAnimSpeedControlStartTime ( 0),
m_fAnimSpeedControlValue ( 0.0f),
m_fOriginalAnimSpeed ( 1.0f),
m_fAccumTimeAdjust ( 0.0f),
m_fBeforeAccumTime ( 0.0f),

m_iTotalShakeTime ( 0),
m_iShakeStartTime ( 0),
m_fShakeValue ( 0),

//m_fLastUpdateTime ( 0),
//m_fStridenUpdateTime ( 0.0f),
//m_fLastDownwardPassUpdateTime ( 0),
m_bVisible ( true),
m_kNormalizedActorPosByCamera ( NiPoint3::ZERO),
m_eInvisibleGrade ( PgActor::VISIBLE),
m_bLoadingComplete(false),
m_kOriginalPos(0,0,0),
//이쁘게 꾸미기 관련
m_bInputNow	( false),
m_bNoName ( false),
m_bTraceUpdate ( false),
m_iEquipCount ( 0),
m_uiActiveGrp ( DEFAULT_ACTIVE_GRP),

m_bUseSkipUpdateWhenNotVisible ( false),

m_fCurrentHeadSize ( 1.0f),
m_fTargetHeadSize ( 1.0f),
m_fDefaultHeadSize ( 1.0f),
m_fHeadSizeTransitSpeed ( 1.0f),
m_fWalkToTargetLocSkillRange ( 0.0f),

m_bInvisible ( false),
m_iFreezeStatus ( EFS_NONE),
m_bNotActionShift(false),

m_dwLastFrameTime ( 0),
m_dwLastActionTime ( 0),
m_dwAccumedOverTime ( 0),
m_fLoadingStartTime ( 0.0f),

m_bSyncVelocity ( false),
m_bSyncCrashed ( false),
m_bSync ( false),
m_bOnlyMoveAction ( false),
m_bOnlyDefaultAttack ( false),
m_bDownState ( false),
m_fTotalDownTime(0),

m_spWeaponTrailNode ( NULL),
m_spBodyTrailNode ( NULL),

m_byLastDirection ( 0),
m_fSpecifiedFrameTime ( 0.0f),
m_bDoSimulateOnServer ( true),
m_dwLastSimulatedTime ( 0),

m_fAutoDeleteActorTime ( 0.0f),
m_fAutoDeleteActorStartTime ( 0.0f),
m_iEventScriptIDOnDie(-1),
m_fDieParticleScale(1.0f),
m_dwActionLatency(0),
m_kSyncStartDir(DIR_NONE),
m_fElapsedSyncTime(0),
m_fVelocityRate(0),
m_bStun(false),
m_pkActorAppearanceMan(NULL),
m_uiMyWeaponNo(0),
m_kControllerHitCallBack(this),
m_vPrevControllerPos(0,0,0),
m_fAnimationStartTime(0.0f),
m_fBlowUpStartTime(0),
m_fLastCheckItemUseTime(0),
m_bIgnoreSlide(true),
//트리거 관련
m_iOtherEquipItemReturnValue(0),

m_bCanRide(false),
m_bShowWarning(true),
m_iOldStrategicPoint(0),

m_kIdleEffectNode("char_root"),
m_iAttachSlotNo(EAPS_GET_START)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.PgActor"), g_pkApp->GetFrameCount()));

	m_kPathNormal = NiPoint3::UNIT_Y;

	m_kDamageBlinkInfo.Reset();
	InitControllerShapeHit();
	m_kUIModelOpt.Clear();

	for (unsigned int i = 0; i < PG_MAX_NB_ABV_SHAPES; ++i)
	{
		m_apkPhysXCollisionActors[i] = 0;
		m_apkPhysXCollisionSrcs[i] = 0;
	}

	m_pkPick = NiNew NiPick;
	m_pkPick->SetPickType(NiPick::FIND_FIRST);
	m_pkPick->SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	m_pkPick->SetCoordinateType(NiPick::WORLD_COORDINATES);
	m_pkPick->SetReturnNormal(true);


	m_AlphaTransitInfo.m_fAlphaTransitionTime = 1.0f;
	m_AlphaTransitInfo.m_fAlphaAccumTime = 0.0f;
	m_AlphaTransitInfo.m_fTargetAlpha = 1.0f;
	m_AlphaTransitInfo.m_fPrevAlpha = 0.0f;
	m_AlphaTransitInfo.m_fCurrentAlpha = 1.0f;
	m_AlphaTransitInfo.bUseLoadingFinishInit = true;

	m_ColorTransitInfo.m_fTransitionTime = 1.0f;
	m_ColorTransitInfo.m_fAccumTime = 0.0f;
	m_ColorTransitInfo.m_kTarget = NiColor::WHITE;
	m_ColorTransitInfo.m_kPrev = NiColor::WHITE;
	m_ColorTransitInfo.m_kCurrent = NiColor::WHITE;

	m_SpecularTransitInfo.m_fTransitionTime = 1.0f;
	m_SpecularTransitInfo.m_fAccumTime = 0.0f;
	m_SpecularTransitInfo.m_kTarget = NiColor::BLACK;
	m_SpecularTransitInfo.m_kPrev = NiColor::BLACK;
	m_SpecularTransitInfo.m_kCurrent = NiColor::BLACK;

//	m_iTwistTimes = 0;


	m_kMovingAbsolute.zero();
	m_kPrevMovingDelta.zero();
	SetMovingDelta(NX_ZERO);
	m_kSlideVector.zero();

	m_kToRotation = NiQuaternion(NI_HALF_PI, NiPoint3::UNIT_X);


	m_OriginalActorGUID.Clear();
//	m_kMasterGuid.Clear();
	m_kReservedAction.clear();

	//m_pkQuestSimpleInfoPool = NiNew PgQuestSimpleInfoPool();

	NeedToUpdateMaterialProp(true);

	for (int i = 0; i < MAX_NUM_INVISIBLE; ++i)
	{
		m_kLastUpdateTimeByInvisibleGrade[i] = 0.0f;;
		m_kCanUpdate[i] = false;
	}	
	
	//InfoUI(0);

	m_pkActionEffectStack = new PgActionEffectStack(this);

	m_kLastFramePos = NxExtendedVec3(0,0,0);

	m_kSyncInterpolDelta.zero();


	// Alpha Property
	m_spAlphaProperty = NiNew NiAlphaProperty;
	m_spAlphaProperty->SetAlphaBlending(false);
	m_spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
	m_spAlphaProperty->SetAlphaTesting(true);
	m_spAlphaProperty->SetTestRef(10);
	m_spAlphaProperty->SetAlphaGroup(AG_EFFECT);
	m_spAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);

#ifndef EXTERNAL_RELEASE
	m_dwLastTransitTime = 0;
	m_dwLastTransitFrame = 0;
#endif

	EquipDefaultItem(false);

	m_kGodTimeInfo.Reset();

	m_kDieParticleNode.clear();

	UseSmoothShow(1.0f);	

	m_kGenerateSetEffectSlotIndex = 500000;

	SetUseBattleIdle(false);
		
	{// TrailContiner
		m_kContTrail.resize(ETAT_MAX);
		CONT_TRAIL::iterator kItor = m_kContTrail.begin();
		while( kItor != m_kContTrail.end() )
		{
			(*kItor) = NULL;
			++kItor;
		}
	}
	_PgOutputDebugString("[PgActor::PgActor] Actor:%#X created\n", this);
	
}

PgActor::~PgActor()
{
	// 여기에서 ActorManager가 AMPool로 리턴이 되므로 그 전에 ActorManager에 했던 내용들을 다 없애자.
	Terminate();
	PgIWorldObject::Terminate();
}
bool PgActor::CreateCopyEx(PgActor *pkNewActor)
{
	pkNewActor->SetName(GetName());

	pkNewActor->m_kDieParticleID = m_kDieParticleID;
	pkNewActor->m_kDieParticleNode = m_kDieParticleNode;
	pkNewActor->m_kDieSoundID = m_kDieSoundID;
	pkNewActor->m_fDieParticleScale = m_fDieParticleScale;
	pkNewActor->m_kLoadingCompleteInitFunc = m_kLoadingCompleteInitFunc;
	pkNewActor->SetEventScriptIDOnDie(GetEventScriptIDOnDie());
	pkNewActor->SetUseBattleIdle(IsUseBattleIdle());

	pkNewActor->m_kReservedAction.insert(std::make_pair(RA_IDLE, ACTIONNAME_IDLE));
	pkNewActor->m_kReservedAction.insert(std::make_pair(RA_OPENING, "a_opening"));
	pkNewActor->m_kReservedAction.insert(std::make_pair(RA_INTRO_IDLE, "a_intro_idle"));

	if(m_pkPilot)
	{
		pkNewActor->m_pkPilot = m_pkPilot->CreateCopy();
		if(pkNewActor->m_pkPilot)
		{
			pkNewActor->m_pkPilot->SetWorldObject(pkNewActor);
		}
	}

	pkNewActor->m_kReservedAction[RA_OPENING] = m_kReservedAction[RA_OPENING];
	pkNewActor->m_kReservedAction[RA_IDLE] = m_kReservedAction[RA_IDLE];

	PgIWorldObjectBase	*pkNewBase = GetWorldObjectBase()->CreateCopy(pkNewActor);
	std::wstring kEventScript = pkNewBase->GetEventScript();
	if(kEventScript.length() != 0)
	{
		pkNewActor->m_pkActorCallback = NiNew ActorCallbackObject;
		if(!pkNewActor->m_pkActorCallback)
		{
			PG_ASSERT_LOG(!"failed to creat ActorCallbackObject");
			return false;
		}

		pkNewActor->m_pkActorCallback->m_pkWorldObject = pkNewActor;
		pkNewActor->m_pkActorCallback->m_kScriptName = MB(kEventScript);

		if(!pkNewActor->GetActorManager())
		{
			PG_ASSERT_LOG(!"ActorCallbackObject : ActorManager must be initialized prior to Callback Object!");
			return false;
		}
		pkNewActor->GetActorManager()->SetCallbackObject(pkNewActor->m_pkActorCallback);

		pkNewActor->SetObjectID(GetObjectID());
	}

	pkNewActor->m_bNoName = m_bNoName;
	pkNewActor->m_VarTextureList = m_VarTextureList;

	if(m_pkActorAppearanceMan)
	{
		pkNewActor->m_pkActorAppearanceMan = m_pkActorAppearanceMan->CreateCopy(pkNewActor);
	}

	NiActorManager *pkAM = pkNewActor->GetActorManager();
	if(!pkAM)
	{
		return false;
	}

	pkAM->Update(0.0f);
	NiTimeController::StartAnimations(pkNewActor->GetNIFRoot(), 0.0f); //PgWorld에 Attach될 때 알아서 된다.
	AMContainer::iterator itr = pkNewActor->m_kSupplementAMContainer.begin();
	while(itr != pkNewActor->m_kSupplementAMContainer.end())
	{
		PG_ASSERT_LOG(itr->m_spAM);
		if (itr->m_spAM)
		{
			itr->m_spAM->Update(0.0f);
		}
		++itr;
	}

	// Actor를 붙일 때는, 무조건 숨긴 다음 로딩을 다 하면 Alpha로 빼준다.
	pkAM->GetNIFRoot()->SetAppCulled(true);
	pkNewActor->NiNode::SetAppCulled(true);

	// 바리에이션 텍스쳐를 로딩한다.
	if (pkNewActor->m_VarTextureList.size() > 0)
	{
		pkNewActor->ChangeTexture(pkNewActor);
	}
	pkNewActor->m_bIgnoreSlide = m_bIgnoreSlide;

	pkNewActor->m_kFixedParticleList = m_kFixedParticleList;
	return	true;
}
PgIWorldObject* PgActor::CreateCopy()
{
	PgActor	*pkNewActor = NiNew PgActor();
	PgActor::CreateCopyEx(pkNewActor);
	return	pkNewActor;
}

CUnit* PgActor::GetUnit() const
{
	if(GetPilot())
	{
		return GetPilot()->GetUnit();
	}
	
	return NULL;
}

void PgActor::SetSendBlowStatus(bool bSend, bool bFirstDown, bool bNoUseStandUpTime)
{	
	if(m_bSendBlowStatus && bSend == false)
	{
		BM::Stream kPacket(PT_C_M_REQ_MON_BLOWSTATUS);
		kPacket.Push(GetPilotGuid());
		kPacket.Push(GetPos().x);
		kPacket.Push(GetPos().y);
		kPacket.Push(GetPos().z);
		kPacket.Push(bFirstDown);
		kPacket.Push(bNoUseStandUpTime);
		NETWORK_SEND(kPacket);
		
		//lua_tinker::call<void, lwGUID, lwPoint3, bool>("Net_C_M_REQ_MON_BLOWSTATUS", lwGUID(GetPilotGuid()),lwPoint3(GetPos()),bFirstDown);

		if(!bFirstDown)
		{
			m_bSendBlowStatus = false;
			return;
		}
	}

	if(bSend)
	{
		m_bSendBlowStatus = bSend;
	}
	else
	{
		BM::GUID kPlayerPilotGuid;
		if(g_kPilotMan.GetPlayerPilotGuid(kPlayerPilotGuid))
		{
			m_bSendBlowStatus = (kPlayerPilotGuid == GetPilot()->GetUnit()->GetBlowAttacker()) || IsMyActor() || g_kPilotMan.IsMySummoned(kPlayerPilotGuid);
			if(!m_bSendBlowStatus)
			{
				PgPilot* pkPilot = g_kPilotMan.FindPilot(GetPilot()->GetUnit()->GetBlowAttacker());
				if(pkPilot)
				{
					switch(pkPilot->GetUnit()->UnitType())
					{
					case UT_ENTITY:
						{
							PgEntity *pkEntity = dynamic_cast<PgEntity*>(pkPilot->GetUnit());
							if(pkEntity)
							{
								m_bSendBlowStatus = (pkEntity->Caller() == kPlayerPilotGuid);
							}
						}break;
					case UT_SUB_PLAYER:
						{
							PgSubPlayer* pkSubPlayer = dynamic_cast<PgSubPlayer*>(pkPilot->GetUnit());
							if(pkSubPlayer)
							{
								m_bSendBlowStatus = (pkSubPlayer->Caller() == kPlayerPilotGuid);
							}
						}break;
					case UT_SUMMONED:
						{
							m_bSendBlowStatus = g_kPilotMan.IsMySummoned(pkPilot->GetUnit());
						}break;
					}
				}
			}
		}
	}
}
void	PgActor::DetachActorAlphaProperty(NiAVObject *pkAVObject)
{
	if(!m_bLoadingComplete)
	{
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iArray = pkNode->GetArrayCount();
		for(int i = 0;i < iArray; ++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				DetachActorAlphaProperty(pkChild);
			}	
		}
		return;
	}

	NiAlphaProperty	*pkAlpha = (NiAlphaProperty*)pkAVObject->GetProperty(NiAlphaProperty::GetType());
	if(pkAlpha && pkAlpha == m_spAlphaProperty)
	{
		pkAVObject->DetachProperty(m_spAlphaProperty);
	}
}
void	PgActor::AttachActorAlphaProperty(NiAVObject *pkAVObject)
{
	if(!m_bLoadingComplete)
	{
		return;
	}

	if(!pkAVObject)
	{
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iArray = pkNode->GetArrayCount();
		for(int i=0;i<iArray; ++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				AttachActorAlphaProperty(pkChild);
			}	
		}
		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(!pkGeom)
	{
		return;
	}

	NiAlphaProperty	*pkAlpha = (NiAlphaProperty*)pkGeom->GetProperty(NiAlphaProperty::GetType());
	if(pkAlpha==NULL)
	{
		pkAVObject->AttachProperty(m_spAlphaProperty);
	}
	else
	{
		pkAlpha->SetAlphaGroup(AG_EFFECT);
		//pkAlpha->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		//pkAlpha->SetTestRef(10);
		//pkAlpha->SetAlphaTesting(true);
	}
}
void PgActor::Terminate()
{
	_PgOutputDebugString("[PgActor::Terminate] Actor:(%s)(%s)(%#X) terminating\n",MB(GetPilotGuid().str()), MB(GetGuid().str()), this);
	RestoreTexture();

	SAFE_DELETE_NI(m_pkShadow);
	SAFE_DELETE(m_pkActorAppearanceMan);

	m_spSpotLightGeom = 0;

	m_pkMountedRidingObject = NULL;
	m_pkMountedRidingPet = NULL;


	//	임시로 붙인 알파프로퍼티를 떼어낸다.
	DetachActorAlphaProperty(this);
	UpdateProperties();

	SetColor(NiColor::WHITE);	//	컬러 복귀

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.Terminate"), g_pkApp->GetFrameCount()));

	PG_ASSERT_LOG(m_pHPGaugeBar == NULL);
	
	m_ActionToggleState.clear();
	m_StatusEffectInstanceListForUpdate.clear();
	m_StatusEffectInstanceList.clear();

	// Before CleanUp으로 옮김.
	//SAFE_DELETE(m_pkActionEffectStack)
	
	DetachAllParts();

	SAFE_DELETE_NI(m_pkPick);
	SAFE_DELETE_NI(m_pkActorCallback);
	//SAFE_DELETE_NI(m_pkQuestSimpleInfoPool);

	// AM Container 정리
	for(AMContainer::iterator itr = m_kSupplementAMContainer.begin(); itr != m_kSupplementAMContainer.end(); ++itr)
	{
		PG_ASSERT_LOG(itr->m_spAM);
		if (itr->m_spAM)
			itr->m_spAM = 0;
	}
	m_kSupplementAMContainer.clear();

	// 대기큐에 등록된 장착할 아이템 제거
	m_kAttachItemLock.Lock();
	for(AttachItemContainer::iterator itr = m_kAttachItemContainer.begin(); itr != m_kAttachItemContainer.end(); ++itr)
	{
		THREAD_DELETE_ITEM(itr->pItem);
	}
	m_kAttachItemContainer.clear();

	RestoreItemModel(EQUIP_LIMIT_WEAPON);
	for(PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin(); itr != m_kPartsAttachInfo.end(); ++itr)
	{
		PgItemEx *pkEquipItem = itr->second;
		THREAD_DELETE_ITEM(pkEquipItem);
	}

	m_kPartsAttachInfo.clear();
	m_kAttachItemLock.Unlock();

	if(GetNIFRoot())
	{
		NiAVObject	*pkHead = GetCharRoot()->GetObjectByName("Bip01 Head");
		if(pkHead)
		{
			pkHead->SetScale(1);
		}
	}

	m_fCurrentHeadSize = m_fTargetHeadSize = 1;
	SetDefaultHeadSize(1.0f);

	m_bDead = false;
	m_kPOTParticle.ClearAll();
	//InfoUI(0);

	ClearTempAction();
}
BM::GUID const &PgActor::GetPilotGuid()
{
	if(GetPilot())
		return	GetPilot()->GetGuid();

	PG_ASSERT_LOG("PgActor::GetPilotGuid() Failed. This actor has not pilot.\n");
	return	BM::GUID::NullData();
}
void	PgActor::OnAbilChanged(int iAbilType,int iValue)
{
	if(iAbilType == AT_HP)
	{
		if(m_pkActorAppearanceMan)
		{
			m_pkActorAppearanceMan->UpdateAppearance();
		}
	}
}
void PgActor::RefreshHPGaugeBar(int iBeforeHP,int iNewHP, PgActor *pkAttacker, bool const bSetAbil)
{
	if(true==bSetAbil)
	{
		GetPilot()->SetAbil( AT_HP, iNewHP );
	}
	_PgOutputDebugString("[PgActor::RefreshHPGaugeBar] Actor:%s Current HP : %d\n",MB(GetPilotGuid().str()), iNewHP );

	//BigArea일때 AT_HP 적용 패킷이 먼저 들어와 HP바가 보이지 않는 경우가 발생
	//그래서 아래와 같은 조건을 추가했음, 헛치는 경우는 어떻게 표현되지?
	//헛치는 경우 RefreshHPGaugeBar를 호출하기 전에 체크하여 함수에 진입하지 않도록 함
	//좀 더 좋은 방법이 있다면 이 방법은 수정했으면 좋겠음
	bool const bEtcVisible = (m_pHPGaugeBar && false==m_pHPGaugeBar->IsVisibleTime() && iBeforeHP==iNewHP && pkAttacker);
	if ( SetHPBarValue( iBeforeHP, iNewHP ) || bEtcVisible )
	{
		bool	bShowHPGauge = false;

		if ( PgContentsBase::ms_pkContents )
		{
			CUnit *pkUnit = GetPilot()->GetUnit();
			if ( pkUnit )
			{
				PgContentsBase::ms_pkContents->RecvHP( pkUnit, iBeforeHP, iNewHP );
			}
		}

		if( m_pHPGaugeBar )
		{
			switch ( m_pHPGaugeBar->GetType() )
			{
			case EGAUGE_CORE:
				{
					bShowHPGauge = true;
				}break;
			default:
				{
					if(pkAttacker)
					{// 공격자가
						CUnit* pkAttackerUnit = pkAttacker->GetUnit();
						if( pkAttackerUnit )
						{
							if ( pkAttacker->IsUnderMyControl() 
								|| g_kPilotMan.IsMySummoned( pkAttackerUnit )
								)
							{// 내 소유이거나(소환체는 따로 체크해야함)
								bShowHPGauge = true;
							}
						}
					}
					else
					{
						CUnit* pkUnit =  GetUnit();
						if( pkUnit && 
							pkUnit->IsUnitType(UT_SUMMONED)
						)
						{
							bShowHPGauge = true;
						}
					}
				}break;
			}

			if( bShowHPGauge && !IsUnderMyControl() )
			{
				m_pHPGaugeBar->ResetVisibleStartTime();
			}
		}
	}
}

bool PgActor::SetHPBarValue(int const iBefore,int const iNew )
{
	if ( !m_pHPGaugeBar )
	{
		return false;
	}

	switch ( m_pHPGaugeBar->GetType() )
	{
	case EGAUGE_PET:
		{
			//Pet은 이곳에서 변경되지 않도록 막는다
			//Pet은 m_pHPGaugeBar가 표시하는 내용이 HP가 아닌 MP 이기 때문에 MP가 변경되는 곳에서 직접 변경해준다.
			return false;
		}break;
	default:
		{
			int const iMaxHP = GetPilot()->GetAbil(AT_C_MAX_HP);

			if(iBefore == iNew && m_pHPGaugeBar->MaxValue() == iMaxHP)
			{
				return false;
			}

			_PgOutputDebugString("[PgActor::RefreshHPGaugeBar] Actor:%s iMaxHP:%d Before HP : %d New HP: %d\n",MB(GetPilotGuid().str()),iMaxHP,iBefore,iNew);
			m_pHPGaugeBar->SetBarValue ( iMaxHP, iBefore,iNew );
		}break;
	}

	
	return true;
}

PgEnergyGauge* PgActor::GetHPGaugeBar()const
{
	return m_pHPGaugeBar;
}

float PgActor::GetDistanceFromPath(NiPoint3 const &kPos)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetDistanceFromPath"), g_pkApp->GetFrameCount()));
	static NiPoint3 akDirs[] =
	{
		NiPoint3(1.0f, 0.0f, 0.0f),
		NiPoint3(-1.0f, 0.0f, 0.0f),
		NiPoint3(0.0f, 1.0f, 0.0f),
		NiPoint3(0.0f, -1.0f, 0.0f),
	};
	
	m_pkPick->SetTarget(m_pkPathRoot);
	m_pkPick->ClearResultsArray();

	NiPoint3 kPickStart = kPos + NiPoint3(0, 0, 30.0f);

	for(int i = 0; i < 4; ++i)
	{
		m_pkPick->PickObjects(kPickStart, akDirs[i], true);
	}

	m_pkPick->RemoveTarget();

	NiPick::Results& rkResults = m_pkPick->GetResults();
	if(rkResults.GetSize() == 0)
	{
		return false;
	}

	NiPick::Record *pkRecord = rkResults.GetAt(0);
	return pkRecord->GetDistance();
}

NiPoint3 PgActor::GetDirectionVector(BYTE byDirection)
{
	bool bLeft = ((byDirection & DIR_LEFT) == 0 ? false : true);
	bool bRight = ((byDirection & DIR_RIGHT) == 0 ? false : true);
	bool bUp = ((byDirection & DIR_UP) == 0 ? false : true);
	bool bDown = ((byDirection & DIR_DOWN) == 0 ? false : true);

	NiPoint3 kMovingDir = NiPoint3::ZERO;

	if(bLeft || bRight)
	{
		kMovingDir += m_kPathNormal.UnitCross(bLeft ? -NiPoint3::UNIT_Z : NiPoint3::UNIT_Z);
	}

	if(bUp || bDown)
	{
		kMovingDir += (bUp ? m_kPathNormal : -m_kPathNormal);
	}
	
	kMovingDir.Unitize();
	return kMovingDir;
}

float PgActor::TraceFly(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, float fLimitZ, float fFloatHeight, bool bCanRotate)
{
	SetFreeMove(true);

	bool bTracing = false;
	NiPoint3 const &rkCurPos = GetPos();
	NiPoint3 kDiffPos = m_kTargetLoc - rkCurPos;
	float const fDistance = kDiffPos.Length();
	kDiffPos.Unitize();
	NiPoint3 kNextPos = GetTranslate();
	if( fLimitDistance < fDistance )
	{
		// 열심히 쫒아가자
		kNextPos = rkCurPos + (kDiffPos * fSpeed * fFrameTime) + (kDiffPos * (fDistance-fLimitDistance) * fAccelateScale * fFrameTime);
		bTracing = true;
	}
	else
	{
		bTracing = false;
	}
	{
		float const fDiffZ = m_kTargetLoc.z + fFloatHeight - rkCurPos.z;
		if( fLimitZ > NiAbs(fDiffZ) )
		{
			//
		}
		else
		{
			//if( 0 > fDiffZ ) // 내가 밑에 있다
			//if( 0 < fDiffZ ) // 내가 위에 있다
			kNextPos.z += (fDiffZ * fFrameTime);
		}
	}

	if(GetTranslate() != kNextPos)
	{
		m_pkController->setPosition( NxExtendedVec3(kNextPos.x, kNextPos.y, kNextPos.z) );
		SetTranslate(kNextPos);
		SetWorldTranslate(kNextPos);

		if( bCanRotate )
		{
			ConcilDirection(kDiffPos, true);
		}
		SetPositionChanged(true);
	}
	return fDistance;
}

float PgActor::TraceGround(float fSpeed, float fFrameTime, float fLimitDistance, float fAccelateScale, bool bCanRotate)
{
	return 0;
}

bool PgActor::Walk(BYTE byDir, float fSpeed, float fFrameTime, bool bCorrectFinalPos)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.Walk"), g_pkApp->GetFrameCount()));
//	if(!IsMyActor() && m_pkAction && m_pkAction->GetBirthTime() != 0 && m_pkPilot->GetUnit()->UnitType() != UT_MONSTER)
//	{
//		// Latency와 실제 거리 차에 따라서 속도를 조절한다.
//		DWORD dwAverageLatency = PgActor::GetAverageLatency();
//		//DWORD dwBirthDelay = PgActor::GetSynchronizedTime() - m_pkAction->GetBirthTime();
//		//PG_ASSERT_LOG(dwBirthDelay > 0);
//		
//		// BirthDelay가 Averange Latency보다 커야 한다.
//		//fSpeed *= (1.0f - dwAverageLatency / 4000.0f);
//	}

	if(fSpeed == 0)
	{
		return	false;
	}

	NiPoint3 kMovingDir = GetDirectionVector(byDir);

	// 가야될 포인트가 지정되어 있으면, 그 방향으로 간다.
	if(m_bWalkingToTarget)
	{
		bool bGoingToTarget = false;
		if(m_bWalkingToTargetForce || kMovingDir == NiPoint3::ZERO)
		{
			kMovingDir = m_kTargetDir;
			NiPoint3 kCross = m_kPathNormal.UnitCross(kMovingDir);
			kCross.Unitize();

			// 축이 꼬일 때는 왼쪽을 보게 한다,
			if(kCross.SqrLength() < 0.0001f)
			{
				kCross = NiPoint3::UNIT_Z;
			}

			byDir = (kCross == NiPoint3::UNIT_Z ? DIR_LEFT : DIR_RIGHT);
			bGoingToTarget = true;
		}

		//NiPoint3 kDiff = m_kTargetLoc - GetTranslate();	//	쓰이지 않는 변수? 왜 존재하는가? leesg213
		//kDiff.z = 0;	

		NiPoint3 kCharPos(GetPosition());
		NiPoint3 kNextPos = kCharPos + (kMovingDir * fSpeed * fFrameTime);
		kNextPos.z = 0;

		NiPoint3	kRemainDist = (m_kTargetLoc - kCharPos);
		kRemainDist.z = 0;

		float fRemainLength = kRemainDist.Length();
		float kDiffLength = (kNextPos - m_kStartLoc).Length();

	/*	_PgOutputDebugString("[PgActor::Walk] Actor:%s (%s) kMovingDir:(%f,%f,%f) kCharPos:(%f,%f,%f) kNextPos:(%f,%f,%f) TargetLoc:(%f,%f,%f) StartLoc:(%f,%f,%f) m_kTargetDir:(%f,%f,%f) fSpeed:%f fFrameTime:%f fRemainLength : %f\n",
			MB(GetPilot()->GetName()),
			MB(GetPilot()->GetGuid().str()),
			kMovingDir.x,kMovingDir.y,kMovingDir.z,
			kCharPos.x,kCharPos.y,kCharPos.z,
			kNextPos.x,kNextPos.y,kNextPos.z,
			m_kTargetLoc.x,m_kTargetLoc.y,m_kTargetLoc.z,
			m_kStartLoc.x,m_kStartLoc.y,m_kStartLoc.z,
			m_kTargetDir.x,m_kTargetDir.y,m_kTargetDir.z,
			fSpeed,fFrameTime,
			fRemainLength);*/

		//_PgOutputDebugString("m_fDiffLength = %.4f, DiffLength = %.4f\n", m_fDiffLength, kDiffLength);
		if(fRemainLength <= m_fWalkToTargetLocSkillRange)
		{
			SetMovingDelta(NX_ZERO);

			// 정해진 방향으로 가는 것을 끝내면서, 지정된 액션을 한다.
			SetNoWalkingTarget(false);
			return false;
		}
		if(m_fDiffLength <= kDiffLength)
		{
			SetMovingDelta(NX_ZERO);
			if(bCorrectFinalPos)
			{
				SetPosition(m_kTargetLoc);
			}
			else
			{
				m_kTargetLoc.z = static_cast<float>(m_pkController->getPosition().z);
				SetPosition(m_kTargetLoc);
			}

			// 정해진 방향으로 가는 것을 끝내면서, 지정된 액션을 한다.
			SetNoWalkingTarget(false);
			return false;
		}
	}
	else if(byDir == DIR_NONE)
	{
	//	return false;
	}
	
	kMovingDir.z = 0;
	kMovingDir.Unitize();

	NxVec3 kMovingDelta;
	NiPhysXTypes::NiPoint3ToNxVec3(kMovingDir, kMovingDelta);

	SetMovingDelta(m_kMovingDelta+(kMovingDelta * fSpeed * m_fMovingSpeedScale));

	// TODO : bDontChangeDirection 구현.
	// No Concil을 해주면 될듯..

	m_kMovingDir.z = 0;
	if(m_bLockBidirection)
	{
		// 2방향으로만 보는 경우
		if((byDir & DIR_VERTICAL) != byDir)
		{
			m_kMovingDir = m_kPathNormal.UnitCross(NiPoint3::UNIT_Z * (byDir & DIR_LEFT ? -1.0f : 1.0f));
			//_PgOutputDebugString("[Set m_kMovingDir 1] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);

		}
	}
	else
	{
		m_kMovingDir = kMovingDir;
		//_PgOutputDebugString("[Set m_kMovingDir 2] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);
	}
	return	true;
}

void PgActor::SetTargetHeadSize(float const fTargetHeadSize, float const fTransitSpeed)
{
	m_fTargetHeadSize = GetDefaultHeadSize()*fTargetHeadSize;
	m_fHeadSizeTransitSpeed = fTransitSpeed;
}

void PgActor::SetFreezed(bool const bTrue, bool const bSetAni, bool const bDoDmgAction)
{
	if(GetFreezed() != bTrue)
	{// 설정이 달라져
		if(bTrue)
		{//얼려야 하고
			m_iFreezeStatus |= EFS_FREEZED;
			lwCommonSkillUtilFunc::TryMustChangeActorAction(this, ACTIONNAME_IDLE);
			if(bDoDmgAction)
			{// 얼어도 데미지 액션을 한다면
				m_iFreezeStatus |= EFS_ALLOW_DMG_ACTION;
			}
		}
		else
		{// 얼은것이 풀려야 한다면
			m_iFreezeStatus = EFS_NONE;
		}

		if(bSetAni)
		{
			if( GetFreezed() )
			{
				SetAnimSpeedInPeriod(0.0001f, 99999999);	//	애니메이션 정지
			}
			else
			{
				SetAnimOriginalSpeed();
			}
		}
	}
}

bool PgActor::GetFreezed() const
{
	return m_iFreezeStatus & EFS_FREEZED;
}
bool PgActor::CanDmgActionOnFreezed() const
{
	return m_iFreezeStatus & EFS_ALLOW_DMG_ACTION;
}

void PgActor::SetNotActionShift(bool const bTrue)
{
	if(m_bNotActionShift != bTrue)
	{
		m_bNotActionShift = bTrue;
	}
}

bool PgActor::IsActionShift(PgAction const* pkAction) const
{
	if(pkAction && pkAction->GetActionOptionEnable(PgAction::AO_IGNORE_NOTACTIONSHIFT))
	{
		return true;
	}

	return !m_bNotActionShift;
}

void PgActor::SetBlowUp(bool const bBlowUp)
{	
	m_bBlowUp = bBlowUp;	
	if(g_pkWorld && bBlowUp && m_fBlowUpStartTime == 0)
	{
		m_fBlowUpStartTime = g_pkWorld->GetAccumTime();
	}
}


void PgActor::SetInvisible(bool const bTrue)
{
	m_bInvisible = bTrue;

	if(m_bInvisible)
	{
		if(IsMyActor())
		{
			SetTargetAlpha(GetAlpha(), 0.5f,1.0f);
		}
	}
	else
	{
		SetTargetAlpha(GetAlpha(), 1.0f, 1.0f);
	}
}

NiPoint3 PgActor::GetWalkingTargetDir()
{
	return m_kTargetDir;
}

void PgActor::SetNoWalkingTarget(bool bDoNextAction)
{
	if(IsMeetFloor() == false)
	{
		NILOG(PGLOG_ERROR,"=================== [Telejump Assert] Actor's Meet Floor != true ===================\n");
	}

	SetMeetFloor(false);
	m_bWalkingToTarget = false;
	m_bWalkingToTargetForce = false;
	m_kTargetLoc = NiPoint3::ZERO;
	m_kTargetDir = NiPoint3::ZERO;
	m_fDiffLength = 0.0f;
	m_bCheckCliff = false;	//	leesg213 추가함
	m_fWalkToTargetLocSkillRange = 0.0f;
	
	if(bDoNextAction && m_kTargetWalkingNextAction.length() != 0)
	{
		ReserveTransitAction(m_kTargetWalkingNextAction.c_str());
	}
}

void PgActor::SetWalkingTarget(BM::GUID &rkGuid, bool const bForceToTarget, char const *pcNextAction, bool bCheckCliff,float fSkillRange)
{
	PgPilot *pkPilot = g_kPilotMan.FindPilot(rkGuid);
	if(!pkPilot)
	{
		return;
	}

	PgActor *pkActor = dynamic_cast<PgActor *> (pkPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	SetWalkingTargetLoc(pkActor->GetTranslate(), bForceToTarget, pcNextAction, bCheckCliff,fSkillRange);
}

void PgActor::SetWalkingTargetLoc(NiPoint3 const &kTargetLoc, bool const bForceToTarget, char const *pcNextAction, bool bCheckCliff,float fSkillRange)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetWorkingTarget"), g_pkApp->GetFrameCount()));
	// bForceToTarget : Target으로 움직이는 동안 강제 캔슬이 안되게 할 것인가.
	NxExtendedVec3 kStartLocEx = m_pkController->getPosition();
	NxVec3 kStartLoc(static_cast<NxReal>(kStartLocEx.x), static_cast<NxReal>(kStartLocEx.y), static_cast<NxReal>(0.0f));
	NiPhysXTypes::NxVec3ToNiPoint3(kStartLoc, m_kStartLoc);

	NiPoint3	kAdjustedTargetLoc = GetWorld()->FindActorFloorPos(kTargetLoc);

	m_kTargetLoc = kAdjustedTargetLoc;
	m_kTargetDir = (kAdjustedTargetLoc - m_kStartLoc);
	m_kTargetDir.z = 0;
	m_fDiffLength = m_kTargetDir.Length();
	if(fSkillRange!=-1)
		m_fWalkToTargetLocSkillRange = fSkillRange;

	if(m_fDiffLength == 0.0f)
	{
		return;
	}

	m_kTargetDir.Unitize();

	if(GetPilot())
	{
		_PgOutputDebugString("[PgActor::SetWalkingTargetLoc] Actor:%s(%s) TargetLoc:(%f,%f,%f) m_kTargetDir:(%f,%f,%f) kAdjustedTargetLoc:(%f,%f,%f) bForceToTarget:%d fSkillRange:%f\n", MB(GetPilot()->GetName()), MB(GetPilotGuid().str()), kTargetLoc.x,kTargetLoc.y,kTargetLoc.z, m_kTargetDir.x,m_kTargetDir.y,m_kTargetDir.z, kAdjustedTargetLoc.x,kAdjustedTargetLoc.y,kAdjustedTargetLoc.z,bForceToTarget,fSkillRange);
	}

	m_bWalkingToTarget = true;
	m_bWalkingToTargetForce = bForceToTarget;
	m_bCheckCliff = bCheckCliff;
	m_kTargetWalkingNextAction = (pcNextAction == 0 ? std::string("") : pcNextAction);
//	ConcilDirection(m_kTargetDir, true);
}

NiPoint3 const &PgActor::GetWalkingTargetLoc()
{
	return m_kTargetLoc;
}

//! 목표 지점으로 움직이도록 설정 되어 있는지 리턴.
bool PgActor::GetWalkingToTarget()
{
	return m_bWalkingToTarget;
}

//! 캐릭터의 Scale 을 변화시킨다.
void PgActor::SetTargetScale(float fScale,unsigned long ulTotalScaleChangeTime)
{
	if (GetActorManager())
		m_fStartScale = GetNIFRoot()->GetScale();
	else
		m_fStartScale = 1.0f;

	m_fTargetScale = fScale;
	m_ulScaleChangeStartTime = BM::GetTime32();
	m_ulTotalScaleChangeTime = ulTotalScaleChangeTime;
}

bool PgActor::IsAlphaTransitDone()
{
	return m_AlphaTransitInfo.m_fAlphaTransitionTime <= m_AlphaTransitInfo.m_fAlphaAccumTime;
}

void PgActor::SetTargetAlpha(float fStartAlpha, float fTargetAlpha, float fTransitionTime, bool bIgnoreLoadingFisnishInit)
{
	m_AlphaTransitInfo.m_fPrevAlpha = m_AlphaTransitInfo.m_fCurrentAlpha = NiClamp(fStartAlpha, 0.0f, 1.0f);
	m_AlphaTransitInfo.m_fTargetAlpha = NiClamp(fTargetAlpha, 0.0f, 1.0f);
	m_AlphaTransitInfo.m_fAlphaTransitionTime = fTransitionTime;
	m_AlphaTransitInfo.m_fAlphaAccumTime = 0.0f;
	m_AlphaTransitInfo.bUseLoadingFinishInit = !bIgnoreLoadingFisnishInit;
}
void PgActor::SetTargetColor(const NiColor& kColor, float const fTransitionTime)
{
	m_ColorTransitInfo.m_kPrev = m_ColorTransitInfo.m_kCurrent = m_ColorTransitInfo.m_kTarget;
	m_ColorTransitInfo.m_kTarget = kColor;
	m_ColorTransitInfo.m_fTransitionTime = fTransitionTime;
	m_ColorTransitInfo.m_fAccumTime = 0.0f;
}

void PgActor::SetTargetSpecular(const NiColor &kColor, float fTransitTime)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetActorColor"), g_pkApp->GetFrameCount()));
	// Material 원래 색상을 읽어서 저장해 둔다.
	if(!m_bMaterialColorCached)
	{
		m_bMaterialColorCached = StoreDefaultMaterialColor(NiDynamicCast(NiNode, GetNIFRoot()), true);
		if(!m_bMaterialColorCached)
		{
			return;
		}
	}

	// 색상을 변화시키기 전에, 현재 색깔을 저장해 둔다.
	m_kMaterialCurrentColorContainer.clear();
	for(MaterialColorContainer::iterator itr = m_kMaterialColorContainer.begin();
		itr != m_kMaterialColorContainer.end();
		++itr)
	{
		NiMaterialProperty *pkMaterialProp = itr->first;
		m_kMaterialCurrentColorContainer.insert(std::make_pair(pkMaterialProp, ColorSet(pkMaterialProp->GetAmbientColor(), pkMaterialProp->GetEmittance(),pkMaterialProp->GetSpecularColor(),pkMaterialProp->GetDiffuseColor())));
	}

	m_SpecularTransitInfo.m_kPrev = m_SpecularTransitInfo.m_kCurrent = m_SpecularTransitInfo.m_kTarget;
	m_SpecularTransitInfo.m_kTarget = kColor;
	m_SpecularTransitInfo.m_fTransitionTime = fTransitTime;
	m_SpecularTransitInfo.m_fAccumTime = 0.0f;

	TurnOnSpecular();
}

//!	액터의 아이템 모델을 임시로 교체한다.
void	PgActor::ChangeItemModel(eEquipLimit kItemPos,char const *pkNewItemXMLPath)
{
	_PgOutputDebugString("PgActor::ChangeItemModel Actor : %s iItemPos : %d pkNewItemXMLPath : %s\n", MB(GetPilot()->GetName()),kItemPos,pkNewItemXMLPath);

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ChangeModel"), g_pkApp->GetFrameCount()));
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kItemPos);	
	if(itr != m_kPartsAttachInfo.end())
	{
		PgItemEx *pkEquipItem = itr->second;
		if(!pkEquipItem) 
		{
			_PgOutputDebugString("PgActor::ChangeItemModel pkEquipItem is NULL\n");
			return;
		}
		_PgOutputDebugString("PgActor::ChangeItemModel pkEquipItem : %s\n",pkEquipItem->GetID().c_str());

		NiNode* pkModel = NiDynamicCast(NiNode, GetCharRoot());
		const	char* pcTargetDummy = pkEquipItem->GetTargetPoint();
		//	기존의 메쉬를 떼어낸다.
		NiNode	*pkTargetNode = NULL;
		if(pkEquipItem->GetMeshRoot())
		{
			pkTargetNode = pkEquipItem->GetMeshRoot()->GetParent();
			if(pkTargetNode)
			{
				_PgOutputDebugString("PgActor::ChangeItemModel pkTargetNode:%s EquipItemMeshRoot : %s\n", pkTargetNode->GetName(),pkEquipItem->GetMeshRoot()->GetName());

				pkTargetNode->DetachChild(pkEquipItem->GetMeshRoot());
			}
			else
			{
				_PgOutputDebugString("PgActor::ChangeItemModel pkTargetNode is NULL\n");
			}

		}
		else
		{
			_PgOutputDebugString("PgActor::ChangeItemModel pkEquipItem->GetMeshRoot() is NULL\n");
		}


		_PgOutputDebugString("PgActor::ChangeItemModel ChangeModel Start\n");
		pkEquipItem->ChangeModel(pkNewItemXMLPath);
		_PgOutputDebugString("PgActor::ChangeItemModel ChangeModel Finished\n");

		//	새 메쉬를 붙인다.
		if(pkTargetNode)
		{
			_PgOutputDebugString("PgActor::ChangeItemModel Attach New Mesh Root\n");
			pkTargetNode->AttachChild(pkEquipItem->GetMeshRoot(), true);
			pkTargetNode->UpdateNodeBound();
			pkTargetNode->UpdateProperties();
			pkTargetNode->UpdateEffects();
			pkTargetNode->Update(0.0f, true);
		}

		return;
	}
	_PgOutputDebugString("PgActor::ChangeItemModel Parts Not Found\n");
}
//!	액터의 무기 모델을 원래 것으로 돌려놓는다.
void	PgActor::RestoreItemModel(eEquipLimit kItemPos)
{
	if(!GetPilot())
	{
		return;
	}

	_PgOutputDebugString("PgActor::RestoreItemModel Actor : %s iItemPos : %d\n", MB(GetPilot()->GetName()),kItemPos);

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.RestorModel"), g_pkApp->GetFrameCount()));
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kItemPos);	
	if(itr != m_kPartsAttachInfo.end())
	{
		PgItemEx *pkEquipItem = itr->second;
		if(!pkEquipItem) 
		{
			_PgOutputDebugString("PgActor::RestoreItemModel pkEquipItem is NULL\n");
			return;
		}
		_PgOutputDebugString("PgActor::RestoreItemModel pkEquipItem : %s\n",pkEquipItem->GetID().c_str());

		if(pkEquipItem->GetOriginalMeshRoot() == 0)
		{
			_PgOutputDebugString("PgActor::RestoreItemModel pkEquipItem->GetOriginalMeshRoot() is NULL\n");
			return;
		}

		NiNode* pkModel = NiDynamicCast(NiNode, GetCharRoot());
		const	char* pcTargetDummy = pkEquipItem->GetTargetPoint();
		//	기존의 메쉬를 떼어낸다.
		NiNode	*pkTargetNode=NULL;
		if(pkEquipItem->GetMeshRoot())
		{
			pkTargetNode = pkEquipItem->GetMeshRoot()->GetParent();
			if(pkTargetNode)
			{
				_PgOutputDebugString("PgActor::RestoreItemModel pkTargetNode:%s EquipItemMeshRoot : %s\n", pkTargetNode->GetName(),pkEquipItem->GetMeshRoot()->GetName());

				pkTargetNode->DetachChild(pkEquipItem->GetMeshRoot());
			}
			else
			{
				_PgOutputDebugString("PgActor::RestoreItemModel pkTargetNode is NULL\n");
			}

		}
		else
		{
			_PgOutputDebugString("PgActor::RestoreItemModel pkEquipItem->GetMeshRoot() is NULL\n");
		}

		pkEquipItem->RestoreOriginalModel();

		//	새 메쉬를 붙인다.
		if(pkTargetNode)
		{
			_PgOutputDebugString("PgActor::RestoreItemModel Attach New Mesh Root\n");

			pkTargetNode->AttachChild(pkEquipItem->GetMeshRoot(), true);
			pkTargetNode->UpdateNodeBound();
			pkTargetNode->UpdateProperties();
			pkTargetNode->UpdateEffects();
			pkTargetNode->Update(0.0f, true);
		}

	
		return;
	}

	_PgOutputDebugString("PgActor::RestoreItemModel Parts Not Found\n");
}

//void PgActor::TwistActor(int const fTwistTimes, float const fSpeed)
//{
//	m_iTwistTimes = fTwistTimes;
//	m_fTwistSpeed = fSpeed;
//}

void PgActor::SetDownState(bool bDownState)
{
	m_bDownState = bDownState;
	GetPilot()->SetAbil(AT_POSTURE_STATE, static_cast<int>(bDownState));
}

void PgActor::RestoreTwistedActor()
{
	NiTransform kTransform = GetNIFRoot()->GetLocalTransform();
	kTransform.m_Rotate.MakeIdentity();
	GetNIFRoot()->SetLocalTransform(kTransform);
}

void PgActor::IncEquipCount()
{
	if (IsMyActor())
		return;

	::InterlockedIncrement(&m_iEquipCount);
//	_PgOutputDebugString("%s actor EquipCount (After Inc) = %d\n", MB(GetGuid().str()), m_iEquipCount);
}

void PgActor::DecEquipCount()
{
	if (IsMyActor())
		return;

	LONG iEquipCount = ::InterlockedDecrement(&m_iEquipCount);
#ifdef PG_ITEM_SEQUENCIAL_LOAD
	if (iEquipCount >= 0)
	{
		AttachItemInfo kInfo;
		m_kAttachItemLock.Lock();
		int	iAttachItemContainerSize = m_kAttachItemContainer.size();
		if(iAttachItemContainerSize > 0)
		{
			kInfo = m_kAttachItemContainer.front();
			if (kInfo.pItem == NULL)
			{
				// Loading Thread에 집어넣자...
				m_kAttachItemContainer.pop_front();
			}
		}
		m_kAttachItemLock.Unlock();

		if (kInfo.pItem == NULL && iAttachItemContainerSize > 0)
		{
			g_kEquipThread.DoEquipItem(this, this->GetGuid(), kInfo.iItemNo, kInfo.bSetDefaultItem, kInfo.iGender, kInfo.iClass);

		}
	}
#endif
//	_PgOutputDebugString("%s actor EquipCount (After Dec) = %d\n", MB(GetGuid().str()), m_iEquipCount);
}

void PgActor::Stop()
{
}

void PgActor::StartJump(float const fHeight)
{
	if(!GetJump())
	{
		m_fJumpAccumHeight = 0.0f;
	}

	SetMeetFloor(false);
	SetJump(true);
	SetJumpTime(0.0f);
	SetInitialVelocity(NiSqrt(2.0f * -GetGravity() * fHeight));
	SetSlide(false);
	m_kSlideVector.zero();
}

float PgActor::StartTeleJump(NiPoint3 const &kTargetPoint, float const fHeight)
{
	NxExtendedVec3 kCharPos = m_pkController->getPosition();
	NiPoint3 kActorPos(static_cast<float>(kCharPos.x), static_cast<float>(kCharPos.y), static_cast<float>(kCharPos.z));
	float fDiffHeight = kTargetPoint.z - kActorPos.z;
	float fPeak = fHeight + (fDiffHeight > 0.0f ? fDiffHeight : 0.0f);

	// 점프하자 (단위가 Inch이다)
	StartJump(fPeak);

	NiPoint3 kDiffDistance = kTargetPoint - kActorPos;
	kDiffDistance.z = 0;
	float fDistance = kDiffDistance.Length();
	
	// 체공 시간
	float fAchiveToPeak = -GetInitialVelocity() / GetGravity();
	float fAchiveToGround = NiSqrt(2.0f * (fDiffHeight > 0.0f ? fHeight : -fDiffHeight + fHeight) * -1.0f / GetGravity());
	float fAchiveTime = fAchiveToPeak + fAchiveToGround;

	return fDistance / fAchiveTime;	
}
bool PgActor::GetForceSync()
{
	return m_bForceSync;
}

void PgActor::SetForceSync(bool bSync)
{
	m_bForceSync = bSync;
}

void PgActor::StopJump()
{
	SetJump(false);
}

bool PgActor::GetJump()	const
{
	return m_bJump;
}

void PgActor::ClearActionState()
{
//	m_uiStateCount = 0;
//	m_kMonitorSlot.clear();
}

unsigned int PgActor::GetActionState(char const *pcActionID)
{
	//MonitorSlot::iterator itr = m_kMonitorSlot.find(std::string(pcActionID));
	//if(itr != m_kMonitorSlot.end())
	//{
	//	return itr->second;
	//}

	return 0;
}

//! Chain Attack Count 를 1 증가시킨다. 단 최근 증가된 시간에서 3초 이상 흘렀을 경우 1 로리셋.
void PgActor::IncreaseChainAttackCount(int const iSkillNo)
{
	g_kChainAttack.IncreaseChainAttackCount(iSkillNo);
}

void PgActor::SetFreeMove(bool bFreeMove)
{
	m_bFreeMove = bFreeMove;
}

void PgActor::MoveActorAbsolute(NxVec3 const &kMoveAbs)
{
	//NxVec3 kResultVec3;
	//kResultVec3.x = (NxReal)(m_pkController->getDebugPosition().x + kMoveAbs.x);
	//kResultVec3.y = (NxReal)(m_pkController->getDebugPosition().y + kMoveAbs.y);
	//kResultVec3.z = (NxReal)(m_pkController->getDebugPosition().z + kMoveAbs.z);

	//// m_kMovingAbsolute은 Controller의 move때 필요한 데이터.
	//// move는.. 이 함수가 실행되고 나서 다음 프레임때 계산 되기 때문에..
	//// 한박자 늦게 움직인다. 그렇기 때문에. actor를 이번프레임 렌더 전에 움직여 줘서.
	//// 즉시 이동 된것 같은 현상처럼 보여준다.
	//m_pkPhysXActor->setGlobalPosition(kResultVec3);
	m_kMovingAbsolute += kMoveAbs;
}
void PgActor::MoveActor(NxVec3 kDelta)
{

	SetMovingDelta(m_kMovingDelta+kDelta);

	//_PgOutputDebugString("Moving Delta MoveActor: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
}

//!	무기 궤적 그리기 시작
void PgActor::StartWeaponTrail()
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.StartWeaponTrail"), g_pkApp->GetFrameCount()));
	EndWeaponTrail();
	
	if(g_spTrailNodeMan && m_spWeaponTrailNode == 0)
	{
		PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(EQUIP_LIMIT_WEAPON);	//	5번이 무기
		if(itr != m_kPartsAttachInfo.end())
		{
			PgItemEx *pkEquipItem = itr->second;
			m_spWeaponTrailNode = g_spTrailNodeMan->StartNewTrail(
				NiDynamicCast(NiAVObject,pkEquipItem->GetMeshRoot()),
				pkEquipItem->GetTrailInfo().m_kTexturePath,pkEquipItem->GetTrailInfo().m_iTotalTime/1000.0f,
				pkEquipItem->GetTrailInfo().m_iBrightTime/1000.0f);
		}

	}
}
//!	무기 궤적 그리기 종료
void PgActor::EndWeaponTrail()
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.EndWeaponTrail"), g_pkApp->GetFrameCount()));
	if(g_spTrailNodeMan && m_spWeaponTrailNode)
	{
		g_spTrailNodeMan->StopTrail(m_spWeaponTrailNode,false);
		m_spWeaponTrailNode = 0;
	}
}
//!	몸 궤적 그리기 시작
void PgActor::StartBodyTrail(char const* strTexPath, int iTotalTime, int iBrightTime)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.StartBodyTrail"), g_pkApp->GetFrameCount()));
	if(NULL == strTexPath)
	{
		return;
	}
	StartTrail(ETAT_BODY, strTexPath, iTotalTime, iBrightTime);
}

//!	몸 궤적 그리기 종료
void PgActor::EndBodyTrail()
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.EndBodyTrail"), g_pkApp->GetFrameCount()));
	EndTrail(ETAT_BODY);
}

bool PgActor::StartTrail(eTrailAttachType const eTrailType, std::string const kTexPath, int iTotalTime, int iBrightTime)
{	
	EndTrail(eTrailType);

	if( IsHide() 
		|| !g_spTrailNodeMan
		|| ETAT_NONE >= eTrailType
		|| ETAT_MAX <= eTrailType
		|| kTexPath.empty()
		|| eTrailType >= static_cast<int>( m_kContTrail.size() )
		)
	{
		return false;
	}
	PgTrailNodePtr spTrailNode = NULL;
	//PgTrailNodePtr spTrailNode = GetTrailNode( eTrailType ); // NULL이 아니면 삭제가 안된것이므로
	//if( NULL != spTrailNode )
	//{// 실패 하고
	//	return false;
	//}
	NiAVObject* pkNode = GetActorManager()->GetNIFRoot();	// 루트로 부터 시작해서
	if( NULL == pkNode )
	{
		return false;
	}
	std::string kTargetNode;
	switch( eTrailType )
	{// Actor 몸에 스키닝으로 붙어있는 아이템 메쉬등의 노드들은 찾지 못하므로 유의
	case ETAT_BODY:
		{// char_root
			//pkNode = GetActorManager()->GetNIFRoot();
		}break;
	case ETAT_R_HAND:
		{
			kTargetNode = "p_wp_r_hand";
		}break;
	case ETAT_L_HAND:
		{
			kTargetNode = "p_wp_l_hand";
		}break;
	case ETAT_R_FOOT:
		{
			kTargetNode = "Boots_R_Bone02";
		}break;
	case ETAT_L_FOOT:
		{
			kTargetNode = "Boots_L_Bone02";
		}break;
	case ETAT_CENTER:
		{
			kTargetNode = "Bip01";
		}break;
	default:
		{
			return false;
		}break;
	}
	if(ETAT_BODY != eTrailType)
	{
		pkNode = pkNode->GetObjectByName( kTargetNode.c_str() );
	}

	if( NULL == pkNode )
	{
		return false;
	}

	spTrailNode = g_spTrailNodeMan->StartNewTrail(
		pkNode,
		kTexPath,iTotalTime/1000.0f,
		iBrightTime/1000.0f);
	SetNodeTrail(eTrailType, spTrailNode);
	return true;
}

bool PgActor::EndTrail(eTrailAttachType const eTrailType)
{	
	if(!g_spTrailNodeMan
		|| ETAT_NONE >= eTrailType
		|| ETAT_MAX <= eTrailType
		|| eTrailType >= static_cast<int>( m_kContTrail.size() )
		)
	{
		return false;
	}
	PgTrailNodePtr spTrailNode = GetTrailNode(eTrailType);
	if(NULL == spTrailNode)
	{
		return false;
	}
	g_spTrailNodeMan->StopTrail(spTrailNode,false);
	spTrailNode = NULL;
	return true;
}

PgTrailNodePtr PgActor::GetTrailNode(eTrailAttachType const eTrailType)
{
	if( ETAT_NONE >= eTrailType
		|| ETAT_MAX <= eTrailType
		|| eTrailType >= static_cast<int>( m_kContTrail.size() )
		)
	{
		return NULL;
	}

	return m_kContTrail.at(eTrailType);
}
bool PgActor::SetNodeTrail(eTrailAttachType const eTrailType, PgTrailNodePtr& rkNodeTrail)
{
	if( ETAT_NONE >= eTrailType
		|| ETAT_MAX <= eTrailType
		|| eTrailType >= static_cast<int>( m_kContTrail.size() )
		)
	{
		return false;
	}
	m_kContTrail.at(eTrailType) = rkNodeTrail;
	return true;
}

void	PgActor::StartGodTime(float fTotalGodTime)
{
	if(!g_pkWorld)
	{
		return;
	}

	NILOG(PGLOG_LOG,"StartGodTime() Actor:%s %s fTotalGodTime:%f\n",MB(GetGuid().str()),MB(GetPilot()->GetName()),fTotalGodTime);

	if(fTotalGodTime<=0)
	{
		m_kGodTimeInfo.m_bGodTime = false;
		g_kStatusEffectMan.RemoveStatusEffectFromActor2(GetPilot(),m_iGodTimeStatusEffectInstanceID);
		m_iGodTimeStatusEffectInstanceID = -1;
		return;
	}

	m_kGodTimeInfo.m_bGodTime = true;
	m_kGodTimeInfo.m_fTotalTime = fTotalGodTime;
	m_kGodTimeInfo.m_fStartTime = g_pkWorld->GetAccumTime();
	m_kGodTimeInfo.m_fBlinkTime = g_pkWorld->GetAccumTime();

	//RemoveStatusEffectFromActor2로 지우는 것들은 MadeByItem을 켜준다.
	if(m_iGodTimeStatusEffectInstanceID==-1)
	{
		m_iGodTimeStatusEffectInstanceID = g_kStatusEffectMan.AddStatusEffectToActor(GetPilot(), GetPilot(), "se_semi_transparent",0,0,0,true, true);
	}
}
void	PgActor::StartDamageBlink(bool bStart)
{
	if(!g_pkWorld)
	{
		return;
	}

	m_kDamageBlinkInfo.bEnable = bStart;

	if(!GetPilot()) return;

	if(bStart)
	{
		m_kDamageBlinkInfo.m_fStartTime = g_pkWorld->GetAccumTime();
		m_kDamageBlinkInfo.m_fLastTime = g_pkWorld->GetAccumTime();
		m_kDamageBlinkInfo.m_fPeriod = 0.05f;
		m_kDamageBlinkInfo.m_fTotalTime = 0.10f;
		m_kDamageBlinkInfo.m_bToggle = true;

		//	이미 있는거는 지우고
		g_kStatusEffectMan.RemoveStatusEffectFromActor2(GetPilot(),m_iDamageBlinkStatusEffectInstanceID);

		//	새로 넣자
		m_iDamageBlinkStatusEffectInstanceID = g_kStatusEffectMan.AddStatusEffectToActor(GetPilot(), GetPilot(), GetDamageBlinkID(),0,0,1,true, true);
	}
	else
	{
		g_kStatusEffectMan.RemoveStatusEffectFromActor2(GetPilot(),m_iDamageBlinkStatusEffectInstanceID);
	}
}

NxActor *PgActor::GetPhysXActor() const
{
	return m_pkPhysXActor;
}

NiPoint3	PgActor::GetPos()
{
	return	GetPosition();
}

NiPoint3 PgActor::GetFloorLoc()
{
	return NiPoint3::ZERO;
}

PgAction *PgActor::GetAction() const
{
	return m_pkAction;
}
//!	현재 플레이 중인 액션을 삭제한다.
//void	PgActor::ReleaseAction()
//{
//	if(m_pkAction)
//	{
//		m_pkAction->LeaveFSM(this,m_pkAction);
//		SAFE_DELETE(m_pkAction);
//	}
//}
PgTrigger *PgActor::GetCurrentTrigger()
{
	return m_pkCurrentTrigger;
}

void PgActor::SetCurrentTrigger(PgTrigger *pkTrigger)
{
	m_pkCurrentTrigger = pkTrigger;
	m_bCurrentTriggerAct = false;
}

bool PgActor::GetIsInTrigger() const
{
	if( m_pkCurrentTrigger )
	{
		return m_pkCurrentTrigger->Enable();
	}
	return false;
}

//bool PgActor::IsToLeft()
//{
//	return (m_byLookingDirection == DIR_LEFT ? true : false);
//}
//
//
//void PgActor::ToLeft(bool const bLeft,bool bTurnRightAway)	
//{
//	SetDirection(bLeft ? DIR_LEFT : DIR_RIGHT);	
//
//	if(bTurnRightAway)
//	{
//		NiPoint3	kMovingDir = m_kPathNormal.UnitCross(NiPoint3::UNIT_Z * (m_eDirection & DIR_LEFT ? -1.0f : 1.0f));
//		LookAt(GetPos()+kMovingDir*30,bTurnRightAway);
//	}
//}

void PgActor::BlinkThis(bool const bBlink, int const iBlinkFreq)
{
	PG_WARNING_LOG(iBlinkFreq > 0);

	SetBlinkHide(false);
	m_fBlinkAccumTime = 0.0f;
	m_bBlink = bBlink;
	//! 0이 들어왔을때 어찌 하는게 좋을까.
	if (iBlinkFreq > 0)
	{
		m_fBlinkFreq = 1.0f / (float)iBlinkFreq;
	}
	else
	{
		PG_WARNING_LOG(m_bBlink == false);
		m_fBlinkFreq = 0.0f;
	}
}

//!	일정 시간동안 액터를 흔든다.
void PgActor::SetShakeInPeriod(float const fShakePower, int const iPeriod)
{
	m_iTotalShakeTime = iPeriod;
	m_iShakeStartTime = BM::GetTime32();
	m_fShakeValue = fShakePower;
	if(m_kOriginalPos == NiPoint3::ZERO)
		m_kOriginalPos = GetNIFRoot()->GetTranslate();
}

//!	일정 시간동안 애니메이션 스피드를 조정한다.
void PgActor::SetAnimSpeedInPeriod(float const fAnimSpeed, int const iPeriod)
{
	if(!g_pkWorld)
	{
		return;
	}

	m_fTotalAnimSpeedControlTime = iPeriod / 1000.0f;
	m_fAnimSpeedControlStartTime = g_pkWorld->GetAccumTime();
	m_fAnimSpeedControlValue = fAnimSpeed;
}

//!	애니메이션 스피드를 원래대로 돌린다.
void PgActor::SetAnimOriginalSpeed()
{
	m_fTotalAnimSpeedControlTime = 0;
	SetAnimSpeed(m_fOriginalAnimSpeed);
}

//! 애니메이션 스피드 얻기
float PgActor::GetAnimSpeed() const
{
	if(!GetActorManager())
		return	1.0f;

	NiControllerSequence *pkController = GetActorManager()->GetSequence(m_kSeqID);
	if(pkController)
	{
		return pkController->GetFrequency();
	}

	return 1.0f;
}

void PgActor::SetAnimSpeed(float fSpeed)
{
	if(!GetActorManager())
		return;

	if(fSpeed <=0 )
	{
		fSpeed = 0.0001f;
	}

	NiControllerSequence *pkController = GetActorManager()->GetSequence(m_kSeqID);
	if(!pkController)
	{
		//PG_ASSERT_LOG(!"Sequence Controller is Null!");
		return;
	}
	if(fSpeed == pkController->GetFrequency()) return;
	
	m_fOriginalAnimSpeed = fSpeed;
	pkController->SetFrequency(fSpeed);
	GetActorManager()->RebuildTimeline();
}

//! 죽어서 날아가던 액터가 카메라와 부딛혔는지 체크하여, 부딛힌 화면 좌표를 돌려준다.
bool PgActor::CheckCollWithCamera(float &fCollScreenX,float &fCollScreenY)
{
	if(!GetWorld()) return false;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CheckCollWithCamera"), g_pkApp->GetFrameCount()));

	NiCamera *pCamera = GetWorld()->m_kCameraMan.GetCamera();
	if(!pCamera) return false;

	const NiFrustum &kFrustum = pCamera->GetViewFrustum();
	NxVec3 vFrontBox[4];

	NxVec3 vCamPos(pCamera->GetWorldLocation().x,pCamera->GetWorldLocation().y,pCamera->GetWorldLocation().z);
	NxVec3 vCamD(pCamera->GetWorldDirection().x,pCamera->GetWorldDirection().y,pCamera->GetWorldDirection().z);
	NxVec3 vCamR(pCamera->GetWorldRightVector().x,pCamera->GetWorldRightVector().y,pCamera->GetWorldRightVector().z);
	NxVec3 vCamU(pCamera->GetWorldUpVector().x,pCamera->GetWorldUpVector().y,pCamera->GetWorldUpVector().z);

	float fNearDistance = pCamera->GetMinNearPlaneDist()+200;
	NxVec3 vFrontPos = vCamPos+vCamD*fNearDistance;
	vFrontBox[0] = vFrontPos+vCamR*kFrustum.m_fLeft+vCamU*kFrustum.m_fTop;
	vFrontBox[1] = vFrontPos+vCamR*kFrustum.m_fRight+vCamU*kFrustum.m_fTop;
	vFrontBox[2] = vFrontPos+vCamR*kFrustum.m_fRight+vCamU*kFrustum.m_fBottom;

	NxVec3 vGlobalPos = GetPhysXActor()->getGlobalPosition();

	NiPoint3 vCurrentPos(vGlobalPos.x,vGlobalPos.y,vGlobalPos.z);
	NxPlane plane(vFrontBox[0],vFrontBox[1],vFrontBox[2]);

	//if(m_kPrevWorldPos.x == 0 && m_kPrevWorldPos.y == 0 && m_kPrevWorldPos.z  == 0)
	if (m_fCheckCam_FirstDValue == 0.0f)
	{
		m_fCheckCam_FirstDValue = plane.distance(NxVec3(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z));
		//PG_ASSERT_LOG(IsZero(m_fCheckCam_FirstDValue) == false);
		m_kPrevWorldPos = vCurrentPos;
		return false;
	}

	float d_value = plane.distance(NxVec3(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z));

	NiPoint3 kPrevPosSaved = m_kPrevWorldPos;
	m_kPrevWorldPos = vCurrentPos;

	if(m_fCheckCam_FirstDValue*d_value<0)	//	뷰 평면과 부딛혔는가?
	{
		// 카메라 뷰 안에 있는가?
		bool bResult = pCamera->WorldPtToScreenPt(vCurrentPos,fCollScreenX,fCollScreenY);
		if(!bResult) return false;

		if(fCollScreenX<0 || fCollScreenY<0 || fCollScreenX>1 || fCollScreenY>1) return false;

		return true;
	}

	return false;
}

void PgActor::SetHide(bool const bHide)
{
	m_bHide = bHide;	

	SetHideBalloon(IsHide());
}

void PgActor::SetHideBalloon(bool const bHide)
{
	//	말풍선도 꺼주자
	if(m_pTextBalloon)
	{
		m_pTextBalloon->SetShow(!bHide);
	}

	if ( m_pMarkBalloon )
	{
		m_pMarkBalloon->SetShow(!bHide);
	}
}

void PgActor::SetBlinkHide(bool const bHide)
{
	m_bBlinkHide = bHide;
}

void PgActor::SetHideShadow(bool const bHide)
{
	m_bShadowHide = bHide;
	PgCircleShadow::ShowShadowRecursive(this,bHide==false);
}

void PgActor::SetHideMiniMap(bool const bHide)
{
	m_bMiniMapHide = bHide;
}

void	PgActor::AbilChangedByEffect(BM::GUID const& kCasterGUID, WORD wAbilID, int iAbilValue, int iDelta)
{
	int	const iCurrentValue = GetPilot()->GetAbil(wAbilID);
	int	const iChangedAmount = iDelta;
	//	int const iServerKnownValue = iCurrentValue - iAbilValue;

	if(AT_HP == wAbilID || AT_MP == wAbilID)
	{
		int	const iCurrent = iCurrentValue;
		int	const iNew = iAbilValue;
		bool bClampScreen = false;
		bool const bAbilTypeIsHP = AT_HP == wAbilID;

		PgPilot* pkCaster = g_kPilotMan.FindPilot(kCasterGUID);
		if(pkCaster)
		{
			PgActor* pkCasterActor = dynamic_cast<PgActor*>(pkCaster->GetWorldObject());
			bClampScreen = pkCasterActor->IsMyActor();

			if(bAbilTypeIsHP)
			{
				RefreshHPGaugeBar(iCurrent, iCurrent+iChangedAmount, pkCasterActor);
			}
		}

		if(bAbilTypeIsHP)
		{
			if(0 < iDelta)
			{
				if(iCurrent <= iNew 
					&& 0 != iCurrentValue
					)
				{//hp가 회복되는것이라면
					ShowHpMpNum(iChangedAmount);
				}
				else
				{
					ShowDamageNum(GetPos(),GetPos(),iChangedAmount, false, false, (BYTE)PgDamageNumMan::C_GREEN);
				}
			}
			else if( 0 > iDelta )
			{
				ShowDamageNum(GetPos(),GetPos(),iChangedAmount, false, false, (BYTE)PgDamageNumMan::C_RED);
			}

			RefreshHPGaugeBar(iCurrent, iNew, NULL);

			if(iAbilValue <= 0)	//	죽었다
			{
				AddEffect(ACTIONEFFECT_DIE,0);
			}
		}
		else
		{
			if(iCurrent <= iNew && 0 != iCurrentValue) //hp가 회복되는것이라면
			{
				ShowHpMpNum(iChangedAmount, bAbilTypeIsHP);
			}
		}
	}
}

void PgActor::SetCanHit(bool const bCanHit)
{
	m_bCanHit = bCanHit;
	NILOG(PGLOG_LOG, "%s is Can Hit = %d\n", this->m_kName, m_bCanHit);
	//_PgOutputDebugString("SetCanHit %d  Actor:%s %s\n", bCanHit,MB(GetPilot()->GetName()),MB(GetPilot()->GetGuid().str()));

	//for (unsigned int i = 0; i < PG_MAX_NB_ABV_SHAPES; ++i)
	//{
	//	if (m_apkPhysXCollisionSrcs[i] != NULL)
	//		m_apkPhysXCollisionSrcs[i]->SetActive(bCanHit);
	//}
}

bool PgActor::GetCanHit() const
{
	return	m_bCanHit
			&&	(!IsGodTime())
			&&	( 0 == GetPilot()->GetUnit()->GetAbil(AT_CANNOT_DAMAGE) );
}

void PgActor::SetThrowStart()
{
	m_bCheckCamColl = true;
	m_bNoName = true;
	m_fCheckCam_FirstDValue = 0.0f;
	m_kPrevWorldPos.x = m_kPrevWorldPos.y = m_kPrevWorldPos.z = 0;
}

void	PgActor::ShowChatBalloon(const EChatType eChatType, char const *Text, int const iUpTime, bool const bFake)
{
	if(!Text) return;
	ShowChatBalloon(eChatType, UNI(Text), iUpTime, bFake);
}

void	PgActor::ShowChatBalloon( EChatType const eChatType, std::wstring const &Text, int const iUpTime, bool const bFake)
{
	if( !m_pTextBalloon )
	{
		if( true == g_kBalloonMan2D.CreateNode( m_pTextBalloon ) )
		{
			m_pTextBalloon->Init(this);
		}
	}

	if ( m_pTextBalloon )
	{
		m_pTextBalloon->SetNewBalloon( eChatType, Text, iUpTime, bFake );
	}
	
	m_bInputNow = bFake;//입력중 상태 해지
}

void PgActor::ShowMark( MARK_TYPE const eMarkType )
{
	// Test
	if( !m_pMarkBalloon )
	{
		m_pMarkBalloon = g_kMarkBalloonMan.CreateNode();
		if ( !m_pMarkBalloon )
		{
			return;
		}
		m_pMarkBalloon->Init(this);
	}

	m_pMarkBalloon->SetNewMarkBalloon( eMarkType );
}

void	PgActor::ShowChatBalloon_Clear()
{
	if(!m_pTextBalloon) return;
	if(m_bInputNow)
	{
		m_bInputNow = false;
		m_pTextBalloon->SetExistTime( 100 );	//100/1000초 후에 종료된다.
	}
}

void	PgActor::ShowMark_Clear()
{
	if ( m_pMarkBalloon )
	{
		m_pMarkBalloon->Release();
	}
}

void	PgActor::SetMovingDelta(NxVec3 const &kDelta)
{
	m_kMovingDelta = kDelta; 

	//_PgOutputDebugString("Moving Delta Set: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
}

NxVec3	const& PgActor::GetMovingDelta()
{
	return	m_kMovingDelta;
}

//void PgActor::SetSyncTargetPos(NiPoint3 &rkTargetPos)
//{
//	m_kSyncPositionTarget = rkTargetPos;
//}
//
//void PgActor::SetSyncStartPos(NiPoint3 &rkStartPos)
//{
//	m_kSyncPositionStart = rkStartPos;
//}

bool PgActor::IsSync()
{
	return	m_bSync;
}

bool PgActor::ProcessActionQueue()
{
	if(m_bSync)
	{
		return true;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ProcessActionQueue"), g_pkApp->GetFrameCount()));

	// 새로온 패킷의 속도가, 이전 패킷의 속도보다 빠를 때만 그만큼 시간을 늦게 두어서 패킷을 처리함.
	BM::CAutoMutex kLock(m_kActionQueueMutex);

	bool bReturn = true;
	ActionQueue::iterator itr = m_kActionQueue.begin();
	while(itr != m_kActionQueue.end())
	{
		PgActionEntity &rkActionEntity = *itr;
		
		DWORD	dwNow = BM::GetTime32();

		if(dwNow <= m_dwLastActionTime)
		{
			// LastActionTime이 수정되면, 여기에 걸릴 수 있으나, 이제 수정될 일이 없음.
			//WriteToConsole("Now[%u] <= LastAction[%u]\n", dwNow, m_dwLastActionTime);
			return true;
		}
		
//		PG_ASSERT_LOG(dwNow > m_dwLastActionTime);

		DWORD dwActionTerm = rkActionEntity.GetActionTerm();
		DWORD dwElapsedTime = (m_dwLastActionTime != 0 ? dwNow - m_dwLastActionTime : dwActionTerm);
//		WriteToConsole("ElapsedTime[%u] = Now[%u] - LastAction[%u] (Action Term : %u)\n", 
//						dwElapsedTime, dwNow, m_dwLastActionTime, dwActionTerm);

		bool bForceToProcessAction = false;
		if(dwElapsedTime < dwActionTerm)
		{
			if(m_dwAccumedOverTime > 0 && m_pkAction && m_pkAction->AlreadySync())
			{
				if(m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) || 
					m_pkAction->GetActionOptionEnable(PgAction::AO_LOOP))
				{
					bForceToProcessAction = true;
				}
			}
			else
			{
				// 아직 액션을 할 때가 아니다.
				//char szBuff[1024];
				//if(rkActionEntity.GetAction())
				//{
				//	sprintf(szBuff, ", NextAction : %s\n", rkActionEntity.GetAction()->GetID().c_str());
				//}
				//else
				//{
				//	sprintf(szBuff, ", NextDirection : %u\n", rkActionEntity.GetDirection());
				//}
				//WriteToConsole("[Wait For Current Action Term] Current Action : %s%s", m_pkAction->GetID().c_str(), szBuff);
				return true;
			}
		}

		DWORD dwSyncTime = 0;
		DWORD dwOverTime = dwElapsedTime - dwActionTerm;

		if(dwElapsedTime<dwActionTerm)
		{
			dwOverTime = 0;
		}
		//WriteToConsole("[ProcessNextAction] Action's OverTime : %u (%u - %u)\t", dwOverTime, dwElapsedTime, dwActionTerm);

		PgAction *pkAction = rkActionEntity.GetAction();
		if(pkAction)
		{
			if(bForceToProcessAction)
			{
				DWORD dwRemainTime = dwActionTerm - dwElapsedTime;
				DWORD dwNewActionTerm = 0;
				PG_ASSERT_LOG(dwRemainTime);
				
				//WriteToConsole("[Reward Synctime 2.1] RemainTime[%u] = ActionTerm[%u] - ElapsedTime[%u]\n", dwRemainTime, dwActionTerm, dwElapsedTime);
				if(m_dwAccumedOverTime <= dwRemainTime)
				{
					pkAction->SetActionTerm(dwRemainTime - m_dwAccumedOverTime);
					m_dwAccumedOverTime = 0;
					//WriteToConsole("[Reward Synctime 2.1.2] SetActionTerm[%u]\n", pkAction->GetActionTerm());
					return true;
				}
				else
				{
					m_dwAccumedOverTime -= dwRemainTime;
					pkAction->SetActionTerm(0);
					dwOverTime = 0;
					//WriteToConsole("[Reward Synctime 2.1.3] Remaind AccummedTime : %u\n", m_dwAccumedOverTime);
				}
			}	

			if(!BeginSync(pkAction, dwOverTime))
			{
				//WriteToConsole("[BeginSync] LastAction Time [%u] : return false\n", m_dwLastActionTime);
				PG_ASSERT_LOG(m_bSync);
				return true;
			}
			
			ProcessAction(pkAction,IsMyActor());
			bReturn = false;
			m_dwLastActionTime = dwNow;
			//WriteToConsole("[ProcessAction] Action : %s, LastActionTime : %u\n", pkAction->GetID().c_str(), m_dwLastActionTime);
		}
		else
		{
			SetDirection(rkActionEntity.GetDirection());
			SetPosition(rkActionEntity.GetDirectionStartPos());
			//WriteToConsole("[ProcessDirection] Direction : %u\n", rkActionEntity.GetDirection());
		}

		// 매 틱당 OverTime이 발생하는데, 이것을 합산해서 Now에서 빼주어야 한다.
		m_dwAccumedOverTime += dwOverTime;
		itr = m_kActionQueue.erase(itr);
	}

	return bReturn;
}


	//if(dwActionTerm == 0 || m_dwLastActionTime == 0)
	//{
	//	m_dwLastActionTime = dwNow;
	//	return;
	//}

	//PG_ASSERT_LOG(dwNow >= m_dwLastActionTime);
	//if(dwNow < m_dwLastActionTime)
	//{
	//	// 수정된 LastActionTime이 더 최근 시간이면 다음 프레임에 처리.
	//	return;
	//}

	//DWORD dwElapsedTime = dwNow - m_dwLastActionTime;
	//if(dwElapsedTime == 0 || dwElapsedTime < dwActionTerm)
	//{
	//	// Action Term만큼 시간이 경과되지 않았다면, 다음에 업데이트 한다.
	//	NILOG(PGLOG_NETWORK, "Elapsed Time : %u, Action Term : %u\n", dwElapsedTime, dwActionTerm);
	//	return;
	//}

	//PG_ASSERT_LOG(dwElapsedTime >= dwActionTerm);
	//DWORD dwOverTime = dwElapsedTime - dwActionTerm;

	//PgAction *pkAction = rkActionEntity.GetAction();
	//if(pkAction)
	//{
	//	WriteToConsole("Action : %s, OverTime : %u\n", pkAction->GetID().c_str(), dwOverTime);
	//	NILOG(PGLOG_NETWORK, "Action : %s, OverTime : %u\n", pkAction->GetID().c_str(), dwOverTime);

	//	// 액션의 시작 위치를 보간한다.
	//	if(!BeginSync(pkAction, dwNow))
	//	{
	//		PG_ASSERT_LOG(m_bSync);
	//		
	//		// Sync중이므로, 더이상 ActionQueue에서는 아무것도 처리하지 않는다.
	//		// 더이상 싱크를 하지 않는다.
	//		pkAction->AlreadySync(true);
	//		return;
	//	}

	//	// Action을 하면서 이전에 바뀌었던 방향이 복구 된다.
	//	if(DoAction(pkAction, false))
	//	{
	//		NILOG(PGLOG_LOG,"Actor : %s %s Process Action (%s, %d, %d, %d, %u) In Queue\n", MB(GetPilot()->GetName()),
	//			MB(GetGuid().str()), pkAction->GetID().c_str(), pkAction->GetActionNo(), pkAction->GetActionInstanceID(),
	//			pkAction->GetTargetList()->size(), pkAction->GetBirthTime());
	//	}
	//	else
	//	{
	//		// Action에 실패하면, 어떻게 해야 하는가??
	//		NILOG(PGLOG_LOG,"Action Failed.\n");
	//		g_kActionPool.ReleaseAction(pkAction);
	//	}

	//	DWORD dwSyncTerm = m_dwEndSyncTime - m_dwBeginSyncTime;
	//	PG_ASSERT_LOG(dwSyncTerm >= 0);
	//	m_dwLastActionTime += (dwActionTerm + dwSyncTerm);
	//	
	//	PG_ASSERT_LOG(m_dwLastActionTime > dwOverTime);
	//	m_dwLastActionTime -= dwOverTime;
	//	m_dwAccumedOverTime += dwSyncTerm;

	//	WriteToConsole("SyncTerm : %u \tOverTime : %u\tAccumedOverTime : %u\t LastActionTime : %u\n", dwSyncTerm, dwOverTime, m_dwAccumedOverTime, m_dwLastActionTime);
	//	NILOG(PGLOG_NETWORK, "SyncTerm : %u \tOverTime : %u\tAccumedOverTime : %u\t LastActionTime : %u\n", dwSyncTerm, dwOverTime, m_dwAccumedOverTime, m_dwLastActionTime);
	//}
	//else
	//{
	//	// Direction 처리.
	//	SetDirection(rkActionEntity.GetDirection());
	//	m_dwLastActionTime += dwActionTerm;
	//	m_dwLastActionTime -= dwOverTime;

	//	PG_ASSERT_LOG(m_dwLastActionTime > dwOverTime);
	//	if(m_dwLastActionTime <= dwOverTime)
	//	{
	//		int i = 1;
	//	}
	//	WriteToConsole("[Monitored Dir] OverTime : %u\tAccumedOverTime : %u\t LastActionTime : %u\n", dwOverTime, m_dwAccumedOverTime, m_dwLastActionTime);
	//	NILOG(PGLOG_NETWORK, "[Monitored Dir] OverTime : %u\tAccumedOverTime : %u\t LastActionTime : %u\n", dwOverTime, m_dwAccumedOverTime, m_dwLastActionTime);
	//}

	//m_kActionQueue.erase(itr);
//}

//void PgActor::ProcessSyncPosition(float fFrameTime, float fSyncRate)
//{
//	PG_STAT(PgStatTimerF timerA((FrameStat*)&g_kActorStatGroup.GetStatInfo("PgActor.ProcessSyncPosition"), g_pkApp->GetFrameCount()));
//	float fMovingSpeed = GetPilot()->GetAbil(AT_C_MOVESPEED) * fSyncRate;
//	if(fMovingSpeed <= g_iSyncPosSpeed)
//		fMovingSpeed = g_iSyncPosSpeed;
//
//	NiPoint3 kStartPos = m_kSyncPositionStart;
//	NiPoint3 kCurPos = GetPos();
//	NiPoint3 kSyncTargetPos = m_kSyncPositionTarget;
//	NiPoint3 kMovVector = (kSyncTargetPos-kStartPos);
//	kMovVector.Unitize();
//	kMovVector = kMovVector * (fMovingSpeed * fFrameTime);
//	
//	NiPoint3 kNextPos = kCurPos + kMovVector;
//	
//	//	시작위치에서 다음 좌표까지의 거리가, 시작위치에서 목표좌표까지의 거리보다 크다면, 지나친것이다.
//	float fDistance1 = (kStartPos-kSyncTargetPos).Length();
//	float fDistance2 = (kStartPos-kNextPos).Length();
//	
//	if (fDistance1 < fDistance2 || (kMovVector == NiPoint3::ZERO))
//	{
//		SetPosition(kSyncTargetPos);
//		m_bSyncPositionProcessing = false;
//		ProcessActionQueue();
//		return;
//	}
//	
//	SetPosition(kNextPos);
//}

//void PgActor::StartSyncPosition()
//{
//	PG_STAT(PgStatTimerF timerA((FrameStat*)&g_kActorStatGroup.GetStatInfo("PgActor.StartSyncPosition"), g_pkApp->GetFrameCount()));
//	m_bSyncPositionProcessing = true;
//
//	if(m_bFloor)
//	{
//		if((m_kSyncPositionStart - m_kSyncPositionTarget).Length() > 10)
//		{
//			NiPoint3 kMovingDir = (m_kSyncPositionStart - m_kSyncPositionTarget);
//			kMovingDir.Unitize();
//
//			//FindPathNormal();
//			//NiPoint3 p3LeftOrRight = GetPathNormal().Cross(kMovingDir);
//			//if(p3LeftOrRight.z<0)
//			//	ToLeft(true,true);
//			//else
//			//	ToLeft(false,true);
//		}
//
//		//	뛰기 모션으로 바꾸자.
//		if(SetTargetAnimation(std::string("run")) == false)
//			SetTargetAnimation(std::string("walk"));
//
//	}
//}

//! 이 캐릭터를 카메라 포거스 캐릭터로 만든다.
void	PgActor::SetCameraFocus()
{
	if(!GetWorld()) return;

	if(GetWorld()->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW)
	{
		PgCameraModeFollow	*pCM = (PgCameraModeFollow*)GetWorld()->m_kCameraMan.GetCameraMode();
		pCM->SetActor(this);
		//_PgOutputDebugString("Set Cam Actor To :%s\n",GetID().c_str());
	}

}
//! 대미지 숫자를 캐릭터 머리 위에 띄워준다.
void	PgActor::ShowDamageNum(NiPoint3 vAttackerPos,NiPoint3 kTargetPos,int iDamage,bool const bClampScreen, bool const bCritical, BYTE const btColor, int const iEnchantLevel, int const iExceptAbil)
{
	if(!GetWorld()|| !GetWorld()->m_pkDamageNumMan) { return; }

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ShowDamageNum"), g_pkApp->GetFrameCount()));

	bool bIsRedColor = true;
	if(0 > iDamage)
	{
		iDamage = iDamage * -1;
		bIsRedColor = false;
	}

	if(GetPilot() && GetPilot()->GetUnit() && GetPilot()->GetUnit()->IsUnitType(UT_PLAYER)) { bIsRedColor = false; }

	
	if(IsMyActor() || bClampScreen)
	{
		GetWorld()->m_pkDamageNumMan->AddNewNum(iDamage,kTargetPos, bIsRedColor,bClampScreen, bCritical, iEnchantLevel, iExceptAbil);
	}
	else
	{
		GetWorld()->m_pkDamageNumMan->AddNewSmallNum(iDamage,kTargetPos, bClampScreen, bCritical, btColor, iEnchantLevel, iExceptAbil);
	}
}

//! 경험치 숫자를 캐릭터 머리 위에 띄워준다.
void	PgActor::ShowExpNum(int const iExp)
{
	if(!GetWorld() || !GetWorld()->m_pkDamageNumMan)
	{
		return; 
	}

	//경험치가 0보다 작을 경우 출력하지 않는다.
	//경험치가 0일 땐 Exp X를 출력해야하므로 >= 를 하면 안된다.
	if(0 > iExp)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ShowExpNum"), g_pkApp->GetFrameCount()));

	NiPoint3 kExpNumPos = GetPos();

	if(GetActorManager() && GetActorManager()->GetNIFRoot())
	{
		//NiAVObject* pkDummy = GetActorManager()->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
		NiAVObject* pkDummy = GetNodePointStar();
		if(pkDummy)
		{
			kExpNumPos = pkDummy->GetWorldTranslate();
			kExpNumPos.z+=25;
		}
	}

	GetWorld()->m_pkDamageNumMan->AddNewExpNum(iExp,kExpNumPos);
}

//! 자동으로 회복되는 HP/MP 회복량을 머리 위에 띄워 준다.
void	PgActor::ShowHpMpNum(int iValue, bool bHp)
{
	if(!GetWorld() || !GetWorld()->m_pkDamageNumMan)
	{
		return;
	}

	//회복량이 0이면 보여주지 않는다.
	if(0 == iValue)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ShowHpMpNum"), g_pkApp->GetFrameCount()));

	NiPoint3	kExpNumPos = GetPos();

	if(GetActorManager() && GetActorManager()->GetNIFRoot())
	{
		//NiAVObject	*pkDummy = GetActorManager()->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
		NiAVObject	*pkDummy = GetNodePointStar();
		if(pkDummy)
		{
			kExpNumPos = pkDummy->GetWorldTranslate();
			if(bHp)
			{
				kExpNumPos.z+=20;
			}
			else
			{
				kExpNumPos.z+=10;
			}
		}
	}

	GetWorld()->m_pkDamageNumMan->AddNewHpMp(iValue,kExpNumPos, bHp);
}

void PgActor::ShowSkillText(NiPoint3 kTargetPos,int iTextType,bool bUp)
{
	if(!GetWorld() || !GetWorld()->m_pkDamageNumMan) { return; }

	GetWorld()->m_pkDamageNumMan->AddNewSkillText(iTextType,bUp,kTargetPos);
}
//!	간단 메세지를 띄운다.
void PgActor::ShowSimpleText(NiPoint3 kTargetPos,int iTextType)
{
	if(!GetWorld() || !GetWorld()->m_pkDamageNumMan) { return; }

	GetWorld()->m_pkDamageNumMan->AddNewSimpleText(iTextType,kTargetPos);
}

//! 캐릭터를 민다.
void	PgActor::PushActor(bool bLeft,float fDistance,float fVelocity,float fAccel)
{
	m_Push.m_bActivated = true;
	m_Push.m_bLeft = bLeft;
	m_Push.m_fDistance = fDistance;
	m_Push.m_fVelocity = fVelocity;
	m_Push.m_fAccel = fAccel;

	//if(!m_pkPhysXActor)
	//{
	//	return ;	//	leesg213 2006.12.06
	//}

	//_PgOutputDebugString("Moving Delta Push: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
}

void	PgActor::PushActor(NiPoint3 const &rkDir,float fDistance,float fVelocity,float fAccel)
{
	m_Push.m_kDir = rkDir;
	m_Push.m_kDir.Unitize();
	PushActor(false, fDistance, fVelocity, fAccel);
}

//! 장비하고 있는 무기의 타입번호를 리턴한다.
int	PgActor::GetEquippedWeaponType()
{
	return m_uiMyWeaponType;
}
PgItemEx* PgActor::GetEquippedWeapon() const
{
	PartsAttachInfo::const_iterator itr = m_kPartsAttachInfo.find(EQUIP_LIMIT_WEAPON);	
	if(itr != m_kPartsAttachInfo.end())
	{
		PgItemEx *pkEquipItem = itr->second;
		return pkEquipItem;
	}
	return	NULL;
}
char* PgActor::GetEquippedWeaponProjectileID()
{
	PartsAttachInfo::const_iterator itr = m_kPartsAttachInfo.find(EQUIP_LIMIT_WEAPON);	
	if(itr != m_kPartsAttachInfo.end())
	{
		PgItemEx *pkEquipItem = itr->second;
		if(!pkEquipItem)
		{
			return "";
		}

		return	(char*)pkEquipItem->GetProjectileID().c_str();
	}
	return	"";
}

//bool PgActor::UpdateCharacterExtents(float fRadius, float fHeight)
//{
//	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateCharacterExtents"), g_pkApp->GetFrameCount()));
//	NxExtendedVec3 kPos = m_pkController->getFilteredPosition();
//
//	NxCapsule kCapsule;
//	
//	kCapsule.p0.x = kCapsule.p1.x = (NxReal)kPos.x;
//	kCapsule.p0.y = kCapsule.p1.y = (NxReal)kPos.y;
//	kCapsule.p0.z = kCapsule.p1.z = (NxReal)kPos.z;
//	kCapsule.p0.y -= fHeight * 0.5f;
//	kCapsule.p1.y += fHeight * 0.5f;
//	
//	kCapsule.radius = fRadius;
//
//	m_pkController->setCollision(false);
//	bool bStatus = m_pkPhysXScene->GetPhysXScene()->checkOverlapCapsule(kCapsule);
//	if(bStatus)
//	{
//		PgOutputPrint("Can not resize capsule!\n");
//		return false;
//	}
//	m_pkController->setCollision(true);
//
//	// 캐릭터의 정확한 위치를 정해준다.
//	m_pkController->setPosition(kPos);
//
//#ifdef PG_USE_CAPSULE_CONTROLLER
//	m_pkController->setHeight(fHeight);
//	m_pkController->setRadius(fRadius);
//#endif
//
//	return true;
//}

int	PgActor::GetABVShapeIndex(char const* strShapeName)
{
	if(strShapeName == NULL || strlen(strShapeName)==0) return 0;

	for(int i=0;i<PG_MAX_NB_ABV_SHAPES; ++i)
	{
		if(strcmp(strShapeName, GetABVShape(i)->m_kTo)==0) return i;
	}

	return	0;
}

NiPoint3	PgActor::GetABVShapeWorldPos(int iIndex)
{
	PG_ASSERT_LOG(iIndex >= 0 && iIndex < PG_MAX_NB_ABV_SHAPES);
	if (iIndex >= 0 && iIndex < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[iIndex])
	{
		NxVec3	vWorldPos;
		vWorldPos = m_apkPhysXCollisionActors[iIndex]->getGlobalPosition();
		return	NiPoint3(vWorldPos.x,vWorldPos.y,vWorldPos.z);
	}
	/*
	ABVShape	*pShape = GetABVShape(iIndex);
	if(!pShape) return	GetWorldTranslate();

	NiAVObjectPtr	spNode = (NiAVObject*)GetNIFRoot()->GetObjectByName(pShape->m_kTo);
	if(spNode)
	{
		NiTransform	kTransform = spNode->GetWorldTransform();

		NiTransform	kTF2;

		kTF2.m_Translate = NiPoint3(pShape->m_kMat.t.x,pShape->m_kMat.t.y,pShape->m_kMat.t.z);
		kTF2.m_fScale = 1.0f;
		kTF2.m_Rotate = pShape->m_kMat.

		return	spNode->GetWorldTranslate()+NiPoint3(pShape->m_kMat.t.x,pShape->m_kMat.t.y,pShape->m_kMat.t.z);
	}*/
	return	GetWorldTranslate();
}

void PgActor::AddToDefaultItem(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const* pEnchant)
{
	if(kItemPos % 2 != 0 && kItemPos != 1)
	{
		return;
	}

	SetDefaultItem(kItemPos, iItemNo, pEnchant);
}

bool PgActor::DelDefaultItem(eEquipLimit kItemPos)
{
	// 기존에 있던 같은 위치의 default item을 지운다.
	DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(kItemPos));
	if(itr != m_kDefaultItem.end())
	{
		m_kDefaultItem.erase(itr);
		return true;
	}
	return false;
}

void PgActor::SetDefaultItem(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const* pEnchant)
{
	DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(kItemPos));
	if(itr == m_kDefaultItem.end())
	{
		m_kDefaultItem.push_back(ItemDesc(kItemPos, iItemNo, (pEnchant == NULL)?(SEnchantInfo()):(*pEnchant)));
		return;
	}

	itr->m_iItemNo = iItemNo;
	itr->m_kEnchantInfo = (pEnchant == NULL)?(SEnchantInfo()):(*pEnchant);
}

int PgActor::GetDefaultItem(eEquipLimit kItemPos)
{
	DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(kItemPos));
	if(itr == m_kDefaultItem.end())
	{
		return 0;
	}

	return itr->m_iItemNo;
}

bool PgActor::GetDefaultItemEnchantInfo(eEquipLimit kItemPos, SEnchantInfo& kInfo)
{
	DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(kItemPos));
	if(itr == m_kDefaultItem.end())
	{
		return false;
	}

	kInfo = itr->m_kEnchantInfo;
	return true;
}

bool PgActor::SetItemColor(eEquipLimit kItemPos, int iItemNo, bool bDefaultItem)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if(!pkItemDef)
	{
		NILOG(PGLOG_ERROR, "[PgActor] SetItemColor, Can't Find %d Item(%d) from defmgr\n", iItemNo, kItemPos);
		return false;
	}

	// 색깔을 바꾸려는 파츠가 Attach되어 있지 않으면 실패
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kItemPos);
	if(itr == m_kPartsAttachInfo.end())
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetItemColor"), g_pkApp->GetFrameCount()));

	// Item Color와 Brightness를 가지고 온다.
	int iItemColor = pkItemDef->GetAbil(AT_HAIR_COLOR);
	float fRed = ((iItemColor & 0x00ff0000) >> 16) / 255.0f;
	float fGreen = ((iItemColor & 0x0000ff00) >> 8) / 255.0f;
	float fBlue = (iItemColor & 0x000000ff) / 255.0f;
	int iBrightness = pkItemDef->GetAbil(AT_HAIRBRIGHTNESS);

	// 아이템 색깔을 변경한다.
	PgItemEx *pkItem = itr->second;
	NiColor kItemColor(fRed, fGreen, fBlue);
	pkItem->SetItemColor(kItemColor, iBrightness);

	if(kItemPos == EQUIP_LIMIT_HAIR && bDefaultItem)
	{
		// Default Item으로 설정한다.
		SetDefaultItem(EQUIP_LIMIT_HAIR_COLOR, iItemNo);
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(GetPilot()->GetUnit());
		if( pkPlayer )
		{
			pkPlayer->SetDefaultItem(EQUIP_POS_HAIR_COLOR, iItemNo);
		}
	}

	return true;
}

#ifndef PG_USE_WORKER_THREAD
bool PgActor::EquipItemProc(int iItemNo, bool bSetToDefaultItem, PgItemEx* pkEquipItem, PgItemEx::ItemLoadType eLoadType)
{
	return true;
}

bool PgActor::EquipItem(PgItemEx *pkEquipItem, int iItemNo, bool bSetToDefault)
{
	return true;
}
#endif

int PgActor::GetAdjustedItemNo( CUnit* pkUnit, EEquipPos const kEquipPos )
{
	if( pkUnit )
	{
		if( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE)) )
		{
			int const iTeamNo = g_kBattleSquareMng.GetTeam(pkUnit->GetID());
			if( TEAM_NONE != iTeamNo )
			{
				switch( kEquipPos )
				{
				case EQUIP_POS_HAIR:
				case EQUIP_POS_FACE:
				case EQUIP_POS_GLASS:
				case EQUIP_POS_WEAPON:
				case EQUIP_POS_SHEILD:
				case EQUIP_POS_KICKBALL:
				case EQUIP_POS_NECKLACE:
				case EQUIP_POS_EARRING:
				case EQUIP_POS_RING_L:
				case EQUIP_POS_RING_R:
					{
					}break;
				default:
					{
						// 리턴 -1이면 아이템을 장착하면 안된다.
						return lua_tinker::call<int,int,int>("GetTeamItemNo", iTeamNo, static_cast<int>(kEquipPos) );
					}break;
				}
			}

			return 0;// 리턴 0이면 원래 아이템을 장착
		}

		if ( !m_kEquipItemCont.empty() )
		{
			EQUIP_ITEM_CONT::const_iterator itor = m_kEquipItemCont.find(static_cast<int>(kEquipPos));
			if(itor != m_kEquipItemCont.end())
			{
				return (*itor).second;
			}
			else
			{
				return m_iOtherEquipItemReturnValue; // default = 0
			}
		}
	}

	return 0;// 리턴 0이면 원래 아이템을 장착
}

void PgActor::SetAdjustedItem(EQUIP_ITEM_CONT const& kEquipItemCont,int const iOtherEquipItemReturnValue)
{
	m_kEquipItemCont = kEquipItemCont;
	m_iOtherEquipItemReturnValue = iOtherEquipItemReturnValue;
}

int PgActor::GetCashItemChanger( EEquipPos const kEquipPos )
{
	if ( !m_kCashChangeItem.empty() )
	{
		CONT_APPEARANCE_CHANGE_INFO::const_iterator itor = m_kCashChangeItem.find(static_cast<int>(kEquipPos));
		if(itor != m_kCashChangeItem.end())
		{
			return (*itor).second.iItemNo;
		}
	}
	return 0;
}

void PgActor::AddCashItemChanger(CONT_APPEARANCE_CHANGE_INFO const& rkEquipItemCont)
{
	CONT_APPEARANCE_CHANGE_INFO::const_iterator kItor = rkEquipItemCont.begin();
	while(rkEquipItemCont.end() != kItor)
	{
		std::pair<CONT_APPEARANCE_CHANGE_INFO::iterator, bool> kRet = m_kCashChangeItem.insert( std::make_pair(kItor->first,kItor->second) );
		if(!kRet.second)
		{// 이미 존재한다면 덮어쓴다
			kRet.first->second = kItor->second;
		}
		++kItor;
	}
}

void PgActor::RemoveCashItemChanger(CONT_APPEARANCE_CHANGE_INFO const& rkEquipItemCont)
{
	CONT_APPEARANCE_CHANGE_INFO::const_iterator kItor = rkEquipItemCont.begin();
	while(rkEquipItemCont.end() != kItor)
	{
		CONT_APPEARANCE_CHANGE_INFO::iterator kRemove_Itor = m_kCashChangeItem.find( kItor->first );
		if( kRemove_Itor != m_kCashChangeItem.end() )
		{
			m_kCashChangeItem.erase(kRemove_Itor);
		}
		++kItor;
	}
}

int PgActor::GetNormalItemChanger( EEquipPos const kEquipPos )
{
	if ( !m_kNormalChangeItem.empty() )
	{
		CONT_APPEARANCE_CHANGE_INFO::const_iterator itor = m_kNormalChangeItem.find(static_cast<int>(kEquipPos));
		if(itor != m_kNormalChangeItem.end())
		{
			return (*itor).second.iItemNo;
		}
	}
	return 0;
}

void PgActor::AddNormalItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont)
{
	CONT_APPEARANCE_CHANGE_INFO::const_iterator kItor = kEquipItemCont.begin();
	while(kEquipItemCont.end() != kItor)
	{
		std::pair<CONT_APPEARANCE_CHANGE_INFO::iterator, bool> kRet = m_kNormalChangeItem.insert( std::make_pair(kItor->first,kItor->second) );
		if(!kRet.second)
		{// 이미 존재한다면 덮어쓴다
			kRet.first->second = kItor->second;
		}
		++kItor;
	}
}
void PgActor::RemoveNormalItemChanger(CONT_APPEARANCE_CHANGE_INFO const& kEquipItemCont)
{
	CONT_APPEARANCE_CHANGE_INFO::const_iterator kItor = kEquipItemCont.begin();
	while(kEquipItemCont.end() != kItor)
	{
		CONT_APPEARANCE_CHANGE_INFO::iterator kRemove_Itor = m_kNormalChangeItem.find( kItor->first );
		if( kRemove_Itor != m_kNormalChangeItem.end() )
		{
			m_kNormalChangeItem.erase(kRemove_Itor);
		}
		++kItor;
	}
}

bool PgActor::IsEquipItemList()const
{
	return !m_kEquipItemCont.empty();
}

// LoadType 0 : MyActor 이면 바로 로딩, 아니면 Thread로
// LoadType 1 : 일단 AttachItemConatiner를 사용하여, 하나씩 붙이는 방식으로 처리 각각의 Item은 Thread를 타게 됨.
// LoadType 2 : 무조건 바로 로딩.
bool PgActor::AddEquipItem( int iItemNo, bool bSetToDefaultItem, PgItemEx::ItemLoadType eLoadType ,bool bReal)
{
//! 한개의 함수를 PG_USE_WORKER_THREAD가 선언되면 3개로 쪼개고 각각의 상황에 맞을 때 부르게 된다.
#ifdef PG_USE_WORKER_THREAD
	PgPilot* pkPilot = GetPilot();
	if( !pkPilot )
	{
		return false;
	}
		
	m_kItemEquipInfo.insert( std::make_pair(iItemNo, true) );

	//	동일한 아이템인지 체크.
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( pkItemDef )
	{
		eEquipLimit const equipLimit = static_cast< eEquipLimit >(pkItemDef->GetAbil(AT_EQUIP_LIMIT));

		PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(equipLimit);
		if( m_kPartsAttachInfo.end() != itr )
		{
			PgItemEx *pkEquipItem2 = itr->second;
			CItemDef *pkItemDef2 = pkEquipItem2->GetItemDef();
			if( NULL != pkEquipItem2
			&&	NULL != pkItemDef2 )
			{
				if( pkItemDef2->No() == pkItemDef->No() )
				{
					CUnit* pkUnit = GetPilot()->GetUnit();
					if( pkUnit )
					{
						PgInventory* pkInven = pkUnit->GetInven();
						if( pkInven )
						{
							int	kEquipPos = 0;
							int	iEquipLimit = equipLimit;
							while( 1 < iEquipLimit )
							{
								iEquipLimit = iEquipLimit >> 1;
								++kEquipPos;
							}

							if( kEquipPos >= EQUIP_POS_HAIR_COLOR
							&&	kEquipPos <= EQUIP_POS_FACE ) // 이것들은 Enchant도 없고 Inven에도 없다.
							{
								return true;
							}

							SItemPos const kNormalItemPos(IT_FIT, kEquipPos);
							SItemPos const kCashItemPos(IT_FIT_CASH, kEquipPos);

							PgBase_Item kItem;
							if( pkInven->GetItem(kNormalItemPos, kItem) != S_OK ) 
							{
								if( pkInven->GetItem(kCashItemPos, kItem) != S_OK
								&&	kItem.IsUseTimeOut() == false )
								{
									SEnchantInfo kEnchant;
									if( GetDefaultItemEnchantInfo(static_cast<eEquipLimit>(1 << kEquipPos), kEnchant) )
									{
										kItem.EnchantInfo(kEnchant);
									}
									else
									{
										return true;	// 인벤에 없다면 기본 장착 아이템이겠지?
									}
								}
							}

							if( kItem.EnchantInfo().PlusLv() == pkEquipItem2->GetEnchantLevel() )
							{
								return true;
							}
						}
					}
				}
			}
		}
	}


#ifdef PG_ITEM_SEQUENCIAL_LOAD
	// Ace Thread에 넣었더니 한프레임에 다 처리해버리는 경향이 있어서 구분을 하고 있다.
	if (eLoadType == PgItemEx::LOAD_TYPE_USEQUEUE && m_iEquipCount > 0 && IsMyActor() == false)
	{
		AttachItemInfo kInfo;
		kInfo.iItemNo = iItemNo;
		kInfo.bSetDefaultItem = bSetToDefaultItem;
		kInfo.pItem = NULL;		// NULL로 만들면 Item Loading Thread에 넣게 된다.
		kInfo.iClass = GetPilot()->GetBaseClassID();
		kInfo.iGender = GetPilot()->GetAbil(AT_GENDER);
		m_kAttachItemLock.Lock();
		m_kAttachItemContainer.push_back(kInfo);
		m_kAttachItemLock.Unlock();
		return true;
	}
#endif

	if (IsMyActor() || eLoadType == PgItemEx::LOAD_TYPE_INSTANT)
	{
		IncEquipCount();
		EquipItemProc(iItemNo, bSetToDefaultItem, NULL, eLoadType);
	}
	else
	{
		g_kEquipThread.DoEquipItem(this, this->GetGuid(), iItemNo, bSetToDefaultItem, GetPilot()->GetAbil(AT_GENDER), GetPilot()->GetBaseClassID());
	}
	return true;
}

bool PgActor::EquipItemProc(int iItemNo, bool bSetToDefaultItem, PgItemEx* pkEquipItem, PgItemEx::ItemLoadType eLoadType)
{
#endif // #ifdef PG_USE_WORKER_THREAD
	if(!GetPilot() || !GetPilot()->GetUnit())
	{
		DecEquipCount();
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.EquipItemProc"), g_pkApp->GetFrameCount()));

	if (pkEquipItem == NULL)
	{
		// ItemDefNo로 부터 PgItemEx 생성!
		pkEquipItem = PgItemEx::GetItemFromDef(iItemNo, GetPilot()->GetAbil(AT_GENDER), GetPilot()->GetBaseClassID());
	}
	
	if (pkEquipItem == NULL)
	{
		// 아이템 생성에 실패해도 패스!
		PG_ASSERT_LOG(m_pkPilot);
		if (m_pkPilot)
		{
			NILOG(PGLOG_ERROR, "%s actor's %d item creation failed!\n", MB(m_pkPilot->GetName()), iItemNo);
		}
		// xxxxxx TODO : 0인 아이템을 착용하지 않게끔 하자.
		DecEquipCount();
		return false;
	}

	CUnit* pkUnit = GetPilot()->GetUnit();

	PgInventory *pkInven = pkUnit->GetInven();
	if(pkInven)
	{
		int	kEquipPos = 0;
		int	iEquipLimit = pkEquipItem->EquipLimit();
		while(iEquipLimit>1)
		{
			iEquipLimit=iEquipLimit>>1;
			++kEquipPos;
		}
		SItemPos kItemPos(IT_FIT, kEquipPos);
		PgBase_Item kItem;
		if(S_OK == pkInven->GetItem(kItemPos, kItem) && iItemNo == kItem.ItemNo())
		{
			pkEquipItem->SetItemInfo(kItem);
		}
		else
		{
			kItemPos.x = IT_FIT_CASH;
			if(S_OK == pkInven->GetItem(kItemPos, kItem) && kItem.IsUseTimeOut() == false)
			{
				pkEquipItem->SetItemInfo(kItem);
			}
			else
			{
				SEnchantInfo kEnchant;
				if( GetDefaultItemEnchantInfo(pkEquipItem->EquipLimit(), kEnchant) )
				{
					kItem.EnchantInfo(kEnchant);
					pkEquipItem->SetItemInfo(kItem);
				}
			}
		}
	}

#ifdef PG_USE_WORKER_THREAD
	if (IsMyActor() || eLoadType == PgItemEx::LOAD_TYPE_INSTANT)
	{
		return EquipItem(pkEquipItem, iItemNo, bSetToDefaultItem);
	}

	// 대기큐에 등록해서 다음 Update에 장착되도록 한다.
	AttachItemInfo kInfo;
	kInfo.iItemNo = iItemNo;
	kInfo.bSetDefaultItem = bSetToDefaultItem;
	kInfo.pItem = pkEquipItem;	
	m_kAttachItemLock.Lock();
	m_kAttachItemContainer.push_front(kInfo);
	m_kAttachItemLock.Unlock();

	PgOutputPrint5("[PgActor] EquipItemAdd 0x%0X, no(%d), gend(%d), class(%d), path(%s)\n", pkEquipItem,
		pkEquipItem->GetItemDef()->No(), pkEquipItem->GetUserGender(), pkEquipItem->GetUserClass(), pkEquipItem->GetXMLPath().c_str());
	return true;
}

bool PgActor::EquipItem(PgItemEx *pkEquipItem, int iItemNo, bool bSetToDefaultItem)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.EquipItem"), g_pkApp->GetFrameCount()));
	//PgOutputPrint5("[PgActor] EquipItem(real) 0x%0X, no(%d), gend(%d), class(%d), path(%s)\n", pkEquipItem,
	//	pkEquipItem->GetItemDef()->No(), pkEquipItem->GetUserGender(), pkEquipItem->GetUserClass(), pkEquipItem->GetXMLPath().c_str());

#endif
	if (!pkEquipItem)
	{
		return false;
	}

	//	이미 동일한 아이템을 장착하고 있다면 그냥 Return 하자. leesg213
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(pkEquipItem->EquipLimit());	
	if(itr != m_kPartsAttachInfo.end())
	{
		PgItemEx *pkEquipItem2 = itr->second;
		if(pkEquipItem && pkEquipItem2 && pkEquipItem->IsEqual(pkEquipItem2))
		{
			DecEquipCount();
			THREAD_DELETE_ITEM(pkEquipItem);
			return true;
		}
	}

	bool bAttachSuccess = false;
	const eEquipLimit ikEquipLimit = pkEquipItem->EquipLimit();
	
	PgPilot* pkPilot = GetPilot();
	if(pkPilot)
	{
		NILOG(PGLOG_LOG,"PgActor::EquipItem() -> ActorGUID : %s ActorName : %s ItemNo : %d ikEquipLimit: %d bSetToDefaultItem : %d EquipCount : %d\n", MB(GetPilotGuid().str()),MB(pkPilot->GetName()),iItemNo,ikEquipLimit,bSetToDefaultItem,m_iEquipCount);
	}

	if(pkEquipItem->GetMeshRoot())
	{
		AttachActorAlphaProperty(pkEquipItem->GetMeshRoot());
		pkEquipItem->GetMeshRoot()->UpdateProperties();
	}

	CONT_NI_COLOR kContColor;
	if(GetCoupleItemColor(kContColor))
	{
		pkEquipItem->ApplyCustomColor(kContColor);
	}

	if(ikEquipLimit <= EQUIP_LIMIT_HELMET)
	{
		bAttachSuccess = AttachNoSkinningParts(pkEquipItem);
	}
	else
	{// 아이템 포지션이 20이상이면, Skinning하여 Attach하는 아이템이다.
		bAttachSuccess = (AttachSkinningParts(pkEquipItem) > 0 ? true : false);
		if(GetNIFRoot())
		{// AttachSkinningParts()하면서 초기화된 HeadSize를 저장된 값으로 돌린다
			NiAVObject	*pkHead = GetCharRoot()->GetObjectByName("Bip01 Head");
			if(pkHead)
			{
				pkHead->SetScale(m_fCurrentHeadSize);
			}
		}
	}

	//_PgOutputDebugString("____ItemPos____ : %d\n", ikEquipLimit);
	DecEquipCount();

	if(!bAttachSuccess)
	{
		// 아이템을 못 붙였으면 즉시 종료
		NILOG(PGLOG_ERROR,"PgActor::EquipItem() -> bAttachSuccess Failed ItemNo : %d ikEquipLimit : %d bSetToDefaultItem : %d \n", iItemNo,ikEquipLimit,bSetToDefaultItem);

		return false;
	}

	if(bSetToDefaultItem)
	{
		SEnchantInfo kEnchant = (pkEquipItem == NULL)?(SEnchantInfo()):(pkEquipItem->GetItemInfo().EnchantInfo());

		SetDefaultItem(ikEquipLimit, iItemNo, &kEnchant);
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
		if( pkPlayer )
		{
			int iEquipPos = 0;
			int iTempLimit = ikEquipLimit;
			while( iTempLimit > 1 )
			{
				iTempLimit = iTempLimit >> 1;
				++iEquipPos;
			}
			pkPlayer->SetDefaultItem(iEquipPos, iItemNo);
		}
	}

	// 파츠를 이쁘게 붙였으면, 붙인 파츠 목록에 넣는다.
	m_kPartsAttachInfo.insert(std::make_pair(ikEquipLimit, pkEquipItem));

	// 처음 로딩 중이면 감춘다.
	// 기본 쉐이프를 보여줘야한다.
	//if(!m_bLoadingComplete)
	//{
	//	_PgOutputDebugString("[PgActor::EquipItem] Actor : %s ItemNo : %d Hide Parts because m_bLoadingComplete == fasle\n",MB(GetPilotGuid().str()),iItemNo);
	//	HideParts(ikEquipLimit, true);
	//}

	// 머리를 붙이는데 성공했으면, 염색을 해야 하는지 체크하자.
	if(ikEquipLimit == EQUIP_LIMIT_HAIR)
	{
		DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(EQUIP_LIMIT_HAIR_COLOR));
		if(itr != m_kDefaultItem.end())
		{
			SetItemColor(ikEquipLimit, itr->m_iItemNo, bSetToDefaultItem);
		}
	}
	
	if(pkPilot)
	{
		CUnit* pkUnit = pkPilot->GetUnit();
		if(pkUnit)
		{
			int iClass = pkPilot->GetAbil(AT_CLASS);
			if( IS_CLASS_LIMIT(UCLIMIT_COMMON_DOUBLE_FIGHTER, iClass) 
				|| pkUnit->UnitType() == UT_SUB_PLAYER
				)
			{
				PgActor* pkSubplayerActor = PgActorUtil::GetSubPlayerActor(this);
				if(pkSubplayerActor)
				{
					switch( ikEquipLimit )
					{
					case EQUIP_LIMIT_WEAPON:
						{
							pkSubplayerActor->AddEquipItem(330100005, false, PgItemEx::LOAD_TYPE_INSTANT, false );
						}break;
					case EQUIP_LIMIT_FACE:
					case EQUIP_LIMIT_HAIR:
						{
							pkSubplayerActor->AddEquipItem(iItemNo, false, PgItemEx::LOAD_TYPE_INSTANT, false );
						}break;
						//case EQUIP_LIMIT_HAIR_COLOR:
						//	{// 헤어 색상은 Recv_PT_M_NFY_ITEM_CHANGE(BM::Stream* pkPacket)에 DISCT_SET_DEFAULT_ITEM에서 직접 바뀌어짐
						//		pkSubplayerActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo);
						//	}break;
					}
				}

				if( ikEquipLimit == EQUIP_LIMIT_BOOTS)
				{
					if( m_kPartsAttachInfo.end() != m_kPartsAttachInfo.find(EQUIP_LIMIT_KICKBALL) )
					{
						HideParts_IgnoreHideCnt(EQUIP_LIMIT_BOOTS, true);
					}
					else
					{
						HideParts_IgnoreHideCnt(EQUIP_LIMIT_BOOTS, false);
					}
				}
				else if( ikEquipLimit == EQUIP_LIMIT_KICKBALL)
				{
					if( m_kPartsAttachInfo.end() != m_kPartsAttachInfo.find(EQUIP_LIMIT_BOOTS) )
					{
						HideParts_IgnoreHideCnt(EQUIP_LIMIT_BOOTS, true);
					}
					else
					{
						HideParts_IgnoreHideCnt(EQUIP_LIMIT_BOOTS, false);
					}
				}
			}
		}
	}
	if( EQUIP_LIMIT_FACE == ikEquipLimit )
	{
		SetNodeHide("D_ear", false);
	}
	pkEquipItem->ApplyEnchantEffect();
	pkEquipItem->ApplyStatusEffect(GetPilot());
	pkEquipItem->SetActorNodesHide(this, true); //아이템 착용으로 숨길 노드가 있으면 숨기자
//////////////////////////////////////////////	
// 커플 칼라.


//////////////////////////////////////////////	
	// 투구 모양에 따른 머리 모양을 바꾸어야 하는지 체크.
	{
		PgItemEx *pkHelm = 0;
		PgItemEx *pkHair = 0;

		if(ikEquipLimit == EQUIP_LIMIT_HELMET)
		{
			PartsAttachInfo::iterator hairItr = m_kPartsAttachInfo.find(EQUIP_LIMIT_HAIR);
			if(hairItr != m_kPartsAttachInfo.end())
			{
				pkHair = hairItr->second;
				pkHelm = pkEquipItem;
			}
		}
		else if(ikEquipLimit == EQUIP_LIMIT_HAIR)
		{
			PartsAttachInfo::iterator helmItr = m_kPartsAttachInfo.find(EQUIP_LIMIT_HELMET);
			if(helmItr != m_kPartsAttachInfo.end())
			{
				pkHelm = helmItr->second;
			}
			pkHair = pkEquipItem;
		}

		ApplyHairType(pkHair,pkHelm);

	}

	if(m_bLoadingComplete)
	{
		ApplyHideParts(ikEquipLimit);
	}

	//! 아이템을 착용하였으면, UIModel을 업데이트 해야 하므로 플래그를 TRUE로 설정
	m_bNeedToUpdateUIModel = true;

	// Material Prop가 바꼈으므로 갱신해야 한다.
	NeedToUpdateMaterialProp(true);
	
	return true;
}

bool PgActor::GetCoupleItemColor(CONT_NI_COLOR &kContColor)
{
	kContColor.clear();

	CUnit* pkUnit = GetPilot()->GetUnit();
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
	
	if(pkPlayer)
	{
		BM::GUID const kCoupleColorGuid = pkPlayer->CoupleColorGuid();
		if(kCoupleColorGuid.IsNotNull())
		{//GUID 를  순방향, 역방향을 XOR로 만듬(시퀀셜 GUID 때문에. 순방, 역방을 섞어야함)
			BYTE const *pkColorMem = reinterpret_cast<BYTE const*>(&kCoupleColorGuid);
			
			size_t const max_color = sizeof(BM::GUID)/3;//rgb 라서 3
			kContColor.resize(max_color);

			size_t i = 0;
			while(max_color > i)
			{
				NiColor & kElement = kContColor.at(i);
				kElement.r = static_cast<float>(*(pkColorMem+0))/255.f; 
				kElement.g = static_cast<float>(*(pkColorMem+1))/255.f; 
				kElement.b = static_cast<float>(*(pkColorMem+2))/255.f; 
				pkColorMem+=3;//3바이트 이동 진행.
				++i;
			}
			return true;
		}
	}
	return false;
}

void	PgActor::RefreshCustomItemColor()
{
	CONT_NI_COLOR kContColor;
	if(GetCoupleItemColor(kContColor))
	{
		for(PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin(); itr != m_kPartsAttachInfo.end(); ++itr)
		{
			PgItemEx *pkParts = itr->second;
			if(!pkParts)
			{
				continue;
			}

			pkParts->ApplyCustomColor(kContColor);
		}
	}
}

void	PgActor::ApplyHairType(PgItemEx *pkHair,PgItemEx *pkHelm)
{
	if(!pkHair)
	{
		return;
	}

	std::string kMeshPath = pkHair->GetMeshPath();
	int iPos = kMeshPath.rfind(".nif") - 1;
	if(!pkHelm || pkHelm->GetMeshType() == PgItemEx::HELM_TYPE_NONE)
	{
		kMeshPath[iPos] = '1';
	}
	else
	{
		kMeshPath[iPos] = pkHelm->GetMeshType() + '0';
	}

	kMeshPath = kMeshPath.substr(kMeshPath.rfind("/")+1, iPos - kMeshPath.rfind("/"));

	pkHair->DetachUselessHairNode(pkHair->GetMeshRoot(), kMeshPath.c_str());


	//pkHair->ReloadMeshRoot(pkHair->GetMeshRoot(), kMeshPath.c_str(), false);
	//pkHair->ReloadMeshRoot(pkHair->GetMeshRoot(), 0, false);

	ApplyHideParts(EQUIP_LIMIT_HAIR);
}
void	PgActor::CheckItemUseTime()
{
	if((NiGetCurrentTimeInSec() - m_fLastCheckItemUseTime) < 5)	//	 5초에 한번씩만 체크한다.
	{
		return;
	}

	m_fLastCheckItemUseTime = NiGetCurrentTimeInSec();

	if(!GetPilot())
	{
		return;
	}

	CUnit	*pkUnit = GetPilot()->GetUnit();
	if(!pkUnit)
	{
		return;
	}

	PgPlayer	*pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
	if(!pkPlayer)
		{
			return;
		}

	PgInventory	*pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return;
	}

	CONT_HAVE_ITEM_DATA	kCashItems;
	if(S_OK != pkInven->GetItems(IT_FIT_CASH,kCashItems))
	{
		return;
	}

	bool	bRefreshAbil = false;

	for(CONT_HAVE_ITEM_DATA::iterator itor = kCashItems.begin(); itor != kCashItems.end(); ++itor)
	{
		PgBase_Item	&kCashItem = itor->second;

		int const iItemNo = itor->first;

		SItemPos	kItemPos;
		if(S_OK != pkInven->GetFirstItem(IT_FIT_CASH,iItemNo,kItemPos))
		{
			continue;
		}

		if(kCashItem.IsUseTimeOut())
		{
			if(m_kItemEquipInfo.find(iItemNo) != m_kItemEquipInfo.end())
			{
				UnequipItem(static_cast<EInvType>(kItemPos.x),static_cast<EEquipPos>(kItemPos.y), iItemNo);
				bRefreshAbil = true;
			}
		}
		else
		{
			if(m_kItemEquipInfo.find(iItemNo) == m_kItemEquipInfo.end())
			{
				PgOptionUtil::SClientDWORDOption const kOption(GetPilot()->GetAbil(AT_CLIENT_OPTION_SAVE));
				if( !kOption.IsHideCashInvenPos(static_cast<EEquipPos>(kItemPos.y)) )
				{
					EquipItemByPos(static_cast<EInvType>(kItemPos.x),static_cast<EEquipPos>(kItemPos.y));
					bRefreshAbil = true;
				}
			}
		}
	}

	if(bRefreshAbil)
	{
		pkPlayer->NftChangedAbil(AT_REFRESH_ABIL_INV, E_SENDTYPE_NONE);
		RefreshCharStateUI();
	}

}

bool PgActor::UnequipItem( EInvType kInvType, EEquipPos eEquipPos, int iItemNo, PgItemEx::ItemLoadType eLoadType, bool const bClear, bool bReal )
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UnequipItem"), g_pkApp->GetFrameCount()));

	PgPilot* pkPilot = GetPilot();
	if( !pkPilot )
	{
		return false;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if( !pkUnit )
	{
		return false;
	}
	PgInventory *pkInv = pkUnit->GetInven();
	if( !pkInv )
	{
		return	false;
	}
	
	PgItemEx* pkItemEx = NULL;
	eEquipLimit	const kEquipLimit = static_cast<eEquipLimit>(1 << eEquipPos);
	PartsAttachInfo::iterator attachItr = m_kPartsAttachInfo.find(kEquipLimit);
	if(attachItr != m_kPartsAttachInfo.end())
	{// eEquipPos 위치에 외관 아이템이 있다면 얻어오고
		pkItemEx = attachItr->second;
	}
	
	if(0 >= iItemNo)
	{// 장착 해제 아이템 번호가 오지 않는다면
		if(!pkItemEx)
		{// 해당 장착 위치에 있는
			return false;
		}
		CItemDef* pkItemDef = pkItemEx->GetItemDef();
		if(!pkItemDef)
		{// 아이템 번호를 얻어와 사용하고
			return false;
		}
		iItemNo = pkItemDef->No();
	}
	else if( bReal)
	{// 장착 해제 아이템 번호가 있고, 진짜 장착 해제 하는것이라면
		GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
		int const iSetNo = kItemSetDefMgr.GetItemSetNo(iItemNo);
		if(0 < iSetNo)
		{// 세트 아이템이 장착 해제 되었는가 확인해서
			RemoveCompletedItemSet(iSetNo);		// 세트 아이템 완성히 적용되는 효과를 제거 해주고
			RemoveItemEffect(iItemNo);			// 아이템에 특정시점에만 붙여줄 파티클 정보가 있었다면 제거해주고
		}
	}
	
	if( !bClear )
	{
		int const iClass = pkUnit->GetAbil(AT_CLASS);
		if( IS_CLASS_LIMIT(UCLIMIT_COMMON_DOUBLE_FIGHTER, iClass) 
			|| pkUnit->UnitType() == UT_SUB_PLAYER 
			)
		{
			if( kEquipLimit == EQUIP_LIMIT_KICKBALL
				&& m_kPartsAttachInfo.end() != m_kPartsAttachInfo.find(EQUIP_LIMIT_BOOTS)
				)
			{
				HideParts_IgnoreHideCnt(EQUIP_LIMIT_BOOTS, false);
			}
		}
	}

	if( bReal 
		&& 0 == m_kItemEquipInfo.erase(iItemNo)	//(모든 장착된 외관 아이템의 정보가 들어있음)
		)
	{// 제대로 장착 해제 되는것인지 확인하여  제거된게 없다면 장착 되어있는 아이템이 아니기에 종료해주고
		return false;
	}
	
	int iType = 0;	// 이 아이템이 어떤 외형 아이템인지 체크 하여
	int iApperanceItemNo = GetAppearanceItemNo(eEquipPos, iType);
	
	switch(iType)
	{
	case EESP_CASH_SET_EFFECT_FIT:
		{// 세트완성 외관이
			if( IT_FIT_CASH == kInvType ) // 캐시로 부터 왔다면
			{// 세트완성 외관과, 지금 장착 해제할 캐시 아이템을 제외한 다른 외관을 다시 얻어오고
				iApperanceItemNo = GetAppearanceItemNo(eEquipPos, iType, true, false, true, false);
			}
		}break;
	case EESP_NORMAL_SET_EFFECT_FIT:
		{
			if( IT_FIT == kInvType ) // 일반 아이템으로 부터 왔다면
			{// 세트완성 외관과, 지금 장착 해제한 일반 아이템을 제외한 다른 외관을 다시 얻어오고
				iApperanceItemNo = GetAppearanceItemNo(eEquipPos, iType, true, false, true, true, false);
			}
		}break;
	default:
		{
		}break;
	}
	
	if( (EESP_NONE == iType || 0 == iApperanceItemNo)
		&& (NULL != pkItemEx)
		)
	{// 교체해줄 외관이 아무것도 없다면
		if(DetachParts(pkItemEx)					// 현재 장착 위치의 외관을 제거해주고
			&& EQUIP_LIMIT_HELMET == kEquipLimit	// 그 부분이 헬멧이라면
			)
		{
			PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(EQUIP_LIMIT_HAIR);
			if(itr != m_kPartsAttachInfo.end())
			{// 머리를 붙여줘야 하고
				ApplyHairType(itr->second, NULL);
			}
		}
		ApplyHideParts(kEquipLimit);
		return true;
	}

	if ( 0 < iApperanceItemNo )
	{
		AddEquipItem( iApperanceItemNo, EESP_DEFAULT == iType, eLoadType );
	}
	return true;
}

bool PgActor::IsEquipMedalPos(EEquipPos kItemPos)
{
	return (kItemPos == EQUIP_POS_MEDAL);
}

bool PgActor::EquipItemByPos(EInvType eInvType, EEquipPos eEquipPos, bool bReal)
{
	PgPilot* pkPilot = GetPilot();
	if( NULL == pkPilot )
	{
		return false;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if( NULL == pkUnit)
	{
		return false;
	}
	PgInventory* pkInv =  pkUnit->GetInven();
	if( NULL == pkInv)
	{
		return false;
	}

	if( 0.0f == m_fLoadingStartTime )
	{
		m_fLoadingStartTime = NiGetCurrentTimeInSec();
	}

	if ( EQUIP_POS_HAIR_COLOR == eEquipPos )
	{// 머리색은 PgItemEx를 만들 필요가 없고, Hair아이템을 만들때 참조만 됨
		return true;
	}
	
	bool bCompleteItemSet = false;
	PgBase_Item kItem;
	if( bReal
		&& S_OK == pkInv->GetItem(eInvType, eEquipPos, kItem) 
		)
	{
		int const iSetNo = CheckItemSetComplete( eInvType, kItem.ItemNo(), bCompleteItemSet );
		if( 0 < iSetNo
			&& bCompleteItemSet
			)
		{// 세트아이템 완성시 추가 처리 부분을 진행하고
			AddCompletedItemSet( iSetNo );
		}
		// 단일 아이템에 의한 추가 파티클이 있다면 확인해서 진행하고
		AddItemEffect( kItem.ItemNo() );
	}
	
	// 현재 보여야할 아이템을 찾아오고,
	int iType = 0;	// 이 아이템이 어떤 외형 아이템인지 체크 하여
	int const iApperanceItemNo = GetAppearanceItemNo(eEquipPos, iType);
		
	if( 0 < iApperanceItemNo )
	{// 해당 아이템을 장착 할 수 있게 한다.
		NILOG(PGLOG_LOG, "[PgActor] EquipItemByPos(%d, %d, %d)\n", eEquipPos, iApperanceItemNo,  EESP_DEFAULT == iType);
		return AddEquipItem( iApperanceItemNo, EESP_DEFAULT == iType, PgItemEx::LOAD_TYPE_USEQUEUE , bReal);
	}

	return false;
}

int PgActor::CheckItemSetComplete(EInvType eInvType, int const iEquipItemNo, bool& rbOutIsComplete)
{// eInvType에 iEquipItemNo이 장착될때 세트아이템이 완성되는것이 있는가
	rbOutIsComplete = false;

	PgPilot* pkPilot = GetPilot();
	if( NULL == pkPilot )
	{
		return 0;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if( NULL == pkUnit)
	{
		return 0;
	}
	PgInventory* pkInv =  pkUnit->GetInven();
	if( NULL == pkInv)
	{
		return 0;
	}
	GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
	int const iSetNo = kItemSetDefMgr.GetItemSetNo(iEquipItemNo);
	CItemSetDef const *pkSetDef = kItemSetDefMgr.GetDef(iSetNo);
	if(pkSetDef)
	{// 세트아이템 완성도를 확인해서 파티클을 붙여주고
		CONT_HAVE_ITEM_DATA kContHaveItems;
		pkInv->GetItems(eInvType, kContHaveItems);
		int const iPieceSet = pkSetDef->CheckNeedItem(kContHaveItems, pkUnit, rbOutIsComplete);
	}
	return iSetNo;
}

void PgActor::AddItemEffect(int const iItemNo)
{// 단일 아이템의 추가 이펙트를 등록한다
	SPOTParticleInfo kTemp;
	if(g_kItemMan.GetItemPOTParticleInfo( iItemNo, kTemp))
	{// // 단일 아이템에, 특정시점에만 붙여줄 파티클 정보가 존재한다면, 해당 관리 객체에 넣어 주고
		m_kPOTParticle.AddInfo(kTemp);
	}
}

void PgActor::RemoveItemEffect(int const iItemNo)
{// 단일 아이템의 추가 이펙트를 제거한다
	SPOTParticleInfo kTemp;
	if(g_kItemMan.GetItemPOTParticleInfo(iItemNo, kTemp))
	{// // 아이템에, 특정시점에만 붙여줄 파티클 정보가 있는지 확인하고 떼어준다
		m_kPOTParticle.RemoveInfo(static_cast<PgPOTParticle::eAttachPointOfTime>(kTemp.iAttachPointOfTime));
	}
}

int PgActor::GetAppearanceItemNo(EEquipPos const eEquipPos, int& riType, bool const bCheckEffect, bool const bCheckCashItemSet, bool const bCheckNormalItemSet, bool const bCheckCash, bool const bCheckNormal)
{
	PgPilot* pkPilot = GetPilot();
	if( NULL == pkPilot )
	{
		return 0;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if( NULL == pkUnit)
	{
		return 0;
	}
	PgInventory* pkInv =  pkUnit->GetInven();
	if( NULL == pkInv)
	{
		return 0;
	}
	// 보여야 하는 우선순위 : Effect외관 > Cash세트 추가 아이템 외관 > 일반세트 추가 아이템 외관 > 캐시아이템 외관 > 일반아이템 외관 > 디폴트아이템 외관
	
	riType = EESP_NONE;
	if(bCheckEffect)
	{// Effect외관
		int const iItemNo = GetAdjustedItemNo( pkUnit, eEquipPos );
		if(0 < iItemNo)
		{//Effect로 인한 외관 아이템 변화가 있는가 확인해서 있으면 보여야하고
			riType = EESP_EFFECT_FIT;
			return iItemNo;
		}
	}
	
	PgOptionUtil::SClientDWORDOption const kOption(pkPilot->GetAbil(AT_CLIENT_OPTION_SAVE));
	PgBase_Item kCashInvItem;

	bool const bShowAbleCashItem = 
		( S_OK == pkInv->GetItem( IT_FIT_CASH, eEquipPos, kCashInvItem )	// 캐시아이템이 존재하고
		&&	!kCashInvItem.IsUseTimeOut()								// 아이템 제한시간이 지나지 않았고
		&&	!kOption.IsHideCashInvenPos(eEquipPos)				// 보이기 옵션에 감추어져있지 않다면
		);														// 캐시아이템은 보이는 아이템이다.
	
	if(bCheckCashItemSet
		&& bShowAbleCashItem
		)
	{// 캐시 세트아이템으로 인한 아이템 교환 부분
		int const iItemNo = GetCashItemChanger( eEquipPos );
		if( 0 < iItemNo )
		{
			riType = EESP_CASH_SET_EFFECT_FIT;
			return iItemNo;
		}
	}	
	if(bCheckCash
		&& bShowAbleCashItem
		)
	{// 캐시 아이템 외관
		riType = EESP_CASH_FIT;
		return kCashInvItem.ItemNo();
	}

	PgBase_Item kNormalInvItem;
	bool const bShowAbleNormalItem =
		( S_OK == pkInv->GetItem( IT_FIT, eEquipPos, kNormalInvItem )			// 일반아이템이 존재하고
		&& !kOption.IsHideEquipInvenPos( eEquipPos )				// 보이기 옵션에 감추어져있지 않다면
		);															// 일반아이템은 보이는 아이템이다

	if(bCheckNormalItemSet
		&& bShowAbleNormalItem
		)
	{// 노멀 세트아이템으로 인한 아이템 교환 부분
		int const iItemNo = GetNormalItemChanger( eEquipPos );
		if(0 < iItemNo)
		{
			riType = EESP_NORMAL_SET_EFFECT_FIT;
			return iItemNo;
		}
	}
	if(bCheckNormal
		&& (bShowAbleNormalItem && EQUIP_POS_MEDAL != eEquipPos)	// 노멀아이템 중에 메달 위치는 아무것도 표시 하지 않기에 외관 아이템번호로 반환하면 안되고
		)
	{// 일반 아이템 외관
		riType = EESP_NORMAL_FIT;
		return kNormalInvItem.ItemNo();
	}
	
	ItemDesc kItemDec( static_cast<eEquipLimit>(1 << eEquipPos) );
	DefaultItemContainer::iterator itr = std::find( m_kDefaultItem.begin(), m_kDefaultItem.end(), kItemDec );
	if(m_kDefaultItem.end() != itr )
	{// 장착부위에 디폴트 아이템이 있으면
		riType = EESP_DEFAULT;
		return itr->m_iItemNo;
	}
	
	return 0;
}

void PgActor::HideEquipItem(int iEquipPos, bool bHide)
{
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find((eEquipLimit)(1 << iEquipPos));
	if(itr == m_kPartsAttachInfo.end())
	{
		return;
	}

	itr->second->Hide(bHide);
	if(iEquipPos == EQUIP_POS_HELMET)
	{
		PgItemEx *pkHelm = itr->second;
		itr = m_kPartsAttachInfo.find(EQUIP_LIMIT_HAIR);
		if(itr == m_kPartsAttachInfo.end())
		{
			return;
		}

		PgItemEx *pkHair = itr->second;
		if(bHide)
		{
			pkHelm = 0;
		}
		ApplyHairType(pkHair,pkHelm);
	}
}

void PgActor::EquipAllItem()
{	
	// xxxxxxxxxxxx 모핑 때문에 얼굴을 먼저 붙여야 하므로

	//	캐쉬 아이템 먼저 장착

	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_FACE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HAIR_COLOR);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HAIR);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHOULDER);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_CLOAK);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_GLASS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_WEAPON);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHEILD);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_NECKLACE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_EARRING);
	//EquipItemByPos(IT_FIT_CASH,EQUIP_POS_EARRING_R);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_RING);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_RING_R);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_BELT);
	//EquipItemByPos(IT_FIT_CASH,EQUIP_POS_ATTSTONE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_MEDAL);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HELMET);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHIRTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_PANTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_BOOTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_GLOVE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_KICKBALL);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_ARM);

	//	일반 아이템 장착
	EquipItemByPos(IT_FIT,EQUIP_POS_FACE);
	EquipItemByPos(IT_FIT,EQUIP_POS_HAIR_COLOR);
	EquipItemByPos(IT_FIT,EQUIP_POS_HAIR);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHOULDER);
	EquipItemByPos(IT_FIT,EQUIP_POS_CLOAK);
	EquipItemByPos(IT_FIT,EQUIP_POS_GLASS);
	EquipItemByPos(IT_FIT,EQUIP_POS_WEAPON);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHEILD);
	EquipItemByPos(IT_FIT,EQUIP_POS_NECKLACE);
	EquipItemByPos(IT_FIT,EQUIP_POS_EARRING);
	//EquipItemByPos(IT_FIT,EQUIP_POS_EARRING_R);
	EquipItemByPos(IT_FIT,EQUIP_POS_RING);
	EquipItemByPos(IT_FIT,EQUIP_POS_RING_R);
	EquipItemByPos(IT_FIT,EQUIP_POS_BELT);
	//EquipItemByPos(IT_FIT,EQUIP_POS_ATTSTONE);
	EquipItemByPos(IT_FIT,EQUIP_POS_MEDAL);
	EquipItemByPos(IT_FIT,EQUIP_POS_HELMET);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHIRTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_PANTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_BOOTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_GLOVE);
	EquipItemByPos(IT_FIT,EQUIP_POS_KICKBALL);
	EquipItemByPos(IT_FIT,EQUIP_POS_ARM);
	
	//int iItemPos = 0;
	//while(++iItemPos < (EQUIP_POS_MAX+1))
	//{
	//	if(iItemPos == EQUIP_POS_FACE)
	//	{
	//		continue;
	//	}

	//	EquipItemByPos(static_cast<EEquipPos>(iItemPos));
	//}
}

int PgActor::GetAttachSlotNo()
{
	++m_iAttachSlotNo;
	if(m_iAttachSlotNo > EAPS_GET_END)
	{
		m_iAttachSlotNo = EAPS_GET_START;
	}
	return m_iAttachSlotNo;
}

bool PgActor::AttachTo(int iSlot, char const *pcTargetName, NiAVObject *pkObject)
{
	PgParticle	*pkParticle = NiDynamicCast(PgParticle,pkObject);
	if(!pkParticle)
	{
		return false;
	}

	PgActor* pkActor = this; 

	PgPilot* pkPilot = GetPilot();
	
	if( pkPilot )
	{// Pilot이 존재하고 (NPC경우에는 없음) 
		CUnit* pkUnit = pkPilot->GetUnit();
		if( pkUnit )
		{// Unit이 있다면
			PgActorUtil::AdjustParticleScaleByUnitScaleAbil( pkUnit, pkParticle );
			PgAction* pkAction = GetAction();
			if( pkAction
				&& 0 < pkAction->GetAbil(AT_APPLY_ATTACK_SPEED)
				)
			{// 현재 액션의 애니 속력에 따라, 파티클 속력 조정하고
				float const fAttackSpeed = pkUnit->GetAbil(AT_C_ATTACK_SPEED)/ABILITY_RATE_VALUE_FLOAT;
				pkParticle->SetPlaySpeed( fAttackSpeed );
			}

			if( IsRidingPet()						// 펫에 타고 있는
				&& pkUnit->IsUnitType(UT_PLAYER)	// 플레이어라면
				)
			{// 펫이 주체가 됨
				pkActor = GetMountTargetPet();
			}
		}
	}
	
	if(pkActor->PgIWorldObject::AttachTo(iSlot,pcTargetName,pkObject))
	{
		if( pkParticle->IsLoop() )
		{
			pkActor->AttachActorAlphaProperty(pkObject);
			pkObject->UpdateProperties();
			pkActor->SetDefaultMaterialNeedsUpdateFlag(false);
		}
		return	true;
	}
	return	false;
}

void PgActor::MoveParticlesToTarget(PgActor* pkTarget)
{
	if(NULL == pkTarget || this == pkTarget)
	{
		return;
	}

	AttachSlot::iterator itAttachSlotSrc = m_kAttachSlot.begin();
	while(itAttachSlotSrc != m_kAttachSlot.end())
	{
		CONT_FIXED_SLOT_LIST::const_iterator itFindFixedElem = m_kFixedParticleList.find( (*itAttachSlotSrc).first );
		if(itFindFixedElem != m_kFixedParticleList.end())
		{ //이동 시키지 말아야할 슬롯이라면 스킵
			++itAttachSlotSrc;
			continue;
		}
		pkTarget->m_kAttachSlot.insert(*itAttachSlotSrc); //대상으로 파티클 이동하고 원본 목록에서 삭제
		m_kAttachSlot.erase(itAttachSlotSrc++);
	}

	AttachSlot::iterator itAttachSlotNoZTestSrc = m_kAttachSlot_NoZTest.begin();
	while(itAttachSlotNoZTestSrc != m_kAttachSlot_NoZTest.end())
	{
		CONT_FIXED_SLOT_LIST::const_iterator itFindFixedElem = m_kFixedParticleList.find( (*itAttachSlotNoZTestSrc).first );
		if(itFindFixedElem != m_kFixedParticleList.end())
		{ //이동 시키지 말아야할 슬롯이라면 스킵
			++itAttachSlotNoZTestSrc;
			continue;
		}
		pkTarget->m_kAttachSlot_NoZTest.insert(*itAttachSlotNoZTestSrc); //대상으로 파티클 이동하고 원본 목록에서 삭제
		m_kAttachSlot_NoZTest.erase(itAttachSlotNoZTestSrc++);
	}
}

bool PgActor::AttachToPoint(int iSlot, NiPoint3 kLoc, NiAVObject *pkObject)
{
	return	PgIWorldObject::AttachToPoint(iSlot,kLoc,pkObject);
}
NiPoint3 PgActor::GetParticleNodeWorldPos(int iSlot, char *strNodeName)
{
	return PgIWorldObject::GetParticleNodeWorldPos(iSlot, strNodeName);
}

NiAVObject* PgActor::GetParticleNode(int iSlot, char const* strNodeName)
{
	return PgIWorldObject::GetParticleNode(iSlot, strNodeName);
}

bool PgActor::DetachFrom(int iSlot, bool bDefaultThreadDelete)
{
	NiAVObject*	pkAVObject = NULL;
	PgActor* pkActor = this;
	if(pkActor->IsRidingPet() && pkActor->GetUnit() && pkActor->GetUnit()->IsUnitType(UT_PLAYER))
	{
		PgActor* pkCallerActor = GetMountTargetPet();
		if(NULL != pkCallerActor)
		{
			pkActor = pkCallerActor;
		}
	}
	AttachSlot::iterator itr = pkActor->m_kAttachSlot.find(iSlot);
	if(itr != pkActor->m_kAttachSlot.end())	//	일반 슬롯에 있으면
	{
		pkAVObject = itr->second;
		pkActor->DetachActorAlphaProperty(pkAVObject);
		pkAVObject->UpdateProperties();
	}

	itr = pkActor->m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != pkActor->m_kAttachSlot_NoZTest.end())
	{
		pkAVObject = itr->second;
		pkActor->DetachActorAlphaProperty(pkAVObject);
		pkAVObject->UpdateProperties();
	}

	pkActor->SetDefaultMaterialNeedsUpdateFlag(false);

	return	pkActor->PgIWorldObject::DetachFrom(iSlot,bDefaultThreadDelete);
}

bool PgActor::IsAttachParticleSlot(int const iSlot)const
{
	NiAVObject*	pkAVObject = NULL;
	PgActor const* pkActor = this;
	if(pkActor->IsRidingPet() && pkActor->GetUnit() && pkActor->GetUnit()->IsUnitType(UT_PLAYER))
	{
		pkActor = GetMountTargetPet();
	}
	AttachSlot::const_iterator itr = pkActor->m_kAttachSlot.find(iSlot);
	if(itr != pkActor->m_kAttachSlot.end())	//	일반 슬롯에 없으면
	{
		return true;
	}

	itr = pkActor->m_kAttachSlot_NoZTest.find(iSlot);	//	Z무시 슬롯에 있는지 찾고
	if(itr != pkActor->m_kAttachSlot_NoZTest.end())
	{
		return true;
	}

	return	false;
}

void	PgActor::SetDefaultMaterialNeedsUpdateFlag(bool bFlag)
{
	NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( this, bFlag );
	
	// Attach 녀석들 정리
	for(AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr)
	{
		PgParticle *pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle)
		{
			NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkParticle, bFlag );
		}
	}

	for(AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); ++itr)
	{
		PgParticle *pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle)
		{
			NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkParticle, bFlag );
		}
	}
}
void	PgActor::SetAlpha(float fAlpha)
{
	if(!GetNIFRoot())
		return;

	if(m_spAlphaProperty)
	{
		bool	bNeedAlphaBlending = fAlpha<1;
		if(bNeedAlphaBlending != m_spAlphaProperty->GetAlphaBlending())
		{
			SetDefaultMaterialNeedsUpdateFlag(false);	//	쉐이더를 업데이트 시켜야한다.
		}
		m_spAlphaProperty->SetAlphaBlending(bNeedAlphaBlending);
	}

	NiColorA	kColor = GetNIFRoot()->GetColorLocal();
	kColor.a = fAlpha;
	GetNIFRoot()->SetColorLocal(kColor);

	if ( m_spTitleName )
	{
		m_spTitleName->SetAlpha( fAlpha );
	}

	if ( m_spNameText )
	{
		m_spNameText->SetAlpha( fAlpha );
	}

	if( m_spGuildNameText )
	{
		m_spGuildNameText->SetAlpha( fAlpha );
	}

	if( m_spAchievementTitle )
	{
		m_spAchievementTitle->SetAlpha( fAlpha );
	}

	if( m_spGIFTitle )
	{
		m_spGIFTitle->SetAlpha( fAlpha );
	}

	if(m_spDuelTitle)
	{
		//m_spDuelTitle->SetAlpha(fAlpha);
	}

	if(m_spEffectCountDown)
	{
		m_spEffectCountDown->SetAlpha(fAlpha);
	}
}
float PgActor::GetAlpha()const
{
	if(!GetNIFRoot())
		return 0.0f;

	return GetNIFRoot()->GetColorLocal().a;
}

void PgActor::TransitAlpha(float fTime)
{
	if(!m_bLoadingComplete)
	{
		return;
	}

	if( 0.0f == m_AlphaTransitInfo.m_fAlphaTransitionTime ) 
		return;

	if(m_AlphaTransitInfo.m_fAlphaTransitionTime <= m_AlphaTransitInfo.m_fAlphaAccumTime)
	{
		m_AlphaTransitInfo.m_fAlphaTransitionTime = 0.0f;
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.TransitAlpha"), g_pkApp->GetFrameCount()));

	m_AlphaTransitInfo.m_fAlphaAccumTime += fTime;
	m_AlphaTransitInfo.m_fAlphaAccumTime = NiMin(m_AlphaTransitInfo.m_fAlphaAccumTime, m_AlphaTransitInfo.m_fAlphaTransitionTime);
	m_AlphaTransitInfo.m_fCurrentAlpha = m_AlphaTransitInfo.m_fPrevAlpha + 
		((m_AlphaTransitInfo.m_fTargetAlpha - m_AlphaTransitInfo.m_fPrevAlpha)
		* m_AlphaTransitInfo.m_fAlphaAccumTime / m_AlphaTransitInfo.m_fAlphaTransitionTime);

	SetAlpha(m_AlphaTransitInfo.m_fCurrentAlpha);

}

void	PgActor::SetColor(const NiColor& kColor)
{
	if(!GetNIFRoot())
		return;

	NiColorA	kGeomColor = GetNIFRoot()->GetColorLocal();
	kGeomColor.r = kColor.r;
	kGeomColor.g = kColor.g;
	kGeomColor.b = kColor.b;

	GetNIFRoot()->SetColorLocal(kGeomColor);

}

void PgActor::TransitActorSpecular(float fTime)
{
	if(m_SpecularTransitInfo.m_fTransitionTime == 0)
	{
		return;
	}
	if(m_SpecularTransitInfo.m_fTransitionTime<m_SpecularTransitInfo.m_fAccumTime)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.TransitActorColor"), g_pkApp->GetFrameCount()));
	m_SpecularTransitInfo.m_fAccumTime += fTime;
	if(m_SpecularTransitInfo.m_fAccumTime > m_SpecularTransitInfo.m_fTransitionTime)
	{
		m_SpecularTransitInfo.m_fAccumTime = m_SpecularTransitInfo.m_fTransitionTime;
	}

	// 보간할 비율을 계산한다.
	float fRate = m_SpecularTransitInfo.m_fAccumTime / m_SpecularTransitInfo.m_fTransitionTime;
	if((fRate = NiClamp(fRate, 0.0f, 1.0f)) == 1.0f)
	{
		m_SpecularTransitInfo.m_fTransitionTime = 0.0f;
		if(m_SpecularTransitInfo.m_kTarget == NiColor::BLACK)
		{
			RestoreSpecular();
		}
	}

	NiColor kTargetSpecular;
	NiColor kCurrentSpecular;

	// Current Color -> Target Color로 선형 보간한다.
	MaterialColorContainer::iterator curItr = m_kMaterialCurrentColorContainer.begin();
	for(MaterialColorContainer::iterator itr = m_kMaterialColorContainer.begin();
		itr != m_kMaterialColorContainer.end();
		++itr)
	{
		NiMaterialProperty *pkMaterialProp = itr->first;
		ColorSet &rkColorSet = itr->second;

		if(curItr != m_kMaterialCurrentColorContainer.end())
		{
			kCurrentSpecular = curItr->second.m_kSpecular;
			++curItr;
		}

		NiColor kSpecular = kCurrentSpecular + (m_SpecularTransitInfo.m_kTarget - kCurrentSpecular) * fRate;

		pkMaterialProp->SetSpecularColor(kSpecular);
	}
}

void PgActor::TransitColor(float fTime)
{
	if(m_ColorTransitInfo.m_fTransitionTime == 0) 
		return;

	if(m_ColorTransitInfo.m_fTransitionTime <= m_ColorTransitInfo.m_fAccumTime)
	{
		m_ColorTransitInfo.m_fTransitionTime = 0.0f;
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.TransitColor"), g_pkApp->GetFrameCount()));

	m_ColorTransitInfo.m_fAccumTime += fTime;
	m_ColorTransitInfo.m_fAccumTime = NiMin(m_ColorTransitInfo.m_fAccumTime, m_ColorTransitInfo.m_fTransitionTime);
	m_ColorTransitInfo.m_kCurrent = m_ColorTransitInfo.m_kPrev + 
		((m_ColorTransitInfo.m_kTarget - m_ColorTransitInfo.m_kPrev)
		* m_ColorTransitInfo.m_fAccumTime / m_ColorTransitInfo.m_fTransitionTime);

	SetColor(m_ColorTransitInfo.m_kCurrent);

}

void PgActor::SetParticleAlpha(NiAVObject* pkRoot, float fAlpha)
{
	if (pkRoot == NULL)
		return;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetParticleAlpha"), g_pkApp->GetFrameCount()));

	if (NiIsKindOf(NiGeometry, pkRoot))
	{
		NiGeometry *pkGeometry = (NiGeometry *)pkRoot;

		// MaterialProperty를 가져온다.
		NiMaterialProperty *pkMaterialProp = pkGeometry->GetPropertyState()->GetMaterial();
		if(!pkMaterialProp)
		{
			pkMaterialProp = NiNew NiMaterialProperty();
			pkRoot->AttachProperty(pkMaterialProp);
		}

		pkMaterialProp->SetAlpha(fAlpha);
	}
	else if (NiIsKindOf(NiNode, pkRoot))
	{
		NiNode* pkNode = NiDynamicCast(NiNode, pkRoot);
		unsigned int uiArrayCount = pkNode->GetArrayCount();
		for (unsigned int i = 0; i < uiArrayCount; ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			SetParticleAlpha(pkChild, fAlpha);
		}
	}
}

//!	액터의 스페큘러를 켠다
void	PgActor::TurnOnSpecular()
{

	if(g_pkWorld)
	{
		g_pkWorld->LightObjectRecurse(g_pkWorld->GetLightRoot(),this);
	}

	UpdateEffects();

	for(SpecularEnableContainer::iterator itor = m_kSpecularContainer.begin(); itor != m_kSpecularContainer.end(); ++itor)
	{
		NiSpecularPropertyPtr spSpecular = itor->first;
		if(spSpecular)
		{
			spSpecular->SetSpecular(true);
		}
		
	}

	m_bSpecularOn = true;

	SetDefaultMaterialNeedsUpdateFlag(false);	//	쉐이더를 업데이트 시켜야한다.

}
//!	액터의 스페큘러를 원상복구시킨다.
void	PgActor::RestoreSpecular()
{

	if(g_pkWorld)
	{
		g_pkWorld->RemoveLightObjectRecurse(g_pkWorld->GetLightRoot(),this);
	}
	m_bSpecularOn = false;

	UpdateEffects();
	for(SpecularEnableContainer::iterator itor = m_kSpecularContainer.begin(); itor != m_kSpecularContainer.end(); ++itor)
	{
		NiSpecularPropertyPtr spSpecular = itor->first;
		if(spSpecular)
		{
			bool	const	bTurnOn = itor->second;

			spSpecular->SetSpecular(bTurnOn);
		}
	}
	SetDefaultMaterialNeedsUpdateFlag(false);	//	쉐이더를 업데이트 시켜야한다.
}


bool PgActor::StoreDefaultMaterialColor(NiNode* pkNode, bool bForce)
{
	if(bForce)
	{
		m_bMaterialColorCached = false;
	}

	if(!m_bMaterialColorCached)
	{
		m_kMaterialColorContainer.clear();
		for(MaterialContainer::const_iterator itr = m_kMaterialContainer.begin();
			itr != m_kMaterialContainer.end();
			++itr)
		{
			NiMaterialProperty *pkMat = *itr;
			m_kMaterialColorContainer.insert(std::make_pair(pkMat, ColorSet(pkMat->GetAmbientColor(), pkMat->GetEmittance(),pkMat->GetSpecularColor(),pkMat->GetDiffuseColor())));
		}

		m_bMaterialColorCached = true;
	}

	return true;
}

bool PgActor::GetAlphaProperty()
{
	return true;
}


bool PgActor::FindMaterialProp(NiNode* pkRoot, bool bCheckNoChange, bool bTraverseAllNode)
{
	if(!pkRoot)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.FindMaterialProp"), g_pkApp->GetFrameCount()));

	if (NiIsKindOf(NiGeometry, pkRoot))
	{
		NILOG(PGLOG_WARNING, "[PgActor] FindMaterialProp %s root is NiGeometry\n", pkRoot->GetName());
	}

	unsigned int uiArrayCount = pkRoot->GetArrayCount();
	for (unsigned int i = 0; i < uiArrayCount; ++i)
	{
		NiAVObject* pkChild = pkRoot->GetAt(i);
		if(!pkChild)
		{
			continue;
		}
		
		const NiFixedString &rkChildName = pkChild->GetName();
		if(bCheckNoChange)
		{
			NiStringExtraData *pkExtra = NiDynamicCast(NiStringExtraData, pkChild->GetExtraData("UserPropBuffer"));
			if(pkExtra)
			{
				// Geometry에 NoColorChange라고 UserProp에 적어뒀으면, 그에 해당하는 Material은 리스트에 추가하지 않는다.
				NiFixedString kUserDefined = pkExtra->GetValue();
				if(kUserDefined.ContainsNoCase("NoColorChange"))
				{
					continue;
				}
			}
		}

		if(NiIsKindOf(NiGeometry, pkChild))
		{
			if(rkChildName.ContainsNoCase("Biped Object") || rkChildName.ContainsNoCase("Bone"))
			{
				continue;
			}

			NiGeometry *pkGeometry = (NiGeometry *)pkChild;

			// MaterialProperty를 가져온다.
			NiMaterialProperty *pkMaterialProp = pkGeometry->GetPropertyState()->GetMaterial();
			if(!pkMaterialProp)
			{
				pkMaterialProp = NiNew NiMaterialProperty();
				pkChild->AttachProperty(pkMaterialProp);
				NILOG(PGLOG_WARNING, "[PgActor.FindMaterialProp] The MaterialProperty has newly allocated. It must be deleted. (%p)\n", &pkMaterialProp);
			}

			NiSpecularProperty *pkSpecularOriginal = pkGeometry->GetPropertyState()->GetSpecular();

			NiSpecularProperty *pkSpecular = NiNew NiSpecularProperty();
			if(pkSpecularOriginal == NULL || pkSpecularOriginal->GetSpecular() == false)
				pkSpecular->SetSpecular(false);
			else
				pkSpecular->SetSpecular(true);

			pkChild->DetachProperty(pkSpecularOriginal);
			pkChild->AttachProperty(pkSpecular);

			m_kSpecularContainer.insert(std::make_pair(pkSpecular,pkSpecular->GetSpecular()));

			pkChild->UpdateProperties();
			m_kMaterialContainer.push_back(pkMaterialProp);
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			if(bTraverseAllNode || rkChildName != NiFixedString("char_root") || !pkChild->GetAppCulled())
			{
				FindMaterialProp((NiNode* )pkChild, bCheckNoChange, bTraverseAllNode);
			}
		}
	}

	return true;
}

void PgActor::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateDownwardPass"), g_pkApp->GetFrameCount()));
	fTime+=m_fAccumTimeAdjust;

	if(IsVisible() == false)
	{
		if(m_fLastDownwardPassUpdateTime == 0.0f || fTime - m_fLastDownwardPassUpdateTime > 10.0f)
		{
			m_fLastDownwardPassUpdateTime = fTime;
			NiNode::UpdateDownwardPass(m_fLastDownwardPassUpdateTime,bUpdateControllers);
		}
		else
		{
			{
				if(GetPositionChanged() || (GetAction() && GetAction()->GetActionOptionEnable(PgAction::AO_ALWAYS_UPDATE)))
				{
					SetPositionChanged(false);
					NiNode::UpdateDownwardPass(fTime,false);
				}
			}
		}
		return;
	}

	if (m_dwLastUpdateFrame == g_pkApp->GetFrameCount())
	{
		m_iUpdateCount++;
		NILOG(PGLOG_WARNING, "[PgActor] %s Update %d frame more than twice(%f)\n", MB(GetGuid().str()), g_pkApp->GetFrameCount(), NiGetCurrentTimeInSec() - m_fLastUpdateFrameTime);
	}
	
	if (fTime != 0.0f)
	{
		if (m_dwLastUpdateFrame != g_pkApp->GetFrameCount())
		{
			m_iUpdateCount = 0;
		}
		m_dwLastUpdateFrame = g_pkApp->GetFrameCount();
	}
	if(g_pkWorld)
	{
		m_fLastUpdateFrameTime = g_pkWorld->GetAccumTime();
	}

	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);
}

// 정밀한 각을 몇번이나 잡는가?
// (0)이면 1도 차이로 떨어질때까지 루프 돌면서 찾는다.
#define PG_GET_DELTALOC_COUNT		(3)
NxVec3 PgActor::GetAdjustValidDetailDeltaLoc(NxVec3 kTargetDeltaLoc, NxVec3 kFindLoc, float fDegree)
{
	NxVec3 kDeltaLoc = kFindLoc;
	NxScene *pkScene = GetWorld()->GetPhysXScene()->GetPhysXScene();
	if (!pkScene)
	{
		return kDeltaLoc;
	}
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetAdjustValidDetailDeltaLoc"), g_pkApp->GetFrameCount()));
	NxExtendedVec3 kCharPos = m_pkController->getPosition();
	NxRaycastHit kHit;

	int iCount = 0;
	float fDeltaDegree = fDegree * 0.5f;
	float fHalfDegree = fDeltaDegree;

	while( (!PG_GET_DELTALOC_COUNT) ||
			(iCount < PG_GET_DELTALOC_COUNT) )
	{
		if (fabs(fHalfDegree) < 1.0f)
		{
			break;
		}

		fHalfDegree = fHalfDegree * 0.5f;

		NxMat33 kMat;
		kMat.id();
		kMat.rotZ(NxMath::degToRad(fDeltaDegree));

		NxVec3 kDelta = kTargetDeltaLoc;
		kDelta = kMat * kDelta;	//	이동벡터를 fDeltaDegree 만큼 Z축을 중심으로 회전시킨다.

		NxExtendedVec3 kAfterPos = kCharPos;
		kAfterPos += kDelta;	//	회전된 이동벡터만큼 전진된 좌표를 구한다.

		if (	//	kAfterPos 에 캐릭터가 설 수 있는가?
			GetWorld()->raycastClosestShape(
			NxRay(NxVec3((NxReal)kAfterPos.x, (NxReal)kAfterPos.y, (NxReal)kAfterPos.z), NX_NEGATIVE_UNIT_Z),
			NX_STATIC_SHAPES, kHit, -1, 100000, NX_RAYCAST_SHAPE))
		{
			kDeltaLoc = kDelta;
			fDeltaDegree -= fHalfDegree;	//	fDeltaDegree 를 반으로 줄인다.
		}
		else
		{
			fDeltaDegree += fHalfDegree;	//	fDeltaDegree 를 증가시킨다.
		}

		iCount += 1;
	}

	return kDeltaLoc;
}

#define PG_USE_DETAIL_DELTALOC
NxVec3 PgActor::GetAdjustValidDeltaLoc(NxVec3 kTargetDeltaLoc)
{
	if (kTargetDeltaLoc.isZero())
	{
		return kTargetDeltaLoc;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetAdjustValidDeltaLoc"), g_pkApp->GetFrameCount()));
	// 다음 좌표가 떨어지지 않게 보정해서 좌표를 리턴한다.
	NxExtendedVec3 kCharPos = m_pkController->getPosition();
	NxExtendedVec3 kBeforePos = kCharPos;
	kBeforePos += kTargetDeltaLoc;

	NxScene *pkScene = GetWorld()->GetPhysXScene()->GetPhysXScene();
	NxRaycastHit kHit;
	bool bFind = false;
	if(pkScene && GetWorld()->raycastClosestShape(
		NxRay(NxVec3((NxReal)kBeforePos.x, (NxReal)kBeforePos.y, (NxReal)kBeforePos.z - (m_pkController->getHeight() * 0.5f)), NX_NEGATIVE_UNIT_Z),
		NX_STATIC_SHAPES, kHit, -1, 100000.0f, NX_RAYCAST_SHAPE))
	{
		// 다음 좌표가 캐릭터가 갈수 있는 좌표라면 아무런 조작없이 Delta를 리턴
		bFind = true;
	}
	// 다음 좌표가 캐릭터가 갈수 없는 좌표라면
	// 각만큼 돌려서 찾은다음 잘 갈수 있게 해준다.
	else if(pkScene)
	{
		NxMat33 kMat1, kMat2;
		kMat1.id();
		kMat2.id();
		kMat1.rotZ(NxMath::degToRad(50.0f));
		kMat2.rotZ(NxMath::degToRad(-50.0f));
		NxVec3 kDelta1 = kTargetDeltaLoc;
		kDelta1 = kMat1 * kDelta1;	//	이동벡터를 Z축을 중심으로 50도 회전
		NxVec3 kDelta2 = kTargetDeltaLoc;
		kDelta2 = kMat2 * kDelta2;	//	이동벡터를 Z축을 중심으로 -50도 회전

		NxExtendedVec3 kAfterPos1 = kCharPos;
		kAfterPos1 += kDelta1;	//	Z축을 중심으로 50도 회전시킨 이동벡터만큼 전진.
		NxExtendedVec3 kAfterPos2 = kCharPos;
		kAfterPos2 += kDelta2;	//	Z축을 중심으로 -50도 회전시킨 이동벡터만큼 전진.
		
		if (	//	kAfterPos1 좌표로 캐릭터가 이동 할 수 있는가?
			GetWorld()->raycastClosestShape(
			NxRay(NxVec3((NxReal)kAfterPos1.x, (NxReal)kAfterPos1.y, (NxReal)kAfterPos1.z - (m_pkController->getHeight() * 0.5f)), NX_NEGATIVE_UNIT_Z),
			NX_STATIC_SHAPES, kHit, -1, 100000.0f, NX_RAYCAST_SHAPE))
		{
#ifdef PG_USE_DETAIL_DELTALOC
			kTargetDeltaLoc = GetAdjustValidDetailDeltaLoc(kTargetDeltaLoc, kDelta1, 50.0f);	//	더 정밀한 좌표를 찾는다.
#else
			kTargetDeltaLoc = kDelta1;
#endif
		}
		else if (	//	kAfterPos2 좌표로 캐릭터가 이동 할 수 있는가?
			GetWorld()->raycastClosestShape(
			NxRay(NxVec3((NxReal)kAfterPos2.x, (NxReal)kAfterPos2.y, (NxReal)kAfterPos2.z - (m_pkController->getHeight() * 0.5f)), NX_NEGATIVE_UNIT_Z),
			NX_STATIC_SHAPES, kHit, -1, 100000.0f, NX_RAYCAST_SHAPE))
		{
#ifdef PG_USE_DETAIL_DELTALOC
			kTargetDeltaLoc = GetAdjustValidDetailDeltaLoc(kTargetDeltaLoc, kDelta2, -50.0f);	//	더 정밀한 좌표를 찾는다.
#else
			kTargetDeltaLoc = kDelta2;
#endif
		}
		else	//	이동할 수 있는 곳이 없다면, 이동벡터를 0로 만든다.
		{
			kTargetDeltaLoc.x = 0.0f;
			kTargetDeltaLoc.y = 0.0f;
		}
	}

	return kTargetDeltaLoc;
}

void PgActor::UpdatePhysX(float fAccumTime, float fFrameTime)
{
	PG_ASSERT_LOG(m_pkPhysXActor);
	PG_ASSERT_LOG(m_pkController);

	bool	const	bIsMyActor = IsMyActor();

	if (m_pkPhysXActor == NULL || m_pkController == NULL)
	{
		return;	//	leesg213 2006.12.06
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdatePhysX"), g_pkApp->GetFrameCount()));

#ifdef PG_USE_ACTOR_TRACE
	NxExtendedVec3 beforeUpdatePos = m_pkController->getPosition();
	NxExtendedVec3 afterUpdatePos = m_pkController->getPosition();	
	NxExtendedVec3 afterMove1Pos = m_pkController->getPosition();
	NxExtendedVec3 beforeMove2Pos = m_pkController->getPosition();
	NxExtendedVec3 afterMove2Pos = m_pkController->getPosition();
#endif
	NxExtendedVec3 beforeMove1Pos = m_pkController->getPosition();

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		GetWorld()->LockPhysX(true);
		PG_STAT(timerA.Stop());
	}

	// 회전을 보간한다.
	if(GetRotationInterpolTime() < 1.0f)
	{
		SetRotationInterpolTime(1.0f);
		SetRotation(m_kToRotation);

		//m_fRotationInterpolTime = NiMin(m_fRotationInterpolTime + fFrameTime * PG_ROTATION_INTERPOL_SPEED, 1.0f);
		//
		//NxQuat kToRot;
		//NiPhysXTypes::NiQuaternionToNxQuat(m_kToRotation, kToRot);
		//NxQuat kCurRot = m_pkPhysXActor->getGlobalOrientationQuat();

		//NxQuat kNewRot;
		//kNewRot.slerp(m_fRotationInterpolTime, kCurRot, kToRot);
		//kNewRot.normalize();
		//m_pkPhysXActor->setGlobalOrientationQuat(kNewRot);
	}

	float	fJumpHeight=0.0f;
	if(GetJump())
	{
		// 점프 중이라면 점프 높이를 계산하자.
		// h = (v0 * (t+ delta t) + 0.5 * g(t + delta t)^2) - (v0*t + 0.5 * gt^2)
		//   = (v0 * (JumpTime + FrameTime) + 0.5 * g(JumpTime + FrameTime)^2) - (v0*t + 0.5 * gt^2)
		//   = ...
		//   = v0 + (g * (0.5 * JumpTime + FrameTime))
		fJumpHeight = GetInitialVelocity() + GetGravity() * (0.5f * fFrameTime +  GetJumpTime());
		SetJumpTime(GetJumpTime()+fFrameTime);
		
		SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,m_kMovingDelta.z+fJumpHeight));
		//m_fJumpAccumHeight += fJumpHeight * fFrameTime;

		//NILOG(PGLOG_MINOR, "Moving Delta Jump: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
		//NILOG(PGLOG_MINOR, "Moving Delta Jump: Initv %.f, fGra %.f, fJump %.f, fFrame %.f\n", m_fInitialVelocity, ms_fGravity, m_fJumpTime, fFrameTime);
	}
	else
	{
		m_fJumpAccumHeight = 0.0f;
		if(!GetFreeMove())
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,-98.0f));
		}
	}
	
	bool	bDoNotSlide = false;
	
	Update_IsAbleSlide(fAccumTime, fFrameTime, bDoNotSlide);	// 경사면에서 미끄러지기가 가능한지 체크 하고
	
	if(m_fSpecifiedFrameTime != 0.0f)	//	이거는 현재 쓰이지 않는다.
	{
		SetMovingDelta(m_kMovingDelta * m_fSpecifiedFrameTime);
		m_fSpecifiedFrameTime = 0.0f;
	}
	else
	{
		SetMovingDelta(m_kMovingDelta * fFrameTime);
	}

	//WriteToConsole("________ Moving Delta : %.1f, %.1f, %.1f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
	//NILOG(PGLOG_MINOR, "Moving Delta Frame: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);

#ifdef PG_USE_CAPSULE_CONTROLLER
	float fControllerHalfHeight = m_pkController->getHeight() * 0.5f;
	float fControllerRadius = m_pkController->getRadius();
#else
	float fControllerHalfHeight = m_pkController->getExtents().y * 0.5f;
	float fControllerRadius = m_pkController->getExtents().x;
#endif
	
	// NxController로 캐릭터를 움직인다.
	NxU32 collisionFlags = 0;

	// TODO : 아래 코드를 Walk로 빼도 되는가?
	// Set Walking Location을 쓸 때, 다음 움직일 곳이 떨어지는 곳이면 못가게 하는 것
	//if(m_bCheckCliff)
	//{
	//	NxExtendedVec3 kCharPos = m_pkController->getPosition();
	//	NxExtendedVec3 kBeforePos = kCharPos;
	//	kBeforePos += m_kMovingDelta;

	//	NxScene *pkScene = GetWorld()->GetPhysXScene()->GetPhysXScene();
	//	NxRaycastHit kHit;
	//	if(pkScene && !pkScene->raycastClosestShape(
	//		NxRay(NxVec3(static_cast<NxReal>(kBeforePos.x), static_cast<NxReal>(kBeforePos.y), static_cast<NxReal>(kBeforePos.z)), NX_NEGATIVE_UNIT_Z),
	//		NX_STATIC_SHAPES, kHit, -1, 50.0f, NX_RAYCAST_SHAPE))
	//	{
	//		m_kMovingDelta.y = m_kMovingDelta.x = 0.0f;
	//		m_kTargetDir = NiPoint3::ZERO;
	//		m_bWalkingToTargetForce = false;
	//		if(m_kTargetWalkingNextAction.length() != 0)
	//		{
	//			TransitAction(m_kTargetWalkingNextAction.c_str());
	//		}
	//	}
	//}

	beforeMove1Pos = m_pkController->getPosition();

	// 얼어 있으면, Z값 외에는 움직이지 않는다.
	if( GetFreezed() )
	{
		SetMovingDelta(NxVec3(0.0f,0.0f,m_kMovingDelta.z));
	}

#ifdef PG_USE_ACTOR_AUTOMOVE_EDGE
	if (GetAdjustValidPos() && GetSlide() == false)
	{
		// 다음 좌표가 떨어지지 않게 보정해서 좌표를 리턴한다.
		float fDeltaZ = m_kMovingDelta.z;
		if(!GetJump())
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,0.0f));
		}

		SetMovingDelta(GetAdjustValidDeltaLoc(m_kMovingDelta));

		if(!GetJump())
		{
			SetMovingDelta(NxVec3(m_kMovingDelta.x,m_kMovingDelta.y,fDeltaZ));
		}
	}
#endif

#ifdef PG_USE_ACTOR_TRACE
	afterMove2Pos = afterMove1Pos = m_pkController->getPosition();
	//_PgOutputDebugString("Cur Loc : %.1f, %.1f, %.1f\n", afterMove1Pos.x, afterMove1Pos.y, afterMove1Pos.z);
#endif

	bool bPreviousFloor = IsMeetFloor();
	
//	m_kSlideVector.zero();
//	m_bSlide = false;
	
	//if(IsMyActor())
	//{
	//	_PgOutputDebugString("_______Moving Delta : %.3f, %.3f, %.3f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
	//}

	
	NxVec3	kFinalMove = m_kMovingDelta + m_kMovingAbsolute;

	NxExtendedVec3	kBeforeMove = m_pkController->getPosition();
	
	if(m_kMovingAbsolute != NX_ZERO)
	{
		unsigned	int	uiActiveGroup = m_uiActiveGrp & (~(1 << PG_PHYSX_GROUP_OBJECT));

		NxU32 collisionFlagsabs = 0;
		m_pkController->move(m_kMovingAbsolute, uiActiveGroup, 0.000001f, collisionFlagsabs, 1.0f);
		m_kMovingAbsolute.zero();
	}

	InitControllerShapeHit();

	m_pkController->move(m_kMovingDelta, m_uiActiveGrp, 0.000001f, collisionFlags, 1.0f);

	NxExtendedVec3	kAfterMove = m_pkController->getDebugPosition();

	NxVec3 kCharPos(static_cast<NxReal>(kAfterMove.x), static_cast<NxReal>(kAfterMove.y), static_cast<NxReal>(kAfterMove.z));
	UpdateBottomRayHit(kCharPos);

	if(!GetFreeMove() && IsOnRidingObject() && GetMovingDelta().z<0)
	{
		NxRay kRay(kCharPos, NX_NEGATIVE_UNIT_Z);
		NxRaycastHit kHit;
		NxShape *pkHitShape = GetWorld()->raycastClosestShape(kRay, NX_ALL_SHAPES, kHit, 1<<PG_PHYSX_GROUP_OBJECT, 10000.0f, NX_RAYCAST_SHAPE|NX_RAYCAST_IMPACT);
		if(pkHitShape && (fabs(kCharPos.z - kHit.worldImpact.z)<PG_CHARACTER_Z_ADJUST) )
		{

			kAfterMove = NxExtendedVec3(kAfterMove.x,kAfterMove.y,kHit.worldImpact.z + PG_CHARACTER_Z_ADJUST);

			m_pkController->setPosition(kAfterMove);
			collisionFlags |= NXCC_COLLISION_DOWN;

		}
	}

	SetPositionChanged( (kAfterMove.x != kBeforeMove.x) || (kAfterMove.y != kBeforeMove.y) || (kAfterMove.z != kBeforeMove.z) );

	SetMeetFloor((collisionFlags & NXCC_COLLISION_DOWN ? true : false));
	m_bSide = (collisionFlags & NXCC_COLLISION_SIDES ? true : false);
	
	if(IsMyActor())
	{
		NxVec3 kTempVec = m_kMovingDelta;
		kTempVec.z = 0.0f;
		float fReserveMagnitude = kTempVec.magnitudeSquared();

		NxExtendedVec3 kNextDebugPos = m_pkController->getDebugPosition();
		NxVec3 kTempVec2 = beforeMove1Pos - kNextDebugPos;
		kTempVec2.z = 0.0f;
		float fRealMagnitude = kTempVec2.magnitudeSquared();


		if(m_bSide && fReserveMagnitude - fRealMagnitude > 1.0f)
		{
			//WriteToConsole("[%s] Now character is leaning the wall\n", __FUNCTION__);

			if(m_bDoSimulateOnServer)
			{
				m_bDoSimulateOnServer = false;
				m_dwLastSimulatedTime = BM::GetTime32();
				g_kPilotMan.BroadcastSimulate(GetPilot(), m_bDoSimulateOnServer);
				//WriteToConsole("[%s] Broadcast Don't Simulate on server\n", __FUNCTION__);

			}
		}
		else if(!m_bDoSimulateOnServer && BM::GetTime32() - m_dwLastSimulatedTime > 500)
		{
			//WriteToConsole("[%s] Broadcast Do Simulate on server\n", __FUNCTION__);
			m_bDoSimulateOnServer = true;
			g_kPilotMan.BroadcastSimulate(GetPilot(), m_bDoSimulateOnServer);
		}
	}

	CheckRidingObject();

	if(!GetFreeMove())
	{
		// 무조건 아래로 Ray를 쏴야 하는 경우이다.
#ifdef USE_CAPSULE_CONTROLLER
		float fCenterHeight = m_pkController->getHeight() * 0.5f + m_pkController->getRadius();
		float fLegHeight = m_pkController->getRadius();
#else
		float fCenterHeight = PG_CHARACTER_CAPSULE_HEIGHT * 0.5f + PG_CHARACTER_CAPSULE_RADIUS;
		float fLegHeight = PG_CHARACTER_CAPSULE_RADIUS;
#endif

		if(IsMeetFloor())
		{
			NxActor *pkActor = GetPhysXActor();
			if(bDoNotSlide == false && m_kControllerShapeHit.shape)
			{
				NxVec3	const	vNormal = m_kControllerShapeHit.worldNormal;
				//NxCollisionGroup kGroup = m_kControllerShapeHit.shape->getGroup(); // 엘가던전 피직스 크래시 부분. 현재 사용하지 않아도 무관하기에 삭제함

				float fAngleToFloor = NiACos(vNormal.dot(NX_UNIT_Z));

				bool	bSlide = false;

				float	fAngleDeg = NxMath::radToDeg(fAngleToFloor);

				if( (91.0f>=fAngleDeg && PG_LIMITED_ANGLE < fAngleDeg) 
					//&& (kGroup == PG_PHYSX_GROUP_BASE_FLOOR || kGroup == PG_PHYSX_GROUP_BASE_WALL || kGroup == PG_PHYSX_GROUP_OBJECT) // kGroup를 얻어올수 없으므로 삭제
					)
				{// 두 번의 Cross Product로 빗변 방향의 벡터를 구한다.
					if(90.0f > fAngleDeg)
					{
						m_kSlideVector = NX_UNIT_Z.cross(vNormal);
						m_kSlideVector.normalize();
						m_kSlideVector = m_kSlideVector.cross(vNormal);
						m_kSlideVector.normalize();
					}
					else
					{
						m_kSlideVector = vNormal;
					}

					if(GetJump())
					{
						if(0.0f >= fJumpHeight)
						{
							bSlide = true;
						}
					}
					else
					{
						bSlide = true;
					}
				}

				if(bSlide && !GetSlide())
				{
					SetSlideStartTime(fAccumTime);
				}

				SetSlide(bSlide);
			}


			if(!GetSlide())
			{
				StopJump();
			}
		}
		else if(!GetJump())
		{
			// 캐릭터 발 끝에서 바닥으로 Ray를 쏴서 필요한 정보를 얻어온다
			NxExtendedVec3 kCharPosEx = m_pkController->getDebugPosition();
			NxVec3 kCharPos(static_cast<NxReal>(kCharPosEx.x), static_cast<NxReal>(kCharPosEx.y), static_cast<NxReal>(kCharPosEx.z));
			NxRay kRay(kCharPos - NxVec3(0.0f, 0.0f, fCenterHeight - 0.5f), NX_NEGATIVE_UNIT_Z);
			NxRaycastHit kHit;
			NxShape *pkHitShape = GetWorld()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 0xffffffff, 10000.0f, NX_RAYCAST_SHAPE);

			if(!pkHitShape || (kCharPos - kHit.worldImpact).magnitude() > fCenterHeight + fLegHeight + 0.5f)
			{
				// 절벽임.
				m_bFalling = true;
				StartJump(0);
				SetSlide(false);
			}
			else
			{
				// 계단이거나, 내리막이다. floor = true로 해주자.
				SetMeetFloor(true);
				SetSlide(false);
			}
		}
	
		// Actor가 천장에 부딪혔을 때
		if((collisionFlags & NXCC_COLLISION_UP)	&& (GetInitialVelocity() != 0.0f))
		{
			SetInitialVelocity(0.0f);
			SetJumpTime(0.0f);
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}

	if(GetJump())
	{
		// 반드시 getDebugPosition을 써야 한다. move를 한 후 Update가 되기 전에는 getPosition으로 가져오면 좌표가 그대로다.
		NxExtendedVec3 kCurPos = m_pkController->getDebugPosition();
		float fRealJumpHeight = static_cast<float>(kCurPos.z - beforeMove1Pos.z);
		m_fJumpAccumHeight += fRealJumpHeight;
	}


	m_kPrevMovingDelta = m_kMovingDelta;
	SetMovingDelta(NX_ZERO);

	if(m_pkAction && m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
	{
		FindPathNormal();
	}

	if (IsMeetFloor())
	{
		NxExtendedVec3 curPos = m_pkController->getDebugPosition();
		m_kLastFloorPos.x = (float)curPos.x;
		m_kLastFloorPos.y = (float)curPos.y;
		m_kLastFloorPos.z = (float)curPos.z;
	}

	if(bPreviousFloor != IsMeetFloor() && GetAction())	//	바닥에서 떨어졌거나 혹은 바닥에 닿았을때 스크립트 호출한다.
	{
		lua_tinker::call<void, lwActor, lwAction,bool>("Actor_OnMeetFloor", lwActor(this), lwAction(GetAction()),IsMeetFloor());
	}

#ifdef PG_USE_ACTOR_TRACE
	afterUpdatePos = m_pkController->getPosition();

	if (m_bTraceUpdate)
	{
		if(m_kLastFramePos.z != beforeUpdatePos.z)
		{
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form last updatePhysx(%f,%f,%f) to (%f,%f,%f)\n", MB(GetGuid().str()), m_kLastFramePos.x, m_kLastFramePos.y, m_kLastFramePos.z, beforeUpdatePos.x, beforeUpdatePos.y, beforeUpdatePos.z);
		}

		if(beforeMove1Pos.z != afterMove1Pos.z)
		{
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form before move1(%f,%f,%f) to after move1(%f,%f,%f)\n", MB(GetGuid().str()), beforeMove1Pos.x, beforeMove1Pos.y, beforeMove1Pos.z, afterMove1Pos.x, afterMove1Pos.y, afterMove1Pos.z);
		}

		if(beforeMove2Pos.z != afterMove2Pos.z)
		{
			NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos changed form before Move2(%f,%f,%f) to after Move2(%f,%f,%f)\n", MB(GetGuid().str()), beforeMove2Pos.x, beforeMove2Pos.y, beforeMove2Pos.z, afterMove2Pos.x, afterMove2Pos.y, afterMove2Pos.z);
		}
	}

	if (m_kLastFramePos.z > 0.0f && afterUpdatePos.z < 0.0f)
	{
		NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos z go underground(%f -> %f)\n", MB(GetGuid().str()), m_kLastFramePos.z, afterUpdatePos.z);
		//m_bTraceUpdate = true;
	}
	else if (m_kLastFramePos.z < 0.0f && afterUpdatePos.z > 0.0f)
	{
		NILOG(PGLOG_MINOR, "[PgActor] UpdatePhysX - %s actor pos z go upperground(%f -> %f)\n", MB(GetGuid().str()), m_kLastFramePos.z, afterUpdatePos.z);
		//m_bTraceUpdate = false;
	}
	else if (m_kLastFramePos.z > 0.0f && afterUpdatePos.z > 0.0f)
	{
		//m_bTraceUpdate = false;
	}

	m_kLastFramePos = afterUpdatePos;
#endif
	m_kLastFramePos = m_pkController->getPosition();
}
void	PgActor::UpdateBottomRayHit(NxVec3 const &kNewPosition)
{
	NxRay kRay(kNewPosition, NX_NEGATIVE_UNIT_Z);
	m_kBottomRayHit.shape = 0;
	GetWorld()->raycastClosestShape(kRay, NX_ALL_SHAPES, m_kBottomRayHit, m_uiActiveGrp, 10000.0f, NX_RAYCAST_SHAPE|NX_RAYCAST_FACE_NORMAL|NX_RAYCAST_DISTANCE|NX_RAYCAST_IMPACT);
}
/*
struct	stTimeCheck
{
	DWORD	m_dwAccumTime;
	DWORD	m_dwStartTime;

	stTimeCheck():
		m_dwAccumTime(0),
		m_dwStartTime(0)
	{
	};

	void	Start()
	{
		m_dwStartTime = BM::GetTime32();
	}
	void	End()
	{
		m_dwAccumTime += BM::GetTime32() - m_dwStartTime;
	}
	void	Reset()
	{
		m_dwStartTime = 0;
		m_dwAccumTime = 0;
	}
};*/

void	PgActor::UpdateWaveEffect(float fAccumTime)
{

	float	fDelay = GetPositionChanged() ? 0.3f : 1.0f;

	if(fAccumTime - m_fLastWaveEffectUpdateTime > fDelay)
	{
		m_fLastWaveEffectUpdateTime = fAccumTime;
		if(g_pkWorld)
		{
			g_pkWorld->UpdateWaveEffect(this,GetPositionChanged());
		}
	}
}
void	PgActor::CheckRidingObject()
{
	if(!m_pkPhysXScene)
	{
		return;
	}

	NxScene	*pkScene = m_pkPhysXScene->GetPhysXScene();
	if(!pkScene)
	{
		return;
	}

	PgObject	*pkFoundObject = NULL;


	NiPoint3	const	&kStart = GetPosition(true);
	if(IsMeetFloor() || (fabs(m_kBottomRayHit.worldImpact.z - kStart.z)<PG_CHARACTER_Z_ADJUST))
	{
		NiPoint3	const	kDir = NiPoint3::UNIT_Z*-1;
		float	const	fRange = 100.0f;

		NxRaycastHit kHit;
		NxRay kRay(NxVec3(kStart.x, kStart.y, kStart.z), NxVec3(kDir.x, kDir.y, kDir.z));
		NxShape *pkHitShape = GetWorld()->raycastClosestShape(kRay, NX_DYNAMIC_SHAPES, kHit, 1<<PG_PHYSX_GROUP_OBJECT, fRange);

		if(pkHitShape)
		{
			if(pkHitShape->userData)
			{
				pkFoundObject = (PgObject*)pkHitShape->userData;
			}
		}
	}


	if(pkFoundObject != m_pkMountedRidingObject && m_pkMountedRidingObject)
	{
		m_pkMountedRidingObject->DemountActor(this);
		m_pkMountedRidingObject = NULL;
	}

	if(pkFoundObject)
	{
		if(pkFoundObject->MountActor(this))
		{
			m_pkMountedRidingObject = pkFoundObject;
		}
	}
}


bool PgActor::UnmountPet(void)
{
	if(!GetPilot() || !GetPilot()->GetUnit() || !GetPilot()->GetUnit()->IsUnitType(UT_PLAYER))
	{
		return false;
	}
	if(m_pkMountedRidingPet == NULL)
	{
		return false;
	}
	NiNodePtr pkNodePetRidePt = NiDynamicCast(NiNode, m_pkMountedRidingPet->GetObjectByName(ATTACH_POINT_MOUNT_PET));
	NiAVObjectPtr pkNodeSceneRoot = GetNIFRoot();
	if(!pkNodeSceneRoot || !pkNodePetRidePt)
	{
		return false;
	}

	pkNodeSceneRoot = pkNodePetRidePt->DetachChild(pkNodeSceneRoot);
	if(!pkNodeSceneRoot)
	{
		return false;
	}
	AttachChild(pkNodeSceneRoot, false);
	pkNodeSceneRoot->SetTranslate(m_kSceneRootPos);
	NiActorManager* pkActorMng = GetActorManager();
	if(pkActorMng)
	{ //pc nif의 원래 scale 값을 복원
		NiAVObject* pkNifRoot = pkActorMng->GetNIFRoot();
		if(pkNifRoot)
		{
			pkNifRoot->SetScale(m_fScaleOrig);
		}
	}

	DetachNameNodes(m_pkMountedRidingPet->GetNIFRoot(), ATTACH_POINT_RIDENAME); //탑승용 이름노드 분리
	AttachNameNodes(pkNodeSceneRoot); //이름 노드 원상 복구
	m_pkMountedRidingPet->AttachNameNodes(m_pkMountedRidingPet->GetNIFRoot());

	ReserveTransitAction(ACTIONNAME_IDLE);
	m_pkMountedRidingPet->ReserveTransitAction(ACTIONNAME_TRACE2);

	NiNode::Update(0);
	m_pkMountedRidingPet->NiNode::Update(0);

	m_pkMountedRidingPet->MoveParticlesToTarget(this); //펫에게 붙은 파티클을 나에게로 이동..

	m_pkMountedRidingPet->SetPickupScript("Pet_Pickup");
	m_pkMountedRidingPet->m_pkMountedRidingPet = NULL;
	m_pkMountedRidingPet = NULL;
	PgPilot* pkPilot = g_kPilotMan.FindPilot(GetGuid());
	if(pkPilot == NULL)
	{
		return false;
	}
	pkPilot->SetRidingPet(false);

	//숨긴 부위를 다시 표시
	PgAction* pkAction = GetAction();
	if(pkAction)
	{
		HideParts(EQUIP_LIMIT_WEAPON, false);
	}

	lua_tinker::call<void, lwActor, bool>("SubActorHide", lwActor(this), true); //쌍둥이 복구

	return true;
}

void PgActor::SyncMountPet(void)
{ //다른 액터가 로딩될 때, 탑승 상태라면 여기서 탑승시킨다.
	CUnit* pkUnit = GetUnit();
	if(!pkUnit)
	{
		return;
	}
	switch(pkUnit->UnitType())
	{
	case UT_PLAYER:
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(!pkPlayer)
			{
				return;
			}
			PgActor* pkPetActor = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
			if(!pkPetActor)
			{
				return;
			}
			CUnit* pkPetUnit = pkPetActor->GetUnit();
			if(!pkPetUnit)
			{
				return;
			}
			if(IsRidingPet() || !pkPetUnit->GetAbil(AT_MOUNTED_PET_SKILL))
			{
				return;
			}
			MountPet();
		}break;
	case UT_PET:
		{
			PgActor* pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
			if(!pkCallerActor || IsRidingPet() || !pkUnit->GetAbil(AT_MOUNTED_PET_SKILL) )
			{
				return;
			}
			pkCallerActor->MountPet();
		}break;
	}

}

bool PgActor::MountPet(void)
{
	if(!GetPilot() || !GetPilot()->GetUnit() || !GetPilot()->GetUnit()->IsUnitType(UT_PLAYER) || IsRidingPet())
	{ //PC가 아니거나 이미 탑승 중이라면 그냥 종료
		return false;
	}
	//-------------Step1. 펫의 액터를 얻는다---------------------------------
	PgPilot* pkPilot = g_kPilotMan.FindPilot(GetGuid());
	if(pkPilot == NULL)
	{
		return false;
	}
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if(pkPlayer == NULL)
	{
		return false;
	}
	PgActor* pkActorPet = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
	if(pkActorPet == NULL || !IsCompleteLoadParts() || !pkActorPet->IsCompleteLoadParts()) //주인과 펫 모두 로딩이 완료되어 있어야만 한다
	{
		return false;
	}
	//-------------Step2. 탑승 가능한 펫인지 체크----------------------------
	PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
	if(pkPetPilot == NULL)
	{
		return false;
	}
	PgPet* pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
	if(pkPet == NULL || pkPet->UnitType() != UT_PET || pkPet->GetPetType() != EPET_TYPE_3)
	{ //펫이 없거나 라이딩 펫이 아니면
		return false; //MSG: 탑승할 수 있는 펫이 없습니다.
	}

	//--Step3. 상태 체크 (탈 수 있는 상태인가?)
//	PgAction* pkAction = GetAction();
//	if(!pkAction/* || !IsMeetFloor() || pkAction->GetActionType() != "IDLE"*/) //현재 지면에 서있거나 아이들 상태일때만 가능
//	{
//		return false; //MSG: 탑승할 수 없는 상태입니다.
//	}



	//탑승할 펫의 새 위치로 이름노드 붙임
	if(m_pTextBalloon) //MountPet() 함수가 두번 이상 호출될 경우를 대비해서 체크해야 한다.
	{
		m_pTextBalloon->SetEnable(false);
	}
	DetachNameNodes(GetNIFRoot()); //노드 분리전에 이름노드 분리
	pkActorPet->DetachNameNodes(pkActorPet->GetNIFRoot());
	AttachNameNodes(pkActorPet->GetNIFRoot(), ATTACH_POINT_RIDENAME);

	//--Step4. 컨트롤러 동기화(펫의 컨트롤러를 PC의 컨트롤러와 같이 강제로 위치 시킨다.)
	pkActorPet->SetPosition(GetPosition(true));
	pkActorPet->SetLookingDirection(GetDirection());

	//--Step5. 노드 붙이기(펫의 특정 노드에 캐릭터 루트 노드를 붙힌다)---
	NiNodePtr pkNodeSceneRoot = NiDynamicCast(NiNode, GetNIFRoot());
	if(!pkNodeSceneRoot)
	{
		return false;
	}
	NiAVObjectPtr pkObjCharRoot = pkNodeSceneRoot->GetObjectByName(ATTACH_POINT_PC_TO_PET);
	NiNodePtr pkNodePetRidePt = NiDynamicCast(NiNode, pkActorPet->GetObjectByName(ATTACH_POINT_MOUNT_PET));
	if(!pkObjCharRoot || !pkNodePetRidePt)
	{
		return false;
	}
	if(pkNodePetRidePt->GetWorldScale() == 0)
	{
		pkNodePetRidePt->SetWorldScale(1.f);
	}

	m_kSceneRootPos = pkNodeSceneRoot->GetTranslate();
	pkNodeSceneRoot->SetTranslate( pkObjCharRoot->GetTranslate() );

	float fScale = pkNodePetRidePt->GetWorldScale();
	NiActorManager* pkActorMng = GetActorManager();
	if(pkActorMng)
	{ //펫 xml에서 scale을 강제로 키웠다면 탑승 후 PC의 scale도 커지므로 탑승 전 pc의 scale을 미리 줄여준다.
		NiAVObject* pkNifRoot = pkActorMng->GetNIFRoot();
		if(pkNifRoot)
		{
			m_fScaleOrig = pkNifRoot->GetScale();
			pkNifRoot->SetScale(1.f / fScale);
		}
	}

	DetachChild(pkObjCharRoot);
	pkNodePetRidePt->AttachChild(pkNodeSceneRoot);

	m_pkMountedRidingPet = pkActorPet;
	m_pkMountedRidingPet->m_pkMountedRidingPet = this;

	NiNode::Update(0);
	m_pkMountedRidingPet->NiNode::Update(0);
	pkPilot->SetRidingPet(true);
	
	ReserveTransitAction(ACTIONNAME_RP_IDLE); //PC액션 아이들로..
	m_pkMountedRidingPet->ReserveTransitAction(ACTIONNAME_RIDING); //펫액션 아이들로..


	MoveParticlesToTarget(m_pkMountedRidingPet); //나에게 붙은 파티클을 펫에게로 이동..

	//무기를 감춘다.
	HideParts(EQUIP_LIMIT_WEAPON, true);
	m_pkMountedRidingPet->SetPickupScript("Actor_Pickup");

	lua_tinker::call<void, lwActor, bool>("SubActorHide", lwActor(this), true); //쌍둥이 숨김

	return true;
}

NiAVObjectPtr PgActor::GetCharRoot(void)
{ //CharRoot Node를 얻어야 한다면 반드시 이 함수에서 얻어야 한다. 펫 탑승 여부에 따라 참조 대상이 달라질 수 있기 때문이다
	return GetMainNIFRoot();
}

void	PgActor::CheckInvalidSpace()	//	Check whether the actor is standing on some impossible location.
{
	if(IsMyActor() == false || !m_pkController)
	{
		return;
	}

	NxVec3 vCurrentPos(static_cast<float>(m_pkController->getPosition().x)
					,static_cast<float>(m_pkController->getPosition().y)
					,static_cast<float>(m_pkController->getPosition().z)
					);

	if(NX_ZERO != m_vPrevControllerPos)
	{
		NxVec3	vDir = vCurrentPos - m_vPrevControllerPos;
		float	fDistance = vDir.normalize();

		if( 0.0f == fDistance )
		{
			return;
		}

		if(g_pkWorld)
		{
			PgUserRaycastReport	kHitReport;
			g_pkWorld->DetachAllParticle();

			NxU32 uCount1 = g_pkWorld->raycastAllShapes(NxRay(m_vPrevControllerPos,vDir),kHitReport,NX_STATIC_SHAPES,(1<<PG_PHYSX_GROUP_BASE_FLOOR),fDistance,NX_RAYCAST_SHAPE);

			for(unsigned int i=0;i<uCount1;++i)
			{
				NxVec3	const &vPos = kHitReport.GetHitReports().at(i).worldImpact;
				g_pkWorld->AttachParticle(g_kParticleMan.GetParticle("QuestNotify_Help"),NiPoint3(vPos.x,vPos.y,vPos.z));
			}
			NxU32 uCount2 = g_pkWorld->raycastAllShapes(NxRay(vCurrentPos,-vDir),kHitReport,NX_STATIC_SHAPES,(1<<PG_PHYSX_GROUP_BASE_FLOOR),fDistance,NX_RAYCAST_SHAPE);
			for(unsigned int i=0;i<uCount2;++i)
			{
				NxVec3	const &vPos = kHitReport.GetHitReports().at(i).worldImpact;
				g_pkWorld->AttachParticle(g_kParticleMan.GetParticle("QuestNotify_Ing"),NiPoint3(vPos.x,vPos.y,vPos.z));
			}

			if(uCount1 > uCount2)
			{

				BM::Stream	kPacket(PT_C_M_NOTI_DETECTION_HACKING);
				kPacket.Push(DHT_HIDE_IN_BOX);
				NETWORK_SEND(kPacket)

					TransitAction(GetReservedAction(RA_IDLE),true,0,DIR_NONE,true);
				SetPosition(NiPoint3(m_vPrevControllerPos.x,m_vPrevControllerPos.y,m_vPrevControllerPos.z));
				return;
			}
		}
	}

	m_vPrevControllerPos = vCurrentPos;
	
}

bool PgActor::Update(float fAccumTime, float fFrameTime)
{
	BM::CAutoMutex kLock(m_kMutex);

	//static	stTimeCheck	kTC[100];
	//static	DWORD	dwStartTime = BM::GetTime32();

	//kTC[0].Start();
	//kTC[1].Start();
	checkVisible();

	//CheckInvalidSpace();

	CheckItemUseTime();

	m_pkPhysXDest->SetOptimizeSleep(m_bIsOptimizeSleep);



	// 각 InvisibleGroup별 Update여부를 설정한다. frameTime과는 무관하게 진행된다.
	float fCurrentTime = NiGetCurrentTimeInSec();
	bool bUpdateThisFrame = false;
	float fModifiedFrameTime = fFrameTime;
	float fFrameTimes[MAX_NUM_INVISIBLE];
	for (int i = 0; i < MAX_NUM_INVISIBLE; ++i)
	{
		fFrameTimes[i] = fCurrentTime - m_kLastUpdateTimeByInvisibleGrade[i];
		if (fFrameTimes[i] >= ms_kUpdateIntervalByInvisibleGrade[i])
		{
			m_kCanUpdate[i] = true;
		}
		else
		{
			m_kCanUpdate[i] = false;
		}
	}

	if (g_bUseVariableActorUpdate)
	{
		bUpdateThisFrame = m_kCanUpdate[GetInvisibleGrade()];
		fModifiedFrameTime = fFrameTimes[GetInvisibleGrade()];
	}
	else
	{
		bUpdateThisFrame = true;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.Update"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerB(g_kActorStatGroup.GetStatInfo("PgActor.Update1"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerC(g_kActorStatGroup.GetStatInfo("PgActor.Update2"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerD(g_kActorStatGroup.GetStatInfo("PgActor.Update3"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerE(g_kActorStatGroup.GetStatInfo("PgActor.Update4"), g_pkApp->GetFrameCount()));

	//kTC[1].End();
	//kTC[2].Start();

	PG_STAT(timerB.Start());
	const	bool bIsMyActor = IsMyActor();
		
	//	애니메이션 타임 계산(속도 조절등에 의해 바뀔 수 있음)
	const	float	fAnimationAccumTime = CalcAnimationAccumTime(fAccumTime,fFrameTime);
	
	// 월드객체 공통 Update를 수행한다.
	if (bUpdateThisFrame)
	{
		PgIWorldObject::Update(fAccumTime, fModifiedFrameTime);
	}
	//kTC[2].End();
	//kTC[3].Start();
	//	충돌 대미지 체크 - 거리가 멀면 아예 안해도 된다.
	if (GetInvisibleGrade() <= PgActor::INVISIBLE_NEAR)
	{
		CheckTouchDmg();
	}
	PG_STAT(timerB.Stop());
	
	if(m_dwLastFrameTime == 0)
	{
		m_dwLastFrameTime = BM::GetTime32();
	}

	if(m_spSpotLightGeom)
	{
		NiAVObject	*pkLightGeom = m_spSpotLightGeom;
		PgParticle	*pkParticle = (PgParticle*)(pkLightGeom);
		if(pkParticle)
		{
			pkParticle->Update(fAccumTime,fFrameTime);
		}
	}

	UpdateWaveEffect(fAccumTime);
	//kTC[3].End();
	//kTC[4].Start();
	
	//	밀기
	UpdatePush(fFrameTime);

	PG_STAT(timerC.Start());
	//if (bUpdateThisFrame || m_kCanUpdate[PgActor::INVISIBLE_MIDDLE])
	{
		bool bUpdateDefaultProcess = true;
		if(IsNowFollowing())
		{
			bUpdateDefaultProcess = ProcessFollowingActor();
		}
		else if(!bIsMyActor)
		{
			bUpdateDefaultProcess = (IsSync() ? UpdateSync(fModifiedFrameTime) : ProcessActionQueue());
		}

		if(bUpdateDefaultProcess)
		{
			int iNextActionNo = 0;
			if( SkillSetAction().GetReservedAction(iNextActionNo, this) )
			{
				//ReserveTransitAction(iNextActionNo);
				TryReserveActionToMyActor(iNextActionNo);
			}

			//	예약된 액션 전환을 처리한다.
			DoReservedTransitAction();
			//kTC[4].End();
			//kTC[5].Start();

			//! 엑터 FSM을 갱신한다.
#ifndef EXTERNAL_RELEASE
			if(lua_tinker::call<bool>("UpdateActorFSM"))
#endif
			{
				if(m_pkAction)
				{
					m_pkAction->UpdateFSM(this, fAccumTime, fModifiedFrameTime);
				}
			}
			//kTC[5].End();

			//kTC[6].Start();
			// PhysX를 업데이트 한다.
#ifndef EXTERNAL_RELEASE
			if(lua_tinker::call<bool>("UpdateActorPhysX"))
#endif
			{
				UpdatePhysX(fAccumTime, fModifiedFrameTime);
			}
			//kTC[6].End();
		}
	}
	//kTC[7].Start();
	// 등록된 스크립트가 있다면, 업데이트 한다.
	if(!GetUpdateScript().empty())
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);		
		lua_tinker::call<bool, lwActor, float, float>(MB(GetUpdateScript().c_str()), lwActor(this), fAccumTime, fFrameTime);
		NIMETRICS_ENDTIMER(a);
	}
	PG_STAT(timerC.Stop());

	PG_STAT(timerD.Start());

	PickUpNearItem();
	
	UpdateTrigger();	// 물려있는 액션 조건 트리거를 발동한다.
	//kTC[7].End();
	//kTC[8].Start();

	// 에니메이션을 갱신한다, 캐릭터가 컬링 되어 있을 때는 일정 시간마다 업데이트를 해준다.
	if (bUpdateThisFrame)
	{
		UpdateActorManager(fAnimationAccumTime);
	}
	//kTC[8].End();
	//kTC[9].Start();
	
	DoLoadingFinishWork();	//	로딩 끝났을 때의 처리
	// 예약된 Item들을 Equip 한다.
	UpdateItemEquip();

	//kTC[9].End();
	//kTC[10].Start();

	//! 액션 레이어가 있으면 업데이트 한다.
	//if(m_kActionLayerContainer.size())
	//{
	//	ActionLayerContainer::const_iterator itr = m_kActionLayerContainer.begin();
	//	while(itr != m_kActionLayerContainer.end())
	//	{
	//		(*itr)->UpdateFSM(this, fAccumTime, fFrameTime);
	//		++itr;
	//	}
	//}

	PG_STAT(timerD.Stop());

	PG_STAT(timerE.Start());
	if(m_pkActionEffectStack && !m_pkActionEffectStack->IsEmpty())
	{
		m_pkActionEffectStack->Update(fAccumTime,fFrameTime);
	}

	if (bUpdateThisFrame)
	{
		
		TransitActorSpecular(fModifiedFrameTime);	//	색깔 바꾸기
		TransitAlpha(fModifiedFrameTime);	//	투명도 바꾸기
		TransitColor(fModifiedFrameTime);	//	투명도 바꾸기
		UpdateBlink(fModifiedFrameTime);// 캐릭터가 깜박거려야 한다면
		//UpdateTwist();// 캐릭터가 빙글빙글 돌아야 한다면
	}
	
	UpdateSkillInfos();

	//	스케일링 처리
	if(bUpdateThisFrame)
	{
		UpdateScale();
	}

	// 노드알파 처리
	if(bUpdateThisFrame)
	{
		UpdateNodeAlpha();
	}

	//	머리크기
	if(bUpdateThisFrame)
	{
		UpdateHeadSize(fFrameTime,fAccumTime);
	}

	// 이펙트 카운드 다운
	UpdateEffectCountDonw(fFrameTime);
	
	UpdateGodTime(fAccumTime);	//	무적 타임
	UpdateDamageBlink(fAccumTime);//	대미지 블링크

	CUnit * pkUnit = GetPilot() && GetPilot()->GetUnit() ? GetPilot()->GetUnit() : NULL;
	if(pkUnit)
	{
		unsigned long const ulElapsedTime = g_pkApp ? g_pkApp->GetEventViewElapsed() : 0;

		pkUnit->AutoGroggy(ulElapsedTime);

	  	if(pkUnit->IsUnitType(UT_SUMMONED))
	    {
		    pkUnit->AutoHeal(ulElapsedTime);
	    }
	}

	//kTC[10].End();
	//kTC[11].Start();

	PG_STAT(timerE.Stop());
	//kTC[11].End();
	//kTC[12].Start();

	// Update시간 기록.
	for (int i = 0; i < MAX_NUM_INVISIBLE; ++i)
	{
		if (m_kCanUpdate[i])
		{
			m_kLastUpdateTimeByInvisibleGrade[i] = fCurrentTime;
		}
	}
	//kTC[12].End();

	if(m_fAutoDeleteActorTime != 0.0f)
	{
		float fElapsedTime = fAccumTime - m_fAutoDeleteActorStartTime;
		if(m_fAutoDeleteActorTime < fElapsedTime)
		{
			GetWorld()->RemoveObjectOnNextUpdate((BM::GUID)GetPilotGuid());
			m_fAutoDeleteActorTime = 0.0f;
		}
	}

	if(IsMyActor() && g_pkWorld)
	{
		if(GetAction() && GetAction()->GetActionOptionEnable(PgAction::AO_DISABLE_SMALLAREACHECK) == false)
		{
			g_pkWorld->UpdateSmallAreaIndex(GetPosition());
		}
	}

	// 마지막 프레임 시간을 구한다.
	m_dwLastFrameTime = BM::GetTime32();
	//kTC[0].End();

	//DWORD	dwElapsedTime = BM::GetTime32() - dwStartTime;
	//if(dwElapsedTime>10000)
	//{
	//	dwStartTime = BM::GetTime32();

	//	int	iTotalTC = 13;
	//	char	msg[5000];
	//	sprintf_s(msg,5000,"[Actor] ");
	//	for(int i=0;i<iTotalTC;i++)
	//	{
	//		char	msg2[100];
	//		sprintf_s(msg2," T[%d]%f",i,kTC[i].fElapsed);
	//		strcat(msg,msg2);

	//		kTC[i].Reset();
	//	}
	//	strcat(msg,"\n");
	//	_PgOutputDebugString(msg);
	//}
	m_bIsOptimizeSleep = true;	//다음번 프레임에는 무조건 다시 켜자

	UpdateStatusEffect(fAccumTime, fFrameTime);


	//탑승 중이면, 펫의 컨트롤러를 PC의 컨트롤러와 일치시킨다 (이동, 방향)
	if(m_pkMountedRidingPet != NULL && GetPilot() && GetPilot()->GetUnit() && GetPilot()->GetUnit()->IsUnitType(UT_PLAYER))
	{ //펫의 경우, 이 루틴을 타면 안된다.
		m_pkMountedRidingPet->SetPosition(GetPosition(true));
		NiPoint3 kDir = GetLookingDir();
		m_pkMountedRidingPet->ConcilDirection(kDir);
		//NiNode::Update(0);
		//m_pkMountedRidingPet->PgActor::Update(fAccumTime, fFrameTime);
	}

	return true;
}

void PgActor::UpdateRidingInfo(float fAccumTime, float fFrameTime)
{
	int	iTotalSrc = m_vKinematicSrcCont.size();
	for(int i=0;i<iTotalSrc;++i)
	{
		NiPhysXKinematicSrc	*pkSrc = m_vKinematicSrcCont[i];
		NiAVObject	*pkGBSource = pkSrc->GetSource();

		if(!pkGBSource)
		{
			continue;
		}

		NxActor	*pkTarget = pkSrc->GetTarget();
		if(!pkTarget)
		{
			continue;
		}

		NxMat34	&kGlobalPose = pkTarget->getGlobalPose();

		kGlobalPose.t.x = pkGBSource->GetWorldTranslate().x;
		kGlobalPose.t.y = pkGBSource->GetWorldTranslate().y;
		kGlobalPose.t.z = pkGBSource->GetWorldTranslate().z;

		pkTarget->setGlobalPose(kGlobalPose);
	}
}
float	PgActor::CalcAnimationAccumTime(float fAccumTime,float fFrameTime)
{
	float	fRealFrameTime = 0;
	if(m_fBeforeAccumTime > 0)
	{
		fRealFrameTime = fAccumTime - m_fBeforeAccumTime;
	}
	m_fBeforeAccumTime = fAccumTime;

	float	fAnimationAccumTime = fAccumTime;
	float	fAnimationFrameTime = fFrameTime;

	if(m_fTotalAnimSpeedControlTime>0 || m_fTotalAnimSpeedControlTime == -1)
	{
		float	fElapsedTime = fAccumTime - m_fAnimSpeedControlStartTime;
		if(m_fTotalAnimSpeedControlTime != -1 && fElapsedTime>m_fTotalAnimSpeedControlTime)
		{
			m_fTotalAnimSpeedControlTime = 0;
		}	
		else
		{
			m_fAccumTimeAdjust -= (fRealFrameTime - fRealFrameTime*m_fAnimSpeedControlValue);
			fAnimationFrameTime *=m_fAnimSpeedControlValue;
		}
	}

	fAnimationAccumTime += m_fAccumTimeAdjust;

	return	fAnimationAccumTime;
}
void	PgActor::UpdateTrigger()
{
	if( m_pkAction 
		&& m_pkCurrentTrigger 
		&& !m_bCurrentTriggerAct 
		&& PgTrigger::TRIGGER_TYPE_JOB_SKILL != m_pkCurrentTrigger->GetTriggerType() 
		)
	{
		if( PgTrigger::CT_ACTION == m_pkCurrentTrigger->GetConditionType()
		&&	(m_pkCurrentTrigger->GetConditionAction() == m_pkAction->GetID() || GetActionState(m_pkCurrentTrigger->GetConditionAction().c_str())) )
		{
			m_pkCurrentTrigger->OnAction(this);
			m_bCurrentTriggerAct = true;
		}
	}
}
void	PgActor::UpdateActorManager(float fAnimationAccumTime)
{
#ifndef EXTERNAL_RELEASE
	if(lua_tinker::call<bool>("UpdateActorManager"))
#endif
	{
		float fTime = fAnimationAccumTime;
		if(GetAnimationStartTime() > 0.0f)
		{
			fTime = fAnimationAccumTime - GetAnimationStartTime();
		}
		NiActorManager* pkActorMgr =  GetActorManager();
		PG_ASSERT_LOG(pkActorMgr);
		if(pkActorMgr)
		{
			pkActorMgr->Update(fTime);
		}

		// 파츠별로 애니가 있다면, 플레이 한다.
		AMContainer::iterator itr = m_kSupplementAMContainer.begin();
		while(itr != m_kSupplementAMContainer.end())
		{
			PG_ASSERT_LOG(itr->m_spAM);
			if (itr->m_spAM)
			{
				itr->m_spAM->Update(fTime);

			}
			++itr;
		}
	}
}
void	PgActor::SetSpotLightColor(int Red,int Green,int Blue)
{
	if(!m_spSpotLightGeom)
	{
		return;
	}

	m_spSpotLightGeom->SetColorLocal(NiColorA(Red/255.0f,Green/255.0f,Blue/255.0f,1.0f));
}

void	PgActor::DoLoadingFinishWork()
{
	// 캐릭터 로딩이 끝났음!! [캐릭터 로딩이 끝나면 파츠를 보여주고, 알파를 먹인다]
	if(!m_iEquipCount && !m_bLoadingComplete)
	{
		bool	const	bIsMyActor = IsMyActor();

		m_bLoadingComplete = true;
		NILOG(PGLOG_LOG, "[PgActor] %s actor loading all items, %f time elasped\n", MB(GetGuid().str()), NiGetCurrentTimeInSec() - m_fLoadingStartTime);
		m_fLoadingStartTime = 0.0f;

		NiActorManager *pkAM = GetActorManager();

		// 로딩이 끝났으므로, 몸체를 다시 보여준다.
		pkAM->GetNIFRoot()->SetAppCulled(false);
		
		_PgOutputDebugString("[PgActor::Update] Unhide Parts because Loading Completed Actor : %s\n",MB(GetPilotGuid().str()));

		// 숨겨뒀던 파츠를 보이게 한다.
		//for(PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin();
		//	itr != m_kPartsAttachInfo.end();
		//	++itr)
		//{
		//	HideParts(itr->first, false);
		//}
		
		// 모든 Material을 읽어서 저장해 둔다.
		m_kMaterialContainer.clear();
		m_kSpecularContainer.clear();
		m_bMaterialCached = FindMaterialProp(this, false, true);

		AttachActorAlphaProperty(this);
		UpdateProperties();

		if(m_AlphaTransitInfo.bUseLoadingFinishInit)
		{
			m_AlphaTransitInfo.m_fTargetAlpha = 0.0f;
			if ( UseSmoothShow() > 0.0f )
			{
				// 변수값이 이상해...
				SetTargetAlpha( GetAlpha(), UseSmoothShow(), m_AlphaTransitInfo.m_fAlphaTransitionTime);
			}
			else
			{
				SetTargetAlpha( GetAlpha(), 1.0f, 0.15f);
			}
		}

		//	상태 이상 적용
		PgPilot* pkPilot = GetPilot();
		if( pkPilot )
		{
			g_kStatusEffectMan.ReAddEveryEffect(pkPilot);
			g_kStatusEffectMan.DoReserved(pkPilot);
		}

		ApplyHidePartsAll();

		// Recv_PT_M_C_NFY_MAPLOADED(..) 함수로 이전하였다..
		// 이유 : GameTime 정보가 그때 오기 때문
		//if(IsMyActor())
		//{
		//	ResetSkillCoolTimeFromUnit();
		//}

		PgPlayer* pkPC = NULL;

		if(pkPilot)
		{
			pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if( pkPilot && g_pkWorld
			&&	g_kPilotMan.IsMyPlayer(pkPilot->GetGuid())
			&&	lua_tinker::call<bool, int, lwUnit>("IsCanAutoAddHelper", g_pkWorld->MapNo(), lwUnit(pkPC)) )
			{
				lua_tinker::call<void>("CreateHelper");
			}
		}

		if( BM::GUID::NullData() != g_kParty.PartyGuid()
		&&	pkPilot
		&&	pkPC
		&&	g_kParty.PartyGuid() == pkPC->PartyGuid() ) // 내 파티원이면 파티 서클 붙인다.
		{
			PgClientPartyUtil::AttachPartyCircle( pkPC->GetID() );
		}

		if(pkPilot)
		{
			g_kMapMoveCompleteEventMgr.Pop( pkPilot->GetGuid() );	// 맵 이동이후 연출 요소
		}

		if(bIsMyActor)
		{
			PgParticle	*pkParticle = g_kParticleMan.GetParticle("e_spotlight");
			if(g_pkWorld && pkParticle)
			{
				NiAVObject	*pkAttachNode = GetObjectByName("char_root");
				if(pkAttachNode)
				{
					pkParticle->SetParticleProcessor(NiNew PgParticleProcessorAttachToNode(pkAttachNode));
					g_pkWorld->AddSpotLightCaster(pkParticle);

					m_spSpotLightGeom = (NiAVObject*)pkParticle;
				}
			}
			PgPilot* pkPilot = GetPilot();
			if(pkPilot)
			{
				UpdateLowHPWarnning(0, pkPilot->GetAbil(AT_HP));
			}
		}

		//	월드에 SpotLight 가 적용되지 않은 상태라면, 캐릭터의 GlowMap 을 제거한다.
		if(g_pkWorld)
		{
			PgRenderer::EnableGlowMap(this,g_pkWorld->GetSpotLightOn() || AlwaysGlowMap());
		}
		
		if(!m_kLoadingCompleteInitFunc.empty())
		{
			lua_tinker::call<void,lwActor>(m_kLoadingCompleteInitFunc.c_str(),lwActor(this));
		}

		if( g_pkWorld 
			&& g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE) )
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if( pkUnit->IsInUnitType(UT_MONSTER) )
			{
				NiAVObject *pkParticle = g_kParticleMan.GetParticle("hero_certificate_mon");
				if( pkParticle )
				{
					int const iAttachToBaseNo = 121212;
					if( !AttachTo(iAttachToBaseNo, "p_ef_star", pkParticle) )
					{
						THREAD_DELETE_PARTICLE(pkParticle);
					}
				}
			}
		}

		//PgRenderer::ChangeShader(this,"FXSkinning");
        NewWare::Scene::ApplyTraversal::Geometry::OptimizeActorSkins( this, PgRenderer::GetBonesPerPartition() );
				
		if(m_pkActorAppearanceMan)
		{
			m_pkActorAppearanceMan->UpdateAppearance(true);
		}

		int const iMonEnchantGradeNo = m_pkPilot->GetAbil(AT_MON_ENCHANT_GRADE_NO);
		if( iMonEnchantGradeNo )
		{
			CONT_DEF_MONSTER_ENCHANT_GRADE const* pkDefMonEnchantGrade = NULL;
			g_kTblDataMgr.GetContDef(pkDefMonEnchantGrade);
			CONT_DEF_MONSTER_ENCHANT_GRADE::const_iterator find_iter = pkDefMonEnchantGrade->find(iMonEnchantGradeNo);
			if( pkDefMonEnchantGrade->end() != find_iter )
			{
				CONT_DEF_MONSTER_ENCHANT_GRADE::mapped_type const& rkMonEnchantGrade = (*find_iter).second;
				for( int iCur=0; MAX_MONSTER_ENCHANT_EFFECT_COUNT > iCur; ++iCur )
				{
					std::string const& rkEffectID = rkMonEnchantGrade.akEffectName[iCur];
					std::string const& rkRootName = rkMonEnchantGrade.akEffectRoot[iCur];
					if( !rkEffectID.empty()
					&&	!rkRootName.empty() )
					{
						NiAVObject *pkParticle = g_kParticleMan.GetParticle(rkEffectID.c_str(), PgParticle::O_SCALE, GetEffectScale());
						if( pkParticle )
						{
							int const iAttachToBaseNo = 200000;
							if( !AttachTo(iAttachToBaseNo + iCur, rkRootName.c_str(), pkParticle) )
							{
								THREAD_DELETE_PARTICLE(pkParticle);
							}
						}
					}
				}
			}
		}

		UpdateQuestDepend();


		SyncMountPet(); //다른 Player가 로딩되면 탑승을 동기화 시켜야 한다.
		SyncActionOnAddUnit(pkPC); //이동 동기화 (AddUnit() -> 로딩완료 후)

	}
}
void	PgActor::NfyActorManagerChanged(bool bBefore)
{
	if(bBefore)
	{
		RestoreTexture();

		//	임시로 붙인 알파프로퍼티를 떼어낸다.
		DetachActorAlphaProperty(this);
		UpdateProperties();

		RestoreSpecular();	//	스페큘러 복귀
		if(m_pkActorCallback)
		{
			GetActorManager()->SetCallbackObject(NULL);
		}
	}
	else
	{
		m_kMaterialContainer.clear();
		m_kSpecularContainer.clear();
		m_bMaterialCached = FindMaterialProp(this, false, true);

		AttachActorAlphaProperty(this);
		UpdateProperties();

		if(m_pkActorCallback)
		{
			GetActorManager()->SetCallbackObject(m_pkActorCallback);
		}

		NiTimeController::StartAnimations(GetNIFRoot(), 0.0f); //PgWorld에 Attach될 때 알아서 된다.

		//	상태 이상 적용
		PgPilot* pkPilot = GetPilot();
		if( pkPilot )
		{
			g_kStatusEffectMan.ReAddEveryEffect(GetPilot());
		}

		//	월드에 SpotLight 가 적용되지 않은 상태라면, 캐릭터의 GlowMap 을 제거한다.
		if(g_pkWorld)
		{
			PgRenderer::EnableGlowMap(this,g_pkWorld->GetSpotLightOn() || AlwaysGlowMap());
		}

		if (m_VarTextureList.size() > 0)
		{
			ChangeTexture(this);
		}

        NewWare::Scene::ApplyTraversal::Geometry::OptimizeActorSkins( this, PgRenderer::GetBonesPerPartition() );

		ResetAnimation();
	}

}
void	PgActor::NfyActorManagerLoadingComplete()
{
	PgIWorldObject::NfyActorManagerLoadingComplete();

	//	그림자 붙이기
	if(PgCircleShadow::AttachCircleShadowRecursive(this,200.0f,0.7f,0,0,&m_kBottomRayHit) == 0)
	{
		PgCircleShadow::AttachCircleShadowRecursive(this,200.0f,0.7f,0,this,&m_kBottomRayHit);
	}

	if(m_bShadowHide)
	{
		SetHideShadow(m_bShadowHide);
	}
}
void PgActor::ResetSkillCoolTimeFromUnit()
{
	m_SkillCoolTimeInfo.Reset();
	if(!m_pkPilot)
	{
		return;
	}
	PgControlUnit* pkUnit = static_cast<PgControlUnit*>(m_pkPilot->GetUnit());
	if(!pkUnit)
	{
		return;
	}

	CSkill* pkSkill = pkUnit->GetSkill();
	if(!pkSkill)
	{
		return;
	}

	DWORD dwServerTime = g_kEventView.GetServerElapsedTime();

	CSkill::CONST_COOLTIME_INDEX kItor;
	pkSkill->GetFirstCoolTime(kItor);

	CSkill::SCoolTimeInfo kInfo;
	while(pkSkill->GetNextCoolTime(kItor,kInfo))
	{
		if(0 == kInfo.iSkillNo)
		{
			continue;
		}

		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(kInfo.iSkillNo);
		if(!pkSkillDef) 
		{
			continue;
		}

		int	iCoolTime = pkSkillDef->GetAbil(ATS_COOL_TIME);
		int	iAddCoolTime =0;

		// 쿨타임이 있는 스킬만 추가 쿨타임을 적용
		if(0 < iCoolTime)
		{
			if(m_pkPilot)
			{
				if(m_pkPilot->GetUnit())
				{
					if(m_pkPilot->GetUnit()->GetSkill())
					{
						iAddCoolTime = m_pkPilot->GetUnit()->GetSkill()->GetSkillCoolTime();
					}
				}
			}

			if(iAddCoolTime > iCoolTime)
			{
				iAddCoolTime = 0;
			}

			iCoolTime += iAddCoolTime;
		}		

		//_PgOutputDebugString("#### Skill CoolTime ServerTime[%I64d], CoolTime[%I64d]\n", ui64ServerTime, kInfo.u64CoolTime);
		int	const iRemainCoolTime = ((kInfo.dwCoolTime < dwServerTime) ? 0 : static_cast<int>(kInfo.dwCoolTime - dwServerTime));

		m_SkillCoolTimeInfo.m_CoolTimeInfoMap.insert(
			std::make_pair(	
			g_kSkillTree.GetKeySkillNo(kInfo.iSkillNo),
			stSkillCoolTimeInfo::stCoolTimeInfoNode(kInfo.iSkillNo,BM::GetTime32()-(iCoolTime-iRemainCoolTime),iCoolTime))
			);
	}


}
void	PgActor::UpdateItemEquip()
{
	AttachItemInfo kInfo;
	
	m_kAttachItemLock.Lock();
	int	iAttachItemContainerSize = m_kAttachItemContainer.size();
	if (iAttachItemContainerSize > 0) // && m_AlphaTransitInfo.m_fAlphaTransitionTime == 0.0f)
	{
		kInfo = m_kAttachItemContainer.front();
		if (kInfo.pItem != NULL)
			m_kAttachItemContainer.pop_front();
	}
	m_kAttachItemLock.Unlock();
	
	if(iAttachItemContainerSize>0 && kInfo.pItem != NULL)
	{
		if (EquipItem(kInfo.pItem, kInfo.iItemNo, kInfo.bSetDefaultItem) == false)
		{
			THREAD_DELETE_ITEM(kInfo.pItem);
		}
	}
}
void	PgActor::UpdateBlink(float fModifiedFrameTime)
{
	if(m_bBlink)
	{
		m_fBlinkAccumTime += fModifiedFrameTime;
		
		if(m_fBlinkAccumTime > m_fBlinkFreq)
		{
			m_fBlinkAccumTime = 0.0f;
			SetBlinkHide(!IsBlinkHide());
		}
	}
}

//void	PgActor::UpdateTwist()
//{
//	if(m_iTwistTimes > 0)
//	{
//		NiTransform kLocalTransform = GetNIFRoot()->GetLocalTransform();
//		NiMatrix3 &kMat = kLocalTransform.m_Rotate;
//		kMat.MakeZRotation(m_fTwistDegree);
//		m_fTwistDegree += (float)(0.2 * m_fTwistSpeed);
//		if(m_fTwistDegree > NI_TWO_PI)
//		{
//			m_fTwistDegree -= NI_TWO_PI;
//			--m_iTwistTimes;
//		}
//		GetNIFRoot()->SetLocalTransform(kLocalTransform);
//	}
//}

void	PgActor::UpdateHeadSize(float fFrameTime,float fAccumTime)	//	머리크기 업데이트
{
	if(m_fCurrentHeadSize != m_fTargetHeadSize)
	{
		if(m_fCurrentHeadSize<m_fTargetHeadSize)
		{
			m_fCurrentHeadSize+=m_fHeadSizeTransitSpeed*fFrameTime;
			if(m_fCurrentHeadSize>=m_fTargetHeadSize)
				m_fCurrentHeadSize = m_fTargetHeadSize;
		}
		else
		{
			m_fCurrentHeadSize-=m_fHeadSizeTransitSpeed*fFrameTime;
			if(m_fCurrentHeadSize<=m_fTargetHeadSize)
				m_fCurrentHeadSize = m_fTargetHeadSize;
		}
		if(GetCharRoot())
		{
			NiAVObject	*pkHead = GetCharRoot()->GetObjectByName("Bip01 Head");
			if(pkHead)
			{
				pkHead->SetScale(m_fCurrentHeadSize);
			}
			UpdateName();
		}
	}
}
void	PgActor::UpdateGodTime(float fAccumTime)
{
	if(m_kGodTimeInfo.m_bGodTime)
	{
		float	fElapsedTime = fAccumTime - m_kGodTimeInfo.m_fStartTime;
		if(fElapsedTime>=m_kGodTimeInfo.m_fTotalTime)
		{
			StartGodTime(0);	//	0 은 곧 중지하라는 의미이다.
		}
	}

}
void	PgActor::UpdateDamageBlink(float fAccumTime)
{
	if(m_kDamageBlinkInfo.bEnable)
	{
		float	fElapsedTime = fAccumTime - m_kDamageBlinkInfo.m_fStartTime;
		if(fElapsedTime>m_kDamageBlinkInfo.m_fTotalTime)
		{
			StartDamageBlink(false);
		}
		else
		{
			fElapsedTime = fAccumTime - m_kDamageBlinkInfo.m_fLastTime;
			if(fElapsedTime>m_kDamageBlinkInfo.m_fPeriod)
			{
				m_kDamageBlinkInfo.m_fLastTime = fAccumTime;
				m_kDamageBlinkInfo.m_bToggle = !m_kDamageBlinkInfo.m_bToggle;

				if(m_kDamageBlinkInfo.m_bToggle)
				{
					g_kStatusEffectMan.AddStatusEffectToActor(GetPilot(), GetPilot(), GetDamageBlinkID(),200000002,0,1,true,false);
				}
				else
				{
					g_kStatusEffectMan.RemoveStatusEffectFromActor(GetPilot(),200000002);
				}
			}
		}
	}
}
void	PgActor::UpdateScale()
{
	if (m_ulScaleChangeStartTime>0)
	{
		unsigned	long	ulElapsedTime = BM::GetTime32() - m_ulScaleChangeStartTime;
		float	fRate = (float)ulElapsedTime / (float)m_ulTotalScaleChangeTime;
		if(fRate>1) fRate =1;
		if(fRate<0) fRate = 0;

		float	fNewScale = m_fStartScale + (m_fTargetScale-m_fStartScale)*fRate;
		GetNIFRoot()->SetScale(fNewScale);
		if(fRate == 1) m_ulScaleChangeStartTime = 0;
	}
}
void	PgActor::UpdatePush(float fFrameTime)
{
	if(m_Push.m_bActivated)
	{
		float	fDistance = m_Push.m_fVelocity*fFrameTime;;
		m_Push.m_fDistance -= fDistance;
		m_Push.m_fVelocity += m_Push.m_fAccel*fFrameTime;
		if(m_Push.m_fDistance<=0)
		{
			fDistance += m_Push.m_fDistance;
			std::max(fDistance,0.0f);
			m_Push.m_bActivated = false;
			m_Push.m_fVelocity = fDistance;
		}

		NiPoint3 kMovingDirNew = m_Push.m_kDir;
		if(NiPoint3::ZERO==m_Push.m_kDir)
		{
			NiPoint3	const	&kPathNormal = GetPathNormal();
			NiPoint3	kRightDir = kPathNormal.Cross(NiPoint3::UNIT_Z);
			kRightDir.Unitize();

			NiPoint3	kLeftDir = kPathNormal.Cross(-NiPoint3::UNIT_Z);
			kLeftDir.Unitize();
			kMovingDirNew = m_Push.m_bLeft ? kLeftDir : kRightDir;
		}

		NxVec3 kDelta;

		//NiPoint3 const &kMovingDirNew = m_Push.m_bLeft ? kLeftDir : kRightDir;

		NxVec3 kMovingDir;
		NiPhysXTypes::NiPoint3ToNxVec3(kMovingDirNew, kMovingDir);

		kMovingDir.z = 0.0f;
		kMovingDir.normalize();
		kDelta = kMovingDir * m_Push.m_fVelocity;
		
		SetMovingDelta(m_kMovingDelta+kDelta);

		//_PgOutputDebugString("Moving Delta PushAct: %.f, %.f, %.f\n", m_kMovingDelta.x, m_kMovingDelta.y, m_kMovingDelta.z);
	}
}

//bool PgActor::IsMyPet()
//{
//	return g_kPilotMan.IsMyPlayer(m_kMasterGuid);
//}
//
//bool PgActor::IsPet()
//{
//	if(m_kMasterGuid == BM::GUID())
//	{
//		return false;
//	}
//	return true;
//}

//! 파츠를 액터에 붙인다.
int PgActor::AttachSkinningParts(PgItemEx *pkParts)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.AttackSkinningParts"), g_pkApp->GetFrameCount()));
	NiNode* pkAttachSrcRoot = (NiNode*)pkParts->GetMeshRoot();
	if(GetPilot())
	{
		_PgOutputDebugString("DetachParts Actor:%s From PgActor::AttachSkinningParts\n",MB(GetPilot()->GetName()));
	}

	DetachParts(pkParts->EquipLimit());

	// Attach된 Node가 1개 이상이면.. 성공
	return SkinningParts(pkParts, pkAttachSrcRoot, true);
}

void	PgActor::CheckTouchDmg()
{
	if(!IsMyActor()) {return;}//내 액터가 아닐경운 할 필요없음
	if(!m_pkController) {return;}
	if(!g_pkWorld) {return;}
	if(!g_pkWorld->GetPhysXScene()) {return;}
	if(!GetPilot()) {return;}
	CUnit *pkActorUnit = GetPilot()->GetUnit();
	if(!pkActorUnit ) {return;}
	if( pkActorUnit->IsUnitType(UT_PLAYER) == false ) {return;}
	if(GetGroupNo() != OGT_PLAYER) {return;}
	if(GetCanHit() == false) {return;}
	if(IsDownState()) {return;}
	//if(IsIgnoreEffect(ACTIONEFFECT_DMG)) {return;}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CheckTouchDmg"), g_pkApp->GetFrameCount()));

	NxScene *pkScene = g_pkWorld->GetPhysXScene()->GetPhysXScene();
	if(!pkScene) {return;}

	static int const nbMaxShapes = 100;
	NxShape	*pkCollidedShapes[nbMaxShapes];

	NxCapsule kCapsule;
	NxExtendedVec3 const kPos = m_pkController->getDebugPosition();
	NxReal const fHeight = m_pkController->getHeight();
	NxReal const fRadius = m_pkController->getRadius();

	kCapsule.p0.x = kCapsule.p1.x = static_cast<NxReal>(kPos.x);
	kCapsule.p0.y = kCapsule.p1.y = static_cast<NxReal>(kPos.y);
	kCapsule.p0.z = kCapsule.p1.z = static_cast<NxReal>(kPos.z);
	kCapsule.p0.y -= fHeight * 0.5f;
	kCapsule.p1.y += fHeight * 0.5f;
	kCapsule.radius = fRadius;	

	static int const iOGT = (1<<(OGT_MONSTER+1))|(1<<(OGT_ENTITY+1));

	int const iTotalShapes = GetWorld()->overlapCapsuleShapes(kCapsule,
		NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,
		NULL,iOGT,NULL,true);

	NxShape *pkHitShape = NULL;
	for(int i=0;i<iTotalShapes;++i)
	{
		pkHitShape = pkCollidedShapes[i];
		if (pkHitShape && pkHitShape->userData)
		{
			PgActor* pkActor = reinterpret_cast<PgActor*>(pkHitShape->userData);//(PgActor*)pkHitShape->userData;
			PgPilot	*pkPilot = pkActor->GetPilot();

			if(pkActor == this) {continue;}
			if(pkActor->IsDownState()) {continue;}
			if(pkActor->GetAction() && pkActor->GetAction()->GetID()=="a_die") {continue;}
			if(!pkPilot)
			{
				continue;
			}
			if(/*pkPilot->GetAbil(AT_GRADE) != EMGRADE_ELITE &&
				pkPilot->GetAbil(AT_GRADE) != EMGRADE_BOSS &&*/
				pkPilot->GetAbil(AT_COLLISION_SKILL) == 0)
			{
				continue;
			}

			//특정 충돌구만 검사
			if(pkPilot->GetAbil(AT_USE_PART_CHECK_COLLISION))
			{
				char const* shapename = pkHitShape->getName();
				if(shapename)
				{
					int	const iABVShapeIndex = pkActor->GetABVShapeIndex(shapename) + 1; //ABVShapeIdx는 0이 나올수도 있어 Abil은 +1하여 등록
					bool bPartCollition = false;
					for(int i=0; i<PG_MAX_NB_ABV_SHAPES; ++i)
					{
						int const iPartCollition = pkPilot->GetAbil(AT_PART_CHECK_COLLISION_01+i);
						if(0==iPartCollition)
						{
							break;
						}
						if(iABVShapeIndex==iPartCollition)
						{
							bPartCollition = true;
							break;
						}
					}
					if(!bPartCollition)
					{
						continue;
					}
				}
			}

			NiPoint3 kCenter;
			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);

			CUnit *pkUnit = pkPilot->GetUnit();	
			if ( pkUnit && pkUnit->IsTarget( pkActorUnit ) && pkUnit->IsAlive() )
			{
				bool bAction = true;
				if(IsIgnoreEffect(ACTIONEFFECT_DMG))
				{
					bAction = 0<pkUnit->GetAbil(AT_COLLISION_USE_FORCE);
				}
				if(bAction)
				{
				    NET_C_M_REQ_TRIGGER(QOT_Monster, pkActor->GetGuid(), TRIGGER_ACTION_COLLISION);
				    PgAction * pkReserve = ReserveTransitAction("a_touch_dmg");
					if(pkReserve)
					{
						pkReserve->SetParamAsPoint(20110630, kCenter);
					}
			    }
		    }
	    }
    }
}

int PgActor::SkinningParts(PgItemEx *pkParts, NiNode* pkNode, bool bAttach)
{
	PG_ASSERT_LOG(pkParts);
	PG_ASSERT_LOG(pkNode);
	if(!pkNode || pkParts == NULL)
	{
		return 0;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SkinningParts"), g_pkApp->GetFrameCount()));

	int iRetAttachedNode = 0;
	int iAttachedThisTime = 0;
	int iArrayCount = pkNode->GetArrayCount();
	NiNode* pkAttachingNode = 0;

	for(int iArrayIndex = 0; iArrayIndex<iArrayCount; ++iArrayIndex)
	{
		NiAVObject *pkChild = pkNode->GetAt(iArrayIndex);
		if(!pkChild)
		{
			continue;
		}

		if(pkChild->GetExtraData("HAS_NO_VALID_GEOMETRY")) // QUESTION? 소문자 비교는 되나.
		{
			continue;
		}

		if(NiIsKindOf(NiGeometry, pkChild) && pkChild->GetAppCulled())
		{
			NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node has appculled\n", pkChild->GetName());
		}

		// Geometry정보이고, Hide되어 있지 않으면
		if(NiIsKindOf(NiGeometry, pkChild) && !pkChild->GetAppCulled())
		{
			NiGeometry *pkGeometry = (NiGeometry *)pkChild;
			
			NiSkinInstance* pkSkin = pkGeometry->GetSkinInstance();
			if(pkSkin)
			{
				NiSkinData* pkSkinData = pkSkin->GetSkinData();
				if (pkSkinData == NULL)
					continue;

				unsigned int uiBoneCount = pkSkinData->GetBoneCount();
				NiAVObject* pkRootParent = pkSkin->GetRootParent();
				NiAVObject*const* pkBones = pkSkin->GetBones();

				NiAVObject* pkDup = NULL;

				for(unsigned int ui = 0; ui < uiBoneCount; ++ui)
				{
					NiAVObject* pkCurBone = pkBones[ui];
					if (pkCurBone)
					{
						pkDup = GetCharRoot()->GetObjectByName(pkCurBone->GetName());
						if(pkDup)
						{
							if (pkDup->GetScale() != 1.0f)
							{
								NILOG(PGLOG_WARNING, "[PgActor] SkinningParts, %s node's %d bone(%s), scale is %f.\n", pkChild->GetName(), ui,pkCurBone->GetName(), pkDup->GetScale());
								pkDup->SetScale(1.0f);
							}
							pkSkin->SetBone(ui, pkDup);
						}
						else
						{
							NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node's %d bone, GetMainNIFRoot()->GetObjectByName %s Failed.\n", pkChild->GetName(), ui, pkCurBone->GetName());
						}
					}
					else
					{
						NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node's %d bone is null\n", pkChild->GetName(), ui);
					}
				}

				if (pkRootParent)
				{
					pkDup = GetCharRoot()->GetObjectByName(pkRootParent->GetName());
					if(!pkDup)
					{
						NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node GetMainNIFRoot()->GetObjectByName pkRootParent:%s Failed\n", pkChild->GetName(),pkRootParent->GetName());

					}
				}
				else
				{
					pkDup = NULL;
					NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node's has no skin root\n", pkChild->GetName());
				}
			
				if(!pkDup)
				{
					continue;
				}

				pkSkin->SetRootParent(pkDup);
			}

			// 붙이려는 Geometry의 가장 가까운 부모의 이름과 같은 노드를
			// 모델의 Tree에서 찾아서, 그 노드에 붙인다.
			bool bFindNewParent = false;
			NiNode* pkCandidateNode = pkNode;			
			while(!bFindNewParent && pkCandidateNode && pkCandidateNode->GetParent())
			{
				if(GetCharRoot()->GetObjectByName(pkCandidateNode->GetParent()->GetName()))
				{
					bFindNewParent = true;
					break;
				}
				pkCandidateNode = pkCandidateNode->GetParent();

			}

			// 붙이려는 노드의 부모를 원래의 모델에서 찾았으면, 노드를 붙일 준비를 한다.
			if(bFindNewParent)
			{
				++iAttachedThisTime;
				if(pkParts)
				{
					pkParts->AddAttachedObject(pkCandidateNode);
				}
				pkAttachingNode = pkCandidateNode;
			}
			else
			{
				NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node can't find new parent [Original Parent : %s]\n", pkChild->GetName(), pkNode->GetName());
			}
		}
		else if(NiIsKindOf(NiNode, pkChild)/* && pkChild->GetName() != "char_root"*/)
		{
			iRetAttachedNode += SkinningParts(pkParts, (NiNode*)pkChild, !(bAttach && iAttachedThisTime != 0));
		}
	}

	if(bAttach && iAttachedThisTime > 0 && pkAttachingNode && pkAttachingNode->GetParent())
	{	
		NiNode* pkDup = NiDynamicCast(NiNode, GetCharRoot()->GetObjectByName(pkAttachingNode->GetParent()->GetName()));

		if(pkDup)
		{
			NiNode* pkParent = pkNode->GetParent();
			if (pkParent)
			{
				unsigned int uiArrayCount = pkParent->GetArrayCount();
				for(unsigned int index = 0; index<uiArrayCount; ++index)
				{
					NiAVObject *pkChild = pkParent->GetAt(index);
					if(NiIsKindOf(NiTextureEffect, pkChild))
					{
						if(pkParts)
						{
							pkParts->AddAttachedObject(pkChild);
						}
						NiNode* pkRoot = NiDynamicCast(NiNode, GetCharRoot());
						pkRoot->AttachChild(pkChild, true);
					}
				}

				pkDup->AttachChild(pkAttachingNode, true);
				pkDup->UpdateNodeBound();
				pkDup->UpdateEffects();
				pkDup->UpdateProperties();
				//remove_zero_update 
				//pkDup->Update(0.0f);
				
				iRetAttachedNode += iAttachedThisTime;
			}
			else
			{
				NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node has no parent\n", pkNode->GetName());
			}
		}
		else
		{
			NILOG(PGLOG_ERROR, "[PgActor] SkinningParts, %s node can't find\n", pkAttachingNode->GetParent()->GetName());
		}
	}

	return iRetAttachedNode;
}

//! 파츠를 액터의 특정 더미에 붙인다.(이때의 파츠는 스키닝을 하지 않는 파츠다)
bool PgActor::AttachNoSkinningParts(PgItemEx *pkParts, char const *pcTargetDummy)
{
	if (pkParts == NULL)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.AttachNoSkinningParts"), g_pkApp->GetFrameCount()));

	if(!pcTargetDummy)
	{
		pcTargetDummy = pkParts->GetTargetPoint();
		if(!pcTargetDummy)
		{
			return false;
		}
	}

	bool const	bSequenceReload = (pkParts->EquipLimit() == EQUIP_LIMIT_FACE);

	//	pcPartsNifPath 의 Scene Root 를 pcTargetDummy 에 Attach 시킨다.
	NiNode* pkItem = (NiNode*) pkParts->GetMeshRoot();
	PG_ASSERT_LOG(pkItem);

	NiNode* pkModel = NiDynamicCast(NiNode, GetCharRoot());
	PG_ASSERT_LOG(pkModel);

	if (pkModel == NULL || pkItem == NULL)
		return false;

	NiNode* pkDestNode = NiDynamicCast(NiNode, pkModel->GetObjectByName(pcTargetDummy));
	if(!pkDestNode)
	{
		PG_ASSERT_LOG(!"pcTargetDummy dummy not found from dest");
		return false;
	}

	NiNode* pkSrcNode = NiDynamicCast(NiNode, pkItem->GetObjectByName(pcTargetDummy));
	if(!pkSrcNode)
	{
		PG_ASSERT_LOG(!"pcTargetDummy dummy not found from src");
		return false;
	}

	// 파츠를 붙이기 전에, 이전 파츠를 제거한다.
	_PgOutputDebugString("DetachParts Actor : %s From PgActor::AttachNoSkinningParts\n",GetPilot() ? MB(GetPilot()->GetName()) : GetID().c_str());

	DetachParts(pkParts->EquipLimit());

	//	파츠를 Detach 한 후, 위에서는 찾아졌던 Node가 없어졌거나, 변경됬을 수 있으므로, 다시 한번 찾는다.
	pkDestNode = NiDynamicCast(NiNode, pkModel->GetObjectByName(pcTargetDummy));
	if(!pkDestNode)
	{
		PG_ASSERT_LOG(!"pcTargetDummy dummy not found from dest again");
		return false;
	}

	pkSrcNode->SetRotate(NiQuaternion::IDENTITY);
	pkSrcNode->SetTranslate(NiPoint3::ZERO);

	NiNode* pkAttachingNode = pkSrcNode->GetParent();
	PG_ASSERT_LOG(pkAttachingNode);
	if (NULL == pkAttachingNode )
	{
		return false;
	}

	//pkAttachingNode->SetName("AttachedParts");

	pkDestNode->AttachChild(pkAttachingNode, true);
	pkDestNode->UpdateNodeBound();
	pkDestNode->UpdateProperties();
	pkDestNode->UpdateEffects();
	pkDestNode->Update(0.0f, true);
	if (pkParts->IsWeapon() && g_pkWorld)
	{
		NiTimeController::StartAnimations(pkDestNode, g_pkWorld->GetAccumTime());
	}

	if(bSequenceReload)
	{
		// Morphing Target을 붙였으면, 그에 해당하는 모핑 애니를 리로딩 해주어야 모핑이 된다.
		NiActorManager *pkAM = GetActorManager();
		if(pkAM)
		{
			pkAM->Reset();
			pkAM->ChangeNIFRoot(pkModel);
			pkAM->SetTargetAnimation(m_kSeqID);
		}
		//DoReservedAction(RA_IDLE);
	}

	// Attach한 노드를 보관하고 있는다.
	pkParts->AddAttachedObject(pkAttachingNode);

	// 파츠에 애니가 포함되어 있으면 AMContainer에 등록한다.
	if(pkParts->IsAvailableAnimation())
	{
		m_kSupplementAMContainer.push_back(AMPair(pkParts->EquipLimit(), pkParts->GetActorManager(),pkParts->GetCustomAniIDChangeSetting()));
	}

	PgItemEx *pkAdditionalItem = pkParts->GetAdditionalItem();
	if(pkAdditionalItem != 0)
	{
		AttachNoSkinningParts(pkAdditionalItem);
	}

	// TODO : Weapon No는 Customizing이 필요하다.
	if(pkParts->IsWeapon())
	{
		m_uiMyWeaponType = pkParts->GetWeaponType();
		m_kMyWeaponEquipLimit = pkParts->EquipLimit();
		m_byMyWeaponAnimFolderNum = pkParts->GetAnimFolderNum();
		if(pkParts->GetItemDef())
		{
			SetMyWeaponNo(pkParts->GetItemDef()->No());
		}

		//	콤보 초기화
		SetComboCount(0);
	}
	
	return true;
}

bool PgActor::DetachParts(eEquipLimit kItemPos)
{
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kItemPos);
	if(itr == m_kPartsAttachInfo.end())
	{
		return false;
	}

	if(GetPilot())
	{
		_PgOutputDebugString("DetachParts Actor : %s From PgActor::DetachParts(%d)\n",MB(GetPilot()->GetName()),kItemPos);
	}

	return DetachParts(itr->second);
}

bool PgActor::DetachParts(PgItemEx *pkParts)
{
	if(!pkParts)
	{
		return false;
	}
	//	Item 모델 교체되었다면, 원상 복구 시킨다.
	RestoreItemModel(pkParts->EquipLimit());
	
	if(pkParts->GetMeshRoot())
	{
		DetachActorAlphaProperty(pkParts->GetMeshRoot());
		pkParts->GetMeshRoot()->UpdateProperties();
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.DetachParts"), g_pkApp->GetFrameCount()));

	// Actor에게서 붙였던 Node들을 모두 떼어낸다.
	if( GetPilot() )
	{
		_PgOutputDebugString("[PgActor::DetachParts] Actor:%s Item : %d\n",MB(GetPilotGuid().str()),pkParts->GetItemDef()->No());
	}

	pkParts->ResetHide();
	pkParts->RestoreAttachedObject();

	if(pkParts->GetAdditionalItem())
	{
		pkParts->GetAdditionalItem()->ResetHide();
		pkParts->GetAdditionalItem()->RestoreAttachedObject();
	}

	// Attached Info List에서 지운다.
	m_kPartsAttachInfo.erase(pkParts->EquipLimit());

	// 파츠애니로 붙어있던 AM을 지운다.
	AMContainer::iterator amItr = std::find(m_kSupplementAMContainer.begin(), m_kSupplementAMContainer.end(), AMPair(pkParts->EquipLimit(), 0, &PgItemEx::stCustomAniIDChangeSetting()));
	if(amItr != m_kSupplementAMContainer.end())
	{	
		PG_ASSERT_LOG(amItr->m_spAM);
		if (amItr->m_spAM)
		{
			amItr->m_spAM->Reset();
			amItr->m_spAM = 0;
		}
		m_kSupplementAMContainer.erase(amItr);
	}

	// 파츠가 무기였다면 WeaponType은 1000(Hand)으로 한다.
	if(pkParts->IsWeapon())
	{
		//	EndWeaponTrail();	//	Thread Safe 때문에 일단 주석처리.

		m_uiMyWeaponType = PgItemEx::IT_FST;
		m_kMyWeaponEquipLimit = EQUIP_LIMIT_WEAPON;
		m_byMyWeaponAnimFolderNum = 1;
		SetMyWeaponNo(0);
	}

	if (pkParts->EquipLimit() == EQUIP_LIMIT_HAIR)
	{
		//pkParts->ReloadMeshRoot(0, 0, true);
	}

	pkParts->ClearStatusEffect(GetPilot());
	pkParts->SetActorNodesHide(this, false);

	// 마지막으로 Parts정보 자체를 지운다.
	THREAD_DELETE_ITEM(pkParts);
	return true;
}

//! 모든 파츠를 제거한다.
bool PgActor::DetachAllParts()
{
	//std::vector<int> akDetachParts(32);
	//PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin();
	//while(itr != m_kPartsAttachInfo.end())
	//{
	//	akDetachParts.push_back(itr->first);
	//	++itr;
	//}

	PartsAttachInfo::iterator detach_iter = m_kPartsAttachInfo.begin();
	while(detach_iter != m_kPartsAttachInfo.end())
	{
		if(GetPilot())
		{
			_PgOutputDebugString("DetachParts Actor:%s From PgActor::DetachAllParts\n",MB(GetPilot()->GetName()));
		}
		DetachParts((*detach_iter).first);
		detach_iter = m_kPartsAttachInfo.begin();
	}

	//std::vector<int>::iterator akItr = akDetachParts.begin();
	//while(akItr != akDetachParts.end())
	//{
	//	if(GetPilot())
	//		_PgOutputDebugString("DetachParts Actor:%s From PgActor::DetachAllParts\n",MB(GetPilot()->GetName()));
	//	DetachParts(*akItr);
	//	++akItr;
	//}
	return true;
}

//! 파츠가 이미 붙어있는지 체크한다.
bool	PgActor::IsExistParts(eEquipLimit kItemPos)
{
	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kItemPos);
	if(itr == m_kPartsAttachInfo.end())
	{
		return false;
	}
	return true;
}

void	PgActor::UpdateShakeActor()
{
	//!	캐릭터 흔들기 처리
	if(m_iTotalShakeTime>0)
	{
		int iElaspedTime =BM::GetTime32() - m_iShakeStartTime;
		if(iElaspedTime>m_iTotalShakeTime)
		{
			m_iTotalShakeTime = 0;
			GetNIFRoot()->SetTranslate(m_kOriginalPos);
			m_kOriginalPos.x=m_kOriginalPos.y=m_kOriginalPos.z=0;
		}
		else
		{

			NiPoint3	kRandom(
				(BM::Rand_Index(100)/100.0f)*m_fShakeValue,
				(BM::Rand_Index(100)/100.0f)*m_fShakeValue,
				(BM::Rand_Index(100)/100.0f)*m_fShakeValue);
			
			if(BM::Rand_Range(1)){kRandom.x*=-1;}
			if(BM::Rand_Range(1)){kRandom.y*=-1;}
			if(BM::Rand_Range(1)){kRandom.z*=-1;}

			NiPoint3 kNewPos = m_kOriginalPos+kRandom;
			GetNIFRoot()->SetTranslate(kNewPos);
		}		
	}
}
void	PgActor::DrawParticle(PgRenderer *pkRenderer,bool bOnlyZTestEnable)
{
	PgParticle	*pkParticle = NULL;

	for ( AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle && pkParticle->GetZTest() == bOnlyZTestEnable)
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->PartialRenderClick_Deprecated(pkParticle);
			pkParticle->SetAppCulled(true);
		}
	}

	if(bOnlyZTestEnable == false)
	{
		for ( AttachSlot::iterator itr = m_kAttachSlot_NoZTest.begin(); itr != m_kAttachSlot_NoZTest.end(); ++itr )
		{
			pkParticle = NiDynamicCast(PgParticle,itr->second);
			if(pkParticle)
			{
				pkParticle->SetAppCulled(false);
				pkRenderer->PartialRenderClick_Deprecated(pkParticle);
				pkParticle->SetAppCulled(true);
			}
		}
	}
}
void	PgActor::UpdateHPGaugeBarPosition(NiCamera *pkCamera)
{ 
	if(m_pHPGaugeBar)
	{
		PG_ASSERT_LOG(GetActorManager());
		PG_ASSERT_LOG(GetActorManager()->GetNIFRoot());

		//NiAVObjectPtr	spTargetPoint = GetActorManager()->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
		NiAVObjectPtr	spTargetPoint = GetNodePointStar();
		if( spTargetPoint )
		{
			NiPoint3 kPos = spTargetPoint->GetWorldTranslate();
			if( m_spNameText )
			{
				NiPoint3 const& kFromPos = m_spNameText->GetWorldTranslate();
				kPos -= kFromPos;
				kPos *= 0.30f;
				kPos += kFromPos;
			}
			//else
			//{
			//}
			m_pHPGaugeBar->SetPosition(kPos, pkCamera);
		}
	}
}
void	PgActor::DrawNameText(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	PG_ASSERT_LOG(pkCamera);

#ifndef EXTERNAL_RELEASE
	if(lua_tinker::call<bool>("DrawNameText"))
#endif
	{
		PgActorUtil::SetNameCulled(m_spNameText, m_spGuildNameText, m_spGuildMark, m_spGuardianMark, m_spTitleName, m_spAchievementTitle, m_spGIFTitle, m_spMyhomeMark, m_spCustomCountText, m_spDuelTitle, m_spEffectCountDown, true); // 일단 숨긴다

		if(!m_pTextBalloon || m_pTextBalloon->IsEnable() == false)
		{
			if( !m_bNoName && m_stbNameVisible && m_spNameText && !g_kQuestMan.IsQuestDialog() )
			{
				bool bDrawName = true;
				PgPilot* pkPilot = GetPilot();
				if( pkPilot )
				{
					CUnit* pkUnit = pkPilot->GetUnit();
					if( pkUnit )
					{
						switch( pkUnit->UnitType() )
						{
						case UT_NPC:
						case UT_MONSTER:
							{
								bDrawName = m_bDrawNameNPC;
							}break;
						case UT_PLAYER:
						case UT_PET:
							{
								bDrawName = m_bDrawNamePC;
							}break;
						}
					}
				}

				if( !bDrawName )
				{
					return;
				}

				PgActorUtil::SetNameCulled(m_spNameText, m_spGuildNameText, m_spGuildMark, m_spGuardianMark, m_spTitleName, m_spAchievementTitle, m_spGIFTitle, m_spMyhomeMark, m_spCustomCountText, m_spDuelTitle, m_spEffectCountDown, false);
			}
		}
	}
}
void TurnOnFog(NiAVObject *pkAVObject,bool bTurnOn)
{
	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		NiFogProperty	*pkFog = pkGeom->GetPropertyState()->GetFog();
		if(pkFog)
		{
			pkFog->SetFog(bTurnOn);
		}
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iCount = pkNode->GetArrayCount();
		for(int i=0;i<iCount;++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				TurnOnFog(pkChild,bTurnOn);
			}
		}
	}
}

void PgActor::RenderShadowObject()
{
	if(!g_bUseProjectionShadow)
		return;

	if(m_pkShadow && g_pkWorld)
	{
		SetAppCulled(false);
		TurnOnFog(this,false);
		UpdateProperties();
		m_pkShadow->ClickAndStuff(g_pkWorld->GetAccumTime());
		TurnOnFog(this,true);
		SetAppCulled(true);
		UpdateProperties();
	}
}

bool PgActor::IsDrawable( PgRenderer* pkRenderer, NiCamera* pkCamera )
{
	PgPilot* pkPilot = GetPilot();
	if(pkPilot == NULL || pkPilot->IsHide() == true) { return false; }

	if(IsCompleteLoadParts() == false) { return false; }
	if (GetInvisibleGrade() >= PgActor::INVISIBLE_FAR && GetIgnoreCameraCulling() == false) { return false; }

	const bool bThisIsEnemyActor = IsEnemy( g_kPilotMan.GetPlayerActor() );
	if ( IsHide() || IsBlinkHide() ) // 숨거나 블링크 숨기 / !(적 & 숨었을 경우)
	{
		if( !bThisIsEnemyActor )
		{
			PgCircleShadow::AddOnlyShadowToVisibleArrayRecursive(this,pkRenderer,pkCamera);	//	그림자만 그리기
		}
		return false;
	}
	if( IsInvisible() && bThisIsEnemyActor ) { return false; }

    return true;
}

void PgActor::Draw( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
    if ( IsDrawable(pkRenderer, pkCamera) == false )
			return;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.Draw"), g_pkApp->GetFrameCount()));

	UpdateShakeActor(); //!	캐릭터 흔들기 처리

	SetAppCulled(false);

	NiFrustum kFrustum = pkCamera->GetViewFrustum();
	float fCameraFar = kFrustum.m_fFar;
	if (GetIgnoreCameraCulling())
	{
		if (g_pkWorld && g_pkWorld->GetFogProperty())
			kFrustum.m_fFar = g_pkWorld->GetFogProperty()->GetFogEnd();
		else
			kFrustum.m_fFar = 40000.0f;
		pkCamera->SetViewFrustum(kFrustum);
	}
	pkRenderer->CullingProcess_Deprecated( pkCamera, this, pkRenderer->GetVisibleArray_Deprecated(), true ); // 그릴 객체 컬링

	if(g_bUseActorControllerUpdateOptimize)
	{
		if ( m_bLoadingComplete && GetPilot()->GetUnit()->IsUnitType(UT_PLAYER) == false )
		{
			m_bVisible = !(pkRenderer->GetVisibleArray_Deprecated()->GetCount() == 0);
			if(false==m_bVisible)
			{
				m_bVisible = IsUnderMyControl();
			}
		}
	}

	CullingProcessParticle( pkCamera, pkRenderer->GetVisibleArray_Deprecated(), pkRenderer ); // 그릴 파티클 컬링(Z-Test수행하는 파티클)

    NewWare::Renderer::DrawActor( pkRenderer, this ); // 위에서 컬링한 객체와 파티클을 그림
	
	if (GetIgnoreCameraCulling())
	{
		kFrustum.m_fFar = fCameraFar;
		pkCamera->SetViewFrustum(kFrustum);
	}

	SetAppCulled(true);
}

bool	PgActor::GetCanBatchRender()	const
{
	if(GetNIFRoot()->GetColorLocal() != NiColorA::WHITE)
		return	false;
	if(m_bSpecularOn)
		return	false;

	return	true;
}

bool	PgActor::CanSee()
{
	if(GetPilot() && GetPilot()->IsHide())
	{
		return false;
	}

	if (m_eInvisibleGrade >= PgActor::INVISIBLE_FAR)
	{
		return false;
	}

	if (IsHide())
	{
		return false;
	}
	return	true;
}



bool PgActor::UpdateName(std::wstring const& rkName)
{
	PgPilot	*pPilot = GetPilot();
	if( pPilot )
	{
		//캐릭터 이름
		std::wstring kName = rkName.empty() ? pPilot->GetName() : rkName;
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			TCHAR szTemp[512] = {};
			_sntprintf_s(szTemp, 512, 511, _T("%s\n<%d>"), kName.c_str(), pPilot->GetAbil(AT_CLASS));
			kName = szTemp;
		}
#endif

		if(!IsUnderMyControl() && IsNowFollowing())
		{
			kName += _T("[Following]");
		}

		PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
		if( pNpc && pNpc->HideMiniMap() )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 이름을 표시하지 않음.
			kName.clear();
		}

		BM::vstring kvstrLevAddedName;
		BM::vstring kvstrLevAddedNameBack;
		CUnit* pkUnit = pPilot->GetUnit();
		if( pkUnit )
		{
			int const iDontShowAll = pkUnit->GetAbil(AT_DONT_DISPLAY_ALL);
			if( 1 == iDontShowAll )
			{
				return true;
			}
			int const iDontShowLevel = pkUnit->GetAbil(AT_DONT_DISPLAY_LEVEL);
			bool bPetName = false;
			if(pkUnit->IsUnitType(UT_PET))
			{
				PgPet* pkPet = dynamic_cast<PgPet*>(pkUnit);
				if(pkPet)
				{
					bPetName = (EPET_TYPE_2 == pkPet->GetPetType()) || (EPET_TYPE_3 == pkPet->GetPetType());
				}
			}
			if(pkUnit->IsUnitType(UT_ENTITY) && 0==iDontShowLevel
			&& ENTITY_GUARDIAN==pkUnit->GetAbil(AT_ENTITY_TYPE) )
			{
				kvstrLevAddedName = TTW(224);
				kvstrLevAddedName+=pkUnit->GetAbil(AT_LEVEL);
				kvstrLevAddedName+=_T(" ");
			}
			if( (pkUnit->IsUnitType(UT_MONSTER) || bPetName )
			&&	0 == iDontShowLevel )
			{
				kvstrLevAddedName = TTW(224);
				kvstrLevAddedName+=pkUnit->GetAbil(AT_LEVEL);
				kvstrLevAddedName+=_T(" ");

				int const iGrade = pkUnit->GetAbil(AT_GRADE);
				if( (EMGRADE_UPGRADED == iGrade || EMGRADE_ELITE == iGrade) && (0 == pkUnit->GetAbil(AT_DONT_DISPLAY_GRADE)) )
				{
					kvstrLevAddedNameBack = TTW(500+iGrade);
				}

				/////////////////////////////////
				//전략모드에서 점수 이펙트 출력
				/////////////////////////////////
				int iPoint = pkUnit->GetAbil(AT_STRATEGIC_POINT);
				if( pkUnit->GetAbil(AT_STRATEGIC_MUL_POINT) )
				{//포인트 2배
					iPoint *= 2;
				}

				if( m_iOldStrategicPoint!=iPoint )
				{// 점수 이펙트 표시 제거
					DetachFrom(EAPS_STRATEGIC_POINT);
				}

				if(m_iOldStrategicPoint!=iPoint && iPoint)
				{
					m_iOldStrategicPoint = iPoint;
					int iNo = 0;
					bool bError = false;
					if(iPoint >= 40)
					{
						iNo = 40;
						bError = (iPoint != 40);
					}
					else if(iPoint >= 30)
					{
						iNo = 30;
						bError = (iPoint != 30);
					}
					else if(iPoint >= 20)
					{
						iNo = 20;
						bError = (iPoint != 20);
					}
					else if(iPoint >= 15)
					{
						iNo = 15;
						bError = (iPoint != 15);
					}
					else if(iPoint >= 10)
					{
						iNo = 10;
						bError = (iPoint != 10);
					}
					else
					{
						iNo = 5;
						bError = (iPoint != 5);
					}

#ifndef EXTERNAL_RELEASE
					if(bError)
					{
						SChatLog kChatLog(CT_ERROR);
						BM::vstring vStr(TTW(401155));
						vStr.Replace(L"#CLASS#", pkUnit->GetAbil(AT_CLASS));
						vStr.Replace(L"#POINT#", iPoint);
						g_kChatMgrClient.AddLogMessage(kChatLog, vStr);
					}
#endif
					char buf[MAX_PATH];
					sprintf_s(buf,sizeof(buf),"eff_common_defence_point_%02d",iNo);

					NiAVObject *pkParticle = g_kParticleMan.GetParticle(buf,PgParticle::O_SCALE,1.f);
					if( pkParticle )
					{
						if( !AttachTo(EAPS_STRATEGIC_POINT, "p_ef_star", pkParticle) )
						{
							THREAD_DELETE_PARTICLE(pkParticle);
						}
					}
				}
			}
		}
		kvstrLevAddedName+=kName;
		kvstrLevAddedName+=kvstrLevAddedNameBack;
		//
		std::wstring const kNameFont(_T("{T=Font_Name/}"));
		std::wstring kNameColor, kEmoticon, kEnchantPrefix;
		GetNameColor(kNameColor);
		GetNameEmoticon(kEmoticon);
		GetEnchantPrefixName(kEnchantPrefix, kNameFont, kNameColor);
		std::wstring const kResultName = kEmoticon + kEnchantPrefix + kNameFont + kNameColor + (std::wstring)kvstrLevAddedName;
		
		CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_NAME); // 이름
		float fNameWidth = 0.f;
		if( pFont )
		{
			if( !m_spNameText )
			{
				//NiNode	*pkNameTargetNode = NiDynamicCast(NiNode,GetObjectByName(ATTACH_POINT_STAR));
				NiNode	*pkNameTargetNode = GetNodePointStar();
				if(pkNameTargetNode)
				{
					m_spNameText = NiNew PgTextObject();
					pkNameTargetNode->AttachChild(m_spNameText, true);
				}
				PG_ASSERT_LOG(m_spNameText);
			}

			if( m_spNameText )
			{
				m_spNameText->SetText(kResultName, pFont);

				NiPoint3 kPos = m_spNameText->GetTranslate();
				kPos.z = PgActorUtil::FindNamePosZ(this);
				m_spNameText->SetTranslate(kPos);

				fNameWidth = static_cast<float>(m_spNameText->GetTextWidth());
			}
		}

		if(pkUnit && pkUnit->IsUnitType(UT_ENTITY) )
		{//전략디펜스 가디언 계급표시
			fNameWidth = 0.0f;
			if(m_spNameText)
			{
				fNameWidth = NiMax(static_cast<float>(m_spNameText->GetTextWidth()), fNameWidth);
			}
			int const iGuardianRank = pkUnit->GetAbil(AT_DISPLAY_LEVEL);
			if( iGuardianRank )
			{
				if( !m_spGuardianMark )
				{
					//NiNode* pkMarkTargetNode = NiDynamicCast(NiNode, GetObjectByName(ATTACH_POINT_STAR));
					NiNode	*pkMarkTargetNode = GetNodePointStar();
					if( pkMarkTargetNode )
					{
						m_spGuardianMark = NiNew PgGuardianMark();
						pkMarkTargetNode->AttachChild(m_spGuardianMark, true);
					}
				}
				if( m_spGuardianMark )
				{
					m_spGuardianMark->Set( static_cast<byte>(iGuardianRank), fNameWidth + PgActorUtil::fNameWidthGap);
					NiPoint3 kPos = m_spGuardianMark->GetTranslate();
					kPos.z = PgActorUtil::FindNamePosZ(this);
					m_spGuardianMark->SetTranslate(kPos);
				}
			}
			else 
			{
				if( m_spGuardianMark )
				{
					//NiNode* pkTargetNode = NiDynamicCast(NiNode, GetObjectByName(ATTACH_POINT_STAR));
					NiNode	*pkTargetNode = GetNodePointStar();
					if( pkTargetNode )
					{
						PgActorUtil::DetachFromNode(pkTargetNode, m_spGuardianMark);
					}
					m_spGuardianMark = 0;
				}
			}
		}
		//
		//길드 이름
		pFont = g_kFontMgr.GetFont(FONT_GUILDNAME);
		bool bHaveGuild = false;
		if( pFont
		&&	pkUnit
		&&	pkUnit->IsUnitType(UT_PLAYER) )
		{
			PgPlayer *pkPC = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPC )
			{
				if( BM::GUID::IsNotNull(pkPC->GuildGuid()) )//길드에 가입되어 있으면
				{
					bHaveGuild = true;
					SGuildOtherInfo kGuildInfo;
					if( !g_kGuildMgr.GetGuildInfo(pkPC->GuildGuid(), pkPC->GetID(), kGuildInfo) )
					{
						// 길드 정보가 없으면 서버로 정보를 요청한다.
						g_kGuildMgr.ReqOtherGuildInfo(pkPC->GuildGuid(), GetGuid());
					}
					else
					{
						// 있으면 길드 이름/마크 생성
						// 길드 이름
						if(!m_spGuildNameText)
						{
							//NiNode	*pkNameTargetNode = NiDynamicCast(NiNode,GetObjectByName(ATTACH_POINT_STAR));
							NiNode	*pkNameTargetNode = GetNodePointStar();
							if( pkNameTargetNode )
							{
								m_spGuildNameText = NiNew PgTextObject();
								pkNameTargetNode->AttachChild(m_spGuildNameText, true);
							}
							PG_ASSERT_LOG(m_spGuildNameText);
						}
						if( m_spGuildNameText )
						{
							NiColorA kGuildNameColor;
							GetGuildNameColor( kGuildNameColor );

							m_spGuildNameText->SetText(kGuildInfo.kName, pFont);
							m_spGuildNameText->SetTextColor(kGuildNameColor);

							NiPoint3 kPos = m_spGuildNameText->GetTranslate();
							kPos.z = PgActorUtil::FindNamePosZ(this) + PgActorUtil::fAddedGuildNameZ;
							m_spGuildNameText->SetTranslate(kPos);

							fNameWidth = NiMax(static_cast<float>(m_spGuildNameText->GetTextWidth()), fNameWidth);
						}

						// 길드 마크
						if( !m_spGuildMark )
						{
							//NiNode* pkMarkTargetNode = NiDynamicCast(NiNode, GetObjectByName(ATTACH_POINT_STAR));
							NiNode* pkMarkTargetNode = GetNodePointStar();
							if( pkMarkTargetNode )
							{
								m_spGuildMark = NiNew PgGuildMark();
								pkMarkTargetNode->AttachChild(m_spGuildMark, true);
							}
						}
						if( m_spGuildMark )
						{
							m_spGuildMark->Set( kGuildInfo.cEmblem, kGuildInfo.byEmporiaGrade, fNameWidth + PgActorUtil::fNameWidthGap);
							NiPoint3 kPos = m_spGuildMark->GetTranslate();
							kPos.z = PgActorUtil::FindNamePosZ(this) + PgActorUtil::fAddedGuildMarkZ;
							m_spGuildMark->SetTranslate(kPos);
						}
					}
				}
				else 
				{
					if( m_spGuildNameText
					||	m_spGuildMark )
					{
						//NiNode* pkTargetNode = NiDynamicCast(NiNode, GetObjectByName(ATTACH_POINT_STAR));
						NiNode* pkTargetNode = GetNodePointStar();
						if( pkTargetNode )
						{
							PgActorUtil::DetachFromNode(pkTargetNode, m_spGuildNameText);
							PgActorUtil::DetachFromNode(pkTargetNode, m_spGuildMark);
						}
						m_spGuildNameText = 0;
						m_spGuildMark = 0;
					}
				}

				//노점
				if(pkPC->OpenVendor())
				{
					if(!m_pVendorBalloon)
					{
						m_pVendorBalloon = g_kVendorBalloonMgr.CreateNode();
						if( m_pVendorBalloon )
						{
							m_pVendorBalloon->Init(this);
						}
					}
					std::wstring kVendorTitle = pkPC->VendorTitle();
					std::wstring const kFormStr(TTW(799439));
					wchar_t szBuf[200] ={0,};
					wsprintfW(szBuf, kFormStr.c_str(), kVendorTitle.c_str());
					kVendorTitle = szBuf;
					m_pVendorBalloon->SetNewTitle(kVendorTitle, pkPC->GetID());
					
					std::string kActionName = "a_vendor";
					PgAction* pkAction = this->GetAction();
					if(!pkAction)
					{
						BM::Stream	kPacket(PT_C_M_REQ_VENDOR_STATE);
						kPacket.Push( pkPC->VendorGuid() );
						NETWORK_SEND(kPacket);
					}
				}
				else
				{
					if( m_pVendorBalloon )
					{
						m_pVendorBalloon->SetNewTitle(_T(""));
					}
				}

				// 파티 & 원정대 벌룬
				bool bPartyMaster = false;
				bool bExpeditionMaster = false;
				bool const bInParty = BM::GUID::IsNotNull(pkPC->PartyGuid());
				bool const bInExpedition = BM::GUID::IsNotNull(pkPC->ExpeditionGuid());
				if( bInExpedition )
				{
					SExpeditionInfo ExpeditionInfo;
					bool const bRet = g_kExpedition.GetExpedition(pkPC->ExpeditionGuid(), pkPC->GetID(), ExpeditionInfo);
					if( false == bRet )	// 찾는 원정대가 원정대 목록에 없을 때
					{
						// 요청한다.
						g_kExpedition.ReqOtherExpeditionInfo(pkPC->ExpeditionGuid(), GetGuid());
					}
					bExpeditionMaster = pkPC->GetID() == ExpeditionInfo.MasterGuid;
					if( bExpeditionMaster )
					{
						// 원정대 벌룬
						if( false == m_pExpeditionBalloon )
						{
							m_pExpeditionBalloon = g_kExpeditionBalloonMgr.CreateNode();
							if( m_pExpeditionBalloon )
							{
								m_pExpeditionBalloon->Init(this);
							}
						}
						
						std::wstring TempExpeditionName = ExpeditionInfo.ExpeditionName;
						std::wstring ExpeditionTitle;
						if( FormatTTW(ExpeditionTitle, 710045, TempExpeditionName.c_str(), ExpeditionInfo.cCurMember, ExpeditionInfo.cMaxMember)
						&&	m_pExpeditionBalloon )
						{
							bool const bTitlePublic = ExpeditionInfo.ExpeditionOption.GetOptionPublicTitle() == EOT_Public;
							m_pExpeditionBalloon->SetNewTitle(ExpeditionTitle, pkPC->ExpeditionGuid(), bTitlePublic);
							m_pExpeditionBalloon->SetMaxMember( (ExpeditionInfo.cCurMember == ExpeditionInfo.cMaxMember) );
						}
					}
				}
				else if( bInParty )
				{
					SClientPartyName kPartyName;
					bool const bRetName = g_kParty.GetPartyName(pkPC->PartyGuid(), pkPC->GetID(), kPartyName);
					if( !bRetName )
					{
						g_kParty.ReqOtherPartyInfo(pkPC->PartyGuid(), GetGuid());
					}

					bPartyMaster = pkPC->GetID() == kPartyName.kMasterGuid;
					if( bPartyMaster )
					{
						if( !m_pPartyBalloon )
						{
							m_pPartyBalloon = g_kPartyBalloonMgr.CreateNode();
							if( m_pPartyBalloon )
							{
								m_pPartyBalloon->Init(this);
							}
						}

						std::wstring kTempPartyName = kPartyName.kPartyName;
						if( kPartyName.kPartyName.empty() )
						{
							kTempPartyName = g_kParty.GeneratePartyName(kPartyName.kPartyGuid);
						}

						std::wstring kPartyTitle;
						if( FormatTTW(kPartyTitle, 401202, kTempPartyName.c_str(), kPartyName.cCurMember, kPartyName.cMaxMember)
						&&	m_pPartyBalloon )
						{
							bool const bTitlePublic = kPartyName.kPartyOption.GetOptionPublicTitle() == POT_Public;
							m_pPartyBalloon->SetNewTitle(kPartyTitle, pkPC->PartyGuid(), bTitlePublic);
							m_pPartyBalloon->SetMaxMember( (kPartyName.cCurMember == kPartyName.cMaxMember) );
						}
					}
				}

				if( !bInParty
				||	!bPartyMaster )
				{
					if( m_pPartyBalloon )
					{
						m_pPartyBalloon->SetNewTitle(_T(""));
					}
				}

				if( !bInExpedition
				||	!bExpeditionMaster )
				{
					if( m_pExpeditionBalloon )
					{
						m_pExpeditionBalloon->SetNewTitle(_T(""));
					}
				}
			}
		}

		// 업적 GIF 타이틀용 폰트
		PgBase_Item kAchieveItem;
		bool const bEquipAchieve = PgActorUtil::GetEquipAchievementItem(this, kAchieveItem);
		int const iAchievementsTitleNo = bEquipAchieve ? PgActorUtil::GetAchievementsTitleNo(kAchieveItem) : 0;
		if( !bEquipAchieve || 0==iAchievementsTitleNo )
		{
			if( m_spAchievementTitle )
			{
//				PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spAchievementTitle);
				if(IsRidingPet())
				{
					PgActorUtil::DetachFromNode(GetMountTargetPet(), ATTACH_POINT_RIDENAME, m_spAchievementTitle);
				}
				else
				{
					PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spAchievementTitle);
				}
				m_spAchievementTitle = 0;
			}
			if( m_spMyhomeMark )
			{
//				PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spMyhomeMark);
				if(IsRidingPet())
				{
					PgActorUtil::DetachFromNode(GetMountTargetPet(), ATTACH_POINT_RIDENAME, m_spMyhomeMark);
				}
				else
				{
					PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spMyhomeMark);
				}
				m_spMyhomeMark = 0;
			}
		}
		else
		{
			if( !m_spAchievementTitle )
			{
				//NiNode* pkNameTargetNode = NiDynamicCast(NiNode,GetObjectByName(ATTACH_POINT_STAR));
				NiNode* pkNameTargetNode = GetNodePointStar();
				if( pkNameTargetNode )
				{
					m_spAchievementTitle = NiNew PgAchievementTitle();
					pkNameTargetNode->AttachChild(m_spAchievementTitle, true);
				}
				PG_ASSERT_LOG(m_spAchievementTitle);
			}
			if( m_spAchievementTitle )
			{
				float fAddZPos = 0.0f;

				int	iEmoticonID = 0;
				// GIF용 타이틀이 있는지 체크
				if( g_kEmoFontMgr.Trans_key_value(iAchievementsTitleNo, iEmoticonID) )
				{
					pFont = g_kFontMgr.GetFont(UNI("AchievementsTitleFont"));
					if( pFont )
					{
						std::wstring kString = _T("{T=AchievementsTitleFont/}");

						WCHAR const	wEmoticonIDChar = iEmoticonID;
						kString += wEmoticonIDChar;
						m_spAchievementTitle->UseBgColor(false);
						m_spAchievementTitle->Set(pFont, kString);
						m_spAchievementTitle->SetScale(1.5f);
						fAddZPos = 7.0f;
					}
				}
				//없으면 일반 타이틀로 출력
				else
				{
					pFont = g_kFontMgr.GetFont(FONT_TITLENAME); // 호칭
					if( pFont )
					{
						E_ITEM_GRADE Grade = IG_NORMAL;

						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const *pDef = kItemDefMgr.GetDef(kAchieveItem.ItemNo());
						if(pDef)
						{
							Grade = static_cast<E_ITEM_GRADE>(pDef->GetAbil(AT_GRADE));
						}
						else
						{
							Grade = GetItemGrade(kAchieveItem);
						}
						
						DWORD dwFontColor = 0xFFFFFFFF;
						DWORD dwBgColor = 0xFF111111;//0xFF4D1101;
						switch( Grade )
						{
						case IG_RARE:		{ dwFontColor = 0xFF00E518; dwBgColor = 0xFF111111; }break;
						case IG_UNIQUE:		{ dwFontColor = 0xFF66CCFF; dwBgColor = 0xFF111111; }break;
						case IG_ARTIFACT:	{ dwFontColor = 0xFFFFBA21; dwBgColor = 0xFF111111; }break;
						case IG_LEGEND:		{ dwFontColor = 0xFFFFD5FF; dwBgColor = 0xFF111111; }break;
						}

						std::wstring const* pkDefString = NULL;
						if( GetDefString(iAchievementsTitleNo, pkDefString) )
						{
							m_spAchievementTitle->UseBgColor(true);
							m_spAchievementTitle->Set(pFont, (*pkDefString), dwBgColor, dwFontColor);
							m_spAchievementTitle->SetScale(1.0f);
						}
					}
				}

				NiPoint3 kPos = m_spAchievementTitle->GetTranslate();
				kPos.z = PgActorUtil::FindNamePosZ(this) + ((bHaveGuild)? PgActorUtil::fAddedGuildNameZ*2.f: PgActorUtil::fAddedGuildNameZ) + fAddZPos;
				m_spAchievementTitle->SetTranslate(kPos);
			}
		}

		// GIF 타이틀용 폰트
		if(pkUnit)
		{
			int const iEmoticonID = pkUnit->GetAbil(AT_DISPLAY_GIF_TITLE);

			if( 0==iEmoticonID )
			{
				if( m_spGIFTitle )
				{
					PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spGIFTitle);
					m_spGIFTitle = 0;
				}
			}
			else
			{
				if( !m_spGIFTitle )
				{
					//NiNode* pkNameTargetNode = NiDynamicCast(NiNode,GetObjectByName(ATTACH_POINT_STAR));
					NiNode* pkNameTargetNode = GetNodePointStar();
					if( pkNameTargetNode )
					{
						m_spGIFTitle = NiNew PgAchievementTitle();
						pkNameTargetNode->AttachChild(m_spGIFTitle, true);
					}
					PG_ASSERT_LOG(m_spGIFTitle);
				}
				if( m_spGIFTitle )
				{
					float fAddZPos = 0.0f;

					pFont = g_kFontMgr.GetFont(UNI("GIFTitleFont"));
					if( pFont )
					{
						std::wstring kString = _T("{T=GIFTitleFont/}");

						WCHAR const	wEmoticonIDChar = iEmoticonID;
						kString += wEmoticonIDChar;
						m_spGIFTitle->UseBgColor(false);
						m_spGIFTitle->Set(pFont, kString);
						m_spGIFTitle->SetScale(1.5f);
						fAddZPos = 2.5f;
						if(pkUnit->IsUnitType(UT_MONSTER))
						{
							fAddZPos = 5.0f;
						}
					}

					NiPoint3 kPos = m_spGIFTitle->GetTranslate();
					kPos.z = PgActorUtil::FindNamePosZ(this) + ((bHaveGuild)? PgActorUtil::fAddedGuildNameZ*2.f: PgActorUtil::fAddedGuildNameZ) + fAddZPos;
					m_spGIFTitle->SetTranslate(kPos);
				}
			}
		}

		if( g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE))
		{
			UpdateCustomCount(0,false);
		}

		RefreshCustomItemColor();


		if(!m_spDuelTitle)
		{
			NiNode* pkNameTargetNode = GetNodePointStar();
			if( pkNameTargetNode && GetUnit() && GetUnit()->IsUnitType(UT_PLAYER) )
			{
				m_spDuelTitle = NiNew PgDuelTitle();
				pkNameTargetNode->AttachChild(m_spDuelTitle, true);
				SetDuelWinnerTitle();
			}
		}
		if(m_spDuelTitle)
		{
			NiNode* pkNameTargetNode = GetNodePointStar();
			if( pkNameTargetNode && GetUnit() && GetUnit()->IsUnitType(UT_PLAYER) )
			{
				NiPoint3 kPos = m_spDuelTitle->GetTranslate();
				if(m_spTitleName)
				{
					kPos = m_spTitleName->GetTranslate();
				}
				kPos.z = PgActorUtil::FindNamePosZ(this) + ((bHaveGuild)? PgActorUtil::fAddedGuildNameZ*2.f: PgActorUtil::fAddedGuildNameZ);
				if( m_spGIFTitle )
				{
					kPos.z += (m_spGIFTitle->GetTranslate().z - kPos.z) + 2.5f;
				}
				if(m_spAchievementTitle)
				{
					kPos.z += (m_spAchievementTitle->GetTranslate().z - kPos.z) + 2.5f;
				}
				kPos.z += 13.f;
				m_spDuelTitle->SetTranslate(kPos);
			}
		}

		//이펙트 카운드 다운
		if(!m_spEffectCountDown)
		{
			NiNode* pkTargetNode = GetNodePointStar();
			if( pkTargetNode && GetEffectCountDownSec() )
			{
				m_spEffectCountDown = NiNew PgEffectCountDown();
				PgActorUtil::AttachToNode(pkTargetNode, m_spEffectCountDown);
			}
		}
		if(m_spEffectCountDown)
		{
			if( NiNode* pkTargetNode = GetNodePointStar() )
			{
				WORD const wCountDown = GetEffectCountDownSec();
				if(wCountDown <= 0)
				{
					PgActorUtil::DetachFromNode(pkTargetNode, m_spEffectCountDown);
					m_spEffectCountDown = NULL;
				}
				else
				{
					NiPoint3 kPos = m_spEffectCountDown->GetTranslate();
					if(m_spTitleName)
					{
						kPos = m_spTitleName->GetTranslate();
					}
					kPos.z = PgActorUtil::FindNamePosZ(this) + ((bHaveGuild)? PgActorUtil::fAddedGuildNameZ*2.f: PgActorUtil::fAddedGuildNameZ);
					if( m_spGIFTitle )
					{
						kPos.z += (m_spGIFTitle->GetTranslate().z - kPos.z) + 2.5f;
					}
					if(m_spAchievementTitle)
					{
						kPos.z += (m_spAchievementTitle->GetTranslate().z - kPos.z) + 2.5f;
					}
					kPos.z += 13.f;
					m_spEffectCountDown->SetTranslate(kPos);
					m_spEffectCountDown->Set(wCountDown);
				}
			}
		}

		return true;
	}
	return false;
}
bool PgActor::UpdateCustomCount(int const iCount, bool bUpdate)
{
	//NiNode	*pkNameTargetNode = NiDynamicCast(NiNode,GetObjectByName(ATTACH_POINT_STAR));
	NiNode	*pkNameTargetNode = GetNodePointStar();
	if(pkNameTargetNode)
	{
		if( !bUpdate)
		{
			if( m_spCustomCountText)
			{
				//PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spCustomCountText);
				if(IsRidingPet())
				{
					PgActorUtil::DetachFromNode(GetMountTargetPet(), ATTACH_POINT_RIDENAME, m_spCustomCountText);
				}
				else
				{
					PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spCustomCountText);
				}
				m_spCustomCountText = 0;
			}
			return false;
		}
		//
		if( !m_spCustomCountText )
		{
			m_spCustomCountText = NiNew PgTextObject();
			pkNameTargetNode->AttachChild(m_spCustomCountText, true);
			PG_ASSERT_LOG(m_spCustomCountText);
		}
		//
		if( m_spCustomCountText)
		{
			if( 1 < iCount)
			{
				std::wstring strCount = std::wstring( BM::vstring(iCount) );
				std::wstring const kNameFont(_T("{T=Font_Name/}"));
				std::wstring kNameColor = L"{C=0xFFFFFF00/}";
				std::wstring const kResultName = kNameFont + kNameColor + strCount;

				CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_NAME); // 이름
				if( pFont )
				{
					pFont->SetStyle2(XUI::CXUI_Font::FS_BOLD);
					m_spCustomCountText->SetText(kResultName, pFont);
				}
				NiPoint3 kPos = m_spCustomCountText->GetTranslate();;
				float fAddZPos = 30.0f;
				kPos.z = PgActorUtil::FindNamePosZ(this) + PgActorUtil::fAddedGuildNameZ*2.f + fAddZPos;
				NiPoint3 kActorPos = GetLookingDir();
				if( 0 < kActorPos.x)
				{
					kPos.x = -3.0f;
				}
				else
				{
					kPos.x = 3.0f;
				}
				m_spCustomCountText->SetScale(5.0f);
				m_spCustomCountText->SetTranslate(kPos);
			}
			else
			{
				//PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spCustomCountText);
				if(IsRidingPet())
				{
					PgActorUtil::DetachFromNode(GetMountTargetPet(), ATTACH_POINT_RIDENAME, m_spCustomCountText);
				}
				else
				{
					PgActorUtil::DetachFromNode(this, ATTACH_POINT_STAR, m_spCustomCountText);
				}
				m_spCustomCountText = 0;
			}
		}
	}
	RefreshCustomItemColor();
	return true;
}

bool PgActor::IsEnemy(PgActor *pkTarget)	//	나의 적인가?
{
	if(!pkTarget)
	{
		return	false;
	}
	PgPilot	*pkPilot = GetPilot();
	PgPilot	*pkTargetPilot = pkTarget->GetPilot();

	if(!pkPilot || !pkTargetPilot)
	{
		return	false;
	}

	if(pkPilot == pkTargetPilot) 
	{
		return false;	//	나 자신
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	PG_ASSERT_LOG(pkUnit);
	if(!pkUnit)
	{
		return false;
	}

	CUnit* pkTargetUnit = pkTargetPilot->GetUnit();
	PG_ASSERT_LOG(pkTargetUnit);
	if(!pkTargetUnit)
	{
		return false;
	}

	bool bIsEnemy = false;

	switch(pkUnit->UnitType())
	{
	case UT_PLAYER:
	case UT_ENTITY:
	case UT_PET:
	case UT_SUB_PLAYER:
	case UT_SUMMONED:
		{
			if(pkTargetUnit->IsUnitType(UT_MONSTER) || pkTargetUnit->IsUnitType(UT_OBJECT))	//	플레이어라면 몬스터가 적이다
			{
				bIsEnemy = true;
			}
			else if ( pkTargetUnit->IsUnitType(UT_PLAYER) 
					|| pkTargetUnit->IsUnitType(UT_SUB_PLAYER)	// SUB_PLAYER를 타격 하고 싶을때
					|| pkTargetUnit->IsUnitType(UT_SUMMONED)
				)
			{
				int iMyTeam = pkPilot->GetAbil(AT_TEAM);
				int iTargetTeam = pkTargetPilot->GetAbil(AT_TEAM);
				if(iMyTeam!=iTargetTeam && iMyTeam!=0 && iTargetTeam!=0)
				{
					bIsEnemy = true;
				}

				if(pkUnit->GetAbil(AT_CALLER_TYPE)&UT_MONSTER)
				{
					bIsEnemy = true;
				}
			}
		}break;
	case UT_MONSTER:
	case UT_BOSSMONSTER:
		{
			if(pkTargetUnit->IsUnitType(UT_PLAYER)		//	몬스터라면 플레이어가 적이다
				|| pkTargetUnit->IsUnitType(UT_SUMMONED))
			{
				bIsEnemy = true;
			}
		}break;
	case UT_OBJECT:
		{
			if( g_pkWorld
				&&  GATTR_MISSION == g_pkWorld->GetAttr() )
			{
				if( pkTargetUnit->IsUnitType(UT_PLAYER) )
				{
					bIsEnemy = true;
				}
			}
		}break;
	}

	return	bIsEnemy;
}

void PgActor::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	//	HP 게이지 바
	UpdateHPGaugeBarPosition(pkCamera);

	bool bEnemy = IsEnemy(g_kPilotMan.GetPlayerActor());

	//렌더링 되지 않도록 설정 한 후
	if(m_pHPGaugeBar)
	{
		if(bEnemy)
		{
			m_pHPGaugeBar->EnableDrawImmediate(false);
		}
	}

	if (m_eInvisibleGrade > INVISIBLE_MIDDLE)
	{
		return;
	}
	if(GetPilot() && GetPilot()->IsHide())
	{
		return;
	}

	if (IsHide() || (bEnemy && IsInvisible()))
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.DrawImmediate"), g_pkApp->GetFrameCount()));

	// 보이지 않아야하는 로직이 다 통과하면 Draw가능하도록 변경
	if(m_pHPGaugeBar)
	{
		m_pHPGaugeBar->EnableDrawImmediate(true);
	}
	if( false == IsHideNameTitle() )
	{//	이름,길드이름 그리기		
		DrawNameText(pkRenderer,pkCamera);
	}

	//	버프 아이콘
	DrawHeadBuffIconList(pkRenderer,pkCamera);
}

void	PgActor::DrawHeadBuffIconList(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	if(!m_pkHeadBuffIconList)
	{
		return;
	}
	PG_ASSERT_LOG(GetActorManager());
	PG_ASSERT_LOG(GetActorManager()->GetNIFRoot());

	//NiAVObjectPtr	spTargetPoint = GetActorManager()->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
	NiAVObjectPtr	spTargetPoint = GetNodePointStar();

	NiPoint3	kTargetPoint = GetPosition();
	if(spTargetPoint)
	{
		kTargetPoint = spTargetPoint->GetWorldTranslate();
		kTargetPoint.z+=20.0f;
	}

	m_pkHeadBuffIconList->SetPosition(kTargetPoint);
	m_pkHeadBuffIconList->DrawImmediate(pkRenderer,pkCamera);
}

void PgActor::AddHeadBuffIcon(int const iEffectID)
{
	if(g_kHeadBuffIconListMgr.IsAlive() && m_pkHeadBuffIconList)
	{
		m_pkHeadBuffIconList->AddNewIcon(iEffectID);
	}
}

void PgActor::RemoveHeadBuffIcon(int const iEffectID)
{
	if(g_kHeadBuffIconListMgr.IsAlive() && m_pkHeadBuffIconList)
	{
		m_pkHeadBuffIconList->RemoveIcon(iEffectID);
	}
}

void PgActor::SetInstallTimerGauge(float fInstallTotalTime)
{
	g_kEnergyGaugeMan.DestroyGauge( m_pHPGaugeBar );
	m_pHPGaugeBar = g_kEnergyGaugeMan.CreateInstallNewGauge(fInstallTotalTime);
}

void PgActor::DestroyInstallTimerGauge()
{
	if(m_pHPGaugeBar)
	{
		g_kEnergyGaugeMan.DestroyGauge(m_pHPGaugeBar);
		m_pHPGaugeBar = NULL;
	}
}

void PgActor::SetAliveTimeGauge(float fAliveTotalTime)
{
	if(fAliveTotalTime > 0.f)
	{
		g_kEnergyGaugeMan.DestroyGauge( m_pHPGaugeBar );
		m_pHPGaugeBar = g_kEnergyGaugeMan.CreateAliveTimeNewGauge(fAliveTotalTime);
	}
}

void PgActor::DestroyAliveTimeGauge()
{
	if(m_pHPGaugeBar)
	{
		g_kEnergyGaugeMan.DestroyGauge(m_pHPGaugeBar);
		m_pHPGaugeBar = NULL;
	}
}

float PgActor::GetAnimationLength(std::string &rkAnimationName)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetAnimationLength"), g_pkApp->GetFrameCount()));
	PgActionSlot* pkActionSlot = GetActionSlot();
	if(GetActorManager() == NULL || pkActionSlot == NULL)
	{
		PG_ASSERT_LOG(!"ActorSlot or ActorManager doesn't exist!");
		return 0;
	}

	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(rkAnimationName, kSeqID))
	{
		return 0;
	}

	NiControllerSequence* pkSeq = GetActorManager()->GetSequence(kSeqID);
	if(pkSeq)
	{
		return pkSeq->GetLength();
	}

	return 0;
}

bool PgActor::GetAnimationInfo(std::string &rkInfoName, int iSeqID,std::string &rkInfoOut, PgAction* pkAction)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetAnimationInfo"), g_pkApp->GetFrameCount()));
	PgActionSlot* pkActionSlot = GetActionSlot();
	if(GetActorManager() == NULL || pkActionSlot == NULL || (pkAction == NULL && m_pkAction == NULL))
	{
		PG_ASSERT_LOG(!"ActorSlot or ActorManager or Action doesn't exist!");
		return false;
	}

	std::string animationName;
	if (pkAction)
	{
		pkAction->GetActionName(animationName, pkAction->GetCurrentSlot());
	}
	else if (m_pkAction->GetActionName(animationName, m_pkAction->GetCurrentSlot()) == false)
	{
		NILOG(PGLOG_LOG, "Can't found %s, %d action slot\n", animationName, m_pkAction->GetCurrentSlot());
		return false;
	}
	
	if(m_pkPilot)
	{// 컨테이너에 들어가있는것은 오리지널 번호로 들어가 있으므로 변환해주고
		iSeqID = PgActorUtil::GetOrigAniSeqID(iSeqID, m_pkPilot->GetBaseClassID(), m_byWeaponAnimFolderNumAtActionStart);
	}

	bool bFindAnimationInfo = pkActionSlot->GetAnimationInfo(animationName, iSeqID,rkInfoName, rkInfoOut);
	if( !bFindAnimationInfo && m_pkAction )
	{
		bFindAnimationInfo = m_pkAction->GetAnimationInfo(m_pkAction->GetCurrentSlot(),rkInfoName,rkInfoOut);
	}
	return bFindAnimationInfo;
}

//bool PgActor::ActivateLayerAnimation(char const *pcAnimationName)
//{
//	if(!pcAnimationName)
//	{
//		return false;
//	}
//	if(!GetActorManager())
//		return	false;
//
//	NiActorManager::SequenceID kSeqID;
//	PgActionSlot* pkActionSlot = GetActionSlot();
//	if(!pkActionSlot->GetAnimation(std::string(pcAnimationName), kSeqID))
//	{
//		return false;
//	}
//
//	GetActorManager()->ActivateSequence(kSeqID, 2);
//
//	return true;
//}
//
//bool PgActor::DeactivateLayerAnimation(char const *pcAnimationName)
//{
//	if(!GetActorManager())
//		return	false;
//
//	PgActionSlot* pkActionSlot = GetActionSlot();
//	if (pcAnimationName == NULL || GetActorManager() == NULL || pkActionSlot == NULL)
//	{
//		return false;
//	}
//
//	NiActorManager::SequenceID kSeqID;
//	if(!pkActionSlot->GetAnimation(std::string(pcAnimationName), kSeqID))
//	{
//		return false;
//	}
//
//	GetActorManager()->DeactivateSequence(kSeqID);
//
//	return true;
//}

bool PgActor::SetTargetAnimation(std::string const& rkAnimationName, bool const bActivate, bool const bNoRandom)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetTargetAnimation"), g_pkApp->GetFrameCount()));
	PgActionSlot* pkActionSlot = GetActionSlot();

	DoKFMTransition();

	NiActorManager *pkAM = GetActorManager();
	if (pkAM == NULL || pkActionSlot == NULL)
	{ 
		PG_ASSERT_LOG(!"ActorSlot or ActorManager doesn't exist!");
		return false;
	}

	NiActorManager::SequenceID kSeqID;
	NiActorManager::SequenceID kSeqIDOrig;
	if(!pkActionSlot->GetAnimation(rkAnimationName, kSeqID,bNoRandom))
	{
		return false;
	}

//	_PgOutputDebugString("Actor[%s] SetTargetAnimation rkAnimationName:%s kSeqID:%d\n",MB(GetPilot()->GetName()),
//		rkAnimationName.c_str(),
//		kSeqID);

	kSeqIDOrig = kSeqID;
	if(m_pkPilot)
	{
		kSeqID = PgActorUtil::GetCalcAniSeqID(kSeqIDOrig, m_pkPilot->GetBaseClassID(), m_byWeaponAnimFolderNumAtActionStart);
	}

	if(!pkAM->GetSequence(kSeqID))
	{
		if(!pkActionSlot->GetDefaultAnimation(rkAnimationName, kSeqID))
		{
			m_kSeqID = NiActorManager::INVALID_SEQUENCE_ID;

			// TODO : 애니가 얼지 않게 해주자. 중요도 높은 작업!!
			return false;
		}
	}

	bool	bPlaySlotSound = false;

	// 즉시 Animation을 Play한다.
	if(bActivate && m_kSeqID != kSeqID)
	{
		ResetAnimation();	//	요거 해줘야 캐릭터 메쉬 터지는 문제가 사라진다.

		m_kSeqID = kSeqID;
		
		bPlaySlotSound = ActivateAnimation();
	}
	else
	{
		NiControllerSequence *pkController = pkAM->GetSequence(m_kSeqID);
		if(pkController && pkController->GetCycleType() !=  NiTimeController::LOOP)
		{
			pkController->ResetSequence();

			pkAM->RebuildTimeline();	//	leesg213 2006.12.11 요걸 해줘야 textkey event 가 정상적으로 발생된다.
			
			bPlaySlotSound = true;
		}
	}

	//Common 이펙트를 붙인다
	std::string kCommonEffectDetachSkip;
	pkActionSlot->GetAnimationInfo(rkAnimationName, kSeqIDOrig, STR_COMMON_EFFECT_DETACH_SKIP, kCommonEffectDetachSkip);
	if(kCommonEffectDetachSkip.compare("TRUE") != 0)
	{
		if(false==m_kContCommonEffect.empty())
		{
			PgActor * pkTargetActor = NULL;
			CONT_COMMON_EFFECT::const_iterator eff_it = m_kContCommonEffect.begin();
			while(eff_it != m_kContCommonEffect.end())
			{
				if( pkTargetActor = g_kPilotMan.FindActor((*eff_it).first) )
				{
					pkTargetActor->DetachFrom((*eff_it).second);
				}
				++eff_it;
			}

			m_kContCommonEffect.clear();
		}

		std::string kCommonEffectID;
		if(pkActionSlot->GetAnimationInfo(rkAnimationName, kSeqIDOrig, STR_COMMON_EFFECT_ID, kCommonEffectID))
		{
			std::string kCommonEffectNode;
			if(false==pkActionSlot->GetAnimationInfo(rkAnimationName, kSeqIDOrig, STR_COMMON_EFFECT_NODE, kCommonEffectNode))
			{
				kCommonEffectNode = "char_root";
			}

			std::string kCommonEffectScale;
			float fScale = 1.0f;
			if(pkActionSlot->GetAnimationInfo(rkAnimationName, kSeqIDOrig, STR_COMMON_EFFECT_SCALE, kCommonEffectScale))
			{
				fScale = static_cast<float>( atof(kCommonEffectScale.c_str()) );
			}

			if(NiAVObject * pkParticle = g_kParticleMan.GetParticle(kCommonEffectID.c_str(),PgParticle::O_SCALE,fScale))
			{
				std::string kCommonEffectTarget;
				int iTargetType = ESTARGET_SELF;
				if(pkActionSlot->GetAnimationInfo(rkAnimationName, kSeqIDOrig, STR_COMMON_EFFECT_TARGET, kCommonEffectTarget))
				{
					iTargetType = atoi(kCommonEffectTarget.c_str());
				}

				if(iTargetType & ESTARGET_ENEMY)
				{
					if(PgAction * pkAction = GetAction())
					{
						PgActor * pkTargetActor = NULL;
						ActionTargetList::const_iterator target_it = pkAction->GetTargetList()->begin();
						while(target_it != pkAction->GetTargetList()->end())
						{
							if( pkTargetActor = g_kPilotMan.FindActor((*target_it).GetTargetPilotGUID()) )
							{
								int const iSlotNo = pkTargetActor->GetAttachSlotNo();
								if(pkTargetActor->AddNewParticle(kCommonEffectID.c_str(), iSlotNo, kCommonEffectNode.c_str(), fScale))
								{
									m_kContCommonEffect.push_back(SCommonEffectSlotInfo(pkTargetActor->GetGuid(),iSlotNo));
								}
							}
							++target_it;
						}
					}
				}
				if(iTargetType & ESTARGET_SELF)
				{
					int const iSlotNo = GetAttachSlotNo();
					if( AddNewParticle(kCommonEffectID.c_str(), iSlotNo, kCommonEffectNode.c_str(), fScale) )
					{
						m_kContCommonEffect.push_back(SCommonEffectSlotInfo(GetGuid(),iSlotNo));
					}
				}
			}
		}
	}

	// 슬롯에 지정된 사운드를 플레이하자
	if(bPlaySlotSound)
	{
		PlaySlotSound(rkAnimationName);
	}

	float	fAnimSpeed = 1.0f;
	std::string kSpeedStr;
	if(pkActionSlot->GetAnimationInfo(rkAnimationName,kSeqIDOrig,std::string(STR_SPEED),kSpeedStr))
	{
		fAnimSpeed = static_cast<float>(atof(kSpeedStr.c_str()));
	}

	SetAnimSpeed(fAnimSpeed);


	return true;
}

bool PgActor::PlaySlotSound(std::string const& rkSlotName)
{
	if( !PgActorUtil::IsCanPlaySound(this) )
	{
		return true;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.PlaySlotSound"), g_pkApp->GetFrameCount()));
	PgActionSlot* pkActionSlot = GetActionSlot();
	if (pkActionSlot == NULL)
		return false;

	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(rkSlotName, kSeqID))
	{
		return false;
	}
	
	PgActionSlot::stSoundInfo kSoundInfo;
	if(pkActionSlot->GetSound(rkSlotName, kSoundInfo))
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,kSoundInfo.m_kSoundID.c_str(),kSoundInfo.m_fVolume,kSoundInfo.m_fMinDist,kSoundInfo.m_fMaxDist,this);
	}

	return true;
}

void PgActor::ResetAnimation()
{
	if(!GetActorManager()) return;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ResetAnimation"), g_pkApp->GetFrameCount()));

	GetActorManager()->Reset();
	GetActorManager()->Update(0);

	// 파츠 애니도 리셋
	AMContainer::iterator itr = m_kSupplementAMContainer.begin();
	while(itr != m_kSupplementAMContainer.end())
	{
		PG_ASSERT_LOG(itr->m_spAM);
		if (itr->m_spAM)
		{
			itr->m_spAM->Reset();
			itr->m_spAM->Update(0);
		}
		++itr;
	}

	m_kSeqID = NiActorManager::INVALID_SEQUENCE_ID;

	if(g_pkWorld && IsVisible() == false)
	{
		m_bVisible = true;
		NiNode::Update(g_pkWorld->GetAccumTime(),true);
		m_bVisible = false;
	}
}

bool PgActor::ActivateAnimation(bool bAllowRepeat)
{
	NILOG(PGLOG_LOG, "[PgActor](%d) %s ActivateAnimation(%d)\n", g_pkApp->GetFrameCount(), MB(GetGuid().str()), m_kSeqID);
	NiActorManager *pkAM = GetActorManager();
	if(!pkAM)
	{
		return	false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ActivateAnimation"), g_pkApp->GetFrameCount()));

	PG_ASSERT_LOG(pkAM);
	if(pkAM->GetTargetAnimation() == m_kSeqID)
	{
		if(bAllowRepeat)
		{
			pkAM->Reset();
			pkAM->Update(0);
			pkAM->SetTargetAnimation(m_kSeqID);
			RegisterCallback(m_kSeqID);

			// 파츠 애니도 같이 플레이 한다.
			int	iSeqID;
			AMContainer::iterator itr = m_kSupplementAMContainer.begin();
			while(itr != m_kSupplementAMContainer.end())
			{
				PG_ASSERT_LOG(itr->m_spAM);
				if (itr->m_spAM)
				{
					itr->m_spAM->Reset();
					itr->m_spAM->Update(0);
				}
				iSeqID = itr->m_stCustomAniIDChangeSetting.GetChangedAniID(m_kSeqID);
				if (itr->m_spAM)
					itr->m_spAM->SetTargetAnimation(iSeqID);    
				++itr;
			}
		}
		return true;
	}

	bool bRet = pkAM->SetTargetAnimation(m_kSeqID);
	if(!bRet)
	{
		return false;
	}

	// 파츠 애니도 같이 플레이 한다.
	int	iSeqID;
	AMContainer::iterator itr = m_kSupplementAMContainer.begin();
	while(itr != m_kSupplementAMContainer.end())
	{
		iSeqID = itr->m_stCustomAniIDChangeSetting.GetChangedAniID(m_kSeqID);
		PG_ASSERT_LOG(itr->m_spAM);
		if (itr->m_spAM)
			itr->m_spAM->SetTargetAnimation(iSeqID);    
		++itr;
	}

	RegisterCallback(m_kSeqID);

	return true;
}
bool	PgActor::SyncActionOnAddUnit(PgPlayer* pkPlayer)
{ //AddUnit->로딩이 끝날때 Unit으로부터 브로드캐스팅 된 동작을 액터에게도 적용시켜 주자.
	if(pkPlayer == NULL || pkPlayer->UnitType() != UT_PLAYER || IsMyActor())
	{ //PC가 아니거나, 내 액터에게는 동기화 해줄 필요가 없다.
		return false;
	}

	int iAction = pkPlayer->GetActionID();
	CONT_DEFSKILL const *pkDefSkill = NULL;
	g_kTblDataMgr.GetContDef( pkDefSkill );
	if (iAction <= 0 || !pkDefSkill)
	{
		return false;
	}
	CONT_DEFSKILL::const_iterator iterSkillDef = pkDefSkill->find(iAction);
	if(iterSkillDef == pkDefSkill->end())
	{
		return false;
	}

	std::string strActionName(MB(iterSkillDef->second.chActionName));
	if(strActionName.compare("a_idle") == 0 || strActionName.compare("a_battle_idle") == 0 ||
		strActionName.compare("a_intro_idle") == 0 || strActionName.compare("a_opening") == 0 || strActionName.compare("a_rp_idle") == 0 )
	{ //아이들 액션은 동기화 해 줄 필요가 없다. 이동만 동기화 해주자.
		return false;
	}

	if( PgContentsBase::ms_pkContents )
	{	// PvP 카운트 다운시에는 동기화 할 필요 없다(동기화가 제대로 이루어지지 않음).
		if( PgContentsBase::STATUS_COUNTDOWN == PgContentsBase::ms_pkContents->GetContentsStatus() )
		{
			return false;
		}
	}

	NiPoint3 pt(pkPlayer->GetPos().x, pkPlayer->GetPos().y, pkPlayer->GetPos().z);
	ReserveTransitAction(MB(iterSkillDef->second.chActionName), pkPlayer->MoveDir());
	POINT3 temp = pkPlayer->GetDirectionVector(pkPlayer->MoveDir());
	NiPoint3 dir(temp.x, temp.y, temp.z);
	SetDirection(pkPlayer->MoveDir());
	ConcilDirection(dir);

	return true;
}
bool	PgActor::StartSyncMove(PgAction *pkSyncMoveNextAction)
{
	if(!g_pkWorld)
	{
		return	false;
	}

	m_fSyncMoveStartTime = g_pkWorld->GetAccumTime();	//	시작 시간
	m_kSyncMoveStartPos = GetPosition();
	//m_bNoFindPathNormal = true;	// 임시 SyncMove

	if(!pkSyncMoveNextAction) return false;


	PgAction	*pkSyncMove = CreateActionForTransitAction("a_sync_move");
	if(pkSyncMove)
	{
		lwAction(pkSyncMove).SetParamFloat(0,pkSyncMoveNextAction->GetActionStartPos().x);
		lwAction(pkSyncMove).SetParamFloat(1,pkSyncMoveNextAction->GetActionStartPos().y);
		lwAction(pkSyncMove).SetParamFloat(2,pkSyncMoveNextAction->GetActionStartPos().z);

		_PgOutputDebugString("StartSyncMove Actor:%s StartPos : (%f,%f,%f) Target:(%f,%f,%f) Time:%f\n", MB(GetPilotGuid().str()),GetPosition().x,GetPosition().y,GetPosition().z, pkSyncMoveNextAction->GetActionStartPos().x,pkSyncMoveNextAction->GetActionStartPos().y,pkSyncMoveNextAction->GetActionStartPos().z, NiGetCurrentTimeInSec());

		if(ProcessAction(pkSyncMove,IsMyActor()))
		{
			if(m_pkSyncMoveNextAction)
				g_kActionPool.ReleaseAction(m_pkSyncMoveNextAction);
			m_pkSyncMoveNextAction = NULL;
			m_pkSyncMoveNextAction = pkSyncMoveNextAction;
			return	true;
		}
	}

	return false;

}
bool	PgActor::UpdateSyncMove(float fSpeed,float fFrameTime)
{
	if(!m_pkSyncMoveNextAction || !g_pkWorld)
		return	false;

	float	fElapsedTime = g_pkWorld->GetAccumTime() - m_fSyncMoveStartTime;

	NiPoint3	kTargetPos = m_pkSyncMoveNextAction->GetActionStartPos();

	// 공중형 처리
	if (GetPilot() && GetPilot()->GetAbil(AT_MONSTER_TYPE) == EMONTYPE_FLYING)
	{
		kTargetPos.z = GetPosition().z;
	}

	//	Total Distance
	float	fTotalDistance = (kTargetPos-m_kSyncMoveStartPos).Length();

	NiPoint3	kDir = (kTargetPos-m_kSyncMoveStartPos);
	kDir.Unitize();

	m_kMovingDir = kDir;	
	m_kMovingDir.z = 0;
	m_kLookingDir = kDir;	// 임시 SyncMove
	//_PgOutputDebugString("[Set m_kMovingDir 0] Actor(%s) m_kMovingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);

	NiPoint3	kNextPos = m_kSyncMoveStartPos+kDir*fElapsedTime*fSpeed;

	float	fNextDistance = (kNextPos-m_kSyncMoveStartPos).Length();

	bool	bContinue = true;

	if(fNextDistance>=fTotalDistance)
	{
		kNextPos = kTargetPos;
		bContinue = false;
		//m_bNoFindPathNormal = false;	// 임시 SyncMove
	}

	SetPosition(kNextPos);

	if(!bContinue)
	{
		ClearReservedAction();
		ReserveTransitAction(m_pkSyncMoveNextAction);

		m_pkSyncMoveNextAction = NULL;
	}

	return	bContinue;
}

void PgActor::DoReservedAction(ReservedActionType eType,bool bTransitRightAway)
{
	char const *pcActionName = GetReservedAction(eType);
	if(pcActionName)
	{
		if(bTransitRightAway)
		{
			TransitAction(pcActionName);
		}
		else
		{
			ReserveTransitAction(pcActionName);
		}
	}
}

char const *PgActor::GetReservedAction(ReservedActionType eType)
{
	ReservedActionTable::const_iterator itr = m_kReservedAction.find(eType);
	if(itr == m_kReservedAction.end())
	{
		return NULL;
	}
	return itr->second.c_str();
}

//bool PgActor::AddToActionLayer(PgAction *pkAction)
//{
////	m_kActionLayerContainer.push_back(pkAction);
//	return true;
//}

void PgActor::ClearActionQueue()
{
	//if(GetPilot())
		//_PgOutputDebugString("Actor : %s Clear All Action Stack\n",MB(GetPilot()->GetName()));
	NILOG(PGLOG_LOG, "[PgActor] %s actor Clear Action Stack\n", MB(GetGuid().str()));

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ClearActionQueue"), g_pkApp->GetFrameCount()));
	BM::CAutoMutex kLock(m_kActionQueueMutex);

	for(ActionQueue::iterator itor = m_kActionQueue.begin(); itor != m_kActionQueue.end(); ++itor)
	{
		PgAction *pkAction = itor->GetAction();
		if(pkAction)
		{
			g_kActionPool.ReleaseAction(pkAction);
		}
	}
	m_kActionQueue.clear();
	m_bSync = false;
}

//float PgActor::GetEstimatedVelocity(float fEstimatedDist, float fRealDist, float fVelocity)
//{
//	if(fRealDist >= fEstimatedDist)
//	{
//		return fVelocity;
//	}
//
//	// TODO : 1초마다 적절한 속도를 예측해서 돌려주자.
//	float fDiff = fEstimatedDist - fRealDist;
//	float fRate = fEstimatedDist / fRealDist;
//
//	return fVelocity + fDiff / 5.0f;
//}

//float PgActor::GetVelocityForInterpolation(float fDelayedTime, float fElapsedTime, float fVelocity, float fDistance)
//{
//	PG_ASSERT_LOG(fDelayedTime > 0.0f);
//
//	float fNewVelocity = 2.0f * (((GetPilot->GetAbil(AT_C_MOVESPEED) * fElapsedTime) / fElapsedTime - fDelayedTime) - GetPilot->GetAbil(AT_C_MOVESPEED));
//}

bool PgActor::BeginSync(PgAction *pkAction, DWORD dwOvertime)
{
	if(!pkAction)
	{
		return false;
	}

	if(pkAction->AlreadySync())
	{
		return true;
	}

	PG_STAT(PgStatTimerF timerD(g_kActorStatGroup.GetStatInfo("PgActor.BeginSync"), g_pkApp->GetFrameCount()));
	pkAction->AlreadySync(true);

	// Action Start Pos가 (0, 0, 0)이면 Sync를 할 필요가 없다는 거다.
	NiPoint3 kActionStartPos = pkAction->GetActionStartPos();
	if(kActionStartPos == NiPoint3::ZERO)
	{
		return false;
	}

//	std::wstring kActionName = g_SkillDefMgr.GetActionName(pkAction->GetActionNo());
//	char chBalloon[1024];

	NiPoint3 kCurPos = GetPosition();
	float fDistance = (kCurPos - kActionStartPos).Length();
	//PG_ASSERT_LOG(fDistance > 0.0f)
	if(fDistance > PG_SYNC_DIST_SLIDE || fDistance < PG_SYNC_DIST_WARP)
	{
		SetPosition((NiPoint3) kActionStartPos);
		if(fDistance > PG_SYNC_DIST_SLIDE)
		{
			// 먼 곳에서 순간이동 할 때는, Alpha를 뺀다.
			m_AlphaTransitInfo.m_fTargetAlpha = 0.0f;
			SetTargetAlpha(0.0f, 1.0f, 0.15f);	
		}
		
//		sprintf_s(chBalloon, 1024, "[%s] Warp(%.4f): %s [%s]", MB(GetPilot()->GetName().c_str()), fDistance, MB(kActionName.c_str()), (pkAction->GetActionType() == "EFFECT" ? "EFFECT" : "NON-EFFECT"));
//		ShowChatBalloon(CT_NORMAL, chBalloon, 10000);
//		WriteToConsole("Action : %s [Distance : %.1f => Warp] [%s]\n", pkAction->GetID().c_str(), fDistance, MB(GetGuid().str()));
		SetMovingDelta(NX_ZERO);
		m_bSync = false;
		return true;
	}

	// if (fDistance <= PG_SYNC_DIST_SLIDE)
	// 시작 위치를 보간하며 달린다.

	// Sync 시작 시간을 적어 둔다.
//	WriteToConsole("Start Interpolating [%s, %s]\n", pkAction->GetID().c_str(), MB(GetGuid().str()));
//	sprintf_s(chBalloon, 1024, "[%s] Start Sync : %s [%s]", MB(GetPilot()->GetName().c_str()), MB(kActionName.c_str()), (pkAction->GetActionType() == "EFFECT" ? "EFFECT" : "NON-EFFECT"));
//	ShowChatBalloon(CT_NORMAL, chBalloon, 10000);

	m_bSync = true;
	m_fElapsedSyncTime = 0.0f;
	m_kSyncPositionStart = kCurPos;
	m_kSyncPositionTarget = kActionStartPos;

	NiPoint3 kMovingDir = (kCurPos - kActionStartPos);
	kMovingDir.Unitize();

	if(IsMeetFloor())
	{
		//	뛰기 모션으로 바꾸자
		NiActorManager *pkAM = GetActorManager();
		if(pkAM)
		{
			NiActorManager::SequenceID kSeqID;
			
			pkAM->GetCurAnimation();
			GetActionSlot()->GetAnimation(std::string("run"), kSeqID);
			if(pkAM->GetCurAnimation() != kSeqID)
			{
				if(SetTargetAnimation(std::string("run")) == false)
				{
					SetTargetAnimation(std::string("walk"));
				}
			}
		}
	}

//	NILOG(PGLOG_LOG,"Push To Action Stack For Sync Position  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()),
//		pkAction->GetID().c_str(), pkAction->GetActionNo(), pkAction->GetActionInstanceID(),
//		pkAction->GetTargetList()->size());
	
	// 보간할 때 속도는 지연시간에 의해 결정된다.
//	DWORD dwActionBirthTime = pkAction->GetBirthTime();
//	m_dwActionLatency = (dwActionBirthTime == 0 ? 0 : PgActor::GetSynchronizedTime() - dwActionBirthTime);

	return false;
}

bool PgActor::UpdateSync(float fFrameTime)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateInitPosition"), g_pkApp->GetFrameCount()));
	
	EUnitType eUType= GetPilot()->GetUnit()->UnitType();

	// Sync 시작, 끝 좌표를 설정.
	NiPoint3 kStartPos = m_kSyncPositionStart;
	NiPoint3 kSyncTargetPos = m_kSyncPositionTarget;
	if(eUType == UT_PET && IsRidingPet()) //라이딩펫 탑승시에 펫의 위치 동기화는 생략해주자
	{
		kSyncTargetPos = kStartPos;
	}

	NiPoint3 kMovVector = (kSyncTargetPos - kStartPos);
	kMovVector.Unitize();
	
	// 아무리 느려도 0.5초 안에 보간 되게 하자
	static float fCriticalTime = 0.05f;
	float fDistance1 = (kStartPos - kSyncTargetPos).Length();
	if(eUType == UT_MONSTER)
	{
		// 몬스터는 약간 천천히 보간하자..
		fCriticalTime *= 1.5f;
	}
	else if(eUType == UT_PET && IsRidingPet())
	{
		fCriticalTime = 0;
	}

	m_fElapsedSyncTime += fFrameTime;

	float fSpeed = static_cast<float>(GetPilot()->GetAbil(AT_C_MOVESPEED));
	if(g_pkWorld)
	{
		if(g_pkWorld->GetAttr() & GATTR_VILLAGE)
		{
			fSpeed += static_cast<float>(GetPilot()->GetAbil(AT_C_VILLAGE_MOVESPEED));
		}
	}
	
	// 이동 속도 계산
	float fMovingSpeed = NiMax(fSpeed * 2.0f, fDistance1 / fCriticalTime);
	float fUnitizedDiffHeight = kMovVector.z;

	kMovVector.z = 0.0f;
	kMovVector.Unitize();
	kMovVector = kMovVector * fMovingSpeed * fFrameTime;
	kMovVector.z += fUnitizedDiffHeight * fMovingSpeed * fFrameTime;
	NiPoint3 kNextPos = GetPosition() + kMovVector;

	float fDistance2 = (kStartPos - kNextPos).Length();

	//WriteToConsole("Now Interpolating : %.1f, %.1f, %.1f\tAdjusted Speed : %.1f\tDistance %.4f[%s]\n", kMovVector.x, kMovVector.y, kMovVector.z, fMovingSpeed, fDistance2, MB(GetGuid().str()));
	
	// TODO : 장애물 때문에 대각선이나 한 층 아래로 걷게 되었다면, 당연히 튄다 => 순간 이동?
	// Z축에 대한 값을 어떻게 조절 할 것인가. 
	
	// 시작위치에서 다음 좌표까지의 거리가, 시작위치에서 목표좌표까지의 거리보다 크다면, 지나친것이다.
	// 보간 시간이 지났으면 순간이동 시키고 바로 보간을 끝낸다.
	if(fDistance1 <= fDistance2 || (kMovVector == NiPoint3::ZERO) || m_fElapsedSyncTime >= fCriticalTime)
	{
		// 목표 지점 도달, 해당 액션을 한다.
		SetPosition(kSyncTargetPos);
		m_fElapsedSyncTime = 0.0f;
		m_bSync = false;
		if(m_pkAction)
		{
			SetLookingDirection(m_pkAction->GetDirection(), true);
		}
//		if(m_pkAction)
//		{
//			char chBalloon[1024];
//			
//			ActionQueue::iterator itr = m_kActionQueue.begin();
//			if(itr->GetAction())
//			{
//				sprintf_s(chBalloon, 1024, "[%s] End Sync [Next Action : %s]", MB(GetPilot()->GetName().c_str()), MB(g_SkillDefMgr.GetActionName(itr->GetAction()->GetActionNo())));
//			}
//			else
//			{
//				sprintf_s(chBalloon, 1024, "[%s] End Sync!! [NextAction Direction : %d]", MB(GetPilot()->GetName().c_str()), itr->GetDirection());
//			}
//			
//			ShowChatBalloon(CT_NORMAL, chBalloon, 10000);
//
//			WriteToConsole("Arrived [%s]'s Start Pos Now [%.1f, %.1f, %.1f] [%s]\n", m_pkAction->GetID().c_str(), kSyncTargetPos.x, kSyncTargetPos.y, kSyncTargetPos.z, MB(GetGuid().str()));
//		}

		ProcessActionQueue();
		return true;
	}
	
	// TODO : move로 움직이는 것이 나을지, SetPosition으로 움직이는게 나을지.
	NxVec3 kMovingDelta;
	NxU32 collisionFlag = 0;
	NiPhysXTypes::NiPoint3ToNxVec3(kMovVector, kMovingDelta);
	m_pkController->move(kMovingDelta, m_uiActiveGrp, 0.001f, collisionFlag);

	//WriteToConsole("[%s]'s interpolated pos : [%.1f, %.1f, %.1f]\n", m_pkAction->GetID().c_str(), m_pkController->getDebugPosition().x, m_pkController->getDebugPosition().y, m_pkController->getDebugPosition().z);
	//_PgOutputDebugString("[Set m_kMovingDir 3] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);

	if(eUType == UT_PLAYER)
	{
		kMovVector.Unitize();
		m_kMovingDir = kMovVector;
		m_kMovingDir.z = 0;
		BYTE byDir = GetDirFromMovingVector(m_kMovingDir);
		SetLookingDirection(byDir, true);
	}

	return false;
}

//NiPoint3 PgActor::GetCubicSplines(NiPoint3 &rkStartPos, NiPoint3 &rkEndPos, NiPoint3 &rkVelocity, float fDeltaT)
//{
//	kVelocity.Unitize();
//	NiPoint3 kPos1 = rkStartPos;
//	NiPoint3 kPos2 = rkStartPos + kVelocity;
//	NiPoint3 kPos3 = rkEndPos - kVelocity;
//	NiPoint3 kPos4 = rkEndPos;
//
//	float fA = kPos4.x - 3.0f * kPos3.x + 3.0f * kPos2.x - kPos1.x;
//	float fB = 3.0f * kPos3.x - 6.0f * kPos2.x + 3.0f * kPos1.x;
//	float fC = 3.0f * kPos2.x - 3.0f * kPos1.x;
//	float fD = kPos1.x;
//
//	float fE = kPos4.y - 3.0f * kPos3.y + 3.0f * kPos2.y - kPos1.y;
//	float fF = 3.0f * kPos3.y - 6.0f * kPos2.y + 3.0f * kPos1.y;
//	float fG = 3.0f * kPos2.y - 3.0f * kPos1.y;
//	float fH = kPos1.y;
//
//	float fI = kPos4.y - 3.0f * kPos3.y + 3.0f * kPos2.y - kPos1.y;
//	float fJ = 3.0f * kPos3.y - 6.0f * kPos2.y + 3.0f * kPos1.y;
//	float fK = 3.0f * kPos2.y - 3.0f * kPos1.y;
//	float fL = kPos1.y;
//
//	float fX = fA * NiPow(fDeltaT, 3.0f) + NiPow(fB, 2.0f) + fC * fDeltaT + fD;
//	float fY = fE * NiPow(fDeltaT, 3.0f) + NiPow(fF, 3.0f) + fG * fDeltaT + fH;
//	float fZ = fI * NiPow(fDeltaT, 3.0f) + NiPow(fJ, 3.0f) + fK * fDeltaT + fL;
//
//	return NiPoint3(fX, fY, fZ);
//}

//BYTE PgActor::GetReverseDirection(BYTE byDirection)
//{
//	if(byDirection > DIR_DOWN)
//	{
//
//	}
//}
void	PgActor::SetStun(bool const bTrue)
{
	if(!m_bStun && bTrue)
	{
		ReserveTransitAction("a_stun");
	}

	m_bStun = bTrue;
}

bool PgActor::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	if(!pkAction)
	{
		return	false;
	}

	bool bIsMyActor = IsMyActor();

	if(pkAction && pkAction->GetEnable() && IsNowFollowing())
	{
		//	따라갈수 없는 액션일 경우, 따라하면 안된다.
		if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_FOLLOW) == false)
		{
			if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) || pkAction->GetActionType() == "EFFECT" )	//	위치 이동하는 액션 또는 Effect라면, 따라가기를 멈춘다
			{
				RequestFollowActor(GetFollowingTargetGUID(),EFollow_Cancel);
			}
			else
			{
				g_kActionPool.ReleaseAction(pkAction);
				return	false;
			}
		}
	}

	if(bInvalidateDirection)
	{
		BYTE byNewDir = pkAction->GetDirection();
		if(byNewDir != DIR_NONE)
		{
			if(pkAction->IsRecord())
			{
				InvalidateDirection();
			}
		}
	}
	else if(pkAction->AlreadySync()) // && !bIsMyActor
	{
		if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
		{
			//_PgOutputDebugString("[PgActor.Action] (Other Actor) Action: %s, Arrived Direction : %d, Action Term : %u\n",  pkAction->GetID().c_str(), pkAction->GetDirection(), pkAction->GetActionTerm());
			m_kMovingDir = NiPoint3::ZERO;
			SetDirection(pkAction->GetDirection());
		}
		else
		{
			//_PgOutputDebugString("[PgActor.Action] (Other Actor) Action: %s, Arrived Looking Direction : %d, Action Term : %u\n", pkAction->GetID().c_str(), pkAction->GetDirection(), pkAction->GetActionTerm());
			SetDirection(DIR_NONE);
		}

		SetLookingDirection(pkAction->GetDirection(), true);
	}

	// 실제 Action을 전이하는 곳.
	bool bActionReturn = DoAction(pkAction,bForceToTransit);
	if(!bActionReturn)
	{
		if( ( bIsMyActor && IsNowFollowing() ) || IsMyPet() )
		{
			if( false == CheckRequirementForAction(pkAction) )
			{
				if( pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) )	//	위치 이동하는 액션 실패시 따라가기를 멈춘다
				{
					RequestFollowActor(GetFollowingTargetGUID(),EFollow_Cancel);
				}
			}
		}

		if(m_pkAction && m_pkAction->IsChangeToNextActionOnNextUpdate() && m_pkAction->GetNextActionName() == pkAction->GetID())
		{
			m_pkAction->SetNextActionName(ACTIONNAME_IDLE);
		}
		bool const bIsMySubPlayer = IsMySubPlayer();
		if( true == bIsMySubPlayer )
		{
			m_pkAction->SetNextActionName(ACTIONNAME_IDLE);
		}

		if(bIsMyActor && m_pkAction && m_pkAction->GetActionOptionEnable(PgAction::AO_DO_MONITOR))
		{
			// Monitoring하는 액션이라면 방향을 Broadcast한다.
			g_kPilotMan.BroadcastDirection(m_pkPilot, m_byMovingDirection);
			//_PgOutputDebugString("[PgActor.Action] Broadcasted Direction(Monitor) : %d \n", m_byMovingDirection);
		}

		// 액션 제거.
		g_kActionPool.ReleaseAction(pkAction);
	}

	return bActionReturn;
}

void	PgActor::OnTargetListModified(PgAction *pkNextAction)
{
	if( m_pkAction == NULL ||
		(pkNextAction->GetActionNo() != m_pkAction->GetActionNo()) || 
		(m_pkAction->GetActionParam() != ESS_FIRE) ) 
	{
		pkNextAction->OnTargetListModified(this,false);	//	새 타겟리스트에 대한 스크립트 처리
		return ;
	}

	//	이것은 TargetList 가 바뀌었음을 의미한다.
	m_pkAction->OnTargetListModified(this,true);	//	기존 타겟리스트에 대한 스크립트 처리
	m_pkAction->GetTargetList()->ApplyActionEffects();

	m_pkAction->SetParamAsPacket(pkNextAction->GetParamAsPacket());
	pkNextAction->SetParamAsPacket(NULL, false);

	m_pkAction->SetActionInstanceID(pkNextAction->GetActionInstanceID());
	m_pkAction->SetTargetList(*pkNextAction->GetTargetList());
	m_pkAction->OnTargetListModified(this,false);	//	새 타겟리스트에 대한 스크립트 처리

	pkNextAction->ClearTargetList();

}
bool	PgActor::ProcessToggleAction(PgAction *pkNextAction)
{
	//	만약 현재상태가 Activated 된 상태라면, Deactivated 로 바꾸고
	//	브로드캐스팅만 한 후 그냥 리턴한다.
	if(GetActionToggleState(pkNextAction->GetActionNo()))
	{
		if(!IsStun())
		{
			ActionToggleStateChange(pkNextAction->GetActionNo(),false);
			pkNextAction->SetActionParam(ESS_TOGGLE_OFF);
			g_kPilotMan.Broadcast(m_pkPilot, pkNextAction, true);

			StartSkillCoolTime(pkNextAction->GetActionNo());	//	쿨타임 시작
			return false;
		}
	}

	return	true;
}
bool	PgActor::CheckRequirementForAction(PgAction *pkNextAction, bool const bShowFailMsg)
{
	PG_ASSERT_LOG(pkNextAction);
	if(!pkNextAction)
	{
		return	false;
	}

	CSkillDef const* pNextSkillDef = pkNextAction->GetSkillDef();
	PG_ASSERT_LOG(pNextSkillDef);
	if(!pNextSkillDef)
	{
		return	false;
	}

#ifndef EXTERNAL_RELEASE
	bool bIsSingleMode = g_pkApp->IsSingleMode();
	if(bIsSingleMode)
	{
		return	true;	//	싱글모드에서는 무조건 true
	}
#endif

	if(pkNextAction->GetActionType() != "IDLE")	//아이들이 아닐 경우에만
	{
		//	스턴 상태에서는 아무 행동도 할 수 없음.
		if(pkNextAction->GetActionType() != "EFFECT" && IsStun())
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(242)), 2, true);
			}
			return	false;
		}
	}
	
	PgPilot* pkPilot = GetPilot();
	if(!pkPilot)
	{
		return false;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return false;
	}
	bool const bIsMyPet = IsMyPet();
	bool const bIsMyActor = IsMyActor();
	bool const bIsMySubPlayer = IsMySubPlayer();
	
	bool bIsSubPlayer = false;
	PgActor* pkCallerActor = NULL;
	switch( pkUnit->UnitType() )
	{
	case UT_SUB_PLAYER:
		{// 보조 캐릭터 타입은 Caller에게 종속되기 때문에
			pkCallerActor = g_kPilotMan.FindActor( pkUnit->Caller() );
			bIsSubPlayer = true;
		}break;
	default:
		{
		}break;
	}

	if(bIsMyActor
		|| bIsMySubPlayer
		)
	{
		if(bIsMySubPlayer && IsHide()) //쌍둥이 액터가 숨겨진 상태라면 액션이 발동되어서는 안된다 (ex: 내가 펫에 탑승 중일 때)
		{
			return false;
		}

		//	배우지 않았다면 사용할 수가 없당.
		PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(pkNextAction));
		if(pFound)
		{
			if(false == lua_tinker::call<bool>("IsActivitySkill"))
			{
				return false;
			}
		}
		if(pFound && pFound->m_bLearned == false)
		{
			if(bShowFailMsg)
			{
				//lwAddWarnDataStr(lwWString(TTW(300)), 2);
				g_kChatMgrClient.ShowNoticeUI(TTW(pNextSkillDef->GetAbil(AT_IS_COUPLE_SKILL) ? 450080 :  300), 2, true);
			}
			return	false;
		}
		if(pFound && pFound->m_pkSkillDef)
		{
			pNextSkillDef = pFound->m_pkSkillDef;
		}
	}
	else
	{
		if(bIsMyPet)
		{
			PgPet* pkPet = dynamic_cast<PgPet*>(pkUnit);

			PgMySkill* pkMyPetSkill = pkPet->GetMySkill();
			if(!pkMyPetSkill)
			{
				return false;
			}

			if(EST_GENERAL != pNextSkillDef->GetAbil(AT_TYPE))
			{
				// 1레벨의 스킬을 배웠는지 체크
				if( !pkMyPetSkill->GetLearnedSkill(pNextSkillDef->No()) )
				{
					if(strcmp(pkNextAction->GetID().c_str(), ACTIONNAME_RUN_PET))
					{
						return false;	
					}
				}
			}
		}
		else if(!bIsSubPlayer)
		{
			return false;
		}
	}

	PgControlUnit *pkControlUnit = dynamic_cast<PgControlUnit*>(pkUnit);
	if( bIsSubPlayer 
		&& NULL != pkCallerActor )
	{
		PgPilot* pkCallerPilot = pkCallerActor->GetPilot();
		if( NULL == pkCallerPilot )
		{
			return false;
		}
		CUnit* pkCallerUnit = pkCallerPilot->GetUnit();
		if( NULL == pkCallerUnit )
		{
			return false;
		}
		pkControlUnit = dynamic_cast<PgControlUnit*>(pkCallerUnit);
	}
	PG_ASSERT_LOG(pkControlUnit);
	if(!pkControlUnit)
	{
		return	false;
	}

	if(bIsMyActor
		|| bIsMySubPlayer
		)
	{
		//	사용 조건 체크
		{
			int const iUnuseableGround = pkNextAction->GetAbil(AT_CANT_USE_THIS_GATTR_FLAG);
			if(0 < iUnuseableGround)
			{// NextAction이 특정 지역에서 사용불가하고
				if(g_pkWorld)
				{
					if(g_pkWorld->GetAttr() & iUnuseableGround)
					{// 현재 장소가, 사용 불가 지역이라면
						lwAddWarnDataStr(lwWString(TTW(244)), 2, true);
						return false; // NextAction으로 전이할 수 없다
					}
				}
			}
			
			if(PgItemEx::IT_JOB_TOOL == GetEquippedWeaponType())
			{// 직업 스킬을 가지고 있고,
				int	const iWeaponLimit = pNextSkillDef->GetAbil(AT_WEAPON_LIMIT);
				if(0 < iWeaponLimit)
				{// 무기 제한이 있다면 사용할수 없다.
					lwAddWarnDataStr(lwWString(TTW(63)), 2, true);
					return false; // NextAction으로 전이할 수 없다
				}
			}

			if(pNextSkillDef->GetType()==EST_ACTIVE && pNextSkillDef->GetTargetType()&ESTARGET_SUMMONED)
			{
				if(0==GetTotalSummonedCount( GetUnit() ))
				{
					lwAddWarnDataStr(lwWString(TTW(792099)), 2, true);
					return false;
				}
			}
			
		}

		//	Caster State Check
		//	Only Check when this action is skill
		if (pNextSkillDef->GetAbil(AT_SKILL_KIND) != ESK_NONE
			|| pNextSkillDef->GetAbil(AT_ITEM_SKILL) != 0 )
		{
			int kCasterState = (pNextSkillDef->GetAbil(AT_CASTER_STATE));
			if(kCasterState == 0)
			{
				kCasterState = ECaster_State_OnGround;
			}

			const	bool	bIsMeetFloor = IsMeetFloor();
			if(bIsMeetFloor && !(kCasterState&(((int)ECaster_State_OnGround))) )
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(239)),2, true);
				}
				return	false;
			}
			if(!bIsMeetFloor && !(kCasterState&(((int)ECaster_State_Jumping))) )
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(240)),2, true);
				}
				return	false;
			}
			if(GetActorDead() && !(kCasterState&(((int)ECaster_State_Dead))) )
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(241)),2, true);
				}
				return	false;
			}
		}

		if(IsRidingPet() && pNextSkillDef->GetAbil(AT_SKILL_ATT))
		{ //탑승 상태이고 전투스킬 사용을 시도하면 실패
			return false;
		}
		

		if( !bIsSubPlayer )
		{//	Class Limit 체크
			int	const	  iMyClassID = pkControlUnit->GetAbil(AT_CLASS);
			__int64	const iClassLimit = pNextSkillDef->GetAbil64(AT_CLASSLIMIT);
			if(iClassLimit>0 && IS_CLASS_LIMIT(iClassLimit,iMyClassID)==false)
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(238)),2, true);
				}
				return	false;
			}
		}

		//	Level Limit 체크
		int	const iLevelLimit = pNextSkillDef->GetAbil(AT_LEVELLIMIT);
		if(iLevelLimit>0 && iLevelLimit > pkControlUnit->GetAbil(AT_LEVEL))
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(237)),2, true);
			}
			return	false;
		}

		//	WeaponLimit 체크
		unsigned int uiMyWeaponType = m_uiMyWeaponType;
		if(bIsSubPlayer
			&& pkCallerActor
			)
		{
			uiMyWeaponType = pkCallerActor->GetEquippedWeaponType();
		}
		
		int	const iWeaponLimit = pNextSkillDef->GetAbil(AT_WEAPON_LIMIT);
		if(iWeaponLimit>0 && (iWeaponLimit&(1<<(uiMyWeaponType-1)))==0)
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(63)),2, true);
			}
			return	false;
		}

		//	필요스킬 체크(NeedSkill)
		int	const iMaxNeedSkill = 3;
		for(int i = 0; i < iMaxNeedSkill; ++i)
		{
			int	const iNeedSkill = pNextSkillDef->GetAbil(AT_NEED_SKILL_01+i);
			if(0 < iNeedSkill)
			{
				PgSkillTree::stTreeNode *pkSkillNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iNeedSkill));
				if(pkSkillNode == NULL)
				{
					return	false;	
				}

				if(false == pkSkillNode->m_bLearned || (unsigned int)iNeedSkill > pkSkillNode->m_ulSkillNo + pkSkillNode->m_iOverSkillLevel)
				{
					return	false;
				}

				// 스킬트리에는 임시로 레벨을 올려둔 상태에서 실제 스킬을 사용 하는 경우
				if(true == pkSkillNode->IsTemporaryLevelChanged())
	            {
					if(pkControlUnit)
                    {
						int const iLearnedSkill = pkControlUnit->GetMySkill()->GetLearnedSkill(iNeedSkill, true);
						if(iLearnedSkill < iNeedSkill)
                        {
			                return false;
		                }
	                }
		        }
	        }
	    }
    }

	bool bCheckHP_MP = CheckHPMPForAction(pNextSkillDef, pkControlUnit, bShowFailMsg, true);

	if(!bCheckHP_MP)
	{
			return false;
	}

	//	이동 액션만 가능한 상태라면
	if(IsOnlyMoveAction())	
	{
		std::string kActionID = pkNextAction->GetID();
		if(pkNextAction->GetActionType() != "IDLE" 
			&&  pkNextAction->GetActionType() != "MOVE"
			&&  pkNextAction->GetActionOptionEnable(PgAction::AO_KIND_OF_TRIGGER_ACTION) == false	// 맵트리거용 액션이 아니라면
			)
		{
			NILOG(PGLOG_LOG,"OnlyMoveAction  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()),pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(),pkNextAction->GetTargetList()->size());
			return false;
		}
	}
	//	기본 공격만 가능한 상태라면
	if(IsOnlyDefaultAttack())
	{
		if(ESK_NONE != pkNextAction->GetAbil(AT_SKILL_KIND))
		{
			NILOG(PGLOG_LOG,"OnlyDefaultAttack  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()), pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());
			ClearActionState();
			return false;
		}
	}


	// 쿨타임이 글로벌 쿨타임인가?
	bool bIsGobalCoolTime = false;
	
	//	쿨타임 체크
	if(IsInCoolTime(pkNextAction->GetActionNo(), bIsGobalCoolTime))
	{
		bool bSkillKind = pkNextAction->GetSkillDef()->GetAbil(AT_SKILL_KIND) != ESK_NONE;
		
		if(pkControlUnit->IsUnitType(UT_PET))
		{
			bSkillKind = true;
		}

		if( pkNextAction->GetSkillDef()->GetAbil(AT_ITEM_SKILL_EXPLAIN_ID) )
		{
			bSkillKind = true;
		}

		if(pkNextAction->GetSkillDef()==NULL || bSkillKind)
		{	//펫 AI가 동작 중일 때는 표시하지 않음
			//쿨타임 체크 메시지 // 글로벌 쿨타임인 경우는 메시지 출력 하지 않는다.
			if(bShowFailMsg && (0==pkControlUnit->GetAbil(AT_AUTO_PET_SKILL)) )// && (false == bIsGobalCoolTime))
			{
				g_kChatMgrClient.ShowNoticeUI(TTW(235), 2, true, true);
			}
		}
		if( true == bShowFailMsg )
		{//여기서 콤보 스킬일 경우 쿨타임 에러 메세지 출력해 주자.
			PgPilot *pkPilot = GetPilot();
			if( NULL != pkPilot )
			{
				if( pkPilot->IsHaveComboAction( pkNextAction->GetActionNo() ) )
				{
					g_kChatMgrClient.ShowNoticeUI(TTW(235), 2, true, true);
				}
			}
		}
		// 일반 동작 이고 글로벌 쿨타임이 아닌 경우
		if(!(EST_GENERAL == pNextSkillDef->GetAbil(AT_TYPE) && bIsGobalCoolTime))
		{
			NILOG(PGLOG_LOG,"CoolTime  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()), pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());
			return false;
		}
	}

	SSFilter_Result	kResult;
	if(!pkControlUnit->CheckSkillFilter(pkNextAction->GetActionNo(),&kResult))
	{
		const wchar_t *pkEffectNameStr = NULL;
		GET_DEF(CEffectDefMgr, kEffectDefMgr);
		if ( true == GetEffectName(kEffectDefMgr.GetCallEffectNum(kResult.iCauseID), pkEffectNameStr) )
		{
			std::wstring const kEffectName(pkEffectNameStr);
			if ( kEffectName.size() )
			{
				switch(kResult.eResult)
				{
				case SSFilter_Result::ESFResult_NeedEffect:
					{
						if( true == bShowFailMsg )
						{
							std::wstring wstrMsg;
							WstringFormat( wstrMsg, MAX_PATH, TTW(93).c_str(), kEffectName.c_str() );
							Notice_Show( wstrMsg, EL_Level2, true);
						}
					}break;
				case SSFilter_Result::ESFResult_LimitEffect:
					{
						if( true == bShowFailMsg )
						{
							std::wstring wstrMsg;
							WstringFormat( wstrMsg, MAX_PATH, TTW(94).c_str(), kEffectName.c_str() );
							Notice_Show( wstrMsg, EL_Level2, true );
						}
					}break;
				}
			}
		}
		return	false;
	}	
	
	// 키가 때어진 것이라면 체크하지 않아야한다.
	if(!pkNextAction->GetEnable())
	{
		return true;
	}

	return	true;
}
void	PgActor::OnCastingCompleted(PgAction *pkNextAction)
{
	//	캐스팅이 완료되어 실제 시전을 하는 액션이다.
	//	현재 액션을 지우고 NextAction 으로 바꾼다.
	if(m_pkAction)
	{
		m_pkAction->CopyParamTo(pkNextAction);
		pkNextAction->SetSlot(m_pkAction->GetCurrentSlot());
		g_kActionPool.ReleaseAction(m_pkAction);
	}
	m_pkAction = pkNextAction;
	m_pkAction->OnCastingCompleted(this,m_pkAction);

	SetIgonreDamageEffect(m_pkAction);

	NILOG(PGLOG_LOG,"Casting Complete  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()), pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());
}

bool	PgActor::ProcessLeaveCurrentAction(PgAction *pkNextAction)
{
	if(NULL == pkNextAction
		|| NULL == m_pkAction)
	{
		return false;
	}

	bool	bSuccessfulLeave = false;
	
	if(pkNextAction->GetActionType() == "EFFECT" && m_pkAction->GetActionType() != "EFFECT")
	{
		bSuccessfulLeave = true;
	}
	else
		bSuccessfulLeave = m_pkAction->LeaveFSM(this, pkNextAction);
	
	if(!bSuccessfulLeave && IsMyActor())
	{
		if(m_pkAction->GetActionParam() == ESS_CASTTIME)		//	현재 캐스팅 중이었다면, 무조건 끊을 수 있다.
		{
			bSuccessfulLeave = true;
		}
	}


	if(bSuccessfulLeave)
	{
		PgActionTargetList* pkTargetList = m_pkAction->GetTargetList();
		if(pkTargetList)
		{
			pkTargetList->ApplyActionEffects();
		}
		
		m_pkAction->CleanUpFSM(this, pkNextAction);
	}

	if(!pkNextAction->GetEnable() || !bSuccessfulLeave)
	{
		// 떼어진 키 이거나, Enter로 진입 불가능
		NILOG(PGLOG_LOG,"Next Action GetEnable or LeaveFSM false  Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()), pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());
		return false;
	}

	return	true;
}
void	PgActor::SetActionParam(PgAction	*pkNextAction)
{
	//	캐스팅 타임이 있는 액션일 경우
	//	ActionParam 을 PgAction::AP_CASTING 로 설정한다.
	//	기타의 경우 PgAction::AP_FIRE 로 설정한다.
	const CSkillDef	*pkSkillDef = pkNextAction->GetSkillDef();
	if(pkSkillDef && pkSkillDef->GetAbil(AT_CASTTYPE) == E_SCAST_CASTSHOT)
	{
		pkNextAction->SetActionParam(ESS_CASTTIME);
	}
	else
	{
		if(pkSkillDef && pkSkillDef->GetType() == EST_TOGGLE)
		{
			pkNextAction->SetActionParam(ESS_TOGGLE_ON);
		}
		else
		{
			pkNextAction->SetActionParam(ESS_FIRE);
		}
	}
}
void	PgActor::SetIgonreDamageEffect(PgAction	*pkNextAction)
{
	ClearIgnoreEffectList();

	bool bAddIgnoreDamageEffect = pkNextAction->GetActionOptionEnable(PgAction::AO_ALWAYS_IGNORE_DMG_EFFECT);
	if( pkNextAction->GetActionParam() == ESS_FIRE )
	{
		bAddIgnoreDamageEffect = pkNextAction->GetActionOptionEnable(PgAction::AO_IGNORE_DMG_EFFECT) || bAddIgnoreDamageEffect; // 대미지 이펙트를 무시할것인다.
	}

	if( bAddIgnoreDamageEffect )
	{
		AddIgnoreEffect(ACTIONEFFECT_DMG);
		//AddIgnoreEffect(100002001);//a_lightning_default
		//AddIgnoreEffect(100002201);//a_knockdown
		//AddIgnoreEffect(100002301);//a_touch_dmg
		//AddIgnoreEffect(100004601);//a_knock_back
		//AddIgnoreEffect(100004701);//a_blow_down
		//AddIgnoreEffect(100004702);//a_down_damage
		//AddIgnoreEffect(100004703);//a_down_damage_2
		//AddIgnoreEffect(100004801);//a_blow_up_small
		//AddIgnoreEffect(100004901);//a_blow_up_medium
		//AddIgnoreEffect(100005001);//a_blow_up_large
		//AddIgnoreEffect(100005424);//a_knock_back_small
		//AddIgnoreEffect(100005429);//a_throw_down
		//AddIgnoreEffect(100005430);//a_float_dmg

		//AddIgnoreEffect(100005431);//a_hold_down
		//AddIgnoreEffect(100005432);//a_lightning_throw_down
		//AddIgnoreEffect(100005433);//a_lightning_float_dmg
		//AddIgnoreEffect(100005451);//a_float_dmg_small
		//AddIgnoreEffect(100005452);//a_throw_down2
		//AddIgnoreEffect(100005453);//a_float_dmg_very_small
		//AddIgnoreEffect(100007501);//a_snatch_down
		//AddIgnoreEffect(100007601);//a_time_freeze_effect
		//AddIgnoreEffect(100009001);//a_reverse_blow_up
		//AddIgnoreEffect(100009002);//a_reverse_blow_up_fast

		//AddIgnoreEffect(100011201);//a_Blind
		//AddIgnoreEffect(101020101);//a_stun
	}
}
void	PgActor::PlayAnimation(PgAction *pkNextAction)
{
	// 액션에 해당하는 애니를 플레이 한다.
	std::string kAnimName;
	if(pkNextAction->GetActionOptionEnable(PgAction::AO_NO_PLAY_ANIMATION) == false && 
		pkNextAction->GetActionName(kAnimName))
	{


		if(IsVisible() == false)
		{	
			ResetAnimation();
		}

		SetTargetAnimation(kAnimName,true,pkNextAction->GetActionOptionEnable(PgAction::AO_NO_RANDOM_ANIMATION));

		//	액션이 공속 어빌의 영향을 받는 액션이라면, 공속을 적용하여 애니메이션 스피드를 변경한다.
		if(pkNextAction->GetAbil(AT_APPLY_ATTACK_SPEED) == 1)
		{
			float	fAttackSpeed = GetPilot()->GetAbil(AT_C_ATTACK_SPEED)/ABILITY_RATE_VALUE_FLOAT;
			SetAnimSpeed(GetAnimSpeed()*fAttackSpeed);
		}
	}
}

void	PgActor::SetSeeFrontAttribute()
{
	//	SEE_FRONT 속성 처리
	std::string kSeeFront;
	if(GetAnimationInfo(std::string("SEE_FRONT"),GetAniSequenceID(),kSeeFront))
	{
		if(kSeeFront == "TRUE")
		{
			//	전방을 보도록 한다.
			SetLookingDirection(DIR_DOWN, true);
		}
	}
}
void	PgActor::AddActionEntityToFollowers(PgActionEntity& kActionEntity)
{

	//	나를 따라오는 넘들이 있다면 그넘들에게도 액션을 넣어준다.
	if(m_kFollowInfo.m_kFollowingMeActorCont.size()>0)
	{
		PgPilot	*pkPilot = NULL;
		PgActor	*pkActor = NULL;

		for(GUIDCont::iterator itor = m_kFollowInfo.m_kFollowingMeActorCont.begin();itor != m_kFollowInfo.m_kFollowingMeActorCont.end(); )
		{	
			BM::GUID kGUID = *itor;

			pkPilot = g_kPilotMan.FindPilot(kGUID);
			if(pkPilot)
			{
				pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					PgActionEntity	kNewActionEntity = kActionEntity.CreateCopy();
					
					PgAction *pkAction = kNewActionEntity.GetAction();
					if(pkAction)
					{
						//	따라갈수 없는 액션일 경우, Idle 로 바꾼다.
						if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_FOLLOW) == false)	
						{
							NiPoint3 kActionPos = pkAction->GetActionStartPos();

							PgAction *pkIdleAction = CreateActionForTransitAction(ACTIONNAME_IDLE,true,&kActionPos);

							pkIdleAction->SetActionTerm(pkAction->GetActionTerm());

							g_kActionPool.ReleaseAction(pkAction);
							kNewActionEntity.SetAction(pkIdleAction);
						}
					}

					pkActor->AddActionEntity(kNewActionEntity);
					
					++itor;
					continue;
				}
			}

			//	액터가 없당.
			itor = m_kFollowInfo.m_kFollowingMeActorCont.erase(itor);
		}
	}
}
void	PgActor::AddActionEntity(PgAction *pkAction,Direction kDirection)
{
	AddActionEntity(PgActionEntity(pkAction, kDirection));
}
void	PgActor::AddActionEntity(PgActionEntity& kActionEntity)
{
	BM::CAutoMutex kLock(m_kActionQueueMutex);

	if(kActionEntity.GetAction())
	{
		kActionEntity.GetAction()->SetAddToActionEntity(false);
	}

	if(IsNowFollowing())
	{
		if(m_kFollowInfo.m_bFollowFirstActionAdded == false)
		{
			if(kActionEntity.GetAction() == NULL)
			{
				return;
			}

			//	첫번째 액션이 들어왔다.
			//	그 액션의 시작위치로 이동한다.

			m_kFollowInfo.m_bFollowFirstActionAdded = true;
			m_kFollowInfo.m_kFollowState = stFollowInfo::FS_MOVE_TO_STARTPOS;

			NiPoint3	kStartPos = GetPosition();
			PgAction	*pkAction = CreateActionForTransitAction(ACTIONNAME_RUN,true,&kStartPos);
			if(pkAction)
			{
				NiPoint3	kTargetPos = kActionEntity.GetAction()->GetActionStartPos();
				pkAction->SetParamAsPoint(0,kTargetPos);
			}
			m_kActionQueue.push_back(PgActionEntity(pkAction,DIR_NONE));

		}
	}

	m_kActionQueue.push_back(kActionEntity);

	AddActionEntityToFollowers(kActionEntity);
}
void	PgActor::ClearAllActionEffect()
{
	if(m_pkActionEffectStack)
	{
		m_pkActionEffectStack->ClearAll();
	}
}
bool PgActor::DoAction(PgAction* pkNextAction, bool bForceToTransit)
{
	if(IsMyActor())
	{
		//MyActor 일 경우 랜덤 시드를 세팅한다.
		if(GetPilot() && GetPilot()->GetUnit())
		{
			if(pkNextAction)
			{
				pkNextAction->StartRandomSeedCallCounter(GetPilot()->GetUnit()->RandomSeedCallCounter());
			}
		}
	}

	if(false == RACEEVENT::CheckSkillEvent(this, pkNextAction))
	{
		return false;
	}
	

	if(!pkNextAction)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.DoAction"), g_pkApp->GetFrameCount()));

	if(m_pkAction)
	{
		_PgOutputDebugString("[PgActor::DoAction]Actor : %s CurAction : %s NextAction %s Enable : %d\n",MB(GetPilot()->GetName()),m_pkAction->GetID().c_str(),pkNextAction->GetID().c_str(),pkNextAction->GetEnable());
	}

	bool bIsMyActor = IsMyActor();
	bool bAlreadyBroadCast = false;
	std::string actionName;
	pkNextAction->GetActionName(actionName);

	NILOG(PGLOG_LOG, "[PgActor] %s actor NextAction(%s,%d,%d,%d,%d)\n", MB(GetGuid().str()), actionName.c_str(), pkNextAction->GetActionNo(), pkNextAction->GetEnable(),pkNextAction->GetActionInstanceID(),pkNextAction->GetTargetList()->size());
	/*_PgOutputDebugString("[PgActor] %s actor NextAction(%s,%d,%d,%d,%d)\n", MB(GetGuid().str()), actionName.c_str(), pkNextAction->GetActionNo(), pkNextAction->GetEnable(),pkNextAction->GetActionInstanceID(),pkNextAction->GetTargetList()->size());*/
	if(m_pkAction)
	{ 
		m_pkAction->GetActionName(actionName);
		NILOG(PGLOG_LOG, "[PgActor] %s actor CurAction(%s,%d,%d)\n", MB(GetGuid().str()), actionName.c_str(), m_pkAction->GetActionNo(),m_pkAction->GetActionInstanceID());
		//WriteToConsole("[PgActor] %s actor CurAction(%s,%d,%d)\n", MB(GetGuid().str()), actionName.c_str(), m_pkAction->GetActionNo(),m_pkAction->GetActionInstanceID());
		/*_PgOutputDebugString("[PgActor] %s actor CurAction(%s,%d,%d)\n", MB(GetGuid().str()), actionName.c_str(), m_pkAction->GetActionNo(),m_pkAction->GetActionInstanceID());*/
	}

	CSkillDef const* pkNextActionSkillDef = pkNextAction->GetSkillDef();
	PG_ASSERT_LOG(pkNextActionSkillDef);
	if(!pkNextActionSkillDef)
	{
		return	false;
	}

	if(!GetPilot() || !GetPilot()->GetUnit())
	{
		return	false;
	}


	ESkillStatus	kNextActionParam = (ESkillStatus)pkNextAction->GetActionParam();
	const	ESkillType	kNextActionType = (ESkillType)pkNextActionSkillDef->GetType();
	const	EUnitType	kUnitType = GetPilot()->GetUnit()->UnitType();

	if(m_pkAction && kNextActionParam == ESS_TARGETLISTMODIFY)
	{
		OnTargetListModified(pkNextAction);
		return false;
	}

	if(kNextActionParam == ESS_TOGGLE_OFF)
	{		
		return	false;
	}

	if(pkNextAction->GetActionType()=="EFFECT")
	{
		ClearActionQueue();
	}
	else if(!IsUnderMyControl() && (IsSync() || pkNextAction->GetAddToActionEntity()))
	{
		PG_ASSERT_LOG(!pkNextAction->AlreadySync());
		NILOG(PGLOG_LOG, "[PgActor] %s actor Push Action(%s,%d,%d) To ActionStack \n", MB(GetGuid().str()), pkNextAction->GetID().c_str(), pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID());
		//WriteToConsole("[PushToActionQueue(Because Now Sync)] Action ID : %s, Action Term : %u, \n", pkNextAction->GetID().c_str(), pkNextAction->GetActionTerm());

		AddActionEntity(pkNextAction, DIR_NONE);
		return true;
	}

	if( kNextActionParam == ESS_MONITOR)
	{// 다음 액션이 액션 중 일때 다른 클라이언트로 보내야 한다면
		return false;
	}

	if( GetFreezed() )
	{// 얼어 있는 상태인데
		if( false == CanDmgActionOnFreezed() )
		{// 데미지 액션을 할수 없으면 종료 하고
			if(m_pkAction
				&& m_pkAction->GetActionNo() == ACTION_NO_IDLE
				|| pkNextAction->GetActionNo() != ACTION_NO_IDLE )
			{
				return false;
			}
		}
		if( pkNextAction->GetActionNo() != ACTIONEFFECT_DMG
			&& pkNextAction->GetActionNo() != ACTION_NO_IDLE
			&& pkNextAction->GetActionNo() != ACTION_NO_BATTLE_IDLE
			)
		{// 얼어있는 상태에서 데미지 액션을 해야면 데미지 액션을 제외한 나머지 액션은 못하게 하고
			return false;
		}
	}

	if(false==IsActionShift(pkNextAction))
	{
		return false;
	}

	if(pkNextAction->CheckCanEnter(this, pkNextAction) == false && bForceToTransit == false)
	{
		NILOG(PGLOG_LOG,"Check Can Enter Failed. Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()), pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());
		return false;
	}

	//	토글 액션이라면
	if(IsUnderMyControl() && pkNextAction->GetEnable())
	{
		if(kNextActionType == EST_TOGGLE)
		{
			if(!ProcessToggleAction(pkNextAction))
			{
				return	false;
			}
		}
	}

	if(pkNextAction->GetAbil(AT_SKILL_CHECK_NONE_EFFECT))
	{ //대상의 이펙트를 체크하는 스킬이라면
		CSkillDef const* pkSkillDef = pkNextAction->GetSkillDef();
		if(pkSkillDef)
		{
			CUnit* pkTargetUnit = NULL;
			switch(pkSkillDef->GetTargetType())
			{
			case ESTARGET_SELF: { pkTargetUnit = GetUnit(); } break;
			case ESTARGET_CASTER:
				{
					CUnit* pkUnit = GetUnit();
					if(NULL != pkUnit)
					{
						PgPilot* pkPilot = g_kPilotMan.FindPilot(pkUnit->Caller());
						if(pkPilot != NULL)
						{
							pkTargetUnit = pkPilot->GetUnit();
						}
					}
				}break;
			}

			if(NULL != pkTargetUnit)
			{
				int iEffectNo = pkSkillDef->GetEffectNo();
				if(NULL != pkTargetUnit->GetEffect(iEffectNo, true))
				{
					::Notice_Show( TTW(790254), EL_Warning, true );
					return false;
				}
			}
		}
	}

	if(bIsMyActor || IsMyPet() || IsMySubPlayer())
	{
		if(false == CheckRequirementForAction(pkNextAction,true) && false == bForceToTransit)
		{
			SkillSetAction().ReserveActionCancel();
			return	false;
		}
	}

	if( "EFFECT" != pkNextAction->GetActionType() && false == m_pkActionEffectStack->IsEmpty() && false == bForceToTransit )
	{
		if(bIsMyActor)
		{
			return	false;	//	액션이펙트를 수행중이라면, 다른 액션으로 전이시키지 않는다.
		}

		if(kUnitType != UT_PLAYER)
		{
			// TODO : 방향 Broadcast해야 하는지 체크.
			return	m_pkActionEffectStack->SaveLastAction(pkNextAction);	//	플레이어가 아니라면, (몬스터 or NPC) 마지막 액션으로 저장해두고, 이펙트가 모두 종료되었을때, 액션을 자동으로 실행해준다.
		}

		ClearAllActionEffect();
	}

	if(bIsMyActor == false)
	{
		if(!pkNextAction->AlreadySync() && pkNextAction->GetActionTerm() != 0 && pkNextAction->GetActionType() != "EFFECT")
		{
			// 자동전이 되는 액션은 Queue에 넣으면 안된다. (실패 한걸로 간주해서, 계속 발생되어 Queue에 들어감)
			//WriteToConsole("[PushToActionQueue] Action ID : %s, Action Term : %u, \n", pkNextAction->GetID().c_str(), pkNextAction->GetActionTerm());
			AddActionEntity(pkNextAction, DIR_NONE);
			return true;
		}
	}

	//	몬스터의 IDLE 액션이라면, 현재 액션이 끝난 다음에 실행하도록 한다(부드러운 연결을 위해..)
	//if(m_pkAction && kUnitType != UT_PLAYER)
	//{
	//	if(pkNextAction->GetActionStartPos() != NiPoint3::ZERO)
	//	{
	//		if(m_pkAction->GetActionType() != "MOVE" && pkNextAction->GetActionType() == "IDLE")
	//		{
	//			NILOG(PGLOG_LOG,"Set To Next Action because this is mon's idle. Actor : %s Action : %s,%d,%d,%d,%d\n",MB(GetGuid().str()),
	//				pkNextAction->GetID().c_str(),pkNextAction->GetActionNo(),pkNextAction->GetActionInstanceID(),
	//				pkNextAction->GetTargetList()->size());

	//			m_pkAction->SetNextActionName(pkNextAction->GetID().c_str());
	//			return false;
	//		}
	//	}
	//}

	if(IsUnderMyControl() == false && m_pkAction)
	{
		if(m_pkAction->GetSkillType() == EST_ACTIVE || m_pkAction->GetSkillType() == EST_TOGGLE)
		{
			if(pkNextAction->GetID() == m_pkAction->GetID())
			{
				if((kNextActionParam == ESS_FIRE || kNextActionParam == ESS_TOGGLE_ON) && kNextActionParam != m_pkAction->GetActionParam())
				{
					CutSkillCasting(m_SkillCastingInfo.m_ulSkillNo);
					OnCastingCompleted(pkNextAction);

					return true;
				}
			}
		}
	}

	// 현재 Action에서 다음 액션으로 가도록 FSM이 허락한다면
	bool bSuccessfulLeave = true;
	if(m_pkAction)
	{
		bSuccessfulLeave = ProcessLeaveCurrentAction(pkNextAction) || (bForceToTransit == true);
		if(!bSuccessfulLeave)
		{
			return	false;
		}
	}

	if(IsUnderMyControl())
	{
		if(bSuccessfulLeave && pkNextAction->GetEnable())
		{			
			//	이미 캐스팅 중인 스킬이 있다면, 캐스팅을 끊는다.
			CutSkillCasting(m_SkillCastingInfo.m_ulSkillNo);
		}

		SetActionParam(pkNextAction);
		kNextActionParam = (ESkillStatus)pkNextAction->GetActionParam();		
	}

	bool bSuccessfulEnter = false;
	if(bSuccessfulLeave)
	{
		bSuccessfulEnter = pkNextAction->EnterFSM(this, pkNextAction) || (bForceToTransit == true);
	}

	NILOG(PGLOG_LOG,"NextActioin Enter : %d bRet : %d Actor : %s Action : %s,%d,%d,%d,%d\n", bSuccessfulEnter, bSuccessfulLeave, MB(GetGuid().str()), pkNextAction->GetID().c_str(), pkNextAction->GetActionNo(), pkNextAction->GetActionInstanceID(), pkNextAction->GetTargetList()->size());

	if(bSuccessfulLeave && bSuccessfulEnter)
	{

		m_byWeaponAnimFolderNumAtActionStart = m_byMyWeaponAnimFolderNum;

		// 이 액터가 내 클라이언트의 액터일 경우 브로드 캐스팅한다.
		SetIgonreDamageEffect(pkNextAction);

		if(IsUnderMyControl())
		{
			if(pkNextAction->GetActionType() != "EFFECT")
			{
				if(kNextActionParam == ESS_FIRE && pkNextAction->GetActionOptionEnable(PgAction::AO_NO_BROADCAST) == false)
				{
					StartSkillCoolTime(pkNextAction->GetActionNo());	//	쿨타임 시작
				}

				StartSkillCasting(pkNextAction->GetActionNo());
			}
		}

		PlayAnimation(pkNextAction);

		if(m_pkAction)
		{			
			// 트랜짓을 성공하면, 예전 액션을 삭제 큐에 추가한다.
			g_kActionPool.ReleaseAction(m_pkAction);
		}

		// 액션을 갱신.
		m_pkAction = pkNextAction;
		

		// Action을 브로드캐스트한다.
		if(IsUnderMyControl())
		{			
			if(m_pkAction->GetActionType() != "IDLE"
				&& m_pkAction->GetActionType() != ACTIONTYPE_JOBSKILL
				)
			{
				//_PgOutputDebugString("[Call ConcilDirection 0]Actor(%s) m_kMovingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);
				ConcilDirection(m_kMovingDir, false);
			}

			if( (m_pkAction->GetActionOptionEnable(PgAction::AO_NO_BROADCAST) == false && m_pkAction->GetActionType() != "EFFECT") 
				|| "a_Resurrection_01" == m_pkAction->GetID() || "a_revive" == m_pkAction->GetID()
				|| lwCommonSkillUtilFunc::IsReActionByTrapSkill(lwAction(m_pkAction))
				) //부활, 트랩 스킬 피격 리액션은 예외 처리 한다.
			{
				//g_kPilotMan.Broadcast(m_pkPilot, m_pkAction, (m_pkAction->CanChangeActorPos() == false));
				if(IsMyActor() || !IsNowFollowing())
				{
					g_kPilotMan.Broadcast(m_pkPilot, m_pkAction, false);
				}
				
				//	토클 액션일 경우 토클 상태를 변환시킨다.
				CSkillDef const* pkSkillDef = m_pkAction->GetSkillDef();
				if(pkSkillDef)
				{
					// 캐스팅이 없이 즉시 시전일 경우에만 토글 상태로 전환 시킨다.
					if (E_SCAST_INSTANT == pkSkillDef->GetAbil(AT_CASTTYPE))
					{
						if(EST_TOGGLE == pkSkillDef->GetType())
						{
							ActionToggleStateChange(m_pkAction->GetActionNo(),true);
						}
					}
				}

#ifndef EXTERNAL_RELEASE
				// Writing Log
				std::string kNextActionName;
				std::string kCurrentActionName;
				
				if (m_pkAction)
				{
					m_pkAction->GetActionName(kCurrentActionName);
				}

				m_pkAction->GetActionName(kNextActionName);
				NILOG(PGLOG_NETWORK, "[PgActor] BroadCast %s Action(%d,%d,%d,%d,%d,%d)(%d) - current %s\n", kNextActionName.c_str(), m_pkAction->GetActionNo(), m_pkAction->GetActionInstanceID(), m_pkAction->GetActionParam(), m_pkAction->GetTargetList()->size(), m_pkAction->GetEnable(), (m_pkAction ? m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) == false : 0), g_pkApp->GetFrameCount(), kCurrentActionName.c_str());
#endif
			}
		}
		else
		{
			SetSeeFrontAttribute();
		}


		//	애니메이션 스피드 원래대로
		SetAnimSpeedInPeriod(1, 0);

		//
		if(m_pkAction->GetActionType()=="IDLE" || m_pkAction->GetActionType()=="MOVE")
		{
			SkillSetAction().NextReservedAction(this);
		}
	}
	else
	{

		return false;
	}

	return true;
}

void PgActor::CancelAction(int iActionID, int iActionInstanceID, char const *pcNextActionName, bool bToggleCancel)
{
	_PgOutputDebugString("[PgActor::CancelAction] Actor:%s(%s) ActionID:%d ActionInstanceID:%d NextActionName:%s\n", MB(GetPilot()->GetName()),MB(GetPilotGuid().str()),iActionID,iActionInstanceID,pcNextActionName);

	if(m_pkAction)
	{
		PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CancelAction"), g_pkApp->GetFrameCount()));
		if(m_pkAction->GetActionNo() == iActionID && m_pkAction->GetActionInstanceID() == iActionInstanceID)
		{
			//	현재 액션을 강제로 지워버린다.
			m_pkAction->LeaveFSM(this, m_pkAction, true);
			m_pkAction->CleanUpFSM(this, m_pkAction);

			g_kActionPool.ReleaseAction(m_pkAction);
			m_pkAction = NULL;

			//	새로운 액션으로 강제 트랜짓!!!
			bool bResult = TransitAction(pcNextActionName, true,0,DIR_NONE,true);
			PG_ASSERT_LOG(m_pkAction != NULL);

		}

		if(bToggleCancel)
		{
			// 토글인 경우 현재의 액션과 같지 않으므로 이렇게 처리해주어야 한다.
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkill = kSkillDefMgr.GetDef(iActionID);
			if(pkSkill)
			{
				if(EST_TOGGLE == pkSkill->GetType())
				{
					if(GetActionToggleState(iActionID))
					{
						ActionToggleStateChange(iActionID, false);
					}
				}
			}
		}
	}
}


void PgActor::AddChangeAction(std::string const& rkFromAction, std::string const& rkToAction)
{
	std::pair<CONT_CHANGE_ACTION::iterator, bool> kRet = m_kContChangeAction.insert(std::make_pair(rkFromAction, rkToAction));
	if(false==kRet.second)
	{
		kRet.first->second = rkToAction;
	}
}

void PgActor::DelChangeAction(std::string const& rkActionName)
{
	m_kContChangeAction.erase(rkActionName);
}

char const* PgActor::GetChangeAction(char const* pcActionName)const
{
	if(pcActionName)
	{
		CONT_CHANGE_ACTION::const_iterator it = m_kContChangeAction.find(pcActionName);
		if(it!=m_kContChangeAction.end())
		{
			return (*it).second.c_str();
		}
	}
	return pcActionName;
}

PgAction* PgActor::CreateActionForTransitAction(char const* pcNextActionName, bool bEnable, NiPoint3* pkActionStartPos, BYTE byDirection, int iActionNo)
{
	char const* pkChangeAction = GetChangeAction(pcNextActionName);
	if(!pkChangeAction)
	{
		return NULL;
	}

	char	strNextActionName[100] = {0, };
	strncpy_s(strNextActionName, 100, pkChangeAction, 99);

	//_PgOutputDebugString("CreateActionForTransitAction pcNextActionName: %s \n",pcNextActionName);

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.TransitAction"), g_pkApp->GetFrameCount()));
#ifndef EXTERNAL_RELEASE
	if (m_dwLastTransitTime == 0)
	{
		m_dwLastTransitTime = BM::GetTime32();
		m_dwLastTransitFrame = g_pkApp->GetFrameCount();
	}
	else if (BM::GetTime32() - m_dwLastTransitTime < 60 || m_dwLastTransitFrame - g_pkApp->GetFrameCount() < 2)
	{
		NILOG(PGLOG_WARNING, "[PgActor] %s Too Fast Transition(%s -> %s) %d, %d\n", MB(GetGuid().str()), GetAction() ? GetAction()->GetID().c_str() : "", pcNextActionName ? pcNextActionName : "", BM::GetTime32() - m_dwLastTransitTime, g_pkApp->GetFrameCount() - m_dwLastTransitFrame);
	}

	m_dwLastTransitTime = BM::GetTime32();
	m_dwLastTransitFrame = g_pkApp->GetFrameCount();
#endif

	PgAction *pkAction = g_kActionPool.CreateAction(strNextActionName);
	if(!pkAction)
	{
//		PgError1("[PgActor::CreateActionForTransitAction] : Failed to creating Action - %s", pcNextActionName);
		return NULL;
	}

	pkAction->SetEnable(bEnable);
	pkAction->SetActionParam(ESS_FIRE);	//	디폴트 값
	pkAction->SetDirection(byDirection);
	
	bool bIsPet = false;
	bool bIsMySubPlayer = false;
	bool bIsSubPlayer = false;
	bool bIsEntity = false;
	PgPilot* pkPilot = GetPilot();
	if(pkPilot)
	{
		CUnit* pkUnit = pkPilot->GetUnit();
		if(pkUnit)
		{
			switch(pkUnit->UnitType())
			{
			case UT_PET :
				{
					bIsPet = true;
				}break;
			case UT_SUB_PLAYER:
				{
					bIsSubPlayer = true;
				}break;
			case UT_ENTITY:
				{
					bIsEntity = true;
				}break;
			}
			
			CSkill* pkSkill = pkUnit->GetSkill();
			if(pkSkill)
			{
				pkAction->SetSkillCoolTime(pkSkill->GetSkillCoolTime());
				pkAction->SetSkillCoolTimeRate(pkSkill->GetSkillCoolTimeRate());
				pkAction->SetSkillCastingTime(pkSkill->GetSkillCastingTime());
			}
		}
	}

	if(IsUnderMyControl())
	{
		//	내 액터가 한 액션일 경우, 액션 인스턴스 ID 를 할당한다.
		pkAction->SetActionInstanceID();
		if(bIsSubPlayer)
		{
			bIsMySubPlayer = true;
		}
	}

	if(IsMyActor()
		|| bIsMySubPlayer)
	{
		if(pkAction->GetSkillType() == EST_ACTIVE || pkAction->GetSkillType() == EST_TOGGLE)
		{
			int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(pkAction);
			PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode(iKeySkillNo);
			if(pkNode)
			{
				int iActionNo = pkNode->GetOriginalSkillNo();

				// 아이템에 의해서 스킬이 오버레벨 되는 경우
				if(GetPilot() && GetPilot()->GetUnit())
				{
					PgPlayer* pkPlayer = static_cast<PgPlayer*>(GetPilot()->GetUnit());
					int const iLearnedSkill = pkPlayer->GetMySkill()->GetLearnedSkill(iActionNo, true);
					if(iLearnedSkill > iActionNo)
					{
						iActionNo = iLearnedSkill;
					}
				}

				pkAction->SetActionNo(iActionNo);
			}
			else // Skill Tree에서 찾지 못할 경우 Cast 와 Fire로 나누어진 스킬일 경우가 있다.
			{
				if(iActionNo > 0 && iActionNo != pkAction->GetActionNo())
				{
					pkAction->SetActionNo(iActionNo);
				}

				// Ex) a_Three Way_Cast / a_Three Way_Fire / a_Rapidly Shot_Cast / a_Rapidly Shot_Fire
				// _Fire 류 스킬에 연결된 _Cast 스킬의 ID를 얻어서 _Fire의 실제 레벨에 해당하는 ID를 세팅한다.
				int const iCastSkillNo = pkAction->GetAbil(AT_PARENT_CAST_SKILL_NO);
				PgSkillTree::stTreeNode *pkNode2 = g_kSkillTree.GetNode(iCastSkillNo);
				if(pkNode2)
				{
					CSkillDef const* pkDef = pkNode2->GetSkillDef();
					if(pkDef)
					{
						int iActionNo = pkDef->No();
						int iLevel = 0;

						// 아이템에 의해서 스킬이 오버레벨 되는 경우
						if(GetPilot() && GetPilot()->GetUnit())
						{
							PgPlayer* pkPlayer = static_cast<PgPlayer*>(GetPilot()->GetUnit());
							iActionNo = pkPlayer->GetMySkill()->GetLearnedSkill(iActionNo, true);
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pkDef2 = kSkillDefMgr.GetDef(iActionNo);
							if(pkDef2)
							{
								iLevel = pkDef2->GetAbil(AT_LEVEL);
							}
							else
							{
								iLevel = pkDef->GetAbil(AT_LEVEL);
							}
						}
						else
						{
							iLevel = 1;
						}

						pkAction->SetActionNo(pkAction->GetActionNo() + iLevel - 1);
					}
				}
			}
		}
	}
	else if(IsUnderMyControl())
	{
		// Ex) a_Three Way_Cast / a_Three Way_Fire / a_Rapidly Shot_Cast / a_Rapidly Shot_Fire
		// 설치류 스킬은 이쪽을 타게 된다.
		int const iCastSkillNo = pkAction->GetAbil(AT_PARENT_CAST_SKILL_NO);
		PgSkillTree::stTreeNode *pkNode2 = g_kSkillTree.GetNode(iCastSkillNo);
		if(pkNode2)
		{
			const CSkillDef* pkDef = pkNode2->GetSkillDef();
			if(pkDef)
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer )
				{
					return NULL;
				}
				PgMySkill* pkSkill = pkPlayer->GetMySkill();
				if( !pkSkill )
				{
					return NULL;
				}
				int const iExtendLevel = pkSkill->GetOverSkillLevel(iCastSkillNo);
				int const iLevel = pkDef->GetAbil(AT_LEVEL);
				pkAction->SetActionNo(pkAction->GetActionNo() + iLevel + iExtendLevel - 1);
			}
		}
		else if( ( bIsEntity || bIsPet)	// 펙 스킬이거나, 소환체 사용 하는 스킬이 배우지 않아도 되는 스킬일 경우
			&& iActionNo
			)
		{
			pkAction->SetActionNo(iActionNo);
		}
	}
	else
	{
		if(iActionNo)
		{
			pkAction->SetActionNo(iActionNo);
		}
	}

	PgActionTargetList	kTargetList;
	kTargetList.SetActionInfo(GetPilotGuid(),pkAction->GetActionInstanceID(),pkAction->GetActionNo(),pkAction->GetTimeStamp());
	pkAction->SetTargetList(kTargetList);

	if(pkActionStartPos)
	{
		pkAction->SetActionStartPos(*pkActionStartPos);
	}

	return	pkAction;
}

bool PgActor::TransitAction(char const *pcNextActionName, bool bEnable, NiPoint3 *pkActionStartPos, BYTE byDirection,bool bForceToTransit)
{
	NILOG(PGLOG_LOG, "[PgActor] %s actor TransitAction(%s,%d)\n", MB(GetGuid().str()), pcNextActionName, bEnable);
	//_PgOutputDebugString("TransitAction pcNextActionName:%s\n",pcNextActionName);

	PgAction	*pkAction = CreateActionForTransitAction(pcNextActionName,bEnable,pkActionStartPos,byDirection);
	if(!pkAction)
	{
		return	false;
	}

	return ProcessAction(pkAction,IsMyActor(),bForceToTransit);
}
PgAction*	PgActor::ReserveTransitAction(PgAction *pkNextAction)
{
	if(!pkNextAction)
	{
		return	NULL;
	}

	m_kReservedTransitAction.push_back(pkNextAction);

	return	pkNextAction;
}
PgAction*	PgActor::ReserveTransitAction(int iActionNo, BYTE byDirection)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ReserveTransitAction"), g_pkApp->GetFrameCount()));
	//_PgOutputDebugString("[PgActor.ReserveTransitAction] Reserved Action : %s\n", kNextAction);

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	std::wstring wActionID = kSkillDefMgr.GetActionName(iActionNo);
	if(wActionID.length() == 0) return NULL;

	std::string kActionID(MB(wActionID));

	PgAction	*pkAction = CreateActionForTransitAction(kActionID.c_str(),true,NULL,byDirection, iActionNo);
	if(pkAction)
	{
		ReserveTransitAction(pkAction);
	}


	return	pkAction;
}
PgAction* PgActor::ReserveTransitAction(char const *kNextAction, BYTE byDirection)
{
#ifndef EXTERNAL_RELEASE
	if(g_pkApp->IsSingleMode())
	{
		PgAction	*pkAction = CreateActionForTransitAction(kNextAction,true,NULL,byDirection);
		ReserveTransitAction(pkAction);

		return	pkAction;
	}
#endif

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	int	iActionID = kSkillDefMgr.GetSkillNoFromActionName(UNI(kNextAction));
	if(iActionID == 0)
	{
		PgAction	*pkAction = CreateActionForTransitAction(kNextAction,true,NULL,byDirection);
		ReserveTransitAction(pkAction);

		return	pkAction;
	}

	return	ReserveTransitAction(iActionID,byDirection);
}

void PgActor::ClearReservedAction()
{
	for(ActionList::iterator itor = m_kReservedTransitAction.begin(); itor != m_kReservedTransitAction.end(); ++itor)
	{
		PgAction *pkAction = *itor;
		g_kActionPool.ReleaseAction(pkAction);
	}
	
	m_kReservedTransitAction.clear();
}

PgAction *PgActor::GetReservedTransitAction()
{
	if(m_kReservedTransitAction.size() == 0)
	{
		return	NULL;
	}
	PgAction	*pkLastReservedAction = m_kReservedTransitAction.back();
	return pkLastReservedAction;
}

bool PgActor::DoReservedTransitAction()
{
	if(m_kReservedTransitAction.size() == 0)
	{
		return	true;
	}

	ActionList kTempList;
	kTempList.swap( m_kReservedTransitAction );
//	ActionList	kTempList(m_kReservedTransitAction);
//	m_kReservedTransitAction.clear();

	for(ActionList::iterator itor = kTempList.begin(); itor != kTempList.end(); ++itor)
	{
		PgAction *pkAction = *itor;
		ProcessAction(pkAction,IsMyActor());
	}

	return	true;
}

BM::GUID PgActor::CreateTempAction(int const iActionNo)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	std::wstring const ActionID( kSkillDefMgr.GetActionName(iActionNo) );
	if(true == ActionID.empty()) return BM::GUID::NullData();

	if(PgAction * pkAction = g_kActionPool.CreateAction(MB(ActionID)))
	{
		BM::GUID kGuid;
		kGuid.Generate();

		std::pair<CONT_TEMP_ACTION::iterator, bool> kRet = m_kContTempAction.insert(std::make_pair(kGuid, pkAction));
		if(kRet.second)
		{
			return kGuid;
		}
		else
		{
			g_kActionPool.ReleaseAction(pkAction);
		}
	}
	return BM::GUID::NullData();
}

PgAction* PgActor::GetTempAction(BM::GUID const& kActionGuid)
{
	CONT_TEMP_ACTION::iterator it = m_kContTempAction.find(kActionGuid);
	if(it != m_kContTempAction.end())
	{
		return (*it).second;
	}
	return NULL;
}

void PgActor::RemoveTempAction(BM::GUID const& kActionGuid)
{
	CONT_TEMP_ACTION::iterator it = m_kContTempAction.find(kActionGuid);
	if(it != m_kContTempAction.end())
	{
		g_kActionPool.ReleaseAction((*it).second);
		m_kContTempAction.erase(it);
	}
}

void PgActor::ClearTempAction()
{
	for(CONT_TEMP_ACTION::iterator it = m_kContTempAction.begin(); it != m_kContTempAction.end(); ++it)
	{
		g_kActionPool.ReleaseAction((*it).second);
	}
	m_kContTempAction.clear();
}

bool PgActor::PlayNext()
{
	// short circuit evaluation
	if(!m_pkAction || !m_pkAction->NextSlot())
	{
		return false;
	}

	return PlayCurrentSlot();
}

bool PgActor::PlayPrev()
{
	// short circuit evaluation
	if(!m_pkAction || !m_pkAction->PrevSlot())
	{
		return false;
	}

	return PlayCurrentSlot();
}

bool PgActor::PlayCurrentSlot(bool bNoRandom)
{
	std::string kSlotName;
	
	if(!m_pkAction || !m_pkAction->GetActionName(kSlotName))
	{
		return false;
	}
	
	if( !SetTargetAnimation(kSlotName,true,bNoRandom) )
	{
		return false;
	}

	//	액션이 공속 어빌의 영향을 받는 액션이라면, 공속을 적용하여 애니메이션 스피드를 변경한다.
	if(m_pkAction->GetAbil(AT_APPLY_ATTACK_SPEED) == 1)
	{
		float	fAttackSpeed = GetPilot()->GetAbil(AT_C_ATTACK_SPEED)/ABILITY_RATE_VALUE_FLOAT;
		SetAnimSpeed(GetAnimSpeed()*fAttackSpeed);
	}
	return true;
}

void PgActor::SetNormalAttackActionID(std::string const kActionID)
{
	if(GetPilot())
	{
		if(kActionID.empty()) // remove key
		{
			GetPilot()->RemoveActionKey(ACTIONKEY_ATTACK);
		}
		else	//	reset key action id
		{
			GetPilot()->SetKeyActionID(ACTIONKEY_ATTACK, kActionID, 0);
		}
	}
}
char const* PgActor::GetNormalAttackActionID()
{
	if(GetPilot())
	{	
		char const *ActionID = GetPilot()->FindActionID(ACTIONKEY_ATTACK);
		if(ActionID) return ActionID;
	}

	return "";
}

bool	PgActor::AddDropItem(PgDropBox *pkItemBox)	//	추가할 수 있으면 true 아니면 false
{
	if(!pkItemBox || !pkItemBox->GetPilot() || !pkItemBox->GetPilot()->GetUnit())
	{
		return false;
	}

	//	현재 HP가 0 이면 추가할 수 없다.(단, 정예는 예외)
	if(GetPilot())
	{
		if(GetPilot()->GetAbil(AT_GRADE) != EMGRADE_ELITE && GetPilot()->GetAbil(AT_HP) == 0)
		{
			return false;
		}
	}

	//	이미 있는 GUID 인지 찾아보자.
	stActorDropItemInfo	*pkDropItemInfo = NULL;
	for(ActorDropItemInfoList::iterator itor = m_ActorDropItemInfoList.begin(); itor != m_ActorDropItemInfoList.end(); ++itor)
	{
		pkDropItemInfo = &(*itor);

		if(pkDropItemInfo->m_kItemGUID == pkItemBox->GetGuid()) { return false; }
	}
	PgGroundItemBox const *pkGroundItem = dynamic_cast<PgGroundItemBox*>(pkItemBox->GetPilot()->GetUnit());
	if(pkGroundItem)
	{
		pkItemBox->SetHide(true);	//변환 가능할 때만 숨기자 에러일때 숨겨버리면 안된다.

		//	새로 추가한다.
		stActorDropItemInfo	kNewInfo;
		kNewInfo.m_kItemGUID = pkItemBox->GetGuid();
		kNewInfo.m_iActionInstanceID = pkGroundItem->ActionInstanceID();

		m_ActorDropItemInfoList.push_back(kNewInfo);
		return true;
	}

	return	false;
}

void	PgActor::DoDropItems(int iActionInstanceID,int iReqCount,float fJumpHeight)	//	iActionInstanceID 에 해당하는 아이템들을 떨군다. -1 일 경우 모든 아이템을 떨군다. 
{
	if(!g_pkWorld)
	{
		return;
	}

	PgDropBox	*pkDropBox = NULL;
	stActorDropItemInfo	*pkDropItemInfo = NULL;
	int	iCount = 0;
	for(ActorDropItemInfoList::iterator itor = m_ActorDropItemInfoList.begin(); itor != m_ActorDropItemInfoList.end();)
	{
		pkDropItemInfo = &(*itor);

		if(-1 == iActionInstanceID || iActionInstanceID >= pkDropItemInfo->m_iActionInstanceID)
		{
			pkDropBox = (PgDropBox*)g_pkWorld->FindObject(pkDropItemInfo->m_kItemGUID);
			if(!pkDropBox)
			{
				++itor;
				continue;
			}
			
			//pkDropBox->StartJump(5.0f);
			pkDropBox->SetOfferer(GetGuid(), GetWorldTranslate(),fJumpHeight);
			pkDropBox->SetHide(false);

			itor = m_ActorDropItemInfoList.erase(itor);

			++iCount;
			if(iReqCount >0 && iCount == iReqCount)
			{
				return;
			}
			continue;
		}

		++itor;
	}
}

void PgActor::CopyEquipItem(PgActor* pkSourceActor)
{
	if( !pkSourceActor )
	{
		return;
	}

	PgPilot* pkPilot = pkSourceActor->GetPilot();
	if( !pkPilot )
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if( !pkPlayer )
	{
		return;
	}

	PLAYER_ABIL kInfo;
	pkPlayer->GetPlayerAbil(kInfo);

	GET_DEF(CItemDefMgr, kItemDefMgr);

	CUnit* pkUnit = pkPilot->GetUnit();
	PgInventory *pkInven = pkPlayer->GetInven();
	if( !pkInven )
	{
		return;
	}

	PgOptionUtil::SClientDWORDOption const kOption(pkPilot->GetAbil(AT_CLIENT_OPTION_SAVE));
	for( int iCur = 0; EQUIP_POS_MAX > iCur ; ++iCur )
	{
		PgBase_Item kItem;

		SItemPos const kCashItemPos(IT_FIT_CASH, iCur);
		if( S_OK == pkInven->GetItem(kCashItemPos, kItem)
		&&	false == kItem.IsUseTimeOut()
		&&	!kOption.IsHideCashInvenPos(static_cast< EEquipPos >(iCur)) )
		{//! 캐쉬 아이템 보임
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( pkItemDef )
			{
				eEquipLimit const eLimit = static_cast< eEquipLimit >(pkItemDef->GetAbil(AT_EQUIP_LIMIT));
				AddToDefaultItem(eLimit, kItem.ItemNo(), &kItem.EnchantInfo());
			}
		}
		else
		{
			SItemPos const kItemPos(IT_FIT, iCur);
			if( S_OK == pkInven->GetItem(kItemPos, kItem) 
				&& !kOption.IsHideEquipInvenPos(static_cast< EEquipPos >(iCur)) )
			{//! 장비 아이템 보임
				CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if( pkItemDef )
				{
					eEquipLimit const eLimit = static_cast< eEquipLimit >(pkItemDef->GetAbil(AT_EQUIP_LIMIT));
					AddToDefaultItem(eLimit, kItem.ItemNo(), &kItem.EnchantInfo());
				}
			}
			else
			{
				switch( iCur )
				{
				case EQUIP_POS_SHIRTS:		{ AddToDefaultItem(EQUIP_LIMIT_SHIRTS, kInfo.iJacket); }break;
				case EQUIP_POS_PANTS:		{ AddToDefaultItem(EQUIP_LIMIT_PANTS, kInfo.iPants); }break;
				case EQUIP_POS_BOOTS:		{ AddToDefaultItem(EQUIP_LIMIT_BOOTS, kInfo.iShoes); }break;
				case EQUIP_POS_GLOVE:		{ AddToDefaultItem(EQUIP_LIMIT_GLOVE, kInfo.iGloves); }break;
				case EQUIP_POS_FACE:		{ AddToDefaultItem(EQUIP_LIMIT_FACE, kInfo.iFace); }break;
				case EQUIP_POS_HAIR:		{ AddToDefaultItem(EQUIP_LIMIT_HAIR, kInfo.iHairStyle); }break;
				case EQUIP_POS_HAIR_COLOR:	{ AddToDefaultItem(EQUIP_LIMIT_HAIR_COLOR, kInfo.iHairColor); }break;
				default:
					{
						eEquipLimit const eLimit = static_cast< eEquipLimit >(0x00000001 << iCur);
						if( DelDefaultItem(eLimit) )
						{
							EInvType const eType = static_cast< EInvType >(kItemPos.x);
							EEquipPos const ePos = static_cast< EEquipPos >(iCur);
							UnequipItem(eType, ePos, 0, PgItemEx::LOAD_TYPE_INSTANT);
						}
					}break;
				}
			}
		}
	}// end for

	EquipAllItem();
}

void PgActor::EquipAllItem_SubPlayer(PgActor* pkCallerActor)
{			
	if( !pkCallerActor )
	{
		return;
	}
	PgPlayer *pkCallerPlayer = dynamic_cast<PgPlayer*>(pkCallerActor->GetUnit());
	if( !pkCallerPlayer )
	{
		return;
	}

	PLAYER_ABIL kInfo;
	pkCallerPlayer->GetPlayerAbil(kInfo);

	AddToDefaultItem(EQUIP_LIMIT_HAIR_COLOR, kInfo.iHairColor);
	AddToDefaultItem(EQUIP_LIMIT_FACE, kInfo.iFace);
	AddToDefaultItem(EQUIP_LIMIT_HAIR, kInfo.iHairStyle);
	AddToDefaultItem(EQUIP_LIMIT_SHIRTS, kInfo.iJacket);
	AddToDefaultItem(EQUIP_LIMIT_PANTS, kInfo.iPants);
	AddToDefaultItem(EQUIP_LIMIT_BOOTS, kInfo.iShoes);
	AddToDefaultItem(EQUIP_LIMIT_GLOVE, kInfo.iGloves);

	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_FACE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HAIR_COLOR);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HAIR);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHOULDER);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_CLOAK);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_GLASS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_WEAPON);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHEILD);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_NECKLACE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_EARRING);
	//EquipItemByPo, IT_FIT_CASH,EQUIP_POS_EARRING_R);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_RING);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_RING_R);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_BELT);
	//EquipItemByPo, IT_FIT_CASH,EQUIP_POS_ATTSTONE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_MEDAL);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_HELMET);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_SHIRTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_PANTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_BOOTS);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_GLOVE);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_KICKBALL);
	EquipItemByPos(IT_FIT_CASH,EQUIP_POS_ARM);

	//	일반 아이템 장착
	EquipItemByPos(IT_FIT,EQUIP_POS_FACE);
	EquipItemByPos(IT_FIT,EQUIP_POS_HAIR_COLOR);
	EquipItemByPos(IT_FIT,EQUIP_POS_HAIR);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHOULDER);
	EquipItemByPos(IT_FIT,EQUIP_POS_CLOAK);
	EquipItemByPos(IT_FIT,EQUIP_POS_GLASS);
	EquipItemByPos(IT_FIT,EQUIP_POS_WEAPON);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHEILD);
	EquipItemByPos(IT_FIT,EQUIP_POS_NECKLACE);
	EquipItemByPos(IT_FIT,EQUIP_POS_EARRING);
	//EquipItemByPo, IT_FIT,EQUIP_POS_EARRING_R);
	EquipItemByPos(IT_FIT,EQUIP_POS_RING);
	EquipItemByPos(IT_FIT,EQUIP_POS_RING_R);
	EquipItemByPos(IT_FIT,EQUIP_POS_BELT);
	//EquipItemByPo, IT_FIT,EQUIP_POS_ATTSTONE);
	EquipItemByPos(IT_FIT,EQUIP_POS_MEDAL);
	EquipItemByPos(IT_FIT,EQUIP_POS_HELMET);
	EquipItemByPos(IT_FIT,EQUIP_POS_SHIRTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_PANTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_BOOTS);
	EquipItemByPos(IT_FIT,EQUIP_POS_GLOVE);
	EquipItemByPos(IT_FIT,EQUIP_POS_KICKBALL);
	EquipItemByPos(IT_FIT,EQUIP_POS_ARM);
	
	int const iBaseClass = pkCallerPlayer->GetAbil(AT_BASE_CLASS);
	if( iBaseClass == UCLASS_DOUBLE_FIGHTER )
	{//격투가 보조 캐릭터 셋팅된 무기 없으면 기본 지급 무기 강제 착용
		AddEquipItem(330100005, false, PgItemEx::LOAD_TYPE_INSTANT, false );
	}

}
bool PgActor::CopyEquipItemFromMainPlayer(PgActor* pkActor, EInvType kInvType, EEquipPos kItemPos)
{
	if( !pkActor )
	{
		return false;
	}
	if (m_fLoadingStartTime == 0.0f)
	{
		m_fLoadingStartTime = NiGetCurrentTimeInSec();
	}

	if ( EQUIP_POS_HAIR_COLOR == kItemPos )	// 머리색은 PgItemEx를 만들 필요가 없다. Hair아이템을 만들때 참조만 됨.
	{
		return true;
	}
	
	PgPilot* pkPilot = pkActor->GetPilot();
	if( pkPilot )
	{
		PgControlUnit* pkPlayer = dynamic_cast<PgControlUnit*>(pkPilot->GetUnit());
		if (pkPlayer)
		{
			int iItemNo = GetAdjustedItemNo( pkPlayer, kItemPos );
			bool bUseDefault = false;

			if ( 0 == iItemNo )
			{
				PgBase_Item kItem;
				PgOptionUtil::SClientDWORDOption const kOption(pkPilot->GetAbil(AT_CLIENT_OPTION_SAVE));
				if ( IT_FIT_CASH == kInvType )
				{
					if ( !kOption.IsHideCashInvenPos( kItemPos ) )
					{// 감추기 설정 되어있으면 감춘다
						if ( S_OK == pkPlayer->GetInven()->GetItem( kInvType, kItemPos, kItem ) )
						{
							iItemNo = kItem.ItemNo();
						}
					}
					else if ( !kOption.IsHideEquipInvenPos( kItemPos ) )
					{// 캐시 아이템이 감추어져 있다면
						if ( S_OK == pkPlayer->GetInven()->GetItem( IT_FIT, kItemPos, kItem ) )
						{// 원래 장착 되어있는 아이템을 찾아서 붙여줄수 있게 준비해준다.
							iItemNo = kItem.ItemNo();
						}
					}
				}
				else
				{
					if (	S_OK == pkPlayer->GetInven()->GetItem( IT_FIT_CASH, kItemPos, kItem ) 
						&&	!kItem.IsUseTimeOut() 
						&&	!kOption.IsHideCashInvenPos(kItemPos) )
					{
						// 동일한 위치에 캐쉬 아이템이 장착되어있으면 장착하지 않아야 한다.
					}
					else
					{
						if ( !kOption.IsHideEquipInvenPos(kItemPos) )
						{
							if ( S_OK == pkPlayer->GetInven()->GetItem( kInvType, kItemPos, kItem ) )
							{
								iItemNo = kItem.ItemNo();
							}
						}

						if ( 0 == iItemNo )
						{
							DefaultItemContainer::iterator itr = std::find(m_kDefaultItem.begin(), m_kDefaultItem.end(), ItemDesc(static_cast<eEquipLimit>(1 << kItemPos)));
							if( itr != m_kDefaultItem.end())
							{
								// 장착부위에 디폴트 아이템은 있다.
								bUseDefault = true;
								iItemNo = itr->m_iItemNo;
							}
						}
					}
				}

				{//하지만 세트아이템은 장착되었을수 있으므로, 해당 세트 아이템이 장착되었을대의 효과를 등록해주어야 한다
					int iCheckItemNo = iItemNo;
					if(0 == iItemNo
						&& S_OK == pkPlayer->GetInven()->GetItem( kInvType, kItemPos, kItem)
						)
					{
						iCheckItemNo = kItem.ItemNo();
					}

					//Attach Item Set Effect
					GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
					int const iSetNo = kItemSetDefMgr.GetItemSetNo(iCheckItemNo);
					CItemSetDef const *pSetDef = kItemSetDefMgr.GetDef(iSetNo);
					if(pSetDef)
					{
						CONT_HAVE_ITEM_DATA kContHaveItems;
						CONT_HAVE_ITEM_DATA kContHaveCashItems;

						if(!pkPilot){return true;}
						if(!pkPilot->GetUnit()){return true;}
						if(!pkPilot->GetUnit()->GetInven()){return true;}

						pkPilot->GetUnit()->GetInven()->GetItems(IT_FIT, kContHaveItems);
						{// 캐시
							pkPilot->GetUnit()->GetInven()->GetItems(IT_FIT_CASH, kContHaveCashItems);
							bool bCompleteSet = false;
							int const iPieceSet = pSetDef->CheckNeedItem(kContHaveCashItems,pkPilot->GetUnit(), bCompleteSet);
							if( bCompleteSet )
							{// 세트아이템에 의한
								SPOTParticleInfo kTemp;
								if(g_kItemMan.GetSetItemParticleInfo(iSetNo, kTemp))
								{// 특정 시점 파티클이 존재한다면, 해당 관리 객체에 넣어 주고
									m_kPOTParticle.AddInfo(kTemp);
								}
								AddCompletedItemSet(iSetNo);
							}
						}
						//일반 세트 아이템을 모두 장착 한 상태
						bool bCompleteSet = false;
						int const iPieceSet = pSetDef->CheckNeedItem(kContHaveItems,GetPilot()->GetUnit(), bCompleteSet);
						if( bCompleteSet )
						{
							{// 아이템에, 특정시점에만 붙여줄 파티클 정보가 있는지 확인하고
								SPOTParticleInfo kTemp;
								if(g_kItemMan.GetSetItemParticleInfo(iSetNo, kTemp))
								{// 존재한다면, 해당 관리 객체에 넣어 주고
									m_kPOTParticle.AddInfo(kTemp);
								}
							}
							AddCompletedItemSet(iSetNo);
						}
					}

					{// 아이템에, 특정시점에만 붙여줄 파티클 정보가 있는지 확인하고
						SPOTParticleInfo kTemp;
						if(g_kItemMan.GetItemPOTParticleInfo(iCheckItemNo, kTemp))
						{// 존재한다면, 해당 관리 객체에 넣어 주고
							m_kPOTParticle.AddInfo(kTemp);
						}
					}
				}
			}
				
			if ( 0 < iItemNo )
			{// GetAdjustedItemNo()에서 리턴값이 -1인경우도 장착을 하면 안되기 때문에 0< 
				NILOG(PGLOG_LOG, "[PgActor] EquipItemByPos(%d, %d, %d)\n", kItemPos, iItemNo, bUseDefault);
				return AddEquipItem( iItemNo, bUseDefault, PgItemEx::LOAD_TYPE_USEQUEUE , false);
			}
			return true;
		}
	}
	return false;
}

void	PgActor::StartNormalAttackFreeze()
{
	m_ulNormalAttackFreezeStartTime = BM::GetTime32();
}
void	PgActor::StopNormalAttackFreeze()
{
	m_ulNormalAttackFreezeStartTime = 0;
}

int	PgActor::GetNormalAttackFreezeElapsedTime()
{
	if(m_ulNormalAttackFreezeStartTime == 0) return -1;

	return	BM::GetTime32()-m_ulNormalAttackFreezeStartTime;
}

//! Normal 공격의 종료 시간을 저장한다
void	PgActor::SetNormalAttackEndTime()
{
	m_ulNormalAttackEndTime = BM::GetTime32();
}
unsigned long PgActor::GetNormalAttackEndTime() const
{
	return m_ulNormalAttackEndTime;
}
bool PgActor::CanNowConnectToNextComboAttack(float const fMaxTime) const
{
	unsigned long const ulComboConnectionDelay = static_cast<unsigned long>(fMaxTime*1000);	//	콤보 입력 가능 구간 시간

	unsigned long const ulCurrentTime = BM::GetTime32();
	if(ulCurrentTime - GetNormalAttackEndTime() > ulComboConnectionDelay) { return false; }

	return	true;
}

bool PgActor::IsAnimationDone()
{
	PG_ASSERT_LOG(GetActorManager());
	if(!GetActorManager())	return	true;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.IsAnimationDone"), g_pkApp->GetFrameCount()));

	NiControllerSequence *pkSequence = GetActorManager()->GetSequence(m_kSeqID);

	if(!pkSequence || pkSequence->GetCycleType() == NiTimeController::LOOP)
	{
		return false;
	}

	if(GetActorManager()->GetCurAnimation() != m_kSeqID)
	{
		return false;
	}

	float fTime = GetActorManager()->GetNextEventTime(NiActorManager::END_OF_SEQUENCE, m_kSeqID);

	if(fTime == NiActorManager::INVALID_TIME)
	{
		return true;
	}

	return false;
}

bool PgActor::IsAnimationLoop() const
{
	PG_ASSERT_LOG(GetActorManager());
	if(!GetActorManager())	return	false;

	NiControllerSequence *pkSequence = GetActorManager()->GetSequence(m_kSeqID);
	if(!pkSequence)
	{
		return false;
	}
	return  pkSequence->GetCycleType() == NiTimeController::LOOP;
}

bool PgActor::HaveAnimationTextKey(char const* szKey, char const* szAnimationName) const
{
	if(!GetActorManager() || !GetActionSlot() || !szKey)
	{
		return false;
	}

	NiActorManager::SequenceID kSeqID;
	if(szAnimationName)
	{	
		PgActionSlot* pkActionSlot = GetActionSlot();
		if(!pkActionSlot->GetAnimation(szAnimationName, kSeqID))
		{
			return false;
		}
	}
	else
	{
		kSeqID = m_kSeqID;
	}

	NiControllerSequence *pkSequence = GetActorManager()->GetSequence(kSeqID);	
	if(!pkSequence)
	{
		return false;
	}

	NiTextKeyExtraData *pkTextKeys = pkSequence->GetTextKeys();
	if(!pkTextKeys)
	{
		return false;
	}

	unsigned int uiTextKeyCount = 0;
	NiTextKey *pkTextKey = pkTextKeys->GetKeys(uiTextKeyCount);
	if(!pkTextKey)
	{
		return false;
	}

	for(unsigned int uiTextKeyIdx = 0; uiTextKeyIdx < uiTextKeyCount; ++uiTextKeyIdx)
	{
		NiFixedString kTextKeyName = pkTextKey[uiTextKeyIdx].GetText();
		if( 0 == strcmp(szKey, kTextKeyName) )
		{
			return true;
		}
	}

	return false;
}

void PgActor::DrawNoZTest(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if (!m_bVisible || !m_bIsVisibleInFrustum) // m_bHide <- QUESTION: 체크해야 하나 말아야 하나. Swift샷 같은건 m_bHide를 켜놓는다.
		return;

	if(IsHide() || (IsEnemy(g_kPilotMan.GetPlayerActor()) && IsInvisible())) return;

	if(GetPilot() && GetPilot()->IsHide())
		return;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.DrawNoZTest"), g_pkApp->GetFrameCount()));

	DrawParticle(pkRenderer,false);	//	Z Test 하지 않는 파티클들을 렌더링한다.
}
void PgActor::ResetABVToNewAM()
{
	if(!m_pkPhysXScene)
	{
		return;
	}
	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (m_pkPhysXScene == NULL || pkPhysXManager == NULL)
		return;

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		GetWorld()->LockPhysX(true);
		PG_STAT(timerA.Stop());
	}
	for(int i = 0; GetABVShape(i)->IsValid() && i < PG_MAX_NB_ABV_SHAPES; ++i)
	{
		NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
		if(pkTarget)
		{
			m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);

			m_apkPhysXCollisionSrcs[i] = NiNew NiPhysXKinematicSrc(pkTarget, m_apkPhysXCollisionActors[i]);
			m_apkPhysXCollisionSrcs[i]->SetActive(true);
			m_pkPhysXScene->AddSource(m_apkPhysXCollisionSrcs[i]);
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}
}
void PgActor::InitPhysX(NiPhysXScene* pkPhysXScene, int uiGroup)
{
	PG_ASSERT_LOG(pkPhysXScene);

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (NULL == pkPhysXScene || NULL == pkPhysXManager)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.InitPhysX"), g_pkApp->GetFrameCount()));

	if (GetWorld() && 1 == g_iUseAddUnitThread)
	{
		PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		GetWorld()->LockPhysX(true);
		PG_STAT(timerA.Stop());
	}

	NxScene* pkNxScene = pkPhysXScene->GetPhysXScene();
#ifdef PG_USE_CAPSULE_CONTROLLER

	NxCapsuleControllerDesc kCtrlDesc;
	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT;
	float fRadius = PG_CHARACTER_CAPSULE_RADIUS;
	NiPoint3 kLoc = GetTranslate();
	kCtrlDesc.position.x = kLoc.x;
	kCtrlDesc.position.y = kLoc.y;
	kCtrlDesc.position.z = kLoc.z;
	kCtrlDesc.radius = fRadius;
	kCtrlDesc.height = fHeight;
	kCtrlDesc.upDirection = NX_Z;
	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(PG_LIMITED_ANGLE + 10.0f));
	kCtrlDesc.skinWidth = 0.1f;
	kCtrlDesc.stepOffset = PG_CHARACTER_CAPSULE_RADIUS * 2.0f;
	kCtrlDesc.callback = &m_kControllerHitCallBack;
#else
	NxBoxControllerDesc kCtrlDesc;
	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT + 10.0f;
	float fRadius = PG_CHARACTER_CAPSULE_RADIUS - 5.0f;
	NiPoint3 kLoc = GetTranslate();
	kCtrlDesc.position.x = kLoc.x;
	kCtrlDesc.position.y = kLoc.y;
	kCtrlDesc.position.z = kLoc.z;
	kCtrlDesc.extents = NxVec3(7.5f, 25.0f , 7.5f);
	kCtrlDesc.upDirection = NX_Z;
	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(30.0f));
	kCtrlDesc.skinWidth = 0.1f;
	kCtrlDesc.stepOffset = fRadius;
#endif

	m_kLastFloorPos = kLoc; //! 초기 위치를 여기에서 넣어준다.

#ifdef PG_USE_CAPSULE_CONTROLLER
	m_pkController = (NxCapsuleController*)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
#else
	m_pkController = (NxBoxController *)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
#endif
	m_pkController->setInteraction(NXIF_INTERACTION_EXCLUDE);
	m_pkController->setPosition(NxExtendedVec3(kLoc.x, kLoc.y, kLoc.z));

	NxMat33 kMat;
	NiPhysXTypes::NiQuaternionToNxMat33(NiQuaternion(NI_HALF_PI, NiPoint3::UNIT_X), kMat);

	m_pkPhysXActor = m_pkController->getActor();
	m_pkPhysXActor->setCMassOffsetLocalOrientation(kMat);
	m_pkPhysXActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
	
	SetRotation(NiQuaternion::IDENTITY);

	NxShape* pkShape = m_pkPhysXActor->getShapes()[0];
	pkShape->setLocalOrientation(kMat);
	pkShape->setLocalPosition(NX_ZERO);
	pkShape->setFlag(NX_SF_DISABLE_COLLISION, true); 
	pkShape->setFlag(NX_SF_DISABLE_RAYCASTING, true);

	GetNIFRoot()->SetTranslate(NiPoint3(0.0f, 0.0f, -PG_CHARACTER_Z_ADJUST));	
			
	// Gamebryo --> PhysX 동기자를 생성한다.
	m_pkPhysXSrc = NiNew NiPhysXKinematicSrc(this, m_pkPhysXActor);
	m_pkPhysXSrc->SetActive(false);
	m_pkPhysXSrc->SetInterpolate(false);
	pkPhysXScene->AddSource(m_pkPhysXSrc);

	// PhysX --> Gamebryo 동기자를 생성한다.
	m_pkPhysXDest = NiNew NiPhysXTransformDest(this, m_pkPhysXActor, 0);
	m_pkPhysXDest->SetActive(true);
	m_pkPhysXDest->SetInterpolate(false);
	pkPhysXScene->AddDestination(m_pkPhysXDest);

/*	//DB에 있는 AT_UNIT_SCALE을 적용 시켜야할 경우 주석을 해제 한다.
	float fUnitScale = 0.0f;
	if(GetPilot() && GetPilot()->GetUnit())
	{
		fUnitScale = static_cast<float>(GetPilot()->GetUnit()->GetAbil(AT_UNIT_SCALE)) / ABILITY_RATE_VALUE_FLOAT;
	}*/

	for(int i = 0; GetABVShape(i)->IsValid() && i < PG_MAX_NB_ABV_SHAPES; ++i)
	{
		NxShapeDesc* kShapeDesc = GetABVShape(i)->GetPhysXShapeDesc();
	/*	//DB에 있는 AT_UNIT_SCALE을 적용 시켜야할 경우 주석을 해제 한다.
		if(0.0f < fUnitScale)
		{
			NxCapsuleShapeDesc* pkCasult = dynamic_cast<NxCapsuleShapeDesc*>(kShapeDesc);
			if(pkCasult)
			{
				pkCasult->height *= fUnitScale;
				pkCasult->radius *= fUnitScale;
			}

			NxBoxShapeDesc* pkBox = dynamic_cast<NxBoxShapeDesc*>(kShapeDesc);
			if(pkBox)
			{
				pkBox->dimensions.x *= fUnitScale;
				pkBox->dimensions.y *= fUnitScale;
				pkBox->dimensions.z *= fUnitScale;
			}
			NxSphereShapeDesc* pkSphere = dynamic_cast<NxSphereShapeDesc*>(kShapeDesc);
			if(pkSphere)
			{
				pkSphere->radius *= fUnitScale;
			}			
		}*/

		kShapeDesc->group = uiGroup + 1;
		kShapeDesc->userData = this;
		kShapeDesc->localPose = GetABVShape(i)->m_kMat;
		kShapeDesc->name = GetABVShape(i)->m_kTo;

		NxBodyDesc kBodyDesc;
		NxActorDesc kActorDesc;
		kActorDesc.shapes.push_back(kShapeDesc);
		kActorDesc.body = &kBodyDesc;
		kActorDesc.density = 0.1f;
		
		NiAVObject* pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
		if(pkTarget)
		{
			m_apkPhysXCollisionActors[i] = pkPhysXScene->GetPhysXScene()->createActor(kActorDesc);
			m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_KINEMATIC);
			//m_apkPhysXCollisionSrcs[i] = NiNew NiPhysXDynamicSrc(pkTarget, m_apkPhysXCollisionActors[i], 0, 0);
			m_apkPhysXCollisionSrcs[i] = NiNew NiPhysXKinematicSrc(pkTarget, m_apkPhysXCollisionActors[i]);
			m_apkPhysXCollisionSrcs[i]->SetActive(true);
			pkPhysXScene->AddSource(m_apkPhysXCollisionSrcs[i]);
			
			m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_FROZEN_POS);
			m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_FROZEN_ROT);
			m_apkPhysXCollisionActors[i]->setGroup(uiGroup + 1);
			m_apkPhysXCollisionActors[i]->userData = this;

			//if(i < PG_MAX_NB_ABV_SHAPES - 1)
			//{
			//	m_apkPhysXCollisionActors[i+1] = 0;
			//	m_apkPhysXCollisionSrcs[i+1] = 0;
			//}
		}
		else
		{
			PgError2("Actor [%s] ABV TargetNode [%s] Not Found.", GetID().c_str(), GetABVShape(i)->m_kTo);
		}
	}
	
	if (GetWorld() && 1 == g_iUseAddUnitThread)
	{
		GetWorld()->LockPhysX(false);
	}

	m_pkPhysXScene = pkPhysXScene;
}

void PgActor::InitRidingInfo(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (pkPhysXScene == NULL || pkPhysXManager == NULL)
	{
		return;
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		_PgOutputDebugString("PgActor::InitRidingInfoName[%s] WaitSDKLock\n",GetID().c_str());
		GetWorld()->LockPhysX(true);
	}

	NiActorManager	*pkAM = GetActorManager();
	if(!pkAM)
	{
		_PgOutputDebugString("PgActor::InitRidingInfo Name[%s] No AM\n",GetID().c_str());
		return;
	}

	NiKFMTool	*pkKFMTool = pkAM->GetKFMTool();
	if(!pkKFMTool)
	{
		_PgOutputDebugString("PgActor::InitRidingInfo Name[%s] No KFM Tool\n",GetID().c_str());
		return;
	}

	char	const	*pkNIFPath = pkKFMTool->GetFullModelPath();
	if(!pkNIFPath)
	{
		_PgOutputDebugString("PgActor::InitRidingInfo Name[%s] No NIFPath\n",GetID().c_str());
		return;
	}

	NiStream kStream;
	if(kStream.Load(pkNIFPath))
	{
		int iCount = kStream.GetObjectCount();
		for(int i=1;i<iCount;++i)
		{

			NiObject *pkObject = kStream.GetObjectAt(i);
			if(NiIsKindOf(NiPhysXScene,pkObject))
			{
				NiPhysXScenePtr spPhysXSceneObj = NiDynamicCast(NiPhysXScene,pkObject);
				m_kPhysXSceneObjCont.push_back(spPhysXSceneObj);

				PgPhysXUtil::MakeStaticMeshNameUnique(spPhysXSceneObj,(char const*)pkNIFPath,GetID(),GetScale());

				NxMat34 kSlaveMat;
				NiMatrix3 kPhysXRotMat;
				kPhysXRotMat.MakeIdentity();
				NiPoint3 kPhysXTranslation = GetTranslate();
				NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);

				spPhysXSceneObj->SetSlaved(pkPhysXScene, kSlaveMat);
				spPhysXSceneObj->CreateSceneFromSnapshot(0);

				unsigned	int	iSourceNum = spPhysXSceneObj->GetSourcesCount();
				for (unsigned int iSrcCount=0 ; iSrcCount< iSourceNum ; iSrcCount++)
				{
					NiPhysXSrc *pkPhysXSrc = spPhysXSceneObj->GetSourceAt(iSrcCount);

					NiPhysXKinematicSrc *pkPhysXKinematicSrcOrg = NiDynamicCast(NiPhysXKinematicSrc,pkPhysXSrc);
					if(!pkPhysXKinematicSrcOrg)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No pkPhysXKinematicSrcOrg\n",GetID().c_str());
						continue;
					}
					NiAVObject	*pkGBSource = GetObjectByName(pkPhysXKinematicSrcOrg->GetSource()->GetName());
					if(!pkGBSource)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No pkGBSource\n",GetID().c_str());
						continue;
					}

					NxActor	*pkTarget = pkPhysXKinematicSrcOrg->GetTarget();
					if(!pkTarget)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No pkTarget\n",GetID().c_str());
						continue;
					}

					NxShape *const *pkShapes = pkTarget->getShapes();
					int	iNumShapes = pkTarget->getNbShapes();

					for(int k=0;k<iNumShapes;k++)
					{

						NxShape	*pkShape = *pkShapes;

						pkShape->setGroup(uiGroup+1);
						pkShape->userData = this;

						pkShapes++;
					}

					pkTarget->setGroup(uiGroup+1);
					pkTarget->userData = this;

					NiPhysXKinematicSrc *pkPhysXKinematicSrc = NiNew NiPhysXKinematicSrc(pkGBSource, pkTarget);
					pkPhysXKinematicSrc->SetActive(true);
					pkPhysXKinematicSrc->SetInterpolate(false);
					pkPhysXScene->AddSource(pkPhysXKinematicSrc);

					m_vKinematicSrcCont.push_back(pkPhysXKinematicSrc);
				}
			}
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}
}

void PgActor::InitPhysical(bool bIsPhysical)
{
	// Ageia PhysX
	if (bIsPhysical)
	{
		m_pkPhysXSrc->SetActive(false);
		m_pkPhysXDest->SetActive(true);
	}
	// Gamebryo
	else
	{
		m_pkPhysXSrc->SetActive(true);
		m_pkPhysXDest->SetActive(false);
	}
}

void PgActor::ReleaseABVShapes()
{
	if(m_pkPhysXScene && m_pkPhysXActor)
	{
		PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ReleasePhysX"), g_pkApp->GetFrameCount()));
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
		if (GetWorld())
		{
			PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
			PG_STAT(timerA.Start());
			GetWorld()->LockPhysX(true);
			PG_STAT(timerA.Stop());
		}

		for(int i = 0; i < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[i]; ++i)
		{
			if(GetABVShape(i))
			{
				GetABVShape(i)->m_eType = PgIWorldObjectBase::ABVShape::ST_NONE;
			}

			m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);

			if (m_pkPhysXScene->GetPhysXScene())
			{
				m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_apkPhysXCollisionActors[i]);
			}

			m_apkPhysXCollisionActors[i] = 0;
		}
		
		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}
}
void PgActor::ReleasePhysX()
{
	if(m_pkPhysXScene && m_pkPhysXActor)
	{
		PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ReleasePhysX"), g_pkApp->GetFrameCount()));
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
		if (GetWorld())
		{
			PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
			PG_STAT(timerA.Start());
			GetWorld()->LockPhysX(true);
			PG_STAT(timerA.Stop());
		}

		m_pkPhysXScene->DeleteDestination(m_pkPhysXDest);
		m_pkPhysXScene->DeleteSource(m_pkPhysXSrc);
		//if (m_pkPhysXScene->GetPhysXScene())
		//	m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_pkPhysXActor);	// dukguru assert : double deletion
		
		for(int i = 0; i < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[i]; ++i)
		{
			m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);
			if (m_pkPhysXScene->GetPhysXScene())
			{
				m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_apkPhysXCollisionActors[i]);
			}
			m_apkPhysXCollisionActors[i] = 0;
		}
		
		if(m_pkController)
		{
			g_kControllerManager.releaseController(*m_pkController);
			m_pkController = 0;
			m_pkPhysXActor = NULL;
		}

		if(m_pkPhysXActor && m_pkPhysXScene->GetPhysXScene())
		{
			m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_pkPhysXActor);
		}

		//////////////////////////////////////////////////////////////////////
		//RidingInfo삭제
		for(NiPhysXSceneCont::iterator itor = m_kPhysXSceneObjCont.begin(); itor != m_kPhysXSceneObjCont.end(); ++itor)
		{
			NiPhysXScenePtr	spScene = *itor;
	
			m_pkPhysXScene->RemoveSlave(spScene);

		}
		m_kPhysXSceneObjCont.clear();

		int	iTotalSrc = m_vKinematicSrcCont.size();
		for(int i=0;i<iTotalSrc;++i)
		{
			m_pkPhysXScene->DeleteSource(m_vKinematicSrcCont[i]);
		}
		//
		//////////////////////////////////////////////////////////////////////

		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}

	m_kPhysXSceneObjCont.clear();
	m_vKinematicSrcCont.clear();

	m_pkPhysXScene = 0;
	m_pkPhysXActor = 0;
	m_pkController = 0;
}

void PgActor::ReleaseAllParticles()
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ReleaseAllParticles"), g_pkApp->GetFrameCount()));
	PgIWorldObject::ReleaseAllParticles();
}

bool PgActor::BeforeUse()
{
	NILOG(PGLOG_LOG, "[PgActor] %s(%#X) actor before use\n", MB(GetGuid().str()), this);
	PgIWorldObject::BeforeUse();

	m_pTextBalloon = NULL;
	m_pMarkBalloon = NULL;
	m_pPartyBalloon = NULL;
	m_pExpeditionBalloon = NULL;
	m_pVendorBalloon = NULL;
	
	PgPilot* pkPilot = GetPilot();

	if( pkPilot )
	{
		CUnit* pkUnit = pkPilot->GetUnit();
		if ( pkUnit )
		{
			EUnitType const eUnitType = pkUnit->UnitType();
			switch( eUnitType )
			{
			case UT_NPC:
				{
				}break;
			case UT_SUMMONED:
				{
					if( g_kPilotMan.IsMySummoned(pkUnit) )
					{
						lwSummon_Info::lwShowSummonList();
					}
					NiAVObjectPtr	spTargetPoint = GetNodePointStar();
					if( spTargetPoint ) // p_ef_star 노드가 없으면 아예 HP바를 생성하지 않는다.
					{
						m_pHPGaugeBar = g_kEnergyGaugeMan.CreateNewGauge( pkPilot );
					}
					else
					{
						m_pHPGaugeBar = NULL;
					}
				}break;
			case UT_PLAYER:
				{
					//SyncMountPet();
					g_kMarkBalloonMan.AddActor( this );
				} // break을 사용하지 않는다.
			default:
				{
					if(UT_ENTITY==eUnitType && ENTITY_GUARDIAN==pkUnit->GetAbil(AT_ENTITY_TYPE))
					{
						PgEntity* pkEntity = dynamic_cast<PgEntity*>(pkUnit);
						if( pkEntity && !pkEntity->IsEternalLife() )
						{
							float const fAliveTotalTime = pkEntity->LifeTime()/1000.f;
							SetAliveTimeGauge(fAliveTotalTime);
							SetAutoDeleteActorTimer(fAliveTotalTime);
						}
					}
					else
					{
						//NiAVObjectPtr	spTargetPoint = GetActorManager()->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
						NiAVObjectPtr	spTargetPoint = GetNodePointStar();
						if( spTargetPoint ) // p_ef_star 노드가 없으면 아예 HP바를 생성하지 않는다.
						{
							m_pHPGaugeBar = g_kEnergyGaugeMan.CreateNewGauge( pkPilot );
						}
						else
						{
							m_pHPGaugeBar = NULL;
						}
					}
				}break;
			}
		}
	}

	m_pkHeadBuffIconList = g_kHeadBuffIconListMgr.CreateNewIconList();
/*
	if(GetPilot()->GetUnit()->UnitType() == UT_PLAYER)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(GetPilot()->GetUnit());
		if(pkPlayer)
		{
			PgActor* pkActorPet = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
			if(pkActorPet && pkActorPet->GetPilot()->GetUnit()->GetAbil(815))
			{
				MountPet();
			}
		}

	}
*/
	return true;
}

bool PgActor::BeforeCleanUp()
{
	NILOG(PGLOG_LOG, "[PgActor] %s(%#X) actor before cleanup\n", MB(GetGuid().str()), this);
	//펫에 탑승 중이면 내린뒤 소멸해야 한다. (char_root 이하 노드가 펫에 붙어있으므로 원래 위치로 복구)
	if(IsRidingPet()) //주인,펫 중 소멸 순서를 알수 없으므로 먼저 소멸되는 놈이 Unmount 해준다.
	{
		UnmountPet();
	}

	//	상태이상 모두 클리어
	g_kStatusEffectMan.RemoveAllStatusEffect(GetPilot(),true);

	//	떨궈야 할 아이템이 남아있다면 다 떨군다.
	DoDropItems();

	DetachNameNodes(GetNIFRoot()); // 이름들을 때준다.
	m_spNameText = 0;
	m_spTitleName = 0;
	m_spGuildNameText = 0;
	m_spGuildMark = 0;
	m_spGuardianMark = 0;
	m_spMyhomeMark = 0;
	m_spCustomCountText = 0;
	m_spAchievementTitle = 0;
	m_spGIFTitle = 0;
	m_spDuelTitle = 0;
	m_spEffectCountDown = 0;

	EndWeaponTrail();
	EndBodyTrail();

	StartDamageBlink(false);

	g_kHeadBuffIconListMgr.ReleaseIconList(m_pkHeadBuffIconList);
	m_pkHeadBuffIconList = NULL;
	
	g_kEnergyGaugeMan.DestroyGauge(m_pHPGaugeBar);
	m_pHPGaugeBar = NULL;

	g_kBalloonMan2D.DestroyNode(m_pTextBalloon);
	g_kMarkBalloonMan.DestroyNode(m_pMarkBalloon);
	g_kPartyBalloonMgr.DestroyNode(m_pPartyBalloon);
	g_kExpeditionBalloonMgr.DestroyNode(m_pExpeditionBalloon);
	g_kVendorBalloonMgr.DestroyNode(m_pVendorBalloon);

	if(m_pkSyncMoveNextAction)
	{
		g_kActionPool.ReleaseAction(m_pkSyncMoveNextAction);
	}

	m_pkSyncMoveNextAction = NULL;

	if(m_pkAction)
	{
		m_pkAction->CleanUpFSM(this,m_pkAction);
		g_kActionPool.ReleaseAction(m_pkAction);
		m_pkAction = NULL;
	}

	ClearReservedAction();
	ClearActionQueue();
	SAFE_DELETE(m_pkActionEffectStack);

	if (g_pkWorld && g_pkWorld->GetCameraMan())
	{
		g_pkWorld->GetCameraMan()->NotifyActorRemove(this);
	}

	RestoreSpecular();	//	스페큘러 복귀

	PgCircleShadow::DetachCircleShadowRecursive(this);

	if(true==IsMyActor() && g_pkLocalManager)
	{
		g_pkLocalManager->ClearInputDirReverse();	//내 캐릭터면 인풋 설정 반되로 된거 무조건 초기화
	}

	return PgIWorldObject::BeforeCleanUp();	
}

PgActor* PgActor::GetPetActor(void)
{
	PgActor* pkActorPet = NULL;
	if(GetPilot()->GetUnit()->UnitType() == UT_PLAYER)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(GetPilot()->GetUnit());
		if(pkPlayer)
		{
			pkActorPet = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
		}
	}
	return pkActorPet;
}

//#define PG_USE_DETAIL_FIND_PATH_NORMAL
bool PgActor::FindPathNormal(bool const bDoNotConcil)
{
	if(m_bNoFindPathNormal)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.FindPathNormal"), g_pkApp->GetFrameCount()));

	static NiPoint3 akDirs[] =
	{
		NiPoint3(1.0f, 0.0f, 0.0f),
		NiPoint3(-1.0f, 0.0f, 0.0f),
		NiPoint3(0.0f, 1.0f, 0.0f),
		NiPoint3(0.0f, -1.0f, 0.0f),
	};

	NiQuaternion kQuat(
		NiACos(GetPathNormal().Dot(-NiPoint3::UNIT_Y)),
		GetPathNormal().UnitCross(-NiPoint3::UNIT_Y));
	NiMatrix3 kAxisRot;
	kQuat.ToRotation(kAxisRot);

	m_pkPick->SetTarget(m_pkPathRoot);
	m_pkPick->ClearResultsArray();

	NxVec3 ktLoc(GetPosition().x,GetPosition().y,GetPosition().z);//m_pkPhysXActor->getGlobalPosition();

	NiPoint3 kPickStart = NiPoint3(ktLoc.x, ktLoc.y, ktLoc.z) + NiPoint3(0, 0, 30.0f);

#ifdef PG_USE_DETAIL_FIND_PATH_NORMAL
	NiPoint3 kNor = NiPoint3(0, 0, 1);
	for(int i=0 ; i<16 ; ++i)
	{
		NiPoint3 kDir = NiPoint3(1, 0, 0);
		float fAngle = (360 / 16) * i;
		float fRadian = (fAngle * 3.1415f / 180.0f);
		NiQuaternion kQuat = NiQuaternion(fRadian, kNor);
		kQuat.FastNormalize();
		NiMatrix3 kMat;
		kQuat.ToRotation(kMat);
		kDir = kMat * kDir;
		kDir.Unitize();
		m_pkPick->PickObjects(kPickStart, kDir, true);
	}
#else
	
	for(int i = 0; i < 4; ++i)
	{
		NiPoint3 kDir = akDirs[i] * kAxisRot;
		kDir.Unitize();
		m_pkPick->PickObjects(kPickStart, kDir, true);
	}
#endif

	m_pkPick->RemoveTarget();

	NiPick::Results& rkResults = m_pkPick->GetResults();

	if(0 == rkResults.GetSize())
	{
		return false;
	}

	NiPick::Record *pkRecord = rkResults.GetAt(0);
	//if(pkRecord->GetDistance() > PG_PATHWALL_DIST_LIMIT)
	//{
	//	return false;
	//}

	// 패스의 노멀을 갱신한다.
	NiPoint3 kIntersection = (NiPoint3)pkRecord->GetIntersection();
	NiPoint3 kPathNormal = pkRecord->GetNormal();
	kPathNormal.z = 0;
	kPathNormal.Unitize();
	SetPathNormal(kPathNormal);
	SetPathImpactPoint(kIntersection);
		
	// 위치를 보정한다
	NiPoint3 kDir = kPickStart - kIntersection;
	kDir.z = 0.0f;

	NiPoint3 kNewLoc = kPickStart;
	kNewLoc.z = ktLoc.z;
	NxExtendedVec3 kExOldLoc = m_pkController->getFilteredPosition();
	NiPoint3 kOldLoc((float)kExOldLoc.x, (float)kExOldLoc.y, (float)kExOldLoc.z);
	NiPoint3 kDisp = kNewLoc - kOldLoc;
	NxVec3 kConcil;
	NiPhysXTypes::NiPoint3ToNxVec3(kDisp, kConcil);

	if(!bDoNotConcil)
	{
		// 두 방향으로만 봐야 할 때는, 보는 방향을 재구성.
		NiPoint3 kLookingDir = m_kMovingDir;
		if(m_bLockBidirection && NiPoint3::ZERO != m_kMovingDir)
		{
			bool bLeft = ((m_kPathNormal.UnitCross(kLookingDir).z>0) ? true : false);
			kLookingDir = m_kPathNormal.UnitCross(NiPoint3::UNIT_Z * (bLeft ? -1.0f : 1.0f));
			//kLookingDir.z = 0;
		}

		// 방향을 보정한다.
		ConcilDirection(kLookingDir);	
	}

	return true;
}

void PgActor::ConcilDirection(NiPoint3& rkLookingDir, bool const bTurnRightAway)
{
	// TODO : NoConcil 정리

	// LookingDir 이 Zero이면 맴버변수에 값을 넣어줌으로써 후에 NPC가 패스벽 찾을 때 필요하다.
	if(m_bNoConcil || NiPoint3::ZERO == rkLookingDir)
	{
		// NoConcil이거나, LookingDir이 Zero이면 아무 것도 하지 않는다.
		return;
	}

	if (GetPilot() && 101 == GetPilot()->GetAbil(AT_NOT_SEE_TARGET))
	{
		return;
	}

	if(m_pkAction && m_pkAction->GetActionOptionEnable(PgAction::AO_BIDIRECTION))
	{
		// 좌우만 보는 액션은 좌우만 보게 함
		bool bLeft = (m_kPathNormal.UnitCross(rkLookingDir).z > 0 ? true : false);
		rkLookingDir = m_kPathNormal.UnitCross(NiPoint3::UNIT_Z * (bLeft ? -1.0f : 1.0f));
		rkLookingDir.z = 0;
	}

	m_kLookingDir = rkLookingDir;
	rkLookingDir.z=0;
	rkLookingDir.Unitize();	//	leesg213 추가함

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ConcilDirection"), g_pkApp->GetFrameCount()));
	
	// 뒤로 밀리는 경우는 이동 방향을 거꾸로
	NiPoint3 kLookingDir = rkLookingDir * (m_bBackMoving ? -1.0f : 1.0f);
	NiPoint3 kCross = kLookingDir.UnitCross(NiPoint3::UNIT_Y);
	if(kCross.SqrLength() < 0.0001f)
	{
		// LookingDir이 Y축과 딱 맞으면, UnitCross는 (0,0,0)을 리턴한다.
		// 이 경우 MovingDir의 y 값을 가지고 방향을 정해준다.
		kCross = (kLookingDir.y > 0 ? NiPoint3::UNIT_Z : -NiPoint3::UNIT_Z);
	}
	
	//_PgOutputDebugString("ConcilDirection Actor(%s) rkLookingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),kLookingDir.x,kLookingDir.y,kLookingDir.z);

	// 방향에 맞도록 회전을 갱신
	NiQuaternion kRot(NiACos(kLookingDir.Dot(-NiPoint3::UNIT_Y)), kCross);
	if(m_kToRotation != kRot)
	{
		m_kToRotation = kRot;
		if(bTurnRightAway)
		{
			SetRotation(kRot);
			m_fRotationInterpolTime = 1.0f;
		}
		else
		{
			m_fRotationInterpolTime = 0.0f;
		}
	}	

	BYTE byLastDirection = GetDirFromMovingVector(kLookingDir);
	if(m_bLockBidirection && (byLastDirection & DIR_HORIZONTAL) != byLastDirection)
	{
		return;
	}

	m_kLookingDir = kLookingDir;
	m_byLastDirection = byLastDirection;
}

void PgActor::SetLookingTarget(NiPoint3 const &rkTarget, bool bRightAway)
{
	SetLookingDirection(GetDirFromMovingVector(rkTarget - GetPosition()), bRightAway);
}
void	PgActor::ReserveDieByActioin(bool bDie)
{
	m_bDieReservedByAction = bDie;
}
bool	PgActor::IsReserveDieByAction()
{
	return	m_bDieReservedByAction;
}

void PgActor::SetLookingDirection(BYTE byDirection, bool bRightAway)
{
	// TODO : 8방향 제한이 걸려 있을 때는 4방향만 보게 하자 
	// (걸을 때는 2방향이지만, 쳐다보는 방향은 4방향이 된다)
	if(m_bLockBidirection)
	{
		if(((byDirection & DIR_HORIZONTAL) != 0) && ((byDirection & DIR_VERTICAL) != 0))
		{
			if((byDirection & DIR_HORIZONTAL) == DIR_HORIZONTAL ||
				(byDirection & DIR_VERTICAL) == DIR_VERTICAL)
			{
				// 충돌되는 두 방향을 모두 바라보게 하려면 아무곳도 쳐다보지 않는다.
				return;
			}				
		}
	}

	bool bLeft = (byDirection & DIR_LEFT);
	bool bRight = (byDirection & DIR_RIGHT);
	bool bFront = (byDirection & DIR_DOWN);
	bool bBack = (byDirection & DIR_UP);

	NiPoint3 kLookingDir(NiPoint3::ZERO);
	if(bLeft || bRight)
	{
		kLookingDir = m_kPathNormal.Cross((bLeft ? -NiPoint3::UNIT_Z : NiPoint3::UNIT_Z));
	}

	if(!m_bLockBidirection && (bFront || bBack))
	{
		kLookingDir += (bFront ? -m_kPathNormal : m_kPathNormal);
	}
	
	kLookingDir.Unitize();

	//_PgOutputDebugString("[Call ConcilDirection 2]Actor(%s) kLookingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),kLookingDir.x,kLookingDir.y,kLookingDir.z);
	ConcilDirection(kLookingDir, bRightAway);
	//if(kLookingDir != NiPoint3::ZERO && m_bLockBidirection)
	//{
	//	m_kMovingDir = NiPoint3::ZERO;
	//}
}

NiPoint3 const& PgActor::GetMovingDir()const
{
	return m_kMovingDir;
}

void PgActor::SetMovingDir(NiPoint3	const kMovingDir)
{
	m_kMovingDir = kMovingDir;
	//_PgOutputDebugString("[Set m_kMovingDir 4] Actor(%s) m_kMovingDir(%f,%f,%f)\n", MB(GetPilot()->GetGuid().str()),m_kMovingDir.x,m_kMovingDir.y,m_kMovingDir.z);
}

bool PgActor::ContainsDirection(BYTE byDir)
{
	return (GetLastDirection() & byDir) != 0;
}

void PgActor::SetRotation(const NiQuaternion &kQuat, bool bForce)
{
	if(kQuat != NiQuaternion::IDENTITY && false == bForce && false == EnableRotation())
	{
		return;
	}

	if(m_pkPhysXActor)
	{
		NxQuat kNxQuat;
		NiPhysXTypes::NiQuaternionToNxQuat(kQuat, kNxQuat);
		m_pkPhysXActor->setGlobalOrientationQuat(kNxQuat);
	}

	SetRotate(kQuat);
	m_bIsOptimizeSleep = false;	//회전했으니까 슬립시키면 안됨
}

void PgActor::SetRotation(const NxQuat &kQuat, bool bForce)
{
	if(false == bForce && false == EnableRotation())
	{
		return;
	}

	if(m_pkPhysXActor)
		m_pkPhysXActor->setGlobalOrientationQuat(kQuat);


	NiQuaternion kNiQuat;
	NiPhysXTypes::NxQuatToNiQuaternion(kQuat, kNiQuat);
	SetRotate(kNiQuat);
	m_bIsOptimizeSleep = false;	//회전했으니까 슬립시키면 안됨
}

bool PgActor::SetRotation(float m_fDegree, NiPoint3 kAxis, bool bForce)
{
	if(false == bForce && false == EnableRotation())
	{
		return false;
	}

	NiQuaternion kRot(m_fDegree, kAxis);
	kRot.Normalize();

	SetRotation(kRot);
	m_kToRotation = kRot;
	m_bIsOptimizeSleep = false;	//회전했으니까 슬립시키면 안됨
	
	return true;
}

bool PgActor::EnableRotation()
{
	CUnit *pkUnit = GetPilot()->GetUnit();
	if(pkUnit)
	{
		int iAngleFix = pkUnit->GetAbil(AT_MON_ANGLE_FIX);
		if(0 != iAngleFix)
		{
			return false;
		}
	}

	return true;
}

NiPoint3 const &PgActor::GetLookingDir()const
{
	return m_kLookingDir;
}

BYTE PgActor::GetLastDirection()
{
	//if(m_pkAction && m_pkAction->CanChangeActorPos())
	//{
	//	return m_byMovingDirection;
	//}
	if(!m_bLockBidirection)
	{
		return m_byMovingDirection;
	}

	return m_byLastDirection;
}

BYTE PgActor::GetDirection()
{
	return m_byMovingDirection;
}

Direction	PgActor::GetDirectionInverse(Direction kDirection)	//	kDirection 의 역방향을 구한다.
{
	int	kInverseDirection = DIR_NONE;

	if(((int)kDirection) & DIR_LEFT)	{ kInverseDirection |= DIR_RIGHT; }
	if(((int)kDirection) & DIR_RIGHT)	{ kInverseDirection |= DIR_LEFT; }
	if(((int)kDirection) & DIR_UP)		{ kInverseDirection |= DIR_DOWN; }
	if(((int)kDirection) & DIR_DOWN)	{ kInverseDirection |= DIR_UP; }

	return	(Direction)kInverseDirection;
}
//실수 허용 오차 범위
float const g_fEpsilon = 0.0001f;

BYTE PgActor::GetDirFromMovingVector(NiPoint3 const &rkMovingVector)
{
	NiPoint3 kMovingVector = rkMovingVector;
	kMovingVector.z = 0;
	kMovingVector.Unitize();

	NiPoint3 kCross = m_kPathNormal.UnitCross(kMovingVector);
	if(kCross.SqrLength() < g_fEpsilon)
	{
		kCross = NiPoint3::UNIT_Z;
	}

	return ( (kCross == NiPoint3::UNIT_Z) ? DIR_LEFT : DIR_RIGHT );
}

BYTE PgActor::GetDirFromMovingVector8Way(NiPoint3 const &rkMovingVector)
{
	NiPoint3 kMovingVector = rkMovingVector;

	// 값이 오차가 생길수 있으므로 == 으로 비교하지 않고 오차로 비교 한다.
	// 같은 방향의 벡터를 - 하게 되면 0이 되어야 하지만 오차가 생길 수 있으므로 최소값으로 비교한다.
	
	// 위쪽인지 검사
	if(abs((kMovingVector - m_kPathNormal).SqrLength()) < g_fEpsilon)
	{
		return DIR_UP;
	}

	// 아래 쪽인지 검사
	if(abs((kMovingVector + m_kPathNormal).SqrLength()) < g_fEpsilon)
	{
		return DIR_DOWN;
	}

	NiPoint3 kRotDir, kRotDir2;
	NiMatrix3 kRot;
	kRot.MakeZRotation(NI_HALF_PI);
	kRotDir = kRot * m_kPathNormal;
	
	// PathNormal은 위쪽을 가리키므로 회전시켜서 오른쪽인지 체크
	if(abs((kMovingVector - kRotDir).SqrLength()) < g_fEpsilon)
	{
		return DIR_RIGHT;
	}

	if(abs((kMovingVector + kRotDir).SqrLength()) < g_fEpsilon)
	{
		return DIR_LEFT;
	}

	kRotDir2 = kRotDir;
	kRotDir2 += m_kPathNormal;
	kRotDir2.Unitize();

	// PathNormal은 위쪽을 가리키므로 회전시켜서 왼쪽인지 체크
	if(abs((kMovingVector - kRotDir2).SqrLength()) < g_fEpsilon)
	{
		return DIR_RIGHT + DIR_UP;
	}

	// 반대 방향
	if(abs((kMovingVector + kRotDir2).SqrLength()) < g_fEpsilon)
	{
		return DIR_LEFT + DIR_DOWN;
	}

	kRotDir2 = kRotDir;
	kRotDir2 += -m_kPathNormal;
	kRotDir2.Unitize();

	if(abs((kMovingVector - kRotDir2).SqrLength()) < g_fEpsilon)
	{
		return DIR_RIGHT + DIR_DOWN;
	}

	if(abs((kMovingVector + kRotDir2).SqrLength()) < g_fEpsilon)
	{
		return DIR_LEFT + DIR_UP;
	}

	return DIR_NONE;
}

void PgActor::InvalidateDirection()
{
	unsigned uiSeqLeft = g_kPilotMan.GetDirectionSlot(DIR_LEFT);
	unsigned uiSeqRight = g_kPilotMan.GetDirectionSlot(DIR_RIGHT);
	unsigned uiSeqUp = g_kPilotMan.GetDirectionSlot(DIR_UP);
	unsigned uiSeqDown = g_kPilotMan.GetDirectionSlot(DIR_DOWN);

	BYTE byNewDirection = DIR_NONE;
	byNewDirection |= (uiSeqLeft > uiSeqRight ? DIR_LEFT :
						uiSeqLeft < uiSeqRight ? DIR_RIGHT : DIR_NONE);

	byNewDirection |= (uiSeqUp > uiSeqDown ? DIR_UP :
						uiSeqUp < uiSeqDown ? DIR_DOWN : DIR_NONE);

	SetDirection(byNewDirection);
}

void PgActor::ReserveDirection(BYTE byDirection, DWORD dwDirectionTerm, NiPoint3 &rkCurPos)
{
	//WriteToConsole("[PgActor.ReserveDirection] Direction : %u, ActionTerm : %u\n", byDirection, dwDirectionTerm);
	PgActionEntity kActionEntity(0, byDirection);
	kActionEntity.SetDirectionTerm(dwDirectionTerm);
	kActionEntity.SetDirectionStartPos(rkCurPos);

	AddActionEntity(kActionEntity);

	//WriteToConsole("[PushToActionQueue] Direction : %u, Term : %u\n", byDirection, dwDirectionTerm);
}

void PgActor::SetDirection(BYTE byDirection)
{
	_PgOutputDebugString("%s m_byMovingDirection : %d -> %d \n", __FUNCTIONW__, m_byMovingDirection, byDirection);

	if(DIR_NONE != m_byMovingDirection)
	{
		m_byLastMovingDirection = m_byMovingDirection;
	}
	m_byMovingDirection = byDirection;
	//if(byDirection == DIR_NONE && IsMyActor() == false)
	//{
	//	m_kMovingDir = NiPoint3::ZERO;
	//}

	if (GetPilot() && GetPilot()->GetUnit())
	{
		// 어디에 쓰는 코드인지~
		GetPilot()->GetUnit()->FrontDirection((Direction)byDirection);
	}
}

bool PgActor::AddEffectToAction(PgPilot	*pkTargetPilot,int iEffectNo,int iEffectValue,int iActionInstanceID)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.AddEffectToAction"), g_pkApp->GetFrameCount()));
	if(!m_pkAction) return	false;
	if(m_pkAction->GetActionInstanceID()!=iActionInstanceID)
		return	false;
	if(!pkTargetPilot) return false;

	PgActionTargetList	*pkTargetList = m_pkAction->GetTargetList();

	if(pkTargetList==NULL) return	false;
	if(pkTargetList->IsActionEffectApplied()) return false;

	PG_ASSERT_LOG(GetPilot());
	PgActionTargetInfo	*pkTargetInfo = pkTargetList->GetTargetByGUID(pkTargetPilot->GetGuid());
	if(!pkTargetInfo) return false;
	/*{
		PgActionResult	kNewResult;
		kNewResult.AddStatusEffect(iEffectNo,iEffectValue);
		kNewResult.SetInvalid(false);
		PgActionTargetInfo	kNewTargetInfo;
		kNewTargetInfo.m_bySphereIndex = 0;
		kNewTargetInfo.m_kActionResult = kNewResult;

		kNewTargetInfo.m_iRemainHP =INVALID_REMAIN_HP;
		kNewTargetInfo.m_TargetPilotGUID = pkTargetPilot->GetGuid();
		pkTargetList->GetList().push_back(kNewTargetInfo);
		return	true;
	}*/

	pkTargetInfo->GetActionResult().AddStatusEffect(iEffectNo,iEffectValue);

	return	true;
}
void	PgActor::AddIgnoreEffect(int iEffectID)
{
	if(IsIgnoreEffect(iEffectID))
	{
		return;
	}

	m_IgnoreEffectList.push_back(iEffectID);
}
void	PgActor::RemoveIgnoreEffect(int iEffectID)
{
	for(IntList::iterator itor = m_IgnoreEffectList.begin(); itor != m_IgnoreEffectList.end(); ++itor)
	{
		if(*itor == iEffectID)
		{
			m_IgnoreEffectList.erase(itor);
			return;
		}
	}
}
void	PgActor::ClearIgnoreEffectList()
{
	m_IgnoreEffectList.clear();
}
bool	PgActor::IsIgnoreEffect(int iEffectID)
{
	for(IntList::iterator itor = m_IgnoreEffectList.begin(); itor != m_IgnoreEffectList.end(); ++itor)
	{
		if(*itor == iEffectID)
		{
			return	true;
		}
	}
	PgPilot* pkPIlot = GetPilot();
	if( !pkPIlot )
	{
		return true;
	}
	CUnit* pkUnit = pkPIlot->GetUnit();
	if( !pkUnit )
	{
		return true;
	}
		
	SSFilter_Result kResult;
	if( !pkUnit->CheckSkillFilter(iEffectID, NULL, ESFilter_Ignore_Action_Effect)
		|| !pkUnit->CheckSkillFilter(iEffectID, &kResult)
		)
	{
		return true;
	}

	return	false;
}

bool PgActor::AddEffect(int const iEffectNo, int  const iEffectValue, float  const fElapsedTime, BM::GUID const &kCasterGUID, int  const iActionInstanceID, DWORD  const dwTimeStamp, bool  const bIsTemporaryEffect, CSkillDef const* pkSkillDef)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.AddEffect"), g_pkApp->GetFrameCount()));

	int iApplyEffectNo = iEffectNo;
	if(ACTIONEFFECT_DIE==iEffectNo && GetUnit() && GetUnit()->GetAbil(AT_VOLUNTARILY_DIE) )
	{
		iApplyEffectNo = ACTIONEFFECT_VOLUNTARILY_DIE;
	}

	//	무시할 이펙트 리스트에 들어있다면 리턴!
	if(IsIgnoreEffect(iApplyEffectNo))
	{
		return false; 
	}

	//	leesg213 액션이펙트 관련 수정
	NILOG(PGLOG_LOG,"PgActor::AddEffect EffectNo : %d CasterGUID : %s ActorGUID : %s ActorName : %s\n",iApplyEffectNo,MB(kCasterGUID.str()),MB(GetPilotGuid().str()),MB(GetPilot()->GetName()));
	_PgOutputDebugString("PgActor::AddEffect EffectNo : %d CasterGUID : %s ActorGUID : %s ActorName : %s\n",iApplyEffectNo,MB(kCasterGUID.str()),MB(GetPilotGuid().str()),MB(GetPilot()->GetName()));

	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pkEffectDef = kEffectDefMgr.GetDef(iApplyEffectNo);
	if(!pkEffectDef) 
	{
		NILOG(PGLOG_ERROR, "[PgActor] %d effect no, can't find effectdef\n", iApplyEffectNo);
		return false;
	}

	if( (pkEffectDef->GetType() != EFFECT_TYPE_CURSED)
	 && (pkEffectDef->GetType() != EFFECT_TYPE_SYSTEM) )
	{ //디버프(강제적) 효과 이면..
		g_kStatusEffectMan.AddStatusEffect(GetPilot(),NULL,0,iApplyEffectNo,iEffectValue,false,true);
		return true;
	}

	//	ActionEffect
	switch( iApplyEffectNo )
	{
	case ACTIONEFFECT_REVIVE:
	case ACTIONEFFECT_RESURRECTION01:
		{
			//	다시 살아 나면 이름을 렌더링 하자
			m_bNoName = false;
			UpdateName();

			// Gauge Bar Refresh
			int	const iCurrentHP = GetPilot()->GetAbil(AT_HP);
			RefreshHPGaugeBar(iCurrentHP,iCurrentHP,this);
		}break;
	case ACTIONEFFECT_DIE:
	case ACTIONEFFECT_VOLUNTARILY_DIE:
		{
			ReserveDieByActioin(true);

			if(GetPilot())
			{
				// 죽을 때 HP가 조금 남아 보는 경우가 있음.
				int	const iCurrentHP = GetPilot()->GetAbil(AT_HP);
				RefreshHPGaugeBar(iCurrentHP, 0, this);
			}

			//	죽었을 경우 이름을 렌더링하지 말자.
			m_bNoName = true;

			if( GetPilot() )
			{
				PgPilot *pkPilot = GetPilot();
				if( pkPilot->GetUnit() )
				{
					if(pkPilot->GetUnit()->IsInUnitType( UT_PLAYER ) && !IsMyActor() )
					{
						SChatLog kChatLog(CT_EVENT_SYSTEM);
						std::wstring kLog = GetPilot()->GetName() + TTW(700012);
						g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
					}
					else if(pkPilot->GetUnit()->IsInUnitType( UT_MONSTER ) && g_pkWorld)
					{
						g_pkWorld->DeleteEffectRemainTime(pkPilot->GetGuid());
					}
				}

				//	상태이상을 모두 해제시킨다.
				g_kStatusEffectMan.RemoveAllStatusEffect(GetPilot());

				//	떨궈야 할 아이템이 있다면 떨군다.(단, 정예는 다른 방식으로 떨굴것이므로 예외로 한다)
				if(GetPilot()->GetAbil(AT_GRADE) != EMGRADE_ELITE)
				{
					DoDropItems();
				}
			}

			//	죽었을 때 실행할 이벤트 스크립트가 있으면 실행
			g_kEventScriptSystem.ActivateEvent(GetEventScriptIDOnDie());

			// 죽으면 스킬세트 취소
			SkillSetAction().ReserveActionCancel();
		}break;
	case ACTIONEFFECT_BLOCK:
		{// 블럭시
			if( pkSkillDef )
			{
				PgPilot* pkPilot = GetPilot();
				if(pkPilot)
				{// 밀리는 인자 값들을 세팅
					int const iDist = lua_tinker::call< int, int >("GetPushActorSkillInfo_Distance", pkSkillDef->No() );
					pkPilot->SetAbil(AT_PUSH_DIST_WHEN_BLOCK, iDist );

					int const iSpeed = lua_tinker::call< int, int >("GetPushActorSkillInfo_Velocity", pkSkillDef->No() );
					pkPilot->SetAbil(AT_PUSH_SPEED_WHEN_BLOCK, iSpeed );

					int const iAccel = lua_tinker::call< int, int >("GetPushActorSkillInfo_Accel", pkSkillDef->No() );
					pkPilot->SetAbil(AT_PUSH_ACCEL_WHEN_BLOCK, iAccel );
				}
			}
		}break;
	}

	m_pkActionEffectStack->AddNewEffect(kCasterGUID,iActionInstanceID,bIsTemporaryEffect,iApplyEffectNo,dwTimeStamp);
	return true;
}

void PgActor::RemoveEffectFromAction(PgPilot *pkTargetPilot, int iEffectNo)
{
	if(!m_pkAction) return;
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.RemoveEffectFromAction"), g_pkApp->GetFrameCount()));

	PgActionTargetList	*pkTargetList = m_pkAction->GetTargetList();

	if(pkTargetList==NULL) return	;
	if(pkTargetList->IsActionEffectApplied()) return ;

	PG_ASSERT_LOG(GetPilot());
	PgActionTargetInfo	*pkTargetInfo = pkTargetList->GetTargetByGUID(pkTargetPilot->GetGuid());
	if(!pkTargetInfo)
		return;

	PgActionResult	kActionResult = pkTargetInfo->GetActionResult();

	pkTargetInfo->GetActionResult().ClearStatusEffect();

	for(size_t i = 0; i<kActionResult.GetStatusEffectNum(); ++i)
	{
		PgActionResult::stStatusEffect	*pkEffectInfo = kActionResult.GetStatusEffect(i);
		if(pkEffectInfo && pkEffectInfo->m_iEffectID != iEffectNo)
		{
			pkTargetInfo->GetActionResult().AddStatusEffect(pkEffectInfo->m_iEffectID,pkEffectInfo->m_iEffectValue);
		}
	}
}

void PgActor::DetachNameNodes(NiAVObject* pkNIFRoot, char const* const szTargetNameNode)
{
	if( !pkNIFRoot )
	{
		return;
	}

	NiNode	*pkNameTargetNode = NiDynamicCast(NiNode, pkNIFRoot->GetObjectByName(szTargetNameNode));
	if( pkNameTargetNode )
	{
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spNameText);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spTitleName);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spGuildNameText);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spGuildMark);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spGuardianMark);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spMyhomeMark);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spCustomCountText);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spAchievementTitle);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spGIFTitle);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spDuelTitle);
		PgActorUtil::DetachFromNode(pkNameTargetNode, m_spEffectCountDown);
	}
}

void PgActor::AttachNameNodes(NiAVObject* pkNIFRoot, char const* const szTargetNameNode)
{
	if( !pkNIFRoot )
	{
		return;
	}

	NiNode	*pkNameTargetNode = NiDynamicCast(NiNode, pkNIFRoot->GetObjectByName(szTargetNameNode));
	if( pkNameTargetNode )
	{
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spNameText);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spTitleName);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spGuildNameText);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spGuildMark);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spGuardianMark);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spMyhomeMark);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spCustomCountText);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spAchievementTitle);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spGIFTitle);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spDuelTitle);
		PgActorUtil::AttachToNode(pkNameTargetNode, m_spEffectCountDown);
	}
}

NiNode* PgActor::GetNodePointStar(void)
{
	PgPilot* pkPilot = GetPilot();
	NiNode* pkNodeRet = NULL;
	if(!pkPilot)
	{
		return NULL;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return NULL;
	}

/*	if(!pkUnit->IsUnitType(UT_PLAYER))
	{
		return NULL;
	}
*/
	PgActor* pkActorPet = GetMountTargetPet();
	if(pkActorPet) //펫에 탑승 중이라면 플레이어의 이름 노드를 사용하지 않고
	{				//펫의 이름 노드를 사용. 탑승 후 플레이어 노드 사용시 이름이 왔다갔다거림
		NiAVObject* pkRoot = pkActorPet->GetNIFRoot();
		if(pkRoot)
		{
			pkNodeRet = NiDynamicCast(NiNode, pkRoot->GetObjectByName(ATTACH_POINT_RIDENAME));
		}
	}
	else //플레이어의 디폴트 이름 노드
	{
		NiAVObject* pkRoot = GetNIFRoot();
		if(pkRoot)
		{
			pkNodeRet = NiDynamicCast(NiNode, pkRoot->GetObjectByName(ATTACH_POINT_STAR));
		}
	}
	return pkNodeRet;
}

void PgActor::RestoreTransformation(char const *pcFirstAction)
{
	m_kTransformedActorID = "";

	if(!m_bTransformed)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.RestorTransformation"), g_pkApp->GetFrameCount()));
	NiActorManager *pkAM = GetActorManager();
	if(!pkAM) return;

	NiAVObject *pkRoot = pkAM->GetNIFRoot();

	DetachNameNodes(pkRoot); // 이름들(길드명, 길드마크, 등)을 변신해제 전에 Node에서 때준다.

#ifdef PG_USE_ACTORMANAGER_REUSE
	pkRoot->SetAppCulled(true);
	/*	//	이부분 주석처리해놓은 후 크래시가 사라졌다. 아직 정확한 원인은 찾을 수 없다.
	DetachChild(pkRoot);
	UpdateEffects();
	UpdateNodeBound();
	UpdateProperties();
	Update(0.0f, true);*/
	g_kAMPool.ReturnActorManager(pkAM);
#else
	pkRoot->SetAppCulled(true);
	THREAD_DELETE_OBJECT(DetachChild(pkRoot));
	Update(0.0f, true);
	UpdateProperties();
#endif

	if(m_pkWorldObjectBase)
	{		
		m_pkWorldObjectBase->DetachEffects(this);
	}

	RemoveSubIWorldObjectBase();


	// 변신 해제!!

	pkAM = GetActorManager();

	PG_ASSERT_LOG(pkAM);
	pkAM->GetNIFRoot()->SetAppCulled(false);
	m_kSeqID = NiActorManager::INVALID_SEQUENCE_ID;

	AttachNameNodes(pkAM->GetNIFRoot()); // 이름을 다시 붙인다.

	std::wstring kEventScript = m_pkWorldObjectBase->GetEventScript();
	if(kEventScript.length() != 0)
	{
		PG_ASSERT_LOG(m_pkActorCallback);
		if(m_pkActorCallback)
		{
			m_pkActorCallback->m_pkWorldObject = this;
			m_pkActorCallback->m_kScriptName = MB(kEventScript);

			pkAM->SetCallbackObject(m_pkActorCallback);
		}
	}

	//	현재 액션을 캔슬시킨다.
	if(m_pkAction
		&& m_pkAction->GetActionType() != "IDLE"
		)
	{ 
		CancelAction(m_pkAction->GetActionNo(),m_pkAction->GetActionInstanceID(),ACTIONNAME_IDLE);
	}


	if(pcFirstAction)
	{
		TransitAction(pcFirstAction);
	}
	else
	{
		DoReservedAction(RA_IDLE);
	}

	m_bTransformed = false;
	UpdateName();	//이름 원래대로
}

void PgActor::Transformation(char const *pcNewModel, char const *pcFirstAction, int const iTransformEffectID)
{
	if(m_bTransformed)
	{
		return;
	}
	if(!GetActorManager())
		return;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.Transformation"), g_pkApp->GetFrameCount()));
	// TODO : ABV를 새로 생성하긴 하는데, PhysX와 물리질 않고 있다 -ㅅ-; 흑흑 귀찮아~
	PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase;
	TiXmlDocument *pkXmlDoc = PgXmlLoader::GetXmlDocumentByID(pcNewModel);
	if(!pkXmlDoc)
	{
		//SAFE_DELETE(pkXmlDoc);
		SAFE_DELETE_NI(pkNewBase);
		return;
	}

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		//SAFE_DELETE(pkXmlDoc);
		SAFE_DELETE_NI(pkNewBase);
		return;
	}

	bool bFindWorldTag = false;
	pkRootNode = pkRootNode->FirstChildElement();
	while(pkRootNode)
	{
		char const *pcTagName = pkRootNode->Value();
		if(strcmp(pcTagName, "WORLDOBJECT") == 0)
		{
			bFindWorldTag = true;
			break;
		}
		pkRootNode = pkRootNode->NextSiblingElement();	
	}

	if(!bFindWorldTag)
	{
		//SAFE_DELETE(pkXmlDoc);
		SAFE_DELETE_NI(pkNewBase);
		return;
	}

	NiActorManager *pkAM = GetActorManager();
	PG_ASSERT_LOG(pkAM);

	DetachNameNodes(pkAM->GetNIFRoot()); // 기존에서 이름들을 때주고

	// 변신!!
	m_bTransformed = pkNewBase->ParseXml(pkRootNode, this, true);
	if(m_bTransformed)
	{
		if(0 < iTransformEffectID)
		{
			// 변신할때 붙은 이펙트(파티클)들에게 상태 이펙트 번호를 정해준다 
			CONT_TRANSFORM_EFFECT_ATTACH_INFO::iterator itor = m_kContTransformEffAttachInfo.begin();
			while(m_kContTransformEffAttachInfo.end() != itor)
			{
				if( PgActor::E_NONE_INIT == itor->iTransformEffectID)
				{
					itor->iTransformEffectID = iTransformEffectID;
					break;
				}
				++itor;
			}
		}
		
		if(g_pkWorld)
		{//변신시에 글로우 체크
			PgRenderer::EnableGlowMap(this,g_pkWorld->GetSpotLightOn() || AlwaysGlowMap());
		}
	
		m_kSeqID = NiActorManager::INVALID_SEQUENCE_ID;

		// 기존의 AM을 숨긴다.
		pkAM->GetNIFRoot()->SetAppCulled(true);

		// 새로 만든 AM을 가져온다.
		pkAM = GetActorManager();
		PG_ASSERT_LOG(pkAM);

		AttachNameNodes(pkAM->GetNIFRoot()); // 이름들(길드명, 길드마크, 등)을 변신한 Node에 다시 붙여 준다.

		// Event Callback이 있으면 등록한다.
		std::wstring kEventScript = pkNewBase->GetEventScript();
		if(pkAM && kEventScript.length() != 0)
		{
			if(!m_pkActorCallback)
			{
				m_pkActorCallback = NiNew ActorCallbackObject;	
			}

			m_pkActorCallback->m_pkWorldObject = this;
			m_pkActorCallback->m_kScriptName = MB(kEventScript);

			pkAM->SetCallbackObject(m_pkActorCallback);
		}

		pkAM->GetNIFRoot()->SetAppCulled(false);

		// 위치를 맞춘다.
#ifdef PG_USE_CAPSULE_CONTROLLER
		pkAM->GetNIFRoot()->SetTranslate(NiPoint3(0, 0, -(m_pkController->getHeight() * 0.5f + m_pkController->getRadius())));
#else
		pkAM->GetNIFRoot()->SetTranslate(NiPoint3(0, 0, -(PG_CHARACTER_CAPSULE_HEIGHT * 0.5f + PG_CHARACTER_CAPSULE_RADIUS)));
#endif

		//	현재 액션을 캔슬시킨다.
		if(m_pkAction)
		{
			CancelAction(m_pkAction->GetActionNo(),m_pkAction->GetActionInstanceID(),ACTIONNAME_IDLE);
		}

		if(pcFirstAction&& 
			::strlen(pcFirstAction) 
			)
		{
			TransitAction(pcFirstAction);
		}
		else
		{
			DoReservedAction(RA_IDLE);
		}
	}
	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_DEVELOP:
	case LOCAL_MGR::NC_VIETNAM:
		{// 베트남은
			if(TRANSFORM_ID_NINJA == iTransformEffectID)
			{// 닌자 변신시에 무기 사이즈를 줄여 버리는 기능을 선택적으로 사용하고 (g_fWeaponSize은 ini로 설정)
				lwActor kTemp(this); 
				kTemp.SetNodeScale("Bip01 Prop1", g_fWeaponSize);
				kTemp.SetNodeScale("Bip01 Prop2", g_fWeaponSize);
			}
		}break;
	}
	//SAFE_DELETE(pkXmlDoc);

	m_kTransformedActorID = std::string(pcNewModel);
}

bool PgActor::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{ 
	int const iType = pkNode->Type();
	
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ParseXml"), g_pkApp->GetFrameCount()));
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "ACTOR") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();
					if (strcmp(pcName, "NAME") == 0)
					{
						SetName(pcValue);
					}
					else if (strcmp(pcName, "ORIGINAL_XML") == 0)
					{
						std::string strName = m_kName;
						PgXmlLoader::Get()->CreateObject(pcValue, pArg, this);
						SetName(strName.c_str());
					}
					else if (strcmp(pcName,"DIE_PARTICLE_ID") == 0)
					{
						m_kDieParticleID = std::string(pcValue);
					}
					else if (strcmp(pcName,"DIE_PARTICLE_NODE") == 0)
					{
						m_kDieParticleNode = std::string(pcValue);
					}
					else if (strcmp(pcName,"DIE_PARTICLE_SCALE") == 0)
					{
						m_fDieParticleScale = (float)atof(pcValue);
					}
					else if (strcmp(pcName,"DIE_SOUND_ID") == 0)
					{
						m_kDieSoundID = std::string(pcValue);
					}
					else if (strcmp(pcName,"LOADING_COMPLETE_INIT") == 0)
					{
						m_kLoadingCompleteInitFunc = std::string(pcValue);
					}
					else if (stricmp(pcName,"EVENT_SCRIPT_ON_DIE") == 0)
					{
						SetEventScriptIDOnDie(atoi(pcValue));
					}
					else  if (stricmp(pcName,"USE_BATTLE_IDLE") == 0)
					{
						SetUseBattleIdle(strcmp( pcValue, "TRUE" ) == 0);
					}
					else if (0 == stricmp(pcName,"ALWAYS_GLOWMAP"))
					{
						AlwaysGlowMap(strcmp( pcValue, "TRUE" ) == 0);
					}
					else if (0 == stricmp(pcName,"CAN_RIDE"))
					{
						m_bCanRide = (strcmp( pcValue, "TRUE" ) == 0);
					}
					else  if (stricmp(pcName,"SHOW_WARNING") == 0)
					{
						m_bShowWarning = (strcmp( pcValue, "TRUE" ) == 0);
					}
					else  if (stricmp(pcName,"INIT_SHOW_ACTOR") == 0)
					{
						bool const bInitShow = (strcmp( pcValue, "TRUE" ) == 0);
						SetHide( !bInitShow );
					}
					else  if (stricmp(pcName,"IDLE_EFFECT_NAME") == 0)
					{
						IdleEffectName(pcValue);
					}
					else if (stricmp(pcName,"IDLE_EFFECT_NODE") == 0)
					{
						IdleEffectNode(pcValue);
					}
					else
					{
						//PG_ASSERT_LOG(!"Unknown Attributes!");
					}
					pkAttr = pkAttr->Next();
				}

				// Actor파싱 할 때 기본 액션을 넣어준다.
				m_kReservedAction.insert(std::make_pair(RA_IDLE, ACTIONNAME_IDLE));
				m_kReservedAction.insert(std::make_pair(RA_OPENING, "a_opening"));
				m_kReservedAction.insert(std::make_pair(RA_INTRO_IDLE, "a_intro_idle"));

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode, pArg))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "PILOTPATH") == 0)
			{
				if(!m_pkPilot)
				{
					m_pkPilot = dynamic_cast<PgPilot *>(PgXmlLoader::CreateObjectFromFile(pkElement->GetText()));
					if(m_pkPilot)
					{
						m_pkPilot->SetWorldObject(this);
					}
				}
			}
			else if(strcmp(pcTagName, "RESERVED_ACTION") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();
					if (strcmp(pcName, "OPENING") == 0)
					{
						m_kReservedAction[RA_OPENING] = pcValue;
					}
					else if(strcmp(pcName, "IDLE") == 0)
					{
						m_kReservedAction[RA_IDLE] = pcValue;
					}
					else
					{
						PG_ASSERT_LOG(!"Unknown Attributes!");
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "WORLDOBJECT") == 0)
			{
				PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase;
				if(pkNewBase->ParseXml(pkNode, this))
				{
					std::wstring kEventScript = pkNewBase->GetEventScript();
					if(kEventScript.length() != 0)
					{
						m_pkActorCallback = NiNew ActorCallbackObject;
						if(!m_pkActorCallback)
						{
							PG_ASSERT_LOG(!"failed to creat ActorCallbackObject");
							return false;
						}

						m_pkActorCallback->m_pkWorldObject = this;
						m_pkActorCallback->m_kScriptName = MB(kEventScript);

						if(!GetActorManager())
						{
							PG_ASSERT_LOG(!"ActorCallbackObject : ActorManager must be initialized prior to Callback Object!");
							return false;
						}
						GetActorManager()->SetCallbackObject(m_pkActorCallback);

						if (pArg)
						{
							PgIXmlObject::XmlObjectID eObjectID = *((PgIXmlObject::XmlObjectID*)pArg);
							SetObjectID(eObjectID);
						}
					}
				}
				else
				{
					return false;
				}
			}
			else if(strcmp(pcTagName,"APPEARANCE_CHANGE")==0)
			{
				SAFE_DELETE(m_pkActorAppearanceMan);
				m_pkActorAppearanceMan = new PgActorAppearanceMan(this);
				m_pkActorAppearanceMan->ParseXml(pkNode);
			}
			else if(strcmp(pcTagName, "NO_NAME") == 0)
			{
				if (atoi(pcTagName))
				{
					m_bNoName = true;
				}
			}
			else if(strcmp(pcTagName, "TEXTURE") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				if(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();

					if(strcmp(pcName, "SRC") == 0)
					{
						char const *pcText = pkElement->GetText();
						m_VarTextureList.insert(std::make_pair(std::string(pcValue), std::string(pcText)));
					}
				}
			}
			else if(strcmp(pcTagName, "COLORSHADOW") == 0)
			{
/*				if( m_pCircleShadow )
				{
					NiDelete m_pCircleShadow;
				}

				m_pCircleShadow = NiNew PgColorShadow;*/

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "SCALE") == 0)
					{
				//		m_pCircleShadow->SetShadowScale( (float)atof(pkAttr->Value()) );
					}
					else if(strcmp(pkAttr->Name(), "PARTICLE") == 0)
					{
				/*		PgColorShadow *pkColorShadow = dynamic_cast<PgColorShadow*>(m_pCircleShadow);
						if ( pkColorShadow )
						{
							pkColorShadow->AttachColorShadow( pkAttr->Value() );
						}*/
					}
					else if(strcmp(pkAttr->Name(), "MAX") == 0)
					{
				//		m_pCircleShadow->SetMaxShadowDistance( (float)atof(pkAttr->Value()) );
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "SHADOW") == 0)
			{
				/*if( m_pCircleShadow )
				{
					NiDelete m_pCircleShadow;
				}

				m_pCircleShadow = NiNew PgCircleShadow;*/

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "SCALE") == 0)
					{
						//m_pCircleShadow->SetShadowScale( (float)atof(pkAttr->Value()) );
					}
					else if(strcmp(pkAttr->Name(), "MAX") == 0)
					{
						//m_pCircleShadow->SetMaxShadowDistance( (float)atof(pkAttr->Value()) );
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "NPCMARK") == 0)
			{
				m_kNpcMarkInfo.IsUse = true;
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "NAME") == 0)
					{
						m_kNpcMarkInfo.kEffectID = pkAttr->Value();
					}
					else if(strcmp(pkAttr->Name(), "ATTACH_TO") == 0)
					{
						m_kNpcMarkInfo.kTargetNodeName = pkAttr->Value();
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "UIMODEL") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "DRAW_HEIGHT") == 0)
					{
						m_kUIModelOpt.fDrawHeight = atoi(pkAttr->Value()) / 100.0f;
					}
					else if(strcmp(pkAttr->Name(), "INCREASE_CAM_RADIUS") == 0)
					{
						m_kUIModelOpt.fIncreaseCamRad = static_cast<float>( atof(pkAttr->Value()) );
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "HEAD") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "DEFAULT_SIZE") == 0)
					{
						m_fTargetHeadSize = PgStringUtil::SafeAtof( pkAttr->Value() );
						SetDefaultHeadSize(m_fTargetHeadSize);
					}
					pkAttr = pkAttr->Next();
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode, pArg))
		{
			return false;
		}
	}

	// 모든 파싱이 끝났다면 Actor를 초기화한다.
	if(strcmp(pkNode->Value(), "ACTOR") == 0)
	{
		NiActorManager *pkAM = GetActorManager();
		if(!pkAM)
		{
			return false;
		}

		pkAM->Update(0.0f);
		NiTimeController::StartAnimations(GetNIFRoot(), 0.0f); //PgWorld에 Attach될 때 알아서 된다.
		AMContainer::iterator itr = m_kSupplementAMContainer.begin();
		while(itr != m_kSupplementAMContainer.end())
		{
			PG_ASSERT_LOG(itr->m_spAM);
			if (itr->m_spAM)
				itr->m_spAM->Update(0.0f);
			++itr;
		}

		// Actor를 붙일 때는, 무조건 숨긴 다음 로딩을 다 하면 Alpha로 빼준다.
		pkAM->GetNIFRoot()->SetAppCulled(true);
		NiNode::SetAppCulled(true);

	/*	if(!m_pCircleShadow)
		{
			m_pCircleShadow = NiNew PgCircleShadow();
		}*/

		// 바리에이션 텍스쳐를 로딩한다.
		if (m_VarTextureList.size() > 0)
		{
			ChangeTexture(this);
		}

		if(m_kNpcMarkInfo.IsUse)
		{// NPC 표시를 사용한다면
			static int const iSlotNo = 9812341;
			NiAVObject *pkParticle = NULL;
			char const* const pcEffectStr = m_kNpcMarkInfo.kEffectID.c_str();
			char const* const pcTargetNodeName = m_kNpcMarkInfo.kTargetNodeName.c_str();
			pkParticle = dynamic_cast<NiAVObject*>( g_kParticleMan.GetParticle(pcEffectStr, PgParticle::O_SCALE,GetEffectScale()) );
			if ( pkParticle )
			{
				if(! AttachTo( iSlotNo, pcTargetNodeName, pkParticle ) )
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
			}
		}
	}

	return true;
}

bool PgActor::AttachToSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax)
{
	if( !PgActorUtil::IsCanPlaySound(this) )
	{
		return true;
	}
	return PgIWorldObject::AttachToSound(uiType,pcID,fVolume,fDistMin,fDistMax);
}

bool PgActor::PlayNewSound(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax)
{
	return AttachToSound(uiType, pcID, fVolume, fDistMin, fDistMax);
}

bool PgActor::AddNewParticle(char const *szParticleID, int const iSlot, char const *szTargetName, float const fScale, bool const bLoop, bool const bZTest, float const fSoundTime, bool const bAutoGround, bool const bUseAppAccumTime, bool bNoFollowParentRotation)
{
	NiAVObject *pkParticle = g_kParticleMan.GetParticle(szParticleID, PgParticle::O_ALL,fScale, bLoop, bZTest, fSoundTime, bAutoGround, bUseAppAccumTime, bNoFollowParentRotation); // Effect
	if( !pkParticle )
	{
		return false;
	}
	if(!AttachTo(iSlot, szTargetName, pkParticle))
	{
		THREAD_DELETE_PARTICLE(pkParticle);
		return	false;
	}
	return true;
}

//!	진행중인 퀘스트 정보 업데이트
//void PgActor::UpdateQuestUserInfo(const SUserQuestState& rkUpdatedState)
//{
//	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateQuestUserInfo"), g_pkApp->GetFrameCount()));
//	PgPilot	*pPilot = GetPilot();
//	if( !pPilot )
//	{
//		return;
//	}
//
//	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
//	if( !pkPlayer )
//	{
//		return;
//	}
//
//	PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(rkUpdatedState.iQuestID);
//	if( !pkQuestInfo )
//	{
//		return;
//	}
//
//	PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
//	if( pkMyQuest )
//	{
//		int const iQuestID = rkUpdatedState.iQuestID;
//		//퀘스트 상태가 변경되면 공지로 보여준다.
//		SUserQuestState kOldState;
//		SUserQuestState const *pkOldState = pkMyQuest->Get(iQuestID);
//		if( pkOldState )
//		{
//			kOldState = *pkOldState;//이전 상태 백업
//		}
//
//		//퀘스트 상태가 변경되면 공지로 보여준다.
//		g_kQuestMan.ShowQuestInfo(kOldState, rkUpdatedState);
//
//		BYTE const cUpdatedState = rkUpdatedState.byQuestState;
//		if( QS_Finished == cUpdatedState )
//		{
//			switch( pkQuestInfo->Type() )
//			{
//			case QT_Loop:
//				{
//					// 반복 퀘스트는 클라이언트에서 저장 플래그를 세우지 않는다.
//					pkMyQuest->DropQuest(iQuestID);
//				}break;
//			default:
//				{
//					pkMyQuest->CompleteQuest(iQuestID, pkQuestInfo->Type());
//				}break;
//			}
//
//			g_kQuestMan.RemoveMiniQuestList(iQuestID);	// Mini 정보창에서 제거
//			g_kQuestMan.DelRecentBeginQuest(iQuestID);	// 최근에 시작한 퀘스트 정보에서 제거
//			PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete2", 1.0f);
//
//			if( pkQuestInfo
//			&&	g_pkWorld
//			&&	-1 != pkQuestInfo->CompleteEventScriptID() )
//			{
//				g_kEventScriptSystem.ActivateEvent(pkQuestInfo->CompleteEventScriptID(), g_pkWorld->GetAccumTime(), g_pkWorld->GetFrameTime());
//			}
//
//			g_kQuestMan.IsComplete(iQuestID);
//			CXUI_Wnd *pkQuestDialogWnd = XUIMgr.Get(_T("FRM_QUEST_FULLSCREEN"));
//			if( !pkQuestDialogWnd )
//			{
//				g_kQuestMan.CallCompleteStamp();
//			}
//		}
//		else if( QS_None == cUpdatedState )
//		{
//			pkMyQuest->DropQuest(iQuestID);
//			g_kQuestMan.DelRecentBeginQuest(iQuestID);
//			PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Drop", 1.0f);
//
//			g_kQuestMan.RemoveMiniQuestList(iQuestID);	// Mini 정보창에서 제거
//		}
//		else
//		{
//			if( !pkMyQuest->UpdateQuest(rkUpdatedState) )
//			{
//				//새로 시작된 퀘스트면
//				pkMyQuest->AddIngQuest(&rkUpdatedState, pkQuestInfo->Type());
//				
//				g_kQuestMan.AddMiniQuestList(iQuestID);		// Mini 정보창에 추가
//				g_kQuestMan.AddRecentBeginQuest(iQuestID);	// 최근에 시작한 퀘스트 정보에 추가
//				PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Accept", 1.0f);
//
//				if( pkQuestInfo
//				&&	g_pkWorld
//				&&	-1 != pkQuestInfo->AcceptEventScriptID() )
//				{
//					g_kEventScriptSystem.ActivateEvent(pkQuestInfo->AcceptEventScriptID(), g_pkWorld->GetAccumTime(), g_pkWorld->GetFrameTime());
//				}
//			}
//
//			if( QS_End == cUpdatedState )//모든 조건을 충족 했을 때
//			{
//				PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete1", 1.0f);
//			}
//			else if( QS_Failed == cUpdatedState )
//			{
//				PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Failed", 1.0f);
//			}
//		}
//	}
//
//}

//void PgActor::SetSyncPosition(NiPoint3 &rkSyncPos)
//{
//	//m_bForceSync = true;
//	m_kSyncPosition = rkSyncPos;
//}
//
//void PgActor::SetSyncInterpolDelta(NxVec3 &rkDelta)
//{
//	m_kSyncInterpolDelta = rkDelta;
//	m_fSyncInterpolTime = PG_SYNC_INTERPOL_TIME;
//	//_PgOutputDebugString("Moving Delta Inter Delta (%s): %.f, %.f, %.f\n", MB(m_pkPilot->GetGuid().str()), m_kSyncInterpolDelta.x, m_kSyncInterpolDelta.y, m_kSyncInterpolDelta.z);
//}
//
//void PgActor::SetSyncInterpolTime(float fTime)
//{
//	m_fSyncInterpolTime = fTime;
//}

void PgActor::SetParam(char const *pcKey, char const *pcVal)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetParam"), g_pkApp->GetFrameCount()));
	ScriptParamContainer::iterator itr = m_kScriptParamContainer.find(pcKey);
	if(itr != m_kScriptParamContainer.end())
	{
		itr->second = std::string(pcVal);
	}
		
	m_kScriptParamContainer.insert(std::make_pair(std::string(pcKey), std::string(pcVal)));
}

char const*	PgActor::GetParam(char const *pcParamName)
{
	if(NULL == pcParamName)
	{
		return NULL;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetParam"), g_pkApp->GetFrameCount()));
	ScriptParamContainer::iterator itr = m_kScriptParamContainer.find(pcParamName);
	if(itr != m_kScriptParamContainer.end())
	{
		return itr->second.c_str();
	}
	
	return NULL;
}

void	PgActor::AddStatusEffectInstance(PgStatusEffectInstance* pkInstance)
{
	if(!pkInstance)
	{
		return;
	}
	m_StatusEffectInstanceList.push_back(pkInstance);

	if(GetUnit() && GetUnit()->IsUnitType(UT_PET))
	{//타겟이 펫인가? 그렇다면 이 파티클은 다른 액터로 이동시키면 안된다
		int iSlot = pkInstance->GetInstanceID() * 1000000;
		m_kFixedParticleList.insert(iSlot);
	}

	if(pkInstance->GetStatusEffect())
	{
		if(false==pkInstance->GetStatusEffect()->GetScriptName().empty())
		{
			GET_DEF(CEffectDefMgr, kEffectDefMgr);
			CEffectDef const* pEffDef = kEffectDefMgr.GetDef(pkInstance->GetEffectID());
			std::string strScriptName = pkInstance->GetStatusEffect()->GetScriptName();
			strScriptName += "_Begin";
			lua_tinker::call<void,lwActor,int>( strScriptName.c_str(), lwActor(this), pkInstance->GetEffectID(), pkInstance->GetEffectKey() );

			if(pEffDef && pEffDef->GetInterval())
			{
				PgStatusEffectManUtil::SEffectUpdateInfo kInfo(pkInstance, pEffDef->GetInterval()*0.001f);
				m_StatusEffectInstanceListForUpdate.push_back(kInfo);
			}
		}
	}
}
bool	PgActor::CheckStatusEffectExist(char const* strStatusEffectXMLID)
{
	if(!GetPilot())
	{
		return false;
	}

	PgStatusEffectInstance* pkInstance = NULL;
	for(StatusEffectInstanceList::iterator itor = m_StatusEffectInstanceList.begin(); itor != m_StatusEffectInstanceList.end(); ++itor)
	{
		pkInstance = (*itor);
		if(pkInstance->GetStatusEffect() && pkInstance->GetStatusEffect()->GetID() == strStatusEffectXMLID)
		{
			return	true;
		}
	}

	return	false;
}

bool	PgActor::CheckStatusEffectTypeExist(BYTE byType)
{ //액터가 버프/디버프..등등의 이펙트의 걸려있나를 검사
	StatusEffectInstanceList kContEffectList = GetStatusEffectInstanceList();
	StatusEffectInstanceList::const_iterator iter = kContEffectList.begin();
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	for( ; iter != kContEffectList.end(); ++iter )
	{
		CEffectDef const* pkEffectDef = kEffectDefMgr.GetDef((*iter)->GetEffectID());
		if(pkEffectDef && pkEffectDef->GetType() == 2)
		{
			return true;
		}
	}

	return false;
}

char const* PgActor::GetStartParamID(char const *kStr)
{
	if(!GetPilot())
	{
		return "";
	}

	PgStatusEffectInstance* pkInstance = NULL;
	for(StatusEffectInstanceList::iterator itor = m_StatusEffectInstanceList.begin(); itor != m_StatusEffectInstanceList.end(); ++itor)
	{
		pkInstance = (*itor);
		if( pkInstance )
		{
			if( 0 == strcmp(pkInstance->GetStatusEffect()->GetStartActionID().c_str(), kStr) )
			{
				return pkInstance->GetStatusEffect()->GetParamID().c_str();
			}
		}
	}
	return "";
}

int const PgActor::GetStartEffectSave(char const *kStr)
{
	if(!GetPilot())
	{
		return 0;
	}

	PgStatusEffectInstance* pkInstance = NULL;
	for(StatusEffectInstanceList::iterator itor = m_StatusEffectInstanceList.begin(); itor != m_StatusEffectInstanceList.end(); ++itor)
	{
		pkInstance = (*itor);
		if( pkInstance )
		{
			if( 0 == strcmp(pkInstance->GetStatusEffect()->GetStartActionID().c_str(), kStr) )
			{
				return pkInstance->GetStatusEffect()->GetEffectSave();
			}
		}
	}
	return 0;
}


std::string PgActor::GetStatusEffectParam(int const iEffectID, std::string const& kKey)const
{
	for(StatusEffectInstanceList::const_iterator c_it=m_StatusEffectInstanceList.begin(); c_it!=m_StatusEffectInstanceList.end(); ++c_it)
	{
		if((*c_it)->GetEffectID() == iEffectID)
		{
			return (*c_it)->GetParam(kKey);
		}
	}
	return "";
}

void PgActor::SetStatusEffectParam(int const iEffectID, std::string const& kKey, std::string const& kValue)
{
	for(StatusEffectInstanceList::const_iterator c_it=m_StatusEffectInstanceList.begin(); c_it!=m_StatusEffectInstanceList.end(); ++c_it)
	{
		if((*c_it)->GetEffectID() == iEffectID)
		{
			(*c_it)->SetParam(kKey, kValue);
			return;
		}
	}
}

StatusEffectInstanceList::iterator	PgActor::RemoveStatusEffectInstance(PgStatusEffectInstance const& kInstance)
{
	int iSlot = kInstance.GetInstanceID() * 1000000;
	m_kFixedParticleList.erase(iSlot);

	for(StatusEffectUpdateList::iterator it = m_StatusEffectInstanceListForUpdate.begin(); it != m_StatusEffectInstanceListForUpdate.end(); ++it)
	{
		PgStatusEffectManUtil::SEffectUpdateInfo & rkInfo = (*it);
		if(rkInfo.m_pkInstance->GetInstanceID() == kInstance.GetInstanceID())
		{
			m_StatusEffectInstanceListForUpdate.erase(it);
			break;
		}
	}

	PgStatusEffectInstance* pkInstance = NULL;
	for(StatusEffectInstanceList::iterator itor = m_StatusEffectInstanceList.begin(); itor != m_StatusEffectInstanceList.end(); ++itor)
	{
		pkInstance  = (*itor);
		if(pkInstance->GetInstanceID() == kInstance.GetInstanceID())
		{
			if ( pkInstance->GetStatusEffect()->GetScriptName().size() )
			{
				std::string strScriptName = pkInstance->GetStatusEffect()->GetScriptName();
				strScriptName += "_End";
				lua_tinker::call<void,lwActor,int>( strScriptName.c_str(), lwActor(this), pkInstance->GetEffectID(), pkInstance->GetEffectKey() );
			}

			delete pkInstance;
			return m_StatusEffectInstanceList.erase(itor);
		}
	}

	return m_StatusEffectInstanceList.begin();
}

bool PgActor::CheckEffectExist(int const iEffectNo, bool const bInGroup)
{
	if(PgPilot* pkPilot = GetPilot())
	{
		if(CUnit* pkUnit = pkPilot->GetUnit())
		{
			if(CEffect* pkEffect = pkUnit->GetEffect(iEffectNo, bInGroup))
			{
				return true;
			}
		}
	}

	return false;
}

int PgActor::CheckSkillExist(int const iSkillNo)
{
	//iSkillNo 베이스 스킬 번호

	//return 값은 스킬을 배웠을 경우 배운 스킬 레벨

	if(PgPilot* pkPilot = GetPilot())
	{
		if(CUnit* pkUnit = pkPilot->GetUnit())
		{
			BM::GUID const& kParentGUID = pkUnit->Caller();
			if(BM::GUID::IsNotNull(kParentGUID) && pkUnit->GetID() != kParentGUID)
			{
				if(PgActor* pkParentActor = g_kPilotMan.FindActor(kParentGUID))
				{
					if(PgPilot* pkParentPilot = pkParentActor->GetPilot())
					{
						if(CUnit* pParentkUnit = pkParentPilot->GetUnit())
						{
							pkUnit = pParentkUnit;
						}
					}
				}
			}

			if(PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit))
			{
				PgMySkill* pkSkill = pkPlayer->GetMySkill();
				int const iLearnedSkill = pkSkill->GetLearnedSkill(iSkillNo);
				if(0 < iLearnedSkill)
				{
					return iLearnedSkill;
				}
			}
		}
	}

	return 0;
}

//! 변신 하기 전의 원래 액터를 설정한다.
void	PgActor::SetOriginalActorGUID(BM::GUID guid)
{
	m_OriginalActorGUID = guid;
}

BM::GUID	PgActor::GetOriginalActorGUID()
{
	return m_OriginalActorGUID;
}


/*	// 매달리는 로직
bool PgActor::HangItOn()
{
	bool bRet = false;
	
	if(m_bSide && m_pkWorld->GetPhysXRoot() != 0)
	{
		PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.HangItOn"), g_pkApp->GetFrameCount()));
		NiPick kPick;
		kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
		kPick.SetPickType(NiPick::FIND_FIRST);
		kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
		kPick.SetTarget(m_pkWorld->GetPhysXRoot());

		//_PgOutputDebugString("=========================\n");
		//_PgOutputDebugString("Side : OK\t");

		// 단의 경사도가 36도 이하면 매달릴 수 있다.
#ifdef PG_USE_CAPSULE_CONTROLLER
		NiPoint3 kOrigin = GetTranslate() + NiPoint3::UNIT_Z * (m_pkController->getHeight() * 0.5f);
		float const fRadius = m_pkController->getRadius();
#else
		NiPoint3 kOrigin = GetTranslate() + NiPoint3::UNIT_Z * (PG_CHARACTER_CAPSULE_HEIGHT * 0.5f);
		float const fRadius = PG_CHARACTER_CAPSULE_RADIUS;
#endif
		float const fRadian = NI_HALF_PI * 0.4f;	
		float const fHangRange = fRadius * 0.4f;
		float const fMinDist = (1.0f/NiATan(fRadian) * fHangRange) + fRadius + 0.5f;

		if(kPick.PickObjects(kOrigin, m_kMovingDir))
		{
			//_PgOutputDebugString("HeadPick : OK\t");
			NiPick::Results &kResults = kPick.GetResults();
			NiPick::Record* pkRecord = kResults.GetAt(0);

			float kDist = pkRecord->GetDistance();
			//_PgOutputDebugString("Dist : %.4f\t", kDist);
			if(kDist < fRadius + 0.5f)
			{
				if(kPick.PickObjects(kOrigin + NiPoint3::UNIT_Z * (fRadius + 30.0f), m_kMovingDir))
				{
					NiPick::Results &kResults = kPick.GetResults();
					NiPick::Record* pkRecord = kResults.GetAt(0);
					float kDist = pkRecord->GetDistance();
			
					//_PgOutputDebugString("HandPick : OK\t");
					//_PgOutputDebugString("Dist : %.4f\t", kDist);

					if(kDist > fMinDist)
					{
						bRet = true;
					}
				}
				else
				{
					//_PgOutputDebugString("HandPick : OK\t");
					//_PgOutputDebugString("Dist : Infinite\t");
					bRet = true;
				}
			}
		}
		//_PgOutputDebugString("\n=========================\n");
	}

	return bRet;
}

*/

bool const PgActor::IsInCoolTime(unsigned long ulSkillNo, bool& rbIsGobalCoolTime) const
{
	PgPilot* pkPilot = GetPilot();
	if(pkPilot)
	{
		CUnit* pkUnit = pkPilot->GetUnit();
		if(pkUnit
			&& pkUnit->IsUnitType(UT_SUB_PLAYER)
			)
		{// 보조 캐릭터는
			PgActor* pkCallerActor = g_kPilotMan.FindActor( pkUnit->Caller() );
			if(pkCallerActor)
			{// 메인 캐릭터(Caller)와 쿨타임을 공유 하고
				return pkCallerActor->IsInCoolTime(ulSkillNo, rbIsGobalCoolTime);
			}
		}
	}
	
	rbIsGobalCoolTime = false;

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.IsInCoolTime"), g_pkApp->GetFrameCount()));
	stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.find(g_kSkillTree.GetKeySkillNo(ulSkillNo));
	if(itor != m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end())
	{
		return true;
	}

	// 글로벌 쿨타임 무시 어빌이 있을 경우
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);
	if(pkSkillDef) 
	{
		if(EST_GENERAL == pkSkillDef->GetAbil(AT_TYPE))
		{ //일반 액션용 스킬이라면 쿨타임 체크를 안해야 한다.
			return false;
		}
		if(0 < pkSkillDef->GetAbil(AT_IGNORE_GLOBAL_COOLTIME))
		{
			return false;
		}
	}

	// 글로벌 쿨타임을 한번더 체크 한다.
	itor = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.find(SKILL_NO_GLOBAL_COOLTIME);
	if(itor != m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end())
	{
		rbIsGobalCoolTime = true;
		return true;
	}
	
	return	false;
}

void PgActor::StartSkillCoolTime(unsigned long const ulSkillNo)
{	
//	if(0 < pkSkillDef->GetAbil(AT_SUB_PLAYER_ACTION))
	{// 격투가 보조캐릭터가 쓰는 스킬이면
		CUnit* pUnit = GetUnit();
		if(pUnit
			&& pUnit->IsUnitType(UT_SUB_PLAYER)
			)
		{
			PgActor* pCaller = g_kPilotMan.FindActor(pUnit->Caller());
			if(pCaller)
			{// 본캐릭에 쿨타임 적용시켜준다
				pCaller->StartSkillCoolTime(ulSkillNo);
			}
		}
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.StartSkillCoolTime"), g_pkApp->GetFrameCount()));
	stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor	 = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.find(g_kSkillTree.GetKeySkillNo(ulSkillNo));
	if(m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end() != itor)
	{
		itor->second.m_ulCoolStartTime = BM::GetTime32();
		return;
	}
	
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);
	if(!pkSkillDef) 
	{
		return;
	}

	int	iCoolTime = pkSkillDef->GetAbil(ATS_COOL_TIME);
	int	iAddCoolTime = 0;

	// 쿨타임이 있는 스킬만 추가 쿨타임을 적용
	if(0 < iCoolTime)
	{
		if(m_pkPilot)
		{
			if(m_pkPilot->GetUnit())
			{
				int const iCoolTimeRate = m_pkPilot->GetUnit()->GetAbil(AT_R_COOLTIME_RATE_SKILL);
				int const iDiffCoolTime = static_cast<int>(iCoolTime * static_cast<double>(iCoolTimeRate) / static_cast<double>(ABILITY_RATE_VALUE));
				iCoolTime = std::max<int>(0,iCoolTime - iDiffCoolTime);
				if(m_pkPilot->GetUnit()->GetSkill())
				{
					iAddCoolTime = m_pkPilot->GetUnit()->GetSkill()->GetSkillCoolTime();
				}
			}
		}

		iCoolTime += iAddCoolTime;
	}

	//모든 스킬을 사용시 글로벌 쿨타임을 추가 한다.
	int iGobalCoolTime = pkSkillDef->GetAbil(AT_GLOBAL_COOLTIME);
	if(0 < iGobalCoolTime)
	{
		m_SkillCoolTimeInfo.m_CoolTimeInfoMap.insert(std::make_pair(SKILL_NO_GLOBAL_COOLTIME, stSkillCoolTimeInfo::stCoolTimeInfoNode(SKILL_NO_GLOBAL_COOLTIME,BM::GetTime32(), iGobalCoolTime)));
	}	
	
	//쿨 다운 타임이 0보다 작아지면 쿨타임을 돌리지 않는다. 0이라는 의미 이다.
	if(0 >= iCoolTime)
	{
		return;
	} 

	m_SkillCoolTimeInfo.m_CoolTimeInfoMap.insert(std::make_pair(g_kSkillTree.GetKeySkillNo(ulSkillNo), stSkillCoolTimeInfo::stCoolTimeInfoNode(ulSkillNo,BM::GetTime32(),iCoolTime)));

	for(int i = AT_JOINT_COOLTIME_SKILL_NO_1; i < AT_JOINT_COOLTIME_SKILL_NO_MAX; ++i)
	{// 쿨타임을 공유하는 스킬이 존재 하면
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(GetPilot()->GetUnit());
		if(!pkPlayer)
		{
			return;
		}
		PgMySkill* pkMySkill = pkPlayer->GetMySkill();
		if(!pkMySkill)
		{
			return;
		}

		int const iJointCoolTimeSkillNo = pkMySkill->GetLearnedSkill(pkSkillDef->GetAbil(i) , true);
		if(0 < iJointCoolTimeSkillNo)
		{// 해당 스킬을 쿨타임 적용 시킨다
			CSkillDef const* pkTempSkillDef = kSkillDefMgr.GetDef(iJointCoolTimeSkillNo);
			int const iJointCoolTime = pkTempSkillDef->GetAbil(AT_JOINT_COOLTIME);
			if(0 < iJointCoolTime)
			{
				m_SkillCoolTimeInfo.m_CoolTimeInfoMap.insert(std::make_pair(g_kSkillTree.GetKeySkillNo(iJointCoolTimeSkillNo), stSkillCoolTimeInfo::stCoolTimeInfoNode(ulSkillNo,BM::GetTime32(), iJointCoolTime)));
			}
			else
			{
				m_SkillCoolTimeInfo.m_CoolTimeInfoMap.insert(std::make_pair(g_kSkillTree.GetKeySkillNo(iJointCoolTimeSkillNo), stSkillCoolTimeInfo::stCoolTimeInfoNode(ulSkillNo,BM::GetTime32(),iCoolTime)));
			}
		}
	}
}

void PgActor::ReCalcCoolTime(int const iCoolTimeRate)
{
	unsigned long ulCurTime = BM::GetTime32();

	for(stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.begin();itor != m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end();++itor)
	{
		stSkillCoolTimeInfo::stCoolTimeInfoNode* pNode = &itor->second;

		if(ulCurTime > pNode->m_ulCoolStartTime)
		{
			unsigned long ulCoolTime = pNode->m_ulTotalCoolTime - (ulCurTime - pNode->m_ulCoolStartTime);
			unsigned long ulDiffCoolTime = static_cast<unsigned long>(ulCoolTime * static_cast<double>(iCoolTimeRate)/static_cast<double>(ABILITY_RATE_VALUE));
			pNode->m_ulCoolStartTime -= ulDiffCoolTime;
		}
	}
}

void PgActor::CutSkillCoolTime(unsigned long const ulSkillNo)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CutSkillCoolTime"), g_pkApp->GetFrameCount()));
	stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor	 = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.find(g_kSkillTree.GetKeySkillNo(ulSkillNo));
	if(m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end() != itor)
	{
		itor->second.m_ulSkillNo = 0;
		return;
	}
}

void PgActor::SkillCastingConfirmed(unsigned long const ulSkillNo, short const sErrorCode)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SkillCastingConfirmed"), g_pkApp->GetFrameCount()));
	bool	bIsSingleMode = 
#ifndef EXTERNAL_RELEASE
		g_pkApp->IsSingleMode();
#else
		false;
#endif

	if(m_pkAction && (m_pkAction->GetActionNo() == ulSkillNo || bIsSingleMode))
	{
		{// 캐스팅 타임 시간 조절
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);
			PG_ASSERT_LOG(pkSkillDef);
			if(!pkSkillDef) { return; }

			int iAddCastTime = 0;
			int iSkillCastTime = pkSkillDef->GetAbil(AT_CAST_TIME);
			int iDecCastTimeRate = 0;

			m_SkillCastingInfo.m_ulSkillNo = ulSkillNo;
			m_SkillCastingInfo.m_ulCastStartTime = BM::GetTime32();
			if(0 < iSkillCastTime)
			{// 캐스팅 타임이 있는 경우에만
				if(m_pkPilot)
				{
					CUnit* pkUnit = m_pkPilot->GetUnit();
					if(pkUnit)
					{// 값을 얻어와
						iDecCastTimeRate = pkUnit->GetAbil(AT_CAST_TIME_RATE);
						CSkill* pkSkill = pkUnit->GetSkill();
						if(pkSkill)
						{
							iAddCastTime = pkSkill->GetSkillCastingTime();
						}
					}
				}
				
			}
			// 계산하고
			m_SkillCastingInfo.m_ulTotalCastTime = lwCommonSkillUtilFunc::CalcTotalCastTime( iSkillCastTime, iAddCastTime, iDecCastTimeRate );
		}

		if(IsMyActor())
		{
			if(0 < m_SkillCastingInfo.m_ulTotalCastTime)
			{
				CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SKILL_CASTTIME_BAR"));
				if(!pkWnd)
				{
					pkWnd = XUIMgr.Call(_T("SKILL_CASTTIME_BAR"));
				}
				if(pkWnd)
				{
					pkWnd->Visible(true);
					CXUI_Wnd*		pkBGBar = pkWnd->GetControl(_T("BG_BAR"));
					CXUI_AniBar*	pkAniBar = (CXUI_AniBar*)pkBGBar->GetControl(_T("ANIBAR"));
					lwUIWnd	kAniBarWnd(pkAniBar);
					kAniBarWnd.SetStartTime(m_SkillCastingInfo.m_ulTotalCastTime);
				}
			}
		}
	}
}

void	PgActor::StartSkillCasting(unsigned long const ulSkillNo)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.StartSkillCasting"), g_pkApp->GetFrameCount()));
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);

	if(pkSkillDef && E_SCAST_CASTSHOT == pkSkillDef->GetAbil(AT_CASTTYPE))
	{
		lua_tinker::call<void,int>("Net_C_M_REQ_BEGINCAST",(int)ulSkillNo);
	}
}

void	PgActor::CutSkillCasting(unsigned long const ulSkillNo)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CutSkillCasting"), g_pkApp->GetFrameCount()));
	m_SkillCastingInfo.m_ulSkillNo = 0;
	if(IsMyActor())
	{
		CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SKILL_CASTTIME_BAR"));
		if(pkWnd)
		{
			pkWnd->Close();
		}
	}
}

void	PgActor::StartSkillToggle(unsigned long const ulSkillNo)
{
	m_SkillToggleInfo.insert(g_kSkillTree.GetKeySkillNo(ulSkillNo));
}

void	PgActor::CutSkillToggle(unsigned long const ulSkillNo)
{
	TOGGLE_SKILL_SET::iterator skill_it = m_SkillToggleInfo.find(g_kSkillTree.GetKeySkillNo(ulSkillNo));
	if (m_SkillToggleInfo.end() != skill_it)
	{
		m_SkillToggleInfo.erase(skill_it);
	}
}

void	PgActor::UpdateSkillInfos()
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.UpdateSkillInfos"), g_pkApp->GetFrameCount()));
	if(IsUnderMyControl() == false) { return; }

	unsigned	long	ulTime = BM::GetTime32();
	unsigned	long	ulElapsedTime = 0;
	if(0 < m_SkillCastingInfo.m_ulSkillNo)
	{
		ulElapsedTime = ulTime - m_SkillCastingInfo.m_ulCastStartTime;
		if(ulElapsedTime > m_SkillCastingInfo.m_ulTotalCastTime)
		{
			if(m_pkAction)
			{
				NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
				NIMETRICS_STARTTIMER(a);
				m_pkAction->OnCastingCompleted(this,m_pkAction);

				SetIgonreDamageEffect(m_pkAction);

				NIMETRICS_ENDTIMER(a);
			}
			m_SkillCastingInfo.m_ulSkillNo = 0;
		}
	}
	if(0 < m_SkillCoolTimeInfo.m_CoolTimeInfoMap.size())
	{
		for(stSkillCoolTimeInfo::CoolTimeInfoMap::iterator itor = m_SkillCoolTimeInfo.m_CoolTimeInfoMap.begin();
			itor != m_SkillCoolTimeInfo.m_CoolTimeInfoMap.end(); )
		{
			stSkillCoolTimeInfo::stCoolTimeInfoNode* pNode = &itor->second;

			if(ulTime>pNode->m_ulCoolStartTime)
			{
				ulElapsedTime = ulTime - pNode->m_ulCoolStartTime;
				if(ulElapsedTime > pNode->m_ulTotalCoolTime)
				{

					m_SkillCoolTimeInfo.m_CoolTimeInfoMap.erase(itor++);
					continue;
				}
			}

			++itor;
		}
	}
}
int	PgActor::GetPosChangeActionCount(const ActionQueue &kQueue)	//	큐에 들어있는 액션 중에, 캐릭터의 위치를 이동시키는 액션이 몇개나 있는가?
{
	int	iCount = 0;

	bool	bCurrentActionCanChangeActorPos = false;
	if(m_pkAction)
	{
		bCurrentActionCanChangeActorPos = m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS);
	}

	for(ActionQueue::const_iterator itor = kQueue.begin(); itor != kQueue.end(); ++itor)
	{
		PgActionEntity const& rkActionEntity = *itor;

		if(NULL == rkActionEntity.GetAction())
		{
			if(bCurrentActionCanChangeActorPos)	//	현재 액션이 이동액션이라면, 방향 전환 역시 이동관련 액션이라고 봐야한다.
			{
				++iCount;
			}
			continue;
		}

		PgAction *pkAction = rkActionEntity.GetAction();
		if(pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
		{
			++iCount;
		}
	}

	return	iCount;
}

bool	PgActor::ProcessFollowingActor()
{
	BM::CAutoMutex kLock(m_kActionQueueMutex);

	if(!g_pkWorld)
	{
		return true;
	}
	if(!IsUnderMyControl())
	{
		return true;
	}

	PgAction* pkCurAction = GetAction();

	if(stFollowInfo::FS_MOVE_TO_STARTPOS == m_kFollowInfo.GetFollowState())
	{
		if(ACTIONNAME_RUN != pkCurAction->GetID())
		{
			m_dwLastActionTime = 0;
			m_kFollowInfo.SetFollowState(stFollowInfo::FS_PROCESS_ACTION_QUEUE);
		}
		else
		{
			return	true;
		}
	}

	DWORD const dwNow = static_cast<int>(g_pkWorld->GetAccumTime()*1000.0);

	int const iMinActionQueueCount = 3;
	float const	fMaxWaitDistance = 100.0f;
	float const fMinFollowDistance = 50.0f;

	PgPilot* pkTargetPilot = g_kPilotMan.FindPilot(m_kFollowInfo.m_kFollowTargetActor);
	PgActor* pkTargetActor = NULL;

	if(pkTargetPilot)
	{
		pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	}

	if(!pkTargetActor)
	{
		if(m_kFollowInfo.IsTargetLost())
		{
			float	fElapsedTime = g_pkWorld->GetAccumTime() - m_kFollowInfo.GetTargetLostStartTime();
			if(30 < fElapsedTime)	//	30초 이상 타겟을 잃었다면, 따라다니기를 중지한다.
			{
				RequestFollowActor(GetFollowingTargetGUID(),EFollow_Cancel);
				return	true;
			}
		}
		else
		{
			//	타겟을 잃었다.
			m_kFollowInfo.SetTargetLostStartTime(g_pkWorld->GetAccumTime());
			m_kFollowInfo.SetTargetLost(true);

			return	true;
		}

		if(0 == m_kActionQueue.size())
		{
			SetDirection(DIR_NONE);
		}
	}
	else
	{
		if(m_kFollowInfo.IsTargetLost())
		{
			RequestFollowActor(pkTargetActor->GetPilotGuid(),EFollow_Request);
			return	true;
		}

	}


	float fCurrentDistance = fMaxWaitDistance;
	
	if(pkTargetActor)
	{
		fCurrentDistance = (pkTargetActor->GetPos()-GetPos()).Length();
	}

	if(pkCurAction)
	{
		if(ACTIONNAME_RUN == pkCurAction->GetID() && IsMeetFloor())
		{
			if(stFollowInfo::FS_WAIT != m_kFollowInfo.GetFollowState() &&
				fCurrentDistance < fMinFollowDistance && 
				GetPosChangeActionCount(m_kActionQueue) < iMinActionQueueCount)
			{
				//	일정 거리 이하로 가깝다면, 강제로 Idle 모션을 하도록한다.
				PgAction* pkAction = CreateActionForTransitAction(ACTIONNAME_IDLE);
				pkAction->SetActionStartPos(GetPosition());

				int	iActionTerm = dwNow - static_cast<int>( pkCurAction->GetActionEnterTime()*1000.0f );
				pkAction->SetActionTerm(iActionTerm);

				m_kFollowInfo.SaveDirection((Direction)GetDirection());
				m_kFollowInfo.SaveActionTerm(iActionTerm);
				m_kFollowInfo.SetFollowState(stFollowInfo::FS_WAIT);

				PgActionEntity	kActionEntity(pkAction,DIR_NONE);
				m_kActionQueue.push_front(kActionEntity);
			}
		}
		else if(stFollowInfo::FS_WAIT == m_kFollowInfo.GetFollowState())
		{
			if(fCurrentDistance < fMaxWaitDistance && GetPosChangeActionCount(m_kActionQueue) < iMinActionQueueCount)
			{
				return	true;
			}
			//	일정 거리 이상이라면, 다시 달려간다.
			PgAction* pkAction = CreateActionForTransitAction(ACTIONNAME_RUN);
			pkAction->SetActionStartPos(GetPosition());

			int	iActionTerm = dwNow - static_cast<int>( pkCurAction->GetActionEnterTime()*1000.0f );
			pkAction->SetActionTerm(iActionTerm);

			Direction kDirection = m_kFollowInfo.GetSavedDirection();
			pkAction->SetDirection(kDirection);
			SetDirection(kDirection);

			m_kFollowInfo.SetFollowState(stFollowInfo::FS_PROCESS_ACTION_QUEUE);

			PgActionEntity	kActionEntity(pkAction,DIR_NONE);
			m_kActionQueue.push_front(kActionEntity);
		}
	}

	// 새로온 패킷의 속도가, 이전 패킷의 속도보다 빠를 때만 그만큼 시간을 늦게 두어서 패킷을 처리함.

	bool bReturn = true;
	ActionQueue::iterator itr = m_kActionQueue.begin();
	while(itr != m_kActionQueue.end())
	{
		PgActionEntity& rkActionEntity = *itr;

		PgAction* pkAction = rkActionEntity.GetAction();

		if(dwNow <= m_dwLastActionTime)
		{
			// LastActionTime이 수정되면, 여기에 걸릴 수 있으나, 이제 수정될 일이 없음.
			//WriteToConsole("Now[%u] <= LastAction[%u]\n", dwNow, m_dwLastActionTime);
			return true;
		}

		DWORD dwActionTerm = rkActionEntity.GetActionTerm();

		if(ACTIONNAME_RUN == m_pkAction->GetID())
		{
			if(0 < m_kFollowInfo.GetSavedActionTerm())
			{
				int	iAdjustedActionTerm = rkActionEntity.GetActionTerm()-m_kFollowInfo.GetSavedActionTerm();

				if(pkAction)
				{
					if(stFollowInfo::FS_WAIT != m_kFollowInfo.GetFollowState())
					{
						m_kFollowInfo.SetSavedActionTerm(0);
						if(0 > iAdjustedActionTerm)
						{
							m_kFollowInfo.SaveActionTerm(-iAdjustedActionTerm);
							iAdjustedActionTerm = 0;
						}

						dwActionTerm = iAdjustedActionTerm;
						pkAction->SetActionTerm(iAdjustedActionTerm);

					}
				}
				else
				{
					if(0 > iAdjustedActionTerm)
					{
						iAdjustedActionTerm = 0;
					}

					dwActionTerm = iAdjustedActionTerm;
				}
			}
		}
		
		//	현재 액션이 이동하지 않는 액션이라면, 기다릴 필요 없이 바로 다음 액션으로 넘어간다.
		if(false == m_pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
		{
			m_dwLastActionTime = dwNow - dwActionTerm;
		}

		DWORD dwElapsedTime = (m_dwLastActionTime != 0 ? dwNow - m_dwLastActionTime : dwActionTerm);

		bool bForceToProcessAction = false;
		if(dwElapsedTime < dwActionTerm)
		{
			return true;
		}

		DWORD dwSyncTime = 0;
		DWORD dwOverTime = dwElapsedTime - dwActionTerm;

		if(pkAction)
		{
			if(NiPoint3::ZERO != pkAction->GetActionStartPos())
			{
				SetPosition(pkAction->GetActionStartPos());
			}
			pkAction->AlreadySync(true);
			ProcessAction(pkAction,false);

			bReturn = false;
			m_dwLastActionTime = dwNow;
		}
		else
		{
			SetDirection(rkActionEntity.GetDirection());

			if(NiPoint3::ZERO != rkActionEntity.GetDirectionStartPos())
			{
				SetPosition(rkActionEntity.GetDirectionStartPos());
			}

			if(IsMyActor() || (IsUnderMyControl() && !IsNowFollowing()))
			{
				g_kPilotMan.BroadcastDirection(m_pkPilot, rkActionEntity.GetDirection());
			}
		}

		if(0 == m_kActionQueue.size())
		{
			break;
		}

		itr = m_kActionQueue.erase(itr);
	}

	return bReturn;

}
void	PgActor::RequestFollowActor(BM::GUID const& kTargetActorGUID, EPlayer_Follow_Mode const kMode, bool const bForce)	//	kTargetActorGUID를 따라갈것을 서버에 요청한다
{
	if(!bForce)
	{
		if( (EFollow_Request == kMode) || (EFollow_TakePerson == kMode) )
		{
			if(!CheckCanFollow(kTargetActorGUID))
			{
				return;
			}
		}
		if( (EFollow_Cancel == kMode) || (EFollow_CancelTakePerson == kMode) )
		{
			if(false == IsNowFollowing())
			{
				return;
			}
		}
	}

	lua_tinker::call<void,lwGUID,BYTE>("Net_PT_C_M_REQ_FOLLOWING",lwGUID(kTargetActorGUID),((BYTE)kMode));
}

void	PgActor::ResponseFollowActor(BM::GUID const& kTargetActorGUID, EPlayer_Follow_Mode const kMode)	//	kTargetActorGUID를 따라갈것을 서버에 요청한다.
{

	if(kMode == EFollow_Request)
	{
		if(!FollowActor(kTargetActorGUID))
		{
			if(IsMyActor())
			{
				RequestFollowActor(kTargetActorGUID,EFollow_Cancel,true);
			}
		}
	}
	else if(kMode == EFollow_Cancel)
	{
		StopFollowActor();
		m_kFollowInfo.RemoveFollowingMeActor(kTargetActorGUID);
	}
}

bool	PgActor::CheckCanFollow(const	BM::GUID &kTargetActorGUID, bool const bMsg)	//	kTargetActorGUID 를 따라갈 수 있는지 체크하고 메세지를 보여준다.
{

	PgPilot	*pkTargetPilot = g_kPilotMan.FindPilot(kTargetActorGUID);
	if(!pkTargetPilot)
	{
		return	false;
	}
	PgPilot	*pkPilot = GetPilot();
	if(!pkPilot)
	{
		return	false;
	}

	PgPlayer	*pkMyPlayer = (PgPlayer*)pkPilot->GetUnit();
	PgPlayer	*pkTargetPlayer = (PgPlayer*)pkTargetPilot->GetUnit();

	if(!pkMyPlayer || !pkTargetPlayer)
	{
		return	false;
	}

	PgActor	*pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return	false;
	}

	if( pkMyPlayer->GetAbil(AT_BEAR_EFFECT_SPEED_LOCK) > 0 )
	{// 이속 변화 금지 어빌이 걸려 있을 때는 따라가기 금지
		return false;
	}

	//	파티원,친구,길드원이어야만 따라갈 수 있다.
	bool	bHasCorrectRelationship = false;
	if(pkMyPlayer->GuildGuid() != BM::GUID::NullData())
	{
		if(pkMyPlayer->GuildGuid() == pkTargetPlayer->GuildGuid())
		{
			bHasCorrectRelationship = true;	//	같은 길드다.
		}
	}
	if(!bHasCorrectRelationship)
	{
		if(pkMyPlayer->PartyGuid() != BM::GUID::NullData())
		{
			if(pkMyPlayer->PartyGuid() == pkTargetPlayer->PartyGuid())
			{
				bHasCorrectRelationship = true;	//	같은 파티다.
			}
		}
	}
	if(!bHasCorrectRelationship)
	{
		SFriendItem kFriendItem;
		if(g_kFriendMgr.Friend_Find_ByGuid(pkTargetPlayer->GetID(),kFriendItem))
		{
			// 양측 모두 친구로 등록 되어 있어야 가능하다.
			if( kFriendItem.ChatStatus() & FCS_ADD_ALLOW )
			{
				bHasCorrectRelationship = true;
			}
		}
	}
	if(!bHasCorrectRelationship)
	{
		if(pkMyPlayer->CoupleGuid() != BM::GUID::NullData())
		{
			if(pkMyPlayer->CoupleGuid() == pkTargetPlayer->GetID())
			{
				bHasCorrectRelationship = true;	//	커플
			}
		}
	}
	if(!bHasCorrectRelationship)
	{
		//	파티원,친구,길드원이어야만 따라갈 수 있다.
		bMsg ? lwAddWarnDataStr(lwWString(TTW(418)), 2) : 0;
		return	false;
	}

	//	거리가 일정거리 이하여야 한다.
	float	fMaxDistance = 100;
	float	fDistance = (pkTargetActor->GetPos() - GetPos()).Length();
	if(fMaxDistance<fDistance)
	{
		//	거리가 일정거리 이하여야 한다.
		bMsg ? lwAddWarnDataStr(lwWString(TTW(404)), 2) : 0;
		return	false;
	}

	if(IsRidingPet() || pkTargetActor->IsRidingPet())
	{ //대상이나 내가 펫이 탑승 중일 때는 따라가기 불가
		bMsg ? lwAddWarnDataStr(lwWString(TTW(451004)), 2) : 0;
		return false;
	}

	//	대상이 이동중 일때는 따라가기를 할 수 없다.

	PgAction *pkTargetAction = pkTargetActor->GetAction();
	if(!pkTargetAction || pkTargetAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
	{
		//	대상이 이동중 일때는 따라가기를 할 수 없다.
		bMsg ? lwAddWarnDataStr(lwWString(TTW(419)), 2) : 0;
		return	false;
	}

	return	true;
}

bool PgActor::FollowActor(const	BM::GUID &kTargetActorGUID)	//	kTargetActorGUID 를 따라가도록 한다.
{


	if(GetPilotGuid() == kTargetActorGUID)	//	내가 나를 쫒아가??? No way~
	{
		return	false;
	}

	//	나의 현재 액션이 위치 이동하지 않는 액션이어야 한다.
	PgAction	*pkAction = GetAction();
	if(IsUnderMyControl())
	{
		if(!pkAction || pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
		{
			return false;
		}
	}

	if(IsUnderMyControl())
	{
		if(m_kFollowInfo.m_kFollowTargetActor != kTargetActorGUID)
		{
			StopFollowActor();	//	기존에 쫒던 사람은 쫒지 않도록 한다.
		}
	}

	PgPilot	*pkMyPilot = GetPilot();
	if(!pkMyPilot)
	{
		return false;
	}

	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kTargetActorGUID);
	if(!pkPilot)
	{
		return false;
	}

	PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(!pkActor)
	{
		return false;
	}

	//	따라갈 대상의 현재 액션이 위치 이동하지 않는 액션이어야 한다.
	if(IsUnderMyControl())
	{
		pkAction = pkActor->GetAction();
		if(!pkAction || pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS))
		{
			lwAddWarnDataStr(lwWString(TTW(419)), 2);
			return false;
		}

		ClearActionQueue();

		m_kFollowInfo.SetStartFollow(kTargetActorGUID);

		pkActor->AddFollowingMeActor(GetPilotGuid());

		g_kSafeFoamMgr.SetFollow(kTargetActorGUID);
	}
	else
	{
		m_kFollowInfo.AddFollowingMeActor(kTargetActorGUID);
	}

	if(IsMyActor())	//	내가 pkActor 를 따라가기로 설정
	{
		WCHAR	kMessage[256];
		swprintf_s(kMessage,256,TTW(401).c_str(),pkPilot->GetName().c_str());
		lwAddWarnDataStr(lwWString(std::wstring(kMessage)), 2);
	}
	else if(pkActor->IsMyActor() && IsUnderMyControl() == false)	//	this actor 가 나를 따라가기로 설정
	{
		WCHAR	kMessage[256];
		swprintf_s(kMessage,256,TTW(402).c_str(),pkMyPilot->GetName().c_str());
		lwAddWarnDataStr(lwWString(std::wstring(kMessage)), 2);
	}

	return	true;

}
void PgActor::StopFollowActor()
{

	if(IsUnderMyControl())
	{
		if(IsNowFollowing() == false)
		{
			return;
		}
	}

	BM::GUID	kTargetGUID = m_kFollowInfo.m_kFollowTargetActor;

	if(IsUnderMyControl())
	{
		m_kFollowInfo.SetStopFollow();

		ClearActionQueue();
	}

	PgPilot	*pkMyPilot = GetPilot();
	if(!pkMyPilot)
	{
		return;
	}	

	if(IsMyActor())
	{
		lwAddWarnDataStr(lwWString(TTW(417)), 2);

		g_kSafeFoamMgr.RemoveFollow();
	}
	else
	{
		WCHAR	kMessage[256];
		swprintf_s(kMessage,256,TTW(414).c_str(),pkMyPilot->GetName().c_str());
		lwAddWarnDataStr(lwWString(std::wstring(kMessage)), 2);
	}

	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kTargetGUID);
	if(!pkPilot)
	{
		return;
	}

	PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	if(IsUnderMyControl())
	{
		pkActor->RemoveFollowingMeActor(GetPilotGuid());		
	}

	/*if(IsMyActor())
	{
		lwAddWarnDataStr(lwWString(TTW(417)), 2);

		g_kSafeFoamMgr.RemoveFollow();
	}
	else if(pkActor->IsMyActor() && IsUnderMyControl() == false)
	{
		WCHAR	kMessage[256];
		swprintf_s(kMessage,256,TTW(414).c_str(),pkMyPilot->GetName().c_str());
		lwAddWarnDataStr(lwWString(std::wstring(kMessage)), 2);
	}*/

	InvalidateDirection();
	ReserveTransitAction(ACTIONNAME_IDLE);
}
void	PgActor::AddFollowingMeActor(const	BM::GUID &kActorGUID)	//	나를 따라오는 액터를 추가한다. FollowActor() 에서 자동으로 호출된다.
{
	m_kFollowInfo.AddFollowingMeActor(kActorGUID);
}
bool	PgActor::FindFollowingMeActor(const	BM::GUID &kActorGUID)
{
	return m_kFollowInfo.FindFollowingMeActor(kActorGUID);
}
void	PgActor::RemoveFollowingMeActor(const	BM::GUID &kActorGUID)	//	나를 따라오는 액터를 제거한다. StopFollowActor() 에서 자동으로 호출된다.
{
	m_kFollowInfo.RemoveFollowingMeActor(kActorGUID);
}
void	PgActor::SetFollowTargetActor(const	BM::GUID &kActorGUID)
{
	m_kFollowInfo.SetStartFollow(kActorGUID);
}
void PgActor::SeeLadder()
{
	if(GetWorld()->GetLadderRoot() == 0)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ClimbUpLadder"), g_pkApp->GetFrameCount()));
	NiPick kPick;
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetTarget(GetWorld()->GetLadderRoot());
	kPick.SetObserveAppCullFlag(false);
	kPick.ClearResultsArray();

	NiPoint3	kOrigin = GetTranslate() + NiPoint3::UNIT_Z * 9.0f;
	NiPoint3 kOriginPt[3];
	//	먼저 자기가 보고 있는 방향에 사다리가 있는지 체크한다.
	NiPoint3	kDelta;
	kOriginPt[0] = kOrigin; 

	NiPoint3	kLookDir = GetLookingDir();

#ifdef PG_USE_CAPSULE_CONTROLLER
	kDelta = kLookDir.UnitCross(NiPoint3::UNIT_Z) * m_pkController->getRadius() * 0.5;
#else
	kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * PG_CHARACTER_CAPSULE_RADIUS * 0.5;
#endif	
	kOriginPt[1] = kOrigin - kDelta;
	kOriginPt[2] = kOrigin + kDelta;
	bool bLadderFound = true;
	for(int iIndex = 0; iIndex < 3; ++iIndex)
	{
		if(!kPick.PickObjects(kOriginPt[iIndex], kLookDir))
		{
			bLadderFound = false;
			break;
		}
	}	
	if(!bLadderFound)	//	자기 앞쪽에 없으면, 카메라 정면쪽에 있는지 다시 한번 체크
	{
		kLookDir = m_kPathNormal;

		kPick.ClearResultsArray();

		// 캐릭터 위치에서 Path의 Normal방향으로 레이를 준비.
#ifdef PG_USE_CAPSULE_CONTROLLER
		kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * m_pkController->getRadius() * 0.5;
#else
		kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * PG_CHARACTER_CAPSULE_RADIUS * 0.5;
#endif

		kOriginPt[1] = kOrigin - kDelta;
		kOriginPt[2] = kOrigin + kDelta;

		// 캐릭터의 가로 너비를 고려하여, 3번의 레이를 쏜다.
		for(int iIndex = 0; iIndex < 3; ++iIndex)
		{
			if(!kPick.PickObjects(kOriginPt[iIndex], m_kPathNormal))
			{
				return;
			}
		}

		SetLookingDirection(DIR_UP, true);
	}

	NiPick::Results &rkResults = kPick.GetResults();
	if(rkResults.GetSize() == 0)
	{
		return;
	}

	float	fDistance = rkResults.GetAt(0)->GetDistance();

	if(fDistance>10.0f || fDistance<8.0f)
	{
		float	fMoveDistance = fDistance - 10;
		NiPoint3	kMoveDir = kLookDir;
		NiPoint3	kCurPos = GetPos();
		NiPoint3	kNewPos = kCurPos + kMoveDir*fMoveDistance;
		SetPosition(kNewPos);
	}

}
bool PgActor::ClimbUpLadder()
{
	if(GetWorld()->GetLadderRoot() == 0)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ClimbUpLadder"), g_pkApp->GetFrameCount()));
	NiPick kPick;
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetTarget(GetWorld()->GetLadderRoot());
	kPick.SetObserveAppCullFlag(false);
	kPick.ClearResultsArray();

	NiPoint3	kOrigin = GetTranslate() + NiPoint3::UNIT_Z * 9.0f;
	NiPoint3 kOriginPt[3];
	//	먼저 자기가 보고 있는 방향에 사다리가 있는지 체크한다.
	NiPoint3	kDelta;
	kOriginPt[0] = kOrigin; 

	NiPoint3	kLookDir = GetLookingDir();

#ifdef PG_USE_CAPSULE_CONTROLLER
	kDelta = kLookDir.UnitCross(NiPoint3::UNIT_Z) * m_pkController->getRadius() * 0.5;
#else
	kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * PG_CHARACTER_CAPSULE_RADIUS * 0.5;
#endif	
	kOriginPt[1] = kOrigin - kDelta;
	kOriginPt[2] = kOrigin + kDelta;
	bool	bLadderFound = true;
	for(int iIndex = 0; iIndex < 3; ++iIndex)
	{
		if(!kPick.PickObjects(kOriginPt[iIndex], kLookDir))
		{
			bLadderFound = false;
			break;
		}
	}	
	if(!bLadderFound)	//	자기 앞쪽에 없으면, 카메라 정면쪽에 있는지 다시 한번 체크
	{
		kPick.ClearResultsArray();

		// 캐릭터 위치에서 Path의 Normal방향으로 레이를 준비.
#ifdef PG_USE_CAPSULE_CONTROLLER
		kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * m_pkController->getRadius() * 0.5;
#else
		kDelta = GetPathNormal().UnitCross(NiPoint3::UNIT_Z) * PG_CHARACTER_CAPSULE_RADIUS * 0.5;
#endif

		kOriginPt[1] = kOrigin - kDelta;
		kOriginPt[2] = kOrigin + kDelta;

		// 캐릭터의 가로 너비를 고려하여, 3번의 레이를 쏜다.
		for(int iIndex = 0; iIndex < 3; ++iIndex)
		{
			if(!kPick.PickObjects(kOriginPt[iIndex], m_kPathNormal))
			{
				return false;
			}
		}
	}

	NiPick::Results &rkResults = kPick.GetResults();
	if(rkResults.GetSize() == 0)
	{
		return false;
	}

	static float const fLadderMinDist = 30.0f;
	if(rkResults.GetAt(0)->GetDistance() >= fLadderMinDist)
	{
		return false;
	}

	return true;
}

/*	// 줄타는 로직 

bool PgActor::HangOnRope()
{
	if(m_pkWorld->GetRopeRoot() == 0)
	{
		return false;
	}

	bool bRet = true;

	NiPick kPick;
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetTarget(m_pkWorld->GetRopeRoot());

	if(!kPick.PickObjects(GetTranslate() + NiPoint3::UNIT_Z * 12.0f, GetPathNormal()))
	{
		bRet = false;
	}

	return bRet;
}

*/

//void PgActor::SwitchPhysical(bool bGoPhysical, float fAccumTime, float fFrameTime)
//{
//	PG_STAT(PgStatTimerF timerA((FrameStat*)&g_kActorStatGroup.GetStatInfo("PgActor.SwitchPhysical"), g_pkApp->GetFrameCount()));
//	if(m_pkPhysXScene->GetInSimFetch())
//	{
//		return;
//	}
//
//	NiPhysXManager::GetPhysXManager()->WaitSDKLock();
////	m_pkPhysXSrc->SetActive(bGoPhysical);
//	m_pkPhysXDest->SetActive(!bGoPhysical);
//	m_pkPhysXDest->SetInterpolate(bGoPhysical);
//	NiPhysXManager::GetPhysXManager()->ReleaseSDKLock();
//
//	if(!m_pkPhysXActor)
//	{
//		return;
//	}
//
//	NxShape *pkShape = m_pkPhysXActor->getShapes()[0];
//
//	//m_bPhysical = bGoPhysical;
//
//	if(bGoPhysical)
//	{
////		pkShape->setLocalPosition(NxVec3(0, 0, m_pkController->getHeight()*0.5f + m_pkController->getRadius()));
////		m_pkController->getActor()->setCMassOffsetLocalPosition(NxVec3(0, 0, m_pkController->getHeight()*0.5f + m_pkController->getRadius()));
//	}
//	else
//	{
////		pkShape->setLocalPosition(NxVec3(0, 0, 0)); 
////		m_pkController->getActor()->setCMassOffsetLocalPosition(NxVec3(0, 0, 0));
////		NiPoint3 kPos = GetObjectByName("char_root NonAccum")->GetWorldTranslate();
////		m_pkController->getActor()->moveGlobalPosition(NxVec3(kPos.x, kPos.y, kPos.z + m_pkController->getHeight()*0.5f + m_pkController->getRadius()));
////		m_pkPhysXScene->UpdateDestinations(fAccumTime);
////		m_pkController->setPosition(NxExtendedVec3(kPos.x, kPos.y, kPos.z + m_pkController->getRadius() + m_pkController->getHeight() * 0.5f));
////		m_pkController->getActor()->setGlobalPosition(NxVec3(kPos.x, kPos.y, kPos.z + m_pkController->getRadius() + m_pkController->getHeight() * 0.5f));
////		m_pkController->getActor()->moveGlobalPosition(NxVec3(kPos.x, kPos.y, kPos.z + m_pkController->getRadius() + m_pkController->getHeight() * 0.5f));
//	}
//}

/*	// 펫 관련 함수들

bool PgActor::OutOfSight(BM::GUID &rkTargetGuid, float fDistance, bool bConsiderZAxis)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.OutOfSight"), g_pkApp->GetFrameCount()));
	PgPilot *pkTargetPilot = g_kPilotMan.FindPilot(rkTargetGuid);
	if(!pkTargetPilot)
	{
		return false;
	}

	NiPoint3 kTargetPoint = pkTargetPilot->GetWorldObject()->GetTranslate();
	NiPoint3 kMyPoint = GetTranslate();

	if(!bConsiderZAxis)
	{
		kMyPoint.z = kTargetPoint.z = 0.0f;
	}

	if(NiAbs((kTargetPoint - kMyPoint).Length()) < fDistance)
	{
		return false;
	}

	return true;
}

bool PgActor::FollowActor(BM::GUID &rkTargetGuid, float fMoveSpeed)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.FollowActor"), g_pkApp->GetFrameCount()));
	PgPilot *pkTargetPilot = g_kPilotMan.FindPilot(rkTargetGuid);
	if(!pkTargetPilot)
	{
		return false;
	}

	std::string kCurrentActionName;
	if(m_pkAction)
	{
		kCurrentActionName = m_pkAction->GetID();
	}

	NiPoint3 const &rkActorPos = pkTargetPilot->GetWorldObject()->GetTranslate();
	NiPoint3 kDistance = rkActorPos - GetTranslate();

	SetWalkingTargetLoc(rkActorPos, true);

	//Walk(GetDirection(), fMoveSpeed, 0);
	//FindPathNormal();

	//CUnit *pkUnit = GetPilot()->GetUnit();

	//BYTE byDir = GetDirection();
	//Walk(byDir, fMoveSpeed, 1);

	//movingSpeed = GetAbil(AT_C_MOVESPEED);
	//kDistance = m_kPathNormal.UnitCross(kDistance);
	//if(kDistance == NiPoint3::UNIT_Z && kCurrentActionName != "a_run_left")
	//{
	//	return TransitAction("a_run_left");
	//}
	//else if(kDistance == -NiPoint3::UNIT_Z && kCurrentActionName != "a_run_right")
	//{
	//	return TransitAction("a_run_right");
	//}

	return ReserveTransitAction("a_run") ? true : false;
}


// Return값이 양수이면, 자신을 기준으로 Target Object가 Axis의 +방향에 있다는 것이다. 음수이면 그 반대 
int PgActor::CompareActorPosition(BM::GUID &rkTargetGuid, NiPoint3 kAxis, float fRange)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.CompareActorPosition"), g_pkApp->GetFrameCount()));
	PgPilot *pkTargetPilot = g_kPilotMan.FindPilot(rkTargetGuid);
	if(!pkTargetPilot)
	{
		return false;
	}

	NiPoint3 kTargetPoint = NiPoint3::ComponentProduct(pkTargetPilot->GetWorldObject()->GetTranslate(), kAxis);
	NiPoint3 kMyPoint = NiPoint3::ComponentProduct(GetTranslate(), kAxis);

	NiPoint3 kDelta = kTargetPoint - kMyPoint;
	if(kDelta.Length() < fRange)
	{
		return 0;
	}

	return (int)kDelta.Dot(NiPoint3::UNIT_ALL);
}

bool PgActor::WillBeFall(bool bDown, float fDistance)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.WillBeFall"), g_pkApp->GetFrameCount()));
#ifdef PG_USE_CAPSULE_CONTROLLER
	float fCenterHeight = m_pkController->getHeight() * 0.5f + m_pkController->getRadius();
	float fLegHeight = m_pkController->getRadius();
#else
	float fCenterHeight = PG_CHARACTER_CAPSULE_HEIGHT * 0.5f + PG_CHARACTER_CAPSULE_RADIUS;
	float fLegHeight = PG_CHARACTER_CAPSULE_RADIUS;
#endif

	// 계단인지 절벽인지 검사한다.
	NxVec3 kMovingDelta;
	NiPhysXTypes::NiPoint3ToNxVec3(m_kMovingDir, kMovingDelta);
	kMovingDelta *= fDistance;
	NxExtendedVec3 kControllerPos = m_pkController->getFilteredPosition();

	// 캐릭터 위치에서 한 번, MovingDir * Distnace 앞에서 한 번 Ray를 쏜다.
	NxVec3 kOriginalPos((NxReal)kControllerPos.x, (NxReal)kControllerPos.y, (NxReal)kControllerPos.z);
	NxVec3 kOriginalFrontPos = kOriginalPos + kMovingDelta;
	NxVec3 kDirection(0.0f, 0.0f, (bDown ? -1.0f : 1.0f));
		
	NxVec3 kDiff(0.0f, 0.0f, fCenterHeight - 10.0f);

	// Ray 최대 거리
	NxReal fMaxDist = fLegHeight + (bDown ? 10.1f : 80.0f);

	NxRay kRay(kOriginalFrontPos - kDiff, kDirection);
	NxRaycastHit kHit;
	NxShape *pkHitShape = m_pkPhysXScene->GetPhysXScene()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 0xffffffff, fMaxDist, NX_RAYCAST_SHAPE);

	// bDown == true 일 때는 pkHitShape(bRet)이 false이면 유효, true이면 false 리턴해야 함.
	// bDown == false 일 때는 pkHitShape(bRet)이 true이면 유효, false이면 false 리턴해야 함.
	bool bFirstTry = !pkHitShape == bDown;
	bool bSecondTry = false;

	if(bFirstTry)
	{
		kRay.orig = kOriginalPos - kDiff;
		pkHitShape = m_pkPhysXScene->GetPhysXScene()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 0xffffffff, fMaxDist, NX_RAYCAST_SHAPE);

		// bDown == true일 때는 두 번째 Ray가 true이면 유효, false이면 false 리턴해야 함.
		// bDown == false일 때는 두 번째 Ray가 false이면 유효, true이면 false를 리턴해야 함.
		bSecondTry = !pkHitShape != bDown;
	}

	return bFirstTry && bSecondTry;
}

bool PgActor::RideMyPet(bool bRide)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.RideMyPet"), g_pkApp->GetFrameCount()));
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(GetPilot()->GetUnit());
	BM::GUID kGuid = pkPlayer->PetGuid();
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);

	if(!pkPilot)
	{
		return 0;
	}

	PgActor *pkPet = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
	if(!pkPet)
	{
		return 0;
	}

	NiNode* pkAttachingNode = (NiNode* ) GetNIFRoot();
		
	if(bRide)
	{
		NiNode* pkSyncNode = (NiNode* )GetNIFRoot()->GetObjectByName("p_pt_ride");		
		NiNode* pkNode = (NiNode* )pkPet->GetNIFRoot()->GetObjectByName("p_pt_ride");

		if(!pkSyncNode || !pkNode)
		{
			return false;
		}

		NiPoint3 kDelta = pkSyncNode->GetWorldTranslate() - pkAttachingNode->GetWorldTranslate();
		NiPoint3 kTargetPos = pkNode->GetTranslate() - kDelta - NiPoint3(0,0,5.0f);
		pkAttachingNode->SetTranslate(NiPoint3(0.0f, 0.0f, kTargetPos.z));
		
		pkNode->AttachChild(pkAttachingNode);
		pkAttachingNode->UpdateNodeBound();
		pkAttachingNode->UpdateProperties();
		pkAttachingNode->UpdateEffects();
//		pkAttachingNode->Update(0.0f);
	}
	else
	{
		NiNode* pkNode = (NiNode* )pkPet->GetObjectByName("p_pt_ride");
		if(!pkNode || !pkNode->GetParent())
		{
			return false;
		}

		NiAVObjectPtr spRoot = pkNode->DetachChild(GetNIFRoot());
#ifdef PG_USE_CAPSULE_CONTROLLER
		spRoot->SetTranslate(NiPoint3(0.0f, 0.0f, -(m_pkController->getHeight() * 0.5f + m_pkController->getRadius())));
#else
		spRoot->SetTranslate(NiPoint3(0.0f, 0.0f, -(PG_CHARACTER_CAPSULE_HEIGHT * 0.5f + PG_CHARACTER_CAPSULE_RADIUS)));
#endif
		AttachChild(spRoot);
//		spRoot->Update(0.0f);
		spRoot->UpdateNodeBound();
		spRoot->UpdateProperties();
		spRoot->UpdateEffects();
	}

	m_bRiding = bRide;
		
	return true;
}
*/

NiAVObject *PgActor::GetUIModelUpdate()
{
	if(!m_bNeedToUpdateUIModel)
	{
		return 0;	
	}
	
	m_bNeedToUpdateUIModel = false;
	return GetNIFRoot();
}

void PgActor::SetNeedUIModelUpdate(bool bFlag)
{
	m_bNeedToUpdateUIModel = bFlag;
}

//void PgActor::SetPetMaster(BM::GUID const &rkGuid)
//{
//	m_kMasterGuid = rkGuid;
//}
//
//BM::GUID &PgActor::GetPetMaster()
//{
//	return m_kMasterGuid;
//}

void PgActor::SetSpeedScale(float fScale)
{
	m_fMovingSpeedScale = NiClamp(fScale, -1.0f, 1.0f);
}

float PgActor::GetSpeedScale()
{
	return m_fMovingSpeedScale;
}

//---------------------------------------------------------------------------
PgActor::ActorCallbackObject::~ActorCallbackObject()
{
    // Class has virtual members. Must have virtual destructor.
}
//---------------------------------------------------------------------------
void PgActor::ActorCallbackObject::TextKeyEvent(
    NiActorManager* pkManager, NiActorManager::SequenceID eSequenceID,
    const NiFixedString& kTextKey, const NiTextKeyMatch* pkMatchObject,
    float fCurrentTime, float fEventTime)
{
	PgActor *pkActor = dynamic_cast<PgActor *>(m_pkWorldObject);

//	if(pkActor && pkActor->IsMyActor()) 
//	{
//		_PgOutputDebugString("PgActor::ActorCallbackObject::TextKeyEvent iIndex:%d eSequenceID:%d kTextKey:%s fCurrentTime:%f fEventTime:%f\n", iIndex,eSequenceID,(char const*)kTextKey,fCurrentTime,fEventTime);
//	}

	if(g_pkWorld && pkActor->IsVisible() == false)
	{
		pkActor->m_bVisible = true;
		pkActor->NiNode::Update(g_pkWorld->GetAccumTime());
		pkActor->m_bVisible  = false;
	}

	if(m_kLastEventName == "")
	{
		m_kLastEventSequenceID = NiActorManager::INVALID_SEQUENCE_ID;
		m_fLastEventTime = -1;
		m_iLastEventIndex = -1;
	}

	//PgOutputPrint6("[%s] SequenceID( %d/ %d ), EventTime( %.3f, %.3f ), %s", __FUNCTION__, m_kLastEventSequenceID, eSequenceID, m_fLastEventTime, fEventTime, kTextKey);
	if(m_kLastEventSequenceID == eSequenceID &&
		strcmp(kTextKey,m_kLastEventName.c_str())==0)
	{
		if(m_fLastEventTime == fEventTime)
		{
			return;
		}
	}
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.TextKeyEvent"), g_pkApp->GetFrameCount()));

	m_kLastEventSequenceID = eSequenceID;
	m_fLastEventTime = fEventTime;
	m_kLastEventName = std::string(((char const*)kTextKey));

	if(pkActor && pkActor->GetAction())
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		//NILOG(PGLOG_LOG, "TextKeyEvent(%s) %s Event called\n", m_kScriptName.c_str(), m_kLastEventName.c_str());
		//if(pkActor->IsMyActor())
		//{
		//	_PgOutputDebugString("TextKeyEvent(%s) %s Event called\n", m_kScriptName.c_str(), m_kLastEventName.c_str());
		//}

		if(!pkActor->GetAction()->EventFSM(pkActor, (char const *)kTextKey,eSequenceID))
		{
			lua_tinker::call<bool, lwActor, char const*>(m_kScriptName.c_str(), lwActor(pkActor), (char const *)kTextKey);
		}
		NIMETRICS_ENDTIMER(a);
	}
}
//---------------------------------------------------------------------------
NxControllerAction PgActor::PgControllerHitReport::onControllerHit(const NxControllersHit &hit)
{
	//NxActor *pkActorA = hit.controller->getActor();
	//NxActor *pkActorB = hit.other->getActor();

	//PG_ASSERT_LOG(pkActorA);
	//PG_ASSERT_LOG(pkActorB);

	//if((pkActorA->getGroup() == 2 && pkActorB->getGroup() == 3)
	//|| (pkActorA->getGroup() == 3 && pkActorB->getGroup() == 2))
	//{
	//	NxActor *pkPlayer = 0;
	//	
	//	if(pkActorA->getGroup() == 2)
	//	{
	//		pkPlayer = pkActorA;
	//	}
	//	else
	//	{
	//		pkPlayer = pkActorB;
	//	}

	//	if(pkPlayer)
	//	{
	//		PgActor *pkPlayerActor = (PgActor *)pkPlayer->userData;

	//		if(pkPlayerActor->GetAction()->GetID() != "a_touch_dmg")
	//		{
	//			//	땜빵 예외 처리, 범퍼카는 대미지하지 않는다.
	//			if(strcmp(((PgIXmlObject*)pkPlayerActor)->GetID().c_str(),"c_bumpercar")!=0)
	//			{
	//				pkPlayerActor->TransitAction("a_touch_dmg");
	//			}
	//		}
	//	}
	//}

	return NX_ACTION_NONE;
}
//---------------------------------------------------------------------------
PgActor::ColorSet::ColorSet(const NiColor &rkAmbient, const NiColor &rkEmissive,const NiColor &rkSpecular,const NiColor &rkDiffuse) :
	m_kAmbient(rkAmbient),
	m_kEmissive(rkEmissive),
	m_kSpecular(rkSpecular),
	m_kDiffuse(rkDiffuse)
{
}
//---------------------------------------------------------------------------
PgActor::ItemDesc::ItemDesc(eEquipLimit kItemPos) :
	m_kItemPos(kItemPos),
	m_iItemNo(0),
	m_kEnchantInfo()
{
}

PgActor::ItemDesc::ItemDesc(eEquipLimit kItemPos, int iItemNo, SEnchantInfo const& kEnchant) :
	m_kItemPos(kItemPos),
	m_iItemNo(iItemNo),
	m_kEnchantInfo(kEnchant)
{
}
//--------------------------------------------------------------------------
bool PgActor::ItemDesc::operator==(const PgActor::ItemDesc &rhs)
{
	return m_kItemPos == rhs.m_kItemPos;
}
//--------------------------------------------------------------------------
PgActor::AMPair::AMPair(eEquipLimit kItemPos, NiActorManagerPtr spAM,PgItemEx::stCustomAniIDChangeSetting *pstCustomAniIDChangeSetting) :
	m_kItemPos(kItemPos),
	m_spAM(spAM),
	m_stCustomAniIDChangeSetting(*pstCustomAniIDChangeSetting)
{
}
//--------------------------------------------------------------------------
bool PgActor::AMPair::operator==(const PgActor::AMPair &rhs)
{
	return m_kItemPos == rhs.m_kItemPos;
}
//--------------------------------------------------------------------------
bool PgActor::LookAt(NiPoint3 const& rkTarget, bool bTurnRightAway,bool bBidirection,bool bNotCheckSameDir)
{
	if(false == EnableRotation())
	{
		return false;
	}

	NiPoint3 kLookingDir = rkTarget - GetPosition();

	kLookingDir.z=0;
	kLookingDir.Unitize();
	if(bBidirection && kLookingDir != NiPoint3::ZERO)
	{
		bool bLeft = ((m_kPathNormal.Cross(kLookingDir).z>0) ? true : false);
		kLookingDir = m_kPathNormal.Cross(NiPoint3::UNIT_Z * (bLeft ? -1.0f : 1.0f));
		kLookingDir.z = 0;

		kLookingDir.Unitize();
	}

	BYTE byDir = GetDirFromMovingVector(kLookingDir);

	if(bNotCheckSameDir == false)
	{
		if(byDir == m_byLastDirection)
		{
			if(IsLockBidirection())
			{
				// 보는 방향 같으면 구별 필요 없다.
				return false;
			}
		}
	}

	//_PgOutputDebugString("[Call ConcilDirection 3]Actor(%s) kLookingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),kLookingDir.x,kLookingDir.y,kLookingDir.z);
	ConcilDirection(kLookingDir, bTurnRightAway);

	if(byDir == DIR_LEFT)	
	{
		//왼쪽일때 true
		return true;
	}

	return false;
}


//bool PgActor::LookAt(NiPoint3 &rkTarget, bool bTurnRightAway)
//{
//	NiPoint3 kSub = rkTarget - GetTranslate();
//	if(kSub.Length() <= PG_EPSILON)
//	{
//		return false;	//보는 방향이 이전과 같으면 왼쪽 오른쪽 구별이 필요 없음
//	}
//
//	NiPoint3 p3LeftOrRight = m_kMovingDir.Cross(rkTarget);
//	m_kMovingDir = rkTarget - GetTranslate();
//	m_kMovingDir.z = 0.0f;
//	m_kMovingDir.Unitize();	
//	ConcilDirection(m_kMovingDir, bTurnRightAway);
//	if( p3LeftOrRight.z > 0 )	{ return true; }	//왼쪽일때 true
//
//	return false;
//}
//bool	PgActor::LookAtBidirection(NiPoint3 &rkTarget)
//{
//	NiPoint3 kSub = rkTarget - GetTranslate();
//	if(kSub.Length() <= PG_EPSILON)
//	{
//		return false;	//보는 방향이 이전과 같으면 왼쪽 오른쪽 구별이 필요 없음
//	}
//
//	NiPoint3 p3LeftOrRight = m_kPathNormal.Cross(kSub);
//	bool	bIsLeft = false;
//	if(p3LeftOrRight.z>0) bIsLeft = true;
//	ToLeft(bIsLeft,true);
//	return	true;
//}
void PgActor::IncRotate(float fRadian)
{
	NiQuaternion kQuat(fRadian, NiPoint3::UNIT_Z);
	
	NiMatrix3 kRot;
	kQuat.ToRotation(kRot);
	m_kLookingDir = kRot * m_kLookingDir;
	m_kLookingDir.Unitize();
	
	if(m_kLookingDir.Length() == 0)
	{
		m_kLookingDir = NiPoint3::UNIT_X;
	}

	//_PgOutputDebugString("[Call ConcilDirection 4]Actor(%s) kLookingDir(%f,%f,%f)\n",MB(GetPilot()->GetGuid().str()),m_kLookingDir.x,m_kLookingDir.y,m_kLookingDir.z);
	ConcilDirection(m_kLookingDir);
}

bool PgActor::ApplyMovingObject_OnEnter(PgTrigger* pkTrigger)
{
	if (!pkTrigger)
	{
		return false;
	}
	NiPhysXRigidBodySrc* pkPhysXRigidBodySrc = pkTrigger->GetPhysXRigidBodySrc();
	if (!pkPhysXRigidBodySrc)
	{
		return false;
	}
	NxMat34 kMat0 = pkPhysXRigidBodySrc->GetPose(0);
	NxMat34 kMat1 = pkPhysXRigidBodySrc->GetPose(1);

	//NxU32 collisionFlags = 0;
	//m_pkController->move(kMat1.t - kMat0.t, m_uiActiveGrp, 0.000001f, collisionFlags, 1.0f);
	//NxVec3 kVec = kMat1.t - kMat0.t;
	//NiPoint3 kPos = GetTranslate();
	//kPos += NiPoint3(kVec.x, kVec.y, kVec.z);
	//SetTranslate(kPos);
	MoveActorAbsolute(kMat1.t - kMat0.t);

	//m_pkPhysXSrc->SetActive(true);
	//m_pkPhysXDest->SetActive(false);

	//NiAVObject* pkSrcObj = pkPhysXRigidBodySrc->GetSource();
	//GetPosition();
	//m_pkPick->SetTarget(m_pkPathRoot);
	//m_pkPick->ClearResultsArray();
	//NiPoint3 kPickStart = kPos + NiPoint3(0, 0, 30.0f);
	//m_pkPick->PickObjects(kPickStart, NiPoint3(0,0,0), false);


	return true;
}

bool PgActor::ApplyMovingObject_OnUpdate(PgTrigger* pkTrigger)
{
	if (!pkTrigger)
	{
		return false;
	}
	return true;
}

bool PgActor::ApplyMovingObject_OnLeave(PgTrigger* pkTrigger)
{
	if (!pkTrigger)
	{
		return false;
	}

	//m_pkPhysXSrc->SetActive(false);
	//m_pkPhysXDest->SetActive(true);
	return true;
}
void PgActor::PlayWeaponSound(PgWeaponSoundManager::EWeaponSoundType eType, PgActor *pkPeer, char const *pcActionID, float fVolume, PgActionTargetInfo* pkTargetInfo)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.PlayWeaponSound"), g_pkApp->GetFrameCount()));
	
	if(!pkPeer)
	{
		return;
	}

	int iWeaponType = pkPeer->GetEquippedWeaponType();
	std::string const &kSoundID = g_kWeaponSoundMan.GetSound(eType, iWeaponType, pcActionID, pkPeer->GetMyWeaponNo());

	NiPoint3* pkPos = NULL;
	NiPoint3 kPos;
	if( pkTargetInfo )
	{
		kPos = GetABVShapeWorldPos( pkTargetInfo->GetSphereIndex() ) - GetWorldTranslate();
		pkPos = &kPos;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,kSoundID.c_str(), fVolume,80,330,this,pkPos);

}

//! 액터 NIF의 노드를 숨기거나 보이게 한다.
void PgActor::HideNode(char const *strNodeName,bool bHide)
{
	_PgOutputDebugString("[PgActor::HideNode] Actor:%s NodeName:%s bHide:%d\n", MB(GetPilotGuid().str()),strNodeName,bHide);

	if(GetActorManager())
	{
		NiAVObject	*pkObj = GetCharRoot()->GetObjectByName(strNodeName);
		if(pkObj)
		{
			pkObj->SetAppCulled(bHide);
		}
	}
}
void PgActor::ApplyHidePartsAll()
{
	for(PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin(); itr != m_kPartsAttachInfo.end(); ++itr)
	{
		eEquipLimit	const	&kEquipLimit = itr->first;
		PgItemEx *pkParts = itr->second;
		if(!pkParts)
		{
			continue;
		}

		bool	bHide = false;
		IntMap::iterator itor_0 = m_kPartsHideInfo.find(kEquipLimit);
		
		if(itor_0 != m_kPartsHideInfo.end())
		{
			bHide = (itor_0->second>0);
		}

		pkParts->Hide(bHide);
	}

}
void PgActor::ApplyHideParts(const eEquipLimit kEquipLimit)
{
	bool	bHide = false;
	IntMap::iterator itor_0 = m_kPartsHideInfo.find(kEquipLimit);
	
	if(itor_0 != m_kPartsHideInfo.end())
	{
		bHide = (itor_0->second>0);
	}

	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kEquipLimit);
	if(itr == m_kPartsAttachInfo.end())
	{
		return;
	}

	PgItemEx *pkParts = itr->second;
	if(!pkParts || pkParts->EquipLimit() != kEquipLimit)
	{
		return;
	}

	pkParts->Hide(bHide);
}

bool PgActor::HideParts(const eEquipLimit kEquipLimit, bool const bHide)
{

	IntMap::iterator itor_0 = m_kPartsHideInfo.find(kEquipLimit);
	int	iHideCount = 0;

	if(itor_0 == m_kPartsHideInfo.end())
	{
		iHideCount = bHide ? 1 : 0;
		m_kPartsHideInfo.insert(std::make_pair(kEquipLimit,iHideCount));
	}
	else
	{
		iHideCount = itor_0->second;
		iHideCount += bHide ? 1 : -1;

		itor_0->second = iHideCount;
	}


	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kEquipLimit);
	if(itr == m_kPartsAttachInfo.end())
	{
		return true;
	}

	PgItemEx *pkParts = itr->second;
	if(!pkParts || pkParts->EquipLimit() != kEquipLimit)
	{
		return true;
	}

	pkParts->Hide(iHideCount>0);

	return true;
}

bool PgActor::GetPartsHideCnt(eEquipLimit const kEquipLimit, int& iCnt_out) const
{// 해당 파츠의 HideCount값을 얻어온다
	IntMap::const_iterator itor = m_kPartsHideInfo.find(kEquipLimit);	
	if(itor == m_kPartsHideInfo.end())
	{
		return false;
	}
	iCnt_out = itor->second;
	return true;
}

bool PgActor::HideParts_IgnoreHideCnt(const eEquipLimit kEquipLimit, bool const bHide)
{//해당 파츠의 HideCount를 무시하고 동작한다.(신종족 격투가 보조무기용)
	IntMap::iterator itor_0 = m_kPartsHideInfo.find(kEquipLimit);
	int	iHideCount = 0;

	if(itor_0 == m_kPartsHideInfo.end())
	{
		iHideCount = bHide ? 1 : 0;
		m_kPartsHideInfo.insert(std::make_pair(kEquipLimit,iHideCount));
	}
	else
	{
		iHideCount = itor_0->second;
		iHideCount += bHide ? 1 : -1;

		itor_0->second = iHideCount;
	}

	PartsAttachInfo::iterator itr = m_kPartsAttachInfo.find(kEquipLimit);
	if(itr == m_kPartsAttachInfo.end())
	{
		return true;
	}

	PgItemEx *pkParts = itr->second;
	if(!pkParts || pkParts->EquipLimit() != kEquipLimit)
	{
		return true;
	}

	pkParts->Hide(bHide);

	return true;
}

void PgActor::LockBidirection(bool bLock)
{
	m_bLockBidirection = bLock;
}

void	PgActor::SetUnderMyControl(bool bUnderMyControl)
{
	m_bIsUnderMyControl = bUnderMyControl;
}
bool	PgActor::IsUnderMyControl()
{
	return	(IsMyActor() || m_bIsUnderMyControl || GetCallerIsMe());
}

bool PgActor::IsMyPet()
{
	if(GetPilot())
	{
		if(GetPilot()->GetUnit())
		{
			if(GetMySelectedPet() == GetPilot()->GetUnit())
			{
				return true;
			}
		}
	}

	return false;
}
bool PgActor::IsMySubPlayer()
{
	bool bIsMySubPlayer = false;
	PgPilot* pkPilot =   GetPilot();
	if(!pkPilot)
	{
		return false;
	}
	CUnit* pkUnit = pkPilot->GetUnit();
	if(!pkUnit)
	{
		return false;
	}
	if( UT_SUB_PLAYER != pkUnit->UnitType() )
	{
		return false;
	}
	return GetCallerIsMe();
}

bool PgActor::GetCallerIsMe()
{
	if(GetPilot())
	{
		if(GetPilot()->GetUnit())
		{
			if(g_kPilotMan.IsMyPlayer(GetPilot()->GetUnit()->Caller()))
			{
				return true;
			}
		}
	}	

	return false;
}

void PgActor::SetActiveGrp(int iGroupNo, bool bUse)
{
	// 맥스에서 UserProp에 NiPhysXShapeGroup = <Group No> 와 같이 적으면 됨
	// (그래픽 팀에 User Properties에 physX 메쉬에 위와 같이 적어서 익스포트 해달라고 요청하면 된다)
	// NiPhysXShapeGroup = <integer> can be used on an object 
	// that will be a PhysX shape (an actor or a proxy for an actor or part of a shape group). 
	// It specifies the shape group which is used in basic collision filtering

	if(bUse)
	{
		m_uiActiveGrp |= (1 << iGroupNo);
	}
	else
	{
		m_uiActiveGrp &= ~(1 << iGroupNo);
	}
}

void PgActor::GetNameEmoticon(std::wstring &rkOut)
{
	PgPilot *pkPilot = GetPilot();
	if( !GetPilot() )
	{
		return;
	}
	CUnit *pkUnit = pkPilot->GetUnit();
	if( !pkUnit )
	{
		return;
	}
	std::wstring kTemp;

	//
	if( pkUnit->IsInUnitType(UT_MONSTER) && g_pkWorld)
	{
		bool const bChaosMap = (0 != (g_pkWorld->GetAttr() & GATTR_CHAOS_F));
		if( bChaosMap )
		{
			kTemp += _T("C");
		}
		bool const bIsDependMonster = g_kQuestMan.IsDependIngQuestMonster(pkUnit->GetAbil(AT_CLASS), g_kNowGroundKey.GroundNo());
		if( bIsDependMonster
		&&	!pkUnit->IsUnitType(UT_OBJECT) ) // 파괴 오브젝트는 여기서 안함
		{
			kTemp += L"E";
		}
	}

	//
	if( pkUnit->IsUnitType(UT_PLAYER) )
	{
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		if( pkPlayer )
		{
			bool const bHaveCouple = BM::GUID::IsNotNull( pkPlayer->CoupleGuid() );
			if( bHaveCouple )
			{
				if( g_kPilotMan.IsMyPlayer(pkPlayer->CoupleGuid()) ) // 내 커플이 근처면
				{
					PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
					if( pkMyActor )
					{
						pkMyActor->UpdateName(); // 내 하트도 두근반근
					}
				}
				bool const bSameGroundCouple = (NULL != g_kPilotMan.FindPilot(pkPlayer->CoupleGuid()));
				//if( true == g_kCoupleMgr.IsSweetHeart() )
				if( (true == g_kCoupleMgr.IsSweetHeart()) || (pkPlayer->GetCoupleStatus() == (CoupleS_Normal | CoupleS_SweetHeart)) )
				{
					kTemp += _T("D");
				}
				else
				{
					kTemp += ((bSameGroundCouple)? _T("B"): _T("A"));
				}
			}
		}
	}

	//std::wstring(_T("{T=EmoticonFont/C=0xFFFFFFFF/}ABCDEFGHIJK"))
	if( !kTemp.empty() )
	{
		rkOut = _T("{T=EmoticonFont24x24/C=0xFFFFFFFF/}") + kTemp;
	}
}
void PgActor::GetEnchantPrefixName(std::wstring& rkOut, std::wstring const& rkNameFont, std::wstring const& rkNameColor)
{
	int const iMonEnchantGradeNo = m_pkPilot->GetAbil(AT_MON_ENCHANT_GRADE_NO);
	if( iMonEnchantGradeNo )
	{
		CONT_DEF_MONSTER_ENCHANT_GRADE const* pkDefMonEnchantGrade = NULL;
		g_kTblDataMgr.GetContDef(pkDefMonEnchantGrade);
		CONT_DEF_MONSTER_ENCHANT_GRADE::const_iterator find_iter = pkDefMonEnchantGrade->find(iMonEnchantGradeNo);
		if( pkDefMonEnchantGrade->end() != find_iter )
		{
			CONT_DEF_MONSTER_ENCHANT_GRADE::mapped_type const& rkMonEnchantGrade = (*find_iter).second;
			if( rkMonEnchantGrade.iPrefixNameNo )
			{
				std::wstring kTemp(TTW(rkMonEnchantGrade.iPrefixNameNo));
				PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$ENCHANT_LEVEL$"), std::wstring(BM::vstring(rkMonEnchantGrade.iEnchantLevel)), kTemp);
				PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$NAME_FONT$"), rkNameFont, kTemp);
				PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$NAME_COLOR$"), rkNameColor, kTemp);
				rkOut.swap(kTemp);
			}
		}
	}
	else
	{
		rkOut.swap( std::wstring() );
	}
}

bool PgActor::GetNameColor(std::wstring &rkOut)
{
	const wchar_t * DEFAULT_COLOR = L"{C=0xFF66FF66/}";

	if( !g_pkWorld )
	{
		return false;
	}

	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetNameColor"), g_pkApp->GetFrameCount()));
	if(GetPilot())
	{
		CUnit *pkUnit = GetPilot()->GetUnit();
		if (!pkUnit)
		{
			rkOut = std::wstring( DEFAULT_COLOR );
			return true;
		}

		int const iNameColor = pkUnit->GetAbil(AT_NAME_COLOR);
		if( iNameColor  )
		{
			wchar_t szBuf[MAX_PATH] ={0,};
			swprintf(szBuf, MAX_PATH, L"{C=0x%08X/}", iNameColor);
			rkOut = szBuf;
			return true;
		}

		int iTeam = pkUnit->GetAbil(AT_TEAM);
		if(g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND))
		{ //커뮤니티 이벤트 던전이면 나와 같은 팀원에게 이름 색상을 빨강으로 설정
			BM::GUID kUnitGuid = pkUnit->GetID();
			if(kUnitGuid.IsNotNull() && PgClientPartyUtil::IsInPartyMemberGuid(kUnitGuid))
			{ //이 유닛은 내 파티에 속한 놈인가?
				iTeam = TEAM_RED;
			}
			else
			{
				iTeam = TEAM_NONE;
			}
		}
		switch( iTeam )
		{
		case TEAM_NONE:
			{
				if( pkUnit->IsInUnitType(UT_MONSTER) )
				{
					const CUnit *pkPlayerUnit = g_kPilotMan.GetPlayerUnit();
					if (!pkPlayerUnit)
					{
						rkOut = std::wstring( DEFAULT_COLOR );
						return true;
					}

					int const iGrade = pkUnit->GetAbil(AT_GRADE);
					if ( EMGRADE_BOSS == iGrade || EMGRADE_ELITE == iGrade )	//레벨로 색을 정해도 몹 등급이 우선한다
					{
						/*switch(iGrade)
						{
						case EMGRADE_NORMAL:{	kColor = NiColorA(0.0f, 1.0f, 0.0f, 1.f);}break;
						case EMGRADE_UPGRADED:{	kColor = NiColorA(0.047f, 0.0f, 1.0f, 1.f);}break;
						case EMGRADE_ELITE:{	kColor = NiColorA(1.0f, 0.2f, 1.0f, 1.f);}break;
						case EMGRADE_BOSS:{	kColor = NiColorA(1.0f, 0.2f, 1.0f, 1.f);}break;
						default:{kColor = NiColorA(0.4f, 1.0f, 0.4f, 1.f);}break;
						}*/

						rkOut = std::wstring( _T("{C=0xFFFF00FF/}") );
						return true;
					}

					int const iMonLevel = pkUnit->GetAbil(AT_LEVEL);
					int const iPlayerLevel = pkPlayerUnit->GetAbil(AT_LEVEL);
					int const iLevDelta = iMonLevel - iPlayerLevel;

					if (iLevDelta >= 10)		{ rkOut = std::wstring( _T("{C=0xFFFF0000/}") );	}
					else if (iLevDelta >= 6)	{ rkOut = std::wstring( _T("{C=0xFFFF593B/}") );	}
					else if (iLevDelta >= 3)	{ rkOut = std::wstring( _T("{C=0xFFFFBD45/}") );	}
					else if (iLevDelta >= -4)	{ rkOut = std::wstring( _T("{C=0xFFB7FF48/}") );	}
					else if (iLevDelta >= -7)	{ rkOut = std::wstring( _T("{C=0xFFFFFFFF/}") );	}
					else						{ rkOut = std::wstring( _T("{C=0xFFB2B2B2/}") );	}
				}
				else if( pkUnit->IsUnitType(UT_PLAYER) )
				{
					rkOut = std::wstring( _T("{C=0xFFFF00FF/}") );

					PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkUnit);
					if ( pkPC )
					{
						BM::GUID const &rkPartyGuid = pkPC->PartyGuid();
						bool const bNullPartyGuid = BM::GUID::NullData() == rkPartyGuid;
						if( !bNullPartyGuid
							&&	g_kParty.PartyGuid() == rkPartyGuid )//같은 파티원은
						{
							rkOut = std::wstring( _T("{C=0xFF00D1FF/}") );
						}
						//else if( !bNullPartyGuid )//다른 파티원
						//{
						//	kColor = NiColorA(0.13f, 0.23f, 1.f, 1.f);
						//}
						else
						{
							int const iGender = pkPC->GetAbil(AT_GENDER);

							switch( iGender )
							{
							case UG_MALE:	{ rkOut = std::wstring( _T("{C=0xFF79BEFF/}") ); } break;
							case UG_FEMALE:	{ rkOut = std::wstring( _T("{C=0xFFFF94D0/}") ); } break;
							default:		{ rkOut = std::wstring( _T("{C=0xFFFFFF00/}") ); } break;
							}
						}	
					}
				}
				else
				{
					//kColor = NiColorA(0.1f, 0.8f, 0.1f, 1.f);
					rkOut = std::wstring( _T("{C=0xFF19CC19/}") );
				}
			}break;
		default:
			{
				std::wstring const kRedTeam( _T("{C=0xFFFF0000/}") );
				std::wstring const kBlueTeam( _T("{C=0xFF0000FF/}") );

				switch( iTeam )
				{
				case TEAM_RED:
					{
						rkOut = kRedTeam;
					}break;
				case TEAM_BLUE:
					{
						rkOut = kBlueTeam;
					}break;
				}
			}break;
// 		default:
// 			{
// 				if ( IsMyActor() )
// 				{
// 					rkOut = std::wstring( _T("{C=0xFF0000FF/}") );
// 				}
// 				else
// 				{
// 					rkOut = std::wstring( _T("{C=0xFFFF0000/}") );
// 				}	
// 			}break;
		}

		if( pkUnit->IsInUnitType(UT_PET) )
		{
			PgPet *pkPet = dynamic_cast<PgPet*>(pkUnit);
			if(pkPet)
			{
				if(EPET_TYPE_2==pkPet->GetPetType() || EPET_TYPE_3==pkPet->GetPetType())
				{
					rkOut = TTW(7515);
				}
				else
				{
					rkOut = std::wstring( DEFAULT_COLOR );
				}
			}
		}
	}
	return true;
}

bool PgActor::GetGuildNameColor( NiColorA &kColor)
{
	if( !m_pkPilot )
	{
		return false;
	}

	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(m_pkPilot->GetUnit());
	if( !pkPlayer )
	{
		return false;
	}

	BM::GUID const &kGuildGuid = pkPlayer->GuildGuid();
	if( BM::GUID::IsNull(kGuildGuid) )
	{
		return false;
	}

	if(g_pkWorld && 0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP) )
	{
		PgGuildMgrUtil::CalcGuidToColor( kGuildGuid, kColor );
		return true;
	}

	int iTeam = pkPlayer->GetAbil(AT_TEAM);
	if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND) )
	{
		PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkMyPlayer )
		{
			if( (pkPlayer->GetPartyGuid().IsNotNull() && pkPlayer->GetPartyGuid() == pkMyPlayer->GetPartyGuid()) ||
				(pkPlayer->GetExpeditionGuid().IsNotNull() && pkPlayer->GetExpeditionGuid() == pkMyPlayer->GetExpeditionGuid()) )
			{ //커뮤니티 이벤트 던전이고 나와 같은 팀원에게만 빨간색 이름으로 설정
				iTeam = TEAM_RED;
			}
			else
			{
				iTeam = TEAM_NONE;
			}
		}
		else
		{
			return false;
		}
	}

	switch( iTeam )
	{
	case TEAM_RED:
		{
			kColor = NiColorA(1.0f, 0.0f, 0.0f, 1.0f);
		}break;
	case TEAM_BLUE:
		{
			kColor = NiColorA(0.0f, 0.0f, 1.0f, 1.0f);
		}break;
	default:
		{
			PgGuildMgrUtil::CalcGuidToColor( kGuildGuid, kColor );
		}break;
	}
	return true;
}

void	PgActor::ActionToggleStateChange(int const iActionNo, bool const bOn)
{
	int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(iActionNo);

	if(bOn)
	{
		for(IntList::iterator itor = m_ActionToggleState.begin(); itor != m_ActionToggleState.end(); ++itor)
		{
			if(*itor == iKeySkillNo)
			{
				return;
			}
		}
		m_ActionToggleState.push_back(iKeySkillNo);
		
		StartSkillToggle(iKeySkillNo);
		
		return;
	}
	for(IntList::iterator itor = m_ActionToggleState.begin(); itor != m_ActionToggleState.end(); ++itor)
	{
		if(*itor == iKeySkillNo)
		{
			m_ActionToggleState.erase(itor);
			
			CutSkillToggle(iKeySkillNo);
			return;
		}
	}
}

bool	PgActor::GetActionToggleState(int const iActionNo) const	//	true : Activated  false : Deactivated
{
	int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(iActionNo);

	//	리스트 내에 있으면 Activated 된 상태이고, 리스트 내에 있지 않으면 Deactivated 된 상태이다.
	for(IntList::const_iterator itor = m_ActionToggleState.begin(); itor != m_ActionToggleState.end(); ++itor)
	{
		if(*itor == iKeySkillNo)
		{
			return	true;
		}
	}

	return	false;
}

NiPoint3 PgActor::GetPosition(bool bDebugPos)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.GetPosition"), g_pkApp->GetFrameCount()));

	if(bDebugPos)
	{
		if(m_pkController)
		{
			NxExtendedVec3 kDebugPos = m_pkController->getDebugPosition();
			return	NiPoint3((float)kDebugPos.x,(float)kDebugPos.y,(float)kDebugPos.z);
		}
	}
	else
	{
		if(m_pkController)
		{
			NxExtendedVec3 kDebugPos = m_pkController->getPosition();
			return	NiPoint3((float)kDebugPos.x,(float)kDebugPos.y,(float)kDebugPos.z);
		}
		
	}

	return	GetWorldTranslate();
}

void	PgActor::PickUpNearItem(bool bCheckCaller, float const fPickRange, int const iPickCount)	//	근처의 아이템을 집는다.
{
	if(!g_pkWorld)
	{
		return;
	}

	if(!IsMyActor())
	{
		if(bCheckCaller && GetPilot() && GetPilot()->GetUnit())
		{
			BM::GUID kPlayerGuid;
			if(g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid) && kPlayerGuid != GetPilot()->GetUnit()->Caller())
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	float	fNowTime = g_pkWorld->GetAccumTime();

	float	fElapsedTime =  fNowTime - m_fLastItemPickUpTime;
	if(fElapsedTime<0.1)
	{
		return;
	}

	m_fLastItemPickUpTime = g_pkWorld->GetAccumTime();


	float const  fPickUpTimeLimit = 1.0f;
	for(ActorPickUpInfoCont::iterator itor = m_kActorPickUpInfoCont.begin(); itor != m_kActorPickUpInfoCont.end();)
	{
		stActorItemPickupInfo *pkInfo = &(itor->second);
		if(fNowTime - pkInfo->m_fItemPickUpTime>fPickUpTimeLimit)
		{
			itor = m_kActorPickUpInfoCont.erase(itor);
			continue;
		}
		++itor;
	}


	NiPoint3 kCurrentPos = GetPosition();
	if(kCurrentPos == m_kLastItemPickUpPos && false==bCheckCaller)
	{
		return;
	}

	m_kLastItemPickUpPos = kCurrentPos;


	NiPhysXScene	*pkPhysXScene = g_pkWorld->GetPhysXScene();
	if(!pkPhysXScene)
	{
		return;
	}

	int	nbMaxShapes = 100;
	NxShape	*pkCollidedShapes[100];

	unsigned	int	uiGroup = 1<<(OGT_GROUNDBOX+1);

	NxVec3	kCenterPos(kCurrentPos.x,kCurrentPos.y,kCurrentPos.z);

	int iCount = GetWorld()->overlapSphereShapes(
		NxSphere(kCenterPos,fPickRange)
		,NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,NULL,uiGroup,NULL,true);

	if(iCount == 0)
	{
		return;
	}

	iCount = std::min(iCount, iPickCount);

	for(int i = 0; i < iCount; ++i)
	{
		if(pkCollidedShapes[i]->userData == NULL) continue;

		PgDropBox	*pkDropBox = (PgDropBox*)pkCollidedShapes[i]->userData;
		if (pkDropBox == NULL)
		{
			continue;
		}

		if( pkDropBox->IsMine() == false )	//	내것이 아니면 패스
		{
			float const fLimitTime = 10.f;
			if( fLimitTime > (g_pkApp->GetAccumTime() - pkDropBox->CreateTime()) )
			{
				// 10초 지나면 아무나 먹는다.
				continue;
			}
		}

		if( PgDropBox::E_BOX_IDLE!=pkDropBox->GetBoxState() )
		{
			continue;
		}

		if(pkDropBox->IsMoney() && !m_bAutoGetItemMoney) continue;
		if(pkDropBox->IsEquip() && !m_bAutoGetItemEquip) continue;
		if(pkDropBox->IsConsume() && !m_bAutoGetItemConsume) continue;
		if(pkDropBox->IsETC() && !m_bAutoGetItemETC) continue;

		PgPilot	*pkPilot = pkDropBox->GetPilot();
		if(!pkPilot) continue;

		BM::GUID	kItemGUID = pkPilot->GetGuid();
		
		//	동일한 아이템을 1초 내에 다시 PickUp 할 수 없다.
		ActorPickUpInfoCont::iterator itor = m_kActorPickUpInfoCont.find(kItemGUID);
		if(itor != m_kActorPickUpInfoCont.end())
		{
			stActorItemPickupInfo	*pkInfo = &(itor->second);
			if(fNowTime - pkInfo->m_fItemPickUpTime<fPickUpTimeLimit)
			{
				continue;
			}

			m_kActorPickUpInfoCont.erase(itor);
		}

		pkDropBox->SetOwnerGuid(GetPilotGuid());
		pkDropBox->Pickup();
		pkDropBox->SetOwnerGuid(BM::GUID::NullData());

		stActorItemPickupInfo	kInfo;
		kInfo.m_kItemGUID = kItemGUID;
		kInfo.m_fItemPickUpTime = fNowTime;

		m_kActorPickUpInfoCont.insert(std::make_pair(kItemGUID,kInfo));
	}
}

bool PgActor::SetPosition(NiPoint3 const &rkTranslate)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetPosition"), g_pkApp->GetFrameCount()));
	if(!m_pkController)
	{
		PG_ASSERT_LOG(!"Character Controller is null!");
		NILOG(PGLOG_ERROR, "[PgActor] SetPosition, %s(%s) actor has no Character Controller\n", GetPilot() ? MB(GetPilot()->GetName()) : "", MB(GetGuid().str()));
		return	true;
	}

	/// Comment : PhysX동기자가 다음 업데이트 시에 Gamebryo Object와 좌표를 맞춰 주기 때문에 
	///	그 프레임에 GetTranslate()을 하면 좌표가 한 프레임 어긋난다. 때문에 AVObject::SetTranslate()을 같이 해준다. 

	if(GetTranslate() == rkTranslate)
	{
		return	true;
	}

	m_pkController->setPosition(NxExtendedVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));
	SetPositionChanged(true);

	/// Comment ; NxCapsuleController::setPosition()가 bool을 리턴하는데, 메뉴얼(2.6.2)에 보면 현재는 항상 true를 리턴한다고 되어 있다.
	return true;
}

void PgActor::RestoreTexture()
{
	OrgTextureContainer::iterator itr;
	for ( itr = m_OrgTextureList.begin() ;
		itr != m_OrgTextureList.end() ;
		++itr)
	{
		TextureInfo orgTextInfo = *itr;
		if(orgTextInfo.pTexturing && orgTextInfo.spTexture)
		{
			NiTexturingProperty::Map *pkMap = orgTextInfo.pTexturing->GetMaps().GetAt(orgTextInfo.kMapEnum);

			if(pkMap)
			{
				pkMap->SetTexture(orgTextInfo.spTexture);
			}
		}
	}
	m_OrgTextureList.clear();
}

void PgActor::ChangeTexture(NiNode* pkRoot)
{
	PG_ASSERT_LOG(g_pkWorld);
	if (g_pkWorld == NULL || pkRoot == NULL)
		return;

	// 텍스쳐가 있으면 텍스쳐를 바꾸어준다.
	NiObjectList kGeometries;
	g_pkWorld->GetAllGeometries(pkRoot, kGeometries);
	while(!kGeometries.IsEmpty())
	{
		NiGeometry *pkGeo = NiDynamicCast(NiGeometry, kGeometries.GetTail());
		kGeometries.RemoveTail();

		if (!pkGeo || !pkGeo->GetPropertyState())
		{
			continue;
		}

		NiTexturingProperty* pkTextureProp = pkGeo->GetPropertyState()->GetTexturing();
		if(!pkTextureProp)
		{
			continue;
		}

		const	NiTexturingProperty::NiMapArray	&kMaps = pkTextureProp->GetMaps();
		const	int	iTotal = kMaps.GetSize();

		for(int i = 0;i < iTotal; ++i)
		{

			NiTexturingProperty::Map *pkMap = kMaps.GetAt(i);
			if(!pkMap || !pkMap->GetTexture())
			{
				continue;
			}

			NiSourceTexture* pkSrc = NiDynamicCast(NiSourceTexture, pkMap->GetTexture());
			if(!pkSrc)
			{
				continue;
			}

			std::string strTexture = pkSrc->GetFilename();

			VariTextureContainer::iterator itr = m_VarTextureList.find(strTexture);
			if(itr == m_VarTextureList.end())
			{
				continue;
			}

			std::string const& strNewTexture = itr->second;
			NiSourceTexture* pTexture = g_kNifMan.GetTexture(strNewTexture);
			if (pTexture)
			{
				TextureInfo orgTextInfo;
				orgTextInfo.pTexturing = pkTextureProp;
				orgTextInfo.kMapEnum = (NiTexturingProperty::MapEnum)i;
				orgTextInfo.spTexture = pkMap->GetTexture();
				m_OrgTextureList.push_back(orgTextInfo);

				pkMap->SetTexture(pTexture);
			}
			else
			{
				std::string strError = "Texture load failed : ";
				strError += strNewTexture.c_str();

				PgError(strError.c_str());
				PG_ASSERT_LOG(!strError.c_str());
			}

		}
	}
}

bool PgActor::GetActorDead()
{
	if(GetPilot() && GetPilot()->GetUnit())
	{
		return GetPilot()->GetUnit()->IsDead();
	}

	return false;
}


float PgActor::GetJumpAccumHeight()
{
	return m_fJumpAccumHeight;
}
float	PgActor::GetJumpTime()	const
{
	return	m_fJumpTime;
}

void PgActor::ResetJumpAccumHeight()
{
	m_fJumpAccumHeight = 0.0f;
}

void PgActor::ResetLastFloorPos()
{
	m_kLastFloorPos.z = INVALID_Z_HEIGHT;
}

NiPoint3 PgActor::GetLastFloorPos()
{
	return m_kLastFloorPos;
}

#ifdef PG_SYNC_ENTIRE_TIME
void PgActor::SyncEntireTime(DWORD dwTime)
{
	// 패킷 전송 시간까지 고려함.
	ms_dwSyncTime = dwTime;
	ms_dwLocalSyncTime = BM::GetTime32();
}

DWORD PgActor::GetSynchronizedTime()
{
	// 서버에서의 절대 시간을 리턴한다. 
	// TODO : PgActor말고 다른 곳으로 빼자.
	DWORD dwNow = BM::GetTime32();
	PG_ASSERT_LOG(dwNow >= ms_dwLocalSyncTime);
	DWORD dwElapsedTime = dwNow - ms_dwLocalSyncTime;
	return ms_dwSyncTime + dwElapsedTime;
}

DWORD PgActor::GetLastSentTime()
{
	return ms_dwLastSentTime;
}

void PgActor::SetLastSentTime(DWORD dwLastSentTime)
{
	ms_dwLastSentTime = dwLastSentTime;
}

void PgActor::SetAverageLatency(DWORD dwRecentLatency)
{
//	WriteToConsole("[PgRemoteManager] Before Latency : (%u)\t", ms_dwAverageLatency);
	if(ms_dwAverageLatency == 0)
	{
		ms_dwAverageLatency = dwRecentLatency;
	}
	else
	{
		// 아래와 같이 해서, 최근 수치에 더 가중치를 먹인다.
		ms_dwAverageLatency = (DWORD)(ms_dwAverageLatency / 2) + (dwRecentLatency / 2);
		//ms_dwAverageLatency = (DWORD)(dwRecentLatency + (ms_dwAverageLatency - dwRecentLatency)/2);
	}
//	WriteToConsole("After Latency : (%u)\n", ms_dwAverageLatency);
}

DWORD PgActor::GetAverageLatency()
{
	return ms_dwAverageLatency;
}
#endif

void PgActor::PrintItemInfo()
{
#ifndef EXTERNAL_RELEASE
	for(PartsAttachInfo::iterator itr = m_kPartsAttachInfo.begin();
		itr != m_kPartsAttachInfo.end();
		++itr)
	{
		itr->second->PrintItemInfo();
	}
#endif
}

bool PgActor::checkVisible()
{
#define NORMALIZED_EDGE_BUFFER	0.05f
	bool m_bVisible = false;
	m_eInvisibleGrade = PgActor::INVISIBLE_FARFAR;

	if (IsMyActor() || (GetPilot() && GetPilot()->GetUnit() && GetPilot()->GetUnit()->UnitType() == UT_BOSSMONSTER))
	{
		m_bVisible = true;
		m_eInvisibleGrade = PgActor::VISIBLE;
		return true;
	}


	if (GetWorld())
	{
		NiCamera *pCamera = GetWorld()->m_kCameraMan.GetCamera();
		if (pCamera)
		{
			if (g_bUseVariableActorUpdate == false)
			{
				m_bVisible = true;
				m_eInvisibleGrade = PgActor::VISIBLE;
				return true;
			}

			NiPoint3 pos = GetPosition();
			if (pCamera->WorldPtToScreenPt3(pos, m_kNormalizedActorPosByCamera.x, m_kNormalizedActorPosByCamera.y, m_kNormalizedActorPosByCamera.z, 0.001f))
			{
				m_bVisible = true;
				m_eInvisibleGrade = PgActor::VISIBLE;
				
				if (m_kNormalizedActorPosByCamera.z >= 0.90f) // z가 클수록 멀리 있는 것이다.
				{
					m_eInvisibleGrade = PgActor::INVISIBLE_FAR;
				}
				else if (m_kNormalizedActorPosByCamera.z >= 0.80f)
				{
					m_eInvisibleGrade = PgActor::INVISIBLE_MIDDLE;
				}
			}
			else
			{
				// 좌우 여유 공간을 좀 둔다.
				// z의 경우는 고려안해도 될 것 같다.
				if (m_kNormalizedActorPosByCamera.x >= -NORMALIZED_EDGE_BUFFER && m_kNormalizedActorPosByCamera.x <= NORMALIZED_EDGE_BUFFER + 1.0f
					&& m_kNormalizedActorPosByCamera.y >= -NORMALIZED_EDGE_BUFFER && m_kNormalizedActorPosByCamera.y <= NORMALIZED_EDGE_BUFFER + 1.0f)
				{
					m_bVisible = true;
				}
				else
				{
					m_bVisible = false;
				}

				if (m_kNormalizedActorPosByCamera.x > -0.2f && m_kNormalizedActorPosByCamera.x < 1.2f
					&& m_kNormalizedActorPosByCamera.y > -0.2f && m_kNormalizedActorPosByCamera.y < 1.2f)
				{
					m_eInvisibleGrade = PgActor::INVISIBLE_NEAR;
				}
				else if (m_kNormalizedActorPosByCamera.x > -0.5f && m_kNormalizedActorPosByCamera.x < 1.5f
					&& m_kNormalizedActorPosByCamera.y > -0.5f && m_kNormalizedActorPosByCamera.y < 1.5f)
				{
					if (m_kNormalizedActorPosByCamera.z >= 0.70f)
						m_eInvisibleGrade = PgActor::INVISIBLE_FAR;
					else
						m_eInvisibleGrade = PgActor::INVISIBLE_MIDDLE;
				}
				else if (m_kNormalizedActorPosByCamera.x > -1.0f && m_kNormalizedActorPosByCamera.x < 2.0f
					&& m_kNormalizedActorPosByCamera.y > -1.0f && m_kNormalizedActorPosByCamera.y < 2.0f)
				{
					if (m_kNormalizedActorPosByCamera.z >= 0.50f)
						m_eInvisibleGrade = PgActor::INVISIBLE_FARFAR;
					else
						m_eInvisibleGrade = PgActor::INVISIBLE_FAR;
				}
				else
				{
					m_eInvisibleGrade = PgActor::INVISIBLE_FARFAR;
				}
			}
		}
	}

	int iLODCount = GetLODCount();
	if (iLODCount > 0 && GetUseLOD())
	{
		switch(m_eInvisibleGrade)
		{
		case VISIBLE:
		case INVISIBLE_NEAR:
			SetLOD(0);
			break;
		case INVISIBLE_MIDDLE:
			SetLOD(NiMin(1, iLODCount));
			break;
		case INVISIBLE_FAR:
		case INVISIBLE_FARFAR:
		default:
			SetLOD(NiMin(2, iLODCount));
			break;
		}
	}
	else
	{
		if (GetCurrentLOD() != 0)
		{
			SetLOD(0);
		}
	}

	return m_bVisible;
}

void PgActor::RestoreLockBidirection()
{
	if(!m_pkPilot || !g_pkWorld)
	{
		return;
	}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return;
	}

	EUnitType eUnitType = pkUnit->UnitType();
	unsigned int uiBiDirection = g_pkWorld->GetLockBidirection();
	LockBidirection((uiBiDirection & eUnitType) != eUnitType);
}
void PgActor::ResetActiveGrp()						
{
	m_uiActiveGrp = DEFAULT_ACTIVE_GRP;
}

void PgActor::SetUpdatePhysXFrameTime(float fFrameTime)
{
	m_fSpecifiedFrameTime = fFrameTime;
}

//-----------------------------------------------------------------------
// PgActionEntity
//-----------------------------------------------------------------------
PgActionEntity::PgActionEntity(PgAction *pkAction, BYTE byDirection) : 
	m_pkAction(pkAction), 
	m_byDirection(byDirection),
	m_dwDirectionTerm(0)
{
	m_kDirectionStartPos.x = 0.0f;
	m_kDirectionStartPos.y = 0.0f;
	m_kDirectionStartPos.z = 0.0f;
}
	
PgAction *PgActionEntity::GetAction()	const
{
	return m_pkAction;
}

BYTE PgActionEntity::GetDirection()	
{
	return m_byDirection;
}

PgActionEntity	PgActionEntity::CreateCopy()	
{
	PgAction	*pkNewAction = NULL;
	if(m_pkAction)
	{
		pkNewAction = g_kActionPool.CreateAction(m_pkAction->GetID().c_str(),true);
		pkNewAction->CopyFrom(m_pkAction);
	}

	PgActionEntity	kCopy(pkNewAction,GetDirection());

	kCopy.SetDirectionTerm(m_dwDirectionTerm);
	kCopy.SetDirectionStartPos(GetDirectionStartPos());

	return	kCopy;
}

void PgActionEntity::SetDirectionTerm(DWORD dwTerm)
{
	m_dwDirectionTerm = dwTerm;
}

void PgActionEntity::SetDirectionStartPos(NiPoint3 &rkPos)
{
	m_kDirectionStartPos = rkPos;
}

NiPoint3 &PgActionEntity::GetDirectionStartPos()
{
	return m_kDirectionStartPos;
}

DWORD PgActionEntity::GetActionTerm()
{
	if(m_pkAction)
	{
		return m_pkAction->GetActionTerm();
	}
	else
	{
		return m_dwDirectionTerm;
	}
}



//--------------------------------------------------------------------------
// PgActionEntity
//--------------------------------------------------------------------------
//PgActionEntity::PgActionEntity(PgAction *pkAction) :
//	m_pkAction(pkAction)
//{
//}
//
//PgActionEntity::~PgActionEntity()
//{
//}
//	
//void PgActionEntity::ProcessAction(PgActor *pkActor)
//{
//	pkActor->ProcessAction(m_pkAction);
//}
//
//DWORD PgActionEntity::GetActionTerm()
//{
//	return pkAction->GetActionTerm();
//}
//
////--------------------------------------------------------------------------
//// PgDirectionEntity
////--------------------------------------------------------------------------
//PgDirectionEntity::PgDirectionEntity(BYTE byDirection, DWORD dwActionTerm) :
//	m_byDirection(byDirection),
//	m_dwActionTerm(dwActionTerm)
//{
//}
//
//PgDirectionEntity::~PgDirectionEntity()
//{
//}
//
//
//void PgDirectionEntity::ProcessAction(PgActor *pkActor)
//{
//	pkActor->SetDirection(m_byDirection);
//}
//
//DWORD PgDirectionEntity::GetActionTerm()
//{
//	return m_dwActionTerm;
//}

//--------------------------------------------------------------------------
// PgActionSay
//--------------------------------------------------------------------------
void PgActionSay::Clear()
{
	m_kIdleSayList.clear();
	m_kClickSayList.clear();
	m_kTalkSayList.clear();
	m_kWarningList.clear();
	m_iTextDialogsID = 0;

	m_pkCurSayItem = NULL;
	m_kCurSayStatus = SAS_Delay;
	
	UpTime(static_cast<float>(BM::Rand_Unit())*5.0f + 1.0f);
}

bool PgActionSay::AddSay(const SSayItem &rkItem)
{
	switch(rkItem.iActionType)
	{
	case SAT_Idle:
		{
			m_kIdleSayList.push_back(rkItem);
		}break;
	case SAT_Click:
		{
			m_kClickSayList.push_back(rkItem);
		}break;
	case SAT_Talk:
		{
			m_kTalkSayList.push_back(rkItem);
		}break;
	case SAT_Warning:
		{
			m_kWarningList.push_back(rkItem);
		}break;
	case SAT_TextDialogs:
		{
			m_iTextDialogsID = rkItem.iTTW;
		}break;
	default:
		return false;
		break;
	}
	return true;
}

bool PgActionSay::GetCur(SSayItem &kItem) const
{
	if( m_pkCurSayItem )
	{
		kItem = *m_pkCurSayItem;
		return true;
	}
	return false;
}

bool PgActionSay::GetRandomSay(ContSayItem const & rkCont, SSayItem const *pkPrev, const SSayItem* &pkOut)const
{
	if( rkCont.empty() )
	{
		return false;
	}

	ContSayItem kSayVec(rkCont.begin(), rkCont.end());
	SayFilter(kSayVec);

	if( kSayVec.empty() )
	{
		return false;
	}

	size_t const iMin = 0;
	size_t const iMax = kSayVec.size()-1;
	size_t iCur = iMin;
	if(iMin != iMax)//같으면 1개
	{
		iCur = BM::Rand_Index(kSayVec.size());
		if( pkPrev
		&&	*pkPrev == kSayVec.at(iCur) )
		{
			if(rkCont.at(iMin) == *pkPrev)//이전과 Min이 같은지 확인
			{
				iCur = iMax;
			}
			else
			{
				iCur = iMin;
			}
		}
	}
	if( iMin <= iCur
	&&	iMax >= iCur )//iCur가 배열 범위면
	{
		pkOut = &rkCont.at(iCur);//성공
		return true;
	}
	return false;
}


ESayActionStatus PgActionSay::Update(float fAccumTime, float fFrameTime)
{
	ESayActionStatus eRet = SAS_None;
	//SAT_None	: 랜덤으로 하나 뽑는다
	//SAT_Idle	: UpTime만큼 기다린다.
	//SAT_Delay	: Delay만큼 기다린다.
	if(m_kIdleSayList.size() > 0)
	{
		UpTime(UpTime() - fFrameTime);
		//Idle 만 여기서 처리한다.
		switch(CurSayStatus())
		{
		case SAS_None:
			{
				const SSayItem *pkItem = NULL;
				bool const bFindSay = GetRandomSay(m_kIdleSayList, m_pkCurSayItem, pkItem);
				if( bFindSay )
				{
					UpTime(pkItem->fUpTime);
					m_pkCurSayItem = pkItem;
					eRet = m_kCurSayStatus = SAS_Run; //여기서 말풍선 띄운다
				}
				else
				{
					//실패할 경우엔 50초 만큼 침묵
					m_pkCurSayItem = NULL;
					UpTime(5000.f);
					eRet = m_kCurSayStatus = SAS_Delay;
				}
			}break;
		case SAS_Run:
			{
				eRet = m_kCurSayStatus = SAS_Wait; //실행
			}break;
		case SAS_Wait:
			{
				if(UpTime() < 0.f)
				{
					if(m_pkCurSayItem) UpTime(m_pkCurSayItem->fDelay);//Delay 시간 설정
					eRet = m_kCurSayStatus = SAS_Delay; //실행 중 대기
				}
			}break;
		case SAS_Delay:
			{
				if(UpTime() < 0.f)
				{
					UpTime(0.f);
					eRet = m_kCurSayStatus = SAS_None; //이전과 다음 실행의 사이 대기
				}
			}break;
		default:
			{
			}break;
		}
	}

	return eRet;
	//SAT_Idle에 CurSayItem()의 글을 띄우고
	//SAT_None/SAT_Delay 에는 아무것도 안한다
}

ESayActionStatus PgActionSay::OnClick()
{
	ESayActionStatus eRet = SAS_None;
	//모든지 캔슬 할수 있다.
	const SSayItem* pkItem = NULL;
	if(GetRandomSay(m_kClickSayList, m_pkCurSayItem, pkItem))
	{
		m_pkCurSayItem = pkItem;
		UpTime(pkItem->fUpTime);
		eRet = m_kCurSayStatus = SAS_Run; //띄워라
	}
	return eRet;
}

ESayActionStatus PgActionSay::OnTalk()
{
	ESayActionStatus eRet = SAS_None;
	const SSayItem* pkItem = NULL;
	if( GetRandomSay(m_kTalkSayList, m_pkCurSayItem, pkItem) )
	{
		m_pkCurSayItem = pkItem;
		UpTime(pkItem->fUpTime);
		eRet = m_kCurSayStatus = SAS_Run; //띄워라
	}
	return eRet;
}

ESayActionStatus PgActionSay::OnWarning()
{
	ESayActionStatus eRet = SAS_None;
	const SSayItem* pkItem = NULL;
	if( GetRandomSay(m_kWarningList, m_pkCurSayItem, pkItem) )
	{
		m_pkCurSayItem = pkItem;
		UpTime(pkItem->fUpTime);
		eRet = m_kCurSayStatus = SAS_Run; //띄워라
	}
	return eRet;
}

void PgActionSay::SayFilter(ContSayItem& rkCont) const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgSayItemChecker kNewChecker(pkPlayer->GetAbil(AT_LEVEL), pkPlayer->GetMyQuest());
		ContSayItem::iterator new_end = std::remove_if(rkCont.begin(), rkCont.end(), kNewChecker);
		rkCont.erase(new_end, rkCont.end());
	}
}

size_t const PgActionSay::TalkCount() const
{
	ContSayItem kSayVec(m_kTalkSayList.begin(), m_kTalkSayList.end());
	SayFilter(kSayVec);
	return kSayVec.size();
}

size_t const PgActionSay::WarningCount() const
{
	ContSayItem kSayVec(m_kWarningList.begin(), m_kWarningList.end());
	SayFilter(kSayVec);
	return kSayVec.size();
}

void PgActor::SetUseSubActorManager(bool bUse)
{ 
	PgIWorldObject::SetUseSubActorManager(bUse); 
	NiActorManager *pkAM = GetActorManager();
	PG_ASSERT_LOG(pkAM);  
	if (pkAM)
	{
		pkAM->SetCallbackObject(m_pkActorCallback); 
	}
}

void PgActor::SetAutoDeleteActorTimer(float fTime)
{
	m_fAutoDeleteActorStartTime = GetWorld()->GetAccumTime();
	m_fAutoDeleteActorTime = fTime;
}

int PgActor::GetNextSetEffectSlotIndex()
{
	++m_kGenerateSetEffectSlotIndex;

	if(m_kGenerateSetEffectSlotIndex > 510000)
	{
		m_kGenerateSetEffectSlotIndex = 500000;
	}

	return m_kGenerateSetEffectSlotIndex;
}

void PgActor::OnChangeInventory()
{
}

void PgActor::onHPChanged()
{
}

bool PgActor::AttachAttackEffect(char const* szActionName, int const iSlot)
{
	if(szActionName)
	{
		PgItemEx* pkItemEx = GetEquippedWeapon();
		if(pkItemEx)
		{
			PgItemEx::SAttackEffect kEffect;
			if(pkItemEx->FindAttackEffect(szActionName, kEffect))
			{
				NiAVObject *pkParticle = g_kParticleMan.GetParticle(kEffect.m_kEffectID.c_str(), PgParticle::O_SCALE,kEffect.m_fScale );
				if(pkParticle)
				{
					if(!AttachTo(0==iSlot ? BM::Rand_Index(100000) : iSlot, kEffect.m_kNodeID.c_str(), pkParticle))
					{
						THREAD_DELETE_PARTICLE(pkParticle);
						return	false;
					}
					return true;
				}
			}
		}
	}
	return false;
}

void PgActor::AttachChild(NiAVObject* pkChild, bool bFirstAvail)
{
	NiNode::AttachChild(pkChild, bFirstAvail);
}

void PgActor::AddTransformEffectAttachInfo(int const iTransformEffectID, PgIWorldObjectBase::stEffectAttachInfo const & kInfo)
{
	CONT_TRANSFORM_EFFECT_ATTACH_INFO::iterator itor = m_kContTransformEffAttachInfo.begin();
	while(m_kContTransformEffAttachInfo.end() != itor)
	{
		if(iTransformEffectID == itor->iTransformEffectID)
		{
			itor->kContInfoList.push_back(kInfo);
			return;
		}
		++itor;
	}
	STransformEffectAttachInfo kTemp;
	kTemp.iTransformEffectID = iTransformEffectID;
	kTemp.kContInfoList.push_back(kInfo);
	m_kContTransformEffAttachInfo.push_back(kTemp);
	return;
}

bool PgActor::RemoveTransformEffectAttachInfo(int const iTransformEffectID)
{
	bool bResult = false;
	CONT_TRANSFORM_EFFECT_ATTACH_INFO::iterator itor = m_kContTransformEffAttachInfo.begin();
	while(m_kContTransformEffAttachInfo.end() != itor)
	{
		if(iTransformEffectID == itor->iTransformEffectID)
		{
			PgIWorldObjectBase::EffectAttachInfoList::iterator itor_Effect = itor->kContInfoList.begin();
			while(itor->kContInfoList.end() != itor_Effect)
			{
				if(DetachFrom(itor_Effect->m_iSlot, true) )
				{
					bResult = true;
					itor_Effect = itor->kContInfoList.erase(itor_Effect);
				}
				else
				{
					++itor_Effect;
				}
			}
		}
		if(itor->kContInfoList.empty())
		{
			itor = m_kContTransformEffAttachInfo.erase(itor);
		}
		else
		{
			++itor;
		}
	}
	return bResult;
}

void PgActor::CullingProcessParticle(NiCamera *pkCamera, NiVisibleArray *pkArray, PgRenderer *pkRenderer)
{
	if ( !pkCamera || !pkArray )
        return;
	if(!pkRenderer)
		pkRenderer = PgRenderer::GetPgRenderer();

	PgParticle	*pkParticle = NULL;
	for ( AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle,itr->second);
		if(pkParticle && pkParticle->GetZTest() == true)
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->CullingProcess_Deprecated( pkCamera, pkParticle, pkArray, false );
			pkParticle->SetAppCulled(true);
		}
	}
}

void PgActor::SetNodeHide(char const* strNodeName, bool bHide)
{
	NiAVObjectPtr spNode = GetCharRoot()->GetObjectByName(strNodeName);
	if(spNode != NULL)
	{
		spNode->SetAppCulled(bHide);
	}
}

void PgActor::SetNodeAlpha(char const* strNodeName, float fAlpha)
{
	NiAVObjectPtr spNode = GetCharRoot()->GetObjectByName(strNodeName);
	if(spNode != NULL)
	{
		SetNodeAlphaRecursive(spNode, fAlpha);
	}
}

void PgActor::SetNodeAlphaRecursive(NiAVObject* pkObject, float fAlpha)
{
	if(NiIsKindOf(NiNode,pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
		if(pkNode)
		{
			int	iTotalChild = pkNode->GetArrayCount();
			NiAVObject	*pkChild = NULL;
			for(int i=0;i<iTotalChild;++i)
			{
				pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					SetNodeAlphaRecursive(pkChild, fAlpha);
				}
			}
		}
	}
	else if(NiIsKindOf(NiGeometry,pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkObject);
		if(pkGeom)
		{
			NiMaterialProperty	*pkMat = pkGeom->GetPropertyState()->GetMaterial();
			pkMat->SetAlpha(fAlpha);

			NiAlphaProperty	*pkAlpha = pkGeom->GetPropertyState()->GetAlpha();
			pkAlpha->SetAlphaBlending(true);
		}
	}
}

void PgActor::SetNodeAlphaChange(char const* strNodeName, float fAlphaStart, float fAlphaEnd, float fChangeTime)
{
	NiAVObject* pkObject = GetCharRoot()->GetObjectByName(strNodeName);
	if( pkObject )
	{
		m_pAlphaNode = pkObject;
		m_fStartAlpha = fAlphaStart;
		m_fEndAlpha = fAlphaEnd;
		m_ulAlphaStartTime = BM::GetTime32();
		m_ulAlphaChangeTime = static_cast<unsigned long>(fChangeTime * 1000);
	}
}

void PgActor::UpdateNodeAlpha()
{
	if (m_ulAlphaChangeTime>0)
	{
		unsigned long ulElapsedTime = BM::GetTime32() - m_ulAlphaStartTime;
		float fRate = static_cast<float>(ulElapsedTime) / static_cast<float>(m_ulAlphaChangeTime);
		if(fRate>1)
		{
			fRate = 1;
		}
		else if(fRate<0)
		{
			fRate = 0;
		}

		float fNewAlpha = m_fStartAlpha + (m_fEndAlpha-m_fStartAlpha)*fRate;
		SetNodeAlphaRecursive(m_pAlphaNode, fNewAlpha);
		if(fRate == 1)
		{
			m_ulAlphaChangeTime = 0;
			m_pAlphaNode = NULL;
		}
	}
}

int GetTotalSummonedSupply(CUnit const* pkCaller)
{
	int iCount = 0;
	if(NULL==pkCaller)
	{
        return iCount;
	}

	PgPilot* pkPilot = NULL;
	PgSummoned * pkSummoned = NULL;
	VEC_SUMMONUNIT const& kContSummonUnit = pkCaller->GetSummonUnit();
	for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
	{
		pkPilot = g_kPilotMan.FindPilot((*c_it).kGuid);
		if(pkPilot && pkPilot->GetUnit() && pkPilot->GetUnit()->IsUnitType(UT_SUMMONED) && pkPilot->GetUnit()->GetAbil(AT_HP)
		&& (pkSummoned = dynamic_cast<PgSummoned*>(pkPilot->GetUnit())) )
		{
			iCount += pkSummoned->Supply();
		}
	}

	return iCount;
}

int GetTotalSummonedCount(CUnit const* pkCaller)
{
	int iCount = 0;
	if(NULL==pkCaller)
	{
        return iCount;
	}

	PgPilot* pkPilot = NULL;
	PgSummoned * pkSummoned = NULL;
	VEC_SUMMONUNIT const& kContSummonUnit = pkCaller->GetSummonUnit();
	for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
	{
		pkPilot = g_kPilotMan.FindPilot((*c_it).kGuid);
		if(pkPilot && pkPilot->GetUnit() && pkPilot->GetUnit()->IsUnitType(UT_SUMMONED) && pkPilot->GetUnit()->GetAbil(AT_HP))
		{
			++iCount;
		}
	}

	return iCount;
}

bool CheckHPMPForAction(CSkillDef const* pkSkillDef, CUnit const* pkUnit, bool const bShowFailMsg, bool const bShowNILog)
{
	int const iNeedSupply = pkSkillDef->GetAbil(AT_CREATE_SUMMONED_SUPPLY);
	if(iNeedSupply)
	{
		int const iCount = GetTotalSummonedSupply(pkUnit);
		int const iMaxSupply = pkUnit->GetAbil(AT_C_SUMMONED_MAX_SUPPLY);
		if(iMaxSupply < iCount+iNeedSupply)
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(169)), 2, true);
			}
			return false;
		}

		bool const bUniqueClass = (pkSkillDef->GetAbil(AT_CREATE_UNIQUE_SUMMONED)>0);
		if(bUniqueClass)
		{
			if( pkUnit->IsSummonUnitClass( pkSkillDef->GetAbil(AT_CLASS) ) ) 
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(792102)), 2, true);
				}
				return false;
			}
		}
	}

	//	마나량 체크
	int iNeedHP = pkSkillDef->GetAbil(AT_NEED_HP);
	int iNeedMP = pkSkillDef->GetAbil(AT_NEED_MP);
	int const iNeedHPPer = pkUnit->GetAbil(AT_ADD_R_NEED_HP); // 소모HP의 %
	int const iNeedMPPer = pkUnit->GetAbil(AT_ADD_R_NEED_MP); // 소모MP의 %
	int const iNeedHPPer2 = pkUnit->GetAbil(AT_ADD_R_NEED_HP_2); // 소모HP의 감소%
	int const iNeedMPPer2 = pkUnit->GetAbil(AT_ADD_R_NEED_MP_2); // 소모MP의 감소%

	int const iMaxHP = pkUnit->GetAbil(AT_C_MAX_HP);
	int const iMaxMP = pkUnit->GetAbil(AT_C_MAX_MP);
	int const iNeedMaxHPPer = pkSkillDef->GetAbil(AT_NEED_MAX_R_HP);
	int const iNeedMaxMPPer = pkSkillDef->GetAbil(AT_NEED_MAX_R_MP);

	//스킬 사용시 MaxHP의 %를 소모 시키는 경우 iNeedHp는 새로 세팅 된다.
	if(0 < iNeedMaxHPPer)
	{
		iNeedHP = static_cast<int>(iMaxHP * (static_cast<float>(iNeedMaxHPPer) / ABILITY_RATE_VALUE_FLOAT));			
	}
	//스킬 사용시 MaxMP의 %를 소모 시키는 경우 iNeedMp는 새로 세팅 된다.
	if(0 < iNeedMaxMPPer)
	{
		iNeedMP = static_cast<int>(iMaxMP * (static_cast<float>(iNeedMaxMPPer) / ABILITY_RATE_VALUE_FLOAT));			
	}

	if(0 < iNeedHPPer)
	{
		iNeedHP -= static_cast<int>(iNeedHP * (static_cast<float>((ABILITY_RATE_VALUE - iNeedHPPer) + iNeedHPPer2) / ABILITY_RATE_VALUE_FLOAT));
	}
	if(0 < iNeedMPPer)
	{
		iNeedMP -= static_cast<int>(iNeedMP * (static_cast<float>((ABILITY_RATE_VALUE - iNeedMPPer) + iNeedMPPer2) / ABILITY_RATE_VALUE_FLOAT));
	}

	{
		// 스킬 사용시 필요한 최소 값이 있을 경우 체크한다
		int const iNeedMinValue = pkSkillDef->GetAbil(AT_NEED_MP_MIN_VALUE);
		int const iCurrent = pkUnit->GetAbil(AT_MP);

		if(0 < iNeedMinValue)
		{
			if(iCurrent < iNeedMinValue)
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(35)),2, true);
				}

				if(bShowNILog)
				{
					NILOG(PGLOG_LOG, "[PgActor] NextAction, ManaCheck failed(%d,%d)\n", pkUnit->GetAbil(AT_MP), iNeedMP);
				}

				return false;
			}
		}
	}

	if(pkUnit->GetAbil(AT_MP) < iNeedMP)
	{
		if(bShowFailMsg && (0==pkUnit->GetAbil(AT_AUTO_PET_SKILL)))
		{
			lwAddWarnDataStr(lwWString(TTW(35)),2, true);
		}

		if(bShowNILog)
		{
			NILOG(PGLOG_LOG, "[PgActor] NextAction, ManaCheck failed(%d,%d)\n", pkUnit->GetAbil(AT_MP), iNeedMP);
		}

		return false;
	}
	//	체력 체크

	{
		// 스킬 사용시 필요한 최소 값이 있을 경우 체크한다
		int const iNeedMinValue = pkSkillDef->GetAbil(AT_NEED_HP_MIN_VALUE);
		int const iCurrent = pkUnit->GetAbil(AT_HP);

		if(0 < iNeedMinValue)
		{
			if(iCurrent < iNeedMinValue)
			{
				if(bShowFailMsg)
				{
					lwAddWarnDataStr(lwWString(TTW(62)),2, true);
				}

				if(bShowNILog)
				{
					NILOG(PGLOG_LOG, "[PgActor] NextAction, HPCheck failed(%d,%d)\n", pkUnit->GetAbil(AT_HP), iNeedHP);
				}

				return false;
			}
		}
	}

	if(pkUnit->GetAbil(AT_HP) < iNeedHP)
	{
		if(bShowFailMsg)
		{
			lwAddWarnDataStr(lwWString(TTW(62)),2, true);
		}

		if(bShowNILog)
		{
			NILOG(PGLOG_LOG, "[PgActor] NextAction, HPCheck failed(%d,%d)\n", pkUnit->GetAbil(AT_HP), iNeedHP);
		}

		return false;
	}

	// 각성 게이지
	int iNeedAwake = pkSkillDef->GetAbil(AT_NEED_AWAKE); // 각성기 게이지 절대값 감소
	int const iNeedAwakePer = pkSkillDef->GetAbil(AT_NEED_MAX_R_AWAKE); // 각성기 게이지를 전체의 %로 소모
	if(0 < iNeedAwakePer)
	{
		iNeedAwake = static_cast<int>(AWAKE_VALUE_MAX * (static_cast<float>(iNeedAwakePer) / ABILITY_RATE_VALUE_FLOAT));
	}

	// 각성기 게이지 증감 어빌
	int const iAddNeedAwakePer = pkUnit->GetAbil(AT_ADD_NEED_R_AWAKE);
	if(iAddNeedAwakePer)
	{
		iNeedAwake -= static_cast<int>(iNeedAwake * (static_cast<float>(iAddNeedAwakePer) / ABILITY_RATE_VALUE_FLOAT));
	}

	// 남아있는 모든 각성기 게이지 소모하는 어빌
	int const iAllNeedAwake = pkSkillDef->GetAbil(AT_ALL_NEED_AWAKE); 
	if(0 < iAllNeedAwake)
	{
		//남아 있는 모든 각성기 게이지를 소모 한다.
		iNeedAwake = pkUnit->GetAbil(AT_AWAKE_VALUE);
	}

	int const iCurrent = pkUnit->GetAbil(AT_AWAKE_VALUE);
	if (0 < iNeedAwake)
	{
		if (iCurrent < iNeedAwake)
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(41003)),2, true);
			}
			return false;
		}

		// 각성 시스템이 활성화 되어있지 않을 경우
		if (!pkUnit->GetAbil(AT_ENABLE_AWAKE_SKILL))
		{
			return false;
		}	
	}
	
	if(0 < iAllNeedAwake)
	{
		if(0 == iCurrent)
		{
			if(bShowFailMsg)
			{
				lwAddWarnDataStr(lwWString(TTW(41003)),2, true);
			}
			return false;
		}
	}

	return true;
}

void PgActor::UpdateStatusEffect(float fAccumTime, float fFrameTime)
{
	for(StatusEffectUpdateList::iterator it = m_StatusEffectInstanceListForUpdate.begin(); it != m_StatusEffectInstanceListForUpdate.end(); ++it)
	{
		PgStatusEffectManUtil::SEffectUpdateInfo & rkInfo = (*it);
		if(rkInfo.Update(fAccumTime) && rkInfo.m_pkInstance)
		{
			lua_tinker::call<void,lwActor,int, float>((rkInfo.m_pkInstance->GetStatusEffect()->GetScriptName() + "_OnTick").c_str(), lwActor(this), rkInfo.m_pkInstance->GetEffectID(), fAccumTime);
		}
	}
}

PgPOTParticle PgActor::GetPOTParticleInfo() const
{ 
	return m_kPOTParticle; 
}

void PgActor::SetState(EUnitState const eState)
{
	PgPilot* pkPilot = GetPilot();
	if( !pkPilot ){ return; }

	CUnit* pkUnit = GetPilot()->GetUnit();
	if ( !pkUnit ){ return; }

	EUnitState const eOld = pkUnit->GetState();

	switch(eOld)
	{
	case US_SKILL_FIRE:
		{
			if (eState == US_IDLE)
			{
				static int const siDefaultAnimTime = 550;
				int iAnimTime = pkUnit->GetSkill()->GetAbil(AT_ANIMATION_TIME);
				if(iAnimTime == 0)
				{
					iAnimTime = siDefaultAnimTime;
				}
				pkUnit->SetDelay(iAnimTime);
			}
		}break;
	}

	pkUnit->SetState(eState);
}

namespace lwSkillSet
{
	extern int lwGetClassLevel(int const iClass);
}

PgSkillSetAction::PgSkillSetAction() 
: m_byReserveSetNo(0)
, m_eState(ES_NONE)
{}

bool PgSkillSetAction::GetReservedAction(std::wstring & rkNextActionName, PgActor* pkActor, bool const bIgnoreTime)
{
	if( !IsReservedNextAction(bIgnoreTime) )
	{ 
		return false;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	rkNextActionName = kSkillDefMgr.GetActionName(m_kContReserveSkill[0].iSkillNo);
	m_kContReserveSkill.pop_front();
	m_eState = ES_READACTION;
	return true;
}

bool PgSkillSetAction::GetReservedAction(int & iNextActionNo, PgActor* pkActor, bool const bIgnoreTime)
{
	if( !IsReservedNextAction(bIgnoreTime) )
	{ 
		return false;
	}

	iNextActionNo = m_kContReserveSkill[0].iSkillNo;
	m_kContReserveSkill.pop_front();
	m_eState = ES_READACTION;
	return true;
}

bool PgSkillSetAction::IsReservedNextAction(bool const bIgnoreTime)
{
	if(m_kContReserveSkill.empty())
	{
		m_byReserveSetNo = 0;
		return false; 
	}
	if(ES_READACTION==m_eState){ return false; }
	if(!bIgnoreTime && (g_pkApp->GetAccumTime() < m_kContReserveSkill[0].fAccumDelay))
	{
		return false;
	}

	if(0==m_kContReserveSkill[0].iSkillNo)
	{
		m_kContReserveSkill.pop_front();
		return false;
	}

	return true;
}

bool PgSkillSetAction::IsReserveActionEmpty()
{
	if(m_kContReserveSkill.empty())
	{
		return true;
	}
	return false;
}

bool PgSkillSetAction::NextReservedAction(PgActor* pkActor)
{
	if( !pkActor ){ return false; }
	if( m_kContReserveSkill.empty() )
	{
		m_byReserveSetNo = 0;
		return false;
	}
	if( ES_NEXTACTION==m_eState ){ return true; }

	m_kContReserveSkill[0].fAccumDelay = (m_kContReserveSkill[0].byDelay/10.f) + g_pkApp->GetAccumTime();
	m_eState = ES_NEXTACTION;
	return true;
}

bool PgSkillSetAction::DoReservedAction(PgActor* pkActor)
{
	if( !pkActor ){ return false; }
	NextReservedAction(pkActor);

	int iActionNo = 0;
	if( pkActor->SkillSetAction().GetReservedAction(iActionNo, pkActor, g_pkApp->GetAccumTime()) )
	{
		pkActor->ReserveTransitAction(iActionNo);
		return true;
	}
	return false;
}

bool PgSkillSetAction::ReserveAction(BYTE const iSetNo)
{
	if(m_byReserveSetNo){ return false; }

	CONT_USER_SKILLSET::const_iterator c_iter = m_kContSkillSet.find(iSetNo);
	if(c_iter == m_kContSkillSet.end())
	{
		return false;
	}

	CONT_USER_SKILLSET::mapped_type const & kSkillSet = c_iter->second;

	for(int i=0; i<MAX_SKILLSET_GROUP; ++i)
	{
		if(0==kSkillSet.byDelay[i] && 0==kSkillSet.iSkillNo[i])
		{
			continue;
		}
		m_kContReserveSkill.push_back(SReserveInfo(kSkillSet.iSkillNo[i],kSkillSet.byDelay[i]));
	}

	m_byReserveSetNo = iSetNo;
	//m_eState = ES_RESERVEACTION;
	return true;
}

bool PgSkillSetAction::ReserveActionCancel()
{
	if(m_kContReserveSkill.empty())
	{
		return false;
	}

	m_kContReserveSkill.clear();
	m_byReserveSetNo = 0;
	return true;
}

void PgSkillSetAction::ContSkillSet(CONT_USER_SKILLSET const & kContSkillSet)
{
	m_kContSkillSet = kContSkillSet;
}

void PgSkillSetAction::ContBasicSkillSet(CONT_USER_SKILLSET const & kContBasicSkillSet)
{
	m_kContBasicSkillSet = kContBasicSkillSet;
}

CONT_USER_SKILLSET& PgSkillSetAction::GetSkillSet()
{
	return m_kContSkillSet;
}

CONT_USER_SKILLSET& PgSkillSetAction::GetBasicSkillSet()
{
	return m_kContBasicSkillSet;
}

void PgSkillSetAction::ApplyBasicSkillSetToSkillSet()
{
	CONT_USER_SKILLSET::const_iterator iter_Basic = m_kContBasicSkillSet.begin();
	while( m_kContBasicSkillSet.end() != iter_Basic )
	{
		if(m_kContSkillSet.end() != m_kContSkillSet.find( (*iter_Basic).first ) )
		{
			m_kContSkillSet.erase( (*iter_Basic).first );
		}
		m_kContSkillSet.insert( std::make_pair( (*iter_Basic).first, (*iter_Basic).second ) );
		++iter_Basic;
	}
}

bool PgSkillSetAction::GetMaxCoolTime(PgActor * pkActor, int const iSetNo, DWORD & dwMaxRemainTime)
{
	if( !pkActor ){ return false; }

	CONT_USER_SKILLSET::mapped_type const * kContSkillSet = find(iSetNo);
	if( !kContSkillSet){ return false; }

	dwMaxRemainTime = 0;
	PgActor::stSkillCoolTimeInfo const * pkInfo = pkActor->GetSkillCoolTimeInfo();
	if( !pkInfo || pkInfo->m_CoolTimeInfoMap.empty() )
	{ 
		return true;
	}

	for(int i=0;i<MAX_SKILLSET_GROUP;++i)
	{
		int const iSkillNo = kContSkillSet->iSkillNo[i];
		if( iSkillNo<1 ){ continue; }

		PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = pkInfo->m_CoolTimeInfoMap.find(iSkillNo);
		if(itor != pkInfo->m_CoolTimeInfoMap.end())
		{
			DWORD dwTotalTime = itor->second.m_ulTotalCoolTime;
			DWORD dwRemainTime = dwTotalTime - BM::GetTime32() - itor->second.m_ulCoolStartTime;

			if(dwRemainTime > dwMaxRemainTime)
			{
				dwMaxRemainTime = dwRemainTime;
			}
		}
	}
	return true;
}

bool PgSkillSetAction::GetSkillText(int const iSetNo, PgSkillSetAction::CONT_SKILLTEXT & rkContSkillText)
{
	CONT_USER_SKILLSET::const_iterator c_iter = m_kContSkillSet.find(iSetNo);
	if(c_iter==m_kContSkillSet.end())
	{
		return false;
	}

	BM::vstring vStr;
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	for(int i=0; i<MAX_SKILLSET_GROUP; ++i)
	{
		int const iSkillNo = c_iter->second.iSkillNo[i];
		if( !iSkillNo ){ continue; }

		int const iKeySkillNo = g_kSkillTree.GetKeySkillNo(iSkillNo);
		PgSkillTree::stTreeNode *pTreeNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
		CSkillDef const* pSkillDef = NULL;
		CSkillDef const* pSkillDef_Passive = NULL;
		if( pTreeNode )
		{
			if(iKeySkillNo == iSkillNo)
			{
				pSkillDef = pTreeNode->GetSkillDef();	
			}
			else
			{
				pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
			}
		}
		else
		{
			pSkillDef_Passive = kSkillDefMgr.GetDef(iSkillNo);
			int const iNameNo = pSkillDef_Passive->NameNo();
			pTreeNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iNameNo));
			if( pTreeNode )
			{
				pSkillDef_Passive = pTreeNode->GetSkillDef();
			}
			pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		}

		if( !pSkillDef ){ continue; }

		wchar_t const* pName = NULL;
		GetDefString(pSkillDef->NameNo(),pName);
		if(pName)
		{
			vStr = pName;
			if(pSkillDef->m_byLv>0)
			{
				vStr += " ";
				vStr += TTW(224);
				if(pSkillDef_Passive)
				{
					vStr += pSkillDef_Passive->m_byLv;
				}
				else
				{
					vStr += pSkillDef->m_byLv;
				}
			}
		}
		rkContSkillText.push_back(vStr);
	}
	return true;
}

CONT_USER_SKILLSET::mapped_type const * PgSkillSetAction::find(BYTE const bySetNo)
{
	CONT_USER_SKILLSET::const_iterator c_iter = m_kContSkillSet.find(bySetNo);
	if(c_iter == m_kContSkillSet.end())
	{
		return NULL;
	}
	return &c_iter->second;
}

ESkillSetResult PgSkillSetAction::CheckCanSkillSetDoAction(PgPlayer const *pkPlayer, BYTE const iSetNo)
{
	if( !pkPlayer ){ return ESSR_ETC; }

	CONT_USER_SKILLSET::const_iterator cit_SkillSet = m_kContSkillSet.find(iSetNo);
	if(cit_SkillSet == m_kContSkillSet.end()){ return ESSR_NOT_SKILLSET; }

	const CONT_DEFSKILLSET *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if( !pkContDefMap ){ return ESSR_NOT_DEFSKILLSET; }

	CONT_DEFSKILLSET::const_iterator it_defskill = pkContDefMap->find(iSetNo);
	if(it_defskill==pkContDefMap->end()){ return ESSR_NOT_FIND_DEFSKILLSET; }

	ESkillSetResult kResult = ESSR_NONE;
	switch(it_defskill->second.byConditionType)
	{
	case SSCT_CLASS:
		{
			int const iMyClass = lwSkillSet::lwGetClassLevel(pkPlayer->GetAbil(AT_CLASS));
			if(iMyClass < it_defskill->second.byConditionValue)
			{
				kResult = ESSR_NOT_CLASS;
			}
		}break;
	case SSCT_ITEM:
		{
			BYTE byMaxSkillSetNo = 0;
			int iSkillSetUseCnt = pkPlayer->GetAbil(AT_SKILLSET_USE_COUNT);
			VEC_INT kContTmp;
			for(CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->begin();
				c_iter!=pkContDefMap->end(); ++c_iter)
			{
				if(SSCT_ITEM == c_iter->second.byConditionType)
				{
					if(iSkillSetUseCnt > 0)
					{
						kContTmp.push_back(c_iter->second.bySetNo);
					}
					--iSkillSetUseCnt;
				}
			}

			VEC_INT::const_iterator c_it = std::find(kContTmp.begin(),kContTmp.end(),iSetNo);
			if( c_it==kContTmp.end() )
			{
				kResult = ESSR_NOT_USE_ITEM;
			}
		}break;
	case SSCT_LEVEL:
		{
			int const iMyLevel = pkPlayer->GetAbil(AT_LEVEL);
			if(iMyLevel >= it_defskill->second.byConditionValue)
			{
				BYTE byMaxSkillSetNo = 0;
				int iSkillSetUseCnt = pkPlayer->GetAbil(AT_SKILLSET_USE_COUNT);
				VEC_INT kContTmp;
				for(CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->begin();
					0<iSkillSetUseCnt && c_iter!=pkContDefMap->end(); ++c_iter)
				{
					if(SSCT_ITEM == c_iter->second.byConditionType)
					{
						--iSkillSetUseCnt;
					}
					else if(SSCT_LEVEL == c_iter->second.byConditionType)
					{
						kContTmp.push_back(c_iter->second.bySetNo);
					}
				}

				for(VEC_INT::const_iterator c_iter=kContTmp.begin();
					0<iSkillSetUseCnt && c_iter!=kContTmp.end(); ++c_iter)
				{
					if(iSetNo == *c_iter)
					{
						break;
					}
					--iSkillSetUseCnt;
				}

				if(iSkillSetUseCnt < 1)
				{
					kResult = ESSR_NOT_USE_ITEM;
				}
			}
		}break;
	}
	return kResult;
}

//void PgActor::DetachPOTParticle(EInvType kInvType,EEquipPos kItemPos,int iItemNo)
void PgActor::RemoveCompletedItemSet(int const iSetNo )
{
	if(!iSetNo)
	{
		return;
	}
	
	CONT_SET_ITEM_SLOT::iterator set_item_slot_itor = m_kContCurSetNo.find(iSetNo);
	if(m_kContCurSetNo.end() != set_item_slot_itor)
	{
		std::vector<int>::const_iterator index_itor = (*set_item_slot_itor).second.begin();
		while((*set_item_slot_itor).second.end() != index_itor)
		{
			DetachFrom(*index_itor);
			++index_itor;
		}
		m_kContCurSetNo.erase(set_item_slot_itor);

		{// 세트아이템에, 특정시점에만 붙여줄 파티클 정보가
			SPOTParticleInfo kTemp;
			if(g_kItemMan.GetSetItemParticleInfo(iSetNo, kTemp))
			{//있었다면 떼어준다
				m_kPOTParticle.RemoveInfo(static_cast<PgPOTParticle::eAttachPointOfTime>(kTemp.iAttachPointOfTime));
			}
		}

		{// 해체된 세트 아이템
			CONT_ITEM_CHANGE_INFO kContChangeInfo;
			if( g_kItemMan.GetChangeItemInfo( iSetNo, kContChangeInfo ) )
			{
				PgActor::CONT_APPEARANCE_CHANGE_INFO kContCash;
				PgActor::CONT_APPEARANCE_CHANGE_INFO kContNormal;
				CONT_ITEM_CHANGE_INFO::const_iterator kItor = kContChangeInfo.begin();
				while( kContChangeInfo.end() != kItor )
				{
					if(IT_FIT_CASH == kItor->eInvType)
					{
						kContCash.insert( std::make_pair(kItor->eEquipPos, SAppearanceChangeInfo(kItor->eInvType, kItor->iItemNo)) );
					}
					else
					{
						kContNormal.insert( std::make_pair(kItor->eEquipPos, SAppearanceChangeInfo(kItor->eInvType, kItor->iItemNo)) );
					}
					++kItor;
				}
				
				{//캐시
					RemoveCashItemChanger(kContCash);
					PgActor::CONT_APPEARANCE_CHANGE_INFO::iterator kChange_Itor = kContCash.begin();
					while(kContCash.end() != kChange_Itor)
					{
						UnequipItem(kChange_Itor->second.eInvType, static_cast<EEquipPos>(kChange_Itor->first), kChange_Itor->second.iItemNo, PgItemEx::LOAD_TYPE_INSTANT, false, false);
						++kChange_Itor;
					}
				}
				{// 노멀
					RemoveNormalItemChanger(kContNormal);
					PgActor::CONT_APPEARANCE_CHANGE_INFO::iterator kChange_Itor = kContNormal.begin();
					while(kContNormal.end() != kChange_Itor)
					{
						EquipItemByPos(kChange_Itor->second.eInvType, static_cast<EEquipPos>(kChange_Itor->first));
						++kChange_Itor;
					}
				}
			}
		}
	}
}

void PgActor::AddDivideReservedTransit(EItemDivideReservedType const eType, SItemPos const& kItemPos, int const iCalcValue, BM::Stream & kAddonPacket)
{
	PgPilot * pkPilot = GetPilot();
	if( !pkPilot ){ return; }

	CUnit * pkPlayer = GetPilot()->GetUnit();
	if( !pkPlayer ){ return; }

	PgInventory * pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(kItemPos, kItem))
	{
		return;
	}

	BM::Stream kPacket;
	kPacket.Push(eType);
	kPacket.Push(kAddonPacket);

	m_kContItemDivideReserved.insert(std::make_pair(kItem.Guid(),kPacket));
	Send_PT_C_M_REQ_ITEM_DIVIDE(kItemPos, kItem.ItemNo(), kItem.Guid(), iCalcValue);
}

void PgActor::DoDivideReservedTransit(BM::GUID const& kSourceGuid, BM::GUID const& kDivideGuid)
{
	PgPilot * pkPilot = GetPilot();
	if( !pkPilot ){ return ; }

	CUnit * pkPlayer = GetPilot()->GetUnit();
	if( !pkPlayer ){ return ; }

	PgInventory * pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	if( kSourceGuid.IsNull() )
	{ return; }

	CONT_ITEM_DIVIDERESERVED::iterator iter = m_kContItemDivideReserved.find(kSourceGuid);
	if(iter == m_kContItemDivideReserved.end())
	{ return; }

	CONT_ITEM_DIVIDERESERVED::mapped_type & rkPacket = (*iter).second;
	EItemDivideReservedType eType = EIDRT_NONE;
	rkPacket.Pop(eType);
	switch(eType)
	{
	case EIDRT_JOBSKILL3_CREATE:
		{
			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_JL3_ITEM_CREATE");
			if( pWnd )
			{
				int iNeedSlot = 0;
				int iSrcSlot = 0;
				rkPacket.Pop(iNeedSlot);
				rkPacket.Pop(iSrcSlot);
				BM::vstring vStr(L"FRM_NEED_SLOT");
				vStr += iNeedSlot;
				BM::vstring vSlotStr(L"ICON_SRC_SLOT");
				vSlotStr += iSrcSlot;

				XUI::CXUI_Wnd* pSlot = pWnd->GetControl(vStr);
				pSlot = pSlot ? pSlot->GetControl(vSlotStr) : NULL;
				
				PgBase_Item kItem;
				SItemPos kItemPos;
				if( S_OK==pkInv->GetItem(kSourceGuid, kItem, kItemPos) )
				{
					lwJobSkillItem::JS3_AddResItem(pSlot, kItemPos, false);
				}
			}
		}break;
	}

	m_kContItemDivideReserved.erase(iter);
}

int PgActor::GetCurOriginSeqID() const
{
	if(!m_pkPilot)
	{
		return 0;
	}
	
	int iSeqID = PgActorUtil::GetOrigAniSeqID(m_kSeqID, m_pkPilot->GetBaseClassID(), m_byWeaponAnimFolderNumAtActionStart);
	return iSeqID;
}

void PgActor::ViewSelectArrow(bool const bShow)
{
	if(bShow)
	{
		AddNewParticle("eff_magic_select00", EAPS_CUSTOMUI_SUMMONED, "char_root", GetEffectScale());
	}
	else
	{
		DetachFrom(EAPS_CUSTOMUI_SUMMONED);
	}
}

float PgActor::GetAnimationTime(std::string const& strActionName)
{
	PgActionSlot* pkActionSlot = GetActionSlot();
	if(!pkActionSlot)
	{
		return 0;
	}
	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(strActionName, kSeqID,true))
	{
		return 0;
	}

	if(m_pkPilot)
	{
		kSeqID = PgActorUtil::GetCalcAniSeqID(kSeqID, m_pkPilot->GetBaseClassID(), m_byWeaponAnimFolderNumAtActionStart);
	}

	NiActorManager *pkAM = GetActorManager();
	NiControllerSequence *pkController = pkAM->GetSequence(kSeqID);
	if(!pkController)
	{
		return 0;
	}
	float fTime = pkController->GetEndKeyTime() - pkController->GetBeginKeyTime();

	return (fTime > 0) ? fTime : 0;
}

float PgActor::GetAnimationSpeed(std::string const& strActionName)
{
	PgActionSlot* pkActionSlot = GetActionSlot();
	if(!pkActionSlot)
	{
		return 0;
	}
	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(strActionName, kSeqID,true))
	{
		return 0;
	}

	if(m_pkPilot)
	{
		kSeqID = PgActorUtil::GetCalcAniSeqID(kSeqID, m_pkPilot->GetBaseClassID(), m_byWeaponAnimFolderNumAtActionStart);
	}

	NiActorManager *pkAM = GetActorManager();
	NiControllerSequence *pkController = pkAM->GetSequence(kSeqID);
	if(!pkController)
	{
		return 0;
	}
	return pkController->GetFrequency();
}

void PgActor::SetLeaveExpedition(bool const& bValue)
{
	m_bLeaveExpedition = bValue;
}

bool PgActor::CheckOutLobby(PgTrigger * pTrigger)
{
	if(!g_pkWorld)
	{
		return false;
	}
	if ( false == GetLeaveExpedition() )	// 현재 상태가 false일 때
	{
		if( true == PgClientExpeditionUtil::IsInExpedition() )	// 원정대에 속해 있으면
		{
			if( g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_LOBBY) )	// 원정대 로비가 맞는지 비교한다.
			{
				SetCurrentTrigger(pTrigger);	// 현재 트리거를 저장하고 팝업창을 띄운다.
				CallYesNoMsgBox(TTW(720011), BM::GUID::NullData(), MBT_CONFIRM_LEAVE_EXPEDITION_LOBBY);
				return true;
			}
		}
	}
	SetLeaveExpedition(false);
	return false;
}

void PgActor::AddCompletedItemSet(int const iSetNo)
{
	std::vector<int> kContTempSlot;
	CONT_SET_ITEM_SLOT::iterator set_item_slot_itor = m_kContCurSetNo.find(iSetNo);
	if(m_kContCurSetNo.end() == set_item_slot_itor)
	{//세트 아이템에 해당되는 이펙트를 얻어옴.
		SSetItemEffectIndexInfo const *pkSetEffectInfo = g_kItemMan.GetSetItemEffectInfo(iSetNo);
		if(pkSetEffectInfo)
		{
			SSetItemEffectIndexInfo::CONT_SET_EFFECT_INFO::const_iterator effect_itor = pkSetEffectInfo->m_kEffect.begin();
			while(pkSetEffectInfo->m_kEffect.end() != effect_itor)
			{
				int index = GetNextSetEffectSlotIndex();
				NiAVObject *pkParticle = g_kParticleMan.GetParticle(MB((*effect_itor).m_strEffectID.c_str()), PgParticle::O_SCALE,GetEffectScale());

				if(!AttachTo(index, MB((*effect_itor).m_strAttachNode.c_str()), pkParticle))
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
				kContTempSlot.push_back(index);

				++effect_itor;
			}
			//! 현재 장착중인 셋트 번호와 셋트 이펙트의 인덱스를 저장
			m_kContCurSetNo.insert(std::make_pair(iSetNo, kContTempSlot));
		}
		
		SPOTParticleInfo kTemp;
		if( g_kItemMan.GetSetItemParticleInfo(iSetNo, kTemp) )
		{// 특정 시점 파티클이 존재한다면, 해당 관리 객체에 넣어 주고
			m_kPOTParticle.AddInfo(kTemp);
		}

		{// 세트 아이템 완성시 외관 변경 아이템 추가
			CONT_ITEM_CHANGE_INFO kContChangeInfo;
			if( g_kItemMan.GetChangeItemInfo( iSetNo, kContChangeInfo ) )
			{
				PgActor::CONT_APPEARANCE_CHANGE_INFO kContCash;
				PgActor::CONT_APPEARANCE_CHANGE_INFO kContNormal;
				CONT_ITEM_CHANGE_INFO::const_iterator kItor = kContChangeInfo.begin();
				while( kContChangeInfo.end() != kItor )
				{
					if(IT_FIT_CASH == kItor->eInvType)
					{
						kContCash.insert( std::make_pair(kItor->eEquipPos, SAppearanceChangeInfo(kItor->eInvType, kItor->iItemNo)) );
					}
					else
					{
						kContNormal.insert( std::make_pair(kItor->eEquipPos, SAppearanceChangeInfo(kItor->eInvType, kItor->iItemNo)) );
					}
					++kItor;
				}
				
				{// 캐시
					AddCashItemChanger(kContCash);
					PgActor::CONT_APPEARANCE_CHANGE_INFO::iterator kChange_Itor = kContCash.begin();
					while(kContCash.end() != kChange_Itor)
					{
						EquipItemByPos(kChange_Itor->second.eInvType, static_cast<EEquipPos>(kChange_Itor->first));
						++kChange_Itor;
					}
				}
				{// 노멀
					AddNormalItemChanger(kContNormal);
					PgActor::CONT_APPEARANCE_CHANGE_INFO::iterator kChange_Itor = kContNormal.begin();
					while(kContNormal.end() != kChange_Itor)
					{
						EquipItemByPos(kChange_Itor->second.eInvType, static_cast<EEquipPos>(kChange_Itor->first));
						++kChange_Itor;
					}
				}
			}
		}
	}
}

void PgActor::SetEffectCountDown(int const iEffectNo, int const iCountDown)
{
	m_kEffectCountDown.iEffectNo = iEffectNo;
	m_kEffectCountDown.fCountDown = iCountDown / 1000;
}

WORD PgActor::GetEffectCountDownSec()const
{
	return m_kEffectCountDown.fCountDown > 0 ? static_cast<WORD>(m_kEffectCountDown.fCountDown) : 0;
}

void PgActor::UpdateEffectCountDonw(float const fFrameTime)
{
	if (m_kEffectCountDown.fCountDown>=0)
	{
		WORD const wOldCountDown = GetEffectCountDownSec();
		float const fOldCountDown = m_kEffectCountDown.fCountDown;
		m_kEffectCountDown.fCountDown -= fFrameTime;
		if(wOldCountDown != GetEffectCountDownSec())
		{
			UpdateName();
		}
	}
}

void PgActor::DelEffectCountDown(int const iEffectNo)
{
	if(0<iEffectNo && iEffectNo==m_kEffectCountDown.iEffectNo)
	{
		m_kEffectCountDown.iEffectNo = 0;
		m_kEffectCountDown.fCountDown = -1.f;
		UpdateName();
	}
}

void PgActor::PreLoadAllAnimation()
{// 미리 애니메이션을 한번에 읽어온다
//  애니메이션 슬롯 정보가 모두 읽어진 상태에서(actor의 xml 파싱이 모두 끝난 시점에서 사용해야 하며
//  현재 플레이어만 읽어오고 있음.
	PgActionSlot* pkSlot = GetActionSlot();
	if(!pkSlot)
	{
		return;
	}
	NiActorManager *pkAM = GetActorManager();
	if(!pkAM)
	{
		return;
	}

	PgActionSlot::AnimationContainer const& rkCont = pkSlot->GetAnimationCont();
	PgActionSlot::AnimationContainer::const_iterator kItor;
	for(kItor = rkCont.begin(); rkCont.end() != kItor; ++kItor)
	{
		PgActionSlot::SequenceContainer const& rkSeqCont = kItor->second;
		for(PgActionSlot::SequenceContainer::const_iterator kSeqItor = rkSeqCont.begin();
			rkSeqCont.end() != kSeqItor;
			++kSeqItor
			)
		{
			NiActorManager::SequenceID const& rkSeqID = (*kSeqItor).first;
			pkAM->GetSequence( rkSeqID );
		}
	}
	
}

void PgActor::Update_IsAbleSlide(float const fAccumTime, float const fFrameTime, bool& bDoNotSlide)
{
	if( IgnoreSlide() )
	{
		bDoNotSlide = true;
		return;
	}

	if(GetFreeMove() == false)
	{
		if(GetSlide())
		{
			float fTime = (fAccumTime-GetSlideStartTime());
			float fAccel = -GetGravity();
			float fSpeed = std::min(fAccel * fTime,100.0f);

			unsigned int uiActiveGroup = m_uiActiveGrp;

			NxU32 collisionFlagsabs = 0;
			m_pkController->move(m_kSlideVector * fSpeed * fFrameTime, uiActiveGroup, 0.000001f, collisionFlagsabs, 1.0f);

			if(collisionFlagsabs & NXCC_COLLISION_SIDES)	
			{//	this actor hit a wall on the side while sliding and cannot slide anymore, so we stop the actor sliding.
				bDoNotSlide = true;

				SetSlide(false);
				StopJump();
			}
		}
	}
}

void PgActor::InitControllerShapeHit()
{// m_kControllerShapeHit 중요한 멤버의 초기화를 외부로 빼고 싶지 않아서 내부 함수로 만듬
	m_kControllerShapeHit.controller = NULL;
	m_kControllerShapeHit.shape = NULL;
	m_kControllerShapeHit.worldPos.zero();
	m_kControllerShapeHit.worldNormal.zero();
	m_kControllerShapeHit.id = 0;
	m_kControllerShapeHit.dir.zero();
	m_kControllerShapeHit.length = 0.0f;
}

void PgActor::SetComboCharge(float const fChargingTime)
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_COMBO_CHARGE_GAUGE"));
	if(!pkWnd)
	{
		pkWnd = XUIMgr.Call(_T("FRM_COMBO_CHARGE_GAUGE"));
	}
	if(!pkWnd)
	{
		return;
	}
	CXUI_Wnd*		pkBGBar = pkWnd->GetControl(_T("BG_BAR"));
	if( !pkBGBar )
	{
		return;
	}
	CXUI_AniBar*	pkAniBar = (CXUI_AniBar*)pkBGBar->GetControl(_T("ANIBAR"));
	if( !pkAniBar )
	{
		return;
	}
	lwUIWnd	kAniBarWnd(pkAniBar);
	kAniBarWnd.SetStartTime( static_cast<int>(fChargingTime) );
}
void	PgActor::CallComboCharge()
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_COMBO_CHARGE_GAUGE"));
	if(!pkWnd)
	{
		pkWnd = XUIMgr.Call(_T("FRM_COMBO_CHARGE_GAUGE"));
	}
	if(!pkWnd)
	{
		return;
	}
	CXUI_Wnd*		pkBGBar = pkWnd->GetControl(_T("BG_BAR"));
	if( !pkBGBar )
	{
		return;
	}
	CXUI_AniBar*	pkAniBar = (CXUI_AniBar*)pkBGBar->GetControl(_T("ANIBAR"));
	if( !pkAniBar )
	{
		return;
	}
	lwUIWnd	kAniBarWnd(pkAniBar);
	kAniBarWnd.SetStartTime(0);
}
void PgActor::CutComboCharge()
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_COMBO_CHARGE_GAUGE"));
	if(pkWnd)
	{
		pkWnd->Close();
	}
}

namespace PgActorUtil
{
	void ExpressAwakeMaxState(PgActor* pkActor)
	{// 각성 게이지 맥스 상태 표현(파티클)
		if (pkActor)
		{
			pkActor->AddNewParticle("ef_awake_point_max", 8386, "char_root", pkActor->GetEffectScale());
			pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Com_awaketime", 0.0f);
		}
	}

	PgActor* GetSubPlayerActor(PgActor* pkActor)
	{
		if(!pkActor)
		{
			return NULL;
		}

		PgPilot* pkPilot = pkActor->GetPilot();
		if(pkPilot
			&& IsClass_OwnSubPlayer(pkPilot->GetAbil(AT_CLASS))
			)
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>( pkPilot->GetUnit() );
			if(pkPlayer)
			{
				PgActor* pkSubPlayerActor = g_kPilotMan.FindActor( pkPlayer->SubPlayerID() );
				return pkSubPlayerActor;
			}
		}
		return NULL;
	}
	
	int const GetCalcAniSeqID(int iSeqID, int const iBaseClassID, int const iWeaponAnimFolderNum)
	{
		if(iSeqID < 1000000)
		{
			// 공용 애니가 아니면, 쥔 무기와 액터의 클래스에 따라서 애니가 바뀐다.
			int const iClass = iBaseClassID - 1;
			iSeqID += iClass * 10000 + iWeaponAnimFolderNum * 1000;
		}
		return iSeqID;
	}

	int const GetOrigAniSeqID(int iSeqID, int const iBaseClassID, int const iWeaponAnimFolderNum)
	{
		if(1000000 > iSeqID)
		{// 공용 애니가 아니면, 쥔 무기와 액터의 클래스에 따라서 애니가 바뀐다.
			int iClass = iBaseClassID - 1;
			iSeqID -= iClass * 10000 + iWeaponAnimFolderNum * 1000;
		}
		return iSeqID;
	}

	void AdjustParticleScaleByUnitScaleAbil(CUnit* pkUnit, PgParticle* pkParticle)
	{
		if( !pkUnit 
			|| !pkParticle
			)
		{
			return;
		}
		if( 0 < pkUnit->GetAbil(AT_ADJUST_PARTICLE_SCALE_BY_UNIT_SCALE) )
		{// AT_UNIT_SCALE로 인해 scale이 커졌다면
			int const iUnitScale = pkUnit->GetAbil(AT_UNIT_SCALE);
			if(0 < iUnitScale
				&& ABILITY_RATE_VALUE != iUnitScale
				)
			{// 파티클을 붙일때 그만큼 줄여서 원래 크기대로 붙을수 있게 해주고
				float fScale = pkUnit->GetAbil(AT_UNIT_SCALE) / ABILITY_RATE_VALUE_FLOAT;
				fScale = pkParticle->GetScale()/fScale;
				pkParticle->SetScale(fScale);
				pkParticle->SetOriginalScale(fScale);
			}
		}
	}
};
