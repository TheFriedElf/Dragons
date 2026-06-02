#include "StdAfx.h"
#include "lwElementDungeonUI.h"
#include "Variant/PgPlayer.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgNetwork.h"

int const MIN_LIMIT_LEVEL_ENTER_LV1 = 40;
int const MAX_LIMIT_LEVEL_ENTER_LV1 = 49;
int const MIN_LIMIT_LEVEL_ENTER_LV2 = 50;
int const MAX_LIMIT_LEVEL_ENTER_LV2 = 59;
int const MIN_LIMIT_LEVEL_ENTER_LV3 = 60;
int const MAX_LIMIT_LEVEL_ENTER_LV3 = 99;

bool lwElementDungeonUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "CallElementDungeonUI", lwCallElementDungeonUI);
	def(pkState, "EnterElementDungeon", lwEnterElementDungeon);
	def(pkState, "CheckElementDungeon", lwCheckElementDungeon);
	def(pkState, "GetEnterLevel", lwGetEnterLevel);
	return true;
}

void lwElementDungeonUI::lwCallElementDungeonUI()
{
	bool bAbleEnter = false;
	PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer->HaveParty() )
	{
		BM::GUID kPartyGuid = pPlayer->PartyGuid();
		if( BM::GUID::NullData() == kPartyGuid )
		{
			return;
		}
		ContPartyMember kCont;
		PgActor* pActor = g_kPilotMan.FindActor( g_kParty.MasterGuid() );
		if(!pActor)
		{//파티는 있는데 파티장이 없다는게 말이 되냐;
			return;
		}
		if( pActor->GetGuid() != pPlayer->GetID() )
		{//파티장만 사용할 수 있는 메뉴 입니다.
			lwAddWarnDataTT(560051);
			return;
		}
		CUnit* pPartyMaster = pActor->GetUnit();
		if( !pPartyMaster )
		{
			return;
		}
		int const iLevel = pPartyMaster->GetAbil(AT_LEVEL);
		bAbleEnter = lwGetEnterLevel(iLevel);
		if( false == bAbleEnter )
		{//"현재 레벨에 입장 가능한 던전이 없습니다."
			lwAddWarnDataTT(799937);
			return;
		}
	}
	else
	{
		int const iLevel = pPlayer->GetAbil(AT_LEVEL);
		bAbleEnter = lwGetEnterLevel(iLevel);
		if( false == bAbleEnter )
		{//"현재 레벨에 입장 가능한 던전이 없습니다."
			lwAddWarnDataTT(799937);
			return;
		}
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(L"FRM_ENTER_ELEMENT_DUNGEON");
}

void lwElementDungeonUI::lwEnterElementDungeon(int const iSuperDungeonNo)
{
	if( !lwCheckElementDungeon() )
	{
		return;
	}

	int bAbleEnter = 0;
	PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer->HaveParty() )
	{
		BM::GUID kPartyGuid = pPlayer->PartyGuid();
		if( BM::GUID::NullData() == kPartyGuid )
		{
			return;
		}
		ContPartyMember kCont;
		PgActor* pActor = g_kPilotMan.FindActor( g_kParty.MasterGuid() );
		if(!pActor)
		{//파티는 있는데 파티장이 없다는게 말이 되냐;
			return;
		}
		if( pActor->GetGuid() != pPlayer->GetID() )
		{//파티장만 사용할 수 있는 메뉴 입니다.
			lwAddWarnDataTT(560051);
			return;
		}
		CUnit* pPartyMaster = pActor->GetUnit();
		if( !pPartyMaster )
		{
			return;
		}
		int const iLevel = pPartyMaster->GetAbil(AT_LEVEL);
		bAbleEnter = lwGetEnterLevel(iLevel);
		if( false == bAbleEnter )
		{//"현재 레벨에 입장 가능한 던전이 없습니다."
			lwAddWarnDataTT(799938);
			return;
		}
	}
	else
	{
		int const iLevel = pPlayer->GetAbil(AT_LEVEL);
		bAbleEnter = lwGetEnterLevel(iLevel);
		if( false == bAbleEnter )
		{//"현재 레벨에 입장 가능한 던전이 없습니다."
			lwAddWarnDataTT(799937);
			return;
		}
	}

	PgInventory * pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return;
	}

	// 입장 아이템 체크
	ContHaveItemNoCount ContHaveItemNoCount;
	if( S_OK == pInv->GetItems(UICT_ELEMENT_ENTER_CONSUME, ContHaveItemNoCount ) )
	{	// 한 종류의 입장아이템으로 여러개의 던전 입장검사해야 되기 때문에, 입장 아이템 보유여부만 검사한다.
		if( true == Find_ElementCousumItem(ContHaveItemNoCount) )
		{
			BM::Stream kPacket(PT_C_M_REQ_ELEMENT_GROUND_MOVE);
			kPacket.Push(iSuperDungeonNo);
			kPacket.Push(0);

			NETWORK_SEND(kPacket);
		}
	}
	else
	{
		lwAddWarnDataTT(799940);
		return;
	}
}

bool lwElementDungeonUI::lwCheckElementDungeon()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	if( pkPlayer->HaveParty() )
	{
		BM::GUID kPartyGuid = pkPlayer->PartyGuid();
		if( BM::GUID::NullData() == kPartyGuid )
		{
			return false;
		}
		ContPartyMember kCont;
		g_kParty.GetPartyMemberList(kCont);
		ContPartyMember::iterator itor = kCont.begin();
		while( kCont.end() != itor)
		{
			bool const bMemberAbleEnter = lwGetEnterLevel( static_cast<int>( (*itor)->sLevel ) );
			if( false == bMemberAbleEnter)
			{//
				lwAddWarnDataTT(799938);
				return false;
			}
			++itor;
		}
	}
	else
	{
		PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
		int const iLevel = pPlayer->GetAbil(AT_LEVEL);
		bool const bAbleEnter = lwGetEnterLevel(iLevel);
		if( false == bAbleEnter )
		{//레벨에 적합한 던전이 없습니다.
			return false;
		}
	}
	return true;
}

bool lwElementDungeonUI::lwGetEnterLevel(int const iPlayerLevel)
{
	return (iPlayerLevel >= MIN_LIMIT_LEVEL_ENTER_LV1) ? true : false;
}

bool lwElementDungeonUI::Find_ElementCousumItem(ContHaveItemNoCount & ContItemNoCount)
{
	GET_DEF(CItemDefMgr, ItemDefMgr);

	ContHaveItemNoCount::const_iterator itor = ContItemNoCount.begin();
	while( ContItemNoCount.end() != itor )
	{
		int const iItemNo = itor->first;
		CItemDef const * pItemDef = ItemDefMgr.GetDef(iItemNo);
		if( pItemDef )
		{
			if( pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1) )
			{	//찾았다
				return true;
			}
		}
		++itor;
	}

	return false;
}