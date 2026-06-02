#include "StdAfx.h"
#include "lwInt64.h"

using namespace lua_tinker;

lwInt64::lwInt64(void)
{
	m_iValue = 0;
}

lwInt64::lwInt64(__int64 iValue)
{
	m_iValue = iValue;
}

bool lwInt64::RegisterWrapper(lua_State *pkState)
{
	// 등록한다.
	class_<lwInt64>(pkState, "INT64")
		.def(pkState, constructor<void>())
		.def(pkState, "Set", &lwInt64::Set)
		.def(pkState, "IsBig", &lwInt64::IsBig)
		;

	return true;
}

void lwInt64::Set(int iLow, int iHigh)
{
	// 루아 내부에서도 Int64값을 넣어 보고 싶다.
	// 좀더 안전한 방법을 찾아 보고 싶지만.. 달리 좋은 방법이 없다.
	// (-) 값 입력도 문제 되네 ㅡㅡ;
	m_iValue = 0;
	if (iHigh != 0)
	{
		::memcpy(LPBYTE(&m_iValue)+4, &iHigh, sizeof(int));
	}
	m_iValue += iLow;
}

bool lwInt64::IsBig( lwInt64 i64Value )
{
	return m_iValue < i64Value.m_iValue;
}
