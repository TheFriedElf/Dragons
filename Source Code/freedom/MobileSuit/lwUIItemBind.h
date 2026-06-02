#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMBIND_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMBIND_H
#include "lwUI.h"
#include "lwGUID.h"
#include "lwPacket.h"
#include "PgScripting.h"

namespace lwUIItemBind
{
	extern bool RegisterWrapper(lua_State *pkState);

	extern void UseBindItem(SItemPos const& rkItemInvPos);
	extern bool CallBindItemUI();
	extern void lwItemBindDisplaySrcIcon(lwUIWnd kWnd);
	extern bool lwItemBindSendReq();
	extern bool lwItemBindCheckOK();
	extern void lwItemBindClearAll();
	extern void lwItemBindClearTargetItemInfo();

	extern void UseUnbindItem(SItemPos const& rkItemInvPos, int const iDays);
	extern bool CallUnbindItemUI(int const iDays);
	extern void lwItemUnbindDisplaySrcIcon(lwUIWnd kWnd);
	extern bool lwItemUnbindSendReq();
	extern bool lwItemUnbindCheckOK();
	extern void lwItemUnbindClearAll();
	extern void lwItemUnbindClearTargetItemInfo();
	extern void RecvBindResult(BM::Stream& rkPacket);
	extern void RecvUnbindResult(BM::Stream& rkPacket);
};

class PgItemBindMgr
{
public:
	PgItemBindMgr();
	virtual ~PgItemBindMgr();

public:	
	// 바인딩 캐쉬템 정보
	void SetCashItemFromInv(SItemPos const& rkItemPos);
	SItemPos const GetCashItemFromInv() const
	{ return m_kCashItemPos; }
	
	// 올려진 대상 아이템 정보
	virtual void SetTargetItemFromInv(SItemPos const& rkItemPos);
	SItemPos const GetTargetItemPosFromInv() const 
	{ return m_kTargetItemPos; }
	BM::GUID const GetTargetItemGuid() const 
	{ return m_kTargetItem.Guid(); }

	// 올려진 대상 아이템을 표시
	void DisplayItem(XUI::CXUI_Wnd *pWnd);

	// 바인딩을 시도할수 있는 상태인가?
	bool CheckOK();
	
	// 캐쉬템 및, 대상 아이템 정보 지움
	void ClearAll();
	// 대상 아이템 정보만 지움
	void ClearTargetItemInfo();
	virtual void ClearAllandCloseUI();

	// 완료시 이펙트
	virtual bool ShowCompleteEffect(std::string const& kEffName);

protected:
	void SetTargetItemPos(SItemPos const& rkItemPos);

protected:
	SItemPos	m_kTargetItemPos;
	PgBase_Item	m_kTargetItem;

	SItemPos	m_kCashItemPos;
	PgBase_Item	m_kCashItem;
};


class PgItemUnbindMgr : public PgItemBindMgr
{
public:
	PgItemUnbindMgr();
	virtual ~PgItemUnbindMgr();

public:		
	virtual void SetTargetItemFromInv(SItemPos const& rkItemPos);
	virtual void ClearAllandCloseUI();
};

#define g_kItemBindMgr SINGLETON_STATIC(PgItemBindMgr)
#define g_kItemUnbindMgr SINGLETON_STATIC(PgItemUnbindMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMBIND_H