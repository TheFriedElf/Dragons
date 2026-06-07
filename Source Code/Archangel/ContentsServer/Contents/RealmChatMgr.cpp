#include "StdAfx.h"
#include "Collins/Log.h"
#include "Variant/PgLogUtil.h"
#include "Variant/Global.h"
#include "RealmChatMgr.h"
#include "Global.h"
#include "PgTask_Contents.h"
#include "PgGuildMgr.h"
#include "Lohengrin/PacketStructLog.h"
#include <chrono>

PgRealmChatMgr::PgRealmChatMgr(void)
{
}

PgRealmChatMgr::~PgRealmChatMgr(void)
{
}

//
bool PgRealmChatMgr::SendChat_Whisper(int const iChatMode, SContentsUser const &rkFromUser, SContentsUser const &rkToUser, std::wstring const &rkContents, BM::Stream const& rkAddonPacket)
{
	PgChatLogUtil::Log(ELogSub_Chat_Whisper, rkFromUser, rkToUser, rkContents);

	BM::Stream kPacket(PT_M_C_NFY_CHAT);
	kPacket.Push((BYTE)CT_WHISPER_BYGUID);//어떤 귓속말이던 상관없음
	kPacket.Push(rkFromUser.kCharGuid);
	kPacket.Push(rkFromUser.Name());
	kPacket.Push(std::chrono::system_clock::now());
	kPacket.Push(rkAddonPacket);

	bool bRet = false;
	bRet = g_kRealmUserMgr.Locked_SendToUser(rkToUser.kMemGuid, kPacket);
	bRet = g_kRealmUserMgr.Locked_SendToUser(rkFromUser.kMemGuid, kPacket);
	return true;
}

bool PgRealmChatMgr::SendChat_ManToMan(SContentsUser const& rkFromUser, SContentsUser const& rkToUser, std::wstring const &rkContents, BM::Stream const& rkAddonPacket)
{
	PgChatLogUtil::Log(ELogSub_Chat_ManToMan, rkFromUser, rkToUser, rkContents);

	//	일단 따로 보내자
	bool bRet = false;
	BM::Stream kToPacket(PT_M_C_NFY_CHAT);//패킷 선 준비
	kToPacket.Push((BYTE)CT_MANTOMAN);
	kToPacket.Push(rkFromUser.kCharGuid);
	kToPacket.Push((BYTE)0);
	kToPacket.Push(rkFromUser.Name());
	kToPacket.Push(rkAddonPacket);

	BM::Stream kFromPacket(PT_M_C_NFY_CHAT);
	kFromPacket.Push((BYTE)CT_MANTOMAN);
	kFromPacket.Push(rkToUser.kCharGuid);
	kFromPacket.Push((BYTE)1);
	kFromPacket.Push(rkToUser.Name());
	kFromPacket.Push(rkAddonPacket);

	bRet = g_kRealmUserMgr.Locked_SendToUser(rkToUser.kMemGuid, kToPacket);
	bRet = g_kRealmUserMgr.Locked_SendToUser(rkFromUser.kMemGuid, kFromPacket);
	return true;	
}

bool PgRealmChatMgr::RecvChat_ManToMan(SContentsUser const& rkFromUser, SContentsUser const& rkToUser, std::wstring const &rkContents, BM::Stream const& rkAddonPacket)
{
	if(rkFromUser.kCharGuid == BM::GUID::NullData())
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	if(rkToUser.kCharGuid == BM::GUID::NullData())
	{//상대편 사용자가 없다
		BM::Stream kPacket(PT_M_C_NFY_CHAT);//실패
		kPacket.Push((BYTE)CT_ERROR);
		kPacket.Push((BYTE)WCR_NotFoundGuid);
		g_kRealmUserMgr.Locked_SendToUser(rkFromUser.kMemGuid, kPacket);
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	return SendChat_ManToMan(rkFromUser, rkToUser, rkContents, rkAddonPacket);
}

bool PgRealmChatMgr::RecvChat_Whisper_ByGuid(int const iChatMode, SContentsUser const &rkFromUser, SContentsUser const &rkToUser, std::wstring const &rkContents, BM::Stream const& rkAddonPacket)
{//귓말(Guid로)
	if(rkFromUser.kCharGuid == BM::GUID::NullData())
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	if(rkToUser.kCharGuid == BM::GUID::NullData())
	{//상대편 사용자가 없다
		BM::Stream kPacket(PT_M_C_NFY_CHAT);//실패
		kPacket.Push((BYTE)CT_ERROR);
		kPacket.Push((BYTE)WCR_NotFoundGuid);
		g_kRealmUserMgr.Locked_SendToUser(rkFromUser.kMemGuid, kPacket);
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	return SendChat_Whisper(iChatMode, rkFromUser, rkToUser, rkContents, rkAddonPacket);
}

//bool PgRealmChatMgr::RecvChat_Party(SContentsUser const &rkFromUser, BM::Stream const & rkPacket) const
//{//파티 대화
//	INFO_LOG(BM::LOG_LV5, _T("[%s] Don't call"), __FUNCTIONW__);
//	LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
//	return false;
//}
bool PgRealmChatMgr::RecvChat_Friend(SContentsUser const &rkFromUser, BM::Stream& rkPacket) const
{//친구
	//유효성 검사
	if(rkFromUser.kCharGuid == BM::GUID::NullData())
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
		return false;
	}

	std::wstring kContents;
	rkPacket.Pop(kContents);

	PgChatLogUtil::Log(ELogSub_Chat_Friend, rkFromUser, SContentsUser(), kContents);

	BM::Stream kPacket(PT_M_C_NFY_CHAT);//패킷 선 준비
	kPacket.Push((BYTE)CT_FRIEND);
	kPacket.Push(rkFromUser.kCharGuid);
	kPacket.Push(rkFromUser.Name());
	kPacket.Push(std::chrono::system_clock::now());
	kPacket.Push(kContents);
	kPacket.Push(rkPacket);

	BM::Stream kFPacket(PT_A_U_SEND_TOFRIEND_BYCHARGUID);
	kFPacket.Push(rkFromUser.kCharGuid);
	kFPacket.Push(kPacket.Data());
	SendToFriendMgr(kFPacket);

	return true;
}
bool PgRealmChatMgr::RecvChat_Guild(SContentsUser const &rkFromUser, BM::Stream& rkPacket) const
{	//길드

	std::wstring kContents;
	rkPacket.Pop(kContents);

	PgChatLogUtil::Log(ELogSub_Chat_Guild, rkFromUser, SContentsUser(), kContents);

	BM::Stream kPacket(PT_M_C_NFY_CHAT);
	size_t const iChatType_WRPos = kPacket.WrPos();
	kPacket.Push((BYTE)CT_GUILD);
	kPacket.Push(rkFromUser.kCharGuid);
	kPacket.Push(rkFromUser.Name());
	kPacket.Push(std::chrono::system_clock::now());
	kPacket.Push(kContents);
	kPacket.Push(rkPacket);

	BM::Stream kGPacket(PT_A_U_SEND_TOGUILD_BYCHARGUID);
	kGPacket.Push(rkFromUser.kCharGuid);
	kGPacket.Push(kPacket.Data());
	SendToGuildMgr(kGPacket);

	//길드 채팅 남기기
	static BYTE const iGuildLoginUserType = CT_GUILD_LOINGUSER;
	kPacket.ModifyData( iChatType_WRPos, &iGuildLoginUserType, sizeof(BYTE) );	//채팅타입 수정

	BM::Stream kNotiPacket;
	kNotiPacket.Push(E_NoticeMsg_GuildChatMsg);
	kNotiPacket.Push(rkFromUser.kGuildGuid);
	kNotiPacket.Push(kPacket.Data());
	SendToNotice(kNotiPacket);
	return true;
}

bool PgRealmChatMgr::RecvChat_Trade(SContentsUser const &rkFromUser, BM::Stream& rkPacket) const
{	// 거래

	std::wstring kContents;
	rkPacket.Pop(kContents);

	PgChatLogUtil::Log(ELogSub_Chat_Trade, rkFromUser, SContentsUser(), kContents);

	BM::Stream kPacket;
	kPacket.Push((BYTE)CT_TRADE);
	kPacket.Push(rkFromUser.kCharGuid);
	kPacket.Push(rkFromUser.Name());
	kPacket.Push(std::chrono::system_clock::now());
	kPacket.Push(kContents);
	kPacket.Push(rkPacket);

	BM::Stream kTPacket(PT_N_T_NFY_TRADE);
	kTPacket.Push(kPacket.Data());

	return ::SendToServerType(CEL::ST_CENTER, kTPacket);
}


//
bool PgRealmChatMgr::ProcessMsg(SEventMessage *pkMsg)
{
	//Lock은 각각 m_kUserHashByCharGuid에 접근하는 함수에서만 걸도록 한다.

	bool bRet = false;
	PACKET_ID_TYPE wType = 0;
	pkMsg->Pop(wType);

	switch(wType)
	{
	case PT_C_M_REQ_CHAT_INPUTNOW_2ND:
		{
			SContentsUser kFromUser;
			BYTE cChatMode = 0;
			bool bPopup = false;

			pkMsg->Pop(cChatMode);
			kFromUser.ReadFromPacket(*pkMsg);
			pkMsg->Pop(bPopup);

			if(kFromUser.kCharGuid == BM::GUID::NullData())
			{
				//INFO_LOG(BM::LOG_LV3, _T("[%s]-[%d] from logout user"), __FUNCTIONW__, __LINE__);
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
				return false;
			}

			if( cChatMode == CT_GUILD )
			{
				BM::Stream Packet(PT_M_C_NFY_CHAT_INPUTNOW, kFromUser.kCharGuid);
				Packet.Push(cChatMode);
				Packet.Push(bPopup);

				BM::Stream GPacket(PT_A_U_SEND_TOGUILD_BYCHARGUID);
				GPacket.Push(kFromUser.kCharGuid);
				GPacket.Push(Packet.Data());

				SendToGuildMgr(GPacket);
				return true;
			}

			if(cChatMode == CT_PARTY)
			{
				INFO_LOG( BM::LOG_LV5, __FL__ << _T("[PT_C_M_REQ_CHAT_INPUTNOW-CT_PARTY] Don't Call") );
			}
			else
			{
				INFO_LOG( BM::LOG_LV0, __FL__ << _T("invalid chat intput now type") );
			}
		}break;
	case PT_C_M_REQ_CHAT_INPUTNOW://일반 채팅 입력도중 외에 전부
		{
			BM::GUID kCharGuid;
			BYTE cChatMode = 0;
			bool bPopup = false;

			pkMsg->Pop(kCharGuid);
			pkMsg->Pop(cChatMode);
			pkMsg->Pop(bPopup);

			{//패킷스콥
				BM::Stream kDPacket(PT_C_M_REQ_CHAT_INPUTNOW_2ND);
				kDPacket.Push(cChatMode);
				::WritePlayerInfoToPacket_ByGuid(kCharGuid, false, kDPacket);
				kDPacket.Push(bPopup);
				SendToRealmChatMgr(kDPacket);
			}
		}break;
	case PT_C_M_REQ_CHAT_2ND:
		{
			SContentsUser kFromUser;
			SContentsUser kToUser;
			BYTE cChatMode = 0;
			pkMsg->Pop(cChatMode);
			kFromUser.ReadFromPacket(*pkMsg);

			switch(cChatMode)
			{
			case CT_WHISPER_BYNAME:
			case CT_WHISPER_BYGUID:
				{
					std::wstring wstrChat;
					DWORD dwColor;
					pkMsg->Pop(wstrChat);
					pkMsg->Pop(dwColor);

					BM::Stream kAddon;
					kAddon.Push(wstrChat);
					kAddon.Push(dwColor);

					kToUser.ReadFromPacket(*pkMsg);
					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kAddon.Push(*pkTempPacket);
						bRet = RecvChat_Whisper_ByGuid(cChatMode, kFromUser, kToUser, wstrChat, kAddon);
					}
				}break;
			case CT_FRIEND:
				{
					bRet = RecvChat_Friend(kFromUser, *pkMsg);
				}break;
			case CT_GUILD:
				{
					bRet = RecvChat_Guild(kFromUser, *pkMsg);
				}break;
			case CT_TRADE:
				{
					bRet = RecvChat_Trade(kFromUser, *pkMsg);
				}break;
			case CT_MANTOMAN:
				{
					std::wstring	wstrChat;
					DWORD			dwColor = 0;
					BYTE			RelationType = 0;

					pkMsg->Pop(RelationType);
					pkMsg->Pop(wstrChat);
					pkMsg->Pop(dwColor);

					BM::Stream kAddon;
					kAddon.Push(RelationType);
					kAddon.Push(wstrChat);
					kAddon.Push(dwColor);

					kToUser.ReadFromPacket(*pkMsg);
					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kAddon.Push(*pkTempPacket);
						bRet = RecvChat_ManToMan(kFromUser, kToUser, wstrChat ,kAddon);
					}
				}break;
			default:
				{
					VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << _T("invalid chat command[") << cChatMode << _T("]") );
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
				}break;
			}
		}break;
	case PT_C_M_REQ_CHAT://일반 채팅 외에 전부
		{
			BM::GUID kCharGuid;
			BYTE cChatMode = 0;
			std::wstring wstrChat;

			pkMsg->Pop(kCharGuid);
			pkMsg->Pop(cChatMode);

			BM::Stream kCPacket(PT_C_M_REQ_CHAT_2ND);
			kCPacket.Push(cChatMode);
			::WritePlayerInfoToPacket_ByGuid(kCharGuid, false, kCPacket);
		
			switch(cChatMode)
			{
			case CT_FRIEND:
			case CT_GUILD:
			case CT_TRADE:
				{
					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kCPacket.Push( *pkTempPacket );
						::SendToRealmChatMgr(kCPacket);
					}
				}break;
			case CT_WHISPER_BYNAME:
				{
					std::wstring	wstrChat;
					DWORD			dwColor = 0;
					std::wstring	kToName;

					pkMsg->Pop(wstrChat);
					pkMsg->Pop(dwColor);
					pkMsg->Pop(kToName);

					kCPacket.Push(wstrChat);
					kCPacket.Push(dwColor);

					WritePlayerInfoToPacket_ByName( kToName, kCPacket);

					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kCPacket.Push(*pkTempPacket);
						SendToRealmChatMgr(kCPacket);
					}
				}break;
			case CT_WHISPER_BYGUID:
				{
					std::wstring	wstrChat;
					DWORD			dwColor = 0;
					BM::GUID		kToGuid;

					pkMsg->Pop(wstrChat);
					pkMsg->Pop(dwColor);
					pkMsg->Pop(kToGuid);

					kCPacket.Push(wstrChat);
					kCPacket.Push(dwColor);					

					WritePlayerInfoToPacket_ByGuid(kToGuid, false, kCPacket);
					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kCPacket.Push(*pkTempPacket);
						SendToRealmChatMgr(kCPacket);
					}
				}break;
			case CT_MANTOMAN:
				{
					std::wstring	wstrChat;
					DWORD			dwColor = 0;

					BYTE			NameType = 0;
					BYTE			RelationType = 0;
					BM::GUID		kToGuid;
					std::wstring	wstrToName;

					pkMsg->Pop(wstrChat);
					pkMsg->Pop(dwColor);

					pkMsg->Pop(NameType);
					if( !NameType )
					{
						pkMsg->Pop(kToGuid);
					}
					else
					{
						pkMsg->Pop(wstrToName);
					}
					pkMsg->Pop(RelationType);
					

					kCPacket.Push(RelationType);
					kCPacket.Push(wstrChat);
					kCPacket.Push(dwColor);

					if( !NameType )
					{
						WritePlayerInfoToPacket_ByGuid(kToGuid, false, kCPacket);
					}
					else
					{
						WritePlayerInfoToPacket_ByName(wstrToName, kCPacket);
					}
					BM::Stream *pkTempPacket = dynamic_cast<BM::Stream *>(pkMsg);
					if( pkTempPacket )
					{
						kCPacket.Push(*pkTempPacket);
						SendToRealmChatMgr(kCPacket);
					}
				}break;
			default:
				{
					INFO_LOG( BM::LOG_LV5, __FL__ << _T("unknown ChatMode [") << cChatMode << _T("]") );
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
				}break;
			}
		}break;
	default:
		{
			INFO_LOG( BM::LOG_LV0, __FL__ << _T("Unhandled packet [") << pkMsg->SecType() << _T("]") );
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
			return false;
		}
	}
	return true;
}