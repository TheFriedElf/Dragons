#include "stdafx.h"
#include "Variant/Global.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgWorld.h"
#include "PgCameraMan.h"
#include "PgObserverMode.h"
#include "PgUIScene.h"


PgObserverMode::PgObserverMode()
:	m_kMode(OBMODE_NONE)
,	m_dwDelay(0)
{
}

PgObserverMode::~PgObserverMode()
{
	
}

bool FObserverModeInputHook( int const iKey, XUI::E_INPUT_EVENT_INDEX type )
{
	switch ( iKey )
	{
	case NiInputKeyboard::KEY_SPACE:
		{
			if ( XUI::IEI_KEY_UP == type )
			{
				g_kOBMode.ToggleTarget();
			}
			return true;
		}break;
	default:
		{

		}break;
	}

	return false;
}

void PgObserverMode::Start( BM::GUID const &kTargetID )
{
	if ( OBMODE_NONE == m_kMode )
	{
		g_kUIScene.InputHook(FObserverModeInputHook);
		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_ObserverMove, true));
		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_ObserverMove, false));
		
		if ( g_pkWorld )
		{
			// 화면을 약간만 어둡게 해주자
			g_pkWorld->SetShowWorldFocusFilter( static_cast<DWORD>(0x00), 0.0f, 0.3f, 1.0f, false, false );
		}
	}

	if ( m_kNowTargetChar != kTargetID )
	{
		m_dwDelay = BM::GetTime32();
		m_kMode = OBMODE_SEARCHING;
		m_kNowTargetChar = kTargetID;
		m_kContTargetChar.insert( m_kNowTargetChar );
	}
}

void PgObserverMode::Failed( BM::GUID const &kTargetID )
{
	m_kContTargetChar.erase( kTargetID );
	m_dwDelay -= 1000;
	ToggleTarget();
}

void PgObserverMode::End()
{
	if ( OBMODE_NONE != m_kMode )
	{
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_ObserverMove, true));
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_ObserverMove, false));
		g_kUIScene.InputHook(NULL);

		m_kContTargetChar.clear();
		m_kNowTargetChar.Clear();

		if ( g_pkWorld )
		{
			PgActor *pkActor = g_kPilotMan.PlayerActor();
			if ( pkActor )
			{
				g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
				g_pkWorld->SetShowWorldFocusFilter( false, NULL, 0.0f );
				m_kMode = OBMODE_NONE;
				return;
			}
			else
			{
				PG_ASSERT_LOG(pkActor);
			}
		}

		// Player Actor을 몾찾으면 어떻게 되는 거지....

		PG_ASSERT_LOG(g_pkWorld);
		m_kMode = OBMODE_ENDING;
		m_dwDelay = 0;
	}
}

void PgObserverMode::Update( float const fAccumTime, float const fFrameTime )
{
	switch ( m_kMode )
	{
	case OBMODE_SEARCHING:
		{
			if ( g_pkWorld )
			{
				PgActor *pkActor = g_kPilotMan.FindActor( m_kNowTargetChar );
				if ( pkActor )
				{
					g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
					m_kMode = OBMODE_FOLLOWING;
				}
			}
		}break;
	case OBMODE_FOLLOWING:
		{

		}break;
	case OBMODE_ENDING:
		{
			if ( g_pkWorld )
			{
				PgActor *pkActor = g_kPilotMan.PlayerActor();
				if ( pkActor 
					&& g_pkWorld
					)
				{
					g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
					g_pkWorld->SetShowWorldFocusFilter( false, NULL, 0.0f );
					m_kMode = OBMODE_NONE;
				}
				else
				{
					PG_ASSERT_LOG(pkActor);
				}
			}
		}break;
	default:
		{

		}break;
	}
}

void PgObserverMode::AddTarget( BM::GUID const &kCharGuid )
{
	m_kContTargetChar.insert( kCharGuid );
}

void PgObserverMode::RemoveTarget( BM::GUID const &kCharGuid )
{
	if ( m_kNowTargetChar == kCharGuid )
	{
		if ( m_kContTargetChar.size() > 1 )
		{
			m_dwDelay = 0;
			ToggleTarget();
		}
	}
	m_kContTargetChar.erase( kCharGuid );
}

void PgObserverMode::SetTarget( BM::GUID const &kCharGuid )
{
	if ( m_kNowTargetChar != kCharGuid )
	{
		DWORD const dwCurTime = BM::GetTime32();
		if ( ::DifftimeGetTime( m_dwDelay, dwCurTime ) > 999 )
		{
			BM::Stream kPacket( PT_C_M_REQ_OBMODE_TARGET_CHANGE, kCharGuid );
			NETWORK_SEND( kPacket );

			m_dwDelay = dwCurTime;
		}
	}
}

bool PgObserverMode::ToggleTarget()
{
	if ( m_kContTargetChar.empty() )
	{
		return false;
	}

	SET_GUID::const_iterator itr = m_kContTargetChar.find(m_kNowTargetChar);
	if ( itr != m_kContTargetChar.end() )
	{
		++itr;
	}
	
	if ( itr == m_kContTargetChar.end() )
	{
		itr = m_kContTargetChar.begin();
	}

	SetTarget( *itr );
	return true;
}
