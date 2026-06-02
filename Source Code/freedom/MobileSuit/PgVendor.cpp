#include "StdAfx.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "PgPilotMan.h"
#include "lohengrin/packetstruct.h"
#include "PgMarket.h"
#include "lwUI.h"
#include "lwMarket.h"
#include "Pg2DString.h"
#include "PgUIScene.h"
#include "PgVendor.h"
#include "lwVendor.h"
#include "PgUICalculator.h"
#include "PgAction.h"


PgVendor::PgVendor(void)
{
	m_kSelectArticleInfo.eCostType = RCT_GOLD;
	m_kSelectArticleInfo.Guid.Clear();
	m_kSelectArticleInfo.i64CostValue = 0;
	m_kSelectArticleInfo.kItem.Clear();
	m_kSelectArticleInfo.kItemType = UMAT_ITEM;
	m_kSelectArticleInfo.SellMoney = 0;
}
PgVendor::~PgVendor(void)
{
	m_kSelectArticleInfo.eCostType = RCT_GOLD;
	m_kSelectArticleInfo.Guid.Clear();
	m_kSelectArticleInfo.i64CostValue = 0;
	m_kSelectArticleInfo.kItem.Clear();
	m_kSelectArticleInfo.kItemType = UMAT_ITEM;
	m_kSelectArticleInfo.SellMoney = 0;
	m_kVendorInfo.Clear();
	m_kOwnerGuid.Clear();
}

void PgVendor::Clear()
{
	ResetSelectArticle();
	m_kVendorInfo.Clear();
	m_kOwnerGuid.Clear();
}

void PgVendor::ResetSelectArticle()
{
	m_kSelectArticleInfo.eCostType = RCT_GOLD;
	m_kSelectArticleInfo.Guid.Clear();
	m_kSelectArticleInfo.i64CostValue = 0;
	m_kSelectArticleInfo.kItem.Clear();
	m_kSelectArticleInfo.kItemType = UMAT_ITEM;
	m_kSelectArticleInfo.SellMoney = 0;
}

void PgVendor::RequestMyVendorInfo(BM::GUID kOwnerGuid)
{
	SPT_C_M_UM_REQ_MY_VENDOR_QUERY Data;
	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);
	kPacket.Push(kOwnerGuid);
	NETWORK_SEND(kPacket);
}
void PgVendor::RequsetVendorCreate(std::wstring kVendorTitle)
{	
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer || pPlayer->HaveParty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799444, true);
		return;
	}
	BM::Stream	kPacket(PT_C_M_REQ_VENDOR_CREATE);
	kPacket.Push( pPlayer->GetID() );
	kPacket.Push( kVendorTitle );
	NETWORK_SEND(kPacket);
}
void PgVendor::RequsetVendorRename(std::wstring kVendorTitle)
{	
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer || pPlayer->HaveParty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799444, true);
		return;
	}
	BM::Stream	kPacket(PT_C_M_REQ_VENDOR_RENAME);
	BM::GUID kOwnerGuid;
	if(true == g_kPilotMan.GetPlayerPilotGuid(kOwnerGuid))
	{
		kPacket.Push( kOwnerGuid );
		kPacket.Push( kVendorTitle );
		NETWORK_SEND(kPacket);
	}
}
void PgVendor::RequsetVendorDelete()
{
	BM::GUID kOwnerGuid;
	g_kPilotMan.GetPlayerPilotGuid(kOwnerGuid);
	BM::Stream	kPacket(PT_C_M_REQ_VENDOR_DELETE);
	kPacket.Push(kOwnerGuid);
	NETWORK_SEND(kPacket);

	lua_tinker::call<void>("CloseVendor");
}

void PgVendor::RequestVendorEnter()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer || pPlayer->HaveParty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799444, true);
		return;
	}
	BM::Stream	kPacket(PT_C_M_UM_REQ_VENDOR_ENTER);
 	kPacket.Push( m_kOwnerGuid );
	NETWORK_SEND(kPacket);
}

void PgVendor::RequestVendorExit()
{
	PgActor* pActor = g_kPilotMan.GetPlayerActor();
	if(!pActor)
	{
		return;
	}
	if( pActor->GetGuid() == m_kVendorInfo.ShopInfo().kOwnerId)
	{
		RequsetVendorDelete();
	}

	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, false));
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_OpenMarket, true));

	PgAction* pkAction = pActor->GetAction();
	if(pkAction)
	{
		pkAction->ChangeToNextActionOnNextUpdate(true);
	}
	BM::Stream	kPacket(PT_C_M_UM_REQ_VENDOR_EXIT);
	NETWORK_SEND(kPacket);

	Clear();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		pkPlayer->OpenVendor(false);
		pkPlayer->VendorGuid(BM::GUID::NullData());
	}

	lua_tinker::call<void>("CloseVendor");
}

void PgVendor::RequestVendorRefresh()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer || pPlayer->HaveParty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799444, true);
		return;
	}
	BM::Stream	kPacket(PT_C_M_UM_REQ_VENDOR_REFRESH_QUERY);
 	kPacket.Push( m_kOwnerGuid );
	NETWORK_SEND(kPacket);
}

bool PgVendor::SetVendorInfo(BM::Stream rkPacket)
{
	SPT_M_C_UM_ANS_MY_MARKET_QUERY Data;
	Data.ReadFromPacket(rkPacket);
	if( UMR_SUCCESS == Data.Result() )
	{
		Clear();
		SMyShopData	ShopInfo;
		//상점정보 받기
		ShopInfo.kOwnerId = Data.OwnerGuId();
		ShopInfo.kCharName = Data.CharName();
		ShopInfo.kMarketName = Data.MarketName();
		ShopInfo.iGrade = Data.MarketGrade();
		ShopInfo.iOnTime = Data.OnlineTime();
		ShopInfo.iOffTime = Data.OfflineTime();
		ShopInfo.iHP = Data.MarketHP();

		m_kOwnerGuid = Data.OwnerGuId();
		m_kVendorInfo.SetShopInfo(ShopInfo);
		m_kVendorInfo.ConvertItemList(Data.ContArticle().kCont);
		//선택 품목 정보 초기화
		ResetSelectArticle();

		return true;
	}
	return false;
}

bool PgVendor::IsMyVendor()
{
	BM::GUID kUserGuid;
	g_kPilotMan.GetPlayerPilotGuid(kUserGuid);
	return ( kUserGuid == m_kOwnerGuid );
}

bool PgVendor::IsMyVendor_Guid(BM::GUID const &rkOwnerGuid)
{
	BM::GUID kUserGuid;
	g_kPilotMan.GetPlayerPilotGuid(kUserGuid);

	return (m_kOwnerGuid == rkOwnerGuid
		 && kUserGuid == rkOwnerGuid);
}

__int64 PgVendor::GetPrice(bool bIsCash)
{
	switch(m_kSelectArticleInfo.eCostType)
	{
	case RCT_GOLD:
		{
			if(!bIsCash)
			{
				return m_kSelectArticleInfo.i64CostValue;
			}
		}break;
	case RCT_CASH:
		{
			if(bIsCash)
			{
				return m_kSelectArticleInfo.i64CostValue;
			}
		}break;
	}
	return 0;
}

bool PgVendor::SetPrice(PgBase_Item const& kItem)
{
	if( g_kVendorMgr.SelectArticleInfo().Guid == kItem.Guid() )
	{//체크된 아이템 다시 클릭한 상황 = 체크 해제되는 상황
		g_kVendorMgr.ResetSelectArticle();
		return false; 
	}
	bool const bIsCashToMoney = kItem.ItemNo() == MARKET_ARTICLE_CASHNO;
	SMarketItemInfoBase kArticleInfo;
	SMyShopInfo kShopInfo = g_kVendorMgr.VendorInfo();
	if(false == kShopInfo.FindItem(kItem.Guid(), kArticleInfo) )
	{
		return false;
	}
	g_kVendorMgr.SelectArticleInfo(kArticleInfo);
	return true;
}

void PgVendor::OnCallVendorBuyUI()
{
	if( true == m_kSelectArticleInfo.kItem.IsEmpty() )
	{//선택한 아이템 없으면 에러메세지 출력
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799446, true);
		return;
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kSelectArticleInfo.kItem.ItemNo());
	if( pItemDef )
	{		
		if( pItemDef->IsAmountItem() 
			&& 1 < m_kSelectArticleInfo.kItem.Count() 
			&& MARKET_ARTICLE_CASHNO != m_kSelectArticleInfo.kItem.ItemNo()
			&& MARKET_ARTICLE_MONEYNO != m_kSelectArticleInfo.kItem.ItemNo() )
		{
			SCalcInfo kInfo;
			kInfo.eCallType = CCT_VENDOR_BUY;
			kInfo.iBasePrice = m_kSelectArticleInfo.i64CostValue;
			kInfo.iItemNo = m_kSelectArticleInfo.kItem.ItemNo();
			kInfo.iMaxValue = m_kSelectArticleInfo.kItem.Count();
			kInfo.kGuid = m_kSelectArticleInfo.Guid;
			kInfo.kGuid1 = m_kVendorInfo.ShopInfo().kOwnerId;
			kInfo.cCostType = m_kSelectArticleInfo.eCostType;
			CallCalculator(kInfo);
			return;
		}
	}
	lwVendor::CallVendorBuyUI(m_kVendorInfo.ShopInfo().kOwnerId, m_kSelectArticleInfo.Guid, m_kSelectArticleInfo.kItem.Count());
}

bool PgVendor::IsEmptyArticle()
{
	return m_kVendorInfo.ItemCont().empty();
}