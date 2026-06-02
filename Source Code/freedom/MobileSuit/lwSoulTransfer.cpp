#include "stdafx.h"
#include "Variant/TableDataManager.h"
#include "ServerLib.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgMobileSuit.h"
#include "lwSoulTransfer.h"
#include "PgUISound.h"

namespace lwSoulTransfer
{	
	std::wstring const TRANSFER_MAIN(L"SFRM_SOULABIL_TRANSFER");
		//메뉴
		std::wstring const TITLE(L"SFRM_TITLE");
			std::wstring const TITLE_COLOR(L"SFRM_COLOR");
				std::wstring const TITLE_SDW(L"SFRM_TITLE_SDW");
		//메인
		std::wstring const MAIN_FRAME(L"FRM_MAIN_FRAME");
		//영력추출/영력보관구슬 창
			std::wstring const RESOURCE_ANI(L"FRM_RESOURCE_ANI");
			std::wstring const RESOURCE_ITEM(L"SFRM_RESOURCE_ITEM");
		//영력삽입 대상아이템창
			std::wstring const DEST_ANI(L"FRM_DEST_ANI");
			//std::wstring const DEST_BIND(L"FRM_DEST_BIND");
			std::wstring const DEST_ITEM(L"SFRM_DEST_ITEM");
		//보험
			std::wstring const INSURANCE_BIND(L"FRM_INSURANCE_BIND");
			std::wstring const INSURANCE_ITEM(L"SFRM_INSURANCE_ITEM");
			std::wstring const DEL_INSURANCE_ITEM(L"BTN_DEL_INSURANCE_ITEM");
			std::wstring const ADD_INSURANCE_ITEM(L"BTN_ADD_INSURANCE_ITEM");
		//확률증가
			std::wstring const RATEUP_BIND(L"FRM_RATEUP_BIND");
			std::wstring const RATEUP_ITEM(L"SFRM_RATEUP_ITEM");
			std::wstring const DEL_RATEUP_ITEM(L"BTN_DEL_RATEUP_ITEM");
			std::wstring const ADD_RATEUP_ITEM(L"BTN_ADD_RATEUP_ITEM");
		//결과창
			std::wstring const RESULT_ANI(L"FRM_RESULT_ANI");
			std::wstring const RESULT_ITEM(L"SFRM_RESULT_ITEM");
		//확률창
			std::wstring const SUCCESS_RATE(L"FRM_SUCCESS_RATE");
				std::wstring const RATE_TITLE(L"FRM_TITLE_TEXT");
				std::wstring const CURRENT_PER(L"FRM_CURRENT_PER");
				std::wstring const MAX_PER(L"FRM_MAX_PER");
		//진행 화살표
			std::wstring const PROGRESS_MAIN(L"FRM_PROGRESS_MAIN");
			std::wstring const PROGRESS_INSURANCE(L"FRM_PROGRESS_INSURANCE");
			std::wstring const PROGRESS_RATEUP(L"FRM_PROGRESS_RATEUP");
		//제한사항
			std::wstring const LIMIT_MAIN(L"FRM_LIMIT_MAIN");
			std::wstring const LIMIT_SUB(L"FRM_LIMIT_SUB");
		//필요 소울
			std::wstring const NEED_SOUL(L"FRM_NEED_SOUL");
		//필요 골드
			std::wstring const GOLD(L"FRM_GOLD");
			std::wstring const SILVER(L"FRM_SILVER");
			std::wstring const COPPER(L"FRM_COPPER");
		//설명
		std::wstring const INFO_SHADOW(L"SFRM_INFO_SHADOW");
		//시작
		std::wstring const PROCESS_START(L"BTN_PROCESS_START");
		//COLOR
		std::wstring const TEXT_RED(L"{C=0XFFFF0000/}");
		std::wstring const TEXT_GREEN(L"{C=0XFF00FF00/}");

	int const INSURANCE_PROGRESS_MAIN_MAX = 22;
	int const RATEUP_PROGRESS_MAIN_MAX = 43;

	struct S_UPDATE_TIME
	{
		float fTotalTime;
		float fStartTime;
	};

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallSoulTransferUI", lwCallSoulTransferUI);

		def(pkState, "SetProgressMain", lwSetProgressMain);
		def(pkState, "SetProgressInsurance", lwSetProgressInsurance);
		def(pkState, "SetProgressRateUp", lwSetProgressRateUp);
		
		def(pkState, "CanStartSoulTransProgress", lwCanStartSoulTransProgress);
		def(pkState, "IsEndSoulTransProgress", lwIsEndSoulTransProgress);
		def(pkState, "StartSoulTransProgress", lwStartSoulTransProgress);
		def(pkState, "UpdateSoulTransProgress", lwUpdateSoulTransProgress);
		
		def(pkState, "SoulTrans_IconAni", lwSoulTrans_IconAni);
		def(pkState, "AutoInsertItem", lwAutoInsertItem);
		def(pkState, "SetExtractitem", lwSetExtractitem);
		
		def(pkState, "SetAdditionItem", lwSetAdditionItem);
		
		def(pkState, "AddInsuranceItem", lwAddInsuranceItem);
		def(pkState, "DelInsuranceItem", lwDelInsuranceItem);
		def(pkState, "UseInsuranceItem", lwUseInsuranceItem);
		
		
		def(pkState, "AddRateUpItem", lwAddRateUpItem);
		def(pkState, "DelRateUpItem", lwDelRateUpItem);
		
		def(pkState, "IsSoulInsertUI", IsSoulInsertUI);
		def(pkState, "SetSoulTransText", lwSetSoulTransText);
		def(pkState, "SetSoulTrans_ResultItem", lwSetSoulTrans_ResultItem);

		def(pkState, "IsHaveRarity", lwIsHaveRarity);
		
	}

	void lwCallSoulTransferUI(bool const bSoulInsert)
	{
		int iInvViewGroup = 0;
		if(bSoulInsert)
		{
			iInvViewGroup = 2;
		}
		else
		{
			iInvViewGroup = 1;
		}
		lua_tinker::call<void,int>("PreChangeInvViewGroupTop", iInvViewGroup);		
		XUI::CXUI_Wnd* pInv = XUIMgr.Get(L"Inv");
		if(!pInv)
		{
			XUIMgr.Call(L"Inv");
		}

		XUI::CXUI_Wnd* pMain = XUIMgr.Call(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		InitMainText(pMain, bSoulInsert);
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			//진행바 초기화
			InitProgress(pMainFrame, bSoulInsert);
			//버튼 설정
			InitIcon(pMainFrame, bSoulInsert);
			//텍스트 설정
			InitFrameText(pMainFrame, bSoulInsert);
		}
	}

///////////////////SetText//////////////////////////
	void InitMainText(XUI::CXUI_Wnd* pMain, bool const bSoulInsert)
	{
		if(!pMain)
		{
			return;
		}
		//영력삽입,추출별 텍스트 설정
		XUI::CXUI_Wnd* pTitleWnd = pMain->GetControl(TITLE);
		if(!pTitleWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pTitle= pTitleWnd->GetControl(TITLE_COLOR);
		if(pTitle)
		{
			XUI::CXUI_Wnd* pText= pTitle->GetControl(TITLE_SDW);
			if(pText)
			{
				if(bSoulInsert)
				{
					pText->Text(TTW(799865));
				}
				else
				{
					pText->Text(TTW(799850));
				}
			}
		}
		XUI::CXUI_Wnd* pInfo = pMain->GetControl(INFO_SHADOW);
		if(!pInfo)
		{
			return;
		}
		if(bSoulInsert)
		{
			pInfo->Text(TTW(799873));
		}
		else
		{
			pInfo->Text(TTW(799857));
		}
		
		XUI::CXUI_Wnd* pStart = pMain->GetControl(PROCESS_START);
		if(!pStart)
		{
			return;
		}
		if(bSoulInsert)
		{
			pStart->Text(TTW(799874) + TTW(2200));
		}
		else
		{
			pStart->Text(TTW(799858) + TTW(2200));
		}
		pMain->SetCustomData(&bSoulInsert, sizeof(bSoulInsert));
	}
	void InitFrameText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert)
	{
		if(!pFrame)
		{
			return;
		}
		SetLimitText(pFrame, bSoulInsert, PgBase_Item::NullData());
		SetRateText(pFrame, bSoulInsert, PgBase_Item::NullData(), PgBase_Item::NullData());
		SetNeedSoulText(pFrame, 0);
		SetNeedGoldText(pFrame, 0);
	}
	void SetRateText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert, PgBase_Item const &kTransitionItem, PgBase_Item const &kDestItem)
	{
		if(!pFrame)
		{
			return;
		}
		XUI::CXUI_Wnd* pRateWnd = pFrame->GetControl(SUCCESS_RATE);
		if(!pRateWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pRate_Title= pRateWnd->GetControl(RATE_TITLE);
		XUI::CXUI_Wnd* pRate_Cur= pRateWnd->GetControl(CURRENT_PER);
		XUI::CXUI_Wnd* pRate_Max= pRateWnd->GetControl(MAX_PER);
		if(!pRate_Cur || !pRate_Max || !pRate_Title)
		{
			return;
		}
		if( bSoulInsert )
		{
			BM::vstring kTitle(TTW(799842));
			pRate_Title->Text(kTitle);
			int iCurrent = 0;
			int iMax = 0;
			int const iGrade = GetItemGrade(kTransitionItem);
			CONT_DEF_JOBSKILL_SOUL_TRANSITION::value_type kTransitionInfo;
			if( true == SoulTransitionUtil::GetSoulTransitionInfo(kDestItem.ItemNo(), iGrade, kTransitionInfo) )
			{
				iCurrent = kTransitionInfo.iSuccessRate + (kTransitionInfo.iProbabilityUpRate * lwGetRateUpItemCount());
				iMax = kTransitionInfo.iSuccessRate + (kTransitionInfo.iProbabilityUpRate * kTransitionInfo.iProbabilityUpItemCount);
			}
			std::wstring kCurrentText;
			if( 0 != iCurrent 
			 && 0 != iMax)
			{
				if( iCurrent != iMax )
				{
					kCurrentText += TEXT_RED;
				}
				else
				{
					kCurrentText += TEXT_GREEN;
				}
			}
			BM::vstring kPercentCur(TTW(799845));
			BM::vstring kPercentMax(TTW(799845));
			kPercentCur.Replace(L"#PER#", (iCurrent*100)/SOUL_TRANSITION_RATE);
			kPercentMax.Replace(L"#PER#", (iMax*100)/SOUL_TRANSITION_RATE);

			kCurrentText += static_cast<std::wstring>(kPercentCur);

			pRate_Cur->Text(kCurrentText);
			pRate_Max->Text(kPercentMax);
		}
		else
		{
			BM::vstring kTitle(TTW(799849));
			pRate_Title->Text(kTitle);
			BM::vstring kPercentText(TTW(799845));
			kPercentText.Replace(L"#PER#", 100);
			pRate_Cur->Text(kPercentText);
			pRate_Max->Text(kPercentText);
		}
	}
	void SetRateText()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		bool bSoulInsert = false;
		pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert));
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			SItemPos kResourceItemPos;
			GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
			PgBase_Item kResourceItem;
			GetMyItem(kResourceItemPos, kResourceItem);
			SItemPos kDestItemPos;
			GetSelectItemPos(DEST_ITEM, kDestItemPos);
			PgBase_Item kDestItem;
			GetMyItem(kDestItemPos, kDestItem);
			SetRateText(pMainFrame, bSoulInsert, kResourceItem, kDestItem);
		}
	}
	void SetLimitText(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert, PgBase_Item const &kItem)
	{
		if(!pFrame)
		{
			return;
		}
		XUI::CXUI_Wnd* pLimit_Main = pFrame->GetControl(LIMIT_MAIN);
		XUI::CXUI_Wnd* pLimit_Sub= pFrame->GetControl(LIMIT_SUB);
		if(!pLimit_Main || !pLimit_Main)
		{
			return;
		}
		if(!bSoulInsert)
		{
			pLimit_Main->Text(L"");
			pLimit_Sub->Text(L"");
			return;
		}
		BM::vstring kLimit_Level(TTW(799846));
		BM::vstring kLimit_Equip(TTW(799871));

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if(pkItemDef)
		{
			BM::vstring kLevelText(TTW(799847));
			int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
			if( 0 != iLevelLimit )
			{
				switch(g_kLocal.ServiceRegion())
				{
				case LOCAL_MGR::NC_JAPAN:
				case LOCAL_MGR::NC_TAIWAN:
					{//일본, 대만은 허용 레벨 범위가 [1Lv ~ 아이템레벨 + 10Lv]
						kLevelText.Replace(L"#MIN#", 1);
					}break;
				default:
					{//기본 허용 레벨 범위는 [아이템 레벨 ~ 아이템 레벨 +10]
						kLevelText.Replace(L"#MIN#", iLevelLimit);
					}break;
				}
				kLevelText.Replace(L"#MAX#", iLevelLimit+10);
				kLimit_Level += kLevelText;
			}
			BM::vstring kEquipText(TTW(799872));
			std::wstring wstrEquipPos;
			GetEqiupPosString( pkItemDef->GetAbil(AT_EQUIP_LIMIT), false, wstrEquipPos);
			if( 0 != wstrEquipPos.size() )
			{
				kEquipText.Replace(L"#EQUIPPOS#", wstrEquipPos);
				kLimit_Equip += kEquipText;
			}
		}
		pLimit_Main->Text(kLimit_Level);
		pLimit_Sub->Text(kLimit_Equip);
	}
	void SetNeedSoulText(XUI::CXUI_Wnd* pFrame, int const iNeedSoul)
	{
		if(!pFrame)
		{
			return;
		}
		BM::vstring kText(TTW(799840));
		kText.Replace(L"#NUM#", iNeedSoul);
		XUI::CXUI_Wnd* pText = pFrame->GetControl(NEED_SOUL);
		if(!pText)
		{
			return;
		}
		pText->Text(kText);
	}
	void SetNeedGoldText(XUI::CXUI_Wnd* pFrame, __int64 const i64NeedMoney)
	{
		if(!pFrame)
		{
			return;
		}
		XUI::CXUI_Wnd* pGold = pFrame->GetControl(GOLD);
		XUI::CXUI_Wnd* pSilver = pFrame->GetControl(SILVER);
		XUI::CXUI_Wnd* pCopper = pFrame->GetControl(COPPER);
		if( !pGold || !pSilver || !pCopper )
		{
			return;
		}
		__int64	i64Gold = i64NeedMoney / 10000;
		pGold->Text(BM::vstring(i64Gold));

		int iSilver = (int)((i64NeedMoney % 10000) / 100);
		pSilver->Text(BM::vstring(iSilver));

		int iCopper = (int)(i64NeedMoney % 100);
		pCopper->Text(BM::vstring(iCopper));

	}
	void ResetUI()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		bool bSoulInsert = false;
		pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert));
		InitMainText(pMain, bSoulInsert);
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			//진행바 초기화
			InitProgress(pMainFrame, bSoulInsert);
			//버튼 설정
			InitIcon(pMainFrame, bSoulInsert);
			//텍스트 설정
			InitFrameText(pMainFrame, bSoulInsert);

			if(bSoulInsert)
			{
				SItemPos kResourceItemPos;
				GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
				PgBase_Item kResourceItem;
				GetMyItem(kResourceItemPos, kResourceItem);

				SItemPos kDestItemPos;
				GetSelectItemPos(DEST_ITEM, kDestItemPos);
				PgBase_Item kDestItem;
				GetMyItem(kDestItemPos, kDestItem);
				
				SetRateText(pMainFrame, bSoulInsert, kResourceItem, kDestItem);
			}
		}
	}
///////////////////SetIcon//////////////////////////
	void InitIcon(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert)
	{
		if(!pFrame)
		{
			return;
		}
		XUI::CXUI_Wnd* pResourceItem = pFrame->GetControl(RESOURCE_ITEM);
		XUI::CXUI_Wnd* pDestItem = pFrame->GetControl(DEST_ITEM);
		XUI::CXUI_Wnd* pResultItem = pFrame->GetControl(RESULT_ITEM);
		XUI::CXUI_Wnd* pInsuranceItem = pFrame->GetControl(INSURANCE_ITEM);
		XUI::CXUI_Wnd* pRateUpItem = pFrame->GetControl(RATEUP_ITEM);
		if(!pResourceItem
		|| !pDestItem
		|| !pResultItem
		|| !pInsuranceItem
		|| !pRateUpItem)
		{
			return;
		}
		XUI::CXUI_Wnd* pResourceIcon = pResourceItem->GetControl(L"Icon");
		XUI::CXUI_Wnd* pDestIcon = pDestItem->GetControl(L"Icon");
		XUI::CXUI_Wnd* pResultIcon = pResultItem->GetControl(L"Icon");
		XUI::CXUI_Wnd* pInsuranceIcon = pInsuranceItem->GetControl(L"Icon");
		XUI::CXUI_Wnd* pRateUpIcon = pRateUpItem->GetControl(L"Icon");
		if(!pResourceIcon
		|| !pDestIcon
		|| !pResultIcon
		|| !pInsuranceIcon
		|| !pRateUpIcon)
		{
			return;
		}
		pResourceIcon->ClearCustomData();
		pDestIcon->ClearCustomData();
		pResultIcon->ClearCustomData();
		pInsuranceIcon->ClearCustomData();
		pRateUpIcon->ClearCustomData();

		XUI::CXUI_Icon* pDest = dynamic_cast<XUI::CXUI_Icon*>(pDestIcon);
		if(pDest)
		{
			SIconInfo kIconInfo = pDest->IconInfo();
			if( bSoulInsert)
			{//영력 삽입이면, 
				kIconInfo.iIconGroup = KUIG_COMMON_INV_POS;
			}
			else
			{
				kIconInfo.iIconGroup = KUIG_ITEM_COUNT;
			}
			pDest->SetIconInfo(kIconInfo);
		}
		
		XUI::CXUI_Wnd* pResourceAni = pFrame->GetControl(RESOURCE_ANI);
		XUI::CXUI_Wnd* pDestAni = pFrame->GetControl(DEST_ANI);
		XUI::CXUI_Wnd* pResultAni = pFrame->GetControl(RESULT_ANI);
		XUI::CXUI_Wnd* pInsurance_Bind = pFrame->GetControl(INSURANCE_BIND);
		XUI::CXUI_Wnd* pRateUp_Bind = pFrame->GetControl(RATEUP_BIND);
		if(!pResourceAni
		|| !pDestAni
		|| !pResultAni
		//|| !pDestBind
		|| !pInsurance_Bind
		|| !pRateUp_Bind)
		{
			return;
		}
		pResourceAni->Visible(false);
		pDestAni->Visible(false);
		pResultAni->Visible(false);
		pInsurance_Bind->Visible(!bSoulInsert);
		pRateUp_Bind->Visible(!bSoulInsert);
		lwSetAdditionItem();
	}
///////////////////SetAniBar//////////////////////////
	void InitProgress(XUI::CXUI_Wnd* pFrame, bool const bSoulInsert)
	{
		if(!pFrame)
		{
			return;
		}
		InitProgressTime();
		SetProgress(0, true, PROGRESS_MAIN);
		SetProgress(0, false, PROGRESS_INSURANCE);
		SetProgress(0, false, PROGRESS_RATEUP);
	}

	void SetProgress(float fPercent, bool const bIsMain, std::wstring kWndText)
	{
		if( 100.0f < fPercent)
		{
			fPercent = 100.0f;
		}
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pProgressWnd = pMainFrame->GetControl(kWndText);
			if(pProgressWnd)
			{
				POINT2 const ptWndSize = pProgressWnd->Size();
				int iSize = 0;
				if(bIsMain)
				{
					iSize = ptWndSize.x;
					int iPercentSize = static_cast<int>(iSize * fPercent/100);
					pProgressWnd->ImgSize(POINT2(iPercentSize, ptWndSize.y));
				}
				else
				{
					iSize = ptWndSize.y;
					int iPercentSize = static_cast<int>(iSize * fPercent/100);
					pProgressWnd->ImgSize(POINT2(ptWndSize.x, iPercentSize));
				}
			}
		}
	}

	void lwSetProgressMain(float const fPercent)
	{
		SetProgress(fPercent, true, PROGRESS_MAIN);
	}
	void lwSetProgressInsurance(float const fPercent)
	{
		SetProgress(fPercent, false, PROGRESS_INSURANCE);
	}
	void lwSetProgressRateUp(float const fPercent)
	{
		SetProgress(fPercent, false, PROGRESS_RATEUP);
	}

	void InitProgressTime()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			S_UPDATE_TIME kTime;
			kTime.fStartTime = 0.0f;
			kTime.fTotalTime = 0.0f;
			pMainFrame->SetCustomData(&kTime, sizeof(kTime));
		}
	}

	bool lwCanStartSoulTransProgress()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		bool bSoulInsert = false;
		pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert));
		SItemPos kResourceItemPos;
		if( !GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos))
		{
			if(!bSoulInsert)
			{
				CallSoulTransMessage(ESTM_NOT_SELECT_EXTRACT_ITEM);
			}
			else
			{
				CallSoulTransMessage(ESTM_NOT_SELECT_TRANSITION_ITEM);
			}
			return false;
		}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgInventory* pInv = pPlayer->GetInven();
		if(!pInv)
		{
			return false;
		}
		if(bSoulInsert)
		{// 삽입일 경우엔 아이템위치로 저장
			PgBase_Item kResourceItem;
			GetMyItem(kResourceItemPos, kResourceItem);
			SItemPos kDestItemPos;
			if(!GetSelectItemPos(DEST_ITEM, kDestItemPos) )
			{
				CallSoulTransMessage(ESTM_NOT_SELECT_DEST_ITEM);
				return false;
			}
			PgBase_Item kDestItem;
			GetMyItem(kDestItemPos, kDestItem);
			if( 0 == kDestItem.Count() )
			{
				CallSoulTransMessage(ESTM_NOT_HAVE_EXTRACT_ITEM);
				return false;
			}
			if(!SoulTransitionUtil::IsTransition(kResourceItem, kDestItem) )
			{
				CallSoulTransMessage(ESTM_NOT_CORRECT_TRANSITION);
				return false;
			}
			//삽입 정보 찾고
			int const iGrade = GetItemGrade(kResourceItem);
			CONT_DEF_JOBSKILL_SOUL_TRANSITION::value_type kTransitionInfo;
			if( !SoulTransitionUtil::GetSoulTransitionInfo(kDestItem.ItemNo(), iGrade, kTransitionInfo) )
			{
				CallSoulTransMessage(ESTM_FAILED_FIND_TRANSITION_INFO);
				return false;
			}
			//보험 적용했으면 삽입 정보에 있는 보험 아이템 찾고
			if(lwUseInsuranceItem())
			{
				PgBase_Item kInsuranceItem;
				SItemPos kInsuranceItemPos;
				if( E_FAIL == pInv->GetFirstItem(kTransitionInfo.iInsuranceitemNo, kInsuranceItemPos) )
				{//없으면 에러
					CallSoulTransMessage(ESTM_NOT_HAVE_INSURANCE);
					return false;
				}
				if( E_FAIL == pInv->GetItem( kInsuranceItemPos, kInsuranceItem) )
				{
					CallSoulTransMessage(ESTM_NOT_HAVE_INSURANCE);
					return false;
				}
			}
			//확률 아이템 적용?(갯수가 0개 이상이냐)
			int iRateUpItemCount = lwGetRateUpItemCount();
			if( 0 < iRateUpItemCount )
			{
				SItemPos kRateUpItemPos;
				PgBase_Item kRateUpItem;
				if( iRateUpItemCount > pInv->GetInvTotalCount( kTransitionInfo.iProbabilityUpItemNo ) )
				{//가지고 있는 갯수가 패킷에서 보낸 갯수 보다 작으면 에러
					CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
					return false;
				}
				if(E_FAIL == pInv->GetFirstItem(kTransitionInfo.iProbabilityUpItemNo, kRateUpItemPos) )
				{//없으면 에러
					CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
					return false;
				}
				if( E_FAIL == pInv->GetItem( kRateUpItemPos, kRateUpItem) )
				{
					CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
					return false;
				}
			}
			//소울 충분한지 확인
			SItemPos kSoulItemPos;
			PgBase_Item kSoulItem;
			if( 0 < kTransitionInfo.iSoulItemCount )
			{
				if( kTransitionInfo.iSoulItemCount > pInv->GetInvTotalCount( ITEM_SOUL_NO ) )
				{//가지고 있는 갯수가 패킷에서 보낸 갯수 보다 작으면 에러
					CallSoulTransMessage(ESTM_NOT_ENOUGH_SOUL);
					return false;
				}
				if( E_FAIL == pInv->GetFirstItem(ITEM_SOUL_NO, kSoulItemPos) )
				{//없으면 에러
					CallSoulTransMessage(ESTM_NOT_ENOUGH_SOUL);
					return false;
				}
			}
			//소지금 충분한지 확인
			if( 0 < kTransitionInfo.iNeedMoney )
			{
				if( kTransitionInfo.iNeedMoney > pPlayer->GetAbil64(AT_MONEY) )
				{
					CallSoulTransMessage(ESTM_NOT_ENOUGH_MONEY);
					return false;
				}
			}
		}
		else
		{
			PgBase_Item kResourceItem;
			GetMyItem(kResourceItemPos, kResourceItem);
			if( !SoulTransitionUtil::IsSoulExtractItem(kResourceItem) )
			{
				CallSoulTransMessage(ESTM_CANT_EXTRACT_ITEM);
				return false;
			}
			PgBase_Item kDestItem;
			GetSelectItem(DEST_ITEM, kDestItem);
			if( 0 == kDestItem.Count() )
			{
				CallSoulTransMessage(ESTM_NOT_HAVE_EXTRACT_ITEM);
				return false;
			}
		}
		SItemPos kItemPos;
		if( !bSoulInsert
		 && !pInv->GetNextEmptyPos(IT_CONSUME, kItemPos) )
		{
			CallSoulTransMessage(ESTM_INVENTORY_FULL);
			return false;
		}
		return true;
	}
	bool lwIsHaveRarity()
	{//영력을 가지고 있는 아이템이냐
		SItemPos kItemPos;
		GetSelectItemPos(DEST_ITEM, kItemPos);
		PgBase_Item kItem;
		GetMyItem(kItemPos, kItem);

		SEnchantInfo kEnchantInfo;
		kEnchantInfo = kItem.EnchantInfo();
		if( 0 != kEnchantInfo.BasicType1()
			|| 0 != kEnchantInfo.BasicType2()
			|| 0 != kEnchantInfo.BasicType3()
			|| 0 != kEnchantInfo.BasicType4() )
		{
			return false;
		}
		return true;
	}
	bool lwIsEndSoulTransProgress()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(!pMainFrame)
		{
		}
		S_UPDATE_TIME kTime;
		if( false == pMainFrame->GetCustomData(&kTime, sizeof(kTime)) )
		{
			return false;
		}
		if( 0 == kTime.fStartTime
			|| 0 ==kTime.fTotalTime)
		{
			return true;
		}
		return false;
	}

	void lwStartSoulTransProgress(float fTotalTime)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		bool bSounInsert = false;
		pMain->GetCustomData(&bSounInsert, sizeof(bSounInsert));
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			InitProgress(pMainFrame, bSounInsert);
			//fTime = fTime*1000;
			S_UPDATE_TIME kTime;
			kTime.fStartTime = g_pkApp->GetAccumTime();
			kTime.fTotalTime = fTotalTime;
			pMainFrame->SetCustomData(&kTime, sizeof(kTime));
			lwPlaySoundByID("soulpower-gauge");
		}
	}
	void lwUpdateSoulTransProgress()
	{
		bool bSoulInsert = true;
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		if( false == pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert)) )
		{
			return;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			S_UPDATE_TIME kTime;
			if( false == pMainFrame->GetCustomData(&kTime, sizeof(kTime)) )
			{
				return;
			}
			if(0.0f == kTime.fTotalTime)
			{
				return;
			}
			float const fNowTime = g_pkApp->GetAccumTime();
			float const fGapTime = fNowTime- kTime.fStartTime;
			float fPercent = 0.0f;
			if( fGapTime >= kTime.fTotalTime )
			{
				fPercent = 100.0f;
				kTime.fStartTime = 0.0f;
				kTime.fTotalTime = 0.0f;
				pMainFrame->SetCustomData(&kTime, sizeof(kTime));
				lwStopSoundByID("Item_Mix_Mixing");
				if(bSoulInsert)
				{
					SendSoulTransition();
				}
				else
				{
					SendSoulExtract();
				}
			}
			else
			{
				fPercent = fGapTime * 100 / kTime.fTotalTime;
			}

			float const fInsurancePer = fPercent * 100 / INSURANCE_PROGRESS_MAIN_MAX;
			float const fRateUpPer = fPercent * 100 / RATEUP_PROGRESS_MAIN_MAX;

			SetProgress(fPercent, true, PROGRESS_MAIN);
			if(bSoulInsert)
			{
				if(lwUseInsuranceItem())
				{
					SetProgress(fInsurancePer, false, PROGRESS_INSURANCE);
				}
				if( 0 < lwGetRateUpItemCount() )
				{
					SetProgress(fRateUpPer, false, PROGRESS_RATEUP);
				}
			}
		}
	}

	void lwSoulTrans_IconAni(char const *pszText , bool const bOn)
	{
		std::wstring const kTextWnd = UNI(pszText);
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pIconAni = pMainFrame->GetControl(kTextWnd);
			if( pIconAni )
			{
				pIconAni->Visible(bOn);
			}
		}
	}

	//추출아이템/영력보관구슬 등록/해제
	bool lwAutoInsertItem()
	{
		XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
		if(!pCursor)
		{
			return false;
		}
		const SIconInfo &kCursorIcon = pCursor->IconInfo();//커서에 묶인것.
		if( 0 == kCursorIcon.iIconGroup )
		{
			return false;
		}

		SItemPos kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);//타겟 아이템
		PgBase_Item kItem;
		GetMyItem(kCasterPos, kItem);
		if( kItem.IsEmpty() )
		{//에러 - 아이템을 소유하고 있지 않습니다.
			return false;
		}
		AutoInsertItem(kCasterPos, kItem);
		return false;
	}

	//보험스크롤/확률증가 아이템 설정
	bool lwSetAdditionItem()
	{
		SItemPos kDestItemPos;
		GetSelectItemPos(DEST_ITEM, kDestItemPos);
		PgBase_Item kDestItem;
		GetMyItem(kDestItemPos, kDestItem);
		if(kDestItem.IsEmpty())
		{
			SetSelectItem(INSURANCE_ITEM, PgBase_Item::NullData());
			SetSelectItem(RATEUP_ITEM, PgBase_Item::NullData());
			SetInsuranceBtn(false, false);
			SetRateUpBtn(false, false);
			return false;
		}
		if( !IsSoulInsertUI() )
		{
			return false;
		}
		
		SItemPos kResourceItemPos;
		GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
		PgBase_Item kResourceItem;
		GetMyItem(kResourceItemPos, kResourceItem);
		int const iGrade = GetItemGrade(kResourceItem);

		CONT_DEF_JOBSKILL_SOUL_TRANSITION::value_type kTransitionInfo;
		SoulTransitionUtil::GetSoulTransitionInfo(kDestItem.ItemNo(), iGrade, kTransitionInfo);

		PgBase_Item kInsuranceItem;
		GetSelectItem(INSURANCE_ITEM, kInsuranceItem);
		if( kInsuranceItem.ItemNo() != kTransitionInfo.iInsuranceitemNo
			|| !IsHaveItem( kTransitionInfo.iInsuranceitemNo ) )
		{
			PgBase_Item kNewItem;
			kNewItem.ItemNo(kTransitionInfo.iInsuranceitemNo);
			SetSelectItem(INSURANCE_ITEM, kNewItem);
		}
		PgBase_Item kRateUpItem;
		GetSelectItem(RATEUP_ITEM, kRateUpItem);
		if( kRateUpItem.ItemNo() != kTransitionInfo.iProbabilityUpItemNo
			|| (kRateUpItem.Count() > GetHaveItemCount(kTransitionInfo.iProbabilityUpItemNo)) )
		{
			PgBase_Item kNewItem;
			kNewItem.ItemNo(kTransitionInfo.iProbabilityUpItemNo);
			SetSelectItem(RATEUP_ITEM, kNewItem);
		}

		SetRateText();
		bool const bUseInsurance = lwUseInsuranceItem();
		SetInsuranceBtn(bUseInsurance, !bUseInsurance);

		if(0 == kRateUpItem.Count())
		{
			SetRateUpBtn(false, true);
		}
		else if( kTransitionInfo.iProbabilityUpItemCount == kRateUpItem.Count())
		{
			SetRateUpBtn(true, false);
		}
		else
		{
			SetRateUpBtn(true, true);
		}
		return true;
	}
	//보험스크롤 등록/해제
	bool lwAddInsuranceItem()
	{
		bool bRet = false;
		PgBase_Item kInsuranceItem;
		GetSelectItem(INSURANCE_ITEM, kInsuranceItem);
		if( 0 == kInsuranceItem.ItemNo() )
		{
			bRet = false;
		}
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			bRet = false;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		SItemPos kItemPos;
		if( S_OK == pkInv->GetFirstItem(kInsuranceItem.ItemNo(), kItemPos) )
		{
			if( 0 == kInsuranceItem.Count() )
			{
				kInsuranceItem.Count(1);
				SetSelectItem(INSURANCE_ITEM, kInsuranceItem);
				bRet = true;
			}
			else
			{
				bRet = false;
			}
		}
		else
		{//에러 메세지 "사용 가능한 보험 아이템이 없습니다."
			CallSoulTransMessage(ESTM_NOT_HAVE_INSURANCE);
			bRet = false;
		}
		SetInsuranceBtn(bRet, !bRet);
		return bRet;
	}
	bool lwDelInsuranceItem()
	{
		PgBase_Item kInsuranceItem;
		GetSelectItem(INSURANCE_ITEM, kInsuranceItem);
		if(0 != kInsuranceItem.ItemNo() )
		{
			kInsuranceItem.Count(0);
			SetSelectItem(INSURANCE_ITEM, kInsuranceItem);
			SetInsuranceBtn(false, true);
		}
		return true;
	}
	bool lwUseInsuranceItem()	//보험스크롤 사용했는지 확인
	{
		PgBase_Item kInsuranceItem;
		if( GetSelectItem(INSURANCE_ITEM, kInsuranceItem) )
		{
			return (0 < kInsuranceItem.Count());
		}
		return false;
	}

	void SetInsuranceBtn(bool const bDel, bool const bAdd)	//보험아이템 +/- 버튼 설정
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(pMain)
		{
			XUI::CXUI_Wnd* pFrame = pMain->GetControl(MAIN_FRAME);
			if(pFrame)
			{
				XUI::CXUI_Wnd* 	DelItem = pFrame->GetControl(DEL_INSURANCE_ITEM);
				XUI::CXUI_Wnd* 	AddItem = pFrame->GetControl(ADD_INSURANCE_ITEM);
				if( DelItem && AddItem)
				{
					DelItem->Visible(bDel);
					AddItem->Visible(bAdd);
				}
			}
		}
	}

	//확률증가아이템 추가/감소
	bool lwAddRateUpItem()
	{		
		SItemPos kDestItemPos;
		GetSelectItemPos(DEST_ITEM, kDestItemPos);
		PgBase_Item kDestItem;
		if( !GetMyItem(kDestItemPos, kDestItem) )
		{
			CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
			return false;
		}
		SItemPos kResourceItemPos;
		GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
		PgBase_Item kResourceItem;
		if( !GetMyItem(kResourceItemPos, kResourceItem) )
		{
			CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
			return false;
		}
		int const iGrade = GetItemGrade(kResourceItem);
		CONT_DEF_JOBSKILL_SOUL_TRANSITION::value_type kTransitionInfo;
		if( !SoulTransitionUtil::GetSoulTransitionInfo(kDestItem.ItemNo(), iGrade, kTransitionInfo) )
		{
			CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
			return false;
		}
		PgBase_Item kRateUpItem;
		GetSelectItem(RATEUP_ITEM, kRateUpItem);
		if( 0 == kRateUpItem.ItemNo() )
		{
			CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
			return false;
		}
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		SItemPos kItemPos;
		
		int const iItemCount = kRateUpItem.Count();
		int iHaveItemCount = 0;
		if( S_OK == pkInv->GetFirstItem(kRateUpItem.ItemNo(), kItemPos) )
		{
			iHaveItemCount = pkInv->GetInvTotalCount(kRateUpItem.ItemNo());
		}
		else
		{//에러 메세지 등록 - 니가 가진게 없구나
			CallSoulTransMessage(ESTM_NOT_HAVE_RATEUP);
			return false;
		}
		if( iItemCount < kTransitionInfo.iProbabilityUpItemCount)
		{
			if( iItemCount < iHaveItemCount )
			{
				kRateUpItem.Count(iItemCount+1);
				SetSelectItem(RATEUP_ITEM, kRateUpItem);
			}
			else
			{//에러 메세지 "가지고 계신 최대 갯수만큼 등록되었습니다.
				CallSoulTransMessage(ESTM_NOT_ENOUGH_RATEUP);
				return false;
			}
		}
		else if(iItemCount >= kTransitionInfo.iProbabilityUpItemCount)
		{//에러 메세지 등록 - 니가 가진게 없구나
			CallSoulTransMessage(ESTM_SELECT_RATEUP_MAX);
			return false;
		}
		if(kRateUpItem.Count() == kTransitionInfo.iProbabilityUpItemCount)
		{
			SetRateUpBtn(true, false);
		}
		else
		{
			SetRateUpBtn(true, true);
		}
		lwSetSoulTransText();
		return true;
	}
	bool lwDelRateUpItem()
	{
		PgBase_Item kRateUpItem;
		GetSelectItem(RATEUP_ITEM, kRateUpItem);
		if(0 != kRateUpItem.ItemNo() )
		{
			int const iCount = kRateUpItem.Count();
			if( 0 < iCount)
			{
				kRateUpItem.Count(iCount-1);
				SetSelectItem(RATEUP_ITEM, kRateUpItem);
				lwSetSoulTransText();
			}
		}
		if(0 == kRateUpItem.Count())
		{
			SetRateUpBtn(false, true);
		}
		else
		{
			SetRateUpBtn(true, true);
		}
		return true;
	}
	int lwGetRateUpItemCount() //확률증가아이템 갯수 리턴
	{
		PgBase_Item kRateUpItem;
		if( false == GetSelectItem(RATEUP_ITEM, kRateUpItem) )
		{
			return 0;
		}
		return kRateUpItem.Count();
	}
	void SetRateUpBtn(bool const bDel, bool const bAdd)	//확률증가아이템 +/- 버튼 설정
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pFrame = pMain->GetControl(MAIN_FRAME);
		if(!pFrame)
		{
			return;
		}
		XUI::CXUI_Wnd* pAddWnd = pFrame->GetControl(ADD_RATEUP_ITEM);
		XUI::CXUI_Wnd* pDelWnd = pFrame->GetControl(DEL_RATEUP_ITEM);
		if(!pAddWnd || !pDelWnd)
		{
			return;
		}
		pAddWnd->Visible(bAdd);
		pDelWnd->Visible(bDel);
	}

	bool lwSetExtractitem()
	{
		SItemPos kResourceItemPos;
		GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
		PgBase_Item kResourceItem;
		GetMyItem(kResourceItemPos, kResourceItem);

		return SetExtractitem(kResourceItem);
	}
	bool SetExtractitem(PgBase_Item const &rkResourceItem)	//추출기 아이템 설정
	{
		CONT_DEF_JOBSKILL_SOUL_EXTRACT::value_type kExtractInfo;
		if( false == SoulTransitionUtil::GetSoulExtractInfo(rkResourceItem.ItemNo(), kExtractInfo) )
		{
			return false;
		}
		PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgInventory *pInv = pPlayer->GetInven();
		if(!pInv)
		{
			return false;
		}
		SItemPos kExtractItemPos;
		PgBase_Item kExtractItem;
		kExtractItem.ItemNo(kExtractInfo.iExtractItemNo);
		bool const bHaveItem = ( S_OK == pInv->GetFirstItem(kExtractInfo.iExtractItemNo, kExtractItemPos) );
		if(bHaveItem)
		{//아이템 있으면 아이템 셋팅, 없으면 아이템 번호만 가진 껍데기 셋팅(아이콘회색처리용)
			GetMyItem(kExtractItemPos, kExtractItem);
			kExtractItem.Count(1);	//숫자 안나오도록 =_+
		}
		SetSelectItem(DEST_ITEM, kExtractItem);
		return true;
	}
	
	bool SetSelectItem(std::wstring const &rkTextWnd, PgBase_Item const &rkItem)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pIconWnd = pMainFrame->GetControl(rkTextWnd);
			if( pIconWnd )
			{
				XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(L"Icon");
				if( pIcon )
				{
					if( 0 == rkItem.ItemNo() )
					{
						pIcon->ClearCustomData();
						return false;
					}
					else
					{
						lwUIWnd kIconWnd = lwUIWnd(pIcon);
						kIconWnd.SetCustomDataAsSItem(rkItem);
					}
				}
			}
		}
		return true;
	}
	bool GetSelectItem(std::wstring const &rkTextWnd, PgBase_Item &rkItemOut)	//아이템 가져오기
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pIconWnd = pMainFrame->GetControl(rkTextWnd);
			if( pIconWnd )
			{
				XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(L"Icon");
				if( pIcon )
				{
					lwUIWnd kIconWnd = lwUIWnd(pIcon);
					rkItemOut = kIconWnd.GetCustomDataAsSItem();
				}
			}
		}
		return (0 != rkItemOut.ItemNo());
	}

	bool SetSelectItemPos(std::wstring const &rkTextWnd, SItemPos const &kItemPos)	//아이템 셋팅하기
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pIconWnd = pMainFrame->GetControl(rkTextWnd);
			if( pIconWnd )
			{
				XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(L"Icon");
				if( pIcon )
				{
					pIcon->SetCustomData(&kItemPos, sizeof(kItemPos));
					return true;
				}
			}
		}
		return false;
	}

	bool GetSelectItemPos(std::wstring const &rkTextWnd, SItemPos &kItemPos)	//아이템 가져오기
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pIconWnd = pMainFrame->GetControl(rkTextWnd);
			if( pIconWnd )
			{
				XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(L"Icon");
				if( pIcon )
				{
					pIcon->GetCustomData(&kItemPos, sizeof(kItemPos));
					PgBase_Item kItem;
					return (GetMyItem(kItemPos, kItem));
				}
			}
		}
		return false;
	}

	bool GetMyItem(SItemPos const &rkItemPos, PgBase_Item &kTargetItem)//내 인벤에 있는 아이템가져오기
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgInventory* pInv = pPlayer->GetInven();
		if(!pInv)
		{
			return false;
		}
		pInv->GetItem(rkItemPos, kTargetItem);

		return !kTargetItem.IsEmpty();
	}
	bool IsHaveItem(int const iItemNo)//내 인벤에 아이템이 있는지
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(pPlayer)
		{
			PgInventory* pInv = pPlayer->GetInven();
			if(pInv)
			{
				SItemPos kItemPos;
				return (S_OK == pInv->GetFirstItem(iItemNo, kItemPos));
			}
		}
		return false;
	}
int GetHaveItemCount(int const iItemNo)//내 인벤에 아이템이 몇개 있는지
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(pPlayer)
	{
		PgInventory* pInv = pPlayer->GetInven();
		if(pInv)
		{
			return pInv->GetTotalCount(iItemNo);
		}
	}
	return 0;
}
	void AutoInsertItem(SItemPos const &rkItemPos, PgBase_Item const &rkItem)
	{
		if( !lwIsEndSoulTransProgress() )
		{
			return;
		}
		bool bSoulInsert = IsSoulInsertUI();
		if(bSoulInsert)
		{//영력삽입	
			if( SoulTransitionUtil::IsSoulTransitionItem(rkItem.ItemNo()) )
			{//삽입구슬
				if(SetSelectItemPos(RESOURCE_ITEM, rkItemPos))
				{
					SetSelectItemPos(DEST_ITEM, SItemPos::NullData());
					lwSoulTrans_IconAni("FRM_RESOURCE_ANI", true);
					PgBase_Item kTransitionItem;
					lua_tinker::call<void,int>("PreChangeInvViewGroupTop", 1);	
				}
			}
			else
			{//삽입대상
				SItemPos kSelectItemPos;
				if( !GetSelectItemPos(RESOURCE_ITEM, kSelectItemPos) )
				{//영력삽입구슬이 먼저 선택되어야 한다.(에러메세지!)
					CallSoulTransMessage(ESTM_NOT_SELECT_TRANSITION_ITEM);
					return;
				}
				else 
				{
					PgBase_Item kTransitionItem;
					GetMyItem(kSelectItemPos, kTransitionItem);
					if( false == SoulTransitionUtil::IsTransition(kTransitionItem, rkItem) 
						|| false == SoulTransitionUtil::IsSoulInsertItem(rkItem) )
					{
						CallSoulTransMessage(ESTM_NOT_CORRECT_TRANSITION);
						return;
					}
					if(SetSelectItemPos(DEST_ITEM, rkItemPos) )
					{
						lwSoulTrans_IconAni("FRM_DEST_ANI", true);
						lwSetAdditionItem();
					}
				}
			}
			SItemPos kResourceItemPos;
			SItemPos kDestItemPos;
			if( GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos) 
			&&	GetSelectItemPos(DEST_ITEM, kDestItemPos) )
			{
				PgBase_Item kDestItem;
				PgBase_Item kResourceItem;
				GetMyItem(kResourceItemPos, kResourceItem);
				GetMyItem(kDestItemPos, kDestItem);
				if( SoulTransitionUtil::TransitionItem(kResourceItem, kDestItem) )
				{
					SetResultItem(kDestItem);
				}
			}
			else
			{
				PgBase_Item kNullItem;
				SetResultItem(kNullItem);
			}
		}
		else
		{//영력추출
			if( SoulTransitionUtil::IsSoulExtractItem(rkItem) )
			{//추출가능한 아이템이면 추출대상으로
				PgBase_Item kTransitionItem;
				if( !SoulTransitionUtil::CreateSoulTransition(rkItem, kTransitionItem) )
				{//에러 - 추출정보를 못찾았다.
					CallSoulTransMessage(ESTM_FAILED_FIND_EXTRACT_INFO);
					return;
				}

				SetExtractitem(rkItem);
				lwSoulTrans_IconAni("FRM_DEST_ANI", true);

				SetSelectItemPos(RESOURCE_ITEM, rkItemPos);
				lwSoulTrans_IconAni("FRM_RESOURCE_ANI", true);
				SetResultItem(kTransitionItem);
			}
			else
			{//추출가능한 아이템이 아니라면 에러 영력 추출 가능한 아이템이 아닙니다.
				CallSoulTransMessage(ESTM_CANT_EXTRACT_ITEM);
				return;
			}
		}
		lwSetSoulTransText();
	}

		//결과 아이템 설정
	void lwSetSoulTrans_ResultItem()
	{
		SItemPos kResourceItemPos;
		GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
		PgBase_Item kResourceItem;
		GetMyItem(kResourceItemPos, kResourceItem);
		if(!IsSoulInsertUI())
		{
			SetResultItem(kResourceItem);
			return;
		}
		else
		{
			SItemPos kDestItemPos;
			if( false == GetSelectItemPos(DEST_ITEM, kDestItemPos) )
			{
				PgBase_Item kNullItem;
				SetResultItem(kNullItem);
				return;
			}
			PgBase_Item kDestItem;
			GetMyItem(kDestItemPos, kDestItem);
			if( SoulTransitionUtil::TransitionItem(kResourceItem, kDestItem) )
			{
				SetResultItem(kDestItem);
			}
		}
	}
	void SetResultItem(PgBase_Item &rkItem)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pMainFrame = pMain->GetControl(MAIN_FRAME);
		if(pMainFrame)
		{
			XUI::CXUI_Wnd* pResultAni = pMainFrame->GetControl(RESULT_ANI);
			XUI::CXUI_Wnd* pResultItem = pMainFrame->GetControl(RESULT_ITEM);
			if( !pResultAni
			||  !pResultItem)
			{
				return;
			}
			XUI::CXUI_Wnd* pIcon = pResultItem->GetControl(L"Icon");
			if(!pIcon)
			{
				return;
			}
			bool bHaveItem = !rkItem.IsEmpty();
			pResultAni->Visible(bHaveItem);
			if(bHaveItem)
			{
				rkItem.Count(1);
				lwUIWnd kIconWnd = lwUIWnd(pIcon);
				kIconWnd.SetCustomDataAsSItem(rkItem);
			}
			else
			{
				pIcon->ClearCustomData();
			}
		}
	}

	//현재 열려있는 UI 타입이 소울 삽입인가?
	bool IsSoulInsertUI()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return false;
		}
		bool bSoulInsert = false;
		pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert));
		return bSoulInsert;
	}
	
	//영력추출기 설정(제한레벨/삽입가능부위/필요소울/필요골드/성공확률(현재,최대))
	void lwSetSoulTransText()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(TRANSFER_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pFrame = pMain->GetControl(MAIN_FRAME);
		if(!pFrame)
		{
			return;
		}
		bool bSoulInsert = false;
		pMain->GetCustomData(&bSoulInsert, sizeof(bSoulInsert) );
		if( !bSoulInsert )
		{
			SetNeedSoulText(pFrame, 0);
			SetNeedGoldText(pFrame, 0);
		}
		else
		{
			SItemPos kResourceItemPos;
			GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
			PgBase_Item kResourceItem;
			GetMyItem(kResourceItemPos, kResourceItem);
			int const iGrade = GetItemGrade(kResourceItem);
			
			SItemPos kDestItemPos;
			GetSelectItemPos(DEST_ITEM, kDestItemPos);
			PgBase_Item kDestItem;
			GetMyItem(kDestItemPos, kDestItem);

			SetLimitText(pFrame, bSoulInsert, kResourceItem);
			SetRateText(pFrame, bSoulInsert, kResourceItem, kDestItem);
			CONT_DEF_JOBSKILL_SOUL_TRANSITION::value_type kTransitionInfo;
			SoulTransitionUtil::GetSoulTransitionInfo(kDestItem.ItemNo(), iGrade, kTransitionInfo);
			SetNeedSoulText(pFrame, kTransitionInfo.iSoulItemCount);
			SetNeedGoldText(pFrame, kTransitionInfo.iNeedMoney);
		}
	}

	//메세지 출력
	void CallSoulTransMessage(ESoulTransMsg const eType, int const iItemNo)
	{
		switch(eType)
		{
		case ESTM_SERVER_ERR:
			{//서버 에러 (클라와 서버 정보가 맞지 않는 상황)
				if(IsSoulInsertUI())
				{
					::Notice_Show( TTW(799885), EL_Warning );
				}
				else
				{
					::Notice_Show( TTW(799886), EL_Warning );
				}
			}break;
		case ESTM_NOT_SELECT_EXTRACT_ITEM:
			{//영력추출대상OR영력삽입구슬 아이템을 선택하지 않았다.
				::Notice_Show( TTW(799887), EL_Warning );
			}break;
		case ESTM_NOT_SELECT_TRANSITION_ITEM:
			{//영력보관구슬을 선택하지 않았다.
				::Notice_Show( TTW(799888), EL_Warning );
			}break;
		case ESTM_NOT_HAVE_EXTRACT_ITEM:
			{//영력추출기를 가지고 있지 않다
				::Notice_Show( TTW(799889), EL_Warning );
			}break;
		case ESTM_NOT_SELECT_DEST_ITEM:
			{//영력삽입대상 아이템을 선택하지 않았다.
				::Notice_Show( TTW(799890), EL_Warning );
			}break;
		case ESTM_CANT_EXTRACT_ITEM:
			{//영력추출을 할 수 없는 아이템이다.
				::Notice_Show( TTW(799891), EL_Warning );
			}break;
		case ESTM_NOT_ENOUGH_MONEY:
			{//소지금 부족 
				::Notice_Show( TTW(799892), EL_Warning );
			}break;
		case ESTM_NOT_ENOUGH_SOUL:
			{//소울 부족
				::Notice_Show( TTW(799893), EL_Warning );
			}break;
		case ESTM_SUCCESS_EXTRACT:
			{//추출 성공
				BM::vstring kResult(TTW(799894));
				//아이템 def 받아와서 아이템명 넣어주자
				wchar_t const* pItemName;
				GetItemName(iItemNo, pItemName);
				kResult.Replace(L"#ITEM#", pItemName);
				::Notice_Show( kResult, EL_Normal );
			}break;
		case ESTM_SUCCESS_TRANSITION:
			{//삽입 성공
				BM::vstring kResult(TTW(799895));
				//아이템 def 받아와서 아이템명 넣어주자
				wchar_t const* pItemName;
				GetItemName(iItemNo, pItemName);
				kResult.Replace(L"#ITEM#", pItemName);
				::Notice_Show( kResult, EL_Normal );
			}break;
		case ESTM_FAILED_FIND_EXTRACT_INFO:
			{//알맞은 추출 정보를 받아오지 못했다.
				::Notice_Show( TTW(799896), EL_Warning );
			}break;
		case ESTM_FAILED_FIND_TRANSITION_INFO:
			{//알맞은 삽입 정보를 받아오지 못했다.
				::Notice_Show( TTW(799897), EL_Warning );
			}break;
		case ESTM_NOT_HAVE_INSURANCE:
			{//보험 아이템 없음 
				::Notice_Show( TTW(799898), EL_Warning );
			}break;
		case ESTM_NOT_ENOUGH_RATEUP:
			{//확률증가 아이템 갯수 부족
				::Notice_Show( TTW(799899), EL_Warning );
			}break;
		case ESTM_FAILED_TRANSITION_INSURANCE:
			{//삽입 실패 - 보험 사용
				::Notice_Show( TTW(799900), EL_Warning );
			}break;
		case ESTM_FAILED_TRANSITION_NOT_INSURANCE:
			{//삽입 실패 - 보험 미사용
				::Notice_Show( TTW(799901), EL_Warning );
				//실패했을 때 보험 미사용이면 싹 지워주자.
				ResetUI();
			}break;
		case ESTM_SELECT_RATEUP_MAX:
			{//최대확률까지 올렸다.
				::Notice_Show( TTW(799902), EL_Warning );
			}break;
		case ESTM_FAIL_EXTRACT:
			{//추출 실패
				::Notice_Show( TTW(799885), EL_Warning );
			}break;
		case ESTM_FAIL_TRANSITION:
			{//삽입 실패
				::Notice_Show( TTW(799886), EL_Warning );
			}break;
		case ESTM_NOT_CORRECT_TRANSITION:
			{//삽입구슬에 맞지 않는 장비를 등록하려고 했다.
				::Notice_Show( TTW(799904), EL_Warning );
			}break;
		case ESTM_INVENTORY_FULL:
			{//인벤이 가득 찼다.
				::Notice_Show( TTW(799905), EL_Warning );
			}break;
		case ESTM_NOT_HAVE_RATEUP:
			{//확률증가 아이템 없다
				::Notice_Show( TTW(799875), EL_Warning );
			}
		}
	}
	
	void SendSoulExtract()
	{
		//추출아이템
		SItemPos kResourceItemPos;
		if( GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos) )
		{
			BM::Stream kPacket(PT_C_M_REQ_SOULTRANSFER_EXTRACT);
			kPacket.Push(kResourceItemPos);
			NETWORK_SEND(kPacket);
		}
	}
	void SendSoulTransition()
	{
		//삽입구슬아이템
		SItemPos kResourceItemPos;
		//삽입대상아이템
		SItemPos kDestItemPos;

		if( GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos)
		 && GetSelectItemPos(DEST_ITEM, kDestItemPos) )
		{	//보험아이템(쓴다/안쓴다)
			bool const bUseInsuranceItem = lwUseInsuranceItem();
			//확률증가아이템(갯수)
			int const iRateUpItemCount = lwGetRateUpItemCount();

			BM::Stream kPacket(PT_C_M_REQ_SOULTRANSFER_TRANSITION);
			kPacket.Push(kResourceItemPos);
			kPacket.Push(kDestItemPos);
			kPacket.Push(bUseInsuranceItem);
			kPacket.Push(iRateUpItemCount);
			NETWORK_SEND(kPacket);
		}
	}

	bool IsWearable(PgBase_Item const &rkItem, bool& rbGray)
	{
		bool bRet = false;
		bRet = IsAddResourceItem(rkItem, rbGray);
		if(rbGray && IsSoulInsertUI() )
		{
			bRet = IsAddDestItem(rkItem, rbGray);
		}
		return bRet;
	}

	bool IsAddResourceItem(PgBase_Item const &rkItem, bool& rbGray)
	{
		PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgInventory *pInv = pPlayer->GetInven();
		if(!pInv)
		{
			return false;
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(TRANSFER_MAIN);
		if( !pkWnd )
		{ 
			return false; 
		}
		rbGray = false;
		if( IsSoulInsertUI() )
		{//삽입이면, 삽입구슬아이템인지 확인
			rbGray = !SoulTransitionUtil::IsSoulTransitionItem(rkItem.ItemNo());
		}
		else
		{//추출이면, 추출가능아이템인지 확인
			rbGray = !SoulTransitionUtil::IsSoulExtractItem(rkItem);
		}
		return true;
	}

	bool IsAddDestItem(PgBase_Item const &rkItem, bool& rbGray)
	{
		PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgInventory *pInv = pPlayer->GetInven();
		if(!pInv)
		{
			return false;
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(TRANSFER_MAIN);
		if( !pkWnd )
		{ 
			return false; 
		}
		rbGray = true;
		if( IsSoulInsertUI() )
		{//삽입이면 삽입가능 아이템인지 확인
			if( !SoulTransitionUtil::IsSoulInsertItem(rkItem) )
			{
				rbGray = true;
			}
			else
			{
				SItemPos kResourceItemPos;
				GetSelectItemPos(RESOURCE_ITEM, kResourceItemPos);
				PgBase_Item kResourceItem;
				if(GetMyItem(kResourceItemPos, kResourceItem))
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pResourceItemDef = kItemDefMgr.GetDef(kResourceItem.ItemNo());
					CItemDef const* pItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
					if(pResourceItemDef && pItemDef)
					{
						rbGray = ( pResourceItemDef->EquipPos() != pItemDef->EquipPos() );
					}
				}
			}
		}
		else
		{//추출이면, 암것도 안함
			return false;
		}
		return true;
	}
}