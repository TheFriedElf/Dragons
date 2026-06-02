#include "StdAfx.h"
#include "PgHomeUI.h"
#include "PgGuild.h"
#include "PgCoupleMgr.h"
#include "PgFriendMgr.h"
#include "PgPilotMan.h"
#include "lwHomeUI_Script.h"
#include "PgHomeRenew.h"

namespace PgHomeUIUtil
{
	E_HOME_CHECK_RESULT const CheckToHomeVisitPossible(SMYHOME const& kHome)
	{
		if( kHome.bEnableVisitBit != MEV_ONLY_OWNER )
		{
			if( (kHome.bEnableVisitBit & MEV_ALL) == MEV_ALL )
			{
				return EHCR_ALL_USER_OPEN;
			}
			if( (kHome.bEnableVisitBit & MEV_COUPLE) == MEV_COUPLE )
			{
				SCouple kMyCouple = g_kCoupleMgr.GetMyInfo();

				if( kMyCouple.CoupleGuid() != BM::GUID::NullData()
				&& kMyCouple.CoupleGuid() == kHome.kOwnerGuid )
				{
					return EHCR_COUPLE_OPEN;
				}
			}
			if( (kHome.bEnableVisitBit & MEV_GUILD) == MEV_GUILD )
			{
				SGuildMemberInfo kTemp;
				if( g_kGuildMgr.IamHaveGuild()
				&& g_kGuildMgr.GetMemberByGuid( kHome.kOwnerGuid, kTemp ) )
				{
					return EHCR_GUILD_OPEN;
				}
			}
			if( (kHome.bEnableVisitBit & MEV_FRIEND) == MEV_FRIEND )
			{
				SFriendItem kTemp;
				if( g_kFriendMgr.Friend_Find_ByGuid( kHome.kOwnerGuid, kTemp ) )
				{
					return EHCR_FRIEND_OPEN;
				}
			}
		}
		return EHCR_CLOSE;
	}

	bool CheckMyHome(SMYHOME const& kHome)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		return g_kPilotMan.IsMyPlayer(kHome.kOwnerGuid) && (pkPlayer->HomeAddr().StreetNo() == kHome.siStreetNo && pkPlayer->HomeAddr().HouseNo() == kHome.iHouseNo);
	}
};

PgHomeUI::PgHomeUI(void)
{
	m_kHomeContainer.clear();
	m_kHomeSortCache.clear();
	m_kSideJobSortCont.clear();
	m_kTownNo.clear();
	m_kSortKey.StreetNo(0);
	m_kSortKey.HouseNo(0);
	m_kHomeVisitors.clear();
	m_kInviteList.clear();

	UseCashShop(false);

	m_kBoardPage.SetPageAttribute(15, 5);
	m_kInvitePage.SetPageAttribute(8, 5);
	m_kSideJobBoardPage.SetPageAttribute(15, 5);
}

PgHomeUI::~PgHomeUI(void)
{
}

bool PgHomeUI::ReadTownBoardItem(BM::Stream& rkPacket)
{
	CONT_MYHOME kTemp;
	kTemp.clear();
	bool const bResult = PU::TLoadTable_AM(rkPacket, kTemp);
	if( bResult )
	{
		m_kHomeContainer.clear();
		m_kHomeContainer.insert(kTemp.begin(), kTemp.end());
		ClearSortInfo();
		ClearSideJobSortList();
		SetSideJobSortList();
		Update();
	}
	return bResult;
}


bool PgHomeUI::Test_AddTownBoardItem(SHOMEADDR const & kAddr)
{
	SMYHOME kHome;
	kHome.siStreetNo = kAddr.StreetNo();
	kHome.iHouseNo = kAddr.HouseNo();
	kHome.i64HomePrice = BM::Rand_Range(1000,1);
	kHome.iLvLimitMin = 0;
	kHome.iLvLimitMax = BM::Rand_Range(30,0);
	kHome.kName = L"TEST";

	m_kHomeContainer.insert(std::make_pair(kAddr, kHome));
	ClearSortInfo();
	ClearSideJobSortList();
	SetSideJobSortList();
	Update();
	return true;
}

void PgHomeUI::GetTownBoardDisplayItem(CONT_MYHOME& kHomeList)
{
	CONT_MYHOME::const_iterator home_itor = m_kHomeSortCache.begin();
	int const ignoreCount = m_kBoardPage.Now() * m_kBoardPage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( home_itor != m_kHomeSortCache.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++home_itor;
			continue;
		}

		kHomeList.insert(std::make_pair(home_itor->first, home_itor->second));
		++home_itor;

		if( (iLoopCount - ignoreCount) == m_kBoardPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}

void PgHomeUI::GetTownHouseCont(short const siStreetNo, CONT_MYHOME& kHomeList)
{
	CONT_MYHOME::const_iterator town_itor = m_kHomeContainer.begin();
	while( town_itor != m_kHomeContainer.end() )
	{
		CONT_MYHOME::key_type const& kAddr = town_itor->first;

		if( kAddr.StreetNo() == siStreetNo )
		{
			kHomeList.insert(std::make_pair(town_itor->first, town_itor->second));
		}
		++town_itor;
	}
}

bool PgHomeUI::SetTownBoardDisplayTownNo(short const iTownNo)
{
	CONT_TOWNNO::iterator townNo_itor = m_kTownNo.find( iTownNo );
	if( iTownNo == 0 || townNo_itor != m_kTownNo.end() )
	{
		m_kSortKey.StreetNo(iTownNo);
		Update();
		return true;
	}
	return false;
}

bool PgHomeUI::SetTownBoardDisplayHomeStateType(int const iStateType)
{
	if( PgHomeUIUtil::ETBSS_ALL_ITEM <= iStateType || PgHomeUIUtil::ETBSS_END > iStateType )
	{
		m_kSortState = static_cast<PgHomeUIUtil::E_TOWN_BOARD_SORT_STATE>(iStateType);
		Update();
		return true;
	}
	return false;
}

bool PgHomeUI::SetSideJobBoardDisplayTownNo(short const iTownNo)
{
	CONT_TOWNNO::iterator townNo_itor = m_kTownNo.find( iTownNo );
	if( iTownNo == 0 || townNo_itor != m_kTownNo.end() )
	{
		m_kSideJobSortKey.StreetNo(iTownNo);
		SetSideJobSortList();
		return true;
	}
	return false;
}

bool PgHomeUI::SetSideJobBoardDisplaySortType(int const iType)
{
	if( PgHomeUIUtil::ESJSS_ALL_ITEM <= iType || PgHomeUIUtil::ESJSS_END > iType )
	{
		m_kSideJobSortState = static_cast<PgHomeUIUtil::E_SIDE_JOB_SORT_STATE>(iType);
		SetSideJobSortList();
		return true;
	}
	return false;
}

bool PgHomeUI::GetHomeInfo(short const iStreetNo, int const iHouseNo, SMYHOME& kHomeInfo) const
{
	CONT_MYHOME::const_iterator home_itor = m_kHomeContainer.find(SHOMEADDR(iStreetNo, iHouseNo));
	if( home_itor != m_kHomeContainer.end() )
	{
		kHomeInfo = home_itor->second;
		return true;
	}

	return false;
}

bool PgHomeUI::UpdateHomeInfo(short const iStreetNo, int const iHouseNo, SMYHOME const& kHomeInfo)
{
	CONT_MYHOME::iterator home_itor = m_kHomeContainer.find(SHOMEADDR(iStreetNo, iHouseNo));
	if( home_itor != m_kHomeContainer.end() )
	{
		home_itor->second = kHomeInfo;
		CONT_MYHOME::iterator cache_itor = m_kHomeSortCache.find(SHOMEADDR(iStreetNo, iHouseNo));
		if( cache_itor != m_kHomeSortCache.end() )
		{
			cache_itor->second = kHomeInfo;
			return true;
		}
	}
	return false;
}

bool PgHomeUI::UpdateHomeInfo(short const iStreetNo, int const iHouseNo, __int64 BiddingCost)
{
	CONT_MYHOME::iterator home_itor = m_kHomeContainer.find(SHOMEADDR(iStreetNo, iHouseNo));
	if( home_itor != m_kHomeContainer.end() )
	{
		home_itor->second.i64LastBiddingCost = BiddingCost;

		CONT_MYHOME::iterator cache_itor = m_kHomeSortCache.find(SHOMEADDR(iStreetNo, iHouseNo));
		if( cache_itor != m_kHomeSortCache.end() )
		{
			cache_itor->second.i64LastBiddingCost = BiddingCost;
			return true;
		}
	}
	return false;
}

void PgHomeUI::ClearSortInfo()
{
	m_kSortKey.StreetNo(0);
	m_kSortKey.HouseNo(0);
	m_kSortState = PgHomeUIUtil::ETBSS_ALL_ITEM;
	m_kHomeSortCache.clear();
}

void PgHomeUI::Update()
{
	m_kHomeSortCache.clear();

	CONT_MYHOME kTemp;
	if( 0 == m_kSortKey.StreetNo() )
	{
		kTemp.insert(m_kHomeContainer.begin(), m_kHomeContainer.end());
		CONT_MYHOME::iterator itr = kTemp.begin();
		while( itr != kTemp.end() )
		{
			if( g_kHomeRenewMgr.IsMyhomeApt(itr->second.siStreetNo) )
			{
				itr = kTemp.erase(itr);
			}
			else
			{
				++itr;
			}
		}
		
	}
	else
	{
		CONT_MYHOME::const_iterator home_itor = m_kHomeContainer.begin();
		while( home_itor != m_kHomeContainer.end() )
		{
			CONT_MYHOME::key_type const& kKeyInfo = home_itor->first;

			if( kKeyInfo.StreetNo() == m_kSortKey.StreetNo() )
			{
				kTemp.insert(std::make_pair(home_itor->first, home_itor->second));
			}
			++home_itor;
		}
	}

	if( PgHomeUIUtil::ETBSS_ALL_ITEM != m_kSortState )
	{
		CONT_MYHOME::iterator cache_home_itor = kTemp.begin();
		while( cache_home_itor != kTemp.end() )
		{
			CONT_MYHOME::mapped_type const& kHomeInfo = cache_home_itor->second;

			switch( m_kSortState )
			{
			case PgHomeUIUtil::ETBSS_VISIT_OPEN_ITEM:
				{
					if( MAS_IS_BIDDING == kHomeInfo.bAuctionState
					||(!g_kPilotMan.IsMyPlayer(kHomeInfo.kOwnerGuid) 
					&&  PgHomeUIUtil::EHCR_CLOSE == PgHomeUIUtil::CheckToHomeVisitPossible( kHomeInfo ) ) )
					{
						cache_home_itor = kTemp.erase(cache_home_itor);
						continue;
					}
				}break;
			case PgHomeUIUtil::ETBSS_VISIT_CLOSE_ITEM:
				{
					if(g_kPilotMan.IsMyPlayer(kHomeInfo.kOwnerGuid)
					|| MAS_IS_BIDDING == kHomeInfo.bAuctionState 
					|| PgHomeUIUtil::EHCR_CLOSE != PgHomeUIUtil::CheckToHomeVisitPossible( kHomeInfo ) )
					{
						cache_home_itor = kTemp.erase(cache_home_itor);
						continue;
					}
				}break;
			case PgHomeUIUtil::ETBSS_AUCTION_REG_ITEM:
				{
					if(!g_kPilotMan.IsMyPlayer(kHomeInfo.kOwnerGuid)
					||MAS_IS_BIDDING != kHomeInfo.bAuctionState )
					{
						cache_home_itor = kTemp.erase(cache_home_itor);
						continue;
					}
				}break;
			case PgHomeUIUtil::ETBSS_BIDDING_ITEM:
				{
					if( g_kPilotMan.IsMyPlayer(kHomeInfo.kOwnerGuid)
					||	MAS_IS_BIDDING != kHomeInfo.bAuctionState )
					{
						cache_home_itor = kTemp.erase(cache_home_itor);
						continue;
					}
				}break;
			}
			
			++cache_home_itor;
		}
	}

	m_kHomeSortCache.insert(kTemp.begin(), kTemp.end());
	m_kBoardPage.SetMaxItem(m_kHomeSortCache.size());
}


bool SideJobListOrder(SHOMESIDEJOB const& rhs, SHOMESIDEJOB const lhs)
{
	return ( rhs.fJobRate > lhs.fJobRate );
}

void PgHomeUI::SetSideJobSortList()
{
	m_kSideJobSortCont.clear();

	CONT_MYHOME::const_iterator home_itor = m_kHomeContainer.begin();
	while( home_itor != m_kHomeContainer.end() )
	{
		CONT_MYHOME::key_type const& kKeyInfo = home_itor->first;

		if( m_kSideJobSortKey.StreetNo() == 0 || kKeyInfo.StreetNo() == m_kSideJobSortKey.StreetNo() )// 마을 필터링
		{
			SMYHOME kMyHome = home_itor->second;
			if( kMyHome.SideJobFlag() )
			{
				CONT_MYHOME_SIDE_JOB kContSideJob = kMyHome.kContSideJob;
				for(CONT_MYHOME_SIDE_JOB::const_iterator job_iter = kContSideJob.begin();job_iter != kContSideJob.end();++job_iter)
				{
					PgHomeUIUtil::E_SIDE_JOB_SORT_STATE kSortState = lwHomeUI::GetSideJobType(job_iter->first, job_iter->second.kJobRateType);
					if( PgHomeUIUtil::ESJSS_ALL_ITEM == m_kSideJobSortState || m_kSideJobSortState == kSortState )// 아르바이트 필터링
					{
						float fJobRate = static_cast<float>(job_iter->second.iJobRate) / 100.0f * static_cast<float>(kMyHome.iEquipItemCount);
						SHOMESIDEJOB kHomeSideJob(job_iter->first, job_iter->second.kJobRateType, fJobRate, kMyHome);
						m_kSideJobSortCont.push_back(kHomeSideJob);
					}
				}
			}
		}
		++home_itor;
	}

	std::sort( m_kSideJobSortCont.begin(), m_kSideJobSortCont.end(), SideJobListOrder);
	m_kSideJobBoardPage.SetMaxItem(m_kSideJobSortCont.size());
}

void PgHomeUI::GetSideJobBoardDisplayItem(CONT_SIDEJOB& kHomeList)
{
	CONT_SIDEJOB::const_iterator home_itor = m_kSideJobSortCont.begin();
	int const ignoreCount = m_kSideJobBoardPage.Now() * m_kSideJobBoardPage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( home_itor != m_kSideJobSortCont.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++home_itor;
			continue;
		}

		kHomeList.push_back(*home_itor);
		++home_itor;

		if( (iLoopCount - ignoreCount) == m_kSideJobBoardPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}

void PgHomeUI::ClearSideJobSortList()
{
	m_kSideJobSortKey.StreetNo(0);
	m_kSideJobSortKey.HouseNo(0);
	m_kSideJobSortState = PgHomeUIUtil::ESJSS_ALL_ITEM;
	m_kSideJobSortCont.clear();
}

bool PgHomeUI::ReadHomeVisitorItem(BM::Stream& rkPacket)
{
	CONT_VISITORINFO kTemp;
	kTemp.clear();
	bool const bResult = PU::TLoadArray_M(rkPacket, kTemp);
	if( bResult )
	{
		m_kHomeVisitors.clear();
		m_kHomeVisitors.insert(m_kHomeVisitors.end(), kTemp.begin(), kTemp.end());
	}
	return bResult;
}

bool PgHomeUI::GetHomeVisitorItem(CONT_VISITORINFO& kVisitorList)
{
	if( m_kHomeVisitors.empty() )
	{
		return false;
	}

	kVisitorList.insert(kVisitorList.end(), m_kHomeVisitors.begin(), m_kHomeVisitors.end());
	return true;
}

bool PgHomeUI::DeleteHomeVisitorItem(BM::GUID const& kGuid)
{
	CONT_VISITORINFO::iterator visitor_itor = m_kHomeVisitors.begin();
	while( visitor_itor != m_kHomeVisitors.end() )
	{
		CONT_VISITORINFO::value_type const& kVisitorInfo = (*visitor_itor);

		if( kVisitorInfo.kOwnerGuid == kGuid )
		{
			m_kHomeVisitors.erase(visitor_itor);
			return true;
		}

		++visitor_itor;
	}
	return false;
}


bool PgHomeUI::ReadHomeInviteItem(BM::Stream& rkPacket)
{
	CONT_INVITATIONCARD kTemp;
	kTemp.clear();
	bool const bResult = PU::TLoadArray_M(rkPacket, kTemp);
	if( bResult )
	{
		m_kInviteList.clear();
		m_kInviteList.insert(m_kInviteList.end(), kTemp.begin(), kTemp.end());
		m_kInvitePage.SetMaxItem(m_kInviteList.size());
	}
	return bResult;
}

bool PgHomeUI::GetHomeInviteItem(CONT_INVITATIONCARD& kInviteList)
{
	if( m_kInviteList.empty() )
	{
		return false;
	}

	CONT_INVITATIONCARD::const_iterator invite_itor = m_kInviteList.begin();
	int const ignoreCount = m_kInvitePage.Now() * m_kInvitePage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( invite_itor != m_kInviteList.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++invite_itor;
			continue;
		}

		kInviteList.insert(kInviteList.end(), (*invite_itor));
		++invite_itor;

		if( (iLoopCount - ignoreCount) == m_kInvitePage.GetMaxItemSlot() )
		{
			break;
		}
	}

	return true;
}

//void PgHomeUI::ReserveEvent(PgHomeUIUtil::SEvent const& eEvent)
//{//Home 정보가 갱신되면 호출될 UI를 등록
//	m_kContEvent.insert(eEvent);
//}
//
//bool PgHomeUI::GetReservedEvent(SHOMEADDR const& kAddr, PgHomeUIUtil::SEvent& eEvent)
//{//Home 정보가 갱신될때 등록된 Event를 수행
//	CONT_EVENT::iterator evt_itor = m_kContEvent.find(PgHomeUIUtil::SEvent(kAddr));
//	if( evt_itor != m_kContEvent.end() )
//	{
//		eEvent = (*evt_itor);
//		m_kContEvent.erase(evt_itor);
//		return true;
//	}
//	return false;
//}