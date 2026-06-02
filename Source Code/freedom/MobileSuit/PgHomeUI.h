#ifndef FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMEUI_H
#define FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMEUI_H

#include "PgPage.h"

namespace PgHomeUIUtil
{
	enum E_TOWN_BOARD_SORT_STATE
	{
		ETBSS_ALL_ITEM			= 0,
		ETBSS_VISIT_OPEN_ITEM	= 1,
		ETBSS_VISIT_CLOSE_ITEM	= 2,
		ETBSS_AUCTION_REG_ITEM	= 3,
		ETBSS_BIDDING_ITEM		= 4,
		ETBSS_END,
	};

	enum E_TOWN_LIST_REQ_TYPE : BYTE
	{
		ETLRT_OPEN_BOARD	= 0,
		ETLRT_OPEN_MINIMAP	= 1,
		ETLRT_OPEN_SIDE_JOB_BOARD = 2,
		ETLRT_OPEN_MYHOME_ENTER = 3,
		ETLRT_OPEN_BUILDING_NAME_BOARD = 4,
	};

	enum E_HOME_CHECK_RESULT
	{
		EHCR_ALL_USER_OPEN	= 0,
		EHCR_COUPLE_OPEN	= 1,
		EHCR_GUILD_OPEN		= 2,
		EHCR_FRIEND_OPEN	= 3,
		EHCR_CLOSE,
	};

	enum E_SIDE_JOB_SORT_STATE
	{
		ESJSS_ALL_ITEM			= 0,
		ESJSS_REPAIR			= 1,
		ESJSS_ENCHANT_GOLD		= 2,
		ESJSS_ENCHANT_RATE		= 3,
		ESJSS_SOUL_GOLD			= 4,
		ESJSS_SOUL_COUNT		= 5,
		ESJSS_SOCKET_GOLD		= 6,
		ESJSS_SOCKET_RATE		= 7,
		ESJSS_END,
	};
	E_HOME_CHECK_RESULT const CheckToHomeVisitPossible(SMYHOME const& kHome);
	bool CheckMyHome(SMYHOME const& kHome);
};

typedef struct tagHOMESIDEJOB
{
	tagHOMESIDEJOB():kSideJob(MSJ_NONE), kSideJobType(MSJRT_NONE), fJobRate(0) {}
	explicit tagHOMESIDEJOB(eMyHomeSideJob _kJob, eMyHomeSideJobRateType _kType, float _fRate, SMYHOME _kHome):
		kSideJob(_kJob), kSideJobType(_kType), fJobRate(_fRate), kHome(_kHome) {}

	eMyHomeSideJob	kSideJob;
	eMyHomeSideJobRateType kSideJobType;
	float fJobRate;
	SMYHOME kHome;

}SHOMESIDEJOB;

typedef std::vector<SHOMESIDEJOB>	CONT_SIDEJOB;

class PgHomeUI
{
public:
	typedef std::set< short > CONT_TOWNNO;

	//TownBoard
	bool ReadTownBoardItem(BM::Stream& rkPacket);
	void GetTownBoardDisplayItem(CONT_MYHOME& kHomeList);
	void GetTownHouseCont(short const siStreetNo, CONT_MYHOME& kHomeList);
	bool SetTownBoardDisplayTownNo(short const iTownNo);
	bool SetTownBoardDisplayHomeStateType(int const iStateType);
	bool GetHomeInfo(short const iStreetNo, int const iHouseNo, SMYHOME& kHomeInfo) const;
	bool UpdateHomeInfo(short const iStreetNo, int const iHouseNo, SMYHOME const& kHomeInfo);
	bool UpdateHomeInfo(short const iStreetNo, int const iHouseNo, __int64 BiddingCost);
	void ClearSortInfo();
	void SetMyHomeInfo(SMYHOME const& kHomeInfo) { m_kMyHomeInfo = kHomeInfo; }
	SMYHOME const& GetMyHomeInfo() { return m_kMyHomeInfo; }

	//MyHome
	bool ReadHomeVisitorItem(BM::Stream& rkPacket);
	bool GetHomeVisitorItem(CONT_VISITORINFO& kVisitorList);
	bool DeleteHomeVisitorItem(BM::GUID const& kGuid);

	//Home
	bool ReadHomeInviteItem(BM::Stream& rkPacket);
	bool GetHomeInviteItem(CONT_INVITATIONCARD& kVisitorList);

	CONT_TOWNNO const& TownNoList() const { return m_kTownNo; }
	PgPage& GetBoardPage() { return m_kBoardPage; };
	PgPage& GetInvitePage() { return m_kInvitePage; };
	PgPage& GetSideJobBoardPage() { return m_kSideJobBoardPage; };

	void SetSideJobSortList();
	void ClearSideJobSortList();
	void GetSideJobBoardDisplayItem(CONT_SIDEJOB& kHomeList);
	bool SetSideJobBoardDisplayTownNo(short const iTownNo);
	bool SetSideJobBoardDisplaySortType(int const iType);

	//Test
	bool Test_AddTownBoardItem(SHOMEADDR const & kAddr);

	PgHomeUI(void);
	~PgHomeUI(void);
protected:
	void Update();

protected:
	CONT_MYHOME	m_kHomeContainer;
	CONT_MYHOME m_kHomeSortCache;
	SMYHOME m_kMyHomeInfo;
	CONT_TOWNNO m_kTownNo;
	SHOMEADDR	m_kSortKey;
	PgHomeUIUtil::E_TOWN_BOARD_SORT_STATE m_kSortState;	

	CONT_SIDEJOB m_kSideJobSortCont;
	SHOMEADDR	m_kSideJobSortKey;
	PgHomeUIUtil::E_SIDE_JOB_SORT_STATE m_kSideJobSortState;	


	CONT_VISITORINFO m_kHomeVisitors;
	CONT_INVITATIONCARD m_kInviteList;

	CLASS_DECLARATION_S(bool, UseCashShop);
	CLASS_DECLARATION(__int64, m_i64FirstBiddingCost, MyHomeFirstBiddingCost);

	PgPage	m_kBoardPage;
	PgPage	m_kInvitePage;
	PgPage	m_kSideJobBoardPage;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMEUI_H