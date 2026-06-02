#ifndef FREEDOM_DRAGOINCA_CONTENTS_BATTLESQUARE_PGBATTLESQUARE_H
#define FREEDOM_DRAGOINCA_CONTENTS_BATTLESQUARE_PGBATTLESQUARE_H

namespace BattleSquareUI
{
	bool RegisterWrapper(lua_State *pkState);
	void NoticePoint(int const iPrevPoint, int const iNextPoint, bool const bUse = true);
	
	typedef enum eChangeReward_State
	{
		ECRS_NOTHING				= 0,
		ECRS_PREV_EFFECT_STARTED	= 1,
		ECRS_NEXT_EFFECT_STARTED,

	}E_CHANGE_REWARD_STATE;
}

typedef enum
{
	E_LEAD_PARE = 0,
	E_LEAD_RED = 1,
	E_LEAD_BLUE = 2
}E_LeadTeam;

//
class PgBSNoticeMng
{
	typedef std::vector< std::wstring > CONT_NOTICE;
public:
	PgBSNoticeMng();
	~PgBSNoticeMng();

	void AddNotice(int const iTTW);
	void Init();
	void Update();

private:
	CLASS_DECLARATION_S(bool, Enable);
	float m_fLastNoticeTime;
	CONT_NOTICE m_kContNotice;
	size_t m_iCurNotice;
	float m_fInitTime;
};

//
class PgBSKillSoundMng
{
public:
	PgBSKillSoundMng();
	~PgBSKillSoundMng();

	void Update();
	void AddKill();
	void Clear();

private:
	size_t m_iKillCount;
	float m_fLastKillTime;
	NiAudioSource* m_pkNowAudio;
	std::string m_kNextAudio;
};

//
typedef struct tagBSClientGameInfo
{
	tagBSClientGameInfo(BM::Stream& rkPacket);
	tagBSClientGameInfo(tagBSClientGameInfo const& rhs);
	~tagBSClientGameInfo();

	bool operator < (tagBSClientGameInfo const& rhs);

	EBS_GAME_STATUS eStatus;
	int iCurUserCount;
	SBSGame kBSGame;
} SBSClientGameInfo;
typedef std::list< SBSClientGameInfo > CONT_BS_CLIENT_INFO;

//
typedef struct tagBSItemUnitPos
{
	tagBSItemUnitPos(BM::Stream& rkPacket);

	BM::GUID kGuid;
	NiPoint3 kPos;
	int iIconType;
} SBSItemUnitPos;
typedef std::list< SBSItemUnitPos > CONT_BS_ITEM_UNIT_POS;

//
typedef struct tagBSGameStatusInfo
{
	tagBSGameStatusInfo();
	tagBSGameStatusInfo(tagBSGameStatusInfo const& rhs);
	~tagBSGameStatusInfo();

	void Clear();
	void ReadFromPacket(BM::Stream& rkPacket);
	bool operator ==(tagBSGameStatusInfo const& rhs) const;

	int iGameIDX;
	EBS_GAME_STATUS eStatus;
	int iChannelNameTextID;
	int iLevelMin;
	int iLevelMax;
	int iCurUserCount;
	int iMaxUser;
	__int64 iDiffTime;
} SBSGameStatusInfo;
typedef std::list< SBSGameStatusInfo > CONT_BS_GAMES_STATUS_INFO;

//
typedef struct tagBSUnitPos
{
	tagBSUnitPos();
	tagBSUnitPos(tagBSUnitPos const& rhs);
	tagBSUnitPos(BM::Stream& rkPacket);
	~tagBSUnitPos();

	void ReadFromPacket(BM::Stream& rkPacket);

	BYTE iTeamNo;
	NiPoint3 kPos;
	//int iIconNo;
	short sIconCount;
} SBSUnitPos;
typedef std::map< BM::GUID, SBSUnitPos > CONT_BS_UNIT_POS;

//
typedef struct tagBSMedalPos
{
	tagBSMedalPos();
	tagBSMedalPos(tagBSMedalPos const& rhs);
	tagBSMedalPos(BM::Stream& rkPacket);
	~tagBSMedalPos();

	void ReadFromPacket(BM::Stream& rkPacket);

	POINT3I  kPos;
} SBSMedalPos;
typedef std::map< BM::GUID, SBSMedalPos > CONT_BS_MEDAL_POS;

//
class PgBattleSquareMng
{
public:
	PgBattleSquareMng();
	~PgBattleSquareMng();

	void Clear();
	void ClearNotice();
	void Update();
	void OnRecvPacket(WORD const wPacketType, BM::Stream& rkPacket);

	void ReadFromPacket(BM::Stream& rkPacket);
	bool GetCanEnterGameIDX(int const& iGameIDX) const;
	bool IsEnterGameIDX(int& iGameIDX) const;
	void CallTeamUI();
	void FlipTeamUI();
	int GetTeam(BM::GUID const& rkCharGuid) const;
	void Init();
	void AddNotice(int const iTextTableNo);
	void RegistMemberTargetObserver();
	void SetAnimateWnd(XUI::CXUI_Wnd*& pkAnimateWnd, std::wstring& strOriginal, int const iLimit = 10);
	
	void UpdateBattleSquareChannel(BM::Stream& rkPacket);

	void UpdateMedalCount(int const iMyMedal=0, int const iTeamRedMedal=0, int const iTeamBlueMedal=0) const;
	void UpdateMovePoint(int const iMyPoint=0, int const iTeamRedPoint=0, int const iTeamBluePoint=0);
	void UpdateLeadTeam(int const iTeamRedPoint=0, int const iTeamBluePoint=0);

private:
	void ProcessNfyInfo(CONT_BS_GAMES_STATUS_INFO const& kContNfyInfo);

protected:
	PgBSKillSoundMng m_kKillCountMng;
	CONT_BS_GAMES_STATUS_INFO m_kContNoticeInfo;
	PgBSNoticeMng m_kInGameNotice;

private:
	CLASS_DECLARATION_S_NO_SET(PgBSGame, BSGame);
	CLASS_DECLARATION_S_NO_SET(int, ManCount);
	CLASS_DECLARATION_S_NO_SET(CONT_BS_ITEM_UNIT_POS, ContItemUnitPos);
	CLASS_DECLARATION_S_NO_SET(CONT_BS_UNIT_POS, ContUnitPos);
	CLASS_DECLARATION_S_NO_SET(CONT_BS_MEDAL_POS, ContMedalPos);
	CLASS_DECLARATION_S_NO_SET(bool, DefaultTabKey);
	PgBSTeam m_kRedTeam;
	PgBSTeam m_kBlueTeam;
	bool m_bPreTabKeyDownStatus;
	E_LeadTeam m_eLeadTeam;	//0 - Pare, 1 - Red, 2 - Blue
};
#define g_kBattleSquareMng SINGLETON_STATIC(PgBattleSquareMng)

#endif // FREEDOM_DRAGOINCA_CONTENTS_BATTLESQUARE_PGBATTLESQUARE_H