#include "stdafx.h"
#include "PgEventScriptSystem.H"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgCameraModeEvent.h"
#include "PgNetwork.h"
#include "PgEventTimer.h"
#include "PgQuestMan.h"
#include "PgOption.h"

PgEventScriptSystem	g_kEventScriptSystem;

void PgEventScriptSystemFunction::SendActivateEventScript(int const iEventID)
{
	//! 이벤트 중에는 HP Display Option을 끈다.
	g_kGlobalOption.OffDisplayHP();
	lua_tinker::call<void, int>("ActivateEventScript", iEventID);
	
	BM::Stream kPacket(PT_C_M_REQ_RUN_EVENT_SCRIPT, iEventID);
	NETWORK_SEND(kPacket) // 이벤트가 시작 되었다고 서버로 알림
}

void PgEventScriptSystemFunction::SendDeActivateEventScript(int const iEventID)
{
	//! 이벤트 종료되었으므로, HP 보이기 옵션 복구한다
	g_kGlobalOption.RestoreDisplayHP();
	lua_tinker::call<void, int>("DeActivateEventScript", iEventID);

	BM::Stream kPacket(PT_C_M_REQ_END_EVENT_SCRIPT, iEventID);
	NETWORK_SEND(kPacket) // 이벤트가 종료 되었다고 서버로 알림
}

void	PgEventScriptSystem::Init()
{
	SetNowUpdate(false);
}


void	PgEventScriptSystem::Terminate()
{
	SetNowUpdate(false);
	ReleaseAll();
}


void	PgEventScriptSystem::Update(float fAccumTime,float fFrameTime)
{
	SetNowUpdate(true);

	//	추가 예약 리스트 처리
	if( m_kProcessing.empty()
	&&	!m_kAddToProcessingReserved.empty() // 한번에 하나의 이벤트 스크립트만 동작 가능
	&&	!g_kQuestMan.IsQuestDialog() ) // 퀘스트 대화도중이 아닐 때에만
	{
		EventScriptInstanceCont::value_type pkEventScript = m_kAddToProcessingReserved.front();
		if( pkEventScript )
		{
			m_kProcessing.push_back( pkEventScript );
			PgEventScriptSystemFunction::SendActivateEventScript(pkEventScript->GetEventID()); //서버에 알림
			m_kAddToProcessingReserved.pop_front();
		}
	}

	// 삭제, 실행 처리
	EventScriptInstanceCont::iterator itor = m_kProcessing.begin();
	while( m_kProcessing.end() != itor )
	{
		PgEventScriptInstance *pkInstance = *itor;
		bool bEraseEvent = m_kDeleteFromProcessingReserved.end() != m_kDeleteFromProcessingReserved.find(pkInstance->GetEventID());
		if( !bEraseEvent )
		{
			bEraseEvent = pkInstance->Update(fAccumTime,fFrameTime);
		}

		if( bEraseEvent )
		{
			PgEventScriptSystemFunction::SendDeActivateEventScript(pkInstance->GetEventID()); //서버에 알림

			PgEventScriptSystemFunction::SafeDelete(pkInstance);
			itor = m_kProcessing.erase(itor);
			continue;
		}

		++itor;
	}

	SetNowUpdate(false);
}


void	PgEventScriptSystem::ReserveToDelete(PgEventScriptInstance *pkInstance)
{
	if( !pkInstance )
	{
		return;
	}
	int const &iEventID = pkInstance->GetEventID();
	m_kDeleteFromProcessingReserved.insert(iEventID);
}


bool	PgEventScriptSystem::DeactivateAll()
{
	//	추가 예약 리스트에 있다면, 추가 예약 리스트에서 제거한다.
	std::for_each(m_kAddToProcessingReserved.begin(), m_kAddToProcessingReserved.end(), PgEventScriptSystemFunction::SafeDelete);
	m_kAddToProcessingReserved.clear();

	//	실행중인 스크립트 리스트에서 찾는다.
	std::for_each(m_kProcessing.begin(), m_kProcessing.end(), SActionReserveToDelete(this));
	return	true;
}


bool	PgEventScriptSystem::DeactivateEvent(int iEventID)
{
	if( !IsNowActivated(iEventID) )
	{
		return false;
	}

	//	추가 예약 리스트에 있다면, 추가 예약 리스트에서 제거한다.
	m_kAddToProcessingReserved.remove_if(SActionOperatorEqualAndDelete(iEventID));

	//	실행중인 스크립트 리스트에서 찾는다.
	std::for_each(m_kProcessing.begin(), m_kProcessing.end(), SActionReserveToDeleteEventID(this, iEventID));

	return true;
}


bool PgEventScriptSystem::ActivateEvent(int iEventID, float fDelayTime)
{
	if( 0.f >= fDelayTime )
	{
		_PgMessageBox("Error", "[%s] Can't Event Script DelayTime 0.f", __FUNCTION__);
		return false;
	}

	m_kDelayActivateEvent.insert(iEventID);

	char szEventScript[MAX_PATH] = {0, };
	sprintf_s(szEventScript, MAX_PATH, "GetEventScriptSystem():ActivateEvent(%d)", iEventID);
	STimerEvent kTimerEvent;
	kTimerEvent.Set(fDelayTime, szEventScript);
	g_kEventTimer.AddLocal("EVENT_END_BEGIN_EVENT_SCRIPT", kTimerEvent);
	return true;
}


bool	PgEventScriptSystem::ActivateEvent(int iEventID)
{
	if(iEventID<0)
	{
		return	false;
	}

	//	이미 실행중인 이벤트를 다시 실행하는것은 안된다.
	if( IsNowActivated(iEventID) )
	{
		_PgMessageBox("ActivateEvent Failed.","Event %d has been already activated.",iEventID);
		return	false;
	}

	// 이벤트 실행시 캐릭터를 기본 상태로 만듬
	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if( pkActor )
	{
		//pkActor->ReserveTransitAction("a_lock_move");
		//pkActor->ClearActionState();
		//pkActor->SetDirection(DIR_NONE);
		/*
		if( g_pkWorld )
		{	
			g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
			PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(g_pkWorld->m_kCameraMan.GetCameraMode());
			if( pkCameraMode )
			{
				pkCameraMode->SetCameraFixed( true );
			}
		}*/
	}

	//	삭제 예약 리스트에 있다면, 삭제 예약 리스트에서 제거한다.
	m_kDeleteFromProcessingReserved.erase(iEventID);
	m_kDelayActivateEvent.erase(iEventID);

	//	객체 생성
	PgEventScriptInstance	*pkInstance = PgEventScriptInstance::Create(iEventID);
	if(!pkInstance)
	{
		return false;
	}

	m_kAddToProcessingReserved.push_back(pkInstance);
	return true;
}


bool	PgEventScriptSystem::IsNowActivate()
{
	return !m_kProcessing.empty() || !m_kAddToProcessingReserved.empty() || !m_kDelayActivateEvent.empty();
}


bool	PgEventScriptSystem::IsNowActivated(int iEventID)
{
	//	삭제 예약 리스트에 있으면, Deactivated 상태이다.
	if( m_kDeleteFromProcessingReserved.end() != m_kDeleteFromProcessingReserved.find(iEventID) )
	{
		return false;
	}

	//	추가 예약 리스트나, 현재 실행중인 리스트에 있으면 Activated 상태이다.
	if( m_kProcessing.end() != std::find_if(m_kProcessing.begin(), m_kProcessing.end(), SActionOperatorEqual(iEventID)) )
	{
		return true;
	}
	if( m_kAddToProcessingReserved.end() != std::find_if(m_kAddToProcessingReserved.begin(), m_kAddToProcessingReserved.end(), SActionOperatorEqual(iEventID)) )
	{
		return true;
	}
	return	false;
}


void	PgEventScriptSystem::ReleaseAll()
{
	std::for_each(m_kAddToProcessingReserved.begin(), m_kAddToProcessingReserved.end(), PgEventScriptSystemFunction::SafeDelete);
	std::for_each(m_kProcessing.begin(), m_kProcessing.end(), PgEventScriptSystemFunction::SafeDelete);
	m_kProcessing.clear();
	m_kAddToProcessingReserved.clear();
	m_kDeleteFromProcessingReserved.clear();
	m_kDelayActivateEvent.clear();
}
