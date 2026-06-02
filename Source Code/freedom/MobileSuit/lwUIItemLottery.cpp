#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgUIScene.h"
#include "PgNetwork.h"
#include "Variant/Item.h"
#include "ServerLib.h"
#include "PgUICalculator.h"
#include "Variant/ItemMakingDefMgr.h"
#include "Variant/ItemBagMgr.h"
#include "lwUIItemLottery.h"

lwUIItemLottery::lwUIItemLottery(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemLottery::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	class_<lwUIItemLottery>(pkState, "ItemLotteryWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "DisplaySrcItem", &lwUIItemLottery::DisplaySrcItem)
		.def(pkState, "SendReqItemLottery", &lwUIItemLottery::SendReqItemLottery)
		.def(pkState, "Clear", &lwUIItemLottery::Clear)
		.def(pkState, "IsAvailable", &lwUIItemLottery::IsAvailable)		
	;
	return true;
}

void lwUIItemLottery::DisplaySrcItem()
{
	if (!self)
	{
		return;
	}

	lwUIWnd kUI(self);
	g_kItemLottery.DisplaySrcItem(self, kUI.GetBuildIndex());
}
void lwUIItemLottery::SendReqItemLottery(bool bIsSend)
{
	g_kItemLottery.SendReqItemLottery(bIsSend);
}
void lwUIItemLottery::Clear()
{
	g_kItemLottery.Clear();
}
bool lwUIItemLottery::IsAvailable()
{
	return g_kItemLottery.IsAvailable();
}


// ---------------------------------------
// PgItemLottery
PgItemLottery::PgItemLottery()
{
	//Clear();
}

PgItemLottery::~PgItemLottery()
{
}

void PgItemLottery::Clear()
{
	m_kLotteryElement.clear();
}

bool PgItemLottery::DragToSlot(const SItemPos &rkItemPos, int iBuildIndex)
{
	CONT_LOTTERYELEMENT::const_iterator itr = m_kLotteryElement.find(iBuildIndex);
	if (itr != m_kLotteryElement.end())
	{
		m_kLotteryElement.erase(iBuildIndex);
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgBase_Item kItem;
	
	if(S_OK == pkPlayer->GetInven()->GetItem(rkItemPos, kItem))
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		int iMakingType = kItemDefMgr.GetAbil(kItem.ItemNo(), AT_MAKING_TYPE);
		if (iMakingType != EMAKING_TYPE_LOTTERY)
		{
			// 뽑기템이 아닙니다.
			lwAddWarnDataTT(1653);
			return false;
		}

		CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		SCalcInfo kInfo;

		kInfo.eCallType = CCT_LOTTERY;
		kInfo.iMaxValue = kItem.Count();
		kInfo.iItemNo = kItem.ItemNo();
		kInfo.iIndex = iBuildIndex;
		kInfo.kItemPos = rkItemPos;

		CallCalculator(kInfo);
		return true;
	}

	return false;
}

bool PgItemLottery::SetSrcItem(const SItemPos &rkItemPos, int iBuildIndex, int iItemCount)
{
	if (!iItemCount)
	{
		// 아이템 수량을 넣어 주세요!
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(pInv)
	{
		PgBase_Item kItem;
		
		if(S_OK != pInv->GetItem(rkItemPos, kItem))
		{
			return false;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		int iMakingType = kItemDefMgr.GetAbil(kItem.ItemNo(), AT_MAKING_TYPE);
		if (iMakingType != EMAKING_TYPE_LOTTERY)
		{
			// 뽑기템이 아닙니다.
			lwAddWarnDataTT(1653);
			return false;
		}

		SLotteryElement kElement;
		kElement.kItem = kItem;
		kElement.iItemCount = iItemCount;
		kElement.kItemPos = rkItemPos;
		kElement.kSrcItem = kItem.Guid();

		// 이미 등록 되어 있는지 확인!!
		CONT_LOTTERYELEMENT::const_iterator itr = m_kLotteryElement.begin();
		while(itr != m_kLotteryElement.end())
		{
			SLotteryElement const& rkElement = (*itr).second;
			if (rkElement.kItem.ItemNo() == kItem.ItemNo())
			{
				// 이미 등록 되어 있습니다.
				lwAddWarnDataTT(1660);
				return false;
			}
			++itr;
		}

		m_kLotteryElement.insert(std::make_pair(iBuildIndex, kElement));

		return true;
	}

	lwAddWarnDataTT(1653);
	return false;
}

void PgItemLottery::DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;
	XUI::CXUI_Wnd *pCountNameWnd = NULL;
	BM::GUID kSrcItem;

	if (!pWnd)
	{
		return;
	}

	pFormWnd = pWnd->Parent();
	if (!pFormWnd)
	{
		return;
	}
	pShadowWnd = pFormWnd->Parent();
	assert(pShadowWnd);
	if(pShadowWnd)
	{
		pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_MATERIAL_NAME"));
		pCountNameWnd = pShadowWnd->GetControl(_T("SFRM_MATERIAL_COUNT"));
	}
	
	CONT_LOTTERYELEMENT::iterator itr = m_kLotteryElement.find(iBuildIndex);
	if (itr == m_kLotteryElement.end())
	{
		goto __HIDE;
	}
	SLotteryElement& rkElement = (*itr).second;
	PgBase_Item const &kItem = rkElement.kItem;
	int iItemCount = rkElement.iItemCount;
	kSrcItem = rkElement.kSrcItem;

	if(!kItem.ItemNo()){goto __HIDE;}
	if(kItem.Guid() != kSrcItem){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	{
		const POINT2 rPT =	pWnd->TotalLocation();
		int const iNeedItemNo = kItem.ItemNo();
		int const iNeedItemCount = iItemCount;

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iNeedItemNo);
		if(pItemDef)
		{
			g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
		}
		//pWnd->SetCustomData(&iNeedItemNo, sizeof(iNeedItemNo));

		const wchar_t *pName = NULL;
		if(	GetItemName(iNeedItemNo, pName))
		{
			PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
			assert(pkPilot);
			if(!pkPilot){return;}
			
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			assert(pkPlayer);
			if(!pkPlayer){return;}
			
			int const iMyCount = pkPlayer->GetInven()->GetTotalCount(iNeedItemNo);
			if (iMyCount<iItemCount)
			{
				iItemCount = iMyCount;
				rkElement.iItemCount = iMyCount;
			}
			
			std::wstring wstrText(pName);
			BM::vstring vNameStr(wstrText);

			assert(pSrcNameWnd);

			if(pSrcNameWnd)
			{
				pSrcNameWnd->Text((const wchar_t*)vNameStr);//리소스 아이템 이름 디스플레이.
			}
			if(pCountNameWnd)
			{
				BM::vstring vstrCount;
				vstrCount += iItemCount;
				vstrCount += _T(" / ");
				vstrCount += TTW(1303);
				vstrCount += iMyCount;
				pCountNameWnd->Text((const wchar_t*)vstrCount);//리소스 아이템 이름 디스플레이.
			}
		}
	}
	
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
		}
		if(pCountNameWnd)
		{
			pCountNameWnd->Text(_T(""));
		}

		m_kLotteryElement.erase(iBuildIndex);
	}
}

bool PgItemLottery::IsAvailable()const
{
	if (!m_kLotteryElement.size())
	{
		// 샤인스톤을 뽑기 슬롯에 올려놔야 합니다.
		lwAddWarnDataTT(1658);
		return false;
	}

	XUI::CXUI_Wnd *pkWnd = NULL;
	if(XUIMgr.IsActivate(L"FRM_LOTTERY_REQUEST", pkWnd))//이미 있는가?
	{
		return false;
	}

	if (!SendReqItemLottery(false))
	{
		return false;
	}

	return true;
}

bool PgItemLottery::SendReqItemLottery(bool bIsSend)const
{
	int iWarnMessage = 0;
	SReqItemMaking kReqItemMaking;

	// Get Def Making
	CONT_DEFITEMMAKING kDefItemMaking;
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (!kItemMakingDefMgr.GetDefItemMaking(kDefItemMaking))
	{
		iWarnMessage = 1502;	// 제조 번호가 이상함.
		goto __FAILED;
	}

	{
		CONT_DEFITEMMAKING kDefResult;
		// find result
		CONT_DEFITEMMAKING::const_iterator itr = kDefItemMaking.begin();
		GET_DEF(CItemBagMgr, kItemContainerMgr);
		while(itr != kDefItemMaking.end())
		{
			const TBL_DEF_ITEMMAKING& rkMaking = (*itr).second;

			TBL_DEF_ITEM_BAG_ELEMENTS kNeedElements;
			TBL_DEF_COUNT_CONTROL kNeedCount;
		
			if (kItemContainerMgr.GetItemBagElements(rkMaking.iNeedElementsNo, kNeedElements) != S_OK)
			{
				++itr;
				continue;
			}
			if (kItemContainerMgr.GetCountControl(rkMaking.iNeedCountControlNo, kNeedCount) != S_OK)
			{
				++itr;
				continue;
			}

			bool bIsSuccess = false;
			for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
			{
				int iSrcEleNo = kNeedElements.aElement[i];
				int iSrcEleCount = kNeedCount.aCount[i];
				// Need Data
				if (!iSrcEleNo || !iSrcEleCount)
				{
					continue;
				}
				bIsSuccess = true;

				CONT_LOTTERYELEMENT::const_iterator find_itr = m_kLotteryElement.begin();
				while(find_itr != m_kLotteryElement.end())
				{
					SLotteryElement const& rkInsertEle = (*find_itr).second;
					if (rkInsertEle.kItem.ItemNo() == iSrcEleNo)
					{
						break;
					}
					++find_itr;
				}
				if (find_itr == m_kLotteryElement.end())
				{
					bIsSuccess = false;
					break;
				}
				SLotteryElement const& rkInsertEle = (*find_itr).second;
				if (rkInsertEle.iItemCount < iSrcEleCount)
				{
					bIsSuccess = false;
					break;
				}
			}

			if (bIsSuccess)
			{
				// 찾았음.
				kDefResult.insert(std::make_pair((*itr).first, (*itr).second));
			}
			++itr;
		}

		CONT_DEFITEMMAKING::const_iterator res_itr = kDefResult.end();
		if (!kDefResult.size())
		{
			// 제조 할게 없다.
			iWarnMessage = 1502;	// 제조 번호가 이상함.
			goto __FAILED;
		}

		{	// choose one
			// 마지막 결론.
			__int64 iResultPrice = 0;
			CONT_DEFITEMMAKING::iterator def_itr = kDefResult.begin();
			GET_DEF(CItemBagMgr, kItemContainerMgr);
			while(def_itr != kDefResult.end())
			{
				const TBL_DEF_ITEMMAKING& rkMaking = (*def_itr).second;

				TBL_DEF_ITEM_BAG_ELEMENTS kNeedElements;
				TBL_DEF_COUNT_CONTROL kNeedCount;
				if (kItemContainerMgr.GetItemBagElements(rkMaking.iNeedElementsNo, kNeedElements) != S_OK)
				{
					++def_itr;
					continue;
				}
				if (kItemContainerMgr.GetCountControl(rkMaking.iNeedCountControlNo, kNeedCount) != S_OK)
				{
					++def_itr;
					continue;
				}

				__int64 iPrice = 0;
				GET_DEF(CItemDefMgr, kItemDefMgr);
				for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
				{
					int iSrcEleNo = kNeedElements.aElement[i];
					int iSrcEleCount = kNeedCount.aCount[i];
					if (!iSrcEleNo || !iSrcEleCount)
					{
						continue;
					}
					CItemDef const *pItemDef = kItemDefMgr.GetDef(iSrcEleNo);
					iPrice += (pItemDef->BuyPrice() * iSrcEleCount);
				}
				if (iPrice > iResultPrice)
				{
					iResultPrice = iPrice;
					res_itr = def_itr;
				}
				++def_itr;
			}
		}

		if (res_itr == kDefResult.end())
		{
			// 제조 할게 없다.
			iWarnMessage = 1502;	// 제조 번호가 이상함.
			goto __FAILED;
		}
		const TBL_DEF_ITEMMAKING& rkResult = (*res_itr).second;

		CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
		if(!pkUnit)
		{
			goto __FAILED;
		}

		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (rkResult.iNeedMoney > iMoney)
		{
			// 비용 부족.
			iWarnMessage = 1657;
			goto __FAILED;
		}

/*
		{
			kReqItemMaking.iMakingNo = rkResult.iNo;
			kReqItemMaking.iMakingCount = 1;
			kReqItemMaking.bAllUseItem = true;

			CONT_LOTTERYELEMENT::const_iterator element_itr = m_kLotteryElement.begin();
			while(element_itr != m_kLotteryElement.end())
			{
				SLotteryElement const& rkElement = (*element_itr).second;

				// InsertSource
				SReqItemMaking_ItemSlot kItemSlot;
				kItemSlot.kItemPos = rkElement.kItemPos;
				kItemSlot.iItemCount = rkElement.iItemCount;
				kReqItemMaking.kItemSlotCont.push_back(kItemSlot);

				++element_itr;
			}
		}
*/

		if (bIsSend)
		{
			BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
			kPacket.Push(1);	// 뽑기 한번 하기.
			kReqItemMaking.WriteToPacket(kPacket);
			kPacket.Push(SItemPos());
			kPacket.Push(SItemPos());

			NETWORK_SEND(kPacket)
		}
	}
	return true;

__FAILED:
	if(iWarnMessage)
	{
		lwAddWarnDataTT(iWarnMessage);
	}
	return false;
}