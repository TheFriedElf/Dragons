#include "StdAfx.h"
#include "lwUIPet.h"
#include "lwUI.h"
#include "lwWorld.h"
#include "lwMarket.h"
#include "lwUICharInfo.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "PgChatMgrClient.h"
#include "PgNetWork.h"
#include "PgSoundMan.h"
#include "PgMyActorViewMgr.h"
#include "bm/Pgfilterstring.h"
#include "PgCommandMgr.h"

int const STATE_CHANGE_LIMIT_PERCENT_VALUE = 3000; // 만분률 값임.

extern void	Recv_PT_M_C_NFY_STATE_CHANGE2(PgPilot *pkPilot, E_SENDABIL_TARGET eType, SAbilInfo const &rkAbilInfo);
PgPet* GetMySelectedPet(void);

namespace lwPetOptChanger
{
	void lwInitDyeingEffect(void);
}

namespace PgPetUIUtil
{
	void lwSendReqFeedPet(bool bIsTrueSend, int iIndex = 0)
	{
		g_kPetUIMgr.SendReqFeedPet(bIsTrueSend, iIndex);
	}

	void lwDisplayPetIconForFeed(lwUIWnd kWnd)
	{
		if(false==kWnd.IsNil())
		{
			g_kPetUIMgr.DisplaySrcItem(kWnd(), kWnd.GetParent().GetCustomData<int>());
		}
	}

	EPetType GetPetType(PgItem_PetInfo const *pkPetInfo)
	{
		GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
		PgClassPetDef kDef;
		if(false==kClassPetDefMgr.GetDef(pkPetInfo->ClassKey(), &kDef))	
		{
			return EPET_TYPE_1;
		}

		return static_cast<EPetType>(kDef.GetPetType());
	}

	void Net_PT_C_M_REQ_ITEM_CHANGE_TO_PET(int iIndex = 0, bool const bUseItem = false)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return;}

		if(pkPlayer->SelectedPetID().IsNull()) {return;}

		SItemPos kItemPos;
		if(bUseItem)
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			if(!pkInv)	{return;}

			ContHaveItemNoCount	kItemCont;
			if(S_OK!=pkInv->GetItems(IT_PET, UICT_PET_COLORCHANGE, kItemCont, false) || kItemCont.empty())	
			{
				return;
			}

			if(S_OK!=pkInv->GetFirstItem((*kItemCont.begin()).first, kItemPos))
			{
				return;
			}

			PgPet* pkPet = GetMySelectedPet();
			if(!pkPet)	{return;}
			if( EPET_TYPE_1 == pkPet->GetPetType() )
			{//! 1차펫이라면 사용불가
				lwAddWarnDataTT(7512);
				return;
			}
		}

		iIndex = std::min(iIndex, 12);//12개 까지만 지원한다

		PgInventory *pInv = pkPlayer->GetInven();
		if(!pInv){return;}

		SItemPos const kPos = PgItem_PetInfo::ms_kPetItemEquipPos;
		PgBase_Item kBaseItem;
		if(S_OK==pInv->GetItem( kPos, kBaseItem))
		{
			BM::Stream kPacket(PT_C_M_REQ_PET_COLOR_CHANGE);
			kPacket.Push(kPos);
			kPacket.Push(static_cast<char>(iIndex));
			if(bUseItem)
			{
				kPacket.Push(kItemPos);
			}

			NETWORK_SEND(kPacket)
			lwPetOptChanger::lwInitDyeingEffect();
		}
	}

	void Net_PT_C_M_REQ_PET_RENAME(lwPacket kPacket, lwWString kEditText, lwUIWnd kWnd, bool const bUseItem = false)
	{
		if(!kPacket()) {return;}
		
		kWnd.ClearCustomData();
		
		BM::Stream kNewPacket(PT_C_M_REQ_PET_RENAME);
		kNewPacket.Push( *(kPacket()) );
		kNewPacket.Push(kEditText());

		if(bUseItem)
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return;}
			
			PgInventory* pkInv = pkPlayer->GetInven();
			if(!pkInv)	{return;}

			SItemPos kItemPos;
			ContHaveItemNoCount	kItemCont;
			if(S_OK!=pkInv->GetItems(IT_PET, UICT_PET_RENAME, kItemCont, false) || kItemCont.empty())	
			{
				return;
			}

			if(S_OK!=pkInv->GetFirstItem((*kItemCont.begin()).first, kItemPos))
			{
				return;
			}

			kNewPacket.Push(kItemPos);
		}

		kWnd.SetCustomDataAsPacket(lwPacket(&kNewPacket));
	}

	void lwUI_RefreshPetSkillUI(lwUIWnd kWnd)
	{
		if(kWnd.IsNil())
		{
			return;
		}

		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}

		bool bDeadPet = false;
		PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
		if(!pkPetPilot || !pkPetPilot->GetUnit())
		{
			bDeadPet = true;
		}

		PgMySkill* pkMySkill = NULL;
		if( !bDeadPet )
		{
			PgControlUnit* pkConUnit = dynamic_cast<PgControlUnit*>(pkPetPilot->GetUnit());
			if(!pkConUnit)
			{
				return;
			}

			pkMySkill = pkConUnit->GetMySkill();
			if(!pkMySkill)
			{
				return;
			}
		}

		PgInventory *pInv = pkPlayer->GetInven();
		if(!pInv){return;}
		PgBase_Item kItem;
		if(S_OK!=pInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem))
		{
			return;
		}

		PgItem_PetInfo *pkPetInfo = NULL;
		if(false == kItem.GetExtInfo(pkPetInfo))
		{
			return;
		}

		GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
		PgClassPetDef kPetDef;
		if ( !kClassPetDefMgr.GetDef( pkPetInfo->ClassKey(), &kPetDef ) )
		{
			return;
		}

		CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
		g_kTblDataMgr.GetContDef(pkDefPetSkill);

		CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID() );
		if ( skill_itr != pkDefPetSkill->end() )
		{
			XUI::CXUI_Wnd* pkBg = pkWnd->GetControl(L"FRM_SKILL");
			if(pkBg)
			{
				CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
				size_t const index = kContElement.find( PgDefClassPetSkillFinder(static_cast<short>(pkPetInfo->GetAbil(AT_LEVEL))) );
				if ( BM::PgApproximate::npos != index )
				{
					CONT_DEFCLASS_PET_SKILL::mapped_type::value_type const &kElement = kContElement.at(index);
					int iResNo = 0;
					bool bGray = true;

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					for ( size_t i=0 ; i<MAX_PET_SKILLCOUNT; ++i )
					{
						iResNo = 0;
						int iSkillNo = kElement.iSkillNo[i];
						if( 0 < iSkillNo )
						{
							CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
							if(pkSkillDef)
							{
								iResNo = pkSkillDef->RscNameNo();
								bGray = (true == bDeadPet)?(bDeadPet):(!pkMySkill->IsExist(iSkillNo));
							}
						}

						BM::vstring kName(L"ICN_SKILL");
						kName+=i;
						XUI::CXUI_Wnd* pkIcon = pkBg->GetControl((std::wstring const&)kName);
						if(pkIcon)
						{
							lua_tinker::call<void, lwUIWnd, int, int, bool>("UI_RefreshSkillIconPet", pkIcon, iSkillNo, iResNo, bGray);
						}
					}
				}
			}
		}
	}

	XUI::CXUI_Wnd* CallFeedResultUI(std::wstring const& rkText, wchar_t const* szWndName = L"SFRM_PET_FEED_RESULT")
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(szWndName);//에러메세지 출력
		if(!pkWnd)	{return NULL;}
		XUI::CXUI_Wnd* pkSdw = pkWnd->GetControl(L"SFR_SDW");
		if(!pkSdw)	{return NULL;}
		pkSdw->Text(rkText);
		lua_tinker::call<void, lwUIWnd>("UI_T_Y_ToCenter", lwUIWnd(pkSdw));	//y축 맞춤
		return pkWnd;
	}

	bool IsUsablePetTrainItem(PgInventory* pkInv, EUseItemPetFeedType eIndex, int const iCustomData = 0)
	{
		PgPet* pkPet = GetMySelectedPet();
		if(!pkPet)	{return false;}
		int const iNowValue = pkPet->GetAbil(eIndex==PET_FEED_TRAINING? AT_HEALTH : AT_MENTAL);
		if(iNowValue && iCustomData!=iNowValue)
		{
			return false;
		}

		return true;
/*		PgBase_Item kOutItem;
		if(S_OK!=pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kOutItem))
		{
			return false;
		}
		PgItem_PetInfo *pkPetInfo = NULL;
		if(false == kOutItem.GetExtInfo(pkPetInfo))	{return false;}

		if(EPET_TYPE_1 == GetPetType(pkPetInfo))	
		{
			lwAddWarnDataTT(358);
			return false;
		}

		bool bRet = true;
		std::wstring kErr = TTW(335);
		if(PET_FEED_REVIVE==eIndex)
		{
			bRet = pkPetInfo->IsDead();
		}
		else
		{
			PgPet* pkPet = GetMySelectedPet();
			if(!pkPet)	{return false;}
			if(PgItem_PetInfo::MAX_PET_STATE_VALUE<=pkPet->GetAbil(4100+eIndex))
			{
				bRet = false;
				kErr.clear();
				WstringFormat(kErr, MAX_PATH, TTW(337).c_str(), TTW(320+eIndex).c_str());
			}
		}

		if(!bRet)
		{
			XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( L"SFRM_PET_FEED_RESULT");
			if(pkWnd)
			{
				XUI::CXUI_Wnd *pkSdw = pkWnd->GetControl(L"SFR_SDW");
				if(pkSdw)
				{
					pkSdw->Text(kErr);
					lua_tinker::call<void, lwUIWnd>("UI_T_Y_ToCenter", lwUIWnd(pkSdw));	//y축 맞춤
				}
			}
		}

		return bRet;
*/
	}

	void lwUsePetTrainItem(int iIndex)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )	{return;}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )	{return;}

		if(false==IsUsablePetTrainItem(pkInv, static_cast<EUseItemPetFeedType>(iIndex)))	{return;}

		ContHaveItemNoCount	kItemCont;
		if(S_OK!=pkInv->GetItems(IT_PET, UICT_PET_FEED, kItemCont))	
		{
			CallFeedResultUI(TTW(328));
			return;
		}
		
		GET_DEF(CItemDefMgr, kItemDefMgr);
		ContHaveItemNoCount	kCorrectItemCont;
		ContHaveItemNoCount::iterator it = kItemCont.begin();
		while(kItemCont.end()!=it)
		{
			CItemDef const *pItemDef = kItemDefMgr.GetDef((*it).first);
			if(pItemDef)
			{
				int const iCustomValue2 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
				if(	iIndex == iCustomValue2 )
				{
					kCorrectItemCont.insert(std::make_pair((*it).first, (*it).second));
				}
			}
			++it;
		}

		if(kCorrectItemCont.empty())
		{
			CallFeedResultUI(TTW(328));
		}
		else
		{
			if(1<kCorrectItemCont.size())
			{ 
				UIItemUtil::CONT_CUSTOM_PARAM	kParam;
				UIItemUtil::CONT_CUSTOM_PARAM_STR kParamStr;
				kParam.insert(std::make_pair(L"Index", iIndex));
				kParam.insert(std::make_pair(L"Pet_Item", 1));
				kParam.insert(std::make_pair(L"CallYesNoBox", 0));
				UIItemUtil::CallCommonUseCustomTypeItems(kCorrectItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, kParamStr);
			}
			else	//한개일 때
			{
				ContHaveItemNoCount::const_iterator it = kCorrectItemCont.begin();
				SItemPos kFeedItemPos;
				if(S_OK==pkInv->GetFirstItem((*it).first, kFeedItemPos, false))
				{
					g_kPetUIMgr.SetFeedItem(kFeedItemPos, iIndex);
					g_kPetUIMgr.SetSrcItem(SItemPos(KUIG_FIT, EQUIP_POS_PET), iIndex);
					g_kPetUIMgr.SendReqFeedPet(true, iIndex);
				}
			}
		}
	}

	void lwTickTwinkleBtn(lwUIWnd UISelf, int const iType)
	{
/*
		XUI::CXUI_Wnd* pkWnd = UISelf.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		std::wstring kText;
		switch( iType )
		{
		case PET_FEED_TYPE_2:	
		case PET_FEED_TRAINING:	
		case PET_FEED_PLAYING:	
			{
				int const iTTNo = 343 + (iType - 1);
				WORD const iAbilType = AT_HUNGER + (iType - 1);

				PgPet* pkPet = GetMySelectedPet();
				if( pkPet )
				{
					int iAbilValue = pkPet->GetAbil(iAbilType);

					if( iAbilValue < STATE_CHANGE_LIMIT_PERCENT_VALUE )
					{
						if( !pkWnd->IsTwinkle() )
						{
							pkWnd->SetTwinkle(true);
						}
					}
					else
					{
						if( pkWnd->IsTwinkle() )
						{
							pkWnd->SetTwinkle(false);
							pkWnd->TwinkleStartTime(0);
							pkWnd->TwinkleInterTime(0);
							pkWnd->TwinkleTime(0);
						}
					}
				}
				else
				{
					if( pkWnd->IsTwinkle() )
					{
						pkWnd->SetTwinkle(false);
						pkWnd->TwinkleStartTime(0);
						pkWnd->TwinkleInterTime(0);
						pkWnd->TwinkleTime(0);
					}
					return;
				}
			}break;
		case PET_FEED_REVIVE:	
			{
				PgPet* pkPet = GetMySelectedPet();
				if( pkPet )
				{
					if( pkWnd->IsTwinkle() )
					{
						pkWnd->SetTwinkle(false);
						pkWnd->TwinkleStartTime(0);
						pkWnd->TwinkleInterTime(0);
						pkWnd->TwinkleTime(0);
					}
				}
				else
				{
					if( !pkWnd->IsTwinkle() )
					{
						pkWnd->SetTwinkle(true);
					}
				}

			}break;
		default:
			return;		
		}
*/
	}

	void lwCallPetStatusToolTip(int const iType, lwPoint2 kPoint)
	{
		std::wstring kText;
		switch( iType )
		{
		case 0://차펫은 없으니까 그냥 쓰자
			{
				PgPet* pkPet = GetMySelectedPet();
				if( pkPet && (pkPet->GetPetType() == EPET_TYPE_2 || pkPet->GetPetType() == EPET_TYPE_3) )
				{
					kText = TTW(360);
				}
				else
					return;
			}break;
		/*
		case PET_FEED_TYPE_2:	
		case PET_FEED_TRAINING:	
		case PET_FEED_PLAYING:	
			{
				int iAbilValue = 0;
				int const iTTNo = 343 + (iType - 1);
				WORD const iAbilType = AT_HUNGER + (iType - 1);
				PgPet* pkPet = GetMySelectedPet();
				if( pkPet )
				{
					iAbilValue = pkPet->GetAbil(iAbilType);
				}
				else
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
					if( pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) != S_OK )
					{
						return;
					}

					PgItem_PetInfo *pkPetInfo = NULL;
					if( !kItem.GetExtInfo( pkPetInfo ) )
					{
						return;
					}

					iAbilValue = pkPetInfo->GetAbil(iAbilType);
				}
				int const iPercent = iAbilValue / 100;
				wchar_t szTemp[1024] = {0,};
				swprintf_s(szTemp, 1024, TTW(iTTNo).c_str(), iPercent);
				kText = szTemp;
			}break;
		case PET_FEED_REVIVE:	
			{ 
				kText = TTW(346); 
			}break;
			*/
		default:
			return;		
		}
		lwCallMutableToolTipByText(lwWString(kText), kPoint);
	}

	void lwDrawTextToPetState(lwUIWnd kWnd = lwUIWnd(NULL))
	{
		XUI::CXUI_Wnd* pkPetUIInfo = kWnd.GetSelf();
		if( !pkPetUIInfo )
		{
			XUI::CXUI_Wnd *pkInfo = XUIMgr.Get(L"CharInfo");
			if(!pkInfo || pkInfo->IsClosed())	//떠 있을때만 해주면 됨
			{
				return;
			}

			pkPetUIInfo = pkInfo->GetControl(L"FRM_PET_INFO");
			if( !pkPetUIInfo )
			{
				return;
			}
		}

		XUI::CXUI_Wnd* pkDrawTextArea = pkPetUIInfo->GetControl(L"FRM_STATE");
		if( !pkDrawTextArea )
		{
			return;
		}

		XUI::CXUI_Wnd* pkDieUI = pkPetUIInfo->GetControl(L"FRM_DIE");
		if( !pkDieUI )
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
		if( pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) != S_OK )
		{
			return;
		}

		//여긴 죽은 놈이다.
		PgItem_PetInfo *pkPetInfo = NULL;
		if( !kItem.GetExtInfo( pkPetInfo ) )
		{
			return;
		}

		wchar_t szTemp[1024] = {0,};
		if( !kItem.IsUseTimeOut() )
		{
			PgPet* pkPet = GetMySelectedPet();
			if( !pkPet ){ return; }

			if( pkDieUI->Visible() )
			{
				pkDieUI->Visible(false);
				XUI::CXUI_Button* pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPetUIInfo->GetControl(L"BTN_TURN_LEFT"));
				if( pkButton )
				{
					pkButton->Disable(false);
				}
				pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPetUIInfo->GetControl(L"BTN_TURN_RIGHT"));
				if( pkButton )
				{
					pkButton->Disable(false);
				}
			}

			//if( pkPet->GetPetType() == EPET_TYPE_1 )	//죽어있는 1차펫을 착용한 상태에서 2차펫을 착용하면 죽음 텍스트가 초기화 되지 않는다.
			{
				pkDrawTextArea->Text(L"");
				return;
			}

			//여긴 산놈이다.
/*			std::wstring kDrawText;

 			swprintf_s(szTemp, MAX_PATH, TTW(353).c_str(), TTW(321).c_str(), (pkPet->GetAbil(AT_HUNGER) / 100) );
 			kDrawText = szTemp;
 			kDrawText += L"\n";
			swprintf_s(szTemp, MAX_PATH, TTW(353).c_str(), TTW(322).c_str(), pkPet->GetAbil(AT_HEALTH) );
			kDrawText += szTemp;
			kDrawText += L"\n";
			swprintf_s(szTemp, MAX_PATH, TTW(353).c_str(), TTW(323).c_str(), pkPet->GetAbil(AT_MENTAL) );
			kDrawText += szTemp;
			pkDrawTextArea->TextPos( POINT2( 5, 5 ) );
			pkDrawTextArea->FontFlag(pkDrawTextArea->FontFlag() & ~XTF_ALIGN_CENTER);
			pkDrawTextArea->Text(kDrawText);*/
		}
		else
		{
			swprintf_s(szTemp, 1024, TTW(354).c_str(), pkPetInfo->Name().c_str());
			pkDrawTextArea->TextPos( POINT2( pkDrawTextArea->Size().x * 0.5f, 5 ) );
			pkDrawTextArea->FontFlag(pkDrawTextArea->FontFlag() | XTF_ALIGN_CENTER);
			pkDrawTextArea->Text(szTemp);

			pkDieUI->Visible(true);
			XUI::CXUI_Button* pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPetUIInfo->GetControl(L"BTN_TURN_LEFT"));
			if( pkButton )
			{
				pkButton->Disable(true);
			}
			pkButton = dynamic_cast<XUI::CXUI_Button*>(pkPetUIInfo->GetControl(L"BTN_TURN_RIGHT"));
			if( pkButton )
			{
				pkButton->Disable(true);
			}
		}
	}

/*	bool lwOnChangePetAbil(PgPilot* pkPetPilot, bool bExist_AT_TIME)
	{
		if(!pkPetPilot)	{return false;}
		PgPet* pkPetUnit = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
		if(!pkPetUnit)	{return false;} 

		if( true == bExist_AT_TIME)
		{
 			if( 0 > pkPetUnit->GetAbil(AT_TIME) )
 			{
 				lwAddWarnDataTT(338);
 			}
		}
		else
		{
			PgActorPet* pkActorPet = dynamic_cast<PgActorPet*>(g_kPilotMan.FindActor(pkPetPilot->GetGuid()));
			__int64 i64OldDieTime = pkPetUnit->GetPeriodEndSecTime();
			E_PET_CONDITION_CHANGE eChange = pkPetUnit->CheckConditionChange();
			if(PET_CONDITION_CHANGE_BAD == eChange)
			{//컨디션이 악화될 때
				lwAddWarnDataTT(341);
				if(pkActorPet)
				{
					g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Pet_Power_Down", 0.0f, 0.0f, 0.0f, pkActorPet);
				}
			}
			else if(PET_CONDITION_CHANGE_RECOVERY == eChange)
			{
				if(pkActorPet)
				{
					g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Pet_Power_UP", 0.0f, 0.0f, 0.0f, pkActorPet);
				}
			}
			else
			{
				if(PgItem_PetInfo::PET_FATIGUE_CONDITION>CalcPetStateAvg(pkPetUnit))	//상태변화가 없는데도 30%이하이면
				{
					__int64 const i64DieTime = pkPetUnit->GetPeriodEndSecTime();
					__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
					__int64 const i64OldDelta = i64CurTime - i64OldDieTime;
					__int64 const i64Delta = i64CurTime - i64DieTime;

					__int64 const i64Hour = CGameTime::HOUR / CGameTime::SECOND;
					if( i64Hour * 2i64 <= i64OldDieTime )
					{
						if( i64Hour * 2i64 > i64DieTime)
						{//죽기까지 두시간전
							std::wstring kText;
							WstringFormat( kText, MAX_PATH, TTW(356).c_str(), 2 );
						}
					}
					else if ( i64Hour <= i64OldDieTime )
					{
						if( i64Hour > i64DieTime )
						{//죽기까지 한시간전
							std::wstring kText;
							WstringFormat( kText, MAX_PATH, TTW(356).c_str(), 1 );
						}
					}
				}
			}

			int const iCon = pkPetUnit->GetConditionGrade();
			int const iOldCon = pkPetUnit->GetConditionGrade(false);
			
			if(iCon!=iOldCon)
			{
				if(pkActorPet)
				{
					pkActorPet->MakePetActionQueue();	//액션을 새로 구성
					g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, iCon<iOldCon ? "Pet_Power_UP" : "Pet_Power_Down", 0.0f, 0.0f, 0.0f, pkActorPet);
				}
				pkPetUnit->SetOldConditionGrade(iCon);
			}
		}

		XUI::CXUI_Wnd *pkInfo = XUIMgr.Get(L"CharInfo");
		if(pkInfo)	//떠 있을때만 해주면 됨
		{
			lwCharInfo::SetPetInfoToUI(pkInfo->GetControl(L"FRM_PET_INFO"), pkPetPilot->GetGuid(), true);//UI가 떠 있을 때에는 UI 모델의 기분 파티클을 변경해야 함. 상태값 재계산
		}

		return true;

		//__int64 i64Old = static_cast<__int64>(pkPetPilot->GetAbil(rkAbil.Type()));
		//float fDelta = (rkAbil.Value() - i64Old)/100.0f;
		//std::wstring const kNum = lwMarketUtil::FloatConvertToStr(fabs(fDelta), 2);

		//std::wstring kText;
		//WstringFormat( kText, MAX_PATH, TTW(326).c_str(), pkPetPilot->GetName().c_str(), TTW(rkAbil.Type()-4100+320).c_str(), kNum.c_str(), TTW(fDelta>0.0f ? 324 : 325).c_str() );
		//SChatLog kChatLog(CT_EVENT_SYSTEM);
		//g_kChatMgrClient.AddLogMessage(kChatLog, kText);//메세지 표시 안함
	}*/

	lwWString lwGetPetRenamePay()
	{
		return lwWString(GetMoneyString(PgPet::ms_i64RenamePetMoney, true, false));
	}

	lwWString lwGetPetDyeingPay()
	{
		return lwWString(GetMoneyString(PgPet::ms_i64ChangeColorPetMoney, true, false));
	}

	lwInt64 lwGetPetRenamePayInt64()
	{
		return lwInt64(PgPet::ms_i64RenamePetMoney);
	}

	lwInt64 lwGetPetDyeingPayInt64()
	{
		return lwInt64(PgPet::ms_i64ChangeColorPetMoney);
	}

	void PetSkillToSkillTree( SClassKey const &kPetClassKey )
	{
		GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
		PgClassPetDef kPetDef;
		if ( !kClassPetDefMgr.GetDef( kPetClassKey, &kPetDef ) )
		{
			return;
		}

		CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
		g_kTblDataMgr.GetContDef(pkDefPetSkill);

		PgPet* pkMyPet = GetMySelectedPet();
		PgActorPet* pkActorPet = NULL;
		if(NULL!=pkMyPet)
		{
			pkActorPet = dynamic_cast<PgActorPet*>(g_kPilotMan.FindActor(pkMyPet->GetID()));
		}
		if(NULL==pkActorPet)	{return;}
		pkActorPet->ClearSkillContForAI();
		g_kSkillTree.DeleteAllPetSkill();

		CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID() );
		if ( skill_itr != pkDefPetSkill->end() )
		{
			CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
			size_t const index = kContElement.find( PgDefClassPetSkillFinder(kPetClassKey.nLv) );
			if ( BM::PgApproximate::npos != index )
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CONT_DEFCLASS_PET_SKILL::mapped_type::value_type const &kElement = kContElement.at(index);
				for ( size_t i=0 ; i<MAX_PET_SKILLCOUNT; ++i )
				{
					int const iSkillNo = kElement.iSkillNo[i];
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					if(pkSkillDef == NULL) { continue; }

					if ( iSkillNo > 0 )
					{
						g_kSkillTree.AddPetSkillToTree(iSkillNo, pkSkillDef);
					}
					if(pkSkillDef->GetAbil(AT_PET_SKILL_FOR_AI))
					{ //이 스킬을 AI에서 사용할 것인가? (TB_DefSkillAbil_Pet 테이블에 저장)
						pkActorPet->AddSkillToContForAI(iSkillNo);
					}
				}
			}
		}
	}

	bool PetSkillToSkillTree( PgPlayer* pkPlayer )
	{
		if(NULL==pkPlayer || NULL==pkPlayer->GetInven())	{return false;}

		PgBase_Item kItem;
		if(S_OK!=pkPlayer->GetInven()->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem))	{return false;}

		if(kItem.IsEmpty())	{return false;}

		PgItem_PetInfo* pkInfo = NULL;
		if(false==kItem.GetExtInfo(pkInfo))	{return false;}
		
		PetSkillToSkillTree(pkInfo->ClassKey());
		return true;
	}

	XUI::CXUI_Wnd* SetPetUIDefaultState(bool bEnableCover)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"CharInfo");
		if( pMainUI )
		{
			XUI::CXUI_Wnd* pPetUI = pMainUI->GetControl(L"FRM_PET_INFO");
			if( pPetUI )
			{
				XUI::CXUI_Wnd* pNotExistCover = pPetUI->GetControl(L"FRM_NOT_EXIST_PET_COVER");
				if( pNotExistCover )
				{
					pNotExistCover->Visible(bEnableCover);
					pNotExistCover->Text(TTW(146));//기본 글자 셋팅
					return pNotExistCover;
				}
			}
		}

		return NULL;
	}

	XUI::CXUI_Wnd* SetPetUIDefaultState(PgBase_Item const& rkItem)
	{
		bool bDisable = true;
		int iTTW = 0;
		//GATTR_FLAG_NOPVP
		if(g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP_ABLE))
		{
			if(rkItem.IsUseTimeOut() )
			{
				bDisable = false;
			}
		}
		else
		{
			bDisable = true;	//PVP에서는 무조건 가림
			iTTW = 149;
		}
		XUI::CXUI_Wnd* pkCover = SetPetUIDefaultState(bDisable);
		if(iTTW && pkCover)
		{
			pkCover->Text(TTW(iTTW));
		}

		return pkCover;
	}

	bool IsExistPetSkill(int const iSkillNo)
	{
		if(0>iSkillNo)	{return false;}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if(!pkSkillDef)	{return false;}
		PgPet* pkPet = GetMySelectedPet();
		if(!pkPet)		{return false;}	//펫없음
		if(pkPet->GetAbil(AT_MP)<pkSkillDef->GetAbil(AT_NEED_MP)){return false;}//MP부족
		if(!pkPet->GetMySkill())	{return false;}
		
		return pkPet->GetMySkill()->IsExist(iSkillNo);
	}

	void ChangeBalloonState(bool const bIsHide, int const SlotNo = 1)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"CharInfo");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			return;
		}

		XUI::CXUI_Wnd* pPetUIPage = pMainUI->GetControl(L"FRM_PET_INFO");
		if( !pPetUIPage || !pPetUIPage->Visible() )
		{
			return;
		}

		XUI::CXUI_Wnd* pBalloon = pPetUIPage->GetControl(L"IMG_BALLOON");
		if( !pBalloon )
		{
			return;
		}

		if( !bIsHide )
		{
			pBalloon->UVUpdate(SlotNo);
		}
		pBalloon->Visible(!bIsHide);
	}

	void lwShowBalloonToIdleActionChange(int const iActionSlotNo)
	{
		ChangeBalloonState(false, iActionSlotNo);
	}

	void lwHideBalloonToIdleActionChange()
	{
		ChangeBalloonState(true);
	}

	int lwCheckPetNameFiltering(lwWString kString)
	{
		if(g_kClientFS.Filter(kString.GetWString(), false, FST_ALL)
			|| !g_kUnicodeFilter.IsCorrect(UFFC_CHARACTER_NAME, kString.GetWString())
			)	//욕설 등이 있으면
		{
			::Notice_Show( TTW(600037), 0);
			return 1;
		}

		PgPet* pkPet = GetMySelectedPet();
		if( !pkPet || kString.GetWString() == pkPet->Name() )
		{
			return 2;
		}
		return 0;
	}

	int lwCheckPetColorChange(int const iColorIndex, bool const bUseItem = false)
	{
		PgPet* pkPet = GetMySelectedPet();
		if( !pkPet )
		{
			return 0;
		}

		int const iPetColor = pkPet->GetAbil(AT_COLOR_INDEX);
		if( iPetColor == iColorIndex )
		{
			return 0;
		}

		SItemPos kItemPos;
		if(bUseItem)
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return 1;}
			PgInventory* pkInv = pkPlayer->GetInven();
			if(!pkInv)	{return 1;}

			ContHaveItemNoCount	kItemCont;
			if(S_OK!=pkInv->GetItems(IT_PET, UICT_PET_COLORCHANGE, kItemCont, false) || kItemCont.empty())	
			{
				return 1;
			}

			if(S_OK!=pkInv->GetFirstItem((*kItemCont.begin()).first, kItemPos))
			{
				return 1;
			}
		}
		return 10;
	}

	bool lwCheckPetNameChange(bool const bUseItem)
	{
		PgPet* pkPet = GetMySelectedPet();
		if( !pkPet )
		{
			return false;
		}

		if(bUseItem)
		{
			SItemPos kItemPos;
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return false;}
			PgInventory* pkInv = pkPlayer->GetInven();
			if(!pkInv)	{return false;}

			ContHaveItemNoCount	kItemCont;
			if(S_OK!=pkInv->GetItems(IT_PET, UICT_PET_RENAME, kItemCont, false) || kItemCont.empty())	
			{
				return false;
			}

			if(S_OK!=pkInv->GetFirstItem((*kItemCont.begin()).first, kItemPos))
			{
				return false;
			}
		}

		return true;
	}

	void lwCallPetUI()
	{
		bool bCallPetUI = false;
		XUI::CXUI_Wnd* pkCharInfo = XUIMgr.Get(L"CharInfo");
		if( pkCharInfo )
		{
			XUI::CXUI_Wnd* pkPetInfo = pkCharInfo->GetControl(L"FRM_PET_INFO");
			if( pkPetInfo )
			{
				bCallPetUI = !pkPetInfo->Visible();
			}
		}
		else
		{
			pkCharInfo = XUIMgr.Call(L"CharInfo");
			if( pkCharInfo )
			{
				bCallPetUI = true;
			}
		}

		if( bCallPetUI )
		{
			XUI::CXUI_Wnd* pkPetBtn = pkCharInfo->GetControl(L"CBTN_TAB_1");
			if( pkPetBtn )
			{
				lwCharInfo::lwChangeInfoTab(lwUIWnd(pkPetBtn), 1);
				lwViewPetEquip();
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkPlayer )
				{
					XUI::CXUI_Wnd* pkPetInfo = pkCharInfo->GetControl(L"FRM_PET_INFO");
					if( pkPetInfo )
					{
						lwCharInfo::SetPetInfoToUI(pkPetInfo, pkPlayer->SelectedPetID(), false);
					}
				}
			}
		}
		else
		{
			pkCharInfo->Close();
		}
	}

	void UpdatePetEquipIconBgText(bool const bEquip, int const iIconKey)
	{
		int iIndex = 0;
		int iIndex_Build = 0;
		switch( iIconKey )
		{
		case 23:{ iIndex = 0; iIndex_Build = 0; }break;
		case 24:{ iIndex = 1; iIndex_Build = 1; }break;
		case 20:{ iIndex = 2; iIndex_Build = 2; }break;
		case 0:{ iIndex = 3; iIndex_Build = 3; }break;			//물약
		case 6:{ iIndex = 70006 - 126; iIndex_Build = 0;}break;	//무기
		case 8:{ iIndex = 70008 - 126; iIndex_Build = 1;}break;	//목걸이
		case 9:{ iIndex = 70009 - 126; iIndex_Build = 2;}break;	//귀걸이
		case 10:{ iIndex = 70010 - 126; iIndex_Build = 3;}break;	//반지
		}
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(L"CharInfo");
		if( !pkMain )
		{
			return;
		}

		XUI::CXUI_Wnd* pkPetInfo = pkMain->GetControl(L"FRM_PET_INFO");
		if( !pkPetInfo )
		{
			return;
		}

		BM::vstring vStr(L"IMG_COSTUM_ICON_BG");
		if(3<iIndex)
		{
			vStr+=L"2_";	//악세사리 류
		}
		vStr += iIndex_Build;
		XUI::CXUI_Wnd* pkIconBg = pkPetInfo->GetControl(vStr);
		if( !pkIconBg )
		{
			return;
		}

		XUI::CXUI_Wnd* pkText = pkIconBg->GetControl(L"IMG_COSTUM");
		if(!pkText)	{pkText = pkIconBg;}	//아이콘모양의 배경이 없음
		if( pkText )
		{
			if( bEquip )
			{
				pkText->Text(L"");
			}
			else
			{
				pkText->Text(TTW(126 + iIndex));
			}
		}
	}

	void SetPetAbilValue(XUI::CXUI_Wnd* pWnd, EAbilType Type, int iValue, wchar_t const* pText)
	{
		if( !pText )
		{
			std::wstring AbilName;
			switch(Type)
			{
			case AT_STR:
				{
					AbilName = TTW(790658);
				}break;
			case AT_INT:
				{
					AbilName = TTW(790659);
				}break;
			case AT_CON:
				{
					AbilName = TTW(790660);
				}break;
			case AT_DEX:
				{
					AbilName = TTW(790661);
				}break;
			default:
				{
					MakeAbilNameString2(Type, AbilName);
				}break;
			}
			pWnd->Text(AbilName);
		}
		else
		{
			pWnd->Text(pText);
		}

		XUI::CXUI_Wnd* pAbliValue = pWnd->GetControl(_T("FRM_VALUE"));
		if( !pAbliValue )
		{
			return;
		}

		BM::vstring vStr(L"{C=0xFF008E21/}");
		std::wstring wstrText;
		if(1<pWnd->Text().size())	//정상적인 내용이 들어 있을 경우에만
		{
			switch(Type)
			{
			case AT_HIT_SUCCESS_VALUE:
			case AT_DODGE_SUCCESS_VALUE:
				{// % 단위가 붙는 항목들, %단위에 맞게 맞춘다. (맨뒤에 %붙이지 않는 어빌)
					float const fRate = iValue / (ABILITY_RATE_VALUE * 0.01f);
					wchar_t szTemp[MAX_PATH] = {0,};
					swprintf(szTemp, MAX_PATH, _T("%.0f"), fRate);
					wstrText += szTemp;
					vStr += wstrText;
				}break;
			case AT_R_CRITICAL_POWER:
				{}break;//해당 어빌은 사용하지 않는다.
			case AT_PHY_DMG_100PERECNT_REFLECT_RATE://= 3914,	// 물리 Damage 100% 반사 확률( 만분률 )// 여기에 값이 데미지 반사량(%)
			case AT_R_MAX_HP:
			case AT_R_HP_RECOVERY_INTERVAL:
			case AT_R_MAX_MP:
			case AT_R_MP_RECOVERY_INTERVAL:
			case AT_R_STR:
			case AT_R_INT:
			case AT_R_CON:
			case AT_R_DEX:
			case AT_R_MOVESPEED:
			case AT_R_PHY_DEFENCE:
			case AT_R_MAGIC_DEFENCE:
			case AT_R_ATTACK_SPEED:
			case AT_R_BLOCK_SUCCESS_VALUE:
			case AT_R_DODGE_SUCCESS_VALUE:
			case AT_R_CRITICAL_SUCCESS_VALUE:
			case AT_R_INVEN_SIZE:
			case AT_R_EQUIPS_SIZE:
			case AT_R_ATTACK_RANGE:
			case AT_R_HP_RECOVERY:
			case AT_R_MP_RECOVERY:
			case AT_R_JUMP_HEIGHT:
			case AT_R_PHY_ATTACK_MAX:
			case AT_R_PHY_ATTACK_MIN:
			case AT_R_NEED_MP:
			case AT_R_NEED_HP:
			case AT_R_MAGIC_ATTACK:
			case AT_R_PHY_ATTACK:
			case AT_R_CRITICAL_MPOWER:
			case AT_R_HIT_SUCCESS_VALUE:
			case AT_R_WALK_SPEED:
			case AT_R_MAGIC_ATTACK_MAX:
			case AT_R_MAGIC_ATTACK_MIN:
			case AT_R_PHY_DMG_DEC:
			case AT_R_MAGIC_DMG_DEC:
			case AT_R_CASTING_SPEED:
			case AT_R_RESIST_FIRE:
			case AT_R_RESIST_WATER:
			case AT_R_RESIST_TREE:
			case AT_R_RESIST_IRON:
			case AT_R_RESIST_EARTH:
			case AT_R_HP_POTION_ADD_RATE:
			case AT_R_MP_POTION_ADD_RATE:
			case AT_R_DMG_REFLECT_RATE:
			case AT_R_DMG_REFLECT_HITRATE:
			case AT_R_PENETRATION_RATE:
			case AT_R_MAX_HP_DEAD:
			case AT_R_MAX_MP_DEAD:
			case AT_R_ABS_ADDED_DMG_PHY:
			case AT_R_ABS_ADDED_DMG_MAGIC:
			case AT_PHY_DMG_DEC:
			case AT_MAGIC_DMG_DEC:
			case AT_ADD_R_NEED_HP:
			case AT_ADD_R_NEED_MP:
			case AT_CRITICAL_SUCCESS_VALUE:
			case AT_I_PHY_DEFENCE_ADD_RATE:
			case AT_I_MAGIC_DEFENCE_ADD_RATE:
				// 	case AT_R_MAX_DP:
				// 	case AT_R_DP_RECOVERY_INTERVAL:
				// 	case AT_R_DP_RECOVERY:
				{// % 단위가 붙는 항목들, %단위에 맞게 맞춘다.
					float const fRate = iValue / (ABILITY_RATE_VALUE * 0.01f);
					wchar_t szTemp[MAX_PATH] = {0,};
					swprintf(szTemp, MAX_PATH, _T("%.1f%%"), fRate);
					wstrText += szTemp;
					vStr += wstrText;
				}break;
			default:
				{
					vStr += iValue;
				}break;
			}
		}

		pAbliValue->Text(vStr);
		pAbliValue->SetCustomData(&Type, sizeof(Type));
	}

	void lwSetPetAbilInfo(BM::GUID const& rkGuid, XUI::CXUI_Wnd* pInfoMain, int iType, char const* szAddon)
	{
		if(rkGuid.IsNull() || NULL==pInfoMain || NULL==szAddon)
		{
			return;
		}

		PgPilot* pPilot = g_kPilotMan.FindPilot(rkGuid);
		if( !pPilot )
		{
			return;
		}

		PgPet* pkPetUnit = dynamic_cast<PgPet*>(pPilot->GetUnit());
		if( !pkPetUnit )
		{
			return;
		}

		PgPilot* pkCaller = g_kPilotMan.FindPilot(pkPetUnit->Caller());
		if(!pkCaller->GetUnit() || !pkCaller->GetUnit()->GetInven())
		{
			return;
		}

		PgInventory* pkInv = pkCaller->GetUnit()->GetInven();
		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) )
		{
			return;
		}

		pInfoMain->OwnerGuid(rkGuid);

		static EAbilType const kAbilList[ABIL_INFO_MAX_SLOT] = {AT_C_PHY_ATTACK_MAX, AT_C_MAGIC_ATTACK_MAX, AT_MAX_MP, AT_C_MP_RECOVERY};

		CONT_ENCHANT_ABIL kEnchantAbil;
		GetDefaultOption(kItem, kEnchantAbil, true);
		CONT_ENCHANT_ABIL::const_iterator kItor = kEnchantAbil.begin();

		for(int i = 0; i < ABIL_INFO_MAX_SLOT; ++i)
		{

			BM::vstring vStr(_T("FRM_INFO_L"));
			vStr += i;

			XUI::CXUI_Wnd* pAbilSlot = pInfoMain->GetControl(vStr);
			if( !pAbilSlot )
			{
				continue;
			}

			EAbilType iAbil = AT_NONE;
			if(iType==6)	// 기본정보일 경우
			{
				int iValue = 0;
				if( kItor != kEnchantAbil.end() )
				{
					iValue = (*kItor).iValue;
					if(iValue)
					{
						iAbil = static_cast<EAbilType>((*kItor).wType);
					}
					
					//HP/MP 회복속도만 '-' -> '+'로 보여지도록
					if(iAbil == AT_R_HP_RECOVERY_INTERVAL || iAbil == AT_R_MP_RECOVERY_INTERVAL)
					{
						iValue = (-iValue);
					}
					++kItor;
				}

				SetPetAbilValue(pAbilSlot, iAbil, iValue, iAbil==AT_NONE?L" " : NULL);
			}
			else
			{
				iAbil = kAbilList[i];
				lwCharInfo::SetAbilValue(pAbilSlot, rkGuid, iAbil, iAbil==AT_NONE?L" " : NULL);
			}
		}
	}

	float GetUseRateInfo(BM::GUID const& rkGuid, EAbilType kType)
	{
		PgPilot* pPilot = g_kPilotMan.FindPilot(rkGuid);
		if( !pPilot )	{return 0.0f;}

		if( !pPilot->GetUnit() ){return 0.0f;}

		PgInventory* pkInv = pPilot->GetUnit()->GetInven();
		if( !pkInv )	{return 0.0f;}

		static EAbilType const kArr1[] = {AT_R_MP_RECOVERY, AT_SKILL_EFFICIENCY};//만분율

		int iIdx = 0;

		switch(kType)
		{
		case AT_HEALTH:	{}break;
		case AT_MENTAL:	{iIdx = 1;}break;
		default:		{return 0.0f;}break;
		}

		float const fValue = static_cast<float>(pkInv->GetAbil(kArr1[iIdx])*0.01f);	//인벤
		float const fValue2 = static_cast<float>(pPilot->GetUnit()->GetAbil(kType));	//유닛

		return fValue+fValue2;
	}

	bool GetUseRateTimeInfo(BM::GUID const& rkGuid, EAbilType kType, std::wstring& rkOut)
	{
		PgPilot* pPilot = g_kPilotMan.FindPilot(rkGuid);
		if( !pPilot )	{return false;}

		PgPet* pkPet = dynamic_cast<PgPet*>(pPilot->GetUnit());

		if( !pkPet ){return false;}

		__int64 kSecTime = 0i64;

		switch(kType)
		{
		case AT_HEALTH:	{kSecTime = pkPet->GetHealthSecTime();}break;
		case AT_MENTAL:	{kSecTime = pkPet->GetMentalSecTime();}break;
		default:		{return false;}break;
		}

		__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::SECOND);

		__int64 kDelta = kSecTime - i64NowTime;

		if(0i64>=kDelta)	{return false;}

		BM::vstring kStr(TTW(120).c_str());
		kStr += L" ";

		__int64 i64Temp = CGameTime::OneDay / CGameTime::SECOND;
		__int64 i64Time = kDelta / i64Temp;
		bool bDisplayDay = false;
		bool bDisplayHour = false;
		bool bSpace = false;
		if( i64Time )			//일 단위 이상이면
		{
			kStr+=i64Time;
			kStr+=TTW(174);//일
			bSpace = true;
			bDisplayDay = true;
		}

		kDelta %= i64Temp;
		i64Temp = CGameTime::HOUR / CGameTime::SECOND;
		i64Time = kDelta / i64Temp;

		if( i64Time )			//시간 단위 이상이면
		{
			if(bSpace)
			{
				kStr+=L" ";
			}
			kStr+=i64Time;
			kStr+=TTW(90004);//시간
			bSpace = true;
			bDisplayHour = true;
		}

		kDelta %= i64Temp;
		i64Temp = CGameTime::MINUTE / CGameTime::SECOND;
		i64Time = kDelta / i64Temp;

		if(		!bDisplayDay
			&&	i64Time )			//분 단위 이상이면//날짜 단위가 없을 때만 나오자
		{
			if(bSpace)
			{
				kStr+=L" ";
			}

			if ( !bDisplayHour )
			{
				i64Time = std::max( i64Time, 1i64 );
			}

			kStr+= i64Time;
			kStr+=TTW(90003);//분	
		}

		rkOut.clear();

		rkOut = (std::wstring const&)kStr;

		return true;
	}

	void lwSetPetUseRateInfo(BM::GUID const& rkGuid, XUI::CXUI_Wnd* pInfoMain, int iType, char const* szAddon)
	{
		if(rkGuid.IsNull() || NULL==pInfoMain || NULL==szAddon)	{return;}
	
		int iCount = 0;
	
		for( int i = AT_HEALTH; i <= AT_MENTAL; ++i, ++iCount)
		{
			int const iValue = static_cast<int>(GetUseRateInfo(rkGuid, static_cast<EAbilType>(i)));	
			BM::vstring kStr(L"FRM_INFO_R");
			kStr+=iCount;
			XUI::CXUI_Wnd* pkInfoWnd = pInfoMain->GetControl((std::wstring const&)kStr);
			if(pkInfoWnd)
			{
				XUI::CXUI_Wnd* pkValueWnd = pkInfoWnd->GetControl(L"SFRM_VALUE");
				if(pkValueWnd)
				{
					BM::vstring kValue;
					kValue+=iValue;
					kValue+=L"%";
					if(0<iValue)
					{
						kValue+=TTW(420121);
					}
					pkValueWnd->Text((std::wstring const&)kValue);
					EAbilType kType = static_cast<EAbilType>(i);
					pkValueWnd->SetCustomData(&kType, sizeof(kType));
				}

				std::wstring kOut;
				if(GetUseRateTimeInfo(rkGuid, static_cast<EAbilType>(i), kOut))
				{
				}
				pkInfoWnd->Text(kOut);
			}
		}
	}

	void AutomaticUsePetItemConsumer( CUnit* pkMyPet, WORD const iType, int const iPrevValue, int const iIncreaseVal)
	{
		if(0<=iIncreaseVal)	{return;}	//증가된거면 리턴
		EUseItemPetFeedType eFeed_Type = PET_FEED_TYPE_1;
		EInvType eInvType = IT_NONE;
		PgInventory* pkInv = NULL;
		switch(iType)
		{
		case AT_MP:
			{
				eFeed_Type = PET_FEED_MP_DRINK;
				eInvType = IT_CONSUME;
				pkInv = pkMyPet->GetInven();
				if(0.5 < static_cast<float>(pkMyPet->GetAbil(AT_MP))/static_cast<float>(pkMyPet->GetAbil(AT_C_MAX_MP)))	//절반 이하일 때만
				{
					return;
				}
			}break;
		default:
			{
				return;//현재로썬 쓰는데가 없음
				/*PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer )		{return;}
				eInvType = IT_PET;
				pkInv = pkPlayer->GetInven();*/
			}break;
		}

		if( !pkInv )		{return;}
		
		ContHaveItemNoCount	kItemCont;
		if(S_OK!=pkInv->GetItems(eInvType, kItemCont))	{return;}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		ContHaveItemNoCount::iterator it = kItemCont.begin();
		while(kItemCont.end()!=it)
		{
			ContHaveItemNoCount::key_type const& kItemNo = it->first;
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemNo);
			if(pItemDef && pItemDef->IsType(ITEM_TYPE_PET) && pItemDef->IsType(ITEM_TYPE_CONSUME))	//펫용 소비 아이템
			{
				SItemPos kFeedItemPos;
				if( S_OK == pkInv->GetFirstItem(kItemNo, kFeedItemPos) )
				{
					BM::Stream kPacket(PT_C_M_REQ_PET_ACTION, kFeedItemPos);
					kPacket.Push(lwGetServerElapsedTime32());
					NETWORK_SEND(kPacket);
				}
			}
			++it;
		}
	}

	void InitPetInfoUI(XUI::CXUI_Wnd* pWnd)
	{
		if(NULL==pWnd)	{return;}
		XUI::CXUI_Wnd* pInfoWnd = pWnd->GetControl(L"FRM_INFO");
		if(pInfoWnd)
		{
			for(int i = 0; i<4; ++i)
			{
				BM::vstring kValue(L"FRM_INFO_L");
				kValue+=i;
				XUI::CXUI_Wnd* pInfoL = pInfoWnd->GetControl((std::wstring const&)kValue);
				if(pInfoL)
				{
					XUI::CXUI_Wnd* pkValue = pInfoL->GetControl(L"FRM_VALUE");
					if(pkValue)
					{
						pkValue->Text(L"");
						pkValue->ClearCustomData();
					}
				}
			}

			for(int i = 0; i<2; ++i)
			{
				BM::vstring kValue(L"FRM_INFO_R");
				kValue+=i;
				XUI::CXUI_Wnd* pInfoR = pInfoWnd->GetControl((std::wstring const&)kValue);
				if(pInfoR)
				{
					XUI::CXUI_Wnd* pkValue = pInfoR->GetControl(L"SFRM_VALUE");
					if(pkValue)
					{
						pkValue->Text(L"");
						pkValue->ClearCustomData();
					}
				}
			}
		}
		XUI::CXUI_Wnd* pkBg = pWnd->GetControl(L"FRM_SKILL");
		if(pkBg)
		{
			for ( size_t i=0 ; i<MAX_PET_SKILLCOUNT; ++i )
			{
				BM::vstring kName(L"ICN_SKILL");
				kName+=i;
				XUI::CXUI_Wnd* pkIcon = pkBg->GetControl((std::wstring const&)kName);
				if(pkIcon)
				{
					lua_tinker::call<void, lwUIWnd, int, int, bool>("UI_RefreshSkillIconPet", pkIcon, 0, 0, false);
				}
			}
		}
	}

	void SaveMyPetInventory(BM::Stream & rkPacket, BM::GUID const& rkGuid)
	{
		if ( !PgActorPet::ms_pkTempPetInventory )
		{
			PgActorPet::ms_pkTempPetInventory = new_tr PgInventory;
		}
		
		if ( !PgActorPet::ms_pkTempPetInventory )	{return;}

		PgActorPet::ms_pkTempPetInventory->OwnerGuid( rkGuid );
		PgActorPet::ms_pkTempPetInventory->ReadFromPacket( rkPacket, WT_DEFAULT );
		PgActorPet::ms_pkTempPetInventory->RefreshAbil(NULL);
	}

	bool LoadMyPetInventory(BM::GUID const& rkGuid, std::wstring const& rkName)
	{
		if ( !PgActorPet::ms_pkTempPetInventory || !PgActorPet::ms_pkMyPetInventory )	{return false;}

		if(PgActorPet::ms_pkTempPetInventory->OwnerGuid()!=rkGuid)
		{
			return false;
		}

		PgActorPet::ms_pkTempPetInventory->Swap((*PgActorPet::ms_pkMyPetInventory), true);
		PgActorPet::ms_pkTempPetInventory->Clear();

		PgActorPet::ms_pkMyPetInventory->OwnerGuid( rkGuid );
		PgActorPet::ms_pkMyPetInventory->OwnerName( rkName );
		PgActorPet::ms_pkMyPetInventory->RefreshAbil(NULL);

		return true;
	}

	int GetPetDyeingRandColorMax(void) //현재 착용중인 펫의 AT_PET_CHOOSE_RAND_COLOR_MAX 어빌 값을 리턴
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			PgBase_Item kPetItem;
			if(pkInv && S_OK == pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kPetItem))
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pkPetItemDef = kItemDefMgr.GetDef(kPetItem.ItemNo());
				if(pkPetItemDef == NULL) { return 0; }
				return pkPetItemDef->GetAbil(AT_PET_CHOOSE_RAND_COLOR_MAX);
			}
		}
		return 0;
	}

	int GetPetDyeingRandColorIndex(void)
	{
		int iMaxNum = PgPetUIUtil::GetPetDyeingRandColorMax();
		PgPet* pkPet = GetMySelectedPet();
		if(iMaxNum == 0 || pkPet == NULL)
		{
			return 0;
		}

		if(iMaxNum == 1)
		{
			return 1;
		}
		//iMaxNum--;
		int iCurIndex = pkPet->GetAbil(AT_COLOR_INDEX);
		int iNewIndex = BM::Rand_Index(iMaxNum - 1) + 1; //랜덤값이 현재 index값과 겹치지 않기 위해 N-1을 해준다
		iNewIndex += iCurIndex;
		if(iCurIndex == 0)
		{
			return 0;
		}

		if(iMaxNum < iNewIndex)
		{
			iNewIndex = iNewIndex % iMaxNum;
		}
		if(iNewIndex <= 0)
		{
			iNewIndex = iMaxNum;
		}

		return iNewIndex;
	}

	bool OnClickDyeingBtn(void)
	{
		if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
		{
			lwAddWarnDataTT( 10410 );
			return false;
		}

		PgPet* pkPet = GetMySelectedPet();
		if(!pkPet)	{return true;}

		if( EPET_TYPE_1 == pkPet->GetPetType() )
		{//! 1차펫이라면, 염색약 사용 불가.
			lwAddWarnDataTT(7512);
			return false;
		}

		if(PgPetUIUtil::GetPetDyeingRandColorMax() > 0) //착용중인 펫의 AT_PET_CHOOSE_RAND_COLOR_MAX 어빌값을 체크
		{
			lua_tinker::call<void, int>("CallPetDyeingRandColorChk", PgPetUIUtil::GetPetDyeingRandColorIndex() );
			//XUIMgr.Activate(L"FRM_PET_RAND_COLOR_CHANGE_CHECK");
			return false;
		}
		if( EPET_TYPE_3 == pkPet->GetPetType() )
		{
			lwAddWarnDataTT(451002);
			return false;
		}

		XUIMgr.Activate(L"FRM_PET_COLOR_TYPE");
		return true;
	}
}

bool PgPetUIManager::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	def(pkState, "SendReqFeedPet", &PgPetUIUtil::lwSendReqFeedPet);
	def(pkState, "DisplayPetIconForFeed", &PgPetUIUtil::lwDisplayPetIconForFeed);
	def(pkState, "Net_PT_C_M_REQ_ITEM_CHANGE_TO_PET", &PgPetUIUtil::Net_PT_C_M_REQ_ITEM_CHANGE_TO_PET);
	def(pkState, "Net_PT_C_M_REQ_PET_RENAME", &PgPetUIUtil::Net_PT_C_M_REQ_PET_RENAME);
	def(pkState, "UI_RefreshPetSkillUI", &PgPetUIUtil::lwUI_RefreshPetSkillUI);
	def(pkState, "UsePetTrainItem", &PgPetUIUtil::lwUsePetTrainItem);
	def(pkState, "TickTwinkleBtn", &PgPetUIUtil::lwTickTwinkleBtn);
	def(pkState, "CallPetStatusToolTip", &PgPetUIUtil::lwCallPetStatusToolTip);
	def(pkState, "GetPetRenamePay", &PgPetUIUtil::lwGetPetRenamePay);
	def(pkState, "GetPetDyeingPay", &PgPetUIUtil::lwGetPetDyeingPay);
	def(pkState, "GetPetRenamePayInt64", &PgPetUIUtil::lwGetPetRenamePayInt64);
	def(pkState, "GetPetDyeingPayInt64", &PgPetUIUtil::lwGetPetDyeingPayInt64);
	def(pkState, "DrawTextToPetState", &PgPetUIUtil::lwDrawTextToPetState);
	def(pkState, "CheckPetNameFiltering", &PgPetUIUtil::lwCheckPetNameFiltering);
	def(pkState, "CheckPetColorChange", &PgPetUIUtil::lwCheckPetColorChange);
	def(pkState, "GetPetDyeingRandColorMax", &PgPetUIUtil::GetPetDyeingRandColorMax);
	def(pkState, "GetPetDyeingRandColorIndex", &PgPetUIUtil::GetPetDyeingRandColorIndex);
	def(pkState, "OnClickDyeingBtn", &PgPetUIUtil::OnClickDyeingBtn);
	def(pkState, "CheckPetNameChange", &PgPetUIUtil::lwCheckPetNameChange);
	def(pkState, "CallPetUI", &PgPetUIUtil::lwCallPetUI);
	def(pkState, "ShowBalloonToIdleActionChange", &PgPetUIUtil::lwShowBalloonToIdleActionChange);
	def(pkState, "HideBalloonToIdleActionChange", &PgPetUIUtil::lwHideBalloonToIdleActionChange);
	return true;
}

bool PgPetUIManager::SetSrcItem(const SItemPos &rkItemPos, int const iNo)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}
	PgBase_Item kItem;
	if(S_OK!=pInv->GetItem(rkItemPos, kItem))
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pItemDef )
	{
		return false;
	}

	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	if(UICT_PET!=iType)
	{
		return false;
	}

	if(MAX_FEEDITEM<=iNo)
	{
		return false;
	}

	PgItem_PetInfo *pkPetInfo = NULL;
	if(false==kItem.GetExtInfo(pkPetInfo))	{return false;}

	GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
	PgClassPetDef kDef;
	if(false==kClassPetDefMgr.GetDef(pkPetInfo->ClassKey(), &kDef))	{return false;}

	if(0==iNo)	//1차펫 사료라면
	{
		if(EPET_TYPE_1 != kDef.GetPetType())	{lwAddWarnDataTT(358); return false;}//2차펫도 1차 펫 사료로 부활시킨다
	}
	else
	{
		if(EPET_TYPE_1 == kDef.GetPetType())	{lwAddWarnDataTT(358); return false;}
	}

	m_kFeedItem[iNo].m_kPetItemPos = rkItemPos;
	m_kFeedItem[iNo].m_kPetItem = kItem;
	m_kFeedItem[iNo].m_iPetResNo = pItemDef->ResNo();

	return true;
}

bool PgPetUIManager::SetSrcItem(SItemPos const &rkItemPos, PgInventory* pkInv, XUI::CXUI_Wnd *pWnd)
{
	if(NULL==pkInv)	{return false;}

	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(rkItemPos, kItem))
	{
		return false;
	}

	int iNo = 0;
	if(pWnd)
	{
		pWnd->GetCustomData(&iNo, sizeof(iNo));
	}

	return SetSrcItem(rkItemPos, iNo);
}

bool PgPetUIManager::SetFeedItem(SItemPos const  &rkItemPos, int const iNo)
{
	if (rkItemPos.x && rkItemPos.y)
	{
		Clear();
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}
	PgBase_Item kItem;
	if(S_OK!=pInv->GetItem(rkItemPos, kItem))
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pItemDef )
	{
		return false;
	}

	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	if(UICT_PET_FEED!=iType)
	{
		return false;
	}

	if(MAX_FEEDITEM<=iNo)
	{
		return false;
	}

	m_kFeedItem[iNo].m_kFeedItemPos = rkItemPos;
	m_kFeedItem[iNo].m_kFeedItem = kItem;

	return true;
}

void PgPetUIManager::Clear()
{
	for(int i = 0; i<MAX_FEEDITEM; ++i)
	{
		m_kFeedItem[i].Clear();
	}
}

void PgPetUIManager::SendReqFeedPet(bool bIsTrueSend, int const iNo)
{
	if(MAX_FEEDITEM<=iNo)
	{
		return;
	}
	if(0==m_kFeedItem[iNo].m_kFeedItemPos.x && 0==m_kFeedItem[iNo].m_kFeedItemPos.y)
	{
		return;
	}
	if(0==m_kFeedItem[iNo].m_kPetItemPos.x && 0==m_kFeedItem[iNo].m_kPetItemPos.y)
	{
		return;
	}

	if(!bIsTrueSend)
	{
		std::wstring kItemName;
		if(!MakeItemName(m_kFeedItem[iNo].m_kFeedItem.ItemNo(), m_kFeedItem[iNo].m_kFeedItem.EnchantInfo(), kItemName))
		{
			return;
		}

		PgItem_PetInfo *pkPetInfo = NULL;
		if ( true == m_kFeedItem[iNo].m_kPetItem.GetExtInfo( pkPetInfo ) )
		{
			std::wstring wstrTime;
			if(0==iNo)	//기본 사료. 1차펫용.
			{
				WstringFormat( wstrTime, MAX_PATH, TTW(132).c_str(), kItemName.c_str(), pkPetInfo->Name().c_str() );
			}
			else
			{
				WstringFormat( wstrTime, MAX_PATH, TTW(327).c_str(), kItemName.c_str(), TTW(330+iNo).c_str() );
			}

			lwUIWnd kWnd(PgPetUIUtil::CallFeedResultUI(wstrTime, L"SFRM_BUY_PET_FEED"));
			if(kWnd.IsNil())	{return;}
			kWnd.SetCustomData<int>(iNo);
		}
	}
	else
	{
		//아이템위치로 한번 더 검사해 보자
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return;}

		PgInventory *pInv = pkPlayer->GetInven();
		if(!pInv){return;}
		/*lwUIWnd kWnd = lwGetUIWnd("SFRM_BUY_PET_FEED");	
		if(kWnd.IsNil())
		{
			return;
		}
		int iNewNo = kWnd.GetCustomData<int>();
		kWnd.Close();	//값만 얻고 닫자*/
		PgBase_Item kItem;
		if(S_OK!=pInv->GetItem(m_kFeedItem[iNo].m_kFeedItemPos, kItem))
		{
			return;
		}
		else if(m_kFeedItem[iNo].m_kFeedItem.Guid() != kItem.Guid())
		{
			return;
		}

		if(S_OK!=pInv->GetItem(m_kFeedItem[iNo].m_kPetItemPos, kItem))
		{
			return;
		}
		else if(m_kFeedItem[iNo].m_kPetItem.Guid() != kItem.Guid())
		{
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, m_kFeedItem[iNo].m_kFeedItemPos);
		kPacket.Push(lwGetServerElapsedTime32());
		kPacket.Push(m_kFeedItem[iNo].m_kPetItemPos);
		NETWORK_SEND(kPacket);

		switch( iNo )
		{
//		case PET_FEED_TYPE_2:	{ g_kMyActorViewMgr.ChangePetAction("PetActor", "a_meal"); } break;
		case PET_FEED_TRAINING:	{ g_kMyActorViewMgr.ChangePetAction("PetActor", "a_exercise"); } break;
		case PET_FEED_PLAYING:	{ g_kMyActorViewMgr.ChangePetAction("PetActor", "a_play"); } break;
		}
	}
}

void PgPetUIManager::UseFeedItem(SItemPos const & rkItemInvPos, int const iNo)
{
	if(SetFeedItem(rkItemInvPos, iNo))
	{
		lwUIWnd kWnd = lwActivateUI("SFRM_USE_PET_FEED");
		kWnd.SetCustomData<int>(iNo);
	}
}

void PgPetUIManager::DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int const iNo)
{
	if(NULL==pWnd || MAX_FEEDITEM<=iNo || m_kFeedItem[iNo].m_kPetItem.IsEmpty() || 0==m_kFeedItem[iNo].m_iPetResNo)
	{
		return;
	}

	g_kUIScene.RenderIcon( m_kFeedItem[iNo].m_iPetResNo, pWnd->TotalLocation(), false );	
	pWnd->SetCustomData(&m_kFeedItem[iNo].m_kPetItem.ItemNo(), sizeof(m_kFeedItem[iNo].m_kPetItem.ItemNo()));
}

void PgPetUIManager::CallResultUI( PgItemWrapper const& rkItemWrap, PgItemWrapper const& rkPrevItemWrap, PgInventory *pkInv)
{
	if ( pkInv )
	{
		PgItem_PetInfo *pkPetInfo = NULL;
		if ( true == rkItemWrap.GetExtInfo( pkPetInfo ) )
		{
			__int64 i64Delta = ( rkItemWrap.CreateDate() == rkPrevItemWrap.CreateDate() ) ? (rkItemWrap.GetDelTime() - rkPrevItemWrap.GetDelTime()) : rkItemWrap.GetUseAbleTime();
			if(0 == i64Delta)
			{// 기간 연장 아니라면 팝업을 띄우지 않음
				return;
			}
			if ( i64Delta % 86400i64 > 86000i64 )
			{// 남은 시간이 오차 범위내면 1일을 더해주자..(오차를 크게 잡았다. 400초???)
				i64Delta /= 86400i64 + 1i64;
			}
			else
			{
				i64Delta /= 86400i64;
			}

			std::wstring kText;
			WstringFormat( kText, MAX_PATH, TTW(133).c_str(), pkPetInfo->Name().c_str(), i64Delta );
			PgPetUIUtil::CallFeedResultUI(kText);
		}
	}
}

namespace PgPetExpItemUI
{
	void lwDisplaySrcItem(lwUIWnd kWnd)
	{
		g_kPetExpUIMgr.DisplaySrcItem(kWnd(), kWnd.GetBuildIndex());
	}

	void lwDisplayResultItem(lwUIWnd kWnd)
	{
		g_kPetExpUIMgr.DisplayResultItem(kWnd(), kWnd.GetBuildIndex());
	}

	void lwSendChangeExpItem(lwUIWnd kWnd, bool bIsTrueSend, bool bClear)
	{
		g_kPetExpUIMgr.SendChangeExpItem(kWnd(), bIsTrueSend, bClear);
	}
}

bool PgPetExpItemUIManager::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	def(pkState, "DisplayPetExpIconSrc", &PgPetExpItemUI::lwDisplaySrcItem);
	def(pkState, "DisplayPetExpIconResult", &PgPetExpItemUI::lwDisplayResultItem);
	def(pkState, "SendChangeExpItem", &PgPetExpItemUI::lwSendChangeExpItem);
	
	return true;
}

bool PgPetExpItemUIManager::SetSrcItem(SItemPos const &rkItemPos, int const iNo)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}
	PgBase_Item kItem;
	if(S_OK!=pInv->GetItem(rkItemPos, kItem))
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pItemDef )
	{
		return false;
	}

	__int64 const i64Exp = pItemDef->GetAbil64(AT_EXPERIENCE);	//경험치 아이템인지 검사
	if(0>=i64Exp)
	{
		return false;
	}

	CONT_DEF_CONVERTITEM const * pkDef = NULL;
	g_kTblDataMgr.GetContDef(pkDef);
	CONT_DEF_CONVERTITEM::const_iterator iter = pkDef->find(kItem.ItemNo());
	if(iter==pkDef->end())
	{
		return false;	//DB에 없으면 
	}

	TBL_DEF_CONVERTITEM const& rkResultDef = (*iter).second;
	CItemDef const* pResultItemDef = kItemDefMgr.GetDef(rkResultDef.iTargetItemNo);
	if( !pResultItemDef )
	{
		return false;
	}

	for(int i = 0; i<MAX_EXPITEM; ++i)
	{
		if(m_kExpItem[i].m_kSrcGuid == kItem.Guid())
		{
			return false;//있는 놈임
		}
	}

	PgBase_Item kResultItem;
	kResultItem.ItemNo(rkResultDef.iTargetItemNo);
	kResultItem.Count(rkResultDef.sTargetItemNum);

	m_kExpItem[iNo].m_kSrcGuid = kItem.Guid();
	m_kExpItem[iNo].m_kSrcItemPos = rkItemPos;
	m_kExpItem[iNo].m_kSrcItem = kItem;
	m_kExpItem[iNo].m_iSrcResNo = pItemDef->ResNo();
	m_kExpItem[iNo].m_kResultItem = kResultItem;
	m_kExpItem[iNo].m_iResultResNo = pResultItemDef->ResNo();

	return true;
}

void PgPetExpItemUIManager::Clear()
{
	for(int i = 0; i<MAX_EXPITEM; ++i)
	{
		m_kExpItem[i].Clear();
	}

	m_kNpcGuid.Clear();
}

void PgPetExpItemUIManager::DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int const iNo)
{
	if(NULL==pWnd || MAX_EXPITEM<=iNo || m_kExpItem[iNo].m_kSrcItem.IsEmpty() || 0==m_kExpItem[iNo].m_iSrcResNo)
	{
		return;
	}

	lwUIWnd kWnd(pWnd);
	kWnd.DrawIconDetail_sub(KUIG_CONSUME, &m_kExpItem[iNo].m_kSrcItem, pWnd->TotalLocation());	
	pWnd->SetCustomData(&m_kExpItem[iNo].m_kSrcItem.ItemNo(), sizeof(m_kExpItem[iNo].m_kSrcItem.ItemNo()));
}

void PgPetExpItemUIManager::DisplayResultItem(XUI::CXUI_Wnd *pWnd, int const iNo)
{
	if(NULL==pWnd || MAX_EXPITEM<=iNo || 0==m_kExpItem[iNo].m_iResultResNo)
	{
		return;
	}

	lwUIWnd kWnd(pWnd);
	kWnd.DrawIconDetail_sub(KUIG_CONSUME, &m_kExpItem[iNo].m_kResultItem, pWnd->TotalLocation());	
	pWnd->SetCustomData(&m_kExpItem[iNo].m_kResultItem.ItemNo(), sizeof(m_kExpItem[iNo].m_kResultItem.ItemNo()));
}

void PgPetExpItemUIManager::SendChangeExpItem(XUI::CXUI_Wnd* pkWnd, bool const bIsTrueSend, bool const bClear)
{
	if(bClear || NULL==pkWnd)
	{
		Clear();
		return;
	}

	//아이템 있는 지 확인하자
	bool bIsEmptyAll = true;
	for(int i = 0; i<MAX_EXPITEM; ++i)
	{
		if(false==m_kExpItem[i].m_kSrcItem.IsEmpty())//한놈이라도 있으면
		{
			bIsEmptyAll = false;
			break;
		}
	}

	if(true==bIsEmptyAll)	{return;}

	//if(!bIsTrueSend)
	{
		NpcGuid(pkWnd->OwnerGuid());
	}
	//else
	{
		for(int i = 0; i<MAX_EXPITEM; ++i)
		{
			if(false==m_kExpItem[i].m_kSrcItem.IsEmpty())//한놈이라도 있으면
			{
				BM::Stream kPacket(PT_C_M_REQ_CONVERTITEM);
				kPacket.Push(NpcGuid()); // Npc Guid
				kPacket.Push(static_cast<int>(m_kExpItem[i].m_kSrcItem.ItemNo()));	// 변환할 아이템 번호
				kPacket.Push(static_cast<int>(m_kExpItem[i].m_kSrcItem.Count())); // 생성될 목표 아이템의 개수
				NETWORK_SEND(kPacket)
			}
		}
		Clear();//보낸 뒤에는 초기화
	}
}

PgPet* GetMySelectedPet()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return NULL;
	}

	PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
	if(!pkPetPilot || !pkPetPilot->GetUnit())
	{
		return NULL;
	}

	return dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
}

bool GetMySelectedPetInfo( PgItem_PetInfo &rkOutPetInfo )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	PgBase_Item kPetItem;
	if( S_OK==pkPlayer->GetInven()->GetItem( PgItem_PetInfo::ms_kPetItemEquipPos, kPetItem ) )
	{
		PgItem_PetInfo *pkItemPetInfo = NULL;
		if ( true == kPetItem.GetExtInfo( pkItemPetInfo ) )
		{
			rkOutPetInfo = *pkItemPetInfo;
			return true;
		}
	}
	return false;
}

PgInventory* GetMySelectedPetInven()
{
	PgPet * pkPet = GetMySelectedPet();
	if ( pkPet )
	{
		return pkPet->GetInven();
	}

	return PgActorPet::ms_pkMyPetInventory;
}
