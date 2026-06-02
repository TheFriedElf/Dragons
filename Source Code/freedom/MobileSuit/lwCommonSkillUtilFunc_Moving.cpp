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
#include "lwCommonSkillUtilFunc.h"
#include "lwWorld.h"

namespace lwCommonSkillUtilFunc
{// 위치 보간 관련 함수 모음 ----------------------------------------------------------------------------------------------------------------------------
	
	// 이동 방식
	std::string const STR_MOVE_TYPE_ = "MOVE_TYPE_";
		std::string const STR_MOVE_TYPE_INTERPOLATION = "I";				// 특정 위치까지 보간
		std::string const STR_MOVE_TYPE_VELOCITY = "V";						// 한 속도를 계속 적용
		std::string const STR_MOVE_TYPE_SEEK	= "SEEK";					// MOVE_GOAL_POS_TARGET_을 못찾으면 없으면 V(한 속도를 계속 적용)로 이동
		std::string const STR_MOVE_TYPE_SEEK2	= "SEEK2";					// MOVE_GOAL_POS_TARGET_을 못찾으면 이동하지 않음
		std::string const STR_MOVE_TYPE_TELEPORT	= "TELEPORT";				// 대상 지점으로 순간이동
		
	// 이동 속력 (V, SEEK, SEEK2 사용)
	std::string const STR_MOVE_SPEED_		= "MOVE_SPEED_";
	
	// 보간 완료시간(I 사용) - 반드시 이 시간에 해당 위치에 오게 됨
	std::string const STR_MOVE_COMPLETE_TIME_ = "MOVE_COMPLETE_TIME_";

	//목표 위치 설정(I, SEEK, SEEK2 사용) - 목표 위치는 기점 위치에서 이격 벡터를 더하는것으로 구함 (GoalPos = TargetPos + DistVec)
	std::string const STR_MOVE_GOAL_POS_TARGET_ = "MOVE_GOAL_POS_TARGET_";											// 이동 목적 위치 기점
	std::string const STR_MOVE_GOAL_POS_DIST_DIR_ = "MOVE_GOAL_POS_DIST_DIR_";									// 기점에서 이격될 거리의 방향
	std::string const STR_MOVE_GOAL_POS_DIST_DIR_ROTATE_HORIZ_DEG_ = "MOVE_GOAL_POS_DIST_DIR_ROTATE_HORIZ_DEG_";	// 이격 방향의 회전값
	std::string const STR_MOVE_GOAL_POS_DIST_DIR_ROTATE_VERT_DEG_ = "MOVE_GOAL_POS_DIST_DIR_ROTATE_VERT_DEG_";	// 이격 방향의 회전값
	std::string const STR_MOVE_GOAL_POS_DIST_ = "MOVE_GOAL_POS_DIST_";												// 기점에서 이격될 거리
	std::string const STR_WHEN_MOVE_DIRECTION("WHEN_MOVE_DIRECTION_");
	
	// 목표 위치를 업데이트 할것인가(I, SEEK, SEEK2 사용)
	std::string const STR_MOVE_GOAL_POS_UPDATE_ = "MOVE_GOAL_POS_UPDATE_";
	// 목표로 향하는 방향을 계속 업데이트 할것인가(I, SEEK, SEEK2 사용)
	std::string const STR_MOVE_DIR_UPDATE_ = "MOVE_DIR_UPDATE_";

	// 옵션적으로 멈출것(특정 조건에서 이동을 멈춘다)
	std::string const STR_MOVE_STOP_OPTION_ = "MOVE_STOP_OPTION_";
		std::string const STR_MEET_FLOOR = "MEET_FLOOR";					// 바닥에 닿으면 멈춘다
	

	void SetMoving(lwActor kActor, lwAction kAction, bool const bMove)
	{// 11000 - 이동시작 on/off
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 11000, bMove ? 1 : 0 );
		
		if( false == bMove )
		{// off시에
			if( IsStopJumpWhenMoving(kAction) )
			{// 보간중 stop jump를 사용 했다면, 다시 풀어준다
				SetStopJump(kActor, kAction, false);
			}
			kActor.SetMovingDelta( lwPoint3(0.0f,0.0f,0.0f) );
		}
	}
	bool IsMoving(lwAction kAction)
	{
		if( 0 < kAction.GetParamInt(11000) )
		{
			return true;
		}
		return false;
	}


	bool InitMovingInfo(lwActor kActor, lwAction kAction, lwWString kTextKey, float fStartTime)
	{// 누구로부터(콜러/나/etc), 방향으로(각도), 만큼(인치), 얼마만큼의 speed로 이동할것인가를 읽어서 세팅
		if( kActor.IsNil() )	{ return false; }
		if( kAction.IsNil() )	{ return false; }
		if(!g_pkWorld)
		{
			return false;
		}
		if( 0 != kActor.GetAbil(AT_NOT_MOVE) )
		{
			return false;
		}
		if( STR_EVENT_MSG_MOVE_START != kTextKey.GetStr() )
		{
			return false;
		}
		
		// 애니메이션의 TextKey중에 몇번째 'move_start'
		int const iCnt = IncMoveStartCnt(kAction);
		
		// 어떤 타입인지 얻어오고
		BM::vstring vStr(STR_MOVE_TYPE_); vStr+=iCnt;
		std::string const kStrParamType = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
		if( kStrParamType.empty() )
		{
			return false;
		}
		
		lwPoint3 kFrom = kActor.GetPos();
		lwPoint3 kDistVecDir = kActor.GetLookingDir();
		bool bFindGoalPosTarget = true;
		bool bFindGoalPosDistAxisTarget = true;
		{// 목적 위치를 구하기 위해
			PgActor* pkTargetActor = NULL;
			vStr = STR_MOVE_GOAL_POS_TARGET_; vStr+=iCnt;
			
			{// 목적 위치 기점과 타겟 대상을 정하고
				std::string const kStrParamFrom = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( STR_SELF == kStrParamFrom)
				{// 자신
					pkTargetActor = kActor();
				}
				else if( STR_CALLER == kStrParamFrom )
				{// 소환자(Caller)
					lwActor kCallerActor = kActor.GetCallerActor();
					if( kCallerActor.IsNil() )
					{
						bFindGoalPosTarget = false;
					}
					else
					{
						kFrom = kCallerActor.GetPos();
					}
					pkTargetActor = kCallerActor();
				}
				else if( STR_TARGET == kStrParamFrom )
				{// 타겟
					kAction.CreateActionTargetList(kActor, true, true);
					lwActionTargetList kTargetList = kAction.GetTargetList();
					bFindGoalPosTarget = 0 < kTargetList.size();
					if(bFindGoalPosTarget)
					{
						for(int i =0; i < 1/*kTargetList.size()*/; ++i)
						{
							lwActionTargetInfo kInfo =  kTargetList.GetTargetInfo(i);
							if( kInfo.IsNil() ) { break; }
							lwGUID kTargetGuid = kInfo.GetTargetGUID();
							if( kTargetGuid.IsNil() ) { break; }

							pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
							if( !pkTargetActor ) { break; }

							kFrom = pkTargetActor->GetPos(); //
						}
					}
					kAction.ClearTargetList();
				}
				else if ( STR_SAVED_TARGET == kStrParamFrom )
				{// OnEnter때에 저장된 타겟
					int const iCurIdx = GetCurUseSavedTargetIdx(kAction);
					int const iMaxCnt = GetMaxSavedTargetCnt(kAction);
					int const iNextIdx = iMaxCnt <= iCurIdx+1 ? 0 : iCurIdx+1;
					SetCurUseSavedTargetIdx(kAction, iNextIdx);
					
					lwGUID kTargetGuid = GetSavedTargetGuid(kAction, iCurIdx);
					if( kTargetGuid.IsNil() )
					{
						PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
						if( pkTargetActor )
						{
							kFrom = pkTargetActor->GetPos();
						}
					}
				}
				else
				{
					return false;
				}
				if( pkTargetActor )
				{// Target이 필요할때 다시 찾을수 있게 대상의 Guid를 저장
					PgPilot* pkTargetPilot = pkTargetActor->GetPilot();
					if( pkTargetPilot )
					{
						SetMovingTargetGuid(kAction, pkTargetPilot->GetGuid() );
					}
				}
			}
			
			{// 기점위치에서 이격 방향
				vStr = STR_MOVE_GOAL_POS_DIST_DIR_; vStr+=iCnt;
				std::string const kStrParamFrom = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
				if( STR_SELF == kStrParamFrom )
				{// 자신
					//kDistVecDir = kActor.GetLookingDir();
				}
				else if( STR_CALLER == kStrParamFrom )
				{// 소환자(Caller)
					lwActor kCallerActor = kActor.GetCallerActor();
					if( kCallerActor.IsNil() )
					{
						bFindGoalPosDistAxisTarget = false;
					}
					else
					{
						kDistVecDir = kCallerActor.GetLookingDir();
					}
				}
				else if( STR_TARGET == kStrParamFrom )
				{// 타겟
					kAction.CreateActionTargetList(kActor, true, true);
					lwActionTargetList kTargetList = kAction.GetTargetList();
					bFindGoalPosDistAxisTarget = 0 < kTargetList.size();
					if(bFindGoalPosDistAxisTarget)
					{
						for(int i =0; i < 1/*kTargetList.size()*/; ++i)
						{
							lwActionTargetInfo kInfo =  kTargetList.GetTargetInfo(i);
							if( kInfo.IsNil() ) { break; }
							lwGUID kTargetGuid = kInfo.GetTargetGUID();
							if( kTargetGuid.IsNil() ) { break; }

							PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
							if( !pkTargetActor ) { break; }

							kDistVecDir = pkTargetActor->GetLookingDir(); //
						}
					}
					kAction.ClearTargetList();
				}
				else if( STR_USER_INPUT_DIR == kStrParamFrom )
				{// 유저 키보드 입력에 의한 방향 세팅
					if( DIR_NONE != kAction.GetDirection() )
					{
						kDistVecDir = kActor.GetDirectionVector( kAction.GetDirection() );
					}
				}
				else
				{
					return false;
				}
				SetDistVecDir(kAction, kDistVecDir);	// 목적 위치에서 이격 거리의 방향
			}

			{
				//이동시 바라보는 방향을 설정
				vStr = STR_WHEN_MOVE_DIRECTION; vStr+=iCnt;
				InitDirection( kActor, kAction, STR_WHEN_START_DIRECTION.c_str() );
			}

			NiPoint3 kRayPos( kFrom() );
			kRayPos.z += 20.0f;
			NiPoint3 kRayDir(-NiPoint3::UNIT_Z);
			NiPoint3 const kHitPos = g_pkWorld->ThrowRay( kRayPos, kRayDir, 150.0f, 1); // 일반적인 벽
			if( -1.0f != kHitPos.x || -1.0f != kHitPos.y || -1.0f != kHitPos.z )
			{// 이동할수 있는 위치라면
				kFrom = kHitPos;
			}
			
			lwPoint3 kRotateRad(0.0f, 0.0f, 0.0f);
			if(bFindGoalPosTarget
				 && bFindGoalPosDistAxisTarget
				 )
			{// 이격 거리 방향 회전을 할수 있다면
				vStr=STR_MOVE_GOAL_POS_DIST_DIR_ROTATE_HORIZ_DEG_; vStr+=iCnt;
				float const fHorizDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
				if(0.0f != fHorizDeg)
				{
					float const fHorizRad = NxMath::degToRad(fHorizDeg);
					kDistVecDir.Rotate( lwPoint3(NiPoint3::UNIT_Z), fHorizRad );
					kRotateRad.SetY(fHorizRad);
				}		
				vStr=STR_MOVE_GOAL_POS_DIST_DIR_ROTATE_VERT_DEG_; vStr+=iCnt;
				float const fVertDeg = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
				if(0.0f != fVertDeg)
				{
					float const fVertRad = NxMath::degToRad(fVertDeg);
					lwPoint3 kRightDir = kActor.GetLookingDir();
					kRightDir.Cross( static_cast<lwPoint3>(UP_VEC) );
					kDistVecDir.Rotate( kRightDir, fVertRad );
					if( 0.0f < kDistVecDir.GetZ() )
					{// 공중으로 뜨는 액션
						SetStopJumpWhenMoving(kAction, true);
					}
					kRotateRad.SetX(fVertRad);
				}
			}
			kDistVecDir.Unitize();
			//회전값을 저장
			SetMovingDistVecRotate(kAction, kRotateRad);
		}
		
		{// 멈추는 옵션
			vStr=STR_MOVE_STOP_OPTION_; vStr+=iCnt;
			std::string const kStrParamOption = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			if( STR_MEET_FLOOR == kStrParamOption )
			{
				SetWhenMeetFloorThenStopMoving(kAction, true);
			}
		}
		

		if( STR_MOVE_TYPE_INTERPOLATION == kStrParamType )
		{// 보간 타입이라면
			SetMoveType(kAction, EMT_INTERPOLATION);
			// 얼마만큼의 거리를 이동 하는지 얻어와
			vStr=STR_MOVE_GOAL_POS_DIST_; vStr+=iCnt;
			float fDist = 0;
			// 저장해 두고
			if(bFindGoalPosTarget
				 && bFindGoalPosDistAxisTarget
				 )
			{
				fDist = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
			}
			SetMovingDistVecLength( kAction, fDist );

			lwPoint3 kDistVec = kDistVecDir;
			kDistVec.Multiply( fDist );
			lwPoint3 kGoalPos = kFrom;
			kGoalPos.Add( kDistVec );

			// 값을 얻어와 저장하고
			SetMoveStartPos(kAction, kActor.GetPos());
			SetMoveGoalPos(kAction, kGoalPos);
			SetMoveStartTime(kAction, fStartTime);

			{// 이동하는데 언제 완료되는지 계산해서
				vStr=STR_MOVE_COMPLETE_TIME_; vStr+=iCnt;
				float fRelativeCompleteTime = kAction.GetScriptParamAsFloat(vStr.operator const std::string().c_str() );

				// 슬롯에서 애니를 빠르게 했는지 확인해서 계산하고
				float const fAniSlotSpeed = GetAniSlotSpeed( kActor, kAction, kAction.GetCurrentSlot() );
				fRelativeCompleteTime /= fAniSlotSpeed;

				if( 0 < kAction.GetAbil(AT_APPLY_ATTACK_SPEED))
				{// 공속에 영향을 받는다면 완료 시간도 빨라져야 하므로
					float const fAttackSpeedRate = kActor.GetAbil(AT_C_ATTACK_SPEED) / ABILITY_RATE_VALUE_FLOAT;
					fRelativeCompleteTime /= fAttackSpeedRate;
				}
				float const fCompliteTime = fStartTime + fRelativeCompleteTime;
				// 저장하고
				SetMoveArriveTime(kAction, fCompliteTime);
			}
		}
		else if( STR_MOVE_TYPE_VELOCITY == kStrParamType )
		{
			SetMoveType(kAction, EMT_VELOCITY);
			vStr=STR_MOVE_SPEED_; vStr+=iCnt;
			float const fSpeed = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
			if( 0.0f != fSpeed )
			{
				SetMovingSpeed( kAction, fSpeed );			// 속력을 저장하고
				lwPoint3 kVelocity = kDistVecDir;
				kVelocity.Multiply( fSpeed );

				SetMovingDelta( kAction, kVelocity );
			}
		}
		else if( STR_MOVE_TYPE_SEEK == kStrParamType 
			|| STR_MOVE_TYPE_SEEK2 == kStrParamType
			)
		{	
			if( STR_MOVE_TYPE_SEEK == kStrParamType )
			{
				SetMoveType(kAction, bFindGoalPosTarget ? EMT_VELOCITY_TO_GOAL_POS : EMT_VELOCITY);
			}
			else if(STR_MOVE_TYPE_SEEK2 == kStrParamType)
			{
				SetMoveType(kAction, EMT_VELOCITY_TO_GOAL_POS );
			}

			{
				// 얼마만큼의 거리를 이동 하는지 얻어와
				vStr=STR_MOVE_GOAL_POS_DIST_; vStr+=iCnt;
				float fDist = 0;
				if(bFindGoalPosTarget
					&& bFindGoalPosDistAxisTarget
				 )
				{
					fDist = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
				}
				lwPoint3 kDistVec = kDistVecDir;

				kDistVec.Multiply( fDist );
				
				// 이격 거리를 저장해 두고
				SetMovingDistVecLength( kAction, fDist );
				
				lwPoint3 kGoalPos = kFrom;
				kGoalPos.Add( kDistVec );

				// 값을 얻어와 저장하고
				SetMoveStartPos(kAction, kActor.GetPos());
				SetMoveGoalPos(kAction, kGoalPos);
			}

			vStr=STR_MOVE_SPEED_; vStr+=iCnt;
			float const fSpeed = kAction.GetScriptParamAsFloat( vStr.operator const std::string().c_str() );
			if( 0.0f != fSpeed )
			{
				SetMovingSpeed(kAction, fSpeed);	//속력을 저장하고
				lwPoint3 kVelocity = kDistVecDir;
				kVelocity.Multiply( fSpeed );

				SetMovingDelta( kAction, kVelocity );
			}
		}
		else if(STR_MOVE_TYPE_TELEPORT == kStrParamType)
		{
			SetMoveType(kAction, EMT_TELEPORT);
		}

		{// GoalPos를 지속적으로 업데이트 할것인가
			vStr=STR_MOVE_GOAL_POS_UPDATE_; vStr+=iCnt;
			std::string const kUpdateParam = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			UseMovingGoalPosUpdate( kAction, STR_TRUE == kUpdateParam );
		}
		{// 방향을 지속적으로 업데이트 할것인가
			vStr=STR_MOVE_DIR_UPDATE_; vStr+=iCnt;
			std::string const kDirUpdateParam = kAction.GetScriptParam( vStr.operator const std::string().c_str() );
			UseMovingDirectionUpdate( kAction, STR_TRUE == kDirUpdateParam );
		}

		return true;
	}


	void SetMoveGoalPos(lwAction kAction, lwPoint3 kGoalPos)
	{// 11001 - 이동할 목표점을 저장
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(11001, kGoalPos);
	}
	lwPoint3 GetMoveGoalPos(lwAction kAction)
	{
		lwPoint3 kGoalPos(0.0f, 0.0f, 0.0f);
		if( kAction.IsNil() )	{ return kGoalPos; }

		kGoalPos = kAction.GetParamAsPoint(11001);
		bool const bUseSavedTarget = 0 < GetMaxSavedTargetCnt(kAction);
		if( true == IsMovingGoalPosUpdate(kAction) 
			|| bUseSavedTarget
		)
		{// 계속 갱신 이라면
			lwGUID kTargetGuid(NULL);
			if( bUseSavedTarget )
			{
				int const iCurIdx = GetCurUseSavedTargetIdx(kAction);
				kTargetGuid = GetSavedTargetGuid(kAction, iCurIdx);
			}
			else
			{
				kTargetGuid = GetMovingTargetGuid(kAction);
			}

			if( kTargetGuid.IsNil() ) { return kGoalPos; }						// 타겟을 얻어올수 없다면, 이전에 저장된 GoalPos를 사용
			PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
			if( !pkTargetActor ) { return kGoalPos; }
			kGoalPos = pkTargetActor->GetPos();
			
			lwPoint3 kDistVec = GetDistVecDir(kAction);//pkTargetActor->GetLookingDir(); // 이격 방향
			
			lwPoint3 kRotateRad = GetMovingDistVecRotate(kAction);				// 라디안 값
			kDistVec.Rotate( lwPoint3(NiPoint3::UNIT_Z), kRotateRad.GetX() );	// 우향 벡터 기준 회전 값 (수직 회전값)
			
			lwPoint3 kRightDir = pkTargetActor->GetLookingDir();
			kRightDir.Cross( static_cast<lwPoint3>(UP_VEC) );
			kDistVec.Rotate( kRightDir, kRotateRad.GetY() );					// 업 벡터 기준 회전 값   (수평 회전값)

			float const fDist = GetMovingDistVecLength(kAction);
			kDistVec.Multiply(fDist);

			kGoalPos.Add( kDistVec );
			SetMoveGoalPos(kAction, kGoalPos);	// 계산 완료된 새 GoalPos를 저장
		}
		return kGoalPos;
	}


	void SetMoveStartPos(lwAction kAction,lwPoint3 kStartPos)
	{// 11002 - 이동 시작 위치를 저장
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(11002, kStartPos);

	}
	lwPoint3 GetMoveStartPos(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return lwPoint3(0.0f,0.0f,0.0f); }
		return kAction.GetParamAsPoint(11002);
	}
	

	int IncMoveStartCnt(lwAction kAction)
	{// 11003 - 애니메이션 Text Key에서, 몇번째 "move_start"가 들어왔는가
		if( kAction.IsNil() )	{ return 0; }
		int const iCurCnt = GetMoveStartCnt(kAction);
		int const iNewCnt = iCurCnt+1;
		kAction.SetParamInt( 11003, iNewCnt );
		return iNewCnt;
	}
	int GetMoveStartCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt( 11003 );
	}
	
	void SetMoveStartTime(lwAction kAction, float fStartTime)
	{// 11004 - 이동이 시작되는 시간(보간 시작시간)
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat(11004, fStartTime);
	}
	float GetMoveStartTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat(11004);
	}

	void SetMoveArriveTime(lwAction kAction, float fArriveTime)
	{// 11005 - 이동이 완료될 시간(보간 완료시간)
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat(11005, fArriveTime);
	}
	float GetMoveArriveTime(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat(11005);
	}
	
	void SetStopJumpWhenMoving(lwAction kAction, bool const bStopJump)
	{// 11006 - 이동 보간중에 StopJump를 해야 하는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(11006, bStopJump ? 1 : 0);
	}
	bool IsStopJumpWhenMoving(lwAction kAction)
	{
		return (0 < kAction.GetParamInt(11006));
	}

	
	void SetWhenMeetFloorThenStopMoving(lwAction kAction, bool const bStopMoving)
	{// 11007 - 이동 보간중에 바닥을 만났을때 멈춰야 하는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(11007, bStopMoving ? 1 : 0);
	}
	bool IsStopMovingWhenMeetFloor(lwAction kAction)
	{
		return (0 < kAction.GetParamInt(11007));
	}


	void SetMoveType(lwAction kAction, int const iMoveType)
	{// 11008 - 이동시작 on/off
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt( 11008, iMoveType );
	}
	int GetMoveType(lwAction kAction)
	{
		return kAction.GetParamInt(11008);
	}
	
	//
	//bool InitMovingDelta(lwAction kAction)
	//{
	//	if( kAction.IsNil() )	{ return false; }
	//	std::string const kStrParam = kAction.GetScriptParam("MOVING_DELTA_VECTOR");
	//	if( kStrParam.empty() )
	//	{
	//		return false;
	//	}

	//	float fX(0.0f), fY(0.0f), fZ(0.0f);
	//	::sscanf_s(kStrParam.c_str(),"%f,%f,%f",&fX,&fY,&fZ);
	//	lwPoint3 kVelocity(fX, fY, fZ);
	//	SetMovingDelta(kAction, kVelocity);
	//	return true;
	//}
	void SetMovingDelta(lwAction kAction, lwPoint3 const kVelocity)
	{// 11009 - MovingDelta값 세팅
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(11009, kVelocity);
	}
	lwPoint3 GetMovingDelta(lwAction kAction)
	{
		lwPoint3 kVelocity(0.0f, 0.0f, 0.0f);
		if( kAction.IsNil() )	{ return kVelocity; }

		kVelocity = kAction.GetParamAsPoint(11009);
		bool const bUseSavedTarget = 0 < GetMaxSavedTargetCnt(kAction);

		if( true == IsMovingGoalPosUpdate(kAction) 
			|| bUseSavedTarget
			)
		{// 계속 갱신 이라면
			lwGUID kTargetGuid(NULL);
			if( bUseSavedTarget )
			{
				int const iCurIdx = GetCurUseSavedTargetIdx(kAction);
				kTargetGuid = GetSavedTargetGuid(kAction, iCurIdx);
			}
			else
			{
				kTargetGuid = GetMovingTargetGuid(kAction);
			}

			if( kTargetGuid.IsNil() ) { return kVelocity; }						// 타겟을 얻어올수 없다면, 이전에 저장된 kVelocity를 사용
			PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
			if( !pkTargetActor ) { return kVelocity; }
			
			lwPoint3 kDistVec = GetDistVecDir(kAction);//pkTargetActor->GetLookingDir(); // 속력 방향
			
			lwPoint3 kRotateRad = GetMovingDistVecRotate(kAction);				// 라디안 값
			kDistVec.Rotate( lwPoint3(NiPoint3::UNIT_Z), kRotateRad.GetX() );	// 우향 벡터 기준 회전 값 (수직 회전값)
			
			lwPoint3 kRightDir = pkTargetActor->GetLookingDir();
			kRightDir.Cross( static_cast<lwPoint3>(UP_VEC) );
			kDistVec.Rotate( kRightDir, kRotateRad.GetY() );					// 업 벡터 기준 회전 값   (수평 회전값)

			float const fSpeed = GetMovingSpeed(kAction);
			kDistVec.Multiply(fSpeed);
			
			SetMovingDelta(kAction, kDistVec);
			kVelocity = kDistVec;
		}
		return kVelocity;
	}
	
	void UseMovingGoalPosUpdate(lwAction kAction, bool const bUseUpdate)
	{// 11010 - GoalPos를 계속해서 갱신 할것인가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(11010, bUseUpdate ? 1 : 0);
		
	}
	bool IsMovingGoalPosUpdate(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return ( 0 < kAction.GetParamInt(11010) );
	}
	
	void SetMovingTargetGuid(lwAction kAction, lwGUID kTagrGetGuid)
	{// 11011 - GoalPos를 계산 하는데 사용된 대상의 TargetGuid
		if( kAction.IsNil() 
			|| kTagrGetGuid.IsNil() 
			)	
		{ 
			return;
		}
		kAction.SetParam( 11011, kTagrGetGuid.GetString() );
	}
	lwGUID GetMovingTargetGuid(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		lwGUID kTargetGuid( kAction.GetParam(11011) );
		return kTargetGuid;
	}
	
	void SetMovingDistVecRotate(lwAction kAction, lwPoint3 kDistVec)
	{// 11012 - x: 우향벡터 회전값, y: 업벡터 기준 회전값, z: LookingDir 기준 회전값
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(11012, kDistVec); // 라디안 값임
	}
	lwPoint3 GetMovingDistVecRotate(lwAction kAction)
	{
		if( kAction.IsNil() )	{ lwPoint3(0.0f, 0.0f, 0.0f); }
		return kAction.GetParamAsPoint(11012);
	}

	void SetMovingDistVecLength(lwAction kAction, float fSpeed)
	{// 11013 - (위치보간(X)/ 속도(O) )
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat(11013, fSpeed);
	}
	float GetMovingDistVecLength(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat(11013);
	}
	
	void UseMovingDirectionUpdate(lwAction kAction, bool const bUseUpdate)
	{// 11014 - 이동중 방향 업데이트를 사용하는가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(11014, bUseUpdate ? 1 : 0);
	}
	bool IsMovingDirectionUpdate(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return false; }
		return ( 0 < kAction.GetParamInt(11014) );
	}
	
	// 11015 - 목표점계산시 기준 위치에 더해줄 이격 벡터
	void SetDistVecDir(lwAction kAction, lwPoint3 kDir)
	{
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamAsPoint(11015, kDir);
	}
	lwPoint3 GetDistVecDir(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return lwPoint3(0.0f, 0.0f, 0.0f);; }
		return kAction.GetParamAsPoint(11015);
	}

	
	void SetMovingSpeed(lwAction kAction, float const fSpeed)
	{// 11016 - 목표점계산시 기준 위치에 더해줄 이격 벡터
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamFloat(11016, fSpeed);
	}
	float GetMovingSpeed(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0.0f; }
		return kAction.GetParamFloat(11016);
	}
		
	void SetMaxSavedTargetCnt(lwAction kAction, int const iMaxCnt)
	{// 11100 - 미리 타게팅해 잡아놓은 대상이 몇개인가
		if( kAction.IsNil() )	{ return; }
		kAction.SetParamInt(11100, iMaxCnt);
	}
	int GetMaxSavedTargetCnt(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(11100);
	}
	
	void SetCurUseSavedTargetIdx(lwAction kAction, int iIdx)
	{// 11101 - 미리 타게팅해 잡아놓은 대상중 몇번째 대상을 사용하고 있는가
		if( kAction.IsNil() )	{ return; }
		int const iMax = GetMaxSavedTargetCnt(kAction);
		if(iMax <= iIdx)
		{
			iIdx = iMax-1;
		}
		kAction.SetParamInt(11101, iIdx);
	}
	int GetCurUseSavedTargetIdx(lwAction kAction)
	{
		if( kAction.IsNil() )	{ return 0; }
		return kAction.GetParamInt(11101);
	}

	int const iSavedTargetGuidStartIdx = 11200;
	void SetSavedTargetGuid(lwAction kAction, int const iIdx, lwGUID kTargetGuid)
	{// 11200 ~ 11300 - 미리 타게팅해 잡아놓은 대상의 Guid를 저장
		if( kAction.IsNil() || kTargetGuid.IsNil() )	{ return; }
		kAction.SetParam( iSavedTargetGuidStartIdx+iIdx, kTargetGuid.GetString() );
	}
	lwGUID GetSavedTargetGuid(lwAction kAction, int const iIdx)
	{
		lwGUID kGuid(NULL);
		if( kAction.IsNil() )	{ return kGuid; }
		kGuid.Set( kAction.GetParam(iSavedTargetGuidStartIdx+iIdx) );
		return kGuid;
	}

	void ProcessMoving(lwActor kActor, lwAction kAction, float fAccumTime, float fFrameTime)
	{
		if( kActor.IsNil() )	{ return; }
		if( kAction.IsNil() )	{ return; }
		if(!g_pkWorld)
		{
			return;
		}

		if( IsMoving(kAction) )
		{// 이동 하는 경우, 보간을 사용하기 위해
			if( kActor.IsMeetFloor() )
			{
				if( IsStopMovingWhenMeetFloor(kAction) )
				{
					SetMoving(kActor, kAction, false);
				}
			}
			if( IsMovingDirectionUpdate(kAction) )
			{// 이동중 방향을 업데이트 하는가
				lwGUID kTargetGuid = GetMovingTargetGuid(kAction);
				if( !kTargetGuid.IsNil() )
				{
					PgActor* pkTargetActor = g_kPilotMan.FindActor( kTargetGuid() );
					if( pkTargetActor )
					{
						NiPoint3 kDir = pkTargetActor->GetPos() - kActor.GetPos()();
						kDir.Unitize();
						kActor.ConcilDirection(kDir, true);
						//kActor.SetMovingDir(kDir);
					}
				}
			}

			switch( GetMoveType(kAction) )
			{
			case EMT_INTERPOLATION:
				{
					bool bStopMoving = false;
					bool const bUseStopJump = IsStopJumpWhenMoving(kAction);
					if( bUseStopJump )
					{// stop jump가 필요하다면
						SetStopJump(kActor, kAction, true);
					}

					lwPoint3 kStartPoint = GetMoveStartPos(kAction);		// 시작 위치와
					// 완료 위치
					lwPoint3 kTargetPos = GetMoveGoalPos(kAction);

					// 보간에 사용되는 t값에 해당하는, 시간 값을 계산 하고
					float const fStartTime = GetMoveStartTime(kAction);
					float const fCompleteTime = GetMoveArriveTime(kAction);

					float const fLinearTotalTime = fCompleteTime - fStartTime;	// 보간 전체 시간중
					float const fCurTime = lwGetAccumTime() - fStartTime;		// 보간 현재 시간을 구해
					float fLinearPer = fCurTime / fLinearTotalTime;				// 얼마만큼 진행되었는지 %로 표현하고

					NiPoint3 kNiStartPos(kStartPoint.GetX(), kStartPoint.GetY(), kStartPoint.GetZ());
					NiPoint3 kNiGoalPos(kTargetPos.GetX(), kTargetPos.GetY(), kTargetPos.GetZ());
					// 선형보간 함수를 통해 새 위치를 얻어와 설정한다
					NiPoint3 const kNiNewPos = PgInterpolator::Lerp(kNiStartPos, kNiGoalPos, (1.0f<fLinearPer ? 1.0f : fLinearPer) );
					lwPoint3 kNewPos(kNiNewPos.x, kNiNewPos.y, kNiNewPos.z);

					if( false == bUseStopJump )
					{// 땅으로의 위치보간일 경우만
						NiPoint3 kRayPos( kNewPos() );
						kRayPos.z += 20.0f;
						NiPoint3 kRayDir(-NiPoint3::UNIT_Z);
						NiPoint3 const kHitPos = g_pkWorld->ThrowRay( kRayPos, kRayDir, 75.0f, 1); // 일반적인 벽
						if( -1.0f == kHitPos.x && -1.0f == kHitPos.y && -1.0f == kHitPos.z )
						{// 이동할수 없는 위치를 체크해서 보간을 멈출지 체크하고
							bStopMoving = true;
						}
					}

					if(1.0f <= fLinearPer)
					{// 100%를 넘었다면, 
						bStopMoving = true;
						//kActor.SetTranslate( kNewPos, false );
					}
					else
					{
						lwPoint3 kCurPos = kActor.GetPos();
						lwPoint3 kDeltaVec = kNewPos.Subtract2(kCurPos);
						kDeltaVec.Multiply(1.0f/fFrameTime);
						kActor.SetMovingDelta(kDeltaVec);
					}

					if(bStopMoving)
					{//Moving을 멈춘다
						SetMoving(kActor,kAction, false);
					}
				}break;

			case EMT_VELOCITY:
				{
					lwPoint3 const kVelocity = GetMovingDelta(kAction);
					if( !kVelocity.IsZero() )
					{
						kActor.SetMovingDelta(kVelocity);
					}
				}break;
			case EMT_VELOCITY_TO_GOAL_POS2:
			case EMT_VELOCITY_TO_GOAL_POS:
				{
					lwPoint3 kTargetPos = GetMoveGoalPos(kAction);
					lwPoint3 kVelocity = kTargetPos.Subtract2( kActor.GetPos() );
					float const fDist = kVelocity.Length();
					float fSpeed = GetMovingSpeed(kAction);
					float const fMinRate = fSpeed*0.02f;
					if(fDist <= fMinRate)	// 아오 이런 질낮은 판단문
					{//남은 이동 거리가 속력의 2% 이하면 위치를 자동 세팅해주고
						//if( 0 == kAction.GetScriptParamAsInt("MOVE_CONTINUE") )
						{
							SetMoving(kActor, kAction, false);
							kActor.SetTranslate(kTargetPos, false);	// 간극을 완벽히 매꿀수 없기에 위치세팅(보장된 시간에 update되지 않으므로)
						}
					}
					else
					{// 충분히 이동 할수 있다면
						kVelocity.Unitize();
						
						if( fSpeed > fDist )
						{// 스피드가 더 빠르면 도착하기에 알맞은
							fSpeed -= fDist;
						}
						kVelocity.Multiply( fSpeed );

						if( !kVelocity.IsZero() )
						{
							kActor.SetMovingDelta(kVelocity);
						}
					}
				}break;
			case EMT_TELEPORT:
				{
					SetMoving(kActor, kAction, false);
					lwPoint3 kTargetPos = GetMoveGoalPos(kAction);
					if(!kTargetPos.IsZero() )
					{
						float const fMaxSpeed = static_cast<float>( kAction.GetAbil(AT_SKILL_MAXIMUM_SPEED) );
						lwPoint3 kVelocity = kTargetPos.Subtract2( kActor.GetPos() );
						if( fMaxSpeed < kVelocity.Length() )
						{
							kVelocity.Unitize();
							kVelocity.Multiply( fMaxSpeed );
						}
						//float const fDist = kVelocity.Length();
						kVelocity.Multiply(1.0f/fFrameTime);
						kActor.SetMovingDelta(kVelocity);
						//kActor.SetTranslate(kTargetPos, false);
					}
				}break;
			}
		}
	}
}