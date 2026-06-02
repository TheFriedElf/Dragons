#include "stdafx.h"

#include "lwFriendItem.h"
#include "lwFriendMgr.h"
#include "lwUI.h"
#include "lwGuildUI.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgChatMgrClient.h"

lwFriendMgr::lwFriendMgr(PgFriendMgr* pkFriendMgr)
{
	m_pkFriendMgr = pkFriendMgr;
}


//! 스크립팅 시스템에 등록한다.
bool lwFriendMgr::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwFriendMgr>(pkState, "FriendMgr")
		.def(pkState, constructor<PgFriendMgr *>())

		.def(pkState, "SendFriend_Command_AddByName", &lwFriendMgr::SendFriend_Command_AddByName)
		.def(pkState, "SendFriend_Command_AddByGuid", &lwFriendMgr::SendFriend_Command_AddByGuid)
		.def(pkState, "SendFriend_Command_AddGroup", &lwFriendMgr::SendFriend_Command_AddGroup)

		.def(pkState, "SendFriend_Command_Delete", &lwFriendMgr::SendFriend_Command_Delete)
		.def(pkState, "SendFriend_Command_DeleteGroup", &lwFriendMgr::SendFriend_Command_DeleteGroup)

		.def(pkState, "SendFriend_Command_ModifyGroup", &lwFriendMgr::SendFriend_Command_ModifyGroup)
		.def(pkState, "SendFriend_Command_ModifyGroupName", &lwFriendMgr::SendFriend_Command_ModifyGroupName)

		.def(pkState, "Friend_List_UpdateReq", &lwFriendMgr::Friend_List_UpdateReq)
		//
		.def(pkState, "RecvFriend_Command", &lwFriendMgr::RecvFriend_Command)

		.def(pkState, "Friend_Accept", &lwFriendMgr::Friend_Accept)
		.def(pkState, "Friend_Reject", &lwFriendMgr::Friend_Reject)

		//
		.def(pkState, "Friend_Count", &lwFriendMgr::Friend_Count)
		//
		.def(pkState, "FriendItem_ByAt", &lwFriendMgr::FriendItem_ByAt)
		.def(pkState, "FriendItem_ByNameW", &lwFriendMgr::FriendItem_ByNameW)
		.def(pkState, "FriendItem_ByName", &lwFriendMgr::FriendItem_ByName)

		.def(pkState, "CountOfGroup", &lwFriendMgr::CountOfGroup)
		.def(pkState, "CountOfLogin", &lwFriendMgr::CountOfLogin)

		.def(pkState, "MyLinkStatus", &lwFriendMgr::MyLinkStatus)
		.def(pkState, "SendFriend_Command_LinkStateChange", &lwFriendMgr::SendFriend_Command_LinkStateChange)

		.def(pkState, "SendFriend_Command_ChatStateChange", &lwFriendMgr::SendFriend_Command_ChatStateChange)

		.def(pkState, "ChangeGroupName", &lwFriendMgr::ChangeGroupName)
		
		;

	def(pkState, "IsMyFriend", lwIsMyFriend);
	return true;
}

bool lwIsMyFriend(lwGUID kCharGuid)
{
	SFriendItem kFriendItem;
	return g_kFriendMgr.Friend_Find_ByGuid(kCharGuid(), kFriendItem);
}

//
int lwFriendMgr::MyLinkStatus()
{
	if (!m_pkFriendMgr)
	{
		return 0;
	}
	return m_pkFriendMgr->MyLinkStatus();
}

//
bool lwFriendMgr::Friend_List_UpdateReq(bool bImmediate)
{
	if (!m_pkFriendMgr) { return false; }
	return m_pkFriendMgr->Friend_List_UpdateReq(bImmediate);
}

bool lwFriendMgr::RecvFriend_Command(lwPacket &rkPacket)
{
	if (!m_pkFriendMgr) { return false; }
	return m_pkFriendMgr->RecvFriend_Command(rkPacket());
}

//Send
bool lwFriendMgr::SendFriend_Command_AddByGuid(lwGUID lwkGuid)
{
	if (!m_pkFriendMgr) { return false; }
	if (lwkGuid.IsNil())  { return false; }
	SFriendItem kFriendItem;
	kFriendItem.CharGuid(lwkGuid());
	return m_pkFriendMgr->SendFriend_Command(FCT_ADD_BYGUID, kFriendItem);
}

bool lwFriendMgr::SendFriend_Command_AddByName(lwWString lwkFriendName)
{
	if (!m_pkFriendMgr) { return false; }
	if (lwkFriendName.IsNil() || lwkFriendName.Length()<=0)
	{
		return false;
	}

	SFriendItem kFriendItem;
	kFriendItem.ExtendName(lwkFriendName());
	return m_pkFriendMgr->SendFriend_Command(FCT_ADD_BYNAME, kFriendItem);
}

bool lwFriendMgr::SendFriend_Command_AddGroup(lwWString lwkGroupName)
{
	if (!m_pkFriendMgr) { return false; }
	if (!lwkGroupName.Length())
	{
		return false;
	}

	SFriendItem kGroup;
	kGroup.GroupName(lwkGroupName());
	m_pkFriendMgr->Friend_Add(kGroup);
	lua_tinker::call<void>("Update_FriendList");
	return false;
}

//local function

bool lwFriendMgr::SendFriend_Command_ModifyGroup(lwGUID lwkGuid, lwWString lwkNewGroupName)	//그룹을 이동시키고
{
	if (!m_pkFriendMgr) { return false; }
	SFriendItem kFriendItem;
	if(0 != lwkNewGroupName.Length() && m_pkFriendMgr->Friend_Find_ByGuid(lwkGuid(), kFriendItem))
	{
		kFriendItem.GroupName(lwkNewGroupName());
		return m_pkFriendMgr->SendFriend_Command(FCT_MODIFY, kFriendItem);
	}
	else
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(EM_Invalid_EditGroup_Friend, kChatLog, true);
	}
	return false;
}
bool lwFriendMgr::SendFriend_Command_ModifyGroupName(lwWString lwkOldGroupName, lwWString lwkNewGroupName)
{
	if (!m_pkFriendMgr) { return false; }
	SFriendItem kFriendItem;
	PgFriendMgr::ContFriend kCont;
	PgFriendMgr::ContFriend kNewCont;

	if(0 != lwkNewGroupName.Length() && m_pkFriendMgr->Friend_Find_ByGroup(lwkOldGroupName(), kCont))	//바꿀려는 그룹의 이름 검색
	{
		SFriendItem kTempItem;
		if(!m_pkFriendMgr->Friend_Find_ByGroup(lwkNewGroupName(), kNewCont)) // 새 이름으로 검색해서 없으면
		{
			kTempItem.GroupName(lwkOldGroupName());
			kTempItem.ExtendName(lwkNewGroupName());
			return m_pkFriendMgr->SendFriend_Command(FCT_GROUP, kTempItem);
		}
		else //있으면
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(EM_Duplicate_Group, kChatLog, true);
		}
	}
	else
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(EM_Invalid_EditGroup, kChatLog, true);
	}
	return false;
}

bool lwFriendMgr::SendFriend_Command_Delete(lwGUID lwkGuid)
{
	if (!m_pkFriendMgr) { return false; }
	SFriendItem kFriendItem;
	if(m_pkFriendMgr->Friend_Find_ByGuid(lwkGuid(), kFriendItem))
	{
		return m_pkFriendMgr->SendFriend_Command(FCT_DELETE, kFriendItem);
	}
	NILOG(PGLOG_ERROR, "Can't find friend\n"); assert(0);

	return false;
}
bool lwFriendMgr::SendFriend_Command_DeleteGroup(lwWString lwkGroupName)
{
	if (!m_pkFriendMgr) { return false; }
	if( 1 <= m_pkFriendMgr->CountInGroupFriend(lwkGroupName()))
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(EM_Is_Not_Empty, kChatLog, true);
		return false;
	}

	if (m_pkFriendMgr->RemoveGroup(lwkGroupName()))
	{
		lua_tinker::call<void>("Update_FriendList");
	}

	return false;
}


bool lwFriendMgr::Friend_Accept(lwGUID lwkGuid)
{
	if (!m_pkFriendMgr) { return false; }
	SFriendItem kFriendItem;
	if(m_pkFriendMgr->Friend_Find_ByGuid(lwkGuid(), kFriendItem))
	{
		return m_pkFriendMgr->SendFriend_Command(FCT_ADD_ACCEPT, kFriendItem);
	}
	NILOG(PGLOG_ERROR, "Can't find friend\n"); assert(0);
	return false;
}
bool lwFriendMgr::Friend_Reject(lwGUID lwkGuid)
{
	if (!m_pkFriendMgr) { return false; }
	SFriendItem kFriendItem;
	if(m_pkFriendMgr->Friend_Find_ByGuid(lwkGuid(), kFriendItem))
	{
		return m_pkFriendMgr->SendFriend_Command(FCT_ADD_REJECT, kFriendItem);
	}
	NILOG(PGLOG_ERROR, "Can't find friend\n"); assert(0);
	return false;
}

int lwFriendMgr::Friend_Count()//초기화 할일이 있으면 갯수가 리턴된다.
{
	if (!m_pkFriendMgr) { return 0; }
	return m_pkFriendMgr->Friend_Count();
}

lwFriendItem lwFriendMgr::FriendItem_ByAt(int iNo)
{
	SFriendItem kFriendItem;
	if (!m_pkFriendMgr) { return kFriendItem; }
	if(!m_pkFriendMgr->Friend_Find_ByAt(iNo, kFriendItem))
	{
		NILOG(PGLOG_ERROR, "invalid array pos [%d]\n", iNo); assert(0);
	}
	return kFriendItem;
}
lwFriendItem lwFriendMgr::FriendItem_ByNameW(lwWString kFriendName)
{
	SFriendItem kFriendItem;
	if (!m_pkFriendMgr) { return kFriendItem; }
	if(!m_pkFriendMgr->Friend_Find_ByName(kFriendName(), kFriendItem))
	{
		NILOG(PGLOG_ERROR, "Can't find friend [%s] \n", MB(kFriendName())); assert(0);
	}
	return kFriendItem;
}
lwFriendItem lwFriendMgr::FriendItem_ByName(char *szFriendName)
{
	SFriendItem kFriendItem;
	if (!m_pkFriendMgr) { return kFriendItem; }
	if(!m_pkFriendMgr->Friend_Find_ByName(UNI(szFriendName), kFriendItem))
	{
		NILOG(PGLOG_ERROR, "Can't find friend [%s]\n", szFriendName); assert(0);
	}
	return kFriendItem;
}

//
int lwFriendMgr::CountOfGroup(lwWString kGroupName, bool bLogin)
{
	if (!m_pkFriendMgr) { return 0; }
	return m_pkFriendMgr->CountOfGroup(kGroupName(), bLogin);
}

int lwFriendMgr::CountOfLogin(bool bLogin)
{
	if (!m_pkFriendMgr) { return 0; }
	return m_pkFriendMgr->CountOfLogin(bLogin);
}

bool lwFriendMgr::SendFriend_Command_LinkStateChange(int const iValue)
{
	if (!m_pkFriendMgr) { return false; }
	if (m_pkFriendMgr->MyLinkStatus() == iValue)
	{
		return false;
	}

	m_pkFriendMgr->MyLinkStatus((BYTE)iValue);	//클라에어 직접 넣으면 안된다. 서버에서 받아서 하자. 지금은 임시로 
	PgFriendMgr::ContFriend rkCont;
	return m_pkFriendMgr->SendFriend_Command_Multi(FCT_LINKSTATUS, rkCont);
}

bool lwFriendMgr::SendFriend_Command_ChatStateChange(lwGUID kGuid, int iType)
{
	if (!m_pkFriendMgr) { return false; }
	if (kGuid.IsNil())
	{
		return false;
	}

	SFriendItem kFriendItem;
	if(m_pkFriendMgr->Friend_Find_ByGuid(kGuid(), kFriendItem))
	{
		kFriendItem.ChatStatus(iType);	//실제 데이터가 아니다
		return m_pkFriendMgr->SendFriend_Command(FCT_MODIFY_CHATSTATUS, kFriendItem);
	}	
	return false;
}

void lwFriendMgr::ChangeGroupName(lwWString kOld, lwWString kNew)
{
	if (!m_pkFriendMgr) { return; }
	if (kOld.IsNil() || kNew.IsNil())
	{
		return;
	}
	int const iCount = m_pkFriendMgr->CountOfGroup(kOld(), false);
	if (0>=iCount)	//클라이언트가 그룹명을 바꿀 수 있는때는 그룹에 아무도 없을때만 바꿀수 있다
	{
		m_pkFriendMgr->ChangeGroupName(kOld(), kNew());
	}
}