#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgUIScene.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "Variant/Item.h"
#include "Variant/ItemBagMgr.h"
#include "Variant/ItemMakingDefMgr.h"
#include "lwUIShineStone.h"

lwUIShineStoneUpgrade::lwUIShineStoneUpgrade(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIShineStoneUpgrade::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	class_<lwUIShineStoneUpgrade>(pkState, "ShineStoneUpgradeWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "Clear", &lwUIShineStoneUpgrade::Clear)
		.def(pkState, "SetDefItemMakingNo", &lwUIShineStoneUpgrade::SetDefItemMakingNo)
		.def(pkState, "SelectItem", &lwUIShineStoneUpgrade::SelectItem)
		.def(pkState, "DisplayTodoItem", &lwUIShineStoneUpgrade::DisplayTodoItem)
		.def(pkState, "DisplayDetailSrcItem", &lwUIShineStoneUpgrade::DisplayDetailSrcItem)
		.def(pkState, "DisplayDetailDestItem", &lwUIShineStoneUpgrade::DisplayDetailDestItem)
		.def(pkState, "MouseOver", &lwUIShineStoneUpgrade::MouseOver)
		.def(pkState, "MouseOut", &lwUIShineStoneUpgrade::MouseOut)
		.def(pkState, "DisplayDetailText", &lwUIShineStoneUpgrade::DisplayDetailText)		
		.def(pkState, "SendReqUpgrade", &lwUIShineStoneUpgrade::SendReqUpgrade)		
	;
	return true;
}

void lwUIShineStoneUpgrade::Clear()
{
	g_kShineStoneUpgrade.Clear();
}
void lwUIShineStoneUpgrade::SetDefItemMakingNo(int iIdx, int iMakingNo)
{
	g_kShineStoneUpgrade.SetDefItemMakingNo(iIdx, iMakingNo);
}
void lwUIShineStoneUpgrade::DisplayTodoItem()
{
	if (!self)
	{
		return;
	}
	lwUIWnd kUI(self);
	g_kShineStoneUpgrade.DisplayTodoItem(self, kUI.GetBuildIndex());
}
void lwUIShineStoneUpgrade::DisplayDetailSrcItem()
{
	g_kShineStoneUpgrade.DisplayDetailSrcItem(self);
}
void lwUIShineStoneUpgrade::DisplayDetailDestItem()
{
	g_kShineStoneUpgrade.DisplayDetailDestItem(self);
}
void lwUIShineStoneUpgrade::SelectItem()
{
	g_kShineStoneUpgrade.SelectItem(self, self->BuildIndex());
}
void lwUIShineStoneUpgrade::MouseOver()
{
	g_kShineStoneUpgrade.MouseOver(self, self->BuildIndex());
}
void lwUIShineStoneUpgrade::MouseOut()
{
	g_kShineStoneUpgrade.MouseOut();
}
void lwUIShineStoneUpgrade::DisplayDetailText()
{
	g_kShineStoneUpgrade.DisplayDetailText(self);
}
void lwUIShineStoneUpgrade::SendReqUpgrade()
{
	g_kShineStoneUpgrade.SendReqUpgrade();
}


// ---------------------------------------
// PgShineStoneUpgrade
PgShineStoneUpgrade::PgShineStoneUpgrade()
{
	//Clear();
}

PgShineStoneUpgrade::~PgShineStoneUpgrade()
{
}

void PgShineStoneUpgrade::Clear()
{
	m_iSelectIndex = -1;
}

void PgShineStoneUpgrade::SetDefItemMakingNo(int iIdx, int iMakingNo)
{
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(iIdx);
	if (itr != m_kUpgradeElement.end())
	{
		m_kUpgradeElement.erase(iIdx);
	}

	SDefItemMakingData kData;
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (!kItemMakingDefMgr.GetDefItemMakingInfo(kData, iMakingNo))
	{
		return;
	}

	SUpgradeElement kElement;
	// Get need item, result item
	for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
	{
		if (kData.kNeedElements.aElement[i] && kData.kNeedCount.aCount[i])
		{
			kElement.iItemNo = kData.kNeedElements.aElement[i];
			kElement.iItemCount = kData.kNeedCount.aCount[i];
			break;
		}
	}

	TBL_DEF_ITEM_BAG_ELEMENTS kNeedElements;
	for (int i=0 ; i<MAX_ITEMMAKING_ARRAY ; i++)
	{
		for (int j=0 ; j<MAX_SUCCESS_RATE_ARRAY ; j++)
		{
			if (kData.akResultItemElements[i].aElement[j] && kData.akResultItemCount[i].aCount[j])
			{
				kElement.iResultItemNo = kData.akResultItemElements[i].aElement[j];
				kElement.iResultItemCount = kData.akResultItemCount[i].aCount[j];
				break;
			}
		}
	}

	kElement.iMakingNo = iMakingNo;
	m_kUpgradeElement.insert(std::make_pair(iIdx, kElement));
}

void PgShineStoneUpgrade::DisplayTodoItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;
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
	}
	
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(iBuildIndex);
	if (itr == m_kUpgradeElement.end())
	{
		goto __HIDE;
	}


	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	assert(pkPilot);
	if(!pkPilot){return;}
	
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	assert(pkPlayer);
	if(!pkPlayer){return;}

	const SUpgradeElement& rkElement = (*itr).second;
	if(!rkElement.iResultItemNo){goto __HIDE;}
	{
		const POINT2 rPT =	pWnd->TotalLocation();

		int const iMyCount = pkPlayer->GetInven()->GetTotalCount(rkElement.iItemNo);
		g_kUIScene.RenderIcon( rkElement.iResultItemNo, rPT, false, 40, 40, iMyCount < rkElement.iItemCount);
		pWnd->SetCustomData(&rkElement.iResultItemNo, sizeof(rkElement.iResultItemNo));

		const wchar_t *pName = NULL;
		if(	GetItemName(rkElement.iResultItemNo, pName))
		{
			BM::vstring vstr;
			BM::vstring vNameStr = vstr;
			std::wstring wstrText(pName);
			vNameStr += wstrText;

			assert(pSrcNameWnd);
			if(pSrcNameWnd)
			{
				pSrcNameWnd->Text((const wchar_t*)vNameStr);//리소스 아이템 이름 디스플레이.
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
		//pWnd->SetCustomData(&iNull, sizeof(iNull));

	}
}
	
void PgShineStoneUpgrade::DisplayDetail(XUI::CXUI_Wnd *pWnd, int iItemNo, int iItemCount)
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
	
	{
		const POINT2 rPT =	pWnd->TotalLocation();
		g_kUIScene.RenderIcon( iItemNo, rPT, false );
		pWnd->SetCustomData(&iItemNo, sizeof(iItemNo));

		const wchar_t *pName = NULL;
		if(	GetItemName(iItemNo, pName))
		{
			PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
			assert(pkPilot);
			if(!pkPilot){return;}
			
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			assert(pkPlayer);
			if(!pkPlayer){return;}
			
			int const iMyCount = pkPlayer->GetInven()->GetTotalCount(iItemNo);
			
			BM::vstring vstr;

			if(iMyCount < iItemCount){vstr += TTW(1302);}//칼라지정.
			else if(iItemCount<=0) {vstr += TTW(1303);}
			else{	vstr += TTW(1301);}
			BM::vstring vNameStr = vstr;
			std::wstring wstrText(pName);
			vNameStr += wstrText;

			assert(pSrcNameWnd);

			if(pSrcNameWnd)
			{
				pSrcNameWnd->Text((const wchar_t*)vNameStr);//리소스 아이템 이름 디스플레이.
			}
			if(pCountNameWnd)
			{
				BM::vstring vstrCount;
				vstrCount += iItemCount;
				pCountNameWnd->Text((const wchar_t*)vstrCount);//리소스 아이템 이름 디스플레이.
			}			
		}
	}

	return;
}

void PgShineStoneUpgrade::DisplayDetailSrcItem(XUI::CXUI_Wnd *pWnd)
{
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(m_iSelectIndex);
	if (itr == m_kUpgradeElement.end())
	{
		return;
	}
	const SUpgradeElement& rkElement = (*itr).second;
	
	DisplayDetail(pWnd, rkElement.iItemNo, rkElement.iItemCount);
}

void PgShineStoneUpgrade::DisplayDetailDestItem(XUI::CXUI_Wnd *pWnd)
{
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(m_iSelectIndex);
	if (itr == m_kUpgradeElement.end())
	{
		return;
	}
	const SUpgradeElement& rkElement = (*itr).second;
	
	DisplayDetail(pWnd, rkElement.iResultItemNo, rkElement.iResultItemCount);
}

void PgShineStoneUpgrade::SelectItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex)
{
	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	assert(pkPilot);
	if(!pkPilot){return;}
	
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	assert(pkPlayer);
	if(!pkPlayer){return;}

	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(iBuildIndex);
	if (itr == m_kUpgradeElement.end())
	{
		return;
	}
	const SUpgradeElement& rkElement = (*itr).second;
	int const iMyCount = pkPlayer->GetInven()->GetTotalCount(rkElement.iItemNo);
	if (iMyCount < rkElement.iItemCount)
	{
		lwAddWarnDataTT(1708);
		return;
	}
	
	// Set 하고 UI를 띄워주자.
	m_iSelectIndex = iBuildIndex;
	lwUIWnd kNewWnd = lwActivateUI("SFRM_SHINESTONE_CHANGE_DETAIL");
	//XUI::CXUI_Wnd* pkNewWnd = XUIMgr.Call(_T());
	if (!kNewWnd())
	{
		return;
	}

	lwUIWnd kWnd = lwActivateUI("SFRM_SHINESTONE_CHANGE");
	if (kWnd())
	{
		kWnd.Close();
	}
}

void PgShineStoneUpgrade::MouseOver(XUI::CXUI_Wnd *pWnd, int iBuildIndex)
{
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;
	XUI::CXUI_Wnd *pFormWnd = NULL;
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
	}
	if (!pSrcNameWnd)
	{
		return;
	}

	// UI Text
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(iBuildIndex);
	if (itr == m_kUpgradeElement.end())
	{
		return;
	}
	const SUpgradeElement& rkElement = (*itr).second;

	const wchar_t *pItemName = NULL;
	GetDefString(rkElement.iItemNo, pItemName);
	const wchar_t *pResultItemName = NULL;
	GetDefString(rkElement.iResultItemNo, pResultItemName);

	std::wstring wstrText;
	wstrText += _T("{C=0xFF0000C4/}");
	wstrText += std::wstring((pItemName ? pItemName : _T("")));
	wstrText += _T(" ");
	wstrText += _T("{C=0xFF4D3413/}");
	wstrText += (std::wstring)(BM::vstring(rkElement.iItemCount));
	wstrText += TTW(1706);		// '개를'
	wstrText += _T("\n");
	wstrText += _T("{C=0xFF3C8000/}");
	wstrText += std::wstring((pResultItemName ? pResultItemName : _T("")));
	wstrText += _T(" ");
	wstrText += _T("{C=0xFF4D3413/}");
	wstrText += (std::wstring)(BM::vstring(rkElement.iResultItemCount));
	wstrText += TTW(1707);		// '개로 교환 가능함'

	m_wstrDetailText = wstrText;
}

void PgShineStoneUpgrade::DisplayDetailText(XUI::CXUI_Wnd *pWnd)
{
	if (!pWnd)
	{
		return;
	}
	pWnd->Text(m_wstrDetailText);
}

void PgShineStoneUpgrade::MouseOut()
{
	m_wstrDetailText.clear();
}
void PgShineStoneUpgrade::SendReqUpgrade()
{
	CONT_UPGRADEELEMENT::const_iterator itr = m_kUpgradeElement.find(m_iSelectIndex);
	if (itr == m_kUpgradeElement.end())
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}
	const SUpgradeElement& rkElement = (*itr).second;
	SItemPos kItemPos;
	if (S_OK != pkPlayer->GetInven()->GetFirstItem(rkElement.iItemNo, kItemPos))
	{
		// 아이템이 없서요.
		lwAddWarnDataTT(1703);
		return;
	}

	// Get Def Making
	SDefItemMakingData kMakingData;
	GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
	if (!kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, rkElement.iMakingNo))
	{
		lwAddWarnDataTT(1705);
		return ;
	}

	// 수량 체크 하자.
	int iMyItemCount = pkPlayer->GetInven()->GetTotalCount(rkElement.iItemNo);
	if (!iMyItemCount)
	{
		lwAddWarnDataTT(1703);
		return;
	}

	for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
	{
		int iSrcEleNo = kMakingData.kNeedElements.aElement[i];
		int iSrcEleCount = kMakingData.kNeedCount.aCount[i];
		// Need Data
		if (!iSrcEleNo || !iSrcEleCount)
		{
			continue;
		}

		if (iSrcEleCount > iMyItemCount)
		{
			// 수량 부족이네.
			lwAddWarnDataTT(1704);
			return;
		}
	}

	SReqItemMaking kReqItemMaking;
	kReqItemMaking.iMakingNo = rkElement.iMakingNo;
	kReqItemMaking.iMakingCount = 1;
	// InsertSource
/*	SReqItemMaking_ItemSlot kItemSlot;
	kItemSlot.kItemPos = kItemPos;
	kItemSlot.iItemCount = rkElement.iItemCount;
	kReqItemMaking.kItemSlotCont.push_back(kItemSlot);
*/
	{
		BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
		kPacket.Push(1);	// 뽑기 한번 하기.
		kReqItemMaking.WriteToPacket(kPacket);
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

		NETWORK_SEND(kPacket)
	}
}
