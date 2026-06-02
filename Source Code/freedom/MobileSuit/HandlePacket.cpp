#include "stdafx.h"
#include "lohengrin/packettype.h"
#include "lohengrin/packetstruct.h"
#include "Lohengrin/PgPlayLimiter.h"
#include "Lohengrin/ActArg.h"
#include "Variant/PgEventView.h"
#include "Variant/PgWorldEvent.h"
#include "Variant/PgWorldEventObject.h"
#include "Variant/PgMissionInfo.h"
#include "Variant/PgEventQuestBase.h"
#include "Variant/PgBattleSquare.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "lohengrin/PgRealmManager.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgQuestMan.h"
#include "PgActor.h"
#include "lwPilot.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgSoundMan.h"
#include "PgQuest.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgChatMgrClient.h"
#include "PgFriendMgr.h"
#include "PgGuild.h"
#include "PgWorld.h"
#include "lwWorld.h"
#include "PgMissionComplete.h"
#include "PgPetitionMgr.h"
#include "HandlePacket.h"//이게 마지막
#include "PgAppProtect.h"
#include "PgWebLogin.h"
#include "lwTrade.h"
#include "lwUIItemMaking.h"
#include "PgOption.h"
#include "PgSelectStage.h"
#include "PgErrorCodeTable.h"
#include "lwUIMission.h"
#include "lwRenderMan.h"
#include "PgPvPGame.h"
#include "PgEmporiaBattleClient_ProtectDragon.h"
#include "PgCoupleMgr.h"
#include "PgDropBox.h"
#include "PgMail.h"
#include "lwMarket.h"
#include "PgSystemInventory.h"
#include "PgHelpSystem.h"
#include "lwPartyUI.h"
#include "PgEmporiaMgr.h"
#include "lwMonsterKillUIMgr.h"
#include "lwCashShop.h"
#include "lwUIBook.h"
#include "lwEventView.h"
#include "PgAchieveNfyMgr.h"
#include "PgAction.h"
#include "PgGMCommand.h"
#include "lwOXQuizUI.h"
#include "PgObserverMode.h"
#include "PgWorldEventClientMgr.h"
#include "PgWEClientObjectMgr.h"
#include "lwCharacterCard.h"
#include "lwUIItemBind.h"
#include "lwUIMemTransCard.h"
#include "PgEnergyGuage.H"
#include "lwUIGemStore.h"
#include "lwCashItem.h"
#include "lwUISealProcess.h"
#include "PgCmdlineParse.h"
#include "PgBattleSquare.h"
#include "PgEmporiaAdministrator.h"
#include "PgItemMix_Script.h"
#include "PgHouse.h"
#include "lwHomeUI_Script.h"
#include "PgHome.h"
#include "PgEventQuestUI.h"
#include "lwUIIGGacha.h"
#include "lwLuckyStarUI.h"
#include "lwUIActiveStatus.h"
#include "lwPlayTime.h"
#include "HardCoreDungeon.h"
#include "lwUICSGacha.h"
#include "PgTrigger.h"
#include "PgHomeRenew.h"
#include "AlramMissionClient.h"
#include "lwUIItemRarityBuildUp.h"
#include "lwUIItemRarityAmplify.h"
#include "PgSuperGroundUI.h"
#include "lwUICostumeMix.h"
#include "lwUILogin.h"
#include "lwGuildUI.h"
#include "lwMarket.h"
#include "lwVendor.h"
#include "Lohengrin/GameTime.h"
#include "PgJobSkillLocationInfo.h"
#include "lwJobSkillLearn.h"
#include "lwJobSkillView.h"
#include "lwJobSkillItem.h"
#include "PgJobSkillLearn.h"
#include "Variant/PgSoulTransfer.h"
#include "lwSoulTransfer.h"
#include "lwUITreasureChest.h"
#include "lwDefenceMode.h"
#include "lwCommonSkillUtilFunc.h"
#include "PgMiniMap.h"
#include "PgCommunityEvent.h"
#include "PgRaceEvent.h"
#include "lwUiCharInfo.h"
#include "PgConstellation.h"

extern PgInventory g_kOtherViewInv;//상대보기 인벤.

extern void NetCallRealmUI(BM::Stream &rkPacket);
extern void NetCallChannelUI(BM::Stream &rkPacket);
extern void RecvNoticePacket( BM::Stream& kPacket );
extern void CloseResponseBar();
extern void Recv_NOTI_NEW_MAIL();
extern void Update_NOTI_NEW_MAIL();
extern void CallMapMoveCause( BYTE const byCause );
extern void Send_PT_C_S_NFY_UNIT_POS(BM::Stream &kInPacket);
extern bool lwPlaySoundByID( char* szID );

POINT3 CheckCurPosOn_PT_C_S_NFY_UNIT_POS(PgActor* pkActor)
{
	POINT3 ptPos(0,0,0);
	NiPoint3 const &niPos3 = pkActor->GetPos();
	PgAction* pkAction = pkActor->GetAction();
	if( pkAction )
	{
		if( g_pkWorld
			&& true == pkAction->GetThrowRayStatus() 
			) 
		{// 특정 액션상태에서는 바닥을 쏜 좌표값을 전달한다.
			NiPoint3 TargetPos = g_pkWorld->ThrowRay(NiPoint3(niPos3.x,niPos3.y,niPos3.z+50.0f),NiPoint3(0,0,-1),500);
			ptPos.x = TargetPos.x; ptPos.y = TargetPos.y; ptPos.z = TargetPos.z;
		}
		else
		{
			ptPos.x = niPos3.x; ptPos.y = niPos3.y; ptPos.z = niPos3.z;
			ptPos.z -= 25;
		}
	}
	else
	{
		ptPos.x = niPos3.x; ptPos.y = niPos3.y; ptPos.z = niPos3.z;
		ptPos.z -= 25;
	}
	return ptPos;
}

namespace lwSkillSet
{
	extern void lwChangeSaveState(bool const bDisable);
}

namespace PgPetUIUtil
{
	bool PetSkillToSkillTree(PgPlayer* pkPlayer);
	void SaveMyPetInventory(BM::Stream & rkPacket, BM::GUID const& rkGuid);
}

void CallLoginErrorMsgBox(int const iErrorNo, std::wstring const& strMsg)
{
	std::string strMessageBoxName = "CommonMsgBox";
	
	if( g_pkApp->UseWebLinkage()
	||	g_kCmdLineParse.IsUseForceAccount() ) //무시 모드 아니면.
	{
		switch( iErrorNo )
		{
			// 웹런처를 사용하는 국가 중 로그인 에러 메세지박스 클릭 시,
			// 클라이언트가 종료되지 않아야 한다면 여기에 추가
			case E_TLR_CHANNEL_NOT_SERVICE_NOW:
			case E_TLR_CHANNEL_FULLUSER:
				{// nothing
				}break;
			default:
				{
					strMessageBoxName = "QuitMsgBox_NC";
				}break;
		}
	}

	if (strMsg.length() > 0)
	{
		lua_tinker::call<void, char const*, bool >(strMessageBoxName.c_str(), MB(strMsg.c_str()), true);
	}
	else if( iErrorNo > E_TLR_NC_ERROR_BASE )
	{
		lua_tinker::call<void, char const*, bool >(strMessageBoxName.c_str(), MB(ECTW(iErrorNo-E_TLR_NC_ERROR_BASE)), true);
	}
	else
	{
		if ( iErrorNo < E_TLR_MOBILE_LOCK )
		{
			lua_tinker::call<void, char const*, bool >(strMessageBoxName.c_str(), MB(TTW(18000 + iErrorNo)), true);
		}
		else
		{
			lua_tinker::call<void, char const*, bool >("CallLockMsg", MB(TTW(109910 + iErrorNo)), E_TLR_MOBILE_LOCK == iErrorNo );
		}
	}
}

namespace lwPetOptChanger
{
	void lwInitDyeingEffect(void);
}

extern bool lwSendSelectCharacter(lwGUID kGuid);
bool HandlePacket(WORD const wPacketType, BM::Stream &rkPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_NFY_GROUNDOWNER_INFO:
		{
			if ( g_pkWorld )
			{
				SGroundOwnerInfo kGroundOwnerInfo;
				kGroundOwnerInfo.ReadFromPacket( rkPacket );
				g_pkWorld->GroundOwnerInfo( kGroundOwnerInfo );
			}
		}break;
	case PT_L_C_NFY_REALM_LIST:
		{	//렐름 선택 UI를 띄워준다 (렐름 선택시: PT_C_L_REQ_CHANNEL_LIST)
			NetCallRealmUI(rkPacket);
		}break;
	case PT_L_C_NFY_CHANNEL_LIST:
		{	//채널 선택 UI를 띄워준다 (채널 선택시: PT_C_L_TRY_LOGIN), (뒤로 선택시: PT_C_L_REQ_REALM_LIST)
			NetCallChannelUI(rkPacket);
		}break;
	case PT_T_C_ANS_CHANNEL_INFORMATION:
		{
			g_kNetwork.TryRealmNo(g_kNetwork.NowRealmNo());
			NetCallChannelUI(rkPacket);
		}break;
	case PT_M_C_NFY_MAPMOVE_COMPLETE:
		{
			Recv_PT_M_C_NFY_MAPMOVE_COMPLETE(rkPacket);
		}break;
	case PT_M_C_NFY_REMOVE_CHARACTER_TEST:
		{
			Recv_PT_M_C_NFY_REMOVE_CHARACTER(rkPacket,1);
		}break;
	case PT_M_C_NFY_REMOVE_CHARACTER:
		{
			Recv_PT_M_C_NFY_REMOVE_CHARACTER(rkPacket,0);
		}break;
	case PT_S_C_TRY_ACCESS_SWITCH_RESULT:
		{
			ETryLoginResult eRet = E_TLR_NONE;
			rkPacket.Pop(eRet);

			switch( eRet )
			{
			case E_TLR_SUCCESS:
				{
					BM::Stream kPacket(PT_C_S_REQ_CHARACTER_LIST);
					NETWORK_SEND_TO_SWITCH(kPacket);
					
					XUIMgr.Close(_T("LoginBg"));
					XUIMgr.Close(_T("FRM_SERVER_BG"));
					XUIMgr.Close(_T("LoginDlg"));
					XUIMgr.Close(_T("AuthFailDlg"));
					XUIMgr.Close(_T("QuitMsgBox_NC"));
					XUIMgr.Close(_T("CommonMsgBox"));
					XUIMgr.Call(_T("FRM_LOADING_IMG"));
					lwLoadingImage(0, GATTR_ALL);
				}break;
			default:
				{
					lwClearAutoLogin();
					CallLoginErrorMsgBox(eRet, std::wstring());
				}break;
			}
		}break;
	case PT_L_C_NFY_TERMINATE_USER:
		{
			int iError = 0;
			std::wstring strErrorMsg;
			rkPacket.Pop(iError);			

			lwClearAutoLogin();
			XUIMgr.Close( _T("SFRM_MSG_LOCK") );
			CallLoginErrorMsgBox(iError, std::wstring());			
		}break;
	case PT_L_C_TRY_LOGIN_RESULT:
		{
			int iError = 0;
			std::wstring strErrorMsg;
			rkPacket.Pop(iError);
			rkPacket.Pop(strErrorMsg);

			lwClearAutoLogin();
/*			
			if ( !lwClearAutoLogin() )
			{
				
				if ( E_TLR_MOBILE_LOCK > iError && E_TLR_CHANNEL_FULLUSER != iError )
				{
					XUIMgr.Close( _T("FRM_DEFAULT_REALM") );
					XUIMgr.Close( _T("FRM_CHANNEL") );
					XUIMgr.Close( _T("FRM_SERVER_BG") );
					XUIMgr.Close( _T("FRM_SV_CH_SELECT_TITLE") );
					XUIMgr.Close( _T("FRM_DEFAULT_CHANNEL") );
					XUIMgr.Activate( _T("LoginBg") );
					XUIMgr.Activate( _T("LoginDlg") );
				}
			}
*/
			XUIMgr.Close( _T("SFRM_MSG_LOCK") );
			CallLoginErrorMsgBox(iError, strErrorMsg);

			if( true == g_bUseLoginFailExit )
			{
				if( E_TLR_WRONG_PASS == iError )
				{
					++LoginUtil::g_iLoginFailExitCount;

					if( LoginUtil::LOGIN_FAIL_COUNT <= LoginUtil::g_iLoginFailExitCount )
					{
						Sleep(100);
						lwExitClient();
					}
				}
			}
		}break;
	case PT_L_C_NFY_ACTIVE_FATIGUE:
		{//! 피로도 UI 호출
			if( !g_kNetwork.IsAutoLogin() )
			{
				g_klwPlayTime.CallFatigueUI();
			}
		}break;
	case PT_L_C_NFY_LAST_LOGOUT_TIME:
		{
			BM::DBTIMESTAMP_EX dtLastLogout;
			rkPacket.Pop(dtLastLogout);
			g_klwPlayTime.SetLastLogOutTime(dtLastLogout);
		}break;
	case PT_M_C_NOTI_MONEY_PACK:
		{
			int iMoney = 0;
			rkPacket.Pop(iMoney);

			int iGold = iMoney * 0.0001f;
			int iSilver = (int)(iMoney * 0.01f) % 100;
			int iCopper = iMoney % 100;

			BM::vstring	kString(L"");
			if( 0 != iGold )
			{
				kString += iGold;
				kString += TTW(401111);
				if( 0 != iCopper || 0 != iSilver ){ kString += L" "; };
			}
			if( 0 != iSilver )
			{
				kString += iSilver;
				kString += TTW(401112);
				if( 0 != iCopper ){ kString += L" "; };
			}
			if( 0 != iCopper )
			{
				kString += iCopper;
				kString += TTW(401113);
			}
			kString += TTW(2151);
			lua_tinker:: call<void, char const*, bool>("CommonMsgBox", MB((std::wstring const&)kString), true);
		}break;
	case PT_M_C_NOTI_CASH_PACK:
	{
		__int64 iCash = 0;
		__int64 iBonus = 0;
		rkPacket.Pop(iCash);
		rkPacket.Pop(iBonus);

		BM::vstring kString;
		BM::vstring kCash(iCash);
		BM::vstring kBonus(iBonus);

		if (iCash && iBonus)
			kString += kCash + TTW(2153) + L"\n" + kBonus + TTW(2152);
		else if (iCash)
			kString += kCash + TTW(2152);
		else if (iBonus)
			kString += kBonus + TTW(2153);
		else
			kString += L"iCash and iBouns are 0!";

		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB((std::wstring const&)kString), true);
	}break;
	case PT_M_C_NFY_USEITEM:
		{
			std::wstring kName;
			int	iItemNo;
			int	iGroundNo;

			rkPacket.Pop(kName);
			rkPacket.Pop(iItemNo);
			rkPacket.Pop(iGroundNo);

			std::wstring kItemName;
			MakeItemName(iItemNo, SEnchantInfo(), kItemName);
			std::wstring kMapName = GetMapName(iGroundNo);

			GET_DEF(CItemDefMgr, kItemDefMgr);
			int iEffectNum = kItemDefMgr.GetAbil(iItemNo, AT_EFFECTNUM1);
			if( iEffectNum )
			{
				const wchar_t *pName = NULL;
				CONT_DEFEFFECT const *pkDefEffect = NULL;
				g_kTblDataMgr.GetContDef(pkDefEffect);
				if( pkDefEffect )
				{
					CONT_DEFEFFECT::const_iterator iter = pkDefEffect->find(iEffectNum);
					if( pkDefEffect->end() != iter )
					{
						GetDefString(iter->second.iName, pName);
					}
				}
				if( pName )
				{
					wchar_t szTemp[1024] = {0,};
					swprintf_s(szTemp, 1024, TTW(699981).c_str(), kName.c_str(), kItemName.c_str(), kMapName.c_str(), pName);
					lwAddWarnDataStr(szTemp, 1);
				}
			}
			else
			{
				wchar_t szTemp[1024] = {0,};
				swprintf_s(szTemp, 1024, TTW(699982).c_str(), kName.c_str(), kMapName.c_str(), kItemName.c_str());
				lwAddWarnDataStr(szTemp, 1);
			}
		}break;
	case PT_M_C_ANS_HIDDEN_ITEM_PACK:
		{
			HRESULT	hResult;
			rkPacket.Pop(hResult);

			switch( hResult )
			{
			case S_OK:
				{
				}break;
			default:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699972, true);
				}break;
			}
		}break;
	case PT_M_C_ANS_OPEN_GAMBLE:
		{
			HRESULT	hResult;
			rkPacket.Pop(hResult);

			switch( hResult )
			{
			case S_OK:
				{
					Recv_PT_M_C_ANS_OPEN_GAMBLE(&rkPacket);
				}break;
			case E_GAMBLE_NOT_FOUND_ITEM:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699971, true);
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_GAMBLE" );
					if(!pkWnd)
					{
						break;
					}
					pkWnd->Close();
				}break;
			case E_GAMBLE_NO_MESSAGE:
				{
					// no message;
				}break;
			default:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699972, true);
				}break;
			}
		}break;
	case PT_M_C_NOTI_OPEN_GAMBLE:
		{
			std::wstring kUserName;
			int iItemNo;
			int iGetItemNo;

			rkPacket.Pop(kUserName);
			rkPacket.Pop(iItemNo);
			rkPacket.Pop(iGetItemNo);

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pUseItemDef = kItemDefMgr.GetDef(iItemNo);
			CItemDef const *pGetItemDef = kItemDefMgr.GetDef(iGetItemNo);
			if(!pUseItemDef || !pGetItemDef)
			{
				return false;
			}

			wchar_t const* UseItemName = NULL;
			wchar_t const* GetItemName = NULL;
			GetDefString(pUseItemDef->NameNo(), UseItemName);
			GetDefString(pGetItemDef->NameNo(), GetItemName);

			if( UseItemName && GetItemName )
			{
				wchar_t szTemp[1024] = {0,};
				swprintf_s(szTemp, 1024, TTW(699980).c_str(), kUserName.c_str(), UseItemName, GetItemName);

				XUI::CXUI_Wnd* pkMsgWnd = XUIMgr.Activate(L"FRM_GACHA_MSG");
				if( pkMsgWnd )
				{
					pkMsgWnd->Text(szTemp);
					pkMsgWnd->AliveTime(5000);
					XUI::CXUI_Style_String kStyleStr = pkMsgWnd->StyleText();
					POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleStr));
					pkMsgWnd->Size(POINT2(pkMsgWnd->Size().x, kTextSize.y));
				}
			}
		}break;	
	case PT_M_C_ANS_EVENT_ITEM_REWARD:
		{
			HRESULT	hResult;
			rkPacket.Pop(hResult);

			switch( hResult )
			{
			case S_OK:
				{
					Recv_PT_M_C_ANS_EVENT_ITEM_REWARD(&rkPacket);
				}break;
			case E_GAMBLE_NOT_FOUND_ITEM:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699971, true);
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_EVENT_BOX" );
					if(!pkWnd)
					{
						break;
					}
					pkWnd->Close();
				}break;
			default:
				{
//					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 699972, true);
				}break;
			}
		}break;
	case PT_M_C_NOTI_EVENT_ITEM_REWARD:
		{
			std::wstring kUserName;
			int iItemNo;
			int iGetItemNo;

			rkPacket.Pop(kUserName);
			rkPacket.Pop(iItemNo);
			rkPacket.Pop(iGetItemNo);

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pUseItemDef = kItemDefMgr.GetDef(iItemNo);
			CItemDef const *pGetItemDef = kItemDefMgr.GetDef(iGetItemNo);
			if(!pUseItemDef || !pGetItemDef)
			{
				return false;
			}

			wchar_t const* UseItemName = NULL;
			wchar_t const* GetItemName = NULL;
			GetDefString(pUseItemDef->NameNo(), UseItemName);
			GetDefString(pGetItemDef->NameNo(), GetItemName);

			if( UseItemName && GetItemName )
			{
				wchar_t szTemp[1024] = {0,};
				swprintf_s(szTemp, 1024, TTW(699980).c_str(), kUserName.c_str(), UseItemName, GetItemName);

				XUI::CXUI_Wnd* pkMsgWnd = XUIMgr.Activate(L"FRM_EVENT_ITEM_MSG");
				if( pkMsgWnd )
				{
					pkMsgWnd->Text(szTemp);
					pkMsgWnd->AliveTime(5000);
					XUI::CXUI_Style_String kStyleStr = pkMsgWnd->StyleText();
					POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleStr));
					pkMsgWnd->Size(POINT2(pkMsgWnd->Size().x, kTextSize.y));
				}
			}
		}break;
	case PT_M_C_ANS_TREASURE_CHEST:
		{
			HRESULT	hResult;
			rkPacket.Pop(hResult);
			switch( hResult )
			{
			case S_OK:
				{
					g_kTreasureChestMgr.Recv_PT_M_C_ANS_TREASURE_CHEST(&rkPacket);
				}break;
			case E_TREASURE_CHEST_UNMATCH:
				{
					::Notice_Show(TTW(699951), EL_Warning, true);
				}break;
			case E_TREASURE_CHEST_NOT_FOUND_CHEST:
				{
					::Notice_Show(TTW(699950), EL_Warning, true);
				}break;
			case E_TREASURE_CHEST_NOT_FOUND_CHEST_INV:
				{
					::Notice_Show(TTW(699952), EL_Warning, true);
				}break;
			case E_TREASURE_CHEST_NOT_FOUND_KEY_INV:
				{
					::Notice_Show(TTW(699952), EL_Warning, true);
				}break;
			default:
				{
				}break;
			}
		}break;
	case PT_M_C_ANS_TREASURE_CHEST_INFO:
		{
			HRESULT	hResult;
			rkPacket.Pop(hResult);
			switch(hResult)
			{
			case S_OK:
				g_kTreasureChestMgr.Recv_PT_M_C_ANS_TREASURE_CHEST_INFO(&rkPacket);
				break;
			default:
				::Notice_Show(TTW(699954), EL_Warning);
				break;
			}
		}break;
	case PT_S_C_ANS_AUTOLOGIN:
		{
			g_kNetwork.RecvAutoLogin(rkPacket);
		}break;
	case PT_S_C_ANS_CHARACTER_LIST:
		{
			//캐릭터 초기화
			g_kAchieveNfyMgr.ClearShowAchievement();

			//
			if ( g_kNetwork.TryRealmNo() )
			{
				g_kNetwork.NowRealmNo( g_kNetwork.TryRealmNo() );
			}

			if ( g_kNetwork.TryChannelNo() )
			{
				g_kNetwork.NowChannelNo(g_kNetwork.TryChannelNo());
			}
			
			g_kNetwork.TryRealmNo(0);
			g_kNetwork.TryChannelNo(0);

			BM::GUID const &kSelectCharacterGuid = g_kNetwork.SelectCharGuid();
			if ( BM::GUID::IsNotNull(kSelectCharacterGuid) )
			{
				lwSendSelectCharacter( lwGUID(kSelectCharacterGuid) );
			}
			else
			{
				lwRenderMan kRenderMan(NULL);
				kRenderMan.Draw();
				kRenderMan.CleanUpScene("");
				kRenderMan.AddScene( PgWorldUtil::kCharacterSelectStateID.c_str() );
				

				g_kNetwork.oldSelectCharacterSec(0);
			}

			if( g_pkWorld )
			{
				lua_tinker::call<void,lwWorld,lwPacket>("Net_AddReadyCharacter",lwWorld(g_pkWorld),lwPacket(&rkPacket));
			}

			g_kNetwork.ConnectionState(EConnect_Switch);
		}break;
	case PT_T_C_ANS_SELECT_CHARACTER:
		{
			ESelectCharacterRet kRet = E_SCR_SUCCESS;
			rkPacket.Pop( kRet );
			switch ( kRet )
			{
			case E_SCR_CHARACTERCOUNT_ERROR:
				{
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(700411)), true);//메시지
				}break;
			case E_SCR_SYSTEMCHECK:
				{
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(700090)), true);//메시지
				}break;
			case E_SCR_ErrorState:
				{
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(50518)), true);//메시지
				}break;
			case E_SCR_RECENEMAP_ERROR:
			case E_SCR_Q_RANDOMMAP_MOVE:
				{
					int iRecentGndNo = 0;
					int iLastVillage = 0;
					rkPacket.Pop( iRecentGndNo );
					rkPacket.Pop( iLastVillage );

					std::wstring wstrText;
					TBL_DEF_MAP const *pMap = PgWorld::GetMapData(iRecentGndNo);
					if(pMap)
					{
						wchar_t const *pText1 = NULL;
						wchar_t const *pText2 = NULL;
						if( GetDefString( pMap->NameNo, pText1) )
						{
							pMap = PgWorld::GetMapData(iLastVillage);
							if ( pMap )
							{
								if( GetDefString( pMap->NameNo, pText2) )
								{	
									WstringFormat( wstrText, MAX_PATH, TTW( kRet == E_SCR_RECENEMAP_ERROR ? 181 : 183 ).c_str(), pText1, pText2 );
								}
							}
						}
					}

					BM::Stream kPacket( PT_C_T_REQ_SELECT_CHARACTER_OTHERMAP, iLastVillage );
					BM::Stream kCancelPacket( PT_C_T_REQ_SELECT_CHARACTER_OTHERMAP, 0 );
					lua_tinker::call<bool, char const*, lwPacket, lwPacket>("CommonNetBox", MB(wstrText), lwPacket(&kPacket), lwPacket(&kCancelPacket) );//메시지
				}break;
			case E_SCR_LASTVILLAGE_ERROR:
				{
					int iLastVillage = 0;
					rkPacket.Pop( iLastVillage );
					TBL_DEF_MAP const *pMap = PgWorld::GetMapData(iLastVillage);
					if(pMap)
					{
						wchar_t const *pText = NULL;
						if( GetDefString( pMap->NameNo, pText) )
						{
							std::wstring wstrText;
							WstringFormat( wstrText, MAX_PATH, TTW(182).c_str(), pText );
							lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(wstrText), true);//메시지
						}
					}
				}break;
			case E_SCR_PLAYTIMEOVER:
				{
					int iRemainSec = 0;
					rkPacket.Pop( iRemainSec );
					
					std::wstring kTimeText;
					TimeToString(static_cast<DWORD>(iRemainSec), kTimeText);

					BM::vstring vstrText(TTW(85)); 
					vstrText.Replace( L"#RemainTime#", kTimeText );
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(vstrText), true);//메시지
				}break;
			}

			XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("FRM_CHAR_LIST"));
			if( !pkTopWnd )
			{
				PG_ASSERT_LOG(0 && "Can't find FRM_CHAR_LIST"); 
				break;
			}

			XUI::CXUI_Wnd* pkTempWnd = pkTopWnd->GetControl(_T("IMG_LOW"));
			if( !pkTempWnd ) 
			{
				PG_ASSERT_LOG(0 && "Can't find IMG_LOW"); 
				break;
			}

			XUI::CXUI_Wnd* pkStartFrm = pkTempWnd->GetControl(_T("StartGame"));
			if( !pkStartFrm ) 
			{
				PG_ASSERT_LOG(0 && "Can't find StartGame"); 
				break;
			}

			XUI::CXUI_Button* pkStartBtn = dynamic_cast<XUI::CXUI_Button*>(pkStartFrm->GetControl(_T("btnStartGame")));
			if( !pkStartBtn ) 
			{
				PG_ASSERT_LOG(0 && "Can't find btnStartGame"); 
				break;
			}

			pkStartBtn->IsClosed(false);//시작버튼 잠금 해제

			if(E_SCR_SUCCESS != kRet)
			{
				g_kNetwork.SelectCharGuid(BM::GUID::NullData());
			}
		}break;
	case PT_S_C_ANS_ACCESS_SWITCH_SECOND:
		{
			BM::GUID guidSwitchKey;
			rkPacket.Pop( guidSwitchKey );
			g_kAppProtect.CheckState( rkPacket, guidSwitchKey );
		}break;
	case PT_S_C_ANS_DELETE_CHARACTER:
		{
			BYTE cDeleteRet = 0;
			BM::GUID kGuid;
			rkPacket.Pop(cDeleteRet);
			rkPacket.Pop(kGuid);//앞으로 변경될꺼다

			int iTTW = 0;
			switch(cDeleteRet)
			{
			case UCDR_Success://삭제 성공
				{
					PgSelectStage* pkSelectStage = &g_kSelectStage;//g_pkWorld->m_pkSelectStage;
					if( pkSelectStage && !pkSelectStage->OnDeleteWaitCharacter(kGuid) )
					{
						BM::GUID const &rkSelectedActorGuid = pkSelectStage->GetSelectedActor();
						if( g_pkWorld
							&& BM::GUID::NullData() != rkSelectedActorGuid 
							&& kGuid == rkSelectedActorGuid 
							)
						{
							g_pkWorld->RemoveObject(rkSelectedActorGuid);
						}
						pkSelectStage->RemoveSpawnSlot(rkSelectedActorGuid);

						pkSelectStage->SelectDefaultCharacter();
//						lua_tinker::call<void>("RotateTreeInit");
						int const iUseExtendSlot = g_kSelectStage.ExtendSlot();
						lua_tinker::call<void, int>("SetLockedExtendSlot", iUseExtendSlot );
					}
					iTTW = 700096;
				}break;
			case UCDR_Failed_GuildMaster://길마면 안되
				{
					iTTW = 700098;
				}break;
			case UCDR_Failed://실패(캐릭터를 찾지 못했다)
			default:
				{
					iTTW = 700097;
				}break;
			}

			if( iTTW )
			{
				std::wstring const kMessage = TTW(iTTW);
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case PT_N_C_NFY_HAVE_PENALTY:
		{
			WORD wType = 0;
			__int64 i64RemainTime = 0i64;
			rkPacket.Pop( wType );
			rkPacket.Pop( i64RemainTime );

			BM::vstring vstr( TTW(900000 + static_cast<int>(wType)) );
			vstr += TTW(900000);
			vstr += i64RemainTime;
			vstr += TTW(9);

			Notice_Show( vstr, EL_Warning );
		}break;
	case PT_N_C_ANS_CHECK_CHARACTERNAME_OVERLAP: // 캐릭터명 중복 체크
		{
			EUserCharacterRealmMergeReturn eResult = UCRMR_None;
			std::wstring kNewName;

			rkPacket.Pop( eResult );
			rkPacket.Pop( kNewName );

			if( UCRMR_Duplicate == eResult ) // 중복된 캐릭터명
			{
				XUI::CXUI_Wnd* pRealmRequest = XUIMgr.Get(L"SFRM_COMBINE_REQUEST");
				if( pRealmRequest )
				{
					XUI::CXUI_Button* pRenameOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_RENAME_OK"));
					XUI::CXUI_Button* pOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_OK"));
					if( pOk )
					{
						pRenameOk->Disable(false);
						pOk->Disable(true);
					}
				}

				const int iTTW = 50514;
				std::wstring const kMessage = TTW(iTTW);
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
			else
			{
				XUI::CXUI_Wnd* pRealmRequest = XUIMgr.Call(L"SFRM_COMBINE_REQUEST");
				if( pRealmRequest )
				{
					XUI::CXUI_Wnd* pAfter = pRealmRequest->GetControl(L"FRM_CARD_AFTER");
					XUI::CXUI_Wnd* pBefore = pRealmRequest->GetControl(L"FRM_CARD_BEFORE");
					XUI::CXUI_Button* pRenameOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_RENAME_OK"));
					XUI::CXUI_Button* pOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_OK"));
					if( pBefore && pAfter && pRenameOk && pOk )
					{
						pRenameOk->Disable(true);
						pOk->Disable(false);
						g_kSelectStage.SetCharInfoToCard( pBefore, pOk->OwnerGuid() );
						g_kSelectStage.SetCharInfoToCard( pAfter, pOk->OwnerGuid(), kNewName.c_str());
					}
				}
			}
		}break;
	case PT_N_C_ANS_REALM_MERGE:
		{
			EUserCharacterRealmMergeReturn kRet = UCRMR_None;
			BM::GUID kRenamedCharGuid;
			std::wstring kNewName;

			rkPacket.Pop( kRet );
			rkPacket.Pop( kRenamedCharGuid );
			rkPacket.Pop( kNewName );

			int iTTW = 0;
			switch(kRet)
			{
			case UCRMR_Success: // 삭제 성공
				{
					PgSelectStage* pkSelectStage = &g_kSelectStage;//g_pkWorld->m_pkSelectStage;
					if( pkSelectStage 
						&& g_pkWorld 
						)
					{
						if( BM::GUID::NullData() != kRenamedCharGuid )
						{
							PgActor* pkActor = dynamic_cast< PgActor* >( g_pkWorld->FindObject(kRenamedCharGuid) );
							if( pkActor )
							{
								PgPilot* pkPilot = pkActor->GetPilot();
								if( pkPilot )
								{
									pkPilot->SetName( kNewName );

									PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkPilot->GetUnit());
									if( pkPlayer )
									{
										pkPlayer->ClearDBPlayerState();
									}
								}
								pkActor->UpdateName();
								pkSelectStage->MoveSlotDelToSpawn(kRenamedCharGuid);
							}
						}
					}
					iTTW = 50511; // 랠름통함에 성공.
				}break;
			case UCRMR_BadName: // 잘못된 단어가 포함되어져 있습니다.
				{
					iTTW = 50513;
				}break;
			case UCRMR_Duplicate: // 중복된 이름입니다.
				{
					iTTW = 50514;
				}break;
			case UCRMR_MaxLen: // 최대 길이를 넘어섰 습니다.
				{
					iTTW = 50515;
				}break;
			case UCRMR_MaxSlot: // 캐릭터를 옮길 수 있는 슬롯이 없습니다.
				{
					iTTW = 50517;
				}break;
			case UCRMR_EmptyName:
				{
					iTTW = 50520;
				}break;
			case UCRMR_Failed: // 이름을 바꿀 수 있는 캐릭터 상태가 아니다
			case UCRMR_None:
			default:
				{
					iTTW = 50516;
				}break;
			}

			if( iTTW )
			{
				std::wstring const kMessage = TTW(iTTW);
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case PT_N_C_ANS_MAP_MOVE_CHECK_FAILED:
		{
			BYTE byErrorType = 0;
			rkPacket.Pop( byErrorType );
			CallMapMoveCause( byErrorType );
		}break;
	case PT_T_C_ANS_MAP_MOVE_TARGET_FAILED:
		{
			bool bGMCommand = false;
			T_GNDATTR kGndAttr = GATTR_DEFAULT;
			bool bLoginedCharacter = false;
			std::wstring wstrTargetCharacterName;

			rkPacket.Pop( bGMCommand );
			rkPacket.Pop( kGndAttr );
			rkPacket.Pop( bLoginedCharacter );
			if ( bLoginedCharacter )
			{
				rkPacket.Pop( wstrTargetCharacterName );
			}
			
			int const iLevel = ((true == bGMCommand) ? EL_GMMsg : EL_Warning);

			if ( bLoginedCharacter )
			{
				switch ( kGndAttr )
				{
				case GATTR_PVP:
					{
						Notice_Show( TTW(99102), iLevel );
					}break;
				case GATTR_EMPORIABATTLE:
					{
						Notice_Show( TTW(99103), iLevel );
					}break;
				default:
					{
						std::wstring wstrMsg;
						WstringFormat( wstrMsg, MAX_PATH, TTW(99104).c_str(), wstrTargetCharacterName.c_str() );
						Notice_Show( wstrMsg, iLevel );
					}break;
				}
			}
			else
			{
				Notice_Show( TTW(99101), iLevel );
			}
		}break;
	case PT_T_C_NFY_MAP_MOVING_CHANNEL_CHANGE:
		{// 공용채널로 이동할 떄 클리어 해주어야 하는것들을 여기다 정의
			g_kParty.Clear(true);
		}break;
	case PT_M_C_NFY_ABILCHANGED:
		{
			//	Effect 에 의해 어빌이 변화되었을 경우 오는 패킷
			WORD	wAbilID;
			int	iNewValue;
			BM::GUID kTargetGuid,kCasterGuid;
			int	iEffectNo;
			int iDelta;

			rkPacket.Pop(kTargetGuid);
			rkPacket.Pop(wAbilID);
			rkPacket.Pop(iNewValue);
			rkPacket.Pop(kCasterGuid);
			rkPacket.Pop(iEffectNo);
			rkPacket.Pop(iDelta);

			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kTargetGuid);
			if(pkPilot)
			{
				PgActor	*pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					pkActor->AbilChangedByEffect(kCasterGuid, wAbilID, iNewValue, iDelta);
				}
				pkPilot->SetAbil(wAbilID,iNewValue);
			}
		}break;
	case PT_L_C_NFY_RESERVED_SWITCH_INFO:
		{
			BM::Stream *pkPacket = &rkPacket;
			
			SSwitchReserveResult kSRR;
			kSRR.ReadFromPacket(*pkPacket);

			//이미 유저 ID와 PW를 가진다. 다시 대입필요 없음.
			//여기서 입력한 아이디 값이 무조건 대문자로 변경됨
			//그런고로 버그 수정겸 주석 처리.
			//g_kNetwork.LoginID(kSRR.szID);
			//g_kNetwork.LoginPW(kSRR.szPW);


			g_kNetwork.SetSwitchAuthKey(kSRR.guidSwitchKey);
			//INFO_LOG( BM::LOG_LV6, _T("스위치 정보 받음! [%s:%d]"), SwitchAddr.wstrIP.c_str(), SwitchAddr.wPort);

			g_kNetwork.ConnectSwitchServer(kSRR.addrSwitch);
		}break;
	case PT_M_C_NFY_ABIL_SYNC:
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			assert(pkPlayer);
			if(pkPlayer)
			{
				pkPlayer->RecvAllAbil(rkPacket);
				RefreshCharStateUI();
			}
		}break;
	case PT_M_C_UNIT_POS_CHANGE:
		{
			PG_ASSERT_LOG( g_pkWorld );
			if ( g_pkWorld )
			{
				g_pkWorld->Recv_PT_M_C_UNIT_POS_CHANGE( rkPacket );
			}
		}break;
	case PT_M_C_NFY_YOU_ARE_REVIVE:
		{
			BM::GUID kCharacterGuid;
			int iHP = 0;
			int iMP = 0;
			bool bRevive = false;
			rkPacket.Pop(kCharacterGuid);
			rkPacket.Pop(iHP);
			rkPacket.Pop(iMP);
			rkPacket.Pop(bRevive);

			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kCharacterGuid);
			
			if ( pkPilot )
			{
				Recv_PT_M_C_NFY_STATE_CHANGE2( pkPilot, E_SENDABIL_TARGET_UNIT, SAbilInfo(AT_HP,iHP) );
				Recv_PT_M_C_NFY_STATE_CHANGE2( pkPilot, E_SENDABIL_TARGET_UNIT, SAbilInfo(AT_MP,iMP) );				

				CUnit* pkUnit = pkPilot->GetUnit();
				if ( pkUnit )
				{
					pkUnit->GetSkill()->ClearCoolTime();
				}

				PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
				if ( pkActor )
				{
					pkActor->ResetSkillCoolTimeFromUnit();
					if ( bRevive )
					{
						pkActor->AddEffect(ACTIONEFFECT_REVIVE,0,0,kCharacterGuid,0,0,true);
						pkActor->RefreshHPGaugeBar(0, iHP, NULL);
					}
					pkActor->UpdateName();
					PgActor* pkSubActor = PgActorUtil::GetSubPlayerActor(pkActor);
					if(pkSubActor)
					{
						pkSubActor->AddEffect(ACTIONEFFECT_REVIVE,0,0,kCharacterGuid,0,0,true);
						//pkSubActor->ReserveTransitAction("a_twin_sub_repos");
						pkSubActor->SetCanHit(true);
					}
				}
			}
			else
			{
				g_kPilotMan.BufferReservedPilotPacket( kCharacterGuid, &rkPacket, 0 );
			}

			if( g_kPilotMan.IsMyPlayer(kCharacterGuid) )
			{// 내가 부활일때만 부활 UI 닫음
				lwCloseUI("DeadDlg");// 사망 다이얼로그 종료.
				lwCloseUI("DeadDlg_NoRevive");// 사망 다이얼로그 종료.
				RefreshCharStateUI();
				CloseResponseBar();
			}
		}break;
	case PT_M_C_CS_NOTI_CASH_MODIFY:
		{
			int iDiffCash = 0;
			rkPacket.Pop(iDiffCash);

			if(iDiffCash)
			{
				int iTTWNo = 0;
				int const iDisplayValue = abs(iDiffCash);
				if(iDiffCash < 0)
				{
					iTTWNo = 1990;	// 캐시 %d를 소비 하였습니다.
				}
				else
				{
					iTTWNo = 1991;	// 캐시 %d를 획득 하였습니다.
				}

				std::wstring kLog;
				bool const bRet = FormatTTW(kLog, iTTWNo, iDisplayValue);
				if(true == bRet)
				{
					SChatLog kChatLog(CT_GOLD);
					g_kChatMgrClient.AddLogMessage(kChatLog, kLog);
				}
			}
		}break;
	case PT_M_C_NFY_STATE_CHANGE2:
		{
			BM::GUID kCharacterGuid;
			BYTE byTarget = 0;
			BYTE byCount = 0;

			rkPacket.Pop(kCharacterGuid);
			rkPacket.Pop(byTarget);
			rkPacket.Pop(byCount);
			
			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kCharacterGuid);
			bool bMyPet = false;
			if(pkPilot)
			{
				CUnit* pkUnit = pkPilot->GetUnit();
				if(pkUnit && pkUnit->IsUnitType(UT_PET))
				{
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if( pkPlayer && pkPlayer->SelectedPetID().IsNotNull() &&
						pkPlayer->SelectedPetID() == kCharacterGuid ) //내 펫인가?
					{
						bMyPet = true;
					}
				}
			}
			
			bool const bMyPlayer = g_kPilotMan.IsMyPlayer(kCharacterGuid);
			int iOldSp = 0;
			int iOldBnsStatus = 0;

			if(bMyPlayer && pkPilot)
			{// 자신의 플레이어일때 
				iOldSp = pkPilot->GetAbil(AT_SP);						// 변경되기전 SP 양을 얻어옴
				iOldBnsStatus = pkPilot->GetAbil(AT_STATUS_BONUS);	// 변경되기전 Bonus Status를 얻어옴
			}

			while( byCount-- )
			{
				SAbilInfo kAbilInfo;
				rkPacket.Pop(kAbilInfo);

				if ( pkPilot )
				{	
					Recv_PT_M_C_NFY_STATE_CHANGE2( pkPilot, (E_SENDABIL_TARGET)byTarget, kAbilInfo );
					lwAchieveNfyMgr::AddAchievementNfyMsg(kAbilInfo.wType);
				}
				else
				{
					if ( kAbilInfo.wType == AT_HP && 0 >= kAbilInfo.iValue )
					{
						_PgOutputDebugString("Recv_PT_M_C_NFY_STATE_CHANGE2 AT_HP ==0 But Pilot Is Null. Try to Remove From Reserved Pilot\n");
						PgPilotManUtil::BufferReservedPilotPacket_NotUnitTypeRemove( kCharacterGuid, UT_PLAYER, &rkPacket, __FUNCTION__, __LINE__);
						return true;
					}
				}
				if(bMyPlayer)
				{// 내 플레이어이고
					switch(kAbilInfo.wType)
					{
					case AT_STATUS_BONUS:
						{// Active Status 호출 버튼
							int  const iDelta = kAbilInfo.iValue - iOldBnsStatus;
							if(0 < iDelta)
							{// Bonus Status 값이 증가 했으면
								XUIMgr.Call(L"FRM_STATUS_UP");
								lua_tinker::call<void, bool>("SetStatusBtnBlink", true);
							}
							else if(0 >= kAbilInfo.iValue)
							{// Bonus Status 값이 존재 하지 않으면
								XUIMgr.Close(L"FRM_STATUS_UP");
							}
						}break;
					case AT_SP:
						{// SP가 수정되었고
							int  const iDelta = kAbilInfo.iValue - iOldSp;
							if(0 < iDelta)
							{// 현재 SP가 남아 있다면스킬 아이콘을 반짝이게 함
								lua_tinker::call<void, bool >("SetSkillIconBlink_InMainUI", true);
							}
						}break;
					default:
						{
						}break;
					}
				}
			}// while

			if ( !pkPilot )
			{
				g_kPilotMan.BufferReservedPilotPacket( kCharacterGuid, &rkPacket, 0 );
			}

			if( bMyPlayer )
			{
				RefreshCharStateUI();
				break;
			}

			//PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			//if(!pkPlayer)	{break;}

			//if(kCharacterGuid!=pkPlayer->SelectedPetID())	{break;}
			XUI::CXUI_Wnd *pkInfo = XUIMgr.Get(L"CharInfo");
			if(!pkInfo)		{break;}

			if(bMyPlayer || bMyPet) //내 플레이어일 경우만 UI 갱신
			{
				lwCharInfo::SetPetInfoToUI(pkInfo->GetControl(L"FRM_PET_INFO"), kCharacterGuid, true);
			}

		}break;
	case PT_M_C_NFY_STATE_CHANGE64:
		{
			BM::GUID kCharacterGuid;
			BYTE byCount = 0;

			rkPacket.Pop(kCharacterGuid);
			
			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kCharacterGuid);
			if(pkPilot)
			{
				rkPacket.Pop(byCount);

				std::vector<SAbilInfo64> kAbils;
				kAbils.reserve(byCount);
				bool const bMyPlayer = g_kPilotMan.IsMyPlayer(kCharacterGuid);
				while(byCount--)
				{
					SAbilInfo64 kAbilInfo;
					rkPacket.Pop(kAbilInfo);
					if(AT_EXPERIENCE == kAbilInfo.wType)
					{// 플레이어 경험치 처리는 DISCT_MODIFY_VALUE에서 처리함
						continue;
					}
					kAbils.push_back(kAbilInfo);
				}
			
				std::vector<SAbilInfo64>::iterator abil_itor = kAbils.begin();
				while(abil_itor != kAbils.end())
				{
					Recv_PT_M_C_NFY_STATE_CHANGE64(pkPilot, (*abil_itor));
					++abil_itor;
				}

				if(g_kPilotMan.IsMyPlayer(kCharacterGuid))
				{
					RefreshCharStateUI();
				}
			}
		}break;
	case PT_M_C_NFY_CHANGE_MONEY:
		{
			Recv_PT_M_C_NFY_CHANGE_MONEY(rkPacket);
		}break;
	case PT_M_C_NFY_CHANGE_CP:
		{
			Recv_PT_M_C_NFY_CHANGE_CP(rkPacket);
		}break;
	case PT_M_C_NFY_QUICKSLOTCHANGE:
		{
			Recv_PT_M_C_NFY_QUICKSLOTCHANGE(rkPacket);
		}break;
	case PT_M_C_NFY_ITEM_CHANGE:
		{
			Recv_PT_M_C_NFY_ITEM_CHANGE(&rkPacket);
		}break;
	case PT_M_C_NFY_WARN_MESSAGE:
		{
			Recv_PT_M_C_NFY_WARN_MESSAGE(&rkPacket);
		}break;
	case PT_M_C_NFY_WARN_MESSAGE2:
		{
			Recv_PT_M_C_NFY_WARN_MESSAGE2(&rkPacket);
		}break;
	case PT_M_C_NFY_WARN_MESSAGE3:
		{
			Recv_PT_M_C_NFY_WARN_MESSAGE3(&rkPacket);
		}break;
	case PT_M_C_NFY_WARN_MESSAGE_STR:
		{
			Recv_PT_M_C_NFY_WARN_MESSAGE_STR(&rkPacket);
		}break;
	case PT_M_C_ANS_PICKUPGBOX:
		{
			Recv_PT_M_C_ANS_PICKUPGBOX(&rkPacket);
		}break;
#ifdef USE_GM
	case PT_A_C_ANS_GODCMD_FAILED:
		{
			GMCOMMAND::ProcessPacket( rkPacket );
		}break;
#endif
	case PT_M_C_ANS_STORE_ITEM_LIST:
		{
			Recv_PT_M_C_ANS_STORE_ITEM_LIST(&rkPacket);
		}break;
	case PT_M_C_NFY_SHINESTONE_MSG:
		{
			Recv_PT_M_C_NFY_SHINESTONE_MSG(&rkPacket);
		}break;
	case PT_M_C_NFY_CHANGE_COMBO_COUNT:
		{
			Recv_PT_M_C_NFY_CHANGE_COMBO_COUNT(&rkPacket);
		}break;
	case PT_M_C_NFY_CHANGE_MISSIONSCORE_COUNT:
		{
			Recv_PT_M_C_NFY_CHANGE_MISSIONSCORE_COUNT(&rkPacket);
		}break;
	case PT_M_C_MISSION_ABILITY_DEMAGE:
		{
			Revc_PT_M_C_MISSION_ABILITY_DEMAGE(&rkPacket);
		}break;
	case PT_M_C_MISSION_RANK_RESULT_ITEM:
		{
			Revc_PT_M_C_MISSION_RANK_RESULT_ITEM(&rkPacket);
		}break;
	// PvP관련->시작

	case PT_M_C_NFY_OBMODE_START:
		{
			BM::GUID kTargetID;
			rkPacket.Pop( kTargetID );
			g_kOBMode.Start( kTargetID );
			if( g_pkWorld )
			{
				if( g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE) )
				{
					g_kBattleSquareMng.RegistMemberTargetObserver();
				}
				if( g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP_ABLE) )
				{
					lua_tinker::call<void,char const*,int,float,int,int,int,int>("EventScriptText", "SFRM_EVENT_SCRIPT_TEXT", 200016, 0.f, 52, 80, 0, 0);
				}
			}
		}break;
	case PT_M_C_NFY_OBMODE_END:
		{
			g_kOBMode.End();
			if( g_pkWorld )
			{
				if( g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP_ABLE) )
				{
					XUIMgr.Close(L"SFRM_EVENT_SCRIPT_TEXT");
				}
			}
		}break;
	case PT_M_C_NFY_OBMODE_TARGET_CHANGE:
		{
			BM::GUID kTargetID;
			bool bIsDisconnect = false;

			rkPacket.Pop( kTargetID );
			rkPacket.Pop( bIsDisconnect );

			if ( true == bIsDisconnect )
			{
				g_kOBMode.RemoveTarget( kTargetID );
			}
			else
			{
				g_kOBMode.Start( kTargetID );
			}
		}break;
	case PT_M_C_ANS_OBMODE_TARGET_FAILED:
		{
			BM::GUID kTargetID;
			rkPacket.Pop( kTargetID );
			g_kOBMode.Failed( kTargetID );
		}break;

	case PT_M_C_ANS_PVP_SELECTOR:
		{
			XUI::CXUI_Wnd * pkWndInvite = XUIMgr.Get( L"SFRM_INVITE_LEAGUE" );
			XUI::CXUI_Wnd * pkWndJoin = XUIMgr.Get( L"SFRM_JOIN_LEAGUE_LOBBY" );
			if ( pkWndInvite || pkWndJoin )
			{
				if( pkWndInvite )
				{
					DirectJoin_PvPLeagueLobby(pkWndInvite);
				}
				else
				{
					DirectJoin_PvPLeagueLobby(pkWndJoin);
				}
			}
			else
			{
				XUI::CXUI_Wnd * pkWnd = XUIMgr.Activate( L"FRM_PVP_SELECTOR", true );
				if ( pkWnd )
				{
					__int64 const i64NowTimeInDay = g_kEventView.GetLocalSecTimeInDay( CGameTime::DEFAULT );

					Set_PvPSelectorUI( pkWnd->GetControl(L"BTN_EXERCISE_MODE"), i64NowTimeInDay, PvP_Lobby_GroundNo_Exercise );
					Set_PvPSelectorUI( pkWnd->GetControl(L"BTN_RANK_MODE"), i64NowTimeInDay, PvP_Lobby_GroundNo_Ranking );
					Set_PvPSelectorUI( pkWnd->GetControl(L"BTN_PVPLEAGUE_MODE"), i64NowTimeInDay, PvP_Lobby_GroundNo_League );
					//Set_PvPSelectorUI( pkWnd->GetControl(L"BTN_HARDCORE_DUNGEON_MODE"), i64NowTimeInDay, HardCoreDungeon );

					XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(L"BTN_BATTLESQUARE_MODE");
					if ( pkTemp )
					{
						int iGameIDX = 0;
						bool const bCanEnter = g_kBattleSquareMng.GetCanEnterGameIDX(iGameIDX);
						pkTemp->Enable( bCanEnter );
						pkTemp->GrayScale( !bCanEnter );
						if( bCanEnter )
						{
							DWORD const dwTwinkleTime = 180000;	//3분간 깜빡거리도록
							DWORD const dwTwinkleInter = 100;
							pkTemp->SetTwinkle(true);
							pkTemp->TwinkleTime(dwTwinkleTime);
							pkTemp->TwinkleInterTime(dwTwinkleInter);
						}

						pkTemp->ClearCustomData();

						pkTemp = pkTemp->GetControl( L"FRM_TIME" );
						if ( pkTemp )
						{
							pkTemp->Text( TTW(bCanEnter ? 460077 : 71061) );
						}
					}
				}
			}
		}break;
	case PT_T_C_ANS_JOIN_LOBBY:
	case PT_T_C_ANS_JOIN_LEAGUE_LOBBY:
		{
			if ( !PgContentsBase::ms_pkContents || (PgContentsBase::ms_pkContents->GetType() != PVP_TYPE_ALL) )
			{
				SAFE_DELETE(PgContentsBase::ms_pkContents);
				PgPvPGame* pkPvPGame = new PgPvPGame;// PgContentsBase생성자에서 ms_pkContents를 셋팅한다.
			}
			
			if ( PgContentsBase::ms_pkContents )
			{
				PgContentsBase::ms_pkContents->ProcessPacket( wPacketType, rkPacket );
			}
		}break;
	case PT_M_C_NFY_GAME_WAIT://
	case PT_M_C_NFY_GAME_READY:
	case PT_M_C_NFY_GAME_START:
		{
			EPVPTYPE kType = PVP_TYPE_NONE;
			rkPacket.Pop( kType );

			if ( !PgContentsBase::ms_pkContents || 0 == (PgContentsBase::ms_pkContents->GetType() & kType) )
			{
				SAFE_DELETE(PgContentsBase::ms_pkContents);
				switch ( kType )
				{
				case PVP_TYPE_DM:
				case PVP_TYPE_KTH:
				case PVP_TYPE_ANNIHILATION:
				case PVP_TYPE_LOVE:
				case PVP_TYPE_DESTROY:
					{
						new PgPvPGame;// PgContentsBase생성자에서 ms_pkContents를 셋팅한다.
					}break;
				case WAR_TYPE_DESTROYCORE:
					{
						new PgEmporiaBattleClient;// PgContentsBase생성자에서 ms_pkContents를 셋팅한다.
					}break;
				case WAR_TYPE_PROTECTDRAGON:
					{
						new PgEmporiaBattleClient_ProtectDragon;// PgContentsBase생성자에서 ms_pkContents를 셋팅한다.
					}break;
				}
			}	
		}// break;사용안함.
	case PT_T_C_ANS_JOIN_ROOM:
	case PT_M_C_NFY_GAME_INTRUDER:
	case PT_M_C_NFY_GAME_EXIT:
	case PT_M_C_NFY_GAME_EVENT_KILL://
	case PT_M_C_NFY_ADD_POINT_USER:
	case PT_M_C_NFY_GAME_END://
	case PT_M_C_NFY_GAME_RESULT_END:
	case PT_M_C_NFY_SYNC_GAMTTIME:
	case PT_T_C_NFY_ROOMLIST_REMOVE:
	case PT_T_C_NFY_ROOMLIST_UPDATE:
	case PT_T_C_NFY_EXIT_ROOM_USER:
	case PT_T_C_ANS_TEAM_CHANGE:
	case PT_T_C_NFY_USER_STATUS_CHANGE:
	case PT_M_C_NFY_GAME_RESULT:
	case PT_T_C_NFY_RELOAD_ROOM:
	case PT_T_C_ANS_MODIFY_ROOM:
	case PT_T_C_ANS_SLOTSTATUS_CHANGE:
	case PT_T_C_ANS_ENTRUST_MASTER:
	case PT_T_C_NFY_PVPLOBBY_LIST_EVENT:
	case PT_T_C_ANS_GET_PVPRANKING:
	case PT_T_C_NFY_PVPRANK:
	case PT_M_C_NFY_BATTLE_AREA_USER:
	case PT_M_C_NFY_BATTLE_CHANGE_AREA_TO_BIGDRAGON:
	case PT_T_C_ANS_INVATE_FAIL:
	case PT_T_C_NFY_REFRESH_LEAGUE_LOBBY:
	case PT_T_C_RELOAD_LEAGUE_LOBBY:
	case PT_T_C_ANS_ENTRY_CHANGE:
	case PT_M_C_NFY_OCCUPY_POINT_SECTION_INFO:
	case PT_M_C_NFY_KOH_INSERT_GUARDIAN:
	case PT_M_C_NFY_UPDATE_LOVE_FENCE:
	case PT_M_C_NFY_DISPLAY_DROPBEAR_TIMER:
	case PT_M_C_NFY_GAME_EVENT_CHANGEPOINT:
	case PT_M_C_NFY_FOCUS_TOUCH_DOWN_USER:
		{
			if ( PgContentsBase::ms_pkContents )
			{
				PgContentsBase::ms_pkContents->ProcessPacket(wPacketType,rkPacket);
			}
		}break;
	case PT_M_C_NFY_KTH_MESSAGE:
		{
			int MessageNo;
			bool bPlaySound;
			std::wstring Name;
			BM::vstring vText;
			
			rkPacket.Pop(MessageNo);
			rkPacket.Pop(bPlaySound);
			vText = TTW(MessageNo);

			if( rkPacket.RemainSize() )
			{
				rkPacket.Pop(Name);

				vText.Replace(L"%Name%", Name);
			}

			if( bPlaySound )
			{
				switch(MessageNo)
				{
				case 74100:
					{
					}break;
				case 74101:
					{
					}break;
				case 74102:
					{
					}break;
				case 74103:
					{
					}break;
				case 74104:
					{
					}break;
				case 74105:
				case 74106:
					{
						lua_tinker::call<void>("PlayWarnSoundKTH");
					}break;
				default:
					{
					}break;
				}
			}
			
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(kChatLog, vText, true, 1);
		}break;
	case PT_M_C_NFY_LOVEMODE_MESSAGE:
		{
			int MessageNo = 0;
			bool bMessageDelay = false;
			ETeam TeamNo = TEAM_NONE;
			std::wstring Name;

			rkPacket.Pop(MessageNo);
			rkPacket.Pop(bMessageDelay);
			rkPacket.Pop(TeamNo);
			rkPacket.Pop(Name);

			if( TeamNo == TEAM_BLUE )
			{
				MessageNo += 1;
			}

			BM::vstring OutText(TTW(MessageNo));
			if( !Name.empty() )
			{
				OutText.Replace(L"#NAME#", Name);
			}

			if( bMessageDelay )
			{
				g_kChatMgrClient.LoveModeMessageDelay(std::wstring(OutText), CT_ERROR_NOTICE);
			}
			else
			{
				g_kChatMgrClient.ShowNoticeUI(std::wstring(OutText), CT_ERROR_NOTICE);
			}
		}break;
	//<-- PvP
	
	//--> Emporia
	case PT_N_C_ANS_EMPORIA_STATUS_LIST:
	case PT_M_C_NFY_EMPORIA_FUNCTION:
	case PT_M_C_NFY_EMPORIA_FUNCTION_REMOVE:
//	case PT_M_C_ANS_EMPORIA_ADMINISTRATION:
		{
			g_kEmporiaMgr.ProcessPacket( wPacketType, rkPacket );
		}break;
	//<-- Emporia

	//--> HardCore Dungeon
	case PT_M_C_ANS_REGIST_HARDCORE_VOTE:
		{
			PgHardCoreDungeon::Recv_PT_M_C_ANS_REGIST_HARDCORE_VOTE( rkPacket );
		}break;
	case PT_M_C_NFY_HARDCORE_BOSS_ENDTIME:
		{
			BM::GUID kPartyGuid;
			__int64 i64EndTime = 0i64;
			rkPacket.Pop( kPartyGuid );
			rkPacket.Pop( i64EndTime );
			PgHardCoreDungeon::UpdateBossGndInfo( kPartyGuid, i64EndTime );
		}break;
	case PT_M_C_NFY_HARDCORE_BOSS_ENDTIME_IN:
		{
			__int64 i64EndTime = 0i64;
			rkPacket.Pop( i64EndTime );
			PgHardCoreDungeon::UpdateRemainTime( i64EndTime );
		}break;
	case PT_M_C_NFY_EMPORIA_PORTAL_INFO:
		{
			Recv_PT_M_C_NFY_EMPORIA_FUNCTION( g_pkWorld, rkPacket );
		}break;
	//<--

	//--> Alram Mission
	case PT_M_C_NFY_ALRAMMISSION_BEGIN:{PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_BEGIN>()(rkPacket);}break;
	case PT_M_C_NFY_ALRAMMISSION_END:{PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_END>()(rkPacket);}break;
	case PT_M_C_NFY_ALRAMMISSION_PARAM:{PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_PARAM>()(rkPacket);}break;
	case PT_M_C_NFY_ALRAMMISSION_SUCCESS:{PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_SUCCESS>()(rkPacket);}break;
	//<--

	//>>Quest
	case PT_M_C_ANS_NPC_QUEST:
		{
			Net_PT_M_C_ANS_NPC_QUEST(&rkPacket);
		}break;
	case PT_M_C_SHOWDIALOG:
		{
			Net_PT_M_C_SHOWDIALOG(&rkPacket);
		}break;
	case PT_M_C_ANS_STEPBYSTEP_QUEST:
		{
			int QuestNo = 0;
			int LastClearObjectNo = 0;

			rkPacket.Pop( QuestNo );
			rkPacket.Pop( LastClearObjectNo );

			PgQuestInfo const * QuestInfo = g_kQuestMan.GetQuest(QuestNo);
			if( NULL == QuestInfo )
			{
				return false;
			}
			
			BM::vstring LogMsg(TTW(798799));
			switch( LastClearObjectNo )
			{
			case 0:
				{
					LogMsg.Replace(L"#OBJECT#", TTW(QuestInfo->m_iObjectTextNo1));
				}break;
			case 1:
				{
					LogMsg.Replace(L"#OBJECT#", TTW(QuestInfo->m_iObjectTextNo2));
				}break;
			case 2:
				{
					LogMsg.Replace(L"#OBJECT#", TTW(QuestInfo->m_iObjectTextNo3));
				}break;
			case 3:
				{
					LogMsg.Replace(L"#OBJECT#", TTW(QuestInfo->m_iObjectTextNo4));
				}break;
			default:
				{
					return false;
				}break;
			}

			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, (std::wstring)LogMsg, true);
		}break;
	//<<Quest

	//>>Chat
	case PT_M_C_NFY_CHAT:
		{
			g_kChatMgrClient.RecvChat(&rkPacket);
		}break;
	case PT_T_C_NFY_BULLHORN:
		{
			SMS_HEADER kSMSHeader;
			kSMSHeader.ReadFromPacket(rkPacket);

			{	// 차단 여부 테스트.
				bool bBlocked = g_kChatMgrClient.CheckChatBlockCharacter(kSMSHeader.kSenderName, ECBT_CASHCHAT);
				if( bBlocked )
				{
					break;
				}
			}

			XUI::PgExtraDataPackInfo kExtraDataPackInfo;
			kExtraDataPackInfo.PopFromPacket(rkPacket);
			CallMegaPhoneMsgUI(kSMSHeader, kExtraDataPackInfo);
		}break;
	case PT_T_C_NFY_NOTICE:
		{
			std::wstring kContents;
			rkPacket.Pop(kContents);
			Notice_Show(kContents, EL_Warning);
		}break;
	case PT_N_C_NFY_NOTICE_PACKET:
		{
			size_t iSize = 0;
			rkPacket.Pop( iSize );

			while( iSize-- )
			{
				RecvNoticePacket(rkPacket);
			}
		}break;
	case PT_M_C_NFY_CHAT_INPUTNOW:
		{
			g_kChatMgrClient.RecvChat_InputNow(&rkPacket);
		}break;
	case PT_M_C_NFY_TRADE:
		{
			g_kChatMgrClient.RecvChat(&rkPacket);
		}break;
	//<<Chat
	//>>Friend
	case PT_N_C_ANS_MSN_FRIENDCOMMAND:
		{
			g_kFriendMgr.RecvFriend_Command(&rkPacket);
			lua_tinker::call<void>("Update_FriendList");
		}break;
	case PT_N_C_NFY_MSN_ELEMENT_MODIFY:
		{
			g_kFriendMgr.RecvFriend_Command(&rkPacket);
			lua_tinker::call<void>("Update_FriendList");
		}break;
	case PT_M_C_ANS_SEARCH_PEOPLE_LIST:
		{
			CONT_SEARCH_UNIT_INFO unit_itr;

			unit_itr.clear();
			PU::TLoadArray_M(rkPacket, unit_itr);

			char const *pszText = "SFRM_PROPLE_SEARCH";

			std::wstring const wstr = UNI(pszText);
			XUI::CXUI_Wnd* pWnd = NULL;

			if(!XUIMgr.IsActivate(wstr, pWnd))
			{
				pWnd = XUIMgr.Call(wstr, false);
			}
			PgClientPartyUtil::SearchPeopleUpdate(unit_itr);
		}break;
	//<<Friend
	//>>Guild
	case PT_N_C_ANS_GUILD_COMMAND:
		{
			g_kGuildMgr.ProcessPacket(&rkPacket);
		}break;
	case PT_N_C_NFY_GUILD_INV_EXTEND:
		{
			HRESULT hRet;
			__int64 iCause;
			EInvType kInvType;
			BYTE byExtendSize;

			rkPacket.Pop(hRet);
			switch(hRet)
			{
			case E_CANNOT_GUILD_INVENTORY_EXTEND_LINE:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403412, true);
					return false;
				}break;
			case E_CANNOT_GUILD_INVENTORY_EXTEND_TAB:
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403413, true);
					return false;
				}break;
			case E_CANNOT_EXTEND:// 더이상 확장 불가
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406109, true);
					return false;
				}break;
			default:
				{
				}break;
			}

			rkPacket.Pop(iCause);
			rkPacket.Pop(kInvType);
			rkPacket.Pop(byExtendSize);

			PgInventory* pkInv = g_kGuildMgr.GetInven();
			if( !pkInv )
			{
				return false;
			}

			switch( iCause )
			{
			case IMET_GUILD_INV_EXTEND_LINE:
				{
					if( false == pkInv->ExtendMaxIdx( kInvType, byExtendSize ) ) // byExtendSize 더함
					{
						return false;
					}
				}break;
			case IMET_GUILD_INV_EXTEND_TAB:
				{
					if( false == pkInv->InvExtend( kInvType, byExtendSize ) ) // byExtendSize 뺌
					{
						return false;
					}
				}break;
			default:
				{
				}break;
			}

			{// 길드금고가 확장 되었습니다.
				std::wstring wstrNotice(TTW(403411));
				SChatLog kChatLog(CT_GUILD);
				g_kChatMgrClient.AddLogMessage(kChatLog, wstrNotice);
			}			

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_GUILD_INVENTORY");
			if( !pkWnd )
			{
				return false;
			}

			lwSetGuildInvViewSize(static_cast<BYTE>(kInvType)); // 인벤 사이즈 다시 계산			
		}break;
	//<<Guild
/*
	case PT_M_C_NFY_CLASS_CHANGE://전직
		{
			BM::GUID kCharGuid;
			BYTE cClassNo = 0;
			short sLevel = 0;
			
			rkPacket.Pop(kCharGuid);
			rkPacket.Pop(cClassNo);

			PgPilot* pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if( pkPilot )
			{
				PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
				if( pkPC )
				{
					//어빌 변경은 Recv_PT_M_C_NFY_STATE_CHANGE2() 에서

					if( g_kPilotMan.IsMyPlayer(kCharGuid) )//내꺼
					{
						g_kSkillTree.CreateSkillTree( pkPC );//전직했으므로 스킬트리를 재구성하자.

						lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");

						int const iClassTTW = 30100 + cClassNo;
						std::wstring const kClassName = TTW(iClassTTW);
						std::wstring kMessage = kClassName + TTW(700028);

						SChatLog kChatLog(CT_EVENT);
						g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true, 2);
					}

					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					if( pkActor )//전직 이펙트
					{
						NiAVObject *pkParticle = g_kParticleMan.GetParticle("ef_class_change", pkActor->GetEffectScale());
						if( pkParticle )
						{
							if(!pkActor->AttachTo(8382, "char_root", (NiAVObject *)pkParticle))
							{
								THREAD_DELETE_PARTICLE(pkParticle);
							}
						}
					}
				}
			}
		}break;
	case PT_M_C_NFY_LV_CHANGE://레벨업
		{
			BM::GUID kCharGuid;
			int iNewLevel = 0;

			rkPacket.Pop(kCharGuid);
			rkPacket.Pop(iNewLevel);

			PgPilot* pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if( pkPilot )
			{
				PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
				if( pkPC )
				{
					//어빌 변경은 Recv_PT_M_C_NFY_STATE_CHANGE2() 에서

					if( g_kPilotMan.IsMyPlayer(kCharGuid) )
					{
						lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
						lwSetReqInvWearableUpdate(true);
						lua_tinker::call<void>("SetUpdateLevelUp");
						if( !lua_tinker::call<bool>("UpdateLevelUI") )
						{
							lua_tinker::call<void>("SetUpdateLevelUp");
						}

						g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_LEVEL_UP), pkPC->GetAbil(AT_LEVEL));
					}
				}

				PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if( pkActor )//레벨업 이펙트
				{
					NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_level_up", pkActor->GetEffectScale());
					if( pkParticle )
					{
						if(!pkActor->AttachTo(8382, "char_root", pkParticle))
						{
							THREAD_DELETE_PARTICLE(pkParticle);
						}
					}

					g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Com_lvup", 0.0f, 0.0f, 0.0f, pkActor);
				}
			}
		}break;
*/
	case PT_N_C_NFY_MISSION_RESULT:
		{
			g_kMissionComplete.ReadFromPacket(rkPacket);
			g_kMissionComplete.BossMission(false);
		}break;
	case PT_N_C_ANS_RANK_TOP:
		{
			int iPoint = 0;
			std::wstring kName;

			rkPacket.Pop(iPoint);
			rkPacket.Pop(kName);

			BM::vstring vstrText(TTW(401192)); 
			vstrText.Replace( L"#NUM#", iPoint );
			vstrText.Replace( L"#NAME#", kName );

			lua_tinker::call<void, char const*>("CallMissionRankingRewardItem", MB(vstrText));			
		}break;
	case PT_N_C_NFY_MISSION_CLOSETIMER:
		{
			g_kMissionComplete.CloseMissionTimer(rkPacket);
		}break;
	case PT_M_C_NFY_MISSION_BONUS_TIME_CLOSE:
		{
			g_kMissionComplete.CloseBonusStageMissionTimer(rkPacket);
		}break;
	case PT_N_C_NFY_MISSION_CLOSESCORE:
		{
			g_kMissionComplete.CloseMissionScore(rkPacket);
		}break;
	case PT_M_C_NFY_SELECTED_BOX:
		{
			g_kBossComplete.ReadFromPacketSelect(rkPacket);
		}break;
	case PT_M_C_NFY_OPEN_BOX:
		{
			g_kBossComplete.ReadFromPacketOpen(rkPacket);
		}break;
	case PT_C_M_ANS_ITEM_PLUS_UPGRADE:
		{
			Recv_PT_C_M_ANS_ITEM_PLUS_UPGRADE(&rkPacket);
		}break;
	case PT_M_C_ANS_ROLLBACK_ENCHANT:
		{
			lwLuckyChanger::ReceivePacket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_ITEM_RARITY_UPGRADE:
		{
			Recv_PT_C_M_ANS_ITEM_RARITY_UPGRADE(&rkPacket);
		}break;
	case PT_M_C_ANS_GEN_SOCKET:
		{
			Recv_PT_M_C_ANS_GEN_SOCKET(&rkPacket);
		}break;
	case PT_M_C_ANS_REMOVE_MONSTERCARD:
		{
			Recv_PT_M_C_ANS_REMOVE_MONSTERCARD(&rkPacket);
		}break;
	case PT_M_C_ANS_RESET_MONSTERCARD:
		{
			Recv_PT_M_C_ANS_RESET_MONSTERCARD(&rkPacket);
		}break;
	case PT_M_C_ANS_EXTRACTION_MONSTERCARD:
		{
			Recv_PT_M_C_ANS_EXTRACTION_MONSTERCARD(&rkPacket);
		}break;
	case PT_M_C_ANS_SET_MONSTERCARD:
		{
			Recv_PT_M_C_ANS_SET_MONSTERCARD(&rkPacket);
		}break;
	case PT_M_C_ANS_CONVERTITEM:
		{
			Recv_PT_M_C_ANS_CONVERTITEM(&rkPacket);
		}break;
	case PT_M_C_ANS_OPEN_LOCKED_CHEST:
		{
			Recv_PT_M_C_ANS_OPEN_LOCKED_CHEST(&rkPacket);
		}break;
	case PT_M_C_ANS_INDUN_START:
		{
			if (g_pkWorld != NULL && !g_pkWorld->GetStartScript().empty() )
			{
				PgEnergyGauge::ms_bDrawEnergyGaugeBar = (0 != g_kGlobalOption.GetValue( "ETC", "BATTLEOP_DISPLAY_HPBAR" ));
				lua_tinker::call<void, lwWorld>(g_pkWorld->GetStartScript().c_str(), lwWorld(g_pkWorld) );
			}
		}break;
	case PT_N_C_NFY_BOSS_RESULT:
		{
			if ( g_pkWorld || (g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
			{
				bool bIsSuccess = false;
				rkPacket.Pop( bIsSuccess );

				if ( true == bIsSuccess )
				{
					g_kBossComplete.ReadFromPacket(rkPacket);
					g_kBossComplete.BossMission(true);
				}
				else
				{
					Net_RecentMapMove();
				}
			}
		}break;
	case PT_GM_C_ANS_RECEIPT_PETITION:
		{
			int iReceiptIndex = 0;
			rkPacket.Pop(iReceiptIndex);
			g_kPetitionMgr.ReceiptPetition_Success(iReceiptIndex);
		}break;
	case PT_GM_C_ANS_WAIT_RECEIPT:
		{
			g_kPetitionMgr.WaitReceipt_Petition();
		}break;
	case PT_GM_C_ANS_REMAINDER_PETITION:
		{
			int icount = 0;
			rkPacket.Pop(icount);
			g_kPetitionMgr.PetitionDlg_Show(icount);
		}break;
// 	case PT_M_C_ANS_GBOXINFO:
// 		{
// 			Recv_PT_M_C_ANS_GBOXINFO(&rkPacket);
// 		}break;
	case PT_M_C_NFY_EXCHANGE_ITEM_ITEM:
		{
			g_kTradeMgr.Recv_PT_M_C_NFY_EXCHANGE_ITEM_ITEM(&rkPacket);
		}break;
	case PT_M_C_NFY_EXCHANGE_ITEM_RESULT:
		{
			g_kTradeMgr.Recv_PT_M_C_NFY_EXCHANGE_ITEM_RESULT(&rkPacket);
		}break;
	case PT_M_C_NFY_EXCHANGE_ITEM_READY:
		{
			g_kTradeMgr.Recv_PT_M_C_NFY_EXCHANGE_ITEM_READY(&rkPacket);
		}break;
	case PT_T_C_NFY_OPTION:
		{
			g_kGlobalOption.ReadFromPacket(rkPacket);
		}break;
	case PT_M_C_ANS_SAVE_SKILLSET:
		{
			lwSkillSet::lwChangeSaveState(false);
			g_kChatMgrClient.ShowNoticeUI(TTW(401611), 1, true, true);			
		}break;
	case PT_M_C_ANS_CLIENT_CUSTOMDATA:
		{
			EClientCustomDataType eType = ECCDT_NONE;

			rkPacket.Pop(eType);

			switch(eType)
			{
			case ECCDT_SKILLTABNO:
				{
				}break;
			}
		}break;
	case PT_M_C_NFY_MISSION_DATA:
		{
			PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if ( pkMyPlayer )
			{
				unsigned int iMissionKey = 0;
				rkPacket.Pop( iMissionKey );

				PgPlayer_MissionData *pkMissionData = pkMyPlayer->GetMissionData( iMissionKey );
				if ( pkMissionData )
				{
					pkMissionData->ReadFromPacket( rkPacket );
				}
			}
		}break;
	case PT_M_C_NFY_MISSION_UNLOCKINPUT:
		{
			g_kMissionComplete.MissionEndUnlockInput();
		}break;
	case PT_M_C_ANS_VIEW_OTHER_EQUIP:
		{//
			g_kOtherViewInv.Clear();
			g_kOtherViewInv.ReadFromPacket(IT_FIT, rkPacket);
			g_kOtherViewInv.ReadFromPacket(IT_FIT_CASH, rkPacket);
		}break;
	case PT_M_C_NFY_ITEM_MAKING_RESULT:
		{
			g_kItemMaking.Recv_PT_M_C_NFY_ITEM_MAKING_RESULT(&rkPacket);
		}break;
	case PT_M_C_SKILL_STATE_CHANGE:
		{
			BM::GUID kGuid;
			rkPacket.Pop(kGuid);
			PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
			if(pkPilot)
			{
				PgPlayer* pkPlayer = reinterpret_cast<PgPlayer*>(pkPilot->GetUnit());
				if(pkPlayer)
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					GET_DEF(CSkillDefMgr, kSkillDefMgr);

					PgMySkill* pkMySkill = pkPlayer->GetMySkill();
					
					// 현재 배운 토글 스킬들을 돌면서 현재 토글 상태인 스킬들을 해제 한다.
					size_t szIndex = 0;
					int iSkillNo = 0;
					while ((iSkillNo = pkMySkill->GetSkillNo(EST_TOGGLE, szIndex++)) > 0)
					{
						if(pkActor)
						{
							pkActor->ActionToggleStateChange(iSkillNo, false);							
							CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
							if(pkSkillDef)
							{
								int const iEffectID = pkSkillDef->GetEffectNo();
								if(0 < iEffectID)
								{
									g_kStatusEffectMan.RemoveStatusEffect(pkPilot, iEffectID);
									g_kStatusEffectMan.RemoveStatusEffectAfterAction(pkPilot, iEffectID);
								}								
							}
						}
					}
					
					pkMySkill->ReadFromPacket(rkPacket);
					
					if( g_kPilotMan.IsMyPlayer(kGuid) )
					{
						g_kSkillTree.CreateSkillTree( pkPlayer );//전직했으므로 스킬트리를 재구성하자.

						lua_tinker::call<void>("UI_SKILLWND_Refresh_All_New");
						lwUIWnd kQuick = lwGetUIWnd("QuickInv");	//새로 그려주자
						if( !kQuick.IsNil() )
						{
							kQuick.SetInvalidate();
						}

						//스킬트리가 초기화 됬으므로 펫 스킬도 다시 넣어줘야 한다.
						PgPetUIUtil::PetSkillToSkillTree(pkPlayer);
					}
				}
			}
		}break;
	case PT_M_C_ANS_MISSION_RESTART_FAILED:
		{
			VEC_GUID kVec;
			rkPacket.Pop(kVec);
			if (!kVec.empty())
			{
				BM::vstring kString(TTW(80020));
				VEC_GUID::const_iterator guid_it = kVec.begin();
				while (kVec.end() != guid_it)
				{
					PgPilot *pkPilot = g_kPilotMan.FindPilot((*guid_it));
					if (pkPilot)
					{
						kString+=pkPilot->GetName();
						kString+=_T(" ");
					}
					++guid_it;
				}

				Notice_Show( (std::wstring)kString, EL_Normal );
			}

			XUIMgr.Call(std::wstring(_T("SFRM_RETRY_MISSION")));
		}break;
	case PT_M_C_ANS_RESTART_MISSION_INFO_UI:
		{
			g_kMissionComplete.CallRetryUI();
		}break;
	case PT_M_C_REQ_MISSION_ROULETTE_STOP:
		{
			BM::GUID kGuid;
			rkPacket.Pop(kGuid);

			g_kMissionComplete.SetRoluetteStopRewardItemDraw(kGuid);
		}break;
	case PT_M_C_NFY_COOLTIME_INIT:
		{
			// Skill CoolTime 이 초기화 되었다 (Player죽었을 때 보냄)
			BM::GUID kUnitGuid;
			rkPacket.Pop(kUnitGuid);
			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kUnitGuid);
			if (pkPilot)
			{
				CUnit* pkUnit = pkPilot->GetUnit();
				if (pkUnit != NULL)
				{
					pkUnit->GetSkill()->ClearCoolTime();
				}
				PgActor	*pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
				if (pkActor)
				{
					pkActor->ResetSkillCoolTimeFromUnit();
				}
			}
		}break;
	//case PT_M_C_ANS_MISSION_QUEST://미션 퀘스트 아이템 공지
		//{
		//	BYTE cCmdType = 0;
		//	rkPacket.Pop(cCmdType);

		//	switch(cCmdType)
		//	{
		//	case MQC_Nfy_CardItem://퀘스트카드아이템 들어오는곳
		//		{
		//			//SMissionObject m_kObjectData;
		//			int iModeType = 0;
		//			ContHaveItemNoCount kCardItemMap;
		//			rkPacket.Pop(kCardItemMap);
		//			rkPacket.Pop(iModeType);
		//			//rkPacket.Pop(m_kObjectData);
		//			
		//			bool const bRet = kCardItemMap.empty();
		//			if( !bRet )
		//			{
		//				g_kQuestMan.MQCardList_Insert(kCardItemMap);

		//				XUI::CXUI_Wnd* pCardWnd = XUIMgr.Call(std::wstring(_T("FRM_MISSION_QUEST")));
		//				MissionQuestUISetting();
		//			}
		//			else
		//			{
		//				if( iModeType == MO_ITEM )
		//				{
		//					lwCallUI("FRM_MISSION_START");

		//					// 미션 아이템 모드 알림 메시지 출력 부분
		//					/*for(int i=0;i<MISSION_PARAMNUM; i++)
		//					{
		//						if( m_kObjectData.iObjectText[i] )
		//						{
		//							lwAddWarnDataTT(m_kObjectData.iObjectText[i]);
		//						}								
		//					}*/
		//				}
		//			}
		//		}break;
		//	case MQC_Ans_SelectCard://카드 선택에 대한 응답
		//		{
		//			int iResult = MQCR_Failed;

		//			rkPacket.Pop(iResult);

		//			switch(iResult)
		//			{
		//			case MQCR_Success:
		//				{
		//					BM::GUID kCharGuid;
		//					SMissionQuest kMissionQuest;

		//					rkPacket.Pop(kCharGuid);//누가
		//					kMissionQuest.ReadFromPacket(rkPacket);//어떤 종류의 퀘스트를

		//					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		//					if( pkPlayer )
		//					{
		//						if( kCharGuid != pkPlayer->GetID() )
		//						{
		//							//	내가 선택한게 아니면 파티퀘만 보여줘
		//							if( MQT_PartyShare == kMissionQuest.eType )
		//							{
		//								//lwMQCard_QuestInfo(kMissionQuest.iQuestID);
		//							}
		//						}
		//						else
		//						{
		//							//	내가 선택한거면 다 보여줘
		//							lwMQCard_QuestInfo(kMissionQuest.iQuestID);
		//						}
		//					}
		//				}
		//			case MQCR_NoQuest://퀘스트가 없다
		//			case MQCR_Failed://실패
		//				{
		//					//선택한 카드를 사용하지 못했다
		//					XUI::CXUI_Wnd* pScaleWnd = XUIMgr.Get(L"FRM_MISSION_QUEST_SCALE");
		//					if( pScaleWnd )
		//					{
		//						pScaleWnd->Close();
		//					}
		//				}break;
		//			default:
		//				{
		//					PG_ASSERT_LOG(false && "Unknown ret code");
		//				}break;
		//			}
		//		}break;
		//	case MQC_Ans_StartQuest://
		//		{
		//		}break;
		//	case MQC_Ans_QuestResult:
		//		{
		//		}break;
		//	default:
		//		{
		//			PG_ASSERT_LOG(false && "Wrong Mission Quest command type");
		//		}break;
		//	}
		//}break;
	case PT_N_C_ANS_COUPLE_COMMAND:
		{
			g_kCoupleMgr.ProcessPacket(rkPacket);
		}break;
	case PT_M_C_ANS_MARRY_COMMAND:
		{
			g_kCoupleMgr.ProcessMarryPacket(rkPacket);
		}break;
	case PT_M_C_POST_NOTI_NEW_MAIL:
		{
			g_kMailMgr.RecvMail_Command(wPacketType, rkPacket);
			Recv_NOTI_NEW_MAIL();
			Update_NOTI_NEW_MAIL();
		}break;
	case PT_M_C_POST_ANS_MAIL_SEND:
	case PT_M_C_POST_ANS_MAIL_RECV:
	case PT_M_C_POST_ANS_MAIL_MODIFY:
	case PT_M_C_POST_ANS_MAIL_MIN:
		{
			g_kMailMgr.RecvMail_Command(wPacketType, rkPacket);
		}break;
	//경매
	case PT_M_C_UM_ANS_MARKET_OPEN:
	case PT_M_C_UM_ANS_MY_MARKET_QUERY:
	case PT_M_C_UM_ANS_ARTICLE_REG:
	case PT_M_C_UM_ANS_ARTICLE_DEREG:
	case PT_M_C_UM_ANS_MINIMUM_COST_QUERY:
	case PT_M_C_UM_ANS_DEALINGS_READ:
	case PT_M_C_UM_ANS_MARKET_QUERY:
	case PT_M_C_UM_ANS_MARKET_ARTICLE_QUERY:
	case PT_M_C_UM_ANS_ARTICLE_BUY:
	case PT_M_C_UM_ANS_MARKET_MODIFY_STATE:
	case PT_M_C_UM_ANS_USE_MARKET_MODIFY_ITEM:
	case PT_M_C_UM_ANS_MARKET_CLOSE:
	case PT_M_C_UM_ANS_BEST_MARKET_LIST:
	case PT_M_C_UM_ANS_MY_MARKET_DEALING_QUERY:
	case PT_M_C_UM_ANS_MARKET_ENTER:
		{
			lwMarket::RecvMarket_Command(wPacketType, rkPacket);
			lwVendor::RecvMarket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_UM_ANS_MY_VENDOR_QUERY:
	case PT_M_C_ANS_VENDOR_STATE:
	case PT_M_C_NFY_VENDOR_STATE:
	case PT_M_C_UM_ANS_VENDOR_ENTER:
	case PT_M_C_UM_NFY_VENDOR_REFRESH_QUERY:
	case PT_M_C_UM_ANS_VENDOR_EXIT:
		{
			lwVendor::RecvMarket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_ITEM_DISCHARGE:
	case PT_M_C_ANS_SEAL_ITEM:
		{// 아이템 봉인 및 봉인해제 결과 받음
			g_kSealProcess.RcevPacket(wPacketType, rkPacket);
		}break;
	case PT_N_C_NOTY_GEN_SYSTEM_INVENTORY:
		{
			g_kSystemInven.RecvSystemInven(rkPacket);

			if( g_pkWorld
				&& (!g_pkWorld->IsHaveAttr(GATTR_INSTANCE|GATTR_FLAG_BATTLESQUARE) ||	g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME|GATTR_FLAG_EMPORIA))
				)
			{
				g_kSystemInven.CheckSystemInventory();
			}
		}break;
	case PT_M_C_ANS_SYSTEM_INVENTORY_RECV:
		{
			g_kSystemInven.RecvModifySystemInven(rkPacket);
		}break;
	case PT_M_C_ANS_SYSTEM_INVENTORY_REMOVE:
		{
			g_kSystemInven.RecvRemove(rkPacket);
		}break;
	case PT_M_C_ANS_REWORD_FRAN_EXP:
		{
			__int64 iExp = 0;
			rkPacket.Pop(iExp);

			std::wstring kLog;
			bool const bRet = FormatTTW(kLog, 550009, iExp);
			if( !bRet )
			{
				break;
			}

			lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(kLog), true);
		}break;
	case PT_M_C_ANS_MSGBOX_CALL:
		{
			DWORD m_iItemNo = 0;
			int iTTW = 0;

			rkPacket.Pop(m_iItemNo);
			rkPacket.Pop(iTTW);

			if( iTTW )
			{
				std::wstring const kMessage = TTW(iTTW);
				BM::Stream kPacket( PT_C_M_REQ_MSGBOX_CALL, false );
				kPacket.Push(m_iItemNo);
				lua_tinker::call<void, char const*, lwPacket, bool >("CommonMsgBoxCancel", MB(kMessage), lwPacket(&kPacket), true);				
			}
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_StatusEffect1, false));
		}break;
	case PT_M_C_ANS_MSGBOX_CALL_RESULT:
		{		
		}break;
	case PT_M_C_NFY_CHANGE_EXPERIENCE:
		{
			// 0 경험치 알리는 용도
			PgActor *pkActor = g_kPilotMan.GetPlayerActor();
			if( pkActor )
			{
				pkActor->ShowExpNum(0);
				g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_CHECK_LV), 0);
			}
		}break;
	case PT_M_C_REQ_PLAYERTIME_DEFEND:
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer && pkPlayer->IsUse() )
			{
				EWallowDefendMode eDefendMode = WDM_NONE;
				int iExp = 0;
				int iMoney = 0;
				int iDrop = 0;
				EPPTCondition eEtcBoolean = PPTC_NONE;

				rkPacket.Pop(eDefendMode);
				rkPacket.Pop(iExp);
				rkPacket.Pop(iMoney);
				rkPacket.Pop(iDrop);
				rkPacket.Pop(eEtcBoolean);


				int iOldExp = pkPlayer->GetExpRate();
				int iOldMoney = pkPlayer->GetMoneyRate();
				int iOldDrop = pkPlayer->GetDropRate();
				EPPTCondition eOldEtc = pkPlayer->GetEtcBoolean();				

				pkPlayer->SetDefendMode(eDefendMode);
				pkPlayer->SetExpRate(iExp);
				pkPlayer->SetMoneyRate(iMoney);
				pkPlayer->SetDropRate(iDrop);
				pkPlayer->SetEtcBoolean(eEtcBoolean);

				if(iOldExp != iExp)
				{
					BM::vstring vstrText(TTW(88)); 
					vstrText.Replace( L"#Befor#", iOldExp );
					vstrText.Replace( L"#After#", iExp );
					vstrText.Replace( L"#Type#", TTW(180) );
					Notice_Show( vstrText, EL_Warning );
				}
				if(iOldMoney != iMoney)
				{
					BM::vstring vstrText(TTW(88)); 
					vstrText.Replace( L"#Befor#", iOldMoney );
					vstrText.Replace( L"#After#", iMoney );
					vstrText.Replace( L"#Type#", TTW(200) );
					Notice_Show( vstrText, EL_Warning );
				}
				if(iOldDrop != iDrop)
				{
					BM::vstring vstrText(TTW(799015));
					vstrText.Replace( L"#Befor#", iOldDrop );
					vstrText.Replace( L"#After#", iDrop );
					Notice_Show( vstrText, EL_Warning );
				}
				
				EPPTCondition eOldItem = eOldEtc & PPTC_NOT_TAKEITEM;
				EPPTCondition eNowItem = eEtcBoolean & PPTC_NOT_TAKEITEM;
				if(eOldItem != eNowItem)
				{
					BM::vstring vstrText(TTW(89)); 
					vstrText.Replace( L"#State#", (!eNowItem ? TTW(2220):TTW(2221)) );
					Notice_Show( vstrText, EL_Warning );
				}

				EPPTCondition eOldQuest = eOldEtc & PPTC_NOT_QUESTFINISH;
				EPPTCondition eNowQuest = eEtcBoolean & PPTC_NOT_QUESTFINISH;
				if(eOldQuest != eNowQuest)
				{
					Notice_Show( !eNowQuest ? TTW(799013):TTW(799014), EL_Warning );
				}
			}
		}break;
	case PT_SYNC_DEF_PLAYERPLAYTIME:
		{
			g_kDefPlayTime.ReadFromPacket(rkPacket);
			g_klwPlayTime.SetNextWarnSec(0);
		}break;
	case PT_M_C_UPDATE_PLAYERPLAYTIME:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				int iAccConSec = 0;
				int iAccDicSec = 0;
				__int64 i64SelectSec = 0i64;
				rkPacket.Pop(iAccConSec);
				rkPacket.Pop(iAccDicSec);
				rkPacket.Pop(i64SelectSec);

				pkPlayer->SetPlayTime(iAccConSec, iAccDicSec);
				pkPlayer->SetSelectCharacterSec(i64SelectSec);
				g_klwPlayTime.SetNextWarnSec(0);
			}
		}break;
	case PT_M_C_CS_ANS_BUY_ARTICLE:
	case PT_M_C_CS_ANS_SEL_ARTICLE:
	case PT_M_C_CS_ANS_SEND_GIFT:
	case PT_M_C_CS_ANS_RECV_GIFT:
	case PT_M_C_CS_ANS_ENTER_CASHSHOP:
	case PT_M_C_CS_ANS_MODIFY_VISABLE_RANK:
	case PT_M_C_CS_ANS_ADD_TIMELIMIT:
	case PT_M_C_CS_ANS_SIMPLE_ENTER_CASHSHOP:
	case PT_M_C_CS_ANS_EXIT_CASHSHOP_UNLOCK:
		{	
			lwCashShop::RecvCashShop_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_ACHIEVEMENT_TO_ITEM:
		{
			SPT_M_C_ANS_ACHIEVEMENT_TO_ITEM kStruct;
			kStruct.ReadFromPacket(rkPacket);
			
			if(kStruct.iError)	//실패
			{
			}
			else	//성공
			{
				lwUIBook::UpdateAchieveItem(kStruct.iSaveIdx, false);
			}

			::Notice_Show_ByTextTableNo(3070+kStruct.iError, EL_Normal);
		}break;
	case PT_M_C_ANS_MOVETOPARTYMEMBER:
		{
			SPT_M_C_ANS_MOVETOPARTYMEMBER kStruct;
			kStruct.ReadFromPacket(rkPacket);

			switch( kStruct.kResult )
			{
			case S_OK:
				{
				}break;
			case E_INCORRECT_ITEM:
				{					
					g_kChatMgrClient.LogMsgBox( 401304 );
				}break;
			case E_NOT_JOINED_PARTY:
				{					
					g_kChatMgrClient.LogMsgBox( 401305 );
				}break;
			case E_NOT_FOUND_MEMBER:
				{
					g_kChatMgrClient.LogMsgBox( 401306 );
				}break;
			case E_INVALID_ITEMPOS:
				{
					g_kChatMgrClient.LogMsgBox( 401307 );
				}break;
			case E_CANNOT_MOVE_MAP:
				{
					g_kChatMgrClient.LogMsgBox( 401312 );
				}break;
			case E_CANNOT_STATE_MAPMOVE: 
				{ 
					g_kChatMgrClient.LogMsgBox( 10410 );
				}break;
			default:
				{
					g_kChatMgrClient.LogMsgBox( 401303 );
				}break;
			}
		}break;
	case PT_M_C_ANS_MOVETOPARTYMASTERGROUND:
		{
			SPT_M_C_ANS_MOVETOPARTYMASTERGROUND kStruct;
			kStruct.ReadFromPacket(rkPacket);

			switch( kStruct.kResult )
			{
			case S_OK:
				{
				}break;
			case E_INCORRECT_ITEM:
				{					
					g_kChatMgrClient.LogMsgBox( 401304 );
				}break;
			case E_NOT_JOINED_PARTY:
				{					
					g_kChatMgrClient.LogMsgBox( 401305 );
				}break;
			case E_NOT_FOUND_MEMBER:
				{
					g_kChatMgrClient.LogMsgBox( 401306 );
				}break;
			case E_INVALID_ITEMPOS:
				{
					g_kChatMgrClient.LogMsgBox( 401307 );
				}break;
			case E_CANNOT_MOVE_MAP:
				{
					g_kChatMgrClient.LogMsgBox( 401312 );
				}break;
			case E_CANNOT_STATE_MAPMOVE: 
				{ 
					g_kChatMgrClient.LogMsgBox( 10410 );
				}break;
			default:
				{
					g_kChatMgrClient.LogMsgBox( 401303 );
				}break;
			}
		}break;
	case PT_M_C_REQ_SUMMONPARTYMEMBER:
		{
			SPT_M_C_REQ_SUMMONPARTYMEMBER kStruct;
			kStruct.ReadFromPacket(rkPacket);

			std::wstring wstrText = std::wstring();

			SPartyMember const* pkMyMemberInfo = NULL;
			if( g_kParty.GetMember(kStruct.kCharGuid, pkMyMemberInfo) )
			{
				if( !pkMyMemberInfo->kName.empty() )
				{
					wchar_t const* pName = pkMyMemberInfo->kName.c_str();
					WstringFormat( wstrText, MAX_PATH, TTW( 401311 ).c_str(), pName, 15 );

					BM::Stream kPacket( PT_C_M_REQ_MOVETOSUMMONER );
					kPacket.Push(true);
					kPacket.Push(kStruct.kCharGuid);
					kPacket.Push(kStruct.kGndKey);
					kPacket.Push(kStruct.kItemPos);
					BM::Stream kCancelPacket( PT_C_M_REQ_MOVETOSUMMONER );
					kCancelPacket.Push(false);
					kCancelPacket.Push(kStruct.kCharGuid);
					kCancelPacket.Push(kStruct.kGndKey);
					kCancelPacket.Push(kStruct.kItemPos);
					lua_tinker::call<bool, char const*, char const*, lwPacket, lwPacket>("CommonNetSummonPartyMemberBox", MB(wstrText), MB(pName), lwPacket(&kPacket), lwPacket(&kCancelPacket) );//메시지
				}
				else
				{
					SPT_C_M_REQ_MOVETOSUMMONER kFailStruct;
					kFailStruct.bYesNo = false;
					kFailStruct.kCharGuid = kStruct.kCharGuid;
					kFailStruct.kGndKey = kStruct.kGndKey;
					kFailStruct.kItemPos = kStruct.kItemPos;

					BM::Stream kPacket;
					kFailStruct.WriteToPacket(kPacket);
					NETWORK_SEND(kPacket)
				}
			}
		}break;
	case PT_M_C_ANS_SUMMONPARTYMEMBER:
		{
			SPT_M_C_ANS_SUMMONPARTYMEMBER kStruct;
			kStruct.ReadFromPacket(rkPacket);

			switch( kStruct.hResult )
			{
			case S_OK:
				{
				}break;
			case E_NOT_FOUND_ITEM:
				{					
					g_kChatMgrClient.LogMsgBox( 401304 );
				}break;
			case E_NOT_JOINED_PARTY:
				{					
					g_kChatMgrClient.LogMsgBox( 401305 );
				}break;
			case E_NOT_FOUND_MEMBER:
				{
					g_kChatMgrClient.LogMsgBox( 401306 );
				}break;
			case E_INVALID_ITEMPOS:
				{
					g_kChatMgrClient.LogMsgBox( 401307 );
				}break;
			case E_CANNOT_MOVE_MAP:
				{
					g_kChatMgrClient.LogMsgBox( 401312 );
				}break;
			case E_RETURN_REJECT:
				{
					g_kChatMgrClient.LogMsgBox( 401313 );
				}break;
			default:
				{
					g_kChatMgrClient.LogMsgBox( 401314 );
				}break;
			}
		}break;
	case PT_M_C_ANS_MOVETOSUMMONER:
		{
			SPT_M_C_ANS_MOVETOSUMMONER kStruct;
			kStruct.ReadFromPacket(rkPacket);

			switch( kStruct.hResult )
			{
			case S_OK:
				{
				}break;
			case E_NOT_FOUND_ITEM:
				{					
					g_kChatMgrClient.LogMsgBox( 401304 );
				}break;
			case E_NOT_JOINED_PARTY:
				{					
					g_kChatMgrClient.LogMsgBox( 401305 );
				}break;
			case E_NOT_FOUND_MEMBER:
				{
					g_kChatMgrClient.LogMsgBox( 401306 );
				}break;
			case E_INVALID_ITEMPOS:
				{
					g_kChatMgrClient.LogMsgBox( 401307 );
				}break;
			case E_CANNOT_MOVE_MAP:
				{
					g_kChatMgrClient.LogMsgBox( 401312 );
				}break;
			case E_RETURN_REJECT:
				{
					g_kChatMgrClient.LogMsgBox( 401313 );
				}break;
			default:
				{
					g_kChatMgrClient.LogMsgBox( 401314 );
				}break;
			}
		}break;
	case PT_S_C_REQ_PING:
		{
			Send_PT_C_S_NFY_UNIT_POS(rkPacket);
		}break;
	case PT_M_C_TRY_TAKE_COUPON:
		{
			int iError = 0,
				iTTWId = 0;
			rkPacket.Pop(iError);
			switch(iError)
			{
			case CRT_SUCCESS:	{ iTTWId = 1970; } break;
			case CRT_INVALID_COUPON: { iTTWId = 1971; } break;
			case CRT_INVALID_EVENT: { iTTWId = 1972; } break;
			case CRT_EXPIRE: { iTTWId = 1973; } break;
			case CRT_TOO_MANY_FAILED: { iTTWId = 1975; } break;
			case CRT_NOT_HAVE_EVENT_COUPON: { iTTWId = 1976; } break;
			case CRT_ALREADY_USED: { iTTWId = 1977; } break;
			case CRT_ONLY_ONCE_USE_MEMBER: { iTTWId = 1978; } break;
			case CRT_ONLY_ONCE_USE_CHARACTER: { iTTWId = 1979; } break;
			default: { iTTWId = 1974; } break;
			}
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iTTWId, true);
		}break;
	case PT_M_C_ANS_RENTALSAFE_EXTEND:
		{
			SPT_M_C_ANS_RENTALSAFE_EXTEND	Info;
			Info.ReadFromPacket(rkPacket);
			if( Info.kResult == S_OK )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2910, true);
				break;
			}
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2909, true);
		}break;
	case PT_M_C_ANS_COUPON_EVENT_SYNC:
	case PT_M_C_ANS_EVENT_LIST:
	case PT_M_C_ANS_JOIN_EVENT:
		{
			lwEventView::ReceivePacket(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_USER_MAP_MOVE:
		{
			HRESULT	hError = 0;
			int iTTWNo = 0;
			rkPacket.Pop(hError);
			if( hError != S_OK )
			{
				switch(hError)
				{
				case E_CANNOT_MOVE_MAP:	{ iTTWNo = 2916; }break;
				case E_INVALID_ITEMPOS:	{ iTTWNo = 2917; }break;
				case E_INCORRECT_ITEM:	{ iTTWNo = 2918; }break;
				case E_NOT_OPENED_MAP:	{ iTTWNo = 2919; }break;
				case E_CANNOT_STATE_MAPMOVE: { iTTWNo = 10410; }break;
				}
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iTTWNo, true);
			}
		}break;
	case PT_M_C_ANS_FIT2PRIMARYINV:
		{
			SPT_M_C_ANS_FIT2PRIMARYINV kData;
			kData.ReadFromPacket(rkPacket);
			if( EF2I_SUCCESS != kData.hResult )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2850 + kData.hResult, true);
			}
		}break;
	case PT_M_C_NFY_ADD_EFFECT:
		{
			PgStatusEffectManUtil::SReservedEffect kNewEffect;

			kNewEffect.ReadFromPacket(rkPacket);

			PgPilot* pkPilot = g_kPilotMan.FindPilot( kNewEffect.kPilotGuid );
			if( pkPilot )
			{
				PgStatusEffectManUtil::AddReservedEffect(g_kStatusEffectMan, kNewEffect);
			}
			else
			{
				if( g_kPilotMan.IsReservedPilot( kNewEffect.kPilotGuid ) )
				{
					g_kStatusEffectMan.AddReserveEffect(kNewEffect);
				}
			}

			if( g_pkWorld )
			{
				g_pkWorld->ProcessNpcEvent();
			}
		}break;
	case PT_M_C_NFY_DELETE_EFFECT:
		{
			BM::GUID kPilotGuid; // Character GUID
			int iEffectID = 0; // EffectKey (EffectID or ItemID)

			rkPacket.Pop( kPilotGuid );
			rkPacket.Pop( iEffectID );

			PgPilot* pkPilot = g_kPilotMan.FindPilot( kPilotGuid );
			if( pkPilot )
			{
				PgStatusEffectManUtil::DelReservedEffect(g_kStatusEffectMan, pkPilot, iEffectID);
			}
			else
			{
				if( g_kPilotMan.IsReservedPilot( kPilotGuid ) )
				{
					g_kStatusEffectMan.DelReserveEffect(kPilotGuid, iEffectID);
				}
			}

			if( g_pkWorld )
			{
				g_pkWorld->ProcessNpcEvent();
			}
		}break;
	case PT_M_C_NFY_DELETE_ALL_EFFECT:
		{
			BM::GUID kPilotGuid; // Character GUID

			rkPacket.Pop( kPilotGuid );
			
			PgPilot* pkPilot = g_kPilotMan.FindPilot( kPilotGuid );
			if( pkPilot )
			{
				g_kStatusEffectMan.RemoveAllStatusEffect( pkPilot );
			}
			else
			{
				if( g_kPilotMan.IsReservedPilot( kPilotGuid ) )
				{
					g_kStatusEffectMan.ClearReserveEffect(kPilotGuid);
				}
			}
		}break;
	case PT_M_C_NFY_ADD_EFFECT_DURATIONTIME:
		{
			BM::GUID kPilotGuid;
			int iEffectID;
			unsigned long ulEndTime;
			rkPacket.Pop( kPilotGuid );
			rkPacket.Pop( iEffectID );
			rkPacket.Pop( ulEndTime );

			PgPilot* pkPilot = g_kPilotMan.FindPilot( kPilotGuid );
			if( pkPilot && pkPilot->GetUnit() )
			{
				PgUnitEffectMgr& rkEffectMgr = pkPilot->GetUnit()->GetEffectMgr();

				if(CEffect* pkEffect = rkEffectMgr.FindInGroup(iEffectID, true))
				{
					pkEffect->SetEndTime(ulEndTime);
					lwUpdateBuffEndTime(iEffectID, ulEndTime);
				}
			}
		}break;
	case PT_M_C_NFY_ACTION_CUSTOM_CONTROL:
		{
			BM::GUID kPilotGuid;
			int iControlActionNo = 0;
			rkPacket.Pop( kPilotGuid );
			rkPacket.Pop( iControlActionNo );
			PgActor *pkActor = g_kPilotMan.FindActor( kPilotGuid );
			if ( pkActor )
			{
				PgAction *pkCurrentAction = pkActor->GetAction();
				if ( pkCurrentAction )
				{
					if ( pkCurrentAction->GetActionNo() == iControlActionNo )
					{
						BM::Stream *pkPacket = new BM::Stream(rkPacket);
						pkCurrentAction->SetParamAsPacket( pkPacket );
					}
				}
			}
			else
			{
				g_kPilotMan.BufferReservedPilotPacket( kPilotGuid, &rkPacket, 0 );
			}
		}break;
	case PT_M_C_ANS_START_HYPER_MOVE:
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
			if(pkPlayer)
			{
				BM::GUID kGuid;
				rkPacket.Pop(kGuid);
				pkPlayer->SetHyperMoveInfo(kGuid);
			}
		}break;
	case PT_M_C_NFY_PARTY_TELE_PORT:
		{
			std::string kTriggerID;
			int iType = 0;

			rkPacket.Pop(kTriggerID);
			rkPacket.Pop(iType);

			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if( pkActor 
				&& g_pkWorld
				)
			{
				PgTrigger *pkTrigger = g_pkWorld->GetTriggerByIDWithIgnoreCase(kTriggerID.c_str());
				if( pkTrigger )
				{
					pkActor->StopFollowActor();
					lua_tinker::call< void, lwTrigger, lwActor, int >("DoTeleportAction", lwTrigger(pkTrigger), lwActor(pkActor), iType);
				}
			}
		}break;
	case PT_M_C_ANS_FAILED_HYPER_MOVE:
		{
			int iType = 0;
			rkPacket.Pop( iType );

			switch( iType )
			{
			case TELE_PARTY_MOVE:
				{
					PgActor* pkActor = g_kPilotMan.GetPlayerActor();
					if( pkActor )
					{
						PgAction* pkAction = pkActor->GetAction();
						if( pkAction )
						{
							lwAction kAction(pkAction);
							kAction.SetParamInt(917, 1); // 실패 처리
						}
					}
				}break;
			default:
				{
				}break;
			}
		}break;
	case PT_S_C_NFY_REFRESH_DATA:
		{
			Recv_PT_S_C_NFY_REFRESH_DATA(rkPacket);
		}break;
	case PT_N_C_ANS_CASHSHOP_LIMIT_ITEM_INFO:
		{
			lwCashShop::RecvCashShop_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_OXQUIZ_ENTER:
	case PT_M_C_ANS_OXQUIZ_EXIT:
	case PT_M_C_REQ_OXQUIZ_REQ_QUIZ:
	case PT_M_C_ANS_OXQUIZ_ANS_QUIZ:
	case PT_M_C_NOTI_OXQUIZ_OPEN:
	case PT_M_C_NOTI_OXQUIZ_LOCK:
	case PT_M_C_NOTI_OXQUIZ_RESULT:
	case PT_M_C_NOTI_OXQUIZ_CLOSE:
		{
			lwOXQuizUI::RecvOXQuiz_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_REQ_LUCKYSTAR_JOIN:
	case PT_M_C_ANS_LUCKYSTAR_ENTER:
	case PT_M_C_LUCKYSTAR_CLOSE:
	case PT_M_C_LUCKYSTAR_RESULT:
	case PT_M_C_LUCKYSTAR_RESULT_CUSTOM:
	case PT_M_C_ANS_LUCKYSTAR_USERINFO:
	case PT_M_C_ANS_LUCKYSTAR_CHANGE_STAR:
		{
			lwLuckyStarUI::RecvLuckyStar_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_GMCMD_SERVERTIME:
		{	
			int iServerType = 0;
			rkPacket.Pop( iServerType );

			bool bRet = false;
			std::wstring kLog;

			switch( iServerType )
			{
			case 0:	// OS시간
				{
					int iHour = 0;
					int iMin = 0;
					int iSec = 0;

					rkPacket.Pop( iHour );
					rkPacket.Pop( iMin );
					rkPacket.Pop( iSec );
					
					bRet = FormatTTW( kLog, 99300, iHour, iMin, iSec );
					if( !bRet )
					{
						return false;
					}					
				}break;
			case 1:	
				 {
					 WORD wHour = 0;
					 WORD wMin = 0;
					 WORD wSec = 0;

					 rkPacket.Pop( wHour );
					 rkPacket.Pop( wMin );
					 rkPacket.Pop( wSec );
					
					 bRet = FormatTTW( kLog, 99301, wHour, wMin, wSec );
					 if( !bRet )
					 {
						return false;
					 }					 
				 }break;
			}

			SChatLog kChatLog( CT_EVENT_GAME );
			g_kChatMgrClient.AddLogMessage( kChatLog, kLog );

		}break;
	case PT_M_C_GMCMD_GODHAND_ON:
		{
			bool bRet = false;
			std::wstring kLog;

			bRet = FormatTTW( kLog, 99400 );
			if( !bRet )
			{
				return false;
			}	
			SChatLog kChatLog( CT_EVENT_GAME );
			g_kChatMgrClient.AddLogMessage( kChatLog, kLog );
		}break;
	case PT_M_C_GMCMD_GODHAND_OFF:
		{
			bool bRet = false;
			std::wstring kLog;

			bRet = FormatTTW( kLog, 99401 );
			if( !bRet )
			{
				return false;
			}	
			SChatLog kChatLog( CT_EVENT_GAME );
			g_kChatMgrClient.AddLogMessage( kChatLog, kLog );
		}break;

	case PT_M_C_NOTI_MACRO_INPUT_PASSWORD:
	case PT_M_C_ANS_MACRO_INPUT_PASSWORD:
		{
			UIMacroCheck::RecvMC_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_INVENTORY_EXTEND:
		{
			int iError = 0;
			rkPacket.Pop(iError);
			if( EC_OK == iError )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406107, true);
			}
			else
			{
			}
		}break;
	case PT_M_C_CS_ANS_LAST_RECVED_GIFT:
		{
			if( g_pkWorld
				&& g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND) )
			{
				return true;
			}

			std::wstring	kGiftTooltip;
			rkPacket.Pop(kGiftTooltip);
			XUI::CXUI_Wnd* pWnd = XUIMgr.Activate(_T("FRM_GIFT_RECV"));
			if( pWnd )
			{
				kGiftTooltip += TTW(1994);
				pWnd->SetCustomData(kGiftTooltip.c_str(), sizeof(std::wstring::value_type)*kGiftTooltip.size());
			}
		}break;
		break;
	case PT_M_C_REQ_MISSION_GADACOIN_ITEM:
		{
			g_kMissionComplete.GadaCoinItemView(rkPacket);
		}break;
	case PT_M_C_NFY_CLIENTOBJECT_CHANGED:
		{
			PgWEClientObjectState kNfyState;
			kNfyState.ReadFromPacket(rkPacket);
			
			g_kWEClientObjectMgr.UpdateClientObject( kNfyState );
		}break;
	case PT_M_C_NFY_TOWER_ATTACK:
		{
			lwAddWarnDataTT( 400978 );
		}break;
	case PT_M_C_ANS_DEFENCE_RESTART:
		{
			lwCMP_STATE_CHANGE(16);
		}break;
	case PT_M_C_ANS_DEFENCE_SELECT_CLOSE:
		{
			BM::Stream kPacket(PT_C_M_NFY_DEFENCE_ENDSTAGE);
			NETWORK_SEND(kPacket);
		}break;
	case PT_NFY_M_C_DEFENCE_ZEN_EFFECT:
		{
			BYTE byDir = 0;
			rkPacket.Pop(byDir);
			if(byDir & 0x01)
			{
				lua_tinker::call< void, bool >("DefenceMonsterZenEffect", false);
			}
			if(byDir & 0x02)
			{
				lua_tinker::call< void, bool >("DefenceMonsterZenEffect", true);
			}
		}break;
	case PT_M_C_ANS_DEFENCE7_RELAY_STAGE:
		{
			std::wstring kName;
			int iStage = 0;
			int iDelayTime = 0;
			rkPacket.Pop(kName);
			rkPacket.Pop(iStage);
			rkPacket.Pop(iDelayTime);

			BM::vstring kMsg1( TTW(401220) );
			kMsg1.Replace(L"#NAME#", kName);
			Notice_Show(kMsg1, EL_Normal);

			BM::vstring kMsg2( TTW(401221) );
			kMsg2.Replace(L"#STAGE#", iStage);
			Notice_Show(kMsg2, EL_Normal);

			XUIMgr.Close(L"SFRM_DEFENCE7_RELAY");

			g_kPilotMan.UnlockPlayerInput( SReqPlayerLock(EPLT_EventScript, false) );
			lua_tinker::call<void,int>("SetDefence7_DeadCount", iDelayTime/1000);
			XUIMgr.Call(L"FRM_DEFENCE_NEXT_DELAY");
		}break;
	case PT_NFY_M_C_DEFENCE_NOTICE_MSG:
		{
			int iTTW = 0;
			bool bRet = false;
			std::wstring kLog;

			rkPacket.Pop( iTTW );

			bRet = FormatTTW( kLog, iTTW );
			if( bRet )
			{
				SChatLog kChatLog( CT_EVENT_GAME );
				g_kChatMgrClient.AddLogMessage( kChatLog, kLog );
			}
		}break;
	case PT_M_C_NFY_WORLDEVENT_CHANGED:
		{
			PgWorldEventState kNfyState;
			kNfyState.ReadFromPacket(rkPacket);

			g_kWorldEventClientMgr.AddWorldEvent(kNfyState);
		}break;
	case PT_M_C_NFY_TRIGGER_ENABLE:
		{
			std::string kTriggerID;
			bool bEnable = false;

			rkPacket.Pop( kTriggerID );
			rkPacket.Pop( bEnable );

			PgWorldUtil::SetTriggerEnable(g_pkWorld, kTriggerID, bEnable);
		}break;
	case PT_M_C_NFY_TRIGGER_SCRIPT:
		{
			std::string m_kScript;

			rkPacket.Pop( m_kScript );

			if( std::string::npos == m_kScript.find("(") )
			{
				if( m_kScript.length() > 0 )
				{
					lua_tinker::call<void>((m_kScript).c_str());
				}
			}
			else
			{
				lua_tinker::dobuffer(*lua_wrapper_user(g_kLuaTinker), m_kScript.c_str(), m_kScript.size());
			}
		}break;
	case PT_M_C_ANS_CREATE_CHARACTER_CARD:
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD:
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD_COMMENT:
	case PT_M_C_ANS_RECOMMEND_CHARACTER:
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD_STATE:
	case PT_M_C_ANS_CHARACTER_CARD_INFO:
	case PT_M_C_NOTI_CREATE_CARD:
	case PT_M_C_ANS_SEARCH_MATCH_CARD:
		{
			lwCharacterCard::ReceivePacket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_ITEM_ACTION_BIND:
		{// 자물쇠 사용 요청 결과
			lwUIItemBind::RecvBindResult(rkPacket);
		}break;
	case PT_M_C_ANS_ITEM_ACTION_UNBIND:
		{// 열쇠 사용 요청 결과
			lwUIItemBind::RecvUnbindResult(rkPacket);
		}break;
	case PT_M_C_ANS_REG_PORTAL:
	case PT_M_C_ANS_USE_PORTAL:
		{// 기억 전송 카드
			g_kMemTransCard.RecvPortalReqResult(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_GEMSTOREINFO:
		{// 보석교환 상점 판매 물품 리스트 받음
			g_kGemStore.RecvGemstoreInfo(rkPacket);
		}break;
	case PT_M_C_ANS_GEMSTORE_BUY:
		{// 보석교환 결과 받음
			g_kGemStore.RecvGemTradeResult(rkPacket);
		}break;
	case PT_M_C_ANS_DEFGEMSTOREINFO:
		{
			g_kGemStore.RecvDefGemstoreInfo(rkPacket);
		}break;
	case PT_M_C_ANS_DEFGEMSTORE_BUY:
		{
			g_kGemStore.RecvDefGemTradeResult(rkPacket);
		}break;
	case PT_M_C_ANS_USE_EMOTION:
	case PT_C_M_NFY_USE_EMOTION:
		{
			lwUniEmotiUI::ReceivePacket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_NFY_PET_INVENTORY:
		{
			BM::GUID kPetID;
			rkPacket.Pop( kPetID );

			bool bFind = false;

			PgActor *pkActor = g_kPilotMan.FindActor( kPetID );
			if ( pkActor )
			{
				PgPilot *pkPilot = pkActor->GetPilot();
				if ( pkPilot )
				{
					bFind = true;

					CUnit *pkUnit = pkPilot->GetUnit();
					if ( pkUnit )
					{
						pkUnit->GetInven()->ReadFromPacket( rkPacket, WT_DEFAULT );
						pkUnit->InvenRefreshAbil();
						pkActor->EquipAllItem();
					}
				}
			}

			if ( !bFind )
			{
				// 이건 내 Pet의 Inventory이다.
				PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if ( pkMyPlayer )
				{
					PgBase_Item kPetItem;
					if ( S_OK == pkMyPlayer->GetInven()->GetSelectedPetItem( kPetItem ) )
					{
						if ( kPetItem.Guid() == kPetID )
						{
							PgItem_PetInfo * pkPetInfo = NULL;
							if ( true == kPetItem.GetExtInfo( pkPetInfo ) )
							{
								if ( !PgActorPet::ms_pkMyPetInventory )
								{
									PgActorPet::ms_pkMyPetInventory = new_tr PgInventory;
								}

								if ( PgActorPet::ms_pkMyPetInventory )
								{
									PgActorPet::ms_pkMyPetInventory->OwnerGuid( kPetID );
									PgActorPet::ms_pkMyPetInventory->OwnerName( pkPetInfo->Name() );
									PgActorPet::ms_pkMyPetInventory->ReadFromPacket( rkPacket, WT_DEFAULT );
									PgActorPet::ms_pkMyPetInventory->RefreshAbil(NULL);
								}
							}
							break;
						}
					}
					
					{
						PgPetUIUtil::SaveMyPetInventory( rkPacket, kPetID );
					}
				}
			}
		}break;
	case PT_M_C_NFY_PET_INVENTORY_ACTION:
		{
			Recv_PT_M_C_NFY_PET_INVENTORY_ACTION(rkPacket);
		}break;
	case PT_M_C_NFY_PET_REVIVE:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if ( pkPlayer )
			{
				PgBase_Item kPetItem;
				if ( S_OK == pkPlayer->GetInven()->GetSelectedPetItem( kPetItem ) )
				{
					PgItem_PetInfo *pkPetInfo = NULL;
					if ( true == kPetItem.GetExtInfo( pkPetInfo ) )
					{
						// 펫이 부활하였습니다.
						std::wstring wstrMsg;
						WstringFormat( wstrMsg, MAX_PATH, TTW(357).c_str(), pkPetInfo->Name().c_str() );
						Notice_Show( wstrMsg, EL_Level2 );
					}
				}
			}
		}break;
	case PT_M_C_NFY_PET_UPGRADE:
		{
			Notice_Show(TTW(339), EL_Level2); //MSG: 펫이 다음 단계로 진화하였습니다.

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_PET_UPGRADE"));
			if(NULL == pkWnd) { break; }
			XUI::CXUI_Wnd* pkEffect = pkWnd->GetControl(_T("FRM_EFFECT_MODEL"));
			if(NULL != pkEffect)
			{
				lwPetOptChanger::lwInitDyeingEffect();
				pkEffect->Visible(true);
				//pkEffect->RefreshLastTickTime();
			}
			XUI::CXUI_Wnd* pkPrice = pkWnd->GetControl(_T("SFRM_PRICE_BG"));
			if(NULL != pkPrice)
			{
				pkPrice->Text(_T(""));
			}

			XUI::CXUI_Button* pkBtnOK = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl( _T("BTN_OK")) );
			if(NULL != pkBtnOK)
			{
				pkBtnOK->Disable(true);
			}
		}break;
	case PT_C_M_REQ_DEACTIVATE_SKILL:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if ( pkPlayer )
			{
				BM::GUID const& kPetGuid = pkPlayer->SelectedPetID();
				if( kPetGuid.IsNotNull() )
				{
					PgPilot* pkPetPilot = g_kPilotMan.FindPilot(kPetGuid);
					if( pkPetPilot )
					{
						PgPet * pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
						if ( pkPet )
						{
							int iSkillNo = 0;
							rkPacket.Pop( iSkillNo );
							pkPet->GetMySkill()->Delete( iSkillNo );
						}
					}
				}
			}
		}break;
	case PT_M_C_NFY_GROUND_DYNAMIC_ATTR:
		{
			if( g_pkWorld )
			{
				int iNewGndDynamicAttr = 0;
				int const iOldGndDynamicAttr = g_pkWorld->DynamicGndAttr();

				rkPacket.Pop( iNewGndDynamicAttr );
				g_pkWorld->DynamicGndAttr( static_cast< EDynamicGroundAttr >(iNewGndDynamicAttr) );

				if( 0 != (iNewGndDynamicAttr & DGATTR_FLAG_FREEPVP) )
				{
					g_kBattleSquareMng.ReadFromPacket( rkPacket );
					g_pkWorld->AllPlayerUpdateName();
				}
			}
		}break;
	case PT_M_C_ANS_EXPCARD_ACTIVATE:
	case PT_M_C_ANS_EXPCARD_DEACTIVATE:
	case PT_M_C_ANS_EXPCARD_USE:
		{
			lwAccumlationExpCard::ReceivePacket_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_MIXUPITEM:
		{
			lwItemMix::RecvCostumeMix_Command(wPacketType, rkPacket);			
		}break;
	//홈 외관
	case PT_M_C_ANS_HOME_EQUIP:
	case PT_M_C_ANS_HOME_UNEQUIP:
	case PT_M_C_ANS_HOME_ITEM_MODIFY:
	case PT_M_C_ANS_HOME_USE_HOME_STYLEITEM:
		{
			if( g_pkWorld )
			{
				if(g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 내부
				{
					if( g_pkWorld->GetHome() )
					{
						g_pkWorld->GetHome()->ReceivePacket_Command(wPacketType, rkPacket);
					}
				}
				else if(g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
				{
					BM::GUID kGuid;
					if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
					{
						PgPilot* pkPilot = g_kPilotMan.FindHouse(kGuid);
						if( pkPilot )
						{
							PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(pkPilot->GetGuid()));
							if( pkHouse )
							{
								pkHouse->RecvHouse_Command(wPacketType, rkPacket);
							}
						}
					}
				}
			}
			
		}break;
	case PT_M_C_ANS_MYHOME_EXIT:
		{
			if( g_pkWorld )
			{
				HRESULT kErr = S_OK;
				rkPacket.Pop(kErr);
				if(S_OK==kErr)
				{
					g_pkWorld->ReleaseHome();
				}
			}

		}break;
	case PT_M_C_ANS_SHAREQUEST:
		{
			int const iNoticeColor = 2;

			BYTE cRet = 0;
			rkPacket.Pop( cRet );

			int iTTW = 0;
			switch( cRet )
			{
			case QSR_Notify:
				{
					BM::GUID kOrgGuid;
					int iShareQuestID = 0;

					rkPacket.Pop( kOrgGuid );
					rkPacket.Pop( iShareQuestID );

					Quest::RecvShareQuest(kOrgGuid, iShareQuestID);
				}break;
			case QSR_Yes:
				{
					BM::GUID kCharGuid;

					rkPacket.Pop( kCharGuid );

					SPartyMember const* pkMember = NULL;
					if( g_kParty.GetMember(kCharGuid, pkMember) )
					{
						std::wstring kTemp;
						if( FormatTTW(kTemp, 401424, pkMember->kName.c_str()) )
						{
							g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), kTemp, true, iNoticeColor);
						}
					}
				}break;
			case QSR_Success:
				{
					ContQuestShareRet kRetVec;
					rkPacket.Pop( kRetVec );

					if( kRetVec.empty() )
					{
						g_kChatMgrClient.LogMsgBox(401422); // 공유할 파티원이 없습니다.
						break;
					}

					ContQuestShareRet::const_iterator iter = kRetVec.begin();
					while( kRetVec.end() != iter )
					{
						ContQuestShareRet::value_type const& rkRet = (*iter);
						switch( rkRet.cRet )
						{
						case QBL_NowTalking:		{ iTTW = 401408; }break; // 퀘스트 대화중이라 공유 안되
						case QBL_NowEventScript:	{ iTTW = 401409; }break; // 이벤트를 보고 있어 공유 안되
						case QBL_Begined:			{ iTTW = 401410; }break; // 이미 가지고 있어
						case QBL_Ended:				{ iTTW = 401411; }break; // 이미 완료 했어
						case QBL_Class:				{ iTTW = 401412; }break; // 공유 할수 있는 직업이 아닙니다.
						case QBL_Time:				{ iTTW = 401413; }break; // 퀘스트를 공유 할 수 있는 시간이 아닙니다
						case QBL_Item:
						case QBL_Quest:				{ iTTW = 401414; }break; // 퀘스트 수행할 조건이 되지 않습니다
						case QBL_Level_One:
						case QBL_Level:				{ iTTW = 401415; }break; // 퀘스트 수행할 레벨이 안되
						case QBL_Reject:			{ iTTW = 401416; }break; // 퀘스트를 거절 하였다
						case QBL_State:				{ iTTW = 401418; }break; // 퀘스트를 공유 할수 없는 상태다(오픈마켓,캐쉬샵)
						case QBL_None:				{ iTTW = 401419; }break; // 퀘스트 공유를 신청 하였다
						case QBL_Couple:			{ iTTW = 401420; }break; // 커플/부부 퀘스트는 커플/부부 끼리만 가능
						case QBL_HaveCoupleQuest:	{ iTTW = 401423; }break; // 다른 커플/부부 퀘스트를 가지고 있음메
						case QBL_SkillHave:			{ iTTW = 401425; }break; // 필요 스킬 안배웠음
						case QBL_Max:
						default:
							{
								iTTW = 401417;
							}break; // 공간이 부족해
						}

						SPartyMember const* pkMember = NULL;
						if( g_kParty.GetMember(rkRet.kGuid, pkMember) )
						{
							std::wstring kTemp;
							if( FormatTTW(kTemp, iTTW, pkMember->kName.c_str()) )
							{
								g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), kTemp, true, iNoticeColor);
							}
						}

						++iter;
					}
				}break;
			case QSR_CantShare: // 공유 할 수 없는 퀘스트
				{
					g_kChatMgrClient.LogMsgBox(401404);
				}break;
			case QSR_OnlyParty: // 파티 상태에서만 공유 가능
				{
					g_kChatMgrClient.LogMsgBox(401407);
				}break;
			case QSR_OnlyHave: // 진행중인 퀘스트만 공유 가능 합니다.
			default:
				{
					g_kChatMgrClient.LogMsgBox(401406);
				}break;
			}

		}break;
	case PT_M_C_ANS_ACCEPT_SHAREQUEST:
		{
			BYTE cRet = 0;
			rkPacket.Pop( cRet );

			switch( cRet )
			{
			case QSR_Ing: // 이미 해당 퀘스트를 진행중
				{
					g_kChatMgrClient.LogMsgBox(401403);
				}break;
			case QSR_CantShare: // 공유 할 수 없는 퀘스트
				{
					g_kChatMgrClient.LogMsgBox(401404);
				}break;
			case QSR_Max: // 퀘스트 슬롯이 꽉 찼어
				{
					g_kChatMgrClient.LogMsgBox(401421);
				}break;
			case QSR_Failed: // 퀘스트 공유자가 사라져 공유 받을 수 없어
			default:
				{
					g_kChatMgrClient.LogMsgBox(401405);
				}break;
			}
		}break;
	case PT_M_C_NFY_EVENT_QUEST_INFO:
		{
			bool bClearInfo = false;
			rkPacket.Pop( bClearInfo );

			if( bClearInfo )
			{
				PgEventQuestUI::ClearInfo();
				PgEventQuestUI::CloseUI();
				PgEventQuestUI::CloseMini();
			}
			else
			{
				bool const bPreStatus = PgEventQuestUI::IsNowEvent();
				PgEventQuestUI::ReadFromPacket( rkPacket );
				if( bPreStatus ) // 이미 시작 중이었으면, 업데이트
				{
					PgEventQuestUI::CheckNowEvent();
				}
				else // 새로 시작한것이면 UI 띄워줘
				{
					PgEventQuestUI::CheckNowEvent();
					PgEventQuestUI::UpdateUI();
				}
				PgEventQuestUI::CheckTargetItem();
			}
		}break;
	case PT_N_C_NFY_EVENT_QUEST_COMPLETE:
		{
			int const iCompleteMsg1 = 450223;
			int const iCompleteMsg2 = 450224;
			VEC_WSTRING kVec;
			std::back_inserter(kVec) = TTW(iCompleteMsg1);
			std::back_inserter(kVec) = TTW(iCompleteMsg2);

			g_kQuestMan.CallCompleteStamp(kVec);
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if( pkActor )
			{
				pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete2", 0.0f);
			}
			PgEventQuestUI::Complete();
		}break;
	//case PT_M_C_ANS_EVENT_QUEST_TALK:
	//	{
	//		lua에 있음
	//	}break;
	case PT_M_C_NFY_EVENT_QUEST_NOTICE:
		{
			std::wstring kNotice;
			rkPacket.Pop( kNotice );
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), kNotice, true, 2);
		}break;
	case PT_M_C_HOME_ANS_MAPMOVE:
		{
			HRESULT kErr = S_OK;
			rkPacket.Pop( kErr );
			switch(kErr)
			{
			case S_OK:
				{
				}break;
			case E_MYHOME_NOT_OPENED_MAP:
				{
					lwAddWarnDataTT(201116);
				}break;
			case E_MYHOME_NOT_HOMETOWN:
				{
					lwAddWarnDataTT(201115);
				}break;
			case E_MYHOME_NOT_FOUND_MAP:
			case E_MYHOME_CANT_MOVE_MAP:
				{
					lwAddWarnDataTT(1);
				}break;
			}
		}break;
	case PT_M_C_NFY_BS_DEAD:
	case PT_M_C_NFY_WANT_JOIN_BS:
	case PT_M_C_ANS_WANT_JOIN_BS:
	case PT_T_C_ANS_WANT_JOIN_BS_CHANNEL:
	case PT_M_C_NFY_BS_DEATHCOUNT:
	case PT_M_C_NFY_BS_REWARD:
	case PT_N_C_NFY_BS_NOTICE:
	case PT_M_C_NFY_BS_ITEM_USER_POS:
	case PT_M_C_NFY_BS_SCORE:
	case PT_M_C_NFY_BS_UNIT_POS:
	case PT_T_C_ANS_BS_CHANNEL_INFO:
	case PT_M_C_NFY_BS_MONSTER_POS:
	case PT_M_C_ANS_BS_EXIT:
		{
			g_kBattleSquareMng.OnRecvPacket(wPacketType, rkPacket);
		}break;
	case NFY_M_C_REMAIN_MONSTER:
	case NFY_M_C_REMAIN_TIME_MSEC: // 시간제한 시작/종료
	case NFY_M_C_LIMITED_TIME_MSEC: // 시간제한으로 실패
		{
			PgSuperGroundUI::ProcessMsg(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_USE_REPAIR_ITEM:
	case PT_M_C_ANS_USE_ENCHANT_ITEM:
		{
			HRESULT kResult = EC_OK;
			rkPacket.Pop(kResult);
			Recv_PT_X_X_ANS_ERROR_CHECK(kResult);
		}break;
	case PT_M_C_ANS_STATUS_CHANGE:
		{
			g_kUIActiveStatus.RecvPacket(rkPacket);
		}break;
	case PT_M_C_ANS_ACHIEVEMENT_RANK:
		{
			lwUIBook::OnRecvBookRankInfo(rkPacket);
		}break;
	case PT_M_C_NOTI_ACHIEVEMENT_COMPLETE_FIRST:
		{
			int iAchieveNo = 0;
			std::wstring kUserName;

			rkPacket.Pop(iAchieveNo);
			rkPacket.Pop(kUserName);

			lwAchieveNfyMgr::CallFirstAchieveUserNfy(iAchieveNo, kUserName);
		}break;
	case PT_M_C_NFY_GENERIC_ACHIEVEMENT_NOTICE:
		{
			int TextNo = 0;
			CONT_GENERIC_NOTICE ContNotice;

			rkPacket.Pop(TextNo);
			PU::TLoadTable_AA(rkPacket, ContNotice);

			lwAchieveNfyMgr::CallGenericNfy( TTW(TextNo), ContNotice );
		}break;
	case PT_M_C_REQ_HIDDEN_MOVE_CHECK:
		{
			Recv_PT_M_C_REQ_HIDDEN_MOVE_CHECK(rkPacket);
		}break;
	case PT_M_C_NFY_ALL_GROUND_WORLD_ENVIRONMENT_STATUS:
		{
			DWORD dwWorldEnvStatus = 0;
			rkPacket.Pop( dwWorldEnvStatus );
			if(g_pkWorld)
			{
				g_pkWorld->OnWorldEnvironmentStatusChanged(dwWorldEnvStatus,false);
			}
		}break;
	case PT_N_C_ANS_REALM_QUEST_INFO:
		{
			// 내가 달성 했을 때(전체 정보 UI를 띄운다)
			SRealmQuestInfo const kRealmQuestInfo(rkPacket);
			g_kQuestMan.CallRealmQuestUI(kRealmQuestInfo);
		}break;
	case PT_N_C_ANS_GET_ENTRANC_OPEN_GUILD:
		{// 길드
			g_kGuildMgr.ReadEntranceOpenGuildFromPacket(rkPacket);
		}break;
	case PT_N_C_ANS_GUILD_ENTRANCE:
		{
			bool bRet = false;
			rkPacket.Pop(bRet);
			g_kGuildMgr.ResultGuildEntrance(bRet);
		}break;
	case PT_N_C_NFY_GUILD_ENTRANCE_STATE:
		{// 길드가입 신청상태
			BYTE byState = 0;
			BM::GUID kGuildGuid;
			__int64 i64GuildEntranceFee = 0i64;

			rkPacket.Pop(byState);
			rkPacket.Pop(kGuildGuid);
			rkPacket.Pop(i64GuildEntranceFee);

			if( kGuildGuid.IsNull() )
			{
				g_kGuildMgr.InitApplicantState();
			}
			else
			{
				g_kGuildMgr.SetGuildApplicationState(kGuildGuid, byState, i64GuildEntranceFee);
			}
		}break;
	case PT_N_C_NFY_NEW_GUILD_APPLICANT:
		{// 신규가입신청 도착
			SGuildEntranceApplicant kNewApplicant;
			kNewApplicant.ReadFromPacket(rkPacket);
			g_kGuildMgr.AddEntranceApplicant(kNewApplicant);
			lwGuild::GuildApplicantSetting(); // UI 갱신

			std::wstring wstrMsg(TTW(400465));
			SChatLog kChatLog(CT_GUILD);
			g_kChatMgrClient.AddLogMessage(kChatLog, wstrMsg);
		}break;
	case PT_N_C_NFY_GUILD_ENTRANCE_CANCEL:
		{// 신청취소 도착
			BM::GUID kCharGuid;
			rkPacket.Pop(kCharGuid);
			g_kGuildMgr.DelEntranceApplicant(kCharGuid);
		}break;
	case PT_N_C_NFY_REALM_QUEST_INFO:
		{
			// 남이 달성하거나 Tick으로 알림(툴팁 형태의 UI를 띄운다)
			SRealmQuestInfo const kRealmQuestInfo(rkPacket);
			g_kQuestMan.CallRealmQuestMiniUI(kRealmQuestInfo);
		}break;
	case PT_N_C_NFY_REALM_QUEST_REWARD:
		{
			// 공지사항을 띄운다
			bool bSuccess = false;
			std::wstring kRealmQuestID;
			DWORD dwNowSec = 0;

			rkPacket.Pop( bSuccess );
			rkPacket.Pop( kRealmQuestID );
			rkPacket.Pop( dwNowSec );
			g_kQuestMan.CallRealmQuestBuffNotice(kRealmQuestID, bSuccess, dwNowSec);
		}break;
	case PT_M_C_ANS_HIDDEN_ITEM:
		{
			BM::DBTIMESTAMP_EX kNow;
			int iItemNo = 0;
			SItemPos rkItemInvPos;

			rkPacket.Pop( kNow );
			rkPacket.Pop( iItemNo );
			rkPacket.Pop( rkItemInvPos );

			kNow.hour = 0;
			kNow.minute = 59 - kNow.minute;
			kNow.second = 59 - kNow.second;

			if( 0 > kNow.minute )
			{
				kNow.minute = 0;
			}
			if( 0 > kNow.second )
			{
				kNow.second = 0;
			}

			BM::PgPackedTime kNowTime(kNow);

			XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call(std::wstring(_T("SFRM_HIDDEN_ITEM")));
			if( !pkTopWnd )
			{
				return false;
			}

			XUI::CXUI_Wnd* pkMsgWnd = pkTopWnd->GetControl(std::wstring(_T("FRM_MESSAGE")));
			if( !pkMsgWnd )
			{
				return false;
			}

			XUI::CXUI_Wnd* pkBtnWnd = pkTopWnd->GetControl(std::wstring(_T("BTN_OK")));
			if( !pkBtnWnd )
			{
				return false;
			}

			pkMsgWnd->SetCustomData(&iItemNo, sizeof(iItemNo));
			pkBtnWnd->SetCustomData(&rkItemInvPos, sizeof(rkItemInvPos));

			std::wstring wstrTemp = _T("");
			FormatTTW( wstrTemp, 400992, kNowTime.Hour(), kNowTime.Min(), kNowTime.Sec() );

			pkMsgWnd->Text(wstrTemp);			
		}break;		
	case PT_M_C_NFY_GAMBLEMACHINE_RESULT:	// 인게임 가챠, 캐시샵 가챠
	case PT_M_C_ANS_USE_GAMBLEMACHINE:		// 인게임 가챠
		{
			g_kIGGacha.RecvIGGachaPacket(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_USE_GAMBLEMACHINE_READY:
	case PT_M_C_ANS_RELOAD_ROULETTE:
	case PT_M_C_ANS_ROULETTE_RESULT:
	case PT_M_C_NOTI_ROULETTE_RESULT_MAIL:
		{// 캐시샵 가챠용 패킷
			g_kCSGacha.RecvCSGachaPacket(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_GAMBLEMACHINEINFO:
		{// 가챠 아이템 리스트 요청을 받아서 
			bool bCashGacha = false;
			rkPacket.Pop(bCashGacha);
			if(bCashGacha)
			{// 캐시샵
				switch(g_kCashShopMgr.GetShopType())
				{// 캐시샵 가챠, 코스튬 조합 또는
				case PgCashShop::ECASHSHOP_GACHA: { g_kCSGacha.RecvCSGachaPacket(wPacketType, rkPacket); } break;
				case PgCashShop::ECOSTUME_MIX:	  { g_kCostumeMix.RecvPacket(wPacketType, rkPacket);	 } break;
				default:{} break;
				}
			}
			else
			{// 인게임 가챠에 넘겨준다
				g_kIGGacha.RecvIGGachaPacket(wPacketType, rkPacket);
			}
		}break;
	case PT_M_C_ANS_USE_GAMBLEMACHINE_MIXUP_READY:
	case PT_M_C_ANS_RELOAD_ROULETTE_MIXUP:
	case PT_M_C_ANS_ROULETTE_MIXUP_RESULT:
	case PT_M_C_NOTI_ROULETTE_MIXUP_RESULT_MAIL:
	case PT_M_C_NFY_GAMBLEMACHINE_MIXUP_RESULT:
		{
			g_kCostumeMix.RecvPacket(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_USE_REDICE_OPTION_ITEM:
		{
			lwCraftOptChanger::Recv_ANS_Packet(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_USE_REDICE_OPTION_PET:
		{
			lwPetOptChanger::Recv_ANS_Packet(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_SKILL_EXTEND:
		{
			lwSkillExtend::Recv_ANS_Packet(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_ENCHANT_SHIFT:
		{
			g_kEnchantShift.RecvPacket(rkPacket);
		}break;
	case PT_M_C_ANS_USE_UPGRADE_OPTION_ITEM:
		{
			g_kItemRarityBuildUpMgr.Recv(rkPacket);
		}break;
	case PT_M_C_ANS_BASIC_OPTION_AMP:
		{
			g_kItemRarityAmplify.Recv(rkPacket);
		}break;
	case PT_M_C_ANS_RESET_ATTATCHED:
		{
			lwUseUnlockItem::Recv_ANS_Packet(wPacketType, rkPacket);
		}break;
	case PT_M_C_NFY_EFFECT_ARGCHANGED:
		{//	Effect의 Arg 값이 변화 되었때 오는값
			BM::GUID kTargetGuid;
			BM::GUID kCasterGuid;
			int	iEffectNo = 0;
			SActArg kRecvedEffectArg;

			rkPacket.Pop(kTargetGuid);					// Target(적용대상)
			rkPacket.Pop(kCasterGuid);					// Caster(시전자)
			rkPacket.Pop(iEffectNo);					// Arg값이 변화할 EffectNo
			kRecvedEffectArg.ReadFromPacket(rkPacket);	// Arg값

			PgPilot	*pkPilot = g_kPilotMan.FindPilot(kTargetGuid);
			if(pkPilot)
			{// 적용 대상이 가진
				CUnit* pkUnit = pkPilot->GetUnit();
				if(pkUnit)
				{
					CEffect* pkEffect = pkUnit->GetEffect(iEffectNo);
					if(pkEffect)
					{// Effect를 얻어와
						SActArg* pkTargetActArg = pkEffect->GetActArg();
						if(pkTargetActArg)
						{
							SActArg::CONT_VALUE& kTargetArgCont = pkTargetActArg->m_contValue;

							SActArg::CONT_VALUE& kRecvArgCont	= kRecvedEffectArg.m_contValue;
							SActArg::CONT_VALUE::const_iterator kRecvItor = kRecvArgCont.begin();
							
							while(kRecvArgCont.end() != kRecvItor)
							{// 그 Effect의 Arg(부가 정보)들을
								int const& iKey = kRecvItor->first;
								BM::Stream const& kSecond = kRecvItor->second;
								// 추가 하려는데
								std::pair<SActArg::CONT_VALUE::iterator, bool> kRet = kTargetArgCont.insert(std::make_pair(iKey, kSecond));
								if(!kRet.second)
								{// 이미 있으면, 새 값으로 교체해 준다.
									kRet.first->second = kSecond;
								}
								++kRecvItor;
							}
						}
					}
				}
			}
		}break;
	case PT_M_C_NFY_UNIT_AWAKE_CHARGE_STATE:
		{
			BYTE byType = 0;
			rkPacket.Pop(byType);
			switch(byType)
			{
			case EACS_UI_CHANGE:
				{// 챠지 시작
					bool bAwakeChargeUI = false;
					rkPacket.Pop(bAwakeChargeUI);
					lua_tinker::call<void, bool>("SetAwakeChargeState", bAwakeChargeUI);
				}break;
			case EACS_AWAKE_CHARGE_END:
				{
					BM::GUID kTargetGuid;
					rkPacket.Pop(kTargetGuid);			// 변경될 녀석
					PgPilot	*pkPilot = g_kPilotMan.FindPilot(kTargetGuid);
					if(pkPilot)
					{// 적용 대상이 가진
						PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
						if(pkActor)
						{
							PgAction* pkAction =  pkActor->GetAction();
							if(pkAction)
							{
								std::string const kActionID = pkAction->GetID();
								if(kActionID == ACTIONNAME_AWAKE_CHARGE)
								{// 각성 챠지 유지 중일때만 battle idle 액션 상태로 바꾼다
									pkActor->TransitAction(ACTIONNAME_BIDLE);
								}
							}
						}
					}
				}break;
			}

		}break;
	case PT_N_C_ANS_REGIST_PVPLEAGUE_TEAM:
		{
			HRESULT hResult;
			BM::GUID kReqUserGuid;
			rkPacket.Pop(hResult);
			rkPacket.Pop(kReqUserGuid);
			
			XUI::CXUI_Wnd *pkUIWnd = XUIMgr.Get(L"SFRM_REGIST_TEAM");
			if( pkUIWnd )
			{//열려있으면 닫아준다
				pkUIWnd->Close();
			}

			BM::vstring vstrNoticeMessage;
			switch( hResult )
			{
			case E_PVPLEAGUE_REGIST_TEAM_SUCCESS:				
				{ 
					std::wstring wstrTeamName;
					rkPacket.Pop(wstrTeamName);

					vstrNoticeMessage = ::TTW(560018);
					vstrNoticeMessage.Replace(L"#TEAM_NAME#", wstrTeamName);
				}break;
			case E_PVPLEAGUE_REGIST_TEAM_NAME_DUPLICATE:	{ vstrNoticeMessage = ::TTW(560056); }break;
			case E_PVPLEAGUE_REGIST_TEAM_LIMIT_COUNT:			{ vstrNoticeMessage = ::TTW(560057); }break;
			case E_PVPLEAGUE_REGIST_TEAM_WRONG_STATE:		{ vstrNoticeMessage = ::TTW(560058); }break;
			case E_PVPLEAGUE_REGIST_TEAM_MEMBER_DUPLICATE: { vstrNoticeMessage = ::TTW(560060); }break;
			case E_PVPLEAGUE_REGIST_TEAM_ERROR_SYSTEM:
			default:
				{
					vstrNoticeMessage = ::TTW(560059);
				}break;
			}

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				if( kReqUserGuid == pkPlayer->GetID() )
				{//요청자는 메시지 박스
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(vstrNoticeMessage), true);
				}
				else
				{//그외 파티원은 공지 메시지
					::Notice_Show(vstrNoticeMessage, EL_Notice3);
				}
			}
		}break;
	case PT_N_C_ANS_GIVEUP_PVPLEAGUE_TEAM:
		{
			BM::GUID kReqUserGuid;
			bool bIsSuccess = false;
			rkPacket.Pop(kReqUserGuid);
			rkPacket.Pop(bIsSuccess);

			BM::vstring vstrNoticeMessage;
			if( bIsSuccess )
			{//팀 해체 성공
				vstrNoticeMessage = ::TTW(560064);
			}
			else
			{//팀 해체 실패
				vstrNoticeMessage = ::TTW(560065);
			}

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				if( kReqUserGuid == pkPlayer->GetID() )
				{//요청자는 메시지 박스
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(vstrNoticeMessage), true);
				}
				else
				{//그외 파티원은 공지 메시지
					::Notice_Show(vstrNoticeMessage, EL_Notice3);
				}
			}
		}break;
	case PT_N_C_ANS_QUERY_PVPLEAGUE_TEAM:
		{
			bool bIsRegistTeam = false;
			rkPacket.Pop(bIsRegistTeam);

			if( bIsRegistTeam )
			{//팀 등록 했다면 포기 UI가 나오고
				lwActivateUI("SFRM_GIVEUP_TEAM");
			}
			else
			{//팀 등록 안했다면 등록 UI가 나온다
				lwActivateUI("SFRM_REGIST_TEAM");
			}
		}break;
	case PT_N_C_NFY_PVPLEAGUE_NOTICE_MSG:
		{
			int iLeagueLevel = 0;
			EPvPLeagueMsg eMessage;
			rkPacket.Pop(eMessage);
			rkPacket.Pop(iLeagueLevel);
			if( PVPLM_INVITE == eMessage )
			{
				if ( PgContentsBase::ms_pkContents 
					&& PgContentsBase::STATUS_NONE != PgContentsBase::ms_pkContents->GetStatus())
				{//로비/룸/게임중이라면 초대 메시지 처리 안함.
					return false;
				}
			}

			BM::vstring vstrLevel;
			PgPvPGame::Set_PvPLeagueLevelText(iLeagueLevel, vstrLevel);
			lua_tinker::call<void, int, char const*>("Recv_PT_N_C_NFY_PVPLEAGUE_NOTICE_MSG", eMessage, MB(vstrLevel));
		}break;
	case PT_M_C_NFY_PVP_HIDEUNIT:
		{
			if(g_pkWorld)
			{
				VEC_GUID kContPlayGuid, kContHideGuid;
				PU::TLoadArray_A( rkPacket, kContHideGuid );
				PU::TLoadArray_A( rkPacket, kContPlayGuid );

				VEC_GUID::const_iterator hide_itr = kContHideGuid.begin();
				while( kContHideGuid.end() != hide_itr )
				{
					PgActor* pkActor = dynamic_cast< PgActor* >( g_pkWorld->FindObject( *hide_itr ) );
					if( pkActor )
					{
						pkActor->SetHide(true);
					}
					++hide_itr;
				}

				VEC_GUID::const_iterator play_itr = kContPlayGuid.begin();
				while( kContPlayGuid.end() != play_itr )
				{
					PgActor* pkActor = dynamic_cast< PgActor* >( g_pkWorld->FindObject( *play_itr ) );
					if( pkActor )
					{
						pkActor->SetHide(false);
					}
					++play_itr;
				}
			}
		}break;
	case PT_M_C_NFY_CARD_REFRESH_ABIL:
		{
			BM::GUID kCharGuid;
			VEC_CARD_ABIL kContCardAbil;

			rkPacket.Pop( kCharGuid );
			rkPacket.Pop( kContCardAbil );

			PgPilot* pkPilot = g_kPilotMan.FindPilot(kCharGuid);
			if( pkPilot )
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
				if( pkPlayer )
				{
					pkPlayer->SetCardAbil(kContCardAbil);
				}
			}
		}break;
		// JobSkill 채집 관련
	case PT_M_C_NFY_LOCATION_ITEM:
	case PT_M_C_NFY_LOCATION_INFO:
	case PT_M_C_NFY_LOCATION_ACTION:
	case PT_M_C_NFY_COMPLETE_JOBSKILL:
		{
			g_kJobSkillLocationInfo.ProcessMsg(wPacketType, rkPacket);
		}break;
	case PT_M_C_ANS_LEARN_JOBSKILL:
		{
			EJOBSKILL_LEARN_RET eRet = JSLR_ERR;
			rkPacket.Pop(eRet);
			lwJobSkillLearn::ReturnResult(eRet);
		}break;
	case PT_M_C_NFY_JOBSKILL_ERROR:
		{
			EJobSkillMessage eRet = JSEC_NONE;
			rkPacket.Pop( eRet );

			switch( eRet )
			{
			case JSEC_NEED_GUIDE:
				{
					int iSaveIdx = 0;
					rkPacket.Pop( iSaveIdx );
					
					std::wstring kItemName;
					if( JobSkillGuide::GetGuideItemName(iSaveIdx, kItemName) )
					{
						std::wstring kTempMsg = TTW(25004);
						PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"$GUIDE_NAME$", kItemName, kTempMsg);
						::Notice_Show(kTempMsg, EL_Warning, true);
						lwPlaySoundByID( "05.messager" );
					}
				}break;
			case JSEC_NEED_SKILL:
				{
					int iSkillNo = 0;
					rkPacket.Pop( iSkillNo );

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					if( pSkillDef )
					{
						std::wstring kTempMsg = TTW(25002);
						wchar_t const* pSkillName = NULL;
						if( ::GetDefString(pSkillDef->NameNo(),pSkillName) )
						{
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"$SKILL_NAME$", pSkillName, kTempMsg);
							::Notice_Show(kTempMsg, EL_Warning, true);
							lwPlaySoundByID( "05.messager" );
						}
					}
				}break;
			case JSEC_NEED_SKILL_EXPERTNESS:
				{
					int iSkillNo = 0, iExpertNess = 0;

					rkPacket.Pop( iSkillNo );
					rkPacket.Pop( iExpertNess );

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					if( pSkillDef )
					{
						std::wstring kTempMsg = TTW(25008);
						wchar_t const* pSkillName = NULL;
						if( ::GetDefString(pSkillDef->NameNo(),pSkillName) )
						{
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"$SKILL_NAME$", pSkillName, kTempMsg);
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"$EXPERTNESS_VALUE$", BM::vstring(iExpertNess/JSE_EXPERTNESS_DEVIDE), kTempMsg);
							::Notice_Show(kTempMsg, EL_Warning, true);
						}
					}
				}break;
			case JSEC_WRONG_SKILL_LOCATION:
				{
					int iWrongSkillNo = 0, iCorrectSkillNo = 0;
					rkPacket.Pop( iWrongSkillNo );
					rkPacket.Pop( iCorrectSkillNo );

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pWrongSkillDef = kSkillDefMgr.GetDef(iWrongSkillNo);
					CSkillDef const* pCorrectSkillDef = kSkillDefMgr.GetDef(iCorrectSkillNo);
					if( pWrongSkillDef && pCorrectSkillDef)
					{
						std::wstring kTempMsg = TTW(25016);
						wchar_t const* pWrongSkillName = NULL;
						wchar_t const* pCorrectSkillName = NULL;
						if( ::GetDefString(pWrongSkillDef->NameNo(), pWrongSkillName) 
							&& ::GetDefString(pCorrectSkillDef->NameNo(), pCorrectSkillName) )
						{
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NOWSKILL#", pWrongSkillName, kTempMsg);
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#USESKILL#", pCorrectSkillName, kTempMsg);
							::Notice_Show(kTempMsg, EL_Warning, true);
							lwPlaySoundByID( "05.messager" );
						}
					}
				}break;
			case JSEC_SUCCEED_GET_ITEM:
				{
					int iItemNo = 0;
					rkPacket.Pop( iItemNo );
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
					if(pItemDef)
					{
						std::wstring kTempMsg = TTW(25020);
						wchar_t const* pItemName = NULL;
						if( ::GetDefString(pItemDef->NameNo(), pItemName) )
						{
							PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#ITEM#", pItemName, kTempMsg);
							::Notice_Show(kTempMsg, EL_Warning, true);
						}
					}
				}break;
			case JSEC_NEED_GUIDE_FAIL_LOCATION:
				{
					int iSaveIdx = 0;
					rkPacket.Pop( iSaveIdx );

					std::wstring kItemName;
					if( JobSkillGuide::GetGuideItemName(iSaveIdx, kItemName) )
					{
						std::wstring kTempMsg = TTW(25023);
						PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"$GUIDE_NAME$", kItemName, kTempMsg);
						::Notice_Show(kTempMsg, EL_Warning, true);
					}
				}break;
			case JSEC_NEED_CHAR_LEVEL:
				{
					int iSkillNo = 0;
					rkPacket.Pop( iSkillNo );
					PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
					if(pPlayer)
					{
						PgJobSkillExpertness const& rkExpertness =  pPlayer->JobSkillExpertness();
						CONT_DEF_JOBSKILL_SKILLEXPERTNESS::value_type rkNextExpertness;
						int const iNexExpertness_Min = JobSkillExpertnessUtil::FindNextExpertness_Min(iSkillNo, rkExpertness.Get(iSkillNo));
						bool const bRet = JobSkillExpertnessUtil::FindExpertnessArea(iSkillNo, iNexExpertness_Min, rkNextExpertness);
						if(bRet)
						{
							BM::vstring kErrMsg( TTW(25015));
							kErrMsg.Replace(L"#NUM#", iNexExpertness_Min / ABILITY_RATE_VALUE);
							kErrMsg.Replace(L"#LEVEL#", rkNextExpertness.iNeed_CharLevel);
							::Notice_Show( static_cast<std::wstring>(kErrMsg), EL_Warning, true);
						}
					}
				}break;
			case JSEC_FULL_EXPERTNESS:
				{
					int iSkillNo = 0;
					rkPacket.Pop( iSkillNo );
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					wchar_t const* pSkillName = NULL;
					if( ::GetDefString(pSkillDef->NameNo(), pSkillName) )
					{
						BM::vstring kErrMsg( TTW(25024));
						kErrMsg.Replace(L"#SKILL#", pSkillName);
						::Notice_Show( static_cast<std::wstring>(kErrMsg), EL_Normal, true);
					}
				}break;
			case JSEC_NEED_USE_RATE_EXHAUSTION:
			case JSEC_NEED_USE_RATE_PRODUCTPOINT:
				{
					int const iBaseValue = 100;
					int iRate = 0;
					rkPacket.Pop( iRate );
					iRate = iBaseValue - iRate / iBaseValue;
					BM::vstring kErrMsg( TTW(25026) );
					kErrMsg.Replace( L"$RATE_EXHAUSTION$", BM::vstring(iRate) );
					::Notice_Show( kErrMsg.operator std::wstring const&(), EL_Warning, true);
				}break;
			case JSEC_NEED_USE_RATE_BLESSPOINT:
				{
					int const iBaseValue = 100;
					int iRate = 0;
					rkPacket.Pop( iRate );
					iRate = iBaseValue - iRate / iBaseValue;
					BM::vstring kErrMsg( TTW(25027) );
					kErrMsg.Replace( L"$RATE_BLESSPOINT$", BM::vstring(iRate) );
					::Notice_Show( kErrMsg.operator std::wstring const&(), EL_Warning, true);
				}break;
			default:
				{
				}break;
			}
		}break;
	case PT_M_C_NFY_JOBSKILL3_HISTORYITEM:
		{
			lwJobSkillItem::RecvContJobSkillHistoryItem(rkPacket);
		}break;
	case PT_M_C_ANS_JOBSKILL3_CREATEITEM:
		{
			lwJobSkillItem::JS3_RecvCreateItem(rkPacket);
		}break;
	case PT_M_C_NFY_INTERACTIVE_EMOTION_REQUEST:
		{// 인터렉티브 이모션을 요청받았으므로
			int iActionNo = 0;
			rkPacket.Pop(iActionNo);
			if(iActionNo)
			{// 어떤 액션인지 확인 해두고,
				BM::GUID kReqPlayerGUID;
				rkPacket.Pop(kReqPlayerGUID);

				BM::GUID kMyPlayerGUID;
				if(g_kPilotMan.GetPlayerPilotGuid(kMyPlayerGUID)
					&& kMyPlayerGUID != kReqPlayerGUID
					)
				{
					PgActor* pkReqActor = g_kPilotMan.FindActor(kReqPlayerGUID);
					PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
					if(pkReqActor 
						&& pkMyActor
						)
					{
						lua_tinker::call<void>("Set_InteractiveEmotion_TargetActor", lwActor(pkReqActor), lwGUID(kMyPlayerGUID), true);	// 상호 이모션 요청자
						lua_tinker::call<void>("Set_InteractiveEmotion_TargetActor", lwActor(pkMyActor), lwGUID(kReqPlayerGUID), false); // 상호 이모션 요청받은자
						BM::Stream kPacket(PT_C_M_ANS_INTERACTIVE_EMOTION_REQUEST);
						kPacket.Push(true);				// 이모션 허락
						kPacket.Push(iActionNo);		// 이모션 스킬 번호
						kPacket.Push(kReqPlayerGUID);	// 신청한 플레이어의 GUID
						kPacket.Push(kMyPlayerGUID);	// 자신의 PlayerGUID(대상이 내가 되므로)

						BM::vstring vStr(TTW(790901));
						{
							PgPilot* pkReqPilot = g_kPilotMan.FindPilot(kReqPlayerGUID);
							if(pkReqPilot)
							{// 요청한 플레이어의 이름과
								vStr.Replace(L"#REQ#", pkReqPilot->GetName());
								{// 액션 이름을 UI에 표시해
									GET_DEF(CSkillDefMgr, kSkillDefMgr);
									CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iActionNo);
									if(pkSkillDef)
									{
										wchar_t const* pwcName = NULL;
										GetDefString(pkSkillDef->NameNo(), pwcName);
										if(pwcName)
										{
											vStr.Replace(L"#ACT#", std::wstring(pwcName));
										}
									}
								}
							}
						}
						// 이모션을 할것인지 의사를 물어본다
						lwCallCommonMsgYesNoBox(static_cast<std::string>(vStr).c_str(), lwPacket(&kPacket), true, MBT_INTERACTIVE_EMOTION_REQUEST_ACCEPT);
					}
				}
			}
		}break;
	case PT_M_C_NFY_DO_INTERACTIVE_EMOTION:
		{
			bool bAnswer = false;
			rkPacket.Pop(bAnswer);
			if(bAnswer)
			{// 상대가 인터렉티브 이모션을 한다고 수락 했기 때문에
				if(lwIsVisible("SFRM_MSG_COMMON"))
				{// 팝업이 아직 떠있다면 창을 닫고
					XUIMgr.Close(L"SFRM_MSG_COMMON");
				}

				int iActionNo = 0;
				rkPacket.Pop(iActionNo);

				BM::GUID kTargetPlayerGUID;
				rkPacket.Pop(kTargetPlayerGUID);	// 현재 따로 쓰진 않음

				PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
				if(iActionNo && pkMyActor)
				{// 액션을 진행 한다
					//{// 바로 액션 전환
					//	lwAction klwAction(pkMyActor->GetAction());
					//	klwAction.ChangeToNextActionOnNextUpdate(true,true);
					//}
					GET_DEF(CSkillDefMgr, kSkillDefMgr); 
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iActionNo);
					BM::vstring kActionName(pkSkillDef->GetActionName());
					pkMyActor->TransitAction(static_cast<std::string const>(kActionName).c_str());
				}
			}
			else
			{// 상대가 거절 했다면, 팝업 메세지로 알려준다
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790903, true);
			}
		}break;
	case PT_M_C_GMCMD_GODHAND_JOBSKILL_SUBTOOL_ON:
		{
			bool bRet = false;
			std::wstring kLog =TTW( 99402 );
			if( 0 != kLog.size() )
			{
				SChatLog kChatLog( CT_EVENT_GAME );
				g_kChatMgrClient.AddLogMessage( kChatLog, kLog );
			}
		}break;
	case PT_M_C_GMCMD_GODHAND_JOBSKILL_SUBTOOL_OFF:
		{
			bool bRet = false;
			std::wstring kLog =TTW( 99403 );
			if( 0 != kLog.size() )
			{
				SChatLog kChatLog( CT_EVENT_GAME );
				g_kChatMgrClient.AddLogMessage( kChatLog, kLog );
			}
		}break;
	case PT_M_C_ANS_SOULTRANSFER_EXTRACT:
	case PT_M_C_ANS_SOULTRANSFER_TRANSITION:
		{
			HRESULT hRes;
			ESoulTransMsg eMsg;
			rkPacket.Pop(hRes);
			rkPacket.Pop(eMsg);
			int iResultItemNo = 0;
			if( S_OK == hRes )
			{
				rkPacket.Pop(iResultItemNo);
				lwPlaySoundByID("EnchantSuccess");
				lwSoulTransfer::ResetUI();
				if( lwSoulTransfer::IsSoulInsertUI() )
				{
					lua_tinker::call<void,int>("PreChangeInvViewGroupTop", 2);
				}
			}
			else
			{
				lwPlaySoundByID("EnchantFail");
				if( lwSoulTransfer::IsSoulInsertUI() )
				{
					lwSoulTransfer::lwSetAdditionItem();
				}
			}
			lwSoulTransfer::CallSoulTransMessage(eMsg, iResultItemNo);
		}break;
	case PT_M_C_ANS_ELEMENT_EXTRACT:
		{
			HRESULT hRes;
			int iResultItemNo = 0;
			int iResultItemCount = 0;
			rkPacket.Pop(hRes);
			rkPacket.Pop(iResultItemNo);
			rkPacket.Pop(iResultItemCount);

			BM::vstring kMsg(TTW(792012));
			wchar_t const* pkItemName = NULL;
			GetDefString(iResultItemNo, pkItemName);
			if( 0 != iResultItemNo && pkItemName)
			{
				kMsg.Replace(L"#ITEM#", pkItemName);
			}
			kMsg.Replace(L"#NUM#", iResultItemCount);	
			::Notice_Show( kMsg, EL_Normal );

		}break;
	case PT_M_C_NFY_END_ACTION:
		{
			BM::GUID kGuid;
			int iEndActionNo = 0;
			rkPacket.Pop(kGuid);
			rkPacket.Pop(iEndActionNo);
			PgActor* pkActor = g_kPilotMan.FindActor(kGuid);
			if(!pkActor)
			{
				break;
			}
			PgAction* pkAction = pkActor->GetAction();
			if(!pkAction)
			{
				break;
			}
			int const iParentSkillNo = pkAction->GetAbil(AT_PARENT_SKILL_NUM);
			if(iParentSkillNo != iEndActionNo)
			{
				break;
			}
			CSkillDef const* pkSkillDef = pkAction->GetSkillDef();
			if(!pkSkillDef)
			{
				break;
			}
			int const iActionNo = pkAction->GetActionNo();
			switch(pkSkillDef->GetType())
			{
			case EST_TOGGLE:
				{// 토글일 경우
					pkActor->ActionToggleStateChange(iActionNo, false);
					pkActor->StartSkillCoolTime(iActionNo);
					int const iLastAniSlot = pkAction->GetSlotCount()-1;
					int const iCurSlot = pkAction->GetCurrentSlot();
					if(iCurSlot == iLastAniSlot)
					{// 마무리 애니는 없거나
						pkAction->SetParam(1, "end");
					}
					else
					{// 무조건 한개 애니만 허용함
						pkAction->SetSlot(iLastAniSlot);
						pkActor->PlayCurrentSlot();
					}
				}break;
			default:
				{
					pkAction->SetParam(1, "end");
				}break;
			}
		}break;
	case PT_S_C_ANS_FIND_CHARACTOR_EXTEND_SLOT:
		{
			BYTE byExtendSlot = 0;
			rkPacket.Pop(byExtendSlot);

			int const iUseExtendSlot = static_cast<int>(byExtendSlot);
			lua_tinker::call<void, int>("SetLockedExtendSlot", iUseExtendSlot );
			g_kSelectStage.ExtendSlot(iUseExtendSlot);
		}break;
	case PT_T_C_ANS_ITEM_USE_RESULT:
		{
			int iResult = 0;
			rkPacket.Pop(iResult);
			ItemUseResultMsg(iResult);
		}break;
	case PT_M_C_REQ_DUEL_PVP:
		{ //딴놈이 나에게 맞짱 뜨자고 함
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_REQ_DUEL_PVP", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_ANS_DUEL_PVP:
		{ //내가 맞짱 걸어서 대답이 왔음
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_ANS_DUEL_PVP", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_ERROR_DUEL_PVP:
		{ //맞짱 건 놈이 맞짱을 철회함
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_ERROR_DUEL_PVP", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_NFY_DUEL_START_COUNT:
		{ //맞짱 시작 카운트를 표시해주자
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_NFY_DUEL_START_COUNT", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_NFY_DUEL_END_COUNT:
		{
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_NFY_DUEL_END_COUNT", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_NFY_DUEL_OUT_COUNT:
		{
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_NFY_DUEL_OUT_COUNT", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_NFY_DUEL_CANCEL_OUT_COUNT:
		{
			lua_tinker::call<void, char*>("CancelDuelCountDown", "FRM_DUEL_AREAOUT_COUNTDOWN" );
		}break;
	case PT_M_C_NFY_DUEL_BEGIN:
		{
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_NFY_DUEL_BEGIN", lwPacket(&rkPacket) );
		}break;
	case PT_M_C_NFY_DUEL_FINISH:
		{
			lua_tinker::call<void, lwPacket>("Net_PT_M_C_NFY_DUEL_FINISH", lwPacket(&rkPacket) );
		}break;

	case PT_M_C_TRY_LOGIN_CHANNELMAPMOVE:
		{
			SChannelMapMove MoveInfo;
			rkPacket.Pop(MoveInfo);

			g_kNetwork.TryRealmNo(g_kNetwork.NowRealmNo());
			g_kNetwork.TryChannelNo(MoveInfo.iChannelNo);

			CONT_CHANNEL::mapped_type	ChannelInfo;
			if( S_OK == g_kRealmMgr.GetChannel(g_kNetwork.TryRealmNo(), MoveInfo.iChannelNo, ChannelInfo) )
			{
				if( !ChannelInfo.IsAlive() )
				{
					// 채널에 접속 할 수 없음.
					Notice_Show( TTW(18044), EL_Warning );
					break;
				}
			}

			BM::Stream Packet(PT_C_L_TRY_LOGIN_CHANNELMAPMOVE);
			Packet.Push(true);
			Packet.Push(g_kNetwork.NowRealmNo());	// 렐름만 보내고 채널, 맵은 서버에서 정해주는데로 이동.

			NETWORK_SEND_TO_SWITCH(Packet);
		}
	case PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT:		// 다른 유저가 이벤트 스크립트를 보고 있기 때문에 인풋 락.
		{
			Recv_PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT(rkPacket);
		}break;
	case PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT:		// 모든 유저가 이벤트 스크립트를 다봐서 인풋 락해제.
		{
			Recv_PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT(rkPacket);
		}break;
	case PT_M_C_ANS_COLLECT_ANTIQUE:
	case PT_M_C_ANS_EXCHANGE_GEMSTORE:
	case PT_M_C_NFY_OPEN_DEFENCE_WINDOW:
	case PT_M_C_NFY_ENTER_DEFENCE:
	case PT_M_C_NFY_CANCLE_DEFENCE:
	case PT_M_C_NFY_USE_POINT_COPY_ITEM:
	case PT_M_C_NFY_ACCUMPOINT_TO_STRATEGICPOINT:
	case PT_M_C_NFY_CURRENT_ACCUMPOINT:
	case PT_M_C_NFY_KILL_MONSTER_BY_ACCUMPOINT:
		{
			lwDefenceMode::RecvDefenceMode_Command(wPacketType, rkPacket);
		}break;
	case PT_M_C_DO_COMBAT_REVIVE_ACTION:
		{
			PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
			if(pkMyActor)
			{
				PgAction* pkAction = pkMyActor->GetAction();
				if( pkAction )
				{
					lwAction kAction(pkAction);
					kAction.SetNextActionName( "a_combat_revive_act" );
					kAction.ChangeToNextActionOnNextUpdate( true,true );
				}
			}
		}break;
	case PT_M_C_NFY_CHANGE_GUARDIAN_INSTALL_DICOUNT:
		{
			Recv_PT_M_C_NFY_CHANGE_GUARDIAN_INSTALL_DICOUNT(rkPacket);
		}break;
	case PT_M_C_ANS_INDUN_PARTY_LIST:
		{
			BM::GUID kOwnerGuid;
			int Type = 0;
			CONT_INDUN_PARTY_CHANNEL kList;
			rkPacket.Pop(kOwnerGuid);
			rkPacket.Pop(Type);
			PU::TLoadTableArray_AMM(rkPacket, kList);
			
			switch( Type )
			{
			case 1:
				{// 일반 던전
					OnRefresh_DungeonPartyList(kOwnerGuid, kList);
				}break;
			case 2:
				{// 별자리 던전
					g_kConstellationEnterUIMgr.OnRefresh_DungeonPartyList(kOwnerGuid, kList);
				}break;
			}
		}break;
	case PT_M_C_NFY_ALARM_MINIMAP:
	case PT_M_C_NFY_TRIGGER_ONENTER:
	case PT_M_C_NFY_TRIGGER_ONLEAVE:
		{
			PgMiniMapUtil::ProcessMsg(wPacketType, rkPacket);
		}break;
	case PT_M_C_NFY_LEARN_SKILL_EVENT:
		{
/*			int iSkillEvent = 0;
			rkPacket.Pop(iSkillEvent);
			if(iSkillEvent == ELSE_MANUFACTURE_HELP)
			{
				XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"BTN_ACHIEVEMENT_MINI");
				if( pkWnd )
				{
					//스킬 습득시 도움말 팝업은 일단 제외시키자
					//DWORD const dwAliveTime = 30000;
					//pkWnd->AliveTime(dwAliveTime);
					//pkWnd->SetInvalidate();
					//g_kHelpSystem.ActivateByCondition( std::string(HELP_CONDITION_ITEMNO), 60000630 );
				}
			}*/
		}break;
	case PT_M_C_NFY_RESULT_REGIST_CHAT_BLOCK:
		{
			bool Result;
			BYTE BlockType = 0;
			BM::GUID CharGuid;
			std::wstring Name;

			rkPacket.Pop(Name);
			rkPacket.Pop(BlockType);
			rkPacket.Pop(Result);

			if( true == Result )
			{
				g_kChatMgrClient.AddChatBlockList(Name, BlockType);

				BM::vstring vText(TTW(404039));
				vText.Replace(L"#Name#", Name);
				SChatLog ChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddLogMessage(ChatLog, vText, true, 1);
			}
			else
			{
				g_kChatMgrClient.RemoveChatBlockList(Name);				

				BM::vstring vText(TTW(404029));
				vText.Replace(L"#Name#", Name);
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddLogMessage(kChatLog, vText, true, 1);
			}

			XUI::CXUI_Wnd * pCommunityWindow = XUIMgr.Get(L"SFRM_COMMUNITY");
			if( pCommunityWindow )
			{
				g_kChatMgrClient.UpdateChatBlockList(pCommunityWindow->GetControl(L"FRM_BLOCK"));
			}			
		}break;
	case PT_M_C_ANS_DELETE_DEBUFF_SUCCESS:
		{
			int DebuffNo = 0;
			int DeleteCount = 0;

			rkPacket.Pop(DebuffNo);
			rkPacket.Pop(DeleteCount);

			if( 0 < DeleteCount )
			{
				BM::vstring vStr;
				std::wstring const * EffectName = NULL;
				if( GetDefString(DebuffNo, EffectName) )
				{
					if( 1 == DeleteCount )
					{
						vStr = TTW(310104);
						vStr.Replace(L"#EFFECT#", *EffectName);
					}
					else
					{
						vStr = TTW(310105);
						vStr.Replace(L"#EFFECT#", *EffectName);
						vStr.Replace(L"#NUM#", DeleteCount - 1);
					}
					SChatLog ChatLog(CT_EVENT_SYSTEM);
					g_kChatMgrClient.AddLogMessage(ChatLog, (std::wstring const&)vStr, true);
				}
			}
			else
			{
				if( 0 == DeleteCount )
				{
					//BM::vstring vStr(TTW(310106));
					//SChatLog ChatLog(CT_EVENT);
					//g_kChatMgrClient.AddLogMessage(ChatLog, (std::wstring const&)vStr, true);
				}
			}
		}break;
	case PT_M_C_ANS_CHECK_CAN_KICK:
		{
			BM::vstring Type;
			BM::GUID CharGuid;
			bool bAbleKick = false;

			rkPacket.Pop(Type);
			rkPacket.Pop(CharGuid);
			rkPacket.Pop(bAbleKick);

			if( bAbleKick )
			{
				if( PgClientExpeditionUtil::IsInExpedition() )
				{ // 현재 원정대에 속해 있는가?
					return false;
				}

				if( Type == "KICKOUT" )
				{
					BM::Stream Packet(PT_C_N_REQ_KICKOUT_PARTY_USER);
					Packet.Push((BYTE)PCT_KICK_CHARGUID);
					Packet.Push(CharGuid);
					NETWORK_SEND(Packet)
				}
				else if( Type == "LEAVE" )
				{
					BM::Stream Packet(PT_C_N_REQ_LEAVE_PARTY, true);
					NETWORK_SEND(Packet);
				}
			}
		}break;
	default:
		{
			if( g_kParty.ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( g_kExpedition.ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( g_kExpeditionComplete.ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( lwPgEmporiaAdministrator::RecvEmporia_AdminPacket(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( lwHomeUI::RecvMyHome_Packet(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( g_kHomeRenewMgr.ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( g_kMissionMgr.ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( lwCommunityEvent::ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if ( RACEEVENT::ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			else if( lwConstellationEnterUIUtil::ProcessMsg(wPacketType, rkPacket) )
			{
				return true;
			}
			return false;//처리 안되면 리턴 펄스
		}break;
	}
	return true;
}

void Recv_PT_S_C_REQ_PING(BM::Stream * const pkPacket)
{//스위치용 핑 응답.
	BM::GUID kGUID;
	pkPacket->Pop(kGUID);

	DWORD const dwTime = BM::GetTime32();//timegettime 으로 바꾸면, 시간이 5ms 내외에서 뒤로 돌아가거나 할 수도 있으니 주의.
	
	::GUID kTempGuid = kGUID;
	kTempGuid.Data1 ^= dwTime;//시간값 XOR 로 줌.

	BM::Stream kResPacket(PT_C_S_ANS_ONLY_PING, kTempGuid);
	
	g_kNetwork.SendToSwitch(kResPacket); 
}

bool Recv_PT_X_X_ANS_ERROR_CHECK(HRESULT const Result)
{
	int iErrorTextTableNo = 0;
	bool bSystemMsg = false;
	switch( Result )
	{
	case EC_OK:
		{
			return true;
		}
	case E_REPAIR_NOT_FOUND_ITEM:	{ iErrorTextTableNo = 2851;		}break;
	case E_REPAIR_NOT_MATCH_LEVEL:	{ iErrorTextTableNo = 22027;	}break;
	case E_REPAIR_NEED_NOT_REPAIR:	{ iErrorTextTableNo = 1220;		}break;
	case E_REPAIR_CANT_REPAIR:		{ iErrorTextTableNo = 1219;		}break;
	case E_ENCHANT_NOT_FOUND_ITEM:	{ iErrorTextTableNo = 2851;		}break;
	case E_ENCHANT_REQUIRE_EQUIP_LIMIT:		{ iErrorTextTableNo = 3200;		}break;
	case E_ENCHANT_REQUIRE_PLUS_LV_LIMIT:	{ iErrorTextTableNo = 3201;		}break;
	case E_ENCHANT_CANT_UPGRADE:	{ iErrorTextTableNo = 1248;		}break;
	case E_REDICE_NOT_FOUND_ITEM:	{ iErrorTextTableNo = 5811;		}break;
	case E_REDICE_NOT_ENOUGH_ITEM:	{ iErrorTextTableNo = 5812;		}break;
	case E_REDICE_NOT_EQUIP_ITEM:	{ iErrorTextTableNo = 5813;		}break;
	case E_SKILLEXTEND_INVALID_EXTEND:		{ iErrorTextTableNo = 790440; bSystemMsg=true;	}break;
	case E_SKILLEXTEND_CANT_EXTEND_LEVEL:	{ iErrorTextTableNo = 790441; bSystemMsg=true;	}break;
	case E_SKILLEXTEND_INVALID_SKILL:		{ iErrorTextTableNo = 790442; bSystemMsg=true;	}break;
	case E_SKILLEXTEND_INVALID_CLASSLIMIT:	{ iErrorTextTableNo = 790443; bSystemMsg=true;	}break;
	default:
		{//선언되지 않은 에러
			iErrorTextTableNo = 790413;
		}break;
	}

	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iErrorTextTableNo, true);

	if(bSystemMsg)
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(iErrorTextTableNo, kChatLog, false, EL_Warning);
	}
	return false;
}

void Send_PT_C_S_NFY_UNIT_POS(BM::Stream &kInPacket)
{
	BM::GUID kGUID;

	kInPacket.Pop(kGUID);

	BM::Stream kResPacket(PT_C_S_NFY_UNIT_POS, kGUID);
	
	PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
	POINT3 ptPos(0, 0, 0);
	if (pkMyActor == NULL)
	{	// Map입장전이다.
		ptPos.x = 0; ptPos.y= 0; ptPos.z = 0;
		return;//입장전엔 안보낸다.
	}
	else
	{// 플레이어의 위치값을 얻어오고
		ptPos = CheckCurPosOn_PT_C_S_NFY_UNIT_POS(pkMyActor);
	}
	kResPacket.Push(ptPos);
	
	PgActor* pkSubActor = PgActorUtil::GetSubPlayerActor( pkMyActor );
	if(pkSubActor)
	{// SC가 존재하면, 함께 위치 정보를 갱신 시키고
		ptPos = CheckCurPosOn_PT_C_S_NFY_UNIT_POS(pkSubActor);
		kResPacket.Push(true);
		kResPacket.Push(ptPos);
	}
	{NETWORK_SEND_TO_SWITCH(kResPacket);}
}

void Set_PvPSelectorUI( XUI::CXUI_Wnd *pkUI, __int64 const i64NowTimeInDay, int const iID )
{
	if ( pkUI )
	{
		CONT_DEF_PLAYLIMIT_INFO const *pkPlayTimeInfo = NULL;
		g_kTblDataMgr.GetContDef( pkPlayTimeInfo );

		CONT_DEF_PLAYLIMIT_INFO::mapped_type::value_type kTempInfo;
		SYSTEMTIME kTempTime[2];
		::memset( kTempTime, 0, sizeof(kTempTime) );

		wchar_t wszTemp[MAX_PATH] = {0,};

		CONT_DEF_PLAYLIMIT_INFO::const_iterator itr = pkPlayTimeInfo->find( iID );
		if ( itr != pkPlayTimeInfo->end() )
		{
			PgPlayLimit_Finder kFinder( itr->second );
			bool const bIsOpen = ( S_OK == kFinder.Find( i64NowTimeInDay, kTempInfo ) );

			pkUI->SetCustomData( &(kTempInfo.nLevelLimit), sizeof(kTempInfo.nLevelLimit) );
			pkUI->Enable( bIsOpen );

			CGameTime::SecTime2SystemTime( kTempInfo.i64BeginTime, kTempTime[0], CGameTime::DEFAULT );
			CGameTime::SecTime2SystemTime( kTempInfo.i64EndTime, kTempTime[1], CGameTime::DEFAULT );
			if ( 0 == kTempTime[1].wHour )
			{
				kTempTime[1].wHour = 24;
			}

			::swprintf_s( wszTemp, MAX_PATH, L"[%02d:%02d ~ %02d:%02d]", kTempTime[0].wHour, kTempTime[0].wMinute, kTempTime[1].wHour, kTempTime[1].wMinute );

			XUI::CXUI_Wnd *pkTemp  = pkUI->GetControl( L"FRM_LEVELLIMIT" );
			if ( pkTemp )
			{
				BM::vstring vstr( TTW(400290) );
				vstr.Replace( L"#LEVEL#", kTempInfo.nLevelLimit );
				pkTemp->Text( static_cast<std::wstring>(vstr) );
			}

			if( PvP_Lobby_GroundNo_League == iID )
			{
				XUI::CXUI_Wnd *pkWndScedule  = pkUI->GetControl( L"FRM_SCEDULE" );
				if( pkWndScedule )
				{
					pkWndScedule->Visible(true);
					Set_PvPLeagueTimeUI(pkWndScedule);
				}
			}
		}
		else
		{
			pkUI->ClearCustomData();
			pkUI->Enable(false);
			::swprintf_s( wszTemp, MAX_PATH, L"%s", TTW(71061).c_str() );

			XUI::CXUI_Wnd *pkTemp  = pkUI->GetControl( L"FRM_LEVELLIMIT" );
			if ( pkTemp )
			{
				pkTemp->Text( std::wstring() );
			}

			if( PvP_Lobby_GroundNo_League == iID )
			{
				XUI::CXUI_Wnd *pkWndScedule  = pkUI->GetControl( L"FRM_SCEDULE" );
				if( pkWndScedule )
				{
					pkWndScedule->Visible(false);
				}
			}
		}

		XUI::CXUI_Wnd *pkTemp = pkUI->GetControl( L"FRM_TIME" );
		if ( pkTemp )
		{
			pkTemp->Text( std::wstring(wszTemp) );
		}
	}
}

void DirectJoin_PvPLeagueLobby(XUI::CXUI_Wnd * pkWnd)
{
	if( pkWnd )
	{
		pkWnd->Close();
		BM::Stream kPacket(PT_C_M_REQ_JOIN_LOBBY);
		kPacket.Push(PvP_Lobby_GroundNo_League);
		NETWORK_SEND(kPacket);
	}
}

bool Set_PvPLeagueTimeUI(XUI::CXUI_Wnd * pkWnd)
{
	int const MAX_LEAGUE_LEVEL = 8;
	const CONT_DEF_PVPLEAGUE_TIME* pkContDefPvPLeagueTime = NULL;
	g_kTblDataMgr.GetContDef(pkContDefPvPLeagueTime);
	if( pkWnd && pkContDefPvPLeagueTime )
	{
		SYSTEMTIME kNowDateTime;
		::GetLocalTime( &kNowDateTime );

		BM::vstring vstrTodayScedule;
		CONT_DEF_PVPLEAGUE_TIME::const_iterator time_itor = pkContDefPvPLeagueTime->begin();
		while( pkContDefPvPLeagueTime->end() != time_itor )
		{
			if( 0 != time_itor->i64EndTime && 0 != time_itor->i64BeginTime
				&& time_itor->iDayOfWeek == kNowDateTime.wDayOfWeek )
			{
				SYSTEMTIME kBeginTime, kEndTime;
				CGameTime::SecTime2SystemTime(time_itor->i64BeginTime, kBeginTime, CGameTime::DEFAULT );
				CGameTime::SecTime2SystemTime(time_itor->i64EndTime, kEndTime, CGameTime::DEFAULT );

				wchar_t wszTemp[MAX_PATH] = {0,};
				::swprintf_s( wszTemp, MAX_PATH, L"[%02d:%02d ~ %02d:%02d]", kBeginTime.wHour, kBeginTime.wMinute, kEndTime.wHour, kEndTime.wMinute );

				switch( time_itor->iLeagueState )
				{
				case PVPLS_JOIN:
					{
						vstrTodayScedule += ::TTW(560015);
					}break;
				case PVPLS_GAME:
					{
						int const iResultLevel = ::pow(2.0f, static_cast<long>(MAX_LEAGUE_LEVEL - (time_itor->iLeagueLevel - 1)));
						vstrTodayScedule += ::TTW(560061);
						vstrTodayScedule.Replace(L"#LEAGUE_LEVEL#",iResultLevel);
					}break;
				}
				vstrTodayScedule += L" : ";
				vstrTodayScedule += wszTemp;
				vstrTodayScedule += L"\n";
			}
			++time_itor;
		}
		pkWnd->Text(vstrTodayScedule);
	}

	return false;
}

void ItemUseResultMsg( int const iMsg )
{
	switch(iMsg)
	{
	case E_CANNOT_EXTEND_CHARACTER_NUM:
		{
			::Notice_Show( TTW(791519), EL_Warning );
		}break;
	}
}