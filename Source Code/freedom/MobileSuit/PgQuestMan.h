#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUESTMAN_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUESTMAN_H
#include "CreateUsingNiNew.inl"
#include "PgIXmlObject.h"
#include "variant/PgQuestInfo.h"
#include "PgQuest.h"
#include "PgMissionCard.h"
#include "PgMobilesuit.h"

#include "PgQuestFullScreenDialog.h"
#include "PgWarningDialog.h"
#include "PgLetterDialog.h"

#include "PgRealmQuestEvent.h"

class XUI::CXUI_Wnd;

typedef struct	tagNPCQuestInfo : public SQuestSimple
{
	tagNPCQuestInfo()
	{
	}

	tagNPCQuestInfo(BM::GUID const &rkGuid, const SQuestSimple &rkState)
		:kNpcGuid(rkGuid), tagQuestSimple(rkState)
	{
	}

	static bool SimpleEqual(const tagNPCQuestInfo &rkLeft, const tagNPCQuestInfo &rkRight)
	{
		return (rkLeft.kNpcGuid == rkRight.kNpcGuid) && (rkLeft.iQuestID == rkRight.iQuestID);
	}

	BM::GUID kNpcGuid;

	//
	typedef struct tagPlayerLevelDiff
	{
		tagPlayerLevelDiff(int const PlayerLevel)
			:iPlayerLevel(PlayerLevel)
		{}

		inline bool const operator () (tagNPCQuestInfo const* pkElement)
		{
			return operator ()(*pkElement);
		}

		bool const operator () (tagNPCQuestInfo const& rkElement)
		{
			if( QS_Begin == rkElement.cState
			&&	rkElement.iQuestMaxLevel < iPlayerLevel )
			{
				return true;//삭제
			}
			return false;
		}
		int const iPlayerLevel;
	} SPlayerLevelDiff;
} SNPCQuestInfo;
typedef std::list< SNPCQuestInfo* > ContNpcQuestInfo;

typedef struct tagImportanceQuestInfo
{
	typedef std::list< int >	CONT_QUEST_ID;

	bool Find( int const iQuestID ) const
	{
		if( !kQuestList.empty() )
		{
			CONT_QUEST_ID::const_iterator qst_itor = kQuestList.begin();
			for(;qst_itor != kQuestList.end(); ++qst_itor)
			{
				CONT_QUEST_ID::value_type const& kValue = (*qst_itor);
				if( kValue == iQuestID )
				{
					return true;
				}
			}
		}
		return false;
	}

	int	iGroupNameNo;
	CONT_QUEST_ID kQuestList;
} SImportanceQuestInfo;

typedef struct tagCustomQuestMarkerInfo
{
	tagCustomQuestMarkerInfo()
		: iQuestID(0), iBeginTTID(0), iEndTTID(0), iBeginIconIDX(0), iEndIconIDX(0)
	{
		iIngIconIDX = 0;
		iNotYetIconIDX = 0;
		iBookIconIDX = 0;
		strBegin.clear();
		strEnd.clear();
		strIng.clear();
		strNotYet.clear();
	}
	int iQuestID;
	int iBeginTTID;
	int iEndTTID;
	int iBeginIconIDX;
	int iEndIconIDX;
	int iIngIconIDX;
	int iNotYetIconIDX;
	int iBookIconIDX;
	std::string strBegin;
	std::string strEnd;
	std::string strIng;
	std::string strNotYet;
}SCustomQuestMarker;

typedef struct tagQuestOrderByResult
{
	explicit tagQuestOrderByResult(int const QuestID, EQuestState const State)
		: iQuestID(QuestID)
		, eState(State)
	{}

	tagQuestOrderByResult(tagQuestOrderByResult const& rhs)
		: iQuestID(rhs.iQuestID)
		, eState(rhs.eState)
	{}

	EQuestState eState;
	int iQuestID;
}SQuestOrderByResult;

typedef std::map< int, SCustomQuestMarker > CONT_QUESTCUSTOMMARKER;

//
float const MAX_QUESTOUT_VISIBLE_TIME = 6.0f;	//6초
typedef struct tagQuestOutInfo	//정보 갱신될 때 알려주는데 쓰이는 구조체
{
	float fStartTime;
	std::wstring wstrInfo;
	tagQuestOutInfo()
	{
		fStartTime = 0;
		wstrInfo.clear();
	}

	tagQuestOutInfo(float fTime, std::wstring wstr)
	{
		fStartTime = fTime;
		wstrInfo = wstr;
	}

	bool IsTimeOver()
	{
		return g_pkApp->GetAccumTime() - fStartTime >= MAX_QUESTOUT_VISIBLE_TIME;
	}
}SQuestOutInfo;
typedef std::list< SQuestOutInfo > ContQuestOut;

//
typedef std::map< int, PgQuestInfo* > ContMyQuestInfo;
typedef std::map< int, PgQuestInfo* > ContQuestInfo;
typedef std::list< SNPCQuestInfo* > NPCQuestInfoList; // All NPC Quest
typedef std::map< BM::GUID, ContQuestID > ContNpcDailyQuest; // NPC -> Daily
typedef std::map< int, SImportanceQuestInfo > ContImportanceQuest;
typedef std::set< int > CONT_QUESTID_SET;


namespace PgQuestManUtil
{
	extern std::wstring const kFullQuestWndName;
	extern std::wstring const kWantedQuestWndName;
	extern std::wstring const kWantedQuestCompleteWndName;
	extern std::wstring const kBoardQuestWndName;

	extern std::wstring const kCompleteButtonName;
	extern std::wstring const kAcceptButtonName;
	extern std::wstring const kRejectButtonName;
	extern std::wstring const kGiveupButtonName;
	extern std::wstring const kFunc1ButtonName;
	extern std::wstring const kFunc2ButtonName;
	extern std::wstring const kRemoteFormName;
	extern std::wstring const kRemoteCompleteBtnName;
	extern std::wstring const kRemoteGiveupBtnName;

	void ParseDialogForStep(std::wstring const &wstrOrigin,  std::wstring const &kKey, ContDialogStep& kCont);
	void LockPlayerInput(int const iLockType); // move lock
	void UnLockPlayerInput(int const iLockType);
	void LockPlayerInputMove(int const iLockType); // action lock
	void UnLockPlayerInputMove(int const iLockType);

	typedef enum eQuestMarkDrawOrder
	{
		EQMDO_HIGH				= 0,
		EQMDO_ING				= 1,
		EQMDO_REPEAT_START,
		EQMDO_ADVENTURE_START,
		EQMDO_ONEDAY_START,
		EQMDO_WEEKLY_START,
		EQMDO_HERO_START,
		EQMDO_COUPLE_START,
		EQMDO_CUSTOM_START,
		EQMDO_REPEAT_END,
		EQMDO_ADVENTURE_END,
		EQMDO_ONEDAY_END,
		EQMDO_WEEKLY_END,
		EQMDO_HERO_END,
		EQMDO_COUPLE_END,
		EQMDO_CUSTOM_END
	} EQuestMarkDrawOrder;

	SQuestOrderByResult QuestOrderByState(BM::GUID const &rkGuid, const ContNpcQuestInfo& rkVec);
	bool IsInResetScheduleQuest(int const iQuestID);

	void CloseAllQuestTalkUI();
};


struct SResumeNextQuestTalk
{
	SResumeNextQuestTalk()
		: kNpcGuid(), iQuestID(0)
	{
	}

	SResumeNextQuestTalk(BM::GUID const& rkNpcGuid, int const iQuestID)
		: kNpcGuid(rkNpcGuid), iQuestID(iQuestID)
	{
	}

	SResumeNextQuestTalk(SResumeNextQuestTalk const& rhs)
		: kNpcGuid(rhs.kNpcGuid), iQuestID(rhs.iQuestID)
	{
	}

	void operator = (SResumeNextQuestTalk const& rhs)
	{
		kNpcGuid = rhs.kNpcGuid;
		iQuestID = rhs.iQuestID;
	}

	bool Empty() const
	{
		return (0 == iQuestID) && (BM::GUID::IsNull(kNpcGuid));
	}

	BM::GUID kNpcGuid;
	int iQuestID;
};

typedef struct tagItemDependQuestInfo
{
private:
	typedef std::set< PgQuestInfo const* > CONT_DEPEND_QUEST_INFO;
public:
	tagItemDependQuestInfo();
	tagItemDependQuestInfo(tagItemDependQuestInfo const& rhs);
	~tagItemDependQuestInfo();

	void Add(PgQuestInfo const* pkQuestInfo);
	bool IsDepend(PgMyQuest const* pkMyQuest) const;
	void ClearLastClearCount();

private:
	CONT_DEPEND_QUEST_INFO kContQuestID;
	mutable size_t m_ulLastClearCount;
} SItemDependQuestInfo;
typedef std::map< int, SItemDependQuestInfo > CONT_QUEST_DEPEND_ITEM;

class	PgQuestMan
	: public PgIXmlObject, public PgQuestFullScreenDialog, public PgLetterDialog, public PgWarningDialog, public PgMissionCard
{
	friend struct Loki::CreateUsingNew< PgQuestMan >;
public:

	PgQuestMan();
	~PgQuestMan();

	void Init();

	void Destroy();
	void Clear();
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	bool ParseImportanceQuest(TiXmlElement const* pkElement, bool const IsReLoad = false);
	bool ParseCustomQuestMarker(TiXmlElement const* pkElement, bool const IsReLoad = false);
	bool CheckCustomQuestMarker(int const iQuestID, SCustomQuestMarker& MarkerInfo);
	bool FindQuest(int const iQuestID);
	const PgQuestInfo* GetQuest(int const iQuestID);//	Quest 폴더에서 해당 ID의 퀘스트 파일을 찾아서 PgQuestXML 객체를 생성해서 리턴한다.
	bool GetQuest(ContQuestInfo& rkCont, EQuestType eType = QT_Scenario);	//기본적으로 영웅퀘스트만 모으자
	bool RemoveQuest(int const iQuestID);//완료 한 퀘스트는 메모리에서 제거 하자
	void User_Quest_State_Changed();//	플레이어의 퀘스트 상태가 변경되었다.
	void Send_Drop_Quest(int const iQuestID);//	특정 퀘스트를 포기할 것을 서버에 전송한다.
	void Send_Dialog_Select(int const iQuestID, int const iDialogID, int const iSelectID, int const iVal1, int const iVal2);
	void Send_ReqDialogClose(int const iQuestID, int const iDialogID);
	void SaveNPCQuestInfo(const SNPCQuestInfo& kQuestSimpleInfo);//	서버로부터  NPC 퀘스트 정보를 받았는데, 월드에 NPC 가 없을때, 일단 이 함수를 이용해서 퀘스트 정보를 저장해둔다.
	size_t PopNPCQuestInfo(BM::GUID const &kNPC_GUID, ContNpcQuestInfo &rkVec) const;
	void ClearNPCQuestInfo();//위의 Pop해도 List에 남는다. 맵 이동할 때 이걸 제거 해 주자
	size_t CanNPCQuestInfo(BM::GUID const &rkNpcGuid);
	void ShowQuestInfo(const SUserQuestState& rkOldState, const SUserQuestState& rkState);//퀘스트 정보를 메인화면에 뿌려준다.
	void ShowQuestInfo(EQuestState const eState, std::wstring const& rkDesc, int const iCur = 0, int const iEnd = 0);

	//새로운 UI
	void CallQuestDialog(BM::GUID const &rkGuid, int const iQuestID, int const iDialogID);
	void CallNewDialog(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkDialogInfo);
	void CallNextDialog();
	void ClearRecentQuestInfo();

	void CallCompleteStamp(int const iQuestID);
	void CallCompleteStamp(std::vector< std::wstring > const& rkVec);
	bool CanNextDialog(const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkCurDialog, int const iNextDialogID);
	bool MakeRewardText(PgQuestInfo const *pkQuestInfo, std::vector< std::wstring >& rkOut);
	bool IsDependIngQuestItem(int const iItemNo, CItemDef const* pkItemDef);
	bool IsDependIngQuestMonster(int const iMonsterNo, int const iGroundNo);

	void AddRecentBeginQuest(int const iQuestID);
	void DelRecentBeginQuest(int const iQuestID);
	int GetRecentBeginQuest();

	void IsComplete(int const iQuestID);
	void DelFromComleteSet(int const iQuestID);
	void ResumeNextQuestTalk(SResumeNextQuestTalk const& rkResumNextQuestTalk);

	bool IsDependQuestItem(int const iItemNo) const;
	void ClearDependQuestItem(bool const bNeedUpdate = false);

	bool IsMiniQuestList(int const iQuestID) const		{ return m_kMiniListMng.IsInList(iQuestID); }
	bool RemoveMiniQuestList(int const iQuestID);
	bool AddMiniQuestList(int const iQuestID);
	bool MiniQuestStatus() const						{ return m_kMiniListMng.MiniQuestStatus(); }
	void ShowMiniQuest(bool const bVisible)				{ m_kMiniListMng.Visible(bVisible); }
	size_t GetMiniQuestCount() const					{ return m_kMiniListMng.Count(); }
	void LoadMiniQuest()								{ m_kMiniListMng.Load(); }
	void SaveMiniQuest()								{ m_kMiniListMng.Save(); }
	ContImportanceQuest const& GetImportanceQuestList() const { return m_kImportanceQuest; }

	void CallRealmQuestUI(SRealmQuestInfo const& rkRealmQuestInfo);
	void CallRealmQuestMiniUI(SRealmQuestInfo const& rkRealmQuestInfo);
	void UpdateRealmQuestCount(SRealmQuestInfo const& rkRealmQuestInfo);
	void UpdateRealmQuestNpc(BM::GUID const& rkNpcGuid);
	void CallRealmQuestBuffNotice(std::wstring const& rkRealmQuestID, bool const bSuccess, DWORD const dwNowSec) const;

	void AddNpcName(BM::GUID const& rkNpcGuid, std::wstring const& rkName);
protected:
	void UpdateRealmQuestCountUI(XUI::CXUI_Wnd* pkTopWnd, CONT_REALM_QUEST_EVENT::mapped_type const& rkRealmQuestEvent) const;

public:
	CLASS_DECLARATION_S_NO_SET(SResumeNextQuestTalk, ResumeNextQuestTalk);
	CLASS_DECLARATION_S(BM::GUID, TalkSessionGuid);

	CLASS_DECLARATION_S(bool, EventScriptDialog);
	CLASS_DECLARATION_S_NO_SET(int, CurLetterTTW);
	CLASS_DECLARATION_S_NO_SET(int, RecentQuestID);
	CLASS_DECLARATION_S_NO_SET(int, RecentDialogID);
	CLASS_DECLARATION_S_NO_SET(EQuestDialogType, RecentDialogType);
	
	CLASS_DECLARATION_S(std::wstring, TempLevelMsg);
	CLASS_DECLARATION_S_NO_SET(int, IsComplete);
	CLASS_DECLARATION_S(int, BeginEventID);

	void UpdateQuestOut(CXUI_Wnd* pUI);
	size_t GetNpcDailyInfo(BM::GUID const& rkNpcGuid, ContQuestID& rkOut)const;
protected:
	void SetResumeNextQuestTalk(SResumeNextQuestTalk const& rkResumNextQuestTalk);
	void ResetResumeNextQuestTalk();
	bool CheckThreadLoading(int const iQuestID);

	void CallNpcTalk(const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkQuestDialog);
	void BuildNpcDailyInfo(PgQuestInfo const* pkQuestInfo);
	void BuildDependItem(PgQuestInfo const* pkQuestInfo);
	void ShowQuestInfo(std::wstring const& rkContents);

	CLASS_DECLARATION_S_NO_SET(CONT_QUESTID_SET, ContRandomQuest);
private:
	mutable Loki::Mutex m_kMutex;
	ContQuestID m_kRecentBeginQuest;	// 가장 최근에 수락한 퀘스트
	ContQuestInfo m_kQuestMap;//로딩된 PgQuestXML 리스트(일종의 캐시같은거)
	NPCQuestInfoList m_SavedNPCQuestInfoList;
	BM::TObjectPool< SNPCQuestInfo > m_kPool_NPCQuestInfo;
	BM::TObjectPool< PgQuestInfo > m_kPool_QuestInfo;
	ContQuestOut m_ContQuestOut;
	ContNpcDailyQuest m_kDailyNpcQuest;
	ContImportanceQuest m_kImportanceQuest;
	CONT_QUESTCUSTOMMARKER m_kCustomQuestMaker;

	PgQuestMiniListMng m_kMiniListMng;
	CONT_QUESTID_SET m_kContCompleteQuestID;
	CONT_QUEST_DEPEND_ITEM m_kContDependItem;

	// Realm quest
	CONT_REALM_QUEST_EVENT m_kContRealmQuestEvent;
};
#define g_kQuestMan SINGLETON_CUSTOM(PgQuestMan, CreateUsingNiNew)

//
XUI::CXUI_Wnd* CallQuestWnd(std::wstring const &rkFormName);

typedef struct tagHideScrollQuestMark
{
	inline bool const operator () (tagNPCQuestInfo const* pkElement)
	{
		return operator ()(*pkElement);
	}

	bool const operator () (tagNPCQuestInfo const& rkElement)
	{
		const PgQuestInfo* pkQuestInfo = g_kQuestMan.GetQuest( rkElement.iQuestID );
		if( !pkQuestInfo )
		{
			return false;
		}

		if( QT_Scroll == pkQuestInfo->Type() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}SHideScrollQuestMark;

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUESTMAN_H