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

int const GOOD_SHOP_OPEN_ITEMNO = 99600010;	//좋은상점개설증
int const HISTORY_SHOP_OPEN_ITEMNO = 99600020;	//역사깊은상점개설증

bool CheckMarketRegItem(PgPlayer const* pkPlayer, PgBase_Item const& kItem, CItemDef const* pDef, bool const bMsg)
{
	if( kItem.EnchantInfo().IsAttached() )
	{
		if(bMsg)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403097, true);
		}
		return false;
	}

	if( kItem.EnchantInfo().IsBinding() )
	{
		return false;
	}

	if( NULL == pDef )
	{
		return false;
	}

	if( pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) == UICT_EXPCARD )
	{
		if( kItem.Guid() == pkPlayer->GetInven()->ExpCardItem() )
		{//현제 토글중인것은 팔 수 없다.
			if(bMsg)
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50763, true);
			}
			return false;
		}

		SExpCard	kExpCard;
		if( kItem.Get(kExpCard) )
		{
			if( 1 != (kExpCard.CurExp() / kExpCard.MaxExp()) )
			{//경험치가 꽉찬거만 팔 수 있다.
				if(bMsg)
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50764, true);
				}
				return false;
			}
		}
	}

	if( pDef->IsType(ITEM_TYPE_QUEST) )
	{
		if(bMsg)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403098, true);
		}
		return false;
	}

	int	const iCantAbil = pDef->GetAbil(AT_ATTRIBUTE);
	if(iCantAbil && ICMET_Cant_Auction == (iCantAbil & ICMET_Cant_Auction))
	{
		if(bMsg)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403099, true);
		}
		return false;
	}
	return true;
}	

PgMarket::PgMarket(void)
	: m_kIsSendOK(false)
	, m_ePrevMarketGrade(MG_NORMAL)
	, m_eCurrentMarketGrade(MG_NORMAL)
{
}

PgMarket::~PgMarket(void)
{
}

int PgMarket::ClearOpenMarket()
{
	m_kMyShopInfo.Clear();
	m_kSoldInfo.Clear();
	return 0;
}

__int64 PgMarket::GetClassLimit(int const TypeID)
{
	__int64	ClassLimit = 0;

	switch( TypeID )
	{
	case UCLASS_FIGHTER:
		{
			ClassLimit = UCLIMIT_MARKET_FIGHTER;
		}break;
	case UCLASS_MAGICIAN:
		{
			ClassLimit = UCLIMIT_MARKET_MAGICIAN;
		}break;
	case UCLASS_ARCHER:
		{
			ClassLimit = UCLIMIT_MARKET_ARCHER;
		}break;
	case UCLASS_THIEF:
		{
			ClassLimit = UCLIMIT_MARKET_THIEF;
		}break;
	case UCLASS_SHAMAN:
		{
			ClassLimit = UCLIMIT_MARKET_SHAMAN;
		}break;
	case UCLASS_DOUBLE_FIGHTER:
		{
			ClassLimit = UCLIMIT_MARKET_DOUBLE_FIGHTER;
		}break;
	}

	return ClassLimit;
}

int PgMarket::GetMaxItemSlot(E_MARKET_GRADE const Type)
{
	switch(Type)
	{
	case MG_GOOD:
		{
			return RSS_TYPE2;
		}break;
	case MG_HISTORY:
		{
			return RSS_TYPE3;		
		}break;
	}
	return RSS_TYPE1;
}

int PgMarket::GetMaxOnlineOpenTime(E_MARKET_GRADE const Type)
{
	if( m_kShopState == MS_CLOSE)
	{
		switch(Type)
		{
		case MG_GOOD:	
		case MG_HISTORY:
			{
				return RON_TYPE2;	
			}break;
		}
		return RON_TYPE1;
	}

	return (int)m_fOnlineOpenTime;
}

int PgMarket::GetMaxOfflineOpenTime(E_MARKET_GRADE const Type)
{
	if( m_kShopState == MS_CLOSE)
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		int iItemNo = 0;
		switch(Type)
		{
		case MG_GOOD:
			{
				iItemNo = GOOD_SHOP_OPEN_ITEMNO;
			}break;
		case MG_HISTORY:
			{
				iItemNo = HISTORY_SHOP_OPEN_ITEMNO;
			}break;
		}
		CItemDef const* pDef = kItemDefMgr.GetDef(iItemNo);
		return pDef ? pDef->GetAbil(AT_MARKET_OFFLINE_TIME) : 0;
	}

	return (int)m_fOfflineOpenTime;
}

bool PgMarket::UpdateTime()
{
	if(!g_pkWorld)
	{
		return false;
	}
	float NowTime = g_pkWorld->GetAccumTime();
	float TickTime = NowTime - m_fPrevTime;
	m_fOnlineOpenTime -= TickTime;
	m_fOfflineOpenTime -= TickTime;
	m_fPrevTime += TickTime;

	return (m_kOnTimeUpdate < NowTime)?(true):(false);
}

void PgMarket::ResultRegShopItemInfo(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_MY_MARKET_QUERY	Data;
	Data.ReadFromPacket(kPacket);

	SMyShopData	ShopInfo;
	if( Data.Result() != UMR_NOT_FOUND_MARKET )
	{
		m_kShopState = (E_MARKET_STATE)Data.State();

		ShopInfo.kOwnerId = Data.OwnerGuId();
		ShopInfo.kCharName = Data.CharName();
		ShopInfo.kMarketName = Data.MarketName();
		ShopInfo.iGrade = Data.MarketGrade();		
//		ShopInfo.i64OpenTime = Data.OpenTime();
		ShopInfo.iOnTime = Data.OnlineTime();
		ShopInfo.iOffTime = Data.OfflineTime();
		ShopInfo.iHP = Data.MarketHP();
		m_kMyShopInfo.SetShopInfo(ShopInfo);
		m_kMyShopInfo.ConvertItemList(Data.ContArticle().kCont);
		//m_kSoldInfo.ConvertSoldList(Data.ContDealing().kCont, Data.CloseTime());
		SetCurrentMarketGrade(ShopInfo.iGrade);
	}
	else
	{
		m_kShopState = MS_CLOSE;
		m_kMyShopInfo.SetShopInfo(ShopInfo);

		lwMarket::UpdateRegList();
	}

	m_kContDealing.kCont.clear();

	//lwMarket::UpdateRegList();
}

void PgMarket::ResultRegShopItemDealingInfo(BM::Stream& kPacket)
{
	EUserMarketResult Result;	

	kPacket.Pop(Result);	
	m_kContDealing.ReadFromPacket(kPacket);			

	switch( Result )
	{
	case UMR_DEALING_END:
		{
			__int64 CloseTime = 0;
			int iDealingMax = 0;

			kPacket.Pop(CloseTime);
			kPacket.Pop(iDealingMax);			

			DealingMax(iDealingMax);
			m_kSoldInfo.ConvertSoldList(m_kContDealing.kCont, CloseTime);

			lwMarket::UpdateRegList();
		}break;
	default:
		{
		}break;
	}		
}

int PgMarket::RequestAddRegItem(SARTICLEINFO const& kRegInfo, SMarketItemInfoBase const& kItemInfo)
{
	if( m_kShopState == MS_OPEN)
	{
		return MET_REG_SHOP_STARTED;
	}

	if( m_kShopState == MS_EDIT)
	{
		if( m_kMyShopInfo.ItemCont().size() >= (size_t)GetMaxItemSlot((E_MARKET_GRADE)m_kMyShopInfo.ShopInfo().iGrade) )
		{
			return MET_REG_MAX_SLOT;
		}
	}
	else
	{
		if( m_kMyShopInfo.ItemCont().size() >= (size_t)GetMaxItemSlot(lwMarket::GetShopGrade()) )
		{
			return MET_REG_MAX_SLOT;
		}
	}

	SMarketItemInfoBase	Info;
	if( m_kMyShopInfo.FindItem(kItemInfo.Guid, Info) )
	{
		return MET_REG_ALREADY_ITEM;
	}

	if( SoldInfo().Size() > DealingMax() )
	{
		return MET_DEALING_MAX_COUNT;
	}

	if( m_kShopState == MS_EDIT )
	{
		//	이때는 바로 패킷을 보내버린다.
		SPT_C_M_UM_REQ_ARTICLE_REG	Data;
		Data.Pos(kRegInfo.Pos());
		Data.CostType(kRegInfo.CostType());
		Data.ArticleCost(kRegInfo.ArticleCost());
		Data.ArticleType(kRegInfo.ArticleType());
		Data.SellMoney(kRegInfo.SellMoney());

		BM::Stream	kPacket;
		Data.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
		IsSendOK(true);
	}
	else
	{
		if( !m_kMyShopInfo.AddRegItem(kItemInfo.Guid, kRegInfo) )
		{
			return MET_REG_ALREADY_ITEM;
		}
		if( !m_kMyShopInfo.AddItem(kItemInfo) )
		{
			return MET_REG_ALREADY_ITEM;
		}

		lwMarket::UpdateRegList();
	}

	return MET_SUCCESS;
}

int PgMarket::RequestDelRegItem(BM::GUID const& Guid)
{
	if( m_kShopState == MS_OPEN)
	{
		return MET_REG_SHOP_STARTED;
	}

	if( !m_kMyShopInfo.ItemCont().size() && !m_kMyShopInfo.TempRegList().size() )
	{
		return MET_REG_LIST_NOTHING;
	}

	if( m_kShopState == MS_EDIT )
	{
		SMarketItemInfoBase Info;
		if( !m_kMyShopInfo.FindItem(Guid, Info) )
		{
			return MET_NOT_EXIST_RECORD;	
		}

		if( !bExistEmptyInv(Info.kItem.ItemNo()) )
		{
			return MET_EXIST_NO_INV_POS;
		}

		// 여기서 패킷을 보내버린다
		SPT_C_M_UM_REQ_ARTICLE_DEREG	Data;
		Data.ArticleGuid(Guid);

		BM::Stream	kPacket;
		Data.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)

		TempDeleteItem(Guid);
		IsSendOK(true);
	}
	else
	{
		if( !m_kMyShopInfo.DelItem(Guid) )
		{
			return MET_NOT_EXIST_RECORD;
		}
		lwMarket::UpdateRegList();
	}

	return MET_SUCCESS;
}

void PgMarket::ResultCloseMyShop(BM::Stream& kPacket)
{
	SPT_M_C_ANS_MARKET_CLOSE kData;
	kData.ReadFromPacket(kPacket);
	if( RequestResult(kData.Error()) )
	{
		m_kMyShopInfo.Clear();
		m_kShopState = MS_CLOSE;
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403205, true);
	}
}

bool PgMarket::DeleteTempItem(EMarketTabState const Type)
{
	switch(Type)
	{
	case ETS_REG:
		{
			if( m_kMyShopInfo.DelItem(m_kTempDeleteItem) )
			{
				lwMarket::UpdateRegList();
			}
		}break;
	case ETS_STOCK:
		{
			if(	m_kSoldInfo.DelSoldItem(m_kTempDeleteItem) )
			{
				lwMarket::DrawSoldPage(m_kSoldPage);
			}
		}break;
	}
	return true;
}

int PgMarket::RequestOpenShop(std::wstring const& ShopName, E_MARKET_GRADE const Type)
{
	//step 1. 상점종류에 따른 개설증서 찾기
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return MET_FAIL;
	}

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return MET_FAIL; 
	}

	SItemPos	ShopOpenContractPos;
	switch(Type)
	{
	case MG_GOOD:
		{
			// 좋은 상점 개설 증서 번호 (99600020)
			if( E_FAIL == pkInven->GetFirstItem(99600010, ShopOpenContractPos) )
			{
				return MET_REG_CONTRACT_INS;
			}
		}break;
	case MG_HISTORY:
		{
			// 역사 상점 개설 증서 번호 (99600020)
			if( E_FAIL == pkInven->GetFirstItem(99600020, ShopOpenContractPos) )
			{
				return MET_REG_CONTRACT_INS;
			}
		}break;
	}

	if( !m_kMyShopInfo.TempRegList().size() )
	{
		return MET_REG_LIST_NOTHING;
	}

	SPT_C_M_UM_REQ_MARKET_OPEN	Data;

	Data.MarketName(ShopName);
	Data.MarketItemPos(ShopOpenContractPos);

	BM::Stream	kPacket;
	Data.WriteToPacket(kPacket);

	SMyShopData	ShopInfo;
	ShopInfo.kMarketName = ShopName;
	ShopInfo.iOnTime = GetMaxOnlineOpenTime(Type);
	ShopInfo.iOffTime = GetMaxOfflineOpenTime(Type);
	ShopInfo.iHP = 0;
	ShopInfo.iGrade = Type;

	m_kMyShopInfo.SetShopInfo(ShopInfo);	

	// 상점 열기 확인 창을 호출한다
	lwMarket::CallMarketOpenConfirm(kPacket);

	return MET_SUCCESS;
}

int PgMarket::RequestMinimumCost(SItemPos kPos, BM::GUID const& Guid)
{
	SPT_C_M_UM_REQ_MINIMUM_COST_QUERY	kData;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return MET_FAIL;
	}

	PgBase_Item kItem;
	if( S_OK == pkPlayer->GetInven()->GetItem(kPos, kItem) )
	{
		if( Guid == kItem.Guid() )
		{
			kData.ItemNo(kItem.ItemNo());
		}
		else
		{
			return MET_FAIL;
		}
	}

	BM::Stream kPacket;
	kData.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)

	return MET_SUCCESS;
}

void PgMarket::ResultAddRegItem(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_ARTICLE_REG	Data;
	Data.ReadFromPacket(kPacket);
	if( !RequestResult(Data.Result()) )
	{
		return;
	}

	SMarketItemInfoBase	ItemInfo;
	ItemInfo.Guid = Data.Article().kItemGuId;
	ItemInfo.eCostType = (ERegCostType)Data.Article().kCostType;
	ItemInfo.i64CostValue = Data.Article().kArticleCost;
	ItemInfo.kItem = Data.Article().kItem;

	if( m_kMyShopInfo.AddItem(ItemInfo) )
	{
		lwMarket::UpdateRegList();
	}

	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403071, true);
}

void PgMarket::ResultMinimumCost(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_MINIMUM_COST_QUERY	Data;
	Data.ReadFromPacket(kPacket);

	int iGold = (int)(Data.Result() / 10000);
	int iSilver = (int)((Data.Result() % 10000) / 100);
	int iCopper = (int)(Data.Result() % 100);

	lwMarket::SetMinimumCost(iGold, iSilver, iCopper);
}

void PgMarket::ResultUpdateOpenTime(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_USE_MARKET_MODIFY_ITEM	Data;
	Data.ReadFromPacket(kPacket);
	if( !RequestResult(Data.Error()) )
	{
		return;
	}
	SMyShopData	ShopData = m_kMyShopInfo.ShopInfo();

	if( ShopData.iGrade != Data.MarketGrade() )
	{
		ShopData.iOnTime = Data.OnlineTime();
		ShopData.iOffTime = Data.OfflineTime();		
		ShopData.iGrade = Data.MarketGrade();
		if( ShopData.iGrade == MG_NORMAL )
		{
			ShopData.iHP = 0;
		}
	}
	else
	{
		if( ShopData.iGrade == MG_NORMAL )
		{
			ShopData.iOnTime = Data.OnlineTime();
			ShopData.iOffTime = Data.OfflineTime();
		}
		else
		{
			ShopData.iOnTime += Data.OnlineTime();
			ShopData.iOffTime += Data.OfflineTime();
		}
		ShopData.iHP += Data.MarketHP();
	}
	ShopData.kMarketName = Data.MarketName();
	m_kMyShopInfo.SetShopInfo(ShopData);
}

void PgMarket::FlowTime(float const TickTime)
{
	m_kMyShopInfo.FlowTime((__int64)TickTime);
}

void PgMarket::ResultUpdateShopState(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_MARKET_MODIFY_STATE	Data;
	Data.ReadFromPacket(kPacket);
	if( !RequestResult(Data.Error()) )
	{
		return;
	}

	m_kShopState = (E_MARKET_STATE)Data.MarketState();
	lwMarket::lwUpdataRegAction();
	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403068 + m_kShopState, true);
}

bool PgMarket::GetSoldPageItem(size_t PageNum, kSoldList& List)
{		
	if( m_kSoldInfo.GetSoldPageItem(PageNum, List) )
	{
		SoldPage(PageNum);
		return true;
	}
	return false;
}

void PgMarket::TempRegItemInfo(XUI::CXUI_Wnd* pSelf, SItemPos const& Pos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}

	PgBase_Item kItem;
	if(S_OK == pkPlayer->GetInven()->GetItem(Pos, kItem))
	{
		if( pSelf )
		{
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
			case LOCAL_MGR::NC_JAPAN:
				{
				}break;
			default:
				{
					if( kItem.EnchantInfo().IsTimeLimit() || true == CheckIsCashItem(kItem) )
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403093, true);
						return;
					}
				}break;
			}
*/
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( NULL == pDef )
			{
				return;
			}

			if(false == CheckMarketRegItem(pkPlayer, kItem, pDef, true))
			{
				return;
			}

			SMarketIconInfo	IconInfo;
			IconInfo.dwItemNum = kItem.ItemNo();
			IconInfo.iCnt = kItem.Count();
			IconInfo.cItemGroup = Pos.x;
			IconInfo.cInvPos = Pos.y;
			pSelf->SetCustomData(&IconInfo, sizeof(IconInfo));
			pSelf->Parent()->SetCustomData(&(kItem.Guid()),sizeof(kItem.Guid()));

			std::wstring const *pkItemName = NULL;

			if(GetDefString(pDef->NameNo(), pkItemName))
			{
				lwMarketUtil::SetUIText(pSelf->Parent(), L"FRM_REG_ITEM_NAME", (*pkItemName), true);
			}

			if(pSelf->Parent())
			{//물품 등록시 판매가격에 자동으로 포커스 지정
				XUI::CXUI_CheckButton* pkCashBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pSelf->Parent()->GetControl(L"CBTN_CASH_SELL"));
				CXUI_Wnd* pkFocus = NULL;
				
				if(pkCashBtn && pkCashBtn->Check())
				{
					pkFocus = pSelf->Parent()->GetControl(L"EDT_SELL_CASH");
				}
				else
				{
					pkFocus = pSelf->Parent()->GetControl(L"EDT_SELL_GOLD");
				}

				if(pkFocus)
				{
					pkFocus->SetEditFocus(true);
				}
			}
		}
	}
}

bool PgMarket::FindStockTabItem(BM::GUID const& Guid, SMarketItemInfoBase& Info)
{
	return m_kSoldInfo.FindItem(Guid, Info);
}

bool PgMarket::FindRegTabItem(BM::GUID const& Guid, SMarketItemInfoBase& Info)
{
	return m_kMyShopInfo.FindItem(Guid, Info);
}

bool PgMarket::RequestResult(EUserMarketResult Result)
{
	switch(Result)
	{
	case UMR_SUCCESS:
		{
			return true;
		}
	default:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403100 + Result, true);
		}break;
	}
	return	false;
}

void PgMarket::ResultDealingRead(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_DEALINGS_READ Data;
	Data.ReadFromPacket(kPacket);
	if( RequestResult(Data.Result()) )
	{
		DeleteTempItem(ETS_STOCK);
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403074, true);
	}
}

void PgMarket::ResultOpenShop(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_MARKET_OPEN	Data;
	Data.ReadFromPacket(kPacket);
	if( RequestResult(Data.Error()) )
	{
		m_kShopState = MS_OPEN;
		lwMarket::lwUpdataRegAction();
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403069, true);	
	}
}

void PgMarket::ResultDelRegItem(BM::Stream& kPacket)
{
	SPT_M_C_UM_ANS_ARTICLE_DEREG Data;
	Data.ReadFromPacket(kPacket);
	if( RequestResult(Data.Result()) )
	{
		if( ShopState() == MS_EDIT )
		{
			DeleteTempItem(ETS_REG);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403072, true);
		}
		else
		{
			DeleteTempItem(ETS_STOCK);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403073, true);
		}
	}

}

void PgMarket::ViewMessage(int Type)
{
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(_T("OK_CANCEL_BOX"), true));
	if( pMsgBox )
	{
		XUI::CXUI_Wnd *pkFrm = pMsgBox->GetControl(_T("FRM_MESSAGE"));
		if( pkFrm )
		{
			pkFrm->Text(TTW(403056 + (Type - MBT_MARKET_REQUEST_BUY)));

			XUI::CXUI_Style_String kStyleString = pkFrm->StyleText();
			POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			int const iFormSizeY = 108;
			pkFrm->TextPos( POINT2(pkFrm->TextPos().x, (iFormSizeY - kTextSize.y) / 2) );
		}
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = Type;
		Info.kOrderGuid = BM::GUID().NullData();
		
		int iExternInt = 0;
		pMsgBox->BoxInfo(Info);
		pMsgBox->SetCustomData(&iExternInt, sizeof(iExternInt));

		XUI::CXUI_Wnd *pkOK = pMsgBox->GetControl(_T("BTN_OK"));
		XUI::CXUI_Wnd *pkCancel = pMsgBox->GetControl(_T("BTN_CANCEL"));
		if( pkOK )		{ pkOK->Text(TTW(96)); }
		if( pkCancel )	{ pkCancel->Text(TTW(97)); }
	}
}

E_MARKET_GRADE PgMarket::GetPrevMarketGrade() const
{
	return m_ePrevMarketGrade;	
}

E_MARKET_GRADE PgMarket::GetCurrentMarketGrade() const
{
	return m_eCurrentMarketGrade;	
}

void PgMarket::SetCurrentMarketGrade(int const iGrade)
{
	switch(iGrade)
	{
	case MG_NORMAL:
		{
			m_ePrevMarketGrade = m_eCurrentMarketGrade;
			m_eCurrentMarketGrade = MG_NORMAL;
		}break;
	case MG_GOOD:
		{
			m_ePrevMarketGrade = m_eCurrentMarketGrade;
			m_eCurrentMarketGrade = MG_GOOD;
		}break;
	case MG_HISTORY:
		{
			m_ePrevMarketGrade = m_eCurrentMarketGrade;
			m_eCurrentMarketGrade = MG_HISTORY;
		}break;
	default:
		{
			_PgMessageBox("PgMarket::SetCurrentMarketGrade()", "Unknown Grade value");
		}break;
	}
}
void PgMarket::SetPrevMarketGrade(int const iGrade)
{
	switch(iGrade)
	{
	case MG_NORMAL:
		{			
			m_ePrevMarketGrade = MG_NORMAL;
		}break;
	case MG_GOOD:
		{			
			m_ePrevMarketGrade = MG_GOOD;
		}break;
	case MG_HISTORY:
		{			
			m_ePrevMarketGrade = MG_HISTORY;
		}break;
	default:
		{
			_PgMessageBox("PgMarket::SetPrevMarketGrade()", "Unknown Grade value");
		}break;
	}
}

void PgMarket::RevertMarketGrade()
{
	E_MARKET_GRADE eTemp = m_eCurrentMarketGrade;
	m_eCurrentMarketGrade = m_ePrevMarketGrade;
	m_ePrevMarketGrade = eTemp;
}


//---------------------------------------------------------------------------------------------------------------------------------
//	Market Shop 분리 2009. 4. 20
//---------------------------------------------------------------------------------------------------------------------------------
PgMarketShopMgr::PgMarketShopMgr()
{
	Clear();
}

PgMarketShopMgr::~PgMarketShopMgr()
{
	Clear();
}

void PgMarketShopMgr::RecvShopItem(BM::Stream& rkPacket)
{
	eUserMarketResult	kError;
	rkPacket.Pop(kError);
	if( PgMarketUtil::RequestResult(kError) )
	{
		m_kShopContainer.clear();

		CONT_MARKET_KEY_LIST::size_type kSize;
		rkPacket.Pop(kSize);
		SMARKET_KEY kMarket;
		for( CONT_MARKET_KEY_LIST::size_type i = 0; i < kSize; ++i )
		{
			kMarket.ReadFromPacket(rkPacket);
			m_kShopContainer.insert(kMarket);
		}
		TotalItem(m_kShopContainer.size());
		TotalPage(TotalItem() * 0.1f);
		TotalPage(TotalPage() + ((0 < (m_kTotalItem % 10))?(1):(0)));
	}
}

void PgMarketShopMgr::GetPageInfo(CONT_SHOP_ITEM& kList) const
{
	CONT_SHOP_ITEM::const_iterator c_iter = m_kShopContainer.begin();
	if( c_iter == m_kShopContainer.end() )
	{
		return;
	}

	//페이지 0부터 시작이다
	for(int i = 0; i < (Page() * GetPageSlotMax()); ++i)
	{
		++c_iter;
		if( c_iter == m_kShopContainer.end() )
		{
			return;
		}
	}

	for(int i = 0; i < GetPageSlotMax(); ++i)
	{
		std::pair<CONT_SHOP_ITEM::iterator, bool> Result = kList.insert(*c_iter);
		if( !Result.second )
		{
			//이건 로그로?
		}

		++c_iter;
		if( c_iter == m_kShopContainer.end() )
		{
			break;
		}
	}
}

bool PgMarketShopMgr::RecvEnterShopItem(BM::Stream& rkPacket)
{
	SPT_M_C_UM_ANS_MARKET_ARTICLE_QUERY	kResult;
	kResult.ReadFromPacket(rkPacket);
	if( !PgMarketUtil::RequestResult(kResult.Result()) )
	{
		DeleteShop(kEnterShopGuid());
		return false;
	}

	m_kEnterShopItemContainer.clear();
	CONT_USER_MARKET_ARTICLEINFO_LIST::const_iterator	c_iter = kResult.ContArticle().kCont.begin();
	while( c_iter != kResult.ContArticle().kCont.end() )
	{
		m_kEnterShopItemContainer.push_back(*c_iter);
		++c_iter;
	}
	return true;
}

bool PgMarketShopMgr::GetEnterShopItemInfo(BM::GUID const& ItemGuid, ITEM_VAL& kItem) const
{
	CONT_ENTER_SHOP_ITEM::const_iterator c_iter = m_kEnterShopItemContainer.begin();
	while( c_iter != m_kEnterShopItemContainer.end() )
	{
		if( c_iter->kItemGuId == ItemGuid )
		{
			kItem = (*c_iter);
			return true;
		}
		++c_iter;
	}
	return false;
}

bool PgMarketShopMgr::RecvBuyEnterShopItem(BM::Stream& rkPacket)
{
	SPT_M_C_UM_ANS_ARTICLE_BUY Result;
	Result.ReadFromPacket(rkPacket);
	if( PgMarketUtil::RequestResult(Result.Result()) )
	{
		if( !Result.LeftItemNum() )
		{
			DeleteEnterShopItem(Result.ArticleGuId());
		}
		else
		{
			CONT_ENTER_SHOP_ITEM::iterator	iter = m_kEnterShopItemContainer.begin();
			while( iter != m_kEnterShopItemContainer.end() )
			{
				if( iter->kItemGuId == Result.ArticleGuId() )
				{
					iter->kItem.Count(iter->kItem.Count() - Result.LeftItemNum());
					if( iter->kItem.Count() <= 0 )
					{
						m_kEnterShopItemContainer.erase(iter);
					}
					break;
				}
				++iter;
			}
		}
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403075, true);
		return true;
	}
	return false;
}

void PgMarketShopMgr::Clear()
{
	TotalItem(0);
	Page(0);
	TotalPage(0);
	m_kShopContainer.clear();
	m_kEnterShopItemContainer.clear();
}

void PgMarketShopMgr::DeleteShop(BM::GUID const& Guid)
{
	CONT_SHOP_ITEM::iterator	iter = m_kShopContainer.begin();
	while( iter != m_kShopContainer.end() )
	{
		if( iter->kOwnerId == Guid )
		{
			m_kShopContainer.erase(iter);
			break;
		}
		++iter;
	}
}

void PgMarketShopMgr::DeleteEnterShopItem(BM::GUID const& Guid)
{
	CONT_ENTER_SHOP_ITEM::iterator	iter = m_kEnterShopItemContainer.begin();
	while( iter != m_kEnterShopItemContainer.end() )
	{
		if( iter->kItemGuId == Guid )
		{
			m_kEnterShopItemContainer.erase(iter);
			break;
		}
		++iter;
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
//	Market Search 분리 2009. 4. 7
//---------------------------------------------------------------------------------------------------------------------------------
PgMarKetSearchMgr::PgMarKetSearchMgr()
{
	Clear();
}

PgMarKetSearchMgr::~PgMarKetSearchMgr()
{
	Clear();
}

void PgMarKetSearchMgr::RecvSearchItem(BM::Stream& rkPacket)
{
	SPT_M_C_UM_ANS_MARKET_QUERY	kResult;
	kResult.ReadFromPacket(rkPacket);
	if( PgMarketUtil::RequestResult(kResult.Result()) )
	{
		TotalItem(kResult.MaxResultCount());
		TotalPage(TotalItem() * 0.1f);
		TotalPage(TotalPage() + ((0 < (m_kTotalItem % 10))?(1):(0)));
		m_kItemContainer.clear();
		m_kItemContainer.insert(kResult.ContMarket().begin(), kResult.ContMarket().end());
	}
}

void PgMarKetSearchMgr::GetPageInfo(CONT_SERACH_ITEM& kList) const
{
	CONT_SERACH_ITEM::const_iterator c_iter = m_kItemContainer.begin();
	if( c_iter == m_kItemContainer.end() )
	{
		return;
	}

	for(int i = 0; i < GetPageSlotMax(); ++i)
	{
		std::pair<CONT_SERACH_ITEM::iterator, bool> Result = kList.insert(std::make_pair(c_iter->first, c_iter->second));
		if( !Result.second )
		{
			//이건 로그로?
		}

		++c_iter;
		if( c_iter == m_kItemContainer.end() )
		{
			break;
		}
	}
}

bool PgMarKetSearchMgr::GetItemInfo(BM::GUID const& ShopGuid, BM::GUID const& ItemGuid, ITEM_VAL& kItem) const
{
	CONT_SERACH_ITEM::const_iterator c_iter = m_kItemContainer.begin();
	while( c_iter != m_kItemContainer.end() )
	{
		if( c_iter->first.kOwnerId == ShopGuid )
		{
			if( c_iter->second.kItemGuId == ItemGuid)
			{
				kItem = c_iter->second;
				return true;
			}
		}
		++c_iter;
	}
	return false;
}

void PgMarKetSearchMgr::DeleteItemInfo(BM::GUID const& ShopGuid, BM::GUID const& ItemGuid)
{
	CONT_SERACH_ITEM::iterator c_iter = m_kItemContainer.begin();
	while( c_iter != m_kItemContainer.end() )
	{
		if( c_iter->first.kOwnerId == ShopGuid )
		{
			if( c_iter->second.kItemGuId == ItemGuid)
			{
				m_kItemContainer.erase(c_iter);
				return;
			}
		}
		++c_iter;
	}
}

void PgMarKetSearchMgr::Clear()
{
	TotalItem(0);
	Page(0);
	TotalPage(0);
	Type(0);
	TypeNode(-1);
	m_kItemContainer.clear();
}