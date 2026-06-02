#include "StdAfx.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgSoundMan.h"
#include "PgEqItemMixCreator.h"
#include "lwUI.h"

int const DEF_EQ_MIX_INSURANCE_ITEM_NO = 99600130;
int const DEF_EQ_MIX_PROB_ITEM_NO = 98001180;

PgEqItemMixCreator::PgEqItemMixCreator(void)
	: PgItemMixBase()
	, m_kInsurance(0)
	, m_kProbAbility(0)
{
	m_kInsurance.iNeed = 1;
	m_kProbAbility.iNeed = 1;
}

PgEqItemMixCreator::~PgEqItemMixCreator(void)
{
}

bool PgEqItemMixCreator::SetRecipe(SItemPos const kItemPos)
{
	LackMaterial(false);

	if( PgItemMixBase::SetRecipe(kItemPos) )
	{
		PgItemMixBase::AutoEntryMaterial();
		if( !m_kInsurance.IsEmpty() )
		{
			SetInsurance();
		}
		if( !m_kProbAbility.IsEmpty() )
		{
			SetProbAbility();
		}
		return true;
	}
	m_kInsurance.Clear();
	m_kProbAbility.Clear();
	return false;
}

bool PgEqItemMixCreator::SetRecipe(DWORD const iItemNo)
{
	if( iItemNo != 0 )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			if( pkInv )
			{
				SItemPos	kPos;
				if( S_OK == pkInv->GetFirstItem(iItemNo, kPos) )
				{
					return this->SetRecipe(kPos);
				}
			}
		}
	}
	this->Clear();
	return false;
}

void PgEqItemMixCreator::RequestMix()
{
	BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
	kPacket.Push(1);	// 요리 한번 하기.
	kPacket.Push(static_cast<int>(0));	//iMakingType
	kPacket.Push(m_iMakingNo);
	kPacket.Push(m_kRecipe.iNeed);
	kPacket.Push(false);	//bUseOptionItem
	kPacket.Push(m_kProbAbility.kItemPos);
	kPacket.Push(m_kInsurance.kItemPos);

	NETWORK_SEND(kPacket);
}

bool PgEqItemMixCreator::SetInsurance(bool const bUICall)
{
	if( bUICall )
	{
		if( !m_kInsurance.IsEmpty() )
		{
			m_kInsurance.Clear();
			return false;
		}
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

	ContHaveItemNoCount	kItemCont;
	if( S_OK == pkInv->GetItems(UICT_MAKING_INSURANCE, kItemCont) )
	{
		ContHaveItemNoCount::iterator item_itor = kItemCont.begin();
		if( item_itor != kItemCont.end() )
		{
			if( kItemCont.size() == 1 )
			{	
				ContHaveItemNoCount::key_type const& kItemNo = item_itor->first;

				m_kInsurance.kItemNo = kItemNo;
				PgItemMixBase::GetItemInfo(m_kInsurance);
			}
			else
			{
				UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, UIItemUtil::CONT_CUSTOM_PARAM(), UIItemUtil::CONT_CUSTOM_PARAM_STR());
			}
		}
	}
	else
	{
		m_kInsurance.Clear();
		return false;
	}
	return true;
}

void PgEqItemMixCreator::SetInsurance(SItemPos const& kItemPos)
{
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
	
	PgBase_Item kItem;
	if( S_OK == pkInv->GetItem(kItemPos, kItem) )
	{
		m_kInsurance.kItemNo = kItem.ItemNo();
		PgItemMixBase::GetItemInfo(m_kInsurance);	
	}
}

bool PgEqItemMixCreator::SetProbAbility(bool const bUICall)
{
	if( bUICall )
	{
		if( !m_kProbAbility.IsEmpty() )
		{
			m_kProbAbility.Clear();
			return false;
		}
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

	ContHaveItemNoCount	kItemCont;
	if( S_OK == pkInv->GetItems(UICT_MAKING_SUCCESS, kItemCont) )
	{
		ContHaveItemNoCount::iterator item_itor = kItemCont.begin();
		if( item_itor != kItemCont.end() )
		{
			if( kItemCont.size() == 1 )
			{	
				ContHaveItemNoCount::key_type const& kItemNo = item_itor->first;

				m_kProbAbility.kItemNo = kItemNo;
				PgItemMixBase::GetItemInfo(m_kProbAbility);
			}
			else
			{
				UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, UIItemUtil::CONT_CUSTOM_PARAM(), UIItemUtil::CONT_CUSTOM_PARAM_STR());
			}
		}
	}
	else
	{
		m_kProbAbility.Clear();
		return false;
	}
	return true;
}

void PgEqItemMixCreator::SetProbAbility(SItemPos const& kItemPos)
{
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
	
	PgBase_Item kItem;
	if( S_OK == pkInv->GetItem(kItemPos, kItem) )
	{
		m_kProbAbility.kItemNo = kItem.ItemNo();
		PgItemMixBase::GetItemInfo(m_kProbAbility);	
	}
}

void PgEqItemMixCreator::Clear()
{
	m_kInsurance.Clear();
	m_kProbAbility.Clear();
	PgItemMixBase::Clear();
}

PgCommonTypeMixer::CONT_MIXER_INITINFO PgCommonTypeMixer::m_kContMixerInitInfo;

PgCommonTypeMixer::PgCommonTypeMixer(void)
	: PgItemMixBase()
	, m_kMixType(MT_NONE)
{
	if( m_kContMixerInitInfo.empty() )
	{
		for( int i = MT_NONE; i < MT_END; ++i )
		{
			SMIXER_INIT_INFO kInfo;
			
			switch( i )
			{
			case MT_MONSTER_CARD:
				{ 
					kInfo.SetBaseInfo(L"MonsterCard", 790451, 790452, 790454, 790456);
				}break;
			case MT_COMPOSITE:		
				{ 
					kInfo.SetBaseInfo(L"Composite", 790471, 790472, 790473, 790474);		
				}break;
			}

			for( int j = 0; j < ME_END; ++j )
			{
				switch( j )
				{
				case ME_NO_REG_RECIPE:
					{
						switch( i )
						{
						case MT_MONSTER_CARD: { kInfo.iErrTT[j] = 403078; }break;
						case MT_COMPOSITE:	  {	kInfo.iErrTT[j] = 790476; }break;
						}
					}break;
				case ME_MATERIAL_LACK:
					{
						kInfo.iErrTT[j] = 50708;
					}break;
				case ME_MONEY_LACK:
					{
						kInfo.iErrTT[j] = 403078;
					}break;
				default:
					{
						continue;
					}
				}
			}

			m_kContMixerInitInfo.insert( std::make_pair( static_cast<E_MIX_TYPE>(i), kInfo) );
		}
	}
}

PgCommonTypeMixer::~PgCommonTypeMixer(void)
{
}

bool PgCommonTypeMixer::SetRecipe(SItemPos const kItemPos)
{
	LackMaterial(false);

	if( PgItemMixBase::SetRecipe(kItemPos) )
	{
		PgItemMixBase::AutoEntryMaterial();
		return true;
	}
	return false;
}

void PgCommonTypeMixer::RequestMix()
{
	BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
	kPacket.Push(1);	// 요리 한번 하기.
	kPacket.Push(static_cast<int>(0));	//iMakingType
	kPacket.Push(m_iMakingNo);
	kPacket.Push(m_kRecipe.iNeed);
	kPacket.Push(false);	//bUseOptionItem
	kPacket.Push(SItemPos());
	kPacket.Push(SItemPos());

	NETWORK_SEND(kPacket);
}

bool PgCommonTypeMixer::GetMixerInitInfo(SMIXER_INIT_INFO& kInfo) const
{
	CONT_MIXER_INITINFO::const_iterator info_itor = m_kContMixerInitInfo.find( MixType() );
	if( info_itor != m_kContMixerInitInfo.end() )
	{
		kInfo = info_itor->second;
		return true;
	}
	return false;
}

bool PgCommonTypeMixer::CheckMixType(int const iType) const
{
	if( (iType & EMAKING_TYPE_MONSTER_CARD) == EMAKING_TYPE_MONSTER_CARD )
	{
		m_kMixType = MT_MONSTER_CARD;
		return true;
	}
	else if( (iType & EMAKING_TYPE_COMPOSITE) == EMAKING_TYPE_COMPOSITE )
	{
		m_kMixType = MT_COMPOSITE;
		return true;
	}
	else
	{
		m_kMixType = MT_NONE;
	}
	return false;
}
