#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgNetwork.h"
#include "PgRemoteManager.h"
#include "lwUI.h"
#include "PgAppProtect.h"
#include "PgWebLogin.h"
#include "PgPilotMan.h"
#include "PgCmdlineParse.h"

PgClientNetwork g_kNetwork;

Loki::Mutex g_NetMutex;
extern bool g_bDisconnectFlush;
/*
Loki::Mutex kTempLock;
typedef std::list< BM::Stream > CONT_TEMP_QUEUE;
CONT_TEMP_QUEUE g_kTempQueue;
*/

void CALLBACK OnTimer5S(DWORD dwUserData)
{
	static DWORD dwTIme = 0;
	if(BM::TimeCheck(dwTIme, 5000))
	{
		g_kTotalObjMgr.ReleaseFlush();
	}

/*
	CONT_TEMP_QUEUE kRealTemp; 
{
	BM::CAutoMutex kLock(kTempLock);
	
	kRealTemp.swap(g_kTempQueue);
}
	CONT_TEMP_QUEUE::const_iterator temp_itor = kRealTemp.begin();
	while(temp_itor != kRealTemp.end())
	{
//		if(BM::Rand_Index(1000) < 50)
//		{//강제랙
//			Sleep(1500);
//		}
		g_kNetwork.Send_Real((*temp_itor));
		++temp_itor;
	}
*/
}

void CALLBACK PgClientNetwork::OnRegist(const CEL::SRegistResult &rkArg)
{
	if( rkArg.iRet != CEL::CRV_SUCCESS)
	{
		PG_ASSERT_LOG( 0 );
	}
	
	if( rkArg.guidOrder == g_kNetwork.m_guidLoginConnectorRegist )
	{
		g_kNetwork.m_kLoginSession.WorkerGuid(rkArg.guidObj);
	}
	
	if( rkArg.guidOrder == g_kNetwork.m_guidSwitchConnectorRegist )
	{
		g_kNetwork.m_kSwitchSession.WorkerGuid(rkArg.guidObj);
	}
	
	if( CEL::RT_FINAL_SIGNAL == rkArg.eType )
	{//추가 등록 완료
		//INFO_LOG( BM::LOG_LV1, _T("[%s]-[%d] 등록 대기 목록 모두 완료"), _T(__FUNCTION__), __LINE__ );
		g_kCoreCenter.SvcStart();
//		//INFO_LOG( BM::LOG_LV1, _T("[%s]-[%d] 서비스 스타트"), _T(__FUNCTION__), __LINE__ );
	}
}

bool PgClientNetwork::Init()
{
	g_kCoreCenter;//인스턴스 생성.
	g_kLogWorker.StartSvc(1);

	ConnectionState(EConneCT_NONE);

	CEL::INIT_CENTER_DESC CenterInit;

	CenterInit.eOT = BM::OUTPUT_JUST_TRACE;
	CenterInit.pOnRegist = PgClientNetwork::OnRegist;
	CenterInit.bIsUseDBWorker = false;
	CenterInit.dwProactorThreadCount = 1;

	if(!g_kCoreCenter.Init(CenterInit))
	{
		PgError("CEL:CoreCenter Init Failed");
		return false;
	}
{
	CEL::REGIST_TIMER_DESC kInit;
	kInit.pFunc			= OnTimer5S;
	kInit.dwInterval	= 2500;
	kInit.dwThreadCount = 1;

	g_kCoreCenter.Regist(CEL::RT_TIMER_FUNC, &kInit);//등록후 바로 불려.
}
//	그니까. OnRegist 할때 이미 락 잡히거나 하는거지... 오더의 GUID를 외부에서 넣어 주는게 옳다고 본다.

	CEL::INIT_CORE_DESC kInit_L;
	kInit_L.OnSessionOpen	= PgClientNetwork::OnConnectFromLogin;
	kInit_L.OnDisconnect	= PgClientNetwork::OnDisconnectFromLogin;
	kInit_L.OnRecv			= PgClientNetwork::OnRecvFromLogin;
	kInit_L.dwThreadCount = 1;
	kInit_L.bIsImmidiateActivate = true;

	m_guidLoginConnectorRegist = kInit_L.kOrderGuid;
	g_kCoreCenter.Regist(CEL::RT_CONNECTOR, &kInit_L);

	CEL::INIT_CORE_DESC kInit_S;
	kInit_S.OnSessionOpen	= PgClientNetwork::OnConnectFromSwitch;
	kInit_S.OnDisconnect	= PgClientNetwork::OnDisconnectFromSwitch;
	kInit_S.OnRecv			= PgClientNetwork::OnRecvFromSwitch;
	kInit_S.dwThreadCount = 1;
	kInit_S.bIsImmidiateActivate = true;

	m_guidSwitchConnectorRegist = kInit_S.kOrderGuid;
	g_kCoreCenter.Regist(CEL::RT_CONNECTOR, &kInit_S);

	CEL::INIT_FINAL_SIGNAL kFinalInit;
	kFinalInit.kIdentity = 0;
	g_kCoreCenter.Regist(CEL::RT_FINAL_SIGNAL, &kFinalInit);
	return true;
}

bool PgClientNetwork::ConnectLoginServer(const CEL::ADDR_INFO &addr)
{
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] ConnectLoginServer\n");
	return g_kCoreCenter.Connect( addr, m_kLoginSession );
}

bool PgClientNetwork::ConnectSwitchServer(const CEL::ADDR_INFO &addr)
{
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] ConnectSwitchServer\n");
	return g_kCoreCenter.Connect( addr, m_kSwitchSession );
}

bool PgClientNetwork::DisConnectLoginServer()
{
	ClearAuthData();
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] DisConnectLoginServer\n");
	ConnectionState(ConnectionState() & (~EConnect_Login));
	return g_kCoreCenter.Terminate(m_kLoginSession);
}

bool PgClientNetwork::DisConnectSwitchServer()
{
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] DisConnectSwitchServer\n");
	ConnectionState(ConnectionState() & (~EConnect_Switch));
	return g_kCoreCenter.Terminate(m_kSwitchSession);
}

void CALLBACK PgClientNetwork::OnConnectFromLogin( CEL::CSession_Base *pkSession )
{
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnConnectFromLogin\n");
	BM::CAutoMutex lock(g_NetMutex);

	if (!pkSession->IsAlive())
	{
		lwCannotConnectLogin();
		g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() & (~EConnect_Login));
	}
	else
	{
		g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() | EConnect_Login);
	}
}

void CALLBACK PgClientNetwork::OnDisconnectFromLogin( CEL::CSession_Base *pkSession )
{//!  세션락
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnDisconnectFromLogin\n");
	BM::CAutoMutex lock(g_NetMutex);
	g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() & (~EConnect_Login));
	//std::cout<< "접속 종료 ["<< 0 <<  "]"<< pkSession << std::endl;
	//NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnDisconnectFromLogin end\n");

	g_kNetwork.SetFirstConnect(true);
	lwClearAutoLogin();

	if(g_pkRemoteManager)
	{
		BM::Stream kDisPacket( PT_C_C_SELF_DISCONNECT_SERVER, false );
		g_pkRemoteManager->AddPacket( kDisPacket );
	}
}

//유저 세션은 pkSession->pData 에다가 현재 정보를 셋팅 해 줄것이다. OnDisconnect 에서 delete 해주면 된다.
void CALLBACK PgClientNetwork::OnRecvFromLogin( CEL::CSession_Base *pkSession, BM::Stream * const pkPacket )
{
//	LockCheck(g_NetLog, Checker);
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgClientNetwork.RecvFromLogin"), g_pkApp->GetFrameCount()));
	char packetType[2]= {0,};
	if(pkPacket->Data().size() >= 2)
	{
		packetType[0] = pkPacket->Data().at(0);
		packetType[1] = pkPacket->Data().at(1);
	}
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnRecvFromLogin(%d,%d)\n", *((unsigned short*)packetType), g_pkApp->GetFrameCount());	

	switch(*((unsigned short*)packetType))
	{
	case PT_A_ENCRYPT_KEY:
		{
			WORD wType;
			pkPacket->Pop(wType);

			std::vector<char> kKey;
			pkPacket->Pop(kKey);
			
			if(S_OK == pkSession->VSetEncodeKey(kKey))
			{
				if(g_kNetwork.OnConnectFromLogin_sub(pkSession))
				{
					
					return;
				}
			}
			else
			{
				PG_ASSERT_LOG( _T("OnRecvFromLogin PT_A_ENCRYPT_KEY SetEncodeKey Failed"));
				pkSession->VTerminate();
			}
		}break;
	default:
		{
			g_pkRemoteManager->AddPacket( *pkPacket );
		}break;
	}
	return;
}


void CALLBACK PgClientNetwork::OnConnectFromSwitch( CEL::CSession_Base *pkSession )
{	//!  세션락
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnConnectFromSwitch\n");
	//INFO_LOG( BM::LOG_LV5, _T("스위치로 접속 성공! [%s:%d]"), pkSession->Addr().wstrIP.c_str(), pkSession->Addr().wPort);
	BM::CAutoMutex lock(g_NetMutex);

	if (pkSession->IsAlive())
	{
		g_kNetwork.SwitchSession(pkSession->SessionKey());
		g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() | EConnect_Switch);
	}
	else
	{
		g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() & (~EConnect_Switch));
		g_kNetwork.SetSwitchAuthKey( BM::GUID::NullData() );// 키 클리어
	}
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnConnectFromSwitch end\n");
}

void CALLBACK PgClientNetwork::OnDisconnectFromSwitch( CEL::CSession_Base *pkSession )
{//!  세션락
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnDisconnectFromSwitch\n");

	BM::CAutoMutex lock(g_NetMutex);

	PG_ASSERT_LOG(g_pkRemoteManager);
	if (g_pkRemoteManager)
	{
		BM::Stream kDisPacket( PT_C_C_SELF_DISCONNECT_SERVER, true );
		g_pkRemoteManager->AddPacket( kDisPacket );
	}
	else
	{
		_PgMessageBox( "DisconnectFromSwitch","RemoteManager is NULL" );
		g_bDisconnectFlush = true;
	}

	//	std::cout<< "접속 종료 ["<< 0 <<  "]"<< pkSession << std::endl;
	g_kNetwork.ConnectionState(g_kNetwork.ConnectionState() & (~EConnect_Switch));
}

//유저 세션은 pkSession->pData 에다가 현재 정보를 셋팅 해 줄것이다. OnDisconnect 에서 delete 해주면 된다.
void CALLBACK PgClientNetwork::OnRecvFromSwitch( CEL::CSession_Base *pkSession, BM::Stream * const pkPacket )
{
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgClientNetwork.RecvFromSwitch"), g_pkApp->GetFrameCount()));
	char packetType[2] = {0,};
	if(pkPacket->Data().size() >= 2)
	{
		packetType[0] = pkPacket->Data().at(0);
		packetType[1] = pkPacket->Data().at(1);
	}
	NILOG(PGLOG_NETWORK, "[PgClientNetwork] OnRecvFromSwitch(%d,%d)\n", *((unsigned short*)packetType), g_pkApp->GetFrameCount());
	
//	SEventMessage kEventMessage;
//	kEventMessage.BM::Stream::operator = (*pkPacket);

	WORD const wPacketType = *((WORD*)packetType);

	switch(wPacketType)
	{
	case PT_A_ENCRYPT_KEY:
		{
			WORD wType;
			pkPacket->Pop(wType);

			std::vector<char> kKey;
			pkPacket->Pop(kKey);
			
			if(S_OK != pkSession->VSetEncodeKey(kKey))
			{
				PG_ASSERT_LOG( _T("OnRecvFromSwitch PT_A_ENCRYPT_KEY SetEncodeKey Failed"));
				pkSession->VTerminate();
			}
			else
			{
				VM_START
				STryAccessSwitch kTAS;
				kTAS.SetVersion( PACKET_VERSION_C, PACKET_VERSION_S );
				kTAS.SetID( g_kNetwork.LoginID() );
				kTAS.SetPW( g_kNetwork.LoginPW());
				kTAS.guidSwitchKey = g_kNetwork.GetSwitchAuthKey();

				BM::Stream Packet(PT_C_S_TRY_ACCESS_SWITCH);
				kTAS.WriteToPacket(Packet);
				
				g_kAppProtect.OnLogin(g_kNetwork.LoginID());
				pkSession->VSend( Packet );
				
				g_kNetwork.SetSwitchAuthKey( BM::GUID::NullData() );// 키 클리어

				VM_END
			}
		}break;
	case PT_S_C_REQ_GAME_GUARD_CHECK:
		{
			WORD wType = 0;
			
			pkPacket->Pop( wType );

			g_kAppProtect.CheckState(*pkPacket);
		}break;
	default:
		{
			g_pkRemoteManager->AddPacket( *pkPacket );
		}break;
	}
}
/*
bool PgClientNetwork::Send_Fake(const BM::Stream &Packet)
{
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgClientNetwork.Send"), g_pkApp->GetFrameCount()));
	PACKET_ID_TYPE kType = 0;

	if ( IsAutoLogin() )
	{// AutoLogin중에는 g_kNetWork.Send()를 사용하면 안된다.
		BM::Stream *pkPacket = const_cast<BM::Stream*>(&Packet);
		pkPacket->Pop(kType);

		NILOG(PGLOG_NETWORK, "[PgClientNetwork] SendPacket(%u) Failed(%d) -> Now ChannelChanging\n", kType, g_pkApp->GetFrameCount() );
		return false;
	}

	BM::CAutoMutex kLock(kTempLock);

	g_kTempQueue.push_back(Packet);
	return true;
}
*/
bool PgClientNetwork::Send(const BM::Stream &Packet)
{
	PG_STAT(PgStatTimerF timerA(g_kRemoteInputStatGroup.GetStatInfo("PgClientNetwork.Send"), g_pkApp->GetFrameCount()));
	PACKET_ID_TYPE kType = 0;

	if ( IsAutoLogin() )
	{// AutoLogin중에는 NETWORK_SEND()를 사용하면 안된다.
		BM::Stream *pkPacket = const_cast<BM::Stream*>(&Packet);
		pkPacket->Pop(kType);

		NILOG(PGLOG_NETWORK, "[PgClientNetwork] SendPacket(%u) Failed(%d) -> Now ChannelChanging\n", kType, g_pkApp->GetFrameCount() );
		return false;
	}

	bool const ret = (ConnectionState() & EConnect_Switch) ?
		g_kCoreCenter.Send(SwitchSession(), Packet) : g_kCoreCenter.Send(LoginSession(), Packet);

	
	{
		BM::Stream *pkPacket = const_cast<BM::Stream*>(&Packet);
		pkPacket->Pop(kType);
		if (ret)
		{
			NILOG(PGLOG_NETWORK, "[PgClientNetwork] SendPacket(%u) success(%d)\n", kType, g_pkApp->GetFrameCount());
		}
		else
		{
			NILOG(PGLOG_NETWORK, "[PgClientNetwork] SendPacket(%u) failed(%d)\n", kType, g_pkApp->GetFrameCount());
		}
	}
	
	return ret;
}

bool PgClientNetwork::SendToLogin(const BM::Stream &Packet)
{
	return g_kCoreCenter.Send(LoginSession(), Packet);
}

bool PgClientNetwork::SendToSwitch(const BM::Stream &Packet)
{
	return g_kCoreCenter.Send(SwitchSession(), Packet);
}

bool PgClientNetwork::ClearAuthData()
{
	if ( !IsAutoLogin() )
	{
		NetState(NS_NONE);
		NowRealmNo(0);
		NowChannelNo(0);
		LoginID(_T(""));
		LoginPW(_T(""));
		ConnectionState(EConneCT_NONE);
		oldSelectCharacterSec(0i64);
		m_kSelectCharGuid.Clear();
		return true;
	}
	return false;
}

bool PgClientNetwork::OnConnectFromLogin_sub(CEL::CSession_Base *pkSession)
{
	VM_START
	BM::Stream kPacket;

	if( IsAutoLogin() )
	{
		kPacket.Push( PT_C_L_AUTO_AUTH );
		kPacket.Push( GetSwitchAuthKey() );
	}
	else if( g_kCmdLineParse.IsUseForceAccount() )
	{
		g_kCmdLineParse.MakeLoginPacket(kPacket);
		pkSession->VSend( kPacket );
	}
	else
	{
		SClientTryLogin kCTL;
		kCTL.SetID( LoginID() );
		kCTL.SetPW( LoginPW() );
		kCTL.SetVersion( PACKET_VERSION_C, PACKET_VERSION_S );
		kCTL.PatchVersion(g_pkApp->PatchVer().iVersion);
		kPacket.Push(PT_C_L_TRY_AUTH);
		kCTL.WriteToPacket(kPacket);

		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_KOREA:
			{
				if( g_pkApp->UseGSM() && g_pkApp->UseWebLinkage() )
				{
					kPacket.Push(g_kWebLogin.SessKey());
				}
			}break;	
		default:
			{
			}break;
		}

		kPacket.Push( AddonMsg() );
	}
	
	pkSession->VSend( kPacket );
	VM_END
	return true;
}

void PgClientNetwork::RecvAutoLogin(BM::Stream& kPacket)
{
	BM::GUID guidOrder;
	kPacket.Pop( guidOrder );
	SetSwitchAuthKey( guidOrder );

	NetState(NS_NONE);
	ConnectionState(EConnect_AutoLogin);

	if ( TryChannelNo() == NowChannelNo() )
	{
		m_kSelectCharGuid.Clear();
	}
	
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkMyPlayer )
	{
		__int64 i64Value = g_kEventView.GetLocalSecTime(CGameTime::SECOND) - (pkMyPlayer->GetSelectCharacterSec() - g_kNetwork.oldSelectCharacterSec());
		if( i64Value > 0 )
		{			
			g_kNetwork.oldSelectCharacterSec(i64Value);
		}			
		else
		{
			g_kNetwork.oldSelectCharacterSec(0);
		}
	}
	else
	{
		g_kNetwork.oldSelectCharacterSec(0);
	}

	// 혹시 모르니까 1초뒤에 채널 변경 시도
//	Sleep(1000);
//	lua_tinker::call< void, short >( "UI_ChannelChanging", nChannelNo );
//	lwTryLogin( lwWString(wstrID), lwWString(wstrPW), false, bSaveID() );
}
