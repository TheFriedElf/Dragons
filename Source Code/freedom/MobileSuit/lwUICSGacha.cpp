#include "StdAfx.h"
#include "Variant/Global.h"
#include "variant/ItemDefMgr.h"
#include "lohengrin/dbtables.h"
#include "lohengrin/ErrorCode.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "lwUI.h"
#include "PgUISound.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "lwUICSGacha.h"
#include "lwUIQuest.h"
#include "lwCashShop.h"
#include "lwUIIGGacha.h"

extern int CALLBACK ClientItemResChooser(int const iGenderLimit);

int const CASH_GACHA_COIN_ITEMNO= 98005580;
//{
//	98005580 캐시 가챠 코인
// 어빌에  5023 : 5  추가하였습니다.  (개발/리뉴얼) 근데 5는 펫 먹이임
//}
int const PgCSGacha::MAX_ITEM_CNT_ON_PAGE = 50;
int const PgCSGacha::BUNDLE_UNIT = 4;
POINT2 const RARE_ITEM_GROUP_IMG_SIZE(490,490);

std::wstring const FRM_CASH_SHOP(L"FRM_CASH_SHOP");
std::wstring const SFRM_CASH_GACHA(L"SFRM_CASH_GACHA");
std::wstring const BLD_SINGLE_ITEM(L"BLD_SINGLE_ITEM");
std::wstring const ICN_SINGLE_ITEM(L"ICN_SINGLE_ITEM");
std::wstring const FRM_COIN_CNT(L"FRM_COIN_CNT");
std::wstring const FRM_ROULETTE(L"FRM_ROULETTE");
std::wstring const ICN_ROULETTE_ITEM(L"ICN_ROULETTE_ITEM");
std::wstring const SFRM_RELOAD_CNT(L"SFRM_RELOAD_CNT");
std::wstring const BLD_PAGE(L"BLD_PAGE");
std::wstring const CBTN_PAGE(L"CBTN_PAGE");
std::wstring const FRM_RARE_ITEM(L"FRM_RARE_ITEM");
std::wstring const SFRM_CASHSHOP_BUY(L"SFRM_CASHSHOP_BUY");
std::wstring const ICN_ITEM(L"ICN_ITEM");

PgCSGacha::PgCSGacha()
{
	Init();
}
PgCSGacha::~PgCSGacha()
{
}

void PgCSGacha::Init()
{
	m_iCurrentPage = 0;
	m_iRemainReloadTime = 0;
	m_iResultItemIdx = 0;
	m_iMaxPage = 0;
	m_iCurrentPageBundle = 0;

	m_kContItemBag.clear();
	m_kContItemPage.clear();

	m_kContRouletteItem.clear();
	m_kContResultItem.clear();

	m_kContRareItem.clear();
	m_kContRareItemGroupInfo.clear();
	m_kContSortedRareGroupNo.clear();

	m_iCurrentRareItemPage = 0;

	// 가챠에서 사용한 미리 보기 삭제
	ClearPreViewItem();
}

void PgCSGacha::SetItemOnListUI()
{
	CONT_GAMBLEMACHINE::iterator Gm_itor = m_kContItemBag.find(GCG_CASHSHOP);
	if(Gm_itor == m_kContItemBag.end())
	{
		return;
	}
	m_kContItemPage.clear();
	m_kContRareItem.clear();

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

			if(PgCSGacha::MAX_ITEM_CNT_ON_PAGE <= iCnt
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
		XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(FRM_CASH_SHOP);
		if(!pkCashShop)
		{
			return;
		}
			
		XUI::CXUI_Wnd* pkGacha = pkCashShop->GetControl(SFRM_CASH_GACHA);
		if(!pkGacha)
		{
			return;
		}
		
		{//정렬
			m_kContSortedRareGroupNo.clear();
			m_kContSortedRareGroupNo.reserve(m_kContRareItemGroupInfo.size());
			CONT_GAMBLEMACHINEGROUPRES::const_iterator RareInfo_itor = m_kContRareItemGroupInfo.begin();
			while(m_kContRareItemGroupInfo.end() != RareInfo_itor)
			{
				CONT_RARE_ITEM::const_iterator  Rare_itor = m_kContRareItem.find(RareInfo_itor->first); 
				if(m_kContRareItem.end() != Rare_itor)
				{
					m_kContSortedRareGroupNo.push_back((*RareInfo_itor).second.iDisplayRank);
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

void PgCSGacha::SetItemOnRouletteUI()
{
	int const MAX_IDX = 10;
	int const DUMMY_ITEM_CNT = MAX_IDX-1;

	XUI::CXUI_Wnd* pkRoulette = XUIMgr.Get(FRM_ROULETTE);
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
		BM::vstring vStr(ICN_ROULETTE_ITEM);
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
						break;
					}
				}
				++kDummyItem_itor;
			}
		}
	}

	int const iResultItemIdx = m_iResultItemIdx;
	
	CONT_INT kContNumber;
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
				BM::vstring vStr(ICN_ROULETTE_ITEM);
				int const iIdx = BM::Rand_Index(kContNumber.size());
				int const iRandIdx = kContNumber.at(iIdx);
				vStr+=iRandIdx;
				XUI::CXUI_Wnd* pkIcn= pkRoulette->GetControl(vStr);
				if(pkIcn)
				{// 나온값 UI에 하나씩 세팅하고, 
					pkIcn->SetCustomData(&rkItemInfo, sizeof(rkItemInfo));
					pkIcn->Text(BM::vstring(rkItemInfo.siCount));
					lwDrawIconToItemNo(pkIcn, rkItemInfo.iItemNo, 1.0f, lwPoint2F(0.5f, 0.5f));
				}

				{//kContNumber에서 하나씩 제거함
					CONT_INT::iterator kNum_itor = kContNumber.begin();
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

bool PgCSGacha::ShowItemPage(int const iPageIdx)
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

	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(FRM_CASH_SHOP);
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkGacha = pkCashShop->GetControl(SFRM_CASH_GACHA);
	if(!pkGacha)
	{
		return false;
	}

	{
		XUI::CXUI_Builder* pkBldPage  = static_cast<XUI::CXUI_Builder*>(pkGacha->GetControl(BLD_PAGE));
		if(pkBldPage)
		{
			int const iTotal = pkBldPage->CountX()*pkBldPage->CountY();
			for(int i=0; i < iTotal; ++i)
			{
				BM::vstring vStr(CBTN_PAGE);
				vStr+=i;
				XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkGacha->GetControl(vStr));
				if(pkChkBtn)
				{// page에 표시할 아이템이
					int const iCorrectlyPageIdx = i+GetCurrentPageBundle()*BUNDLE_UNIT;
					if(iCorrectlyPageIdx < m_iMaxPage
						&& i < m_kContItemPage.size()
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

	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkGacha->GetControl(BLD_SINGLE_ITEM));
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
			BM::vstring vStr(ICN_SINGLE_ITEM);
			vStr+=iCnt;
			XUI::CXUI_Wnd* pkIcon  = pkGacha->GetControl(vStr);
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

bool PgCSGacha::HideAllItemIcon()
{// 아이템 ItemPage Icon 모두 감춤
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(FRM_CASH_SHOP);
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkGacha = pkCashShop->GetControl(SFRM_CASH_GACHA);
	if(!pkGacha)
	{
		return false;
	}

	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkGacha->GetControl(BLD_SINGLE_ITEM));
	if(!pkBld)
	{
		return false;
	}
	int const iTotal = pkBld->CountX()*pkBld->CountY();
	for(int i=0; i< iTotal; ++i)
	{
		BM::vstring vStr(ICN_SINGLE_ITEM);
		vStr+=i;
		XUI::CXUI_Wnd* pkIcon  = pkGacha->GetControl(vStr);
		if(pkIcon)
		{
			pkIcon->Visible(false);
			pkIcon->ClearCustomData();
			pkIcon->Text(L"");
		}
	}
	return true;
}

void PgCSGacha::ReqCSGachaItemList()
{//겜블 머신 물품 리스트 요청
	BM::Stream kPacket(PT_C_M_REQ_GAMBLEMACHINEINFO);
	kPacket.Push(true);		// 캐시샵 가챠인가?
	NETWORK_SEND(kPacket);
}

bool PgCSGacha::ReqStartRoulette()
{// 가챠 룰렛 사용 요청
	if( m_kContItemPage.empty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1261, true);
		return false;
	}

	if(!CheckEnoughCoin())
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 550005, true);
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

	SItemPos kItemPos;
	if(E_FAIL == pkInv->GetFirstItem(CASH_GACHA_COIN_ITEMNO, kItemPos))
	{// 코인 없음 메세지를 보여줌
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 550005, true);
		return false;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_GAMBLEMACHINE_READY);
	kPacket.Push(kItemPos);

	if(LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion())
	{// 한국은 룰렛이 뜨면 안된다
		lwCallCommonMsgYesNoBox(MB(TTW(790788)), lwPacket(&kPacket), true, MBT_NONE_CASH_GACHA_ROULLET);
	}
	else
	{
		NETWORK_SEND(kPacket);
	}
	return true;
}

//대기창(on/offl) - lwui에것을 사용
bool PgCSGacha::RecvCSGachaPacket(WORD const dwPacketType, BM::Stream& rkPacket)
{
	switch(dwPacketType)
	{
	case PT_M_C_ANS_GAMBLEMACHINEINFO:
		{// 아이템 리스트가 옴
			PgPlayer const* const pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return false;
			}

			Init();
			g_kGambleMachine.ReadFromPacket(rkPacket);
			m_kContItemBag.clear();
			g_kGambleMachine.GetCont(m_kContItemBag, pkPlayer->UnitRace());				// 아이템 목록을 얻고
			g_kGambleMachine.GetContRes(m_kContRareItemGroupInfo);	// 참고할 레어아이템 그룹 정보를 얻는다

			SetItemOnListUI();	// 단품 아이템을 UI 세팅함
			return true;
		}break;
	//case PT_M_C_NFY_GAMBLEMACHINE_RESULT:	// lwUICSGacha에서 처리(인게임가챠)
	//		{//겜블 결과 통보
	//		}break;
	case PT_M_C_ANS_USE_GAMBLEMACHINE_READY:
		{//코인 사용 결과s
			HRESULT kResult = S_OK;
			if(S_OK == kResult)
			{
				rkPacket.Pop(kResult);			// HRESULT : ErrorCode.h 겜블 머신 에러 코드 참고
				rkPacket.Pop(m_iRemainReloadTime);	// int : 총 사용 가능한 룰랫 리로드 횟수
				
				if(LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion())
				{// 한국은 룰렛이 뜨면 안된다
					ReqGetResultItem();
					lua_tinker::call<void, bool>("Roulette_StartBtnDisable", false);
				}
			}
			else
			{// 에러 메세지
				ShowErrorMsg(kResult);
			}
		}break;
	case PT_M_C_ANS_RELOAD_ROULETTE:
		{
			HRESULT kResult = S_OK;
			m_kContResultItem.clear();
			m_kContRouletteItem.clear();
			
			rkPacket.Pop(kResult);	//HRESULT : ErrorCode.h
			if(S_OK == kResult)
			{
				PU::TLoadArray_M(rkPacket, m_kContResultItem);	// 결과 아이템
				PU::TLoadArray_A(rkPacket, m_kContRouletteItem); // 룰렛판에 올라갈 아이템 리스트
				rkPacket.Pop(m_iRemainReloadTime);				// Int : 남은 룰랫 리로드 횟수
				if(LOCAL_MGR::NC_KOREA != g_kLocal.ServiceRegion())
				{// 한국은 룰렛이 뜨면 안된다
					lua_tinker::call<bool>("CallRoulette");			// 최초 호출(남은 룰렛횟수로 판단)시 UI를 호출
					SetItemOnRouletteUI();							// 룰렛 아이템에서 아이템 세팅,화살표가 가르킬 결과 아이템도 세팅
					UpdateReloadTextUI();							// 룰렛 재설정 횟수를 UI에 세팅				
				}
			}
			else
			{// 에러 메세지
				ShowErrorMsg(kResult);
			}
		}break;
	case PT_M_C_ANS_ROULETTE_RESULT:
		{//겜블 결과 통보
			HRESULT kResult = S_OK;
			m_kContResultItem.clear();
			rkPacket.Pop(kResult);	//HRESULT : ErrorCode.h
			if(S_OK == kResult)
			{
				PU::TLoadArray_M(rkPacket, m_kContResultItem);	//CONT_GAMBLEMACHINERESULT : 결과 통보 (PgBase_Item 리스트 PU::TLoadArray_M 으로 읽음 )
				CONT_GAMBLEMACHINERESULT::const_iterator itor = m_kContResultItem.begin();
				while(m_kContResultItem.end() != itor)
				{
					NotifyEarnItem(*itor);
					++itor;
				}
			}
			else
			{// 에러 메세지
				ShowErrorMsg(kResult);
			}
		}break;
	case PT_M_C_NOTI_ROULETTE_RESULT_MAIL:
		{// 룰렛아이템이 메일로 발송 되었음
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790631, true);
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

//ㄴ
void PgCSGacha::UpdateOwnCoinCnt(lwUIWnd kWnd)			//소지한 코인 수량을 UI에 출력하고
{
	XUI::CXUI_Wnd *pkWnd = kWnd();
	if( !pkWnd )
	{
		return;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return;
	}

	// UICT타입으로 갯수를 얻어올것(AT_USE_ITEM_CUSTOM_TYPE)
	int const iCoinCnt = pkInv->GetTotalCount(CASH_GACHA_COIN_ITEMNO);

	BM::vstring vStr(iCoinCnt);
	pkWnd->Text(vStr);
}

//ㄷ
void PgCSGacha::CallBuyCoinUI()				// 코인 구입 UI 호출할수 있고(modal타입)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(CASH_GACHA_COIN_ITEMNO);
	if(!pDef)
	{//아이템이 디비에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 22020, true);
		return;
	}

	int iIndex = 0;
	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if(!g_kCashShopMgr.FindArticle(CASH_GACHA_COIN_ITEMNO, iIndex, kArticle))
	{//캐시샵 판매물품에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
		return;
	}

	if (kArticle.kContCashItemPrice.empty())
	{
		return;
	}

	__int64 i64MyCash = 0;
	__int64 i64MyMileage = 0;
	{// 플레이어의 소지금. 마일리지 얻기
		PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
		if(NULL == pPilot)
		{
			return;
		}
		i64MyCash = pPilot->GetAbil64(AT_CASH);
		i64MyMileage = pPilot->GetAbil64(AT_BONUS_CASH);
	}

	TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
	CONT_CASH_SHOP_ITEM_PRICE::iterator	Price_iter = kArticle.kContCashItemPrice.begin();
	while( Price_iter != kArticle.kContCashItemPrice.end() )
	{
		kPrice = Price_iter->second;
		if( 0 != kPrice.iExtendCash || ( kPrice.IsExtendDiscount && 0 != kPrice.iDiscountExtendCash ) )
		{
			if( 0 != kPrice.iItemUseTime )
			{
				break;
			}
		}
		//else
		//{
		//	Price_iter = kArticle.kContCashItemPrice.erase(Price_iter);
		//	continue;
		//}
		++Price_iter;
	}

	if( 0>=kPrice.iItemUseTime && 0>=kPrice.bTimeType )
	{// 비정상
		return;
	}

	// 소지금 체크
	int const iBuyCash = (kPrice.IsDiscount ? kPrice.iDiscountCash : kPrice.iCash);
	if(kPrice.IsOnlyMileage)
	{// 마일리지용 아이템이면
		if(i64MyMileage < iBuyCash)
		{// 마일리지로 비교
			lwAddWarnDataTT(790216);
			return;
		}
	}
	else
	{// 캐시용 아이템이면
		if(i64MyCash < iBuyCash)
		{// 캐시로 비교
			lwAddWarnDataTT(1900);
			return;
		}
	}
	XUI::CXUI_Wnd* pkBuyWnd = XUIMgr.Call(SFRM_CASHSHOP_BUY, true);
	if (!pkBuyWnd)	
	{
		return;
	}
	lwCashShop::SetDayBtnInitialize(pkBuyWnd, kArticle, iBuyCash, lwCashShop::CIBT_BUY);
	XUI::CXUI_Wnd* kTempWnd = pkBuyWnd->GetControl(L"BTN_BUY");
	if(!kTempWnd)
	{
		return;
	}
	kTempWnd->SetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));
	pkBuyWnd->SetCustomData(&iIndex, sizeof(int));
	{// 아이템 가격을 기억
		g_kCashShopMgr.SetRememberTotalPrice(iBuyCash);
	}
	{// 유저에게 지급될 마일리지 값을 기억
		__int64 i64CalculatedMileage = g_kCashShopMgr.CalcAddBonus(iBuyCash, kArticle.iMileage);
		i64CalculatedMileage += g_kCashShopMgr.CalcLocalAddBonus(iBuyCash);
		g_kCashShopMgr.SetRememberItemMileage(i64CalculatedMileage);
	}
	
	Quest::SetCutedTextLimitLength(pkBuyWnd->GetControl(L"FRM_NAME"), kArticle.wstrName, _T("..."));
	BM::vstring kstrPrice;
	int iItemCount = 0;
	CONT_CASH_SHOP_ITEM::const_iterator item_iter = kArticle.kContCashItem.begin();
	if( item_iter != kArticle.kContCashItem.end() )
	{
		iItemCount = item_iter->iItemCount;
	}
	lwCashShop::MakeUseTimeStr(kArticle.bSaleType, kPrice, kstrPrice, iItemCount, lwCashShop::CIBT_BUY);

	if(lwCashShop::IsBuyingCashItem())
	{
		lwCashShop::OffAllMileageWnd(false, pkBuyWnd);
	}
	else
	{
		lwCashShop::OffAllMileageWnd(true, pkBuyWnd);
	}
	
	kTempWnd = pkBuyWnd->GetControl(L"FRM_COST");
	if(!kTempWnd)
	{
		return;
	}
	kTempWnd->Text(BM::vstring(iBuyCash));

	kTempWnd = pkBuyWnd->GetControl(L"FRM_CASH");
	if(!kTempWnd)
	{
		return;
	}
	kTempWnd->Text(BM::vstring(i64MyCash));

	kTempWnd = pkBuyWnd->GetControl(L"FRM_TOTAL");
	if(!kTempWnd)
	{
		return;
	}
	kTempWnd->Text(BM::vstring((i64MyCash - iBuyCash)));

	lwCashShop::DrawCashItem(pkBuyWnd, kArticle);
	lwCashShop::CashNoEquip(pkBuyWnd,iIndex,kArticle);
	lwCashShop::CashMiniIcon(pkBuyWnd, kArticle);
}

bool PgCSGacha::CheckEnoughCoin()
{// 충분한 코인이 있는지 확인
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

	// UICT타입으로 갯수를 얻어올것(AT_USE_ITEM_CUSTOM_TYPE)
	int const iCoinCnt = pkInv->GetTotalCount(CASH_GACHA_COIN_ITEMNO);
	if(0 < iCoinCnt)
	{
		return true;
	}
	return false;
}

bool PgCSGacha::ReqReloadRoulette()
{// 룰렛 리로드 요청
	BM::Stream kPacket(PT_C_M_REQ_RELOAD_ROULETTE);
	NETWORK_SEND(kPacket);
	return true;
}
bool PgCSGacha::ReqGetResultItem()
{// 룰렛이 끝나서 받을 아이템을 요청함
	BM::Stream kPacket(PT_C_M_REQ_ROULETTE_RESULT);
	NETWORK_SEND(kPacket);
	return true;
}

bool PgCSGacha::DecRemainReloadCnt()
{
	if(0 >= m_iRemainReloadTime)
	{
		return false;
	}
	--m_iRemainReloadTime;
	UpdateReloadTextUI();
	return true;
}

void PgCSGacha::UpdateReloadTextUI()
{
	XUI::CXUI_Wnd* pkRoulette = XUIMgr.Get(FRM_ROULETTE);
	if(!pkRoulette)
	{
		return;
	}

	XUI::CXUI_Wnd* pkReloadText= pkRoulette->GetControl(SFRM_RELOAD_CNT);
	if(!pkReloadText)
	{
		return;
	}
	BM::vstring vStr(GetRemainReloadCnt());
	pkReloadText->Text(vStr);
}

bool PgCSGacha::PrevPage()
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

bool PgCSGacha::NextPage()
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

bool PgCSGacha::PrevPageBundle()
{
	int const iPageBundle = m_iCurrentPageBundle-1;
	if(0 <= iPageBundle )
	{
		ShowPageBundle(iPageBundle);
		return true;
	}
	return false;
}

bool PgCSGacha::NextPageBundle()
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

bool PgCSGacha::ShowPageBundle(int const iPageBundle)
{
	int const iMaxPageBundle = GetMaxBundlePage();
	if(0 > iPageBundle || iMaxPageBundle < iPageBundle)
	{
		return false;
	}
		
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(FRM_CASH_SHOP);
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkGacha = pkCashShop->GetControl(SFRM_CASH_GACHA);
	if(!pkGacha)
	{
		return false;
	}

	XUI::CXUI_Builder* pkBld  = static_cast<XUI::CXUI_Builder*>(pkGacha->GetControl(BLD_PAGE));
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

int PgCSGacha::GetMaxBundlePage() const
{
	int const iMaxBundlePage = static_cast<int>(m_kContItemPage.size())/BUNDLE_UNIT;;
	return iMaxBundlePage;
}

bool PgCSGacha::EquipPreViewItem(lwUIWnd kWnd)
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

bool PgCSGacha::EquipPreViewItemUpdate(SGAMBLEMACHINEITEM const& kItemInfo)
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

bool PgCSGacha::EquipPreViewRareGroupItem(int const iGroupNo)
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

bool PgCSGacha::CallRareGroupTooltip(int const iGroupNo)
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

void PgCSGacha::ClearPreViewItem()
{
	RECV_UI_EQUIP_PREVIEW_CONT::iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{
		g_kCashShopMgr.DelPreviewEquip((*itor).dwItemNo);
		++itor;
	}
	m_kContPreViewItem.clear();
}

bool PgCSGacha::ExchangeSamePosEquipPreViewItem(SGAMBLEMACHINEITEM const& kItemInfo)
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

bool PgCSGacha::DelPreViewItem(int const iItemNo)
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

void PgCSGacha::UpdatePreViewItem()
{
	RECV_UI_EQUIP_PREVIEW_CONT::const_iterator itor = m_kContPreViewItem.begin();
	while(m_kContPreViewItem.end() != itor)
	{		
		g_kCashShopMgr.SetPreviewEquipItem((*itor).dwItemNo);			// 아이템 UI 모델에 장착
		++itor;
	}
	lwCashShop::SetPreviewEquipIcon(m_kContPreViewItem);	// 아이콘 장착창에 아이콘 넣기
}

int PgCSGacha::GetGroupNoBySlot(int iSlot)
{
	const int iRank = m_kContSortedRareGroupNo.at(iSlot);
	CONT_GAMBLEMACHINEGROUPRES::const_iterator iterRareGroup;
	for(iterRareGroup = m_kContRareItemGroupInfo.begin(); iterRareGroup != m_kContRareItemGroupInfo.end(); ++iterRareGroup)
	{
		if(iRank == (*iterRareGroup).second.iDisplayRank)
		{
			return (*iterRareGroup).first;
		}
	}
	return -1;
}

bool PgCSGacha::ShowRareItemPage(int const iPage)
{
	int const iMaxPage = GetMaxRareItemPage();
	if(0 > iPage || iMaxPage < iPage)
	{
		return false;
	}
		
	XUI::CXUI_Wnd* pkCashShop = XUIMgr.Get(FRM_CASH_SHOP);
	if(!pkCashShop)
	{
		return false;
	}
	XUI::CXUI_Wnd* pkGacha = pkCashShop->GetControl(SFRM_CASH_GACHA);
	if(!pkGacha)
	{
		return false;
	}

	int const iMaxRareGroupCnt = static_cast<int>(m_kContSortedRareGroupNo.size());
	int const iMaxCnt = lua_tinker::call<int>("CSGacha_MaxRareItemCntOnPage");
	int iDeActivateUICnt =0;
	for(int iBldIdx=0; iBldIdx < iMaxCnt; ++iBldIdx)
	{
		BM::vstring vStr(FRM_RARE_ITEM);
		vStr+=iBldIdx;
		XUI::CXUI_Wnd* pkRare  = pkGacha->GetControl(vStr);
		if(pkRare)
		{
			int iCurRare = (iPage*iMaxCnt)+iBldIdx;
			if(iCurRare < iMaxRareGroupCnt)
			{// 그룹 번호를 지정
				//int const iGroupNo = m_kContSortedRareGroupNo.at(iCurRare);
				int const iGroupNo = GetGroupNoBySlot(iCurRare);
				if(iGroupNo == -1)
				{ //현재 에러처리 되지 않음.
					continue;
				}
				////
				CONT_GAMBLEMACHINEGROUPRES::const_iterator RareInfo_itor = m_kContRareItemGroupInfo.find(iGroupNo);
				if(m_kContRareItemGroupInfo.end() != RareInfo_itor)
				{// 존재 하면

					XUI::CXUI_Wnd* pkItemWnd = pkRare->GetControl(ICN_ITEM);
					if(pkItemWnd)
					{
						SGAMBLEMACHINEGROUPRES const& kInfo = RareInfo_itor->second;
						pkItemWnd->DefaultImgName(kInfo.kIconPath);
						SUVInfo kUVInfo(kInfo.bU, kInfo.bV, kInfo.siUVIndex);

						{// 남녀 아이콘이 다를 경우 indxex를 계산함
							kUVInfo.Index += CalcGroupResIndex(iGroupNo);
						}

						pkItemWnd->UVInfo(kUVInfo);
						pkItemWnd->ImgSize(RARE_ITEM_GROUP_IMG_SIZE);	//기본적으로 캐시 아이콘 이미지파일 사이즈와 같음

						// 아이콘을 클릭하건, 툴팁을 봐야 할때 그룹정보를 사용하기 위해서 넣어준다
						pkRare->SetCustomData(&kInfo.iGroupNo, sizeof(kInfo.iGroupNo));
					}
				}
			}
			else
			{// 나머지는 아래 CSGacha_SetRareUIPos에서 보이지 않게함
				++iDeActivateUICnt;
			}
		}
	}

	// UI 위치 정렬을 수행한다
	int const iActivateUICnt = iMaxCnt-iDeActivateUICnt;
	lua_tinker::call<void, int, int>("CSGacha_SetRareUIPos", iActivateUICnt, iPage);
	SetCurrentRareItemPage(iPage);

	return true;
}

bool PgCSGacha::PrevRareItemPage()
{
	int const iRarePage = m_iCurrentRareItemPage-1;
	if(0 <= iRarePage )
	{
		ShowRareItemPage(iRarePage);
		return true;
	}
	return false;
}

bool PgCSGacha::NextRareItemPage()
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

int PgCSGacha::GetMaxRareItemPage()
{
	int const iMaxCnt = lua_tinker::call<int>("CSGacha_MaxRareItemCntOnPage");
	if(iMaxCnt == static_cast<int>(m_kContSortedRareGroupNo.size()))
	{
		return 0;
	}
	int iMaxPage= static_cast<int>(m_kContSortedRareGroupNo.size()) / iMaxCnt;
	return iMaxPage;
}

void PgCSGacha::NotifyEarnItem(PgBase_Item const& rkItem) const
{
	if(rkItem.IsEmpty())
	{
		return;
	}
	
	std::wstring kItemName;
	GetItemName(rkItem.ItemNo(), kItemName);
		
	BM::vstring vStr(TTW(790126));
	vStr.Replace(L"%s", kItemName.c_str());
	CallGetItemAlam2(static_cast<std::wstring>(vStr),rkItem.ItemNo());
	{// 행운 알림 UI가 떠있으면,
		XUI::CXUI_Wnd* pkNoti =	 XUIMgr.Get(L"FRM_NOTIFY_EARN_INGACHA_ITEM");

		if(pkNoti)
		{// 그사운드를 끄고
			lwStopSoundByID("Event_Firework");
		}
	}
	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_JAPAN:
		{
		}break;
	default:
		{// 아이템 얻음 사운드를 재생
			lwStopSoundByID("roulette_fanfare");
			lwPlaySoundByID("roulette_fanfare");
		}break;
	}
}

bool PgCSGacha::ShowErrorMsg(HRESULT const& kResult) const
{
	switch(kResult)
	{
	case E_GAMBLEMACHINE_INVALID_COIN:
		{// 잘못된 코인을 사용하였다.
			lwAddWarnDataTT(22020);
		}break;
	case E_GAMBLEMACHINE_INVALID_ITEM_DATA:
		{// 잘못된 아이템 테이블이다. (유저에게 보여줄 필요가 있을까...)
			lwAddWarnDataTT(976);
		}break;
	case E_GAMBLEMACHINE_OVER_RANGE_RATE:
		{// 랜덤 확률 범위를 벗어났다.(절대 발생하지 않아야 한다. 버그가 아니라면) 
			// 랜덤 확률을 벗어났다고 유저에게 알려줄 필요가 없으므로
			lwAddWarnDataTT(1974); // 알수 없는 오류입니다 메세지 표시
		}break;
	case E_GAMBLEMACHINE_NOT_FOUND_COIN:
		{// 코인을 찾을수 없다.
			lwAddWarnDataTT(2851);
		}break;
	case E_GAMBLEMACHINE_NOT_ENOUGH_CASH:
		{// 캐시가 부족하다.
			lwAddWarnDataTT(403079);
		}break;
	case E_GAMBLEMACHINE_ALREADY_GAMBLING:
		{// 이미 겜블 진행 중이다.
			lwAddWarnDataTT(790625);
		}break;
	case E_GAMBLEMACHINE_NOT_GAMBLING:
		{// 겜블 중이 아니다.
			lwAddWarnDataTT(790626);
		}break;
	case E_GAMBLEMACHINE_NOT_HAVE_ROULETTE:
		{// 룰렛 카운트가 없다.
			lwAddWarnDataTT(790627);
		}break;
// 코스튬 조합 에러 메세지도 포함 시킴 - - - - - - - - - 
	case E_MIXUP_NOT_FOUND_ITEM:
		{// 아이템을 찾을수 없다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790735, true);
		}break;
	case E_MIXUP_CANNOT_MIXUP_ITEM:
		{// 조합할 수 없는 아이템이다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790736, true);
		}break;
	case E_MIXUP_NOT_MATCH_EQUIP:
		{// 같은 장착 부위가 아니다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790737, true);
		}break;
	case E_MIXUP_CANT_USE_RATE_ITEM:
		{// 확률 증가 아이템을 사용할 수 없다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790738, true);
		}break;
	case E_MIXUP_NOT_MATCH_GENDER:
		{// 같은 성별 아이템이 아니다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790739, true);
		}break;
	case E_MIXUP_NOT_MATCH_GRADE:
		{// 같은 등급의 아이템이 아니다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790785, true);
		}break;
	case E_GAMBLEMACHINE_INVALID_MIXPOINT:
		{// 잘못된 조합 포인트 이다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790786, true);
		}break;
	case E_GAMBLEMACHINE_INVALID_MIXLISTSIZE:
		{// 잘못된 조합 테이블 이다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790787, true);
		}break;
	case E_GAMBLEMACHINE_NOT_USE_OVERDATE_ITEM:
		{// 기간 만료된 아이템은 조합할수 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790743, true);
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

bool PgCSGacha::CallSingleItemToolTip(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd *pkWnd = kWnd();
	if( !pkWnd )
	{
		return false;
	}

	SGAMBLEMACHINEITEM kItemInfo;
	pkWnd->GetCustomData(&kItemInfo, sizeof(kItemInfo));
	
	CallToolTip_ItemNo(kItemInfo.iItemNo, kWnd.GetLocation(), false, false, false, kItemInfo.bTimeType, kItemInfo.siUseTime);
	return true;
}

bool PgCSGacha::CallEquipItemToolTip(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd *pkWnd = kWnd();
	if( !pkWnd )
	{
		return false;
	}
	
	SPreviewItemInfo kItemInfo;
	pkWnd->GetCustomData(&kItemInfo, sizeof(kItemInfo));

	CallToolTip_ItemNo(kItemInfo.dwItemNo, kWnd.GetLocation(), false, false, false, kItemInfo.byTimeType, kItemInfo.siUseTime);
	return true;
}

int PgCSGacha::CalcGroupResIndex(int const iGroupNo)
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


namespace lwUICSGacha
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "ReqCSGachaItemList", lwUICSGacha::ReqCSGachaItemList);
		def(pkState, "CallCSGachaBuyCoinUI", lwUICSGacha::CallCSGachaBuyCoinUI);
		def(pkState, "UpdateCSGachaCoinCnt", lwUICSGacha::UpdateCsGachaCoinCnt);
		def(pkState, "ReqStartCSGachaRoulette", lwUICSGacha::ReqStartCSGachaRoulette);
		def(pkState, "ReqReloadCSGachaRoulette", lwUICSGacha::ReqReloadCSGachaRoulette);
		def(pkState, "ReqGetCSGachaResultItem", lwUICSGacha::ReqGetCSGachaResultItem);
		def(pkState, "GetCSGachaRemainReloadCnt", lwUICSGacha::GetCSGachaRemainReloadCnt);
		def(pkState, "DecCSGachaRemainReloadCnt", lwUICSGacha::DecCSGachaRemainReloadCnt);
		
		def(pkState, "PrevCSGachaItemPage", lwUICSGacha::PrevCSGachaItemPage);
		def(pkState, "NextCSGachaItemPage", lwUICSGacha::NextCSGachaItemPage);
		def(pkState, "GetCSGachaCurrentPage", lwUICSGacha::GetCSGachaCurrentPage);
		def(pkState, "SetCSGachaCurrentPage", lwUICSGacha::SetCSGachaCurrentPage);
		def(pkState, "ShowCSGachaItemPage", lwUICSGacha::ShowCSGachaItemPage);

		def(pkState, "ShowCSGachaPageBundle", lwUICSGacha::ShowCSGachaPageBundle);
		def(pkState, "PrevCSGachaPageBundle", lwUICSGacha::PrevCSGachaPageBundle);
		def(pkState, "NextCSGachaPageBundle", lwUICSGacha::NextCSGachaPageBundle);
		def(pkState, "GetCSGachaCurrentPageBundle", lwUICSGacha::GetCSGachaCurrentPageBundle);
		def(pkState, "SetCSGachaCurrentPageBundle", lwUICSGacha::SetCSGachaCurrentPageBundle);

		def(pkState, "SetCSGachaPreViewEquipItem", lwUICSGacha::SetCSGachaPreViewEquipItem);
		def(pkState, "ClearCSGachaPreViewEquipItem", lwUICSGacha::ClearCSGachaPreViewEquipItem);

		def(pkState, "EquipCSGachaPreViewRareGroupItem", lwUICSGacha::EquipCSGachaPreViewRareGroupItem);
		def(pkState, "CallCSGachaRareGroupTooltip", lwUICSGacha::CallCSGachaRareGroupTooltip);

		def(pkState, "ShowCSGachaRareItemPage", lwUICSGacha::ShowCSGachaRareItemPage);
		def(pkState, "PrevCSGachaRareItemPage", lwUICSGacha::PrevCSGachaRareItemPage);
		def(pkState, "NextCSGachaRareItemPage", lwUICSGacha::NextCSGachaRareItemPage);
		def(pkState, "GetCSGachaCurrentRareItemPage", lwUICSGacha::GetCSGachaCurrentRareItemPage);
		def(pkState, "SetCSGachaCurrentRareItemPage", lwUICSGacha::SetCSGachaCurrentRareItemPage);
		def(pkState, "GetCSGachaMaxRareItemPage", lwUICSGacha::GetCSGachaMaxRareItemPage);

		def(pkState, "CallCSGachaItemToolTip", lwUICSGacha::CallCSGachaItemToolTip);
		def(pkState, "CallCSGachaEquipItemToolTip", lwUICSGacha::CallCSGachaEquipItemToolTip);
	}

	void ReqCSGachaItemList()
	{// 가차 페이지 아이템 목록 요청
		g_kCSGacha.ReqCSGachaItemList();
	}

	void CallCSGachaBuyCoinUI()
	{// 코인 구매 UI 호출
		g_kCSGacha.CallBuyCoinUI();
	}
	void UpdateCsGachaCoinCnt(lwUIWnd kWnd)
	{// 소유한 코인 갯수 표시
		g_kCSGacha.UpdateOwnCoinCnt(kWnd);
	}

	bool ReqStartCSGachaRoulette()
	{// 룰렛 시작 요청
		return g_kCSGacha.ReqStartRoulette();
	}
	void ReqReloadCSGachaRoulette()
	{// 룰렛 리로드 요청
		g_kCSGacha.ReqReloadRoulette();
	}
	void ReqGetCSGachaResultItem()
	{// 최종 아이템 요청
		g_kCSGacha.ReqGetResultItem();
	}
	int GetCSGachaRemainReloadCnt()
	{// 룰렛 재설정이 얼마나 남았는가
		return g_kCSGacha.GetRemainReloadCnt();
	}
	bool DecCSGachaRemainReloadCnt()
	{// 룰렛 재설정 횟수 감소(기록용)
		return g_kCSGacha.DecRemainReloadCnt();
	}

	bool PrevCSGachaItemPage()
	{// 아이템 리스트의 이전 페이지 보기 버튼
		return g_kCSGacha.PrevPage();	
	}
	bool NextCSGachaItemPage()
	{// 아이템 리스트의 다음 페이지 보기 버튼
		return g_kCSGacha.NextPage();
	}
	int	GetCSGachaCurrentPage()
	{// 현재 페이지 값 얻기
		return g_kCSGacha.GetCurrentPage();
	}
	void SetCSGachaCurrentPage(int const iPageIdx)
	{// 현재 페이지 값 설정
		return g_kCSGacha.SetCurrentPage(iPageIdx);
	}
	bool ShowCSGachaItemPage(int const iPageIdx)
	{// 해당 페이지 아이템 리스트 보기
		return g_kCSGacha.ShowItemPage(iPageIdx);
	}

	bool ShowCSGachaPageBundle(int const iPageBundle)
	{// 페이지 묶음 보기(ex 0~3 page 인덱스) 
		return g_kCSGacha.ShowPageBundle(iPageBundle);
	}
	bool PrevCSGachaPageBundle()
	{// 이전 페이지 묶음보기
		return g_kCSGacha.PrevPageBundle();
	}
	bool NextCSGachaPageBundle()
	{// 다음 페이지 묶음보기
		return g_kCSGacha.NextPageBundle();
	}
	int	GetCSGachaCurrentPageBundle()
	{// 현재 페이지 묶음 idx 얻기
		return g_kCSGacha.GetCurrentPageBundle();
	}
	void SetCSGachaCurrentPageBundle(int const iPageBundle)
	{// 현재 페이지 묶음 idx 설정
		g_kCSGacha.SetCurrentPageBundle(iPageBundle);
	}

	void SetCSGachaPreViewEquipItem(lwUIWnd kWnd)
	{// 아이템 번호에 해당하는 아이템을 미리 보기에 장착 시키기(캐시샵 가차용)
		g_kCSGacha.EquipPreViewItem(kWnd);
	}
	void ClearCSGachaPreViewEquipItem()
	{// 미리보기에 아이템을 모두 벗기고, 미리 보기 아이콘도 제거
		g_kCSGacha.ClearPreViewItem();
	}

	bool EquipCSGachaPreViewRareGroupItem(int const iGroupNo)
	{// 레어 그룹번호에 해당하는 아이템을 미리보기
		return g_kCSGacha.EquipPreViewRareGroupItem(iGroupNo);
	}
	bool CallCSGachaRareGroupTooltip(int const iGroupNo)
	{// 레어 그룹번호에 해당하는 툴팁을 출력
		return g_kCSGacha.CallRareGroupTooltip(iGroupNo);
	}
	bool ShowCSGachaRareItemPage(int const iPage)
	{// 레어 그룹 페이지를 보여줌
		return g_kCSGacha.ShowRareItemPage(iPage);
	}
	bool PrevCSGachaRareItemPage()
	{// 이전 레어 그룹 페이지를 보여줌
		return g_kCSGacha.PrevRareItemPage();
	}
	bool NextCSGachaRareItemPage()
	{// 다음 레어 그룹 페이지를 보여줌
		return g_kCSGacha.NextRareItemPage();
	}
	int	 GetCSGachaCurrentRareItemPage()
	{// 현재 레어 그룹 페이지 값을 얻어옴
		return g_kCSGacha.GetCurrentRareItemPage();
	}
	void SetCSGachaCurrentRareItemPage(int const iPage)
	{// 현재 레어 그룹 페이지값을 설정함(기록용)
		g_kCSGacha.SetCurrentRareItemPage(iPage);
	}
	int  GetCSGachaMaxRareItemPage()
	{// 최대 레어 그룹페이지 값을 얻어옴
		return g_kCSGacha.GetMaxRareItemPage();
	}

	bool CallCSGachaItemToolTip(lwUIWnd kWnd)
	{
		return g_kCSGacha.CallSingleItemToolTip(kWnd);
	}

	bool CallCSGachaEquipItemToolTip(lwUIWnd kWnd)
	{
		return g_kCSGacha.CallEquipItemToolTip(kWnd);
	}
};