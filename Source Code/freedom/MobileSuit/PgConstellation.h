#ifndef FREEDOM_DRAGONICA_CONTENTS_CONSTELLATION_PGCONSTELLATION_H
#define FREEDOM_DRAGONICA_CONTENTS_CONSTELLATION_PGCONSTELLATION_H

#include "Variant/PgConstellation.h"
#include "Variant/PgSimpleTime.h"
#include "lwUI.h"

using namespace Constellation;

int const MIN_CONSTELLATION_DUNGEON = 1;
int const MAX_CONSTELLATION_DUNGEON = 12;

int const MIN_CONSTELLATION_DIFFICULTY = 1;
int const MAX_CONSTELLATION_DIFFICULTY = 4;

namespace lwConstellationEnterUIUtil
{
	void RegisterWrapper(lua_State *pkState);
	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);

	void lwReqEnterConstellation(int const Position, int const Difficulty);
	void lwReqEnterConstellation_Boss();
	bool lwIsInDay(int const Position); 
	bool lwEnable(int const Position, int const Difficulty);
	void lwUpdateConstellationInfo(int const Position, int const Difficulty);	// 입장 UI에 던전정보 세팅

	void lwReqConstellationPartyList(lwUIWnd lwWnd);	// 파티리스트 요청
	void lwFakeUpdateConstellationPartyList(lwUIWnd lwWnd); // 요청 실패 시 눈속임용 업데이트
	void lwOnEnter_ConstellationParty(lwUIWnd lwWnd);	// 입장 요청

	void lwInitCashItem(); // 캐쉬아이템 사용 초기화
	void lwInitConstellationUI(lwUIWnd lwWnd);	// 던전 정보 UI 초기화

	bool lwCheckCondition_CashItem();
	bool lwUseConstellationCashItem(lwUIWnd lwWnd, bool const bUse);

	int lwGetMinConstellationDungeon();
	int lwGetMaxConstellationDungeon();

	void lwReqConstellationMission();
	void lwDifficultyName(int const Position, int Difficulty);
	void lwDifficultyStory(int const Position, int Difficulty);
	bool lwDifficultyLegend(int const Position);

	void lwConstellationUpdate();
	void lwConstellationResultClose();

	bool lwReqAddReward(void);
	
	lwWString lwGetConstellationPartyListToolTip(lwGUID lwGuid);
}

class PgConstellationEnterUIMgr
{
public:
		typedef enum {
		ES_NONE			= 0,
		ES_OPEN,
		ES_STEP_1,
		ES_BONUS_PROCESS_READY,
		ES_BONUS_PROCESSING,
		ES_RESET,
		ES_RESET_OK,
		}E_STATE;

		typedef enum {
			EINSUR_DESTORY	= 0,
			EINSUR_MINUS_2,
			EINSUR_MINUS_1,
			EINSUR_ZERO,
			EINSUR_PLUS_1,
			EINSUR_PLUS_2,
			EINSUR_MAX,
		}E_INSURANCETYPE;

		typedef enum {
		EPS_INIT,
		EPS_NORMAL,
		EPS_SENDING,
		EPS_RECV_WAIT,
		EPS_RECV,
		EPS_FINISH,
	}E_PROCESS_STATE;

	PgConstellationEnterUIMgr();
	~PgConstellationEnterUIMgr();

	friend void lwConstellationEnterUIUtil::lwConstellationUpdate();
	friend void lwConstellationEnterUIUtil::lwDifficultyName(int const Position, int const Difficulty);
	friend void lwConstellationEnterUIUtil::lwDifficultyStory(int const Position, int Difficulty);
	friend bool lwConstellationEnterUIUtil::lwDifficultyLegend(int const Position);
	
	//결산 프로세스
	bool ConstellationIsReset()const;
	void ConstellationResultUpdate();
	bool ConstellationResultOpenningUpdate(float fCurTime);
	void ConstellationClear(bool const bClearState=false);
	E_STATE GetState()const { return m_eState; }

	void Clear();

	void LoadFile(const char* pcFilename);
	TiXmlNode const* ParseXml(TiXmlNode const* pNode
							, CONT_DEFITEM const* pDefItem
							, CONT_DEFMONSTER const* pDefMonster
							, CONT_DEF_QUEST_REWARD const* pDefQuestReward
							, int & rOutGroundNo);

	void CallConstellationUI();													// 입장 UI 호출
	void InitConstellationUI(XUI::CXUI_Wnd * pWnd);								// 입장 UI 초기화

	void ReqEnterConstellation(int const Position, int const Difficulty);		// 입장 요청
	void ReqEnterConstellation_Boss();	// 보스방 입장 요청

	DWORD CheckCondition(ConstellationKey const& Key);							// 조건 체크
	void ConditionErrorMsg(DWORD const dwRet);

	void RegisterWrapper(lua_State *pkState);
	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);			// 패킷 처리

	void AddMission();

	bool IsInDay(int const Position);
	bool Enable(int const Position, int const Difficulty);
	int IsInDifficulty(int const Position);

	void GetString_Difficulty(int const Difficulty, BM::vstring & OutStr);
	void GetString_Position(int const Position, BM::vstring & OutStr);
	void GetString_DayofWeek(VEC_WSTRING const& DayofWeek, BM::vstring & OutStr);

	void UpdateConstellationInfo(int const Position, int const Difficulty);	// 입장 UI에 던전정보 세팅
	void UpdateConstellationInfo(XUI::CXUI_Wnd * pWnd, int const Position, int const Difficulty);	// 입장 UI에 던전정보 세팅
	void UpdateRewardInfo();

	void ReqConstellationMission(); // 던전임무 업데이트
	bool GetConstellationPartyListToolTip(BM::GUID const& guid, std::wstring & TooltipStr); //툴팁

	void InitCashItem();
	bool UseConstellationCashItem(XUI::CXUI_Wnd * pWnd, bool const bUse);	// 아이템 사용 선택 및 취소
	bool CheckCondition_CashItem();	// 캐쉬아이템 체크(프리패스)

	// 파티 창 ///////////////////////////////////////////////////////////////////////////////////////////////////
	void ReqConstellationPartyList(XUI::CXUI_Wnd * pWnd);
	void FakeUpdateConstellationPartyList(XUI::CXUI_Wnd * pWnd);
	void OnRefresh_DungeonPartyList(BM::GUID const& KeyGuid, CONT_INDUN_PARTY_CHANNEL const& PartyList);
	void AddItem_DungeonPartyList(XUI::CXUI_List * pList, short const sChannel, SIndunPartyInfo const& rInfo);

	void OnEnter_ConstellationParty(XUI::CXUI_Wnd * pPartyWnd); // 입장 요청
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 패킷 처리 /////////////////////////////////////////////////////////////////////////////////////////////////
	void Recv_PT_M_C_NFY_ENTER_CONSTELLATION(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_MISSION(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_MODIFY_MISSION(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_MISSION_FAIL(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_RESULT_WAIT(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_RESULT(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_CONSTELLATION_CASH_REWARD(BM::Stream& Packet);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	// 입장 체크 /////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD CheckCondition_DayofWeek(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo);
	DWORD CheckCondition_Level(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo);
	DWORD CheckCondition_Item(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo);
	DWORD CheckCondition_Quest(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 던전 정보 갱신 ////////////////////////////////////////////////////////////////////////////////////////////
	void UpdateConstellationInfo_Name(XUI::CXUI_Wnd * pWnd, int const Difficulty, int const TextNo);
	void UpdateConstellationInfo_Day(XUI::CXUI_Wnd * pWnd, VEC_WSTRING const& DayofWeek,int const Position, int const Difficulty);
	void UpdateConstellationInfo_Level(XUI::CXUI_Wnd * pWnd, int const MinLevel, int const MaxLevel);
	void UpdateConstellationInfo_Quest(XUI::CXUI_Wnd * pWnd, int const QuestNo);
	void UpdateConstellationInfo_CashItem(XUI::CXUI_Wnd * pWnd, bool const enable);
	void UpdateConstellationInfo_Monster(XUI::CXUI_Wnd * pWnd, VEC_INT const& ContMonsterNo);
	void UpdateConstellationInfo_Story(XUI::CXUI_Wnd * pWnd,int const Difficulty,int const TextNo);

	int CheckCashItemCount(int & firstItemNo);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 던전 임무 기능 ////////////////////////////////////////////////////////////////////////////////////////////
	void AddDungeonMission(XUI::CXUI_List * pList);
	void AddNormalMission(XUI::CXUI_List * pList);
	void AddBossMission(XUI::CXUI_List * pList);
	void SetMissionState(XUI::CXUI_List * pList, int const MissionTextNo, BYTE const MissionState);

	void AllClearMission();

	void NoticeMissionState(std::wstring const& MissionText, BYTE const State); // 알림 기능
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//결과창 
	typedef std::set<size_t> CONT_REWARD_CHOSEN_SLOT_LIST;
	CONT_REWARD_CHOSEN_SLOT_LIST m_kContChosenList;

	bool GetPlayerRewardConsumItemCount(int& riOutNeedItemNo, size_t& riOutNeedItemCount, size_t& riOutHaveItemCount);
	void UpdateRewardBounsConsumeInfo(void);

	void SetState(E_STATE const eState);
	void SetProcessState(E_PROCESS_STATE const eState);	
	E_PROCESS_STATE GetProcessState()const { return m_eProcessState; }

	void SetRewardBonusMixSlot(XUI::CXUI_Wnd* pkWnd, size_t iSlotIndex, bool bVisible);
	void SetRewardBonusChosenSlot(XUI::CXUI_Wnd* pkWnd, size_t iSlotIndex);

	bool m_bConstellationFreePass;

	CONT_CONSTELLATION_BONUS_ITEM m_ContBonusItem;	// 체크할 보너스아이템 컨테이너
	CONT_CONSTELLATION_ENTRANCE_CASH_ITEM m_EntranceCashItem; // 입장조건 패스시켜주는 캐쉬아이템 컨테이너
	CONT_CONSTELLATION m_ContEnter;		// 별자리던전 XML 파싱할 컨테이너
	SConstellationMission m_Mission;	// 입장한 던전 내부에서 해결해야할 미션 컨테이너

	CONT_INDUN_PARTY_CHANNEL m_TempCont;	// 별자리던전 파티리스트 임시 저장소
	
	typedef struct tagPartyMemberReward
	{
		float AccumTime;
		std::wstring FormName;
		int ItemNo;
	}SPartyMemberReward;

	typedef std::vector<SPartyMemberReward> CONT_PARTY_MEMBER_REWARD;
	CONT_PARTY_MEMBER_REWARD m_PartyMemberReward;
private:
	E_STATE m_eState;
	E_PROCESS_STATE m_eProcessState;
	int m_NeedItemNo;
	int m_NeedQuestNo;
	int m_CashItemNo;
	int m_iNeedRewardConsumeItemNoAtUI;
	int m_iNeedRewardConsumeItemNo;
	size_t m_iNeedRewardConsumeItemCount;
	size_t m_iHaveRewardConsumeItemCount;
	size_t m_iMaxRewardBonusCount;
	CLASS_DECLARATION_S(bool, EnoughConsumeItem);
	CLASS_DECLARATION_S(size_t, RewardCurrentChoice);
	CLASS_DECLARATION_S(size_t, RewardCurrentStop);

protected:
	typedef struct tagREWARDINFO
	{
		std::wstring wstrName; //파티원 이름.
		int iRandRewardItemNo; //랜덤 지급 보상 아이템 번호
		int iRewardItemNo; //기본 지급 보상 아이템 번호
		int iAddChoiceRewardBagNo; //아이템백 번호

		tagREWARDINFO() : wstrName(), iRandRewardItemNo(0), iRewardItemNo(0), iAddChoiceRewardBagNo(0)
		{}
	}REWARDINFO;
	static const size_t MAX_PLAYER = 4;
	REWARDINFO m_kArrRewardInfo[MAX_PLAYER];
	size_t m_iRewardPlayer;
	size_t m_iMyRewardIndex;

	enum EAPPSTEP : WORD
	{
		EAS_INIT = 0,
		EAS_APPEAR_WINDOW,
		EAS_REWARD_ITEMBAG,
		EAS_ESSENTIAL_REWARD,
		EAS_FINISH
	};

	WORD m_eAppStep;

public:
	static const size_t MAX_CHOICE = 9;
};


#define g_kConstellationEnterUIMgr SINGLETON_STATIC(PgConstellationEnterUIMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_CONSTELLATION_PGCONSTELLATION_H