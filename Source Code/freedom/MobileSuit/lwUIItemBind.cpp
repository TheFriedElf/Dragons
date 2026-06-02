#include "StdAfx.h"
#include "PgNetwork.h"
#include "lwUIItemBind.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "lwActor.h"

std::string const EFFECT_UNBIND_GOLD("ef_Unbind01");
std::string const EFFECT_UNBIND_PLATINUM("ef_Unbind02");

namespace lwUIItemBind
{
	bool lwUIItemBind::RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallBindItemUI", &CallBindItemUI);
		def(pkState, "ItemBindDisplaySrcIcon", &lwItemBindDisplaySrcIcon);
		def(pkState, "ItemBindSendReq", &lwItemBindSendReq);		
		def(pkState, "ItemBindCheckOK", &lwItemBindCheckOK);
		def(pkState, "ItemBindClearAll", &lwItemBindClearAll);
		def(pkState, "ItemBindClearTargetItemInfo", &lwItemBindClearTargetItemInfo);		

		def(pkState, "CallUnbindItemUI", &CallUnbindItemUI);
		def(pkState, "ItemUnbindDisplaySrcIcon", &lwItemUnbindDisplaySrcIcon);
		def(pkState, "ItemUnbindSendReq", &lwItemUnbindSendReq);
		def(pkState, "ItemUnbindCheckOK", &lwItemUnbindCheckOK);
		def(pkState, "ItemUnbindClearAll", &lwItemUnbindClearAll);
		def(pkState, "ItemUnbindClearTargetItemInfo", &lwItemUnbindClearTargetItemInfo);

		return true;
	}

	// 자물쇠 사용 관련 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void UseBindItem(SItemPos const& rkItemInvPos)
	{// 자물쇠 아이템 사용
		if(!CallBindItemUI())
		{
			return;
		}
		g_kItemBindMgr.SetCashItemFromInv(rkItemInvPos);
	}
	bool CallBindItemUI()
	{
		{
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_ITEM_UNBINDING"));
			if(pkWnd)
			{// 열쇠 창이 이미 떠있다면 바인드 창을 호출할수 없다.
				::Notice_Show(TTW(790317), EL_Warning);
				return false;
			}
		}

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_ITEM_BINDING"));
		if( !pkWnd || pkWnd->IsClosed() )
		{
			pkWnd = XUIMgr.Call(_T("SFRM_ITEM_BINDING"));
			if( !pkWnd )
			{
				return false;
			}
		}
		XUI::CXUI_Wnd* pkTitleBG = pkWnd->GetControl(_T("SFRM_TITLE_BG"));
		if(pkTitleBG)
		{
			XUI::CXUI_Wnd* pkTitle = pkTitleBG->GetControl(_T("TITLE"));
			if(pkTitle)
			{
				pkTitle->Text(TTW(790299));
			}
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl( _T("LIST_TEXT")) );
		if(!pList)
		{
			return false;
		}
		POINT2 const DefaultSize(pList->Size().x - 25, pList->Size().y - 8);

		XUI::SListItem*	pItem = NULL;
		if( 0 == pList->GetTotalItemCount() )
		{
			pItem = pList->AddItem(_T(""));
		}
		else
		{
			pItem = pList->FirstItem();
		}
		if( !pItem )
		{
			return false;
		}
		XUI::CXUI_Wnd* pText = pItem->m_pWnd;
		pText->Size(DefaultSize);
		std::wstring kText = TTW(790302);
		pText->Text(kText);

		XUI::CXUI_Style_String	kString = pText->StyleText();
		POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kString));
		//if( kTextSize.y > DefaultSize.y )
		//{
		pText->Size(kTextSize);
		//}
		//else
		//{
		//	pText->TextPos(POINT2(pText->TextPos().x, (DefaultSize.y - kTextSize.y) * 0.5f));
		//}
		pText->SetInvalidate();

		XUI::CXUI_AniBar* pkAniWnd = dynamic_cast<XUI::CXUI_AniBar*>( pkWnd->GetControl(_T("BAR_TIMER")) );
		if(!pkAniWnd)
		{
			_PgMessageBox("lwUIItemBind::CallBindItemUI", "Can't find \"BAR_TIMER\"");
			return false;
		}
		pkAniWnd->Max(0);
		pkAniWnd->Now(0);	
		return true;
	}
	void lwItemBindDisplaySrcIcon(lwUIWnd kWnd)
	{
		g_kItemBindMgr.DisplayItem(kWnd.GetSelf());
	}

	bool lwItemBindSendReq()
	{// 서버로 아이템 자물쇠 사용 요청을 보낼때

		if(true == g_kItemBindMgr.CheckOK())
		{// 모든 체크 항목이 정상적이라면 패킷을 보낸다
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION_BIND);
			kPacket.Push(g_kItemBindMgr.GetCashItemFromInv());		//자물쇠 위치
			kPacket.Push(g_kItemBindMgr.GetTargetItemPosFromInv());	//대상 아이템 위치
			NETWORK_SEND(kPacket);
			return true;
		}
		return false;
	}
	bool lwItemBindCheckOK()
	{
		return g_kItemBindMgr.CheckOK();
	}
	void lwItemBindClearAll()
	{
		g_kItemBindMgr.ClearAll();
	}
	void lwItemBindClearTargetItemInfo()
	{
		g_kItemBindMgr.ClearTargetItemInfo();
	}

	void RecvBindResult(BM::Stream& rkPacket)
	{// 자물쇠 사용 요청 결과
		HRESULT hResult;
		rkPacket.Pop(hResult);
		switch(hResult)
		{
		case S_OK:
			{
				std::string const kEff("ef_Bind");
				Notice_Show(TTW(790303), EL_Normal);
				g_kItemBindMgr.ShowCompleteEffect(kEff);
			}break;
		case E_INCORRECT_ITEM:
			{//잘못된 아이템정보
				_PgMessageBox("RecvUnbindResult","Incorrect item");
			}break;
		case E_INVALID_ITEMPOS:
			{//잘못된 아이템위치값
				_PgMessageBox("RecvUnbindResult","Invalid item pos");
			}break;
		case E_CANNOT_MODIFYITEM:
			{//아이템수정실패
				_PgMessageBox("RecvUnbindResult","Can\'t item modify");
			}break;
		case E_NOT_FOUND_ITEM:
			{//없는 아이템
				_PgMessageBox("RecvUnbindResult","Not found item");
			}break;
		}
	}

	// 열쇠 사용 관련 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	void UseUnbindItem(SItemPos const& rkItemInvPos, int const iDays)
	{// 열쇠 아이템 사용 
		if(!CallUnbindItemUI(iDays))
		{
			return;
		}
		g_kItemUnbindMgr.SetCashItemFromInv(rkItemInvPos);
	}

	bool CallUnbindItemUI(int iDays)
	{
		{
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_ITEM_BINDING"));
			if(pkWnd)
			{// 자물쇠 UI가 이미 떠있다면 열쇠 UI를 호출할수 없다.
				::Notice_Show(TTW(790316), EL_Warning);
				return false;
			}
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_ITEM_UNBINDING"));
		if( !pkWnd || pkWnd->IsClosed() )
		{
			pkWnd = XUIMgr.Call(_T("SFRM_ITEM_UNBINDING"));
			if( !pkWnd )
			{
				return false;
			}
		}
		XUI::CXUI_Wnd* pkTitleBG = pkWnd->GetControl(_T("SFRM_TITLE_BG"));
		if(pkTitleBG)
		{
			XUI::CXUI_Wnd* pkTitle = pkTitleBG->GetControl(_T("TITLE"));
			if(pkTitle)
			{
				pkTitle->Text(TTW(790300));
			}
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl( _T("LIST_TEXT")) );
		if(!pList)
		{
			return false;
		}
		POINT2 const DefaultSize(pList->Size().x - 25, pList->Size().y - 8);

		XUI::SListItem*	pItem = NULL;
		if( 0 == pList->GetTotalItemCount() )
		{
			pItem = pList->AddItem(_T(""));
		}
		else
		{
			pItem = pList->FirstItem();
		}
		if( !pItem )
		{
			return false;
		}

		XUI::CXUI_Wnd* pText = pItem->m_pWnd;
		pText->Size(DefaultSize);
		std::wstring kText = TTW(790306);

		{// 몇일 후에, 자물쇠가 풀리는지를 문장에 넣음
			size_t const NullStrSize = 1;
			size_t const DayStrSize = 3;
			size_t const BufSize = kText.size()+DayStrSize+NullStrSize;
			wchar_t* pBuff= new wchar_t[BufSize];
			::memset(pBuff,NULL, sizeof(wchar_t)*BufSize);
			::wsprintfW(pBuff, kText.c_str(), iDays);
			kText = pBuff;
			SAFE_DELETE_ARRAY(pBuff);
		}
		pText->Text(kText);

		XUI::CXUI_Style_String	kString = pText->StyleText();
		POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kString));
		//if( kTextSize.y > DefaultSize.y )
		//{
		pText->Size(kTextSize);
		//}
		//else
		//{
		//	pText->TextPos(POINT2(pText->TextPos().x, (DefaultSize.y - kTextSize.y) * 0.5f));
		//}
		pText->SetInvalidate();

		XUI::CXUI_AniBar* pkAniWnd = dynamic_cast<XUI::CXUI_AniBar*>( pkWnd->GetControl(_T("BAR_TIMER")) );
		if(!pkAniWnd)
		{
			_PgMessageBox("lwUIItemBind::CallUnbindItemUI", "Can't find \"BAR_TIMER\"");
			return false;
		}
		pkAniWnd->Max(0);
		pkAniWnd->Now(0);
		return true;
	}

	void lwItemUnbindDisplaySrcIcon(lwUIWnd kWnd)
	{
		g_kItemUnbindMgr.DisplayItem(kWnd.GetSelf());
	}

	bool lwItemUnbindSendReq()
	{// 서버로 열쇠 사용 요청을 보낼때
		if(true == g_kItemUnbindMgr.CheckOK())
		{// 모든 체크 항목이 정상적이라면 패킷을 보낸다
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION_UNBIND);
			kPacket.Push(g_kItemUnbindMgr.GetCashItemFromInv());		//열쇠 위치
			kPacket.Push(g_kItemUnbindMgr.GetTargetItemPosFromInv());	//대상 아이템 위치
			NETWORK_SEND(kPacket);
			return true;
		}
		return false;
	}

	bool lwItemUnbindCheckOK()
	{
		return g_kItemUnbindMgr.CheckOK();
	}

	void lwItemUnbindClearAll()
	{
		g_kItemUnbindMgr.ClearAll();
	}

	void lwItemUnbindClearTargetItemInfo()
	{
		g_kItemUnbindMgr.ClearTargetItemInfo();
	}

	void RecvUnbindResult(BM::Stream& rkPacket)
	{// 열쇠 아이템 사용 요청 결과
		HRESULT hResult;
		DWORD dwItemNo;
		rkPacket.Pop(hResult);		
		rkPacket.Pop(dwItemNo);
		
		switch(hResult)
		{
		case S_OK:
			{
				::Notice_Show(TTW(790308), EL_Normal);
				switch(dwItemNo)
				{
				case ITEM_NO_UNBIND_GOLD:
					{
						g_kItemUnbindMgr.ShowCompleteEffect(EFFECT_UNBIND_GOLD);
					}break;
				case ITEM_NO_UNBIND_PLATINUM:
					{
						g_kItemUnbindMgr.ShowCompleteEffect(EFFECT_UNBIND_PLATINUM);
					}break;
				default:
					{
						_PgMessageBox("RecvUnbindResult()", "Unknown type item");
					}break;
				}
			}break;
		case E_INCORRECT_ITEM:
			{//잘못된 아이템정보
				_PgMessageBox("RecvUnbindResult","Incorrect item");
			}break;
		case E_INVALID_ITEMPOS:
			{//잘못된 아이템위치값
				_PgMessageBox("RecvUnbindResult","Invalid item pos");
			}break;
		case E_CANNOT_MODIFYITEM:
			{//아이템수정실패
				_PgMessageBox("RecvUnbindResult","Can\'t item modify");
			}break;
		case E_NOT_FOUND_ITEM:
			{//없는 아이템
				_PgMessageBox("RecvUnbindResult","Not found item");
			}break;
		}
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

PgItemBindMgr::PgItemBindMgr()
{
	ClearAll();
}

PgItemBindMgr::~PgItemBindMgr()
{
	ClearAll();
}

void PgItemBindMgr::ClearAll()
{
	ClearTargetItemInfo();
	m_kCashItemPos.Clear();
	m_kCashItem.Clear();
}

void PgItemBindMgr::ClearTargetItemInfo()
{
	m_kTargetItemPos.Clear();
	m_kTargetItem.Clear();
}

void PgItemBindMgr::ClearAllandCloseUI()
 {
	ClearAll();	
	lwCloseUI("SFRM_ITEM_BINDING");	
 }

void PgItemBindMgr::SetTargetItemFromInv(SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player not exist!");		
		ClearAllandCloseUI();
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv) 
	{ 
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player doesn't have Inventory");
		ClearAllandCloseUI();
		return;
	}
	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(rkItemPos, kItem))
	{
		ClearAllandCloseUI();
		return;
	}
	if(true == kItem.IsEmpty())
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Empty Inv pos");
		ClearAllandCloseUI();
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return;
	}
		
	SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
	if(!pDef->CanEquip() || kEhtInfo.IsBinding())
	{// 장비 할 수 없거나, 
		unsigned long ulTextId = 790313;
		if(kEhtInfo.IsBinding())
		{//이미 자물쇠가 사용 되어있다면 경고 메세지를 하고
			ulTextId = 790312;
		}
		::Notice_Show(TTW(ulTextId), EL_Warning);
		// 얻어온 장비 할 수 없는 아이템의 정보를 지운다
		ClearTargetItemInfo();
		return;
	}

	m_kTargetItem = kItem;
	m_kTargetItemPos = rkItemPos;
}

void PgItemBindMgr::SetTargetItemPos(SItemPos const& rkItemPos)
{
	m_kTargetItemPos = rkItemPos;
}

void PgItemBindMgr::DisplayItem(XUI::CXUI_Wnd *pkWnd)
{
	if (!pkWnd)
	{
		_PgMessageBox("PgItemBindMgr::DisplayItem", "pkWnd is NULL");
		return;
	}

	if(false == CheckOK())
	{
		pkWnd->ClearCustomData();
		return;
	}
	
	DWORD const& dwItemNo = m_kTargetItem.ItemNo();
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	if(CItemDef const *pItemDef = kItemDefMgr.GetDef(dwItemNo))
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), pkWnd->TotalLocation());
	}
	pkWnd->SetCustomData(&dwItemNo, sizeof(dwItemNo));
}

bool PgItemBindMgr::CheckOK()
{
	if(true == m_kTargetItem.IsEmpty())
	{// 체크를 하려면 타겟 아이템이 존재해야 하고
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		_PgMessageBox("PgItemBindMgr::CheckOK", "Player not exist!");
		return false;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv) 
	{ 
		_PgMessageBox("PgItemBindMgr::CheckOK", "Player doesn't have Inventory");
		return false;
	}

	{// 자물쇠 아이템 위치를 수정 하거나, 자물쇠 아이템이 있어야 하고
		PgBase_Item kItem;
		if(S_OK != pkInv->GetItem(GetCashItemFromInv(), kItem))
		{
			ClearAllandCloseUI();
			return false;
		}
		BM::GUID const& kUseItemGuid = m_kCashItem.Guid();
		if(true == kUseItemGuid.IsNull())
		{
			ClearAllandCloseUI();
			return false;
		}
		if(kItem.Guid() != kUseItemGuid)
		{
			ClearAllandCloseUI();
			return false;
		}
	}

	// 기억하는 인벤 위치로 부터 아이템을 얻어오고
	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(GetTargetItemPosFromInv(), kItem))
	{// 해당 부분이 비어있다면
		ClearTargetItemInfo();
		return false;
	}
	// 자물쇠 UI에 올렸을때 대상이 위치가 변경되었거나, 없다면 
	// 기억하던 정보를 초기화 한다
	if(true == kItem.IsEmpty())
	{
		ClearTargetItemInfo();
		return false;
	}

	BM::GUID const& InvenItemGuid = kItem.Guid();
	if(GetTargetItemGuid() != InvenItemGuid)
	{
		ClearTargetItemInfo();
		return false;
	}
	if(true == InvenItemGuid.IsNull() )
	{
		ClearTargetItemInfo();
		return false;
	}
	if(kItem != m_kTargetItem)
	{
		ClearTargetItemInfo();
		return false;
	}
	return true;
}

void PgItemBindMgr::SetCashItemFromInv(SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player not exist!");
		ClearAllandCloseUI();
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv) 
	{ 
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player doesn't have Inventory");
		ClearAllandCloseUI();
		return;
	}
	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(rkItemPos, kItem))
	{
		ClearAllandCloseUI();
		return;
	}
	if(true == kItem.IsEmpty())
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Empty Inv pos");
		ClearAllandCloseUI();
		return;
	}

	m_kCashItem = kItem;
	m_kCashItemPos = rkItemPos;
} 

// 완료시 이펙트
bool PgItemBindMgr::ShowCompleteEffect(std::string const& kEffName)
{
	//ef_Bind, ef_Unbind01, ef_Unbind02
	PgActor* pPlayer = g_kPilotMan.GetPlayerActor();
	if(!pPlayer)
	{
		return false;
	}
	lwActor kPlayer(pPlayer);
	if(!kPlayer.AttachParticle(ACTOR_ATTACH_SLOT_BIND_EFFECT,ATTACH_POINT_STAR, kEffName.c_str()))
	{
		return false;
	}
	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
PgItemUnbindMgr::PgItemUnbindMgr()
{	
}

PgItemUnbindMgr::~PgItemUnbindMgr()
{
}

void PgItemUnbindMgr::SetTargetItemFromInv(SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player not exist!");		
		ClearAllandCloseUI();
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv) 
	{ 
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Player doesn't have Inventory");
		ClearAllandCloseUI();
		return;
	}
	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(rkItemPos, kItem))
	{
		ClearAllandCloseUI();
		return;
	}
	if(true == kItem.IsEmpty())
	{
		_PgMessageBox("PgItemBindMgr::SetTargetItemFromInv", "Empty Inv pos");
		ClearAllandCloseUI();
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return;
	}	
	SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
	if(!kEhtInfo.IsBinding() || kEhtInfo.IsUnbindReq())
	{// 자물쇠가 사용 된 아이템이 아니거나
		unsigned long ulTextId = 790307;
		if(kEhtInfo.IsUnbindReq())
		{// 이미 자물쇠 해제 요청이 되어있다면
			ulTextId =790311;
		}
		::Notice_Show(TTW(ulTextId), EL_Warning);
		// 얻어온 장비 할 수 없는 아이템의 정보를 지운다
		ClearTargetItemInfo();
		return;
	}
	m_kTargetItem = kItem;
	m_kTargetItemPos = rkItemPos;
}

void PgItemUnbindMgr::ClearAllandCloseUI()
{
	ClearAll();	
	lwCloseUI("SFRM_ITEM_UNBINDING");
}
