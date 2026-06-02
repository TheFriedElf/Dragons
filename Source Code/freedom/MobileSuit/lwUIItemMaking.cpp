#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/ItemMakingDefMgr.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "lwUIItemMaking.h"
#include "PgEqItemMixCreator.h"
#include "PgItemMix_Script.h"

lwUIItemMaking::lwUIItemMaking(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemMaking::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	class_<lwUIItemMaking>(pkState, "ItemMakingWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "RefreshTodoList", &lwUIItemMaking::RefreshTodoList)
		.def(pkState, "DisplaySrcItem", &lwUIItemMaking::DisplaySrcItem)
		.def(pkState, "DisplayDestItem", &lwUIItemMaking::DisplayDestItem)
		.def(pkState, "DisplaySrcList", &lwUIItemMaking::DisplaySrcList)
		.def(pkState, "DisplayDestList", &lwUIItemMaking::DisplayDestList)
		.def(pkState, "Clear", &lwUIItemMaking::Clear)
		.def(pkState, "ClearItemCount", &lwUIItemMaking::ClearItemCount)
		.def(pkState, "SetItemCount", &lwUIItemMaking::SetItemCount)
		.def(pkState, "SelectTodoList", &lwUIItemMaking::SelectTodoList)
		.def(pkState, "SelectDestList", &lwUIItemMaking::SelectDestList)
//		.def(pkState, "GetCurTitleTextNo", &lwUIItemMaking::GetCurTitleTextNo)
		.def(pkState, "GetNeedMoney", &lwUIItemMaking::GetNeedMoney)
		.def(pkState, "MakingItem", &lwUIItemMaking::MakingItem)
	;
	return true;
}

void lwUIItemMaking::RefreshTodoList()
{
	XUI::CXUI_List* pkTodoList = dynamic_cast<XUI::CXUI_List*>(self);
	if (pkTodoList)
	{
		pkTodoList->ClearList();

		CONT_DEFITEMMAKING kDefItemMaking;
		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		if (!kItemMakingDefMgr.GetDefItemMaking(kDefItemMaking))
		{
			return;
		}

		CONT_DEFITEMMAKING::const_iterator itr = kDefItemMaking.begin();
		while(itr != kDefItemMaking.end())
		{
			const TBL_DEF_ITEMMAKING& rkMaking = (*itr).second;

			//const wchar_t *pTitleText = NULL;
			//if (rkMaking.iTitleStringNo)
			//	GetDefString(rkMaking.iTitleStringNo, pTitleText);
			const wchar_t *pContentText = NULL;
			if (rkMaking.iContentStringNo)
			{
				GetDefString(rkMaking.iContentStringNo, pContentText);
			}

			XUI::SListItem* pElement = 0;
			//if (pContentText)
			//{
			//	pElement = pkTodoList->AddItem(pTitleText);
			//}
			//else
			{
				pElement = pkTodoList->AddItem(L"");
			}

			if (!pElement)
			{
				return;
			}
			XUI::CXUI_Wnd *pBg = pElement->m_pWnd;

			if (pElement->m_pWnd)
			{
				XUI::CXUI_Wnd *pBtn = pElement->m_pWnd->GetControl(_T("BTN_TODO"));
				if(pBtn && pBg)
				{
					pBtn->SetCustomData(&rkMaking.iNo, sizeof(rkMaking.iNo));
					//if (pTitleText)
					//{
					//	pBg->Text(pTitleText);
					//}
				}
			}

			++itr;
		}
	}
}

void lwUIItemMaking::DisplaySrcItem()
{
	g_kItemMaking.DisplaySrcItem(self);
}

void lwUIItemMaking::DisplayDestItem()
{
	g_kItemMaking.DisplayDestItem(self);
}

void lwUIItemMaking::DisplaySrcList()
{
	g_kItemMaking.DisplaySrcList(self);
}

void lwUIItemMaking::DisplayDestList()
{
	g_kItemMaking.DisplayDestList(self);
}

void lwUIItemMaking::Clear()
{
	g_kItemMaking.Clear();
}
void lwUIItemMaking::ClearItemCount()
{
	g_kItemMaking.ClearItemCount();
}

bool lwUIItemMaking::SetItemCount(lwWString kItemCount)
{
	int iItemCount = _wtoi(kItemCount().c_str());
	return g_kItemMaking.SetItemCount(self, iItemCount);
}

void lwUIItemMaking::SelectTodoList()
{
	if (self)
	{
		int iMakingNo = 0;
		self->GetCustomData(&iMakingNo, sizeof(iMakingNo));
		g_kItemMaking.SelectTodoList(iMakingNo);
	}
}

void lwUIItemMaking::SelectDestList()
{
	g_kItemMaking.SelectDestList(self);
}

int lwUIItemMaking::GetNeedMoney()
{
	return g_kItemMaking.GetNeedMoney();
}

//int lwUIItemMaking::GetCurTitleTextNo()
//{
//	return g_kItemMaking.GetCurTitleTextNo();
//}

void lwUIItemMaking::MakingItem()
{
	g_kItemMaking.MakingItem();
}

////
//
PgItemMaking::PgItemMaking()
{
	Clear();
}

PgItemMaking::~PgItemMaking()
{
}

void PgItemMaking::Clear()
{
	CurMakingNo(0);
	m_kSrcItemPos.clear();
	m_kDestItemPos.clear();
	ClearItemCount();
	CurDestListIndex(0);
}

HRESULT PgItemMaking::IsCorrectItem(SItemPos const& rkItemPos, PgBase_Item &kOutItem)const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return E_FAIL;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(pInv)
	{
		PgBase_Item kItem;
		if(S_OK == pInv->GetItem(rkItemPos, kOutItem))
		{
			return S_OK;
		}
	}

	return E_FAIL;
}

bool PgItemMaking::SetSrcItem(const SItemPos &rkItemPos, int iBuildIndex)
{
	// 제조 리스트에서 선택 하지 않았으면 리턴.
	if (!CurMakingNo())
	{
		return false;
	}

	// 처음 여기로 들어와도 아이템이 표시 안된다. 왜냐하면 iItemCount이 0이기 때문.
	// 아이템을 슬롯에 놓고 다음 카운트를 입력하면 표시 될 것이다.
	ClearItemCount();
	int iWarnMessage = 0;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv)
	{
		return false;
	}

	PgBase_Item kItem;
	if(S_OK == pInv->GetItem(rkItemPos, kItem))
	{
		//DWORD dwItemNo = pItem->ItemNo();
		SItemMaking_ItemData kItemData;
		kItemData.kItem = kItem;
		kItemData.iBuildIndex = iBuildIndex;
		kItemData.kItemPos = rkItemPos;

		CONT_ITEMMAKING_ITEM::const_iterator find_itr = m_kSrcItemPos.find(kItemData.iBuildIndex);
		if (m_kSrcItemPos.end() != find_itr)
		{
			m_kSrcItemPos.erase(kItemData.iBuildIndex);
		}
		std::pair<CONT_ITEMMAKING_ITEM::iterator, bool> ret = m_kSrcItemPos.insert(std::make_pair(kItemData.iBuildIndex, kItemData));
		m_pkCurItemData = &((*ret.first).second);
	}
	else
	{
		// 그냥 클릭시 그냥 삭제.
		CONT_ITEMMAKING_ITEM::const_iterator find_itr = m_kSrcItemPos.find(iBuildIndex);
		if (m_kSrcItemPos.end() != find_itr)
		{
			m_kSrcItemPos.erase(iBuildIndex);
		}
		return false;
	}
	return true;
}

void PgItemMaking::DisplayItem(XUI::CXUI_Wnd *pWnd, const CONT_ITEMMAKING_ITEM& rkItemPos)const
{
	XUI::CXUI_Icon* pkSrcIcon = dynamic_cast<XUI::CXUI_Icon*>(pWnd);
	if (pkSrcIcon)
	{
		int iBuildIndex = pkSrcIcon->BuildIndex();
		CONT_ITEMMAKING_ITEM::const_iterator find_itr = rkItemPos.find(iBuildIndex);
		if (find_itr != rkItemPos.end())
		{
			SItemMaking_ItemData const& rkItemData = (*find_itr).second;
			bool bIsDraw = false;
			if (rkItemData.kItemPos != SItemPos::NullData())
			{
				PgBase_Item kItem;
				
				if(	S_OK == IsCorrectItem(rkItemData.kItemPos, kItem)
				&&	kItem == rkItemData.kItem 
				&&	rkItemData.iItemCount != 0)
				{
					bIsDraw = true;
				}
			}
			else
			{
				// ItemPos가 없는 경우는 OutPut의 아이템 슬롯.
				if (rkItemData.iItemCount != 0)
				{
					bIsDraw = true;
				}
			}

			if (bIsDraw)
			{
				const POINT2 rPT = pWnd->TotalLocation();
				SRenderTextInfo kRenderTextInfo;
				kRenderTextInfo.wstrText = (std::wstring)(BM::vstring(rkItemData.iItemCount));
				kRenderTextInfo.kLoc = rPT;
				kRenderTextInfo.wstrFontKey = FONT_TEXT;
				kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
				g_kUIScene.RenderText(kRenderTextInfo);
				
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(rkItemData.kItem.ItemNo());
				if(pItemDef)
				{
					g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false);
				}
			}
		}
	}
}

void PgItemMaking::DisplaySrcItem(XUI::CXUI_Wnd *pWnd)const
{
	DisplayItem(pWnd, m_kSrcItemPos);
}

void PgItemMaking::DisplayDestItem(XUI::CXUI_Wnd *pWnd)const
{
	DisplayItem(pWnd, m_kDestItemPos);
}

void PgItemMaking::DisplaySrcList(XUI::CXUI_Wnd *pWnd)const
{
	if (pWnd && g_kItemMaking.CurMakingNo())
	{
		const SDefItemMakingData& rkMakingData = g_kItemMaking.m_kCurMakingData;
		int const iNeedItemNo = rkMakingData.kNeedElements.aElement[pWnd->BuildIndex()];
		int const iNeedItemCount = rkMakingData.kNeedCount.aCount[pWnd->BuildIndex()];

		if (iNeedItemNo)
		{
			const wchar_t *pContentText = NULL;
			if (!GetDefString(iNeedItemNo, pContentText))
			{
				return;
			}

			int iCustomNeedCount = 0;
			pWnd->GetCustomData(&iCustomNeedCount, sizeof(iCustomNeedCount));

			// 아이템이 다르고, 카운트가 다르면. 다시 텍스트 적어준다.
			if (pWnd->Text().find(pContentText) == -1 || iCustomNeedCount != iNeedItemCount)
			{
				std::wstring wstrText = pContentText;
				wstrText += L" ";
				wstrText += (std::wstring)(BM::vstring(iNeedItemCount));
				wstrText += TTW(50413);
				pWnd->Text(wstrText);
				pWnd->SetCustomData(&iNeedItemCount, sizeof(iNeedItemCount));
			}
		}
		else if (pWnd->Text().size())
		{
			pWnd->Text(L"");
		}
	}
}

void PgItemMaking::DisplayDestList(XUI::CXUI_Wnd *pWnd)const
{
	if (pWnd && g_kItemMaking.CurMakingNo())
	{
		const SDefItemMakingData& rkMakingData = g_kItemMaking.m_kCurMakingData;

		// 아이템이 있으면. 띄워 주자.
		bool bIsCorrect = false;
		for(int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
		{
			int iItemNo = rkMakingData.akResultItemElements[pWnd->BuildIndex()].aElement[i];
			if (iItemNo)
			{
				bIsCorrect = true;
				break;
			}
		}

		int iIndex = pWnd->BuildIndex();
		if (bIsCorrect)
		{
			std::wstring wstrText = L"";
			wstrText += TTW(1503);
			wstrText += L" ";
			wstrText += (std::wstring)(BM::vstring(iIndex+1));
			pWnd->Text(wstrText);
			pWnd->Enable(true);
		}
		else
		{
			pWnd->Text(L"");
			pWnd->Enable(false);
		}
	}
	else if (pWnd && pWnd->Text().size())
	{
		pWnd->Text(L"");
	}
}

void PgItemMaking::ClearItemCount()
{
	m_pkCurItemData = 0;
}

bool PgItemMaking::SetItemCount(XUI::CXUI_Wnd *pWnd, int iItemCount)
{
	// CurrentSlot의 아이템의 수량 체크후 카운트를 올려준다.
	if (iItemCount && m_pkCurItemData)
	{
		if(iItemCount <= m_pkCurItemData->kItem.Count())
		{
			m_pkCurItemData->iItemCount = iItemCount;
			return true;
		}
	}

	return false;
}

void PgItemMaking::SelectTodoList(int iMakingNo)
{
	Clear();
	
	SDefItemMakingData kMakingData;
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, iMakingNo))
	{
		m_kCurMakingData = kMakingData;
		CurMakingNo(iMakingNo);
	}
}

void PgItemMaking::SelectDestList(XUI::CXUI_Wnd *pWnd)
{
	if (pWnd)
	{
		m_kDestItemPos.clear();
		CurDestListIndex(pWnd->BuildIndex());

		//int aElement[MAX_SUCCESS_RATE_ARRAY];//SuccessRateControl과 연관이 있으므로 숫자가 변할때는 한꺼번에
		for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY  ; i++)
		{
			int const iItemNo = m_kCurMakingData.akResultItemElements[CurDestListIndex()].aElement[i];
			int const iItemCount = m_kCurMakingData.akResultItemCount[CurDestListIndex()].aCount[i];

			PgBase_Item kItem;
			if(S_OK == CreateSItem(iItemNo, iItemCount, GIOT_MAKING, kItem))
			{
				SItemMaking_ItemData kItemData;
				kItemData.iBuildIndex = i;
				kItemData.kItem = kItem;
				kItemData.iItemCount = iItemCount;
				m_kDestItemPos.insert(std::make_pair(i, kItemData));
			}
		}
	}
}

void PgItemMaking::MakingItem()const
{
	// Get My Items.
	CONT_ITEMMAKING_ITEM kMySrcItem;	// ItemNo, ItemData
	CONT_ITEMMAKING_ITEM::const_iterator src_itr = m_kSrcItemPos.begin();
	while (src_itr != m_kSrcItemPos.end())
	{
		SItemMaking_ItemData const& rkItemData = (*src_itr).second;
		PgBase_Item kItem;
		if(S_OK == IsCorrectItem(rkItemData.kItemPos, kItem)
		&& kItem == rkItemData.kItem 
		&& rkItemData.iItemCount != 0)
		{
			kMySrcItem.insert(std::make_pair(kItem.ItemNo(), rkItemData));
		}
		++src_itr;
	}

	// 필요한 돈 검사.
	int iWarnMessage = 0;
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (GetNeedMoney() > iMoney)
		{
			iWarnMessage = 1500;
			goto __FAILED;
		}
	}

	{
/*===============================================================
		임시 :: 같은 아이템 생성 개수 처리
===============================================================*/
		int iTargetMakingItemCount = 1;
/*=============================================================*/

		BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
		int iMakingCount = 0;
		kPacket.Push(iMakingCount);	// 제조의 횟수	아래 SReqItemMaking의 갯수.
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

//		kReqItemMaking.WriteToPacket(kPacket);
/*
		for (int i=0 ; i<iMakingCount ; i++)
		{
			// Check Items
			SDefItemMakingData kMakingData;
			GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
			bool bRet = kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, CurMakingNo());
			if (!bRet)
			{
				iWarnMessage = 1502;	// 제조 번호가 이상함.
				goto __FAILED;
			}
			const TBL_DEF_ITEM_BAG_ELEMENTS& rkNeedElements = kMakingData.kNeedElements;
			const TBL_DEF_COUNT_CONTROL& rkNeedCount = kMakingData.kNeedCount;
			for (int iEle=0 ; iEle<MAX_SUCCESS_RATE_ARRAY ; ++iEle)
			{
				if (!rkNeedElements.aElement[iEle])
				{
					continue;
				}

				int iEleItemNo = rkNeedElements.aElement[iEle];
				int iEleCount = rkNeedCount.aCount[iEle] * iTargetMakingItemCount;

				CONT_ITEMMAKING_ITEM::const_iterator find_itr = kMySrcItem.find(iEleItemNo);
				if (find_itr == kMySrcItem.end())
				{
					iWarnMessage = 1501;		// 재료가 이상함.
					goto __FAILED;
				}

				SItemMaking_ItemData const& rkItemData = (*find_itr).second;
				PgBase_Item kItem;
				
				if(	S_OK != IsCorrectItem(rkItemData.kItemPos, kItem)
				|| kItem != rkItemData.kItem)
				{
					iWarnMessage = 1501;		// 재료가 이상함.
					goto __FAILED;
				}

				// 재료 초과.
				int iResItemCount = rkItemData.iItemCount * iTargetMakingItemCount;
				if (iResItemCount != iEleCount)
				{
					iWarnMessage = 1501; 		// 재료가 이상함.
					goto __FAILED;
				}
			}
			// end of check items.

			SReqItemMaking kReqItemMaking;
			kReqItemMaking.iMakingNo = CurMakingNo();
			kReqItemMaking.iMakingCount = iTargetMakingItemCount;	// 같은 아이템을 여러개 만들경우 이 수치를 변경하면된다.

			CONT_ITEMMAKING_ITEM::const_iterator itr = m_kSrcItemPos.begin();
			while (itr != m_kSrcItemPos.end())
			{
				SItemMaking_ItemData const& rkItemData = (*itr).second;
				PgBase_Item kItem;
				if(S_OK == IsCorrectItem(rkItemData.kItemPos, kItem)
				&& kItem == rkItemData.kItem 
				&& rkItemData.iItemCount != 0)
				{
					SReqItemMaking_ItemSlot kItemSlot;
					kItemSlot.kItemPos = rkItemData.kItemPos;
					kItemSlot.iItemCount = rkItemData.iItemCount * iTargetMakingItemCount;
					kReqItemMaking.kItemSlotCont.push_back(kItemSlot);
				}
				++itr;
			}
		}
		*/

		NETWORK_SEND(kPacket)
	}

	return;

__FAILED:
	if(iWarnMessage)
	{
		lwAddWarnDataTT(iWarnMessage);
	}
	return;
}

int PgItemMaking::GetNeedMoney()const
{
	if (CurMakingNo())
	{
		return m_kCurMakingData.iNeedMoney;
	}
	return 0;
}

//int PgItemMaking::GetCurTitleTextNo()const
//{
//	if (CurMakingNo())
//	{
//		return m_kCurMakingData.iTitleStringNo;
//	}
//	return 0;
//}

PgBase_Item const* PgItemMaking::GetMouseOverItem(XUI::CXUI_Icon *pIcon, const CONT_ITEMMAKING_ITEM& rkItemPos)const
{
	const SIconInfo &kIconInfo = pIcon->IconInfo();
	int iKey = kIconInfo.iIconKey;
	CONT_ITEMMAKING_ITEM::const_iterator itr = rkItemPos.find(iKey);
	if (itr != rkItemPos.end())
	{
		SItemMaking_ItemData const& rkData = (*itr).second;

		return &rkData.kItem;
	}
	return 0;
}

PgBase_Item const* PgItemMaking::GetMouseOverSrcItem(XUI::CXUI_Icon *pIcon)const
{
	return GetMouseOverItem(pIcon, m_kSrcItemPos);
}

PgBase_Item const* PgItemMaking::GetMouseOverDestItem(XUI::CXUI_Icon *pIcon)const
{
	return GetMouseOverItem(pIcon, m_kDestItemPos);
}

void PgItemMaking::Recv_PT_M_C_NFY_ITEM_MAKING_RESULT(BM::Stream* pkPacket)
{//	CIE_Make 타입으로 오는 패킷들인데. 일부는 Result 패킷이 없는 경우도 있어서, 문제가 될 수도 있다.
	int iMakingCount = 0;
	int iMakingNo = 0;
	int iMakingResult = 0;
	int iMakingRequestCount = 0;	// 생성 요청 갯수
	CONT_RESULT_ITEM_DATA kContResultItems;
	int bIsMazeItem = 0;

	PU::TLoadTable_MM(*pkPacket, kContResultItems);
	pkPacket->Pop(bIsMazeItem);

	if(!kContResultItems.size())
	{// 오류
		if(EMAKING_TYPE_MONSTER_CARD_TYPE2 == (bIsMazeItem&EMAKING_TYPE_MONSTER_CARD_TYPE2))
		{
			XUIMgr.Call(L"FRM_MONSTER_CARD_MIX_TYPE2_FAILED", true);
			lwItemMix::SetMonsterCardMixerType2MixItem();
		}
		else
		{
			XUIMgr.Call(L"FRM_MIX_CREATOR_FAILED", true);
		}
		return;
	}

	// 제조 갯수가 하나면.. 하나 전용 UI를 띄운다.
	GET_DEF(CItemDefMgr, kItemDefMgr);
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	if(kContResultItems.size() == 1)//하나면 전용 UI.
	{
		CONT_RESULT_ITEM_DATA::const_iterator items_itor = kContResultItems.begin();
		while(items_itor != kContResultItems.end())
		{
			PgBase_Item const &kItem = (*items_itor).second;
			
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if (!pItemDef)
			{
				return;
			}
		
			CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pItemDef->ResNo());
			if(itrDest == pContDefRes->end())
			{
				// 아이템 테이블에서 못 찾으면 패스!
				return ;
			}

			// 아웃풋이 하나밖에 없으니. 지정 UI를 띄우자.
			if(EMAKING_TYPE_MAZE_ITEM == (bIsMazeItem&EMAKING_TYPE_MAZE_ITEM))
			{
				XUI::CXUI_Wnd* pMain = XUIMgr.Activate(L"FRM_MIX_CREATOR_SUCCESS", true);
				if (!pMain)
				{
					return;
				}
				XUI::CXUI_Wnd* pIcon = pMain->GetControl(L"ICN_ITEM");
				if (!pIcon)
				{
					return;
				}
				lwUIWnd(pIcon).SetCustomDataAsSItem(kItem);
				PgUIUtil::DrawIconToItemNo(pIcon, kItem.ItemNo(), 1.5f);

				const wchar_t *pText = NULL;
				std::wstring wstrText;
				GetDefString(kItem.ItemNo(), pText);


				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf_s(szTemp, MAX_PATH, TTW(50712).c_str(), pText);

				pMain->Text(szTemp);
			}
			else if(EMAKING_TYPE_MONSTER_CARD_TYPE2 == (bIsMazeItem&EMAKING_TYPE_MONSTER_CARD_TYPE2))
			{
				lwItemMix::OnCallMonsterCardMixerType2Result(pItemDef, pContDefRes, kItem);
				lwItemMix::SetMonsterCardMixerType2MixItem();
			}
			else
			{
				lwUIWnd kResultUI = lwActivateUI("FRM_ITEM_MAKING_RESULT_SINGLE");
				if (!kResultUI())
				{
					return ;
				}
				lwUIWnd kImgUI = kResultUI.GetControl("IMG_GET");
				if (!kImgUI())
				{
					return;
				}

				const TBL_DEF_RES& rkRes = (*itrDest).second;
				std::string strIconPath = MB(rkRes.strIconPath);
				kImgUI.ChangeImage( (char*)strIconPath.c_str(), false );
				kImgUI.SetUVIndex(rkRes.UVIndex);
				kImgUI.SetCustomData<int>(kItem.ItemNo());

				std::wstring wstrText;
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if( pDef )
				{
					const wchar_t *pText = NULL;
					if( GetDefString(pDef->NameNo(), pText) )
					{
						wstrText += pText;
						wstrText += _T("\n");
						if( !pDef->CanEquip() )
						{
							wstrText += (std::wstring)(BM::vstring(kItem.Count()));
							wstrText += TTW(1709);		// '개'
							wstrText += _T(" ");
						}
						wstrText += TTW(1600);
					}
				}
				kResultUI()->Text(wstrText);
			}
			++items_itor;
		}
	}
	else
	{// 메세지라도 좀 뿌려줘라.
	}
}
