#include "StdAfx.h"

#include "Variant/DefAbilType.h"
#include "Variant/Global.h"
#include "Variant/PgComboCounter.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgControlDefMgr.h"
#include "Variant/PgMission.h"
#include "variant/PgStore.h"
#include "Variant/PgEventQuestBase.h"
#include "Variant/PetSkillDataConverter.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgActor.h"
#include "PgActorPet.h"
#include "lwPilot.h"
#include "PgActionEffect.h"
#include "PgSoundMan.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgRemoteManager.h"
#include "HandlePacket.h"
#include "PgDropBox.h"
#include "PgWorld.h"
#include "PgShineStone.h"
#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "PgClientParty.h"
#include "PgUIScene.h"
#include "PgSoundMan.h"
#include "PgContentsBase.h"
#include "PgSelectStage.h"
#include "PgCoupleMgr.h"
#include "PgHelpSystem.h"
#include "lwGuildUI.h"
#include "PgQuestMan.h"
#include "PgEventScriptSystem.h"
#include "PgAchieveNfyMgr.h"
#include "lwUICharInfo.h"
#include "lwMonsterKillUIMgr.h"
#include "lwUIBook.h"
#include "lwWorld.h"
#include "PgEnergyGuage.h"
#include "PgSafeFoamMgr.h"
#include "PgCashShop.h"
#include "PgUISound.h"
#include "PgActorUtil.h"
#include "lwCharacterCard.h"
#include "lwMarket.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIItemRarityUpgrade.h"
#include "lwUIPet.h"
#include "PgEqItemMixCreator.h"
#include "PgItemMix_Script.h"
#include "lwCashItem.h"
#include "PgHouse.h"
#include "PgHome.h"
#include "PgEventQuestUI.h"
#include "lwQuestNfy.h"
#include "PgMyActorViewMgr.h"
#include "lwUIActiveStatus.h"
#include "PgMissionComplete.h"
#include "PgDailyQuestUI.h"
#include "PgChainAttack.h"
#include "lwInventoryUI.h"
#include "lwJobSkillView.h"
#include "lwItemSkillView.h"
#include "PgMail.h"
#include "lwCommonSkillUtilFunc.h"
#include "lwUIGemStore.h"
#include "PgConstellation.h"

extern bool IsDefenceMode();
extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);
extern bool MakeItemName(int const iItemNo, const SEnchantInfo &rkEnchant, std::wstring &wstrName, int* Grade);
extern int g_iInvViewGrp;
int const MINIMUM_ENDURANCE_VALUE = 5;
int const FULL_INVEN_MESSAGE_NUM = 400715;

int const CHECK_QUEST_MESSAGE_MIN = 699991;
int const CHECK_QUEST_MESSAGE_MAX = 699994;
int const CHECK_EFFECT_MESSAGE_MIN = 680000;
int const CHECK_EFFECT_MESSAGE_MAX = 683999;

namespace PgPetUIUtil
{
//	bool lwOnChangePetAbil(PgPilot* pkPetPilot, bool bExist_AT_TIME);
	void lwDrawTextToPetState(lwUIWnd kWnd = lwUIWnd(NULL));
	void PetSkillToSkillTree(SClassKey const &kPetClassKey);
	XUI::CXUI_Wnd* SetPetUIDefaultState(PgBase_Item const& rkItem);
	XUI::CXUI_Wnd* SetPetUIDefaultState(bool bEnableCover);
	void AutomaticUsePetItemConsumer( CUnit* pkMyPet, WORD const iType, int const iPrevValue, int const iIncreaseVal);
	bool LoadMyPetInventory(BM::GUID const& rkGuid, std::wstring const& rkName);
	void lwUI_RefreshPetSkillUI(lwUIWnd kWnd);
};

namespace lwSkillSet
{
	extern void lwSkillSetInit(lwUIWnd kWnd);
}

namespace lwJobSkillItem
{
	extern void JS3_RefreshResItem();
}

extern int const SOUL_ITEM_NO;
void Recv_PT_M_C_NFY_STATE_CHANGE64(PgPilot *pkPilot, SAbilInfo64 const &rkAbilInfo)
{	
	if(!pkPilot)
	{
		return;
	}

	PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());

	if(!pkActor)
	{
		return;
	}

	__int64 const iPrevValue = pkPilot->GetAbil64(rkAbilInfo.wType);
	__int64 const iIncreaseVal = rkAbilInfo.iValue - iPrevValue;

	pkPilot->SetAbil64(rkAbilInfo.wType, rkAbilInfo.iValue);
	
	//_PgOutputDebugString("Process_State_Change Player: %s Abil : %d abilValue: %d\n",MB(pkPilot->GetName()), rkAbilInfo.wType,rkAbilInfo.iValue);

	lwPilot klwPilot(pkPilot);

	if(!klwPilot.IsMyPlayerPilot())//경험치 증가
	{
		return;
	}
	
	if(0 == iIncreaseVal)
	{
		return;
	}

	TCHAR szTemp[512] = {0 ,};
	switch(rkAbilInfo.wType)
	{
	case AT_EXPERIENCE:
		{
			std::wstring kTTWForm = (0 < iIncreaseVal) ? TTW(700001): TTW(700002);
			SChatLog kChatLog(CT_EXP);
			if(0 < iIncreaseVal)
			{
				_sntprintf_s(szTemp, 512, kTTWForm.c_str(), iIncreaseVal);				
			}
			else
			{
				_sntprintf_s(szTemp, 512, kTTWForm.c_str());
			}

			std::wstring kLog = szTemp;
			g_kChatMgrClient.AddLogMessage(kChatLog, kLog);

			pkActor->ShowExpNum((int)iIncreaseVal);
		}break;
	case AT_FRAN:
		{
			std::wstring kTTWForm = (0 < iIncreaseVal) ? TTW(700132) : TTW(700139);
			SChatLog kChatLog(CT_GOLD);
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), _abs64(iIncreaseVal));
			std::wstring kLog = szTemp;
			g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
		}break;
	case AT_TACTICS_EXP:
		{
			//pkActor->ShowExpNum((int)iIncreaseVal);
		}break;
	case AT_MUTE:
		{
			if( pkActor->IsUnderMyControl() )
			{
				// 이걸 호출해주면 채팅금지가 된걸 알 수 있다.
				g_kChatMgrClient.SendChat_MuteCheck();
			}
		}break;
	case AT_REST_EXP_ADD_MAX:
		{// 휴식 경험치, 경험치 변경이 있다면 UI를 키거나, 끈다
			lwShowRestExpBar();
		}break;
	//case AT_MONEY:
	//	{
	//	}break;
	default:
		{}break;
	}

	if(pkActor->IsUnderMyControl() )
	{
		lwCharInfo::UpdateMyActor();
	}
}

void Recv_PT_M_C_NFY_CHANGE_MONEY(BM::Stream& rkPacket)
{
	BYTE cCause = 0;
	__int64 iDiffMoney = 0;

	rkPacket.Pop(cCause);
	rkPacket.Pop(iDiffMoney);
	
	if( !iDiffMoney )
	{
		return ;
	}

	std::wstring kFormatMoney;
	__int64 const iAbsVal = (0 < iDiffMoney)? iDiffMoney: iDiffMoney*-1;//절대값
	if( !FormatMoney(iAbsVal, kFormatMoney) )
	{
		kFormatMoney = (std::wstring)BM::vstring((int)iAbsVal);
	}
	
	bool bShowNotice = false;
	int iTTW = 0;
	switch(cCause)
	{
	case MCE_SellItem://Inc
		{
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, "BuySell_Coin", 0.0f);
			::Notice_Show_ByTextTableNo(9015, EL_Normal);
		}
	//case MCE_Reward:
	case MCE_Loot:
		{
			iTTW = 700048;//돈을 얻다.
		}break;
	case MCE_BuyItem://Dec
		{
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, "BuySell_Coin", 0.0f);
			iTTW = 700053;//돈을 소비 하다
			::Notice_Show_ByTextTableNo(9014, EL_Normal);
		}break;
	//case MCE_None://Lost
	default:
		{
			if( 0 < iDiffMoney )
			{
				iTTW = 700048;//Inc
			}
			else
			{
				iTTW = 700052;//Dec(돈을 잃다)
			}
		}break;
	}

	std::wstring kLog;
	bool const bRet = FormatTTW(kLog, iTTW, kFormatMoney.c_str());
	if( !bRet )
	{
		return;
	}

	SChatLog kChatLog(CT_GOLD);
	g_kChatMgrClient.AddLogMessage(kChatLog, kLog, bShowNotice);

	XUI::CXUI_Wnd*	pUpInfo = XUIMgr.Get(_T("FRM_MINIMAP"));
	if( pUpInfo )
	{
		pUpInfo->SetInvalidate();
	}
}

void Recv_PT_M_C_NFY_STATE_CHANGE2( PgPilot *pkPilot, E_SENDABIL_TARGET eTarget, SAbilInfo const &rkAbilInfo)
{	
	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(!pkActor)
	{
		if( pkPilot )
		{
			PgMyHome* pkHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
			if( pkHome )
			{
				pkHome->SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);
			}
		}
		return;
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	if ( !pkUnit )
	{
		return;
	}

//	if( pkActor->IsMyActor() )
//	{// 내 플레이어에
//		switch(rkAbilInfo.wType)
//		{// 어빌 변경이 있으면 무조건 실행
//		case AT_SKILL_LINKAGE_POINT_MAX:
//			{
//				SkillLinkage::lwCallLinkageUI();
//			}break;
//		}
//	}
	if( eTarget == E_SENDABIL_TARGET_EFFECT )
	{
		//int const iPrevValue = pkUnit->GetEffectMgr().GetAbil(rkAbilInfo.wType);
		//int const iIncreaseVal = iPrevValue - rkAbilInfo.iValue;

		pkUnit->GetEffectMgr().SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);
		pkUnit->NftChangedAbil(rkAbilInfo.wType, E_SENDTYPE_NONE);

		if( UT_PET != pkUnit->UnitType() && pkActor->IsUnderMyControl() )
		{
			lwCharInfo::UpdateMyActor();
		}

		switch(rkAbilInfo.wType)
		{
		case AT_HP:			// break; 금지
		case AT_MAX_HP:		// break; 금지
		case AT_R_MAX_HP:	// break; 금지
		case AT_C_MAX_HP:	// break; 금지
			{
				pkActor->RefreshHPGaugeBar(pkUnit->GetAbil(AT_HP), pkUnit->GetAbil(AT_HP), NULL);
			}break;
		case AT_R_MOVESPEED:
			{
				if(false == pkUnit->IsUnitType(UT_PLAYER)) { break; }
				PgActor* pkPetActor = pkActor->GetPetActor(); //펫이 있다면...
				if(NULL == pkPetActor) { break; }
				CUnit* pkPetUnit = pkPetActor->GetUnit();
				if(NULL == pkPetUnit) { break; }

				//펫의 이동속도도 같이 적용해줌 (탑승 했을 때를 위함)
				pkPetUnit->GetEffectMgr().SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);
				pkPetUnit->NftChangedAbil(rkAbilInfo.wType, E_SENDTYPE_NONE);
			}break;
		}	

		return;
	}

	int const iPrevValue = pkPilot->GetAbil(rkAbilInfo.wType);
	int const iIncreaseVal = iPrevValue - rkAbilInfo.iValue;

	// Set Abil하기 전에 할께 있다.
	if(	iIncreaseVal )
	{
		switch( rkAbilInfo.wType )
		{
		case AT_CLASS:
		case AT_LEVEL:
		case AT_BATTLE_LEVEL:
			{
				if ( pkActor->IsMyActor() )
				{
					int iStr = pkUnit->GetAbil( AT_STR );
					int iInt = pkUnit->GetAbil( AT_INT );
					int iCon = pkUnit->GetAbil( AT_CON );
					int iDex = pkUnit->GetAbil( AT_DEX );

					pkPilot->SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);

					iStr = pkUnit->GetAbil( AT_STR ) - iStr;
					iInt = pkUnit->GetAbil( AT_INT ) - iInt;
					iCon = pkUnit->GetAbil( AT_CON ) - iCon;
					iDex = pkUnit->GetAbil( AT_DEX ) - iDex;

					std::wstring wstrMsg;
					if ( iStr )
					{
						std::wstring wstrTemp;
						FormatTTW( wstrTemp, 7000, TTW(AT_STR).c_str(), iStr );
						wstrMsg += wstrTemp;
						wstrMsg += L"\n";
					}

					if ( iInt )
					{
						std::wstring wstrTemp;
						FormatTTW( wstrTemp, 7000, TTW(AT_INT).c_str(), iInt );
						wstrMsg += wstrTemp;
						wstrMsg += L"\n";
					}

					if ( iCon )
					{
						std::wstring wstrTemp;
						FormatTTW( wstrTemp, 7000, TTW(AT_CON).c_str(), iCon );
						wstrMsg += wstrTemp;
						wstrMsg += L"\n";
					}

					if ( iDex )
					{
						std::wstring wstrTemp;
						FormatTTW( wstrTemp, 7000, TTW(AT_DEX).c_str(), iDex );
						wstrMsg += wstrTemp;
						wstrMsg += L"\n";
					}

					if( wstrMsg.size() )
					{
						if( g_kQuestMan.IsQuestDialog() )
						{
							g_kQuestMan.TempLevelMsg(wstrMsg);
						}
						else
						{
							Notice_Show( wstrMsg, EL_Notice4 );
						}
					}
				}
				else
				{
					pkPilot->SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);
				}
			}break;
		case AT_STR:
		case AT_INT:
		case AT_CON:
		case AT_DEX:
			{
				if ( pkActor->IsMyActor() )
				{
					std::wstring wstrMsg;
					FormatTTW( wstrMsg, 7000, TTW(rkAbilInfo.wType).c_str(), iIncreaseVal );
					wstrMsg += L"\n";

					if( g_kQuestMan.IsQuestDialog() )
					{
						g_kQuestMan.TempLevelMsg(wstrMsg);
					}
					else
					{
						Notice_Show( wstrMsg, EL_Normal );
					}
				}
			}// break을 사용하지 않는다.(다음에 반드시 default:가 와야 함)
		default:
			{
				pkPilot->SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);

				WORD const wCal = GetCalculateAbil(rkAbilInfo.wType);
				if ( 0 < wCal )
				{
					pkUnit->NftChangedAbil(rkAbilInfo.wType, E_SENDTYPE_NONE);
				}
			}break;
		}
	}
	
//	ODS("Process_State_Change Player:"..pilot:GetName():GetStr().." Abil : "..abilType.." abilValue:"..abilValue.."\n");
	_PgOutputDebugString("Process_State_Change Player: %s Abil : %d abilValue: %d\n",MB(pkPilot->GetName()), rkAbilInfo.wType,rkAbilInfo.iValue);
	switch(rkAbilInfo.wType)
	{
	case AT_CLASS://어빌만 바꾼다
		{
			if ( 0 != iIncreaseVal )
			{
				if( pkActor->IsMyActor() )//내꺼
				{
					lwSkillSet::lwSkillSetInit(lwUIWnd(XUIMgr.Get(L"SFRM_SKILLSET")));
					g_kSkillTree.CreateSkillTree( dynamic_cast<PgPlayer*>(pkUnit) );//전직했으므로 스킬트리를 재구성하자.

					lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");

					int const iClassTTW = 30100 + rkAbilInfo.iValue;
					std::wstring const kClassName = TTW(iClassTTW);
					std::wstring kMessage = kClassName + TTW(700028);

					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true, 2);
					if( IS_CLASS_LIMIT(UCLIMIT_ALL_DRAGONIAN, pkUnit->GetAbil(AT_CLASS) ) )
					{
						lwCommonSkillUtilFunc::TryMustChangeActorAction( pkActor, "a_class_promotion_dragonian");
					}
					else
					{
						lwCommonSkillUtilFunc::TryMustChangeActorAction( pkActor, "a_class_promotion_human");
					}
					
					int const iWeaponType = pkActor->GetEquippedWeaponType();
					int const iClass = pkPilot->GetAbil(AT_CLASS);
					int const iLevel = pkPilot->GetAbil(AT_LEVEL);
					int const iComboID = g_kSkillTree.FindCombo( iWeaponType, iLevel, iClass);
					if( 0 != iComboID )
					{
						lua_tinker::call< void, int, int >("CallComboAlramUI", iWeaponType, iComboID);
					}
				}
				
				//전직 이펙트
				NiAVObject *pkParticle = g_kParticleMan.GetParticle("ef_class_change", PgParticle::O_SCALE,pkActor->GetEffectScale());
				
				switch ( pkUnit->UnitType() )
				{
				case UT_PET:
					{// 펫은 모양이 바뀌어야 한다.
						if ( pkParticle )
						{
							PgActor * pkCallerActor = g_kPilotMan.FindActor(pkUnit->Caller());
							if ( pkCallerActor )
							{
								if ( !pkCallerActor->AttachToPoint(8382, lwActor(pkActor).GetNodeWorldPos("char_root")(), (NiAVObject *)pkParticle) )
								{
									THREAD_DELETE_PARTICLE(pkParticle);
								}

								g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Pet_Evolution", 0.0f, 0.0f, 0.0f, pkActor);

								BM::GUID kPlayerGuid;
								if(g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid))
								{
									if(pkUnit->Caller()==kPlayerGuid)	//내 펫이면
									{
										Notice_Show( TTW(339), EL_Level2 );
									}
								}
							}
						}

						if ( g_pkRemoteManager )
						{
							pkUnit->SetPos( POINT3(pkActor->GetWorldTranslate().x, pkActor->GetWorldTranslate().y, pkActor->GetWorldTranslate().z) );

							// 1. AddUnit Packet을 만들고
							UNIT_PTR_ARRAY kAddUnitArray;
							kAddUnitArray.Add(pkUnit);

							BM::Stream kAPacket(PT_M_C_ADD_UNIT);

							if ( pkActor->IsMyActor() || pkActor->IsMyPet() )
							{
								kAddUnitArray.WriteToPacket( kAPacket, WT_DEFAULT );
							}
							else
							{
								kAddUnitArray.WriteToPacket( kAPacket, WT_SIMPLE );
							}

							if ( g_pkWorld )
							{
								// 2. 지우라고 Queue에 넣어놓고
								g_pkWorld->RemoveObjectOnNextUpdate( pkUnit->GetID() );
							}

							// 3. 패킷을 넣어주면.
							g_pkRemoteManager->AddPacket( kAPacket );

							// 다음 업데이트에서 유닛을 지우고.
							// 패킷처리가 되서 유닛을 새로 만들것이다.
						}
					}break;
				case UT_PLAYER:
					{
					}break;
				default: 
					{
						if( pkParticle )
						{
							if( !pkActor->AttachTo(8382, "char_root", (NiAVObject *)pkParticle) )
							{
								THREAD_DELETE_PARTICLE(pkParticle);
							}
						}
					}break;
				}
			}
		}break;
	case AT_LEVEL:
		{
			if ( 0 != iIncreaseVal )
			{
				if( pkActor->IsMyActor() )
				{
					lwSkillSet::lwSkillSetInit(lwUIWnd(XUIMgr.Get(L"SFRM_SKILLSET")));

					lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
					lwSetReqInvWearableUpdate(true);
					lua_tinker::call<bool,bool>("UpdateLevelUI", false);

					g_kHelpSystem.ActivateByCondition( std::string(HELP_CONDITION_LEVEL_UP), rkAbilInfo.iValue );
					lwQuestNfy::lwCheckNewQuestNfy(lwQuestNfy::EQNCT_LEVELUP);
					lwAchieveNfyMgr::lwCheckNewAchieveNfy();

					int const iWeaponType = pkActor->GetEquippedWeaponType();
					int const iClass = pkPilot->GetAbil(AT_CLASS);
					int const iLevel = pkPilot->GetAbil(AT_LEVEL);
					int const iComboID = g_kSkillTree.FindCombo( iWeaponType, iLevel, iClass);
					if( 0 != iComboID )
					{
						lua_tinker::call< void, int, int >("CallComboAlramUI", iWeaponType, iComboID);
					}
				}

				NiAVObject *pkParticle = NULL;
				switch( pkUnit->UnitType() )
				{
				case UT_PET :
					{
						pkParticle = g_kParticleMan.GetParticle( "ef_Pet_levelup_01_char_root",PgParticle::O_SCALE, pkActor->GetEffectScale());
						pkActor->UpdateName();
						if ( true == pkActor->IsUnderMyControl() )
						{//내 펫이면
							PgPet* pkPet = dynamic_cast<PgPet*>(pkUnit);
							if( pkPet )
							{
								GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
								if( kClassPetDefMgr.IsChangeClass(pkPet->GetClassKey()) )
								{
									lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 363, true);
								}
								else
								{
									short const sLastLv = kClassPetDefMgr.GetLastLv(pkPet->GetClassKey().iClass);
									if(sLastLv<=pkPet->GetClassKey().nLv)
									{
										lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 369, true);
									}
								}

								PgClassPetDef kPetDef;
								if ( kClassPetDefMgr.GetDef( pkPet->GetClassKey(), &kPetDef ) )
								{
									CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
									g_kTblDataMgr.GetContDef(pkDefPetSkill);

									CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID()  );
									if ( skill_itr != pkDefPetSkill->end() )
									{
										CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
										size_t const index = kContElement.find( PgDefClassPetSkillFinder(static_cast<short>(rkAbilInfo.iValue)) );
										if ( BM::PgApproximate::npos != index && index)
										{
											PgPetUIUtil::PetSkillToSkillTree(SClassKey(kPetDef.GetSkillDefID(), rkAbilInfo.iValue));
										}
									}
								}
							}
							Notice_Show( TTW(340), EL_Level2 );
						}
					}break;
				case UT_ENTITY:
					{
						if( ENTITY_GUARDIAN==pkUnit->GetAbil(AT_ENTITY_TYPE) )
						{
							pkActor->UpdateName();
						}

						EEntityType eType = static_cast<EEntityType>(pkUnit->GetAbil(AT_ENTITY_TYPE));
						std::string kEffectName;
						if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION) && IsDefenceMode() )
						{
							kEffectName = "EF_SaSin_Death_attk_01_basic";
						}
						else if(ENTITY_GUARDIAN==eType)
						{
						}
						else
						{
							kEffectName = "e_level_up";
						}

						if( !kEffectName.empty() )
						{
							pkParticle = g_kParticleMan.GetParticle( kEffectName.c_str(),PgParticle::O_SCALE, pkActor->GetEffectScale() );
						}
					}break;
				default:
					{
						pkParticle = g_kParticleMan.GetParticle( "e_level_up",PgParticle::O_SCALE, pkActor->GetEffectScale() );
					}break;
				}

				if( pkParticle )
				{
					if(!pkActor->AttachTo(8382, "char_root", pkParticle))
					{
						THREAD_DELETE_PARTICLE(pkParticle);
					}
				}
				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, UT_PET == pkUnit->UnitType() ? "Pet_Level_UP" : "Com_lvup", 0.0f, 0.0f, 0.0f, pkActor);
			}
		}break;
	case AT_BATTLE_LEVEL:
		{
			if ( 0 != iIncreaseVal )
			{
				if( pkActor->IsMyActor() )
				{
					std::wstring wstrText;
					if ( 0 == rkAbilInfo.iValue )
					{
						wstrText = TTW(700011);
					}
					else
					{
						WstringFormat( wstrText, MAX_PATH, TTW(700010).c_str(), rkAbilInfo.iValue );	
					}

					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, wstrText, true, 2);

					lua_tinker::call<bool,bool>("UpdateLevelUI", true);
				}
			}
		}break;
	case AT_LEVEL_RANK:
		{
			if ( 0 == rkAbilInfo.iValue )
			{
				Notice_Show( TTW(120003), EL_Level2 );
			}
			else
			{
				BM::vstring vstr( TTW(120002) );
				vstr.Replace( L"#NUM#", rkAbilInfo.iValue + 1 );
				Notice_Show( static_cast<std::wstring>(vstr), EL_Level2 );
			}
		}break;
	case AT_TACTICS_LEVEL:
		{

		}break;
	case AT_MAX_HP:
	case AT_R_MAX_HP:
	case AT_C_MAX_HP:
		{
			pkActor->RefreshHPGaugeBar(pkUnit->GetAbil(AT_HP), pkUnit->GetAbil(AT_HP), NULL);
		}break;
	case AT_HP:
		{
//			pkActor->SetActorDead(0 == rkAbilInfo.iValue && !iDeadSkill);

			if( pkUnit->IsUnitType(UT_PLAYER) )//Player일때
			{
				if( 0 >= rkAbilInfo.iValue )//HP가 0이면 죽인다.
				{
					pkActor->AddEffect(ACTIONEFFECT_DIE, 0, 0, pkUnit->GetID(), 0, 0, true);//매직넘버
				}
				else if( pkActor->IsMyActor() )
				{
					if( -iIncreaseVal > 0 )
					{
						pkActor->ShowHpMpNum(abs(iIncreaseVal));
					}
				}
			}
			else//Player 가 아닐때
			{
				if( 0 >= rkAbilInfo.iValue )
				{
					bool bDeadSkill = false;
					int const iDeadSkill = pkPilot->GetAbil(AT_SKILL_ON_DIE);
					if( iDeadSkill )
					{
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						std::wstring const& kActionName = kSkillDefMgr.GetActionName(iDeadSkill);
						bDeadSkill = !kActionName.empty() && kActionName != L"0";
					}

					if( false == pkActor->IsReserveDieByAction() && !bDeadSkill)//죽음이 예약된 상태가 아니면 죽인다.
					{
						pkActor->AddEffect(ACTIONEFFECT_DIE, 0);//매직넘버
					}
					else
					{
						pkPilot->SetAbil(rkAbilInfo.wType, iDeadSkill?1:iPrevValue);
					}

					if( pkPilot->GetAbil(AT_ELITEPATTEN_STATE) )
					{
						lua_tinker::call<void,lwGUID>("DieElitePattenMonster", lwGUID(pkPilot->GetGuid()));
					}
				}
			}

			if( PgContentsBase::ms_pkContents )
			{
				PgContentsBase::ms_pkContents->RecvHP( pkUnit, iPrevValue, rkAbilInfo.iValue );
			}

			pkActor->RefreshHPGaugeBar(iPrevValue, rkAbilInfo.iValue, NULL);

		}break;
	case AT_MP:
		{
			if( pkActor->IsMyActor() )
			{
				if( -iIncreaseVal > 0 )
				{
					pkActor->ShowHpMpNum(abs(iIncreaseVal), false);
				}
			}

			if ( PgContentsBase::ms_pkContents )
			{
				PgContentsBase::ms_pkContents->RecvMP( pkUnit->GetID(), pkUnit->GetAbil( AT_C_MAX_MP), rkAbilInfo.iValue );
			}

			if(pkUnit->IsUnitType(UT_PET))
			{
				PgActorPet* pkActorPet = dynamic_cast<PgActorPet*>(pkActor);
				if(pkActorPet)
				{
					pkActorPet->SetMPBarValue(iPrevValue, rkAbilInfo.iValue);
				}

				if(pkActor->GetCallerIsMe())	//내펫이면
				{
					PgPetUIUtil::AutomaticUsePetItemConsumer(pkUnit, rkAbilInfo.wType, iPrevValue, -iIncreaseVal);
				}
			}
		}break;
	case AT_SP:
		{
			if( pkActor->IsMyActor() )
			{
				g_kSkillTree.SetRemainSkillPoint(rkAbilInfo.iValue);
				lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
			}
		}break;
	case AT_QUEST_TALK://다른 플레이어가 퀘스트 대화중
		{
			if( QTS_None != rkAbilInfo.iValue )
			{
				std::wstring const kMessage = TTW(400573);
				pkActor->ShowChatBalloon(CT_NORMAL, kMessage, 0, true);//무한
			}
			else// QTS_None == 
			{
				pkActor->ShowChatBalloon_Clear();
			}
		}break;
	case AT_TEAM:
		{
			pkActor->UpdateName();

			PgIWorldObject *pkWorldObject = dynamic_cast<PgIWorldObject*>(pkActor);
			if ( pkWorldObject )
			{
				if( NULL != g_pkWorld )
				{
					using namespace PgActorUtil;
					UpdateColorShadow( *pkWorldObject, g_pkWorld, true, false );
				}
				else
				{
					PG_ASSERT_LOG(g_pkWorld);
				}
			}
			else
			{
				PG_ASSERT_LOG(NULL);
			}
		}break;
	case AT_MONSTER_KILL_COUNT:
		{
			g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_MONSTER_KILL_COUNTER), rkAbilInfo.iValue);
			g_kMonKillUIMgr.Update(rkAbilInfo.iValue);
		}break;
	case AT_SEND_UNIT_STATE:
		{
			pkActor->SetState(static_cast<EUnitState>(rkAbilInfo.iValue));
		}break;
	case AT_DEC_GROGGYPOINT_RATE:
		{
			lua_tinker::call<void,int,int>("SetGroggyGaugebarStateUI", pkPilot->GetAbil(AT_ELITEPATTEN_STATE), rkAbilInfo.iValue);
		}break;
	case AT_ELITEPATTEN_STATE:
		{
			lua_tinker::call<void,lwGUID,int,int>("SetElitePattenState", lwGUID(pkUnit->GetID()), pkPilot->GetAbil(AT_ELITEPATTEN_STATE), pkPilot->GetAbil(AT_GROGGY_MAX));
		}break;
	case AT_FRENZY_TIME:
		{
			lua_tinker::call<void,int>("SetElitePattenFrenzyTime", rkAbilInfo.iValue);
		}break;
	case AT_ADDED_GAUGE_VALUE:
		{
			PgEnergyGauge* pkHPGauge = pkActor->GetHPGaugeBar();
			if( pkHPGauge )
			{
				if( 0 > iIncreaseVal ) // 증가
				{
					pkHPGauge->Init(EGAUGE_CHARGING1);
					pkHPGauge->SetBarValue( rkAbilInfo.iValue, iPrevValue, rkAbilInfo.iValue );
				}
				else // 감소
				{
					pkHPGauge->Init(EGAUGE_CHARGING1);
					pkHPGauge->SetBarValue( pkHPGauge->MaxValue(), iPrevValue, rkAbilInfo.iValue );
				}

				if( 0 == rkAbilInfo.iValue )
				{
					// HP 로 돌아감
					pkHPGauge->Init(EGAUGE_CHARGING2);
					pkHPGauge->SetBarValue( pkPilot->GetAbil(AT_C_MAX_HP), 0, pkPilot->GetAbil(AT_HP) );
				}

				pkHPGauge->ResetVisibleStartTime();
			}
		}break;
	case AT_C_MOVESPEED:
		{
			if( pkActor->IsNowFollowing() || pkActor->IsNowFollowingMeActor() || g_pkWorld->IsHaveAttr(GATTR_RACE_GROUND))
			{
				pkPilot->SetAbil(rkAbilInfo.wType, rkAbilInfo.iValue);
			}
		}break;
	case AT_STRATEGIC_MUL_POINT:
		{
			pkActor->UpdateName();
		}break;
	case AT_HEALTH:
		{
			PgPet* pkPet = dynamic_cast<PgPet*>(pkPilot->GetUnit());
			if(pkPet)
			{
				PgItem_PetInfo::SStateValue kHealth( rkAbilInfo.iValue );
				pkPet->SetState_Health( kHealth );
			}
		}break;
	case AT_MENTAL:
		{
			PgPet* pkPet = dynamic_cast<PgPet*>(pkPilot->GetUnit());
			if(pkPet)
			{
				PgItem_PetInfo::SStateValue kMental( rkAbilInfo.iValue );
				pkPet->SetState_Mental( kMental );
			}
		}break;
	case AT_COLOR_INDEX:
		{
			PgActorPet* pkActorPet = dynamic_cast<PgActorPet*>(pkActor);
			if(pkActorPet)
			{
				pkActorPet->DoChangeColor();
				NiAVObject *pkParticle = g_kParticleMan.GetParticle("ef_Pet_dyeing_01_p_ef_heart", PgParticle::O_SCALE,1.0f );
				if(pkParticle)
				{
					if(!pkActorPet->AttachTo(1199, "p_ef_color", pkParticle))
					{ //캐릭터 nif에 p_ef_color 더미가 없다면
						pkActorPet->AttachTo(1199, "char_root", pkParticle); //그냥 기본 노드에 붙히자
					}
				}

				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Pet_Dyeing", 0.0f, 0.0f, 0.0f, pkActor);

				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pkPlayer)	{break;}
				if(pkPlayer->SelectedPetID()!=pkUnit->GetID())	{break;}
				XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"CharInfo");
				if( pWnd && !pWnd->IsClosed() )
				{
					g_kMyActorViewMgr.ChangePetColor("PetActor", pkActorPet->GetGuid());
				}
			}
		}break;
	case AT_AWAKE_STATE:
		{
			if( EAS_MAX_GUAGE == rkAbilInfo.iValue )
			{
				PgActor* pkViewActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if( pkViewActor 
					&& !pkViewActor->IsMyActor()
					)
				{
					PgActorUtil::ExpressAwakeMaxState(pkViewActor);
				}
			}
		}break;
	case AT_AWAKE_VALUE:
		{
			if( pkActor->IsMyActor() )
			{
				lwUpdateCheckDrawLimitBreakBar();
			}
		}break;
	case AT_SKILLSET_USE_COUNT:
		{
			if( pkActor->IsMyActor() )//내꺼
			{
				lwSkillSet::lwSkillSetInit(lwUIWnd(XUIMgr.Get(L"SFRM_SKILLSET")));
			}
		}break;
	case AT_STRATEGYSKILL_OPEN:
		{
			if( pkActor->IsMyActor() )//내꺼
			{
				lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
			}
		}
	}


	if( UT_PET != pkUnit->UnitType() )
	{
		if(pkActor->IsUnderMyControl())
		{
			lwCharInfo::UpdateMyActor();
		}
	}
	else
	{
		if(pkActor->GetCallerIsMe())
		{
			XUI::CXUI_Wnd* pkInfo = XUIMgr.Get(L"CharInfo");
			if(pkInfo)
			{
				XUI::CXUI_Wnd* pkPetInfo = pkInfo->GetControl(L"FRM_PET_INFO");
				if(pkPetInfo)
				{
					lwCharInfo::lwSetPetInfoToUI(pkPetInfo, lwGUID(pkUnit->GetID()));
				}
			}
		}
	}
}

void Recv_PT_M_C_NFY_QUICKSLOTCHANGE(BM::Stream &rkPacket)// 패킷으로 받음
{
	size_t slot_idx;
	SQuickInvInfo kQuickInvInfo;

	rkPacket.Pop(slot_idx);
	rkPacket.Pop(kQuickInvInfo);

	PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
	if(!pkPilot){return;}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgQuickInventory *pkQInven = pkPlayer->GetQInven();
	if(!pkQInven){return;}

	pkQInven->Modify(slot_idx, kQuickInvInfo);
	
	lwUIWnd kQuick = lwGetUIWnd("QuickInv");	//새로 그려주자
	if( !kQuick.IsNil() )
	{
		kQuick.SetInvalidate();
	}
}

//>>local function
bool ModifyItem(EItemModifyParentEventType const kCause, BM::GUID const &rkOwnCharGuid, EItemModifyParentEventType const kEventType, PgBase_Item const &rkItem, int const iDiffCount, bool const bMine)
{
	std::wstring kItemName;
	bool const bMakeItemName = MakeItemName(rkItem.ItemNo(), rkItem.EnchantInfo(), kItemName);
	if( !bMakeItemName )
	{
		return false;
	}

	switch( kCause )
	{//전처리다
	case IMEPT_MISSIONBONUS:
	case MCE_EXP:
		{
			lwAccumlationExpCard::lwUdateAccumulateCardUsing(rkItem.Guid());
			return true;
		}break;
	case CIE_Repair:
		{
			if( bMine )
			{
				BM::vstring vStr(TTW(1237));
				vStr.Replace(L"#NAME#", kItemName);
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_ITEM), vStr);
				return true;
			}
		}break;
	case CIE_Divide:
	case IMEPT_EVENTQUEST:
		{
		}break;
	case CIE_Inventory_Sort:
		{
			if( MIE_AddItem == kEventType
			||	MIE_Destroy	== kEventType )
			{
				return false;
			}
		}break;
	case CIE_MoveFromGuildInvToGuildInv:
	case CIE_MoveFromGuildInvToInv:
	case CIE_MoveFromInvToGuildInv:
		{
			return true;
		}break;
	case CIE_Constellation:
		{
			 if( MIE_UseItem == kEventType )
			 {
				 g_kConstellationEnterUIMgr.UpdateRewardInfo();
			 }
		}break;
	default:
		{
			if( PgEventQuestUI::IsInTargetItem(rkItem.ItemNo()) )
			{
				PgEventQuestUI::CheckTargetItem();
			}
		}break;
	}

	int const iTempSize = 512;

	if( !iDiffCount )
	{
		return false;
	}
	if( !bMine )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pkItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
	if( !pkItemDef )
	{
		return false;
	}

	int const iAbsCount = (pkItemDef->CanEquip() || (false == pkItemDef->IsAmountItem())) ? 1: abs(iDiffCount);

	EChatType iChatType = CT_EVENT_SYSTEM;
	int iTTW = 0;
	bool bShowNotice = false;
	bool bShowItemAlam = false;
	switch( kEventType )
	{
	case MIE_AddItem:
		{
			iChatType = CT_ITEM;
			if( 0 > iDiffCount )
			{
				switch( kCause )
				{
				case IMEPT_QUEST_DIALOG_End:
					{
						iTTW = (1 < iAbsCount)? 700512: 700511;
					}break;
				default:
					{
						iTTW = (1 < iAbsCount)? 700047: 700046;
					}break;
				}
			}
			else
			{
				iTTW = (1 < iAbsCount)? 700043: 700042;
			}

			bShowNotice = (CIE_OpenChest == kCause) || (CIE_OpenPack == kCause) || (CIE_OpenPack2 == kCause);
		}break;
	case MIE_UseItem:
		{
			iChatType = CT_ITEM;
			iTTW = (1 < iAbsCount)? 700047: 700046;
		}break;
	case MIE_Destroy:
		{
			iChatType = CT_ITEM;
			switch( kCause )
			{
			case CIE_Exchange:
				{
					iTTW = (1 <iAbsCount)? 700410: 700409;
				}break;
			case CIE_Home_Equip:
				{
					iTTW = 700046;
				}break;
			default:
				{
					iTTW = (1 <iAbsCount)? 700045: 700044;
				}break;
			}
		}break;
	case MIE_Equip:
		{
			iTTW = 700040;
		}break;
	case MIE_UnEquip:
		{
			iTTW = 700041;
		}break;
	case MIE_Modify:
	case MIE_Move:
	case IMEPT_NONE:
	default:
		{
			iTTW = 0;
		}break;
	}

	if( !iTTW )
	{
		return false;
	}

	std::wstring kLogStr;
	if( 1 < iAbsCount )
	{
		FormatTTW(kLogStr, iTTW, kItemName.c_str(), iAbsCount);
	}
	else
	{
		FormatTTW(kLogStr, iTTW, kItemName.c_str());
	}

	if( MIE_UseItem == kEventType && CIE_UM_Article_Buy != kCause)
	{
		if ( UICT_REVIVE == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{
			if( (ICMET_Cant_SelfUse & pkItemDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_SelfUse )
			{
				FormatTTW(kLogStr, 80032, kItemName.c_str());
				bShowNotice = true;
			}
		}
	}

	if( !kLogStr.empty() )
	{
		if(SOUL_ITEM_NO == rkItem.ItemNo() )
		{
			bShowNotice = true;
		}
		else
		{
			if( iDiffCount > 0 )
			{
				const int iTTW_ID = 790127;
				std::wstring kName;
				GetItemName( rkItem.ItemNo(), kName );

				switch( kCause )
				{
				case CIE_Revive_Feather_Login:
				case CIE_Revive_Feather_LevelUp:
				case CIE_Login_Event:
				case CIE_GemStore_Buy:
				case CIE_CoinChange_Buy:
					{
						bShowItemAlam = true;
						bShowNotice = false;
					}break;
				case CIE_Open_Gamble:
					{
						bShowItemAlam = false;
						bShowNotice = true;
					}break;
				case CIE_OpenPack:
				case CIE_OpenPack2:
					{//팩을 오픈
						switch( rkItem.ItemNo() )
						{
						case ITEM_NO_TOPAZ:
						case ITEM_NO_EMERALD:
						case ITEM_NO_SAPPHIER:
						case ITEM_NO_RUBY:
						case ITEM_NO_DIAMOND:
							{	
								bShowItemAlam = true;
								bShowNotice = false;
							}break;
						default:
							{
								bShowItemAlam = false;
								bShowNotice = true;
							}break;
						}
					}break;				
				case CIE_Manufacture:
					{
						bShowItemAlam = true;
						bShowNotice = false;
					}break;
				default:
					{
					}break;
				}

				if( bShowItemAlam )
				{// 아이템 획득을 UI로 표시해 준다.
					wchar_t buf[100] = {0,};
					BM::vstring vStr;
					int iLocationY = 0;

					if( CIE_Revive_Feather_LevelUp == kCause )
					{
						iLocationY = ( XUI::EXV_DEFAULT_SCREEN_HEIGHT * 0.5 ); // TODO: XUIMgr.GetResolution().y 로 대체 할 것.
						vStr += TTW(700412);
						vStr += L"\n";
					}
					else if( CIE_Manufacture == kCause )
					{
						iLocationY -= 100;
					}

					vStr += kName;
					wsprintfW(buf, TTW(iTTW_ID).c_str(), UNI(vStr), iDiffCount);
					CallGetItemAlam(buf, rkItem.ItemNo(), 5000, iLocationY);
				}
			}
		}
		SChatLog kChatLog(iChatType);
		switch( kCause )
		{
		case CIE_Mission:
		case CIE_Mission1:
		case CIE_Mission2:
		case CIE_Mission3:
		case CIE_Mission4:
			{
				kChatLog.DelayCause( CIE_Mission );
			}break;
		
		default:
			{
			}break;
		}
		if(CIE_Divide != kCause)
		{// 수량 나누기가 아니라면 메세지를 남김
		 // 메세지 외에도 아래에서 다른 이벤트 처리를 하고 있어 이런식으로 작성함
			g_kChatMgrClient.AddLogMessage(kChatLog, kLogStr, bShowNotice);
		}
	}


	//
	// 로그 출력외에 다른 이벤트 처리
	switch( kEventType )
	{
	case MIE_AddItem:
		{

			//
			if( 0 < iDiffCount )
			{
				// Class Limit
				PgPilot	*pkPilot = g_kPilotMan.FindPilot(rkOwnCharGuid);
				if (pkPilot)
				{
					CUnit* pkUnit = pkPilot->GetUnit();

					if( pkUnit )
					{
						if( pkItemDef->CanEquip() )
						{
							__int64 const i64ClassLimit = pkItemDef->GetAbil64(AT_CLASSLIMIT);
							if (! IS_CLASS_LIMIT(i64ClassLimit, pkUnit->GetAbil(AT_CLASS)))
							{
								// 타클래스
								g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_CLASS_ITEM), 0);
							}
							else
							{
								g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_ITEMNO), rkItem.ItemNo());
							}
						}
						else
						{
							g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_ITEMNO), rkItem.ItemNo());
						}
					}
				}
			}

			if( g_kPilotMan.IsMyPlayer(rkOwnCharGuid) )	//내꺼면 퀵인벤 갱신
			{
				XUI::CXUI_Wnd *pkQuick = XUIMgr.Get(L"QuickInv");
				if(pkQuick)
				{
					pkQuick->SetInvalidate(true);
				}
			}

			if( IG_CURSE == ::GetItemGrade(rkItem) )
			{
				g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_CURSEITEM), 0);
			}

			//
			//switch( kCause )
			//{
			//case CIE_OpenChest:
			//	{
			//		PgBase_Item kItemInfo = rkItem;
			//		if(pkItemDef->IsAmountItem())
			//		{//수량아이템이면 수량 표시.
			//			kItemInfo.Count( iDiffCount );
			//		}
			//		SIconInfo kIconInfo(KUIG_VIEW_ONLY_ITEM, rkItem.ItemNo());
			//		lwGuild::ShowSingleResultWindow(kIconInfo, kLogStr, std::wstring(), false, &kItemInfo);

			//		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
			//		if( pkMyActor )
			//		{
			//			pkMyActor->PlayNewSound(NiAudioSource::TYPE_3D, "WoodenBox_Open", 0.f);
			//		}
			//	}break;
			//}
		}break;
	case MIE_UseItem:
		{
			PgPilot *pkPilot = g_kPilotMan.FindPilot(rkOwnCharGuid);
			if(pkPilot)
			{
				PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
				if( pkActor )
				{
					switch(rkItem.ItemNo())
					{
					case ITEM_NO_BIND_ITEM:
						{// 자물쇠 사용시 소리 
							pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Binding_Lock", 0.0f);
						}break;
					case ITEM_NO_UNBIND_GOLD:
					case ITEM_NO_UNBIND_PLATINUM:
						{// 열쇠 사용시 소리 
							pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Binding_Open", 0.0f);
						}break;
					default:
						{
							pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Use_UseItem", 0.0f);
						}break;
					}
				}

				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
				if(pkPlayer)
				{
					if( CIE_Divide != kCause )
					{
						pkPlayer->GetInven()->AddCoolTime(rkItem.ItemNo(),PgInventory::EICool_UseItem,lwGetServerElapsedTime32(),pkPlayer->GetAbil(AT_R_COOLTIME_RATE_ITEM));
					}
				}
			}
			
			if(g_kPilotMan.IsMyPlayer(rkOwnCharGuid))	//내꺼면 퀵인벤 갱신
			{
				XUI::CXUI_Wnd *pkQuick = XUIMgr.Get(L"QuickInv");
				if(pkQuick)
				{
					pkQuick->SetInvalidate(true);
				}
			}
		}break;
	case MIE_Equip:
		{
			// 안내 메시지가 있다면 출력해서 보여준다.
			int const iEquipMsg = pkItemDef->GetAbil(AT_EQUIP_MESSAGE);
			if (iEquipMsg > 0)
			{
				std::wstring strEquipMsg = TTW(iEquipMsg);
				if (strEquipMsg.size() == 0)
				{
					PgError2("[%s] Cannot find EquipMsg[%d]", __FUNCTION__, iEquipMsg);
				}
				else
				{
					TextChange_Item(strEquipMsg, pkItemDef);
					SChatLog kEquipChat(CT_ITEM);
					g_kChatMgrClient.AddLogMessage(kEquipChat, strEquipMsg);
				}
			}
		}break;
	}

	return true;
}
namespace SystemMessageUtil
{
	class PgSystemLog_Item 
		: public PgBase_Item
	{
	public:
		PgSystemLog_Item()
		{
		}
		PgSystemLog_Item(PgBase_Item const& rhs, int const iDiffCount, BM::GUID const& rkOwnerGuid)
			:PgBase_Item(rhs), m_kTotalCount(iDiffCount), m_kOwnerGuid(rkOwnerGuid)
		{
		}

		virtual ~PgSystemLog_Item()
		{
		}

		CLASS_DECLARATION_S(int, TotalCount);
		CLASS_DECLARATION_S_NO_SET(BM::GUID, OwnerGuid);
	};

	class PgSystemLogKey
	{
	public:
		PgSystemLogKey(int const iItemNo,BM::GUID const& rkItemGuid, BM::GUID const& rkOwnerGuid)
			: m_kItemNo(iItemNo),m_kItemGuid(rkItemGuid),m_kOwnerGuid(rkOwnerGuid)
		{
		}
		~PgSystemLogKey()
		{
		}

		bool operator < (PgSystemLogKey const& rhs)const
		{
			if( m_kItemNo < rhs.m_kItemNo )
			{
				return true;
			}
			
			if( m_kItemGuid < rhs.m_kItemGuid )
			{
				return true;
			}

			if( m_kOwnerGuid < rhs.m_kOwnerGuid)
			{
				return true;
			}

			return false;
		}

		CLASS_DECLARATION_S_NO_SET(int, ItemNo);
		CLASS_DECLARATION_S_NO_SET(BM::GUID, ItemGuid);
		CLASS_DECLARATION_S_NO_SET(BM::GUID, OwnerGuid);
	};
	typedef std::map< PgSystemLogKey, PgSystemLog_Item > ContLogItem;
};
//<<local function

void Recv_PT_M_C_NFY_PET_INVENTORY_ACTION(BM::Stream &rkPacket)
{
	BM::GUID kPetID;
	rkPacket.Pop( kPetID );
	if ( PgActorPet::ms_pkMyPetInventory && PgActorPet::ms_pkMyPetInventory->OwnerGuid() == kPetID )
	{
		EItemModifyParentEventType kEventCause = IMEPT_NONE;
		DB_ITEM_STATE_CHANGE_ARRAY kArray;
		rkPacket.Pop(kEventCause);
		PU::TLoadArray_M( rkPacket, kArray);

		DB_ITEM_STATE_CHANGE_ARRAY::const_iterator item_itr = kArray.begin();
		for( ; item_itr != kArray.end() ; ++item_itr )
		{
			DB_ITEM_STATE_CHANGE_ARRAY::value_type const &rkCurModify = *item_itr;

			PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();
			PgActorPet::ms_pkMyPetInventory->Modify(rkItemWrapper.Pos(), rkItemWrapper);
		}

		lwViewPetEquip();
	}
}

void Recv_PT_M_C_NFY_ITEM_CHANGE(BM::Stream *pkPacket)
{
	EItemModifyParentEventType kEventCause = IMEPT_NONE;
	BM::GUID kGuid;
	DB_ITEM_STATE_CHANGE_ARRAY kArray;
	bool bItemRepair = false;

	pkPacket->Pop(kEventCause);
	pkPacket->Pop(kGuid);
	PU::TLoadArray_M( *pkPacket, kArray);

	bool const bMyPilot = g_kPilotMan.IsMyPlayer(kGuid);
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);
	if( !pkPilot )
	{
		return;
	}

	PgControlUnit* pkControlUnit = dynamic_cast<PgControlUnit*>(pkPilot->GetUnit());
	if( !pkControlUnit )
	{
		return;
	}

	PgInventory *pkInv = pkControlUnit->GetInven();
	if( !pkInv )
	{
		return;
	}

	PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
	//if( !pkActor )	//홈 아이템이 들어감에 따라 여기서 튕겨내면 안됨
	//{
	//	return;
	//}

	_PgOutputDebugString("[Recv_PT_M_C_NFY_ITEM_CHANGE] Actor:%s EventType Size : %d\n",MB(pkControlUnit->GetID().str()), kArray.size());

	SystemMessageUtil::ContLogItem kDelayItemUseLog;

	bool bCallNpcEvent = false;
	bool bModifyPos = false;
	bool bRefreshAbil = false;
	bool bNeedQuestUIUpdate = false;
	bool bNeedReBuildSkillTree = false;
//	bool bSafeFoamUseClear = false;
	bool bChangePetItem = false;
	bool bNeedUpdateMonsterName = false;
	int iCompleteQuestID = 0;
	DB_ITEM_STATE_CHANGE_ARRAY::const_iterator item_itor = kArray.begin();
	while(kArray.end() != item_itor )
	{
		DB_ITEM_STATE_CHANGE_ARRAY::value_type const &rkCurModify = (*item_itor);

		BM::Stream kAddonData = rkCurModify.kAddonData;

		switch( rkCurModify.State() )
		{
		case DISCT_SET_HOME_ADDR:
			{
				if( g_kPilotMan.IsMyPlayer(kGuid) )
				{
					PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
					if ( pkPlayer )
					{
						SMOD_MyHome_MyHome_Set_Addr kData;
						kData.ReadFromPacket(kAddonData);
						pkPlayer->HomeAddr(SHOMEADDR(kData.StreetNo(),kData.HouseNo()));
					}
				}
			}break;
		case DISCT_SET_HOME_STATE:
			{	
				PgMyHome * pkMyHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
				if( !pkMyHome )
				{
					break;
				}

				BYTE byState = 0;
				kAddonData.Pop(byState);

				pkMyHome->SetAbil(AT_MYHOME_STATE, byState);
			}break;
		case DISCT_SET_HOME_VISITFLAG:
			{
				PgMyHome * pkMyHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
				if( !pkMyHome )
				{
					break;
				}

				BYTE byVisitFlag = 0;
				kAddonData.Pop(byVisitFlag);

				pkMyHome->SetAbil(AT_MYHOME_VISITFLAG, byVisitFlag);
			}break;
		case DISCT_SET_HOME_OWNER_INFO:
			{
				SMOD_MyHome_MyHome_Set_OwnerInfo kData;
				kData.ReadFromPacket(kAddonData);

				PgPilot *pkPilot = g_kPilotMan.FindPilot(rkCurModify.RetOwnerGuid());
				if( !pkPilot )
				{
					break;
				}

				PgMyHome * pkMyHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
				if( !pkMyHome )
				{
					break;
				}

				pkMyHome->OwnerGuid(kData.OwnerGuid());
				pkMyHome->OwnerName(kData.OwnerName());
			}break;
		case DISCT_SET_HOME_DEFAULT_ITEM:
			{
				SMOD_SetHomeStyle kData;
				kData.ReadFromPacket(kAddonData);

				PgPilot *pkPilot = g_kPilotMan.FindPilot(rkCurModify.RetOwnerGuid());
				if( !pkPilot )
				{
					break;
				}

				PgMyHome * pkMyHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
				if( !pkMyHome )
				{
					break;
				}

				pkMyHome->SetDefaultItem(kData.EquipPos(),kData.ItemNo());
				if( g_pkWorld )
				{
					if(g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 내부
										{
						if( pkMyHome->GetID() == g_pkWorld->GetHome()->GetHomeGuid() )// id 비교
						{
							if( EQUIP_POS_ROOM_WALL == kData.EquipPos() || EQUIP_POS_ROOM_FLOOR == kData.EquipPos() )
							{
								g_pkWorld->ReleaseHome();
								PgHome* pkHome = g_pkWorld->CreateHome();
								if( pkHome )
								{
									pkHome->SetHomeUnit(pkMyHome);
									pkHome->HomeUnitAddProcess();
								}
							}
						}						
					}
					else if(g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
					{
						PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(pkMyHome->GetID()));
						if(pkHouse)
						{
							PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();//Current data
							pkHouse->EquipExteria(kData.ItemNo(), NiPoint3(0,0,0));
						}
					}
				}
			}break;
		case DISCT_ADD_EMOTION:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					SPMOD_AddEmotion kData;
					kData.ReadFromPacket(kAddonData);
					PgBitArray<MAX_DB_EMOTICON_SIZE> kCont;
					if(true == pkPlayer->GetContEmotion(kData.Type(),kCont))
					{
						kCont.Set(kData.GroupNo(),true);
						pkPlayer->SetContEmotion(kData.Type(),kCont);
					}
				}
			}break;
		case DISCT_CREATE_PORTAL:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					SMOD_Portal_Create kData;
					kData.ReadFromPacket(kAddonData);
					pkPlayer->InsertPortal(kData.Guid(),CONT_USER_PORTAL::mapped_type(kData.Comment(),kData.GroundNo(),kData.Pos()));
				}
			}break;
		case DISCT_DELETE_PORTAL:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					SMOD_Portal_Delete kData;
					kData.ReadFromPacket(kAddonData);
					pkPlayer->RemovePortal(kData.Guid());
				}
			}break;
		case DISCT_MODIFY_CARDCOMMENT:
			{
				if( !bMyPilot )
				{
					break;
				}

				SMOD_Modify_Comment kData;
				kData.ReadFromPacket(kAddonData);
				g_kCharacterCardInfo.Comment(kData.Comment());
				lwCharacterCard::lwSetMyCharacterCard();
			}break;
		case DISCT_MODIFY_CARDSTATE:
			{
				if( !bMyPilot )
				{
					break;
				}

				SMOD_Modify_CardState kData;
				kData.ReadFromPacket(kAddonData);
				g_kCharacterCardInfo.Enable(kData.Enable());
				lwCharacterCard::lwSetMyCharacterCard();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", kData.Enable() ? 51056 : 51055 , true);
				XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"CharInfo");
				if( pMainUI )
				{
					XUI::CXUI_Wnd* pParent = pMainUI->GetControl(L"FRM_CHAR_CARD");
					if( pParent )
					{
						XUI::CXUI_Wnd* pHideBtn = pParent->GetControl(L"BTN_ACTION");
						if( pHideBtn )
						{
							pHideBtn->Text( TTW(kData.Enable() ? 51020 : 51017) );
						}
					}
				}
			}break;
		case DISCT_CREATE_CHARACTERCARD:
			{
				if( !bMyPilot )
				{
					break;
				}

				SMOD_CreateCard kData;
				kData.ReadFromPacket(kAddonData);
				g_kCharacterCardInfo.BGndNo(kData.BGndNo());
				g_kCharacterCardInfo.Blood(kData.Blood());
				g_kCharacterCardInfo.Comment(kData.Comment());
				g_kCharacterCardInfo.Constellation(kData.Constellation());
				g_kCharacterCardInfo.Hobby(kData.Hobby());
				g_kCharacterCardInfo.Local(kData.Local());
				g_kCharacterCardInfo.Sex(kData.Sex());
				g_kCharacterCardInfo.Style(kData.Style());
				g_kCharacterCardInfo.Year(kData.Year());
				lwCharacterCard::lwSetMyCharacterCard();
			}break;
		case DISCT_MODIFY_CHARACTERCARD:
			{
				if( !bMyPilot )
				{
					break;
				}

				SMOD_ModifyCard kData;
				kData.ReadFromPacket(kAddonData);
				g_kCharacterCardInfo.BGndNo(kData.BGndNo());
				g_kCharacterCardInfo.Blood(kData.Blood());
				g_kCharacterCardInfo.Comment(kData.Comment());
				g_kCharacterCardInfo.Constellation(kData.Constellation());
				g_kCharacterCardInfo.Hobby(kData.Hobby());
				g_kCharacterCardInfo.Local(kData.Local());
				g_kCharacterCardInfo.Sex(kData.Sex());
				g_kCharacterCardInfo.Style(kData.Style());
				g_kCharacterCardInfo.Year(kData.Year());
				lwCharacterCard::lwSetMyCharacterCard();
			}break;
		case DISCT_MODIFY_POPULARPOINT:
			{
				if( !bMyPilot )
				{
					break;
				}

				__int64 iTotalPoint = 0;
				__int64 iTodayPoint = 0;
				int iPoint = 0;
				kAddonData.Pop(iTotalPoint);
				kAddonData.Pop(iTodayPoint);
				g_kCharacterCardInfo.PopularPoint(iTotalPoint);
				g_kCharacterCardInfo.TodayPopularPoint(iTodayPoint);
			}break;
		case DISCT_MODIFY_REFRESHDATE:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					BM::PgPackedTime kDate;
					kAddonData.Pop(kDate);
					pkPlayer->RefreshDate(kDate);
				}
			}break;
		case DISCT_MODIFY_RECOMMENDPOINT:
			{
				if( bMyPilot )
				{
					PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
					if ( pkPlayer )
					{
						int iRecommandPoint = 0;
						kAddonData.Pop(iRecommandPoint);
						pkPlayer->RecommendPoint(iRecommandPoint);
					}
				}
			}break;
		case DISCT_MODIFY_STRATEGYSKILL:
			{
				if( bMyPilot )
				{
					PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
					if( pkPlayer && pkActor )
					{
						int iValue = 0;
						iValue = 0; kAddonData.Pop(iValue); pkPlayer->SetAbil(AT_STRATEGYSKILL_TABNO, iValue);
						iValue = 0; kAddonData.Pop(iValue); pkPlayer->SetAbil(AT_SP, iValue);
						pkPlayer->GetMySkill()->ReadFromPacket(kAddonData);
						pkPlayer->GetQInven()->ReadFromPacket(kAddonData);

						pkActor->SkillSetAction().ContSkillSet( pkPlayer->GetMySkill()->ContSkillSet() );

						g_kSkillTree.CreateSkillTree( pkPlayer );

						lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
						lwSkillSet::lwSkillSetInit(lwUIWnd(XUIMgr.Get(L"SFRM_SKILLSET")));
						lwUIWnd kQuick = lwGetUIWnd("QuickInv");	//새로 그려주자
						if( !kQuick.IsNil() )
						{
							kQuick.SetInvalidate();
						}
					}
				}
			}break;
		case DISCT_INVENTORY_EXTEND:
			{
				if( !bMyPilot )
				{
					break;
				}

				EInvType kInvType;
				BYTE kReductionNum;
				kAddonData.Pop(kInvType);
				kAddonData.Pop(kReductionNum);
				pkInv->InvExtend(kInvType,kReductionNum);

				switch(kInvType)
				{
				case IT_EQUIP:
				case IT_CONSUME:
				case IT_ETC:
				case IT_CASH:
					{
						lua_tinker:: call<void, int, bool>("CommonMsgBoxByTextTable", 406105, true);
						if( g_iInvViewGrp == kInvType )
						{
							XUI::CXUI_Wnd* pInv = XUIMgr.Get(_T("Inv"));
							if( pInv )
							{
								lwSetInvViewSize(lwUIWnd(pInv));
							}
						}
					}break;
				default:
					{}break;
				}
			}break;
		case DISCT_EXTEND_MAX_IDX:
			{
				if( !bMyPilot )
				{
					break;
				}

				EInvType kInvType;
				BYTE kReductionNum;
				kAddonData.Pop(kInvType);
				kAddonData.Pop(kReductionNum);
				pkInv->ExtendMaxIdx(kInvType,kReductionNum);

				switch(kInvType)
				{
				case IT_EQUIP:
				case IT_CONSUME:
				case IT_ETC:
				case IT_CASH:
					{
						lua_tinker:: call<void, int, bool>("CommonMsgBoxByTextTable", 406105, true);
						if( g_iInvViewGrp == kInvType )
						{
							XUI::CXUI_Wnd* pInv = XUIMgr.Get(L"Inv");
							if( pInv )
							{
								lwSetInvViewSize(lwUIWnd(pInv));
								lwSetReqInvWearableUpdate(true);
							}
						}
					}break;
				case IT_SAFE:
				case IT_RENTAL_SAFE1:
				case IT_RENTAL_SAFE2:
				case IT_RENTAL_SAFE3:
					{
						lua_tinker:: call<void, int, bool>("CommonMsgBoxByTextTable", 406105, true);
						XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_INV_SAFE");
						if( pkMainUI && !pkMainUI->IsClosed() )
						{
							BM::vstring kBtnStr(L"CBTN_BAG");
							switch( kInvType )
							{
							case IT_SAFE:
								{
									kBtnStr += 0;
								}break;
							case IT_RENTAL_SAFE1:
							case IT_RENTAL_SAFE2:
							case IT_RENTAL_SAFE3:
								{
									kBtnStr += kInvType - IT_RENTAL_SAFE1 + 1;
								}break;
							}

							XUI::CXUI_CheckButton* pkCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkMainUI->GetControl(kBtnStr));
							if( pkCBtn )
							{
								if( pkCBtn->Check() )
								{
									lwChangeCashBoxViewGroup(lwUIWnd(pkMainUI), pkCBtn->BuildIndex());
								}
							}
						}
					}break;
				default:
					{}break;
				}
			}break;
		case DISCT_SET_DEFAULT_ITEM:
			{
				int iEquipLimit = 0;
				int iItemNo = 0;
				kAddonData.Pop(iEquipLimit);
				kAddonData.Pop(iItemNo);
				if(pkActor)
				{
					switch( iEquipLimit )
					{
					case EQUIP_POS_HAIR_COLOR:
						{// 머리카락 색상이 바뀌어질때
							pkActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo);	// 자신의 색상을 변경하고
							int iClass = pkPilot->GetAbil(AT_CLASS);
							if( IS_CLASS_LIMIT(UCLIMIT_COMMON_DOUBLE_FIGHTER, iClass) 
								|| pkControlUnit->UnitType() == UT_SUB_PLAYER 
								)
							{// 쌍둥이 보조 캐릭터가 있으면
								PgActor* pkSubplayerActor = PgActorUtil::GetSubPlayerActor(pkActor);
								if(pkSubplayerActor)
								{// 쌍둥이 보조 캐릭터도 머리 색상을 바꾸어 준다
									pkSubplayerActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo);
								}
							}
						}break;
					default:
						{
							pkActor->EquipItemProc(iItemNo,true,NULL,PgItemEx::LOAD_TYPE_INSTANT);
						}break;
					}
				}
			}break;
		case DISCT_ADD_UNBIND_DATE:
			{
				BM::GUID kItemGuid;
				BYTE kDelayDays = 0;
				BM::DBTIMESTAMP_EX kDBTimeEx;
				kAddonData.Pop(kItemGuid);
				kAddonData.Pop(kDelayDays);
				kAddonData.Pop(kDBTimeEx);
				pkInv->AddUnbindDate(kItemGuid,kDBTimeEx);
			}break;
		case DISCT_DEL_UNBIND_DATE:
			{
				BM::GUID kItemGuid;
				kAddonData.Pop(kItemGuid);
				pkInv->DelUnbindDate(kItemGuid);
			}break;
		case DISCT_REMOVE_SYSTEM:
			{
			}break;
		case DISCT_SET_RENTALSAFETIME:
			{
				EInvType kInvType;
				BM::DBTIMESTAMP_EX kDBTime;
				kAddonData.Pop(kInvType);
				kAddonData.Pop(kDBTime);
				
				HRESULT	hRet = pkInv->CheckEnableUseRentalSafe(kInvType);
				pkInv->SetRentalSafeEnableTime(kInvType,kDBTime);
				if( hRet != S_OK )
				{
					UpdateCashBoxViewGroup();
				}
			}break;
		case DISCT_ACHIEVEMENT_TIMELIMIT_MODIFY:
			{
				int iSaveIdx = 0;
				BM::PgPackedTime kPackedTime;
				kAddonData.Pop(iSaveIdx);
				kAddonData.Pop(kPackedTime);

				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkControlUnit);
				if ( pkPlayer )
				{
					pkPlayer->GetAchievements()->SetAchievementTimeLimit(iSaveIdx,kPackedTime);
				}
			}break;
		case DISCT_ACHIEVEMENT_TIMELIMIT_DELETE:
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkControlUnit);
				if ( pkPlayer )
				{
					int iSaveIdx = 0;
					kAddonData.Pop(iSaveIdx);
					pkPlayer->GetAchievements()->ResetAchievementTimeLimit(iSaveIdx);
				}
			}break;
		case DISCT_SET_ACHIEVEMENT:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					int iSaveIdx = 0;
					BYTE bValue = 0;

					kAddonData.Pop(iSaveIdx);
					kAddonData.Pop(bValue);
	
					if(bValue)
					{
						pkPlayer->GetAchievements()->Complete(iSaveIdx);
					}
					else
					{
						if( pkPlayer->GetAchievements()->Reset(iSaveIdx) )
						{
							if( bMyPilot )
							{
								// 메달 정보를 얻어 온다
								DWORD AchieveType = 0;
								DWORD AchieveIdx = 0;
								if( !g_kAchieveNfyMgr.SIdxToIdxAndType(iSaveIdx, AchieveType, AchieveIdx) )
								{
									return;
								}
		
								TBL_DEF_ACHIEVEMENTS	kAchievementInfo;
								if( !lwAchieveNfyMgr::GetAchievementInfo(AchieveType, AchieveIdx, kAchievementInfo) )
								{
									return;
								}

								std::wstring wstrText = lwAchieveNfyMgr::GetAchievementString(kAchievementInfo.iIdx, kAchievementInfo.iTitleNo);

								BM::vstring vStr(TTW(2805));
								vStr.Replace(L"#TITLE#", wstrText);

								g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), vStr, true, EL_Warning);
							}
						}
					}
				}
			}break;
		case DISCT_ACHIEVEMENT2INV:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					int iSaveIdx = 0;
					kAddonData.Pop(iSaveIdx);
					pkPlayer->GetAchievements()->ResetItem(iSaveIdx,true);
					if( g_kPilotMan.IsMyPlayer(pkPlayer->GetID()) )
					{					
						lwUIBook::UpdateAchieveItem(iSaveIdx, true);
					}
				}
			}break;
		case DISCT_INV2ACHIEVEMENT:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					int iSaveIdx = 0;
					kAddonData.Pop(iSaveIdx);
					pkPlayer->GetAchievements()->ResetItem(iSaveIdx,false);
					if( g_kPilotMan.IsMyPlayer(pkPlayer->GetID()) )
					{					
						lwUIBook::UpdateAchieveItem(iSaveIdx, false);
					}
				}
			}break;
		case DISCT_COMPLETE_ACHIEVEMENT:
			{
				if( bMyPilot )
				{
					PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
					if ( pkPlayer )
					{
						SPMOD_Complete_Achievement kData;
						kData.ReadFromPacket( kAddonData );
						pkPlayer->GetAchievements()->Complete(kData.SaveIdx());
						pkPlayer->GetAchievements()->AddAchievementPoint(static_cast< EAchievementsCategory >(kData.Category()), kData.RankPoint());
						g_kAchieveNfyMgr.AddShowAchieve(kData.SaveIdx());
						g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_ACHIEVE), 0);
					}
				}
			}break;
		case DISCT_MODIFY_VALUE:
			{
				int iAbilType = 0;
				kAddonData.Pop(iAbilType);
				switch(iAbilType)
				{
				case AT_EXPERIENCE:
					{// 플레이어 경험치 변화는 이곳에서 처리함(휴식경험치 때문에 PT_M_C_NFY_STATE_CHANGE64에서 옮겨옴)
						__int64 i64NewTotalExp = 0;
						kAddonData.Pop(i64NewTotalExp);
						SAbilInfo64 const kAbilInfo64(static_cast<WORD>(iAbilType), i64NewTotalExp);
						Recv_PT_M_C_NFY_STATE_CHANGE64(pkPilot, kAbilInfo64);
					}break;
				default:
					{
					}break;
				}
			}break;
		case DISCT_MODIFY_VALUEEX:
			{
				if( bMyPilot )
				{// 내 플레이어의
					PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
					if ( pkPlayer )
					{
						int iAbilType = 0;
						kAddonData.Pop(iAbilType);
						switch(iAbilType)
						{// 휴식 경험치 적용된 경험치를 얻을때
						case AT_EXPERIENCE:
							{
								__int64 i64NewTotalExp = 0;
								__int64 i64EarnExp = 0;
								__int64 i64EarnRestBonusExp = 0;

								bool bRestMode = false;			
								kAddonData.Pop(i64NewTotalExp);		// 갱신된 플레이어의 전체 경험치
								kAddonData.Pop(i64EarnExp);			// 얻은 일반 경험치
								kAddonData.Pop(i64EarnRestBonusExp);// 휴식 모드로 얻은 경험치
								
								pkPlayer->SetAbil64(iAbilType, i64NewTotalExp);
								if(0 == i64EarnExp
									&& 0 == i64EarnRestBonusExp)
								{
									break;
								}
								{// 채팅창에 경험치 메세지 
									TCHAR szTemp[512] = {0 ,};
									std::wstring kTTWForm;
									SChatLog kChatLog(CT_EXP);
									kTTWForm = TTW(799022);
									_sntprintf_s(szTemp, 512, kTTWForm.c_str(), i64EarnExp, i64EarnRestBonusExp);
									std::wstring kLog = szTemp;
									g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
								}
								{// 머리위에 뜨는 exp 숫자
									PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
									if(pkActor)
									{
										pkActor->ShowExpNum(static_cast<int>(i64EarnExp+i64EarnRestBonusExp));
									}
								}
							}break;
						default:
							{
							}break;
						}
					}
				}
			}break;
		case DISCT_MODIFY_SKILLEXTEND:
			{
				SPMOD_SetSkillExtend kData;
				kData.ReadFromPacket(kAddonData);
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkControlUnit);
				if( pkPlayer )
				{
					pkPlayer->GetMySkill()->SetExtendLevel(kData.SkillNo(), kData.ExtendLevel());

					PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
					if(pkActor && pkActor->IsMyActor())
					{
						g_kSkillTree.CreateSkillTree( pkPlayer );//스킬트리를 재구성하자.
						lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
					}
				}
			}break;
		case DISCT_MODIFY_GM_INIT_SKILL:
			{
				SPlayerStrategySkillData kData;
				kAddonData.Pop( kData );

				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkControlUnit);
				if( pkPlayer )
				{
					PgMySkill* pkMySkill = pkPlayer->GetMySkill();
					if( pkMySkill )
					{
						pkMySkill->Init();
						pkMySkill->Add(MAX_DB_SKILL_SIZE, kData.abySkills);
						pkMySkill->InitSkillExtend(MAX_DB_SKILL_EXTEND_SIZE, kData.abySkillExtends);

						PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
						if(pkActor && pkActor->IsMyActor())
						{
							g_kSkillTree.CreateSkillTree( pkPlayer );//스킬트리를 재구성하자.
							lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
						}
					}
				}
			}break;
		case DISCT_MODIFY_SKILL:
		case DISCT_MODIFY_MONEY:
		case DISCT_MODIFY_CP:
			{
			}break;
		case DISCT_MODIFY_SET_DATE_CONTENTS:
			{
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					EDateContentsType eType = DCT_NONE;
					BM::DBTIMESTAMP_EX kDateTime;

					kAddonData.Pop( eType );
					kAddonData.Pop( kDateTime );

					pkPlayer->SetDateContents(eType, kDateTime);
				}
			}break;
		case DISCT_UPDATE_QUEST_CLEAR_COUNT:
		case DISCT_MODIFY_QUEST_ING: //Begin/Drop(none)/Complete(finished)/Fail만
		case DISCT_MODIFY_QUEST_END:
		case DISCT_MODIFY_QUEST_PARAM:
		case DISCT_MODIFY_QUEST_EXT:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if( bMyPilot && pkPlayer && pkActor )
				{
					int iType = 0;
					kAddonData.Pop(iType);

					int const iQuestID = iType;

					bool const bOldRandomQuestBuild = pkPlayer->GetMyQuest()->BuildedRandomQuest();
					bool const bOldTacticsRandomQuestBuild = pkPlayer->GetMyQuest()->BuildedTacticsQuest();
					bool const bOldWantedQuestBuild = pkPlayer->GetMyQuest()->BuildedWantedQuest();
					SUserQuestState const *pkOldState = pkPlayer->GetQuestState(iQuestID);
					SUserQuestState const kOldState = (pkOldState)? *pkOldState: SUserQuestState();
					int const iOldRandomQuestCompleteCount = pkPlayer->GetMyQuest()->TacticsQuestCompleteCount();
					int const iOldRandomTacticsQuestCompleteCount = pkPlayer->GetMyQuest()->TacticsQuestCompleteCount();
					
					pkPlayer->UpdateQuestResult(rkCurModify);
					WantedQuestUI::CallWantedQuestBtn();

					switch( rkCurModify.State() )
					{
					case DISCT_MODIFY_QUEST_ING: //Begin/Drop(none)/Complete(finished)/Fail만
						{
							int iValue = 0;
							kAddonData.Pop(iValue);

							PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
							bNeedQuestUIUpdate = true;
							bNeedUpdateMonsterName = true;
							switch( iValue )
							{
							case QS_Begin: //시작
								{
									// 퀘스트 [퀘스트 제목]을(를) 수락하였습니다.
									BM::vstring vstr( TTW(19995) );
									std::wstring kQuestTitle = TTW( pkQuestInfo->m_iTitleTextNo );
									vstr.Replace( L"#TITLE#", kQuestTitle );

									SChatLog kChatLog(CT_EVENT);
									g_kChatMgrClient.AddLogMessage(kChatLog, vstr, true);

									if( !g_kQuestMan.AddMiniQuestList(iQuestID) )		// Mini 정보창에 추가
									{
										g_kChatMgrClient.AddMessage(450268, SChatLog(CT_EVENT_SYSTEM), true, 1);
									}
									g_kQuestMan.AddRecentBeginQuest(iQuestID);	// 최근에 시작한 퀘스트 정보에 추가
									pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Accept", 0.0f);

									if( pkQuestInfo )
									{
										if( -1 != pkQuestInfo->AcceptEventScriptID() )
										{
											//g_kEventScriptSystem.ActivateEvent(pkQuestInfo->AcceptEventScriptID(), g_pkWorld->GetAccumTime(), g_pkWorld->GetFrameTime());
											g_kQuestMan.BeginEventID( pkQuestInfo->AcceptEventScriptID() );
										}
										switch( pkQuestInfo->Type() )
										{
										case QT_Random:
											{
												RandomQuestUI::CallRandomQuestBtn();
											}break;
										case QT_RandomTactics:
											{
												RandomTacticsQuestUI::CallRandomTacticsQuestBtn();
											}break;
										case QT_Scroll:
											{
												CONT_HAVE_ITEM_POS kContHaveItemPos; //아이템 번호 + (수량, 위치들)
												if( S_OK == pkInv->GetItems( IT_CONSUME, UICT_SCROLL_QUEST, kContHaveItemPos ) )
												{
													CONT_HAVE_ITEM_POS::const_iterator c_iter = kContHaveItemPos.begin();
													while( c_iter != kContHaveItemPos.end() )
													{
														CONT_HAVE_ITEM_POS::mapped_type kItemPos = c_iter->second;
														SItemPos kQuestScrollPos = (*kItemPos.kContItemPos.begin());

														GET_DEF(CItemDefMgr, kItemDefMgr);
														CItemDef const *pDef = kItemDefMgr.GetDef(c_iter->first);
														if( pDef )
														{
															if( iQuestID == pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1) )
															{
																BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kQuestScrollPos);
																kPacket.Push(lwGetServerElapsedTime32());
																NETWORK_SEND(kPacket);
																break;
															}
														}

														++c_iter;
													}
												}
											}break;
										default:
											{
											}break;
										}
									}
								}break;
							case QS_Finished:
								{
									g_kQuestMan.RemoveMiniQuestList(iQuestID);	// Mini 정보창에서 제거
									g_kQuestMan.DelRecentBeginQuest(iQuestID);	// 최근에 시작한 퀘스트 정보에서 제거
									pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete2", 0.0f);

									//if( pkQuestInfo
									////&&	g_pkWorld
									//&&	-1 != pkQuestInfo->CompleteEventScriptID() )
									//{
									//	//g_kEventScriptSystem.ActivateEvent(pkQuestInfo->CompleteEventScriptID(), g_pkWorld->GetAccumTime(), g_pkWorld->GetFrameTime());
									//	g_kQuestMan.CompleteEventID( pkQuestInfo->CompleteEventScriptID() );
									//}

									//CXUI_Wnd *pkQuestDialogWnd = XUIMgr.Get(_T("FRM_QUEST_FULLSCREEN"));
									iCompleteQuestID = iQuestID;
								}break;
							case QS_None:
								{
									g_kQuestMan.DelRecentBeginQuest(iQuestID);
									pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Drop", 0.0f);
									g_kQuestMan.RemoveMiniQuestList(iQuestID);	// Mini 정보창에서 제거
									if( pkQuestInfo )
									{
										switch( pkQuestInfo->Type() )
										{
										case QT_Random:
											{
												RandomQuestUI::CallRandomQuestBtn();
											}break;
										case QT_RandomTactics:
											{
												RandomTacticsQuestUI::CallRandomTacticsQuestBtn();
											}break;
										default:
											{
											}break;
										}
									}
								}break;
							case QS_Failed:
								{
									pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Failed", 0.0f);
								}break;
							case QS_End:
								{
									pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete1", 0.0f);
									if( pkQuestInfo )
									{
										switch( pkQuestInfo->Type() )
										{
										case QT_Random:
											{
												RandomQuestUI::CallRandomQuestBtn();
											}break;
										case QT_RandomTactics:
											{
												RandomTacticsQuestUI::CallRandomTacticsQuestBtn();
											}break;	
										case QT_Wanted:
											{
												WantedQuestUI::CallWantedQuestBtn(true);
											}break;
										default:
											{
											}break;
										}
									}
								}break;
							case QS_Ing:
								{
								}break;
								//case QS_Failed:
							}

							if( g_pkWorld )
							{
								g_pkWorld->ProcessNpcEvent();
							}
						}break;
					case DISCT_MODIFY_QUEST_PARAM:
						{
							bNeedQuestUIUpdate = true;
							bNeedUpdateMonsterName = true;
						}break;
					case DISCT_MODIFY_QUEST_END:
						{
							int iValue = 0;
							kAddonData.Pop(iValue);

							if( 0 == iValue )
							{
								g_kQuestMan.DelFromComleteSet(iQuestID);
							}
							else
							{
								CONT_DEF_QUEST_REWARD const* pkDefQuestReward = NULL;
								g_kTblDataMgr.GetContDef(pkDefQuestReward);
								if( pkDefQuestReward )
								{
									CONT_DEF_QUEST_REWARD::const_iterator find_iter = pkDefQuestReward->find(iQuestID);
									if( pkDefQuestReward->end() != find_iter )
									{
										switch( (*find_iter).second.iDBQuestType )
										{
										case QT_Random:
											{
												if( NULL != XUIMgr.Get(RandomQuestUI::kRandomQuestUIName) )
												{
													RandomQuestUI::CallRandomQuestUI(true); // UI Update
												}
												else
												{
													RandomQuestUI::CallRandomQuestBtn(true);
													if( RandomQuestUI::IsCanRun() )
													{
														RandomQuestUI::CallRandomQuestUI(true); // UI Update
													}
												}
											}break;
										case QT_RandomTactics:
											{
												if( NULL != XUIMgr.Get(RandomQuestUI::kRandomQuestUIName) )
												{
													RandomTacticsQuestUI::CallRandomTacticsQuestUI(true);
												}
												else
												{
													RandomTacticsQuestUI::CallRandomTacticsQuestBtn(true);
													if( RandomTacticsQuestUI::IsCanRun() )
													{
														RandomTacticsQuestUI::CallRandomTacticsQuestUI(true);
													}
												}
											}break;
										default:
											{
											}break;
										}
									}
								}
							}
							g_kQuestMan.ClearDependQuestItem(true);

							if( g_pkWorld )
							{
								g_pkWorld->ProcessNpcEvent();
							}
						}break;
					case DISCT_MODIFY_QUEST_EXT:
						{
							PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
							if( pkMyQuest )
							{
								if( bOldRandomQuestBuild != pkMyQuest->BuildedRandomQuest() )
								{
									if( pkMyQuest->BuildedRandomQuest() )
									{
										// 시작한 경우
										RandomQuestUI::CallRandomQuestBtn(true);
									}
									else
									{
										// 초기화 되었음
										XUIMgr.Close(RandomQuestUI::kRandomQuestBtnName);
									}
								}
								else
								{
									if( IMEPT_QUEST_BUILD_RANDOM_BY_ITEM == kEventCause )
									{
										RandomQuestUI::CallRandomQuestBtn(true); // 아이템에 의해서 제 시작 된 경우
									}
								}

								if( bOldTacticsRandomQuestBuild != pkMyQuest->BuildedTacticsQuest() )
								{
									if( pkMyQuest->BuildedTacticsQuest() )
									{
										// 시작한 경우
										RandomTacticsQuestUI::CallRandomTacticsQuestBtn(true);
									}
									else
									{
										// 초기화 되었음
										XUIMgr.Close(RandomTacticsQuestUI::kRandomTacticsQuestBtnName);
									}
								}
								else
								{
									if( IMEPT_QUEST_BUILD_TACTICS_RANDOM_BY_ITEM == kEventCause )
									{
										RandomTacticsQuestUI::CallRandomTacticsQuestBtn(true); // 아이템에 의해서 제 시작 된 경우
									}
								}

								if( bOldWantedQuestBuild != pkMyQuest->BuildedWantedQuest() )
								{
									if( pkMyQuest->BuildedWantedQuest() )
									{// 시작한 경우
										WantedQuestUI::CallWantedQuestUI();
									}
									else
									{// 초기화 되었음
										XUIMgr.Close(WantedQuestUI::kWantedQuestBtnName);
									}
								}								
							}
						}break;
					default:
						{
						}break;
					}
					//퀘스트 상태가 변경되면 공지로 보여준다.
					SUserQuestState const *pkUpdatedState = pkPlayer->GetQuestState(iQuestID);
					SUserQuestState const kUpdatedState = (pkUpdatedState)? *pkUpdatedState: SUserQuestState();

					g_kQuestMan.ShowQuestInfo(kOldState, kUpdatedState);
				}
			}break;
		case DISCT_MODIFY_CLEAR_ING_QUEST:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if( bMyPilot && pkPlayer && pkActor )
				{
					pkPlayer->ClearIngQuest();
					bNeedQuestUIUpdate = true;
				}
			}break;
		case DISCT_MODIFY_SET_GUID:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				PgMySkill * pkMySkill = ( pkPlayer ? pkPlayer->GetMySkill() : NULL );

				if ( pkMySkill )
				{
					int iType = 0;
					BM::GUID kGuid;

					kAddonData.Pop(iType);
					kAddonData.Pop(kGuid);

					switch( iType )
					{
					case SGT_Guild:
						{
							pkPlayer->GuildGuid( kGuid );

							if( pkMySkill
								&&	BM::GUID::IsNull(kGuid) )
							{
								pkMySkill->EraseSkill(SDT_Guild);
								bNeedReBuildSkillTree = true;
							}
						}break;
					case SGT_Couple:
						{
							pkPlayer->CoupleGuid( kGuid );

							if( pkMySkill
								&&	BM::GUID::IsNull(kGuid) )
							{
								pkMySkill->EraseSkill(SDT_Couple);
								bNeedReBuildSkillTree = true;
							}
						}break;
					case SGT_CoupleColor:
						{
							pkPlayer->ForceSetCoupleColorGuid( kGuid );
						}break;
					}
				}
				
				if(pkActor)
				{
					pkActor->UpdateName();
				}
			}break;
		case DISCT_RENAME_PET:
			{
				PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();
				pkInv->Modify(rkItemWrapper.Pos(), rkItemWrapper);

				PgActor *pkPetActor = g_kPilotMan.FindActor( rkItemWrapper.Guid() );
				if( pkPetActor )
				{
					PgItem_PetInfo *pkPetInfo = NULL;
					if ( true == rkItemWrapper.GetExtInfo( pkPetInfo ) )
					{
						PgPilot *pkPetPilot = pkPetActor->GetPilot();
						if ( pkPetPilot )
						{
							pkPetPilot->SetName( pkPetInfo->Name() );
							pkPetActor->UpdateName();
						}
					}

					if ( pkPetActor->IsUnderMyControl() )
					{// 내 펫이면
						XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"CharInfo");
						if(pkWnd)
						{//UI가 열려 있으면 UI나오는 펫의 이름도 변경해주어야 한다.
							XUI::CXUI_Wnd *pkPetUI = pkWnd->GetControl( L"FRM_PET_INFO" );
							if ( pkPetUI && pkPetUI->Visible() )
							{
								pkPetUI = pkPetUI->GetControl( L"SFRM_NAME_SHADOW" );
								if ( pkPetUI )
								{
									pkPetUI->Text( pkPetInfo->Name() );
								}
							}
						}
					}
				}
			}break;
		case DISCT_SETABIL_PET:
			{
				SPMOD_AddAbilPet::CONT_ABILLIST kAbilList;
				kAddonData.Pop( kAbilList );
					
				PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();
				pkInv->Modify(rkItemWrapper.Pos(), rkItemWrapper);

				if ( PgItem_PetInfo::ms_kPetItemEquipPos == rkItemWrapper.Pos() )
				{
					PgPilot *pkPetPilot = g_kPilotMan.FindPilot( rkItemWrapper.Guid() );
					if ( pkPetPilot )
					{
						bool bIsMyPet = false;
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if(pkPlayer)
						{
							bIsMyPet = rkItemWrapper.Guid()==pkPlayer->SelectedPetID();
						}
						int iSkillNo = 0;
						if(bIsMyPet)
						{
							SPMOD_AddAbilPet::CONT_ABILLIST::const_iterator abil_itr = kAbilList.begin();
							
							for ( ; abil_itr != kAbilList.end() ; ++abil_itr )
							{
								switch ( abil_itr->Type() )
								{
								case AT_EXPERIENCE:
									{
										Recv_PT_M_C_NFY_STATE_CHANGE64( pkPetPilot, SAbilInfo64(abil_itr->Type(), abil_itr->Value()) );
									}break;
								case AT_MON_SKILL_01:
								case AT_MON_SKILL_02:
								case AT_MON_SKILL_03:
								case AT_MON_SKILL_04:
								case AT_MON_SKILL_05:
								case AT_MON_SKILL_06:
								case AT_MON_SKILL_07:
								case AT_MON_SKILL_08:
								case AT_MON_SKILL_09:
								case AT_MON_SKILL_10:
									{
										PgPet * pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
										if ( pkPet )
										{
											PgItem_PetInfo::SStateValue const kStateValue( static_cast<int>(abil_itr->Value()) );
											int const iAddSkillNo = PgPetSkillDataSet::SetSkill( pkPet, kStateValue );
											if ( iAddSkillNo )
											{
												GET_DEF(CSkillDefMgr, kSkillDefMgr);
												CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iAddSkillNo);
												if( pkSkillDef )
												{
													wchar_t const *wszSkillName = NULL;
													if ( true == GetDefString( pkSkillDef->NameNo(), wszSkillName ) )
													{
														int iTTWNo = 187;
														PgItem_PetInfo::SStateValue const kTemp(UINT_MAX);
														if ( kTemp.Time() == kStateValue.Time() )
														{// 무제한
															iTTWNo = 188;
														}
														iSkillNo = iAddSkillNo;
														BM::vstring vstrText(TTW(iTTWNo));
														vstrText.Replace( L"#NAME#", wszSkillName );
														Notice_Show( static_cast<std::wstring>(vstrText), EL_Normal );
													}
												}
											}
										}
									}break;
								default:
									{
										Recv_PT_M_C_NFY_STATE_CHANGE2( pkPetPilot, E_SENDABIL_TARGET_UNIT, SAbilInfo(abil_itr->Type(), static_cast<int>(abil_itr->Value())) );
									}break;
								}
							}
						}

						if( true == bIsMyPet )	//내 펫이고 AT_TIME이 왔고 나머지 어빌이 다 계산되었으면
						{
//							PgPetUIUtil::lwOnChangePetAbil(pkPetPilot, false );
							if(0<iSkillNo)	//흑백처리 된 아이콘 리프래쉬
							{
								XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"CharInfo");
								if(pkWnd)
								{
									PgPetUIUtil::lwUI_RefreshPetSkillUI(pkWnd->GetControl(L"FRM_PET_INFO"));
								}
							}

							PgPetUIUtil::lwDrawTextToPetState();
						}
					}
					else
					{
						BM::Stream kAbilPacket( PT_M_C_NFY_STATE_CHANGE2, rkItemWrapper.Guid() );
						kAbilPacket.Push( static_cast<BYTE>(E_SENDABIL_TARGET_UNIT) );

						BM::Stream kAbil64Packet( PT_M_C_NFY_STATE_CHANGE64, rkItemWrapper.Guid() );

						size_t const iWrPos = kAbilPacket.WrPos();
						size_t const iWrPos64 = kAbil64Packet.WrPos();

						BYTE byNum = 0;
						kAbilPacket.Push( byNum );

						BYTE byNum64 = 0;
						kAbil64Packet.Push( byNum64 );


						SPMOD_AddAbilPet::CONT_ABILLIST::const_iterator abil_itr = kAbilList.begin();
						for ( ; abil_itr != kAbilList.end() ; ++abil_itr )
						{
							switch ( abil_itr->Type() )
							{
							case AT_EXPERIENCE:
								{
									++byNum64;
									kAbil64Packet.Push( SAbilInfo64(abil_itr->Type(), abil_itr->Value()) );
								}break;
							default:
								{
									++byNum;
									kAbilPacket.Push( SAbilInfo(abil_itr->Type(), static_cast<int>(abil_itr->Value())) );
								}break;
							}
						}

						if ( 0 < byNum )
						{
							kAbilPacket.ModifyData( iWrPos, &byNum, sizeof(byNum) );
							g_kPilotMan.BufferReservedPilotPacket( rkItemWrapper.Guid(), &kAbilPacket, 0 );
						}

						if ( 0 < byNum )
						{
							kAbil64Packet.ModifyData( iWrPos64, &byNum64, sizeof(byNum64) );
							g_kPilotMan.BufferReservedPilotPacket( rkItemWrapper.Guid(), &kAbil64Packet, 0 );
						}
					}
				}
			}break;//DISCT_SETABIL_PET
		case DISCT_MODIFY_WORLD_MAP:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					int iMapNo = 0;
					kAddonData.Pop( iMapNo );
					pkPlayer->UpdateWorldMap( iMapNo );

					if ( g_kPilotMan.IsMyPlayer( pkPlayer->GetID()) )
					{
						lua_tinker::call< void, lwGUID >( "CallNpcTRANSPORT", lwGUID(BM::GUID::NullData()) );
						if(g_pkWorld && (g_pkWorld->MapNo() == iMapNo))	//진짜로 방문했을 때 만
						{
							lua_tinker::call< void, bool >( "SetVisitFirstMap", true );
						}
					}
				}
			}break;//DISCT_MODIFY_WORLD_MAP
		case DISCT_TRANSTOWER_SAVE_RECENT:
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
				if ( pkPlayer )
				{
					SRecentInfo kRecentInfo;
					kRecentInfo.ReadFromPacket( kAddonData );

					pkPlayer->TransTowerSaveRct( kRecentInfo );

					Notice_Show( TTW(2707), EL_Normal );
				}
			}break;//DISCT_TRANSTOWER_SAVE_RECENT
		case DISCT_JOBSKILL_SET_SKILL_EXPERTNESS:
		case DISCT_JOBSKILL_DEL_SKILL_EXPERTNESS:
		case DISCT_JOBSKILL_SAVE_EXHAUSTION:
			{
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					//패킷 복사
					if(rkCurModify.State() == DISCT_JOBSKILL_SET_SKILL_EXPERTNESS)
					{
						BM::Stream kReadData(kAddonData);
						SPMOD_JobSkillExpertness kData;
						kData.ReadFromPacket( kReadData );
						NoticeAddExpertnessMsg(*pkPlayer, kData.SkillNo(), kData.Expertness() );
					}

					JobSkillExpertnessUtil::Update(rkCurModify, kAddonData, *pkPlayer);
				}
				if(lwJobSkillView::IsActivateMainWnd())
				{
					lwJobSkillView::UpdateMainWnd();
				}
			}break;
		case DISCT_PREMIUM_SERVICE_INSERT:
			{
				SMemberPremiumData kPremiumData;
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					kAddonData.Pop( kPremiumData.iServiceNo );
					kAddonData.Pop( kPremiumData.kStartDate );
					kAddonData.Pop( kPremiumData.kEndDate );
					kAddonData.Pop( kPremiumData.kCustomData.Data() );
					kPremiumData.kCustomData.PosAdjust();

					pkPlayer->GetPremium().SetService(kPremiumData.iServiceNo, kPremiumData.kCustomData);
					pkPlayer->GetPremium().StartDate(kPremiumData.kStartDate);
					pkPlayer->GetPremium().EndDate(kPremiumData.kEndDate);
				}

				if(bMyPilot)
				{
					g_kPilotMan.MemberPremium(kPremiumData);
					lua_tinker::call<void>("ActivatePremiumUI");
					g_kUISound.PlaySoundByID( L"premium_crown" );

					lua_tinker::call<void,float>("PremiumReminAlram", pkPlayer->GetPremium().GetEndSecTime());
				}
			}break;
		case DISCT_PREMIUM_SERVICE_MODIFY:
			{
				SMemberPremiumData kPremiumData;
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					int iServiceNo = 0;
					BM::DBTIMESTAMP_EX kStartDate;
					BM::DBTIMESTAMP_EX kEndDate;
					kAddonData.Pop( kPremiumData.iServiceNo );
					kAddonData.Pop( kPremiumData.kStartDate );
					kAddonData.Pop( kPremiumData.kEndDate );

					pkPlayer->GetPremium().EndDate(kPremiumData.kEndDate);
				}

				if(bMyPilot)
				{
					g_kPilotMan.MemberPremium(kPremiumData);
					lua_tinker::call<void>("ActivatePremiumUI");

					lua_tinker::call<void,float>("PremiumReminAlram", pkPlayer->GetPremium().GetEndSecTime());
				}
			}break;
		case DISCT_PREMIUM_SERVICE_REMOVE:
			{
				SMemberPremiumData kPremiumData;
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					pkPlayer->GetPremium().Clear();
				}

				if(bMyPilot)
				{
					g_kPilotMan.MemberPremium(kPremiumData);
					lua_tinker::call<void>("ActivatePremiumUI");

					lua_tinker::call<void,float>("PremiumReminAlram", pkPlayer->GetPremium().GetEndSecTime());
				}
			}break;
		case DISCT_PREMIUM_ARTICLE_MODIFY:
			{
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkControlUnit);
				if( pkPlayer )
				{
					int iArticleType = 0;
					kAddonData.Pop(iArticleType);

					switch(iArticleType)
					{
					case PST_QUEST_ONCE_MORE:
						{
							if(S_PST_QuestOnceMore * pkPremium = pkPlayer->GetPremium().SetType<S_PST_QuestOnceMore>())
							{
								EQuestType eType = QT_None;
								BM::PgPackedTime kNextTime;
								kAddonData.Pop(eType);
								kAddonData.Pop(kNextTime);
								pkPremium->AddQuestType(eType, kNextTime);
							}
						}break;
					}
				}
			}break;
		default:
			{
				PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();//Current data
				PgItemWrapper const &rkPrevItemWrapper = rkCurModify.PrevItemWrapper();//Current data

				bool const bInvModifyRet = pkInv->Modify(rkItemWrapper.Pos(), rkItemWrapper);//Apply modify item state

				bool const bFitItem = (IT_FIT == rkItemWrapper.Pos().x || IT_FIT_CASH == rkItemWrapper.Pos().x);
				bool const bNullItem = (0 == rkItemWrapper.ItemNo());
				bool bNeedUpdateName = false;

				if(IT_HOME == rkItemWrapper.Pos().x)
				{
					if( g_pkWorld )
					{
						if(g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 내부
						{
							PgHome* pkHome = g_pkWorld->GetHome();
							if( pkHome )
							{// 가구
								//if( false==rkPrevItemWrapper.IsEmpty() && false==rkItemWrapper.IsEmpty() )// 정보 변경
								//{
								//	pkHome->AddFurniturebyGuid(rkItemWrapper.Guid());
								//}
								if((true == rkItemWrapper.IsEmpty() && false==rkPrevItemWrapper.IsEmpty()) && !(rkCurModify.Cause() & IMET_MODIFY_EXTEND_DATA))// 삭제
								{
									pkHome->RemoveFurniturebyServer(rkPrevItemWrapper.Guid(), rkPrevItemWrapper.ItemNo());
								}
								else if(false==rkItemWrapper.IsEmpty() && true == rkPrevItemWrapper.IsEmpty())// 추가
								{
									pkHome->AddFurniturebyGuid(rkItemWrapper.Guid());
									XUIMgr.Close(L"FRM_MYHOME_EDIT_MODE");
								}
							}
						}
						else if(g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
						{
							PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(kGuid));
							if(pkHouse)
							{
								if( false==rkPrevItemWrapper.IsEmpty() && false==rkItemWrapper.IsEmpty() )// 정보 변경
								{
									pkHouse->AttachFurniture(rkItemWrapper.Guid());
								}
								else if(false==rkPrevItemWrapper.IsEmpty())// 삭제
								{
									pkHouse->RemoveFurniture(rkPrevItemWrapper);
								}
								else if(false==rkItemWrapper.IsEmpty())// 추가
								{
									pkHouse->AttachFurniture(rkItemWrapper.Guid());
								}
							}
						}
					}
				}
				else if( bFitItem )//장착 아이템은 호출
				{
					if(!pkActor) { break; }

					bool bDisable = false;
					PgItem_PetInfo *pkPetInfo = NULL;
					BM::GUID kPlayerGuid;
					g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid);
					bool bCallerIsMe = pkControlUnit->UnitType()==UT_PET ? kPlayerGuid==pkControlUnit->Caller() : false;
					if( !bNullItem )
					{
						if(rkCurModify.Cause() & (IMET_MODIFY_POS | IMET_INSERT_FIXED))
						{
							_PgOutputDebugString("[Recv_PT_M_C_NFY_ITEM_CHANGE] EquipItemByPos SItemPos : %d\n", rkItemWrapper.Pos().y);
							pkActor->EquipItemByPos(static_cast<EInvType>(rkItemWrapper.Pos().x),static_cast<EEquipPos>(rkItemWrapper.Pos().y));
							
							if( pkActor->IsMyActor() || bCallerIsMe )
							{ //\PC_Sound\Commonaction\Com_Item_equipment.wav
								g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, "Com_Item_Equipment", 0.0f);
								if( pkPilot->GetUnit()->UnitType()==UT_PET )
								{ 
									bChangePetItem = true; 
								}
								rkItemWrapper.GetExtInfo(pkPetInfo);//펫 정보를 뽑아보자
							}

							bNeedUpdateName = true;
							ModifyItem(kEventCause, kGuid, MIE_Equip, rkItemWrapper, 1, bMyPilot);//아이템 장비 이벤트
						}
						else if(rkCurModify.Cause() & IMC_INC_DUR_BY_REPAIR)
						{
							ModifyItem(CIE_Repair, kGuid, MIE_Modify, rkItemWrapper, 1, bMyPilot);//아이템 수리
							bItemRepair = true;
						}

						GET_DEF(CItemDefMgr, ItemDefMgr);
						CItemDef const * pItemDef = ItemDefMgr.GetDef(rkItemWrapper.ItemNo());
						if( pItemDef )
						{
							if( 0 < pItemDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE) )		// 추가 인벤 어빌이 있는 아이템이라면.
							{
								lwCallSafeAdditionUI("SFRM_ADDITIONAL_INV", true);
							}
						}
					}
					else
					{
						bool const bQuestRemoveItem = ((rkCurModify.Cause() & IMET_ADD_ANY) && rkCurModify.State() == DISCT_REMOVE);
						if( rkCurModify.Cause() & (IMET_MODIFY_POS | IMET_MODIFY_COUNT)
						||	bQuestRemoveItem )
						{
							_PgOutputDebugString("[Recv_PT_M_C_NFY_ITEM_CHANGE] UnequipItem SItemPos : %d\n", rkItemWrapper.Pos().y);

							if(!pkActor->UnequipItem(static_cast<EInvType>(rkPrevItemWrapper.Pos().x),static_cast<EEquipPos>(rkPrevItemWrapper.Pos().y), rkPrevItemWrapper.ItemNo() )
								&& !pkActor->IsMyActor()
								)
							{
								//pkActor->DetachPOTParticle(static_cast<EInvType>(rkPrevItemWrapper.Pos().x),static_cast<EEquipPos>(rkPrevItemWrapper.Pos().y), rkPrevItemWrapper.ItemNo());

							}
							bNeedUpdateName = true;
							EItemModifyParentEventType kModifyEvent = IMEPT_NONE;

							if(!rkPrevItemWrapper.IsUseTimeOut())
							{
								kModifyEvent = MIE_UnEquip;
							}
							
							if( pkActor->IsMyActor() || bCallerIsMe )
							{ //\PC_Sound\Commonaction\Com_Item_equipment.wav
								g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, "Com_Item_TakeOff", 0.0f);
								if( pkPilot->GetUnit()->UnitType()==UT_PET )
								{ 
									bChangePetItem = true; 
								}
								if(rkPrevItemWrapper.GetExtInfo(pkPetInfo))
								{
									bDisable = true;
								}
							}

							ModifyItem(kEventCause, kGuid, kModifyEvent, rkPrevItemWrapper, 1, bMyPilot);//아이템 해제 이벤트
							if( bQuestRemoveItem )
							{
								ModifyItem(kEventCause, kGuid, MIE_AddItem, rkPrevItemWrapper, -1, bMyPilot);//
							}
						}

						GET_DEF(CItemDefMgr, ItemDefMgr);
						CItemDef const * pItemDef = ItemDefMgr.GetDef(rkPrevItemWrapper.ItemNo());
						if( pItemDef )
						{
							if( 0 < pItemDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE) )		// 추가 인벤 어빌이 있는 아이템이라면.
							{
								lwCallSafeAdditionUI("SFRM_ADDITIONAL_INV", true);
							}
						}
					}
					if( bNeedUpdateName )
					{
						pkActor->UpdateName();
					}
					
					{// 직업도구 장착 및 해제시에 Player 바닥에 문양을 바꿔준다
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const* pkDef = kItemDefMgr.GetDef(rkItemWrapper.ItemNo());
						bool bUpdateColorShadow = false;
						if(!pkDef)
						{// 무기 장비를 해제 했을때
							if(rkPrevItemWrapper.Pos().y == EQUIP_POS_WEAPON
								&& (rkPrevItemWrapper.Pos().x == IT_FIT || rkPrevItemWrapper.Pos().x == IT_FIT_CASH)
							)
							{
								bUpdateColorShadow = true;
							}
						}
						else if( rkItemWrapper.Pos().y == EQUIP_POS_WEAPON
								&& (rkItemWrapper.Pos().x == IT_FIT || rkItemWrapper.Pos().x == IT_FIT_CASH)
							)
						{// 무기 장비를 장착했을때
							bUpdateColorShadow = true;
						}
						if(bUpdateColorShadow)
						{
							PgActorUtil::UpdateColorShadow(pkActor, true);
						}
					}

					bRefreshAbil = true;

					//펫 장착 탈착에 따라 UI 버튼 상태 조작하기
					if(pkPetInfo)
					{
						if(bMyPilot)
						{
							PgPetUIUtil::SetPetUIDefaultState(rkItemWrapper);
						}
						
						PgPetUIUtil::PetSkillToSkillTree(pkPetInfo->ClassKey());
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if ( pkPlayer && pkPlayer->SelectedPetID() == rkItemWrapper.Guid() )
						{
							PgPetUIUtil::LoadMyPetInventory(rkItemWrapper.Guid(), pkPetInfo->Name());
							{
								XUI::CXUI_Wnd *pkInfo = XUIMgr.Get(L"CharInfo");
								if(pkInfo)
								{
									lwCharInfo::SetDiePetInfoToUI(pkInfo->GetControl(L"FRM_PET_INFO"), rkItemWrapper.Guid());
								}
							}

							lwCallSafeAdditionUI("SFRM_ADDITIONAL_INV", true);
// 							if ( true == pkPetInfo->IsDead() )
// 							{
// 								XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"CharInfo");//UI가 열려 있으면 버튼 상태를 바꿔 줘야 함
// 								if(pkWnd)
// 								{
// 									g_kMyActorViewMgr.DeleteActor("PetActor");
// 									if( false == lwCharInfo::SetDiePetInfoToUI(pkWnd->GetControl(L"FRM_PET_INFO"), rkItemWrapper.Guid()) )
// 									{
// 										PgPetUIUtil::SetPetUIDefaultState(true);
// 									}
// 								}
// 							}
						}

						XUI::CXUI_Wnd *pkQuick = XUIMgr.Get(L"QuickInv");	//새로 그려주자
						if( pkQuick )
						{
							pkQuick->SetInvalidate();
						}
					}

					//	FRM_REPAIR_ALARM 여따가 느믄 되겠네.
					//	FRM_FIGHTER -> 알려주는것도 일이네.
					//	FRM_MAGICIAN
					//	FRM_ARCHER
					//	FRM_THIFE
				}
				else// if( 1 == kArray.size() )//기타 아이템 변화
				{
					if(!pkActor) { break; }

					int iCount = rkItemWrapper.Count() - rkPrevItemWrapper.Count();//수량 변화 체크
					int const iItemNo = (0 != rkItemWrapper.ItemNo())? rkItemWrapper.ItemNo(): rkPrevItemWrapper.ItemNo();
					PgBase_Item const& rkLogItem = ((0 != rkItemWrapper.ItemNo())? rkItemWrapper: rkPrevItemWrapper);
					
					if(	!rkPrevItemWrapper.IsEmpty() )
					{//빼기전 아이템이 있고.
						if( (IT_FIT == rkPrevItemWrapper.Pos().x || IT_FIT_CASH == rkPrevItemWrapper.Pos().x) 
							&& rkItemWrapper.Count() == 0
							)
						{
							pkActor->UnequipItem(static_cast<EInvType>(rkPrevItemWrapper.Pos().x),static_cast<EEquipPos>(rkPrevItemWrapper.Pos().y), rkPrevItemWrapper.ItemNo());
						}
					}
					else
					{
						//pkActor->DetachPOTParticle(static_cast<EInvType>(rkPrevItemWrapper.Pos().x),static_cast<EEquipPos>(rkPrevItemWrapper.Pos().y), rkPrevItemWrapper.ItemNo());
					}

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
					if( NULL == pDef )
					{
						break;
					}

					if( pDef->CanEquip() )
					{
						if( 0 > iCount )
						{
							iCount = -1;
						}
						else if( 0 == iCount )
						{
							// iCount = 0;
						}
						else
						{
							iCount = 1;
						}
					}

// 					if( SAFE_FOAM_EFFECTNO == kItemDefMgr.GetAbil(iItemNo, AT_EFFECTNUM1) )
// 					{
// 						bSafeFoamUseClear = true;
// 					}

					__int64 const iCause = rkCurModify.Cause();
					__int64 const iState = rkCurModify.State();


					if( rkLogItem.EnchantInfo().IsTimeLimit() &&
						(true == bMyPilot) && 
						(UICT_REST == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) || UICT_REST_EXP == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)) && 
						(IMET_MODIFY_ENCHANT & iCause))
					{
						pkInv->AddCoolTime(iItemNo,PgInventory::EICool_UseItem,lwGetServerElapsedTime32(),pkControlUnit->GetAbil(AT_R_COOLTIME_RATE_ITEM));
					}


					if( iCause & IMC_DEC_DUR_BY_USE )//아이템 사용
					{
						ModifyItem(kEventCause, kGuid, MIE_UseItem, rkLogItem, iCount, bMyPilot);
						
						bModifyPos = true;	//아이템 사용했을때도 새로 그려줘야 함

						if( true == bMyPilot )
						{
							switch(pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
							{
							case UICT_CALL_MARKET:
								{
									lwMarket::lwCallOpenMarket();
								}break;
							case UICT_CALL_SAFE:
								{
									PgSafeInventory::CallInvSafeUI(BM::GUID());
								}break;
							case UICT_CALL_SHARE_SAFE:
								{
									PgSafeInventory::CallInvShareSafeUI(BM::GUID());
								}break;
							case UICT_COOLTIME_RATE_SKILL:
								{
									int const iCustomValue1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
									DWORD const dwCurTime = g_kEventView.GetServerElapsedTime();
									pkActor->ReCalcCoolTime(iCustomValue1);
								}break;
							case UICT_COOLTIME_RATE_ITEM:
								{
									int const iCustomValue1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
									DWORD const dwCurTime = g_kEventView.GetServerElapsedTime();
									pkInv->ReCalcCoolTime(dwCurTime,iCustomValue1);
								}break;
							case UICT_ADD_COOLTIME_SKILL:
								{
									int const iCustomValue1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
									DWORD const dwCurTime = g_kEventView.GetServerElapsedTime();
									pkActor->ReCalcCoolTime(iCustomValue1);
								}break;
							case UICT_ADD_COOLTIME_ITEM:
								{
									int const iCustomValue1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
									DWORD const dwCurTime = g_kEventView.GetServerElapsedTime();
									pkInv->ReCalcCoolTime(dwCurTime,iCustomValue1);
								}break;
							case UICT_STATUS_RESET:
								{// Active Status 아이템 사용으로 인한 리셋일때 UI 갱신
									g_kUIActiveStatus.InitUIData();
									g_kUIActiveStatus.UpdateUI();
								}break;
							default:
								{
								}break;
							}
						}
					}
					else if( iCause & IMET_ADD_MONEY )//돈 바뀌었어 ;; 안써(PT_M_C_NFY_CHANGE_MONEY 에서 처리)
					{
					}
					else if( iCause & IMC_DEC_DUR_BY_SHOP_SELL )//상점에 팔어
					{
					}
					//else if( iCause & IMC_POS_BY_EXCHANGE )// 다른 사용자랑 교환
					//{
					//}
					else if ( iCause & IMET_MODIFY_POS )
					{
						bModifyPos = true;
						
						CXUI_Wnd* pInvSafeWnd = XUIMgr.Get( _T( "SFRM_INV_SAFE" ) );
						CXUI_Wnd* pShareInvSafeWnd = XUIMgr.Get( _T( "SFRM_SHARE_SAFE" ) );
						CXUI_Wnd* pInv = XUIMgr.Get( _T( "Inv" ) );

						if( bMyPilot
						&&	pInv )
						{
							XUI::CXUI_Wnd* pkTitle = pInv->GetControl( std::wstring( _T( "SFRM_TITLE" ) ) );
							if( pkTitle )
							{
								XUI::CXUI_CheckButton* pkCheckBtn = NULL;								

								switch( rkItemWrapper.Pos().x )
								{
								case IT_EQUIP:
									{
										pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_EQUIP_BG"))->GetControl(_T("CBTN_EQUIP")));
									}break;
								case IT_CONSUME:
									{
										pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_CONSUM_BG"))->GetControl(_T("CBTN_CONSUM")));
									}break;
								case IT_ETC:
									{
										pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_ETC_BG"))->GetControl(_T("CBTN_ETC")));
									}break;
								case IT_CASH:
									{
										pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_CASH_BG"))->GetControl(_T("CBTN_CASH")));
									}break;
								default:
									{
									}break;
								}
								lua_tinker::call<void, int, lwUIWnd>("PreChangeInvViewGroup", static_cast<int>(rkItemWrapper.Pos().x), lwUIWnd(pkCheckBtn));
							}
						}

						if( pInvSafeWnd )
						{
							int index = INT_MAX;
							switch( rkItemWrapper.Pos().x )
							{					
							case IT_SAFE:			{ index = 0; }break;				
							case IT_RENTAL_SAFE1:		
							case IT_RENTAL_SAFE2:		
							case IT_RENTAL_SAFE3:		
								{
									index = rkItemWrapper.Pos().x - IT_RENTAL_SAFE1 + 1;
								}break;
							default:
								{
								}break;
							}

							if(index != INT_MAX)
							{
								BM::vstring vStr(L"CBTN_BAG");
								vStr += index;
								XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pInvSafeWnd->GetControl(vStr));
								if(pBtn)
								{
									pBtn->DoScript(SCRIPT_ON_GET_FOCUS);
								}
							}
						}

						if( pShareInvSafeWnd )
						{
							switch( rkItemWrapper.Pos().x )
							{					
							case IT_SHARE_RENTAL_SAFE1: 
							case IT_SHARE_RENTAL_SAFE2: 
							case IT_SHARE_RENTAL_SAFE3: 
							case IT_SHARE_RENTAL_SAFE4: 
								{
									BM::vstring vStr(L"CBTN_BAG");
									vStr += rkItemWrapper.Pos().x - IT_SHARE_RENTAL_SAFE1 + 4;
									XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pShareInvSafeWnd->GetControl(vStr));
									if(pBtn)
									{
										pBtn->DoScript(SCRIPT_ON_GET_FOCUS);
									}
									
								}break;
							default:
								{
								}break;
							}
						}

					}
					else if( iCause & IMET_MODIFY_COUNT )
					{
						std::wstring kName;
						BM::vstring kWarn;
						bool const bName = MakeItemName(rkItemWrapper.ItemNo(), rkItemWrapper.EnchantInfo(), kName );
						if(DISCT_REMOVE == iState)
						{
							bRefreshAbil = bMyPilot;
							
							ModifyItem(kEventCause, kGuid, MIE_Destroy, rkLogItem, iCount, bMyPilot);
							if (!rkItemWrapper.IsEmpty())
							{
								if( bMyPilot && IT_FIT == rkPrevItemWrapper.Pos().x && MINIMUM_ENDURANCE_VALUE>=rkItemWrapper.Count() )
								{
									kWarn = TTW(1280);
									if (bName)
									{
										kWarn+=kName;
										::Notice_Show((std::wstring const&)(kWarn), 1);
									}
								}
							}

							if(bMyPilot)
							{
								PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkControlUnit);
								if ( pkPlayer )
								{
									PgQuickInventory *pkQInven = pkPlayer->GetQInven();
									if( pkQInven && pkQInven->IsExist(iItemNo) == S_OK)
									{
										bModifyPos = true;
									}
								}
							}
						}
						else
						{
							if( iCause & IMC_INC_DUR_BY_REPAIR )
							{
								ModifyItem(CIE_Repair, kGuid, MIE_Modify, rkLogItem, iCount, bMyPilot);
								bItemRepair = true;
							}
							else
							{
								ModifyItem(kEventCause, kGuid, MIE_UseItem, rkLogItem, iCount, bMyPilot);
							}

							if( IT_FIT == rkPrevItemWrapper.Pos().x  )
							{
								bRefreshAbil = bMyPilot;

								if ( bMyPilot && bName && MINIMUM_ENDURANCE_VALUE>=rkItemWrapper.Count())
								{
									kWarn+=kName;
									kWarn += TTW(1281);
									::Notice_Show((std::wstring const&)(kWarn), 1);
								}
							}
						}

					}
					else if( 0 != (iCause & (IMET_INSERT_FIXED|IMET_ADD_ANY|IMET_MODIFY_SYS2INV|IMC_POS_BY_EXCHANGE)) )
					{
						if( bMyPilot )
						{
							if( 0 > iCount )
							{ // 소모시 아이템 로그 합침
								SystemMessageUtil::PgSystemLogKey kFindKey(iItemNo, BM::GUID::NullData(), kGuid);
								SystemMessageUtil::ContLogItem::iterator find_iter = kDelayItemUseLog.find(kFindKey);
								if( kDelayItemUseLog.end() == find_iter )
								{
									kDelayItemUseLog.insert( std::make_pair(kFindKey, SystemMessageUtil::PgSystemLog_Item(rkLogItem, iCount, kGuid)) );
								}
								else
								{
									SystemMessageUtil::PgSystemLog_Item &rkLogItem = (*find_iter).second;
									rkLogItem.TotalCount( rkLogItem.TotalCount() + iCount );
								}
							}
							else
							{ // 획득시에 아이템 로그 분리
								ModifyItem(kEventCause, kGuid, MIE_AddItem, rkLogItem, iCount, bMyPilot);
								
								if( CIE_MoveFromGuildInvToInv == kEventCause ) // 길드금고에서 아이템을 찾을 때 자동으로 해당 탭 전환
								{
									CXUI_Wnd* pInv = XUIMgr.Get( _T( "Inv" ) );
									if( pInv )
									{
										XUI::CXUI_Wnd* pkTitle = pInv->GetControl( std::wstring( _T( "SFRM_TITLE" ) ) );
										if( pkTitle )
										{
											XUI::CXUI_CheckButton* pkCheckBtn = NULL;

											switch( rkItemWrapper.Pos().x )
											{
											case IT_EQUIP:
												{
													pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_EQUIP_BG"))->GetControl(_T("CBTN_EQUIP")));
												}break;
											case IT_CONSUME:
												{
													pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_CONSUM_BG"))->GetControl(_T("CBTN_CONSUM")));
												}break;
											case IT_ETC:
												{
													pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_ETC_BG"))->GetControl(_T("CBTN_ETC")));
												}break;
											case IT_CASH:
												{
													pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(_T("SFRM_CASH_BG"))->GetControl(_T("CBTN_CASH")));
												}break;
											default:
												{
												}break;
											}

											lua_tinker::call<void, int, lwUIWnd>("PreChangeInvViewGroup", static_cast<int>(rkItemWrapper.Pos().x), lwUIWnd(pkCheckBtn));
										}
									}
								}
							}
						}
					}
					else if(0 != (iCause & IMET_MODIFY_DB2INV))
					{
						switch( kEventCause )
						{
						case CIE_Post_Mail_Item_Recv: // 우편 예외 처리. 마이너스 값이 먼저 오는 기본 루틴 처리 내에서 해결 방법이 없음
						case CIE_UM_Article_Buy:
							{
								if( 0<iCount && bMyPilot && DISCT_REMOVE!=rkCurModify.State())	//채팅창에 로그 두번 남는 현상 막음
								{
									ModifyItem(kEventCause, kGuid, MIE_AddItem, rkLogItem, iCount, bMyPilot);
								}
							}break;
						default:
							{
							}
						}
					}
					else  if(IMEPT_MISSIONBONUS == kEventCause || MCE_EXP == kEventCause)
					{
						if( pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
						{
							SExpCard kNew, kOld;
							if( rkItemWrapper.Get(kNew) &&rkPrevItemWrapper.Get(kOld) )
							{
								__int64 const iGab = kNew.CurExp() - kOld.CurExp();
								if( 0 != iGab )
								{
									wchar_t szTemp[MAX_PATH] = {0,};
									swprintf_s(szTemp, MAX_PATH, TTW(50767).c_str(), iGab);
									SChatLog kChatLog(CT_EXP);
									g_kChatMgrClient.AddLogMessage(kChatLog, szTemp);
								}
							}
						}
						ModifyItem(kEventCause, kGuid, IMEPT_NONE, rkLogItem, iCount, bMyPilot);
					}


					if( true == bMyPilot )
					{
						switch(pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
						{
						case UICT_SKILL_EXTEND:
							{
								lwItemMix::SetMonsterCardMixerType2MixItem();
							}break;
						}
					}
				}

				if(rkCurModify.Cause() & IMET_MODIFY_ENCHANT)
				{
					if(UIT_STATE_PET == rkItemWrapper.State())
					{
						PgPilot *pkPilot = g_kPilotMan.FindPilot(rkItemWrapper.Guid());
						if(pkActor->IsMyPet())
						{
							g_kPetUIMgr.CallResultUI( rkItemWrapper, rkPrevItemWrapper,  pkInv);
						}
						if(pkPilot)
						{
							PgPet* pkPet = dynamic_cast<PgPet*>(pkPilot->GetUnit());
							if(pkPet)
							{
								pkPet->SetPeriodEndSecTime(rkItemWrapper.GetDelTime());
							}
						}
					}	
				}
				
				if( CIE_Inventory_Sort == kEventCause ||
					CIE_Mouse_Event == kEventCause ||
					CIE_Equip == kEventCause)
				{//아이템정렬,마우스이벤트,장비장착으로 인한 인벤토리 변화는 표기하지 않도록
					lwInventory::lwApplyCurrentInv( static_cast<EInvType>(rkItemWrapper.Pos().x) );
				}
				lwInventory::lwCallNewInvItem( static_cast<EInvType>(rkItemWrapper.Pos().x) );
			}break;
		}

		switch(rkCurModify.State())
		{
		case DISCT_CREATE:
		case DISCT_MODIFY:
		case DISCT_REMOVE:
			{
				bCallNpcEvent = true;
			}break;
		}
		++item_itor;
	}

	if( bCallNpcEvent && g_pkWorld )
	{
		g_pkWorld->ProcessNpcEvent();
	}

	SystemMessageUtil::ContLogItem::const_iterator delaylog_iter = kDelayItemUseLog.begin();
	while( kDelayItemUseLog.end() != delaylog_iter )
	{
		SystemMessageUtil::PgSystemLog_Item const& rkLogItem = (*delaylog_iter).second;
		ModifyItem(kEventCause, rkLogItem.OwnerGuid(), MIE_AddItem, rkLogItem, rkLogItem.TotalCount(), bMyPilot);
		++delaylog_iter;
	}
	
	if( bRefreshAbil )//Abil Refresh
	{
		pkControlUnit->NftChangedAbil(AT_REFRESH_ABIL_INV, E_SENDTYPE_NONE);
		RefreshCharStateUI();
		lwRefreshRepairAlarmUI();

		if(bMyPilot)
		{
			bNeedReBuildSkillTree = true;
			CONT_SKILL_OPT const kSkillList = pkControlUnit->GetMySkill()->GetCalcOverSkillList(PgMySkill::SOLT_ONLY_ONE_SKILL);

			g_kSkillTree.ClearSkillTreeOverSkillLevel();
			for(CONT_SKILL_OPT::const_iterator itor = kSkillList.begin(); itor != kSkillList.end(); ++itor)
			{
				PgSkillTree::stTreeNode* pFound = g_kSkillTree.GetNode((*itor).first);
				if(!pFound)
				{
					continue;
				}

				pFound->SetOverSkillLevel((*itor).second);
			}
		}
	}
	if( bNeedQuestUIUpdate )
	{
		g_kQuestMan.User_Quest_State_Changed();
	}
	if( bNeedUpdateMonsterName 
		&& g_pkWorld
		)
	{
		g_pkWorld->AllMonsterUpdateName();
	}
	if( bNeedReBuildSkillTree )
	{
		g_kSkillTree.CreateSkillTree( dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit()) );
		lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
	}
// 	if( bSafeFoamUseClear )
// 	{
// 		g_kSafeFoamMgr.Clear();
// 		g_kSafeFoamMgr.SetSafeFoamUsed();
// 		g_kSafeFoamMgr.CheckSafeFoam();
// 	}
	if( 0 < iCompleteQuestID
	&&	PgQuestInfoUtil::IsCompleteQuestOrder(kEventCause) )
	{
		if( g_kQuestMan.IsQuestDialog() )
		{
			g_kQuestMan.IsComplete(iCompleteQuestID);
		}
		else
		{
			g_kQuestMan.CallCompleteStamp(iCompleteQuestID);
		}
	}
	if (bModifyPos || bRefreshAbil)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_POST");
		if (pkWnd)
		{
			XUI::CXUI_Wnd* pkWndPost = pkWnd->GetControl(L"FRM_POST2");
			if (pkWndPost)
			{
				XUI::CXUI_List* pkListpWndPost = dynamic_cast<XUI::CXUI_List*>(pkWndPost->GetControl(L"LST_INV"));
				if (pkListpWndPost)
				{
					SListItem* pkItem = pkListpWndPost->GetItemAt(0);
					if (pkItem && pkItem->m_pWnd)
					{
						for (int i = 0; i < 4; ++i)
						{
							BM::vstring kString(L"FRM_INVEN");
							kString+=i;
							XUI::CXUI_Wnd* pkChild = pkItem->m_pWnd->GetControl((std::wstring const &)kString);
							if (pkChild)
							{
								lwCanAddToMail(lwUIWnd(pkChild));
							}
						}
						
					}
				}
			}
		}

		lwUIWnd kQuick = lwGetUIWnd("QuickInv");	//새로 그려주자
		if( !kQuick.IsNil() )
		{
			kQuick.SetInvalidate();
		}
	}

	lwUIWnd kInv = lwGetUIWnd("Inv");
	if (!kInv.IsNil())
	{
		kInv.SetInvalidate();
	}

	lwUIWnd kSafeInv = lwGetUIWnd("SFRM_SHARE_SAFE");
	if (!kSafeInv.IsNil())
	{
		int const iInvType = kSafeInv.GetCustomData<int>() - IT_SHARE_RENTAL_SAFE1 + 4;
		lwChangeCashBoxViewGroup(kSafeInv.GetSelf(), iInvType);
	}

	if( bMyPilot )
	{
		lwUIWnd kEquip = lwGetUIWnd("CharInfo");
		if (!kEquip.IsNil())
		{
			kEquip = kEquip.GetControl("FRM_CHAR_INFO");
			if( !kEquip.IsNil() )
			{
				lwCharInfo::UpdateMyActor();
				lwCharInfo::lwViewEquipIconInitialize(kEquip);
				lwCharInfo::lwSetCharAbilInfo(kEquip, 0, "L", false);
				lwCharInfo::lwSetCharAbilInfo(kEquip, 0, "R", false);
				kEquip.SetInvalidate();
			}
		}
	}

	if( bChangePetItem )
	{
		lwViewPetEquip();
		if(pkActor->GetCallerIsMe())
		{
			XUI::CXUI_Wnd* pkInfo = XUIMgr.Get(L"CharInfo");
			if(pkInfo)
			{
				XUI::CXUI_Wnd* pkPetInfo = pkInfo->GetControl(L"FRM_PET_INFO");
				if(pkPetInfo)
				{
					lwCharInfo::lwSetPetInfoToUI(pkPetInfo, lwGUID(pkPilot->GetGuid()));
				}
			}
		}
	}

	if( true == bMyPilot && ( bItemRepair || kEventCause == CIE_Repair ) )
	{
		g_kUISound.PlaySoundByID( L"Item_Repair" );
		pkActor->AddNewParticle("ef_repair_item", 8386, "char_root", pkActor->GetEffectScale());
		lua_tinker:: call<void, int, bool>("CommonMsgBoxByTextTable", 1227, true);
	}

	bool bRefreshUI = false;
	switch( kEventCause )
	{
	case CIE_Make:
		{
			CXUI_Wnd* pkTopWnd = NULL;

			pkTopWnd = XUIMgr.Get( std::wstring(_T("FRM_TRADE_UNSEALINGSCROLL")) );
			if( pkTopWnd )
			{
				int iCount = 1;	//기본 수량은 1
				CXUI_Wnd* pkOnBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_OK")) );
				if( pkOnBtn )
				{
					pkOnBtn->GetCustomData(&iCount, sizeof(iCount));	//UI에서 설정된 지정 수량 받아오기
				}
				if( 0 >= iCount)
				{
					iCount = 1;
				}
				SoulCraft::UpdateTradeUnselingScroll(pkTopWnd, iCount);
			}
			pkTopWnd = XUIMgr.Get( SoulStoneTrade::kSoulStoneTradeWndName );
			if( pkTopWnd )
			{
				SoulStoneTrade::UpdateSoulStoneTrade();
			}
			pkTopWnd = XUIMgr.Get(L"SFRM_ITEM_MIX_CREATOR");
			if( pkTopWnd )
			{
				S_MIX_ITEM_INFO kRecipe = g_kEqItemMixCreator.GetRecipe();
				S_MIX_ITEM_INFO kInsu = g_kEqItemMixCreator.GetInsurance();
				S_MIX_ITEM_INFO kProb = g_kEqItemMixCreator.GetProbAbility();
				g_kEqItemMixCreator.Clear();
				g_kEqItemMixCreator.SetRecipe(kRecipe.kItemNo);
				if( !kInsu.IsEmpty() )
				{
					g_kEqItemMixCreator.SetInsurance();
				}
				if( !kProb.IsEmpty() )
				{
					g_kEqItemMixCreator.SetProbAbility();
				}
				lwItemMix::CallEqItemMixCreatorUI();
			}
		}break;
	case CIE_EnchantLvUp:
	case CIE_SoulCraft:
	case CIE_BasicOptionAmp:
		{
			XUI::CXUI_Wnd* pTemp = XUIMgr.Get(L"SFRM_ITEM_PLUS_UPGRADE");
			if( pTemp )
			{
				lwUIItemPlusUpgrade	PlusUpgrade(pTemp);
				PlusUpgrade.ReSetUpgradeData();
				PlusUpgrade.ResultProcess();
			}
			else
			{
				pTemp = XUIMgr.Get(L"SFRM_ITEM_RARITY_UPGRADE");
				if( pTemp )
				{
					lwUIItemRarityUpgrade	RarityUpgrade(pTemp);
					RarityUpgrade.ReSetUpgradeData();
					RarityUpgrade.ResultProcess();
				}
			}
		}break;
	case IMEPT_EVENT_TW_EFFECTQUEST:
		{
			EventTaiwanEffectQuest::UpdateUI();
		}break;
	case CIE_Convert_Item:
		{
			/*CXUI_Wnd* pkTopWnd = NULL;

			pkTopWnd = XUIMgr.Get( std::wstring(_T("FRM_ITEM_CONVERT")) );
			if( pkTopWnd )
			{
				//SoulCraft::UpdateTradeUnselingScroll(pkTopWnd, SoulCraft::GetCanMaxTradeMaking(SoulCraft::iSoulUnsealingMakingNo));
			}
			pkTopWnd = XUIMgr.Get( Item_Convert::kItem_ConvertWndName );
			if( pkTopWnd )
			{
				Item_Convert::UpdateItem_Convert();
			}*/
		}break;
	case CIE_JOBSKILL_DELETE:
		{
			if( lwJobSkillView::IsActivateMainWnd() )
			{
				lwJobSkillView::UpdateMainWnd();
			}
		}break;
	case CIE_Equip:
	case CIE_UnEquip:
	case CIE_Repair:
	case CAE_Inv2Achievement:
	case CAE_Achievement2Inv:
		{
			lwItemSkillUI::UpdateList();
		}break;
	case CIE_Divide:
		{
			bRefreshUI = true;
			BM::GUID kSourceGuid;
			BM::GUID kDivideGuid;
			DB_ITEM_STATE_CHANGE_ARRAY::const_iterator item_itor = kArray.begin();
			while(kArray.end() != item_itor)
			{
				DB_ITEM_STATE_CHANGE_ARRAY::value_type const &rkCurModify = (*item_itor);
				__int64 const iState = rkCurModify.State();

				PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();//Current data
				if(iState == DISCT_MODIFY)
				{
					kSourceGuid = rkItemWrapper.Guid();
				}
				else if(iState == DISCT_CREATE)
				{
					kDivideGuid = rkItemWrapper.Guid();
				}

				++item_itor;
			}

			if( pkActor )
			{
				pkActor->DoDivideReservedTransit(kSourceGuid, kDivideGuid);
			}
		}break;
	case CIE_Dump:
		{
			bRefreshUI = true;
		}break;
	case CIE_GemStore_Buy:
		{
			g_kGemStore.UpdateItemTree();
		}break;
	default:
		{
		}break;
	}

	if(bRefreshUI)
	{
		lwJobSkillItem::JS3_RefreshResItem();
	}
}

void Recv_PT_M_C_NFY_WARN_MESSAGE(BM::Stream *pkPacket)
{
	int iSendMessage;
	BYTE byLevel = EL_Warning;
	pkPacket->Pop(iSendMessage);
	pkPacket->Pop(byLevel);
	
	switch(iSendMessage)
	{
	case 20028:	 // "쿨타임이 아직 남아있습니다"
		{
			g_kChatMgrClient.ShowNoticeUI(TTW(iSendMessage), static_cast<int>(byLevel));
		}break;
	case 700037:
		{// 인벤이 가득찼다는 메세지는
			if(PgCashShop::ECASHSHOP_GACHA == g_kCashShopMgr.GetShopType())
			{// 캐시샵 가차 상태에서는 출력 할 수 없다.
				return;
			}
		}//break; //의도적 생략
	default:
		{
			Notice_Show( TTW(iSendMessage), static_cast<int>(byLevel) );
		}break;
	}	

	if(  25001 == iSendMessage
		|| 25006 == iSendMessage
		|| 25012 == iSendMessage
		|| 25013 == iSendMessage
		|| 25014 == iSendMessage
		|| 25017 == iSendMessage
		|| 25018 == iSendMessage)
	{// 직업 스킬 관련 메세지일 경우 사운드 출력 하도록 추가
		lwPlaySoundByID( "05.messager" );
	}

	if (FULL_INVEN_MESSAGE_NUM==iSendMessage)
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Full_Inven", 0.0f,0.0f,0.0f,g_kPilotMan.GetPlayerPilot()->GetWorldObject());
	}
}

void Recv_PT_M_C_NFY_WARN_MESSAGE2(BM::Stream *pkPacket)
{
	int iSendMessage = 0, iValue = 0;
	BYTE byLevel = EL_Warning;
	pkPacket->Pop(iSendMessage);
	pkPacket->Pop(iValue);
	pkPacket->Pop(byLevel);

	std::wstring wstrNotice;

	if(CHECK_QUEST_MESSAGE_MIN <= iSendMessage && CHECK_QUEST_MESSAGE_MAX >= iSendMessage)
	{
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iValue);
		if(pkQuestInfo)
		{
			WstringFormat( wstrNotice, MAX_PATH, TTW(iSendMessage).c_str(), TTW(pkQuestInfo->m_iTitleTextNo).c_str() );
		}
	}
	else
	{
		WstringFormat( wstrNotice, MAX_PATH, TTW(iSendMessage).c_str(), iValue );
	}

	Notice_Show( wstrNotice, static_cast<int>(byLevel) );

	if ( FULL_INVEN_MESSAGE_NUM==iSendMessage )
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Full_Inven", 0.0f,0.0f,0.0f,g_kPilotMan.GetPlayerPilot()->GetWorldObject());
	}
}

void Recv_PT_M_C_NFY_WARN_MESSAGE3(BM::Stream *pkPacket)
{
	int iSendMessage = 0;
	BM::Stream kValue;
	BYTE byLevel = EL_Warning;
	pkPacket->Pop(iSendMessage);
	pkPacket->Pop(kValue.Data());				kValue.PosAdjust();
	pkPacket->Pop(byLevel);

	BM::vstring vNotice( TTW(iSendMessage) );
	switch(iSendMessage)
	{
	case 8020:
		{//나간유저에 대한 다른 그룹원에게 메시지전송
			std::wstring kName;
			kValue.Pop(kName);

			vNotice.Replace(L"#NAME#", kName);
		}break;
	case 18198:
		{
			int iTime = 0;
			DWORD iHour = 0;
			DWORD iMin = 0;
			kValue.Pop(iTime);
			kValue.Pop(iHour);
			kValue.Pop(iMin);

			vNotice.Replace(L"#TIME#", iTime);
			vNotice.Replace(L"#HOUR#", iHour);
			vNotice.Replace(L"#MIN#", iMin);
		}break;
	default:
		{
			if(CHECK_EFFECT_MESSAGE_MIN <= iSendMessage && CHECK_EFFECT_MESSAGE_MAX >= iSendMessage)
			{
				VEC_INT kCont;
				int iToken = 0;
				kValue.Pop(kCont);
				kValue.Pop(iToken);

				BM::vstring vEffectName;
				wchar_t const* pkEffectNameStr = NULL;
				for(VEC_INT::const_iterator effno_it = kCont.begin(); effno_it != kCont.end(); ++effno_it)
				{
					if(0 != vEffectName.size())
					{//다음항목 구분자
						vEffectName += iToken ? TTW(iToken) : L"/";
					}
					if( GetEffectName(*effno_it, pkEffectNameStr) )
					{
						vEffectName += pkEffectNameStr;
#ifndef EXTERNAL_RELEASE
						vEffectName += L"(";
						vEffectName += BM::vstring(*effno_it);
						vEffectName += L")";
#endif
					}
				}
				vNotice.Replace(L"#NAME#", vEffectName);
			}
		}break;
	}

	Notice_Show( vNotice, static_cast<int>(byLevel) );
}

void Recv_PT_M_C_NFY_WARN_MESSAGE_STR(BM::Stream *pkPacket)
{
#ifndef EXTERNAL_RELEASE
	int iSendMessage = 0, iValue = 0;
	BYTE byLevel = EL_Warning;
	std::wstring wstrNotice;
	pkPacket->Pop(wstrNotice);
	pkPacket->Pop(byLevel);
	
	Notice_Show( wstrNotice, static_cast<int>(byLevel) );
#endif
}

void Recv_PT_M_C_ANS_PICKUPGBOX(BM::Stream *pkPacket)
{
	BM::GUID kOwnerGuid;
	BM::GUID kGroundBoxGuid;
	pkPacket->Pop(kOwnerGuid);
	pkPacket->Pop(kGroundBoxGuid);

	PgPilot *pkBoxPilot = g_kPilotMan.FindPilot(kGroundBoxGuid);
	if( pkBoxPilot )
	{
		PgDropBox *pkBox = dynamic_cast<PgDropBox*>(pkBoxPilot->GetWorldObject());
		if( pkBox )
		{
			bool bRemoveBox = false;
			PgPilot *pkPilot = g_kPilotMan.FindPilot(kOwnerGuid);
			if ( pkPilot )
			{
				PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
				if (pkActor)
				{
					pkBox->SetOwnerGuid(kOwnerGuid);
					pkBox->ChaseOwner();
					lwActor klwActor(pkActor);
					klwActor.AttachSound(0, pkBox->PickupSound().c_str(), 1.0f);
				}
				else
				{
					bRemoveBox = true;
				}
			}
			else
			{
				bRemoveBox = true;
			}

					
			if (bRemoveBox && g_pkWorld != NULL)
			{
				g_pkWorld->RemoveObjectOnNextUpdate(kGroundBoxGuid);
			}
			return;
		}
	}

	// 아이템은 지우기라도 해야 하자나
	PgPilotManUtil::RemoveReservePilotUnit( kGroundBoxGuid, __FUNCTION__, __LINE__ );
}

extern PgStore g_kViewStore;//지금 보고있는 
extern int g_iShopViewGrp;

int InitShopCategoryTab(lwUIWnd kWnd)
{
	int iNum = 0;
	int iCategory = 1;
	bool bCheck = false;
	if (!kWnd.IsNil())
	{
		for (int i = 1; i < 5; ++i)
		{
			BM::vstring kTabName(L"CBTN_TAB");
			kTabName+=i;

			XUI::CXUI_CheckButton* pkTab = dynamic_cast<XUI::CXUI_CheckButton*>(kWnd()->GetControl(kTabName));
			if( NULL!=pkTab )
			{
				int const iCount = g_kViewStore.GetGoodsCountByView(i); 
				bool const bVisible = (0 < iCount);
				pkTab->Visible(bVisible);
				pkTab->Check(!bCheck && bVisible);
				pkTab->ClickLock(!bCheck && bVisible);
				if (bVisible)
				{
					bCheck = bVisible;
				}
				if(0==iNum)
				{
					iNum = iCount; 
					iCategory = i;
				}
			}
		}
		BM::vstring kTabName(L"CBTN_TAB");
		kTabName+=iCategory;
	}

	return iCategory;
}

void Recv_PT_M_C_ANS_STORE_ITEM_LIST(BM::Stream *pkPacket)
{//목록 받아서 띄워 주는게지.
	//상점창 두개 띄우면 어째됨?상점 두개는 안열어.
	g_kViewStore.ReadFromPacket(*pkPacket);

	std::wstring kShopName = L"FRM_SHOP";
	switch( g_kViewStore.GetType() )
	{
	case STORE_TYPE_STOCK:
		{
			kShopName = L"FRM_SHOP_STOCK";
		}break;
	case STORE_TYPE_GAMBLE:
		{

		}break;
    case STORE_TYPE_EMPORIA:
        {
            kShopName = L"SFRM_EMPORIA_STORE";
        }break;
	case STORE_TYPE_JOBKSILL:
		{
			kShopName = L"FRM_SHOP_JOB"; // TODO:
		}break;
	case STORE_TYPE_DEFAULT:
	default:
		{
			if ( g_kViewStore.IsHaveCoinItem() )
			{
				CallCoinChanger();
				return;
			}

			if( g_kViewStore.IsHaveCPItem() )
			{
				kShopName = L"FRM_SHOP_CP";
			} 
		}break;
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( kShopName, false);
	if ( pkWnd )
	{
		switch( g_kViewStore.GetType() )
		{
		case STORE_TYPE_STOCK:
        case STORE_TYPE_EMPORIA:
        case STORE_TYPE_JOBKSILL:
			{
			}break;
		default:
			{
				lwChangeShopViewGroup(InitShopCategoryTab(lwUIWnd(pkWnd)));
			}break;
		}

		PgActor *pkActor = g_kPilotMan.GetPlayerActor();
		if (pkActor)
		{
			g_kUIScene.RegistUIAction(kShopName, _T("CloseUI"), pkActor->GetTranslate());
		}
	}
}


void Recv_PT_M_C_NFY_SHINESTONE_MSG(BM::Stream* pkPacket)
{
	BM::GUID kGuid;
	int iItemNo = 0;
	pkPacket->Pop(kGuid);
	pkPacket->Pop(iItemNo);

	// 오브젝트 찾아서 먹는 코드 실행.
//	std::string strName = SHINESTONE_PREFIX_STRING;
//	std::string strID = MB(BM::vstring(iID));
//	strName += strID;
	if(g_pkWorld)
	{
		PgWorld::ShineStoneContainer::iterator itr = g_pkWorld->m_kShineStoneContainer.find(kGuid);
		if(itr == g_pkWorld->m_kShineStoneContainer.end())
		{
			PG_ASSERT_LOG(0);
			return;
		}

		PgShineStone* pkStone = itr->second;
		if(!pkStone)
		{
			PG_ASSERT_LOG(0);
			return;
		}
		
//		g_pkWorld->CountShineStone(pkStone->PlayerStone().iItemNo);
		pkStone->SetItemNo(iItemNo);
		pkStone->ChaseOwner();
	}
}


void Recv_PT_M_C_NFY_CHANGE_COMBO_COUNT(BM::Stream* pkPacket)
{
	if( !pkPacket )
	{return;}

	COMBO_TYPE iNewComboCount = 0;
	int iSkillNo = 0;

	pkPacket->Pop(iNewComboCount);
	pkPacket->Pop(iSkillNo);

	g_kChainAttack.NewCount(iNewComboCount, iSkillNo);
}

void Recv_PT_M_C_NFY_CHANGE_MISSIONSCORE_COUNT(BM::Stream* pkPacket)
{
	if( !pkPacket )
	{return;}

	EMissionUpdateType cType;
	int iSense = 0;
	int iAbility = 0;
	int iTotal = 0;

	
	pkPacket->Pop(cType);

	switch( cType )
	{
	case EMission_Update_Sense:
		{
			pkPacket->Pop(iSense);
			lua_tinker::call<void, int>("MissionUpdateSense", iSense);
		}break;
	case EMission_Update_Ability:
		{
			pkPacket->Pop(iAbility);
			lua_tinker::call<void, int>("MissionUpdateAbility", iAbility);
		}break;
	case EMission_Update_Total:
		{
			pkPacket->Pop(iTotal);
			lua_tinker::call<void, int>("MissionUpdateTotalScore", iTotal);
		}break;
	case EMission_Update_Ani:
		{
			pkPacket->Pop(iTotal);
			lua_tinker::call<void>("MissionUpdateScoreAni");
		}
	default:
		{
		}break;
	}
}

void Revc_PT_M_C_MISSION_RANK_RESULT_ITEM(BM::Stream* rkPacket)
{
	if( !rkPacket )
	{return;}

	DWORD iItemNo = 0;
	int iNewRank = 0;

	rkPacket->Pop( iItemNo );
	rkPacket->Pop( iNewRank );

	g_kMissionComplete.RankItemNo(iItemNo);
	g_kMissionComplete.NewRank(iNewRank);	
}

void Revc_PT_M_C_MISSION_ABILITY_DEMAGE(BM::Stream* pkPacket)
{
	if( !pkPacket )
	{return;}

	EMissionUpdateType cType;
	BM::GUID kCharGuid = BM::GUID::NullData();

	pkPacket->Pop(cType);
	pkPacket->Pop(kCharGuid);

	switch( cType )
	{
	case EMission_OVERHIT:
		{
			PgPilot* pkPilot = NULL;
			PgActor* pkActor = NULL;
			pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if(pkPilot)
			{
				pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					NiPoint3 kTargetPos = pkActor->GetPos();
					//NiAVObject* pkDummy = pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
					NiAVObject* pkDummy = pkActor->GetNodePointStar();
					if(pkDummy)
					{
						kTargetPos = pkDummy->GetWorldTranslate();
					}

					/*if(pkActionResult->GetAbil(AT_DAMAGEACTION_TYPE) != 0 && pkActionResult->GetValue() == 0)
					{
						return;
					}

					//스킬에 의해서 데미지를 흡수 했을 때
					if(pkActionResult->GetAbsorbValue())
					{
						return;
					}*/
					if( g_pkWorld )
					{
						g_pkWorld->m_pkDamageNumMan->AddNewMissionText(0, kTargetPos);
					}
				}
			}
		}break;
	case EMission_BACKATTACK:
		{
			PgPilot* pkPilot = NULL;
			PgActor* pkActor = NULL;
			pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if(pkPilot)
			{
				pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					NiPoint3 kTargetPos = pkActor->GetPos();
					//NiAVObject* pkDummy = pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
					NiAVObject* pkDummy = pkActor->GetNodePointStar();
					if(pkDummy)
					{
						kTargetPos = pkDummy->GetWorldTranslate();
					}

					/*if(pkActionResult->GetAbil(AT_DAMAGEACTION_TYPE) != 0 && pkActionResult->GetValue() == 0)
					{
						return;
					}

					//스킬에 의해서 데미지를 흡수 했을 때
					if(pkActionResult->GetAbsorbValue())
					{
						return;
					}*/
					if( g_pkWorld )
					{
						g_pkWorld->m_pkDamageNumMan->AddNewMissionText(1, kTargetPos);
					}
				}
			}
		}break;
	case EMission_STYLE:
		{
			PgPilot* pkPilot = NULL;
			PgActor* pkActor = NULL;
			pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if(pkPilot)
			{
				pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					NiPoint3 kTargetPos = pkActor->GetPos();
					//NiAVObject* pkDummy = pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
					NiAVObject* pkDummy = pkActor->GetNodePointStar();
					if(pkDummy)
					{
						kTargetPos = pkDummy->GetWorldTranslate();
					}

					/*if(pkActionResult->GetAbil(AT_DAMAGEACTION_TYPE) != 0 && pkActionResult->GetValue() == 0)
					{
						return;
					}

					//스킬에 의해서 데미지를 흡수 했을 때
					if(pkActionResult->GetAbsorbValue())
					{
						return;
					}*/
					if( g_pkWorld )
					{
						g_pkWorld->m_pkDamageNumMan->AddNewMissionText(2, kTargetPos);
					}
				}
			}
		}break;
	default:
		{
		}break;
	}
}

void Recv_PT_M_C_NFY_REMOVE_CHARACTER(BM::Stream& rkPacket, int const iCallType)
{
	size_t remove_count =0;
	rkPacket.Pop(remove_count);

	while( remove_count )
	{
		BM::GUID kOutGuid;
		rkPacket.Pop(kOutGuid);
 
		if ( g_kPilotMan.IsMyPlayer(kOutGuid) )
		{
			// 내 플레이어는 지우지 말자..
			// OBMode에서 이거 올 수 있어.
			--remove_count;
			continue;
		}

		DUMMY_LOG	CAUTION_LOG( BM::LOG_LV1, L"REMOVE USER1 ["<<kOutGuid<<L"], Count["<<remove_count<<L"], Type["<<iCallType<<L"]");

		PgPilot* pkPilot = g_kPilotMan.FindPilot(kOutGuid);
		if(pkPilot)
		{
			// Zone이 바뀔때 오는 패킷이므로 아무것도 체크하지 않고 지우는게 맞다.
			if (g_pkWorld)
			{
				g_pkWorld->RemoveObjectOnNextUpdate(kOutGuid);
			}
			else
			{
				PG_ASSERT_LOG( g_pkWorld );
			}

			CUnit * pkUnit = pkPilot->GetUnit();
			if ( pkUnit )
			{
				switch( pkUnit->UnitType() )
				{
				case UT_PET:
					{
						PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
						if ( pkPlayer )
						{
							if ( pkPlayer->SelectedPetID() == pkUnit->GetID() )
							{
								if ( !PgActorPet::ms_pkMyPetInventory )
								{
									PgActorPet::ms_pkMyPetInventory = new_tr PgInventory;
								}

								if ( PgActorPet::ms_pkMyPetInventory )
								{
									PgInventory* pkInven = pkUnit->GetInven();
									PgActorPet::ms_pkMyPetInventory->OwnerGuid( pkInven->OwnerGuid() );
									PgActorPet::ms_pkMyPetInventory->OwnerName( pkInven->OwnerName() );
									PgActorPet::ms_pkMyPetInventory->Swap( *pkInven );
								}

								XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"CharInfo");//UI가 열려 있으면 버튼 상태를 바꿔 줘야 함
								if(pkWnd)
								{
									g_kMyActorViewMgr.DeleteActor("PetActor");
									if( false == lwCharInfo::SetDiePetInfoToUI(pkWnd->GetControl(L"FRM_PET_INFO"), pkUnit->GetID()) )
									{
										PgPetUIUtil::SetPetUIDefaultState(true);
									}
								}
							}
						}
					}break;
				case UT_GROUNDBOX:
					{
						PgDropBox* pkDropBox = dynamic_cast<PgDropBox*>(pkPilot->GetWorldObject());

						if(pkDropBox)
						{
							BM::GUID kOwnerGuid = pkDropBox->GetOwnerGuid();
							if(BM::GUID::IsNull(kOwnerGuid) && g_pkWorld)
							{
								g_pkWorld->RemoveObjectOnNextUpdate(kOutGuid);
							}
						}
						else
						{
							//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("%s %d Unit Convert Failed"), __FUNCTIONW__, __LINE__);
						}
					}break;
				}
			}
			
			/*
			//there;//내 파일럿이면 오류가 나야 맞지?
			int const iDieMotion = pkPilot->GetAbil(AT_MANUAL_DIEMOTION);
			if(!iDieMotion)
			{//-- 0은 서버에서 죽임
				if(pkPilot->GetUnit()->IsUnitType(UT_GROUNDBOX))//아이템 박스.
				{
					PgDropBox* pkDropBox = dynamic_cast<PgDropBox*>(pkPilot->GetWorldObject());

					if(pkDropBox)
					{
						BM::GUID kOwnerGuid = pkDropBox->GetOwnerGuid();
						if(BM::GUID::IsNull(kOwnerGuid) && g_pkWorld)
						{
							g_pkWorld->RemoveObjectOnNextUpdate(kOutGuid);
						}
					}
					else
					{
						VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("%s %d Unit Convert Failed"), __FUNCTIONW__, __LINE__);
					}
				}
				else if(!pkPilot->GetUnit()->IsUnitType(UT_MONSTER)
				|| pkPilot->GetAbil(AT_HP) > 0 )
				{
					if (g_pkWorld)
						g_pkWorld->RemoveObjectOnNextUpdate(kOutGuid);
				}
			}
			else
			{//101은 죽이지 않음(후처리 필요)
				//아무것도 없음.
			}
			*/
		}
		else
		{//none;
			PgPilotManUtil::RemoveReservePilotUnit(kOutGuid, __FUNCTION__, __LINE__);
		}
		--remove_count;
	}
}

void Recv_PT_M_C_NFY_MAPMOVE_COMPLETE(BM::Stream& rkPacket)
{
	BM::GUID kCharGuid;
	BYTE cMapMoveCause = 0;

	rkPacket.Pop( kCharGuid );
	rkPacket.Pop( cMapMoveCause );

	g_kMapMoveCompleteEventMgr.Push(kCharGuid, cMapMoveCause);	// 이벤트 추가 하고
	g_kMapMoveCompleteEventMgr.Pop(kCharGuid);					// 바로 테스트

	Update_NOTI_NEW_MAIL();	//메일알림 아이콘 갱신
}

void Recv_PT_M_C_NFY_CHANGE_CP(BM::Stream& rkPacket)
{
	BYTE cCause = 0;
	int iDiffCP = 0;

	rkPacket.Pop(cCause);
	rkPacket.Pop(iDiffCP);
	
	if( !iDiffCP )
	{
		return ;
	}

	std::wstring kFormatCP;
	int const iAbsVal = (0 < iDiffCP)? iDiffCP*10: iDiffCP*-10;//절대값(CP는 보이기가 10을 곱해서 보여야 한다.)

	kFormatCP = (std::wstring)BM::vstring((int)iAbsVal);
	
	int iTTW = 0;
	switch(cCause)
	{
	//case MCE_Reward:
	case MCE_SellItem://Inc
	case MCE_Loot:
		{
			iTTW = 700128;//돈을 얻다.
		}break;
	case MCE_BuyItem://Dec
		{
			iTTW = 700129;//돈을 소비 하다
		}break;
	//case MCE_None://Lost
	default:
		{
			if( 0 < iDiffCP )
			{
				iTTW = 700128;//Inc
			}
			else
			{
				iTTW = 700129;//Dec(돈을 잃다)
			}
		}break;
	}

	std::wstring kLog;	
	bool const bRet = FormatTTW(kLog, iTTW, kFormatCP.c_str());
	if( !bRet )
	{
		return;
	}

	SChatLog kChatLog(CT_GOLD);
	g_kChatMgrClient.AddLogMessage(kChatLog, kLog);

	XUI::CXUI_Wnd*	pUpInfo = XUIMgr.Get(_T("FRM_MINIMAP"));
	if( pUpInfo )
	{
		pUpInfo->SetInvalidate();
	}
}

void Recv_PT_S_C_NFY_REFRESH_DATA(BM::Stream& rkPacket)
{//스위치 서버의 PgReloadableDataMgr 가 연관 됨.
	BM::GUID kStoreValueKey;//가격표 GUID
	DWORD dwFlag = 0;
//	rkPacket.Pop(dwFlag);
	rkPacket.Pop(kStoreValueKey);

//	if(RF_CASH_SHOP & dwFlag)
	{
	
		bool bIsCompress = false;
		size_t org_size = 0;

		rkPacket.Pop(bIsCompress);
		rkPacket.Pop(org_size);
		BM::Stream kTempPacket, kUnCompPacket;
		kTempPacket.Push(rkPacket);
		
		kUnCompPacket.Data().resize(org_size);

		if(bIsCompress)
		{
			BM::UnCompress(kUnCompPacket.Data(), kTempPacket.Data());
		}
		else
		{
			kUnCompPacket = kTempPacket;
		}
		
		kUnCompPacket.PosAdjust();

		CONT_DEF_CASH_SHOP kContShopMain;
		CONT_DEF_CASH_SHOP_ARTICLE kContShopArticle;
		CONT_CASH_SHOP_ITEM_LIMITSELL kContShopLimitSell;

		PU::TLoadTable_MM(kUnCompPacket, kContShopMain);
		PU::TLoadTable_MM(kUnCompPacket, kContShopArticle);
		PU::TLoadTable_MM(kUnCompPacket, kContShopLimitSell);

		g_kTblDataMgr.SetContDef(kContShopMain);
		g_kTblDataMgr.SetContDef(kContShopArticle);
		g_kTblDataMgr.SetContDef(kContShopLimitSell);

		g_kCashShopMgr.OnBuild(kStoreValueKey);
	}
//	*/
}

void Recv_PT_M_C_REQ_HIDDEN_MOVE_CHECK(BM::Stream &rkPacket)
{
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return;
	}

	int const	iWrongPortalInfo = 790332;
	BM::GUID	kCharGuid;
	int			m_iParam = 0;
	SItemPos	rkPos;

	rkPacket.Pop( kCharGuid );
	rkPacket.Pop( m_iParam );

	if( pkPlayer->GetID() != kCharGuid )
	{
		return;
	}


	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_iParam);
	if( !pItemDef )
	{
		::Notice_Show(TTW(790401), EL_Warning);
		return;
	}

	int const iMapNo = pItemDef->GetAbil(AT_MAP_NUM);

	if( 0 == iMapNo )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);		
		return;
	}

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return;
	}

	if( S_OK != pkInven->GetFirstItem(m_iParam, rkPos) )
	{
		wchar_t const* GetItemName = NULL;
		GetDefString(pItemDef->NameNo(), GetItemName);

		if( GetItemName )
		{
			wchar_t szTemp[1024] = {0,};
			swprintf_s(szTemp, 1024, TTW(400986).c_str(), GetItemName);
			::Notice_Show(szTemp, EL_Warning);
		}
		return;
	}	
	
	CONT_DEFMAP const* pkDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkDefMap);
	if( !pkDefMap )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	CONT_DEFMAP::const_iterator find_iter = pkDefMap->find(iMapNo);
	if( pkDefMap->end() == find_iter )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_HIDDEN_MOVE_CHECK);
	kPacket.Push(rkPos);
	kPacket.Push(lwGetServerElapsedTime32());
	NETWORK_SEND(kPacket);
}

void Recv_PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT(BM::Stream & Packet)
{
	lua_tinker::call<void, bool>("LockGlobalHotKey", true);	//글로벌 핫키 고정
	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_ExpeditionEventScirpt, true));	//캐릭터 고정 
	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_ExpeditionEventScirpt, false));	//캐릭터 고정 
}

void Recv_PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT(BM::Stream & Packet)
{
	lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 해제
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_ExpeditionEventScirpt, true));	//캐릭터 고정 해제
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_ExpeditionEventScirpt, false));	//캐릭터 고정 해제
}

void Recv_PT_M_C_NFY_CHANGE_GUARDIAN_INSTALL_DICOUNT(BM::Stream & Packet)
{
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		CONT_GUARDIAN_INSTALL_DISCOUNT RecvContanier;
		PU::TLoadTable_AA(Packet, RecvContanier);

		pPlayer->SetGuardianDicountContainer(RecvContanier);
	}
}