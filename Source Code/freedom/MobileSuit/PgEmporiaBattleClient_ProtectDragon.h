#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_PROTECTDRAGON_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_PROTECTDRAGON_H

#include "PgEmporiaBattleClient.h"

enum E_SLOT_TYPE
{
	EST_MAIN_TOP_VIEW	= 0,
	EST_TAB_VIEW,
	EST_MAIN_BIG_DRAGON,
};

class PgEmporiaBattleClient_ProtectDragon
	:	public PgEmporiaBattleClient
{
public:
	PgEmporiaBattleClient_ProtectDragon();
	virtual ~PgEmporiaBattleClient_ProtectDragon();

	virtual EPVPTYPE GetType()const{return WAR_TYPE_PROTECTDRAGON;}

	virtual bool ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket );
	virtual bool RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP );

	virtual void ReadFromStartAddOnPacket(BM::Stream &kPacket);

	virtual bool SyncPoint( WORD const nUserKillPoint_Atk, WORD const nCoreBreakPoint_Atk, WORD const nUserKillPoint_Def, WORD const nCoreBreakPoint_Def );

	virtual void StartUI();

	virtual void Recv_PT_M_C_NFY_GAME_INTRUDER( BM::Stream &kPacket );
	virtual void Recv_PT_M_C_NFY_GAME_EXIT( BM::Stream &kPacket );
	virtual void Recv_PT_M_C_NFY_BATTLE_AREA_USER( BM::Stream &kPacket );
	void Recv_PT_M_C_NFY_BATTLE_CHANGE_AREA_TO_BIGDRAGON( BM::Stream& kPacket);

protected:
	virtual bool DestroyCore( BM::GUID const &kCoreGuid );
	virtual void BuildBattleAreaInfo(void);

	void SetDragonAreaInfo( SBattleAreaInfo const &kInfo, E_SLOT_TYPE const SlotType = EST_MAIN_TOP_VIEW )const;
	void SetDragonAreaInfo(XUI::CXUI_Wnd* pSlot, SBattleAreaInfo const& kInfo, E_SLOT_TYPE const SlotType ) const;
	void UpdateDefWinLimitCount(XUI::CXUI_Wnd* pMainUI = NULL) const;

	virtual void SetPointEffect( BM::GUID const &kKillerID, BM::GUID const &kDieID, short const usAddPoint, bool const bDestroyCore );
	int const GetTeamUserCount( int const iTeam ) const;

private:
	virtual XUI::CXUI_Wnd *GetWnd( bool bActivate )const;

//	int	m_iRebirthCount;
	WORD					m_usDefWinPointLimit;
	bool					m_bCallBigDragon;
};

inline XUI::CXUI_Wnd *PgEmporiaBattleClient_ProtectDragon::GetWnd( bool bActivate )const
{
	if ( bActivate )
	{
		return XUIMgr.Activate(ms_FRM_EMBATTLE_MAIN_DRAGON);
	}
	return XUIMgr.Get(ms_FRM_EMBATTLE_MAIN_DRAGON);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_PGEMPORIABATTLECLIENT_PROTECTDRAGON_H