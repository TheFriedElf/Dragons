#include "StdAfx.h"
#include "PgWebLogin.h"
#include "lohengrin/packettype.h"
#include "PgMobileSuit.h"

PgWebLogin::PgWebLogin(void)
{
	IsDisconnectServer(false);
}

PgWebLogin::~PgWebLogin(void)
{
}

bool PgWebLogin::Init(std::wstring const &strCmd)
{
	typedef std::list< std::wstring > CUT_STRING;
	CUT_STRING kMainList;
	BM::vstring::CutTextByKey<std::wstring>(strCmd, L"/", kMainList);

	CUT_STRING::iterator main_txt_itor = kMainList.begin();
	while(main_txt_itor != kMainList.end())
	{
		CUT_STRING kSubList;

		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_JAPAN:
			{
				CUT_STRING::size_type const first_pos = (*main_txt_itor).find_first_of(L":");
				kSubList.push_back( (*main_txt_itor).substr(0, first_pos ) );
				kSubList.push_back( (*main_txt_itor).substr( first_pos + 1 ) );
			}break;
		default:
			{
				BM::vstring::CutTextByKey<std::wstring>((*main_txt_itor), L":", kSubList);
			}break;
		}

		if( kSubList.size() )
		{
			CUT_STRING::iterator sub_itor = kSubList.begin();

			if(	0 == ::_tcsicmp(_T("NexonPassport"), (*sub_itor).c_str()))
			{
				++sub_itor;
				CUT_STRING kDetailList;
				BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

				CUT_STRING::iterator detail_itor = kDetailList.begin();
				if(kDetailList.end() != detail_itor)
				{
					NexonPassport((*detail_itor));
				}
			}

			if(	0 == ::_tcsicmp(_T("SessKey"), (*sub_itor).c_str()))
			{
				++sub_itor;
				CUT_STRING kDetailList;
				BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

				CUT_STRING::iterator detail_itor = kDetailList.begin();
				if(kDetailList.end() != detail_itor)
				{
					SessKey((*detail_itor));
				}
			}

			if(	0 == ::_tcsicmp(_T("ServerAddr"), (*sub_itor).c_str()))
			{
				++sub_itor;
				CUT_STRING kDetailList;
				BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

				CUT_STRING::iterator detail_itor = kDetailList.begin();
				if(kDetailList.end() != detail_itor)
				{
					ServerAddr((*detail_itor));
				}
			}

			if(	0 == ::_tcsicmp(_T("MacAddr"), (*sub_itor).c_str()))
			{
				++sub_itor;
				CUT_STRING kDetailList;
				BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

				CUT_STRING::iterator detail_itor = kDetailList.begin();
				if(kDetailList.end() != detail_itor)
				{
					MacAddr((*detail_itor));
				}
			}
		}

		++main_txt_itor;
	}

	TCHAR acText[MAX_PATH] = {0,};

	int iRet = 0;
	iRet = GetEnvironmentVariable(L"NexonPassport", acText, MAX_PATH);
	if(iRet)
	{
		NexonPassport(acText);
	}
	
	iRet = GetEnvironmentVariable(L"SessKey", acText, MAX_PATH);
	if(iRet)
	{
		SessKey(acText);
	}

	iRet = GetEnvironmentVariable(L"ServerAddr", acText, MAX_PATH);
	if(iRet)
	{
		ServerAddr(acText);
	}
	
	iRet = GetEnvironmentVariable(L"MacAddr", acText, MAX_PATH);
	if(iRet)
	{
		MacAddr(acText);
	}

	return true;
}

bool PgWebLogin::IsCorrectArg()const
{
	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_KOREA:
		{
			if(	SessKey().size() &&	ServerAddr().size() )
			{
				return true;
			}
		}break;
	case LOCAL_MGR::NC_JAPAN:
		{
			if( NexonPassport().size() )
			{
				return true;
			}
		}break;
	default:
		{
		}break;
	}

	return false;
}
