#include "StdAfx.h"
#include "PgCmdlineParse.h"
#include "PgMobileSuit.h"

PgCmdlineParse::PgCmdlineParse(void)
	: m_kFullMode(-1)
	, m_kGraphicMode(-1)
	, m_kSiteIndex(-1)
	, m_kLaunchByLauncher(false)
	, m_bIsUseForceAccount(false)
	, m_bIsLoginFuncOff(false)
{
	
}

PgCmdlineParse::~PgCmdlineParse(void)
{
}

bool PgCmdlineParse::Init(std::wstring const &strCmd)
{
	typedef std::list< std::wstring > CUT_STRING;
	CUT_STRING kMainList;
	BM::vstring::CutTextByKey<std::wstring>(strCmd, L"/", kMainList);

	CUT_STRING::iterator main_txt_itor = kMainList.begin();
	while(main_txt_itor != kMainList.end())
	{
		CUT_STRING kSubList;
		BM::vstring::CutTextByKey<std::wstring>((*main_txt_itor), L":", kSubList);

		switch(kSubList.size())
		{
		case 2:
			{
				CUT_STRING::iterator sub_itor = kSubList.begin();

				if(	0 == ::_tcsicmp(_T("ID"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						ID((*detail_itor));
						NILOG(PGLOG_LOG, "ID %s\n", MB(ID()));
					}
				}
				if(	0 == ::_tcsicmp(_T("PW"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						if(ID().size())
						{
							m_bIsUseForceAccount = true;
							PW((*detail_itor));
							NILOG(PGLOG_LOG, "PW %s\n", MB(PW()));
						}
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
						ServerIP((*detail_itor));
						NILOG(PGLOG_LOG, "Local Server IP %s\n", MB(ServerIP()));
					}
				}
				if(	0 == ::_tcsicmp(_T("ServerPort"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						std::wstring strPort(*detail_itor);
						ServerPort(atoi(MB(strPort.c_str())));
						NILOG(PGLOG_LOG, "Local Server PORT %d\n", ServerPort());
					}
				}
				if( 0 == ::_tcsicmp(_T("FullScreen"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);
					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						FullMode(::_wtoi((*detail_itor).c_str()));
					}
				}
				if( 0 == ::_tcsicmp(_T("GraphicOption"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);
					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						GraphicMode(::_wtoi((*detail_itor).c_str()));
					}
				}

				if(	0 == ::_tcsicmp(_T("SiteName"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						SiteName((*detail_itor));						
					}
				}

				if(	0 == ::_tcsicmp(_T("SiteIndex"), (*sub_itor).c_str()))
				{
					++sub_itor;
					CUT_STRING kDetailList;
					BM::vstring::CutTextByKey<std::wstring>((*sub_itor), L"\"", kDetailList);

					CUT_STRING::iterator detail_itor = kDetailList.begin();
					if(kDetailList.end() != detail_itor)
					{
						SiteIndex(::_wtoi((*detail_itor).c_str()));
					}
				}
			}break;
		case 1:
			{
				CUT_STRING::iterator sub_itor = kSubList.begin();

				if( 0 == ::_tcsicmp(_T("LaunchByLauncher"), (*sub_itor).c_str()))
				{
					LaunchByLauncher(true);
				}
			}break;
		}
		++main_txt_itor;
	}
	return true;
}

std::wstring PgCmdlineParse::ServerAddr()const
{
	return m_kServerIP;
	BM::vstring vstr; 
	vstr << m_kServerIP;
	vstr << L":";
	vstr << m_kServerPort;
	return vstr;
}

bool PgCmdlineParse::MakeLoginPacket(BM::Stream &kPacket)const
{
	SClientTryLogin kCTL;
	kCTL.SetID( ID() );
	kCTL.SetPW( PW() );
	kCTL.SetVersion( PACKET_VERSION_C, PACKET_VERSION_S );
	kCTL.PatchVersion(g_pkApp->PatchVer().iVersion);

	kPacket.Push(PT_C_L_TRY_AUTH);
	kCTL.WriteToPacket(kPacket);
	return true;
}
