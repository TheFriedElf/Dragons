#ifndef FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET_H
#define FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET_H

extern bool HandlePacket(WORD const wPacketType, BM::Stream &rkPacket);

extern void Recv_PT_S_C_REQ_PING(BM::Stream * const pkPacket);
extern bool Recv_PT_X_X_ANS_ERROR_CHECK(HRESULT const Result);
extern void Set_PvPSelectorUI( XUI::CXUI_Wnd *pkUI, __int64 const i64NowTimeInDay, int const iID );
extern void DirectJoin_PvPLeagueLobby(XUI::CXUI_Wnd * pkWnd);
extern bool Set_PvPLeagueTimeUI(XUI::CXUI_Wnd * pkWnd);
extern void ItemUseResultMsg( int const iMsg );

#include "HandlePacket01.h"

#endif // FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET_H