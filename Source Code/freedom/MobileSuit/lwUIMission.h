#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMISSION_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMISSION_H
#include "Variant/PgMissionInfo.h"
#include "Variant/PgMission_Result.h"

class CEffect;

typedef std::map<BM::GUID, BYTE> CONT_USER_DIRECTION;
typedef std::map<int, int> CONT_RECOMMEND_MISSION_GROUND;	// <그라운드번호, 추천타입> 추천타입 1 : 퀘스트, 2 : 달성률
typedef std::map<int, int> CONT_RECOMMEND_MISSION_QUEST;	// <퀘스트ID, 퀘스트정보>

enum eDEFENSEMODE_TYPE
{
    EDFST_NONE  = 0,
    EDFST_F6    = 0x1,
    EDFST_F7    = 0x2,
    EDFST_F8    = 0x4,

    DEFST_STRATEGIC = EDFST_F7|EDFST_F8,
    DEFST_ALL   = EDFST_F6|EDFST_F7|EDFST_F8,
};

extern int lwRecv_PT_M_C_ANS_MISSION_INFO(lwPacket kPacket);
extern bool lwIsAutoLevel(int const iMissionNo);
extern bool lwIsHaveArcadeMode();
extern void lwSetMissionGray(lwUIWnd kWnd, int const iMissionNo, int const iLevel);
extern int lwGetMissionMapCount();
extern int lwGetMissionDifficultyLevel();
extern int lwGetMissionNo();
extern void lwSetMissionNo(int iMissionNo);
extern int lwGetDefence7MinLevel();
extern int lwGetDefence7MaxLevel();
extern bool lwDefence7EnterLevelCheck();
extern bool lwIsOpenDefenseModeTable(eDEFENSEMODE_TYPE const eType, int const iMissionNo);
extern char const* lwGetMissionImgPath(int const iNum);
extern int lwHiddenRewordItemView(int const iItemNo);
extern int lwHiddenRewordItemView2(int const iItemCount);
extern int lwHiddenRewordItemViewIcon(int const iItemCount);
extern int lwGetMissionKey( int const iMissionNo );
extern void lwSendMemo(char const* pkStr);
extern bool lwSendTakeCoupon(char const* pkStr);
extern void lwSendReqRankInfo();
extern int lwMissionRankingRewardItem( const int iMissionNo, const int iMissionLevel, const int iMissionRanking );
extern void lwSetMissionCountMap(int const iCount);
extern void lwSetMissionDifficultyLevel(int const iLevel);
extern int lwCalcMissionCount(int const iNum, int const iDiff);
extern lwWString lwSetMissionSelectMapName(int const iLevel, int const iIndex);
extern void lwSend_PT_C_N_REQ_MISSION_RESTART();
extern lwUIWnd lwCallMissionRetryUI();
extern bool lwCallMissionRebirthUI(bool bUseCoin=false);
extern void lwDisplayIcon_InfallibleSelection();
extern void lwSend_REQ_DEFENCE_INFALLIBLE_SELECTION(BYTE const kDirection);
extern void lwUpdate_DefenceSelectUI(BM::Stream &rkPacket);
extern void lwSet_DefenceSelectUI(int const iStageNo);
extern bool lwEnter_DefenceMission(int const iMissionNo);
extern bool Find_DefenceConsumeItem(ContHaveItemNoCount& rkContItemNoCount, int const iMissionNo);
extern void lwSetMission_Defence_Hp(lwGUID kGuid, lwUIWnd kWnd);
extern void lwCallRankInfo(int const iMissionKey, int const iLevel);
extern void lwCallRankTop(int const iMissionKey, int const iLevel);
extern bool CheckScenarioMissionEnter();
extern bool lwCheckHaveMissionEnterItem(int iIndex);
extern void lwUpdateRewardItemCount(size_t iInfallibleItemCount);

//extern void MissionQuestUISetting(int const iCardStart = 0);
//extern void lwMQCard_Select(lwUIWnd UISelf);
//extern void lwMQCard_Update(int const iIndex);
//extern void lwMQCard_QuestInfo(DWORD dwQuestID);
//extern bool lwMQCard_ForwardID(lwUIWnd UISour, char const* pDest);
//extern void lwMQCard_Scale(lwUIWnd UISelf, float const MQCLStartTime, float const VScale);
//extern void lwMQCard_TimeText(lwUIWnd UISelf, int const iTime);

extern void lwSetMissionLevelingText(lwUIWnd UISelf, int const iNum);
extern bool lwGetMissionLevelArea(int const iNum);
extern lwWString lwGetMissionLevelingText(lwUIWnd UISelf, int const iNum);
extern bool lwIsMissionLastStage();

extern void Mission_Item_Info(SMissionInfo& kMission);
extern void Mission_Scenario_Info(SMissionInfo& kMission,VEC_INT const& VecNeedItem);
extern void Mission_PartyList_Info(SMissionInfo& Mission);

extern void Mission_Hidden_Info(SMissionInfo& kMission);
extern void Mission_Item_Info_Event(SMissionInfo& kMission);

extern void Mission_Scenario_Not_Have_Arcade_Info(SMissionInfo& kMission);
extern void Mission_Item_Not_Have_Arcade_Info(SMissionInfo& Mission);

extern int lwRecommendMissionQuestCount(int const index);
extern int lwRecommendMissionAchievementCount(int const index);
extern int lwGetMissionRequiredItemNo(int const index);
extern void lwSetEventMission(int);
extern int lwGetEventMission(void);
extern void lwMissionUISetQuest(lwUIWnd lwWnd);
extern bool RecommendMisssionMap(int const index,
								 CONT_DEF_MISSION_ROOT const * pContDefMission,
								 CONT_DEF_MISSION_CANDIDATE const * pContDefMissionCandi,
								 ContUserQuestState const & QuestCont,
								 CONT_RECOMMEND_MISSION_GROUND &MissionGround,
								 CONT_RECOMMEND_MISSION_QUEST &MissionQuest);

extern void lwDisPlayMissionUpdateScore(int const iType, int const iScore);
extern void lwOnSelectGadaCoinBox();
extern int lwGetMissionType();
extern void lwGetMissionPlayCount(lwUIWnd UISelf);
extern float GetMissionCountAddExpText(int iMissionCount);
extern bool lwGetMissionExpUI(int const iMissionNo);
extern bool IsDeffenseMode( int const iMissionNo );
extern bool Result_DefenceInfallibleSelection(BM::Stream& rkPacket);
extern bool Result_DefenceTimePlus(BM::Stream& rkPacket);
extern void Result_DefenceHpPlus(BM::Stream& rkPacket);
extern int lwGetDefenceResultStage(int const iStage);
extern void lwDisplayIcon_DefenceSelectUI(int const iBuildIndex, int const iItemNo);

extern bool GetDefenceAddMonsterInfo(int const iStageNo, int const iSelect_SuccessCount, CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type& kElement);
extern bool GetDefenceStage(int const iStage, CONT_MISSION_DEFENCE_STAGE_BAG::mapped_type& kElement);
extern bool GetDefenceAddMonster(int const iMonsterGroupNo, int const iSelect_SuccessCount, CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type& kElement);
extern void lwCallDefenceMissionReward(int const iNowStage, int const iMaxStage);
extern void lwUpdateDefenceMissionGuardian(lwUIWnd kWnd);
extern void lwUpdateDefenceMissionSkill(lwUIWnd kWnd);
extern void lwDisplayStrategicPoint(lwUIWnd kWnd);
extern bool lwStrategyDefenceResultUI(lwUIWnd kWnd=lwUIWnd(NULL,false));
extern void lwStrategyItemDoAction(int const iIndex);
extern void lwMissionMgrInit();
extern lwPoint3 lwGetDefenceGuardianNeedPos();
extern void lwViewGuardianDetectRangeEffect(lwUIWnd kWnd);
extern void lwClearGuardianDetectRangeEffect();
extern void lwOnDefenceGuardianMouseOver(lwUIWnd kWnd);
extern void lwOnDefenceGuardianMouseOut();
extern bool lwIsGuardianInstall(lwGUID kGuid);
extern bool lwIsGuardianInstallStr(char const* pkTriggerName);
extern void lwSetGuardianInstallLoc(char const* pkLoc);
extern void lwCallGuardianUI(lwGUID kGuid);
extern void lwCallInstallGuardian(lwUIWnd kWnd);
extern void lwCallUpgradeGuardian(lwUIWnd kWnd);
extern void lwCallRemoveGuardian();
extern void lwCallDefenceSkillLearn(lwUIWnd kWnd);
extern void lwOnCallDefenceLearnSkill(lwUIWnd kWnd);
extern CEffect* GetGuardianSkill(int const iSkillNo, BM::GUID const & kGuardianGuid);
extern void lwOnEnterGuardianInstallTrigger(char const* pkTriggerName, lwPoint3 kPos);
extern void lwOnLeaveGuardianInstallTrigger();
extern void lwSetScenarioMissionMapCount(int const iNum, int const iDiff);
extern size_t GetNeedInfallibleItemCount(PgMission_ResultC const& rkMissionResult);

extern int g_iMissionNo;
extern int g_iMissionKey;

typedef struct tagGuardianInfo
{
	tagGuardianInfo():iGuardianNo(0),iMonsterNo(0){}
	int iGuardianNo;
	int iMonsterNo;
}GuardianInfo;

typedef std::map< std::wstring, SGuardian_Install > CONT_GUARDIAN_INSTALL;

typedef struct tagMissionUnitPos
{
	tagMissionUnitPos():iIconType(0),iTeam(0),kPos(0,0,0){}

	BM::GUID kGuid;
	NiPoint3 kPos;
	int iIconType;
	int iTeam;
} SMissionUnitPos;
typedef std::vector< SMissionUnitPos > CONT_MISSION_UNIT_POS;

class PgMissionMgr
{
public:
	typedef std::map<int, int> CONT_STRATEGIC_POINT;	// 팀, 포인트
	typedef std::map<std::wstring, NiPoint3> CONT_TRIGGERPOS;
	typedef std::map<std::wstring, BM::GUID> CONT_ADDUNIT;

public:
	PgMissionMgr();
	~PgMissionMgr();

	void Init();
	void Clear();

	bool ProcessMsg(WORD const wPacketType, BM::Stream &rkPacket);
	bool IsMode6ComplatedQuest()const;
	bool IsMode7ComplatedQuest()const;
	std::wstring GetClearQuestOriginalText(int const iLevel)const;
	std::wstring GetClearQuestText(int const iLevel)const;
	int GetStrategicPoint(int const iTeam)const;
	int GetStageClearPoint()const;
	int GetDefenceMyItem(int const iIndex)const;
	void StrategyItemDoAction(int const iIndex);
	void GurdianDoAction(int const iGuardinaNo);
	void MissionStageSubInfo()const;
	void StratagyMissionStageResult()const;
	void StratagyMissionStageEnd(int const iSec)const;
	std::wstring GetNeedGuardianPosStr()const;
	NiPoint3 GetNeedGuardianPos()const;
	std::wstring GuardianPosStr(BM::GUID const kGuid)const;
	bool IsGuardianInstall(std::wstring const& kTriggerName)const;
	bool IsGuardianInstall(BM::GUID const kGuid)const;
	bool IsGuardianInstall()const;

	void ViewGuardianDetectRangeEffect(XUI::CXUI_Wnd* pkWnd);
	void ClearGuardianDetectRangeEffect();
	void OnGuardianMouseOver(lwUIWnd & kWnd);
	void OnGuardianMouseOut();
	void RemoveTempGuardian();

	void CallGuardianUI(BM::GUID const kGuid);
	void CallInstallGuardian(XUI::CXUI_Wnd* pkWnd);
	void CallUpgradeGuardian(lwUIWnd & kWnd);
	void CallRemoveGuardian();
	void GurdianUpgradeDoAction(int const iGuardianNo);
	void GurdianRemoveDoAction();
	void CallDefenceSkillLearn(XUI::CXUI_Wnd* pkWnd);
	void OnCallDefenceLearnSkill(XUI::CXUI_Wnd* pkWnd);

	bool MakeGuardianToolTipText(int iIconKey, std::wstring &wstrTextOut);
	bool MakeStratagySkillToolTipText(int const iGuardianNo, int iSkillNo, std::wstring &wstrTextOut, std::wstring& wstrType, bool bNextLevel);

	CONT_MISSION_UNIT_POS const& GetMinimapGuardianIcon()const{return m_kMinimapGuardian;}

	void SetUI(XUI::CXUI_Wnd* pkWnd, int const iGuardianNo, int const iMonsterNo, BM::vstring & vPoint, bool const bGrayScale);
	void SetGuardianInstallLoc(std::wstring const& kLoc){m_kGuardianUpgradeLoc = kLoc;}
	BM::GUID GetSelectGuardianGuid()const;

	void PgMissionMgr::OnEnterGuardianInstallTrigger(BM::vstring const & rkTriggerName, NiPoint3 kPos);
	void PgMissionMgr::OnLeaveGuardianInstallTrigger();
	void SetClearQuestNoLv6(VEC_INT const& kContQuest);
	void SetClearQuestNo(VEC_INT const& kContQuest);
	int GetClearQuestNo(int const Level)const;
private:
	void SetGuardianTransforUI(XUI::CXUI_Wnd* pkWnd, SMISSION_DEFENCE7_GUARDIAN const & kDef);
	void SetGuardianUpgradeUI(XUI::CXUI_Wnd* pkWnd, int const iGuardianNo, SMISSION_DEFENCE7_GUARDIAN const & kDef);
	void UpdateGuardianSkillUI();
	void UpdateEffectGuardianPos();
	void UpdateGuardianName();
	void SetDefenceMinimapCamera()const;
	void UpdateMinimapGuardian();
	void UpdateMinimapItem();

	int	m_iMode6ClearQuestNo;						//디펜스모드 F6 ClearQuest 번호
	int m_iMode7ClearQuestNo;						//전략모드 F7 ClearQuest 번호
	VEC_INT m_kTempGuardianSlot;
	SGuardian_Install m_kTempGuardianInstall;		//임시 설치 가디언
	CONT_GUARDIAN_INSTALL m_kGuardianInstall;		//실제 설치된 가디언
	CONT_MISSION_UNIT_POS m_kMinimapGuardian;		//미니맵에표시할 가디언
	CONT_STRATEGIC_POINT m_kContStrategicPoint;	
	CONT_TRIGGERPOS m_kContGuardianTriggerPos;		//맵에 박혀있는 가디언 트리거 위치저장
	VEC_INT m_kContDefenceMyItem;
	CLASS_DECLARATION_S(int, NowStage);
	CLASS_DECLARATION_S(int, StagePoint);
	PgDefenceMissionSkill m_kDefenceSkill;
	std::wstring m_kGuardianUpgradeLoc;	//가디언 업그레이드 위치
	int m_iGuardianUpgradeNo;
	int m_iSelectGuardianDetectRangeSlot;//Upgrade detect rage effect
	int m_iStageClearPoint;
	int m_iEnterGuardianTriggerEffectSlotNo;

	mutable Loki::Mutex m_kMutex;
};

#define g_kMissionMgr SINGLETON_STATIC(PgMissionMgr)
#endif //FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMISSION_H