#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_HARDCOREDUNGEON_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_HARDCOREDUNGEON_H

class PgHardCoreDungeon
{
public:
	static std::wstring const ms_SFRM_HCD_VOTE;

public:
	static void RegisterWrapper(lua_State *pkState);
	static bool lwNet_JoinHardCoreDungeon( int const iType );

	static void Recv_PT_M_C_ANS_REGIST_HARDCORE_VOTE( BM::Stream &rkPacket );
	static void RemoveMember( BM::GUID const &kCharGuid );
	static bool UpdateRemainTime( __int64 const i64EndTime );
	static void UpdateBossGndInfo( BM::GUID const &kPartyGuid, __int64 const i64EndTime );
	static bool IsVote();

protected:
	static void SetEmptyUI( XUI::CXUI_Wnd *pkUI );
};

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_HARDCOREDUNGEON_H