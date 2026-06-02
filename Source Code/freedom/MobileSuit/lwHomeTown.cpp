#include "stdafx.h"
#include "PgHomeTown.h"
#include "lwHomeTown.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgPuppet.h"
#include "PgPilotMan.h"
#include "PgHouse.h"
#include "PgPilot.h"

void lwHomeTown::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	//def(pkState, "HomeTownOpenUI", lwHomeTown::lwHomeTownOpenUI);
	def(pkState, "OnClickHomeTown", lwHomeTown::lwOnClickHomeTown);
	def(pkState, "GetHomeTownCount", lwHomeTown::lwGetHomeTownCount);
	def(pkState, "GetHomeTownNoAt", lwHomeTown::lwGetHomeTownNoAt);
	def(pkState, "OnClickNamePlate", lwHomeTown::lwOnClickNamePlate);
	def(pkState, "OnClickRemoveFurniture", lwHomeTown::OnClickRemoveFurniture);
	def(pkState, "OnClickMoveFurniture", lwHomeTown::OnClickMoveFurniture);
}

//void lwHomeTown::lwHomeTownOpenUI(lwTrigger kTrigger, lwActor kActor)
//{
//}

void lwHomeTown::lwOnClickHomeTown(int const iIndex)
{
	int const iMapNo = lwGetHomeTownNoAt(iIndex);
	if(iMapNo)
	{
		BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_ENTER);
		kPacket.Push(iMapNo);
		kPacket.Push(1);//임시. 포탈번호는 무조건 1
		NETWORK_SEND(kPacket);
	}
}

int lwHomeTown::lwGetHomeTownCount()
{
	return g_kHomeTownMgr.GetHomeTownCount();
}

int lwHomeTown::lwGetHomeTownNoAt(int const iAt)
{
	if(0>iAt)
	{
		return 0;
	}

	return g_kHomeTownMgr.GetHomeTownMapNoAt(static_cast<size_t>(iAt));
}

lwGUID lwHomeTown::lwOnClickNamePlate(PgIWorldObject *pkObject)
{
	PgPuppet* pkPuppet = dynamic_cast<PgPuppet*>(pkObject);
	if(pkPuppet)
	{
		NiStringExtraData* pkGuidStr = NiDynamicCast(NiStringExtraData, pkPuppet->GetExtraData("CALLER_GUID"));
		if(pkGuidStr)
		{
			return lwGUID(pkGuidStr->GetValue());
		}
	}

	return lwGUID("");
}

void lwHomeTown::OnClickRemoveFurniture()
{
	if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
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
					pkHouse->RemovePickedFurniture();
				}
			}
		}
	}
}

void lwHomeTown::OnClickMoveFurniture()
{
	if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
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
					pkHouse->SetArrangeMode(true);
					pkHouse->SelectArrangingFurnitureByMenu();
				}
			}
		}
	}
}
