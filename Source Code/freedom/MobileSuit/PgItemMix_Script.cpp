#include "stdafx.h"
#include "lwUI.h"
#include "ServerLib.h"
#include "PgItemMix_Script.h"
#include "PgMobileSuit.h"
#include "PgUIScene.h"
#include "PgNifMan.h"
#include "PgPilotMan.h"
#include "PgUISound.h"
#include "PgNetwork.h"
#include "lwUIQuest.h"
#include "PgMonsterCardMixer.h"

namespace lwItemMix
{
	namespace MixUIUtil
	{
		void InitMixGearAni(XUI::CXUI_Wnd* pSelf, float const fStartTime)
		{
			if( !pSelf )
			{
				return;
			}

			pSelf->SetCustomData(&fStartTime, sizeof(fStartTime));
		}

		void UpdateMixGearAni(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return;
			}

			float fEndTime = 0.0f;
			pSelf->GetCustomData(&fEndTime, sizeof(fEndTime));
			fEndTime += 0.05f;
			
			if( fEndTime > g_pkApp->GetAccumTime() )
			{
				return;
			}

			pSelf->SetCustomData(&fEndTime, sizeof(fEndTime));

			SUVInfo	kInfo = pSelf->UVInfo();
			int const TotalFrame = kInfo.U * kInfo.V;
			kInfo.Index += 1;
			if( TotalFrame < kInfo.Index )
			{
				kInfo.Index = 1;
			}
			pSelf->UVInfo(kInfo);
		}

		void ClearMixGearAni(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return;
			}

			pSelf->ClearCustomData();
			pSelf->UVUpdate(1);
		}

		void InitMixProgressing(XUI::CXUI_Wnd* pSelf, S_UPDATE_TIME const& kTimeInfo)
		{
			if( !pSelf )
			{
				return;
			}

			pSelf->SetCustomData(&kTimeInfo, sizeof(kTimeInfo));
		}

		bool UpdateMixProgressing(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return false;
			}

			S_UPDATE_TIME	kTime;
			pSelf->GetCustomData(&kTime, sizeof(kTime));

			if( kTime.fTotalTime == 0 )
			{
				return false;
			}

			float fNowTime = g_pkApp->GetAccumTime();
			if( kTime.fStartTime > fNowTime )
			{
				return false;
			}

			float fPercent = (fNowTime - kTime.fStartTime) / kTime.fTotalTime;
			if( fPercent >= 0.99f )
			{
				fPercent = 1.0f;
			}

			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pSelf);
			if( pAniBar )
			{
				pAniBar->Now(static_cast<int>(fPercent * 100));
			}
			else
			{
				POINT2 const& kWndSize = pSelf->Size();
				POINT2 kImgSize = pSelf->ImgSize();
				kImgSize.y = kWndSize.y * fPercent;
				pSelf->ImgSize(kImgSize);
			}

			return (1.0f == fPercent);
		}

		void ClearMixProgressing(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return;
			}

			pSelf->ClearCustomData();
			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pSelf);
			if( pAniBar )
			{
				pAniBar->Now(0);
			}
			else
			{
				POINT2 kImgSize = pSelf->ImgSize();
				kImgSize.y = 0;
				pSelf->ImgSize(kImgSize);
			}
		}

		void SetMixInsurance(XUI::CXUI_Wnd* pSelf, S_MIX_ITEM_INFO const& kItemInfo, int const DEFAULT_INS_ITEMNO)
		{
			if( !pSelf )
			{
				return;
			}

			bool bUseItem = (0 != kItemInfo.iHave);

			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp )
			{
				pTemp->SetCustomData(&DEFAULT_INS_ITEMNO, sizeof(DEFAULT_INS_ITEMNO));
			}

			pTemp = pSelf->GetControl(L"ICN_RATEUP");
			if( pTemp )
			{
				if( bUseItem )
				{
					pTemp->SetCustomData(&kItemInfo.kItemPos, sizeof(kItemInfo.kItemPos));
				}
				else
				{
					pTemp->ClearCustomData();
				}
			}

			SetMixInsuranceSlotBtn(pSelf, (bUseItem)?(EIBS_USE_DEREG):(EIBS_USE_REG));
		}

		void SetMixInsuranceSlotBtn(XUI::CXUI_Wnd* pSelf, eInsuranceBtnState const State)
		{
			if( !pSelf )
			{
				return;
			}
			
			XUI::CXUI_Wnd* pReg = pSelf->GetControl(L"BTN_REG");
			XUI::CXUI_Wnd* pDeReg = pSelf->GetControl(L"BTN_DEREG");
			if( pReg && pDeReg )
			{
				if( EIBS_NO_USE == State )
				{
					pReg->Visible(false);
					pDeReg->Visible(false);
				}
				else
				{
					pReg->Visible((EIBS_USE_REG == State));
					pDeReg->Visible((EIBS_USE_DEREG == State));
				}
			}
		}

		void ClearMixInsuranceSlot(XUI::CXUI_Wnd* pSelf, int const DEFAULT_INS_ITEMNO)
		{
			if( !pSelf )
			{
				return;
			}


			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp )
			{
				pTemp->SetCustomData(&DEFAULT_INS_ITEMNO, sizeof(DEFAULT_INS_ITEMNO));
			}

			pTemp = pSelf->GetControl(L"ICN_RATEUP");
			if( pTemp )
			{
				pTemp->ClearCustomData();
			}

			SetMixInsuranceSlotBtn(pSelf, EIBS_NO_USE);
		}

		void SetMixMerterialSlot(XUI::CXUI_Wnd* pSelf, S_MIX_ITEM_INFO const& kItemInfo)
		{
			if( !pSelf )
			{
				return;
			}

			XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pSelf->GetControl(L"ICN_MATERIAL"));
			if( pkIcon )
			{
				switch( pkIcon->IconInfo().iIconGroup )
				{
				case KUIG_COMMON_ITEMNO:
					{
						pkIcon->SetCustomData( &kItemInfo.kItemNo, sizeof(kItemInfo.kItemNo) );
					}
				case KUIG_COMMON_INV_POS:
					{
						pkIcon->SetCustomData( &kItemInfo.kItemPos, sizeof(kItemInfo.kItemPos) );
					}break;
				default:
					{
						pkIcon->ClearCustomData();
					}return;
				}
			}

			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_MATERIAL_BAG");
			if( pTemp )
			{
				pTemp->SetCustomData( &kItemInfo.kItemNo, sizeof(kItemInfo.kItemNo) );
			}

			pTemp = pSelf->GetControl(L"FRM_ITEM_NAME");
			if( pTemp )
			{
				std::wstring kItemName = L"";
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItemInfo.kItemNo);
				if(pDef)
				{
					std::wstring const* pkItemName = NULL;
					if( GetDefString(pDef->NameNo(), pkItemName) )
					{
						kItemName = *pkItemName;
					}
				}
				int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
				Quest::SetCutedTextLimitLength(pTemp, kItemName, WSTR_UI_REGULATIONSTR, iWidth);
			}
		}

		void ClearMixMarterialSlot(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return;
			}

			XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pSelf->GetControl(L"ICN_MATERIAL"));
			if( pkIcon )
			{
				pkIcon->ClearCustomData();
			}

			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"SFRM_MATERIAL_BG");
			if( pTemp )
			{
				pTemp->ClearCustomData();
			}

			pTemp = pSelf->GetControl(L"FRM_ITEM_NAME");
			if( pTemp )
			{
				pTemp->Text(L"");
			}
		}

		void SetMixResultSlot(XUI::CXUI_Wnd* pSelf, DWORD const kItemNo)
		{
			if( !pSelf )
			{
				return;
			}

			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_SRC");
			if( pTemp )
			{
				pTemp->SetCustomData(&kItemNo, sizeof(kItemNo));
			}

			pTemp = pSelf->GetControl(L"FRM_ITEM_NAME");
			if( pTemp )
			{
				std::wstring kItemName = L"";
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItemNo);
				if(pDef)
				{
					std::wstring const* pkItemName = NULL;
					if( GetDefString(pDef->NameNo(), pkItemName) )
					{
						kItemName = *pkItemName;
					}
				}
				int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
				Quest::SetCutedTextLimitLength(pTemp, kItemName, WSTR_UI_REGULATIONSTR, iWidth);
			}
		}

		void ClearMixResultSlot(XUI::CXUI_Wnd* pSelf)
		{
			if( !pSelf )
			{
				return;
			}

			XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_SRC");
			if( pTemp )
			{
				pTemp->ClearCustomData();
			}

			pTemp = pSelf->GetControl(L"FRM_ITEM_NAME");
			if( pTemp )
			{
				pTemp->Text(L"");
			}
		}
	}

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "UpdateEqItemMixCreatorUI", UpdateEqItemMixCreatorUI);
		def(pkState, "StartEpItemMixing", StartEpItemMixing);
		def(pkState, "StopEpItemMixing", StopEpItemMixing);
		def(pkState, "UpdateEpItemMixInsuranc", UpdateEpItemMixInsuranc);
		def(pkState, "UpdateEpItemMixProbability", UpdateEpItemMixProbability);
		def(pkState, "CallEqItemMixToolTip", CallEqItemMixToolTip);
		

		def(pkState, "CallCostumeMixNpcMenu", CallCostumeMixNpcMenu);
		def(pkState, "CallCostumeMixUI", CallCostumeMixUI);
		def(pkState, "StartCostumeMixing", StartCostumeMixing);
		def(pkState, "UpdateCostumeMixUI", UpdateCostumeMixUI);
		def(pkState, "OnDragDropCostumeMixItem", OnDragDropCostumeMixItem);

		def(pkState, "DeleteCommonMixerInfo", DeleteCommonMixerInfo);
		def(pkState, "UpdateCommonTypeMixer", UpdateCommonTypeMixer);
		def(pkState, "StartCommonTypeMixer", StartCommonTypeMixer);

		def(pkState, "OnCallMonsterCardMixerType2", lwOnCallMonsterCardMixerType2);
		def(pkState, "OnUpdateMonsterCardMixerType2", lwOnUpdateMonsterCardMixerType2);
		def(pkState, "OnClickMonsterCardMixerType2Start", lwOnClickMonsterCardMixerType2Start);
		def(pkState, "SetMonsterCardMixerType2Ins", lwSetMonsterCardMixerType2Ins);
		def(pkState, "SetDefaultInsItemNo", lwSetDefaultInsItemNo);
		def(pkState, "OnCloseMonsterCardMixerType2", lwOnCloseMonsterCardMixerType2);
	}

	void StopEpItemMixing(lwUIWnd kParent)
	{
		g_kEqItemMixCreator.bMixing(false);
		g_kEqItemMixCreator.ClearProbAbility();
		lwStopSoundByID("Item_Mix_Mixing");
	}

	void StartEpItemMixing(lwUIWnd kParent)
	{
		if( g_kEqItemMixCreator.GetRecipe().IsEmpty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50707, true);
			return;
		}

		if( g_kEqItemMixCreator.LackMaterial() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50708, true);
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( g_kEqItemMixCreator.Cost() > pkPlayer->GetAbil64(AT_MONEY) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403078, true);
			return;
		}

		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		float const fNowTime = g_pkApp->GetAccumTime();

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"SFRM_BG");
		if( pTemp )
		{
			float fStartTime = g_pkApp->GetAccumTime();
			XUI::CXUI_Wnd* pGear = pTemp->GetControl(L"IMG_GEAR_L");
			if( pGear )
			{
				pGear->SetCustomData(&fStartTime, sizeof(fStartTime));
			}
			pGear = pTemp->GetControl(L"IMG_GEAR_R");
			if( pGear )
			{
				pGear->SetCustomData(&fStartTime, sizeof(fStartTime));
			}
		}

		pTemp = pParent->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			S_UPDATE_TIME kTime;
			kTime.fStartTime = fNowTime;
			kTime.fTotalTime = 2.0f;
			pTemp->SetCustomData(&kTime, sizeof(kTime));
		}

		XUI::CXUI_Builder* pBuild1 = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT1"));
		XUI::CXUI_Builder* pBuild2 = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT2"));
		if( pBuild1 && pBuild2 )
		{
			int const iMaxSlot = pBuild1->CountX() * pBuild1->CountY() + pBuild2->CountX() * pBuild2->CountY();
			float fStartTime = fNowTime + 0.2f;
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pProgBar = pSlot->GetControl(L"IMG_SUBPROG");
					if( pProgBar )
					{
						S_UPDATE_TIME kTime;
						kTime.fStartTime = fStartTime + (i % 4) * 0.32f;
						kTime.fTotalTime = 0.34f;
						pProgBar->SetCustomData(&kTime, sizeof(kTime));
					}
				}
			}
		}

		pTemp = pParent->GetControl(L"BTN_MIX");
		if( pTemp )
		{
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(true);
			}
		}

		g_kEqItemMixCreator.bMixing(true);
		lwPlaySoundByID("Item_Mix_Mixing");
	}

	void UpdateEqItemMixCreatorUI(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( !g_kEqItemMixCreator.LackMaterial() )
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				if( g_kEqItemMixCreator.Cost() <= pkPlayer->GetAbil64(AT_MONEY) )
				{
					XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"IMG_MIX");
					if( pTemp )
					{
						float fEndTime = 0.0f;
						pTemp->GetCustomData(&fEndTime, sizeof(fEndTime));
						fEndTime += 0.05f;
						if( fEndTime < g_pkApp->GetAccumTime() )
						{
							pTemp->SetCustomData(&fEndTime, sizeof(fEndTime));
							pTemp->UVUpdate((pTemp->UVInfo().Index == 1)?(2):(1));
						}
					}
				}
			}
		}

		if( !g_kEqItemMixCreator.bMixing() )
		{
			return;
		}

		XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"SFRM_BG");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pGear = pTemp->GetControl(L"IMG_GEAR_L");
			if( pGear )
			{
				UpdateEpItemMixGearAni(pGear);
			}
			pGear = pTemp->GetControl(L"IMG_GEAR_R");
			if( pGear )
			{
				UpdateEpItemMixGearAni(pGear);
			}
		}

		XUI::CXUI_Builder* pBuild1 = dynamic_cast<XUI::CXUI_Builder*>(pSelf->GetControl(L"BLD_MATERIAL_SLOT1"));
		XUI::CXUI_Builder* pBuild2 = dynamic_cast<XUI::CXUI_Builder*>(pSelf->GetControl(L"BLD_MATERIAL_SLOT2"));
		if( pBuild1 && pBuild2 )
		{
			int const iMaxSlot = pBuild1->CountX() * pBuild1->CountY() + pBuild2->CountX() * pBuild2->CountY();
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pSelf->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pProgBar = pSlot->GetControl(L"IMG_SUBPROG");
					if( pProgBar )
					{
						UpdateEpItemMixProgressing(pProgBar);
					}
				}
			}
		}

		pTemp = pSelf->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			UpdateEpItemMixProgressing(pTemp);
			if( pTemp->ImgSize().y == pTemp->Size().y )
			{
				g_kEqItemMixCreator.RequestMix();
				ReSetEpItemMixProgressing(pSelf);
				CallEqItemMixCreatorUI();
				g_kEqItemMixCreator.bMixing(false);
			}
		}
	}

	void UpdateEpItemMixInsuranc(lwUIWnd kSelf)
	{
		if( g_kEqItemMixCreator.bMixing() )
		{
			lwAddWarnDataTT(50714);
			return;
		}

		XUI::CXUI_Button* pSelf = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pIcon = pParent->GetControl(L"ICN_RATEUP");
		if( !pIcon )
		{
			return;
		}

		std::wstring kOtherBtnName( L"BTN_REG" );
		bool const bIsReg = (pSelf->ID() == kOtherBtnName);
		if( bIsReg )
		{
			kOtherBtnName = L"BTN_DEREG";
		}
		XUI::CXUI_Wnd* pOther = pParent->GetControl(kOtherBtnName);
		if( !pOther )
		{
			return;
		}

		if( bIsReg )
		{
			if( g_kEqItemMixCreator.SetInsurance(true) )
			{
				S_MIX_ITEM_INFO const& kInsurance = g_kEqItemMixCreator.GetInsurance();
				pIcon->SetCustomData(&kInsurance.kItemPos, sizeof(kInsurance.kItemPos));
			}
			else
			{
				pIcon->ClearCustomData();
				return;
			}
		}
		else
		{
			g_kEqItemMixCreator.ClearInsurance();
			pIcon->ClearCustomData();
		}
		pSelf->Visible(false);
		pOther->Visible(true);

		SetEpItemInsuranceEffect(NULL, !g_kEqItemMixCreator.GetInsurance().IsEmpty());
	}

	void UpdateEpItemMixProbability(lwUIWnd kSelf)
	{
		if( g_kEqItemMixCreator.bMixing() )
		{
			lwAddWarnDataTT(50714);
			return;
		}

		XUI::CXUI_Button* pSelf = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pIcon = pParent->GetControl(L"ICN_RATEUP");
		if( !pIcon )
		{
			return;
		}

		std::wstring kOtherBtnName( L"BTN_REG" );
		bool const bIsReg = (pSelf->ID() == kOtherBtnName);
		if( bIsReg )
		{
			kOtherBtnName = L"BTN_DEREG";
		}
		XUI::CXUI_Wnd* pOther = pParent->GetControl(kOtherBtnName);
		if( !pOther )
		{
			return;
		}

		if( bIsReg )
		{
			if( g_kEqItemMixCreator.SetProbAbility(true) )
			{
				S_MIX_ITEM_INFO const& kProbAbility = g_kEqItemMixCreator.GetProbAbility();
				pIcon->SetCustomData(&kProbAbility.kItemPos, sizeof(kProbAbility.kItemPos));
			}
			else
			{
				pIcon->ClearCustomData();
				return;
			}
		}
		else
		{
			g_kEqItemMixCreator.ClearProbAbility();
			pIcon->ClearCustomData();
		}
		pSelf->Visible(false);
		pOther->Visible(true);
	}

	void CallEqItemMixToolTip(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf )
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			PgInventory* pkInv = pkPlayer->GetInven();
			if( !pkInv )
			{
				return;
			}

			SItemPos	kPos;
			pSelf->GetCustomData(&kPos, sizeof(kPos));
			
			PgBase_Item kItem;
			if( kPos != SItemPos::NullData() && S_OK == pkInv->GetItem(kPos, kItem) )
			{
				CallToolTip_SItem(&kItem, lwPoint2(XUIMgr.MousePos().x + 3, XUIMgr.MousePos().y + 3));
			}
			else
			{
				pSelf->ClearCustomData();

				XUI::CXUI_Wnd* pParent = pSelf->Parent();
				if( pParent )
				{
					pSelf = pParent->GetControl(pSelf->ID() + std::wstring(L"_BAG"));
					if( pSelf )
					{
						DWORD kItemNo;
						pSelf->GetCustomData(&kItemNo, sizeof(kItemNo));

						CallToolTip_ItemNo(kItemNo, lwPoint2(XUIMgr.MousePos().x + 3, XUIMgr.MousePos().y + 3));
					}
				}
			}
		}

	}

	void CallEqItemMixCreatorUI()
	{
		XUI::CXUI_Wnd* pParent = XUIMgr.Activate(L"SFRM_ITEM_MIX_CREATOR");
		if( !pParent )
		{
			g_kEqItemMixCreator.Clear();
			return;
		}

		lwItemMix::ReSetEpItemMixProgressing(pParent);

		S_MIX_ITEM_INFO const& kRecipe = g_kEqItemMixCreator.GetRecipe();
		S_MIX_ITEM_INFO const& kInsurance = g_kEqItemMixCreator.GetInsurance();
		S_MIX_ITEM_INFO const& kProbAbil = g_kEqItemMixCreator.GetProbAbility();
		CONT_MATERIAL const& kMaterial = g_kEqItemMixCreator.GetMaterialInfo();

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"ICN_RECIPE");
		if( pTemp )
		{
			pTemp->SetCustomData(&kRecipe.kItemPos, sizeof(kRecipe.kItemPos));
		}

		SetEpItemMixResultItem(kRecipe.kItemNo);

		pTemp = pParent->GetControl(L"FRM_INSURANCE_SCROLL");
		if( pTemp )
		{
			bool bIsEmptySlot = kInsurance.IsEmpty();

			SetEpItemInsuranceEffect(pParent, !bIsEmptySlot);

			XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp2 )
			{
				int const iItemNo = (bIsEmptySlot)?(DEF_EQ_MIX_INSURANCE_ITEM_NO):(kInsurance.kItemNo);
				pTemp2->SetCustomData(&iItemNo, sizeof(iItemNo));
			}

			if( !bIsEmptySlot )
			{
				pTemp2 = pTemp->GetControl(L"ICN_RATEUP");
				if( pTemp2 )
				{
					pTemp2->SetCustomData(&kInsurance.kItemPos, sizeof(kInsurance.kItemPos));
				}

				pTemp2 = pTemp->GetControl(L"BTN_DEREG");
				if( pTemp2 )
				{
					pTemp2->Visible(!bIsEmptySlot);
				}
				pTemp2 = pTemp->GetControl(L"BTN_REG");
				if( pTemp2 )
				{
					pTemp2->Visible(bIsEmptySlot);
				}
			}
		}

		pTemp = pParent->GetControl(L"FRM_RATE_UP_SCROLL");
		if( pTemp )
		{
			bool bIsEmptySlot = kProbAbil.IsEmpty();

			XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp2 )
			{
				int const iProbItemNo = (bIsEmptySlot)?(DEF_EQ_MIX_PROB_ITEM_NO):(kProbAbil.kItemNo);
				pTemp2->SetCustomData(&iProbItemNo, sizeof(iProbItemNo));
			}

			if( !bIsEmptySlot )
			{
				pTemp2 = pTemp->GetControl(L"ICN_RATEUP");
				if( pTemp2 )
				{
					pTemp2->SetCustomData(&kProbAbil.kItemPos, sizeof(kProbAbil.kItemPos));
				}

				pTemp2 = pTemp->GetControl(L"BTN_DEREG");
				if( pTemp2 )
				{
					pTemp2->Visible(!bIsEmptySlot);
				}
				pTemp2 = pTemp->GetControl(L"BTN_REG");
				if( pTemp2 )
				{
					pTemp2->Visible(bIsEmptySlot);
				}
			}
		}

		bool bIsLack = false;

		XUI::CXUI_Builder* pBuild1 = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT1"));
		XUI::CXUI_Builder* pBuild2 = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT2"));
		if( pBuild1 && pBuild2 )
		{
			CONT_MATERIAL::const_iterator mtrl_itor = kMaterial.begin();

			int const iMaxSlot = pBuild1->CountX() * pBuild1->CountY() + pBuild2->CountX() * pBuild2->CountY();
			int const iHalfSlot = iMaxSlot / 2;
			float fStartTime = g_pkApp->GetAccumTime() + 0.2f;
			for(int i = 0; i < iHalfSlot; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					BM::vstring vStr(L"FRM_MATERIAL_SLOT");
					vStr += j * iHalfSlot + i;

					XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
					if( pSlot )
					{
						if( kRecipe.kItemNo == 0 )
						{
							SetEpItemMixItemSlot(pSlot, 0, S_MIX_ITEM_INFO(0));
							g_kEqItemMixCreator.LackMaterial(true);
						}
						else
						{
							S_MIX_ITEM_INFO	kItemInfo(0);
							if( mtrl_itor != kMaterial.end() )
							{
								CONT_MATERIAL::mapped_type const& kMaterialInfo = mtrl_itor->second;
								kItemInfo = kMaterialInfo;
								++mtrl_itor;
							}

							if( kItemInfo.kItemNo == kRecipe.kItemNo )
							{
								--j;
								continue;
							}

							if( !SetEpItemMixItemSlot(pSlot, kRecipe.iNeed, kItemInfo) )
							{
								g_kEqItemMixCreator.LackMaterial(true);
							}
						}
					}
				}
			}
		}

		pTemp = pParent->GetControl(L"SFRM_NOTICE");
		if( pTemp )
		{
			__int64 iCost = g_kEqItemMixCreator.Cost();
			std::wstring kStr = TTW(65);
			kStr += std::wstring(L" ") + TTW(401111) + std::wstring(L" : ");
			kStr += GetMoneyString(iCost);
			pTemp->Text( kStr );
		}

		if( 0 == kRecipe.iHave)
		{
			g_kEqItemMixCreator.bMixing(false);
			pParent->Close();//레시피 갯수가 0이면 UI 닫힘
		}

	}

	void UpdateEpItemMixProgressing(XUI::CXUI_Wnd* pSelf)
	{
		if( g_kEqItemMixCreator.bMixing() )
		{
			S_UPDATE_TIME	kTime;
			pSelf->GetCustomData(&kTime, sizeof(kTime));

			if( kTime.fTotalTime == 0 )
			{
				return;
			}

			float fNowTime = g_pkApp->GetAccumTime();
			if( kTime.fStartTime > fNowTime )
			{
				return;
			}

			float fPercent = (fNowTime - kTime.fStartTime) / kTime.fTotalTime;
			if( fPercent >= 0.99f )
			{
				fPercent = 1.0f;
			}

			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pSelf);
			if( pAniBar )
			{
				pAniBar->Now(static_cast<int>(fPercent * 100));
			}
			else
			{
				POINT2 const& kWndSize = pSelf->Size();
				POINT2 kImgSize = pSelf->ImgSize();
				kImgSize.y = kWndSize.y * fPercent;
				pSelf->ImgSize(kImgSize);
			}
		}
	}

	void UpdateEpItemMixGearAni(XUI::CXUI_Wnd* pSelf)
	{
		if( g_kEqItemMixCreator.bMixing() )
		{
			MixUIUtil::UpdateMixGearAni(pSelf);
		}
	}

	bool SetEpItemMixItemSlot(XUI::CXUI_Wnd* pSelf, int const RecipeCnt, S_MIX_ITEM_INFO const& kItemInfo)
	{
		if( NULL == pSelf )
		{
			return false;
		}

		pSelf->Visible(!kItemInfo.IsEmpty());
		if( !pSelf->Visible() )
		{
			return true;
		}

		XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"ICN_MATERIAL");
		if( pTemp && kItemInfo.iHave )
		{
			pTemp->SetCustomData(&kItemInfo.kItemPos, sizeof(kItemInfo.kItemPos));
		}
		else if( pTemp )
		{
			pTemp->ClearCustomData();
		}
		
		pTemp = pSelf->GetControl(L"ICN_MATERIAL_BAG");
		if( pTemp )
		{
			pTemp->SetCustomData(&kItemInfo.kItemNo, sizeof(kItemInfo.kItemNo));
		}

		pTemp = pSelf->GetControl(L"FRM_ITEM_NAME");
		if( pTemp )
		{
			std::wstring kItemName = L"";
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItemInfo.kItemNo);
			if(pDef)
			{
				std::wstring const* pkItemName = NULL;
				if( GetDefString(pDef->NameNo(), pkItemName) )
				{
					kItemName = *pkItemName;
				}
			}
			int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
			Quest::SetCutedTextLimitLength(pTemp, kItemName, WSTR_UI_REGULATIONSTR, iWidth);
		}

		int const iTotalNeed = kItemInfo.iNeed * RecipeCnt;
		BM::vstring kStr(iTotalNeed);
		pTemp = pSelf->GetControl(L"FRM_CNT_NEED");
		if( pTemp )
		{
			pTemp->Text(kStr);
		}

		pTemp = pSelf->GetControl(L"FRM_CNT_HAVE");
		if( pTemp )
		{
			if( kItemInfo.iHave < iTotalNeed )
			{
				pTemp->FontColor(0xFFFF4A51);
				kStr = kItemInfo.iHave;
			}
			else
			{
				pTemp->FontColor(0xFF88FF27);
				kStr = iTotalNeed;
			}
			pTemp->Text(kStr);
		}

		return (kItemInfo.iHave >= iTotalNeed);
	}

	void SetEpItemMixResultItem(int const iRecipeNo)
	{
		XUI::CXUI_Wnd* pSelf = XUIMgr.Get(L"SFRM_ITEM_MIX_CREATOR");
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pSrc = NULL;
		XUI::CXUI_Wnd* pName = NULL;

		pSrc = pSelf->GetControl(L"FRM_RESULT_ITEM");
		if( pSrc )
		{
			pSrc = pSrc->GetControl(L"ICN_SRC");
			if( pSrc )
			{
				pSrc->ClearCustomData();
			}
		}

		pName = pSelf->GetControl(L"FRM_RESULT_ITEM_NAME");
		if( pName )
		{
			pName->Text(L"");
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pkDefItem = NULL;

		//레시피 아이템 정보
		pkDefItem = kItemDefMgr.GetDef(iRecipeNo);
		if( !pkDefItem ){ return; }

		int const iMakingNo = pkDefItem->GetAbil(AT_ITEMMAKING_NO);

		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		SDefItemMakingData kMakingData;
		bool const bRet = kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, iMakingNo);
		if( !bRet ){ return; }

		int iResultItemNo = 0;
		for(int i=0;i<MAX_ITEMMAKING_ARRAY;++i)
		{
			for(int j=0;j<MAX_SUCCESS_RATE_ARRAY;++j)
			{
				if(0<kMakingData.akResultItemElements[i].aElement[j])
				{
					iResultItemNo = kMakingData.akResultItemElements[i].aElement[j];
					break;
				}
			}
		}
		
		//결과 아이템 정보
		pkDefItem = kItemDefMgr.GetDef(iResultItemNo);
		if( !pkDefItem ){ return; }

		if(pSrc)
		{
			pSrc->SetCustomData(&iResultItemNo, sizeof(iResultItemNo));
		}
		if(pName)
		{
			std::wstring const* pItemName = NULL;
			if(GetDefString(pkDefItem->NameNo(),pItemName))
			{
				pName->Text(*pItemName);
			}
		}
	}

	void ReSetEpItemMixProgressing(XUI::CXUI_Wnd* pSelf)
	{
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"SFRM_BG");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pGear = pTemp->GetControl(L"IMG_GEAR_L");
			if( pGear )
			{
				pGear->ClearCustomData();
			}
			pGear = pTemp->GetControl(L"IMG_GEAR_R");
			if( pGear )
			{
				pGear->ClearCustomData();
			}
		}

		pTemp = pSelf->GetControl(L"ICN_RECIPE");
		if( pTemp )
		{
			pTemp->ClearCustomData();
		}

		pTemp = pSelf->GetControl(L"FRM_INSURANCE_SCROLL");
		if( pTemp )
		{
			SetEpItemInsuranceEffect(pSelf, false);

			XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp2 )
			{
				pTemp2->ClearCustomData();
			}

			pTemp2 = pTemp->GetControl(L"ICN_RATEUP");
			if( pTemp2 )
			{
				pTemp2->ClearCustomData();
			}

			pTemp2 = pTemp->GetControl(L"BTN_DEREG");
			if( pTemp2 )
			{
				pTemp2->Visible(false);
			}
			pTemp2 = pTemp->GetControl(L"BTN_REG");
			if( pTemp2 )
			{
				pTemp2->Visible(true);
			}
		}

		pTemp = pSelf->GetControl(L"FRM_RATE_UP_SCROLL");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(L"ICN_RATEUP_BAG");
			if( pTemp2 )
			{
				pTemp2->ClearCustomData();
			}

			pTemp2 = pTemp->GetControl(L"ICN_RATEUP");
			if( pTemp2 )
			{
				pTemp2->ClearCustomData();
			}

			pTemp2 = pTemp->GetControl(L"BTN_DEREG");
			if( pTemp2 )
			{
				pTemp2->Visible(false);
			}
			pTemp2 = pTemp->GetControl(L"BTN_REG");
			if( pTemp2 )
			{
				pTemp2->Visible(true);
			}
		}

		XUI::CXUI_Builder* pBuild1 = dynamic_cast<XUI::CXUI_Builder*>(pSelf->GetControl(L"BLD_MATERIAL_SLOT1"));
		XUI::CXUI_Builder* pBuild2 = dynamic_cast<XUI::CXUI_Builder*>(pSelf->GetControl(L"BLD_MATERIAL_SLOT2"));
		if( pBuild1 && pBuild2 )
		{
			int const iMaxSlot = pBuild1->CountX() * pBuild1->CountY() + pBuild2->CountX() * pBuild2->CountY();
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pSelf->GetControl(vStr);
				if( pSlot )
				{
					SetEpItemMixItemSlot(pSlot, 0, S_MIX_ITEM_INFO(0));
					XUI::CXUI_AniBar* pProgBar = dynamic_cast<XUI::CXUI_AniBar*>(pSlot->GetControl(L"IMG_SUBPROG"));
					if( pProgBar )
					{
						pProgBar->Now(0);
						pProgBar->ClearCustomData();
					}
				}
			}
		}

		pTemp = pSelf->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			POINT2 kImgSize = pTemp->ImgSize();
			kImgSize.y = 0;
			pTemp->ImgSize(kImgSize);
			pTemp->ClearCustomData();
		}

		SetEpItemMixResultItem(0);

		pTemp = pSelf->GetControl(L"BTN_MIX");
		if( pTemp )
		{
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(false);
			}
		}
	}

	void SetEpItemInsuranceEffect(XUI::CXUI_Wnd* pSelf, bool const bVisible)
	{
		if( !pSelf )
		{
			pSelf = XUIMgr.Get( L"SFRM_ITEM_MIX_CREATOR" );
		}
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pEffect = pSelf->GetControl(L"IMG_INSURANCE_RECIPE");
		if(pEffect)
		{
			pEffect->Visible(bVisible);
		}
	}

	//이건 코스튬 조합
	void PgUIData_CostumeMix::WriteToPacket(BM::Stream& kPacket)
	{
		kPacket.Push(PT_C_M_REQ_MIXUPITEM);
		kPacket.Push(m_kNpcGUID);
		kPacket.Push(m_kLeftItem);
		kPacket.Push(m_kRightItem);
		kPacket.Push(m_kRateItem);
	}

	void PgUIData_CostumeMix::ItemClear()
	{
		m_kLeftItem.Clear();
		m_kRightItem.Clear();
		m_kRateItem.Clear();
	}

	void CallCostumeMixNpcMenu(lwGUID kGuid)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_COSTUME_MIXER_NPC_MENU");
		if( pMainUI )
		{
			pMainUI->OwnerGuid( kGuid.GetGUID() );
		}

		RegistUIAction(pMainUI, L"CloseUI");
	}

	void CallCostumeMixUI(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		if( pParent->OwnerGuid().IsNull() )
		{
			return;
		}

		const BM::GUID kGuid = pParent->OwnerGuid();
		pParent->Close();

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_COSTUME_MIXER");
		if( !pMainUI )
		{
			return;
		}

		RegistUIAction(pMainUI, L"CloseUI");		
		
		pMainUI->OwnerGuid( kGuid );
		ClearCostumeMixUI( pMainUI );

		g_kUIDataMgr.Remove( UIDATA_COSTUME_MIX );
		PgUIData_CostumeMix* pkCostumeMixData = new PgUIData_CostumeMix;
		if( pkCostumeMixData )
		{
			pkCostumeMixData ->m_kNpcGUID = kGuid;
			g_kUIDataMgr.Add( pkCostumeMixData );
		}

		XUIMgr.Activate(L"Inv");
	}

	void StartCostumeMixing(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		PgUIData_CostumeMix* pkCostumeMixData = NULL;
		if( !g_kUIDataMgr.Get( UIDATA_COSTUME_MIX, pkCostumeMixData ) )
		{
			return;
		}

		if( SItemPos::NullData() == pkCostumeMixData->m_kRightItem 
		||  SItemPos::NullData() == pkCostumeMixData->m_kLeftItem )
		{
			SetCostumeLogMessage(TTW(50809));
			return;
		}

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"SFRM_BG");
		if( pTemp )
		{
			float fStartTime = g_pkApp->GetAccumTime();
			XUI::CXUI_Wnd* pGear = pTemp->GetControl(L"IMG_GEAR_L");
			if( pGear )
			{
				pGear->SetCustomData(&fStartTime, sizeof(fStartTime));
			}
			pGear = pTemp->GetControl(L"IMG_GEAR_R");
			if( pGear )
			{
				pGear->SetCustomData(&fStartTime, sizeof(fStartTime));
			}
		}

		pTemp = pParent->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			S_UPDATE_TIME kTime;
			kTime.fStartTime = g_pkApp->GetAccumTime();
			kTime.fTotalTime = 1.5f;
			pTemp->SetCustomData(&kTime, sizeof(kTime));
		}

		pTemp = pParent->GetControl(L"IMG_MIX");
		if( pTemp )
		{
			pTemp->Visible(false);
		}

		pTemp = pParent->GetControl(L"BTN_MIX");
		if( pTemp )
		{
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(true);
			}
			pTemp->Text(TTW(50705));
			
		}

		pkCostumeMixData->m_kProgressing = true;
		SetCostumeLogMessage(TTW(50817));
	}

	void UpdateCostumeMixUI(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		PgUIData_CostumeMix* pkCostumeMixData = NULL;
		if( !g_kUIDataMgr.Get( UIDATA_COSTUME_MIX, pkCostumeMixData ) )
		{
			return;
		}

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"IMG_MIX");
		if( pTemp )
		{
			if( SItemPos::NullData() != pkCostumeMixData->m_kRightItem 
			&&  SItemPos::NullData() != pkCostumeMixData->m_kLeftItem )
			{
				float fEndTime = 0.0f;
				pTemp->GetCustomData(&fEndTime, sizeof(fEndTime));
				fEndTime += 0.05f;
				if( fEndTime < g_pkApp->GetAccumTime() )
				{
					fEndTime = g_pkApp->GetAccumTime() + 0.05f;
					pTemp->SetCustomData(&fEndTime, sizeof(fEndTime));
					pTemp->UVUpdate((pTemp->UVInfo().Index == 1)?(2):(1));
				}
			}
		}

		if( !pkCostumeMixData->m_kProgressing )
		{
			return;
		}

		pTemp = pParent->GetControl(L"SFRM_BG");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pGear = pTemp->GetControl(L"IMG_GEAR_L");
			if( pGear )
			{
				MixUIUtil::UpdateMixGearAni(pGear);
			}
			pGear = pTemp->GetControl(L"IMG_GEAR_R");
			if( pGear )
			{
				MixUIUtil::UpdateMixGearAni(pGear);
			}
		}

		pTemp = pParent->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			MixUIUtil::UpdateMixProgressing(pTemp);
			if( pTemp->ImgSize().y == pTemp->Size().y )
			{
				BM::Stream kPacket;
				pkCostumeMixData->WriteToPacket(kPacket);
				NETWORK_SEND(kPacket);
				ClearCostumeMixUI(pParent);
			}
		}
	}

	void OnDragDropCostumeMixItem(lwUIWnd kSelf, int const iSlot)
	{
		XUI::CXUI_Icon* pIconUI = dynamic_cast<XUI::CXUI_Icon*>(kSelf.GetSelf());
		if( !pIconUI )
		{
			return;
		}

		SItemPos kItemPos;
		pIconUI->GetCustomData(&kItemPos, sizeof(kItemPos));

		XUI::CXUI_Wnd* pParent = pIconUI->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pItemWnd = pParent->GetControl(L"FRM_ITEM_NAME");
		if( !pItemWnd )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem(kItemPos, kItem) )
		{
			return;
		}

		PgUIData_CostumeMix* pkCostumeMixData = NULL;
		if( !g_kUIDataMgr.Get( UIDATA_COSTUME_MIX, pkCostumeMixData ) )
		{
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pItemDef )
		{
			return;
		}

		switch( iSlot )
		{
		case EST_LEFT_ITEM:
		case EST_RIGHT_ITEM:
			{
				std::wstring const* pItemName = NULL;
				if( GetDefString( pItemDef->NameNo(), pItemName ) )
				{
					Quest::SetCutedTextLimitLength(pItemWnd, *pItemName, _T("..."), pItemWnd->Size().x - 10);
				}

				E_ITEM_GRADE eGrade = GetItemGrade(kItem);
				if( 0 != pItemDef->GetAbil(AT_GRADE)//그래이드가 있고
				&& !kItem.EnchantInfo().IsBinding()//바인딩이 아니고
				&& (eGrade != IG_SEAL && eGrade != IG_CURSE)//봉인도 저주도 아니고
				&& CheckComstumeMixCompareSlot(static_cast<lwItemMix::ESLOT_TYPE>(iSlot), kItem, pkCostumeMixData) )//조건 넘기면
				{//처리하자
					SItemPos* pItemPos = NULL;
					if( iSlot == EST_LEFT_ITEM )
					{
						pItemPos = &pkCostumeMixData->m_kLeftItem;
						pParent->GetControl(L"IMG_ITEM0")->Visible(false);
					}
					else
					{
						pItemPos = &pkCostumeMixData->m_kRightItem;
						pParent->GetControl(L"IMG_ITEM1")->Visible(false);
					}

					*pItemPos = kItemPos;
					if( pkCostumeMixData->m_kLeftItem == pkCostumeMixData->m_kRightItem )
					{
						SetCostumeLogMessage(TTW(50815));
						pItemPos->Clear();
						pIconUI->ClearCustomData();
						pItemWnd->Text(L"");
					}
					else
					{
						if( pkCostumeMixData->m_kLeftItem != SItemPos::NullData() 
						&& pkCostumeMixData->m_kRightItem != SItemPos::NullData() )
						{
							SetCostumeLogMessage(TTW(50816));
							pItemWnd = pParent->GetControl(L"IMG_MIX");
							if( pItemWnd )
							{
								pItemWnd->Visible(true);
							}
						}
					}
				}
				else
				{
					SetCostumeLogMessage(TTW(50810));
					switch(iSlot)
					{
					case EST_LEFT_ITEM: { pkCostumeMixData->m_kLeftItem.Clear(); }break;
					case EST_RIGHT_ITEM:{ pkCostumeMixData->m_kRightItem.Clear(); }break;
					}
					pIconUI->ClearCustomData();
					pItemWnd->Text(L"");
				}
			}break;
		case EST_RATE_ITEM:
			{
				pkCostumeMixData->m_kRateItem = kItemPos;
				pkCostumeMixData->m_kAutoRegRateItem = true;
			}break;
		default:
			{
			}return;
		}
	}

	void ClearCostumeMixUI(XUI::CXUI_Wnd* pMainUI)
	{
		int const MAX_SLOT = 2;

		if( !pMainUI )
		{
			pMainUI = XUIMgr.Get(L"SFRM_COSTUME_MIXER");
			if( pMainUI && pMainUI->IsClosed() )
			{
				return;
			}
		}

		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_MATERIAL_SLOT_");
			vStr += i;

			XUI::CXUI_Wnd* pSlotUI = pMainUI->GetControl(vStr);
			if( pSlotUI )
			{
				XUI::CXUI_Icon* pIconUI = dynamic_cast<XUI::CXUI_Icon*>(pSlotUI->GetControl(L"ICN_MATERIAL"));
				if( pIconUI )
				{
					pIconUI->ClearCustomData();
				}

				XUI::CXUI_Wnd* pNameUI = pSlotUI->GetControl(L"FRM_ITEM_NAME");
				if( pNameUI )
				{
					pNameUI->Text(L"");
				}
			}
		}

		XUI::CXUI_Wnd* pProgressUI = pMainUI->GetControl(L"IMG_MAIN_PROGBAR");
		if( pProgressUI )
		{
			POINT2 kImgSize = pProgressUI->ImgSize();
			kImgSize.y = 0;
			pProgressUI->ImgSize(kImgSize);
			pProgressUI->ClearCustomData();
		}

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"IMG_MIX");
		if( pTemp )
		{
			pTemp->Visible(false);
		}

		pTemp = pMainUI->GetControl(L"BTN_MIX");
		if( pTemp )
		{
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			if( pButton )
			{
				pButton->Disable(false);
			}
			pTemp->Text(TTW(50704));
		}

		SetCostumeLogMessage(TTW(50819));

		PgUIData_CostumeMix* pkCostumeMixData = NULL;
		if( g_kUIDataMgr.Get( UIDATA_COSTUME_MIX, pkCostumeMixData ) )
		{
			pkCostumeMixData->ItemClear();
			pkCostumeMixData->m_kProgressing = false;
		}
	}

	bool CheckComstumeMixCompareSlot(ESLOT_TYPE const kInItemSlot, PgBase_Item const& kItem, PgUIData_CostumeMix* pkCostumeMixData)
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pItemDef )
		{
			return false;
		}

		if( !pkCostumeMixData )
		{
			return false;
		}

		SItemPos* kCompItemSlot = NULL;
		switch( kInItemSlot )
		{
		case EST_LEFT_ITEM:	{ kCompItemSlot = &(pkCostumeMixData->m_kRightItem); }break;
		case EST_RIGHT_ITEM:{ kCompItemSlot = &(pkCostumeMixData->m_kLeftItem);  }break;
		default:{}return false;
		}

		if( (*kCompItemSlot) == SItemPos::NullData() )
		{
			return true;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		PgBase_Item kCompItem;
		if( S_OK != pkInv->GetItem((*kCompItemSlot), kCompItem) )
		{
			SetCostumeLogMessage(TTW(50811));
			return false;
		}

		CItemDef const* pCompItemDef = kItemDefMgr.GetDef(kCompItem.ItemNo());
		if( !pCompItemDef )
		{
			return false;
		}

		if( pItemDef->EquipPos() != pCompItemDef->EquipPos() )
		{
			SetCostumeLogMessage(TTW(50811));
			return false;
		}

		if( pItemDef->GetAbil(AT_GENDERLIMIT) != pCompItemDef->GetAbil(AT_GENDERLIMIT) )
		{
			SetCostumeLogMessage(TTW(50813));
			return false;
		}

		if( pItemDef->GetAbil(AT_GRADE) != pCompItemDef->GetAbil(AT_GRADE) )
		{
			SetCostumeLogMessage(TTW(50814));
			return false;
		}

		return true;
	}

	void SetCostumeLogMessage(std::wstring const& kText)
	{
		XUI::CXUI_Wnd* pParent = XUIMgr.Get(L"SFRM_COSTUME_MIXER");
		if( pParent )
		{
			XUI::CXUI_Wnd* pLogBox = pParent->GetControl(L"SFRM_NOTICE");
			if( pLogBox )
			{
				pLogBox->Text(kText);

				XUI::CXUI_Style_String kString = pLogBox->StyleText();
				POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kString));

				POINT2 kPos( pLogBox->TextPos().x, (pLogBox->Size().y - kTextSize.y) * 0.5f );
				pLogBox->TextPos(kPos);
			}
		}
	}

	void RecvCostumeMix_Command(WORD wPacketType, BM::Stream& kPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_MIXUPITEM:
			{
				HRESULT kResult;
				kPacket.Pop(kResult);
				if( kResult == S_OK )
				{
					PgBase_Item kItem;
					kItem.ReadFromPacket(kPacket);

					ClearCostumeMixUI(NULL);

					XUI::CXUI_Wnd* pMain = XUIMgr.Activate(L"FRM_MIX_CREATOR_SUCCESS", true);
					if (!pMain)
					{
						return;
					}
					XUI::CXUI_Wnd* pIcon = pMain->GetControl(L"ICN_ITEM");
					if (!pIcon)
					{
						return;
					}
					lwUIWnd(pIcon).SetCustomDataAsSItem(kItem);
					PgUIUtil::DrawIconToItemNo(pIcon, kItem.ItemNo(), 1.5f);

					const wchar_t *pText = NULL;
					std::wstring wstrText;
					GetDefString(kItem.ItemNo(), pText);

					wchar_t szTemp[MAX_PATH] = {0,};
					swprintf_s(szTemp, MAX_PATH, TTW(50820).c_str(), pText);

					pMain->Text(szTemp);
				}
				else
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50809 + kResult - E_MIXUP_NOT_FOUND_ITEM, true);
				}
			}break;
		default:
			{
			}break;
		}
	}

	PgUIData_CommonMixer::PgUIData_CommonMixer(E_UIDATA_TYPE const kType)
		: m_pkMixer(NULL)
		, m_kType(kType)
	{
	}

	PgUIData_CommonMixer::~PgUIData_CommonMixer(void)
	{
	}

	void AddNewCommonMixerInfo(SItemPos const& kItemPos)
	{
		if( g_kCommonTypeMixer.GetRecipe().IsEmpty() )
		{
			g_kCommonTypeMixer.Clear();
			if( g_kCommonTypeMixer.SetRecipe(kItemPos) )
			{
				lwItemMix::CallCommonTypeMixer();
				return;
			}
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1151, true);
		}
	}

	void DeleteCommonMixerInfo()
	{
		g_kCommonTypeMixer.Clear();
	}

	void CallCommonTypeMixer()
	{
		PgCommonTypeMixer::SMIXER_INIT_INFO MixerInitInfo;

		if( !g_kCommonTypeMixer.GetMixerInitInfo(MixerInitInfo) )
		{
			return;
		}

		std::wstring const kUIName(L"SFRM_COMMONTYPE_MIXER");
		BM::vstring vNewID(kUIName);
		vNewID += MixerInitInfo.kAddonText;
		XUI::CXUI_Wnd* pParent = XUIMgr.Get(vNewID);
		if( !pParent || pParent->IsClosed() )
		{
			pParent = XUIMgr.Call(kUIName, false, vNewID);
			if( !pParent )
			{
				return;
			}
		}

		InitCommonTypeMixer(pParent, MixerInitInfo);
		ClearCommonTypeMixer(pParent);

		S_MIX_ITEM_INFO const& kRecipe = g_kCommonTypeMixer.GetRecipe();
		CONT_MATERIAL const& kMaterial = g_kCommonTypeMixer.GetMaterialInfo();
		DWORD const& kRewardItemNo = g_kCommonTypeMixer.RewordItemNo();

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"ICN_RECIPE");
		if( pTemp )
		{
			pTemp->SetCustomData(&kRecipe.kItemPos, sizeof(kRecipe.kItemPos));
		}

		bool bIsLack = false;

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT"));
		if( pBuild )
		{
			CONT_MATERIAL::const_iterator mtrl_itor = kMaterial.begin();

			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
				if( pSlot )
				{
					if( kRecipe.kItemNo == 0 )
					{
						SetCommonTypeMixerSlot(pSlot, 0, S_MIX_ITEM_INFO(0));
						g_kCommonTypeMixer.LackMaterial(true);
					}
					else
					{
						S_MIX_ITEM_INFO	kItemInfo(0);
						if( mtrl_itor != kMaterial.end() )
						{
							CONT_MATERIAL::mapped_type const& kMaterialInfo = mtrl_itor->second;
							kItemInfo = kMaterialInfo;
							++mtrl_itor;
						}

						if( kItemInfo.kItemNo == kRecipe.kItemNo )
						{
							--i;
							continue;
						}

						if( !SetCommonTypeMixerSlot(pSlot, kRecipe.iNeed, kItemInfo) )
						{
							g_kCommonTypeMixer.LackMaterial(true);
						}
					}
				}
			}
		}

		pTemp = pParent->GetControl(L"ICN_DEST");
		if( pTemp )
		{
			pTemp->SetCustomData(&kRewardItemNo, sizeof(kRewardItemNo));
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);

		pTemp = pParent->GetControl(L"SFRM_DEST_NAME");
		if( pTemp )
		{
			bool bGetNamed = false;
			std::wstring const* pItemName = NULL;
			CItemDef const *pDef = kItemDefMgr.GetDef(kRewardItemNo);
			if( pDef )
			{
				if( GetDefString(pDef->NameNo(), pItemName) )
				{
					int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
					Quest::SetCutedTextLimitLength(pTemp, *pItemName, WSTR_UI_REGULATIONSTR, iWidth);
					bGetNamed = true;
				}
			}

			if( !bGetNamed )
			{
				pTemp->Text(L"");
			}
		}

		pTemp = pParent->GetControl(L"SFRM_RECIPE_NAME");
		if( pTemp )
		{
			bool bGetNamed = false;
			std::wstring const* pItemName = NULL;
			CItemDef const *pDef = kItemDefMgr.GetDef(kRecipe.kItemNo);
			if( pDef )
			{
				if( GetDefString(pDef->NameNo(), pItemName) )
				{
					int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
					Quest::SetCutedTextLimitLength(pTemp, *pItemName, WSTR_UI_REGULATIONSTR, iWidth);
					bGetNamed = true;
				}
			}

			if( !bGetNamed )
			{
				pTemp->Text(L"");
			}
		}
	}

	void UpdateCommonTypeMixer(lwUIWnd kParent)
	{
		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		if( !g_kCommonTypeMixer.bMixing() )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT"));
		if( pBuild )
		{
			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pProgBar = pSlot->GetControl(L"IMG_SUBPROG");
					if( pProgBar )
					{
						MixUIUtil::UpdateMixProgressing(pProgBar);
					}
				}
			}
		}

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			MixUIUtil::UpdateMixProgressing(pTemp);
			if( pTemp->ImgSize().y == pTemp->Size().y )
			{
				g_kCommonTypeMixer.RequestMix();
				ClearCommonTypeMixer(pParent);
				g_kCommonTypeMixer.bMixing(false);
				pParent->Close();
			}
		}
	}

	void StartCommonTypeMixer(lwUIWnd kParent)
	{
		if( g_kCommonTypeMixer.bMixing() )
		{
			return;
		}

		PgCommonTypeMixer::SMIXER_INIT_INFO MixerInitInfo;

		if( !g_kCommonTypeMixer.GetMixerInitInfo(MixerInitInfo) )
		{
			return;
		}

		if( g_kCommonTypeMixer.GetRecipe().IsEmpty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", MixerInitInfo.iErrTT[PgCommonTypeMixer::ME_NO_REG_RECIPE], true);
			return;
		}

		if( g_kCommonTypeMixer.LackMaterial() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", MixerInitInfo.iErrTT[PgCommonTypeMixer::ME_MATERIAL_LACK], true);
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( g_kCommonTypeMixer.Cost() > pkPlayer->GetAbil64(AT_MONEY) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", MixerInitInfo.iErrTT[PgCommonTypeMixer::ME_MONEY_LACK], true);
			return;
		}

		XUI::CXUI_Wnd* pParent = kParent.GetSelf();
		if( !pParent )
		{
			return;
		}

		float const fNowTime = g_pkApp->GetAccumTime();

		XUI::CXUI_Wnd* pTemp = pParent->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			S_UPDATE_TIME kTime;
			kTime.fStartTime = fNowTime;
			kTime.fTotalTime = 2.0f;
			pTemp->SetCustomData(&kTime, sizeof(kTime));
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_MATERIAL_SLOT"));
		if( pBuild )
		{
			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			float fStartTime = fNowTime + 0.2f;
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pParent->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pProgBar = pSlot->GetControl(L"IMG_SUBPROG");
					if( pProgBar )
					{
						S_UPDATE_TIME kTime;
						kTime.fStartTime = fStartTime + i * 0.32f;
						kTime.fTotalTime = 0.34f;
						pProgBar->SetCustomData(&kTime, sizeof(kTime));
					}
				}
			}
		}

		g_kCommonTypeMixer.bMixing(true);
		lwPlaySoundByID("Item_Mix_Mixing");
	}

	bool SetCommonTypeMixerSlot(XUI::CXUI_Wnd* pSlot, int const RecipeCnt, S_MIX_ITEM_INFO const& kItemInfo)
	{
		if( kItemInfo.kItemNo == 0 )
		{
			pSlot->Visible(false);
			return true;
		}
		else
		{
			pSlot->Visible(true);
		}

		XUI::CXUI_Wnd* pIcon = pSlot->GetControl(L"ICN_MATERIAL");
		if( pIcon )
		{
			pIcon->SetCustomData(&kItemInfo.kItemPos, sizeof(kItemInfo.kItemPos));
		}

		XUI::CXUI_Wnd* pGrayIcon = pSlot->GetControl(L"ICN_MATERIAL_GRAY");
		if( pGrayIcon )
		{
			pGrayIcon->SetCustomData(&kItemInfo.kItemNo, sizeof(kItemInfo.kItemNo));
			pGrayIcon->GrayScale(true);
		}

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_MATERIAL_NAME");
		if( pTemp )
		{
			std::wstring const* ItemName = NULL;
			if( GetDefString(kItemInfo.kItemNo, ItemName) )
			{
				int const iWidth = pTemp->Width()+REGULATION_UI_WIDTH;
				Quest::SetCutedTextLimitLength(pTemp, *ItemName, WSTR_UI_REGULATIONSTR, iWidth);
			}
			else
			{
				pTemp->Text(L"");
			}
		}

		pTemp = pSlot->GetControl(L"IMG_SUBPROG");
		if( pTemp )
		{
			XUI::CXUI_AniBar* pAnibar = dynamic_cast<XUI::CXUI_AniBar*>(pTemp);
			if( pAnibar )
			{
				pAnibar->Now(0);
				pAnibar->ClearCustomData();
			}
		}

		int const iTotalNeed = kItemInfo.iNeed * RecipeCnt;
		if( 0 == iTotalNeed )
		{
			pSlot->Text(L"");
			return false;
		}
		else
		{
			BM::vstring vStr(TTW(790458));
			vStr += L" ";
			if( kItemInfo.iHave < iTotalNeed )
			{
				vStr += TTW(1302);
				vStr += kItemInfo.iHave;
			}
			else
			{
				vStr += TTW(1301);
				vStr += iTotalNeed;
			}
			vStr += TTW(1300);
			vStr += _T(" / ");
			vStr += iTotalNeed;

			pSlot->Text(vStr);
		}

		bool bIsEnough = (kItemInfo.iHave >= iTotalNeed);
		if( pIcon ){ pIcon->Visible(bIsEnough); }
		if( pGrayIcon ){ pGrayIcon->Visible(!bIsEnough); }
		return bIsEnough;
	}

	void ClearCommonTypeMixer(XUI::CXUI_Wnd* pMainUI)
	{
		if( pMainUI == NULL )
		{
			pMainUI = XUIMgr.Get(L"SFRM_COMMONTYPE_MIXER");
			if( !pMainUI || pMainUI->IsClosed() )
			{
				return;
			}
		}

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"ICN_RECIPE");
		if( pTemp )
		{
			pTemp->ClearCustomData();
		}

		bool bIsLack = false;

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_MATERIAL_SLOT"));
		if( pBuild )
		{
			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			for(int i = 0; i < iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_MATERIAL_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
				if( pSlot )
				{
					pSlot->Visible(true);
					SetCommonTypeMixerSlot(pSlot, 0, S_MIX_ITEM_INFO(0));
				}
			}
		}

		pTemp = pMainUI->GetControl(L"IMG_MAIN_PROGBAR");
		if( pTemp )
		{
			pTemp->ClearCustomData();
			pTemp->ImgSize(POINT2(pTemp->ImgSize().x, 0));
		}

		pTemp = pMainUI->GetControl(L"ICN_DEST");
		if( pTemp )
		{
			pTemp->ClearCustomData();
		}

		pTemp = pMainUI->GetControl(L"SFRM_DEST_NAME");
		if( pTemp )
		{
			pTemp->Text(L"");
		}

		pTemp = pMainUI->GetControl(L"SFRM_RECIPE_NAME");
		if( pTemp )
		{
			pTemp->Text(L"");
		}
	}

	void InitCommonTypeMixer(XUI::CXUI_Wnd* pMainUI, PgCommonTypeMixer::SMIXER_INIT_INFO const& kInfo)
	{
		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"SFRM_TITLE");
		if( pTemp )
		{
			pTemp = pTemp->GetControl(L"SFRM_COLOR");
			if( pTemp )
			{
				pTemp = pTemp->GetControl(L"IMG_TAB");
				if( pTemp )
				{
					pTemp->Text(TTW(kInfo.iTitleTT));
				}
			}
		}

		pTemp = pMainUI->GetControl(L"SFRM_COLOR");
		if( pTemp )
		{
			pTemp = pTemp->GetControl(L"SFRM_SHADOW");
			if( pTemp )
			{
				XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(L"FRM_ARROW1");
				if( pTemp2 )
				{
					pTemp2->Text(TTW(kInfo.iRegistSlotTT));
				}

				pTemp2 = pTemp->GetControl(L"FRM_ARROW2");
				if( pTemp2 )
				{
					pTemp2->Text(TTW(kInfo.iResultSlotTT));
				}
			}
		}

		pTemp = pMainUI->GetControl(L"BTN_TRY_REFINE");
		if( pTemp )
		{
			pTemp->Text(TTW(kInfo.iOKButtonTT) + TTW(2200));
		}
	}

	wchar_t const * const SZ_MONSTER_CARD_MIXER_TYPE2_MAIN = L"SFRM_MONSTERCARD_MIXER";
	wchar_t const * const SZ_MONSTER_CARD_PROG_BAR = L"IMG_MAIN_PROGBAR";
	wchar_t const * const SZ_MONSTER_CARD_BG = L"SFRM_BG";
	wchar_t const * const SZ_MONSTER_CARD_GEAR_L = L"IMG_GEAR_L";
	wchar_t const * const SZ_MONSTER_CARD_GEAR_R = L"IMG_GEAR_R";
	wchar_t const * const SZ_MONSTER_CARD_MARTERIAL = L"FRM_MATERIAL_SLOT_";
	wchar_t const * const SZ_MONSTER_CARD_INS = L"FRM_INSURANCE_SCROLL";
	wchar_t const * const SZ_MONSTER_CARD_RST = L"FRM_RESULT_ITEM";
	wchar_t const * const SZ_MONSTER_CARD_MIX_BTN = L"BTN_MIX";
	int const MAX_MARTERIAL_SLOT = 2;

	void lwOnCallMonsterCardMixerType2()
	{
		if( g_kItemMixMgr.CreateMixer(EMT_MONSTER_CARD_MIXER) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN);
			if( pMainUI )
			{
				PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
				if( pkMixer )
				{
					ClearMonsterCardMixerType2(pMainUI, pkMixer->DefaultInsItem());
					RegistUIAction(pMainUI);

					XUI::CXUI_Wnd* pkInvUI = XUIMgr.Activate(L"Inv");
					if( pkInvUI )
					{
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if( pkPlayer )
						{
							PgInventory* pkInv = pkPlayer->GetInven();
							if( pkInv )
							{
								ContHaveItemNoCount kCont;
								if( S_OK == pkInv->GetItems( UICT_SKILL_EXTEND, kCont ) )
								{
									SItemPos kItemPos;
									if( S_OK == pkInv->GetFirstItem( kCont.begin()->first, kItemPos ) )
									{
										XUI::CXUI_Wnd* pkTitle = pkInvUI->GetControl(L"SFRM_TITLE");
										if( pkTitle )
										{
											std::wstring kFormName;
											std::wstring kBtnName;
											switch( kItemPos.x )
											{
											case IT_EQUIP:		{ kFormName = L"SFRM_EQUIP_BG"; kBtnName = L"CBTN_EQUIP"; } break;
											case IT_CONSUME:	{ kFormName = L"SFRM_CONSUM_BG"; kBtnName = L"CBTN_CONSUM"; } break;
											case IT_ETC:		{ kFormName = L"SFRM_ETC_BG"; kBtnName = L"CBTN_ETC"; } break;
											case IT_CASH:		{ kFormName = L"SFRM_CASH_BG"; kBtnName = L"CBTN_CASH"; } break;
											}

											XUI::CXUI_Wnd* BgForm = pkTitle->GetControl(kFormName);
											if( BgForm )
											{
												XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(BgForm->GetControl(kBtnName));
												if( pkBtn )
												{
													pkBtn->DoScript(SCRIPT_ON_GET_FOCUS);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void lwOnUpdateMonsterCardMixerType2(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pMainUI = kSelf.GetSelf();
		if( !pMainUI )
		{
			return;
		}

		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( !pkMixer )
		{
			return;
		}

		if( pkMixer->bMixing() )
		{
			XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(SZ_MONSTER_CARD_BG);
			if( pTemp )
			{
				MixUIUtil::UpdateMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_L));
				MixUIUtil::UpdateMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_R));
			}

			if( MixUIUtil::UpdateMixProgressing(pMainUI->GetControl(SZ_MONSTER_CARD_PROG_BAR)) )
			{
				pkMixer->RequestMix();
				pkMixer->bMixing(false);
			}
		}
	}

	void lwOnCloseMonsterCardMixerType2()
	{
		lwStopSoundByID("Item_Mix_Mixing");
		g_kItemMixMgr.DeleteMixer(EMT_MONSTER_CARD_MIXER);
	}

	void lwSetDefaultInsItemNo(DWORD const InsItemNo)
	{
		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( pkMixer )
		{
			pkMixer->DefaultInsItem(InsItemNo);
		}		
	}

	void lwOnClickMonsterCardMixerType2Start()
	{
		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( !pkMixer )
		{
			return;
		}	

		if( pkMixer->bMixing() )
		{//이미 진행중
			return;
		}

		SetMonsterCardMixerType2MixItem();

		if( pkMixer->LeftItem().IsEmpty() || pkMixer->RightItem().IsEmpty() )
		{//아이템을 등록해
			lwAddWarnDataTT(790491);
			return;
		}

		if( pkMixer->LackMaterial() )
		{//재료가 부족해
			return;
		}

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN);
		if( pMainUI )
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			if( pkMixer->Cost() > pkPlayer->GetAbil64(AT_MONEY) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403078, true);
				return;
			}

			float const fNowTime = g_pkApp->GetAccumTime();
			XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(SZ_MONSTER_CARD_BG);
			if( pTemp )
			{
				MixUIUtil::InitMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_L), fNowTime);
				MixUIUtil::InitMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_R), fNowTime);
			}

			S_UPDATE_TIME kTime;
			kTime.fStartTime = fNowTime;
			kTime.fTotalTime = 2.0f;
			MixUIUtil::InitMixProgressing(pMainUI->GetControl(SZ_MONSTER_CARD_PROG_BAR), kTime);
			XUI::CXUI_Button* pBtn = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(SZ_MONSTER_CARD_MIX_BTN));
			if( pBtn )
			{
				pBtn->Disable(true);
			}

			pkMixer->bMixing(true);
			lwPlaySoundByID("Item_Mix_Mixing");
		}
	}

	void lwSetMonsterCardMixerType2Ins()
	{
		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( pkMixer )
		{
			if(pkMixer->bMixing())
			{
				lwAddWarnDataTT( 790488 );
				return;
			}

			pkMixer->bAutoRegInsItem(!pkMixer->bAutoRegInsItem());
			SetMonsterCardMixerType2InsItem(pkMixer);
		}		
	}

	void ClearMonsterCardMixerType2(XUI::CXUI_Wnd* pMainUI, int const DEFAULT_INS_ITEMNO)
	{
		if( !pMainUI )
		{
			return;
		}

		MixUIUtil::ClearMixProgressing(pMainUI->GetControl(SZ_MONSTER_CARD_PROG_BAR));
		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(SZ_MONSTER_CARD_BG);
		if( pTemp )
		{
			MixUIUtil::ClearMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_L));
			MixUIUtil::ClearMixGearAni(pTemp->GetControl(SZ_MONSTER_CARD_GEAR_R));
		}

		for( int i = 0; i < MAX_MARTERIAL_SLOT; ++i )
		{
			BM::vstring vStr(SZ_MONSTER_CARD_MARTERIAL);
			vStr += i;
			MixUIUtil::ClearMixMarterialSlot(pMainUI->GetControl(vStr));
		}

		MixUIUtil::ClearMixInsuranceSlot(pMainUI->GetControl(SZ_MONSTER_CARD_INS), DEFAULT_INS_ITEMNO);
		MixUIUtil::ClearMixResultSlot(pMainUI->GetControl(SZ_MONSTER_CARD_RST));

		XUI::CXUI_Button* pBtn = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(SZ_MONSTER_CARD_MIX_BTN));
		if( pBtn )
		{
			pBtn->Disable(false);
		}
	}

	void SetMonsterCardMixerType2MixItem()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( !pkMixer )
		{
			return;
		}

		SItemPos kItemPos;
		ClearMonsterCardMixerType2(XUIMgr.Get(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN), pkMixer->DefaultInsItem());
		if( S_OK == pkInv->GetFirstItem( pkMixer->LeftItem().kItemNo, kItemPos ) )
		{
			SetMonsterCardMixerType2MixItem( kItemPos, true );
		}
		else
		{
			pkMixer->SemiClear();
		}
	}

	void SetMonsterCardMixerType2MixItem(SItemPos const& kItemPos, bool const bIsNoMSG)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem(kItemPos, kItem) )
		{
			//아이템이 없다
			(bIsNoMSG)?(0):(lwAddWarnDataTT( 790487 ));
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef( kItem.ItemNo() );
		if( !pDef )
		{
			return;
		}

		if( UICT_SKILL_EXTEND != pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{	//몬스터카드 아니다
			(bIsNoMSG)?(0):(lwAddWarnDataTT( 1712 ));
			return;
		}

		if( EMCG_LEGEND <= pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2) )
		{
			//레전드가 마지막이라 합성불가
			(bIsNoMSG)?(0):(lwAddWarnDataTT( 790485 ));
			return;
		}

		PgMonsterCardMixer* pkMixer = dynamic_cast<PgMonsterCardMixer*>(g_kItemMixMgr.GetMixer(EMT_MONSTER_CARD_MIXER));
		if( !pkMixer || pkMixer->bMixing() )
		{
			return;
		}

		pkMixer->LackMaterial(false);
		if( !pkMixer->SetMonsterCard( kItem.ItemNo(), kItemPos ) )
		{
			(bIsNoMSG)?(0):(lwAddWarnDataTT( 790490 ));
			return;
		}

		if( !pkMixer->LackMaterial() )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN);
			if( !pMainUI )
			{
				return;
			}

			BM::vstring vLeft(SZ_MONSTER_CARD_MARTERIAL);
			vLeft += 0;
			MixUIUtil::SetMixMerterialSlot(pMainUI->GetControl(vLeft), pkMixer->LeftItem());
			BM::vstring vRight(SZ_MONSTER_CARD_MARTERIAL);
			vRight += 1;
			MixUIUtil::SetMixMerterialSlot(pMainUI->GetControl(vRight), pkMixer->RightItem());

			SetMonsterCardMixerType2InsItem(pkMixer, bIsNoMSG);
			MixUIUtil::SetMixResultSlot(pMainUI->GetControl(SZ_MONSTER_CARD_RST), pkMixer->RewordItemNo());

			XUI::CXUI_Button* pBtn = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(SZ_MONSTER_CARD_MIX_BTN));
			if( pBtn )
			{
				pBtn->Disable(false);
			}
		}
		else
		{
			//수량 부족 여기는 메시지 뿌려주자
			(bIsNoMSG)?(0):(lwAddWarnDataTT( 790484 ));
			ClearMonsterCardMixerType2(XUIMgr.Get(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN), pkMixer->DefaultInsItem());
		}
	}

	bool SetMonsterCardMixerType2InsItem(PgMonsterCardMixer* pkMixer, bool const bIsNoMSG)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		if( !pkMixer )
		{
			return false;
		}

		bool bFailed = false;
		if( pkMixer->bAutoRegInsItem() )	//자동등록일때만
		{
			S_MIX_ITEM_INFO kItemInfo = pkMixer->InsItem();
			if( kItemInfo.IsEmpty() )
			{
				ContHaveItemNoCount kItemCont;
				if( S_OK != pkInv->GetItems(UICT_SKILLEXTEND_INSURANCE, kItemCont) )
				{
					//아이템이 없다.
					bFailed = true;
				}
				else
				{
					kItemInfo.kItemNo = kItemCont.begin()->first;
					kItemInfo.iHave = kItemCont.begin()->second;
					kItemInfo.iNeed = 1;
					if( S_OK != pkInv->GetFirstItem(kItemInfo.kItemNo, kItemInfo.kItemPos) )
					{
						//아이템이 없어
						bFailed = true;
					}
				}
			}
			else
			{
				PgBase_Item kItem;
				if( S_OK != pkInv->GetItem(kItemInfo.kItemPos, kItem) )
				{
					if( S_OK != pkInv->GetFirstItem( kItemInfo.kItemNo, kItemInfo.kItemPos ) )
					{
						//아이템이 없어
						bFailed = true;
					}
				}

				if( false == bFailed )
				{
					kItemInfo.iHave = pkInv->GetTotalCount( kItemInfo.kItemNo );
				}
			}

			if( bFailed )
			{
				pkMixer->InsItem(S_MIX_ITEM_INFO());
				pkMixer->bAutoRegInsItem(false);
				SetMonsterCardMixerType2InsItemToPos( pkMixer );
				(bIsNoMSG)?(0):(lwAddWarnDataTT( 790486 ));
				return false;
			}
			else
			{
				pkMixer->InsItem(kItemInfo);
				SetMonsterCardMixerType2InsItemToPos( pkMixer );
			}
		}
		else
		{
			pkMixer->InsItem(S_MIX_ITEM_INFO());
			pkMixer->bAutoRegInsItem(false);
			SetMonsterCardMixerType2InsItemToPos( pkMixer );
		}
		return true;
	}

	bool SetMonsterCardMixerType2InsItemToPos(PgMonsterCardMixer* pkMixer)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(SZ_MONSTER_CARD_MIXER_TYPE2_MAIN);
		if( pMainUI && pkMixer )
		{
			MixUIUtil::SetMixInsurance(pMainUI->GetControl(SZ_MONSTER_CARD_INS), pkMixer->InsItem(), pkMixer->DefaultInsItem());
			return true;
		}
		return false;
	}

	void OnCallMonsterCardMixerType2Result(CItemDef const* pDef, CONT_DEFRES const* pResDef, PgBase_Item const& kItem)
	{
		if( !pDef || !pResDef || kItem.IsEmpty() )
		{
			return;
		}
	
		CONT_DEFRES::const_iterator itrDest = pResDef->find(pDef->ResNo());
		if(itrDest == pResDef->end())
		{
			// 아이템 테이블에서 못 찾으면 패스!
			return;
		}

		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(L"FRM_MONSTER_CARD_MIX_TYPE2_SUCCESS", true);
		if (!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pIcon = pMain->GetControl(L"ICN_ITEM");
		if (!pIcon)
		{
			return;
		}
		lwUIWnd(pIcon).SetCustomDataAsSItem(kItem);
		PgUIUtil::DrawIconToItemNo(pIcon, kItem.ItemNo(), 1.5f);

		std::wstring const* wstrText;
		if( GetDefString(pDef->NameNo(), wstrText) )
		{
			BM::vstring vStr(TTW(790489));
			vStr.Replace(L"#NAME#", wstrText->c_str());
			pMain->Text(vStr);
		}
	}
};
