#include "StdAfx.h"
#include "lwUI.h"
#include "lwUIItemRepair.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "lwHomeUI_Script.h"

namespace PgUIItemRepairUtil
{
	void CallRepairUI(lwGUID kNpcGuid)
	{
		g_kItemRepair.NpcGuid( kNpcGuid() );
		std::wstring const kRepairUIName(_T("SFRM_REPAIR"));
		CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kRepairUIName );
		if( pkTopWnd )
		{
			RegistUIAction(pkTopWnd);
		}
	}
}

lwUIItemRepair::lwUIItemRepair(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemRepair::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallRepairUI", PgUIItemRepairUtil::CallRepairUI);
	
	class_<lwUIItemRepair>(pkState, "ItemRepairWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "DisplaySrcIcon", &lwUIItemRepair::DisplaySrcIcon)
		.def(pkState, "Clear", &lwUIItemRepair::Clear)
		.def(pkState, "SendReqRepair", &lwUIItemRepair::SendReqRepair)
		.def(pkState, "GetRepairNeedMoney", &lwUIItemRepair::GetRepairNeedMoney)
		.def(pkState, "SetRepairAllInfo", &lwUIItemRepair::SetRepairAllInfo)
		.def(pkState, "CheckOK", &lwUIItemRepair::CheckOK)
		
	;
	return true;
}

void lwUIItemRepair::SetRepairAllInfo()
{
	g_kItemRepair.SetRepairAllItem();
}

int lwUIItemRepair::GetRepairNeedMoney()
{
	return g_kItemRepair.NeedMoney();
}

void lwUIItemRepair::DisplaySrcIcon()
{
	g_kItemRepair.DisplaySrcIcon(self);
}

void lwUIItemRepair::Clear(bool const bAllClear)
{
	g_kItemRepair.Clear(bAllClear);
}

void lwUIItemRepair::SendReqRepair(bool const bIsAllRepair)
{
	g_kItemRepair.SendReqRepair(bIsAllRepair);
}

int lwUIItemRepair::CheckOK()
{
	return g_kItemRepair.CheckOK();
}




PgItemRepairMgr::PgItemRepairMgr()
{
	RepairAll(false);
}

PgItemRepairMgr::~PgItemRepairMgr()
{
}

void PgItemRepairMgr::Clear(bool const bAllClear)
{
	NeedMoney(0);
	m_guidSrcItem = BM::GUID::NullData();
	m_kContRepair.clear();
	m_kSrcItemPos.Clear();
	SetSrcName(_T(""));
	RepairAll(false);
	if( bAllClear )
	{
		m_kNpcGuid.Clear();
	}
}

bool PgItemRepairMgr::SetRepairAllItem()//모두 수리 한다고 생각.
{
	Clear();//클리어 밟아야함.
	int iWarnMessage = 0;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv)
	{
		return false;
	}

	int iNeedMoney = 0;

	m_kContRepair.clear();//전부수리는 컨테이너를 비움.

	bool bRepairAll = false;

	for( int i = 0; MAX_FIT_IDX>i; i++)
	{
		const SItemPos kItemPos(IT_FIT,i);
		PgBase_Item kItem;
		
		if(S_OK == pInv->GetItem(kItemPos, kItem)
			&& IsCanRepair(kItem.ItemNo() ) )
		{
			int iOutNeedMoney = 0;
			if(S_OK == CalcRepairNeedMoney(kItem, pkPlayer->GetAbil( AT_ADD_REPAIR_COST_RATE ), iOutNeedMoney))
			{
				if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 마이홈 내부이면 할인율 적용
				{
					iOutNeedMoney = static_cast<int>(static_cast<float>(iOutNeedMoney) * ((100.0f - lwHomeUI::GetMyHomeSideJobDiscountRate(MSJ_REPAIR, MSJRT_GOLD)) / 100.0f ));
				}
				iNeedMoney += iOutNeedMoney;
				bRepairAll = true;//하나라도 수리할 아이템이 있으면 전부 수리.
//				m_kContRepair.push_back(kItemPos);
			}
		}
	}

	NeedMoney(iNeedMoney);
	RepairAll(bRepairAll);
	return bRepairAll;
}

bool PgItemRepairMgr::SetSrcItem(const SItemPos &rkItemPos)
{
/*
	XUIMgr.Get("SFRM_REPAIR")
SFRM_COLOR1 ->이거 아래에.
					<STATIC_FORM ID = "SFRM_ALL">
				<ATTR X="6" Y="94" W="242" H="29" STATIC_NUM="5" CAN_COLOR_CHANGE="1" COLOR_TYPE="2"/>
				<ATTR TEXT = "400603" T_X = "30" T_Y ="6"/>
				<CHECK_BUTTON ID = "CBTN_CHECK">

			SFRM_SHADOW
			SFRM_SINGLE
			*/
	Clear();//클리어 밟아야함.
	int iWarnMessage = 0;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv)
	{
		return false;
	}
	PgBase_Item kItem;

	switch(rkItemPos.x)
	{
	case KUIG_FIT:
	case KUIG_EQUIP:
	case KUIG_SAFE_ADDITION:
		{
			if(S_OK != pInv->GetItem(rkItemPos, kItem))
			{
				iWarnMessage = 1219;
				goto __FAILED;
			}
			if(!IsCanRepair(kItem.ItemNo()))
			{
				iWarnMessage = 1219;
				goto __FAILED;
			}

			int iValue = 0;
			const HRESULT hRet = CalcRepairNeedMoney(kItem, pkPlayer->GetAbil( AT_ADD_REPAIR_COST_RATE ), iValue);
			if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 마이홈 내부이면 할인율 적용
			{
				iValue = static_cast<int>(static_cast<float>(iValue) * ((100.0f - lwHomeUI::GetMyHomeSideJobDiscountRate(MSJ_REPAIR, MSJRT_GOLD)) / 100.0f ));
			}
			
			if(S_OK == hRet)
			{
				m_kSrcItemPos = rkItemPos;
				m_guidSrcItem = kItem.Guid();
				goto __SUCCESS;
			}
			iWarnMessage = hRet;
			goto __FAILED;
		}break;
	case KUIG_ITEM_REPAIR:	//수리 아이콘 위치를 그냥 클릭했다
	case KUIG_NONE:
		{
			return false;
		}break;
	default:
		{
			iWarnMessage = 1219;
		}break;
	}
	goto __FAILED;
__SUCCESS:
	{
		if(!PgBase_Item::IsEmpty(&kItem))
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());

			if(pDef)
			{
				const wchar_t *pName = NULL;
				if(GetDefString(pDef->NameNo(), pName))//이름 디스플레이
				{	//여기서 돈 셋팅.
					int iNeedMoney = 0;
					if(S_OK == CalcRepairNeedMoney((kItem), pkPlayer->GetAbil( AT_ADD_REPAIR_COST_RATE ), iNeedMoney))
					{
						if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 마이홈 내부이면 할인율 적용
						{
							iNeedMoney = static_cast<int>(static_cast<float>(iNeedMoney) * ((100.0f - lwHomeUI::GetMyHomeSideJobDiscountRate(MSJ_REPAIR, MSJRT_GOLD)) / 100.0f ));
						}
						NeedMoney(iNeedMoney);
						SetSrcName(pName);
						return true;
					}
				}
			}
		}
		//이름 셋팅 실패.
		iWarnMessage = 1219;
		m_kSrcItemPos.Clear();
		m_guidSrcItem = BM::GUID::NullData();
		RepairAll(false);
	}
__FAILED:
	{	
		if(iWarnMessage)
		{
			lwAddWarnDataTT(iWarnMessage);
		}
		return false;
	}
}

void PgItemRepairMgr::SetSrcName(std::wstring const &kStr)
{
	CXUI_Wnd *pWnd = XUIMgr.Get(_T("SFRM_REPAIR"));
	if(pWnd)
	{
		CXUI_Wnd *pColor = pWnd->GetControl(_T("SFRM_COLOR1"));
		if(pColor)
		{
			CXUI_Wnd *pShadow = pColor->GetControl(_T("SFRM_SHADOW"));
			if(pShadow)
			{
				CXUI_Wnd *pNameWnd = pShadow->GetControl(_T("SFRM_NAME"));
				if(pNameWnd)
				{
					pNameWnd->Text(kStr);
				}
			}
		}
	}
}

void PgItemRepairMgr::DisplaySrcIcon(XUI::CXUI_Wnd *pWnd)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgInventory *pInv = pkPlayer->GetInven();

	if(pWnd && pInv)
	{
		int const iNull = 0;
		POINT2 const rPT = pWnd->TotalLocation();
		
		PgBase_Item kItem;
		if(S_OK == pInv->GetItem(m_kSrcItemPos, kItem))
		{
			if(kItem.Guid() == m_guidSrcItem)
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if(pItemDef)
				{
					g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );	
				}
				pWnd->SetCustomData(&kItem.ItemNo(), sizeof(kItem.ItemNo()));
			}
			else
			{
				pWnd->SetCustomData(&iNull, sizeof(iNull));
			}
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
		}
	}
}

void PgItemRepairMgr::SendReqRepair(bool const bIsAllRepair)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv)
	{
		return;
	}

	if(bIsAllRepair)
	{//모두.
		goto __SUCCESS;
	}
	else
	{//타겟.
		PgBase_Item kItem;
		if(S_OK == pInv->GetItem(m_kSrcItemPos, kItem))
		{
			if(kItem.Guid() == m_guidSrcItem)
			{
				m_kContRepair.push_back(m_kSrcItemPos);
				goto __SUCCESS;
			}
		}
	}
	goto __FAILED;
__FAILED:
	{
		lwAddWarnDataTT(1225);
		Clear();
		return;
	}
__SUCCESS:
	{
		BM::Stream kPacket(PT_C_M_REQ_ITEM_REPAIR);
		kPacket.Push( m_kNpcGuid );
		kPacket.Push( m_kContRepair );
		NETWORK_SEND(kPacket)
		Clear();
		return;
	}
}

int PgItemRepairMgr::CheckOK()
{
	if (!RepairAll() && SItemPos::NullData() == m_kSrcItemPos)
	{
		return 1224;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer)
	{
		if (NeedMoney() > pkPlayer->GetAbil64(AT_MONEY))
		{
			return 1226;
		}
	}

	return 0;
}

/*
HRESULT PgItemRepairMgr::CalcRepairNeedMoney(PgBase_Item const &rkItem, int &iOutValue)
{
	CItemDef const *pDef = g_kItemDefMgr.GetDef(rkItem.ItemNo());
	if(pDef && pDef->CanEquip())
	{
		if(rkItem.Count() < pDef->GetAbil(AT_DEFAULT_AMOUNT))
		{	
			//여기서 돈 셋팅.
			int const iSellPrice = pDef->GetAbil(AT_SELLPRICE);
			int const iMaxDur = pDef->GetAbil(AT_DEFAULT_AMOUNT);
			int const iNowDur = rkItem.Count();
			iOutValue = (float)iSellPrice * 100.0f * (float)iNowDur / (float)iMaxDur * (1.0f+ (float)rkItem.EnchantInfo().TotalLv()/(float)2);
			return S_OK;
		}
		else
		{
			return 1220;
		}
	}
	return 1219;
}*/