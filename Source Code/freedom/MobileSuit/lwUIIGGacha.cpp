#include "StdAfx.h"
#include "Variant/Global.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "lwUI.h"
#include "PgUISound.h"
#include "lwUIIGGacha.h"
#include "lwUIQuest.h"
#include "PgAchieveNfyMgr.h"

std::wstring const WSTR_INGAMEGAHCA_UI(L"SFRM_INGAME_GACHA");
std::wstring const WSTR_COIN_CNT(L"SFRM_COIN_CNT");
std::wstring const WSTR_BTN_COIN(L"BTN_COIN");

std::wstring const WSTR_CBTN_TAB(L"CBTN_TAB");

std::wstring const WSTR_SHOW_IDX(L"SFRM_IDX");
std::wstring const WSTR_ITEM_NAME(L"SFRM_ITEM_NAME");
std::wstring const WSTR_ITEM_TIME(L"SFRM_USE_TIME");

std::wstring const WSTR_NOTIFY_EARN(L"FRM_NOTIFY_EARN_INGACHA_ITEM");

std::wstring const WSTR_ICON_ITEM(L"ICON_ITEM");
std::wstring const WSTR_ICON(L"ICON");


// UICT타입으로 수정할것
int const GOLD_COIN_ITEMNO = 98003050;
int const GOLD_SILVER_ITEMNO = 98003060;
int const GOLD_BRONZE_ITEMNO = 98003070;

PgIGGacha::PgIGGacha()
{
}

PgIGGacha::~PgIGGacha()
{
	m_kContItemBag.clear();
}

void PgIGGacha::ReqIGGachaList()
{//0. 인게임 가챠서 뽑을수 있는 아이템 목록 요청
	BM::Stream kPacket(PT_C_M_REQ_GAMBLEMACHINEINFO);
	kPacket.Push(false);	// 캐시샵 가챠인가?
	NETWORK_SEND(kPacket);
}

void PgIGGacha::CallIGGachaUI()
{//1. 자판기 오픈
	XUI::CXUI_Wnd* pkMain = XUIMgr.Call(WSTR_INGAMEGAHCA_UI);
	if(!pkMain)
	{
		return;
	}
	UpdateCoinCnt(pkMain);
	// 골드 부분 아이템 최초로 먼저 보여줌
	ShowItemList(pkMain, GCG_GOLD);
}

bool PgIGGacha::SendUseCoin(eGambleCoinGrade const eCoinType)
{//2. 코인 사용
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
	int iCoinItemNo = 0;
	switch(eCoinType)
	{
	case GCG_GOLD:
		{
			iCoinItemNo = GOLD_COIN_ITEMNO;
		}break;
	case GCG_SILVER:
		{
			iCoinItemNo = GOLD_SILVER_ITEMNO;
		}break;
	case GCG_BRONZE:
		{
			iCoinItemNo = GOLD_BRONZE_ITEMNO;
		}break;
	default:
		{
			return 0;
		}break;
	}

	SItemPos kItemPos;
	if(E_FAIL == pkInv->GetFirstItem(iCoinItemNo, kItemPos))
	{// 코인 없음 메세지를 보여줌
		return false;
	}
	BM::Stream kPacket(PT_C_M_REQ_USE_GAMBLEMACHINE);
	kPacket.Push(kItemPos);
	NETWORK_SEND(kPacket);
	return true;
}

bool PgIGGacha::SendUseCash(eGambleCoinGrade const eCoinType)
{
	if(!CheckEnoughCash(eCoinType))
	{
		return false;
	}
	BYTE byCoinType=0;
	switch(eCoinType)
	{
	case GCG_GOLD:
		{
			byCoinType = 1;
		}break;
	case GCG_SILVER:
		{
			byCoinType = 2;
		}break;
	case GCG_BRONZE:
		{
			byCoinType = 3;
		}break;
	default:
		{
			return false;
		}break;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_GAMBLEMACHINE_CASH);
	kPacket.Push(byCoinType);
	NETWORK_SEND(kPacket);
	return true;
}

bool PgIGGacha::RecvIGGachaPacket(WORD const dwPacketType, BM::Stream& rkPacket)
{// 패킷을 받음
	switch(dwPacketType)
	{
	case PT_M_C_NFY_GAMBLEMACHINE_RESULT:
		{// 가챠 결과중 외치기 옵션이 있는 아이템 당첨시 서버 전체 공지가 옴
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
			}

			CONT_GAMBLEMACHINERESULT kResult;
			std::wstring kPlayerName;
			rkPacket.Pop(kPlayerName);
			PU::TLoadArray_M(rkPacket, kResult);
			if(false == kResult.empty())
			{
				NotifyLuckyGuy(kPlayerName, kResult.front());
			}
			return true;
		}break;
	case PT_M_C_ANS_USE_GAMBLEMACHINE:
		{// 가챠 결과
			HRESULT hResult = S_OK;
			rkPacket.Pop(hResult);
			if(S_OK == hResult)
			{//	B.	PgBase_Item : 결과 아이템 정보( 실패시에는 없음)
				CONT_GAMBLEMACHINERESULT kResult;
				PU::TLoadArray_M(rkPacket, kResult);
				if(false == kResult.empty())
				{
					// 아이템 얻음을 알림
					NotifyEarnItem(kResult.front());
				}
			}
			else
			{//얻은 아이템이 없으면
				switch(hResult)
				{
				case E_GAMBLEMACHINE_INVALID_COIN:
					{// 잘못된 코인을 사용하였다.
						lwAddWarnDataTT(22020);
					}break;
				case E_GAMBLEMACHINE_INVALID_ITEM_DATA:
					{// 잘못된 아이템 테이블이다. (유저에게 보여줄 필요가 있을까...)
						lwAddWarnDataTT(18053);
					}break;
				case E_GAMBLEMACHINE_OVER_RANGE_RATE:
					{// 랜덤 확률 범위를 벗어났다.(절대 발생하지 않아야 한다. 버그가 아니라면) 
						lwAddWarnDataTT(1923);
					}break;
				case E_GAMBLEMACHINE_NOT_FOUND_COIN:
					{// 코인을 찾을수 없다.
						lwAddWarnDataTT(1240);
					}break;
				case E_GAMBLEMACHINE_NOT_ENOUGH_CASH:
					{// 캐시가 부족하다.
						lwAddWarnDataTT(1900);
					}break;
				default:
					{// 정의 되지 않은 오류
						lwAddWarnDataTT(790413);
					}break;
				}
				StopRoulette(0);
			}
			return true;
		}break;
	case PT_M_C_ANS_GAMBLEMACHINEINFO:
		{
			PgPlayer const* const pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return false;
			}
			// 아이템 리스트가 올때 캐시 가격도 갱신됨
			g_kGambleMachine.ReadFromPacket(rkPacket);
			m_kContItemBag.clear();
			g_kGambleMachine.GetCont(m_kContItemBag, pkPlayer->UnitRace());	// 아이템 목록을 얻고
			Sort();										// 각 코인 종류별 아이템 확률 별로 정렬(낮은것 우선)
			CallIGGachaUI();							// UI를 호출함
			return true;
		}break;
	}
	return false;
}

bool PgIGGacha::ShowItemList(XUI::CXUI_Wnd* pkMain, eGambleCoinGrade const eCoinType)
{// 해당 코인 타입의 아이템 리스트를 보여준다.
	if(!pkMain)
	{
		return false;
	}

	for(int i=0; i < 3; ++i)
	{
		BM::vstring vStr(WSTR_CBTN_TAB);
		vStr+=i;
		XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkMain->GetControl(vStr));
		if(pkChkBtn)
		{
			pkChkBtn->Check(false);
		}
	}
	
	{
		BM::vstring vStr(WSTR_CBTN_TAB);
		switch(eCoinType)
		{
		case GCG_GOLD:
			{
				vStr+=0;
				XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkMain->GetControl(vStr));
				if(pkChkBtn)
				{
					pkChkBtn->Check(true);
				}
			}break;
		case GCG_SILVER:
			{
				vStr+=1;
				XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkMain->GetControl(vStr));
				if(pkChkBtn)
				{
					pkChkBtn->Check(true);
				}
			}break;
		case GCG_BRONZE:
			{
				vStr+=2;
				XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkMain->GetControl(vStr));
				if(pkChkBtn)
				{
					pkChkBtn->Check(true);
				}
			}break;
		default:
			{
				return false;
			}break;
		}
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkMain->GetControl(L"LIST_ITEM_LIST"));
	if(pkList)
	{
		// 리스트를 Clear 하고
		pkList->ClearList();

		// 코인에 해당하는 컨테이너를 얻어와
		CONT_GAMBLEMACHINE::const_iterator Gm_itor = m_kContItemBag.find(eCoinType);
		if(Gm_itor == m_kContItemBag.end())
		{
			return false;
		}
		
		SGAMBLEITEMBAG const& rkContItemBag = Gm_itor->second;
		CONT_GAMBLEITEM const& rkContGambleItem = rkContItemBag.kCont;

		CONT_GAMBLEITEM::const_iterator Item_itor = rkContGambleItem.begin();
		while(Item_itor != rkContGambleItem.end())
		{// 그 컨테이너에 들어있는 아이템 정보들을
			SGAMBLEMACHINEITEM const& rkItem = (*Item_itor);

			XUI::SListItem* pkListItem = pkList->AddItem(_T(""));
			if( pkListItem
				&&	pkListItem->m_pWnd )
			{// UI List에 넣어준다
				XUI::CXUI_Wnd* pkWnd = pkListItem->m_pWnd;
				XUI::CXUI_Wnd* pkName = pkWnd->GetControl(WSTR_ITEM_NAME);
				XUI::CXUI_Wnd* pkTime = pkWnd->GetControl(WSTR_ITEM_TIME);

				if(pkName)
				{
					std::wstring kItemName;
					if(GetItemName(rkItem.iItemNo, kItemName))
					{//아이템 이름
						int const iWidth = pkName->Width()+REGULATION_UI_WIDTH;
						Quest::SetCutedTextLimitLength(pkName, kItemName, WSTR_UI_REGULATIONSTR, iWidth);
					}
				}
				if(pkTime)
				{// 시간 설정
					BM::vstring vStr(rkItem.siUseTime);
					if(rkItem.siUseTime)
					{
						switch(rkItem.bTimeType)
						{
						case UIT_DAY:	//일
							vStr+=TTW(174);
							break;
						case UIT_HOUR:	//시간
							vStr+=TTW(90004);
							break;
						case UIT_MIN:	//분
							vStr+=TTW(176);
							break;
						}
					}
					else
					{//무기한
						vStr = TTW(178);
					}
					pkTime->Text(vStr);
				}

				XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(WSTR_ICON_ITEM);
				if( pkIcon )
				{// 아이콘 그릴수 있게 아이템 번호 등록
					pkIcon->SetCustomData(&rkItem.iItemNo, sizeof(rkItem.iItemNo));
				}
			}
			++Item_itor;
		}
	}
	return true;
}

void PgIGGacha::NotifyEarnItem(PgBase_Item const& rkItem) const
{// 얻은 아이템 알림
	if(rkItem.IsEmpty())
	{
		return;
	}
	std::wstring wstrForm(TTW(790126));
	std::wstring kItemName;
	GetItemName(rkItem.ItemNo(), kItemName);
	wchar_t szBuf[MAX_PATH] ={0,};
	::wsprintfW(szBuf, wstrForm.c_str(), kItemName.c_str());
	CallGetItemAlam(szBuf,rkItem.ItemNo());
	{// 행운 알림 UI가 떠있으면,
		XUI::CXUI_Wnd* pkNoti =	 XUIMgr.Get(WSTR_NOTIFY_EARN);
		if(pkNoti)
		{// 사운드를 그사운드를 끄고
			lwStopSoundByID("Event_Firework");
		}
	}
	// 아이템 얻음 사운드를 재생
	lwPlaySoundByID("Item_Gachapon_Result");
	// 아이템 보여줌
	StopRoulette(rkItem.ItemNo());
}

void PgIGGacha::StopRoulette(int const iResultItemNo) const
{
	XUI::CXUI_Wnd* pkMain = XUIMgr.Get(WSTR_INGAMEGAHCA_UI);
	if(pkMain)
	{
		XUI::CXUI_Wnd* pkImg = pkMain->GetControl(WSTR_ICON);
		if(pkImg)
		{
			pkImg->SetCustomData(&iResultItemNo, sizeof(iResultItemNo));
			lua_tinker::call<void>("ArrivedEarnPacketIGGacha");	//lua로 아이템 얻음 패킷이 왔음을 알림
		}
	}
}

void PgIGGacha::NotifyLuckyGuy(std::wstring const& rkPlayerName, PgBase_Item const& rkItem) const
{// 행운의 당첨자 알림
	wchar_t szBuf[MAX_PATH] ={0,};
	std::wstring kItemName;
	if(!GetItemName(rkItem.ItemNo(), kItemName))
	{
		return;
	}		

	std::wstring rkTransPlayerName;

	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_TAIWAN:
	case LOCAL_MGR::NC_THAILAND:
		{	
			if( g_pkWorld
				&& 0 != (g_pkWorld->GetAttr() & (GATTR_INSTANCE|GATTR_FLAG_STATIC_DUNGEON|GATTR_FLAG_BATTLESQUARE)) 
				)
			{//인던이면 표시하지 않는다.
				return ;
			}


			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return;
			}
			//플레이어랑 이름 같으면(자기자신이면) 기존의 UI,다른 플레이어라면 작은 폰트
			if(pkPlayer->Name() == rkPlayerName)
			{
				std::wstring kFormStr(TTW(790618));
				kFormStr = TTW(40004) + kFormStr;
				::wsprintfW(szBuf, kFormStr.c_str(), rkPlayerName.c_str(), kItemName.c_str());	
				XUI::CXUI_Wnd* pkNoti =	 XUIMgr.Call(WSTR_NOTIFY_EARN);
				if(!pkNoti)
				{
					return;
				}
				pkNoti->Text(szBuf);
			}
			else
			{
				rkTransPlayerName += rkPlayerName.at(0) + TTW(799300);	//유저네임 변경(첫글자 + 별5개)
				std::wstring kFormStr(TTW(799301));	//일본용 축하메세지 형식
				kFormStr = TTW(40003) + kFormStr;
				::wsprintfW(szBuf, kFormStr.c_str(), rkTransPlayerName.c_str(), kItemName.c_str());	

				PgSlideTextMgr::S_SLIDE_TEXT_INFO kInfo;
				kInfo.Type = PgSlideTextMgr::EATT_ITEM_RARE_NOTIFY;
				kInfo.kText = szBuf;
				g_kSlideTextMgr.Push(kInfo);

				XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_ACHIEVE_NEWS");
				if( pkMainUI && !pkMainUI->IsClosed() )
				{
					XUI::CXUI_Wnd* pkButton = pkMainUI->GetControl(L"BTN_GREET_MSG");
					if( pkButton )
					{
						pkButton->Close();
					}
					return;
				}
				else
				{
					pkMainUI = XUIMgr.Call(L"FRM_ACHIEVE_NEWS");
					if( !pkMainUI )
					{
						return;
					}
					XUI::CXUI_Wnd* pkButton = pkMainUI->GetControl(L"BTN_GREET_MSG");
					if( pkButton )
					{
						pkButton->Close();
					}
				}
				lua_tinker::call<void, lwUIWnd>("UpdateSlideTextViewUI", pkMainUI);
			}
		}break;
	default:
		{
			std::wstring kFormStr(TTW(790618));
			kFormStr = TTW(40004) + kFormStr;
			::wsprintfW(szBuf, kFormStr.c_str(), rkPlayerName.c_str(), kItemName.c_str());	
			XUI::CXUI_Wnd* pkNoti =	 XUIMgr.Call(WSTR_NOTIFY_EARN);
			if(!pkNoti)
			{
				return;
			}
			pkNoti->Text(szBuf);
	}break;	
	}
}

int PgIGGacha::GetMyPlayerCoinCnt(eGambleCoinGrade const eCoinType)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return 0;
	}
	int iCoinItemNo = 0;
	switch(eCoinType)
	{
	case GCG_GOLD:
		{
			iCoinItemNo = GOLD_COIN_ITEMNO;
		}break;
	case GCG_SILVER:
		{
			iCoinItemNo = GOLD_SILVER_ITEMNO;
		}break;
	case GCG_BRONZE:
		{
			iCoinItemNo = GOLD_BRONZE_ITEMNO;
		}break;
	default:
		{
			return 0;
		}break;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return 0;
	}
	// UICT타입으로 갯수를 얻어올것
	int iCoinCnt = pkInv->GetTotalCount(iCoinItemNo);
	return iCoinCnt;
}

bool PgIGGacha::CheckEnoughCash(eGambleCoinGrade const eCoinType)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}	

	int iNeedCash=0;
	__int64 const i64Money = pkPlayer->GetAbil64(AT_CASH);
	g_kGambleMachine.GetGambleMachineCost(eCoinType,iNeedCash);
	int const i64NeedCash = iNeedCash;
	if(i64NeedCash > i64Money)
	{
		return false;
	}
	return true;
}

bool PgIGGacha::PopLess(SGAMBLEMACHINEITEM const& rhs, SGAMBLEMACHINEITEM const& lhs)
{
	return rhs.iRate < lhs.iRate;
}

void PgIGGacha::Sort()
{//각 코인 종류별 아이템 확률 별로 정렬(낮은것 우선)
	m_kContResultItems.clear();
	for(int i=0; i< 3; ++i)
	{
		eGambleCoinGrade eCoinType = static_cast<eGambleCoinGrade>(GCG_GOLD+i);
		CONT_GAMBLEMACHINE::iterator Gm_itor = m_kContItemBag.find(eCoinType);
		if(Gm_itor == m_kContItemBag.end())
		{// 자료가 없음
			return;
		}
		SGAMBLEITEMBAG& rkContItemBag = Gm_itor->second;
		CONT_GAMBLEITEM& rkContGambleItem = rkContItemBag.kCont;
		std::sort(rkContGambleItem.begin(), rkContGambleItem.end(), PgIGGacha::PopLess);
		
		{// 아이템 이미지를 보여주기 위해 아이템 번호들을 따로 모은다.
			CONT_ITEMNO kContItemNo;
			CONT_GAMBLEITEM::const_iterator Item_itor = rkContGambleItem.begin();
			while(Item_itor != rkContGambleItem.end())
			{
				kContItemNo.push_back(Item_itor->iItemNo);
				++Item_itor;
			}
			m_kContResultItems.insert(std::make_pair(eCoinType, kContItemNo));
		}
	}
}

int PgIGGacha::GetItemNo(eGambleCoinGrade const eCoinType, int iIdx)
{
	CONT_IGGACHA_ITEMNO::const_iterator ContItemNo_itor = m_kContResultItems.find(eCoinType);
	if(ContItemNo_itor == m_kContResultItems.end())
	{// 컨테이너 체크
		return 0;
	}
	
	CONT_ITEMNO const& rkContItemNo = ContItemNo_itor->second;
	if(iIdx >= rkContItemNo.size()
		|| iIdx < 0)
	{// 사이즈 체크(empty 인경우도 포함됨)
		return 0;
	}
	return rkContItemNo.at(iIdx);
}

int PgIGGacha::GetItemNextItemNo(eGambleCoinGrade const eCoinType)
{
	CONT_IGGACHA_ITEMNO::const_iterator ContItemNo_itor = m_kContResultItems.find(eCoinType);
	if(ContItemNo_itor == m_kContResultItems.end())
	{// 컨테이너 체크
		return 0;
	}
	CONT_ITEMNO const& rkContItemNo = ContItemNo_itor->second;
	if(rkContItemNo.empty())
	{// 컨테이너 체크
		return 0;
	}
	switch(eCoinType)
	{
	case GCG_GOLD:
		{
			static size_t stIdx = 0;
			if(rkContItemNo.size() <= stIdx)
			{// idx 범위 보정
				stIdx = 0;
			}			
			return rkContItemNo.at(stIdx++);
		}break;
	case GCG_SILVER:
		{
			static size_t stIdx = 0;
			if(rkContItemNo.size() <= stIdx)
			{// idx 범위 보정
				stIdx = 0;
			}
			return rkContItemNo.at(stIdx++);
		}break;
	case GCG_BRONZE:
		{
			static size_t stIdx = 0;
			if(rkContItemNo.size() <= stIdx)
			{// idx 범위 보정
				stIdx = 0;
			}
			return rkContItemNo.at(stIdx++);
		}break;
	default:
		{
			return 0;
		}break;
	}
	return 0;
}

void PgIGGacha::UpdateCoinCnt(XUI::CXUI_Wnd* pkMain)
{
	if(!pkMain)
	{
		return;
	}

	for(int i=0; i < 3; ++i)
	{
		BM::vstring vStr(WSTR_COIN_CNT);
		vStr+=i;
		XUI::CXUI_Wnd* pkCoinCnt = pkMain->GetControl(vStr);
		if(pkCoinCnt)
		{// 코인의 갯수를 얻어와 넣어주고
			int iCoinCnt = GetMyPlayerCoinCnt(static_cast<eGambleCoinGrade>(GCG_GOLD+i));
			pkCoinCnt->Text(BM::vstring(iCoinCnt));

			{//코인 사용 버튼
				vStr=WSTR_BTN_COIN;
				vStr+=i;
				XUI::CXUI_Wnd* pkBtnCoin = pkMain->GetControl(vStr);
				if(pkBtnCoin)
				{
					if(0 == iCoinCnt)
					{// 코인 갯수가 0이면, 해당 버튼 비활성화
						pkBtnCoin->Enable(false);
					}
					else
					{// 코인 갯수가 0 아니면 활성화
						pkBtnCoin->Enable(true);
					}
				}
			}
		}
	}
}

void PgIGGacha::UpdateCashEnough(XUI::CXUI_Wnd* pkMain)
{
	if(!pkMain)
	{
		return;
	}

	for(int i=0; i < 3; ++i)
	{
		bool const bEnough = CheckEnoughCash(static_cast<eGambleCoinGrade>(GCG_GOLD+i));
		{//캐시 사용 버튼
			BM::vstring vStr(WSTR_BTN_COIN);
			vStr+=i;
			XUI::CXUI_Wnd* pkBtnCoin = pkMain->GetControl(vStr);
			if(pkBtnCoin)
			{
				if(!bEnough)
				{// 캐시가 부족하면 비활성화
					pkBtnCoin->Enable(false);
				}
				else
				{// 캐시가 충분하면 활성화
					pkBtnCoin->Enable(true);
				}
			}
		}
	}
}


namespace lwUIIGGacha
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "CallIGGachaUI", lwUIIGGacha::CallIGGachaUI);
		def(pkState, "ReqIGGachaList", lwUIIGGacha::ReqIGGachaList);
		def(pkState, "SendUseCoinIGGacha", lwUIIGGacha::SendUseCoinIGGacha);
		def(pkState, "SendUseCashIGGacha", lwUIIGGacha::SendUseCashIGGacha);
		def(pkState, "ShowItemListIGGacha", lwUIIGGacha::ShowItemListIGGacha);
		def(pkState, "UpdateCoinCntIGGacha", lwUIIGGacha::UpdateCoinCntIGGacha);
		def(pkState, "UpdateCashEnoughIGGacha", lwUIIGGacha::UpdateCashEnoughIGGacha);
		def(pkState, "GetNextItemNo", lwUIIGGacha::GetNextItemNo);
		def(pkState, "GetMyPlayerIGGachaCoinCnt", lwUIIGGacha::GetMyPlayerIGGachaCoinCnt);
		def(pkState, "CheckEnoughIGGachaCash", lwUIIGGacha::CheckEnoughIGGachaCash);
		def(pkState, "GetNeedCashIGGacha", lwUIIGGacha::GetNeedCashIGGacha);
	}

	void CallIGGachaUI()
	{
		g_kIGGacha.CallIGGachaUI();
	}

	void ReqIGGachaList()
	{
		g_kIGGacha.ReqIGGachaList();
	}

	bool SendUseCoinIGGacha(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return false;
		}
		return g_kIGGacha.SendUseCoin(static_cast<eGambleCoinGrade>(iCoinType));
	}

	bool SendUseCashIGGacha(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return false;
		}
		return g_kIGGacha.SendUseCash(static_cast<eGambleCoinGrade>(iCoinType));
	}

	bool ShowItemListIGGacha(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return false;
		}
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(WSTR_INGAMEGAHCA_UI);
		if(!pkMain)
		{
			return false;
		}
		return g_kIGGacha.ShowItemList(pkMain , static_cast<eGambleCoinGrade>(iCoinType));
	}

	void UpdateCoinCntIGGacha(lwUIWnd kMain)
	{
		g_kIGGacha.UpdateCoinCnt(kMain.GetSelf());
	}

	void UpdateCashEnoughIGGacha(lwUIWnd kMain)
	{
		g_kIGGacha.UpdateCashEnough(kMain.GetSelf());
	}

	int GetNextItemNo(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return 0;
		}
		int iItemNo = g_kIGGacha.GetItemNextItemNo(static_cast<eGambleCoinGrade>(iCoinType));
		return iItemNo;
	}

	int GetMyPlayerIGGachaCoinCnt(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return 0;
		}
		return g_kIGGacha.GetMyPlayerCoinCnt(static_cast<eGambleCoinGrade>(iCoinType));
	}

	bool CheckEnoughIGGachaCash(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return false;
		}
		return g_kIGGacha.CheckEnoughCash(static_cast<eGambleCoinGrade>(iCoinType));
	}

	int GetNeedCashIGGacha(int const iCoinType)
	{
		if(iCoinType < GCG_GOLD
			&& iCoinType > GCG_BRONZE)
		{
			return 0;
		}

		int iNeedCash=0;
		if(g_kGambleMachine.GetGambleMachineCost(static_cast<eGambleCoinGrade>(iCoinType),iNeedCash))
		{
			return iNeedCash;
		}
		return 0;
	}
};