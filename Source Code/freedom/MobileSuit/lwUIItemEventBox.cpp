#include "StdAfx.h"
#include "lwUI.h"
#include "lwUIItemEventBox.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgNifMan.H"
#include "PgUIModel.h"

#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgChatMgrClient.h"
#include "PgSoundMan.h"
#include "lwUIQuest.h"
#include "lwWorld.h"

namespace PgEventBoxUtil
{
	void EventBoxItemUpdateClear()
	{
		g_kEventBoxMgr.Clear();
	}

	void EventBoxItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_EVENT_BOX"));
		if( !pkTopWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkOkWnd = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
		if( pkOkWnd )
		{			
			pkOkWnd->Text(TTW(222) + TTW(2200));
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

		// Init
		int const iNull = 0;

		XUI::CXUI_Wnd* pkText = pkTopWnd->GetControl(L"SFRM_COLOR_TEXT");
		if( pkText )
		{
			pkText->Text(TTW(790521));
		}
		XUI::CXUI_Wnd* pkIconLeftItem = pkTopWnd->GetControl(_T("ICN_RESULT"));
		if( pkIconLeftItem )
		{
			pkIconLeftItem->SetCustomData(&iNull, sizeof(iNull));
		}

		XUI::CXUI_Wnd* pkIconLeftBG = pkTopWnd->GetControl(L"ICN_RESULT_BG");
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

		if( UICT_EVENT_ITEM_REWARD_GAMBLE != pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{
			// 상자
			return;
		}
		
		// Box Item
		g_kEventBoxMgr.SetMaterialItem(rkItemInvPos);
		// EventBox Item Number
		g_kEventBoxMgr.SetEventBoxItemNo(iItemNo);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	lwUIItemEventBox
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
lwUIItemEventBox::lwUIItemEventBox(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemEventBox::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "EventBoxItemUpdate", &PgEventBoxUtil::EventBoxItemUpdate);
	
	class_<lwUIItemEventBox>(pkState, "ItemEventBoxWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "Clear", &lwUIItemEventBox::Clear)
		.def(pkState, "SendReqEventBox", &lwUIItemEventBox::SendReqEventBox)
		.def(pkState, "GetAgainSend", &lwUIItemEventBox::GetAgainSend)
		.def(pkState, "GetInProcess", &lwUIItemEventBox::GetInProcess)	
		.def(pkState, "GetEventBoxItemCount", &lwUIItemEventBox::GetEventBoxItemCount)
	;

	return true;
}

void lwUIItemEventBox::Clear()
{
	g_kEventBoxMgr.Clear();
}

void lwUIItemEventBox::SendReqEventBox()
{
	if( false == GetAgainSend() )
	{
		g_kEventBoxMgr.SendReqEventBox();
	}
	else
	{
		g_kEventBoxMgr.AgainSendUpdate();
	}
}

bool lwUIItemEventBox::GetAgainSend()
{
	return g_kEventBoxMgr.AgainSend();
}

bool lwUIItemEventBox::GetInProcess()
{
	return g_kEventBoxMgr.InProcess();
}

size_t lwUIItemEventBox::GetEventBoxItemCount()
{
	return g_kEventBoxMgr.GetEventBoxItemCount();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemEventBoxMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemEventBoxMgr::PgItemEventBoxMgr()
{
	Clear();
}

void PgItemEventBoxMgr::Clear()
{
	m_kSrcItemPos.Clear();
	AgainSend(false);
	InProcess(true);
	m_iEventBoxItemNo = 0;
}

void PgItemEventBoxMgr::AgainSendUpdate()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return;}

	PgBase_Item kSrcItem;

	//아이템 위치 바뀔수 있기 때문에 다시 위치 찾기
	pInv->GetFirstItem(m_iEventBoxItemNo, m_kSrcItemPos);

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
		if(!pkWnd)
		{
			return;
		}
		pkWnd->Close();
		return;
	}

	size_t iItemCount2 = pInv->GetTotalCount(kSrcItem.ItemNo());

	AgainSend(false);
	InProcess(true);
	PgEventBoxUtil::EventBoxItemUpdate(0, kSrcItem.ItemNo(), m_kSrcItemPos);

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
		if(!pkWnd)
		{
			return;
		}
		pkWnd->Close();
		return;
	}

}

bool PgItemEventBoxMgr::SendReqEventBox()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kSrcItem;

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
		if(!pkWnd)
		{
			return false;
		}
		pkWnd->Close();
		return false;
	}

	if( SItemPos::NullData() == m_kSrcItemPos )
	{
		lwAddWarnDataTT(790401);
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
		if(!pkWnd)
		{
			return false;
		}
		pkWnd->Close();
		return false;
	}

	BM::Stream kPacket( PT_C_M_REQ_ITEM_ACTION );
	kPacket.Push( m_kSrcItemPos );
	kPacket.Push(lwGetServerElapsedTime32());
	NETWORK_SEND( kPacket );

	return true;
}

bool PgItemEventBoxMgr::SetMaterialItem(SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kItem;
	if(S_OK != pInv->GetItem(rkItemPos, kItem)){ return false; }

	m_kSrcItemPos = rkItemPos;

	return true;
}

void PgItemEventBoxMgr::SetEventBoxItemNo(int const& iEventBoxItemNo)
{
	m_iEventBoxItemNo = iEventBoxItemNo;
}

size_t PgItemEventBoxMgr::GetEventBoxItemCount()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return 0;}
	
	return pInv->GetTotalCount(m_iEventBoxItemNo);
}

void EventBoxUpdateUI(int const iItemNo)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
	if(!pkWnd)
	{
		return;
	}

	g_kEventBoxMgr.InProcess(false);

	XUI::CXUI_Wnd* pkIconLeftItem = pkWnd->GetControl(L"ICN_RESULT");
	if(pkIconLeftItem)
	{
		pkIconLeftItem->SetCustomData(&iItemNo, sizeof(iItemNo));
	}

	XUI::CXUI_Wnd* pkIconLeftBG = pkWnd->GetControl(L"ICN_RESULT_BG");
	if(pkIconLeftBG)
	{
		pkIconLeftBG->Visible(false);
	}

	XUI::CXUI_Wnd* pkOkWnd = pkWnd->GetControl(_T("BTN_TRY_REFINE"));
	if( pkOkWnd )
	{			
		pkOkWnd->Text(TTW(400002) + TTW(2200));
	}
			
	std::wstring wstrMsg = TTW(790522);
	XUI::CXUI_Wnd* pkText = pkWnd->GetControl(L"SFRM_COLOR_TEXT");
	if( pkText )
	{
		pkText->Text(wstrMsg);
	}
	
	g_kEventBoxMgr.AgainSend(true);
}

void Recv_PT_M_C_ANS_EVENT_ITEM_REWARD(BM::Stream* pkPacket)
{
	// S_OK 일때만 들어 온다.

	int iEventBoxItemNo = 0;
	int iResultItemNo = 0;

	pkPacket->Pop(iEventBoxItemNo);
	pkPacket->Pop(iResultItemNo);

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(iEventBoxItemNo);
	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return;}
	
	g_kEventBoxMgr.SetEventBoxItemNo(iEventBoxItemNo);
	char szName[100] = "Item_Gachapon_Result";
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	EventBoxUpdateUI(iResultItemNo);
}