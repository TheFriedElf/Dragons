#include "StdAfx.h"
#include "lwUI.h"
#include "lwUIItemJewelBox.h"
#include "PgPilotMan.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"

#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgSoundMan.h"
#include "PgChatMgrClient.h"

namespace PgJewelBoxUtil
{
	void JewelBoxKeyItemUpdate(int const iBoxType)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_JEWEL_BOX"));
		if( !pkTopWnd )
		{
			return;
		}	

		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}

		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven)
		{
			return;
		}

		CONT_HAVE_ITEM_NO kContHaveItemNo;
		kContHaveItemNo.clear();

		if(E_FAIL == pkInven->GetItems(IT_CASH, kContHaveItemNo))
		{
			return;
		}
		if(!kContHaveItemNo.size())
		{
			return;
		}

		// Init
		int const iNull = 0;

		XUI::CXUI_Wnd* pkText = pkTopWnd->GetControl(L"SFRM_COLOR_TEXT");
		if( pkText )
		{
			pkText->Text(TTW(790501));
		}
		XUI::CXUI_Wnd* pkIconLeftItem = pkTopWnd->GetControl(_T("ICN_LEFT"));
		if( pkIconLeftItem )
		{
			pkIconLeftItem->SetCustomData(&iNull, sizeof(iNull));
		}
		XUI::CXUI_Wnd* pkIconRightBGItem = pkTopWnd->GetControl(_T("ICN_RIGHT_BG"));
		if( pkIconRightBGItem )
		{
			pkIconRightBGItem->SetCustomData(&iNull, sizeof(iNull));
		}
		XUI::CXUI_Wnd* pkIconRightItem = pkTopWnd->GetControl(_T("ICN_RIGHT"));
		if( pkIconRightItem )
		{
			pkIconRightItem->SetCustomData(&iNull, sizeof(iNull));
		}


		// Key Item
		int iKeyItemNo = 0;

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CONT_HAVE_ITEM_NO::const_iterator item_no_itr = kContHaveItemNo.begin();
		for ( ; item_no_itr!=kContHaveItemNo.end() ; ++item_no_itr )
		{
			CItemDef const *pItemKeyDef = kItemDefMgr.GetDef( (*item_no_itr) );
			if ( pItemKeyDef )
			{
				if( UICT_CHEST_KEY == pItemKeyDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
				{
					int const iKeyType = pItemKeyDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

					if( iBoxType == iKeyType )
					{
						iKeyItemNo = (*item_no_itr);

						if(pkIconRightItem)
						{
							pkIconRightItem->SetCustomData(&iKeyItemNo, sizeof(iKeyItemNo));
						}
					}
				}
			}
		}		

		if( iKeyItemNo )
		{
			SItemPos kKeyItemPos;
			pkInven->GetFirstItem( IT_CASH, iKeyItemNo, kKeyItemPos );
			g_kJewelBoxMgr.SetMaterialItem(1, kKeyItemPos);

			if(pkIconRightBGItem)
			{
				pkIconRightBGItem->SetCustomData(&kKeyItemPos, sizeof(SItemPos));
			}
		}
		else
		{
			if(pkIconRightItem)
			{
				pkIconRightItem->SetCustomData(&iNull, sizeof(iNull));
			}
			if(pkIconRightBGItem)
			{				
				pkIconRightBGItem->SetCustomData(&iNull, sizeof(iNull));
			}
		}
	}

	void JewelBoxItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_JEWEL_BOX"));
		if( !pkTopWnd )
		{
			return;
		}	

		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}

		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven)
		{
			return;
		}

		XUI::CXUI_Wnd* pkIconLeftBG = pkTopWnd->GetControl(L"ICN_LEFT_BG");
		if(pkIconLeftBG)
		{
			pkIconLeftBG->Visible(true);
		}

		pkTopWnd->SetCustomData(&rkItemInvPos, sizeof(SItemPos));

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
		if( !pItemDef )
		{
			return;
		}

		if( UICT_LOCKED_CHEST != pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{
			// 상자
			return;
		}

		g_kJewelBoxMgr.Clear();
		// Box Item
		g_kJewelBoxMgr.SetMaterialItem(0, rkItemInvPos);
		int const iBoxType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

		JewelBoxKeyItemUpdate(iBoxType);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	lwUIItemJewelBox
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
lwUIItemJewelBox::lwUIItemJewelBox(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemJewelBox::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "JewelBoxItemUpdate", &PgJewelBoxUtil::JewelBoxItemUpdate);
	
	class_<lwUIItemJewelBox>(pkState, "ItemJewelBoxWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "Clear", &lwUIItemJewelBox::Clear)
		.def(pkState, "SendReqJewelBox", &lwUIItemJewelBox::SendReqJewelBox)
	;

	return true;
}

void lwUIItemJewelBox::Clear()
{
	g_kJewelBoxMgr.Clear();
}

void lwUIItemJewelBox::SendReqJewelBox()
{
	g_kJewelBoxMgr.SendReqJewelBox();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemJewelBoxMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemJewelBoxMgr::PgItemJewelBoxMgr()
{
	Clear();
}

void PgItemJewelBoxMgr::Clear()
{
	m_kSrcItemPos.Clear();
	m_kKeyItemPos.Clear();
}

bool PgItemJewelBoxMgr::SendReqJewelBox()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kSrcItem;
	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		return false;
	}

	PgBase_Item kKeyItem;
	if(S_OK != pInv->GetItem(m_kKeyItemPos, kKeyItem))
	{ 
		if( !kSrcItem.IsEmpty() )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kSrcItem.ItemNo());
			if( !pItemDef )
			{
				return false;
			}

			if( UICT_LOCKED_CHEST != pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
			{
				// 상자
				return false;
			}
			int const iBoxType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
			PgJewelBoxUtil::JewelBoxKeyItemUpdate(iBoxType);
		}		

		lwAddWarnDataTT(790504);
		return false;
	}

	if( SItemPos::NullData() == m_kSrcItemPos )
	{
		lwAddWarnDataTT(790401);
		return false;
	}
	if( SItemPos::NullData() == m_kKeyItemPos )
	{
		lwAddWarnDataTT(790401);
		return false;
	}

	BM::Stream kPacket(PT_C_M_REQ_OPEN_LOCKED_CHEST);
	kPacket.Push(m_kSrcItemPos);
	kPacket.Push(m_kKeyItemPos);
	NETWORK_SEND(kPacket)

	return true;
}

bool PgItemJewelBoxMgr::SetMaterialItem(int const iType, SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kItem;
	if(S_OK != pInv->GetItem(rkItemPos, kItem)){ return false; }

	switch( iType )
	{
	case 0:	{ m_kSrcItemPos = rkItemPos; } break;
	case 1:	{ m_kKeyItemPos = rkItemPos; } break;
	default:
		{
			return false;
		}
	}
	return true;
}

void JewelBoxUpdateUI(int const iItemNo)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_JEWEL_BOX" );
	if(!pkWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkIconLeftBG = pkWnd->GetControl(L"ICN_LEFT_BG");
	if(pkIconLeftBG)
	{
		pkIconLeftBG->Visible(false);
	}
	XUI::CXUI_Wnd* pkIconLeftItem = pkWnd->GetControl(L"ICN_LEFT");
	if(pkIconLeftItem)
	{
		pkIconLeftItem->SetCustomData(&iItemNo, sizeof(iItemNo));
	}
}

void Recv_PT_M_C_ANS_OPEN_LOCKED_CHEST(BM::Stream* pkPacket)
{
	HRESULT hResult;
	int iItemType = 0;
	int iItemNo = 0;

	pkPacket->Pop(hResult);	
	
	int iWarnMessage = 0;
	
	if( S_OK == hResult )
	{
		pkPacket->Pop(iItemType);
		pkPacket->Pop(iItemNo);

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_JEWEL_BOX" );
		if(!pkWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pkText = pkWnd->GetControl(L"SFRM_COLOR_TEXT");

		std::wstring wstrMsg = _T("");

		switch( iItemType )
		{
		case UICT_BLANK: // 꽝 아이템
			{
				wstrMsg = TTW(790507);
			}break;
		case UICT_ONECEMORE: // 한번 더
			{
				wstrMsg = TTW(790508);
			}break;
		default:
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
				if( pItemDef )
				{
					const wchar_t *pName = NULL;
					if(GetDefString(pItemDef->NameNo(), pName) )
					{
						wchar_t szTemp[1024] = {0,};
						swprintf_s(szTemp, 1024, TTW(790509).c_str(), pName);
						wstrMsg += szTemp;
					}
				}
			}break;
		}

		if(pkText && !wstrMsg.empty() )
		{
			pkText->Text(wstrMsg);
		}
	}
	else
	{		
		iWarnMessage = 790505;

		switch( hResult )
		{
		case E_OC_NOT_FOUND_TARGET_ITEM:
			{
				iWarnMessage = 790503;
			}break;
		case E_OC_NOT_FOUND_KEY_ITEM:
			{
				iWarnMessage = 790504;
			}break;
		case E_OC_EMPTY_ITEMBAG:
			{
				iWarnMessage = 790505;
			}break;
		case E_OC_NOT_MATCH_KEY:
			{
				iWarnMessage = 790506;
			}break;
		default:
			{
				iWarnMessage = 790505;
			}break;
		}		
	}

	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if( S_OK == hResult )
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	if( 0 != iWarnMessage )
	{
		Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	}
	if( S_OK == hResult )
	{
		JewelBoxUpdateUI(iItemNo);
	}
}