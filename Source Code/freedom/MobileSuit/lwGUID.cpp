#include "StdAfx.h"
#include "lwGUID.h"

using namespace lua_tinker;

lwGUID::lwGUID(char const *pcGuid)
{
	if(pcGuid)
		m_kGuid.Set(pcGuid);
}

lwGUID::lwGUID(BM::GUID const &rkGuid)
{
	m_kGuid = rkGuid;
}

bool lwGUID::RegisterWrapper(lua_State *pkState)
{
	// 등록한다.
	class_<lwGUID>(pkState, "GUID")
		.def(pkState, constructor<char const *>())
		.def(pkState, "Set", &lwGUID::Set)
		.def(pkState, "IsEqual", &lwGUID::IsEqual)
		.def(pkState, "Generate", &lwGUID::Generate)
		.def(pkState, "GetString", &lwGUID::GetString)
		.def(pkState, "MsgBox", &lwGUID::MsgBox)
		.def(pkState, "ODS", &lwGUID::ODS)
		.def(pkState, "IsNil", &lwGUID::IsNil)
		;

	return true;
}

BM::GUID &lwGUID::operator ()()
{
	return m_kGuid;
}

bool lwGUID::Set(char const *pcGuid)
{
	if(pcGuid)
	{
		return m_kGuid.Set(pcGuid);
	}
	return false;
}

bool lwGUID::IsEqual(lwGUID kGuid)
{
	return m_kGuid == kGuid();
}
char const*	lwGUID::GetString()
{
	static std::string strGuid[100];
	static	int	guid_index = -1;

	guid_index++;
	if(guid_index>=100) guid_index = 0;
		
	strGuid[guid_index]= MB(m_kGuid.str());
	return strGuid[guid_index].c_str();
}
void lwGUID::MsgBox(char const *Caption)
{
	MessageBox(NULL,m_kGuid.str().c_str(),UNI(Caption),MB_OK);
}

void lwGUID::ODS()
{
	_PgOutputDebugString("Guid : %s\n", MB(m_kGuid.str()));
}

bool lwGUID::IsNil()const
{
	return (m_kGuid == BM::GUID::NullData());
}

void lwGUID::Generate()
{
	m_kGuid.Generate();
}

