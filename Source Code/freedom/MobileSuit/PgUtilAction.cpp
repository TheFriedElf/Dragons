#include "stdafx.h"
#include "PgPilot.h"
#include "PgUtilAction.h"
#include "PgWorld.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "lwWorld.h"
#include "PgChatMgrClient.h"
#include "PgCommunityEvent.h"

PgUtilAction_HaveReviveItem::PgUtilAction_HaveReviveItem( PgInventory * const pkInv, int const iLevel )
:	m_rkInv(*pkInv)
,	m_iLevel(iLevel)
{
}

bool PgUtilAction_HaveReviveItem::IsHave( const EInvType kInvType )const
{
	CONT_HAVE_ITEM_NO kContHaveItemNo;
	if ( S_OK == m_rkInv.GetItems( kInvType, kContHaveItemNo ) )
	{
		if ( kContHaveItemNo.size() > 0 )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);

			CONT_HAVE_ITEM_NO::const_iterator item_no_itr = kContHaveItemNo.begin();
			for ( ; item_no_itr!=kContHaveItemNo.end() ; ++item_no_itr )
			{
				CItemDef const *pItemDef = kItemDefMgr.GetDef( *item_no_itr );
				if ( pItemDef )
				{
					if ( UICT_REVIVE == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
					{
						int const iMinLv = pItemDef->GetAbil(AT_LEVELLIMIT);
						int const iMaxLv = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
						if( m_iLevel >= iMinLv )
						{
							if ( (0 == iMaxLv) || m_iLevel <= iMaxLv )
							{
								return true;
							}
						}
					}
				}
			}	
		}
	}
	return false;
}

bool PgUtilAction_HaveReviveItem::GetItemPos( SItemPos &rkOutItemPos, const EInvType kInvType )const
{
	CONT_HAVE_ITEM_NO kContHaveItemNo;
	if ( S_OK == m_rkInv.GetItems( kInvType, kContHaveItemNo ) )
	{
		if ( kContHaveItemNo.size() > 0 )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);

			int iItemNo = 0;
			int iLastCheckMaxLevelLimit = INT_MAX;

			CONT_HAVE_ITEM_NO::const_iterator item_no_itr = kContHaveItemNo.begin();
			for ( ; item_no_itr!=kContHaveItemNo.end() ; ++item_no_itr )
			{
				CItemDef const *pItemDef = kItemDefMgr.GetDef( *item_no_itr );
				if ( pItemDef )
				{
					if ( UICT_REVIVE == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
					{
						if( (ICMET_Cant_SelfUse & pItemDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_SelfUse )
						{
							continue;
						}
						int const iMinLv = pItemDef->GetAbil(AT_LEVELLIMIT);
						int iMaxLv = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
						if ( 0 == iMaxLv )
						{
							iMaxLv = 1000;
						}

						if(		m_iLevel >= iMinLv 
							&&	m_iLevel <= iMaxLv 
							)
						{
							if ( iLastCheckMaxLevelLimit > iMaxLv )
							{
								iItemNo = *item_no_itr;
								iLastCheckMaxLevelLimit = iMaxLv;
							}
						}
					}
				}
			}

			if( iItemNo )
			{
				if( S_OK == m_rkInv.GetFirstItem( kInvType, iItemNo, rkOutItemPos, true ) )
				{
					return true;
				}
			}
		}
	}

	return false;
}


PgUtilAction_HaveCrossItem::PgUtilAction_HaveCrossItem( PgInventory * const pkInv )
:	m_rkInv(*pkInv)
{
}

bool PgUtilAction_HaveCrossItem::GetItemPos( SItemPos &rkOutItemPos )const
{
	CONT_HAVE_ITEM_NO kContHaveItemNo;
	if ( S_OK == m_rkInv.GetItems( IT_CASH, kContHaveItemNo ) )
	{
		if ( kContHaveItemNo.size() > 0 )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);

			int iItemNo = 0;

			CONT_HAVE_ITEM_NO::const_iterator item_no_itr = kContHaveItemNo.begin();
			for ( ; item_no_itr!=kContHaveItemNo.end() ; ++item_no_itr )
			{
				CItemDef const *pItemDef = kItemDefMgr.GetDef( (*item_no_itr) );
				if ( pItemDef )
				{
					if ( UICT_REVIVE == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
					{
						if( (ICMET_Cant_SelfUse & pItemDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_SelfUse )
						{
							iItemNo = (*item_no_itr);
						}
					}
				}
			}

			if ( iItemNo )
			{
				if( S_OK == m_rkInv.GetFirstItem(IT_CASH, iItemNo, rkOutItemPos) )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pkItmDef = kItemDefMgr.GetDef(iItemNo);
					if( pkItmDef
					&&	g_pkWorld )
					{
						T_GNDATTR const iNotApplyGattr = (T_GNDATTR)(pkItmDef->GetAbil(AT_NOTAPPLY_MAPATTR));
						T_GNDATTR const iApplyGattr = (T_GNDATTR)(pkItmDef->GetAbil(AT_CAN_GROUND_ATTR));
						bool bCantUseGround = (0 != (iNotApplyGattr & g_pkWorld->GetAttr()));
						bCantUseGround = ((0 != iApplyGattr)? 0 == (g_pkWorld->GetAttr() & iApplyGattr): false) || bCantUseGround;
						if( !bCantUseGround )
						{
							return true; // 사용 할 수 있어야 만
						}
					}
				}
			}
		}
	}
	return false;
}

PgUtilAction_Revive::PgUtilAction_Revive(const ESpendMoneyType kType,PgPilot* pkPilot)
:	m_kType(kType),	m_pkPilot(pkPilot)
{
}

__int64 PgUtilAction_Revive::GetNeedMoney()const
{
	if ( !m_pkPilot )
	{
		PG_ASSERT_LOG(m_pkPilot);
		return 0;
	}

	GET_DEF(PgDefSpendMoneyMgr, kDefSpendMoneyMgr)
	return (__int64)(kDefSpendMoneyMgr.GetSpendMoney(m_kType,m_pkPilot->GetAbil(AT_LEVEL)));
}

bool PgUtilAction_Revive::DoAction(bool const bRealAction)
{
	if ( !g_pkWorld )
	{
		PG_ASSERT_LOG(g_pkWorld);
		return false;
	}

	if ( !m_pkPilot )
	{
		PG_ASSERT_LOG(m_pkPilot);
		return false;
	}

	if(g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND))
	{ //현재 맵이 이벤트 던전이고 아이템 부활 아이템 사용이 불가 상태라면..
		SEventScheduleData kEventSchedule;
		if(COMMUNITY_EVENT::GetEventSchedule(kEventSchedule) && kEventSchedule.UnUsableReviveItem)
		{
			return false;
		}
	}


	switch(m_kType)
	{
	case ESMT_REVIVE_MISSION:
		{
			if(GetNeedMoney() > m_pkPilot->GetAbil64(AT_MONEY))
			{	// 돈이 부족하다.
				lwAddWarnDataTT(80024);
				return false;
			}

			if( !g_pkWorld->IsHaveAttr(GATTR_MISSION) )
			{
				return false;
			}

			if ( m_pkPilot->GetAbil(AT_HP) > 0 )
			{
				return false;
			}
		}break;
	case ESMT_REVIVE_BY_INSURANCE:
		{
			PgInventory * pkInv = m_pkPilot->GetUnit()->GetInven();
			if ( pkInv )
			{
				SItemPos kItemPos;

				PgUtilAction_HaveReviveItem kAction( pkInv, m_pkPilot->GetAbil(AT_LEVEL) );

				// 검사 조건 순서 바꾸지 마세요.
				if( true == kAction.GetItemPos( kItemPos, IT_CONSUME )	// 일반 부활 깃털 먼저!!
				||	true == kAction.GetItemPos( kItemPos, IT_CASH ) )	// 캐시 부활 깃털은 그 후에 사용
				{
					BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
					kPacket.Push(lwGetServerElapsedTime32());
					NETWORK_SEND(kPacket)
					return true;
				}
			}

			lwAddWarnDataTT(80031);//사망보험이 없습니다.
			return false;
		}break;
	case ESMT_REVIVE_BY_CROSS:// 십자가 부활
		{
			PgInventory * pkInv = m_pkPilot->GetUnit()->GetInven();
			if ( pkInv )
			{
				SItemPos kItemPos;

				PgUtilAction_HaveCrossItem kAction( pkInv );
				if ( true == kAction.GetItemPos( kItemPos ) )
				{
					BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
					kPacket.Push(lwGetServerElapsedTime32());
					NETWORK_SEND(kPacket)
					return true;
				}
				else
				{
					return false;
				}
			}
			return false;
		}break;
	case ESMT_REVIVE_BY_SUPER_GROUND_FEATHER:
		{
			PgInventory * pkInv = m_pkPilot->GetUnit()->GetInven();
			if ( pkInv )
			{
				CONT_HAVE_ITEM_DATA kContItemData;
				if( S_OK == pkInv->GetItems(IT_CASH, UICT_SUPER_GROUND_FEATHER, kContItemData) )
				{
					int const iItemNo = (*kContItemData.begin()).first;
					PgBase_Item kItem = (*kContItemData.begin()).second;
					if( iItemNo )
					{
						SItemPos kItemPos;
						if( S_OK == pkInv->GetItem(kItem.Guid(), kItem, kItemPos) )
						{
							BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
							kPacket.Push(lwGetServerElapsedTime32());
							NETWORK_SEND(kPacket)
						}
					}
				}
				else
				{
					SChatLog kChatLog(CT_EVENT_SYSTEM);
					g_kChatMgrClient.AddMessage(799224, kChatLog, true);
				}
			}
			return false;
		}break;
	default:
		{
			return false;
		}break;
	}

	if ( bRealAction )
	{
		NETWORK_SEND(BM::Stream(PT_C_M_REQ_SPEND_MONEY,m_kType))
	}
	return true;
}

