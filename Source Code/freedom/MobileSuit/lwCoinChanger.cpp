#include "stdafx.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "variant/PgStore.h"

extern PgStore g_kViewStore;//지금 보고있는 
int const COIN_ITEMNO = 20200188;
int const MAX_COIN_SLOT = 8;

typedef struct stCoinChanger
{
	stCoinChanger()
	{
		Clear();
	}
	void Clear()
	{
		iItemNo = 0;
		iItemCount = 0;
		iResultItemCount = 0;
		iResultItemNo = 0;
		iMakingNo = 0;
	}

	void Set(int iNo, int iCount, int iRNo, int iRCount, int iMNo )
	{
		iItemNo = iNo;
		iItemCount = iCount;
		iResultItemNo = iRNo;
		iResultItemCount = iRCount;
		iMakingNo = iMNo;
	}
	int iItemNo;
	int iItemCount;
	int iResultItemNo;
	int iResultItemCount;
	int iMakingNo;
} SCoinChanger;

typedef std::map<int, SCoinChanger> CoinChangerMap;

CoinChangerMap g_CoinChangerMap;

void lwSelectCoinItem(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pWnd = kWnd.GetSelf();
	if( !pWnd )
	{
		return;
	}
	int const iNo = kWnd.GetCustomData<int>();
	if( !iNo )
	{
		return;
	}
	XUI::CXUI_Wnd* pParent = pWnd->Parent()->Parent();
	
	for(int i = 0; i < MAX_COIN_SLOT; ++i)
	{
		BM::vstring vStr(_T("FRM_ITEM_VAL"));
		vStr += i;
		XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
		if( pSlot )
		{
			XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pSlot->GetControl(_T("CBTN_SELECT_ITEM")));
			if( pCheck && pCheck->Check() )
			{
				pCheck->ClickLock(false);
				pCheck->Check(false);
				break;
			}
		}
	}

	XUI::CXUI_CheckButton* pCheckMe = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd);
	if( pCheckMe )
	{
		pCheckMe->Check(true);
		pCheckMe->ClickLock(true);
	}
	pParent->SetCustomData(&iNo, sizeof(iNo));
}

void CallCoinChanger()
{
	// UI를 콜하고
	XUI::CXUI_Wnd* pCoinChanger = XUIMgr.Call(_T("SFRM_COIN_CHANGER"));
	if( !pCoinChanger )
	{
		return;
	}

	// 슬롯 갯수를 찾아서
	XUI::CXUI_Builder* pBuilder = dynamic_cast<XUI::CXUI_Builder *>(pCoinChanger->GetControl(_T("BLD_ITEM_VAL")));
	if( !pBuilder )
	{
		return;
	}

	int const MAX_SLOT = pBuilder->CountX() * pBuilder->CountY();

	// 갯수만큼 돈다
	for( int i = 0; i < MAX_SLOT; ++i )
	{
		BM::vstring	kStr(_T("FRM_ITEM_VAL"));
		kStr += i;
		// 슬롯을 세팅하자
		XUI::CXUI_Wnd* pSlot = pCoinChanger->GetControl(kStr);
		if( pSlot )
		{
			XUI::CXUI_Wnd* pText = pSlot->GetControl(_T("SFRM_TEXT"));
			XUI::CXUI_Wnd* pIcon = pSlot->GetControl(_T("ICN_ITEM_ICON"));
			XUI::CXUI_Wnd* pBtn = pSlot->GetControl(_T("CBTN_SELECT_ITEM"));
			if( !pText || !pIcon || !pBtn )
			{
				continue;
			}

			PgStore::CONT_GODDS::value_type kItemInfo;
			if( S_OK != g_kViewStore.GetGoodsToIdx(i, kItemInfo) )
			{
				pText->Text(_T(""));
				pIcon->ClearCustomData();
				pBtn->ClearCustomData();
				continue;
			}

			kStr = kItemInfo.iCoin;
			kStr += _T("\n");
			kStr += TTW(550002);
			pText->Text((std::wstring const&)kStr);

			PgBase_Item kItem;
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItemInfo.iItemNo);
			if( !pDef )
			{
				continue;
			}

			kItem.ItemNo(kItemInfo.iItemNo);
			kItem.Count(pDef->GetAbil(AT_DEFAULT_AMOUNT));
			if( kItemInfo.iUseTime )
			{
				kItem.SetUseTime(kItemInfo.bTimeType, kItemInfo.iUseTime);
			}
			lwUIWnd(pIcon).SetCustomDataAsSItem(kItem);

			//kStr = 1;
			//if( !pDef->CanEquip() )
			//{
			//	switch( pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
			//	{
			//	case UICT_ELIXIR:
			//	case UICT_REST:{}break;
			//	default:{ kStr = kItem.Count(); }break;
			//	}
			//}
			//pIcon->Text((std::wstring const&)kStr);

			pBtn->SetCustomData(&kItemInfo.iItemNo, sizeof(kItemInfo.iItemNo));
		}
	}
}

void UnSelectCoinItem(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Builder *pkBuild = dynamic_cast<XUI::CXUI_Builder *>(pkWnd->GetControl(L"BLD_ITEM_VAL"));
	if( !pkBuild )
	{
		return;
	}

	int const iCount = pkBuild->CountX()*pkBuild->CountY();
	for(int i=0; i<iCount; ++i)
	{
		BM::vstring vStr(L"FRM_ITEM_VAL");
		vStr += i;
		XUI::CXUI_Wnd* pkItem = pkWnd->GetControl(vStr);
		if( pkItem )
		{
			XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pkItem->GetControl(L"CBTN_SELECT_ITEM"));
			if( pCheck && pCheck->Check() )
			{
				pCheck->ClickLock(false);
				pCheck->Check(false);
			}
		}
	}
}

void lwSendBuyCoinItem(lwUIWnd kWnd)
{
	if( kWnd.IsNil() )
	{
		return;
	}

	int const iItemNo = kWnd.GetCustomData<int>();
	if( 0 >= iItemNo )
	{
		return;
	}

	PgStore::CONT_GODDS::value_type kItemInfo;
	if( S_OK != g_kViewStore.GetGoods(iItemNo, kItemInfo) )
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	SItemPos kCoinPos;
	if (E_FAIL == pkInv->GetFirstItem(COIN_ITEMNO, kCoinPos))
	{//코인없음
		lwAddWarnDataTT(550005);
		//lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 7, 0); // 판매유도 기능인데 미완성인듯
		return;
	}

	int const iCoinCount = pkInv->GetTotalCount(COIN_ITEMNO);
	if( iCoinCount < kItemInfo.iCoin )
	{
		lwAddWarnDataTT(550005);
		return;
	}
	
	UnSelectCoinItem( kWnd() );

	int const iBuyCount = 1;

	BM::Stream Packet(PT_C_M_REQ_STORE_ITEM_BUY);
	Packet.Push(g_kViewStore.ShopGuid());
	Packet.Push(g_kViewStore.GetType());
	Packet.Push(ESST_COIN_CHANGE);
	Packet.Push(iItemNo);
	Packet.Push(iBuyCount);//무조건 1개만 살 수 있음
	NETWORK_SEND(Packet)
}

int lwGetCoinChangerResNo(lwUIWnd kWnd)
{
	if (kWnd.IsNil())
	{
		return 0;
	}
	ItemMakingDef_Cont pkCont;
	
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (!kItemMakingDefMgr.GetContainer(EMAKING_TYPE_COIN, pkCont))
	{
		return 0;
	}

	XUI::CXUI_Builder *pkBuild = dynamic_cast<XUI::CXUI_Builder *>(kWnd.GetControl("BLD_ITEM_VAL").GetSelf());
	if (!pkBuild)
	{
		return 0;
	}

	int iCount = pkBuild->CountX()*pkBuild->CountY();
	ItemMakingDef_Cont::const_iterator def_it = pkCont.begin();
	g_CoinChangerMap.clear();

	char szName[255] = {0,};
	for (int i = 0; i < iCount; ++i)
	{
		sprintf_s(szName, 254, "FRM_ITEM_VAL%d", i);
		XUI::CXUI_Wnd* pkItem = kWnd.GetControl(szName).GetSelf();
		if (pkItem)
		{
			XUI::CXUI_Wnd* pkText = pkItem->GetControl(_T("SFRM_TEXT"));
			XUI::CXUI_Wnd* pkIcon = pkItem->GetControl(_T("ICN_ITEM_ICON"));
			XUI::CXUI_Wnd* pkBtn = pkItem->GetControl(_T("CBTN_SELECT_ITEM"));
			int iItemNo = 0;
			int iItemCount = 0;
			int iResultItemCount = 0;
			int iResultItemNo = 0;
			wchar_t szCoin[255] = {0,};
			if (pkCont.end() != def_it)
			{
				const ItemMakingDef_Cont::mapped_type &rkType = (*def_it).second;
				iItemNo = rkType.kNeedElements.aElement[0];
				iItemCount = rkType.kNeedCount.aCount[0];		// 필요한 동전 갯수
				iResultItemCount = rkType.akResultItemCount[0].aCount[0];	//결과물의 갯수
				iResultItemNo = rkType.akResultItemElements[0].aElement[0];	// 결과물 넘버
				
				wsprintf(szCoin, _T("%d\n%s"), iItemCount, TTW(550002).c_str());
				SCoinChanger kTemp;
				kTemp.Set(iItemNo, iItemCount, iResultItemNo, iResultItemCount, (*def_it).first);
				g_CoinChangerMap.insert(std::make_pair(iResultItemNo, kTemp));
				++def_it;
			}
			
			if (pkText)
			{
				pkText->Text(std::wstring(szCoin));
			}
			if (pkIcon)
			{
				PgBase_Item kItem;

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(iResultItemNo);
				if( pDef )
				{
					kItem.ItemNo(iResultItemNo);
					if( pDef->CanEquip() )
					{
						kItem.Count(iResultItemCount);
					}
					else
					{
						kItem.Count(pDef->GetAbil(AT_DEFAULT_AMOUNT));
					}
				}

				lwUIWnd(pkIcon).SetCustomDataAsSItem(kItem);
				//pkIcon->SetCustomData(&iResultItemNo, sizeof(iResultItemNo));
				if (iResultItemCount)
				{
					BM::vstring kStr(iResultItemCount);
					pkIcon->Text((std::wstring const&)kStr);
				}
			}
			if (pkBtn)
			{
				pkBtn->SetCustomData(&iResultItemNo, sizeof(iResultItemNo));
			}
		}
	}

	return 0;
}

void lwSendCoinMaking(int iNo)
{
	if( 0>=iNo )
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	SItemPos kPos;
	SItemPos kNextPos;
	int const iCoinNo = 20200188;
	if (E_FAIL == pkInv->GetFirstItem(iCoinNo, kPos))
	{
		lwAddWarnDataTT(550005);
		//코인없음
		return;
	}
	PgBase_Item kFirstItem;

	int iItemCount = 0;
	if(S_OK == pkInv->GetItem(kPos, kFirstItem))
	{
		iItemCount += kFirstItem.Count();
	}

	while (E_FAIL != pkInv->GetNextItem(iCoinNo, kNextPos))
	{
		PgBase_Item kItem;
		if(S_OK == pkInv->GetItem(kPos, kItem))
		{
			iItemCount += kItem.Count();

			PgBase_Item kNextItem;
			
			pkInv->GetItem(kNextPos, kNextItem);

			int const iNextItemCount = kNextItem.Count();
			if(iItemCount<iNextItemCount)
			{
				kPos = kNextPos;
			}
		}
	}

	int iNeedCount = 0;
	SReqItemMaking kReqItemMaking;
	kReqItemMaking.iMakingCount = 1;

	CoinChangerMap::const_iterator map_it = g_CoinChangerMap.find(iNo);
	if (g_CoinChangerMap.end() != map_it)
	{
		iNeedCount = (*map_it).second.iItemCount;
		kReqItemMaking.iMakingNo = (*map_it).second.iMakingNo;
	}

	if (iItemCount < iNeedCount)
	{	//코인부족
		lwAddWarnDataTT(550005);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
	kPacket.Push(1);	// 코인 한번 뽑기
	kReqItemMaking.WriteToPacket(kPacket);
	kPacket.Push(SItemPos());
	kPacket.Push(SItemPos());

	NETWORK_SEND(kPacket)
	
};
