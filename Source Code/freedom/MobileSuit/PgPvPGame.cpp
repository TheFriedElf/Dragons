#include "stdafx.h"
#include "PgNetwork.h"
#include "PgChatMgrClient.h"
#include "PgPvPGame.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "PgUISound.h"
#include "PgAction.H"
#include "PgObserverMode.h"
#include "PgDamageNumMan.h"
#include "PgFriendMgr.h"
#include "Lohengrin/PgPlayLimiter.h"
#include "PgTrigger.h"
#include "PgParticleMan.h"
#include "PgMobileSuit.h"

extern int const NULL_ITEM_NUM;
extern int const TAB_ALLNUM = 3;
extern int const CP_VALUE = 10;
extern int const MAX_LEAGUE_LEVEL = 8;
extern int const MAX_LEAGUE_LEVEL_HALF = MAX_LEAGUE_LEVEL/2;
extern int const MAX_LEAGUE_PAGE_ELEMENT = 24;
extern int const MAX_TEAMELEMENT_COUNT = 15;
extern int const MAX_TEAM_TITLE_LENGTH = 20;

extern int const LOVE_MODE_REVIVE_TIME = 8000;

extern void OnSetPvpDieUserSlot( lwUIWnd UISelf, bool const bDie );

char const * const EFX_DOMINATION_RANGE_01_BLUE = "efx_domination_range_01_blue";
char const * const EFX_DOMINATION_RANGE_01_RED = "efx_domination_range_01_red";

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

void PgPvPRankingUI::Refresh()
{
	if ( m_pWnd )
	{
		BM::vstring vstr;

		XUI::CXUI_Wnd * pkControl = m_pWnd->GetControl( L"SFRM_RANK" );
		if ( pkControl )
		{
			vstr = m_iRank;
			pkControl->Text( vstr );
		}

		pkControl = m_pWnd->GetControl( L"SFRM_POINT" );
		if ( pkControl )
		{
			vstr = m_kRankingInfo.iPoint;
			pkControl->Text( vstr );
		}

		pkControl = m_pWnd->GetControl( L"SFRM_LEVEL" );
		if ( pkControl )
		{
			vstr = m_kRankingInfo.kClassKey.nLv;
			pkControl->Text( vstr );
		}

		pkControl = m_pWnd->GetControl( L"IMG_CLASS" );
		lwSetMiniClassIconIndex( pkControl, m_kRankingInfo.kClassKey.iClass);

		pkControl = m_pWnd->GetControl( L"SFRM_NAME" );
		if ( pkControl )
		{
			pkControl->Text( m_kRankingInfo.wstrName );
		}

		pkControl = m_pWnd->GetControl( L"SFRM_RECORD" );
		if ( pkControl )
		{
			vstr = m_kRankingInfo.iWin;
			vstr += TTW(200154);
			vstr += L" ";
			vstr += m_kRankingInfo.iLose;
			vstr += TTW(200155);

			pkControl->Text( vstr );
		}
	}
}

void PgLobbyUserUI::Refresh()
{
	if ( m_pWnd )
	{
		m_pWnd->Text(m_kUserInfo.wstrName);

		XUI::CXUI_Wnd *pkControl = m_pWnd->GetControl( _T("FRM_POS") );
		if ( pkControl )
		{
			if ( PVPUTIL::IsLobbyUser( m_kUserInfo ) )
			{
				pkControl->Text( TTW(200140) );
			}
			else
			{
				BM::vstring vStr(TTW(400375));
				vStr.Replace(_T("#NUM#"), m_kUserInfo.iRoomIndex + 1);
				pkControl->Text( vStr );
			}
		}

		XUI::CXUI_Wnd *pkItem = m_pWnd->GetControl( _T("IMG_CLASS") );
		lwSetMiniClassIconIndex( pkItem, m_kUserInfo.iClass);
	}
}

void PgLobbyUserUI::SetSelect()
{
	if ( m_pWnd )
	{
		XUI::CXUI_Wnd *pkControl = m_pWnd->GetControl( _T("FRM_COMMUNITY_FRAME") );
		if ( pkControl )
		{
			pkControl->Visible( true );
		}

		// 자기정보를 보여주기 수정. 이제 안쓴다.
		return;

		XUI::CXUI_Wnd *pkInfo = m_pWnd->Parent();
		if ( pkInfo )
		{
			pkInfo = pkInfo->Parent();
		}

		if ( pkInfo )
		{
			pkInfo = pkInfo->Parent();
		}

		if ( pkInfo )
		{
			pkControl = pkInfo->GetControl( _T("SFRM_NAME") );
			if ( pkControl )
			{
				pkControl->Text( m_kUserInfo.wstrName );
			}

			BM::vstring vstr;

			pkControl = pkInfo->GetControl( _T("SFRM_LEVEL") );
			if ( pkControl )
			{
				vstr = m_kUserInfo.sLevel;
				pkControl->Text( vstr );
			}

			pkControl = pkInfo->GetControl( _T("SFRM_CLASS") );
			if ( pkControl )
			{
				pkControl->Text( TTW(30000+m_kUserInfo.iClass) );
			}

			pkControl = pkInfo->GetControl( _T("SFRM_BATTLEFIELD"));
			if ( pkControl )
			{
				vstr = m_kUserInfo.kPvPReport.m_iRecords[GAME_WIN];
				vstr << _T(" - ") << m_kUserInfo.kPvPReport.m_iRecords[GAME_LOSE];
				pkControl->Text( vstr );
			}
		}
	}
}

void PgLobbyUserUI::DelSelect()
{
	if ( m_pWnd )
	{
		XUI::CXUI_Wnd *pkControl = m_pWnd->GetControl( _T("FRM_COMMUNITY_FRAME") );
		if ( pkControl )
		{
			pkControl->Visible( false );
		}
	}
}


PgPvPGame::PgPvPGame()
:	m_iLobbyID(0)
{
	Clear();
}

PgPvPGame::PgPvPGame(size_t const iMyRank)
:	m_iLobbyID(0)
{
	Clear();
	m_iMyRank = iMyRank;
}

PgPvPGame::~PgPvPGame()
{
	Clear();
}

void PgPvPGame::Clear(bool bClearAll)
{
	for(int i=0;i<TAB_ALLNUM;++i)
	{
		XUI::CXUI_List2 *pkList = GetLobbyTabList(i);
		if ( pkList )
		{
			pkList->ClearList();
		}
	}

	PgContentsBase::Clear();
	m_kRoomBaseInfo.Clear();
	m_kRoomExtInfo.Clear();
	m_iRoomKey = 0;
	m_kContRoom.clear();
	m_kContPvPUser.clear();
	m_bMyRoom = false;
	m_ucNowRound = 0;
	::memset( m_ucRoundScore, 0, sizeof(m_ucRoundScore) );
	m_kPage.Clear();
	m_kReward.clear();
	m_fCameraZoom = 0.0f;
	m_kContTeamResult.clear();

	m_ContLink.clear();
	m_ContTriggerEntity.clear();
	m_ContTriggerParticle.clear();
	m_LevelUpPointInfo.clear();

	if( bClearAll )
	{
		m_kContDefPvPLeagueTime.clear();
		m_kLeagueInfo.Clear();
		m_kTournamentInfo.Clear();
		m_kContBattle.clear();
		m_kContTeam.clear();
		m_kContCharToTeam.clear();
		m_kContIndexToTeam.clear();
		m_kContGroup.clear();
	}

	SetPvPLock( false );

	m_iMyRank = 0;
	m_iBeginPageNo = 0;
	m_iEndPageNo = 0;
}

void PgPvPGame::DrawTime( size_t iRemainTimeSec )
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	if ( XUIMgr.IsActivate( ms_FRM_PVP_MAIN, pkWnd ) )
	{
		pkWnd = pkWnd->GetControl(_T("FRM_TIME"));

		if ( pkWnd )
		{
			size_t iRemainTimeMin = (iRemainTimeSec / 60) % 100;
			iRemainTimeSec %= 60;

			XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("IMG_MIN10"));
			if ( pkTemp )
			{
				pkTemp->UVUpdate( iRemainTimeMin/10+1 );
			}

			pkTemp = pkWnd->GetControl(_T("IMG_MIN01"));
			if ( pkTemp )
			{
				pkTemp->UVUpdate( iRemainTimeMin%10+1 );
			}

			pkTemp = pkWnd->GetControl(_T("IMG_SEC10"));
			if ( pkTemp )
			{
				pkTemp->UVUpdate( iRemainTimeSec/10+1 );
			}

			pkTemp = pkWnd->GetControl(_T("IMG_SEC01"));
			if ( pkTemp )
			{
				pkTemp->UVUpdate( iRemainTimeSec%10+1 );
			}
		}
	}
}

bool PgPvPGame::Update( float const fAccumTime, float const fFrameTime )
{
	if ( m_eStatus == STATUS_PLAYING )
	{
		size_t iRemainTimeSec = 0;
		GetRemainTime( iRemainTimeSec );
		DrawTime( iRemainTimeSec );
	}
	return true;
}

bool PgPvPGame::ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket )
{
	switch( wType )
	{
	case PT_M_C_NFY_GAME_WAIT:
		{
			SetStatus( STATUS_READY );
		}break;
	case PT_M_C_NFY_GAME_START:
		{
			RecvSyncRemainTime(kPacket);
			Start( kPacket );
		}break;
	case PT_T_C_ANS_JOIN_LOBBY:
		{
			Recv_PT_T_C_ANS_JOIN_LOBBY(kPacket);
		}break;
	case PT_T_C_NFY_ROOMLIST_REMOVE:
		{
			Recv_PT_T_C_NFY_ROOMLIST_REMOVE(kPacket);
		}break;
	case PT_T_C_NFY_ROOMLIST_UPDATE:
		{
			Recv_PT_T_C_NFY_ROOMLIST_UPDATE(kPacket);
		}break;
	case PT_T_C_NFY_REFRESH_LEAGUE_LOBBY:
		{
			Recv_PT_T_C_NFY_REFRESH_LEAGUE_LOBBY(kPacket);
		}break;
	case PT_T_C_ANS_JOIN_ROOM:
		{
			Recv_PT_T_C_ANS_JOIN_ROOM(kPacket);
		}break;
	case PT_T_C_ANS_MODIFY_ROOM:
		{
			Recv_PT_T_C_ANS_MODIFY_ROOM(kPacket);
		}break;
	case PT_T_C_ANS_TEAM_CHANGE:
		{
			Recv_PT_T_C_ANS_TEAM_CHANGE(kPacket);
		}break;
	case PT_T_C_NFY_USER_STATUS_CHANGE:
		{
			Recv_PT_T_C_NFY_USER_STATUS_CHANGE(kPacket);
		}break;
// 	case PT_T_C_NFY_CHANGE_HANDYCAP:
// 		{
// 			Recv_PT_T_C_NFY_CHANGE_HANDYCAP(kPacket);
// 		}break;
	case PT_T_C_NFY_EXIT_ROOM_USER:
		{
			Recv_PT_T_C_NFY_EXIT_ROOM_USER(kPacket);
		}break;
	case PT_M_C_NFY_GAME_EVENT_CHANGEPOINT:
		{
			Recv_PT_M_C_NFY_GAME_EVENT_CHANGEPOINT(kPacket);
		}break;
	case PT_M_C_NFY_GAME_EVENT_KILL:
		{
			Recv_PT_M_C_NFY_GAME_EVENT_KILL(kPacket);
		}break;
	case PT_M_C_NFY_GAME_END:
		{
			Recv_PT_M_C_NFY_GAME_END(kPacket);
		}break;
	case PT_M_C_NFY_GAME_RESULT:
		{
			Recv_PT_M_C_NFY_GAME_RESULT(kPacket);
		}break;
	case PT_T_C_NFY_RELOAD_ROOM:
		{
			Recv_PT_T_C_NFY_RELOAD_ROOM(kPacket);
		}break;
	case PT_T_C_ANS_SLOTSTATUS_CHANGE:
		{
			Recv_PT_T_C_ANS_SLOTSTATUS_CHANGE(kPacket);
		}break;
	case PT_T_C_ANS_ENTRUST_MASTER:
		{
			BM::GUID kNewMasterGuid;
			BM::GUID kOldMasterGuid;
			kPacket.Pop( kNewMasterGuid );
			kPacket.Pop( kOldMasterGuid );
			SetChangeMaster( kNewMasterGuid, kOldMasterGuid );
		}break;
	case PT_T_C_NFY_PVPLOBBY_LIST_EVENT:
		{
			UpdateLobbyList( kPacket );
		}break;
	case PT_T_C_ANS_GET_PVPRANKING:
		{
			m_kRankIngMgr.ReadFromPacket( kPacket );

			BM::GUID kMyCharGuid;
			g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
			m_iMyRank = m_kRankIngMgr.GetRank(kMyCharGuid);

			UpdateRankingUI( XUIMgr.Get( ms_SFRM_PVP_RANKING ), m_kRankIngMgr );
			MyInfoRefresh( NULL );
		}break;
	case PT_T_C_NFY_PVPRANK:
		{
			kPacket.Pop( m_iMyRank );
			MyInfoRefresh( NULL );
		}break;
	case PT_T_C_ANS_INVATE_FAIL:
		{
			std::wstring kPlayerName;
			kPacket.Pop(kPlayerName);

			std::wstring wstrMsg;
			::WstringFormat( wstrMsg, MAX_PATH, TTW(200129).c_str(), kPlayerName.c_str() );
			if ( !wstrMsg.empty() )
			{
				Notice_Show( wstrMsg, EL_PvPMsg );
			}
		}break;
	case PT_T_C_ANS_JOIN_LEAGUE_LOBBY:
		{
			Recv_PT_T_C_ANS_JOIN_LEAGUE_LOBBY( kPacket );
		}break;
	case PT_T_C_RELOAD_LEAGUE_LOBBY:
		{
			Recv_PT_T_C_RELOAD_LEAGUE_LOBBY( kPacket );
		}break;
	case PT_T_C_ANS_ENTRY_CHANGE:
		{
			Recv_PT_T_C_ANS_ENTRY_CHANGE( kPacket );
		}break;
	case PT_M_C_NFY_ADD_POINT_USER:
		{
			Recv_PT_M_C_NFY_ADD_POINT_USER( kPacket );
		}break;
	case PT_M_C_NFY_OCCUPY_POINT_SECTION_INFO:
		{
			Reve_PT_M_C_NFY_OCCUPY_POINT_SECTION_INFO( kPacket );
		}break;
	case PT_M_C_NFY_KOH_INSERT_GUARDIAN:
		{
			Recv_PT_M_C_NFY_KOH_INSERT_GUARDIAN( kPacket );
		}break;
	case PT_M_C_NFY_UPDATE_LOVE_FENCE:
		{
			Recv_PT_M_C_NFY_UPDATE_LOVE_FENCE( kPacket );
		}break;
	case PT_M_C_NFY_DISPLAY_DROPBEAR_TIMER:
		{
			Recv_PT_M_C_NFY_DISPLAY_DROPBEAR_TIMER( kPacket );
		}break;
	case PT_M_C_NFY_FOCUS_TOUCH_DOWN_USER:
		{
			Recv_PT_M_C_NFY_FOCUS_TOUCH_DOWN_USER( kPacket );
		}break;
	default:
		{
			return PgContentsBase::ProcessPacket( wType, kPacket );
		}
	}
	return true;
}

void PgPvPGame::RecvSyncTeamPoint( BM::Stream& kPacket, bool const bReady )
{
	m_kContTeamResult.clear();
	PU::TLoadTable_MM( kPacket, m_kContTeamResult );

	int iIndex = 0;
	CONT_PVP_TEAM_RESULT::const_iterator rst_itr;
	for ( rst_itr = m_kContTeamResult.begin(); rst_itr != m_kContTeamResult.end(); ++rst_itr )
	{
		this->SyncPoint( rst_itr->second.iPoint, rst_itr->first );
	}
}

void PgPvPGame::Recv_PT_T_C_ANS_JOIN_LEAGUE_LOBBY( BM::Stream &kPacket )
{
	kPacket.Pop( m_iMyRank );

	int iLobbyID = 0;
	kPacket.Pop( iLobbyID );
	m_iLobbyID = iLobbyID;
	
	bool bKick = false;
	kPacket.Pop( bKick );

	m_kContRoom.clear();
	PU::TLoadTable_AM( kPacket, m_kContRoom );

	lwCloseAllUI(true);
	SetStatus( STATUS_LOBBY );

	m_kContDefPvPLeagueTime.clear();
	PU::TLoadArray_A(kPacket, m_kContDefPvPLeagueTime);

	m_kLeagueInfo.Clear();
	m_kLeagueInfo.ReadFromPacket(kPacket);

	m_kTournamentInfo.Clear();
	m_kTournamentInfo.ReadFromPacket(kPacket);

	m_kContTeam.clear();
	PU::TLoadTable_AM(kPacket, m_kContTeam);

	m_kContBattle.clear();
	PU::TLoadTable_AM(kPacket, m_kContBattle);

	RefreshLeagueLobby();

	if ( bKick )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400343, true);	// 추방당하셨습니다.
	}
}

void PgPvPGame::Recv_PT_T_C_ANS_JOIN_LOBBY( BM::Stream &kPacket )
{
	kPacket.Pop( m_iMyRank );

	int iLobbyID = 0;
	kPacket.Pop( iLobbyID );

	XUI::CXUI_List2::CExport kExport[TAB_ALLNUM];
	bool bExport = false;

	if ( m_iLobbyID == iLobbyID )
	{
		for(int i=0;i<TAB_ALLNUM;++i)
		{
			XUI::CXUI_List2 *pList = GetLobbyTabList(i);
			if ( pList )
			{
				pList->Export( kExport[i] );
				bExport = true;
			}
		}
	}
	m_iLobbyID = iLobbyID;

	bool bKick = false;
	kPacket.Pop( bKick );

	lwCloseAllUI(true);
	SetStatus( STATUS_LOBBY );

	m_kContRoom.clear();
	PU::TLoadTable_AM( kPacket, m_kContRoom );

	RefreshRoomList();

	if ( bExport )
	{
		for(int i=0;i<TAB_ALLNUM;++i)
		{
			XUI::CXUI_List2 *pList = GetLobbyTabList(i);
			if ( pList )
			{
				pList->Import( kExport[i] );
			}
		}
	}

	if ( bKick )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400343, true);	// 추방당하셨습니다.
	}
}

void PgPvPGame::Recv_PT_T_C_NFY_ROOMLIST_REMOVE( BM::Stream &kPacket )
{
	if ( m_eStatus == STATUS_LOBBY )
	{
		CONT_PVPROOM_LIST::key_type iRoomNo = 0;
		kPacket.Pop( iRoomNo );
		m_kContRoom.erase( iRoomNo );
		RefreshRoomList();
	}
}
void PgPvPGame::Recv_PT_T_C_NFY_REFRESH_LEAGUE_LOBBY( BM::Stream &kPacket )
{
	if ( m_eStatus == STATUS_LOBBY )
	{
		size_t iSize = 0;
		if ( kPacket.Pop( iSize ) )
		{
			CONT_PVPROOM_LIST::key_type iRoomKey = 0;
			CONT_PVPROOM_LIST::mapped_type kElement;

			while ( iSize-- )
			{
				kPacket.Pop( iRoomKey );
				kElement.ReadFromPacket(kPacket);

				if ( kElement.IsColsed() )
				{
					m_kContRoom.erase( iRoomKey );
				}
				else
				{
					std::pair<CONT_PVPROOM_LIST::iterator, bool> kPair = m_kContRoom.insert( std::make_pair( iRoomKey, kElement ) );
					if ( !kPair.second )
					{
						kPair.first->second = kElement;
					}
				}
			}
			RefreshLeagueLobby();
		}	
	}
}

void PgPvPGame::Recv_PT_T_C_RELOAD_LEAGUE_LOBBY( BM::Stream &kPacket )
{
	lwCloseAllUI(true);

	m_kContRoom.clear();
	PU::TLoadTable_AM( kPacket, m_kContRoom );

	m_kContDefPvPLeagueTime.clear();
	PU::TLoadArray_A(kPacket, m_kContDefPvPLeagueTime);

	m_kLeagueInfo.Clear();
	m_kLeagueInfo.ReadFromPacket(kPacket);

	m_kTournamentInfo.Clear();
	m_kTournamentInfo.ReadFromPacket(kPacket);

	m_kContTeam.clear();
	PU::TLoadTable_AM(kPacket, m_kContTeam);

	m_kContBattle.clear();
	PU::TLoadTable_AM(kPacket, m_kContBattle);

	RefreshLeagueLobby();
}

void PgPvPGame::Recv_PT_T_C_NFY_ROOMLIST_UPDATE( BM::Stream &kPacket )
{
	if ( m_eStatus == STATUS_LOBBY )
	{
		size_t iSize = 0;
		if ( kPacket.Pop( iSize ) )
		{
			CONT_PVPROOM_LIST::key_type iRoomKey = 0;
			CONT_PVPROOM_LIST::mapped_type kElement;

			while ( iSize-- )
			{
				kPacket.Pop( iRoomKey );
				kElement.ReadFromPacket(kPacket);

				if ( kElement.IsColsed() )
				{
					m_kContRoom.erase( iRoomKey );
				}
				else
				{
					std::pair<CONT_PVPROOM_LIST::iterator, bool> kPair = m_kContRoom.insert( std::make_pair( iRoomKey, kElement ) );
					if ( !kPair.second )
					{
						kPair.first->second = kElement;
					}
				}
			}
			RefreshRoomList();
		}	
	}
}

void PgPvPGame::BuildStrongholdLink()
{
	if( !g_pkWorld )
	{
		return;
	}

	SPvPStrongholdLinkInfo LinkInfo;
	CONT_TRIGGER_INFO ContTrigger;

	//월드에서 점령지 속성을 가진 트리거를 모두 찾는다.
	g_pkWorld->GetTriggerByType( PgTrigger::TRIGGER_TYPE_KING_OF_HILL, ContTrigger);
	if( ContTrigger.empty() )
	{
		return;
	}

	CONT_TRIGGER_INFO::const_iterator TrgIter;
	for( TrgIter = ContTrigger.begin(); TrgIter != ContTrigger.end(); ++TrgIter )
	{
		if( (*TrgIter) )
		{
			std::vector<std::string> Link = (*TrgIter)->m_ContEntityLinkName;
			std::vector<std::string>::const_iterator LinkIter;
			for( LinkIter = Link.begin(); LinkIter != Link.end(); ++LinkIter )
			{//링크 정보가 있으면 데이터를 순회하면서 PVPGAME의 링크 컨테이너에 삽입한다.
				LinkInfo.LinkTeam = TEAM_NONE;				// 중립
				LinkInfo.LTrigger = (*TrgIter)->GetID();	// 링크를 구성하는 트리거 1
				LinkInfo.RTrigger = *LinkIter;				// 링크를 구성하는 트리거 2

				if( m_ContLink.empty() )
				{//링크 컨테이너가 비었으면 바로 넣는다.
					m_ContLink.push_back(LinkInfo);
				}
				else
				{//데이터가 있으면 데이터 중복검사를 하고 넣는다.
					bool CheckOverlap = false;
					CONT_STRONGHOLD_LINK::const_iterator Iter;
					for( Iter = m_ContLink.begin(); Iter != m_ContLink.end(); ++Iter )
					{
						if( (*Iter) == LinkInfo )
						{//데이터가 동일하면 중복이므로 패스
							CheckOverlap = true;
							break;
						}
						if( Iter->RTrigger == LinkInfo.LTrigger
							&& Iter->LTrigger == LinkInfo.RTrigger )
						{//동일한 데이터가 순서만 바뀐 것도 패스
							CheckOverlap = true;
							break;
						}
					}
					if( false == CheckOverlap )
					{
						m_ContLink.push_back(LinkInfo);
					}
				}
			}
		}
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_ADD_POINT_USER( BM::Stream &kPacket )
{
	if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
	{//점령전일 때만
		ETeam eTeam = TEAM_NONE;
		int iPoint = 0;
		kPacket.Pop( eTeam );
		kPacket.Pop( iPoint );

		lua_tinker::call<void, int, int>("OnUpdate_Team_TotalPoint", eTeam, iPoint); // 전체 점수 갱신
	}
}

void PgPvPGame::Reve_PT_M_C_NFY_OCCUPY_POINT_SECTION_INFO( BM::Stream &kPacket )
{
	if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
	{//점령전일 때만
		int LvUpPoint = 0;
		kPacket.Pop( LvUpPoint );
		m_LevelUpPointInfo.LvUpPoint.insert( std::make_pair( 1, LvUpPoint ) );
		kPacket.Pop( LvUpPoint );
		m_LevelUpPointInfo.LvUpPoint.insert( std::make_pair( 2, LvUpPoint ) );
		kPacket.Pop( LvUpPoint );
		m_LevelUpPointInfo.LvUpPoint.insert( std::make_pair( 3, LvUpPoint ) );
		kPacket.Pop( LvUpPoint );
		m_LevelUpPointInfo.LvUpPoint.insert( std::make_pair( 4, LvUpPoint ) );

		BuildStrongholdLink();
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_KOH_INSERT_GUARDIAN( BM::Stream &kPacket )
{
	if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
	{//점령전일 때만
		std::string TriggerID;
		BM::GUID EntityGuid;
		int EntityTeam = 0;

		kPacket.Pop(TriggerID);
		kPacket.Pop(EntityGuid);
		kPacket.Pop(EntityTeam);

		if( TriggerID.empty()
			|| BM::GUID::NullData() == EntityGuid )
		{// 트리거 이름이 비어있거나 엔티티 GUID가 없으면 종료
			return;
		}

		if( m_ContTriggerEntity.empty() )
		{
			m_ContTriggerEntity.insert( std::make_pair( TriggerID, EntityGuid ) );
		}
		else
		{
			CONT_PVP_TRIGGER_ENTITY::iterator iter = m_ContTriggerEntity.find( TriggerID );
			if( m_ContTriggerEntity.end() != iter )
			{
				iter->second = EntityGuid;
			}
			else
			{
				m_ContTriggerEntity.insert( std::make_pair( TriggerID, EntityGuid ) );
			}
		}

		if( g_pkWorld )
		{//점령지 파티클 관련 부분
			if( TEAM_NONE != EntityTeam )
			{//가디언이 팀이 있으면 파티클을 붙여야 한다.
				PgTrigger * pTrigger = g_pkWorld->GetTriggerByID(TriggerID);
				if( pTrigger )
				{//파티클을 설치할 좌표는 트리거에서 가져온다. (가디언과 트리거의 좌표가 같으므로)
					CONT_PVP_TRIGGER_PARTICLE::const_iterator iter = m_ContTriggerParticle.find(pTrigger->GetID());
					if( iter == m_ContTriggerParticle.end() )
					{//이미 파티클이 적용되어 있는지 검사하고 없으면 적용시킨다.
						PgParticle * pParticle = NULL;
						if( TEAM_RED == EntityTeam )
						{//팀 별로 다른 파티클을 적용한다.
							pParticle = g_kParticleMan.GetParticle(EFX_DOMINATION_RANGE_01_RED);
						}
						else if( TEAM_BLUE == EntityTeam )
						{
							pParticle = g_kParticleMan.GetParticle(EFX_DOMINATION_RANGE_01_BLUE);
						}
						if( pParticle )
						{//파티클이 존재하면
							int const Slot = g_pkWorld->AttachParticle(pParticle, pTrigger->GetTriggerObjectPos());
							if( -1 == Slot )
							{//실패 하면 Clone된 파티클을 삭제해주고
								THREAD_DELETE_PARTICLE(pParticle);
							}
							else
							{//성공하면 다음 검사를 위해 '트리거 이름'과 '슬롯 번호'를 저장해둔다.
								m_ContTriggerParticle.insert( std::make_pair( pTrigger->GetID(), Slot ) );
							}
						}
					}
				}
			}
			else
			{// 팀이 없으면 파티클을 해제한다.
				PgTrigger * pTrigger = g_pkWorld->GetTriggerByID(TriggerID);
				if( pTrigger )
				{
					CONT_PVP_TRIGGER_PARTICLE::iterator iter = m_ContTriggerParticle.find(pTrigger->GetID());
					if( m_ContTriggerParticle.end() != iter )
					{
						g_pkWorld->DetachParticle( iter->second );
						m_ContTriggerParticle.erase(iter);
					}
				}
			}
		}
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_UPDATE_LOVE_FENCE( BM::Stream &kPacket )
{
	if( PVP_TYPE_LOVE == m_kRoomBaseInfo.m_kType )
	{//러브러브전일 때만
		int ObjectNo = 0;
		bool bFenceObjectLife = false;
		kPacket.Pop(ObjectNo);
		kPacket.Pop(bFenceObjectLife);

		CONT_TRIGGER_INFO ContTrigger;
		if( NULL != g_pkWorld )
		{
			g_pkWorld->GetTriggerByType( PgTrigger::TRIGGER_TYPE_LOVE_FENCE, ContTrigger);
		}
		if( ContTrigger.empty() )
		{// 월드에서 울타리 속성을 가진 트리거를 찾는다.
			return;
		}
		CONT_TRIGGER_INFO::const_iterator TrgIter;
		for( TrgIter = ContTrigger.begin(); TrgIter != ContTrigger.end(); ++TrgIter )
		{
			if( (*TrgIter) )
			{
				if( (*TrgIter)->ObjectNo() == ObjectNo )
				{// 이름으로 변경된 트리거를 찾아서 값을 갱신시켜준다.
					(*TrgIter)->LoveFenceLife(bFenceObjectLife);
					return;
				}
			}
		}
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_DISPLAY_DROPBEAR_TIMER( BM::Stream &kPacket )
{
	int State = 0;
	ETeam BearTeam = TEAM_NONE;
	WORD RemainTime = 0;
	kPacket.Pop(State);
	kPacket.Pop(BearTeam);
	kPacket.Pop(RemainTime);

	RemainTime = RemainTime / 1000;

	switch( State )
	{
	case 0:
		{// 곰이 본진으로 돌아감
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				pPlayer->BearEffect(false);
			}
			switch( BearTeam )
			{
			case TEAM_RED:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 1, -1, false);
				}break;
			case TEAM_BLUE:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 2, -1, false);
				}break;
			default:
				break;
			}
		}break;
	case 1:
		{// 곰이 필드에 드랍 됨
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				pPlayer->BearEffect(false);
			}
			XUIMgr.Close(L"BTN_EMERGENCY_ESCAPE");	// 긴급회피 스킬 UI 닫는다
			switch( BearTeam )
			{
			case TEAM_RED:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 1, RemainTime, true);
				}break;
			case TEAM_BLUE:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 2, RemainTime, true);
				}break;
			default:
				break;
			}
		}break;
	case 2: // 드랍 된 곰을 누가 주웠음
		{
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				int BearEffect = pPlayer->GetAbil(AT_BEAR_EFFECT_RED);
				BearEffect += pPlayer->GetAbil(AT_BEAR_EFFECT_BLUE);
				if( BearEffect )
				{ // 자신이 곰을 업은 사람일 경우에는 긴급회피 스킬 UI 연다
					XUIMgr.Activate(L"BTN_EMERGENCY_ESCAPE");
					pPlayer->BearEffect(true);
				}
			}
			switch( BearTeam )
			{
			case TEAM_RED:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 1, RemainTime, false);
				}break;
			case TEAM_BLUE:
				{
					lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 2, RemainTime, false);
				}break;
			default:
				break;
			}
		}break;
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_FOCUS_TOUCH_DOWN_USER( BM::Stream & Packet )
{
	if( NULL == g_pkWorld )
	{
		return;
	}
	bool bFocusCamera = false;
	Packet.Pop(bFocusCamera);
	if( bFocusCamera )
	{
		BM::GUID FocusCharacterGuid;
		Packet.Pop(FocusCharacterGuid);

		PgActor * pActor = g_kPilotMan.FindActor(FocusCharacterGuid);
		if( pActor )
		{
			g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pActor);
		}
	}
	else
	{
		PgActor * pMyActor = g_kPilotMan.GetPlayerActor();
		if( pMyActor )
		{
			g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pMyActor);
		}
	}
}

void PgPvPGame::InitRoomListUI(XUI::CXUI_Wnd *pkWndRoomList)
{
	if( m_kContBattle.empty() || m_kContRoom.empty() )
	{//전부 안보이게
		for( size_t iRoomIndex=1; iRoomIndex<=15; ++iRoomIndex )
		{
			BM::vstring vstrRoomItem( _T("FRM_ROOMITEM_") );
			vstrRoomItem += iRoomIndex;
			XUI::CXUI_Wnd *pkWndRoomItem = pkWndRoomList->GetControl( vstrRoomItem.operator const std::wstring &() );
			if ( pkWndRoomItem )
			{
				pkWndRoomItem->Visible(false);
			}
		}
		return;
	}

	for( size_t iRoomIndex=1; iRoomIndex<=15; ++iRoomIndex )
	{
		BM::vstring vstrRoomItem( _T("FRM_ROOMITEM_") );
		vstrRoomItem += iRoomIndex;
		XUI::CXUI_Wnd *pkWndRoomItem = pkWndRoomList->GetControl( vstrRoomItem.operator const std::wstring &() );
		if ( pkWndRoomItem )
		{
			int const iIndexToPage = (m_kPage.iNowPage*15)+iRoomIndex;
			CONT_PVPROOM_LIST::const_iterator room_itr = m_kContRoom.find(iIndexToPage);
			if( m_kContRoom.end() != room_itr )
			{
				pkWndRoomItem->Visible(true);

				XUI::CXUI_Wnd *pkTemp = NULL;
				wchar_t wszTemp[MAX_PATH] = {0,};

				// 방번호
				pkTemp = pkWndRoomItem->GetControl( _T("FRM_ROOM_NO") );
				if ( pkTemp )
				{
					swprintf_s( wszTemp, MAX_PATH, L"%d", room_itr->first );
					pkTemp->Text(wszTemp);
				}	

				pkTemp = pkWndRoomItem->GetControl(L"FRM_TEAM_NAME");
				if( pkTemp )
				{
					TBL_DEF_PVPLEAGUE_BATTLE kBattleInfo;
					if( GetBattleInfo(room_itr->second.m_kBattleGuid, kBattleInfo) )
					{
						TBL_DEF_PVPLEAGUE_TEAM kTeamInfo1, kTeamInfo2;
						if( GetTeamInfo(kBattleInfo.kTeamGuid1, kTeamInfo1) && GetTeamInfo(kBattleInfo.kTeamGuid2, kTeamInfo2) )
						{
							std::wstring wstrTeamName = kTeamInfo1.wstrTeamName;
							wstrTeamName += L" : ";
							wstrTeamName += kTeamInfo2.wstrTeamName;
							pkTemp->Text(wstrTeamName);
						}
					}
				}

				// 인원
				pkTemp = pkWndRoomItem->GetControl( _T("SFRM_USER_COUNT") );
				if ( pkTemp )
				{
					swprintf_s (wszTemp, MAX_PATH, L"%u / %u", room_itr->second.m_ucNowUser, room_itr->second.m_ucMaxUser );
					pkTemp->Text(wszTemp);
				}

				//레벨
				pkTemp = pkWndRoomItem->GetControl( _T("SFRM_LEVEL") );
				if ( pkTemp )
				{
					std::wstring wstrMsg;
					if( room_itr->second.IsHaveLimitLevel() )
					{
						WstringFormat( wstrMsg, MAX_PATH, TTW(450257).c_str(), room_itr->second.m_sLevelLimit_Min, room_itr->second.m_sLevelLimit_Max );
					}
					else
					{
						wstrMsg = TTW(3331);
					}
					pkTemp->Text(wstrMsg);
				}

				//Type
				pkTemp = pkWndRoomItem->GetControl( _T("SFRM_MODE") );
				if ( pkTemp )
				{
					pkTemp->Text( TTW(lua_tinker::call<int,EPVPTYPE>("GetPvPTypeTextTable", room_itr->second.m_kType) ) );	
				}

				// State
				pkTemp = pkWndRoomItem->GetControl( _T("SFRM_STATE") );
				if ( pkTemp )
				{
					if ( room_itr->second.m_kStatus == ROOM_STATUS_LOBBY )
					{
						pkTemp->Text(TTW(200001));
					}
					else
					{
						pkTemp->Text(TTW(200002));
					}
				}
			}
			else//if( m_kContRoom.end() == room_itr )
			{
				pkWndRoomItem->Visible(false);
			}
		}
	}
}

void PgPvPGame::Set_PvPLeagueLevelText(int const iLeagueLevel, BM::vstring &rkOutText)
{
	if( MAX_LEAGUE_LEVEL+1 == iLeagueLevel )
	{//우승
		rkOutText += ::TTW(560075);
	}
	else if( MAX_LEAGUE_LEVEL == iLeagueLevel )
	{//결승
		rkOutText += ::TTW(560067);
	}
	else if( MAX_LEAGUE_LEVEL == iLeagueLevel+1 )
	{//준결승
		rkOutText += ::TTW(560066);
	}
	else
	{//x강
		int const iResultLevel = ::pow(2.0f, static_cast<long>(MAX_LEAGUE_LEVEL - iLeagueLevel+1));
		rkOutText += ::TTW(560061);
		rkOutText.Replace(L"#LEAGUE_LEVEL#",iResultLevel);
	}
}

void PgPvPGame::InitTournamentUI(XUI::CXUI_Wnd *pkWndTournament)
{
	XUI::CXUI_Wnd *pkWndProgress = pkWndTournament->GetControl(L"SFRM_PROGRESS_MAIN");
	if( !pkWndProgress )
	{
		return;
	}

	XUI::CXUI_Wnd *pkWndToday = pkWndProgress->GetControl(L"FRM_TODAY_TEXT");
	if( pkWndToday )
	{
		if( m_kTournamentInfo.iLeagueLevel )
		{
			BM::vstring vstrTodayProcess(L"[");
			Set_PvPLeagueLevelText(m_kTournamentInfo.iLeagueLevel, vstrTodayProcess);
			vstrTodayProcess += L"]";
			vstrTodayProcess += ::TTW(560052);
			pkWndToday->Text(vstrTodayProcess);
		}
		else
		{
			pkWndToday->Text(TTW(560032));
		}
	}

	//Init LevelBox Info
	for(int iLevel=1; iLevel<=MAX_LEAGUE_LEVEL; ++iLevel)
	{
		BM::vstring vstrBoxName(L"IMG_BOX_");
		vstrBoxName += iLevel;
		XUI::CXUI_Wnd *pkWndBox = pkWndProgress->GetControl(vstrBoxName.operator const std::wstring &());
		if( pkWndBox )
		{
			int const iResultLevel = ::pow(2.0f, static_cast<long>(MAX_LEAGUE_LEVEL - (iLevel - 1)));
			BM::vstring vstrLevel, vstrBeginTime;
			DWORD dwLevelFontColor = 0, dwTimeFontColor = 0;
			if( m_kTournamentInfo.iLeagueLevel == iLevel )
			{//진행중인 레벨은 활성화
				pkWndBox->UVUpdate(2);
				dwLevelFontColor = 0xFFFFFF00;
				dwTimeFontColor = 0xFFFFFF00;
			}
			else
			{
				pkWndBox->UVUpdate(1);
				if( m_kTournamentInfo.iLeagueLevel > iLevel )
				{//지난거
					dwLevelFontColor = 0xFFA0A0A0;
					dwTimeFontColor = 0xFFFFFFFF;
				}
				else
				{//앞으로
					dwLevelFontColor = 0xFFA0A0A0;
					dwTimeFontColor = 0xFF8CF8F7;
				}
			}

			XUI::CXUI_Wnd *pkWndLevel = pkWndBox->GetControl(L"FRM_LEVEL_TEXT");
			if( pkWndLevel )
			{
				Set_PvPLeagueLevelText(iLevel, vstrLevel);
				pkWndLevel->FontColor(dwLevelFontColor);
				pkWndLevel->OutLineColor(0xFF000000);
				pkWndLevel->Text(vstrLevel);
			}

			XUI::CXUI_Wnd *pkWndTime = pkWndBox->GetControl(L"FRM_TIME_TEXT");
			if( pkWndTime )
			{
				SYSTEMTIME kTime;
				TBL_DEF_PVPLEAGUE_TIME kTimeInfo;
				if( GetLeagueBeginTime(iLevel, kTimeInfo) 
					&& CGameTime::SecTime2SystemTime(kTimeInfo.i64BeginTime, kTime, CGameTime::DEFAULT) )
				{
					BM::vstring vstrHour, vstrMinute;
					vstrHour = kTime.wHour;
					vstrMinute = kTime.wMinute;
					if( 0 == kTime.wHour )
					{
						vstrHour += L"0";
					}
					if( 0 == kTime.wMinute )
					{
						vstrMinute += L"0";
					}
					vstrBeginTime += ::TTW(11000 + kTimeInfo.iDayOfWeek);
					vstrBeginTime += L" ";
					vstrBeginTime += vstrHour;
					vstrBeginTime += L":";
					vstrBeginTime += vstrMinute;
					pkWndTime->FontColor(dwTimeFontColor);
					pkWndTime->OutLineColor(0xFF000000);
					pkWndTime->Text(vstrBeginTime);
				}
			}
		}
	}

	//Init Process Arrow
	for(int iLevel=1; iLevel<MAX_LEAGUE_LEVEL; ++iLevel)
	{
		BM::vstring vstrArrowName(L"IMG_ARROW_");
		vstrArrowName += iLevel;
		XUI::CXUI_Wnd *pkWndArrow = pkWndProgress->GetControl(vstrArrowName.operator const std::wstring &());
		if( pkWndArrow )
		{
			if( m_kTournamentInfo.iLeagueLevel > iLevel )
			{//진행중인 레벨까지 화살표 활성화
				pkWndArrow->UVUpdate(2);
			}
			else
			{
				pkWndArrow->UVUpdate(1);
			}
		}
	}

	//Init GroupInfo
	if( m_kContGroup.empty() )
	{
		return;
	}
	
	for(int iGroupIndex=0; iGroupIndex<6; ++iGroupIndex)
	{
		BM::vstring vstrGroup(L"FRM_GROUP_");
		vstrGroup += (iGroupIndex+1);
		int const iIndexToPage = m_kPage.iNowPage*6;
		XUI::CXUI_Wnd *pkWndGroup = pkWndTournament->GetControl(vstrGroup.operator const std::wstring &());
		CONT_BATTLE_GROUP::const_iterator group_itr = m_kContGroup.find(iGroupIndex+iIndexToPage+1);
		if( pkWndGroup && m_kContGroup.end() != group_itr )
		{
			pkWndGroup->Visible(true);
			CONT_DEF_PVPLEAGUE_BATTLE const &kContBattle = group_itr->second;
			CONT_DEF_PVPLEAGUE_BATTLE::const_iterator battle_itr = kContBattle.begin();
			int iIndexUI = 0;
			while( kContBattle.end() != battle_itr )
			{
				CONT_DEF_PVPLEAGUE_TEAM::const_iterator team_itr = m_kContTeam.find(battle_itr->second.kTeamGuid1);
				if( m_kContTeam.end() != team_itr )
				{
					TBL_DEF_PVPLEAGUE_TEAM const &kTeamInfo = team_itr->second;
					InitGroupUI(pkWndGroup, iIndexUI, kTeamInfo);
				}
				else
				{
					ClearGroupUI(pkWndGroup, iIndexUI);
				}
				++iIndexUI;

				team_itr = m_kContTeam.find(battle_itr->second.kTeamGuid2);
				if( m_kContTeam.end() != team_itr )
				{
					TBL_DEF_PVPLEAGUE_TEAM const &kTeamInfo = team_itr->second;
					InitGroupUI(pkWndGroup, iIndexUI, kTeamInfo);
				}
				else
				{
					ClearGroupUI(pkWndGroup, iIndexUI);
				}

				XUI::CXUI_Wnd *pkWndNextLevel = pkWndGroup->GetControl(L"SFRM_CENTER_ITEM");
				if( pkWndNextLevel )
				{
					BM::vstring vstrNextLevel;
					Set_PvPLeagueLevelText(m_kTournamentInfo.iLeagueLevel+1, vstrNextLevel);
					pkWndNextLevel->Text(vstrNextLevel);
				}
				++iIndexUI;
				++battle_itr;
			}
		}
		else
		{
			pkWndGroup->Visible(false);
		}
	}
}

void PgPvPGame::ClearGroupUI(XUI::CXUI_Wnd *pkWndGroup, int const iIndexUI)
{
	BM::vstring vstrTeam(L"FRM_TEAM_");
	vstrTeam += iIndexUI;
	XUI::CXUI_Wnd *pkWndTeam = pkWndGroup->GetControl(vstrTeam.operator const std::wstring &());
	if( pkWndTeam )
	{
		pkWndTeam->Visible(true);
		XUI::CXUI_Wnd *pkWndRoomNo = pkWndTeam->GetControl(L"FRM_ROOM_NO");
		XUI::CXUI_Wnd *pkWndTeamName = pkWndTeam->GetControl(L"IMG_BOX_ITEM");
		if( pkWndRoomNo && pkWndTeamName )
		{
			BM::vstring vstrText;
			pkWndRoomNo->Text(vstrText);
			pkWndTeamName->Text(vstrText);
		}
	}
}

void PgPvPGame::InitGroupUI(XUI::CXUI_Wnd *pkWndGroup, int const iIndexUI, TBL_DEF_PVPLEAGUE_TEAM const &rkTeamInfo)
{
	BM::vstring vstrTeam(L"FRM_TEAM_");
	vstrTeam += iIndexUI;
	XUI::CXUI_Wnd *pkWndTeam = pkWndGroup->GetControl(vstrTeam.operator const std::wstring &());
	if( pkWndTeam )
	{
		pkWndTeam->Visible(true);
		XUI::CXUI_Wnd *pkWndRoomNo = pkWndTeam->GetControl(L"FRM_ROOM_NO");
		XUI::CXUI_Wnd *pkWndTeamName = pkWndTeam->GetControl(L"IMG_BOX_ITEM");
		if( pkWndRoomNo && pkWndTeamName )
		{
			BM::vstring vstrText;
			vstrText = rkTeamInfo.iTournamentIndex;
			pkWndRoomNo->Text(vstrText);

			vstrText = rkTeamInfo.wstrTeamName;
			pkWndTeamName->Text(vstrText);
		}
	}
}

void PgPvPGame::InitTeamListUI(XUI::CXUI_Wnd *pkWndTeamList)
{
	if( m_kContTeam.empty() )
	{
		return;
	}

	int const iIndexBegin = (m_kPage.iNowPage*MAX_TEAMELEMENT_COUNT);
	for(int iIndex=1; iIndex<=MAX_TEAMELEMENT_COUNT; ++iIndex)
	{
		BM::vstring vstrBattle(L"FRM_ROOMITEM_");
		vstrBattle += iIndex;
		XUI::CXUI_Wnd *pkWndTeam = pkWndTeamList->GetControl(vstrBattle.operator const std::wstring &());
		if( pkWndTeam )
		{
			TBL_DEF_PVPLEAGUE_TEAM kTeamInfo;
			if( GetTeamInfo(iIndexBegin+iIndex, kTeamInfo) )
			{
				pkWndTeam->Visible(true);
				BM::vstring vstrText;
				XUI::CXUI_Wnd *pkWndTeamNo = pkWndTeam->GetControl(L"FRM_TEAM_NO");
				if( pkWndTeamNo )
				{
					vstrText = kTeamInfo.iTournamentIndex;
					pkWndTeamNo->Text(vstrText);
				}

				XUI::CXUI_Wnd *pkWndTeamName = pkWndTeam->GetControl(L"FRM_TEAM_NAME");
				if( pkWndTeamName )
				{
					pkWndTeamName->Text(kTeamInfo.wstrTeamName);
				}

				int iUserIndex = 1;
				CONT_LEAGUE_USER::const_iterator user_itr = kTeamInfo.kContUserGuid.begin();
				while(kTeamInfo.kContUserGuid.end() != user_itr)
				{
					vstrText = L"SFRM_USER_NAME_";
					vstrText += iUserIndex;
					XUI::CXUI_Wnd *pkWndUserName = pkWndTeam->GetControl(vstrText.operator const std::wstring &());
					if( pkWndUserName )
					{//아마따.. 이건 이름인데
						pkWndUserName->Text( user_itr->wstrName );
					}
					++iUserIndex;
					++user_itr;
				}
			}
			else
			{
				pkWndTeam->Visible(false);
			}
		}
	}
}

void PgPvPGame::RefreshLeagueLobby()
{
	if ( m_eStatus != STATUS_LOBBY )
	{
		return;
	}

	InitTeamInfo();
	InitGroupInfo();

	XUIMgr.Activate(ms_FRM_PVP_BACKGROUND);
	XUI::CXUI_Wnd* pkWndMain = XUIMgr.Activate(ms_FRM_PVP_LEAGUE);
	XUIMgr.BlockGlobalScript(true);

	if ( pkWndMain )
	{
		int iOldLobbyID = 0;
		pkWndMain->GetCustomData( &iOldLobbyID, sizeof(iOldLobbyID) );
		pkWndMain->SetCustomData( &m_iLobbyID, sizeof(m_iLobbyID) );

		wchar_t wszTemp[MAX_PATH] = {0,};
		XUI::CXUI_Wnd *pkWnd = NULL;
		XUI::CXUI_Wnd *pkTemp = NULL;

		CONT_PVPROOM_LIST::key_type iSelectRoomNo = 0;
		bool bFindNo = false;
		XUI::CXUI_Wnd *pkFrameWnd = pkWndMain->GetControl(PgContentsBase::ms_FRM_CLICK_FRAME);
		if ( pkFrameWnd && pkFrameWnd->Visible() )
		{
			pkFrameWnd->GetCustomData( &iSelectRoomNo, sizeof(iSelectRoomNo) );
		}
		
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkMyPlayer )
		{
			MyInfoRefresh( dynamic_cast<SPvPReport*>(pkMyPlayer) );
		}

		if( m_kTournamentInfo.iLeagueLevel )
		{//경기 진행 기간
			if( PVPLS_READY == m_kLeagueInfo.iLeagueState
				|| PVPLS_GAME == m_kLeagueInfo.iLeagueState )
			{//게임중
				XUI::CXUI_Wnd *pkWndRoomList = pkWndMain->GetControl(PgContentsBase::ms_SFRM_MAIN_BG_ROOM_LIST);
				if( pkWndRoomList )
				{
					InitRoomListUI(pkWndRoomList);
					pkWndRoomList->Visible(true);
					Set_PvPLeagueExplainText(pkWndRoomList, m_kTournamentInfo.iLeagueLevel);
				}
			}
			else
			{//게임시간 아님
				XUI::CXUI_Wnd *pkWndTeamList = pkWndMain->GetControl(PgContentsBase::ms_SFRM_MAIN_BG_TEAM_LIST);
				if( pkWndTeamList )
				{
					InitTeamListUI(pkWndTeamList);
					m_iEndPageNo = m_kContTeam.size() / MAX_TEAMELEMENT_COUNT;
					if( m_kContTeam.size() % MAX_TEAMELEMENT_COUNT )
					{
						++m_iEndPageNo;
					}
					Set_PvPLeagueBeginTimeText(pkWndTeamList, m_kTournamentInfo.iLeagueLevel);
				}

				XUI::CXUI_Wnd *pkWndTournament = pkWndMain->GetControl(PgContentsBase::ms_SFRM_MAIN_BG_TOURNAMENT);
				if( pkWndTournament )
				{
					InitTournamentUI(pkWndTournament);
					Set_PvPLeagueBeginTimeText(pkWndTournament, m_kTournamentInfo.iLeagueLevel);
				}

				XUI::CXUI_Wnd *pkWndBtnTab1 = pkWndMain->GetControl(L"BTN_TAB_TEAM_LIST");
				XUI::CXUI_Wnd *pkWndBtnTab2 = pkWndMain->GetControl(L"BTN_TAB_TOURNAMENT");
				if( pkWndBtnTab1 && pkWndBtnTab2 )
				{
					pkWndBtnTab1->Visible(true);
					pkWndBtnTab2->Visible(true);
				}
			}
		}
		else
		{//참가 신청 기간
			XUI::CXUI_Wnd *pkWndExplain = pkWndMain->GetControl(PgContentsBase::ms_SFRM_MAIN_BG_EXPLAIN);
			if( pkWndExplain )
			{
				pkWndExplain->Visible(true);
				Set_PvPLeagueEndTimeText(pkWndExplain);
			}
		}
	}
}

bool PgPvPGame::Set_PvPLeagueExplainText(XUI::CXUI_Wnd * pkWnd, int const iLeagueLevel) const
{
	XUI::CXUI_Wnd *pkWndTodayScedule = pkWnd->GetControl(L"FRM_TODAY_SCEDULE");
	if( !pkWndTodayScedule || 0 >= iLeagueLevel )
	{
		return false;
	}

	BM::vstring vstrTodayScedule( ::TTW(560074) );
	Set_PvPLeagueLevelText(iLeagueLevel, vstrTodayScedule);
	pkWndTodayScedule->Text(vstrTodayScedule);
	return true;
}

bool PgPvPGame::Set_PvPLeagueBeginTimeText(XUI::CXUI_Wnd * pkWnd, int const iLeagueLevel) const
{
	XUI::CXUI_Wnd *pkWndTodayScedule = pkWnd->GetControl(L"FRM_TODAY_SCEDULE");
	if( !pkWndTodayScedule || 0 >= iLeagueLevel )
	{
		return false;
	}
	
	const CONT_DEF_PVPLEAGUE_TIME* pkContDefPvPLeagueTime = NULL;
	g_kTblDataMgr.GetContDef(pkContDefPvPLeagueTime);
	if( pkContDefPvPLeagueTime )
	{
		BM::vstring vstrTodayScedule( ::TTW(560073) );
		SYSTEMTIME kNowDateTime, kBeginTime;
		::GetLocalTime( &kNowDateTime );
		__int64 const i64NowTimeInDay = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );

		CONT_DEF_PVPLEAGUE_TIME::const_iterator time_itor = pkContDefPvPLeagueTime->begin();
		while( pkContDefPvPLeagueTime->end() != time_itor )
		{
			if( 0 != time_itor->i64EndTime && 0 != time_itor->i64BeginTime
				&& time_itor->iLeagueLevel == iLeagueLevel )
			{
				CGameTime::SecTime2SystemTime(time_itor->i64BeginTime, kBeginTime, CGameTime::DEFAULT );
				kBeginTime.wDayOfWeek = time_itor->iDayOfWeek;
				break;
			}
			++time_itor;
		}

		int const iResultLevel = ::pow(2.0f, static_cast<long>(MAX_LEAGUE_LEVEL - (iLeagueLevel - 1)));
		if( MAX_LEAGUE_LEVEL == iLeagueLevel )
		{//결승
			vstrTodayScedule.Replace(L"#LEAGUE_LEVEL#", ::TTW(560067) );
		}
		else if( MAX_LEAGUE_LEVEL == iLeagueLevel +1 )
		{//준결승
			vstrTodayScedule.Replace(L"#LEAGUE_LEVEL#", ::TTW(560066) );
		}
		else
		{//x강
			BM::vstring vstrTemp( ::TTW(560061) );
			vstrTemp.Replace(L"#LEAGUE_LEVEL#", iResultLevel );
			vstrTodayScedule.Replace(L"#LEAGUE_LEVEL#", vstrTemp );
		}
		vstrTodayScedule.Replace(L"#WEEK#", ::TTW(11010+kBeginTime.wDayOfWeek) );
		vstrTodayScedule.Replace(L"#HOUR#", kBeginTime.wHour );
		vstrTodayScedule.Replace(L"#MINUET#", kBeginTime.wMinute );
		pkWndTodayScedule->Text(vstrTodayScedule);
		return true;
	}
	return false;
}

bool PgPvPGame::Set_PvPLeagueEndTimeText(XUI::CXUI_Wnd * pkWnd) const
{
	XUI::CXUI_Wnd *pkWndTodayScedule = pkWnd->GetControl(L"FRM_TODAY_SCEDULE");
	if( !pkWndTodayScedule )
	{
		return false;
	}

	const CONT_DEF_PVPLEAGUE_TIME* pkContDefPvPLeagueTime = NULL;
	g_kTblDataMgr.GetContDef(pkContDefPvPLeagueTime);
	if( pkContDefPvPLeagueTime )
	{
		BM::vstring vstrTodayScedule( ::TTW(560072) );
		SYSTEMTIME kNowDateTime, kEndTime;
		::GetLocalTime( &kNowDateTime );

		CONT_DEF_PVPLEAGUE_TIME::const_iterator time_itor = pkContDefPvPLeagueTime->begin();
		while( pkContDefPvPLeagueTime->end() != time_itor )
		{
			if( PVPLS_JOIN == time_itor->iLeagueState  )
			{
				CGameTime::SecTime2SystemTime(time_itor->i64EndTime, kEndTime, CGameTime::DEFAULT );
				kEndTime.wDayOfWeek = time_itor->iDayOfWeek;
			}
			++time_itor;
		}
		if( 7 < kEndTime.wDayOfWeek )
		{
			return false;
		}

		vstrTodayScedule.Replace(L"#WEEK#", ::TTW(11010+kEndTime.wDayOfWeek) );
		vstrTodayScedule.Replace(L"#HOUR#", kEndTime.wHour );
		vstrTodayScedule.Replace(L"#MINUET#", kEndTime.wMinute );
		pkWndTodayScedule->Text(vstrTodayScedule);
		return true;
	}
	return false;
}

bool PgPvPGame::GetTeamInfo(int const iTeamIndex, TBL_DEF_PVPLEAGUE_TEAM& rkOut) const
{
	if( 0 > iTeamIndex || m_kContIndexToTeam.empty() )
	{
		return false;
	}

	CONT_PVPLEAGUE_INDEX_TO_TEAM::const_iterator teamIndex_itr = m_kContIndexToTeam.find(iTeamIndex);
	if( m_kContIndexToTeam.end() != teamIndex_itr )
	{
		CONT_DEF_PVPLEAGUE_TEAM::const_iterator team_itr = m_kContTeam.find(teamIndex_itr->second);
		if( m_kContTeam.end() != team_itr )
		{
			rkOut = team_itr->second;
			return true;
		}	
	}

	return false;
}

bool PgPvPGame::GetTeamInfo(BM::GUID const & rkTeamGuid, TBL_DEF_PVPLEAGUE_TEAM& rkOut) const
{
	
	CONT_DEF_PVPLEAGUE_TEAM::const_iterator team_itr = m_kContTeam.find(rkTeamGuid);
	if( m_kContTeam.end() != team_itr )
	{
		rkOut = team_itr->second;
		return true;
	}
	return false;
}

bool PgPvPGame::GetBattleInfo(BM::GUID const & rkBattleGuid, TBL_DEF_PVPLEAGUE_BATTLE& rkOut) const
{
	CONT_DEF_PVPLEAGUE_BATTLE::const_iterator battle_itr = m_kContBattle.find(rkBattleGuid);
	if( m_kContBattle.end() != battle_itr )
	{
		rkOut = battle_itr->second;
		return true;
	}
	return false;
}


void PgPvPGame::InitTeamInfo()
{
	if( m_kContTeam.empty() )
	{
		return;
	}

	m_kContCharToTeam.clear();
	m_kContIndexToTeam.clear();

	CONT_DEF_PVPLEAGUE_TEAM::const_iterator team_itr = m_kContTeam.begin();
	while( m_kContTeam.end() != team_itr )
	{
		CONT_DEF_PVPLEAGUE_TEAM::mapped_type const& kElement = team_itr->second;
		CONT_LEAGUE_USER::const_iterator charid_itr = kElement.kContUserGuid.begin();
		while( kElement.kContUserGuid.end() != charid_itr )
		{
			m_kContIndexToTeam.insert( std::make_pair(kElement.iTournamentIndex, team_itr->first) );
			m_kContCharToTeam.insert( std::make_pair(charid_itr->kGuid, team_itr->first) );
			++charid_itr;
		}
		++team_itr;
	}
}

void PgPvPGame::InitGroupInfo()
{
	if( m_kContBattle.empty() )
	{
		return;
	}

	m_kContGroup.clear();

	CONT_DEF_PVPLEAGUE_BATTLE::const_iterator battle_itr = m_kContBattle.begin();
	while( m_kContBattle.end() != battle_itr )
	{
		CONT_BATTLE_GROUP::iterator group_itr = m_kContGroup.find(battle_itr->second.iGroupIndex);
		if( m_kContGroup.end() != group_itr )
		{
			group_itr->second.insert(*battle_itr);
		}
		else
		{
			CONT_DEF_PVPLEAGUE_BATTLE kContBattle;
			kContBattle.insert(*battle_itr);
			m_kContGroup.insert( std::make_pair(battle_itr->second.iGroupIndex, kContBattle) );
		}
		++battle_itr;
	}

	if( 1 == m_kContGroup.size() )
	{
		CONT_BATTLE_GROUP::iterator group_itr = m_kContGroup.begin();
		if( m_kContGroup.end() != group_itr 
			&& 1 == group_itr->second.size() )
		{//결승전 예외처리
			TBL_DEF_PVPLEAGUE_BATTLE kDummy;
			group_itr->second.insert( std::make_pair(BM::GUID::NullData(), kDummy) );
		}
	}
}

bool PgPvPGame::GetLeagueBeginTime(int const iLeagueLevel, TBL_DEF_PVPLEAGUE_TIME& rkOut) const
{
	if( !m_kContDefPvPLeagueTime.empty() )
	{
		CONT_DEF_PVPLEAGUE_TIME::const_iterator time_itr = m_kContDefPvPLeagueTime.begin();
		while( m_kContDefPvPLeagueTime.end() != time_itr )
		{
			if( iLeagueLevel == time_itr->iLeagueLevel 
				&& 0 != time_itr->iLeagueState )
			{
				rkOut = *time_itr;
				return true;
			}
			++time_itr;
		}
	}

	return false;
}

void PgPvPGame::RefreshRoomList()
{
	if ( m_eStatus != STATUS_LOBBY )
	{
		return;
	}

	XUIMgr.Activate(ms_FRM_PVP_BACKGROUND);
	XUI::CXUI_Wnd* pkLobby = XUIMgr.Activate(ms_FRM_PVP_LOBBY);
	XUIMgr.BlockGlobalScript(true);

	if ( pkLobby )
	{
		int iOldLobbyID = 0;
		pkLobby->GetCustomData( &iOldLobbyID, sizeof(iOldLobbyID) );
		pkLobby->SetCustomData( &m_iLobbyID, sizeof(m_iLobbyID) );

		wchar_t wszTemp[MAX_PATH] = {0,};
		XUI::CXUI_Wnd *pkWnd = NULL;
		XUI::CXUI_Wnd *pkTemp = NULL;

		{
			int iID = 0;
			::swprintf_s( wszTemp, MAX_PATH, L"CBTN_LOBBYID_%d", ++iID );
			XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>( pkLobby->GetControl( std::wstring(wszTemp) ) );
			while( pkChkBtn )
			{
				pkChkBtn->Check( m_iLobbyID == iID );
				pkChkBtn->Enable( m_iLobbyID != iID );
				if ( m_iLobbyID != iOldLobbyID )
				{
					pkChkBtn->ClickLock( true );
					int iValue = 10;
					pkChkBtn->SetCustomData( &iValue, sizeof(iValue) );
				}
				
				::swprintf_s( wszTemp, MAX_PATH, L"CBTN_LOBBYID_%d", ++iID );
				pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>( pkLobby->GetControl( std::wstring(wszTemp) ) );
			}

		}

		CONT_PVPROOM_LIST::key_type iSelectRoomNo = 0;
		bool bFindNo = false;
		XUI::CXUI_Wnd *pkFrameWnd = pkLobby->GetControl(PgContentsBase::ms_FRM_CLICK_FRAME);
		if ( pkFrameWnd && pkFrameWnd->Visible() )
		{
			pkFrameWnd->GetCustomData( &iSelectRoomNo, sizeof(iSelectRoomNo) );
		}
		
		CONT_PVPROOM_LIST::const_iterator room_itr = m_kContRoom.begin();

		XUI::CXUI_Wnd* pkPageControl = pkLobby->GetControl( _T("SFRM_MAIN_BG") );
		if ( pkPageControl )
		{
			pkPageControl = pkPageControl->GetControl( _T("FRM_PAGE_CONTROL") );
		}

		if ( pkPageControl )
		{
			CONT_PVPROOM_LIST::const_iterator temp_itr = m_kContRoom.begin();

			size_t iStartPage = 0;
			size_t iLastPage = 0;

			if ( m_kPage.bOnlyWait )
			{// 골치아프네 ㅋㅋ
				size_t iWaitRoomCount = 0;
				while ( temp_itr!=m_kContRoom.end() )
				{
					if ( temp_itr->second.m_kStatus == ROOM_STATUS_LOBBY )
					{
						if ( (iWaitRoomCount % 15) == 0 )
						{
							if ( (iWaitRoomCount / 15) <= m_kPage.iNowPage )
							{
								room_itr = temp_itr;
							}
						}
						++iWaitRoomCount;
					}
					++temp_itr;
				}

				if ( !iWaitRoomCount )
				{
					iLastPage = 0;
				}
				else
				{
					iLastPage = iWaitRoomCount / 15; 
					if ( iWaitRoomCount % 15 == 0 )
					{
						--iLastPage;
					}
				}
				
				if ( iLastPage < m_kPage.iNowPage )
				{
					m_kPage.iNowPage = iLastPage;
				}
			}
			else
			{
				if ( m_kContRoom.empty() )
				{
					iLastPage = 0;
				}
				else
				{
					iLastPage = m_kContRoom.size() / 15; 
					if ( m_kContRoom.size() % 15 == 0 )
					{
						--iLastPage;
					}
				}

				if ( iLastPage < m_kPage.iNowPage )
				{
					m_kPage.iNowPage = iLastPage;
				}
				
				size_t iPassCount = m_kPage.iNowPage * 15;
				while ( iPassCount-- )
				{
					++room_itr;
				}
			}

			// First
			XUI::CXUI_Wnd* pkBtn = pkPageControl->GetControl( _T("BTN_TO_FIRST") );
			if ( pkBtn )
			{
				pkBtn->IsClosed( 0==m_kPage.iNowPage );
			}

			// Prev
			pkBtn = pkPageControl->GetControl( _T("BTN_TO_PREV") );
			if ( pkBtn )
			{
				pkBtn->IsClosed( 0==m_kPage.iNowPage );
				if ( m_kPage.iNowPage )
				{
					size_t iPage = m_kPage.iNowPage-1;
					pkBtn->SetCustomData( &iPage, sizeof(size_t) );
				}
			}

			// Next
			pkBtn = pkPageControl->GetControl( _T("BTN_TO_NEXT") );
			if ( pkBtn )
			{
				pkBtn->IsClosed( iLastPage==m_kPage.iNowPage );
				if ( iLastPage > m_kPage.iNowPage )
				{
					size_t iPage = m_kPage.iNowPage+1;
					pkBtn->SetCustomData( &iPage, sizeof(size_t) );
				}
			}

			// End
			pkBtn = pkPageControl->GetControl( _T("BTN_TO_END") );
			if ( pkBtn )
			{
				pkBtn->IsClosed( iLastPage==m_kPage.iNowPage );
				pkBtn->SetCustomData( &iLastPage, sizeof(size_t) );
			}

			//1, 2, 3, 4, 5
			size_t iStart = (m_kPage.iNowPage / 5) * 5;
			for( int i=0; i!=5; ++i )
			{
				::swprintf_s( wszTemp, MAX_PATH, L"BTN_NUM_%d", i );
				XUI::CXUI_Button* pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPageControl->GetControl( std::wstring(wszTemp) ));
				if ( pkButton )
				{
					size_t iPage = iStart + i;
					
					pkButton->Visible(iPage <= iLastPage);
					if ( iPage == m_kPage.iNowPage )
					{
						pkButton->FontColor(0xFFFFFF00);
						pkButton->IsClosed(true);
					}
					else
					{
						pkButton->FontColor(0xFFFFFFFF);
						pkButton->IsClosed(false);
						pkButton->SetCustomData(&iPage, sizeof(iPage));
					}
					swprintf_s(wszTemp,MAX_PATH,L"%u", iPage+1 );
					pkButton->Text(std::wstring(wszTemp));
				}
			}
		}

		bool bEndList = (m_kContRoom.end() == room_itr);
		
		int iCount = 1;
		while ( 1 )
		{
			swprintf_s( wszTemp, MAX_PATH, L"FRM_ROOMITEM_%d", iCount++ );
			pkWnd = pkLobby->GetControl( std::wstring(wszTemp) );
			if ( pkWnd )
			{
				if ( m_kPage.bOnlyWait )
				{
					while ( !bEndList )
					{
						if ( room_itr->second.m_kStatus != ROOM_STATUS_LOBBY )
						{
							bEndList = (++room_itr == m_kContRoom.end());
						}
						else
						{
							break;
						}
					}
				}

				if ( bEndList )
				{
					pkWnd->Visible(false);
					continue;
				}

				pkWnd->Visible(true);
				pkTemp = pkWnd->GetControl( _T("FRM_DUMMY") );
				if ( pkTemp )
				{
					pkTemp->SetCustomData( &room_itr->first, sizeof(room_itr->first) );

					if ( iSelectRoomNo == room_itr->first )
					{
						bFindNo = true;
						pkFrameWnd->Location( pkTemp->GetTotalLocation() );
					}
				}

				// 방번호
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_NO") );
				if ( pkTemp )
				{
					swprintf_s( wszTemp, MAX_PATH, L"%d", room_itr->first );
					pkTemp->Text(wszTemp);
				}	

				// 방제목
				std::wstring wstrName = room_itr->second.m_wstrName;
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_TITLE") );
				if ( pkTemp )
				{
					// 패스워드
					XUI::CXUI_Wnd *pkPwd = pkTemp->GetControl( _T("FRM_LOCK") );
					if ( pkPwd )
					{
						if( room_itr->second.m_bPwd )
						{
							pkPwd->Visible(true);
							wstrName = std::wstring(L"       ") + room_itr->second.m_wstrName;
						}
						else
						{
							pkPwd->Visible(false);
						}	
					}
					pkTemp->Text(wstrName);
				}

				// 인원
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_USERCOUNT") );
				if ( pkTemp )
				{
					swprintf_s (wszTemp, MAX_PATH, L"%u / %u", room_itr->second.m_ucNowUser, room_itr->second.m_ucMaxUser );
					pkTemp->Text(wszTemp);
				}

				//Type
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_MODE") );
				if ( pkTemp )
				{
					if( false == IsExercise() )
					{
						// 랭킹모드라면...
						std::wstring wstrMsg;
						WstringFormat( wstrMsg, MAX_PATH, TTW(450257).c_str(), room_itr->second.m_sLevelLimit_Min, room_itr->second.m_sLevelLimit_Max );
						pkTemp->Text(wstrMsg);
					}
					else
					{
						pkTemp->Text( TTW(lua_tinker::call<int,EPVPMODE>("GetPvPTypeTextTable", room_itr->second.m_kType) ) );			
					}
				}

				// Map Name
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_MAP") );
				if ( pkTemp )
				{
					if( false == IsExercise() )
					{
						// 랭킹모드라면...
						wstrName = TTW(lua_tinker::call<int,EPVPMODE>("GetPvPTypeTextTable", room_itr->second.m_kType));
					}
					else
					{
						GetGroundName(  room_itr->second.m_iGndNo, wstrName );
					}
					pkTemp->Text(wstrName);
				}

				// State
				pkTemp = pkWnd->GetControl( _T("FRM_ROOM_STATE") );
				if ( pkTemp )
				{
					if ( room_itr->second.m_kStatus == ROOM_STATUS_LOBBY )
					{
						pkTemp->Text(TTW(200001));
					}
					else
					{
						pkTemp->Text(TTW(200002));
					}
				}

				bEndList = (++room_itr == m_kContRoom.end());
			}
			else
			{
				break;
			}
		}

		if ( iSelectRoomNo  )
		{
			if ( !bFindNo )
			{
				lua_tinker::call<void,lwUIWnd>("InitPvP_LobbyRoomFrame", lwUIWnd(pkFrameWnd) );
			}
		}

		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkMyPlayer )
		{
			MyInfoRefresh( dynamic_cast<SPvPReport*>(pkMyPlayer) );
		}
	}
}

void PgPvPGame::Recv_PT_T_C_ANS_JOIN_ROOM( BM::Stream &kPacket )
{
	EPvPRoomJoinRet eRet;
	CONT_PVPROOM_LIST::key_type iRoomKey = 0;
	kPacket.Pop( iRoomKey );

	if ( kPacket.Pop( eRet ) )
	{
		bool bJoin = false;
		kPacket.Pop( bJoin );
		switch ( eRet )
		{
		case PVP_JOIN_SUCCEED:
			{
				bool bExport = false;
				XUI::CXUI_List2::CExport kExport[TAB_ALLNUM];

				for(int i=0;i<TAB_ALLNUM;++i)
				{
					XUI::CXUI_List2 *pList = GetLobbyTabList(i);
					if ( pList )
					{
						pList->Export( kExport[i] );
						bExport = true;
						pList = NULL;
					}
				}

				Clear(false);
				ClearEntryUI();

				CONT_PVP_SLOT kContSlot;
				m_iRoomKey = iRoomKey;
				m_kRoomBaseInfo.ReadFromPacket(kPacket);
				m_kRoomExtInfo.ReadFromPacket(kPacket);
				PU::TLoadTable_AM( kPacket, m_kContPvPUser );
				PU::TLoadKey_A( kPacket, kContSlot );

				lwCloseAllUI(true);
				SetStatus( STATUS_ROOM );
				g_kChatMgrClient.ChatStation(ECS_PVPLOBBY);
				lwCallChatWindow(1);

				OpenRoom( kContSlot );

				if ( bExport )
				{
					for(int i=0;i<TAB_ALLNUM;++i)
					{
						XUI::CXUI_List2 *pList = GetLobbyTabList(i);
						if ( pList )
						{
							pList->Import( kExport[i] );
						}
					}
				}

				m_iKillCount = 0;// KillCount 초기화
			}break;
		case PVP_JOIN_PASSWORD_EMPTY:
			{
				XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( ms_FRM_PVP_JOIN_PASSWORD, true );
				if ( pkWnd )
				{
					pkWnd = pkWnd->GetControl( _T("BTN_OK") );
					if ( pkWnd )
					{
						pkWnd->SetCustomData( &iRoomKey, sizeof(iRoomKey) );
					}
				}
			}break;
		default:
			{
				if( (false == IsExercise()) && (false == bJoin) )
				{
					// 랭킹모드에서 빠른 입장을 누른 경우 방을 만들어 준다.
					OnClick_EmptyCreateRoom();
				}
				else
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", eRet, true);
				}
			}break;
		}
	}
}

void PgPvPGame::OnClick_EmptyCreateRoom()
{
	if( false == IsExerciseType() )
	{
		// 기본 랭킹모드 방 만들기(대기방에서 빠른입장시 입장에 실패한 경우 기본방 만들기)
		m_kRoomBaseInfo.m_wstrName = TTW(BM::Rand_Index(4) + 200010);
		m_kRoomBaseInfo.m_bPwd = false;
		m_kRoomBaseInfo.m_kStatus = ROOM_STATUS_LOBBY;
		m_kRoomBaseInfo.m_iGndNo = GetRandomPvPGround(false, PVP_TYPE_ANNIHILATION);
		m_kRoomBaseInfo.m_kType = PVP_TYPE_ANNIHILATION;
		m_kRoomBaseInfo.m_kMode = PVP_MODE_TEAM;
		m_kRoomBaseInfo.m_iGameTime = 3;
		m_kRoomBaseInfo.m_sLevelLimit_Max = GetRankingModeLevelMax();
		m_kRoomBaseInfo.m_sLevelLimit_Min = GetRankingModeLevelMin();
		m_kRoomBaseInfo.m_ucMaxUser = 8;

		m_kRoomExtInfo.strPassWord = "";
		m_kRoomExtInfo.ucRound = 9;
		m_kRoomExtInfo.kOption = (E_PVP_OPT_USEITEM | E_PVP_OPT_USEBATTLELEVEL); //10

		BM::Stream kPacket(PT_C_T_REQ_CREATE_ROOM);
		m_kRoomBaseInfo.WriteToPacket(kPacket);
		m_kRoomExtInfo.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
	}
}

int const PgPvPGame::GetRankingModeLevelMax()
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		CONT_DEF_PLAYLIMIT_INFO const *pkPlayTimeInfo = NULL;
		g_kTblDataMgr.GetContDef( pkPlayTimeInfo );

		CONT_DEF_PLAYLIMIT_INFO::mapped_type::value_type kTempInfo;
		SYSTEMTIME kTempTime[2];
		::memset( kTempTime, 0, sizeof(kTempTime) );

		wchar_t wszTemp[MAX_PATH] = {0,};

		CONT_DEF_PLAYLIMIT_INFO::const_iterator itr = pkPlayTimeInfo->find( m_iLobbyID );
		if ( itr != pkPlayTimeInfo->end() )
		{
			PgPlayLimit_Finder kFinder( itr->second );
			__int64 const i64NowTimeInDay = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );
			if( S_OK == kFinder.Find( i64NowTimeInDay, kTempInfo ) )
			{
				int const iMyLevel = pkPlayer->GetAbil( AT_LEVEL );
				int const iRoomLevel = iMyLevel + kTempInfo.iBalanceLevelMax;
				if( 0 < iRoomLevel )
				{
					return iRoomLevel;
				}
			}
		}
	}

	return 100;
}

int const PgPvPGame::GetRankingModeLevelMin()
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		CONT_DEF_PLAYLIMIT_INFO const *pkPlayTimeInfo = NULL;
		g_kTblDataMgr.GetContDef( pkPlayTimeInfo );

		CONT_DEF_PLAYLIMIT_INFO::mapped_type::value_type kTempInfo;
		SYSTEMTIME kTempTime[2];
		::memset( kTempTime, 0, sizeof(kTempTime) );

		wchar_t wszTemp[MAX_PATH] = {0,};

		CONT_DEF_PLAYLIMIT_INFO::const_iterator itr = pkPlayTimeInfo->find( m_iLobbyID );
		if ( itr != pkPlayTimeInfo->end() )
		{
			PgPlayLimit_Finder kFinder( itr->second );
			__int64 const i64NowTimeInDay = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );
			if( S_OK == kFinder.Find( i64NowTimeInDay, kTempInfo ) )
			{
				int const iMyLevel = pkPlayer->GetAbil( AT_LEVEL );
				int const iRoomLevel = iMyLevel - kTempInfo.iBalanceLevelMin;
				if( 0 < iRoomLevel )
				{
					return iRoomLevel;
				}
			}
		}
	}

	return 1;
}

void PgPvPGame::Recv_PT_T_C_ANS_MODIFY_ROOM( BM::Stream &kPacket )
{
	bool bBasic = true;
	if ( kPacket.Pop( bBasic ) )
	{
		if ( bBasic )
		{
			m_kRoomBaseInfo.ReadFromPacket(kPacket);

			bool bRefreshExtInfo = false;
			kPacket.Pop(bRefreshExtInfo);
			if ( bRefreshExtInfo )
			{
				m_kRoomExtInfo.ReadFromPacket(kPacket);
			}

			bool bRefreshSlot = false;
			kPacket.Pop( bRefreshSlot );

			if ( bRefreshSlot )
			{
				m_kContPvPUser.clear();
				CONT_PVP_SLOT kContSlot;
				PU::TLoadTable_AM( kPacket, m_kContPvPUser );
				PU::TLoadKey_A( kPacket, kContSlot );

				XUIMgr.Close(ms_FRM_PVP_ROOM[PVP_MODE_TEAM]);
				XUIMgr.Close(ms_FRM_PVP_ROOM[PVP_MODE_PERSONAL]);
				OpenRoom( kContSlot );
			}
		}
		else
		{
//			bool bUseHandyCap = m_kRoomExtInfo.bUseHandyCap;
			m_kRoomExtInfo.ReadFromPacket(kPacket);

//			if ( bUseHandyCap != m_kRoomExtInfo.bUseHandyCap )
//			{// 핸디캡이 교체 되었으면
//				RefreshHandyCap();
//			}
		}
		RefreshRoomAttr( XUIMgr.Get(ms_FRM_PVP_ROOM_ATTR) );
	}
}


void PgPvPGame::Recv_PT_T_C_ANS_TEAM_CHANGE( BM::Stream &kPacket )
{
	BM::GUID kGuid;
	CONT_PVP_GAME_USER::mapped_type kElement;
	kPacket.Pop(kGuid);
	kElement.ReadFromPacket(kPacket);

	std::pair<CONT_PVP_GAME_USER::iterator, bool> kPair = m_kContPvPUser.insert( std::make_pair(kGuid, kElement) );
	if ( !kPair.second )
	{
		XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( kPair.first->second );
		if ( pkSlotWnd )
		{
			ClearSlot( pkSlotWnd, false, kPair.first->second.kTeamSlot.GetTeam() );
		}
		kPair.first->second = kElement;
	}
	else
	{
		// 새로 입장한 놈이다.
		std::wstring kMsg;
		WstringFormat( kMsg, MAX_PATH, TTW(400397).c_str(), kElement.wstrName.c_str() );
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage( kChatLog, kMsg );
	}

	OpenSlot( kGuid, kPair.first->second, NULL );
	RefreshStartButton();
}

void PgPvPGame::Recv_PT_T_C_NFY_USER_STATUS_CHANGE( BM::Stream &kPacket )
{
	BM::GUID kGuid;
	BYTE kStatus = PS_PNONE;
	kPacket.Pop(kGuid);
	kPacket.Pop(kStatus);

	CONT_PVP_GAME_USER::mapped_type *pkElement = Get(kGuid);
	if ( pkElement )
	{
		pkElement->byStatus = kStatus;
		OpenSlot( kGuid, *pkElement, NULL );
		RefreshStartButton();
	}
}

/*
void PgPvPGame::Recv_PT_T_C_NFY_CHANGE_HANDYCAP( BM::Stream &kPacket )
{
	BM::GUID kGuid;
	int iHandyCap = 0;
	kPacket.Pop(kGuid);
	kPacket.Pop(iHandyCap);

	CONT_PVP_GAME_USER::mapped_type *pkElement = Get(kGuid);
	if ( pkElement )
	{
		pkElement->iHandyCap = iHandyCap;
		OpenSlot( kGuid, *pkElement, NULL );
	}
}
*/

void PgPvPGame::Recv_PT_T_C_NFY_EXIT_ROOM_USER( BM::Stream &kPacket )
{
	BM::GUID kCharGuid;
	bool bChangeMaster = false;
	kPacket.Pop(kCharGuid);
	kPacket.Pop( bChangeMaster );

	CONT_PVP_GAME_USER::iterator user_itr = m_kContPvPUser.find(kCharGuid);
	if ( user_itr!=m_kContPvPUser.end() )
	{
		// 나갔네.
		std::wstring kMsg;
		WstringFormat( kMsg, MAX_PATH, TTW(400398).c_str(), user_itr->second.wstrName.c_str() );
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage( kChatLog, kMsg );

		XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( user_itr->second );
		if ( pkSlotWnd )
		{
			ClearSlot( pkSlotWnd, false, user_itr->second.kTeamSlot.GetTeam() );

			if ( m_eStatus == STATUS_PLAYING )
			{
				SortSlot( user_itr->second.kTeamSlot.GetSlot(), user_itr->second.kTeamSlot.GetSlot(), user_itr->second.kTeamSlot.GetTeam() );
			}
		}
		m_kContPvPUser.erase(user_itr);
	}

	if ( bChangeMaster )
	{
		BM::GUID kMasterGuid;
		kPacket.Pop(kMasterGuid);
		SetChangeMaster( kMasterGuid, BM::GUID::NullData() );
	}

	RefreshStartButton();
}

void PgPvPGame::SetChangeMaster( BM::GUID const &kNewMasterGuid, BM::GUID const &kOldMasterGuid )
{
	CONT_PVP_GAME_USER::iterator user_itr = m_kContPvPUser.find(kNewMasterGuid);
	if ( user_itr!=m_kContPvPUser.end() )
	{
		// 방장이 교체 되었다.
		std::wstring kMsg;
		WstringFormat( kMsg, MAX_PATH, TTW(400399).c_str(), user_itr->second.wstrName.c_str() );
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage( kChatLog, kMsg );

		user_itr->second.byStatus |= PS_MASTER;
		user_itr->second.byStatus &= (~PS_READY);
		OpenSlot( kNewMasterGuid, user_itr->second, NULL );
	}

	user_itr = m_kContPvPUser.find(kOldMasterGuid);
	if ( user_itr!=m_kContPvPUser.end() )
	{
		user_itr->second.byStatus &= (~PS_MASTER);
		OpenSlot( kOldMasterGuid, user_itr->second, NULL );
	}

	RefreshRoomAttr( XUIMgr.Get(ms_FRM_PVP_ROOM_ATTR) );
	RefreshStartButton();
}

bool PgPvPGame::IsMaster()
{
	PgPilot const *pkMyPilot = g_kPilotMan.GetPlayerPilot();
	if( pkMyPilot )
	{
		BM::GUID const &kMasterGuid = pkMyPilot->GetGuid();

		CONT_PVP_GAME_USER::iterator user_itr = m_kContPvPUser.find(kMasterGuid);
		if ( user_itr!=m_kContPvPUser.end() )
		{
			bool const bMaster = (user_itr->second.byStatus & PS_MASTER);
			return bMaster;
		}
	}
	return false;
}

XUI::CXUI_Wnd* PgPvPGame::OpenRoom( CONT_PVP_SLOT const &kContSlot )
{
	XUIMgr.Activate(ms_FRM_PVP_BACKGROUND);

	// Room Slot
	XUI::CXUI_Wnd *pkRoomWnd = XUIMgr.Activate(ms_FRM_PVP_ROOM[m_kRoomBaseInfo.m_kMode]);
	if ( pkRoomWnd )
	{
		InitSlot( pkRoomWnd, &kContSlot );

		int iMyTeam = TEAM_NONE;
		bool bSetEntryUI = false;
		PgPilot const * pkPilot = g_kPilotMan.GetPlayerPilot();
		XUI::CXUI_Wnd *pkWndEntryBG = pkRoomWnd->GetControl(L"FRM_ENTRY_BG");
		if(pkPilot && pkWndEntryBG)
		{
			switch( m_kRoomBaseInfo.m_kType )
			{
			case PVP_TYPE_WINNERS:
			case PVP_TYPE_WINNERS_TEAM:
			case PVP_TYPE_ACE:
				{
					CONT_PVP_GAME_USER::const_iterator user_itr = m_kContPvPUser.find(pkPilot->GetGuid());
					if( m_kContPvPUser.end() != user_itr )
					{
						iMyTeam = user_itr->second.kTeamSlot.GetTeam();
						bSetEntryUI = true;
					}
					pkWndEntryBG->Visible(true);
				}break;
			default: 
				{
					pkWndEntryBG->Visible(false);
				}break;
			}
		}

		VEC_INT kContEntry;
		CONT_PVP_GAME_USER::iterator user_itr;
		for ( user_itr=m_kContPvPUser.begin(); user_itr!=m_kContPvPUser.end(); ++user_itr )
		{
			CONT_PVP_GAME_USER::mapped_type &kElement = user_itr->second;
			OpenSlot( user_itr->first, kElement, NULL );
			if( bSetEntryUI && iMyTeam == kElement.kTeamSlot.GetTeam() )
			{
				if( kElement.byEntryNo )
				{
					kContEntry.push_back(kElement.byEntryNo);
				}
			}
		}

		VEC_INT::const_iterator entry_itr = kContEntry.begin();
		while( kContEntry.end() != entry_itr )
		{
			BM::vstring vstrEntry(L"BTN_ENTRY_");
			vstrEntry += *(entry_itr);
			XUI::CXUI_Wnd *pkBtnEntry = pkWndEntryBG->GetControl(vstrEntry.operator const std::wstring &());
			if( pkBtnEntry )
			{
				pkBtnEntry->Enable(false);
			}
			++entry_itr;
		}
	}

	RefreshRoomTitle( pkRoomWnd );
	RefreshRoomAttr( XUIMgr.Activate(ms_FRM_PVP_ROOM_ATTR) );
	RefreshStartButton();

	m_ucNowRound = 0;
	::memset( m_ucRoundScore, 0, sizeof(m_ucRoundScore) );

	return pkRoomWnd;
}


void PgPvPGame::RefreshRoomTitle( XUI::CXUI_Wnd *pkRoomWnd )
{
	if ( pkRoomWnd )
	{
		XUI::CXUI_Wnd *pkRoomNo = pkRoomWnd->GetControl( _T("FRM_ROOMNO") );
		if ( pkRoomNo )
		{
			std::wstring wstrRoomNo;
			WstringFormat(wstrRoomNo, MAX_PATH, TTW(400329).c_str(), m_iRoomKey );
			pkRoomNo->Text(wstrRoomNo);
			
			pkRoomNo = pkRoomNo->GetControl( _T("SUBJECT") );
			if ( pkRoomNo )
			{
				TBL_DEF_PVPLEAGUE_BATTLE kBattleInfo;
				if( IsLeague() && GetBattleInfo(m_kRoomBaseInfo.m_kBattleGuid, kBattleInfo) )
				{
					TBL_DEF_PVPLEAGUE_TEAM kTeamInfo1, kTeamInfo2;
					if( GetTeamInfo(kBattleInfo.kTeamGuid1, kTeamInfo1) && GetTeamInfo(kBattleInfo.kTeamGuid2, kTeamInfo2) )
					{
						m_kRoomBaseInfo.m_wstrName = kTeamInfo1.wstrTeamName;
						m_kRoomBaseInfo.m_wstrName += L" : ";
						m_kRoomBaseInfo.m_wstrName += kTeamInfo2.wstrTeamName;

					}
				}
				Quest::SetCutedTextLimitLength(pkRoomNo, m_kRoomBaseInfo.m_wstrName, L"...");
				
			}
		}
	}
}

void PgPvPGame::RefreshRoomAttr( XUI::CXUI_Wnd *pkRoomAttrWnd )
{
	if( pkRoomAttrWnd )
	{
		XUI::CXUI_Wnd* pkTemp = NULL;
		wchar_t wszTemp[MAX_PATH] = {0,};

		// Map Name
		pkTemp = pkRoomAttrWnd->GetControl(_T("SFRM_TAB_MAPNAME"));
		if (pkTemp)
		{
			std::wstring wstrGndName;

			GetGroundName( m_kRoomBaseInfo.m_iGndNo, wstrGndName );

			if( false == IsExerciseType() )
			{
				wstrGndName = TTW(450259);
			}

			pkTemp->Text(wstrGndName);
			pkTemp->SetCustomData(&m_kRoomBaseInfo.m_iGndNo,sizeof(int));
		}
		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_MAPNAME"));
		if ( pkTemp )
		{
			pkTemp->Visible( IsModifyRoomAttr() );
		}

		// Preview Img
		pkTemp = pkRoomAttrWnd->GetControl(_T("IMG_PREVIEW"));
		if ( pkTemp )
		{
			std::wstring wstrPreviewImgPath;

			if( false == IsExerciseType() )
			{
				wstrPreviewImgPath = _T("../Data/6_UI/pvp/pvMapBg.tga");
			}
			else
			{
				GetPreviewImgPath( m_kRoomBaseInfo.m_iGndNo, wstrPreviewImgPath );
			}			
			pkTemp->DefaultImgName(wstrPreviewImgPath);
		}

		// Mode Name
		pkTemp = pkRoomAttrWnd->GetControl( _T("SFRM_TAB_GAMEMODE") );
		if ( pkTemp )
		{
			pkTemp->Text( TTW(lua_tinker::call<int,EPVPMODE>( "GetPvPModeTextTable", m_kRoomBaseInfo.m_kMode )) );
		}
		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_GAMEMODE"));
		if ( pkTemp )
		{
			pkTemp->Visible( IsModifyRoomAttr() && IsPersonalGame() );
		}

		// Type Name
		pkTemp = pkRoomAttrWnd->GetControl( _T("SFRM_TAB_GAMETYPE") );
		if ( pkTemp )
		{
			pkTemp->Text( TTW(lua_tinker::call<int,EPVPTYPE>( "GetPvPTypeTextTable", m_kRoomBaseInfo.m_kType )) );
		}
		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_GAMETYPE"));
		if ( pkTemp )
		{
			pkTemp->Visible( IsModifyRoomAttr() );
		}

		// Time
		pkTemp = pkRoomAttrWnd->GetControl(_T("SFRM_TAB_TIMELIMIT"));
		if (pkTemp)
		{
			int GameLimit = 0;
			if( m_kRoomBaseInfo.IsHavePvPType(PVP_TYPE_KTH) )
			{ // 점수 조건만 있는 맵
				GameLimit = m_kRoomBaseInfo.m_iGamePoint;
				swprintf_s(wszTemp, MAX_PATH, L"%d", GameLimit );
			}
			else if( m_kRoomBaseInfo.IsHavePvPType(PVP_TYPE_DM)
				|| m_kRoomBaseInfo.IsHavePvPType(PVP_TYPE_ANNIHILATION) )
			{ // 시간 조건만 있는 맵
				GameLimit = m_kRoomBaseInfo.m_iGameTime;
				swprintf_s(wszTemp, MAX_PATH, L"%d", GameLimit );
			}
			else if( m_kRoomBaseInfo.IsHavePvPType(PVP_TYPE_LOVE) )
			{ // 시간과 점수 조건이 모두 없는 맵
				swprintf_s(wszTemp, MAX_PATH, L"%s", TTW(3331).c_str());
			}
			else
			{ // 시간과 점수 조건이 동시에 있는 맵
			}
			pkTemp->Text(std::wstring(wszTemp));
		}
		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_TIMELIMIT"));
		if ( pkTemp )
		{
			if( m_kRoomBaseInfo.IsHavePvPType(PVP_TYPE_LOVE) )
			{
				pkTemp->Visible( false );
			}
			else
			{
				pkTemp->Visible( IsModifyRoomAttr() );
			}
		}

		// Level
		pkTemp = pkRoomAttrWnd->GetControl(_T("SFRM_TAB_LEVELLIMIT"));
		if (pkTemp)
		{
			std::wstring wstrMsg;
			if( m_kRoomBaseInfo.IsHaveLimitLevel() )
			{
				WstringFormat( wstrMsg, MAX_PATH, TTW(450257).c_str(), m_kRoomBaseInfo.m_sLevelLimit_Min, m_kRoomBaseInfo.m_sLevelLimit_Max );
			}
			else
			{
				wstrMsg = TTW(3331);
			}
			pkTemp->Text(wstrMsg);
		}

		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_LEVELLIMIT"));
		if ( pkTemp )
		{
			pkTemp->Visible( IsModifyRoomAttr() );
		}

		// Round
		pkTemp = pkRoomAttrWnd->GetControl(_T("SFRM_TAB_ROUNDCOUNT"));
		if (pkTemp)
		{
			swprintf_s(wszTemp, MAX_PATH, L"%u", m_kRoomExtInfo.ucRound );
			pkTemp->Text( std::wstring(wszTemp) );
		}
		pkTemp = pkRoomAttrWnd->GetControl(_T("CBTN_ROUNDCOUNT"));
		if ( pkTemp )
		{
			pkTemp->Visible( IsModifyRoomAttr() && IsRoundCount() );
		}

		
		pkTemp = pkRoomAttrWnd->GetControl(_T("SFRM_OPTION"));
		if ( pkTemp )
		{
			// HandyCap
			XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_HANDYCAP")));
			if ( pkChkBtn )
			{
				pkChkBtn->Check( m_kRoomExtInfo.IsUseHandyCap() );
				pkChkBtn->ClickLock(!IsModifyRoomAttr());
			}

			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_USE_ITEM")));
			if ( pkChkBtn )
			{
				pkChkBtn->Check( m_kRoomExtInfo.IsUseItem() );
				pkChkBtn->ClickLock(!IsModifyRoomAttr());
			}

			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_DISABLE_DASHJUMP")));
			if ( pkChkBtn )
			{
				pkChkBtn->Check( m_kRoomExtInfo.IsDisableDashJump() );
				pkChkBtn->ClickLock(!IsModifyRoomAttr());
			}

			pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_USE_BATTLELEVEL")));
			if ( pkChkBtn )
			{
				pkChkBtn->Check( m_kRoomExtInfo.IsUseBattleLevel() );
				pkChkBtn->ClickLock(!IsModifyRoomAttr());
			}
		}
		lua_tinker::call<void, int, int, int, int, int, int>("SetRoomBaseInfo",
					m_kRoomBaseInfo.m_kMode, m_kRoomBaseInfo.m_kType, m_kRoomBaseInfo.m_iGndNo,
					m_kRoomBaseInfo.m_iGamePoint, m_kRoomBaseInfo.m_iGameTime, m_kRoomBaseInfo.m_ucMaxUser);
	}
}

void PgPvPGame::RefreshStartButton()
{
	XUI::CXUI_Wnd *pkRoom = NULL;
	if ( XUIMgr.IsActivate(ms_FRM_PVP_ROOM_ATTR, pkRoom) )
	{
		XUI::CXUI_Wnd *pkFrame = pkRoom->GetControl( _T("SFRM_BTN") );
		if ( pkFrame )
		{
			bool bImpossibleExit = true;

			XUI::CXUI_Wnd *pkButton = pkFrame->GetControl( _T("BTN_START") );
			if ( pkButton )
			{
				if( PvP_Lobby_GroundNo_League == m_iLobbyID )
				{//리그전에선 이 버튼을 사용할 수 없음.
					pkButton->Enable(false);
				}
				else if ( true == m_bMyRoom )
				{
					pkButton->Text(TTW(400335) + TTW(2200));

					// 전부다 레디해 있으면 Start버튼을 깜빡여야 한다.
					int iMyTeam = TEAM_NONE;
					BM::GUID kPlayerCharGuid;
					if ( g_kPilotMan.GetPlayerPilotGuid(kPlayerCharGuid) )
					{
						CONT_PVP_GAME_USER::mapped_type const *pkElement = Get( kPlayerCharGuid );
						if ( pkElement )
						{
							iMyTeam = pkElement->kTeamSlot.GetTeam();
						}
					}

					bool bTwinkle = false;
					CONT_PVP_GAME_USER::const_iterator user_itr = m_kContPvPUser.begin();
					for ( ; user_itr != m_kContPvPUser.end() ; ++user_itr )
					{
						CONT_PVP_GAME_USER::mapped_type const &kElement = user_itr->second;
						if ( kPlayerCharGuid != user_itr->first )
						{
							if ( 0 == (kElement.byStatus & PS_READY) )
							{
								bTwinkle = false;
								break;
							}
							else
							{
								if ( iMyTeam != kElement.kTeamSlot.GetTeam() )
								{// 적이 있으면 일단 반짝이게
									bTwinkle = true;
								}
							}
						}
					}

					if ( true == bTwinkle )
					{
						pkButton->TwinkleTime(UINT_MAX);//무한
						pkButton->TwinkleInterTime(500);
					}

					pkButton->SetTwinkle(bTwinkle);
				}
				else 
				{
					BM::GUID kPlayerCharGuid;
					if ( g_kPilotMan.GetPlayerPilotGuid(kPlayerCharGuid) )
					{
						CONT_PVP_GAME_USER::mapped_type const *pkElement = Get( kPlayerCharGuid );
						if ( pkElement && ( pkElement->byStatus & PS_READY ) )
						{
							pkButton->Text(TTW(400337) + TTW(2200));
							pkButton->SetTwinkle(false);

							bImpossibleExit = false;// Ready상태에서는 나가기 버튼이 동작하지 않게.
							/*
							XUI::CXUI_Wnd *pkWnd = GetSlotWnd( *pkElement );
							if ( pkWnd )
							{
								XUI::CXUI_HScroll *pkScroll = dynamic_cast<XUI::CXUI_HScroll*>(pkWnd->GetControl(_T("HSCRL_HANDYCAP")));
								if ( pkScroll )
								{
									pkScroll->Enable(false);
								}
							}
							*/
							
						}
						else
						{
							pkButton->Text(TTW(400336) + TTW(2200));

							// 레디를 하지 않았으면 버튼을 깜빡여야해
							pkButton->TwinkleTime(UINT_MAX);//무한
							pkButton->TwinkleInterTime(500);
							pkButton->SetTwinkle(true);

							/*
							XUI::CXUI_Wnd *pkWnd = GetSlotWnd( *pkElement );
							if ( pkWnd )
							{
								XUI::CXUI_HScroll *pkScroll = dynamic_cast<XUI::CXUI_HScroll*>(pkWnd->GetControl(_T("HSCRL_HANDYCAP")));
								if ( pkScroll )
								{
									pkScroll->Enable(m_kRoomExtInfo.bUseHandyCap);
								}
							}
							*/
						}
					}
				}
			}

			pkButton = pkFrame->GetControl( _T("BTN_EXIT") );
			if ( pkButton )
			{
				pkButton->Enable( bImpossibleExit );
			}
		}
	}
}

/*
void PgPvPGame::RefreshHandyCap()
{
	if ( m_kRoomExtInfo.bUseHandyCap )
	{
		// 핸디캡을 사용한다면
		BM::GUID kCharGuid;
		g_kPilotMan.GetPlayerPilotGuid(kCharGuid);
		CONT_PVP_GAME_USER::mapped_type *pkElement = Get( kCharGuid );
		if ( pkElement )
		{
			OpenSlot( kCharGuid, *pkElement, NULL );
		}
	}
	else
	{
		// 핸디캡을 사용하지 않는다면
		// 모든 유저의 핸디캡을 초기화 해야 한다.
		CONT_PVP_GAME_USER::iterator user_itr;
		for ( user_itr=m_kContPvPUser.begin(); user_itr!=m_kContPvPUser.end(); ++user_itr )
		{
			user_itr->second.iHandyCap = 100;
			OpenSlot( user_itr->first, user_itr->second, NULL );
		}
	}
}
*/

void PgPvPGame::InitSlot( XUI::CXUI_Wnd *pkRoom, CONT_PVP_SLOT const *pkContSlot )
{
	std::wstring wstrSlotName;

	if ( PVP_MODE_TEAM == m_kRoomBaseInfo.m_kMode )
	{	
		for ( int iTeam=TEAM_RED; iTeam<TEAM_MAX; ++iTeam )
		{
			bool bClose = true;
			XUI::CXUI_Wnd *pkControl = NULL;
			XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( pkRoom, iTeam );
			if ( pkSlotWnd )
			{
				int iCount = 0;
				while( 1 )
				{
					if ( pkContSlot )
					{
						CONT_PVP_SLOT::const_iterator empty_itr = pkContSlot->find( SPvPTeamSlot(iTeam,iCount) );
						bClose = empty_itr==pkContSlot->end();
					}
					else
					{
						bClose = true;
					}

					GetSlotName( iCount, wstrSlotName );
					pkControl = pkSlotWnd->GetControl(wstrSlotName);
					if ( !pkControl )
					{
						break;
					}

					ClearSlot( pkControl, bClose, iTeam );
					++iCount;
				}
			}
		}
	}
	else
	{
		bool bClose = true;
		XUI::CXUI_Wnd *pkControl = NULL;
		XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( pkRoom, 0 );
		if ( pkSlotWnd )
		{
			int iCount = 0;
			int iTeam = TEAM_PERSONAL_BEGIN;
			while ( 1 )
			{
				if ( pkContSlot )
				{
					CONT_PVP_SLOT::const_iterator empty_itr = pkContSlot->find( SPvPTeamSlot(iTeam,iCount) );
					bClose = empty_itr==pkContSlot->end();
				}
				else
				{
					bClose = true;
				}

				GetSlotName( iCount, wstrSlotName );
				pkControl = pkSlotWnd->GetControl(wstrSlotName);
				if ( !pkControl )
				{
					break;
				}

				ClearSlot( pkControl, bClose, iTeam );

				++iTeam;
				++iCount;
			}
		}
	}
	
}

XUI::CXUI_Wnd* PgPvPGame::GetSlotWnd( XUI::CXUI_Wnd *pkWnd, int const iTeam )const
{
	if ( PVP_MODE_TEAM == m_kRoomBaseInfo.m_kMode )
	{
		wchar_t wszTemp[MAX_PATH] = {0,};
		::swprintf_s( wszTemp, MAX_PATH, L"FRM_PVP_SLOT_%d", iTeam );
		if ( pkWnd )
		{
			return pkWnd->GetControl(std::wstring(wszTemp));
		}
		return XUIMgr.Activate(std::wstring(wszTemp));
	}

	if ( pkWnd )
	{
		return pkWnd->GetControl(_T("FRM_PVP_SLOT"));
	}
	return XUIMgr.Activate(_T("FRM_PVP_SLOT"));
}

XUI::CXUI_Wnd* PgPvPGame::GetSlotWnd( CONT_PVP_GAME_USER::mapped_type const &kElement )const
{
	if( kElement.wstrUI.empty() )
	{
	//	VERIFY_INFO_LOG( false, BM::LOG_LV0, _T("[%s] Error"), __FUNCTIONW__ );
	}

	XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( XUIMgr.Get( ms_FRM_PVP_ROOM[m_kRoomBaseInfo.m_kMode] ), kElement.kTeamSlot.GetTeam() );
	if ( pkSlotWnd )
	{
		return pkSlotWnd->GetControl( kElement.wstrUI );
	}
	return NULL;
}

XUI::CXUI_Wnd* PgPvPGame::GetSlot( XUI::CXUI_Wnd *pkWnd, int const iIndex, int const iTeam )const
{
	XUI::CXUI_Wnd* pkSlot = GetSlotWnd( pkWnd, iTeam );
	if ( pkSlot )
	{
		std::wstring wstrSlotName;
		GetSlotName( iIndex, wstrSlotName );
		return pkSlot->GetControl(wstrSlotName);
	}
	return NULL;
}

void PgPvPGame::GetSlotName( int const iIndex, std::wstring &wstrName )const
{
	wchar_t wszTemp[MAX_PATH] = {0,};
	::swprintf_s( wszTemp, MAX_PATH, L"SLOT_PVP_%d", iIndex );
	wstrName = wszTemp;
}

bool PgPvPGame::OpenSlot( BM::GUID const &kCharGuid, CONT_PVP_GAME_USER::mapped_type &kElement, XUI::CXUI_Wnd *pkOldSolt )
{
	if ( kElement.wstrUI.empty() )
	{
		GetSlotName( kElement.kTeamSlot.GetSlot(), kElement.wstrUI );
	}

	bool const bIsMySlot = g_kPilotMan.IsMyPlayer(kCharGuid);
	DWORD const dwFontColor = bIsMySlot ? 0xFFFFFF00 : 0xFFFFFFFF;

	switch( m_eStatus )
	{
	case STATUS_ROOM:
		{
			XUI::CXUI_Wnd *pkRoom = NULL;
			if ( XUIMgr.IsActivate( ms_FRM_PVP_ROOM[m_kRoomBaseInfo.m_kMode], pkRoom ) )
			{
				if ( bIsMySlot )
				{
					if ( PVP_MODE_TEAM == m_kRoomBaseInfo.m_kMode )
					{
						XUI::CXUI_Wnd *pkBtn_Red = pkRoom->GetControl(_T("BTN_RED"));
						if ( pkBtn_Red )
						{
							pkBtn_Red->IsClosed( kElement.kTeamSlot.GetTeam()==TEAM_RED );
						}

						//Blue
						XUI::CXUI_Wnd *pkBtn_Blue = pkRoom->GetControl(_T("BTN_BLUE"));
						if ( pkBtn_Blue )
						{
							pkBtn_Blue->IsClosed( kElement.kTeamSlot.GetTeam()==TEAM_BLUE );
						}
					}

					m_bMyRoom = kElement.byStatus & PS_MASTER;
				}

				XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( pkRoom, kElement.kTeamSlot.GetTeam() );
				if ( pkSlotWnd )
				{
					XUI::CXUI_Wnd *pkSlot = pkSlotWnd->GetControl( kElement.wstrUI );
					if ( pkSlot )
					{
						switch( m_kRoomBaseInfo.m_kType )
						{
						case PVP_TYPE_WINNERS:
						case PVP_TYPE_WINNERS_TEAM:
						case PVP_TYPE_ACE:
							{
								XUI::CXUI_Wnd *pkWndSlotEntry = pkSlot->GetControl(L"IMG_ENTRY");
								if( pkWndSlotEntry && kElement.byEntryNo )
								{
									BM::vstring vstrEntry;
									vstrEntry = kElement.byEntryNo;
									pkWndSlotEntry->Visible(true);
									pkWndSlotEntry->Text(vstrEntry);

								}
							}break;
						default:
							{
							}break;
						}

						// Name
						XUI::CXUI_Wnd *pkItem = pkSlot->GetControl( _T("FRM_NAME") );
						if ( pkItem )
						{
							pkItem->Text( kElement.wstrName );
							pkItem->FontColor(dwFontColor);
						}

						// Level
						pkItem = pkSlot->GetControl( _T("FRM_LEVEL") );
						if ( pkItem )
						{
							wchar_t wszTemp[MAX_PATH] = {0,};
							::swprintf_s( wszTemp, MAX_PATH, L"%d", kElement.sLevel);
							pkItem->Text( std::wstring(wszTemp) );
							pkItem->FontColor(dwFontColor);
						}

						// Class
						pkItem = pkSlot->GetControl( std::wstring(L"IMG_CLASS") );
						lwSetMiniClassIconIndex( pkItem, kElement.iClass);

						bool const bMaster = kElement.byStatus & PS_MASTER;

						// Master
						pkItem = pkSlot->GetControl(_T("IMG_MASTER"));
						if ( pkItem && !IsLeague() )
						{//리그전에서는 표시 안함.
							pkItem->Visible( bMaster );
						}

						// Ready
						pkItem = pkSlot->GetControl(_T("IMG_READY"));
						if ( pkItem )
						{
							pkItem->Visible( (PS_READY & kElement.byStatus) );
							if(pkItem->Visible())
							{// 값 바꿔보며 테스트 하기 쉽게 하기 위해 lua로 뺐음
								lua_tinker::call<void>("SetReadyScaleEffect", lwUIWnd(pkItem));
							}
						}

						/*
						// HandyCap
						XUI::CXUI_HScroll *pkScroll = dynamic_cast<XUI::CXUI_HScroll*>(pkSlot->GetControl(_T("HSCRL_HANDYCAP")));
						if ( pkScroll )
						{
							pkScroll->OwnerGuid(kCharGuid);
							pkScroll->Visible(true);
							pkScroll->CurValue(kElement.iHandyCap);
							pkScroll->IsClosed(!m_kRoomExtInfo.bUseHandyCap);
							pkScroll->Enable( bIsMySlot && m_kRoomExtInfo.bUseHandyCap );
						}
						*/

						pkItem = pkSlot->GetControl(_T("OVER_SLOT"));
						if ( pkItem )
						{
							pkItem->SetCustomData( &kElement.kTeamSlot, sizeof(SPvPTeamSlot) );
							pkItem->OwnerGuid(kCharGuid);
						}

						pkItem = pkSlot->GetControl(_T("IMG_CLOSE"));
						if ( pkItem )
						{
							pkItem->Visible(false);
						}

						// BattleRecord
						pkItem = pkSlot->GetControl(_T("FRM_BATTLERECORD"));
						if ( pkItem )
						{
							/*
							float fPer = 0.0f;
							if ( 0 < kElement.kPvPReport.m_iWin )
							{
								if ( 0 < kElement.kPvPReport.m_iLose )
								{
									fPer = (float)kElement.kPvPReport.m_iWin/(float)(kElement.kPvPReport.m_iWin + kElement.kPvPReport.m_iLose + kElement.kPvPReport.m_iDraw);
									fPer *= 100.0f;
								}
								else
								{
									fPer = 100.0f;
								}
							}
							*/

							std::wstring wstrBattleRecord;
							if ( true == IsExercise() )
							{
								WstringFormat( wstrBattleRecord, MAX_PATH, TTW(400405).c_str(), kElement.kPvPReport.m_iRecords[GAME_WIN_EXERCISE], kElement.kPvPReport.m_iRecords[GAME_DRAW_EXERCISE] , kElement.kPvPReport.m_iRecords[GAME_LOSE_EXERCISE]  );
							}
							else
							{
								WstringFormat( wstrBattleRecord, MAX_PATH, TTW(400405).c_str(), kElement.kPvPReport.m_iRecords[GAME_WIN], kElement.kPvPReport.m_iRecords[GAME_DRAW] , kElement.kPvPReport.m_iRecords[GAME_LOSE]  );
							}
							
							pkItem->Text(wstrBattleRecord);
							pkItem->FontColor(dwFontColor);
						}
					}
				}
				return true;
			}
		}break;
	case STATUS_PLAYING:
		{
			XUI::CXUI_Wnd *pkSlotWnd = GetSlotWnd( NULL, kElement.kTeamSlot.GetTeam() );
			if ( pkSlotWnd )
			{
				XUI::CXUI_Wnd *pkSlot = pkSlotWnd->GetControl( kElement.wstrUI );
				if ( pkSlot )
				{
					// Name
					pkSlot->Visible(true);
					pkSlot->OwnerGuid(kCharGuid);
					pkSlot->SetCustomData( &kElement.kResult.iPoint, sizeof(kElement.kResult.iPoint) );

					if( pkSlot->OwnerGuid() != kCharGuid )
					{
						lua_tinker::call<void, lwUIWnd, float>("OnCallPvPUserSlotChange", lwUIWnd(pkSlot), 0.3f);
					}
					
					XUI::CXUI_Wnd *pkItem = pkSlot->GetControl( _T("FRM_NAME") );
					if ( pkItem )
					{
						pkItem->Text( kElement.wstrName );
						pkItem->FontColor(dwFontColor);
					}

					wchar_t wszTemp[MAX_PATH] = {0,};

					pkItem = pkSlot->GetControl( _T("FRM_KILL") );
					if ( pkItem )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%u", kElement.kResult.usKillCount );
						pkItem->Text( wszTemp );
					}

					pkItem = pkSlot->GetControl( _T("FRM_DEATH") );
					if ( pkItem )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%u", kElement.kResult.usDieCount );
						pkItem->Text( wszTemp );
					}

					pkItem = pkSlot->GetControl( _T("FRM_TOTALPOINT") );
					if( pkItem )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%d", kElement.kResult.iPoint );
						pkItem->Text( wszTemp );
					}

					if ( pkOldSolt )
					{
						int iTempMaxValue = 0;
						int iTempNowValue = 0;

						XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>( pkSlot->GetControl( _T("BAR_HP") ) );
						XUI::CXUI_AniBar *pkOldBar = dynamic_cast<XUI::CXUI_AniBar*>( pkOldSolt->GetControl( _T("BAR_HP") ) );
						if ( pkBar && pkOldBar )
						{
							iTempMaxValue = pkOldBar->Max();
							iTempNowValue = pkOldBar->Now();
							pkOldBar->Max( pkBar->Max() );
							pkOldBar->Now( pkBar->Now() );
							pkBar->Max( iTempMaxValue );
							pkBar->Now( iTempNowValue );
						}
						OnSetPvpDieUserSlot(pkSlot, ((0 >= iTempNowValue) ? true : false));
						OnSetPvpDieUserSlot(pkOldSolt, ((0 >= pkOldBar->Now()) ? true : false));
					}
					return true;
				}
			}
		}break;
	default:
		{

		}break;
	}
	return false;
}

void PgPvPGame::ClearSlot( XUI::CXUI_Wnd *pkSlot, bool const bClose, int const iTeam )
{
	switch( m_eStatus )
	{
	case STATUS_ROOM:
		{
			// Name
			XUI::CXUI_Wnd *pkItem = pkSlot->GetControl( _T("FRM_NAME") );
			if ( pkItem )
			{
				pkItem->Text(std::wstring());
			}

			// Entry
			pkItem = pkSlot->GetControl( _T("IMG_ENTRY") );
			if ( pkItem )
			{
				pkItem->Visible(false);
			}

			// Class
			pkItem = pkSlot->GetControl( _T("IMG_CLASS") );
			if ( pkItem )
			{
				pkItem->Visible(false);
			}

			// Master
			pkItem = pkSlot->GetControl(_T("IMG_MASTER"));
			if ( pkItem )
			{
				pkItem->Visible(false);
			}

			// Ready
			pkItem = pkSlot->GetControl(_T("IMG_READY"));
			if ( pkItem )
			{
				pkItem->Visible(false);
			}

			// Level
			pkItem = pkSlot->GetControl( _T("FRM_LEVEL") );
			if ( pkItem )
			{
				pkItem->Text(std::wstring());
			}

			// BattleRecord
			pkItem = pkSlot->GetControl(_T("FRM_BATTLERECORD"));
			if ( pkItem )
			{
				pkItem->Text(std::wstring());
			}

			pkItem = pkSlot->GetControl( _T("OVER_SLOT") );
			if ( pkItem )
			{
				if ( m_bMyRoom )
				{
					lua_tinker::call<void,lwGUID>("ClosePvP_ExileBox",lwGUID(pkItem->OwnerGuid()));
				}
				SPvPTeamSlot kSlot( iTeam, pkSlot->BuildIndex() );
				pkItem->SetCustomData( &kSlot, sizeof(SPvPTeamSlot) );
				pkItem->OwnerGuid(BM::GUID::NullData());
			}

			pkItem = pkSlot->GetControl( _T("IMG_CLOSE") );
			if ( pkItem )
			{
				pkItem->Visible(bClose);
			}

			/*
			pkItem = pkSlot->GetControl( _T("HSCRL_HANDYCAP") );
			if ( pkItem )
			{
				XUI::CXUI_HScroll *pkScroll = dynamic_cast<XUI::CXUI_HScroll*>(pkItem);
				if ( pkScroll )
				{
					pkScroll->CurValue(100);
				}
				pkItem->IsClosed(true);
				pkItem->Visible(!bClose);	
			}
			*/
		}break;
	case STATUS_PLAYING:
		{
			XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>( pkSlot->GetControl( _T("BAR_HP") ) );
			if ( pkBar )
			{
				pkBar->Max( 1 );
				pkBar->Now( 1 );
			}

			pkBar = dynamic_cast<XUI::CXUI_AniBar*>( pkSlot->GetControl( _T("BAR_MP") ) );
			if ( pkBar )
			{
				pkBar->Max( 1 );
				pkBar->Now( 1 );
			}

			pkSlot->OwnerGuid(BM::GUID::NullData());
			pkSlot->ClearCustomData();
			pkSlot->Visible(false);
		}break;
	}
}

DWORD PgPvPGame::GetTeamFontColor( int const iTeam )const
{
	switch ( iTeam )
	{
	case TEAM_RED:	{return 0xFFFF5842;}break;
	case TEAM_BLUE:	{return 0xFF3EA5FF;}break;
	}
	return 0xFF000000;
}

void PgPvPGame::ViewKillMsg( BM::GUID const &kKillerID, BM::GUID const &kDieID )
{
	if( PVP_TYPE_KTH != m_kRoomBaseInfo.m_kType )
	{
		return;
	}

	bool const IsKillerIsMine = g_kPilotMan.IsMyPlayer(kKillerID);
	POINT2 FormPos;

	XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Activate( L"LST_PVP_KOH__KILL_MSG" ));
	if ( !pkWnd )
	{
		return;
	}

	XUI::SListItem*	pkItem = pkWnd->AddItem(_T(""));
	if( !pkItem || !pkItem->m_pWnd )
	{
		return;
	}

	float const fNowTime = g_pkApp->GetAccumTime();
	pkItem->m_pWnd->SetCustomData(&fNowTime, sizeof(fNowTime));
	FormPos.x = pkWnd->Location().x;
	FormPos.y = pkWnd->Location().y + (pkItem->m_pWnd->Size().y * (pkWnd->GetTotalItemCount() - 1)) + (pkItem->m_pWnd->Size().y * 0.5f);

	XUI::CXUI_Wnd*	pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_ICON"));
	if( pkTemp )
	{
		SUVInfo	Info = pkTemp->UVInfo();
		Info.Index = (BM::GUID::IsNull(kKillerID))?(KMM_SELFKILL):(KMM_TARGETKILL);
		pkTemp->UVInfo(Info);
	}

	pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_KILLER"));
	if( pkTemp )
	{
		bool bFind = BM::GUID::IsNotNull( kKillerID );

		if ( true == bFind )
		{
			CONT_PVP_GAME_USER::const_iterator user_itr = m_kContPvPUser.find( kKillerID );
			if ( user_itr != m_kContPvPUser.end() )
			{
				pkTemp->FontColor( (IsKillerIsMine)?(0xFFFFE98E):(GetTeamFontColor(user_itr->second.kTeamSlot.kTeam)) );
				pkTemp->Text(user_itr->second.wstrName);
			}
			else
			{
				bFind = false;
			}
		}

		if ( !bFind )
		{
			// 자살
			pkTemp->Text( std::wstring() );
		}
	}

	pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_INJURE"));
	if( pkTemp )
	{
		bool const IsMyActor = g_kPilotMan.IsMyPlayer(kDieID);

		CONT_PVP_GAME_USER::const_iterator user_itr = m_kContPvPUser.find( kDieID );
		if ( user_itr != m_kContPvPUser.end() )
		{
			pkTemp->FontColor( (IsMyActor)?(0xFFFFE98E):(GetTeamFontColor(user_itr->second.kTeamSlot.kTeam)) );
			pkTemp->Text(user_itr->second.wstrName);
		}
	}

	int const iMaxListItem = pkWnd->GetTotalItemCount();
	for(int i = 0; i < (iMaxListItem - MAX_KILL_NFY_MSG); ++i)
	{
		pkWnd->DeleteItem(pkWnd->FirstItem());
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_GAME_EVENT_CHANGEPOINT( BM::Stream &kPacket )
{
	CONT_SEND_PVP_POINT ContPvPPoint;
	PU::TLoadTable_AM( kPacket, ContPvPPoint );

	CONT_SEND_PVP_POINT::iterator iter;
	for( iter = ContPvPPoint.begin(); iter != ContPvPPoint.end(); ++iter )
	{
		CONT_PVP_GAME_USER::mapped_type *pUser = Get(iter->first);
		if( pUser )
		{
			pUser->kResult.SetPoint(iter->second.iPoint);
			RefreshUserPoint( iter->first, *pUser );
		}
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_GAME_EVENT_KILL( BM::Stream &kPacket )
{
	BM::GUID kDieGuid;
	int iDeadPoint = 0;
	BM::GUID kKillGuid;
	int iKillPoint = 0;
	kPacket.Pop( kDieGuid );
	kPacket.Pop( iDeadPoint );
	kPacket.Pop( kKillGuid );
	kPacket.Pop( iKillPoint );

	CONT_PVP_GAME_USER::mapped_type *pkDie = Get(kDieGuid);
	if ( pkDie )
	{
		pkDie->kResult.Dead(iDeadPoint);
		if ( PVP_TYPE_ANNIHILATION == m_kRoomBaseInfo.m_kType )
		{
			pkDie->byStatus |= PS_DEAD;
		}
		RefreshUserPoint( kDieGuid, *pkDie );
	}

	CONT_PVP_GAME_USER::mapped_type *pkKill = Get(kKillGuid);
	if ( pkKill )
	{		
		pkKill->kResult.Kill(iKillPoint);
		RefreshUserPoint( kKillGuid, *pkKill );

		PgActor *pkDieActor = g_kPilotMan.FindActor(kDieGuid);
		if ( pkDieActor )
		{
			std::wstring wstrTalk;
			WstringFormat(wstrTalk, MAX_PATH, TTW(400341).c_str(), pkKill->wstrName.c_str() );
			pkDieActor->ShowChatBalloon(CT_NORMAL, wstrTalk, 5000);

			if ( pkDieActor->IsMyActor() )
			{
				std::wstring kMsg;
				WstringFormat( kMsg, MAX_PATH, TTW(400410).c_str(), pkKill->wstrName.c_str() );
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage( kChatLog, kMsg );

				m_iKillCount = 0;
			}
		}

		PgActor *pkKillerActor = g_kPilotMan.FindActor(kKillGuid);
		if ( pkKillerActor )
		{
			if ( true == pkKillerActor->IsMyActor() )
			{
				lua_tinker::call< void, size_t >("CallPvPKillCount", ++m_iKillCount );
			}
// 			else
// 			{
// 				pkKillerActor->ShowExpNum(usKillPoint);
// 			}
		}
	}

	ViewKillMsg(kKillGuid, kDieGuid);
}

void PgPvPGame::Recv_PT_M_C_NFY_GAME_END( BM::Stream &kPacket )
{
	if ( SetStatus(STATUS_ENDBEFORE) )
	{
		DrawTime(0);

		// 캐릭터 고정
		SetPvPLock( true );

		bool bTimeOver = false;
		int iWinTeam = TEAM_NONE;
		bool bEndGame = false;
		kPacket.Pop(bTimeOver);
		kPacket.Pop(iWinTeam);
		kPacket.Pop(bEndGame);
		++m_ucRoundScore[iWinTeam];

		XUI::CXUI_Wnd *pkWnd = NULL;
		if ( XUIMgr.IsActivate( ms_FRM_PVP_MAIN, pkWnd ) )
		{
			XUI::CXUI_Wnd *pLoveRound = pkWnd->GetControl(_T("FRM_LOVE_ROUND"));
			if( pLoveRound && pLoveRound->Visible() )
			{
				BM::vstring FormName("SLOT_HEART_");
				if( TEAM_RED == iWinTeam )
				{
					FormName += "RED_";
				}
				else if ( TEAM_BLUE == iWinTeam )
				{
					FormName += "BLUE_";
				}
				FormName += ((int)m_ucRoundScore[iWinTeam] - 1);
				XUI::CXUI_Wnd *pHeart = pLoveRound->GetControl((static_cast<std::wstring>(FormName).c_str()));
				if( pHeart )
				{
					pHeart->Visible( true );
					//pHeart->UVUpdate( 2 );
				}
			}
			XUI::CXUI_Wnd *pkRound = pkWnd->GetControl(_T("FRM_ROUND"));
			if ( pkRound && pkRound->Visible() )
			{
				XUI::CXUI_Wnd *pkRoundScore = pkRound->GetControl(_T("FRM_ROUND_SCORE"));
				if ( pkRoundScore && pkRoundScore->Visible() )
				{
					wchar_t wszTemp[MAX_PATH] = {0,};
					::swprintf_s( wszTemp, MAX_PATH, L"IMG_NUM10_%d", iWinTeam );
					XUI::CXUI_Wnd *pkTemp = pkRoundScore->GetControl( std::wstring(wszTemp) );
					if ( pkTemp )
					{
						pkTemp->UVUpdate( m_ucRoundScore[iWinTeam]/10 +1 );
					}

					::swprintf_s( wszTemp, MAX_PATH, L"IMG_NUM01_%d", iWinTeam );
					pkTemp = pkRoundScore->GetControl( std::wstring(wszTemp) );
					if ( pkTemp )
					{
						pkTemp->UVUpdate( m_ucRoundScore[iWinTeam]%10 +1 );
					}
				}
			}
		}
		
		if( IsLoveMode() )
		{// 러브모드 라운드가 끝났을 때만 나오는 사운드
			lwPlaySoundByID("Couple_Marriage02");
		}

		if ( true == bEndGame )
		{
			int iValue = -1;
			if ( true == bTimeOver )
			{
				pkWnd = XUIMgr.Call(_T("FRM_PVP_TIMEOVER"));
				if ( pkWnd )
				{
					pkWnd->SetCustomData( &iValue, sizeof(int) );
				}
			}
			else
			{
				pkWnd = XUIMgr.Call(_T("FRM_PVP_GAMESET"));
				if ( pkWnd )
				{
					pkWnd->SetCustomData( &iValue, sizeof(int) );
				}
			}
		}
		else
		{
			BM::vstring vstrMsg( m_ucNowRound );
			vstrMsg += TTW(400354);
			vstrMsg += TTW(400385+iWinTeam);
			lua_tinker::call< void, lwWString >("CallPvPMessage", lwWString( vstrMsg.operator std::wstring const&() ) );
		}
	}
}

void PgPvPGame::Recv_PT_M_C_NFY_GAME_RESULT( BM::Stream &kPacket )
{
	if ( SetStatus(STATUS_RESULT) )
	{
		m_kReward.clear();
		int iWinTeam = TEAM_NONE;
		kPacket.Pop(iWinTeam);
		PU::TLoadArray_M( kPacket, m_kReward );

		XUI::CXUI_Wnd *pkWnd = NULL;
		if ( XUIMgr.IsActivate( _T("FRM_PVP_TIMEOVER"), pkWnd ) )
		{
			pkWnd->SetCustomData( &iWinTeam, sizeof(int) );
			return;
		}

		if ( XUIMgr.IsActivate( _T("FRM_PVP_GAMESET"), pkWnd ) )
		{
			pkWnd->SetCustomData( &iWinTeam, sizeof(int) );
			return;
		}

		Result( iWinTeam );
	}
}

void PgPvPGame::Recv_PT_T_C_NFY_RELOAD_ROOM( BM::Stream &kPacket )
{
	if (	((m_eStatus == STATUS_RESULT) || (m_eStatus == STATUS_READY))
		&&	SetStatus( STATUS_ROOM )
		)
	{
		CONT_PVP_SLOT kContSlot;
		m_kContPvPUser.clear();
		PU::TLoadTable_AM( kPacket, m_kContPvPUser );
		PU::TLoadKey_A( kPacket, kContSlot );

		XUIMgr.Close(ms_FRM_PVP_RESULT);
		XUIMgr.Close(ms_FRM_PVP_RESULT_ITEM);

		// 이모션을 종료해야 한다.
		if ( g_pkWorld )
		{
			PgCameraModeFollow *pkMode = dynamic_cast<PgCameraModeFollow*>(g_pkWorld->m_kCameraMan.GetCameraMode());
			if ( pkMode )
			{
				pkMode->SetCameraFixed( false );
			}

			CONT_PVP_GAME_REWARD::const_iterator itr;
			for ( itr=m_kReward.begin(); itr!=m_kReward.end(); ++itr )
			{
				PgActor *pkActor = g_kPilotMan.FindActor( itr->kCharGuid );
				if ( pkActor )
				{
					pkActor->ReserveTransitAction(ACTIONNAME_IDLE);
					if ( pkActor->IsMyActor() )
					{
						g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
					}
				}
			}
		}
		PgCameraModeFollow::SetCameraZoom(m_fCameraZoom);
		m_kReward.clear();

		lwCloseAllUI(true);
		g_kChatMgrClient.ChatStation(ECS_PVPLOBBY);
		lwCallChatWindow(1);
		OpenRoom( kContSlot );
		UpdateLobbyList( kPacket );
	}
}

void PgPvPGame::Recv_PT_T_C_ANS_SLOTSTATUS_CHANGE( BM::Stream &kPacket )
{
	bool bOpen = false;
	kPacket.Pop( bOpen );

	XUI::CXUI_Wnd *pkRoom = XUIMgr.Get( ms_FRM_PVP_ROOM[m_kRoomBaseInfo.m_kMode] );
	
	size_t const iLastSize = sizeof(m_kRoomBaseInfo.m_ucMaxUser);
	XUI::CXUI_Wnd *pkSlot = NULL;
	SPvPTeamSlot kSlot;
	while ( kPacket.RemainSize() > iLastSize )
	{
		kSlot.ReadFromPacket( kPacket );
		pkSlot = GetSlot( pkRoom, kSlot.GetSlot(), kSlot.GetTeam() );
		if ( pkSlot )
		{
			ClearSlot( pkSlot, !bOpen, kSlot.GetTeam() );
		}
	}

	kPacket.Pop( m_kRoomBaseInfo.m_ucMaxUser );
}

void PgPvPGame::Recv_PT_PM_C_NFY_HILL_TEAM( BM::Stream &kPacket )
{
	if ( (PVP_TYPE_KTH  == m_kRoomBaseInfo.m_kType) && (m_eStatus == STATUS_PLAYING) )
	{
		int iHillTeam = TEAM_NONE;
		kPacket.Pop(iHillTeam);

		XUI::CXUI_Wnd *pkWnd = NULL;
		if ( XUIMgr.IsActivate( ms_FRM_PVP_MAIN, pkWnd ) )
		{
			XUI::CXUI_Wnd *pkMsg1 = pkWnd->GetControl(_T("FRM_MESSAGE_1"));
			XUI::CXUI_Wnd *pkMsg2 = pkWnd->GetControl(_T("FRM_MESSAGE_2"));
			if ( pkMsg1 && pkMsg2 )
			{
				switch( iHillTeam )
				{
				case TEAM_RED:
					{
						std::wstring wstrMessage1;
						WstringFormat( wstrMessage1, MAX_PATH, TTW(400391).c_str(), TTW(400408).c_str() );
						pkMsg1->Text(wstrMessage1);
					}break;
				case TEAM_BLUE:
					{
						std::wstring wstrMessage1;
						WstringFormat( wstrMessage1, MAX_PATH, TTW(400391).c_str(), TTW(400409).c_str() );
						pkMsg1->Text(wstrMessage1);
					}break;
				default:
					{
						pkMsg1->Visible(false);
						pkMsg2->Visible(false);
						return;
					}break;
				}
				
				pkMsg1->Visible(true);
				pkMsg2->Visible(true);
			}		
		}
	}
}

void PgPvPGame::UpdatePoint( XUI::CXUI_Wnd *pkWnd, XUI::CXUI_Wnd *pkGraphWnd )
{
	if ( pkWnd )
	{
		int iPoint[2] = {0,};
		pkWnd->GetCustomData( iPoint, sizeof(iPoint) );
		if ( !iPoint[1] )
		{
			return;
		}

		wchar_t wszTemp[MAX_PATH] = {0,};
		XUI::CXUI_Wnd *pkTemp = NULL;

		++iPoint[0];
		bool bView = --iPoint[1] > 0;
		pkWnd->SetCustomData( &iPoint, sizeof(iPoint) );

		// Graph
		if ( pkGraphWnd )
		{
			int const iIndex = iPoint[0] / 100;
			swprintf_s( wszTemp, 50, L"IMG_%d", iIndex );
			pkTemp = pkGraphWnd->GetControl( std::wstring(wszTemp) );
			if ( pkTemp )
			{
				pkTemp->Visible( true );
			}
		}

		bool bStart = true;
		int iPos = 10000;
		while ( iPos > 1 )
		{
			iPoint[0] %= iPos;
			iPoint[1] %= iPos;
			iPos /= 10;

			swprintf_s( wszTemp, 50, L"IMG_POINT_%04u", iPos );
			pkTemp = pkWnd->GetControl( std::wstring(wszTemp) );
			if ( pkTemp )
			{
				size_t const uv = static_cast<size_t>(iPoint[0] / iPos) + 1;
				pkTemp->UVUpdate(uv);
			}

			swprintf_s( wszTemp, 50, L"IMG_POINT_PLUS_%04u", iPos );
			pkTemp = pkWnd->GetControl( std::wstring(wszTemp) );
			if ( pkTemp )
			{
				pkTemp->Visible(bView);
				if ( bStart )
				{
					pkTemp->UVUpdate(11);
					bStart = false;
				}
				else if ( bView )
				{
					size_t const uv = static_cast<size_t>(iPoint[1] / iPos) + 1;
					pkTemp->UVUpdate(uv);
				}	
			}	
		}
	}
}

void PgPvPGame::AddPoint( int const iPoint, int const iTeam )
{
	if ( (TEAM_RED != iTeam) && (TEAM_BLUE != iTeam) )
	{
		return;
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( ms_FRM_PVP_MAIN );
	if ( pkWnd )
	{
		wchar_t wszTemp[MAX_PATH] = {0,};
		swprintf_s( wszTemp, MAX_PATH, L"FRM_POINT_%d", iTeam );
		pkWnd = pkWnd->GetControl( std::wstring(wszTemp) );
		if ( pkWnd )
		{
			int _iPoint[2] = {0,};
			pkWnd->GetCustomData( _iPoint, sizeof(_iPoint) );
			_iPoint[1] += iPoint;
			pkWnd->SetCustomData( _iPoint, sizeof(_iPoint) );
		}
	}
}

void PgPvPGame::SortSlot( int iIndex, int iOldIndex, int const iTeam )
{	
	XUI::CXUI_Wnd *pkSlot = GetSlot( NULL, iIndex, iTeam );
	if ( pkSlot )
	{
		if ( true == pkSlot->Visible() )
		{
			if ( iIndex != iOldIndex )
			{
				XUI::CXUI_Wnd *pkOldSlot = GetSlot( NULL, iOldIndex, iTeam );

				CONT_PVP_GAME_USER::mapped_type* pkNewSlotUserInfo = Get( pkSlot->OwnerGuid() );
				if ( pkNewSlotUserInfo )
				{
					pkNewSlotUserInfo->wstrUI = pkOldSlot->ID();
					OpenSlot( pkSlot->OwnerGuid(), *pkNewSlotUserInfo, pkOldSlot );
					ClearSlot( pkSlot, true, iTeam );
				}
			}
			++iOldIndex;
		}

		SortSlot( iIndex+1, iOldIndex, iTeam );
	}
}

void PgPvPGame::SortSlot( XUI::CXUI_Wnd*& pkSlot, int const iTeam, int const iPoint, bool const bInc )
{
	XUI::CXUI_Wnd *pkNextSlot = NULL;
	while ( 1 )
	{
		if ( bInc )
		{
			pkNextSlot = GetSlot( NULL, pkSlot->BuildIndex()-1, iTeam );
		}
		else
		{
			pkNextSlot = GetSlot( NULL, pkSlot->BuildIndex()+1, iTeam );
		}

		if ( pkNextSlot )
		{
			CONT_PVP_GAME_USER::const_iterator UserIter = m_kContPvPUser.find(pkNextSlot->OwnerGuid());
			if( m_kContPvPUser.end() != UserIter )
			{
				pkNextSlot->Visible(true);
				
				int iSlotPoint = 0;
				pkNextSlot->GetCustomData( &iSlotPoint, sizeof(iSlotPoint) );
				if (	(bInc && iSlotPoint < iPoint)
					||	(!bInc && iSlotPoint >= iPoint ) )
				{
					// 바꿔치기 해야 한다.
					CONT_PVP_GAME_USER::mapped_type* pkNewSlotUserInfo = Get( pkNextSlot->OwnerGuid() );
					if ( pkNewSlotUserInfo )
					{
						pkNewSlotUserInfo->wstrUI = pkSlot->ID();
						OpenSlot( pkNextSlot->OwnerGuid(), *pkNewSlotUserInfo, pkNextSlot );
						pkSlot = pkNextSlot;
						continue;
					}	
				}
			}
			else
			{
				if ( bInc )
				{

					// 바꿔치기 해야 한다.
					CONT_PVP_GAME_USER::mapped_type* pkNewSlotUserInfo = Get( pkSlot->OwnerGuid() );
					if ( pkNewSlotUserInfo )
					{
						pkNewSlotUserInfo->wstrUI = pkNextSlot->ID();
						OpenSlot( pkSlot->OwnerGuid(), *pkNewSlotUserInfo, pkSlot );
						ClearSlot( pkSlot, true, iTeam );// bClose true로 해야 한다(안그럼 무한루프)
						pkSlot = pkNextSlot;
						continue;
					}	
				}
			}
		}
		break;
	}
}

void PgPvPGame::RefreshUserPoint( BM::GUID const &kCharGuid, CONT_PVP_GAME_USER::mapped_type &kUserInfo )
{
	XUI::CXUI_Wnd* pkUserSlot = GetSlotWnd( kUserInfo );
	if ( pkUserSlot )
	{
		int iOldPoint = 0;
		pkUserSlot->GetCustomData( &iOldPoint, sizeof(iOldPoint) );

		// 포인트의 변동이 없으면 정렬은 하지 않는다.
		if ( kUserInfo.kResult.iPoint != iOldPoint )
		{
			SortSlot( pkUserSlot, kUserInfo.kTeamSlot.GetTeam(), kUserInfo.kResult.iPoint, kUserInfo.kResult.iPoint > iOldPoint );
		}
		
		kUserInfo.wstrUI = pkUserSlot->ID();
		OpenSlot( kCharGuid, kUserInfo, NULL );	
	}
}

bool PgPvPGame::SyncPoint( int const iPoint, int const iTeam )
{
	if ( (TEAM_RED != iTeam) && (TEAM_BLUE != iTeam) )
	{
		return false;
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( ms_FRM_PVP_MAIN );
	if ( pkWnd )
	{
		wchar_t wszTemp[MAX_PATH] = {0,};
		swprintf_s( wszTemp, MAX_PATH, L"FRM_POINT_%d", iTeam );
		XUI::CXUI_Wnd* pkPointWnd = pkWnd->GetControl( std::wstring(wszTemp) );
		if ( pkPointWnd )
		{
			int _iPoint[2] = {0,};
			pkPointWnd->GetCustomData( _iPoint, sizeof(_iPoint) );
			_iPoint[0] = iPoint;
			_iPoint[1] = 0;
			pkPointWnd->SetCustomData( _iPoint, sizeof(_iPoint) );

			XUI::CXUI_Wnd *pkTemp = NULL;
			int iPos = 10000;
			while ( iPos > 1 )
			{
				_iPoint[0] %= iPos;
				iPos /= 10;

				swprintf_s( wszTemp, 50, L"IMG_POINT_%04u", iPos );
				pkTemp = pkPointWnd->GetControl( std::wstring(wszTemp) );
				if ( pkTemp )
				{
					size_t uv = _iPoint[0] / iPos + 1;
					pkTemp->UVUpdate(uv);
				}

				swprintf_s( wszTemp, 50, L"IMG_POINT_PLUS_%04u", iPos );
				pkTemp = pkPointWnd->GetControl( std::wstring(wszTemp) );
				if ( pkTemp )
				{
					pkTemp->Visible(false);
				}
			}

			// Graph
			swprintf_s( wszTemp, 50, L"FRM_ENERGY_%d", iTeam );
			XUI::CXUI_Wnd* pkEnergy = pkWnd->GetControl( std::wstring(wszTemp) );
			if ( pkEnergy )
			{
				int iTemp = iPoint;
				for ( int j=1; j<11; ++j )
				{
					iTemp -= 100;
					swprintf_s( wszTemp, 50, L"IMG_%d", j );
					pkTemp = pkEnergy->GetControl( std::wstring(wszTemp) );
					if ( pkTemp )
					{
						pkTemp->Visible( iTemp > -1 );
					}
				}
			}
		}
	}
	return true;
}

void PgPvPGame::CountDown( DWORD const dwRemainTime )
{
	if ( SetStatus(STATUS_COUNTDOWN) )
	{
		XUIMgr.CloseAll();

		if(PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType)
		{
			BM::Stream kPacket;
			kPacket.Push(1);	//m_ucNowRound : 1
			Start( kPacket );

			lua_tinker::call<void, DWORD>("CallPvPCountDown_2", (dwRemainTime-1000)/1000);
		}
		else
		{
			// 캐릭터 고정
			SetPvPLock( true );
			lua_tinker::call<void, DWORD>("CallPvPCountDown", (dwRemainTime-1000)/1000);
		}
	}
}

void PgPvPGame::Start( BM::Stream& kPacket )
{
	if ( SetStatus(STATUS_PLAYING) )
	{
		g_kChatMgrClient.ChatStation(ECS_COMMON);
		lwCallChatWindow(1);

		kPacket.Pop(m_ucNowRound);

		XUIMgr.CloseAll();
		lua_tinker::call<void>("UI_BaseActivate");

		// 캐릭터 고정 해제
		SetPvPLock( false );
		g_kChatMgrClient.ShowNotice();

		if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType
			|| PVP_TYPE_LOVE == m_kRoomBaseInfo.m_kType )
		{
			lwOnCallCenterMiniMap(lwGUID(BM::GUID()));
		}

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( pPlayer )
		{
			pPlayer->BearEffect(false);
		}

		XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate(ms_FRM_PVP_MAIN);
		if ( pkWnd )
		{
			XUI::CXUI_Wnd *pkTemp = NULL;
			XUI::CXUI_Wnd *pkRound = pkWnd->GetControl(_T("FRM_ROUND"));
			if ( pkRound )
			{
				if ( IsRoundCount()	&& !IsLoveMode() )
				{// 러브모드는 다른 UI를 쓴다.
					pkRound->Visible(true);
					pkTemp = pkRound->GetControl( _T("IMG_NUM10") );
					if ( pkTemp )
					{
						pkTemp->UVUpdate( m_ucNowRound/10 + 1 );
					}

					pkTemp = pkRound->GetControl( _T("IMG_NUM01") );
					if ( pkTemp )
					{
						pkTemp->UVUpdate( m_ucNowRound%10 + 1 );
					}

					XUI::CXUI_Wnd *pkRoundScore = pkRound->GetControl(_T("FRM_ROUND_SCORE"));
					if ( pkRoundScore )
					{
						if ( PVP_MODE_PERSONAL == m_kRoomBaseInfo.m_kMode )
						{
							pkRoundScore->Visible(false);
						}
						else
						{
							pkRoundScore->Visible(true);

							wchar_t wszTemp[MAX_PATH] = {0,};
							for( int i=TEAM_NONE; i<TEAM_MAX; ++ i )
							{
								::swprintf_s( wszTemp, MAX_PATH, L"IMG_NUM10_%d", i );
								pkTemp = pkRoundScore->GetControl( std::wstring(wszTemp) );
								if ( pkTemp )
								{
									pkTemp->UVUpdate( m_ucRoundScore[i]/10 +1 );
								}

								::swprintf_s( wszTemp, MAX_PATH, L"IMG_NUM01_%d", i );
								pkTemp = pkRoundScore->GetControl( std::wstring(wszTemp) );
								if ( pkTemp )
								{
									pkTemp->UVUpdate( m_ucRoundScore[i]%10 +1 );
								}
							}
						}
					}		
				}
				else
				{
					pkRound->Visible(false);
				}
				lua_tinker::call< void, lwUIWnd, EPVPTYPE>("OnCall_FRM_PVP_MAIN", lwUIWnd(pkWnd), m_kRoomBaseInfo.m_kType );
			}

			XUI::CXUI_Wnd *pLoveRound = pkWnd->GetControl(_T("FRM_LOVE_ROUND"));
			if ( pLoveRound && IsLoveMode() )
			{
				pLoveRound->Visible(true);

				lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 1, -1, false);	// 타이머 초기화
				lua_tinker::call<void, int, int, bool>("SetLoveBearTimer", 2, -1, false);	// 타이머 초기화

				XUI::CXUI_Image * pRoundNum = dynamic_cast<XUI::CXUI_Image*>(pLoveRound->GetControl(L"IMG_ROUNDNUM"));
				if( pRoundNum )
				{
					pRoundNum->UVUpdate( m_ucNowRound%10 + 1 );
				}

				XUI::CXUI_Builder * pRedHeart = dynamic_cast<XUI::CXUI_Builder*>(pLoveRound->GetControl(L"BLD_SLOT_HEART_RED"));
				if( pRedHeart )
				{
					int TotalCount = pRedHeart->CountX() * pRedHeart->CountY();
					for( int index = 0; index < TotalCount; ++index )
					{
						BM::vstring FormName("SLOT_HEART_RED_");
						FormName += index;
						XUI::CXUI_Wnd *pHeart = pLoveRound->GetControl((static_cast<std::wstring>(FormName).c_str()));
						if( pHeart )
						{
							if( index < m_ucRoundScore[TEAM_RED] )
							{// 승리한 라운드면 불을 키고
								pHeart->Visible( true );
								//pHeart->UVUpdate( 1 );
							}
							else
							{
								int RealCount = m_kRoomExtInfo.ucRound / 2;
								if( index <= RealCount )
								{// 아니면 불을 끈다.
									pHeart->Visible( false );
									//pHeart->UVUpdate( 2 );
								}
								else
								{// 그 이외에는 사용할 수 없는 슬롯
									pHeart->Visible( false );
									//pHeart->UVUpdate( 3 );
								}
							}
						}
					}
				}
				XUI::CXUI_Builder * pBlueHeart = dynamic_cast<XUI::CXUI_Builder*>(pLoveRound->GetControl(L"BLD_SLOT_HEART_BLUE"));
				if( pBlueHeart )
				{
					int TotalCount = pBlueHeart->CountX() * pBlueHeart->CountY();
					for( int index = 0; index < TotalCount; ++index )
					{
						BM::vstring FormName("SLOT_HEART_BLUE_");
						FormName += index;
						XUI::CXUI_Wnd *pHeart = pLoveRound->GetControl((static_cast<std::wstring>(FormName).c_str()));
						if( pHeart )
						{
							if( index < m_ucRoundScore[TEAM_BLUE] )
							{// 승리한 라운드면 불을 키고
								pHeart->Visible( true );
								//pHeart->UVUpdate( 1 );
							}
							else
							{
								int RealCount = m_kRoomExtInfo.ucRound / 2;
								if( index <= RealCount )
								{// 아니면 불을 끈다.
									pHeart->Visible( false );
									//pHeart->UVUpdate( 2 );
								}
								else
								{// 그 이외에는 사용할 수 없는 슬롯
									pHeart->Visible( false );
									//pHeart->UVUpdate( 3 );
								}
							}
						}
					}
				}
			}
			else
			{
				pLoveRound->Visible(false);
			}

			if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
			{//승리포인트 초기화
				lua_tinker::call<void, int, int>("OnUpdate_Team_TotalPoint", TEAM_RED, m_kRoomBaseInfo.m_iGamePoint);
				lua_tinker::call<void, int, int>("OnUpdate_Team_TotalPoint", TEAM_BLUE, m_kRoomBaseInfo.m_iGamePoint);
			}

			InitSlot();
			CONT_PVP_GAME_USER::iterator user_itr = m_kContPvPUser.begin();
			for ( ; user_itr!=m_kContPvPUser.end(); ++user_itr )
			{
				user_itr->second.byStatus &= (~PS_DEAD);
				OpenSlot( user_itr->first, user_itr->second, NULL );
			}

			if ( PVP_MODE_TEAM == m_kRoomBaseInfo.m_kMode )
			{
				SortSlot( 0, 0, TEAM_RED );
				SortSlot( 0, 0, TEAM_BLUE );
			}
			else
			{
				SortSlot( 0, 0, TEAM_NONE );
			}

			// Global HotKey On
			XUIMgr.BlockGlobalScript(false);
		}
	}
}

void PgPvPGame::Result( int const iWinTeam )
{
	PG_ASSERT_LOG(g_pkWorld);
	if ( STATUS_RESULT == m_eStatus && g_pkWorld )
	{
		m_fCameraZoom = PgCameraModeFollow::GetCameraZoom();
		if ( m_fCameraZoom > 1.0f )
		{
			PgCameraModeFollow::SetCameraZoom(1.0f);
		}

		if(g_pkWorld && IsKTHMode())
		{
			g_pkWorld->m_kCameraMan.SaveCameraPos();
			//g_world:SetCameraZoomMax(zoom)
			PgCameraModeFollow::SetCameraZoom(0.15f);
		}

		m_iKillCount = 0;// KillCount 초기화

		//Global HotKey
		XUIMgr.BlockGlobalScript(true);
		
		XUIMgr.Close( ms_FRM_PVP_MAIN );

		// 내 정보를 찾아야 한다.
		int iWinSpawn = 1;
		int iLoseSpawn = 1;

		char akSpawnName[30] = {0,};
		CONT_PVP_GAME_USER::mapped_type *pkElement = NULL;
		PgActor *pkActor = NULL;

		bool bFollowCamera = false;
		CONT_PVP_GAME_REWARD::const_iterator itr;
		for ( itr=m_kReward.begin(); itr!=m_kReward.end(); ++itr )
		{
			int iWinLoseImg = 0;
			pkElement = Get(itr->kCharGuid);
			if ( pkElement )
			{
				pkActor = g_kPilotMan.FindActor( itr->kCharGuid );
				if ( pkActor )
				{

					if ( PVP_MODE_PERSONAL == m_kRoomBaseInfo.m_kMode )
					{
						iWinLoseImg = (1 == itr->kRank) ? 1 : 2; 
						if ( g_pkWorld
							&& iWinSpawn < 5 
							)
						{
							if( g_kLocal.ServiceRegion() != LOCAL_MGR::NC_USA )
							{
								if ( 1 == iWinSpawn ){g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);}
							}
							::sprintf_s( akSpawnName, 30, "win_spawn_%d", iWinSpawn++ );
						}
						else
						{
							::sprintf_s( akSpawnName, 30, "lose_spawn_%d", iLoseSpawn++ );
						}
					}
					else
					{
						//카메라 줌
						if( NULL != g_pkWorld
							&& g_kLocal.ServiceRegion() != LOCAL_MGR::NC_USA )
						{
							if(IsKTHMode())
							{
								if( true == g_kPilotMan.IsMyPlayer(itr->kCharGuid) )
								{
									g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
								}
							}
							else
							{
								if ( false==bFollowCamera && pkElement->kTeamSlot.GetTeam() == iWinTeam )
								{
									g_pkWorld->m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
									bFollowCamera = true;
								}
							}
						}

						if ( TEAM_NONE != iWinTeam )
						{
							iWinLoseImg = ( pkElement->kTeamSlot.GetTeam() == iWinTeam ) ? 1 : 2;
						}

						switch ( iWinLoseImg )
						{
						case 0:
							{
								if ( g_pkWorld
									&& pkElement->kTeamSlot.GetTeam() == TEAM_RED 
									)
								{
									::sprintf_s( akSpawnName, 30, "win_spawn_%d", iWinSpawn++ );
								}
								else
								{
									::sprintf_s( akSpawnName, 30, "lose_spawn_%d", iLoseSpawn++ );
								}
							}break;
						case 1:
							{
								::sprintf_s( akSpawnName, 30, "win_spawn_%d", iWinSpawn++ );
							}break;
						case 2:
							{
								::sprintf_s( akSpawnName, 30, "lose_spawn_%d", iLoseSpawn++ );
							}break;
						}
					}

					if( g_kLocal.ServiceRegion() == LOCAL_MGR::NC_USA )
					{
						if( true == g_kPilotMan.IsMyPlayer(itr->kCharGuid) )
						{
							char const* pkMotionName = iWinLoseImg == 1 ? "a_pvp_win" : "a_pvp_lose";
							if(PgAction * pkAction = pkActor->GetAction())
							{
								pkActor->CancelAction(pkAction->GetActionNo(), pkAction->GetActionInstanceID(), pkMotionName );
							}
							else
							{
								pkActor->ReserveTransitAction( pkMotionName );
							}
						}
					}
					else
					{
						NiPoint3 kSpawnLoc;
						if ( g_pkWorld
							&& g_pkWorld->FindSpawnLoc(akSpawnName, kSpawnLoc) 
							)
						{
							pkActor->SetPosition( kSpawnLoc );
						}

						char const* pkMotionName = iWinLoseImg == 1 ? "a_pvp_win" : "a_pvp_lose";
						if(PgAction * pkAction = pkActor->GetAction())
						{
							pkActor->CancelAction(pkAction->GetActionNo(), pkAction->GetActionInstanceID(), pkMotionName );
						}
						else
						{
							pkActor->ReserveTransitAction( pkMotionName );
						}
					}
				}

				// 내 플레이어이면
				if ( g_kPilotMan.IsMyPlayer(itr->kCharGuid) )
				{
					XUIMgr.Close(L"SFRM_BIG_MAP");
					XUI::CXUI_Wnd *pkWnd = XUIMgr.Call( ms_FRM_PVP_RESULT );
					if ( pkWnd )
					{
						wchar_t wszTemp[MAX_PATH] = {0,};
						XUI::CXUI_Wnd* pkTemp = NULL;

						int iImg = 0;
						while( iImg < TEAM_MAX )
						{
							::swprintf_s( wszTemp, MAX_PATH, L"IMG_WINLOSE_%d", iImg );
							pkTemp = pkWnd->GetControl( std::wstring(wszTemp) );
							if ( pkTemp )
							{
								pkTemp->Visible( iWinLoseImg == iImg );
							}
							++iImg;
						}

						// 전투포인트
						pkTemp = pkWnd->GetControl( _T("FRM_BATTLE_POINT") );
						if ( pkTemp )
						{
							XUI::CXUI_Wnd * pBP = pkWnd->GetControl(_T("SFRM_BATTEL_POINT"));
							if( pBP )
							{
								if( IsKTHMode() )
								{
									pBP->Text(TTW(400289));
								}
								else if( IsLoveMode() )
								{
									pBP->Text(TTW(400288));
								}
								else
								{
									pBP->Text(TTW(400414));
								}
							}
							::swprintf_s( wszTemp, MAX_PATH, L"%d", itr->kResult.iPoint );
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// 순위포인트
						pkTemp = pkWnd->GetControl( _T("FRM_RANK_POINT") );
						if ( pkTemp )
						{
							::swprintf_s( wszTemp, 30, L"%u", itr->kRankPoint );
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// 우승보너스
						pkTemp = pkWnd->GetControl( _T("FRM_WIN_POINT") );
						if ( pkTemp )
						{
							::swprintf_s( wszTemp, 30, L"%u", itr->kWinPoint);
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// 특별보너스
						pkTemp = pkWnd->GetControl( _T("FRM_SPECIAL_POINT") );
						if ( pkTemp )
						{
							::swprintf_s( wszTemp, 30, L"%u", itr->kSpecialPoint );
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// 종합포인트
						pkTemp = pkWnd->GetControl( _T("FRM_TOTAL_POINT") );
						if ( pkTemp )
						{
							int Point = 0;
							if( IsKTHMode() )
							{
								Point = itr->GetTotalPoint() + itr->kResult.iCapturePoint;
							}
							else
							{
								Point = itr->GetTotalPoint();
							}
							::swprintf_s( wszTemp, 30, L"%d",  Point);
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// CP
						pkTemp = pkWnd->GetControl( _T("FRM_CP") );
						if ( pkTemp )
						{
							// CP는 10을 곱해서 보여준다.
							::swprintf_s( wszTemp, 30, L"%d", itr->iCP * CP_VALUE );
							pkTemp->Text( std::wstring(wszTemp) );
						}

						// 아이템을 그려야 한다~
						if ( itr->kContItem.size() )
						{
							pkWnd = XUIMgr.Call( ms_FRM_PVP_RESULT_ITEM );
							if ( pkWnd )
							{
								int index = 1;

								SEnchantInfo kEnchantInfo;
								std::wstring wstrItemMsg;

								REWARD_ITEM_LIST::const_iterator item_itr = itr->kContItem.begin();
								for ( ; item_itr != itr->kContItem.end() ; ++item_itr )
								{
									::swprintf_s( wszTemp, 30, L"ICON_ITEM_%d", index++ );
									pkTemp = pkWnd->GetControl( std::wstring(wszTemp) );
									if ( pkTemp )
									{
										pkTemp->SetCustomData( &(*item_itr), sizeof(REWARD_ITEM_LIST::value_type) );

										std::wstring kItemName;
										if ( true == MakeItemName( item_itr->iItemNo, kEnchantInfo, kItemName ) )
										{
											std::wstring wstrTemp;
											WstringFormat( wstrTemp, MAX_PATH, TTW(700042).c_str(), kItemName.c_str() );
											wstrItemMsg += wstrTemp;
											wstrItemMsg += L"\n";
										}
									}
									else
									{
										break;
									}
								}

								while ( 1 )
								{
									::swprintf_s( wszTemp, 30, L"ICON_ITEM_%d", index++ );
									pkTemp = pkWnd->GetControl( std::wstring(wszTemp) );
									if ( pkTemp )
									{
										pkTemp->ClearCustomData();
									}
									else
									{
										break;
									}
								}

								pkWnd->Text( wstrItemMsg );
							}
						}
					}
				}
			}
		}
		if(g_pkWorld)
		{
			PgCameraModeFollow *pkMode = dynamic_cast<PgCameraModeFollow*>(g_pkWorld->m_kCameraMan.GetCameraMode());
			if ( pkMode )
			{
				pkMode->SetCameraFixed( true );
			}
			
			CONT_PVP_TRIGGER_PARTICLE::iterator iter;
			for( iter = m_ContTriggerParticle.begin(); iter != m_ContTriggerParticle.end(); ++iter )
			{
				g_pkWorld->DetachParticle( iter->second );
			}
			m_ContTriggerParticle.clear();
		}
		m_ContLink.clear();
		m_ContTriggerEntity.clear();
		m_LevelUpPointInfo.clear();
	}
}

bool PgPvPGame::ClickSlot( XUI::CXUI_Wnd *pkSlot, bool bRButton )
{
	if ( pkSlot )
	{
		if ( bRButton )
		{
			BM::GUID const &kOwnerGuid = pkSlot->OwnerGuid();
			if ( BM::GUID::IsNotNull(kOwnerGuid) )
			{
				if ( m_bMyRoom && !g_kPilotMan.IsMyPlayer(kOwnerGuid) )
				{
					XUI::CXUI_Wnd* pkParentWnd = pkSlot->Parent();
					if( pkParentWnd )
					{
						XUI::CXUI_Wnd* pkCloseWnd = pkParentWnd->GetControl( _T("IMG_CLOSE") );
						if( pkCloseWnd && !pkCloseWnd->Visible() )
						{
							// Ready
							XUI::CXUI_Wnd *pkReady = pkParentWnd->GetControl(_T("IMG_READY"));
							if ( pkReady )
							{
								if ( true == pkReady->Visible() )
								{
									return false;
								}
							}

							XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( _T("FRM_PVP_INROOM_INFO") );
							if( pkWnd )
							{
								pkWnd->OwnerGuid(kOwnerGuid);
								pkWnd->Location(XUIMgr.MousePos());	
								return true;
							}		
						}
					}
				}
			}
		}
		else
		{
			if ( m_bMyRoom )
			{
				XUI::CXUI_Wnd* pkParentWnd = pkSlot->Parent();
				if( pkParentWnd )
				{
					XUI::CXUI_Wnd* pkCloseWnd = pkParentWnd->GetControl( _T("IMG_CLOSE") );
					if( pkCloseWnd )
					{
						SPvPTeamSlot kTeamSlot;
						pkSlot->GetCustomData( &kTeamSlot, sizeof(kTeamSlot) );

						XUI::CXUI_Wnd *pkEnemySlot = NULL;
						if ( true == IsBalance() )
						{
							SPvPTeamSlot kEnemyTeamSlot = kTeamSlot;
							kEnemyTeamSlot.SetEnemySlot();
							pkEnemySlot = GetSlot( XUIMgr.Get( ms_FRM_PVP_ROOM[m_kRoomBaseInfo.m_kMode] ), kEnemyTeamSlot.GetSlot(), kEnemyTeamSlot.GetTeam() );
						}

						if ( kTeamSlot.IsCorrect( IsExercise() ) ) 
						{
							if ( pkCloseWnd->Visible() )
							{
								if ( !pkEnemySlot || BM::GUID::IsNull(pkEnemySlot->OwnerGuid()) )
								{
									BM::Stream kPacket( PT_C_T_REQ_SLOTSTATUS_CHANGE, true );
									kTeamSlot.WriteToPacket(kPacket);
									NETWORK_SEND( kPacket )
								}
							}
							else
							{ 
								if ( BM::GUID::IsNull( pkSlot->OwnerGuid() ) )
								{
									if ( !pkEnemySlot || BM::GUID::IsNull(pkEnemySlot->OwnerGuid()) )
									{
										if( false == IsExercise() )
										{
											if( 1 == pkEnemySlot->BuildIndex() )
											{
												::Notice_Show( TTW(200157), EL_PvPMsg );
												return false;
											}
										}

										BM::Stream kPacket( PT_C_T_REQ_SLOTSTATUS_CHANGE, false );
										kTeamSlot.WriteToPacket(kPacket);
										NETWORK_SEND( kPacket )
									}	
								}
							}

							g_kUISound.PlaySoundByID( L"button_UI_PVP" );
						}
					}
				}
			}
		}
		
	}
	return false;
}

bool PgPvPGame::ClickEntry( int const iEntryNo )
{
	int iMyTeam = TEAM_NONE;
	BM::GUID kPlayerCharGuid;
	if ( g_kPilotMan.GetPlayerPilotGuid(kPlayerCharGuid) )
	{
		CONT_PVP_GAME_USER::mapped_type const *pkElement = Get( kPlayerCharGuid );
		if ( pkElement )
		{
			iMyTeam = pkElement->kTeamSlot.GetTeam();
		}
	}

	SPvPTeamEntry kTeamEntry(iMyTeam, iEntryNo);

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		BM::Stream kPacket( PT_C_T_REQ_ENTRY_CHANGE );
		kTeamEntry.WriteToPacket(kPacket);
		NETWORK_SEND( kPacket )
	}

	return false;
}

bool PgPvPGame::KickUser( BM::GUID const &kCharGuid, bool bKick )
{
	CONT_PVP_GAME_USER::mapped_type *pkElement = Get(kCharGuid);
	if ( pkElement )
	{
		if ( bKick )
		{
			switch( g_kLocal.ServiceRegion() )
			{
			case LOCAL_MGR::NC_SINGAPORE:
			case LOCAL_MGR::NC_THAILAND:
			case LOCAL_MGR::NC_INDONESIA:
			case LOCAL_MGR::NC_VIETNAM:
			case LOCAL_MGR::NC_RUSSIA:
			case LOCAL_MGR::NC_PHILIPPINES:
				{
					// 싱가폴은 무조건 Kick 가능
				}break;
			default:
				{
					if ( PS_READY & pkElement->byStatus )
					{
						// 준비완료한 참가자는 추방 하실 수 없습니다.
						::Notice_Show( TTW(400404), EL_PvPMsg );
					}
				}break;
			}

			NETWORK_SEND( BM::Stream( PT_C_T_REQ_KICK_ROOM_USER, kCharGuid ) )
			return true;
		}
		else
		{
			XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate(_T("BOX_PVP_EXILE"), true );
			if ( pkWnd )
			{
				XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_MESSAGE"));
				if ( pkTemp )
				{
					std::wstring wstrMsg;
					WstringFormat( wstrMsg, MAX_PATH, TTW(400342).c_str(), pkElement->wstrName.c_str() );
					pkTemp->Text(wstrMsg);
				}

				pkTemp = pkWnd->GetControl(_T("BTN_OK"));
				if ( pkTemp )
				{
					pkTemp->OwnerGuid(kCharGuid);
					return true;
				}
			}
		}
	}
	return false;
}

bool PgPvPGame::EntrustMaster( BM::GUID const &kCharGuid )
{
	CONT_PVP_GAME_USER::mapped_type *pkElement = Get(kCharGuid);
	if ( pkElement )
	{
		XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate(_T("BOX_PVP_ASKOUT"), true );
		if ( pkWnd )
		{
			XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_MESSAGE"));
			if ( pkTemp )
			{
				std::wstring wstrMsg;
				WstringFormat( wstrMsg, MAX_PATH, TTW(400295).c_str(), pkElement->wstrName.c_str() );
				pkTemp->Text(wstrMsg);
			}

			pkTemp = pkWnd->GetControl(_T("BTN_OK"));
			if ( pkTemp )
			{
				BM::Stream kPacket( PT_C_T_REQ_ENTRUST_MASTER, kCharGuid );
				pkTemp->SetCustomData( &kPacket.Data().at(0), kPacket.Data().size() );
			}

			return true;
		}
	}
	return false;
}

bool PgPvPGame::CallResponse()
{
	if ( STATUS_PLAYING == m_eStatus )
	{
		if ( PVP_TYPE_ANNIHILATION == m_kRoomBaseInfo.m_kType )
		{
			CONT_PVP_GAME_USER::const_iterator user_itr;
			for ( user_itr=m_kContPvPUser.begin(); user_itr!=m_kContPvPUser.end(); ++user_itr )
			{
				// 팀이 같은지 검사
//				if ( user_itr->second.kTeamSlot.GetTeam() )
//				{
					g_kOBMode.AddTarget( user_itr->first );
//				}
			}
		}
		else
		{
			XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate(ms_FRM_PVP_RESPONSE);
			if ( pkWnd )
			{
				lwUIWnd kWnd(pkWnd->GetControl(_T("BAR_TIMER")));
				if( IsLoveMode() )
				{
					kWnd.SetStartTime(LOVE_MODE_REVIVE_TIME);
				}
				else
				{
					kWnd.SetStartTime(5000);
				}
				return true;
			}
		}		
	}
	return false;
}

XUI::CXUI_List2 *PgPvPGame::GetLobbyList()const
{
	XUI::CXUI_Wnd *pkCommunityTopWnd = NULL;
	switch ( m_eStatus )
	{
	case STATUS_LOBBY:
		{
			pkCommunityTopWnd = XUIMgr.Get( PgContentsBase::ms_FRM_PVP_LOBBY );
		}break;
	case STATUS_ROOM:
		{
			pkCommunityTopWnd = XUIMgr.Get( PgContentsBase::ms_FRM_PVP_ROOM_ATTR );
		}break;
	}

	if ( pkCommunityTopWnd )
	{
		pkCommunityTopWnd = pkCommunityTopWnd->GetControl(_T("FRM_COMMUNITY"));
		if ( pkCommunityTopWnd )
		{
			pkCommunityTopWnd = pkCommunityTopWnd->GetControl(_T("FRM_0"));
			if ( pkCommunityTopWnd )
			{
				return dynamic_cast<XUI::CXUI_List2*>(pkCommunityTopWnd->GetControl(_T("LST_LIST0")));
			}
		}
	}

	return NULL;
}

XUI::CXUI_List2 *PgPvPGame::GetLobbyTabList(int const iSetMode)const
{
	XUI::CXUI_Wnd *pkCommunityTopWnd = NULL;
	switch ( m_eStatus )
	{
	case STATUS_LOBBY:
		{
			pkCommunityTopWnd = XUIMgr.Get( PgContentsBase::ms_FRM_PVP_LOBBY );
		}break;
	case STATUS_ROOM:
		{
			pkCommunityTopWnd = XUIMgr.Get( PgContentsBase::ms_FRM_PVP_ROOM_ATTR );
		}break;
	}

	if ( pkCommunityTopWnd )
	{
		pkCommunityTopWnd = pkCommunityTopWnd->GetControl(_T("FRM_COMMUNITY"));
		if ( pkCommunityTopWnd )
		{
			BM::vstring vstr;
			vstr << _T("FRM_") << iSetMode;

			pkCommunityTopWnd = pkCommunityTopWnd->GetControl( vstr );

			if ( pkCommunityTopWnd )
			{
				BM::vstring vListstr;
				vListstr << _T("LST_LIST") << iSetMode;

				return dynamic_cast<XUI::CXUI_List2*>(pkCommunityTopWnd->GetControl( vListstr ));
			}
		}
	}

	return NULL;
}

bool PgPvPGame::SortLobbyList( bool const bName )
{
	for(int i=0;i<TAB_ALLNUM;++i)
	{
		XUI::CXUI_List2 *pList = GetLobbyTabList(i);
		if ( pList )
		{
			if ( bName )
			{
				pList->Sort( PgLobbyUserUI::SSortName() );
			}
			else
			{
				pList->Sort( PgLobbyUserUI::SSortPos() );
			}
			return true;
		}
	}
	return false;
}

void PgPvPGame::MyInfoRefresh( SPvPReport const * pkPvPReport )
{
	XUI::CXUI_Wnd *pkCommunityTopWnd = XUIMgr.Get( PgContentsBase::ms_FRM_PVP_LOBBY );

	if( !pkCommunityTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkRank = pkCommunityTopWnd->GetControl( L"FRM_RANKING" );
	if ( pkRank )
	{
		int index = 0;

		bool const bViewRank = ( 0 < m_iMyRank );
		BM::vstring const vstrRankImgBase(L"IMG_RANKING_NUM_");
		size_t iRank = m_iMyRank;
		BM::vstring vstr = vstrRankImgBase + index;
		XUI::CXUI_Wnd *pkTemp = pkRank->GetControl( static_cast<std::wstring>(vstr) );
		while ( pkTemp )
		{
			pkTemp->UVUpdate( iRank % 10 + 1 );
			pkTemp->Visible( bViewRank );
			
			iRank /= 10;

			++index;
			vstr = vstrRankImgBase + index;
			pkTemp = pkRank->GetControl( static_cast<std::wstring>(vstr) );
		}

		pkTemp = pkRank->GetControl( L"FRM_RANKING_NONE" );
		if ( pkTemp )
		{
			pkTemp->Visible( !bViewRank );
		}

		if ( pkPvPReport )
		{
			index = 0;
			BM::vstring const vstrPointImgBase(L"IMG_POINT_NUM_");
			size_t iPoint = static_cast<size_t>(pkPvPReport->m_iPoint); 
			vstr = vstrPointImgBase + index;
			pkTemp = pkRank->GetControl( static_cast<std::wstring>(vstr) );
			while ( pkTemp )
			{
				pkTemp->UVUpdate( iPoint % 10 + 1 );

				iPoint /= 10;

				++index;
				vstr = vstrPointImgBase + index;
				pkTemp = pkRank->GetControl( static_cast<std::wstring>(vstr) );
			}
		}
	}

	if ( pkPvPReport )
	{
		XUI::CXUI_Wnd *pkInfo = pkCommunityTopWnd->GetControl(_T("FRM_COMMUNITY"));
		if( pkInfo )
		{
			BM::vstring vstr;
		
			XUI::CXUI_Wnd *pkControl = pkInfo->GetControl( _T("SFRM_BATTLEFIELD"));
			if ( pkControl )
			{
				bool const bIsExercise = IsExercise();

				vstr = pkPvPReport->GetRecord(GAME_WIN, bIsExercise);
				vstr += TTW(200154);
				vstr += L" ";
				vstr += pkPvPReport->GetRecord(GAME_LOSE, bIsExercise);
				vstr += TTW(200155);
				pkControl->Text( vstr );
			}

			pkControl = pkInfo->GetControl( _T("SFRM_CP") );
			if ( pkControl )
			{
				PgPilot *pkMyPilot = g_kPilotMan.GetPlayerPilot();
				if( pkMyPilot )
				{
					// CP는 * 10 해서 보여준다.
					vstr = (pkMyPilot->GetAbil(AT_CP) * CP_VALUE);				
				}
				else
				{
					vstr = 0;
				}

				pkControl->Text( vstr );
			}
		}
	}
}

void PgPvPGame::UpdateLobbyTabList(int const iSetMode, BM::GUID const& kOwnerGuid, SPvPLobbyUser_Base &kElement)
{
	XUI::CXUI_List2 *pList = GetLobbyTabList(iSetMode);
	if ( !pList )
	{
		return;
	}

	switch( iSetMode )
	{
	case 0:
		{
			return;
		}break;
	case 1:
		{
			SFriendItem kFriendItem;
			if(!g_kFriendMgr.Friend_Find_ByGuid(kOwnerGuid, kFriendItem))
			{
				// 친구가 아니네
				return;
			}
		}break;
	case 2:
		{
			PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkMyPlayer )
			{
				return;
			}
			if( BM::GUID::IsNull( pkMyPlayer->GuildGuid() ) )
			{
				return;
			}
			if( pkMyPlayer->GuildGuid() != kElement.kGuildGuid )
			{
				return;
			}
		}break;
	default:
		{
			return;
		}break;
	}

	PgLobbyUserUI_CreatePolicy<> kCreatePolicy( kElement );
	pList->AddItem(kOwnerGuid, kCreatePolicy);

	/*pFriendList->ClearList();

	XUI::CXUI_List2::CONT_LIST::const_iterator iter = pFriendList->Begin();
	for( ; iter!=pFriendList->End(); ++iter )
	{	
		// 친구
		BM::GUID const kCharGuid = (*iter)->OwnerGuid();
		SFriendItem kFriendItem;
		if(!g_kFriendMgr.Friend_Find_ByGuid(kCharGuid, kFriendItem))
		{
			// 친구가 아니네
			continue;
		}

		SPvPLobbyUser_Base kElement;
		PgLobbyUserUI_CreatePolicy<> kCreatePolicy( kElement );
		pFriendList->AddItem(kCharGuid, kCreatePolicy);
	}*/
}

bool PgPvPGame::UpdateLobbyList( BM::Stream &kPacket )
{
	XUI::CXUI_List2 *pList = GetLobbyList();
	if ( !pList )
	{
		return false;
	}

	size_t iSize = 0;
	BYTE byType = 0;
	kPacket.Pop( iSize );

	_PgOutputDebugString("[UpdateLobbyList] Size : %u\n", iSize);
	while ( iSize-- )
	{
		kPacket.Pop( byType );
		switch ( byType )
		{
		case PVPUTIL::E_EVENT_ADD:
		case PVPUTIL::E_EVENT_JOIN_ROOM:
		case PVPUTIL::E_EVENT_EXIT_ROOM:
			{
				SPvPLobbyUser_Base kElement;
				kElement.ReadFromPacket( kPacket );

				if ( !g_kPilotMan.IsMyPlayer( kElement.kCharacterGuid ) )
				{
					PgLobbyUserUI_CreatePolicy<> kCreatePolicy( kElement );
					pList->AddItem(kElement.kCharacterGuid, kCreatePolicy);

					UpdateLobbyTabList(1, kElement.kCharacterGuid, kElement);
					UpdateLobbyTabList(2, kElement.kCharacterGuid, kElement);
				}
				else
				{
					MyInfoRefresh( &(kElement.kPvPReport) );
				}
			}break;
		case PVPUTIL::E_EVENT_REMOVE:
			{
				BM::GUID kCharGuid;
				kPacket.Pop( kCharGuid );
				//pList->DeleteItem( kCharGuid );

				for(int i=0;i<TAB_ALLNUM;++i)
				{
					XUI::CXUI_List2 *pTabList = GetLobbyTabList(i);
					if ( pTabList )
					{
						pTabList->DeleteItem( kCharGuid );
					}
				}
			}break;
// 		case PVPUTIL::E_EVENT_JOIN_ROOM:
// 			{
// 				BM::GUID kCharGuid;
// 				int iRoomIndex = 0;
// 				kPacket.Pop( kCharGuid );
// 				kPacket.Pop( iRoomIndex );
// 
// 				PgLobbyUserUI* pUI = dynamic_cast<PgLobbyUserUI*>(pList->GetItem( kCharGuid ));
// 				if ( pUI )
// 				{
// 					pUI->m_kUserInfo.iRoomIndex = iRoomIndex;
// 					pUI->Refresh();
// 				}
// 			}break;
// 		case PVPUTIL::E_EVENT_EXIT_ROOM:
// 			{
// 				BM::GUID kCharGuid;
// 				int iRoomIndex = 0;
// 				SPvPReport kReport;
// 				kPacket.Pop( kCharGuid );
// 				kPacket.Pop( iRoomIndex );
// 				kPacket.Pop( kReport );
// 
// 				PgLobbyUserUI* pUI = dynamic_cast<PgLobbyUserUI*>(pList->GetItem( kCharGuid ));
// 				if ( pUI )
// 				{
// 					pUI->m_kUserInfo.iRoomIndex = iRoomIndex;
// 					pUI->m_kUserInfo.kPvPReport = kReport;
// 					pUI->Refresh();
// 				}
// 			}break;
		}
	}

	return true;
}

bool PgPvPGame::RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP )
{
	if ( pkUnit )
	{
		CONT_PVP_GAME_USER::mapped_type *pkElement = Get( pkUnit->GetID() );
		if ( pkElement )
		{
			XUI::CXUI_Wnd* pkSlotUI = GetSlotWnd( *pkElement );
			if ( pkSlotUI )
			{
				lua_tinker::call< void, lwUIWnd, int, int >("OnSet_PvPSlotHP", lwUIWnd(pkSlotUI), pkUnit->GetAbil( AT_C_MAX_HP ), iNewHP );
			}
		}
	}
	return false;
}

bool PgPvPGame::RecvMP( BM::GUID const &kCharGuid, int const iMaxMP, int const iNowMP )
{
	CONT_PVP_GAME_USER::mapped_type *pkElement = Get( kCharGuid );
	if ( pkElement )
	{
		XUI::CXUI_Wnd* pkSlotUI = GetSlotWnd( *pkElement );
		if ( pkSlotUI )
		{
			XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pkSlotUI->GetControl( _T("BAR_MP") ));
			if ( pkBar )
			{
				if( pkBar->Now() > iNowMP )
				{
					lua_tinker::call<void, lwUIWnd, DWORD, DWORD>("OnSetPvpDamageTwinkle", lwUIWnd(pkBar), 50, 1000);
				}
				pkBar->Max( iMaxMP );
				pkBar->Now( iNowMP );
				return true;
			}
		}
	}

	return false;
}

bool PgPvPGame::IsSlowMotionDie( PgActor *pkActor )
{
	if( pkActor ){ return false; }

	if ( PVP_TYPE_ANNIHILATION == m_kRoomBaseInfo.m_kType )
	{
		CONT_PVP_GAME_USER::mapped_type *pkUserInfo = Get(pkActor->GetGuid());
		if ( pkUserInfo )
		{
			CONT_PVP_GAME_USER::const_iterator user_itr = m_kContPvPUser.begin();
			for ( ; user_itr!=m_kContPvPUser.end(); ++user_itr )
			{
				if ( user_itr->first != pkActor->GetGuid() )
				{
					if ( pkUserInfo->kTeamSlot.GetTeam() == user_itr->second.kTeamSlot.GetTeam() )
					{
						if ( 0 == (PS_DEAD & user_itr->second.byStatus) )
						{
							return false;
						}
					}
				}

			}
			return true;
		}
	}
	return false;
}

bool PgPvPGame::UpdateRankingUI( XUI::CXUI_Wnd * pkWnd, PgPvPRankingMgr const &kPvPRankingMgr )
{
	if ( pkWnd )
	{
		XUI::CXUI_Wnd *pkControl = pkWnd->GetControl( L"SFRM_MIDDLE" );
		if ( pkControl )
		{
			XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(pkControl->GetControl(L"LST_LIST") );
			if ( pkList )
			{
				pkList->ClearList();

				for ( size_t i=1; i<=PgPvPRanking::ms_iMaxRank ; ++i )
				{
					SPvPRanking kRanking;
					if ( true == kPvPRankingMgr.GetRanking( i, kRanking ) )
					{
						if ( BM::GUID::IsNotNull(kRanking.kCharacterGuid) )
						{
							PgPvPRankingUI_CreatePolicy<> kCreatePolicy( kRanking, i );
							pkList->AddItem( kRanking.kCharacterGuid, kCreatePolicy );
						}
					}
				}

				return true;
			}
		}
	}
	return false;
}

void PgPvPGame::Recv_PT_T_C_ANS_ENTRY_CHANGE( BM::Stream &kPacket )
{
	if( PVP_MODE_TEAM != m_kRoomBaseInfo.m_kMode )
	{
		return;
	}

	int iTargetTeam = TEAM_NONE, iSlotIndex = 0;
	kPacket.Pop(iTargetTeam);
	kPacket.Pop(iSlotIndex);
	SPvPTeamEntry kPrevEntry, kNewEntry;
	kPrevEntry.ReadFromPacket(kPacket);
	kNewEntry.ReadFromPacket(kPacket);

	int iMyTeam = TEAM_NONE;
	BM::GUID kPlayerCharGuid;
	if ( g_kPilotMan.GetPlayerPilotGuid(kPlayerCharGuid) )
	{
		CONT_PVP_GAME_USER::mapped_type const *pkElement = Get( kPlayerCharGuid );
		if ( pkElement )
		{
			iMyTeam = pkElement->kTeamSlot.GetTeam();
		}
	}

	if( iMyTeam != iTargetTeam )
	{
		return;
	}

	XUI::CXUI_Wnd *pkWndRoom = XUIMgr.Get(ms_FRM_PVP_ROOM[PVP_MODE_TEAM]);
	if ( pkWndRoom )
	{
		BM::vstring vstrSlot( _T("FRM_PVP_SLOT_") );
		vstrSlot += kPrevEntry.GetTeam();
		XUI::CXUI_Wnd *pkWndSlot = pkWndRoom->GetControl(vstrSlot.operator const std::wstring &());
		if( pkWndSlot )
		{
			vstrSlot =L"SLOT_PVP_";
			vstrSlot += iSlotIndex;
			XUI::CXUI_Wnd *pkWndTargetSlot = pkWndSlot->GetControl(vstrSlot.operator const std::wstring &());
			if( pkWndTargetSlot )
			{
				XUI::CXUI_Wnd *pkWndSlotEntry = pkWndTargetSlot->GetControl(L"IMG_ENTRY");
				if( pkWndSlotEntry )
				{
					BM::vstring vstrEntry;
					vstrEntry = kNewEntry.GetEntry();
					pkWndSlotEntry->Visible(true);
					pkWndSlotEntry->Text(vstrEntry);
				}
			}
		}

		XUI::CXUI_Wnd *pkWndEntryBG = pkWndRoom->GetControl(L"FRM_ENTRY_BG");
		if( pkWndEntryBG )
		{
			switch( m_kRoomBaseInfo.m_kType )
			{
			case PVP_TYPE_WINNERS:
			case PVP_TYPE_WINNERS_TEAM:
			case PVP_TYPE_ACE:
				{
					pkWndEntryBG->Visible(true);

					BM::vstring vstrEntry(L"BTN_ENTRY_");
					vstrEntry += kPrevEntry.GetEntry();
					XUI::CXUI_Wnd *pkBtnEntry = pkWndEntryBG->GetControl(vstrEntry.operator const std::wstring &());
					if( pkBtnEntry )
					{
						pkBtnEntry->Enable(true);
					}

					vstrEntry = L"BTN_ENTRY_";
					vstrEntry += kNewEntry.GetEntry();
					pkBtnEntry = pkWndEntryBG->GetControl(vstrEntry.operator const std::wstring &());
					if( pkBtnEntry )
					{
						pkBtnEntry->Enable(false);
					}
				}break;
			default:
				{
					pkWndEntryBG->Visible(false);
				}break;
			}
		}
	}
}

void PgPvPGame::ClearEntryUI()
{
	XUI::CXUI_Wnd *pkWndRoom = XUIMgr.Get(ms_FRM_PVP_ROOM[PVP_MODE_TEAM]);
	if ( pkWndRoom )
	{
		for(int iTeam=TEAM_RED; iTeam<TEAM_MAX; ++iTeam)
		{
			BM::vstring vstrSlot( _T("FRM_PVP_SLOT_") );
			vstrSlot += iTeam;
			XUI::CXUI_Wnd *pkWndSlot = pkWndRoom->GetControl(vstrSlot.operator const std::wstring &());
			if( pkWndSlot )
			{
				for(int iSlotIndex=0; iSlotIndex<5;  ++iSlotIndex)
				{
					vstrSlot =L"SLOT_PVP_";
					vstrSlot += iSlotIndex;
					XUI::CXUI_Wnd *pkWndTargetSlot = pkWndSlot->GetControl(vstrSlot.operator const std::wstring &());
					if( pkWndTargetSlot )
					{
						XUI::CXUI_Wnd *pkWndSlotEntry = pkWndTargetSlot->GetControl(L"IMG_ENTRY");
						if( pkWndSlotEntry )
						{
							pkWndSlotEntry->Visible(false);
						}
					}
				}
			}
		}
		XUI::CXUI_Wnd *pkWndEntryBG = pkWndRoom->GetControl(L"FRM_ENTRY_BG");
		if( pkWndEntryBG )
		{
			switch( m_kRoomBaseInfo.m_kType )
			{
			case PVP_TYPE_WINNERS:
			case PVP_TYPE_WINNERS_TEAM:
			case PVP_TYPE_ACE:
				{
					pkWndEntryBG->Visible(true);
					for(int iEntryNo=0; iEntryNo<5;  ++iEntryNo)
					{
						BM::vstring vstrEntry(L"BTN_ENTRY_");
						vstrEntry += iEntryNo;
						XUI::CXUI_Wnd *pkBtnEntry = pkWndEntryBG->GetControl(vstrEntry.operator const std::wstring &());
						if( pkBtnEntry )
						{
							pkBtnEntry->Enable(true);
						}
					}
				}break;
			default:
				{
					pkWndEntryBG->Visible(false);
				}break;
			}
		}
	}
}