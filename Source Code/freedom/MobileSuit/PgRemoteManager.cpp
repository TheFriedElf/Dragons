#include "StdAfx.h"
#include <Mmsystem.h>
#include "PgMobileSuit.h"
#include "PgRemoteManager.h"
#include "lwPacket.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgStat.h"
#include "PgNetwork.h"
#include "PgRenderMan.h"
#include "BM/FileSupport.h"
#include "variant/PgEventView.h"
#include "variant/pgcouponeventview.h"
#include "HandlePacket.h"
#include "PgWorld.h"
#include "PgChatMgrClient.h"
#include "lwEventView.h"
#include "PgCmdlineParse.h"
#include "PgErrorCodeTable.h"
#include "PgWebLogin.h"

extern	Loki::Mutex g_NetMutex;
extern bool g_bDisconnectFlush;
extern std::wstring g_wstrDisconnectMessage;
extern DWORD g_dwLastTime;

PgRemoteManager *g_pkRemoteManager = 0;
//extern PgRenderMan *g_pkRenderMan;

PgRemoteManager::PgRemoteManager() :
	m_bSyncEntireTime(false),
	m_dwLastSyncTime(0)
{
}

PgRemoteManager::~PgRemoteManager()
{
}

//  <SLOT KEY="112352" SCRIPT="Net_StateChange" /> <!-- PT_M_C_NFY_STATE_CHANGE : 플레이어의 상태가 변경되었다 -->
//  <SLOT KEY="112419" SCRIPT="Net_StateChange2" /> <!-- PT_M_C_NFY_STATE_CHANGE2  Unit State Changed -->

void PgRemoteManager::ProcessPacket(WORD const wPacketType, BM::Stream &rkPacket)
{
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgRemoteManager.ProcessPacket"), g_pkApp->GetFrameCount()));
	switch(wPacketType)
	{
	case PT_C_C_SELF_DISCONNECT_SERVER:
		{
			// 웹연동에서 LoginDlg에 대해 Activate가 불리면 자동으로 로그인 시도를 한다.
			// IsDisconnectServer를 셋팅해줌으로써 그 상황을 막아야한다!
			g_kWebLogin.IsDisconnectServer(true);

			rkPacket.Pop( g_bDisconnectFlush );

			if( !g_bDisconnectFlush 
				&&	BM::GUID::IsNull( g_kNetwork.GetSwitchAuthKey() ) )				
			{
				XUIMgr.Close( _T("FRM_DEFAULT_REALM") );
				XUIMgr.Close( _T("FRM_MANY_REALM") );
				XUIMgr.Close( _T("FRM_CHANNEL") );
				XUIMgr.Close( _T("FRM_SERVER_BG") );
				XUIMgr.Close( _T("FRM_SV_CH_SELECT_TITLE") );
				XUIMgr.Close( _T("FRM_TRADE_CHANNEL_TIP") );
				XUIMgr.Close( _T("FRM_DEFAULT_CHANNEL") );
				XUIMgr.Activate( _T("LoginBg") );

				if( !g_pkApp->UseWebLinkage() )
				{
					XUIMgr.Activate( _T("LoginDlg") );
				}
			}
		}break;
	case PT_M_C_NFY_ENTIRE_SYNC_TIME:
		{
			DWORD dwSyncTime;
			rkPacket.Pop(dwSyncTime);
			PgActor::SetAverageLatency((DWORD)((BM::GetTime32() - PgActor::GetLastSentTime()) / 2));

			// Packet Latency까지 고려한다.
			PgActor::SyncEntireTime(dwSyncTime + PgActor::GetAverageLatency());
			//WriteToConsole("[PgRemoteManager] Average Latency : (%u)\n", PgActor::GetAverageLatency());
		}break;
	default:
		{
			if(!HandlePacket(wPacketType, rkPacket))
			{
				PgInput kInput;
				kInput.SetUKey(wPacketType + PgInput::UR_REMOTE_BEGIN);
				kInput.SetPacket(&rkPacket);
				ActionContainer::iterator actitr = m_kActionContainer.find(kInput.GetUKey());
				if(actitr != m_kActionContainer.end())
				{
					NILOG(PGLOG_NETWORK, "[PgRemoteManager] %s Action from Network start\n", actitr->second.c_str());
					if(!lua_tinker::call<bool, lwPacket>(actitr->second.c_str(), lwPacket(&rkPacket)))
					{
						TransferToObserver(kInput);
					}
				}
				else
				{
					TransferToObserver(kInput);
				}
			}
		}break;
	}
}

bool PgRemoteManager::UpdateInput()
{
//	LockCheck(g_NetLog, Checker);
	//NILOG(PGLOG_NETWORK, "[PgRemoteManager] UpdateInput\n");
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgRemoteManager.UpdateInput"), g_pkApp->GetFrameCount()));
	
	PacketContainer kPacketBuffer;

	{//
		BM::CAutoMutex lock(g_NetMutex);
		kPacketBuffer.swap(m_kPacketPool);
	}

#ifndef EXTERNAL_RELEASE
	static float fAveragePacketCount = 0.0f;
	static unsigned int iPacketCount = 0;
	static unsigned int iFrameCount = 0;

	iPacketCount += kPacketBuffer.size();
	iFrameCount++;
	fAveragePacketCount = (float)iPacketCount / (float)iFrameCount;

	if (iFrameCount % 2000 == 0)
	{
		NILOG(PGLOG_NETWORK, "[PgRemoteManager] UpdateInput statistics, %f %d\n", fAveragePacketCount, iPacketCount);
	}
#endif

	if (kPacketBuffer.size() > 0)
	{
		NILOG(PGLOG_NETWORK, "[PgRemoteManager] Process %d Packet at %d frame\n", kPacketBuffer.size(), g_pkApp->GetFrameCount());
	}

	PacketContainer::iterator itr = kPacketBuffer.begin();
	while(itr != kPacketBuffer.end() )
	{
		BM::Stream &rkPacket = *(itr);

//		BM::MemToFile(UNI("PacketDump.bin"), rkPacket.Data());

		WORD wPacketType = 0;
		rkPacket.Pop(wPacketType);
		NILOG(PGLOG_NETWORK, "[PgRemoteManager] OnProcessPacketStart(%d,%d)\n", wPacketType, g_pkApp->GetFrameCount());

		ProcessPacket(wPacketType, rkPacket);

		g_kRenderMan.InvalidateSceneContainer();
		
		itr++;
		NILOG(PGLOG_NETWORK, "[PgRemoteManager] OnProcessPacketEnd(%d,%d)\n", wPacketType, g_pkApp->GetFrameCount());
	}	

	if(m_bSyncEntireTime)
	{
		// 10초마다 서버와 틱을 맞춘다.
		SyncEntireTime(10000);
	}

	//NILOG(PGLOG_NETWORK, "[PgRemoteManager] UpdateInput end\n");
	return true;
}

void PgRemoteManager::EnableSyncEntireTime(bool bEnable)
{
	m_bSyncEntireTime = bEnable;
}

void PgRemoteManager::SyncEntireTime(DWORD dwSyncTerm)
{
	DWORD dwNow = BM::GetTime32();
	if(m_dwLastSyncTime != 0 && dwNow - m_dwLastSyncTime < dwSyncTerm)
	{
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_ENTIRE_SYNC_TIME);
	m_dwLastSyncTime = dwNow;
	PgActor::SetLastSentTime(dwNow);
	NETWORK_SEND(kPacket)
}

void PgRemoteManager::AddPacket(BM::Stream &kPacket)
{
	if ( !PreFetchPacket(kPacket) )
	{
		PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgRemoteManager.AddPacket"), g_pkApp->GetFrameCount()));
		BM::CAutoMutex lock(g_NetMutex);
		m_kPacketPool.push_back(kPacket);
	}
}

bool PgRemoteManager::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	PG_ASSERT_LOG(strcmp(pkNode->Value(), "REMOTEINPUT") == 0);

	const TiXmlElement *pkChildElement = pkNode->FirstChildElement();
	while(pkChildElement)
	{
		if(strcmp(pkChildElement->Value(), "SLOT") == 0)
		{
			unsigned int uiUKey = 0;
			char const *pcScript = 0;

			const TiXmlAttribute *pkAttr = pkChildElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "KEY") == 0)
				{
					uiUKey = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "SCRIPT") == 0)
				{
					pcScript = pcAttrValue;
				}
				else
				{
					PgXmlError1(pkChildElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}
				
				pkAttr = pkAttr->Next();
			}

			if(uiUKey && pcScript)
			{
				m_kActionContainer.insert(std::make_pair(uiUKey, std::string(pcScript)));
			}
			else
			{
				PgXmlError(pkChildElement, "Incorrect Config Set");
			}
		}

		pkChildElement = pkChildElement->NextSiblingElement();
	}

	return true;
}

void PgRemoteManager::SetLastSendPositionTime()
{
	LastSendPositionTime(BM::GetTime32());
}

bool PgRemoteManager::PreFetchPacket(BM::Stream &rkPacket)
{
	PACKET_ID_TYPE usType = 0;
	rkPacket.Pop(usType);
	switch (usType)
	{
	case PT_A_S_NFY_USER_DISCONNECT:
		{
			unsigned long iReason = 0;
			BYTE byReason = 0;
			rkPacket.Pop( iReason );
			rkPacket.Pop( byReason ); // NC 사유 코드

			if( byReason )
			{
				g_wstrDisconnectMessage = ECTW(byReason-E_TLR_NC_ERROR_BASE);				
			}
			else if( iReason )
			{
				g_wstrDisconnectMessage = TTW(iReason);
			}
			else
			{
				g_wstrDisconnectMessage.clear();
			}
			return true;
		}break;
	case PT_M_C_SYNC_GAMETIME:
		{
			g_kEventView.ReadFromPacket(rkPacket);
			//g_dwLastTime = g_kEventView.GetServerElapsedTime();
			//_PgOutputDebugString("GameElapsedTime[%u]\n", g_dwLastTime );
			
			return true;
		}break;
	case PT_M_C_NFY_MAPLOADED:
		{
			g_kEventView.ReadFromPacket(rkPacket);
			g_dwLastTime = g_kEventView.GetServerElapsedTime();
			_PgOutputDebugString("GameElapsedTime[%u]\n", g_dwLastTime );

			BM::Stream kTempPacket( PT_M_C_NFY_MAPLOADED, rkPacket );
			rkPacket.Swap( kTempPacket );
		}break;
	case PT_M_C_ANS_SYNCTIME_CHECK:
		{
			SYSTEMTIME kNowTime;
			g_kEventView.GetLocalTime( &kNowTime );

			__int64 i64ServerTime = 0i64;
			rkPacket.Pop( i64ServerTime );

			SYSTEMTIME kServerTime;
			CGameTime::SecTime2SystemTime( i64ServerTime, kServerTime, CGameTime::DEFAULT );

			std::wstring wstrTime;
			MakeStringForTime( wstrTime, kServerTime, true );
			wstrTime += L" -> ";
			MakeStringForTime( wstrTime, kNowTime, true );

			Notice_Show( wstrTime, EL_Normal ); // 약간 위험성 있음.. 테스트 패킷인긴 한데..
			return true;
		}break;
	case PT_S_C_REQ_PING:
		{
			Recv_PT_S_C_REQ_PING(&rkPacket);

			if(PgWorld::GetNowLoading())
			{//맵로딩중이면. fetch 되었다고 처리 한다.
				return true;
			}
		}break;
	}

	rkPacket.PosAdjust();
	return false;
}
