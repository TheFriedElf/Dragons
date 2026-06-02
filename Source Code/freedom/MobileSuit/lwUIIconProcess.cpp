#include "StdAfx.h"
#include "Variant/PgEventView.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "lwUI.h"
#include "lwGuildUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgAction.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "variant/PgJobSkillTool.h"
#include "variant/PgJobSkill.h"
#include "PgQuest.h"
#include "PgRemoteManager.h"
#include "PgNifMan.h"
#include "Variant/PgClassDefMgr.h"
//#include "PgClientParty.h"
#include "PgRenderMan.h"
#include "PgSkillTree.h"
#include "PgQuestMan.h"
#include "Variant/Inventory.H"
#include "PgOption.h"
#include "lwUIMission.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIItemRarityUpgrade.h"
#include "lwUIItemRepair.h"
#include "lwUIItemMaking.h"
#include "lwUIItemCooking.h"
#include "lwUIItemLottery.h"
#include "variant/PgStore.h"
#include "PgUICalculator.h"
#include "lohengrin/packettype.h"
#include "lwTrade.h"
#include "lwCouple.h"
#include "PgMail.h"
#include "PgMarket.h"
#include "PgMarketDef.h"
#include "lwWorld.h"
#include "PgSystemInventory.h"
#include "PgChatMgrClient.h"
#include "lwTeleCardUI.h"
#include "lwUICharInfo.h"
#include "PgMyActorViewMgr.h"
#include "PgSafeFoamMgr.h"
#include "PgGuild.h"
#include "lwCharacterCard.h"
#include "lwUIItemBind.h"
#include "lwUIMemTransCard.h"
#include "lwCashItem.h"
#include "lwUIItemSocketSystem.h"
#include "lwUIItemJewelBox.h"
#include "lwUIItemGamble.h"
#include "lwUIItemEventBox.h"
#include "lwUIItemConvertSystem.h"
#include "lwUISealProcess.h" 
#include "lwUIPet.h"
#include "PgEqItemMixCreator.h"
#include "PgItemMix_Script.h"
#include "PgHome.h"
#include "PgHouse.h"
#include "lwUIActiveStatus.h"
#include "PgItemUseChecker.h"
#include "lwMarket.h"
#include "HardCoreDungeon.h"
#include "lwUIItemRarityBuildUp.h"
#include "lwSkillSet.h"
#include "lwUIItemRarityAmplify.h"
#include "PgSuperGroundUI.h"
#include "AbilTextHelper.h"
#include "lwVendor.h"
#include "lwInventoryUI.h"
#include "PgTrigger.h"
#include "PgJobSkillLocationInfo.h"
#include "Variant/ItemSkillUtil.h"
#include "lwJobSkillItem.h"
#include "lwUITreasureChest.h"
#include "lwSoulTransfer.h"
#include "Variant/PgSocketFormula.h"
#include "lwCommonSkillUtilFunc.h"
#include "lwUIManufacture.h"
#include "Variant/Global.h"
#include "lwUIGemStore.h"
#include "lwDefenceMode.h"

extern void DoItemSell(const SItemPos &kCasterPos, PgBase_Item const *pkItem, bool const bIsStockShop = false, int const iBasePrice = 0);
extern bool lwGetHaveSkill(lwUnit kUnit, int iSkillNo, bool const bOverSkill = false);
extern PgPet* GetMySelectedPet();

namespace lwUIWndUtil
{
	void GetSkillSetCoolTimeInfo(lwActor klwActor, int const iSetNo, XUI::CXUI_Icon* pParent, float& fMaxRemainTime, float& fMaxTotalTime);
};

namespace PgPetUIUtil
{
	void lwUsePetTrainItem(int iIndex);
	bool IsExistPetSkill(int const iSkillNo);
	void UpdatePetEquipIconBgText(bool const bEquip, int const iIconKey);
	bool IsUsablePetTrainItem(PgInventory* pkInv, EUseItemPetFeedType eIndex, int const iCustomData = 0);
	int GetPetDyeingRandColorMax(void);
	int GetPetDyeingRandColorIndex(void);
	bool OnClickDyeingBtn(void);
};

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

int g_iInvViewGrp = 1;//EQUIP을 봐야되서.	
int g_iShopViewGrp = 1;//
bool g_bIsEqComp = false;//툴팁 장비비교 하는가.
__int64 g_iShopViewSubOrder = 0;//
bool g_bReqInvWearableUpdate = false;
int const MAX_EXTEND_SLOT = 6;

int const MAX_DEF_INVUI_LINE_COUNT = 6;
int const MAX_CUR_INVUI_LINE_COUNT = 8;
int const MAX_DEF_INVUI_SLOT_COUNT = 36;
int const MAX_CUR_INVUI_SLOT_COUNT = 72;

PgStore g_kViewStore;//지금 보고있는 

typedef std::map<std::wstring const, DWORD>	InvenMap;
typedef InvenMap::iterator		InvenMapIter;

InvenMap g_InvenMap;

PgInventory g_kOtherViewInv;//상대보기 인벤.
PgInventory g_kPetViewInv;	//펫 인벤

bool lwGetReqInvWearableUpdate()
{
	return g_bReqInvWearableUpdate;
}

void  lwSetReqInvWearableUpdate(bool bValue)
{
	g_bReqInvWearableUpdate = bValue;
}

void UpdateReqInvWearableUpdate()
{

}

inline void RenderIconItemCount(int const iItemCount, CItemDef const * const pItemDef, POINT2 const& ptPos)
{
	if( !pItemDef ){ return; }

	size_t const iMinimumAmount = 1;
	if( iItemCount
		&&	pItemDef->IsAmountItem()
		&&	iMinimumAmount < pItemDef->GetAbil(AT_MAX_LIMIT) )//소비형 이고 최대 겹쳐지는 수가 1 초과 일 경우
	{
		SRenderTextInfo kRenderTextInfo;
		kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(iItemCount);
		kRenderTextInfo.kLoc = ptPos;
		kRenderTextInfo.wstrFontKey = FONT_TEXT;
		kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
		g_kUIScene.RenderText(kRenderTextInfo);
	}
}

namespace PgGuildInventory
{
	BM::GUID kGuildInventoryNpc;

	void SetGuildInventoryNpc(lwGUID kNpcGuid)
	{
		kGuildInventoryNpc = kNpcGuid();
	}

	void ClearGuildInventoryNpc()
	{
		kGuildInventoryNpc.Clear();
	}
}

namespace PgSafeInventory
{
	BM::GUID kSafeInventoryNpc;
	std::wstring const kInvSafeUIName(_T("SFRM_INV_SAFE"));
	std::wstring const kInvShareSafeUIName(_T("SFRM_SHARE_SAFE"));
	void CallInvSafeUI(lwGUID kNpcGuid)
	{
		kSafeInventoryNpc = kNpcGuid();
		CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kInvSafeUIName );
		if( pkTopWnd )
		{
			RegistUIAction(pkTopWnd);
		}
	}

	void SendCloseSafe()
	{
		bool bShare = false;
		BM::Stream kPacket(PT_C_M_REQ_EXIT_SAFE);
		kPacket.Push(bShare);
		NETWORK_SEND(kPacket);
	}

	void SendCloseShareSafe()
	{
		bool bShare = true;
		BM::Stream kPacket(PT_C_M_REQ_EXIT_SAFE);
		kPacket.Push(bShare);
		NETWORK_SEND(kPacket);
	}

	void CloseInvSafeUI()
	{
		kSafeInventoryNpc.Clear();
		XUIMgr.Close( kInvSafeUIName );
	}

	void CallInvShareSafeUI(lwGUID kNpcGuid)
	{
		kSafeInventoryNpc = kNpcGuid();
		CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kInvShareSafeUIName );
		if( pkTopWnd )
		{
			RegistUIAction(pkTopWnd);
		}
	}

	void CloseInvShareSafeUI()
	{
		kSafeInventoryNpc.Clear();
		XUIMgr.Close( kInvShareSafeUIName );
	}
}

void Send_PT_C_M_REQ_ITEM_CHANGE_GUILD(const SItemPos &kCasterPos, const SItemPos &kTargetPos, const bool bCasterIsGuildInv, const bool bTargetIsGuildInv)
{
	if( (0 == kCasterPos.x && 0 == kCasterPos.y)
		||	 (0 == kTargetPos.x && 0 == kTargetPos.y) )
	{
		return;
	}

	EGuildInvTradeType eType = EGIT_NONE;
	if( bCasterIsGuildInv && bTargetIsGuildInv )
	{// 길드금고내 아이템 이동(길드금고 ==> 길드금고)
		eType = EGIT_ITEM_MOVE;
	}
	else if( bCasterIsGuildInv && !bTargetIsGuildInv )
	{// 길드금고 아이템 꺼내기(길드금고 ==> 인벤토리)
		eType = EGIT_ITEM_OUT;
	}
	else if( !bCasterIsGuildInv && bTargetIsGuildInv )
	{// 길드금고 아이템 보관(인벤토리 ==> 길드금고)
		eType = EGIT_ITEM_IN;
	}
	else
	{
		return ;
	}

	BM::Stream Packet(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryUpdate));
	Packet.Push(g_kGuildMgr.GuildGuid());
	Packet.Push(static_cast<int>(eType));
	Packet.Push(kCasterPos);
	Packet.Push(kTargetPos);
	Packet.Push(lwGetServerElapsedTime32());
	Packet.Push(PgGuildInventory::kGuildInventoryNpc);
	NETWORK_SEND(Packet)
}

void Send_PT_C_M_REQ_ITEM_CHANGE(const SItemPos &kCasterPos, const SItemPos &kTargetPos)
{
	if( (0 == kCasterPos.x && 0 == kCasterPos.y)
		||	 (0 == kTargetPos.x && 0 == kTargetPos.y) )
	{
		return;
	}

	BM::Stream Packet(PT_C_M_REQ_ITEM_CHANGE);
	Packet.Push(kCasterPos);
	Packet.Push(kTargetPos);
	Packet.Push(lwGetServerElapsedTime32());
	Packet.Push(PgSafeInventory::kSafeInventoryNpc);

	PgBase_Item kItem = CompairToWarnForSafeAddition(kCasterPos, kTargetPos);
	if(false==kItem.IsEmpty())
	{
		CheckSafeAddition(kItem, Packet);
		return;
	}

	kItem = CompairToWarnForSafeAdditionItem(kCasterPos, kTargetPos);
	if( false == kItem.IsEmpty() )
	{
		CheckSafeAdditionItem(kItem, Packet);
		return;
	}

	NETWORK_SEND(Packet)
}

void Send_PT_C_M_REQ_ITEM_CHANGE_YES_NO(int const TTWID, const SItemPos &kCasterPos, const SItemPos &kTargetPos)
{
	if( (0 == kCasterPos.x && 0 == kCasterPos.y)
		||	 (0 == kTargetPos.x && 0 == kTargetPos.y) )
	{
		return;
	}

	BM::Stream Packet(PT_C_M_REQ_ITEM_CHANGE);
	Packet.Push(kCasterPos);
	Packet.Push(kTargetPos);

	lwCallCommonMsgYesNoBox(MB(TTW(TTWID)), lwPacket(&Packet), true, MBT_ITEM_ACTION_YESNO_TO_PACKET);
}

void Send_PT_C_M_REQ_ITEM_CHANGE_TO_PET(SItemPos const& rkPlayerItemPos, SItemPos const& rkPetItemPos, bool bPlayerToPet = true)
{
	if( (0 == rkPlayerItemPos.x && 0 == rkPlayerItemPos.y)
		||	 (0 == rkPetItemPos.x && 0 == rkPetItemPos.y) )
	{
		return;
	}

	PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if ( !pkMyPlayer )	{return;}

	if(BM::GUID::NullData()==pkMyPlayer->SelectedPetID())	{return;}
	BM::GUID kItemID;
	if ( S_OK == pkMyPlayer->GetInven()->GetItemID( rkPlayerItemPos, kItemID ) )
	{// 플레이어 인벤에 아이템이 있는 경우에는 펫이 살아 있어야 한다.
		PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkMyPlayer->SelectedPetID());
		if ( !pkPetPilot )
		{
			Notice_Show( TTW(355), EL_Warning );
			return;
		}

		PgPet *pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
		if(!pkPet)	{return;}
		if(EPET_TYPE_1==static_cast<EPetType>(pkPet->GetPetType()))	{return;}	//1차펫이면 장착하지 말자
	}

	BM::Stream Packet(PT_C_M_REQ_ITEM_CHANGE_TO_PET);
	Packet.Push(rkPlayerItemPos);
	Packet.Push(rkPetItemPos);
	Packet.Push(lwGetServerElapsedTime32());
	Packet.Push(bPlayerToPet);
	NETWORK_SEND(Packet)
}

bool CanChangeEquipItem(CItemDef const *pkItemDef)
{

	//	플레이어가 스킬을 사용중일때에는 장비 변경을 할 수 없음.
	if(true == pkItemDef->CanEquip())
	{
		PgActor * pkActor = g_kPilotMan.GetPlayerActor();
		if(pkActor)
		{
			{// 채집 보조도구 장착시
				int const iJobSkillSubToolType = pkItemDef->GetAbil(AT_JOBSKILL_TOOL_TYPE);
				if(ETOOL_CHAINSAW <= iJobSkillSubToolType
					&& ETOOL_NET >= iJobSkillSubToolType
					)
				{// 장착하는 아이템이 보조 채집 도구인데
					PgItemEx* pkWeaponItem = pkActor->GetEquippedWeapon();
					if(pkWeaponItem)
					{
						CItemDef* pkWeaponItemDef = pkWeaponItem->GetItemDef();
						int const iJobSkillMainToolType = pkWeaponItemDef->GetAbil(AT_JOBSKILL_TOOL_TYPE);
						int const iCheckVal = iJobSkillSubToolType - (ETOOL_CHAINSAW-ETOOL_AXE);
						if(iCheckVal != iJobSkillMainToolType)
						{// 메인 무기가 채집 도구가 아니라면
							lwAddWarnDataTT(791120+iCheckVal); // 먼저 채집 도구를 장착해야함
							return false;//사용 불가
						}
					}
				}
			}

			PgAction	*pkAction = pkActor->GetAction();
			if(pkAction)
			{
				CSkillDef	const	*pkSkillDef = pkAction->GetSkillDef();
				if(pkSkillDef)
				{
					if(EST_GENERAL != pkSkillDef->GetType())
					{
						lwAddWarnDataTT(20026); // 장착/해제 할 수 없는 상태입니다.
						return false;//사용 불가
					}
				}
			}
		}
	}

	if(pkItemDef->IsPetItem() || pkItemDef->IsType(ITEM_TYPE_PET)) //현재 장착/해제할 아이템이 펫이고
	{
		PgActor * pkActor = g_kPilotMan.GetPlayerActor();
		if(pkActor && pkActor->IsRidingPet()) //PC가 탑승중이라면..
		{
			lwAddWarnDataTT(20026); // 장착/해제 할 수 없는 상태입니다.
			return false;
		}
	}

	return	true;
}
bool IsCanUseItemGround(int const iItemNo)
{
	if(!g_pkWorld)
	{
		return false;
	}
	if( !iItemNo )
	{
		return true;//비어 있으면 통과
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pItemDef )
	{
		return true;//아이템이 아니면 통과
	}


	/*
	유저가 점프중일때 의자 사용을 못하게 막는다.
	*/

	bool bRest = (UICT_REST == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE));
	if(	true == bRest ||
		UICT_REST_EXP == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) ||
		UICT_CHAOS_PORTAL == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) ||
		UICT_HIDDEN_PORTAL == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) ||
		(0 != pItemDef->GetAbil(AT_MAP_NUM)) )
	{
		PgActor * pkActor = g_kPilotMan.GetPlayerActor();

		if( NULL == pkActor )
		{
			return false;
		}

		if( false == pkActor->IsMeetFloor() || pkActor->IsOnRidingObject() )
		{
			return false;
		}

		if( true == bRest && true == pkActor->IsRidingPet() )
		{ //펫 탑승 중에는 휴식 아이템 사용 불가
			return false;
		}

		if( true == pkActor->IsNowFollowing() )
		{
			lwAddWarnDataTT(10410);
			return false;
		}

		PgAction* pkCurAction = pkActor->GetAction();
		if( pkCurAction )
		{
			std::string prevActionID = pkCurAction->GetID();
			if( "IDLE" != pkCurAction->GetActionType() )
			{
				return false;
			}
		}

		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		if( true == pkPlayer->IsItemEffect(SAFE_FOAM_EFFECTNO) )
		{
			lwAddWarnDataTT(10410);
			return false;
		}
	}

	T_GNDATTR const kNotAttr = (T_GNDATTR)(pItemDef->GetAbil(AT_NOTAPPLY_MAPATTR));
	T_GNDATTR const kCanAttr = (T_GNDATTR)(pItemDef->GetAbil(AT_CAN_GROUND_ATTR));
	bool bCantUseGround = (0 != (kNotAttr & g_pkWorld->GetAttr()));
	bCantUseGround = ((0 != kCanAttr)? 0 == (g_pkWorld->GetAttr() & kCanAttr): false) || bCantUseGround;
	if( bCantUseGround )//아이템인데 사용제약 걸리면
	{
		lwAddWarnDataTT(20027);//사용할 수 없는 장소입니다.
		return false;//사용 불가
	}

	int const iMapNo = pItemDef->GetAbil(AT_MAP_NUM);
	if( iMapNo )
	{//원정대 로비에서 포탈 아이템을 사용 못하도록 한다.
		if( g_pkWorld->IsHaveAttr( GATTR_EXPEDITION_LOBBY ) )
		{
			lwAddWarnDataTT(20027);
			return false;
		}
	}
	return true;//통과
}

bool UseItemCustomType(int const iItemNo, PgBase_Item const* pkItem, SItemPos const & rkItemInvPos, bool bDblClick = false)
{
	if( !g_pkWorld )
	{
		return true;
	}

	if( !iItemNo )
	{
		return true;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pItemDef
		||	!pkItem )
	{
		return true;
	}

	if ( true == PgHardCoreDungeon::IsVote() )
	{
		lwAddWarnDataTT( 402203 );
		return true;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		bool bIsStateOk = false;
		if(pkPlayer->IsAlive())
		{
			if(ITEM_USE_STATE_DEAD != pItemDef->GetAbil(AT_ITEM_USE_STATUS))
			{//살아 있을때만 쓸 수 있음.
				bIsStateOk = true;
			}
		}
		else
		{//사망 했다면.
			if(ITEM_USE_STATE_DEAD != pItemDef->GetAbil(AT_ITEM_USE_STATUS))
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			else
			{//사망시만 쓸 수 있는 아이템.
				bIsStateOk = true;
			}
		}


		if(!bIsStateOk)
		{//쓸 수 있는 상태가 아님.
			lwAddWarnDataTT( 10410 );
			return true;
		}
	}

	int const iMakingType = pItemDef->GetAbil(AT_MAKING_TYPE);
	if( iMakingType & EMAKING_TYPE_COOKING || iMakingType & EMAKING_TYPE_MAZE_ITEM )	//요리 아이템 or 아이템조합이면
	{
		if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
		{// NO_DUPLICATE로 지정된 UI가 열려 있으면 사용 불가
			lwAddWarnDataTT( 10410 );
			return true;
		}
	}

	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);

	// 아이템 사용 레벨 제한 체크
	switch(iType)
	{
	case UICT_CHAOS_PORTAL:
	case UICT_EXPCARD:
		{
			int const iCurLevel = pkPlayer->GetAbil(AT_LEVEL);
			int const iMinLv = pItemDef->GetAbil(AT_LEVELLIMIT);
			int const iMaxLv = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
			if( ((iMinLv > 0) && (iCurLevel < iMinLv))
			 || ((iMaxLv > 0) && (iCurLevel > iMaxLv)) )
			{
				lwAddWarnDataTT(22027);
				return true;
			}
		}break;
	}

	switch( iType )
	{
	case UICT_SMS:
		{
			int const iValue = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			tagSMSInfo kInfo;
			kInfo.iItemNo = pkItem->ItemNo();
			kInfo.kItemGuid = pkItem->Guid();
			kInfo.kItemPos = rkItemInvPos;
			kInfo.iValue = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			kInfo.iValue2 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
			CallSMS(kInfo);
			//return;
		}break;
	case UICT_LETTER:
		{
			int const iValue = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			g_kQuestMan.CallNewLetter(iValue);
		}break;
	case UICT_COUPLE:
		{
			lwCouple::ReqWarpCouple();
		}break;	
	case UICT_SEAL:
	case UICT_SEAL_REMOVE:
		{// 봉인 및 봉인해제 아이템 사용시
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			g_kSealProcess.CallSealProcessUI(iType, iItemNo);
		}break;
	case UICT_MONSTERCARD:
		{
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			PgMonsterCardUtil::MonsterCardListUpdate(iType, iItemNo, rkItemInvPos);
		}break;
	case UICT_LOCKED_CHEST:
		{
			PgJewelBoxUtil::JewelBoxItemUpdate(iType, iItemNo, rkItemInvPos);
		}break;
	case UICT_USERMAPMOVE:
		{
			lwTeleCardUI::CallTeleCardUI(iItemNo);
		}break;
	case UICT_REVIVE:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return true;
			}

			ESkillTargetType const kTargetType = static_cast<ESkillTargetType>(pItemDef->GetAbil(AT_TARGET_TYPE));

			if ( true == pkPlayer->IsAlive() && (ESTARGET_NONE == kTargetType))
			{
				lwAddWarnDataTT( 10410 );// 죽었을때만 사용해야 해
				return true;
			}
			return false;//처리안되었어.
		}break;
	case UICT_CHAOS_PORTAL:
		{
			int const iMissionNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			if( 0 < iMissionNo )
			{
				lwSetMissionNo(iMissionNo);
				lua_tinker::call<void, int>("CallMissionSelectMapUI", 1);
			}

			/*PgPilot *pkPilot =g_kPilotMan.GetPlayerPilot();
			if (pkPilot)
			{
				PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					PgAction *pkAction = pkActor->GetAction();
					if( pkAction )
					{
						lua_tinker::call<void, char const*, lwActor, BYTE, BYTE>("World_ScrollPortal_Action","Chaos" , lwActor(pkActor), rkItemInvPos.x, rkItemInvPos.y);
					}					
				}
			}*/
		}break;
	case UICT_HIDDEN_PORTAL:
		{
			BM::Stream Packet(PT_C_M_REQ_HIDDEN_ITEM);			
			Packet.Push(iItemNo);
			Packet.Push(rkItemInvPos);
			NETWORK_SEND(Packet)
		}break;
	case UICT_HIDDEN_REWORD:
		{
			BM::Stream Packet(PT_C_M_REQ_HIDDEN_ITEM_PACK);			
			Packet.Push(rkItemInvPos);
			NETWORK_SEND(Packet)
		}break;
	case UICT_MISSION_DEFENCE_CONSUME:
	case UICT_MISSION_DEFENCE_CONSUME_ALL:
		{
			return false;
		}break;
	case UICT_MISSION_DEFENCE_TIMEPLUS:
		{
			if(g_pkWorld)
			{
				if( GATTR_MISSION != g_pkWorld->GetAttr() )
				{
					::Notice_Show_ByTextTableNo(400917, EL_Warning);
					return true;
				}

				XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
				if( NULL != pkWndTop )
				{
					::Notice_Show_ByTextTableNo(400930, EL_Warning);
					return true;
				}

				BM::Stream Packet(PT_C_M_REQ_DEFENCE_TIMEPLUS);
				Packet.Push(rkItemInvPos);
				NETWORK_SEND(Packet)
			}
		}break;
	case UICT_MISSION_DEFENCE7_ITEM:
		{
			int iOption = 0;
			int const iCustomType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			if(DI_D7_RELAY_STAGE==iCustomType)
			{
				XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(L"SFRM_DEFENCE7_RELAY");
				if( NULL != pkWndTop )
				{
					XUI::CXUI_CheckButton * pkWnd = dynamic_cast<XUI::CXUI_CheckButton*>( pkWndTop->GetControl(L"CBTN_REMOVE_GUARDIAN") );
					if(pkWnd)
					{
						iOption = false==pkWnd->Check() ? 0 : 1;
					}
				}
			}

			BM::Stream kPacket(PT_C_M_REQ_DEFENCE_ITEM_USE);
			kPacket.Push(iItemNo);
			kPacket.Push(rkItemInvPos);
			kPacket.Push(iOption);
			NETWORK_SEND(kPacket);
			//// 미션 레벨 체크
			//int const iMissionValue = (1 << g_pkWorld->GetDifficultyLevel()-1);


			//if( iMissionValue < MILT_MAX )
			//{
			//	return false;
			//}

			//if( iMissionValue & GetAbil(AT_MISSIONITEM_LEVELTYPE) )
			//{
			//	// Send Packet
			//	return true;
			//}
		}break;
	case UICT_MISSION_DEFENCE_POTION:
		{
			if( g_pkWorld
				&& GATTR_MISSION != g_pkWorld->GetAttr() )
			{
				::Notice_Show_ByTextTableNo(400917, EL_Warning);
				return true;
			}
			return false;

			/*BM::Stream Packet(PT_C_M_REQ_DEFENCE_POTION);
			Packet.Push(rkItemInvPos);
			NETWORK_SEND(Packet)*/
		}break;
	case UICT_GUILDMarkChange1:
		{
			PgGuildMgrUtil::CallGuildChangeMark1(iItemNo, rkItemInvPos);
		}break;
	case UICT_INVEXTEND:
		{
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, rkItemInvPos);
			kPacket.Push(lwGetServerElapsedTime32());

			int InvType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			int ExtendSlotCount = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pPlayer ){ return true; }

			PgInventory* pInv = pPlayer->GetInven();
			if( !pInv ){ return true; }


			if( 0 == pInv->GetInvExtendSize((EInvType)InvType) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406109, true);
				return true;
			}

			std::wstring kStr = TTW(400123 + InvType);

			wchar_t szTemp[1024] = {0,};
			swprintf_s(szTemp, 1024, TTW(406103).c_str(), ExtendSlotCount, MAX_EXTEND_SLOT*3);
			kStr += szTemp;

			lwCallCommonMsgYesNoBox(MB(kStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, MB(TTW(406106)));
		}break;
	case UICT_PREMIUM_SERVICE:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return true;
			}

			int const iServiceNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			GET_DEF(PgDefPremiumMgr, kDefPremium);
			SPremiumData const* pkDefPremium = kDefPremium.GetDef(iServiceNo);
			if(!pkDefPremium)
			{
				::Notice_Show_ByTextTableNo(64003, EL_Warning);
				return true;
			}

			BM::vstring vStr;
			if(pkPlayer->GetPremium().IsUserService())
			{
				if(pkDefPremium->iServiceNo == pkPlayer->GetPremium().GetServiceNo())
				{
					vStr = TTW(64001);
				}
				else
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 64002, true);
					return true;
				}
			}
			else
			{
				vStr = TTW(64000);
			}
			vStr.Replace(L"#TITLE#", pkDefPremium->kTitle );

			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, rkItemInvPos);
			kPacket.Push(lwGetServerElapsedTime32());

			lwCallCommonMsgYesNoBox(MB(vStr), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		}break;
	case UICT_SAFEEXTEND:
		{
			lwUIWnd kWnd = lwCallUI("SFRM_EXTEND_INVSAFE_SLOT");
			if( kWnd.IsNil() )
			{
				return false;
			}
			kWnd.SetCustomData<int>(iItemNo);
		}break;
	case UICT_GUILD_INV_EXTEND_LINE:
	case UICT_GUILD_INV_EXTEND_TAB:
		{// 길드금고 확장
			if( !g_kGuildMgr.HaveGuildInventory() )
			{// 길드금고를 개설 하지 않았습니다.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403410, true);
				return true;
			}

			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, rkItemInvPos);
			kPacket.Push(lwGetServerElapsedTime32());
			NETWORK_SEND(kPacket);
			return true;			
		}break;	
	case UICT_MODIFY_CARD:
		{
			PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return true;
			}
			if( !pkPlayer->IsCreateCard() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51061, true);
				return true;
			}
		}	
	case UICT_CREATE_CARD:
		{
			lwCharacterCard::CallCharacterCardUI(true, static_cast<EUseItemCustomType>(iType), g_kCharacterCardInfo, pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1), false);
		}break;
	case UICT_BIND:
		{// 자물쇠 아이템 사용
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
			if(pkPlayerActor)
			{				
				pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "button_UI_Open", 0.0f);
				lwUIItemBind::UseBindItem(rkItemInvPos);
			}
		}break;
	case UICT_UNBIND:
		{// 열쇠 아이템 사용
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
			if(pkPlayerActor)
			{
				pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "button_UI_Open", 0.0f);
				int const iDays = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);	//해제 기간
				lwUIItemBind::UseUnbindItem(rkItemInvPos, iDays);
			}			
		}break;
	case UICT_USER_PORTAL:
		{// 기억 전송 카드(위치저장포탈 사용)			
			if(!g_pkWorld)
			{
				return false;
			}
			switch( g_pkWorld->GetAttr() )
			{// 다음 맵에서는
			case GATTR_PVP:					// PVP
			case GATTR_EMPORIABATTLE:		// 엠포리아
				{// 기억 전송 카드를 사용할수 없음			
					::Notice_Show(TTW(20027), EL_Warning);
					return false;
				}break;
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
			PgBase_Item kItem;
			if(S_OK != pInv->GetItem(rkItemInvPos, kItem))
			{
				return false;
			}
			g_kMemTransCard.SetCashItemPos(rkItemInvPos);
			BM::GUID kOwnerGuid;
			if(!g_kPilotMan.GetPlayerPilotGuid(kOwnerGuid)) 
			{
				return false;
			}			

			std::wstring kMapName;
			if(!g_kMemTransCard.GetSavedMapName(kMapName))
			{// 저장된 맵 이름을 얻어와
				return false;
			}

			std::wstring kText = TTW(790335);
			{// 표시할 문장에 합친후
				size_t const NullStrSize = 1;
				size_t const BufSize = kText.size() + kMapName.size()+NullStrSize;
				wchar_t* pBuff= new wchar_t[BufSize];
				::memset(pBuff,NULL, sizeof(wchar_t)*BufSize);
				::wsprintfW(pBuff, kText.c_str(), kMapName.c_str());
				kText = pBuff;
				SAFE_DELETE_ARRAY(pBuff);
			}
			//UI를 호출한다
			CallYesNoMsgBox(kText, kOwnerGuid, MBT_USER_PORTAL);
		}break;
	case UICT_STYLEITEM:
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pPlayer)
			{ 
				return true; 
			}
			__int64 const i64ClassLimit = pItemDef->GetAbil64(AT_CLASSLIMIT);
			int const iGenderLimit = pItemDef->GetAbil(AT_GENDERLIMIT);
			if( IS_CLASS_LIMIT(i64ClassLimit, pPlayer->GetAbil(AT_CLASS) ) == false)
			{
				lwAddWarnDataTT(22024);
				return true;
			}
			int const iGender = pPlayer->GetAbil(AT_GENDER);
			if(  iGender != iGenderLimit&&  
				3 != iGenderLimit)
			{//공용이 아닌데 내 성별이랑 다르면 안됨
				lwAddWarnDataTT(790739);
				return true;
			}

			int const StyleType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			lwStyleModifier::CallStyleModifyUI((E_STYLE_TYPE)StyleType, iItemNo);
		}break;
	case UICT_ROLLBACK:
		{
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			lwLuckyChanger::CallLuckyChanger();
		}break;
	case UICT_GAMBLE:
		{
			PgGambleUtil::GambleItemUpdateClear();
			PgGambleUtil::GambleItemUpdate(iType, iItemNo, rkItemInvPos);
		}break;
	case UICT_ELIXIR:
		{
			if(false==pItemDef->IsType(ITEM_TYPE_PET))	{return false;}
			PgPet* pkPet = GetMySelectedPet();
			if(NULL==pkPet)	//펫 착용 여부
			{
				lwAddWarnDataTT( 3410 );	//대상 펫이 없습니다.
				return true;
			}

			if(EPET_TYPE_1==pkPet->GetPetType())	//1차펫이면
			{
				lwAddWarnDataTT( 358 );	//펫의 종류에 맞지 않는 아이템입니다.
				return true;
			}

			return false;

		}break;
	case UICT_PET_FEED:
		{
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			int const iUICT2 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
			switch(iUICT2)
			{
			case PET_FEED_TYPE_1:
			case PET_FEED_TYPE_2:
				{
					g_kPetUIMgr.UseFeedItem(rkItemInvPos, iUICT2);
				}break;
			case PET_FEED_TRAINING:
			case PET_FEED_PLAYING:
			case PET_FEED_MP_DRINK:
				{
					PgPet* pkPet = GetMySelectedPet();
					if(NULL==pkPet)	//펫 착용 여부
					{
						lwAddWarnDataTT( 3410 );	//대상 펫이 없습니다.
						return true;
					}
					if( false == PgPetUIUtil::IsUsablePetTrainItem(NULL, static_cast<EUseItemPetFeedType>(iUICT2), pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3)))
					{
						lwAddWarnDataTT( 386 );
						return true;
					}
					else
					{
						if(EPET_TYPE_1==pkPet->GetPetType())	//1차펫이면
						{
							lwAddWarnDataTT( 358 );	//펫의 종류에 맞지 않는 아이템입니다.
							return true;
						}
					}

					return false;
				}break;
				/*
				case PET_FEED_TYPE_2:
				case PET_FEED_TRAINING:
				case PET_FEED_PLAYING:	
				case PET_FEED_REVIVE:
				{
				PgInventory* pkInv = pkPlayer->GetInven();
				if( pkInv )
				{
				if( false == PgPetUIUtil::IsUsablePetTrainItem(pkInv, static_cast<EUseItemPetFeedType>(iUICT2)))
				{
				return true;
				}

				g_kPetUIMgr.SetFeedItem(rkItemInvPos, iUICT2);
				g_kPetUIMgr.SetSrcItem(PgItem_PetInfo::ms_kPetItemEquipPos, iUICT2);
				g_kPetUIMgr.SendReqFeedPet(true, iUICT2);
				//PgPetUIUtil::lwUsePetTrainItem(iUICT2);
				return true;
				}
				}break;
				case PET_FEED_OTHER:
				case PET_FEED_MP_DRINK:
				{
				PgPet* pkPet = GetMySelectedPet();
				if(!pkPet)	{return true;}

				if(EPET_TYPE_1 == static_cast<EPetType>(pkPet->GetPetType()))
				{
				lwAddWarnDataTT(358);
				return true;
				}

				bool bCanUse = false;
				bool bUseAbil = false;
				static int const kPetAbilArray[4] = {AT_MP, AT_HUNGER, AT_HEALTH, AT_MENTAL};
				int const kPetAbilMaxArray[4] = {pkPet->GetAbil(AT_C_MAX_MP), PgItem_PetInfo::MAX_PET_STATE_VALUE, PgItem_PetInfo::MAX_PET_STATE_VALUE, PgItem_PetInfo::MAX_PET_STATE_VALUE};

				for(int i = 0; i<4; ++i)
				{
				const int iValue = pItemDef->GetAbil(kPetAbilArray[i]);
				if(iValue)
				{
				bUseAbil = true;
				if(kPetAbilMaxArray[i]>pkPet->GetAbil(kPetAbilArray[i]))
				{
				bCanUse = true;
				break;
				}
				}
				}

				if(bUseAbil)
				{
				return !bCanUse;
				}

				return false;
				}break;
				*/
			default:
				{
					return false;
				}break;
			}
		}break;
	case UICT_EXPCARD:
		{
			lwAccumlationExpCard::CheckUseExpCard(rkItemInvPos);
			return true;
		}break;
	case UICT_SAFE_EXTEND_MAX_IDX:
		{
			/*
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pPlayer)
			{ 
			return true; 
			}
			PgInventory* pInv = pPlayer->GetInven();
			if(!pInv)
			{ 
			return true;
			}

			switch(g_kLocal.ServiceRegion())
			{
			case LOCAL_MGR::NC_THAILAND:
			case LOCAL_MGR::NC_INDONESIA:
			case LOCAL_MGR::NC_JAPAN:
			{
			//금고 확장 여부
			int const iCustomType_Inv = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			int const iExtendSize = static_cast<int>(pInv->GetEnableExtendMaxIdx(static_cast<EInvType>(iCustomType_Inv)));
			if( iCustomType_Inv > iExtendSize )
			{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406109, true);
			}
			}break;
			default:
			{

			}break;
			}
			*/
		}break;
	case UICT_EXTEND_MAX_IDX:
		{
			return (true == PgItemUseChecker::CommonExtendInvTypeCheck( iType, 0, rkItemInvPos ))?(false):(true);
		}break;
	case UICT_COUPLE_ITEM:
		{
			PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer || !g_pkWorld)
			{
				return true;
			}

			if(pkPlayer->GetCoupleGuid().IsNull())
			{
				lwAddWarnDataTT(450080);
				return true;
			}

			if(g_pkWorld)
			{
				PgActor* pkCaster = g_kPilotMan.GetPlayerActor();
				if(!pkCaster)
				{
					return true;
				}

				int const iCustomType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
				PgActor* pkTarget = dynamic_cast<PgActor*>(g_pkWorld->FindObject(pkPlayer->GetCoupleGuid()));
				if(!pkTarget)
				{
					lwAddWarnDataTT(450081);
					return true;
				}

				if(iCustomType < (pkCaster->GetPos() - pkTarget->GetPos()).Length())
				{
					lwAddWarnDataTT(450081);
					return true;
				}

				if(pkTarget->GetFreezed())
				{
					lwAddWarnDataTT(10410);
					return true;
				}
			}

			// 아이템을 사용 못하는 경우만 체크 한다.
			return false;
		}
	case UICT_FORCED_ENCHANT:
		{
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			lwUIWnd kWnd = lwCallUI("SFRM_ENCHANT_USE_ITEM");
			if( kWnd.IsNil() )
			{
				return false;
			}
			kWnd.SetCustomData<int>(iItemNo);
			lwMakeForceInchantUIText(kWnd);
		}break;
	case UICT_REPAIR_ITEM:
		{
			CallRepairItemUseUI(rkItemInvPos, *pkItem);
		}break;
	case UICT_TELEPORT:
		{
			lwOnCallCenterMiniMap(pkItem->Guid());
		}break;
	case UICT_STATUS_RESET:
		{// 스테이터스 초기화 아이템
			lwUIActiveStatus::UseInitStatusItem(static_cast<int>(pkItem->ItemNo()));
		}break;
	case UICT_CALL_SAFE:
		{//창고 호출
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}
			return false;//처리는 밖에서
		}		
	case UICT_PET_RENAME:
		{
			if( lwIsExistUniqueType( "NO_DUPLICATE" ) )
			{
				lwAddWarnDataTT( 10410 );
				return true;
			}

			PgPet* pkPet = GetMySelectedPet();
			if(!pkPet)	{return true;}
			lua_tinker::call< void, bool >("CallPetRenameChk", true);	//이 함수가 호출되었다는것은 캐쉬아이템을 사용하겠다는 뜻이므로 True
		}break;
	case UICT_PET_COLORCHANGE:
		{
			if(!PgPetUIUtil::OnClickDyeingBtn())
			{
				return true;
			}
		}break;
	case UICT_CALL_HELP:
		{
			XUIMgr.Activate(L"FRM_HELP_MAIN");
			return true;
		}break;
	case UICT_EVENT_ITEM_REWARD_GAMBLE:
		{
			PgEventBoxUtil::EventBoxItemUpdateClear();
			PgEventBoxUtil::EventBoxItemUpdate(iType, iItemNo, rkItemInvPos);
		}break;
	case UICT_AWAKE_INCREASE_ITEM:
		{
			if ( pkPlayer->GetAbil(AT_ENABLE_AWAKE_SKILL) )
			{
				return false;
			}

			lwAddWarnDataTT( 41004 );
		}break;
	case UICT_REBUILD_RAND_QUEST_CONSUME:
	case UICT_REBUILD_RAND_QUEST:
		{
			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( pkMyQuest )
			{
				int const iNoticeLevel = 2;
				SChatLog kMsgLog(CT_EVENT_SYSTEM);
				if( !pkItem->IsTimeOuted() )
				{
					int const iQuestType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
					switch( iQuestType )
					{
					case QT_Random:
						{
							if( RandomQuest::IsCanRebuild(pkMyQuest, iQuestType) )
							{
								XUIMgr.Call( std::wstring(L"FRM_ACCEPT_RANDOM_QUEST") );
							}
							else
							{
								g_kChatMgrClient.AddMessage(799105, kMsgLog, true, iNoticeLevel);
							}
						}break;
					case QT_RandomTactics:
						{
							if( BM::GUID::IsNotNull(pkPlayer->GetGuildGuid()) )
							{
								if( RandomQuest::IsCanRebuild(pkMyQuest, iQuestType) )
								{
									XUIMgr.Call( std::wstring(L"FRM_ACCEPT_TACTICS_RANDOM_QUEST") );
								}
								else
								{
									g_kChatMgrClient.AddMessage(799106, kMsgLog, true, iNoticeLevel);
								}
							}
							else
							{
								g_kChatMgrClient.AddMessage(799104, kMsgLog, true, iNoticeLevel);
							}
						}break;
					default:
						{
						}break;
					}
				}
				else
				{
					g_kChatMgrClient.AddMessage(799103, kMsgLog, true, iNoticeLevel);
				}
			}
			return true;
		}break;
	case UICT_REDICE_ITEM_OPTION:
		{
			lwCraftOptChanger::CallCraftOptChanger(SItemPos());
		}break;
	case UICT_REDICE_PET_OPTION:
		{
			lwPetOptChanger::Call(rkItemInvPos);
		}break;
	case UICT_SKILL_EXTEND:
		{
			lwSkillExtend::Call(rkItemInvPos);
		}break;
	case UICT_SUPER_GROUND_MOVE_FLOOR:
		{
			int const iValue = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			if( PgSuperGroundUI::IsCanUseFloorMoveItem(iValue) )
			{
				return false; // 패킷 보내기 위해서
			}
		}break;
	case UICT_RESET_ATTACHED:
		{
			lwUseUnlockItem::CreateCall(rkItemInvPos);
		}break;
	case UICT_VENDOR:
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pPlayer)
			{ 
				return true; 
			}
			PgInventory* pInv = pPlayer->GetInven();
			if(!pInv)
			{ 
				return true;
			}
			PgBase_Item kItem;
			if(S_OK != pInv->GetItem(rkItemInvPos, kItem))
			{
				return false;
			}
			XUI::CXUI_Wnd* pAuction;
			if( XUIMgr.IsActivate(_T("FRM_AUCTION_MAIN"), pAuction) )
			{//오픈마켓에 들어가 있다면 불가
				return false;
			}
			if( false == lwVendor::lwCheckVendorGround() )
			{//노점 사용 가능한 필드인가
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 799456, true);
				return false;
			}	
			if( pPlayer->VendorGuid().IsNotNull() )
			{//이미 노점이 열려있으면(들어가있어도) 불가
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 799457, true);
				return false;
			}
			if( pPlayer->HaveParty() )
			{//파티중이면 불가
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 799444, true);
				return false;
			}
			if( lwVendor::lwCheckVendorRemainTime() )
			{//기간 만료면 불가
				XUIMgr.Activate(L"SFRM_VENDOR_OPEN");
			}
			else
			{
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 1973, true);
			}
		}break;
	case UICT_TREASURE_CHEST:
		{
			//XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_LOCKED_CHEST"));
			lwTreasureChest::TryOpen(pkItem, rkItemInvPos);
		}break;
	case UICT_TREASURE_CHEST_KEY:
		{
			lwTreasureChest::InsertKey(pkItem, rkItemInvPos);
		}break;
	case UICT_SCROLL_QUEST:// 퀘스트 스크롤 아이템
		{
			int const iQuestID = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1); // 퀘스트 번호
			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( iQuestID );
			if( pkQuestInfo )
			{
				ContQuestNpc::const_iterator c_iter = pkQuestInfo->m_kNpc_Client.begin();
				if( c_iter != pkQuestInfo->m_kNpc_Client.end() )
				{
					ContQuestNpc::value_type kQuestNpc = (*c_iter);

					BM::Stream kPacket(PT_C_M_REQ_QUESTSCROLL);
					kPacket.Push(iQuestID);
					kPacket.Push( kQuestNpc.kNpcGuid );
					kPacket.Push(lwGetServerElapsedTime32());
					NETWORK_SEND(kPacket)

					//g_kQuestMan.CallQuestDialog(kQuestNpc.kNpcGuid, iQuestID, QRDID_Begin );
				}
			}
		}break;
	case UICT_REVIVE_PARTY:
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(pPlayer)
			{
				if( pPlayer->HaveParty() )
				{
					return false;
				}
				else
				{
					lwAddWarnDataTT(799421);
					return true;
				}
			}
		}break;
	case UICT_REVIVE_EXPEDITION:
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(pPlayer)
			{
				if( pPlayer->HaveExpedition() )
				{
					return false;
				}
				else
				{
					lwAddWarnDataTT(799423);
					return true;
				}
			}
		}break;
	case UICT_MANUFACTURE:
		{
			if(bDblClick)
			{
				g_kManufacture.SetMaterialItem(rkItemInvPos, pItemDef, iItemNo);
			}
		}break;
	case UICT_REMOVE_SOCKET:
		{
			lwAddWarnDataTT(799527);
			return true;
		}break;
	case UICT_REST_EXP:
		{
			if(!g_pkWorld)
			{
				return true;
			}
			PgHome* pkHome = g_pkWorld->GetHome();
			if( pkHome && false == pkHome->IsMyHome() )
			{
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 201774, true);
				return true;
			}
		}//브레이크 원래 없다.
	case UICT_SUPER_GROUND_ENTER_FLOOR:
	default:
		{
			return false;//처리 안되었어
		}break;
	}
	return true;//처리 되었다
}

void Send_PT_C_M_REQ_STORE_ITEM_BUY(BM::GUID const &kShopGuid, int const iItemNo, int const iCount)
{
	if(kShopGuid == BM::GUID::NullData() || 0 == iItemNo)
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgStore::CONT_GODDS::value_type kItemInfo;
	if(S_OK == g_kViewStore.GetGoods(iItemNo, kItemInfo))
	{
		if(pkPlayer->GetAbil64(AT_MONEY) < kItemInfo.iPrice * iCount)	//소지금 부족
		{
			lwAddWarnDataTT(700036);
			return;
		}

		if( g_kViewStore.GetType() != STORE_TYPE_JOBKSILL )
		{
			if(pkPlayer->GetAbil(AT_CP) < kItemInfo.iCP * iCount)	//CP 부족
			{
				lwAddWarnDataTT(700127);
				return;
			}
		}
	}
	else
	{
		return;
	}


	BM::Stream Packet(PT_C_M_REQ_STORE_ITEM_BUY);
	Packet.Push(kShopGuid);
	Packet.Push(g_kViewStore.GetType());
	Packet.Push(ESST_NONE);
	Packet.Push(iItemNo);
	Packet.Push(iCount);

	NETWORK_SEND(Packet)
}

void DoItemBuy(int const iViewNo)
{
	PgBase_Item const* pkItem = g_kViewStore.GetViewItem(iViewNo);
	if(!PgBase_Item::IsEmpty(pkItem))
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());

		if(pItemDef->IsAmountItem() &&
			1 < pItemDef->MaxAmount() )//소비형일 경우
		{//몇개 살꺼냐에 대한 UI 를 띄움.
			PgStore::CONT_GODDS::value_type kItemInfo;
			if(S_OK == g_kViewStore.GetGoods(pkItem->ItemNo(), kItemInfo))
			{
				SCalcInfo kInfo;

				kInfo.eCallType = CCT_SHOP_BUY;
				kInfo.iBasePrice = kItemInfo.iPrice;
				kInfo.kGuid = g_kViewStore.ShopGuid();
				kInfo.iItemNo = pkItem->ItemNo();

				CallCalculator(kInfo);
			}
			return;
		}

		Send_PT_C_M_REQ_STORE_ITEM_BUY(g_kViewStore.ShopGuid(), pkItem->ItemNo(), 1);
	}
}

void Send_PT_C_M_REQ_STORE_ITEM_SELL(const SItemPos &kItemPos, int const iItemNo, int const iCount, bool const bIsStockShop)
{
	if(0 == iItemNo || 0 == iCount )
	{
		return;
	}

	BM::Stream Packet(PT_C_M_REQ_STORE_ITEM_SELL);
	Packet.Push( g_kViewStore.ShopGuid() );
	Packet.Push( g_kViewStore.GetType() );
	Packet.Push(kItemPos);
	Packet.Push(iItemNo);
	Packet.Push(iCount);
	Packet.Push(bIsStockShop);

	NETWORK_SEND(Packet)
}

void Send_PT_C_M_REQ_ITEM_DIVIDE(const SItemPos &kItemPos, int const iItemNo, BM::GUID const &kItemGuid, int const iCount)
{
	if(0 == iItemNo || 0 == iCount || !lwGetUIWnd("FRM_AUCTION_MAIN").IsNil() || !lwGetUIWnd("SFRM_GUILD_INVENTORY").IsNil())
	{
		return;
	}

	BM::Stream Packet(PT_C_M_REQ_ITEM_DIVIDE);
	Packet.Push(kItemPos);
	Packet.Push(iItemNo);
	Packet.Push(kItemGuid);
	Packet.Push(iCount);

	NETWORK_SEND(Packet)
}

void DoItemSell(const SItemPos &kCasterPos, PgBase_Item const *pkItem, bool const bIsStockShop, int const iBasePrice)
{
	if(!pkItem)
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());

	if(!pItemDef)
	{
		return;
	}
	ECalcInfoType const eCallType = ((bIsStockShop)?CCT_STOCK_SHOP_SELL:CCT_SHOP_SELL);

	if(	CCT_STOCK_SHOP_SELL != eCallType//스톡샵은 판매가를 상점이 정한다.
		&&	0>=pItemDef->SellPrice())
	{
		lwAddWarnDataTT(400195);
		return;
	}

	if(pItemDef->IsAmountItem())
	{
		SCalcInfo kInfo;

		kInfo.eCallType = eCallType;
		kInfo.kItemPos = kCasterPos;
		kInfo.iItemNo = pkItem->ItemNo();
		kInfo.iMaxValue = pkItem->Count();
		kInfo.iBasePrice = pItemDef->SellPrice();
		if(bIsStockShop)
		{
			kInfo.iBasePrice = iBasePrice;
		}

		if(PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit())
		{
			if( S_PST_StoreSellPremium const* pkPremiumSell = pPlayer->GetPremium().GetType<S_PST_StoreSellPremium>() )
			{
				kInfo.iBasePrice += SRateControl::GetValueRate<int>( kInfo.iBasePrice, pkPremiumSell->iPremiumRate );
			}
		}

		CallCalculator(kInfo);
	}
	else
	{
		Send_PT_C_M_REQ_STORE_ITEM_SELL(kCasterPos, pkItem->ItemNo(), 1, bIsStockShop);
	}
}

bool IsWearable(const SIconInfo kIconInfo, CXUI_Icon* pkWnd, PgBase_Item const* pkItem, SItemPos const& rkItemPos)
{
	if (!pkWnd) {return false;}

	CXUI_Wnd* pkGray = pkWnd->GetControl(std::wstring(L"FRM_CANT_EQUIP"));
	if (NULL == pkGray) {return false;}
	if (!pkItem || PgBase_Item::IsEmpty(pkItem))
	{
		pkGray->Visible(false); 
		return false;
	}

	bool bValue = false;
	SItemPos kItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey);
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
	if( pItemDef )
	{
		bool bExchangeForm = false;
		for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
		{
			if( XUIMgr.Get( szExchangeUI[FormCount].szFrom ) )
			{
				bValue = !(lwDefenceMode::IsCanExchangeItem(pkItem->ItemNo(), szExchangeUI[FormCount].Menu));
				bExchangeForm = true;
				break;
			}
		}
		
		if( false == bExchangeForm )
		{
			if(g_kItemRarityAmplify.IsUseableItem(*pkItem, bValue))
			{
			}
			else if(g_kEnchantShift.IsUseableItem(rkItemPos, bValue))
			{
			}
			else if(XUIMgr.Get(L"SFRM_ARTICLE_REG_WIN"))
			{
				bValue = (false == CheckMarketRegItem(g_kPilotMan.GetPlayerUnit(), *pkItem, pItemDef, false));
			}
			else if(XUIMgr.Get(L"SFRM_ENCHANT_USE_ITEM"))
			{
				XUI::CXUI_Wnd* pkEnchantUseItem = XUIMgr.Get(L"SFRM_ENCHANT_USE_ITEM");
				DWORD dwMaterialNo = 0;
				pkEnchantUseItem->GetCustomData(&dwMaterialNo, sizeof(dwMaterialNo));
				int iErrorMsg = 0;
				bValue = !IsUsableItemForceInchant(dwMaterialNo, pkItem, iErrorMsg);
			}
			else if (XUIMgr.Get(std::wstring(_T("SFRM_SHINESTONE_LOTTERY"))))
			{
				bValue = 20700010 > pItemDef->No() || 20700050 < pItemDef->No();
			}
			else if (XUIMgr.Get(std::wstring(_T("SFRM_SOCKET_SYSTEM"))))
			{
				/*SEnchantInfo const kEhtInfo = pkItem->EnchantInfo();
				E_ITEM_GRADE const eItemGrade = GetItemGrade(*pkItem);
				bValue = false; //!kEhtInfo.EanbleGenSocket(eItemGrade);*/

				//bool const bIsCashItem = pItemDef->IsType(ITEM_TYPE_AIDS);
				bool const bIsEquipItem = pItemDef->IsType(ITEM_TYPE_EQUIP);

				if( bIsEquipItem /*&& !bIsCashItem*/ )
				{
					int iSocketIndex = 0;
					switch( g_kSocketSystemMgr.GetMenuType() )
					{
					case PgSocketFormula::SII_FIRST:
						{
							iSocketIndex = PgSocketFormula::GetCreateSocketItemOrder(*pkItem);					
						}break;
					case PgSocketFormula::SII_SECOND:
						{
							for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
							{
								iSocketIndex = PgSocketFormula::GetRemoveSocketItemOrder(*pkItem, i);
								if( 0 < iSocketIndex )
								{
									break;
								}
							}					
						}break;
					case PgSocketFormula::SII_THIRD:
						{
							iSocketIndex = PgSocketFormula::GetResetSocketItemOrder(*pkItem);
						}break;
					case PgSocketFormula::SII_FOURTH:
						{
							for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
							{
								iSocketIndex = PgSocketFormula::GetRemoveSocketItemOrder(*pkItem, 0);
								if( 0 < iSocketIndex )
								{
									break;
								}		
							}
						}break;
					default:
						{
						}break;
					}
					if( 2 < iSocketIndex )
					{ // 소켓이 2개 이상이면 회색배경 활성화.
						bValue = true;
					}
					else
					{
						bValue = !( 0 < iSocketIndex );
					}
				}
				else
				{
					bValue = true;
				}
				
				int	const iCantAbil = pItemDef->GetAbil(AT_ATTRIBUTE);
				if(iCantAbil)
				{
					if((iCantAbil & ICMET_Cant_GenSocket) == ICMET_Cant_GenSocket)
					{// 소캣 생성 금지
						bValue = true;
					}
				}
			}
			//else if(pkItem->Count() && pItemDef->IsAmountItem())//소비형일 경우
			//{
			//pkGray->Visible(false);
			//}
			else if( XUIMgr.Get(L"SFRM_COLLECT_ANTIQUE") )
			{// 골동품 수집창이 떠있을 때
				bValue = !(lwDefenceMode::IsCanChangeAntique(EGT_ANTIQUE, pkItem->ItemNo()));
			}
			else if( XUIMgr.Get(L"SFRM_SOULABIL_TRANSFER") )
			{
				lwSoulTransfer::IsWearable(*pkItem, bValue);
				if( pItemDef->GetAbil(AT_JS3_NOT_EQUIP_ITEM) )
				{
					bValue = false;
				}
			}
			else if( JobSkill_Util::IsJobSkill_Tool(pkItem->ItemNo()) )
			{//1차직업기술 - 채집도구
				PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
				if(pPlayer)
				{
					bValue = !JobSkill_Util::IsUseableJobSkill_Tool(pPlayer, pkItem->ItemNo());
				}
			}
			else if( UICT_MONSTERCARD == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) ) // 소켓카드 교환창이 떠있을 때보다 위에 있으면 안된다.
			{//  몬스터 카드의 ( AT_LEVELLIMIT는 장착 아이템의 레벨이다 )
				bValue = true;

				PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
				if (pkPilot && pkPilot->GetUnit() && pkPilot->GetUnit()->GetInven())
				{
					int const iItemLevelMin = pItemDef->GetAbil(AT_LEVELLIMIT);
					int const iItemLevelMax = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
					int const iEquipLimit = pItemDef->GetAbil(AT_EQUIP_LIMIT);
					int const iOrderIndex = pItemDef->GetAbil(AT_MONSTER_CARD_ORDER);
					PgInventory* pInv = pkPilot->GetUnit()->GetInven();
					int const MAX_IDX = pInv->GetMaxIDX(IT_EQUIP);
					PgBase_Item kItem;
					for(int j = 0; j < MAX_IDX; ++j)
					{
						if( S_OK != pInv->GetItem(SItemPos(IT_EQUIP,j), kItem) )
						{
							continue;
						}

						CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
						if( NULL == pDef )
						{
							continue;
						}

						if( false==PgMonsterCardUtil::IsMonsterCardSocketState(kItem.EnchantInfo(), iOrderIndex) )
						{
							continue;
						}

						if(0 == (pDef->GetAbil(AT_EQUIP_LIMIT) & iEquipLimit))
						{
							continue;
						}

						if( (iItemLevelMin > pDef->GetAbil(AT_LEVELLIMIT)) || (iItemLevelMax < pDef->GetAbil(AT_LEVELLIMIT)) )
						{
							continue;
						}

						bValue = false;
						break;
					}
				}
			}
			else if( JobSkill_Util::IsJobSkill_Item( pkItem->ItemNo() ) )
			{//일반 인벤토리가 열릴 때에는 회색 배경 셋팅을 하지 않는다.
			}
			else if (XUIMgr.Get(L"SFRM_DIMENTION"))
			{
			}
			else
			{
				PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
				if (pkPilot)
				{
					int	iClass = pkPilot->GetAbil(AT_CLASS);
					bValue = (false==IS_CLASS_LIMIT(pItemDef->m_i64ClassLimit, iClass) || (pkPilot->GetAbil(AT_LEVEL) + pkPilot->GetAbil(AT_EQUIP_LEVELLIMIT_MIN + pItemDef->EquipPos())) < pItemDef->GetAbil(AT_LEVELLIMIT) || !(pItemDef->GetAbil(AT_GENDERLIMIT) & pkPilot->GetAbil(AT_GENDER)) ); //인벤 아이콘 회색처리. 성별체크 추가.
					/*if (false==IS_CLASS_LIMIT(pItemDef->m_i64ClassLimit, iClass) || pkPilot->GetAbil(AT_LEVEL) < pItemDef->GetAbil(AT_LEVELLIMIT))
					{
					//pkGray->Visible(true); 
					//return false;
					}
					else
					{
					//pkGray->Visible(false); 
					//return true;
					}*/
				}
				if(false==bValue)	//봉인되었으면 장착불가
				{	
					bValue = pkItem->EnchantInfo().IsSeal();
				}
			}
		}
	}

	pkGray->Visible(bValue); 
	return !bValue;
}


void lwChangeInvViewGroup(int iGrp)
{
	g_iInvViewGrp = iGrp;
	lwSetReqInvWearableUpdate(true);
}

void lwSetInvViewSize(lwUIWnd UIParent)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	int const iTotalSize = pInv->GetMaxIDX((EInvType)g_iInvViewGrp);
	bool bIsBigExtend = false;

	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_DEVELOP:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_TAIWAN:
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_KOREA:
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_SINGAPORE:
	case LOCAL_MGR::NC_PHILIPPINES:
	case LOCAL_MGR::NC_VIETNAM:
		{
			bIsBigExtend = true;
		}break;
	case LOCAL_MGR::NC_CHINA:
		{
			if( IT_SAFE == g_iInvViewGrp )
			{
				// 중국은 
				// 금고는 6 * 3, 인벤토리는 8 * 3
			}
			else
			{
				bIsBigExtend = true;
			}
		}break;
	default:
		{
			bIsBigExtend = iTotalSize > MAX_DEF_INVUI_SLOT_COUNT;
		}break;
	}

	if( UIParent.IsNil() ){ return; }
	XUI::CXUI_Wnd* pInvWnd = UIParent.GetSelf();

	XUI::CXUI_Wnd* pFormBG1 = pInvWnd->GetControl(L"SFRM_ITEM_BG");
	if( !pFormBG1 ){ return; }

	XUI::CXUI_Wnd* pFormBG2 = pInvWnd->GetControl(L"SFRM_ITEM_BIG");
	if( !pFormBG2 ){ return; }

	bool const bChangedDefToCur = (pFormBG1->Visible() && bIsBigExtend);
	bool const bChangedCurToDef = (pFormBG2->Visible() && !bIsBigExtend);

	pFormBG1->Visible(!bIsBigExtend);
	pFormBG2->Visible(!pFormBG1->Visible());

	XUI::CXUI_Wnd* pParentBG = (pFormBG2->Visible())?(pFormBG2):(pFormBG1);

	XUI::CXUI_Wnd* pFormSDW = pParentBG->GetControl(L"SFRM_SDW");
	if( !pFormSDW ){ return; }

	XUI::CXUI_Wnd* pIconArea = pFormSDW->GetControl(L"FRM_ICON");
	if( !pIconArea ){ return; }

	int DEF_TO_CUR_GAB_X = (bChangedDefToCur || bChangedCurToDef)?(pFormBG2->Size().x - pFormBG1->Size().x):(0);
	if( bChangedDefToCur || bChangedCurToDef )
	{
		DEF_TO_CUR_GAB_X *= (bChangedDefToCur)?(1):(-1);
	}

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pIconArea->GetControl(L"BLD_ITEM_ICON"));
	if( !pkBuilder ){ return; }

	int const iLINE_SLOT = pkBuilder->CountX();
	int const iTOTAL_SLOT = pkBuilder->CountX() * pkBuilder->CountY();

	int CalcHeight = SetInvIconVisible(pFormSDW, L"SFRM_ICON", iTotalSize, iTOTAL_SLOT);
	SetInvIconVisible(pIconArea, L"InvIcon", iTotalSize, iTOTAL_SLOT);

	if( iTOTAL_SLOT > iTotalSize )
	{
		switch( g_kLocal.ServiceRegion() )
		{
		//case LOCAL_MGR::NC_THAILAND:	// 미개방 컨텐츠. 인벤토리 확장 유도 버튼 #NOT_OPEN#
		//case LOCAL_MGR::NC_INDONESIA:
		//case LOCAL_MGR::NC_JAPAN:
		//case LOCAL_MGR::NC_EU:
		//case LOCAL_MGR::NC_USA:
		//case LOCAL_MGR::NC_FRANCE:
		//case LOCAL_MGR::NC_GERMANY:
		//case LOCAL_MGR::NC_SINGAPORE:
		//case LOCAL_MGR::NC_PHILIPPINES:
		//case LOCAL_MGR::NC_DEVELOP:
		//	{
		//		int iExtendSize = (iTOTAL_SLOT < (iTotalSize + 24))?(iTOTAL_SLOT - iTotalSize):(24);
		//		int iHeight = SetLeadInvExtendInvIconVisible(pFormSDW, L"SFRM_ICON", g_iInvViewGrp, iTotalSize, iExtendSize);
		//		if( iHeight )
		//		{
		//			CalcHeight = iHeight;
		//		}
		//	}break;
		default:
			{
				SetLeadInvExtendInvIconVisible(pFormSDW, L"SFRM_ICON", g_iInvViewGrp, iTotalSize, 0);
			}break;
		}
	}
	else
	{
		SetLeadInvExtendInvIconVisible(pFormSDW, L"SFRM_ICON", g_iInvViewGrp, iTotalSize, 0);
	}

	XUI::CXUI_Wnd* pTitle = pInvWnd->GetControl(L"SFRM_TITLE");
	if( pTitle )
	{
		PgUIUtil::ResizeFormSize(pTitle, DEF_TO_CUR_GAB_X);

		XUI::CXUI_Wnd* pCloseBt = pTitle->GetControl(L"SFRM_CLOSE_BG");
		PgUIUtil::MoveFormLocation(pCloseBt, DEF_TO_CUR_GAB_X);		

		XUI::CXUI_Wnd* pSortBt = pTitle->GetControl(L"SFRM_SORTINV_BG");
		PgUIUtil::MoveFormLocation(pSortBt, DEF_TO_CUR_GAB_X);
	}

	PgUIUtil::ResizeFormSize(pIconArea, 0, CalcHeight + 6, true);
	PgUIUtil::ResizeFormSize(pFormSDW, 0, CalcHeight + 6, true);
	PgUIUtil::ResizeFormSize(pParentBG, 0, CalcHeight + 6, true);

	XUI::CXUI_Wnd* pETCInfo = pInvWnd->GetControl(L"FRM_ETC_INFO");
	if( pETCInfo )
	{
		bool bIsCashInv = (g_iInvViewGrp == IT_CASH);
		bool bIsEtcInv = (g_iInvViewGrp == IT_ETC);
		XUI::CXUI_Wnd* pkBtn0 = pETCInfo->GetControl(L"BTN_CURSOR_STATE_CHANGE");
		XUI::CXUI_Wnd* pkBtn1 = pETCInfo->GetControl(L"BTN_CURSOR_STATE_CHANGE_TO_CS_TR");
		XUI::CXUI_Wnd* pkBtn2 = pETCInfo->GetControl(L"BTN_CURSOR_STATE_CHANGE_TO_EX_EL");
		XUI::CXUI_Wnd* pkBtnText = pETCInfo->GetControl(L"FRM_ITEM_DISASS");
		if( pkBtn0 && pkBtn1 && pkBtnText )
		{
			pkBtn1->Visible(bIsCashInv);
			pkBtn2->Visible(bIsEtcInv);
			pkBtn0->Visible( (!bIsCashInv && !bIsEtcInv ));

			if(bIsCashInv)
			{//캐시 - 기간연장
				pkBtnText->Text(TTW(5103));
			}
			else if(bIsEtcInv)
			{//기타 - 속성추출
				pkBtnText->Text(TTW(792007));
			}
			else
			{//기본
				pkBtnText->Text(TTW(60084));
			}
		}

		PgUIUtil::MoveFormLocation(pETCInfo, 0, pParentBG->Location().y + pParentBG->Size().y + 2, true );
		PgUIUtil::ResizeFormSize(pETCInfo, DEF_TO_CUR_GAB_X);

		XUI::CXUI_Wnd* pMoney = pETCInfo->GetControl(L"SFRM_MONEY_BG");
		if( pMoney )
		{
			PgUIUtil::ResizeFormSize(pMoney, DEF_TO_CUR_GAB_X);

			pMoney = pMoney->GetControl(L"SFRM_SDW");
			if( pMoney )
			{
				PgUIUtil::ResizeFormSize(pMoney, DEF_TO_CUR_GAB_X);

				PgUIUtil::MoveFormLocation(pMoney->GetControl(L"FRM_COIN1"), DEF_TO_CUR_GAB_X);
				PgUIUtil::MoveFormLocation(pMoney->GetControl(L"FRM_COIN2"), DEF_TO_CUR_GAB_X);
				PgUIUtil::MoveFormLocation(pMoney->GetControl(L"FRM_COIN3"), DEF_TO_CUR_GAB_X);
			}
		}

		pMoney = pETCInfo->GetControl(L"SFRM_MONEY_BG2");
		if( pMoney )
		{
			PgUIUtil::ResizeFormSize(pMoney, DEF_TO_CUR_GAB_X);

			pMoney = pMoney->GetControl(L"SFRM_SDW");
			if( pMoney )
			{
				PgUIUtil::ResizeFormSize(pMoney, DEF_TO_CUR_GAB_X);

				PgUIUtil::MoveFormLocation(pMoney->GetControl(L"FRM_COIN2"), DEF_TO_CUR_GAB_X);
				PgUIUtil::MoveFormLocation(pMoney->GetControl(L"FRM_COIN3"), DEF_TO_CUR_GAB_X);
			}
		}

		XUI::CXUI_Wnd* pkGuidText = pInvWnd->GetControl(L"FRM_GUIDE_TEXT");
		XUI::CXUI_Wnd* pkGuidText_Sub = pInvWnd->GetControl(L"FRM_GUIDE_TEXT_SUB");
		XUI::CXUI_Wnd* pkGuidText_ItemLink = pInvWnd->GetControl(L"FRM_GUIDE_TEXT_ITEMLINK");
		if(pkGuidText && pkGuidText_Sub && pkGuidText_ItemLink)
		{
			PgUIUtil::MoveFormLocation(pkGuidText, 0, pETCInfo->Location().y + pETCInfo->Size().y + 7, true);
			PgUIUtil::ResizeFormSize(pkGuidText, DEF_TO_CUR_GAB_X);
			pkGuidText->TextPos(POINT2(pkGuidText->Size().x * 0.5f, pkGuidText->TextPos().y));
			PgUIUtil::ResizeFormSize(pInvWnd, DEF_TO_CUR_GAB_X);
			PgUIUtil::ResizeFormSize(pInvWnd, 0, pkGuidText->Location().y + 25, true);

			PgUIUtil::MoveFormLocation(pkGuidText_Sub, 0, pETCInfo->Location().y + pETCInfo->Size().y + 25, true);
			PgUIUtil::ResizeFormSize(pkGuidText_Sub, DEF_TO_CUR_GAB_X);
			pkGuidText_Sub->TextPos(POINT2(pkGuidText_Sub->Size().x * 0.5f, pkGuidText_Sub->TextPos().y));
			PgUIUtil::ResizeFormSize(pInvWnd, 0, pkGuidText_Sub->Location().y + 25, true);

			PgUIUtil::MoveFormLocation(pkGuidText_ItemLink, 0, pETCInfo->Location().y + pETCInfo->Size().y + 43, true);
			PgUIUtil::ResizeFormSize(pkGuidText_ItemLink, DEF_TO_CUR_GAB_X);
			pkGuidText_ItemLink->TextPos(POINT2(pkGuidText_ItemLink->Size().x * 0.5f, pkGuidText_ItemLink->TextPos().y));
			PgUIUtil::ResizeFormSize(pInvWnd, 0, pkGuidText_ItemLink->Location().y + 25, true);

			switch( g_iInvViewGrp )
			{
			case IT_EQUIP:	{ pkGuidText->Text(TTW(790039));	pkGuidText_Sub->Text(TTW(790038)); pkGuidText_ItemLink->Text(TTW(799332)); }break;
			case IT_CONSUME:{ pkGuidText->Text(TTW(790037));	pkGuidText_Sub->Text(TTW(790035)); pkGuidText_ItemLink->Text(TTW(799332)); }break;
			case IT_ETC:	{ pkGuidText->Text(TTW(790034));	pkGuidText_Sub->Text(TTW(790033)); pkGuidText_ItemLink->Text(TTW(799332)); }break;
			case IT_CASH:	{ pkGuidText->Text(TTW(5104));	pkGuidText_Sub->Text(TTW(5111)); pkGuidText_ItemLink->Text(TTW(799332)); }break;
			}
		}
	}	
}

bool lwUseBigSizeInv( const EInvType kInvType )
{
	switch( kInvType )
	{// 8*3 타입 사용하지 않는 인벤토리
	case IT_SAFE:
	case IT_RENTAL_SAFE1:
	case IT_RENTAL_SAFE2:
	case IT_RENTAL_SAFE3:
	case IT_SHARE_RENTAL_SAFE1:
	case IT_SHARE_RENTAL_SAFE2:
	case IT_SHARE_RENTAL_SAFE3:
	case IT_SHARE_RENTAL_SAFE4:
		{
			return false;
		}
	default:
		{
		}break;
	}

	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_DEVELOP:
	case LOCAL_MGR::NC_THAILAND:
	case LOCAL_MGR::NC_INDONESIA:
	case LOCAL_MGR::NC_JAPAN:
	case LOCAL_MGR::NC_TAIWAN:	
	case LOCAL_MGR::NC_RUSSIA:
	case LOCAL_MGR::NC_KOREA:
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
	case LOCAL_MGR::NC_SINGAPORE:
	case LOCAL_MGR::NC_PHILIPPINES:
		{
			return true;
		}break;
	default:
		{
			return false;
		}break;
	}
}

void lwSetInvSafeViewSize(XUI::CXUI_Wnd* pTopUI, int const iInvType)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	int const iTotalSize = pInv->GetMaxIDX((EInvType)iInvType);

	bool bIsBigExtend = false;
	if( lwUseBigSizeInv( static_cast<EInvType>(iInvType) ) )
	{
		bIsBigExtend = true;
	}
	else
	{
		bIsBigExtend = iTotalSize > MAX_DEF_INVUI_SLOT_COUNT;
	}

	if( !pTopUI ){ return; }
	XUI::CXUI_Wnd* pFormBG1 = pTopUI->GetControl(L"SFRM_ITEM_BG");
	if( !pFormBG1 ){ return; }

	XUI::CXUI_Wnd* pFormBG2 = pTopUI->GetControl(L"SFRM_ITEM_BIG");
	if( !pFormBG2 ){ return; }

	bool const bChangedDefToCur = (pFormBG1->Visible() && bIsBigExtend);
	bool const bChangedCurToDef = (pFormBG2->Visible() && !bIsBigExtend);

	pFormBG1->Visible(!bIsBigExtend);
	pFormBG2->Visible(!pFormBG1->Visible());

	XUI::CXUI_Wnd* pParentBG = (pFormBG2->Visible())?(pFormBG2):(pFormBG1);

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pParentBG->GetControl(L"BLD_INV_ICON"));
	if( !pkBuilder )
	{
		return;
	}

	int const iLINE_SLOT = pkBuilder->CountX();
	int const iTOTAL_SLOT = pkBuilder->CountX() * pkBuilder->CountY();

	XUI::CXUI_Wnd* pFormSDW = pParentBG->GetControl(L"FRM_BG");
	if( !pFormSDW ){ return; }

	XUI::CXUI_Wnd* pIconArea = pFormSDW->GetControl(L"SFRM_BG");
	if( !pIconArea ){ return; }

	int DEF_TO_CUR_GAB_X = (bChangedDefToCur || bChangedCurToDef)?(pFormBG2->Size().x - pFormBG1->Size().x):(0);
	if( bChangedDefToCur || bChangedCurToDef )
	{
		DEF_TO_CUR_GAB_X *= (bChangedDefToCur)?(1):(-1);
	}

	int CalcHeight = SetInvIconVisible(pIconArea, L"SFRM_ICON_BG", iTotalSize, iTOTAL_SLOT);
	SetInvIconVisible(pParentBG, L"ICON_ITEM", iTotalSize, iTOTAL_SLOT);

	if( iTOTAL_SLOT > iTotalSize )
	{
		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_THAILAND:
		case LOCAL_MGR::NC_INDONESIA:
		case LOCAL_MGR::NC_PHILIPPINES:
		case LOCAL_MGR::NC_JAPAN:
			{
				int iExtendSize = (iTOTAL_SLOT < (iTotalSize + 24))?(iTOTAL_SLOT - iTotalSize):(24);
				switch( iInvType )
				{
				case IT_RENTAL_SAFE1:
				case IT_RENTAL_SAFE2:
				case IT_RENTAL_SAFE3:
					{
						if( S_OK != pInv->CheckEnableUseRentalSafe((EInvType)iInvType) )
						{
							iExtendSize = 0;
						}
					}break;
				case IT_SHARE_RENTAL_SAFE1:
				case IT_SHARE_RENTAL_SAFE2:
				case IT_SHARE_RENTAL_SAFE3:
				case IT_SHARE_RENTAL_SAFE4:
					{
						iExtendSize = 0;
					}break;
				}
				int iHeight = SetLeadInvExtendInvIconVisible(pIconArea, L"SFRM_ICON_BG", iInvType, iTotalSize, iExtendSize);
				if( iHeight )
				{
					CalcHeight = iHeight;
				}
			}break;
		default:
			{
				SetLeadInvExtendInvIconVisible(pIconArea, L"SFRM_ICON_BG", iInvType, iTotalSize, 0);
			}break;
		}
	}
	else
	{
		SetLeadInvExtendInvIconVisible(pIconArea, L"SFRM_ICON_BG", iInvType, iTotalSize, 0);
	}

	XUI::CXUI_Wnd* pTitle = pTopUI->GetControl(L"FRM_TITLE");
	if( pTitle )
	{
		PgUIUtil::ResizeFormSize(pTitle, DEF_TO_CUR_GAB_X);

		XUI::CXUI_Wnd* pCloseBt = pTitle->GetControl(L"SFRM_BTN_BG");
		PgUIUtil::MoveFormLocation(pCloseBt, DEF_TO_CUR_GAB_X);		
	}

	PgUIUtil::ResizeFormSize(pTopUI, DEF_TO_CUR_GAB_X);
	PgUIUtil::ResizeFormSize(pFormSDW, 0, CalcHeight + 102, true);
	PgUIUtil::ResizeFormSize(pIconArea, 0, CalcHeight + 6, true);
	PgUIUtil::ResizeFormSize(pIconArea->GetControl(L"SFRM_BG_SD"), 0, pIconArea->Size().y, true);
	PgUIUtil::MoveFormLocation(pFormSDW->GetControl(L"SFRM_TIME"), 0, pIconArea->Location().y + pIconArea->Size().y + 3, true);

	XUI::CXUI_Wnd* pkTimeWnd = pTopUI->GetControl(L"FRM_TIME");
	if( pkTimeWnd )
	{
		PgUIUtil::MoveFormLocation(pkTimeWnd, 0, pIconArea->Location().y + pIconArea->Size().y + 3, true);
		PgUIUtil::ResizeFormSize(pkTimeWnd, DEF_TO_CUR_GAB_X);
		pkTimeWnd->TextPos(POINT2(pkTimeWnd->TextPos().x + DEF_TO_CUR_GAB_X, pkTimeWnd->TextPos().y));
	}

	XUI::CXUI_Wnd* pkIntercept = pTopUI->GetControl(L"FRM_INTERCEPT_TEXT");
	if( pkIntercept )
	{
		PgUIUtil::ResizeFormSize(pkIntercept, DEF_TO_CUR_GAB_X);
		PgUIUtil::MoveFormLocation(pkIntercept, 0, pkTimeWnd->Location().y, true);
		pkIntercept->TextPos(POINT2(pkIntercept->Size().x * 0.5f, pkIntercept->TextPos().y));
	}
	PgUIUtil::MoveFormLocation(pTopUI->GetControl(L"BTN_EXPAND"), DEF_TO_CUR_GAB_X);
	PgUIUtil::MoveFormLocation(pTopUI->GetControl(L"BTN_EXPAND"), 0, pkTimeWnd->Location().y + 2, true);
	XUI::CXUI_Wnd* pIntercept = pTopUI->GetControl(L"SFRM_INTERCEPTER");
	if( pIntercept )
	{
		PgUIUtil::ResizeFormSize(pIntercept, pIconArea->Size().x, pIconArea->Size().y, true);
		XUI::CXUI_Style_String kString = pIntercept->StyleText();
		POINT2 kTextSize(Pg2DString::CalculateOnlySize(kString));
		kTextSize.x = pIntercept->Size().x * 0.5f;
		kTextSize.y = pIntercept->Size().y * 0.5f - kTextSize.y * 0.5f;
		pIntercept->TextPos(kTextSize);
	}
	PgUIUtil::ResizeFormSize(pTopUI, 0, pkTimeWnd->Location().y + pkTimeWnd->Size().y + 12, true);
}

void lwCashShopInvViewSize(lwUIWnd Parent, bool const bIsSafe)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	int const iTotalSize = pInv->GetMaxIDX((false == bIsSafe)?(IT_CASH):(IT_CASH_SAFE));

	XUI::CXUI_Wnd* pParent = Parent.GetSelf();
	if( !pParent ){ return; }

	int const bIsHeight = SetInvIconVisible(pParent, L"SFRM_ICON_BG", iTotalSize, MAX_DEF_INVUI_SLOT_COUNT);
	SetInvIconVisible(pParent, L"ICN_CASH_INV", iTotalSize, MAX_DEF_INVUI_SLOT_COUNT);
}

void lwExtendInvSafeSlot(lwUIWnd kWnd, int const Idx)
{
	if( kWnd.IsNil() ){ return; }

	DWORD kItemNo = static_cast<DWORD>(kWnd.GetCustomData<int>());
	EInvType const InvType = (0 == Idx)?(IT_SAFE):(static_cast<EInvType>(IT_FIT_CASH + Idx));

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){ return; }

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv ){ return; }

	if( 0 == pInv->GetInvExtendSize(InvType) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 406109, true);
		return;
	}

	SItemPos	kItemPos;
	if( S_OK == pInv->GetFirstItem(kItemNo, kItemPos) )
	{
		BM::Stream	kPacket(PT_C_M_REQ_INVENTORY_EXTEND);
		kPacket.Push(kItemPos);
		kPacket.Push(InvType);

		wchar_t szTemp[ 1024 ] = {0,};
		swprintf_s(szTemp, 1024, TTW(406104).c_str(), (Idx + 1), MAX_EXTEND_SLOT);

		lwCallCommonMsgYesNoBox(MB(szTemp), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, MB(TTW(406108)));
		kWnd.Close();
	}
}

int SetInvIconVisible(XUI::CXUI_Wnd* pParent, std::wstring const& kBuildName, int const iMaxCount, int const iMaxSlotCount)
{
	int Height = 0;
	for( int i = 0; i < iMaxSlotCount; ++i )
	{
		BM::vstring	vIcon(kBuildName);
		vIcon += i;
		XUI::CXUI_Wnd* pIcon = pParent->GetControl(vIcon);
		if( !pIcon ){ return 0; }

		if( i < iMaxCount )
		{
			pIcon->Visible(true);
			Height = pIcon->Location().y + pIcon->Size().y;
		}
		else
		{
			pIcon->Visible(false);
		}
	}

	return Height;
}

int SetLeadInvExtendInvIconVisible(XUI::CXUI_Wnd* pParent, std::wstring const& kBuildName, int const iInvType, int const iUsingSlot, int const iExtendSlot)
{
	int const LOOP_MAX = iUsingSlot + iExtendSlot;
	int Height = 0;
	int iStartHeight = 0;

	for( int i = iUsingSlot; i < LOOP_MAX; ++i )
	{
		BM::vstring	vIcon(kBuildName);
		vIcon += i;
		XUI::CXUI_Wnd* pIcon = pParent->GetControl(vIcon);
		if( !pIcon )
		{ 
			return 0; 
		}

		if( i == iUsingSlot )
		{
			iStartHeight = pIcon->Location().y;
		}

		pIcon->Visible(true);
		Height = pIcon->Location().y + pIcon->Size().y;
	}

	XUI::CXUI_Wnd* pkGuideForm = pParent->GetControl(L"SFRM_EXTENDINV_GUIDE");
	if( pkGuideForm )
	{
		pkGuideForm->Visible(iExtendSlot != 0);
		int const FormHeight = Height - iStartHeight;
		pkGuideForm->Size( POINT2(pkGuideForm->Size().x, FormHeight) );
		pkGuideForm->Location( POINT2(pkGuideForm->Location().x, iStartHeight) );

		XUI::CXUI_Wnd* pkExtendBtn = pkGuideForm->GetControl(L"BTN_EXTEND");
		if( pkExtendBtn )
		{
			BM::vstring kBtnText(TTW(406131));
			switch( iInvType )
			{
			case IT_EQUIP:
			case IT_CONSUME:
			case IT_ETC:
			case IT_CASH:
				{
					kBtnText.Replace(L"#TYPE#", TTW(406134 + iInvType - IT_EQUIP));
				}break;
			case IT_SAFE:
			case IT_RENTAL_SAFE1:
			case IT_RENTAL_SAFE2:
			case IT_RENTAL_SAFE3:
				{
					BM::vstring kStr(TTW(406138));
					kStr += (iInvType == IT_SAFE)?(1):(iInvType - IT_RENTAL_SAFE1 + 2);
					kBtnText.Replace(L"#TYPE#", kStr);
				}break;
			}
			pkExtendBtn->Text(kBtnText);
			pkExtendBtn->SetCustomData(&iInvType, sizeof(iInvType));
			pkExtendBtn->Location( POINT2((pkGuideForm->Size().x - pkExtendBtn->Size().x) * 0.5f,
				(pkGuideForm->Size().y - pkExtendBtn->Size().y) * 0.5f) );
		}
	}
	return Height;
}

void lwCallExtendInvMessageBox(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	int iInvType = 0;
	if( pSelf->GetCustomDataSize() )
	{
		pSelf->GetCustomData(&iInvType, sizeof(iInvType));
	}

	EUseItemCustomType kExtendItemType = UICT_NONE;

	switch( iInvType )
	{
	case IT_EQUIP:
	case IT_CONSUME:
	case IT_ETC:
	case IT_CASH:
		{
			kExtendItemType = UICT_EXTEND_MAX_IDX;
		}break;
	case IT_SAFE:
	case IT_RENTAL_SAFE1:
	case IT_RENTAL_SAFE2:
	case IT_RENTAL_SAFE3:
		{
			kExtendItemType = UICT_SAFE_EXTEND_MAX_IDX;
		}break;
	case IT_SHARE_RENTAL_SAFE1:
	case IT_SHARE_RENTAL_SAFE2:
	case IT_SHARE_RENTAL_SAFE3:
	case IT_SHARE_RENTAL_SAFE4:
		{

		}break;
	default:
		return;
	}

	if( UICT_NONE == kExtendItemType )
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

	ContHaveItemNoCount kItemCont;
	HRESULT const kResult = pkInv->GetItems(kExtendItemType, kItemCont);
	if( kResult != S_OK || !PgItemUseChecker::PickOutOtherExtendInvItem( kItemCont, iInvType, kExtendItemType ) )
	{// 확장 아이템이 없으면 뜰 구매유도창. 현재 기능 없으므로 재끼자.
		lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 5, iInvType);
		//BM::vstring kDlgText(TTW(406132));
		//switch( kExtendItemType )
		//{
		//case UICT_EXTEND_MAX_IDX:
		//	{
		//		kDlgText.Replace(L"#TYPE#", TTW(406134 + iInvType - IT_EQUIP));
		//	}break;
		//case UICT_SAFE_EXTEND_MAX_IDX:
		//	{
		//		BM::vstring kStr(TTW(406138));
		//		kStr += (iInvType == IT_SAFE)?(1):(iInvType - IT_RENTAL_SAFE1 + 2);
		//		kDlgText.Replace(L"#TYPE#", kStr);
		//	}break;
		//}
		//lua_tinker::call<void, char const*, bool>("CommonMsgBox", kDlgText.operator const std::string().c_str(), true);
		return;
	}

	if( 1 == kItemCont.size() )
	{//여긴 그냥쓴다.
		SItemPos kItemPos;
		if( S_OK == pkInv->GetFirstItem(kItemCont.begin()->first, kItemPos) )
		{
			bool bExtendOK = false;
			BM::Stream kPacket;
			BM::vstring kDlgText(TTW(406133));
			switch( kExtendItemType )
			{
			case UICT_EXTEND_MAX_IDX:
				{
					if( PgItemUseChecker::CommonExtendInvTypeCheck(kExtendItemType, 0, kItemPos) )
					{
						kPacket.Push(PT_C_M_REQ_ITEM_ACTION);
						kPacket.Push(kItemPos);
						kPacket.Push(lwGetServerElapsedTime32());
						kDlgText.Replace(L"#TYPE#", TTW(406134 + iInvType - IT_EQUIP));
						bExtendOK = true;
					}
				}break;
			case UICT_SAFE_EXTEND_MAX_IDX:
				{
					if( PgItemUseChecker::CommonExtendInvTypeCheck(kExtendItemType, iInvType, kItemPos) )
					{//사용할 아이템을 결정했다.
						kPacket.Push(PT_C_M_REQ_INVENTORY_EXTENDIDX);
						kPacket.Push(kItemPos);
						kPacket.Push(iInvType);
						BM::vstring kStr(TTW(406138));
						kStr += (iInvType == IT_SAFE)?(1):(iInvType - IT_RENTAL_SAFE1 + 2);
						kDlgText.Replace(L"#TYPE#", kStr);
						bExtendOK = true;
					}
				}break;
			}
			if( bExtendOK )
			{
				lwCallCommonMsgYesNoBox(MB(kDlgText), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			}
		}
	}
	else
	{//여긴 종류를 보여준다.
		UIItemUtil::CONT_CUSTOM_PARAM	kParam;
		UIItemUtil::CONT_CUSTOM_PARAM_STR kParamStr;
		kParam.insert(std::make_pair(L"INV_TYPE", iInvType));
		kParam.insert(std::make_pair(L"CallYesNoBox", 0));
		UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, kParamStr);
	}
}

int lwGetInvViewGroup()
{
	return g_iInvViewGrp;
}

std::wstring GetItemName(int const iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);

	if( pDef == NULL )
	{
		return L"";
	}

	const wchar_t *pName = NULL;
	if(GetDefString(pDef->NameNo(), pName) )//이름 디스플레이
	{
		return pName;
	}
	return L"";
}

void SetDescriptionText(XUI::CXUI_Wnd* pkWnd, int const iItemNo)
{
	if( !pkWnd ){ return; }

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
	if( pDef == NULL ){ return; }

	const TCHAR *pText = NULL;
	int const iValue = pDef->GetAbil(ATI_EXPLAINID);
	if(iValue && GetDefString(iValue, pText))
	{
		Quest::SetCutedTextLimitLength(pkWnd, pText, L"..");
	}
}

__int64 GetShopMoney(int const iItemNo)
{
	PgStore::CONT_GODDS::value_type kItemInfo;
	if(S_OK == g_kViewStore.GetGoods(iItemNo, kItemInfo))
	{
		return kItemInfo.iPrice;
	}
	return 0i64;
}

void lwCallEmporiaShop(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if( !pkWnd ){ return; }

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LIST_ITEMS"));
	if( !pkList ){ return; }

	pkList->ClearList();

	g_kViewStore.BuildByView(0);
	int const iViewSize = g_kViewStore.GetGoodsViewSize();
	for(int i=0; i<iViewSize; ++i)
	{
		XUI::SListItem* pkItem = pkList->AddItem(L"");
		if(pkItem)
		{
			XUI::CXUI_Wnd* pkItemWnd = pkItem->m_pWnd;
			PgBase_Item const* pkItemData = g_kViewStore.GetViewItem(i);
			if(pkItemWnd && pkItemData)
			{
				int const iItemNo = pkItemData->ItemNo();
				__int64 const iMoney = GetShopMoney(iItemNo);

				XUI::CXUI_Wnd* pkTmp = NULL;
				pkTmp = pkItemWnd->GetControl(L"ICN_ITEM");
				pkTmp ? pkTmp->SetCustomData(&iItemNo, sizeof(iItemNo)) : 0;

				pkTmp = pkItemWnd->GetControl(L"FRM_ITEM_NAME");
				pkTmp ? pkTmp->Text(GetItemName(iItemNo)) : 0;

				pkTmp = pkItemWnd->GetControl(L"FRM_BUY_MONEY");
				pkTmp ? pkTmp->Text(GetMoneyString(iMoney)) : 0;

				pkTmp = pkItemWnd->GetControl(L"SFRM_DESCRIPTION");
				SetDescriptionText(pkTmp, iItemNo);
			}
		}
	}
}

void lwDoBuyEmporiaShop(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if( !pkWnd ){ return; }

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LIST_ITEMS"));
	if( !pkList ){ return; }

	SListItem* kItem = pkList->FirstItem();
	XUI::CXUI_CheckButton* kBuyWnd = NULL;
	while(kItem)
	{
		kBuyWnd = dynamic_cast<XUI::CXUI_CheckButton*>(kItem->m_pWnd->GetControl(L"CBTN_SELECT"));
		if(kBuyWnd->Check())
		{
			DoItemBuy(kItem->m_iIndex);
			break;
		}
		kItem = pkList->NextItem(kItem);
	}
}

/////////////직업스킬 구매 UI
void lwCallJobSkillShop(lwUIWnd kWnd)
{
	int const iGrp = 1;
	g_iShopViewGrp = iGrp;
	g_kViewStore.BuildAll();

	lwSetFilter_JobSkillShop(kWnd);
}

void lwSetFilter_JobSkillShop(lwUIWnd kWnd)
{//드랍다운 메뉴 설정
	XUI::CXUI_Wnd* pMain = kWnd();
	if(!pMain) 
	{
		return;
	}
	XUI::CXUI_Wnd* pShopMain = pMain->GetControl(L"SFRM_SHOP");
	if(!pShopMain)
	{
		return;
	}
	XUI::CXUI_Wnd* pItemBg = pShopMain->GetControl(L"SFRM_ITEM_BG");
	if(!pItemBg)
	{
		return;
	}
	XUI::CXUI_Wnd* pSelectWnd = pItemBg->GetControl(L"SFRM_SELECT_ITEMTYPE_LIST");
	if(!pSelectWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pListWnd = pSelectWnd->GetControl(L"LST_SELECT_TYPE");
	if(!pListWnd)
	{
		return;
	}
	XUI::CXUI_List* pSelectList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
	if(!pSelectList)
	{
		return;
	}
	pSelectList->ClearList();
	CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
	g_kTblDataMgr.GetContDef(pkContDefJobSkill);
	if(!pkContDefJobSkill->size())
	{
		return;
	}
	//전체 보기 미리 추가
	XUI::SListItem* pItem = pSelectList->AddItem(L"");
	if(pItem)
	{
		pItem->m_pWnd->Text( TTW(799501) );
		int const iJobSkillType = 0;
		pItem->m_pWnd->SetCustomData(&iJobSkillType, sizeof(iJobSkillType));
	}
	CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Job = pkContDefJobSkill->begin();
	while( iter_Job != pkContDefJobSkill->end() )
	{
		CONT_DEF_JOBSKILL_SKILL::key_type const &rkJobSkillKey = (*iter_Job).first;
		CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*iter_Job).second;

		if( JST_1ST_MAIN == rkJobSkillInfo.eJobSkill_Type ||
			JST_2ND_MAIN == rkJobSkillInfo.eJobSkill_Type ||
			JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type )
		{//주스킬의 경우만 받아서 Add하기
			wchar_t const* pName = NULL;
			if(!GetDefString(rkJobSkillKey ,pName))
			{
				return;
			}
			BM::vstring kTitle;
			switch(rkJobSkillInfo.eJobSkill_Type)
			{
			case JST_1ST_MAIN:
				{
					kTitle = TTW(799583);
				}break;
			case JST_2ND_MAIN:
				{
					kTitle = TTW(799584);
				}break;
			case JST_3RD_MAIN:
				{
					kTitle = TTW(799585);
				}break;
			default:
				{
					kTitle = TTW(799501);
				}break;
			}
			kTitle += BM::vstring(pName);
			XUI::SListItem* pItem = pSelectList->AddItem(L"");
			if(pItem)
			{
				pItem->m_pWnd->Text(kTitle);
				pItem->m_pWnd->SetCustomData(&rkJobSkillKey, sizeof(rkJobSkillKey));
			}
		}
		++iter_Job;
	}
}
void lwSelectFilter_JobSkillShop(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pItem = kWnd();
	if(!pItem) 
	{
		return;
	}
	int iSelect = 0;
	pItem->GetCustomData(&iSelect, sizeof(iSelect) );
	g_kViewStore.BuildByJobSkill(iSelect, 0);
}
void lwLearnFilter_JobSkillShop(bool const bCheck, int const iMovePage)
{
	XUI::CXUI_Wnd* pWndMain = XUIMgr.Get(L"FRM_SHOP_JOB");
	if(!pWndMain) 
	{
		return;
	}
	XUI::CXUI_Wnd* pShop = pWndMain->GetControl(L"SFRM_SHOP");
	if(!pShop)
	{
		return;
	}
	XUI::CXUI_Wnd* pItemBg = pShop->GetControl(L"SFRM_ITEM_BG");
	if(!pItemBg)
	{
		return;
	}
	XUI::CXUI_Wnd* pPage = pItemBg->GetControl(L"FRM_PAGE_NUM");
	if(!pPage)
	{
		return;
	}
	int iPage = 1;
	if( 0 != iMovePage)
	{
		iPage = iMovePage;
	}
	else
	{
		pPage->GetCustomData(&iPage, sizeof(iPage));
	}
	if( bCheck )
	{
		XUI::CXUI_Wnd* pSdw = pItemBg->GetControl(L"SFRM_SDW");
		if(!pSdw)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconWnd = pSdw->GetControl(L"FRM_ICON");
		if(!pIconWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pBuildIcon = pIconWnd->GetControl(L"BLD_ICON");
		if(!pBuildIcon)
		{
			return;
		}
		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pBuildIcon);
		if(!pBuild)
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		std::list<int> kContGoods;
		int const iContSize = g_kViewStore.GetGoodsViewSize();
		for(int idx=0; idx<iContSize; ++idx)
		{
			PgBase_Item const *pItem = g_kViewStore.GetViewItem(idx);
			if(!pItem)
			{
				return;
			}if( false == JobSkill_Util::IsJobSkill_SaveIndex(pItem->ItemNo()) )
			{//도감 아니면 그냥 넣어
				kContGoods.push_back(idx);
			}
		}
		int const iViewSize = kContGoods.size();
		std::list<int>::const_iterator iter = kContGoods.begin();
		int const iInvCount = pBuild->CountX() * pBuild->CountY();
		int iMaxPage = iViewSize / iInvCount;
		if( 0 != (iViewSize % iInvCount) )
		{
			++iMaxPage;
		}
		if( iMaxPage < iPage)
		{
			iPage = iMaxPage;
		}
		int iIterCount = 0;
		if( 1 < iPage)
		{
			int const iPageCount = (iPage-1) * iInvCount;
			while( iIterCount < iPageCount)
			{
				if(kContGoods.end() == iter)
				{
					break;
				}
				++iter;
				++iIterCount;
			}
		}
		BM::vstring kPage("#CUR# / #MAX#");
		kPage.Replace(L"#CUR#", iPage);
		kPage.Replace(L"#MAX#", iMaxPage);
		pPage->Text(static_cast<std::wstring>(kPage));
		pPage->SetCustomData(&iPage, sizeof(iPage));
		for(int iIdx=0; iIdx<iInvCount; ++iIdx)
		{
			BM::vstring kIcon(L"InvIcon");
			kIcon += BM::vstring(iIdx);
			XUI::CXUI_Wnd* pInvIcon = pIconWnd->GetControl(static_cast<std::wstring>(kIcon));
			if(!pInvIcon)
			{
				return;
			}
			XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pInvIcon);
			if(!pIcon)
			{
				return;
			}
			SIconInfo kIconInfo = pIcon->IconInfo();
			if( kContGoods.end() != iter)
			{
				kIconInfo.iIconKey = (*iter++);
			}
			else
			{
				kIconInfo.iIconKey = -1;
			}
			pIcon->SetIconInfo(kIconInfo);
		}
	}
	else
	{
		lua_tinker::call<void, lwUIWnd, int >("OnShopPageView", lwUIWnd(pItemBg), iPage);
	}
}
/////////////////////////////

void lwChangeShopViewGroup(int iGrp)
{
	g_iShopViewGrp = iGrp;
	g_kViewStore.BuildByView(g_iShopViewGrp, g_iShopViewSubOrder);//장착으로 셋팅해서 뜬다.
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("Inv"));
	if (pWnd)
	{
		XUI::CXUI_Wnd* pkTitle = pWnd->GetControl(std::wstring(_T("SFRM_TITLE")));
		if (pkTitle)
		{
			int const iItemTypeNum = 4;
			XUI::CXUI_CheckButton* pkCheck[iItemTypeNum] = {0,};

			pkCheck[0] = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(std::wstring(_T("SFRM_EQUIP_BG")))->GetControl(std::wstring(_T("CBTN_EQUIP"))));
			pkCheck[1] = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(std::wstring(_T("SFRM_CONSUM_BG")))->GetControl(std::wstring(_T("CBTN_CONSUM"))));
			pkCheck[2] = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(std::wstring(_T("SFRM_ETC_BG")))->GetControl(std::wstring(_T("CBTN_ETC"))));
			pkCheck[3] = dynamic_cast<XUI::CXUI_CheckButton*>(pkTitle->GetControl(std::wstring(_T("SFRM_CASH_BG")))->GetControl(std::wstring(_T("CBTN_CASH"))));

			int iViewGrp = 0;
			switch(iGrp)
			{
			case 1:	//무기
			case 2:	//방어구
			case 3:	//장신구
				{
					iViewGrp = 0;//lwChangeInvViewGroup(1);
				}break;
			case 4:	//소모아이템
				{
					iViewGrp = 1;//lwChangeInvViewGroup(2);
				}break;
			case 5://슈퍼걸 샵
				{
					iViewGrp = 3;
					g_iShopViewGrp = 3;
				}break;
			}

			bool bLock = false;
			for (int i = 0; i < iItemTypeNum; ++i)
			{
				bLock = false;
				if (iViewGrp == i)
				{
					lwChangeInvViewGroup(i+1);
					lwSetInvViewSize(lwUIWnd(pWnd));
					bLock = true;
				}

				if (pkCheck[i])
				{
					pkCheck[i]->ClickLock(iViewGrp == i);
					pkCheck[i]->Check(iViewGrp == i);
				}
			}
		}
	}
}

void lwChangeShopViewSubOrder(char const* szOrder)
{
	g_iShopViewSubOrder = _atoi64(szOrder);
	g_kViewStore.BuildByView(g_iShopViewGrp, g_iShopViewSubOrder);//장착으로 셋팅해서 뜬다.
}

void Send_PT_C_M_REQ_REGQUICKSLOT(size_t const slot_idx, SQuickInvInfo const &kQuickInvInfo)
{
	BM::Stream kPacket(PT_C_M_REQ_REGQUICKSLOT);
	kPacket.Push(slot_idx);//슬롯 인덱스로 본다.
	kPacket.Push(kQuickInvInfo);

	NETWORK_SEND(kPacket)
}

void Send_PT_C_M_REQ_REGQUICKSLOT_VIEWPAGE(char const cViewPage)
{
	BM::Stream kPacket(PT_C_M_REQ_REGQUICKSLOT_VIEWPAGE);
	kPacket.Push(cViewPage);//슬롯 인덱스로 본다.
	NETWORK_SEND(kPacket)
}

void lwUIWnd::DrawIconDetail_sub( SSimpleItemInfo const &kItemInfo, POINT2 const &ptPos )
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.iItemNo);
	PG_ASSERT_LOG(pItemDef);
	if ( pItemDef )
	{
		size_t const iMinimumAmount = 1;
		if( pItemDef->IsAmountItem()
			&&	iMinimumAmount < pItemDef->GetAbil(AT_MAX_LIMIT) )//소비형 이고 최대 겹쳐지는 수가 1 초과 일 경우
		{
			SRenderTextInfo kRenderTextInfo;
			kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(kItemInfo.iCount);
			kRenderTextInfo.kLoc = ptPos;
			kRenderTextInfo.wstrFontKey = FONT_TEXT;
			kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
			g_kUIScene.RenderText(kRenderTextInfo);
		}

		DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
		g_kUIScene.RenderIcon( pItemDef->ResNo(), ptPos, false, static_cast<unsigned short>(self->Width()), static_cast<unsigned short>(self->Height()), self->GrayScale() );
	}
}

void lwUIWnd::DrawIconDetail_sub(int const iIconGrp, PgBase_Item const* pkItem, const POINT2 &ptPos)
{
	if(!PgBase_Item::IsEmpty(pkItem))
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
		PG_ASSERT_LOG(pItemDef);
		if (pItemDef == NULL)
			return;

		size_t const iMinimumAmount = 1;
		if( pkItem->Count()
			&&	pItemDef->IsAmountItem()
			&&	iMinimumAmount < pItemDef->GetAbil(AT_MAX_LIMIT) )//소비형 이고 최대 겹쳐지는 수가 1 초과 일 경우
		{
			SRenderTextInfo kRenderTextInfo;
			kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(pkItem->Count());
			kRenderTextInfo.kLoc = ptPos;
			kRenderTextInfo.wstrFontKey = FONT_TEXT;
			kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
			g_kUIScene.RenderText(kRenderTextInfo);
		}

		bool bQuestDependItem = false, bIsIng = true;
		switch( iIconGrp )
		{
		case KUIG_ETC:
			{
				bQuestDependItem = g_kQuestMan.IsDependQuestItem(pkItem->ItemNo());
				if( bQuestDependItem )
				{
					bIsIng = g_kQuestMan.IsDependIngQuestItem(pkItem->ItemNo(), pItemDef);
				}
			}break;
		default:
			{
				bIsIng = bQuestDependItem = g_kQuestMan.IsDependIngQuestItem(pkItem->ItemNo(), pItemDef);
			}break;
		}
		if( bQuestDependItem )
		{
			int const iQuestItemMarkResNo = 1005;
			g_kUIScene.RenderIcon(iQuestItemMarkResNo, ptPos, false, 40, 40, !bIsIng);
		}

		bool bGray = ( pkItem->IsUseTimeOut() );
		self->GrayScale(bGray);

		DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
		g_kUIScene.RenderIcon( pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale() );

	}
}

void lwUIWnd::DrawIconDetail_AddRes(int const iResNo, POINT2 const& ptPos, unsigned short sW, unsigned short sH)
{
	CONT_DEFRES const* pkContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pkContDefRes);
	CONT_DEFRES::const_iterator iterDefRes = pkContDefRes->find(TBL_KEY_INT(iResNo));
	if(iterDefRes != pkContDefRes->end())
	{ //DrawList에 PushFront하므로 앞서는 순서대로 Push 해준다
		if( (*iterDefRes).second.AddResNo[2] != 0 &&
			pkContDefRes->find(TBL_KEY_INT( (*iterDefRes).second.AddResNo[2]) ) != pkContDefRes->end() )
		{
			g_kUIScene.RenderIcon((*iterDefRes).second.AddResNo[2], ptPos, false, sW, sH, self->GrayScale());
		}
		if( (*iterDefRes).second.AddResNo[1] != 0 &&
			pkContDefRes->find(TBL_KEY_INT( (*iterDefRes).second.AddResNo[1]) ) != pkContDefRes->end() )
		{
			g_kUIScene.RenderIcon((*iterDefRes).second.AddResNo[1], ptPos, false, sW, sH, self->GrayScale());
		}
		if( (*iterDefRes).second.AddResNo[0] != 0 &&
			pkContDefRes->find(TBL_KEY_INT( (*iterDefRes).second.AddResNo[0]) ) != pkContDefRes->end() )
		{
			g_kUIScene.RenderIcon((*iterDefRes).second.AddResNo[0], ptPos, false, sW, sH, self->GrayScale());
		}
	}
}

void lwUIWnd::DrawIconDetail(int const IconGrp, int const IconKey, const POINT2 &ptPos, const SIconInfo& rkIconInfo, bool const bDrawCount)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	switch(IconGrp)
	{
	case KUIG_EQUIP:
	case KUIG_CONSUME:
	case KUIG_ETC:
	case KUIG_CASH:
	case KUIG_SAFE:
	case KUIG_SAFE_ADDITION:
	case KUIG_CASH_SAFE:
	case KUIG_FIT:
	case KUIG_FIT_CASH:
	case KUIG_RENTAL_SAFE1:
	case KUIG_RENTAL_SAFE2:
	case KUIG_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE1:
	case KUIG_SHARE_RENTAL_SAFE2:
	case KUIG_SHARE_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE4:
	case KUIG_PET:
		{
			PgBase_Item kItem;
			if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(IconGrp, IconKey), kItem))
			{
				DrawIconDetail_sub(IconGrp, &kItem, ptPos);
			}
		}break;
	case KUIG_GUILD_INV_VIEW:
		{
			DrawIconDetail(g_kGuildMgr.GetGuildInvView(), IconKey, ptPos, rkIconInfo, true);
		}break;
	case KUIG_GUILD_INVENTORY1:
	case KUIG_GUILD_INVENTORY2:
	case KUIG_GUILD_INVENTORY3:
	case KUIG_GUILD_INVENTORY4:
		{
			PgBase_Item kItem;
			SItemPos kItemPos(IconGrp, IconKey);
			ConvertItemPosToGuildInvType(kItemPos);

			if(S_OK == g_kGuildMgr.GetInven()->GetItem(kItemPos, kItem))
			{
				DrawIconDetail_sub(IconGrp, &kItem, ptPos);
			}
		}break;
	case KUIG_CONSUME_PET:
	case KUIG_FIT_PET:
		{
			XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(self);

			PgBase_Item kItem;
			PgInventory* pkPetInven = GetMySelectedPetInven();
			bool bRet = (pkPetInven && S_OK == pkPetInven->GetItem(SItemPos(KUIG_CONSUME_PET==IconGrp?IT_CONSUME:IT_FIT, IconKey), kItem));
			if(bRet)
			{
				DrawIconDetail_sub(IconGrp, &kItem, ptPos);
			}

			PgPetUIUtil::UpdatePetEquipIconBgText(bRet, (pkIcon)?(pkIcon->IconInfo().iIconKey):(0));
		}break;
	case KUIG_SYSTEM_INVEN:
		{
			PgBase_Item const* pkItem = g_kSystemInven.GetAt(IconKey);
			DrawIconDetail_sub(IconGrp, pkItem, ptPos);
		}break;
	case KUIG_QUICK_INVEN:
		{
			SQuickInvInfo kQuickInvInfo;
			PgQuickInventory* pkQInv = pkPlayer->GetQInven();

			HRESULT const hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(IconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다

			if(S_OK == hRet)
			{
				switch(kQuickInvInfo.Grp())
				{
				case KUIG_CASH:
				case KUIG_CONSUME:
				case KUIG_PET:
					{
						int const iItemNo = kQuickInvInfo.ID();
						size_t const total_count = pkPlayer->GetInven()->GetTotalCount(iItemNo);
						PgBase_Item kItem;
						kItem.ItemNo(iItemNo);
						kItem.Count(total_count);
						SetGrayScale(false);
						DrawIconDetail_sub(IconGrp, &kItem, ptPos);
					}break;
				case KUIG_SKILLTREE:
					{
						PgSkillTree::stTreeNode	*pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(kQuickInvInfo.ID()));
						if(pkNode)
						{
							CSkillDef const *pSkillDef = pkNode->GetSkillDef();//g_SkillDefMgr.GetDef(iSkillNo);
							if(pSkillDef)
							{
								bool bGrayScale = !CheckHPMPForAction(pSkillDef, pkPlayer, false, false);

								if(pkNode->IsLearned() == false)
								{
									bGrayScale = true;
								}

								if(ESTARGET_COUPLE & pSkillDef->GetTargetType())
								{
									bGrayScale = pkPlayer->GetCoupleGuid().IsNull();
								}

								g_kUIScene.RenderIcon( pSkillDef->RscNameNo(), ptPos, false,40,40,bGrayScale);
							}
						}
					}break;
				case KUIG_SKILLTREE_PET:
					{
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(kQuickInvInfo.ID());
						if(pkSkillDef)
						{
							g_kUIScene.RenderIcon( pkSkillDef->RscNameNo(), ptPos, false,40,40,self->GrayScale());
						}
					}break;
				case KUIG_SKILLSET_BASIC:
					{
						bool const bGreyScale = lwSkillSet::lwCheckUseSkillSetNo(kQuickInvInfo.ID(), true);
						if(!bGreyScale)
						{
							g_kUIScene.RenderIcon(kQuickInvInfo.ID(), ptPos, false, 40, 40, !bGreyScale);
						}
						else
						{
							g_kUIScene.RenderIcon(kQuickInvInfo.ID(), ptPos, false, 40, 40, self->GrayScale());
						}
					}break;
				case KUIG_SKILLSET_SETICON:
					{
						const CONT_DEFSKILLSET *pkContDefMap = NULL;
						g_kTblDataMgr.GetContDef(pkContDefMap);
						if( !pkContDefMap ){ break; }

						CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->find(kQuickInvInfo.ID());
						if( c_iter==pkContDefMap->end() ){ break; }

						bool const bGreyScale = lwSkillSet::lwCheckUseSkillSetNo(kQuickInvInfo.ID(), true);
						if(!bGreyScale)
						{
							g_kUIScene.RenderIcon(c_iter->second.iResNo, ptPos, false, 40, 40, !bGreyScale);
						}
						else
						{
							g_kUIScene.RenderIcon(c_iter->second.iResNo, ptPos, false, 40, 40, self->GrayScale());
						}
					}break;
				case KUIG_JOB_SKILL:
				case KUIG_JOB_SKILL3_ITEM:
					{
						PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
						if(pPlayer)
						{
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pSkillDef;
							if( 0 != kQuickInvInfo.ID() )
							{
								pSkillDef = kSkillDefMgr.GetDef(kQuickInvInfo.ID());
								if(pSkillDef)
								{
									bool const bRet = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, kQuickInvInfo.ID() );
									g_kUIScene.RenderIcon(pSkillDef->RscNameNo(), ptPos, false, 40, 40, !bRet);
								}
								else
								{
									GET_DEF(CItemDefMgr, kItemDefMgr);
									CItemDef const *pItemDef = kItemDefMgr.GetDef(kQuickInvInfo.ID());
									if(pItemDef)
									{
										int const iCustomValue1 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
										bool const bHaveIdx = JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iCustomValue1);
										g_kUIScene.RenderIcon(rkIconInfo.iIconResNumber, ptPos, false, 40, 40, !bHaveIdx);
									}
								}
							}
						}
					}break;
				case KUIG_ITEM_SKILL:
					{
						int const iItemSkillNo = kQuickInvInfo.ID();
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
						if(pkSkillDef)
						{
							bool bIsUseable = ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo);
							g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, !bIsUseable);
						}
					}break;
				default:
					{
						//assert(NULL);//올라오면 안될 것이 왔다.
					}break;
				}
			}
		}break;
	case KUIG_SKILLTREE_PET:
	case KUIG_SKILLTREE:
		{
			unsigned int const iIconResNo = rkIconInfo.iIconResNumber;
			//#ifndef EXTERNAL_RELEASE			
			//			if (iIconResNo == 0)
			//			{
			//				PgSkillTree::stTreeNode *pFound = (PgSkillTree::stTreeNode *)g_kSkillTree.SerachEveryNodeRecursiveByKeySkillNo(IconKey);
			//				if(pFound) 			
			//				{
			//					if (iIconResNo != pFound->m_pkSkillDef->RscNameNo())
			//					{
			//						NILOG(PGLOG_ERROR, "[lwUIIconProcess] DrawIconDetail, %d, %d vs %d\n", IconKey, iIconResNo, pFound->m_pkSkillDef, pFound->m_pkSkillDef->RscNameNo());
			//						//rkIconInfo.iIconResNumber = pFound->m_pkSkillDef->RscNameNo();
			//					}
			//				}
			//			}
			//#endif
			//			if(self->VType() == E_XUI_CURSOR)
			//			{
			//g_kUIScene.RenderIcon(iIconResNo, ptPos, false,40,40,self->GrayScale());//이거 지우면 스킬 아이콘 드래그가 안됩니다.
			SRenderInfo kInfo;
			kInfo.bGrayScale = self->GrayScale();
			kInfo.kLoc.x = ptPos.x;
			kInfo.kLoc.y = ptPos.y;
			kInfo.kSizedScale.ptSrcSize.x = 40;
			kInfo.kSizedScale.ptSrcSize.y = 40;
			XUI::CXUI_Icon *pkIcon = dynamic_cast<XUI::CXUI_Icon *>(self);
			if(pkIcon)
			{
				pkIcon->GetClipRect(kInfo.rcClip);
				if(KUIG_SKILLTREE_PET==IconGrp)
				{
					if(0==iIconResNo)	{break;}
					::SetRect(&kInfo.rcClip, 0,0,0,0);
				}
			}
			g_kUIScene.RenderIcon(iIconResNo, kInfo);//이거 지우면 스킬 아이콘 드래그가 안됩니다.
			//			}
		}break;
	case KUIG_SKILLSET_BASIC:
		{
			bool const bGreyScale = lwSkillSet::lwCheckUseSkillSetNo(IconKey, true);
			if(!bGreyScale)
			{
				g_kUIScene.RenderIcon(IconKey, ptPos, false, 40, 40, !bGreyScale);
			}
			else
			{
				g_kUIScene.RenderIcon(IconKey, ptPos, false, 40, 40, self->GrayScale());
			}
		}break;
	case KUIG_SKILLSET_SETICON:
		{
			const CONT_DEFSKILLSET *pkContDefMap = NULL;
			g_kTblDataMgr.GetContDef(pkContDefMap);
			if( !pkContDefMap ){ break; }

			CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->find(IconKey);
			if( c_iter==pkContDefMap->end() ){ break; }

			bool const bGreyScale = lwSkillSet::lwCheckUseSkillSetNo(IconKey, true);
			if(!bGreyScale)
			{
				g_kUIScene.RenderIcon(c_iter->second.iResNo, ptPos, false, 40, 40, !bGreyScale);
			}
			else
			{
				g_kUIScene.RenderIcon(c_iter->second.iResNo, ptPos, false, 40, 40, self->GrayScale());
			}
		}break;
	case KUIG_SKILLSET_SKILLICON:
		{
			SIconInfo kData;
			self->GetCustomData(&kData, sizeof(kData));

			int const iSkillNo = kData.iIconKey;
			switch(kData.iIconGroup)
			{
			case KUIG_SKILLTREE:
				{
					PgSkillTree::stTreeNode	*pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
					if(!pkNode){ break; }

					CSkillDef const *pSkillDef = pkNode->GetSkillDef();//g_SkillDefMgr.GetDef(iSkillNo);
					if(!pSkillDef){ break; }

					bool bGrayScale = !CheckHPMPForAction(pSkillDef, pkPlayer, false, false);

					if(pkNode->IsLearned() == false)
					{
						bGrayScale = true;
					}

					if(ESTARGET_COUPLE & pSkillDef->GetTargetType())
					{
						bGrayScale = pkPlayer->GetCoupleGuid().IsNull();
					}

					g_kUIScene.RenderIcon( pSkillDef->RscNameNo(), ptPos, false,40,40,bGrayScale);
				}break;
			case KUIG_SKILLTREE_PET:
			case KUIG_SKILLSET_JUMPICON:
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					if(pkSkillDef)
					{
						g_kUIScene.RenderIcon( pkSkillDef->RscNameNo(), ptPos, false,40,40,self->GrayScale());
					}
				}break;
			case KUIG_ITEM_SKILL:
				{
					int const iItemSkillNo = iSkillNo;
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
					if(pkSkillDef)
					{
						bool bIsUseable = ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo);
						g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, !bIsUseable);
					}
				}break;
			}
		}break;
	case KUIG_SKILLSET_JUMPICON:
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(JUMPSKILLNO);
			if(!pkSkillDef)	{break;}

			g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, self->GrayScale());			
		}break;
	case KUIG_EFFECT:
		{
			int iCustom = rkIconInfo.iIconKey;
			if(iCustom > 0)
			{
				//				const CSkillDef* pDef = g_SkillDefMgr.GetDef(iCustom);
				GET_DEF(CEffectDefMgr, kEffectDefMgr);
				const CEffectDef *pDef = kEffectDefMgr.GetDef(iCustom);
				if (pDef)
				{ 
					int const iRes = pDef->GetAbil(AT_DEF_RES_NO);
					//					int iRes = pDef->RscNameNo();
					if( iRes > 0 )
					{
						g_kUIScene.RenderIcon(iRes, ptPos, false,32,32,self->GrayScale());
					}
				}
			}
			else
			{
				self->Text(std::wstring(_T("")));
			}
		}break;
	case KUIG_KEY_SET:
		{
			std::wstring kName;
			unsigned int iResNo = 0;
			bool bRet = g_kGlobalOption.GetKeyFuncResource(IconKey, kName, iResNo);
			if(bRet)
			{
				SRenderTextInfo kRenderTextInfo;

				kRenderTextInfo.wstrText = kName;
				kRenderTextInfo.kLoc = ptPos;
				kRenderTextInfo.wstrFontKey = FONT_TEXT;
				kRenderTextInfo.dwDiffuseColor = 0xFF573f1b;
				g_kUIScene.RenderText(kRenderTextInfo);

				g_kUIScene.RenderIcon(iResNo, ptPos, false, 40, 40,self->GrayScale());
			}
		}break;
	case KUIG_INV_VIEW:
		{
			DrawIconDetail(g_iInvViewGrp, IconKey, ptPos, rkIconInfo, true);
		}break;
	case KUIG_STORE:
	case KUIG_STOCK_STORE:
		{
			PgBase_Item const* pkItem = g_kViewStore.GetViewItem(IconKey);
			if(!PgBase_Item::IsEmpty(pkItem))
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
				if(pItemDef)
				{
					DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
					g_kUIScene.RenderIcon( pItemDef->ResNo(), ptPos, false,40,40, self->GrayScale());
				}
			}
		}break;
	case KUIG_MARKET:
		{
			SMarketIconInfo kInfo;
			self->GetCustomData(&kInfo, sizeof(SMarketIconInfo));

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kInfo.dwItemNum);
			if(pItemDef)
			{
				DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
				g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
				PgBase_Item kItem;
				PgInventory* pkInv = pkPlayer->GetInven();
				if( pkInv )
				{
					if( S_OK == pkInv->GetItem(SItemPos(kInfo.cItemGroup, kInfo.cInvPos), kItem) )
					{
						lwAccumlationExpCard::CheckUsingExpCardInfo(self, kItem, false);
					}
				}
			}
		}break;
	case KUIG_COMMON_GUID_TO_GRAY:
		{
			BM::GUID kGuid = self->OwnerGuid();

			PgInventory* pkInv = pkPlayer->GetInven();
			if( pkInv )
			{
				SItemPos	kPos;
				PgBase_Item	kItem;
				if( S_OK == pkInv->GetItem(kGuid, kItem, kPos) )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if(pItemDef)
					{
						g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
					}
				}
				else
				{
					DWORD dwItemNo = 0;
					self->GetCustomData(&dwItemNo, sizeof(dwItemNo));

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pItemDef = kItemDefMgr.GetDef(dwItemNo);
					if(pItemDef)
					{
						g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, true);
					}					
				}
			}
		}break;
	case KUIG_ANTIQUE_SRC:
		{
			if(NULL == pkPlayer->GetInven()){break;}

			TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
			g_kTblDataMgr.GetContDef(kObjLock);
			CONT_DEFANTIQUE const * pDefAntique = kObjLock.Get();
			if(NULL == pDefAntique){break;}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			PgInventory* pInv = pkPlayer->GetInven();

			SItemPos	kPos;
			self->GetCustomData(&kPos, sizeof(kPos));

			PgBase_Item kItem;
			if( S_OK != pInv->GetItem(kPos, kItem) ){break;}

			CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if(NULL == pItemDef){break;}

			//
			XUI::CXUI_Wnd * pkTargetWnd = self->Parent()->GetControl(L"ICN_TARGET");
			if(NULL == pkTargetWnd){break;}

			DWORD dwTargetItemNo = 0;
			pkTargetWnd->GetCustomData(&dwTargetItemNo, sizeof(dwTargetItemNo));
			if(0 > dwTargetItemNo){break;}

			CItemDef const* pTargetItemDef = kItemDefMgr.GetDef(dwTargetItemNo);
			if(NULL == pTargetItemDef){break;}

			int ChangeItemCount = 0;
			TBL_PAIR_KEY_INT const kKey(g_kGemStore.GetGemStoreType(),kItem.ItemNo());
			CONT_DEFANTIQUE::const_iterator Total_iter = pDefAntique->find(kKey);
			if(Total_iter != pDefAntique->end())
			{
				ChangeItemCount = Total_iter->second.kContArticles.size();
			}
			if(0 == ChangeItemCount){break;}

			int const iHaveNeedItemCount = (pItemDef->IsAmountItem() ? kItem.Count() : 1);
			CONT_DEF_ANTIQUE_ARTICLE::const_iterator Target_iter = Total_iter->second.kContArticles.begin();
			while(Target_iter != Total_iter->second.kContArticles.end())
			{
				if(dwTargetItemNo == Target_iter->second.first)
				{
					int const iNeedCount = Total_iter->second.iNeedCount;
					int const iAddCount = iHaveNeedItemCount / iNeedCount;
					int const iResultCount = Target_iter->second.second;

					SRenderTextInfo kRenderTextInfo;
					kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(iNeedCount * iAddCount);
					kRenderTextInfo.kLoc = self->TotalLocation();
					kRenderTextInfo.wstrFontKey = FONT_TEXT;
					kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
					g_kUIScene.RenderText(kRenderTextInfo);
					break;
				}
				++Target_iter;
			}

			DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
			g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
		}break;
	case KUIG_ANTIQUE_TGT:
		{
			if(NULL == pkPlayer->GetInven()){break;}

			TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
			g_kTblDataMgr.GetContDef(kObjLock);
			CONT_DEFANTIQUE const * pDefAntique = kObjLock.Get();
			if(NULL == pDefAntique){break;}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			PgInventory* pInv = pkPlayer->GetInven();

			//
			DWORD dwTargetItemNo = 0;
			self->GetCustomData(&dwTargetItemNo, sizeof(dwTargetItemNo));

			CItemDef const *pTargetItemDef = kItemDefMgr.GetDef(dwTargetItemNo);
			if(NULL == pTargetItemDef){break;}

			//
			XUI::CXUI_Wnd * pkSrcWnd = self->Parent()->GetControl(L"ICN_MATERIAL");
			if(NULL == pkSrcWnd){break;}

			SItemPos	kPos;
			pkSrcWnd->GetCustomData(&kPos, sizeof(kPos));

			PgBase_Item kItem;
			if(S_OK != pInv->GetItem(kPos, kItem)){break;}

			CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if(NULL == pItemDef){break;}

			int ChangeItemCount = 0;
			TBL_PAIR_KEY_INT const kKey(g_kGemStore.GetGemStoreType(),kItem.ItemNo());
			CONT_DEFANTIQUE::const_iterator Total_iter = pDefAntique->find(kKey);
			if(Total_iter != pDefAntique->end())
			{
				ChangeItemCount = Total_iter->second.kContArticles.size();
			}
			if(0 == ChangeItemCount){break;}

			int const iHaveNeedItemCount = (pItemDef->IsAmountItem() ? kItem.Count() : 1);
			CONT_DEF_ANTIQUE_ARTICLE::const_iterator Target_iter = Total_iter->second.kContArticles.begin();
			while(Target_iter != Total_iter->second.kContArticles.end())
			{
				if(dwTargetItemNo == Target_iter->second.first)
				{
					int const iNeedCount = Total_iter->second.iNeedCount;
					int const iAddCount = iHaveNeedItemCount / iNeedCount;
					int const iResultCount = Target_iter->second.second;

					SRenderTextInfo kRenderTextInfo;
					kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(iResultCount*iAddCount);
					kRenderTextInfo.kLoc = self->TotalLocation();
					kRenderTextInfo.wstrFontKey = FONT_TEXT;
					kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
					g_kUIScene.RenderText(kRenderTextInfo);
					break;
				}
				++Target_iter;
			}

			DrawIconDetail_AddRes(pTargetItemDef->ResNo(), ptPos);
			g_kUIScene.RenderIcon(pTargetItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
		}break;
	case KUIG_ITEM_MAKING_MAZE:
	case KUIG_COMMON_INV_POS:
	case KUIG_REDICE_CRAFT_OPT:
	case KUIG_REDICE_PET_OPT:
	case KUIG_ITEM_UNLOCK_SLOT:
	case KUIG_ENCHANT_SHIFT_SRC:
	case KUIG_ENCHANT_SHIFT_TGT:
	case KUIG_ITEM_PET_UPGRADE:
		{
			SItemPos	kPos;
			self->GetCustomData(&kPos, sizeof(kPos));

			PgInventory* pInv = pkPlayer->GetInven();
			if( pInv )
			{
				PgBase_Item kItem;
				if( S_OK == pInv->GetItem(kPos, kItem) )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if(pItemDef)
					{
						DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
						g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
					}
				}
			}
		}break;
	case KUIG_JS3_RES_ITEM:
		{
			std::pair<int,int> kData;
			self->GetCustomData(&kData, sizeof(kData));

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kData.first);
			if(pItemDef)
			{
				RenderIconItemCount(kData.second, pItemDef, ptPos);
				g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
			}
		}break;
	case KUIG_COMMON_ITEMNO:
	case KUIG_BOOK_RECOMMEND:
	case KUIG_DEFENCE_ITEM:
		{
			if( bDrawCount && rkIconInfo.iIconCount > 0 )
			{
				SRenderTextInfo kRenderTextInfo;
				kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(rkIconInfo.iIconCount);
				kRenderTextInfo.kLoc = ptPos;
				kRenderTextInfo.wstrFontKey = FONT_TEXT;
				kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
				g_kUIScene.RenderText(kRenderTextInfo);
			}

			DWORD dwItemNo = 0;
			self->GetCustomData(&dwItemNo, sizeof(dwItemNo));

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(dwItemNo);
			if(pItemDef)
			{
				DrawIconDetail_AddRes(pItemDef->ResNo(), ptPos);
				g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, self->GrayScale());
			}
		}break;
	case KUIG_ITEM_TRADE_MY:
	case KUIG_ITEM_TRADE_OTHER:
		{
			PgBase_Item const *pkItem = g_kTradeMgr.GetTradeItemAt(rkIconInfo.iIconKey, KUIG_ITEM_TRADE_MY==IconGrp);
			DrawIconDetail_sub(IconGrp, pkItem, ptPos);
			lwAccumlationExpCard::CheckUsingExpCardInfo(self, (pkItem)?(*pkItem):(PgBase_Item()));
		}break;
	case KUIG_VIEW_OTHER_EQUIP:
	case KUIG_VIEW_OTHER_EQUIP_CASH:
		{
			PgBase_Item kItem;
			if(S_OK == g_kOtherViewInv.GetItem(SItemPos( IconGrp == KUIG_VIEW_OTHER_EQUIP ? IT_FIT : IT_FIT_CASH, IconKey), kItem))
			{
				DrawIconDetail_sub(IconGrp, &kItem, ptPos);
			}
		}break;
	case KUIG_ITEM_MAIL_RECEIVED:
		{
			PgBase_Item const* pkItem = g_kMailMgr.GetItemAt(IconKey);
			DrawIconDetail_sub(IconGrp, pkItem, ptPos);
			lwAccumlationExpCard::CheckUsingExpCardInfo(self, (pkItem)?(*pkItem):(PgBase_Item()), false);
		}break;
	case KUIG_VIEW_ONLY_ITEM:
		{
			if( self )
			{
				PgBase_Item const kItemInfo = GetCustomDataAsSItem();
				DrawIconDetail_sub(IconGrp, &kItemInfo, ptPos);
			}
		}break;
	case KUIG_ITEM_COUNT:
		{
			if( self )
			{
				PgBase_Item const kItemInfo = GetCustomDataAsSItem();
				int const iCount = kItemInfo.Count();
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(kItemInfo.ItemNo());
				if (pItemDef == NULL)
				{
					return;
				}
				size_t const iMinimumAmount = 1;
				if( 0 < iCount
					&&	pItemDef->IsAmountItem()
					&&	iMinimumAmount < pItemDef->GetAbil(AT_MAX_LIMIT) )//소비형 이고 최대 겹쳐지는 수가 1 초과 일 경우
				{
					SRenderTextInfo kRenderTextInfo;
					kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(iCount);
					kRenderTextInfo.kLoc = ptPos;
					kRenderTextInfo.wstrFontKey = FONT_TEXT;
					kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
					g_kUIScene.RenderText(kRenderTextInfo);
				}
				g_kUIScene.RenderIcon(pItemDef->ResNo(), ptPos, false, 40, 40, (0<iCount)?(false):(true) );
			}
		}break;
	case KUIG_VIEW_ITEMNO_COUNT:
		{
			if ( self )
			{
				SSimpleItemInfo kSimpleItemInfo;
				if ( self->GetCustomData( &kSimpleItemInfo, sizeof(SSimpleItemInfo) ) )
				{
					DrawIconDetail_sub( kSimpleItemInfo, ptPos );
				}
			}
		}break;
	case KUIG_DEFENCE_SKILL:
		{
			int iResNo = 0;
			SGuardian_Skill kSkillInfo;
			self->GetCustomData(&kSkillInfo, sizeof(kSkillInfo));

			if(kSkillInfo.iSkillNo>0)
			{
				const CONT_DEFSKILL *pkContDefMap = NULL;
				g_kTblDataMgr.GetContDef(pkContDefMap);
				if( pkContDefMap )
				{
					CONT_DEFSKILL::const_iterator c_iter = pkContDefMap->find(kSkillInfo.iSkillNo);
					if( c_iter!=pkContDefMap->end() )
					{
						iResNo = c_iter->second.RscNameNo;
					}
				}
			}

			if( iResNo )
			{
				g_kUIScene.RenderIcon(iResNo, ptPos, false, GetWidth(), GetHeight(), self->GrayScale());
			}
		}break;
	case KUIG_DEFENCE_GUARDIAN:
		{
			int iResNo = 0;
			GuardianInfo kInfo;
			self->GetCustomData(&kInfo, sizeof(kInfo));

			if(kInfo.iMonsterNo>0)
			{
				GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
				CMonsterDef const *pDef = kMonsterDefMgr.GetDef(kInfo.iMonsterNo);
				if( pDef )
				{
					iResNo = pDef->GetAbil(AT_DEF_RES_NO);
				}
			}

			if( iResNo )
			{
				//g_kUIScene.RenderIcon(iResNo, ptPos, false, GetWidth(), GetHeight(), self->GrayScale());


				SRenderInfo kInfo;
				kInfo.bGrayScale = self->GrayScale();
				kInfo.kLoc = ptPos;
				kInfo.kSizedScale.ptSrcSize.x = GetWidth();
				kInfo.kSizedScale.ptSrcSize.y = GetHeight();
				kInfo.fScale = self->Scale();
				g_kUIScene.RenderIcon(iResNo, kInfo);
			}
		}break;
	case KUIG_MONSTER_BIG:
		{
			g_kUIScene.RenderIcon( GetCustomData<int>(), ptPos, false, GetWidth(), GetHeight(), self->GrayScale());
		}break;
	case KUIG_TRANSTOWER_FREE:
		{
			XUI::CXUI_Icon *pkIcon = dynamic_cast<XUI::CXUI_Icon *>(self);
			if(pkIcon)
			{
				if ( pkIcon->IconInfo().iIconResNumber )
				{
					g_kUIScene.RenderIcon( pkIcon->IconInfo().iIconResNumber, ptPos, false, GetWidth(), GetHeight(), self->GrayScale());
				}
			}
		}break;
	case KUIG_JOB_SKILL:
	case KUIG_JOB_SKILL3_ITEM:
		{
			PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
			if(pPlayer)
			{
				PgMySkill const * pMySkill = pPlayer->GetMySkill();
				if(pMySkill)
				{
					if( 0 != IconKey)
					{
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(IconKey);
						if(pkSkillDef)	
						{
							bool const bHaveSkill = pMySkill->IsExist(IconKey);
							if(bHaveSkill)
							{
								bool const bRet = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, IconKey);
								g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, !bRet);
							}
							else
							{
								g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, false);
							}
						}
						else
						{//도감,레시피의 경우
							GET_DEF(CItemDefMgr, kItemDefMgr);
							CItemDef const *pItemDef = kItemDefMgr.GetDef(IconKey);
							if(pItemDef)
							{
								int const iCustomValue1 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
								bool const bHaveIdx = JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iCustomValue1);
								g_kUIScene.RenderIcon(rkIconInfo.iIconResNumber, ptPos, false, 40, 40, !bHaveIdx);
							}
						}
					}
				}
			}
		}break;
	case KUIG_ITEM_SKILL:
		{
			int const iItemSkillNo = IconKey;
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
			if(pkSkillDef)
			{
				bool bIsUseable = ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo);
				g_kUIScene.RenderIcon(pkSkillDef->RscNameNo(), ptPos, false, 40, 40, !bIsUseable);
			}
		}break;
	}
}

void lwUIWnd::CompEquipToolTip(PgPlayer* pkPlayer, DWORD const& iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
	if (pItemDef == NULL)
		return;

	int const iGenderLimit = pItemDef->GetAbil(AT_GENDERLIMIT);
	if(0 == (iGenderLimit & pkPlayer->GetAbil(AT_GENDER)))//성별 검사.
	{
		return;
	}

	if (! IS_CLASS_LIMIT(pItemDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
	{
		return;
	}

	//	이건 장비중인거
	PgInventory* pkInv = pkPlayer->GetInven();
	if (!pkInv)	{ return; }

	PgBase_Item kEqupItem;

	if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, pItemDef->EquipPos()), kEqupItem))
	{
		lwPoint2 kPoint = (lwPoint2)POINT2();
		XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
		XUI::CXUI_Wnd *pkTooltip = XUIMgr.Get(_T("ToolTip2"));
		if (pCursor && pkTooltip)
		{
			kPoint.SetX(pCursor->Location().x + pCursor->Size().x + pkTooltip->Size().x);
			kPoint.SetY(pCursor->Location().y + pCursor->Size().y);
		}
		CallToolTip_SItem(&kEqupItem, kPoint, TBL_SHOP_IN_GAME::NullData(), "ToolTip_Equip", TTW(40060).c_str());
	}
}

void CallItemToolTipByPos(SItemPos const & rkPos, CXUI_Wnd *pkWnd, PgPlayer* pkPlayer)
{// CallIconToolTip() 함수를 믿고. NULL체크를 하지 않는다. 주의!!!
	PgBase_Item kItem;
	if(S_OK == pkPlayer->GetInven()->GetItem(rkPos, kItem))
	{
		CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)pkWnd->TotalLocation());
	}
}

void lwUIWnd::CallIconToolTip()
{
	CXUI_Icon *pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(!pkIconWnd){return;}

	const SIconInfo &kIconInfo = pkIconWnd->IconInfo();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	switch(kIconInfo.iIconGroup)
	{
	case KUIG_EQUIP:
	case KUIG_CONSUME:
	case KUIG_ETC:
	case KUIG_CASH:
	case KUIG_SAFE:
	case KUIG_SAFE_ADDITION:
	case KUIG_CASH_SAFE:
	case KUIG_FIT:
	case KUIG_FIT_CASH:
	case KUIG_RENTAL_SAFE1:
	case KUIG_RENTAL_SAFE2:
	case KUIG_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE1:
	case KUIG_SHARE_RENTAL_SAFE2:
	case KUIG_SHARE_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE4:
	case KUIG_PET:
		{
			PgBase_Item kItem;
			if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey), kItem))
			{
				CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;	
	case KUIG_GUILD_INV_VIEW:
		{
			PgBase_Item kItem;
			SItemPos kItemPos(g_kGuildMgr.GetGuildInvView(), kIconInfo.iIconKey);
			ConvertItemPosToGuildInvType(kItemPos);

			if(S_OK == g_kGuildMgr.GetInven()->GetItem(kItemPos, kItem))
			{
				CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_SYSTEM_INVEN:
		{
			PgBase_Item const *pkItem = g_kSystemInven.GetAt(kIconInfo.iIconKey);
			if(!PgBase_Item::IsEmpty(pkItem))
			{
				CallToolTip_SItem(pkItem, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_INV_VIEW:
		{
			PgBase_Item kItem;

			if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(g_iInvViewGrp, kIconInfo.iIconKey), kItem))
			{
				lwPoint2 kPoint = (lwPoint2)(POINT2)self->TotalLocation();
				ToolTipComp_SItem(kItem, kPoint);
			}
		}break;
	case KUIG_SKILLTREE:
		{
			PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(kIconInfo.iIconKey));
			if(pFound) 
			{				
#ifndef EXTERNAL_RELEASE
				bool const bUseDebug = lua_tinker::call<bool>("GetSkillDebugDialog");
				if(!bUseDebug)
				{
					CallSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation());
				}
				else
				{				
					lwPoint2 pt2 = (lwPoint2)(POINT2)self->TotalLocation();
					CallSkillAbilToolTip(pFound->GetOriginalSkillNo(), pt2);
				}
#else
				CallSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation());
#endif

			}
			else
			{
				CallSkillToolTip(g_kSkillTree.GetKeySkillNo(kIconInfo.iIconKey), (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_JOB_SKILL:
	case KUIG_JOB_SKILL3_ITEM:
		{
			if( 0 != kIconInfo.iIconKey)
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CSkillDef const* pSkillDef;
				pSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
				if(pSkillDef)
				{
					CallJobSkillToolTip(g_kSkillTree.GetKeySkillNo(kIconInfo.iIconKey), (lwPoint2)(POINT2)self->TotalLocation());
				}
				else
				{//도감,레시피의 경우
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pItemDef = kItemDefMgr.GetDef(kIconInfo.iIconKey);
					if( pItemDef )
					{
						CallToolTip_ItemNo(kIconInfo.iIconKey, (lwPoint2)(POINT2)self->TotalLocation());
					}
				}
			}
		}break;
	case KUIG_ITEM_SKILL:
		{
			int const iItemSkillNo = kIconInfo.iIconKey;
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
			if(pkSkillDef)
			{
				CallSkillToolTip(iItemSkillNo, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_CONSUME_PET:
	case KUIG_FIT_PET:
		{
			PgInventory *pkPetInven = GetMySelectedPetInven();
			if ( pkPetInven )
			{
				PgBase_Item kItem;
				if(S_OK == pkPetInven->GetItem(SItemPos(KUIG_FIT_PET==kIconInfo.iIconGroup?KUIG_FIT : KUIG_CONSUME, kIconInfo.iIconKey), kItem))
				{
					CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
				}
			}
		}break;
	case KUIG_SKILLTREE_PET:
		{
			if(kIconInfo.iIconKey)
			{	
				CallSkillToolTip_Pet(kIconInfo.iIconKey, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_QUICK_INVEN:
		{
			SQuickInvInfo kQuickInvInfo;
			PgQuickInventory* pkQInv = pkPlayer->GetQInven();
			const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다

			if(S_OK == hRet)
			{
				switch(kQuickInvInfo.Grp())
				{
				case KUIG_CASH:
				case KUIG_CONSUME:
				case KUIG_PET:
					{
						int const iItemNo = kQuickInvInfo.ID();

						// 						GET_DEF(CItemDefMgr, kItemDefMgr);
						// 						CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
						// 						if(pItemDef)
						// 						{
						size_t const total_count = pkPlayer->GetInven()->GetTotalCount(iItemNo);

						PgInventory* pkInv = pkPlayer->GetInven();
						if( !pkInv )
						{
							return;
						}

						SItemPos kItemPos;
						if( S_OK != pkInv->GetFirstItem(static_cast<EInvType>(kQuickInvInfo.Grp()), iItemNo, kItemPos) )
						{
							return;
						}

						PgBase_Item kItem;
						if( S_OK != pkInv->GetItem(kItemPos, kItem) )
						{
							return;
						}

						CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());//여기서 툴팁은 갯수가 이상 할 수 있지.
						//						}
					}break;
				case KUIG_SKILLTREE:
					{
						int const iSkillNo = kQuickInvInfo.ID();

						PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
						if(pFound) 
						{
#ifndef EXTERNAL_RELEASE
							bool const bUseDebug = lua_tinker::call<bool>("GetSkillDebugDialog");
							if(!bUseDebug)
							{
								CallQuickSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation(), pFound->GetOriginalSkillNo(), false, true);
							}
							else
							{
								lwPoint2 pt2 = (lwPoint2)(POINT2)self->TotalLocation();
								CallSkillAbilToolTip(pFound->GetOriginalSkillNo(), pt2);
							}
#else
							CallQuickSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation(), pFound->GetOriginalSkillNo(), false, true);
#endif
						}
						else
						{
							CallSkillToolTip(g_kSkillTree.GetKeySkillNo(iSkillNo), (lwPoint2)(POINT2)self->TotalLocation());
						}
					}break;
				case KUIG_JOB_SKILL:
				case KUIG_JOB_SKILL3_ITEM:
					{
						if( 0 != kQuickInvInfo.ID())
						{
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pSkillDef;
							pSkillDef = kSkillDefMgr.GetDef(kQuickInvInfo.ID());
							if(pSkillDef)
							{
								CallJobSkillToolTip(g_kSkillTree.GetKeySkillNo(kQuickInvInfo.ID()), (lwPoint2)(POINT2)self->TotalLocation());
							}
							else
							{//도감,레시피의 경우
								GET_DEF(CItemDefMgr, kItemDefMgr);
								CItemDef const* pItemDef = kItemDefMgr.GetDef(kQuickInvInfo.ID());
								if( pItemDef )
								{
									CallToolTip_ItemNo(kQuickInvInfo.ID(), (lwPoint2)(POINT2)self->TotalLocation());
								}
							}
						}
					}break;
				case KUIG_ITEM_SKILL:
					{
						int const iItemSkillNo = kQuickInvInfo.ID();
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
						if(pkSkillDef)
						{
							CallSkillToolTip(iItemSkillNo, (lwPoint2)(POINT2)self->TotalLocation());
						}
					}break;
				case KUIG_SKILLTREE_PET:
					{
						int const iSkillNo = kQuickInvInfo.ID();
						PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo), true);
						//if(pFound) 
						{
							lwPoint2 pt2 = (lwPoint2)(POINT2)self->TotalLocation();
							CallSkillToolTip_Pet(pFound?pFound->m_ulSkillNo:iSkillNo, pt2); 
						}
					}break;
				case KUIG_SKILLSET_BASIC:
					{
						lwSkillSet::lwCallSkillSetToolTip_Basic(kQuickInvInfo.ID());
					}break;
				case KUIG_SKILLSET_SETICON:
					{
						lwSkillSet::lwCallSkillSetToolTip(kQuickInvInfo.ID());
					}break;
				default:
					{//올라가면 안될께 올라왔다.
						assert(NULL);
					}break;
				}
			}
		}break;
	case KUIG_SKILLSET_BASIC:
		{
			lwSkillSet::lwCallSkillSetToolTip_Basic(kIconInfo.iIconKey);
		}break;
	case KUIG_SKILLSET_SETICON:
		{
			lwSkillSet::lwCallSkillSetToolTip(kIconInfo.iIconKey);
		}break;
	case KUIG_SKILLSET_SKILLICON:
		{
			SIconInfo kData;
			self->GetCustomData(&kData, sizeof(kData));

			int const iSkillNo = kData.iIconKey;
			switch(kData.iIconGroup)
			{
			case KUIG_SKILLTREE:
				{
					PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
					if(pFound) 
					{
						CallQuickSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation(), pFound->GetOriginalSkillNo(), false, true);
					}
				}break;
			case KUIG_SKILLTREE_PET:
				{
					lwPoint2 pt2 = (lwPoint2)(POINT2)self->TotalLocation();
					CallSkillToolTip_Pet(iSkillNo, pt2);
				}break;
			}
		}break;
	case KUIG_EFFECT:
		{
			int iEffectID = GetCustomData<int>();
			if(iEffectID > 0)
			{
				PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iEffectID));
				if(pFound) 
				{
					CallQuickSkillToolTip(pFound,(lwPoint2)(POINT2)self->TotalLocation(), iEffectID);
				}
				else//스킬에서 못찾으면
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iEffectID);
					if(pkSkillDef)
					{
						PgSkillTree::stTreeNode kTempNode(iEffectID, iEffectID, pkSkillDef, true);
						CallQuickSkillToolTip(&kTempNode,(lwPoint2)(POINT2)self->TotalLocation(), iEffectID, true);
					}
				}
			}
			else
			{
				self->Text(std::wstring(_T("")));
			}
		}break;
	case KUIG_STORE:
	case KUIG_STOCK_STORE:
		{
			PgBase_Item const* pkItem = g_kViewStore.GetViewItem(kIconInfo.iIconKey);

			if(!PgBase_Item::IsEmpty(pkItem))
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(pkItem->ItemNo(), kItemInfo))
				{
					lwPoint2 kPoint = (lwPoint2)(POINT2)self->TotalLocation();
					g_bIsEqComp = true;
					PgBase_Item kTempItem(*pkItem);
					if( 0 <= kItemInfo.bTimeType
						&&	0 != kItemInfo.iUseTime )
					{
						SYSTEMTIME kSysTime;
						g_kEventView.GetLocalTime(&kSysTime);
						BM::PgPackedTime kGenTime;
						kGenTime = kSysTime;
						kTempItem.CreateDate(kGenTime);//4 35	생성날짜
						kTempItem.SetUseTime(kItemInfo.bTimeType, kItemInfo.iUseTime);
					}
					CallToolTip_SItem(&kTempItem, kPoint, kItemInfo);
					g_bIsEqComp = false;

					CompEquipToolTip(pkPlayer, pkItem->ItemNo());
				}
			}
		}break;
	case KUIG_ITEM_REPAIR:
		{
			CallItemToolTipByPos(g_kItemRepair.GetSrcPos(), self, pkPlayer);
		}break;
	case KUIG_MONSTER:
	case KUIG_MONSTER_BIG:
		{
			//if (kIconInfo.iIconResNumber != 43)
			{
				CallMonsterToolTip(kIconInfo.iIconKey, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_DEFENCE_GUARDIAN:
		{
			GuardianInfo kInfo;
			self->GetCustomData(&kInfo, sizeof(kInfo));
			CallGuardianToolTip(kInfo.iGuardianNo, (lwPoint2)(POINT2)self->TotalLocation());
		}break;	
	case KUIG_DEFENCE_SKILL:
		{
			SGuardian_Skill kSkillInfo;
			int iSkillNo = 0;
			self->GetCustomData(&kSkillInfo, sizeof(kSkillInfo));
			if(kSkillInfo.iSkillNo)
			{
				//CallSkillToolTip(kSkillInfo.iSkillNo, (lwPoint2)(POINT2)self->TotalLocation());
				CallStratagySkillToolTip(kSkillInfo.iGuardianNo, kSkillInfo.iSkillNo, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_ITEM_TRADE_MY:
	case KUIG_ITEM_TRADE_OTHER:
		{
			int const iIdx = self->BuildIndex();

			PgBase_Item const *pkItem = g_kTradeMgr.GetTradeItemAt(iIdx, kIconInfo.iIconGroup==(int)KUIG_ITEM_TRADE_MY);
			if (pkItem)
			{
				lwPoint2 kPoint = (lwPoint2)(POINT2)self->TotalLocation();

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
				if( pItemDef->IsType(ITEM_TYPE_AIDS) )
				{
					CallToolTip_SItem(pkItem, kPoint);
				}
				else
				{
					g_bIsEqComp = true;
					CallToolTip_SItem(pkItem, kPoint);
					g_bIsEqComp = false;

					CompEquipToolTip(pkPlayer, pkItem->ItemNo());
				}
			}
		}break;
	case KUIG_ITEM_MAKING:
		{
			PgBase_Item const* pkItem = g_kItemMaking.GetMouseOverSrcItem(pkIconWnd);
			if (pkItem)
			{
				CallToolTip_SItem(pkItem, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_ITEM_MAKING_OUT:
		{
			PgBase_Item const* pkItem = g_kItemMaking.GetMouseOverDestItem(pkIconWnd);
			if (pkItem)
			{
				CallToolTip_SItem(pkItem, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_VIEW_OTHER_EQUIP:
	case KUIG_VIEW_OTHER_EQUIP_CASH:
		{
			PgBase_Item kItem;

			if(S_OK != g_kOtherViewInv.GetItem(SItemPos(kIconInfo.iIconGroup == KUIG_VIEW_OTHER_EQUIP ? IT_FIT : IT_FIT_CASH, kIconInfo.iIconKey), kItem))
			{
				break;
			}

			if( kIconInfo.iIconGroup != KUIG_VIEW_OTHER_EQUIP )
			{
				CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation(),TBL_SHOP_IN_GAME::NullData(),NULL,NULL,true);
				break;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if (pItemDef == NULL)
			{
				break;
			}

			lwPoint2 kPoint = (lwPoint2)(POINT2)self->TotalLocation();

			int iFitPos = pItemDef->EquipPos();;
			g_bIsEqComp = true;
			CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation(), TBL_SHOP_IN_GAME::NullData(), NULL, NULL, true);
			g_bIsEqComp = false;
			CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if (!pkPlayer) {break;}

			int const iGenderLimit = pItemDef->GetAbil(AT_GENDERLIMIT);
			if(0 == (iGenderLimit & pkPlayer->GetAbil(AT_GENDER)))//성별 검사.
			{
				break;
			}

			if (! IS_CLASS_LIMIT(pItemDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
			{
				break;
			}

			//	이건 장비중인거
			PgInventory* pkInv = pkPlayer->GetInven();
			if (!pkInv)	{ break; }

			PgBase_Item kEqupItem;
			if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, iFitPos), kEqupItem))
			{
				XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
				XUI::CXUI_Wnd *pkTooltip = XUIMgr.Get(_T("ToolTip2"));
				if (pCursor && pkTooltip)
				{
					kPoint.SetX(pCursor->Location().x + pCursor->Size().x + pkTooltip->Size().x);
					kPoint.SetY(pCursor->Location().y + pCursor->Size().y);
				}
				CallToolTip_SItem(&kEqupItem, kPoint, TBL_SHOP_IN_GAME::NullData(), "ToolTip_Equip", TTW(40060).c_str());
			}
		}break;
	case KUIG_ITEM_UPGRADE_DEST:
		{
			if(!PgBase_Item::IsEmpty(&g_kItemPlusUpgradeMgr.GetResultItem()))
			{
				CallToolTip_SItem(&g_kItemPlusUpgradeMgr.GetResultItem(), (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_ITEM_PLUS_UPGRADE_SRC:
		{
			CallItemToolTipByPos(g_kItemPlusUpgradeMgr.GetSrcItemPos(), self, pkPlayer);
		}break;	
	case KUIG_ITEM_RARITY_UPGRADE_SRC:
		{
			CallItemToolTipByPos(g_kItemRarityUpgradeMgr.GetSrcItemPos(), self, pkPlayer);
		}break;	
	case KUIG_SOCKET_SYSTEM:
		{
			CallItemToolTipByPos(g_kSocketSystemMgr.GetSrcItemPos(), self, pkPlayer);
		}break;
	case KUIG_ITEMCONVERT_SYSTEM:
		{
			CallItemToolTipByPos(g_kItemConvertSystemMgr.GetSrcItemPos(), self, pkPlayer);
		}break;
	case KUIG_ITEM_MAIL_RECEIVED:
		{	
			PgBase_Item const* pkItem = g_kMailMgr.GetItemAt( kIconInfo.iIconKey);
			CallToolTip_SItem(pkItem, (lwPoint2)(POINT2)self->TotalLocation());
		}break;
	case KUIG_ITEM_MAIL:
		{
			CallItemToolTipByPos(g_kMailMgr.SendItemPos(), self, pkPlayer);
		}break;
	case KUIG_VIEW_ONLY_ITEM:
		{
			if( self )
			{
				PgBase_Item const kItemInfo = GetCustomDataAsSItem();
				CallToolTip_SItem(&kItemInfo, lwPoint2(self->TotalLocation().x, self->TotalLocation().y));
			}
		}break;
	case KUIG_ITEM_COUNT:
		{//추출기면 툴팁은 그냥 보여주자.
			if( self )
			{
				PgBase_Item const kItemInfo = GetCustomDataAsSItem();
				if(kItemInfo.IsEmpty())
				{
					CallToolTip_ItemNo(kItemInfo.ItemNo(), lwPoint2(self->TotalLocation().x, self->TotalLocation().y));
				}
				else
				{
					CallToolTip_SItem(&kItemInfo, lwPoint2(self->TotalLocation().x, self->TotalLocation().y));
				}
			}
		}break;
	case KUIG_VIEW_ITEMNO_COUNT:
		{
			if ( self )
			{
				SSimpleItemInfo kItemInfo;
				if ( self->GetCustomData( &kItemInfo, sizeof(SSimpleItemInfo) ) )
				{
					CallToolTip_ItemNo( kItemInfo.iItemNo, lwPoint2(self->TotalLocation().x, self->TotalLocation().y) );
				}
			}
		}break;
	case KUIG_COMMON_GUID_TO_GRAY:
		{
			BM::GUID kGuid = self->OwnerGuid();

			PgInventory* pkInv = pkPlayer->GetInven();
			if( pkInv )
			{
				SItemPos	kPos;
				PgBase_Item	kItem;
				if( S_OK == pkInv->GetItem(kGuid, kItem, kPos) )
				{
					CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
				}
				else
				{
					DWORD dwItemNo = 0;
					self->GetCustomData(&dwItemNo, sizeof(dwItemNo));
					CallToolTip_ItemNo(dwItemNo, (lwPoint2)(POINT2)self->TotalLocation());
				}
			}
		}break;
	case KUIG_ITEM_MAKING_MAZE:
	case KUIG_COMMON_INV_POS:
	case KUIG_ANTIQUE_SRC:
	case KUIG_REDICE_CRAFT_OPT:
	case KUIG_REDICE_PET_OPT:
	case KUIG_ITEM_UNLOCK_SLOT:
	case KUIG_ENCHANT_SHIFT_SRC:
	case KUIG_ENCHANT_SHIFT_TGT:
	case KUIG_ITEM_PET_UPGRADE:
		{
			SItemPos	kPos;
			self->GetCustomData(&kPos, sizeof(kPos));

			PgInventory* pInv = pkPlayer->GetInven();
			if( pInv )
			{
				PgBase_Item kItem;
				if( S_OK == pInv->GetItem(kPos, kItem) )
				{
					CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
				}
			}
		}break;
	case KUIG_JS3_RES_ITEM:
		{
			std::pair<int,int> kData;
			self->GetCustomData(&kData, sizeof(kData));

			CallToolTip_ItemNo(kData.first, (lwPoint2)(POINT2)self->TotalLocation());
		}break;
	case KUIG_ITEM_MAKING_COOK:
	case KUIG_COMMON_ITEMNO:
	case KUIG_ANTIQUE_TGT:
	case KUIG_EXP_POTION:
	case KUIG_EXP_POTION_PET:
		{
			DWORD dwItemNo = 0;
			self->GetCustomData(&dwItemNo, sizeof(dwItemNo));

			DWORD iCloseToolTipItemNo = 90000000;
			if( dwItemNo != iCloseToolTipItemNo ) // "꽝" 아이템 툴팁을 없애준다
			{
				CallToolTip_ItemNo(dwItemNo, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	case KUIG_TREASURE_CHEST_KEY:
	case KUIG_ITEM_PLUS_UPGRADE_INSURENCE:
	case KUIG_ITEM_PLUS_UPGRADE_PROBABILITY:
	case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:
	case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:
	case KUIG_DEFENCE_ITEM:
	case KUIG_MANUFACTURE_MATERIAL:
	case KUIG_BOOK_RECOMMEND:
		{
			DWORD dwItemNo = 0;
			self->GetCustomData(&dwItemNo, sizeof(dwItemNo));
			CallToolTip_ItemNo(dwItemNo, (lwPoint2)(POINT2)self->TotalLocation());
		}break;
	case KUIG_ITEM_BIND:
		{// 자물쇠 UI에 올린 아이템의 툴팁 띄우기
			CallItemToolTipByPos(g_kItemBindMgr.GetTargetItemPosFromInv(), self, pkPlayer);
		}break;
	case KUIG_ITEM_UNBIND:
		{// 열쇠 UI에 올린 아이템의 툴팁 띄우기
			CallItemToolTipByPos(g_kItemUnbindMgr.GetTargetItemPosFromInv(), self, pkPlayer);
		}break;
	case KUIG_TRANSTOWER_FREE:
		{
			CallToolTip_ItemNo( kIconInfo.iIconResNumber, static_cast<lwPoint2>(static_cast<POINT2>(self->TotalLocation())) );
		}break;
	case KUIG_ITEM_RARITY_BUILDUP_SRC:
		{
			CallItemToolTipByPos(g_kItemRarityBuildUpMgr.GetSrcItemPos(), self, pkPlayer);
		}break;
	case KUIG_ITEM_RARITY_AMPLIFY_SRC:
		{
			CallItemToolTipByPos(g_kItemRarityAmplify.GetSrcItemPos(), self, pkPlayer);
		}break;
	case KUIG_COMBO_SKILL:
		{
			int iSkillNo = 0;
			self->GetCustomData( &iSkillNo, sizeof(iSkillNo) );
			lwPoint2 kPoint = (lwPoint2)(POINT2)self->TotalLocation();
			CallComboSkillToolTip(iSkillNo, kPoint);
			
		}break;
	case KUIG_ITEM_INFALLIBLE_SELECTION_SRC:
	case KUIG_ITEM_RARITY_METERIAL_SRC:
		{
			DWORD dwItemNo = 0;
			self->GetCustomData(&dwItemNo, sizeof(dwItemNo));

			if( dwItemNo )
			{
				CallToolTip_ItemNo(dwItemNo, (lwPoint2)(POINT2)self->TotalLocation());
			}
		}break;
	}
}


void lwUIWnd::CallImageIconToolTip(int const iType)
{
	PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	if( self )
	{
		switch( iType )
		{
		case EIIT_CUSTOM_INV_POS:
			{
				SItemPos	kItemPos;
				self->GetCustomData(&kItemPos, sizeof(kItemPos));

				int iItemNo = 0;
				PgBase_Item kItem;
				if( S_OK == pkInv->GetItem(kItemPos, kItem) )
				{
					CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)self->TotalLocation());
				}
			}break;
		default:
			{
				return;
			}break;
		}
	}
}

void lwUIWnd::DrawIcon()
{
	CXUI_Icon* pkIcon = dynamic_cast<CXUI_Icon*>(self);

	if(pkIcon)
	{
		const SIconInfo &kIconInfo = pkIcon->IconInfo();

		if(KUIG_KEY_SET == kIconInfo.iIconGroup)
		{
			//char szKey[200] = {0, };
			int iKey = 0;
			if(self->GetCustomData(&iKey, sizeof(iKey)))
			{
				//if(!szKey)
				std::string kKeyStr;
				if(!g_kGlobalOption.GetKeynoToKeystr(iKey, kKeyStr))
				{
					NILOG(PGLOG_ERROR, "CXUI_Wnd.. Custom Data is NULL");
					PG_ASSERT_LOG(0);
					return;
				}

				int const iUKey = g_kGlobalOption.GetValue(XML_ELEMENT_KEYSET, kKeyStr);
				if(kIconInfo.iIconKey != iUKey)
				{
					SIconInfo kTempIconInfo = kIconInfo;
					kTempIconInfo.iIconKey = iUKey;
					pkIcon->SetIconInfo(kTempIconInfo);
				}
			}
			else
			{
				NILOG(PGLOG_ERROR, "[DrawIcon] Can't get customedata this wnd"); 
				assert(0); 
				return;
			}
		}
		else if(KUIG_INV_VIEW == kIconInfo.iIconGroup 
			|| IT_SAFE_ADDITION == kIconInfo.iIconGroup )	// 인벤 일 경우
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return;}
			PgBase_Item kItem;

			DWORD dwItemNo = 0;
			int iInvGroup = KUIG_INV_VIEW == kIconInfo.iIconGroup ? lwGetInvViewGroup(): kIconInfo.iIconGroup;
			SItemPos const kItemPos(iInvGroup, kIconInfo.iIconKey);
			if(S_OK == pkPlayer->GetInven()->GetItem(kItemPos, kItem))
			{
				dwItemNo = kItem.ItemNo();
			}	

			std::pair<InvenMap::iterator, bool> ib = g_InvenMap.insert(std::make_pair(pkIcon->ID(), dwItemNo));

			if (lwGetReqInvWearableUpdate())	//강제로 업데이트
			{
				IsWearable(kIconInfo, pkIcon, &kItem, kItemPos);//체크
				if (pkPlayer->GetInven()->GetMaxIDX((EInvType)lwGetInvViewGroup()) <= kIconInfo.iIconKey + 1)	//0베이스라서 1더하자
				{
					lwSetReqInvWearableUpdate(false);
				}
			}
			else
			{
				if (!ib.second)
				{
					if ((ib.first)->second != dwItemNo)
					{
						IsWearable(kIconInfo, pkIcon, &kItem, kItemPos);//체크
						(ib.first)->second = dwItemNo;
					}
				}
				else
				{
					IsWearable(kIconInfo, pkIcon, &kItem, kItemPos);//체크
				}
			}
			// 자물쇠 마크 표시
			CXUI_Wnd* pkParentWnd = pkIcon->Parent();				
			CXUI_Wnd* pkBindMark=NULL;
			if(pkParentWnd)
			{// 자물쇠 마크 폼 얻기
				BM::vstring kBindMarkForm(_T("FRM_BIND_MARK"));				
				kBindMarkForm+=pkIcon->BuildIndex();
				pkBindMark = pkParentWnd->GetControl(kBindMarkForm);				
			}
			if(pkBindMark)
			{
				if(kItem.IsEmpty())
				{// 아이템이 없으면 마크를 감춤
					pkBindMark->Visible(false);
				}
				else
				{
					SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
					pkBindMark->Visible(kEhtInfo.IsBinding());
				}
			}
			lwAccumlationExpCard::CheckUsingExpCardInfo(pkIcon, kItem, true, true);

		}
		else if(KUIG_ITEM_MAIL == kIconInfo.iIconGroup)
		{
			lwDisplayMailSrcIcon(lwUIWnd(pkIcon));
			return;
		}

		DrawIconDetail(kIconInfo.iIconGroup, kIconInfo.iIconKey, pkIcon->TotalLocation(), kIconInfo, true);
	}
}

void lwUIWnd::DrawImageIcon(int const iType)
{
	PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	if( self )
	{
		switch( iType )
		{
		case EIIT_CUSTOM_INV_POS:
			{
				SItemPos	kItemPos;
				self->GetCustomData(&kItemPos, sizeof(kItemPos));

				int iItemNo = 0;
				PgBase_Item kItem;
				if( S_OK == pkInv->GetItem(kItemPos, kItem) )
				{
					iItemNo = kItem.ItemNo();
				}
				PgUIUtil::DrawIconToItemNo(self, iItemNo);
			}break;
		default:
			{
				return;
			}break;
		}
	}
}

void lwUIWnd::DrawCursorIcon()
{
	CXUI_Cursor* pkCursor = dynamic_cast<CXUI_Cursor*>(self);

	if(pkCursor)
	{
		const SIconInfo &kIconInfo = pkCursor->IconInfo();

		static const POINT2 ptAdd(-24,-24);

		POINT2 ptTemp(XUIMgr.MousePos().x, XUIMgr.MousePos().y);

		DrawIconDetail(kIconInfo.iIconGroup, kIconInfo.iIconKey,  ptTemp + ptAdd, kIconInfo, false);
	}
}

void lwUIWnd::DrawCoolTimeIcon()
{
	XUI::CXUI_Wnd *pkParent = self->Parent();
	CXUI_Icon* pkIcon = dynamic_cast<CXUI_Icon*>(pkParent);

	if( !pkParent || !pkIcon )
	{
		return;
	}

	const SIconInfo &kIconInfo = pkIcon->IconInfo();
	switch(kIconInfo.iIconGroup)
	{
	case KUIG_DEFENCE_SKILL:
		{
			SGuardian_Skill kSkillInfo;
			pkParent->GetCustomData(&kSkillInfo, sizeof(kSkillInfo));

			XUI::CXUI_Wnd *pkImg = pkParent->GetControl(L"FRM_COOTIME_IMG");
			if( pkImg )
			{
				pkImg->ImgSize(POINT2(pkImg->ImgSize().x, 0));
				pkImg->Visible(false);
				self->Text(_T(""));
			}

			if( pkImg && kSkillInfo.kGuardianGuid.IsNotNull() )
			{
				CEffect* pkEffect = GetGuardianSkill(kSkillInfo.iSkillNo, kSkillInfo.kGuardianGuid);
				float fRemainCoolTime = 0.f;
				float fTotalCoolTime = 0.f;
				if(pkEffect)
				{
					DWORD dwMainTime = 0;
					int const iDurTime = std::max<int>(0, pkEffect->GetDurationTime());
					DWORD const dwEndTime = pkEffect->GetEndTime();
					if (dwEndTime <= 0)
					{
						dwMainTime = std::max<DWORD>(0, iDurTime - static_cast<DWORD>(pkEffect->GetTime()));
					}
					else if (0 < iDurTime)
					{
						DWORD const dwServerElapsedTime32 = g_kEventView.GetServerElapsedTime();
						dwMainTime = std::max<DWORD>(0, dwEndTime - dwServerElapsedTime32);
					}

					fRemainCoolTime = dwMainTime/1000.f;
					fTotalCoolTime = iDurTime/1000.f;
				}

				if (0.0f<fRemainCoolTime && fTotalCoolTime > 0)
				{
					if (10.0f<fRemainCoolTime)
					{
						SetFontColorRGBA(255,255,0, 255);
					}
					else
					{
						SetFontColorRGBA(255,100,128, 255);
					}

					int iText = 90002;

					const POINT2 &rkSize = self->Size();
					float fCoolRate = __max(0.0f, fRemainCoolTime/fTotalCoolTime*rkSize.y);
					fCoolRate = __min(fCoolRate, rkSize.y);

					if (3600.0f<=fRemainCoolTime)
					{
						iText+=2;
						fRemainCoolTime = fRemainCoolTime/3600.0f;
					}
					else if (60.0f<=fRemainCoolTime)
					{
						iText+=1;
						fRemainCoolTime = fRemainCoolTime/60.0f;
					}

					pkImg->Visible(true);
					pkImg->Location(POINT2(pkImg->Location().x, rkSize.y - fCoolRate));
					pkImg->ImgSize(POINT2(pkImg->ImgSize().x, fCoolRate));

					BM::vstring vStr(static_cast<int>(fRemainCoolTime));
					vStr += TTW(iText);
					self->Text(vStr);
				}
			}
		}break;
	}
}

lwUIWnd::EICON_ACTION_RESULT lwUIWnd::IconDoAction()
{
	CXUI_Icon *pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(!pkIconWnd){return EIAR_FAIL;}

	const SIconInfo &kIconInfo = pkIconWnd->IconInfo();

	SItemPos kCasterPos(kIconInfo.iIconGroup, kIconInfo.iIconKey);//타겟 아이템

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return EIAR_FAIL;}

	switch(kCasterPos.x)
	{
	case KUIG_CASH:
	case KUIG_CASH_SAFE:
		{
			if( !lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true) )
			{
				break;
			}

			SItemPos const kCasterPos(kIconInfo.iIconGroup, kIconInfo.iIconKey);
			PgBase_Item kCasterItem;
			if(S_OK == pkPlayer->GetInven()->GetItem(kCasterPos, kCasterItem))
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const* pItemDef = kItemDefMgr.GetDef(kCasterItem.ItemNo());
				if (pItemDef && pItemDef->IsAmountItem())
				{
					SCalcInfo kInfo;
					kInfo.eCallType = CCT_ITEM_DIVIDE;
					kInfo.iBasePrice = 0;
					kInfo.kGuid = kCasterItem.Guid();
					kInfo.iItemNo = kCasterItem.ItemNo();
					kInfo.kItemPos = kCasterPos;
					CallCalculator(kInfo);
				}
			}
		}break;
	case KUIG_PET:
	case KUIG_INV_VIEW:
		{
			int iNowViewGrp = g_iInvViewGrp;
			if(KUIG_PET==kCasterPos.x)
			{
				iNowViewGrp = IT_PET;
				if(!pkPlayer)	{return EIAR_FAIL;}
			}
			SItemPos kViewerItemPos(iNowViewGrp, kIconInfo.iIconKey);
			PgBase_Item kItem;
			HRESULT const kRet = pkPlayer->GetInven()->GetItem(kViewerItemPos, kItem);
			if( S_OK != kRet )
			{
				break;
			}
			else
			{
				if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
				{
					lwAddWarnDataTT(400732);
					break;
				}
			}

			CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_SHOP"));

			bool bIsStockShop = false;
			if(!pWnd)
			{
				pWnd = XUIMgr.Get(_T("FRM_SHOP_STOCK"));
				if(pWnd)
				{
					bIsStockShop = true;
				}
			}
			if(!pWnd)
			{
				pWnd = XUIMgr.Get(_T("FRM_SHOP_JOB"));
			}


			if(pWnd)
			{//상점 UI 떠있으면 판매.
				if(bIsStockShop)
				{
					PgStore::CONT_GODDS::value_type kOut;
					if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kOut))
					{
						DoItemSell(kViewerItemPos, &kItem, true, kOut.iPrice);						
					}
					else
					{//판매 할 수 없음.
						lwAddWarnDataTT(400195);						
					}
				}
				else
				{
					DoItemSell(kViewerItemPos, &kItem);					
				}
			}
			else
			{
				CXUI_Wnd* pWndPetUpgrade = XUIMgr.Get(_T("SFRM_PET_UPGRADE"));
				if(pWndPetUpgrade)
				{ //펫 전직 창이 떠있다면 전직 진행
					lwPetUpgrade::SetSrcItem(kViewerItemPos);
					break;
				}

				if(!PgBase_Item::IsEmpty(&kItem))
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if(pItemDef)
					{
						if( g_pkWorld && pItemDef->GetAbil(AT_FURNITURE_TYPE) )// 가구이면
						{
							if( kItem.GetUseAbleTime() < 0 )// 기간이 지났으면
							{
								lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
								break;
							}

							if(g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 내부
							{
								if( false == pItemDef->IsType(ITEM_TYPE_MYHOME_IN) )
								{
									lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405174, true);
									break;
								}

								if( !g_pkWorld->GetHome() || !g_pkWorld->GetHome()->IsMyHome() )
								{
									lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405173, true);
									break;
								}

								if( g_pkWorld->GetHome() )
								{
									if( g_pkWorld->GetHome()->IsArrangeMode() == true )
									{
										XUIMgr.Close(L"Inv");
										break;
									}
									if( g_pkWorld->GetHome()->GetHomeUnit() )
									{
										int iState = g_pkWorld->GetHome()->GetHomeUnit()->GetAbil(AT_MYHOME_STATE);
										if( MAS_NOT_BIDDING == iState )
										{
											int iFurnitureType = pItemDef->GetAbil(AT_FURNITURE_TYPE);
											eEquipLimit equipLimit = static_cast<eEquipLimit>(pItemDef->GetAbil(AT_EQUIP_LIMIT));
											if( EQUIP_LIMIT_HAIR == equipLimit || FURNITURE_TYPE_WALL ==iFurnitureType || FURNITURE_TYPE_FLOOR == iFurnitureType )
											{
												PLAYER_ABIL kInfo;
												g_pkWorld->GetHome()->GetHomeUnit()->GetPlayerAbil(kInfo);
												if( kItem.ItemNo() == kInfo.iPants || kItem.ItemNo() == kInfo.iShoes || kItem.ItemNo() == kInfo.iHairStyle )// 같은 아이템이면
												{
													lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405171, true);
													break;
												}
												if( EQUIP_LIMIT_HAIR == equipLimit )
												{
													CItemDef const* pDefStyle = kItemDefMgr.GetDef(kInfo.iHairStyle);
													if( pDefStyle )
													{
														if( pDefStyle->GetAbil(AT_MYHOME_ROOM_X) > pItemDef->GetAbil(AT_MYHOME_ROOM_X) ||
															pDefStyle->GetAbil(AT_MYHOME_ROOM_Y) > pItemDef->GetAbil(AT_MYHOME_ROOM_Y) )
														{
															lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405172, true);
															break;
														}
													}
												}

												g_pkWorld->GetHome()->HomeItemUseStyleItem(kViewerItemPos);
												XUIMgr.Close(L"Inv");
											}
											else
											{
												if( FURNITURE_TYPE_OBJECT_ATTACH == iFurnitureType )
												{
													if( g_pkWorld->GetHome()->IsHaveSetPropPlace() )
													{
														if( g_pkWorld->GetHome()->AddFurniturebyInven( kItem.ItemNo(), kItem.Guid()) )
														{
															XUIMgr.Call(L"FRM_MYHOME_EDIT_MODE");
															XUIMgr.Close(L"Inv");
														}
													}
													else
													{
														lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405170, true);
													}
												}
												else
												{
													if( g_pkWorld->GetHome()->AddFurniturebyInven( kItem.ItemNo(), kItem.Guid()) )
													{
														XUIMgr.Call(L"FRM_MYHOME_EDIT_MODE");
														XUIMgr.Close(L"Inv");
													}
												}
											}
										}
										else
										{
											lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799789, true);
										}
									}
								}
							}
							else if(g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
							{
								if( false == pItemDef->IsType(ITEM_TYPE_MYHOME_OUT) )
								{
									lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405175, true);
									break;
								}

								BM::GUID kGuid;
								if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
								{
									PgPilot* pkPilot = g_kPilotMan.FindHouse(kGuid);
									if( pkPilot )
									{
										PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(pkPilot->GetGuid()));
										if( pkHouse )
										{
											if( pkHouse->IsArrangeMode() == true )
											{
												XUIMgr.Close(L"Inv");
												break;
											}

											int iState =  pkPilot->GetAbil(AT_MYHOME_STATE);
											if( MAS_NOT_BIDDING == iState )
											{
												eEquipLimit equipLimit = static_cast<eEquipLimit>(pItemDef->GetAbil(AT_EQUIP_LIMIT));
												if( EQUIP_LIMIT_HAIR == equipLimit || EQUIP_LIMIT_FACE == equipLimit || EQUIP_LIMIT_SHIRTS == equipLimit )
												{
													PgPlayer* pkHousePlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
													if(pkHousePlayer)
													{
														PLAYER_ABIL kInfo;
														pkHousePlayer->GetPlayerAbil(kInfo);
														if( kItem.ItemNo() == kInfo.iHairStyle || kItem.ItemNo() == kInfo.iFace || kItem.ItemNo() == kInfo.iJacket )// 같은 아이템이면
														{
															lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405171, true);
															break;
														}
														if( EQUIP_LIMIT_HAIR == equipLimit )
														{
															CItemDef const* pDefStyle = kItemDefMgr.GetDef(kInfo.iHairStyle);
															if( pDefStyle )
															{
																if( pDefStyle->GetAbil(AT_MYHOME_ROOM_X) > pItemDef->GetAbil(AT_MYHOME_ROOM_X) || 
																	pDefStyle->GetAbil(AT_MYHOME_ROOM_Y) > pItemDef->GetAbil(AT_MYHOME_ROOM_Y) )
																{
																	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405172, true);
																	break;
																}
															}
														}
													}
													pkHouse->HomeItemUseStyleItem(kViewerItemPos);
												}
												else if(pItemDef->GetAbil(AT_FURNITURE_TYPE) == FURNITURE_TYPE_OBJECT)
												{
													pkHouse->AddFurniturebyInven(kItem.ItemNo(), kItem.Guid());
													lwCloseUI("Inv");
												}
											}
											else
											{
												lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
											}
										}
									}
								}
							}
							else
							{
								lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201809, true);
							}
							break;
						}

						if(	true == pItemDef->IsType(ITEM_TYPE_EQUIP) ) 
						{
							// 장비아이템은 금지
							break;
						}

						if(	true == pItemDef->IsType(ITEM_TYPE_USE_UI) ) 
						{	// 특정 UI를 통해서만 사용할수 있는 아이템.
							lwAddWarnDataTT(750048);
							break;
						}

						if( !IsCanUseItemGround(kItem.ItemNo()) )
						{
							break;
						}

						if( UseItemCustomType(kItem.ItemNo(), &kItem, kViewerItemPos) )
						{
							return EIAR_UICT_DONE;
						}

						int const AbilValue = pItemDef->GetAbil(AT_MAKING_TYPE);
						if( AbilValue & EMAKING_TYPE_COOKING )	//요리 아이템이면
						{
							if( AbilValue & EMAKING_TYPE_MAZE_ITEM )
							{
								if( g_kEqItemMixCreator.SetRecipe(kViewerItemPos) )
								{
									lwItemMix::CallEqItemMixCreatorUI();
								}
							}
							else if( (AbilValue != EMAKING_TYPE_COOKING) && (AbilValue != EMAKING_TYPE_NEW_COOKING) )
							{
								lwItemMix::AddNewCommonMixerInfo(kViewerItemPos);
							}
							else
							{
								if( kItem.IsUseTimeOut() )
								{
									::Notice_Show( TTW(1973), EL_Warning );
									break;
								}
								lwActivateUI("SFRM_ITEM_MAKING_COOK");
								g_kItemCookingMgr.SetSrcItem(kViewerItemPos);
							}
						}

						// 아이템 사용 레벨 제한 체크
						int const iCurLevel = pkPlayer->GetAbil(AT_LEVEL);
						int const iMinLv = pItemDef->GetAbil(AT_LEVELLIMIT);
						int const iMaxLv = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
						if( ((iMinLv > 0) && (iCurLevel < iMinLv))
							|| ((iMaxLv > 0) && (iCurLevel > iMaxLv)) )
						{
							lwAddWarnDataTT(22027);
							break;
						}

						//이펙트가 걸려야 하는 아이템 체크
						if(int const iNewEffect1 = pItemDef->GetAbil(AT_EFFECTNUM1))
						{
							int const iNewLevel = pItemDef->GetAbil(AT_LEVEL);
							if(CEffect* pkEffect = pkPlayer->GetEffect(iNewEffect1, true))
							{
								if (pkEffect->Level() > iNewLevel)
								{
									break;
								}
							}
						}

						//Send
						BM::Stream Packet(PT_C_M_REQ_ITEM_ACTION, kViewerItemPos);
						Packet.Push(lwGetServerElapsedTime32());

						int const UseConfirmTxtID = pItemDef->GetAbil(AT_ITEM_USE_CONFIRM_TXT);
						std::wstring const* pkConfirmTxt = NULL;
						if( 0 != UseConfirmTxtID && GetDefString(UseConfirmTxtID, pkConfirmTxt) )
						{
							BM::vstring kTempMsg(*pkConfirmTxt);
							AbilTextHelper::ReplaceText(pItemDef, kTempMsg);
							lwCallCommonMsgYesNoBox(MB(kTempMsg), lwPacket(&Packet), true, MBT_COMMON_YESNO_TO_PACKET);
						}
						else
						{
							NETWORK_SEND(Packet);
						};
					}
				}// if(pItemDef)
			}// if( IsEmpty )
		}break;
	case KUIG_QUICK_INVEN:
		{
			lwQuickInvDoAction(kIconInfo.iIconKey);
		}break;
	case KUIG_SKILLSET_BASIC:
	case KUIG_SKILLSET_SETICON:
		{
			if(true == lwSkillSet::lwCheckUseSkillSetNo(kIconInfo.iIconKey, true) )
			{
				lwSkillSet::lwSkillSetDoAction(kIconInfo.iIconKey);
			}
		}break;
	case KUIG_SKILLSET_SKILLICON:
		{
			SIconInfo kData;
			self->GetCustomData(&kData, sizeof(kData));

			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(kData.iIconKey);

			PgActor	*pkActor = NULL;
			if(KUIG_SKILLTREE==kData.iIconGroup || KUIG_SKILLSET_JUMPICON==kData.iIconGroup)
			{
				pkActor = g_kPilotMan.GetPlayerActor();
			}
			else if(KUIG_SKILLTREE_PET==kData.iIconGroup)
			{
				pkActor = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
			}

			if(pkActor)
			{
				pkActor->ReserveTransitAction(MB(kActionScriptName));
			}
		}break;
	case KUIG_SKILLSET_JUMPICON:
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(JUMPSKILLNO);

			PgActor	*pkActor = NULL;
			if(pkActor)
			{
				pkActor->ReserveTransitAction(MB(kActionScriptName));
			}
		}break;
	case KUIG_DEFENCE_ITEM:
		{
			int iItemNo = 0;
			self->GetCustomData(&iItemNo, sizeof(iItemNo));
			if( iItemNo>0 )
			{
				SItemPos kItemPos;
				int iOption = 0;
				BM::Stream kPacket(PT_C_M_REQ_DEFENCE_ITEM_USE);
				kPacket.Push(iItemNo);
				kPacket.Push(kItemPos);
				kPacket.Push(iOption);
				NETWORK_SEND(kPacket);
			}
		}break;
	case KUIG_DEFENCE_GUARDIAN:
		{
			g_kMissionMgr.CallInstallGuardian(self);
		}break;
	case KUIG_DEFENCE_SKILL:
		{
			g_kMissionMgr.CallDefenceSkillLearn(self);
		}break;
	case KUIG_EFFECT:
		{
			int iEffectID = GetCustomData<int>();
			if(iEffectID > 0)
			{
				GET_DEF(CEffectDefMgr, kEffectDefMgr);
				const CEffectDef *pkDef = kEffectDefMgr.GetDef(iEffectID);
				PgPlayer *pkMine = g_kPilotMan.GetPlayerUnit();
				if (pkDef && pkMine)
				{
					// 돈보유량 체크
					EffectControlType kResult = E_EFFCONTROL_NONE;
					int const iLevel = pkMine->GetAbil(AT_LEVEL);
					__int64 const iNeedMoney = (__int64)(pkDef->GetAbil(AT_EFFCONTROL_DEL_MONEY)+(pkDef->GetAbil(AT_EFFCONTROL_DEL_MONEY_PERLEVEL)*iLevel));
					if ( iNeedMoney > 0 )
					{
						__int64 const iMyMoney = pkMine->GetAbil64(AT_MONEY);
						if ( iNeedMoney > iMyMoney )
						{
							// 돈이 부족하다
							break;
						}
						kResult |= E_EFFCONTROL_DEL_MONEY;
					}

					// 아이템 체크(이걸쓸까??)
					int const iItemNo = pkDef->GetAbil(AT_EFFCONTROL_DEL_ITEM);
					if ( iItemNo > 0 )
					{
						// 아이템을 보유했는지 체크...
						{
							break;
						}
						kResult |= E_EFFCONTROL_DEL_ITEM;
					}

					// 나중에 위치를 옮겨야 한다
					if( kResult & E_EFFCONTROL_REQ_SERVER )
					{
						NETWORK_SEND(BM::Stream(PT_C_M_REQ_EFFECT_CONTROL,iEffectID))
					}
				}
			}
		}break;
	case KUIG_ITEM_TRADE_MY:
		{
			if ( g_kTradeMgr.IsConfirm() )
			{
				break;
			}

			PgBase_Item const* pkDelItem = g_kTradeMgr.GetTradeItemAt(kCasterPos.y, true);
			g_kTradeMgr.ModifyItem(kCasterPos, pkDelItem, true);

			pkIconWnd->SetInvalidate();
		}break;
	case KUIG_SKILLTREE:
		{
			if( g_kPilotMan.IsLockMyInput() )
			{
				break;
			}

			int iTemp = g_kSkillTree.GetKeySkillNo(kIconInfo.iIconKey);
			if (0>=iTemp)
			{
				break;
			}

			const PgSkillTree::stTreeNode* pkNode = g_kSkillTree.GetNode(iTemp);
			if (NULL == pkNode)
			{
				break;
			}

			const CSkillDef* pkDef = pkNode->m_pkSkillDef;
			if (NULL == pkDef)
			{
				break;
			}

			//	배우지 않았다면 사용할 수가 없당.
			if(pkNode && pkNode->m_bLearned == false)
			{
				//lwAddWarnDataStr(lwWString(TTW(300)), 2);
				g_kChatMgrClient.ShowNoticeUI(TTW( pkDef->GetAbil(AT_IS_COUPLE_SKILL) ? 450080 :  300 ), 2);
				break;
			}

			if (EST_PASSIVE==pkDef->GetType())
			{
				lwAddWarnDataTT(307);
				break;	//패시브 스킬은 사용 불가
			}

			PgPilot *pkPilot =g_kPilotMan.GetPlayerPilot();
			if (pkPilot)
			{
				PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if(pkActor)
				{
					TryReserveActionToMyActor(iTemp);
				}
			}
		}break;
	case KUIG_FIT:
	case KUIG_FIT_CASH:
		{
			if(false == IsItemMoveEnable())
			{
				break;
			}

			PgBase_Item kItem;
			if( S_OK == pkPlayer->GetInven()->GetItem(kCasterPos, kItem) )
			{
				SPT_C_M_REQ_FIT2PRIMARYINV	kData;
				kData.kItemPos = kCasterPos;
				BM::Stream	kPacket;
				kData.WriteToPacket(kPacket);
				
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pkDef = kItemDefMgr.GetDef(kItem.ItemNo());
				bool bCheckSafeAddition = CheckSafeAddition(kItem, kPacket);
				bool bCheckSafeAdditionItem = CheckSafeAdditionItem(kItem, kPacket);
				if( (!bCheckSafeAddition && !bCheckSafeAdditionItem) && pkDef && CanChangeEquipItem(pkDef))
				{
					NETWORK_SEND(kPacket)
				}
			}
		}break;
	case KUIG_CONSUME_PET:
	case KUIG_FIT_PET:
		{
			PgInventory *pkPetInven = GetMySelectedPetInven();
			if ( pkPetInven )
			{
				PgBase_Item kItem;
				SItemPos const kPetItemPos( EKindUIIconGroup(kCasterPos.x-190), kCasterPos.y );
				if ( S_OK == pkPetInven->GetItem( kPetItemPos, kItem ) )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pkDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if( pkDef )
					{
						bool bRet = false;
						EInvType kInvType = static_cast<EInvType>(pkDef->PrimaryInvType());

						SItemPos kPlayerInvenEmptyPos;
						if( true == pkPlayer->GetInven()->GetFirstEmptyPos( kInvType, kPlayerInvenEmptyPos) )
						{
							BM::Stream Packet(PT_C_M_REQ_ITEM_CHANGE_TO_PET);
							Packet.Push(kPlayerInvenEmptyPos);
							Packet.Push(kPetItemPos);
							Packet.Push(lwGetServerElapsedTime32());
							Packet.Push(true);
							NETWORK_SEND(Packet)
						}
					}
				}
			}
		}break;
	case KUIG_SKILLTREE_PET:
		{
			if(0==kIconInfo.iIconKey)	{break;}			
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
			if(!pkSkillDef)	{break;}
			if (EST_PASSIVE==pkSkillDef->GetType())
			{
				lwAddWarnDataTT(307);
				break;	//패시브 스킬은 사용 불가
			}
			if(EST_ACTIVE!=pkSkillDef->GetType() && EST_TOGGLE!=pkSkillDef->GetType())	{break;}
			std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(kIconInfo.iIconKey);
			if(2>kActionScriptName.size())			
			{
				NILOG(PGLOG_ERROR, "Skill ActionName is Err. No[%d]", kIconInfo.iIconKey); 
				assert(0);
				break;
			}	//이거 뭔가 잘못된것임
			PgActor	*pkActor = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
			if(!pkActor)	{break;}
			pkActor->ReserveTransitAction(MB(kActionScriptName));
		}break;
	case KUIG_TRANSTOWER_FREE:
		{
			if ( kIconInfo.iIconResNumber )
			{
				XUI::CXUI_Wnd *pkParentUI = self->Parent();
				if ( pkParentUI )
				{
					XUI::CXUI_Wnd *pkSelectUI = pkParentUI->GetControl( L"IMG_ICON_SELECT" );
					if ( pkSelectUI )
					{
						POINT3I pt3Loc = self->Location();
						--pt3Loc.x;
						--pt3Loc.y;
						pkSelectUI->Location(pt3Loc);
						pkSelectUI->Visible( true );
					}

					std::vector< char > kCustomData;
					self->GetCustomData( kCustomData );
					pkParentUI->SetCustomData( kCustomData );
				}
			}
		}break;
	case KUIG_REDICE_CRAFT_OPT:
		{
			lwCraftOptChanger::CallCraftOptChanger(SItemPos());
		}break;
	case KUIG_JOB_SKILL:
		{
			// 트리거 스크립트를 호출 이후 처리는 자동
			// 트리거는 PgActor 에서 얻을 수 있다
			//	트리거가 없으면 에러
			//	트리거가 있어도 채집 트리거 아니면 에러
			//	채집 트리거면, 트리거 스크립트 강제 실행
			UseJobSkill(kIconInfo.iIconKey);
		}break;
	case KUIG_JOB_SKILL3_ITEM:
		{
			int iSaveIndex = 0;
			self->GetCustomData( &iSaveIndex, sizeof(iSaveIndex) );
			if(iSaveIndex)
			{
				lwJobSkillItem::lwJS3_SelectMakeItem(iSaveIndex);
			}
		}break;
	case KUIG_ITEM_SKILL:
		{
			int const iItemSkillNo = kIconInfo.iIconKey;
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
			if(pkSkillDef)
			{
				if( ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo) )
				{
					UseItemSkill(iItemSkillNo);
				}
			}
		}break;
	default:
		{
		}break;
	}

	return EIAR_OK;
}

void lwQuickInvDoAction(int const iSlotNum)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	SQuickInvInfo kQuickInvInfo;
	PgQuickInventory* pkQInv = pkPlayer->GetQInven();
	if (!pkQInv)
	{
		return;
	}

	const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(iSlotNum), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
	if(S_OK == hRet)
	{
		switch(kQuickInvInfo.Grp())
		{
		case KUIG_SKILLTREE:
			{
				if( !g_kPilotMan.IsLockMyInput() )
				{
					PgSkillTree::stTreeNode const* pkNode = g_kSkillTree.GetNode( kQuickInvInfo.ID() );
					if (NULL == pkNode)
					{
						break;
					}
					CSkillDef const* pkDef = pkNode->m_pkSkillDef;
					if (NULL == pkDef)
					{
						break;
					}
					if(pkNode 
						&& false == pkNode->m_bLearned
						)
					{//	배우지 않았다면 사용할 수가 없다
						g_kChatMgrClient.ShowNoticeUI(TTW( pkDef->GetAbil(AT_IS_COUPLE_SKILL) ? 450080 :  300 ), 2);
						break;
					}
					if( EST_PASSIVE==pkDef->GetType() )
					{//패시브 스킬은 사용 불가
						lwAddWarnDataTT(307);
						break;
					}
					TryReserveActionToMyActor( kQuickInvInfo.ID() );
				}
			}break;
		case KUIG_SKILLTREE_PET:
			{
				if(pkPlayer->IsDead())	{break;}
				if( !g_kPilotMan.IsLockMyInput() )
				{
					PgActor	*pkActor = g_kPilotMan.FindActor(pkPlayer->SelectedPetID());
					if(pkActor)
					{
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(kQuickInvInfo.ID());
						pkActor->ReserveTransitAction(MB(kActionScriptName));
					}
				}
			}break;
		case KUIG_SKILLSET_BASIC:
		case KUIG_SKILLSET_SETICON:
			{
				if( !g_kPilotMan.IsLockMyInput() )
				{
					if(true == lwSkillSet::lwCheckUseSkillSetNo(kQuickInvInfo.ID(), true) )
					{
						lwSkillSet::lwSkillSetDoAction(kQuickInvInfo.ID());
					}
				}
			}break;
		case KUIG_JOB_SKILL:
			{			
				// 트리거 스크립트를 호출 이후 처리는 자동
				// 트리거는 PgActor 에서 얻을 수 있다
				//	트리거가 없으면 에러
				//	트리거가 있어도 채집 트리거 아니면 에러
				//	채집 트리거면, 트리거 스크립트 강제 실행	
				UseJobSkill(kQuickInvInfo.ID());
			}break;
		case KUIG_ITEM_SKILL:
			{
				int const iItemSkillNo = kQuickInvInfo.ID();
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
				if(pkSkillDef)
				{
					if( ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo) )
					{
						UseItemSkill(kQuickInvInfo.ID());
					}
				}
			}break;
		default:
			{
				if(!kQuickInvInfo.ID())
				{
					//					lwAddWarnDataStr(TTW(700019), 0);//아무것도 없단 말이지.
					g_kChatMgrClient.ShowNoticeUI(TTW(700019), 0);
					return;
				}

				if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
				{
					lwAddWarnDataTT(400732);
					break;
				}

				UIItemUtil::DoQuickUseItem(kQuickInvInfo.ID());
			}break;
		}
	}

}

void lwDoQuickUseItem(int const iItemNo)
{
	UIItemUtil::DoQuickUseItem(iItemNo);
}

void CallDisassembleItemUI(PgBase_Item const* pkItem, SIconInfo const& rkInfo)
{
	if (!pkItem)
	{
		return;
	}

	SEnchantInfo const& kEhtInfo = pkItem->EnchantInfo();
	if (0>=kEhtInfo.Rarity())
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 60082, true );
		return;
	}
	CXUI_Wnd *pkCalledWnd = XUIMgr.Call(UNI("SFRM_DISASSEMBLE_ITEM"), true);
	assert(pkCalledWnd);
	if(pkCalledWnd)
	{
		SItemPos kItemPos;
		kItemPos.x = rkInfo.iIconGroup;
		kItemPos.y = rkInfo.iIconKey;
		pkCalledWnd->SetCustomData(&kItemPos, sizeof(kItemPos));		//모달로 띄우자

		int iText = 60079;
		/*if (0>=pkItem->EnchantInfo().Rarity())
		{
		iText = 60080;
		}*/

		CXUI_Wnd* pkColor = pkCalledWnd->GetControl(L"SFRM_COLOR");
		if (pkColor)
		{
			CXUI_Wnd* pkSdw = pkColor->GetControl(L"SFR_SDW");
			if (pkSdw)
			{
				pkSdw->Text(TTW(iText));
			}	
		}
	}
}

void	AppendItemInfoToEditBox(PgBase_Item const &kItem, SItemPos const &kCasterItem)
{
	CXUI_Edit	*pkEdit = dynamic_cast<CXUI_Edit*>(XUIMgr.GetFocusedEdit());
	if(NULL == pkEdit)
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return;
	}

	std::wstring	kItemName;
	if(::MakeItemName(kItem.ItemNo(),kItem.EnchantInfo(),kItemName) == false)
	{
		return;
	}

	std::wstring	kCurrentString = pkEdit->EditText();
	kItemName = std::wstring(_T("[")) + kItemName + std::wstring(_T("]"));


	int	const	iLimitLength = pkEdit->LimitLength();
	if(kItemName.length()+kCurrentString.length() > iLimitLength)
	{
		return;
	}

	E_ITEM_GRADE const eItemLv = GetItemGrade(kItem);
	std::wstring	kTemp;

	bool const bIsCash = CheckIsCashItem(kItem);
	DWORD	dwColor = (eItemLv == IG_SEAL) ? 0xffedca88 : SetGradeColor(eItemLv,bIsCash,kTemp);

	pkEdit->EditText(kCurrentString + kItemName,true);
	pkEdit->AddTextBlock(kCurrentString.length(),kItemName,sizeof(kCasterItem),(BYTE*)(&kCasterItem),dwColor);

}

void lwUIWnd::IconToCursor()
{
	XUI::CXUI_Cursor *pkCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(!pkCursor)
	{
		return;
	}

	CXUI_Icon *pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(!pkIconWnd)
	{
		pkCursor->ResetCursorState();
		return;
	}

	const SIconInfo &kIconInfo = pkIconWnd->IconInfo();

	switch(kIconInfo.iIconGroup)
	{
	case	KUIG_PET:
	case	KUIG_INV_VIEW:
	case	KUIG_FIT:
	case	KUIG_SAFE_ADDITION:
		{
			if(g_pkLocalManager)
			{
				NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
				if(pkKeyboard && pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_LMENU))
				{
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if(pkPlayer)
					{
						unsigned	int	uiGroup = (kIconInfo.iIconGroup == KUIG_INV_VIEW) ?g_iInvViewGrp : kIconInfo.iIconGroup;
						SItemPos const kCasterPos(uiGroup, kIconInfo.iIconKey);
						PgBase_Item kCasterItem;
						if(S_OK == pkPlayer->GetInven()->GetItem(kCasterPos, kCasterItem))
						{
							AppendItemInfoToEditBox(kCasterItem, kCasterPos);
							pkCursor->IconInfo().Clear();
							return;
						}
					}
				}
			}
		}
		break;
	}

	switch(kIconInfo.iIconGroup)
	{
	case KUIG_PET:
	case KUIG_INV_VIEW:
	case KUIG_SAFE_ADDITION:
		{	
			int const iTempViewGrp = KUIG_INV_VIEW==kIconInfo.iIconGroup ? g_iInvViewGrp : kIconInfo.iIconGroup;
			bool bNotToCursor = false;
			switch(iTempViewGrp)
			{
			case KUIG_EQUIP:
			case KUIG_CONSUME:
			case KUIG_ETC:
			case KUIG_CASH:
			case KUIG_SAFE:
			case KUIG_SAFE_ADDITION:
			case KUIG_CASH_SAFE:
			case KUIG_RENTAL_SAFE1:
			case KUIG_RENTAL_SAFE2:
			case KUIG_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE1:
			case KUIG_SHARE_RENTAL_SAFE2:
			case KUIG_SHARE_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE4:
			case KUIG_PET:
				{
					if(lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true) && BM::GUID::IsNull(g_kTradeMgr.GetTradeGuid()) )
					{
						if(XUIMgr.Get(_T("FRM_AUCTION_MAIN")))
						{
							pkCursor->IconInfo().iIconGroup = iTempViewGrp;
							pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
							return;
						}

						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if(pkPlayer)
						{
							SItemPos const kCasterPos(iTempViewGrp, kIconInfo.iIconKey);
							PgBase_Item kCasterItem;
							if(S_OK == pkPlayer->GetInven()->GetItem(kCasterPos, kCasterItem))
							{
								GET_DEF(CItemDefMgr, kItemDefMgr);
								CItemDef const* pItemDef = kItemDefMgr.GetDef(kCasterItem.ItemNo());
								if (pItemDef && pItemDef->IsAmountItem())
								{
									SCalcInfo kInfo;
									kInfo.eCallType = CCT_ITEM_DIVIDE;
									kInfo.iBasePrice = 0;
									kInfo.kGuid = kCasterItem.Guid();
									kInfo.iItemNo = kCasterItem.ItemNo();
									kInfo.kItemPos = kCasterPos;
									CallCalculator(kInfo);

									bNotToCursor = true;
									pkCursor->IconInfo().Clear();
								}
							}
						}
					}
				}break;
			} 
			if (!bNotToCursor)
			{
				pkCursor->IconInfo().iIconGroup = iTempViewGrp;
				pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
			}
		}break;
	case KUIG_SKILLTREE:
		{
			PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(kIconInfo.iIconKey));
			if(!pFound || pFound->m_bLearned == false || (pFound->m_pkSkillDef && pFound->m_pkSkillDef->GetType() == EST_PASSIVE) || pFound->IsTemporaryLevelChanged() || pkIconWnd->GrayScale()) //	배우지 않은 스킬이나 Passive 스킬을 드래그 할 수 없다. 임시로 배웠을 때도 드래그 불가
			{
				pkCursor->IconInfo().Clear();
				return;
			}
			else
			{
				pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
				pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
			}
		}break;
	case KUIG_SKILLTREE_PET:
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
			if(!pkSkillDef || pkSkillDef->GetType() == EST_PASSIVE || pkIconWnd->GrayScale() || 0==kIconInfo.iIconKey || !lwGetHaveSkill(lwUnit(GetMySelectedPet()), kIconInfo.iIconKey))
			{
				pkCursor->IconInfo().Clear();
				return;
			}
			else
			{
				pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
				pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
				pkCursor->IconInfo().iIconResNumber= kIconInfo.iIconResNumber;
			}
		}break;
	case KUIG_SKILLSET_SKILLICON:
		{
			if(self->GetCustomDataSize() > 0)
			{
				self->ClearCustomData();
				lwSkillSet::lwChangeSaveState(true);
			}
			lwSkillSet::lwSetMinTime(lwUIWnd(self), 0);
		}break;
	case KUIG_SKILLSET_JUMPICON:
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(JUMPSKILLNO);
			if( !pkSkillDef ){ return; }

			pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
			pkCursor->IconInfo().iIconKey = JUMPSKILLNO;
			pkCursor->IconInfo().iIconResNumber = pkSkillDef->RscNameNo();
		}break;
	case KUIG_ENCHANT_SHIFT_SRC:
		{
			g_kEnchantShift.ClearSrcItem();
			g_kEnchantShift.Refresh();
		}break;
	case KUIG_ENCHANT_SHIFT_TGT:
		{
			g_kEnchantShift.ClearTargetItem();
			g_kEnchantShift.Refresh();
		}break;
	case KUIG_JOB_SKILL:
		{
			PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
			if(pPlayer)
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CSkillDef const* pSkillDef;
				pSkillDef = kSkillDefMgr.GetDef(kIconInfo.iIconKey);
				if(pSkillDef)
				{
					if( pSkillDef->GetType() != EST_PASSIVE)
					{
						bool const bRet = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, kIconInfo.iIconKey);
						if(bRet)
						{
							pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
							pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
							pkCursor->IconInfo().iIconResNumber= kIconInfo.iIconResNumber;
						}
					}
				}
			}
		}break;
	case KUIG_ITEM_SKILL:
		{
			int const iItemSkillNo = kIconInfo.iIconKey;
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
			if(pkSkillDef)
			{
				if( ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo) )
				{
					pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
					pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
					pkCursor->IconInfo().iIconResNumber= kIconInfo.iIconResNumber;
				}
			}
		}break;
	case KUIG_FIT:
		{
			if( kIconInfo.iIconKey == EQUIP_POS_MEDAL )
			{
				pkCursor->IconInfo().Clear();
				return;
			}
		}
	default:
		{
			pkCursor->IconInfo().iIconGroup = kIconInfo.iIconGroup;
			pkCursor->IconInfo().iIconKey = kIconInfo.iIconKey;
			pkCursor->IconInfo().iIconResNumber= kIconInfo.iIconResNumber;
		}break;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (!pkPlayer)
	{
		return;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return;
	}

	switch( pkCursor->CursorState() )
	{
	case XUI::CS_STATE_1://분해.
		{
			if( pkCursor->IconInfo().iIconGroup == KUIG_GUILD_INV_VIEW )
			{// 길드금고에서 사용할 수 없는 기능입니다.				
				::Notice_Show(TTW(403409), EL_Warning);
				pkCursor->ResetCursorState();
				return;
			}

			if (!IsItemMoveEnable())	//거래나 업그레이드 도중에 분해하지 말자
			{
				return;
			} 

			if (XUIMgr.Get(_T("SFRM_ITEM_MAKING_COOK")))
			{
				lwAddWarnDataTT(1612);
				return;
			}

			SItemPos kItemPos(pkCursor->IconInfo().iIconGroup, pkCursor->IconInfo().iIconKey);
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kItemPos, kItem) )
			{// 인벤에 아이템이 없다면 종료
				return;
			}

			SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
			if(kEhtInfo.IsBinding())
			{// 자물쇠가 사용되었다면 분해 할수 없다  메세지 후 종료
				::Notice_Show(TTW(790315), EL_Warning);
				pkCursor->ResetCursorState();
				return;
			}

			switch(pkCursor->IconInfo().iIconGroup)
			{
			case KUIG_EQUIP://장비템만 분해.
			case KUIG_CONSUME:
			case KUIG_ETC:
				{
					if(kItem.ItemNo())
					{
						CallDisassembleItemUI(&kItem, pkCursor->IconInfo());
					}
					else
					{
						pkCursor->ResetCursorState();
					}
					pkCursor->IconInfo().Clear();//끝날때 집은거 지움.
				}break;
			default:
				{
					if(kItem.ItemNo())
					{
						pkCursor->ResetCursorState();
						lwAddWarnDataTT(1450);//장비만 분해 할 수 있습니다.
					}
					pkCursor->IconInfo().Clear();//끝날때 집은거 지움.
				}break;
			}
		}break;
	case XUI::CS_STATE_2:
		{//기간 연장

			if( pkCursor->IconInfo().iIconGroup == KUIG_GUILD_INV_VIEW )
			{// 길드금고에서 사용할 수 없는 기능입니다.				
				::Notice_Show(TTW(403409), EL_Warning);
				pkCursor->ResetCursorState();
				return;
			}

			lua_tinker::call< void, int, int >("OnCallStaticCashItemRenew", pkCursor->IconInfo().iIconGroup, pkCursor->IconInfo().iIconKey);
			CXUI_Cursor::m_IconInfo.Clear();
			pkCursor->ResetCursorState();
			pkCursor->IconInfo().Clear();//끝날때 집은거 지움.
		}break;
	case XUI::CS_STATE_3:
		{//원소 추출
			if( pkCursor->IconInfo().iIconGroup == KUIG_GUILD_INV_VIEW )
			{// 길드금고에서 사용할 수 없는 기능입니다.				
				::Notice_Show(TTW(403409), EL_Warning);
				pkCursor->ResetCursorState();
				return;
			}
			SItemPos kItemPos(pkCursor->IconInfo().iIconGroup, pkCursor->IconInfo().iIconKey);
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kItemPos, kItem) )
			{// 인벤에 아이템이 없다면 종료
				return;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			const CItemDef* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if(!pItemDef)
			{//Def에 없으면 종료
				return;
			}

			if( !pItemDef->GetAbil(AT_EXTRACT_ELEMENT_FIRE)
				&&!pItemDef->GetAbil(AT_EXTRACT_ELEMENT_ICE)
				&&!pItemDef->GetAbil(AT_EXTRACT_ELEMENT_NATURE)
				&&!pItemDef->GetAbil(AT_EXTRACT_ELEMENT_CURSE)
				&&!pItemDef->GetAbil(AT_EXTRACT_ELEMENT_DESTROY) 
				&&!pItemDef->GetAbil(AT_EXTRACT_ELEMENT_RANDOM) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 792009, true);
				pkCursor->ResetCursorState();
			}
			else
			{
				lwCallYesNoMsgBox(TTW(792008), BM::GUID::NullData(), MBT_EXTRACT_ELEMENT, kItemPos.y );
			}
			pkCursor->IconInfo().Clear();//끝날때 집은거 지움.
		}break;
	} 
}

void lwUIWnd::CursorToIconFast(int bIsDblClicked)
{//사실 커서는 상관 없지.
	if(XUIMgr.Get(_T("FRM_AUCTION_MAIN")))
	{
		return;
	}

	//더블클릭에는 커서 상관없고.
	//대충놓기에는 커서 상관 있고..
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (!pkPlayer) { return; }

	SIconInfo kWndIconInfo;
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(bIsDblClicked)
	{//더블클릭 이벤트에 의한 빠른 착용.
		if(pCursor && (XUI::CS_NONE != pCursor->CursorState()))//분해중이면 
		{
			return;// 입지 말자
		}
		XUI::CXUI_Icon *pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>(self);
		kWndIconInfo = pkIconWnd->IconInfo();

		switch(kWndIconInfo.iIconGroup)
		{
		case KUIG_EQUIP:
			{
				if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
				{
					lwAddWarnDataTT(400732); 
					break;
				}
				if (XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE")))
				{
					lwAddWarnDataTT(1208);
					break;
				}
				if(XUIMgr.Get(_T("FRM_AUCTION_MAIN")))
				{
					lwAddWarnDataTT(403082);
					break;
				}
			}
		case KUIG_CONSUME:
		case KUIG_ETC:
		case KUIG_CASH:
		case KUIG_SAFE:
		case KUIG_SAFE_ADDITION:
		case KUIG_CASH_SAFE:
		case KUIG_FIT:
		case KUIG_FIT_CASH:
		case KUIG_RENTAL_SAFE1:
		case KUIG_RENTAL_SAFE2:
		case KUIG_RENTAL_SAFE3:
		case KUIG_SHARE_RENTAL_SAFE1:
		case KUIG_SHARE_RENTAL_SAFE2:
		case KUIG_SHARE_RENTAL_SAFE3:
		case KUIG_SHARE_RENTAL_SAFE4:
		case KUIG_GUILD_INVENTORY1:
		case KUIG_GUILD_INVENTORY2:
		case KUIG_GUILD_INVENTORY3:
		case KUIG_GUILD_INVENTORY4:
			{//하는일 없음.
			}break;
		case KUIG_PET:
		case KUIG_INV_VIEW:
			{
				int const iInvViewGrp = KUIG_PET==kWndIconInfo.iIconGroup ? KUIG_PET : g_iInvViewGrp;
				SItemPos kCasterItemPos(iInvViewGrp, kWndIconInfo.iIconKey);
				PgInventory *pkInv = pkPlayer->GetInven();
				if (pkInv)
				{
					PgBase_Item kItem;

					if(S_OK == pkInv->GetItem(kCasterItemPos, kItem))
					{
						if( UseItemCustomType(kItem.ItemNo(), &kItem, kCasterItemPos, bIsDblClicked) )
						{
							break;
						}

						if (!IsItemMoveEnable())
						{
							break;
						}
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
						if(pkItemDef)
						{
							if( pkItemDef->IsType(ITEM_TYPE_EQUIP) )
							{
								if( true == CanChangeEquipItem(pkItemDef))
								{
									int const iPosLimit = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
									int iResultPos = 0;
									if( EQUIP_LIMIT_RING == iPosLimit )
									{
										PgBase_Item kItem;
										EInvType equip_target = IT_FIT;
										if( iInvViewGrp != KUIG_PET )
										{
											if( iInvViewGrp != KUIG_CASH )
											{
												equip_target = IT_FIT;
											}
											else
											{
												equip_target = IT_FIT_CASH;
											}
										}

										if( S_OK != pkInv->GetItem(equip_target, EQUIP_POS_RING_L, kItem) )
										{
											iResultPos = EQUIP_POS_RING_L;
										}

										if( 0 == iResultPos && S_OK != pkInv->GetItem(equip_target, EQUIP_POS_RING_R, kItem) )
										{
											iResultPos = EQUIP_POS_RING_R;
										}
									}

									if( 0 == iResultPos )
									{
										iResultPos = CItemDef::EquipLimitToPos(iPosLimit);
									}

									if( iResultPos )
									{
										SItemPos kTargetPos( kCasterItemPos.x == IT_CASH ? IT_FIT_CASH : IT_FIT , iResultPos );
										if ( EQUIP_POS_PET == iResultPos )
										{
											kTargetPos.x = IT_FIT;// Pet
										}


										if( pkItemDef->GetAbil(AT_REVERSION_ITEM) && !kItem.EnchantInfo().IsAttached() )
										{
											Send_PT_C_M_REQ_ITEM_CHANGE_YES_NO(5018, kCasterItemPos, kTargetPos);
											break;
										}
										else if(IT_PET==pkItemDef->GetAbil(AT_PRIMARY_INV))
										{
											Send_PT_C_M_REQ_ITEM_CHANGE_TO_PET(kCasterItemPos, kTargetPos);
											break;
										}
										Send_PT_C_M_REQ_ITEM_CHANGE(kCasterItemPos, kTargetPos);
									}
								}
							}
						}
					}	
				}
			}break;
		case KUIG_STORE:
		case KUIG_STOCK_STORE:
			{//구입.
				DoItemBuy(kWndIconInfo.iIconKey);
			}break;
		}
	}
	else
	{//대충 놔도 장착되기 기능.
		if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
		{
			lwAddWarnDataTT(400732);
			return;
		}

		if(pCursor)
		{
			kWndIconInfo = pCursor->IconInfo();

			PgInventory *pkInv = pkPlayer->GetInven();
			SItemPos kCasterPos(pCursor->IconInfo().iIconGroup, pCursor->IconInfo().iIconKey);

			PgBase_Item kItem;

			if(S_OK == pkInv->GetItem(kCasterPos, kItem))
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if(pkItemDef)
				{
					XUI::CXUI_Wnd *pkWnd = dynamic_cast<XUI::CXUI_Wnd*>(self);
					if (!pkWnd)
					{
						return;
					}
					XUI::CXUI_Wnd *pkMainWnd = pkWnd->Parent();

					while(pkMainWnd && pkMainWnd->Parent())
					{
						if(pkMainWnd->Parent())
						{
							pkMainWnd = pkMainWnd->Parent();
						}
						else
						{
							break;
						}
					}

					if(pkMainWnd->ID() == _T("FRM_SHOP"))
					{
						if(kCasterPos.x != KUIG_FIT
							&& kCasterPos.x != KUIG_FIT_CASH )
						{
							DoItemSell(kCasterPos, &kItem);
						}
						else
						{
							lwAddWarnDataTT(9016);
						}
					}
					else
					{
						if(pkItemDef->CanEquip())
						{
							if(false == CanChangeEquipItem(pkItemDef))
							{
								return;
							}

							int const iPosLimit = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
							int const iResultPos = CItemDef::EquipLimitToPos(iPosLimit);
							bool bSamePos = (IT_FIT==kWndIconInfo.iIconGroup) && (kWndIconInfo.iIconKey==iResultPos);
							if(iResultPos && !bSamePos)//장착 위치가 같으면 아무일도 하지 않는다.(이전 위치랑 지금 위치랑 같은 위치이다.)
							{
								if (XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE")))
								{
									lwAddWarnDataTT(1208);
									return;
								}
								SItemPos kTargetPos(IT_FIT, iResultPos);
								Send_PT_C_M_REQ_ITEM_CHANGE( kCasterPos, kTargetPos);
							}
						}
					}
				}
			}
		}
	}
}


bool CheckEnchantBundle(PgBase_Item const& kItem)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef )
	{		
		return false;
	}
	SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
	if( !pDef->CanEquip() || kEhtInfo.IsBinding() || kEhtInfo.IsCurse() || kEhtInfo.IsSeal() ||
		(ICMET_Cant_Enchant & pDef->GetAbil(AT_ATTRIBUTE)) )
	{// 장비 할수 없거나, 자물쇠가 사용되거나 저주거나 봉인 아이템이면 안됨		
		return false;
	}

	if( (ICMET_Cant_Enchant & pDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_Enchant )
	{//	인챈트 불가 속성이면 당근 안됨		
		return false;
	}

	eEquipLimit	kEquipLimit = static_cast<eEquipLimit>(0x00000001 << pDef->EquipPos());
	if( (EQUIP_INCHANT_POSSIBLE_BUNDLE & kEquipLimit) != kEquipLimit )
	{//	인챈트 가능한 부위는 이걸로 체크
		return false;
	}

	if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
		&& CheckIsCashItem(kItem))
	{//일본의 경우 캐시 아이템은 인챈트 관련 작업 불가
		return false;
	}

	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if(pItemDef)
		{// 다음 인챈트 정보가 있다면 인챈트 가능
			int const iEquipPos = pItemDef->GetAbil(AT_EQUIP_LIMIT);
			SItemPlusUpgradeKey kKey(  pItemDef->IsPetItem(), iEquipPos, kItem.EnchantInfo().PlusLv()+1i64 );//장착위치를 넣어야함.

			CONT_DEF_ITEM_PLUS_UPGRADE const *pCont = NULL;
			g_kTblDataMgr.GetContDef(pCont);
			if(pCont)
			{
				CONT_DEF_ITEM_PLUS_UPGRADE::const_iterator itor = pCont->find(kKey);
				if(itor != pCont->end())
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool lwUIWnd::AutoMoveItemToInven(PgPlayer* pkPlayer, SItemPos const& rkItemPos, EInvType const& eInvType)
{
	if(NULL==pkPlayer)	{return true;}

	CXUI_Wnd* pWnd = XUIMgr.Get( _T( "Inv" ) );
	if( !pWnd )
	{//인벤토리가 열려있지 않다면 열어준다.
		lwCallUI("Inv");
		pWnd = XUIMgr.Get( _T( "Inv" ) );
		if( !pWnd )
		{
			return true;
		}
	}

	PgInventory* pInv = pkPlayer->GetInven();
	if( !pInv )
	{
		return true;
	}

	SItemPos EmptyPos;
	if( !pInv->GetFirstEmptyPos( eInvType, EmptyPos ) )
	{// 인벤토리가 가득참
		lwAddWarnDataTT(3074);
		return true;
	}
	else
	{
		Send_PT_C_M_REQ_ITEM_CHANGE(rkItemPos, EmptyPos);
		return true;
	}

	return false;
}

bool lwUIWnd::CursorToIcon( bool bIsRBtnClicked )
{
	CXUI_Icon *pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(!pkIconWnd)
	{
		return false;
	}

	const SIconInfo &kTargetIconInfo = pkIconWnd->IconInfo();	

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(!pCursor)
	{
		return false;
	}

	const SIconInfo &kCursorIcon = pCursor->IconInfo();//커서에 묶인것.
	if( 0 == kCursorIcon.iIconGroup )
	{
		return true;
	}

	if(KUIG_QUICK_INVEN == kCursorIcon.iIconGroup)
	{
		SQuickInvInfo kQuickInvInfo;
		PgQuickInventory* pkQInv = pkPlayer->GetQInven();
		const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kCursorIcon.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다

		if(S_OK == hRet)
		{
			//			kCursorIcon.x = kQuickInvInfo.Grp();
			//			kCursorIcon.y = kQuickInvInfo.ID();
		}
		else
		{
			return false;
		}
	}	

	/* 마우스 우클릭 */
	if( bIsRBtnClicked )
	{
		// 이동 할 아이템
		SItemPos CasterPos;
		PgBase_Item kItem;

		HRESULT kRet = E_FAIL;
		if( KUIG_GUILD_INV_VIEW == kCursorIcon.iIconGroup )
		{
			CasterPos.x = g_kGuildMgr.GetGuildInvView();
			CasterPos.y = kCursorIcon.iIconKey;
			ConvertItemPosToGuildInvType(CasterPos);
			kRet = g_kGuildMgr.GetInven()->GetItem( CasterPos, kItem );
		}
		else
		{
			CasterPos.x = kCursorIcon.iIconGroup;
			CasterPos.y = kCursorIcon.iIconKey;
			kRet = pkPlayer->GetInven()->GetItem( CasterPos, kItem );
		}

		if( S_OK != kRet )
		{
			return false;
		}

		EInvType eInvType;// 이동 할 위치
		GET_DEF( CItemDefMgr, kItemDefMgr );
		CItemDef const* pItemDef = kItemDefMgr.GetDef( kItem.ItemNo() );
		if( pItemDef )
		{
			eInvType = static_cast<EInvType>( pItemDef->PrimaryInvType() );
		}

		XUI::CXUI_CheckButton* pkCheckBtn = NULL;// 아이템 이동시 해당 탭을 보여주기 위함

		switch( CasterPos.x )
		{// 이동 할 아이템의 현재 위치가
		case IT_NONE:
			{
				return false;
			}break;

			// 현재위치: 창고
		case IT_SAFE:
		case IT_RENTAL_SAFE1:
		case IT_RENTAL_SAFE2:
		case IT_RENTAL_SAFE3:
		case IT_SHARE_RENTAL_SAFE1:
		case IT_SHARE_RENTAL_SAFE2:
		case IT_SHARE_RENTAL_SAFE3:
		case IT_SHARE_RENTAL_SAFE4:
			{// 인벤토리(장비, 소비, 기타, 캐시)로 이동
				if(AutoMoveItemToInven(pkPlayer, CasterPos, eInvType))
				{
					return true;
				}
			}break;
			// 현재위치: 인벤토리(또는 길드인벤)
		case IT_EQUIP:
		case IT_CONSUME:
		case IT_ETC:
		case IT_CASH:
		case IT_SAFE_ADDITION:
			{
				PgInventory* pkInv = pkPlayer->GetInven();
				if( !pkInv )
				{
					return false;
				}

				{//길드인벤 관련 이동
					if( KUIG_GUILD_INV_VIEW == kCursorIcon.iIconGroup ) // 길드인벤 --> 인벤
					{
						CXUI_Wnd *pWnd = XUIMgr.Get( _T( "Inv" ) );
						if( pWnd )
						{
							if(!IsItemMoveEnable())
							{// 아이템을 이동 할 수 없는 경우
								break;
							}

							SItemPos EmptyPos;
							if( !pkInv->GetFirstEmptyPos( eInvType, EmptyPos ) )
							{// 인벤토리가 가득참
								lwAddWarnDataTT(3074);
								return false;
							}
							else
							{	
								Send_PT_C_M_REQ_ITEM_CHANGE_GUILD(CasterPos, EmptyPos, true, false);
								return true;
							}
						}
					}
					else // 인벤 --> 길드인벤
					{
						PgInventory* pkGuildInv = g_kGuildMgr.GetInven();
						if( !pkGuildInv )
						{
							return false;
						}

						CXUI_Wnd *pWnd = XUIMgr.Get( _T( "SFRM_GUILD_INVENTORY" ) );
						if( pWnd )
						{
							if(!IsItemMoveEnable())
							{// 아이템을 이동 할 수 없는 경우
								break;
							}

							SItemPos EmptyPos;
							bool bEmpty = false;

							for( int i = 0; i < 4; ++i ) // 유효한 길드인벤 만큼 루프돌게 바꾸자.
							{
								if( pkGuildInv->GetFirstEmptyPos( static_cast<EInvType>(i + 1), EmptyPos ) )
								{
									bEmpty = true;
									break;
								}
							}

							if( bEmpty )
							{
								Send_PT_C_M_REQ_ITEM_CHANGE_GUILD(CasterPos, EmptyPos, false, true);
								return true;
							}
							else
							{
								lwAddWarnDataTT(3074);
								return false;
							}
						}
					}
				}

				// 창고로 아이템 이동
				CXUI_Wnd *pWnd = XUIMgr.Get( _T( "SFRM_INV_SAFE" ) );
				if( pWnd )
				{
					if(!IsItemMoveEnable())
					{// 아이템을 이동 할 수 없는 경우
						break;
					}

					SItemPos EmptyPos;
					bool bEmpty = false;// 창고에 빈공간이 있는지

					if( !pkInv->GetFirstEmptyPos( static_cast<EInvType>(KUIG_SAFE), EmptyPos ) )
					{// 창고가 가득 찼으면 캐시(확장) 창고로 이동
						for( size_t i = 0; i < 3; ++i )
						{
							int iRealInvType = IT_RENTAL_SAFE1 + i;
							if( S_OK == pkInv->CheckEnableUseRentalSafe( static_cast<EInvType>( iRealInvType ) ) )
							{
								if( pkInv->GetFirstEmptyPos( static_cast<EInvType>(iRealInvType), EmptyPos ) )
								{
									bEmpty = true;
									break;
								}
							}
						}
					}
					else
					{
						bEmpty = true;
					}

					if( bEmpty )
					{
						Send_PT_C_M_REQ_ITEM_CHANGE(CasterPos, EmptyPos);
						return true;
					}				
					else
					{
						lwAddWarnDataTT(20022);
						return false;
					}
				}

				// 계정공유 창고로 이동
				pWnd = XUIMgr.Get( _T( "SFRM_SHARE_SAFE" ) );
				if( pWnd )
				{
					if(!IsItemMoveEnable())
					{// 아이템을 이동 할 수 없는 경우
						break;
					}

					SItemPos EmptyPos;

					for( size_t i = 0; i < 4; ++i )
					{
						int iRealInvType = IT_SHARE_RENTAL_SAFE1 + i;
						if( IsShareRentalPremium(iRealInvType, pkPlayer)
						 || S_OK == pkInv->CheckEnableUseRentalSafe( static_cast<EInvType>( iRealInvType ) ) )
						{
							if( pkInv->GetFirstEmptyPos( static_cast<EInvType>(iRealInvType), EmptyPos ) )
							{	
								Send_PT_C_M_REQ_ITEM_CHANGE(CasterPos, EmptyPos);
								return true;
							}
						}
					}
					lwAddWarnDataTT(20022);
					return false;
				}

				// 거래
				pWnd = XUIMgr.Get(_T("SFRM_TRADE"));
				if( pWnd )
				{
					if( KUIG_FIT == kCursorIcon.iIconGroup )
					{
						lwAddWarnDataTT(400723);
						return false;
					}
					if( g_kTradeMgr.IsConfirm() )
					{
						return false;
					}
					if( PgTradeMgr::CheckItemToGiveOther( CasterPos, pkPlayer, kItem ) )
					{
						g_kTradeMgr.ModifyItem( CasterPos, &kItem );
						return true;
					}
					else
					{// 팔 수 없는 아이템
						if(0!=kCursorIcon.iIconGroup && KUIG_ITEM_TRADE_MY!=kCursorIcon.iIconGroup)
						{
							lwAddWarnDataTT(400719);
							return false;
						}
					}
				}
				// 우편함
				pWnd = XUIMgr.Get(_T("SFRM_POST"));
				if( pWnd )
				{
					XUI::CXUI_Wnd* pkPost = pWnd->GetControl(L"FRM_POST1");
					if( pkPost )
					{
						if( pkPost->Visible() )
						{
							XUI::CXUI_Wnd* pkItem = pkPost->GetControl(L"ICN_ITEM");
							if( pkItem )
							{
								if(KUIG_FIT == kCursorIcon.iIconGroup)
								{
									lwAddWarnDataTT(954);
									return false;
								}
								const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
								if(g_kMailMgr.SetSrcItem(kItemPos))
								{								
									return true;
								}
								else
								{
									return false;
								}
							}
						}						
					}
				}
				// 소켓
				pWnd = XUIMgr.Get(_T("SFRM_SOCKET_SYSTEM"));
				if( pWnd )
				{					
					XUI::CXUI_Wnd* pkSrc = pWnd->GetControl(L"ICN_SRC");
					if(pkSrc)
					{
						/*g_kSocketSystemMgr.UpdateSoulItemView();						
						lua_tinker::call<void, lwUIWnd, int>("SocketSystemUIUpdate", lwUIWnd(pkSrc), 0);
						return true;*/

						SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
						PgBase_Item kItem;
						if(S_OK != pkInv->GetItem(kItemPos, kItem))
						{
							return false;
						}

						GET_DEF( CItemDefMgr, kItemDefMgr );
						CItemDef const* pItemDef = kItemDefMgr.GetDef( kItem.ItemNo() );
						if( !pItemDef )
						{
							return false;
						}

						if( (ICMET_Cant_GenSocket == (pItemDef->GetAbil(AT_ATTRIBUTE) & ICMET_Cant_GenSocket)) ||
							(false == pItemDef->CanEquip()) /*|| (false == kEnchant.EanbleGenSocket(kItemGrade))*/)
						{
							g_kSocketSystemMgr.Clear();
							lwAddWarnDataTT(790114);
							return false;
						}

						g_kSocketSystemMgr.SetItem( KUIG_SOCKET_SYSTEM, kCursorIcon );
						//g_kSocketSystemMgr.SetItem((EKindUIIconGroup)kCursorIcon.iIconGroup, kCursorIcon);

						if( true == g_kSocketSystemMgr.IsSocketItemEmpty() )
						{
							g_kSocketSystemMgr.Clear();
						}
						lua_tinker::call<void, lwUIWnd>("CheckMenuSocketItem", lwUIWnd(pkSrc));
						lua_tinker::call<void, lwUIWnd, int>("SocketSystemUIUpdate", lwUIWnd(pkSrc), 0);
						g_kSocketSystemMgr.SetSocketItemMenuUIUpdate();
						return true;
					}
				}

				// 인챈트
				pWnd = XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE"));
				if( pWnd )
				{
					const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					PgBase_Item kItem;
					if(S_OK != pkInv->GetItem(kItemPos, kItem))
					{
						return false;
					}

					if( !CheckEnchantBundle(kItem) )
					{
						g_kItemPlusUpgradeMgr.Clear();
						lwAddWarnDataTT(1248);
						return false;
					}
					g_kItemPlusUpgradeMgr.SetItem(KUIG_ITEM_PLUS_UPGRADE_SRC, kCursorIcon);
					return true;
				}

				// 인챈트 변경권
				pWnd = XUIMgr.Get(_T("SFRM_ENCHANT_USE_ITEM"));
				if( pWnd )
				{
					CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_TARGET"));
					if( !pIcon )
					{
						return false;
					}

					lwOnMouseUpEnchantItemUseIcon(pIcon);
					return true;
				}

				// 인챈트전이 이동
				pWnd = XUIMgr.Get( _T( "SFRM_ENCHANT_SHIFT" ) );
				if( pWnd )
				{
					SItemPos const kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					if( PgEnchantShift::ES_STEP_1==g_kEnchantShift.GetState() )
					{
						if(g_kEnchantShift.SetSrcItem(kCasterPos) )
						{
							XUI::CXUI_Wnd* pkTmp = NULL;
							pkTmp = pWnd->GetControl(L"ICN_SOURCE");
							pkTmp ? pkTmp->SetCustomData(&kCasterPos, sizeof(kCasterPos)) : 0;
						}
					}
					else
					{
						if(g_kEnchantShift.SetTargetItem(kCasterPos) )
						{
							XUI::CXUI_Wnd* pkTmp = NULL;
							pkTmp = pWnd->GetControl(L"ICN_TARGET");
							pkTmp ? pkTmp->SetCustomData(&kCasterPos, sizeof(kCasterPos)) : 0;
						}
					}
				}

				// 소울크래프트
				pWnd = XUIMgr.Get(_T("SFRM_ITEM_RARITY_UPGRADE"));
				if( pWnd )
				{
					g_kItemRarityUpgradeMgr.SetItem(KUIG_ITEM_RARITY_UPGRADE_SRC, kCursorIcon);
					return true;
				}

				// 소울크래프트 변경권
				pWnd = XUIMgr.Get(_T("SFRM_SOUL_OPTION_CHANGER"));
				if( pWnd )
				{
					lwCraftOptChanger::CallCraftOptChanger(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey));
					return true;
				}

				// 펫 옵션 변경권
				pWnd = XUIMgr.Get(_T("SFRM_REDICE_ITEM"));
				if( pWnd )
				{
					lwPetOptChanger::SetSrcItem(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey));
					return true;
				}

				// 아이템 수리
				pWnd = XUIMgr.Get(_T("SFRM_REPAIR"));
				if( pWnd )
				{				
					const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					return g_kItemRepair.SetSrcItem(kItemPos);
				}

				// 자물쇠
				pWnd = XUIMgr.Get(_T("SFRM_ITEM_BINDING"));
				if( pWnd )
				{
					SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					PgBase_Item kItem;
					if(S_OK != pkInv->GetItem(kTargetItemPos, kItem))
					{
						return false;
					}
					PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
					if(pkPlayerActor)
					{
						pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Binding_Drop", 0.0f);			
					}

					g_kItemBindMgr.SetTargetItemFromInv(kTargetItemPos);
					return true;
				}

				// 펫사료
				pWnd = XUIMgr.Get(_T("SFRM_USE_PET_FEED"));
				if( pWnd )
				{
					SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					return g_kPetUIMgr.SetSrcItem(kTargetItemPos, pkInv, pWnd);
				}

				// 오픈마켓 아이템 등록
				pWnd = XUIMgr.Get(_T("SFRM_ARTICLE_REG_WIN"));
				if( pWnd )
				{
					XUI::CXUI_Wnd* pkSrc = pWnd->GetControl(L"ICON_ITEM");
					if( !pkSrc )
					{// 캐시판매 국가들은 FRM_REG_TYPE0 하위에 ICON_ITEM이 존재
						XUI::CXUI_Wnd* pkType = pWnd->GetControl(L"FRM_REG_TYPE0");
						if( pkType )
						{
							pkSrc = pkType->GetControl(L"ICON_ITEM");
						}
					}

					if( pkSrc )
					{					
						const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
						g_kMarketMgr.TempRegItemInfo(pkSrc, kItemPos);

						return true;
					}				
				}

				// 옵션 강화
				pWnd = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
				if( pWnd )
				{
					g_kItemRarityBuildUpMgr.SetItem( KUIG_ITEM_RARITY_BUILDUP_SRC, kCursorIcon );
					return true;
				}

				// 옵션 증폭
				pWnd = XUIMgr.Get(_T("SFRM_ITEM_RARITY_AMPLIFY"));
				if( pWnd )
				{
					g_kItemRarityAmplify.SetItem( KUIG_ITEM_RARITY_AMPLIFY_SRC, kCursorIcon );
					return true;
				}

				//몬스터카드 등록
				pWnd = XUIMgr.Get(L"SFRM_MONSTERCARD_MIXER");
				if( pWnd )
				{
					lwItemMix::SetMonsterCardMixerType2MixItem(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey));
					return true;
				}

				//스페셜 언락 등록
				pWnd = XUIMgr.Get(L"SFRM_SPECIAL_UNLOCK");
				if( pWnd )
				{
					lwUseUnlockItem::SetTargetItem( SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey) );
					return true;
				}

				//크래프트 옵션 삭제
				pWnd = XUIMgr.Get(L"SFRM_LUCKY_CHANGER");
				if( pWnd )
				{
					SItemPos const kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					if( lwLuckyChanger::SetItem(kCasterPos) )
					{
						CXUI_Wnd * pTmp = pWnd->GetControl(L"ICN_ICON");
						pTmp ? pTmp->SetCustomData(&kCasterPos, sizeof(kCasterPos)) : 0;
						lwLuckyChanger::OnDragUpItem(pTmp);
					}
					return true;
				}

				//아이템 교환
				pWnd = XUIMgr.Get(L"FRM_ITEM_CONVERT");
				if( pWnd )
				{
					if(g_kItemConvertSystemMgr.SetItem(kCursorIcon))
					{
						lwUIItemConvertSystem kConvert(pWnd->GetControl(L"ICON_Item1"));
						kConvert.OnItemConvertInit(1);
					}
					return true;
				}

				//직업생산 영력 전이
				pWnd = XUIMgr.Get(_T("SFRM_SOULABIL_TRANSFER"));
				if( pWnd )
				{
					SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					PgBase_Item kItem;
					if(S_OK != pkInv->GetItem(kItemPos, kItem))
					{
						return false;
					}
					lwSoulTransfer::AutoInsertItem(kItemPos, kItem);
					return true;
				}
				if(IT_SAFE_ADDITION==CasterPos.x)	//모든 동작도 하지 않았다면
				{
					if(AutoMoveItemToInven(pkPlayer, CasterPos, eInvType))
					{
						return true;
					}
				}

				pWnd = XUIMgr.Get(L"SFRM_COLLECT_ANTIQUE");
				if( pWnd )
				{ // 골동품 수집 중
					XUI::CXUI_Wnd * const MaterialWnd = dynamic_cast<XUI::CXUI_Wnd*>(pWnd->GetControl(L"ICN_MATERIAL"));
					XUI::CXUI_Wnd * const TargetWnd = dynamic_cast<XUI::CXUI_Wnd*>(pWnd->GetControl(L"ICN_TARGET"));
					if( MaterialWnd && TargetWnd )
					{
						SItemPos const ItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
						PgBase_Item Item;
						if(S_OK != pkInv->GetItem(ItemPos, Item))
						{
							return false;
						}
						if( lwDefenceMode::IsCanChangeAntique(EGT_ANTIQUE, Item.ItemNo()) )
						{ // 골동품 수집 가능한 아이템 이라면 재료창에 등록
							MaterialWnd->SetCustomData(&ItemPos, sizeof(ItemPos));
							lwDefenceMode::lwShowTargetAntiqueItem(lwUIWnd(pWnd));
							lwDefenceMode::lwClickAntiqueButtonUpDown(lwUIWnd(pWnd));
							lua_tinker::call<void>("InitTargetIcnAniTime");
							return true;
						}
						else
						{
							MaterialWnd->ClearCustomData();
							TargetWnd->ClearCustomData();
							int const iNo = (EGT_ANTIQUE == g_kGemStore.GetGemStoreType()) ? 750009 : 790544;
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iNo, true);
							return false;
						}
					}
				}

				for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
				{//아이템 교환 관련 처리
					pWnd = XUIMgr.Get( szExchangeUI[FormCount].szFrom );
					if( pWnd )
					{
						XUI::CXUI_Wnd * const MaterialWnd = dynamic_cast<XUI::CXUI_Wnd*>(pWnd->GetControl(L"ICN_MATERIAL"));
						XUI::CXUI_Wnd * const TargetWnd = dynamic_cast<XUI::CXUI_Wnd*>(pWnd->GetControl(L"ICN_TARGET"));
						if( MaterialWnd && TargetWnd )
						{
							SItemPos const ItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
							PgBase_Item Item;
							if(S_OK != pkInv->GetItem(ItemPos, Item))
							{
								return false;
							}
							DWORD TargetItemNo = 0;
							if( lwDefenceMode::IsCanExchangeItem(Item.ItemNo(), TargetItemNo, szExchangeUI[FormCount].Menu) )
							{ // 골동품 수집 가능한 아이템 이라면 재료창에 등록
								MaterialWnd->SetCustomData(&ItemPos, sizeof(ItemPos));
								TargetWnd->SetCustomData(&TargetItemNo, sizeof(TargetItemNo));
								lwDefenceMode::lwClickItemButtonUpDown(lwUIWnd(pWnd), szExchangeUI[FormCount].Menu);
								lwDefenceMode::lwShowTargetItem(lwUIWnd(pWnd), szExchangeUI[FormCount].Menu);
								lua_tinker::call<void>("InitTargetIcnAniTime");
								return true;
							}
							else
							{
								lwDefenceMode::lwInitExchangeItemWnd(lwUIWnd(pWnd));
								lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", szExchangeUI[FormCount].ErrorMsgNo, true);
								return false;
							}
						}
						break;
					}
				}
			}break;
		default:
			{
			}break;
		}
		if(IconDoAction() == EIAR_UICT_DONE)
		{ //마우스 우클릭으로 AT_USE_ITEM_CUSTOM_TYPE 속성의 아이템을 사용했을 경우 함수를 그냥 종료
			return true;
		}
	}/* 마우스 우클릭 아이템 이동 종료*/

	if( !bIsRBtnClicked && lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true) && XUIMgr.Get(_T("FRM_AUCTION_MAIN")) )
	{//마우스 좌클릭 + Shift = 검색란에 아이템 이름 등록		
		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return true;
		}

		SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
		PgBase_Item kItem;
		if(S_OK != pkInv->GetItem(kItemPos, kItem))
		{
			return true;
		}

		GET_DEF( CItemDefMgr, kItemDefMgr );
		CItemDef const* pItemDef = kItemDefMgr.GetDef( kItem.ItemNo() );
		if( !pItemDef )
		{
			return true;
		}

		std::wstring const* pkItemName = NULL;
		if( GetDefString(pItemDef->NameNo(),pkItemName) )
		{
			lwMarketUtil::SendMarketSearchText(*pkItemName);
		}

		return true;
	}

	switch(kTargetIconInfo.iIconGroup)
	{
	case KUIG_SAFE:
	case KUIG_SAFE_ADDITION:
	case KUIG_INV_VIEW://놓을곳이.
	case KUIG_FIT:
	case KUIG_FIT_CASH:
	case KUIG_RENTAL_SAFE1:
	case KUIG_RENTAL_SAFE2:
	case KUIG_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE1:
	case KUIG_SHARE_RENTAL_SAFE2:
	case KUIG_SHARE_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE4:
	case KUIG_PET:
	case KUIG_CONSUME_PET:
	case KUIG_FIT_PET:
	case KUIG_GUILD_INV_VIEW:
		{
			SItemPos CasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);//원래 아이템
			SItemPos TargetPos(kTargetIconInfo.iIconGroup, kTargetIconInfo.iIconKey);//타겟 아이템

			{//킥볼의 경우 UI가 실드와 겹쳐있어서 드래그 장착에 문제 발생. 때문에 강제로 위치 맞춰줌.
				if( KUIG_FIT == kTargetIconInfo.iIconGroup
					|| KUIG_FIT_CASH == kTargetIconInfo.iIconGroup )
				{
					PgInventory* pkInv = pkPlayer->GetInven();
					if( !pkInv )
					{
						return true;
					}
					SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
					PgBase_Item kItem;
					if(S_OK != pkInv->GetItem(kItemPos, kItem))
					{
						return true;
					}

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pItemDef = kItemDefMgr.GetDef( kItem.ItemNo() );
					if(pItemDef)
					{
						if( EQUIP_POS_KICKBALL == pItemDef->EquipPos() 
							&&EQUIP_POS_KICKBALL != kTargetIconInfo.iIconKey)
						{
							TargetPos.y = EQUIP_POS_KICKBALL;
						}
						else if ( EQUIP_POS_SHEILD == pItemDef->EquipPos() 
							&&EQUIP_POS_SHEILD != kTargetIconInfo.iIconKey)
						{
							TargetPos.y = EQUIP_POS_SHEILD;
						}
					}
				}
			}
			if(CasterPos == TargetPos)
			{// 지우지 마세요.
				break;
			}

			if( CasterPos == SItemPos::NullData() )
			{
				break;
			}

			switch(TargetPos.x)
			{
			case KUIG_INV_VIEW:
				{
					TargetPos.x = g_iInvViewGrp;
				}break;
			case KUIG_GUILD_INV_VIEW:
				{
					TargetPos.x = g_kGuildMgr.GetGuildInvView();
				}break;
			case KUIG_SAFE_ADDITION:
				{
					if(KUIG_FIT==CasterPos.x && EQUIP_POS_PET==CasterPos.y)
					{//장착하고 있는 추가인벤을 자기 자신에게 담을 수 없음
						return false;
					}
					if( (KUIG_FIT_CASH == CasterPos.x)  && (EQUIP_POS_RING_L == CasterPos.y) || (EQUIP_POS_RING_R == CasterPos.y) )
					{// 추가 인벤을 주는 반지 캐시아이템은 넣을수 없다.
						return false;
					}
				}break;
			}

			if(CasterPos == TargetPos)
			{
				break;
			}

			if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
			{
				lwAddWarnDataTT(400732);
				break;
			}

			if(XUIMgr.Get(_T("FRM_AUCTION_MAIN")))
			{
				lwAddWarnDataTT(403082);
				break;
			}

			if( true == g_kStyleModifier.IsAlive() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51207, true);
				break;
			}

			switch(CasterPos.x)
			{
			case KUIG_STORE:
			case KUIG_STOCK_STORE:
				{
					DoItemBuy(CasterPos.y);
				}break;
			case KUIG_INV_VIEW:
			case KUIG_FIT:
			case KUIG_FIT_PET:
			case KUIG_EQUIP:
			case KUIG_CONSUME:
			case KUIG_CONSUME_PET:
			case KUIG_ETC:
			case KUIG_CASH:
			case KUIG_GUILD_INV_VIEW:
			case KUIG_SAFE_ADDITION:
			case KUIG_FIT_CASH:
			case KUIG_SAFE:
			case KUIG_RENTAL_SAFE1:
			case KUIG_RENTAL_SAFE2:
			case KUIG_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE1:
			case KUIG_SHARE_RENTAL_SAFE2:
			case KUIG_SHARE_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE4:
			case KUIG_PET:
				{
					if(!IsItemMoveEnable())//"SFRM_ITEM_PLUS_UPGRADE", "SFRM_ITEM_RARITY_UPGRADE", "SFRM_REPAIR", "SFRM_SOCKET_SYSTEM", "FRM_ITEM_CONVERT"
					{// 아이템을 이동 할 수 없는 경우
						break;
					}
				}//no break
			case KUIG_CASH_SAFE:
				{
					PgBase_Item	kCasterItem;
					HRESULT kRet = S_FALSE;

					bool bCasterIsGuildInv = false;
					bool bTargetIsGuildInv = false;

					switch(CasterPos.x)
					{
					case KUIG_FIT_PET:
					case KUIG_CONSUME_PET:
						{
							PgInventory * pkPetInven = GetMySelectedPetInven();
							if(pkPetInven)
							{
								kRet = pkPetInven->GetItem(SItemPos(EKindUIIconGroup(CasterPos.x-190), CasterPos.y), kCasterItem);
							}
						}break;					
					case KUIG_GUILD_INV_VIEW:
						{
							bCasterIsGuildInv = true;
							CasterPos.x = g_kGuildMgr.GetGuildInvView();
							PgInventory* pkGuildInv = g_kGuildMgr.GetInven();
							if( pkGuildInv )
							{
								ConvertItemPosToGuildInvType(CasterPos);
								kRet = pkGuildInv->GetItem(CasterPos, kCasterItem);
							}
						}break;
					default:
						{
							kRet = pkPlayer->GetInven()->GetItem(CasterPos, kCasterItem);
						}break;
					}

					if( S_OK != kRet)
					{
						break;
					}

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pItemDef = kItemDefMgr.GetDef(kCasterItem.ItemNo());
					if( !pItemDef )
					{
						break;
					}

					if( S_OK == kRet )
					{
						if( true == CheckIsCashItem(kCasterItem) )
						{
							if( TargetPos.x == KUIG_FIT )
							{
								lwAddWarnDataTT(1279);
								break;
							}
						}
					}

					if( pItemDef->CanEquip())
					{if(    KUIG_FIT == CasterPos.x
						 && KUIG_FIT_CASH == CasterPos.x 
						 && false == CanChangeEquipItem(pItemDef) )
						{
							break;
						}
					}

					if( pItemDef->CanEquip() && pItemDef->GetAbil(AT_REVERSION_ITEM) && (TargetPos.x == KUIG_FIT) )
					{
						if( !kCasterItem.EnchantInfo().IsAttached() )
						{
							Send_PT_C_M_REQ_ITEM_CHANGE_YES_NO(5018, CasterPos, TargetPos);
							break;
						}
					}
					if(lwKeyIsDown(NiInputKeyboard::KEY_LSHIFT,true))
					{
						PgBase_Item kTargetItem;
						SItemPos kTempTargetPos(TargetPos);
						PgInventory* pkInv = NULL;

						switch( TargetPos.x )
						{
						case KUIG_GUILD_INVENTORY1:
						case KUIG_GUILD_INVENTORY2:
						case KUIG_GUILD_INVENTORY3:
						case KUIG_GUILD_INVENTORY4:
							{//지원안해.
								return false;
							}break;
						default:
							{
								pkInv = pkPlayer->GetInven();
							}break;
						}

						if( !pkInv )
						{
							break;
						}

						if(	!kCasterItem.IsEmpty()
							&&	S_OK == pkInv->GetItem(kTempTargetPos, kTargetItem))
						{
							if (pItemDef && pItemDef->IsAmountItem())
							{
								SCalcInfo kInfo;
								kInfo.eCallType = CCT_ITEM_DIVIDE;
								kInfo.iBasePrice = 0;
								kInfo.kGuid = kCasterItem.Guid();
								kInfo.iItemNo = kCasterItem.ItemNo();
								kInfo.kItemPos = CasterPos;
								CallCalculator(kInfo);

								break;
							}
						}
					}

					if(CasterPos.x==KUIG_PET && TargetPos.x!=KUIG_PET )
					{
						switch(TargetPos.x)
						{
						case KUIG_FIT_PET:		{TargetPos.x = KUIG_FIT;}break;
						case KUIG_CONSUME_PET:	{TargetPos.x = KUIG_CONSUME;}break;
						default:				{return false;}break;
						}

						Send_PT_C_M_REQ_ITEM_CHANGE_TO_PET(CasterPos, TargetPos, true);
					}
					else if(TargetPos.x==KUIG_PET && CasterPos.x!=KUIG_PET)
					{
						switch(CasterPos.x)
						{
						case KUIG_FIT_PET:		{CasterPos.x = KUIG_FIT;}break;
						case KUIG_CONSUME_PET:	{CasterPos.x = KUIG_CONSUME;}break;	
						default:				{return false;}break;
						}

						Send_PT_C_M_REQ_ITEM_CHANGE_TO_PET(TargetPos, CasterPos, true);
					}
					else
					{
						switch( TargetPos.x )
						{
						case KUIG_GUILD_INVENTORY1:
						case KUIG_GUILD_INVENTORY2:
						case KUIG_GUILD_INVENTORY3:
						case KUIG_GUILD_INVENTORY4:
							{
								bTargetIsGuildInv = true;
								ConvertItemPosToGuildInvType(TargetPos);
							}break;
						case KUIG_FIT:	//장착창에
							{
								if(EQUIP_POS_PET==TargetPos.y && KUIG_SAFE_ADDITION==CasterPos.x) //펫 위치에 추가 인벤토리 아이템을 놓으면 
								{
									return false;
								}
							}break;
						}

						if( bCasterIsGuildInv || bTargetIsGuildInv )
						{
							Send_PT_C_M_REQ_ITEM_CHANGE_GUILD(CasterPos, TargetPos, bCasterIsGuildInv, bTargetIsGuildInv );
						}
						else
						{
							Send_PT_C_M_REQ_ITEM_CHANGE(CasterPos, TargetPos);
						}
					}
				}break;
			case KUIG_SYSTEM_INVEN:
				{
					// 시스템 인벤에서 --> 장비/소비/기타/캐쉬 에만 들어 가도록
					switch( g_iInvViewGrp )
					{
					case KUIG_EQUIP:
					case KUIG_CONSUME:
					case KUIG_ETC:
					case KUIG_CASH:
						{
							g_kSystemInven.SendItemGetAt(CasterPos.y, TargetPos);
						}break;
					default:
						{
						}break;
					}
				}break;
			default:
				{//아무짓 안함.
				}break;
			}
		}break;
	case KUIG_QUICK_INVEN:
		{
			SQuickInvInfo kNewQuickInvInfo;

			kNewQuickInvInfo.Grp(kCursorIcon.iIconGroup);

			PgQuickInventory* pkQInv = pkPlayer->GetQInven();

			switch(kCursorIcon.iIconGroup)
			{
			case KUIG_CONSUME:
			case KUIG_ETC:
			case KUIG_CASH:
			case KUIG_EQUIP:
			case KUIG_PET:
				{
					if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
					{
						lwAddWarnDataTT(400732);
						break;
					}
				}
			case KUIG_SAFE:
			case KUIG_CASH_SAFE:
			case KUIG_RENTAL_SAFE1:
			case KUIG_RENTAL_SAFE2:
			case KUIG_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE1:
			case KUIG_SHARE_RENTAL_SAFE2:
			case KUIG_SHARE_RENTAL_SAFE3:
			case KUIG_SHARE_RENTAL_SAFE4:
				{
					PgBase_Item kItem;
					if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey), kItem))
					{
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());

						if( pItemDef->IsType(ITEM_TYPE_CONSUME) )
						{
							kNewQuickInvInfo.ID(kItem.ItemNo());
						}
						else if( pItemDef->IsType(ITEM_TYPE_USE_UI) )
						{
							lwAddWarnDataStr(TTW(700018), 0);
						}
						else
						{
							lwAddWarnDataStr(TTW(700018), 0);
						}
					}
				}break;
			case KUIG_GUILD_INVENTORY1:
			case KUIG_GUILD_INVENTORY2:
			case KUIG_GUILD_INVENTORY3:
			case KUIG_GUILD_INVENTORY4:
				{
				}break;
			case KUIG_SKILLTREE:
			case KUIG_SKILLTREE_PET:
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					const CSkillDef *pkSkillDef = kSkillDefMgr.GetDef(kCursorIcon.iIconKey);
					if(!pkSkillDef)
					{
						return false;
					}

					kNewQuickInvInfo.ID(kCursorIcon.iIconKey);
				}break;
			case KUIG_SKILLSET_BASIC:
			case KUIG_SKILLSET_SETICON:
				{
					kNewQuickInvInfo.ID(kCursorIcon.iIconKey);
				}break;
			case KUIG_QUICK_INVEN:
				{
					SQuickInvInfo kPrevQuickInvInfo;
					const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kCursorIcon.iIconKey), kPrevQuickInvInfo);//이전 퀵 인벤 가져옴.
					if(S_OK == hRet)
					{
						SQuickInvInfo kReserveQuickInvInfo;
						const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kTargetIconInfo.iIconKey), kReserveQuickInvInfo);//놓을 자리것. 온놈으로 보내기.

						kNewQuickInvInfo = kPrevQuickInvInfo;

						Send_PT_C_M_REQ_REGQUICKSLOT(pkQInv->AdjustSlotIDX(kCursorIcon.iIconKey), kReserveQuickInvInfo);//지우는것.
					}
				}
				break;
			case KUIG_JOB_SKILL:
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					const CSkillDef *pkSkillDef = kSkillDefMgr.GetDef(kCursorIcon.iIconKey);
					if(pkSkillDef)
					{
						if( pkSkillDef->GetType() != EST_PASSIVE)
						{
							PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
							if(pPlayer)
							{
								bool const bRet = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, kCursorIcon.iIconKey);
								if(bRet)
								{
									kNewQuickInvInfo.ID(kCursorIcon.iIconKey);
								}
							}
						}
					}
				}break;
			case KUIG_SAFE_ADDITION:
				{
					lwAddWarnDataTT(270);
				}break;
			case KUIG_ITEM_SKILL:
				{
					int const iItemSkillNo = kCursorIcon.iIconKey;
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iItemSkillNo);
					if(pkSkillDef)
					{
						if( ItemSkillUtil::CheckUseItemSkillAll(g_kPilotMan.GetPlayerUnit(), iItemSkillNo) )
						{
							kNewQuickInvInfo.ID(kCursorIcon.iIconKey);
						}
					}
				}break;
			default:
				{
					return false;
				}break;
			}

			if(!kNewQuickInvInfo.IsEmpty())
			{
				Send_PT_C_M_REQ_REGQUICKSLOT( pkQInv->AdjustSlotIDX(kTargetIconInfo.iIconKey), kNewQuickInvInfo);
			}
		}break;
	case KUIG_SKILLSET_SKILLICON:
		{
			switch(kCursorIcon.iIconGroup)
			{
			case KUIG_SKILLTREE:
			case KUIG_SKILLSET_JUMPICON:
				{
					self->SetCustomData(&kCursorIcon, sizeof(kCursorIcon));
					lwSkillSet::lwSetMinTime(lwUIWnd(self), kCursorIcon.iIconKey);
					lwSkillSet::lwChangeSaveState(true);
				}break;
			case KUIG_SKILLTREE_PET:
				{
					lwAddWarnDataTT(799414);	//펫스킬은 스킬세트 포함X
				}break;
			}
		}break;
	case KUIG_KEY_SET:
		{
			//From Trg, To Org
			//iIconGrp, iIconKey
			if(0 == kCursorIcon.iIconKey)
			{
				NILOG(PGLOG_ERROR, "Target Icon Key number is 0");
				PG_ASSERT_LOG(0);
				return false;
			}
			if(0 == kCursorIcon.iIconGroup || 0 == kCursorIcon.iIconKey)
			{
				return false;
			}

			std::vector<CXUI_Wnd*> kVec;
			CXUI_Icon *pkFromIcon = NULL;

			if(self == NULL) {	return false;	}
			if(self->Parent()->GetControlList_ByType(XUI::E_XUI_ICON, kVec))//from icon set
			{
				for(unsigned int i=0; i<kVec.size(); ++i)
				{
					CXUI_Icon *pkTempIconWnd = dynamic_cast<CXUI_Icon*>(kVec[i]);
					if(!pkTempIconWnd){assert(NULL); return false;}

					const SIconInfo &kTempInfo = pkTempIconWnd->IconInfo();
					if(kCursorIcon.iIconGroup==kTempInfo.iIconGroup && kCursorIcon.iIconKey==kTempInfo.iIconKey)//커서의 그룹/키를 이용해 원본을 찾는다.
					{
						pkFromIcon = pkTempIconWnd;
						break;
					}
				}
			}

			if(pkFromIcon)
			{
				int iFromSubKey = 0, iToSubKey = 0;
				if(pkFromIcon->GetCustomData(&iFromSubKey, sizeof(iFromSubKey)) && pkIconWnd->GetCustomData(&iToSubKey, sizeof(iToSubKey)))
				{
					std::string kSubKeyFrom, kSubKeyTo;
					if(!g_kGlobalOption.GetKeynoToKeystr(iFromSubKey, kSubKeyFrom))
					{
						NILOG(PGLOG_ERROR, "iKey[%d] is not found from str list", iFromSubKey); 
						return false;
					}
					if(!g_kGlobalOption.GetKeynoToKeystr(iToSubKey, kSubKeyTo))
					{
						NILOG(PGLOG_ERROR, "iKey[%d] is not found from str list", iToSubKey); 
						return false;
					}

					const SIconInfo kTempInfo = pkFromIcon->IconInfo();//Swap Icon Key (From, To)
					pkFromIcon->SetIconInfo(pkIconWnd->IconInfo());
					pkIconWnd->SetIconInfo(kTempInfo);

					g_kGlobalOption.SetConfig(XML_ELEMENT_KEYSET, kSubKeyFrom, pkFromIcon->IconInfo().iIconKey, NULL);
					g_kGlobalOption.SetConfig(XML_ELEMENT_KEYSET, kSubKeyTo, pkIconWnd->IconInfo().iIconKey, NULL);
				}
				else
				{
					PG_ASSERT_LOG(0); NILOG(PGLOG_ERROR, "Can't getcustom data");
				}
			}
			else
			{
				PG_ASSERT_LOG(0);
				NILOG(PGLOG_ERROR, "Target icon is not found");
				return false;
			}
		}break;
	case KUIG_STORE:
	case KUIG_STOCK_STORE:
		{
			SItemPos kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);//타겟 아이템

			if(kCasterPos.x == KUIG_QUICK_INVEN)
			{//퀵인벤도 취소.
				break;
			}

			if((kCasterPos.x == KUIG_FIT)
				|| (kCasterPos.x == KUIG_FIT_CASH) )
			{//장비창에서 바로 팔 수 없음
				lwAddWarnDataTT(9016);
				break;
			}

			PgInventory *pkInv = pkPlayer->GetInven();
			if (pkInv)
			{
				PgBase_Item kItem;
				if(S_OK == pkInv->GetItem(kCasterPos, kItem))
				{
					if(KUIG_STOCK_STORE == kTargetIconInfo.iIconGroup)
					{
						PgStore::CONT_GODDS::value_type kOut;
						if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kOut))
						{
							DoItemSell(kCasterPos, &kItem, true, kOut.iSellPrice);
						}
						else
						{//판매 할 수 없음.
							lwAddWarnDataTT(400195);
						}
					}
					else
					{
						DoItemSell(kCasterPos, &kItem);
					}
				}
			}
			return true;
		}break;
	case KUIG_MARKET:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			g_kMarketMgr.TempRegItemInfo(self, kItemPos);
		}break;
	case KUIG_ITEM_PLUS_UPGRADE_SRC:
	case KUIG_ITEM_PLUS_UPGRADE_INSURENCE:
	case KUIG_ITEM_PLUS_UPGRADE_PROBABILITY:
		{
			PgInventory *pkInv = pkPlayer->GetInven();
			if( !pkInv )
			{
				return false;
			}

			SItemPos kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kCasterPos, kItem))
			{
				return false;
			}

			if( kTargetIconInfo.iIconGroup == KUIG_ITEM_PLUS_UPGRADE_SRC )
			{
				if( !CheckEnchantBundle(kItem) )
				{
					lwAddWarnDataTT(1248);
					return false;
				}
				g_kItemPlusUpgradeMgr.SetItem((EKindUIIconGroup)kTargetIconInfo.iIconGroup, kCursorIcon);
				break;
			}
			g_kItemPlusUpgradeMgr.SetMaterialItem(self->Parent()->Parent());
		}break;
	case KUIG_ITEM_RARITY_UPGRADE_SRC:
	case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:
	case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:
		{
			if( KUIG_ITEM_RARITY_UPGRADE_SRC != kTargetIconInfo.iIconGroup )
			{
				if( kCursorIcon.iIconGroup == KUIG_NONE )
				{
					break;
				}

				switch( kTargetIconInfo.iIconGroup )
				{
				case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:	{ g_kItemRarityUpgradeMgr.SetMaterialItem(self->Parent(), PgItemRarityUpgradeMgr::RIT_INSUR_ITEM);  } break;
				case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:	{ g_kItemRarityUpgradeMgr.SetMaterialItem(self->Parent(), PgItemRarityUpgradeMgr::RIT_PROBABILITY); } break;
				}
				break;
			}
			g_kItemRarityUpgradeMgr.SetItem((EKindUIIconGroup)kTargetIconInfo.iIconGroup, kCursorIcon);
		}break;
	case KUIG_ITEM_RARITY_BUILDUP_SRC:
		{// 옵션 강화
			g_kItemRarityBuildUpMgr.SetItem( (EKindUIIconGroup)kTargetIconInfo.iIconGroup, kCursorIcon );
		}break;
	case KUIG_ITEM_RARITY_AMPLIFY_SRC:
		{// 옵션 증폭
			g_kItemRarityAmplify.SetItem( (EKindUIIconGroup)kTargetIconInfo.iIconGroup, kCursorIcon );
		}break;
	case KUIG_REDICE_CRAFT_OPT:
		{
			if( KUIG_REDICE_CRAFT_OPT != kCursorIcon.iIconGroup )
			{
				lwCraftOptChanger::CallCraftOptChanger(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey));
			}
		}break;
	case KUIG_ENCHANT_SHIFT_SRC:
		{
			SItemPos const kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			if( g_kEnchantShift.SetSrcItem(kCasterPos) )
			{
				self->SetCustomData(&kCasterPos, sizeof(kCasterPos));
			}
		}break;
	case KUIG_ENCHANT_SHIFT_TGT:
		{
			SItemPos const kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			if( g_kEnchantShift.SetTargetItem(kCasterPos) )
			{
				self->SetCustomData(&kCasterPos, sizeof(kCasterPos));
			}
		}break;
	case KUIG_COMMON_INV_POS:
		{
			PgInventory *pkInv = pkPlayer->GetInven();
			if( !pkInv )
			{
				return false;
			}

			switch( kCursorIcon.iIconGroup )
			{
			case KUIG_EQUIP:
			case KUIG_CONSUME:
			case KUIG_ETC:
			case KUIG_CASH:
				{
					SItemPos kCasterPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);//타겟 아이템

					switch( pkIconWnd->IconInfo().iIconKey )
					{
					case EKUCT_MON_CARD_T2:
						{
							lwItemMix::SetMonsterCardMixerType2MixItem( kCasterPos );
						}break;
					case EKUCT_CRAFT:
						{
							if( lwLuckyChanger::SetItem(kCasterPos) )
							{
								self->SetCustomData(&kCasterPos, sizeof(kCasterPos));
								lwLuckyChanger::OnDragUpItem(self);
							}
						}break;
					default:
						{
							self->SetCustomData(&kCasterPos, sizeof(kCasterPos));
						}break;
					}

				}break;
			default:
				{	
				}break;
			}
		}break;
	case KUIG_SOCKET_SYSTEM:
		{
			if( KUIG_SOCKET_SYSTEM != kTargetIconInfo.iIconGroup )
			{
				if( kCursorIcon.iIconGroup == KUIG_NONE )
				{
					break;
				}
				switch( kTargetIconInfo.iIconGroup )
				{
				case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:	{ g_kSocketSystemMgr.SetMaterialItem(self->Parent(), PgItemRarityUpgradeMgr::RIT_INSUR_ITEM, true);  } break;
				case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:	{ g_kSocketSystemMgr.SetMaterialItem(self->Parent(), PgItemRarityUpgradeMgr::RIT_PROBABILITY, true); } break;
				}
				//g_kSocketSystemMgr.SetMaterialItem(self->Parent(), PgItemSocketSystemMgr::SIT_INSUR_ITEM);
				break;
			}
			g_kSocketSystemMgr.SetItem((EKindUIIconGroup)kTargetIconInfo.iIconGroup, kCursorIcon);

			if( true == g_kSocketSystemMgr.IsSocketItemEmpty() )
			{
				g_kSocketSystemMgr.Clear();
			}
		}break;
	case KUIG_ITEMCONVERT_SYSTEM:
		{
			if( KUIG_ITEMCONVERT_SYSTEM != kTargetIconInfo.iIconGroup )
			{
				if( kCursorIcon.iIconGroup == KUIG_NONE )
				{
					break;
				}
				g_kItemConvertSystemMgr.SetMaterialItem(self->Parent());
				break;
			}
			return g_kItemConvertSystemMgr.SetItem(kCursorIcon);
		}break;
	case KUIG_ITEM_REPAIR:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			if(g_kItemRepair.SetSrcItem(kItemPos))
			{
			}
		}break;
	case KUIG_ITEM_TRADE_MY:	//교환
		{
			if (KUIG_FIT == kCursorIcon.iIconGroup)
			{
				lwAddWarnDataTT(400723);
				break;
			}
			if (g_kTradeMgr.IsConfirm())
			{
				break;
			}
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgBase_Item kItem;

			if(PgTradeMgr::CheckItemToGiveOther(kItemPos, pkPlayer, kItem))
			{
				g_kTradeMgr.ModifyItem(kItemPos, &kItem);
			}
			else
			{
				if(0!=kCursorIcon.iIconGroup && KUIG_ITEM_TRADE_MY!=kCursorIcon.iIconGroup)	//교환창 빈 곳을 더블클릭 혹은 드래그
				{
					lwAddWarnDataTT(400719);
				}
			}
		}break;
	case KUIG_ITEM_MAKING:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			return g_kItemMaking.SetSrcItem(kItemPos, pkIconWnd->BuildIndex());
		}break;
	case KUIG_ITEM_MAKING_MAZE:
		{
			SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			g_kEqItemMixCreator.Clear();
			if( g_kEqItemMixCreator.SetRecipe(kItemPos) )
			{
				lwItemMix::CallEqItemMixCreatorUI();
			}
		}break;
	case KUIG_ITEM_UNLOCK_SLOT:
		{
			SItemPos const kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			lwUseUnlockItem::SetTargetItem( kItemPos );
		}break;
	case KUIG_ITEM_MAKING_COOK:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);

			PgBase_Item kItem;
			HRESULT const kRet = pkPlayer->GetInven()->GetItem(kItemPos, kItem);
			if( S_OK != kRet )
			{
				break;
			}
			else
			{
				g_kItemCookingMgr.SetSrcItem(kItemPos);
			}
		}break;
	case KUIG_ITEM_SHINE_LOTTERY:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			lwUIWnd kUI(pkIconWnd);
			return g_kItemLottery.DragToSlot(kItemPos, kUI.GetBuildIndex());
		}break;
	case KUIG_ITEM_MAIL:
		{
			if (KUIG_FIT == kCursorIcon.iIconGroup)
			{
				lwAddWarnDataTT(954);
				break;
			}
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			if(!g_kMailMgr.SetSrcItem(kItemPos))
			{
				lwCloseToolTip();
			}
		}break;
	case KUIG_ITEM_BIND:
		{
			PgInventory *pkInv = pkPlayer->GetInven();
			if(!pkInv) 
			{
				return false; 
			}

			SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kTargetItemPos, kItem))
			{
				return false;
			}
			PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
			if(pkPlayerActor)
			{
				pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Binding_Drop", 0.0f);			
			}
			g_kItemBindMgr.SetTargetItemFromInv(kTargetItemPos);
			return true;
		}break;
	case KUIG_ITEM_UNBIND:
		{
			PgInventory *pkInv = pkPlayer->GetInven();
			if(!pkInv) 
			{
				return false;
			}

			SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kTargetItemPos, kItem))
			{
				return false;
			}
			PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
			if(pkPlayerActor)
			{
				pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "Item_Binding_Drop", 0.0f);			
			}
			g_kItemUnbindMgr.SetTargetItemFromInv(kTargetItemPos);
			return true;
		}break;
	case KUIG_ITEM_PET_FEED:
		{
			g_kPetUIMgr.SetSrcItem(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey), pkPlayer->GetInven(), pkIconWnd->Parent());
		}break;
	case KUIG_REDICE_PET_OPT:
		{
			SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			lwPetOptChanger::SetSrcItem(kTargetItemPos);
		}break;
	case KUIG_ITEM_PET_UPGRADE:
		{
			SItemPos kTargetItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			lwPetUpgrade::SetSrcItem(kTargetItemPos);
		}break;
	case KUIG_EXP_POTION:
		{
			g_kPetExpUIMgr.SetSrcItem(SItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey), pkIconWnd->BuildIndex());
		}break;
	case KUIG_TREASURE_CHEST_KEY:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgInventory *pkInv = pkPlayer->GetInven();
			if(!pkInv) 
			{
				return false;
			}
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kItemPos, kItem))
			{
				return false;
			}
			lwTreasureChest::InsertKey(&kItem, kItemPos);
		}break;
	case KUIG_MANUFACTURE_MATERIAL:
		{
			const SItemPos kItemPos(kCursorIcon.iIconGroup, kCursorIcon.iIconKey);
			PgInventory *pkInv = pkPlayer->GetInven();
			if(!pkInv) 
			{
				return false;
			}
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kItemPos, kItem))
			{
				return false;
			}
			lwManufacture::InsertBundleManMtrl(&kItem, kItemPos);
			//여기서 UI 아이템 등록 처리를 해준다...
		}break;
	case KUIG_ANY:
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(pkPlayer)
			{
				unsigned int uiGroup = kCursorIcon.iIconGroup;
				SItemPos const kCasterPos(uiGroup, kCursorIcon.iIconKey);
				PgBase_Item kCasterItem;
				if(S_OK == pkPlayer->GetInven()->GetItem(kCasterPos, kCasterItem))
				{
					AppendItemInfoToEditBox(kCasterItem, kCasterPos);
				}
			}
			else
			{
				return false;
			}
		}break;
	default:
		{
			return false;
		}break;
	}
	//	pCursor->IconInfo().Clear();//아이콘으로 주고. 클리어

	return true;
}

void lwUIWnd::lwSetBuildIconKey()
{
	//XUI::CXUI_Wnd* pWnd = ;
	XUI::CXUI_Icon *pkIcon = dynamic_cast<XUI::CXUI_Icon*>(self);
	if(NULL != pkIcon)
	{
		SIconInfo kIconInfo = pkIcon->IconInfo();

		int iIconKey= 0;

		if(KUIG_KEY_SET == kIconInfo.iIconGroup)//키셋일 때
		{
			std::wstring kDefaultID = WSTR_KEYSET_BTN_HEADER;//here use Only one this text
			std::wstring kID = pkIcon->ID();
			std::wstring kTemp = kID.substr(kDefaultID.size(), kID.size()-kDefaultID.size());
			int iKey = _ttoi(kTemp.c_str());
			if(0 != iKey)
			{
				ConvNiKey(iKey);
				std::string kKeyStr;
				//char const* szKeyStr = g_kGlobalOption.GetKeynoToKeystr(iKey, kKeyStr);
				if(g_kGlobalOption.GetKeynoToKeystr(iKey, kKeyStr))
				{
					iIconKey = g_kGlobalOption.GetValue(XML_ELEMENT_KEYSET, kKeyStr);//<-Current Config로 부터
				}
				else
				{
					NILOG(PGLOG_ERROR, "unknown key number");
					PG_ASSERT_LOG(0);
				}
				if(0 == kIconInfo.iIconKey)
				{
					NILOG(PGLOG_LOG, "Key %s is not user config using default map", kKeyStr.c_str());
					iIconKey = iKey + 1000;//
				}

				//pkIcon->SetCustomData(szKeyStr, sizeof(szKeyStr));//
				pkIcon->SetCustomData(&iKey, sizeof(iKey));
			}
			else
			{
				NILOG(PGLOG_ERROR, "Can't Converted KeySet Object ID [%s] to Unique Key No", kID.c_str());
				PG_ASSERT_LOG(0);
			}
		}
		else//키셋이 아닐 때
		{
			iIconKey = pkIcon->BuildIndex();
		}

		kIconInfo.iIconKey = iIconKey;
		pkIcon->SetIconInfo(kIconInfo);

		return;
	}
	NILOG(PGLOG_ERROR, "self is can't conter to icon");
	PG_ASSERT_LOG(0);
}

void lwUIWnd::lwSetBuildIChannelBtn()
{
	//XUI::CXUI_Wnd* pWnd = ;
	XUI::CXUI_Button *pkBtn = dynamic_cast<XUI::CXUI_Button*>(self);

	if(NULL != pkBtn)
	{
		POINT2 ptPosIdx(pkBtn->BuildIndex()/5,pkBtn->BuildIndex()%5);

		//		pkBtn->Location( pkBtn->Width() * ptPosIdx.x, pkBtn->Height() * ptPosIdx.y);	
		/*		
		pkBtn->Location(

		SIconInfo kIconInfo = pkIcon->IconInfo();

		int iIconKey= 0;

		if(KUIG_KEY_SET == kIconInfo.iIconGroup)//키셋일 때
		{
		std::wstring kDefaultID = WSTR_KEYSET_BTN_HEADER;//here use Only one this text
		std::wstring kID = pkIcon->ID();
		std::wstring kTemp = kID.substr(kDefaultID.size(), kID.size()-kDefaultID.size());
		int iKey = _ttoi(kTemp.c_str());
		if(0 != iKey)
		{
		std::string kKeyStr;
		//char const* szKeyStr = g_kGlobalOption.GetKeynoToKeystr(iKey, kKeyStr);
		if(g_kGlobalOption.GetKeynoToKeystr(iKey, kKeyStr))
		{
		iIconKey = g_kGlobalOption.GetValue(XML_ELEMENT_KEYSET, kKeyStr);//<-Current Config로 부터
		}
		else
		{
		NILOG(PGLOG_ERROR, "unknown key number");
		PG_ASSERT_LOG(0);
		}
		if(0 == kIconInfo.iIconKey)
		{
		NILOG(PGLOG_LOG, "Key %s is not user config using default map", kKeyStr.c_str());
		iIconKey = iKey + 1000;//
		}

		//pkIcon->SetCustomData(szKeyStr, sizeof(szKeyStr));//
		pkIcon->SetCustomData(&iKey, sizeof(iKey));
		}
		else
		{
		NILOG(PGLOG_ERROR, "Can't Converted KeySet Object ID [%s] to Unique Key No", kID.c_str());
		PG_ASSERT_LOG(0);
		}
		}
		else//키셋이 아닐 때
		{
		iIconKey = pkIcon->BuildIndex();
		}

		kIconInfo.iIconKey = iIconKey;
		pkIcon->SetIconInfo(kIconInfo);

		return;
		*/
	}

	NILOG(PGLOG_ERROR, "self is can't conter to icon");
	PG_ASSERT_LOG(0);
}

void lwUIWnd::DeleteIcon()// 지운다고 서버에게 통보
{
	BM::GUID kGuid;

	CXUI_Icon *pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(!pkIconWnd){return;}

	const SIconInfo &kIconInfo = pkIconWnd->IconInfo();

	switch(kIconInfo.iIconGroup)
	{
		//	case KUIG_INVEN:
		//		{
		//		}break;
	case KUIG_EQUIP:
		{
			if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
			{
				lwAddWarnDataTT(400732);
				break;
			}
		}break;
	case KUIG_QUICK_INVEN:
		{	// 찾아서 아무것도 없다면 보내지 않는다.
			// 서버에서는 아무것도 없다면 응답을 보내지 않는다. (그다지 효용성은 없을듯...) - 맵서버에서 false 반환하면 됨!
			SQuickInvInfo kQuickInvInfo;
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return;}

			PgQuickInventory *pkQInv = pkPlayer->GetQInven();

			const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다

			if(S_OK == hRet
				&& !kQuickInvInfo.IsEmpty() )
			{
				SQuickInvInfo kTempQuickInvInfo;
				Send_PT_C_M_REQ_REGQUICKSLOT(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kTempQuickInvInfo);//지워서 등록
			}
		}break;
	case KUIG_SKILLTREE:
		{
		}break;
	default:
		{
		}break;
	}
}

void lwUIWnd::GetSkillCoolTimeInfo(lwActor klwActor, XUI::CXUI_Icon* pParent, XUI::CXUI_Wnd* pToggleImg, float& fRemainTime, float& fTotalTime)
{
	bool const bPastGray = pParent->GrayScale();
	PgSkillTree::stTreeNode *pkNode = NULL;
	if(klwActor.GetSkillTreeNode(pParent->IconInfo().iIconKey, pkNode))
	{
		if (pkNode->m_pkSkillDef)
		{
			bool bPetSkill = false;
			if(90000001 > pkNode->GetSkillDef()->No() && 80000000 < pkNode->GetSkillDef()->No())
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(pkPlayer)
				{
					bPetSkill = true;
					PgActorPet* pPet = dynamic_cast<PgActorPet*>(g_kPilotMan.FindActor(pkPlayer->SelectedPetID()));
					if(pPet)
					{
						klwActor = lwActor(pPet);
					}
				}
			}
			if (pkNode->m_pkSkillDef->GetType() == EST_TOGGLE)
			{
				PgActor::TOGGLE_SKILL_SET const * kToggleSet = klwActor()->GetSkillToggleInfo();
				if (kToggleSet)
				{
					PgActor::TOGGLE_SKILL_SET::const_iterator skill_it = kToggleSet->find(pkNode->m_ulKeySkillNo);
					pToggleImg->Visible(kToggleSet->end() != skill_it);
				}
			}
			else
			{
				pToggleImg->Visible(false);
			}

			CUnit* pkUnit = NULL;
			if(klwActor()->GetPilot() && klwActor()->GetPilot()->GetUnit())
			{
				pkUnit = klwActor()->GetPilot()->GetUnit();
			}

			bool bGrayScale = !CheckHPMPForAction(pkNode->m_pkSkillDef, pkUnit, false, false);

			if(pkNode->IsLearned() == false)
			{
				bGrayScale = true;
			}

			if(bPetSkill)
			{
				bGrayScale = !PgPetUIUtil::IsExistPetSkill(pkNode->GetSkillDef()->No());
			}

			if ( bPastGray != bGrayScale )	//이전이랑 다르면
			{
				pParent->GrayScale(bGrayScale);
				pParent->Invalidate(true);
			}
		}

		PgActor::stSkillCoolTimeInfo const *pkInfo = klwActor()->GetSkillCoolTimeInfo();
		if (pkInfo && !pkInfo->m_CoolTimeInfoMap.empty())
		{
			PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = pkInfo->m_CoolTimeInfoMap.find(pkNode->m_ulKeySkillNo);
			if(itor != pkInfo->m_CoolTimeInfoMap.end())
			{
				fTotalTime = itor->second.m_ulTotalCoolTime*0.001f;

				fRemainTime = (float)itor->second.m_ulTotalCoolTime - ((float)BM::GetTime32() - itor->second.m_ulCoolStartTime);

				fRemainTime *= 0.001f;
			}	
		}
	}
	else
	{
		pToggleImg->Visible(false);
		if(!bPastGray)
		{
			pParent->GrayScale(true);
			pParent->Invalidate(true);
		}
	}
}

void lwUIWndUtil::GetSkillSetCoolTimeInfo(lwActor klwActor, int const iSetNo, XUI::CXUI_Icon* pParent, float& fMaxRemainTime, float& fMaxTotalTime)
{
	PgActor * pkActor =  klwActor();
	if( !pkActor ){ return; }
	if( !pParent ){ return; }

	CUnit* pkUnit = NULL;
	if( !pkActor->GetPilot() || !pkActor->GetPilot()->GetUnit() )
	{
		return;
	}
	pkUnit = pkActor->GetPilot()->GetUnit();

	bool const bPastGray = pParent->GrayScale();

	PgActor::stSkillCoolTimeInfo const * pkInfo = pkActor->GetSkillCoolTimeInfo();
	if( !pkInfo ){ return; }

	CONT_USER_SKILLSET::mapped_type const * kContSkillSet = pkActor->SkillSetAction().find(iSetNo);
	if( !kContSkillSet)
	{
		pParent->GrayScale(true);
		pParent->Invalidate(true);
		return; 
	}

	fMaxTotalTime = 0.f;
	fMaxRemainTime = 0.f;

	bool bGrayScale = false;
	bool bFirstHPMPCheck = true;
	bool bSkillSetEmpty = true;
	for(int i=0;i<MAX_SKILLSET_GROUP;++i)
	{
		int const iSkillNo = kContSkillSet->iSkillNo[i];
		if( iSkillNo<1 ){ continue; }
		bSkillSetEmpty = false;

		PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
		if( !pkNode ){ continue; }

		if( !bGrayScale && bFirstHPMPCheck )
		{
			bGrayScale = !CheckHPMPForAction(pkNode->m_pkSkillDef, pkUnit, false, false);
			bFirstHPMPCheck = false;
		}

		if( !pkNode->IsLearned() )
		{
			bGrayScale = true;
		}

		if( pkInfo->m_CoolTimeInfoMap.empty() )
		{
			continue;
		}
		PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = pkInfo->m_CoolTimeInfoMap.find(pkNode->m_ulKeySkillNo);
		if(itor != pkInfo->m_CoolTimeInfoMap.end())
		{
			float const fTotalTime = itor->second.m_ulTotalCoolTime*0.001f;

			float fRemainTime = (float)itor->second.m_ulTotalCoolTime - ((float)BM::GetTime32() - itor->second.m_ulCoolStartTime);
			fRemainTime *= 0.001f;

			if(fRemainTime > fMaxRemainTime)
			{
				fMaxTotalTime = fTotalTime;
				fMaxRemainTime = fRemainTime;
			}
		}
	}

	if( !pkActor->SkillSetAction().IsReserveActionEmpty() )
	{
		bGrayScale = true;
	}
	if( bSkillSetEmpty )
	{
		bGrayScale = true;
	}

	switch( pkActor->SkillSetAction().CheckCanSkillSetDoAction(g_kPilotMan.GetPlayerUnit(),iSetNo) )
	{
	case ESSR_NOT_CLASS:
	case ESSR_NOT_USE_ITEM:
		{
			bGrayScale = true;
		}break;
	}

	if ( bPastGray != bGrayScale )	//이전이랑 다르면
	{
		pParent->GrayScale(bGrayScale);
		pParent->Invalidate(true);
	}
}

void lwUIWnd::GetItemSkillCoolTimeInfo(lwActor klwActor, int const iSkillNo, XUI::CXUI_Icon* pParent, float& fRemainTime, float& fTotalTime)
{
	bool const bPastGray = pParent->GrayScale();
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const *pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(pSkillDef)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		CUnit* pkUnit = NULL;
		if(klwActor()->GetPilot() && klwActor()->GetPilot()->GetUnit())
		{
			pkUnit = klwActor()->GetPilot()->GetUnit();
		}

		bool bGrayScale = !CheckHPMPForAction(pSkillDef, pkUnit, false, false);

		if( ItemSkillUtil::CheckUseItemSkill(pkPlayer, pSkillDef->No(), IT_FIT) == false
			&& ItemSkillUtil::CheckUseItemSkill(pkPlayer, pSkillDef->No(), IT_FIT_CASH) == false)
		{
			bGrayScale = true;
		}

		if ( bPastGray != bGrayScale )	//이전이랑 다르면
		{
			pParent->GrayScale(bGrayScale);
			pParent->Invalidate(true);
		}

		PgActor::stSkillCoolTimeInfo const *pkInfo = klwActor()->GetSkillCoolTimeInfo();
		if (pkInfo && !pkInfo->m_CoolTimeInfoMap.empty())
		{
			PgActor::stSkillCoolTimeInfo::CoolTimeInfoMap::const_iterator itor = pkInfo->m_CoolTimeInfoMap.find(pSkillDef->GetParentSkill() );
			if(itor != pkInfo->m_CoolTimeInfoMap.end())
			{
				fTotalTime = itor->second.m_ulTotalCoolTime*0.001f;

				fRemainTime = (float)itor->second.m_ulTotalCoolTime - ((float)BM::GetTime32() - itor->second.m_ulCoolStartTime);

				fRemainTime *= 0.001f;
			}	
		}
	}
	else
	{
		if(!bPastGray)
		{
			pParent->GrayScale(true);
			pParent->Invalidate(true);
		}
	}
}

void lwUIWnd::QuickSlotIcon_OnDisplay()
{
	if (!self)
	{
		NILOG(PGLOG_ERROR, "[QuickSlotIcon_OnDisplay] self is NULL");
		return;
	}

	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if (!pkActor)
	{
		NILOG(PGLOG_ERROR, "[QuickSlotIcon_OnDisplay] pkActor is NULL");
		return;
	}

	XUI::CXUI_Icon *pkParent = dynamic_cast<XUI::CXUI_Icon*>(self->Parent());
	if (!pkParent)
	{
		NILOG(PGLOG_ERROR, "[QuickSlotIcon_OnDisplay] pkParent is NULL");
		return;
	}

	XUI::CXUI_Wnd *pkToggleImg = pkParent->GetControl(_T("FRM_TOGGLE_IMG"));
	if (!pkToggleImg)
	{
		NILOG(PGLOG_ERROR, "[QuickSlotIcon_OnDisplay] pkToggleImg is NULL");
		return;
	}

	float fRemainCoolTime = 0.0f;
	float fTotalCoolTime = 0.0f;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	XUI::CXUI_Wnd *pkImg = pkParent->GetControl(_T("FRM_COOTIME_IMG"));
	if (!pkImg)
	{
		NILOG(PGLOG_ERROR, "[QuickSlotIcon_OnDisplay] pkImg is NULL");
		return;
	}

	SQuickInvInfo kQuickInvInfo;
	PgQuickInventory* pkQInv = pkPlayer->GetQInven();
	const HRESULT hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(pkParent->IconInfo().iIconKey), kQuickInvInfo);
	if( hRet == S_OK )
	{
		switch(kQuickInvInfo.Grp())
		{
		case KUIG_CONSUME:
		case KUIG_CASH:
		case KUIG_PET:
			{
				PgInventory::SItemCoolTimeInfo kCoolTime;
				if( !pkPlayer->GetInven()->GetCoolTime(kQuickInvInfo.ID(), kCoolTime) )
				{
					break;
				}

				size_t const total_count = pkPlayer->GetInven()->GetTotalCount(kQuickInvInfo.ID());
				if( 0 == total_count )
				{
					break;
				}
				fTotalCoolTime = kCoolTime.dwTotalCoolTime * 0.001f;
				float const fEndTime = kCoolTime.dwEndCoolTime * 0.001f;
				float const fNowTime = g_kEventView.GetServerElapsedTime() * 0.001f; //현제시간을 구해야 하는데 언제인지 알 수 없다 어떻게 하나?
				fRemainCoolTime = fEndTime - fNowTime;
			}break;
		case KUIG_SKILLTREE:
		case KUIG_SKILLTREE_PET:
			{
				GetSkillCoolTimeInfo(lwActor(pkActor), pkParent, pkToggleImg, fRemainCoolTime, fTotalCoolTime);
			}break;
		case KUIG_SKILLSET_BASIC:
		case KUIG_SKILLSET_SETICON:
			{
				lwUIWndUtil::GetSkillSetCoolTimeInfo(lwActor(pkActor), kQuickInvInfo.ID(), pkParent, fRemainCoolTime, fTotalCoolTime);
			}break;
		case KUIG_ITEM_SKILL:
			{
				GetItemSkillCoolTimeInfo(lwActor(pkActor), kQuickInvInfo.ID(), pkParent, fRemainCoolTime, fTotalCoolTime);
			}break;
		default:
			{
				if( pkImg->Visible() )
					pkImg->Visible(false);
				self->Text(_T(""));
				return;
			}
		}
	}

	if (0.0f<fRemainCoolTime)
	{
		if (10.0f<fRemainCoolTime)
		{
			SetFontColorRGBA(255,255,0, 255);
		}
		else
		{
			SetFontColorRGBA(255,100,128, 255);
		}

		int iText = 90002;

		const POINT2 &rkSize = self->Size();
		float fCoolRate = __max(0.0f, fRemainCoolTime/fTotalCoolTime*rkSize.y);
		fCoolRate = __min(fCoolRate, rkSize.y);

		if (3600.0f<=fRemainCoolTime)
		{
			iText+=2;
			fRemainCoolTime = fRemainCoolTime/3600.0f;
		}
		else if (60.0f<=fRemainCoolTime)
		{
			iText+=1;
			fRemainCoolTime = fRemainCoolTime/60.0f;
		}

		pkImg->Visible(true);
		pkImg->Location(POINT2(pkImg->Location().x, rkSize.y - fCoolRate));
		pkImg->ImgSize(POINT2(pkImg->ImgSize().x, fCoolRate));

		BM::vstring str((int)fRemainCoolTime);
		str+=TTW(iText);
		self->Text((std::wstring const&)str);
	}
	else
	{
		pkImg->ImgSize(POINT2(pkImg->ImgSize().x, 0));
		pkImg->Visible(false);
		self->Text(_T(""));
	}
}

void lwUIWnd::SetIconGroup(int iGroup)
{
	XUI::CXUI_Icon *pkIcon = dynamic_cast<XUI::CXUI_Icon *>(self);
	if (pkIcon)
	{
		SIconInfo kIconInfo = pkIcon->IconInfo();
		kIconInfo.iIconGroup = iGroup;
		pkIcon->SetIconInfo(kIconInfo);
	}
}

bool lwViewOtherEquip(lwGUID kGuid)
{
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid());
	if (!pkPilot)	{return false;}
	CUnit *pkUnit = pkPilot->GetUnit();
	if (!pkUnit)	{return false;}
	PgInventory *pkInv = pkUnit->GetInven();
	if (!pkInv)		{return false;}

	BM::Stream kPacket;
	pkInv->WriteToPacket(IT_FIT, kPacket);
	pkInv->WriteToPacket(IT_FIT_CASH, kPacket);
	g_kOtherViewInv.Clear();
	g_kOtherViewInv.ReadFromPacket(IT_FIT, kPacket);
	g_kOtherViewInv.ReadFromPacket(IT_FIT_CASH, kPacket);

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_OTHER_CharInfo"));
	if( !pkWnd || pkWnd->IsClosed() )
	{
		pkWnd = XUIMgr.Call(_T("SFRM_OTHER_CharInfo"));
	}
	else
	{
		g_kMyActorViewMgr.DeleteActor("CHAROTHERINFO");
	}

	if(pkWnd)
	{
		g_kMyActorViewMgr.Update("CHAROTHERINFO", pkUnit->GetID());
		lwCharInfo::lwSetCharInfoToUI(lwUIWnd(pkWnd->GetControl(L"FRM_CHAR_INFO")), pkUnit->GetID());
		lwCharacterCard::lwCallCharacterCardUI(pkUnit->GetID());
	}

	return true;
}

bool lwViewPetEquip()
{
	PgInventory *pkInv = GetMySelectedPetInven();
	if ( !pkInv )
	{
		return false;
	}

	BM::Stream kPacket;
	pkInv->WriteToPacket(IT_FIT, kPacket);
	g_kPetViewInv.Clear();
	g_kPetViewInv.ReadFromPacket(IT_FIT, kPacket);

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("CharInfo"));
	if( !pkWnd || pkWnd->IsClosed() )
	{
		return false;	//UI가 안떳으면 끝
	}

	g_kMyActorViewMgr.ChangePetEquip("PetActor", pkInv->OwnerGuid());
	//lwCharInfo::lwSetCharInfoToUI(lwUIWnd(pkWnd), pkInv->OwnerGuid());

	return true;
}

void lwUIWnd::CallCashItemToolTip(bool bGift)
{
	assert(self);
	assert(self->Parent());
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	assert(pCursor);
	if(pCursor)
	{
		if(bGift)
		{// 캐시샵의 선물함용 툴팁
			XUI::CXUI_Wnd *pkParent= self->Parent();
			if(pkParent)
			{
				XUI::CXUI_Wnd* pkGetBtn = pkParent->GetControl(L"BTN_GET_GIFT");
				if(pkGetBtn)
				{
					lwUIWnd kTempWnd(pkGetBtn);
					lwPacket klwPacket = kTempWnd.GetCustomDataAsPacket();
					SCASHGIFTINFO kCashShopGiftInfo;
					BM::Stream* pkPacket = klwPacket();
					if(pkPacket)
					{
						kCashShopGiftInfo.ReadFromPacket(*pkPacket);
						CallToolTip_GiftInCashShop(self, pCursor->GetTotalLocation(), kCashShopGiftInfo);
					}
				}
			}
		}
		else
		{// 캐시 아이템 툴팁
			CallToolTip_Cash(self, pCursor->GetTotalLocation());
		}
	}
}

typedef struct tagItemMoveEnable
{
	char szFrom[MAX_PATH];
	int  iTTNo;
}SItemMoveEnable;

bool IsItemMoveEnable(bool const bWarn)
{
	int iWarnNo = 0;
	static const SItemMoveEnable szUI[] = {
		{"SFRM_ITEM_PLUS_UPGRADE",			1208	},
		{"SFRM_ITEM_RARITY_UPGRADE",		1482	},
		{"SFRM_REPAIR",						1223	},
		{"SFRM_SOCKET_SYSTEM",				790110	},
		{"SFRM_LUCKY_CHANGER",				50614	},
		{"FRM_ITEM_CONVERT",				790537	},
		{"SFRM_ITEM_MIX_CREATOR",			50714	},
		{"SFRM_COSTUME_MIXER",				50714	},
		{"SFRM_MONSTER_CARD_MIXER",			50714	},
		{"FRM_TRADE_UNSEALINGSCROLL",		400732	},
		{"SFRM_SOUL_OPTION_CHANGER",		5808	},
		{"SFRM_REDICE_ITEM",				3405	},
		{"SFRM_ITEM_RARITY_BUILDUP",		1550	},
		{"SFRM_ITEM_RARITY_AMPLIFY",		1551	},
		{"FRM_MISSION_SELECT_DIRECTION",	400926	},
		{"SFRM_MONSTERCARD_MIXER",			790488	},
		{"SFRM_SPECIAL_UNLOCK",				405314	},
		{"SFRM_ENCHANT_SHIFT",				1278	},
		{"SFRM_CASH_OUT_BUY",				5113	},
		{"SFRM_JL3_ITEM_CREATE",			799818	},
		{"SFRM_SOULABIL_TRANSFER",			799906	},
		{"SFRM_COLLECT_ANTIQUE",			750008	},
		{"SFRM_MONSTER_CARD",				759996	},
		{"SFRM_EXCHANGE_SOCKETCARD",		759904	},
		{"SFRM_REPAIR_USE_ITEM",			1223	},
		{"SFRM_PET_UPGRADE",			3418	},
		{"SFRM_EXCHANGE_ELUNIUM",			310003	},
		{"SFRM_EXCHANGE_PURESILVER_KEY",	310011	},
		{"MAX",								0		}
	};
	static const int iMaxSize = sizeof(szUI) / sizeof(SItemMoveEnable);

	for(int i=0;i<iMaxSize;++i)
	{
		if(XUIMgr.Get(UNI(szUI[i].szFrom)))
		{
			iWarnNo = szUI[i].iTTNo;
			break;
		}
	}

	if (0==iWarnNo && BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
	{
		iWarnNo = 400732;
	}

	if(bWarn && 0 < iWarnNo)
	{
		lwAddWarnDataTT(iWarnNo);
	}

	return 0==iWarnNo;
}

int CALLBACK ClientItemResChooser(int const iGenderLimit)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){	return 0; }

	int iGender = pPlayer->GetAbil(AT_GENDER);
	int const iClassNo = pPlayer->GetAbil(AT_CLASS);

	if(iGenderLimit == GWL_MALE)
	{
		iGender = 1;
	}
	if(iGenderLimit == GWL_FEMALE)
	{
		iGender = 2;
	}

	//남자 && (전사|궁수|도적)=> 0
	//여자 && (전사|궁수|도적)=> 1
	bool const bIsRobeUser = (UCLIMIT_MARKET_MAGICIAN & (INT64_1 << iClassNo)) || (UCLIMIT_MARKET_SHAMAN & (INT64_1 << iClassNo));
	if(1 == iGender
		&& !bIsRobeUser
		)
	{
		return 0;
	}

	if(2 == iGender
		&& !bIsRobeUser
		)
	{
		return 1;
	}

	if(1 == iGender
		&& bIsRobeUser
		)
	{
		return 2;
	}

	if(2 == iGender
		&& bIsRobeUser
		)
	{
		return 3;
	}

	//남자 && 법사	=> 3
	//여자 && 법사	=> 4
	return 0;
}
int CALLBACK ClientItemNewResChooser(int const iGenderLimit)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){	return 0; }

	int iGender = pPlayer->GetAbil(AT_GENDER);
	
	if(iGenderLimit == GWL_MALE)
	{
		iGender = 1;
	}
	if(iGenderLimit == GWL_FEMALE)
	{
		iGender = 2;

	}
	if(1 == iGender)
	{
		return 0;
	}
	if(2 == iGender)
	{
		return 1;
	}
	return 0;
}

namespace UIItemUtil
{
	bool DoQuickUseItem(int const iItemNo)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory* pkInventory = pkPlayer->GetInven();
		if( !pkInventory )
		{
			return false;
		}

		if( !IsCanUseItemGround(iItemNo) )
		{
			return false;
		}

		SItemPos kItemPos;
		HRESULT const hRet = pkInventory->GetFirstItem(iItemNo, kItemPos);//이벤에서 첫번째 아이템을 찾는다
		if( S_OK != hRet )
		{
			return false;
		}

		PgBase_Item kItem;
		if( S_OK != pkPlayer->GetInven()->GetItem(kItemPos, kItem) )
		{
			return false;
		}

		if( UseItemCustomType(iItemNo, &kItem, kItemPos) )
		{
			// General 사용 형태가 아닌 특수 사용 형태
		}
		else
		{
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
			kPacket.Push(lwGetServerElapsedTime32());
			NETWORK_SEND(kPacket)
		}
		return true;
	}

	bool SearchEqualTypeItemList(int const iTargetType, ContHaveItemNoCount& kList)
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

		return (S_OK == pkInv->GetItems(static_cast<EUseItemCustomType>(iTargetType), kList)) ? true : false;
	}

	void CallCommonUseCustomTypeItems(ContHaveItemNoCount const& kList, E_CUSTOM_ITEM_USE_TYPE const ec_Type, CONT_CUSTOM_PARAM const& kParam, CONT_CUSTOM_PARAM_STR const& kParamStr)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_USE_CUSTOM_ITEMS_SELECTER", true);
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_List* pItemList = dynamic_cast<XUI::CXUI_List*>(pMainUI->GetControl(L"LIST_CUSTOM_ITEMS"));
		if( !pItemList )
		{
			return;
		}

		XUIListUtil::SetMaxItemCount(pItemList, kList.size());
		
		XUI::SListItem* pItem = pItemList->FirstItem();
		ContHaveItemNoCount::const_iterator item_c_itor = kList.begin();
		while( kList.end() != item_c_itor )
		{
			ContHaveItemNoCount::key_type const& kKey = item_c_itor->first;
			ContHaveItemNoCount::mapped_type const& kVal = item_c_itor->second;

			if( pItem && pItem->m_pWnd )
			{
				SCustomItemUseInfo	kItemInfo;
				kItemInfo.eType = ec_Type;
				kItemInfo.iItemNo = kKey;
				kItemInfo.ContParam = kParam;
				kItemInfo.ContParamStr = kParamStr;

				BM::Stream	kPacket = kItemInfo.MakeCustomData();
				lwUIWnd(pItem->m_pWnd).SetCustomDataAsPacket(lwPacket(&kPacket));

				XUI::CXUI_Wnd* pImgWnd = pItem->m_pWnd->GetControl(L"IMG_ICON");
				if( pImgWnd )
				{
					pImgWnd->SetCustomData(&kKey, sizeof(kKey));
					pImgWnd->Text(BM::vstring(kVal).operator const std::wstring &());

					XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pImgWnd->GetControl(L"CBTN_ITEM_SELECT"));
					if( pCheckBtn )
					{
						pCheckBtn->ClickLock(false);
						pCheckBtn->Check(false);
					}
				}

				XUI::CXUI_Wnd* pNameWnd = pItem->m_pWnd->GetControl(L"SFRM_ITEM_NAME");
				if( pNameWnd )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pDef = kItemDefMgr.GetDef(kKey);
					if(pDef)
					{
						wchar_t const* pName = NULL;
						if( GetDefString(pDef->NameNo(), pName) )
						{
							pNameWnd->Text(pName);
						}
					}
				}
				pItem = pItemList->NextItem(pItem);
			}
			++item_c_itor;
		}
	}

	void OnClickCustomTypeItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pListItem = pSelf->Parent();
		if( !pListItem )
		{
			return;
		}
		BM::Stream	kPacket = (*lwUIWnd(pListItem).GetCustomDataAsPacket()());

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListItem->Parent());
		if( !pList )
		{
			return;
		}

		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem && pItem->m_pWnd )
		{
			XUI::CXUI_Wnd* pImg = pItem->m_pWnd->GetControl(L"IMG_ICON");
			if( pImg )
			{
				XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pImg->GetControl(L"CBTN_ITEM_SELECT"));
				if( pCheckBtn )
				{
					pCheckBtn->ClickLock(false);
					pCheckBtn->Check(false);
				}
			}
			pItem = pList->NextItem(pItem);
		}


		XUI::CXUI_Wnd* pMainUI = pList->Parent();
		if( pMainUI )
		{
			XUI::CXUI_Wnd* pOKBtn = pMainUI->GetControl(L"BTN_OK");
			if( pOKBtn )
			{
				lwUIWnd(pOKBtn).SetCustomDataAsPacket(lwPacket(&kPacket));
			}
		}

		XUI::CXUI_CheckButton* pCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pSelf->GetControl(L"CBTN_ITEM_SELECT"));
		if( pCheck )
		{
			pCheck->Check(true);
			pCheck->ClickLock(true);
		}
	}

	void OnDrawCustomTypeItemIcon(lwUIWnd UISelf)
	{
		PgUIUtil::DrawIconToItemNo(UISelf.GetSelf(), UISelf.GetCustomData<int>());
	}

	void OnCallCustomItemTooltip(lwUIWnd UISelf)
	{
		if( UISelf.IsNil() )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			if( pkInv )
			{
				SItemPos	kPos;
				if( S_OK == pkInv->GetFirstItem(UISelf.GetCustomData<int>(), kPos) )
				{
					PgBase_Item kItem;
					if( S_OK == pkInv->GetItem(kPos, kItem) )
					{
						CallToolTip_SItem(&kItem, (lwPoint2)(POINT2)UISelf.GetSelf()->TotalLocation());
					}
				}
			}
		}
	}

	void OnOKUseCustomTypeItem(lwUIWnd UISelf)
	{
		if( UISelf.IsNil() )
		{
			return;
		}

		SCustomItemUseInfo	kCustomUseInfo;
		if( UISelf.GetCustomDataSize() )
		{
			BM::Stream	kPacket = (*UISelf.GetCustomDataAsPacket()());
			kCustomUseInfo.ReadCustomData(kPacket);
		}

		XUI::CXUI_Wnd* pParent = UISelf.GetParent().GetSelf();
		while( pParent->Parent() )
		{
			pParent = pParent->Parent();
		}
		pParent->Close();

		if( !kCustomUseInfo.IsEmpty() )
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

			SItemPos	kCustomItemPos;
			CONT_CUSTOM_PARAM::const_iterator itor = kCustomUseInfo.ContParam.find(L"Pet_Item");	//펫 아이템은 예외처리
			if( S_OK != pkInv->GetFirstItem(kCustomUseInfo.iItemNo, kCustomItemPos, itor==kCustomUseInfo.ContParam.end()) )
			{
				lua_tinker::call<void, int, bool >("CommonMsgBoxByTextTable", 2851, true);
				return;
			}

			itor = kCustomUseInfo.ContParam.find(L"CallYesNoBox");	//YesNo 박스를 띄울건가?
			bool const bCallYesNoBox = kCustomUseInfo.ContParam.end()!=itor ? (*itor).second : true;

			std::wstring kDlgText;
			std::wstring kCancelText;
			EMsgBoxType kDlgType = MBT_COMMON_YESNO_TO_PACKET;
			BM::Stream kPacket;

			switch( kCustomUseInfo.eType )
			{
			case ECIUT_NORMAL_CONSUME:
				{
					kPacket.Push(PT_C_M_REQ_ITEM_ACTION);
					kPacket.Push(kCustomItemPos);
					kPacket.Push(lwGetServerElapsedTime32());
					NETWORK_SEND(kPacket);
				}break;
			case ECIUT_CUSTOM_DEFINED:
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pItemDef = kItemDefMgr.GetDef(kCustomUseInfo.iItemNo);
					if( !pItemDef )
					{
						return;
					}

					int iUictType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
					if(	PgEnchantShift::ES_STEP_3 == g_kEnchantShift.GetState() )
					{//인챈트 전이중이고
						if( kCustomUseInfo.iItemNo == g_kEnchantShift.GetCurrentInsurance() )
						{//인챈트 전이 인게임 보험아이템이면 추가해주자.
							iUictType = UICT_ENCHANT_INSURANCE;
						}
					}
					switch( iUictType )
					{
					case UICT_RENTALSAFE:
					case UICT_SHARE_RENTALSAFE_CASH:
					case UICT_SHARE_RENTALSAFE_GOLD:
						{
							CONT_CUSTOM_PARAM::iterator Param_itor = kCustomUseInfo.ContParam.find(L"INV_TYPE");
							if( kCustomUseInfo.ContParam.end() != Param_itor )
							{
								CONT_CUSTOM_PARAM::mapped_type const& kInvType = Param_itor->second;

								SPT_C_M_REQ_RENTALSAFE_EXTEND	Data;
								Data.kInvType = kInvType;
								Data.kItemPos = kCustomItemPos;
								Data.WriteToPacket(kPacket);
								kDlgText = TTW((iUictType == UICT_RENTALSAFE)?(2907):(2922));
							}
						}break;
					case UICT_MARKET_MODIFY:
						{
							SPT_M_C_UM_NOTI_USE_MARKET_MODIFY_ITEM	Data;
							Data.Pos(kCustomItemPos);

							CONT_CUSTOM_PARAM::iterator Param_itor = kCustomUseInfo.ContParam.find(L"SHOP_GRADE");
							if( kCustomUseInfo.ContParam.end() == Param_itor )
							{
								return;
							}
							CONT_CUSTOM_PARAM::mapped_type const& kShopGrade = Param_itor->second;

							CONT_CUSTOM_PARAM_STR::iterator ParamStr_itor = kCustomUseInfo.ContParamStr.find(L"SHOP_NAME");
							if( kCustomUseInfo.ContParamStr.end() == ParamStr_itor )
							{
								return;
							}
							CONT_CUSTOM_PARAM_STR::mapped_type const& kShopName = ParamStr_itor->second;

							Data.MarketGrade(kShopGrade);
							Data.MarketName(kShopName);

							BM::Stream	kPacket;
							Data.WriteToPacket(kPacket);
							NETWORK_SEND(kPacket);
							return;
						}break;
					case UICT_ENCHANT_INSURANCE:
						{
							CONT_CUSTOM_PARAM::iterator Param_itor = kCustomUseInfo.ContParam.find(L"CALL_UI");
							if( kCustomUseInfo.ContParam.end() != Param_itor )
							{
								CONT_CUSTOM_PARAM::mapped_type const& kCallUIType = Param_itor->second;
								switch( kCallUIType )
								{
								case EICUT_PLUS_UPGRADE:
									{
										g_kItemPlusUpgradeMgr.SetMaterialItem(ENEEDIO_INSURANCE, kCustomItemPos, true);
									}break;
								case EICUT_RARITY_UPGRADE:
									{
										g_kItemRarityUpgradeMgr.SetMaterialItem(PgItemRarityUpgradeMgr::RIT_INSUR_ITEM, kCustomItemPos, true);
									}break;
								case EICUT_ENCHANT_SHIFT:
									{//kCustomItemPos 아이템 삽입
										PgBase_Item kItem;
										if( S_OK == pkInv->GetItem(kCustomItemPos, kItem) )
										{
											bool const bRet = g_kEnchantShift.SetInsurance( g_kEnchantShift.eReadyInsertInsurance(), static_cast<int>(kItem.ItemNo()), true );
											if( bRet )
											{
												lwEnchantShift::lwSetInsurance_Display(g_kEnchantShift.eReadyInsertInsurance(), static_cast<int>(kItem.ItemNo()) );
											}
											else
											{
												lwAddWarnDataTT(1299);
											}
											g_kEnchantShift.eReadyInsertInsurance( PgEnchantShift::EINSUR_MAX );
										}
									}break;
								}
							}
							return;
						}break;
					case UICT_SOCKET_SUCCESS:
						{
							g_kSocketSystemMgr.SetMaterialItem(PgItemSocketSystemMgr::SIT_PROBABILITY, kCustomItemPos, false);
							return;
						}break;
					case UICT_MAKING_SUCCESS:
						{
							g_kEqItemMixCreator.SetProbAbility(kCustomItemPos);
						}break;
					case UICT_PLUSE_SUCCESS:
						{
							//g_kItemPlusUpgradeMgr.SetMaterialItem(PgItemPlusUpgradeMgr::RIT_PROBABILITY, kCustomItemPos, true);							
						}break;
					case UICT_RARITY_SUCCESS:
						{
							//g_kItemRarityUpgradeMgr.SetMaterialItem(PgItemRarityUpgradeMgr::RIT_PROBABILITY, kCustomItemPos, true);
							return;
						}break;
					case UICT_PET_FEED:
						{
							CONT_CUSTOM_PARAM::iterator Param_itor = kCustomUseInfo.ContParam.find(L"Index");
							if(kCustomUseInfo.ContParam.end()!=Param_itor)
							{
								int const iIndex = (*Param_itor).second;
								g_kPetUIMgr.SetFeedItem(kCustomItemPos, iIndex);
								g_kPetUIMgr.SetSrcItem(SItemPos(KUIG_FIT, EQUIP_POS_PET), iIndex);
								g_kPetUIMgr.SendReqFeedPet(true, iIndex);
							}
						}break;
					case UICT_EXTEND_MAX_IDX:
						{
							if( PgItemUseChecker::CommonExtendInvTypeCheck(iUictType, 0, kCustomItemPos) )
							{
								kPacket.Push(PT_C_M_REQ_ITEM_ACTION);
								kPacket.Push(kCustomItemPos);
								kPacket.Push(lwGetServerElapsedTime32());
								kDlgText = TTW(21);
							}
						}break;
					case UICT_SAFE_EXTEND_MAX_IDX:
						{
							CONT_CUSTOM_PARAM::iterator Param_itor = kCustomUseInfo.ContParam.find(L"INV_TYPE");
							if( kCustomUseInfo.ContParam.end() != Param_itor )
							{
								CONT_CUSTOM_PARAM::mapped_type const& kInvType = Param_itor->second;

								if( PgItemUseChecker::CommonExtendInvTypeCheck(iUictType, kInvType, kCustomItemPos) )
								{//사용할 아이템을 결정했다.
									kPacket.Push(PT_C_M_REQ_INVENTORY_EXTENDIDX);
									kPacket.Push(kCustomItemPos);
									kPacket.Push(kInvType);
									kDlgText = TTW(21);
								}
							}
						}break;
					default:
						{

						}break;
					}
				}
			}
			if(bCallYesNoBox)
			{
				lwCallCommonMsgYesNoBox(MB(kDlgText), lwPacket(&kPacket), true, kDlgType, (kCancelText.empty())?(NULL):(MB(kCancelText)));
			}
		}
	}

	bool GetUICTInfo(int const iItemNo, SUICT_Info& kResult)
	{//아이템 번호로 UICT 타입을 얻어온다(User Item Custom Type)
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);
		if( !pItemDef )
		{
			return false;
		}
		kResult.eType = static_cast<EUseItemCustomType>(pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE));
		kResult.iCustomVal1 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
		kResult.iCustomVal2 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
		kResult.iCustomVal3 = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);
		return true;
	}
}

void UseJobSkill(int const iJobSkillNo)
{
	EJobSkillType const eSkillType = JobSkill_Util::GetJobSkillType(iJobSkillNo);
	if( eSkillType == JST_1ST_SUB
		|| eSkillType == JST_2ND_SUB )
	{//패시브 스킬은 사용 할수 없다.
		return;
	}
	PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
	if(pkMyActor)
	{// 퀵슬롯에서 사용된 직업스킬에 토글 표시를 해주기위해6
		PgAction* pkAction = pkMyActor->GetAction();
		if(!pkAction)
		{
			return;
		}
		BM::vstring kCurActionName(pkAction->GetID());
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iJobSkillNo);
		if(!pkSkillDef)
		{
			return;
		}
		BM::vstring kNewActionName(pkSkillDef->GetActionName());
		if(		(kNewActionName == kCurActionName)
			|| (0 < pkAction->GetAbil(AT_JOBSKILL_TOOL_TYPE) && 0 < pkSkillDef->GetAbil(AT_JOBSKILL_TOOL_TYPE))
			)
		{// 이미 스킬을 사용중인 상태에서 또 사용하는것이라면, 스킬을 종료 요청을 서버로 보내고
			lwSetJobSkillTogleState(iJobSkillNo, false);
			lua_tinker::call<void, lwActor>("JobSkill_Net_ReqCancelJobSkill", lwActor(pkMyActor));
			return;
		}
		else
		{
			// 스킬을 사용하는 것이라면
			bool bShowToggleIcon = false;
			PgTrigger* pkTrigger = pkMyActor->GetCurrentTrigger(); // NULL or *
			if(!pkTrigger)
			{// 트리가 존재 하는지 확인하고
				::Notice_Show(TTW(25014), EL_Warning, true);
			}
			else
			{// 있다면
				PgTrigger::TriggerType kType = pkTrigger->GetTriggerType();
				if( PgTrigger::TRIGGER_TYPE_JOB_SKILL == kType )
				{// 현재 채집 트리거의 타입과 
					EGatherType eTriggerGatherType = EGatherType::GT_None;
					g_kJobSkillLocationInfo.GetGatherType(pkTrigger->GetID(), eTriggerGatherType);
					bool bCheckSuccess = false;
					PgItemEx* pkWeaponItem = pkMyActor->GetEquippedWeapon();
					if(pkWeaponItem)
					{// 장착 하고 있는 무기의 타입 
						if(pkSkillDef)
						{// 그리고 사용하는 스킬의 타입이 모두 일치 한다면
							int const iJobToolType = pkWeaponItem->GetItemAbil(AT_JOBSKILL_TOOL_TYPE);
							int const iJobSkillToolType = pkSkillDef->GetAbil(AT_JOBSKILL_TOOL_TYPE);
							if( (GT_ToolBox == iJobSkillToolType)
								&& (GT_ToolBox == iJobToolType) )
							{//만능도구는 트리거 액션으로 전이
								pkTrigger->OnAction( pkMyActor );
								return;
							}
							else if(  (iJobSkillToolType == static_cast<int>(eTriggerGatherType))
								&& (iJobToolType == static_cast<int>(eTriggerGatherType) )
								) 
							{// 스킬을 사용할수 있으므로
								bCheckSuccess = true;
							}
							else if( (static_cast<int>(GT_Smelting) == eTriggerGatherType || static_cast<int>(GT_Jewelry) == eTriggerGatherType)
								&& (static_cast<int>(GT_Smelting) == iJobSkillToolType || GT_Jewelry == iJobSkillToolType )
								&& (static_cast<int>(GT_Smelting) == iJobToolType || static_cast<int>(GT_Jewelry) == iJobToolType)
								) 
							{// 예외적으로 채광은 보석류의 트리거와 같으니 모두 같지 않더라도 이것은 예외로 하고
								bCheckSuccess = true;
							}
						}
					}
					if(bCheckSuccess)
					{// 스킬이 성공 했으니, 토글을 활성화 할수 있게 해주고
						bShowToggleIcon = true;
					}
					else
					{// 스킬이 실패 했다면, 모든 퀵슬롯의 채집 스킬의 토글 상태를 끄고
						lwTogleOffAllJobSkillQuicIcon();
					}
					// 성공 했다면, 이 스킬 사용 요청은 스킬을 발동 시킬것이고, 실패 했다면, 서버처리를 포함해 스킬 사용을 중지 시킬것이고
					lua_tinker::call<void>("CloseJobSkillGatherTimer");
					lua_tinker::call<bool, lwTrigger, lwActor, int, bool>("World_JobSkill", lwTrigger(pkTrigger), lwActor(pkMyActor), iJobSkillNo , true);
				}
				else
				{//채집장소 아님 에러 메세지 출력
					::Notice_Show(TTW(25014), EL_Warning, true);
				}
			}
			if(!bShowToggleIcon)
			{// 토글을 꺼야 하는 경우에만 실행 시켜준다
				lwSetJobSkillTogleState(iJobSkillNo, false);
			}
		}
	}
}

void UseItemSkill(int const iItemSkillNo)
{
	PgPilot *pkPilot =g_kPilotMan.GetPlayerPilot();
	if (pkPilot)
	{
		PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if(pkActor)
		{
			pkActor->ReserveTransitAction(iItemSkillNo);
		}
	}
}

void GetEqiupPosString(DWORD const dwEquipLimit, bool const bIsPet, std::wstring& wstrText)
{
	switch(dwEquipLimit & 0xFFFFFFFF)
	{// EQUIP_LIMIT_HAIR 이걸로 찾아봐라.
	case EQUIP_LIMIT_HAIR:		{wstrText += TTW(70001);}break;//머리카락
	case EQUIP_LIMIT_FACE:		{wstrText += TTW(70002);}break;//얼굴
	case EQUIP_LIMIT_SHOULDER:	{wstrText += TTW(70003);}break;//어깨
	case EQUIP_LIMIT_CLOAK:		{wstrText += TTW(70004);}break;//망토
	case EQUIP_LIMIT_GLASS:		{wstrText += TTW(70005);}break;//안경
	case EQUIP_LIMIT_WEAPON:	{wstrText += TTW(1852);} break;//무기
	case EQUIP_LIMIT_SHEILD:	{wstrText += TTW(70007);}break;//방패
	case EQUIP_LIMIT_NECKLACE:	{wstrText += TTW(70008);}break;//목걸이
	case EQUIP_LIMIT_EARRING:	{wstrText += TTW(70009);}break;//귀걸이
	case EQUIP_LIMIT_RING:		{wstrText += TTW(70010);}break;//반지
	case EQUIP_LIMIT_BELT:		{wstrText += TTW(70011);}break;//벨트
	case EQUIP_LIMIT_ATTSTONE:	{wstrText += TTW(70012);}break;//속성석
	case EQUIP_LIMIT_MEDAL:		{wstrText += TTW(399906);}break;//기타
	case EQUIP_LIMIT_HELMET:	{wstrText += TTW(bIsPet?50003:70020);}break;//투구//펫 악세사리
	case EQUIP_LIMIT_SHIRTS:	{wstrText += TTW(70021);}break;//상의 
	case EQUIP_LIMIT_PANTS:		{wstrText += TTW(70022);}break;//하의 
	case EQUIP_LIMIT_BOOTS:		{wstrText += TTW(bIsPet?126:70023);}break;//부츠//펫모자
	case EQUIP_LIMIT_GLOVE:		{wstrText += TTW(bIsPet?70021:70024);}break;//장갑 //펫 상의
	case EQUIP_LIMIT_ARM:		{wstrText += TTW(bIsPet?129:70013);}break;//팔//펫장비
	case EQUIP_LIMIT_KICKBALL:	{wstrText += TTW(70007);}break;//킥볼(인데 툴팁엔 방패 -_-)
	}
}

void GetEqiupPosStringMonsterCard(DWORD const dwEquipLimit, std::wstring& wstrText)
{
	bool bFirst = true;

	if( (dwEquipLimit & EQUIP_LIMIT_HAIR) == EQUIP_LIMIT_HAIR ) {wstrText += TTW(70001); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_FACE) == EQUIP_LIMIT_FACE )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70002); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_SHOULDER) == EQUIP_LIMIT_SHOULDER )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70003); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_CLOAK) == EQUIP_LIMIT_CLOAK ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70004); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_GLASS) == EQUIP_LIMIT_GLASS ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70005); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_WEAPON) == EQUIP_LIMIT_WEAPON ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(1852); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_SHEILD) == EQUIP_LIMIT_SHEILD ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70007); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_NECKLACE) == EQUIP_LIMIT_NECKLACE ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70008); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_EARRING) == EQUIP_LIMIT_EARRING ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70009); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_RING) == EQUIP_LIMIT_RING ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70010); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_BELT) == EQUIP_LIMIT_BELT )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70011); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_ATTSTONE) == EQUIP_LIMIT_ATTSTONE )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70012); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_MEDAL) == EQUIP_LIMIT_MEDAL )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(399906); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_HELMET) == EQUIP_LIMIT_HELMET )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70020); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_SHIRTS) == EQUIP_LIMIT_SHIRTS )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70021); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_PANTS) == EQUIP_LIMIT_PANTS )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70022); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_BOOTS) == EQUIP_LIMIT_BOOTS )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70023); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_GLOVE) == EQUIP_LIMIT_GLOVE )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70024); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_ARM) == EQUIP_LIMIT_ARM )	{if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70013); bFirst=false;}
	if( (dwEquipLimit & EQUIP_LIMIT_KICKBALL) == EQUIP_LIMIT_KICKBALL ) {if(!bFirst) {wstrText+=L"/";} wstrText += TTW(70026); bFirst=false;}
}

bool TryReserveActionToMyActor(int const iSkillNo)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(pkSkillDef)
	{
		PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
		if (pkPilot)
		{
			PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(pkActor)
			{
				if( 0 == pkSkillDef->GetAbil(AT_SUB_PLAYER_ACTION) )
				{
					PgAction * pkAction = pkActor->GetAction();
					if( pkAction )
					{// 콤보 키 입력 가능 시간 중 연결 가능
						char const *pParam = pkAction->GetParam(40001);
						if( pParam && 0 != atoi(pParam) )
						{
							static char szBuff[32];
							NiSprintf(szBuff, 31, "%d", iSkillNo);
							pkAction->SetParam(40007, szBuff);
							return true;
						}
					}
				}
				std::wstring const &kActionScriptName = kSkillDefMgr.GetActionName(iSkillNo);
				if( 0 < pkSkillDef->GetAbil(AT_SUB_PLAYER_ACTION) )
				{// SubPlayer에 시켜야 하는것이고
					if( 0 < pkPilot->GetAbil(AT_DEL_SUB_PLAYER)			// 퓨전 상태인데다
						&& 0 < pkPilot->GetAbil(AT_SUB_PLAYER_ACTION)	// 보조캐릭터 액션을 자기가 하는 어빌값이 있으면
						)
					{// 자신이 액션을 하고
						pkActor->ReserveTransitAction(MB(kActionScriptName));
					}
					else
					{// 아니라면
						PgAction* pkAction = pkActor->GetAction();
						if(pkAction
							&& true == lwCommonSkillUtilFunc::IsBanSubPlayerAction( pkAction ) 
							)
						{// SC 캐릭터 스킬을 사용자 입력으로 사용하려 할때 가능한지 확인하고
							lwAddWarnDataStr(lwWString(TTW(799960)),2, true);
							return false;
						}
						lwActor kActor(pkActor);
						PgActor* pkSubActor = kActor.GetSubPlayer()();
						if(pkSubActor)
						{// 보조캐릭터에게 액션을 시킨다
							pkSubActor->ReserveTransitAction(MB(kActionScriptName));
						}
					}
				}
				else
				{
					pkActor->ReserveTransitAction(MB(kActionScriptName));
				}
				return true;
			}
		}
	}
	return false;
}

bool SetUIAddResToImage(XUI::CXUI_Image* pItemImg, int iItemNo)
{
	if(!pItemImg || iItemNo == 0)
	{
		return false;
	}

	pItemImg->ReleaseOverlayImg();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	PG_ASSERT_LOG(pkItemDef);
	if(!pkItemDef)
	{
		return false;
	}

	CONT_DEFRES const* pkContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pkContDefRes);
	if(!pkContDefRes)
	{
		return false;
	}

	CONT_DEFRES::const_iterator iterDefRes = pkContDefRes->find(TBL_KEY_INT(pkItemDef->ResNo()));
	if(iterDefRes != pkContDefRes->end())
	{
		for(int i = 0; i < 3; i++)
		{
			int iAddResNo = iterDefRes->second.AddResNo[i];
			if(iAddResNo == 0)
			{
				continue;
			}
			CONT_DEFRES::const_iterator iterAddRes = pkContDefRes->find(TBL_KEY_INT( iAddResNo ));
			if(iterAddRes != pkContDefRes->end())
			{
				NiSourceTexture const* tex = g_kNifMan.GetTexture(MB(iterAddRes->second.strIconPath));
				if (tex)
				{
					std::wstring wsTemp = iterAddRes->second.strIconPath;
					pItemImg->AddOverlayImg(wsTemp, POINT2(tex->GetWidth(), tex->GetHeight()),
						iterAddRes->second.U, iterAddRes->second.V, iterAddRes->second.UVIndex);
				}
			}
		}

	}

	return true;
}