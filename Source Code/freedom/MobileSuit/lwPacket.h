#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPACKET_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPACKET_H

#include "lwWString.h"
#include "lwGUID.h"
#include "lwInt64.h"
#include "lwSItem.h"
#include "lwDateTime.h"

class BM::Stream;
class lwPoint3;

class lwPacket
{
public:
	lwPacket(BM::Stream *m_pkPacket = NULL, bool bDeletePacket = false);
	lwPacket(lwPacket const& rhs);
	lwPacket const& operator=(lwPacket const& rhs);
	~lwPacket();

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	bool IsNil();
	bool Empty();

	char PopChar();
	BYTE PopByte();
	short PopShort();
	WORD PopWord();
	int PopInt();
	unsigned int PopUInt();
	size_t PopSize_t();
	float PopFloat();
	bool PopBool();
	//	PopMemoryAddr
	//	leesg213 2007-01-15
	//	szLen 만큼 메모리를 할당한 후, 패킷 내용으로 채우고, 그 주소값을 리턴한다.
	//	(주의!)리턴받은 주소의 메모리를 반드시 해제시켜줘야한다.
	unsigned long PopMemoryAddr(size_t const szLen);	
	char const *PopString();
	lwWString PopWStringBuffer(size_t const szLen);
	lwWString PopWString();
	char const *PopArray(size_t const szLen);
	lwGUID PopGuid();
	lwInt64 PopInt64();
	lwPoint3 PopPoint3();
	lwDateTime PopDateTime();

	void PushChar(char cData);
	void PushByte(BYTE cData);
	void PushBool(bool bData);
	void PushShort(short sData);
	void PushWord(WORD sData);
	void PushInt(int iData);
	void PushUInt(unsigned int uiData);
	void PushSize_t(size_t iData);
	void PushFloat(float fData);
	void PushString(char const *pcData);
	void PushWString(lwWString wstr);
	void PushArray(char const *pcData, size_t const szLen);
	void PushGuid(lwGUID kGuid);
	void PushGuidForString(char const *pcData);
	//void PushWString(char const *pcData);
	void PushInt64(lwInt64 kInt64);
	void PushPoint3(lwPoint3 kPoint3);
	void PushPacket(lwPacket kPacket);

	BM::Stream * operator()();
	
	void Release();
protected:
	BM::Stream *m_pkPacket;
	bool m_bDeleteMemory;
};

extern lwPacket lwNewPacket(unsigned short usPacketType);
#endif //FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPACKET_H