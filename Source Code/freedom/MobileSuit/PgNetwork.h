#ifndef FREEDOM_DRAGONICA_NETWORK_PGNETWORK_H
#define FREEDOM_DRAGONICA_NETWORK_PGNETWORK_H
#include "Themida/ThemidaSDk.h"
#include "Loki/Threads.h"
#include "BM/PgTask.h"
#include "lohengrin/packettype.h"

extern void NetReqMapMove( SGroundKey const &kGroundKey, short const nSpawnNo );
extern void Net_RecentMapMove();
extern void ViewSelectCharacterDisplay();
extern SGroundKey g_kNowGroundKey;

typedef enum ENetState
{
	NS_NONE = 0,
	
	NS_AUTH_COMPLETE,//인증이 되었음.

	NS_TRY_LOGIN,
}eNetState;

typedef enum : BYTE
{
	EConneCT_NONE = 0x00,
	EConnect_Login = 0x01,
	EConnect_Switch = 0x02,
	EConnect_AutoLogin = 0x04,
} EConnectState;

class PgClientNetwork
{
//	friend struct ::Loki::CreateStatic< PgClientNetwork >;
public:
	PgClientNetwork() : m_bIsFirstConnect(false)
	{
		ClearAuthData();
		NetState(NS_NONE);
	}
	~PgClientNetwork()
	{
	}

protected:
	typedef struct tagRegistState
	{
		tagRegistState()
		{
			bIsComplete = false;
		}

		BM::GUID guidOrder;
		bool bIsComplete;
	}SRegistState;
	typedef std::vector< SRegistState > REGIST_ARRAY;

public:
	bool Init();

	bool ConnectLoginServer(const CEL::ADDR_INFO &addr);
	bool ConnectSwitchServer(const CEL::ADDR_INFO &addr);
	bool DisConnectLoginServer();
	bool DisConnectSwitchServer();
	
	bool IsRegistAllComplete()const;

//	bool Send_Fake(const BM::Stream &Packet);
	bool Send(const BM::Stream &Packet);//스위치로 보내는것임
	bool SendToLogin(const BM::Stream &Packet);
	bool SendToSwitch(const BM::Stream &Packet);

	void SetSwitchAuthKey(BM::GUID const &guidKey) {m_SRR.guidSwitchKey = guidKey;}
	BM::GUID const &GetSwitchAuthKey()const{return m_SRR.guidSwitchKey;}

	bool ClearAuthData();

	void RecvAutoLogin(BM::Stream& kPacket);
	bool IsAutoLogin()const{return ConnectionState() & EConnect_AutoLogin;}
	void SetFirstConnect(const bool bFirstConnect)	{ m_bIsFirstConnect = bFirstConnect; }
	const bool IsFirstConnect() const				{ return m_bIsFirstConnect; }
protected:
	CLASS_DECLARATION_S(eNetState, NetState);
	CLASS_DECLARATION_S(short, NowRealmNo);//현재 채널 번호. 변할 수 있음.
	CLASS_DECLARATION_S(short, NowChannelNo);//현재 채널 번호. 변할 수 있음.
	
	CLASS_DECLARATION_S(short, TryRealmNo);//현재 채널 번호. 변할 수 있음.
	CLASS_DECLARATION_S(short, TryChannelNo);//현재 채널 번호. 변할 수 있음.
	
	CLASS_DECLARATION_S(std::wstring, LoginID);//
	CLASS_DECLARATION_S(std::wstring, LoginPW);//인증 끝나면 지울 것.
	CLASS_DECLARATION_S(std::wstring, AddonMsg);//인증 끝나면 지울 것.//넥슨패스포트 값.

	CLASS_DECLARATION_S(std::wstring, ServerIP);//인증 끝나면 지울 것.
	CLASS_DECLARATION_S(int, ServerPort);//인증 끝나면 지울 것.
	CLASS_DECLARATION_S(bool, bSaveID);

	CLASS_DECLARATION_S(BM::GUID,  SelectCharGuid); 

protected:
	static void CALLBACK OnRegist(const CEL::SRegistResult &rkArg);

	static void CALLBACK OnConnectFromLogin( CEL::CSession_Base *pkSession );
	static void CALLBACK OnDisconnectFromLogin( CEL::CSession_Base *pkSession );
	static void CALLBACK OnRecvFromLogin( CEL::CSession_Base *pkSession, BM::Stream * const pkPacket );

	static void CALLBACK OnConnectFromSwitch( CEL::CSession_Base *pkSession );
	static void CALLBACK OnDisconnectFromSwitch( CEL::CSession_Base *pkSession );
	static void CALLBACK OnRecvFromSwitch( CEL::CSession_Base *pkSession, BM::Stream * const pkPacket );

	bool OnConnectFromLogin_sub(CEL::CSession_Base *pkSession);

protected:
	
	SSwitchReserveResult m_SRR;
	CLASS_DECLARATION_S(CEL::SESSION_KEY, LoginSession); 
	CLASS_DECLARATION_S(CEL::SESSION_KEY, SwitchSession);

	BM::GUID m_guidLoginConnectorRegist;
	BM::GUID m_guidSwitchConnectorRegist;
	CLASS_DECLARATION_S(BYTE, ConnectionState);
	CLASS_DECLARATION_S(__int64, oldSelectCharacterSec);
	mutable Loki::Mutex m_kMutex;

private:
	bool m_bIsFirstConnect;	
};

extern PgClientNetwork g_kNetwork;

#define NETWORK_SEND(in_p)				{VM_START} bool const bSendRet = g_kNetwork.Send(in_p); {VM_END}
#define NETWORK_SEND_TO_LOGIN(in_p)		{VM_START} bool const bSendRet = g_kNetwork.SendToLogin(in_p); {VM_END}
#define NETWORK_SEND_TO_SWITCH(in_p)	{VM_START} bool const bSendRet = g_kNetwork.SendToSwitch(in_p); {VM_END}

extern Loki::Mutex g_NetMutex;

#endif // FREEDOM_DRAGONICA_NETWORK_PGNETWORK_H