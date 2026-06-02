#include "stdafx.h"
#include "lohengrin/packetstruct.h"
#include "variant/PgGambleMachine.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "lwUICostumeMix.h"
#include "lwUICSGacha.h"
#include "lwUIIGGacha.h"
#include "lwUIQuest.h"
#include "lwCashShop.h"
#include "lwWString.h"

extern int CALLBACK ClientItemResChooser(int const iGenderLimit);
int const MAX_SLOT = MAX_GAMBLEMACHINE_MIXUP_NUM;

PgCashCostumeMix::PgCashCostumeMix()
{
	Init();
}

PgCashCostumeMix::~PgCashCostumeMix()
{
}

bool PgCashCostumeMix::Init()
{
	m_kPreview_ResultItem.Init();
	SetCurMixPoint(0);
	SetMinMixPoint(0);
	SetMaxMixPoint(0);
	SetRemainReloadCnt(0);

	m_kContMixUp.clear();
	m_kContMixUpPoint.clear();
	
	m_kCont_Slot.clear();
	m_kCont_Slot.resize(MAX_SLOT);
	ClearAllSlotUI();
		
	m_kContResultItem.clear();
	m_kContRouletteItem.clear();
	m_iRemainReloadCnt = 0;
	return true;
}

void PgCashCostumeMix::ReqResultItemList()
{// 메인 페이지를 구성하기 위해 필요한 정보를 요청
	g_kCSGacha.ReqCSGachaItemList();	// 요청은 같은 형식
}

bool PgCashCostumeMix::RecvPacket(WORD const dwPacketType, BM::Stream& rkPacket)
{
	HRESULT hResult = S_OK;
	switch(dwPacketType)
	{
	case PT_M_C_ANS_GAMBLEMACHINEINFO:
		{// 메인 페이지 구성을 위해 필요한 정보들(결과 예상 아이템)
			Init();
			g_kGambleMachine.ReadFromPacket(rkPacket);
			g_kGambleMachine.GetContMixup(m_kContMixUp);
			{// 2개 이상이면 min, max를 정할수 있으므로
				CONT_GAMBLEMACHINEMIXUP::const_iterator itor = m_kContMixUp.begin();
				if(m_kContMixUp.end() != itor)
				{// 합성을 위한 최소 Point
					SetMinMixPoint(itor->first);
				}
				CONT_GAMBLEMACHINEMIXUP::reverse_iterator ritor  = m_kContMixUp.rbegin();
				if(m_kContMixUp.rend() != ritor)
				{// 합성을 위한 최대 Point
					SetMaxMixPoint(ritor->first);
				}
			}
			g_kGambleMachine.GetContMixupPoint(m_kContMixUpPoint);
			
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return false;
			}

			CONT_GAMBLEMACHINE kContItemBag;
			CONT_GAMBLEMACHINEGROUPRES kContRareItemGroupInfo;
			g_kGambleMachine.GetCont(kContItemBag, pkPlayer->UnitRace());	// 아이템 목록을 얻고
			g_kGambleMachine.GetContRes(kContRareItemGroupInfo);	// 참고할 레어아이템 그룹 정보를 얻는다
			m_kPreview_ResultItem.SetUI_ExpectResultItem(kContItemBag, kContRareItemGroupInfo);
			
			SetPointExplanText();	// 모든 정보가 설정된 후 각 아이템이 몇 포인트가 되는지 알리는 텍스트를 설정
			SetPrevRoulletTbl();
		}break;
	case PT_M_C_ANS_USE_GAMBLEMACHINE_MIXUP_READY:
		{// 커스튬 조합 시작 결과 회신
			hResult = RecvReady(rkPacket);
		}break;
	case PT_M_C_ANS_RELOAD_ROULETTE_MIXUP:
		{// 커스튬 조합 룰렛 리셋 결과 회신
			hResult = RecvRoulletItemSetting(rkPacket);
		}break;
	case PT_M_C_ANS_ROULETTE_MIXUP_RESULT:
		{// 현재 룰렛의 당첨 아이템 받기 결과 받음
			m_kContResultItem.clear();
			rkPacket.Pop(hResult);	//HRESULT : ErrorCode.h
			if(S_OK == hResult)
			{
				PU::TLoadArray_M(rkPacket, m_kContResultItem);	//CONT_GAMBLEMACHINERESULT : 결과 통보 (PgBase_Item 리스트 PU::TLoadArray_M 으로 읽음 )
				CONT_GAMBLEMACHINERESULT::const_iterator itor = m_kContResultItem.begin();
				while(m_kContResultItem.end() != itor)
				{
					g_kCSGacha.NotifyEarnItem(*itor);
					++itor;
				}
				// 아이템을 받고 난후에 처리 사용한 슬롯 정리등을 정리한다
				ClearSlotInfo();
			}
		}break;
	case PT_M_C_NOTI_ROULETTE_MIXUP_RESULT_MAIL:
		{// 인벤토리 공간 없을 때 메일로 회신 메세지
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790631, true);
		}break;
	case PT_M_C_NFY_GAMBLEMACHINE_MIXUP_RESULT:
		{// 현재 룰렛의 당첨 아이템이 공지 설정이 되어 있으면 전 서버에 공지
			if(g_pkWorld)
			{
				switch(g_pkWorld->GetAttr())
				{
				case GATTR_EMPORIABATTLE:
				case GATTR_EMPORIA:
				case GATTR_BATTLESQUARE:
					{// 공지를 띄우면 안되는 장소
						return true;
					}break;
				default:
					{
					}break;
				}
				CONT_GAMBLEMACHINERESULT kResult;
				std::wstring kPlayerName;
				rkPacket.Pop(kPlayerName);
				PU::TLoadArray_M(rkPacket, kResult);
				if(false == kResult.empty())
				{// 한개만 알려줌
					g_kIGGacha.NotifyLuckyGuy(kPlayerName, kResult.front());
				}
			}
		}break;
	default:
		{
			return false;
		}break;
	}

	if(S_OK != hResult)
	{
		ShowErrorMsg(hResult);
	}

	return true;
}

bool PgCashCostumeMix::InsertSlotForMix(XUI::CXUI_Wnd* pkCashInvSlot)
{
	if(!pkCashInvSlot)
	{
		return false;
	}

	PgPlayer const* const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	if( !g_kGambleMachine.IsCanCostumeMixup(pkPlayer->UnitRace()) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 66, true);
		return false;
	}

	//if(!IsEnableSlot())
	//{
	//	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790726, true);
	//	return false;
	//}

	PgBase_Item kItem;
	{
		int iErrTTNo=0;
		if(!GetItemInfoFromCashInvSlot(pkCashInvSlot, kItem, iErrTTNo))
		{// 등록 가능한지 확인하고
			if(0 < iErrTTNo )
			{
				lwAddWarnDataTT(iErrTTNo);
			}
			return false;
		}
	}
	
	XUI::CXUI_Icon* pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>(pkCashInvSlot);
	if(!pkIconWnd)
	{
		return false;
	}
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkDefItem = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pkDefItem)
	{
		return false;
	}
	int const iItemMixUpPoint = g_kGambleMachine.GetMixupPoint(pkDefItem->GetAbil(AT_COSTUME_GRADE));

	{//UI에 등록 하기위해

		// 아이템 정보를 만들고
		SIconInfo const& kIconInfo = pkIconWnd->IconInfo();	
		SSlotInfo kNewItemInfo(kItem, SItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey), iItemMixUpPoint);
		if(IsExistSameItemInSlot(kNewItemInfo))
		{//  현재 아이템이 중복인지 확인 후 
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790720, true);
			return false;
		}
		int iEmptSlotIdx = -1;
		if(!GetEmptySlotIdx(iEmptSlotIdx))
		{// 슬롯이 가득 찼는지 확인하고
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790721, true);
			return false;
		}
		if(!SetSlotInfo(iEmptSlotIdx, kNewItemInfo))
		{// 중복이 아니면 적당한 슬롯에 등록하고
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790722, true);
			return false;
		}
		if(!SetSlotInfoToUI(iEmptSlotIdx, kNewItemInfo))
		{// UI에 등록해서 보여주고
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790723, true);
			return false;
		}
	}
	if(!AddMixPoint(iItemMixUpPoint))
	{// 등록 가능 한 아이템이라면 현재 포인트를 증가 시킨다
		return false;
	}
	if(GetMaxMixPoint() < GetCurMixPoint())
	{// 포인트가 최대 포인트를 넘어가면 경고 문구를 팝업 시켜주고 
		//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790725, true);
		lwAddWarnDataTT(790725, true);
		// 남은 슬롯을 닫는다 
		//return CloseAllRemainSlot();
	}
	return true;
}

bool PgCashCostumeMix::RemoveSlot(XUI::CXUI_Wnd* pkRegSlot)
{
	if(!pkRegSlot)
	{
		return false;
	}
	int iSlotIdx = 0;
	XUI::CXUI_Wnd* pkParent = pkRegSlot->Parent();
	if(pkParent)
	{// 슬롯 index를 얻어옴
		iSlotIdx = pkParent->BuildIndex();
	}

	SSlotInfo kSlotInfo;
	if(!GetSlotInfo(iSlotIdx, kSlotInfo))
	{// 정보를 얻어 올수 없으면
		return false;
	}
	
	PgBase_Item const& kItem = kSlotInfo.kItem;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkDefItem = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pkDefItem)
	{
		return false;
	}

	if(!RemoveSlotInfo(iSlotIdx))
	{//현재 UI의 idx를 찾아와 지우고,
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790724, true);
		return false;
	}
	pkRegSlot->Visible(false);
	
	int const iItemMixUpPoint = g_kGambleMachine.GetMixupPoint(pkDefItem->GetAbil(AT_COSTUME_GRADE));
	if(!AddMixPoint(-iItemMixUpPoint))
	{// 등록된 포인트를 감소(갱신) 시키고
		return false;
	}

	// 지워진 슬롯정보를 재정렬하고
	ReSortSlotInfo();
	// 슬롯 UI를 싹 지운다음에 
	ClearAllSlotUI();

	int iIdx = 0;
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	while(m_kCont_Slot.end() != itor)
	{// 남아있는 정보를 기반으로 슬롯 UI 표시를 다시
		if((*itor).bUsing)
		{
			if(!SetSlotInfoToUI(iIdx, (*itor)))
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790723, true);
				return false;
			}
		}
		++itor;
		++iIdx;
	}
	return true;
}

bool PgCashCostumeMix::GetItemInfoFromCashInvSlot(XUI::CXUI_Wnd* pkCashInvSlot, PgBase_Item& kResultItem, int& iErrTTNo)
{
	iErrTTNo = 790740;
	if(!pkCashInvSlot)
	{
		return false;
	}
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return false;
	}
	XUI::CXUI_Icon* pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>(pkCashInvSlot);
	if(!pkIconWnd)
	{
		return false;
	}

	SIconInfo const& kIconInfo = pkIconWnd->IconInfo();
	
	if(S_OK != pkInv->GetItem(SItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey), kResultItem))
	{// 캐시샵 인벤토리로 부터 아이템을 얻어와
		iErrTTNo = 0;
		return false;
	}

	{// 조건이 맞는지 체크 한다
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pkDefItem = kItemDefMgr.GetDef(kResultItem.ItemNo());
		if(!pkDefItem)
		{
			return false;
		}
		if(0 >= pkDefItem->GetAbil(AT_COSTUME_GRADE))
		{// AT_Grade  = 1 이상 
			return false;
		}		
		if(4 != pkDefItem->GetAbil(AT_PRIMARY_INV))
		{// AT_PRIMARY_INV   = 4 
			return false;
		}	
		//if(0 < pkDefItem->GetAbil(AT_WEAPON_TYPE))
		//{//AT_WEAPON_TYP == False / 무기가 아닌 것만 가능 
		//	return false;
		//}
		if(!pkDefItem->IsType(ITEM_TYPE_AIDS) 
			&& !pkDefItem->IsType(ITEM_TYPE_EQUIP)
			)
		{//(34 == [TB_Def_Item].Type = 34) 
			return false;
		}
		if( LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion() )
		{//한국 이면
			if( kResultItem.IsUseTimeOut() )
			{// 기간만료되면 넣을수 없다
				iErrTTNo = 790743;
				return false;
			}
		}
	}
	return true;
}

bool PgCashCostumeMix::AddMixPoint(int const iAdd)
{
	int const iNewPoint = m_iCurMixPoint+iAdd;
	int const iDelta = iNewPoint - m_iCurMixPoint;
	if(0 > iNewPoint
		//|| (0 < iDelta && IsFullSlot())	//MaxMixPoint를 오버할수 있는것은 한번으로 충분
		) 
	{// 현재 포인트보다 증가할때
		return false;
	}
	return SetCurMixPoint(iNewPoint);
}

bool PgCashCostumeMix::SetCurMixPoint(int const iNewPoint) 
{ 
	if(0 > iNewPoint
		//|| GetMaxMixPoint() < iNewPoint
		//|| IsFullSlot()	//MaxMixPoint를 오버할수 있는것은 한번으로 충분
		)
	{
		return false;
	}
	bool bDisableStartBtn = true;
	m_iCurMixPoint = iNewPoint;
	if(GetMinMixPoint() > 0
		&& GetMinMixPoint() <= GetCurMixPoint())
	{// 세팅된 포인트가 설정 가능할때만 메인 UI에 룰렛 테이블을 갱신한다
		SExpectRoulletTbl kRoulletTblInfo;
		if(!GetExpectRoulletTbl(m_iCurMixPoint, kRoulletTblInfo))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790727, true);
		}
		else if(!SetRoulletTblInfoToMainUI(kRoulletTblInfo))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790727, true);
		}
		else
		{
			bDisableStartBtn = false;
		}
	}
	if(bDisableStartBtn)
	{
		lua_tinker::call<void>("CostumeMix_DisablePreviewRoulletTbl_MainUI");
	}
	lua_tinker::call<void, bool>("CostumeMix_DisableStartBtn", bDisableStartBtn);
	return true;
}

bool PgCashCostumeMix::IsFullSlot()
{
	if(GetMaxMixPoint() <= GetCurMixPoint())
	{
		return true;
	}
	return false;
}

lwWString PgCashCostumeMix::GetCurrentPointState()
{
	BM::vstring vStr(TTW(790709));
	vStr.Replace(L"#CURRENT#", GetCurMixPoint());
	vStr.Replace(L"#MAX#", GetMaxMixPoint());
	return lwWString(static_cast<std::wstring>(vStr));
}

bool PgCashCostumeMix::SetPointExplanText()
{
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	BM::vstring vStr(TTW(790698));
	int iPoint  = 0;
	XUI::CXUI_Wnd* pkText = pkCostumeMixWnd->GetControl(L"SFRM_NOMAL_POINT");
	{// 일반이 몇 포인트로 환산되는가
		if(!pkText)
		{
			return false;
		}
		iPoint = g_kGambleMachine.GetMixupPoint(E_NORMAL_GRADE);
		vStr.Replace(L"#POINT#", iPoint);
		pkText->Text(vStr);
	}
	
	{// 고급이 몇 포인트로 환산되는가
		vStr = TTW(790699);
		pkText = pkCostumeMixWnd->GetControl(L"SFRM_ADV_POINT");
		if(!pkText)
		{
			return false;
		}	
		iPoint = g_kGambleMachine.GetMixupPoint(E_ADV_GRADE);
		vStr.Replace(L"#POINT#", iPoint);
		pkText->Text(vStr);
	}

	{// 스페셜이 몇 포인트로 환산되는가
		vStr = TTW(790700);
		pkText = pkCostumeMixWnd->GetControl(L"SFRM_SPCL_POINT");
		if(!pkText)
		{
			return false;
		}
		iPoint = g_kGambleMachine.GetMixupPoint(E_SPCL_GRADE);
		vStr.Replace(L"#POINT#", iPoint);
		pkText->Text(vStr);
	}

	{// 유물이 몇 포인트로 환산되는가
		vStr = TTW(790715);
		pkText = pkCostumeMixWnd->GetControl(L"SFRM_ARTIFACT_POINT");
		if(!pkText)
		{
			return false;
		}
		iPoint = g_kGambleMachine.GetMixupPoint(E_ARTIFACT_GRADE);
		vStr.Replace(L"#POINT#", iPoint);
		pkText->Text(vStr);
	}

	{// 전설이 몇 포인트로 환산되는가
		vStr = TTW(790716);
		pkText = pkCostumeMixWnd->GetControl(L"SFRM_LEGND_POINT");
		if(!pkText)
		{
			return false;
		}
		iPoint = g_kGambleMachine.GetMixupPoint(E_LEGEND_GRADE);
		vStr.Replace(L"#POINT#", iPoint);
		pkText->Text(vStr);
	}

	for(int i = 4; i >= 0; --i)
	{
		int const iGrade = E_NORMAL_GRADE+i;
		if( g_kGambleMachine.GetMixupPoint(iGrade) > 0 )
		{
			lua_tinker::call<void, lwUIWnd, int>("CostumeMix_InitMainUI", lwUIWnd(pkCostumeMixWnd), iGrade);
			break;
		}
	}

	{// MIN, MAX가 몇포인트인지 알림
		vStr = TTW(790696);
		pkText = pkCostumeMixWnd->GetControl(L"SFRM_MIN_MAX_TEXT");
		if(!pkText)
		{
			return false;
		}
		vStr.Replace(L"#MIN#", GetMinMixPoint());
		vStr.Replace(L"#MAX#", GetMaxMixPoint());
		pkText->Text(vStr);
	}
	return true;
}

bool PgCashCostumeMix::SetSlotInfo(int const iIdx, SSlotInfo const kNewSlotInfo)
{
	if(static_cast<int>(m_kCont_Slot.size()) <= iIdx)
	{// 사이즈 검사
		return false;
	}
	
	SSlotInfo& kOldSlotInfo = m_kCont_Slot.at(iIdx);
	kOldSlotInfo = kNewSlotInfo;
	kOldSlotInfo.bUsing = true;
	return true;
}

bool PgCashCostumeMix::GetSlotInfo(int const iIdx, SSlotInfo& kResult)
{
	if(static_cast<int>(m_kCont_Slot.size()) <= iIdx)
	{// 사이즈 검사
		return false;
	}
	kResult = m_kCont_Slot.at(iIdx);
	return true;
}

bool PgCashCostumeMix::RemoveSlotInfo(int const iIdx)
{
	if(static_cast<int>(m_kCont_Slot.size()) <= iIdx)
	{// 사이즈 검사
		return false;
	}
	SSlotInfo& kResult = m_kCont_Slot.at(iIdx);
	kResult.bUsing = false;
	kResult.iPoint = 0;
	return true;
}

bool PgCashCostumeMix::IsExistSameItemInSlot(SSlotInfo const& kSlotInfo)
{
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	while(m_kCont_Slot.end() != itor)
	{
		if( (*itor) == kSlotInfo )
		{
			return true;
		}
		++itor;
	}
	return false;
}

bool PgCashCostumeMix::GetEmptySlotIdx(int& iResult)
{	
	int iIdx = 0;
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	while(m_kCont_Slot.end() != itor)
	{
		if(false == itor->bUsing)
		{
			iResult = iIdx;
			return true;
		}
		++itor;
		++iIdx;
	}
	return false;
}

void PgCashCostumeMix::ReSortSlotInfo()
{//중간에 빈 슬롯을 재정렬
	CONT_SLOT kContTemp(m_kCont_Slot.size());
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	int iIdx = 0;
	while(m_kCont_Slot.end() != itor)
	{
		if(itor->bUsing)
		{
			SSlotInfo& kSlotInfo = kContTemp.at(iIdx);
			kSlotInfo = (*itor);
			++iIdx;
		}
		++itor;
	}
	m_kCont_Slot.swap(kContTemp);
}

bool PgCashCostumeMix::SetSlotInfoToUI(int const iIdx, SSlotInfo const& kSlotInfo)
{
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	BM::vstring vStr("SFRM_RESOURCE_SLOT");
	vStr+=iIdx;
	XUI::CXUI_Wnd* pkSlot = pkCostumeMixWnd->GetControl(vStr);
	if(!pkSlot)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkIcn = pkSlot->GetControl(L"ICN_SLOT");
	if(!pkIcn)
	{
		return false;
	}
	// 아이템 정보를 출력한다
	pkIcn->Visible(true);
	PgBase_Item const& kItem = kSlotInfo.kItem;
	lwDrawIconToItemNo(pkIcn, kItem.ItemNo(), 1.0f, lwPoint2F(0.5f, 0.5f));

	XUI::CXUI_Wnd* pkPoint = pkSlot->GetControl(L"FRM_POINT");
	if(pkPoint)
	{// 아이템 포인트를 갱신하고
		pkPoint->Text( static_cast<std::wstring>(BM::vstring(kSlotInfo.iPoint)) );
	}
	XUI::CXUI_Wnd* pkAni = pkSlot->GetControl(L"IMG_EMPTY_ANI");
	if(pkAni)
	{// 공백 애니를 감추고
		pkAni->Visible(false);
	}
	XUI::CXUI_Wnd* pkStroke = pkSlot->GetControl(L"IMG_STROKE");
	if(pkStroke)
	{
		pkStroke->Visible(true);
	}

	//XUI::CXUI_Wnd* pkClosed = pkSlot->GetControl(L"IMG_CLOSED");
	//if(pkClosed)
	//{// 닫힘 UI를 감춘다
	//	pkClosed->Visible(false);
	//}
	
	return true;
}

bool PgCashCostumeMix::ClearAllSlotUI()
{
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	for(int i=0; i < MAX_SLOT; ++i)
	{
		BM::vstring vStr("SFRM_RESOURCE_SLOT");
		vStr+=i;
		XUI::CXUI_Wnd* pkSlot = pkCostumeMixWnd->GetControl(vStr);
		if(pkSlot)
		{
			XUI::CXUI_Wnd* pkIcn = pkSlot->GetControl(L"ICN_SLOT");
			if(pkIcn)
			{// 아이템 정보를 출력한다
				pkIcn->Visible(false);
			}		
			XUI::CXUI_Wnd* pkPoint = pkSlot->GetControl(L"FRM_POINT");
			if(pkPoint)
			{// 아이템 포인트를 리셋하고
				pkPoint->Text( static_cast<std::wstring>(BM::vstring(0)));
			}
			XUI::CXUI_Wnd* pkAni = pkSlot->GetControl(L"IMG_EMPTY_ANI");
			if(pkAni)
			{// 공백 애니를 키고
				pkAni->Visible(true);
			}
			XUI::CXUI_Wnd* pkStroke = pkSlot->GetControl(L"IMG_STROKE");
			if(pkStroke)
			{// 아이템 장착 됐을때 외곽선
				pkStroke->Visible(false);
			}
			//XUI::CXUI_Wnd* pkClosed = pkSlot->GetControl(L"IMG_CLOSED");
			//if(pkClosed)
			//{// 닫힘 UI를 감춘다
			//	pkClosed->Visible(false);
			//}
		}
	}
	return true;
}

bool PgCashCostumeMix::IsEnableSlot() const
{// 더이상 슬롯에 등록이 가능한가?
	if(GetMaxMixPoint() <= GetCurMixPoint())
	{// 포인트가 최대 포인트이상 이면
		return false;
	}

	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	CONT_SLOT::size_type stUseCnt = 0;
	while(m_kCont_Slot.end() != itor)
	{
		if(itor->bUsing)
		{
			++stUseCnt;
		}
		++itor;
	}
	if(m_kCont_Slot.size() == stUseCnt)
	{// 모든 슬롯이 사용중이라면 
		return false;
	}	
	return true;
}

bool PgCashCostumeMix::CloseAllRemainSlot()
{
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	bool bResult = true;
	int iIdx = 0;
	while(m_kCont_Slot.end() != itor)
	{
		if(!itor->bUsing)
		{// 사용하지 않는 슬롯
			bResult = CloseSlotUI(iIdx);
		}
		++itor;
		++iIdx;
	}
	return bResult;
}

bool PgCashCostumeMix::CloseSlotUI(int const iIdx)
{
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	BM::vstring vStr("SFRM_RESOURCE_SLOT");
	vStr+=iIdx;
	XUI::CXUI_Wnd* pkSlot = pkCostumeMixWnd->GetControl(vStr);
	if(!pkSlot)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkIcn = pkSlot->GetControl(L"ICN_SLOT");
	if(!pkIcn)
	{
		return false;
	}
	// 아이템 정보를 출력한다
	pkIcn->Visible(false);
	//PgBase_Item const& kItem = kSlotInfo.kItem;
	//lwDrawIconToItemNo(pkIcn, kItem.ItemNo(), 1.0f, lwPoint2F(0.5f, 0.5f));

	XUI::CXUI_Wnd* pkPoint = pkSlot->GetControl(L"FRM_POINT");
	if(pkPoint)
	{// 아이템 포인트를 부분을 삭제하고
		pkPoint->Text(L"");
	}
	XUI::CXUI_Wnd* pkAni = pkSlot->GetControl(L"IMG_EMPTY_ANI");
	if(pkAni)
	{// 공백 애니를 감추고
		pkAni->Visible(false);
	}
	XUI::CXUI_Wnd* pkStroke = pkSlot->GetControl(L"IMG_STROKE");
	if(pkStroke)
	{
		pkStroke->Visible(true);
	}
	//XUI::CXUI_Wnd* pkClosed = pkSlot->GetControl(L"IMG_CLOSED");
	//if(pkClosed)
	//{// 닫힘 UI를 보인다
	//	pkClosed->Visible(true);
	//}
	return true;
}

bool PgCashCostumeMix::GetExpectRoulletTbl(int iPoint, SExpectRoulletTbl& kResult) const
{
	if(GetMinMixPoint() > iPoint
		|| m_kContMixUp.empty()
		)
	{
		return false;
	}
	if(GetMaxMixPoint() < iPoint)
	{// 최대 포인트가 넘어도 테이블의 정보는 최대 포인트 값
		iPoint = GetMaxMixPoint();
	}

	
	CONT_GAMBLEMACHINEMIXUP::const_iterator kMixup_itor = m_kContMixUp.find(iPoint);
	CONT_GAMBLEMACHINEMIXUP::const_iterator kClosestItor = m_kContMixUp.end();
	if(m_kContMixUp.end() == kMixup_itor)
	{// 포인트에 해당하는 예측되는 결과가 없으면
		kMixup_itor = m_kContMixUp.begin();
		int iDist = 0;
		{
			CONT_GAMBLEMACHINEMIXUP::const_reverse_iterator kRearItor = m_kContMixUp.rbegin();
			if(m_kContMixUp.rend() != kRearItor)
			{
				iDist = kRearItor->first;
			}
			else
			{
				return false;
			}
		}
		while(m_kContMixUp.end() != kMixup_itor)
		{// 가장 현재 입력된 점수보다 작으면서 차이가 제일 적은 정보를 얻어온다
			int const iCurDist =  iPoint - kMixup_itor->first;
			if(0 < iCurDist
				&& iDist > iCurDist
				)
			{
				iDist = iCurDist;
				kClosestItor = kMixup_itor;
			}
			++kMixup_itor;
		}
		if(m_kContMixUp.end() == kClosestItor)
		{// 못찾았으면 
			return false;
		}
		kMixup_itor = kClosestItor;
	}

	kResult = SExpectRoulletTbl();	//결과값 초기화 

	CONT_GAMEBLEMACHINEMIXUPLELEMENT const& kContItem = kMixup_itor->second.kCont;	// 아이템 컨테이너를 얻어와
	CONT_GAMEBLEMACHINEMIXUPLELEMENT::const_iterator kMixupItem_itor = kContItem.begin();
	while(kContItem.end() != kMixupItem_itor)
	{
		SGAMBLEMACHINEMIXUPELEMENT const& iItemInfo = (*kMixupItem_itor);
		//iItemInfo.iGradeNo; 이걸 다시 컨테이너에서 찾아야 되나? m_kContItemBag  여기에서 또 찾아야 하나?
		switch(iItemInfo.iGradeNo)
		{
		case GCG_COSTUMEMIX_NORMAL:	{ ++kResult.iNomalCnt;	} break;
		case GCG_COSTUMEMIX_ADV:	{ ++kResult.iAdvCnt;	} break;
		case GCG_COSTUMEMIX_SPCL:	{ ++kResult.iSpclCnt;	} break;
		case GCG_COSTUMEMIX_ARTIFACT:	{ ++kResult.iArtifactCnt;	} break;
		case GCG_COSTUMEMIX_LEGND:	{ ++kResult.iLegendCnt;	} break;
		default:{}break;
		}
		++kMixupItem_itor;
	}
	return true;
}

bool PgCashCostumeMix::SetRoulletTblInfoToMainUI(SExpectRoulletTbl const& kRoulletTblInfo)
{
	lua_tinker::call<void, int, int >("CostumeMix_SetPreviewRoulletTbl_MainUI", 
		kRoulletTblInfo.iLegendCnt, kRoulletTblInfo.iArtifactCnt, kRoulletTblInfo.iSpclCnt, kRoulletTblInfo.iAdvCnt);
	return true;
}

bool PgCashCostumeMix::SetRoulletTblInfoToSubUI(SExpectRoulletTbl const& kRoulletTblInfo, int const iPoint)
{
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkMix = 	pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkMix)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpect = 	pkMix->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpect)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkPreviewPage = 	pkExpect->GetControl(L"FRM_PREVIEW_ROULLET_TBL");
	if(!pkPreviewPage)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkText =	pkPreviewPage->GetControl(L"SFRM_EXPECT_TEXT");
	if(!pkText)
	{
		return false;
	}
	BM::vstring vStr(TTW(790712));
	vStr.Replace(L"#PREVIEW_POINT#", iPoint);
	pkText->Text(vStr);

	lua_tinker::call<void, int, int >("CostumeMix_SetPreviewRoulletTbl_UnderTab", 
		kRoulletTblInfo.iLegendCnt, kRoulletTblInfo.iArtifactCnt, kRoulletTblInfo.iSpclCnt, kRoulletTblInfo.iAdvCnt);
	return true;
}

void PgCashCostumeMix::ReqMixUpReady()
{// 조합 시작 버튼 누를때
	CONT_ITEMPOS kContItemPos; // 조합에 사용할 아이템 위치 리스트 
	int iCnt = 0;
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	while(m_kCont_Slot.end() != itor)
	{
		if(MAX_GAMBLEMACHINE_MIXUP_NUM < iCnt)
		{// 서버에서 허용하는 아이템 조합 갯수를 넘었을 경우(일반적으로 생길수 없음)
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790728, true);
			return;
		}
		if(itor->bUsing)
		{
			kContItemPos.insert(itor->kInvPos);
			++iCnt;
		}
		++itor;
	}
	int const iSlotMaxCnt = lua_tinker::call<int>("CostumeMix_GetSlotMaxCnt");
	if(iSlotMaxCnt != iCnt)
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790734, true);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_GAMBLEMACHINE_MIXUP_READY);
	PU::TWriteArray_A(kPacket, kContItemPos);

	if( LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion() )
	{
		lwCallCommonMsgYesNoBox(MB(TTW(790789)), lwPacket(&kPacket), true, MBT_NONE_COSTUME_ROULLET);
	}
	else
	{
		NETWORK_SEND(kPacket);
	}
}

void PgCashCostumeMix::ReqRoulletReload()
{// 룰렛 리로드 요청
	BM::Stream kPacket(PT_C_M_REQ_RELOAD_ROULETTE_MIXUP);
	NETWORK_SEND(kPacket);
}
void PgCashCostumeMix::ReqGetResultItem()
{// 룰렛이 끝나서 받을 아이템을 요청함
	BM::Stream kPacket(PT_C_M_REQ_ROULETTE_MIXUP_RESULT);
	NETWORK_SEND(kPacket);
}

HRESULT PgCashCostumeMix::RecvReady(BM::Stream& rkPacket)
{// 리로드 카운트를갱신
	HRESULT hResult = S_OK;
	rkPacket.Pop(hResult);
	if(S_OK == hResult)
	{
		int iReloadCnt = 0;
		rkPacket.Pop(iReloadCnt);
		SetRemainReloadCnt(iReloadCnt);
	}
	return hResult;
}
HRESULT PgCashCostumeMix::RecvRoulletItemSetting(BM::Stream& rkPacket)
{
	m_kContResultItem.clear();
	m_kContRouletteItem.clear();
		
	HRESULT hResult = S_OK;
	rkPacket.Pop(hResult);
	if(S_OK == hResult)
	{
  		PU::TLoadArray_M(rkPacket, m_kContResultItem);	// 결과 아이템		당첨 아이템 리스트
		PU::TLoadArray_A(rkPacket, m_kContRouletteItem); // 룰렛판에 올라갈 아이템 리스트
		rkPacket.Pop(m_iRemainReloadCnt);				// Int : 남은 룰랫 리로드 횟수
		if( LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion() )
		{// 한국이면 바로 결과 아이템을 받을수 있게 요청함
			ReqGetResultItem();
		}
		else
		{
			lua_tinker::call<bool>("CostumeMix_CallRoulette");			// 최초 호출(남은 룰렛횟수로 판단)시 UI를 호출
			SetItemOnRouletteUI();							// 룰렛 아이템에서 아이템 세팅,화살표가 가르킬 결과 아이템도 세팅
			UpdateReloadTextUI();							// 룰렛 재설정 횟수를 UI에 세팅
		}

	}
	return hResult;
}

bool PgCashCostumeMix::DecRemainReloadCnt()
{
	if(0 >= m_iRemainReloadCnt)
	{
		return false;
	}
	--m_iRemainReloadCnt;
	UpdateReloadTextUI();
	return true;
}

bool PgCashCostumeMix::ShowErrorMsg(HRESULT const& hResult) const
{
	return g_kCSGacha.ShowErrorMsg(hResult);
}

void PgCashCostumeMix::SetItemOnRouletteUI()
{
	int const MAX_IDX = 10;
	int const DUMMY_ITEM_CNT = MAX_IDX-1;

	XUI::CXUI_Wnd* pkRoulette = XUIMgr.Get(L"FRM_COSTUMEMIX_ROULETTE");
	if(!pkRoulette)
	{
		return;
	}

	CONT_GAMBLEMACHINERESULT::const_iterator kResult_itor = m_kContResultItem.begin();
	if(m_kContResultItem.end() == kResult_itor)
	{
		return;
	}
	
	int iResultItemNo = 0;
	{// 결과 아이템을 배치
		PgBase_Item kResultItem = (*kResult_itor);
		m_iResultItemIdx = BM::Rand_Index(MAX_IDX);
		BM::vstring vStr(L"ICN_ROULETTE_ITEM");
		vStr+= m_iResultItemIdx;
		XUI::CXUI_Wnd* pkIcn= pkRoulette->GetControl(vStr);
		if(pkIcn)
		{
			iResultItemNo = static_cast<int>(kResultItem.ItemNo());

			CONT_GAMBLEITEM::const_iterator kDummyItem_itor = m_kContRouletteItem.begin();
			while(kDummyItem_itor != m_kContRouletteItem.end())
			{// 결과 아이템을 찾아서 갯수를 UI에 표현하기 위함
				SGAMBLEMACHINEITEM const& rkItemInfo = (*kDummyItem_itor);
				if(iResultItemNo == rkItemInfo.iItemNo)
				{
					if(pkIcn)
					{
						pkIcn->SetCustomData(&rkItemInfo, sizeof(rkItemInfo));
						pkIcn->Text(BM::vstring(rkItemInfo.siCount));
						lwDrawIconToItemNo(pkIcn, rkItemInfo.iItemNo, 1.0f, lwPoint2F(0.5f, 0.5f));
						{
							int const iCostumeMixItemGrade = GetItemMixGrade(rkItemInfo.iItemNo);
							lua_tinker::call<void, int, int>("CostumeMix_SetRoulletTbl", m_iResultItemIdx, iCostumeMixItemGrade);
						}
						break;
					}
				}
				++kDummyItem_itor;
			}
		}
	}

	int const iResultItemIdx = m_iResultItemIdx;
	
	PgCashCostumeMix_ExpectResultItemUI::CONT_INT kContNumber;
	kContNumber.reserve(DUMMY_ITEM_CNT);

	for(int i=0; i < MAX_IDX; ++i)
	{// 0~9까지 idx 중
		if(iResultItemIdx != i)
		{// 랜덤으로 정해진 결과값으로 쓸 idx와 같지 않다면 임시 컨테이너에 넣고
			kContNumber.push_back(i);
		}
	}
	
	bool bPassedResultItem = false;
	CONT_GAMBLEITEM::const_iterator kDummyItem_itor = m_kContRouletteItem.begin();
	while(kDummyItem_itor != m_kContRouletteItem.end())
	{
		SGAMBLEMACHINEITEM const& rkItemInfo = (*kDummyItem_itor);
		
		if(iResultItemNo != rkItemInfo.iItemNo
			|| bPassedResultItem)
		{
			if(!kContNumber.empty())
			{// 0~size까지 랜덤으로 돌림
				BM::vstring vStr(L"ICN_ROULETTE_ITEM");
				int const iIdx = BM::Rand_Index(kContNumber.size());
				int const iRandIdx = kContNumber.at(iIdx);
				vStr+=iRandIdx;
				XUI::CXUI_Wnd* pkIcn= pkRoulette->GetControl(vStr);
				if(pkIcn)
				{// 나온값 UI에 하나씩 세팅하고, 
					pkIcn->SetCustomData(&rkItemInfo, sizeof(rkItemInfo));
					pkIcn->Text(BM::vstring(rkItemInfo.siCount));
					lwDrawIconToItemNo(pkIcn, rkItemInfo.iItemNo, 1.0f, lwPoint2F(0.5f, 0.5f));
					{
						int const iCostumeMixItemGrade = GetItemMixGrade(rkItemInfo.iItemNo);
						lua_tinker::call<void, int, int>("CostumeMix_SetRoulletTbl", iRandIdx, iCostumeMixItemGrade);
					}
				}

				{//kContNumber에서 하나씩 제거함
					PgCashCostumeMix_ExpectResultItemUI::CONT_INT::iterator kNum_itor = kContNumber.begin();
					while(kNum_itor !=kContNumber.end())
					{
						if( (*kNum_itor) == iRandIdx)
						{
							kNum_itor = kContNumber.erase(kNum_itor);
							break;
						}
						else
						{
							++kNum_itor;
						}
					}
				}
			}
		}
		else
		{
			bPassedResultItem = true;
		}
		++kDummyItem_itor;
	}
	lua_tinker::call<void, int>("Roulette_SetResultIdx", iResultItemIdx);
}

void PgCashCostumeMix::UpdateReloadTextUI()
{
	XUI::CXUI_Wnd* pkRoulette = XUIMgr.Get(L"FRM_COSTUMEMIX_ROULETTE");
	if(!pkRoulette)
	{
		return;
	}

	XUI::CXUI_Wnd* pkReloadText= pkRoulette->GetControl(L"SFRM_RELOAD_CNT");
	if(!pkReloadText)
	{
		return;
	}
	BM::vstring vStr(GetRemainReloadCnt());
	pkReloadText->Text(vStr);
}

int PgCashCostumeMix::GetItemMixGrade(int const iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkDefItem = kItemDefMgr.GetDef(iItemNo);
	if(!pkDefItem)
	{
		return E_NONE;
	}
	return pkDefItem->GetAbil(AT_COSTUME_GRADE);
}

void PgCashCostumeMix::ClearSlotInfo()
{
	SetCurMixPoint(0);
	SetRemainReloadCnt(0);
	
	m_kCont_Slot.clear();
	m_kCont_Slot.resize(MAX_SLOT);
	ClearAllSlotUI();

	m_kContResultItem.clear();
	m_kContRouletteItem.clear();
	lua_tinker::call<void>("CostumeMix_HideSeletedCover");	// 인벤토리에 붉은색 커버를 제거
}

bool PgCashCostumeMix::SetPrevRoulletTbl()
{
	if(m_kContMixUp.empty())
	{
		return false;
	}

	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkMix = 	pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkMix)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpect = 	pkMix->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpect)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkPreviewPage = 	pkExpect->GetControl(L"FRM_PREVIEW_ROULLET_TBL");
	if(!pkPreviewPage)
	{
		return false;
	}
	XUI::CXUI_List* pkPreviewList = dynamic_cast<XUI::CXUI_List*>(pkPreviewPage->GetControl(L"LST_PREVIEW"));	 
	if(!pkPreviewList)
	{
		return false;
	}
	pkPreviewList->ClearList();
	
	int iSlotCntInOneEle = 0;	// 리스트 원소 하나에 슬롯이 몇개가 있는가?
	XUI::SListItem* pkAddedItem = pkPreviewList->AddItem(L"");
	if(!pkAddedItem)
	{
		return false;
	}
	XUI::CXUI_Builder* pkBuilder = 
		dynamic_cast<XUI::CXUI_Builder*>(pkAddedItem->m_pWnd->GetControl(L"BLD_BTN_PREVIEW"));
	if(!pkBuilder)
	{
		return false;
	}
	iSlotCntInOneEle = pkBuilder->CountX() * pkBuilder->CountY();
	if(0 == iSlotCntInOneEle)
	{
		pkPreviewList->ClearList();
		return false;
	}
	pkPreviewList->ClearList();	// 다시 싹 지우고 

	int iNeedElemCnt = static_cast<int>(m_kContMixUp.size())/iSlotCntInOneEle;
	if(0 != static_cast<int>(m_kContMixUp.size())%iSlotCntInOneEle)
	{// 딱 나눠 떨어지는게 아니라면 한줄 더 생성
		++iNeedElemCnt;
	}
	
	CONT_GAMBLEMACHINEMIXUP::const_iterator kMixup_itor = m_kContMixUp.begin();
	for(int i=0; i < iNeedElemCnt; ++i)
	{// 필요한 만큼 버튼 라인(라인당 iSlotCntInOneEle개의 버튼이 있음)을 추가하고
		XUI::SListItem* pkItem = pkPreviewList->AddItem(L"");
		if(pkItem)
		{
			for(int j=0; j < iSlotCntInOneEle; ++j)
			{
				BM::vstring vStr(L"BTN_PREVIEW");
				vStr+=j;
				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkItem->m_pWnd->GetControl(vStr));
				if(pkBtn)
				{// 그 버튼에서 넣을수 있을 만큼
					if(kMixup_itor != m_kContMixUp.end())
					{// Point 를 넣어주고
						int const iPoint = kMixup_itor->first;
						pkBtn->SetCustomData(&iPoint, sizeof(iPoint));	//이 point값으로 미리보기 룰렛을 세팅
						{
							XUI::CXUI_Wnd* pkText =  pkBtn->GetControl(L"FRM_TEXT");
							if(pkText)
							{
								pkText->Text(BM::vstring(iPoint));
							}
						}
						
						++kMixup_itor;
					}
					else
					{// 충분히 다 넣었으면 남는 버튼은 감춘다
						pkBtn->Visible(false);
					}
				}
			}
		}
	}
	//모든 설정이 되었으므로, 최소 point에 해당하는 미리 보기 룰렛을 설정 한다
	kMixup_itor = m_kContMixUp.begin();
	SExpectRoulletTbl kRoulletTblInfo;
	GetExpectRoulletTbl(kMixup_itor->first, kRoulletTblInfo);
	SetRoulletTblInfoToSubUI(kRoulletTblInfo, kMixup_itor->first);
	return true;
}

bool PgCashCostumeMix::ShowSubUIPreviewRoulletTbl(int const iPoint)
{
	SExpectRoulletTbl kRoulletTblInfo;
	GetExpectRoulletTbl(iPoint, kRoulletTblInfo);
	return SetRoulletTblInfoToSubUI(kRoulletTblInfo, iPoint);
}

bool PgCashCostumeMix::IsInSlot(XUI::CXUI_Wnd* pkCashInvSlot)
{
	if(!pkCashInvSlot)
	{
		return false;
	}
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return false;
	}
	XUI::CXUI_Icon* pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>(pkCashInvSlot);
	if(!pkIconWnd)
	{
		return false;
	}

	SIconInfo const& kIconInfo = pkIconWnd->IconInfo();
	//if(S_OK != pkInv->GetItem(SItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey), kResultItem))
	//{// 캐시샵 인벤토리로 부터 아이템을 얻어와
	//	return false;
	//}
	SItemPos const kItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey);
	CONT_SLOT::const_iterator itor = m_kCont_Slot.begin();
	while(m_kCont_Slot.end() != itor)
	{
		if(itor->bUsing 
			&& kItemPos == itor->kInvPos
			)
		{
			return true;
		}
		++itor;
	}
	return false;
}

bool PgCashCostumeMix::CallSlotItemToolTip(int const iIdx)
{
	if(m_kCont_Slot.size() <= iIdx)
	{
		return false;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return false;
	}
	CONT_SLOT::value_type const& kSlotInfo = m_kCont_Slot.at(iIdx);
	
	PgBase_Item kItemInfo;
	if(S_OK != pkInv->GetItem(kSlotInfo.kInvPos, kItemInfo))
	{// 캐시샵 인벤토리로 부터 아이템을 얻어와
		return false;
	}
	
	CallToolTip_ItemNo(kItemInfo.ItemNo(), lwGetCursorPos(), false, false, false,
		static_cast<int>(kItemInfo.EnchantInfo().TimeType()), 
		static_cast<int>(kItemInfo.EnchantInfo().UseTime()));
	return true;
}

/// - - - - - -------------------------------------------------------------------------------------------

int const PgCashCostumeMix_ExpectResultItemUI::MAX_ITEM_CNT_ON_PAGE = 24;
int const PgCashCostumeMix_ExpectResultItemUI::BUNDLE_UNIT = 4;

PgCashCostumeMix_ExpectResultItemUI::PgCashCostumeMix_ExpectResultItemUI()
{
	Init();
}

PgCashCostumeMix_ExpectResultItemUI::~PgCashCostumeMix_ExpectResultItemUI()
{
}

void PgCashCostumeMix_ExpectResultItemUI::Init()
{
	m_iCurrentPage = 0;
//	m_iRemainReloadTime = 0;
//	m_iResultItemIdx = 0;
	m_iMaxPage = 0;
	m_iCurrentPageBundle = 0;

	m_kContItemBag.clear();
	m_kContItemPage.clear();

//	m_kContRouletteItem.clear();
//	m_kContResultItem.clear();

	m_kContRareItem.clear();
	m_kContRareItemGroupInfo.clear();
	m_kContSortedRareGroupNo.clear();

	m_iCurrentRareItemPage = 0;

	// 가챠에서 사용한 미리 보기 삭제
	ClearPreViewItem();
}

bool PgCashCostumeMix_ExpectResultItemUI::SetUI_ExpectResultItem(CONT_GAMBLEMACHINE const& kContItemBag,CONT_GAMBLEMACHINEGROUPRES const& kContRareItemGroupInfo)
{
	Init();
	m_kContItemBag = kContItemBag;
	m_kContRareItemGroupInfo = kContRareItemGroupInfo;
	{// 처음 들어온거니 '일반 탭'부분을 먼저 보여준다
		//보기 기능은, 일반, 고급, 스페셜이 되어야 한다
		return  ShowPart(GCG_COSTUMEMIX_NORMAL); // private 부분이 되어야 겠지
	}
}

bool PgCashCostumeMix_ExpectResultItemUI::ShowPart(eGambleCoinGrade eType)
{
	{// 요게 하나의 보이기 기능이 되야 되는 부분
		m_kContItemPage.clear();
		m_kContRareItem.clear();
		m_iMaxPage = 0;
		m_iCurrentPage = 0;
		SetCurrentPageBundle(0);

		CONT_GAMBLEMACHINE::iterator Gm_itor = m_kContItemBag.find(eType);
		if(Gm_itor == m_kContItemBag.end())
		{
			return false;
		}

		int iPageIdx = 0;	// UI에서 표시할 Page
		int iCnt = 0;	// 현재 Page에 들어갈 아이템 갯수를 카운트

		SGAMBLEITEMBAG const & rkContItemBag = Gm_itor->second;
		CONT_GAMBLEITEM const & rkContGambleItem = rkContItemBag.kCont;
		{// 아이템 이미지를 보여주기 위해 아이템 번호들을 따로 모은다.
			CONT_GAMBLEITEM kContItemInfo;
			CONT_GAMBLEITEM::const_iterator NextItem_itor = rkContGambleItem.begin();
			while(NextItem_itor != rkContGambleItem.end())
			{
				CONT_GAMBLEITEM::const_iterator CurItem_itor = NextItem_itor++;

				kContItemInfo.push_back(*CurItem_itor);
				++iCnt;

				if(PgCashCostumeMix_ExpectResultItemUI::MAX_ITEM_CNT_ON_PAGE <= iCnt
					|| rkContGambleItem.end() == NextItem_itor
					)
				{// 한페이지에 표현할수 있는 제한을 넘었다면 컨테이너에 넣고( 0 Page가 UI에서는 1 Page)
					m_kContItemPage.insert(std::make_pair(iPageIdx++, kContItemInfo));
					kContItemInfo.clear(); // 임시 컨테이너를 비우고
					iCnt = 0;			// 카운터를 초기화 하여, 다음 페이지 아이템 정보를 넣을수 있게함
					++m_iMaxPage;
				}

				int const iItem_GroupNo = CurItem_itor->iGroupNo;
				if(0 < iItem_GroupNo)
				{// 그룹번호가 있으면 레어아이템이므로
					CONT_RARE_ITEM::iterator Rare_itor = m_kContRareItem.find(iItem_GroupNo);
					if(m_kContRareItem.end() == Rare_itor)
					{// 새로운 그룹 번호라면 만들어 추가하고
						CONT_GAMBLEITEM kCont;
						kCont.push_back((*CurItem_itor));
						m_kContRareItem.insert(std::make_pair(iItem_GroupNo, kCont));
					}
					else
					{// 존재 하는 그룹 번호라면 존재하는곳에 추가 한다
						CONT_GAMBLEITEM& kCont = Rare_itor->second;
						kCont.push_back((*CurItem_itor));
					}
				}
			}
		}
		{// 단일 아이템 List UI
			ShowItemPage(0);		// 0번 인덱스(1page)를 보여준다
		}

		{// 레어 아이템 그룹 UI
			//XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
			//if(!pkCashShop)
			//{
			//	return;
			//}

			//XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_CASH_GACHA");
			//if(!pkCostumeMixWnd)
			//{
			//	return;
			//}

			{//정렬
				m_kContSortedRareGroupNo.clear();
				m_kContSortedRareGroupNo.reserve(m_kContRareItemGroupInfo.size());
				CONT_GAMBLEMACHINEGROUPRES::const_iterator RareInfo_itor = m_kContRareItemGroupInfo.begin();
				while(m_kContRareItemGroupInfo.end() != RareInfo_itor)
				{
					CONT_RARE_ITEM::const_iterator  Rare_itor = m_kContRareItem.find(RareInfo_itor->first); 
					if(m_kContRareItem.end() != Rare_itor)
					{
						m_kContSortedRareGroupNo.push_back(RareInfo_itor->first);
					}

					++RareInfo_itor;
				}

				// iDisplay가 낮은 순서대로 정렬한다(아이템이 보일 순서)
				std::sort(m_kContSortedRareGroupNo.begin(), m_kContSortedRareGroupNo.end(), std::less<CONT_INT::value_type>());
			}
			// 레어 그룹 출력
			ShowRareItemPage(0);
		}

	}

	return true;
}

bool PgCashCostumeMix_ExpectResultItemUI::ShowItemPage(int const iPageIdx)
{
	if(m_kContItemPage.empty())
	{
		return false;
	}

	CONT_CSGACHA_PAGE::const_iterator itemPg_itor = m_kContItemPage.find(iPageIdx);
	if(m_kContItemPage.end() == itemPg_itor)
	{
		return false;
	}

	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpectResultWnd = pkCostumeMixWnd->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpectResultWnd)
	{
		return false;
	}

	XUI::CXUI_Wnd* pkItemListWnd = pkExpectResultWnd->GetControl(L"FRM_ITEM_LIST");
	if(!pkItemListWnd)
	{
		return false;
	}

	{
		XUI::CXUI_Builder* pkBldPage  = static_cast<XUI::CXUI_Builder*>(pkItemListWnd->GetControl(L"BLD_PAGE"));
		if(pkBldPage)
		{
			int const iTotal = pkBldPage->CountX()*pkBldPage->CountY();
			for(int i=0; i < iTotal; ++i)
			{
				BM::vstring vStr(L"CBTN_PAGE");
				vStr+=i;
				XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkItemListWnd->GetControl(vStr));
				if(pkChkBtn)
				{// page에 표시할 아이템이
					int const iCorrectlyPageIdx = i+GetCurrentPageBundle()*BUNDLE_UNIT;
					if(iCorrectlyPageIdx < m_iMaxPage
						&& i < static_cast<int>(m_kContItemPage.size())
						)
					{// 있으면 버튼에 번호를 넣고
						BM::vstring vStrNo(iCorrectlyPageIdx+1);
						pkChkBtn->Text(vStrNo);
						pkChkBtn->SetCustomData(&iCorrectlyPageIdx, sizeof(iCorrectlyPageIdx));
						if(iPageIdx == iCorrectlyPageIdx)
						{// 첫번째 page 버튼에만 클릭된 표시하고
							pkChkBtn->Check(true);
							pkChkBtn->ClickLock(true);
						}
						else
						{// 나머지는 해제한다
							pkChkBtn->Check(false);
							pkChkBtn->ClickLock(false);
						}
					}
					else
					{// 없으면 더이상 번호를 넣지 않는다
						pkChkBtn->Text(L"");
						pkChkBtn->ClearCustomData();
						pkChkBtn->Check(false);
						pkChkBtn->ClickLock(true);
					}
				}
			}
		}
	}

	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkItemListWnd->GetControl(L"BLD_SINGLE_ITEM"));
	if(!pkBld)
	{
		return false;
	}
	int const iTotal = pkBld->CountX()*pkBld->CountY();


	CONT_GAMBLEITEM const& kContItemInfo = itemPg_itor->second;
	if(kContItemInfo.empty())
	{
		return false;
	}

	{// 아이템 아이콘을 모두 감추고
		HideAllItemIcon();
	}

	{// ItemPage의 아이콘에 보여질 아이템 번호를 세팅한다
		int iCnt = 0;
		CONT_GAMBLEITEM::const_iterator itemNo_itor = kContItemInfo.begin();
		while(kContItemInfo.end() != itemNo_itor)
		{
			BM::vstring vStr(L"ICN_SINGLE_ITEM");
			vStr+=iCnt;
			XUI::CXUI_Wnd* pkIcon  = pkItemListWnd->GetControl(vStr);
			if(pkIcon)
			{
				SGAMBLEMACHINEITEM const& kItemInfo = (*itemNo_itor);
				pkIcon->Visible(true);
				pkIcon->SetCustomData(&kItemInfo, sizeof(kItemInfo));
				pkIcon->Text(BM::vstring(kItemInfo.siCount));
				lwDrawIconToItemNo(pkIcon, kItemInfo.iItemNo, 1.0f, lwPoint2F(0.5f, 0.5f));
			}

			++iCnt;
			if(iTotal == iCnt)
			{
				break;
			}
			++itemNo_itor;
		}
	}
	m_iCurrentPage = iPageIdx;
	return true;
}

bool PgCashCostumeMix_ExpectResultItemUI::ShowRareItemPage(int const iPage)
{
	int const iMaxPage = GetMaxRareItemPage();
	if(0 > iPage || iMaxPage < iPage)
	{
		return false;
	}
		
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpectResultWnd = pkCostumeMixWnd->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpectResultWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkItemListWnd = pkExpectResultWnd->GetControl(L"FRM_ITEM_LIST");
	if(!pkItemListWnd)
	{
		return false;
	}


	int const iMaxRareGroupCnt = static_cast<int>(m_kContSortedRareGroupNo.size());
	int const iMaxCnt = lua_tinker::call<int>("CostumeMix_MaxRareItemCntOnPage");
	int iDeActivateUICnt =0;
	for(int iBldIdx=0; iBldIdx < iMaxCnt; ++iBldIdx)
	{
		BM::vstring vStr(L"FRM_RARE_ITEM");
		vStr+=iBldIdx;
		XUI::CXUI_Wnd* pkRare  = pkItemListWnd->GetControl(vStr);
		if(pkRare)
		{
			int iCurRare = (iPage*iMaxCnt)+iBldIdx;
			if(iCurRare < iMaxRareGroupCnt)
			{// 그룹 번호를 지정
				int const iGroupNo = m_kContSortedRareGroupNo.at(iCurRare);
				////
				CONT_GAMBLEMACHINEGROUPRES::const_iterator RareInfo_itor = m_kContRareItemGroupInfo.find(iGroupNo);
				if(m_kContRareItemGroupInfo.end() != RareInfo_itor)
				{// 존재 하면

					XUI::CXUI_Wnd* pkItemWnd = pkRare->GetControl(L"ICN_ITEM");
					if(pkItemWnd)
					{
						SGAMBLEMACHINEGROUPRES const& kInfo = RareInfo_itor->second;
						pkItemWnd->DefaultImgName(kInfo.kIconPath);
						SUVInfo kUVInfo(kInfo.bU, kInfo.bV, kInfo.siUVIndex);

						{// 남녀 아이콘이 다를 경우 indxex를 계산함
							kUVInfo.Index += CalcGroupResIndex(iGroupNo);
						}

						pkItemWnd->UVInfo(kUVInfo);
						pkItemWnd->ImgSize(POINT2(493,493));	//기본적으로 캐시 아이콘 이미지파일 사이즈와 같음

						// 아이콘을 클릭하건, 툴팁을 봐야 할때 그룹정보를 사용하기 위해서 넣어준다
						pkRare->SetCustomData(&kInfo.iGroupNo, sizeof(kInfo.iGroupNo));
					}
				}
				pkRare->Visible(true);
			}
			else
			{// 나머지는 아래 CSGacha_SetRareUIPos에서 보이지 않게함
				++iDeActivateUICnt;
				pkRare->Visible(false);
			}
		}
	}

	// UI 위치 정렬을 수행한다
	int const iActivateUICnt = iMaxCnt-iDeActivateUICnt;
	lua_tinker::call<void, int, int>("CostumeMix_SetRareUIPos", iActivateUICnt, iPage);
	SetCurrentRareItemPage(iPage);

	return true;
}

bool PgCashCostumeMix_ExpectResultItemUI::HideAllItemIcon()
{// 아이템 ItemPage Icon 모두 감춤
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}

	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpectResultWnd = pkCostumeMixWnd->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpectResultWnd)
	{
		return false;
	}

	XUI::CXUI_Wnd* pkItemListWnd = pkExpectResultWnd->GetControl(L"FRM_ITEM_LIST");
	if(!pkItemListWnd)
	{
		return false;
	}


	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkItemListWnd->GetControl(L"BLD_SINGLE_ITEM"));
	if(!pkBld)
	{
		return false;
	}
	int const iTotal = pkBld->CountX()*pkBld->CountY();
	for(int i=0; i< iTotal; ++i)
	{
		BM::vstring vStr(L"ICN_SINGLE_ITEM");
		vStr+=i;
		XUI::CXUI_Wnd* pkIcon  = pkItemListWnd->GetControl(vStr);
		if(pkIcon)
		{
			pkIcon->Visible(false);
			pkIcon->ClearCustomData();
			pkIcon->Text(L"");
		}
	}
	return true;
}

int PgCashCostumeMix_ExpectResultItemUI::GetMaxRareItemPage()
{
	int const iMaxCnt = lua_tinker::call<int>("CostumeMix_MaxRareItemCntOnPage");
	if(iMaxCnt == static_cast<int>(m_kContSortedRareGroupNo.size()))
	{
		return 0;
	}
	int iMaxPage= static_cast<int>(m_kContSortedRareGroupNo.size()) / iMaxCnt;
	return iMaxPage;
}

int PgCashCostumeMix_ExpectResultItemUI::CalcGroupResIndex(int const iGroupNo)
{
	CONT_RARE_ITEM::iterator Rare_itor = m_kContRareItem.find(iGroupNo);
	if(m_kContRareItem.end() == Rare_itor)
	{
		return 0;
	}
	CONT_GAMBLEITEM& kCont = Rare_itor->second;
	CONT_GAMBLEITEM::const_iterator Item_Itor = kCont.begin();
	while(kCont.end() != Item_Itor)
	{
		int const& iItemNo = Item_Itor->iItemNo;
		{// 모든 아이템 번호를 얻어와
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
			if(pkItemDef)
			{// 아이콘이 여러종류가
				if(pkItemDef->GetAbil(AT_MULTI_RES))
				{// 맞다면, 증가할 index를 계산
					int const iGenderLimitAbil = pkItemDef->GetAbil(AT_GENDERLIMIT);
					return ClientItemResChooser(iGenderLimitAbil);
				}
			}
		}
		++Item_Itor;
	}
	return 0;
}


bool PgCashCostumeMix_ExpectResultItemUI::PrevItemPage()
{
	int const iPageIdx = m_iCurrentPage-1;
	if(0 <= iPageIdx)
	{
		int const iFirstPage = BUNDLE_UNIT*GetCurrentPageBundle(); // 첫페이지의 번호를 구함 (0이면 1page)
		if(iFirstPage > iPageIdx)
		{// 현재 첫페이지 보다 작다면, 이전 페이지묶음으로
			PrevPageBundle();
		}
		else
		{
			ShowItemPage(iPageIdx);
		}
		return true;
	}
	return false;
}

bool PgCashCostumeMix_ExpectResultItemUI::NextItemPage()
{// 수정 필요
	int const iPageIdx = m_iCurrentPage+1;
	if(iPageIdx < m_iMaxPage)
	{
		int const iNextBundleFirstPage = (GetCurrentPageBundle()+1)*BUNDLE_UNIT;
		if(iNextBundleFirstPage <=iPageIdx)
		{// 다음 첫페이지 이상이라면, 다음 페이지묶음으로
			NextPageBundle();
		}
		else
		{
			ShowItemPage(iPageIdx);
		}
		return true;
	}
	return false;
}

bool PgCashCostumeMix_ExpectResultItemUI::PrevPageBundle()
{
	int const iPageBundle = m_iCurrentPageBundle-1;
	if(0 <= iPageBundle )
	{
		ShowPageBundle(iPageBundle);
		return true;
	}
	return false;
}

bool PgCashCostumeMix_ExpectResultItemUI::NextPageBundle()
{
	int const iPageBundle = m_iCurrentPageBundle+1;
	int const iMaxPageBundle = GetMaxBundlePage();
	if(iPageBundle <= iMaxPageBundle)
	{
		ShowPageBundle(iPageBundle);
		return true;
	}
	return false;
}

int PgCashCostumeMix_ExpectResultItemUI::GetMaxBundlePage() const
{
	int const iMaxBundlePage = static_cast<int>(m_kContItemPage.size())/BUNDLE_UNIT;;
	return iMaxBundlePage;
}

bool PgCashCostumeMix_ExpectResultItemUI::ShowPageBundle(int const iPageBundle)
{
	int const iMaxPageBundle = GetMaxBundlePage();
	if(0 > iPageBundle || iMaxPageBundle < iPageBundle)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(L"FRM_CASH_SHOP");
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkCostumeMixWnd = pkCashShop->GetControl(L"SFRM_COSTUME_MIX");
	if(!pkCostumeMixWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkExpectResultWnd = pkCostumeMixWnd->GetControl(L"SFRM_EXPECT_RESULT");
	if(!pkExpectResultWnd)
	{
		return false;
	}

	XUI::CXUI_Wnd* pkItemListWnd = pkExpectResultWnd->GetControl(L"FRM_ITEM_LIST");
	if(!pkItemListWnd)
	{
		return false;
	}

	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkItemListWnd->GetControl(L"BLD_PAGE"));
	if(!pkBld)
	{
		return false;
	}	
	
	int iShowPage = BUNDLE_UNIT*iPageBundle; // 첫페이지의 번호를 구하고 (0이면 1page)
	if(GetCurrentPageBundle() > iPageBundle)
	{// 뒤로 가기라면
		iShowPage += (BUNDLE_UNIT-1); //마지막 페이지를 구함
	}

	SetCurrentPageBundle(iPageBundle);
	
	// 해당 페이지 묶음의 첫 페이지 또는 마지막 페이지를 보이게 한다
	return ShowItemPage(iShowPage);
}

bool PgCashCostumeMix_ExpectResultItemUI::PrevRareItemPage()
{
	int const iRarePage = m_iCurrentRareItemPage-1;
	if(0 <= iRarePage )
	{
		ShowRareItemPage(iRarePage);
		return true;
	}
	return false;
}

bool PgCashCostumeMix_ExpectResultItemUI::NextRareItemPage()
{
	int const iRarePage = m_iCurrentRareItemPage+1;
	int const iMaxPage = GetMaxRareItemPage();
	if(iRarePage <= iMaxPage)
	{
		ShowRareItemPage(iRarePage);
		return true;
	}
	return false;
}



bool PgCashCostumeMix_ExpectResultItemUI::EquipPreViewItem(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd *pkWnd = kWnd();
	if( !pkWnd )
	{
		return false;
	}

	SGAMBLEMACHINEITEM kItemInfo;
	pkWnd->GetCustomData(&kItemInfo, sizeof(kItemInfo));
	return EquipPreViewItemUpdate(kItemInfo);
}

bool PgCashCostumeMix_ExpectResultItemUI::EquipPreViewItemUpdate(SGAMBLEMACHINEITEM const& kItemInfo)
{
		//내 성별을 찾는다.
	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return false;
	}

	int const iGender = pPlayer->GetAbil(AT_GENDER);
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItemInfo.iItemNo);
	if(!pDef)	
	{// DB에 아이템 있는지 검사
		lwAddWarnDataTT(22020);
		return false;
	}
	int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
	if( (iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
	{//다른 성별 아이템이므로 에러 출력(성별 검사)
		lwAddWarnDataTT(1992);
		return false;
	}
	if(!pDef->CanEquip())
	{// 장착 가능 검사
		lwAddWarnDataTT(790229);
		return false;
	}
	if(!IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pPlayer->GetAbil(AT_CLASS)))
	{//다른 클래스 아이템이므로 에러 출력
		lwAddWarnDataTT(1993);
		return false;
	}

	SPreviewItemInfo kInfo;
	kInfo.dwItemNo = kItemInfo.iItemNo;
	kInfo.iEquipPos = pDef->EquipPos();
	kInfo.byTimeType = kItemInfo.bTimeType;
	kInfo.siUseTime = kItemInfo.siUseTime;

	{// 스타일 아이템인가
		int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		kInfo.bStyle = (iCustomType == UICT_STYLEITEM);
	}
	if(!ExchangeSamePosEquipPreViewItem(kItemInfo))
	{// 중복 위치 이면 교체하고, 아니면 push한다
		m_kContPreViewItem.push_back(kInfo);
	}	

	// 캐시샵 캐릭터 모델에 아이템 장착, 아이콘 갱신
	UpdatePreViewItem();

	return true;
}

bool PgCashCostumeMix_ExpectResultItemUI::EquipPreViewRareGroupItem(int const iGroupNo)
{
	ClearPreViewItem();
	CONT_RARE_ITEM::iterator Rare_itor = m_kContRareItem.find(iGroupNo);
	if(m_kContRareItem.end() == Rare_itor)
	{
		return false;
	}
	CONT_GAMBLEITEM& kCont = Rare_itor->second;
	CONT_GAMBLEITEM::const_iterator Item_Itor = kCont.begin();
	while(kCont.end() != Item_Itor)
	{
		//int const& iItemNo = Item_Itor->iItemNo;
		if(!EquipPreViewItemUpdate((*Item_Itor)))
		{
			return false;
		}
		++Item_Itor;
	}
	return true;
}

bool PgCashCostumeMix_ExpectResultItemUI::CallRareGroupTooltip(int const iGroupNo)
{
	XUI::CXUI_Cursor *pkCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(!pkCursor)
	{
		return false;
	}

	CONT_GAMBLEMACHINEGROUPRES::const_iterator RareInfo_itor = m_kContRareItemGroupInfo.find(iGroupNo);
	if(m_kContRareItemGroupInfo.end() == RareInfo_itor)
	{// 레어 아이템 그룹 제목과 설명정보를 담아오기 위해 준비하고
		return false;
	}
	SGAMBLEMACHINEGROUPRES const& kRareGroupInfo = RareInfo_itor->second;	// 툴팁에 표현할 제목, 설명 정보

	CONT_RARE_ITEM::const_iterator RareItem_itor = m_kContRareItem.find(iGroupNo);
	if(m_kContRareItem.end() == RareItem_itor)
	{// 구성 아이템 번호를 얻어오기 위해 준비 한 후
		return false;
	}	
	CONT_GAMBLEITEM const& kContRareItem = RareItem_itor->second;	// 해당 그룹에 들어가있는 아이템들의 정보(필요한건 아이템번호)

	{// 툴팁 내용 작성(기본적으로 CallToolTip_Cash()를 복사해서 작성 했음)
		//typedef std::set< std::wstring > EQ_LIMIT_CONT;
		//std::wstring kSetAbilString;
		//EQ_LIMIT_CONT	kLimitCont;

		// 1. 이름 띄우고 
		BM::vstring wstrText;
		wstrText += TTW(40002);
		wstrText += kRareGroupInfo.kTitle; // 레어 아이템 그룹 이름
		wstrText += TTW(40011);	//폰트 초기화
		wstrText += _T("\n\n");
		GET_DEF(CItemDefMgr, kItemDefMgr);

		CONT_GAMBLEITEM::const_iterator Item_itor =  kContRareItem.begin();
		
		while(kContRareItem.end() != Item_itor)
		{// 2. 아이템 구성 띄우고
			SGAMBLEMACHINEITEM const& kRareItem = (*Item_itor);
			CItemDef const *pDef = kItemDefMgr.GetDef(kRareItem.iItemNo);
			if(pDef)
			{
				const wchar_t *pName = NULL;
				GetDefString(pDef->NameNo(), pName);	// 아이템 이름
				wstrText += std::wstring((pName ? pName : _T("")));
				//wstrText += L" x ";					// 수량 정보이지만 현재를 수량정보를 알 수 없음
				//wstrText += (*Item_itor).iItemCount;
				//wstrText += TTW(50413);
				wstrText += _T("\n");
				//if( pDef->CanEquip() )
				//{
				//	std::wstring kLimit;

				//	PgBase_Item kItem;
				//	kItem.ItemNo( kRareItem.iItemNo );
				//	kItem.Count( pDef->MaxAmount() );

					//if( kArticle.bSaleType != ECST_PACKEGE )
					//{//패키지는 아이템 개별 옵션을 출력하지 않는다.
					//	std::wstring kBaseAbil;
					//	MakeBaseAbilStr(kItem, pDef, kBaseAbil);
					//	if( !kBaseAbil.empty() )
					//	{
					//		wstrText += _T("\n");
					//		wstrText += TTW(40020);
					//		wstrText += TTW(40051);
					//		wstrText += kBaseAbil;
					//	}
					//}

					//if( 0 != (pDef->GetAbil(AT_GENDERLIMIT) & GWL_PET_UNISEX) )
					//{
					//	MakePetClassLimitText(pDef, kLimit);
					//}
					//else
					//{
					//	_MakeClassLimitText(pDef, kLimit);//착용 제한 직업
					//}

					//if( kSetAbilString.empty() )
					//{//패키지 아이템은 무조건 1가지 세트로 구성한다. 출력은 나중에
					//	GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
					//	int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
					//	if( iSetNo )
					//	{//세트 번호 있다
					//		MakeSetItemAbilString(kItem, pDef, false, kSetAbilString);
					//	}
					//}
					//kLimitCont.insert(kLimit);
				//}
			}
			++Item_itor;
		}
		// 3. 설명 띄우고 
		wstrText += _T("\n");
		wstrText += TTW(1892);
		wstrText += TTW(40051);
		wstrText += _T("\n");
		wstrText += kRareGroupInfo.kDescription;
		wstrText += _T("\n");
		//if( !kLimitCont.empty() )
		//{// 착용 가능 직업
		//	wstrText += _T("\n");
		//	wstrText += TTW(1896);
		//	wstrText += TTW(40051);
		//	wstrText += _T("\n");
		//	EQ_LIMIT_CONT::iterator iter = kLimitCont.begin();
		//	if( iter != kLimitCont.end() )
		//	{
		//		wstrText += (*iter);
		//		wstrText += _T("\n");
		//	}
		//}
		//if( !kSetAbilString.empty() )
		//{
		//	wstrText += kSetAbilString;
		//}

		XUI::CXUI_Wnd *pkToolTip = XUIMgr.Call(L"TOOLTIP2", false);
		if(pkToolTip)
		{
			int const TT_OL_SIZE = 25;	//얘는 글과 경계 사이

			pkToolTip->Size(600, pkToolTip->Size().y);//사이즈는 화면 절반만큼 미리 잡아주고
			BM::vstring wstrName;
			wstrName += TTW(40002);
			wstrName += kRareGroupInfo.kTitle;// 레어아이템 그룹 이름

			pkToolTip->Text(wstrName);
			//스타일 텍스트 뽑고
			XUI::CXUI_Style_String	kStyleName = pkToolTip->StyleText();
			//길이 계산 하고
			POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));

			//길이 비교해서 아이템 이름이 더 길면 늘리고 짧으면 보통 크기로 냅두고

			if(iTOOLTIP_X_SIZE_MAX < kTextSize.x + TT_OL_SIZE)
			{
				pkToolTip->Size(TT_OL_SIZE + kTextSize.x, pkToolTip->Size().y);
				pkToolTip->Text(wstrText); 
			}
			else
			{
				pkToolTip->Size(iTOOLTIP_X_SIZE_MAX, pkToolTip->Size().y);
				pkToolTip->Text(wstrText);
			}
	
			pkToolTip->Location(pkCursor->GetTotalLocation());		//툴팁 위치 잡아주고
			pkToolTip->Invalidate(true);	//그려주고
		}
	}
	return true;
}

void PgCashCostumeMix_ExpectResultItemUI::ClearPreViewItem()
{
	RECV_UI_EQUIP_PREVIEW_CONT::iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{
		g_kCashShopMgr.DelPreviewEquip((*itor).dwItemNo);
		++itor;
	}
	m_kContPreViewItem.clear();
}

bool PgCashCostumeMix_ExpectResultItemUI::ExchangeSamePosEquipPreViewItem(SGAMBLEMACHINEITEM const& kItemInfo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkDefTagetItem = kItemDefMgr.GetDef(kItemInfo.iItemNo);
	if(!pkDefTagetItem)
	{
		return false;
	}

	RECV_UI_EQUIP_PREVIEW_CONT::iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{
		CItemDef const *pkDefItem = kItemDefMgr.GetDef((*itor).dwItemNo);
		if(pkDefItem)
		{
			if(pkDefTagetItem->EquipPos() == pkDefItem->EquipPos())
			{
				(*itor).dwItemNo = kItemInfo.iItemNo;
				(*itor).iEquipPos = pkDefTagetItem->EquipPos();
				int const iCustomType = pkDefTagetItem->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
				(*itor).bStyle  = (iCustomType == UICT_STYLEITEM);
				(*itor).byTimeType = kItemInfo.bTimeType;
				(*itor).siUseTime = kItemInfo.siUseTime;
				return true;
			}
		}
		++itor;
	}
	return false;
}

bool PgCashCostumeMix_ExpectResultItemUI::DelPreViewItem(int const iItemNo)
{
	RECV_UI_EQUIP_PREVIEW_CONT::iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{		
		if(iItemNo == (*itor).dwItemNo)
		{
			g_kCashShopMgr.DelPreviewEquip((*itor).dwItemNo);
			itor = m_kContPreViewItem.erase(itor);
			return true;
		}
		++itor;
	}
	return false;
}

void PgCashCostumeMix_ExpectResultItemUI::UpdatePreViewItem()
{
	RECV_UI_EQUIP_PREVIEW_CONT::const_iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{		
		g_kCashShopMgr.SetPreviewEquipItem((*itor).dwItemNo);			// 아이템 UI 모델에 장착
		++itor;
	}
	lwCashShop::SetPreviewEquipIcon(m_kContPreViewItem);	// 아이콘 장착창에 아이콘 넣기
}

//bool PgCSGacha::CallEquipItemToolTip(lwUIWnd kWnd)
//{
//	XUI::CXUI_Wnd *pkWnd = kWnd();
//	if( !pkWnd )
//	{
//		return false;
//	}
//	
//	SPreviewItemInfo kItemInfo;
//	pkWnd->GetCustomData(&kItemInfo, sizeof(kItemInfo));
//
//	CallToolTip_ItemNo(kItemInfo.dwItemNo, kWnd.GetLocation(), false, false, false, kItemInfo.byTimeType, kItemInfo.siUseTime);
//	return true;
//}

//bool PgCSGacha::CallSingleItemToolTip(lwUIWnd kWnd)
//{
//	XUI::CXUI_Wnd *pkWnd = kWnd();
//	if( !pkWnd )
//	{
//		return false;
//	}
//
//	SGAMBLEMACHINEITEM kItemInfo;
//	pkWnd->GetCustomData(&kItemInfo, sizeof(kItemInfo));
//	
//	CallToolTip_ItemNo(kItemInfo.iItemNo, kWnd.GetLocation(), false, false, false, kItemInfo.bTimeType, kItemInfo.siUseTime);
//	return true;
//}
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace lwUICostumeMix
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CostumeMixShowPart", lwUICostumeMix::CostumeMixShowPart);

		def(pkState, "CostumeMixPrevItemPage", lwUICostumeMix::CostumeMixPrevItemPage);
		def(pkState, "CostumeMixNextItemPage", lwUICostumeMix::CostumeMixNextItemPage);
		def(pkState, "CostumeMixShowItemPage", lwUICostumeMix::CostumeMixShowItemPage);

		//def(pkState, "SetCSGachaPreViewEquipItem", lwUICostumeMix::SetCSGachaPreViewEquipItem);
		//def(pkState, "ClearCSGachaPreViewEquipItem", lwUICostumeMix::ClearCSGachaPreViewEquipItem);

		//def(pkState, "EquipCSGachaPreViewRareGroupItem", lwUICostumeMix::EquipCSGachaPreViewRareGroupItem);
		//def(pkState, "CallCSGachaRareGroupTooltip", lwUICostumeMix::CallCSGachaRareGroupTooltip);

		def(pkState, "CostumeMixShowRareItemPage", lwUICostumeMix::CostumeMixShowRareItemPage);
		def(pkState, "CostumeMixPrevRareItemPage", lwUICostumeMix::CostumeMixPrevRareItemPage);
		def(pkState, "CostumeMixNextRareItemPage", lwUICostumeMix::CostumeMixNextRareItemPage);

		def(pkState, "CostumeMixInsertSlotForMix", lwUICostumeMix::CostumeMixInsertSlotForMix);
		def(pkState, "CostumeMixRemoveSlot", lwUICostumeMix::CostumeMixRemoveSlot);
		
		def(pkState, "CostumeMixGetCurMixPoint", lwUICostumeMix::CostumeMixGetCurMixPoint);
		def(pkState, "CostumeMixSetCurMixPoint", lwUICostumeMix::CostumeMixSetCurMixPoint);

		def(pkState, "CostumeMixGetMinMixPoint", lwUICostumeMix::CostumeMixGetMinMixPoint);
		def(pkState, "CostumeMixGetMaxMixPoint", lwUICostumeMix::CostumeMixGetMaxMixPoint);

		def(pkState, "CostumeMixGetCurrentPointState", lwUICostumeMix::CostumeMixGetCurrentPointState);
		def(pkState, "CostumeMixClearAllSlotUI", lwUICostumeMix::CostumeMixClearAllSlotUI);
		
		def(pkState, "CostumeMixGetRemainReloadCnt", lwUICostumeMix::CostumeMixGetRemainReloadCnt);
		def(pkState, "CostumeMixDecRemainReloadCnt", lwUICostumeMix::CostumeMixDecRemainReloadCnt);
		def(pkState, "CostumeMixReqResultItemList", lwUICostumeMix::CostumeMixReqResultItemList);
		def(pkState, "CostumeMixReqMixUpReady", lwUICostumeMix::CostumeMixReqMixUpReady);
		def(pkState, "CostumeMixReqRoulletReload", lwUICostumeMix::CostumeMixReqRoulletReload);
		def(pkState, "CostumeMixReqGetResultItem", lwUICostumeMix::CostumeMixReqGetResultItem);
		def(pkState, "CostumeMixShowSubUIPreviewRoulletTbl", lwUICostumeMix::CostumeMixShowSubUIPreviewRoulletTbl);

		def(pkState, "CostumeMixSetPreViewEquipItem", lwUICostumeMix::CostumeMixSetPreViewEquipItem);
		def(pkState, "CostumeMixClearPreViewEquipItem", lwUICostumeMix::CostumeMixClearPreViewEquipItem);
		def(pkState, "CostumeMixEquipPreViewRareGroupItem", lwUICostumeMix::CostumeMixEquipPreViewRareGroupItem);
		def(pkState, "CostumeMixCallRareGroupTooltip", lwUICostumeMix::CostumeMixCallRareGroupTooltip);
		def(pkState, "CostumeMix_IsInSlot", lwUICostumeMix::CostumeMix_IsInSlot);
		def(pkState, "CostumeMix_CallSlotItemToolTip", lwUICostumeMix::CostumeMix_CallSlotItemToolTip);
		def(pkState, "CostumeMix_ClearSlotInfo", lwUICostumeMix::CostumeMix_ClearSlotInfo);
		def(pkState, "CostumeMix_GetMaxRareItemPage", lwUICostumeMix::CostumeMix_GetMaxRareItemPage);
	}

	bool CostumeMixShowPart(int iType)
	{
		return g_kCostumeMix.GetPreviewResultItemUIMgr().ShowPart(static_cast<eGambleCoinGrade>(iType));
	}
	bool CostumeMixPrevItemPage()
	{// 아이템 리스트의 이전 페이지 보기 버튼
		return g_kCostumeMix.GetPreviewResultItemUIMgr().PrevItemPage();
	}
	bool CostumeMixNextItemPage()
	{// 아이템 리스트의 다음 페이지 보기 버튼
		return g_kCostumeMix.GetPreviewResultItemUIMgr().NextItemPage();
	}	
	bool CostumeMixShowItemPage(int const iPageIdx)
	{// 해당 페이지 아이템 리스트 보기
		return g_kCostumeMix.GetPreviewResultItemUIMgr().ShowItemPage(iPageIdx);
	}
	
	bool  CostumeMixShowRareItemPage(int const iPage)
	{// 레어 그룹 페이지를 보여줌
		return g_kCostumeMix.GetPreviewResultItemUIMgr().ShowRareItemPage(iPage);
	}
	bool  CostumeMixPrevRareItemPage()
	{// 이전 레어 그룹 페이지를 보여줌
		return g_kCostumeMix.GetPreviewResultItemUIMgr().PrevRareItemPage();
	}
	bool  CostumeMixNextRareItemPage()
	{// 다음 레어 그룹 페이지를 보여줌
		return g_kCostumeMix.GetPreviewResultItemUIMgr().NextRareItemPage();
	}
	bool CostumeMixInsertSlotForMix(lwUIWnd kCashInvSlot)
	{
		return g_kCostumeMix.InsertSlotForMix(kCashInvSlot.GetSelf());
	}
	bool CostumeMixRemoveSlot(lwUIWnd kRegSlot)
	{
		return g_kCostumeMix.RemoveSlot(kRegSlot.GetSelf());
	}
	int CostumeMixGetCurMixPoint()
	{
		return g_kCostumeMix.GetCurMixPoint();
	}
	bool CostumeMixSetCurMixPoint(int const iNewPoint)
	{
		return g_kCostumeMix.SetCurMixPoint(iNewPoint);
	}
	int CostumeMixGetMinMixPoint()
	{
		return g_kCostumeMix.GetMinMixPoint();
	}
	int CostumeMixGetMaxMixPoint()
	{
		return g_kCostumeMix.GetMaxMixPoint();
	}
	lwWString CostumeMixGetCurrentPointState()
	{
		return g_kCostumeMix.GetCurrentPointState(); 
	}
	bool CostumeMixClearAllSlotUI()
	{
		return g_kCostumeMix.ClearAllSlotUI();
	}

	int CostumeMixGetRemainReloadCnt()
	{
		return g_kCostumeMix.GetRemainReloadCnt();
	}
	bool CostumeMixDecRemainReloadCnt()
	{
		return g_kCostumeMix.DecRemainReloadCnt();
	}
	void CostumeMixReqResultItemList()
	{
		g_kCostumeMix.ReqResultItemList();
	}
	void CostumeMixReqMixUpReady()
	{// 조합 시작 버튼 누를때 요청
		g_kCostumeMix.ReqMixUpReady();
	}
	void CostumeMixReqRoulletReload()
	{// 룰렛 리로드 요청
		g_kCostumeMix.ReqRoulletReload();
	}
	void CostumeMixReqGetResultItem()
	{// 룰렛이 끝나서 받을 아이템을 요청함
		g_kCostumeMix.ReqGetResultItem();
	}
	bool CostumeMixShowSubUIPreviewRoulletTbl(int const iPoint)
	{
		return g_kCostumeMix.ShowSubUIPreviewRoulletTbl(iPoint);
	}
	
	void CostumeMixSetPreViewEquipItem(lwUIWnd kWnd)
	{// 아이템 번호에 해당하는 아이템을 미리 보기에 장착 시키기(캐시샵 가차용)
		g_kCostumeMix.GetPreviewResultItemUIMgr().EquipPreViewItem(kWnd);
	}
	void CostumeMixClearPreViewEquipItem()
	{// 미리보기에 아이템을 모두 벗기고, 미리 보기 아이콘도 제거
		g_kCostumeMix.GetPreviewResultItemUIMgr().ClearPreViewItem();
	}
	bool CostumeMixEquipPreViewRareGroupItem(int const iGroupNo)
	{// 레어 그룹번호에 해당하는 아이템을 미리보기
		return g_kCostumeMix.GetPreviewResultItemUIMgr().EquipPreViewRareGroupItem(iGroupNo);
	}
	bool CostumeMixCallRareGroupTooltip(int const iGroupNo)
	{// 레어 그룹번호에 해당하는 툴팁을 출력
		return g_kCostumeMix.GetPreviewResultItemUIMgr().CallRareGroupTooltip(iGroupNo);
	}
	bool CostumeMix_IsInSlot(lwUIWnd kCashInvSlot)
	{
		return g_kCostumeMix.IsInSlot(kCashInvSlot.GetSelf());
	}
	bool CostumeMix_CallSlotItemToolTip(int const iSlotIdx)
	{
		return g_kCostumeMix.CallSlotItemToolTip(iSlotIdx);
	}
	void CostumeMix_ClearSlotInfo()
	{
		g_kCostumeMix.ClearSlotInfo();
	}
	int CostumeMix_GetMaxRareItemPage()
	{// 최대 레어 그룹페이지 값을 얻어옴
		return g_kCostumeMix.GetPreviewResultItemUIMgr().GetMaxRareItemPage();
	}
}

std::wstring GetCostumeMixGradeString(int const iCostumeMixGrade)
{
	return TTW(790750+iCostumeMixGrade);
	//switch(iCostumeMixGrade)
	//{
	//case E_NORMAL_GRADE:	{ return TTW(790751); } break;
	//case E_ADV_GRADE:		{ return TTW(790752); } break;
	//case E_SPCL_GRADE:		{ return TTW(790753); } break;
	//case E_LEGEND_GRADE:	{ return TTW(790754); } break;
	//default:				{ return L""; }break;
	//}
}