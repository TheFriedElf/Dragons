#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLWORKMACHINE_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLWORKMACHINE_H

class lwWString;
class PgJobSkillWorkBenchMgr;
class PgBase_Item;
typedef T_PT2<BYTE> SItemPos;
class PgMyHome;
class lwGUID;

struct SHelpSkillInfo
{
	SHelpSkillInfo(int const iToolType_in, int const iJobSkillNo_in)
		:iToolType(iToolType_in)
		,iJobSkillNo(iJobSkillNo_in)
	{}
	int iToolType;
	int iJobSkillNo;
};
typedef std::vector<SHelpSkillInfo> CONT_HELP_SKILLINFO;

class PgJobSkillWorkMachine
{
public:
	struct STimeInfo
	{
	public:
		STimeInfo(__int64 i64RemainTime_in, __int64 i64RecvedTime_in)
			:m_i64RemainTime(i64RemainTime_in)
			,m_i64RecvedTime(i64RecvedTime_in)
			,m_bSeted(false)
			,m_i64_Text_RemainTime(0)
			,m_i64_Progress_TurnTime(0)
		{};

		STimeInfo()
			:m_i64RemainTime(0)
			,m_i64RecvedTime(0)
			,m_bSeted(false)
			,m_i64_Text_RemainTime(0)
			,m_i64_Progress_TurnTime(0)
		{};

		void Init(__int64 i64RemainTime, __int64 i64RecvedTime)
		{
			m_i64RemainTime = i64RemainTime;
			m_i64RecvedTime = i64RecvedTime;
			m_i64_Text_RemainTime = 0;
			m_i64_Progress_TurnTime = 0;
			m_bSeted = false;
		}

		__int64 GetElapsedTime() const;
		__int64 GetCalcRemainTotalSec(PgJobSkillWorkBench const& rkWorkBench, int const iSlot, int const iRate=0) const;
		bool Test(PgJobSkillWorkBench const& rkWorkBench, float const fRemainTimeReduceValue, __int64& iOutTotalSec, __int64& iOutTurnTime );
		
		bool IsSetted() const { return m_bSeted; }
		__int64 GetRemainTime() const { return m_i64_Text_RemainTime; }
		__int64 GetProgressTurnTime() const { return m_i64_Progress_TurnTime; }
		
	public:
		__int64 m_i64RemainTime;
		__int64 m_i64RecvedTime;
			
		__int64 m_i64_Text_RemainTime;
		__int64 m_i64_Progress_TurnTime;
		bool m_bSeted;
	};

	typedef std::map<int, BM::GUID> CONT_WORKBENCH_GUID_ON_UI;								// 미니UI build index(int)와, 물려있는 작업대(GUID)
	typedef std::map<size_t, STimeInfo> CONT_WORKBENCH_PROCESS_TIME;						// 
	typedef std::map<BM::GUID, CONT_WORKBENCH_PROCESS_TIME> CONT_WORKBENCH_UPGRADE_TIME;	// 작업대(GUID), 서버에서 보내준 남은 작업시간을 받은때의 클라의 시간(DWORD)
	typedef std::map<int, std::wstring> CONT_LOG_TEXT;										// 한줄 한줄 저장된 로그 텍스트
	
public:
	PgJobSkillWorkMachine();
	~PgJobSkillWorkMachine();
	
	void Clear();

	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket);	// 패킷 처리
	void SetCurWorkBenchGUID(BM::GUID const& rkWorkBenchGUID);								// 현재 작업대 GUID 기억
	BM::GUID const& GetCurWorkBenchGUID() const { return m_kCurWorkBenchGUID; }				

	void InitUI(PgMyHome* const pkHomeUnit, PgFurniture* const pkWorkBenchFurniture);		// UI 초기화 및 가구 메쉬 형태 설정
	size_t GetCurUpgradeSlot() const;														// 현재 선택한 작업대의 가공되고있는 슬롯 번호(0이면 없는것)
	void ReqInsertToWorkBench(SItemPos const& rkItemPos);									// 현재 선택한 작업대에 아이템 넣기
	void ReqRepair() const;																	// 현재 선택한 작업대 수리 요청
	void ReqTakeItem(int iSlot, bool const bMouseRClick) const;								// 현재 선택한 작업대 아이템 꺼내기
	bool CallWorkBenchUI(int const iMiniUIBldIdx);											// 현재 선택한 작업대의 UI 호출	
	void CallSlotItemToolTip(int iSlot);													// 현재 선택한 작업대의 iSlot의 툴팁 보여주기
	void UpdateCurWorkBenchRemainTime();													// 현재 선택한 작업대의 시간 정보 UI 업데이트
	void ShowLogByToolTip(int const iIdx);													// 현재 선택한 작업대의 Log의 Idx 내용을 툴팁으로 보여줌
	void DisplayRemainDestroyTime(int iSlot);												// 현재 선택한 작업대의 완료된 슬롯의 아이템이 파괴되기까지 걸리는 시간을 보여줌

	bool IsAbleUseHelp() const;																// 도움주기가 가능한가? 확인
	void ShowHelpSkillUI() const;															// 도움 주기 UI 호출
	void CallMiniUI();																		// 미니UI(퀵UI) 호출
	bool IsWorkBenchOwner(BM::GUID const& kGUID) const;										// kGUID가 작업대의 소유자인가? 확인
	
	static float GetRemainTimeReduceValue();													// 도움(축복) 받을때 
	
	bool IsPublicAlterSeted() const;

private:
	void UpdateAllFurnitureMeshState();																								// 모든 작업대 메쉬 갱신
	void UpdateFurnitureMeshState(BM::GUID const& rkFurnitureGUID, PgJobSkillWorkBench const& rkWorkBench);							// 특정 작업대 메쉬 갱신
	void UpdateUI();																												// 모든 작업대 관련 UI 갱신
		
	void UpdateLog(BM::GUID const& rkWorkBenchGUID);																				// 작업대 로그 갱신
	void AddMachineLog(int const iIdx, int const iMonth, int const iDay, int const iHour, int const iMin, BM::vstring const kLog);	// 작업대 로그 추가
		
	void ShowErrMsg(int const iResult) const;																						// 에러 메세지 처리
	void SetFurnitureMeshState(PgFurniture* pkWorkBenchFuniture, int const iState, bool const bTrouble,								// 작업대 메쉬 설정
		SJobSkillItemUpgrade const* pkItemUpgradeInfo=NULL, bool const bWorking = false) const;		
	
	void UpdateTimeUI(PgJobSkillWorkBench const& rkWorkBench);																	// 시간표시 관련 UI 갱신
	bool InitInfoRecvTime(BM::GUID const& rkWorkBenchGUID, SJobSkillWorkBenchSlotItemStatus const& kSlotItemStatus,				// 서버에서 작업대 가공시간 정보를 받아 저장해야할때
		int const iSlot, __int64 const i64CurTime, bool const bEnforcement);
	bool OverrideInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot, STimeInfo const& rkTimeInfo);					// 이미 받은 작업대 가공시간 정보를 덮어써야 할때
	bool GetInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot,  STimeInfo& rkOut) const;							// 서버에서 받은 가공시간 정보 얻어오기
	bool ClearWorkBenchInfoRecvTime(BM::GUID const& rkWorkBenchGUID);															// 서버에서 받은 가공시간 정보 삭제
	bool ClearWorkBenchSlotInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot);										// 서버에서 받은 iSlot의 가공시간 정보 삭제

	void GetCalcSecToTime(__int64 i64TotalSec, int& riHour, int& riMin, int& riSec);											// 초 -> 시, 분, 초로 변환

private:
	PgJobSkillWorkBenchMgr m_kWorkBenchMgr;
	BM::GUID	m_kCurWorkBenchGUID;
	CONT_JS_WORKBENCH_EVENT m_kContWorkBenchEvent;
	CONT_WORKBENCH_GUID_ON_UI m_kContWorkBenchOnUI;
	CONT_WORKBENCH_UPGRADE_TIME m_kContWorkBenchUpgradeTime;
	CONT_LOG_TEXT m_kContLogText;
};

#define g_kJobSkillWorkMachine SINGLETON_STATIC(PgJobSkillWorkMachine)

namespace PgJobSkillWorkMachineWorkUtil
{
};

namespace lwJobSkillWorkMachine
{
	void RegisterWrapper(lua_State *pkState);

	void lwSetMachineDurability(int const iCur, int const iMax);							// 작업대 내구도 변화 받아 적용하는 부분
	void lwSetUpgradeInfo(int iRemainUpgradeCnt, int iHour, int iMin, int iSec);			// 작업대 UI에 남은 성장 횟수, 남은 가공시간 적용 하는 부분
	void lwClearLogList();																	// 작업대 로그 삭제
	
	void lwSetMachineImg(int iGatherType);													// 작업대 이미지 설정
	void lwSetMachineName(int iGatherType);													// 작업대 종류 이름 설정
	void lwPlayMachineSound(int iGatherType);												// 작업대 UI열때 사운드 플레이
	void lwSetOwnerName(lwWString kOwnerName);												// 작업대 소유자 이름 설정
	void lwSetProgressBarSize(int const iNow, int const iMax);								// 작업대 프로그래스바 설정
		
	void lwShowUpgradeCompleteUI(int const iSlot, bool const bShow);						// 완료 표시, 제거
	void lwSetRemainTimeToDestroy(int const iSlot, int const iHour, int const iMin, int const iSec);	// iSlot에 파괴되기 까지 남은 시간 표시
	void lwCallMiniUI();																	// 미니UI(퀵UI) 호출
	void lwReqJobSkillWorkBenchInfo();														// 현재 들어와있는 마이홈에 모든 작업대 정보를 요청

	void SetSlotUI(int const iItemNo, int const iSlot, bool const bClose, bool const bStrokeAni, bool const bComplete);	// iSlot에 해당하는 UI 정보를 세팅함
	int GetUIMaxSlotCnt();																	// UI에서 최대로 표현할수 있는 슬롯
	
	void lwShowHelpSkillUI();																// 도움(축복) 스킬 UI를 호출
	void lwReqdHelpJobSkill(int const iHelpJobSkillNo);										// 도움(축복) 스킬 사용 요청
	void lwReqRepair();																		// 수리 요청
	void lwReqTakeItem(int const iSlot, bool const bNoConfirmMsg);							// 아이템 꺼내기 요청
	
	bool lwCallWorkBenchUI(int const iMiniUIBldIdx);										// 작업대 UI 호출
	void lwShowRepairHelpIcon(bool const bTrouble, bool const bHelped);						// 작업대 UI에 고장, 도움(축복) 아이콘 표시
	void lwCallToolTipText(int const iSlot);												// 작업대 iSlot에 툴팁 호출
	
	void lwUpdateRemainTime();																// 가공 완료 남은 시간 갱신
	void lwShowLog(int const iIdx);															// 현재 선택된 작업대 idx에 내용을 툴팁으로 호출

	bool lwIsMyPlayerWorkBenchOwner();														// 내 플레이어가 작업대의 소유주 인가?
	
	void lwHelpIconToolTip(int iMiniUIBldIdx);												// 미니UI(퀵UI)에 떠있는 도움(축복) 아이콘에 대한 툴팁
	void lwDisplayRemainDestroyTime(int iSlotNo);											// iSlot에 아이템이 파괴되기까지 남은 시간 표시(완료된 아이템이어야함)
	lwGUID lwGetCurrentWorkBenchGUID();														// 현재 선택된 작업대의 GUID
	void lwHelpSkillIconToolTip(int iSkillNo);												// 직업스킬 iSkillNo에 대한 툴팁

	void TestJobMachine_MeshChange(PgFurniture* pkWorkBenchFuniture, int const iTestState);	// 작업대Mesh 상태 테스트를 위해 만든 변수함수
};

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLWORKMACHINE_H