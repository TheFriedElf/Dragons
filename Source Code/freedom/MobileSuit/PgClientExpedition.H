#ifndef FREEDOM_DRAGONICA_CONTENTS_EXPEDITION_PGCLIENTEXPEDITION_H
#define FREEDOM_DRAGONICA_CONTENTS_EXPEDITION_PGCLIENTEXPEDITION_H

#include "Variant/PgWaiter.h"
#include "Variant/PgParty.h"
#include "lwUI.h"

typedef struct tagExpeditionMember : public SExpeditionUserInfo
{
	tagExpeditionMember()
	{
		Clear();
	}

	void Clear()
	{
		SExpeditionUserInfo::Clear();
	}
	
	void operator = (SExpeditionUserInfo const& Info)
	{
		Clear();
		SExpeditionUserInfo::Set(Info);
	}
} SExpeditionMember;	// 원정대원 정보를 담을 구조체

typedef struct tagExpeditionUser : public SExpeditionInviteUserInfo
{
	tagExpeditionUser()
	{
		Clear();
	}

	void Clear()
	{
		SExpeditionInviteUserInfo::Clear();
	}
} SExpeditionUser;	// 초대 리스트에 보여줄 유저 정보를 담을 구조체

typedef struct tagExpeditionInfo
{
	BM::GUID			ExpeditionGuid;
	std::wstring		ExpeditionName;
	BYTE				cCurMember;
	BYTE				cMaxMember;
	BM::GUID			MasterGuid;
	SExpeditionOption	ExpeditionOption;

	ContGuidSet GuidSet; // 패킷으로 부터 읽지 않는다.

	tagExpeditionInfo()
	{
		Clear();
	}
	
	void Clear()
	{
		ExpeditionGuid.Clear();
		ExpeditionName.clear();
		cCurMember = 0;
		cMaxMember = 0;
		MasterGuid.Clear();
		ExpeditionOption.Clear();
	}

	void SetOnlyInfo(tagExpeditionInfo const& rhs)
	{
		ExpeditionGuid			=	rhs.ExpeditionGuid;
		ExpeditionName			=	rhs.ExpeditionName;
		cCurMember				=	rhs.cCurMember;	
		cMaxMember				=	rhs.cMaxMember;	
		MasterGuid				=	rhs.MasterGuid;
		ExpeditionOption		=	rhs.ExpeditionOption;
	}

	void WriteToPacket(BM::Stream& Packet)
	{
		Packet.Push(ExpeditionGuid);
		Packet.Push(ExpeditionName);
		Packet.Push(cCurMember);
		Packet.Push(cMaxMember);
		Packet.Push(MasterGuid);
		ExpeditionOption.WriteToPacket(Packet);
	}

	void ReadFromPacket(BM::Stream& Packet)
	{
		Packet.Pop(ExpeditionGuid);
		Packet.Pop(ExpeditionName);
		Packet.Pop(cCurMember);
		Packet.Pop(cMaxMember);
		Packet.Pop(MasterGuid);
		ExpeditionOption.ReadFromPacket(Packet);
	}
} SExpeditionInfo;

typedef struct tagExpeditionRejectedUser
{
	BM::GUID kExpeditionGuid;
	BM::GUID kCharGuid;
	std::wstring kCharName;
	float fRejectedTime;
} SExpeditionRejectedUser;

namespace PgClientExpeditionUtil	// 원정대에 관련된 함수들
{
	void SetExpeditionNpcGuid(lwGUID lwNpcGuid);
	lwGUID GetExpeditionNpcGuid();

	void SetCharacterExpeditionGuid(BM::GUID const& CharGuid, BM::GUID const& ExpeditionGuid);

	bool CheckEnableEnter();						// 원정대 입장 가능한지 체크
	bool CheckExpeditionDailyQuest();				// 원정대 일일퀘스트 완료 체크

	void ClearNotHaveKeyItemMemberList();			// 원정대 메달 미소지자 리스트 초기화

	// 윈도우에 원정대 정보를 보여줌
	void ShowExpeditionTitleText(lwUIWnd lwWnd);
	void ShowExpeditionInfoText(lwUIWnd lwWnd);
	void ShowExpeditionName(lwUIWnd lwWnd);
	void ShowExpeditionArea(lwUIWnd lwWnd);
	void ShowExpeditionCurrMemNum(lwUIWnd lwWnd);
	void ShowExpeditionImage(lwUIWnd lwWnd);

	void UpdateExpeditionMemberInfo(lwUIWnd lwWnd, int const teamNum);		// 원정대 등록창
	void UpdateExpeditionMainBarInfo(lwUIWnd lwWnd, int const teamNum);		// 원정대 미니바
	void UpdateExpeditionUserList(lwUIWnd lwWnd);							// 유저 검색창

	// 시스템 메시지의 #~# 치환
	lwWString ReplaceSystemMgs(int const iTextNumber);
	std::wstring GetClassNameTTW(int const iClassNo);

	SExpeditionMember const* GetMember(std::wstring const& CharName);	// 내 원정대에서 이름으로 한명을 찾는다
	SExpeditionMember const* GetMember(BM::GUID const& CharGuid);		// 내 원정대에서 GUID로 한명을 찾는다

	int GetMaxTeamNumber();	// 자기 원정대의 최대 팀 갯수
	int GetMemberTeamNumber(lwGUID lwCharGuid);	// 멤버의 팀 번호

	int GetExpeditionTeamMemberNum(int const teamNum);	// 해당 팀에 지금 몇명인지
	bool GetExpeditionJoinLevelCheck(BM::GUID const& CharGuid);	// 레벨 체크

	int GetExpeditionOptionExp();	// 원정대 옵션 가져오기
	int GetExpeditionOptionItem();
	int GetExpeditionOptionPublic();
	int GetExpeditionOptionLevel();

	lwWString GetExpeditionOptionWStr();		// 현재 원정대 옵션 정보를 스트링으로 뽑아낸다.
	int GetClassNumber(lwGUID lwCharGuid);		// 대상의 클래스 번호를 가져온다.

	bool IsInExpedition();
	bool IsInExpeditionMemberGuid(lwGUID lwCharGuid);
	bool IsExpeditionMaster(lwGUID lwCharGuid);

	// 원정대 옵션 세팅 함수(아이템획득방식, 원정대공개여부, 제한레벨, 원정대 이름)
	void SetExpeditionOption(int const OptionExp,
		int const OptionItem, int const OptionPublicTitle, int const Level);

	bool ExpeditionOptionStr(const SExpeditionOption& NewOption, std::wstring& Out, bool bChange);

	void ExpeditionListNextPage();	// 원정대 참가 리스트 페이지 전환
	void ExpeditionListPrevPage();
	
	bool IsInExpeditionMember(BM::GUID const& rkCharGuid);						// 내 원정대에 포함된 멤버인가?

	void Send_C_N_REQ_CREATE_EXPEDITION();										// 원정대 생성 요청
	void Send_C_N_REQ_DISPERSE_EXPEDITION();									// 원정대 해산 요청
	void Send_C_N_REQ_JOIN_EXPEDITION(lwGUID lwCharGuid);						// 원정대 가입 요청
	void Send_C_N_ANS_JOIN_EXPEDITION(bool bAccept, BM::GUID const& UserGUID);	// 원정대 가입요청 수락여부를 서버로
	void Send_C_N_REQ_INVITE_EXPEDITION(lwUIWnd lwWnd);							// 원정대 초대 요청
	void Send_C_N_REQ_INVITE_EXPEDITION_BYGUID(lwGUID lwCharGuid);				// 원정대 초대 요청(BY GUID)
	void Send_C_N_ANS_INVITE_EXPEDITION(bool bAccept, BM::GUID const& ExpeditionGUID);	// 원정대 초대요청 수락여부를 서버로
	void Send_C_N_REQ_LEAVE_EXPEDITION();										// 원정대 탈퇴 요청
	void Send_C_N_REQ_CHANGEMASTER_EXPEDITION(lwGUID lwNewMasterGuid);			// 원정대장 변경 요청
	void Send_C_N_REQ_LIST_EXPEDITION(int const iContinent);					// 원정대 리스트 요청
	void Send_C_N_REQ_RENAME_EXPEDITION(lwWString lwNewName);					// 원정대명 변경 요청
	void Send_C_N_REQ_CHANGEOPTION_EXPEDITION(int const OptionExp,				// 원정대옵션 변경 요청
		int const OptionItem, int const OptionPublicTitle, int const Level);
	void Send_C_N_REQ_KICKOUT_EXPEDITION(lwGUID lwCharGuid);					// 원정대원 추방 요청
	void Send_C_N_REQ_TEAM_MOVE_EXPEDITION(lwGUID lwCharGuid, int const TeamNum);	// 원대대원 이동 요청
	void Send_C_M_REQ_LIST_USER_EXPEDITION();									// 원정대 초대가능한 유저 정보 요청
	void Send_C_M_REQ_NPC_ENTER_EXPEDITION();									// 원정대 입장 요청
	void Send_C_M_REQ_REFRESH_NEED_ITEM_INFO();									// 원정대 입장 아이템 보유 여부 요청
};

typedef std::vector< SExpeditionMember* > ContExpeditionMember;
typedef std::map< BM::GUID, SExpeditionUser > ContOtherPlayer;
typedef std::map< BM::GUID, SExpeditionInfo > ContOtherExpedition;
typedef std::vector< BM::GUID > ContUserGuid;
typedef std::list< SExpeditionRejectedUser > ContWaitExpedition;
typedef std::list< SExpeditionRejectedUser > ContExpeditionRejectUser;
// 원정대
class PgClientExpedition
{
	friend struct Loki::CreateUsingNew< PgClientExpedition >;
	typedef PgWaiterSet< BM::GUID, BM::GUID > ContInfoWaiter;
public:
	PgClientExpedition();
	~PgClientExpedition();

	static bool RegisterWrapper(lua_State* pState);

	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);
	
	void AddMember(SExpeditionUserInfo const& MemberInfo);					// 멤버를 원정대에 가입
	void RemoveMember(BM::GUID const& CharGuid, std::wstring* pOutName);	// 멤버를 파티에서 탈퇴

	bool const IsMember(std::wstring const& CharName) const;
	bool const IsMember(BM::GUID const& CharGuid) const;
	bool const IsExpeditionMaster(std::wstring const& CharName) const;
	bool const IsExpeditionMaster(BM::GUID const& CharGuid) const;

	void Clear();

	void AddExpedition(SExpeditionInfo const& Expedition);
	bool GetExpedition(BM::GUID const& ExpeditionGuid, SExpeditionInfo& Out) const; // 캐릭터가 아닌 곳에서
	bool GetExpedition(BM::GUID const& ExpeditionGuid, BM::GUID const& CharGuid, SExpeditionInfo& Out);

	int const GetMaxTeamNumber() const;	// 자기 원정대의 최대 팀 갯수
	int const GetMemberTeamNumber(BM::GUID const& CharGuid) const;	// 멤버의 팀 번호

	void RemoveExpedition(BM::GUID const& ExpeditionGuid, BM::GUID const& CharGuid);
	void ReqOtherExpeditionInfo(BM::GUID const& ExpeditionGuid, BM::GUID const& CharGuid);

	int const GetNumExpeditionMember() const;				// 원정대원 수
	ContExpeditionMember const& GetMembers() const;			// 모든 원정대원의 정보 가져오기
	ContOtherPlayer const& GetOtherPlayers() const;			// 초대 가능한 유저들 정보 가져오기
	ContOtherExpedition const& GetOtherExpedition() const;	// 현재 생성되어 있는 모든 원정대 리스트
	ContUserGuid const& GetNotHaveKeyItemMember() const;	// 키아이템을 가지지 않은 멤버 리스트

	bool CheckLastInviteTime(std::wstring const& CharName, float const CheckTime);
	bool CheckLastInviteTime(BM::GUID const& CharGuid, float const CheckTime);
	void AddCheckInviteTime(ContExpeditionRejectUser::value_type& NewRejectedUser);
	void AddCheckInviteTime(BM::GUID const& NewRejectedUserGuid);
	void AddCheckInviteTime(std::wstring const& NewRejectedUserName);

	bool CheckLastJoinTime(BM::GUID const& ExpeditionGuid, float const CheckTime);
	void AddCheckJoinTime(BM::GUID const & NewExpedition);

	BM::GUID const & GetRejectedMaster(BM::GUID const & ExpeditionGuid);

	void UpdateBossGndInfo(BM::GUID const& ExpeditionGuid, DWORD EndTimeTick);	// 원정대 시간제한 스타트
	void UpdateExpeditionAniBar(BM::GUID const & CharGuid);						// 원정대 미니바(체력)
		
	int ExpeditionListMaxPage();					// 원정대 참가 리스트 최대 페이지
	int ExpeditionListNowPage();					// 원정대 참가 리스트 현재 페이지
	void SetExpeditionListInit();					// 원정대 참가 리스트 초기화
	void ExpeditionListViewRefresh(int NowPage);	// 원정대 참가 리스트 갱신

	bool SetExpeditionInfo(BM::GUID const& ExpeditionNpcGuid);	// 테이블에서 원정대 정보 가져오기
	void ClearExpeditionInfo();									// 원정대 정보 지우기
	void ClearNotHaveKeyItemMember();							// 원정대 메달 미소지자 리스트 지우기
protected:
	void AddSelfExpedition();
	void UpdateExpeditionBalloon(BM::GUID const& CharGuid);

	// 원정대원 정보 가져오기
	SExpeditionMember* GetMember(std::wstring const& CharName) const;
	SExpeditionMember* GetMember(BM::GUID const& CharGuid) const;

	CLASS_DECLARATION_S(SExpeditionOption, Option);					// 원정대 옵션
	CLASS_DECLARATION_S_NO_SET(BM::GUID, ExpeditionGuid);			// 원정대 GUID
	CLASS_DECLARATION_S_NO_SET(BM::GUID, MasterGuid);				// 원정대장 GUID
	CLASS_DECLARATION_S_NO_SET(std::wstring, ExpeditionName);		// 원정대 이름

	CLASS_DECLARATION_S(int, NowPage);								// 원정대 참가 창 현재 페이지

	CLASS_DECLARATION_S_NO_SET(BM::GUID, NpcGuid);					// NPC GUID (KEY값)
	CLASS_DECLARATION_S_NO_SET(int, MapNo);							// 던전 맵 번호
	CLASS_DECLARATION_S_NO_SET(int, NeedItemNo);					// 입장 아이템 번호
	CLASS_DECLARATION_S_NO_SET(int, NeedItemCnt);					// 입장 아이템 필요 갯수
	CLASS_DECLARATION_S_NO_SET(int, MaxExpeditionMember);			// 원정대 최대 인원 수
	CLASS_DECLARATION_S_NO_SET(int, MinExpeditionMember);			// 원정대 최소 인원 수
	CLASS_DECLARATION_S_NO_SET(int, DungeonStringNo);				// 던전 이름 번호
	CLASS_DECLARATION_S_NO_SET(std::wstring, ImgPath);				// 원정대 이미지 경로
	CLASS_DECLARATION_S_NO_SET(int, DungeonInfoStringNo);			// 던전 정보 번호

	void Recv_PT_N_C_NFY_CREATE_EXPEDITION(BM::Stream& Packet);	// 원정대 신청 알림(파티원에게)
	void Recv_PT_N_C_ANS_CREATE_EXPEDITION(BM::Stream& Packet);	// 원정대 신청 응답(원정대장에게)

	void Recv_PT_N_C_NFY_DISPERSE_EXPEDITION(BM::Stream& Packet);	// 원정대 해체 알림
	void Recv_PT_N_C_ANS_DISPERSE_EXPEDITION(BM::Stream& Packet);	// 원정대 헤체 응답

	void Recv_PT_N_C_REQ_JOIN_EXPEDITION(BM::Stream& Packet);		// 서버가 원정대장에게 가입요청 요구
	void Recv_PT_N_C_NFY_JOIN_EXPEDITION(BM::Stream& Packet);		// 원정대 가입요청 알림
	void Recv_PT_N_C_ANS_JOIN_EXPEDITION(BM::Stream& Packet);		// 원정대 가입요청 응답

	void Recv_PT_N_C_NFY_LEAVE_EXPEDITION(BM::Stream& Packet);		// 원정대 탈퇴요청 알림
	void Recv_PT_N_C_ANS_LEAVE_EXPEDITION(BM::Stream& Packet);		// 원정대 탈퇴요청 응답

	void Recv_PT_N_C_ANS_RENAME_EXPEDITION(BM::Stream& Packet);		// 원정대명 변경 응답

	void Recv_PT_N_C_NFY_CHANGEMASTER_EXPEDITION(BM::Stream& Packet);	// 원정대장 변경요청 알림
	void Recv_PT_N_C_ANS_CHANGEMASTER_EXPEDITION(BM::Stream& Packet);	// 원정대장 변경요청 응답

	void Recv_PT_N_C_NFY_CHANGEOPTION_EXPEDITION(BM::Stream& Packet);	// 원정대옵션 변경요청 알림
	void Recv_PT_N_C_ANS_CHANGEOPTION_EXPEDITION(BM::Stream& Packet);	// 원정대옵션 변경요청 응답
	
	void Recv_PT_N_C_ANS_LIST_EXPEDITION(BM::Stream& Packet);		// 원정대 리스트 요청 응답
	void Recv_PT_N_C_ANS_INFO_EXPEDITION(BM::Stream& Packet);		// 원정대 정보 요청 응답
	void Recv_PT_M_C_ANS_LIST_USER_EXPEDITION(BM::Stream& Packet);	// 유저 리스트 요청 응답

	void Recv_PT_M_C_NFY_EXPEDITION_JOIN(BM::Stream& Packet);			// 타인의 원정대 가입 알림
	void Recv_PT_N_C_NFY_EXPEDITION_MODIFY_MEMBER_STATE(BM::Stream& Packet);	// 원정대원 사망

	void Recv_PT_N_C_NFY_KICKOUT_EXPEDITION(BM::Stream& Packet);	// 원정대 추방요청 알림
	void Recv_PT_N_C_ANS_KICKOUT_EXPEDITION(BM::Stream& Packet);	// 원정대 추방요청 응답

	void Recv_PT_N_C_ANS_TEAM_MOVE_EXPEDITION(BM::Stream& Packet);	// 원정대 팀 이동요청 응답
	void Recv_PT_N_C_NFY_TEAM_MOVE_EXPEDITION(BM::Stream& Packet);	// 원정대 팀 이동요청 알림

	void Recv_PT_N_C_REQ_INVITE_EXPEDITION(BM::Stream& Packet);	// 서버가 유저에게 원정대 초대요청에 응답을 요구
	void Recv_PT_N_C_ANS_INVITE_EXPEDITION_TO_MASTER(BM::Stream& Packet);	// 원정대장에게 유저의 수락여부를 응답

	void Recv_PT_M_C_ANS_NPC_ENTER_EXPEDITION(BM::Stream& Packet);	// 원정대 입장 결과 처리
	void Recv_PT_M_C_NFY_NPC_ENTER_EXPEDITION(BM::Stream& Packet);

	void Recv_PT_M_C_NFY_EXPEDITION_REMAIN_TIME(BM::Stream& Packet);	// 보스 시간제한 알림
	void Recv_PT_M_C_NFY_EXPEDITION_LIMITED_TIME(BM::Stream& Packet);	// 시간제한 실패 알림

	void Recv_PT_M_C_NFY_CHANGE_EXPEDITION_MEMBER_ABIL(BM::Stream& Packet);	// 원정대원의 상태 변화 알림

	void Recv_PT_C_M_ANS_REFRESH_NEED_ITEM_INFO(BM::Stream& Packet);	// 원정대 입장 아이템 보유 여부 응답
protected:
	mutable Loki::Mutex m_Mutex;

	// 원정대원들의 정보가 들어갈 컨테이너와 메모리
	ContExpeditionMember m_Members;
	BM::TObjectPool< SExpeditionMember > m_MemberPool;

	ContInfoWaiter				m_Waiter;

	ContOtherExpedition			m_OtherExpedition;		// 벌룬을 위한 원정대 리스트

	ContOtherPlayer				m_OtherPlayers;			// 원정대 초대를 위한 유저 리스트

	ExpeditionInfoList			m_ExpeditionList;		// 원정대 검색을 위한 원정대 리스트

	ContExpeditionRejectUser	m_RejectedUser;			// 가입요청 또는 초대를 거부한 유저 리스트
	ContWaitExpedition			m_WaitExpedition;		// 가입 수락 대기중인 원정대

	ContUserGuid				m_NotHaveItemMember;	// 키 아이템이 없는 유저 리스트
public:
};

#define g_kExpedition SINGLETON_CUSTOM(PgClientExpedition, Loki::CreateUsingNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_EXPEDITION_PGCLIENTEXPEDITION_H