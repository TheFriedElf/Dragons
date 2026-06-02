#include "StdAfx.h"
#include "Variant/PgStringUtil.h"
#include "lwWString.h"
#include "BM/PgFilterString.h"
#include "PgCommandMgr.h"

using namespace lua_tinker;

lwWString::lwWString(char const *pcWString)
{
	if( NULL != pcWString )
	{
		m_kWString = UNI(pcWString);
	}
}

lwWString::lwWString(const wchar_t *pcWString, size_t const szCount)
{//count 는 의미 없음.
//	wchar_t szTemp[1024] = {0,};
//	::memcpy(szTemp, pcWString, szCount*sizeof(wchar_t));

	if( NULL != pcWString )
	{
		m_kWString = pcWString;
	}
}

lwWString::lwWString(lwWString &rkWString)
{
	m_kWString = rkWString.GetWString();
}

lwWString::lwWString(std::wstring const &wstr)
{
	m_kWString = wstr;
}

bool lwWString::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "GetAppendTextW", &lwWString::GetAppendTextW);

	// 등록한다.
	class_<lwWString>(pkState, "WideString")
		.def(pkState, constructor<char const *>())
		.def(pkState, "Set", &lwWString::Set)
		.def(pkState, "Add", &lwWString::Add)
		.def(pkState, "IsEqual", &lwWString::IsEqual)
		.def(pkState, "Generate", &lwWString::Generate)
		.def(pkState, "GetStr", &lwWString::GetStr)
		.def(pkState, "Length", &lwWString::Length)
		.def(pkState, "IsNil", &lwWString::IsNil)
		.def(pkState, "EraseCRLF", &lwWString::EraseCRLF)
		.def(pkState, "FilterString", &lwWString::FilterString)
		.def(pkState, "ReplaceInt", &lwWString::ReplaceInt)
		.def(pkState, "ReplaceStr", &lwWString::ReplaceStr)
		;

	return true;
}

lwWString	lwWString::GetAppendTextW(lwWString Text1,lwWString Text2)	//	Text1 과 Text2 를 연결한 새로운 텍스트를 리턴한다.
{
	return	lwWString(Text1()+Text2());
}

char const*	lwWString::GetStr()
{
	static std::string strString[100];
	static	int	index = -1;

	index++;
	if(index>=100) index = 0;

		
	strString[index]= MB(m_kWString);
	return strString[index].c_str();
}

std::wstring &lwWString::operator ()()
{
	return m_kWString;
}

bool lwWString::Set(char const *pcWString)
{
	if( pcWString == NULL )
		return false;

	m_kWString = UNI(pcWString);

	return true;
}

void lwWString::Add(char const *pcText)
{
	if( NULL != pcText )
	{
		m_kWString += UNI(pcText);
	}
}

bool lwWString::IsEqual(lwWString kWString)
{
	return m_kWString == kWString();
}

void lwWString::Generate()
{
	//m_kGuid.Generate();	//필요 없을 듯
}

int lwWString::Length()
{
	return m_kWString.size();
}

bool lwWString::IsNil()
{
	return (m_kWString.size() == 0);
}

void lwWString::EraseCRLF()
{
	PgStringUtil::EraseStr<std::wstring>(m_kWString, _T("\n"), m_kWString);//케리지 리턴 삭제
}

bool lwWString::FilterString(int const iMode, bool const bConvert)
{
	EFilterSourceType eType = FST_NONE;
	switch(iMode)
	{
	case 1:{ eType = FST_BADWORD; }break;
	case 2:{ eType = FST_NICKNAME; }break;
	case 3:{ eType = FST_ALL; }break;
	default:
		{
			PG_ASSERT_LOG(0 && "Invalid iMode");
		}break;
	}
	bool const bIsFiltered = g_kClientFS.Filter(m_kWString, bConvert, eType);
	return bIsFiltered;
}

template< typename T >
void lwWString::Replace(char const* pkFindStr, T const& kData)
{
	if(!pkFindStr)
	{
		return;
	}
	BM::vstring kTmpStr(m_kWString);
	kTmpStr.Replace(UNI(pkFindStr), kData);
	m_kWString = static_cast<std::wstring>(kTmpStr);
}

void lwWString::ReplaceInt(char const* pkFindStr, int const kData)
{
	Replace(pkFindStr, kData);
}

void lwWString::ReplaceStr(char const* pkFindStr, char const* kData)
{
	Replace(pkFindStr, std::string(kData));
}

