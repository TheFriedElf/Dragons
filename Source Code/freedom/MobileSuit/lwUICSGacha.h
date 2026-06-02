#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWUICSGACHA_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWUICSGACHA_H
#include "variant/PgGambleMachine.h"
#include "PgCashShop.h"

class lwUIWnd;

class PgCSGacha
{
private:
	typedef std::map<int,CONT_GAMBLEITEM> CONT_CSGACHA_PAGE;
	typedef std::map<int,CONT_GAMBLEITEM> CONT_RARE_ITEM;
	typedef std::vector<int> CONT_INT;
	static int const MAX_ITEM_CNT_ON_PAGE;
	static int const BUNDLE_UNIT;
	
public:
	PgCSGacha();
	~PgCSGacha();

	void ReqCSGachaItemList();			//아이템 리스트 요청하고(캐시샵에서 가챠 버튼 클릭)
	bool RecvCSGachaPacket(WORD const dwPacketType, BM::Stream& rkPacket);
	void UpdateOwnCoinCnt(lwUIWnd kWnd);			//소지한 코인 수량을 UI에 출력하고
	void CallBuyCoinUI();				// 코인 구입 UI 호출할수 있고(modal타입)
	bool ReqStartRoulette();			// 받을 아이템을 요청한다(5회 반복할때까지 이걸 썻으면 한다)
	bool ReqReloadRoulette();			// 룰렛 리로드
	bool ReqGetResultItem();		// 룰렛이 끝나서 받을 아이템을 요청함

	void NotifyEarnItem(PgBase_Item const& rkItem) const;		// 자기가 얻은 아이템 표시

	bool HideAllItemIcon();
	
	int  GetRemainReloadCnt() const { return m_iRemainReloadTime; }
	bool  DecRemainReloadCnt();
	
	bool ShowItemPage(int const iPage);
	bool PrevPage();
	bool NextPage();
	int	 GetCurrentPage() const { return m_iCurrentPage; }
	void SetCurrentPage(int const iPage) { m_iCurrentPage = iPage; }
	
	bool ShowPageBundle(int const iPageBundle);
	bool PrevPageBundle();
	bool NextPageBundle();
	int	 GetCurrentPageBundle() const { return m_iCurrentPageBundle; }
	void SetCurrentPageBundle(int const iPageBundle) { m_iCurrentPageBundle = iPageBundle; }
	
	bool EquipPreViewItem(lwUIWnd kWnd);
	bool EquipPreViewItemUpdate(SGAMBLEMACHINEITEM const& kItemInfo);
	bool EquipPreViewRareGroupItem(int const iGroupNo);
	void ClearPreViewItem();
	bool DelPreViewItem(int const iItemNo);
	bool ExchangeSamePosEquipPreViewItem(SGAMBLEMACHINEITEM const& kItemInfo);
	void UpdatePreViewItem();
	bool CallEquipItemToolTip(lwUIWnd kWnd);
		
	bool CallRareGroupTooltip(int const iGroupNo);

	bool ShowRareItemPage(int const iPage);
	bool PrevRareItemPage();
	bool NextRareItemPage();
	int	 GetCurrentRareItemPage() const { return m_iCurrentRareItemPage; }
	void SetCurrentRareItemPage(int const iPage) { m_iCurrentRareItemPage = iPage; }
	int GetMaxRareItemPage();

	bool CallSingleItemToolTip(lwUIWnd kWnd);
	int  CalcGroupResIndex(int const iGroupNo);

	bool ShowErrorMsg(HRESULT const& kResult) const;
private:
	void Init();
	void SetItemOnListUI();
	void SetItemOnRouletteUI();
	void UpdateReloadTextUI();
	int GetMaxBundlePage() const;

	void SetRareItemOnUI();

	bool CheckEnoughCoin();		// 충분한 코인이 있는지 확인
	void SetFixedResultOnRouletteUI();		// 결과 아이템과 랜덤 아이템을 룰렛에 세팅하고, 룰렛이 멈출때 아이템 설곳을 정한다
	int GetGroupNoBySlot(int iSlot); //레어아이템 목록에서 일치하는 랭크값의 인덱스 번호를 얻음
private:
	CONT_GAMBLEMACHINE m_kContItemBag;		// 서버에서 보내주는 단일 아이템 목록 (인간족 또는 용족)
	CONT_CSGACHA_PAGE  m_kContItemPage;		// 단일 아이템 각 페이지 마다 포함하고 있는 아이템 
	int m_iCurrentPage;
	int m_iCurrentPageBundle;
	int m_iMaxPage;

	CONT_GAMBLEITEM	   m_kContRouletteItem;	// 룰렛에 보여질 아이템들(아이템 번호만 사용)
	CONT_GAMBLEMACHINERESULT m_kContResultItem;		// 룰렛에서 최종 선택되어 얻을 아이템
	int m_iRemainReloadTime;				// 룰렛 재설정 횟수(기록용)
	int m_iResultItemIdx;					// 룰렛 UI에 아이템을 보여주는 인덱스 중 최종 선택될 아이템의 UI 인덱스
	
	RECV_UI_EQUIP_PREVIEW_CONT m_kContPreViewItem;	// 미리 보기 장비창에 보여지는 아이템들

	CONT_RARE_ITEM	m_kContRareItem;						// 레어 아이템 목록
	CONT_GAMBLEMACHINEGROUPRES m_kContRareItemGroupInfo;	// 레어아이템 그룹 정보
	CONT_INT	m_kContSortedRareGroupNo;						// 먼저 보여져야할(iDisplayRank로 정렬된) 그룹 정보
	int	m_iCurrentRareItemPage;
	
};
#define g_kCSGacha SINGLETON_STATIC(PgCSGacha)

namespace lwUICSGacha
{
	extern void RegisterWrapper(lua_State *pkState);
	extern void ReqCSGachaItemList();
	extern void CallCSGachaBuyCoinUI();
	extern void UpdateCsGachaCoinCnt(lwUIWnd kWnd);
	extern bool ReqStartCSGachaRoulette();
	extern void ReqReloadCSGachaRoulette();
	extern void ReqGetCSGachaResultItem();
	extern int GetCSGachaRemainReloadCnt();
	extern bool DecCSGachaRemainReloadCnt();
	
	extern bool PrevCSGachaItemPage();
	extern bool NextCSGachaItemPage();
	extern int	GetCSGachaCurrentPage();
	extern void SetCSGachaCurrentPage(int const iPage);
	extern bool ShowCSGachaItemPage(int const iPage);

	extern bool ShowCSGachaPageBundle(int const iPageBundle);
	extern bool PrevCSGachaPageBundle();
	extern bool NextCSGachaPageBundle();
	extern int	GetCSGachaCurrentPageBundle();
	extern void SetCSGachaCurrentPageBundle(int const iPageBundle);
	
	extern void SetCSGachaPreViewEquipItem(lwUIWnd kWnd);
	extern void ClearCSGachaPreViewEquipItem();

	extern bool EquipCSGachaPreViewRareGroupItem(int const iGroupNo);
	extern bool CallCSGachaRareGroupTooltip(int const iGroupNo);

	extern bool ShowCSGachaRareItemPage(int const iPage);
	extern bool PrevCSGachaRareItemPage();
	extern bool NextCSGachaRareItemPage();
	extern int	GetCSGachaCurrentRareItemPage();
	extern void SetCSGachaCurrentRareItemPage(int const iPage);
	extern int  GetCSGachaMaxRareItemPage();

	extern bool CallCSGachaItemToolTip(lwUIWnd kWnd);
	extern bool CallCSGachaEquipItemToolTip(lwUIWnd kWnd);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWUICSGACHA_H