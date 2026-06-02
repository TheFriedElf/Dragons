#ifndef FREEDOM_DRAGONICA_CONTENTS_PARTY_PGCLIENTPARTY_H
#define FREEDOM_DRAGONICA_CONTENTS_PARTY_PGCLIENTPARTY_H

//#include "NiMain.h"
//#include "Variant/PgParty.h"
//#include "PgScripting.h"
//#include "lwGuid.h"
//#include "lwWString.h"

#include "Variant/PgWaiter.h"
#include "Variant/PgParty.h"
#include "lwUI.h"

class lwWString;
#pragma pack(1)
typedef struct tagPartyMember : public SPartyUserInfo
{
	tagPartyMember()
	{
		Clear();
	}

	void Clear()
	{
		SPartyUserInfo::Clear();
	}

	void operator = (const SPartyUserInfo &rkInfo)
	{
		Clear();
		SPartyUserInfo::Set(rkInfo);
	}
} SPartyMember;

typedef struct tagPartyList : public SPartyListInfo
{
	tagPartyList()
	{
		Clear();
	}

	void Clear()
	{
		SPartyListInfo::Clear();
	}

	void operator = (const SPartyListInfo &rkInfo)
	{
		Clear();
		SPartyListInfo::Set(rkInfo);
	}
} SPartyList;

typedef struct tagClientPartyName
{
	BM::GUID kPartyGuid;
	std::wstring kPartyName;
	BYTE cCurMember;
	BYTE cMaxMember;
	BM::GUID kMasterGuid;
	SPartyOption kPartyOption;

	ContGuidSet kGuidSet; // 패킷으로 부터 읽지 않는다.

	void SetOnlyInfo(tagClientPartyName const& rhs)
	{
		kPartyGuid		= rhs.kPartyGuid;
		kPartyName		= rhs.kPartyName;
		cCurMember		= rhs.cCurMember;
		cMaxMember		= rhs.cMaxMember;
		kMasterGuid		= rhs.kMasterGuid;
		kPartyOption	= rhs.kPartyOption;
	}

	void ReadFromPacket(BM::Stream& rkPacket)
	{
		rkPacket.Pop(kPartyGuid);
		rkPacket.Pop(kPartyName);
		rkPacket.Pop(cCurMember);
		rkPacket.Pop(cMaxMember);
		rkPacket.Pop(kMasterGuid);
		kPartyOption.ReadFromPacket(rkPacket);
	}
} SClientPartyName;

typedef struct tagRejectedInviteUser
{
	BM::GUID kCharGuid;
	std::wstring kCharName;
	float fRejectedTime;
} SRejectedInviteUser;
#pragma pack()

typedef struct tagPartyAttributeList
{
	int iAttribute;
	int iArea_NameNo;
}SPartyAttributeList;

typedef enum ePartyListSort : BYTE	// 파티 리스트 정렬 방법
{
	PLS_NONE			= 0,
	PLS_STATEGREATER	= 1,
	PLS_STATELESS		= 2,
	PLS_NAMEGREATER		= 3,
	PLS_NAMELESS		= 4,
	PLS_ATTRGREATER		= 5,
	PLS_ATTRLESS		= 6,
	PLS_MEMBERGRATER	= 7,
	PLS_MEMBERLESS		= 8,
	PLS_END,
} EPartyListSort;

typedef enum eFindPartyUserListSort : BYTE
{
	FPL_NONE			= 0,
	FPL_TITLEGREATER	= 1,
	FPL_TITLELESS		= 2,
	FPL_LEVELGREATER	= 3,
	FPL_LEVELLESS		= 4,
	FPL_CLASSGREATER	= 5,
	FPL_CLASSLESS		= 6,
	FPL_END,
} EFindPartyUserListSort;

typedef std::list< SPartyAttributeList > ContPartyAttributeList;
typedef std::list< SPartyMember* > ContPartyMember;
typedef std::list< SPartyListInfo* > ContPartyList;
typedef std::list< SRejectedInviteUser > ContRejectedUser;
typedef std::map< int, ContPartyAttributeList > CONT_ATTRIBUTE_LIST;		// Continent에 해당되는 SPartyAttributeList<Attribute, Area_NameNo List>
namespace PgClientPartyUtil
{
	void PartyResultMsg(int iResult);
	void SetCharacterPartyGuid(BM::GUID const &rkCharGuid, BM::GUID const &rkPartyGuid);
	void AttachPartyCircle(BM::GUID const &rkPilotGuid);
	void DetachPartyCircle(BM::GUID const &rkPilotGuid);
	void UpdatePartyWndAll();
	bool IsCanPartyArea( bool const bChkPublic = false );
	bool IsCanInviteFriendPartyArea();
	bool IsCanPartyBreakIn(int const MasterMapNo, CONT_NEED_KEY_ITEM & KeyItem, CONT_NEED_KEY_ITEM & CashKeyItem);
	bool IsHaveKeyItemPartyBreakIn(CONT_NEED_KEY_ITEM const& KeyItem, CONT_NEED_KEY_ITEM const& CashKeyItem, int & NeedItemNo);
	bool IsCanPvPArea();
	bool IsInParty();
	bool IsInPartyMemberGuid(BM::GUID const &rkCharGuid);
	bool IsPartyMaster(BM::GUID const &rkCharGuid);
	bool IsOhterPartyMaster(BM::GUID const &rkCharGuid);
	void Net_C_N_REQ_JOIN_PARTYFIND_ByGuid(lwGUID kGuid);
	void Net_C_N_ANS_JOIN_PARTY(bool bAccept, BM::GUID const &rkPartyGUID);
	void Net_C_N_ANS_JOIN_PARTYFIND(bool bAccept, BM::GUID const &rkUserGUID);
	void Net_C_N_REQ_KICKOUT_PARTY_USER(lwGUID kGuid);
	void Net_C_N_ANS_JOIN_PARTY_EX(bool bAccept, BM::GUID const &rkPartyGUID);
	void Send_Request_Leave_Party();//	자신이 파티를 탈퇴하겠음을 서버에 요청
	void Send_RequestChangeMaster(const lwGUID NewMasterGuid);
	void Send_RequestPartyList();
	bool PartyOptionStr(const SPartyOption& rkNewOption, std::wstring& rkOut, bool bChange);
	void AttachPartyCircle(BM::GUID const &rkPilotGuid);
	bool GetPartyJoinLevelCheck(BM::GUID const &rkCharGuid);
	void Send_RequestSearchPeopleList();
	lwGUID lwGetMemberGuidAt(int const iAt);
	lwWString lwGetMemberNameAt(int const iAt);
	std::wstring GetClassNameTTW(int const iClassNo);
	void lwGetPartyListAreaName(bool bAll, int const iAttribute, int const iContinent, lwUIWnd lwWnd);
	void lwFakePartyListViewRefresh(int iNowPage, int iAttribute);
	void lwPartyListViewRefresh(int iNowPage, int iAttribute);
	void lowFindPartyUserListViewRefresh(int iNowPage, int iAttribute);
	int lwPartyListNowPage();
	int lwPartyListMaxPage();
	lwWString lwGetPartyListToolTip(lwGUID kGuid);
	void lwSetPartyListInit();
	void ClearOtherPartyName();
};


//
class	PgClientParty
{
	typedef std::map< BM::GUID, SClientPartyName > ContOtherPartyName;
	typedef PgWaiterSet< BM::GUID, BM::GUID > ContInfoWaiter;
	friend struct Loki::CreateUsingNew< PgClientParty >;
	typedef std::vector< std::wstring > ContReserveName;
	typedef std::map< BM::GUID, SPartyOption > ContPartyOption;
	typedef std::map< BM::GUID, SPartyListInfo > ContPartyListName;	
	typedef std::list<SPartyListInfo>	ContTempPartyListName;
	typedef std::map< BM::GUID, SFindPartyUserListInfo > ContFindPartyUserList;
	typedef std::list<SFindPartyUserListInfo> ContTempFindPartyUserList;
public:
	PgClientParty();
	~PgClientParty();
	
	static bool RegisterWrapper(lua_State *pkState);

	bool GetMember(std::wstring const &rkName, SPartyMember const*& pkMember)const;
	bool GetMember(BM::GUID const &rkCharGuid, SPartyMember const*& pkMember)const;
	bool IsMember(std::wstring const &rkName)const;
	bool IsMember(BM::GUID const &rkCharGuid)const;
	
	void Clear(bool const bLeave = false);//	파티 멤버를 초기화
	void ClearCache();//파티 이름 캐쉬 초기화
	void ClearOtherPartyName(); // 다른 파티 정보 초기화

	void GetPartyMemberList(ContPartyMember &rkPartyMemberList)const { rkPartyMemberList = m_Members; }
	size_t const MemberCount()const { return m_Members.size(); }

	int PartyListMaxPage();
	int PartyListMaxPage(ContTempPartyListName const& PartyListName);
	int PartyListNowPage();
	int PartyFindPeopleListMaxPage();
	int PartyFindPeopleListNowPage();
	void SetPartyListInit();
	int GetAttributeValue(int iAttribute);
	void FakePartyListViewRefresh(int iNowPage, int iAttribute);
	void PartyListViewRefresh(int iNowPage, int iAttribute);
	void FindPartyUserListViewRefresh(int iNowPage, int iAttribute);
	void AddPartyName(SClientPartyName const& rkPartyName);
	bool GetPartyName(BM::GUID const &rkPartyGuid, SClientPartyName& rkOut) const; // 캐릭터가 아닌 곳에서
	bool GetPartyName(BM::GUID const &rkPartyGuid, BM::GUID const& rkCharGuid, SClientPartyName& rkOut); // 캐릭터에 관련되서
	void RemovePartyName(BM::GUID const& rkPartyGuid, BM::GUID const& rkCharGuid);
	void ReqOtherPartyInfo(BM::GUID const &rkPartyGuid, BM::GUID const &rkCharGuid);
	std::wstring const& GeneratePartyName(BM::GUID const &rkPartyGuid);
	bool IsReserveName(std::wstring const &rkNewPartyName);

	//
	bool ProcessMsg(unsigned short const usType, BM::Stream& rkPacket);

	bool CheckLastInviteTime(std::wstring const &rkCharName);
	bool CheckLastInviteTime(BM::GUID const &rkCharGuid);
	void AddCheckInvietTime(ContRejectedUser::value_type& rkNewRejectedUser);
	BM::GUID const& GetMemberGuidAt(int const iAt) const;
	bool GetMemberNameAt(int const iAt, std::wstring& rkName);
	void SetPartyTotalList();
	void GetPartyListAreaName(bool bAll, int const iAttribute, int const iContinent, lwUIWnd lwWnd);
	bool GetPartyListToolTip(BM::GUID kGuid, std::wstring& kStr);
	bool GetPartyListStateToolTip(BM::GUID kGuid, std::wstring& kStr);

	void ArrangePartyList( ContPartyListName & PartyList );
	void SetPartyListAttribute( ContPartyListName & PartyList );
	void PartyListNewSort(ContTempPartyListName & PartyList);

	short GetPartyChannelNo( BM::GUID MasterGuid ) const;
private:
	bool CheckPartyList_Status( BYTE const Status );
	bool CheckPartyList_MemberFull( BYTE const CurMember, BYTE const MaxMember );
	bool CheckPartyList_Map( int const MasterMapNo );
	bool CheckPartyList_MissionDungeon( CONT_DEFMAP const* pDefMap, int const MasterMapNo );
	bool CheckPartyList_Town( CONT_DEFMAP const* pDefMap, int const MasterMapNo );
	bool CheckPartyList_MapLevel( CONT_DEFMAP const* pDefMap, int const MasterMapNo );
	bool CheckPartyList_MasterLevel( int const MasterLevel );
protected:
	void AddSelfPartyName();
	void AddMember(const SPartyUserInfo &rkMemberInfo);//	신규 멤버를 추가
	void RemoveMember(BM::GUID const &rkCharGuid, std::wstring *pkOutName );//	멤버를 파티에서 탈퇴시킨다.
	void MemberMapMove(lwGUID lwkGuid, SGroundKey const &rkKey);
	void ChangeMaster(BM::GUID const &rkNewMaster);
	void UpdatePartyBalloon(BM::GUID const& rkCharGuid);
	void UpdatePartyMiniMapIcon();
	SPartyMember* GetMember(std::wstring const &rkName)const;
	SPartyMember* GetMember(BM::GUID const &rkCharGuid)const;
	
	CLASS_DECLARATION_S_NO_SET(std::wstring, PartyName);
	CLASS_DECLARATION_S_NO_SET(BM::GUID, MasterGuid);
	CLASS_DECLARATION_S_NO_SET(BM::GUID, PartyGuid);
	CLASS_DECLARATION_S(SPartyOption, Option);
	CLASS_DECLARATION_S(int, NowPage);
	CLASS_DECLARATION_S(int, FindUserNowPage);
	CLASS_DECLARATION_S(bool, PartyState);
	CLASS_DECLARATION_S(int, PartyListSortMethod);
	CLASS_DECLARATION_S(int, FindPartyUserListSortMethod);
	CLASS_DECLARATION_S(bool, IsPrivateRegist);

	CLASS_DECLARATION_S(__int64, ToPartyMasterWarpCost);
	CLASS_DECLARATION_S(BM::GUID, PartyMasterTransTowerGuid);
	CLASS_DECLARATION_S(TBL_DEF_TRANSTOWER_TARGET_KEY, PartyMasterTransTowerKey);

	CLASS_DECLARATION_S(BM::GUID, PartyListGuid);

	CLASS_DECLARATION_S(short, PartyListChannelCount);
	CLASS_DECLARATION_S(short, TotalChannelCount);

	void Receive_Party_Join_Request(BM::Stream& rkPacket);//	가입 요청을 받은 사람에게, 가입 요청 받았음을 표시해주고, 수락,거부 응답을 기다린다.
	void Receive_PartyFind_Join_Request(BM::Stream& rkPacket);//	파티 가입 요청 메시지
	void Recv_PT_M_C_ANS_PARTY_NAME(BM::Stream& rkPacket);
	void Recv_PT_N_C_NFY_CREATE_PARTY(BM::Stream& rkPacket);
	void Recv_PT_N_C_ANS_JOIN_PARTY_TO_MASTER(BM::Stream& pkPacket);
	void Recv_PT_N_C_NFY_JOIN_PARTY(BM::Stream& pkPacket);
	void Recv_PT_N_C_ANS_JOIN_PARTY(BM::Stream& pkPacket);
	void Recv_PT_N_C_NFY_LEAVE_PARTY_USER(BM::Stream& pkPacket);
	void Recv_PT_N_C_NFY_PARTY_USER_MAP_MOVE(BM::Stream& pkPacket);
	void Recv_PT_N_C_NFY_KICKOUT_PARTY_USER(BM::Stream& pkPacket);
	void Recv_PT_M_C_NFY_CHANGE_MEMBER_ABIL(BM::Stream& pkPacket);
	void Recv_PT_N_C_NFY_PARTY_DESTROY(BM::Stream& pkPacket);
	void Recv_PT_N_C_ANS_PARTY_CHANGE_MASTER(BM::Stream& pkPacket);
	void Recv_PT_N_C_ANS_PARTY_CHANGE_OPTION(BM::Stream& pkPacket);
	void Recv_PT_N_C_ANS_PARTY_RENAME(BM::Stream& rkPacket);
	void Recv_PT_M_C_NFY_PARTY_JOIN(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_PARTY_LIST(BM::Stream& rkPacket);
	void Recv_PT_M_C_NFY_PARTY_MEMBER_ITEM(BM::Stream& rkPacket);
	void Recv_PT_T_C_NFY_PARTY_MEMBER_MYHOME(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_FIND_PARTY_USER_LIST(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_REGIST_PRIVATE_PARTY_FIND(BM::Stream& rkPacket);
	void Recv_PT_N_C_NFY_UNREGIST_PRIVATE(BM::Stream& rkPacket);
	void Recv_PT_T_C_ANS_OTHER_CHANNEL_PARTY(BM::Stream & rkPacket);
	
	void Recv_PT_M_C_ANS_JOIN_PARTYFIND_ITEM_CHECK(BM::Stream & rkPacket);

protected:
	mutable Loki::Mutex m_kMutex;

	ContPartyMember	m_Members;	//	 파티의 파티원 목록 //private 로 바꾸려면 UI 코드를 뜯어 오든지 해야함.
	BM::TObjectPool< SPartyMember > m_kMemberPool;

	//
	ContInfoWaiter m_kWaiter;
	ContOtherPartyName m_kOtherPartyName;

	ContReserveName m_kReservePartyName;

	ContRejectedUser m_kRejectedUser;

	ContPartyOption m_kPartyOption;

	ContPartyListName m_kPartyListName;

	ContFindPartyUserList m_kFindPartyUserList;

public:
	CONT_ATTRIBUTE_LIST		m_kPartyListAttribute;	
	SPartyOption TempPartyOption;
};
//void DiePartyMember(BM::GUID &rkCharGuid);

#define g_kParty SINGLETON_CUSTOM(PgClientParty, Loki::CreateUsingNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_PARTY_PGCLIENTPARTY_H