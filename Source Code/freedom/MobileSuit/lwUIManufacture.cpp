
#include "stdafx.h"
#include "lwUI.h"
#include "lwUIManufacture.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/PgPlayer.h"
#include "PgNetwork.h"
#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "lwWorld.h"
#include "PgActor.h"
#include "lwSkillTree.h"

char const* ACTIONNAME_MAN_SPELL = "a_Man_Spell";
char const* ACTIONNAME_MAN_COOK = "a_Man_Cook";
char const* ACTIONNAME_MAN_WORKMANSHIP = "a_Man_Workmanship";

void lwManufacture::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "SendReqManufacture", lwManufacture::SendReqManufacture);
	def(pkState, "CheckCanUseManufacture", lwManufacture::CheckCanUseManufacture);

	def(pkState, "InsertBundleManMtrl", lwManufacture::InsertBundleManMtrl);
	def(pkState, "RemoveBundleManMtrl", lwManufacture::RemoveBundleManMtrl);
	def(pkState, "SetBundleManMtrlCount", lwManufacture::SetBundleManMtrlCount);
	def(pkState, "GetBundleManMtrlCount", lwManufacture::GetBundleManMtrlCount);
	def(pkState, "SendReqBundleMan", lwManufacture::SendReqBundleMan);
}

bool lwManufacture::SendReqManufacture(BYTE byScore)
{
	return g_kManufacture.SendReqItem(byScore);
}

bool lwManufacture::CheckCanUseManufacture(void)
{
	return g_kManufacture.CheckCanUse();
}

//-----대량생산 UI 관련--------
bool lwManufacture::InsertBundleManMtrl(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos)
{
	//즉석 가공 가능여부 체크
	if(!pkItem) { return false; }
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
	if(!pkItemDef) { return false; }
	if(pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) != UICT_MANUFACTURE) { return false; }


	SetBundleManMtrlIcon(pkItem->ItemNo());

	return true;
}

void lwManufacture::RemoveBundleManMtrl(void)
{
	SetBundleManMtrlIcon(0);
}

void lwManufacture::SetBundleManMtrlIcon(int iItemNo)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_BUNDLE_MANUFACTURE"));
	if(!pkTopWnd)
	{ //즉석가공 UI창이 떠있지 않다면 사용할 수 없음
		//lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(699953)), true);
		return;
	}
	/*
	//OK버튼 활성화
	XUI::CXUI_Wnd* pkRefineBtn = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(!pkRefineBtn)
	{
		return false;
	}
	*/
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>( pkTopWnd->GetControl(_T("ICON_MATERIAL")) );
	if(!pkIcon) { return; }
	int iOld = 0;
	pkIcon->GetCustomData(&iOld, sizeof(int));
	if(iItemNo == 0)
	{ //재료를 해제
		SetBundleManMtrlCount(0);
	}
	else if(iOld == 0)
	{ //재료를 최초 등록
		SetBundleManMtrlCount(1);
	}
	pkIcon->SetCustomData(&iItemNo, sizeof(int));


}

void lwManufacture::SetBundleManMtrlCount(int iCount)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_BUNDLE_MANUFACTURE"));
	if(!pkTopWnd) { return; }

	BM::vstring vsText(L"");
	vsText += iCount;

	XUI::CXUI_Wnd* pkMtrlCountWnd = pkTopWnd->GetControl(_T("SFRM_MTRL_COUNT"));
	if(!pkMtrlCountWnd) { return; }
	pkMtrlCountWnd->SetCustomData(&iCount, sizeof(int));
	pkMtrlCountWnd->Text(vsText);

	XUI::CXUI_Wnd* pkResCountWnd = pkTopWnd->GetControl(_T("SFRM_RESULT_COUNT"));
	if(!pkResCountWnd) { return; }
	pkResCountWnd->SetCustomData(&iCount, sizeof(int));
	pkResCountWnd->Text(vsText);

	XUI::CXUI_Wnd* pkBtnOk = pkTopWnd->GetControl(_T("BTN_OK"));
	if(!pkBtnOk) { return; }
	if(iCount == 0 && pkBtnOk->Enable())
	{
		pkBtnOk->Enable(false);
	}
	else if(iCount != 0 && !pkBtnOk->Enable())
	{
		pkBtnOk->Enable(true);
	}
}

int lwManufacture::GetBundleManMtrlCount(void)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_BUNDLE_MANUFACTURE"));
	if(!pkTopWnd) { return 0; }

	int iCount = 0;
	XUI::CXUI_Wnd* pkMtrlCountWnd = pkTopWnd->GetControl(_T("SFRM_MTRL_COUNT"));
	if(!pkMtrlCountWnd) { return 0; }
	if(!pkMtrlCountWnd->GetCustomData(&iCount, sizeof(int)) ) { return 0; }

	return iCount;
}

bool lwManufacture::SendReqBundleMan(void)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_BUNDLE_MANUFACTURE"));
	if(!pkTopWnd)
	{ //즉석가공 UI창이 떠있지 않다면 사용할 수 없음
		//lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(699953)), true);
		return false;
	}
	/*
	//OK버튼 활성화
	XUI::CXUI_Wnd* pkRefineBtn = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(!pkRefineBtn)
	{
		return false;
	}
	*/
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>( pkTopWnd->GetControl(_T("ICON_MATERIAL")) );
	if(!pkIcon)
	{
		return false;
	}
	int iItemNo = 0;
	if(!pkIcon->GetCustomData(&iItemNo, sizeof(int)) || iItemNo == 0 || !g_kManufacture.CheckCanUse())
	{
		return false;
	}

	int iCount = GetBundleManMtrlCount();

	BM::Stream kPacket(PT_C_M_REQ_BUNDLE_MANUFACTURE);
	kPacket.Push(iItemNo);
	kPacket.Push(iCount);
	NETWORK_SEND( kPacket );


	return true;
}

//------------------------------



PgManufacture::PgManufacture() :
	m_kSrcItemPos(), m_iSrcItemNo(0)
{
}

void PgManufacture::Clear(void)
{
	m_kSrcItemPos.Clear();
	m_iSrcItemNo = 0;
}

bool PgManufacture::SendReqItem(BYTE byScore)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv){return false;}

	PgBase_Item kSrcItem;
	if(pkInv->GetItem(m_kSrcItemPos, kSrcItem) != S_OK || kSrcItem.ItemNo() != m_iSrcItemNo)
	{ //아이템이 인벤에서 옮겨졌거나 버려졌나?
		SItemPos kItemPos;
		if(pkInv->GetFirstItem(m_iSrcItemNo, kItemPos) != S_OK) //다시 찾아라
		{
			//Msg790401: 아이템을 찾을 수 없습니다.
			return false;
		}
		m_kSrcItemPos = kItemPos;
	}




	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"FRM_MANUFACTURE_GAUGE" );
	if(!pkWnd)
	{
		return false;
	}

	bool bFinished = lua_tinker::call<bool>("IsFinishedManufactureGaugeUI");
	if(!bFinished)
	{
		return false;
	}

	BM::Stream kPacket(PT_C_M_REQ_MANUFACTURE);
	kPacket.Push(m_kSrcItemPos);
	kPacket.Push(byScore);
	NETWORK_SEND( kPacket );

	return true;
}

bool PgManufacture::CheckCanUse(void)
{
	if(m_kSrcItemPos == SItemPos::NullData() || m_iSrcItemNo == 0)
	{
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgMySkill* pkSkill = pkPlayer->GetMySkill();
	if(!pkSkill)
	{
		return false;
	}

	//아이템 유무 체크
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv){return false;}

	PgBase_Item kSrcItem;
	if(pkInv->GetItem(m_kSrcItemPos, kSrcItem) != S_OK || kSrcItem.ItemNo() != m_iSrcItemNo)
	{ //아이템이 인벤에서 옮겨졌거나 버려졌나?
		SItemPos kItemPos;
		if(pkInv->GetFirstItem(m_iSrcItemNo, kItemPos) != S_OK) //다시 찾아라
		{
			//Msg790401: 아이템을 찾을 수 없습니다.
			return false;
		}
		m_kSrcItemPos = kItemPos;
	}

	//스킬 유무 체크
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkItemDef = kItemDefMgr.GetDef(m_iSrcItemNo);
	if(!pkItemDef)
	{
		return false;
	}
	int iManType = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
	if(iManType == EMANT_SPELL)
	{
		if(!pkSkill->GetLearnedSkill(11901))
		{
			return false;
		}
	}
	else if(iManType == EMANT_COOK)
	{
		if(!pkSkill->GetLearnedSkill(11701))
		{
			return false;
		}
	}
	else if(iManType == EMANT_WORKMANSHIP)
	{
		if(!pkSkill->GetLearnedSkill(11801))
		{
			return false;
		}
	}
	else if(iManType == EMANT_AUTO)
	{
	}
	else
	{
		return false;
	}
	return true;
}

bool PgManufacture::SetMaterialItem(SItemPos const& rkItemPos, CItemDef const *pkItemDef, int iItemNo)
{
	if(rkItemPos == SItemPos::NullData() || pkItemDef == NULL || iItemNo == 0)
	{
		return false;
	}

	m_kSrcItemPos = rkItemPos;
	m_iSrcItemNo = iItemNo;

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_BUNDLE_MANUFACTURE"));
	if(pkTopWnd) //대량 가공(캐쉬템) UI창이 떠있다면 UI에 등록 후 종료
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer) { return false; }

		PgInventory *pkInv = pkPlayer->GetInven();
		if(!pkInv) { return false; }

		PgBase_Item kItem;
		if(S_OK != pkInv->GetItem(rkItemPos, kItem)) { return false; }

		lwManufacture::InsertBundleManMtrl(&kItem, rkItemPos);
		return true;
	}


	if(!CheckCanUse())
	{
		return false;
	}

	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if(!pkActor)
	{
		return false;
	}
	int iManType = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
	if(iManType == EMANT_SPELL)
	{
		pkActor->ReserveTransitAction(ACTIONNAME_MAN_SPELL);
	}
	else if(iManType == EMANT_COOK)
	{
		pkActor->ReserveTransitAction(ACTIONNAME_MAN_COOK);
	}
	else if(iManType == EMANT_WORKMANSHIP)
	{
		pkActor->ReserveTransitAction(ACTIONNAME_MAN_WORKMANSHIP);
	}
	else if(iManType == EMANT_AUTO)
	{
	}
	else
	{
		return false;
	}

	return true;
}

