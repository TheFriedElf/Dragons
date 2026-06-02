#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRADE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRADE_H

#include "Lohengrin/packetstruct.h"

struct tagItem;
class BM::Stream;
class PgPlayer;

typedef std::map<BM::GUID, SExchangeItem> ItemContForUI;

class PgTradeMgr
{
public:
	PgTradeMgr();
	virtual ~PgTradeMgr();

	void NewTradeStart(BM::GUID const kTradeGuid, BM::GUID const kOtherGuid );
	void ModifyItem(SItemPos const& rkPos, PgBase_Item const* pkItem, bool const bDel = false);
	void Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM(BM::Stream* pkPacket);
	void Recv_PT_M_C_NFY_EXCHANGE_ITEM_RESULT(BM::Stream* pkPacket);
	void Recv_PT_M_C_NFY_EXCHANGE_ITEM_READY(BM::Stream* pkPacket);
	void SetTradeGuid(BM::GUID kGuid);
	BM::GUID const GetTradeGuid() const;
	void OnConfirmButton(bool const bValue);
	void UpdateWnd(XUI::CXUI_Wnd* pkSdw, int const iWho, bool const bReady, bool const bConf = false);
	void OnOkButton();
	bool IsConfirm()	{ return m_kTradeInfo.bCasterConfirm; }
	static bool CheckItemToGiveOther(SItemPos const& rkItemPos, PgPlayer* pkPlayer, PgBase_Item& rkItem);
	
	PgBase_Item const* GetTradeItemAt(int const iAt, bool const bMine = true) const;	//이걸로 툴팁 정보를 만들자
	void Clear();

private:
	ItemContForUI		m_kItemContForUI;
	ItemContForUI		m_kOtherItemContForUI;
	STradeInfo			m_kTradeInfo;
	bool MakePacketStruct(STradeInfoPacket &rkInfo);
	bool DiplayCoin(XUI::CXUI_Wnd* pkSdw, __int64 const 
iCoin, int const iWho);
	__int64 const GetMyCoin();
	XUI::CXUI_Wnd* GetSdwUI(BM::GUID const &rkGuid, int &rkWho);	//UI얻어오기 귀찮다
};

#define g_kTradeMgr SINGLETON_STATIC(PgTradeMgr)

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRADE_H