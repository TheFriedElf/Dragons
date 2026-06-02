#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_H

#include "Variant/PgEmporia.h"

class lwWString;

typedef enum eGuildListMode
{
	GLM_None = 0,
	GLM_Grade = 1, //Default
	GLM_Location = 2,
	GLM_Max,
} EGuildListMode;

class lwUIWnd;
namespace PgGuildMgrUtil
{
	extern int const iMsgOnlyMaster;

	void CalcGuidToColor(BM::GUID const &rkGuid, NiColorA& rkOutColor);
	void CalcSizetToColor(size_t const iSize, std::wstring& rkOut);
	void CallGuildChangeMark1(int const iItemNo, SItemPos const& rkItemInvPos);
	bool CallChangeMarkYesNo(lwUIWnd kTopWnd, BYTE const cNewGuildMark);
	void SendReqChangeGuildMark(XUI::CXUI_Wnd* pkTopWnd);
	int GetMaxGuildMaxCount();
};

struct SGuildOtherClientInfo : public SGuildOtherInfo
{
	SGuildOtherClientInfo()
		: SGuildOtherInfo(), kGuidSet()
	{
	}

	SGuildOtherClientInfo(SGuildOtherInfo const& rhs)
		: SGuildOtherInfo(rhs), kGuidSet()
	{
	}

	void Add(BM::GUID const& rkCharGuid)
	{
		kGuidSet.insert(rkCharGuid);
	}

	void Del(BM::GUID const& rkCharGuid)
	{
		kGuidSet.erase(rkCharGuid);
	}

	bool Empty() const
	{
		return kGuidSet.empty();
	}

	ContGuidSet kGuidSet;
};

typedef struct tagApplicationState // 길드 신청 상태
{
	BM::GUID kGuildGuid;
	BYTE byApplicantionState;
	__int64 i64GuildEntranceFee;

	tagApplicationState()
	{
		Init();
	}

	void Init()
	{
		kGuildGuid.Clear();
		byApplicantionState = AS_NONE;
		i64GuildEntranceFee = 0i64;
	}

	void SetState( BM::GUID const & rkGuildGuid, BYTE const byState, __int64 i64Fee )
	{
		kGuildGuid = rkGuildGuid;
		byApplicantionState = byState;
		i64GuildEntranceFee = i64Fee;
	}
}SApplicationState;


typedef std::map< BM::GUID, SGuildOtherClientInfo > ContOtherGuild;
typedef std::set< BM::GUID > ContActorGuid;
typedef std::map< BM::GUID, ContActorGuid > ContWaiter;
typedef std::vector< SEntranceOpenGuild > CONT_ENTRANCE_OPEN_GUILD_LIST;
typedef std::map< BM::GUID, SGuildEntranceApplicant > CONT_MY_GUILD_ENTRANCE_APPLICANTS; // characterGuid


class PgGuildMgr
{
public:// 상수

public:
	PgGuildMgr();
	~PgGuildMgr();

	void Clear();

	bool ProcessPacket(BM::Stream *pkPacket);

	unsigned short Level()const		{ return m_kBasicInfo.sLevel; }
	BYTE Emblem()const				{ return m_kBasicInfo.cEmblem; }

	bool PreCreateGuild(std::wstring &rkGuildName);
	bool CreateGuild();
	bool DestroyGuild();
	bool LeaveGuild();
	bool KickUser(BM::GUID const &rkCharGuid);
	bool ChangeOwner(BM::GUID const &rkNewOwnerGuid);

	bool GetMemberByName(std::wstring const &rkName, SGuildMemberInfo& rkMemberInfo) const;
	bool GetMemberByGuid(BM::GUID const &rkCharGuid, SGuildMemberInfo& rkMemberInfo) const;

	bool ReqOtherGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const &rkActorGuid);

	bool ReqBasicInfo(BM::GUID const &rkGuildGuid);
	bool ReqMemberInfo(BM::GUID const &rkGuildGuid);
	bool ReqGuildEntranceApplicantList(BM::GUID const &rkGuildGuid);	
	bool GetGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const& rkCharGuid, SGuildOtherInfo& rkOut); // 캐릭터가 있을 때
	bool GetGuildInfo(BM::GUID const &rkGuildGuid, SGuildOtherInfo& rkOut) const; // 시스템 전용
	bool DelGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const& rkCharGuid);

	//Packet
	void ReqJoinGuild(BM::GUID const &rkCharGuid);//Guid로 초대
	void ReqJoinGuild(std::wstring const &rkCharName);//이름으로 초대
	void ReqJoinAnswer(BM::GUID const &rkGuildGuid, bool const bJoin);//응답
	void ReqGuildAddSkill(int const iSkillNo);
	
	bool IsHaveEntranceGuild() const; // 가입 신청한 길드가 있는가?
	void ReqGuildEntrance();// 길드가입 신청
	void ResultGuildEntrance(bool const bRet);
	
	void InitApplicantState() { m_kApplicationState.Init(); }
	void SetGuildApplicationState(BM::GUID const &rkGuildGuid, BYTE byState, __int64 i64EntranceFee);
	const SApplicationState& GetGuildApplicationState() const { return m_kApplicationState; }
	
	void ReadEntranceOpenGuildFromPacket(BM::Stream& rkPacket);
	bool IsEntranceOpenGuildEmpty() { return m_kEntranceOpenGuild.empty(); }	

	bool IsOwner(BM::GUID const &rkCharGuid) const;
	bool IsMaster(BM::GUID const &rkCharGuid) const;
	bool IamHaveGuild()const;
	bool HaveGuildInventory();
	bool Member_Find_ByGuid(BM::GUID const& kCharGuid, SGuildMemberInfo& kMemberInfo);

	bool UpdateGuildUI(XUI::CXUI_Wnd* pkGuildTopWnd = NULL) const;
	bool UpdateGuildSkillUI(XUI::CXUI_Wnd* pkSkillTopWnd = NULL) const;
	
	void CallGuildApplicationUI(int const iCustomDataAsInt);
	void WrapperUpdateEntranceOpenGuildUI(int const iCurrentPage = 0);
	void SearchGuild(lwWString lwSearchGuildName);
	void InitSearchGuild() { m_kSearchEntranceOpenGuild.clear(); }

	__int64 GetExp()const{return m_kBasicInfo.iExperience;}
	void RefreshExp( __int64 const i64Exp );
	
	void AddEntranceApplicant(const SGuildEntranceApplicant& kNewApplicant);
	void DelEntranceApplicant(BM::GUID const& rkCharGuid);
	void ClearEntranceApplicant() { m_kGuildEntranceApplicant.clear(); } // 길드 가입 신청서 전부 클리어
	CONT_MY_GUILD_ENTRANCE_APPLICANTS const& GetGuildEntranceApplicantList() const { return m_kGuildEntranceApplicant; }
	
	SSetGuildEntrancedOpen const& GetGuildEntranceOpenInfo() const { return m_kGuildEntranceOpen; }
	PgLimitClass const& GetMercenaryInfo() const { return m_kSetMercenary; };

	PgInventory* GetInven()							{ return &m_kInv; }
	PgInventory const* GetInven() const				{ return &m_kInv; }
	void SwapInven( PgInventory & rkInven )			{ m_kInv.Swap( rkInven );}

	void SetGuildInvView( const int iGuildInvViewGrp )	{ m_iGuildInvView = iGuildInvViewGrp; }
	const int GetGuildInvView() const					{ return m_iGuildInvView; }

protected:
	bool AddMember(const SGuildMemberInfo& rkMember);
	bool DelMember(BM::GUID const &rkCharGuid);

	bool AddGuildInfo(const SGuildOtherInfo& rkOtherInfo);

	bool OtherGuildInfo(const SGuildOtherInfo& rkInfo);
	bool GetGradeName(int const iGrade, std::wstring& rkOut) const;

	void ReadListFromPacket(BM::Stream& rkPacket,bool const bUpdate);
	void ReadGuildEntranceApplicantListFromPacket(BM::Stream& rkPacket);

	void ReadEmporiaInfoFromPacket( BM::Stream &rkPacket );

	void UpdateEntranceOpenGuildUI(int const iCurrentPage, const CONT_ENTRANCE_OPEN_GUILD_LIST& rkContGuild);

public:
	bool SetPlayerGuildGuid(BM::GUID const &rkCharGuid, BM::GUID const &rkGuildGuid);
	bool UpdateActorGuildName(BM::GUID const &rkActorGuid);	
	
	std::wstring const &GuildName()const { return m_kBasicInfo.kGuildName;}
	BM::GUID const &GuildGuid()const{return m_kBasicInfo.kGuildGuid;}
	SGuildEmporiaInfo const &GetEmporiaInfo()const{return m_kEmporiaInfo;}
	void SetEmporiaInfo(SGuildEmporiaInfo const& rInfo){ m_kEmporiaInfo = rInfo; }
	CLASS_DECLARATION_NO_SET(bool, m_bWait, Wait);
	CLASS_DECLARATION_S_NO_SET(std::wstring, TempGuildName);
	CLASS_DECLARATION_S(int, TempGuildEmblem);
	CLASS_DECLARATION_S(EGuildListMode, ListMode);
	CLASS_DECLARATION_S_NO_SET(std::wstring, GuildNotice);

	SApplicationState m_kApplicationState; // 길드가입처리 상태

	const BYTE GetGuildInvAuthority_In()const	{ return m_kBasicInfo.abyInvAuthority[1]; }
	const BYTE GetGuildInvAuthority_Out()const	{ return m_kBasicInfo.abyInvAuthority[3]; }
	void SetGuildInvAuthority(const BYTE byIn, const BYTE byOut )	{ m_kBasicInfo.abyInvAuthority[1] = byIn; m_kBasicInfo.abyInvAuthority[3] = byOut; }

protected:
	mutable Loki::Mutex m_kMutex;

	SGuildBasicInfo m_kBasicInfo;
	SGuildEmporiaInfo m_kEmporiaInfo;
	SSetGuildEntrancedOpen m_kGuildEntranceOpen;
	CONT_MY_GUILD_ENTRANCE_APPLICANTS m_kGuildEntranceApplicant;	
	CONT_ENTRANCE_OPEN_GUILD_LIST m_kEntranceOpenGuild;
	CONT_ENTRANCE_OPEN_GUILD_LIST m_kSearchEntranceOpenGuild;
	PgLimitClass	m_kSetMercenary;
	ContGuildMember m_kMember;
	ContGuildGrade m_kMemberGrade;
	PgMySkill m_kGuildSkill;
	PgInventory m_kInv;

	int m_iGuildInvView; //
	int m_iRecommendGuildRank; // 추천길드의 랭킹

	//캐슁
	ContOtherGuild m_kOtherInfo;//
	ContWaiter m_kRequester;
};

#define g_kGuildMgr SINGLETON_STATIC(PgGuildMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_H