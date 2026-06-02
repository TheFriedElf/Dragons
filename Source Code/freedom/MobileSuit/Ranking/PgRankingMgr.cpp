#include "stdafx.h"
#include "BM/STLSupport.h"
#include "BM/filesupport.h"
#include "Variant/PgEventView.h"
#include "../PgNetwork.h"
#include "../lwGUID.h"
#include "../lwUI.h"
#include "../lwUIQuest.h"
#include "PgRankingMgr.h"

TCHAR PgRankingMgr::ms_szURL[MAX_PATH] = {0,};

bool lwInitRanking( lwUIWnd kWnd )
{
	if ( kWnd.IsNil() )
	{
		return false;
	}

	kWnd()->ClearCustomData();
	return g_kRanking.Init();
}

void lwClearRanking()
{
	g_kRanking.Clear();
}

void lwCallRanking( int const iRankType )
{
	g_kRanking.RankType(iRankType);
	g_kRanking.Call( SRankingKey( g_kNetwork.TryRealmNo(), iRankType ) );
}

void lwDivideClass( int const iClassType )
{
	g_kRanking.Call( SRankingKey( g_kNetwork.TryRealmNo(), g_kRanking.RankType(), iClassType) );
}

void lwRefreshRanking( int iPage )
{
	g_kRanking.RefreshRanking( iPage );
}

bool lwFindRank( lwGUID kGuid )
{
	if ( S_OK == g_kRanking.FindRank( kGuid() ) )
	{
		return true;
	}
	return false;
}

void lwUpDateClassType(lwUIWnd Parent, int Type)
{
	lwUIWnd TempWnd = Parent.GetControl("SFRM_SELECT_CLASS_TYPE_LIST");
	if ( TempWnd.IsNil() )
	{
		return;
	}
	
	XUI::CXUI_List* pListWnd = dynamic_cast<XUI::CXUI_List*>(TempWnd.GetControl("LST_SELECT_TYPE").GetSelf());
	if ( !pListWnd )
	{
		return;
	}
	pListWnd->ClearList();

	int iMaxClassCount = 28;
	switch(g_kLocal.ServiceRegion())
	{	
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_VIETNAM:
	case LOCAL_MGR::NC_KOREA:
		{//3차 전직 릴리즈된 안된국가
			iMaxClassCount = 20;
		}break;
	default:
		{
			iMaxClassCount = 28;
		}break;
	}

	switch(g_kRanking.GetFilterClass(Type))
	{
	case RANKFILTER_CLASS_ONE:
		{
			XUI::SListItem* kItem = pListWnd->AddItem(TTW(30000));
			if( kItem )
			{
				XUI::CXUI_Wnd* pItemWnd = kItem->m_pWnd;
				pItemWnd->Text(TTW(30000));
			}

			CONT_DEFUPGRADECLASS const* pkDefUpgradeClass = NULL;
			g_kTblDataMgr.GetContDef( pkDefUpgradeClass );

			if ( pkDefUpgradeClass )
			{
				CONT_DEFUPGRADECLASS::const_iterator itr = pkDefUpgradeClass->begin();
				for( ; itr != pkDefUpgradeClass->end() ; ++itr )
				{
					if( itr->first > iMaxClassCount ) 
					{
						break;
					}

					XUI::SListItem* kItem = pListWnd->AddItem(TTW(30000 + itr->first.kKey));
					if( kItem )
					{
						XUI::CXUI_Wnd* pItemWnd = kItem->m_pWnd;
						pItemWnd->Text(TTW(30000 + itr->first.kKey));
						pItemWnd->SetCustomData(&itr->first.kKey, sizeof(itr->first.kKey));
					}
				}
			}
		}break;
	case RANKFILTER_CLASS_PARTY:
		{
			for(int i = 0; i < 5; ++i)
			{
				XUI::SListItem* kItem = pListWnd->AddItem(TTW(400167 + i));
				if( kItem )
				{
					XUI::CXUI_Wnd* pItemWnd = kItem->m_pWnd;
					pItemWnd->Text(TTW(400167 + i));
					pItemWnd->SetCustomData(&i, sizeof(i));
				}
			}
		}break;
	}
	
	TempWnd.GetSelf()->Size(POINT2(TempWnd.GetSelf()->Size().x, 23 * pListWnd->GetTotalItemCount() + 3));
	pListWnd->Size(POINT2(pListWnd->Size().x, 23 * pListWnd->GetTotalItemCount() + 3));
}

PgRankingMgr::PgRankingMgr()
:	m_iLastUpdateDate(0)
,	m_iNextUpdateDate(0)
{
}

PgRankingMgr::~PgRankingMgr()
{
	Clear();
}

void PgRankingMgr::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "InitRanking", &lwInitRanking);
	def(pkState, "ClearRanking", &lwClearRanking);
	def(pkState, "CallRanking", &lwCallRanking);
	def(pkState, "RefreshRanking", &lwRefreshRanking);
	def(pkState, "FindRank", &lwFindRank);
	def(pkState, "DivideClass", &lwDivideClass);
	def(pkState, "UpDateClassType", &lwUpDateClassType);
}

E_RANK_FILTER_CLASS PgRankingMgr::GetFilterClass( int const iType )
{
	switch( iType )
	{
	case RANKTYPE_RANK_EXP:
	case RANKTYPE_RANK_PVP:
	case RANKTYPE_RANK_MAXCOMBO:
		{
			return RANKFILTER_CLASS_ONE;
		}break;
	case RANKTYPE_RANK_LEVELUP_20:
	case RANKTYPE_RANK_LEVELUP_40:
	case RANKTYPE_RANK_LEVELUP_60:
	case RANKTYPE_RANK_LEVELUP_80:
	case RANKTYPE_RANK_LEVELUP_100:
		{
			return RANKFILTER_CLASS_PARTY;
		}break;
	}
	return RANKFILTER_CLASS_NONE;
}

bool PgRankingMgr::Check( SRankingKey& kKey )
{
	CONT_DEFUPGRADECLASS const* pkDefUpgradeClass = NULL;
	g_kTblDataMgr.GetContDef( pkDefUpgradeClass );

	if ( pkDefUpgradeClass )
	{
		CONT_DEFUPGRADECLASS::const_iterator class_itr = pkDefUpgradeClass->find( kKey.iClass );
		if ( class_itr != pkDefUpgradeClass->end() )
		{
			switch ( GetFilterClass(kKey.iType ) )
			{
			case RANKFILTER_CLASS_NONE:
				{
					if ( !kKey.iClass )
					{
						return true;
					}
				}break;
			case RANKFILTER_CLASS_PARTY:
				{
					if ( (int)(class_itr->second.byKind) != kKey.iClass )
					{
						kKey.iClass = (int)(class_itr->second.byKind);
						return false;
					}
				}// break;//No break
			default:
				{
					return true;
				}break;
			}
		}
	}

	kKey.iClass = 0;
	return false;
}

void PgRankingMgr::RefreshNextUpdate( __int64 i64NowTime )
{
	SYSTEMTIME kNowTime;
	CGameTime::SecTime2SystemTime( i64NowTime, kNowTime, CGameTime::DEFAULT );

	if ( kNowTime.wHour < 6 )
	{
		i64NowTime -= CGameTime::DEFAULT;
		CGameTime::SecTime2SystemTime( i64NowTime, kNowTime, CGameTime::DEFAULT );
	}

	kNowTime.wHour = 6;
	kNowTime.wMinute = 0;
	kNowTime.wSecond = 0;
	kNowTime.wMilliseconds = 0;
	CGameTime::GetNextTime( m_iNextUpdateDate, kNowTime, CGameTime::EVERY_DAY );
}

bool PgRankingMgr::Dump()
{
	if ( !m_iLastUpdateDate )
	{
		m_iLastUpdateDate = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
	}

	RefreshNextUpdate( m_iLastUpdateDate );

	BM::Stream kPacket;
	kPacket.Push(m_iNextUpdateDate);
	kPacket.Push(m_iLastUpdateDate);
	

	size_t iSize = m_kContData.size();
	kPacket.Push(iSize);

	CONT_RANKDATA::const_iterator itr;
	for ( itr=m_kContData.begin(); itr!=m_kContData.end(); ++itr )
	{
		kPacket.Push(itr->first);
		PU::TWriteArray_M( kPacket, itr->second );
	}

	return BM::EncSave(std::wstring(L"./RANK.DAT"), kPacket.Data());
}

bool PgRankingMgr::LoadDump()
{
	m_kContData.clear();
	m_iLastUpdateDate = 0;
	m_iNextUpdateDate = 0;

	std::wstring wstrRankFile( L"./RANK.DAT" );
	BM::Stream kPacket;

	// 파일이 열려 있는지 확인해보자
	{
		BM::IFStream ffile(wstrRankFile);
		if ( !ffile.IsOpen() )
		{
			// 파일이 열려 있지 않으면 파일을 만들어야 한다.
			kPacket.Push(_I64_MIN);
			if ( !BM::EncSave( wstrRankFile, kPacket.Data() ) )
			{
				return false;
			}
			kPacket.Clear();
		}
	}
	
	
	if( BM::DecLoad(wstrRankFile, kPacket.Data()) )
	{
		kPacket.PosAdjust();
		kPacket.Pop( m_iNextUpdateDate );

		__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
		if ( i64NowTime < m_iNextUpdateDate )
		{
			kPacket.Pop( m_iLastUpdateDate );
			size_t iSize = 0;
			kPacket.Pop( iSize );

			
			CONT_RANKDATA::mapped_type kElement;
			while ( iSize-- )
			{
				CONT_RANKDATA::key_type kKey;
				kPacket.Pop( kKey );
				kElement.clear();
				PU::TLoadArray_M( kPacket, kElement );
				m_kContData.insert( std::make_pair( kKey, kElement ) );
			}

			if ( kPacket.RemainSize() )
			{// 남은 사이즈가 있다면 뭔가 잘못된 데이터 이다.
				m_kContData.clear();
			}
		}
		else
		{
			RefreshNextUpdate( i64NowTime );
		}
		return true;
	}
	return false;
}


bool PgRankingMgr::Init()
{
	if ( !m_iNextUpdateDate )
	{
		if ( !LoadDump() )
		{
			RefreshNextUpdate(g_kEventView.GetLocalSecTime(CGameTime::DEFAULT));
		}
		return true;
	}
	return false;
}

void PgRankingMgr::Clear()
{
	if ( Dump() )
	{
		m_kContData.clear();
		m_iLastUpdateDate = 0;
		m_iNextUpdateDate = 0;
	}
}

bool PgRankingMgr::RefreshRanking( int iPage )
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	if ( XUIMgr.IsActivate( _T("SFRM_RANKING"), pkWnd) )
	{
		SRankingKeyPage kKeyPage;
		if ( pkWnd->GetCustomData( &kKeyPage, sizeof(SRankingKeyPage) ) )
		{
			if ( iPage )
			{
				kKeyPage.iPage = iPage;
			}
			return Call( pkWnd, kKeyPage );
		}
	}
	return false;
}

HRESULT PgRankingMgr::FindRank( BM::GUID const &kCharGuid )
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	if ( XUIMgr.IsActivate( _T("SFRM_RANKING"), pkWnd ) )
	{
		SRankingKeyPage kKeyPage;
		if ( pkWnd->GetCustomData( &kKeyPage, sizeof(SRankingKeyPage) ) )
		{
			CONT_RANKDATA::const_iterator itr = m_kContData.find( kKeyPage.kKey );
			if ( itr != m_kContData.end() )
			{
				int iCount = 0;
				CONT_RANKDATA::mapped_type::const_iterator find_itr = std::find_if( itr->second.begin(), itr->second.end(), SRankData_FindGUID(kCharGuid,iCount) );
				if ( find_itr != itr->second.end() )
				{
					kKeyPage.iPage = iCount / ms_iItemCountForPage + 1;
					Call( pkWnd, kKeyPage );

					int iSlot = iCount % ms_iItemCountForPage;
					wchar_t szTemp[MAX_PATH] = {0,};
					::swprintf_s(szTemp, MAX_PATH, _T("FRM_ITEM_%d"), iSlot - 1);
					XUI::CXUI_Wnd* pTemp = pkWnd->GetControl(std::wstring(szTemp));
					if( pTemp )
					{
						pTemp = pTemp->GetControl(_T("CBTN_ITEM_SELECT"));
						if( pTemp )
						{
							lua_tinker::call<void, lwUIWnd>("RankingSelectedSet", lwUIWnd(pTemp));
						}
					}
					return S_OK;
				}
				return S_FALSE;
			}
		}
	}
	return E_FAIL;
}

HRESULT PgRankingMgr::FindRank( std::wstring& wstrCharacterName )
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	if ( XUIMgr.IsActivate( _T("SFRM_RANKING"), pkWnd ) )
	{
		SRankingKeyPage kKeyPage;
		if ( pkWnd->GetCustomData( &kKeyPage, sizeof(SRankingKeyPage) ) )
		{
			CONT_RANKDATA::const_iterator itr = m_kContData.find( kKeyPage.kKey );
			if ( itr != m_kContData.end() )
			{
				int iCount = 0;
				CONT_RANKDATA::mapped_type::const_iterator find_itr = std::find_if( itr->second.begin(), itr->second.end(), SRankData_FindName(wstrCharacterName,iCount) );
				if ( find_itr != itr->second.end() )
				{
					kKeyPage.iPage = iCount / ms_iItemCountForPage + 1;
					Call( pkWnd, kKeyPage );
					return S_OK;
				}
				return S_FALSE;
			}
		}
	}
	return E_FAIL;
}

bool PgRankingMgr::Call( CONT_RANKDATA::key_type &kKey )
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( _T("SFRM_RANKING") );
	if ( pkWnd && IsInit() )
	{
		XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("SFRM_SELECT_TYPE") );
		if ( pkTemp )
		{
			pkTemp->Text( TTW(kKey.iType+300000) );
		}

		Check(kKey);

		bool const bIsVisible = (RANKFILTER_CLASS_NONE < GetFilterClass(kKey.iType));

		pkTemp = pkWnd->GetControl(_T("FRM_CLASS_TYPE"));
		if ( pkTemp )
		{
			pkTemp->Visible( bIsVisible );			
		}

		pkTemp = pkWnd->GetControl(_T("SFRM_SELECT_CLASS"));
		if ( pkTemp )
		{
			pkTemp->Visible( bIsVisible );
		}

		pkTemp = pkWnd->GetControl(_T("SFRM_SELECT_CLASS_SHADOW"));
		if ( pkTemp )
		{
			pkTemp->Visible( bIsVisible );
			pkTemp = pkTemp->GetControl(_T("BTN_DROPDOWN"));
			if ( pkTemp )
			{
				pkTemp->Visible( bIsVisible );
			}
		}

		return Call( pkWnd, SRankingKeyPage(kKey) );
	}
	return false;
}

bool PgRankingMgr::Call( XUI::CXUI_Wnd* pkWnd, SRankingKeyPage &kKeyPage )
{
	if ( kKeyPage.iPage <= 0 ){kKeyPage.iPage=1;}

	__int64 const i64NowTime =  g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);

	if ( i64NowTime >= m_iNextUpdateDate )
	{
		m_kContData.clear();
		RefreshNextUpdate( i64NowTime );
	}

	CONT_RANKDATA::const_iterator itr = m_kContData.find( kKeyPage.kKey );
	if ( itr == m_kContData.end() )
	{
		CONT_RANKDATA::mapped_type kCont;
		switch ( GetHttpData( kKeyPage.kKey, kCont ) )
		{
		case S_OK:
			{
				std::pair<CONT_RANKDATA::iterator, bool> kPair = m_kContData.insert( std::make_pair(kKeyPage.kKey, kCont) );
				itr = kPair.first;
				m_iLastUpdateDate = i64NowTime;
			}break;
		case E_ABORT:
			{
				lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(301202)), true);
				return false;
			}break;
		case E_FAIL:
			{
				lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(301201)), true);
				return false;
			}break;
		}
	}

	pkWnd->SetCustomData( &kKeyPage, sizeof(SRankingKeyPage) );
	CONT_RANKDATA::mapped_type const& kElement = itr->second;

	int iIndex = (kKeyPage.iPage-1) * ms_iItemCountForPage;
	int i = 0;

	wchar_t wszTemp[MAX_PATH] = {0,};
	XUI::CXUI_Wnd *pkTemp = NULL;
	XUI::CXUI_Wnd *pkControl = NULL;

	pkControl = pkWnd->GetControl( std::wstring(L"SFRM_POINT") );
	if ( pkControl )
	{
		pkControl->Text( TTW( 300100 + kKeyPage.kKey.iType ) );
	}

	while ( 1 )
	{
		::swprintf_s( wszTemp, MAX_PATH, L"FRM_ITEM_%d", i++ );
		pkControl = pkWnd->GetControl( std::wstring(wszTemp) );
		if ( !pkControl )
		{
			break;
		}

		if ( kElement.size() > (size_t)iIndex )
		{
			std::wstring kToolTipText;
			pkControl->Visible(true);
			pkTemp = pkControl->GetControl(_T("FRM_ORDER_NO"));
			if ( pkTemp )
			{
				::swprintf_s( wszTemp, MAX_PATH, L"%u", kElement.at(iIndex).usRank );
				Quest::SetCutedTextLimitLength(pkTemp, wszTemp, L"...");
				kToolTipText += TTW(300001) + L" : ";
				kToolTipText += wszTemp;
				kToolTipText +=  L"\n";
			}

			pkTemp = pkControl->GetControl(_T("FRM_NAME"));
			if ( pkTemp )
			{
				std::wstring const& rkName = kElement.at(iIndex).wstrName;
				Quest::SetCutedTextLimitLength(pkTemp, rkName, L"...");
				kToolTipText += TTW(300002) + L" : ";
				kToolTipText += rkName;
				kToolTipText +=  L"\n";
			}

			pkTemp = pkControl->GetControl(_T("FRM_LEVEL"));
			if ( pkTemp )
			{
				::swprintf_s( wszTemp, MAX_PATH, L"%u", kElement.at(iIndex).usLevel );
				Quest::SetCutedTextLimitLength(pkTemp, wszTemp, L"...");
				kToolTipText += TTW(300003) + L" : ";
				kToolTipText += wszTemp;
				kToolTipText +=  L"\n";
			}

			pkTemp = pkControl->GetControl(_T("FRM_GUILD"));
			if ( pkTemp )
			{
				std::wstring const& rkGuildName = kElement.at(iIndex).wstrGuildName;
				Quest::SetCutedTextLimitLength(pkTemp, rkGuildName, L"...");
				kToolTipText += TTW(300004) + L" : ";
				kToolTipText += rkGuildName;
				kToolTipText +=  L"\n";
			}

			pkTemp = pkControl->GetControl(_T("FRM_POINT"));
			if ( pkTemp )
			{
				switch ( kKeyPage.kKey.iType )
				{
				case RANKTYPE_RANK_LEVELUP_20:
				case RANKTYPE_RANK_LEVELUP_40:
				case RANKTYPE_RANK_LEVELUP_60:
				case RANKTYPE_RANK_LEVELUP_80:
				case RANKTYPE_RANK_LEVELUP_100:
					{
						int const iSecond = (int)(kElement.at(iIndex).iValue % 60i64);
						int const iMin = (int)(kElement.at(iIndex).iValue / 60i64 % 60i64);
						__int64 const iHour = kElement.at(iIndex).iValue / 3600i64;
						::swprintf_s( wszTemp, MAX_PATH, L"%I64d:%02d:%02d", iHour, iMin, iSecond );
						Quest::SetCutedTextLimitLength(pkTemp, wszTemp, L"...");
						kToolTipText += TTW(300005) + L" : ";
						kToolTipText += wszTemp;
						kToolTipText +=  L"\n";
					}break;
				default:
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%I64d", kElement.at(iIndex).iValue );
						Quest::SetCutedTextLimitLength(pkTemp, wszTemp, L"...");
						kToolTipText += TTW(300005) + L" : ";
						kToolTipText += wszTemp;
						kToolTipText +=  L"\n";
					}break;
				}
			}

			pkTemp = pkControl->GetControl(_T("FRM_CLASS"));
			if ( pkTemp )
			{
				pkTemp = pkTemp->GetControl( _T("IMG_CLASS") );
				if( kElement.size() > iIndex
					&& 0 <= iIndex
					)
				{// 직업 아이콘 설정
					VEC_RANKDATA::value_type const& rkElem = kElement.at(iIndex);
					lwSetMiniClassIconIndex(pkTemp, rkElem.usClass);
				}
			}

			pkTemp = pkControl->GetControl(_T("CBTN_ITEM_SELECT"));
			if( pkTemp )
			{
				lwUIWnd(pkTemp).SetCustomDataAsStr(MB(kToolTipText));
				XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTemp);
				if( pBtn )
				{
					pBtn->Check(false);
				}
				pkTemp = pkTemp->GetControl(_T("IMG_SELECT"));
				if( pkTemp )
				{
					pkTemp->Visible(pBtn->Check());
				}
			}
		}
		else
		{
			pkControl->Visible(false);
		}
		++iIndex;
	};

	// Page Control
	XUI::CXUI_Wnd* pkPageControl = pkWnd->GetControl( _T("FRM_PAGE_CONTROL") );
	if ( pkPageControl )
	{
		int const iLastPage = (int)kElement.size() / ms_iItemCountForPage + 1;

		int iControlStartPage = 5 * (kKeyPage.iPage/5) + 1;
		if ( 0 == (kKeyPage.iPage%5) )
		{
			iControlStartPage -= 5;
		}

		pkTemp = pkPageControl->GetControl( _T("BTN_TO_FIRST") );
		if ( pkTemp )
		{
			pkTemp->IsClosed(1==kKeyPage.iPage);
			int iTemp = iControlStartPage - 5;
			if ( iTemp < 1 )
			{
				iTemp = 1;
			}
			pkTemp->SetCustomData( &iTemp, sizeof(int) );
		}

		pkTemp = pkPageControl->GetControl( _T("BTN_TO_PREV") );
		if ( pkTemp )
		{
			pkTemp->IsClosed(1==kKeyPage.iPage);
			int iTemp = kKeyPage.iPage - 1;
			if ( iTemp < 1 )
			{
				iTemp = 1;
			}
			pkTemp->SetCustomData( &iTemp, sizeof(int) );
		}

		pkTemp = pkPageControl->GetControl( _T("BTN_TO_NEXT") );
		if ( pkTemp )
		{
			pkTemp->IsClosed(kKeyPage.iPage==iLastPage);
			int iTemp = kKeyPage.iPage + 1;
			if ( iTemp < iLastPage )
			{
				pkTemp->SetCustomData( &iTemp, sizeof(int) );
			}
			else
			{
				pkTemp->SetCustomData( &iLastPage, sizeof(int) );
			}	
		}

		pkTemp = pkPageControl->GetControl( _T("BTN_TO_END") );
		if ( pkTemp )
		{
			pkTemp->IsClosed(kKeyPage.iPage==iLastPage);
			int iTemp = iControlStartPage + 5;
			if ( iTemp < iLastPage )
			{
				pkTemp->SetCustomData( &iTemp, sizeof(int) );
			}
			else
			{
				pkTemp->SetCustomData( &iLastPage, sizeof(int) );
			}	
		}

		//1, 2, 3, 4, 5
		for( int i=0; i!=5; ++i )
		{
			::swprintf_s( wszTemp, MAX_PATH, L"BTN_NUM_%d", i );
			XUI::CXUI_Button* pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPageControl->GetControl( std::wstring(wszTemp) ));
			if ( pkButton )
			{
				int iSet = (int)iControlStartPage + i;

				pkButton->Visible( iSet <= iLastPage );
				if ( kKeyPage.iPage == iSet )
				{
					pkButton->FontColor(0xFF0000FF);
					pkButton->IsClosed(true);
				}
				else
				{
					pkButton->FontColor(0xFFFFFFFF);
					pkButton->IsClosed(false);
					pkButton->SetCustomData(&iSet, sizeof(iSet));
				}
				::swprintf_s(wszTemp,MAX_PATH,L"%d", iSet );
				pkButton->Text(std::wstring(wszTemp));
			}
		}
	}
	return true;
}

bool PgRankingMgr::Parse( TiXmlElement const *pkElement, CONT_RANKDATA::mapped_type::value_type &kData )
{
	while ( pkElement )
	{
		if ( !strcmp( pkElement->Value(), "CHARACTERID") )
		{
			if ( pkElement->GetText() )
			{
				kData.kCharGuid.SetUniqueidentifier( pkElement->GetText() );
			}		
		}
		else if ( !strcmp( pkElement->Value(), "NAME") )
		{
			if ( pkElement->GetText() )
			{
				wchar_t wszTemp[MAX_CHARACTER_NAME_LEN+1] = {0,};
				MultiByteToWideChar( CP_UTF8, 0, pkElement->GetText(), strlen(pkElement->GetText()), wszTemp, MAX_CHARACTER_NAME_LEN);
				kData.wstrName = wszTemp;
			}
		}
		else if ( !strcmp( pkElement->Value(), "GUILDNAME") )
		{
			if ( pkElement->GetText() )
			{
				wchar_t wszTemp[MAX_CHANNEL_NAME+1] = {0,};
				MultiByteToWideChar( CP_UTF8, 0, pkElement->GetText(), strlen(pkElement->GetText()), wszTemp, MAX_CHANNEL_NAME);
				kData.wstrGuildName = wszTemp;
			}
		}
		else if ( !strcmp( pkElement->Value(), "VALUE") )
		{
			if ( pkElement->GetText() )
			{
				kData.iValue = ::_atoi64( pkElement->GetText());
			}
		}
		else if ( !strcmp( pkElement->Value(), "CLASS") )
		{
			if ( pkElement->GetText() )
			{
				kData.usClass = (WORD)::atol( pkElement->GetText());
			}
		}
		else if ( !strcmp( pkElement->Value(), "LV") )
		{
			if ( pkElement->GetText() )
			{
				kData.usLevel = (WORD)::atol( pkElement->GetText());
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

HRESULT PgRankingMgr::GetHttpData( CONT_RANKDATA::key_type const &kKey, CONT_RANKDATA::mapped_type &kCont )
{
	HRESULT hRet = E_FAIL;
	HINTERNET hInternet = ::InternetOpen( _T("RANKING"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( hInternet )
	{
		TCHAR szURL[MAX_PATH] = {0,};
		::swprintf_s( szURL, MAX_PATH, ms_szURL, kKey.iType, kKey.iClass, g_kNetwork.NowRealmNo() );
		HINTERNET hHttp = ::InternetOpenUrl( hInternet, szURL, NULL, 0, 0, 0 );
		if ( hHttp )
		{
			hRet = E_ABORT;
//			SetLastError(0);
			std::vector<char> kBuffer;

			DWORD dwOldSize = 0;
			DWORD dwBytesRead = 0;
			do 
			{
				DWORD dwBytesAvailable = 0;
				if ( !::InternetQueryDataAvailable( hHttp, &dwBytesAvailable, 0, 0) || !dwBytesAvailable )
				{
					break;
				}

				kBuffer.resize( dwOldSize+dwBytesAvailable );
				if ( !::InternetReadFile( hHttp, &(kBuffer.at(dwOldSize)), dwBytesAvailable, &dwBytesRead ) )
				{
					break;
				}
				dwOldSize += dwBytesRead;
			} while ( dwBytesRead );

			if ( !kBuffer.empty() )
			{
				TiXmlDocument kXmlDoc;
				kXmlDoc.Parse( &kBuffer.at(0), NULL, TIXML_ENCODING_UTF8 );
				if ( !kXmlDoc.Error() )
				{
					size_t iRankCount = 0;
					char *pTemp = NULL;
					TiXmlElement const *pkElement = kXmlDoc.FirstChildElement();
					while ( pkElement )
					{
						if ( !strcmp( pkElement->Value(), "RANKING") )
						{
							TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();
							while ( pkAttr )
							{
								if ( !strcmp( pkAttr->Name(),"TOTALCOUNT" ) )
								{
									iRankCount = (size_t)(::strtol( pkAttr->Value(), &pTemp, 10 ) );
								}
								else if ( !strcmp( pkAttr->Name(),"DATE" ) )
								{

								}
								pkAttr = pkAttr->Next();
							}

							pkElement = pkElement->FirstChildElement();
						}
						else if ( !strcmp( pkElement->Value(), "ITEMS") )
						{
							CONT_RANKDATA::mapped_type::value_type kData;
							if ( Parse( pkElement->FirstChildElement(), kData ) )
							{
								if ( kCont.empty() )
								{
									kData.usRank = 1;
								}
								else
								{
									if ( kCont.back().iValue == kData.iValue )
									{
										kData.usRank = kCont.back().usRank;
									}
									else
									{
										kData.usRank = (WORD)(kCont.size()+1);
									}
								}
								kCont.push_back( kData );
							}
							pkElement = pkElement->NextSiblingElement();
						}
					}

					if ( iRankCount == kCont.size() )
					{
						hRet = S_OK;
					}
				}
			}
			InternetCloseHandle(hHttp);
		}
		InternetCloseHandle(hInternet);
	}
	return hRet;
}