#ifndef FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET01_H
#define FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET01_H

extern void	Recv_PT_M_C_NFY_STATE_CHANGE2(PgPilot *pkPilot, E_SENDABIL_TARGET eType, SAbilInfo const &rkAbilInfo);
extern void Recv_PT_M_C_NFY_STATE_CHANGE64(PgPilot *pkPilot, SAbilInfo64 const &rkAbilInfo);
extern void Recv_PT_M_C_NFY_QUICKSLOTCHANGE(BM::Stream &rkPacket);
extern void Recv_PT_M_C_NFY_PET_INVENTORY_ACTION(BM::Stream &rkPacket);
extern void Recv_PT_M_C_NFY_ITEM_CHANGE(BM::Stream *pkPacket);
extern void Recv_PT_M_C_NFY_WARN_MESSAGE(BM::Stream *pkPacket);
extern void Recv_PT_M_C_NFY_WARN_MESSAGE2(BM::Stream *pkPacket);
extern void Recv_PT_M_C_NFY_WARN_MESSAGE3(BM::Stream *pkPacket);
extern void Recv_PT_M_C_NFY_WARN_MESSAGE_STR(BM::Stream *pkPacket);
//extern void Recv_PT_M_C_ANS_GBOXINFO(BM::Stream *pkPacket);
extern void Recv_PT_M_C_ANS_PICKUPGBOX(BM::Stream *pkPacket);
extern void Recv_PT_M_C_ANS_STORE_ITEM_LIST(BM::Stream *pkPacket);
extern void Recv_PT_M_C_NFY_SHINESTONE_MSG(BM::Stream* pkPacket);
extern void Recv_PT_C_M_ANS_ITEM_PLUS_UPGRADE(BM::Stream* pkPacket);
extern void Recv_PT_C_M_ANS_ITEM_RARITY_UPGRADE(BM::Stream* pkPacket);
extern void Recv_PT_M_C_NFY_CHANGE_MONEY(BM::Stream& rkPacket);
extern void Recv_PT_M_C_NFY_CHANGE_COMBO_COUNT(BM::Stream* pkPacket);
extern void Recv_PT_M_C_NFY_MAPMOVE_COMPLETE(BM::Stream& rkPacket);
extern void Recv_PT_M_C_NFY_REMOVE_CHARACTER(BM::Stream& rkPacket, int const iCallType);
extern void Recv_PT_M_C_NFY_CHANGE_CP(BM::Stream& rkPacket);
extern void Recv_PT_M_C_NFY_CHANGE_MISSIONSCORE_COUNT(BM::Stream* rkPacket);
extern void Recv_PT_S_C_NFY_REFRESH_DATA(BM::Stream& rkPacket);
extern void Revc_PT_M_C_MISSION_ABILITY_DEMAGE(BM::Stream* rkPacket);
extern void Recv_PT_M_C_ANS_GEN_SOCKET(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_RESET_MONSTERCARD(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_EXTRACTION_MONSTERCARD(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_SET_MONSTERCARD(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_OPEN_LOCKED_CHEST(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_OPEN_GAMBLE(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_EVENT_ITEM_REWARD(BM::Stream* pkPacket);
extern void Recv_PT_M_C_ANS_CONVERTITEM(BM::Stream* pkPacket);
extern void Recv_PT_M_C_NFY_EMPORIA_FUNCTION( PgWorld *pkWorld, BM::Stream &rkPacket);
extern void Revc_PT_M_C_MISSION_RANK_RESULT_ITEM(BM::Stream* rkPacket);
extern void Recv_PT_M_C_REQ_HIDDEN_MOVE_CHECK(BM::Stream &rkPacket);
extern void Recv_PT_M_C_ANS_REMOVE_MONSTERCARD(BM::Stream* rkPacket);

extern void Recv_PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT(BM::Stream & Packet);
extern void Recv_PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT(BM::Stream & Packet);
extern void Recv_PT_M_C_NFY_CHANGE_GUARDIAN_INSTALL_DICOUNT(BM::Stream & Packet);

#endif // FREEDOM_DRAGONICA_NETWORK_HANDLEPACKET01_H