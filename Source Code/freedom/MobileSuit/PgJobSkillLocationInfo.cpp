#include "stdafx.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "Variant/PgStringUtil.h"
#include "PgJobSkillLocationInfo.h"
#include "PgPilotMan.h"
#include "PgAction.h"
#include "lwAction.h"
#include "PgTrigger.h"
#include "PgParticleMan.h"
#include "PgUISound.h"
#include "lwUI.h"

std::string const STR_IDLE_ACTION_NAME(ACTIONNAME_IDLE);
std::string const STR_JOB_SKILL_END("end");
std::string const STR_JOB_SKILL_STOP_LOOP("STOP_LOOP");
std::string const STR_JOB_SKILL_SUCCESS("SUCCESS");
std::wstring const WSTR_JOB_SKILL_SUCCESS_SOUND_ID(L"job-success");
std::wstring const WSTR_JOB_SKILL_FAIL_SOUND_ID(L"job-fail");

std::string const STR_JOBSKILL_TRIGGER_LOOKAT_PARAM_ID("LOOK_AT");
std::string const STR_JOBSKILL_TRIGGER_NOT_EFFECT_PARAM_ID("NOT_EFFECT");

PgJobSkillLocationInfo::PgJobSkillLocationInfo()
{
}
PgJobSkillLocationInfo::~PgJobSkillLocationInfo()
{
}

void PgJobSkillLocationInfo::ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket)
{
	switch( wPacketType )
	{
	case PT_M_C_NFY_LOCATION_ITEM:
		{
			ReadFromPacket(rkPacket);
			RefrashAllSpotRenderObj();
		}break;
	case PT_M_C_NFY_LOCATION_INFO:
		{
			CONT_JOBSKILL_LOCATIONINFO::key_type kTriggerID;
			CONT_JOBSKILL_LOCATIONINFO::mapped_type kElement;

			// ID에 맞는 정보를 갱신 해주자.
			rkPacket.Pop(kTriggerID);
			kElement.ReadFromPacket(rkPacket);			

			if( m_kLocationItem.Set(kTriggerID, kElement) )
			{
				RefrashSpotParticle(kTriggerID, kElement);
			}
			else
			{// 갱신 안됨
			}
		}break;
	case PT_M_C_NFY_LOCATION_ACTION:
		{
			bool bResult = false;

			rkPacket.Pop( bResult );
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if(pkActor)
			{
				PgAction* pkAction = pkActor->GetAction();
				if(pkAction)
				{
					if(bResult)
					{// 채집 시작하기 위해
						int iJobSkillNo = 0;
						rkPacket.Pop( iJobSkillNo );	// 사용할 스킬 번호 얻어오고

						{// 바라 봐야할 위치를 얻어와 바라보게 한 후,
							
							std::string kTriggerID;
							rkPacket.Pop( kTriggerID );
							SetActorLookAtByJobSkillTrigger(kTriggerID, pkActor);
						}

						DWORD dwRunTurnTime = 0;
						{
							rkPacket.Pop( dwRunTurnTime );	// 현재 시간으로 부터 채집 진행시간(총 시간)
						}

						if(0 < iJobSkillNo)
						{// 사용할 스킬을 지정하고,
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pkJobSkillDef = kSkillDefMgr.GetDef(iJobSkillNo);
							BM::vstring kActionName(pkJobSkillDef->GetActionName());
							{
								lwAction klwAction(pkAction);
								klwAction.SetParamInt( 20110817, static_cast<int>(dwRunTurnTime) );	// 애니 속도 조절을 위해 채집 진행되는 시간을 저장
								klwAction.ChangeToNextActionOnNextUpdate(true,true);
							}
							pkActor->TransitAction(static_cast<std::string const>(kActionName).c_str());
							//프로그래스 UI 호출해주자
							CallJobSkillGatherTimer(iJobSkillNo, dwRunTurnTime);
							int const iMainJobSkillNo = pkJobSkillDef->GetAbil(AT_CUSTOMDATA1);
							if(0 < iMainJobSkillNo)
							{// 토글표시를 실행시켜준다
								lwSetJobSkillTogleState(iMainJobSkillNo, true);
							}
						}
					}
					else
					{// 채집 취소
						if(pkAction->GetActionType() == ACTIONTYPE_JOBSKILL)
						{
							pkAction->SetParam(1, STR_JOB_SKILL_END.c_str()); // 현재 채집 스킬이 진행중이라면 종료할수 있게 param값을 설정해주고
							pkAction->SetParam(20200, STR_JOB_SKILL_STOP_LOOP.c_str()); // 더이상 반복되지 않게 한다.
							int const iMainJobSkillNo = pkAction->GetAbil(AT_CUSTOMDATA1);
							if(0 < iMainJobSkillNo)
							{// 토글표시를 꺼준다
								lwSetJobSkillTogleState(iMainJobSkillNo, false);
							}
						}
					}
				}
			}
		}break;
	case PT_M_C_NFY_COMPLETE_JOBSKILL:
		{	//프로그래스 UI 닫아주자 (루아에서도 해줘야 한다. - 유저 캔슬 할 때)
			lua_tinker::call<void>("CloseJobSkillGatherTimer");

			bool bEquippedToolItemDestroy = false;
			rkPacket.Pop(bEquippedToolItemDestroy);
			bool bJobSkillSuccess = false;
			rkPacket.Pop(bJobSkillSuccess);
			 
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if(pkActor)
			{
				PgAction* pkAction = pkActor->GetAction();
				if(pkAction
					&& pkAction->GetActionType() == ACTIONTYPE_JOBSKILL
					)
				{
					pkAction->SetParam(1, STR_JOB_SKILL_END.c_str()); // 현재 채집 스킬이 진행중이라면 종료할수 있게 param값을 설정해준다.
					if(bJobSkillSuccess)
					{// 성공 아이템을 얻었다면,
						pkAction->SetParam(20010, STR_JOB_SKILL_SUCCESS.c_str());
						g_kUISound.PlaySoundByID(WSTR_JOB_SKILL_SUCCESS_SOUND_ID.c_str());
					}
					else
					{
						g_kUISound.PlaySoundByID(WSTR_JOB_SKILL_FAIL_SOUND_ID.c_str());
					}

					if(bEquippedToolItemDestroy)
					{// 아이템이 파괴되었다면, 더이상 반복되지 않게 한다.
						pkAction->SetParam(20200, STR_JOB_SKILL_STOP_LOOP.c_str());
						int const iMainJobSkillNo = pkAction->GetAbil(AT_CUSTOMDATA1);
						if(0 < iMainJobSkillNo)
						{// 토글표시를 꺼준다
							lwSetJobSkillTogleState(iMainJobSkillNo, false);
						}
						PgAction* pkAction = pkActor->GetAction();
						if(pkAction)
						{
							lwAction klwAction(pkAction);
							klwAction.ChangeToNextActionOnNextUpdate(true,true);
						}
						pkActor->TransitAction(STR_IDLE_ACTION_NAME.c_str());
					}
				}
			}
		}break;
	default:
		{
		}break;
	}
}

void PgJobSkillLocationInfo::ReadFromPacket(BM::Stream & kPacket)
{
	m_kLocationItem.ReadFromPacket(kPacket);
}

bool IsShowParticle(PgTrigger * pkTrigger)
{
	if(!pkTrigger)
	{
		return false;
	}

	std::string kParamStr;
	char const* pkParamStr = pkTrigger->GetParamFromParamMap(STR_JOBSKILL_TRIGGER_NOT_EFFECT_PARAM_ID.c_str());
	if(pkParamStr && (0 == strcmp("TRUE", pkParamStr)) )
	{
		return false;
	}

	return true;
}

void PgJobSkillLocationInfo::RefrashSpotParticle(std::string const& kTriggerID, SJobSkill_LocationItem& kInfo)
{
	if(!g_pkWorld)
	{
		return;
	}

	if(kInfo.bSpot)
	{// Spot지역의 
		PgTrigger *pkTrigger = g_pkWorld->GetTriggerByIDWithIgnoreCase(kTriggerID.c_str());
		if( pkTrigger && IsShowParticle(pkTrigger) )
		{// 트리거가
			if(kInfo.bEnable)
			{// 사용 가능하다면
				DetachSpotParticle(pkTrigger);
				AttachSpotParticle(pkTrigger, "eff_spot_on");
			}
			else
			{// 사용 불가하다면
				DetachSpotParticle(pkTrigger);
				AttachSpotParticle(pkTrigger, "eff_spot_off");
			}
		}
	}
}


void PgJobSkillLocationInfo::RefrashAllSpotRenderObj()
{// 
	CONT_JOBSKILL_LOCATIONINFO kLocationInfo;
	m_kLocationItem.GetAll(kLocationInfo);

	CONT_JOBSKILL_LOCATIONINFO::iterator kInfoIter = kLocationInfo.begin();
	while(kLocationInfo.end() != kInfoIter)
	{// 모든
		RefrashSpotParticle(kInfoIter->first, kInfoIter->second);
		++kInfoIter;
	}
}

void PgJobSkillLocationInfo::AttachSpotParticle(PgTrigger *pkTrigger, std::string const kParticleName)
{
	if(pkTrigger
		&& g_pkWorld
		)
	{
		// 위치를 얻어와
		NiPoint3 const& kPos = pkTrigger->GetTriggerObjectPos();
		// 월드 맵에 붙여주고, 파티클을 제거할때를 위해 Slot번호를 받아두고
		PgParticle* pkParticle = g_kParticleMan.GetParticle(kParticleName.c_str(), PgParticle::O_SCALE, 1);
		int const iSlot = g_pkWorld->AttachParticle(pkParticle, kPos);
		// Slot 관리 컨테이너에 삽입하는데
		std::pair<CONT_SPOT_PARTICLE_SLOT::iterator, bool> kRet = m_kContSpotSlot.insert(std::make_pair(pkTrigger->GetID(), iSlot));
		if(!kRet.second)
		{// 이미 존재한다면, 슬롯 번호를 갱신해주고
			if(iSlot != kRet.first->second)
			{// 기존에 있던 파티클을 제거해준다
				kRet.first->second = iSlot;
				g_pkWorld->DetachParticle(iSlot);
			}
		}
	}
}

void PgJobSkillLocationInfo::DetachSpotParticle(PgTrigger *pkTrigger)
{
	if(pkTrigger
		&& g_pkWorld
		)
	{
		CONT_SPOT_PARTICLE_SLOT::iterator kItor = m_kContSpotSlot.find(pkTrigger->GetID());
		if(m_kContSpotSlot.end() != kItor)
		{// 파티클이 존재하는 Slot을 찾아
			int const iSlot = kItor->second;
			// 제거 해주고
			g_pkWorld->DetachParticle(iSlot);
			// 활성화가 없기 때문에, 컨테이너에서도 삭제해준다
			m_kContSpotSlot.erase(kItor);
		}
	}
}

void PgJobSkillLocationInfo::Clear()
{
	m_kContSpotSlot.clear();
}

bool PgJobSkillLocationInfo::GetGatherType(std::string const& kTriggerID, EGatherType& eOutValue) const
{
	return m_kLocationItem.GetGatherType(kTriggerID, eOutValue);
}

bool PgJobSkillLocationInfo::IsEnable(std::string const& kTriggerID) const
{
	return m_kLocationItem.IsEnable(kTriggerID);
}

void PgJobSkillLocationInfo::SetActorLookAtByJobSkillTrigger(std::string const& kTriggerID, PgActor* pkActor)
{
	if(!pkActor
		|| kTriggerID.empty()
		|| !g_pkWorld
		)
	{
		return;
	}
	PgTrigger* pkTrigger = g_pkWorld->GetTriggerByID(kTriggerID);
	if(!pkTrigger)
	{// 사용하는 트리거를 얻어온 후
		return;
	}
	std::string kParamStr;
	{// 바라봐야할(LookAt) param값을 얻어와
		char const* pkParamStr = pkTrigger->GetParamFromParamMap(STR_JOBSKILL_TRIGGER_LOOKAT_PARAM_ID.c_str());
		if(!pkParamStr)
		{// 없다면 종료하고
			return;
		}
		kParamStr = pkParamStr;
		if(kParamStr.empty())
		{// 문자가 비었다면 종료하고
			return;
		}
	}
	NiPoint3 kLookingPos(0.0f, 0.0f, 0.0f);

	// 공백을 제거하고
	PgStringUtil::TrimAll<std::string>(kParamStr, " ", kParamStr);
	// LookAt 타입을 얻어온후 
	std::string kLookAtType(kParamStr, 0, kParamStr.find(","));
	int const iLookAtType = PgStringUtil::SafeAtoi(kLookAtType);

	switch(iLookAtType)
	{// LookAt 타입에 따라
	case EJSTLT_LOOKAT_TRIGGER_POS:
		{// 트리거를 바라보거나,
			kLookingPos = pkTrigger->GetTriggerObjectPos();
		}break;
	case EJSTLT_LOOKAT_TRIGGER_ID:
		{// 다른 트리거 아이디를 바라본다면
			std::string const kLookAtTriggerID = kParamStr.erase(0, kParamStr.find(",")+1);
			if(kLookAtTriggerID.empty()
				|| !g_pkWorld
				)
			{
				return;
			}
			PgTrigger* pkLookAtTrigger = g_pkWorld->GetTriggerByID(kLookAtTriggerID);
			if(!pkLookAtTrigger)
			{// 사용하는 트리거를 얻어온 후
				return;
			}
			// 바라볼 위치를 얻어와
			kLookingPos = pkLookAtTrigger->GetTriggerObjectPos();
		}break;
	case EJSTLT_LOOKAT_CAMERA:
		{
			std::string::size_type const stLength =  kParamStr.size();
			kParamStr.erase(0, kParamStr.find(",")+1);
			Direction eLookAtDir = (stLength == kParamStr.size()) ? DIR_DOWN : DIR_UP;	// 카메라쪽, 카메라 반대쪽
			pkActor->LockBidirection(false);
			pkActor->SetLookingDirection(eLookAtDir, true);
			return;
		}break;
	default:
		{// 바라볼곳이 없음
			return;
		}break;
	}
	// 방향을 지정하기 전에, 8방향 볼수 있게 설정하고(직업스킬 액션에서 제거해줌)
	pkActor->LockBidirection(false);
	pkActor->LookAt(kLookingPos);
}