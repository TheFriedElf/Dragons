#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISEALPROCESS_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISEALPROCESS_H
#include "lwUI.h"
#include "lwGUID.h"
#include "lwPacket.h"
#include "PgScripting.h"
typedef enum eSealProcessListItemSeletecType
{	
	E_OFF_CHECK=1,
	E_ON_CHECK= 2,
}ESealProcessListItemSelectType;

typedef enum eSealScroll_ItemCustomValue
{// AT_USE_ITEM_CUSTOM_VALUE_1에 해당 하는 값
	E_SEAL_REMOVE_NORMAL= 102,
	E_SEAL_REMOVE_GOOD	= 103,
	E_SEAL_REMOVE_BEST	= 104,
}ESeal_ItemType;


class PgSealProcess
{
public:
	typedef struct tagScrollCnt
	{
		int iUseScrollCnt;
		int iScrollMaxCnt;
		tagScrollCnt():iUseScrollCnt(0), iScrollMaxCnt(0){}
		tagScrollCnt(int const iMaxCnt):iUseScrollCnt(0), iScrollMaxCnt(iMaxCnt){}
	}SSealScrollCnt;

	typedef SItemPos TargetItemPos;
	typedef SItemPos ScrollItemPos;
	typedef std::map<TargetItemPos, ScrollItemPos> CONT_SEALPROCESS_PAIR;
	typedef std::map<ScrollItemPos, SSealScrollCnt> CONT_SCROLLCNT;

public:
	PgSealProcess();
	virtual ~PgSealProcess();
	
	// UI를 호출 하고 
	void CallSealProcessUI(DWORD const dwUseItemType, int const iItemNo);
	// 대상 아이템을 UI에 추가하고
	bool TargetItemAddtoUI(XUI::CXUI_Wnd* pWnd, PgBase_Item const* pkItem, SItemPos rkPos, bool const bFindSeal=false);
	
	// UI에 등록된 아이템을 선택할시 저장하고(패킷 만들기 위해)
	bool AddItemtoSealProcess(TargetItemPos const& kTargetItemPos, ScrollItemPos const& kScrollPos);
	// UI에서 선택된 아이템을 해제 할때 저장 정보를 지운다
	bool RemoveItemtoSealProcess(TargetItemPos const& kTargetItemPos);
	bool GetSealScrollPos(SItemPos& kScrollPos_out, bool const bRelease=false);

	// 하나씩 서버와 주고 받으며 처리한다 (서버에서 이렇게 밖에 안됨)
	bool SendPacket();
	void RcevPacket(WORD const& wPacketType, BM::Stream& rkPacket);

	// 처리중이라는 알림 UI 호출
	void CallProcessingNoticeUI();	

	bool IsComplete();	
	void Clear();
	int  GetUsedScrollItemNo() const { return m_iScrollItemNo; }
	
	// 보내야할 패킷이 딜레이 되었는가
	void SetSendPacketDelayed(bool const bDelayed) { m_bPacketSendDelayed = bDelayed; }
	bool IsSendPacketDelayed() const { return m_bPacketSendDelayed; }

	void SetSendPacketTime();
	bool IsAbleToSendPacket();
		
	// 한번에 몽땅 보내기 (서버에서 지원 안함)
	//	void SendUnSealProcessPacket();
	//	void SendSealProcessPacket();
	
	bool SelectAll();
	void SetSelectAll(bool const bCheck) { m_bSelectAll =  bCheck;}
	bool IsSelectAll() const { return m_bSelectAll; }
protected:
	bool Init(int const iScrollItemNo);
	void OffCheckAllListItem();
protected:
	int					  m_iScrollItemNo;
	CONT_SCROLLCNT		  m_kContScrollCnt;
	CONT_SEALPROCESS_PAIR m_kContSealPair;

	DWORD				  m_dwPacketSendTime;
	bool				  m_bPacketSendDelayed;

	bool				  m_bSelectAll;
};

#define g_kSealProcess SINGLETON_STATIC(PgSealProcess)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
// 이하는 Lua로 노출 부분 

extern bool lwItemSealProcessListSelect(lwUIWnd kItem, int const iSelectType);
extern void lwItemSealProcessStop();

extern bool lwItemSealProcessSendPacket(lwUIWnd Self);

extern void lwItemSealProcessIconDraw(lwUIWnd Self);
extern void lwItemSealProcessIconToolTip(lwUIWnd Self);

// 보내야하지만 딜레이된 패킷을 지켜보다 시간이 되면 패킷 보냄
extern bool lwItemSealProcessWatchingDelayedPacket();
//모두 선택
extern bool lwItemSealProcessSelectAll();
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISEALPROCESS_H