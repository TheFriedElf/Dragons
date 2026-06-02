#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "variant/Inventory.h"
#include "PgItemUseChecker.h"

namespace PgItemUseChecker
{
	bool PickOutOtherExtendInvItem(ContHaveItemNoCount& kItemCont, int const INV_TYPE, int const UICT_TYPE)
	{
		//아이템 사용 우선처리
		switch( UICT_TYPE )
		{
		case UICT_EXTEND_MAX_IDX:
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				ContHaveItemNoCount::iterator item_itor = kItemCont.begin();
				while( item_itor != kItemCont.end() )
				{
					ContHaveItemNoCount::key_type const& kItemNo = item_itor->first;
					CItemDef const* pItemDef = kItemDefMgr.GetDef(kItemNo);
					if( pItemDef )
					{
						int const TARTGET_INV = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
						if( TARTGET_INV != INV_TYPE )
						{
							item_itor = kItemCont.erase(item_itor);
							continue;
						}
					}
					++item_itor;
				}
			}break;
		}

		return !(kItemCont.empty());
	}

	bool CommonExtendInvTypeCheck(int const UICT_TYPE, int iInvType,  SItemPos const& kItemPos)
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

		PgBase_Item kItem;
		if( S_OK == pkInv->GetItem(kItemPos, kItem) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( pItemDef )
			{
				switch( UICT_TYPE )
				{
				case UICT_EXTEND_MAX_IDX:
					{
						iInvType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
					}break;
				}

				switch(g_kLocal.ServiceRegion())
				{
				case LOCAL_MGR::NC_DEVELOP:
				case LOCAL_MGR::NC_THAILAND:
				case LOCAL_MGR::NC_INDONESIA:
				case LOCAL_MGR::NC_JAPAN:
				case LOCAL_MGR::NC_TAIWAN:
				case LOCAL_MGR::NC_CHINA:
				case LOCAL_MGR::NC_RUSSIA:
				case LOCAL_MGR::NC_KOREA:
				case LOCAL_MGR::NC_EU:
				case LOCAL_MGR::NC_USA:
				case LOCAL_MGR::NC_FRANCE:
				case LOCAL_MGR::NC_GERMANY:
				case LOCAL_MGR::NC_SINGAPORE:
				case LOCAL_MGR::NC_PHILIPPINES:
				case LOCAL_MGR::NC_VIETNAM:
					{
						//인벤 확장 여부
						int const iExtendSize = static_cast<int>(pkInv->GetEnableExtendMaxIdx(static_cast<EInvType>(iInvType)));
						int const iCustomType_Inv = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
						if( iCustomType_Inv > iExtendSize )
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406109, true);
						}
						else
						{
							return true;
						}
					}break;
				default:
					{
						//인벤 확장 여부
						int const iExtendSize = static_cast<int>(pkInv->GetInvExtendSize(static_cast<EInvType>(iInvType)));
						if( 0 < iExtendSize )
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2919, true);
						}
						else
						{
							return true;
						}
					}break;
				}
			}
		}
		return false;
	}
}