#include "StdAfx.h"
#include "BM/Stream.h"
#include "lwPacket.h"
#include "PgNetwork.h"
#include "FreedomPool.h"
#include "lwPoint3.h"

lwPacket lwNewPacket(unsigned short usPacketType)
{
	BM::Stream *pkPacket = new BM::Stream;
	
	if( usPacketType)
	{
		pkPacket->Push(usPacketType);
	}
	
	return lwPacket(pkPacket, true);
}

void lwDeletePacket(lwPacket kPacket)
{
	kPacket.Release();
}

bool lwNet_Send(lwPacket kPacket)
{
	if( kPacket.Empty() )
	{
		return false;
	}
	NETWORK_SEND(*(kPacket()))
	return bSendRet;
}

lwPacket::lwPacket(BM::Stream *pkPacket, bool bDeletePacket)
:	m_pkPacket(pkPacket), m_bDeleteMemory(bDeletePacket)
{
}

lwPacket::~lwPacket()
{
	if (m_bDeleteMemory)
	{
		Release();
	}
}

lwPacket::lwPacket(lwPacket const& rhs)
{
	*this = rhs;
}

lwPacket const& lwPacket::operator=(lwPacket const& rhs)
{
	m_pkPacket = rhs.m_pkPacket;
	m_bDeleteMemory = rhs.m_bDeleteMemory;
	if (true == rhs.m_bDeleteMemory)
	{
		lwPacket& kCastRhs = const_cast<lwPacket&>(rhs);
		kCastRhs.m_bDeleteMemory = false;
	}
	return (*this);
}

void lwPacket::Release()
{
	if (true == m_bDeleteMemory)
	{
		SAFE_DELETE(m_pkPacket);
		m_bDeleteMemory = false;
	}
}

bool lwPacket::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "NewPacket", &lwNewPacket);
	def(pkState, "DeletePacket", &lwDeletePacket);
	def(pkState, "Net_Send", &lwNet_Send);

	class_<lwPacket>(pkState, "Packet")
		.def(pkState, constructor<BM::Stream *>())
		.def(pkState, "IsNil", &lwPacket::IsNil)
		.def(pkState, "Empty", &lwPacket::Empty)
		.def(pkState, "PopChar", &lwPacket::PopChar)
		.def(pkState, "PopByte", &lwPacket::PopByte)
		.def(pkState, "PopBool", &lwPacket::PopBool)
		.def(pkState, "PopShort", &lwPacket::PopShort)
		.def(pkState, "PopWord", &lwPacket::PopWord)
		.def(pkState, "PopInt", &lwPacket::PopInt)
		.def(pkState, "PopUInt", &lwPacket::PopUInt)
		.def(pkState, "PopSize_t", &lwPacket::PopSize_t)
		.def(pkState, "PopFloat", &lwPacket::PopFloat)
		.def(pkState, "PopString", &lwPacket::PopString)
		.def(pkState, "PopWString", &lwPacket::PopWString)
		.def(pkState, "PopWStringBuffer", &lwPacket::PopWStringBuffer)
		.def(pkState, "PopArray", &lwPacket::PopArray)
		.def(pkState, "PopMemoryAddr", &lwPacket::PopMemoryAddr)
		.def(pkState, "PopGuid", &lwPacket::PopGuid)
		.def(pkState, "PopWString", &lwPacket::PopWString)
		.def(pkState, "PopInt64", &lwPacket::PopInt64)
		.def(pkState, "PopPoint3", &lwPacket::PopPoint3)
		.def(pkState, "PopDateTime", &lwPacket::PopDateTime)

		.def(pkState, "PushChar", &lwPacket::PushChar)
		.def(pkState, "PushByte", &lwPacket::PushByte)
		.def(pkState, "PushBool", &lwPacket::PushBool)
		.def(pkState, "PushShort", &lwPacket::PushShort)
		.def(pkState, "PushWord", &lwPacket::PushWord)
		.def(pkState, "PushInt", &lwPacket::PushInt)
		.def(pkState, "PushUInt", &lwPacket::PushUInt)
		.def(pkState, "PushSize_t", &lwPacket::PushSize_t)
		.def(pkState, "PushFloat", &lwPacket::PushFloat)
		.def(pkState, "PushString", &lwPacket::PushString)
		.def(pkState, "PushWString", &lwPacket::PushWString)
		.def(pkState, "PushArray", &lwPacket::PushArray)
		.def(pkState, "PushGuid", &lwPacket::PushGuid)
		.def(pkState, "PushGuidForString", &lwPacket::PushGuidForString)
		.def(pkState, "PushInt64", &lwPacket::PushInt64)
		.def(pkState, "PushPoint3", &lwPacket::PushPoint3)
		.def(pkState, "PushPacket", &lwPacket::PushPacket)
		;
	return true;
}

BM::Stream * lwPacket::operator()()
{
	if(m_pkPacket)
	{
		return m_pkPacket;
	}

	return NULL;
}

bool lwPacket::IsNil()
{
	return (m_pkPacket == 0);
}

bool lwPacket::Empty()
{
	if ( !IsNil() )
	{
		return !m_pkPacket->Size();
	}
	return true;
}

char lwPacket::PopChar()
{
	if(m_pkPacket)
	{
		char cRet = 0;
		m_pkPacket->Pop(cRet);
		return cRet;
	}
	return 0;
}

BYTE lwPacket::PopByte()
{
	if(m_pkPacket)
	{
		BYTE cRet = 0;
		m_pkPacket->Pop(cRet);
		return cRet;
	}

	return 0;
}

short lwPacket::PopShort()
{
	if(m_pkPacket)
	{
		short sRet = 0;
		m_pkPacket->Pop(sRet);
		return sRet;
	}

	return 0;
}

WORD lwPacket::PopWord()
{
	if(m_pkPacket)
	{
		WORD sRet = 0;
		m_pkPacket->Pop(sRet);
		return sRet;
	}

	return 0;
}

int lwPacket::PopInt()
{
	if(m_pkPacket)
	{
		int iRet = 0;
		m_pkPacket->Pop(iRet);
		return iRet;
	}

	return 0;
}

unsigned int lwPacket::PopUInt()
{
	if(m_pkPacket)
	{
		unsigned int uiRet = 0;
		m_pkPacket->Pop(uiRet);
		return uiRet;
	}

	return 0;
}

size_t lwPacket::PopSize_t()
{
	if(m_pkPacket)
	{
		size_t iRet = 0;
		m_pkPacket->Pop(iRet);
		return iRet;
	}

	return 0;
}

float lwPacket::PopFloat()
{
	if(m_pkPacket)
	{
		float fRet = 0;
		m_pkPacket->Pop(fRet);
		return fRet;
	}

	return 0.0f;
}

char const *lwPacket::PopString()
{
	if(m_pkPacket)
	{
		static std::string kRet;
		kRet.clear();

		m_pkPacket->Pop(kRet);

		return kRet.c_str();
	}

	return NULL;
}

lwWString lwPacket::PopWStringBuffer(size_t const szLen)
{
	if(m_pkPacket)
	{
		static BYTE acRet[1024] = {0,};
		m_pkPacket->PopMemory(acRet, szLen);
		return lwWString((const wchar_t*)acRet, szLen/sizeof(wchar_t));
	}

	return lwWString("");

}
lwWString lwPacket::PopWString()
{
	if(m_pkPacket)
	{
		std::wstring kRet = _T("");
		m_pkPacket->Pop(kRet);
		return lwWString(kRet.c_str(),kRet.length());
	}

	return lwWString("");

}

char const* lwPacket::PopArray(size_t const szLen)
{
	if(m_pkPacket)
	{
		static BYTE acRet[4096] = {0,};
		m_pkPacket->PopMemory(acRet, szLen);
		return (char const*)&acRet[0];
	}

	return NULL;
}

unsigned long lwPacket::PopMemoryAddr(size_t const szLen)
{
	if(m_pkPacket)
	{
		char	*pMem = new char[szLen];	//	(주의!)요거 반드시 받은 쪽에서 해제시켜줘야한다.
		m_pkPacket->PopMemory(pMem,szLen);
		return (unsigned long)pMem;
	}

	return 0;
}

bool lwPacket::PopBool()
{
	if(m_pkPacket)
	{
		bool bData = false;
		m_pkPacket->Pop(bData);
		return bData;
	}

	return false;
}

lwGUID lwPacket::PopGuid()
{
	BM::GUID kGuid;
	if(m_pkPacket)
	{		
		m_pkPacket->Pop(kGuid);
		return lwGUID(MB(kGuid.str()));
	}

	return lwGUID(kGuid);
}

lwInt64 lwPacket::PopInt64()
{
	if(m_pkPacket)
	{
		__int64 i64Value = 0;
		m_pkPacket->Pop(i64Value);

		return lwInt64(i64Value);
	}

	return lwInt64(0);
}

lwPoint3 lwPacket::PopPoint3()
{
	POINT3 ptPos(0.0f, 0.0f, 0.0f);
	if(m_pkPacket)
	{		
		m_pkPacket->Pop(ptPos);
		return lwPoint3(ptPos.x, ptPos.y, ptPos.z);
	}

	return lwPoint3(ptPos.x, ptPos.y, ptPos.z);
}

lwDateTime lwPacket::PopDateTime()
{
	BM::DBTIMESTAMP_EX kDateTime;
	if(m_pkPacket)
	{		
		m_pkPacket->Pop(kDateTime);		
	}

	return lwDateTime(kDateTime);
}

void lwPacket::PushChar(char cData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(cData);
	}
}

void lwPacket::PushByte(BYTE cData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(cData);
	}
}

void lwPacket::PushBool(bool bData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(bData);
	}
}

void lwPacket::PushShort(short sData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(sData);
	}
}

void lwPacket::PushWord(WORD sData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(sData);
	}
}

void lwPacket::PushInt(int iData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(iData);
	}
}	

void lwPacket::PushUInt(unsigned int uiData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(uiData);
	}
}

void lwPacket::PushSize_t(size_t iData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(iData);
	}
}

void lwPacket::PushFloat(float fData)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(fData);
	}
}

void lwPacket::PushString(char const *pcData)
{
	if(m_pkPacket)
	{
		if ( pcData )
		{
			m_pkPacket->Push(std::string(pcData));
		}
		else
		{
			m_pkPacket->Push(std::string());
		}
	}
}

void lwPacket::PushWString(lwWString wstr)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(wstr());
	}
}

void lwPacket::PushArray(char const *pcData, size_t const szLen)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(pcData, szLen);
	}
}

void lwPacket::PushGuid(lwGUID kGuid)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(kGuid());
	}
}

void lwPacket::PushGuidForString(char const *pcData)
{
	if(m_pkPacket)
	{
		lwGUID kGuid(pcData);
		m_pkPacket->Push(kGuid());
	}
}

void lwPacket::PushInt64(lwInt64 kInt64)
{
	if(m_pkPacket)
	{
		m_pkPacket->Push(kInt64.GetValue());
	}
}

void lwPacket::PushPoint3(lwPoint3 kPoint3)
{
	if(m_pkPacket)
	{
		assert(m_pkPacket);
		POINT3 ptPos(kPoint3.GetX(), kPoint3.GetY(), kPoint3.GetZ());
		m_pkPacket->Push(ptPos);
	}
}

void lwPacket::PushPacket(lwPacket kPacket)
{
	if ( m_pkPacket && kPacket() )
	{
		m_pkPacket->Push( *(kPacket()) );
	}
}
