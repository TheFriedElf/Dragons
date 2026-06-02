#ifndef FREEDOM_DRAGONICA_INPUT_PGREMOTEMANAGER_H
#define FREEDOM_DRAGONICA_INPUT_PGREMOTEMANAGER_H

#include "PgIInputManager.h"
#include "PgIXmlObject.h"

class PgRemoteManager :
	public PgIInputManager,
	public PgIXmlObject
{
	typedef std::list<BM::Stream> PacketContainer;
	typedef std::map<unsigned int, std::string> ActionContainer;

public:
	PgRemoteManager(void);
	virtual ~PgRemoteManager(void);

	//! PgRemoteManager를 업데이트 한다.
	bool UpdateInput();

	//! 패킷 풀에 등록한다.
	void AddPacket(BM::Stream &rkPacket);
	void EnableSyncEntireTime(bool bSyncEntireTime);
	bool PreFetchPacket(BM::Stream &rkPacket);

	//! 패킷 처리
	void ProcessPacket(WORD const wPacketType, BM::Stream &rkPacket);

	//! 마지막 위치 보냈던 시간을 기록.
	void SetLastSendPositionTime();

	//! XML 노드를 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

protected:
	void SyncEntireTime(DWORD dwSyncTerm);

private:
	bool m_bSyncEntireTime;
	DWORD m_dwLastSyncTime;
	CLASS_DECLARATION_S(DWORD, LastSendPositionTime);

	PacketContainer m_kPacketPool;
	ActionContainer m_kActionContainer;
};


extern PgRemoteManager *g_pkRemoteManager;

#endif // FREEDOM_DRAGONICA_INPUT_PGREMOTEMANAGER_H