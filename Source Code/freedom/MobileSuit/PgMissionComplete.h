#ifndef FREEDOM_DRAGONICA_UI_PGMISSIONCOMPLETE_H
#define FREEDOM_DRAGONICA_UI_PGMISSIONCOMPLETE_H
#include "PgRewardBox.h"
#include "PgNetwork.h"
#include "XUI/XUI_Wnd.h"
#include "PgUIModel.h"
#include "variant/PgMission_Result.h"
#include "lwUI.h"

//class CXUI_Wnd;

int const PARTY_MEMBER_COUNT = 4;
int const REWARD_BOX_COUNT = PARTY_MEMBER_COUNT;	//화면에 나타날 상자 4개
int const EXPEDITION_REWARD_BOX_COUNT = 6;			//원정대는 6개

typedef struct tagMissionRewardInfo
{
	BM::GUID kCharGuid;

	int iNormalExp;
	int iStrangthExp;
	int iCrackExp;

	int iGetExp;
	int iTotalExp;

	int iLevel;
	std::wstring wstrName;

	int	iItemCount;
	CONT_ITEM_CREATE_ORDER kResultItemList;
	float fParticipationRate;

	void ReadFromPacket(BM::Stream &kPacket)
	{
		kPacket.Pop(kCharGuid);	// 멤버GUID는 씹어버리자(나중에 멤버GUID없애야지...*YonMy*
		kPacket.Pop(kCharGuid);
		kPacket.Pop(iNormalExp);
		kPacket.Pop(iStrangthExp);
		kPacket.Pop(iCrackExp);

		kPacket.Pop(iGetExp);
		kPacket.Pop(iTotalExp);

		kPacket.Pop(iLevel);
		kPacket.Pop(wstrName);

		kPacket.Pop(iItemCount);

		if (iItemCount > RWD_ITEM_CNT)
		{
			iItemCount = RWD_ITEM_CNT;
		}

		kResultItemList.clear();
		PU::TLoadArray_M(kPacket, kResultItemList );
		kPacket.Pop(fParticipationRate);
	}

	tagMissionRewardInfo()	
	{
		Init();
	}

	void Init()
	{
		kCharGuid = BM::GUID::NullData();

		iNormalExp = 0;
		iStrangthExp = 0;
		iCrackExp = 0;

		iGetExp = 0;
		iTotalExp = 0;

		iLevel = 0;
		wstrName.clear();
		iItemCount = 0;
		kResultItemList.clear();
		fParticipationRate = 0;
	}
} SMissionRewardInfo;

typedef struct tagIncreaseNumber
{
	tagIncreaseNumber()
	{
		m_fStartTime = 0.0f;
		m_fMainTime = 0.0f;
		m_fOriginNum = 0;
		m_fDisplayNum = 0;
		m_fDelayTime = 0;
		m_bIncrease = true;
		m_wstrName.clear();
	}

	bool Update(float const fAccumTime)
	{
		float const fDeltaTime = __max(0, fAccumTime - m_fStartTime - m_fDelayTime);
		
		float const fRate = fDeltaTime/m_fMainTime;
		if (m_bIncrease)
		{
			if (fDeltaTime >= m_fMainTime)
			{
				m_fDisplayNum = m_fOriginNum;
				return true;
			}

			m_fDisplayNum = fRate*m_fOriginNum;
			return false;
		}
		else
		{
			if (fDeltaTime >= m_fMainTime)
			{
				m_fDisplayNum = 0;
				return true;
			}

			m_fDisplayNum = m_fOriginNum - fRate*m_fOriginNum;
		}

		return true;
	}

	bool Set(std::wstring &strName, float fOrigin, float fStartTime, float fMainTime = 1.5f, bool bInc = true)
	{
		if (strName.empty())
		{
			return false;
		}
		m_wstrName = strName;
		m_fOriginNum = fOrigin;
		m_fStartTime = fStartTime;
		m_fMainTime = fMainTime;
		m_bIncrease = bInc;

		return true;
	}

	void operator = (tagIncreaseNumber const& Src)
	{
		m_fStartTime = Src.m_fStartTime;
		m_fMainTime = Src.m_fMainTime;
		m_fOriginNum = Src.m_fOriginNum;
		m_fDisplayNum = Src.m_fDisplayNum;
		m_fDelayTime = Src.m_fDelayTime;
		m_bIncrease = Src.m_bIncrease;
		m_wstrName = Src.m_wstrName;
	}

	float	m_fStartTime;
	float	m_fMainTime;
	float	m_fOriginNum;
	float	m_fDisplayNum;
	bool	m_bIncrease;
	std::wstring m_wstrName;
	float	m_fDelayTime;

}SIncreaseNumber;

typedef std::map<std::wstring const, SIncreaseNumber> IncreaseNumberMap;

class	PgMobileSuit;
class PgMissionComplete
{
	friend	class	PgMobileSuit;

public:
	PgMissionComplete();
	virtual ~PgMissionComplete();

	virtual bool Update(float const fAccumTime, float const fFrameTime);
	virtual void Draw(PgRenderer *pkRenderer, float fFrameTime);
	void Start();
	void ReadFromPacket(BM::Stream &kPacket);
	void ReadFromPacketSelect(BM::Stream &kPacket);	//박스 선택 정보
	void ReadFromPacketOpen(BM::Stream &kPacket);		//자 이제 박스를 열자
	void Pickup(int const iValue = 0);
	void CleanUp();
	void SendMemo(char const* pkStr);
	void SendReqRankInfo();
	void CallRankUI();
	int CalcMinLevelLimit(int const iMissionNo, int const iLev);
	int CalcMaxLevelLimit(int const iMissionNo, int const iLev);
	bool IsOwner() const;
	XUI::CXUI_Wnd* CallRetryUI();
	void CallRankResultItem(int const iItemNo, int const iNewRank);
	void StopResult();
	void StopMissionResult();
	PgMission_ResultC const & GetMission_ResultC() { return m_kMission_Result; }
	void CallSelectDirectionUI();
	void MissionEndUnlockInput();
	int GetUICloseTime2()const;

	typedef enum eMissionCplState
	{
		E_MSN_CPL_NONE = 0,
		E_MSN_CPL_WAIT,			//3초 기다리자
		E_MSN_CPL_FADE,			//화면 어둡게
		E_MSN_CPL_BG,				//배경이 움직이고
		E_MSN_CPL_DROP,			//상자가 움직이고
		E_MSN_CPL_SELECT,		//상자 선택을 기다리자
		E_MSN_CPL_MOVE_START,	// 선택한 박스가 이동 시작
		E_MSN_CPL_MOVE,			// 선택한 박스가 이동하는 시간
		E_MSN_CPL_OPEN,			//상자 열리는 애니동안 기다리고
		E_MSN_CPL_LIGHT,			//화면을 하얗게
		E_MSN_CPL_REWARD,		//UI를 움직이고
		E_MSN_CPL_REWARD_DISPLAY,	// 화면에 표시
		E_MSN_CPL_REWARD_WAIT,	//캐릭터 승리포즈 
		E_MSN_CPL_UI_CLOSE,
		E_MSN_CPL_WAIT_RETRY,	//다시하기 대기
		E_MSN_CPL_SELECT_NEXT,	//다시할 미션 고르기
		E_MSN_CPL_END,			//끝
	}E_MSN_CPL_STATE;

	typedef enum eMissionUIDisplayState
	{
		E_MSN_UI_NONE = 0,
		E_MSN_UI_MISSION, 
		E_MSN_UI_SCORE0,
		E_MSN_UI_SCORE1,
		E_MSN_UI_SCORE2,
		E_MSN_UI_USER1,
		E_MSN_UI_USER2,
		E_MSN_UI_USER3,
		E_MSN_UI_USER4,
		E_MSN_UI_ANIBAR_TOTAL,
		E_MSN_UI_ANIBAR_SENSE,
		E_MSN_UI_ANIBAR_ABILITY,
		E_MSN_UI_TIME,
		E_MSN_UI_NUM,
		E_MSN_UI_EXP,
		E_MSN_UI_EXP_PLUS,
		E_MSN_UI_EXP_SUM,
		E_MSN_UI_BONUSEXP,
		E_MSN_UI_RANK,
		E_MSN_UI_NEXT_ITEM,
		E_MSN_UI_OTHER,
		E_MSN_UI_ITEM,
		E_MSN_UI_END,
	}E_MSN_UI_STATE;

	typedef enum eTimeStamp
	{
		E_TS_NONE	= 0,
		E_TS_CONSTELLATION_RESULT_TIME = 1, //별자리 던전 결산창
	}E_TIMESTAMP;

	typedef std::map<E_TIMESTAMP,float> CONT_TIMESTAMP;

protected:

	void Init();
	void Destroy();

private:

	NiAVObjectPtr	m_spBackGround;
	NiPoint3		m_kBGPos;
	XUI::CXUI_Wnd	*m_pkWnd;
	XUI::CXUI_Wnd	*m_pkWndModel;
	PgUIModel		*m_pkWndUIModel;
	PgUIModel		*m_pkWndUIModel_NumEff;

	typedef std::vector<NiPoint3>	BoxPosVec;
	BoxPosVec		m_vecBoxPos;

	typedef std::vector<Pg2DString>	BoxNameVec;
	BoxNameVec		m_vecBoxName;

	NiAVObjectPtr	m_spWhiteBillboard;
	NiMaterialPropertyPtr	m_spAlphaMaterial;
	NiMaterialPropertyPtr	m_spBGAlphaMaterial;

	NiAlphaPropertyPtr m_spNewAlphaProp;
	
	CLASS_DECLARATION_S(int, SelectBoxNum);				//몇번째 박스를 선택했는지
	CLASS_DECLARATION_S(int, ItemCount);				//아이템 몇개나 줄지
	CLASS_DECLARATION_S(E_MSN_CPL_STATE, CompleteState);
	CLASS_DECLARATION_S(E_MSN_UI_STATE, CompleteUIState);
	CLASS_DECLARATION_S(NiPoint3, CamWorldPos);			//카메라 좌표
	CLASS_DECLARATION_S(float, TimeStamp);				//매 스테이트가 바뀔 때마다 시간을 저장
	CLASS_DECLARATION_S(float, UITimeStamp);				//매 스테이트가 바뀔 때마다 시간을 저장
	CLASS_DECLARATION_S(NiQuaternion, PastCamSpeed);	//카메라 멈추기 전에 이전 속도를 저장.
	CLASS_DECLARATION_S(size_t, MemberCount);				//몇명?
	CLASS_DECLARATION_S(int, MyNumber);					//내 정보가 몇번째 걸까?
	CLASS_DECLARATION_S(int, NowViewItem);				//몇번째 내 아이템 공개할지
	CLASS_DECLARATION_S(bool, BossMission);				//이번 판이 보스 판인지 아닌지. 기본적으로 false
	CLASS_DECLARATION_S(bool, IsRevcRank);				//랭크 결과 받았는지.
	CLASS_DECLARATION_S(int, AniBar_Value);
	CLASS_DECLARATION_S(int, RankItemNo);
	CLASS_DECLARATION_S(int, NewRank);
	CLASS_DECLARATION_S(bool, SpaceStop);
	CLASS_DECLARATION_S(bool, MissionEnd);				// 미션 결산까지 마무리 되었는지

	SMissionRewardInfo	m_kReward[REWARD_BOX_COUNT];
	PgRewardBox			m_kRewardBox[REWARD_BOX_COUNT];

	NiCameraPtr			m_spCamera;

	PgMission_ResultC	m_kMission_Result;

	IncreaseNumberMap	m_kIncreaseNumberList;

	VEC_GUID			m_kContUserResultItemCheckList;

	CONT_TIMESTAMP		m_kContTimeStamp;		//타임스템프

	bool UpdateBox(float fAccumTime, float fFrameTime);	//박스 업데이트
	void DrawBox(PgRenderer *pkRenderer, float fFrameTime);//박스 렌더링

	bool SetRoluetteStopRewardItemDraw(XUI::CXUI_Wnd* pkParentWnd, BM::GUID const &rkCharGuid);
	void SetRoluetteStopRewardItemDraw(BM::GUID const &rkGuid);
	bool PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit = false);		//내정보 UI에 정보 넣기
	bool PutOtherInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit = false);	//다른사람 정보UI에 정보 넣기
	bool PutRankInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit = false);
	void UpdateMyInfo(XUI::CXUI_Wnd* pkParentWnd, float const fAccumTime);
	bool AddNewNum(std::wstring &strName, float fOrigin,  float fDelayTime = 0, float fMainTime = 1.5f, bool bInc = true);
	void DisplayNum(std::wstring const &kstrName, XUI::CXUI_Wnd *pkWnd);
	float GetIncleaseNumber(std::wstring const &strName);
	void SetAlphaByRecursiveProcess(NiAVObject* pkObject, float const fAlpha);

	bool PlaySound(char const *szPath);

	bool PutTimerText(XUI::CXUI_Wnd* pkWnd, int const iValue);

	bool PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, std::wstring const &rkName, std::wstring const &rkText);
	bool PutRankBonusInfoUI(int idx, XUI::CXUI_Wnd* pkParentWnd, DWORD const dwColor, std::wstring const& ImgPath, std::wstring const& Value, bool const IsVisible);
	void CloseMissionTimer(BM::Stream &kPacket);
	void CloseBonusStageMissionTimer(BM::Stream &kPacket);
	void CloseMissionScore(BM::Stream &kPacket);
	bool MissionPartyScoreUI(XUI::CXUI_Wnd* pkParentWnd, int const iNumber);
	void GetSenseAbilityTotalScore(int &iSense, int &iAbility);
	bool PutMyInfoToUINext(XUI::CXUI_Wnd* pkParentWnd, bool bInit);
	bool MissionTotalUI(XUI::CXUI_Wnd* pkParentWnd, E_MSN_UI_STATE eType);
	void OnSelectGadaCoinBox();
	int GetMissionType();
	void GadaCoinItemView(BM::Stream &kPacket);
	void AddAniBar_Value();
};
#define g_kMissionComplete SINGLETON_STATIC(PgMissionComplete)

class PgBossComplete
{
	friend	class	PgMobileSuit;

public:
	PgBossComplete();
	virtual ~PgBossComplete();

	virtual bool Update(float const fAccumTime, float const fFrameTime);
	virtual void Draw(PgRenderer *pkRenderer, float fFrameTime);
	void Start();
	void ReadFromPacket(BM::Stream &kPacket);
	void ReadFromPacketSelect(BM::Stream &kPacket);	//박스 선택 정보
	void ReadFromPacketOpen(BM::Stream &kPacket);		//자 이제 박스를 열자
	void Pickup(int const iValue = 0);
	void CleanUp();

	typedef enum eMissionCplState
	{
		E_MSN_CPL_NONE = 0,
		E_MSN_CPL_WAIT,			//3초 기다리자
		E_MSN_CPL_FADE,			//화면 어둡게
		E_MSN_CPL_BG,				//배경이 움직이고
		E_MSN_CPL_DROP,			//상자가 움직이고
		E_MSN_CPL_SELECT,		//상자 선택을 기다리자
		E_MSN_CPL_MOVE_START,	// 선택한 박스가 이동 시작
		E_MSN_CPL_MOVE,			// 선택한 박스가 이동하는 시간
		E_MSN_CPL_OPEN,			//상자 열리는 애니동안 기다리고
		E_MSN_CPL_LIGHT,			//화면을 하얗게
		E_MSN_CPL_REWARD,		//UI를 움직이고
		E_MSN_CPL_REWARD_DISPLAY,	// 화면에 표시
		E_MSN_CPL_REWARD_WAIT,	//캐릭터 승리포즈 
		E_MSN_CPL_END,			//끝
	}E_MSN_CPL_STATE;

protected:

	void Init();
	void Destroy();

private:

	NiAVObjectPtr	m_spBackGround;
	NiPoint3		m_kBGPos;
	XUI::CXUI_Wnd	*m_pkWnd;
	XUI::CXUI_Wnd	*m_pkWndModel;
	PgUIModel		*m_pkWndUIModel;

	typedef std::vector<NiPoint3>	BoxPosVec;
	BoxPosVec		m_vecBoxPos;

	typedef std::vector<Pg2DString>	BoxNameVec;
	BoxNameVec		m_vecBoxName;

	NiAVObjectPtr	m_spWhiteBillboard;
	NiMaterialPropertyPtr	m_spAlphaMaterial;
	NiMaterialPropertyPtr	m_spBGAlphaMaterial;

	NiAlphaPropertyPtr m_spNewAlphaProp;

	CLASS_DECLARATION_S(int, SelectBoxNum);				//몇번째 박스를 선택했는지
	CLASS_DECLARATION_S(int, ItemCount);				//아이템 몇개나 줄지
	CLASS_DECLARATION_S(E_MSN_CPL_STATE, CompleteState);
	CLASS_DECLARATION_S(NiPoint3, CamWorldPos);			//카메라 좌표
	CLASS_DECLARATION_S(float, TimeStamp);				//매 스테이트가 바뀔 때마다 시간을 저장
	CLASS_DECLARATION_S(NiQuaternion, PastCamSpeed);	//카메라 멈추기 전에 이전 속도를 저장.
	CLASS_DECLARATION_S(size_t, MemberCount);				//몇명?
	CLASS_DECLARATION_S(int, MyNumber);					//내 정보가 몇번째 걸까?
	CLASS_DECLARATION_S(int, NowViewItem);				//몇번째 내 아이템 공개할지
	CLASS_DECLARATION_S(bool, BossMission);				//이번 판이 보스 판인지 아닌지. 기본적으로 false

	SMissionRewardInfo	m_kReward[REWARD_BOX_COUNT];
	PgRewardBox			m_kRewardBox[REWARD_BOX_COUNT];

	NiCameraPtr			m_spCamera;

	bool UpdateBox(float fAccumTime, float fFrameTime);	//박스 업데이트
	void DrawBox(PgRenderer *pkRenderer, float			//박스 렌더링
		fFrameTime);
	bool PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit = false);		//내정보 UI에 정보 넣기
	bool PutOtherInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit = false);	//다른사람 정보UI에 정보 넣기
	void SetAlphaByRecursiveProcess(NiAVObject* pkObject, float const fAlpha);

	bool PlaySound(char const *szPath);	

	void ResizeBGByResolution();
};
#define g_kBossComplete SINGLETON_STATIC(PgBossComplete)


namespace PgExpeditionCompleteUtil
{
	void UpdateRewardInfo(lwUIWnd lwWnd);		// 기본 보상 정보 갱신
	int GetTenderChance();						// 입찰 가능 횟수 가져오기
	int GetCompleteState();						// 결과창 상태 가져오기
	void StampEffectUI(lwUIWnd lwWnd, float const fFlowTime, float const fDuring); // UI 진동
	void Send_PT_C_N_REQ_TENDER_ITEM();			// 입찰 요청
	void Send_PT_C_N_REQ_GIVEUP_TENDER();		// 포기 요청
}

typedef struct tagExpeditionRewardInfo
{
	BM::GUID CharGuid;
	int	ItemCount;
	CONT_ITEM_CREATE_ORDER ResultItemList;

	void ReadFromPacket(BM::Stream &Packet)
	{
		Packet.Pop(CharGuid);
		Packet.Pop(ItemCount);

		if( ItemCount > EXPEDITION_REWARD_BOX_COUNT )
		{
			ItemCount = EXPEDITION_REWARD_BOX_COUNT;
		}

		ResultItemList.clear();
		PU::TLoadArray_M( Packet, ResultItemList );
	}

	tagExpeditionRewardInfo()
	{
		Clear();
	}

	void Clear()
	{
		CharGuid = BM::GUID::NullData();

		ItemCount = 0;
		ResultItemList.clear();
	}
} SExpeditionRewardInfo;

typedef struct tagExpeditionTenderInfo
{
	int	ItemCount;
	CONT_SPECIALITEM ResultItemList;

	void ReadFromPacket(BM::Stream &Packet)
	{
		Packet.Pop(ItemCount);
		ResultItemList.clear();
		PU::TLoadArray_M( Packet, ResultItemList );
	}

	tagExpeditionTenderInfo()
	{
		Clear();
	}

	void Clear()
	{
		ItemCount = 0;
		ResultItemList.clear();
	}
} SExpeditionTenderInfo;

typedef struct tagExpeditionBestInfo
{
	int BestTeamNum;
	size_t DP_BestTeam;
	BM::GUID BestPlayerGuid;
	size_t DP_BestPlayer;
	BM::GUID BestSupporterGuid;
	size_t HP_BestSupporter;

	tagExpeditionBestInfo()
	{
		Clear();
	}

	void Clear()
	{
		BestTeamNum = 0;
		DP_BestTeam = 0;
		BestPlayerGuid.Clear();
		DP_BestPlayer = 0;
		BestSupporterGuid.Clear();
		HP_BestSupporter = 0;
	}

	void ReadFromPacket(BM::Stream& Packet)
	{
		Packet.Pop(BestTeamNum);
		Packet.Pop(DP_BestTeam);
		Packet.Pop(BestPlayerGuid);
		Packet.Pop(DP_BestPlayer);
		Packet.Pop(BestSupporterGuid);
		Packet.Pop(HP_BestSupporter);
	}
} SExpeditionBestInfo;

typedef std::vector< SExpeditionRewardInfo > ContRewardItem;

class PgExpeditionComplete
{
	friend	class	PgMobileSuit;
public:
	PgExpeditionComplete();
	virtual ~PgExpeditionComplete();

	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);
	static bool RegisterWrapper(lua_State *pkState);

	virtual bool Update(float const fAccumTime, float const fFrameTime);
	virtual void Draw(PgRenderer *pRenderer, float fFrameTime);
	void Start();
	void CleanUp();
	
	XUI::CXUI_Wnd * GetParentWnd() const { return m_pWnd; }

	void SetExitButtonDown(bool const bValue);

	bool IsHaveTenderChance();

	typedef enum eMissionCplState
	{
		E_MSN_CPL_NONE = 0,
		E_MSN_CPL_WAIT,					//준비
		E_MSN_CPL_START,				//시작
		E_MSN_CPL_WAIT_2,				//준비2
		E_MSN_CPL_MOVE_RESULT,			//RESULT 이미지 이동
		E_MSN_CPL_BEST_MOVE,			//BESTPLAY 윈도우 이동
		E_MSN_CPL_REWARD_MOVE,			//REWARD 윈도우 이동
		E_MSN_CPL_BESTPLAY_DISPLAY,		//BESTPLAY 연출
		E_MSN_CPL_TENDER_MOVE,			//TENDER 윈도우 이동
		E_MSN_CPL_TENDER_WAIT,			//특별보상 준비
		E_MSN_CPL_TENDER_ITEM_DISPLAY,	//특별보상 아이템 연출
		E_MSN_CPL_TENDER_START,			//특별보상 아이템 보여지는 시간
		E_MSN_CPL_TENDER_INIT,			//특별보상 입찰 전 변수 초기화
		E_MSN_CPL_TENDER_SELECT,		//특별보상 입찰 선택 시간
		E_MSN_CPL_END_WAIT,				//마지막 대기시간
		E_MSN_CPL_END_MOVE,				//창 원위치로 이동
		E_MSN_CPL_END,					//끝
	}E_MSN_CPL_STATE;

	E_MSN_CPL_STATE GetCompleteState() { return m_kCompleteState; }
protected:
	void Init();
	void Destroy();
private:
	NiAVObjectPtr	m_spBackGround;
	NiAVObjectPtr	m_spWhiteBillboard;
	NiAlphaPropertyPtr m_spNewAlphaProp;
	NiMaterialPropertyPtr	m_spAlphaMaterial;
	XUI::CXUI_Wnd	*m_pWnd;
	
	NiCameraPtr		m_spCamera;

	CLASS_DECLARATION_S(NiPoint3, CamWorldPos);			//카메라 좌표
	CLASS_DECLARATION_S(E_MSN_CPL_STATE, CompleteState);//결과창 상태
	CLASS_DECLARATION_S(float, TimeStamp);				//매 스테이트가 바뀔 때마다 시간을 저장
	CLASS_DECLARATION_S(NiQuaternion, PastCamSpeed);	//카메라 멈추기 전에 이전 속도를 저장.
	CLASS_DECLARATION_S(size_t, MemberCount);			//몇명?

	CLASS_DECLARATION_S(int, CurrTender);				//현재 입찰 진행 중인 리스트 번호
	CLASS_DECLARATION_S(float, TenderTime);				//시간경과를 알기 위한 변수
	CLASS_DECLARATION_S(int, TenderLimitTime);			//입찰 제한시간
	CLASS_DECLARATION_S(float, NameShuffleTime);		//당첨자 연출을 위한 시간체크 변수
	CLASS_DECLARATION_S(float, ItemShuffleTime);		//아이템 연출을 위한 시간체크 변수

	CLASS_DECLARATION_S(BM::GUID, WinnerGuid);			//당첨자 GUID
	CLASS_DECLARATION_S(std::wstring, WinnerName);		//당첨자 이름

	CLASS_DECLARATION_S(int, TenderChance);				//입찰기회
	
	CLASS_DECLARATION_S(bool, ResultSound);				//효과음 타이밍
	CLASS_DECLARATION_S(bool, BestPlayerSound);

	SExpeditionBestInfo			m_BestInfo;				//베스트플레이 정보
	ContRewardItem				m_RewardItemList;		//기본보상 아이템 리스트
	SExpeditionTenderInfo		m_TenderItemList;		//특별보상 아이템 리스트

	bool bExitButtonDown;								//나가기 버튼을 눌렀을 때 활성화
	int NextNameCount;									//이름 연출을 위한 카운트 변수

	void PutBestPlayInfoToUI(XUI::CXUI_Wnd* pParentWnd);	//베스트플레이 정보 넣기
	void PutRewardInfoToUI(XUI::CXUI_Wnd* pParentWnd);		//아이템보상 정보 넣기
	void PutTenderInfoToUI(XUI::CXUI_Wnd* pParentWnd);		//특별보상 아이템 정보 넣기
	void UpdateTenderInfoToUI(XUI::CXUI_Wnd* pParentWnd);	//특별보상 아이템 정보 갱신
	void TenderPassButtonState(XUI::CXUI_Wnd* pParentWnd, bool const bDisable);	//특별보상 입찰 포기 버튼 활성화
	void UpdateTenderLimitTime(XUI::CXUI_Wnd* pParentWnd);	//특별보상 제한시간 갱신
	void UpdateTenderWinnerName(XUI::CXUI_Wnd* pParentWnd,	//원정대원의 이름들 중 랜덤으로 하나 뽑아서 표시
		std::wstring const * const pWinnerName = NULL);		//당첨자 이름이 있다면 그 이름을 표시해줌

	void NotifyEarnItem() const;			// 당첨자 팡파레 연출
	bool PlaySound(char const *szPath);

	void SetAlphaByRecursiveProcess(NiAVObject* pObject, float const fAlpha);
	void ResizeBGByResolution();

	void Recv_PT_N_C_NFY_EXPEDITION_BOSS_RESULT(BM::Stream& Packet);	// 결산창 스타트 알림
	void Recv_PT_N_C_NFY_START_TENDER(BM::Stream& Packet);				// 특별보상 입찰 시작 알림
	void Recv_PT_N_C_NFY_ACCEPTANCE_USER(BM::Stream& Packet);			// 특별보상 당첨자 알림
	void Recv_PT_M_C_NFY_EXPEDITION_RESULT_END(BM::Stream& Packet);	// 결산창 종료 알림
};

#define g_kExpeditionComplete SINGLETON_STATIC(PgExpeditionComplete)

#endif // FREEDOM_DRAGONICA_UI_PGMISSIONCOMPLETE_H