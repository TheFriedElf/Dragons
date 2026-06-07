#include "StdAfx.h"
#include "XUI/xui_font.h"
#include "Lohengrin/VariableContainer.h"
#include "Lohengrin/PgPlayLimiter.h"
#include "Collins/Log.h"
#include "Variant/PgLogUtil.h"
#include "Variant/Global.h"
#include "Variant/Emoticon.h"
#include "Variant/PgPvPRanking.h"
#include "constant.h"
#include "PgTask_Contents.h"
#include "PgPvPLobby_League.h"
//#include <chrono>

// PgPvPLobby_League_League
PgPvPLobby_League::PgPvPLobby_League( int const iLobbyID )
:	PgPvPLobby_Base( iLobbyID )
,	m_iBalanceCellIndex(0)
,	m_iBalanceCellUserSize(0)
,	m_dwLastEventTime(0)
,	m_dwOldCheckTime(0)
{
	m_kContRoom.reserve( MAX_PVP_ROOM_COUNT );
	int iIndex = 0;
	while ( iIndex < MAX_PVP_ROOM_COUNT )
	{
		PgPvPRoom *pkRoom = new_tr PgPvPRoom( iIndex, GetID() );
		if ( pkRoom )
		{
			m_kContRoom.push_back( pkRoom );
			m_kContCloseRoom.insert( std::make_pair( iIndex, pkRoom ) );
			++iIndex;
		}
	}

	for ( int n=0; n<MAX_CELL_SIZE; ++n )
	{
		m_kCellArray[n].SetIndex(n);
	}

	m_kContBattle.clear();
	m_kContTeamToBattle.clear();
	m_kPacketLeagueInfo.Clear();
}

PgPvPLobby_League::~PgPvPLobby_League()
{
	CONT_ALL_ROOM::iterator room_itr = m_kContRoom.begin();
	for ( ; room_itr!=m_kContRoom.end() ; ++room_itr )
	{
		SAFE_DELETE(*room_itr);
	}
	m_kContRoom.clear();

	CONT_USER::iterator user_itr = m_kContUser.begin();
	for ( ; user_itr != m_kContUser.end() ; ++user_itr )
	{
		SAFE_DELETE(user_itr->second);
	}
	m_kContUser.clear();

	m_kContBattle.clear();
	m_kContTeamToBattle.clear();
	m_kPacketLeagueInfo.Clear();
}

bool PgPvPLobby_League::ProcessMsg( BM::Stream::DEF_STREAM_TYPE const kType, SEventMessage * pkMsg )
{
	switch ( kType )
	{
	case PT_A_NFY_USER_DISCONNECT:
		{
			BM::GUID kMemberGuid;
			BM::GUID kCharacterGuid;
			pkMsg->Pop(kMemberGuid);
			pkMsg->Pop(kCharacterGuid);
			Locked_Exit( kCharacterGuid, true );
		}break;
	case PT_T_T_REQ_JOIN_LOBBY:
		{
			UNIT_PTR_ARRAY kUnitArray;
			kUnitArray.ReadFromPacket( *pkMsg );

			UNIT_PTR_ARRAY::iterator unit_itr = kUnitArray.begin();
			for ( ; unit_itr != kUnitArray.end() ; ++unit_itr )
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(unit_itr->pkUnit);
				if ( pkPlayer )
				{
					if ( true == Locked_Join( pkPlayer ) )
					{
						unit_itr->bAutoRemove = false;
					}
					else
					{
						if ( PvP_Lobby_GroundNo_AnterRoom != GetID() )
						{
							BM::Stream kContents( PT_T_T_REQ_JOIN_LOBBY );
							pkPlayer->WriteToPacket( kContents, WT_DEFAULT );
							SendToPvPLobby( kContents, PvP_Lobby_GroundNo_AnterRoom );
						}
					}
				}
				else
				{
					VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << L"Only Player Join PvP!!!" );
				}
			}
		}break;
	case PT_C_T_REQ_EXIT_ROOM:
		{
			bool bKick = false;
			pkMsg->Pop( bKick );

			size_t iSize = 0;
			pkMsg->Pop( iSize );

			while ( iSize-- )
			{
			//	UNIT_PTR_ARRAY::ReadFromPacket 과 동일하게
				size_t const iRDPos = pkMsg->RdPos();
				EUnitType kUnitType = UT_NONETYPE;
				BM::GUID kCharGuid;
				pkMsg->Pop(kUnitType);
				pkMsg->Pop(kCharGuid);
				pkMsg->RdPos( iRDPos );

				if ( UT_PLAYER == kUnitType )
				{
					if ( true == Locked_ReadFromUserInfo( kCharGuid, pkMsg, bKick ) )
					{
						if ( !bKick )
						{
							RecvFromUser( kCharGuid, kType, pkMsg );
						}
					}
				}
				else
				{
					VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << L"Error UnitType<" << kUnitType << L">" );
					break;
				}
			}
		}break;
	case PT_S_M_WRAPPED_USER_PACKET:
		{
			BM::GUID kCharacterGuid;
			BM::Stream::DEF_STREAM_TYPE usPacketType = 0;
			pkMsg->Pop( kCharacterGuid );
			pkMsg->Pop( usPacketType );
			RecvFromUser( kCharacterGuid, usPacketType, pkMsg );
		}break;
	case PT_C_M_REQ_CHAT:
		{
			BM::GUID kCharacterGuid;
			pkMsg->Pop( kCharacterGuid );
			RecvFromUser( kCharacterGuid, kType, pkMsg );
		}break;
	case PT_ROOM_TO_LOBBY_USER:
		{
			VEC_GUID kVecGuid;
			bool bKick = false;
			pkMsg->Pop( kVecGuid );
			pkMsg->Pop( bKick );

			VEC_GUID::iterator guid_itr = kVecGuid.begin();
			for ( ;guid_itr != kVecGuid.end() ; ++guid_itr )
			{
				Locked_ReJoin( *guid_itr, bKick );
			}
		}break;
	case PT_ROOM_TO_LOBBY_REMOVED:
		{
			int iIndex = 0;
			pkMsg->Pop( iIndex );
			Locked_RemoveRoom( iIndex);
		}break;
	case PT_ROOM_TO_LOBBY_MODIFYED:
		{
			CONT_PVPROOM_LIST::key_type kKey;
			CONT_PVPROOM_LIST::mapped_type kElement;
			pkMsg->Pop(kKey);
			kElement.ReadFromPacket(*pkMsg);
			Locked_ModifyRoom( kKey, kElement );
		}break;
	case PT_LOBBY_TO_CELL_JOIN:
		{
			PgPvPCell::SJoinProcess kProcess;
			pkMsg->Pop( kProcess.m_kMemberGuid );
			pkMsg->Pop( kProcess.m_kSwitchServer );
			std::for_each( m_kCellArray, m_kCellArray+MAX_CELL_SIZE, kProcess );
		}break;
// 	case PT_LOBBY_TO_CELL_BROADCAST:
// 		{
// 			bool bOnlyLobby = true;
// 			unsigned short iCellIndex = 0;
// 			pkMsg->Pop( bOnlyLobby );
// 			if ( pkMsg->Pop( iCellIndex ) )
// 			{
// 				BM::Stream kPacket;
// 				kPacket.Push( *pkMsg );
// 				m_kCellArray[iCellIndex].Locked_BroadCast( kPacket, bOnlyLobby );
// 			}
// 		}break;
	case PT_M_T_ANS_CREATE_GROUND:
		{
			SGroundKey kGndkey;
			HRESULT hRet = E_FAIL;
			int iRoomIndex = 0;
			pkMsg->Pop(kGndkey);
			pkMsg->Pop( hRet );
			pkMsg->Pop( iRoomIndex );

			PgPvPRoom *pkRoom = GetRoom( iRoomIndex );// GroundNo에 PvP Room의 Index가 저장되어 있다.
			if ( pkRoom )
			{
				pkRoom->Start( kGndkey, SUCCEEDED(hRet) );
			}
		}break;
	case PT_A_TO_ROOM:
		{
			int iRoomIndex = 0;
			if ( pkMsg->Pop( iRoomIndex ) )
			{
				PgPvPRoom *pkRoom = GetRoom( iRoomIndex );
				if ( pkRoom )
				{
					pkRoom->ProcessMsg( pkMsg );
				}
			}
		}break;
	case PT_I_M_ANS_MODIFY_ITEM:
		{
			BM::GUID kCasterGuid;
			BM::GUID kOwnerGuid;
			DB_ITEM_STATE_CHANGE_ARRAY kChangeArray;
			EItemModifyParentEventType kCause;
			HRESULT hRet = E_FAIL;
			bool bIsAddonPacket = false;
			pkMsg->Pop(kCause);//
			pkMsg->Pop(hRet);//
			pkMsg->Pop(kCasterGuid);//
			pkMsg->Pop(kOwnerGuid);
			PU::TLoadArray_M( *pkMsg, kChangeArray );
			pkMsg->Pop(bIsAddonPacket);
			Locked_ReadFromItemChangeArray( kCasterGuid, kChangeArray );
		}break;
	//case PT_C_T_REQ_INVITE_PVP:
	case PT_N_T_REQ_CREATE_PVPLEAGUE_ROOM:
		{
			Locked_Recv_PT_N_T_REQ_CREATE_PVPLEAGUE_ROOM( pkMsg );
		}break;
	case PT_N_T_REQ_START_PVPLEAGUE_GAME:
		{
			Locked_Recv_PT_N_T_REQ_START_PVPLEAGUE_GAME();
		}break;
	case PT_N_T_REQ_END_PVPLEAGUE_GAME:
		{
			Locked_Recv_PT_N_T_REQ_END_PVPLEAGUE_GAME();
		}break;
	case PT_N_C_NFY_CLOSE_PVPLEAGUE_GAME:
		{
			Locked_Recv_PT_N_C_NFY_CLOSE_PVPLEAGUE_GAME(pkMsg);
		}break;
	case PT_N_T_NFY_CHANGE_STATE_LEAGUE_LOBBY:
		{
			Locked_ReadLeagueInfo(*pkMsg);

			BM::Stream kCPacket( PT_T_C_RELOAD_LEAGUE_LOBBY);
			Locked_WriteRoomList( kCPacket );
			Locked_WriteLeagueInfo( kCPacket );
			BroadCast( kCPacket, true );
		}break;
	default:
		{
			VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << _T("invalid Type=") <<  kType);
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
			return false;
		}break;
	}
	return true;
}

bool PgPvPLobby_League::Locked_SetOpen( bool const bIsOpen )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );
	return SetOpen( bIsOpen );
}

bool PgPvPLobby_League::SetOpen( bool const bIsOpen )
{
	if ( m_bIsOpen != bIsOpen )
	{
		m_bIsOpen = bIsOpen;
		if ( !m_bIsOpen )
		{
			UNIT_PTR_ARRAY kUnitArray;

			// 현재 접속해있는 유저를 모두 쫒아내야 한다.
			CONT_USER::iterator user_itr = m_kContUser.begin();
			while ( user_itr != m_kContUser.end() )
			{
				if ( true == IsLobbyUser( user_itr->second ) )
				{
					kUnitArray.Add( user_itr->second, true );// autoremove
					SetEvent( user_itr->second, PVPUTIL::E_EVENT_REMOVE );
					user_itr = m_kContUser.erase( user_itr );
				}
				else
				{
					++user_itr;
				}
			}

			if( kUnitArray.size() )
			{
				if ( PvP_Lobby_GroundNo_AnterRoom != GetID() )
				{
					BM::Stream kContents( PT_T_T_REQ_JOIN_LOBBY );
					kUnitArray.WriteToPacket( kContents, WT_DEFAULT );
					SendToPvPLobby( kContents, PvP_Lobby_GroundNo_AnterRoom );
				}
				else
				{
					BM::Stream kTPacket( PT_T_T_REQ_EXIT_LOBBY );
					kUnitArray.WriteToPacket( kTPacket, WT_DEFAULT );
					SendToCenter( kTPacket );
				}
			}

			// 방을 강제로 닫아야 한다.
			CONT_ALL_ROOM::iterator room_itr = m_kContRoom.begin();
			for ( ; room_itr != m_kContRoom.end() ; ++room_itr )
			{
				(*room_itr)->Close();
			}
		}

		return true;
	}
	return false;
}

bool PgPvPLobby_League::Locked_SetPlayLimitInfo( CONT_DEF_PLAYLIMIT_INFO::mapped_type const &kContPlayLimitInfo )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );

	m_kContPlayLimitInfo = kContPlayLimitInfo;

	__int64 const i64CurTime = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );

	PgPlayLimit_Finder kFinder( m_kContPlayLimitInfo );
	bool const bIsOpen = ( S_OK == kFinder.Find( i64CurTime, m_kPlayLimitInfo ) );
	SetOpen( bIsOpen );
	return true;
}

void PgPvPLobby_League::RecvFromUser( BM::GUID const &kCharGuid, BM::Stream::DEF_STREAM_TYPE const usType, BM::Stream * const pkPacket )
{
	switch ( usType )
	{
	case PT_C_T_REQ_EXIT_LOBBY:
		{
			Locked_Exit( kCharGuid, false );
		}break;
	case PT_C_T_REQ_JOIN_LEAGUE_ROOM:
		{
			BM::GUID kTeamGuid;
			pkPacket->Pop( kTeamGuid );

			int iRoomIndex = 0, iTeam = TEAM_NONE;
			if( Locked_GetBattleRoomInfo( kTeamGuid, iRoomIndex, iTeam ) )
			{
				Locked_JoinRoom( iRoomIndex, iTeam, kCharGuid );
			}
		}break;
	case PT_C_T_REQ_CHANGE_LOBBY:
		{
			int iTargetLobbyID = 0;
			pkPacket->Pop( iTargetLobbyID );
			Locked_Recv_PT_C_T_REQ_CHANGE_LOBBY( kCharGuid, iTargetLobbyID );
		}break;
	case PT_C_T_REQ_EXIT_ROOM:
	case PT_C_T_REQ_ENTRY_CHANGE:
		{
			PgPvPRoom *pkRoom = Locked_GetRoom( kCharGuid );
			if ( pkRoom )
			{
				pkRoom->RecvPacket( kCharGuid, usType, pkPacket );
			}
			else
			{
				INFO_LOG( BM::LOG_LV0, __FL__ << _T("Bad Packet=") << usType << _T(", CharacterGuid=") << kCharGuid);
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Bad Packet=") << usType );
			}
		}break;
	case PT_C_M_REQ_CHAT:
		{
			Locked_Recv_PT_C_M_REQ_CHAT( kCharGuid, pkPacket );
		}break;
	case PT_C_T_REQ_GET_PVPRANKING:
		{	
			__int64 i64LastUpdateTime = 0i64;
			pkPacket->Pop( i64LastUpdateTime );

			BM::Stream kAnsPacket( PT_T_C_ANS_GET_PVPRANKING );
			if ( true == g_kPvPRankingMgr.WriteToPacket( kAnsPacket, i64LastUpdateTime ) )
			{
				Locked_SendToUser( kCharGuid, kAnsPacket );
			}
		}break;
	default:
		{
			INFO_LOG( BM::LOG_LV0, __FL__ << _T("invalid CharGuid=") << kCharGuid << ", Type=" << usType );
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("invalid CharGuid=") << kCharGuid );
		}break;
	}	
}

void PgPvPLobby_League::Locked_OnTick()
{
	BM::Stream kUpdatePacket( PT_T_C_NFY_REFRESH_LEAGUE_LOBBY );
	bool bUpdate = false;

	{
		static CONT_PVPROOM_LIST::mapped_type const kTempCloseRoomInfo(ROOM_STATUS_CLOSE);

		BM::CAutoMutex kLock( m_kRoomMutex );// Lock
		
		if ( m_kContUpdateRoom.size() )
		{
			kUpdatePacket.Push(m_kContUpdateRoom.size());
			
			CONT_UPDATE_ROOM_LIST::const_iterator roomkey_itr = m_kContUpdateRoom.begin();
			for ( ; roomkey_itr!=m_kContUpdateRoom.end(); ++roomkey_itr )
			{
				CONT_PVPROOM_LIST::const_iterator room_itr = m_kContActiveRoom.find( *roomkey_itr );
				if ( room_itr!=m_kContActiveRoom.end() )
				{
					kUpdatePacket.Push( room_itr->first );
					room_itr->second.WriteToPacket( kUpdatePacket );
				}
				else
				{// 못찾으면 닫힌 방이다.
					kUpdatePacket.Push( *roomkey_itr );
					kTempCloseRoomInfo.WriteToPacket( kUpdatePacket );
				}
			}

			bUpdate = true;
			m_kContUpdateRoom.clear();	
		}	
	}

	if ( bUpdate )
	{
		BroadCast( kUpdatePacket, true );
	}

	__int64 const i64CurTime = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );

	if ( true == m_bIsOpen )
	{
		if ( true == CGameTime::IntoInDayTime( m_kPlayLimitInfo.i64BeginTime, m_kPlayLimitInfo.i64EndTime, i64CurTime ) )
		{
			return;
		}
	}

	PgPlayLimit_Finder kFinder( m_kContPlayLimitInfo );
	bool const bIsOpen = ( S_OK == kFinder.Find( i64CurTime, m_kPlayLimitInfo ) );
	SetOpen( bIsOpen );

	{//Tick 10s Sync LeagueInfo
		BM::CAutoMutex kLock(m_kLeagueInfoMutex);

		DWORD const dwCurTime = BM::GetTime32();
		if ( 10000 < ::DifftimeGetTime( m_dwOldCheckTime, dwCurTime ) )
		{
			if( m_kPacketLeagueInfo.IsEmpty() )
			{
				BM::Stream kPacket( PT_T_N_REQ_CHANGE_STATE_LEAGUE_LOBBY );
				::SendToRealmContents( PMET_PVP_LEAGUE, kPacket );
			}
			m_dwOldCheckTime = dwCurTime;
		}
	}
}

void PgPvPLobby_League::Locked_OnTickEvent(void)
{
	using namespace PVPUTIL;
	PgUserEvent kTempUserEvent;

	{
		BM::CAutoMutex kLock( m_kUserMutex, true );
		DWORD const dwCurTime = BM::GetTime32();

		if (	m_kUserEvent.Size() > 30 
			||	10000 < ::DifftimeGetTime( m_dwLastEventTime, dwCurTime )
			)
		{
			kTempUserEvent.Swap( m_kUserEvent );
			m_dwLastEventTime = dwCurTime;
		}
	}

	if ( kTempUserEvent.Size() )
	{
		size_t iTempSize = 0;

		BM::Stream kEventPacket( PT_T_C_NFY_PVPLOBBY_LIST_EVENT );
		kTempUserEvent.WriteToPacket( kEventPacket );

		for ( int i=0; i<MAX_CELL_SIZE; ++i )
		{
			iTempSize = m_kCellArray[i].Locked_SetUserEvent( kEventPacket );

			if ( m_iBalanceCellUserSize > iTempSize )
			{
				// 현재 벨런싱 되고 있지 않은 셀의 유저수가 더 적으면 벨런싱 정보를 교체한다.
				m_iBalanceCellUserSize = iTempSize;
				m_iBalanceCellIndex = i;
			}
			else if ( m_iBalanceCellIndex == i )
			{
				m_iBalanceCellUserSize = iTempSize;
			}
		}

		CONT_ALL_ROOM::const_iterator room_itr = m_kContRoom.begin();
		for ( ; room_itr!=m_kContRoom.end() ; ++room_itr )
		{
			(*room_itr)->SetUserEvent( kEventPacket, kTempUserEvent );
		}
	}
}

void PgPvPLobby_League::Locked_OnTickLog(void)
{
	// PvP에 접속해있는 유저숫자 보내기
	BM::CAutoMutex kLock( m_kUserMutex, false );

	// 로그를 남겨
	PgLogCont kLogCont(ELogMain_Contents_PVP, ELogSub_PVP_UserCount);
	kLogCont.ChannelNo( g_kProcessCfg.ChannelNo() );

	PgLog kLog( ELOrderMain_User_Count );
	kLog.Set( 0, static_cast<int>(m_kContUser.size()) );		// iValue1 이용자수
	kLogCont.Add( kLog );
	kLogCont.Commit();
}


bool PgPvPLobby_League::Locked_Join( PgPlayer *pkPlayer )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );

	if ( true == m_bIsOpen )
	{

		if ( pkPlayer->GetAbil( AT_LEVEL ) >= static_cast<int>(m_kPlayLimitInfo.nLevelLimit) )
		{
			std::pair<CONT_USER::iterator, bool> kPair = m_kContUser.insert( std::make_pair( pkPlayer->GetID(), pkPlayer ) );
			if ( kPair.second )
			{
				__int64 const i64CurTime = g_kEventView.GetLocalSecTime( CGameTime::DEFAULT );

				pkPlayer->SetAbil( AT_PVP_CELL_INDEX, m_iBalanceCellIndex );
				pkPlayer->SetAbil64( AT_TIME, i64CurTime );// SetAbil64에는 Lobby에 들어온 시간이 저장

				Join( pkPlayer, false );
				SetEvent( pkPlayer, PVPUTIL::E_EVENT_ADD );
				return true;
			}
			else
			{
				VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << L"overrap User[" << pkPlayer->Name() << L"-" << pkPlayer->GetID() << L"]");
			}
		}
		else
		{
			// 제한레벨에 걸림
			BM::Stream kMsgPacket( PT_M_C_NFY_WARN_MESSAGE, 460018);
			kMsgPacket.Push(static_cast<BYTE>(EL_PvPMsg));
			::SendToUser( pkPlayer->GetMemberGUID(), pkPlayer->GetSwitchServer(), kMsgPacket );
		}
	}
	else
	{
		// 입장이 가능한 시간이 아닙니다.
		BM::Stream kMsgPacket( PT_M_C_NFY_WARN_MESSAGE, 200153);
		kMsgPacket.Push(static_cast<BYTE>(EL_PvPMsg));
		::SendToUser( pkPlayer->GetMemberGUID(), pkPlayer->GetSwitchServer(), kMsgPacket );
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
	return false;
}

bool PgPvPLobby_League::Locked_Exit( BM::GUID const &kCharGuid, bool const bDisConnected )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );

	CONT_USER::iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		Exit( user_itr->second, bDisConnected );

		g_kTotalObjMgr.ReleaseUnit( user_itr->second );
		m_kContUser.erase( user_itr );
		return true;
	}
	return false;
}

void PgPvPLobby_League::Exit( PgPlayer *pkPlayer, bool const bDisConnected )
{
	if(pkPlayer)
	{
		int const iHP = pkPlayer->GetAbil(AT_MEMORIZED_HP);
		int const iMP = pkPlayer->GetAbil(AT_MEMORIZED_MP);
		if(0 < iHP 
			&& 0 < iMP 
			)
		{
			pkPlayer->SetAbil(AT_HP		, iHP);
			pkPlayer->SetAbil(AT_MP		, iMP);
		}
	}

	PgPvPRoom *pkRoom = GetRoom( pkPlayer->GetAbil( AT_PVP_ROOM_INDEX ) );
	if ( pkRoom )
	{
		pkRoom->Exit( pkPlayer->GetID(), true );
	}

	if ( !bDisConnected )
	{
		BM::Stream kTPacket( PT_T_T_REQ_EXIT_LOBBY );
		pkPlayer->WriteToPacket( kTPacket, WT_DEFAULT );
		SendToCenter( kTPacket );
	}
// 	else
// 	{
// 		if( PvP_Lobby_GroundNo_Ranking == GetID() )
// 		{
// 			SPvPRanking kPvPRanking( pkPlayer );
// 			g_kPvPRankingMgr.Update( kPvPRanking );//나갈때 업데이트
// 		}
// 	}

	SetEvent( pkPlayer, PVPUTIL::E_EVENT_REMOVE );
}

void PgPvPLobby_League::Join( PgPlayer *pkPlayer, bool const bKick )const
{
	pkPlayer->SetAbil( AT_PVP_ROOM_INDEX, -1 );

	// ServerSetMgr로 통보->Switch
	SAnsMapMove_MT kAMM( g_kProcessCfg.ServerIdentity(), MMET_PvP, MMR_PVP_LOBBY );
	kAMM.kAttr = GATTR_PVP;
	kAMM.kGroundKey.GroundNo(GetID());

	BM::Stream kTPacket( PT_T_T_ANS_MAP_MOVE_RESULT, kAMM );
	kTPacket.Push( pkPlayer->GetID() );
	::SendToCenter( kTPacket );

	size_t const iRank = g_kPvPRankingMgr.GetRank( pkPlayer->GetID() );

	BM::Stream kCPacket( PT_T_C_ANS_JOIN_LEAGUE_LOBBY);
	kCPacket.Push( iRank );
	kCPacket.Push( GetID() );
	kCPacket.Push( bKick );
	Locked_WriteRoomList( kCPacket );
	Locked_WriteLeagueInfo( kCPacket );
	::SendToUser( pkPlayer->GetMemberGUID(), pkPlayer->GetSwitchServer(), kCPacket );
}

bool PgPvPLobby_League::Locked_ReJoin( BM::GUID const &kCharGuid, bool const bKick )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );
	CONT_USER::iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		if ( true == m_bIsOpen )
		{
			Join( user_itr->second, bKick );

			SetEvent( user_itr->second, PVPUTIL::E_EVENT_EXIT_ROOM );
		}
		else
		{
			UNIT_PTR_ARRAY kUnitArray;
			kUnitArray.Add( user_itr->second, true );// autoremove

			SetEvent( user_itr->second, PVPUTIL::E_EVENT_REMOVE );
			user_itr = m_kContUser.erase( user_itr );

			if ( PvP_Lobby_GroundNo_AnterRoom != GetID() )
			{
				BM::Stream kContents( PT_T_T_REQ_JOIN_LOBBY );
				kUnitArray.WriteToPacket( kContents, WT_DEFAULT );
				SendToPvPLobby( kContents, PvP_Lobby_GroundNo_AnterRoom );
			}
		}
		return true;
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
	return false;
}
bool PgPvPLobby_League::Locked_GetBattleRoomInfo( BM::GUID const & rkTeamGuid, int &iOutRoomIndex, int &iOutTeam  ) const
{
	BM::CAutoMutex kLock( m_kBattleMutex );// Battle Lock

	PgPvPRoom *pkRoom = NULL;
	CONT_PVPLEAGUE_TEAM_TO_BATTLE::const_iterator team_itr = m_kContTeamToBattle.find(rkTeamGuid);
	if( m_kContTeamToBattle.end() != team_itr )
	{
		CONT_DEF_PVPLEAGUE_BATTLE::const_iterator battle_itr = m_kContBattle.find(team_itr->second);
		if( m_kContBattle.end() != battle_itr )
		{
			iOutRoomIndex = battle_itr->second.iRoomIndex;
			if( rkTeamGuid == battle_itr->second.kTeamGuid1 )
			{
				iOutTeam = TEAM_RED;
			}
			else
			{
				iOutTeam = TEAM_BLUE;
			}

			return true;
		}
	}

	iOutTeam = TEAM_NONE;
	return false;
}

bool PgPvPLobby_League::Locked_JoinRoom( int const iRoomIndex, int const iTeam, BM::GUID const &rkCharGuid )
{
	PgPvPRoom *pkRoom = GetRoom( iRoomIndex );
	if ( pkRoom && iTeam )
	{
		BM::CAutoMutex kLock( m_kUserMutex, true );//Lock

		CONT_USER::iterator user_itr = m_kContUser.find( rkCharGuid );
		if ( user_itr != m_kContUser.end() )
		{
			if ( IsLobbyUser( user_itr->second ) )
			{
				if ( SUCCEEDED( pkRoom->Join( user_itr->second, iTeam ) ) )
				{
					user_itr->second->SetAbil( AT_PVP_ROOM_INDEX, pkRoom->GetIndex() );
					SetEvent( user_itr->second, PVPUTIL::E_EVENT_JOIN_ROOM );
					return true;
				}
			}
		}
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
	return false;
}

bool PgPvPLobby_League::Locked_ReadFromUserInfo( BM::GUID const &kCharGuid, BM::Stream * const pkPacket, bool const bKickRoom )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );

	CONT_USER::iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		user_itr->second->ReadFromPacket( *pkPacket );

		if ( true == bKickRoom )
		{
			PgPvPRoom *pkRoom = GetRoom( user_itr->second->GetAbil(AT_PVP_ROOM_INDEX) );
			if ( pkRoom )
			{
				pkRoom->Exit( kCharGuid, false );
			}

			Join( user_itr->second, true );
			SetEvent( user_itr->second, PVPUTIL::E_EVENT_EXIT_ROOM );
		}
		return true;
	}

	PgPlayer kTempPlayer;
	kTempPlayer.ReadFromPacket( *pkPacket );

	// 패킷순서 문제로 이런경우가 있을 수도 있을 것 같은데...혹시 몰라서.
	INFO_LOG( BM::LOG_LV0, __FL__ << _T("NotFound User=") << kCharGuid );
	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
	return false;
}

bool PgPvPLobby_League::Locked_ReadFromItemChangeArray( BM::GUID const &kCharGuid, DB_ITEM_STATE_CHANGE_ARRAY &kArray )
{
	BM::CAutoMutex kLock( m_kUserMutex, true );

	CONT_USER::iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		bool bUpdateRank = false;

		DB_ITEM_STATE_CHANGE_ARRAY::iterator change_itr = kArray.begin();
		for ( ; change_itr!=kArray.end() ; ++change_itr )
		{
			DB_ITEM_STATE_CHANGE_ARRAY::value_type& rkItemChange = *change_itr;
			switch ( rkItemChange.State() )
			{
			case DISCT_MODIFY_SET_PVP_RECORD:
				{
					SPMOD_SAbil kData;
					kData.ReadFromPacket( rkItemChange.kAddonData );

					user_itr->second->SetPvPRecord( kData.Type(), kData.Value() );

					bUpdateRank = ( PvP_Lobby_GroundNo_Ranking == GetID() );
				}break;
			}
		}

		if( true == bUpdateRank )
		{
			SPvPRanking kPvPRanking( user_itr->second );
			g_kPvPRankingMgr.Update( kPvPRanking );// 업데이트
		}

		return true;
	}
	return false;
}

bool PgPvPLobby_League::Locked_SendToUser( BM::GUID const &kCharGuid, BM::Stream const &kPacket )const
{
	BM::CAutoMutex kLock( m_kUserMutex, false );
	return SendToUser( kCharGuid, kPacket );
}

void PgPvPLobby_League::BroadCast( BM::Stream const &kPacket, bool const bOnlyLobby )const
{
	for ( int i=0; i<MAX_CELL_SIZE; ++i )
	{
		m_kCellArray[i].Locked_BroadCast( kPacket,  bOnlyLobby );
	}
}

bool PgPvPLobby_League::SendToUser( BM::GUID const &kCharGuid, BM::Stream const &kPacket )const
{
	CONT_USER::const_iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		CONT_USER::mapped_type const &pkElement = user_itr->second;
		return ::SendToUser( pkElement->GetMemberGUID(), pkElement->GetSwitchServer(), kPacket );
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return flase"));
	return false;
}

void PgPvPLobby_League::Locked_ReadLeagueInfo( BM::Stream &kPacket )
{
	BM::CAutoMutex kLock(m_kLeagueInfoMutex);
	m_kPacketLeagueInfo.Clear();
	m_kPacketLeagueInfo.Push(kPacket);
}

void PgPvPLobby_League::Locked_WriteLeagueInfo( BM::Stream &kPacket )const
{
	BM::CAutoMutex kLock(m_kLeagueInfoMutex);
	kPacket.Push( m_kPacketLeagueInfo );
}

void PgPvPLobby_League::Locked_WriteRoomList( BM::Stream& kPacket )const
{
	BM::CAutoMutex kLock(m_kRoomMutex);
	PU::TWriteTable_AM( kPacket, m_kContActiveRoom );
}

bool PgPvPLobby_League::Locked_Recv_PT_N_T_REQ_CREATE_PVPLEAGUE_ROOM( BM::Stream * const pkPacket )
{
	SPvPRoomBasicInfo kBasicInfo;
	SPvPRoomExtInfo kExtInfo;
	kBasicInfo.ReadFromPacket( *pkPacket );
	kExtInfo.ReadFromPacket( *pkPacket );

	BM::CAutoMutex kLock(m_kBattleMutex);// Battle lock

	m_kContBattle.clear();
	m_kContTeamToBattle.clear();
	PU::TLoadTable_AM( *pkPacket, m_kContBattle );

	if( m_kContBattle.empty() )
	{
		BM::Stream kAnsPacket( PT_T_N_ANS_CREATE_PVPLEAGUE_ROOM, false );
		::SendToRealmContents( PMET_PVP_LEAGUE, kAnsPacket );

		VERIFY_INFO_LOG(false, BM::LOG_LV5, __FL__ << L"Battle is Empty!!" );
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	CONT_DEF_PVP_GROUNDGROUP const *pkDefPvPGroup = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPGroup);

	VEC_GUID kVecGuidAutoWin;
	m_kContTeamToBattle.clear();
	CONT_DEF_PVPLEAGUE_BATTLE::iterator battle_itr = m_kContBattle.begin();
	while( m_kContBattle.end() != battle_itr )
	{	
		if ( !m_kContCloseRoom.empty() )
		{
			if( battle_itr->second.kWinTeamGuid.IsNull() 
				&& battle_itr->second.kTeamGuid1.IsNotNull() && battle_itr->second.kTeamGuid2.IsNotNull() )
			{//부전승 배틀은 룸 생성 안함...(부전승 처리는 컨텐츠에서 이미 했음)

				m_kContTeamToBattle.insert( std::make_pair(battle_itr->second.kTeamGuid1, battle_itr->first) );
				m_kContTeamToBattle.insert( std::make_pair(battle_itr->second.kTeamGuid2, battle_itr->first) );
				
				CONT_CLOSE_ROOM::iterator closeRoom_itr = m_kContCloseRoom.begin();
				PgPvPRoom *pkRoom = closeRoom_itr->second;
				if( pkRoom )
				{
					kBasicInfo.m_iGndNo = PVPUTIL::GetRandomPvPGround(pkDefPvPGroup, kBasicInfo.m_kType, kBasicInfo.m_ucMaxUser);
					kBasicInfo.m_kBattleGuid = battle_itr->first;
					battle_itr->second.iRoomIndex = pkRoom->GetIndex();

					if ( SUCCEEDED(pkRoom->Create( battle_itr->first, kBasicInfo, kExtInfo )) )
					{
						BM::CAutoMutex kLock( m_kRoomMutex );// room lock
						m_kContActiveRoom.insert( std::make_pair( pkRoom->GetRoomNo(), kBasicInfo ) );
						m_kContUpdateRoom.insert( pkRoom->GetRoomNo() );
						m_kContCloseRoom.erase(closeRoom_itr);
					}
					else
					{
						VERIFY_INFO_LOG(false, BM::LOG_LV1, __FL__ << _T("CreateRoom Failed!! INDEX < ") << battle_itr->second.iRoomIndex << _T(" >") );
					}
				}
			}
		}
		else
		{
			// 더이상 방을 생성 할 수 없습니다.
			VERIFY_INFO_LOG(false, BM::LOG_LV1, __FL__ << _T("Room is Full!!, RoomCount < ") << m_kContActiveRoom.size() << _T(" >") );

			BM::Stream kAnsPacket( PT_T_N_ANS_CREATE_PVPLEAGUE_ROOM, false );
			::SendToRealmContents( PMET_PVP_LEAGUE, kAnsPacket );
			return false;
		}
		++battle_itr;
	}

	BM::Stream kAnsPacket( PT_T_N_ANS_CREATE_PVPLEAGUE_ROOM, true );
	::SendToRealmContents( PMET_PVP_LEAGUE, kAnsPacket );

	return true;
}

bool PgPvPLobby_League::Locked_Recv_PT_C_T_REQ_CHANGE_LOBBY( BM::GUID const &kCharGuid, int const iTargetLobbyID )
{
	if ( iTargetLobbyID != GetID() )
	{
		BM::CAutoMutex kLock( m_kUserMutex, true );

		CONT_USER::iterator user_itr = m_kContUser.find( kCharGuid );
		if ( user_itr != m_kContUser.end() )
		{
			if ( true == IsLobbyUser( user_itr->second ) )
			{
				__int64 const i64CurTime = g_kEventView.GetLocalSecTime( CGameTime::DEFAULT );
				__int64 const i64JoinTime = user_itr->second->GetAbil64( AT_TIME );
				__int64 const i64CheckTime = i64CurTime - i64JoinTime;
				if ( ms_i64ChangeLobbyTime < i64CheckTime )
				{
					Exit( user_itr->second, true );

					UNIT_PTR_ARRAY kUnitArray;
					kUnitArray.Add( user_itr->second, true );// AutoRemove true

					BM::Stream kContents( PT_T_T_REQ_JOIN_LOBBY );
					kUnitArray.WriteToPacket( kContents, WT_DEFAULT );
					SendToPvPLobby( kContents, iTargetLobbyID );

				//	g_kTotalObjMgr.RegistUnit( user_itr->second );AutoRemove에서 지우니까 이걸 호출하면 절대로 안된다.
					m_kContUser.erase( user_itr );
					return true;
				}
				else
				{
					// 너무 자주 바꿀수 없습니다.
				}
			}
		}
	}
	return false;
}

bool PgPvPLobby_League::Locked_Recv_PT_C_M_REQ_CHAT( BM::GUID const &kCharGuid, BM::Stream * const pkPacket )const
{	
	BYTE ucChatMode = 0;
	pkPacket->Pop(ucChatMode);

	std::wstring wstrChat;
	DWORD dwFontColor = 0;
	pkPacket->Pop( wstrChat );
	pkPacket->Pop( dwFontColor );

	BM::CAutoMutex kLock( m_kUserMutex, false );

	CONT_USER::const_iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr == m_kContUser.end() )
	{
		return false;
	}

	PgPlayer * pkPlayer = user_itr->second;
	if ( !pkPlayer )
	{
		VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << L"Player is NULL CharGuid<" << kCharGuid << L">" );
		LIVE_CHECK_LOG( BM::LOG_LV0, __FL__ << L"Return false" );
		return false;
	}
	// 필터 검사

	if ( true == Emoticon::FilterEmoticon( pkPlayer, wstrChat ) )
	{
		if ( true == g_kUnicodeFilter.IsCorrect( UFFC_CHAT, wstrChat ) )
		{
			XUI::PgExtraDataPackInfo kSendExtraDataPackInfo;
			GetExtraDataPackInfo( pkPlayer->GetInven(), pkPacket, kSendExtraDataPackInfo );//4

			int iEmoticonNo = 0;
			if( S_OK == g_kVariableContainer.Get(EVar_Kind_GM_Emoticon, EVar_GM_Emoticon_Default, iEmoticonNo) )
			{
				Emoticon::AddGMEmoticon( pkPlayer, wstrChat, iEmoticonNo);
			}

			if( S_OK == g_kVariableContainer.Get(EVar_Kind_Premium_Service, EVar_VIP_Emoticon, iEmoticonNo) )
			{
				Emoticon::AddVIPEmoticon(pkPlayer, wstrChat, iEmoticonNo);
			}

			switch ( ucChatMode )
			{
			case CT_NORMAL:
			case CT_TEAM:
			case CT_PARTY:
				{
					BM::Stream kPacket( PT_M_C_NFY_CHAT, ucChatMode );
					kPacket.Push( kCharGuid );
					
					int iTeam = TEAM_NONE;

					kPacket.Push( user_itr->second->Name() );
					//kPacket.Push(std::chrono::system_clock::now());
					kPacket.Push( wstrChat );
					kPacket.Push( dwFontColor );
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					kPacket.Push( *pkPacket );

					PgPvPRoom *pkRoom = GetRoom( user_itr->second->GetAbil(AT_PVP_ROOM_INDEX) );
					if ( pkRoom )
					{
						pkRoom->BroadCast( kPacket, iTeam, PS_PLAYING );

						SContentsUser kFromUser;
						if( S_OK == GetPlayerByGuid(kCharGuid, false, kFromUser) )
						{
							PgChatLogUtil::Log(ELogSub_Chat_PvpRoom, kFromUser, SContentsUser(), wstrChat);
						}	
					}
					else
					{
						LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
						return false;
					}
				}break;
			case CT_TRADE:
				{
				}break;
			default:
				{
					BM::Stream kPacket(PT_C_M_REQ_CHAT);
					kPacket.Push(kCharGuid);
					kPacket.Push(ucChatMode);
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kPacket.Push( *pkPacket );
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					::SendToRealmChatMgr(kPacket);
				}break;
			}

			return true;
		}
	}

	LIVE_CHECK_LOG( BM::LOG_LV5, __FL__ << L"Return false");
	return false;
}

PgPvPRoom* PgPvPLobby_League::Locked_GetRoom( BM::GUID const &kCharGuid )const
{
	BM::CAutoMutex kLock( m_kUserMutex, false );
	CONT_USER::const_iterator user_itr = m_kContUser.find( kCharGuid );
	if ( user_itr != m_kContUser.end() )
	{
		return GetRoom( user_itr->second->GetAbil(AT_PVP_ROOM_INDEX) );
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return NULL"));
	return NULL;
}

bool PgPvPLobby_League::Locked_ModifyRoom( CONT_PVPROOM_LIST::key_type const &kKey, CONT_PVPROOM_LIST::mapped_type const &kElement )
{
	BM::CAutoMutex kLock(m_kRoomMutex);
	CONT_PVPROOM_LIST::iterator room_itr = m_kContActiveRoom.find( kKey );
	if ( room_itr != m_kContActiveRoom.end() )
	{
		room_itr->second = kElement;
		m_kContUpdateRoom.insert( kKey );
		return true;
	}

	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
	return false;
}

void PgPvPLobby_League::Locked_RemoveRoom( int const iIndex )
{
	PgPvPRoom *pkRoom = GetRoom( iIndex );
	if ( pkRoom )
	{
		BM::GUID kBattleGuid;
		if( pkRoom->GetBattleGuid(kBattleGuid) )
		{
			BM::CAutoMutex kLock(m_kBattleMutex);// Battle lock
			m_kContBattle.erase(kBattleGuid);
		}

		BM::CAutoMutex kLock(m_kRoomMutex);// Room lock

		m_kContActiveRoom.erase( pkRoom->GetRoomNo() );
		m_kContUpdateRoom.insert( pkRoom->GetRoomNo() );// Update에 넣어놓으면 알아서 지우게 패킷을 보낸다.
		m_kContCloseRoom.insert( std::make_pair( iIndex, pkRoom ) );
	}
	else
	{
		VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << _T("Not Found Room Index=") << iIndex );
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Not Found Room Index=") << iIndex );
	}
}

void PgPvPLobby_League::SetEvent( PgPlayer const *pkPlayer, BYTE const byType )
{// m_kUserMutex WriteLock에서 호출
	using namespace PVPUTIL;
	switch ( byType )
	{
	case E_EVENT_ADD:
		{
			PgPvPCell *pkCell = GetCell( pkPlayer );
			if ( pkCell )
			{
				pkCell->Locked_Join( pkPlayer );

				BM::Stream kMsg( PT_LOBBY_TO_CELL_JOIN, pkPlayer->GetMemberGUID() );
				kMsg.Push( pkPlayer->GetSwitchServer() );
				SendToPvPLobby( kMsg, GetID() );
			}
		}break;
	case E_EVENT_REMOVE:
		{
			PgPvPCell *pkCell = GetCell( pkPlayer );
			if ( pkCell )
			{
				pkCell->Locked_Exit( pkPlayer );
			}
		}break;		
	case E_EVENT_JOIN_ROOM:
		{
			PgPvPCell *pkCell = GetCell( pkPlayer );
			if ( pkCell )
			{
				pkCell->Locked_JoinRoom( pkPlayer );
			}
		}break;
	case E_EVENT_EXIT_ROOM:
		{
			PgPvPCell *pkCell = GetCell( pkPlayer );
			if ( pkCell )
			{
				pkCell->Locked_ExitRoom( pkPlayer );
			}
		}break;
	}

	// m_kUserMutex WriteLock에서 호출
	SPvPLobbyUser_Base kPvPLobbyUser;
	CopyPlayerToLobbyUser( pkPlayer, kPvPLobbyUser );
	m_kUserEvent.DoAdd( kPvPLobbyUser, byType );
}

void PgPvPLobby_League::Locked_Recv_PT_N_T_REQ_START_PVPLEAGUE_GAME()
{
	VEC_GUID kContBattleID;
	CONT_ALL_ROOM::iterator room_itr = m_kContRoom.begin();
	while(m_kContRoom.end() != room_itr)
	{
		PgPvPRoom* pkRoom = (*room_itr);
		BM::GUID kBattleGuid;
		if( pkRoom && pkRoom->GetBattleGuid(kBattleGuid) )
		{
			BM::CAutoMutex kLock(m_kBattleMutex);// Battle Lock

			CONT_DEF_PVPLEAGUE_BATTLE::iterator battle_itr = m_kContBattle.find(kBattleGuid);
			if( m_kContBattle.end() != battle_itr )
			{
				HRESULT hRet = pkRoom->AutoReady();
				if( S_OK == hRet )
				{//2명 이상이라면 정상 플레이 시작
					battle_itr->second.iBattleState = PVPLB_PLAY;
				}
				else
				{
					if( S_FALSE == hRet )
					{//유저 1명인경우 그 팀을 승리 처리
						int const iWinTeam = pkRoom->GetAutoWinTeam();
						SEventMessage kMessage;
						kMessage.Push(iWinTeam);
						pkRoom->End(&kMessage);
					}
					else// if( E_FAIL == hRet )
					{//시작 실패하면 둘다 실격 처리
						kContBattleID.push_back(kBattleGuid);
					}

					//룸 클로즈
					pkRoom->Close();
				}
			}
		}
		++room_itr;
	}
	BM::Stream kAnsPacket(PT_T_N_ANS_START_PVPLEAGUE_GAME, kContBattleID.empty());
	if(!kContBattleID.empty())
	{	
		kAnsPacket.Push(kContBattleID);
	}
	::SendToRealmContents( PMET_PVP_LEAGUE, kAnsPacket );
}

bool PgPvPLobby_League::Locked_Recv_PT_N_T_REQ_END_PVPLEAGUE_GAME()
{
	VEC_GUID kContBattleID;
	CONT_ALL_ROOM::const_iterator room_itr = m_kContRoom.begin();
	while(m_kContRoom.end() != room_itr)
	{
		PgPvPRoom* pkRoom = (*room_itr);
		if( pkRoom )
		{
			BM::GUID kBattleGuid;
			if( pkRoom->GetBattleGuid(kBattleGuid) )
			{
				BM::CAutoMutex kLock( m_kBattleMutex );//! Battle Lock

				CONT_DEF_PVPLEAGUE_BATTLE::iterator battle_itr = m_kContBattle.find(kBattleGuid);
				if( m_kContBattle.end() != battle_itr )
				{
					CONT_DEF_PVPLEAGUE_BATTLE::mapped_type &kElement = battle_itr->second;
					if( PVPLB_PLAY != kElement.iBattleState )
					{
						kContBattleID.push_back(kBattleGuid);
						m_kContTeamToBattle.erase(kElement.kTeamGuid1);
						m_kContTeamToBattle.erase(kElement.kTeamGuid2);
						m_kContBattle.erase(battle_itr);
						INFO_LOG(BM::LOG_LV6, _T("Close Game, BattleGuid : [ ") << kBattleGuid << _T("") );
					}
				}
			}
			//뭐가 어찌되었건 룸 클로즈
			pkRoom->Close();
		}
		++room_itr;
	}

	BM::Stream kAnsPacket(PT_T_N_ANS_END_PVPLEAGUE_GAME, kContBattleID.empty());
	if(!kContBattleID.empty())
	{	
		kAnsPacket.Push(kContBattleID);
	}
	::SendToRealmContents( PMET_PVP_LEAGUE, kAnsPacket );

	return true;
}

bool PgPvPLobby_League::Locked_Recv_PT_N_C_NFY_CLOSE_PVPLEAGUE_GAME(BM::Stream * const pkPacket )
{
	BM::GUID kBattleGuid;
	int iRoomIndex;
	if( pkPacket->Pop(kBattleGuid) && pkPacket->Pop(iRoomIndex) )
	{
		PgPvPRoom *pkRoom = GetRoom( iRoomIndex );
		if ( pkRoom )
		{
			if( pkRoom->GetBattleGuid(kBattleGuid) )
			{
				BM::CAutoMutex kLock( m_kBattleMutex );//! Battle Lock

				CONT_DEF_PVPLEAGUE_BATTLE::iterator battle_itr = m_kContBattle.find(kBattleGuid);
				if( m_kContBattle.end() != battle_itr )
				{
					CONT_DEF_PVPLEAGUE_BATTLE::mapped_type &kElement = battle_itr->second;
					if( PVPLB_PLAY != kElement.iBattleState )
					{
						m_kContTeamToBattle.erase(kElement.kTeamGuid1);
						m_kContTeamToBattle.erase(kElement.kTeamGuid2);
						m_kContBattle.erase(battle_itr);
						INFO_LOG(BM::LOG_LV6, _T("Close Game, BattleGuid : [ ") << kBattleGuid << _T("") );
					}
				}
			}
			//뭐가 어찌되었건 룸 클로즈
			pkRoom->Close();
		}
		return true;
	}

	return false;
}