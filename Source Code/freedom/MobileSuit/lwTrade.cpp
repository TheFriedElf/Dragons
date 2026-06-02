#include "StdAfx.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "lwTrade.h"
#include "Variant/Item.h"
#include "PgPilotMan.h"
#include "PgPilot.h"

int const MAX_EXCHANGE_ITEM = 10;

PgTradeMgr::PgTradeMgr()
{
	Clear();
}
PgTradeMgr::~PgTradeMgr()
{
	Clear();
}

void PgTradeMgr::Clear()
{
	m_kTradeInfo.Init();
	m_kItemContForUI.clear();
	m_kOtherItemContForUI.clear();
}

void PgTradeMgr::NewTradeStart(BM::GUID const kTradeGuid, BM::GUID const kOtherGuid )
{
	Clear();

	if (BM::GUID::NullData() == kTradeGuid || BM::GUID::NullData() == kOtherGuid)
	{
		NILOG(PGLOG_WARNING, "[NewTradeStart] GUID Is Null kTradeGuid:%s  kOtherGuid:%s\n", kTradeGuid.str().c_str(), kOtherGuid.str().c_str());
		return;
	}
	
	m_kTradeInfo.kTradeGuid = kTradeGuid;
	m_kTradeInfo.kTargetGuid = kOtherGuid;
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer)
	{
		m_kTradeInfo.kCasterGuid = pkPlayer->GetID();
	}
}

void PgTradeMgr::ModifyItem(SItemPos const& rkPos, PgBase_Item const* pkItem, bool const bDel)
{
	if (!pkItem)
	{
		NILOG(PGLOG_WARNING, "[ModifyItem] pkItem Is Null\n");
		return;
	}

	if (BM::GUID::NullData() == m_kTradeInfo.kTradeGuid)	//거래 GUID가 없으면 안되
	{
		NILOG(PGLOG_WARNING, "[ModifyItem] TradeGuid Is Null\n");
		return;
	}

	if (bDel)	//삭제
	{
		ItemContForUI::iterator ui_it = m_kItemContForUI.find(pkItem->Guid());
		if (m_kItemContForUI.end() != ui_it)
		{
			m_kItemContForUI.erase(ui_it);
		}
		else
		{
			NILOG(PGLOG_WARNING, "[ModifyItem] Cannot Find Item. Guid Is %s\n", pkItem->Guid().str().c_str());
			return;
		}
	}
	else		//추가
	{
		if(static_cast<int>(MAX_EXCHANGE_ITEM) <= m_kItemContForUI.size())	//갯수 초과
		{
			lwAddWarnDataTT(400703);
			return;
		}

		SExchangeItem kItem(*pkItem);
		kItem.kPos = rkPos;
		m_kItemContForUI.insert(std::make_pair(pkItem->Guid(), kItem));	//UI상 순서는 상관없음
	}

	BM::Stream kPacket(PT_C_M_NFY_EXCHANGE_ITEM_ITEM);

	STradeInfoPacket kTempInfo;
	kTempInfo.kTradeGuid = m_kTradeInfo.kTradeGuid;
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer)
	{
		kTempInfo.kOwnerGuid = pkPlayer->GetID();
	}

	ItemContForUI::iterator item_it = m_kItemContForUI.begin();
	while (m_kItemContForUI.end() != item_it)
	{
		const SExchangeItem kMyItem = (*item_it).second;
		kTempInfo.kItemCont.push_back(kMyItem);
		++item_it;
	}

	kTempInfo.kMoney = GetMyCoin();//m_kTradeInfo.i64CasterMoney;//from이 내꺼. 무조건.

	kTempInfo.WriteToPacket(kPacket);

	kPacket.Push(false);		//확정용 패킷인가, 아이템 수정인가
	kPacket.Push(false);		//확정용 일땐 확정인가 취소인가

	NETWORK_SEND(kPacket)
}

void PgTradeMgr::Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM(BM::Stream* pkPacket)
{
	if (!pkPacket)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Packet Is NULL\n");
		return;
	}

	int iRet = 0;
	pkPacket->Pop(iRet);
	if (TR_SUCCESS != iRet)
	{
		lwAddWarnDataTT(400710+iRet);
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] iRet Is not TR_SUCCESS\n");
		return;
	}

	STradeInfoPacket kInfo;
	kInfo.ReadFromPacket(*(pkPacket));
	bool bConfirm = false;	//확정이면 UI로 표시해 줘야 함
	pkPacket->Pop(bConfirm);

	XUI::CXUI_Wnd *pkTrade = XUIMgr.Get(std::wstring(_T("SFRM_TRADE")));
	if (!pkTrade)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui SFRM_TRADE\n");
		return;
	}

	int iWho = 1;
	int iOther = 2;
	int iSize = 0;

	CONT_TRADE_ITEM::iterator item_it;
	if (kInfo.kOwnerGuid == m_kTradeInfo.kCasterGuid)	//내꺼냐
	{
		iWho=1;
		iOther=2;
		m_kTradeInfo.kCasterItemCont.clear();
		m_kTradeInfo.kCasterItemCont = kInfo.kItemCont;
		m_kTradeInfo.bCasterConfirm = bConfirm;
		m_kItemContForUI.clear();
		item_it = m_kTradeInfo.kCasterItemCont.begin();
		iSize = m_kTradeInfo.kCasterItemCont.size();
	}
	else if (kInfo.kOwnerGuid == m_kTradeInfo.kTargetGuid)	//남꺼냐
	{
		iWho=2;
		iOther=1;
		m_kTradeInfo.kTargetItemCont.clear();
		m_kTradeInfo.kTargetItemCont = kInfo.kItemCont;
		m_kTradeInfo.bTargetConfirm = bConfirm;
		if (!bConfirm)
		{
			m_kTradeInfo.bCasterConfirm = bConfirm;	//남이 바꿔도 풀어야함
		}
		m_kOtherItemContForUI.clear();
		item_it = m_kTradeInfo.kTargetItemCont.begin();
		iSize = m_kTradeInfo.kTargetItemCont.size();
	}
	else	//어라 둘다 아니면 에러
	{
		//이거 에러니까 없애야되.
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Cannot Find Guid %s\n", kInfo.kOwnerGuid.str().c_str());
		return;
	}

	BM::vstring kWndColorName(L"SFRM_COLOR");
	kWndColorName+=iWho;

	XUI::CXUI_Wnd *pkColor = pkTrade->GetControl((std::wstring)kWndColorName);
	if (!pkColor)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui %s\n", kWndColorName);
		return;
	}

	XUI::CXUI_Wnd *pkSdw = pkColor->GetControl(std::wstring(_T("SFRM_SDW")));

	if (!pkSdw)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui SFRM_SDW\n");
		return;
	}

	UpdateWnd(pkSdw, iWho, bConfirm);
	if (/*2==iWho && */false == bConfirm)	//남에 물건이 업데 되도 내 준비 확인을 풀어주자
	{
		kWndColorName = L"SFRM_COLOR";
		kWndColorName+=iOther;//L"1";

		XUI::CXUI_Wnd *pkColorMy = pkTrade->GetControl((std::wstring)kWndColorName);
		if (!pkColorMy)
		{
			NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui %s\n", kWndColorName);
			return;
		}

		XUI::CXUI_Wnd *pkSdwMy = pkColorMy->GetControl(std::wstring(_T("SFRM_SDW")));

		if (!pkSdwMy)
		{
			NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui SFRM_SDW\n");
			return;
		}

		UpdateWnd(pkSdwMy, iOther, bConfirm);
	}

	int iCount = 0;
	for (; iCount < iSize; ++iCount)
	{
		BM::vstring kName(L"ICN_MY_ITEM");
		kName+=iCount;
		XUI::CXUI_Icon *pkItem = dynamic_cast<XUI::CXUI_Icon*>(pkSdw->GetControl(std::wstring(kName)));
		if (pkItem)
		{
			SIconInfo kIconInfo = pkItem->IconInfo();
			kIconInfo.iIconResNumber = (*item_it).kItem.ItemNo();
			//pkItem->SetIconInfo(kIconInfo);
		}
		else
		{
			NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui %s\n", kName);
		}
		if (1==iWho)
		{
			m_kItemContForUI.insert(std::make_pair((*item_it).kItem.Guid(), (*item_it)));
		}
		else if (2==iWho)
		{
			m_kOtherItemContForUI.insert(std::make_pair((*item_it).kItem.Guid(), (*item_it)));
		}
		++item_it;
	}

	int const iNull = 0;
	//이놈들은 초기화
	for (int i = iCount; i < MAX_EXCHANGE_ITEM; ++i)
	{
		BM::vstring kName(L"ICN_MY_ITEM");
		kName+=iCount;
		XUI::CXUI_Icon *pkItem = dynamic_cast<XUI::CXUI_Icon*>(pkSdw->GetControl(std::wstring(kName)));
		if (pkItem)
		{
			SIconInfo kIconInfo = pkItem->IconInfo();
			kIconInfo.iIconResNumber = 0;
			pkItem->SetCustomData(&iNull, sizeof(iNull));
		}
		else
		{
			NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM] Can't find ui %s\n", kName);
		}
	}	

	DiplayCoin(pkSdw, kInfo.kMoney, iWho);
}

void PgTradeMgr::UpdateWnd(XUI::CXUI_Wnd* pkSdw, int const iWho, bool const bReady, bool const bConf)	//버튼과 빨간장막만 업데이트 하자
{
	if (!iWho || !pkSdw)
	{
		return;
	}

	BM::vstring szReady(L"CBTN_READY");
	BM::vstring szOk(L"CBTN_OK");

	if (2==iWho)
	{
		szReady = L"CBTN_OTHER_READY";
		szOk = L"CBTN_OTHER_OK";
	}

	XUI::CXUI_Wnd *pkTrade = XUIMgr.Get(std::wstring(_T("SFRM_TRADE")));
	if (!pkTrade)
	{
		NILOG(PGLOG_WARNING, "[UpdateWnd] Can't find ui SFRM_TRADE\n");
		return;
	}

	XUI::CXUI_CheckButton* pkReady = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl((std::wstring)szReady));
	if (!pkReady)
	{
		NILOG(PGLOG_WARNING, "[UpdateWnd] Can't find ui CBTN_READY\n");
		return;
	}

	pkReady->Check(bReady);
	pkReady->Text(TTW(400701+(int)bReady));	//400701 = 준비완료
	
	//여기서 빨간장막 키고 끄자
	XUI::CXUI_Wnd* pkConf = pkSdw->Parent()->GetControl(std::wstring(_T("SFRM_CONFIRM")));
	if (pkConf)
	{
		pkConf->Visible(bReady);
	}

	XUI::CXUI_CheckButton* pkOK = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl((std::wstring)szOk));
	if (!pkOK)
	{
		NILOG(PGLOG_WARNING, "[UpdateWnd] Can't find ui CBTN_OK\n");
		return;
	}

	pkOK->Check(bConf);
	pkOK->ClickLock(bConf || 2==iWho);	//다른사람 버튼은 항상 고정
	pkOK->IsClosed(bConf || 2==iWho);	//다른사람 버튼은 항상 고정
	//if (bConf)
	{
		pkReady->ClickLock(bConf || 2==iWho);
		if(bConf || 2==iWho)
		{
			pkReady->IsClosed(true);
		}
	}

	pkOK->Text(TTW(bReady ? 400533 : 400534)); //준비가 되면 취소버튼을 확인 버튼으로 바꿔주자
	XUI::CXUI_CheckButton* pkMyOK = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl(L"CBTN_OK"));
	XUI::CXUI_CheckButton* pkMyReady = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl(L"CBTN_READY"));
	if(pkMyOK && pkMyReady)
	{
		if(2==iWho) //다른 사람이 준비 눌렀을 때만 내 확인 버튼 누를 수 있도록 하자
		{
			pkMyOK->ClickLock(!bReady && pkMyReady->Check());	
			pkMyOK->IsClosed(!bReady && pkMyReady->Check());
			if(!bReady && !bConf)//아이템이 바뀐것
			{
				pkMyReady->ClickLock(false);
				pkMyReady->IsClosed(false);
			}
		}
		else	//내 동작일 때
		{
			XUI::CXUI_CheckButton* pkOtherReady = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl(L"CBTN_OTHER_READY"));
			if(pkOtherReady)
			{
				pkMyOK->ClickLock(bReady && (!pkOtherReady->Check() || bConf));	
				pkMyOK->IsClosed(bReady && (!pkOtherReady->Check() || bConf));	
			}
		}
	}
}

void PgTradeMgr::Recv_PT_M_C_NFY_EXCHANGE_ITEM_RESULT(BM::Stream* pkPacket)
{
	if (!pkPacket)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_RESULT] Packet Is NULL\n");
		m_kTradeInfo.Init();

		XUIMgr.Close(std::wstring(_T("SFRM_TRADE")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
		XUIMgr.Close(std::wstring(_T("SFRM_MSG_TRADE")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
		XUIMgr.Close(std::wstring(_T("SFRM_MSG_TRADE_REQUEST")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
		return;
	}

	lwCloseToolTip();
	int iRet = 0;
	pkPacket->Pop(iRet);
	BM::GUID kProblemUser;
	std::string kstrName;
	
	if (TR_SUCCESS != iRet)
	{
		pkPacket->Pop(kProblemUser);
		PgPilot* pkPilot = g_kPilotMan.FindPilot(kProblemUser);
		if (pkPilot)
		{
			kstrName = pkPilot->GetID();
		}
	}

	Clear();
	switch(iRet)
	{
	case TR_SUCCESS:
	case TR_FAIL:
		{
			lwAddWarnDataTT(400710+iRet);
		}break;
	case TR_NO_INVEN:	//아마 이것만 들어올것임. 나머지는 Net_PT_M_C_NFY_EXCHANGE_ITEM_REQ 에서 해결
	case TR_TRADED_IN_3SEC:
	case TR_ON_TRADEING:
		{
			BM::vstring kErrorMsg(TTW(400710+iRet));
			if (!kstrName.empty())
			{
				kErrorMsg+=L" : ";
				kErrorMsg+=UNI(kstrName);
			}
			lwAddWarnDataStr(lwWString((std::wstring)kErrorMsg),1);
		}break;
	case TR_NOT_FIND_ITEM:
		{
			lwAddWarnDataTT(400719);
		}break;
	case TR_CASTER_IS_DEAD:
	case TR_TARGET_IS_DEAD:
		{
			lwAddWarnDataTT(400717+iRet);
		}break;
	default:
	    break;
	}

	m_kTradeInfo.Init();

	XUIMgr.Close(std::wstring(_T("SFRM_TRADE")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
	XUIMgr.Close(std::wstring(_T("SFRM_MSG_TRADE")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
	XUIMgr.Close(std::wstring(_T("SFRM_MSG_TRADE_REQUEST")));	//성공하건 말건 일단 UI는 닫자. 마지막 패킷이니까
}

void PgTradeMgr::Recv_PT_M_C_NFY_EXCHANGE_ITEM_READY(BM::Stream* pkPacket)
{
	if (!pkPacket)
	{
		NILOG(PGLOG_WARNING, "[Recv_PT_M_C_NFY_EXCHANGE_ITEM_READY] Packet Is NULL\n");
		return;
	}

	BM::GUID kCallerGuid;
	pkPacket->Pop(kCallerGuid);

	int iWho = 0;

	XUI::CXUI_Wnd* pkSdw = GetSdwUI(kCallerGuid, iWho);
	if (!pkSdw || !iWho)
	{
		return;
	}

	UpdateWnd(pkSdw, iWho, true, true);
}

void PgTradeMgr::SetTradeGuid(BM::GUID kGuid)
{
	m_kTradeInfo.kTradeGuid = kGuid;
}
BM::GUID const PgTradeMgr::GetTradeGuid() const
{
	return m_kTradeInfo.kTradeGuid;
}

PgBase_Item const* PgTradeMgr::GetTradeItemAt(int const iAt, bool const bMine) const
{
	ItemContForUI::size_type const iAt_size_type = iAt;
	if (bMine)
	{
		if (m_kItemContForUI.size() > iAt_size_type)
		{
			ItemContForUI::const_iterator item_it = m_kItemContForUI.begin();
			for (ItemContForUI::size_type i = 0; i < iAt_size_type; ++i)
			{
				++item_it;
			}

			if (m_kItemContForUI.end() != item_it)
			{
				return &((*item_it).second.kItem);
			}
		}
	}
	else
	{
		if (m_kOtherItemContForUI.size() > iAt_size_type)
		{
			ItemContForUI::const_iterator item_it = m_kOtherItemContForUI.begin();
			for (ItemContForUI::size_type i = 0; i < iAt_size_type; ++i)
			{
				++item_it;
			}

			if (m_kOtherItemContForUI.end() != item_it)
			{
				return &((*item_it).second.kItem);
			}
		}
	}

	return NULL;
}

bool PgTradeMgr::MakePacketStruct(STradeInfoPacket &rkInfo)
{
	if (BM::GUID::NullData() == m_kTradeInfo.kTradeGuid)
	{
		NILOG(PGLOG_WARNING, "[MakePacketStruct] m_kTradeInfo.kTradeGuid Is Null\n");
		return false;
	}
	rkInfo.kTradeGuid = m_kTradeInfo.kTradeGuid;
	rkInfo.kMoney = m_kTradeInfo.i64CasterMoney;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer)
	{
		rkInfo.kOwnerGuid = pkPlayer->GetID();
	}
	else
	{
		NILOG(PGLOG_WARNING, "[MakePacketStruct] GetPlayerUnit Is Null\n");
		return false;
	}

	rkInfo.kItemCont = m_kTradeInfo.kCasterItemCont;	//from이 무조건 내꺼다

	return true;
}

void PgTradeMgr::OnConfirmButton(bool const bValue)
{
	if(false==bValue)
	{
		return;
	}
	__int64 const i64TotalCoin = GetMyCoin();

	BM::Stream kPacket(PT_C_M_NFY_EXCHANGE_ITEM_ITEM);

	STradeInfoPacket kInfo;
	if (!MakePacketStruct(kInfo))
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] MakePacketStruct Is Fail\n");
		return;
	}

	XUI::CXUI_Wnd *pkTrade = XUIMgr.Get(std::wstring(_T("SFRM_TRADE")));
	if(pkTrade)
	{
		XUI::CXUI_CheckButton* pkMyReady = dynamic_cast<XUI::CXUI_CheckButton*>(pkTrade->GetControl(L"CBTN_READY"));
		if(pkMyReady)
		{
			pkMyReady->ClickLock(bValue);
			pkMyReady->IsClosed(bValue);
		}
	}

	kInfo.kMoney = i64TotalCoin;

	kInfo.WriteToPacket(kPacket);

	kPacket.Push(!m_kTradeInfo.bCasterConfirm);		//확정용 패킷인가, 아이템 수정인가
	kPacket.Push(bValue);	//확정용 일땐 확정인가 취소인가

	NETWORK_SEND(kPacket)
}

bool PgTradeMgr::DiplayCoin(XUI::CXUI_Wnd* pkSdw, __int64 const iCoin, int const iWho)
{	
	if (!pkSdw)
	{
		NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui pkSdw \n");
		return false;
	}

	__int64 const i64G = iCoin/10000i64;
	__int64 const i64S = iCoin%10000i64/100i64;
	__int64 const i64C = iCoin%100i64;

	char szCoin[255] = {0,};

	XUIMgr.ClearEditFocus();

	XUI::CXUI_Wnd *pkG = pkSdw->GetControl(std::wstring(_T("SFRM_GOLD")));
	if (!pkG)
	{
		NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui SFRM_GOLD\n");
		return false;
	}

	_i64toa(i64G, szCoin, 10);
	std::wstring wstrCoin = UNI(szCoin);

	if (1==iWho)
	{
		XUI::CXUI_Edit *pkGE = dynamic_cast<XUI::CXUI_Edit*>(pkG->GetControl(std::wstring(_T("EDT_COIN"))));
		if (!pkGE)
		{
			NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui EDT_COIN\n");
			return false;
		}

		pkGE->EditText(wstrCoin);
	}
	else
	{
		pkG->Text(wstrCoin);
	}
	

	XUI::CXUI_Wnd *pkS = pkSdw->GetControl(std::wstring(_T("SFRM_SILVER")));
	if (!pkS)
	{
		NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui SFRM_SILVER\n");
		return false;
	}
	
	_i64toa(i64S, szCoin, 10);
	wstrCoin = UNI(szCoin);

	if (1==iWho)
	{
		XUI::CXUI_Edit *pkSE = dynamic_cast<XUI::CXUI_Edit*>(pkS->GetControl(std::wstring(_T("EDT_COIN"))));
		if (!pkSE)
		{
			NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui EDT_COIN\n");
			return false;
		}

		pkSE->EditText(wstrCoin);
	}
	else
	{
		pkS->Text(wstrCoin);
	}


	XUI::CXUI_Wnd *pkC = pkSdw->GetControl(std::wstring(_T("SFRM_COPER")));
	if (!pkC)
	{
		NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui SFRM_COPER\n");
		return false;
	}

	_i64toa(i64C, szCoin, 10);
	wstrCoin = UNI(szCoin);

	if (1==iWho)
	{
		XUI::CXUI_Edit *pkCE = dynamic_cast<XUI::CXUI_Edit*>(pkC->GetControl(std::wstring(_T("EDT_COIN"))));
		if (!pkCE)
		{
			NILOG(PGLOG_WARNING, "[DiplayCoin] Can't find ui EDT_COIN\n");
			return false;
		}

		pkCE->EditText(std::wstring(UNI(szCoin)));
	}
	else
	{
		pkC->Text(wstrCoin);
	}
	
	return true;
}

void PgTradeMgr::OnOkButton()
{
	BM::Stream kPacket(PT_C_M_NFY_EXCHANGE_ITEM_READY);
	NETWORK_SEND(kPacket)
}

XUI::CXUI_Wnd* PgTradeMgr::GetSdwUI(BM::GUID const &rkGuid, int &rkWho)
{
	if(BM::GUID::NullData() == rkGuid)
	{
		return NULL;
	}
	
	if (BM::GUID::NullData() == m_kTradeInfo.kTradeGuid)
	{
		return NULL;
	}

	int iWho = 1;
	const PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer)
	{
		if (pkPlayer->GetID() == rkGuid)	//내꺼
		{
			iWho = 1;
		}
		else if (m_kTradeInfo.kTargetGuid == rkGuid)
		{
			iWho = 2;
		}
		else
		{
			NILOG(PGLOG_WARNING, "[GetSdwUI] Can't find Player GUID %s\n", rkGuid.str().c_str());
			return NULL;
		}
	}

	rkWho = iWho;

	XUI::CXUI_Wnd *pkTrade = XUIMgr.Get(std::wstring(_T("SFRM_TRADE")));
	if (!pkTrade)
	{
		NILOG(PGLOG_WARNING, "[GetSdwUI] Can't find ui SFRM_TRADE\n");
		return NULL;
	}

	BM::vstring kName(L"SFRM_COLOR");
	kName+=iWho;
	XUI::CXUI_Wnd *pkColor = pkTrade->GetControl((std::wstring)kName);
	if (!pkColor)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_COLOR%d\n", iWho);
		return NULL;
	}

	XUI::CXUI_Wnd *pkSdw = pkColor->GetControl(std::wstring(_T("SFRM_SDW")));
	if (!pkSdw)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_SDW\n");
		return NULL;
	}

	return pkSdw;
}

__int64 const PgTradeMgr::GetMyCoin()
{
	int iWho = 0;
	XUI::CXUI_Wnd *pkSdw = GetSdwUI(m_kTradeInfo.kCasterGuid, iWho);
	if (!pkSdw)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_SDW\n");
		return 0;
	}

	XUI::CXUI_Wnd *pkG = pkSdw->GetControl(std::wstring(_T("SFRM_GOLD")));
	if (!pkG)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_GOLD\n");
		return 0;
	}

	XUI::CXUI_Edit *pkGE = dynamic_cast<XUI::CXUI_Edit*>(pkG->GetControl(std::wstring(_T("EDT_COIN"))));
	if (!pkGE)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui EDT_COIN\n");
		return 0;
	}

	std::wstring const wstrTextG = pkGE->EditText();
	__int64 const i64G = _atoi64(MB(wstrTextG.c_str()))*10000i64;

	XUI::CXUI_Wnd *pkS = pkSdw->GetControl(std::wstring(_T("SFRM_SILVER")));
	if (!pkS)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_GOLD\n");
		return 0;
	}

	XUI::CXUI_Edit *pkSE = dynamic_cast<XUI::CXUI_Edit*>(pkS->GetControl(std::wstring(_T("EDT_COIN"))));
	if (!pkSE)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui EDT_COIN\n");
		return 0;
	}

	std::wstring const wstrTextS = pkSE->EditText();
	__int64 const i64S = _atoi64(MB(wstrTextS.c_str()))*100i64;

	XUI::CXUI_Wnd *pkC = pkSdw->GetControl(std::wstring(_T("SFRM_COPER")));
	if (!pkC)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui SFRM_GOLD\n");
		return 0;
	}

	XUI::CXUI_Edit *pkCE = dynamic_cast<XUI::CXUI_Edit*>(pkC->GetControl(std::wstring(_T("EDT_COIN"))));
	if (!pkGE)
	{
		NILOG(PGLOG_WARNING, "[OnConfirmButton] Can't find ui EDT_COIN\n");
		return 0;
	}

	std::wstring const wstrTextC = pkCE->EditText();
	__int64 const i64C = _atoi64(MB(wstrTextC.c_str()));

	__int64 const i64TotalCoin = i64G + i64S + i64C;

	return i64TotalCoin;
}

bool PgTradeMgr::CheckItemToGiveOther(SItemPos const& rkItemPos, PgPlayer* pkPlayer, PgBase_Item& rkItem)
{
	if(!pkPlayer) {return false;}

	if(S_OK == pkPlayer->GetInven()->GetItem(rkItemPos, rkItem) )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
		if (pItemDef)
		{
			SEnchantInfo const& kEhtInfo = rkItem.EnchantInfo();
/*			switch( g_kLocal.ServiceRegion() )    //캐시거래 코드제한 부분 주석처리    2010.06.08 조현건
			{
			case LOCAL_MGR::NC_TAIWAN:
			case LOCAL_MGR::NC_EU:
			case LOCAL_MGR::NC_FRANCE:
			case LOCAL_MGR::NC_GERMANY:
			case LOCAL_MGR::NC_CHINA:
			case LOCAL_MGR::NC_SINGAPORE:
			case LOCAL_MGR::NC_JAPAN:
			case LOCAL_MGR::NC_THAILAND:
			case LOCAL_MGR::NC_INDONESIA:
			case LOCAL_MGR::NC_KOREA:
				{
				}break;
			default:
				{
					if( kEhtInfo.IsTimeLimit() )// 시간제한 옵션(타이완 제외)
					{
						return false;
					}
				}break;
			}
*/
			if( pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) == UICT_EXPCARD )
			{
				if( rkItem.Guid() == pkPlayer->GetInven()->ExpCardItem() )
				{//현제 토글중인것은 팔 수 없다.
					return false;
				}

				SExpCard	kExpCard;
				if( rkItem.Get(kExpCard) )
				{
					if( kExpCard.MaxExp() == 0 )
					{//잘못된 카드다.
						return false;
					}

					if( 1 != (kExpCard.CurExp() / kExpCard.MaxExp()) )
					{//경험치가 꽉찬거만 팔 수 있다.
						return false;
					}
				}
			}

			if(	pItemDef->IsType(ITEM_TYPE_QUEST)													// 퀘스트 아이템
				|| (pItemDef->GetAbil(AT_ATTRIBUTE) & ICMET_Cant_PlayerTrade)						// 거래불가 옵션
				|| kEhtInfo.IsAttached()															// 착용귀속 옵션
				|| kEhtInfo.IsBinding()																// 자물쇠가 사용된 아이템은
				)
			{// 거래 창에 올릴수 없다
				return false;
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}