#include "stdafx.h"
#include "lwUI.h"
#include "lwUITreasureChest.h"
#include "lwWorld.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "PgUIModel.h"
#include "PgMobileSuit.h"
#include "ServerLib.h"
#include "PgUISound.h"

void lwTreasureChest::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "SendReqOpenChest", lwTreasureChest::SendReqOpenChest);
	def(pkState, "DrawChestIcon", lwTreasureChest::DrawIcon);
	def(pkState, "RemoveKey", lwTreasureChest::RemoveKey);
	def(pkState, "BeginOpenAnimation", lwTreasureChest::BeginOpenAnimation);
	def(pkState, "BeginFailAnimation", lwTreasureChest::BeginFailAnimation);
	def(pkState, "SetRewardItem", lwTreasureChest::SetRewardItem);
	def(pkState, "InitChestCamera", lwTreasureChest::InitChestCamera);
	def(pkState, "OpenChest", lwTreasureChest::OpenChest);
	def(pkState, "IsAvailableChest", lwTreasureChest::IsAvailableChest);
}

void lwTreasureChest::SendReqOpenChest(void)
{
	g_kTreasureChestMgr.Send_PT_M_C_REQ_TREASURE_CHEST();
}

void lwTreasureChest::InitChestCamera(const char* szRenderModelName, const char* szRenderActorName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if( !pkModel )
	{
		return;
	}

	pkModel->SetCameraZoomMinMax(0.15f, 1.0f);

	SCameraResetInfo kInfo;
	kInfo.fYCenter = 0.68f;
	kInfo.eHeightSet = SCameraResetInfo::EHS_FIX_MODEL_VALUE;

	NiActorManager* pkAM = pkModel->GetActor( szRenderActorName );
	if( pkAM )
	{
		NiAVObject* pkRoot = pkAM->GetNIFRoot();
		if( pkRoot )
		{
			pkModel->CameraReset(pkRoot, kInfo, 1.f);
		}
	}

}

bool lwTreasureChest::Init(PgBase_Item const* pkItem)
{
	if(!pkItem)
	{
		return false;
	}
	if( XUIMgr.Get(_T("SFRM_TREASURE_CHEST")) )
	{ //같은 UI창이 이미 떠있다면 종료
		return false;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_TREASURE_CHEST"));
	if(!pkTopWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkTitle = pkTopWnd->GetControl(_T("SFRM_TITLE"));
	if(pkTitle)
	{
		XUI::CXUI_Wnd* pkTitleColor = pkTitle->GetControl(_T("SFRM_COLOR"));
		if(pkTitleColor)
		{
			XUI::CXUI_Wnd* pkTitleSdw = pkTitleColor->GetControl(_T("SFRM_SHADOW"));
			if(pkTitleSdw)
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(pkItem->ItemNo());
				const wchar_t *pNameText = NULL;
				if(pDef && GetDefString(pDef->NameNo(), pNameText))
				{
					pkTitleSdw->Text(pNameText);
				}
			}
		}
	}

	XUI::CXUI_Wnd* pkRefineBtn = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(!pkRefineBtn)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkModelWnd = pkTopWnd->GetControl(_T("FRM_MODEL"));
	if(!pkModelWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkResultWnd = pkModelWnd->GetControl(_T("FRM_REWARDSLOT"));
	if(!pkResultWnd)
	{
		return false;
	}
	pkResultWnd->Visible(false);

	XUI::CXUI_Wnd* pkRewardSlot = pkModelWnd->GetControl(_T("ICN_REWARD"));
	if(!pkRewardSlot)
	{
		return false;
	}
	pkRewardSlot->ClearCustomData();

	XUI::CXUI_Wnd* pkKeyIcon = pkModelWnd->GetControl(_T("ICN_KEY"));
	if(!pkKeyIcon)
	{
		return false;
	}
	lwUIWnd kIconWnd(pkKeyIcon);
	RemoveKey();

	return true;
}

bool lwTreasureChest::TryOpen(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos)
{
	if(!pkItem || !Init(pkItem))
	{
		return false;
	}
	g_kTreasureChestMgr.Send_PT_C_M_REQ_TREASURE_CHEST_INFO();
	return g_kTreasureChestMgr.RegisterChest(pkItem->ItemNo(), rkItemInvPos);
}

bool lwTreasureChest::InsertKey(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_TREASURE_CHEST"));
	if(!pkTopWnd)
	{ //보물상자 UI창이 떠있지 않다면 사용할 수 없음
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(699953)), true);
		return false;
	}
	//OK버튼 활성화
	XUI::CXUI_Wnd* pkRefineBtn = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(!pkRefineBtn)
	{
		return false;
	}
	//만약 애니메이션 실행 중이라면 열쇠 등록은 되지 않는다.
	float fAnimationTime = 0;
	pkRefineBtn->GetCustomData(&fAnimationTime, sizeof(fAnimationTime));
	if(fAnimationTime)
	{
		return false;
	}

	pkRefineBtn->Enable(true);

	//아이콘 표시
	XUI::CXUI_Wnd* pkModelWnd = pkTopWnd->GetControl(_T("FRM_MODEL"));
	if(!pkModelWnd)
	{
		return false;
	}
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>( pkModelWnd->GetControl(_T("ICN_KEY")) );
	if(!pkIcon)
	{
		return false;
	}
	pkIcon->SetCustomData(&pkItem->ItemNo(), sizeof(int));
	if(!g_kTreasureChestMgr.RegisterKey(pkItem->ItemNo(), rkItemInvPos)) return false;

	return true;
}

bool lwTreasureChest::IsAvailableChest(void)
{
	PgBase_Item kChest, kKey;
	return g_kTreasureChestMgr.GetChestAndKeyItem(kChest, kKey);
}

float lwTreasureChest::BeginOpenAnimation(void)
{
	SetChestAni(TC_ANI_SUCCESSED);
	g_kUISound.PlaySoundByID(L"jewelbox_open");
	return g_pkApp->GetAccumTime();
}

float lwTreasureChest::BeginFailAnimation(void)
{
	SetChestAni(TC_ANI_FAILED);
	g_kUISound.PlaySoundByID(L"jewelbox_fail");
	return 0;
}

float lwTreasureChest::OpenChest(void)
{
	if(g_kTreasureChestMgr.TestMatching())
	{
		return BeginOpenAnimation();
	}
	return BeginFailAnimation();
}


void lwTreasureChest::RemoveKey(void)
{ //UI 열쇠 해제
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_TREASURE_CHEST"));
	if(!pkTopWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkModelWnd = pkTopWnd->GetControl(_T("FRM_MODEL"));
	if(!pkModelWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkKeySlot = pkModelWnd->GetControl(_T("ICN_KEY"));
	if(!pkKeySlot)
	{
		return;
	}
	pkKeySlot->ClearCustomData(); //열쇠 아이콘 지운다.

	XUI::CXUI_Wnd* pkRefineBtn = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(pkRefineBtn)	//요청 버튼을 비활성화 시킨다.
	{
		pkRefineBtn->Enable(false);
	}
}

void lwTreasureChest::SetRewardItem(int const iItemNo)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_TREASURE_CHEST"));
	if(!pkTopWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkModelWnd = pkTopWnd->GetControl(_T("FRM_MODEL"));
	if(!pkModelWnd)
	{
		return;
	}
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>( pkModelWnd->GetControl(_T("ICN_REWARD")) );
	if(!pkIcon)
	{
		return;
	}
	XUI::CXUI_Wnd* pkBtnRefine = pkTopWnd->GetControl(_T("BTN_TRY_REFINE"));
	if(!pkBtnRefine)
	{
		return;
	}

	//결과 아이콘 슬롯의 테두리 애니메이션 활성화 시킴
	XUI::CXUI_Wnd* pkResultWnd = pkModelWnd->GetControl(_T("FRM_REWARDSLOT"));
	if(!pkResultWnd)
	{
		return;
	}
	if(iItemNo == 0)
	{
		pkIcon->ClearCustomData();
		pkResultWnd->Visible(false);
	}
	else
	{
		pkIcon->SetCustomData(&iItemNo, sizeof(int));
		pkResultWnd->Visible(true);
	}

	pkBtnRefine->Enable(true);
	pkBtnRefine->ClearCustomData();
}

void lwTreasureChest::DrawIcon(lwUIWnd kIconWnd)
{
	XUI::CXUI_Wnd* pkKeyIcon = kIconWnd.GetSelf();
	if(true == kIconWnd.IsNil() || NULL == pkKeyIcon)
	{
		return;
	}

	int iItemNo = 0;
	pkKeyIcon->GetCustomData(&iItemNo, sizeof(int));
	if(iItemNo > 0)
	{
		POINT2 ptIcon = pkKeyIcon->TotalLocation();
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
		if(pItemDef)
		{
			kIconWnd.DrawIconDetail_AddRes(pItemDef->ResNo(), ptIcon);
			g_kUIScene.RenderIcon(pItemDef->ResNo(), ptIcon, false);
		}
	}
}

void lwTreasureChest::SetChestAni(EANITYPE eAniType)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_TREASURE_CHEST"));
	if(!pkTopWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkModelWnd = pkTopWnd->GetControl(_T("FRM_MODEL"));
	if(!pkModelWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkModelViewWnd = pkModelWnd->GetControl(_T("FRM_MODELVIEW"));
	if(!pkModelViewWnd)
	{
		return;
	}

	//상황별 애니메이션 초기화
	lwUIWnd kModelWnd(pkModelWnd);
	int iActinID = 100002;
	switch(eAniType)
	{
	case TC_ANI_SUCCESSED:
		iActinID = 100001;
		break;
	case TC_ANI_FAILED:
		iActinID = 100003;
		break;
	}

	kModelWnd.SetRenderModelActorTargetAnimation("TCModel", "TCActor", iActinID);

}



PgTreasureChestMgr::PgTreasureChestMgr() :
	m_kChestPos(SItemPos::NullData()), 
	m_kKeyPos(SItemPos::NullData()),
	m_iChestItemNo(0),
	m_iKeyItemNo(0)
{
}

PgTreasureChestMgr::~PgTreasureChestMgr()
{
}

void PgTreasureChestMgr::Clear(void)
{
	m_kChestPos = SItemPos::NullData();
	m_kKeyPos = SItemPos::NullData();
	m_iChestItemNo = 0;
	m_iKeyItemNo = 0;
}

bool PgTreasureChestMgr::RegisterChest(int iItemNo, const SItemPos& kChestPos)
{
	if(kChestPos == SItemPos::NullData() || iItemNo == 0)
	{
		return false;
	}
	m_kChestPos = kChestPos;
	m_iChestItemNo = iItemNo;
	return true;
}

bool PgTreasureChestMgr::RegisterKey(int iItemNo, const SItemPos &kKeyPos)
{
	if(kKeyPos == SItemPos::NullData() || iItemNo == 0)
	{
		return false;
	}
	m_kKeyPos = kKeyPos;
	m_iKeyItemNo = iItemNo;
	return true;
}

void PgTreasureChestMgr::UnregisterKey(void)
{
	m_kKeyPos = SItemPos::NullData();
}

bool PgTreasureChestMgr::GetChestAndKeyItem(PgBase_Item& kChest, PgBase_Item& kKey)
{
	//인벤 내에서 상자위치로 찾을 수 없으면
	PgInventory* pkInv = TreasureChestUtil::GetInventory();
	if(!pkInv) //인벤 못 얻었다.
	{
		return false;
	}

	bool bFoundChest = false, bFoundKey = false;
	//인벤에서 정상적으로 아이템을 찾는다면 성공
	if(S_OK == pkInv->GetItem(m_kChestPos, kChest) && m_iChestItemNo > 0 && m_iChestItemNo == kChest.ItemNo() )
	{ //상자
		bFoundChest = true;
	}
	else if(m_iChestItemNo <= 0 || E_FAIL == pkInv->GetFirstItem(m_iChestItemNo, m_kChestPos))
	{ //아이템 위치가 무효하다면 다시 검색해서 올바른 위치를 얻는다.
		return false;
	}
	if(S_OK == pkInv->GetItem(m_kKeyPos, kKey) && m_iKeyItemNo > 0 && m_iKeyItemNo == kKey.ItemNo() )
	{ //열쇠
		bFoundKey = true;
	}
	else if(m_iKeyItemNo <= 0 || E_FAIL == pkInv->GetFirstItem(m_iKeyItemNo, m_kKeyPos))
	{
		return false;
	}

	//아이템 위치를 다시 검사한 경우라면 GetItem을 다시 시도
	if(!bFoundChest && E_FAIL == pkInv->GetItem(m_kChestPos, kChest) )
	{
		::Notice_Show(TTW(699952), EL_Warning, true);
		return false;
	}
	if(!bFoundKey && E_FAIL == pkInv->GetItem(m_kKeyPos, kKey) )
	{
		::Notice_Show(TTW(699952), EL_Warning, true);
		return false;
	}
	return true;
}

bool PgTreasureChestMgr::TestMatching(void)
{
	PgBase_Item kChestItem, kKeyItem;
	if(!GetChestAndKeyItem(kChestItem, kKeyItem))
	{
		return false;
	}

	CONT_TREASURE_CHEST::const_iterator iterChest = m_kContTableData.find(kChestItem.ItemNo());
	if(iterChest == m_kContTableData.end()) //DB 등록되지 않은 상자?
	{
		::Notice_Show(TTW(699950), EL_Warning, true);
		return false;
	}

	CONT_TREASURE_CHEST_KEY_GROUP::const_iterator iterKey = iterChest->second.kCont.find(kKeyItem.ItemNo());
	if(iterKey == iterChest->second.kCont.end()) //상자에 맞지 않는 열쇠
	{
		::Notice_Show(TTW(699951), EL_Warning, true);
		return false;
	}

	return true;
}

void PgTreasureChestMgr::Send_PT_M_C_REQ_TREASURE_CHEST(void)
{
	//요청전 유효성을 먼저 검사한다
	if(m_kChestPos == SItemPos::NullData() || m_kKeyPos == SItemPos::NullData())
	{
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_OPEN_TREASURE_CHEST);
	kPacket.Push(m_kChestPos);
	kPacket.Push(m_kKeyPos);
	kPacket.Push(lwGetServerElapsedTime32());
	NETWORK_SEND( kPacket );
}

void PgTreasureChestMgr::Recv_PT_M_C_ANS_TREASURE_CHEST(BM::Stream* pkPacket)
{
	if(pkPacket == NULL)
	{
		return;
	}
	int iRewardItemNo = 0;
	pkPacket->Pop(iRewardItemNo);
	lwTreasureChest::SetRewardItem(iRewardItemNo);
}

void PgTreasureChestMgr::Send_PT_C_M_REQ_TREASURE_CHEST_INFO(void)
{
	if(m_kContTableData.empty())
	{ //게임 시작 후 최초일 경우만 요청
		BM::Stream kPacket(PT_C_M_REQ_TREASURE_CHEST_INFO);
		kPacket.Push(lwGetServerElapsedTime32());
		NETWORK_SEND(kPacket);
	}
}

void PgTreasureChestMgr::Recv_PT_M_C_ANS_TREASURE_CHEST_INFO(BM::Stream* pkPacket)
{
	if(pkPacket == NULL)
	{
		return;
	}
	PU::TLoadTable_AM(*pkPacket, m_kContTableData);
}

PgInventory* TreasureChestUtil::GetInventory(void)
{
	PgPlayer* pkPlayer = NULL;
	PgInventory* pkInv = NULL;
	if(pkPlayer = g_kPilotMan.GetPlayerUnit())
	{
		pkInv = pkPlayer->GetInven();
	}
	return pkInv;
}