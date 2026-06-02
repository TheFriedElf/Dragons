#include "StdAfx.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "Lohengrin/ErrorCode.h "
#include "Variant/Inventory.H"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "lwUISealProcess.h"

DWORD const SEND_PACKET_DELAY_TIME = 100; // 0.1초

PgSealProcess::PgSealProcess()
: m_iScrollItemNo(0)
, m_dwPacketSendTime(0)
, m_bPacketSendDelayed(false)
, m_bSelectAll(false)
{}
PgSealProcess::~PgSealProcess()
{}

bool PgSealProcess::Init(int const iScrollItemNo)
{
	//if(!IsComplete())
	//{// 처리가 아직 끝나지 않았으면 사용 할수 없다
	//	return false;
	//}
	Clear();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgInventory* pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return false;
	}

	SItemPos kScrollPos;
	if(E_FAIL == pkInven->GetFirstItem(iScrollItemNo, kScrollPos))
	{
		return false;
	}
	PgBase_Item kScrollItem;
	if(E_FAIL == pkInven->GetItem(kScrollPos, kScrollItem))
	{
		return false;
	}
	// 사용하는 스크롤 아이템 번호 기억
	m_iScrollItemNo = iScrollItemNo;

	SSealScrollCnt kOwnedScrollCnt(kScrollItem.Count());
	m_kContScrollCnt.insert(std::make_pair(kScrollPos, kOwnedScrollCnt));
	
	while(S_OK == pkInven->GetNextItem(iScrollItemNo, kScrollPos))
	{// 이게 kScrollPos의 다음 iScrollItemNo를 가진 아이템 위치를 kScrollPos에 세팅하는게 맞는지 확인 필요
		if(S_OK == pkInven->GetItem(kScrollPos, kScrollItem))
		{// to do: 최초 사용된 스크롤 아이템은 이미 있으므로 등록 안되니 상관없다
			SSealScrollCnt kOwnedScrollCnt(kScrollItem.Count());
			m_kContScrollCnt.insert(std::make_pair(kScrollPos, kOwnedScrollCnt));
		}
	}
	return true;
}

void PgSealProcess::Clear()
{
	m_iScrollItemNo = 0;
	m_bPacketSendDelayed = false;
	m_dwPacketSendTime = 0;			//초기화 하는게 맞을까..
	SetSelectAll(false);
	m_kContScrollCnt.clear();
	m_kContSealPair.clear();
}

void PgSealProcess::CallSealProcessUI(DWORD const dwUseItemType, int const iItemNo)
{
	Init(iItemNo);
	//if(!Init(iItemNo))
	//{// 봉인, 봉인해제가 완료된 이후에 다시 열수 있다.
	//	::Notice_Show(TTW(790254), EL_Warning);
	//	return;
	//}

	unsigned long ulTitleTxtID=0;
	unsigned long ulExplainTxtID=406001;
	unsigned long ulNotFoundTxtID=406003;
	unsigned long ulItemNo = 0;
	bool isDoSeal = false;
	
	UIItemUtil::SUICT_Info kUICTInfo;
	if(!UIItemUtil::GetUICTInfo(iItemNo, kUICTInfo))
	{
		return;
	}
	
	switch(kUICTInfo.eType)
	{
	case UICT_SEAL:
		{// 봉인하기(캐시템)
			ulTitleTxtID = 790231;
			ulExplainTxtID = 790230;
			ulNotFoundTxtID = 790232;
			isDoSeal = true;
		}break;
	case UICT_SEAL_REMOVE:
		{
			switch(kUICTInfo.iCustomVal1)
			{
			case E_SEAL_REMOVE_GOOD:
				{// 고급 봉인해제(캐시템)
					ulTitleTxtID = 790235;
				}break;
			case E_SEAL_REMOVE_BEST:
				{// 최고급 봉인해제(캐시템)
					ulTitleTxtID = 790236;
				}break;
			case E_SEAL_REMOVE_NORMAL:
			default:
				{// 일반 봉인해제(소울 및 캐시템)
					ulTitleTxtID = 406002;
				}break;
			}
		}break;
	default:
		{
			return;
		}break;
	}

	XUI::CXUI_Wnd*	pkParent = XUIMgr.Call(_T("SFRM_SEAL_PROCESS"));
	if( !pkParent )
	{
		return;
	}
	// 봉인 및 봉인해제 스크롤 아이템 번호를 저장	
	pkParent->SetCustomData(&iItemNo, sizeof(iItemNo));	
		
	{// 타이틀 이름 설정
		XUI::CXUI_Wnd*	pkTitleBar =  pkParent->GetControl(_T("SFRM_TITLEBAR"));
		if( !pkTitleBar )
		{
			return;
		}
		XUI::CXUI_Wnd*	pkTitleName =  pkTitleBar->GetControl(_T("SFRM_NAME_BG"));
		if( !pkTitleName )
		{
			return;
		}
		pkTitleName->Text(TTW(ulTitleTxtID));		
	}
	{// 지시 문장 설정
		XUI::CXUI_Wnd*	pkBg = pkParent->GetControl(_T("SFRM_BACKGROUND"));
		if( !pkBg )
		{
			return;
		}
		XUI::CXUI_Wnd*	pkExplain = pkBg->GetControl(_T("IMG_ARROW"));
		if( !pkExplain )
		{
			return;
		}
		pkExplain->Text(TTW(ulExplainTxtID));
	}	
	
	XUI::CXUI_List*	pkList = dynamic_cast<XUI::CXUI_List*>(pkParent->GetControl(_T("LIST_SEAL_ITEMS")));
	if( !pkList )
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

	ContHaveItemNoCount	kEquipCount;
	if(E_FAIL == pkInven->GetItems(IT_EQUIP, kEquipCount))
	{
		return;
	}

	if(E_FAIL == pkInven->GetItems(IT_CASH, kEquipCount))
	{
		return;
	}

	if(!kEquipCount.size())
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);

	pkList->DeleteAllItem();
	ContHaveItemNoCount::iterator	it = kEquipCount.begin();
	bool bIsExist = false;
	while(it != kEquipCount.end())
	{
		SItemPos	rkPos;
		if(S_OK == pkInven->GetFirstItem(it->first, rkPos))
		{
			while(true)
			{
				PgBase_Item kItem;
				if(S_OK == pkInven->GetItem(rkPos, kItem))
				{
					CItemDef const * pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if(pItemDef && (true == pItemDef->CanEquip()))
					{
						if(false == (pItemDef->GetAbil(AT_ATTRIBUTE) & ICMET_Cant_Seal))
						{//봉인 관련 동작을 할수 있는 아이템이라면
							if(TargetItemAddtoUI(pkList, &kItem, rkPos, !isDoSeal))
							{// UI에 추가 한다
								bIsExist = true;
							}
						}
					}
				}

				if(E_FAIL == pkInven->GetNextItem(it->first, rkPos))
				{
					break;
				}
			}
		}
		++it;
	}

	if( !bIsExist )
	{
		lwCloseUI("SFRM_SEAL_PROCESS");
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", ulNotFoundTxtID, true);
	}
}

bool PgSealProcess::TargetItemAddtoUI(XUI::CXUI_Wnd* pWnd, PgBase_Item const* pkItem, SItemPos rkPos, bool const bFindSeal)
{
	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pWnd);
	if( !pkList )
	{
		return false;
	}

	SEnchantInfo const Info = pkItem->EnchantInfo();
	if(bFindSeal == Info.IsSeal())
	{// 봉인된 아이템, 또는 봉인 안된 아이템을 UI 리스트에 추가함
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
		if(!pItemDef)
		{
			return false;
		}

		XUI::SListItem* Item = pkList->AddItem(_T(""));
		if(Item)
		{
			XUI::CXUI_Wnd* pkItemWnd = Item->m_pWnd;
			pkItemWnd->SetCustomData(&rkPos, sizeof(rkPos));
			XUI::CXUI_Wnd* pkIcon = pkItemWnd->GetControl(_T("IMG_ICON"));
			if(pkIcon)
			{
				DWORD ItemNo = pkItem->ItemNo();
				pkIcon->SetCustomData(&ItemNo, sizeof(ItemNo));
			}

			XUI::CXUI_Wnd* pkName = pkItemWnd->GetControl(_T("SFRM_ITEM_NAME"));
			if(pkName)
			{
				std::wstring const*	pszName = NULL;
				if(GetDefString(pItemDef->NameNo(), pszName))
				{
					int const iWidth = pkName->Width()-30;
					Quest::SetCutedTextLimitLength(pkName, *pszName, _T("..."));					
				}
			}
		}

		return true;
	}
	return false;
}



bool PgSealProcess::AddItemtoSealProcess(TargetItemPos const& kTargetItemPos, ScrollItemPos const& kScrollPos)
{//같은 아이템 위치를 두번 add할수는 없게됨
	std::pair<CONT_SEALPROCESS_PAIR::iterator, bool> ret = m_kContSealPair.insert(std::make_pair(kTargetItemPos, kScrollPos));	
	return ret.second;	// true이면 등록된것, false이면 등록 안된것
}

bool PgSealProcess::RemoveItemtoSealProcess(TargetItemPos const& kTargetItemPos)
{
	CONT_SEALPROCESS_PAIR::iterator itor = m_kContSealPair.find(kTargetItemPos);
	if(itor != m_kContSealPair.end())
	{
		m_kContSealPair.erase(itor);		
		GetSealScrollPos(SItemPos(), true);	// 사용 스크롤 카운트 값 낮춤
		return true;
	}
	return false;
}

bool PgSealProcess::GetSealScrollPos(SItemPos& kScrollPos_out,  bool const bRelease)
{
	CONT_SCROLLCNT::iterator itor = m_kContScrollCnt.begin();
	while(itor != m_kContScrollCnt.end())
	{// 사용 가능한 스크롤의 위치를 얻어온다
		int const& iScrollMaxCnt = itor->second.iScrollMaxCnt;
		int& iUseScrollCnt = itor->second.iUseScrollCnt;
		if(!bRelease) 
		{// 스크롤 사용이면
			if(iScrollMaxCnt > iUseScrollCnt)
			{
				++iUseScrollCnt;
				kScrollPos_out = itor->first;
				return true;
			}
		}
		else
		{// 스크롤 사용해제 이면
			if(iUseScrollCnt > 0)
			{
				--iUseScrollCnt;
				//kScrollPos_out = itor->first;
				return true;
			}
		}		
		++itor;
	}
	return false;
}

bool PgSealProcess::SendPacket()
{
	if(!IsAbleToSendPacket())
	{// 패킷을 보낼 적당한 시간이 아니라면
		SetSendPacketDelayed(true);	// 딜레이 시켜야 한다
		return false;
	}
	else
	{
		SetSendPacketDelayed(false);
	}
	
	UIItemUtil::SUICT_Info kUICTInfo;
	if(!UIItemUtil::GetUICTInfo(m_iScrollItemNo, kUICTInfo))
	{
		return false;
	}

	BM::Stream kPacket;
	
	switch(kUICTInfo.eType)
	{
	case UICT_SEAL:
		{// 봉인 스크롤이거나
			kPacket.Push(PT_C_M_REQ_SEAL_ITEM);
		}break;
	case UICT_SEAL_REMOVE:
		{// 봉인 해제 스크롤이면
			kPacket.Push(PT_C_M_REQ_ITEM_DISCHARGE);
		}break;
	default:
		{
			return false;
		}break;
	}

	CONT_SEALPROCESS_PAIR::const_iterator itor = m_kContSealPair.begin();
	if(itor != m_kContSealPair.end())
	{// 패킷을 보냄
		ScrollItemPos const& kScrollPos = itor->second;
		TargetItemPos const& kTargetItemPos = itor->first;			
		kPacket.Push(kScrollPos);
		kPacket.Push(kTargetItemPos);
		NETWORK_SEND(kPacket)
		// 패킷 보낸 시간을 기억
		SetSendPacketTime();
	}
	return true;
}

void PgSealProcess::RcevPacket(WORD const& wPacketType, BM::Stream& rkPacket)
{
	DWORD dwTextID=0;
	switch(wPacketType)
	{
		case PT_M_C_ANS_ITEM_DISCHARGE:
		{
			dwTextID=670;
		}break;
		case PT_M_C_ANS_SEAL_ITEM:
		{
			dwTextID=790237;
		}break;
		default:
		{
			_PgMessageBox("PgSealProcess::RcevPacket", "not proper packet arrived");
			return;
		}break;
	}
	if(!IsComplete())
	{
		CONT_SEALPROCESS_PAIR::iterator itor = m_kContSealPair.begin();
		if(itor != m_kContSealPair.end())
		{// 응답을 받을때마다 하나씩 지우고
			m_kContSealPair.erase(itor);
			EItemDischargeResult Type;
			rkPacket.Pop(Type);
			if( IDR_SUCCESS != Type )
			{// 정상처리가 안되었다면, 봉인인지, 봉인 해제 인지 체크하여 적절한 TextID를 얻은후				
				DWORD dwTextID=0;
				if(ITEM_NO_SEAL_SCROLL == GetUsedScrollItemNo())
				{// 봉인이거나
					dwTextID = 790251;
				}
				else
				{// 봉인 해제 일때 메세지 설정
					dwTextID = 790253;
				}
				// 사용중이었던 스크롤, 대상 아이템 정보를 지우고
				//lwCloseUI("SFRM_SEAL_PROCESS");
				lwCloseUI("SFRM_NOTICE_SEAL_PROCESSING");				
				Clear();
				// 실패 메세지를 띄움
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", dwTextID, true);
			}
			else
			{
				itor = m_kContSealPair.begin();
				if(itor != m_kContSealPair.end())
				{
					SendPacket();
				}
				else
				{// 완료가 되면
					lwCloseUI("SFRM_NOTICE_SEAL_PROCESSING");
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", dwTextID + Type, true);
					Clear();
//					lwCloseUI("SFRM_SEAL_PROCESS");
				}
			}
		}
	}
}

void PgSealProcess::CallProcessingNoticeUI()
{//진행중이라는 UI띄움	
	XUI::CXUI_Wnd* pkNoticeWnd = XUIMgr.Call(L"SFRM_NOTICE_SEAL_PROCESSING", true);	//이게 모달이 아니면 큰일남.
	if(pkNoticeWnd)
	{
		XUI::CXUI_Wnd* pkBgWnd = pkNoticeWnd->GetControl(L"SFRM_COLOR");
		if(pkBgWnd)
		{
			XUI::CXUI_Wnd* TextWnd = pkBgWnd->GetControl(L"SFR_SDW");
			if(TextWnd)
			{
				if(ITEM_NO_SEAL_SCROLL == GetUsedScrollItemNo())
				{// 봉인 처리중 알림
					TextWnd->Text(TTW(790250));
				}
				else
				{// 봉인 해제중 알림
					TextWnd->Text(TTW(790252));
				}
			}
		}
	}
}

bool PgSealProcess::IsComplete()
{// 비어있으면 사용 가능
	return m_kContSealPair.empty();
}

void PgSealProcess::SetSendPacketTime()
{// 패킷을 보낸 시간을 기억
	m_dwPacketSendTime = BM::GetTime32();
}

bool PgSealProcess::IsAbleToSendPacket()
{
	if(SEND_PACKET_DELAY_TIME > BM::GetTime32() - m_dwPacketSendTime)
	{// 딜레이 되야 할 시간이 아직 안됐다면, 패킷 보낼 때가 아니다.
		return false;
	}
	return true;
}

bool PgSealProcess::SelectAll()
{	
	XUI::CXUI_Wnd*	pkMainWnd = XUIMgr.Get(L"SFRM_SEAL_PROCESS");
	if(!pkMainWnd)
	{
		return false;
	}
	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkMainWnd->GetControl(L"LIST_SEAL_ITEMS"));
	if(!pkListWnd)
	{
		return false;
	}	
	if(IsSelectAll())
	{// 전체 선택 해제 이면, 
		// 초기화 하고, 모든 체크 리스트를 푼다
		Init(GetUsedScrollItemNo());
		OffCheckAllListItem();
		return false;
	}

	// 전체 선택이면 각 대상 아이템들을 찾아 체크한다.
	SetSelectAll(true);
	XUI::SListItem* pItem = pkListWnd->FirstItem();
	while(NULL != pItem )
	{
		XUI::CXUI_Wnd* pkItemWnd = pItem->m_pWnd;
		if(pkItemWnd)
		{
			XUI::CXUI_Wnd* pkIconWnd = pkItemWnd->GetControl(L"IMG_ICON");
			if(pkIconWnd)
			{
				if(false == lwItemSealProcessListSelect(pkIconWnd, E_ON_CHECK))
				{// 처리할 스크롤이 부족하거나, 윈도우가 비정상적이면 중단
					break;
				}
			}
		}
		pItem = pkListWnd->NextItem(pItem);
	}
	return true;
}

void PgSealProcess::OffCheckAllListItem()
{
	XUI::CXUI_Wnd*	pkMainWnd = XUIMgr.Get(L"SFRM_SEAL_PROCESS");
	if(!pkMainWnd)
	{
		return;
	}
	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkMainWnd->GetControl(L"LIST_SEAL_ITEMS"));
	if(!pkListWnd)
	{
		return;
	}
	XUI::SListItem* pItem = pkListWnd->FirstItem();
	while(NULL != pItem )
	{// 리스트 아이템의 모든 체크 상태를 푼다
		XUI::CXUI_Wnd* pkItemWnd = pItem->m_pWnd;
		if(pkItemWnd)
		{
			XUI::CXUI_Wnd* pkIconWnd = pkItemWnd->GetControl(L"IMG_ICON");
			if(pkIconWnd)
			{
				XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkIconWnd->GetControl(L"CBTN_ITEM_SELECT"));
				if(pkChkBtn)
				{
					pkChkBtn->Check(false);
				}
			}
		}
		pItem = pkListWnd->NextItem(pItem);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
// 이하는 Lua로 노출 부분 

bool lwItemSealProcessListSelect(lwUIWnd kItem, int const iSelectType)
{
	if(true == kItem.IsNil()) 
	{
		return false;
	}
	lwUIWnd CheckWnd = kItem.GetControl("CBTN_ITEM_SELECT");
	if(false == CheckWnd.IsNil())
	{
		lwUIWnd kItemParent = kItem.GetParent();
		if(kItemParent.IsNil())
		{
			return false;
		}
		XUI::CXUI_Wnd* pWnd = kItemParent.GetSelf();
		if(!pWnd)
		{
			return false;
		}
		// 대상 아이템의 인벤 위치 얻기
		PgSealProcess::TargetItemPos kTargetItemPos;
		pWnd->GetCustomData(&kTargetItemPos, sizeof(kTargetItemPos));
			
		if(E_ON_CHECK != iSelectType
			&& true == CheckWnd.GetCheckState()
			)
		{// 대상 아이템 체크 해제 이면
			g_kSealProcess.RemoveItemtoSealProcess(kTargetItemPos);
			CheckWnd.CheckState(false);
		}
		else if(E_OFF_CHECK != iSelectType
				&& false == CheckWnd.GetCheckState()
				)
		{// 대상아이템 사용 설정
			SItemPos kScrollPos;
			if(g_kSealProcess.GetSealScrollPos(kScrollPos))
			{// 여유 스크롤이 존재하면, 서버 요청할 아이템 묶음에 저장하고
				g_kSealProcess.AddItemtoSealProcess(kTargetItemPos, kScrollPos);
				CheckWnd.CheckState(true);
			}
			else
			{// 여유 스크롤이 없다면 선택 실패를 알린다
				return false;
			}
		}
	}
	return true;
}

void lwItemSealProcessStop()
{
	g_kSealProcess.Clear();
}

bool lwItemSealProcessSendPacket(lwUIWnd Self)
{	
	XUI::CXUI_Wnd*	pkMainWnd = dynamic_cast<XUI::CXUI_Wnd*>(Self.GetSelf());
	if( !pkMainWnd )
	{
		return false;
	}	
	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkMainWnd->GetControl(L"LIST_SEAL_ITEMS"));
	if(!pkListWnd)
	{
		return false;
	}
	bool isAble = false;
	XUI::SListItem* pItem = pkListWnd->FirstItem();
	while(NULL != pItem )
	{
		XUI::CXUI_Wnd* pkItemWnd = pItem->m_pWnd;
		if(pkItemWnd)
		{
			XUI::CXUI_Wnd* pkIconWnd = pkItemWnd->GetControl(L"IMG_ICON");
			if(pkIconWnd)
			{	
				XUI::CXUI_CheckButton* pkChkBtn = 
					dynamic_cast<XUI::CXUI_CheckButton* >(pkIconWnd->GetControl(L"CBTN_ITEM_SELECT"));
				if(pkChkBtn)
				{
					if(true == pkChkBtn->Check())
					{
						isAble = true;
						break;
					}
				}
			}
		}
		pItem = pkListWnd->NextItem(pItem);
	}
	if(false == isAble)
	{// 아무것도 선택되지 않은 상태에서는 처리를 할 수 없다
		return false;
	}

	// 봉인 관련 스크롤 아이템 번호를 얻어와
	int iItemNo;
	pkMainWnd->GetCustomData(&iItemNo, sizeof(iItemNo));
	
	UIItemUtil::SUICT_Info kUICTInfo;
	if(!UIItemUtil::GetUICTInfo(iItemNo, kUICTInfo))
	{
		return false;
	}

	switch(kUICTInfo.eType)
	{
	case UICT_SEAL:
		{// 봉인하기 라면, 확인 창을 띄우고
			BM::Stream kEmpty;
			lwCallCommonMsgYesNoBox(MB(TTW(790234)), lwPacket(&kEmpty), true, MBT_CONFIRM_SEAL);
		}break;
	case UICT_SEAL_REMOVE:
		{// 봉인 해제라면, 패킷을 보내고 UI를 닫는다
			g_kSealProcess.SendPacket();
			lwCloseUI("SFRM_SEAL_PROCESS");
			g_kSealProcess.CallProcessingNoticeUI();
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void lwItemSealProcessIconDraw(lwUIWnd Self)
{
	if( Self.IsNil() ){ return; }

	XUI::CXUI_Image* pkWnd = dynamic_cast<XUI::CXUI_Image*>(Self.GetSelf());
	if( !pkWnd ){ return; }

	DWORD	dwItemNum = 0;
	pkWnd->GetCustomData(&dwItemNum, sizeof(dwItemNum));

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(dwItemNum);
	if( pItemDef && pItemDef->IsAmountItem() )
	{
		//
	}

	PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(dwItemNum);
	if( !pkSprite )
	{
		pkWnd->DefaultImgTexture(NULL);
		pkWnd->SetInvalidate();
		return;
	}

	PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
	if( !pkUVSprite ){ return; }

	pkWnd->DefaultImgTexture(pkUVSprite);
	SUVInfo& rkUV = pkUVSprite->GetUVInfo();
	pkWnd->UVInfo(rkUV);
	POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
	pkWnd->ImgSize(kPoint);
	pkWnd->SetInvalidate();
}

void lwItemSealProcessIconToolTip(lwUIWnd UISelf)
{
	if (UISelf.IsNil()) {	return; }
	XUI::CXUI_Wnd*	pWnd = UISelf.GetParent().GetSelf();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	SItemPos	rkPos;
	pWnd->GetCustomData(&rkPos, sizeof(rkPos));

	PgBase_Item kItem;
	if(S_OK == pkPlayer->GetInven()->GetItem(rkPos, kItem))
	{
		CallToolTip_SItem(&kItem, UISelf.GetTotalLocation());
	}
}

bool lwItemSealProcessWatchingDelayedPacket()
{
	if(g_kSealProcess.IsSendPacketDelayed() 
		&& g_kSealProcess.IsAbleToSendPacket()
		)
	{// 딜레이된 패킷이 있고, 보내도 되는 시간이라면 패킷을 보냄
		g_kSealProcess.SendPacket();
		return true;
	}
	return false;
}

bool lwItemSealProcessSelectAll()
{
	return g_kSealProcess.SelectAll();
}