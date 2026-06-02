#include "stdafx.h"
#include "PgInventoryUI.h"
#include "PgPilotMan.h"

PgInventoryUI::PgInventoryUI()
{
	m_kCompInv.Clear();
}

PgInventoryUI::~PgInventoryUI()
{
	m_kCompInv.Clear();
}

void PgInventoryUI::Init(PgInventory* pkOrgInv)
{
	if( !pkOrgInv || pkOrgInv->IsEmpty())
	{
		return;
	}

	size_t iItemCount = 0;
	iItemCount += m_kCompInv.GetItemCount(IT_EQUIP);
	iItemCount += m_kCompInv.GetItemCount(IT_CONSUME);
	iItemCount += m_kCompInv.GetItemCount(IT_ETC);
	iItemCount += m_kCompInv.GetItemCount(IT_CASH);
	if( 0 == iItemCount)
	{
		BM::Stream kPacket;
		pkOrgInv->WriteToPacket(kPacket,WT_DEFAULT);
		m_kCompInv.ReadFromPacket(kPacket,WT_DEFAULT);
	}
}

void PgInventoryUI::Clear()
{
	m_kCompInv.Clear();
}

void PgInventoryUI::ModifyCompInvType(const EInvType eInvType, PgInventory *pkOrgInv)
{
	if( !pkOrgInv)
	{
		return;
	}

	BM::Stream kPacket;
	pkOrgInv->WriteToPacket(eInvType, kPacket);
	m_kCompInv.Clear(eInvType);
	m_kCompInv.ReadFromPacket(eInvType, kPacket);
}

EITEM_POS_DIFF_RET PgInventoryUI::CheckDiff(const SItemPos &rPos, PgBase_Item const & kOrgItem)
{
	if(m_kCompInv.IsEmpty() )
	{
		//return IPD_ERROR;
	}

	if( kOrgItem.IsEmpty() )
	{//빈공간으로 바뀌었다.
		return IPD_EMPTY;
	}

	PgBase_Item kCompItem;
	if( E_FAIL == m_kCompInv.GetItem(rPos, kCompItem) )
	{//비교 인벤에 없으면 새로 생긴 아이템
		return IPD_NEW;
	}

	BM::PgPackedTime const kGenTime_Org = kOrgItem.CreateDate();
	BM::PgPackedTime const kGenTime_Comp = kCompItem.CreateDate();
	if(kOrgItem.ItemNo() != kCompItem.ItemNo() ||
		kGenTime_Org.GetTime() != kGenTime_Comp.GetTime() )
	{//다른 아이템으로 바뀜
		return IPD_CHANGE;
	}
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kOrgItem.ItemNo());
	if(!pDef)
	{
		return IPD_ERROR;
	}
	if(false == pDef->IsType(ITEM_TYPE_EQUIP) )
	{
		if(kOrgItem.Count() > kCompItem.Count() )
		{//갯수 증가
			return IPD_INCREASE;
		}
		else if(kOrgItem.Count() < kCompItem.Count() )
		{//갯수 감소
			return IPD_DECREASE;
		}
	}

	return IPD_NONE;
}

bool PgInventoryUI::CheckDiff_InvType(const EInvType eInvType)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{
		return false;
	}
	PgInventory* pPlayerInv = pPlayer->GetInven();
	if(!pPlayerInv)
	{
		return false;
	}
	if( 0 == pPlayerInv->GetItemCount(eInvType) )
	{
		return false;
	}
	SItemPos kItemPos;
	int const MaxInvCount = pPlayerInv->GetMaxIDX(eInvType);
	int iCount = 0;
	while( iCount < MaxInvCount)
	{
		kItemPos.Set(eInvType,iCount);
		PgBase_Item kItem;
		if( S_OK == pPlayerInv->GetItem(kItemPos, kItem) )
		{
			EITEM_POS_DIFF_RET const eResult = CheckDiff(kItemPos, kItem);
			if( IPD_NONE != eResult &&
				IPD_ERROR != eResult )
			{
				return true;
			}
		}
		++iCount;
	}

	return false;
}