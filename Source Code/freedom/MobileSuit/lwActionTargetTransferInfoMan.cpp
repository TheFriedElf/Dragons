#include "stdafx.h"
#include "lwActionTargetTransferInfoMan.h"
#include "lwGUID.H"
#include "lwPoint3.H"
#include "lwActionResult.H"
#include "PgActionTargetList.H"

using namespace lua_tinker;

lwActionTargetTransferInfoMan	lwGetActionTargetTransferInfoMan()
{
	return	lwActionTargetTransferInfoMan(&g_kActionTargetTransferInfoMan);
}

void lwActionTargetTransferInfoMan::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "GetActionTargetTransferInfoMan", &lwGetActionTargetTransferInfoMan);

	LW_REG_CLASS(ActionTargetTransferInfoMan)
		LW_REG_METHOD(ActionTargetTransferInfoMan, AddTransferInfo_Actor)
		LW_REG_METHOD(ActionTargetTransferInfoMan, AddTransferInfo_Projectile)
		LW_REG_METHOD(ActionTargetTransferInfoMan, DeleteTransferInfo_Actor)
		LW_REG_METHOD(ActionTargetTransferInfoMan, DeleteTransferInfo_Projectile)
		;
}

void	lwActionTargetTransferInfoMan::AddTransferInfo_Actor(lwGUID kOriginalActor,
	int iOriginalActionNo,int iOriginalActionInstanceID,
	lwGUID kTransferredActorGUID)
{
	g_kActionTargetTransferInfoMan.AddTransferInfo(
		kOriginalActor(),
		iOriginalActionNo,
		iOriginalActionInstanceID,
		kTransferredActorGUID());

}

void	lwActionTargetTransferInfoMan::AddTransferInfo_Projectile(lwGUID kOriginalActor,
	int iOriginalActionNo,int iOriginalActionInstanceID,
	int iProjectileUID)
{
	g_kActionTargetTransferInfoMan.AddTransferInfo(
		kOriginalActor(),
		iOriginalActionNo,
		iOriginalActionInstanceID,
		iProjectileUID);
}

void	lwActionTargetTransferInfoMan::DeleteTransferInfo_Actor(lwGUID kOriginalActor,
	int iOriginalActionNo,int iOriginalActionInstanceID,
	lwGUID kTransferredActorGUID)
{
	g_kActionTargetTransferInfoMan.DeleteTransferInfo(
		kOriginalActor(),
		iOriginalActionNo,
		iOriginalActionInstanceID,
		kTransferredActorGUID());
}

void	lwActionTargetTransferInfoMan::DeleteTransferInfo_Projectile(lwGUID kOriginalActor,
	int iOriginalActionNo,int iOriginalActionInstanceID,
	int iProjectileUID)
{
	g_kActionTargetTransferInfoMan.DeleteTransferInfo(
		kOriginalActor(),
		iOriginalActionNo,
		iOriginalActionInstanceID,
		iProjectileUID);
}