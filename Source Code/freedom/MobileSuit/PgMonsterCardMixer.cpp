#include "StdAfx.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgMonsterCardMixer.h"

PgMonsterCardMixer::PgMonsterCardMixer(void)
	: m_kDefaultInsItem(0)
	, m_kbAutoRegInsItem(false)
{
}

PgMonsterCardMixer::~PgMonsterCardMixer(void)
{
}

void PgMonsterCardMixer::RequestMix()
{
	BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
	kPacket.Push(1);	// 요리 한번 하기.
	kPacket.Push(0);	//iMakingType
	kPacket.Push(m_iMakingNo);
	kPacket.Push(m_kLeftItem.iNeed);
	kPacket.Push(false);	//bUseOptionItem
	kPacket.Push(SItemPos(0,0));
	kPacket.Push(m_kInsItem.kItemPos);
	NETWORK_SEND(kPacket);
}

void PgMonsterCardMixer::Clear()
{
	PgIMixBase::Clear();
	DefaultInsItem(0);
	bAutoRegInsItem(false);
	m_kLeftItem.Clear();
	m_kRightItem.Clear();
	m_kInsItem.Clear();
}

bool PgMonsterCardMixer::SetMonsterCard(DWORD const dwItemNo, SItemPos const& kItem)
{
	SemiClear();

	if( SetMaterialInfo( dwItemNo ) )
	{
		if( AutoEntrySlot(m_kLeftItem) && AutoEntrySlot(m_kRightItem) )
		{
			if( m_kLeftItem.kItemNo == m_kRightItem.kItemNo )
			{
				int const iTotalNeed = m_kLeftItem.iNeed + m_kRightItem.iNeed;
				if( iTotalNeed > m_kLeftItem.iHave )
				{
					LackMaterial(true);
				}
			}
		}
		return true;
	}
	return false;
}

bool PgMonsterCardMixer::AutoEntrySlot(S_MIX_ITEM_INFO& kSlot)
{
	if( kSlot.IsEmpty() )
	{
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			if( S_OK == pkInv->GetFirstItem(kSlot.kItemNo, kSlot.kItemPos) )
			{
				kSlot.iHave = pkInv->GetTotalCount(kSlot.kItemNo);
				if( !LackMaterial() && kSlot.iHave < kSlot.iNeed )
				{
					LackMaterial(true);
				}
				return true;
			}
		}
	}
	kSlot.iHave = 0;
	kSlot.kItemPos = SItemPos::NullData();
	return false;
}

void PgMonsterCardMixer::SemiClear()
{
	PgIMixBase::Clear();
	m_kLeftItem.Clear();
	m_kRightItem.Clear();
}

bool PgMonsterCardMixer::SetMaterialInfo(DWORD const kMakingItem)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	int iMakingNo = kItemDefMgr.GetAbil(kMakingItem, AT_ITEMMAKING_NO);
	if( !iMakingNo )
	{
		return false;
	}

	int iMakingType = kItemDefMgr.GetAbil(kMakingItem, AT_MAKING_TYPE);
	if( !CheckMixType(iMakingType) )
	{
		return false;
	}

	SDefItemMakingData kMakingData;
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, iMakingNo))
	{
		for(int i = 0; i < MAX_ITEMMAKING_ARRAY; ++i)
		{
			for(int j = 0; j < MAX_SUCCESS_RATE_ARRAY; ++j)
			{
				if( kMakingData.akResultItemElements[ i ].aElement[ j ] && kMakingData.akResultItemCount[ i ].aCount[ j ] )
				{
					m_kRewordItemNo = kMakingData.akResultItemElements[ i ].aElement[ j ];
				}
			}
		}

		//두개초과는 없다
		m_kLeftItem.kItemNo = kMakingData.kNeedElements.aElement[ 0 ];
		m_kLeftItem.iNeed = kMakingData.kNeedCount.aCount[ 0 ];
		m_kRightItem.kItemNo = kMakingData.kNeedElements.aElement[ 1 ];
		m_kRightItem.iNeed = kMakingData.kNeedCount.aCount[ 1 ];

		Cost(kMakingData.iNeedMoney);
		m_iMakingNo = iMakingNo;
		return true;
	}

	return false;
}

bool PgMonsterCardMixer::CheckMixType(int const iType) const
{
	return ((iType & EMAKING_TYPE_MONSTER_CARD_TYPE2) == EMAKING_TYPE_MONSTER_CARD_TYPE2);
}
