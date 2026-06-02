#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_H

#include "lwUI.h"
#include "PgContentsBase.h"
#include "Variant/PgEmporia.h"

struct SEmporiaBattleInfo_Client
:	SEmporiaBattleObjectInfo
{
	SEmporiaBattleInfo_Client()
	{}

	std::wstring	wstrUI;
	std::wstring	wstrEftUI;
};
typedef std::map< BM::GUID, SEmporiaBattleInfo_Client >		CONT_EMBATTLE_OBJECT_INFO_CLIENT;

typedef struct tagCoreHealEffectInfo
{
	tagCoreHealEffectInfo()
		: HealValue(0)
		, CoreCount(0)
	{}
	int HealValue;
	int CoreCount;
}SCoreHealEffectInfo;

typedef struct tagHPInfo
{
	tagHPInfo()
		:	iMaxHP(0)
		,	iCurrentHP(0)
	{}

	tagHPInfo( int const iMax, int const iCurrent )
		:	iMaxHP(iMax)
		,	iCurrentHP(iCurrent)
	{}

	int iMaxHP;
	int iCurrentHP;
}SHPInfo;

typedef struct tagMYKILLPOINT
{
	tagMYKILLPOINT()
		: PC_KILL(0)
		, CORE_DESTROY(0)
	{};
	WORD PC_KILL;
	WORD CORE_DESTROY;
}SMYKILLPOINT;

struct	SBattleAreaInfo
{
	SBattleAreaInfo()
		: iIndex(0), MaxDragonHP(1), NowDragonHP(0)
	{}

	SBattleAreaInfo( BM::GUID const &kID, SEmporiaBattleInfo_Client const &kBattleInfo )
		:	MaxDragonHP(kBattleInfo.iHP)
		,	NowDragonHP(kBattleInfo.iHP)
		,	DragonGuid(kID)
		,	iIndex(kBattleInfo.iBattleAreaIndex)
	{}

	SET_GUID kContAtkUser;
	SET_GUID kContDefUser;
	BM::GUID DragonGuid;
	int iIndex;
	int MaxDragonHP;
	int NowDragonHP;
};

typedef std::map< int, SBattleAreaInfo >		CONT_BATTLEAREA_INFO;

class PgEmporiaBattleClient
	:	public PgContentsBase
{
public:
	static DWORD GetTeamFontColor( int const iTeam );

public:
	PgEmporiaBattleClient();
	virtual ~PgEmporiaBattleClient();

	virtual EPVPTYPE GetType()const{return WAR_TYPE_DESTROYCORE;}
	virtual void Clear();

	virtual bool Update( float const fAccumTime, float const fFrameTime );
	virtual bool ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket );
	virtual void CountDown( DWORD const dwRemainTime );
	virtual void Start( BM::Stream& kPacket );
	virtual bool RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP );
	virtual bool RecvMP( BM::GUID const &kCharGuid, int const iMaxMP, int const iNowMP ){return false;}

	virtual void ReadFromStartAddOnPacket(BM::Stream &kPacket);

	virtual bool CallResponse();
	virtual void CallLocationJoinUI(bool const bTabGame=false);
	virtual bool IsFullArea(int const iIndex)const;
	void RandomMoveBattleArea()const;

	virtual bool SyncPoint( int const iPoint, int const iTeam ){ return true; }
	virtual bool SyncPoint( WORD const nUserKillPoint_Atk, WORD const nCoreBreakPoint_Atk, WORD const nUserKillPoint_Def, WORD const nCoreBreakPoint_Def );
	virtual void SetMaxPoint( int const iPoint );
	void CloseKillEffect(bool const bIsCore, int const iPoint);
	virtual void TabGame( bool bDisplay );
	void RefreshCoreHP( BM::GUID const &kCoreGuid );

protected:
	virtual void BuildBattleAreaInfo(void);
	void SetCoreBalance( XUI::CXUI_Wnd *pkWnd );
	virtual void RecvSyncTeamPoint( BM::Stream& kPacket, bool const bReady = false );
	void SetChangeLeaderTeam( int const iLeaderTeam );

	virtual void Recv_PT_M_C_NFY_GAME_INTRUDER( BM::Stream &kPacket );
	virtual void Recv_PT_M_C_NFY_GAME_EXIT( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_EVENT_KILL( BM::Stream &kPacket );
	virtual void Recv_PT_M_C_NFY_ADD_POINT_USER( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_BATTLE_AREA_USER( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_END( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_GAME_RESULT_END( BM::Stream &kPacket );

protected:
//	void ShowCoreState( XUI::CXUI_Wnd *pkWnd, int iNum, BM::GUID const &kCoreGuid, int const iTeam );
	virtual bool DestroyCore( BM::GUID const &kCoreGuid );
	void ShowResult();
	void RefreshCoreHP( BM::GUID const &kCoreGuid, int const iValueHP, int const iMaxHP );
	void RefreshCoreHP( CONT_EMBATTLE_OBJECT_INFO_CLIENT::mapped_type &kCoreInfo, int const iValueHP, int const iMaxHP );
	void SetVisibleSubCoreBG(XUI::CXUI_Wnd* pkWnd, int const iTeam, int iNum, bool const bIsView = true);
	void StartKillEffect(int const Team, WORD const Point, POINT2 const StartPos, bool const bIsCore);

	void UpdateGuildUserInfo(BM::GUID const& kKill, BM::GUID const& kDie, int const iAddPoint);
	void DivideAndSortTeamUserInfo(CONT_EMPORIA_BATTLE_RESULT& kATK, CONT_EMPORIA_BATTLE_RESULT& kDEF);
	bool SetGuildUserInfo(XUI::CXUI_Wnd* pWnd, SGuildBattleInfo const& kTeam, CONT_EMPORIA_BATTLE_RESULT const& kUser, std::wstring const& AddText, int& rTotalHeight);
	void SetGuildUserItem(XUI::CXUI_Wnd* pWnd, SGuildBattleUser const& kUser, int const iLank, bool& IsMe);
	void SetGuildUserInfoBG(XUI::CXUI_Wnd* pWnd, int const iMiddleHeight, std::wstring const& AddText, bool const bIsExist);

	virtual void SetPointEffect( BM::GUID const &kKillerID, BM::GUID const &kDieID, short const usAddPoint, bool const bDestroyCore );

	template<typename T>
	void SetUIText(XUI::CXUI_Wnd* pWnd, std::wstring const& kChildID, T const& Text)
	{
		XUI::CXUI_Wnd* pTemp = pWnd->GetControl(kChildID);
		if( pTemp )
		{
			BM::vstring	vStr(Text);
			pTemp->Text(vStr);
		}
	}

	void DrawTime( size_t iRemainTimeSec, size_t iRemainTimeMS );

	virtual void StartUI();

private:
	virtual XUI::CXUI_Wnd *GetWnd( bool bActivate )const;
	XUI::CXUI_Wnd *GetCoreWnd(int const iTeam, bool bActivate )const;
	void SetMyTeam();
	void RemoveNPC()const;

protected:
	int							m_iMyTeam;
	SGuildBattleInfo			m_kDefenceGuild;
	SGuildBattleInfo			m_kAttackGuild;

	CONT_EMBATTLE_OBJECT_INFO_CLIENT	m_kContObejctInfo;

	CONT_EMPORIA_BATTLE_USER			m_kContUser;

	SMYKILLPOINT						m_MyKillPoint;

	CONT_BATTLEAREA_INFO				m_kContBattleAreaInfo;

	size_t								m_iUserCount_MaxInTeam;
	WORD								m_usATKAuthTotalPoint;

	DWORD								m_dwReBirthTime;

	bool								m_bTimeOver;
	int									m_iWinTeam;
	bool								m_bCoreDestory;
};

inline XUI::CXUI_Wnd *PgEmporiaBattleClient::GetWnd( bool bActivate )const
{
	if ( bActivate )
	{
		return XUIMgr.Activate(ms_FRM_EMBATTLE_MAIN);
	}
	return XUIMgr.Get(ms_FRM_EMBATTLE_MAIN);
}

inline XUI::CXUI_Wnd *PgEmporiaBattleClient::GetCoreWnd(int const iTeam, bool bActivate )const
{
	BM::vstring vMain(L"FRM_EMBATTLE_HP_");
	vMain += (TEAM_RED==iTeam ? L"RED" : L"BLUE");
	if ( bActivate )
	{
		return XUIMgr.Activate(vMain);
	}
	return XUIMgr.Get(vMain);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_H