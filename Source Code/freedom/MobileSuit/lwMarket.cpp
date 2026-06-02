#include "StdAfx.h"
#include "lwGuid.h"
#include "ServerLib.h"
#include "lwWString.h"
#include "PgPilotMan.h"
#include "lwUI.h"
#include "lwMarket.h"
#include "PgMarket.h"
#include "PgNetwork.h"
#include "Lohengrin/GameTime.h"
#include "PgUIScene.h"
#include "PgUICalculator.h"
#include "PgChatMgrClient.h"
#include "PgWorld.h"
#include "PgCommandMgr.h"
#include "lwUIQuest.h"
#include "PgPilot.h"
#include "lwCashItem.h"
#include "PgMobileSuit.h"

extern DWORD SetGradeColor(E_ITEM_GRADE const eItemLv, bool bIsCash, std::wstring& wstrName);
extern bool MakeItemName(int const iItemNo, const SEnchantInfo &rkEnchant, std::wstring &wstrName);

int const MAX_CHANG_MONEY_VALUE = 30000;
int const GOLD_MONEY_VALUE = 10000;
void lwMarket::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallOpenMarket", lwCallOpenMarket);
	
	//------------------------------------------------------------------
	//	관리용
	//------------------------------------------------------------------
	def(pkState, "DrawShopHistoryPoint", lwDrawShopHistoryPoint);
	def(pkState, "CallAddRegUI", lwCallAddRegUI);
	def(pkState, "AddRegItem", lwAddRegItem);
	def(pkState, "CloseAddRegUI", lwCloseAddRegUI);
	def(pkState, "DelRegListItem", lwDelRegListItem);
	def(pkState, "ClearShopRegUI", lwClearShopRegUI);
	def(pkState, "RequestShopAction", lwRequestShopAction);
	def(pkState, "RequestMinimumCost", lwRequestMinimumCost);
	def(pkState, "RequestMyShopInfo", lwRequestMyShopInfo);
	def(pkState, "DrawShopOpenTime", lwDrawShopOpenTime);
	def(pkState, "RequestShopInfoChange", lwRequestShopInfoChange);
	def(pkState, "RequestShopStateChange", lwRequestShopStateChange);
	def(pkState, "RequestShopUpdateOpenTime", lwRequestShopUpdateOpenTime);
	def(pkState, "RequestShopUpdateHP", lwRequestShopUpdateHP);
	def(pkState, "UpdateRegList", lwUpdateRegList);
	def(pkState, "IsChangeShop", lwIsChangeShop);
	def(pkState, "CloseMyShop", lwCloseMyShop);	

	//------------------------------------------------------------------
	//	재고용
	//------------------------------------------------------------------
	def(pkState, "RequestSoldReceive", lwRequestSoldReceive);
	def(pkState, "PrevSoldPageInfo", lwPrevSoldPageInfo);
	def(pkState, "NextSoldPageInfo", lwNextSoldPageInfo);
	def(pkState, "DrawSoldPage", lwDrawSoldPage);
	def(pkState, "DrawSoldPageInfo", lwDrawSoldPageInfo);

	//------------------------------------------------------------------
	//	공용
	//------------------------------------------------------------------
	def(pkState, "DrawIconImage", lwDrawIconImage);
	def(pkState, "DrawMarketItemTooltip", lwDrawMarketItemTooltip);
	def(pkState, "TabChangeState", lwTabChangeState);
	def(pkState, "MarketTabChange", lwTabBtnChange);
	def(pkState, "MarketAction", lwMarketAction);
	def(pkState, "ClearOpenMarket", lwClearOpenMarket);
	def(pkState, "BuildItemCheckUpdate", lwBuildItemCheckUpdate);
	def(pkState, "ListItemCheckUpdate", lwListItemCheckUpdate);
	def(pkState, "InOpenMarket", lwInOpenMarket);
	def(pkState, "OutOpenMarket", lwOutOpenMarket);

	def(pkState, "SetCurrentMarketGrade", lwSetCurrentMarketGrade);
	def(pkState, "FixGradeRadioBtn", lwFixGradeRadioBtn);
	def(pkState, "MarketOpenConfirm", lwMarketOpenConfirm);
	def(pkState, "MarketOpenCancel", lwMarketOpenCancel);
	def(pkState, "MarKetGradeRadioBtnInit", lwMarKetGradeRadioBtnInit);

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market Shop 분리 2009. 4. 20
	//---------------------------------------------------------------------------------------------------------------------------------
	def(pkState, "RequestShopList", lwRequestShopList);
	def(pkState, "RequestEnterShop", lwRequestEnterShop);
	def(pkState, "ClearSearchInfoUI", lwClearSearchInfoUI);
	def(pkState, "SelectItemType", lwSelectItemType);
	def(pkState, "DrawShopListUpdate", lwDrawShopListUpdate);
	def(pkState, "SearchBtnState", lwSearchBtnState);

	def(pkState, "PrevShopPageInfo", lwPrevShopPageInfo);
	def(pkState, "NextShopPageInfo", lwNextShopPageInfo);
	def(pkState, "DrawShopPageInfo", lwDrawShopPageInfo);

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market Search 분리 2009. 4. 7
	//---------------------------------------------------------------------------------------------------------------------------------
	def(pkState, "SearchItemList", lwSearchItemList);
	def(pkState, "OnOverSearchListItem", lwOnOverSearchListItem);
	def(pkState, "OnCallSearchListPopUpMenu", lwOnCallSearchListPopUpMenu);
	def(pkState, "OnCallItemBuyUI", lwOnCallItemBuyUI);
	def(pkState, "SelectBuyOK", lwSelectBuyOK);
	def(pkState, "SelectShopInOK", lwSelectShopInOK);
	def(pkState, "PrevSearchPageInfo", lwPrevSearchPageInfo);
	def(pkState, "NextSearchPageInfo", lwNextSearchPageInfo);
	def(pkState, "DrawSearchPageInfo", lwDrawSearchPageInfo);

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market RegItem 분리 2009. 7. 19
	//---------------------------------------------------------------------------------------------------------------------------------
	def(pkState, "OnClickChangeRegTabType", lwOnClickChangeRegTabType);
	def(pkState, "OnClickMoneyChangeRate", lwOnClickMoneyChangeRate);
	def(pkState, "OnTickCheckChangeMaxValue", lwOnTickCheckChangeMaxValue);

	//---------------------------------------------------------------------------------------------------------------------------------
	//	Market Common 분리 2009. 4. 7
	//---------------------------------------------------------------------------------------------------------------------------------
	def(pkState, "OnClickListItem", lwOnClickListItem);
	
	def(pkState, "DrawMarketItemIcon", lwDrawMarketItemIcon);
	def(pkState, "OnOverMarketItemToolTip", lwOnOverMarketItemToolTip);
}

void lwMarket::lwCloseMyShop()
{
	if( g_kMarketMgr.ShopState() == MS_OPEN )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403114, true);
		return;
	}

	if( g_kMarketMgr.MyShopInfo().ItemCont().size()
	||  g_kMarketMgr.SoldInfo().Size() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403202, true);
		return;
	}

	BM::Stream kPacket;
	SPT_C_M_REQ_MARKET_CLOSE kData;
	kData.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void lwMarket::lwInOpenMarket()
{
	BM::Stream	kPacket(PT_C_M_UM_REQ_MARKET_ENTER);
	NETWORK_SEND(kPacket)
}

void lwMarket::lwOutOpenMarket()
{
	BM::Stream	kPacket(PT_C_M_UM_REQ_MARKET_EXIT);
	NETWORK_SEND(kPacket);

	g_kMKTSearchMgr.Clear();
	
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, false));
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, true));
	XUI::CXUI_Wnd* pLeg = XUIMgr.Get(L"SFRM_ARTICLE_REG_WIN");
	if( !pLeg ){ return; }
	lwCloseAddRegUI(pLeg);
}

bool lwMarket::lwIsChangeShop(int const iType)
{
	if( g_kMarketMgr.ShopState() == MS_OPEN )
	{
		return false;
	}

	if( iType < g_kMarketMgr.MyShopInfo().ShopInfo().iGrade )
	{
		int iItemCount = g_kMarketMgr.MyShopInfo().ItemCont().size() + g_kMarketMgr.MyShopInfo().TempRegList().size();

		if( iItemCount > g_kMarketMgr.GetMaxItemSlot((E_MARKET_GRADE)iType) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403203, true);
			return false;
		}
	}

	return true;
}

void lwMarket::lwCallOpenMarket()
{
	if( !g_pkWorld )
	{
		return;
	}
	if( false == lua_tinker::call<bool>("CheckActionForEnterCashShop") )
	{
		return;
	}
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		if ( BM::GUID::NullData() != pkPlayer->VendorGuid() )
		{// 입장할 수 없는 곳
			lwAddWarnDataTT(799452);
			return;
		}
	}

	int iUseCallMarket = 0;

	PgPilot const * pkPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPilot)
	{
		iUseCallMarket = pkPilot->GetAbil(AT_CALL_MARKET);
	}

	if(iUseCallMarket || g_pkWorld->IsHaveAttr(GATTR_VILLAGE) )
	{
		if(lwGetUIWnd("FRM_AUCTION_MAIN").IsNil())
		{
			lwInOpenMarket();
		}
		else
		{
			lwCloseUI("FRM_AUCTION_MAIN");
		}
	}
	else
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403080, true);
	}
}

void lwMarket::lwDrawShopHistoryPoint(lwUIWnd UISelf)
{
	BM::vstring	vStr(g_kMarketMgr.MyShopInfo().ShopInfo().iHP);
	UISelf.SetStaticTextW((std::wstring const&)vStr);
}

void lwMarket::lwUpdateRegList(lwUIWnd UISelf)
{
	if( g_kMarketMgr.ShopState() == MS_CLOSE )
	{
		if( !UpdateRegList() )
		{
			return;
		}
	}
}

void lwMarket::lwRequestMyShopInfo()
{
	// 여기서 상점 정보를 요청한다
	SPT_C_M_UM_REQ_MY_MARKET_QUERY Data;

	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

E_MARKET_GRADE lwMarket::GetShopGrade()
{
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return MG_NORMAL;
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab.IsNil() )
	{
		return MG_NORMAL;
	}

	lwUIWnd kTemp = UITab.GetControl("CBTN_SHOP_STYLE2");
	if( kTemp.IsNil() )
	{
		return MG_NORMAL;
	}

	if( !kTemp.GetCheckState() )
	{
		kTemp = UITab.GetControl("CBTN_SHOP_STYLE3");
		if( kTemp.IsNil() )
		{
			return MG_NORMAL;
		}

		if( kTemp.GetCheckState() )
		{
			return MG_HISTORY;
		}
	}
	else
	{
		return MG_GOOD;
	}

	return MG_NORMAL;
}

bool lwMarket::UpdateRegList()
{
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return false;
	}

	lwUIWnd UIStockBtn = UIAuction.GetControl("BTN_SALE_RESULT");
	if( UIStockBtn.IsNil() )
	{
		return false;
	}

	if( !g_kMarketMgr.SoldInfo().Size() )
	{
		UIStockBtn.Disable(true);
	}
	else
	{
		UIStockBtn.Disable(false);
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab.IsNil() )
	{
		return false;
	}

	{// Disable 되지 않은 좋은, 최고급 상점 체크 버튼의 툴팁이 보이면서 체크버튼 조정한다.
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			lwRevertMarketGrade();
			return false;
		}
		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven)
		{
			lwRevertMarketGrade();
			return false;
		}
		lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
		lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
		lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");
		SItemPos	ShopContractPos;
		
		if(kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil())
		{
			lwRevertMarketGrade();
			return false;
		}

		SMyShopInfo const& kMyShopInfo = g_kMarketMgr.MyShopInfo();
		SMyShopData const& kMyShopData = kMyShopInfo.ShopInfo();
		int const& iOpenedGrade = kMyShopData.iGrade;

		bool const bPremiumService = pkPlayer->GetPremium().IsUserService(true) && (NULL != pkPlayer->GetPremium().GetType<S_PST_OpenmarketState>());
		if(true == kShop2.GetCheckState())
		{// 좋은 상점의 체크 버튼이 클릭 되었으나
			if(iOpenedGrade != MG_GOOD
				&& (false==bPremiumService && E_FAIL == pkInven->GetFirstItem(99600010, ShopContractPos)))
			{// 상점을 열기 위한 아이템이 없다면, 이전에 체크되었던 버튼으로 되돌린다
				lwRevertMarketGrade();
				return false;
			}
		}
		else if(true == kShop3.GetCheckState())
		{// 최고급 상점의 체크 버튼이 클릭 되었으나
			if(iOpenedGrade != MG_HISTORY
				&& (false==bPremiumService && E_FAIL == pkInven->GetFirstItem(99600020, ShopContractPos)))
			{// 상점을 열기 위한 아이템이 없다면, 이전에 체크되었던 버튼으로 되돌린다
				lwRevertMarketGrade();
				return false;
			}
		}
	}
	
	lwUIWnd UIShopBtn = UITab.GetControl("BTN_SHOP_OPEN");
	if( UIShopBtn.IsNil() )
	{
		if( g_kMarketMgr.ShopState() == MS_OPEN )
		{
			UIShopBtn.SetStaticText(MB(TTW(403068)));	
		}
		else
		{
			UIShopBtn.SetStaticText(MB(TTW(403023)));	
		}
	}

	lwUIWnd UIList = UITab.GetControl("LIST_AUCTION_REG_ITEM");
	if( UIList.IsNil() )
	{
		return false;
	}

	UIList.ClearAllListItem();

	E_MARKET_GRADE ShopType = (E_MARKET_GRADE)g_kMarketMgr.MyShopInfo().ShopInfo().iGrade;
	if( g_kMarketMgr.ShopState() == MS_CLOSE )
	{
		//if( ShopType != GetShopGrade() )
		//{
		//	ShopType = GetShopGrade();
		//}
		if( ShopType != g_kMarketMgr.GetCurrentMarketGrade() )
		{
			ShopType = g_kMarketMgr.GetCurrentMarketGrade();
		}		
	}

	kBaseItemContainer const& kCont = g_kMarketMgr.MyShopInfo().ItemCont();

	kBaseItemContainer::const_iterator c_iter = kCont.begin();
	for(int i = 0; i < g_kMarketMgr.GetMaxItemSlot(ShopType); ++i)
	{
		if( c_iter != kCont.end() )
		{
			AddItemListItem(UIList, c_iter->second);
			++c_iter;
		}
		else
		{
			lwUIListItem UIItem = UIList.AddNewListItemChar("");
			if( UIItem.IsNil() )
			{
				return false;
			}			
		}
	}

	lwUpdataRegAction();
	return true;
}

void lwMarket::lwAddRegItem(lwUIWnd UIParent)
{
	SARTICLEINFO kRegInfo;
	SMarketItemInfoBase	kItemInfo;
	
	XUI::CXUI_Wnd* pParent = UIParent.GetSelf();
	if( !pParent )
	{
		return;
	}

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
			if( !AddRegistSellArticleInItem(pParent, kRegInfo, kItemInfo) )
			{
				return;
			}
		}break;
	default :
		{
			for( int i = 0; i < 3; ++i )
			{
				BM::vstring vStr(L"CBTN_REG_TYPE");
				vStr += i;

				XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
				if( !pCheck )
				{
					continue;
				}

				if( !pCheck->Check() )
				{
					continue;
				}

				int const iBuildIndex = pCheck->BuildIndex();

				switch( iBuildIndex )
				{
				case 0:
					{
						XUI::CXUI_Wnd* pRealParent = pParent->GetControl(L"FRM_REG_TYPE0");
						if( !AddRegistSellArticleInItem(pRealParent, kRegInfo, kItemInfo) )
						{
							return;
						}
					}break;
				default:
					{
						XUI::CXUI_Wnd* pRealParent = pParent->GetControl(L"FRM_REG_TYPE1");
						if( !AddRegistSellArticleInMoney(pRealParent, iBuildIndex, kRegInfo, kItemInfo) )
						{
							return;
						}
					}break;
				}
				break;
			}
		}break;
	}

	if( ReturnResult((EMarketErrType)g_kMarketMgr.RequestAddRegItem(kRegInfo, kItemInfo)) )
	{
		lwUIWnd UIBtn = UIParent.GetControl("BTN_OK");
		if( !UIBtn.IsNil() && g_kMarketMgr.IsSendOK() )
		{
			UIBtn.Disable(g_kMarketMgr.IsSendOK());
		}
		else
		{
			lwCloseAddRegUI(UIParent);
		}
	}
}

bool lwMarket::AddRegistSellArticleInItem(XUI::CXUI_Wnd* pParent, SARTICLEINFO& kArticleInfo, SMarketItemInfoBase& kViewItemInfo)
{
	if( !pParent )
	{ 
		return false; 
	}

	kArticleInfo.ArticleType( UMAT_ITEM );

	SMarketIconInfo	IconInfo;
	
	//step 1. 아이템 정보 및 수량 얻기
	XUI::CXUI_Icon*	pIcon = dynamic_cast<XUI::CXUI_Icon*>(pParent->GetControl(L"ICON_ITEM"));
	if( pIcon )
	{
		pIcon->GetCustomData(&IconInfo, sizeof(SMarketIconInfo));
		kArticleInfo.Pos(SItemPos(IconInfo.cItemGroup, IconInfo.cInvPos));
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	PgBase_Item kItem;
	if( S_OK != pkPlayer->GetInven()->GetItem(kArticleInfo.Pos(), kItem) )
	{
		ReturnResult(MET_NOT_ADD_ITEM);
		return false;
	}
	kViewItemInfo.kItem = kItem;

	lwUIWnd UIParent(pParent);

	//step 2. 판매 금액 타입 얻기
	lwUIWnd kTemp = UIParent.GetControl("CBTN_GOLD_SELL");
	if( kTemp.IsNil() )
	{
		return false;
	}

	if( kTemp.GetCheckState() )
	{
		kArticleInfo.CostType(RCT_GOLD);
	}
	else
	{
		kTemp = UIParent.GetControl("CBTN_CASH_SELL");
		if( !kTemp.IsNil() )
		{
			if( kTemp.GetCheckState() )
			{
				kArticleInfo.CostType(RCT_CASH);
			}
		}
	}
	
	//step 3. 판매 금액 얻기
	switch(kArticleInfo.CostType())
	{
	case RCT_GOLD:
		{
			std::wstring	wstrVal;
			__int64			i64Money = 0;
			kTemp = UIParent.GetControl("EDT_SELL_GOLD");
			if( kTemp.IsNil() )
			{
				return false;
			}
			wstrVal = kTemp.GetEditText()();
			i64Money = (_wtoi64(wstrVal.c_str()) * 10000);

			kTemp = UIParent.GetControl("EDT_SELL_SILVER");
			if( kTemp.IsNil() )
			{
				return false;
			}
			wstrVal = kTemp.GetEditText()();
			i64Money += (_wtoi64(wstrVal.c_str()) * 100);

			kTemp = UIParent.GetControl("EDT_SELL_COPPER");
			if( kTemp.IsNil() )
			{
				return false;
			}
			wstrVal = kTemp.GetEditText()();
			i64Money += (_wtoi64(wstrVal.c_str()));

			kArticleInfo.ArticleCost(i64Money);
		}break;
	case RCT_CASH:
		{
			kTemp = UIParent.GetControl("EDT_SELL_CASH");
			if(!kTemp.IsNil() )
			{
				std::wstring wstrVal = kTemp.GetEditText()();
				kArticleInfo.ArticleCost(_wtoi(wstrVal.c_str()));
			}			
		}break;
	}

	if( !kArticleInfo.ArticleCost() )
	{
		//ReturnResult(MET_NOT_COST_VALUE);
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403184, true);
		return false;
	}

	//step 4. 등록 결과 얻기
	kViewItemInfo.Guid = kItem.Guid();
	kViewItemInfo.eCostType = ((kArticleInfo.CostType())? RCT_CASH: RCT_GOLD);
	kViewItemInfo.i64CostValue = kArticleInfo.ArticleCost();
	kViewItemInfo.kItemType = kArticleInfo.ArticleType();
	kViewItemInfo.SellMoney = kArticleInfo.SellMoney();

	return true;
}

bool lwMarket::AddRegistSellArticleInMoney(XUI::CXUI_Wnd* pParent, int const iType, SARTICLEINFO& kArticleInfo, SMarketItemInfoBase& kViewItemInfo)
{
	if( !pParent )
	{ 
		return false; 
	}

	SMarketIconInfo	IconInfo;
	
	kArticleInfo.ArticleType( static_cast<eUserMarketArticleType>(iType) );

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	std::wstring kMyMoney;
	std::wstring kChangeMoney;
	__int64 iMoney = 0;
	switch( kArticleInfo.ArticleType() )
		{
	case UMAT_CASH:	
		{ 
			iMoney = pkPlayer->GetAbil64(AT_CASH);
			kMyMoney = TTW(403256);
			kChangeMoney = TTW(403257);
			kViewItemInfo.kItem.ItemNo( MARKET_ARTICLE_CASHNO );
		}break;
	case UMAT_MONEY:
		{ 
			iMoney = pkPlayer->GetAbil64(AT_MONEY);
			kMyMoney = TTW(403257);
			kChangeMoney = TTW(403256);
			kViewItemInfo.kItem.ItemNo( MARKET_ARTICLE_MONEYNO );
		}break;
	default:
		{
			return false;
		}
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(L"EDT_SELL_VALUE"));
	if( pEdit )
	{
		kArticleInfo.SellMoney(static_cast<short>(_wtoi(pEdit->EditText().c_str())));
	}

	pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(L"EDT_SELL_COST"));
	if( pEdit )
	{
		__int64 iCost = _wtoi64(pEdit->EditText().c_str());
		if( UMAT_CASH == iType )
		{
			iCost *= GOLD_MONEY_VALUE;
		}
		kArticleInfo.ArticleCost(iCost);
	}

	wchar_t szTemp[MAX_PATH] = {0,};
	if( !kArticleInfo.ArticleCost() )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403264).c_str(), kMyMoney.c_str());
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
		return false;
	}

	if( !kArticleInfo.SellMoney() )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403264).c_str(), kChangeMoney.c_str());
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
		return false;
	}

	int iSellMoney = kArticleInfo.SellMoney();
	if( kArticleInfo.ArticleType() == UMAT_MONEY )
	{
		iSellMoney *= GOLD_MONEY_VALUE;
	}

	if( iSellMoney > iMoney )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403265).c_str(), kMyMoney.c_str());
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
		return false;
	}

	//step 4. 등록 결과 얻기
	kViewItemInfo.Guid.Generate();
	kViewItemInfo.i64CostValue = kArticleInfo.ArticleCost();
	kViewItemInfo.kItemType = kArticleInfo.ArticleType();
	kViewItemInfo.SellMoney = kArticleInfo.SellMoney();

	// Item 정보 설정
	switch( kArticleInfo.ArticleType() )
		{
	case UMAT_CASH:	
		{ 
			kViewItemInfo.kItem.ItemNo( MARKET_ARTICLE_CASHNO );
			kViewItemInfo.eCostType = RCT_GOLD;
		}break;
	case UMAT_MONEY:
		{ 
			kViewItemInfo.kItem.ItemNo( MARKET_ARTICLE_MONEYNO );
			kViewItemInfo.eCostType = RCT_CASH;
		}break;
	default:
		{
			return false;
		}
	}
	kViewItemInfo.kItem.Count( kArticleInfo.SellMoney() );

	return true;
}

void lwMarket::lwCloseAddRegUI(lwUIWnd UIParent)
{
	UIParent.Close();
	XUI::CXUI_Wnd* pInv = XUIMgr.Activate(L"Inv");
	pInv->Priority(0);
	pInv->Close();
}

void lwMarket::lwCallAddRegUI()
{
	if( g_kMarketMgr.ShopState() != MS_OPEN )
	{
		XUI::CXUI_Wnd*	pkWnd = XUIMgr.Call(L"SFRM_ARTICLE_REG_WIN");
		if( !pkWnd ){ return; }

		XUI::CXUI_Wnd* pkRealWnd = pkWnd;
		switch(g_kLocal.ServiceRegion())
		{
		case LOCAL_MGR::NC_EU:
		case LOCAL_MGR::NC_FRANCE:
		case LOCAL_MGR::NC_GERMANY:
		case LOCAL_MGR::NC_USA:
		case LOCAL_MGR::NC_THAILAND:
		case LOCAL_MGR::NC_INDONESIA:
		case LOCAL_MGR::NC_RUSSIA:
		case LOCAL_MGR::NC_PHILIPPINES:
		case LOCAL_MGR::NC_JAPAN:
		case LOCAL_MGR::NC_KOREA:
			{
			}break;
		default :
			{
				XUI::CXUI_Wnd* pkTemp = pkWnd->GetControl(L"FRM_REG_TYPE0");
				if( !pkTemp )
				{
					pkWnd->Close();
					return;
				}

				pkRealWnd = pkTemp;
			}break;
		}



		XUI::CXUI_Wnd* pExpCard = pkRealWnd->GetControl(L"FRM_EXPCARD");
		if( pExpCard ){ pExpCard->Visible(false); };

		XUI::CXUI_Wnd* pIcon = pkRealWnd->GetControl(L"ICON_ITEM");
		if( !pIcon ){ return; }
		pIcon->ClearCustomData();

		XUI::CXUI_Wnd* pIconText = pkRealWnd->GetControl(L"FRM_REG_ITEM_NAME");
		if( !pIconText ){ return; }
		pIconText->Text(L"");

		XUI::CXUI_CheckButton* pGoldCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pkRealWnd->GetControl(L"CBTN_GOLD_SELL"));
		if( !pGoldCheck ){ return; }
		pGoldCheck->Check(true);

		XUI::CXUI_CheckButton* pCashCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pkRealWnd->GetControl(L"CBTN_CASH_SELL"));
		if(pCashCheck)
		{
			pCashCheck->Check(false);
		}

		XUI::CXUI_Edit* pEditGold = dynamic_cast<XUI::CXUI_Edit*>(pkRealWnd->GetControl(L"EDT_SELL_GOLD"));
		XUI::CXUI_Edit* pEditSilver = dynamic_cast<XUI::CXUI_Edit*>(pkRealWnd->GetControl(L"EDT_SELL_SILVER"));
		XUI::CXUI_Edit* pEditCopper = dynamic_cast<XUI::CXUI_Edit*>(pkRealWnd->GetControl(L"EDT_SELL_COPPER"));
		XUI::CXUI_Edit* pEditCash = dynamic_cast<XUI::CXUI_Edit*>(pkRealWnd->GetControl(L"EDT_SELL_CASH"));
		if( !pEditGold ||  !pEditSilver ||  !pEditCopper )
		{
			return;
		}
		pEditGold->EditText(L"");
		pEditSilver->EditText(L"");
		pEditCopper->EditText(L"");
		if(pEditCash)
		{
			pEditCash->EditText(L"");
		}

		XUI::CXUI_Button* pSendBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
		if( !pSendBtn ){ return; }
		pSendBtn->Disable(false);

		XUI::CXUI_Wnd* pInv = XUIMgr.Call(L"Inv");
		if( !pInv ){ return; }
		POINT2	pt(pkWnd->Location().x + pkWnd->Size().x + 30, pkWnd->Location().y);
		pInv->Location(pt);
		pInv->Priority(5);
	}
	else
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403114, true);
	}
}

void lwMarket::lwDelRegListItem(lwUIWnd UITab)
{
	lwUIWnd	kList = UITab.GetControl("LIST_AUCTION_REG_ITEM");
	if( kList.IsNil() )
	{
		return;
	}

	lwUIListItem kItem = kList.ListFirstItem();
	while( !kItem.IsNil() )
	{
		lwUIWnd kItemWnd = kItem.GetWnd();

		lwUIWnd kTemp = kItemWnd.GetControl("FRM_ITEM_CLICK");
		if( kTemp.IsNil() )
		{
			return;
		}

		if( kTemp.IsVisible() )
		{
			if( ReturnResult((EMarketErrType)g_kMarketMgr.RequestDelRegItem(kItemWnd.GetCustomDataAsGuid()())) )
			{
				lwUIWnd UIBtn = UITab.GetParent().GetControl("BTN_ACTION");
				if( !UIBtn.IsNil() )
				{
					UIBtn.Disable(g_kMarketMgr.IsSendOK());
				}
				return;
			}
		}

		kItem = kList.ListNextItem(kItem);
	}
}

void lwMarket::lwDrawShopOpenTime(lwUIWnd UISelf, lwUIWnd UITab, int Type)
{
	if(!g_pkWorld)
	{
		return;
	}
	static float PrevTime = g_pkWorld->GetAccumTime();

	E_MARKET_GRADE ShopType = MG_NORMAL;
	if( g_kMarketMgr.ShopState() == MS_CLOSE)
	{
		//ShopType = GetShopGrade();
		ShopType = g_kMarketMgr.GetCurrentMarketGrade();
	}
	else
	{
		ShopType = (E_MARKET_GRADE)g_kMarketMgr.MyShopInfo().ShopInfo().iGrade;
		float NowTime = g_pkWorld->GetAccumTime();
		if( MS_OPEN == g_kMarketMgr.ShopState() && (NowTime - PrevTime) > 1.0f )
		{
			g_kMarketMgr.FlowTime(NowTime - PrevTime);
			PrevTime = NowTime;
		}
	}

	__int64 iTime = 0;
	SYSTEMTIME	Time;
	if( g_kMarketMgr.ShopState() == MS_CLOSE)
	{
		switch( Type )
		{
		case RTT_ONLINE:
			{
				iTime = g_kMarketMgr.GetMaxOnlineOpenTime(ShopType);
			}break;
		case RTT_OFFLINE:
			{
				iTime = g_kMarketMgr.GetMaxOfflineOpenTime(ShopType);
			}break;
		}
	}
	else
	{
		switch( Type )
		{
		case RTT_ONLINE:
			{
				iTime = g_kMarketMgr.MyShopInfo().ShopInfo().iOnTime;
			}break;
		case RTT_OFFLINE:
			{
				iTime = g_kMarketMgr.MyShopInfo().ShopInfo().iOffTime;
			}break;
		}
	}

	iTime = __max(0, iTime);
	Time.wDay = ((iTime / DEF_SEC_TIME) / DEF_MIN_TIME) / DEF_HOUR_TIME;
	Time.wHour = ((iTime / DEF_SEC_TIME) / DEF_MIN_TIME) % DEF_HOUR_TIME;
	Time.wMinute = (iTime / DEF_SEC_TIME) % DEF_MIN_TIME;

	wchar_t	szTemp[MAX_PATH] = {0,};
	swprintf_s(szTemp, MAX_PATH, TTW(403059).c_str(), Time.wDay, Time.wHour, Time.wMinute);
	UISelf.SetStaticTextW(szTemp);
}

void lwMarket::AddItemListItem(lwUIWnd& UIList, SMarketItemInfoBase const& ItemInfo)
{
	lwUIListItem kItem = UIList.AddNewListItem(lwWString(""));
	if( kItem.IsNil() )
	{
		return;
	}

	lwUIWnd kItemWnd = kItem.GetWnd();
	kItemWnd.SetCustomDataAsGuid(ItemInfo.Guid);

	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
		}break;
	default :
		{
			if( ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				bIsNonCashItem = false;
			}
		}break;
	}

	if( bIsNonCashItem )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(ItemInfo.kItem.ItemNo());
		if (!pItemDef)
		{
			return;
		}

		//step 1. 아이콘 설정
		lwMarketUtil::SetUIIconToBaseItem(kItemWnd.GetSelf(), L"IMG_ICON", ItemInfo.kItem);

		//step 2. 이름
		lwMarketUtil::SetUITextToItemName(kItemWnd.GetSelf(), L"FRM_ITEM_NAME", ItemInfo.kItem.ItemNo(), ItemInfo.kItem.EnchantInfo(), true);
		lwMarketUtil::SetUIText(kItemWnd.GetSelf(), L"FRM_ITEM_DISPLAY_GRADE", lwChangeIndexToSysEmoFont(pItemDef->GetAbil(AT_ITEM_DISPLAY_GRADE)).GetWString());

		//step 3. 레벨
		BM::vstring vStr(TTW(224));
		vStr << (pItemDef->GetAbil(AT_LEVELLIMIT));
		lwMarketUtil::SetUIText(kItemWnd.GetSelf(), L"FRM_ITEM_LEVEL", vStr);
	}
	else
	{
		bool const bIsCashToMoney = ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;

		float fMinDest, fMinSour, fRate;
		lwMarketUtil::CalcMoneyChangeRate( ItemInfo.kItem.Count(), (bIsCashToMoney)?(ItemInfo.i64CostValue / GOLD_MONEY_VALUE):(ItemInfo.i64CostValue), fMinDest, fMinSour, fRate );
		std::wstring kMoneyText = TTW((bIsCashToMoney)?(403256):(403257));

		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, L"[%s]%d %s (%s : %s)"
			, kMoneyText.c_str()
			, ItemInfo.kItem.Count()
			, kMoneyText.c_str()
			, lwMarketUtil::FloatConvertToStr(fMinDest, 4).c_str()
			, lwMarketUtil::FloatConvertToStr(fMinSour, 4).c_str() );

		//step 1. 아이콘 설정
		lwMarketUtil::SetUIIconToItemNo(kItemWnd.GetSelf(), L"IMG_ICON", (bIsCashToMoney)?(MARKET_ARTICLE_CASHNO):(MARKET_ARTICLE_MONEYNO), SEnchantInfo(), 1, BM::PgPackedTime());

		//step 2. 이름
		std::wstring kItemName(L"{C=");
		kItemName += TTW(4106);
		kItemName += L"/}";
		kItemName += szTemp;
		lwMarketUtil::SetUIText(kItemWnd.GetSelf(), L"FRM_ITEM_NAME", kItemName, true);
		lwMarketUtil::SetUIText(kItemWnd.GetSelf(), L"FRM_ITEM_DISPLAY_GRADE",  std::wstring(L""));

		//step 3. 레벨
		lwMarketUtil::SetUIText(kItemWnd.GetSelf(), L"FRM_ITEM_LEVEL",  std::wstring(L"-"));
	}

	//step 4. 가격
	lwMarketUtil::SetUITextToItemCost(kItemWnd.GetSelf(), L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)ItemInfo.eCostType, ItemInfo.i64CostValue);
}

void lwMarket::RegListClear()
{
	lwUIWnd kAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( kAuction.IsNil() )
	{
		return;
	}

	lwUIWnd kTab = kAuction.GetControl("FRM_SALE_REG");
	if( kTab.IsNil() )
	{
		return;
	}

	lwUIWnd kList = kTab.GetControl("LIST_AUCTION_REG_ITEM");
	if( kList.IsNil() )
	{
		return;
	}
	kList.ClearAllListItem();
}

void lwMarket::lwRequestShopStateChange()
{
	SPT_C_M_UM_REQ_MARKET_MODIFY_STATE	Data;

	switch(g_kMarketMgr.ShopState())
	{
	case MS_OPEN:
		{
			Data.MarketState(MS_EDIT);
		}break;
	case MS_EDIT:
		{
			Data.MarketState(MS_OPEN);
		}break;
	default:
		{
			return;
		}
	}
	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);	
	
	if(MS_EDIT == g_kMarketMgr.ShopState())
	{// 수정하기라면
		SMyShopInfo const& kShopInfo = g_kMarketMgr.MyShopInfo();
		SMyShopData const& kShopData = kShopInfo.ShopInfo();		
		{// 선택되어있는 상점 종류를 원래대로 세팅하고
			lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
			if( UIAuction.IsNil() )
			{
				return;
			}

			lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
			if( UITab.IsNil() )
			{
				return;
			}

			lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
			lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
			lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");
			if(kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil())
			{
				return;
			}
			// 화면에 보이는 체크 버튼을 모두 초기화 한후
			kShop1.CheckState(false);
			kShop2.CheckState(false);
			kShop3.CheckState(false);
			
			g_kMarketMgr.SetCurrentMarketGrade(kShopData.iGrade);
			g_kMarketMgr.SetPrevMarketGrade(kShopData.iGrade);
			switch(g_kMarketMgr.GetCurrentMarketGrade())
			{// 되돌린 체크 버튼의 체크 상태를 true로 하여 화면에 보여줌
			case MG_NORMAL:
				{
					kShop1.CheckState(true);
				}break;
			case MG_GOOD:
				{			
					kShop2.CheckState(true);
				}break;
			case MG_HISTORY:
				{
					kShop3.CheckState(true);
				}break;
			default:
				{
					_PgMessageBox("lwMarket::lwRevertMarketGrade()", "Unknown MarketGrade value");
				}break;	
			}
		}		
		{// 상점 이름을 원래대로 한다.			
			lwUIWnd kAuctionMain = lwGetUIWnd("FRM_AUCTION_MAIN");
			if(true == kAuctionMain.IsNil())
			{
				return;
			}

			lwUIWnd kSaleWnd = kAuctionMain.GetControl("FRM_SALE_REG");
			if(true == kSaleWnd.IsNil())
			{
				return;
			}

			lwUIWnd kEdtWnd  = kSaleWnd.GetControl("EDT_SHOP_NAME");
			if(true == kEdtWnd.IsNil())
			{
				return;
			}

			XUI::CXUI_Edit* pEdtWnd = dynamic_cast<XUI::CXUI_Edit*>(kEdtWnd.GetSelf());			
			if(NULL == pEdtWnd) 
			{
				return;
			}						
			pEdtWnd->EditText(kShopData.kMarketName);
		}
		{// 상점 열기 확인 창을 다시 띄운다
			CallMarketOpenConfirm(kPacket);
		}
	}
	else
	{// 상점 중지 라면 바로 패킷을 보낸다
		NETWORK_SEND(kPacket)
	}
}

void lwMarket::lwRequestShopInfoChange()
{
	if( g_kMarketMgr.ShopState() == MS_OPEN )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403114, true);
		return;
	}
	else if( g_kMarketMgr.ShopState() == MS_CLOSE )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403112, true);
		return;
	}

	XUI::CXUI_Wnd* pTemp = XUIMgr.Get(_T("FRM_AUCTION_MAIN"));
	XUI::CXUI_Edit* pEdit = NULL;
	XUI::CXUI_Button* pBtn = NULL;
	if( pTemp )
	{
		pTemp = pTemp->GetControl(_T("FRM_SALE_REG"));
		if( !pTemp )
		{
			return;
		}
	}

	pEdit = dynamic_cast<XUI::CXUI_Edit*>(pTemp->GetControl(_T("EDT_SHOP_NAME")));
	if( !pEdit )
	{
		return;
	}
	std::wstring kShopName = pEdit->EditText();
	if(g_kClientFS.Filter(kShopName, false)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_OPEM_MARKET_NAME, kShopName)
		)	//욕설 등이 있으면
	{
		::Notice_Show( TTW(600037), 0);
		return;
	}
	E_MARKET_GRADE Type = (E_MARKET_GRADE)g_kMarketMgr.MyShopInfo().ShopInfo().iGrade;
	//bool bIsChangeInfo = (Type != GetShopGrade())?(true):(false);
	bool bIsChangeInfo = (Type != g_kMarketMgr.GetCurrentMarketGrade())?(true):(false);	
	if( bIsChangeInfo )
	{
		//Type = GetShopGrade();
		Type = g_kMarketMgr.GetCurrentMarketGrade();
	}
	std::wstring kText = (kShopName.compare(g_kMarketMgr.MyShopInfo().ShopInfo().kMarketName) ==0)?(_T("")):(pEdit->EditText());
	if( bIsChangeInfo || kText.size() )
	{
		RequestUseShopModifyItem(pTemp, kText, Type, bIsChangeInfo);
	}
}

void lwMarket::lwRequestShopUpdateOpenTime(lwUIWnd UITab)
{
	if( g_kMarketMgr.ShopState() == MS_OPEN )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403114, true);
		return;
	}
	else if( g_kMarketMgr.ShopState() == MS_CLOSE )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403112, true);
		return;
	}

	lwUIWnd kTemp = UITab.GetControl("EDT_SHOP_NAME");
	if( kTemp.IsNil() )
	{
		return;
	}
	std::wstring kShopName = kTemp.GetEditText()();
	if(g_kClientFS.Filter(kShopName, false)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_OPEM_MARKET_NAME, kShopName)
		)	//욕설 등이 있으면
	{
		::Notice_Show( TTW(600037), 0);
		return;
	}

	SPT_M_C_UM_NOTI_USE_MARKET_MODIFY_ITEM	Data;

	E_MARKET_GRADE Type = (E_MARKET_GRADE)g_kMarketMgr.MyShopInfo().ShopInfo().iGrade;
	//if( GetShopGrade() != Type )
	if( g_kMarketMgr.GetCurrentMarketGrade() != Type )	
	{
		return;
	}
	
	//step 1. 상점종류에 따른 개설증서 찾기
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		ReturnResult(MET_FAIL);
		return;
	}

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		ReturnResult(MET_FAIL);
		return;
	}

	RequestUseShopModifyItem(UITab.GetSelf(), _T(""), Type);
}

void lwMarket::RequestUseShopModifyItem(XUI::CXUI_Wnd* pWnd, std::wstring const& kShopName, E_MARKET_GRADE const eType, bool const bIsChange)
{
	if( !pWnd )
	{
		return;
	}

	SPT_M_C_UM_NOTI_USE_MARKET_MODIFY_ITEM	Data;

	XUI::CXUI_Wnd* kEdit = pWnd->GetControl(_T("EDT_SHOP_NAME"));
	if( kEdit )
	{
		Data.MarketName(kEdit->Text());
	}

	//step 1. 상점종류에 따른 개설증서 찾기
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		ReturnResult(MET_FAIL);
		return;
	}

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		ReturnResult(MET_FAIL);
		return;
	}

	SItemPos	TimeAddItem;
	bool const bPremiumService = pkPlayer->GetPremium().IsUserService(true) && (NULL != pkPlayer->GetPremium().GetType<S_PST_OpenmarketState>());
	if(false==bPremiumService && eType != MG_NORMAL )
	{
		DWORD const dwItemNo = (eType == MG_GOOD)?(99600010):(99600020);

		if( bIsChange || !kShopName.size() )
		{
			if( dwItemNo != 0 )
			{
				// 좋은 상점 개설 증서 번호 (99600010)
				if( E_FAIL == pkInven->GetFirstItem(dwItemNo, TimeAddItem) )
				{
					ReturnResult(MET_REG_CONTRACT_INS);
					return;
				}
			}
		}
	}
	Data.Pos(TimeAddItem);
	Data.MarketGrade(eType);

	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);
	if(MG_NORMAL == eType)
	{// 일반 상점으로 수정일 경우, 상점 유지시간 초기화 메세지를 알려준다
		lwCallCommonMsgYesNoBox(MB(TTW(403254)), lwPacket(&kPacket), true, MBT_MODIFY_TO_NOMALMARKET);
		return;
	}
	NETWORK_SEND(kPacket)
}

void lwMarket::lwRequestShopUpdateHP(int const ItemNo)
{
	if( g_kMarketMgr.ShopState() == MS_OPEN )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403114, true);
		return;
	}
	else if( g_kMarketMgr.ShopState() == MS_CLOSE )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403112, true);
		return;
	}

	if( g_kMarketMgr.MyShopInfo().ShopInfo().iGrade == MG_NORMAL )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403119, true);
		return;
	}

	ContHaveItemNoCount	kItemCont;
	if( UIItemUtil::SearchEqualTypeItemList(UICT_MARKET_MODIFY, kItemCont) )
	{
		if( 1 < kItemCont.size() )
		{
			UIItemUtil::CONT_CUSTOM_PARAM	kParam;
			UIItemUtil::CONT_CUSTOM_PARAM_STR kParamStr;
			std::pair<UIItemUtil::CONT_CUSTOM_PARAM::iterator, bool> ParamRst = kParam.insert(std::make_pair(std::wstring(L"SHOP_GRADE"), g_kMarketMgr.MyShopInfo().ShopInfo().iGrade));
			if( !ParamRst.second )
			{
				return;
			}

			std::pair<UIItemUtil::CONT_CUSTOM_PARAM_STR::iterator, bool> ParamStrRst = kParamStr.insert(std::make_pair(std::wstring(L"SHOP_NAME"), g_kMarketMgr.MyShopInfo().ShopInfo().kMarketName));
			if( !ParamStrRst.second )
			{
				return;
			}
			UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, kParamStr);
		}
		else
		{
			ContHaveItemNoCount::iterator	item_itor = kItemCont.begin();
			if( kItemCont.end() == item_itor )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403091, true);
			}

			ContHaveItemNoCount::mapped_type const& kKey = item_itor->first;

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			PgInventory* pkInv = pkPlayer->GetInven();
			if( !pkInv )
			{
				return;
			}

			SPT_M_C_UM_NOTI_USE_MARKET_MODIFY_ITEM	Data;
			SItemPos	HPUpItem;
			if( S_OK == pkInv->GetFirstItem(kKey, HPUpItem) )
			{
				Data.Pos(HPUpItem);
				Data.MarketGrade(g_kMarketMgr.MyShopInfo().ShopInfo().iGrade);
				Data.MarketName(g_kMarketMgr.MyShopInfo().ShopInfo().kMarketName);

				BM::Stream	kPacket;
				Data.WriteToPacket(kPacket);
				NETWORK_SEND(kPacket);
			}
		}
		return;
	}
	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403091, true);
}

void lwMarket::AddRegListItemToPacketData(SMarketItemInfoBase const& ItemInfo)
{
	lwUIWnd kAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( kAuction.IsNil() )
	{
		return;
	}

	lwUIWnd kTab = kAuction.GetControl("FRM_SALE_REG");
	if( kTab.IsNil() )
	{
		return;
	}

	lwUIWnd kList = kTab.GetControl("LIST_AUCTION_REG_ITEM");
	if( kList.IsNil() )
	{
		return;
	}

	AddItemListItem(kList, ItemInfo);
}

void lwMarket::SetMinimumCost(int const iGold, int const iSilver, int const iCopper)
{
	lwUIWnd kReg = lwGetUIWnd("SFRM_ARTICLE_REG_WIN");
	if( kReg.IsNil() )
	{
		return;
	}

	lwUIWnd kMoney = kReg.GetControl("EDT_SELL_GOLD");
	if( kMoney.IsNil() )
	{
		return;
	}
	BM::vstring	vStr(iGold);
	kMoney.SetEditTextW(lwWString((std::wstring const&)vStr));

	kMoney = kReg.GetControl("EDT_SELL_SILVER");
	if( kMoney.IsNil() )
	{
		return;
	}
	vStr = iSilver;
	kMoney.SetEditTextW(lwWString((std::wstring const&)vStr));

	kMoney = kReg.GetControl("EDT_SELL_COPPER");
	if( kMoney.IsNil() )
	{
		return;
	}
	vStr = iCopper;
	kMoney.SetEditTextW(lwWString((std::wstring const&)vStr));
}

void lwMarket::lwRequestShopAction(lwUIWnd UITab, lwUIWnd UISelf)
{
	lwUIWnd kTemp = UITab.GetControl("EDT_SHOP_NAME");
	if( kTemp.IsNil() )
	{
		return;
	}
	std::wstring kShopName = kTemp.GetEditText()();
	if(g_kClientFS.Filter(kShopName, false)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_OPEM_MARKET_NAME, kShopName)
		)	//욕설 등이 있으면
	{
		::Notice_Show( TTW(600037), 0);
		return;
	}

	switch(g_kMarketMgr.ShopState())
	{
	case MS_CLOSE:
		{
			//E_MARKET_GRADE Type = GetShopGrade();
			E_MARKET_GRADE Type = g_kMarketMgr.GetCurrentMarketGrade();

			ReturnResult((EMarketErrType)g_kMarketMgr.RequestOpenShop(kShopName, Type));
			if( UISelf.IsNil() )
			{
				UISelf.Disable(g_kMarketMgr.IsSendOK());
			}
			lwClearShopRegUI(UITab);
		}break;
	default:
		{
			lwRequestShopStateChange();
		}break;
	}
}

void lwMarket::lwUpdataRegAction()
{
	lwUIWnd	UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UIReg = UIAuction.GetControl("FRM_SALE_REG");
	if( UIReg.IsNil() )
	{
		return;
	}

	lwUIWnd UIBtn = UIReg.GetControl("BTN_SHOP_OPEN");
	if( UIBtn.IsNil() )
	{
		return;
	}

	switch(g_kMarketMgr.ShopState())
	{
	case MS_OPEN:
		{
			UIBtn.SetStaticText(MB(TTW(403068)));
		}break;
	case MS_EDIT:
		{
			UIBtn.SetStaticText(MB(TTW(403023)));
		}break;
	default:
		{
			UIBtn.SetStaticText(MB(TTW(403023)));
			return;
		}
	}
}

void lwMarket::lwClearShopRegUI(lwUIWnd UITab)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}

	lwUIWnd kShopName = UITab.GetControl("EDT_SHOP_NAME");
	if( kShopName.IsNil() )
	{
		return;
	}

	if( g_kMarketMgr.ShopState() != MS_CLOSE )
	{
		kShopName.SetEditText(MB(g_kMarketMgr.MyShopInfo().ShopInfo().kMarketName));
	}
	else
	{
		kShopName.SetEditText(MB(pkPlayer->Name()));
	}

	lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
	lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
	lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");
	if( kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil() )
	{
		return;
	}
	kShop1.CheckState(false);
	kShop2.CheckState(false);
	kShop3.CheckState(false);

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return;
	}

	SItemPos	ShopContractPos;
	//if( E_FAIL == pkInven->GetFirstItem(99600010, ShopContractPos) )
	//{// Disable 하면 툴팁이 보이지 않기 때문에 제거
	//	kShop2.Disable(false);
	//}
	//else
	//{
	//	kShop2.Disable(false);
	//}


	//if( E_FAIL == pkInven->GetFirstItem(99600020, ShopContractPos) )
	//{// Disable 하면 툴팁이 보이지 않기 때문에 제거
	//	kShop3.Disable(false);
	//}
	//else
	//{
	//	kShop3.Disable(false);
	//}

	switch(g_kMarketMgr.MyShopInfo().ShopInfo().iGrade)
	{
	case MG_NORMAL:
		{
			kShop1.CheckState(true);
		}break;
	case MG_GOOD:
		{
			kShop2.CheckState(true);
		}break;
	case MG_HISTORY:
		{
			kShop3.CheckState(true);
		}break;
	}
}

void lwMarket::lwRequestMinimumCost(lwUIWnd UITab)
{
	lwUIWnd	kTemp = UITab.GetControl("ICON_ITEM");
	if( kTemp.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* pTemp = kTemp.GetSelf();
	
	SMarketIconInfo	kInfo;
	pTemp->GetCustomData(&kInfo, sizeof(SMarketIconInfo));

	lwGUID	kGuid = UITab.GetCustomDataAsGuid();
	
	g_kMarketMgr.RequestMinimumCost(SItemPos(kInfo.cItemGroup, kInfo.cInvPos), kGuid());
}

void lwMarket::SetSoldListItem(lwUIWnd& UIItem, SMarketSoldItemInfo const& ItemInfo)
{
	UIItem.SetCustomDataAsGuid(ItemInfo.Guid);

	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
		}break;
	default :
		{
			if( ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				bIsNonCashItem = false;
			}
		}break;
	}

	if( bIsNonCashItem )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(ItemInfo.kItem.ItemNo());
		if(!pDef)
		{
			return;
		}

		//step 1. 아이콘
		lwMarketUtil::SetUIIconToBaseItem(UIItem.GetSelf(), L"IMG_ICON", ItemInfo.kItem);

		//step 2. 이름 입력
		lwMarketUtil::SetUITextToItemName(UIItem.GetSelf(), L"FRM_ITEM_NAME", ItemInfo.kItem.ItemNo(), ItemInfo.kItem.EnchantInfo(), true);
		lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_DISPLAY_GRADE", lwChangeIndexToSysEmoFont(pDef->GetAbil(AT_ITEM_DISPLAY_GRADE)).GetWString());
		
		//step 3. 요구 레벨
		BM::vstring vStr(TTW(224));
		vStr << (pDef->GetAbil(AT_LEVELLIMIT));
		lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_LEVEL", vStr);
	}
	else
	{
		bool const bIsCashToMoney = ItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;

		float fMinDest, fMinSour, fRate;
		lwMarketUtil::CalcMoneyChangeRate( ItemInfo.kItem.Count(), (bIsCashToMoney)?(ItemInfo.i64CostValue / GOLD_MONEY_VALUE):(ItemInfo.i64CostValue), fMinDest, fMinSour, fRate );
		std::wstring kMoneyText = TTW((bIsCashToMoney)?(403256):(403257));

		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, L"[%s]%d %s (%s : %s)"
			, kMoneyText.c_str()
			, ItemInfo.kItem.Count()
			, kMoneyText.c_str()
			, lwMarketUtil::FloatConvertToStr(fMinDest, 4).c_str()
			, lwMarketUtil::FloatConvertToStr(fMinSour, 4).c_str() );

		//step 1. 아이콘
		lwMarketUtil::SetUIIconToItemNo(UIItem.GetSelf(), L"IMG_ICON", (bIsCashToMoney)?(MARKET_ARTICLE_CASHNO):(MARKET_ARTICLE_MONEYNO), SEnchantInfo(), 1, BM::PgPackedTime());

		//step 2. 이름 입력
		std::wstring kItemName(L"{C=");
		kItemName += TTW(4106);
		kItemName += L"/}";
		kItemName += szTemp;
		lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_NAME", kItemName, true);
		lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_DISPLAY_GRADE", std::wstring(L""));
		
		//step 3. 요구 레벨
		lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_LEVEL", std::wstring(L"-"));		
	}
	
	//step 4. 판매가격
	lwMarketUtil::SetUITextToItemCost(UIItem.GetSelf(), L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)ItemInfo.eCostType, ItemInfo.i64CostValue);

	//step 5. 수수료
	BM::vstring vStr(ItemInfo.iRate);
	vStr += _T("%");
	lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_CHARGE", vStr);

	int const iRateValue = ItemInfo.i64CostValue * static_cast<double>(ItemInfo.iRate) / 100.0;
	__int64 const iRealCostValue = ItemInfo.i64CostValue - iRateValue;

	//step 6. 수수료 뺀 가격
	lwMarketUtil::SetUITextToItemCost(UIItem.GetSelf(), L"FRM_ITEM_MONEY3", L"FRM_ITEM_MONEY4", (ERegCostType)ItemInfo.eCostType, iRealCostValue);

	//step 7. 구매자 이름
	lwMarketUtil::SetUIText(UIItem.GetSelf(), L"FRM_ITEM_BUYER", ItemInfo.wstrBuyerName);

	UIItem.Visible(true);
}

void lwMarket::DrawSoldPage(int const iPage)
{
	if(  iPage < 0 )
	{
		return;
	}

	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_LIST");
	if( UITab.IsNil() )
	{
		return;
	}

	kSoldList	List;
	if( !g_kMarketMgr.GetSoldPageItem(iPage, List) )
	{
		return;
	}

	kSoldList::iterator	iter = List.begin();
	for(int i = 0; i < MAX_STOCK_SLOT; ++i)
	{
		BM::vstring	vStr(_T("FRM_SOLD_ITEM"));
		vStr += i;
		std::wstring wstrName = (std::wstring const&)vStr;
		lwUIWnd UIItem = UITab.GetControl(MB(wstrName));
		if( UIItem.IsNil() )
		{
			return;
		}

		if( iter != List.end() )
		{
			SetSoldListItem(UIItem, (*iter));
			++iter;
		}
		else
		{
			UIItem.Visible(false);
		}
	}
}

void lwMarket::lwRequestSoldReceive(lwUIWnd UIGroup)
{
	if( UIGroup.IsNil() )
	{
		return;
	}	

	//상점 재고 물품을 받아라
	for( int i = 0; i < MAX_STOCK_SLOT; ++i)
	{
		BM::vstring	vStr(_T("FRM_SOLD_ITEM"));
		vStr += i;
		std::wstring	wstrName = (std::wstring const&)vStr;
		lwUIWnd kItem = UIGroup.GetControl(MB(wstrName));
		if( kItem.IsNil() )
		{
			return;
		}

		lwUIWnd kTemp = kItem.GetControl("FRM_ITEM_CLICK");
		if( kTemp.IsNil() )
		{
			return;
		}

		if( kTemp.IsVisible() )
		{
			BM::GUID Guid = kItem.GetCustomDataAsGuid()();

			SPT_C_M_UM_REQ_DEALINGS_READ	Data;
			Data.DealingGuid(Guid);

			BM::Stream kPacket;
			Data.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket)

			g_kMarketMgr.TempDeleteItem(Guid);
			g_kMarketMgr.IsSendOK(true);
			lwUIWnd UIBtn = UIGroup.GetControl("BTN_SELL_LIST_GETITEM");
			if( !UIBtn.IsNil() )
			{
				UIBtn.Disable(true);
			}
		}
	}
}

void lwMarket::lwPrevSoldPageInfo()
{
	DrawSoldPage(g_kMarketMgr.SoldPage() - 1);
}

void lwMarket::lwNextSoldPageInfo()
{
	DrawSoldPage(g_kMarketMgr.SoldPage() + 1);
}

void lwMarket::lwDrawSoldPage()
{
	DrawSoldPage(g_kMarketMgr.SoldPage());
}

void lwMarket::lwDrawSoldPageInfo(lwUIWnd UISelf)
{
	int iMax = g_kMarketMgr.SoldInfo().Size();
	int iPageFirst = g_kMarketMgr.SoldPage() * MAX_STOCK_SLOT;
	int iPageFinal = ((iPageFirst + MAX_STOCK_SLOT) > iMax)?(iMax):(iPageFirst + MAX_STOCK_SLOT);
	if((iPageFinal % MAX_STOCK_SLOT))
	{
		iPageFirst+=1;
	}
	wchar_t	szTemp[MAX_PATH] = {0,};
	swprintf_s(szTemp, MAX_PATH, TTW(403056).c_str(), iPageFirst, iPageFinal, iMax);
	UISelf.SetStaticTextW(szTemp);	
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void lwMarket::lwMarketAction(lwUIWnd UISelf)
{
	lwUIWnd UIAuction = UISelf.GetParent();

	switch( g_kMarketMgr.TabState() )
	{
	case ETS_SHOPLIST:
		{
			lwRequestEnterShop();
		}break;
	case ETS_SHOPINLIST:
		{
			lwUIWnd UITab = UIAuction.GetControl("FRM_SEARCH");
			if( UITab.IsNil() )
			{
				return;
			}
			OnClickEnterShopBuyItem(UITab);
		}break;
	case ETS_REG:
		{
			lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
			if( UITab.IsNil() )
			{
				return;
			}
			lwDelRegListItem(UITab);
		}break;
	}

	UISelf.Disable(true);
}

void lwMarket::lwTabChangeState(int const TabID)
{
	g_kMarketMgr.TabState((EMarketTabState)TabID);
}

void lwMarket::lwTabBtnChange(int const TabID)
{
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UISearchBtn = UIAuction.GetControl("BTN_SEARCH");
	if( UISearchBtn.IsNil() )
	{
		return;
	}
	UISearchBtn.Disable(false);
	g_kMarketMgr.bIsPushSearchBtn(false);

	lwUIWnd UIRegBtn = UIAuction.GetControl("BTN_ITEM_REGIST");
	if( UIRegBtn.IsNil() )
	{
		return;
	}
	UIRegBtn.Disable(false);
	lwUIWnd UIStockBtn = UIAuction.GetControl("BTN_SALE_RESULT");
	if( UIStockBtn.IsNil() )
	{
		return;
	}
	if( !g_kMarketMgr.SoldInfo().Size() )
	{
		UIStockBtn.Disable(true);
	}
	else
	{
		UIStockBtn.Disable(false);
	}

	lwUIWnd UIActionBtn = UIAuction.GetControl("BTN_ACTION");
	if( UIActionBtn.IsNil() )
	{
		return;
	}
	UIActionBtn.Visible(true);
	UIActionBtn.Disable(true);

	switch(TabID)
	{
	case ETS_SEARCHLIST:
	case ETS_SHOPINLIST:
	case ETS_SHOPLIST:
		{
			if( TabID != ETS_SHOPINLIST )
			{
				lwWString	wStr(TTW(403041));
				UIActionBtn.SetStaticTextW(wStr);
			}
			else
			{
				lwWString	wStr(TTW(403042));
				UIActionBtn.SetStaticTextW(wStr);
			}
		}break;
	case ETS_REG:
		{
			UIRegBtn.Disable(true);
			lwWString	wStr(TTW(403043));
			UIActionBtn.SetStaticTextW(wStr);
		}break;
	case ETS_STOCK:
		{
			UIStockBtn.Disable(true);
			UIActionBtn.Visible(false);
		}break;
	}

	TabChange(TabID);

	if( TabID == ETS_SHOPLIST )
	{
		lwRequestShopList();
	}
}

void lwMarket::TabChange(int const TabID)
{
	lwUIWnd	UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UISearch = UIAuction.GetControl("FRM_SEARCH");
	if( UISearch.IsNil() )
	{
		return;
	}
	UISearch.Visible(false);

	lwUIWnd UIReg = UIAuction.GetControl("FRM_SALE_REG");
	if( UIReg.IsNil() )
	{
		return;
	}
	UIReg.Visible(false);

	lwUIWnd UIStock = UIAuction.GetControl("FRM_SALE_LIST");
	if( UIStock.IsNil() )
	{
		return;
	}
	UIStock.Visible(false);

	lwUIWnd UIBackBtn = UIAuction.GetControl("BTN_BACK_MOVE");
	if( UIBackBtn.IsNil() )
	{
		return;
	}
	UIBackBtn.Visible(false);

	lwUIWnd UIAction = UIAuction.GetControl("BTN_ACTION");
	if( UIAction.IsNil() )
	{
		return;
	}

	lwUIWnd UITitle = UIAuction.GetControl("SFRM_TITLEBAR");
	if( UITitle.IsNil() )
	{
		return;
	}

	std::wstring	kAddText = _T(" {C=") + TTW(4001) + _T("/}") + TTW(403000);
	lwWString	lwStr(TTW(699) + kAddText);
	UITitle.SetStaticTextW(lwStr);

	switch(TabID)
	{
	case ETS_SHOPLIST:
	case ETS_SEARCHLIST:
	case ETS_SHOPINLIST:
		{
			UISearch.Visible(true);

			lwUIWnd ShopList = UISearch.GetControl("FRM_SHOP_LIST_GROUP");
			if( ShopList.IsNil() )
			{
				return;
			}
			ShopList.Visible(false);

			lwUIWnd SearchList = UISearch.GetControl("FRM_SEARCH_LIST_GROUP");
			if( SearchList.IsNil() )
			{
				return;
			}
			SearchList.Visible(false);
			
			lwUIWnd ItemList = UISearch.GetControl("FRM_SHOP_ITEM_GROUP");
			if( ItemList.IsNil() )
			{
				return;
			}
			ItemList.Visible(false);

			if( TabID == ETS_SHOPLIST )
			{
				ShopList.Visible(true);
			}
			else if( TabID == ETS_SEARCHLIST )
			{
				SearchList.Visible(true);
				UIAction.Visible(false);
			}
			else
			{	
				ItemList.Visible(true);
				UIBackBtn.Visible(true);
				UIBackBtn.SetCustomData<int>(g_kMarketMgr.TabState());
			}
			UIAction.SetStaticText(MB(TTW((ETS_SHOPINLIST == TabID)?(403042):(403041))));
		}break;
	case ETS_REG:
		{
			lwClearShopRegUI(UIReg);
			UpdateRegList();
			UIReg.Visible(true);
		}break;
	case ETS_STOCK:
		{
			UIStock.Visible(true);
			lwDrawSoldPage();
		}break;
	}
	lwTabChangeState(TabID);
}

void lwMarket::lwDrawIconImage(lwUIWnd UISelf)
{
	XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(UISelf.GetSelf());
	if( !pIcon )
	{ 
		return; 
	}

	SMarketIconInfo kInfo;
	pIcon->GetCustomData(&kInfo, sizeof(SMarketIconInfo));

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const CItemDef* pItemDef = kItemDefMgr.GetDef(kInfo.dwItemNum);

	PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(kInfo.dwItemNum);
	if( !pkSprite )
	{
		pIcon->DefaultImgTexture(NULL);
		pIcon->SetInvalidate();
		return;
	}

	PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
	if( !pkUVSprite ){ return; }

	pIcon->DefaultImgTexture(pkUVSprite);
	SUVInfo& rkUV = pkUVSprite->GetUVInfo();
	pIcon->UVInfo(rkUV);
	POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
	pIcon->ImgSize(kPoint);
	pIcon->SetInvalidate();
}

void lwMarket::lwDrawMarketItemTooltip(lwUIWnd UISelf)
{
	lwGUID	lwGuid = UISelf.GetParent().GetCustomDataAsGuid();
	if( lwGuid.IsNil() )
	{
		return;
	}

	SMarketItemInfoBase	ItemInfo;
	switch( g_kMarketMgr.TabState() )
	{
	case ETS_SHOPLIST:
		{
			return;
		}break;
	case ETS_REG:
		{
			if( !g_kMarketMgr.FindRegTabItem(lwGuid(), ItemInfo) )
			{
				return;
			}
		}break;
	case ETS_STOCK:
		{
			if( !g_kMarketMgr.FindStockTabItem(lwGuid(), ItemInfo) )
			{
				return;
			}
		}break;
	}

	CallToolTip_SItem(&ItemInfo.kItem, UISelf.GetTotalLocation());
}

int lwMarket::LimitLength(int Data, int Min, int Max)
{
	Data = (Data < Min)?(Min):(Data);
	Data = (Data > Max)?(Max):(Data);

	return Data;
}

void lwMarket::lwClearOpenMarket()
{
	g_kMarketMgr.ClearOpenMarket();
}

bool lwMarket::ReturnResult(EMarketErrType const Rst)
{
	switch(Rst)
	{
	case MET_SUCCESS:
		{
			return true;
		}
	case MET_EXIST_NO_INV_POS:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 20023, true);
		}break;
	case MET_NOT_ADD_ITEM:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403018, true);
		}break;
	case MET_DEALING_MAX_COUNT:
		{
			wchar_t	szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(403268).c_str(), g_kMarketMgr.DealingMax());
			lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);

		}break;
	default:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403119 + Rst, true);
		}break;
	}
	return false;
}

void lwMarket::lwBuildItemCheckUpdate(lwUIWnd UISelf)
{
	lwUIWnd UIGroup = UISelf.GetParent();
	
	for(int i = 0; i < MAX_STOCK_SLOT; ++i)
	{
		BM::vstring vStr(_T("FRM_SOLD_ITEM"));
		vStr += i;
		std::wstring wstrName = (std::wstring const&)vStr;
		lwUIWnd UIItem = UIGroup.GetControl(MB(wstrName));
		if( UIItem.IsNil() )
		{
			return;
		}

		SelectItemCheckT2(UIItem, false);
	}

	lwUIWnd UIButton = UIGroup.GetControl("BTN_SELL_LIST_GETITEM");
	if( UIButton.IsNil() )
	{
		return;
	}
	UIButton.Disable(false);
	SelectItemCheckT2(UISelf, true);
}

void lwMarket::lwListItemCheckUpdate(lwUIWnd UISelf)
{
	lwUIWnd UIList = UISelf.GetParent().GetParent();
	int const iMax = UIList.GetListItemCount();

	lwUIListItem UIItem = UIList.ListFirstItem();
	while( !UIItem.IsNil() )
	{
		SelectItemCheck(UIItem.GetWnd(), false);
		UIItem = UIList.ListNextItem(UIItem);
	}

	SelectItemCheck(UISelf.GetParent(), true);
}

void lwMarket::SelectItemCheck(lwUIWnd& UISelf, bool const bIsClick)
{
	lwUIWnd UICBtn = UISelf.GetControl("CBTN_ITEM_SELECT");
	if( UICBtn.IsNil() )
	{
		return;
	}

	UICBtn.LockClick(bIsClick);
	UICBtn.CheckState(bIsClick);
	lwUIWnd UIBtnImg = UICBtn.GetControl("IMG_SELECT");
	if( !UIBtnImg.IsNil() )
	{
		UIBtnImg.Visible(UICBtn.GetCheckState());
	}
}

void lwMarket::SelectItemCheckT2(lwUIWnd& UISelf, bool const bIsClick)
{
	lwUIWnd UICBtn = UISelf.GetControl("FRM_ITEM_CLICK");
	if( UICBtn.IsNil() )
	{
		return;
	}
	UICBtn.Visible(bIsClick);
}

void lwMarket::RecvMarket_Command(WORD const wPacketType, BM::Stream& rkPacket)
{
	if( wPacketType == PT_M_C_UM_ANS_MARKET_ENTER )
	{
		lwCallUI("FRM_AUCTION_MAIN");
		lwClearOpenMarket();
		lwRequestMyShopInfo();
		lwTabBtnChange(0);

		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, true));
		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, false));

		return ;
	}

	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UITab1 = UIAuction.GetControl("FRM_SEARCH");
	lwUIWnd UITab2 = UIAuction.GetControl("FRM_SALE_LIST");
	lwUIWnd UITab3 = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab1.IsNil() || UITab1.IsNil() || UITab1.IsNil() )
	{
		return;
	}

	switch(wPacketType)
	{
	case PT_M_C_UM_ANS_MARKET_OPEN:	//상점등록
		{
			g_kMarketMgr.ResultOpenShop(rkPacket);
			lwUIWnd UIBtn = UITab3.GetControl("BTN_SHOP_OPEN");
			if( !UIBtn.IsNil() )
			{
				UIBtn.Disable(false);
			}
			g_kMarketMgr.IsSendOK(false);
		}break;
	case PT_M_C_UM_ANS_MY_MARKET_QUERY:
		{
			g_kMarketMgr.ResultRegShopItemInfo(rkPacket);
		}break;
	case PT_M_C_UM_ANS_MY_MARKET_DEALING_QUERY:
		{
			g_kMarketMgr.ResultRegShopItemDealingInfo(rkPacket);
		}break;
	case PT_M_C_UM_ANS_ARTICLE_REG:	//물품등록
		{
			lwUIWnd UIReg = lwGetUIWnd("SFRM_ARTICLE_REG_WIN");
			if( !UIReg.IsNil() )
			{
				lwCloseAddRegUI(UIReg);
			}
			g_kMarketMgr.ResultAddRegItem(rkPacket);
			g_kMarketMgr.IsSendOK(false);
		}break;
	case PT_M_C_UM_ANS_ARTICLE_DEREG: //물품취소
		{
			lwUIWnd UIBtn = UIAuction.GetControl("BTN_ACTION");
			if( !UIBtn.IsNil() && UIBtn.IsDisable() )
			{
				UIBtn.Disable(false);
			}
			g_kMarketMgr.ResultDelRegItem(rkPacket);
			g_kMarketMgr.IsSendOK(false);
		}break;
	case PT_M_C_UM_ANS_MINIMUM_COST_QUERY:	//최하값
		{
			g_kMarketMgr.ResultMinimumCost(rkPacket);
		}break;
	case PT_M_C_UM_ANS_DEALINGS_READ: //금액수령
		{
			g_kMarketMgr.ResultDealingRead(rkPacket);
			lwUIWnd UIBtn = UITab2.GetControl("BTN_SELL_LIST_GETITEM");
			if( !UIBtn.IsNil() )
			{
				UIBtn.Disable(false);
			}
			g_kMarketMgr.IsSendOK(false);
		}break;
	case PT_M_C_UM_ANS_MARKET_QUERY:	//마켓조회
		{
			g_kMKTSearchMgr.RecvSearchItem(rkPacket);
			lwTabBtnChange(ETS_SEARCHLIST);
			DrawSearchList();
		}break;
	case PT_M_C_UM_ANS_MARKET_ARTICLE_QUERY:	//마켓물품조회
		{
			if( g_kMKTShopMgr.RecvEnterShopItem(rkPacket) )
			{
				lwTabBtnChange(ETS_SHOPINLIST);
				DrawEnterShopItemList();
			}
		}break;
	case PT_M_C_UM_ANS_ARTICLE_BUY:	//물품구입
		{
			if( ETS_SEARCHLIST == g_kMarketMgr.TabState() )
			{
				SPT_M_C_UM_ANS_ARTICLE_BUY Data;
				Data.ReadFromPacket(rkPacket);
				if( PgMarketUtil::RequestResult(Data.Result()) )
				{
					SPT_C_M_UM_REQ_MARKET_QUERY	kData = g_kMKTSearchMgr.SearchKey();
					RequestSearchItem(kData);
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403075, true);
				}
			}
			else
			{
				g_kMKTShopMgr.RecvBuyEnterShopItem(rkPacket);
				DrawEnterShopItemList();
			}
			g_kMarketMgr.IsSendOK(false);
		}break;
	case PT_M_C_UM_ANS_MARKET_MODIFY_STATE:
		{
			g_kMarketMgr.ResultUpdateShopState(rkPacket);
		}break;
	case PT_M_C_UM_ANS_USE_MARKET_MODIFY_ITEM:
		{
			size_t const iRDPos = rkPacket.RdPos();

			SPT_M_C_UM_ANS_USE_MARKET_MODIFY_ITEM	Data;
			Data.ReadFromPacket(rkPacket);
			rkPacket.RdPos(iRDPos);

			g_kMarketMgr.ResultUpdateOpenTime(rkPacket);
			if(Data.Type() != EMMIT_NO_MSG)
			{
				lwClearShopRegUI(UITab3);
				UpdateRegList();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403204, true);
			}
		}break;
	case PT_M_C_UM_ANS_MARKET_CLOSE:
		{
			g_kMarketMgr.ResultCloseMyShop(rkPacket);
			lwMarket::lwClearShopRegUI(UITab3);
		}break;
	case PT_M_C_UM_ANS_BEST_MARKET_LIST:
		{
			g_kMKTShopMgr.RecvShopItem(rkPacket);
			DrawShopList();
		}break;	
	}
}

int lwMarket::lwGetPrevMarketGrade()
{
	switch(g_kMarketMgr.GetPrevMarketGrade())	
	{
	case MG_NORMAL:
		{
			return static_cast<int>(MG_NORMAL);
		}break;
	case MG_GOOD:
		{
			return static_cast<int>(MG_GOOD);
		}break;
	case MG_HISTORY:
		{
			return static_cast<int>(MG_HISTORY);			
		}break;
	default:
		{
			_PgMessageBox("lwMarket::lwGetPrevMarketGrade()", "Unknown MarketGrade value");
		}break;
	}
	return static_cast<int>(MG_NORMAL);
}

void lwMarket::lwSetCurrentMarketGrade(int const iGrade)
{
	g_kMarketMgr.SetCurrentMarketGrade(iGrade);	

	if(g_kMarketMgr.GetCurrentMarketGrade() == g_kMarketMgr.GetPrevMarketGrade())
	{//변화 한게 없으면 RegList를 Update 하지 않음
		return;
	}
	
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab.IsNil() )
	{
		return;
	}

	// Disable 되지 않은 좋은, 최고급 상점 체크 버튼의 툴팁이 보이면서 체크버튼 조정한다.
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{		
		return;
	}
	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{	
		return;
	}
	lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
	lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
	lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");
	SItemPos	ShopContractPos;

	if(kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil())
	{		
		return;
	}
	
	//현재 열려 있는 상점의 등급을 얻어오고
	SMyShopInfo const& kMyShopInfo = g_kMarketMgr.MyShopInfo();
	SMyShopData const& kMyShopData = kMyShopInfo.ShopInfo();
	int const& iOpenedGrade = kMyShopData.iGrade;
	bool const bPremiumService = pkPlayer->GetPremium().IsUserService(true) && (NULL != pkPlayer->GetPremium().GetType<S_PST_OpenmarketState>());
	if(true == kShop2.GetCheckState())
	{// 좋은 상점의 체크 버튼이 클릭 되었으나
		if(iOpenedGrade != MG_GOOD
			&& (false==bPremiumService && (E_FAIL == pkInven->GetFirstItem(99600010, ShopContractPos))))
		{// 상점을 열기 위한 아이템이 없다면, 이전에 체크되었던 버튼으로 되돌린다
			lwRevertMarketGrade();
			return;
		}
	}
	else if(true == kShop3.GetCheckState())
	{// 최고급 상점의 체크 버튼이 클릭 되었으나
		if(iOpenedGrade != MG_HISTORY
			&& (false==bPremiumService && (E_FAIL == pkInven->GetFirstItem(99600020, ShopContractPos))))
		{// 상점을 열기 위한 아이템이 없다면, 이전에 체크되었던 버튼으로 되돌린다
			lwRevertMarketGrade();
			return;
		}
	}
	
	switch(g_kMarketMgr.ShopState())
	{
	case MS_EDIT:
		{
			UpdateRegList();
		}break;
	}
}

void lwMarket::lwFixGradeRadioBtn()
{// 이전에 체크되었던 상점 버튼으로 체크 상태를 되돌린다.
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab.IsNil() )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{		
		return;
	}
	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{	
		return;
	}
	lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
	lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
	lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");

	if(kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil())
	{
		return;
	}
	// 화면에 보이는 체크 버튼을 모두 초기화
	kShop1.CheckState(false);
	kShop2.CheckState(false);
	kShop3.CheckState(false);
		
	switch(g_kMarketMgr.GetCurrentMarketGrade())
	{
	case MG_NORMAL:
		{
			kShop1.CheckState(true);
		}break;
	case MG_GOOD:
		{			
			kShop2.CheckState(true);
		}break;
	case MG_HISTORY:
		{
			kShop3.CheckState(true);
		}break;
	default:
		{
			_PgMessageBox("lwMarket::lwFixGradeRadioBtn()", "Unknown MarketGrade value");
		}break;	
	}
}

void lwMarket::lwRevertMarketGrade()
{// 이전에 체크되었던 상점 버튼으로 체크 상태를 되돌린다.
	lwUIWnd UIAuction = lwGetUIWnd("FRM_AUCTION_MAIN");
	if( UIAuction.IsNil() )
	{
		return;
	}

	lwUIWnd UITab = UIAuction.GetControl("FRM_SALE_REG");
	if( UITab.IsNil() )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{		
		return;
	}
	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{	
		return;
	}
	lwUIWnd	kShop1 = UITab.GetControl("CBTN_SHOP_STYLE1");
	lwUIWnd	kShop2 = UITab.GetControl("CBTN_SHOP_STYLE2");
	lwUIWnd	kShop3 = UITab.GetControl("CBTN_SHOP_STYLE3");

	if(kShop1.IsNil() || kShop2.IsNil() || kShop3.IsNil())
	{
		return;
	}
	// 화면에 보이는 체크 버튼을 모두 초기화 한후
	kShop1.CheckState(false);
	kShop2.CheckState(false);
	kShop3.CheckState(false);
	
	// 이전 체크 버튼의 상태로 되돌리고
	g_kMarketMgr.RevertMarketGrade();
	
	switch(g_kMarketMgr.GetCurrentMarketGrade())
	{// 되돌린 체크 버튼의 체크 상태를 true로 하여 화면에 보여줌
	case MG_NORMAL:
		{
			kShop1.CheckState(true);
		}break;
	case MG_GOOD:
		{			
			kShop2.CheckState(true);
		}break;
	case MG_HISTORY:
		{
			kShop3.CheckState(true);
		}break;
	default:
		{
			_PgMessageBox("lwMarket::lwRevertMarketGrade()", "Unknown MarketGrade value");
		}break;	
	}
}

void lwMarket::CallMarketOpenConfirm(BM::Stream kPacket)
{// 상점 오픈 최종 확인창을 띄운다 
	SMyShopInfo const& kShopInfo   = g_kMarketMgr.MyShopInfo();
	SMyShopData const& kShopData   = kShopInfo.ShopInfo();
	lwUIWnd kConfrmWnd			   = lwCallUI("SFRM_CONFIRM_OPEN",true);
	if(true == kConfrmWnd.IsNil())
	{
		return;
	}
	XUI::CXUI_Wnd* pkConfrmWnd     = kConfrmWnd.GetSelf();
	if(NULL == pkConfrmWnd) 
	{
		return;
	}
	XUI::CXUI_Wnd* pkBgwnd		   = pkConfrmWnd->GetControl(L"SFRM_BG_COLOR");
	if(NULL == pkBgwnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkMarketNameWnd = pkBgwnd->GetControl(L"SFRM_MARKET_NAME");
	if(NULL == pkMarketNameWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkGrademWnd	   = pkBgwnd->GetControl(L"SFRM_MARKET_GRADE");
	if(NULL == pkGrademWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkPointWnd	   = pkBgwnd->GetControl(L"SFRM_MARKET_POINT");
	if(NULL == pkPointWnd)
	{
		return;
	}
	
	// 현재  생성하려는 상점 이름을 표시하고
	pkMarketNameWnd->Text(kShopData.kMarketName);
	int const& iGrade = kShopData.iGrade;
	switch(iGrade)
	{// 등급을 표시하고
	case MG_NORMAL:	{ pkGrademWnd->Text(TTW(403014)); }break;
	case MG_GOOD:	{ pkGrademWnd->Text(TTW(403015)); }break;
	case MG_HISTORY:{ pkGrademWnd->Text(TTW(403016)); }break;
	default:		
		{
			_PgMessageBox("lwMarket::lwRequestShopStateChange()", "Unknown iGrade value"); 
		}break;
	}
	// 상점 포인트를 표시함
	BM::vstring kPoint;
	kPoint+=kShopData.iHP;
	pkPointWnd->Text(static_cast<std::wstring>(kPoint));	
	
	// 서버에 전달할 상점 정보 패킷을 상점 오픈 확인창에 저장한다.
	lwPacket klwPacket(&kPacket);
	kConfrmWnd.SetCustomDataAsPacket(klwPacket);		
}

void CheckRequestNormalShopOfflineTime()
{
	SMyShopInfo const& kMyShopInfo = g_kMarketMgr.MyShopInfo();
	SMyShopData const& kMyShopData = kMyShopInfo.ShopInfo();
	if(kMyShopData.iGrade!=MG_NORMAL)
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"FRM_SALE_REG");
	pkTmp = pkTmp ? pkTmp->GetControl(_T("EDT_SHOP_NAME")) : 0;
	if( !pkTmp )
	{
		return;
	}

	SPT_M_C_UM_NOTI_USE_MARKET_MODIFY_ITEM	Data(EMMIT_NO_MSG);
	Data.MarketName(kMyShopData.kMarketName);
	Data.MarketGrade(MG_NORMAL);

	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void lwMarket::lwMarketOpenConfirm()
{// 상점 오픈 최종 확인 버튼을 눌렀을때
	bool bIsFirstOpen = g_kMarketMgr.ShopState() == MS_CLOSE;
	if(MS_OPEN == g_kMarketMgr.ShopState())
	{
		return;
	}
	// EDIT 상태이거나, CLOSE 상태이면 상점을 열고
	lwUIWnd	kWnd = lwGetUIWnd("SFRM_CONFIRM_OPEN");
	
	lwPacket kPacket = kWnd.GetCustomDataAsPacket();
	if(kPacket.IsNil())
	{// 상점 오픈 확인창에 저장된 패킷을 서버로 전송한다.
		return;
	}
	
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	CheckRequestNormalShopOfflineTime();
	NETWORK_SEND(*kPacket());

	if(MS_EDIT != g_kMarketMgr.ShopState())
	{
		SMyShopInfo const& kMyShop = g_kMarketMgr.MyShopInfo();
		kRegItemContainer const kCont = kMyShop.TempRegList();

		kRegItemContainer::const_iterator atl_itor = kCont.begin();
		while( atl_itor != kCont.end() )
		{
			kRegItemContainer::mapped_type const& kArticle = atl_itor->second;

			//	이때는 바로 패킷을 보내버린다.
			SPT_C_M_UM_REQ_ARTICLE_REG	Data;
			Data.Pos(kArticle.Pos());
			Data.CostType(kArticle.CostType());
			Data.ArticleCost(kArticle.ArticleCost());
			Data.ArticleType(kArticle.ArticleType());
			Data.SellMoney(kArticle.SellMoney());

			BM::Stream	kPacket2;
			Data.WriteToPacket(kPacket2);
			NETWORK_SEND(kPacket2);

			++atl_itor;
		}
		g_kMarketMgr.ClearMyShopInfo();
	}
	if( bIsFirstOpen )
	{
		SPT_C_M_UM_REQ_MARKET_MODIFY_STATE	Data;
		Data.MarketState(MS_OPEN);
		BM::Stream kPacket;
		Data.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket);
	}
	g_kMarketMgr.IsSendOK(true);
}

void lwMarket::lwMarketOpenCancel()
{
	if(MS_EDIT == g_kMarketMgr.ShopState())
	{// 수정하기 상태에서 취소했다면, 깜빡임 표시를 함
		lwUIWnd kAuctionMain = lwGetUIWnd("FRM_AUCTION_MAIN");
		if( true == kAuctionMain.IsNil() )
		{
			return;
		}
		lwUIWnd kSaleReg = kAuctionMain.GetControl("FRM_SALE_REG");
		if( true == kSaleReg.IsNil() )
		{
			return;
		}
		lwUIWnd kBlinkBtn = kSaleReg.GetControl("AUCTION_BLINK_BTN");
		if(true ==  kBlinkBtn.IsNil())
		{
			return;
		}
		kBlinkBtn.Visible(true);
		kBlinkBtn.SetCustomData<int>(20);
	}
}
void lwMarket::lwMarKetGradeRadioBtnInit()
{// 상점 등급 라디오 버튼을 현재 저장되어있는 상점 등급으로 표시한다.
	SMyShopInfo const& kShopInfo = g_kMarketMgr.MyShopInfo();
	SMyShopData const& kShopData = kShopInfo.ShopInfo();

	int const iCurGrade = kShopData.iGrade;
	g_kMarketMgr.SetCurrentMarketGrade(iCurGrade);
	g_kMarketMgr.SetPrevMarketGrade(iCurGrade);		
}


//---------------------------------------------------------------------------------------------------------------------------------
//	Market Shop 분리 2009. 4. 20
//---------------------------------------------------------------------------------------------------------------------------------
void lwMarket::lwRequestShopList()
{
	BM::Stream kPacket(PT_C_M_UM_REQ_BEST_MARKET_LIST);
	NETWORK_SEND(kPacket);
}

void lwMarket::lwRequestEnterShop()
{
	lwSelectShopInOK(false);
}

void lwMarket::lwClearSearchInfoUI(lwUIWnd UITab, int const DefaultLankTT, int const DefaultClassTT)
{
	lwUIWnd kTemp = UITab.GetControl("CBTN_ITEM");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.CheckState(true);

	kTemp = UITab.GetControl("CBTN_SELLER");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.CheckState(false);

	kTemp = UITab.GetControl("EDT_SEARCH_VALUE");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.SetEditText("");

	kTemp = UITab.GetControl("EDT_LV_LOW_LIMIT");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.SetEditText("");

	kTemp = UITab.GetControl("EDT_LV_HIGH_LIMIT");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.SetEditText("");

	kTemp = UITab.GetControl("SFRM_ITEM_LANK");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.SetStaticText(MB(TTW(DefaultLankTT)));
	kTemp.SetCustomData<int>(IG_MAX);

	kTemp = UITab.GetControl("SFRM_ITEM_GRADE");
	if( false == kTemp.IsNil() )
	{
		kTemp.SetStaticText(MB(TTW(DefaultLankTT)));
		kTemp.SetCustomData<int>(IDG_MAX);
	}

	kTemp = UITab.GetControl("SFRM_ITEM_CLASS");
	if( kTemp.IsNil() )
	{
		return;
	}
	kTemp.SetStaticText(MB(TTW(DefaultClassTT)));
	kTemp.SetCustomData<int>(0);

	kTemp = UITab.GetControl("CBTN_ONLY_CASH");
	if(!kTemp.IsNil())
	{
		kTemp.CheckState(false);		
	}
	lwSelectItemType(false, -1);
}

void lwMarket::lwDrawShopListUpdate(lwUIWnd UISelf)
{
	int const iState = UISelf.GetCustomData<int>();
	lwTabBtnChange(iState);
	switch(iState)
	{
	case ETS_SHOPLIST:
		{
			lwRequestShopList();
		}break;
	case ETS_SEARCHLIST:
		{
			SPT_C_M_UM_REQ_MARKET_QUERY Key = g_kMKTSearchMgr.SearchKey();
			RequestSearchItem(Key);
		}break;
	default:{}break;
	}
}

void lwMarket::lwSelectItemType(int const Type, int const Mode)
{
	XUI::CXUI_Wnd* pTemp = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( !pTemp )
	{
		return;
	}

	pTemp = pTemp->GetControl(L"FRM_SEARCH");
	if( !pTemp )
	{
		return;
	}

	pTemp = pTemp->GetControl(L"EDT_SEARCH_VALUE");
	if( !pTemp )
	{
		return;
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pTemp);
	if( !pEdit )
	{
		return;
	}

	switch( Type )
	{
	case 91:
	case 92:
	case 93:
		{
			g_kMKTSearchMgr.Type(SMT_CASH + Type - 91);
			pEdit->IsOnlyNum(1);
		}break;
	default:
		{
			g_kMKTSearchMgr.Type(Type);
			g_kMKTSearchMgr.TypeNode(Mode);
			pEdit->IsOnlyNum(0);
		}break;
	}
}

void lwMarket::lwSearchBtnState(lwUIWnd UISelf)
{
	if( g_kMarketMgr.bIsPushSearchBtn() )
	{
		if( g_kMarketMgr.UpdateTimeSearchBtn() < g_pkApp->GetAccumTime() )
		{
			g_kMarketMgr.bIsPushSearchBtn(false);
			UISelf.Disable(false);
		}
	}
}

void lwMarket::lwPrevShopPageInfo()
{
	if( g_kMKTShopMgr.PrevPage() )
	{
		DrawShopList();
	}
}

void lwMarket::lwNextShopPageInfo()
{
	if( g_kMKTShopMgr.NextPage() )
	{
		DrawShopList();
	}
}

void lwMarket::lwDrawShopPageInfo(lwUIWnd UISelf)
{
	UISelf.SetStaticTextW(lwWString(g_kMKTShopMgr.GetPageString()));
}

void lwMarket::DrawShopList()
{
	if( 0 > g_kMKTShopMgr.Page() )
	{
		g_kMKTShopMgr.Page(0);
	}

	if( g_kMKTShopMgr.Page() >= g_kMKTShopMgr.TotalPage() && 0 != g_kMKTShopMgr.TotalPage() )
	{
		g_kMKTShopMgr.Page(g_kMKTShopMgr.TotalPage() - 1);
	}

	XUI::CXUI_Wnd* pMarketUI = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( !pMarketUI ){ return; }

	XUI::CXUI_Wnd* pSearchUI = pMarketUI->GetControl(L"FRM_SEARCH");
	if( !pSearchUI ){ return; }

	XUI::CXUI_Wnd* pListGroup = pSearchUI->GetControl(L"FRM_SHOP_LIST_GROUP");
	if( !pListGroup ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGroup->GetControl(L"LIST_AUCTION_ITEMS"));
	if( !pList ){ return; }

	PgMarketShopMgr::CONT_SHOP_ITEM	kList;
	g_kMKTShopMgr.GetPageInfo(kList);
	if( !kList.empty() )
	{
		int const iListCount = pList->GetTotalItemCount();
		if( iListCount < kList.size() )
		{
			for(int i = iListCount; i < kList.size(); ++i)
			{
				pList->AddItem(L"");
			}
		}
		else if( iListCount > kList.size() )
		{
			for(int i = iListCount; i > kList.size(); --i)
			{
				pList->DeleteItem(pList->FirstItem());
			}
		}

		XUI::SListItem* pUIItem = pList->FirstItem();
		PgMarketShopMgr::CONT_SHOP_ITEM::const_iterator c_iter = kList.begin();
		while( c_iter != kList.end() )
		{
			if( pUIItem )
			{
				ClearShopListItem(pUIItem->m_pWnd);
				DrawShopListItem(pUIItem->m_pWnd, *c_iter);
				pUIItem = pList->NextItem(pUIItem);
			}
			++c_iter;
		}
	}
	else
	{
		if( 0 == g_kMKTSearchMgr.Page() )
		{
			pList->DeleteAllItem();
		}
	}
}

void lwMarket::ClearShopListItem(XUI::CXUI_Wnd* pItemWnd)
{
	if( !pItemWnd )
	{
		return;
	}

	for(int i = MG_GOOD; i < MG_END; ++i)
	{
		BM::vstring vStr("FRM_ITEM_BGTYPE");
		vStr += i;

		XUI::CXUI_Wnd* pGradeWnd = pItemWnd->GetControl(vStr);
		if( pGradeWnd )
		{
			pGradeWnd->Visible(false);
		}
	}
}

void lwMarket::DrawShopListItem(XUI::CXUI_Wnd* pItemWnd, SMARKET_KEY const& kShopInfo)
{
	if( !pItemWnd )
	{
		return;
	}

	pItemWnd->OwnerGuid(kShopInfo.kOwnerId);

	//step 1. 상점 등급 표시
	if( MG_NORMAL != kShopInfo.iGrade )
	{
		BM::vstring vStr("FRM_ITEM_BGTYPE");
		vStr += kShopInfo.iGrade;

		XUI::CXUI_Wnd* pGradeWnd = pItemWnd->GetControl(vStr);
		if( pGradeWnd )
		{
			pGradeWnd->Visible(true);
		}
	}

	//step 2. 상점이름 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_NAME", kShopInfo.kMarketName);
	
	//step 3. 상점등급 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_LANK", TTW(403014 + kShopInfo.iGrade));
	
	//step 4. 역사포인트 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_HP", BM::vstring(kShopInfo.iHP));

	//step 5. 판매자 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_SELLER", kShopInfo.kCharName);
}

void lwMarket::DrawEnterShopItemList()
{
	XUI::CXUI_Wnd* pMarketUI = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( !pMarketUI ){ return; }

	XUI::CXUI_Wnd* pSearchUI = pMarketUI->GetControl(L"FRM_SEARCH");
	if( !pSearchUI ){ return; }

	XUI::CXUI_Wnd* pListGroup = pSearchUI->GetControl(L"FRM_SHOP_ITEM_GROUP");
	if( !pListGroup ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGroup->GetControl(L"LIST_AUCTION_ITEMS"));
	if( !pList ){ return; }

	PgMarketShopMgr::CONT_ENTER_SHOP_ITEM const& kList = g_kMKTShopMgr.GetEnterShopItemList();
	if( !kList.empty() )
	{
		int const iListCount = pList->GetTotalItemCount();
		if( iListCount < kList.size() )
		{
			for(int i = iListCount; i < kList.size(); ++i)
			{
				pList->AddItem(L"");
			}
		}
		else if( iListCount > kList.size() )
		{
			for(int i = iListCount; i > kList.size(); --i)
			{
				pList->DeleteItem(pList->FirstItem());
			}
		}

		XUI::SListItem* pUIItem = pList->FirstItem();
		PgMarketShopMgr::CONT_ENTER_SHOP_ITEM::const_iterator c_iter = kList.begin();
		while( c_iter != kList.end() )
		{
			if( pUIItem )
			{
				DrawEnterShopItem(pUIItem->m_pWnd, *c_iter);
				pUIItem = pList->NextItem(pUIItem);
			}
			++c_iter;
		}
	}
	else
	{
		pList->DeleteAllItem();
	}
}

void lwMarket::DrawEnterShopItem(XUI::CXUI_Wnd* pItemWnd, SUserMarketArticleInfo const& kItemInfo)
{
	if( !pItemWnd )
	{
		return;
	}

	pItemWnd->SetCustomData(&kItemInfo.kItemGuId, sizeof(kItemInfo.kItemGuId));

	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
		}break;
	default :
		{
			if( kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				bIsNonCashItem = false;
			}
		}break;
	}

	if( bIsNonCashItem )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
		if (!pItemDef)
		{
			return;
		}

		//step 1. 아이콘 설정
		lwMarketUtil::SetUIIconToBaseItem(pItemWnd, L"IMG_ICON", kItemInfo.kItem);

		//step 2. 이름
		lwMarketUtil::SetUITextToItemName(pItemWnd, L"FRM_ITEM_NAME", kItemInfo.kItem.ItemNo(), kItemInfo.kItem.EnchantInfo(), true);
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_DISPLAY_GRADE", lwChangeIndexToSysEmoFont(pItemDef->GetAbil(AT_ITEM_DISPLAY_GRADE)).GetWString());

		//step 3. 레벨
		BM::vstring vStr(TTW(224));
		vStr << (pItemDef->GetAbil(AT_LEVELLIMIT));
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_LEVEL", vStr);
	}
	else
	{
		bool const bIsCashToMoney = kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;

		float fMinDest, fMinSour, fRate;
		lwMarketUtil::CalcMoneyChangeRate( kItemInfo.kItem.Count(), (bIsCashToMoney)?(kItemInfo.kArticleCost / GOLD_MONEY_VALUE):(kItemInfo.kArticleCost), fMinDest, fMinSour, fRate );
		std::wstring kMoneyText = TTW((bIsCashToMoney)?(403256):(403257));

		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, L"[%s]%d %s (%s : %s)"
			, kMoneyText.c_str()
			, kItemInfo.kItem.Count()
			, kMoneyText.c_str()
			, lwMarketUtil::FloatConvertToStr(fMinDest, 4).c_str()
			, lwMarketUtil::FloatConvertToStr(fMinSour, 4).c_str() );

		//step 1. 아이콘 설정
		lwMarketUtil::SetUIIconToItemNo(pItemWnd, L"IMG_ICON", (bIsCashToMoney)?(MARKET_ARTICLE_CASHNO):(MARKET_ARTICLE_MONEYNO), SEnchantInfo(), 1, BM::PgPackedTime());

		//step 2. 이름
		std::wstring kItemName(L"{C=");
		kItemName += TTW(4106);
		kItemName += L"/}";
		kItemName += szTemp;
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_NAME", kItemName, true);
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_DISPLAY_GRADE", std::wstring(L""));
		//step 3. 레벨
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_LEVEL", std::wstring(L"-"));
	}

	//step 4. 가격
	lwMarketUtil::SetUITextToItemCost(pItemWnd, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.kCostType, kItemInfo.kArticleCost);
}

void lwMarket::OnClickEnterShopBuyItem(lwUIWnd UITab)
{
	if( UITab.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* pListGroup = UITab.GetSelf()->GetControl(L"FRM_SHOP_ITEM_GROUP");
	if( !pListGroup ){ return; };

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGroup->GetControl(L"LIST_AUCTION_ITEMS"));
	if( !pList ){ return; };

	XUI::SListItem* pItem = pList->FirstItem();
	while( pItem && pItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pClick = pItem->m_pWnd->GetControl(L"FRM_ITEM_CLICK");
		if( !pClick )
		{
			return;
		}

		if( pClick->Visible() )
		{
			lwOnCallItemBuyUI(lwUIWnd(pItem->m_pWnd));
			break;
		}
		pItem = pList->NextItem(pItem);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------
//	Market Search 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
void lwMarket::lwSearchItemList(lwUIWnd UITab, lwUIWnd UISelf)
{
	if( g_kMarketMgr.bIsPushSearchBtn() )
	{
		return;
	}

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	SPT_C_M_UM_REQ_MARKET_QUERY	Data;
	Data.SearchItemModeType(static_cast<eSearchModeType>(g_kMKTSearchMgr.Type()));
	Data.SearchItemMode(g_kMKTSearchMgr.TypeNode());

	// step 1. 검색 타입
	lwUIWnd kTemp = UITab.GetControl("CBTN_ITEM");
	if( kTemp.IsNil() )
	{
		return;
	}

	if( kTemp.GetCheckState() )
	{
		Data.SearchNameType(ST_ITEM);
	}
	else
	{
		kTemp = UITab.GetControl("CBTN_SELLER");
		if( kTemp.IsNil() )
		{
			return;
		}

		if( !kTemp.GetCheckState() )
		{
			return;
		}
		Data.SearchNameType(ST_SELLER);
	}

	//step 2. 검색어
	kTemp = UITab.GetControl("EDT_SEARCH_VALUE");
	if( kTemp.IsNil() )
	{
		return;
	}

	if( SMT_CASH == Data.SearchItemModeType()
	||  SMT_MONEY == Data.SearchItemModeType()
	||  SMT_PET ==  Data.SearchItemModeType() )
	{
		Data.SearchItemMode(_wtoi(kTemp.GetEditText()().c_str()));
	}
	else
	{
		Data.SearchName(kTemp.GetEditText()());
		if( Data.SearchName().size() > MAX_OPENMARKET_SEARCH_STRING_LEN)
		{
			wchar_t szTemp[MAX_PATH] = {0, };
			swprintf_s(szTemp, MAX_PATH, TTW(7101).c_str(), MAX_OPENMARKET_SEARCH_STRING_LEN);
			lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
			return;
		}
	}

	//step 3. 상하 레벨 제한
	kTemp = UITab.GetControl("EDT_LV_LOW_LIMIT");
	if( kTemp.IsNil() )
	{
		return;
	}
	Data.LevelMin(_wtoi(kTemp.GetEditText()().c_str()));

	kTemp = UITab.GetControl("EDT_LV_HIGH_LIMIT");
	if( kTemp.IsNil() )
	{
		return;
	}
	Data.LevelMax(_wtoi(kTemp.GetEditText()().c_str()));

	//step 4. 아이템 등급
	kTemp = UITab.GetControl("SFRM_ITEM_LANK");
	if( kTemp.IsNil() )
	{
		return;
	}
	E_ITEM_GRADE kItemGrade = static_cast<E_ITEM_GRADE>( kTemp.GetCustomData<int>() );
	Data.ItemGrade(kItemGrade);
	
	LOCAL_MGR::NATION_CODE const code = (LOCAL_MGR::NATION_CODE)g_kLocal.ServiceRegion();
	switch(code)
	{
	case LOCAL_MGR::NC_DEVELOP:
		{
			//step 4-1. 아이템 등급
			kTemp = UITab.GetControl("SFRM_ITEM_GRADE");
			if( kTemp.IsNil() )
			{
				return;
			}
			Data.ItemDisplayGrade(kTemp.GetCustomData<int>());
		}break;
	default:
		{
			Data.ItemDisplayGrade(IDG_MAX);
		}break;
	}

	//step 5. 착용 직업 제한
	kTemp = UITab.GetControl("SFRM_ITEM_CLASS");
	if( kTemp.IsNil() )
	{
		return;
	}
	Data.ClassLimit(g_kMarketMgr.GetClassLimit(kTemp.GetCustomData<int>()));

	kTemp = UITab.GetControl("CBTN_ONLY_CASH");
	if(!kTemp.IsNil())
	{
		Data.CostType(kTemp.GetCheckState());		
	}
	else
	{
		Data.CostType(false);
	}
	
	//step 8. 페이지 정보
	Data.ArticleIndex(0);
	g_kMKTSearchMgr.Page(0);

	//step 9. 전송
	RequestSearchItem(Data);
	g_kMarketMgr.UpdateTimeSearchBtn(g_pkApp->GetAccumTime() + 5.0f);
	g_kMarketMgr.bIsPushSearchBtn(true);
	UISelf.Disable(true);
	lwTabChangeState(ETS_SEARCHLIST);
}

void lwMarket::lwOnOverSearchListItem(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
	if( !pList ){ return; }

	if( pList->GetTotalItemCount() )
	{
		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem && pItem->m_pWnd )
		{
			XUI::CXUI_Wnd* pOver = pItem->m_pWnd->GetControl(L"FRM_ITEM_OVER");
			pOver->Visible(false);
			pItem = pList->NextItem(pItem);
		}
	}

	XUI::CXUI_Wnd* pMyOver = pSelf->GetControl(L"FRM_ITEM_OVER");
	if( pMyOver )
	{
		pMyOver->Visible(true);
	}
}

void lwMarket::lwOnCallSearchListPopUpMenu(lwUIWnd UISelf)
{
	if( lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true) )
	{//마우스 좌클릭 + Shift = 검색란에 아이템 이름 등록, 팝업은 필요없음
		return;
	}

	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	if( pSelf->OwnerGuid().IsNull() || !pSelf->GetCustomDataSize() )
	{
		return;
	}

	BM::GUID kGuid;
	pSelf->GetCustomData(&kGuid, sizeof(kGuid));

	XUI::CXUI_Wnd* pPopupUI = XUIMgr.Get(L"SFRM_SEARCH_ITEM_POPUP");
	if( !pPopupUI || pPopupUI->IsClosed() )
	{
		pPopupUI = XUIMgr.Call(L"SFRM_SEARCH_ITEM_POPUP");
		if( !pPopupUI )
		{
			return;
		}
	}

	pPopupUI->OwnerGuid(pSelf->OwnerGuid());
	pPopupUI->SetCustomData(&kGuid, sizeof(kGuid));
	pPopupUI->Location(XUIMgr.MousePos().x + 5, XUIMgr.MousePos().y + 5);
	pPopupUI->VOnCall();
}

void lwMarket::lwSelectBuyOK(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	int iBuyCount = 0;
	if( pSelf->GetCustomDataSize() )
		pSelf->GetCustomData(&iBuyCount, sizeof(iBuyCount));

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent || pParent->OwnerGuid().IsNull() || !pParent->GetCustomDataSize() )
	{
		return;
	}

	BM::GUID kGuid;
	pParent->GetCustomData(&kGuid, sizeof(kGuid));

	SUserMarketArticleInfo	kItemInfo;
	switch( g_kMarketMgr.TabState() )
	{
	case ETS_SEARCHLIST:
		{
			if( !g_kMKTSearchMgr.GetItemInfo(pParent->OwnerGuid(), kGuid, kItemInfo) )
			{
				return;	
			}
		}break;
	case ETS_SHOPINLIST:
		{
			if( !g_kMKTShopMgr.GetEnterShopItemInfo(kGuid, kItemInfo) )
			{
				return;	
			}
		}break;
	default: return;
	}

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	__int64 kHaveCost = 0;
	switch( kItemInfo.kCostType )
	{
	case RCT_GOLD:{	kHaveCost = pPlayer->GetAbil64(AT_MONEY);	}break;
	case RCT_CASH:{ kHaveCost = pPlayer->GetAbil64(AT_CASH);	}break;
	}

	int iItemCost = kItemInfo.kArticleCost;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
	if( pItemDef )
	{
		if( pItemDef->IsAmountItem() )
		{
			iItemCost = iItemCost * iBuyCount;
		}
	}
	
	if( iItemCost > kHaveCost )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403078 + kItemInfo.kCostType, true);
		return;
	}

	if( !bExistEmptyInv(kItemInfo.kItem.ItemNo()) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 20023, true);
		return;
	}

	RequestBuySelectItem(pParent->OwnerGuid(), kGuid, kItemInfo.kItem.Count());
}

void lwMarket::lwSelectShopInOK(bool bSearch)
{
	XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( !pMainUI ){ return; };

	XUI::CXUI_Wnd* pPageUI = pMainUI->GetControl(L"FRM_SEARCH");
	if( !pPageUI ){ return; };

	XUI::CXUI_Wnd* pListGroup = NULL;
	if( !bSearch )
	{
		pListGroup = pPageUI->GetControl(L"FRM_SHOP_LIST_GROUP");
	}
	else
	{
		pListGroup = pPageUI->GetControl(L"FRM_SEARCH_LIST_GROUP");
	}
	if( !pListGroup ){ return; };

	XUI::CXUI_Wnd* pItemGroup = pPageUI->GetControl(L"FRM_SHOP_ITEM_GROUP");
	if( !pItemGroup ){ return; };

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGroup->GetControl(L"LIST_AUCTION_ITEMS"));
	if( !pList ){ return; };

	XUI::SListItem* pItem = pList->FirstItem();
	while( pItem && pItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pOnClick = pItem->m_pWnd->GetControl(L"FRM_ITEM_CLICK");
		if( !pOnClick )
		{
			return;
		}

		if( pOnClick->Visible() )
		{
			pItemGroup->ClearCustomData();
			pItemGroup->SetCustomData(&pItem->m_pWnd->OwnerGuid(), sizeof(pItem->m_pWnd->OwnerGuid()));

			SPT_C_M_UM_REQ_MARKET_ARTICLE_QUERY	Data;
			Data.MarketGuId(pItem->m_pWnd->OwnerGuid());
			
			BM::Stream	kPacket;
			Data.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket);

			g_kMKTShopMgr.kEnterShopGuid(pItem->m_pWnd->OwnerGuid());
			break;
		}
		pItem = pList->NextItem(pItem);
	}
}

void lwMarket::lwOnCallItemBuyUI(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}
	BM::GUID kGuid, kShopGuid;
	SUserMarketArticleInfo	kItemInfo;
	if( pSelf->GetCustomDataSize() )
	{
		pSelf->GetCustomData(&kGuid, sizeof(kGuid));
	}

	switch( g_kMarketMgr.TabState() )
	{
	case ETS_SEARCHLIST:
		{
			kShopGuid = pSelf->OwnerGuid();
			if( !g_kMKTSearchMgr.GetItemInfo(kShopGuid, kGuid, kItemInfo) )
			{
				return;
			}
		}break;
	case ETS_SHOPINLIST:
		{
			kShopGuid = g_kMKTShopMgr.kEnterShopGuid();
			if( !g_kMKTShopMgr.GetEnterShopItemInfo(kGuid, kItemInfo) )
			{
				return;
			}
		}break;
	default: return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
	if( pItemDef )
	{
		int	const iCantAbil = pItemDef->GetAbil(AT_ATTRIBUTE);
		if(iCantAbil && ICMET_Cant_Auction == (iCantAbil & ICMET_Cant_Auction))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403099, true);
			return;
		}

		if( pItemDef->IsAmountItem() 
			&& 1 < kItemInfo.kItem.Count() 
			&& MARKET_ARTICLE_CASHNO != kItemInfo.kItem.ItemNo()
			&& MARKET_ARTICLE_MONEYNO != kItemInfo.kItem.ItemNo() )
		{
			SCalcInfo kInfo;
			kInfo.eCallType = CCT_MARKET_BUY;
			kInfo.iBasePrice = kItemInfo.kArticleCost;
			kInfo.iItemNo = kItemInfo.kItem.ItemNo();
			kInfo.iMaxValue = kItemInfo.kItem.Count();
			kInfo.kGuid = kItemInfo.kItemGuId;
			kInfo.kGuid1 = kShopGuid;
			kInfo.cCostType = kItemInfo.kCostType;
			CallCalculator(kInfo);
			return;
		}
	}
	CallItemBuyUI(kShopGuid, kGuid, kItemInfo.kItem.Count());
}

void lwMarket::lwPrevSearchPageInfo()
{
	if( g_kMKTSearchMgr.PrevPage() )
	{
		SPT_C_M_UM_REQ_MARKET_QUERY Key = g_kMKTSearchMgr.SearchKey();
		Key.ArticleIndex( g_kMKTSearchMgr.Page() * g_kMKTSearchMgr.GetPageSlotMax() );
		RequestSearchItem(Key);
	}
}

void lwMarket::lwNextSearchPageInfo()
{
	if( g_kMKTSearchMgr.NextPage() )
	{
		SPT_C_M_UM_REQ_MARKET_QUERY Key = g_kMKTSearchMgr.SearchKey();
		Key.ArticleIndex( g_kMKTSearchMgr.Page() * g_kMKTSearchMgr.GetPageSlotMax() );
		RequestSearchItem(Key);
	}
}

void lwMarket::lwDrawSearchPageInfo(lwUIWnd UISelf)
{
	UISelf.SetStaticTextW(lwWString(g_kMKTSearchMgr.GetPageString()));
}

void lwMarket::RequestBuySelectItem(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iItemCount)
{
	SPT_C_M_UM_REQ_ARTICLE_BUY	kData;
	kData.MarketGuid(kShopGuid);
	kData.ArticleGuid(kItemGuid);
	kData.BuyNum(iItemCount);
	
	BM::Stream	kPacket;
	kData.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket);
}

void lwMarket::CallItemBuyUI(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iBuyCount)
{
	XUI::CXUI_Wnd* pBuyUI = XUIMgr.Call(L"SFRM_OPEN_MARKET_BUY");
	if( !pBuyUI )
	{
		return;
	}

	pBuyUI->OwnerGuid(kShopGuid);
	pBuyUI->SetCustomData(&kItemGuid, sizeof(kItemGuid));

	SUserMarketArticleInfo	kItemInfo;

	switch( g_kMarketMgr.TabState() )
	{
	case ETS_SEARCHLIST:
		{
			if( !g_kMKTSearchMgr.GetItemInfo(kShopGuid, kItemGuid, kItemInfo) )
			{
				return;	
			}
		}break;
	case ETS_SHOPINLIST:
		{
			if( !g_kMKTShopMgr.GetEnterShopItemInfo(kItemGuid, kItemInfo) )
			{
				return;	
			}
		}break;
	default: return;
	}

	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
		}break;
	default :
		{
			if( kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				bIsNonCashItem = false;
			}
		}break;
	}

	if( bIsNonCashItem )
	{
	//step 1. 선택 아이템 표시
	lwMarketUtil::SetUIIconToBaseItem(pBuyUI, L"IMG_ICON", kItemInfo.kItem);
	
	//step 2. 선택 아이템 가격 표시
	__int64 iCost = kItemInfo.kArticleCost;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
	if( pItemDef )
	{
		if( pItemDef->IsAmountItem() )
		{
			iCost *= iBuyCount;
		}
	}
		lwMarketUtil::SetUITextToItemCost(pBuyUI, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.kCostType, iCost);
	}
	else
	{
		bool const bIsCashToMoney = kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;

		//float fMinDest, fMinSour, fRate;
		//lwMarketUtil::CalcMoneyChangeRate( kItemInfo.kItem.Count(), (bIsCashToMoney)?(kItemInfo.kArticleCost / GOLD_MONEY_VALUE):(kItemInfo.kArticleCost), fMinDest, fMinSour, fRate );
		//std::wstring kMoneyText = TTW((bIsCashToMoney)?(403256):(403257));

		//wchar_t szTemp[MAX_PATH] = {0,};
		//swprintf_s(szTemp, MAX_PATH, L"[%s]%d %s (%s : %s)"
		//	, kMoneyText.c_str()
		//	, kItemInfo.kItem.Count()
		//	, kMoneyText.c_str()
		//	, lwMarketUtil::FloatConvertToStr(fMinDest, 4).c_str()
		//	, lwMarketUtil::FloatConvertToStr(fMinSour, 4).c_str() );

		//step 1. 선택 아이템 표시
		lwMarketUtil::SetUIIconToItemNo(pBuyUI, L"IMG_ICON", (bIsCashToMoney)?(MARKET_ARTICLE_CASHNO):(MARKET_ARTICLE_MONEYNO), SEnchantInfo(), 1, BM::PgPackedTime());

		//step 2. 선택 아이템 가격 표시
		lwMarketUtil::SetUITextToItemCost(pBuyUI, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.kCostType, kItemInfo.kArticleCost);
	}

	XUI::CXUI_Wnd* pBTNBuy = pBuyUI->GetControl(L"BTN_BUY");
	if( pBTNBuy )
	{
		pBTNBuy->SetCustomData(&iBuyCount, sizeof(iBuyCount));
	}
}

void lwMarket::RequestSearchItem(SPT_C_M_UM_REQ_MARKET_QUERY& kSearchKey)
{
	BM::Stream kPacket;
	kSearchKey.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket);
	g_kMKTSearchMgr.SearchKey(kSearchKey);
}

void lwMarket::DrawSearchList()
{
	if( 0 > g_kMKTSearchMgr.Page() )
	{
		g_kMKTSearchMgr.Page(0);
	}

	if( g_kMKTSearchMgr.Page() >= g_kMKTSearchMgr.TotalPage() && 0 != g_kMKTSearchMgr.TotalPage() )
	{
		g_kMKTSearchMgr.Page(g_kMKTSearchMgr.TotalPage() - 1);
	}

	XUI::CXUI_Wnd* pMarketUI = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( !pMarketUI ){ return; }

	XUI::CXUI_Wnd* pSearchUI = pMarketUI->GetControl(L"FRM_SEARCH");
	if( !pSearchUI ){ return; }

	XUI::CXUI_Wnd* pListGroup = pSearchUI->GetControl(L"FRM_SEARCH_LIST_GROUP");
	if( !pListGroup ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGroup->GetControl(L"LIST_AUCTION_ITEMS"));
	if( !pList ){ return; }

	PgMarKetSearchMgr::CONT_SERACH_ITEM kList;
	g_kMKTSearchMgr.GetPageInfo(kList);
	if( !kList.empty() )
	{
		int const iListCount = pList->GetTotalItemCount();
		if( iListCount < kList.size() )
		{
			for(int i = iListCount; i < kList.size(); ++i)
			{
				pList->AddItem(L"");
			}
		}
		else if( iListCount > kList.size() )
		{
			for(int i = iListCount; i > kList.size(); --i)
			{
				pList->DeleteItem(pList->FirstItem());
			}
		}

		XUI::SListItem* pUIItem = pList->FirstItem();
		PgMarKetSearchMgr::CONT_SERACH_ITEM::const_iterator c_iter = kList.begin();
		while( c_iter != kList.end() )
		{
			if( pUIItem )
			{
				ClearSearchListItem(pUIItem->m_pWnd);
				DrawSearchListItem(pUIItem->m_pWnd, c_iter->first, c_iter->second);
				pUIItem = pList->NextItem(pUIItem);
			}
			++c_iter;
		}
	}
	else
	{
		if( 0 == g_kMKTSearchMgr.Page() )
		{
			pList->DeleteAllItem();
		}
	}
}

void lwMarket::ClearSearchListItem(XUI::CXUI_Wnd* pItemWnd)
{
	if( !pItemWnd )
	{
		return;
	}

	for(int i = MG_GOOD; i < MG_END; ++i)
	{
		BM::vstring vStr("FRM_ITEM_BGTYPE");
		vStr += i;

		XUI::CXUI_Wnd* pGradeWnd = pItemWnd->GetControl(vStr);
		if( pGradeWnd )
		{
			pGradeWnd->Visible(false);
		}
	}	
}

void lwMarket::DrawSearchListItem(XUI::CXUI_Wnd* pItemWnd, SMARKET_KEY const& kShopInfo, SUserMarketArticleInfo const& kItemInfo)
{
	if( !pItemWnd )
	{
		return;
	}

	pItemWnd->OwnerGuid(kShopInfo.kOwnerId);
	pItemWnd->SetCustomData(&kItemInfo.kItemGuId, sizeof(kItemInfo.kItemGuId));

	//step 1. 상점 등급 표시
	if( MG_NORMAL != kShopInfo.iGrade )
	{
		BM::vstring vStr("FRM_ITEM_BGTYPE");
		vStr += kShopInfo.iGrade;

		XUI::CXUI_Wnd* pGradeWnd = pItemWnd->GetControl(vStr);
		if( pGradeWnd )
		{
			pGradeWnd->Visible(true);
		}
	}

	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_KOREA:
		{
		}break;
	default :
		{
			if( kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				bIsNonCashItem = false;
			}
		}break;
	}

	if( bIsNonCashItem )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
		if (!pItemDef)
		{
			return;
		}

		//step 2. 검색 아이템 표시
		lwMarketUtil::SetUIIconToBaseItem(pItemWnd, L"IMG_ICON", kItemInfo.kItem);

		//step 3. 검색 아이템 이름 표시
		lwMarketUtil::SetUITextToItemName(pItemWnd, L"FRM_ITEM_NAME", kItemInfo.kItem.ItemNo(), kItemInfo.kItem.EnchantInfo(), true);
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_DISPLAY_GRADE", lwChangeIndexToSysEmoFont(pItemDef->GetAbil(AT_ITEM_DISPLAY_GRADE)).GetWString());

		//step 4. 검색 아이템 요구 레벨 표시
		BM::vstring vStr(TTW(224));
		vStr << (pItemDef->GetAbil(AT_LEVELLIMIT));
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_LEVEL", vStr);
	}
	else
	{
		bool const bIsCashToMoney = kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;

		float fMinDest, fMinSour, fRate;
		lwMarketUtil::CalcMoneyChangeRate( kItemInfo.kItem.Count(), (bIsCashToMoney)?(kItemInfo.kArticleCost / GOLD_MONEY_VALUE):(kItemInfo.kArticleCost), fMinDest, fMinSour, fRate );
		std::wstring kMoneyText = TTW((bIsCashToMoney)?(403256):(403257));

		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, L"[%s]%d %s (%s : %s)"
			, kMoneyText.c_str()
			, kItemInfo.kItem.Count()
			, kMoneyText.c_str()
			, lwMarketUtil::FloatConvertToStr(fMinDest, 4).c_str()
			, lwMarketUtil::FloatConvertToStr(fMinSour, 4).c_str() );

		//step 2. 검색 아이템 표시
		lwMarketUtil::SetUIIconToItemNo(pItemWnd, L"IMG_ICON", (bIsCashToMoney)?(MARKET_ARTICLE_CASHNO):(MARKET_ARTICLE_MONEYNO), SEnchantInfo(), 1, BM::PgPackedTime());

		//step 3. 검색 아이템 이름 표시
		std::wstring kItemName(L"{C=");
		kItemName += TTW(4106);
		kItemName += L"/}";
		kItemName += szTemp;
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_NAME", kItemName, true);
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_DISPLAY_GRADE", std::wstring(L""));

		//step 4. 검색 아이템 요구 레벨 표시
		lwMarketUtil::SetUIText(pItemWnd, L"FRM_ITEM_LEVEL", std::wstring(L"-"));
	}

	//step 5. 검색 상점 이름 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_NAME", kShopInfo.kMarketName);

	//step 6. 검색 상점 포인트 표시
	lwMarketUtil::SetUIText(pItemWnd, L"FRM_SHOP_HP", BM::vstring(kShopInfo.iHP));

	//step 7. 검색 아이템 가격 표시
	lwMarketUtil::SetUITextToItemCost(pItemWnd, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.kCostType, kItemInfo.kArticleCost);
}

//---------------------------------------------------------------------------------------------------------------------------------
//	Market RegItem 분리 2009. 7. 19
//---------------------------------------------------------------------------------------------------------------------------------
void lwMarket::lwOnClickChangeRegTabType(lwUIWnd kSelf, int const iType)
{
	XUI::CXUI_CheckButton* pSelf = dynamic_cast<XUI::CXUI_CheckButton*>(kSelf.GetSelf());
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	for(int i = 0; i < 3; ++i)
	{
		BM::vstring vStr(L"CBTN_REG_TYPE");
		vStr += i;

		XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
		if( pCheckBtn )
		{
			pCheckBtn->ClickLock(false);
			pCheckBtn->Check(false);
		}
	}

	pSelf->Check(true);
	pSelf->ClickLock(true);

	XUI::CXUI_Wnd* pBgParent = pParent->GetControl(L"FRM_WINBG");
	if( !pBgParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pTab0 = pBgParent->GetControl(L"FRM_REG_TYPE0");
	XUI::CXUI_Wnd* pTab1 = pBgParent->GetControl(L"FRM_REG_TYPE1");
	XUI::CXUI_Wnd* pCtrlTab0 = pParent->GetControl(L"FRM_REG_TYPE0");
	XUI::CXUI_Wnd* pCtrlTab1 = pParent->GetControl(L"FRM_REG_TYPE1");
	if( !pTab0 || !pTab1 || !pCtrlTab0 || !pCtrlTab1 )
	{
		return;
	}

	switch( iType )
	{
	case 0:
		{
			pTab0->Visible(true);
			pCtrlTab0->Visible(true);
			pTab1->Visible(false);
			pCtrlTab1->Visible(false);
			return;
		}break;
	default:
		{
			pTab0->Visible(false);
			pCtrlTab0->Visible(false);
			pTab1->Visible(true);
			pCtrlTab1->Visible(true);
		}break;
	}

	for(int i = 1; i < 3; ++i)
	{
		BM::vstring vStr(L"FRM_REG_MONEY_TYPE");
		vStr += i;

		XUI::CXUI_Wnd* pMoneyFormBG = pTab1->GetControl(vStr);
		if( pMoneyFormBG )
		{
			pMoneyFormBG->Visible( i == iType );
		}
	}

	wchar_t szTemp[MAX_PATH] = {0,};
	XUI::CXUI_Wnd* pTemp = pTab1->GetControl(L"FRM_ARROW1");
	if( pTemp )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403263).c_str(), TTW(403255 + iType).c_str());
		pTemp->Text(szTemp);
	}

	pTemp = pTab1->GetControl(L"FRM_TEXT3");
	if( pTemp )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403258).c_str(), TTW(403255 + iType).c_str());
		pTemp->Text(szTemp);
	}

	pTemp = pCtrlTab1->GetControl(L"BTN_SELL_VALUE");
	if( pTemp )
	{
		swprintf_s(szTemp, MAX_PATH, TTW(403260).c_str(), TTW(403255 + iType).c_str());
		pTemp->Text(szTemp);
	}

	pTemp = pCtrlTab1->GetControl(L"FRM_MONEY0");
	if( pTemp )
	{
		pTemp->Text(L"");
	}

	pTemp = pCtrlTab1->GetControl(L"FRM_MONEY1");
	if( pTemp )
	{
		pTemp->Text(L"");
	}

	pTemp = pCtrlTab1->GetControl(L"FRM_RATE");
	if( pTemp )
	{
		pTemp->Text(L"");
	}

	XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pCtrlTab1->GetControl(L"EDT_SELL_VALUE"));
	if( pkEdit )
	{
		pkEdit->EditText(L"");
	}

	pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pCtrlTab1->GetControl(L"EDT_SELL_COST"));
	if( pkEdit )
	{
		pkEdit->EditText(L"");
	}
}

void lwMarket::lwOnClickMoneyChangeRate(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	int iDest = 0;
	int iSour = 0;

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pSelf->GetControl(L"EDT_SELL_VALUE"));
	if( pEdit )
	{
		iDest = _wtoi(pEdit->EditText().c_str());
	}

	pEdit = dynamic_cast<XUI::CXUI_Edit*>(pSelf->GetControl(L"EDT_SELL_COST"));
	if( pEdit )
	{
		iSour = _wtoi(pEdit->EditText().c_str());
	}

	float fDest = 0, fSour = 0, fRate = 0;
	lwMarketUtil::CalcMoneyChangeRate(iDest, iSour, fDest, fSour, fRate);

	XUI::CXUI_Wnd* pkDest = pSelf->GetControl(L"FRM_MONEY0");
	XUI::CXUI_Wnd* pkSour = pSelf->GetControl(L"FRM_MONEY1");
	if( pkDest && pkSour )
	{
		pkDest->Text(lwMarketUtil::FloatConvertToStr(fDest, 4));
		pkSour->Text(lwMarketUtil::FloatConvertToStr(fSour, 4));
	}

	XUI::CXUI_Wnd* pkRate = pSelf->GetControl(L"FRM_RATE");
	if( pkRate )
	{
		pkRate->Text(lwMarketUtil::FloatConvertToStr(fRate * 100, 4) + L"%");
	}
}

void lwMarket::lwOnTickCheckChangeMaxValue(lwUIWnd kSelf)
{
	XUI::CXUI_Edit* pSelf = dynamic_cast<XUI::CXUI_Edit*>(kSelf.GetSelf());
	if( !pSelf )
	{
		return;
	}

	int iCost = _wtoi(pSelf->EditText().c_str());
	if( iCost > MAX_CHANG_MONEY_VALUE )
	{
		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, L"%d", MAX_CHANG_MONEY_VALUE);
		pSelf->EditText(szTemp);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Common 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
void lwMarket::lwOnClickListItem(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
	if( !pList ){ return; }

	if( pList->GetTotalItemCount() )
	{
		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem && pItem->m_pWnd )
		{
			XUI::CXUI_Wnd* pClick = pItem->m_pWnd->GetControl(L"FRM_ITEM_CLICK");
			pClick->Visible(false);
			pItem = pList->NextItem(pItem);
		}
	}

	XUI::CXUI_Wnd* pMyClick = pSelf->GetControl(L"FRM_ITEM_CLICK");
	if( pMyClick )
	{
		pMyClick->Visible(true);
	}

	XUI::CXUI_Wnd* pMarketUI = XUIMgr.Get(L"FRM_AUCTION_MAIN");
	if( pMarketUI )
	{
		XUI::CXUI_Button* pAction = dynamic_cast<XUI::CXUI_Button*>(pMarketUI->GetControl(L"BTN_ACTION"));
		if( pAction )
		{
			pAction->Disable(false);
		}
	}

	if( lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true) )
	{//마우스 좌클릭 + Shift = 검색란에 아이템 이름 등록
		XUI::CXUI_Wnd* pIcon = pSelf->GetControl(L"IMG_ICON");
		if( !pIcon )
		{
			return;
		}

		if( !pIcon->GetCustomDataSize() )
		{
			return;
		}

		PgBase_Item kItem;
		{
			BM::Stream kPacket;
			pIcon->GetCustomData(kPacket.Data());
			kPacket.PosAdjust();
			kItem.ReadFromPacket(kPacket);
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());

		if( !pDef )
		{
			return;
		}

		const char * pItemName = lwGetDefString(pDef->NameNo());
		if(pItemName)
		{
			lwMarketUtil::SendMarketSearchText(UNI(pItemName));
		}
	}
}

void lwMarket::lwDrawMarketItemIcon(lwUIWnd UISelf)
{
	XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(UISelf.GetSelf());
	if( !pIcon )
	{ 
		return; 
	}

	if( !pIcon->GetCustomDataSize() )
	{
		return;
	}

	PgBase_Item kItem;
	{
		BM::Stream kPacket;
		pIcon->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		kItem.ReadFromPacket(kPacket);
	}

	if( !kItem.IsEmpty() )
	{
		PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(kItem.ItemNo());
		if( !pkSprite )
		{
			pIcon->DefaultImgTexture(NULL);
			pIcon->SetInvalidate();
			return;
		}

		PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
		if( !pkUVSprite ){ return; }

		pIcon->DefaultImgTexture(pkUVSprite);
		SUVInfo& rkUV = pkUVSprite->GetUVInfo();
		pIcon->UVInfo(rkUV);
		POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
		pIcon->ImgSize(kPoint);
		pIcon->SetInvalidate();
	}

	lwAccumlationExpCard::CheckUsingExpCardInfo(pIcon, kItem, false);
}

void lwMarket::lwOnOverMarketItemToolTip(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	if( !pSelf->GetCustomDataSize() )
	{
		return;
	}

	PgBase_Item kItem;
	{
		BM::Stream kPacket;
		pSelf->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		kItem.ReadFromPacket(kPacket);
	}

	E_TOOLTIP_FLAG kFlag = static_cast<E_TOOLTIP_FLAG>(TTF_NOT_METHOD|TTF_NOT_AMONUT);
	ToolTipComp_SItem(kItem, lwPoint2(pSelf->TotalLocation().x + pSelf->Size().x, pSelf->TotalLocation().y), SToolTipFlag(kFlag));
}

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Util 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
void lwMarketUtil::SetUIText(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, std::wstring const& kText, bool bIsReduce)
{
	if( !pItemWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pTarget = pItemWnd->GetControl(kChild);
	if( pTarget )
	{
		if( bIsReduce )
		{
			Quest::SetCutedTextLimitLength(pTarget, kText, L"...", pTarget->Size().x - 10);
		}
		else
		{
			pTarget->Text(kText);
		}
	}
}

void lwMarketUtil::SetUITextToItemName(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, DWORD const dwItemNo, SEnchantInfo const& kEnchantInfo, bool bIsReduce, bool bIsCRLF)
{
	if( !pItemWnd )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(dwItemNo);
	if (!pItemDef)
	{
		return;
	}

	PgBase_Item kItem;
	kItem.EnchantInfo(kEnchantInfo);
	E_ITEM_GRADE eItemLv = GetItemGrade(kItem);
	bool const bIsCash = pItemDef->IsType(ITEM_TYPE_AIDS);

	int iItemType = 0;
	int iColor = 0;
	switch(eItemLv)
	{
	case IG_SEAL: 
		{ 
			iItemType = 50401;
			iColor = 4107;
		}break;
	case IG_CURSE:
		{	
			iItemType = 50400;
			iColor = 4108;
		}break;
	default: 
		{
			iItemType = 50300 + eItemLv; 
			iColor = 4109 + eItemLv;
		}break;
	}

	std::wstring kItemName;
	MakeItemName(dwItemNo, kEnchantInfo, kItemName);

	bool const bIsConsume = pItemDef->IsType(ITEM_TYPE_CONSUME);
	wchar_t szTemp[MAX_PATH] = {0,};
	if(bIsConsume)
	{
		swprintf_s(szTemp, MAX_PATH, L"{C=%s/}%s", 
			TTW(iColor).c_str(), kItemName.c_str());
	}
	else
	{
		swprintf_s(szTemp, MAX_PATH, L"{C=%s/}[%s]%s%s", 
			TTW(iColor).c_str(), TTW(iItemType).c_str(), bIsCRLF ? L"\n":L" ", kItemName.c_str());
	}

	SetUIText(pItemWnd, kChild, std::wstring(szTemp), bIsReduce);
}

void lwMarketUtil::SetUITextToItemCost(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kGold, std::wstring const& kCash, ERegCostType const Type, __int64 const CostValue)
{
	if( !pItemWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pCost1 = NULL;
	XUI::CXUI_Wnd* pCost2 = NULL;
	switch( Type )
	{
	case RCT_GOLD:
		{
			pCost1 = pItemWnd->GetControl(kGold);
			pCost2 = pItemWnd->GetControl(kCash);
			if( !pCost1 || !pCost2 )
			{
				return;
			}
			__int64	i64Gold = CostValue / 10000;
			SetUIText(pCost1, L"FRM_GOLD", BM::vstring(i64Gold));
			int iSilver = (int)((CostValue % 10000) / 100);
			SetUIText(pCost1, L"FRM_SILVER", BM::vstring(iSilver));
			int iCopper = (int)(CostValue % 100);
			SetUIText(pCost1, L"FRM_COPPER", BM::vstring(iCopper));
		} break;
	case RCT_CASH:
		{
			pCost1 = pItemWnd->GetControl(kCash);
			pCost2 = pItemWnd->GetControl(kGold);
			if( !pCost1 || !pCost2 )
			{
				return;
			}
			SetUIText(pItemWnd, kCash, BM::vstring(CostValue));
		} break;
	default:
		{
			return;
		}
	}
	pCost1->Visible(true);
	pCost2->Visible(!pCost1->Visible());
}

void lwMarketUtil::SetUIIconToItemNo(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, DWORD const dwItemNo, SEnchantInfo const& kEnchantInfo, int const iItemCount, BM::PgPackedTime const& kGenTime)
{
	PgBase_Item kItem;
	kItem.ItemNo(dwItemNo);
	kItem.EnchantInfo(kEnchantInfo);
	kItem.Count(iItemCount);
	kItem.CreateDate(kGenTime);

	SetUIIconToBaseItem(pItemWnd, kChild, kItem);
}

void lwMarketUtil::SetUIIconToBaseItem(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, PgBase_Item const& kItem)
{
	if( !pItemWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pIconImg = pItemWnd->GetControl(kChild);
	if( !pIconImg )
	{
		return;
	}

	BM::Stream kPacket;
	kItem.WriteToPacket(kPacket);
	pIconImg->SetCustomData(kPacket.Data());

	XUI::CXUI_Wnd* pIconCount = pIconImg->GetControl(L"FRM_ITEM_COUNT");
	if( !pIconCount )
	{
		return;
	}
	pIconCount->Visible(false);

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if (pItemDef && !pItemDef->CanEquip())
	{
		int iCount = 1;
		pIconCount->Visible(true);
		int const iCustomType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		if( iCustomType != UICT_ELIXIR )
		{
			iCount = kItem.Count();
		}
		pIconCount->Text(BM::vstring(iCount));
	}


	//xml에서 아이템을 이미지 속성으로 쓰는 경우 AddRes(등급표시)용 이미지 레이어를 겹쳐서 그려준다
	XUI::CXUI_Image* pOverlayIconImg = dynamic_cast<XUI::CXUI_Image*>(pIconImg);
	if(pOverlayIconImg) //현재는 CXUI_Image만 지원
	{
		SetUIAddResToImage(pOverlayIconImg, kItem.ItemNo());
	}
}
void lwMarketUtil::CalcMoneyChangeRate(int const iDest, int const iSour, float& fDest, float& fSour, float& fRate)
{
	if( iDest == iSour )
	{
		fDest = 1;
		fSour = 1;
		fRate = 1.0f;
	}
	else
	{
		if( 0 != iDest && 0 != iSour )
		{
			fRate = iSour / static_cast<float>(iDest);
			if( iDest < iSour )
			{
				fDest = 1;
				fSour = iSour / static_cast<float>(iDest);
			}
			else
			{
				fDest = iDest / static_cast<float>(iSour);
				fSour = 1;
			}
		}
	}
}

std::wstring const lwMarketUtil::FloatConvertToStr(float const fValue, int const iDecimalCount)
{
	wchar_t szTemp[MAX_PATH] = {0,};

	BM::vstring vStr(L"%.");
	vStr += iDecimalCount;
	vStr += L"f";

	std::wstring kOutPut;
	swprintf_s(szTemp, MAX_PATH, vStr.operator const std::wstring &().c_str(), fValue);
	kOutPut = szTemp;

	if( !kOutPut.empty() )
	{
		std::wstring::size_type kpos = kOutPut.size();
		for(; kpos > 0; --kpos)
		{
			if( kOutPut.at(kpos - 1) == L'.' )
			{
				--kpos;
				break;
			}
			if( kOutPut.at(kpos - 1) != L'0' )
			{
				break;
			}
		}

		kOutPut = kOutPut.substr(0, kpos);
	}

	return kOutPut;
}

bool lwMarketUtil::SendMarketSearchText(std::wstring const & kSearchText)
{
	if( kSearchText.empty() )
	{
		return false;
	}

	CXUI_Wnd *pkWnd = XUIMgr.Get(_T("FRM_AUCTION_MAIN"));
	if( !pkWnd )
	{
		return false;
	}
	XUI::CXUI_Wnd* pkSearch = pkWnd->GetControl(L"FRM_SEARCH");
	if( !pkSearch )
	{
		return false;
	}
	XUI::CXUI_Edit* const pkEdtWnd = dynamic_cast<XUI::CXUI_Edit*>(pkSearch->GetControl(L"EDT_SEARCH_VALUE"));
	if( !pkEdtWnd )
	{
		return false;
	}
		
	pkEdtWnd->EditText(kSearchText);
	return true;
}