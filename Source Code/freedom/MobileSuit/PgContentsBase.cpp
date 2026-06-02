#include "stdafx.h"
#include "Variant/PgEventView.h"
#include "PgWorld.h"
#include "PgContentsBase.h"

std::wstring const PgContentsBase::ms_SFRM_PVP_RANKING = std::wstring(L"SFRM_PVP_RANKING");

std::wstring const PgContentsBase::ms_FRM_CLICK_FRAME = std::wstring(L"FRM_CLICK_FRAME");
std::wstring const PgContentsBase::ms_FRM_PVP_JOIN_PASSWORD = std::wstring(L"FRM_PVP_JOIN_PASSWORD");
std::wstring const PgContentsBase::ms_FRM_PVP_BACKGROUND = std::wstring(L"FRM_PVP_BACKGROUND");
std::wstring const PgContentsBase::ms_FRM_PVP_LOBBY = std::wstring(L"FRM_PVP_LOBBY");
std::wstring const PgContentsBase::ms_FRM_PVP_LEAGUE = std::wstring(L"FRM_PVP_LEAGUE");
std::wstring const PgContentsBase::ms_FRM_PVP_MAIN = std::wstring(L"FRM_PVP_MAIN");
std::wstring const PgContentsBase::ms_FRM_PVP_ROOM[2] = { std::wstring(L"FRM_PVP_ROOM_TEAM"), std::wstring(L"FRM_PVP_ROOM_PERSONAL") };
std::wstring const PgContentsBase::ms_FRM_PVP_ROOM_ATTR = std::wstring(L"FRM_PVP_ROOM_ATTR");
std::wstring const PgContentsBase::ms_FRM_PVP_START = std::wstring(L"FRM_PVP_START");
std::wstring const PgContentsBase::ms_FRM_PVP_RESULT = std::wstring(L"FRM_PVP_RESULT");
std::wstring const PgContentsBase::ms_FRM_PVP_RESULT_ITEM = std::wstring(L"FRM_PVP_RESULT_ITEM");

std::wstring const PgContentsBase::ms_FRM_EMBATTLE_MAIN = std::wstring(L"FRM_EMBATTLE_MAIN");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_WAIT_STATUS = std::wstring(L"FRM_EMBATTLE_WAIT_STATUS");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_CORE_STATUS = std::wstring(L"FRM_EMBATTLE_CORE_STATUS");
std::wstring const PgContentsBase::ms_LST_EMBATTLE_KILL_MSG = std::wstring(L"LST_EMBATTLE_KILL_MSG");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_RESULT = std::wstring(L"FRM_EMBATTLE_RESULT");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_RESULT_MYINFO = std::wstring(L"FRM_EMBATTLE_MY_RESULT");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_COREDESTROY = std::wstring(L"FRM_EMBATTLE_COREDESTROY");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_PROTECTDRAGON = std::wstring(L"FRM_EMBATTLE_PROTECTDRAGON");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_TIMEOVER = std::wstring(L"FRM_EMBATTLE_TIMEOVER");
std::wstring const PgContentsBase::ms_FRM_EMBATTLE_TABSTATE = std::wstring(L"FRM_EMBATTLE_TABSTATE");

std::wstring const PgContentsBase::ms_FRM_EMBATTLE_MAIN_DRAGON = std::wstring(L"FRM_EMBATTLE_MAIN_DRAGON");
std::wstring const PgContentsBase::ms_SFRM_EMBATTLE_PTDRA_TAB = std::wstring(L"SFRM_EMBATTLE_PTDRA_TAB");

std::wstring const PgContentsBase::ms_SFRM_NAME[2] = { std::wstring(L"SFRM_NAME"), std::wstring(L"SFRM_NAME_MINE") };
std::wstring const PgContentsBase::ms_SFRM_KILL[2] = { std::wstring(L"SFRM_KILL"), std::wstring(L"SFRM_KILL_MINE") };
std::wstring const PgContentsBase::ms_SFRM_DEATH[2] = { std::wstring(L"SFRM_DEATH"), std::wstring(L"SFRM_DEATH_MINE") };
std::wstring const PgContentsBase::ms_SFRM_POINT[2] = { std::wstring(L"SFRM_POINT"), std::wstring(L"SFRM_POINT_MINE") };
std::wstring const PgContentsBase::ms_IMG_RANK[2] = { std::wstring(L"IMG_RANK"), std::wstring(L"IMG_RANK_MINE") };

std::wstring const PgContentsBase::ms_FRM_PVP_RESPONSE = std::wstring(L"FRM_PVP_RESPONSE");

std::wstring const PgContentsBase::ms_SFRM_MAIN_BG_EXPLAIN = std::wstring(L"SFRM_MAIN_BG_EXPLAIN");
std::wstring const PgContentsBase::ms_SFRM_MAIN_BG_TOURNAMENT = std::wstring(L"SFRM_MAIN_BG_TOURNAMENT");
std::wstring const PgContentsBase::ms_SFRM_MAIN_BG_TEAM_LIST = std::wstring(L"SFRM_MAIN_BG_TEAM_LIST");
std::wstring const PgContentsBase::ms_SFRM_MAIN_BG_ROOM_LIST = std::wstring(L"SFRM_MAIN_BG_ROOM_LIST");

PgContentsBase* PgContentsBase::ms_pkContents = NULL;

PgContentsBase::PgContentsBase()
:	m_dwEndTime(0)
,	m_eStatus(STATUS_NONE)
,	m_iKillCount(0)
,	m_iSetMode(0)
{
	New();
}

PgContentsBase::~PgContentsBase()
{
	Delete();
}

void PgContentsBase::New()
{
	assert(ms_pkContents);
	ms_pkContents = this;
}

void PgContentsBase::Delete()
{
	if ( ms_pkContents == this )
	{
		ms_pkContents = NULL;
	}
}

void PgContentsBase::Clear()
{
	m_dwEndTime = 0;
	m_eStatus = STATUS_NONE;
	m_iKillCount = 0;
	m_iSetMode = 0;
}

void PgContentsBase::RecvSyncRemainTime(BM::Stream& kPacket)
{
	DWORD dwRemainTime = 0;
	kPacket.Pop(dwRemainTime);

	DWORD const dwCurTime = BM::GetTime32();
	m_dwEndTime = dwCurTime + dwRemainTime;
}

bool PgContentsBase::ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket )
{
	switch( wType )
	{
	case PT_M_C_NFY_GAME_READY:
		{
			DWORD dwRemainTime = 0;
			kPacket.Pop(dwRemainTime);
			this->CountDown(dwRemainTime);
		}break;
	case PT_M_C_NFY_SYNC_GAMTTIME:
		{
			this->RecvSyncRemainTime(kPacket);
		}break;
	default:
		{
			return false;
		}break;
	}
	
	return true;
}
