#include "stdafx.h"
#include "lwInventoryUI.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "Variant/Inventory.H"


std::wstring const kNewItem(L"FRM_NEW_ITEM");

void lwInventory::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "ModifyCompInv", lwModifyCompInv);
	def(pkState, "ApplyCurrentInv", lwApplyCurrentInv);
	def(pkState, "CallNewInvItem_Tab", lwCallNewInvItem_Tab);
	def(pkState, "CallNewInvItem", lwCallNewInvItem);
}

void lwInventory::ClearCompInv()
{
	g_kInvUIMgr.Clear();
}

void lwInventory::InitCompInv(PgInventory* m_kOrgInv)
{
	g_kInvUIMgr.Init(m_kOrgInv);
}

void lwInventory::lwModifyCompInv(EInvType const eInvType, PgInventory* m_kOrgInv)
{
	g_kInvUIMgr.ModifyCompInvType(eInvType, m_kOrgInv);
}

void lwInventory::lwApplyCurrentInv(EInvType const eInvType)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{
		return;
	}
	PgInventory* pInv = pPlayer->GetInven();
	if(!pInv)
	{
		return;
	}

	g_kInvUIMgr.ModifyCompInvType(eInvType, pInv);
}

void lwInventory::lwCallNewInvItem_Tab(EInvType const eInvType)
{
	XUI::CXUI_Wnd* pInvUI = XUIMgr.Get(L"Inv");
	if(!pInvUI)
	{
		return;
	}
	XUI::CXUI_Wnd* pTitle = pInvUI->GetControl(L"SFRM_TITLE");
	if(!pTitle)
	{
		return;
	}
	XUI::CXUI_Wnd* pInvType = NULL;
	XUI::CXUI_Wnd* pTypeBtn = NULL;

	switch(eInvType)
	{
	case IT_EQUIP:
		{
			pInvType = pTitle->GetControl(L"SFRM_EQUIP_BG");
			if(!pInvType)
			{
				return;
			}
			pTypeBtn = pInvType->GetControl(L"CBTN_EQUIP");
			if(!pTypeBtn)
			{
				return;
			}
		}break;
	case IT_CONSUME:
		{
			pInvType = pTitle->GetControl(L"SFRM_CONSUM_BG");
			if(!pInvType)
			{
				return;
			}
			pTypeBtn = pInvType->GetControl(L"CBTN_CONSUM");
			if(!pTypeBtn)
			{
				return;
			}
		}break;
	case IT_ETC:
		{
			pInvType = pTitle->GetControl(L"SFRM_ETC_BG");
			if(!pInvType)
			{
				return;
			}
			pTypeBtn = pInvType->GetControl(L"CBTN_ETC");
			if(!pTypeBtn)
			{
				return;
			}
		}break;
	case IT_CASH:
		{
			pInvType = pTitle->GetControl(L"SFRM_CASH_BG");
			if(!pInvType)
			{
				return;
			}
			pTypeBtn = pInvType->GetControl(L"CBTN_CASH");
			if(!pTypeBtn)
			{
				return;
			}
		}break;
	default:
		{
			return;
		}break;
	}

	bool const bCheck = g_kInvUIMgr.CheckDiff_InvType( eInvType );
	XUI::CXUI_Wnd* pNew = pTypeBtn->GetControl( kNewItem );
	if( pNew )
	{
		pNew->Visible(bCheck);
	}
}

bool lwInventory::lwCallNewInvItem(EInvType const eInvType)
{
	XUI::CXUI_Wnd* pInvUI = XUIMgr.Get(L"Inv");
	if(!pInvUI)
	{
		return false;
	}
	XUI::CXUI_Wnd* pItemMain = pInvUI->GetControl(L"SFRM_ITEM_BIG");
	if(!pItemMain)
	{
		return false;
	}
	XUI::CXUI_Wnd* pItemSdw = pItemMain->GetControl(L"SFRM_SDW");
	if(!pItemSdw)
	{
		return false;
	}
	XUI::CXUI_Wnd* pIconWnd = pItemSdw->GetControl(L"FRM_ICON");
	if(!pIconWnd)
	{
		return false;
	}

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

	if( lwGetInvViewGroup() == static_cast<int>(eInvType) )
	{//현재 보여지는 인벤그룹과 같으면 연출 보여주자
		SItemPos kItemPos;
		int const MaxInvCount = pPlayerInv->GetMaxIDX(eInvType);
		int iCount = 0;
		while( iCount < MaxInvCount)
		{
			BM::vstring kIcon = BM::vstring(L"InvIcon") + iCount;
			XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(kIcon);
			if(!pIcon)
			{
				return false;
			}
			XUI::CXUI_Wnd* pNew = pIcon->GetControl(kNewItem);
			if(!pNew)
			{
				return false;
			}

			kItemPos.Set( eInvType, iCount);
			PgBase_Item kItem;
			if( S_OK == pPlayerInv->GetItem(kItemPos, kItem) )
			{
				EITEM_POS_DIFF_RET const eResult = g_kInvUIMgr.CheckDiff(kItemPos, kItem);
				if( IPD_INCREASE == eResult ||
					IPD_NEW      == eResult ||
					IPD_CHANGE   == eResult)
				{//증가/변화/신규일때만 표기
					pNew->Visible(true);
				}
				else
				{
					pNew->Visible(false);
				}
			}
			else
			{
				pNew->Visible(false);
			}
			++iCount;
		}
	}
	//이벤그룹 탭 깜빡임 연출
	lwCallNewInvItem_Tab(eInvType);

	if( lwGetInvViewGroup() == eInvType )
	{//현재 보여지는 인벤그룹과 다르면 아직 동기화 하지 말자
		lwApplyCurrentInv(eInvType);
	}
	return true;
}