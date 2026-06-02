#include "stdafx.h"
#include "PgPilotMan.h"
#include "PgMarketDef.h"

bool PgMarketUtil::RequestResult(EUserMarketResult const Result)
{
	switch(Result)
	{
	case UMR_SUCCESS:
		{
			return true;
		}
	default:
		{
			if( UMR_MAX > Result)
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403100 + Result, true);
			}
			else
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699972, true);
			}
		}break;
	}
	return	false;
}

bool bExistEmptyInv(DWORD const dwItemNo)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return false;
	}

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(dwItemNo);
	if(!pDef)
	{
		return false;
	}

	EInvType InvType = (EInvType)pDef->PrimaryInvType();

	SItemPos EmptyPos;
	return pInv->GetFirstEmptyPos(InvType,  EmptyPos);
}

bool tagShopItemInfo::ConvertItemList(CONT_USER_MARKET_ARTICLEINFO_LIST const& ItemList)
{
	m_kItemCont.clear();

	CONT_USER_MARKET_ARTICLEINFO_LIST::const_iterator	iter = ItemList.begin();
	while( iter != ItemList.end() )
	{
		SMarketItemInfoBase	ItemData;

		ItemData.Guid = iter->kItemGuId;
		ItemData.eCostType = (ERegCostType)iter->kCostType;
		ItemData.i64CostValue = iter->kArticleCost;
		ItemData.kItem = iter->kItem;
		ItemData.SellMoney = iter->kItem.Count();
		switch( iter->kItem.ItemNo() )
		{
		case MARKET_ARTICLE_CASHNO:	{ ItemData.kItemType = UMAT_CASH;	}break;
		case MARKET_ARTICLE_MONEYNO:{ ItemData.kItemType = UMAT_MONEY;	}break;
		default:{ ItemData.kItemType = UMAT_ITEM;	}break;
		}

		std::pair<kBaseItemContainer::iterator, bool> Rst = m_kItemCont.insert(std::make_pair(iter->kItemGuId, ItemData));
		if( !Rst.second )
		{
			// 실패
		}
		++iter;
	}
	return true;
}

bool tagShopItemInfo::AddItem(SMarketItemInfoBase const& ItemInfo)
{
	std::pair<kBaseItemContainer::iterator, bool> Rst = m_kItemCont.insert(std::make_pair(ItemInfo.Guid, ItemInfo));
	if( !Rst.second )
	{
		// 실패
		return false;
	}
	return true;
}
bool tagShopItemInfo::DelItem(BM::GUID const& Guid)
{
	kBaseItemContainer::iterator iter = m_kItemCont.find(Guid);
	if( iter != m_kItemCont.end() )
	{
		m_kItemCont.erase(iter);
		return true;
	}
	return false;
}

bool tagShopItemInfo::FindItem(BM::GUID const& Guid, SMarketItemInfoBase& Base)
{
	kBaseItemContainer::iterator iter = m_kItemCont.find(Guid);
	if( iter != m_kItemCont.end() )
	{
		Base = iter->second;
		return true;
	}

	return false;
}

bool tagShopItemInfo::UpdateInfo(BM::GUID const& Guid, SMarketItemInfoBase const& Base)
{
	kBaseItemContainer::iterator iter = m_kItemCont.find(Guid);
	if( iter != m_kItemCont.end() )
	{
		iter->second = Base;
		return true;
	}

	return false;
}

tagMyShopInfo::tagMyShopInfo() 
	: SShopItemInfo()
	, m_kShopInfo()
{
	CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
	if( pkUnit )
	{
		m_kShopInfo.kMarketName = pkUnit->Name();
	}
	Clear();
};

bool tagMyShopInfo::AddRegItem(BM::GUID const& Guid, SARTICLEINFO const& RegInfo)
{
	kRegItemContainer::iterator	iter = m_kTempRegList.find(Guid);
	if( iter != m_kTempRegList.end() )
	{
		return false;
	}

	std::pair<kRegItemContainer::iterator, bool> Rst = m_kTempRegList.insert(std::make_pair(Guid, RegInfo));
	if( Rst.second )
	{
		return true;
	}
	return false;
}

void tagMyShopInfo::UpdateTimeInfo(__int64 const OnTime, __int64 const OffTime)
{
	m_kShopInfo.iOnTime += OnTime;
	m_kShopInfo.iOffTime += OffTime;
}

void tagMyShopInfo::FlowTime(__int64 const TickTime)
{
	m_kShopInfo.iOnTime -= TickTime;
	//m_kShopInfo.iOffTime -= TickTime;	
}

void tagMyShopInfo::UpdateHP(int const Hp)
{
	m_kShopInfo.iHP += Hp;
}

bool tagMyShopInfo::DelItem(BM::GUID const& Guid)
{
	kRegItemContainer::iterator	iter = m_kTempRegList.find(Guid);
	if( iter != m_kTempRegList.end() )
	{
		m_kTempRegList.erase(iter);
	}
	
	return tagShopItemInfo::DelItem(Guid);
}

bool tagMyShopInfo::FindRegItem(BM::GUID const& Guid, SARTICLEINFO& RegInfo)
{
	kRegItemContainer::iterator	iter = m_kTempRegList.begin();
	if( iter != m_kTempRegList.end() )
	{
		RegInfo = iter->second;
		return true;
	}
	return false;
}

bool tagSoldItemInfo::ConvertSoldList(CONT_USER_MARKET_DEALINGINFO_LIST const& SoldList, __int64 const iSaveTime)
{
	m_kSoldCont.clear();

	CONT_USER_MARKET_DEALINGINFO_LIST::const_iterator iter = SoldList.begin();
	while( iter != SoldList.end() )
	{
		SMarketSoldItemInfo	SoldInfo;

		SoldInfo.Guid = iter->kDealingGuid;
		SoldInfo.eCostType = (ERegCostType)iter->cCostType;
		SoldInfo.i64CostValue = iter->i64SellCost;
		SoldInfo.wstrBuyerName = iter->kDealerName;
		SoldInfo.iRate = iter->iCostRate;
		SoldInfo.SaveTime = iSaveTime;

		SoldInfo.kItem.ItemNo(iter->dwItemNo);
		SoldInfo.kItem.Count(iter->wItemNum);
		SEnchantInfo	kEnchantInfo;
		kEnchantInfo.Field_1(iter->i64Enchant_01);
		kEnchantInfo.Field_2(iter->i64Enchant_02);
		kEnchantInfo.Field_3(iter->i64Enchant_03);
		kEnchantInfo.Field_4(iter->i64Enchant_04);
		SoldInfo.kItem.EnchantInfo(kEnchantInfo);
		BM::PgPackedTime kCreateDate(iter->kItemGenTime);
		SoldInfo.kItem.CreateDate(kCreateDate);

		std::pair<kSoldItemContainer::iterator, bool> Rst = m_kSoldCont.insert(std::make_pair(iter->kDealingGuid, SoldInfo));
		if( !Rst.second )
		{

		}
		++iter;
	}

	return true;
}

bool tagSoldItemInfo::DelSoldItem(BM::GUID const& Guid)
{
	kSoldItemContainer::iterator	iter = m_kSoldCont.find(Guid);
	if( iter != m_kSoldCont.end() )
	{
		m_kSoldCont.erase(iter);
		return true;
	}
	return false;
}

bool tagSoldItemInfo::GetSoldPageItem(size_t& PageNum, kSoldList& kList)
{
	size_t const& size = m_kSoldCont.size();
	size_t const& MaxPageNum = (size/MAX_SHOP_LIST_ITEM);	
	if(PageNum > MaxPageNum)
	{// 입력된 페이지 번호가 표시 할수 없는것이면 최초 페이지를 보인다.
		PageNum = 0;
	}

	kSoldItemContainer::iterator	iter = m_kSoldCont.begin();
	for(int i = 0; i < (PageNum * MAX_STOCK_SLOT);  ++i)
	{
		if( iter == m_kSoldCont.end() )
		{
			return false;
		}
		++iter;
	}

	for(int i = 0; i < MAX_STOCK_SLOT; ++i)
	{
		if( iter == m_kSoldCont.end() )
		{
			break;
		}
		kList.push_back(iter->second);
		++iter;
	}

	return true;
}

bool tagSoldItemInfo::FindItem(BM::GUID const& Guid, tagMarketItemInfoBase& Info)
{
	kSoldItemContainer::iterator	so_iter = m_kSoldCont.find(Guid);
	if( so_iter != m_kSoldCont.end() )
	{
		Info.Guid = so_iter->second.Guid;
		Info.eCostType = so_iter->second.eCostType;
		Info.i64CostValue = so_iter->second.i64CostValue;
		Info.kItem = so_iter->second.kItem;
		return true;
	}
	return false;
}
