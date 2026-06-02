#ifndef FREEDOM_DRAGONICA_CONTENTS_BOOK_LWUIBOOK_H
#define FREEDOM_DRAGONICA_CONTENTS_BOOK_LWUIBOOK_H

#include "lwUI.h"
#include "PgWorldMapPopUpUI.h"

class PgAchievements;
namespace lwUIBook
{
	typedef enum : BYTE
	{
		ACTOR_NONE		= 0,
		ACTOR_NPC		= 1,
		ACTOR_MONSTER,
		ACTOR_ELITE,
	}EActorType;

	typedef struct tagMonDistinctInfo
	{
		int iResNo;
		SResourceIcon kRscIcon;
		SIconImage kIconImage;
		std::wstring strPath;
		tagMonDistinctInfo()
		{
			iResNo = 0;
			strPath.clear();
		}
	}SMonDistinctInfo;

	typedef struct tagAchieveSortKey
	{
		tagAchieveSortKey()
			: iIdx(0)
			, bIsAttainment(false)
		{};
		tagAchieveSortKey(int Idx, bool Attainment)
			: iIdx(Idx)
			, bIsAttainment(Attainment)
		{};
		bool operator<(tagAchieveSortKey const& rhs) const;

		int iIdx;
		bool bIsAttainment;
	}SAchieveSortKey;

	typedef struct tagBookMonsterInfo
	{
		tagBookMonsterInfo()
			: kInfo()
			, kName()
			, MonID(0)
		{}

		bool IsNull()
		{
			return kName.empty();
		}
		SMonDistinctInfo kInfo;
		std::wstring kName;
		int MonID;
	}SBookMonsterInfo;

	typedef struct tagBookButtonSaveActorInfo
	{
		tagBookButtonSaveActorInfo()
			: Type(ACTOR_NONE)
			, kName()
			, iMonID(0)
		{}

		EActorType	Type;
		std::wstring kName;
		int iMonID;

		void WriteFormPacket(BM::Stream& kPacket)
		{
			kPacket.Push(Type);
			kPacket.Push(kName);
			kPacket.Push(iMonID);
		}
		void ReadFromPacket(BM::Stream& rkPacket)
		{
			rkPacket.Pop(Type);
			rkPacket.Pop(kName);
			rkPacket.Pop(iMonID);
		}
	}SBBSAVEACTORINFO;

	typedef std::map<SAchieveSortKey, TBL_DEF_ACHIEVEMENTS> kAchieveSortCont;

	typedef std::vector<SBookMonsterInfo> ContMonDistinctVec;	//UI를 위해서 필요한 정보를 순서대로 모으자

	typedef std::map<std::wstring, std::wstring> ContMonDistinct;

	extern int iForceRecommandItemNo; // 추천 장비에서 강제로 세트를 보여 줄 때 사용

	//public
	bool RegisterWrapper(lua_State *pkState);
	bool SetWorldPage(lwUIWnd kWnd, int const iMapNo, int const iType);
	void SetBookMiniMap(lwUIWnd kWnd, int const iMapNo);
	void SetQuestPage(lwUIWnd kWnd, int const iType);
	void SetAchievePage(lwUIWnd kWnd, int const iType);
	void OnClickBookQuest(lwUIWnd kWnd);
	void OnClickBookNPC(lwUIWnd kWnd);
	int OnClickBookAchieveIcon(lwUIWnd kWnd);
	void OnClickBookAchieveGetItem(lwUIWnd kWnd, bool bReal);
	void UpdateAchieveItem(int const iIndex, bool const bEquip);
	void OnClockBookMonList(lwUIWnd kWnd, int iDelta = 0);
	void OnClickSetBestItem(lwUIWnd kWnd);
	void OnClickSetBestItemUpdateUI(PgInventory* pkInv, lwUIWnd kWnd, CONT_ITEMNO const& kContItemNo);
	void RefreshMyEquipItemUI(PgInventory* pkInv, lwUIWnd kWnd);
	void SetClearBestItem(lwUIWnd kWnd);
	void CallBookItemToolTip(lwUIWnd kWnd, int const iType, lwPoint2& rPT);
	void CallActorInfo(lwUIWnd UISelf);
	void ClickActorSlot(lwUIWnd UISelf, int const Type);
	void ClickSearchMyLoc(lwUIWnd UISelf);
	void ClickSearchNpcLoc(lwUIWnd UISelf);
	void OnClickQuestListTypeChange(lwUIWnd UIParent);
	void OnClickBookRankPage(lwUIWnd kWnd);
	void OnCallDetailAchievementView();
	void OnCloseBookUI();

	//private
	void SetTownPage(lwUIWnd kWnd, int const iMapNo);
	void SetFieldPage(lwUIWnd kWnd, int const iMapNo);
	void SetBossPage(lwUIWnd kWnd, int const iMapNo);
	bool GetMonsterBookInfo(int const MonID, SBookMonsterInfo& kInfo, bool const IsElite, int& BossResNo);
	void ClearBookActorSlot(XUI::CXUI_Wnd* pWnd);

	void SetHeroQuestPage(lwUIWnd kWnd);
	void SetNormalQuestPage(lwUIWnd kWnd);
	void SetImportanceQuestList(lwUIWnd kWnd);
	static bool SortQuestPageListItem(SUserQuestState const& rhs, SUserQuestState const lhs);

	int GetQuestMarkUVIndex(EQuestState const State);

	void SetClearAchieveEle(XUI::CXUI_Wnd* pkEle);
	void SetAchieveEle(XUI::CXUI_Wnd *pkEle, int const iIndex, PgAchievements const* pkAchieve, TBL_DEF_ACHIEVEMENTS const& rkValue, bool const bEquip);

	void ClearMonsterIcon(XUI::CXUI_Wnd* pkWnd);

	void SetAchievementRankSlot(XUI::CXUI_Wnd* pkSlot, SACHIEVEMENTRANK const& kRankInfo);
	void OnRecvBookRankInfo(BM::Stream& rkPacket);
}

struct SBookCameraInfo
{
	SBookCameraInfo()
		: Pos()
		, ZoomMin(0.0f)
		, ZoomMax(0.0f)
	{}
	NiPoint3 Pos;
	float ZoomMin;
	float ZoomMax;
};

typedef std::vector< int >	CONT_BOOK_MAPNO;
typedef std::map< std::string, SBookCameraInfo >	CONT_BOOK_ACTOR_CAMERA;
typedef std::map< int, CONT_BOOK_MAPNO >			CONT_BOOK_MONSTER_CONTANTS;
class PgBookActorCameraMgr
{
public:
	PgBookActorCameraMgr();
	~PgBookActorCameraMgr();

	bool Init();
	bool GetActorCameraInfo(std::string const& ActorName, SBookCameraInfo& CameraInfo);
	SBookCameraInfo const& GetDefaultInfo() const { return m_kDefaultInfo; };
	CONT_BOOK_MAPNO const& GetMonsterContants(int const MonID);
private:
	CONT_BOOK_ACTOR_CAMERA	m_kCameraContainer;
	SBookCameraInfo			m_kDefaultInfo;
	CONT_BOOK_MONSTER_CONTANTS	m_kMonContantsContainer;
};

#define g_kBookActorCameraMgr SINGLETON_STATIC(PgBookActorCameraMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_BOOK_LWUIBOOK.H