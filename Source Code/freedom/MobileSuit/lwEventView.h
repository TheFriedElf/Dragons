#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTVIEW_LWEVENTVIEW_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTVIEW_LWEVENTVIEW_H

#include "lwUI.h"
#include "variant/PgCouponEventView.h"

namespace lwEventView
{
	enum eEVENT_TYPE
	{
		EVT_COUPON	= 0,
		EVT_NEWTYPE	= 1,
	};

	void RegisterWrapper(lua_State *pkState);

	void lwCallEventViewUI();
	void lwViewSelectEventInfo(lwUIWnd UISelf);
	void lwGetEventItem(lwUIWnd UISelf);

	void ReceivePacket(WORD const wPacketType, BM::Stream& rkPacket);

	void AllClearEventUI();
	void ClearUIEventInfo();
	int const SetEventList(XUI::CXUI_Wnd* pParent);
	bool ViewSelectEventInfo(int const& EventKey, XUI::CXUI_Wnd* pParent, bool bIsNewType);
	bool ViewSelectEventInfoNewType(int const& EventKey, XUI::CXUI_Wnd* pParent);

	std::wstring const GetTimeToString(BM::PgPackedTime const& kTimeInfo);
}

typedef std::map< int, std::wstring >	CONT_EVENT_TINY_INFO_LIST;
typedef std::map< int, CONT_COUPON_EVENT_REWARD::const_iterator > CONT_EVENT_REWARD_LIST;

class PgEventViewClient
	: public PgCouponEventView
{
public:
	void ReadFromPacket(BM::Stream& kPacket);
	bool GetActivatedCouponEvent(CONT_EVENT_TINY_INFO_LIST& List);
	bool GetKeyMatchInfo(int const& EventKey, SCOUPON_EVENT_INFO& Info, CONT_EVENT_REWARD_LIST& Reward);

	void ReadFromPacketToServerEvent(BM::Stream& kPacket);
	bool GetActivatedServerEvent(CONT_EVENT_TINY_INFO_LIST& List);
	bool GetKeyMatchInfo(int const& EventKey, SCOUPON_EVENT_INFO& Info, TBL_EVENT& evt);

	PgEventViewClient(){};
	virtual ~PgEventViewClient(){};

protected:
	typedef struct tagEventContIterator
	{
		CONT_COUPON_EVENT_INFO::iterator	Info_iter;
		CONT_EVENT_REWARD_LIST	RewardList;
	}SEventContIterator;

	typedef std::map< int, SEventContIterator >	CONT_CP_EVENT_CACHE;
	CONT_CP_EVENT_CACHE	m_kEventCache;

	CONT_EVENT_LIST	m_kServerEventContainer;
};

#define g_kEventViewClient SINGLETON_STATIC(PgEventViewClient)

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTVIEW_LWEVENTVIEW_H