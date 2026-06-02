#ifndef FREEDOM_DRAGONICA_CONTENTS_FRIEND_PGFRIENDMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_FRIEND_PGFRIENDMGR_H

#include <vector>
#include <list>
#include <map>

#include "lwWString.h"
#include "lwPacket.h"

#include "CreateUsingNiNew.inl"

class PgFriendMgr
{
public:
	PgFriendMgr();
	virtual ~PgFriendMgr();

	typedef enum eFriendPalam
	{
		EFP_FriendPalam_Last_Update		=	20000,// millisecond: 20 초 마다 업데이트 할 기회를 준다.
	} EFriendPalam;

	CLASS_DECLARATION(int, iMyLinkStatus, MyLinkStatus);
	//CLASS_DECLARATION(bool, bGroup, Group);
	//CLASS_DECLARATION(bool, bChange, Change);
	typedef std::vector<SFriendItem> ContFriend;
	
public://Messenger
	bool RecvFriend_Command(BM::Stream* pkPacket);

	bool SendFriend_Command_Multi(int const iCommandType, const ContFriend &rkCont, int const iPalam=0);

	bool SendFriend_Command(int const iCommandType, SFriendItem &rkFriendItem, int const iPalam = 0);	//구조체 한개만
	//
	int Friend_Add(const SFriendItem &rkFriendItem);

	void Friend_ListUpdate();

	//외부용
	bool Friend_Find_ByGuid(BM::GUID const &rkMemberGuid, SFriendItem &rkFriendItem);
	bool Friend_Find_ByName(std::wstring const &rkFriendName, SFriendItem &rkFriendItem);
	bool Friend_Find_ByAt(const unsigned int iNo, SFriendItem &rkFriendItem);
	//SFriendItem* Friend_Find_ByName(std::wstring const &rkFriendName);

	bool Find_GroupItem(std::wstring const &rkGroupName, SFriendItem &rkFriendItem);
	bool Friend_Find_ByGroup(std::wstring const &rkGroupName, ContFriend &rkCont);
	int CountOfGroup(std::wstring &rkGroupName, bool bLogin=false);
	int CountOfLogin(bool bLogin);

	int CountInGroupFriend(std::wstring const &rkGroupName);
	/*
	bool Friend_GroupName_ByGroupAt(int const iNo, std::wstring &rkGroupName);
	SFriendItem* Friend_Find_ByAt_InGroup(std::wstring const &rkGroupName, int const iNo);
	SFriendItem* Friend_Find_ByAt_InOnOff(std::wstring const &rkOnOff, int const iNo);
	*/

	int Friend_Count();

	/*
	bool Friend_Count_Group(int &riCount);
	int Friend_Count_GroupInFriend(std::wstring &rkGroupName);
	int Friend_Count_OnOffInFriend(std::wstring &rkOnOff);
	*/
	bool Friend_Clear();

	//UI
	bool Friend_List_UpdateReq(bool bImmediate=false);

	bool RemoveGroup(std::wstring const &kName);	//클라이언트 용
	bool ChangeGroupName(std::wstring const &kOldName, std::wstring const &kNewName);	//클라이언트 용

protected:
	SFriendItem* Friend_Find_ByGuid(BM::GUID const &rkFriendMemberGuid);

protected:
	//메신져
	//typedef std::map<std::wstring, int> ContGroup;

	Loki::Mutex m_kFriendList_Mutex;

	ContFriend m_kFriendList;//메신져(기본)
	//CONT_GROUP_MAP m_kFriendMap_Group;//그룹별(그룹명: 카운트)
	//CONT_GROUP_MAP m_kFriendMap_OnOff;//그룹별(온/오프: 카운트)


	DWORD m_dwLastUpdateFriendList;//업데이트 시간

	void ModifyList(const ContFriend &rkCont, int const iCommandType = FCT_MODIFY);
};

#define g_kFriendMgr SINGLETON_CUSTOM(PgFriendMgr, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_FRIEND_PGFRIENDMGR_H