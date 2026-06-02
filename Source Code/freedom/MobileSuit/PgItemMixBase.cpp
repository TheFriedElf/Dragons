#include "StdAfx.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgItemMixBase.h"
#include "PgMonsterCardMixer.h"

PgIMixBase::PgIMixBase()
	: m_kLackMaterial(false)
	, m_iMakingNo(0)
	, m_kbMixing(false)
	, m_kRewordItemNo(0)
	, m_kCost(0)
{
}

PgIMixBase::~PgIMixBase()
{
}

void PgIMixBase::Clear()
{
	m_kLackMaterial = false;
	m_kRewordItemNo = 0;
	bMixing(false);
	Cost(0);
}

PgItemMixBase::PgItemMixBase(void)
	: m_kRecipe(0)
	, m_kMakeCount(0)
{
}

PgItemMixBase::~PgItemMixBase(void)
{
}

bool PgItemMixBase::SetRecipe(SItemPos const kItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			PgBase_Item	kItem;
			if( S_OK == pkInv->GetItem(kItemPos, kItem) )
			{
				if( m_kRecipe.kItemNo != kItem.ItemNo() )
				{
					m_kRecipe.kItemNo = kItem.ItemNo();
					if( !SetMaterialInfo(m_kRecipe.kItemNo) )
					{
						m_kRecipe.Clear();
						return false;
					}
				}
				GetItemInfo(m_kRecipe);
				return true;
			}
			else
			{
				m_kRecipe.Clear();
				m_kMaterialContainer.clear();
			}
		}
	}
	return false;
}

void PgItemMixBase::Clear()
{
	m_kMaterialContainer.clear();
	m_kRecipe.Clear();
	MakeCount(0);
	PgIMixBase::Clear();
}

bool PgItemMixBase::SetMaterial(DWORD const dwItemNo)
{
	CONT_MATERIAL::iterator mtrl_itor = m_kMaterialContainer.find( dwItemNo );
	if( mtrl_itor != m_kMaterialContainer.end() )
	{
		CONT_MATERIAL::mapped_type& kMaterialInfo = mtrl_itor->second;

		return GetItemInfo(kMaterialInfo);
	}
	return false;
}

bool PgItemMixBase::GetItemInfo(S_MIX_ITEM_INFO& kItemInfo)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			if( S_OK == pkInv->GetFirstItem(kItemInfo.kItemNo, kItemInfo.kItemPos) )
			{
				kItemInfo.iHave = pkInv->GetTotalCount(kItemInfo.kItemNo);
				return true;
			}
		}
	}
	kItemInfo.iHave = 0;
	return false;
}

bool PgItemMixBase::AutoEntryMaterial()
{
	if( m_kRecipe.kItemNo == 0 )
	{
		return false;
	}

	if( m_kMaterialContainer.empty() )
	{
		return false;
	}

	CONT_MATERIAL::iterator mtrl_itor = m_kMaterialContainer.begin();
	while( mtrl_itor != m_kMaterialContainer.end() )
	{
		CONT_MATERIAL::mapped_type& kMaterialInfo = mtrl_itor->second;
		GetItemInfo(kMaterialInfo);
		++mtrl_itor;
	}
	return true;
}

bool PgItemMixBase::SetMaterialInfo(DWORD const kMakingItem)
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
		m_kMaterialContainer.clear();

		for(int i = 0; i < MAX_ITEMMAKING_ARRAY; ++i)
		{
			for(int j = 0; j < MAX_SUCCESS_RATE_ARRAY; ++j)
			{
				if( kMakingData.akResultItemElements[ i ].aElement[ j ] && kMakingData.akResultItemCount[ i ].aCount[ j ] )
				{
					m_kRewordItemNo = kMakingData.akResultItemElements[ i ].aElement[ j ];
				}
			}

			int const MaterialNo = kMakingData.kNeedElements.aElement[ i ];
			if( MaterialNo == 0 )
			{
				continue;
			}

			m_kRecipe.iNeed = 1;

			S_MIX_ITEM_INFO kMaterialInfo(MaterialNo);
			kMaterialInfo.iNeed = kMakingData.kNeedCount.aCount[ i ];

			if( MaterialNo == m_kRecipe.kItemNo )
			{
				m_kRecipe.iNeed = kMaterialInfo.iNeed;
				continue;
			}

			std::pair<CONT_MATERIAL::iterator, bool> Result = m_kMaterialContainer.insert(std::make_pair(kMaterialInfo.kItemNo, kMaterialInfo));
			if( !Result.second )
			{//같은재료가 2개 있을리 없다
				return false;
			}
		}
		Cost(kMakingData.iNeedMoney);
		m_iMakingNo = iMakingNo;

		return true;
	}

	return false;
}

PgItemMixMgr::PgItemMixMgr()
{
}

PgItemMixMgr::~PgItemMixMgr()
{
	Clear();
}

void PgItemMixMgr::Clear()
{
	CONT_ITEM_MIXER::iterator mix_itor = m_kContMixer.begin();
	while( mix_itor != m_kContMixer.end() )
	{
		CONT_ITEM_MIXER::mapped_type& kMixer = mix_itor->second;
		SAFE_DELETE(kMixer);
		mix_itor = m_kContMixer.erase(mix_itor);
	}
}

bool PgItemMixMgr::CreateMixer(E_MIX_TYPE const& MixType)
{
	PgIMixBase* pMixer = NewMixer(MixType);
	std::pair<CONT_ITEM_MIXER::iterator, bool> Result = m_kContMixer.insert( std::make_pair( MixType, pMixer ) );
	if( false == Result.second )
	{
		SAFE_DELETE(pMixer);
		return false;
	}
	return true;
}

PgIMixBase* PgItemMixMgr::GetMixer(E_MIX_TYPE const& MixType)
{
	PgIMixBase* pMixer = NULL;
	CONT_ITEM_MIXER::iterator mix_itor = m_kContMixer.find(MixType);
	if( mix_itor != m_kContMixer.end() )
	{
		return mix_itor->second;
	}
	return NULL;
}

PgIMixBase* PgItemMixMgr::NewMixer(E_MIX_TYPE const& MixType)
{
	switch( MixType )
	{
	case EMT_MONSTER_CARD_MIXER:
		{
			return new PgMonsterCardMixer;
		}break;
	}
	return NULL;
}

void PgItemMixMgr::DeleteMixer(E_MIX_TYPE const& MixType)
{
	CONT_ITEM_MIXER::iterator mix_itor = m_kContMixer.find(MixType);
	if( mix_itor != m_kContMixer.end() )
	{
		CONT_ITEM_MIXER::mapped_type& kMixer = mix_itor->second;
		SAFE_DELETE(kMixer);
		m_kContMixer.erase(mix_itor);
	}	
}


