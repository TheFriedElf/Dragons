#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_PGCONTENTSBASE_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_PGCONTENTSBASE_H

#include "Lohengrin/PacketStruct2.h"
#include "Variant/Global.h"

typedef enum eKillMsgMark
{
	KMM_DEFAULT = 1,
	KMM_TARGETKILL = 1,
	KMM_SELFKILL = 2,
}EKILL_MSG_MARK;

int const MAX_KILL_NFY_MSG = 3;

class PgContentsBase
{
public:// 상수
	static std::wstring const ms_SFRM_PVP_RANKING;

	static std::wstring const ms_FRM_CLICK_FRAME;
	static std::wstring const ms_FRM_PVP_JOIN_PASSWORD;
	static std::wstring const ms_FRM_PVP_BACKGROUND;
	static std::wstring const ms_FRM_PVP_LOBBY;
	static std::wstring const ms_FRM_PVP_LEAGUE;
	static std::wstring const ms_FRM_PVP_MAIN;
	static std::wstring const ms_FRM_PVP_ROOM[2];
	static std::wstring const ms_FRM_PVP_ROOM_ATTR;
	static std::wstring const ms_FRM_PVP_START;
	static std::wstring const ms_FRM_PVP_RESULT;
	static std::wstring const ms_FRM_PVP_RESULT_ITEM;

	static std::wstring const ms_FRM_EMBATTLE_MAIN;
	static std::wstring const ms_FRM_EMBATTLE_WAIT_STATUS;
	static std::wstring const ms_FRM_EMBATTLE_CORE_STATUS;
	static std::wstring const ms_LST_EMBATTLE_KILL_MSG;
	static std::wstring const ms_FRM_EMBATTLE_RESULT;
	static std::wstring const ms_FRM_EMBATTLE_RESULT_MYINFO;
	static std::wstring const ms_FRM_EMBATTLE_COREDESTROY;
	static std::wstring const ms_FRM_EMBATTLE_PROTECTDRAGON;
	static std::wstring const ms_FRM_EMBATTLE_TIMEOVER;
	static std::wstring const ms_FRM_EMBATTLE_TABSTATE;

	static std::wstring const ms_FRM_EMBATTLE_MAIN_DRAGON;
	static std::wstring const ms_SFRM_EMBATTLE_PTDRA_TAB;

	static std::wstring const ms_SFRM_NAME[2];
	static std::wstring const ms_SFRM_KILL[2];
	static std::wstring const ms_SFRM_DEATH[2];
	static std::wstring const ms_SFRM_POINT[2];
	static std::wstring const ms_IMG_RANK[2];

	static std::wstring const ms_FRM_PVP_RESPONSE;

	static std::wstring const ms_SFRM_MAIN_BG_EXPLAIN;
	static std::wstring const ms_SFRM_MAIN_BG_TOURNAMENT;
	static std::wstring const ms_SFRM_MAIN_BG_TEAM_LIST;
	static std::wstring const ms_SFRM_MAIN_BG_ROOM_LIST;

public:
	enum Status
	{
		STATUS_NONE			= 0,
		STATUS_LOBBY		= 1,
		STATUS_ROOM			= 2,
		STATUS_READY		= 3,
		STATUS_COUNTDOWN	= 4,
		STATUS_PLAYING		= 5,
		STATUS_ENDBEFORE	= 6,
		STATUS_RESULT		= 7,
	};

public:
	static PgContentsBase*		ms_pkContents;

	static void RegisterWrapper(lua_State *pkState);
	static Status GetContentsStatus();
	static void SetPvPLock( bool const bLock );
	static void SetPvpTabListMode(int const iSetMode);
	static int GetPvpTabListMode();

public:
	PgContentsBase();
	virtual ~PgContentsBase();

	virtual EPVPTYPE GetType()const{return PVP_TYPE_NONE;}
	virtual void Clear();
	virtual bool Update( float const fAccumTime, float const fFrameTime ) = 0;
	virtual bool ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket );
	virtual void CountDown( DWORD const dwRemainTime ) = 0;
	virtual void Start( BM::Stream& kPacket ) = 0;
	virtual bool RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP ) = 0;
	virtual bool RecvMP( BM::GUID const &kCharGuid, int const iMaxMP, int const iNowMP ) = 0;

	bool SetStatus(Status eStatus);
	Status GetStatus()const{return m_eStatus;}
	virtual void TabGame( bool bDisplay ){}
	virtual bool IsSlowMotionDie( PgActor * pkActor ){return false;}
	virtual bool CallResponse() = 0;
	virtual void CallLocationJoinUI(bool const bTabGame=false){}
	virtual bool IsFullArea(int const iIndex)const{return false;}

protected:
	void RecvSyncRemainTime(BM::Stream &kPacket);
	virtual void RecvSyncTeamPoint( BM::Stream &kPacket, bool const bReady = false ) = 0;

	void GetRemainTime( size_t &iSecTime, size_t *piMSTime=NULL )const;

	virtual bool SyncPoint( int const iPoint, int const iTeam ) = 0;

private:// Private!!!!!!!!!!!!!!!
	void New();
	void Delete();

protected:
	// Timer

	Status		m_eStatus;
	DWORD		m_dwEndTime;
	size_t		m_iKillCount;
	int			m_iSetMode;
};

inline bool PgContentsBase::SetStatus(Status eStatus)
{
	if ( m_eStatus == eStatus )
	{
		return false;
	}
	m_eStatus = eStatus;
	return true;
}

inline void PgContentsBase::GetRemainTime( size_t &iSecTime, size_t *piMSTime )const
{
	DWORD dwRemainTime = ::DifftimeGetTime( BM::GetTime32(), m_dwEndTime );
	
	iSecTime = (size_t)dwRemainTime / 1000;
	if ( piMSTime )
	{
		*piMSTime = (size_t)dwRemainTime % 1000;
	}

	++iSecTime;// 미리초가 있으면 1초전에 표시해주어야한다.
}

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_PGCONTENTSBASE_H