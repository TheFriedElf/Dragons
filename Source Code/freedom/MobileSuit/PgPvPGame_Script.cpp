#include "stdafx.h"
#include "ServerLib.h"
#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "PgCommandMgr.h"
#include "PgPilotMan.h"
#include "PgPvPGame.h"
#include "PgClientParty.h"
#include "PgTrigger.h"
#include "PgMinimap.h"
#include "PgUIScene.h"

int const SKILL_EMERGENCY_ESCAPE = 99803700;

int PgPvPGame::GetRandomPvPGround( bool const bDuel, BYTE const ucType, bool const bMatchLevel, BYTE const ucNowUser )
{
	if ( bDuel )
	{

	}
	else
	{
		if( bMatchLevel )
		{//! 인원에 맞는 맵중에서 랜덤.
			CONT_DEF_PVP_GROUNDGROUP const *pkDefPvPGroup = NULL;
			g_kTblDataMgr.GetContDef(pkDefPvPGroup);

			if ( pkDefPvPGroup && !(pkDefPvPGroup->empty()) )
			{
				CONT_DEF_PVP_GROUNDGROUP::const_iterator pvp_itr = PVPUTIL::IsHavePvPType(pkDefPvPGroup, ucType);

				if( pkDefPvPGroup->end() != pvp_itr )
				{
					CONT_DEF_PVP_MAP const &kMap = pvp_itr->second;
					CONT_DEF_PVP_MAP::const_iterator map_itr = kMap.find(ucNowUser);
					if( kMap.end() != map_itr )
					{
						CONT_DEF_PVP_GROUND const & UserCountGround = map_itr->second;
						CONT_DEF_PVP_GROUND::const_iterator gnd_itr;
						if( S_OK == ::RandomElement(UserCountGround, gnd_itr) )
						{//랜덤 나와랑~
							return gnd_itr->iGroundNo;
						}
					}
					else
					{//현재 인원수에 해당하는 맵이 없으면 그냥 랜덤
						if( S_OK == ::RandomElement(kMap, map_itr) )
						{//랜덤 나와랑~
							CONT_DEF_PVP_GROUND const &kGround = map_itr->second;
							CONT_DEF_PVP_GROUND::const_iterator gnd_itr;
							if( S_OK == ::RandomElement(kGround, gnd_itr) )
							{//랜덤 나와랑~
								return gnd_itr->iGroundNo;
							}
						}
					}
				}
			}
		}
		else
		{//그냥 랜덤
			CONT_DEF_PVP_GROUNDGROUP const *pkDefPvPGroup = NULL;
			g_kTblDataMgr.GetContDef(pkDefPvPGroup);

			if ( pkDefPvPGroup && !(pkDefPvPGroup->empty()) )
			{
				CONT_DEF_PVP_GROUNDGROUP::const_iterator pvp_itr = PVPUTIL::IsHavePvPType(pkDefPvPGroup, ucType);

				if( pkDefPvPGroup->end() != pvp_itr )
				{
					CONT_DEF_PVP_MAP const &kMap = pvp_itr->second;
					CONT_DEF_PVP_MAP::const_iterator map_itr;
					if( S_OK == ::RandomElement(kMap, map_itr) )
					{//랜덤 나와랑~
						CONT_DEF_PVP_GROUND const &kGround = map_itr->second;
						CONT_DEF_PVP_GROUND::const_iterator gnd_itr;
						if( S_OK == ::RandomElement(kGround, gnd_itr) )
						{//랜덤 나와랑~
							return gnd_itr->iGroundNo;
						}
					}
				}
			}
		}
	}
	return 0;
}

bool PgPvPGame::GetPvPGroundType( int const iGndNo, EPVPTYPE &kType, bool bRandom )
{
	CONT_DEF_PVP_GROUNDMODE const *pkDefPvPMode = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPMode);

	if ( pkDefPvPMode )
	{
		CONT_DEF_PVP_GROUNDMODE::const_iterator mode_itr = pkDefPvPMode->find(iGndNo);
		if ( mode_itr != pkDefPvPMode->end() )
		{
			EPVPTYPE const _kType = (EPVPTYPE)mode_itr->second.iType;
			if ( !bRandom )
			{
				return kType & _kType;
			}
			else
			{
				std::vector<EPVPTYPE> kTemp;
				kType = PVP_TYPE_DM;	
				while ( kType )
				{
					if ( kType & _kType )
					{
						kTemp.push_back(kType);
					}
					kType <<= 1;
				}

				if ( kTemp.empty() )
				{
					return PVP_TYPE_NONE;
				}

				std::random_shuffle(kTemp.begin(),kTemp.end());
				kType = *(kTemp.begin());
			}
			return true;
		}
	}

	kType = PVP_TYPE_NONE;
	return false;
}

bool PgPvPGame::GetGroundName( int const iGndNo, std::wstring &wstrName )
{
	CONT_DEF_PVP_GROUNDMODE const *pkDefPvPMode = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPMode);

	if ( pkDefPvPMode )
	{
		CONT_DEF_PVP_GROUNDMODE::const_iterator mode_itr = pkDefPvPMode->find(iGndNo);
		if ( mode_itr != pkDefPvPMode->end() )
		{
			wchar_t const *pText = NULL;
			if ( GetDefString( mode_itr->second.iName, pText ) )
			{
				wstrName = pText;
				return true;
			}
		}
	}
	return false;
}

bool PgPvPGame::GetPreviewImgPath( int const iGndNo, std::wstring &wstrPath )
{
	CONT_DEF_PVP_GROUNDMODE const *pkDefPvPMode = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPMode);

	if ( pkDefPvPMode )
	{
		CONT_DEF_PVP_GROUNDMODE::const_iterator mode_itr = pkDefPvPMode->find(iGndNo);
		if ( mode_itr != pkDefPvPMode->end() )
		{
			wstrPath = mode_itr->second.wstrPreviewImg;
			return true;
		}
	}
	return false;
}

bool PgPvPGame::OnClick_CreateRoom( XUI::CXUI_Wnd *pkUIWnd )
{
	if ( pkUIWnd )
	{
		if ( STATUS_LOBBY == m_eStatus )
		{
			XUI::CXUI_Wnd *pkTemp = pkUIWnd->GetControl(_T("SFRM_INPUT"));
			if ( pkTemp )
			{
				// 방이름
				XUI::CXUI_Edit *pkTemp_Edit = dynamic_cast<XUI::CXUI_Edit*>(pkTemp->GetControl(_T("EDT_NAME")));
				if ( pkTemp_Edit )
				{
					m_kRoomBaseInfo.m_wstrName = pkTemp_Edit->EditText();
					if ( g_kClientFS.Filter( m_kRoomBaseInfo.m_wstrName, false, FST_BADWORD ) 
						|| !g_kUnicodeFilter.IsCorrect(UFFC_PVP_ROOM_NAME, m_kRoomBaseInfo.m_wstrName)
						)
					{
						lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(200000)), true);
						return false;
					}
				}

				// 패스워드
				pkTemp_Edit = dynamic_cast<XUI::CXUI_Edit*>(pkTemp->GetControl(_T("EDT_PW")));
				if ( pkTemp_Edit )
				{
					m_kRoomExtInfo.strPassWord = MB(pkTemp_Edit->EditText());
				}
				m_kRoomBaseInfo.m_bPwd = !m_kRoomExtInfo.strPassWord.empty();
				if ( m_kRoomBaseInfo.m_bPwd )
				{
					if ( m_kRoomExtInfo.strPassWord.size() != 4 )
					{
						lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(400406)), true);
						return false;
					}
				}
			}

			m_kRoomBaseInfo.m_kStatus = ROOM_STATUS_LOBBY;
			m_kRoomBaseInfo.m_iGndNo = lua_tinker::call<int,char const*>( "GetPvPAttribute", MB(L"GROUNDNO") );
			m_kRoomBaseInfo.m_kType = lua_tinker::call<EPVPTYPE,char const*>( "GetPvPAttribute", MB(L"TYPE") );
			m_kRoomBaseInfo.m_kMode = lua_tinker::call<EPVPMODE,char const*>( "GetPvPAttribute", MB(L"MODE") );
			m_kRoomBaseInfo.m_iGameTime = lua_tinker::call<int,char const*>( "GetPvPAttribute", MB(L"GAMETIME")  );
			m_kRoomBaseInfo.m_iGamePoint = lua_tinker::call<int,char const*>( "GetPvPAttribute", MB(L"GAMEPOINT")  );
			
			int LimitLevel = lua_tinker::call<short,char const*>( "GetPvPAttribute", MB(L"LIMITLEVEL") );
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				if( LimitLevel )
				{
					int const iLevel = pPlayer->GetAbil(AT_LEVEL);
					m_kRoomBaseInfo.m_sLevelLimit_Max = iLevel + LimitLevel;
					m_kRoomBaseInfo.m_sLevelLimit_Min = iLevel - LimitLevel;
				}
				else
				{
					m_kRoomBaseInfo.m_sLevelLimit_Max = 0;
					m_kRoomBaseInfo.m_sLevelLimit_Min = 0;
				}
			}

			if( false == IsExerciseType() )
			{
				m_kRoomBaseInfo.m_sLevelLimit_Min = GetRankingModeLevelMin();
				m_kRoomBaseInfo.m_sLevelLimit_Max = GetRankingModeLevelMax();
			}

			m_kRoomBaseInfo.m_ucMaxUser = lua_tinker::call<BYTE,char const*>( "GetPvPAttribute", MB(L"MAXUSER") );
			m_kRoomExtInfo.ucRound = lua_tinker::call<BYTE,char const*>( "GetPvPAttribute", MB(L"MAXROUND") );

			pkTemp = pkUIWnd->GetControl(_T("SFRM_OPTION"));
			if ( pkTemp )
			{
				XUI::CXUI_CheckButton *pkTemp_CheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_HANDYCAP")));
				if ( pkTemp_CheckBtn )
				{
					if ( true == pkTemp_CheckBtn->Check() )
					{
						m_kRoomExtInfo.kOption |= E_PVP_OPT_USEHANDYCAP;
					}
					else
					{
						m_kRoomExtInfo.kOption &= (~E_PVP_OPT_USEHANDYCAP);
					}
				}

				pkTemp_CheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_USE_ITEM")));
				if ( pkTemp_CheckBtn )
				{
					if ( true == pkTemp_CheckBtn->Check() )
					{
						m_kRoomExtInfo.kOption |= E_PVP_OPT_USEITEM;
					}
					else
					{
						m_kRoomExtInfo.kOption &= (~E_PVP_OPT_USEITEM);
					}
				}

				pkTemp_CheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_DISABLE_DASHJUMP")));
				if ( pkTemp_CheckBtn )
				{
					if ( true == pkTemp_CheckBtn->Check() )
					{
						m_kRoomExtInfo.kOption |= E_PVP_OPT_DISABLEDASHJUMP;
					}
					else
					{
						m_kRoomExtInfo.kOption &= (~E_PVP_OPT_DISABLEDASHJUMP);
					}
				}

				pkTemp_CheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp->GetControl(_T("CBTN_USE_BATTLELEVEL")));
				if ( pkTemp_CheckBtn )
				{
					if ( true == pkTemp_CheckBtn->Check() )
					{
						m_kRoomExtInfo.kOption |= E_PVP_OPT_USEBATTLELEVEL;
					}
					else
					{
						m_kRoomExtInfo.kOption &= (~E_PVP_OPT_USEBATTLELEVEL);
					}
				}
			}
			
			BM::Stream kPacket(PT_C_T_REQ_CREATE_ROOM);
			m_kRoomBaseInfo.WriteToPacket(kPacket);
			m_kRoomExtInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
			return true;
		}
	}
	return false;
}

bool PgPvPGame::OnClick_JoinRoom( CONT_PVPROOM_LIST::key_type const &iRoomID, std::string const &strPwd )
{
	CONT_PVPROOM_LIST::const_iterator room_itr = m_kContRoom.find(iRoomID);
	if ( room_itr != m_kContRoom.end() )
	{
		if ( room_itr->second.m_bPwd && strPwd.empty() )
		{
			// 패스워드를 입력하게 해야 한다.
			XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( ms_FRM_PVP_JOIN_PASSWORD, true );
			if ( pkWnd )
			{
				pkWnd = pkWnd->GetControl( _T("BTN_OK") );
				if ( pkWnd )
				{
					pkWnd->SetCustomData( &(iRoomID), sizeof(iRoomID) );
				}
			}
			return false;
		}

		BM::Stream kPacket( PT_C_T_REQ_JOIN_ROOM, room_itr->second.m_iRoomIndex );
		kPacket.Push(strPwd);
		kPacket.Push(true);// 클릭해서 입장
		NETWORK_SEND(kPacket)
		return true;
	}
	return false;
}

bool PgPvPGame::OnClick_RandomJoinRoom()
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkPlayer )
	{
		short const sMyLevel = static_cast<short>(pkPlayer->GetAbil( AT_LEVEL ));

		std::vector<CONT_PVPROOM_LIST::key_type> kTempRoomList;
		CONT_PVPROOM_LIST::const_iterator room_itr = m_kContRoom.begin();
		for ( ; room_itr!=m_kContRoom.end(); ++room_itr )
		{
			if (	(room_itr->second.m_kStatus == ROOM_STATUS_LOBBY)
				&&	!room_itr->second.m_bPwd
				&&	(room_itr->second.m_ucMaxUser > room_itr->second.m_ucNowUser ) )
			{
				if ( false == room_itr->second.IsHaveLimitLevel() 
					|| ( (room_itr->second.m_sLevelLimit_Min <= sMyLevel)
					&& (room_itr->second.m_sLevelLimit_Max >= sMyLevel) ) )
				{ // 레벨 제한이 없거나 레벨제한에 허용된다면 입장
					kTempRoomList.push_back(room_itr->first);
				}
			}
		}

		if ( kTempRoomList.empty() )
		{
			// 대기 방이 없습니다.
			//lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(200107)), true);

			if( false == IsExercise() )
			{
				// 랭킹모드에서 빠른 입장을 누른 경우 방을 만들어 준다.
				OnClick_EmptyCreateRoom();
				return true;
			}
		}
		else
		{
			std::random_shuffle( kTempRoomList.begin(), kTempRoomList.end() );
			
			CONT_PVPROOM_LIST::const_iterator room_itr = m_kContRoom.find(*kTempRoomList.begin());
			if( room_itr != m_kContRoom.end() )
			{
				int const iRoomIndex = room_itr->second.m_iRoomIndex;

				std::string strPwd;
				BM::Stream kPacket( PT_C_T_REQ_JOIN_ROOM, iRoomIndex );
				kPacket.Push(strPwd);
				kPacket.Push(false);// 빠른 입장
				NETWORK_SEND(kPacket)
				return true;
			}
		}
	}
	return false;
}

bool PgPvPGame::OnClick_AutoJoinRoom()
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( m_kContCharToTeam.empty() 
		|| m_kContTeam.empty() 
		|| !pkMyPlayer )
	{
		return false;
	}

	int iRoomIndex = 0;
	CONT_PVPLEAGUE_CHAR_TO_TEAM::const_iterator charid_itr = m_kContCharToTeam.find(pkMyPlayer->GetID());
	if( m_kContCharToTeam.end() != charid_itr )
	{
		CONT_DEF_PVPLEAGUE_TEAM::const_iterator team_itr = m_kContTeam.find(charid_itr->second);
		if( m_kContTeam.end() != team_itr )
		{
			if( m_kTournamentInfo.iLeagueLevel == team_itr->second.iLeagueLevel )
			{
				BM::Stream kPacket( PT_C_T_REQ_JOIN_LEAGUE_ROOM);
				kPacket.Push( team_itr->first );
				NETWORK_SEND( kPacket );
				return true;
			}
		}
	}

	return false;
}

bool PgPvPGame::OnClick_RandomInviteUser()
{
	using namespace XUI;
	CXUI_List2 *pkList = GetLobbyList();
	if ( pkList )
	{
		VEC_GUID kVecGuidList;
		kVecGuidList.reserve( pkList->GetSize() );

		PgLobbyUserUI* pItem = NULL;
		CXUI_List2::CONT_LIST::const_iterator itr = pkList->Begin();
		for ( ; itr!=pkList->End() ; ++itr )
		{
			pItem = dynamic_cast<PgLobbyUserUI*>(*itr);
			if ( pItem && !(pItem->IsClosed()) )
			{
				if ( PVPUTIL::IsLobbyUser(pItem->m_kUserInfo) )
				{
					kVecGuidList.push_back( pItem->OwnerGuid() );
				}
			}
		}

		if ( kVecGuidList.size() )
		{
			std::random_shuffle( kVecGuidList.begin(), kVecGuidList.end() );

			size_t iSize = (kVecGuidList.size() > PVP_INVITE_MAX_USER_COUNT) ? PVP_INVITE_MAX_USER_COUNT : kVecGuidList.size();
			BM::Stream kPacket( PT_C_T_REQ_INVITE_PVP, iSize );

			VEC_GUID::iterator guid_itr = kVecGuidList.begin();
			while ( iSize-- )
			{
				kPacket.Push( *guid_itr );
				++guid_itr;
			}
			NETWORK_SEND( kPacket )
			return true;
		}
	}
	return false;
}

void PgPvPGame::RefreshPageControl( XUI::CXUI_Wnd *pkUIWnd )
{
	for ( int iIndex=0; iIndex<5; ++iIndex )
	{
		BM::vstring vstrBtnName(L"BTN_NUM_");
		vstrBtnName += iIndex;
		XUI::CXUI_Wnd * pkBtn = pkUIWnd->GetControl(vstrBtnName.operator const std::wstring &());
		if( pkBtn )
		{
			vstrBtnName = m_kPage.iNowPage + iIndex +1;
			pkBtn->Text(vstrBtnName);
		}
	}
}

bool PgPvPGame::OnClick_ChangePage( size_t iPage )
{
	if ( m_kPage.iNowPage != iPage )
	{
		m_kPage.iNowPage = iPage;
		RefreshRoomList();
		return true;
	}
	return false;
}

bool PgPvPGame::OnClick_LeagueLobbyChangePage( XUI::CXUI_Wnd *pkUIWnd )
{
	XUI::CXUI_Wnd * pkWndTop = pkUIWnd->Parent();
	if( pkWndTop )
	{
		size_t iPage = 0;
		pkUIWnd->GetCustomData( &iPage, sizeof(size_t) );
		if ( m_kPage.iNowPage != m_iBeginPageNo+iPage )
		{
			BM::vstring vstrBtnName(L"BTN_NUM_");
			vstrBtnName += m_kPage.iNowPage - m_iBeginPageNo;
			XUI::CXUI_Wnd * pkOldBtn = pkWndTop->GetControl(vstrBtnName.operator const std::wstring &());
			if( pkOldBtn )
			{
				m_kPage.iNowPage = m_iBeginPageNo+iPage;
				pkOldBtn->Enable(true);
				pkUIWnd->Enable(false);
				RefreshPageControl(pkUIWnd);
				RefreshLeagueLobby();
			}
			return true;
		}
	}
	return false;
}

bool PgPvPGame::OnClick_LeagueLobbyNextPage( XUI::CXUI_Wnd *pkUIWnd )
{
	if( m_iEndPageNo >= m_iBeginPageNo+1 )
	{
		++m_iBeginPageNo;
		++m_kPage.iNowPage;
		RefreshPageControl(pkUIWnd);
		RefreshLeagueLobby();
		return true;
	}
	return false;
}

bool PgPvPGame::OnClick_LeagueLobbyPrevPage( XUI::CXUI_Wnd *pkUIWnd )
{
	if( 0 < m_iBeginPageNo )
	{
		--m_iBeginPageNo;
		--m_kPage.iNowPage;
		RefreshPageControl(pkUIWnd);
		RefreshLeagueLobby();
		return true;
	}
	return false;
}

bool PgPvPGame::OnClick_LeagueLobbyBeginPage( XUI::CXUI_Wnd *pkUIWnd )
{
	int iOrgPageNo = m_kPage.iNowPage - m_iBeginPageNo;
	BM::vstring vstrBtnName(L"BTN_NUM_");
	vstrBtnName += iOrgPageNo;
	XUI::CXUI_Wnd * pkOldBtn = pkUIWnd->GetControl(vstrBtnName.operator const std::wstring &());

	XUI::CXUI_Wnd * pkNewBtn = pkUIWnd->GetControl(L"BTN_NUM_0");
	if( pkOldBtn && pkNewBtn )
	{
		pkOldBtn->Enable(true);
		pkNewBtn->Enable(false);
		m_iBeginPageNo = 0;
		m_kPage.iNowPage = 0;
		RefreshPageControl(pkUIWnd);
		RefreshLeagueLobby();
		return true;
	}

	return false;
}

bool PgPvPGame::OnClick_LeagueLobbyEndPage( XUI::CXUI_Wnd *pkUIWnd )
{
	int iOrgPageNo = m_kPage.iNowPage - m_iBeginPageNo;
	BM::vstring vstrBtnName(L"BTN_NUM_");
	vstrBtnName += iOrgPageNo;
	XUI::CXUI_Wnd * pkOldBtn = pkUIWnd->GetControl(vstrBtnName.operator const std::wstring &());

	vstrBtnName = L"BTN_NUM_4";
	XUI::CXUI_Wnd * pkNewBtn = pkUIWnd->GetControl(vstrBtnName.operator const std::wstring &());

	if( pkOldBtn && pkNewBtn )
	{
		pkOldBtn->Enable(true);
		pkNewBtn->Enable(false);
		m_iBeginPageNo = m_iEndPageNo-4;
		m_kPage.iNowPage = m_iEndPageNo;
		RefreshPageControl(pkUIWnd);
		RefreshLeagueLobby();
		return true;
	}

	return false;
}

void PgPvPGame::OnClick_ViewWaitRoom( XUI::CXUI_Wnd *pkUI )
{
	if ( pkUI )
	{
		m_kPage.bOnlyWait = !m_kPage.bOnlyWait;
		RefreshRoomList();

		if ( m_kPage.bOnlyWait )
		{
			pkUI->Text( TTW(400299) );
		}
		else
		{
			pkUI->Text( TTW(400302) );
		}
	}
}

void PgPvPGame::OnClick_ViewRanking(void)
{
	UpdateRankingUI( XUIMgr.Activate( ms_SFRM_PVP_RANKING, true ), m_kRankIngMgr );
	
	BM::Stream kPacket( PT_C_T_REQ_GET_PVPRANKING, m_kRankIngMgr.GetLastUpdateTime() );
	NETWORK_SEND( kPacket );
}

void PgPvPGame::OnClick_GameStart(BYTE const ucType)
{
	if( IsMaster() && IsRanking() )
	{//방장이고 랭킹 모드면, 레벨에 맞는 맵을 골르자 m_kContPvPUser
		m_kRoomBaseInfo.m_iGndNo = GetRandomPvPGround(false, ucType, true, m_kContPvPUser.size() );
	}
	BM::Stream kPacket(PT_C_T_REQ_GAME_READY);
	m_kRoomBaseInfo.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

bool PgPvPGame::SetEdit_Ground( int const iGroundNo, bool bNoSend )
{
	//if ( true == IsModifyRoomAttr() )
	{
		std::wstring wstrName;
		if ( GetGroundName( iGroundNo, wstrName ) )
		{
			m_kRoomBaseInfo.m_iGndNo = iGroundNo;

			if ( !bNoSend )
			{
				BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
				m_kRoomBaseInfo.WriteToPacket(kPacket);
				NETWORK_SEND(kPacket)
			}
			return true;
		}
	}
	return false;
}

bool PgPvPGame::IsRoomMaster()
{	
	return IsMaster();
}

bool PgPvPGame::IsExerciseType()
{
	return IsExercise();
}

bool PgPvPGame::IsLeagueType()
{
	return IsLeague();
}

int const PgPvPGame::GetRankingModeLevel()
{
	return GetRankingModeLevelMax();
}

bool PgPvPGame::SetEdit_Mode( EPVPMODE kMode, bool bNoSend )
{
	if ( IsModifyRoomAttr() && IsPersonalGame() )
	{
		m_kRoomBaseInfo.m_kMode = kMode;
		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
			m_kRoomBaseInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::SetEdit_Type( EPVPTYPE kType, bool bNoSend )
{
	if ( true == IsModifyRoomAttr() )
	{
		m_kRoomBaseInfo.m_kType = kType;

		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
			m_kRoomBaseInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::SetEdit_Time( int const iMinTime, bool bNoSend )
{
	if ( true == IsModifyRoomAttr() )
	{
		m_kRoomBaseInfo.m_iGameTime = iMinTime;
		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
			m_kRoomBaseInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::SetEdit_Point( int const iMinPoint, bool bNoSend )
{
	if ( true == IsModifyRoomAttr() )
	{
		m_kRoomBaseInfo.m_iGamePoint = iMinPoint;
		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
			m_kRoomBaseInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::SetEdit_MaxLevel( short const sLevel, bool bNoSend )
{
	if ( true == IsModifyRoomAttr() )
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( pPlayer )
		{
			if( sLevel )
			{
				int const MyLevel = pPlayer->GetAbil(AT_LEVEL);
				int const iRoomLevelMax = MyLevel + sLevel;
				int const iRoomLevelMin = MyLevel - sLevel;

				if( 0 < iRoomLevelMin )
				{
					m_kRoomBaseInfo.m_sLevelLimit_Min = iRoomLevelMin;
				}
				else
				{
					m_kRoomBaseInfo.m_sLevelLimit_Min = 1;
				}

				m_kRoomBaseInfo.m_sLevelLimit_Max = iRoomLevelMax;
			}
			else
			{
				m_kRoomBaseInfo.m_sLevelLimit_Min = 0;
				m_kRoomBaseInfo.m_sLevelLimit_Max = 0;
			}

			if ( !bNoSend )
			{
				BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, true);
				m_kRoomBaseInfo.WriteToPacket(kPacket);
				NETWORK_SEND(kPacket)
			}
			return true;
		}
	}
	return false;
}

bool PgPvPGame::SetEdit_RoundCount( BYTE const kRoundCount, bool bNoSend )
{
	if ( IsModifyRoomAttr() )
	{
		m_kRoomExtInfo.ucRound = kRoundCount;
		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, false);
			m_kRoomExtInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::SetEdit_Option( BYTE const kOption, bool const bNoSend )
{
	if ( true == IsModifyRoomAttr() )
	{
		m_kRoomExtInfo.kOption ^= kOption;
		if ( !bNoSend )
		{
			BM::Stream kPacket(PT_C_T_REQ_MODIFY_ROOM, false);
			m_kRoomExtInfo.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)
		}
		return true;
	}
	return false;
}

bool PgPvPGame::LoadPvPMaxPlayer( XUI::CXUI_List* pList, int const Type )
{
	if ( !pList )
	{
		return false;
	}

	CONT_DEF_PVP_GROUNDGROUP const* pkDefPvPMode = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPMode);

	if ( pkDefPvPMode )
	{
		pList->ClearList();
		POINT2 kTemp(0, 5);

		CONT_DEF_PVP_GROUNDGROUP::const_iterator type_itr = PVPUTIL::IsHavePvPType(pkDefPvPMode, Type);

		if( pkDefPvPMode->end() != type_itr )
		{ // 해당 타입을 가진 PVP 맵들의 정보를 찾았다.
			CONT_DEF_PVP_GROUNDGROUP::mapped_type const & PvPMap = type_itr->second;
			CONT_DEF_PVP_GROUNDGROUP::mapped_type::const_iterator map_itr;
			for( map_itr = PvPMap.begin(); map_itr != PvPMap.end(); ++map_itr )
			{
				int const UserCount = map_itr->first;
				BM::vstring vstr(UserCount);
				XUI::SListItem* pListItem = pList->AddItem((std::wstring)vstr);
				if( pListItem )
				{
					XUI::CXUI_Wnd* pkItemWnd = pListItem->m_pWnd;
					if( pkItemWnd )
					{
						pkItemWnd->Text((std::wstring)vstr);
						pkItemWnd->SetCustomData( &map_itr->first, sizeof(int) );
						kTemp.x = pkItemWnd->Size().x;
						kTemp.y += pkItemWnd->Size().y;
					}
				}
			}
			pList->Size(kTemp);

			XUI::CXUI_Wnd *pkParent = pList->Parent();
			if ( pkParent )
			{
				pkParent->Size(kTemp);
				XUI::CXUI_Wnd *pkOpBg = pkParent->GetControl(_T("FRM_Option_BG"));
				if ( pkOpBg )
				{
					POINT3I kPos = pkOpBg->Location();
					kPos.y = kTemp.y-4;
					pkOpBg->Location(kPos);
				}
			}
			return true;
		}
	}
	return false;
}

bool PgPvPGame::LoadPvPGroundList( XUI::CXUI_List* pList, int const Type, int const UserCount )
{
	if ( !pList )
	{
		return false;
	}

	CONT_DEF_PVP_GROUNDGROUP const* pkDefPvPMode = NULL;
	g_kTblDataMgr.GetContDef(pkDefPvPMode);

	if ( pkDefPvPMode )
	{
		pList->ClearList();
		POINT2 kTemp(0, 5);

		CONT_DEF_PVP_GROUNDGROUP::const_iterator type_itr = PVPUTIL::IsHavePvPType(pkDefPvPMode, Type);

		if( pkDefPvPMode->end() != type_itr )
		{ // 해당 타입을 가진 PVP 맵들의 정보를 찾았다.
			CONT_DEF_PVP_GROUNDGROUP::mapped_type const& kMap = type_itr->second;
			CONT_DEF_PVP_MAP::const_iterator map_itr = kMap.find(UserCount);
			if( kMap.end() != map_itr )
			{
				CONT_DEF_PVP_MAP::mapped_type const& kGround = map_itr->second;
				CONT_DEF_PVP_GROUND::const_iterator gnd_itr;
				for( gnd_itr = kGround.begin(); gnd_itr != kGround.end(); ++gnd_itr )
				{
					std::wstring const * pwstrText = NULL;
					if( GetDefString( gnd_itr->iName, pwstrText ) )
					{
						XUI::SListItem* pListItem = pList->AddItem(*pwstrText);
						if( pListItem )
						{
							XUI::CXUI_Wnd* pkItemWnd = pListItem->m_pWnd;
							if( pkItemWnd )
							{
								pkItemWnd->Text(*pwstrText);
								pkItemWnd->SetCustomData( &gnd_itr->iGroundNo, sizeof(int) );
								kTemp.x = pkItemWnd->Size().x;
								kTemp.y += pkItemWnd->Size().y;
							}
						}
					}
				}
				pList->Size(kTemp);

				XUI::CXUI_Wnd *pkParent = pList->Parent();
				if ( pkParent )
				{
					pkParent->Size(kTemp);
					XUI::CXUI_Wnd *pkOpBg = pkParent->GetControl(_T("FRM_Option_BG"));
					if ( pkOpBg )
					{
						POINT3I kPos = pkOpBg->Location();
						kPos.y = kTemp.y-4;
						pkOpBg->Location(kPos);
					}
				}
				return true;
			}
		}
	}
	return false;
}

int PgPvPGame::GetEntityPointByTriggerID( char const * pTriggerID ) const
{
	if( pTriggerID )
	{
		CONT_PVP_TRIGGER_ENTITY::const_iterator Trg_iter = m_ContTriggerEntity.find( std::string(pTriggerID) );
		if( m_ContTriggerEntity.end() != Trg_iter )
		{
			if( g_pkWorld )
			{
				UNIT_PTR_ARRAY AddUnitArray;
				g_kPilotMan.FindUnit(UT_ENTITY, AddUnitArray);
				UNIT_PTR_ARRAY::const_iterator Ent_iter;
				for( Ent_iter = AddUnitArray.begin(); Ent_iter != AddUnitArray.end(); ++Ent_iter )
				{
					CUnit const * pUnit = Ent_iter->pkUnit;
					if( NULL == pUnit )
					{
						continue;
					}

					if( Trg_iter->second == pUnit->GetID() )
					{
						int const & Point = pUnit->GetAbil(AT_OCCUPY_POINT);
						return Point;
					}
				}
			}
		}
	}
	return 0;
}

int PgPvPGame::GetEntityLevelByTriggerID( char const * pTriggerID ) const
{
	if( pTriggerID )
	{
		int Point = GetEntityPointByTriggerID(pTriggerID);
		if( Point < 0 )
		{
			Point *= -1;
		}
		return GetLevelByPoint( Point );
	}
	return 0;
}

int PgPvPGame::GetEntityLinkByTriggerID( char const * pTriggerID )
{
	if( pTriggerID )
	{
		int LinkCount = 0;
		CONT_STRONGHOLD_LINK::const_iterator iter;
		for( iter = m_ContLink.begin(); iter != m_ContLink.end(); ++iter )
		{
			if( iter->LTrigger == (std::string)pTriggerID
				|| iter->RTrigger == (std::string)pTriggerID )
			{
				if( g_pkWorld )
				{
					PgTrigger * pTrigger = g_pkWorld->GetTriggerByID( (std::string)pTriggerID );
					if( pTrigger )
					{
						if( pTrigger->EntityTeam() == iter->LinkTeam )
						{
							++LinkCount;
						}
					}
				}
			}
		}
		return LinkCount;
	}
	return 0;
}

CUnit const* PgPvPGame::GetEntityByTriggerID( char const * pTriggerID ) const
{
	if( pTriggerID )
	{
		CONT_PVP_TRIGGER_ENTITY::const_iterator Trg_iter = m_ContTriggerEntity.find( std::string(pTriggerID) );
		if( m_ContTriggerEntity.end() != Trg_iter )
		{
			if( g_pkWorld )
			{
				UNIT_PTR_ARRAY AddUnitArray;
				g_kPilotMan.FindUnit(UT_ENTITY, AddUnitArray);
				UNIT_PTR_ARRAY::const_iterator Ent_iter;
				for( Ent_iter = AddUnitArray.begin(); Ent_iter != AddUnitArray.end(); ++Ent_iter )
				{
					CUnit const * pUnit = Ent_iter->pkUnit;
					if( NULL == pUnit )
					{
						continue;
					}

					if( Trg_iter->second == pUnit->GetID() )
					{
						return pUnit;
					}
				}
			}
		}
	}
	return NULL;
}

int PgPvPGame::GetPointByLevel( int const Level ) const
{
	return m_LevelUpPointInfo.GetPointByLevel( Level );
}

int PgPvPGame::GetLevelByPoint( int const Point ) const
{
	return m_LevelUpPointInfo.GetLevelByPoint( Point );
}

int PgPvPGame::OnUpdateUIEmergencyEscape()
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"BTN_EMERGENCY_ESCAPE");
	if( NULL == pWnd )
	{
		return -1;
	}

	PgActor * pActor = g_kPilotMan.GetPlayerActor();
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pActor || NULL == pPlayer )
	{
		return -1;
	}

	if( false == pPlayer->BearEffect() )
	{// 곰 디버프에 걸려있지 않은 상태라면 볼일 없음
		return -1;
	}

	PgActor::stSkillCoolTimeInfo * pSkillCoolTimeInfo = pActor->GetSkillCoolTimeInfo();
	if( NULL == pSkillCoolTimeInfo )
	{
		return -1;
	}
	
	PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::iterator iter = pSkillCoolTimeInfo->m_CoolTimeInfoMap.find(SKILL_EMERGENCY_ESCAPE);
	if( iter != pSkillCoolTimeInfo->m_CoolTimeInfoMap.end() )
	{
		int iRemainCoolTime = iter->second.m_ulTotalCoolTime - (BM::GetTime32() - iter->second.m_ulCoolStartTime);
		if( iRemainCoolTime < 0 )
		{
			iRemainCoolTime = 0;
		}
		return iRemainCoolTime / 1000;
	}
	return -1;
}

void PgPvPGame::UpdateStrongholdLink()
{
	if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
	{//점령전일 때만
		CONT_STRONGHOLD_LINK::iterator Iter;
		for( Iter = m_ContLink.begin(); Iter != m_ContLink.end(); ++Iter )
		{// 트리거 이름으로 엔티티 어빌에 있는 점령지 포인트를 찾는다.
			int LPoint = GetEntityPointByTriggerID( Iter->LTrigger.c_str() );
			int RPoint = GetEntityPointByTriggerID( Iter->RTrigger.c_str() );

			// 포인트로 현재 이 링크가 어느 팀인지 파악한다.
			int FirstLvUpPoint = m_LevelUpPointInfo.LvUpPoint.begin()->second;
			if( LPoint >= FirstLvUpPoint && RPoint >= FirstLvUpPoint )
			{
				Iter->LinkTeam = TEAM_RED;
			}
			else if( LPoint <= -FirstLvUpPoint && RPoint <= -FirstLvUpPoint )
			{
				Iter->LinkTeam = TEAM_BLUE;
			}
			else
			{
				Iter->LinkTeam = TEAM_NONE;
			}
		}
		PgAlwaysMiniMap* pMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( pMinimap )
		{
			pMinimap->UpdateStrongholdLink();	// 링크 그려주기
		}
	}
}

void PgPvPGame::UpdateStrongholdTrigger()
{
	if( PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType )
	{//점령전일 때만
		CONT_STRONGHOLD_LINK::iterator Iter;
		for( Iter = m_ContLink.begin(); Iter != m_ContLink.end(); ++Iter )
		{// 트리거 이름으로 엔티티 어빌에 있는 점령지 포인트를 찾는다.
			int const LPoint = GetEntityPointByTriggerID( Iter->LTrigger.c_str() );
			int const RPoint = GetEntityPointByTriggerID( Iter->RTrigger.c_str() );

			PgTrigger * pLTrigger = g_pkWorld->GetTriggerByID( Iter->LTrigger.c_str() );
			PgTrigger * pRTrigger = g_pkWorld->GetTriggerByID( Iter->RTrigger.c_str() );
			if( pLTrigger && pRTrigger )
			{
				int TempPoint = LPoint;
				if( TempPoint < 0 )
				{
					TempPoint *= -1;
				}
				int EntityLevel = GetLevelByPoint( TempPoint );
				if( 0 == EntityLevel )
				{// TEAM_NONE
					pLTrigger->EntityTeam( TEAM_NONE );
				}
				else
				{
					if( LPoint > 0 )
					{// TEAM_RED
						pLTrigger->EntityTeam( TEAM_RED );
					}
					else
					{// TEAM_BLUE
						pLTrigger->EntityTeam( TEAM_BLUE );
					}
				}
				
				TempPoint = RPoint;
				if( TempPoint < 0 )
				{
					TempPoint *= -1;
				}
				EntityLevel = GetLevelByPoint( TempPoint );
				if( 0 == EntityLevel )
				{// TEAM_NONE
					pRTrigger->EntityTeam( TEAM_NONE );
				}
				else
				{
					if( RPoint > 0 )
					{// TEAM_RED
						pRTrigger->EntityTeam( TEAM_RED );
					}
					else
					{// TEAM_BLUE
						pRTrigger->EntityTeam( TEAM_BLUE );
					}
				}
			}
		}
	}
}