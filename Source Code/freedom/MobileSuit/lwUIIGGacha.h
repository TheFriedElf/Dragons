#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIIGGACHA_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIIGGACHA_H

#include "BM/twrapper.h"
#include "variant/PgGambleMachine.h"
class lwUIWnd;

class PgIGGacha
{
protected:
	typedef std::vector<int> CONT_ITEMNO;
	typedef std::map<eGambleCoinGrade,CONT_ITEMNO> CONT_IGGACHA_ITEMNO;
public:
	PgIGGacha();
	~PgIGGacha();

	//@가다코인 NPC 작동
	void ReqIGGachaList();

	//1. 가다코인 자판기 오픈
	//	- 소유 코인을 얻어와 각 UI에 세팅(골드, 실버, 브론즈)
	//		: 코인이 부족하면 해당 버튼 비활성화함
	void CallIGGachaUI();

	//	- tbl서 아이템 관련 데이터를 읽어들임(골드, 실버, 브론즈)
	//	- 각 종류별 아이템 확률 별로 정렬(낮은것 우선)
	//	- 골드 부분 최초 활성화
	//		: 활성화 된 컨테이너 UI에 보여줌

	//2. 코인 사용 
	//	- 사용한다는 패킷을 보냄
	//	- 충분한 코인을 소유했는가 체크
	bool SendUseCoin(eGambleCoinGrade const eCoinType);
	bool SendUseCash(eGambleCoinGrade const eCoinType);

	//3. 패킷을 받음 
	bool RecvIGGachaPacket(WORD const dwPacketType, BM::Stream& rkPacket);

	bool ShowItemList(XUI::CXUI_Wnd* pkMain, eGambleCoinGrade const eCoinType);
	void UpdateCoinCnt(XUI::CXUI_Wnd* pkMain);
	void UpdateCashEnough(XUI::CXUI_Wnd* pkMain);

	int GetItemNo(eGambleCoinGrade const eCoinType, int iIdx);
	int GetItemNextItemNo(eGambleCoinGrade const eCoinType);

	int GetMyPlayerCoinCnt(eGambleCoinGrade const eCoinType);
	bool CheckEnoughCash(eGambleCoinGrade const eCoinType);

	void NotifyEarnItem(PgBase_Item const& rkItem) const;	// 얻은 아이템 알림
	void NotifyLuckyGuy(std::wstring const& rkPlayerName, PgBase_Item const& rkItem) const;	// 행운의 당첨자 알림

protected:
	void Sort();
	void StopRoulette(int const iResultItemNo) const;		// UI에 결과 아이템 보여줌
	static bool PopLess(SGAMBLEMACHINEITEM const& rhs, SGAMBLEMACHINEITEM const& lhs);
protected:
	CONT_GAMBLEMACHINE m_kContItemBag;
	CONT_IGGACHA_ITEMNO m_kContResultItems;
};

#define g_kIGGacha SINGLETON_STATIC(PgIGGacha)

namespace lwUIIGGacha
{
	extern void RegisterWrapper(lua_State *pkState);
	extern void CallIGGachaUI();
	extern void ReqIGGachaList();
	extern bool SendUseCoinIGGacha(int const iCoinType);
	extern bool SendUseCashIGGacha(int const iCoinType);
	extern bool ShowItemListIGGacha(int const iCoinType);

	extern void UpdateCoinCntIGGacha(lwUIWnd kMain);
	extern void UpdateCashEnoughIGGacha(lwUIWnd kMain);
	
	extern int GetNextItemNo(int const iCoinType);
	extern int GetMyPlayerIGGachaCoinCnt(int const iCoinType);
	extern bool CheckEnoughIGGachaCash(int const iCoinType);
	extern int GetNeedCashIGGacha(int const iCoinType);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIIGGACHA_H