#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWSOULTRANSFER_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWSOULTRANSFER_H

#include "Variant/PgSoulTransfer.h"
#include "lwUI.h"

namespace lwSoulTransfer
{
	void RegisterWrapper(lua_State *pkState);
	void lwCallSoulTransferUI(bool const bSoulInsert);

	void InitIcon(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert);

	void InitMainText(XUI::CXUI_Wnd* pMain, bool const bSoulInsert);
	void InitFrameText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert);
	void SetLimitText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert, PgBase_Item const &kItem);
	void SetRateText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert, PgBase_Item const &kTransitionItem, PgBase_Item const &kDestItem);
	void SetRateText();
	void SetNeedSoulText(XUI::CXUI_Wnd* pFramem, int const iNeedSoul);
	void SetNeedGoldText(XUI::CXUI_Wnd* pFrame, __int64 const i64NeedMoney);
	void ResetUI();
	
//진행바
	void InitProgress(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert);
	void SetProgress(float fPercent, bool const bIsMain, std::wstring kWndText);
	void InitProgressTime();

	void lwSetProgressMain(float const fPercent);
	void lwSetProgressInsurance(float const fPercent);
	void lwSetProgressRateUp(float const fPercent);
	bool lwCanStartSoulTransProgress();
	bool lwIsHaveRarity();
	bool lwIsEndSoulTransProgress();
	void lwStartSoulTransProgress(float fTotalTime);
	void lwUpdateSoulTransProgress();
//아이콘
	void lwSoulTrans_IconAni(char const *pszText, bool const bOn);	//돌아가는 애니메이션 ON/OFF
	bool lwAutoInsertItem();	//아이템 등록

	bool lwSetAdditionItem();	//보험스크롤/확률증가 아이템 설정
	bool lwAddInsuranceItem();	//보험스크롤 추가
	bool lwDelInsuranceItem();	//보험스크롤 해제
	bool lwUseInsuranceItem();	//보험스크롤 사용했는지 확인
	void SetInsuranceBtn(bool const bDel, bool const bAdd);	//보험아이템 -/+ 버튼 설정

	bool lwAddRateUpItem();		//확률증가아이템 추가
	bool lwDelRateUpItem();		//확률증가아이템 감소
	int lwGetRateUpItemCount(); //확률증가아이템 갯수 리턴
	void SetRateUpBtn(bool const bDel, bool const bAdd);	//확률증가아이템 -/+ 버튼 설정
	
	bool lwSetExtractitem();	//추출기 아이템 설정(From XML)
	bool SetExtractitem(PgBase_Item const &rkItem);		//추출기 아이템 설정

	bool SetSelectItem(std::wstring const &rkText, PgBase_Item const &rkItem);	//아이템 셋팅하기
	bool GetSelectItem(std::wstring const &rkText, PgBase_Item &rkItem);	//아이템 가져오기
	bool SetSelectItemPos(std::wstring const &rkText, SItemPos const &kItemPos);	//아이템 셋팅하기
	bool GetSelectItemPos(std::wstring const &rkText, SItemPos &kItemPos);	//아이템 가져오기
	bool GetMyItem(SItemPos const &rkItemPos, PgBase_Item &kTargetItem);//내 인벤에 있는 아이템가져오기
	bool IsHaveItem(int const iItemNo);//내 인벤에 아이템이 있는지
	int GetHaveItemCount(int const iItemNo);//내 인벤에 아이템이 몇개 있는지
	void AutoInsertItem(SItemPos const &rkItemPos, PgBase_Item const &rkItem);	//인벤 아이템 우클릭 시 자동으로 들어가기
	
	void lwSetSoulTrans_ResultItem();	//결과 아이템 설정
	void SetResultItem(PgBase_Item &rkItem);		//결과 아이템 설정

	//현재 열려있는 UI 타입이 소울 삽입인가?
	bool IsSoulInsertUI();
	//영력추출/삽입기 텍스트 설정(제한레벨/삽입가능부위/필요소울/필요골드)
	void lwSetSoulTransText();
	//메세지 출력
	void CallSoulTransMessage(ESoulTransMsg const eType, int const iItemNo = 0);
	//패킷전송
	void SendSoulExtract();
	void SendSoulTransition();

	//인벤토리 표시용
	bool IsWearable(PgBase_Item const &rkItem, bool& rbGray);
	bool IsAddResourceItem(PgBase_Item const &rkItem, bool& rbGray);
	bool IsAddDestItem(PgBase_Item const &rkItem, bool& rbGray);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWSOULTRANSFER_H