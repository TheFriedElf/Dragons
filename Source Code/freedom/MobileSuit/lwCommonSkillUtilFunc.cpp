#include "stdafx.h"
#include "Variant/DefAbilType.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgActionSlot.h"
#include "PgChatMgrClient.h"
#include "PgInterpolator.h"
#include "PgProjectileMan.h"
#include "lwBase.h"
#include "lwDeathSnatchSkillScriptHelpFunc.h"
#include "lwPilot.h"
#include "lwPilotMan.h"
#include "lwActor.h"
#include "lwAction.h"
#include "lwGuid.h"
#include "lwPoint3.h"
#include "lwFindTargetParam.h"
#include "lwQuaternion.h"
#include "lwActionTargetInfo.h"
#include "lwActionTargetList.h"
#include "lwActionResult.h"
#include "lwProjectile.h"
#include "lwCommonSkillUtilFunc.h"
#include "lwWorld.h"

namespace lwCommonSkillUtilFunc
{
	float const DEFAULT_ATTACK_HIT_HOLDING_TIME = 0.145f;

	int const ATTACK_TIMER_ID = 0;							// 때릴때 사용하는 Timer의 아이디
	
	// 다음 액션으로 캔슬 가능 한지 체크 하는 액션이름을 저장하는 시작 인덱스
	int const IDX_CHECK_ACTION_NAME = 10011;
	// 다음 액션으로 캔슬 가능 한지 체크할 이펙트 번호를 저장하는 시작 인덱스
	int const IDX_CANCEL_ACTION_EFFECT_NO = 10221;

	// 액션을 사용하기 위해 이전 액션 네임 체크
	int const IDX_CHECK_PREV_ACTION_NAME = 10121;
	
	// 액션을 하는중 자신소유의 대상에게 어떤 액션을 시키기 위해 필요한 값을 저장하는 시작 인덱스
	int const IDX_ORDER_ACTION_TARGET	= 11021;	// 어떤 대상에
	int const IDX_ORDER_ACTION_TIME		= 11031;	// 어떤 시점에
	int const IDX_ORDER_ACTION_NAME		= 11041;	// 어떤 액션을
	int const IDX_ORDER_ACTION_IDX_MAX	= 10;		// 모든 ORDER_ACTION류 idx는 10개가 최대

	//Param쪽 인덱스
	int const IDX_SPECIFY_PROCESS_ANI_SLOT_PARAM = 20100;	// AniSlot일때 특정 동작을 하기 위해서 param을 저장함, 100개씩 저장하며, 10개의 슬롯까지 가능(20100~21000)
		int const SUB_IDX_MEET_FOOLR_THEN_NEXT_ANI_SLOT = 1;

	int const IDX_PARTICLE_SLOT_AT_CAST_COMPLETE = 100000;
	int const IDX_PARTICLE_SLOT_AT_ON_CLEANUP	 = 100100;
	int const IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG	 = 100200;
	int const IDX_PARTICLE_SLOT_AT_ON_HIT_MSG	 = 100300;
	int const IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG = 100400;
	int const IDX_PARTICLE_SLOT_AT_ANI_START	 = 200000;
	int const IDX_PARTICLE_SLOT_AT_MOVE_START	 = 300000;
	int const IDX_PARTICLE_SLOT_AT_MOVE_END		 = 400000;
	
	int const IDX_PLAY_HIT_SOUND				 = 2784;
	int const IDX_PLAY_HIT_GENDER_SOUND			 = 2785;
	
	//--------------------------------------------------------

	std::string const STR_DEFALT_TRAIL_TEX("../Data/5_Effect/9_Tex/efx_pctrail.dds");
	
	std::string const STR_ENTER("ENTER");
	std::string const STR_CLEANUP("CLEANUP");
	std::string const STR_MSG_SC_START("MSG_SC_START");
	//--------------------------------------------------------
	// OnEvent에서 사용하는 이벤트 키 문자
	std::string const STR_EVENT_MSG_HIT("hit");
	std::string const STR_EVENT_MSG_FIRE("fire");
	std::string const STR_EVENT_MSG_MOVE_START("move_start");
	std::string const STR_EVENT_MSG_MOVE_STOP("move_stop");
	//std::string const STR_EVENT_MSG_ORDER_ACTION("oa");
	std::string const STR_EVENT_MSG_SC_START("sc_start");
	std::string const STR_EVENT_MSG_STOP_JUMP_START("stopjump_start");
	std::string const STR_EVENT_MSG_STOP_JUMP_END("stopjump_end");
	std::string const STR_EVENT_MSG_ALPHA_START("alpha_start");
	std::string const STR_EVENT_MSG_QUAKE_CAM("cam");
	std::string const STR_EVENT_MSG_QUAKE_MY_CAM("my_cam");
	std::string const STR_EVENT_MSG_EFFECT("effpos");
	std::string const STR_EVENT_MSG_CHANGE_CAM("c_cam");
	std::string const STR_EVENT_MSG_TAT_START("tat_start");
	std::string const STR_EVENT_MSG_TAT_END("tat_end");
	std::string const STR_EVENT_MSG_COMBO_DELAY_CANCEL("combo_cancel");

	std::string const STR_CAMERA_ZOOM_WHEN_Z_CAM_MSG("camera_zoom");
	std::string const STR_UI_MSG("ui");
	//--------------------------------------------------------

	std::string const STR_EFFECT_CNT_WHEN_ANI_SLOT = "EFFECT_CNT_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_ID_WHEN_ANI_SLOT = "EFFECT_ID_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_TYPE_WHEN_ANI_SLOT = "EFFECT_TYPE_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_SCALE_WHEN_ANI_SLOT = "EFFECT_SCALE_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_TARGET_NODE_WHEN_ANI_SLOT = "EFFECT_TARGET_NODE_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_DETACH_TIME_ANI_SLOT = "EFFECT_DETACH_TIME_ANI_SLOT";
	std::string const STR_EFFECT_HORIZ_DEG_ANI_SLOT  = "EFFECT_HORIZ_DEG_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_VERT_DEG_ANI_SLOT  = "EFFECT_VERT_DEG_WHEN_ANI_SLOT";
	std::string const STR_EFFECT_DIST_ANI_SLOT  = "EFFECT_DIST_WHEN_ANI_SLOT";

	std::string const STR_TRUE("TRUE");
	std::string const STR_FALSE("FALSE");

	std::string const STR_REAR("REAR");
	std::string const STR_CALLER("CALLER");
	std::string const STR_SELF("SELF");
	std::string const STR_TARGET("TARGET");
	std::string const STR_SAVED_TARGET("SAVED_TARGET");
	std::string const STR_SC("SC");
	std::string const STR_ANI_SLOT("ANI_SLOT");
	

	std::string const STR_USER_INPUT_DIR("USER_INPUT_DIR");
	
	std::string const STR_FIRE_PROJECTILE_ID = "FIRE_PROJECTILE_ID_";
	std::string const STR_FIRE_PROJECTILE_START_NODE_POS = "FIRE_PROJECTILE_START_NODE_POS_";
	std::string const STR_FIRE_PROJECTILE_MOVE_TYPE = "FIRE_PROJECTILE_MOVE_TYPE_";
	std::string const STR_FIRE_PROJECTILE_ARRIVE_POS_TYPE = "FIRE_PROJECTILE_ARRIVE_POS_TYPE_";
	std::string const STR_FIRE_PROJECTILE_NUM = "FIRE_PROJECTILE_NUM_";
	std::string const STR_FIRE_PROJECTILE_NUM_FROM_PARENT_SKILL_NO = "FIRE_PROJECTILE_NUM_FROM_PARENT_SKILL_NO";	//Parent Skill를 기준으로 발사체 갯수를 정한다.
//	std::string const STR_HIT_TARGET_EFFECT_TARGET_NODE = "HIT_TARGET_EFFECT_TARGET_NODE_";
	//std::string const STR_HIT_TARGET_EFFECT_ID = "HIT_TARGET_EFFECT_ID_";
	std::string const STR_FIRE_PROJECTILE_START_POS_ADD_X = "FIRE_PROJECTILE_START_POS_ADD_X_";
	std::string const STR_FIRE_PROJECTILE_START_POS_ADD_Y = "FIRE_PROJECTILE_START_POS_ADD_Y_";
	std::string const STR_FIRE_PROJECTILE_START_POS_ADD_Z = "FIRE_PROJECTILE_START_POS_ADD_Z_";
	std::string const STR_FIRE_PROJECTILE_SIN_POWER = "FIRE_PROJECTILE_SIN_POWER_";
	std::string const STR_FIRE_PROJECTILE_SIN_ANGLE = "FIRE_PROJECTILE_SIN_ANGLE_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG1 = "FIRE_PROJECTILE_BEZIER_HOREZ_DEG1_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_VERT_DEG1 = "FIRE_PROJECTILE_BEZIER_VERT_DEG1_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_DIST1 = "FIRE_PROJECTILE_BEZIER_DIST1_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG2 = "FIRE_PROJECTILE_BEZIER_HOREZ_DEG2_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_VERT_DEG2 = "FIRE_PROJECTILE_BEZIER_VERT_DEG2_";
	std::string const STR_FIRE_PROJECTILE_BEZIER_DIST2 = "FIRE_PROJECTILE_BEZIER_DIST2_";
	std::string const STR_WHEN_START_DIRECTION("WHEN_START_DIRECTION");
	lwPoint3 const UP_VEC(NiPoint3::UNIT_Z);
	
	std::string const STR_C_CAM_MSG_KEEP = "C_CAM_MSG_KEEP";
	std::string const STR_C_CAM_MSG_TYPE = "C_CAM_MSG_TYPE_";
	std::string const STR_C_CAM_MSG_CAMERA_NAME = "C_CAM_MSG_CAMERA_NAME_";	

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "TryMustChangeSubPlayerAction", TryMustChangeSubPlayerAction);
		def(pkState, "TryMustChangeActorAction", TryMustChangeActorAction);
		
		def(pkState, "SkillFunc_OnCheckCanEnter", OnCheckCanEnter);
		def(pkState, "SkillFunc_OnEnter", OnEnter);
		def(pkState, "SkillFunc_OnCastingCompleted", OnCastingCompleted);
		def(pkState, "SkillFunc_OnUpdate", OnUpdate);
		def(pkState, "SkillFunc_OnLeave", OnLeave);
		def(pkState, "SkillFunc_OnCleanUp", OnCleanUp);
		def(pkState, "SkillFunc_OnEvent", OnEvent);
		def(pkState, "SkillFunc_OnTimer", OnTimer);
		def(pkState, "SkillFunc_OnFindTarget", OnFindTarget);
		def(pkState, "SkillFunc_OnTargetListModified", OnTargetListModified);

		def(pkState, "SkillFunc_ProcessHitMsg", ProcessHitMsg);
		def(pkState, "SkillFunc_InitUseTrail", InitUseTrail);
		def(pkState, "SkillFunc_ProcessTrail", ProcessTrail);
		
		def(pkState, "SkillFunc_QuakeCamera", ProcessQuakeCamera);
		def(pkState, "SkillFunc_InitSpinDegPerSec", InitSpinDegPerSec);
		def(pkState, "SkillFunc_SetSpinDegPerSec", SetSpinDegPerSec);
		def(pkState, "SkillFunc_GetSpinDegPerSec", GetSpinDegPerSec);

		def(pkState, "SkillHelpFunc_DefaultDoDamage", DefaultDoDamage);
		def(pkState, "SkillHelpFunc_DefaultHitOneTime", DefaultHitOneTime);
		
		def(pkState, "SkillHelpFunc_SetReActionByTrapSkill", SetReActionByTrapSkill);
		def(pkState, "SkillHelpFunc_IsReActionByTrapSkill", IsReActionByTrapSkill);
		def(pkState, "SkillHelpFunc_PlayHitSound", PlayHitSound);
		def(pkState, "SkillHelpFunc_PlayWeaponSound", PlayWeaponSound);
		def(pkState, "SkillHelpFunc_DoDividedDamage", DoDividedDamage);
		
		def(pkState, "SkillFunc_InitUseDmgEffect", InitUseDmgEffect);

		def(pkState, "GetSummonedHitSound", GetSummonedHitSound);
		def(pkState, "Actor_Mob_Base_DoDefaultMeleeAttack", Actor_Mob_Base_DoDefaultMeleeAttack);
		def(pkState, "Act_Mon_Melee_DoDamage", Act_Mon_Melee_DoDamage);

		def(pkState, "SkillFunc_ProjectileHitOneTime", ProjectileHitOneTime);
		
		def(pkState, "ProcessEtc", ProcessEtc);
	}

	void ProcessEtc(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( EAT_ANI_SLOT0 <= eActionTime
			&& EAT_ANI_SLOT_MAX >= eActionTime
			)
		{
			int const iCurAniSlot = static_cast<int>(eActionTime - EAT_ANI_SLOT0);
			ProcessOnAniSlotTime(kActor, kAction, iCurAniSlot);
		}
		switch(eActionTime)
		{// 특정 액션 시점에 일부만 처리되어도 된다면
		case EAT_MSG_UI:
			{
				ProcessUIMsg(kActor, kAction, eActionTime);
			}break;
		case EAT_MSG_EFFECT:
			{
				ProcessEffectMsg(kActor, kAction);
				return;
			}break;
		case EAT_MSG_QUAKE_CAM:
			{
				ProcessQuakeCamera( kAction, eActionTime );
				return;
			}break;
		case EAT_MSG_QUAKE_MY_CAM:
			{
				if( kActor.IsMyActor() ||
					kActor.IsMySubPlayer() ) 
				{
					ProcessQuakeCamera( kAction, eActionTime );
				}
				return;
			}break;
		case EAT_MSG_ALPHA:
			{
				ProcessAlphaMsg( kActor, kAction );
				return;
			}break;
		case EAT_MSG_FIRE:
			{
				ProcessPlaySound( kActor, kAction, eActionTime );
				AttachParticle( kActor, kAction, eActionTime );
				ProcessFireMsg(kActor, kAction);
				ProcessCameraZoomRestore(kActor, kAction, EAT_MSG_FIRE);
				return;
			}break;
		case EAT_MSG_HIT:
			{
				ProcessPlaySound( kActor, kAction, eActionTime );
				AttachParticle( kActor, kAction, eActionTime );
				ProcessHitMsg(kActor, kAction);
				ProcessCameraZoomRestore(kActor, kAction, EAT_MSG_HIT);
				return;
			}break;
		case EAT_MSG_CHANGE_CAMERA:
			{
				ProcessChangeCam(kActor, kAction, eActionTime);
				return;
			}break;
		case EAT_HIT_TARGET:
			{
				if( kActor.IsMyActor() )
				{
					ProcessQuakeCamera( kAction, eActionTime );
					ProcessWorldFocusFilterColorAlpha(kActor, kAction, eActionTime);
				}
				return;
			}break;
		}
		// 그외의 액션 시점에는 모두 처리되어야 하는것
		ProcessQuakeCamera( kAction, eActionTime );
		AttachParticle( kActor, kAction, eActionTime );
		ProcessPlaySound( kActor, kAction, eActionTime );
		ProcessDecorationActor(kActor, kAction, eActionTime );
		ProcessTrail( kActor, kAction, eActionTime );
		ProcessOrderAction( kActor, kAction, eActionTime );
		ProcessWorldFocusFilterColorAlpha(kActor, kAction, eActionTime);
		ProcessDrawActorFilter(kActor, kAction, eActionTime);
		ProcessChangeCam(kActor, kAction, eActionTime);
		//DetachAniSlotParticle	// 이녀석은 예외임
	}

	bool ProcessCameraZoomRestore(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil()
			|| kAction.IsNil() 	)
		{
			return false;
		}
		if( !kActor.IsMyActor() )
		{
			return false;
		}
		if( NULL == g_pkWorld )
		{
			return false;
		}
		switch(eActionTime)
		{// 특정 액션 시점에 일부만 처리되어도 된다면
		case EAT_MSG_HIT:
		case EAT_MSG_FIRE:
			{
				if( 0 == strcmp("TRUE" , kAction.GetScriptParam("CAMERA_ZOOM_RESTORE_WHEN_HIT_MSG")) )
				{
					g_pkWorld->m_kCameraMan.RestoreCameraPos();
					return true;
				}
			}break;
		case EAT_ON_CLEANUP_COMBO_CANCEL:
			{
				if( 0 == strcmp("TRUE" , kAction.GetScriptParam("CAMERA_ZOOM_RESTORE_WHEN_ON_CLEANUP_CANCEL_COMBO")) )
				{
					g_pkWorld->m_kCameraMan.RestoreCameraPos();
					return true;
				}
			}break;
		}
		return false;
	}

	int CalcTotalCastTime(int const iOrigCastTime, int const iAddCastTime, int const iDecCastTimeRate)
	{
		int iResult = iOrigCastTime;
		if(0 == iOrigCastTime)
		{
			return iResult;
		}
		if(0 != iDecCastTimeRate)
		{// 캐스팅 타임 가감 Abil이 걸려있는지 확인해서 계산해주고
			iResult -= (iResult * iDecCastTimeRate) / ABILITY_RATE_VALUE;
		}
		iResult += iAddCastTime;
		iResult = std::max(0, iResult);
		return iResult;
	}

	bool TryMustChangeSubPlayerAction(lwActor kOwnerActor, char const* pcActionName, BYTE byDir, int const iCheckCoolTimeSkillNo, bool const bDoSameAction)
	{
		if( kOwnerActor.IsNil() )	{ return false; }
		if( !IsClass_OwnSubPlayer(kOwnerActor.GetAbil(AT_CLASS)) ) { return false; }

		lwActor kSubActor = kOwnerActor.GetSubPlayer();
		if( kSubActor.IsNil() )	{ return false; }

		lwAction kSubActorCurAction = kSubActor.GetAction();
		if( kSubActorCurAction.IsNil() )	{ return false; }
		
		if( false == bDoSameAction
			&& std::string(pcActionName) == kSubActorCurAction.GetID()
			)
		{
			return false;
		}
		
		if( kSubActorCurAction.CheckCanEnter(kSubActor, pcActionName, false) == false )
		{// 스킬 사용을 할수 있는가 체크
			return false;
		}

		if(0 < iCheckCoolTimeSkillNo)
		{
			if( kOwnerActor.IsInCoolTime(iCheckCoolTimeSkillNo, false) )
			{
				g_kChatMgrClient.ShowNoticeUI(TTW(235), 2, true, true);
				return false;
			}
			// 소유자의 액션에 쿨타임을 건다(보조캐릭터에게 걸수 없고, 보조캐릭터의 스킬은 소유자가 가지고 있기 때문)
			kOwnerActor.StartSkillCoolTime(iCheckCoolTimeSkillNo);
		}
		
		if( 0 != byDir )
		{
			kSubActorCurAction.SetDirection( byDir );
		}
		kSubActorCurAction.SetNextActionName( pcActionName );
		kSubActorCurAction.ChangeToNextActionOnNextUpdate( true,true );

		return true;
	}

	bool TryMustChangeActorAction(lwActor kActor, char const* pcActionName)
	{
		if( kActor.IsNil() )	{ return false; }
		lwAction kCurAction = kActor.GetAction();
		if( kCurAction.IsNil() )	{ return false; }
		//PgPilot* pkPilot = kActor.GetPilot();
		//if(!pkPilot)
		//{
		//	return false;
		//}
		//CUnit* pkUnit = pkPilot->GetUnit();
		//if(pkUnit)
		//{
		//	return false;
		//}
		//if( UT_SUB_PLAYER != pkUnit->GetUnitType() )
		//{
		//	return false;
		//}// 쌍둥이 캐릭에 sub 캐릭터 라면
		kCurAction.SetNextActionName(pcActionName);
		kCurAction.ChangeToNextActionOnNextUpdate(true,true);
		return true;
	}
	
	bool InitDirection(lwActor kActor, lwAction kAction, char const* pcScriptParam)
	{
		if( kActor.IsNil() )	{ return false; }
		if( kAction.IsNil() )	{ return false; }
		if( NULL == pcScriptParam) { return false; }

		std::string const kStrParamFrom = kAction.GetScriptParam( pcScriptParam );
		
		if( STR_CALLER == kStrParamFrom )
		{// 소환자(Caller)와 같은 방향
			lwActor kCallerActor =  kActor.GetCallerActor();
			if( kCallerActor.IsNil() ) { return false; }
			lwPoint3 const kDir = kCallerActor.GetLookingDir();
			//kActor.LookAt(kDir, true, false, false);
			kActor.ConcilDirection(kDir, true);
			kActor.SetMovingDir(kDir);							// 이동하면서도 방향이 바뀌려면 이렇게 세팅이 필요
		}
		else if( STR_TARGET == kStrParamFrom )
		{
			kAction.CreateActionTargetList(kActor, true, true);
			lwActionTargetList kTargetList = kAction.GetTargetList(); // 타격할 때와 겹칠일 없음(타격 시점에 즉시 생성후 제거해버리므로)
			
			for(int i =0; i < 1/*kTargetList.size()*/; ++i)
			{
				lwActionTargetInfo kInfo = kTargetList.GetTargetInfo(0); //kTargetList.GetTargetInfo(i);
				
				if( kInfo.IsNil() ) { break; }
				lwGUID kTargetGuid = kInfo.GetTargetGUID();
				if( kTargetGuid.IsNil() ) { break; }

				PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
				if( !pkTargetActor ) { break; }
				
				NiPoint3 kDir = pkTargetActor->GetPos() - kActor.GetPos()();
				kDir.Unitize();
				kActor.ConcilDirection(kDir, true);
				kActor.SetMovingDir(kDir);
			}
			kAction.ClearTargetList();
		}
		else if( STR_SAVED_TARGET == kStrParamFrom )
		{
			int const iCurIdx = GetCurUseSavedTargetIdx(kAction);
			lwGUID kTargetGuid = GetSavedTargetGuid(kAction, iCurIdx);
			if( kTargetGuid.IsNil() ) { return false; }
			
			PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
			if( !pkTargetActor ) { return false; }

			NiPoint3 kDir = pkTargetActor->GetPos() - kActor.GetPos()();
			kDir.Unitize();
			kActor.ConcilDirection(kDir, true);
			kActor.SetMovingDir(kDir);
		}
		else if( kActor.GetDirection() != static_cast<int>(kAction.GetDirection()) )
		{
			lwPoint3 const kDir = kActor.GetLookingDir();
			kActor.ConcilDirection(kDir, true);
			kActor.SetMovingDir(kDir);							// 이동하면서도 방향이 바뀌려면 이렇게 세팅이 필요
		}
		return true;
	}
	
	float GetAniSlotSpeed(lwActor kActor, lwAction kAction, int const iSlot)
	{// 현재 애니메이션 슬롯의 Speed 파라메터가 있다면 얻어온다(XML/PC/ 경로에 XML에 정의된 애니 SPEED)
		if( kActor.IsNil() )	{ return 0.0f; }
		if( kAction.IsNil() )	{ return 0.0f; }

		PgActor* pkActor = kActor();
		if(!pkActor)
		{
			return 0.0f;
		}
		PgActionSlot* pkActionSlot = pkActor->GetActionSlot();
		if(!pkActionSlot)
		{
			return 0.0f;
		}
		PgAction* pkAction = kAction();
		if(!pkAction)
		{
			return 0.0f;
		}

		std::string kCurAniName;
		pkAction->GetActionName(kCurAniName, pkAction->GetCurrentSlot());
		int const iAniSeq = pkActor->GetCurOriginSeqID();
		
		std::string kOutStr;
		if( pkActionSlot->GetAnimationInfo(kCurAniName, iAniSeq, std::string(STR_SPEED), kOutStr) )
		{
			float const fAniSpeed = PgStringUtil::SafeAtof(kOutStr);
			return fAniSpeed;
		}
		// 없다면 기본 속력
		return 1.0f;
	}

	bool InitSpinDegPerSec(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		float const fDeg = PgStringUtil::SafeAtof( kAction.GetScriptParam("SPIN_DEG_PER_SEC") );
		if(0.0f == fDeg)
		{
			return false;
		}
		SetSpinDegPerSec(kAction, fDeg);
		return true;
	}	
	
	bool SubFuncAttachParticle(lwActor kActor, lwAction kAction,int const iSlot, char const* pckParticleScriptParam, char const* pcTargetNodeScriptParam, char const* pcScaleScriptParam, char const* pcAttachTypeScriptParam
		,char const* pcHorizDeg, char const* pcVertDeg, char const* pcDist )
	{// 타겟 노드에 파티클 붙이기
		if(NULL == pckParticleScriptParam 
			|| NULL == pcTargetNodeScriptParam
			|| kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return false;
		}

		std::string const kParticleScriptParam( pckParticleScriptParam );
		std::string const kTargetNodeScriptParam( pcTargetNodeScriptParam );
		std::string const kScaleScriptParam( pcScaleScriptParam );
		std::string const kAttachTypeScriptParam( pcAttachTypeScriptParam );
		
		if( kParticleScriptParam.empty() 
			|| kTargetNodeScriptParam.empty()
			)
		{
			return false;
		}

		std::string const kEffectID = kAction.GetScriptParam( kParticleScriptParam.c_str() );
		std::string const kEffectTargetNodeID = kAction.GetScriptParam( kTargetNodeScriptParam.c_str() );
		
		int const iAttachType = PgStringUtil::SafeAtoi( kAction.GetScriptParam(kAttachTypeScriptParam.c_str()) );
		
		switch(iAttachType)
		{// 내 소유의 Actor에게만 붙이는 것이라면
		case APT_NODE_ONLY_MINE:
		case APT_NODE_POS_ONLY_MINE:
		case APT_NODE_POS_WITH_ROTATE_ONLY_MINE:
		case APT_CALLER_ROTATE_ONLY_MINE:
		case APT_ACTOR_POS_ONLY_MINE:
		case APT_ATTACH_TO_WORLD_ONLY_MINE:
		case APT_ATTACH_TO_WORLD_WITH_ROTATE_ONLY_MINE:
		//case APT_ATTACH_TO_RAY_DIR_COLL_PATH:
			{// 한번 검사해서
				if( false == kActor.IsUnderMyControl() )
				{// 불필요한 연산을 하지 않게끔 해주고
					return false;
				}
			}break;
		}

		if( kEffectID.empty()
			|| ( kEffectTargetNodeID.empty() && APT_ATTACH_TO_WORLD != iAttachType && APT_ACTOR_POS != iAttachType )
			)
		{
			return false;
		}
		float fHorizDeg = 0.0f;
		if(NULL != pcHorizDeg)
		{
			fHorizDeg = kAction.GetScriptParamAsFloat( pcHorizDeg );
		}
		float fVertDeg = 0.0f;
		if(NULL != pcVertDeg)
		{
			fVertDeg = kAction.GetScriptParamAsFloat( pcVertDeg );
		}
		float fDist = 0.0f;
		if(NULL != pcDist)
		{
			fDist = kAction.GetScriptParamAsFloat( pcDist );
		}

		std::string const kScale = kAction.GetScriptParam( kScaleScriptParam.c_str() );
		float fScale = 1.0f;
		if( !kScale.empty() )
		{
			fScale = PgStringUtil::SafeAtof( kScale.c_str() );
		}
		
		if( "__FRONT_ACTOR" == kEffectTargetNodeID )
		{
			lwPoint3 kDir = kActor.GetLookingDir();
			float const fDistance = 30.0f;
			kDir.Multiply(fDistance);
			kDir.Add(kActor.GetPos());
			kDir.SetZ(kDir.GetZ()-28.0f);
			kActor.AttachParticleToPointS(iSlot, kDir, kEffectID.c_str(), fScale);
		}
		else
		{
			lwPoint3 kDistVec = kActor.GetLookingDir();
			if(0.0f != fDist)
			{
				if(0.0f != fHorizDeg)
				{
					float const fHorizRad = NxMath::degToRad(fHorizDeg);
					kDistVec.Rotate( lwPoint3(NiPoint3::UNIT_Z), fHorizRad );
				}
				if(0.0f != fVertDeg)
				{
					lwPoint3 kRightDir = kActor.GetLookingDir();
					kRightDir.Cross( static_cast<lwPoint3>(NiPoint3::UNIT_Z) );
					kRightDir.Unitize();
					float const fVertRad = NxMath::degToRad(fVertDeg);
					kDistVec.Rotate( kRightDir, fVertRad);
				}
				kDistVec.Multiply(fDist);
			}
			switch(iAttachType)
			{
			case APT_NODE_POS:
			case APT_NODE_POS_ONLY_MINE:
				{// 노드 위치에 붙이기
					lwPoint3 kPos = kActor.GetNodeTranslate( kEffectTargetNodeID.c_str() );
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					kActor.AttachParticleToPointS(iSlot, kPos, kEffectID.c_str(), fScale);
				}break;
			case APT_NODE_POS_WITH_ROTATE:
			case APT_NODE_POS_WITH_ROTATE_ONLY_MINE:
				{// 노드 위치에 붙이면서, 노드의 방향을 적용시킨다
					lwPoint3 kPos = kActor.GetNodeTranslate( kEffectTargetNodeID.c_str() );
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					lwQuaternion kQuat = kActor.GetNodeWorldRotate( kEffectTargetNodeID.c_str() );
					kActor.AttachParticleToPointWithRotate(iSlot, kPos, kEffectID.c_str(), kQuat, fScale);
				}break;
			case APT_CALLER_ROTATE:
			case APT_CALLER_ROTATE_ONLY_MINE:
				{// 소환자의 방향을 적용
					lwPoint3 kPos = kActor.GetNodeTranslate( kEffectTargetNodeID.c_str() );
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					lwActor kCallerActor =  kActor.GetCallerActor();
					if( kCallerActor.IsNil() ) { return false; }
					lwQuaternion kQuat = kCallerActor.GetRotateQuaternion();
					kActor.AttachParticleToPointWithRotate(iSlot, kPos, kEffectID.c_str(), kQuat, fScale);
				}break;
			case APT_ACTOR_POS:
			case APT_ACTOR_POS_ONLY_MINE:
				{// Actor의 위치에 붙임
					lwPoint3 kPos = kActor.GetPos();
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					kActor.AttachParticleToPointS(iSlot, kPos, kEffectID.c_str(), fScale);
				}break;
			case APT_ATTACH_TO_WORLD:
			case APT_ATTACH_TO_WORLD_ONLY_MINE:
				{// World에 붙임
					lwWorld kWorld(g_pkWorld);
					if( kWorld.IsNil() ) { return false; }
					lwPoint3 kPos(0.0f, 0.0f, 0.0f);
					if( kEffectTargetNodeID.empty() )
					{
						kPos = kActor.GetPos();
					}
					else
					{
						kPos = kActor.GetNodeTranslate( kEffectTargetNodeID.c_str() );
					}
					
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					// iSlot이 저장되지 않기 때문에 Loop 파티클이면 안됨
					kWorld.AttachParticleSWithRotate(kEffectID.c_str(), kPos, fScale, lwPoint3(0.0f, 0.0f, 0.0f), 0.0f); 
				}break;
			case APT_ATTACH_TO_WORLD_WITH_ROTATE:
			case APT_ATTACH_TO_WORLD_WITH_ROTATE_ONLY_MINE:
				{// World에 붙임
					if( NULL == g_pkWorld )
					{
						return false;
					}
					lwWorld kWorld(g_pkWorld);
					if( kWorld.IsNil() ) { return false; }
					lwPoint3 kPos = kActor.GetNodeTranslate( kEffectTargetNodeID.c_str() );
					if(1.0f < kDistVec.Length() )
					{
						kPos.Add( kDistVec );
					}
					lwQuaternion kQuat = kActor.GetNodeWorldRotate( kEffectTargetNodeID.c_str() );
					// iSlot이 저장되지 않기 때문에 Loop 파티클이면 안됨
					kWorld.AttachParticleToPointWithRotate(kEffectID.c_str(), kPos, kQuat, fScale, 0.0f);
				}break;
			case APT_ATTACH_TO_RAY_DIR_COLL_PATH:
				{
					if( NULL == g_pkWorld )
					{
						return false;
					}
					lwWorld kWorld(g_pkWorld);
					if( kWorld.IsNil() ) { return false; }
					float fStart_X = kActor.GetPos().GetX()+kDistVec.GetX();
					float fStart_Y = kActor.GetPos().GetY()+kDistVec.GetY();
					float fStart_Z = kActor.GetPos().GetZ()+kDistVec.GetZ();
					lwPoint3 kStartPos = lwPoint3( fStart_X, fStart_Y, fStart_Z);
					NxRaycastHit kFloorHit;
					NxShape* pkFloorHitShape = g_pkWorld->raycastClosestShape(NxRay(NxVec3(kStartPos.GetX(),kStartPos.GetY(),kStartPos.GetZ()),NxVec3(0,0,-1)), NX_STATIC_SHAPES, kFloorHit, 1, 1000, NX_RAYCAST_SHAPE);
					if ( pkFloorHitShape == NULL)
					{
						return false;
					}
					lwPoint3 kPos( kFloorHit.worldImpact.x, kFloorHit.worldImpact.y, kFloorHit.worldImpact.z );
					lwQuaternion kQuat = kActor.GetNodeWorldRotate( kEffectTargetNodeID.c_str() );
					// iSlot이 저장되지 않기 때문에 Loop 파티클이면 안됨
					kWorld.AttachParticleToPointWithRotate(kEffectID.c_str(), kPos, kQuat, fScale, 0.0f);
				}break;
			default:
				{// 노드에 붙이기
					kActor.AttachParticleS(iSlot, kEffectTargetNodeID.c_str(), kEffectID.c_str(), fScale);
				}break;
			}
		}

		return true;
	}
	
	void AttachParticleOnAniSlotTime(lwActor kActor, lwAction kAction, int const iAniSlot)
	{// 현재 슬롯일때 처리할 파티클이 존재 하는가
		BM::vstring kEffectCnt(STR_EFFECT_CNT_WHEN_ANI_SLOT);
		kEffectCnt+=iAniSlot;

		std::string const kParam = kAction.GetScriptParam(kEffectCnt.operator const std::string().c_str());
		int const iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() );
		if(0 < iEffectCnt)
		{
			int const iStartSlot = IDX_PARTICLE_SLOT_AT_ANI_START + (iAniSlot*100);
			kAction.SetParamInt(iStartSlot, iEffectCnt);	// 지우기 위해

			BM::vstring kEffectParam( STR_EFFECT_ID_WHEN_ANI_SLOT );
			kEffectParam+=iAniSlot;
			BM::vstring kTargetNodeParam(STR_EFFECT_TARGET_NODE_WHEN_ANI_SLOT);
			kTargetNodeParam+=iAniSlot;
			BM::vstring kScaleParam(STR_EFFECT_SCALE_WHEN_ANI_SLOT);
			kScaleParam+=iAniSlot;
			BM::vstring kAttachTypeParam(STR_EFFECT_TYPE_WHEN_ANI_SLOT);
			kAttachTypeParam+=iAniSlot;

			BM::vstring kHorzDegParam(STR_EFFECT_HORIZ_DEG_ANI_SLOT);
			kHorzDegParam+=iAniSlot;
			BM::vstring kVertDegParam(STR_EFFECT_VERT_DEG_ANI_SLOT);
			kVertDegParam+=iAniSlot;
			BM::vstring kDistParam(STR_EFFECT_DIST_ANI_SLOT);
			kDistParam+=iAniSlot;

			for(int i=1; i <= iEffectCnt; ++i)
			{
				BM::vstring kEffect = (kEffectParam+"_")+i;
				BM::vstring kTargetNode = (kTargetNodeParam+"_")+i;
				BM::vstring kScale = (kScaleParam+"_")+i;
				BM::vstring kAttachType = (kAttachTypeParam+"_")+i;
				BM::vstring kHorzDeg = (kHorzDegParam+"_")+i;
				BM::vstring kVertDeg = (kVertDegParam+"_")+i;
				BM::vstring kDist = (kDistParam+"_")+i;

				bool const bSucess = SubFuncAttachParticle(kActor, kAction, iStartSlot+i
					, kEffect.operator const std::string().c_str()
					, kTargetNode.operator const std::string().c_str()
					, kScale.operator const std::string().c_str()
					, kAttachType.operator const std::string().c_str()
					, kHorzDeg.operator const std::string().c_str()
					, kVertDeg.operator const std::string().c_str()
					, kDist.operator const std::string().c_str()
					);
				if(!bSucess)
				{
					break;
				}
			}
		}
		
		if(0 < iAniSlot)
		{// 이전 애니에 붙은 파티클이 있는지 확인해서 떼어주고
			int const iPrevSlot = iAniSlot-1;
			DetachAniSlotParticle(kActor, kAction, iPrevSlot, EAT_ANI_COMPLETE);
		}
	}

	bool InitUseDmgEffect(lwAction kAction)
	{// 타격 이펙트 사용 유무
		if( kAction.IsNil() )	{ return false; }
		std::string const kParam = kAction.GetScriptParam("USE_DMG_EFFECT");
		if(kParam.empty()
			|| "FALSE" != kParam
			)
		{
			SetUseDmgEffect(kAction, true);
		}
		else
		{
			SetUseDmgEffect(kAction, false);
		}
		return true;
	}

	bool HideWeapon(lwActor kActor, lwAction kAction, bool const bHide)
	{
		if( kActor.IsNil() )	{ return false; }
		if( kAction.IsNil() )	{ return false; }

		if(bHide)
		{// 감추려 하는데
			if( 0 == kActor.GetPartsHideCnt(EQUIP_POS_WEAPON) )
			{// 숨기기가 안되어 있을경우에만
				kActor.HideParts(EQUIP_POS_WEAPON, true);
			}
		}
		else if( 0 < kActor.GetPartsHideCnt(EQUIP_POS_WEAPON) )
		{// 보여주는것이라면 숨기기가 되어있을경우에만
			kActor.HideParts(EQUIP_POS_WEAPON, false);
		}
		return true;
	}
	
	void InitUseAutoBroadCast(lwAction kAction)
	{// 액션을 자동으로 브로드 캐스팅 하지 않는다
		if( kAction.IsNil() )	{ return; }
		std::string const kAutoBroadCast = kAction.GetScriptParam("AUTO_BROADCAST");
		if( STR_FALSE == kAutoBroadCast )
		{
			kAction.SetDoNotBroadCast(true);
		}
	}
	
	void InitUseAutoBroadCastWhenCastingCompleted(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return; }
		std::string const kAutoBroadCast = kAction.GetScriptParam("AUTO_BROADCAST_WHEN_CASTING_COMPLETED");
		if( STR_FALSE == kAutoBroadCast )
		{
			kAction.SetAutoBroadCastWhenCastingCompleted(false);
		}
	}

	bool ProcessPlaySound(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil() || kAction.IsNil() )	{ return false; }
		std::string kSoundID;
		
		PgActor* pkTarget = kActor();
		switch( eActionTime )
		{
		case EAT_CAST_COMPLETE:
			{// 캐스팅 컴플리트 시점에 사운드 붙이기
				kSoundID = kAction.GetScriptParam("FIRE_SOUND_ID");
			}break;
		case EAT_ON_CLEANUP:
			{
				kSoundID = kAction.GetScriptParam("CLEANUP_SOUND_ID");
				if( kSoundID.empty() )
				{
					return false;
				}
				if( kActor.IsUnitType(UT_ENTITY) )
				{
					pkTarget = kActor.GetCallerActor()();
				}
			}break;
		case EAT_MSG_FIRE:
			{
				kSoundID = kAction.GetScriptParam("FIRE_MSG_SOUND_ID");
			}break;
		case EAT_MSG_HIT:
			{
				kSoundID = kAction.GetScriptParam("HIT_MSG_SOUND_ID");
			}break;
		case EAT_ON_ENTER:
		case EAT_ANI_COMPLETE:
		default:
			{
				return false;
			}break;
		}
		if( kSoundID.empty() 
			|| NULL == pkTarget
			)
		{
			return false;
		}
		pkTarget->AttachToSound(NiAudioSource::TYPE_3D, kSoundID.c_str(), 0.0f, 0.0f, 0.0f);

		return true;
	}
	
	bool ProcessQuakeCamera(lwAction kAction, EActionTime const eActionTime)
	{
		if( kAction.IsNil() )	{ return false; }
		
		BM::vstring vHeadStr;
		switch( eActionTime )
		{
		case EAT_CAST_COMPLETE:
			{
				vHeadStr="CAST_COMPLETE";
			}break;
		case EAT_ON_CLEANUP:
			{
				vHeadStr="CLEANUP";
			}break;
		case EAT_MSG_FIRE:
			{
				vHeadStr="FIRE_MSG";
			}break;
		case EAT_MSG_HIT:
			{
				vHeadStr="HIT_MSG";
			}break;
		case EAT_ON_ENTER:
			{
				vHeadStr="ENTER";
			}break;
		case EAT_MSG_QUAKE_CAM:
		case EAT_MSG_QUAKE_MY_CAM:
			{
				vHeadStr="CAM_MSG";	// 몬스터에서 카메라 흔들때 이것을 사용하므로 같이 이 메세지를 사용
			}break;
		case EAT_HIT_TARGET:
			{
				vHeadStr="HIT_TARGET";
			}break;
		case EAT_ANI_COMPLETE:
		default:
			{
				return false;
			}break;
		}
		BM::vstring vDurParamStr(vHeadStr+"_QUAKE_CAMERA_DURATION");
		BM::vstring vFactorParamStr(vHeadStr+"_QUAKE_CAMERA_FACTOR");
		BM::vstring vTypeParamStr(vHeadStr+"_QUAKE_CAMERA_TYPE");
		BM::vstring vFreqParamStr(vHeadStr+"_QUAKE_CAMERA_FREQ");
		BM::vstring vCntParamStr(vHeadStr+"_QUAKE_CAMERA_COUNT");
		
		if( EAT_MSG_QUAKE_CAM == eActionTime )
		{// 메세지로 
			int const iCnt = IncMsgCamCnt(kAction);
			vDurParamStr+="_";
			vFactorParamStr+="_";
			vTypeParamStr+="_";
			vFreqParamStr+="_";
			vCntParamStr+="_";
			vDurParamStr+=iCnt;
			vFactorParamStr+=iCnt;
			vTypeParamStr+=iCnt;
			vFreqParamStr+=iCnt;
			vCntParamStr+=iCnt;
		}

		float const fDuration = kAction.GetScriptParamAsFloat( vDurParamStr.operator const std::string().c_str() );
		float const fFactor = kAction.GetScriptParamAsFloat( vFactorParamStr.operator const std::string().c_str() );
		if( 0.0f >= fDuration
			|| 0.0f == fFactor
			)
		{
			if( EAT_MSG_QUAKE_CAM == eActionTime ||
				EAT_MSG_QUAKE_MY_CAM == eActionTime )
			{// 메세지로 들어오는것은 값이 없더라도 기본값으로 흔들어주고
				lwQuakeCamera(0.5f, 1.0f, 0, 0.0f, 0);
				return true;
			}
			return false;
		}
		
		int const iType = kAction.GetScriptParamAsInt( vTypeParamStr.operator const std::string().c_str() );
		float const fFreq = kAction.GetScriptParamAsFloat( vFreqParamStr.operator const std::string().c_str() );
		int const iCnt = kAction.GetScriptParamAsInt( vCntParamStr.operator const std::string().c_str() );
		
		lwQuakeCamera(fDuration, fFactor, iType, fFreq, iCnt);
		
		return true;
	}

	//bool IsCanAttack(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	//{// 공격 할수 있는가
	//	if(false == IsOverHitCnt(kAction) )
	//	{
	//		IncHitCnt(kAction);
	//	}
	//}

	bool DetachAniSlotParticle(lwActor kActor, lwAction kAction, int const iAniSlot, int const iActionTimeType)
	{
		if(0 > iAniSlot
			|| kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return false;
		}

		// 애니메이션 슬롯에 사용한 파티클 갯수가
		int const iStartSlot = IDX_PARTICLE_SLOT_AT_ANI_START + (iAniSlot*100);
		int const iEffectCnt = kAction.GetParamInt(iStartSlot);
		if(0 >= iEffectCnt)
		{
			return false;
		}

		BM::vstring kDetachTimeParam(STR_EFFECT_DETACH_TIME_ANI_SLOT);
		kDetachTimeParam+=iAniSlot;
		kDetachTimeParam+="_";

		for(int i=1; i <= iEffectCnt; ++i)
		{// 각 파티클 마다
			BM::vstring kDetachTime = kDetachTimeParam+i;
			// 언제 떼어야 하는지 확인해서
			std::string const kParam = kAction.GetScriptParam(kDetachTime.operator const std::string().c_str());
			if( kParam.empty() 
				&& ESS_CASTTIME != kAction.GetActionParam()	// 캐스팅 중에 끝난 액션
				)
			{
				continue;
			}
			
			EActionTime eActionTimeParam = EAT_ON_CLEANUP;	// EAT_ON_CLEANUP 이라면 무조건 뗌
			if( EAT_ON_CLEANUP != iActionTimeType )
			{// CleanUp 시점이 아니고
				if( "ANI_DONE" == kParam )
				{// 애니 완료시 떼는것이라면, 애니 완료시 떼는것이고
					eActionTimeParam = EAT_ANI_COMPLETE;
				}
			}

			if( eActionTimeParam == iActionTimeType )
			{// 떼어야 하면 뗀다
				kActor.DetachFrom(iStartSlot+i, true);
			}
		}
		return true;
	}

	bool AttachParticle(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{// 100000~ 110000
		if( kAction.IsNil() )
		{
			return false;
		}

		int iStartSlot = 0;
		int iEffectCnt = 0;
		BM::vstring kEffectScriptParam;
		BM::vstring kTargetNodeScriptParam;
		BM::vstring kScaleScriptParam;
		BM::vstring kAttachTypeParam;
		BM::vstring kHorzDegParam;
		BM::vstring kVertDegParam;
		BM::vstring kDistParam;

		switch(eActionTime)
		{
		case EAT_CAST_COMPLETE:
			{// 캐스트가 끝난 시점에
				{// 몇개의 파티클을 붙일지 얻어와
					std::string const kParam = kAction.GetScriptParam( "FIRE_CASTER_EFFECT_CNT" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{// cleanup시 몇번 slot까지 Detach 시켜야 할지 저장하고
					iStartSlot = IDX_PARTICLE_SLOT_AT_CAST_COMPLETE;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_CAST_COMPLETE, iEffectCnt);
				}
				//파티클 붙이는 타입, ID, 노드, 크기에 대한 스크립트 이름을 정하고
				kAttachTypeParam		= "FIRE_CASTER_EFFECT_TYPE_";
				kEffectScriptParam		= "FIRE_CASTER_EFFECT_ID_";
				kTargetNodeScriptParam	= "FIRE_CASTER_EFFECT_TARGET_NODE_";
				kScaleScriptParam		= "FIRE_CASTER_EFFECT_SCALE_";
				kHorzDegParam			= "FIRE_CASTER_EFFECT_HORIZ_DEG_";
				kVertDegParam			= "FIRE_CASTER_EFFECT_VERT_DEG_";
				kDistParam				= "FIRE_CASTER_EFFECT_DIST_";
			}break;
		case EAT_MSG_FIRE:
			{
				{
					std::string const kParam = kAction.GetScriptParam( "EFFECT_CNT_WHEN_FIRE_MSG" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{
					iStartSlot = IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG, iEffectCnt);
				}
				kAttachTypeParam		= "EFFECT_TYPE_WHEN_FIRE_MSG_";
				kEffectScriptParam		= "EFFECT_ID_WHEN_FIRE_MSG_";
				kTargetNodeScriptParam	= "EFFECT_TARGET_NODE_WHEN_FIRE_MSG_";
				kScaleScriptParam		= "EFFECT_SCALE_WHEN_FIRE_MSG_";
				kHorzDegParam			= "EFFECT_HORIZ_DEG_WHEN_FIRE_MSG_";
				kVertDegParam			= "EFFECT_VERT_DEG_WHEN_FIRE_MSG_";
				kDistParam				= "EFFECT_DIST_WHEN_FIRE_MSG_";
			}break;
		case EAT_ON_CLEANUP:
			{// 액션이 종료되는 시점에
				{
					std::string const kParam = kAction.GetScriptParam( "CLEANUP_CASTER_EFFECT_CNT" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{// cleanup시에 붙이고, 바로 뗄순 없으니 Detach를 하기위한 정보로는 사용하지 않는다(그러므로 loop 이펙트는 안됨)
					iStartSlot = IDX_PARTICLE_SLOT_AT_ON_CLEANUP;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_ON_CLEANUP, iEffectCnt);
				}
				kAttachTypeParam		= "CLEANUP_CASTER_EFFECT_TYPE_";
				kEffectScriptParam		= "CLEANUP_CASTER_EFFECT_ID_";
				kTargetNodeScriptParam	= "CLEANUP_CASTER_EFFECT_TARGET_NODE_";
				kScaleScriptParam		= "CLEANUP_CASTER_EFFECT_SCALE_";
				kHorzDegParam			= "CLEANUP_CASTER_EFFECT_HORIZ_DEG_";
				kVertDegParam			= "CLEANUP_CASTER_EFFECT_VERT_DEG_";
				kDistParam				= "CLEANUP_CASTER_EFFECT_DIST_";
			}break;
		case EAT_MSG_HIT:
			{
				{
					std::string const kParam = kAction.GetScriptParam( "EFFECT_CNT_WHEN_HIT_MSG" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{
					iStartSlot = IDX_PARTICLE_SLOT_AT_ON_HIT_MSG;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_ON_HIT_MSG, iEffectCnt);
				}
				kAttachTypeParam		= "EFFECT_TYPE_WHEN_HIT_MSG_";
				kEffectScriptParam		= "EFFECT_ID_WHEN_HIT_MSG_";
				kTargetNodeScriptParam	= "EFFECT_TARGET_NODE_WHEN_HIT_MSG_";
				kScaleScriptParam		= "EFFECT_SCALE_WHEN_HIT_MSG_";
				kHorzDegParam			= "EFFECT_HORIZ_DEG_WHEN_HIT_MSG_";
				kVertDegParam			= "EFFECT_VERT_DEG_WHEN_HIT_MSG_";
				kDistParam				= "EFFECT_DIST_WHEN_HIT_MSG_";
			}break;
		case EAT_MSG_EFFECT:
			{
				int const iCnt = GetMsgEffectCnt(kAction);
				{
					BM::vstring kTemp("EFFECT_CNT_WHEN_EFFECT_MSG"); kTemp += iCnt;
					std::string const kParam = kAction.GetScriptParam( kTemp.operator const std::string().c_str() );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else
					{// 붙이는 파티클 갯수가 없으면 사용 안하는것으로 간주하고
						return false;
					}
				}
				{
					iStartSlot = IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG + ((iCnt-1)*10);	// 메세지 1개당 이펙트 9개를 최대 붙일수 있게 하고
					kAction.SetParamInt(iStartSlot, iEffectCnt);
				}
				kAttachTypeParam		= "EFFECT_TYPE_WHEN_EFFECT_MSG";		kAttachTypeParam += iCnt;		kAttachTypeParam += "_";
				kEffectScriptParam		= "EFFECT_ID_WHEN_EFFECT_MSG";			kEffectScriptParam += iCnt;		kEffectScriptParam += "_";
				kTargetNodeScriptParam	= "EFFECT_TARGET_NODE_WHEN_EFFECT_MSG";	kTargetNodeScriptParam += iCnt;	kTargetNodeScriptParam += "_";
				kScaleScriptParam		= "EFFECT_SCALE_WHEN_EFFECT_MSG";		kScaleScriptParam += iCnt;		kScaleScriptParam += "_";
				kHorzDegParam			= "EFFECT_HORIZ_DEG_WHEN_EFFECT_MSG";	kHorzDegParam += iCnt;			kHorzDegParam += "_";
				kVertDegParam			= "EFFECT_VERT_DEG_WHEN_EFFECT_MSG";	kVertDegParam += iCnt;			kVertDegParam += "_";
				kDistParam				= "EFFECT_DIST_WHEN_EFFECT_MSG";		kDistParam += iCnt;				kDistParam += "_";
			}break;
		case EAT_MSG_MOVE_START:
			{
				{
					std::string const kParam = kAction.GetScriptParam( "EFFECT_CNT_WHEN_MOVE_START" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{
					iStartSlot = IDX_PARTICLE_SLOT_AT_MOVE_START;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_MOVE_START, iEffectCnt);
				}
				kAttachTypeParam		= "EFFECT_TYPE_WHEN_MOVE_START_";
				kEffectScriptParam		= "EFFECT_ID_WHEN_MOVE_START_";
				kTargetNodeScriptParam	= "EFFECT_TARGET_NODE_WHEN_MOVE_START_";
				kScaleScriptParam		= "EFFECT_SCALE_WHEN_MOVE_START_";
				kHorzDegParam			= "EFFECT_HORIZ_DEG_WHEN_MOVE_START_";
				kVertDegParam			= "EFFECT_VERT_DEG_WHEN_MOVE_START_";
				kDistParam				= "EFFECT_DIST_WHEN_MOVE_START_";
			}break;
		case EAT_MSG_MOVE_END:
			{
				{
					std::string const kParam = kAction.GetScriptParam( "EFFECT_CNT_WHEN_MOVE_END" );
					if( !kParam.empty() ) { iEffectCnt = PgStringUtil::SafeAtoi( kParam.c_str() ); }
					else { iEffectCnt = 1; }
				}
				{
					iStartSlot = IDX_PARTICLE_SLOT_AT_MOVE_END;
					kAction.SetParamInt(IDX_PARTICLE_SLOT_AT_MOVE_END, iEffectCnt);
				}
				kAttachTypeParam		= "EFFECT_TYPE_WHEN_MOVE_END_";
				kEffectScriptParam		= "EFFECT_ID_WHEN_MOVE_END_";
				kTargetNodeScriptParam	= "EFFECT_TARGET_NODE_WHEN_MOVE_END_";
				kScaleScriptParam		= "EFFECT_SCALE_WHEN_MOVE_END_";
				kHorzDegParam			= "EFFECT_HORIZ_DEG_WHEN_MOVE_END_";
				kVertDegParam			= "EFFECT_VERT_DEG_WHEN_MOVE_END_";
				kDistParam				= "EFFECT_DIST_WHEN_MOVE_END_";
			}break;
		default:
			{
				return false;
			}break;
		}

		for(int i=1; i <= iEffectCnt; ++i)
		{// 붙여야할 파티클 갯수만큼 각 Param을 얻어와 붙인다
			BM::vstring kEffect = kEffectScriptParam+i;
			BM::vstring kTargetNode = kTargetNodeScriptParam+i;
			BM::vstring kScale = kScaleScriptParam+i;
			BM::vstring kAttachType = kAttachTypeParam+i;
			BM::vstring kHorzDeg = kHorzDegParam+i;
			BM::vstring kVertDeg = kVertDegParam+i;
			BM::vstring kDist = kDistParam+i;
			bool const bSuccess = SubFuncAttachParticle(kActor, kAction, iStartSlot+i
				, kEffect.operator const std::string().c_str()
				, kTargetNode.operator const std::string().c_str()
				, kScale.operator const std::string().c_str()
				, kAttachType.operator const std::string().c_str()
				, kHorzDeg.operator const std::string().c_str()
				, kVertDeg.operator const std::string().c_str()
				, kDist.operator const std::string().c_str()
				);

			if( !bSuccess )
			{// 더이상 붙일게 없다면 멈춘다
				break;
			}
		}
		return true;
	}

	
	bool ProcessEffectMsg(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ return false; }
		if( kAction.IsNil() )	{ return false; }
		IncMsgEffectCnt(kAction); 	// Effect메세지가 들어왔으므로 증가시켜주고
		AttachParticle(kActor, kAction, EAT_MSG_EFFECT);
		return true;
	}
	bool ProcessHitMsg(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ return false; }
		if( kAction.IsNil() )	{ return false; }

		PgPilot* pkPilot = kActor.GetPilot()();
		if( !pkPilot )	{ return false; }
		CUnit* pkUnit = pkPilot->GetUnit();
		if( !pkUnit )	{ return false; }
		//IncMsgHitCnt(kAction);	// hit메세지가 들어왔으므로 증가시켜주고 (현재는 쓸일이 없음)

		if( kActor.IsUnderMyControl()			// 내 제어하에 있는 Actor이거나( Player, Entity)
			|| g_kPilotMan.IsMySummoned(pkUnit)	// 소환사의 소환체(Entity  아님)
			)
		{
			if( !IfAbleAttackTimerThenStart(kAction, EAT_MSG_HIT)	// Hit 메세지에 Timer를 쓰는게 아니고
				&& !IsUseAttackTimer(kAction)						// 어택 타이머를 사용하지 않아야만 hit 메세지로 때릴수 있으니 확인하고
				&& IsUseTLM(kAction)								// 스킬 사용 중간에 다시 때릴수 있다면
				)
			{
				if( false == IsOverHitCnt(kAction) )
				{// 최대 공격 수를 넘지 않았다면
					IncHitCnt(kAction);
					kAction.CreateActionTargetList(kActor, false, false);
					if( 0 < kAction.GetTargetList().size() )
					{
						AttachLinkageUseSkillEffect(kActor, kAction, EAT_MSG_HIT);
						kAction.BroadCastTargetListModify(kActor.GetPilot(), false);
					}
				}
			}
			kAction.ClearTargetList();
			return true;
		}
		return false;
	}

	bool ProcessFireMsg(lwActor kActor, lwAction kAction)
	{
		int const iCnt = IncMsgFireCnt(kAction); // 몇번째 fire msg가 들어왔는가

		lwGUID kGuid = kActor.GetPilotGuid();
		std::string kProjectileID;
		BM::vstring vStr(STR_FIRE_PROJECTILE_ID); vStr+=iCnt;
		{// 발사체 ID를 Action XML에서 찾아보고
			kProjectileID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( kProjectileID.empty() )
			{// 없다면 Animation Slot XXML에서 찾아보고
				char const* pcStr = kActor.GetAnimationInfo(STR_PROJECTILE_ID, 0);
				if(NULL == pcStr) { return true; }	// 그래도 없으면 종료한다.
				kProjectileID = pcStr;
			}
		}
		lwPoint3 kStartPos(0.0f, 0.0f, 0.0f);
		{// 발사체 시작위치
			std::string kNodeID;
			vStr=STR_FIRE_PROJECTILE_START_NODE_POS; vStr+=iCnt;
			kNodeID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( kNodeID.empty() )
			{
				char const* pcStr = kActor.GetAnimationInfo(STR_FIRE_START_NODE, 0);
				if(NULL == pcStr) { return true; }	// 그래도 없으면 종료한다.
				kNodeID = pcStr;
			}
			kStartPos = kActor.GetNodeWorldPos( kNodeID.c_str() );
			//lwPoint3 kDir = kActor.GetLookingDir();
			//kDir.Multiply(30.0f);
			//kStartPos.Add(kDir);
		}

		vStr=STR_FIRE_PROJECTILE_MOVE_TYPE; vStr+=iCnt;
		PgProjectile::MovingType eMovingType = static_cast<PgProjectile::MovingType>( kAction.GetScriptParamAsInt( vStr.operator const std::string().c_str() ) );
		vStr=STR_FIRE_PROJECTILE_ARRIVE_POS_TYPE; vStr+=iCnt;
		int const iArriveMoveType = kAction.GetScriptParamAsInt( vStr.operator const std::string().c_str() );
		int const iAttackRange = kAction.GetSkillRange(0, kActor);// - 45;

		int iSkillLevel = kAction.GetSkillLevel();
		if( 0 != STR_FIRE_PROJECTILE_NUM_FROM_PARENT_SKILL_NO.size() ) 
		{
			int const iParentSkillNo = kAction.GetScriptParamAsInt( STR_FIRE_PROJECTILE_NUM_FROM_PARENT_SKILL_NO.c_str() );
			if( 0 != iParentSkillNo )
			{
				PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode( iParentSkillNo );
				if( NULL != pkNode )
				{
					CSkillDef const *pDef = pkNode->GetSkillDef();
					if( NULL != pDef )
					{
						iSkillLevel = static_cast<int>(pDef->m_byLv);
					}
				}
			}
		}
		vStr=STR_FIRE_PROJECTILE_NUM; vStr+=iCnt; vStr+="_ON_SKILL_LV"; vStr+=iSkillLevel;
		int iProjectileCnt = kAction.GetScriptParamAsInt( vStr.operator const std::string().c_str() );//kAction.GetAbil( AT_PROJECTILE_NUM );
		if(0 == iProjectileCnt) { iProjectileCnt = 1; }

		for(int i = 1; i <= iProjectileCnt; ++i)
		{
			//IncFireProjectileCnt(kAction);	// 몇번째 발사체 인가 // 현재는 사용할 일이 없음

			PgProjectile* pkProjectile = g_kProjectileMan.CreateNewProjectile( kProjectileID, kAction(), kGuid() );
			if( !pkProjectile )
			{
				return false;
			}
			//vStr=STR_HIT_TARGET_EFFECT_TARGET_NODE; vStr+=iCnt;
			std::string const kEffectHitTargetNode = kAction.GetScriptParam( "HIT_TARGET_EFFECT_TARGET_NODE" );
			//vStr=STR_HIT_TARGET_EFFECT_ID; vStr+=iCnt;
			std::string const kEffectID = kAction.GetScriptParam( "HIT_TARGET_EFFECT_ID" );
			if( !kEffectHitTargetNode.empty()
				&& !kEffectID.empty() )
			{
				//vStr=STR_HIT_TARGET_EFFECT_TARGET_NODE; vStr+=iCnt;
				pkProjectile->SetParam( "HIT_TARGET_EFFECT_TARGET_NODE", kEffectHitTargetNode.c_str() );
				//vStr=STR_HIT_TARGET_EFFECT_ID; vStr+=iCnt;
				pkProjectile->SetParam( "HIT_TARGET_EFFECT_ID", kEffectID.c_str() );
			}
			lwPoint3 kFinalStartPos = kStartPos;
			{
				float fAddPos_X = 0.0f;
				vStr = STR_FIRE_PROJECTILE_START_POS_ADD_X;
				vStr+=iCnt;
				vStr+="_"; 
				vStr+=i;
				std::string kNodeID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( !kNodeID.empty() )
				{
					fAddPos_X = static_cast<float>( ::atof( kNodeID.c_str() ) );
					if( 0 != fAddPos_X )
					{
						lwPoint3 kLookingDir = kActor.GetLookingDir();
						lwPoint3 kPathNormal = kLookingDir.Cross( lwPoint3(1,0,0) );
						lwPoint3 kAddPos = kPathNormal.Multiply2(fAddPos_X);
						kFinalStartPos = kStartPos.Add2( kAddPos );
					}
				}
				float fAddPos_Y = 0.0f;
				vStr = STR_FIRE_PROJECTILE_START_POS_ADD_Y;
				vStr+=iCnt;
				vStr+="_"; 
				vStr+=i;
				kNodeID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( !kNodeID.empty() )
				{
					fAddPos_Y = static_cast<float>( ::atof( kNodeID.c_str() ) );
					if( 0 != fAddPos_Y )
					{
						lwPoint3 kLookingDir = kActor.GetLookingDir();
						lwPoint3 kPathNormal = kLookingDir.Cross( lwPoint3(0,1,0) );
						lwPoint3 kAddPos = kPathNormal.Multiply2(fAddPos_Y);
						kFinalStartPos = kStartPos.Add2( kAddPos );
					}
				}
				float fAddPos_Z = 0.0f;
				vStr = STR_FIRE_PROJECTILE_START_POS_ADD_Z;
				vStr+=iCnt;
				vStr+="_"; 
				vStr+=i;
				kNodeID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( !kNodeID.empty() )
				{
					fAddPos_Z = static_cast<float>( ::atof( kNodeID.c_str() ) );
					if( 0 != fAddPos_Z )
					{
						lwPoint3 kLookingDir = kActor.GetLookingDir();
						lwPoint3 kPathNormal = kLookingDir.Cross( lwPoint3(0,0,1) );
						lwPoint3 kAddPos = kPathNormal.Multiply2(fAddPos_Z);
						kFinalStartPos = kStartPos.Add2( kAddPos );
					}
				}
				pkProjectile->SetWorldPos( kFinalStartPos() );
			}

			lwPoint3 kArrivePos(0.0f, 0.0f, 0.0f);
			{// 도착 위치
				kArrivePos = kActor.GetLookingDir();
				switch(iArriveMoveType)
				{
				case EPAT_TARGET_POS:
					{// 타겟의 위치 따라가기
						kAction.CreateActionTargetList(kActor, true, true);
						lwActionTargetList kTargetList = kAction.GetTargetList();

						if( kTargetList.size() )
						{
							//lwActionTargetInfo kInfo = kTargetList.GetTargetInfo(i);
							//lwGUID kTargetGUID =  kInfo.GetTargetGUID();
							int const iIdx = i % kTargetList.size();
							lwGUID kTargetGUID = kAction.GetTargetGUID(iIdx);
							PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGUID() );
							if(pkTargetActor)
							{
								//int const iABVIndex = kAction.GetTargetABVShapeIndex(i);
								//kArrivePos = pkTargetActor->GetABVShapeWorldPos(iABVIndex);
								kArrivePos = pkTargetActor->GetPos();
							}
						}
						else
						{
							kArrivePos.Multiply( static_cast<float>(iAttackRange) );
							kArrivePos.Add(kFinalStartPos);
						}
						kAction.ClearTargetList();
					}break;
				case EPAT_SET_DIRECTION:
					{// 설정한 방향으로 발사하기
						BM::vstring vDirDeg("FIRE_PROJECTILE_DEG"); 	
						vDirDeg+=iCnt;
						vDirDeg+="_"; 
						vDirDeg+=i;
						BM::vstring vDirDegType("FIRE_PROJECTILE_DEG_TYPE"); 	
						vDirDegType+=iCnt;
						vDirDegType+="_"; 
						vDirDegType+=i;

						float fProjectileDir = kAction.GetScriptParamAsFloat( vDirDeg.operator const std::string().c_str() );
						fProjectileDir = NxMath::degToRad(fProjectileDir);

						lwPoint3 kRotateVec(0.0f, 0.0f, 0.0f);
						int iDegType = kAction.GetScriptParamAsInt( vDirDegType.operator const std::string().c_str() );
						switch(iDegType)
						{
						case EPRT_UP_DOWN:
							{
								lwPoint3 kLookVec = kActor.GetLookingDir();
								lwPoint3 kUpVec = UP_VEC;
								lwPoint3 kRightVec = kLookVec.Cross( kUpVec );
								kRotateVec = kRightVec;
							}
						}
						
						kArrivePos.Rotate( kRotateVec, fProjectileDir );
						kArrivePos.Multiply(  static_cast<float>(iAttackRange) );
						kArrivePos.Add(kFinalStartPos);
					}break;
				default:
					{
						BM::vstring vDirDeg("FIRE_PROJECTILE_DEG"); 	vDirDeg+=iCnt; vDirDeg+="_"; vDirDeg+=i;
						float fProjectileDir = kAction.GetScriptParamAsFloat( vDirDeg.operator const std::string().c_str() );
						fProjectileDir = NxMath::degToRad(fProjectileDir);
						
						kArrivePos = kActor.GetLookingDir();
						kArrivePos.Rotate( UP_VEC, fProjectileDir );
						kArrivePos.Multiply(  static_cast<float>(iAttackRange) );
						kArrivePos.Add(kFinalStartPos);
					}break;
				}
				pkProjectile->SetTargetLoc( kArrivePos() );
			}

			{// 이동 방식
				pkProjectile->SetMovingType( eMovingType );
				switch( eMovingType )
				{
				case PgProjectile::MT_SIN_CURVELINE:
					{//	사인곡선 이동
						vStr=STR_FIRE_PROJECTILE_SIN_POWER; vStr+=iCnt; vStr+="_"; vStr+=i;
						float const fPower = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
						vStr=STR_FIRE_PROJECTILE_SIN_ANGLE; vStr+=iCnt; vStr+="_"; vStr+=i;
						float const fAngle = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
						pkProjectile->SetParam_SinCurveLineType( fPower, fAngle );
					}break;
				case PgProjectile::MT_BEZIER4_SPLINE:
					{//	베지어 곡선 이동
						lwPoint3 kDir = kArrivePos.Subtract2( kStartPos );
						kDir.Unitize();
						lwPoint3 kRightDir = kDir;
						kRightDir.Cross( static_cast<lwPoint3>(NiPoint3::UNIT_Z) );

						lwPoint3 kPos1 = kDir;
						{
							vStr=STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG1; vStr+=iCnt;  vStr+="_"; vStr+=i;
							float const fHorizDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							if(0.0f != fHorizDeg)
							{
								float const fHorizRad = NxMath::degToRad(fHorizDeg);
								kPos1.Rotate( lwPoint3(NiPoint3::UNIT_Z), fHorizRad );
							}
							vStr=STR_FIRE_PROJECTILE_BEZIER_VERT_DEG1; vStr+=iCnt;  vStr+="_"; vStr+=i;
							float const fVertDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							if(0.0f != fVertDeg)
							{
								float const fVertRad = NxMath::degToRad(fVertDeg);
								kPos1.Rotate( kRightDir, fVertRad );
							}
							vStr=STR_FIRE_PROJECTILE_BEZIER_DIST1; vStr+=iCnt;  vStr+="_"; vStr+=i;
							float const fRate = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							float const fDist = static_cast<float>(iAttackRange)*fRate;
							kPos1.Multiply( fDist );
							kPos1.Add( kStartPos );
						}

						lwPoint3 kPos2 = kActor.GetLookingDir();
						{
							vStr=STR_FIRE_PROJECTILE_BEZIER_HOREZ_DEG2; vStr+=iCnt;  vStr+="_"; vStr+=i;
							float const fHorizDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							if(0.0f != fHorizDeg)
							{
								float const fHorizRad = NxMath::degToRad(fHorizDeg);
								kPos2.Rotate( lwPoint3(NiPoint3::UNIT_Z), fHorizRad );
							}
							vStr=STR_FIRE_PROJECTILE_BEZIER_VERT_DEG2; vStr+=iCnt; vStr+="_"; vStr+=i;
							float const fVertDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							if(0.0f != fVertDeg)
							{
								float const fVertRad = NxMath::degToRad(fVertDeg);
								kPos2.Rotate( kRightDir, fVertRad );
							}
							vStr=STR_FIRE_PROJECTILE_BEZIER_DIST2; vStr+=iCnt; vStr+="_"; vStr+=i;
							float const fRate = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
							float const fDist = static_cast<float>(iAttackRange)*fRate;
							kPos2.Multiply( fDist );
							kPos2.Add( kStartPos );
						}
						pkProjectile->SetParam_Bezier4SplineType( kStartPos(), kPos1(), kPos2(), kArrivePos() );

					}break;
				case PgProjectile::MT_HOMMING:	// 위치 따라가기
				default:
					{
					}break;
				}
			}

			{// 관통
				int iPentrationCount = kAction.GetAbil(AT_PENETRATION_COUNT);
				if( 0 == iPentrationCount )
				{//액션에 물려있는 관통 갯수가 0이라면 액터에 걸려있는 어빌에서 다시 찾아보자
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = (CSkillDef*) kSkillDefMgr.GetDef( kAction.GetSkillNo() );
					if(pkSkillDef)
					{
						if(0 < pkSkillDef->GetAbil(AT_PENETRATION_USE_TYPE))
						{//무조건 넣으면 안되고 스킬에서 사용하는것만 넣어줘야한다.
							iPentrationCount = kActor.GetPilot().GetAbil(AT_PENETRATION_COUNT);
						}
					}
				}
				pkProjectile->PenetrationCount(iPentrationCount);					// 몇번 관통
				pkProjectile->SetMultipleAttack( (0<iPentrationCount) );
			}
			{
				pkProjectile->SetParentActionInfo( kAction.GetActionNo(), kAction.GetActionInstanceID(), kAction.GetTimeStamp() );
				pkProjectile->Fire();
				//kActor.ClearIgnoreEffectList();
			}
		}
		return true;
	}
	bool ProcessAlphaMsg(lwActor kActor, lwAction kAction)
	{// actor에 alpha 값을 넣는다면
		if( kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return false;
		}
		int const iNowCnt = IncMsgAlphaCnt(kAction);
		BM::vstring vStr("MSG_ALPHA_START_");	vStr+=iNowCnt;
		float const fStartAlpha = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
		
		vStr = "MSG_ALPHA_END_";	vStr+=iNowCnt;
		float const fTargetAlpha = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );

		vStr = "MSG_ALPHA_TIME_";	vStr+=iNowCnt;
		float const fDuration = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );

		kActor.SetTargetAlpha(fStartAlpha, fTargetAlpha, fDuration, false);
		return true;
	}

	bool IsComboDelay(lwActor kActor)
	{
		if( kActor.IsNil() )
		{
			return false;
		}
		char const *pkIsCancle = kActor.GetAnimationInfo("COMBO_DELAY",0);
		if( pkIsCancle )
		{
			int const iIsCancle = atoi(pkIsCancle);
			return (0 < iIsCancle);
		}
		return false;
	}

	int IsAbleToNextAniSlot(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)
	{
		if( kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return ENAIR_NONE;
		}

		int iResult = ENAIR_NONE;
		if( true == GetMeetFloorThenPlayNextAni(kAction) 
			&& false == kActor.IsMeetFloor()
			)
		{// 바닥에 부딪혀야 다음 애니로 갈수 있음
			iResult = iResult | ENAIR_ON_FLOATING;
		}
		int const iCurAniSlot = kAction.GetCurrentSlot();
		if( !IsDoneRepeatAniSlot(kAction, iCurAniSlot) )
		{// 반복 완료가 안되었다면, 현재 애니를 다시 시작
			iResult = iResult | ENAIR_REMAIN_ANI_REPEAT;
		}
		return iResult;
	}
	void ProcessNextAniIncompleteReason(int const iIncompleteReason, lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)
	{// 다음 애니 슬롯으로 넘어 갈수 없는 이유 중
		if( kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return;
		}
		if(iIncompleteReason & ENAIR_REMAIN_ANI_REPEAT)
		{// 반복 완료가 안되었다면
			int const iCurAniSlot = kAction.GetCurrentSlot();
			IncCntRepeatAniSlot(kAction, iCurAniSlot);			// 반복 횟수를 증가 시키고
			kActor.ResetAnimation();
			kActor.PlayCurrentSlot(false);						// 현재 애니를 다시 시작하고 
			// 이 AniSlot 시간에 해야할 것을 다시 수행 할수 있게 처리 한다
			ProcessEtc(kActor, kAction, static_cast<EActionTime>(EAT_ANI_SLOT0+iCurAniSlot));
		}
	}

//--------------------------------------------------------------------------------------------
	// 10000
	void SetUseTLM(lwAction kAction, bool const bUseTLM)
	{	// 액션 사용 중간에 타겟을 다시 잡아 때리는가? 에 대한 설정
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10000, bUseTLM ? 1 : 0 );
	}
	bool IsUseTLM(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(10000) )
		{
			return true;
		}
		return false;
	}


	void SetStopJump(lwActor kActor, lwAction kAction, bool const bStop)
	{// 10001 - 점프 중에 멈춰야 하는 액션인가
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( bStop )
		{
			kActor.StopJump();
			kActor.FreeMove(true);
			kAction.SetParamInt(10001, 1);
			return;
		}
		kActor.FreeMove(false);
		kAction.SetParamInt(10001, 0);
	}
	bool IsStopJump(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if(0 < kAction.GetParamInt(10001))
		{
			return true;
		}
		return false;
	}
	
		
	void SetSpinDegPerSec(lwAction kAction, float const fRotateDegPerSec)
	{// 10002 초당 회전 각도
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat(10002, fRotateDegPerSec);
	}
	float GetSpinDegPerSec(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat(10002);
	}

	void SetMeetFloorThenPlayNextAni(lwAction kAction, bool const bOn)
	{// 10003 바닥에 만났을때 다음 애니를 한다
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10003, bOn ? 1 : 0);
	}
	bool GetMeetFloorThenPlayNextAni(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10003);
	}

	void SetUseDmgEffect(lwAction kAction, bool const bOn)
	{//10004 - 기본적인 타격 이펙트를 사용하는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10004, bOn ? 1 : 0);
	}
	bool GetUseDmgEffect(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10004);
	}
	
	void InitSyncInputDir(lwActor kActor, lwAction kAction)
	{
		if( STR_TRUE == kAction.GetScriptParam("MOVE_SYNC_INPUT_DIR") )
		{
			UseSyncInputDir(kAction, true);
			BYTE byDir = kActor.GetDirection();
			kAction.SetDirection(byDir);
		}
		else
		{
			UseSyncInputDir(kAction, false);
		}
	}
	void UseSyncInputDir(lwAction kAction, bool const bUse)
	{// 10005 - 사용자의 입력에 의한 방향을 사용하는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10005, bUse ? 1 : 0);
	}
	bool IsSyncInputDir( lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10005);
	}
	
	int IncFireProjectileCnt(lwAction kAction)
	{// 10006 - 몇번째 발사체가 발사 되는가
		if( kAction.IsNil() )	{ return 0; }
		int const iCurCnt = GetFireProjectileCnt(kAction);
		int const iNewCnt = iCurCnt+1;
		kAction.SetParamInt( 10006, iNewCnt );
		return iNewCnt;
	}
	int GetFireProjectileCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 10006 );
	}
	
	void AdjustAniSpeedOnCastingTime(lwActor kActor, lwAction kAction)
	{// 10007
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		
		PgActor* pkActor = kActor();
		PgPilot* pkPilot = pkActor->GetPilot();
		if( !pkPilot ) { return; }
		CUnit* pkUnit = pkPilot->GetUnit();
		if( !pkUnit ) { return; }
		
		PgAction* pkAction = kAction();
		CSkillDef const* pkCurActionSkillDef = pkAction->GetSkillDef();
		if(!pkCurActionSkillDef)	{ return; }

		int const iOrigCastTime = pkCurActionSkillDef->GetAbil(AT_CAST_TIME);
		if( 0 >= iOrigCastTime )
		{// 캐스팅이 있는 스킬 중
			return;
		}
		CSkill* pkSkill = pkUnit->GetSkill();
		if( !pkSkill ) { return; }
		// 캐스팅 가감을 계산해서
		int const iCalcedCastTime = CalcTotalCastTime( iOrigCastTime, pkSkill->GetSkillCastingTime(), pkUnit->GetAbil(AT_CAST_TIME_RATE) );

		PgActionSlot* pkActionSlot = pkActor->GetActionSlot();
		if( !pkActionSlot ) { return; }

		std::string kCurAniName;
		pkAction->GetActionName(kCurAniName, pkAction->GetCurrentSlot());

		float fAniTime = pkActor->GetAnimationTime(kCurAniName);
		float fAbsoluteAniTime = iOrigCastTime/1000.0f;			// 애니시간을 캐스팅과 정확히 맞춘다면 애니 총시간은 이시간이 되어야함	
		float const fAblsoluteAniMaker = fAniTime/fAbsoluteAniTime;

		float fAttackSpeed = 1.0f;
		if( pkAction->GetAbil(AT_APPLY_ATTACK_SPEED) == 1 )
		{// 공속 적용을 받는다면 애니가 더 빨라진다
			fAttackSpeed = kActor.GetAbil(AT_C_ATTACK_SPEED)/ABILITY_RATE_VALUE_FLOAT;
		}
		
		//float const fAnimationSpeed = pkActor->GetAnimationSpeed(kCurAniName);	// 현재 슬롯의 애니메이션 속력
		float const fPer = static_cast<float>(iCalcedCastTime)/static_cast<float>(iOrigCastTime);
		if( 0.0f == fPer )
		{
			kAction.SetParamFloat(10007, fAblsoluteAniMaker /** fAnimationSpeed*/ * 10.0f * fAttackSpeed); // 즉시시전이 되었다면 애니를 10배 빠르게 해서 즉시 시전과 같은 느낌을 주고
			return;
		}
		// pkActor->GetAnimSpeed()에서 현재 액션의 시퀀스가 아닌 이전 액션의 시퀀스를 가지고 있기 때문에 엉뚱한 애니메이션 스피드를 적용하고 있음
		float const fSpeed = 1.0f/fPer;
		kAction.SetParamFloat(10007, fAblsoluteAniMaker /** fAnimationSpeed */* fSpeed * fAttackSpeed);
	}
	void ApplyAniSpeedOnUpdate(lwActor kActor, lwAction kAction)
	{// 10007
		if( kActor.IsNil() 
			|| kAction.IsNil()
			||  0 != kAction.GetCurrentSlot() // 첫번째 슬롯만이 캐스팅 애니메이션
			)
		{// 캐스팅 애니가 아닌경우에는 적용하지 않는다
			return;
		}
		
		float fAniSpeed = kAction.GetParamFloat(10007);
		if(0.0f == fAniSpeed || 1.0f == fAniSpeed)
		{
			return;
		}
		kActor()->SetAnimSpeed(fAniSpeed);
	}
	
	void InitMaxHitCnt(lwAction kAction)
	{// 10008 최대 몇번이나 Create Target을 할수 있는가 (공격을 몇회 하는가)
		if( kAction.IsNil() )	{ return; }
		int const iMaxHitCnt =  kAction.GetAbil(AT_COUNT);
		SetMaxHitCnt(kAction, iMaxHitCnt);
	}
	void SetMaxHitCnt(lwAction kAction, int const iMaxHitCnt)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10008, 0 >= iMaxHitCnt ? 1 : iMaxHitCnt);
	}
	int GetMaxHitCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10008);
	}
	
	int IncHitCnt(lwAction kAction)
	{// 10009 현재 몇번이나 CreateTarget을 했는가 (공격을 몇회 했는가)
		if( kAction.IsNil() )	{ return 0; }
		int const iCurHitCnt = GetHitCnt(kAction) + 1;
		kAction.SetParamInt(10009, iCurHitCnt);
		return GetHitCnt(kAction);
	}
	int GetHitCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10009);
	}
	bool IsOverHitCnt(lwAction kAction)
	{// CreateTargetList할때 IsOverHitCnt() 이 true 일때만, 타격을 위한 CreateTarget 할것
		if( kAction.IsNil() )	{ return true; }
		int const iCurHitCnt = GetHitCnt(kAction);
		int const iMaxHitCnt = GetMaxHitCnt(kAction);
		return (iCurHitCnt >= iMaxHitCnt);
	}

	void InitCancelOption(lwAction kAction)
	{// 10010 - 액션 사용중 캔슬할수 있는 상태(EActionCancelType)
		if( kAction.IsNil() )	{ return; }
		int const iType = kAction.GetScriptParamAsInt("CANCEL_TYPE");
		if(EACT_NONE >= iType 
			&& EACT_MAX <= iType
			)
		{
			return;
		}
		kAction.SetParamInt( 10010, iType );
		switch(iType)
		{
		case EACT_ALLOW_SOME_ACTION:
		case EACT_REJECT_SOME_ACTION:
			{
				BM::vstring vStr;
				for(int i=0; i<100; ++i)
				{
					{// 캔슬 허용/불가 액션 이름 목록
						vStr = "CANCEL_ACTION_NAME";
						vStr+=(i+1);
						std::string const kParam = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
						if( kParam.empty() ) { break; }
						SetCheckActionName( kAction, i, kParam.c_str() );
					}
					{// 캔슬 허용/불가를 위한 이펙트가 있는가(아 너무 유연하지 못한데..)
						vStr = "CANCEL_ACTION_EFFECT_NO";
						vStr+=(i+1);
						int const iCheckEffectNo = kAction.GetScriptParamAsInt( vStr.operator const std::string().c_str() );
						if(iCheckEffectNo)
						{
							SetCancelActionCheckEffect(kAction, i, iCheckEffectNo);
						}
					}
				}
			}break;
		}

	}
	int GetCancelOption(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return  kAction.GetParamInt( 10010 );
	}
	bool CheckCancelAction(lwActor kActor, lwAction kAction, lwAction kNextAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( kActor.IsNil() )	{ return false; }
		if( kNextAction.IsNil() )	{ return false; }

		if( IsComboDelayCancle(kAction) &&
			!IsEnableComboKeyEnter(kAction) )
		{//combo_cancel이 tat_end보다 앞에 있을 경우에 OnUpdate에서 전이하는 것보다 키입력으로 전이요청이 빨리 오기 때문에
		//combo_cancel이면서 tat_end를 강제 설정할 때에만 전이 수락하도록 한다.
			return true;
		}
		switch( GetCancelOption(kAction) )
		{
		case EACT_ALLOW_ALL:
			{// 무조건 허용
				return true;
			}break;
		case EACT_ALLOW_SOME_ACTION:
			{// 몇몇 액션만 허용
				for(int i=0; i<100; ++i)
				{
					char const* pcNextActionName = GetCheckActionName(kAction, i);
					if( 0 == ::strcmp("null", pcNextActionName) )
					{
						break;
					}
					if( 0 == ::strcmp( kNextAction.GetID(), pcNextActionName) )
					{// 허용 가능한 액션이지만 
						int const iCheckEffectNo = GetCancelActionEffect(kAction, i);
						if(iCheckEffectNo)
						{// 추가적으로 이펙트를 확인해야할 것이 있다면
							return kActor.CheckEffectExist(iCheckEffectNo, true);
						}
						return true;
					}
				}
				return false;
			}break;
		case EACT_REJECT_SOME_ACTION:
			{// 몇몇 액션만 허용안함
				for(int i=0; i<100; ++i)
				{
					char const* pcNextActionName = GetCheckActionName(kAction, i);
					if( 0 == ::strcmp("null", pcNextActionName) )
					{
						break;
					}
					if( 0 == ::strcmp( kNextAction.GetID(), pcNextActionName) )
					{// 허용 안하는 액션이지만
						int const iCheckEffectNo = GetCancelActionEffect(kAction, i);
						if(iCheckEffectNo)
						{// 추가적으로 이펙트를 확인해야할 것이 있다면
							return !( kActor.CheckEffectExist(iCheckEffectNo, true) );
						}
						return false;
					}
				}
				return true;
			}break;
		case EACT_SPECIFY_TOGGLE_ACTION:
			{
				if( false == kActor.GetActionToggleState( kAction.GetActionNo() )	// 토글이 꺼져있고
					&& kAction.GetSlotCount() == kAction.GetCurrentSlot()+1		// 마지막 애니 중이라면
					)
				{// 캔슬 할수 있다
					return true;
				}
			}break;
		}
		return false;
	}
	
	void SetCheckActionName(lwAction kAction, int const iIdx, char const* pcActionName)
	{// 10011~10110
		if(kAction.IsNil() 
			|| NULL == pcActionName
			)
		{
			return;
		}
		kAction.SetParam(IDX_CHECK_ACTION_NAME+iIdx, pcActionName);
	}
	char const* GetCheckActionName(lwAction kAction, int const iIdx)
	{
		if(kAction.IsNil())
		{
			return "";
		}
		return kAction.GetParam(IDX_CHECK_ACTION_NAME+iIdx);
	}
	
	void InitTransitCheck(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return; }
		int const iType = kAction.GetScriptParamAsInt("TRANSIT_CHECK_TYPE");
		if(EATCT_ALLOW_ALL >= iType 
			&& EATCT_MAX <= iType
			)
		{
			return;
		}
		kAction.SetParamInt( 10120, iType );	// 전이 조건 체크 타입 저장
		switch(iType)
		{
		case EATCT_ALLOW_SOME_ACTION:
		case EATCT_REJECT_SOME_ACTION:
			{
				BM::vstring vStr;
				for(int i=0; i<100; ++i)
				{
					{// 전이를 위해 체크해야할 이전 액션 이름
						vStr = "TRANSIT_CHECK_PREV_ACTION_NAME";
						vStr+=(i+1);
						std::string kParam = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
						if( kParam.empty() ) { break; }
						SetTrasitCheckPrevActionName( kAction, i, kParam.c_str() );
					}
				}
			}break;
		}
	}
	int GetTransitCheckType(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return  kAction.GetParamInt( 10120 );
	}
	bool CheckTransitAction(lwActor kActor, lwAction kAction, lwAction kPrevAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( kActor.IsNil() )	{ return false; }
		//if( kPrevAction.IsNil() )	{ return false; }// Entity같은 경우 이전 액션이 존재할리 없기에 주석처리

		switch( GetTransitCheckType(kAction) )
		{
		case EATCT_ALLOW_ALL:
			{// 무조건 허용
				return true;
			}break;
		case EATCT_ALLOW_SOME_ACTION:
			{// 몇몇 액션만 허용
				if( kPrevAction.IsNil() )
				{
					return false; 
				}
				for(int i=0; i<100; ++i)
				{
					char const* pcPrevActionName = GetTrasitCheckPrevActionName(kAction, i);
					if( 0 == ::strcmp("null", pcPrevActionName) )
					{
						break;
					}
					if( 0 == ::strcmp(kPrevAction.GetID(), pcPrevActionName) )
					{// 전이가 가능한 액션
						return true;
					}
				}
				return false;
			}break;
		case EATCT_REJECT_SOME_ACTION:
			{// 몇몇 액션만 허용안함
				if( kPrevAction.IsNil() )
				{
					return false; 
				}
				for(int i=0; i<100; ++i)
				{
					char const* pcPrevActionName = GetTrasitCheckPrevActionName(kAction, i);
					if(0 == ::strcmp("null", pcPrevActionName) )
					{
						break;
					}
					if( 0 == ::strcmp(kPrevAction.GetID(), pcPrevActionName) )
					{// 전이 시키면 안되는 액션
						return false;
					}
					else if( 0 == ::strcmp("a_jump", pcPrevActionName) )
					{
						if(!kActor.IsMeetFloor())
						{
							return false;
						}
					}
				}
				return true;
			}break;
		}
		return false;
	}
	void SetTrasitCheckPrevActionName(lwAction kAction, int const iIdx, char const* pcActionName)
	{// 10121~10220 - 다음 액션을 사용하기 위해 현재 액션 네임 체크
		if(kAction.IsNil() 
			|| NULL == pcActionName
			)
		{
			return;
		}
		kAction.SetParam(IDX_CHECK_PREV_ACTION_NAME+iIdx, pcActionName);
	}
	char const* GetTrasitCheckPrevActionName(lwAction kAction, int const iIdx)
	{
		if(kAction.IsNil())
		{
			return "";
		}
		return kAction.GetParam(IDX_CHECK_PREV_ACTION_NAME+iIdx);
	}
	
	void SetCancelActionCheckEffect(lwAction kAction, int const iIdx, int iCheckEffectNo)
	{// 10221 ~ 10320 - 현재 액션을 캔슬하기 위해 현재 이펙트 체크
		if(kAction.IsNil() 
			|| 0 == iCheckEffectNo
			)
		{
			return;
		}
		kAction.SetParamInt(IDX_CANCEL_ACTION_EFFECT_NO+iIdx, iCheckEffectNo);
	}
	int GetCancelActionEffect(lwAction kAction, int const iIdx)
	{
		if(kAction.IsNil())
		{
			return 0;
		}
		return kAction.GetParamInt(IDX_CANCEL_ACTION_EFFECT_NO+iIdx);
	}
	
	int IncMsgCamCnt(lwAction kAction)
	{// 10500 - 애니메이션 중 QuakeCamera 메세지인 'cam' Event Msg가 몇번 들어왔는가?
		if(kAction.IsNil())
		{
			return 0;
		}
		int const iCurCnt = GetMsgCamCnt(kAction) + 1;
		kAction.SetParamInt(10500, iCurCnt);
		return GetMsgCamCnt(kAction);
	}
	int GetMsgCamCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10500);
	}

	void InitUseTrail(lwAction kAction)
	{// 10800 - Trail 을 사용하는가
		if(kAction.IsNil())
		{
			return;
		}
		if( STR_TRUE ==  kAction.GetScriptParam("USE_TRAIL") )
		{
			kAction.SetParamInt(10800, 1);
		}
	}
	bool IsUseTrail(lwAction kAction)
	{
		if(kAction.IsNil())
		{
			return false;
		}
		return ( 0 < kAction.GetParamInt(10800) );
	}
	void SetTrailUseFlag(lwAction kAction, int const iFlag)
	{// 10801 - Trail 어떤때 Trail을 쓰는가
		if(kAction.IsNil())
		{
			return;
		}
		kAction.SetParamInt(10801, iFlag);
	}
	int GetTrailUseFlag(lwAction kAction)
	{
		if(kAction.IsNil())
		{
			return 0;
		}
		return kAction.GetParamInt(10801);
	}
	void ProcessTrail(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil()
			|| kAction.IsNil()
			)
		{
			return;
		}
		if( false == IsUseTrail(kAction) )
		{
			return;
		}

		
		switch( eActionTime )
		{
		case EAT_ANI_SLOT_TIME:
			{
				int const iCurAniSlot = kAction.GetCurrentSlot();
				BM::vstring kCnt("TRAIL_CNT_WHEN_ANI_SLOT"); kCnt+=iCurAniSlot;
				int const iMax = kAction.GetScriptParamAsInt(kCnt.operator const std::string().c_str());
				for(int i=1; i <= iMax; ++i)
				{
					BM::vstring kTrailType("TRAIL_TYPE_WHEN_ANI_SLOT"); kTrailType+=iCurAniSlot; kTrailType+="_"; kTrailType+=i;
					int const iType = kAction.GetScriptParamAsInt(kTrailType.operator const std::string().c_str());
					if(ETT_NONE >= iType 
						|| ETT_MAX <= iType
						)
					{
						return;
					}
					
					BM::vstring kPath("TRAIL_PATH_WHEN_ANI_SLOT"); kPath+=iCurAniSlot; kPath+="_"; kPath+=i;
					std::string const kPathParam = kAction.GetScriptParam(kPath.operator const std::string().c_str());
					
					BM::vstring kTotalTime("TRAIL_TOTAL_TIME_WHEN_ANI_SLOT"); kTotalTime+=iCurAniSlot; kTotalTime+="_"; kTotalTime+=i;
					int const iTotalTime = static_cast<int>( kAction.GetScriptParamAsFloat(kTotalTime.operator const std::string().c_str()) * 1000.0f );

					BM::vstring kBrightTime("TRAIL_BRIGHT_TIME_WHEN_ANI_SLOT"); kBrightTime+=iCurAniSlot; kBrightTime+="_"; kBrightTime+=i;
					int const iBrightTime = static_cast<int>( kAction.GetScriptParamAsFloat(kBrightTime.operator const std::string().c_str()) * 1000.0f );

					kActor.StartTrail(iType-1, ( kPathParam.empty() ? STR_DEFALT_TRAIL_TEX.c_str() : kPathParam.c_str() ), iTotalTime, iBrightTime );
					int const iFlag = GetTrailUseFlag(kAction);
					SetTrailUseFlag(kAction, iFlag | 1<<iType);
				}
			}break;
		case EAT_ON_CLEANUP:
			{
				int const iFlag = GetTrailUseFlag(kAction);
				if(1 << ETT_BODY_TRAIL & iFlag)
				{
					kActor.EndTrail(PgActor::ETAT_BODY);
				}
				if(1 << ETT_R_HAND_TRAIL & iFlag)
				{
					kActor.EndTrail(PgActor::ETAT_R_HAND);
				}
				if(1 << ETT_L_HAND_TRAIL & iFlag)
				{
					kActor.EndTrail(PgActor::ETAT_L_HAND);
				}
				if(1 << ETAT_R_FOOT_TRAIL & iFlag)
				{
					kActor.EndTrail(PgActor::ETAT_R_FOOT);
				}
				if(1 << ETAT_L_FOOT_TRAIL & iFlag)
				{
					kActor.EndTrail(PgActor::ETAT_L_FOOT);
				}
			}break;
		//case AT_CAST_COMPLETE:
		//case AT_FIRE_MSG:
		//case AT_HIT_MSG:
		//case AT_ON_ENTER:
		//case AT_ANI_COMPLETE:
		default:
			{
				return;
			}break;
		}
	}

	// 11000 - 이동시작 on/off
	// 11008 - 이동 타입을 설정
	// 11001 - 이동할 목표점을 저장
	// 11002 - 이동 시작 위치를 저장
	// 11003 - 애니메이션 Text Key에서, 몇번째 "move_start"가 들어왔는가
	// 11004 - 이동이 시작되는 시간(보간 완료시간)
	// 11005 - 이동이 완료될 시간(보간 완료시간)
	// 11006 - 이동 보간중에 StopJump를 해야 하는가
	// 11007 - 이동 보간중에 바닥을 만났을때 멈춰야 하는가
	// 11009 - MovingDelta값 세팅
	// 11010 - GoalPos를 계속해서 갱신 할것인가
	// 11011 - GoalPos를 계산 하는데 사용된 대상의 TargetGuid
	// 11012 - Target의 위치로부터 더해줄 거리벡터의 방향
	// 11013 - Target의 위치로부터 더해줄 거리벡터의 크기(위치보간 - 거리 / 속도 - Speed )
	// 11014 - 이동중 방향 업데이트를 사용하는가
	// 11015 - 목표점계산시 기준 위치에 더해줄 이격 방향
	// 11016 - 속도를 통한 이동시, 속력을 저장

	int IncOrderActionCnt(lwAction kAction)
	{// 11020 - 몇번째 다른 대상에 액션을 시키는(ORDER_ACTION)이 들어왔는가
		if( kAction.IsNil() )	{ return 0; }
		int const iCurCnt = GetOrderActionCnt(kAction);
		int const iNewCnt = iCurCnt+1;
		kAction.SetParamInt( 11020, iNewCnt );
		return iNewCnt;
	}
	int GetOrderActionCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 11020 );
	}
	
	bool SetOrderActionTargetType(lwAction kAction, int const iIdx, char const* pcTargetType)
	{// 11021 - 11030 어떤 다른 대상인가를  저장(ORDER_ACTION_TARGET)
		if( kAction.IsNil() 
			|| NULL == pcTargetType 
			|| 0 > iIdx 
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return false;
		}
		int iOrderActionTargetType = static_cast<int>(EOAT_NONE);
		std::string const kParam(pcTargetType);
		if( kParam == STR_SC )
		{// SUB_PLAYER 보조 캐릭터류
			iOrderActionTargetType = static_cast<int>(EOAT_SUB_PLAYER);
		}
			
		kAction.SetParamInt(IDX_ORDER_ACTION_TARGET+iIdx, iOrderActionTargetType);
		return true;
	}
	int GetOrderActionTargetType(lwAction kAction, int const iIdx)
	{
		if( kAction.IsNil()
			|| 0 > iIdx
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return 0;
		}
		return kAction.GetParamInt(IDX_ORDER_ACTION_TARGET+iIdx);
	}
	
	bool SetOrderActionTime(lwAction kAction, int const iIdx, char const* pcActionTime)
	{// 11031 - 11040 어떤 ActionTime에 액션을 하라고 전달 할것인가
		if( kAction.IsNil()
			|| NULL == pcActionTime
			|| 0 > iIdx
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return false;
		}
		int iActionTime = static_cast<int>(EAT_NONE);
		std::string const kParam(pcActionTime);
		if( kParam == STR_ENTER )
		{// OnEnter 시에
			iActionTime = static_cast<int>(EAT_ON_ENTER);
		}
		else if( kParam == STR_MSG_SC_START )
		{// "sc_start" Event key 시에(애니메이션 이벤트키)
			iActionTime = static_cast<int>(EAT_MSG_SC_START);
		}
		else if( kParam == STR_CLEANUP)
		{// OnCleanUp 시에
			iActionTime = static_cast<int>(EAT_ON_CLEANUP);
		}
		else if( std::string::npos != kParam.find(STR_ANI_SLOT) )
		{
			std::string kAniSlotNo = kParam.substr( STR_ANI_SLOT.length() );
			int iSlot = PgStringUtil::SafeAtoi(kAniSlotNo);
			iActionTime = iSlot+ static_cast<int>( EAT_ANI_SLOT0 );
		}
		else
		{
			return false;
		}
		kAction.SetParamInt(IDX_ORDER_ACTION_TIME+iIdx, iActionTime);
		return true;
	}
	int GetOrderActionTime(lwAction kAction, int const iIdx)
	{
		if( kAction.IsNil()
			|| 0 > iIdx
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return 0;
		}
		return kAction.GetParamInt(IDX_ORDER_ACTION_TIME+iIdx);
	}
	
	bool SetOrderActionName(lwAction kAction, int const iIdx, char const* pcActionName)
	{// 11041 - 11050 ORDER_ACTION_TARGET 대상에게 ActionTime에 시킬 액션 이름은 무엇인가
		if( kAction.IsNil() 
			|| NULL == pcActionName
			|| 0 > iIdx
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return false;
		}
		kAction.SetParam(IDX_ORDER_ACTION_NAME+iIdx, pcActionName);
		return true;
	}
	char const* GetOrderActionName(lwAction kAction, int const iIdx)
	{
		if( kAction.IsNil()
			|| 0 > iIdx
			|| IDX_ORDER_ACTION_IDX_MAX < iIdx
			)
		{
			return "";
		}
		return kAction.GetParam(IDX_ORDER_ACTION_NAME+iIdx);
	}

	void InitOrderActionInfo(lwAction kAction)
	{// 자신의 소유 대상에게 어떤 액션을 걸기 위해
		if( kAction.IsNil() ) { return; }
		for(int i=0; i < IDX_ORDER_ACTION_IDX_MAX; ++i)
		{// 정보를 읽어오는데
			// 어느 시점에
			int const iIdx = i+1;								// idx는 1부터 시작
			BM::vstring vStr("ORDER_ACTION_TIME_");	vStr+=iIdx;
			std::string kOrderActionTime = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( kOrderActionTime.empty() )
			{// 필요한 정보들이 없다면 거기서 종료
				return;
			}
			// 어떤 대상에
			vStr = "ORDER_ACTION_TARGET_";	vStr+=iIdx;
			std::string kOrderActionTarget = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( kOrderActionTarget.empty() )
			{
				return;
			}
			// 어떤 액션을
			vStr = "ORDER_ACTION_NAME_";	vStr+=iIdx;
			std::string kOrderActionName = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( kOrderActionName.empty() )
			{
				return;
			}
			// 하게 할 것인지 저장한다
			SetOrderActionTime( kAction, i, kOrderActionTime.c_str() );
			SetOrderActionTargetType( kAction, i, kOrderActionTarget.c_str() );
			SetOrderActionName( kAction, i, kOrderActionName.c_str() );
		}
	}
	void ProcessOrderAction(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kAction.IsNil() || kActor.IsNil() ) { return; }
		int const iIdx = GetOrderActionCnt(kAction);
		EActionTime const eCurParam = static_cast<EActionTime>( GetOrderActionTime(kAction, iIdx) );
		if( eActionTime == eCurParam )
		{// 시점이 맞다면
			IncOrderActionCnt(kAction);	// 다음 Order액션을 준비하고
			std::string const kOrderActionName = GetOrderActionName(kAction, iIdx);	// 액션 이름을 얻어오고
			EOrderActionTarget const eOrderActionTarget = static_cast<EOrderActionTarget>( GetOrderActionTargetType(kAction, iIdx) );
			PgActor* pkTargetActor = NULL;
			switch( eOrderActionTarget )
			{// 액션을 시킬 대상을 얻어와
			case EOAT_SUB_PLAYER:
				{
					lwActor kSubActor = kActor.GetSubPlayer();
					pkTargetActor = kSubActor();
				}break;
			default:
				{
					return;
				}break;
			}
			if( !pkTargetActor )
			{
				return;
			}
			//전이 가능한 타겟 액션 체크 (OR 체크여야 함)
			bool bUseAllowAction = false;
			bool bAllowAction = false;
			int iCount = 1;
			BM::vstring kAllowTargetActionName("ORDER_ACTION_ALLOW_TARGET_ACTION_NAME_#IDX#_#COUNT#");
			kAllowTargetActionName.Replace(L"#IDX#", iIdx);
			kAllowTargetActionName.Replace(L"#COUNT#", iCount);
			std::string kAllowActionName = kAction.GetScriptParam( MB(kAllowTargetActionName) );
			if( 0 != kAllowActionName.size() )
			{
				bUseAllowAction = true;
			}
			while( 0 != kAllowActionName.size() )
			{
				PgAction *pkTargetAction = pkTargetActor->GetAction();
				if( NULL == pkTargetAction )
				{
					return;
				}
				std::string kTargetActionName;
				if( false == pkTargetAction->GetActionName(kTargetActionName) )
				{
					return;
				}
				if( 0 == strcmp( kAllowActionName.c_str(), kTargetActionName.c_str() ) )
				{
					bAllowAction = true;
					break;
				}
				else
				{
					bAllowAction = false;
				}
				++iCount;
				kAllowTargetActionName = "ORDER_ACTION_ALLOW_TARGET_ACTION_NAME_#IDX#_#COUNT#";
				kAllowTargetActionName.Replace(L"#IDX#", iIdx);
				kAllowTargetActionName.Replace(L"#COUNT#", iCount);
				kAllowActionName = kAction.GetScriptParam( MB(kAllowTargetActionName) );
			}
			if( true == bUseAllowAction 
				&& false == bAllowAction)
			{
				return;
			}
			// 액션을 시킨다.
			TryMustChangeActorAction( pkTargetActor, kOrderActionName.c_str() );
		}
	}

	bool InitAttackTimer(lwAction kAction)
	{//// 12000 - Timer
		if( kAction.IsNil() )	{ return false; }
		std::string const kStartTime = kAction.GetScriptParam("WHEN_ATTACK_TIMER_START");
		if( kStartTime.empty() )
		{
			return false;
		}

		if( kStartTime == "ENTER" )
		{
			kAction.SetParamInt(12000, EAT_ON_ENTER);
		}
		if( kStartTime == "MSG_HIT" )
		{
			kAction.SetParamInt(12000, EAT_MSG_HIT);
		}
				
		return true;
	}
	bool IfAbleAttackTimerThenStart(lwAction kAction, EActionTime const eActionTime)
	{
		if( kAction.IsNil() )	{ return false; }
		EActionTime const eCurParam = static_cast<EActionTime>( kAction.GetParamInt(12000) );
		if( eActionTime == eCurParam )
		{
			float const fTimerTime = PgStringUtil::SafeAtof( kAction.GetScriptParam("ATTACK_TIMER_TIME") );
			float const fTimerTick = PgStringUtil::SafeAtof( kAction.GetScriptParam("ATTACK_TIMER_TICK") );
			if(0 >= fTimerTime 
				|| 0 >= fTimerTick
				)
			{
				return false;
			}
			kAction.StartTimer(fTimerTime, fTimerTick, ATTACK_TIMER_ID);	// 0
			return true;
		}
		return false;
	}
	bool IsUseAttackTimer(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(12000);
	}
//------------------------------------------------

	void DefaultHitOneTime(lwActor kActor, lwAction kAction, bool bApplyDmg)
	{// 때릴때 적이 데미지 받는 부분등을 표현해주는 기능
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }

		//int iHitCount = lua_tinker::call<int, lwAction>("SkillHelpFunc_GetHitCount", kAction);
		//if( 0 == iHitCount )
		//{// 때릴때 캐스터에게 붙일 파티클 표현 하고
		//	lua_tinker::call<void, lwActor, lwAction>("SkillHelpFunc_ExpressHitCasterEffect", kActor, kAction);
		//}

		// 때릴때 사운드
		lwCommonSkillUtilFunc::PlayHitSound( kActor, kAction );
		
		PgActionTargetList* pkTargetList = kAction.GetTargetList()();
		if(NULL == pkTargetList)
		{
			return;
		}
		bool bWeaponSoundPlayed = false;
		ActionTargetList::iterator kItor = pkTargetList->GetList().begin();

		while( pkTargetList->end() != kItor )
		{
			lwActionResult kActionResult( &(*kItor).GetActionResult() );

			if(kActionResult.IsNil() == false
				&& kActionResult.IsMissed() == false
				)
			{
				lwGUID kTargetGUID = (*kItor).GetTargetPilotGUID();
				lwPilot kTargetPilot = g_kPilotMan.FindPilot( kTargetGUID() );

				if(kTargetPilot.IsNil() == false)
				{
					lwActor kTargetActor = kTargetPilot.GetActor();
					if (bWeaponSoundPlayed == false )
					{
						bWeaponSoundPlayed = true;
						lwCommonSkillUtilFunc::PlayWeaponSound( kActor, kAction, kActionResult.GetCritical(), kTargetActor, &(*kItor) );
					}
					if( true == GetUseDmgEffect(kAction) )
					{
						lwCommonSkillUtilFunc::DefaultDoDamage(kActor, kTargetActor, kActionResult, &(*kItor) );
					}
				}
			}
			++kItor;
		}

		if( kActor.IsUnderMyControl()
			&& !bApplyDmg
			)
		{
			pkTargetList->ApplyActionEffects(true, true, false);
		}
		else
		{
			pkTargetList->ApplyActionEffects(false, false, false);
			kActor.ClearIgnoreEffectList();
		}
	}
	
	int SkillHelpFunc_GetTotalHitCount(lwAction kAction)
	{//-- 총 때려야할 횟수(TotalHit)
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetAbil(AT_COUNT);
	}

	void DoDividedDamage(lwActor kActor, lwAction kAction, int iHitCount)
	{// 실제는 1방 때린것이지만, 클라이언트에서 여러대 때리는것으로 연출하는 함수
		if( kAction.IsNil() )	{ return; }
		if( kActor.IsNil() )	{ return; }
		
		//DoDividedDamage를 현재 스크립트에서만 사용하고 있어서 GetMaxHitCnt를 사용하고 있지 않다.
		int iTotalHit = kAction.GetAbil(AT_COUNT);//GetMaxHitCnt(kAction);	
		if(0 >= iTotalHit)
		{
			return;
		}

		PgActionTargetList* pkTargetList = kAction.GetTargetList()();
		if(pkTargetList)
		{
			bool bWeaponSoundPlayed = false;
			ActionTargetList::iterator kItor = pkTargetList->GetList().begin();

			while( pkTargetList->end() != kItor )
			{
				lwActionResult kActionResult( &(*kItor).GetActionResult() );

				if(kActionResult.IsNil() == false
					&& kActionResult.IsMissed() == false
					)
				{
					if( iHitCount == iTotalHit-1 ) 
					{
						int const iOneDmg = kActionResult.GetValue() / iTotalHit;
						//SkillHelpFunc_SetDividedDamage(kAction, iOneDmg);	// 이게 왜 있어야 하는가? 추후에 봐서 사용되는곳이 없으면 닦아버릴것
						kActionResult.SetValue( kActionResult.GetValue() - iOneDmg*(iTotalHit-1));
					}

					lwGUID kTargetGUID = (*kItor).GetTargetPilotGUID();
					lwPilot kTargetPilot = g_kPilotMan.FindPilot( kTargetGUID() );

					if( false == kTargetPilot.IsNil() )
					{
						lwActor kTargetActor = kTargetPilot.GetActor();
						if( false == bWeaponSoundPlayed )
						{
							bWeaponSoundPlayed = true;
							lwCommonSkillUtilFunc::PlayWeaponSound( kActor, kAction, kActionResult.GetCritical(), kTargetActor, &(*kItor) );
						}
						if( true == GetUseDmgEffect(kAction) )
						{
							lwCommonSkillUtilFunc::DefaultDoDamage(kActor, kTargetActor, kActionResult, &(*kItor) );
						}
					}
				}
				++kItor;
			}

			if( iHitCount == iTotalHit-1 ) 
			{//ODS("쪼개진 데미지 막타 적용\n", false, 912)	
				//kAction.GetTargetList():ApplyActionEffects();
				pkTargetList->ApplyActionEffects();
			}
			else
			{//ODS("쪼개진 데미지 적용중\n", false, 912)
				//kAction.GetTargetList():ApplyOnlyDamage(iTotalHit);
				pkTargetList->ApplyOnlyDamage(iTotalHit);
			}
		}
	}

	void PlayHitSound(lwActor kActor, lwAction kAction)
	{//때릴때 사운드
		if( kActor.IsNil() ) { return; }
		if( kAction.IsNil() ) { return; }
		std::string kSoundID = kAction.GetScriptParam("HIT_SOUND_ID");
		if( false == kSoundID.empty() )
		{
			kActor.AttachSound( IDX_PLAY_HIT_SOUND, kSoundID.c_str() , 0.0f );
		}
		if( UG_MALE == kActor.GetAbil(AT_GENDER) )
		{
			std::string kMaleSoundID = kAction.GetScriptParam("HIT_SOUND_ID_MALE");
			if( false == kMaleSoundID.empty() )
			{//남성용 사운드
				kActor.AttachSound( IDX_PLAY_HIT_GENDER_SOUND, kMaleSoundID.c_str(), 0.0f );
			}
			else
			{//여성용 사운드
				std::string kFemaleSoundID = kAction.GetScriptParam("HIT_SOUND_ID_FEMALE");
				if( false == kFemaleSoundID.empty() )
				{
					kActor.AttachSound( IDX_PLAY_HIT_GENDER_SOUND, kFemaleSoundID.c_str(), 0.0f );
				}
			}
		}
	}

	void PlayWeaponSound(lwActor kActor, lwAction kAction, bool const bCritical, lwActor kTargetActor, lwActionTargetInfo kTargetInfo)
	{// 타격시 무기 소리를 재생하는 부분
		if( kActor.IsNil() ) { return; }
		if( kAction.IsNil() ) { return; }
		std::string kActionName;
		if (bCritical)
		{
			kActionName = "Critical_dmg";
		}
		else
		{
			kActionName = kAction.GetID();
		}
		kTargetActor.PlayWeaponSound(PgWeaponSoundManager::WST_HIT, kActor, kActionName.c_str(), 0, kTargetInfo);
	}

	void DefaultDoDamage(lwActor kActor, lwActor kActorTarget,lwActionResult kActionResult,lwActionTargetInfo kActionTargetInfo)
	{// 대상이 맞을때 흔들리거나, 크리티컬 표시를 내주는 부분
		if( kActor.IsNil() ) { return; }
		if( kActorTarget.IsNil() ) { return; }
		if( kActionResult.IsNil() ) { return; }
		if( kActionTargetInfo.IsNil() ) { return; }
		lwAction kAction = kActor.GetAction();
		if( kAction.IsNil() ) { return; }
		//std::string kActionID = kAction.GetID();

		if( kActor.GetPilotGuid().IsEqual(kActorTarget.GetPilotGuid()) ) 
		{
			return;
		}

		int iABVIndex = kActionTargetInfo.GetABVIndex();
		lwPoint3 pt = kActorTarget.GetABVShapeWorldPos(iABVIndex);
			//타겟이 맞을때 붙는 이펙트
		//SkillHelpFunc_ExpressHitTargetEffect(kActor, kAction, kActorTarget);

		ProcessEtc(kActor, kAction, EAT_HIT_TARGET);

		if( 0 != strcmp(kAction.GetScriptParam("NO_DEFAULT_HIT_EFFECT"), "TRUE") ) 
		{
			int const iHitTargetEffectOffset = kAction.GetScriptParamAsInt("HIT_TARGET_EFFECT_RANDOM_OFFSET");

			if( 0 != iHitTargetEffectOffset ) 
			{
				pt.SetX(pt.GetX() + static_cast<float>( BM::Rand_Range(iHitTargetEffectOffset,-iHitTargetEffectOffset) ));
				pt.SetY(pt.GetY() + static_cast<float>( BM::Rand_Range(iHitTargetEffectOffset,-iHitTargetEffectOffset) ));
				pt.SetZ(pt.GetZ() + static_cast<float>( BM::Rand_Range(iHitTargetEffectOffset,-iHitTargetEffectOffset) ));
			}

			int	iHitCount = 0;//SkillHelpFunc_GetHitCount(action);

			std::string kHitTargetEffectID = kAction.GetScriptParam("HIT_TARGET_EFFECT_ID");
			std::string kCriticalHitTargetEffectID = kAction.GetScriptParam("CRITICAL_HIT_TARGET_EFFECT_ID");
			std::string kHitTargetEffectTargetNodeID = kAction.GetScriptParam("HIT_TARGET_EFFECT_TARGET_NODE");
			
			bool bCritical = kActionResult.GetCritical();
			char const* pcDamageEff = kActor.GetDamageEffectID(bCritical);
			if( bCritical )
			{
				if( 0 != kCriticalHitTargetEffectID.size() )
				{
					pcDamageEff = kCriticalHitTargetEffectID.c_str();
				}
				else
				{
					pcDamageEff = "e_dmg_cri";
				}
			}
			else
			{
				if( 0 != kHitTargetEffectID.size() )
				{
					pcDamageEff = kHitTargetEffectID.c_str();
				}
				else
				{
					pcDamageEff = "e_dmg";
				}
			}
			if( false == kHitTargetEffectTargetNodeID.empty() ) 
			{
				lwPoint3 kPos = kActorTarget.GetNodeWorldPos( kHitTargetEffectTargetNodeID.c_str() );
				kActorTarget.AttachParticleToPoint(12+iHitCount, kPos, pcDamageEff);
			}
			else
			{
				kActorTarget.AttachParticleToPoint(12 + iHitCount, pt, pcDamageEff);
			}

			char const* pcDamageHold = kAction.GetScriptParam("NO_DAMAGE_HOLD");
			if( 0 == ::strcmp(pcDamageHold, "FALSE") )
			{// 충격 효과 적용
				float const fHoldTimeParam = kAction.GetScriptParamAsFloat("DAMAGE_HOLD_TIME");
				float const fHoldTime = 0 < fHoldTimeParam ? fHoldTimeParam : DEFAULT_ATTACK_HIT_HOLDING_TIME;
				int const iShakeTime = static_cast<int>(fHoldTime * 1000.0f);
				kActor.SetAnimSpeedInPeriod(0.01f, iShakeTime);
				kActor.SetShakeInPeriod(5.0f, iShakeTime/2);
				kActorTarget.SetShakeInPeriod(5.0f, iShakeTime);
				// 위에것으로 써야하지만, 변수화 되어있지 않으니 변수화가 필요할때까지 그냥 숫자로 쓴다
				//kActor.SetAnimSpeedInPeriod(0.01f, 145);
				//kActor.SetShakeInPeriod(5.0f, 72);
				//kActorTarget.SetShakeInPeriod(5.0f, 145);
			}
		}
	}

	void AttachLinkageUseSkillEffect(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{// 연계 포인트를 사용 하는 스킬이라면, 발동 이펙트 등을 붙여줄수 있게 처리
		if( kAction.IsNil() )	{ return; }		
		if( 0 >= kAction.GetAbil(AT_USE_SKILL_LINKAGE_POINT) ) { return; }
		int const iLinkagePoint = kActor.GetAbil( AT_SKILL_LINKAGE_POINT );
		if( 0 >= iLinkagePoint ) { return; }
		if( kActor.IsNil() )	{ return; }

		switch( eActionTime )
		{// 연계 포인트 사용 스킬은
		case EAT_MSG_HIT:
			{// 처음 타격시에 적용 되므로 
				if( !GetCheckLinkageUseSkillEffect(kAction) )
				{// 이펙트가 붙지 않았으면(hit 메세지가 여러개일 수 있으므로)
					SetCheckLinkageUseSkillEffect(kAction, true); // 이미 한번 붙였다고 해둬서 안뜨게 하고
					std::string const kSoundID = kAction.GetScriptParam("USE_LINKAGE_SOUND");

					BM::vstring vStr("USE_LINKAGE_EFFECT_PT");
					vStr+=iLinkagePoint;
					std::string const kEffectID = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
					
					vStr="USE_LINKAGE_EFFECT_TARGET_PT";
					vStr+=iLinkagePoint;
					std::string const kTargetNode = kAction.GetScriptParam( vStr.operator const std::string().c_str() );

					if( !kSoundID.empty() )
					{
						kActor.AttachSound( 0, kSoundID.c_str(), 0.0f );
					}

					if( !kEffectID.empty() )
					{
						lwPoint3 kPos = kActor.GetNodeTranslate( kTargetNode.c_str() );
						kActor.AttachParticleToPointS( 20110720, kPos, kEffectID.c_str(), 1.0f );
					}
				}
			}break;
		}
	}

	void SetCheckLinkageUseSkillEffect(lwAction kAction, bool const bUsed)
	{// 10802 스킬 체크 이펙트를 이미 한번 사용했는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10802, (bUsed ? 1 : 0) );
	}
	bool GetCheckLinkageUseSkillEffect(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return true; } // 이미 한번 붙였다고 해둬서 안뜨게끔
		return ( 0 < kAction.GetParamInt(10802) );
	}
	
	void InitSeeFrontInfo(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( 0 < kAction.GetScriptParamAsInt("SEE_FRONT") )
		{
			SetSeeFront(kAction, true);
			kActor.LockBidirection(false);	// 2방향 잠금을 풀고
			kActor.SeeFront(true, true);
		}
	}

	void SetSeeFront(lwAction kAction, bool const bSeeFront)
	{// 10810 정면 바라보기 설정
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10810, (bSeeFront ? 1 : 0) );
	}
	bool IsSeeFront(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return ( 0 < kAction.GetParamInt(10810) );
	}
	
	int IncMsgAlphaCnt(lwAction kAction)
	{// 10811 alpha_start 메세지가 몇번이나 들어왔는가
		if( kAction.IsNil() )	{ return 0; }
		int const iNowCnt = GetMsgAlphaCnt(kAction)+1;
		kAction.SetParamInt(10811, iNowCnt);
		return iNowCnt;
	}
	int GetMsgAlphaCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10811);
	}
	
	void InitDrawActorFilter(lwAction kAction)
	{// 특정 액터만 화면에 표시 하는것을 사용한다면 정보를 읽어옴
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10812, kAction.GetScriptParamAsInt("DRAW_ACTOR_FILTER_USE") );
		kAction.SetParamInt( 10813, kAction.GetScriptParamAsInt("DRAW_ACTOR_FILTER_CLEAR") );
	}
	bool IsUseDrawActorFilter(lwAction kAction)
	{// 10812 특정 액터만 화면에 표시 하는것을 사용하는가?
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10812);
	}
	bool IsClearDrawActorFilter_OnCleanUp(lwAction kAction)
	{// CleanlUp시에 원상 복귀 하는가? ( 실제 액션 타임을 얻어와 하는게 좋지만, 그렇게 필요하게 되면 확장함)
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10813);
	}
	void ProcessDrawActorFilter(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{// 특정 액터만 골라서 화면에 보여주는것을 처리하는데
		if(kActor.IsNil() || kAction.IsNil() )
		{
			return;
		}
		switch(eActionTime)
		{// 액션 타임과 실행 해야할 타임을 저장해두고, 현재 액션 타임과 저장해 액션타임이 맞을때
		 // 실행하는것이 확장성이 제일 좋지만, 그렇게 까진 쓸일이 없을것 같아 이렇게 준비만 해둠
		case EAT_ON_ENTER:
			{
				if( IsUseDrawActorFilter(kAction)
					&& kActor.IsMyActor()
					&& g_pkWorld
					)
				{
					g_pkWorld->AddDrawActorFilter( kActor.GetPilotGuid()() );
					lwActor kSubPlayer = kActor.GetSubPlayer();
					if( false == kSubPlayer.IsNil() )
					{
						g_pkWorld->AddDrawActorFilter( kSubPlayer.GetPilotGuid()() );
					}

					kAction.CreateActionTargetList(kActor, true);
					PgActionTargetList* pkTargetList = kAction.GetTargetList()();
					if(pkTargetList)
					{
						ActionTargetList const& kCont = pkTargetList->GetList();
						ActionTargetList::const_iterator kItor = kCont.begin();
						while( kItor != kCont.end() )
						{
							g_pkWorld->AddDrawActorFilter( (*kItor).GetTargetPilotGUID() );
							++kItor;
						}
					}
					kAction.ClearTargetList();
				}
			}break;
		case EAT_ON_CLEANUP:
			{
				if( IsClearDrawActorFilter_OnCleanUp(kAction)
					//&& IsUseDrawActorFilter(kAction)
					&& g_pkWorld
					)
				{
					g_pkWorld->ClearDrawActorFilter();
				}
			}break;
		default:
			{
			}break;
		}
	}
	
	void InitIsBanSubPlayerAction(lwActor kActor, lwAction kAction)
	{
		if( kAction.IsNil() )
		{
			return;
		}
		if( 0 < kAction.GetScriptParamAsInt("BAN_USER_INPUT_SC_SKILL") )
		{
			kAction.SetParamInt(10814, 1);
		}
	}
	
	bool IsBanSubPlayerAction(lwAction kAction)
	{// 10814 kAction중 SC스킬을 쓸수 있는가?
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt( 10814 );
	}
	
	void InitWorldFocusFilterColorAlpha(lwActor kActor, lwAction kAction)
	{// 화면 어두워 지게 바꾸기
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		
		int const iActionTime = kAction.GetScriptParamAsInt("WFF_COLOR_ALPHA_ACTION_TIME");
		if(0 >= iActionTime) 
		{// 10826 어느 시점에
			return;
		}
		SetWorldFoucsFilterColorAlpha_ActionTime(kAction, iActionTime);
		
		int const iTarget = kAction.GetScriptParamAsInt("WFF_COLOR_ALPHA_TARGET");
		if(0 >= iTarget) 
		{// 10821 대상이 누구일때(나, 모두)
			return;
		}
		SetWorldFoucsFilterColorAlpha_Target(kAction, iTarget);
		
		float const fTotal = kAction.GetScriptParamAsFloat("WFF_COLOR_ALPHA_TOTAL");
		if(0.0f >= fTotal) 
		{// 10825 몇초에 걸쳐서
			return;
		}
		SetWorldFoucsFilterColorAlpha_TotalTime(kAction, fTotal);

		// 10822 무슨 색으로
		int const iColor = kAction.GetScriptParamAsInt("WFF_COLOR_ALPHA_COLOR");
		SetWorldFoucsFilterColorAlpha_Color(kAction, iColor);

		// 10823 alpha 몇에서
		float const fAlphaStart = kAction.GetScriptParamAsFloat("WFF_COLOR_ALPHA_START");
		SetWorldFoucsFilterColorAlpha_AlphaStart(kAction, fAlphaStart);

		// 10824 alpha 몇으로
		float const fAlphaEnd = kAction.GetScriptParamAsFloat("WFF_COLOR_ALPHA_END");
		SetWorldFoucsFilterColorAlpha_AlphaEnd(kAction, fAlphaEnd);
		
		// 10827 색바꾼것(필터)를 유지 할것인가, 자동적으로 사라질것인가
		int const iIsKeep = kAction.GetScriptParamAsInt("WFF_COLOR_ALPHA_KEEP");
		SetWorldFoucsFilterColorAlpha_Keep(kAction, iIsKeep);
	}
	
	void SetWorldFoucsFilterColorAlpha_Target(lwAction kAction, int const iTargetType)
	{// 10821 대상이 누구일때(나, 모두)
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10821, iTargetType );
	}
	int GetWorldFoucsFilterColorAlpha_Target(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 10821 );
	}	
	void SetWorldFoucsFilterColorAlpha_Color(lwAction kAction, int const iColor)
	{// 10822 무슨 색으로
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10822, iColor );
	}
	int GetWorldFoucsFilterColorAlpha_Color(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 10822 );
	}
	void SetWorldFoucsFilterColorAlpha_AlphaStart(lwAction kAction, float const fAlphaStart)
	{// 10823 alpha 몇에서
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat( 10823, fAlphaStart );
	}
	float GetWorldFoucsFilterColorAlpha_AlphaStart(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat( 10823 );
	}
	void SetWorldFoucsFilterColorAlpha_AlphaEnd(lwAction kAction, float const fAlphaEnd)
	{// 10824 alpha 몇으로
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat( 10824, fAlphaEnd );
	}
	float GetWorldFoucsFilterColorAlpha_AlphaEnd(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat( 10824 );
	}
	void SetWorldFoucsFilterColorAlpha_TotalTime(lwAction kAction, float const fTotalTime)
	{// 10825 몇초에 걸쳐서
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat( 10825, fTotalTime );
	}
	float GetWorldFoucsFilterColorAlpha_TotalTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat( 10825 );
	}
	void SetWorldFoucsFilterColorAlpha_ActionTime(lwAction kAction, int const iActionTime)
	{// 10826 어느 시점에
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10826, iActionTime );
	}
	int GetWorldFoucsFilterColorAlpha_ActionTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 10826 );
	}
	
	void SetWorldFoucsFilterColorAlpha_Keep(lwAction kAction, int const iKeep)
	{// 10827 색바꾼것(필터)를 유지 할것인가, 자동적으로 사라질것인가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 10827, iKeep );
	}
	int GetWorldFoucsFilterColorAlpha_Keep(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return kAction.GetParamInt( 10827 );
	}

	void ProcessWorldFocusFilterColorAlpha(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( !g_pkWorld ) { return; }

		int iActionTime = GetWorldFoucsFilterColorAlpha_ActionTime(kAction);
		if( eActionTime == static_cast<EActionTime>(iActionTime) )
		{
			int const iType = GetWorldFoucsFilterColorAlpha_Target(kAction);
			switch( iType )
			{
			case EWFFCAT_ME:
				{
					if( !kActor.IsUnderMyControl() )
					{
						return;
					}
				}break;
			case EWFFCAT_ALL:
				{
				}break;
			default:
				{
					return;
				}break;
			}
			float const fAlphaStart = GetWorldFoucsFilterColorAlpha_AlphaStart(kAction);
			float const fAlphaEnd = GetWorldFoucsFilterColorAlpha_AlphaEnd(kAction);
			DWORD const dwColor = static_cast<DWORD>( GetWorldFoucsFilterColorAlpha_Color(kAction) );
			float const fTotal = GetWorldFoucsFilterColorAlpha_TotalTime(kAction);
			int const iKeep = GetWorldFoucsFilterColorAlpha_Keep(kAction);
			g_pkWorld->SetShowWorldFocusFilter(dwColor, fAlphaStart, fAlphaEnd, fTotal, 0 == iKeep);
		}
	}
		
	int IncMsgChangeCamCnt(lwAction kAction)
	{// 10900  - 애니메이션 중 카메라 변경 메세지인 'c_cam' Event Msg가 몇번 들어왔는가?
		if( kAction.IsNil() )	{ return 0; }
		int const iCurCnt = GetMsgChangeCamCnt(kAction) + 1;
		kAction.SetParamInt(10900, iCurCnt);
		return GetMsgChangeCamCnt(kAction);
	}
	int GetMsgChangeCamCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10900);
	}
	
	void SetCurCamMode(lwAction kAction, int const iCamMode)
	{// 10901 현재 카메라 모드를 저장한다
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10901, iCamMode);
	}
	int GetCurCamMode(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10901);
	}
	
	void SetKeepCamMode(lwAction kAction, bool const bKeepCamMode)
	{// 10902 현재 카메라 모드를 액션이 종료해도 유지 하는가?
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(10902, bKeepCamMode ? 1 : 0);
	}
	bool IsKeepCamMode(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return 0 < kAction.GetParamInt(10902);
	}
	void InitKeepCamMode(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return; }
		BM::vstring vKeepParam(STR_C_CAM_MSG_KEEP);
		int const iKeep = kAction.GetScriptParamAsInt( vKeepParam.operator const std::string().c_str() );
		SetKeepCamMode( kAction, 0 < iKeep );
	}

	void ProcessChangeCam(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{// 카메라 모드를 바꾸기 위해
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( !g_pkWorld )
		{
			return;
		}
		lwWorld kWorld(g_pkWorld);

		switch(eActionTime)
		{
		case EAT_MSG_CHANGE_CAMERA:
			{
				int const iCnt = IncMsgChangeCamCnt(kAction);
				
				BM::vstring kTypeParam(STR_C_CAM_MSG_TYPE); kTypeParam+=iCnt;
				int iType = kAction.GetScriptParamAsInt( kTypeParam.operator const std::string().c_str() );
				bool bDoSetCamera = false;
				switch(iType)
				{// 카메라 모드중
				case ECM_STOP_ONLY_MINE:	// 내 actor에게만 적용해야 하는것들을 체크하고
				case ECM_USE_CAMERA_NAME_IN_ACTOR_ONLY_MINE: { if( !kActor.IsUnderMyControl() ) { break; } }//break 의도적 삭제
				
											// 카메라 세팅이 가능한 범주라면
				case ECM_STOP:					   // 카메라 멈추기
				case ECM_USE_CAMERA_NAME_IN_ACTOR: // Actor에 들어있는 카메라를 사용하기
					{
						bDoSetCamera = true;
					}break;
				}
				if(bDoSetCamera)
				{// 사용할 정보들을
					SetCurCamMode( kAction, iType );
					switch(iType)
					{// 카메라 종류 별로 세팅한다
					case ECM_STOP_ONLY_MINE:
					case ECM_STOP:
						{
							iType = static_cast<int>(PgCameraMan::CMODE_STOP);
							kWorld.SetCameraMode(iType, kActor);
						}break;
					case ECM_USE_CAMERA_NAME_IN_ACTOR_ONLY_MINE:
					case ECM_USE_CAMERA_NAME_IN_ACTOR:
						{
							BM::vstring kCamNameParam(STR_C_CAM_MSG_CAMERA_NAME);
							kCamNameParam += iCnt;
							std::string strCamera = kAction.GetScriptParam( kCamNameParam.operator const std::string().c_str() );
							kWorld.SetCameraModeByActorInObjectName( kActor, strCamera.c_str() );
						}break;
					}
				}
			}break;
		case EAT_ON_CLEANUP:
			{
				switch( GetCurCamMode(kAction) )
				{// 카메라 모드가 
				case ECM_STOP_ONLY_MINE:
				case ECM_USE_CAMERA_NAME_IN_ACTOR_ONLY_MINE:
					{// 내 소유의 액터가 아니라면 나의 카메라는 바뀌지 않았을것이므로
						if( !kActor.IsUnderMyControl() )
						{// 처리를 중단하고
							break;
						}
					}//의도적 break 제거
				case ECM_STOP:
				case ECM_USE_CAMERA_NAME_IN_ACTOR:
					{// 원래대로 돌려야 하는 모드 이면서
						if(false == IsKeepCamMode(kAction)
							&& PgCameraMan::CMODE_FOLLOW <  g_pkWorld->GetCameraMan()->GetCameraModeE()
							)
						{ // 카메라 모드를 유지하지 않는다면
							kWorld.SetCameraMode( PgCameraMan::CMODE_FOLLOW, /*g_kPilotMan.GetPlayerActor()*/ kActor );	// 기본 모드로 바꾼다
						}
					}break;
				default:
					{
					}break;
				}
			}break;
		}
	}
	
	//void InitUIControlInfo(lwActor kActor, lwAction kAction)
	//{// UI 조정 EUIControlType
	//	if( kActor.IsNil() )	{ return; }
	//	if( kAction.IsNil() )	{ return; }
	//	int const iType = kAction.GetScriptParamAsInt("UI_CONTROL_TYPE");
	//	if(!iType)
	//	{
	//		return;
	//	}
	//	int const iActionTime =kAction.GetScriptParamAsInt("UI_CONTROL_ACTIONTIME");
	//	if(!iActionTime)
	//	{
	//		return;
	//	}
	//	SetUIControlType(kAction, iType);
	//	SetUIControlActionTime(kAction, iActionTime);
	//}
	//
	//void SetUIControlType(lwAction kAction, int const iUIControlType)
	//{// 10950 UI 조정 타입
	//	if( kAction.IsNil() )	{ return; }
	//	kAction.SetParamInt(10950, iUIControlType);
	//}
	//int GetUIControlType(lwAction kAction)
	//{
	//	if( kAction.IsNil() )	{ return 0; }
	//	return kAction.GetParamInt(10950);
	//}
	//void SetUIControlActionTime(lwAction kAction, int const iActionTime)
	//{// 10951 UI 조정 시작할때
	//	if( kAction.IsNil() )	{ return; }
	//	kAction.SetParamInt(10951, iActionTime);
	//}
	//int GetUIControlActionTime(lwAction kAction)
	//{
	//	if( kAction.IsNil() )	{ return 0; }
	//	return kAction.GetParamInt(10951);
	//}

	int IncUIControlMsgCnt(lwAction kAction)
	{// 10960 ui 이벤트 키가 몇번 들어왔는가?
		if(kAction.IsNil())
		{
			return 0;
		}
		int const iCurCnt = GetUIControlMsgCnt(kAction) + 1;
		kAction.SetParamInt(10960, iCurCnt);
		return GetUIControlMsgCnt(kAction);
	}
	int GetUIControlMsgCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(10960);
	}

	void ProcessUIMsg(lwActor kActor, lwAction kAction, EActionTime const eActionTime)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( !kActor.IsUnderMyControl() )	{ return; }	// UI관련된것은 오직 내 액터들 일때만 동작하게끔 하고
		
		switch(eActionTime)
		{
		case EAT_MSG_UI:
			{
				int const iCnt = IncUIControlMsgCnt(kAction);
				BM::vstring vTypeParam("UI_CONTROL_TYPE_WHEN_UI_MSG"); vTypeParam+=iCnt;
				int const iType = kAction.GetScriptParamAsInt( vTypeParam.operator const std::string().c_str() );
				switch(iType)
				{
				case EUICT_CLOSE_ALL_UI:
					{
						lwCloseAllUI();
					}break;
				case EUICT_CALL_BASIC_UI:
					{
						lua_tinker::call< void >("UI_DefaultActivate" );
					}break;
				case EUICT_CALL_UI:
					{
						BM::vstring vIDParam("UI_CONTROL_CALL_ID_WHEN_UI_MSG"); vIDParam+=iCnt;
						BM::vstring vID( kAction.GetScriptParam( vIDParam.operator const std::string().c_str() ) );
						XUIMgr.Activate(vID);
					}break;
				}
			}break;
		}
	}
	void InitHideNameTitle(lwActor kActor, lwAction kAction)
	{// 언제 감출지 결정할 필요가 없다고 생각해서 Init에서 바로 처리함
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( 0 < kAction.GetScriptParamAsInt("HIDE_NAME_TITLE") )
		{
			kActor.SetHideNameTitle(true);
		}
	}
	void ReleaseHideNameTitle(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( 0 < kAction.GetScriptParamAsInt("HIDE_NAME_TITLE") 
			&& kActor.IsHideNameTitle() 
			)
		{
			kActor.SetHideNameTitle(false);
		}
	}
	void InitHideSubActor(lwActor kActor, lwAction kAction)
	{// 언제 감출지 결정할 필요가 없다고 생각해서 Init에서 바로 처리함
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( 0 < kAction.GetScriptParamAsInt("HIDE_SUB_ACTOR") )
		{
			lwActor kSubActor = kActor.GetSubPlayer();
			if( kSubActor.IsNil() )	{ return; }
			kSubActor.SetHide(true);
		}
	}
	void ReleaseHideSubActor(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( 0 < kAction.GetScriptParamAsInt("HIDE_SUB_ACTOR") )
		{
			lwActor kSubActor = kActor.GetSubPlayer();
			if( kSubActor.IsNil() )	{ return; }
			kSubActor.SetHide(false);
		}
	}

	bool InitSpecifyProcessAnislot(lwAction kAction)
	{// 특정 애니슬롯에서 해야할 특별한 처리가 있는가
		if( kAction.IsNil() )	{ return false; }

		{// 몇번 anislot에  땅에 닿기전엔 애니를 넘어갈수 없는 옵션이 있는가
			
			std::string const kStrParam = kAction.GetScriptParam("MEET_FLOOR_THEN_NEXT_ANI_SLOT");
			if( !kStrParam.empty() )
			{//여기서 파싱하는 부분이 필요함
				int const iSlot = PgStringUtil::SafeAtoi( kStrParam.c_str() );
				CONT_INT kCont;
				kCont.push_back(iSlot);

				CONT_INT::const_iterator kItor = kCont.begin();
				while( kCont.end() != kItor )
				{
					int const iAniSlotNo =  (*kItor);
					int const iRelativeIdx = (iAniSlotNo * 100) + SUB_IDX_MEET_FOOLR_THEN_NEXT_ANI_SLOT;
					int const iSaveIdx = IDX_SPECIFY_PROCESS_ANI_SLOT_PARAM + iRelativeIdx;
					kAction.SetParamInt(iSaveIdx, 1);
					++kItor;
				}
			}
		}
		return true;
	}
	// 20000 특정 애니슬롯에서 해야할 특별한 처리가 있는가
	void SetCurSpecifyProcessAniSlot(lwAction kAction, int const iAniSlot)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(20000, iAniSlot);
	}
	int GetCurSpecifyProcessAniSlot(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return -1; }
		if( NULL == kAction.GetParam(20000) ) { return -1; }
		return kAction.GetParamInt(20000);
	}


	// 20100~21000 - AniSlot일때 특정 동작을 하기 위해서 param을 저장함, 100개씩 저장하며, 10개의 슬롯까지

	
	//21100 ~ 22000 애니 슬롯이 몇번까지 반복되어야 하는가
	void InitRepeatAniSlotBySkillLevel(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return; }
		// ex) REPEAT_ANISLOT1_ON_SKILL_LV1	: 스킬 1레벨일때, 1번 애니슬롯 반복 횟수
		int const iLv = kAction.GetSkillLevel();
		for(int iAniSlot=0; iAniSlot < kAction.GetSlotCount(); ++iAniSlot)
		{
			BM::vstring vHeadStr("REPEAT_ANISLOT");
			vHeadStr += iAniSlot;						// 애니 슬롯
			BM::vstring vTailStr("_ON_SKILL_LV");
			vTailStr += +iLv;							// 스킬레벨
			BM::vstring vResult( vHeadStr+vTailStr );
			int const iRepeatCnt = kAction.GetScriptParamAsInt( vResult.operator const std::string().c_str() );
			if( 1 < iRepeatCnt )
			{
				SetMaxRepeatAniSlot(kAction, iAniSlot, iRepeatCnt-1); // 애니 1회를 완료한후 부터 체크하기 때문.(-1 없다면 2회 반복 이라고 입력하면 3회 반복하게됨)
			}
		}
	}
	void SetMaxRepeatAniSlot(lwAction kAction, int const iAniSlot, int const iRepeatCnt)
	{// iAniSlot를 몇번 반복해야 하는가 ( 21100+ (0*10)	 : 0번 애니 슬롯이 몇번 반복 되어야 하는가 )
		if( kAction.IsNil() )	{ return; }
		// 21100+ (0*10)	 : 0번 애니 슬롯이 몇번 반복 되어야 하는가
		// 21100+ (0*10) + 1 : 0번 애니 슬롯이 몇번 반복 했는가
		int const iIndex = 21100 + (iAniSlot*10);
		kAction.SetParamInt(iIndex, iRepeatCnt);
	}
	int GetMaxRepeatAniSlot(lwAction kAction, int const iAniSlot)
	{
		if( kAction.IsNil() )	{ return 0; }
		int const iIndex = 21100 + (iAniSlot*10);
		int const iRepeatCnt = kAction.GetParamInt(iIndex);
		return iRepeatCnt;
	}
	void SetCntRepeatAniSlot(lwAction kAction, int const iAniSlot, int const iRepeatCnt)
	{// iAniSlot를 몇번 반복했는가 ( 21100+ (0*10) + 1 : 0번 애니 슬롯이 몇번 반복 했는가 )
		if( kAction.IsNil() )	{ return; }
		int const iIndex = 21100 + (iAniSlot*10) + 1;
		kAction.SetParamInt(iIndex, iRepeatCnt);
	}
	int GetCntRepeatAniSlot(lwAction kAction, int const iAniSlot)
	{
		if( kAction.IsNil() )	{ return 0; }
		int const iIndex = 21100 + (iAniSlot*10) + 1;
		int const iRepeatedCnt = kAction.GetParamInt(iIndex);
		return iRepeatedCnt;
	}
	int IncCntRepeatAniSlot(lwAction kAction, int const iAniSlot)
	{// iAniSlot이 반복된 횟수를 기록
		if( kAction.IsNil() )	{ return 0; }
		int const iIncreasedRepeatCnt = GetCntRepeatAniSlot(kAction, iAniSlot) + 1;
		SetCntRepeatAniSlot(kAction, iAniSlot, iIncreasedRepeatCnt);
		return iIncreasedRepeatCnt;
	}
	bool IsDoneRepeatAniSlot(lwAction kAction, int const iAniSlot)
	{// iAniSlot이 모두 반복되었나
		if( kAction.IsNil() )	{ return true; }
		int const iMaxRepeat = GetMaxRepeatAniSlot(kAction, iAniSlot);
		int const iCurRepeat = GetCntRepeatAniSlot(kAction, iAniSlot);
		return iCurRepeat == iMaxRepeat;
	}
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
	void InitDecorationActorInfo(lwActor kCallerActor, lwAction kCallerAction)
	{
		if( kCallerActor.IsNil() )	{ return; }
		if( kCallerAction.IsNil() )	{ return; }

		{// 연출 액터에 쓸 클래스 번호를 읽어오고
			int const iClassNo = kCallerAction.GetScriptParamAsInt("DECO_ACTOR_CLASS_NO");
			if( 0 >= iClassNo )
			{
				return;
			}
			SetDecorationActor_ClassNo( kCallerAction, iClassNo );
		}
		{// 어느때에 붙일 것인지
			int const iActionTime = kCallerAction.GetScriptParamAsInt("DECO_ACTOR_ACTIONTIME");
			if(0 >= iActionTime )
			{
				return;
			}
			SetAddDecorationActor_ActionTime(kCallerAction, iActionTime);
		}
		{// 연출 액터가 등장 할때 어떤 액션을 할 것 인지
			std::string const kActionName = kCallerAction.GetScriptParam("DECO_ACTOR_ACTION_NAME");
			if( kActionName.empty() )
			{
				return;
			}
			SetDecorationActor_StartActionName( kCallerAction, kActionName.c_str() );
		}
		{// 시작 위치를 Caller 위치로 부터 얼마만큼 떨어져서 붙일것인지 알아오고
			float const fHorizDeg = kCallerAction.GetScriptParamAsFloat("DECO_ACTOR_POS_DIST_DIR_ROTATE_HORIZ_DEG");
			float const fVertDeg = kCallerAction.GetScriptParamAsFloat("DECO_ACTOR_POS_DIST_DIR_ROTATE_VERT_DEG");
			float const fDist = kCallerAction.GetScriptParamAsFloat("DECO_ACTOR_DIST");
			//float const fAlpha = kCallerAction.GetScriptParamAsFloat("DECO_ACTOR_ALPHA");

			lwPoint3 kDistVecDir = kCallerActor.GetLookingDir();
			bool bDoMultiply = false;
			if(0.0f != fHorizDeg)
			{
				float const fHorizRad = NxMath::degToRad(fHorizDeg);
				kDistVecDir.Rotate( lwPoint3(NiPoint3::UNIT_Z), fHorizRad );
				bDoMultiply = true;
			}
			if(0.0f != fVertDeg)
			{
				float const fVertRad = NxMath::degToRad(fVertDeg);
				lwPoint3 kRightDir = kCallerActor.GetLookingDir();
				kRightDir.Cross( static_cast<lwPoint3>(UP_VEC) );
				kDistVecDir.Rotate( kRightDir, fVertRad );
				bDoMultiply = true;
			}
			if(bDoMultiply)
			{
				kDistVecDir.Multiply(fDist);
				SetDecorationActor_DistVec(kCallerAction, kDistVecDir);
			}
			//SetDecorationActor_StartAlpha(kCallerAction, fAlpha);
		}
		{
			int const iType = kCallerAction.GetScriptParamAsInt("DECO_ACTOR_TYPE");
			SetDecorationActor_Type(kCallerAction, iType);
		}
	}
	
	void SetDecorationActor_ClassNo(lwAction kAction, int const iDecoActorClassNo)
	{// 30000 DecorationClassNo 
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(30000, iDecoActorClassNo);
	}
	int GetDecorationActor_ClassNo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(30000);
	}
	
	void SetDecorationActor_DistVec(lwAction kAction, lwPoint3 const kDistVec)
	{// 30001 소환자 위치와 이 벡터가 합해져 연출용 액터가 나타날 지점을 정한다
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(30001, kDistVec);
	}
	lwPoint3 GetDecorationActor_DistVec(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return lwPoint3(0.0f, 0.0f, 0.0f); }
		return kAction.GetParamAsPoint(30001);
	}

	void SetDecorationActor_StartActionName(lwAction kAction, char const* pcActionName)
	{// 30002 이 연출용 액터가 시작할 액션 DecorationActor Action
		if( kAction.IsNil()
			|| NULL == pcActionName
			)
		{
			return;
		}
		kAction.SetParam(30002, pcActionName);
	}
	char const* GetDecorationActor_StartActionName(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return ""; }
		return kAction.GetParam(30002);
	}

	void SetAddDecorationActor_ActionTime(lwAction kAction, int const iActionTime)
	{// 30003 어느 액션 타임에 연출용 액터를 추가 할 것인지 설정한다
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(30003, iActionTime);
	}
	int GetAddDecorationActor_ActionTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return EAT_NONE; }
		return kAction.GetParamInt(30003);
	}
	//void SetDecorationActor_StartAlpha(lwAction kAction, float const fAlpha)
	//{// 30004 시작시에 alpha값을 설정(안보이게 라든지 약간 투명하게 라든지)
	//	if( kAction.IsNil() )	{ return; }
	//	kAction.SetParamFloat(30004, fAlpha);
	//}
	//float GetDecorationActor_StartAlpha(lwAction kAction)
	//{
	//	if( kAction.IsNil() )	{ return 1.0f; }
	//	return kAction.GetParamFloat(30004);
	//}
	
	void SetDecorationActor_Type(lwAction kAction, int const iType)
	{// 30005 타입 설정(나한테만 보이게 등)
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(30005, iType);
	}
	int GetDecorationActor_Type(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(30005);
	}

	bool ProcessDecorationActor(lwActor kCallerActor, lwAction kCallerAction, EActionTime const eActionTime)
	{
		if( kCallerActor.IsNil() )	{ return false; }
		if( kCallerAction.IsNil() )	{ return false; }
		int const iActionTime = GetAddDecorationActor_ActionTime(kCallerAction);

		if( eActionTime == static_cast<EActionTime>(iActionTime) )
		{
			switch( GetDecorationActor_Type( kCallerAction ) )
			{
			case EDAT_ONLY_MINE:
				{
					if( !kCallerActor.IsUnderMyControl() )
					{
						return false;
					}
				}break;
			}

			int const iDecoClassNo = GetDecorationActor_ClassNo( kCallerAction );
			if(0 >= iDecoClassNo)
			{
				return false;
			}
			std::string const kDecoActorAction = GetDecorationActor_StartActionName( kCallerAction );
			if( kDecoActorAction.empty() )
			{
				return false;
			}

			BM::GUID kTempGuid;
			kTempGuid.Generate();
			lwPilot kDecoPilot = g_kPilotMan.NewPilot( kTempGuid, iDecoClassNo );
			if( false == kDecoPilot.IsNil() )
			{
				kDecoPilot.SetUnit(kTempGuid,UT_ENTITY,1,1,0);
				lwActor kDecoActor = kDecoPilot.GetActor();
				if( false == kDecoActor.IsNil()
					&& g_pkWorld
					)
				{
					if( IsUseDrawActorFilter(kCallerAction)
						&& kCallerActor.IsMyActor()
						)
					{
						g_pkWorld->AddDrawActorFilter( kDecoActor.GetPilotGuid()() );
					}
					lwPoint3 kPos = kCallerActor.GetTranslate();
					kDecoActor.FreeMove(true);
					
					//float const fStartAlpha = GetDecorationActor_StartAlpha(kCallerAction);
					//kDecoActor.SetAlpha(fStartAlpha);
					
					kPos.SetZ(kPos.GetZ()+5);
					if( g_pkWorld->AddObject(kTempGuid, kDecoActor(), kPos(), OGT_ENTITY) )
					{
						CUnit* pkUnit = kDecoPilot.GetUnit()();
						if(pkUnit)
						{
							pkUnit->Caller( kCallerActor.GetPilotGuid()() );
						}
						{
							//lwPoint3 LookAt = kCallerActor.GetLookingDir();
							//kDecoActor.LookAt(kCallerActor.GetTranslate()._Add(kLookAt), true);	//캐릭터랑 같은 방향으로
							InitDirection(kDecoActor, kCallerAction, "DECO_ACTOR_WHEN_START_DIRECTION");
						}
						TryMustChangeActorAction( kDecoActor, kDecoActorAction.c_str() );
						kDecoActor.ReserveTransitAction( kDecoActorAction.c_str(), DIR_NONE );
						kDecoActor.ClearActionState();
						if ( kCallerActor.IsMyActor() )
						{
							kDecoActor.SetUnderMyControl(true);
						}
						g_kPilotMan.InsertPilot( kTempGuid, kDecoPilot() );
						return true;
					}
				}
			}
		}
		return false;
	}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 연계콤보
	// 40000
	void SetUseCombo(lwAction kAction, bool const bUseCombo)
	{	// 액션 사용 중간에 타겟을 다시 잡아 때리는가? 에 대한 설정
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40000, bUseCombo ? 1 : 0 );
	}
	bool IsUseCombo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40000) )
		{
			return true;
		}
		return false;
	}
	// 40001
	 void SetEnableComboKeyEnter(lwAction kAction, bool const bEnterComboKey)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40001, bEnterComboKey ? 1 : 0 );
	}
	bool IsEnableComboKeyEnter(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40001) )
		{
			return true;
		}
		return false;
	}
	//40002
	void SetStartNextCombo(lwAction kAction, bool const bEnterComboKey)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40002, bEnterComboKey ? 1 : 0 );
	}
	bool IsStartNextCombo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40002) )
		{
			return true;
		}
		return false;
	}
	//40003
	void SetUseChargeCombo(lwAction kAction, bool const bCharge)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40003, bCharge ? 1 : 0 );
	}
	bool IsUseChargeCombo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40003) )
		{
			return true;
		}
		return false;
	}
	//40004
	void SetComboDelayCancle(lwAction kAction, bool const bCharge)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40004, bCharge ? 1 : 0 );
	}
	bool IsComboDelayCancle(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40004) )
		{
			return true;
		}
		return false;
	}
	//40005
	void ResetZoomCameraCount(lwAction kAction)
	{
		kAction.SetParamInt( 40005, 0);
	}
	void IncreaseZoomCameraCount(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return; }
		int iZoomCount = GetZoomCameraCount(kAction);
		kAction.SetParamInt( 40005, ++iZoomCount);
	}
	int GetZoomCameraCount(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return kAction.GetParamInt(40005);
	}
	
	// 40006 스킬 중계 설정
	void SetConnectCombo(lwAction kAction, int const iActionNo)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40006, iActionNo );
	}
	int GetConnectCombo(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return kAction.GetParamInt(40006);
	}
	
	// 40007 단축키를 이용한 기본 스킬 사용
	void SetSkillUse(lwAction kAction, int const iActionNo)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40007, iActionNo );
	}
	int GetSkillUse(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return kAction.GetParamInt(40007);
	}
	//40008 스킬 사용 중 캐릭터 이동 가능(방향전환X)
	void SetMoveControl(lwAction kAction, bool const bMove)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40008, bMove ? 1 : 0 );
	}
	bool IsMoveControl(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(40008) )
		{
			return true;
		}
		return false;
	}
	//40009 보조 캐릭터 스킬 예약
	void SetNextSubPlayerAction(lwAction kAction, int const iNextActionNo)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 40009, iNextActionNo );
	}
	int GetNextSubPlayerAction(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return kAction.GetParamInt(40009);
	}
//연계콤보 끝
	int IncMsgFireCnt(lwAction kAction)
	{// 100299 - 애니메이션 중 hit 메세지가 몇번 들어왔는가? int const IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG	 = 100200;
		if(kAction.IsNil())
		{
			return 0;
		}
		int const iCurCnt = GetMsgFireCnt(kAction) + 1;
		kAction.SetParamInt(100299, iCurCnt);
		return GetMsgFireCnt(kAction);
	}
	int GetMsgFireCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(100299);
	}

	int IncMsgHitCnt(lwAction kAction)
	{// 100399 - 애니메이션 중 hit 메세지가 몇번 들어왔는가? int const IDX_PARTICLE_SLOT_AT_ON_HIT_MSG	 = 100300;
		if(kAction.IsNil())
		{
			return 0;
		}
		int const iCurCnt = GetMsgHitCnt(kAction) + 1;
		kAction.SetParamInt(100399, iCurCnt);
		return GetMsgHitCnt(kAction);
	}
	int GetMsgHitCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(100399);
	}

	int IncMsgEffectCnt(lwAction kAction)
	{// 100499 - 애니메이션 중 effpos 메세지가 몇번 들어왔는가? IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG = 100400;
		if(kAction.IsNil())
		{
			return 0;
		}
		int const iCurCnt = GetMsgEffectCnt(kAction) + 1;
		kAction.SetParamInt(100499, iCurCnt);
		return GetMsgEffectCnt(kAction);
	}
	int GetMsgEffectCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(100499);
	}

	
	
	void SetReActionByTrapSkill(lwAction kAction, bool bSetOn)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 210100, bSetOn ? 1 : 0 );
	}

	bool IsReActionByTrapSkill(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		if( 0 < kAction.GetParamInt(210100) )
		{
			return true;
		}
		return false;
	}
	
	char const* GetSummonedHitSound(lwActor kActor)
	{
		if( kActor.IsNil() )	{ return NULL; }
		lwAction kAction = kActor.GetAction();
		if( kAction.IsNil() )	{ return NULL; }

		int const iSkillNo = kAction.GetParentSkillNo();
		//ODS("소환수 스킬 : "..action:GetSkillNo().."\n", false, 3851)
		//ODS("사용할 부모 스킬 : "..action:GetParentSkillNo().."\n", false, 3851)
		if (1 == iSkillNo)
		{
			return "sum_pet-hit-01";
		}
		else
		{
			return "sum_pet-hit-02";
		}
		return NULL;
	}

	void Actor_Mob_Base_DoDefaultMeleeAttack(lwActor kActor,lwAction kAction)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if( kAction.GetActionParam() != ESS_FIRE ) 
		{
			return;
		}
		
		PgActionTargetList* pkTargetList = kAction.GetTargetList()();
		if(NULL == pkTargetList
			|| 0 == pkTargetList->size()
			)
		{
			return;
		}
		
		bool bWeaponSoundPlayed = false;
		ActionTargetList::iterator kItor = pkTargetList->GetList().begin();

		while( pkTargetList->end() != kItor )
		{
			lwActionResult kActionResult( &(*kItor).GetActionResult() );

			if(kActionResult.IsNil() == false
				&& kActionResult.IsMissed() == false
				)
			{
				lwGUID kTargetGUID = (*kItor).GetTargetPilotGUID();
				lwPilot kTargetPilot = g_kPilotMan.FindPilot( kTargetGUID() );

				if(kTargetPilot.IsNil() == false)
				{
					lwActor kTargetActor = kTargetPilot.GetActor();
					if (bWeaponSoundPlayed == false )
					{
						bWeaponSoundPlayed = true;
						if( true == kActor.IsUnitType(UT_SUMMONED) )
						{
							char const* pcSoundID = GetSummonedHitSound(kActor);
							if( NULL != pcSoundID )
							{
								kTargetActor.AttachSound( 0, pcSoundID, 0.0f );
							}
						}
						else
						{
							lwCommonSkillUtilFunc::PlayWeaponSound( kActor, kAction, kActionResult.GetCritical(), kTargetActor, &(*kItor) );
						}
					}
					//if( true == GetUseDmgEffect(kAction) )
					{
						lwCommonSkillUtilFunc::DefaultDoDamage(kActor, kTargetActor, kActionResult, &(*kItor) );
					}
				}
			}
			++kItor;
		}
		
		pkTargetList->ApplyActionEffects(false, false, false);
		kAction.ClearTargetList();
	}

	void Act_Mon_Melee_DoDamage(lwActor kActor,lwAction kAction)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		
		PgActionTargetList* pkTargetList = kAction.GetTargetList()();
		if(NULL == pkTargetList
			|| 0 == pkTargetList->size()
			)
		{
			return;
		}
		
		bool bWeaponSoundPlayed = false;
		ActionTargetList::iterator kItor = pkTargetList->GetList().begin();

		while( pkTargetList->end() != kItor )
		{
			lwActionResult kActionResult( &(*kItor).GetActionResult() );

			if(kActionResult.IsNil() == false
				&& kActionResult.IsMissed() == false
				)
			{
				lwPilot kTargetPilot( g_kPilotMan.FindPilot((*kItor).GetTargetPilotGUID()) );
				
				if( kTargetPilot.IsNil() == false )
				{
					lwActor kTargetActor = kTargetPilot.GetActor();
					if( false == kTargetActor.IsNil() )
					{
						//
						char const* targetEffectNode = kActor.GetAnimationInfo("TARGET_EFFECT_NODE", 0);
						char const* targetEffect = kActor.GetAnimationInfo("TARGET_EFFECT", 0);
						if( targetEffectNode != NULL
							&& targetEffect != NULL
							)
						{
							kTargetActor.AttachParticle(124, targetEffectNode, targetEffect);
						}

						//
						if (bWeaponSoundPlayed == false)
						{
							bWeaponSoundPlayed = true;
							if( true == kActor.IsUnitType(UT_SUMMONED) )
							{
								char const* pcSoundID = GetSummonedHitSound(kActor);
								if( NULL != pcSoundID )
								{
									kTargetActor.AttachSound( 0, pcSoundID, 0.0f );
								}
							}
							else
							{
								lwCommonSkillUtilFunc::PlayWeaponSound( kActor, kAction, kActionResult.GetCritical(), kTargetActor, &(*kItor) );
							}
						}
					}
					{
						lwCommonSkillUtilFunc::DefaultDoDamage(kActor, kTargetActor, kActionResult, &(*kItor) );
					}
				}
			}
			++kItor;
		}
		
		if( pkTargetList
			&& 1 != kAction.GetParamInt(20110615)
			) 
		{
			pkTargetList->ApplyActionEffects();
		}

		if( 0 != PgStringUtil::SafeStrcmp("true", kActor.GetParam("lightning")) )
		{
			kAction.ClearTargetList();
		}
	}

	bool ProjectileHitOneTime(lwProjectile kProjectile)
	{
		if( kProjectile.IsNil() )	{ return false; }
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(kProjectile.GetParentActionNo());
		if(NULL == pkSkillDef)
		{
			return false;
		}

		PgActionTargetList* pkTargetList = kProjectile.GetActionTargetList()();
		if(NULL == pkTargetList
			|| 0 == pkTargetList->size()
			|| pkTargetList->IsActionEffectApplied()
			)
		{
			return false;
		}
		
		lwPilot kParentPilot = g_kPilotMan.FindPilot( kProjectile.GetParentPilotGuid()() );
		if( kParentPilot.IsNil() ) 
		{
			pkTargetList->ApplyActionEffects();
			return false;
		}
		lwActor kActor = kParentPilot.GetActor();

		bool bWeaponSoundPlayed = false;
		ActionTargetList::iterator kItor = pkTargetList->GetList().begin();
		while( pkTargetList->end() != kItor )
		{
			lwActionResult kActionResult( &(*kItor).GetActionResult() );
			
			if(kActionResult.IsNil() == false
				&& kActionResult.IsMissed() == false
				)
			{
				lwGUID kTargetGUID = (*kItor).GetTargetPilotGUID();
				lwPilot kTargetPilot = g_kPilotMan.FindPilot( kTargetGUID() );

				if(kTargetPilot.IsNil() == false)
				{
					lwActor kTargetActor = kTargetPilot.GetActor();
					if (bWeaponSoundPlayed == false 
						&& pkSkillDef
						)
					{
						bWeaponSoundPlayed = true;
						//lwCommonSkillUtilFunc::PlayWeaponSound( kActor, kAction, kActionResult.GetCritical(), kTargetActor, &(*kItor) );
						kTargetActor.PlayWeaponSound(PgWeaponSoundManager::WST_HIT, kActor, MB(pkSkillDef->GetActionName()) , 0, &(*kItor) );
					}
					//if( true == GetUseDmgEffect(kAction) )
					{
						lwCommonSkillUtilFunc::DefaultDoDamage( kActor, kTargetActor, kActionResult, &(*kItor) );
					}
				}
			}
			++kItor;
		}
		pkTargetList->ApplyActionEffects();
		int iMaxCount = kProjectile.GetPenetrationCount();
		if( 0 != iMaxCount )
		{
			return	true;
		}
		return	false;
	}
}