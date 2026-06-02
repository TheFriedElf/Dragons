#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDITEM_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDITEM_H

#include "lwUI.h"
#include "PgFriendMgr.h"

class lwFriendItem
{
public:
	lwFriendItem(SFriendItem& rkFriendItem);
	
	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwWString CharacterName();
	lwWString GroupName();
	void SetGroupName(char const* szName);
	lwGUID Guid();

	int Location()const;
	
	bool IsOnline();
	bool IsOffline();
	int IsLinkStatus();
	bool IsBlock_HeCanTalk();//(그사람) 말하는걸 막는다
	bool IsBlock_HeCanListen();//(내말) 듣는걸 막는다

	bool IsHaveHome();
	bool SetMyHomeStatus(lwUIWnd kWnd);

	bool IsRequester();//요청자 인가?
	bool IsAccepter();//수락자 인가?

	bool IsNil();

	int Level();
	int Channel();
	int Class();

protected:
	/*
	BM::GUID	kFriendGuid;//CharGuid
	TCHAR		szCharName[MAX_FRIEND_NAME_LEN];
	TCHAR		szGroupName[MAX_FRIEND_GROUPNAME_LEN];
	BYTE		cChatStatus;
	BYTE		cLinkStatus;
	*/
	SFriendItem m_kFriendItem;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_FRIEND_LWFRIENDITEM_H