#include "stdafx.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "lwCashShop.h"
#include "PgUIModel.h"
#include "PgUIScene.h"
#include "PgChatMgrClient.h"
#include "PgWorld.h"
#include "lwWorld.h"
#include "lwUIQuest.h"
#include "PgCommandMgr.h"
#include <shellapi.h>
#include "PgClientParty.h"
#include "variant/PgMyHome.h"
#include "PgHomeTown.h"
#include "PgHome.h"
#include "lwUICSGacha.h"
#include "lwUICostumeMix.h"
#include "PgMobileSuit.h"
#include "PgWebLogin.h"
#include "Utility/md5.h"

int const MAX_ICON_SLOT = 36;
int const MAX_DEF_INVUI_SLOT_COUNT = 36;

extern int CALLBACK ClientItemResChooser(int const iGenderLimit);
extern int CALLBACK ClientItemNewResChooser(int const iGenderLimit);

//! 스크립팅 시스템에 등록한다.
bool lwCashShop::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "CallCashShop", lwCashShop::lwCallCashShop);
	def(pkState, "TopCategoryOnClick", lwCashShop::TopCategoryOnClick);
	def(pkState, "SubCategoryOnClick", lwCashShop::SubCategoryOnClick);
	def(pkState, "SetCategoryPageClick", lwCashShop::SetCategoryPageClick);
	def(pkState, "DayBtnOnClick", lwCashShop::lwDayBtnOnClick);
	def(pkState, "BuyCashItemOnClick", lwCashShop::lwBuyCashItemOnClick);
	def(pkState, "RecvGift", lwCashShop::lwRecvGift);
	def(pkState, "DoActionCashItem", lwCashShop::lwDoActionCashItem);
	def(pkState, "SendExitCashShop", lwCashShop::lwExitCashShop);
	def(pkState, "AddPreviewActor", lwCashShop::lwAddPreviewActor);
	def(pkState, "RemovePreviewActor", lwCashShop::lwRemovePreviewActor);
	def(pkState, "DrawPreviewEquipIcon", lwCashShop::lwDrawPreviewEquipIcon);
	def(pkState, "CallPreviewEquipItemToolTip", lwCashShop::lwCallPreviewEquipItemToolTip);
	def(pkState, "ClearPreviewEquipIcon", lwCashShop::lwClearPreviewEquipIcon);
	def(pkState, "SendRegMyLank", lwCashShop::lwSendRegMyLank);
	def(pkState, "RotatePreviewActor", lwCashShop::lwRotatePreviewActor);
	def(pkState, "SelectCashInvType", lwCashShop::lwSelectCashInvType);
	def(pkState, "SelectItemMoveToInv", lwCashShop::lwSelectItemMoveToInv);
	def(pkState, "SelectItemReNew", lwCashShop::lwSelectItemReNew);
	def(pkState, "ExistPosInItem", lwCashShop::lwExistPosInItem);
	def(pkState, "OpenCashChargeURL", lwCashShop::lwOpenCashChargeURL);
	def(pkState, "SetUseMileage", lwCashShop::lwSetUseMileage);	
	def(pkState, "SetShopType", lwCashShop::lwSetShopType);		
	def(pkState, "GetShopType", lwCashShop::lwGetShopType);
	def(pkState, "UpdateTotalCashExpression", lwCashShop::lwUpdateTotalCashExpression);
	def(pkState, "ClearCashShopInfo", lwCashShop::lwClearCashShopInfo);	
	def(pkState, "CurrentBuyType", lwCashShop::lwCurrentBuyType);
	def(pkState, "OnClickChangePage", lwCashShop::lwOnClickChangePage);
	def(pkState, "OnClickUnRegPreviewItem", lwCashShop::lwOnClickUnRegPreviewItem);
	def(pkState, "ConfirmBasketRegItem", lwCashShop::lwConfirmBasketRegItem);
	def(pkState, "ConfirmBasketDeRegItem", lwCashShop::lwConfirmBasketDeRegItem);
	def(pkState, "DeleteSelectBasketItem", lwCashShop::lwDeleteSelectBasketItem);
	def(pkState, "GetStaticBuyType", lwCashShop::lwGetStaticBuyType);
	def(pkState, "OnCallStaticCashItemBuy", lwCashShop::lwOnCallStaticCashItemBuy);
	def(pkState, "OnCallStaticCashItemRenew", lwCashShop::lwOnCallStaticCashItemRenew);
	def(pkState, "OnCloseStaticCashItemBuy", lwCashShop::lwOnCloseStaticCashItemBuy);
	
	return true;
}

void lwCashShop::SelectCashTypeClear(XUI::CXUI_Wnd* pParent)
{
	if( !pParent ){ return; }
	pParent = pParent->GetControl(_T("SFRM_CASH_TITLE"));
	if( !pParent ){ return; }

	for(int i = 0; i < 2; ++i)
	{
		BM::vstring vSelect(_T("BTN_TITLE_TAB"));
		vSelect += i;
		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vSelect));
		if( pCheck )
		{	
			if( i == 0 )
			{
				lwSelectCashInvType(lwUIWnd(pCheck), 0);
				pCheck->Check(true);
				continue;
			}

			pCheck->Check(false);
		}
	}
}

void lwCashShop::SelectedMoveBtnClear(XUI::CXUI_Wnd* pParent)
{
	if( !pParent ){ return; }

	for(int i = 0; i < MAX_ICON_SLOT; ++i)
	{
		BM::vstring vSelect(_T("BTN_SLOT_SELECT"));
		vSelect += i;
		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vSelect));
		if( pCheck )
		{	
			pCheck->Check(false);
		}
	}
}

void lwCashShop::lwSelectItemMoveToInv(lwUIWnd UIParent)
{
	typedef std::vector< SItemPos >	KCONT_POSLIST;

	XUI::CXUI_Wnd* pParent = UIParent.GetSelf();

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	KCONT_POSLIST	kItemCont;
	for(int i = 0; i < MAX_ICON_SLOT; ++i)
	{
		BM::vstring	vIcon(_T("ICN_CASH_INV"));
		BM::vstring vSelect(_T("BTN_SLOT_SELECT"));

		vIcon += i;
		vSelect += i;

		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pParent->GetControl(vIcon));
		if( !pIcon )
		{
			continue;
		}

		SItemPos kItemPos(pIcon->IconInfo().iIconGroup, pIcon->IconInfo().iIconKey);
		PgBase_Item kItem;
		if( S_OK != pInv->GetItem(kItemPos, kItem) )
		{
			continue;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pkItemDef )
		{
			continue;
		}

		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vSelect));
		if( pCheck && pCheck->Check() )
		{
			pCheck->Check(false);
			kItemCont.push_back(kItemPos);
		}
	}

	if( !kItemCont.size() )
	{
		return;
	}

	int const iTargetInvType = (kItemCont.at(0).x == KUIG_CASH)?(KUIG_CASH_SAFE):(KUIG_CASH);

	KCONT_POSLIST	kTargetCont;
	SItemPos kTargetPos;
	if(pInv->GetFirstEmptyPos((EInvType)iTargetInvType, kTargetPos))
	{
		kTargetCont.push_back(kTargetPos);
		while(kItemCont.size() != kTargetCont.size())
		{
			++kTargetPos.y;
			if( !pInv->GetNextEmptyPos((EInvType)iTargetInvType, kTargetPos) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 700037, true);
				return;
			}
			kTargetCont.push_back(kTargetPos);
		}
	}
	else
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 700037, true);
		return;
	}

	for(size_t i = 0; i < kItemCont.size(); ++i)
	{
		if( (0 == kItemCont.at(i).x) || (0 == kTargetCont.at(i).x) )
		{
			return;
		}

		Send_PT_C_M_REQ_ITEM_CHANGE(kItemCont.at(i), kTargetCont.at(i));
	}
}

void lwCashShop::lwSelectItemReNew(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd* pParent = UIParent.GetSelf();
	if( !pParent ){ return; }

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	int iSelectItemCount = 0;
	SItemPos kPos;

	for(int i = 0; i < MAX_ICON_SLOT; ++i)
	{
		BM::vstring	vIcon(_T("ICN_CASH_INV"));
		BM::vstring vSelect(_T("BTN_SLOT_SELECT"));

		vIcon += i;
		vSelect += i;

		//체크 상태를 먼저 검사하고
		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vSelect));
		if( pCheck && pCheck->Check() )
		{
			++iSelectItemCount;
			pCheck->Check(false);

			XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pParent->GetControl(vIcon));
			if( !pIcon )
			{
				continue;
			}

			kPos.x = pIcon->IconInfo().iIconGroup;
			kPos.y = pIcon->IconInfo().iIconKey;
		}
	}

	if( 0 == iSelectItemCount )
	{//선택된게 없네
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1849, true);
		return;
	}

	if( iSelectItemCount > 1 )
	{//2개이상 선택
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1844, true);
		return;
	}

	PgBase_Item kItem;
	if( S_OK != pInv->GetItem(kPos, kItem) )
	{//빈슬롯은 체크되지 않는다.
		return;
	}

	if( !kItem.EnchantInfo().IsTimeLimit() )
	{//기간제 아이템이 아니다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1845, true);
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{//아이템이 디비에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 22020, true);
		return;
	}

	int Idx = 0;
	if(!g_kCashShopMgr.FindArticle(kItem.ItemNo(), Idx))
	{//캐시샵 판매물품에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1909, true);
		return;
	}

	g_kCashShopMgr.RenewItemPos(kPos);
	lwBuyCashItemOnClick(UIParent, CIBT_RENEW, Idx);
}

bool lwCashShop::lwExistPosInItem(lwUIWnd UIIcon, bool const bIsBasket)
{
	XUI::CXUI_Wnd* pIcon = UIIcon.GetSelf();
	if( pIcon )
	{
		if( bIsBasket )
		{
			TBL_DEF_CASH_SHOP_ARTICLE kArticle;
			return g_kCashShopMgr.ChangeShoppingBasketSlotIDXToArticleIDX( pIcon->BuildIndex(), kArticle );
		}
		else
		{
			XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pIcon);
			if( !pkIcon )
			{
				return false;
			}

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return false;
			}

			PgInventory* pkInv = pkPlayer->GetInven();
			if( !pkInv )
			{
				return false;
			}

			SItemPos kCasterPos(pkIcon->IconInfo().iIconGroup, pkIcon->IconInfo().iIconKey);
			PgBase_Item kCasterItem;
			if(S_OK == pkInv->GetItem(kCasterPos, kCasterItem))
			{
				return true;
			}
		}
	}
	return false;
}

void lwCashShop::lwSelectCashInvType(lwUIWnd UISelf, int const iType)
{
	XUI::CXUI_Wnd* pTitle = UISelf.GetParent().GetSelf();

	int const MAX_TAB_CONTROL = 3;
	for( int i = 0; i < MAX_TAB_CONTROL; ++i )
	{
		BM::vstring vStr(_T("BTN_TITLE_TAB"));
		vStr += i;

		XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pTitle->GetControl(vStr));
		if( pCheckBtn && pCheckBtn->Check() )
		{
			pCheckBtn->ClickLock(false);
			pCheckBtn->Check(false);
		}
	}
	UISelf.CheckState(true);
	UISelf.LockClick(true);

	XUI::CXUI_Wnd* pParent = pTitle->Parent();

	SetCashInvViewSlot(pParent, iType, 0);
}

void lwCashShop::lwOnClickChangePage(lwUIWnd UISelf)
{
	XUI::CXUI_Button* pBtn = dynamic_cast<XUI::CXUI_Button*>(UISelf.GetSelf());
	if( pBtn )
	{
		int iCalc = 0;
		SItemPos kPageInfo;
		pBtn->GetCustomData(&iCalc, sizeof(iCalc));

		XUI::CXUI_Wnd* pParent = pBtn->Parent();
		if( !pParent )
		{
			return;
		}
		pParent->GetCustomData(&kPageInfo, sizeof(kPageInfo));

		int iCurPage = kPageInfo.x + iCalc;
		if( iCurPage < 0 )
		{
			return;
		}
		else if( iCurPage >= kPageInfo.y )
		{
			return;
		}
		else
		{
			int iType = -1;
			XUI::CXUI_Wnd* pInfo = pParent->Parent();
			if(pInfo)
			{
				XUI::CXUI_Wnd* pTitle = pInfo->GetControl(L"SFRM_CASH_TITLE");
				if(pTitle)
				{
					for(int i = 0; i < 2; ++i)
					{
						BM::vstring vSelect(_T("BTN_TITLE_TAB"));
						vSelect += i;
						XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pTitle->GetControl(vSelect));
						if( pCheck && (true==pCheck->Check()) )
						{	
							iType = i;
						}
					}
				}
			}

			SetCashInvViewSlot(pParent->Parent(), iType, iCurPage);
		}
	}
}

void lwCashShop::lwExitCashShop()
{
	//케릭터 락 해제
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_CashShop, true));
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_CashShop, false));

	bool bHomeIn = false;
	PgHome* pkHome = g_pkWorld->GetHome();
	if( pkHome )
	{
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			if( pkHomeUnit->IsInSide() )
			{
				g_kChatMgrClient.ChatStation(ECS_MYHOME);
				bHomeIn = true;
			}
		}
	}

	if( !bHomeIn )
	{
		g_kChatMgrClient.ChatStation(ECS_COMMON);
	}

	lwClearPreviewFurnitureModel();
	XUIMgr.Close(L"FRM_CASH_SHOP_BG");
	int const iOldChatMode = g_kChatMgrClient.OldChatMode();
	lwCallChatWindow(iOldChatMode);
	PgClientPartyUtil::UpdatePartyWndAll();

	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	if ( pkPilot )
	{
		pkPilot->SetAbil64( AT_MILEAGE_TOTAL_CASH, 0i64 );
		pkPilot->SetAbil64( AT_MILEAGE_100, 0i64 );
		pkPilot->SetAbil64( AT_MILEAGE_1000, 0i64 );
	}

	g_kHomeTownMgr.UseCashShop(false);

	SPT_C_M_CS_REQ_EXIT_CASHSHOP kStruct;

	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void lwCashShop::SetShopPage(XUI::CXUI_Wnd* pkWnd, CONT_DEF_CASH_SHOP_ARTICLE const & rkCate, int const iPage)
{
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator cate_it = rkCate.begin();

	XUI::CXUI_Wnd* pkGoods = NULL;
	if ( pkGoods = SetShopForm(pkWnd, PgCashShop::ECSFORM_COMMON) )
	{
		int const iStartCount = iPage*CASH_ITEM_PER_PAGE;
		int const iEndCount = (iPage+1)*CASH_ITEM_PER_PAGE;

		for(int i = 0; i < iStartCount; ++i)
		{
			if( cate_it != rkCate.end() )
			{
				++cate_it;
			}
		}

		g_kCashShopMgr.InitPageArticle();
		for(int i = iStartCount; i < iEndCount; ++i)
		{
			BM::vstring kName(L"FRM_GOODS_ELE");
			kName += (i - iStartCount);
			XUI::CXUI_Wnd* pkEle = pkGoods->GetControl(kName);
			if( pkEle )
			{
				if( cate_it != rkCate.end() )
				{
					TBL_DEF_CASH_SHOP_ARTICLE const& rkArticle = (*cate_it).second;
					WriteToUI(pkEle, rkArticle);
					g_kCashShopMgr.AddToPageArticle(rkArticle);

					++cate_it;
				}
				else
				{
					InitUI(pkEle);
				}
			}
		}
	}
}

void lwCashShop::SetGiftPage(XUI::CXUI_Wnd* pkWnd, CONT_CASHGIFTINFO const& rkCate, bool bRecv)
{
	if(!pkWnd) {return;}
	XUI::CXUI_Wnd* pkGift = pkWnd->GetControl(L"FRM_GIFT");
	if(!pkGift) {return;}

	CONT_CASHGIFTINFO::const_iterator gift_it = rkCate.begin();
	for(int i = 0; i < CASH_GIFT_PER_PAGE; ++i)
	{
		BM::vstring kstrEle(L"FRM_GIFT_ELE");
		kstrEle+=i;
		SCASHGIFTINFO kInfo;
		kInfo.kGiftGuId.Clear();
		kInfo.bType = 0;
		kInfo.bTimeType = 0;
		kInfo.iUseTime = 0;
		kInfo.iCashItemIdx = 0;

		if(rkCate.end() != gift_it)
		{
			kInfo = (*gift_it);
			++gift_it;
		}
		WriteToGiftUI(pkGift->GetControl(kstrEle), kInfo, bRecv);
	}
}

void lwCashShop::SetLimitedPage(XUI::CXUI_Wnd* pkWnd, CONT_DEF_CASH_SHOP_ARTICLE const & rkCate, CONT_CASH_SHOP_ITEM_LIMITSELL const & rContLimited, int iPage)
{ //한정판매 폼/페이지/컨텐츠 설정

	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator iterArticle;
	CONT_CASH_SHOP_ITEM_LIMITSELL::const_iterator iterFoundLimited;
	CONT_DEF_CASH_SHOP_ARTICLE kContLimitedView;

	//유효한 원소들만을 필터링해서 임시 저장소인 kContLimitedView에 저장
	for( iterArticle = rkCate.begin(); iterArticle != rkCate.end(); ++iterArticle)
	{
		if(iterArticle->second.kSaleStartDate.IsNull() || iterArticle->second.kSaleEndDate.IsNull())
		{
			continue;
		}
		if( (iterFoundLimited = rContLimited.find(iterArticle->second.iIdx)) == rContLimited.end() )
		{
			continue;
		}

		kContLimitedView.insert(std::make_pair(TBL_KEY_INT(iterArticle->first), iterArticle->second));

	}
	InitPageUI(pkWnd->GetControl(L"FRM_PAGE"), kContLimitedView.size(), PgCashShop::ECSFORM_LIMITED);



	XUI::CXUI_Wnd* pkLimited = NULL;
	if ( pkLimited = SetShopForm(pkWnd, PgCashShop::ECSFORM_LIMITED))
	{
		int iStart = CASH_LIMITED_PER_PAGE * iPage;
		CONT_DEF_CASH_SHOP_ARTICLE::const_iterator iterLimitedView = kContLimitedView.begin();

		for(int iOffset = 0; iOffset < iStart; ++iOffset)
		{
			if(iOffset >=  kContLimitedView.size() || iterLimitedView == kContLimitedView.end())
			{ //iPage 값이 비정상적인 경우 그냥 함수를 종료
				return;
			}
			++iterLimitedView;
		}
		g_kCashShopMgr.InitPageArticle();
		for(int iElem = 0; iElem < CASH_LIMITED_PER_PAGE; ++iElem)
		{
			BM::vstring kstrElem(L"FRM_LIMITED_ELE");
			kstrElem += iElem;
			XUI::CXUI_Wnd* pkElem = pkLimited->GetControl(kstrElem);
			if(!pkElem) continue;

			if( iterLimitedView != kContLimitedView.end() )
			{
				CONT_CASH_SHOP_ITEM_LIMITSELL::const_iterator iterLimitedInfo;
				if((iterLimitedInfo = rContLimited.find(iterLimitedView->second.iIdx)) != rContLimited.end())
				{
					WriteToLimitedUI(pkElem, iterLimitedView->second, iterLimitedInfo->second);
					g_kCashShopMgr.AddToPageArticle(iterLimitedView->second);
				}
				//
				++iterLimitedView;
			}
			else
			{ //공백 처리
				InitLimitedUI(pkElem);
			}
		}


	}


}

bool lwCashShop::IsLimitedCategory(const TBL_DEF_CASH_SHOP_CATEGORY& rkSubCate)
{
	if(!rkSubCate.kContArticle.empty() && !rkSubCate.kContArticle.begin()->second.kSaleStartDate.IsNull() && !rkSubCate.kContArticle.begin()->second.kSaleEndDate.IsNull())
	{
		return true;
	}

	return false;
}

XUI::CXUI_Wnd* lwCashShop::SetShopForm(XUI::CXUI_Wnd* pkWndParent, PgCashShop::ECashShopForm eShopForm)
{ //JJW: 사용되는 센터 폼은 현재까지 총 3개이다. (공통/선물함/한정판매)
	if(!pkWndParent) { return NULL; }
	XUI::CXUI_Wnd* pkGoods = pkWndParent->GetControl(L"FRM_GOODS");
	XUI::CXUI_Wnd* pkGift = pkWndParent->GetControl(L"FRM_GIFT");
	XUI::CXUI_Wnd* pkLimited = pkWndParent->GetControl(L"FRM_LIMITED");
	if(!pkGoods || !pkGift || !pkLimited) { return NULL; }
	if(!g_kCashShopMgr.SetShopSubType(eShopForm)) return NULL;

	switch(eShopForm)
	{
	case PgCashShop::ECSFORM_COMMON:
		{
			pkGoods->Visible(true);
			pkLimited->Visible(false);
			pkGift->Visible(false);
		}
		return pkGoods;
	case PgCashShop::ECSFORM_LIMITED:
		{
			pkLimited->Visible(true);
			pkGoods->Visible(false);
			pkGift->Visible(false);
		}
		return pkLimited;
	case PgCashShop::ECSFORM_GIFT:
		{
			pkGift->Visible(true);
			pkLimited->Visible(false);
			pkGoods->Visible(false);
		}
		return pkGift;
	case PgCashShop::ECSFORM_UNKNOWN: //캐쉬샵 관련된 모든 폼을 숨김
		{
			pkGift->Visible(false);
			pkLimited->Visible(false);
			pkGoods->Visible(false);
		}
		return NULL;
	}

	return NULL;
}

void lwCashShop::WriteToGiftUI(XUI::CXUI_Wnd* pkWnd, SCASHGIFTINFO const & rkArticle, bool bRecv)
{
	if(!pkWnd) {return;}
	Quest::SetCutedTextLimitLength(pkWnd->GetControl(L"FRM_NAME"), rkArticle.kCharName, L"...");
	BM::vstring kDate;
	bool bBtnView = bRecv;

	if(!rkArticle.kSendTime.IsNull())
	{
		kDate=rkArticle.kSendTime.year;
		kDate+=TTW(172);
		kDate+=L" ";
		kDate+=rkArticle.kSendTime.month;
		kDate+=TTW(173);
		kDate+=L" ";
		kDate+=rkArticle.kSendTime.day;
		kDate+=TTW(174);
		kDate+=L" ";
		kDate+=rkArticle.kSendTime.hour;
		kDate+=TTW(175);
		kDate+=L" ";
		kDate+=rkArticle.kSendTime.minute;
		kDate+=TTW(176);
	}
	else
	{
		bBtnView = false;
	}

	pkWnd->GetControl(L"FRM_TIME_TEXT")->Text(kDate);
	Quest::SetCutedTextLimitLength(pkWnd->GetControl(L"FRM_TEXT"), rkArticle.kComment, L"...");
	pkWnd->SetCustomData(&rkArticle.kGiftGuId, sizeof(rkArticle.kGiftGuId));

	XUI::CXUI_Wnd* pkGetBtn = pkWnd->GetControl(L"BTN_GET_GIFT");
	if(pkGetBtn)
	{
		pkGetBtn->Visible(bBtnView);
		BM::Stream kPacket;
		rkArticle.WriteToPacket(kPacket);
		lwUIWnd kTempWnd(pkGetBtn);
		kTempWnd.SetCustomDataAsPacket(lwPacket(&kPacket));
	}
	pkWnd->GetControl(L"IMG_ARROW1")->Text(TTW(bRecv ? 910 : 917));

	TBL_DEF_CASH_SHOP_ARTICLE kDefArt;
	kDefArt.iIdx = 0;

	if(0<rkArticle.iCashItemIdx)
	{
		if(g_kCashShopMgr.FindPageArticle(rkArticle.iCashItemIdx, kDefArt) || g_kCashShopMgr.GetItem(rkArticle.iCashItemIdx, kDefArt))
		{
			DrawCashItem(pkWnd, kDefArt);
			CashMiniIcon(pkWnd, kDefArt);
		}
	}
	else
	{
		XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_ICON");
		if(pkImg)
		{
			pkImg->Visible(false);
		}
	}
}	

void lwCashShop::DrawCashItem(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const &rkArticle, float const fScale, int const iSize)
{
	XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_ICON");
	if(pkImg)
	{
		CashItemDrawToScaling(pkImg, rkArticle, fScale, iSize);
	}
	
	XUI::CXUI_Wnd* pkGoldImg = pkWnd->GetControl(L"IMG_GOLD");
	if(pkGoldImg)
	{
		pkGoldImg->Visible(false);
		CONT_CASH_SHOP_ITEM::const_iterator detail_itor = rkArticle.kContCashItem.begin();
		while(detail_itor != rkArticle.kContCashItem.end())
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef((*detail_itor).iItemNo);
			if(pkItemDef)
			{
				if(UICT_MONEY_PACK == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
				{// 머니 팩이 들어있으면, 골드 아이콘 표시를 보여준다
					pkGoldImg->Visible(true);
					break;
				}
			}
			++detail_itor;
		}
	}
}

inline void UnVisible(XUI::CXUI_Wnd* pWnd)
{
	if(pWnd)
	{
		pWnd->Visible(false);
	}
}
void lwCashShop::SetDayBtnInitialize(XUI::CXUI_Wnd* pWnd, TBL_DEF_CASH_SHOP_ARTICLE const& kArticle, int const SelectCost, int const iBuyType)
{
	XUI::CXUI_Wnd* pBtn = pWnd;
	int UseValue = 0;
	if( ECST_CONSUME == kArticle.bSaleType )
	{
		if( !kArticle.kContCashItem.empty() )
		{
			UseValue = kArticle.kContCashItem.begin()->iItemCount;
		}
	}

	if( 0 > SelectCost )
	{
		UnVisible( pBtn->GetControl(L"FRM_INPUT0") );
		UnVisible( pBtn->GetControl(L"FRM_INPUT1") );
		UnVisible( pBtn->GetControl(L"FRM_INPUT2") );
		UnVisible( pBtn->GetControl(L"FRM_INPUT3") );

		BM::vstring vStr(L"FRM_INPUT");
		vStr += kArticle.kContCashItemPrice.size();
		pBtn = pBtn->GetControl(vStr);
		if( !pBtn )
		{
			::Notice_Show( L"CAN'T CashShop FRM_INPUT", EL_Warning );
			return;
		}
		pBtn->Visible(true);
	}
	else
	{
		XUI::CXUI_Wnd* pTemp0 = pBtn->GetControl(L"CBTN_DAY0");
		XUI::CXUI_Wnd* pTemp1 = pBtn->GetControl(L"CBTN_DAY1");
		XUI::CXUI_Wnd* pTemp2 = pBtn->GetControl(L"CBTN_DAY2");
		if( !pTemp0 || !pTemp1 || !pTemp2 )
		{
			return;
		}

		XUI::CXUI_Wnd* pTempTxt0 = pBtn->GetControl(L"FRM_DAY0");
		XUI::CXUI_Wnd* pTempTxt1 = pBtn->GetControl(L"FRM_DAY1");
		XUI::CXUI_Wnd* pTempTxt2 = pBtn->GetControl(L"FRM_DAY2");
		if( !pTempTxt0 || !pTempTxt1 || !pTempTxt2 )
		{
			return;
		}

		CONT_CASH_SHOP_ITEM_PRICE::const_iterator price_it = kArticle.kContCashItemPrice.begin();
		while( price_it != kArticle.kContCashItemPrice.end() )
		{
			if( ECST_CONSUME != kArticle.bSaleType )
			{
				if( 0 == price_it->second.iItemUseTime && 1 == kArticle.kContCashItemPrice.size() )
				{
					pTemp2->Location(pTemp0->Location());
					pTemp0->Location(pTemp1->Location());
					pTemp1->Location(pTemp1->Location().x, pTemp0->Location().y + (pTemp0->Location().y - pTemp2->Location().y));

					pTempTxt2->Location(pTempTxt0->Location());
					pTempTxt0->Location(pTempTxt1->Location());
					pTempTxt1->Location(pTempTxt1->Location().x, pTempTxt0->Location().y + (pTempTxt0->Location().y - pTempTxt2->Location().y));
					break;
				}
				else if( 0 == price_it->second.iItemUseTime && 2 == kArticle.kContCashItemPrice.size() )
				{
					pTemp2->Location(pTemp1->Location());
					pTemp1->Location(pTemp2->Location().x, pTemp2->Location().y + (pTemp2->Location().y - pTemp0->Location().y));

					pTempTxt2->Location(pTempTxt1->Location());
					pTempTxt1->Location(pTempTxt1->Location().x, pTempTxt2->Location().y + (pTempTxt2->Location().y - pTempTxt0->Location().y));
					break;
				}
			}

			++price_it;
		}
	}

	CONT_CASH_SHOP_ITEM_PRICE::const_iterator price_it = kArticle.kContCashItemPrice.begin();
	int iMaxButton = 3;
	for( int i = 0; i < iMaxButton; ++i )
	{
		BM::vstring vStr(L"CBTN_DAY");
		vStr += i;
		XUI::CXUI_CheckButton* pCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pBtn->GetControl(vStr));
		if( pCBtn )
		{
			if( price_it != kArticle.kContCashItemPrice.end() )
			{
				if( CIBT_RENEW == iBuyType )
				{
					if( 0 == price_it->second.iExtendCash && (!price_it->second.IsExtendDiscount || 0 != price_it->second.iDiscountExtendCash ) )
					{
						--i;
						++price_it;
						continue;
					}
				}

				XUI::CXUI_Wnd* pCostTextWnd = pCBtn;
				if( 0 > SelectCost )
				{
					if( ECST_CONSUME != kArticle.bSaleType && 0 == price_it->second.iItemUseTime && 0 > SelectCost)
					{
						--i;
						--iMaxButton;
						pCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pBtn->GetControl(L"CBTN_DAY2"));
					}
					if( pCBtn )
					{
						pCBtn->Visible(true);
						pCBtn->SetCustomData(&(*price_it).second, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
						pCBtn->Text(MakeUseTimeStr(kArticle.bSaleType, price_it->second.iItemUseTime, static_cast<EUseItemType>(price_it->second.bTimeType)));

						if( 0 == i || 1 == kArticle.kContCashItemPrice.size())
						{
							pCBtn->Check(true);
							pCBtn->ClickLock(true);
							XUI::CXUI_Wnd* pkDay = pWnd->GetControl(L"FRM_DAY");
							if( pkDay )
							{
								pkDay->Text(MakeBuyValueStr(price_it->second, iBuyType));
								pkDay->SetCustomData(&(*price_it).second, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
							}
						}
					}
				}
				else
				{
					vStr = L"FRM_DAY";
					vStr += i;
					pCostTextWnd = pBtn->GetControl(vStr);

					if( ECST_CONSUME != kArticle.bSaleType && 0 == price_it->second.iItemUseTime )
					{
						--i;
						--iMaxButton;
						pCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pBtn->GetControl(L"CBTN_DAY2"));
						pCostTextWnd = pBtn->GetControl(L"FRM_DAY2");
					}
					pCBtn->Visible(true);
					pCBtn->SetCustomData(&(*price_it).second, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));

					if( pCostTextWnd )
					{
						pCostTextWnd->Visible(true);
						vStr = L"";
						MakeUseTimeStr(kArticle.bSaleType, price_it->second, vStr, UseValue, iBuyType);
						pCostTextWnd->Text(vStr);
					}

					int iPriceValue = 0;

					if( CIBT_RENEW == iBuyType )
					{
						iPriceValue = ((price_it->second.IsExtendDiscount) ? price_it->second.iDiscountExtendCash : price_it->second.iExtendCash);
					}
					else
					{
						iPriceValue = ((price_it->second.IsDiscount) ? price_it->second.iDiscountCash : price_it->second.iCash);
					}

					if( SelectCost == iPriceValue )
					{// 선택한 가격과, 아이템에 등록된 가격이 같으면 체크된 상태로 표시한다. (7일, 30일, 무기한)
						pCBtn->Check(true);
						pCBtn->ClickLock(true);
					}
				}

				++price_it;
			}
			else
			{
				pCBtn->Visible(false);
				pCBtn->Text(L"");

				if( 0 <= SelectCost )
				{
					vStr = L"FRM_DAY";
					vStr += i;
					XUI::CXUI_Wnd* pCostTextWnd = pBtn->GetControl(vStr);
					if( pCostTextWnd )
					{
						pCostTextWnd->Visible(false);
					}
				}
			}
		}
	}
}

void lwCashShop::WriteToUI(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle)
{
	if (!pkWnd)
	{
		return;
	}
	InitUI(pkWnd);
	int iIndexFrom = 0;
	bool MainUINoEquip = true;
	int const iWidth = pkWnd->Width() - 102;
	Quest::SetCutedTextLimitLength(pkWnd, rkArticle.wstrName, _T("..."), iWidth);
	pkWnd->SetCustomData(&rkArticle.iIdx, sizeof(rkArticle.iIdx));

	//기간 정보를 뽑아 와야 함
	SetDayBtnInitialize(pkWnd, rkArticle);

	switch(rkArticle.bDisplayTypeIdx)
	{
	case CIDT_NEW:
		{
			pkWnd->GetControl(L"IMG_NEW")->Visible(true);
		}break;
	case CIDT_COOL:
		{
			pkWnd->GetControl(L"IMG_COOL")->Visible(true);
		}break;
	case CIDT_SALE:
	case CIDT_EVENT:
	case CIDT_HOT:
	case CIDT_BEST:
		{
			XUI::CXUI_Wnd* pkMark = pkWnd->GetControl(L"IMG_MARK");
			if(pkMark)
			{
				pkMark->Visible(true);
				SUVInfo kUVInfo= pkMark->UVInfo();
				kUVInfo.Index = rkArticle.bDisplayTypeIdx-2;
				pkMark->UVInfo(kUVInfo);
			}
		}break;
	case CIDT_RECOMMAND:
		{
			XUI::CXUI_Wnd* pkMark2 = pkWnd->GetControl(L"IMG_MARK2");
			if(pkMark2)
			{
				BM::vstring vStr(lua_tinker::call<char*, int>("CashShopItemMarkFileName", rkArticle.bDisplayTypeIdx));
				pkMark2->DefaultImgName(vStr);
				pkMark2->Visible(true);
			}
		}break;
	default:
		{
		}break;
	}
	CashMiniIcon(pkWnd, rkArticle);
	DrawCashItem(pkWnd, rkArticle);
	CashNoEquip(pkWnd,iIndexFrom,rkArticle,MainUINoEquip);
}
void lwCashShop::CashMiniIcon(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle)
{
	//tls
	if (!pkWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkParts = pkWnd->GetControl(L"IMG_PARTS");
	if(!pkParts)
	{
		return;
	}
	pkParts->Visible(false);
	GET_DEF(CItemDefMgr, kItemDefMgr);       
	CONT_CASH_SHOP_ITEM::const_iterator item_it = rkArticle.kContCashItem.begin();
	int SaleType = rkArticle.bSaleType; //세트일 경우 값이 3이 들어온다.
	while( item_it != rkArticle.kContCashItem.end())
	{
		CItemDef const *pkItemDef = kItemDefMgr.GetDef((*item_it).iItemNo);
		if(!pkItemDef)
		{
			break;
		}
		int const iItemFilter = pkItemDef->GetAbil(AT_ITEMFILTER);
		int const iEquipLimit = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
		if((iEquipLimit == iItemFilter) && (SaleType !=3) )
		{		
			SUVInfo kUVInfo = pkParts->UVInfo();
			switch(iEquipLimit)
			{
			case EQUIP_LIMIT_WEAPON:
				{
					kUVInfo.Index = 1;
				}break;
			case EQUIP_LIMIT_HELMET:
				{
					kUVInfo.Index = 2;
				}break;

			case EQUIP_LIMIT_SHIRTS:
				{
					kUVInfo.Index = 3;
				}break;
			case EQUIP_LIMIT_PANTS:
				{
					kUVInfo.Index = 4;
				}break;
			case EQUIP_LIMIT_SHOULDER:
				{
					kUVInfo.Index = 5;
				}break;
			case EQUIP_LIMIT_GLOVE:
				{
					kUVInfo.Index = 6;
				}break;
			case EQUIP_LIMIT_BOOTS:
				{
					kUVInfo.Index = 7;
				}break;
			case EQUIP_LIMIT_CLOAK:
				{
					kUVInfo.Index = 8;
				}break;
			default:
				{
					pkParts->Visible(false);
					return;
				}
				break;	
			}
			pkParts->Visible(true);
			pkParts->UVInfo(kUVInfo);
			return;
		}
		break;
	}
}


void lwCashShop::SetLimitedStateUI(XUI::CXUI_Wnd* pkWnd, ELimitedStateType eStateType)
{
	if(!pkWnd) return;

	BM::vstring strStateTitle(L"");
	XUI::CXUI_Wnd* pkState = pkWnd->GetControl(L"FRM_STATE");
	if(pkState)
	{
		pkState->SetCustomData(&eStateType, sizeof(int));
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_FINISH");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_FINISH)
		{
			pkState->Visible(true);
			strStateTitle = _T("");
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_WAIT");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_WAIT)
		{
			pkState->Visible(true);
			strStateTitle = _T("");
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_SOLDOUT");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_SOLDOUT)
		{
			pkState->Visible(true);
			strStateTitle = _T("");
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_TIME");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_TIME)
		{
			pkState->Visible(true);
			strStateTitle = TTW(3520);
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_IMPEND");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_IMPEND)
		{
			pkState->Visible(true);
			strStateTitle = TTW(3520);
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_HOT");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_HOT)
		{
			pkState->Visible(true);
			strStateTitle = TTW(3520);
		}
		else pkState->Visible(false);
	}
	pkState = pkWnd->GetControl(L"FRM_STATE_SALE");
	if(pkState)
	{
		if(eStateType == LIMITEDSTATE_SALE)
		{
			pkState->Visible(true);
			strStateTitle = TTW(3520);
		}
		else pkState->Visible(false);
	}

	XUI::CXUI_Wnd* pkWndText = pkWnd->GetControl(L"FRM_STATETITLE");
	if(pkWndText)
	{
		pkWndText->Text(strStateTitle);
		pkWndText->Visible(true);
	}
	else
	{
		pkWndText->Visible(false);
	}
}

void lwCashShop::SetLimitedState(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle,
								 TBL_DEF_CASH_SHOP_ITEM_LIMITSELL const& rkLimitedInfo)
{
	SYSTEMTIME kTempTime;
	if( !g_kEventView.GetLocalTime(&kTempTime) )
	{ //현재시간을 구할 수 없으면 그냥 종료.
		SetLimitedStateUI(pkWnd, LIMITEDSTATE_NA);
		return;
	}
	BM::PgPackedTime kTimeNow(kTempTime);
	BM::PgPackedTime kTimeImpend(kTimeNow);
	if(!rkArticle.kSaleEndDate.IsNull())
	{
		kTimeImpend = rkArticle.kSaleEndDate;
		CGameTime::AddTime(kTimeImpend, -(CGameTime::HOUR));
	}

	ELimitedStateType eState = LIMITEDSTATE_NA;

	if(!rkArticle.kSaleEndDate.IsNull() && rkArticle.kSaleEndDate < kTimeNow)
	{ //판매기간 종료 여부 체크(종료)
		eState = LIMITEDSTATE_FINISH;
	}
	else if(!rkArticle.kSaleStartDate.IsNull() && kTimeNow < rkArticle.kSaleStartDate)
	{ //판매시작전인지 체크(판매대기)
		eState = LIMITEDSTATE_WAIT;
	}
	else if(rkLimitedInfo.iItemTotalCount > 0 && rkLimitedInfo.iItemLimitCount <= 0)
	{ //한정 수량일 경우 다 팔렸는지 체크
		eState = LIMITEDSTATE_SOLDOUT;
	}
	else if(!rkArticle.kSaleStartDate.IsNull() && !rkArticle.kSaleEndDate.IsNull() &&
		rkArticle.kSaleStartDate < kTimeNow && kTimeNow < rkArticle.kSaleEndDate &&
		kTimeImpend < kTimeNow)
	{ //현재가 판매기간이고 && 현재가 판매마감 1시간 이내인지 체크(마감임박)
		eState = LIMITEDSTATE_TIME;
	}
	else
	{
		eState = LIMITEDSTATE_SALE;
		if(rkLimitedInfo.iItemTotalCount > 0 && rkLimitedInfo.iItemLimitCount > 0)
		{
			float iPercent = (rkLimitedInfo.iItemLimitCount * 100) / rkLimitedInfo.iItemTotalCount;
			if(iPercent <= 10.f)
			{ //전체 수량의 10% 이하가 남았는지 체크(매진임박)
				eState = LIMITEDSTATE_IMPEND;
			}
			else if(iPercent <= 70.f)
			{ //전체 수량의 70% 이하가 남았는지 체크(인기판매)
				eState = LIMITEDSTATE_HOT;
			}
		}

	}

	SetLimitedStateUI(pkWnd, eState);

}

void lwCashShop::WriteToLimitedUI(XUI::CXUI_Wnd* pkWnd, TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle,
								  TBL_DEF_CASH_SHOP_ITEM_LIMITSELL const& rkLimitedInfo)
{
	if (!pkWnd)
	{
		return;
	}
	pkWnd->SetCustomData(&rkArticle.iIdx, sizeof(rkArticle.iIdx));
	BM::vstring strRename(rkArticle.wstrName);
	Quest::SetCutedTextLimitLength(pkWnd, strRename, L"...", LIMITED_ITEM_NAME_MAX_LENGTH); //이름이 길어지면 ...으로 짤라준다.
	//pkWnd->Text(rkArticle.wstrName); //아이템 이름

	SetLimitedState(pkWnd, rkArticle, rkLimitedInfo);
	XUI::CXUI_Wnd* pkRemain = pkWnd->GetControl(L"FRM_REMAIN");
	if(pkRemain)
	{
		if(rkLimitedInfo.iItemLimitCount && rkLimitedInfo.iItemTotalCount)
		{
			BM::vstring kstrRemain(L"");
			kstrRemain += TTW(3540);
			kstrRemain += _T(" ");
			kstrRemain += rkLimitedInfo.iItemLimitCount;
			//kstrRemain += TTW(50413);
			kstrRemain += TTW(403262);
			kstrRemain += rkLimitedInfo.iItemTotalCount;
			//kstrRemain += TTW(50413);
			pkRemain->Text(kstrRemain);
			pkRemain->Visible(true);
		}
		else
		{
			pkRemain->Visible(false);
		}
	}
	XUI::CXUI_Wnd* pkCountDown = pkWnd->GetControl(L"FRM_LIMITEDTYPE");
	if(pkCountDown)
	{
		const TBL_DEF_CASH_SHOP_ITEM_PRICE& rkPriceInfo = rkArticle.kContCashItemPrice.begin()->second;
		BM::vstring strLimitedType(L"");

		switch(rkPriceInfo.bTimeType)
		{
		case LT_TIME:
			{
				if(rkPriceInfo.iItemUseTime == 0) strLimitedType += TTW(178);
				else
				{
					strLimitedType += rkPriceInfo.iItemUseTime;
					strLimitedType += TTW(700591);
				}
			}
			break;
		case LT_TIMELESS:
			{
				strLimitedType += TTW(178);
			}
			break;
		case LT_PACKAGE:
			{
				if(rkPriceInfo.iItemUseTime)
				{
					strLimitedType += rkPriceInfo.iItemUseTime;
					strLimitedType += TTW(1709);
				}
			}
			break;
		}

		pkCountDown->Text(strLimitedType);
		pkCountDown->Visible(true);
	}
	XUI::CXUI_Wnd* pkPrice = pkWnd->GetControl(L"FRM_PRICE");
	if(pkPrice)
	{
		BM::vstring strCash(L"");
		strCash = MakeBuyValueStr(rkArticle.kContCashItemPrice.begin()->second);
//		strCash += rkArticle.kContCashItemPrice.begin()->second.iCash;
//		strCash += _T(" ");
//		strCash += TTW(5997);
		pkPrice->Text(strCash);
		pkPrice->Visible(true);
	}
	XUI::CXUI_Wnd* pkBeginDay = pkWnd->GetControl(L"FRM_BEGINDAY");
	if(pkBeginDay)
	{
		std::wstring strDay = MakeTimeStrW(rkArticle.kSaleStartDate);
		strDay += _T("~");
		pkBeginDay->Text(strDay);
		pkBeginDay->Visible(true);
	}
	XUI::CXUI_Wnd* pkEndDay = pkWnd->GetControl(L"FRM_ENDDAY");
	if(pkEndDay)
	{
		pkEndDay->Text(MakeTimeStrW(rkArticle.kSaleEndDate));
		pkEndDay->Visible(true);
	}
	XUI::CXUI_Wnd* pkLimitTime = pkWnd->GetControl(L"FRM_LIMITTIME");
	if(pkLimitTime)
	{
		if(rkLimitedInfo.iItemTotalCount)
		{
			pkLimitTime->Text(TTW(3508));
		}
		else
		{
			pkLimitTime->Text(TTW(3507));
		}
		pkLimitTime->Visible(true);
	}
	XUI::CXUI_Wnd* pkIconSlot = pkWnd->GetControl(L"IMG_ICON");
	if(pkIconSlot)
	{
		CashItemDrawToScaling(pkIconSlot, rkArticle);
	}
}

std::wstring lwCashShop::MakeTimeStrW(const BM::PgPackedTime& rkTime)
{
	if(rkTime.IsNull()) return std::wstring();

	BM::vstring kstrDay(L"20");
	if(rkTime.Year() < 10) kstrDay += _T("0");
	kstrDay += rkTime.Year();
	kstrDay += TTW(403262);
	if(rkTime.Month() < 10) kstrDay += _T("0");
	kstrDay += rkTime.Month();
	kstrDay += TTW(403262);
	if(rkTime.Day() < 10) kstrDay += _T("0");
	kstrDay += rkTime.Day();
	kstrDay += _T(" ");

	if(rkTime.Hour() < 12)
	{
		kstrDay += TTW(3541);
		kstrDay += _T(" ");
	}
	else
	{
		kstrDay += TTW(3542);
		kstrDay += _T(" ");
	}

	if(rkTime.Hour() < 10) kstrDay += _T("0");
	if(rkTime.Hour() > 12) kstrDay += rkTime.Hour() - 12;
	else kstrDay += rkTime.Hour();
	kstrDay += _T(":");
	if(rkTime.Min() < 10) kstrDay += _T("0");
	kstrDay += rkTime.Min();

	return kstrDay;
}

std::wstring const lwCashShop::MakeUseTimeStr(BYTE const bSaleType, int const UseValue, EUseItemType TimeType)
{
	BM::vstring vStr(UseValue);
	if( 0 != UseValue )
	{
		switch( TimeType )
		{
		case UIT_DAY:	{ vStr += TTW(174);	  }break;//일
		case UIT_HOUR:	{ vStr += TTW(90004); }break;//시
		case UIT_MIN:	{ vStr += TTW(176);	  }break;//분
		case UIT_NOLIMIT:
			{
			}break;
		case UIT_COUNT:
			{ 
				vStr += TTW(50413);//개
			}break;
		}
	}
	else
	{
		vStr = TTW(178);
	}
	return vStr;
}

std::wstring const lwCashShop::MakeBuyValueStr(TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, int const iBuyType)
{
	BM::vstring	vStr;
	if( CIBT_RENEW == iBuyType )
	{
		vStr += ((rkPrice.IsExtendDiscount) ? rkPrice.iDiscountExtendCash : rkPrice.iExtendCash);
	}
	else
	{
		vStr += ((rkPrice.IsDiscount) ? rkPrice.iDiscountCash : rkPrice.iCash);
	}

	vStr += L" ";
	switch(g_kCashShopMgr.GetShopType())
	{
	case PgCashShop::ECASH_SHOP:
		{
			vStr += TTW(403062);
		}break;
	case PgCashShop::EMILEAGE_SHOP:
		{
			vStr += TTW(1819);
		}break;
	default:
		{
			//_PgMessageBox("lwCashShop::MakeBuyValueStr", "Unknown shop type");
		}break;
	}
	return vStr;
}

void lwCashShop::MakeUseTimeStr(BYTE const bSaleType, TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, BM::vstring& rkStr, int const iItemCount, int const iBuyType)
{
	int iTime = rkPrice.iItemUseTime;
	if(iTime)
	{
		rkStr=iTime;
		switch(rkPrice.bTimeType)
		{
		case UIT_DAY:
			{//일
				rkStr+=TTW(174);
			}break;
		case UIT_HOUR:
			{//시간
				rkStr+=TTW(90004);
			}break;
		case UIT_MIN:
			{//분
				rkStr+=TTW(176);
			}break;
		case UIT_NOLIMIT:
			{
			}break;
		case UIT_COUNT:
			{ 
				rkStr += TTW(50413);//개
			}break;
		}
	}
	else
	{
		rkStr = TTW(178);	//무기한
	}

	rkStr+=L" / ";//{C=0xFFfff799/}";
	if( CIBT_RENEW == iBuyType )
	{
		rkStr+= (rkPrice.IsExtendDiscount) ? rkPrice.iDiscountExtendCash : rkPrice.iExtendCash;
	}
	else
	{
		rkStr+= (rkPrice.IsDiscount) ? rkPrice.iDiscountCash : rkPrice.iCash;
	}
	rkStr+=L" ";
	if(IsBuyingCashItem())
	{
		rkStr+=TTW(403062);
	}
	else
	{
		rkStr+=TTW(1819);
	}
}

XUI::CXUI_Wnd* lwCashShop::AddPriceToList(BYTE const bSaleType, TBL_DEF_CASH_SHOP_ITEM_PRICE const& rkPrice, XUI::CXUI_List* pkList, int const iItemCount, int const iBuyType)
{
	if(!pkList)	{return NULL;}

	BM::vstring kStr;
	MakeUseTimeStr(bSaleType, rkPrice, kStr, iItemCount, iBuyType);
	if( CIBT_RENEW == iBuyType )
	{
		if( (0 == rkPrice.iExtendCash) || (rkPrice.IsExtendDiscount && rkPrice.iDiscountExtendCash) )
		{
			return NULL;
		}
	}

	XUI::SListItem* pkItem = pkList->AddItem(kStr);
	if(pkItem)
	{
		pkItem->m_pWnd->SetCustomData(&rkPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
		pkItem->m_pWnd->Text(kStr);

		return pkItem->m_pWnd;
	}

	return NULL;
}

inline void CheckBoxDayInit(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd ){ return; }
	for(int i = 0; i < 3; ++i)
	{
		BM::vstring vStr(L"CBTN_DAY");
		vStr += i;

		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(vStr));
		if( pCheck )
		{
			pCheck->Text(L"");
			pCheck->ClickLock(false);
			pCheck->Check(false);
			pCheck->Visible(false);
		}
	}
}


void lwCashShop::InitUI(XUI::CXUI_Wnd* pkWnd)
{
	if (!pkWnd)
	{
		return;
	}

	pkWnd->Text(L"");
	pkWnd->ClearCustomData();

	XUI::CXUI_Wnd* pkNew = pkWnd->GetControl(L"IMG_NEW");
	if (pkNew)
	{
		pkNew->Visible(false);
	}
	XUI::CXUI_Wnd* pkCool = pkWnd->GetControl(L"IMG_COOL");
	if (pkCool)
	{
		pkCool->Visible(false);
	}
	XUI::CXUI_Wnd* pkMark = pkWnd->GetControl(L"IMG_MARK");
	if (pkMark)
	{
		pkMark->Visible(false);
	}
	XUI::CXUI_Wnd* pkMark2 = pkWnd->GetControl(L"IMG_MARK2");
	if (pkMark2)
	{
		pkMark2->Visible(false);
	}
	XUI::CXUI_Wnd* pkDay = pkWnd->GetControl(L"FRM_DAY");
	if (pkDay)
	{
		pkDay->Text(L"");
		pkDay->ClearCustomData();
	}
	XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_ICON");
	if(pkImg)
	{
		pkImg->Visible(false);
	}
	XUI::CXUI_Wnd* pkMiniIcon = pkWnd->GetControl(L"IMG_PARTS");
	if(pkMiniIcon)
	{
		pkMiniIcon->Visible(false);
	}


	CheckBoxDayInit( pkWnd->GetControl(L"FRM_INPUT0") );
	CheckBoxDayInit( pkWnd->GetControl(L"FRM_INPUT1") );
	CheckBoxDayInit( pkWnd->GetControl(L"FRM_INPUT2") );
	CheckBoxDayInit( pkWnd->GetControl(L"FRM_INPUT3") );
}

void lwCashShop::InitLimitedUI(XUI::CXUI_Wnd* pkWnd)
{
	if (!pkWnd)
	{
		return;
	}

	pkWnd->Text(L"");
	pkWnd->ClearCustomData();

	SetLimitedStateUI(pkWnd, LIMITEDSTATE_NA);

	XUI::CXUI_Wnd* pkSale = pkWnd->GetControl(L"FRM_STATETITLE");
	if(pkSale)
	{
		pkSale->Text(L"");
		pkSale->Visible(false);
	}
	XUI::CXUI_Wnd* pkRemain = pkWnd->GetControl(L"FRM_REMAIN");
	if(pkRemain)
	{
		pkRemain->Text(L"");
		pkRemain->Visible(false);
	}
	XUI::CXUI_Wnd* pkCountDown = pkWnd->GetControl(L"FRM_LIMITEDTYPE");
	if(pkCountDown)
	{
		pkCountDown->Text(L"");
		pkCountDown->Visible(false);
	}
	XUI::CXUI_Wnd* pkPrice = pkWnd->GetControl(L"FRM_PRICE");
	if(pkPrice)
	{
		pkPrice->Text(L"");
		pkPrice->Visible(false);
	}
	XUI::CXUI_Wnd* pkBeginDay = pkWnd->GetControl(L"FRM_BEGINDAY");
	if(pkBeginDay)
	{
		pkBeginDay->Text(L"");
		pkBeginDay->Visible(false);
	}
	XUI::CXUI_Wnd* pkEndDay = pkWnd->GetControl(L"FRM_ENDDAY");
	if(pkEndDay)
	{
		pkEndDay->Text(L"");
		pkEndDay->Visible(false);
	}
	XUI::CXUI_Wnd* pkLimitTime = pkWnd->GetControl(L"FRM_LIMITTIME");
	if(pkLimitTime)
	{
		pkLimitTime->Visible(false);
	}
	XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(L"IMG_ICON");
	if(pkIcon)
	{
		pkIcon->Visible(false);
	}
}

void lwCashShop::InitPageUI(XUI::CXUI_Wnd* pkWnd, int const iMaxItems, const PgCashShop::ECashShopForm eFormType)	//아이템용임. 선물함용은 따로 만들어야 함
{
	if(!pkWnd){ return; }
	XUI::CXUI_Wnd* pkFistBtn = pkWnd->GetControl(L"BTN_FIRST");
	XUI::CXUI_Wnd* pkFistBtnBG = pkWnd->GetControl(L"BTN_FIRST_BG");
	XUI::CXUI_Wnd* pkPrevBtn = pkWnd->GetControl(L"BTN_PREV");
	XUI::CXUI_Wnd* pkPrevBtnBG = pkWnd->GetControl(L"BTN_PREV_BG");
	XUI::CXUI_Wnd* pkNextBtn = pkWnd->GetControl(L"BTN_NEXT");
	XUI::CXUI_Wnd* pkNextBtnBG = pkWnd->GetControl(L"BTN_NEXT_BG");
	XUI::CXUI_Wnd* pkEndBtn = pkWnd->GetControl(L"BTN_END");
	XUI::CXUI_Wnd* pkEndBtnBG = pkWnd->GetControl(L"BTN_END_BG");
	if(!pkFistBtn || !pkFistBtnBG || !pkPrevBtn || !pkPrevBtnBG || !pkNextBtn || !pkNextBtnBG
		|| !pkEndBtn || !pkEndBtnBG)
	{
		return;
	}
	pkWnd->Visible(true);

	if( !iMaxItems )
	{
		pkWnd->Visible(false);
		return;
	}

	int iSlotSize = 0;
	switch( eFormType )
	{
	case PgCashShop::ECSFORM_GIFT:		{ iSlotSize = CASH_GIFT_PER_PAGE; }break;
	case PgCashShop::ECSFORM_COMMON:	{ iSlotSize = CASH_ITEM_PER_PAGE; }break;
	case PgCashShop::ECSFORM_LIMITED:	{ iSlotSize = CASH_LIMITED_PER_PAGE; }break;
	}
	int iStartPage = 0;
	int const iTotalPage = (iMaxItems / iSlotSize) + ((iMaxItems % iSlotSize)?(1):(0));
	int iInitPage = iStartPage + 5;
	if( iInitPage > iTotalPage )
	{
		iInitPage = iTotalPage;
	}

	bool bIsGoEndVisible = true;
	if( iTotalPage <= 5 )
	{
		bIsGoEndVisible = false;
		if( iTotalPage <= 1 )
		{
			pkWnd->Visible(false);
		}
	}

	pkFistBtn->Visible(bIsGoEndVisible);	
	pkFistBtnBG->Visible(bIsGoEndVisible);
	pkEndBtn->Visible(bIsGoEndVisible);		
	pkEndBtnBG->Visible(bIsGoEndVisible);

	pkPrevBtn->Visible(bIsGoEndVisible);	
	pkPrevBtnBG->Visible(bIsGoEndVisible);
	pkPrevBtn->SetCustomData(&iStartPage, sizeof(iStartPage));
	pkNextBtn->Visible(bIsGoEndVisible);	
	pkNextBtnBG->Visible(bIsGoEndVisible);
	pkNextBtn->SetCustomData(&iStartPage, sizeof(iStartPage));

	SetUIPageInfo(pkWnd, iInitPage, iStartPage, true);
}

void lwCashShop::SetUIPageInfo(XUI::CXUI_Wnd* pkWnd, int const iMaxPage, int const iStartPage, bool const bIsReset)
{
	if(!pkWnd){ return; }
	XUI::CXUI_Wnd* pkPrevBtn = pkWnd->GetControl(L"BTN_PREV");
	XUI::CXUI_Wnd* pkNextBtn = pkWnd->GetControl(L"BTN_NEXT");
	if(!pkPrevBtn || !pkNextBtn )
	{
		return;
	}

	pkPrevBtn->SetCustomData(&iStartPage, sizeof(iStartPage));
	pkNextBtn->SetCustomData(&iStartPage, sizeof(iStartPage));

	for(int i = 0; i < MAX_PAGE_SLOT; ++i)
	{
		BM::vstring	vStr(L"CBTN_NUM");
		vStr += i;
		XUI::CXUI_Wnd* pPage = pkWnd->GetControl(vStr);
		if( pPage )
		{
			pPage->Visible(true);
			if( (iStartPage + i) < iMaxPage )
			{
				vStr = ((iStartPage + i) + 1);
				pPage->Text(vStr);
				XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pPage);
				pCheck->ClickLock(false);
				pCheck->Check(false);

				if( bIsReset && i == 0 )
				{
					pCheck->Check(true);
					pCheck->ClickLock(true);
				}
			}
			else
			{
				pPage->Visible(false);
			}
		}
	}
}

void lwCashShop::lwCallCashShop()
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		if ( BM::GUID::NullData() != pkPlayer->VendorGuid() )
		{// 입장할 수 없는 곳
			lwAddWarnDataTT(799454);
			return;
		}
	}
	if( false == lua_tinker::call<bool>("CheckActionForEnterCashShop") )
	{
		return;
	}
	if(g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_DEFAULT) || g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME| GATTR_FLAG_BATTLESQUARE))
		&& !g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND) )
	{
		g_kHomeTownMgr.UseCashShop(true);

		SPT_C_M_CS_REQ_ENTER_CASHSHOP kStruct(g_kCashShopMgr.ValueKey());
		BM::Stream kPacket;
		kStruct.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
		return;
	}
	lwAddWarnDataTT(1935);
}

void lwCashShop::TopCategoryOnClick(lwUIWnd kWnd, int const iCategory)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if(!pkWnd)
	{
		return;
	}

	TBL_DEF_CASH_SHOP_CATEGORY kCate;
	if( g_kCashShopMgr.GetCategory(iCategory, kCate) )
	{
		int iSelectCate = -1;
		CONT_DEF_CASH_SHOP::const_iterator art_it = kCate.kSubCategory.begin();
		for( int i = 0; i < 12; ++i)	//중분류 목록
		{
			BM::vstring kTabID(L"CBTN_SECOND_TAB");
			kTabID+=i;
			XUI::CXUI_Wnd* pkTab = pkWnd->GetControl(kTabID);
			if (pkTab)
			{
				POINT2 kPos(666,77);
				POINT2 kSize(1,20);
				bool bClose = true;
				XUI::CXUI_CheckButton *pChkBtn= dynamic_cast<XUI::CXUI_CheckButton*>(pkTab);
				if (pChkBtn)
				{
					bool bCheckState = false;
					if( art_it != kCate.kSubCategory.end() && kCate.kSubCategory.size() > i )
					{
						bClose = false;
						pkTab->Size(POINT2(200,20));//임시
						pkTab->Text((*art_it).second.wstrName);
						kSize.x = lwUIWnd(pkTab).GetTextSize().GetX()+1;
						pChkBtn->Disable(!art_it->second.bIsUsed);
						if( 0 == i )
						{
							kPos.x = 52;
						}
						else
						{
							kTabID = L"CBTN_SECOND_TAB";
							kTabID+=(i-1);
							XUI::CXUI_Wnd *pkPreChkBtn = pkWnd->GetControl(kTabID);
							if (pkPreChkBtn)
							{
								kPos.x = pkPreChkBtn->Location().x + pkPreChkBtn->Size().x + 13;//13은 공백
							}
						}
						if (pChkBtn->Enable() && -1 == iSelectCate)//첫번째면
						{
							pChkBtn->ClickLock(true);
							bCheckState = true;
							iSelectCate = i;
						}
						++art_it;
					}
					else
					{	
						pkTab->Text(L"");
					}
					pChkBtn->Check(bCheckState);
				}

				pkTab->Size(kSize);
				pkTab->Location(kPos);
				pkTab->IsClosed(bClose);
			}
		}
		if(-1 == iSelectCate)
		{// 카테고리 UI가 활성화 되어있지 않은 상태 이므로 종료
			return;
		}

		SCateInfo	Info;
		Info.iTab = iCategory;
		Info.iSub = iSelectCate;
		pkWnd->GetControl(L"FRM_PAGE")->ClearCustomData();
		pkWnd->GetControl(L"FRM_PAGE")->SetCustomData(&Info, sizeof(Info));

		if ( !kCate.kSubCategory.empty() )	// 첫 하위 카테고리의 첫번째 아이템 
		{
			CONT_DEF_CASH_SHOP::const_iterator art_it = kCate.kSubCategory.begin();
			if( art_it != kCate.kSubCategory.end() )
			{
				int i = 0;
				while(true)
				{
					if( i == iSelectCate )
					{
						break;
					}
					++i;
					if( art_it != kCate.kSubCategory.end() )
					{
						++art_it;
					}
				}
			}

			if(IsLimitedCategory((*art_it).second))
			{ //선택된 카테고리가 한정판매인지 알아보고 맞으면 한정판매 폼으로 변경
				CONT_CASH_SHOP_ITEM_LIMITSELL kContLimitedInfo;
				g_kCashShopMgr.GetLimitedInfo(kContLimitedInfo);
				SetLimitedPage(pkWnd, (*art_it).second.kContArticle, kContLimitedInfo, 0);
			}
			else
			{ //공용폼
				InitPageUI(pkWnd->GetControl(L"FRM_PAGE"), (*art_it).second.kContArticle.size());
				SetShopPage(pkWnd, (*art_it).second.kContArticle, 0);
			}

		}
		else//if ( !kCate.kContArticle.empty() )	//최상위 카테고리에 아이템이 있으면
		{
			InitPageUI(pkWnd->GetControl(L"FRM_PAGE"), kCate.kContArticle.size());
			SetShopPage(pkWnd, kCate.kContArticle, 0);
		}
	}

	bool bGift = (iCategory == PgCashShop::ECSCATEGORY_GIFT);
//	pkWnd->GetControl(L"FRM_GIFT")->Visible(bGift);
//	pkWnd->GetControl(L"FRM_GOODS")->Visible(!bGift);


	if(bGift)
	{
		SetShopForm(pkWnd, PgCashShop::ECSFORM_GIFT);
		for( int i = 0; i < 11; ++i)	//중분류 목록
		{
			BM::vstring kTabID(L"CBTN_SECOND_TAB");
			kTabID+=i;
			XUI::CXUI_Wnd* pkTab = pkWnd->GetControl(kTabID);
			if (pkTab)
			{
				POINT2 kPos(666,77);
				POINT2 kSize(1,20);
				bool bClose = true;
				XUI::CXUI_CheckButton *pChkBtn= dynamic_cast<XUI::CXUI_CheckButton*>(pkTab);
				if (pChkBtn)
				{
					bool bCheckState = false;
					pChkBtn->Disable(false);
					if(i<2)
					{
						bClose = false;

						pkTab->Size(POINT2(200,20));//임시
						pkTab->Text(TTW(1890+i));
						kSize.x = lwUIWnd(pkTab).GetTextSize().GetX()+1;
						if (0==i)//첫번째면
						{
							kPos.x = 52;
							pChkBtn->ClickLock(true);
							bCheckState = true;
						}
						else
						{
							kTabID = L"CBTN_SECOND_TAB";
							kTabID+=(i-1);
							XUI::CXUI_Wnd *pkPreChkBtn = pkWnd->GetControl(kTabID);
							if (pkPreChkBtn)
							{
								kPos.x = pkPreChkBtn->Location().x + pkPreChkBtn->Size().x + 13;//13은 공백
							}
						}
					}
					else
					{
						pkTab->Text(L"");
					}
					pChkBtn->Check(bCheckState);
				}
				pkTab->Size(kSize);
				pkTab->Location(kPos);
				pkTab->IsClosed(bClose);
			}
		}

		CONT_CASHGIFTINFO kCont;
		g_kCashShopMgr.GetGiftContRecv(kCont);
		SCateInfo	Info;
		Info.iTab = iCategory;
		pkWnd->GetControl(L"FRM_PAGE")->ClearCustomData();
		pkWnd->GetControl(L"FRM_PAGE")->SetCustomData(&Info, sizeof(Info));
		InitPageUI(pkWnd->GetControl(L"FRM_PAGE"), g_kCashShopMgr.GetGiftRecvCount(), PgCashShop::ECSFORM_GIFT);
		SetGiftPage(pkWnd, kCont);
	}
}

void lwCashShop::SetCategoryPageClick(lwUIWnd kWnd, int const iBtnType)
{
	XUI::CXUI_Wnd* pParent = kWnd.GetSelf()->Parent();
	if( !pParent ){ return;	}

	int iNow = 0;
	int iStartPage = 0;
	iNow = _wtoi(kWnd.GetStaticText()().c_str()) - 1;
	iStartPage = (iNow / MAX_PAGE_SLOT) * MAX_PAGE_SLOT;

	// 페이지 변경
	XUI::CXUI_Wnd* pkShop = pParent->Parent();
	if( !pkShop ){ return; }

	int iViewType = 0;
	SCateInfo	Info;
	pParent->GetCustomData(&Info, sizeof(Info));

	switch( g_kCashShopMgr.GetShopSubType() )
	{
	case PgCashShop::ECSFORM_GIFT:
		{
			if( iBtnType != PBT_NORMAL )
			{
				iStartPage = kWnd.GetCustomData<int>();

				size_t kCount = 0;
				if( !Info.iSub )
				{
					kCount = g_kCashShopMgr.GetGiftRecvCount();
				}
				else
				{
					kCount = g_kCashShopMgr.GetGiftSendCount();
				}
				int const iTotalPage = (kCount / CASH_GIFT_PER_PAGE) + ((kCount % CASH_GIFT_PER_PAGE)?(1):(0));

				BM::vstring	vStr(kWnd.GetID()());
				vStr += L"_BG";
				lwUIWnd lwBgImg(pParent->GetControl(vStr));
				if( !lwBgImg.IsNil() )
				{
					assert(0);
				}

				switch(iBtnType)
				{
				case PBT_JUMP:
					{
						int iCalcNum = MAX_PAGE_SLOT * lwBgImg.GetCustomData<int>();
						if(((iStartPage + iCalcNum) >= 0) 
							&& ((iStartPage + iCalcNum) < iTotalPage) )
						{
							iStartPage += iCalcNum;
						}
						iNow = iStartPage;
					}break;
				case PBT_LAST:
					{
						if( lwBgImg.GetCustomData<int>() )
						{
							iStartPage = 0;
							iNow = iStartPage;
						}
						else
						{
							iStartPage = (iTotalPage / MAX_PAGE_SLOT) * MAX_PAGE_SLOT;
							iNow = iTotalPage - 1;
						}
					}break;
				default:
					break;
				}

				int iInitPage = iStartPage + 5;
				if( iInitPage > iTotalPage )
				{
					iInitPage = iTotalPage;
				}
				SetUIPageInfo(pkShop->GetControl(L"FRM_PAGE"), iInitPage, iStartPage, true);
			}

			CONT_CASHGIFTINFO	kCate;
			if( !Info.iSub )
			{
				g_kCashShopMgr.GetGiftContRecv(kCate, iNow);
			}
			else
			{
				g_kCashShopMgr.GetGiftContSend(kCate, iNow);
			}

			SetGiftPage(pkShop, kCate, !((bool)Info.iSub));
		}break;
	case PgCashShop::ECSFORM_LIMITED:
		{
			TBL_DEF_CASH_SHOP_CATEGORY kCate;
			if( g_kCashShopMgr.GetSubategory(Info.iSub, kCate) )
			{
				size_t const kCount = g_kCashShopMgr.GetLimitedSize();
				int const iTotalPage = (kCount / CASH_LIMITED_PER_PAGE) + ((kCount % CASH_LIMITED_PER_PAGE)?(1):(0));

				if( iBtnType != PBT_NORMAL )
				{
					iStartPage = kWnd.GetCustomData<int>();

					BM::vstring	vStr(kWnd.GetID()());
					vStr += L"_BG";
					lwUIWnd lwBgImg(pParent->GetControl(vStr));
					if( !lwBgImg.IsNil() )
					{
						assert(0);
					}

					switch(iBtnType)
					{
					case PBT_JUMP:
						{
							int iCalcNum = MAX_PAGE_SLOT * lwBgImg.GetCustomData<int>();
							if(((iStartPage + iCalcNum) >= 0) 
								&& ((iStartPage + iCalcNum) < iTotalPage) )
							{
								iStartPage += iCalcNum;
							}
							iNow = iStartPage;
						}break;
					case PBT_LAST:
						{
							if( lwBgImg.GetCustomData<int>() )
							{
								iStartPage = 0;
								iNow = iStartPage;
							}
							else
							{
								iStartPage = (iTotalPage / MAX_PAGE_SLOT);
								if( iTotalPage != 0 && (iTotalPage % MAX_PAGE_SLOT) == 0 )
								{
									iStartPage -= 1;
								}
								iStartPage *= MAX_PAGE_SLOT;
								iNow = iTotalPage - 1;
							}
						}break;
					default:
						break;
					}

					int iInitPage = iStartPage + 5;
					if( iInitPage > iTotalPage )
					{
						iInitPage = iTotalPage;
					}

					SetUIPageInfo(pkShop->GetControl(L"FRM_PAGE"), iInitPage, iStartPage, true);
				}

				CONT_CASH_SHOP_ITEM_LIMITSELL kContLimitedInfo;
				if(g_kCashShopMgr.GetLimitedInfo(kContLimitedInfo))
					SetLimitedPage(pkShop, kCate.kContArticle, kContLimitedInfo, iNow);
			}
		}
		break;
	case PgCashShop::ECSFORM_COMMON:
		{
			TBL_DEF_CASH_SHOP_CATEGORY kCate;
			if( g_kCashShopMgr.GetSubategory(Info.iSub, kCate) )
			{
				size_t const kCount = kCate.kContArticle.size();
				int const iTotalPage = (kCount / CASH_ITEM_PER_PAGE) + ((kCount % CASH_ITEM_PER_PAGE)?(1):(0));

				if( iBtnType != PBT_NORMAL )
				{
					iStartPage = kWnd.GetCustomData<int>();

					BM::vstring	vStr(kWnd.GetID()());
					vStr += L"_BG";
					lwUIWnd lwBgImg(pParent->GetControl(vStr));
					if( !lwBgImg.IsNil() )
					{
						assert(0);
					}

					switch(iBtnType)
					{
					case PBT_JUMP:
						{
							int iCalcNum = MAX_PAGE_SLOT * lwBgImg.GetCustomData<int>();
							if(((iStartPage + iCalcNum) >= 0) 
								&& ((iStartPage + iCalcNum) < iTotalPage) )
							{
								iStartPage += iCalcNum;
							}
							iNow = iStartPage;
						}break;
					case PBT_LAST:
						{
							if( lwBgImg.GetCustomData<int>() )
							{
								iStartPage = 0;
								iNow = iStartPage;
							}
							else
							{
								iStartPage = (iTotalPage / MAX_PAGE_SLOT);
								if( iTotalPage != 0 && (iTotalPage % MAX_PAGE_SLOT) == 0 )
								{
									iStartPage -= 1;
								}
								iStartPage *= MAX_PAGE_SLOT;
								iNow = iTotalPage - 1;
							}
						}break;
					default:
						break;
					}

					int iInitPage = iStartPage + 5;
					if( iInitPage > iTotalPage )
					{
						iInitPage = iTotalPage;
					}

					SetUIPageInfo(pkShop->GetControl(L"FRM_PAGE"), iInitPage, iStartPage, true);
				}

				SetShopPage(pkShop, kCate.kContArticle, iNow);
			}
		}break;
	}

	// 눌림정보 초기화
	for(int i = 0; i < 5; ++i)
	{
		BM::vstring	vStr(L"CBTN_NUM");
		vStr += i;
		XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
		if( !pBtn )
		{
			assert(0);
			continue;
		}

		if( pBtn->Check() )
		{
			pBtn->ClickLock(false);
			pBtn->Check(false);
			break;
		}
	}

	switch( iBtnType )
	{
	case PBT_NORMAL:
		{
			kWnd.CheckState(true);
			kWnd.LockClick(true);
		}break;
	case PBT_JUMP:
	case PBT_LAST:
		{
			BM::vstring	vStr(L"CBTN_NUM");
			vStr += (iNow % MAX_PAGE_SLOT);
			XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
			if( !pBtn )
			{
				assert(0);
			}
			pBtn->Check(true);
			pBtn->ClickLock(true);
		}break;
	}
}

void lwCashShop::SubCategoryOnClick(lwUIWnd kWnd, int const iCategory)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if(!pkWnd) {return;}
	XUI::CXUI_Wnd* pkParent = pkWnd->Parent();
	if(!pkParent) {return;}

	SCateInfo	Info;
	pkParent->GetControl(L"FRM_PAGE")->GetCustomData(&Info, sizeof(Info));
	Info.iSub = iCategory;
	pkParent->GetControl(L"FRM_PAGE")->ClearCustomData();
	pkParent->GetControl(L"FRM_PAGE")->SetCustomData(&Info, sizeof(Info));

	if(pkWnd->Text() == TTW(1890))
	{
		CONT_CASHGIFTINFO	kCate;
		g_kCashShopMgr.GetGiftContRecv(kCate);
		InitPageUI(pkParent->GetControl(L"FRM_PAGE"), g_kCashShopMgr.GetGiftRecvCount(), PgCashShop::ECSFORM_GIFT);
		SetGiftPage(pkParent, kCate, true);
	}
	else if(pkWnd->Text() == TTW(1891))
	{
		CONT_CASHGIFTINFO	kCate;
		g_kCashShopMgr.GetGiftContSend(kCate);
		InitPageUI(pkParent->GetControl(L"FRM_PAGE"), g_kCashShopMgr.GetGiftSendCount(), PgCashShop::ECSFORM_GIFT);
		SetGiftPage(pkParent, kCate, false);
	}
	else //선물함이 아니라면
	{
		TBL_DEF_CASH_SHOP_CATEGORY kCate;
		if( g_kCashShopMgr.GetSubategory(iCategory, kCate) )
		{
			if(IsLimitedCategory(kCate)) //한정판매 항목인가?
			{
				CONT_CASH_SHOP_ITEM_LIMITSELL kContLimitedInfo;
				if(g_kCashShopMgr.GetLimitedInfo(kContLimitedInfo))
					SetLimitedPage(pkParent, kCate.kContArticle, kContLimitedInfo, 0);
			}
			else
			{
				InitPageUI(pkParent->GetControl(L"FRM_PAGE"), kCate.kContArticle.size());
				SetShopPage(pkParent, kCate.kContArticle, 0);
			}
		}
	}
}

void lwCashShop::UpdateLimitedPage(XUI::CXUI_Wnd* pkWnd)
{
	if(!pkWnd)
	{
		return;
	}

	XUI::CXUI_Wnd* pkElem;
	int iIndex;
	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	CONT_CASH_SHOP_ITEM_LIMITSELL kContLimited;
	CONT_CASH_SHOP_ITEM_LIMITSELL::const_iterator iterLimited;

	if(!g_kCashShopMgr.GetLimitedInfo(kContLimited))
	{
		return;
	}

	for(int i = 0; i < 4; i++)
	{
		BM::vstring strWnd(L"FRM_LIMITED_ELE");
		strWnd += i;
		pkElem = pkWnd->GetControl(strWnd);
		if(pkElem)
		{
			iIndex = 0;
			pkElem->GetCustomData(&iIndex, sizeof(int));
			if(iIndex == 0 || !g_kCashShopMgr.FindArticleByKey(TBL_KEY_INT(iIndex), kArticle) ||
				(iterLimited = kContLimited.find(TBL_KEY_INT(iIndex))) == kContLimited.end() )
			{
				continue;
			}

			WriteToLimitedUI(pkElem, kArticle, iterLimited->second);

		}
	}
}

void lwCashShop::lwDayBtnOnClick(lwUIWnd kWnd, int const iBuyType)
{// 아이템 구매 창에서 기간제 버튼 클릭시
	if (kWnd.IsNil()) { return; }
	XUI::CXUI_CheckButton* pkWnd = dynamic_cast<XUI::CXUI_CheckButton*>(kWnd());

	XUI::CXUI_Wnd* pkInput = pkWnd->Parent();
	if (!pkInput)	{return;}
	XUI::CXUI_Wnd* pkArticleWnd = pkInput->Parent();
	if (!pkArticleWnd)
	{
		pkArticleWnd = pkInput;
	}
	
	int iIndex =0;
	int iSlotIDX = 0;
	switch( iBuyType )
	{
	case CIBT_BASKET:
	case CIBT_BASKET_PART:
		{
			pkArticleWnd = pkInput;

			lwPacket kPacket = lwUIWnd(pkArticleWnd).GetCustomDataAsPacket();
			iSlotIDX = kPacket.PopInt();
			iIndex = kPacket.PopInt();
		}break;
	default:
		{
			pkArticleWnd->GetCustomData(&iIndex, sizeof(iIndex));
		}break;
	}
	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if(!g_kCashShopMgr.FindPageArticle(iIndex, kArticle))
	{
		if(!g_kCashShopMgr.GetItem(iIndex, kArticle))
		{
			return;
		}
	}

	TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
	pkWnd->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));

	for(int i = 0; i < 3; ++i)
	{
		BM::vstring vStr(L"CBTN_DAY");
		vStr += i;

		XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkInput->GetControl(vStr));
		if( pBtn )
		{
			pBtn->ClickLock(false);
			pBtn->Check(false);
		}
	}
	pkWnd->Check(true);
	pkWnd->ClickLock(true);

	XUI::CXUI_Wnd* pFormMain = NULL;
	__int64 i64TotalCost = 0;
	switch( iBuyType )
	{
	case CIBT_NONE:
		{
			XUI::CXUI_Wnd* pkDay = pkArticleWnd->GetControl(L"FRM_DAY");
			if (!pkDay)	{return;}
			{
				pkDay->Text(MakeBuyValueStr(kPrice));
			}
			pkDay->SetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			return;
		}
	case CIBT_BUY:
	case CIBT_STATIC_ITEM_BUY:
		{// 각 각 여기서 기간제 변화를 체크 할수 있다
			i64TotalCost = (kPrice.IsDiscount)?(kPrice.iDiscountCash):(kPrice.iCash);
			XUI::CXUI_Wnd* pBuy = pkArticleWnd->GetControl(L"BTN_BUY");
			if( pBuy )
			{
				g_kCashShopMgr.SetRememberTotalPrice(i64TotalCost);
				pBuy->SetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
				{// 마일리지 값을 기억
					__int64 i64CalculatedMileage = g_kCashShopMgr.CalcAddBonus(i64TotalCost, kArticle.iMileage);
					i64CalculatedMileage += g_kCashShopMgr.CalcLocalAddBonus(i64TotalCost);
					g_kCashShopMgr.SetRememberItemMileage(i64CalculatedMileage);
				}
			}
		}break;
	case CIBT_RENEW:
	case CIBT_STATIC_ITEM_RENEW:
		{// 각 각 여기서 기간제 변화를 체크 할수 있다
			i64TotalCost = (kPrice.IsExtendDiscount)?(kPrice.iDiscountExtendCash):(kPrice.iExtendCash);
			XUI::CXUI_Wnd* pBuy = pkArticleWnd->GetControl(L"BTN_BUY");
			if( pBuy )
			{
				g_kCashShopMgr.SetRememberTotalPrice(i64TotalCost);
				pBuy->SetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
				{// 마일리지 값을 기억
					__int64 i64CalculatedMileage = g_kCashShopMgr.CalcAddBonus(i64TotalCost, kArticle.iMileage);
					i64CalculatedMileage += g_kCashShopMgr.CalcLocalAddBonus(i64TotalCost);
					g_kCashShopMgr.SetRememberItemMileage(i64CalculatedMileage);
				}				
			}
		}break;
	case CIBT_PV_ALL:
		{// 각 각 여기서 기간제 변화를 체크 할수 있다
			g_kCashShopMgr.UpDatePreiewItemCost(iIndex, kPrice);
			i64TotalCost = g_kCashShopMgr.GetPreviewItemTotalCost();
			g_kCashShopMgr.SetRememberTotalPrice(i64TotalCost);
			{// 최상위 UI얻음
				XUI::CXUI_Wnd* pTempWnd = pkArticleWnd->Parent();
				pkArticleWnd = pTempWnd->Parent();
			}			
		}break;
	case CIBT_BASKET:
		{
			g_kCashShopMgr.UpDateShoppingBasketItemCost(iSlotIDX, iIndex, kPrice);
			i64TotalCost = g_kCashShopMgr.GetBasketItemTotalCost();
			g_kCashShopMgr.SetRememberTotalPrice(i64TotalCost);
			{// 최상위 UI얻음
				XUI::CXUI_Wnd* pTempWnd = pkArticleWnd->Parent();
				pkArticleWnd = pTempWnd->Parent();
			}			
		}break;
	case CIBT_BASKET_PART:
		{
			g_kCashShopMgr.UpDateShoppingBasketItemCost(iSlotIDX, iIndex, kPrice);
			i64TotalCost = GetSelectedSlotItemTotalCost();
			g_kCashShopMgr.SetRememberTotalPrice(i64TotalCost);
			{// 최상위 UI얻음
				XUI::CXUI_Wnd* pTempWnd = pkArticleWnd->Parent();
				pkArticleWnd = pTempWnd->Parent();
			}
		}break;
	}

	__int64 i64MyCash = 0;
	PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
	if(pPilot != NULL)
	{
		i64MyCash = pPilot->GetAbil64(AT_CASH);
	}

	if( pkArticleWnd )
	{
		pkArticleWnd->GetControl(L"FRM_COST")->Text(BM::vstring(i64TotalCost));
		lwUpdateTotalCashExpression(lwUIWnd(pkArticleWnd));
	}	
}

void lwCashShop::lwBuyCashItemOnClick(lwUIWnd kWnd, int const iBuyType, int const iIndexFrom)
{
	if(CIBT_NONE==iBuyType) { return; }

	if (kWnd.IsNil()) { return; }
	XUI::CXUI_Wnd* pkWnd = kWnd();
	XUI::CXUI_Wnd* pkParent = pkWnd->Parent();
	if (!pkParent)	{return;}

	int iIndex = 0;
	if(0==iIndexFrom)
	{
		pkParent->GetCustomData(&iIndex, sizeof(int));	//물품번호
	}
	else
	{
		iIndex = iIndexFrom;
	}

	__int64 i64MyCash = 0;
	__int64 i64MyMileage = 0;
	PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
	if(pPilot != NULL)
	{
		i64MyCash = pPilot->GetAbil64(AT_CASH);
		i64MyMileage = pPilot->GetAbil64(AT_BONUS_CASH);
	}

	switch(iBuyType)
	{
	case CIBT_BUY:
	case CIBT_RENEW:
	case CIBT_GIFT:
		{// 아이템 구매, 기간 연장, 선물 구매 선택
			if(0>=iIndex)
			{
				return;
			}

			TBL_DEF_CASH_SHOP_ARTICLE kArticle;
			if(!g_kCashShopMgr.FindPageArticle(iIndex, kArticle))
			{
				if(!g_kCashShopMgr.GetItem(iIndex, kArticle))
				{
					return;
				}
			}

			BM::PgPackedTime kTempTime;
			kTempTime.Clear();
			if( CIBT_RENEW == iBuyType && !(kTempTime == kArticle.kSaleEndDate) )
			{//연장시 여기서 판매 종료 여부를 검사한다.
				SYSTEMTIME kTemp;
				if( g_kEventView.GetLocalTime(&kTemp) )
				{
					kTempTime = kTemp;
					if( kArticle.kSaleEndDate < kTempTime )
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1908, true);
						return;
					}
				}
			}

			TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
			XUI::CXUI_Wnd* pkDay = pkParent->GetControl(L"FRM_DAY");
			if (!pkDay && !kArticle.kContCashItemPrice.empty())	
			{
				CONT_CASH_SHOP_ITEM_PRICE::iterator	Price_iter = kArticle.kContCashItemPrice.begin();
				while( Price_iter != kArticle.kContCashItemPrice.end() )
				{
					kPrice = Price_iter->second;
					if( CIBT_RENEW != iBuyType )
					{
						if( 0 != kPrice.iItemUseTime )
						{
							break;
						}
					}
					else
					{
						if( 0 != kPrice.iExtendCash || ( kPrice.IsExtendDiscount && 0 != kPrice.iDiscountExtendCash ) )
						{
							if( 0 != kPrice.iItemUseTime )
							{
								break;
							}
						}
						else
						{
							Price_iter = kArticle.kContCashItemPrice.erase(Price_iter);
							continue;
						}
					}
					++Price_iter;
				}

				if( CIBT_RENEW == iBuyType && kArticle.kContCashItemPrice.empty() )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1909, true);
					return;
				}
			}
			else if( pkDay )
			{
				pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			}
			else
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1909, true);
				return;
			}

			if( 0>=kPrice.iItemUseTime && 0>=kPrice.bTimeType )
			{
				return;
			}
			if(kPrice.IsOnlyMileage /*&& CIBT_RENEW != iBuyType*/)
			{// 마일리지 아이템 구입 이라는것을 기억한다
				g_kCashShopMgr.SetBuyTypeUI(PgCashShop::EMILEAGE_SHOP);
			}
			else
			{// 마일리지 아이템 구입이 아니면 일반 캐시 아이템 구입으로 기억한다
				g_kCashShopMgr.SetBuyTypeUI(PgCashShop::ECASH_SHOP);
			}

			int iBuyCash = 0;
			switch(iBuyType)
			{
			case CIBT_BUY:
			case CIBT_GIFT:
				{// 일반 가격 할인 여부를 판단해 가격을 얻어옴
					iBuyCash = (kPrice.IsDiscount ? kPrice.iDiscountCash : kPrice.iCash);
				}break;
			case CIBT_RENEW:
				{// 기간 연장 가격 할인 여부를 판단해 가격을 얻어온다
					iBuyCash = (kPrice.IsExtendDiscount ? kPrice.iDiscountExtendCash : kPrice.iExtendCash);
				}break;
			default:
				{// 잘못된 구매 타입
					return;
				}break;
			}

			if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
			{
				// 한국은 톨+해피포인트 합산 가격 체크
				// 해피포인트 먼저 차감되고, 모자라는 금액은 톨에서 차감
				switch( iBuyType )
				{
				case CIBT_BUY:
				case CIBT_RENEW:
					{
						const __int64 i64MyUnitCoin = i64MyCash + i64MyMileage;
						if( i64MyUnitCoin < iBuyCash )
						{
							lwAddWarnDataTT(1900);
							return;
						}
					}break;
				case CIBT_GIFT: // 선물은 톨만 가능
					{
						if( i64MyCash < iBuyCash )
						{	
							lwAddWarnDataTT(1900);
							return;
						}
					}break;
				default:
					{
						return ;
					}break;
				}
			}
			else
			{
				if(IsBuyingCashItem())
				{//캐시 구입이면 캐시로 비교
					if(i64MyCash < iBuyCash)
					{
						lwAddWarnDataTT(1900);
						return;
					}
				}
				else
				{//마일리지 구입이면 소유 마일리지로 비교
					if(i64MyMileage < iBuyCash)
					{
						lwAddWarnDataTT(790216);
						return;
					}
					//이후 소지금은 마일리지 값으로 체크 해야 하므로
					i64MyCash = i64MyMileage;
				}
			}

			std::wstring kUIName;
			switch( iBuyType )
			{
			case CIBT_BUY:	{ kUIName = L"SFRM_CASHSHOP_BUY";	}break;
			case CIBT_RENEW:{ kUIName = L"SFRM_CASHSHOP_RENEW"; }break;
			case CIBT_GIFT:	{ kUIName = L"SFRM_GIFT_SEND";		}break;
			}

			XUI::CXUI_Wnd* pkBuyWnd = XUIMgr.Call(kUIName, true);
			if (!pkBuyWnd)	{return;}

			SetDayBtnInitialize(pkBuyWnd, kArticle, iBuyCash, iBuyType);

			pkBuyWnd->GetControl(L"BTN_BUY")->SetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			pkBuyWnd->SetCustomData(&iIndex, sizeof(int));
			{// 아이템 가격을 기억
				 g_kCashShopMgr.SetRememberTotalPrice(iBuyCash);
			}
			{// 유저에게 지급될 마일리지 값을 기억
				__int64 i64CalculatedMileage = g_kCashShopMgr.CalcAddBonus(iBuyCash, kArticle.iMileage);
				i64CalculatedMileage += g_kCashShopMgr.CalcLocalAddBonus(iBuyCash);
				g_kCashShopMgr.SetRememberItemMileage(i64CalculatedMileage);
			}
			if(CIBT_GIFT==iBuyType)
			{
				XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pkBuyWnd->GetControl(L"EDT_NAME"));
				if( pEdit )
				{
					pEdit->SetEditFocus(true);
				}
				break;
			}

			Quest::SetCutedTextLimitLength(pkBuyWnd->GetControl(L"FRM_NAME"), kArticle.wstrName, _T("..."));
			BM::vstring kstrPrice;
			int iItemCount = 0;
			CONT_CASH_SHOP_ITEM::const_iterator item_iter = kArticle.kContCashItem.begin();
			if( item_iter != kArticle.kContCashItem.end() )
			{
				iItemCount = item_iter->iItemCount;
			}
			MakeUseTimeStr(kArticle.bSaleType, kPrice, kstrPrice, iItemCount, iBuyType);

			if(IsBuyingCashItem())
			{
				OffAllMileageWnd(false, pkBuyWnd);
			}
			else
			{
				OffAllMileageWnd(true, pkBuyWnd);
			}

			if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
			{// 참고로 한국은 마일리지를 해피코인으로 사용한다.(개념은 틀림, 변수만 사용)
				pkBuyWnd->GetControl(L"FRM_COST")->Text(BM::vstring(iBuyCash));
				pkBuyWnd->GetControl(L"FRM_CASH")->Text(BM::vstring(i64MyCash));

				__int64 i64UseTol = 0i64;
				__int64 i64UseHappyCoin = 0i64;

				if( i64MyMileage < iBuyCash )
				{// 해피코인만으로 결제가 부족하면
					i64UseHappyCoin = i64MyMileage;			// 차감되는 해피코인: 현재 보유한 해피코인 전체
					i64UseTol = iBuyCash - i64UseHappyCoin; // 차감되는 톨: 나머지 금액
				}
				else
				{// 해피코인으로만으로 결제가 가능하면
					i64UseHappyCoin = iBuyCash;
				}

				pkBuyWnd->GetControl(L"FRM_TOTAL_TOL")->Text(BM::vstring((i64MyCash - i64UseTol)));
				pkBuyWnd->GetControl(L"FRM_TOTAL_HAPPYCOIN")->Text(BM::vstring((i64MyMileage - i64UseHappyCoin)));
			}
			else
			{
				pkBuyWnd->GetControl(L"FRM_COST")->Text(BM::vstring(iBuyCash));
				pkBuyWnd->GetControl(L"FRM_CASH")->Text(BM::vstring(i64MyCash));
				pkBuyWnd->GetControl(L"FRM_TOTAL")->Text(BM::vstring((i64MyCash - iBuyCash)));
			}

			CashMiniIcon(pkBuyWnd,kArticle);
			DrawCashItem(pkBuyWnd,kArticle);
			CashNoEquip(pkBuyWnd,iIndexFrom,kArticle);
		}break;
		case CIBT_BUY_REAL:
		case CIBT_STATIC_ITEM_BUY_REAL:
		{// 아이템 구매 확정
			if(0>=iIndex)
			{
				return;
			}

			TBL_DEF_CASH_SHOP_ITEM_PRICE kPriceToBuy;
			pkWnd->GetCustomData(&kPriceToBuy, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			int const iBuyCash = (kPriceToBuy.IsDiscount ? kPriceToBuy.iDiscountCash : kPriceToBuy.iCash);
			
			if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
			{
				// 한국은 톨+해피코인 합산 가격으로 체크한다.
				// 해피코인이 차감 되고, 모자르는 금액은 톨이 차감 됨. 단, 선물의 경우 톨만 사용할 수 있음.
				// 또한 한국은 마일리지로 아이템을 판매하지 않기 때문에 캐시에 대해서만 체크한다.
				const __int64 i64MyUnitCoin = i64MyCash + i64MyMileage;
				if( i64MyUnitCoin < iBuyCash )
				{
					lwAddWarnDataTT(1900);
					return;
				}
			}
			else
			{
				if(IsBuyingCashItem())
				{// 캐시로 아이템 구입하는데
					if (i64MyCash < iBuyCash)
					{// 돈이 부족하다면
						lwAddWarnDataTT(1900);
						return;
					}
				}
				else
				{// 마일리지로 아이템 구입하는데
					if(i64MyMileage < iBuyCash)
					{// 돈이 부족하다면
						lwAddWarnDataTT(790216);
						return;
					}
				}
			}

			XUI::CXUI_Wnd* const pkBuyWnd = XUIMgr.Get((CIBT_STATIC_ITEM_BUY_REAL==iBuyType)?(L"SFRM_CASH_OUT_BUY"):(L"SFRM_CASHSHOP_BUY"));
			if(!pkBuyWnd)
			{
				return;
			}
			__int64 i64UseMileage=0;
			XUI::CXUI_CheckButton* const pkMileageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBuyWnd->GetControl(L"CBTN_USER_DP"));
			if(pkMileageBtn)
			{
				pkMileageBtn->GetCustomData(&i64UseMileage, sizeof(i64UseMileage));	
			}
			BM::Stream kPacket;
			SPT_C_M_CS_REQ_BUY_ARTICLE kSendArticle(g_kCashShopMgr.ValueKey());
			REQ_BUY_ARTICLE kArticleOne;
			kArticleOne.iArticleIdx = iIndex;
			kArticleOne.bTimeType = kPriceToBuy.bTimeType;
			kArticleOne.iUseTime = kPriceToBuy.iItemUseTime;
			kSendArticle.kContBuyArticle.push_back(kArticleOne);
			if(IsBuyingCashItem())
			{				
				kSendArticle.i64Bonus = i64UseMileage;
			}
			else
			{
				kSendArticle.i64Bonus = 0;	
			}			
			kSendArticle.iBuyType = static_cast<BYTE>(iBuyType);
			kSendArticle.WriteToPacket(kPacket, (CIBT_STATIC_ITEM_BUY_REAL==iBuyType)?(PT_C_M_CS_REQ_SIMPLE_BUY_ARTICLE):(0));
			
			if(0 < g_kCashShopMgr.GetRememberItemMileage()
				&& IsBuyingCashItem()
				)
			{// 지급할 마일리지가 존재하고, 캐시로 구매 할 때
				std::wstring kStr;
				{// 적립되는 마일리지를 유저에게 메세지 박스로 알리고
					std::wstring const kFormStr(TTW(790212));
					wchar_t szBuf[200] ={0,};
					wsprintfW(szBuf, kFormStr.c_str(), g_kCashShopMgr.GetRememberItemMileage());
					kStr = szBuf;			
				}
				// 구매 패킷은 메세지 박스 확인창을 누를때 보낸다.
				if(CIBT_STATIC_ITEM_BUY_REAL==iBuyType)
				{
					lwCallCommonMsgYesNoBox(MB(kStr), lwPacket(&kPacket), true, MBT_STATIC_CASHITEM_BUY_TO_PACKET);
				}
				else
				{
					lwCallCommonMsgYesNoBox(MB(kStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
				}
			}
			else
			{// 지급할 마일리지가 없다면, 즉시 구매 패킷을 보낸다.
				if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
				{//한국 일 경우 마일리지와 관계없이 청약 관련 경고 문구 표시 필요
					lwCallCommonMsgYesNoBox(MB(TTW(790209)), lwPacket(&kPacket), true, 
						CIBT_BUY_REAL==iBuyType ? MBT_COMMON_YESNO_TO_PACKET:
						MBT_STATIC_CASHITEM_BUY_TO_PACKET);
				}//캐쉬샾 나간다는 메세지도 나중에 보내야 함
				else
				{
					NETWORK_SEND(kPacket);
					if(CIBT_STATIC_ITEM_BUY_REAL==iBuyType)
					{
						lwOnCloseStaticCashItemBuy();
					}
				}
			}
			pkParent->Close();
		}break;
	case CIBT_RENEW_REAL:
	case CIBT_STATIC_ITEM_RENEW_REAL:
		{// 기간 연장 구매 선택
			if(0>=iIndex)
			{
				return;
			}
			TBL_DEF_CASH_SHOP_ITEM_PRICE kPriceToBuy;
			pkWnd->GetCustomData(&kPriceToBuy, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			int const iBuyCash = kPriceToBuy.IsExtendDiscount ? kPriceToBuy.iDiscountExtendCash: kPriceToBuy.iExtendCash;
			if(IsBuyingCashItem())
			{// 캐시로 아이템을 구매하는데
				if(i64MyCash < iBuyCash)
				{// 보유 캐시가 부족하다면
					lwAddWarnDataTT(1848);
					return;
				}
			}
			else
			{// 마일리지로 아이템을 구매하는데
				if(i64MyMileage < iBuyCash)
				{// 보유 마일리지가 부족하다면
					lwAddWarnDataTT(790216);
					return;
				}
				//이후 소지금은 마일리지 값으로 체크 해야 하므로
				i64MyCash = i64MyMileage;
			}
			pkParent->Close();
			
			XUI::CXUI_Wnd* const pkBuyWnd = XUIMgr.Get((CIBT_STATIC_ITEM_RENEW_REAL==iBuyType)?(L"SFRM_CASH_OUT_BUY"):(L"SFRM_CASHSHOP_RENEW"));
			if(!pkBuyWnd)
			{
				return;
			}
			__int64 i64UseMileage=0;
			XUI::CXUI_CheckButton* const pkMileageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBuyWnd->GetControl(L"CBTN_USER_DP"));
			if(pkMileageBtn)
			{
				pkMileageBtn->GetCustomData(&i64UseMileage, sizeof(i64UseMileage));
			}
			BM::vstring	vStr;
			if( 0 == kPriceToBuy.iItemUseTime )
			{
				vStr += TTW(178);
			}
			else
			{
				vStr += kPriceToBuy.iItemUseTime;
				switch(kPriceToBuy.bTimeType)
				{
				case UIT_DAY:	{ vStr += TTW(174);		} break;//일
				case UIT_HOUR:	{ vStr += TTW(90004);	} break;//시간
				case UIT_MIN:	{ vStr += TTW(176);		} break;//분
				}
			}
			vStr += TTW(1846);

			BM::Stream	kPacket;
			kPacket.Push((CIBT_STATIC_ITEM_RENEW_REAL==iBuyType)?(PT_C_M_CS_REQ_SIMPLE_ADD_TIMELIMIT):(PT_C_M_CS_REQ_ADD_TIMELIMIT));
			kPacket.Push(iIndex);
			kPacket.Push(kPriceToBuy.bTimeType);
			kPacket.Push(kPriceToBuy.iItemUseTime);
			kPacket.Push(g_kCashShopMgr.RenewItemPos());
			if(IsBuyingCashItem())
			{
				kPacket.Push(i64UseMileage);
			}
			else
			{
				__int64 i64Zero=0;
				kPacket.Push(i64Zero);
			}
			kPacket.Push(g_kCashShopMgr.ValueKey());
			if(0 < g_kCashShopMgr.GetRememberItemMileage()
				&& IsBuyingCashItem()
				)
			{
				std::wstring kStr;
				{
					std::wstring const kFormStr(TTW(790212));
					wchar_t szBuf[200] ={0,};
					wsprintfW(szBuf, kFormStr.c_str(), g_kCashShopMgr.GetRememberItemMileage());
					kStr = szBuf;
				}
				vStr+=kStr;
			}
			else
			{
				if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
				{//한국 일 경우 마일리지와 관계없이 청약 관련 경고 문구 표시 필요
					vStr += L" ";
					vStr += TTW(790209);
				}
			}

			if(CIBT_STATIC_ITEM_RENEW_REAL==iBuyType)
			{
				lwCallCommonMsgYesNoBox(MB(vStr), lwPacket(&kPacket), true, MBT_STATIC_CASHITEM_BUY_TO_PACKET);
			}
			else
			{
				lwCallCommonMsgYesNoBox(MB(vStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			}
		}break;
	case CIBT_GIFT_REAL:
		{// 선물 구매 확정
			std::wstring kName;
			std::wstring kComment;
			XUI::CXUI_Edit *pkEdtName = dynamic_cast<XUI::CXUI_Edit *>(pkParent->GetControl(L"EDT_NAME"));
			if(pkEdtName)
			{
				kName = pkEdtName->EditText();
			}
			XUI::CXUI_Edit *pkEdtComm = dynamic_cast<XUI::CXUI_Edit *>(pkParent->GetControl(L"EDT_COMMENT"));
			if(pkEdtComm)
			{
				kComment = pkEdtComm->EditText();
				if(g_kClientFS.Filter(kComment, false)
					|| !g_kUnicodeFilter.IsCorrect(UFFC_CASHSHOP_PRESENT_COMMENT, kComment)
					)
				{//욕설 등이 있으면 안됨
					lwAddWarnDataTT(700144);
					return;
				}
			}

			if(kName.empty())
			{// 대상의 이름이 없거나
				lwAddWarnDataTT(1901);
				return;
			}
			else if(pPilot->GetName() == kName)
			{// 자신에게 보내는 경우는 금지한다
				lwAddWarnDataTT(1902);
				return;
			}

			TBL_DEF_CASH_SHOP_ITEM_PRICE kPriceToBuy;
			pkWnd->GetCustomData(&kPriceToBuy, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
			
			int const iBuyCash = (kPriceToBuy.IsDiscount ? kPriceToBuy.iDiscountCash : kPriceToBuy.iCash);
			if(IsBuyingCashItem())
			{// 캐시로 구매라면
				if (i64MyCash < iBuyCash)
				{// 보유 캐시가 충분한지 판별하고
					lwAddWarnDataTT(1900);
					return;
				}
			}
			else
			{// 마일리지로 구매라면
				if(i64MyMileage < iBuyCash)
				{// 보유 마일리지가 충분한지 판별한다
					lwAddWarnDataTT(790216);
					return;
				}
			}

			BM::Stream kPacket;
			SPT_C_M_CS_REQ_SEND_GIFT kStruct(g_kCashShopMgr.ValueKey());
			kStruct.iArticleIdx = iIndex;
			kStruct.bTimeType = kPriceToBuy.bTimeType;
			kStruct.iUseTime = kPriceToBuy.iItemUseTime;
			kStruct.kRecverName = kName;
			kStruct.kComment = kComment;
			kStruct.i64BonusUse = 0;

			kStruct.WriteToPacket(kPacket);
			std::wstring kStr;
			{
				std::wstring const kFormStr(TTW(790212));
				wchar_t szBuf[200] ={0,};
				wsprintfW(szBuf, kFormStr.c_str(), g_kCashShopMgr.GetRememberItemMileage());
				kStr = szBuf;
			}
			if(0 < g_kCashShopMgr.GetRememberItemMileage()
				&& IsBuyingCashItem())
			{// 지급할 마일리지가 존재 한다면, 메세지 박스로 알려주고, 확인을 누르면 구매 패킷을 보낸다
				lwCallCommonMsgYesNoBox(MB(kStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			}
			else
			{// 지급할 마일리지가 없거나, 마일리지로 구입이라면 구매 패킷을 바로 보낸다
				if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
				{//한국 일 경우 마일리지와 관계없이 청약 관련 경고 문구 표시 필요
					lwCallCommonMsgYesNoBox(MB(TTW(790209)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
				}
				else
				{
					NETWORK_SEND(kPacket)
				}
			}
			pkParent->Close();
		}break;
	case CIBT_BASKET_PART:
	case CIBT_BASKET:
	case CIBT_PV_ALL:
		{// 모두 구매 선택

			XUI::CXUI_Wnd* pAllBuy = XUIMgr.Call(L"SFRM_CASHSHOP_ALL_BUY", true);
			if( !pAllBuy ){ return; }
			try 
			{
				XUI::CXUI_List* pItemList = dynamic_cast<XUI::CXUI_List*>(pAllBuy->GetControl(L"LIST_PREVIEW_ITEM"));
				if( !pItemList ){ throw 0; }
				lwUIWnd(pAllBuy).SetCustomData<int>(iBuyType + 1);

				EQUIP_PREVIEW_CONT	kItems;
				CONT_SHOPPING_BASKET_SLOT_IDX kIDX;
				switch( iBuyType )
				{
				case CIBT_PV_ALL:		{ g_kCashShopMgr.GetPreviewItems(kItems);	} break;
				case CIBT_BASKET:		{ g_kCashShopMgr.GetShoppingBasket(kIDX, kItems);	} break;
				case CIBT_BASKET_PART:	{ CheckSelectedSlotItem(kIDX, kItems);		} break;
				default: throw 0;
				}
				if( !kItems.size() ){ throw 0; }

				// 미리 보기에 올라가있는 아이템의 가격을 계산하고(모두구매)
				// 지급할 마일리지를 내부에서 계산하고
				__int64 i64TotalBuyCash = 0;
				switch( iBuyType )
				{
				case CIBT_PV_ALL:		{ i64TotalBuyCash = g_kCashShopMgr.GetPreviewItemTotalCost();	} break;
				case CIBT_BASKET:		{ i64TotalBuyCash = g_kCashShopMgr.GetBasketItemTotalCost();	} break;
				case CIBT_BASKET_PART:	{ i64TotalBuyCash = GetSelectedSlotItemTotalCost();			} break;
				default: throw 0;
				}

				// 총 아이템 가격을 기억한다
				g_kCashShopMgr.SetRememberTotalPrice(i64TotalBuyCash);

				if(IsBuyingCashItem())
				{// 캐시로 구매라면
					if (i64MyCash < i64TotalBuyCash)
					{// 보유 캐시가 충분한지 판별하고
						throw 1900;
					}
					// 부분 마일리지 사용 UI를 킨다.
					OffAllMileageWnd(false, pAllBuy);
				}
				else
				{// 마일리지로 구매라면
					if(i64MyMileage < i64TotalBuyCash)
					{// 보유 마일리지가 충분한지 판별하고
						throw 790216;
					}
					// 부분 마일리지 사용 UI를 끈다
					OffAllMileageWnd(true, pAllBuy);
					//이후 소지금은 마일리지 값으로 체크 해야 하므로
					i64MyCash = i64MyMileage;
				}

				pAllBuy->GetControl(L"FRM_CASH")->Text(BM::vstring(i64MyCash));
				pAllBuy->GetControl(L"FRM_COST")->Text(BM::vstring(i64TotalBuyCash));
				pAllBuy->GetControl(L"FRM_TOTAL")->Text(BM::vstring((i64MyCash - i64TotalBuyCash)));
				EQUIP_PREVIEW_CONT::const_iterator c_iter = kItems.begin();
				CONT_SHOPPING_BASKET_SLOT_IDX::const_iterator idx_itor = kIDX.begin();
				while( c_iter != kItems.end() )
				{
					if( c_iter->iIdx )
					{
						XUI::SListItem* pkItem = pItemList->AddItem(L"");
						if( !pkItem->m_pWnd ){ throw 0; }

						XUI::CXUI_Wnd*	pItem = pkItem->m_pWnd;

						TBL_DEF_CASH_SHOP_ARTICLE kArticle;
						if(!g_kCashShopMgr.GetItem(c_iter->iIdx, kArticle))
						{
							throw 0;
						}

						TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
						if( kArticle.kContCashItemPrice.empty() )
						{
							throw 0;
						}
						kPrice = kArticle.kContCashItemPrice.begin()->second;
						if( 0>=kPrice.iItemUseTime && 0>=kPrice.bTimeType )
						{
							throw 0;
						}
						int const iBuyCash = (kPrice.IsDiscount)?(kPrice.iDiscountCash):(kPrice.iCash);
						SetDayBtnInitialize(pItem, kArticle, iBuyCash);

						if( CIBT_PV_ALL == iBuyType )
						{
							pItem->SetCustomData(&c_iter->iIdx, sizeof(int));
						}
						else
						{
							if( idx_itor != kIDX.end() )
							{
								BM::Stream kPacket;
								kPacket.Push( (*idx_itor) );
								kPacket.Push( c_iter->iIdx );
								lwUIWnd(pItem).SetCustomDataAsPacket(lwPacket(&kPacket));
								++idx_itor;
							}						
						}

						XUI::CXUI_Wnd* pkName = pItem->GetControl(L"FRM_NAME");
						if( pkName )
						{
							lwUIWnd(pkName).SetCustomData<int>(iBuyType);
							Quest::SetCutedTextLimitLength(pkName, kArticle.wstrName, _T("..."));
						}

						BM::vstring kstrPrice;
						int iItemCount = 0;
						CONT_CASH_SHOP_ITEM::const_iterator item_iter = kArticle.kContCashItem.begin();
						if( item_iter != kArticle.kContCashItem.end() )
						{
							iItemCount = item_iter->iItemCount;
						}
						MakeUseTimeStr(kArticle.bSaleType, kPrice, kstrPrice, iItemCount);
						CashMiniIcon(pItem,kArticle);
						CashNoEquip(pItem,iIndexFrom,kArticle);
						DrawCashItem(pItem,kArticle);
					}
					++c_iter;
				}
			}
			catch(int iTTNo)
			{
				if(pAllBuy)
				{
					pAllBuy->Close();
				}
				if(iTTNo > 0)
				{
					lwAddWarnDataTT(iTTNo);
				}
				return;
			}
		}break;
	case CIBT_BASKET_PART_REAL:
	case CIBT_BASKET_REAL:
	case CIBT_PV_ALL_REAL:
		{// 모두 구매 확정
			EQUIP_PREVIEW_CONT	kItems;
			CONT_SHOPPING_BASKET_SLOT_IDX kIDX;
			switch( iBuyType )
			{
			case CIBT_PV_ALL_REAL:		{ g_kCashShopMgr.GetPreviewItems(kItems);	} break;
			case CIBT_BASKET_REAL:		{ g_kCashShopMgr.GetShoppingBasket(kIDX, kItems);	} break;
			case CIBT_BASKET_PART_REAL:	{ CheckSelectedSlotItem(kIDX, kItems);		} break;
			default: return;
			}
			if( !kItems.size() )
			{//아이템이 없다. 버그
				assert(0);
			}

			// 미리 보기에 올라가있는 아이템의 가격을 계산하고(모두구매)
			// 지급할 마일리지를 내부에서 계산한다.
			__int64 const i64TotalBuyCash = g_kCashShopMgr.GetPreviewItemTotalCost();

			if(IsBuyingCashItem())
			{// 캐시로 구매라면
				if (i64MyCash < i64TotalBuyCash)
				{// 보유 캐시가 충분한지 판별하고
					lwAddWarnDataTT(1900);
					return;
				}
			}
			else
			{// 마일리지로 구매라면
				if(i64MyMileage < i64TotalBuyCash)
				{// 보유 마일리지가 충분한지 판별하고
					lwAddWarnDataTT(790216);
					return;
				}
			}

			XUI::CXUI_Wnd* const pkBuyWnd = XUIMgr.Get(_T("SFRM_CASHSHOP_ALL_BUY"));
			if(!pkBuyWnd)
			{
				return;
			}
			__int64 i64UseMileage=0;
			XUI::CXUI_CheckButton* const pkMileageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBuyWnd->GetControl(L"CBTN_USER_DP"));
			if(pkMileageBtn)
			{
				pkMileageBtn->GetCustomData(&i64UseMileage, sizeof(i64UseMileage));
			}
			SPT_C_M_CS_REQ_BUY_ARTICLE kSendArticle(g_kCashShopMgr.ValueKey());
			EQUIP_PREVIEW_CONT::const_iterator	c_iter = kItems.begin();
			while( c_iter != kItems.end() )
			{
				EQUIP_PREVIEW_CONT::value_type const& kArticleItem = (*c_iter);

				REQ_BUY_ARTICLE	kArticle;
				kArticle.iArticleIdx = kArticleItem.iIdx;

				CONT_CASH_SHOP_ITEM_PRICE::const_iterator price_iter = kArticleItem.kContCashItemPrice.begin();
				if( price_iter != kArticleItem.kContCashItemPrice.end() )
				{
					CONT_CASH_SHOP_ITEM_PRICE::mapped_type const& kPriceInfo = price_iter->second;

					kArticle.bTimeType = kPriceInfo.bTimeType;
					kArticle.iUseTime = kPriceInfo.iItemUseTime;
				}
				kSendArticle.kContBuyArticle.push_back(kArticle);
				++c_iter;
			}
			if(IsBuyingCashItem())
			{
				kSendArticle.i64Bonus = i64UseMileage;
			}
			else
			{
				kSendArticle.i64Bonus = 0;
			}
			kSendArticle.iBuyType = static_cast<BYTE>(iBuyType);
			BM::Stream kPacket;
			kSendArticle.WriteToPacket(kPacket);
			
			std::wstring kStr;
			{
				std::wstring const kFormStr(TTW(790212));
				wchar_t szBuf[200] ={0,};
				wsprintfW(szBuf, kFormStr.c_str(), g_kCashShopMgr.GetRememberItemMileage());
				kStr = szBuf;
			}
			if(0 < g_kCashShopMgr.GetRememberItemMileage()
				&& IsBuyingCashItem()
				)
			{// 지급할 마일리지가 존재 한다면, 메세지 박스로 알려주고, 확인을 누르면 구매 패킷을 보낸다
				lwCallCommonMsgYesNoBox(MB(kStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			}
			else
			{// 지급할 마일리지가 없거나, 마일리지로 구입이라면 구매 패킷을 바로 보낸다
				NETWORK_SEND(kPacket)
			}
			pkParent->Close();
		}break;
	}	
}
void lwCashShop::CashNoEquip(XUI::CXUI_Wnd* pWnd,int const iIndexFrom,TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle,bool MainUINoEquip)
{
	/////////////////////////////////////////////착용불가.이미지 작업
	XUI::CXUI_Wnd* pkEquitTestWnd = pWnd->GetControl(L"SFRM_EQUIP_TEST");
	if(NULL == pkEquitTestWnd)
	{
		return;
	}
	pkEquitTestWnd->Visible(false);
	XUI::CXUI_Wnd* pkParts = pWnd->GetControl(L"IMG_PARTS");
	if(!pkParts)
	{
		return;
	}
	int iIndex = 0;
	if(0==iIndexFrom)
	{
		pWnd->GetCustomData(&iIndex, sizeof(int));	//물품번호
	}
	else
	{
		iIndex = iIndexFrom;
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);         
	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}
	CONT_DEFITEM const* pkDefItem = NULL;
	g_kTblDataMgr.GetContDef(pkDefItem);
	if( !pkDefItem )
	{
		return;
	}

	XUI::CXUI_Wnd* pkImg = pWnd->GetControl(L"IMG_ICON");
	if(!pkImg)
	{
		return;
	}
	pkImg->GrayScale(false);
	bool bCanEquip = false;
	bool bCanHuman = false;
	bool bCashDra = false;
	int const iGender = pPlayer->GetAbil(AT_GENDER);
	int PlayerClass = pPlayer->GetAbil(AT_CLASS);
	CONT_CASH_SHOP_ITEM::const_iterator item_it = rkArticle.kContCashItem.begin();
	while( item_it != rkArticle.kContCashItem.end())
	{
		CItemDef const *pDef = kItemDefMgr.GetDef((*item_it).iItemNo);
		if( !pDef )
		{
			++item_it;
			continue;	
		}

		int ItemNo = (*item_it).iItemNo;
		CONT_DEFITEM::const_iterator def_item = pkDefItem->find(ItemNo);
		if( pkDefItem->end() == def_item )
		{
			return;
		}

		//내가 착용 불가능한게 하나라도 있으면 리턴시키자
		int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
		if (3 < iGenderLimit)
		{
			break;
		}
		if( (iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
		{
			bCanEquip = true;
			break;
		}

		if(!IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pPlayer->GetAbil(AT_CLASS)))
		{//다른 클래스 아이템이므로 에러 출력
			bCanEquip = true;
			break;
		}

		bCanHuman = (def_item->second.i64ClassLimit > 0);
		bCashDra = (def_item->second.i64DraClassLimit > 0);

		if(bCanHuman && bCashDra)
		{//두개다 값이 있다는 것은 종족에 상관없는 아이템이다.
			bCanEquip = false;
		}
		else
		{
			if(PlayerClass >= EPCLASS_TYPE::UCLASS_SHAMAN)
			{//내가 용족일때 인간족 장착아이템이면
				bCanEquip = bCanHuman;
			}
			else
			{//내가 인간족일 때 용족 장착아이템이면
				bCanEquip = bCashDra;
			}
		}
		break;
	}
	if(true == MainUINoEquip)
	{
		pkEquitTestWnd->Visible(false);
	}
	else
	{
		pkEquitTestWnd->Visible(bCanEquip);
	}
	
	pkImg->GrayScale(bCanEquip);
	pkParts->GrayScale(bCanEquip);
}

void lwCashShop::RecvCashShop_Command(WORD const wPacketType, BM::Stream &rkPacket)
{
	int iError = 0;
	switch(wPacketType)
	{
	case PT_M_C_CS_ANS_SIMPLE_ENTER_CASHSHOP:
		{
			ECashShopResult	kResult;
			rkPacket.Pop(kResult);
			if( ReturnResult(kResult) )
			{
				if( !g_kCashShopMgr.StaticItemNo() )
				{
					CallStaticCashItemBuyUI(g_kCashShopMgr.GetItemTypeToArticleNo(g_kCashShopMgr.StaticItemType(), g_kCashShopMgr.StaticItemSubKey()));
				}
				else
				{
					int iArticleIDX = 0;
					if( g_kCashShopMgr.FindArticle(g_kCashShopMgr.StaticItemNo(), iArticleIDX) )
					{
						CallStaticCashItemBuyUI(iArticleIDX, true);
					}
				}
			}
			g_kCashShopMgr.StaticItemType(-1);
			g_kCashShopMgr.StaticItemSubKey(-1);
			g_kCashShopMgr.StaticItemNo(0);
		}break;
	case PT_M_C_CS_ANS_BUY_ARTICLE:
	case PT_M_C_CS_ANS_SEL_ARTICLE:
	case PT_M_C_CS_ANS_SEND_GIFT:
		{
			int iReqLevel =0;
			rkPacket.Pop(iError);
			rkPacket.Pop(iReqLevel);
			if(ReturnResult(iError, iReqLevel))
			{
				lwAddWarnDataTT(wPacketType-PT_CASH_SHOP_BASE+1930);
				lwClearPreviewEquipIcon();
			}
			if( PT_M_C_CS_ANS_BUY_ARTICLE == wPacketType )
			{
				BYTE BuyType = 0;
				rkPacket.Pop(BuyType);
				switch( BuyType )
				{
				case CIBT_BASKET_REAL:		{ lwDeleteSelectBasketItem(true);	}break;
				case CIBT_BASKET_PART_REAL:	{ lwDeleteSelectBasketItem(false);	}break;
				}
			}
		}break;
	case PT_M_C_CS_ANS_ADD_TIMELIMIT:
		{
			rkPacket.Pop(iError);
			if(ReturnResult(iError))
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1847, true);
			}
		}break;
	case PT_N_C_ANS_CASHSHOP_LIMIT_ITEM_INFO:
		{
			CONT_CASH_SHOP_ITEM_LIMITSELL kContShopLimitSell;
			PU::TLoadTable_MM(rkPacket, kContShopLimitSell);		
			
			g_kCashShopMgr.SetLimitedArticle(kContShopLimitSell);
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_CASH_SHOP"));
			if(pkWnd)
			{
				UpdateLimitedPage(pkWnd->GetControl(_T("FRM_LIMITED")));
			}

			g_kTblDataMgr.SetContDef(kContShopLimitSell); //현재 이 함수를 거치면 값이 바뀌는 경우가 있으므로 맨마지막에 실행하도록 한다.
		}break;
	case PT_M_C_CS_ANS_ENTER_CASHSHOP:
		{
			SPT_M_C_CS_ANS_ENTER_CASHSHOP kStruct;
			kStruct.ReadFromPacket(rkPacket);

			if(0==kStruct.iError)
			{
				g_kCashShopMgr.SetGiftCont(kStruct.kContGift);
				g_kCashShopMgr.SetRankCont(kStruct.kContRank);
			}

			g_kCashShopMgr.UpdateValidShoppingBasketList();

			XUI::CXUI_Wnd* pGiftNfy = XUIMgr.Get(_T("FRM_GIFT_RECV"));
			if( pGiftNfy )
			{
				pGiftNfy->Close();
			}

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_CASH_SHOP"));
			if( !pkWnd || pkWnd->IsClosed() )
			{
				lwCallUI("FRM_CASH_SHOP_BG");
				g_kChatMgrClient.ChatStation(ECS_CASHSHOP);
				lwCallChatWindow(1);

				//케릭터 이동 금지
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_CashShop, true));
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_CashShop, false));

				//패킷을 받아야만 열리도록
				pkWnd = XUIMgr.Call(L"FRM_CASH_SHOP");
				if(pkWnd)
				{
					SelectedMoveBtnClear(pkWnd->GetControl(_T("SFRM_INFO")));
					SelectCashTypeClear(pkWnd->GetControl(_T("SFRM_INFO")));
					ViewChangeModel(true);
					TBL_DEF_CASH_SHOP_CATEGORY kCate;//임시
					XUI::CXUI_Wnd* pDefault = NULL;
					for(int i = PgCashShop::ECSCATEGORYNUM_BEGIN; i < PgCashShop::ECSCATEGORYNUM_END; ++i)
					{
						BM::vstring kTabName(L"CBTN_TAB");
						kTabName+=i;
						XUI::CXUI_Wnd* pkTab = pkWnd->GetControl(kTabName);
						if (pkTab)
						{
							const bool IsVisible = g_kCashShopMgr.GetCategory(i, kCate);
							pkTab->Visible(IsVisible);
							if(IsVisible)
							{
								pkTab->Text(kCate.wstrName);//탭 이름
							}

							XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTab);
							if( pBtn )
							{
								pBtn->Disable(!kCate.bIsUsed);
								if( pBtn->Enable() && i == 0 )
								{
									pBtn->Check(true);
									pDefault = pkTab;
								}
							}

							if( NULL == pDefault )
							{
								pDefault = pkTab;
							}
						}
					}
					SetLankingData(pkWnd, kStruct.kContRank);
					SetBestItemInfo(pkWnd);
					lua_tinker::call<void, lwUIWnd>("UI_Click_CashShop_Tab", lwUIWnd(pDefault));
				}
			}
		}break;
	case PT_M_C_CS_ANS_RECV_GIFT:
		{
			SPT_M_C_CS_ANS_RECV_GIFT kStruct;
			kStruct.ReadFromPacket(rkPacket);

			if(ReturnResult(kStruct.iError))
			{
				g_kCashShopMgr.DeleteGift(kStruct.kGiftGuId);
				CONT_CASHGIFTINFO	kCate;
				g_kCashShopMgr.GetGiftContRecv(kCate);
				SetGiftPage(XUIMgr.Get(L"FRM_CASH_SHOP"), kCate);
				InitPageUI(XUIMgr.Get(L"FRM_CASH_SHOP")->GetControl(L"FRM_PAGE"), g_kCashShopMgr.GetGiftRecvCount(), PgCashShop::ECSFORM_GIFT);
			}

		}break;
	case PT_M_C_CS_ANS_MODIFY_VISABLE_RANK:
		{
			SPT_M_C_CS_ANS_MODIFY_VISABLE_RANK Data;
			Data.ReadFromPacket(rkPacket);
			if( ReturnResult(Data.iError) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1906, true);
			}
		}break;
	case PT_M_C_CS_ANS_EXIT_CASHSHOP_UNLOCK:
		{
			lwExitCashShop();
		}break;
	}
}

void lwCashShop::SetCashNum(XUI::CXUI_Wnd* pkWnd)
{
	if(pkWnd)
	{
		PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
		if(pPilot != NULL)
		{
			pkWnd->Text(BM::vstring(pPilot->GetAbil64(AT_CASH)));
			pkWnd->Invalidate(true);
		}
	}
}

void lwCashShop::lwRecvGift(lwUIWnd kWnd)
{
	if(kWnd.IsNil()) {return;}
	XUI::CXUI_Wnd* pkWnd = kWnd.GetParent()();
	if(!pkWnd) {return;}

	BM::GUID kGuid;
	pkWnd->GetCustomData(&kGuid, sizeof(kGuid));
	if(BM::GUID::IsNotNull(kGuid))
	{
		SPT_C_M_CS_REQ_RECV_GIFT kStruct;
		kStruct.kGiftGuId = kGuid;
		BM::Stream kPacket;
		kStruct.WriteToPacket(kPacket);

		NETWORK_SEND(kPacket)
	}
}

void lwCashShop::lwDoActionCashItem(lwUIWnd kWnd)
{
	if(kWnd.IsNil()) {return;}
	int iIndex = kWnd.GetCustomData<int>();
	bool bIsNotEquipItem = false;
	if(0<iIndex)
	{
		TBL_DEF_CASH_SHOP_ARTICLE kDefArt;
		if(g_kCashShopMgr.FindPageArticle(iIndex, kDefArt) || g_kCashShopMgr.GetItem(iIndex, kDefArt))
		{//아이템 가격은 한개만 남기고 지우자
			if( kDefArt.kContCashItem.size() >= 1 )
			{
				CONT_CASH_SHOP_ITEM::iterator item_iter = kDefArt.kContCashItem.begin();
				if( item_iter == kDefArt.kContCashItem.end() )
				{
					return;
				}

				CONT_CASH_SHOP_ITEM::value_type const& kItemInfo = (*item_iter);

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItemInfo.iItemNo);
				if( !pDef )
				{
					return;
				}

				if( pDef->IsType(ITEM_TYPE_MYHOME_IN)
				||  pDef->IsType(ITEM_TYPE_MYHOME_OUT) )
				{
					bIsNotEquipItem = true;
				}
			}

			if( !PgCashShop::ArticleCostDeleteToOneSave(kDefArt.kContCashItemPrice) )
			{
				return;
			}

			ViewChangeModel(!bIsNotEquipItem);

			if( bIsNotEquipItem )
			{//여기서 오브젝트 프리뷰 아이템을 등록하는 부분을 생성한다.
				lwClearPreviewFurnitureModel();
				if( g_kCashShopMgr.SetPreviewFurniture(kDefArt) )
				{
					SetPreviewFurnitureItem();
				}
			}
			else
			{
				RECV_UI_EQUIP_PREVIEW_CONT	UIItemCont;
				if(g_kCashShopMgr.SetPreviewEquip(kDefArt, UIItemCont))
				{
					SetPreviewEquipIcon(UIItemCont);
				}
			}
		}
	}
}

int ChangeEqLocToIconLoc(int const EqLoc)
{
	switch( EqLoc )
	{
	case EQUIP_POS_HAIR_COLOR:	return 3;
	case EQUIP_POS_HAIR:	return 2;
	case EQUIP_POS_FACE:	return 1;
	case EQUIP_POS_MEDAL:	return 13;
	case EQUIP_POS_GLASS:	return 0;
	case EQUIP_POS_HELMET:	return 4;
	case EQUIP_POS_SHIRTS:	return 6;
	case EQUIP_POS_PANTS:	return 7;
	case EQUIP_POS_SHOULDER:return 5;
	case EQUIP_POS_GLOVE:	return 8;
	case EQUIP_POS_BOOTS:	return 9;
	case EQUIP_POS_WEAPON:	return 10;
	case EQUIP_POS_SHEILD:
	case EQUIP_POS_KICKBALL:return 11;
	case EQUIP_POS_CLOAK:	return 12;
	}
	return -1;
}

void lwCashShop::lwDrawPreviewEquipIcon(lwUIWnd UISelf)
{
	XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(UISelf.GetSelf());
	if( !pIcon )
	{ 
		return; 
	}

	SPreviewItemInfo kInfo;
	pIcon->GetCustomData(&kInfo, sizeof(SPreviewItemInfo));

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const CItemDef* pItemDef = kItemDefMgr.GetDef(kInfo.dwItemNo);

	PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(kInfo.dwItemNo);
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

void lwCashShop::lwCallPreviewEquipItemToolTip(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pkSelf = UISelf.GetSelf();
	if( !pkSelf )
	{
		return; 
	}

	if( pkSelf->GetCustomDataSize() )
	{
		SPreviewItemInfo kInfo;
		pkSelf->GetCustomData(&kInfo, sizeof(SPreviewItemInfo));

		if( kInfo.dwItemNo )
		{
			CallToolTip_ItemNo(kInfo.dwItemNo, UISelf.GetLocation());
		}
	}
}

void lwCashShop::SetPreviewFurnitureItem()
{
	PgUIModel* pkModel = g_kUIScene.FindUIModel("OBJECTVIEW");
	if( pkModel )
	{
		PgFurniture* pkFurniture = g_kCashShopMgr.GetPreviewFurniture();
		if( pkFurniture )
		{
			NiNode* pNode = NiDynamicCast(NiNode, pkFurniture->GetNIFRoot());
			if( pNode )
			{
				pkModel->AddNIF("PreviewObject", pNode, false, false);
				SCameraResetInfo kInfo;
				kInfo.eHeightSet = SCameraResetInfo::EHS_USE_RADIAN_VALUE;

				switch( pkFurniture->GetFurnitureType() )
				{
				case FURNITURE_TYPE_WALL_ATTACH:
					{
					}break;
				case FURNITURE_TYPE_FLOOR:
					{
						kInfo.fYCenter = 0.6f;
					}break;
				default:
					{
						kInfo.fYCenter = 0.8f;
					}break;
				}
				pkModel->CameraReset(pkFurniture->GetNIFRoot(), kInfo);
				pkModel->SetCameraZoomMinMax(0.5f, 1.5f);
				pkModel->SetOrthoZoom(1.0f);
				pkModel->CameraZoomSubstitution(-20.0f);
				pkModel->SetWorldLimitRotate(45.0f, 45.0f, 45.0f);
			}
		}
	}
}

void lwCashShop::lwClearPreviewFurnitureModel()
{
	PgUIModel* pkModel = g_kUIScene.FindUIModel("OBJECTVIEW");
	if( pkModel )
	{
		pkModel->DeleteNIF("PreviewObject");
		g_kCashShopMgr.ClearPreviewFurniture();
	}
}

void lwCashShop::lwOnClickUnRegPreviewItem(lwUIWnd kSelf)
{
	XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(kSelf.GetSelf());
	if( !pIcon )
	{ 
		return; 
	}

	SPreviewItemInfo kInfo;
	pIcon->GetCustomData(&kInfo, sizeof(SPreviewItemInfo));
	switch(g_kCashShopMgr.GetShopType())
	{
	case PgCashShop::ECASHSHOP_GACHA:
		{// 캐시샵 가차 일때
			g_kCSGacha.DelPreViewItem(static_cast<int>(kInfo.dwItemNo));
			g_kCSGacha.UpdatePreViewItem();
		}break;
	case PgCashShop::ECOSTUME_MIX:
		{// 코스튭 조합 일때
			PgCashCostumeMix_ExpectResultItemUI& kPreviewMgr = g_kCostumeMix.GetPreviewResultItemUIMgr();
			kPreviewMgr.DelPreViewItem(static_cast<int>(kInfo.dwItemNo));
			kPreviewMgr.UpdatePreViewItem();
		}break;
	default:
		{
			RECV_UI_EQUIP_PREVIEW_CONT kEquipCont;
			if( g_kCashShopMgr.ReSetPreviewEquip(kInfo.iArticleNo, kEquipCont) )
			{
				SetPreviewEquipIcon(kEquipCont);
			}
		}break;
	}
}

void lwCashShop::lwConfirmBasketRegItem(lwUIWnd kSelf)
{
	if( g_kCashShopMgr.ShoppingBasketSize() >= MAX_BASKET_SLOT)
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1995, true);
		return;
	}

	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	int const iArticleNo = kParent.GetCustomData<int>();

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if( g_kCashShopMgr.GetItem(iArticleNo, kArticle) )
	{
		BM::vstring kString(TTW(790257));
		kString.Replace(L"#NAME#", kArticle.wstrName);
		BM::Stream kPacket;
		kPacket.Push((bool)true);
		kPacket.Push(iArticleNo);
		CallCommonMsgYesNoBox(kString, 96, 97, lwPacket(&kPacket), true, MBT_CONFIRM_CASHSHOP_BASKET_REG, NULL);
	}
}

void lwCashShop::lwConfirmBasketDeRegItem(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	int const iArticleNo = kSelf.GetCustomData<int>();

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if( g_kCashShopMgr.GetItem(iArticleNo, kArticle) )
	{
		BM::vstring kString(TTW(790258));
		kString.Replace(L"#NAME#", kArticle.wstrName);
		BM::Stream kPacket;
		kPacket.Push((bool)false);
		kPacket.Push(iArticleNo);
		kPacket.Push(kSelf.GetBuildIndex());
		CallCommonMsgYesNoBox(kString, 96, 97, lwPacket(&kPacket), true, MBT_CONFIRM_CASHSHOP_BASKET_REG, NULL);
	}
}

void lwCashShop::lwDeleteSelectBasketItem(bool const bIsAllDelete)
{
	if( bIsAllDelete )
	{
		g_kCashShopMgr.ClearShoppingBasket();
	}
	else
	{
		XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_CASH_SHOP");
		if( !pkMainUI ){ return; }

		XUI::CXUI_Wnd* pkParent = pkMainUI->GetControl(L"SFRM_INFO");
		if( !pkParent ){ return; }

		XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkParent->GetControl(L"BLD_ICON2"));
		if( !pkBuilder ){ return; }

		int const SlotMax = pkBuilder->CountX() * pkBuilder->CountY();

		CONT_SHOPPING_BASKET kList;
		if( !g_kCashShopMgr.GetShoppingBasket(kList) )
		{
			return;
		}

		CONT_SHOPPING_BASKET::iterator item_itor = kList.begin();
		for(int i = 0; i < SlotMax; ++i )
		{
			CONT_SHOPPING_BASKET::value_type kArticle;
			
			if( item_itor != kList.end() )
			{
				kArticle = (*item_itor);
				++item_itor;
			}
			else
			{
				break;
			}

			BM::vstring kStr(L"FRM_BASKET_ICON");
			kStr += i;

			XUI::CXUI_Wnd* pkSlot = pkParent->GetControl(kStr);
			
			kStr = L"BTN_SLOT_SELECT";
			kStr += i;

			XUI::CXUI_CheckButton* pkSlotSelecter = dynamic_cast<XUI::CXUI_CheckButton*>(pkParent->GetControl(kStr));

			if( !pkSlot || !pkSlotSelecter )
			{
				continue;
			}

			if( pkSlotSelecter->Check() )
			{
				int const iArticleNo = lwUIWnd(pkSlot).GetCustomData<int>();
				if( kArticle.iIdx == iArticleNo )
				{
					g_kCashShopMgr.DelArticleToBasket( i, iArticleNo );
				}
			}
		}	
	}
	UpdateShoppingBasketItemSlot();
}

void lwCashShop::lwOnCallStaticCashItemBuy(int const ItemType, int const Subkey)
{
	if( false == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_DEVELOP ) ) //개발이 아닐 때는
	{
		g_kCashShopMgr.StaticItemType(ItemType);
		g_kCashShopMgr.StaticItemSubKey(Subkey);
		BM::GUID kGuid;
		if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
		{
			BM::Stream kPacket(PT_C_M_CS_REQ_SIMPLE_ENTER_CASHSHOP);
			kPacket.Push(kGuid);
			NETWORK_SEND(kPacket);
		}
		return;
	}

	int RealSubKey = g_kCashShopMgr.GetItemTypeToSubKey(ItemType, Subkey);
	if( 0 > RealSubKey )
	{
		return;
	}

	g_kCashShopMgr.StaticItemType(ItemType);
	g_kCashShopMgr.StaticItemSubKey(RealSubKey);
	BM::GUID kGuid;
	if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
	{
		BM::Stream kPacket(PT_C_M_CS_REQ_SIMPLE_ENTER_CASHSHOP);
		kPacket.Push(kGuid);

		int ConfirmTTID = g_kCashShopMgr.GetItemTypeToConfirmTTID(ItemType);
		if( ConfirmTTID )
		{
			CallCommonMsgYesNoBox(TTW(ConfirmTTID), 403042, 406115, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
		}
		else
		{
			NETWORK_SEND(kPacket);
		}
	}
}

void lwCashShop::lwOnCallStaticCashItemRenew(int const iIconGroup, int const iIconKey)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return; }

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }
	
	SItemPos kItemPos(iIconGroup, iIconKey);
	PgBase_Item kItem;
	if( S_OK == pkInv->GetItem(kItemPos, kItem) )
	{
		if( !kItem.EnchantInfo().IsTimeLimit() )
		{//기간제 아이템이 아니다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1845, true);
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if(!pDef)
		{//아이템이 디비에 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 22020, true);
			return;
		}
		g_kCashShopMgr.RenewItemPos(kItemPos);
		g_kCashShopMgr.StaticItemNo(kItem.ItemNo());
		BM::GUID kGuid;
		if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
		{
			BM::Stream kPacket(PT_C_M_CS_REQ_SIMPLE_ENTER_CASHSHOP);
			kPacket.Push(kGuid);
			NETWORK_SEND(kPacket);
		}
	}
}

void lwCashShop::lwOnCloseStaticCashItemBuy()
{
	BM::Stream kPacket(PT_C_M_CS_REQ_SIMPLE_EXIT_CASHSHOP);
	NETWORK_SEND(kPacket);
}

void lwCashShop::SetPreviewEquipIcon(RECV_UI_EQUIP_PREVIEW_CONT const& ItemCont)
{
	XUI::CXUI_Wnd*	pUICashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( !pUICashShop ){ return; }
	XUI::CXUI_Wnd*	pInfoWnd = pUICashShop->GetControl(L"SFRM_INFO");
	if( !pInfoWnd ){ return; }
	XUI::CXUI_Wnd*	pEquipInfo = pInfoWnd->GetControl(L"SFRM_BODY_BG");
	if( !pEquipInfo ){ return; }
	pEquipInfo = pEquipInfo->GetControl(L"SFRM_SHADOW");
	if( !pEquipInfo ){ return; }

	ClearPreviewEquipIcon();

	RECV_UI_EQUIP_PREVIEW_CONT::const_iterator	iter = ItemCont.begin();
	while( iter != ItemCont.end() )
	{
		BM::vstring	vStr(L"FRM_ICON");
		vStr += ChangeEqLocToIconLoc(iter->iEquipPos);
		XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(pEquipInfo->GetControl(vStr));
		if( pIcon )
		{
			pIcon->SetCustomData(&(*iter), sizeof(SPreviewItemInfo));
		}
		++iter;
	}
}

void lwCashShop::lwClearPreviewEquipIcon()
{
	g_kCashShopMgr.ClearPreviewEquip();
	ClearPreviewEquipIcon();
}

void lwCashShop::lwSendRegMyLank(bool const bIsVisible)
{
	SPT_C_M_CS_REQ_MODIFY_VISABLE_RANK	kData;
	kData.bState = (BYTE)bIsVisible;

	BM::Stream	kPacket;
	kData.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void lwCashShop::ClearPreviewEquipIcon()
{
	XUI::CXUI_Wnd*	pUICashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( !pUICashShop ){ return; }
	XUI::CXUI_Wnd*	pInfoWnd = pUICashShop->GetControl(L"SFRM_INFO");
	if( !pInfoWnd ){ return; }
	XUI::CXUI_Wnd*	pEquipInfo = pInfoWnd->GetControl(L"SFRM_BODY_BG");
	if( !pEquipInfo ){ return; }
	pEquipInfo = pEquipInfo->GetControl(L"SFRM_SHADOW");
	if( !pEquipInfo ){ return; }

	for( int i = 0; i < MAX_EQUIP_SLOT; ++i )
	{
		BM::vstring	vStr(L"FRM_ICON");
		vStr += i;
		XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(pEquipInfo->GetControl(vStr));
		if( pIcon )
		{
			pIcon->ClearCustomData();
		}
	}
}

void lwCashShop::SetLankingData(XUI::CXUI_Wnd* pkWnd, CONT_CASHRANKINFO const& rkData)
{
	if( !rkData.size() ){ return; }
	XUI::CXUI_Wnd* pkLanking = pkWnd->GetControl(L"FRM_RANK");
	if( !pkLanking ){ return; }

	int iElemCnt = 0;
	CONT_CASHRANKINFO::const_iterator	c_iter = rkData.begin();
	while( c_iter != rkData.end() )
	{
		if( iElemCnt == 10 ){ break; }
		BM::vstring	kStr(L"FRM_RANK_ELE");
		kStr += iElemCnt;
		XUI::CXUI_Wnd* kElem = pkLanking->GetControl(kStr);
		if( kElem )
		{
			kStr = c_iter->i64Cash;
			kStr += L" ";
			kStr += TTW(403062);
			kElem->Text(c_iter->kName);
			kElem->GetControl(L"FRM_CASH_TEXT")->Text(kStr);
		}
		++iElemCnt;
		++c_iter;
	}
}

void lwCashShop::SetBestItemInfo(XUI::CXUI_Wnd* pkWnd)
{
	EQUIP_PREVIEW_CONT kBest;
	g_kCashShopMgr.GetItemByDispType(kBest);

	if( kBest.empty() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkBest = pkWnd->GetControl(L"FRM_BEST");
	if(!pkBest)
	{
		return;
	}

	EQUIP_PREVIEW_CONT::const_iterator best_it = kBest.begin();
	for(int i = 0; i < 5; ++i)
	{
		BM::vstring kEle(L"FRM_BEST_ELE");
		kEle+=i;
		XUI::CXUI_Wnd* pkEle = pkBest->GetControl(kEle);
		if(!pkEle)
		{
			return;
		}

		std::wstring kName;
		BM::vstring kTime;
		XUI::CXUI_Wnd* pkImg = pkEle->GetControl(L"IMG_ICON");
		pkImg->Visible(false);
		if (best_it != kBest.end())
		{
			assert((*best_it).kContCashItemPrice.empty());//가격이 없는 놈은 에러!
			CONT_CASH_SHOP_ITEM_PRICE::const_iterator price_it = (*best_it).kContCashItemPrice.begin();
			if( (*best_it).kContCashItemPrice.end() != price_it )
			{
				kName = (*best_it).wstrName;
				kTime = ( (*price_it).second.IsDiscount ? (*price_it).second.iDiscountCash : (*price_it).second.iCash);
				kTime+=TTW(403062);
				DrawCashItem(pkEle, (*best_it), 0.571428f);
				++best_it;
			}
		}

		XUI::CXUI_Wnd* pArtName = pkEle->GetControl(L"FRM_NAME_TEXT");
		if( pArtName )
		{
			int const iLimitWidth = pArtName->Width() - 4;
			Quest::SetCutedTextLimitLength(pArtName, kName, _T("..."), iLimitWidth);
		}
		pkEle->GetControl(L"FRM_CASH_TEXT")->Text(kTime);
	}
}

void lwCashShop::lwRotatePreviewActor(float const fAngle)
{
	PgActor* pActor = g_kCashShopMgr.GetPreviewActor();
	if( pActor )
	{
		pActor->IncRotate(fAngle);
	}
}

void lwCashShop::lwAddPreviewActor(char const* UIModel, char const* UIActorName)
{
	PgActorPtr	pkActor = g_kPilotMan.GetPlayerActor();
	if(!pkActor)
	{
		return;
	}

	PgUIModel* pkModel = g_kUIScene.FindUIModel(UIModel);
	if( pkModel )
	{
		pkModel->DeletePgActor(UIActorName);
		pkModel->AddPgActor(UIActorName, pkActor, true, false);
		PgActor* pkCloneActor = pkModel->GetPgActor(UIActorName);
		g_kCashShopMgr.SetPreviewActor(SModelActorInfo(UIModel, UIActorName));
		pkModel->SetCameraZoomMinMax(0.15f, 1.0f);
		pkModel->SetOrthoZoom(1.0f);

		SCameraResetInfo kInfo;
		kInfo.fYCenter = 0.9f;
		kInfo.eHeightSet = SCameraResetInfo::EHS_FIX_MODEL_VALUE;
		pkModel->CameraReset(pkCloneActor->GetNIFRoot(), kInfo);
		g_kCashShopMgr.InitPreviewActor();
	}
}

void lwCashShop::lwRemovePreviewActor(char const* UIModel, char const* UIActorName)
{
	PgActorPtr	pkActor = g_kPilotMan.GetPlayerActor();
	if(!pkActor)
	{
		return;
	}

	PgUIModel* pkModel = g_kUIScene.FindUIModel(UIModel);
	if( pkModel )
	{
		pkModel->DeletePgActor(UIActorName);
	}
}

bool lwCashShop::ReturnResult(int const iError, int const iReqLevel)
{
	switch(iError)
	{
	case CSR_SUCCESS:
		{
			return true;
		}
	case CSR_USE_BADWORD:
		{
			lwAddWarnDataTT(700144);
			lwAddWarnDataTT(1919);
		}break;
	case CSR_NOT_HAVE_TIMELIMIT:
	case CSR_NOT_MATCH_TIMELIMIT:
	case CSR_TIMELIMIT_OVERFLOW:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1920 + iError, true);
		}break;
	case CSR_NEED_RELOAD_DATA:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1939, true);
		}break;
	case CSR_NOT_ENOUGH_LEVEL:
		{// iReqLevel 레벨 이상이어야만 가능함을 알림
			wchar_t szBuf[MAX_PATH] ={0,};
			wsprintfW(szBuf, TTW(790217).c_str(), iReqLevel);
			lua_tinker::call<void, lwWString, bool>("CommonMsgBoxByText", lwWString(szBuf), true);
		}break;
	case CSR_NOT_BUY_LIMIT: //한정판매 수량 초과(계정당)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 3571, true);
		}
		break;
	case CSR_NOT_DATE_LIMIT: // 한정판매 기간 초과 또는 시간이 안된경우 
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 3570, true);
		}
		break;
	case CSR_NOT_BUY_LIMIT_OVEW: // 한정판매 수량 초과(전체 목록)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 3572, true);
		}
		break;
	default:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1919 + iError, true);
		}break;
	}

	return false;
}

void lwCashShop::lwOpenCashChargeURL()
{
	std::wstring kCashChargeURL = g_pkApp->GetCashChargeURL();

	if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
	{
		kCashChargeURL += g_kWebLogin.SessKey();
	}

	if (g_pkApp)
	{
		if (g_pkApp->GetCashChargeURL().size())
		{
			if( true == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_TAIWAN ) )
			{
				static int const iDigestSize = 16;

				time_t cur_Time;
				time(&cur_Time);
				char time_sec[255] = {0,};
				_i64toa_s(cur_Time, time_sec, 255, 10);
				std::string strKey = "525|";
				strKey+=time_sec;
				strKey+="|ml|";//game name
				strKey+=MB(g_kNetwork.LoginID());
				strKey+="|0|3hfahoreidogxoscyutzgmlmj+4=";//서버 아이디|키
				
				md5_state_t md5state;
				md5_byte_t digest[iDigestSize];
				md5_init(&md5state);
				md5_append(&md5state, (md5_byte_t const *)(strKey.c_str()), static_cast<int>(strKey.length()));
				md5_finish(&md5state, digest);

				char hex_output[iDigestSize*2+1] = {0,};
				for (int i = 0; i<iDigestSize; ++i)
				{
					sprintf_s(hex_output + i*2, iDigestSize*2, "%02x", digest[i]);
				}

				std::wstring url = kCashChargeURL;//L"https://gfb.gameflier.com/Billing/ingame/index_new.asp?";
				url+=L"APPID=525&USERNAME=";
				url+=g_kNetwork.LoginID();
				url+=L"&GAME=ml&SERVER_ID=0&TS=";
				url+=UNI(time_sec);
				url+=L"&TOKEN=";
				url+=UNI(hex_output);

				HINSTANCE hInstance = ShellExecute(NULL, _T("open") , url.c_str(), NULL, NULL, SW_SHOWNORMAL);
			} 
			else
			{
				HINSTANCE hInstance = ShellExecute(NULL, _T("open") , kCashChargeURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
}

void lwCashShop::lwSetUseMileage(bool const bUse, lwUIWnd kWnd)
{	
	g_kCashShopMgr.SetUseMileage(bUse, kWnd.GetSelf());
	if(!bUse)
	{
		lwUpdateTotalCashExpression(kWnd);
	}
}

int	 lwCashShop::lwGetShopType()
{
	return static_cast<int>(g_kCashShopMgr.GetShopType());
}

bool lwCashShop::lwSetShopType(int const i)
{
	PgCashShop::ECashShopType eType = static_cast<PgCashShop::ECashShopType>(i);
	SetVisibleCashShopUI(eType);

	switch(eType)
	{
	case PgCashShop::ECASH_SHOP:
		{
			g_kCashShopMgr.SetBuyTypeUI(PgCashShop::ECASH_SHOP);
		}break;
	case PgCashShop::EMILEAGE_SHOP:
		{
			g_kCashShopMgr.SetBuyTypeUI(PgCashShop::EMILEAGE_SHOP);
		}break;
	}

	switch(eType)
	{
	case PgCashShop::ECASH_SHOP:
	case PgCashShop::EMILEAGE_SHOP:
		{
			g_kCashShopMgr.SetShopType(eType);
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_CASH_SHOP"));
			if( pkWnd && !pkWnd->IsClosed() )
			{				
				SelectedMoveBtnClear(pkWnd->GetControl(_T("SFRM_INFO")));
				SelectCashTypeClear(pkWnd->GetControl(_T("SFRM_INFO")));
				TBL_DEF_CASH_SHOP_CATEGORY kCate;//임시
				XUI::CXUI_Wnd* pDefault = NULL;
				for(int k = PgCashShop::ECSCATEGORYNUM_BEGIN; k < PgCashShop::ECSCATEGORYNUM_END; ++k)
				{
					BM::vstring kTabName(L"CBTN_TAB");
					kTabName+=k;
					XUI::CXUI_Wnd* pkTab = pkWnd->GetControl(kTabName);
					if (pkTab)
					{
						const bool IsVisible = g_kCashShopMgr.GetCategory(k, kCate);
						pkTab->Visible(IsVisible);
						if(IsVisible)
						{
							pkTab->Text(kCate.wstrName);//탭 이름
						}
						XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTab);
						if( pBtn )
						{
							pBtn->Disable(!kCate.bIsUsed);
							if( pBtn->Enable() && k == 0 )
							{
								pBtn->Check(true);
								pDefault = pkTab;
							}
						}
						if( NULL == pDefault )
						{
							pDefault = pkTab;
						}
					}
				}
				lua_tinker::call<void, lwUIWnd>("OnCall_CashShopTopCbtn", lwUIWnd(pDefault));
				lua_tinker::call<void, lwUIWnd>("UI_Click_CashShop_Tab", lwUIWnd(pDefault));
			}
		}break;
	case PgCashShop::ECASHSHOP_GACHA:
	case PgCashShop::ECOSTUME_MIX:
		{
			g_kCashShopMgr.SetShopType(eType);
		}break;
	default:
		{
			_PgMessageBox("lwCashShop::lwSetShopType()", "Unknown ShopType");
			return false;
		}break;
	}
	return true;
}

void lwCashShop::SetVisibleCashShopUI(PgCashShop::ECashShopType const eCashShopType)
{
	XUI::CXUI_Wnd* pkMain= XUIMgr.Get(_T("FRM_CASH_SHOP"));
	if(!pkMain)
	{
		return;
	}
	bool bVisibleDefaultCashShopUI = true;
	bool bVisibleCashGacha = false;
	bool bVisibleCostumeMix = false;


	PgCashShop::ECashShopForm eFormType = PgCashShop::ECSFORM_UNKNOWN;



	switch(eCashShopType)
	{
	case PgCashShop::ECASH_SHOP:
	case PgCashShop::EMILEAGE_SHOP:
		{
			bVisibleDefaultCashShopUI = true;
			bVisibleCashGacha = false;
			bVisibleCostumeMix = false;
		}break;
	case PgCashShop::ECASHSHOP_GACHA:
		{
			bVisibleDefaultCashShopUI = false;
			bVisibleCashGacha = true;
			bVisibleCostumeMix = false;
		}break;
	case PgCashShop::ECOSTUME_MIX:
		{
			bVisibleDefaultCashShopUI = false;
			bVisibleCashGacha = false;
			bVisibleCostumeMix = true;
		}break;
	default:
		{
			_PgMessageBox("lwCashShop::lwSetShopType()", "Unknown ShopType");
			return;
		}break;
	}
	
	{// 캐시샵 UI 부분
		XUI::CXUI_Wnd* pkWnd = pkMain->GetControl(L"FRM_PAGE_BG");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}

		if(bVisibleDefaultCashShopUI)
		{
			SetShopForm(pkMain, PgCashShop::ECSFORM_COMMON);
		}
		else
		{
			SetShopForm(pkMain, PgCashShop::ECSFORM_UNKNOWN);
		}

		pkWnd = pkMain->GetControl(L"FRM_LIST_BG");
		if(pkWnd)
		{
			pkWnd->Visible(false);
		}
		pkWnd = pkMain->GetControl(L"FRM_RANK");
		if(pkWnd)
		{
			pkWnd->Visible(false);
		}
		pkWnd = pkMain->GetControl(L"FRM_BEST");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}
		pkWnd = pkMain->GetControl(L"CBTN_LEFT_TAB1");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}
		pkWnd = pkMain->GetControl(L"CBTN_LEFT_TAB0");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}
		pkWnd = pkMain->GetControl(L"SFRM_BG");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}
		pkWnd = pkMain->GetControl(L"IMG_BG_TOP");
		if(pkWnd)
		{
			pkWnd->Visible(bVisibleDefaultCashShopUI);
		}
		pkWnd = pkMain->GetControl(L"FRM_PAGE");
		if(pkWnd)
		{
			pkWnd->Visible(false);
		}

		{// 탭 Visible
			XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(L"BLD_CBTN_TAB"));
			if(pkBuild)
			{
				int const iMaxUICnt = pkBuild->CountX()*pkBuild->CountX();
				for( int i = 0; i < iMaxUICnt; ++i)
				{
					if(PgCashShop::ECSCATEGORY_GIFT != i)
					{// 7번은 선물함 버튼으로 UI에 하드코딩 되어있음
						BM::vstring kTabID(L"CBTN_TAB");
						kTabID+=i;
						XUI::CXUI_CheckButton* pkTab =dynamic_cast<XUI::CXUI_CheckButton*>( pkMain->GetControl(kTabID));
						if(pkTab)
						{
							pkTab->Visible(bVisibleDefaultCashShopUI);
							if(0 == i && bVisibleDefaultCashShopUI)
							{
								pkTab->Check(true);
								pkTab->ClickLock(true);
								lua_tinker::call<void, lwUIWnd>("UI_Click_CashShop_Tab", lwUIWnd(pkTab));
							}
						}
					}
				}
			}
			pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(L"BLD_CBTN_SECOND_TAB"));
			if(pkBuild)
			{
				int const iMaxUICnt = pkBuild->CountX()*pkBuild->CountX();
				for( int i = 0; i < iMaxUICnt; ++i)
				{
					BM::vstring kTabID(L"CBTN_SECOND_TAB");
					kTabID+=i;
					XUI::CXUI_Wnd* pkTab = pkMain->GetControl(kTabID);
					if(pkTab)
					{
						pkTab->Visible(bVisibleDefaultCashShopUI);
					}
				}
			}
		}
	}

	{// 캐시샵 가차 UI를
		XUI::CXUI_Wnd* pkWnd = pkMain->GetControl(L"SFRM_CASH_GACHA");
		if(pkWnd)
		{// 보이거나 감춘다
			pkWnd->Visible(bVisibleCashGacha);
		}
	}

	{
		XUI::CXUI_Wnd* pkWnd = pkMain->GetControl(L"SFRM_COSTUME_MIX");
		if(pkWnd)
		{// 보이거나 감춘다
			pkWnd->Visible(bVisibleCostumeMix);
		}
	}
}

void lwCashShop::lwUpdateTotalCashExpression(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* const pkBuyWnd = kWnd.GetSelf(); //XUIMgr.Get(_T("SFRM_CASHSHOP_BUY"));
	if(!pkBuyWnd)
	{
		return;
	}	
	XUI::CXUI_Wnd* const pkBuyBtn = pkBuyWnd->GetControl(L"BTN_BUY");
	if(!pkBuyBtn)
	{
		return;
	}
	BM::vstring kTotalCashWndText;
	if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
	{
		kTotalCashWndText = L"FRM_TOTAL_HAPPYCOIN";
	}
	else
	{
		kTotalCashWndText = L"FRM_TOTAL";
	}
	XUI::CXUI_Wnd* const pkTotalCashWnd = pkBuyWnd->GetControl(kTotalCashWndText);
	if(!pkTotalCashWnd)
	{
		return;
	}
	BM::vstring kTotalMileageWndText;
	if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
	{
		kTotalMileageWndText = L"FRM_TOTAL_TOL";
	}
	else
	{
		kTotalMileageWndText = L"FRM_TOTAL";
	}
	XUI::CXUI_Wnd* const pkCostWnd = pkBuyWnd->GetControl(kTotalMileageWndText);
	if(!pkCostWnd)
	{
		return;
	}

	PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
	if(!pPilot)
	{
		return;
	}

	XUI::CXUI_Edit* const pkEdtWnd = dynamic_cast<XUI::CXUI_Edit*>(pkBuyWnd->GetControl(L"EDT_DP"));
	XUI::CXUI_CheckButton* const pkMileageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBuyWnd->GetControl(L"CBTN_USER_DP"));

	//int iIndex=0;
	//pkBuyWnd->GetCustomData(&iIndex, sizeof(int));	//아티클 index
	
//	TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
//	pkBuyBtn->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
	__int64 i64UseMileage=0;
	__int64 iTotalCost = g_kCashShopMgr.GetRememberTotalPrice();
							//(kPrice.IsDiscount)?(kPrice.iDiscountCash):(kPrice.iCash);
	if( pkEdtWnd )
	{
		{// 마일리지 edit칸에 적혀있는 값을 얻어와
			std::wstring const& kTemp = pkEdtWnd->EditText();
			i64UseMileage = ::_wtoi64(kTemp.c_str());
		}

		__int64 const iMyMileage = pPilot->GetAbil64(AT_BONUS_CASH);
		if(iMyMileage < i64UseMileage)
		{// 가지고 있는 마일리지 보다 크면, 가지고 있는 마일리지 만큼으로 수정하고
			i64UseMileage = iMyMileage;
			wchar_t	szTemp[MAX_PATH] = {0,};
			::_i64tow_s(i64UseMileage, szTemp, MAX_PATH, 10);
			pkEdtWnd->EditText(szTemp);
		}

		if(iTotalCost < i64UseMileage)
		{// 아이템 가격보다 마일리지가 크면, 아이템 가격만큼으로 수정하고
			i64UseMileage = iTotalCost;
			wchar_t	szTemp[MAX_PATH] = {0,};
			::_i64tow_s(i64UseMileage, szTemp, MAX_PATH, 10);
			pkEdtWnd->EditText(szTemp);
		}	
	}

	if( pkMileageBtn )
	{
		// 이 마일리지 값을 마일리지 버튼에 저장한후
		pkMileageBtn->SetCustomData(&i64UseMileage, sizeof(i64UseMileage));
	}

	if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
	{
		__int64 i64MyTol = 0;
		i64MyTol = pPilot->GetAbil64(AT_CASH);
		__int64 i64MyHappyCoin = 0;
		i64MyHappyCoin = pPilot->GetAbil64(AT_BONUS_CASH);

		if(IsBuyingCashItem())
		{//아이템구매,기간 연장시 : 해피코인 우선 차감, 모자라는 금액 톨에서 차감
			__int64 const i64RemainCost = iTotalCost - i64MyHappyCoin;
			if( 0 < i64RemainCost)
			{
				pkTotalCashWnd->Text(L"0");
				pkCostWnd->Text(BM::vstring( i64MyTol - i64RemainCost));
			}
			else
			{
				pkTotalCashWnd->Text(BM::vstring(i64MyHappyCoin - iTotalCost));
			}
		}
		else
		{//아이템 선물 시: 톨만 소모 할 수 있음, 해피 코인 사용 불가
			pkCostWnd->Text(BM::vstring( i64MyTol - iTotalCost));
		}
	}
	else
	{
		__int64 i64MyCash = 0;
		if(IsBuyingCashItem())
		{
			i64MyCash = pPilot->GetAbil64(AT_CASH);
		}
		else
		{
			i64MyCash = pPilot->GetAbil64(AT_BONUS_CASH);
		}
		__int64 const iCalculatedItemPrice =iTotalCost-i64UseMileage;
		pkCostWnd->Text(BM::vstring(iCalculatedItemPrice));	

		if(iCalculatedItemPrice==0)
		{// 하단의 계산된 남은 캐시를 표현한다
			pkTotalCashWnd->Text(BM::vstring(i64MyCash));
		}
		else
		{
			pkTotalCashWnd->Text(BM::vstring((i64MyCash-iCalculatedItemPrice)));
		}
	}
}

void lwCashShop::lwClearCashShopInfo()
{
	__int64 i64Zero=0;
	g_kCashShopMgr.SetRememberTotalPrice(i64Zero);
	g_kCashShopMgr.SetRememberItemMileage(i64Zero);
}

void lwCashShop::OffAllMileageWnd(bool const bOff, XUI::CXUI_Wnd* const pkBuyWnd)
{	
	if(!pkBuyWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* const pkBgWnd = pkBuyWnd->GetControl(L"SFRM_DPNUM_BG");
	if(!pkBgWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* const pkEdtWnd = pkBuyWnd->GetControl(L"EDT_DP");
	if(!pkEdtWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* const pkMileageBtn = pkBuyWnd->GetControl(L"CBTN_USER_DP");
	if(!pkMileageBtn)
	{
		return;
	}
	XUI::CXUI_Wnd* const pkCashBtn = pkBuyWnd->GetControl(L"BTN_CASH");
	if(!pkCashBtn)
	{
		return;
	}

	if(bOff)
	{	
		pkEdtWnd->Visible(false);
		pkMileageBtn->Visible(false);
		pkBgWnd->Visible(false);
		pkCashBtn->Visible(false);
	}
	else
	{
		pkMileageBtn->Visible(true);
		pkCashBtn->Visible(true);
	}
}

int lwCashShop::lwCurrentBuyType()
{
	int iType = static_cast<int>(g_kCashShopMgr.GetBuyTypeUI());
	return iType;
}

bool lwCashShop::IsBuyingCashItem()
{
	return (PgCashShop::ECASH_SHOP == g_kCashShopMgr.GetBuyTypeUI());
}

bool lwCashShop::PageUIVisibleCheck(int const iType, int& iMaxPage)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return false;
	}

	int const MAX_Slot = pkInv->GetMaxIDX(static_cast<EInvType>(iType));

	iMaxPage = MAX_Slot / MAX_DEF_INVUI_SLOT_COUNT;
	iMaxPage += ((MAX_Slot % MAX_DEF_INVUI_SLOT_COUNT) != 0)?(1):(0);

	return iMaxPage > 1;
}

void lwCashShop::SetCashInvViewSlot(XUI::CXUI_Wnd* pParent, int const iType, int const iPage)
{
	if( !pParent )
	{
		return;
	}

	SelectedMoveBtnClear(pParent);

	int iInvType = 0;

	for( int i = 0; i < MAX_ICON_SLOT; ++i )
	{
		BM::vstring vStr(_T("ICN_CASH_INV"));
		vStr += i;

		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pParent->GetControl(vStr));

		vStr = L"FRM_BASKET_ICON";
		vStr += i;
		XUI::CXUI_Wnd* pBasketIcon = pParent->GetControl(vStr);
		if( pIcon && pBasketIcon )
		{
			pIcon->Visible(true);
			pBasketIcon->Visible(false);
			SIconInfo	kInfo = pIcon->IconInfo();
			switch( iType )
			{
			case 0:{ kInfo.iIconGroup = KUIG_CASH;		}break;
			case 1:{ kInfo.iIconGroup = KUIG_CASH_SAFE; }break;
			default:
				{ 
					pIcon->Visible(false);
					pBasketIcon->Visible(true);
					continue;
				}break;
			}
			kInfo.iIconKey = (iPage * MAX_DEF_INVUI_SLOT_COUNT) + pIcon->BuildIndex();
			pIcon->SetIconInfo(kInfo);

			iInvType = kInfo.iIconGroup;
		}
	}

	if( iType == 2 )
	{
		SetShoppingBasketItemSlot(pParent);
	}

	XUI::CXUI_Wnd* pMoveBtn = pParent->GetControl(_T("BTN_MOVEITEM"));
	XUI::CXUI_Wnd* pRenewBtn = pParent->GetControl(_T("BTN_RENEWITEM"));
	if( pMoveBtn )
	{
		switch( iType )
		{
		case 0:
		case 1:
			{
				pMoveBtn->Text(TTW(1889 - iType));
				pRenewBtn->Text(TTW(1842));
			}break;
		case 2:
			{
				pMoveBtn->Text(TTW(790259));
				pRenewBtn->Text(TTW(790260));
			}break;
		default:
			{
			}break;
		}
	}

	XUI::CXUI_Wnd* pPageUI = pParent->GetControl(L"FRM_PAGE");
	if( pPageUI )
	{
		int iMaxPage = 0;
		pPageUI->Visible(PageUIVisibleCheck(iInvType, iMaxPage));
		BM::vstring kStr(iPage + 1);
		pPageUI->Text(kStr);

		SItemPos kPageInfo(iPage, iMaxPage);
		pPageUI->ClearCustomData();
		pPageUI->SetCustomData(&kPageInfo, sizeof(kPageInfo));
	}
}

void lwCashShop::ViewChangeModel(bool const bIsEquipView)
{
	XUI::CXUI_Wnd* pParent = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( !pParent )
	{
		return;
	}

	pParent = pParent->GetControl(L"SFRM_INFO");
	if( !pParent )
	{
		return;
	}

	pParent = pParent->GetControl(L"SFRM_BODY_BG");
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pEquipModel = pParent->GetControl(L"SFRM_SHADOW");
	XUI::CXUI_Wnd* pObjectModel = pParent->GetControl(L"FRM_UIMODEL_OBJECT");
	if( !pEquipModel || !pObjectModel )
	{
		return;
	}

	pEquipModel->Visible(bIsEquipView);
	pObjectModel->Visible(!pEquipModel->Visible());
}

void lwCashShop::UpdateShoppingBasketItemSlot()
{
	XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( pMainUI )
	{
		XUI::CXUI_Wnd* pkParent = pMainUI->GetControl(L"SFRM_INFO");
		if( pkParent )
		{
			XUI::CXUI_Wnd* TapMain = pkParent->GetControl(L"SFRM_CASH_TITLE");
			if( TapMain )
			{
				XUI::CXUI_CheckButton* pkTapBasket = dynamic_cast<XUI::CXUI_CheckButton*>(TapMain->GetControl(L"BTN_TITLE_TAB2"));
				if( pkTapBasket )
				{
					if( pkTapBasket->Check() )
					{
						SetShoppingBasketItemSlot(pkParent);
					}
				}
			}
		}
	}
}

void lwCashShop::SetShoppingBasketItemSlot(XUI::CXUI_Wnd* pkParent)
{
	if( !pkParent ){ return; }

	CONT_SHOPPING_BASKET kBasket;
	g_kCashShopMgr.GetShoppingBasket(kBasket);

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkParent->GetControl(L"BLD_ICON2"));
	if( !pkBuilder ){ return; }

	int const MAX_SLOT = pkBuilder->CountX() * pkBuilder->CountY();

	CONT_SHOPPING_BASKET::const_iterator article_itor = kBasket.begin();
	for( int i = 0; i < MAX_SLOT; ++i )
	{
		BM::vstring kStr(L"FRM_BASKET_ICON");
		kStr += i;

		XUI::CXUI_Wnd* pkSlot = pkParent->GetControl(kStr);
		if( !pkSlot )
		{
			continue;
		}

		TBL_DEF_CASH_SHOP_ARTICLE kArticle;
		if( article_itor != kBasket.end() )
		{
			if( (*article_itor).iIdx == 0 )
			{
				CashItemDrawToScaling(pkSlot, kArticle, 1.0f, 40);
			}
			else
			{
				pkSlot->Visible(true);
				pkSlot->SetCustomData(&(*article_itor).iIdx, sizeof((*article_itor).iIdx));
				CashItemDrawToScaling(pkSlot, (*article_itor), 0.571428f);
			}
			++article_itor;
		}
		else
		{
			CashItemDrawToScaling(pkSlot, kArticle, 1.0f, 40);
		}
	}
}

void lwCashShop::CashItemDrawToScaling(XUI::CXUI_Wnd* pkSlot, TBL_DEF_CASH_SHOP_ARTICLE const &rkArticle, float const fScale, int const IconSize)
{
	if( !pkSlot ){ return; }

	if( rkArticle.wstrIconPath.empty() )
	{
		SUVInfo kUVInfo(1, 1, 1);
		POINT2 kLoc = pkSlot->Location();
		POINT2 kSize( IconSize, IconSize );
		pkSlot->DefaultImgName(L"");
		pkSlot->ImgSize( kSize );
		pkSlot->UVInfo(kUVInfo);
		pkSlot->Scale(fScale);
		pkSlot->Location(kLoc);
		pkSlot->SetInvalidate();
		pkSlot->ClearCustomData();
		pkSlot->Visible(false);
	}
	else
	{
		POINT2 kSize( IconSize * rkArticle.bU, IconSize * rkArticle.bV );
		POINT2 kLoc = pkSlot->Location();
		pkSlot->DefaultImgName(rkArticle.wstrIconPath);
		pkSlot->ImgSize( kSize );
		int iGenderLimit = 0;

		bool bIsMultiRes = false;
		bool bIsNewMultiRes = false;

		CONT_CASH_SHOP_ITEM::const_iterator detail_itor = rkArticle.kContCashItem.begin();
		while(detail_itor != rkArticle.kContCashItem.end())
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef((*detail_itor).iItemNo);
			if(pkItemDef)
			{
				if(pkItemDef->GetAbil(AT_MULTI_RES))
				{
					bIsMultiRes = true;
				}
				if(pkItemDef->GetAbil(AT_NEW_MULTI_TRS))
				{
					bIsNewMultiRes = true;
				}

				int const iGenderLimitAbil = pkItemDef->GetAbil(AT_GENDERLIMIT);
				if(	iGenderLimitAbil == GWL_MALE
					||	iGenderLimitAbil == GWL_FEMALE )
				{
					iGenderLimit = iGenderLimitAbil;
				}
			}

			++detail_itor;
		}

		SUVInfo kUVInfo(rkArticle.bU, rkArticle.bV, rkArticle.bUVIndex);

		if(true == bIsMultiRes)
		{
			kUVInfo.Index += ClientItemResChooser(iGenderLimit);
		}
		if(true == bIsNewMultiRes)
		{
			kUVInfo.Index += ClientItemNewResChooser(iGenderLimit);
		}

		pkSlot->UVInfo(kUVInfo);
		pkSlot->Scale(fScale);
		pkSlot->Location(kLoc);
		pkSlot->SetInvalidate();
		pkSlot->SetCustomData(&(rkArticle.iIdx), sizeof(rkArticle.iIdx));
		pkSlot->Visible(true);
		
	}
}
void lwCashShop::RegShoppingBasketItem(int const iArticleIDX)
{
	g_kCashShopMgr.AddNewArticleToBasket(iArticleIDX);
	UpdateShoppingBasketItemSlot();
}

void lwCashShop::UnRegShoppingBasketItem(int const iSlotIDX, int const iArticleIDX)
{
	g_kCashShopMgr.DelArticleToBasket(iSlotIDX, iArticleIDX);
	UpdateShoppingBasketItemSlot();
}

void lwCashShop::CheckSelectedSlotItem(CONT_SHOPPING_BASKET_SLOT_IDX& kIDX, EQUIP_PREVIEW_CONT& kItems)
{
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( !pkMainUI ){ return; }

	XUI::CXUI_Wnd* pkParent = pkMainUI->GetControl(L"SFRM_INFO");
	if( !pkParent ){ return; }

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkParent->GetControl(L"BLD_ICON2"));
	if( !pkBuilder ){ return; }

	int const SlotMax = pkBuilder->CountX() * pkBuilder->CountY();

	CONT_SHOPPING_BASKET kList;
	if( !g_kCashShopMgr.GetShoppingBasket(kList) )
	{
		return;
	}

	CONT_SHOPPING_BASKET::iterator item_itor = kList.begin();
	for(int i = 0; i < SlotMax; ++i )
	{
		CONT_SHOPPING_BASKET::value_type kArticle;
		
		if( item_itor != kList.end() )
		{
			kArticle = (*item_itor);
			++item_itor;
		}
		else
		{
			break;
		}

		BM::vstring kStr(L"FRM_BASKET_ICON");
		kStr += i;

		XUI::CXUI_Wnd* pkSlot = pkParent->GetControl(kStr);
		
		kStr = L"BTN_SLOT_SELECT";
		kStr += i;

		XUI::CXUI_CheckButton* pkSlotSelecter = dynamic_cast<XUI::CXUI_CheckButton*>(pkParent->GetControl(kStr));

		if( !pkSlot || !pkSlotSelecter )
		{
			continue;
		}

		if( pkSlotSelecter->Check() )
		{
			int const iArticleNo = lwUIWnd(pkSlot).GetCustomData<int>();
			if( kArticle.iIdx == iArticleNo )
			{
				kIDX.push_back(i);
				kItems.push_back( kArticle );
			}
		}
	}
}

__int64 const lwCashShop::GetSelectedSlotItemTotalCost()
{
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( !pkMainUI ){ return 0; }

	XUI::CXUI_Wnd* pkParent = pkMainUI->GetControl(L"SFRM_INFO");
	if( !pkParent ){ return 0; }

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkParent->GetControl(L"BLD_ICON2"));
	if( !pkBuilder ){ return 0; }

	int const SlotMax = pkBuilder->CountX() * pkBuilder->CountY();

	CONT_SHOPPING_BASKET kList;
	if( !g_kCashShopMgr.GetShoppingBasket(kList) )
	{
		return 0;
	}

	__int64 i64TotalCost = 0;
	__int64 i64BuyCash = 0;
	__int64 i64AccMileage = 0;

	CONT_SHOPPING_BASKET::iterator item_itor = kList.begin();
	for(int i = 0; i < SlotMax; ++i )
	{
		CONT_SHOPPING_BASKET::value_type kArticle;
		
		if( item_itor != kList.end() )
		{
			kArticle = (*item_itor);
			++item_itor;
		}
		else
		{
			break;
		}

		BM::vstring kStr(L"FRM_BASKET_ICON");
		kStr += i;

		XUI::CXUI_Wnd* pkSlot = pkParent->GetControl(kStr);
		
		kStr = L"BTN_SLOT_SELECT";
		kStr += i;

		XUI::CXUI_CheckButton* pkSlotSelecter = dynamic_cast<XUI::CXUI_CheckButton*>(pkParent->GetControl(kStr));

		if( !pkSlot || !pkSlotSelecter )
		{
			continue;
		}

		if( pkSlotSelecter->Check() )
		{
			int const iArticleNo = lwUIWnd(pkSlot).GetCustomData<int>();
			if( kArticle.iIdx == iArticleNo )
			{
				CONT_CASH_SHOP_ITEM_PRICE::iterator price_iter = kArticle.kContCashItemPrice.begin();
				if( price_iter != kArticle.kContCashItemPrice.end() )
				{//가격은 무조건 한개만 가진다.
					int const iBuyCash = ((*price_iter).second.IsDiscount)?((*price_iter).second.iDiscountCash):((*price_iter).second.iCash);
					int const iMileage = kArticle.iMileage;
					i64TotalCost += iBuyCash;
					i64AccMileage += g_kCashShopMgr.CalcAddBonus(iBuyCash, iMileage);
				}
			}
		}
	}

	__int64 i64TotalBonusMileage = g_kCashShopMgr.CalcLocalAddBonus(i64TotalCost);
	g_kCashShopMgr.SetRememberItemMileage(i64AccMileage+i64TotalBonusMileage);
	return i64TotalCost;
}

int lwCashShop::lwGetStaticBuyType(lwUIWnd kMainUI)
{
	if( kMainUI.IsNil() )
	{
		return 0;
	}

	lwUIWnd kTitle = kMainUI.GetControl("SFRM_CASH_TITLE");
	if( kTitle.IsNil() )
	{
		return 0;
	}

	return kTitle.GetCustomData<int>();
}

void lwCashShop::CallStaticCashItemBuyUI(int const iArticleIDX, bool const bRenew)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		lwOnCloseStaticCashItemBuy();
		return;
	}

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if( !g_kCashShopMgr.GetItem(iArticleIDX, kArticle) )
	{
		lua_tinker::call< void, char const*, bool >("CommonMsgBox", MB(TTW(403108)), true);
		lwOnCloseStaticCashItemBuy();
		return;
	}

	if( kArticle.kContCashItemPrice.empty() || kArticle.kContCashItem.empty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1909, true);
		lwOnCloseStaticCashItemBuy();
		return;
	}

	BM::PgPackedTime kTempTime;
	kTempTime.Clear();
	if( !(kTempTime == kArticle.kSaleEndDate) )
	{//연장시 여기서 판매 종료 여부를 검사한다.
		SYSTEMTIME kTemp;
		if( g_kEventView.GetLocalTime(&kTemp) )
		{
			kTempTime = kTemp;
			if( kArticle.kSaleEndDate < kTempTime )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1908, true);
				return;
			}
		}
	}
	if (!kArticle.kContCashItemPrice.empty())	
	{
		TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
		CONT_CASH_SHOP_ITEM_PRICE::iterator	Price_iter = kArticle.kContCashItemPrice.begin();
		while( Price_iter != kArticle.kContCashItemPrice.end() )
		{
			kPrice = Price_iter->second;
			if( 0 != kPrice.iExtendCash || ( kPrice.IsExtendDiscount && 0 != kPrice.iDiscountExtendCash ) )
			{
				if( 0 != kPrice.iItemUseTime )
				{
					break;
				}
			}
			else
			{
				Price_iter = kArticle.kContCashItemPrice.erase(Price_iter);
				continue;
			}
			++Price_iter;
		}
		if( kArticle.kContCashItemPrice.empty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1909, true);
			return;
		}
	}

	CONT_CASH_SHOP_ITEM_PRICE::mapped_type const& kPrice = kArticle.kContCashItemPrice.begin()->second;

	int iBuyCash = 0;
	if( bRenew )
	{// 기간 연장 가격 할인 여부를 판단해 가격을 얻어온다
		iBuyCash = (kPrice.IsExtendDiscount ? kPrice.iDiscountExtendCash : kPrice.iExtendCash);
	}
	else
	{// 일반 가격 할인 여부를 판단해 가격을 얻어옴
		iBuyCash = (kPrice.IsDiscount ? kPrice.iDiscountCash : kPrice.iCash);
	}
	
	XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_CASH_OUT_BUY");
	if( !pMainUI ){ return; }

	pMainUI->SetCustomData(&iArticleIDX, sizeof(iArticleIDX));

	DrawCashItem(pMainUI, kArticle);

	XUI::CXUI_Wnd* pkTitle = pMainUI->GetControl(L"SFRM_CASH_TITLE");
	if( pkTitle )
	{
		XUI::CXUI_Wnd* pkTemp = pkTitle->GetControl(L"IMG_TITLE_TAB");
		if( pkTemp )
		{
			pkTemp->Text(TTW((bRenew)?(1842):(1835)));
		}
		
		int iType = (bRenew)?(CIBT_STATIC_ITEM_RENEW):(CIBT_STATIC_ITEM_BUY);
		pkTitle->SetCustomData(&iType, sizeof(iType));
	}

	XUI::CXUI_Wnd* pkTemp = pMainUI->GetControl(L"FRM_NAME");
	if( pkTemp )
	{
		Quest::SetCutedTextLimitLength(pkTemp, kArticle.wstrName, L"...");
	}


	XUI::CXUI_Wnd* pMainContents = XUIMgr.Call(L"FRM_CONTENTS_LIST");
	if( !pMainContents ){ return; }
	if( pMainContents )
	{
		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pMainUI->GetControl(L"LST_CONTENTS"));
		if( pkList )
		{
			pkList->ClearList();
			XUI::SListItem* pkItem = pkList->AddItem(L"");
			std::wstring strDiscription = kArticle.wstrDiscription;
			
			pkItem->m_pWnd->Text(strDiscription );
			XUI::CXUI_Style_String kString = pkItem->m_pWnd->StyleText();
			const POINT kCalcSize = Pg2DString::CalculateOnlySize(kString);
			pkItem->m_pWnd->Size(pkItem->m_pWnd->Size().x,kCalcSize.y);
		}
	}

	pkTemp = pMainUI->GetControl(L"FRM_CASH");
	if( pkTemp )
	{
		pkTemp->Text(BM::vstring(pkPlayer->GetAbil64(AT_CASH)));
	}

	SetDayBtnInitialize(pMainUI, kArticle, iBuyCash, (bRenew)?(CIBT_RENEW):(CIBT_BUY));

	pkTemp = pMainUI->GetControl(L"CBTN_DAY0");
	if( pkTemp )
	{
		pkTemp->DoScript(SCRIPT_ON_L_BTN_DOWN);
	}

	pkTemp = pMainUI->GetControl(L"BTN_BUY");
	if( pkTemp )
	{
		BM::vstring kStr(TTW((bRenew)?(121):(400172)));
		kStr += TTW(2200);
		pkTemp->Text(kStr);
	}
}
