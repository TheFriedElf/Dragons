#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICOSTUMEMIX_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICOSTUMEMIX_H
#include "variant/PgGambleMachine.h"
#include "PgCashShop.h"
class lwUIWnd;
class lwWString;
class PgBase_Item;

typedef enum {
		E_NONE = 0,
		E_NORMAL_GRADE = 1,
		E_ADV_GRADE = 2,
		E_SPCL_GRADE = 3,
		E_ARTIFACT_GRADE = 4,
		E_LEGEND_GRADE = 5,
}eCostumeMixResultItemGradeType;

class PgCashCostumeMix_ExpectResultItemUI
{// 예측되는 아이템 UI부분을 설정함
public:
	typedef std::map<int,CONT_GAMBLEITEM> CONT_CSGACHA_PAGE;
	typedef std::map<int,CONT_GAMBLEITEM> CONT_RARE_ITEM;
	typedef std::vector<int> CONT_INT;
	static int const MAX_ITEM_CNT_ON_PAGE;
	static int const BUNDLE_UNIT;

public:
	PgCashCostumeMix_ExpectResultItemUI();
	~PgCashCostumeMix_ExpectResultItemUI();
	
	void Init();

	// 모든 예상 아이템의 정보를 설정
	bool SetUI_ExpectResultItem(CONT_GAMBLEMACHINE const& kContItemBag,CONT_GAMBLEMACHINEGROUPRES const& kContRareItemGroupInfo);

	// 일반, 고급, 스페셜타입의 나올수 있는 아이템 리스트를 보여줌(탭)
	bool ShowItemList(int const iType);
	
	// 낱개 아이템 페이지 보기
	bool ShowItemPage(int const iPage);
	bool PrevItemPage();
	bool NextItemPage();
	
	// 레어 아이템 보기
	bool ShowRareItemPage(int const iPage);
	bool PrevRareItemPage();
	bool NextRareItemPage();

	int	 GetCurrentRareItemPage() const { return m_iCurrentRareItemPage; }
	void SetCurrentRareItemPage(int const iPage) { m_iCurrentRareItemPage = iPage; }
	
	bool ShowPageBundle(int const iPageBundle);
	bool PrevPageBundle();
	bool NextPageBundle();
	int	 GetCurrentPageBundle() const { return m_iCurrentPageBundle; }
	void SetCurrentPageBundle(int const iPageBundle) { m_iCurrentPageBundle = iPageBundle; }
	
	bool ShowPart(eGambleCoinGrade eType);

	// 점수 별로 발생하는 코스튬 등급 보기
	bool ShowExpectResultRoullet(int const iPoint);
	
	bool HideAllItemIcon();
	
	int GetMaxRareItemPage();
	int  CalcGroupResIndex(int const iGroupNo);
	// 미리 보기 장착 페이지 - 젤나중에 할것
	bool EquipPreViewItem(lwUIWnd kWnd);
	bool EquipPreViewItemUpdate(SGAMBLEMACHINEITEM const& kItemInfo);
	bool EquipPreViewRareGroupItem(int const iGroupNo);
	void ClearPreViewItem();
	bool DelPreViewItem(int const iItemNo);
	bool ExchangeSamePosEquipPreViewItem(SGAMBLEMACHINEITEM const& kItemInfo);
	void UpdatePreViewItem();

	bool CallEquipItemToolTip(lwUIWnd kWnd);
	bool CallRareGroupTooltip(int const iGroupNo);

	//bool CallSingleItemToolTip(lwUIWnd kWnd);		// lwUICSGacha것 씀
	//int  CalcGroupResIndex(int const iGroupNo);	// lwUICSGacha것 씀
	/////////////////////////////////////////////////////////////////////////////////////////////

private:
	int GetMaxBundlePage() const;

private:
	int m_iCurrentPage;
	int m_iCurrentPageBundle;
	int m_iMaxPage;
	int m_iCurrentRareItemPage;

	CONT_GAMBLEMACHINE m_kContItemBag;		// 서버에서 보내주는 단일 아이템 목록
	CONT_CSGACHA_PAGE  m_kContItemPage;		// 단일 아이템 각 페이지 마다 포함하고 있는 아이템 
	
	RECV_UI_EQUIP_PREVIEW_CONT m_kContPreViewItem;	// 미리 보기 장비창에 보여지는 아이템들

	CONT_RARE_ITEM	m_kContRareItem;						// 레어 아이템 목록
	CONT_GAMBLEMACHINEGROUPRES m_kContRareItemGroupInfo;	// 레어아이템 그룹 정보
	CONT_INT	m_kContSortedRareGroupNo;						// 먼저 보여져야할(iDisplayRank로 정렬된) 그룹 정보
};

class PgCashCostumeMix
{
private:
	struct SSlotInfo
	{
	public:
		SSlotInfo()
			:bUsing(false)
			,iPoint(0)
		{}
		SSlotInfo(PgBase_Item kitem, SItemPos kinvPos, int ipoint)
			:kItem(kitem)
			,kInvPos(kinvPos)
			,bUsing(true)
			,iPoint(ipoint)
		{}

		bool operator == (SSlotInfo const& rhs) const
		{
			return (this->kInvPos == rhs.kInvPos);
		}
	public:
		PgBase_Item kItem;
		SItemPos	kInvPos;
		bool bUsing;
		int iPoint;
	};
	typedef std::vector<SSlotInfo> CONT_SLOT;
	
	struct SExpectRoulletTbl
	{
	public:
		SExpectRoulletTbl()
			:iNomalCnt(0)
			,iAdvCnt(0)
			,iSpclCnt(0)
			,iArtifactCnt(0)
			,iLegendCnt(0)
		{}
		SExpectRoulletTbl(int inormalCnt, int iadvCnt=0, int ispclCnt=0, int iartifactCnt=0, int ilegendCnt=0)
			:iNomalCnt(inormalCnt)
			,iAdvCnt(iadvCnt)
			,iSpclCnt(ispclCnt)
			,iArtifactCnt(iartifactCnt)
			,iLegendCnt(ilegendCnt)
		{}
	public:
		int iNomalCnt;
		int iAdvCnt;
		int iSpclCnt;
		int iArtifactCnt;
		int iLegendCnt;
	};

public:
	PgCashCostumeMix();
	~PgCashCostumeMix();

	bool Init();
	void ReqResultItemList();
	bool RecvPacket(WORD const dwPacketType, BM::Stream& rkPacket);
	
	bool InsertSlotForMix(XUI::CXUI_Wnd* pkCashInvSlot);
	bool RemoveSlot(XUI::CXUI_Wnd* pkRegSlot);

	PgCashCostumeMix_ExpectResultItemUI& GetPreviewResultItemUIMgr() { return m_kPreview_ResultItem; }
	
	int GetCurMixPoint() const { return m_iCurMixPoint; }
	bool AddMixPoint(int const iAdd);
	bool SetCurMixPoint(int const iNewPoint);
	
	int GetMinMixPoint() const { return m_iMinMixPoint; }
	void SetMinMixPoint(int const iMin) { m_iMinMixPoint = iMin; }

	int GetMaxMixPoint() const { return m_iMaxMixPoint; }
	void SetMaxMixPoint(int const iMax) { m_iMaxMixPoint = iMax; }
	
	bool IsFullSlot();
	lwWString GetCurrentPointState();
	
	bool SetSlotInfo(int const iIdx, SSlotInfo const kNewSlotInfo);
	bool GetSlotInfo(int const iIdx, SSlotInfo& kResult);
	bool RemoveSlotInfo(int const iIdx);
	bool IsExistSameItemInSlot(SSlotInfo const& kSlotInfo);
	bool GetEmptySlotIdx(int& iResult);
	bool SetSlotInfoToUI(int const iIdx, SSlotInfo const& kSlotInfo);
	void ReSortSlotInfo();	//중간에 빈 슬롯을 재정렬
	bool ClearAllSlotUI();

	bool IsEnableSlot() const;	// 더이상 슬롯에 등록이 가능한가?
	bool CloseAllRemainSlot();
	bool CloseSlotUI(int const iIdx);
	
	// 포인트에 따른 테이블 배치도
	bool GetExpectRoulletTbl(int iPoint, SExpectRoulletTbl& kResult) const;
	bool SetRoulletTblInfoToMainUI(SExpectRoulletTbl const& kRoulletTblInfo);
	bool SetRoulletTblInfoToSubUI(SExpectRoulletTbl const& kRoulletTblInfo, int const iPoint);

	//패킷
	void ReqMixUpReady();		// 조합 시작 버튼 누를때 요청
	void ReqRoulletReload();	// 룰렛 리로드 요청
	void ReqGetResultItem();	// 룰렛이 끝나서 받을 아이템을 요청함

	int  GetRemainReloadCnt() const { return m_iRemainReloadCnt; }
	void SetRemainReloadCnt(int const iReloadCnt) { m_iRemainReloadCnt = iReloadCnt; }
	bool  DecRemainReloadCnt();
	
	bool ShowErrorMsg(HRESULT const& hResult) const;

	void SetItemOnRouletteUI();
	void UpdateReloadTextUI();

	int GetItemMixGrade(int const iItemNo);
	bool SetPointExplanText();
	bool SetPrevRoulletTbl();

	bool ShowSubUIPreviewRoulletTbl(int const iPoint);
	bool IsInSlot(XUI::CXUI_Wnd* pkCashInvSlot);
	bool CallSlotItemToolTip(int const iIdx);
	void ClearSlotInfo();	// 슬롯 정보를 비움

private:
	bool GetItemInfoFromCashInvSlot(XUI::CXUI_Wnd* pkCashInvSlot, PgBase_Item& kResultItem, int& iErrTTNo);
	HRESULT RecvReady(BM::Stream& rkPacket);
	HRESULT RecvRoulletItemSetting(BM::Stream& rkPacket);

private:
	PgCashCostumeMix_ExpectResultItemUI m_kPreview_ResultItem;
	CONT_GAMBLEMACHINEMIXUP		 m_kContMixUp;
	CONT_GAMBLEMACHINEMIXUPPOINT m_kContMixUpPoint;
	int m_iCurMixPoint;

	int m_iMinMixPoint;
	int m_iMaxMixPoint;

	CONT_SLOT m_kCont_Slot;

	int m_iRemainReloadCnt;
	CONT_GAMBLEITEM	   m_kContRouletteItem;	// 룰렛에 보여질 아이템들(아이템 번호만 사용)
	CONT_GAMBLEMACHINERESULT m_kContResultItem;		// 룰렛에서 최종 선택되어 얻을 아이템
	int m_iResultItemIdx;					// 룰렛 UI에 아이템을 보여주는 인덱스 중 최종 선택될 아이템의 UI 인덱스
};
#define g_kCostumeMix SINGLETON_STATIC(PgCashCostumeMix)


namespace lwUICostumeMix
{
	extern void RegisterWrapper(lua_State *pkState);
	extern bool CostumeMixShowPart(int iType);
	extern bool CostumeMixPrevItemPage();
	extern bool CostumeMixNextItemPage();
	extern bool CostumeMixShowItemPage(int const iPageIdx);
	extern bool CostumeMixShowRareItemPage(int const iPage);
	extern bool CostumeMixPrevRareItemPage();
	extern bool CostumeMixNextRareItemPage();

	extern bool CostumeMixInsertSlotForMix(lwUIWnd kCashInvSlot);
	extern bool CostumeMixRemoveSlot(lwUIWnd kRegSlot);

	extern int CostumeMixGetCurMixPoint();
	extern bool CostumeMixSetCurMixPoint(int const iNewPoint);

	extern int CostumeMixGetMinMixPoint();
	extern int CostumeMixGetMaxMixPoint();
	extern lwWString CostumeMixGetCurrentPointState();
	extern bool CostumeMixClearAllSlotUI();

	extern int  CostumeMixGetRemainReloadCnt();
	extern bool CostumeMixDecRemainReloadCnt();

	extern void CostumeMixReqResultItemList();
	extern void CostumeMixReqMixUpReady();		// 조합 시작 버튼 누를때 요청
	extern void CostumeMixReqRoulletReload();	// 룰렛 리로드 요청
	extern void CostumeMixReqGetResultItem();	// 룰렛이 끝나서 받을 아이템을 요청함

	extern bool CostumeMixShowSubUIPreviewRoulletTbl(int const iPoint);

	extern void CostumeMixSetPreViewEquipItem(lwUIWnd kWnd);
	extern void CostumeMixClearPreViewEquipItem();
	extern bool CostumeMixEquipPreViewRareGroupItem(int const iGroupNo);
	extern bool CostumeMixCallRareGroupTooltip(int const iGroupNo);

	extern bool CostumeMix_IsInSlot(lwUIWnd kCashInvSlot);
	extern bool CostumeMix_CallSlotItemToolTip(int const iSlotIdx);
	extern void CostumeMix_ClearSlotInfo();

	extern int CostumeMix_GetMaxRareItemPage();
};

extern std::wstring GetCostumeMixGradeString(int const iCostumeMixGrade);
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICOSTUMEMIX_H