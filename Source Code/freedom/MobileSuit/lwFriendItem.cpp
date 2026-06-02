#include "stdafx.h"
#include "lwFriendItem.h"

lwFriendItem::lwFriendItem(SFriendItem& rkFriendItem)
{
	m_kFriendItem = rkFriendItem;
}

bool lwFriendItem::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwFriendItem>(pkState, "FriendItem")
		.def(pkState, constructor<SFriendItem&>())

		.def(pkState, "CharacterName", &lwFriendItem::CharacterName)
		.def(pkState, "GroupName", &lwFriendItem::GroupName)
		.def(pkState, "SetGroupName", &lwFriendItem::SetGroupName)
		.def(pkState, "Guid", &lwFriendItem::Guid)
	
		.def(pkState, "IsOnline", &lwFriendItem::IsOnline)
		.def(pkState, "IsOffline", &lwFriendItem::IsOffline)
		.def(pkState, "IsLinkStatus", &lwFriendItem::IsLinkStatus)

		.def(pkState, "Location", &lwFriendItem::Location)

		.def(pkState, "IsBlock_HeCanTalk", &lwFriendItem::IsBlock_HeCanTalk)
		.def(pkState, "IsBlock_HeCanListen", &lwFriendItem::IsBlock_HeCanListen)

		.def(pkState, "IsHaveHome", &lwFriendItem::IsHaveHome)
		.def(pkState, "SetMyHomeStatus", &lwFriendItem::SetMyHomeStatus)
		
		.def(pkState, "IsRequester", &lwFriendItem::IsRequester)//요청자냐?
		.def(pkState, "IsAccepter", &lwFriendItem::IsAccepter)

		.def(pkState, "IsNil", &lwFriendItem::IsNil)

		.def(pkState, "Level", &lwFriendItem::Level)
		.def(pkState, "Channel", &lwFriendItem::Channel)
		.def(pkState, "Class", &lwFriendItem::Class)
		;

	return true;
}

lwWString lwFriendItem::CharacterName()
{
	return lwWString(m_kFriendItem.ExtendName());
}
lwWString lwFriendItem::GroupName()
{
	return lwWString(m_kFriendItem.GroupName());
}

void lwFriendItem::SetGroupName(char const* szName)
{
	if (NULL != szName)
	{
		m_kFriendItem.GroupName(UNI(szName));
	}
}
lwGUID lwFriendItem::Guid()
{
	return lwGUID(m_kFriendItem.CharGuid());
}

int lwFriendItem::Location()const
{
	return m_kFriendItem.GroundKey().GroundNo();
}

bool lwFriendItem::IsOnline()
{
	return (m_kFriendItem.LinkStatus() != FLS_OFFLINE);
}
bool lwFriendItem::IsOffline()
{
	return (m_kFriendItem.LinkStatus() == FLS_OFFLINE);
}
int lwFriendItem::IsLinkStatus()
{
	return (int)m_kFriendItem.LinkStatus();
}
bool lwFriendItem::IsBlock_HeCanTalk()//(그사람) 말하는걸 막는다
{
	return (m_kFriendItem.ChatStatus() & FCS_BLOCK_NO_RECV) == FCS_BLOCK_NO_RECV;
}
bool lwFriendItem::IsBlock_HeCanListen()//(내말) 듣는걸 막는다
{
	return (m_kFriendItem.ChatStatus() & FCS_BLOCK_NO_SEND) == FCS_BLOCK_NO_SEND;
}

bool lwFriendItem::IsHaveHome()
{
	return !(m_kFriendItem.HomeAddr().IsNull());
}

bool lwFriendItem::SetMyHomeStatus(lwUIWnd kWnd)
{
	if( kWnd.IsNil() )
	{
		return false;
	}
	XUI::CXUI_Wnd* pkParentWnd = kWnd();
	if(!pkParentWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkMyHomeGoWnd = pkParentWnd->GetControl( L"BTN_MYHOME_GO_NONE" );
	if( !pkMyHomeGoWnd )
	{
		return false;
	}
	XUI::CXUI_Button* pkMyHomeGoBtn = dynamic_cast<XUI::CXUI_Button*>( pkMyHomeGoWnd );
	if( !pkMyHomeGoBtn )
	{
		return false;
	}
	pkMyHomeGoBtn->Disable( m_kFriendItem.HomeAddr().IsNull() );
	return true;
}

bool lwFriendItem::IsRequester()//요청자 인가?
{
	return (0 != (m_kFriendItem.ChatStatus()&FCS_ADD_WAIT_ANS));//허가대기중 인 사람이다
}
bool lwFriendItem::IsAccepter()
{
	return (0 != (m_kFriendItem.ChatStatus()&FCS_ADD_WAIT_REQ));//내가 추가 했고 수락할 사람이다.
}

//if(ECFS_FriendStatus_AddWait_Req==(kFriendItem.cChatStatus&ECFS_FriendStatus_AddWait_Req))//수락대기중(요청자)
//else if(ECFS_FriendStatus_AddWait_Ans==(kFriendItem.cChatStatus&ECFS_FriendStatus_AddWait_Ans))//허가대기중(수락자)

bool lwFriendItem::IsNil()
{
	return (m_kFriendItem.CharGuid() == BM::GUID::NullData() && m_kFriendItem.GroupName().empty());
}

int lwFriendItem::Level()
{
	return m_kFriendItem.Level();
}

int lwFriendItem::Channel()
{
	return m_kFriendItem.Channel();
}

int lwFriendItem::Class()
{
	return m_kFriendItem.Class();
}