#include "StdAfx.h"
#include "lwUI.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "Variant/ItemMakingDefMgr.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgNetwork.h"
#include "lwUIItemCooking.h"
#include "PgSoundMan.h"
#include "lwUIQuest.h"


int const ROLLING_DELAY_TIME = 3000;

lwUIItemCooking::lwUIItemCooking(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemCooking::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	class_<lwUIItemCooking>(pkState, "ItemCookingWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "DisplaySrcIcon", &lwUIItemCooking::DisplaySrcIcon)
		.def(pkState, "DisplayNeedItemIcon", &lwUIItemCooking::DisplayNeedItemIcon)
		.def(pkState, "DisplayResultItemIcon", &lwUIItemCooking::DisplayResultItem)
		.def(pkState, "ClearCookingData", &lwUIItemCooking::ClearCookingData)
		.def(pkState, "SendReqItemCooking", &lwUIItemCooking::SendReqItemCooking)
		.def(pkState, "GetCookingNeedMoney", &lwUIItemCooking::GetCookingNeedMoney)
		.def(pkState, "CallComfirmMessageBox", &lwUIItemCooking::CallComfirmMessageBox)
		.def(pkState, "Clear", &lwUIItemCooking::Clear)
		.def(pkState, "CheckOK", &lwUIItemCooking::CheckOK)
		.def(pkState, "GetNowNeedItemCount", &lwUIItemCooking::GetNowNeedItemCount)
		.def(pkState, "SetNowCooking", &lwUIItemCooking::SetNowCooking)
		.def(pkState, "NowCooking", &lwUIItemCooking::NowCooking)
		.def(pkState, "AddMakingCount", &lwUIItemCooking::AddMakingCount)
		.def(pkState, "GetMakingCount", &lwUIItemCooking::GetMakingCount)
		.def(pkState, "SetUseOptionItem", &lwUIItemCooking::SetUseOptionItem)
		.def(pkState, "NextResultItem", &lwUIItemCooking::NextResultItem)
	;
	return true;
}

void lwUIItemCooking::Clear()
{
	g_kItemCookingMgr.Clear();
}

void lwUIItemCooking::DisplaySrcIcon()
{
	g_kItemCookingMgr.DisplaySrcItem(self);
}

void lwUIItemCooking::SetNowCooking(bool bValue)
{
	g_kItemCookingMgr.NowCooking(bValue);
}

bool lwUIItemCooking::NowCooking()
{
	return g_kItemCookingMgr.NowCooking();
}

void lwUIItemCooking::DisplayResultItem()
{
	g_kItemCookingMgr.DisplayResultItem(self);
}

void lwUIItemCooking::DisplayNeedItemIcon()
{
	CXUI_Wnd *pParent = self->Parent();//SFRM_MATERIAL
	if(pParent)
	{
		CXUI_Wnd *pGrandParent = pParent->Parent();//FRM_MATERIAL
		if(pGrandParent)
		{
			g_kItemCookingMgr.DisplayNeedItemIcon( pGrandParent->BuildIndex(), self );
			return;
		}
	}
	assert(NULL && "lwUIItemCooking::DisplayNeedItemIcon");
}

void lwUIItemCooking::ClearCookingData()
{
	g_kItemCookingMgr.Clear();
}

bool lwUIItemCooking::SendReqItemCooking(bool bIsTrueSend)
{
	return g_kItemCookingMgr.SendReqItemCooking(bIsTrueSend);
}

int lwUIItemCooking::GetCookingNeedMoney()
{
	return g_kItemCookingMgr.GetCookingNeedMoney();
}

void lwUIItemCooking::CallComfirmMessageBox()
{
	g_kItemCookingMgr.CallComfirmMessageBox();
}


/*
void RECV_PT_C_M_ANS_ITEM_PLUS_COOKING(BM::Stream* pkPacket)
{
	//PLUS_ITEM_UPGRADE_RESULT kPIUR;
	PLUS_ITEM_COOKING_RESULT kPIUR;
	
	if(pkPacket->Pop(kPIUR))
	{
		int iWarnMessage = 0;
		ENoticeLevel eLevel = EL_Warning;
		switch(kPIUR.eResult)
		{
		case PIUR_SUCCESS://	= 1,
			{
				iWarnMessage = 1201;//	제련에 성공 하였습니다
				eLevel = EL_Normal;
			}break;
		case PIUR_NOT_ENOUGH_RES: //	= 2,//재료 부족
			{
				iWarnMessage = 1202;//	제련에 필요한 재료가 부족합니다
			}break;
		case PIUR_CANT_DELETE_RES: //	= 3,//재료 부족
			{
				iWarnMessage = 1203;//	제련에 사용된 재료를 소모 할 수 없어 실패 하였습니다
			}break;
		case PIUR_NORMAL_FAILED: //	= 4,//걍 실패(초급)
			{
				iWarnMessage = 1204;//	제련에 실패 하였습니다
			}break;
		case PIUR_PANALTY_FAILED: //	= 5,//등급이 깎이는 실패(중급)
			{
				iWarnMessage = 1205;//	제련에 실패 하여 등급이 하락 되었습니다
			}break;
		case PIUR_NOT_ENOUGH_MONEY: //	= 2,//재료 부족
			{
				iWarnMessage = 1207;//돈이 모자랍니다.
			}break;
		default:
			{
				return;
			}break;
		}
		Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
		
//		PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
//		if(!pPilot){return;}
//		
//		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>pPilot->GetUnit();
//		if(!pkPlayer){return;}
		
//		PgInventory *pkInv = pkPlayer->GetInven();
//
//		PgBase_Item const *pkItem = pkInv->GetItem(kPIUR.kItemPos);
//		if (PgBase_Item::IsEmpty(pkItem))
//		{
//			return;
//		}
//		pkInv->SetItem(kPIUR.kItemPos, &(kPIUR.kItem));

//		int x = 0;
	}
}
*/

int lwUIItemCooking::CheckOK()
{
	__int64 const iNeedMoney = g_kItemCookingMgr.GetCookingNeedMoney();
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (iNeedMoney > iMoney)
		{
			return 1608;
		}
	}
	if (g_kItemCookingMgr.CheckNeedItem())
	{
		return 1609;
	}
	return 0;
}

int const lwUIItemCooking::GetNowNeedItemCount(int const iNeed) const
{
	return g_kItemCookingMgr.GetNowNeedItemCount(iNeed);
}

int lwUIItemCooking::AddMakingCount(int i)
{
	int const iRecipeCount = g_kItemCookingMgr.RecipeCount(); // iRecipeCount
	int const iCookCount = g_kItemCookingMgr.MakingCount(); // Pre Count

	if( 0 > i ) // Dec
	{
		if( 1 < iCookCount )
		{
			g_kItemCookingMgr.MakingCount(iCookCount-1);
		}
		else
		{
			int const iMaxCount = std::min(SoulCraft::iOneTimeMaxMakingCount, g_kItemCookingMgr.MaximumCount());
			if(0 < iMaxCount)
			{// 만들수 있는 최대 개수가 0개 이면, 업데이트 하지 않는다
				g_kItemCookingMgr.MakingCount( iMaxCount );
			}
		}
	}
	else // Inc
	{
		int const iMaxCount = std::min(g_kItemCookingMgr.MaximumCount(), SoulCraft::iOneTimeMaxMakingCount);
		int const iNextCount = iCookCount+i;
		if( iNextCount > iMaxCount )
		{
			g_kItemCookingMgr.MakingCount( 1 );
		}
		else
		{
			g_kItemCookingMgr.MakingCount( iNextCount );
		}
	}
	return g_kItemCookingMgr.MakingCount();
}

int lwUIItemCooking::GetMakingCount()
{
	return g_kItemCookingMgr.MakingCount();
}

void lwUIItemCooking::SetUseOptionItem(bool const bUse)
{
	g_kItemCookingMgr.SetUseOptionItem(self, bUse);
}

void lwUIItemCooking::NextResultItem()
{
	g_kItemCookingMgr.NextResultItem();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemCookingMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemCookingMgr::PgItemCookingMgr()
{
	Clear();
	NowCooking(false);
}

void PgItemCookingMgr::Clear()
{
	m_kContResultItem.clear();
	m_iDisplayResultItemIdx = 0;
	m_iDisplayResultItemTime = 0;
	UseOptionItem(false);
	m_guidSrcItem = BM::GUID();
	m_kItem.Clear();
	m_kSrcItemPos = SItemPos();
	CurMakingNo(0);
	CurMakingType(0);

	m_spCookingData.reset();
	NowCooking(false);
	MakingCount(1);
	RecipeCount(1);
}

int PgItemCookingMgr::CallComfirmMessageBox()
{
//		const TBL_DEF_ITEMPLUSUPGRADE* pDef = GetPlusInfo();
	if(!PgBase_Item::IsEmpty(&m_kItem))
	{
		int iTextNo = 0;
		switch(m_kItem.EnchantInfo().PlusLv())
		{
		case 0:
		case 1:
			{
				iTextNo = 1607;
			}break;
		case 2:
		case 3:
			{
				iTextNo = 1607;
			}break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			{
				iTextNo = 1607;
			}break;
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			{
				iTextNo = 1607;
			}break;
		default:
			{//업그레이드 할 수 없는 등급.
				goto __ERROR;
			}break;
		}
		
		XUI::CXUI_Wnd *pWnd =  XUIMgr.Call(_T("SFRM_MSG_COOKING_REFINE"));
		if(pWnd)
		{
			XUI::CXUI_Wnd *pColorWnd =  pWnd->GetControl(_T("SFRM_COLOR"));
			if(pColorWnd)
			{
				XUI::CXUI_Wnd *pSdwWnd =  pColorWnd->GetControl(_T("SFR_SDW"));
				if(pSdwWnd)
				{//커스텀 데이터 넘겨줘.
					pSdwWnd->Text(TTW(iTextNo));
					return 0;
				}
			}
		}

		assert(pWnd);
		return 0;
	}
__ERROR:
	{
		lwAddWarnDataTT(1611);
		return 0;
	}
}

int PgItemCookingMgr::GetCookingNeedMoney()
{
	if (CurMakingNo() && m_spCookingData)
	{
		return m_spCookingData->NeedMoney()*MakingCount();
	}
	return 0;
}

bool PgItemCookingMgr::SetSrcItem(const SItemPos &rkItemPos)
{
	if (NowCooking())
	{
		return false;
	}
	Clear();//클리어 밟아야함.

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	if(S_OK != pInv->GetItem(rkItemPos, m_kItem))
	{
		return false;
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);
	int iMakingNo = kItemDefMgr.GetAbil(m_kItem.ItemNo(), AT_ITEMMAKING_NO);
	if (!iMakingNo)
	{
		// 제조 할 수 있는 아이템이 아닙니다.
		lwAddWarnDataTT(1611);
		return false;
	}

	int const iMakingType = kItemDefMgr.GetAbil(m_kItem.ItemNo(), AT_MAKING_TYPE);
	if(EMAKING_TYPE_NEW_COOKING == iMakingType)
	{
		CONT_DEFCOOKING const* pkDefCooking = NULL;
		g_kTblDataMgr.GetContDef(pkDefCooking);

		if(NULL == pkDefCooking)
		{
			lwAddWarnDataTT(1611);
			return false;
		}

		CONT_DEFCOOKING::const_iterator cooking_it = pkDefCooking->find(iMakingNo);
		if(cooking_it == pkDefCooking->end())
		{
			lwAddWarnDataTT(1611);
			return false;
		}

		SCookingData * pkData = new SCookingData;
		if(pkData)
		{
			pkData->SetDef( &(*cooking_it).second );
			m_spCookingData.reset(pkData);
		}

		m_guidSrcItem = m_kItem.Guid();//GUID 기록
		m_kSrcItemPos = rkItemPos;
		CurMakingNo(iMakingNo);
		CurMakingType(iMakingType);
		g_kItemCookingMgr.RecipeCount(m_kItem.Count());

		UpdateResultItem();
		return true;
	}
	else if(EMAKING_TYPE_COOKING == iMakingType)
	{
		SDefItemMakingData kMakingData;
		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		if (kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, iMakingNo))
		{
			SMakingData * pkData = new SMakingData;
			if(pkData)
			{
				pkData->m_kMakingData = kMakingData;
				m_spCookingData.reset(pkData);
			}

			m_guidSrcItem = m_kItem.Guid();//GUID 기록
			m_kSrcItemPos = rkItemPos;
			CurMakingNo(iMakingNo);
			CurMakingType(iMakingType);
			g_kItemCookingMgr.RecipeCount(m_kItem.Count());

			UpdateResultItem();
			return true;
		}
	}

	// 요리템이 아닙니다.
	lwAddWarnDataTT(1611);
	return false;
}

void PgItemCookingMgr::SetUseOptionItem(XUI::CXUI_Wnd *pWnd, bool const bUse)
{
	if(!pWnd){ return; }
	if(!m_spCookingData){ return; }
	if(UseOptionItem() == bUse){ return; }
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if(!pkPlayer){ return; }

	if(EMAKING_TYPE_NEW_COOKING == CurMakingType())
	{
		if(false == bUse)
		{
			UseOptionItem(bUse);
			UpdateResultItem();
		}
		else
		{
			CXUI_Wnd *pParent = pWnd->Parent();
			pParent = pParent ? pParent->Parent() : NULL;
			if(NULL==pParent)
			{
				return;
			}

			int const iNeedIndex = pParent->BuildIndex();

			bool const bOptionItem = m_spCookingData->IsOptionItem(iNeedIndex);
			int const iNeedItemNo = m_spCookingData->NeedItemNo(iNeedIndex);
			int const iNeedItemCount = m_spCookingData->NeedItemCount(iNeedIndex)*MakingCount();

			int const iMyCount = pkPlayer->GetInven()->GetTotalCount(iNeedItemNo);
			if(iMyCount < iNeedItemCount)
			{
				return;
			}

			UseOptionItem(bUse);
			UpdateResultItem();
		}
	}
}

void PgItemCookingMgr::DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;
	XUI::CXUI_Wnd *pPlusWnd = NULL;
	XUI::CXUI_Wnd *pMinusWnd = NULL;

	int const iNull = 0;

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
	XUI::CXUI_Wnd *pTopWnd = pFormWnd->Parent();
	assert(pTopWnd);
	if (!pTopWnd)
	{
		return;
	}
	pPlusWnd = pFormWnd->GetControl(L"BTN_PLUS");
	if(!pPlusWnd){ return; }
	pMinusWnd = pFormWnd->GetControl(L"BTN_MINUS");
	if(!pMinusWnd){ return; }

	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(m_kItem.Guid() != m_guidSrcItem){goto __HIDE;}//guid 바뀌었으면 찾아가든가.
	if(!CurMakingNo()){goto __HIDE;}
	if(!m_spCookingData){goto __HIDE;}

	if(iNeedIndex >= 0 )
	{
		const POINT2 rPT =	pWnd->TotalLocation();
		bool const bOptionItem = m_spCookingData->IsOptionItem(iNeedIndex);
		int const iNeedItemNo = m_spCookingData->NeedItemNo(iNeedIndex);
		int const iNeedItemCount = m_spCookingData->NeedItemCount(iNeedIndex)*MakingCount();

		pPlusWnd->Visible(bOptionItem ? !UseOptionItem() : false);
		pMinusWnd->Visible(bOptionItem ? UseOptionItem() : false);
		
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iNeedItemNo);

		if(pItemDef)
		{
			g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
		}
		pWnd->SetCustomData(&iNeedItemNo, sizeof(iNeedItemNo));

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
			
			BM::vstring vstr;

			if(iMyCount < iNeedItemCount){vstr += TTW(1302);}//칼라지정.
			else if(bOptionItem && (false==UseOptionItem())){vstr += TTW(1618);}	//옵션아이템을 선택하지 않은 상태는 회색으로 처리
			else if(iNeedItemCount<=0) {vstr += TTW(1303);}
			else{	vstr += TTW(1301);}
			BM::vstring vNameStr = vstr;
			std::wstring wstrText(pName);
			vNameStr += wstrText;

			vstr += bOptionItem ? TTW(1619) : L"";
			vstr += TTW(1613);
			vstr += _T(" ");
			vstr += (!bOptionItem || (bOptionItem && UseOptionItem())) ? __min(iMyCount,iNeedItemCount) : 0;
			vstr += TTW(1300);//화이트
			vstr += _T(" / ");
			vstr += iNeedItemCount;

			assert(pSrcNameWnd);

			if(pSrcNameWnd)
			{//리소스 아이템 이름 디스플레이.				
				int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
				Quest::SetCutedTextLimitLength(pSrcNameWnd, static_cast<std::wstring>(vNameStr), WSTR_UI_REGULATIONSTR, iWidth);
				
			}
			if (pTopWnd)
			{
				pTopWnd->Text((const wchar_t*)vstr);
			}
		}
		else
		{
			goto __HIDE;
		}
	}
	else
	{
		assert(NULL);
	}

	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
		}
		if (pTopWnd)
		{
			pTopWnd->Text(_T(""));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));

	}
}

void PgItemCookingMgr::DisplaySrcItem(XUI::CXUI_Wnd *pWnd)
{//네임 필드를 찾아서 이름 셋팅. 아이템이 없거나 하면 올리세요 라는 메세지로 셋팅.
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;

	int const iNull = 0;

	if (!pWnd)
	{
		return;
	}

	pFormWnd = pWnd->Parent();
	assert(pFormWnd);
	if (!pFormWnd)
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		return;
	}
	if(pFormWnd)
	{
		pShadowWnd = pFormWnd->Parent();
		assert(pShadowWnd);
		if(pShadowWnd)
		{
			pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_SRC_NAME"));
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
			return;
		}
	}

	std::wstring wstrName;
	POINT2 rPT;

	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(m_kItem.Guid() != m_guidSrcItem){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	rPT =	pWnd->TotalLocation();
	
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());

	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
	}
	MakeItemName(m_kItem.ItemNo(), m_kItem.EnchantInfo(), wstrName);

	pWnd->SetCustomData(&m_kItem.ItemNo(), sizeof(m_kItem.ItemNo()));
}
	

	assert(pSrcNameWnd);
	if(pSrcNameWnd)
	{
		int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pSrcNameWnd, wstrName, WSTR_UI_REGULATIONSTR, iWidth);
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{	
			int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
			Quest::SetCutedTextLimitLength(pSrcNameWnd, TTW(1610), WSTR_UI_REGULATIONSTR, iWidth);

		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));
	}
	return;
}

void PgItemCookingMgr::DisplayResultItem(XUI::CXUI_Wnd *pWnd)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;
	SEnchantInfo kInfo;

	int const iNull = 0;

	if (!pWnd)
	{
		return;
	}
	pFormWnd = pWnd->Parent();
	assert(pFormWnd);
	if (!pFormWnd)
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		return;
	}
	if(pFormWnd)
	{
		pShadowWnd = pFormWnd->Parent();
		assert(pShadowWnd);
		if(pShadowWnd)
		{
			pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_DEST_NAME"));
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
			return;
		}
	}

	std::wstring wstrName;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	POINT2 rPT;
	
	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(m_kItem.Guid() != m_guidSrcItem){goto __HIDE;}//guid 바뀌었으면 찾아가든가.
	if(m_kContResultItem.empty()){goto __HIDE;}

//		pWnd->Visible(true);

	rPT = pWnd->TotalLocation();

	//결과가 여러개인경우 롤링시키기
	if(m_kContResultItem.size() > 1)
	{
		DWORD const dwNow = BM::GetTime32();
		if((dwNow - m_iDisplayResultItemTime) > ROLLING_DELAY_TIME)
		{
			NextDisplayResultItem(dwNow);
		}

		//페이지 표시
		BM::vstring vText;
		vText += m_iDisplayResultItemIdx+1;
		vText += L"/";
		vText += m_kContResultItem.size();

		POINT2 RenderPT = rPT;
		RenderPT.x += 17;
		RenderPT.y += 26;

		SRenderTextInfo kRenderTextInfo;
		kRenderTextInfo.wstrText = vText.operator const std::wstring &();
		kRenderTextInfo.kLoc = RenderPT;
		kRenderTextInfo.wstrFontKey = FONT_TEXT;
		kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
		g_kUIScene.RenderText(kRenderTextInfo);
	}

	int const iItemNo = m_kContResultItem.at(m_iDisplayResultItemIdx).first;
	int const iCount = m_kContResultItem.at(m_iDisplayResultItemIdx).second;

	if(EMAKING_TYPE_NEW_COOKING == CurMakingType())
	{
		//Count 표시
		SRenderTextInfo kRenderTextInfo;
		kRenderTextInfo.wstrText = BM::vstring(iCount).operator const std::wstring &();
		kRenderTextInfo.kLoc = rPT;
		kRenderTextInfo.wstrFontKey = FONT_TEXT;
		kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
		g_kUIScene.RenderText(kRenderTextInfo);
	}

	CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);

	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
	}
	pWnd->SetCustomData(&iItemNo, sizeof(iItemNo));

	kInfo.Clear();
	MakeItemName(iItemNo, kInfo, wstrName);

	assert(pSrcNameWnd);
	if(pSrcNameWnd)
	{
		int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pSrcNameWnd, wstrName, WSTR_UI_REGULATIONSTR, iWidth);
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
			pWnd->SetCustomData(NULL, sizeof(iItemNo));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));
	}
}

void PgItemCookingMgr::NextDisplayResultItem(DWORD const dwNow)
{
	m_iDisplayResultItemTime = dwNow;
	++m_iDisplayResultItemIdx;
	m_iDisplayResultItemIdx = m_iDisplayResultItemIdx % m_kContResultItem.size();
}

void PgItemCookingMgr::NextResultItem()
{
	NextDisplayResultItem(BM::GetTime32());
}

bool PgItemCookingMgr::SendReqItemCooking(bool bIsTrueSend)
{
	// 필요한 돈 검사.
	int iWarnMessage = 0;
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (GetCookingNeedMoney() > iMoney)
		{
			iWarnMessage = 1500;
			goto __FAILED;
		}
	}
/*
	// Need Item Check
	const SDefItemMakingData& rkMakingData = m_kMakingData;
	const TBL_DEF_ITEM_BAG_ELEMENTS& rkNeedElements = rkMakingData.kNeedElements;
	const TBL_DEF_COUNT_CONTROL& rkNeedCount = rkMakingData.kNeedCount;
	for (int iEle=0 ; iEle<MAX_SUCCESS_RATE_ARRAY ; ++iEle)
	{
		if (!rkNeedElements.aElement[iEle])
		{
			continue;
		}

		int iEleItemNo = rkNeedElements.aElement[iEle];
		int iEleCount = rkNeedCount.aCount[iEle]*MakingCount();

		PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
		assert(pkPilot);
		if(!pkPilot)
		{
			return false;
		}
		
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
		assert(pkPlayer);
		if(!pkPlayer)
		{
			return false;
		}
		
		int const iMyCount = pkPlayer->GetInven()->GetTotalCount(iEleItemNo);
		if(iMyCount < iEleCount)
		{
			iWarnMessage = 1501;		// 재료가 이상함.
			goto __FAILED;
		}

		SItemPos kItemPos;
		if (S_OK != pkPlayer->GetInven()->GetFirstItem(iEleItemNo, kItemPos))
		{
			iWarnMessage = 1501;		// 재료가 이상함.
			goto __FAILED;
		}

		kItemSlot.kItemPos = kItemPos;
		kItemSlot.iItemCount = iEleCount;
		kReqItemMaking.kItemSlotCont.push_back(kItemSlot);
	}
*/

	if (bIsTrueSend)
	{
		SReqItemMaking kReqItemMaking;
		kReqItemMaking.iMakingType = CurMakingType();
		kReqItemMaking.iMakingNo = CurMakingNo();
		kReqItemMaking.iMakingCount = MakingCount();
		kReqItemMaking.bUseOptionItem = UseOptionItem();

		BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
		kPacket.Push(1);	// 요리 한번 하기.
		kReqItemMaking.WriteToPacket(kPacket);
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

		NETWORK_SEND(kPacket)
		XUIMgr.Close(L"SFRM_ITEM_MAKING_COOK");
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, bIsTrueSend ? "Cook_Finish" : "Cook_Cooking", 0.0f);

	return true;

__FAILED:
	if(iWarnMessage)
	{
		lwAddWarnDataTT(iWarnMessage);
	}
	return false;
}

int PgItemCookingMgr::CheckNeedItem()
{
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return -1;
	}

	if(m_spCookingData)
	{
		return m_spCookingData->CheckNeedItem(pkPlayer->GetInven(), MakingCount(), UseOptionItem());
	}
	return 0;
}

int const PgItemCookingMgr::GetNowNeedItemCount(int const iNeed) const
{
	if(m_spCookingData)
	{
		bool const bOptionItem = m_spCookingData->IsOptionItem(iNeed);
		int const iNeedItemCount = m_spCookingData->NeedItemCount(iNeed)*MakingCount();

		return (!bOptionItem || (bOptionItem && UseOptionItem())) ? iNeedItemCount : 0;
	}

	return 0;
}

int PgItemCookingMgr::MaximumCount()
{
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return -1;
	}

	if(m_spCookingData)
	{
		return m_spCookingData->MaximumCount(pkPlayer->GetInven(), RecipeCount(), UseOptionItem());
	}
	return 0;
}

void PgItemCookingMgr::UpdateResultItem()
{
	if(NULL == m_spCookingData)
	{
		return;
	}

	m_iDisplayResultItemTime = BM::GetTime32();
	m_iDisplayResultItemIdx = 0;

	m_kContResultItem.clear();
	m_spCookingData->GetResultItem(m_kContResultItem, UseOptionItem());

	BM::vstring vText( TTW(1603) );
	switch( CurMakingType() )
	{
	case EMAKING_TYPE_COOKING:
		{
		}break;
	case EMAKING_TYPE_NEW_COOKING:
		{
			vText += TTW(1617);
			vText.Replace(L"#COUNT#", m_kContResultItem.size());
		}break;
	}

	XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"SFRM_ITEM_MAKING_COOK");
	if(!pWnd){ return; }

	XUI::CXUI_Wnd *pArrowWnd = NULL;
	pArrowWnd = pWnd->GetControl(L"SFRM_COLOR");
	if(!pArrowWnd){ return; }

	pArrowWnd = pArrowWnd->GetControl(L"SFRM_SHADOW");
	if(!pArrowWnd){ return; }

	pArrowWnd = pArrowWnd->GetControl(L"FRM_ARROW2");
	if(!pArrowWnd){ return; }

	pArrowWnd->Text(vText);
}