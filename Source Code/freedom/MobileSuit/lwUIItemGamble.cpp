#include "StdAfx.h"
#include "lwUI.h"
#include "lwUIItemGamble.h"
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

namespace PgGambleUtil
{
	void GambleItemUpdateClear()
	{
		g_kGambleMgr.Clear();
	}

	void GambleItemUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_GAMBLE"));
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

		if( UICT_GAMBLE != pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{
			// 상자
			return;
		}
		
		// Box Item
		g_kGambleMgr.SetMaterialItem(rkItemInvPos);
		// Gamble Item Number
		g_kGambleMgr.SetGambleItemNo(iItemNo);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	lwUIItemGamble
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
lwUIItemGamble::lwUIItemGamble(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemGamble::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GambleItemUpdate", &PgGambleUtil::GambleItemUpdate);
	
	class_<lwUIItemGamble>(pkState, "ItemGambleWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "Clear", &lwUIItemGamble::Clear)
		.def(pkState, "SendReqGamble", &lwUIItemGamble::SendReqGamble)
		.def(pkState, "GetAgainSend", &lwUIItemGamble::GetAgainSend)
		.def(pkState, "GetInProcess", &lwUIItemGamble::GetInProcess)	
		.def(pkState, "GetGambleItemCount", &lwUIItemGamble::GetGambleItemCount)
	;

	return true;
}

void lwUIItemGamble::Clear()
{
	g_kGambleMgr.Clear();
}

void lwUIItemGamble::SendReqGamble()
{
	if( false == GetAgainSend() )
	{
		g_kGambleMgr.SendReqGamble();
	}
	else
	{
		g_kGambleMgr.AgainSendUpdate();
	}
}

bool lwUIItemGamble::GetAgainSend()
{
	return g_kGambleMgr.AgainSend();
}

bool lwUIItemGamble::GetInProcess()
{
	return g_kGambleMgr.InProcess();
}

size_t lwUIItemGamble::GetGambleItemCount()
{
	return g_kGambleMgr.GetGambleItemCount();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemGambleMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemGambleMgr::PgItemGambleMgr()
{
	Clear();
}

void PgItemGambleMgr::Clear()
{
	m_kSrcItemPos.Clear();
	AgainSend(false);
	InProcess(true);
	m_iGambleItemNo = 0;
}

void PgItemGambleMgr::AgainSendUpdate()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return;}

	PgBase_Item kSrcItem;

	//아이템 위치 바뀔수 있기 때문에 다시 위치 찾기
	pInv->GetFirstItem(m_iGambleItemNo, m_kSrcItemPos);

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
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
	PgGambleUtil::GambleItemUpdate(0, kSrcItem.ItemNo(), m_kSrcItemPos);

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
		if(!pkWnd)
		{
			return;
		}
		pkWnd->Close();
		return;
	}

}

bool PgItemGambleMgr::SendReqGamble()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kSrcItem;

	if(S_OK != pInv->GetItem(m_kSrcItemPos, kSrcItem))
	{ 
		lwAddWarnDataTT(790401);
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
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
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
		if(!pkWnd)
		{
			return false;
		}
		pkWnd->Close();
		return false;
	}

	BM::Stream kPacket( PT_C_M_REQ_OPEN_GAMBLE );
	kPacket.Push( m_kSrcItemPos );
	NETWORK_SEND( kPacket );

	return true;
}

bool PgItemGambleMgr::SetMaterialItem(SItemPos const& rkItemPos)
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

void PgItemGambleMgr::SetGambleItemNo(int const& iGambleItemNo)
{
	m_iGambleItemNo = iGambleItemNo;
}

size_t PgItemGambleMgr::GetGambleItemCount()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return 0;}
	
	return pInv->GetTotalCount(m_iGambleItemNo);
}

void GambleUpdateUI(int const iItemNo)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
	if(!pkWnd)
	{
		return;
	}

	g_kGambleMgr.InProcess(false);

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
	
	g_kGambleMgr.AgainSend(true);
}

void Recv_PT_M_C_ANS_OPEN_GAMBLE(BM::Stream* pkPacket)
{
	// S_OK 일때만 들어 온다.

	std::wstring kName = _T("");
	int iGambleItemNo = 0;
	int iItemNo = 0;
	bool bRet = false;

	pkPacket->Pop(bRet);
	pkPacket->Pop(kName);
	pkPacket->Pop(iGambleItemNo);
	pkPacket->Pop(iItemNo);

	g_kGambleMgr.SetGambleItemNo(iGambleItemNo);

	char szName[100] = "Item_Gachapon_Result";
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	GambleUpdateUI(iItemNo);
}