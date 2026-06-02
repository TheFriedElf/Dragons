#include "stdafx.h"
#include "Variant/DefAbilType.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"
#include "BM/Stream.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgActionSlot.h"
#include "PgChatMgrClient.h"
#include "PgInterpolator.h"
#include "PgProjectileMan.h"
#include "PgActionPool.h"
#include "PgNetwork.h"
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
#include "lwCommonSkillUtilFunc.h"
#include "lwWorld.h"
#include "PgComboAdvisor.H"
#include "lwEffect.h"
#include "PgLocalManager.h"
#include "PgOption.h"

namespace lwCommonSkillUtilFunc
{
	bool OnCheckCanEnter(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kAction.IsNil() )	{ lwSetBreak(); return false; }
		//int const iWeaponType = kActor.GetEquippedWeaponType();	// 기본적으로 Action 전이 할때 Weapon 체크함

		InitTransitCheck(kAction);
		if( !CheckTransitAction(kActor, kAction, kActor.GetAction()) )
		{// 전이 가능한지 확인하고
			return false;
		}

		return true;
	}

	bool OnEnter(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kAction.IsNil() )	{ lwSetBreak(); return false; }
//		if( kActor.IsUnitType(UT_ENTITY) )
//		{// ENTITY가 유독 피직스 업데이트 하던중 크래시나는 문제가 있어 명확히 잡힐때까지 해당 부분을 update 타지 않게끔 설정하는 값을 기본적으로 줌
//			kActor.FreeMove(true);
//		}
		kActor.ResetAnimation();
		// 이 스킬을 사용시 바로 브로드 캐스팅하는지 확인하고
		InitUseAutoBroadCast(kAction);
		// 캐스팅 완료되었을대 브로드 캐스팅 하는것을 사용하는지 확인하고
		InitUseAutoBroadCastWhenCastingCompleted(kAction);
		// 애니메이션 속도를 캐스팅 시간과 맞추고
		AdjustAniSpeedOnCastingTime(kActor, kAction);
		// 정면 보는것을 사용하는가
		InitSeeFrontInfo(kActor, kAction);
		// 스킬 사용시 방향을 초기화 하고
		InitDirection( kActor, kAction, STR_WHEN_START_DIRECTION.c_str() );
		// 최대 타격 횟수를확인 하고
		InitMaxHitCnt(kAction);
		// 캐릭터가 뱅글 뱅글 회전하는 값이 있다면 초기화 하고
		InitSpinDegPerSec(kAction);
		// Trail을 사용하는지 초기화하고
		InitUseTrail(kAction);
		{//스킬 사용 중간에 다시 때릴수
			std::string const kUseTlmParam = kAction.GetScriptParam("USE_TLM");
			if( kUseTlmParam.empty()
				|| STR_TRUE == kUseTlmParam
				)
			{// 있다면
				SetUseTLM( kAction, true );
			}
			else
			{// 없다면
				SetUseTLM( kAction, false );
			}
		}

		{// 타이머로 공격하는게 있다면
			InitAttackTimer(kAction);
			if( kActor.IsUnderMyControl() )
			{
				IfAbleAttackTimerThenStart(kAction, EAT_ON_ENTER);
			}
		}

		{// 특정 애니슬롯에서 해야할 특별한 처리가 있는가
			SetCurSpecifyProcessAniSlot(kAction, 0);	// 특별동작을 하기위해 몇번째 슬롯의 애니를 체크해야 하는가를 확인하기 위한 값을 저장하고
			InitSpecifyProcessAnislot(kAction);
		}
		InitUseDmgEffect(kAction);				// 타격 이펙트 사용 유무를 확인하고
		InitRepeatAniSlotBySkillLevel(kAction);	// 애니메이션 슬롯이 몇번까지 반복되어야 하는가 확인하고
		
		if( STR_TRUE == kAction.GetScriptParam("USE_SAVE_PRE_TARGET_LIST") )
		{// 사전에 타겟 리스트를 작성해서 저장해두는가
			PgActionTargetList	kTempTargetList;
			lwActionTargetList kTargetList(&kTempTargetList);
			{
				int const iArea_Type = kAction.GetScriptParamAsInt("PRE_FIND_TARGET_TYPE");
				float const fRange = kAction.GetScriptParamAsFloat("PRE_FIND_TARGET_RANGE");
				float fWideAngle = kAction.GetScriptParamAsFloat("PRE_FIND_TARGET_AREA_PARAM2");
				float fEffectRadius = fWideAngle;
				
				NiPoint3 kStartPos = kActor.GetPos()(); //AT_SKILL_BACK_DISTANCE
				NiPoint3 kDir = kActor.GetLookingDir()();
				
				int const iMaxTarget = kAction.GetAbil(AT_MAX_TARGETNUM);
				lwFindTargetParam kFindTargetParam;
				switch(iArea_Type)
				{
				case ESArea_Sphere:
					{
						fEffectRadius = fRange;
					}break;
				case ESArea_Cube:
				case ESArea_Cone:
					{
						if(0 == fWideAngle)
						{
							fWideAngle = 40.0f;
						}
					}break;
				
				case ESArea_Front_Sphere:
					{
						kDir *= fRange;
						kStartPos += kDir;
						if(0 >= fWideAngle)
						{
							fEffectRadius = fWideAngle = fRange;
						}
					}break;
				}
				kFindTargetParam.SetParam_1( kStartPos, kDir );
				kFindTargetParam.SetParam_2( fRange , fWideAngle, fEffectRadius, iMaxTarget);
				//kFindTargetParam.SetParam_3();
				PgAction::TargetAcquireType const eFindType = PgAction::ServerTypeToTargetAcquireType(static_cast<ESkillArea>(iArea_Type) );
				kAction.FindTargets(eFindType, kFindTargetParam, kTargetList, kTargetList);
			}
			

			for(int i =0; i < kTargetList.size(); ++i)
			{
				lwActionTargetInfo kInfo =  kTargetList.GetTargetInfo(i);
				if( kInfo.IsNil() ) { break; }
				lwGUID kTargetGuid = kInfo.GetTargetGUID();
				if( kTargetGuid.IsNil() ) { break; }
				SetSavedTargetGuid(kAction, i, kTargetGuid);
			}
			SetMaxSavedTargetCnt( kAction, kTargetList.size() );
			SetCurUseSavedTargetIdx(kAction, 0);
			kAction.ClearTargetList();
		}
		InitCancelOption(kAction);				// 캔슬 할수 있는 액션에 대한 정보를 갱신하고
		InitSyncInputDir(kActor, kAction);		// 유저의 입력에 의한 방향을 사용하는 액션인가 확인하고

		if( kActor.IsUnderMyControl() )
		{// 내 제어하에 있는 Actor라면
		}
		else
		{// 타인의 Actor라면 Sync 할 정보들을 처리하고
			while( ProcessOverridePacket(kActor, kAction) ) {}
		}
		
		// 연출용 액터를 사용한다면 정보를 초기화 하고
		InitDecorationActorInfo(kActor, kAction);
		// 어떤 시점에 어떤 대상(자신이 소유한 UT_SUB_PLAYER등)에게 어떤 액션을 시킬 정보에 대해 초기화 하고
		InitOrderActionInfo(kAction);
		
		InitWorldFocusFilterColorAlpha(kActor, kAction);	// 화면을 어둡게 하거나 하는 효과를 쓸것인가
		
		InitDrawActorFilter(kAction);
		
		InitKeepCamMode(kAction);	// 카메라 변경이 되면 그것을 액션 종료하고도 유지할것인가?
		
		InitHideNameTitle(kActor, kAction);	// 캐릭터 이름, 업적, 길드이름들을 감추는가?
		InitHideSubActor(kActor, kAction); // SubActor를 감추는가? 

		ProcessEtc( kActor, kAction, EAT_ON_ENTER );
		//OnCastingCompleted(kActor, kAction);
		

		InitIsBanSubPlayerAction(kActor, kAction);	// SC 스킬을 쓰면 안되는 스킬인가
		if( kActor.IsMyActor())
		{
			int const iSubPlayerAction = GetNextSubPlayerAction(kAction);
			if( iSubPlayerAction )
			{
				lwActor kSubActor = kActor.GetSubPlayer();
				if( !kSubActor.IsNil() &&
					kSubActor.IsMySubPlayer() )
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSubPlayerAction);
					if (NULL != pkSkillDef)
					{
						TryMustChangeActorAction( kSubActor, MB(std::wstring(pkSkillDef->GetActionName())) );
					}
				}
			}
			//스킬 사용중 이동 가능하냐
			std::string const kUseMoveControl = kAction.GetScriptParam("USE_MOVE_CONTROL");
			if( !kUseMoveControl.empty()
				&& STR_TRUE == kUseMoveControl )
			{// 있다면
				SetMoveControl( kAction, true );
			}
			else
			{// 없다면
				SetMoveControl( kAction, false );
			}
			//콤보키 입력 사용 불가
			SetEnableComboKeyEnter(kAction, false);
			//연계콤보 사용하냐

			SetEnableComboKeyEnter(kAction, false);

			std::string const kUseComboParam = kAction.GetScriptParam("USE_COMBO");
			if( !kUseComboParam.empty()
				&& STR_TRUE == kUseComboParam )
			{// 있다면
				SetUseCombo( kAction, true );
			}
			else
			{// 없다면
				SetUseCombo( kAction, false );
			}
			//차지 스킬이냐
			bool bChargeCombo = false;
			SetUseChargeCombo( kAction, false );
			std::string const kUseChargeActionParam = kAction.GetScriptParam("USE_COMBO_CHARGE");
			if( !kUseChargeActionParam.empty()
				&& STR_TRUE == kUseChargeActionParam )
			{// 있다면
				SetUseChargeCombo( kAction, true );
				if (g_pkWorld)
				{
					g_pkWorld->SetShowWorldFocusFilter( static_cast<DWORD>(0x000000), 0.0, 0.4, 0.4,false,true, false);
				}
				kActor.CallComboCharge();
				bChargeCombo = true;
			}
			else
			{// 없다면
				SetUseChargeCombo( kAction, false );
			}

			g_kComboAdvisor.OnNewActionEnter(kAction.GetID(), bChargeCombo);
			lwPilot kPilot = kActor.GetPilot();
			if( !kPilot.IsNil() )
			{
				std::list<int> kNextComboList;
				if( kPilot.GetNextComboList( kAction.GetActionNo(), kNextComboList ) )
				{
					CONT_DEFSKILL const *pkDefSkill = NULL;
					g_kTblDataMgr.GetContDef( pkDefSkill );
					if( pkDefSkill )
					{
						std::list<int>::const_iterator iter_list = kNextComboList.begin();
						while( kNextComboList.end() != iter_list )
						{
							CONT_DEFSKILL::const_iterator skill_itor = pkDefSkill->find( (*iter_list) );
							if( pkDefSkill->end() != skill_itor )
							{
								g_kComboAdvisor.AddNextAction( MB((*skill_itor).second.chActionName) );
							}
							++iter_list;
						}
						g_kComboAdvisor.ChangeChildNode(false);
					}
				}
				else
				{
					g_kComboAdvisor.CompleteNode(true);
				}
			}
		}
		if( 0 == strcmp("TRUE" , kAction.GetScriptParam("SAVE_CAMERA_POS")) )
		{//최초 카메라 위치를 저장해야 한다면
			if( NULL != g_pkWorld )
			{
				g_pkWorld->m_kCameraMan.SaveCameraPos();
			}
		}
		//카메라 줌인 카운트 초기화
		ResetZoomCameraCount(kAction);
		return true;
	}

	void OnOverridePacket(lwActor kActor, lwAction kAction, lwPacket kPacket)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return; }
		if( kAction.IsNil() )	{ lwSetBreak(); return; }
		
		if( STR_TRUE == kAction.GetScriptParam("MOVE_SYNC_INPUT_DIR") )
		{// 방향을 맞춤
			kPacket.PushInt( static_cast<int>(EAOPT_SYNC_DIR) );
			kPacket.PushByte( kAction.GetDirection() );
		}
	}

	bool ProcessOverridePacket(lwActor kActor, lwAction kAction)
	{
		lwPacket kPacket = kAction.GetParamAsPacket();
		switch( kPacket.PopInt() )
		{
		case EAOPT_SYNC_DIR:
			{
				BYTE byDir =  kPacket.PopByte();
				kAction.SetDirection( byDir );
			}return true;;
		}
		return false;
	}
	void ProcessOnAniSlotTime(lwActor kActor, lwAction kAction, int const iCurAniSlot)
	{// 각 애니 슬롯이 되었을때의 처리
		AttachParticleOnAniSlotTime(kActor, kAction, iCurAniSlot);	// 현재 슬롯일때 처리할 파티클이 존재 하는가
		ProcessEtc( kActor, kAction, EAT_ANI_SLOT_TIME );
		
		{// 사운드를 플레이 해야하는가
			BM::vstring kSoundID("SOUND_WHEN_ANI_SLOT");
			kSoundID+=iCurAniSlot;
			std::string const kParam = kAction.GetScriptParam(kSoundID.operator const std::string().c_str());
			if( !kParam.empty() )
			{
				kActor.AttachSoundToPoint(0, kParam.c_str(), kActor.GetPos(), 0.0f);
			}
		}

		{// 이시점에 무기를 감추어야 하는가
			BM::vstring kHideWeaponParam("HIDE_WEAPON_WHEN_ANI_SLOT");
			kHideWeaponParam+=iCurAniSlot;
			std::string const kParam = kAction.GetScriptParam(kHideWeaponParam.operator const std::string().c_str());
			if( STR_TRUE == kParam )
			{
				HideWeapon(kActor, kAction, true);
			}
			else if( STR_FALSE == kParam )
			{
				HideWeapon(kActor, kAction, false);
			}
		}
		{// 이시점에 무기 모양을 바꿔야 하는가
			BM::vstring kChangeWeaponParam("CHANGE_WEAPON_XML_WHEN_ANI_SLOT");
			kChangeWeaponParam+=iCurAniSlot;
			std::string const kParam = kAction.GetScriptParam(kChangeWeaponParam.operator const std::string().c_str());
			if( STR_TRUE == kParam )
			{
				BM::vstring kWeaponXmlParam("CHANGE_WEAPON_XML_ANI_SLOT");
				kWeaponXmlParam+=iCurAniSlot;
				std::string const kParam_WeaponXml = kAction.GetScriptParam(kWeaponXmlParam.operator const std::string().c_str());
				if( !kParam_WeaponXml.size() )
				{
					return;
				}
				kActor.ChangeItemModel(EQUIP_LIMIT_WEAPON, kParam_WeaponXml.c_str() );
			}
		}
		{// 이시점에 무기 모양을 복원해야 하는가
			BM::vstring kRestoreWeaponParam("RESTORE_WEAPON_WHEN_ANI_SLOT");
			kRestoreWeaponParam+=iCurAniSlot;
			std::string const kParam = kAction.GetScriptParam(kRestoreWeaponParam.operator const std::string().c_str());
			if( STR_TRUE == kParam )
			{
				kActor.RestoreItemModel(EQUIP_LIMIT_WEAPON);
			}
		}
	}

	bool OnCastingCompleted(lwActor kActor, lwAction kAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kAction.IsNil() )	{ lwSetBreak(); return false; }
		bool const bIsLoop = kAction.IsSlotLoop();
		if( 1 < kAction.GetSlotCount()
			&& (bIsLoop || (!bIsLoop && kActor.IsAnimationDone()) )
			)
		{
			int const iNextSlot = kAction.GetCurrentSlot()+1;
			kAction.SetSlot(iNextSlot); // 첫 슬롯은 캐스팅 모션(캐스팅이 없더라도 의무적으로 넣어주어야함)
			SetCurSpecifyProcessAniSlot(kAction, iNextSlot);
			kActor.PlayCurrentSlot(false);
		}
		ProcessEtc( kActor, kAction, EAT_CAST_COMPLETE );
		return true;
	}

	bool OnUpdate(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kAction.IsNil() )	{ lwSetBreak(); return false; }

		std::string const kEndParam = kAction.GetParam(1);
		if("end" == kEndParam)
		{
			return false;
		}
		// 캐스팅 시간 감소에 의한 애니 속도 수정
		ApplyAniSpeedOnUpdate(kActor, kAction);
		
		{
			int const iCurAniSlot = kAction.GetCurrentSlot();
			int const iCurSpecifyProcessAniSlot = GetCurSpecifyProcessAniSlot(kAction);

			if( iCurSpecifyProcessAniSlot == iCurAniSlot )
			{// iCurAniSlot일때 해야할 수행을 처리 하는데
				SetCurSpecifyProcessAniSlot(kAction, iCurSpecifyProcessAniSlot+1); // 현재슬롯의 실행부분이 다시 반복되지 않기 위해
				int const iCurBeginIndex = IDX_SPECIFY_PROCESS_ANI_SLOT_PARAM 
					+ (iCurSpecifyProcessAniSlot * 100);

				{// 바닥에 닿아야 다음 애니를 하기가 있는지 확인해서 처리해 주고
					int const iParam = kAction.GetParamInt(iCurBeginIndex+ SUB_IDX_MEET_FOOLR_THEN_NEXT_ANI_SLOT); // 해야할 동작의 on/off등을 얻어와 동작하게끔
					SetMeetFloorThenPlayNextAni(kAction, 0 < iParam);
				}
				if( IsSeeFront(kAction) )
				{// 정면을 바라봐야 한다면 보게 해주고(update시에 설정해줘야 바라봄. 바람직한 형태가 아님. 애니 slot 변할대마다 한번씩 실행하게됨)
					kActor.SeeFront(true, true);
				}
				ProcessEtc(kActor, kAction, static_cast<EActionTime>(EAT_ANI_SLOT0+iCurAniSlot));
			}
		}

		if( kActor.IsMyActor())
		{
			///////////////////////////////////////////////////
			if( IsMoveControl(kAction) )
			{
				OnMoveControl(kActor, kAction);
			}
			///////////////////////////////////////////////////
			lwPilot kPilot = kActor.GetPilot();
			if( !kPilot.IsNil() )
			{
				if( kPilot.IsHaveComboAction(kAction.GetActionNo()) ||
					kPilot.IsHaveConnectComboAction(kActor.GetEquippedWeaponType(), kAction.GetActionNo()) )
				{// 연계콤보 애니라면
					bool const bCharge = IsUseChargeCombo(kAction);
					if( bCharge )
					{//현재 액션이 차지 준비 액션일 경우
						int iNextActionNo_SC = 0;
						int iNextActionNo = 0;
						if( kPilot.HaveInputKeyState_Release() && kPilot.IsInputAttackKey())
						{// 릴리즈 된 [공격]키가 하나라도 있으면 키입력 체크
							//기본콤보 스킬 검사
							iNextActionNo = FindComboAction(kActor, kAction, bCharge, iNextActionNo_SC);
							if( !iNextActionNo )
							{//키입력 정보 없으면 바로 종료
								kActor.CutComboCharge();
								kAction.SetParam(1, "end");
								return false;
							}
							else if( IsUseCombo(kAction) )
							{//들어온 키조합에 맞는 연계콤보가 있으면 그걸로 연결
								lwAction kNextAction = kActor.ReserveTransitActionByActionNo(iNextActionNo, kActor.GetDirection() );
								if( !kNextAction.IsNil() && IsComboDelay(kActor) )
								{//딜레이 애니일 경우 바로 현재 애니 캔슬
									if(bCharge)
									{
										kActor.CutComboCharge();
									}
									kAction.ChangeToNextActionOnNextUpdate(false, true);
									kNextAction.SetSlot(0);
									kAction.SetParam(1, "end");
									if( iNextActionNo_SC )
									{
										SetNextSubPlayerAction(kNextAction, iNextActionNo_SC);
									}
									return false;
								}
							}
						}
					}
					if( IsEnableComboKeyEnter(kAction) && kPilot.IsInputAttackKey())
					{//키입력 타임중에 && 공격키가 들어왔을 경우
						int iNextActionNo_SC = 0;
						int iNextActionNo = 0;
						//기본콤보 스킬 검사
						iNextActionNo = FindComboAction(kActor, kAction, bCharge, iNextActionNo_SC);
						if( IsUseCombo(kAction) )
						{
							if(!iNextActionNo && !bCharge)
							{//기본 공격키에 물리는 공격 액션 검색(차지 아닐때만)
								char const *pcActionID = kPilot.FindActionID( kPilot.GetInputAttackKey() );
								if( pcActionID )
								{
									lwAction kTempAction = kActor.ReserveTransitAction( pcActionID, 0 );
									if( !kTempAction.IsNil() )
									{
										iNextActionNo =kTempAction.GetActionNo();
									}
								}
							}
						}
						if( IsUseCombo(kAction) && iNextActionNo )
						{//들어온 키조합에 맞는 연계콤보가 있으면 그걸로 연결
							if( IsComboDelay(kActor) )
							{
								lwAction kNextAction = kActor.ReserveTransitActionByActionNo(iNextActionNo, kActor.GetDirection() );
								if( !kNextAction.IsNil() )
								{//딜레이 애니일 경우 바로 현재 애니 캔슬
									if(bCharge)
									{
										kActor.CutComboCharge();
									}
									kAction.ChangeToNextActionOnNextUpdate(false, true);
									kNextAction.SetSlot(0);
									kAction.SetParam(1, "end");
									if( iNextActionNo_SC )
									{
										SetNextSubPlayerAction(kNextAction, iNextActionNo_SC);
									}
									return false;
								}
							}
						}
						else if( kPilot.IsInputAttackKey() && !IsUseChargeCombo(kAction))
						{//차지 준비 액션이 아닐 때, 액션 못찾았는데 공격키가 들어온 상태면 입력키 초기화
							kPilot.ClearInputKey();
						}
					}
					int iNextActionNo = GetSkillUse(kAction);
					if( iNextActionNo )
					{//단축키 스킬 사용이 저장되어 있다면
						bool bUseSubActorSkill = false;
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef( iNextActionNo );
						if( pkSkillDef )
						{
							if( pkSkillDef->GetAbil(AT_SUB_PLAYER_ACTION) )
							{
								bUseSubActorSkill = true;
								kActor = kActor.GetSubPlayer();
								if( true == kActor.IsNil() )
								{
									return false;
								}
							}
						}
						lwAction kNextAction = kActor.ReserveTransitActionByActionNo(iNextActionNo, kActor.GetDirection() );
						if( !kNextAction.IsNil() && IsComboDelay(kActor) )
						{//딜레이 애니일 경우 바로 현재 애니 캔슬
							if(bCharge)
							{
								kActor.CutComboCharge();
							}
							kNextAction.SetSlot(0);
							if( false == bUseSubActorSkill )
							{
								kAction.ChangeToNextActionOnNextUpdate(false, false);
								kAction.SetParam(1, "end");
								return false;
							}
						}
					}
				}
				if( IsComboDelayCancle(kAction) )
				{//콤보 캔슬 딜레이 시, 화살표 입력된 값 찾아서 해당 방향으로 a_run 액션으로 전이
					NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
					if( pkKeyboard)
					{
						int const iDirKeySet[5] = {ACTIONKEY_JUMP, ACTIONKEY_LEFT, ACTIONKEY_RIGHT, ACTIONKEY_UP, ACTIONKEY_DOWN};
						BYTE const byDirSet[5] = {DIR_LOOKING, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN};
						for( int iKey = 0; iKey < 5; ++iKey)
						{
							//bool const bKeyDown = kPilot.FindInputKey( iDirKeySet[iKey] );
							int const iKeyNum = g_kGlobalOption.GetUKeyToKey(iDirKeySet[iKey]);
							NiInputKeyboard::KeyCode kKeycode = static_cast<NiInputKeyboard::KeyCode>(iKeyNum);
							bool const bKeyDown = pkKeyboard->KeyIsDown( kKeycode );
							if( bKeyDown )
							{
								char const *pcActionID = kPilot.FindActionID( iDirKeySet[iKey] );
								if( pcActionID )
								{
									lwAction kNextAction = kActor.ReserveTransitAction( pcActionID, byDirSet[iKey] );
									if( false == kNextAction.IsNil() )
									{
										SetEnableComboKeyEnter(kAction, false);
										kAction.ChangeToNextActionOnNextUpdate(false, false);
										kNextAction.SetSlot(0);
										kAction.SetParam(1, "end");
										return false;
									}
								}
							}
						}
					}
				}
			}
		}

		if( kActor.IsAnimationDone() )
		{// 현재 슬롯의 애니가 완료 되어
			if( kActor.IsMyActor() )
			{
				lwPilot kPilot = kActor.GetPilot();
				if( !kPilot.IsNil() )
				{
					if( kPilot.IsHaveComboAction(kAction.GetActionNo()) ||
						kPilot.IsHaveConnectComboAction(kActor.GetEquippedWeaponType(), kAction.GetActionNo()) )
					{// 연계콤보 애니라면
						bool const bCharge = IsUseChargeCombo(kAction);
						if( kPilot.IsInputAttackKey() )
						{//키입력 타임중에
							int iNextActionNo_SC = 0;
							int iNextActionNo = 0;
							iNextActionNo = FindComboAction(kActor, kAction, bCharge, iNextActionNo_SC);
							if(!iNextActionNo && !bCharge)
							{//기본 공격키에 물리는 공격 액션 검색
								char const *pcActionID = kPilot.FindActionID( kPilot.GetInputAttackKey() );
								if( pcActionID )
								{
									lwAction kTempAction = kActor.ReserveTransitAction( pcActionID, 0 );
									if( !kTempAction.IsNil() )
									{
										iNextActionNo =kTempAction.GetActionNo();
									}
								}
							}
							if( IsUseCombo(kAction) && iNextActionNo )
							{//들어온 키조합에 맞는 연계콤보가 있으면 그걸로 연결
								lwAction kNextAction = kActor.ReserveTransitActionByActionNo(iNextActionNo, kActor.GetDirection() );
								if( !kNextAction.IsNil() && IsComboDelay(kActor) )
								{//딜레이 애니일 경우 바로 현재 애니 캔슬
									if(bCharge)
									{
										kActor.CutComboCharge();
									}
									kAction.ChangeToNextActionOnNextUpdate(false, true);
									kNextAction.SetSlot(0);
									kAction.SetParam(1, "end");
									if( iNextActionNo_SC )
									{
										SetNextSubPlayerAction(kNextAction, iNextActionNo_SC);
									}
									return false;
								}
							}
							else if( kPilot.IsInputAttackKey() && !bCharge)
							{//차지 준비 액션이 아닐 때, 액션 못찾았는데 공격키가 들어온 상태면 입력키 초기화
								kPilot.ClearInputKey();
							}
						}	
						int iNextActionNo = GetSkillUse(kAction);
						if( iNextActionNo )
						{//단축키 스킬 사용이 저장되어 있다면
							bool bUseSubActorSkill = false;
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef( iNextActionNo );
							if( pkSkillDef )
							{
								if( pkSkillDef->GetAbil(AT_SUB_PLAYER_ACTION) )
								{
									bUseSubActorSkill = true;
									kActor = kActor.GetSubPlayer();
									if( true == kActor.IsNil() )
									{
										return false;
									}
								}
							}
							lwAction kNextAction = kActor.ReserveTransitActionByActionNo(iNextActionNo, kActor.GetDirection() );
							if( !kNextAction.IsNil() && IsComboDelay(kActor) )
							{//딜레이 애니일 경우 바로 현재 애니 캔슬
								if(bCharge)
								{
									kActor.CutComboCharge();
								}
								kNextAction.SetSlot(0);
								if( false == bUseSubActorSkill )
								{
									kAction.ChangeToNextActionOnNextUpdate(false, false);
									kAction.SetParam(1, "end");
									return false;
								}
							}
						}
					}
				}
			}
			int const iResult = IsAbleToNextAniSlot(kActor, kAction, fAccumTime, fFrameTime);
			if( ENAIR_NONE == iResult )
			{// 다음 애니 슬롯으로 넘어갈수 있는 조건이라면
				if( !kActor.PlayNext() )
				{
					kAction.SetParam(1, "end");
					return false;
				}
			}
			else
			{// 넘어 갈수 없다면, 그 이유에 대한 후처리를 진행하고
				ProcessNextAniIncompleteReason(iResult, kActor, kAction, fAccumTime, fFrameTime);
			}
		}

		float const fSpinDegPerSec = GetSpinDegPerSec(kAction);
		if(0.0f != fSpinDegPerSec )
		{
			kActor.IncRotate( NxMath::degToRad(fSpinDegPerSec*fFrameTime) );
		}

		ProcessMoving(kActor, kAction, fAccumTime, fFrameTime);
		
		return true;
	}


	bool OnLeave(lwActor kActor, lwAction kNextAction, bool bCancel)
	{// 다른 액션으로 전이
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kNextAction.IsNil() )	{ lwSetBreak(); return false; }

		lwAction kCurAction =  kActor.GetAction();
		if( kCurAction.IsNil() )	{ lwSetBreak(); return false; }

		int iNextSkillNo = kNextAction.GetActionNo();
		std::string const kEndParam = kCurAction.GetParam(1);
		std::string const kNextActionID = kNextAction.GetID();
		bool bCanLeave = false;
		
		if("end" == kEndParam)
		{// 정상적인 과정을 통해 전이 되는것이라면
			if( kNextActionID == ACTIONNAME_JUMP )
			{// 다음 액션이 jump 라면 떨어지는 모션을 취하게 한다
				kNextAction.SetSlot(2);
				kNextAction.SetDoNotBroadCast(true);
			}
			bCanLeave = true;
		}

		if( false == kActor.IsUnderMyControl() ) 
		{// 타인이라면 추가적으로 확인할 필요가 없고
			return true;
		}
		// 내 제어 하에 있는 Actor이고

		if( true ==  kActor.IsUnitType(UT_SUB_PLAYER) )
		{// SubPlayer(쌍둥이등)이라면 기본 액션으로 돌려주고
			TryMustChangeActorAction(kActor, "a_twin_sub_trace_ground");
			bCanLeave = true;
		}
		
		if( CheckCancelAction(kActor, kCurAction, kNextAction) )
		{// 캔슬 가능한지 확인하고
			return true;
		}

		if( ESS_CASTTIME == kCurAction.GetActionParam() ) 
		{// 캐스팅중에 끝난 액션
			kCurAction.SetParam(1, "end");
			kCurAction.SetNextActionName( kNextActionID.c_str() );
			return true;
		}
		else
		{// 아니라면 자동 전이되어선 안되니까 미리 체크 하고
			BM::vstring const VSTR_TRANSIT_ACTION("TRANSIT_ACTION");
			BM::vstring const VSTR_TRANSIT_ACTION_CONDITION("TRANSIT_ACTION_CONDITION");
			for(int i=1; i <=10; ++i)
			{
				// 액션 이름을 얻어오고
				BM::vstring vStr = VSTR_TRANSIT_ACTION+i;
				BM::vstring const vHeadCondiStr = VSTR_TRANSIT_ACTION_CONDITION+i;
				std::string const kActionParamStr = kCurAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( kActionParamStr.empty() )
				{// 액션 이름이 존재하면
					break;
				}
				bool bTransitAction = true;
				{// 기본외에 다른 액션으로 전이하는데 필요한 조건을
					for(int j=1; j <= 10; ++j)
					{// 얻어와
						BM::vstring vTotalCondiStr = vHeadCondiStr + "_";
						vTotalCondiStr+=j;
						int const iCondition = kCurAction.GetScriptParamAsInt( vTotalCondiStr.operator const std::string().c_str() );
						if(0 == iCondition)	{ break; }
						switch(iCondition)
						{// 체크 하고
						case ENTACT_FIND_TARGET:
							{// 멈춘 자리에서 타겟이 잡히면
								PgAction* pkTempAction = kActor()->CreateActionForTransitAction( kActionParamStr.c_str() );
								lwAction kTransitAction( pkTempAction );
								if( false == kTransitAction.IsNil() )
								{
									kTransitAction.CreateActionTargetList(kActor, true); // 내 플레이어만 CreateActionTargetList를 사용
									lwActionTargetList kTargetList = kTransitAction.GetTargetList();
									bTransitAction = 0 < kTargetList.size();
									kTransitAction.ClearTargetList();
									g_kActionPool.ReleaseAction( pkTempAction );
								}
							}break;
						case ENTACT_IS_PLAYER:
							{// 유닛 타입이 PLAYER이고
								bTransitAction = kActor.IsUnitType(UT_PLAYER);
								if( bTransitAction )
								{// 기본적으로 전이 되어야할 액션이 들어 왔다면
									std::string const kBasicNextActionName( kCurAction.GetNextActionName() );	// 원래 기본적으로 전이 되어야할 액션 이름
									std::string const kNextActionName( kNextAction.GetID() );					// 현재 전이 되려고 들어온 액션 이름
									bTransitAction = (kNextActionName == kBasicNextActionName);
								}
							}break;
						default:
							{
								bTransitAction = false;
							}break;
						}
						// 한개라도 안맞으면 전이 실패로 체크하고
						if(false == bTransitAction) { break; }
					}
				}
				if(bTransitAction)
				{// 기본 액션 외의 다른 액션으로 전이 가능하면 전이 시킴
					bCanLeave = true;
					kNextAction.SetNextActionName( kActionParamStr.c_str() );
					kNextAction.ChangeToNextActionOnNextUpdate(true,true);
					break;
				}
			}
		}
		/*	
		if( 반드시 예정된 Action으로 진행 되어야 한다면 
		&& kNextActionID != kCurAction.GetNextActionName()
		)
		{
		ODS("다른게 들어옴:"..actionID.."\n",false, 912)
		return false;
		}
		*/
		
		return bCanLeave;
	}

	bool OnCleanUp(lwActor kActor, lwAction kNextAction)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kNextAction.IsNil() )	{ lwSetBreak(); return false; }

		lwAction kCurAction =  kActor.GetAction();
		if( kCurAction.IsNil() )	{ lwSetBreak(); return false; }
		if(	!g_pkWorld )	{ return false; 	}

		ProcessEtc( kActor, kCurAction, EAT_ON_CLEANUP );
		
		if( IsMoving(kCurAction) )
		{
			SetMoving(kActor, kCurAction, false);
		}
		if( IsStopJump(kCurAction) )
		{
			SetStopJump(kActor, kCurAction, false);
		}

		{// 붙었던 파티클 제거
			int iCnt = kCurAction.GetParamInt(IDX_PARTICLE_SLOT_AT_CAST_COMPLETE);
			for(int i=1; i <= iCnt; ++i)
			{// fire 캐스팅 완료 시점에 붙은 파티클 제거
				kActor.DetachFrom(IDX_PARTICLE_SLOT_AT_CAST_COMPLETE+i, true);
			}

			iCnt = kCurAction.GetParamInt(IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG);
			for(int i=1; i <= iCnt; ++i)
			{// fire 이벤트 메세지때 붙었던 파티클 제거
				kActor.DetachFrom(IDX_PARTICLE_SLOT_AT_ON_FIRE_MSG+i, true);
			}

			iCnt = kCurAction.GetParamInt(IDX_PARTICLE_SLOT_AT_ON_HIT_MSG);
			for(int i=1; i <= iCnt; ++i)
			{// hit 이벤트 메세지때 붙었던 파티클 제거
				kActor.DetachFrom(IDX_PARTICLE_SLOT_AT_ON_HIT_MSG+i, true);
			}
			
			{//총 몇개의 effect 메세지 중에
				iCnt = GetMsgEffectCnt(kCurAction);
				for(int i=1; i <= iCnt; ++i)
				{//
					int const iIdx = IDX_PARTICLE_SLOT_AT_ON_EFFECT_MSG+(i-1)*10;
					int const iMaxCnt = kCurAction.GetParamInt(iIdx);
					for(int j=0; j < iMaxCnt; ++j)
					{
						kActor.DetachFrom(iIdx+j, true);
					}
				}
			}

			for(int iAniSlot=0; iAniSlot < kCurAction.GetSlotCount(); ++iAniSlot)
			{// 애니 슬롯 시점에 붙었던 파티클 제거
				DetachAniSlotParticle(kActor, kCurAction, iAniSlot, EAT_ON_CLEANUP);
			}
		}

		HideWeapon(kActor, kCurAction, false);
		kActor.RestoreItemModel(EQUIP_LIMIT_WEAPON);

		if( true == kActor.IsUnderMyControl()
			&& true == kActor.GetActionToggleState( kCurAction.GetActionNo() ) 
			&& 0 < kCurAction.GetAbil(AT_WHEN_ACTION_END_THEN_TOGGLE_OFF)
			)
		{// 액션을 끝낼때 토글을 종료하는 스킬인가
			BM::Stream kPacket(PT_C_M_REQ_TOGGLSKILL_OFF);
			kPacket.Push( kActor.GetPilotGuid()() );
			kPacket.Push( kCurAction.GetActionNo() );
			NETWORK_SEND( kPacket );
		}
		

		if( kActor.IsUnitType(UT_ENTITY) )
		{// ENTITY actor를 삭제 할것인가
			std::string const kParam = kCurAction.GetScriptParam("REMOVE_ON_NEXTUPDATE");
			if( STR_TRUE == kParam )
			{
				g_pkWorld->RemoveObjectOnNextUpdate( kActor.GetPilotGuid()() );
			}
		}
		if( IsSeeFront( kCurAction ) )
		{// 정면 바라 보았다면 맵의 방향으로 복구 하고
			kActor.RestoreLockBidirection();
		}

		if( true == IsKeepCamMode(kCurAction) )
		{// 카메라 모드를 저장한다면 다음 액션으로 저장해 준다
			SetCurCamMode( kNextAction, GetCurCamMode(kCurAction) );
		}

		if( kActor.IsMyActor() )
		{
			lwPilot kPilot = kActor.GetPilot();
			if( !kPilot.IsNil() )
			{
				std::string const kUseChargeActionParam = kNextAction.GetScriptParam("USE_COMBO_CHARGE");
				if( kUseChargeActionParam.empty()
					|| STR_TRUE != kUseChargeActionParam )
				{// 다음 액션이 차지 액션이 아니라면 키입력 리셋
					kPilot.ClearInputKey();
				}

				if( kPilot.IsHaveConnectComboAction( kActor.GetEquippedWeaponType(), kNextAction.GetActionNo() ) )
				{
					SetConnectCombo(kNextAction, kCurAction.GetActionNo() );
				}
			}
			if( IsUseChargeCombo(kCurAction) )
			{
				g_pkWorld->SetShowWorldFocusFilter( static_cast<DWORD>(0x000000), 1.0, 0.0, 0.4,true,true, false);
			}
			if( false == kPilot.IsHaveComboAction( kNextAction.GetActionNo() ) )
			{
				kActor.CutComboCharge();
				g_kComboAdvisor.ResetComboAdvisor();
				ProcessCameraZoomRestore(kActor, kCurAction, EAT_ON_CLEANUP_COMBO_CANCEL);
			}
		}
		switch( GetWorldFoucsFilterColorAlpha_Keep(kCurAction) )
		{
		case EWFFCAKT_KEEP_UNTIL_CLEAN_UP:
			{
				g_pkWorld->ResetFocusFilter();
			}break;
		}

		ReleaseHideNameTitle(kActor, kCurAction);
		ReleaseHideSubActor(kActor, kCurAction);
		return true;
	}

	bool OnEvent(lwActor kActor, char const* pcText, int iSeqID)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		lwAction kCurAction = kActor.GetAction();
		if( kCurAction.IsNil() )	{ lwSetBreak(); return false; }
		if(NULL == pcText)
		{
			return false;
		}
		std::string const kTextKey(pcText);

		if( STR_EVENT_MSG_HIT == kTextKey )
		{
			ProcessEtc( kActor, kCurAction, EAT_MSG_HIT );
		}
		else if(STR_EVENT_MSG_MOVE_START == kTextKey)
		{
			if( InitMovingInfo(kActor, kCurAction, kTextKey.c_str(), lwGetAccumTime()) )
			{
				SetMoving(kActor, kCurAction, true);
			}
			else
			{
				if( 0 == kActor.GetAbil(AT_NOT_MOVE) )
				{
					lua_tinker::call< void >("ChaseActor", kActor, kCurAction);
				}
			}
			AttachParticle(kActor, kCurAction, EAT_MSG_MOVE_START);
		}
		else if(STR_EVENT_MSG_MOVE_STOP == kTextKey)
		{
			switch( GetMoveType(kCurAction) )
			{
			case EMT_INTERPOLATION:
				{// 보간이 완료 되지 않았는데 메세지가 들어오는 경우가 있으므로 여기서 중지시키면 안됨.
				}break;
			default:
				{
					SetMoving(kActor, kCurAction, false);
				}break;
			}
			AttachParticle(kActor, kCurAction, EAT_MSG_MOVE_END);
		}
		else if(STR_EVENT_MSG_STOP_JUMP_START == kTextKey)
		{
			SetStopJump(kActor, kCurAction,  true);
		}
		else if(STR_EVENT_MSG_STOP_JUMP_END == kTextKey)
		{
			SetStopJump(kActor, kCurAction, false);
		}
		else if(STR_EVENT_MSG_FIRE == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_FIRE);
		}
		else if(STR_EVENT_MSG_SC_START == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_SC_START);
		}
		else if(STR_EVENT_MSG_ALPHA_START == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_ALPHA);
		}
		else if(STR_EVENT_MSG_QUAKE_CAM == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_QUAKE_CAM);
		}
		else if(STR_EVENT_MSG_QUAKE_MY_CAM == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_QUAKE_MY_CAM);
		}
		else if(STR_EVENT_MSG_EFFECT == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_EFFECT);
		}
		else if(STR_EVENT_MSG_CHANGE_CAM == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_CHANGE_CAMERA);
		}
		else if(STR_EVENT_MSG_TAT_START == kTextKey)
		{//콤보 키 입력 시작
			if( kActor.IsMyActor() )
			{					
				lwPilot kPilot = kActor.GetPilot();
				if( !kPilot.IsNil() )
				{
					SetEnableComboKeyEnter(kCurAction, true);
					if( !IsUseChargeCombo(kCurAction) )
					{//현재 액션이 차지액션이 아닐 경우, 남아있는 키입력 상태를 비운다.
						kPilot.ClearInputKey();
					}
					else
					{//현재 액션이 차지액션일 경우, 남아있는 키입력 상태를 갱신한다.
						kPilot.RefreshInputKey();
						kActor.SetComboCharge(2000);
					}
				}
				g_kComboAdvisor.ChangeChildNode(true);
			}
		}
		else if(STR_EVENT_MSG_TAT_END == kTextKey)
		{//콤보 키입력 끝
			if( kActor.IsMyActor() )
			{
				SetEnableComboKeyEnter(kCurAction, false);
				if( IsUseChargeCombo(kCurAction) )
				{//현재 액션이 차지액션일 경우 현재까지 입력된 상태를 전부 놓은 상태로 수정한다.
					lwPilot kPilot = kActor.GetPilot();
					if( !kPilot.IsNil() )
					{
						kPilot.SetInputKeyState_Release();
					}
				}
				g_kComboAdvisor.ClearNextAction();
			}
		}
		else if(STR_EVENT_MSG_COMBO_DELAY_CANCEL == kTextKey)
		{//콤보 딜레이 캔슬 가능
			if( kActor.IsMyActor() )
			{
				SetComboDelayCancle(kCurAction, true);
				lwPilot kPilot = kActor.GetPilot();
				if( !kPilot.IsNil() )
				{
					if( !IsUseChargeCombo(kCurAction) )
					{//현재 액션이 차지액션이 아닐 경우, 남아있는 키입력 상태를 비운다.
						kPilot.ClearInputKey();
					}
				}
			}
		}
		else if(STR_CAMERA_ZOOM_WHEN_Z_CAM_MSG == kTextKey)
		{//카메라 줌인 기능
			if( kActor.IsMyActor() )
			{
				int iCount = GetZoomCameraCount(kCurAction);
				BM::vstring kZoomMsg(L"CAMERA_ZOOM_WHEN_Z_CAM_MSG_#COUNT#");
				kZoomMsg.Replace( L"#COUNT#", iCount );
				float fZoom = kCurAction.GetScriptParamAsFloat( static_cast<std::string>(kZoomMsg).c_str() );
				if( fZoom )
				{
					PgCameraModeFollow::SetCameraZoom(fZoom);
				}
				IncreaseZoomCameraCount(kCurAction);
			}
		}
		else if(STR_UI_MSG == kTextKey)
		{
			ProcessEtc(kActor, kCurAction, EAT_MSG_UI);
		}
		return true;
	}

	bool OnTimer(lwActor kActor, lwAction kAction, float fCallTime, int iTimerID)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return false; }
		if( kAction.IsNil() )	{ lwSetBreak(); return false; }

		switch(iTimerID)
		{
		case 0:	//ATTACK_TIMER_ID
			{
				if( kActor.IsUnderMyControl() )
				{// 내 제어하에 있는 Actor 이면, 타겟을 잡아 서버로 타격 요청을 하고
					if( IsUseTLM(kAction) )
					{// 스킬 사용 중간에 다시 때릴수 있고
						if( false == IsOverHitCnt(kAction) )
						{// 최대 공격 수를 넘지 않았다면
							IncHitCnt(kAction);
							kAction.CreateActionTargetList(kActor, false, false);
							if( 0 < kAction.GetTargetList().size() )
							{
								kAction.BroadCastTargetListModify(kActor.GetPilot(), false);
							}
						}
					}
					kAction.ClearTargetList();
				}
			}break;
		default:
			{
			}break;
		}

		return true;
	}

	int OnFindTarget(lwActor kActor, lwAction kAction, lwActionTargetList kTargetList)
	{// 일반적으로 USE_CFUNC_ONTARGET="TRUE"로 사용하게끔하고 아주 특수한 경우 커스텀하게 타겟을 찾아야할때만 이것을 사용
		if( kActor.IsNil() )	{ lwSetBreak(); return 0; }
		if( kAction.IsNil() )	{ lwSetBreak(); return 0; }

		return 0;
	}

	void OnTargetListModified(lwActor kActor, lwAction kAction, bool bIsBefore)
	{
		if( kActor.IsNil() )	{ lwSetBreak(); return; }
		if( kAction.IsNil() )	{ lwSetBreak(); return; }
		//	if(0 == kAction.GetParamInt(100) )
		//	{
		if( false == bIsBefore )
		{// hit 키 이후라면, 바로 대미지 적용시켜 준다
			DefaultHitOneTime(kActor, kAction, true);
		}
		//	}
	}

	void OnMoveControl(lwActor kActor, lwAction kAction)
	{//입력키로 액터 이동
		if( kActor.IsNil() )	{ lwSetBreak(); return; }
		if( kAction.IsNil() )	{ lwSetBreak(); return; }
		if( NULL == g_pkWorld ) { lwSetBreak(); return; }

		if( 0 != kActor.GetAbil(AT_NOT_MOVE) )
		{//이동불가 어빌이 있다면
			return;
		}

		NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
		if( !pkKeyboard )
		{
			return;
		}
		int arrActionKey[4] = {ACTIONKEY_UP, ACTIONKEY_DOWN, ACTIONKEY_LEFT, ACTIONKEY_RIGHT};
		int arrDir[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
		int iDir = 0;
		for( int iCount=0; iCount<4; ++iCount )
		{
			int iKeyNum = g_kGlobalOption.GetUKeyToKey( arrActionKey[iCount] );
			NiInputKeyboard::KeyCode kKeycode = static_cast<NiInputKeyboard::KeyCode>(iKeyNum);
			bool const bKeyDown = pkKeyboard->KeyIsDown(kKeycode);
			if( bKeyDown )
			{
				iDir += arrDir[iCount];
			}

		}
		if( iDir != 0 )
		{
			int const iMoveSpeed = kActor.GetAbil(AT_MOVESPEED);
			lwPoint3 kDirection = kActor.GetDirectionVector(iDir);
			kDirection.Multiply(iMoveSpeed);
			kActor.SetMovingDelta(kDirection);
			float fTimeDelta = g_pkWorld->GetAccumTime() - kAction.GetParamFloat(30);
			if(fTimeDelta > 0.02 ) // 서버로 이동 위치 갱신 보내는 시간 간격
			{
				g_kPilotMan.BroadcastDirection( g_kPilotMan.GetPlayerPilot(), iDir);
				kAction.SetParamFloat(30, g_pkWorld->GetAccumTime());
			}
		}
	}
	int FindComboAction(lwActor kActor, lwAction kAction, bool const bCharge, int &iNextAction_SC )
	{//기본콤보 스킬 검사
		if( kActor.IsNil() )	{ lwSetBreak(); return 0; }
		if( kAction.IsNil() )	{ lwSetBreak(); return 0; }
		lwPilot kPilot = kActor.GetPilot();
		if( kPilot.IsNil() )	{ lwSetBreak(); return 0; }

		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef( kAction.GetActionNo() );
		int iSkillNo = kAction.GetActionNo();
		if( pkSkillDef )
		{
			int iParentSkillNo = pkSkillDef->GetParentSkill();
			if( iParentSkillNo )
			{
				iSkillNo = iParentSkillNo;
			}
		}
		int iNextActionNo = kPilot.GetNextComboAction(iSkillNo, bCharge, iNextAction_SC );
		if( !iNextActionNo )	
		{
			int const iConnectActionNo = GetConnectCombo(kAction);
			if( iConnectActionNo )
			{//중계하는 스킬 검사
				iNextActionNo = kPilot.GetNextComboAction(iConnectActionNo, bCharge, iNextAction_SC );
			}
			if( !iNextActionNo )
			{
				if( kActor.IsMeetFloor() &&
					kPilot.IsHaveComboAction(kAction.GetActionNo()) )
				{//중계되는 콤보 검사
					iNextActionNo = kPilot.GetNextConnectComboAction( kActor.GetEquippedWeaponType() );
				}
			}
		}
		if( 0 != iNextAction_SC )
		{
			lwActor kSubActor = kActor.GetSubPlayer();
			if( false == kSubActor.IsNil() )
			{
				lwAction kSubAction_Cur = kSubActor.GetAction();
				if( false == kSubAction_Cur.IsNil() )
				{
					if( 0 != strcmp("TRUE" , kSubAction_Cur.GetScriptParam("USE_COMBO")) 
						&& ACTION_NO_IDLE != kSubAction_Cur.GetActionNo()
						&& ACTION_NO_BATTLE_IDLE != kSubAction_Cur.GetActionNo()
						&& ACTION_NO_TWIN_SUB_REPOS != kSubAction_Cur.GetActionNo()
						&& ACTION_NO_TWIN_SUB_TRACE_GROUND != kSubAction_Cur.GetActionNo() )
					{//서브캐릭 액션이 있을 때, 전이 가능한지 체크하고, 전이가 불가능하다면 전부 초기화 시켜버리자
						iNextActionNo = 0;
						iNextAction_SC = 0;
					}
				}
			}
		}
		return iNextActionNo;
	}
	//////////////////////////////////////////////////
}