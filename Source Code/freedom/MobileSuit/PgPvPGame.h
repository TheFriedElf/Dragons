#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_PGPVPGAME_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_PGPVPGAME_H

#include "variant/PgPvPUtil.h"
#include "XUI/XUI_List2.h"
#include "Variant/PgPvPRanking.h"
#include "lwUI.h"
#include "PgContentsBase.h"
#include "lohengrin/dbtables.h"

extern int const MAX_LEAGUE_LEVEL;
extern int const MAX_LEAGUE_LEVEL_HALF;
extern int const MAX_LEAGUE_PAGE_ELEMENT;
extern int const MAX_TEAMELEMENT_COUNT;
extern int const MAX_TEAM_TITLE_LENGTH;

extern int const LOVE_MODE_REVIVE_TIME;

struct SPvPLevelUpPointInfo
{//점령지 레벨업 구간 정보
	typedef std::map<int, int> CONT_LEVELUP_POINT;

	CONT_LEVELUP_POINT LvUpPoint;

	SPvPLevelUpPointInfo()
	{
		LvUpPoint.clear();
	}

	int GetLevelByPoint(int const Point) const
	{
		CONT_LEVELUP_POINT::const_iterator iter;
		for( iter = LvUpPoint.begin(); iter != LvUpPoint.end(); ++iter )
		{
			if( Point < iter->second )
			{
				return iter->first - 1;
			}
			else if( Point == iter->second )
			{
				CONT_LEVELUP_POINT::const_iterator temp_iter = iter;
				++temp_iter;
				if( temp_iter == LvUpPoint.end() )
				{
					return iter->first - 1;
				}
				else
				{
					return iter->first;
				}
			}
		}
		return 0;
	}
	
	int GetPointByLevel(int const Level) const
	{
		CONT_LEVELUP_POINT::const_iterator iter = LvUpPoint.find(Level);
		if( iter != LvUpPoint.end() )
		{
			return iter->second;
		}
		return 0;
	}

	void clear()
	{
		LvUpPoint.clear();
	}
};

struct SPvPStrongholdLinkInfo
{
	ETeam LinkTeam;
	std::string LTrigger;
	std::string RTrigger;

	bool operator == ( SPvPStrongholdLinkInfo const & rhs ) const
	{
		bool Ret = (LTrigger == rhs.LTrigger) ? true : false;
		Ret &= (RTrigger == rhs.RTrigger) ? true : false;
		return Ret;
	}
};

typedef std::vector<SPvPStrongholdLinkInfo> CONT_STRONGHOLD_LINK;

struct SPvPPageControl
{
	SPvPPageControl()
		:	iNowPage(0)
		,	bOnlyWait(false)
	{}

	void Clear()
	{
		iNowPage = 0;
		bOnlyWait = false;
	}

	size_t	iNowPage;
	bool	bOnlyWait;
};

struct SPvPRewardFind
{
	SPvPRewardFind( BM::GUID const &_kCharGuid )
		:	kCharGuid(_kCharGuid)
	{}

	bool operator()( SPvPReward const &kPvPReward )const
	{
		return kPvPReward.kCharGuid == kCharGuid;
	}

	BM::GUID const kCharGuid;
};

class PgPvPRankingUI
	:	public XUI::CXUI_List2_Item
{
public:
	PgPvPRankingUI( BM::GUID const &kOwnerGuid )
		:	XUI::CXUI_List2_Item( kOwnerGuid )
	{}

	virtual ~PgPvPRankingUI(){}

	virtual void Refresh();

	SPvPRanking		m_kRankingInfo;
	size_t			m_iRank;
};

template< typename T=PgPvPRankingUI >
class PgPvPRankingUI_CreatePolicy
	:	public XUI::CXUI_List2_CreatePolicy<T>
{
public:
	explicit PgPvPRankingUI_CreatePolicy( SPvPRanking const &kRankingInfo, size_t const iRank )
		:	m_rkRankingInfo(kRankingInfo)
		,	m_iRank(iRank)
	{}

	virtual void Set( T* pType )const
	{
		pType->m_kRankingInfo = m_rkRankingInfo;
		pType->m_iRank = m_iRank;
	}

private:
	SPvPRanking const	&m_rkRankingInfo;
	size_t const		m_iRank;

private:
	PgPvPRankingUI_CreatePolicy();
};

class PgLobbyUserUI
	:	public XUI::CXUI_List2_Item
{
public:
	struct SSortName
	{
		bool operator()( XUI::CXUI_List2_Item *pLeft, XUI::CXUI_List2_Item *pRight )const
		{
			PgLobbyUserUI *pkLeft = dynamic_cast<PgLobbyUserUI*>(pLeft);
			PgLobbyUserUI *pkRight = dynamic_cast<PgLobbyUserUI*>(pRight);
			if ( pkLeft && pkRight )
			{
				return pkLeft->m_kUserInfo.wstrName < pkRight->m_kUserInfo.wstrName;
			}
			return false;
		}
	};

	struct SSortPos
	{
		bool operator()( XUI::CXUI_List2_Item *pLeft, XUI::CXUI_List2_Item *pRight )const
		{
			PgLobbyUserUI *pkLeft = dynamic_cast<PgLobbyUserUI*>(pLeft);
			PgLobbyUserUI *pkRight = dynamic_cast<PgLobbyUserUI*>(pRight);
			if ( pkLeft && pkRight )
			{
				return pkLeft->m_kUserInfo.iRoomIndex < pkRight->m_kUserInfo.iRoomIndex;
			}
			return false;
		}
	};

public:
	PgLobbyUserUI( BM::GUID const &kOwnerGuid )
		:	XUI::CXUI_List2_Item( kOwnerGuid )
	{}

	virtual ~PgLobbyUserUI(){}

	virtual void Refresh();
	virtual void SetSelect();
	virtual void DelSelect();

	SPvPLobbyUser_Base	m_kUserInfo;	
};

template< typename T=PgLobbyUserUI >
class PgLobbyUserUI_CreatePolicy
	:	public XUI::CXUI_List2_CreatePolicy<T>
{
public:
	explicit PgLobbyUserUI_CreatePolicy( SPvPLobbyUser_Base const &kUserInfo )
		:	m_rkUserInfo(kUserInfo)
	{}

	virtual void Set( T* pType )const{pType->m_kUserInfo = m_rkUserInfo;}

private:
	SPvPLobbyUser_Base const &m_rkUserInfo;

private:
	PgLobbyUserUI_CreatePolicy();
};

class PgPvPGame
	:	public PgContentsBase
{

public:
	typedef std::vector<size_t>			CONT_HILL;
	typedef std::map< int, BM::GUID > CONT_PVPLEAGUE_INDEX_TO_TEAM;
	typedef std::map< int, CONT_DEF_PVPLEAGUE_BATTLE > CONT_BATTLE_GROUP;
	typedef std::map< std::string, int > CONT_PVP_TRIGGER_PARTICLE;

	PgPvPGame(void);
	PgPvPGame(size_t const iMyRank);
	virtual ~PgPvPGame();

	//	UI
	static int GetRandomPvPGround( bool const bDuel, BYTE const ucType = PVP_TYPE_DM, bool const bMatchLevel=false, BYTE const ucNowUser = 0);
	static bool GetPvPGroundType( int const iGndNo, EPVPTYPE &kType, bool bRandom );
	static bool GetGroundName( int const iGndNo, std::wstring &wstrName );
	static bool GetPreviewImgPath( int const iGndNo, std::wstring &wstrPath );
	static bool LoadPvPMaxPlayer( XUI::CXUI_List* pList, int const Type );
	static bool LoadPvPGroundList( XUI::CXUI_List* pList, int const Type, int const UserCount );	
	static void Set_PvPLeagueLevelText(int const iLeagueLevel, BM::vstring &rkOutText);

	bool OnClick_CreateRoom( XUI::CXUI_Wnd *pkUIWnd );
	bool OnClick_JoinRoom( CONT_PVPROOM_LIST::key_type const &iRoomID, std::string const &strPwd );
	bool OnClick_AutoJoinRoom();
	bool OnClick_RandomJoinRoom();
	bool OnClick_RandomInviteUser();
	bool OnClick_ChangePage( size_t iPage );
	bool OnClick_LeagueLobbyChangePage( XUI::CXUI_Wnd *pkUIWnd );
	bool OnClick_LeagueLobbyNextPage( XUI::CXUI_Wnd *pkUIWnd );
	bool OnClick_LeagueLobbyPrevPage( XUI::CXUI_Wnd *pkUIWnd );
	bool OnClick_LeagueLobbyBeginPage( XUI::CXUI_Wnd *pkUIWnd );
	bool OnClick_LeagueLobbyEndPage( XUI::CXUI_Wnd *pkUIWnd );
	void OnClick_ViewWaitRoom( XUI::CXUI_Wnd *pkUI );
	void OnClick_ViewRanking(void);
	void OnClick_GameStart(BYTE const ucType);

	bool SetEdit_Ground( int const iGroundNo, bool bNoSend=false );
	bool SetEdit_Mode( EPVPMODE kMode, bool bNoSend=false );
	bool SetEdit_Type( EPVPTYPE kType, bool bNoSend=false );
	bool SetEdit_Time( int const iMinTime, bool bNoSend=false );
	bool SetEdit_Point( int const iMinPoint, bool bNoSend=false );
	bool SetEdit_MaxLevel( short const sLevel, bool bNoSend=false );
	bool SetEdit_RoundCount( BYTE const kRoundCount, bool bNoSend=false );
	bool SetEdit_Option( BYTE const kOption, bool const bNoSend=false );

	//	bool SendChange_HandyCap( int const iHandyCap );

	void RefreshPageControl( XUI::CXUI_Wnd *pkUIWnd );
	XUI::CXUI_List2* GetLobbyList()const;
	XUI::CXUI_List2* GetLobbyTabList(int const iSetMode)const;
	bool UpdateLobbyList( BM::Stream &kPacket );
	bool SortLobbyList( bool const bName );
	void MyInfoRefresh( SPvPReport const * pkPvPReport );	
	void UpdateLobbyTabList(int const iSetMode, BM::GUID const& kOwnerGuid, SPvPLobbyUser_Base &kElement);
	//

	bool UpdateRankingUI( XUI::CXUI_Wnd * pkWnd, PgPvPRankingMgr const &kPvPRankingMgr );

	virtual EPVPTYPE GetType()const{return PVP_TYPE_ALL;}
	virtual void Clear(bool bClearAll = true);
	virtual bool Update( float const fAccumTime, float const fFrameTime );
	virtual bool ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket );
	virtual void CountDown( DWORD const dwRemainTime );
	virtual void Start(  BM::Stream &kPacket );
	void Result( int iWinTeam );

	virtual bool RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP );
	virtual bool RecvMP( BM::GUID const &kCharGuid, int const iMaxMP, int const iNowMP );

	virtual bool CallResponse();

	virtual bool IsSlowMotionDie( PgActor *pkActor );

	void Recv_PT_T_C_ANS_JOIN_LEAGUE_LOBBY( BM::Stream &kPacket );
	void Recv_PT_T_C_ANS_JOIN_LOBBY( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_ROOMLIST_REMOVE( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_ROOMLIST_UPDATE( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_REFRESH_LEAGUE_LOBBY( BM::Stream &kPacket );
	void RefreshLeagueLobby();
	void RefreshRoomList();
	void Recv_PT_T_C_ANS_JOIN_ROOM( BM::Stream &kPacket );
	void Recv_PT_T_C_ANS_MODIFY_ROOM( BM::Stream &kPacket );
	void Recv_PT_T_C_ANS_TEAM_CHANGE( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_USER_STATUS_CHANGE( BM::Stream &kPacket );
	//	void Recv_PT_T_C_NFY_CHANGE_HANDYCAP( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_EXIT_ROOM_USER( BM::Stream &kPacket );
	void Recv_PT_T_C_RELOAD_LEAGUE_LOBBY( BM::Stream &kPacket );
	void Recv_PT_T_C_ANS_ENTRY_CHANGE( BM::Stream &kPacket );

	void RefreshRoomTitle( XUI::CXUI_Wnd *pkRoomWnd );
	void RefreshRoomAttr( XUI::CXUI_Wnd *pkRoomAttrWnd );
	void RefreshStartButton();
	//	void RefreshHandyCap();

	void Recv_PT_M_C_NFY_GAME_EVENT_CHANGEPOINT( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_EVENT_KILL( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_END( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_RESULT( BM::Stream &kPacket );
	void Recv_PT_T_C_NFY_RELOAD_ROOM( BM::Stream &kPacket );
	void Recv_PT_PM_C_NFY_HILL_TEAM( BM::Stream &kPacket );
	void Recv_PT_T_C_ANS_SLOTSTATUS_CHANGE( BM::Stream &kPacket );

	void SetChangeMaster( BM::GUID const &kNewMasterGuid, BM::GUID const &kOldMasterGuid );

	bool ClickSlot( XUI::CXUI_Wnd *pkSlot, bool bRButton );
	bool KickUser( BM::GUID const &kCharGuid, bool bKick );
	bool EntrustMaster( BM::GUID const &kCharGuid );
	
	//bool ClickEntry( XUI::CXUI_Wnd *pkEntry );
	bool ClickEntry( int const iEntryNo );

	void UpdatePoint( XUI::CXUI_Wnd *pkWnd, XUI::CXUI_Wnd *pkGraphWnd );

	bool IsRoundCount()const{return !((PVP_TYPE_DM==m_kRoomBaseInfo.m_kType) || (PVP_TYPE_KTH==m_kRoomBaseInfo.m_kType)) ;}
	bool IsPersonalGame()const{return PVP_TYPE_DM==m_kRoomBaseInfo.m_kType;}
	bool IsKTHMode() const { return PVP_TYPE_KTH == m_kRoomBaseInfo.m_kType; }
	bool IsLoveMode() const { return PVP_TYPE_LOVE == m_kRoomBaseInfo.m_kType; }

	bool IsLeague(void)const{return PvP_Lobby_GroundNo_League == m_iLobbyID;}
	bool IsRanking()const{return PvP_Lobby_GroundNo_Ranking == m_iLobbyID;}
	bool IsExercise(void)const{return PvP_Lobby_GroundNo_Exercise == m_iLobbyID;}
	bool IsModifyRoomAttr(void)const{return m_bMyRoom && IsExercise();}
	bool IsBalance(void)const{return !IsExercise() && (PVP_MODE_TEAM == m_kRoomBaseInfo.m_kMode);}

	bool IsLeagueType();
	bool IsExerciseType();
	bool IsRoomMaster();
	bool IsMaster();
	void OnClick_EmptyCreateRoom();
	int const GetRankingModeLevel();
	int const GetRankingModeLevelMax();
	int const GetRankingModeLevelMin();

	void UpdateStrongholdLink();	// 링크 정보 업데이트
	void UpdateStrongholdTrigger();	// 점령지 상태 업데이트
	
	CONT_STRONGHOLD_LINK const & GetContLinkData()const { return m_ContLink; }

	int GetEntityPointByTriggerID( char const * pTriggerID )const; // 점령지 트리거 이름으로 해당 가디언의 현재 포인트를 얻는다.
	int GetEntityLevelByTriggerID( char const * pTriggerID )const; // 점령지 트리거 이름으로 해당 가디언의 현재 레벨을 얻는다.
	int GetEntityLinkByTriggerID( char const * pTriggerID ); // 연결된 링크가 몇개인지 알아본다.
	CUnit const* GetEntityByTriggerID( char const * pTriggerID )const; // 점령지 트리거 이름으로 해당 가디언 유닛을 얻는다.
	int GetPointByLevel( int const Level )const; // 점령지 가디언이 해당 레벨이 되기위한 최소 포인트가 몇인지
	int GetLevelByPoint( int const Point )const; // 점령지 가디언의 포인트로 현재 레벨이 몇인지

	int OnUpdateUIEmergencyEscape();	// 긴급회피 스킬 아이콘 업데이트
protected:
	virtual void RecvSyncTeamPoint( BM::Stream& kPacket, bool const bReady = false );

	XUI::CXUI_Wnd* OpenRoom( CONT_PVP_SLOT const &kContSlot );

	void InitSlot( XUI::CXUI_Wnd *pkRoom=NULL, CONT_PVP_SLOT const *pkContSlot=NULL );
	bool OpenSlot( BM::GUID const &kCharGuid, CONT_PVP_GAME_USER::mapped_type &kElement, XUI::CXUI_Wnd *pkOldSolt );
	void ClearSlot( XUI::CXUI_Wnd *pkSlot, bool const bClose, int const iTeam );

	void AddPoint( int const iPoint, int const iTeam );
	virtual bool SyncPoint( int const iPoint, int const iTeam );

	XUI::CXUI_Wnd *GetSlotWnd( XUI::CXUI_Wnd *pkWnd=NULL, int const iTeam=TEAM_NONE )const;
	XUI::CXUI_Wnd *GetSlotWnd( CONT_PVP_GAME_USER::mapped_type const& kElement )const;
	XUI::CXUI_Wnd *GetSlot( XUI::CXUI_Wnd *pkWnd, int const iIndex, int const iTeam )const;
	void GetSlotName( int const iIndex, std::wstring &wstrName )const;

	void RefreshUserPoint( BM::GUID const &kCharGuid, CONT_PVP_GAME_USER::mapped_type &kUserInfo );
	void SortSlot( int iIndex, int iOldIndex, int const iTeam );
	void SortSlot( XUI::CXUI_Wnd*& pkSlot, int const iTeam, int const iPoint, bool const bInc );

	CONT_PVP_GAME_USER::mapped_type *Get( CONT_PVP_GAME_USER::key_type const &kKey );

	void DrawTime( size_t iRemainTimeSec );

	bool GetLeagueBeginTime(int const iLeagueLevel, TBL_DEF_PVPLEAGUE_TIME& rkOut) const;

	void InitTeamInfo();
	void InitGroupInfo();
	bool GetTeamInfo(int const iTeamIndex, TBL_DEF_PVPLEAGUE_TEAM& rkOut) const;
	bool GetTeamInfo(BM::GUID const & rkTeamGuid, TBL_DEF_PVPLEAGUE_TEAM& rkOut) const;
	bool GetBattleInfo(BM::GUID const & rkBattleGuid, TBL_DEF_PVPLEAGUE_BATTLE& rkOut) const;
	void ClearEntryUI();

	void InitTournamentUI(XUI::CXUI_Wnd *pkWndTournament);
	void InitTeamListUI(XUI::CXUI_Wnd *pkWndTeamList);
	void InitRoomListUI(XUI::CXUI_Wnd *pkWndRoomList);
	void InitGroupUI(XUI::CXUI_Wnd *pkWndGroup, int const iIndexUI, TBL_DEF_PVPLEAGUE_TEAM const &rkTeamInfo);
	void ClearGroupUI(XUI::CXUI_Wnd *pkWndGroup, int const iIndexUI);
	bool Set_PvPLeagueEndTimeText(XUI::CXUI_Wnd * pkWnd) const;
	bool Set_PvPLeagueBeginTimeText(XUI::CXUI_Wnd * pkWnd, int const iLeagueLevel) const;
	bool Set_PvPLeagueExplainText(XUI::CXUI_Wnd * pkWnd, int const iLeagueLevel) const;

	void BuildStrongholdLink();

	void Recv_PT_M_C_NFY_ADD_POINT_USER( BM::Stream &kPacket );
	void Reve_PT_M_C_NFY_OCCUPY_POINT_SECTION_INFO( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_KOH_INSERT_GUARDIAN( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_UPDATE_LOVE_FENCE( BM::Stream &kPacket );	// 러브모드 울타리가 리젠 또는 파괴되었을 때 트리거변수를 갱신한다.
	void Recv_PT_M_C_NFY_DISPLAY_DROPBEAR_TIMER( BM::Stream &kPacket ); // 러브모드 곰이 드랍되었을 때 타이머 동작 처리
	void Recv_PT_M_C_NFY_FOCUS_TOUCH_DOWN_USER( BM::Stream & Packet ); // 스코어 획득할 때 점수 획득한 캐릭터에게 카메라 맞춤.

	DWORD GetTeamFontColor( int const iTeam )const;
	void ViewKillMsg( BM::GUID const &kKillerID, BM::GUID const &kDieID );
protected:
	//	Lobby에서 사용하는거
	int						m_iLobbyID;
	SPvPPageControl			m_kPage;
	CONT_PVPROOM_LIST		m_kContRoom;
	int						m_iBeginPageNo;
	int						m_iEndPageNo;

	CONT_PVPROOM_LIST::key_type	m_iRoomKey;
	SPvPRoomBasicInfo		m_kRoomBaseInfo;
	SPvPRoomExtInfo			m_kRoomExtInfo;

	bool					m_bMyRoom;

	BYTE					m_ucNowRound;// 현재 몇번째 라운드 인지
	BYTE					m_ucRoundScore[3];// 라운드의 스코어 0번 인덱스는 비긴횟수이다.

	CONT_PVP_GAME_USER		m_kContPvPUser;
	CONT_PVP_GAME_REWARD	m_kReward;
	CONT_PVP_TEAM_RESULT	m_kContTeamResult;

	CONT_STRONGHOLD_LINK		m_ContLink;				// 점령전 링크 정보
	CONT_PVP_TRIGGER_ENTITY		m_ContTriggerEntity;	// 점령전 트리거, 엔티티 정보
	CONT_PVP_TRIGGER_PARTICLE	m_ContTriggerParticle;	// 점령전 트리거, 파티클 슬롯
	SPvPLevelUpPointInfo		m_LevelUpPointInfo;		// 점령지 레벨업 수치 정보

	float					m_fCameraZoom;

	// 랭킹
	size_t					m_iMyRank;
	PgPvPRankingMgr			m_kRankIngMgr;

	// 리그
	CONT_DEF_PVPLEAGUE_TIME m_kContDefPvPLeagueTime;
	TBL_DEF_PVPLEAGUE m_kLeagueInfo;
	TBL_DEF_PVPLEAGUE_TOURNAMENT m_kTournamentInfo;

	CONT_DEF_PVPLEAGUE_BATTLE m_kContBattle;
	CONT_DEF_PVPLEAGUE_TEAM m_kContTeam;

	CONT_PVPLEAGUE_CHAR_TO_TEAM m_kContCharToTeam;
	CONT_PVPLEAGUE_INDEX_TO_TEAM m_kContIndexToTeam;
	
	CONT_BATTLE_GROUP m_kContGroup;
};

inline CONT_PVP_GAME_USER::mapped_type *PgPvPGame::Get( CONT_PVP_GAME_USER::key_type const &kKey )
{
	CONT_PVP_GAME_USER::iterator user_itr = m_kContPvPUser.find(kKey);
	if ( user_itr != m_kContPvPUser.end() )
	{
		return &(user_itr->second);
	}
	return NULL;
}

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_PGPVPGAME_H