#ifndef FREEDOM_DRAGONICA_CONTENTS_LWDEFENCEMODE_H
#define FREEDOM_DRAGONICA_CONTENTS_LWDEFENCEMODE_H

#include "lwUI.h"

typedef enum eExchangeItemResultTextType
{// value가 텍스트번호이자 타입을 정의하도록 했음. 추가되는 타입이 있으면 해당 TT를 연결해주면 된다.
	EIRTT_NONE			= -1,		// 아무것도 보여주지 않는다.
	EIRTT_TARGET_CNT	= 0,		// 교환될 아이템의 갯수만 보여준다.
	EIRTT_MATERIAL_CNT	= 759902,	// 재료 아이템의 보유 갯수와 교환에 필요한 갯수를 보여준다.
}EEIResultTT;

typedef struct tagItemExchangeForm
{
	std::wstring		szFrom;			// 윈도우 이름
	EGemStoreMenu		Menu;			// 교환 타입
	EEIResultTT			ResultTextType;	// 결과 메시지 타입
	int					ContentMsgNo;	// 설명 텍스트 번호
	int					ErrorMsgNo;		// 에러 텍스트 번호
}SItemExchangeForm;

SItemExchangeForm const szExchangeUI[] = {
	{L"SFRM_EXCHANGE_SOCKETCARD",		EGemStoreMenu::EGM_SOCKETCARD,		EEIResultTT::EIRTT_MATERIAL_CNT,	759900,	759901},
	{L"SFRM_EXCHANGE_ELUNIUM",			EGemStoreMenu::EGM_ELUNIUM,			EEIResultTT::EIRTT_TARGET_CNT,		310002,	310007},
	{L"SFRM_EXCHANGE_PURESILVER_KEY",	EGemStoreMenu::EGM_PURESILVER_KEY,	EEIResultTT::EIRTT_TARGET_CNT,		310012,	310013}
};

int const ExchangeFormMaxSize = sizeof(szExchangeUI) / sizeof(SItemExchangeForm);

namespace lwDefenceMode
{
	// 골동품 관련 코드 ///////////////////////////////////////////////////////////////////////////
	bool lwOnMouseUpChangeAntiqueIcon(lwUIWnd MaterialSelf); // 드래그앤드롭으로 아이템 등록
	bool lwShowTargetAntiqueItem(lwUIWnd TopWnd); // 아이콘 윈도우에 해당 아이템을 보여준다.
	void lwClickChangeAntiqueButton(lwUIWnd lwSelf, bool const bConfirm); // 교환 신청
	void lwClickAntiqueButtonUpDown(lwUIWnd lwSelf, bool const IsUpButton = false); // 교환할 아이템 선택
	bool IsCanChangeAntique(E_GEMSTORE_TYPE	const eGemStoreType, DWORD const & ItemNo); // 교환 가능한 골동품인지 확인
	void lwInitCollectAntiqueWnd(lwUIWnd TopWnd); // 창 초기화
	///////////////////////////////////////////////////////////////////////////////////////////////

	// 아이템 교환 관련 코드 /////////////////////////////////////////////////////////////////////////
	bool lwOnMouseUpChangeIcon(lwUIWnd MaterialSelf, int const Menu); // 드래그앤드롭으로 아이템 등록
	bool lwShowTargetItem(lwUIWnd TopWnd, int const Menu); // 아이콘 윈도우에 해당 아이템을 보여준다.
	void lwClickChangeItemButton(lwUIWnd lwSelf, bool const bConfirm, int const Menu); // 교환 신청
	void lwClickItemButtonUpDown(lwUIWnd lwSelf, int const Menu, bool const IsUpButton = false); // 교환할 아이템 선택

	void GetAbleChangeItem(CONT_DEFGEMSTORE const * pDef, int const MaterialItemNo, int const Menu, std::map<int, int> & ContAbleChangeItem );
	// 교환 가능한 아이템인지 확인
	bool IsCanExchangeItem(DWORD const & MaterialItemNo, int const Menu);
	bool IsCanExchangeItem(DWORD const & MaterialItemNo, DWORD & TargetItemNo, int const Menu);
	bool IsCanExchangeItem(CONT_DEFGEMSTORE const * pDef, DWORD const & MaterialItemNo, int const Menu);
	bool IsCanExchangeItem(DWORD const & MaterialItemNo, DWORD const & TargetItemNo, DWORD & NeedItemCnt, DWORD & TargetItemCnt, int const Menu);

	void lwInitExchangeItemWnd(lwUIWnd TopWnd); // 창 초기화
	///////////////////////////////////////////////////////////////////////////////////////////////

	// 피로도 관련 코드 ///////////////////////////////////////////////////////////////////////////
	int lwGetDefenceFatigue(); // 현재 피로도를 가져온다
	int lwGetMaxDefenceFatigue(); // 최대 피로도를 가져온다
	lwWString lwCallDefenceFatigueWindow(lwUIWnd lwWnd); // 피로도 정보를 보여주는 창을 오픈
	void lwSend_PT_C_M_REQ_OPEN_DEFENCE_WINDOW(); // 파티장이 디펜스모드 창을 열었다고 파티원들에게 알려주기 위해 요청
	void lwSend_PT_C_M_REQ_ENTER_DEFENCE(); // 디펜스모드에 들어간다고 요청
	void lwSend_PT_C_M_REQ_CANCLE_DEFENCE(); // 디펜스모드 입장 취소
	void lwUseRestoreItem(lwUIWnd lwWnd); // 피로도회복 아이템 사용
	///////////////////////////////////////////////////////////////////////////////////////////////

	// 포인트복제기 관련 코드 /////////////////////////////////////////////////////////////////////
	int lwGetHideDamageStageNo();	// Config.ini에서 설정한 값
	int lwGetCopyPointChargeCount();	// 포인트 복제기 누적횟수
	int lwGetMaxCopyPointChargeCount();	// 포인트 복제기 최대 누적횟수
	void lwSend_C_M_REQ_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT(); // 포인트 전환 요청
	void lwSend_PT_C_M_REQ_KILL_MONSTER_BY_ACCUMPOINT(); // 몬스터 제거 요청.
	void SetUVIndextMarbleForm(int const Count = 0); // 누적횟수에 따라 구슬 표시
	void SetNowPoint(int const Point= 0); // 현재 누적포인트 표시
	void SetNextPoint(int const Point = 0); // 다음 누적포인트 표시
	void OnOffExchangeButton(bool bValue); // 전략포인트 전환 버튼 활성화, 비활성화
	void SetMonsterKillButtonText(int Num = 0); // 몬스터 제거 회수 텍스트.
	///////////////////////////////////////////////////////////////////////////////////////////////

	void RegisterWrapper(lua_State *pkState);
	void RecvDefenceMode_Command(WORD const wPacketType, BM::Stream& Packet);	// 패킷 처리
};

#endif //FREEDOM_DRAGONICA_CONTENTS_PGDEFENCEMODE_H