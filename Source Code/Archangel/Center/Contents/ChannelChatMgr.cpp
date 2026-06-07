#include "StdAfx.h"

#include "Collins/Log.h"
#include "Variant/PgLogUtil.h"
#include "Global.h"
#include "PgTask_Contents.h"
#include "Lohengrin/PacketStructLog.h"
#include "ChannelChatMgr.h"
//#include <chrono>

PgChannelChatMgr::PgChannelChatMgr(void)
{
}

PgChannelChatMgr::~PgChannelChatMgr(void)
{
}

bool PgChannelChatMgr::RecvChat_Party(SContentsUser const &rkFromUser, BM::Stream &rkPacket)
{//친구대화

	std::wstring kContents;
	rkPacket.Pop(kContents);

	BM::Stream kPacket(PT_M_C_NFY_CHAT);//패킷 선 준비
	kPacket.Push((BYTE)CT_PARTY);
	kPacket.Push(rkFromUser.kCharGuid);
	kPacket.Push(rkFromUser.Name());
	//kPacket.Push(std::chrono::system_clock::now());
	kPacket.Push(kContents);
	kPacket.Push(rkPacket);
	
	PgChatLogUtil::Log(ELogSub_Chat_Party, rkFromUser, SContentsUser(), kContents);

	BM::Stream kSendPacket(PT_A_U_SEND_TOPARTY_BYCHARGUID);
	kSendPacket.Push(rkFromUser.kCharGuid);
	kSendPacket.Push(kPacket.Data());
	SendToGlobalPartyMgr(kSendPacket);
	return true;
}


//
bool PgChannelChatMgr::ProcessMsg(SEventMessage *pkMsg)
{
	//Lock은 각각 m_kUserHashByCharGuid에 접근하는 함수에서만 걸도록 한다.

	PACKET_ID_TYPE wType;
	pkMsg->Pop(wType);
	bool bRet = false;
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

			if(cChatMode == CT_PARTY)
			{
				BM::Stream kPacket(PT_M_C_NFY_CHAT_INPUTNOW, kFromUser.kCharGuid);
				kPacket.Push(cChatMode);
				kPacket.Push(bPopup);

				BM::Stream kChatPacket(PT_A_U_SEND_TOPARTY_BYCHARGUID);
				kChatPacket.Push(kFromUser.kCharGuid);
				kChatPacket.Push(kPacket.Data());
				SendToGlobalPartyMgr(kChatPacket);
			}
			else
			{
				INFO_LOG(BM::LOG_LV0, __FL__ << _T(" invalid chat intput now type=") << cChatMode);
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Not Matched ChatMode"));
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
			case CT_PARTY:
				{
					bRet = RecvChat_Party(kFromUser, *pkMsg);
				}break;
			default:
				{
					INFO_LOG(BM::LOG_LV0, __FL__ << _T("invalid ChatMode=") << cChatMode);
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
				}break;
			}
		}break;
	case PT_C_M_REQ_CHAT://일반 채팅 외에 전부
		{
			BM::GUID kCharGuid;
			BYTE cChatMode = 0;

			pkMsg->Pop(kCharGuid);
			pkMsg->Pop(cChatMode);

			BM::Stream kNPacket( PT_C_M_REQ_CHAT_2ND, cChatMode );
			::WritePlayerInfoToPacket_ByGuid(kCharGuid, false, kNPacket);
		
			switch(cChatMode)
			{
			case CT_PARTY:
				{
					kNPacket.Push(*(BM::Stream*)pkMsg);
					SendToChannelChatMgr(kNPacket);
				}break;
			default:
				{
					INFO_LOG(BM::LOG_LV5, __FL__ << _T("Invalid ChatMode=") << cChatMode);
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
				};
			}
		}break;
	default:
		{
			VERIFY_INFO_LOG(false, BM::LOG_LV3, __FL__ << _T("invalid packet type=") << pkMsg->SecType());
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Warning!! Invalid CaseType"));
			return false;
		}
	}
	return true;
}