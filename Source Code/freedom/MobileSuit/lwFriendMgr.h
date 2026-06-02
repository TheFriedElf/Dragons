#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDMGR_H

#include "PgFriendMgr.h"

class lwFriendItem;

extern bool lwIsMyFriend(lwGUID kCharGuid);

class lwFriendMgr
{
public:
	lwFriendMgr(PgFriendMgr* pkFriendMgr);
	
	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//
	bool SendFriend_Command_AddByGuid(lwGUID lwkGuid);
	bool SendFriend_Command_AddByName(lwWString lwkFriendName);
	bool SendFriend_Command_AddGroup(lwWString lwkGroupName);
	bool SendFriend_Command_Delete(lwGUID lwkGuid);
	bool SendFriend_Command_DeleteGroup(lwWString lwkGroupName);
	bool SendFriend_Command_ModifyGroup(lwGUID lwkGuid, lwWString lwkNewGroupName);//친구용
	bool SendFriend_Command_ModifyGroupName(lwWString lwkGroupName, lwWString lwkNewGroupName);//그룹용
	bool SendFriend_Command_LinkStateChange(int const iValue);;
	bool SendFriend_Command_ChatStateChange(lwGUID kGuid, int iType);

	bool RecvFriend_Command(lwPacket &rkPacket);

	//친구

	//친구목록
	//0. 친구목록 아이템은 lua로 감싼다.
	//1. 첫 로그인 시에는 무조건 펼처진 상태이다
	//2. 열 때는 없다면 CallUI, 있다면 Visible(true), 닫을때에는 무조건 Visible(false)
	//3. 각각의 갱신은 XUI의 아이템에서 호출 하도록 한다.
	//4. 추가, 삭제, 시에는 무조건 초기화를 한다.

	bool Friend_List_UpdateReq(bool bImmediate = false);//서버로 목록 요청

	bool Friend_Accept(lwGUID lwkGuid);
	bool Friend_Reject(lwGUID lwkGuid);

	//
	//bool Change();
	//bool Friend_Clear();
	int Friend_Count();
	//int Friend_Count_Group();
	//int Friend_Count_GroupInFriend(char *szGroupName);
	//int Friend_Count_GroupInFriendW(lwWString kGroupName);
	//lwWString Friend_GroupName_ByGroupAt(int iNo);

	//lwFriendItem Friend_Find_ByAt_InGroup(lwWString kGroupName, int iNo);


	lwFriendItem FriendItem_ByAt(int iNo);
	lwFriendItem FriendItem_ByNameW(lwWString kFriendName);
	lwFriendItem FriendItem_ByName(char *szFriendName);

	int CountOfGroup(lwWString kGroupName, bool bLogin=false);
	int CountOfLogin(bool bLogin);

	//bool Group_Get();
	//void Group_Set(bool bSet);
	int MyLinkStatus();
	void ChangeGroupName(lwWString kOld, lwWString kNew);
protected:
	PgFriendMgr *m_pkFriendMgr;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDMGR_H