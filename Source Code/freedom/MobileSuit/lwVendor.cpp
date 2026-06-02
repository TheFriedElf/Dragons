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
#include "PgAction.h"
#include "lwCashItem.h"
#include "PgMobileSuit.h"
#include "lwVendor.h"
#include "PgVendor.h"

int const MAX_VENDOR_INV_COUNT = 24;

namespace lwVendor
{
	void lwOnOverVendorItemToolTip(lwUIWnd UISelf);
	void lwDrawVendorItemIcon(lwUIWnd UISelf);
	void lwVendorBuyOK(lwUIWnd UISelf);
	bool lwSetPrice(lwUIWnd UISelf);
}

void lwVendor::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "RequestMyVendorInfo", lwRequestMyVendorInfo);
	def(pkState, "RequestVendorExit", lwRequestVendorExit);
	def(pkState, "RequestVendorCreate", lwRequsetVendorCreate);
	def(pkState, "RequestVendorRename", lwRequsetVendorRename);

	def(pkState, "CallVendor", lwCallVendor);
	def(pkState, "OnOverVendorItemToolTip", lwOnOverVendorItemToolTip);
	def(pkState, "DrawVendorItemIcon", lwDrawVendorItemIcon);
	def(pkState, "SetPrice", lwSetPrice);
	def(pkState, "GetPrice", lwGetPrice);
	def(pkState, "ResetSelectArticle", lwResetSelectArticle);
	def(pkState, "VendorBuyOK", lwVendorBuyOK);
	def(pkState, "OnCallVendorBuyUI", lwOnCallVendorBuyUI);
	def(pkState, "IsMyVendor", lwIsMyVendor);

	def(pkState, "CheckVendorRemainTime", lwCheckVendorRemainTime);
	def(pkState, "CheckVendorGround", lwCheckVendorGround);
}

void lwVendor::lwRequestMyVendorInfo(BM::GUID kOwnerGuid)
{
	g_kVendorMgr.RequestMyVendorInfo(kOwnerGuid);
}
void lwVendor::lwRequestVendorExit()
{
	g_kVendorMgr.RequestVendorExit();
}
void lwVendor::lwRequsetVendorCreate(std::wstring kVendorTitle)
{
	if(kVendorTitle.empty())
	{
		PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
		if(!pkPilot)
		{
			return;
		}
		std::wstring const kFormStr(TTW(799459));
		wchar_t szBuf[200] ={0,};
		wsprintfW(szBuf, kFormStr.c_str(), pkPilot->GetName().c_str() );
		kVendorTitle = szBuf;			
	}
	g_kVendorMgr.RequsetVendorCreate(kVendorTitle);
}
void lwVendor::lwRequsetVendorRename(std::wstring kVendorTitle)
{
	g_kVendorMgr.RequsetVendorRename(kVendorTitle);
}
void lwVendor::lwRequsetVendorDelete()
{
	g_kVendorMgr.RequsetVendorDelete();
}

bool lwVendor::lwIsMyVendor()
{
	return g_kVendorMgr.IsMyVendor();
}

void lwVendor::RecvMarket_Command(WORD const wPacketType, BM::Stream& rkPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_UM_ANS_MY_VENDOR_QUERY:
		{
			if( true == g_kVendorMgr.SetVendorInfo(rkPacket) )
			{
				if( g_kVendorMgr.IsEmptyArticle() )
				{
					g_kVendorMgr.Clear();
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799442, true);
				}
				if( g_kVendorMgr.IsMyVendor() )
				{//내가 만들었으면if(pkMyPlayer)
					lwCallInputBox(TTW(799441), 9, true, 15);
				}
			}
			else
			{//에러메세지 박스 써주자 TT - 799442
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799442, true);
				g_kVendorMgr.RequestVendorExit();
			}
		}break;
	case PT_M_C_ANS_VENDOR_STATE:
		{
			bool bResult;
			rkPacket.Pop(bResult);
			if(bResult)
			{
				BM::GUID kVendorGuid;
				std::wstring kVendorTitle;
				rkPacket.Pop(kVendorGuid);
				rkPacket.Pop(kVendorTitle);

				PgActor* pkActor = g_kPilotMan.FindActor(kVendorGuid);
				if(pkActor)
				{
					//노점 액션
					PgAction* pkAction = pkActor->GetAction();
					if(pkAction)
					{
						pkActor->ReserveTransitAction("a_vendor");
						pkAction->ChangeToNextActionOnNextUpdate(true);
					}
					else
					{//현재 액션이 없으면 바로 실행
						pkActor->TransitAction("a_vendor");
					}

					if(g_kVendorMgr.IsMyVendor()
					&& g_kVendorMgr.IsMyVendor_Guid(kVendorGuid) )
					{//열렸을 때, 내 상점이면 UI호출
						lwVendor::lwCallVendor();
					}
				}
			}
		}break;
	case PT_M_C_NFY_VENDOR_STATE:
		{
			bool bResult;
			rkPacket.Pop(bResult);
			if(bResult)
			{
				BM::GUID kCharGuid;
				BM::GUID kVendorGuid;
				bool bOpenVendor;
				std::wstring kVendorTitle;
				rkPacket.Pop(kCharGuid);
				rkPacket.Pop(bOpenVendor);
				rkPacket.Pop(kVendorTitle);
				rkPacket.Pop(kVendorGuid);

				PgActor* pkActor = g_kPilotMan.FindActor(kCharGuid);
				if(!pkActor)
				{
					return;
				}
				PgPilot* pkPilot = pkActor->GetPilot();
				if(!pkPilot)
				{
					return;
				}
				CUnit* pkUnit = pkPilot->GetUnit();
				if(!pkUnit)
				{
					return;
				}
				PgPlayer* pkOwnerPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if(!pkOwnerPlayer)
				{
					return;
				}
				if(bOpenVendor)
				{
					pkOwnerPlayer->OpenVendor(bOpenVendor);
					pkOwnerPlayer->VendorTitle(kVendorTitle);
					pkOwnerPlayer->VendorGuid(kVendorGuid);

					PgAction* pkAction = pkActor->GetAction();
					if(pkAction)
					{
						pkActor->ReserveTransitAction("a_vendor");
						pkAction->ChangeToNextActionOnNextUpdate(true);
					}
					else
					{//현재 액션이 없으면 바로 실행
						pkActor->TransitAction("a_vendor");
					}

					if(g_kVendorMgr.IsMyVendor()
					&& g_kVendorMgr.IsMyVendor_Guid(kVendorGuid) )
					{//열렸을 때, 내 상점이면 UI호출
						lwVendor::lwCallVendor();
					}
				}
				else
				{//노점 닫혔을 때, 내가 들어가 있으면 닫히도록
					PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
					if(!pkMyPlayer)
					{
						return;
					}
					if( pkOwnerPlayer->GetID() == pkMyPlayer->VendorGuid() )
					{
						pkMyPlayer->VendorGuid(kVendorGuid);
						lua_tinker::call<void>("CloseVendor");
						g_kVendorMgr.RequestVendorExit();
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799451, true);
					}
					pkOwnerPlayer->OpenVendor(bOpenVendor);
					pkOwnerPlayer->VendorTitle(kVendorTitle);
					pkOwnerPlayer->VendorGuid(kVendorGuid);
				}
				pkActor->UpdateName();
			}
		}break;
	case PT_M_C_UM_ANS_VENDOR_ENTER:
		{
			if( true == g_kVendorMgr.SetVendorInfo(rkPacket) )
			{
				PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if(pkMyPlayer)
				{
					BM::GUID kOwnerGuid;
					//kOwnerGuid = g_kVendorMgr.VendorInfo().ShopInfo().kOwnerId;
					kOwnerGuid = g_kVendorMgr.OwnerGuid();
					pkMyPlayer->VendorGuid(kOwnerGuid);
				}
				lwVendor::lwCallVendor();
			}	
			else
			{
				g_kVendorMgr.RequestVendorExit();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799455, true);
			}
		}break;
	case PT_M_C_UM_ANS_ARTICLE_BUY:
		{//노점이 열려있을 경우에만 체크하도록
			PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if(pkMyPlayer && BM::GUID::NullData() != pkMyPlayer->VendorGuid())
			{
				if(  0 != rkPacket.RemainSize() )
				{
					SPT_M_C_UM_ANS_ARTICLE_BUY Result;
					Result.ReadFromPacket(rkPacket);
					if( PgMarketUtil::RequestResult(Result.Result()) )
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403075, true);
					}
					//구매가 됐건 안됐건 무조건 노점 정보 갱신
					g_kVendorMgr.RequestVendorRefresh();
				}
			}
		}break;
	case PT_M_C_UM_NFY_VENDOR_REFRESH_QUERY:
		{
			BM::GUID kOwnerGuid;
			rkPacket.Pop(kOwnerGuid);
			if(kOwnerGuid.IsNotNull())
			{
				PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pkMyPlayer)
				{
					return;
				}
				if( kOwnerGuid == pkMyPlayer->VendorGuid() )
				{
					if( true == g_kVendorMgr.SetVendorInfo(rkPacket) )
					{
						PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
						if(pkMyPlayer)
						{
							BM::GUID kOwnerGuid;
							kOwnerGuid = g_kVendorMgr.OwnerGuid();
							pkMyPlayer->VendorGuid(kOwnerGuid);
						}
						lwVendor::lwCallVendor();
					}	
					else
					{//아이템이 다 팔렸을 경우, 자동으로 닫게 수정
						g_kVendorMgr.RequestVendorExit();
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799455, true);
					}
				}
			}
		}break;
	case PT_M_C_UM_ANS_VENDOR_EXIT:
		{
			BM::GUID kCharGuid;
			rkPacket.Pop(kCharGuid);

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return;
			}
			if( kCharGuid == pkPlayer->GetID() )
			{
				if( BM::GUID::NullData() != pkPlayer->VendorGuid() )
				{
					g_kVendorMgr.RequestVendorExit();
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799451, true);
				}
			}

		}break;
	}
}

void lwVendor::lwCallVendor()
{
	XUI::CXUI_Wnd* pVendorWnd = XUIMgr.Activate(L"SFRM_VENDOR_INV");
	if(!pVendorWnd)
	{
		return;
	}
	lua_tinker::call<void>("ResetVendorSelect");
	SMyShopData const kVendorInfo = g_kVendorMgr.VendorInfo().ShopInfo();
	//노점 주인
	XUI::CXUI_Wnd* pInfo =pVendorWnd->GetControl(L"FRM_INFO");
	if(pInfo)
	{
		std::wstring kCharName = kVendorInfo.kCharName;
		std::wstring const kFormStr(TTW(799437));
		wchar_t szBuf[200] ={0,};
		wsprintfW(szBuf, kFormStr.c_str(), kCharName.c_str());
		kCharName = szBuf;			
		pInfo->Text(kCharName);
	}
	//노점 간판
	XUI::CXUI_Wnd* pTitle =pVendorWnd->GetControl(L"SFRM_TITLE");
	if(pTitle)
	{
		PgActor* pkActor = g_kPilotMan.FindActor(kVendorInfo.kOwnerId);
		if(!pkActor)
		{
			return;
		}
		PgPilot* pkPilot = pkActor->GetPilot();
		if(!pkPilot)
		{
			return;
		}
		CUnit* pkUnit = pkPilot->GetUnit();
		if(!pkUnit)
		{
			return;
		}
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		if(!pkPlayer || !pkPlayer->OpenVendor() )
		{
			return;
		}
		std::wstring kTitle = pkPlayer->VendorTitle();
		std::wstring const kFormStr(TTW(799438));
		wchar_t szBuf[200] ={0,};
		wsprintfW(szBuf, kFormStr.c_str(), kTitle.c_str());
		kTitle = szBuf;			
		pTitle->Text(kTitle);
	}
	//주인일 경우, 상점 닫기만 뜨고, 손님일 경우, 구매와 닫기만 뜨도록
	XUI::CXUI_Wnd* pBuyOk = pVendorWnd->GetControl(L"BTN_BUY");
	XUI::CXUI_Wnd* pCancle = pVendorWnd->GetControl(L"BTN_CANCLE");
	XUI::CXUI_Wnd* pClose = pVendorWnd->GetControl(L"BTN_CLOSE");
	if( !pBuyOk || !pCancle || !pClose)
	{
		return;
	}
	if(g_kVendorMgr.IsMyVendor())
	{
		pBuyOk->Visible(false);
		pCancle->Visible(false);
		pClose->Visible(true);
	}
	else
	{
		pBuyOk->Visible(true);
		pCancle->Visible(true);
		pClose->Visible(false);
	}
	//노점 판매 목록
	XUI::CXUI_Wnd* pInv =pVendorWnd->GetControl(L"SFRM_INV_SHADOW"); 
	if(!pInv)
	{
		return;
	}
	int iCount = 0;
	BM::vstring kInvSlot(_T("FRM_BASKET_ICON"));
	for(int iSlot = 0; iSlot < MAX_VENDOR_INV_COUNT; ++iSlot)
	{//초기화
		std::wstring strInvSlot = kInvSlot + BM::vstring(iSlot);
		XUI::CXUI_Wnd* pIconWnd = pInv->GetControl(strInvSlot);
		if(!pIconWnd)
		{
			continue;
		}
		XUI::CXUI_Wnd* pIconImg = pIconWnd->GetControl(L"IMG_ICON");
		if( !pIconImg )
		{
			continue;
		}
		pIconImg->ClearCustomData();
		pIconImg->DefaultImgTexture(NULL);
		XUI::CXUI_Wnd* pIconCount = pIconImg->GetControl(L"FRM_ITEM_COUNT");
		if( !pIconCount )
		{
			return;
		}
		pIconCount->Text(L"");
	}
	kBaseItemContainer kContList = g_kVendorMgr.VendorInfo().ItemCont();
	for(kBaseItemContainer::const_iterator kIter_List = kContList.begin(); kIter_List != kContList.end(); ++kIter_List)
	{
		std::wstring strInvSlot = kInvSlot + BM::vstring(iCount);
		XUI::CXUI_Wnd* pIconWnd = pInv->GetControl(strInvSlot);
		if(!pIconWnd)
		{
			++iCount;
			continue;
		}
		PgBase_Item kItem = (*kIter_List).second.kItem;
		if( !kItem.IsEmpty() )
		{
			SetUIIconToBaseItem(pIconWnd, L"IMG_ICON", kItem);

			BM::Stream kPacket;
			kItem.WriteToPacket(kPacket);
			pIconWnd->SetCustomData(kPacket.Data());
		}
		++iCount;
	}
	//다 뜨면 플레이어 입력 lock
	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, true));
	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, false));
}

void lwVendor::SetUIIconToBaseItem(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, PgBase_Item const& kItem)
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

void lwVendor::lwOnOverVendorItemToolTip(lwUIWnd UISelf)
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

void lwVendor::lwDrawVendorItemIcon(lwUIWnd UISelf)
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

bool lwVendor::lwSetPrice(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pItemSlot = dynamic_cast<XUI::CXUI_Wnd*>(UISelf.GetSelf());
	if( !pItemSlot || !pItemSlot->GetCustomDataSize() )
	{ 
		g_kVendorMgr.ResetSelectArticle();
		return false; 
	}
	PgBase_Item kItem;
	{
		BM::Stream kPacket;
		pItemSlot->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		kItem.ReadFromPacket(kPacket);
	}
	return g_kVendorMgr.SetPrice(kItem);
}
__int64 lwVendor::lwGetPrice(bool bIsCash)
{
	return g_kVendorMgr.GetPrice(bIsCash);
}

void lwVendor::lwResetSelectArticle()
{
	g_kVendorMgr.ResetSelectArticle();
}

void lwVendor::lwOnCallVendorBuyUI()
{
	g_kVendorMgr.OnCallVendorBuyUI();
}

void lwVendor::RequestBuySelectItem(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iItemCount)
{
	if( true == g_kVendorMgr.SelectArticleInfo().kItem.IsEmpty() )
	{//선택한 아이템 없으면 에러메세지 출력
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799446, true);
		return;
	}
	SPT_C_M_UM_REQ_ARTICLE_BUY	kData;
	kData.MarketGuid(kShopGuid);
	kData.ArticleGuid(kItemGuid);
	kData.BuyNum(iItemCount);

	BM::Stream	kPacket;
	kData.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket);
}


void lwVendor::CallVendorBuyUI(BM::GUID const& kShopGuid, BM::GUID const& kItemGuid, int const iBuyCount)
{
	if( true == g_kVendorMgr.SelectArticleInfo().kItem.IsEmpty() )
	{//선택한 아이템 없으면 에러메세지 출력
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799446, true);
		return;
	}

	XUI::CXUI_Wnd* pBuyUI = XUIMgr.Call(L"SFRM_VENDOR_BUY");
	if( !pBuyUI )
	{
		return;
	}

	pBuyUI->OwnerGuid(kShopGuid);
	pBuyUI->SetCustomData(&kItemGuid, sizeof(kItemGuid));

	SMarketItemInfoBase const kItemInfo = g_kVendorMgr.SelectArticleInfo();
	bool bIsNonCashItem = true;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_INDONESIA:
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
		lwVendor::SetUIIconToBaseItem(pBuyUI, L"IMG_ICON", kItemInfo.kItem);

		//step 2. 선택 아이템 가격 표시
		__int64 iCost = kItemInfo.i64CostValue;
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
		if( pItemDef )
		{
			if( pItemDef->IsAmountItem() )
			{
				iCost *= iBuyCount;
			}
		}
		lwVendor::SetUITextToItemCost(pBuyUI, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.eCostType, iCost);
	}
	else
	{
		bool const bIsCashToMoney = kItemInfo.kItem.ItemNo() == MARKET_ARTICLE_CASHNO;
		//step 1. 선택 아이템 표시
		lwVendor::SetUIIconToBaseItem(pBuyUI, L"IMG_ICON", kItemInfo.kItem);
		//step 2. 선택 아이템 가격 표시
		lwVendor::SetUITextToItemCost(pBuyUI, L"FRM_ITEM_MONEY1", L"FRM_ITEM_MONEY2", (ERegCostType)kItemInfo.eCostType, kItemInfo.i64CostValue);
	}

	XUI::CXUI_Wnd* pBTNBuy = pBuyUI->GetControl(L"BTN_BUY");
	if( pBTNBuy )
	{
		pBTNBuy->SetCustomData(&iBuyCount, sizeof(iBuyCount));
	}
}

void lwVendor::lwVendorBuyOK(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	int iBuyCount = 0;
	if( pSelf->GetCustomDataSize() )
	{
		pSelf->GetCustomData(&iBuyCount, sizeof(iBuyCount));
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent || pParent->OwnerGuid().IsNull() || !pParent->GetCustomDataSize() )
	{
		return;
	}

	BM::GUID kGuid;
	pParent->GetCustomData(&kGuid, sizeof(kGuid));

	SMarketItemInfoBase const kItemInfo = g_kVendorMgr.SelectArticleInfo();
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	__int64 kHaveCost = 0;
	switch( kItemInfo.eCostType )
	{
	case RCT_GOLD:{	kHaveCost = pPlayer->GetAbil64(AT_MONEY);	}break;
	case RCT_CASH:{ kHaveCost = pPlayer->GetAbil64(AT_CASH);	}break;
	}

	__int64 iItemCost = kItemInfo.i64CostValue;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.kItem.ItemNo());
	if( pItemDef )
	{
		if( pItemDef->IsAmountItem() )
		{
			iItemCost = iItemCost * iBuyCount;
		}
	}
	else
	{//아이템을 찾을 수 없습니다.
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
		return;
	}

	if( iItemCost > kHaveCost )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403078 + kItemInfo.eCostType, true);
		return;
	}

	if( !bExistEmptyInv(kItemInfo.kItem.ItemNo()) )
	{//인벤토리가 가득 찼습니다.
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 20023, true);
		return;
	}

	lwVendor::RequestBuySelectItem(pParent->OwnerGuid(), kGuid, kItemInfo.kItem.Count());
}


void lwVendor::SetUITextToItemCost(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kGold, std::wstring const& kCash, ERegCostType const Type, __int64 const CostValue)
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
			lwVendor::SetUIText(pCost1, L"FRM_GOLD", BM::vstring(i64Gold));
			int iSilver = (int)((CostValue % 10000) / 100);
			lwVendor::SetUIText(pCost1, L"FRM_SILVER", BM::vstring(iSilver));
			int iCopper = (int)(CostValue % 100);
			lwVendor::SetUIText(pCost1, L"FRM_COPPER", BM::vstring(iCopper));
		} break;
	case RCT_CASH:
		{
			pCost1 = pItemWnd->GetControl(kCash);
			pCost2 = pItemWnd->GetControl(kGold);
			if( !pCost1 || !pCost2 )
			{
				return;
			}
			lwVendor::SetUIText(pItemWnd, kCash, BM::vstring(CostValue));
		} break;
	default:
		{
			return;
		}
	}
	pCost1->Visible(true);
	pCost2->Visible(!pCost1->Visible());
}

void lwVendor::SetUIText(XUI::CXUI_Wnd* pItemWnd, std::wstring const& kChild, std::wstring const& kText, bool bIsReduce)
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

bool lwVendor::lwCheckVendorRemainTime()
{
	if( g_kVendorMgr.IsMyVendor() )
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{ 
			return false; 
		}
		PgInventory* pInv = pPlayer->GetInven();
		if(!pInv)
		{ 
			return false;
		}
		PgBase_Item kItem;
		ContHaveItemNoCount kContVendorItem;
		if(S_OK != pInv->GetItems( UICT_VENDOR, kContVendorItem, true) )
		{
			return false;
		}
	}
	return true;
}

bool lwVendor::lwCheckVendorGround()
{
	if(!g_pkWorld)
	{
		return false;
	}
	bool bResult = false;
	
	if( g_pkWorld->IsEqualAttr(GATTR_VILLAGE) )
	{
		bResult = true;
	}
	else if( g_pkWorld->IsEqualAttr(GATTR_DEFAULT) )
	{
		bResult = true;
	}
	return bResult;
}