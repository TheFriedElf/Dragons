#include "StdAfx.h"
#include "lwUI.h"
#include "PgUICalculator.h"
#include "lwUIItemLottery.h"
#include "XUI/XUI_Wnd.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgMarket.h"
#include "lwMarket.h"
#include "PgVendor.h"
#include "lwVendor.h"
#include "PgEmporiaMgr.h"
#include "PgCommandMgr.h"
#include "PgChatMgrClient.h"
#include "PgEmporiaAdministrator.h"
#include "PgGuild.h"

using namespace XUI;

extern void Send_PT_C_M_REQ_STORE_ITEM_BUY(BM::GUID const &kShopGuid, int const iItemNo, int const iCount);
extern void Send_PT_C_M_REQ_STORE_ITEM_SELL(const SItemPos &kItemPos, int const iItemNo, int const iCount, bool const bIsStockShop = false);
extern void Send_PT_C_M_REQ_ITEM_DIVIDE(const SItemPos &kItemPos, int const iItemNo, BM::GUID const &kItemGuid, int const iCount);

int g_iCalcValue = 0;//
int g_iCalcMaxValue = 0;//
bool g_bDefaultDigit = true;

void lwCalcValue(int iValue)
{
	if (g_bDefaultDigit)
	{
		g_iCalcValue = 0;
		g_bDefaultDigit = false;
	}

	CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_CALCULATOR"));
	if(pkWnd)
	{
		SCalcInfo kInfo;
		pkWnd->GetCustomData(&kInfo, sizeof(kInfo));
		switch(iValue)
		{
		case -1:
			{
				g_iCalcValue = g_iCalcValue/10;
			}break;
		case -2:
			{
				g_iCalcValue = 0;
			}break;
		case -3:
			{
				switch(kInfo.eCallType)
				{
				case CCT_SHOP_BUY:
					{
						g_iCalcValue = 100;//100이 최대 맞죠?

						CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
						if(pkUnit)
						{
							__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
							g_iCalcValue = __min(100,iMoney/(__int64)kInfo.iBasePrice);
						}
						
					}break;
				case CCT_SHOP_SELL:
				case CCT_STOCK_SHOP_SELL:
					{
						g_iCalcValue = g_iCalcMaxValue;
					}break;
				case CCT_MARKET_BUY:
				case CCT_VENDOR_BUY:
					{
						CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
						if( pkUnit )
						{
							__int64 UserMoney = 0;
							if( !kInfo.cCostType )
							{
								UserMoney = pkUnit->GetAbil64(AT_MONEY);
							}
							else
							{
								UserMoney = pkUnit->GetAbil64(AT_CASH);
							}

							g_iCalcValue = UserMoney / kInfo.iBasePrice;
							g_iCalcValue = __min(g_iCalcValue, g_iCalcMaxValue);
						}
					}break;
				case CCT_EMWEEK:
					{

					}break;
				case CCT_EMPORIA_WEEK_SELECT:
					{
						__int64 iTotalExp = g_kGuildMgr.GetExp();
						g_iCalcValue = iTotalExp / kInfo.iBasePrice;
						g_iCalcValue = __min(g_iCalcValue, g_iCalcMaxValue);
					}break;
				}
			}break;
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			{
				switch(kInfo.eCallType)
				{
				case CCT_MARKET_BUY:
				case CCT_VENDOR_BUY:
					{
						g_iCalcValue = (g_iCalcValue*10) +iValue;
						g_iCalcValue = __min(g_iCalcValue, g_iCalcMaxValue);

						__int64 const iTotalPrice = g_iCalcValue * kInfo.iBasePrice;
					
						CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
						if( pkUnit )
						{
							__int64 UserMoney = 0;
							if( !kInfo.cCostType )
							{
								UserMoney = pkUnit->GetAbil64(AT_MONEY);
							}
							else
							{
								UserMoney = pkUnit->GetAbil64(AT_CASH);
							}

							if( iTotalPrice > UserMoney )
							{
								g_iCalcValue = UserMoney / kInfo.iBasePrice;
							}
						}
					}break;
				case CCT_EMPORIA_WEEK_SELECT:
					{
						g_iCalcValue = (g_iCalcValue*10) +iValue;
						g_iCalcValue = __min(g_iCalcValue, g_iCalcMaxValue);

						__int64 const iNeedExp = g_iCalcValue * kInfo.iBasePrice;
						__int64 iTotalExp = g_kGuildMgr.GetExp();
						if( iNeedExp > iTotalExp )
						{
							g_iCalcValue = iTotalExp / kInfo.iBasePrice;
						}
					}break;
				default:
					{
						g_iCalcValue = (g_iCalcValue*10) +iValue;
						g_iCalcValue = __min(g_iCalcValue, g_iCalcMaxValue);//100이 최대.//최대값을 정해야 되는데.
					}break;
				}
			}break;
		default:
			{
				g_iCalcValue = iValue;
			}break;
		}
	}
}

void lwSetCalcDigit()
{//콜 타이밍에 하면 얘기 등록 되어있질 않으니. 실행이 되나..
	CXUI_Wnd* pkRoot = XUIMgr.Get(_T("SFRM_CALCULATOR"));
	if(!pkRoot)
	{
		return;
	}
	CXUI_Wnd* pkSubRoot = pkRoot->GetControl(_T("SFRM_COLOR"));

	CXUI_Wnd *pkSDW = pkSubRoot->GetControl(_T("SFRM_SDW")); 
	if(pkSDW)
	{
		SCalcInfo kInfo;
		pkRoot->GetCustomData(&kInfo, sizeof(kInfo));

		CXUI_Wnd* pkDigit= pkSDW->GetControl(_T("SFRM_DIGIT"));
		pkDigit->Text((BM::vstring)g_iCalcValue);

		CXUI_Wnd* pkMoneyParent = pkSDW->GetControl(_T("SFRM_MONEY"));

		CXUI_Wnd* pkChild1 = pkMoneyParent->GetControl(_T("FRM_COIN1"));
		CXUI_Wnd* pkChild2 = pkMoneyParent->GetControl(_T("FRM_COIN2"));
		CXUI_Wnd* pkChild3 = pkMoneyParent->GetControl(_T("FRM_COIN3"));
		CXUI_Wnd* pkChild4 = pkMoneyParent->GetControl(_T("FRM_COIN4"));

		__int64 const iMoney = kInfo.iBasePrice * g_iCalcValue;
		if( !kInfo.cCostType )
		{
			pkChild1->Text( (BM::vstring)(int)(iMoney/10000));
			pkChild2->Text( (BM::vstring)(int)((int)(iMoney/100)%100));
			pkChild3->Text( (BM::vstring)(int)((int)(iMoney%100)));
		}
		else
		{
			pkChild4->Text( (BM::vstring)(int)(iMoney));
		}
	}
}

void CallCalculator(const SCalcInfo &kInfo)
{
	ECalcInfoType const &eCallType = kInfo.eCallType;
	switch ( eCallType )
	{
	case CCT_EMWEEK:
	case CCT_EMPORIA_WEEK_SELECT:
		{
		}break;
	default:
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kInfo.iItemNo);
			if(		!pItemDef 
				||	!pItemDef->IsAmountItem()	)
			{
				lwAddWarnDataTT(400195);
				return;
			}
		}break;
	}

	int const iCount = kInfo.iMaxValue;
	if( iCount == 1 )
	{
		switch(eCallType)
		{
		case CCT_SHOP_SELL:
		case CCT_STOCK_SHOP_SELL:
			{	
				bool const bIsStockShop = ((CCT_STOCK_SHOP_SELL == eCallType)?true:false);
				Send_PT_C_M_REQ_STORE_ITEM_SELL(kInfo.kItemPos, kInfo.iItemNo, iCount, bIsStockShop);
			}return;
		case CCT_MARKET_BUY:
			{
				lwMarket::RequestBuySelectItem(kInfo.kGuid1, kInfo.kGuid, iCount);
			}return;
		case CCT_VENDOR_BUY:
			{
				lwVendor::RequestBuySelectItem(kInfo.kGuid1, kInfo.kGuid, iCount);
			}return;
		}
	}

	CXUI_Wnd* pkWnd = XUIMgr.Call(_T("SFRM_CALCULATOR"), true);
	if(pkWnd)
	{
		g_iCalcValue = 0;
		pkWnd->SetCustomData(&kInfo, sizeof(kInfo));
		CXUI_Wnd* pkBtnWnd = pkWnd->GetControl(_T("BTN_BUY"));
		CXUI_Wnd* pkColorWnd = pkWnd->GetControl(_T("SFRM_COLOR"));
		CXUI_Wnd* pkSdwWnd = pkColorWnd->GetControl(_T("SFRM_SDW"));
		CXUI_Wnd* pkDigit= pkSdwWnd->GetControl(_T("SFRM_DIGIT"));
		CXUI_Wnd* pkArrowWnd = pkSdwWnd->GetControl(_T("FRM_ARROW8"));
		CXUI_Wnd* pkMoneyWnd = pkSdwWnd->GetControl(_T("SFRM_MONEY"));
		CXUI_Wnd* pkChild1 = pkMoneyWnd->GetControl(_T("FRM_COIN1"));
		CXUI_Wnd* pkChild2 = pkMoneyWnd->GetControl(_T("FRM_COIN2"));
		CXUI_Wnd* pkChild3 = pkMoneyWnd->GetControl(_T("FRM_COIN3"));
		CXUI_Wnd* pkChild4 = pkMoneyWnd->GetControl(_T("FRM_COIN4"));
		pkDigit->Text((BM::vstring)g_iCalcValue);
		pkChild1->Visible(false);
		pkChild2->Visible(false);
		pkChild3->Visible(false);
		pkChild4->Visible(false);

		if( kInfo.cCostType )
		{
			pkChild4->Visible(true);
		}
		else
		{
			pkChild1->Visible(true);
			pkChild2->Visible(true);
			pkChild3->Visible(true);
		}

		if(pkMoneyWnd)
		{
			pkMoneyWnd->Visible(true);
		}

		switch(kInfo.eCallType)
		{
		case CCT_SHOP_BUY:
			{
				//pkBtnWnd->Text(TTW(400172));
				pkArrowWnd->Text(TTW(400174));
				g_iCalcMaxValue = iCount;
				g_bDefaultDigit = true;
			}break;
		case CCT_SHOP_SELL:
		case CCT_STOCK_SHOP_SELL:
			{
				pkBtnWnd->Text(TTW(400173));
				pkArrowWnd->Text(TTW(400189));
				g_iCalcMaxValue = iCount;
				g_iCalcValue = g_iCalcMaxValue;
				pkDigit->Text((BM::vstring)g_iCalcValue);
				g_bDefaultDigit = true;
				lwSetCalcDigit();
			}break;
		case CCT_LOTTERY:
			{
				if (pkMoneyWnd)
				{
					pkMoneyWnd->Visible(false);
				}
				std::wstring wstrBtn = TTW(1655) + TTW(2200);
				pkBtnWnd->Text(wstrBtn);
				pkArrowWnd->Text(TTW(1654));
				g_iCalcMaxValue = iCount;
				g_bDefaultDigit = true;
				lwSetCalcDigit();
			}break;
		case CCT_ITEM_DIVIDE:
			{
				if (pkMoneyWnd)
				{
					pkMoneyWnd->Visible(false);
				}
				std::wstring wstrBtn = TTW(1655) + TTW(2200);
				pkBtnWnd->Text(wstrBtn);
				pkArrowWnd->Text(TTW(1659));
				g_iCalcMaxValue = iCount;
				g_bDefaultDigit = true;
				lwSetCalcDigit();
			}break;
		case CCT_MARKET_BUY:
		case CCT_VENDOR_BUY:
			{
				//pkBtnWnd->Text(TTW(400172));
				pkArrowWnd->Text(TTW(400174));
				g_iCalcMaxValue = iCount;
				lwSetCalcDigit();
			}break;
		case CCT_EMWEEK:
		case CCT_EMPORIA_WEEK_SELECT:
			{				
				std::wstring wstrBtn = TTW(1655) + TTW(2200);
				pkBtnWnd->Text(wstrBtn);
				pkArrowWnd->Text(TTW(72014));
				g_iCalcMaxValue = iCount;
				g_bDefaultDigit = false;
				lwSetCalcDigit();

				if(pkMoneyWnd)
				{
					pkMoneyWnd->Visible(false);
				}
			}break;
		}
	}
}

void lwDoCalcuratorAction()
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_CALCULATOR"));
	if(!pkWnd)
	{
		return;
	}

	SCalcInfo kInfo;
	pkWnd->GetCustomData(&kInfo, sizeof(kInfo));

	ECalcInfoType const &eCallType = kInfo.eCallType;

	// 아이템이 아닌경우는 여기에 정의한다.
	switch(eCallType)
	{
// 	case CCT_EMWEEK:
// 		{
// 			g_kEmporiaMgr.KeepFunction( kInfo.iIndex, g_iCalcValue );
// 			return;
// 		}break;
	case CCT_EMPORIA_WEEK_SELECT:
		{
			lwPgEmporiaAdministrator::SendEmporiaAddonBuildingOrder( static_cast<short>(kInfo.iIndex), kInfo.kGuid, g_iCalcValue);
			return;
		}break;
	default:
		{

		}break;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kInfo.iItemNo);
	if( !pItemDef )
	{
		return;	
	}

	// 아이템인 경우는 여기에 정의한다.
	_PgOutputDebugString("Sell, GUID : %s Item No : %d CalcValue : %d \n", kInfo.kGuid.str().c_str(), kInfo.iItemNo, g_iCalcValue);
	switch( eCallType )
	{
	case CCT_SHOP_BUY:
		{
			Send_PT_C_M_REQ_STORE_ITEM_BUY(kInfo.kGuid, kInfo.iItemNo, g_iCalcValue);
		}break;
	case CCT_ITEM_DIVIDE:
		{
			Send_PT_C_M_REQ_ITEM_DIVIDE(kInfo.kItemPos, kInfo.iItemNo, kInfo.kGuid, g_iCalcValue);
		}break;
	case CCT_MARKET_BUY:
		{
			lwMarket::RequestBuySelectItem(kInfo.kGuid1, kInfo.kGuid, g_iCalcValue);
		}break;
	case CCT_VENDOR_BUY:
		{
			lwVendor::RequestBuySelectItem(kInfo.kGuid1, kInfo.kGuid, g_iCalcValue);
		}break;
	case CCT_SHOP_SELL:
	case CCT_STOCK_SHOP_SELL:
		{
			if(	CCT_STOCK_SHOP_SELL != eCallType//스톡샵은 판매가를 상점이 정한다.
			&&	0>=pItemDef->SellPrice())//스톡샵이 아닐때 판매가가 없으면 팔 수 없다.
			{
				lwAddWarnDataTT(400195);
				return;
			}

			bool const bIsStockShop = ((CCT_STOCK_SHOP_SELL == eCallType)?true:false);

			Send_PT_C_M_REQ_STORE_ITEM_SELL(kInfo.kItemPos, kInfo.iItemNo, g_iCalcValue, bIsStockShop);
		}break;
	case CCT_LOTTERY:
		{
			g_kItemLottery.SetSrcItem(kInfo.kItemPos, kInfo.iIndex, g_iCalcValue);
		}break;

	}
}

void CallSMS(const tagSMSInfo &kInfo)//문자 보내기.
{
	//해당 아이템이 SMS용 이면.
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kInfo.iItemNo);
	if(pItemDef 
	&& pItemDef->IsAmountItem())
	{
		std::wstring kFormName;
		std::wstring kFocusEdit;
		bool bModal = true;
		switch(kInfo.iValue2)
		{
		case CARD_LOVE:
			{
				kFormName = _T("FRM_EVT_SMS");
				kFocusEdit = _T("EDT_NAME");
			}break;
		case CARD_FIRE:
			{// 모달로 띄우지 않음
				kFormName = _T("SFRM_FIRE_OF_LOVE_SMS");
				kFocusEdit = _T("EDIT_BOY_NAME");
				bModal = false;
			}break;
		default:
			{
				kFormName = _T("SFRM_SMS");
				kFocusEdit = _T("EDT_MEMO");
				// 확성기에 아이템 링크를 위해서 모달로 하지 않음.
				bModal = false;
			}break;
		}

		CXUI_Wnd* pkWnd = XUIMgr.Call(kFormName, bModal);
		if( !pkWnd )
		{
			return;
		}
		pkWnd->SetCustomData(&kInfo, sizeof(kInfo));
		CXUI_Edit* pEdit = dynamic_cast<CXUI_Edit*>(pkWnd->GetControl(kFocusEdit));
		if( !pEdit )
		{
			return;
		}
		
		CXUI_Wnd* pkMega = pkWnd->GetControl(std::wstring(_T("IMG_MEGA")));
		if (pkMega)
		{
			SUVInfo kUVInfo= pkMega->UVInfo();
			kUVInfo.Index = kInfo.iValue % 2 + 1;
			pkMega->UVInfo(kUVInfo);
			pkMega->SetInvalidate();
		}
		XUIMgr.ClearEditFocus();
		pEdit->SetEditFocus(true);

		if(kFormName == _T("SFRM_SMS"))
		{
			int const iNo = (SMS_IN_REALM==kInfo.iValue ? 404957 : 404950);
			pEdit->PreviewText(TTW(iNo));
		}
	}
}

void lwDoSMSAction(lwUIWnd Self)
{
	CXUI_Wnd* pkWnd = Self.GetSelf();
	if(pkWnd)
	{
		SSMSInfo kInfo;
		XUI::CXUI_Style_String kStyleString;
		pkWnd->GetCustomData(&kInfo, sizeof(kInfo));
		
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(kInfo.iItemNo);
		CXUI_Edit* pkEdtWnd = dynamic_cast<CXUI_Edit*>(pkWnd->GetControl(_T("EDT_MEMO")));
		if(	pItemDef && pkEdtWnd )
		{			
			pkEdtWnd->GetEditText_TextBlockApplied(kStyleString);
			std::wstring strMemo = kStyleString.GetOriginalString();					
			std::wstring TargetName;
			std::wstring MailTitle;
			if( CARD_LOVE == kInfo.iValue2 )
			{
				CXUI_Edit* pkEdtName = dynamic_cast<CXUI_Edit*>(pkWnd->GetControl(_T("EDT_NAME")));
				if( pkEdtName )
				{
					TargetName = pkEdtName->EditText();
				}

				bool IsSendOK = true;
				int iErrTTWID = 0;
				if( TargetName.empty() )
				{
					IsSendOK = false;
					iErrTTWID = 600013;
				}

				if( TargetName.size() > (MAX_CHARACTERNAME_LEN / sizeof(std::wstring::value_type) ) )
				{
					IsSendOK = false;
					iErrTTWID = 50515;
				}

				if( g_kClientFS.Filter(TargetName, false, FST_NICKNAME) 
					|| !g_kUnicodeFilter.IsCorrect(UFFC_CHARACTER_NAME, TargetName) )
				{
					IsSendOK = false;
					iErrTTWID = 700144;
				}

				if( !IsSendOK )
				{
					pkEdtName->EditText(L"");
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(iErrTTWID, kChatLog, true);
					return;
				}

				PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
				if( pPlayer )
				{
					MailTitle = pPlayer->Name();
				}
			}

			bool const UseChatTag = g_kChatMgrClient.CheckChatTag(strMemo);
			if( UseChatTag )
			{
				bool IsItemExtraData = 
					g_kChatMgrClient.IsItemLinkContainData(kStyleString.GetExtraDataPackInfo());
				if(false==IsItemExtraData)
				{
					SChatLog kChatLog((EChatType)CT_EVENT);
					g_kChatMgrClient.AddMessage(799394, kChatLog, true, 11);
					return;
				}
			}

			if(kStyleString.Length())
			{
				g_kClientFS.Filter(strMemo, true);
				if((kStyleString.Length() > 40))
				{
					strMemo.resize(40);//40자로 제한.
				}
				BM::Stream kPacket(PT_C_M_REQ_ITEM_SMS);
				kPacket.Push(true);
				kPacket.Push(kInfo.iItemNo);
				kPacket.Push(kInfo.kItemGuid);
				kPacket.Push(kInfo.kItemPos);
				kPacket.Push(strMemo);
				kPacket.Push(TargetName);
				kPacket.Push(MailTitle);
				XUI::PgExtraDataPackInfo const &kExtraDataPackInfo = kStyleString.GetExtraDataPackInfo();
				kExtraDataPackInfo.PushToPacket(kPacket);

				NETWORK_SEND(kPacket)

				pkWnd->Close();
			}
		}
	}
}

void lwSendFireOfLoveSMS(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* const pkWnd = kWnd.GetSelf();
	if(!pkWnd)
	{
		return;
	}
	SSMSInfo kInfo;
	pkWnd->GetCustomData(&kInfo, sizeof(kInfo));		
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* const pItemDef = kItemDefMgr.GetDef(kInfo.iItemNo);
	if(!pItemDef)
	{
		return;
	}
	if(CARD_FIRE != kInfo.iValue2)
	{
		return;
	}

	XUI::CXUI_Edit* const pkEdt_BoyName = static_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("EDIT_BOY_NAME")));	
	if(!pkEdt_BoyName)
	{
		return;
	}
	XUI::CXUI_Edit* const pkEdt_GirlName = static_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("EDIT_GIRL_NAME")));	
	if(!pkEdt_GirlName)
	{
		return;
	}
	std::wstring kBoyName = pkEdt_BoyName->EditText();
	std::wstring kGirlName = pkEdt_GirlName->EditText();
	if(kBoyName.empty()	|| kGirlName.empty())
	{// 남녀 이름이 비어 있으면 안되고
		lwAddWarnDataTT(45);
		return;
	}
	if(g_kClientFS.Filter(kBoyName, false) 
		|| g_kClientFS.Filter(kGirlName, false) 		
		)
	{// 욕설 필터링 한후
		lwAddWarnDataTT(700144);
		return;
	}
	if(!g_kUnicodeFilter.IsCorrect(UFFC_CHAT, kBoyName)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_CHAT, kGirlName)
		)
	{// 유니코드 필터링 하고
		lwAddWarnDataTT(600037);
		return;
	}
	// 패킷을 보낸다
	BM::Stream kPacket(PT_C_M_REQ_ITEM_SMS);
	kPacket.Push(false);
	kPacket.Push(kInfo.iItemNo);
	kPacket.Push(kInfo.kItemGuid);
	kPacket.Push(kInfo.kItemPos);
	kPacket.Push(kBoyName);
	kPacket.Push(kGirlName);
	kPacket.Push(std::wstring(L""));
	NETWORK_SEND(kPacket)
	pkWnd->Close();
}