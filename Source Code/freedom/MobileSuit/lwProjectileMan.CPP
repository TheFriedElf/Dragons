#include "stdafx.h"
#include "lwProjectileMan.h"
#include "PgProjectileMan.h"
#include "lwProjectile.h"
#include "lwAction.H"
#include "PgPilot.H"
#include "lwPilot.H"
#include "lwGUID.H"

lwProjectileMan	GetProjectileMan()
{
	return	lwProjectileMan();
}

lwProjectileMan::lwProjectileMan()
{
}

//! 스크립팅 시스템에 등록한다.
bool lwProjectileMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "GetProjectileMan", &GetProjectileMan);

	class_<lwProjectileMan>(pkState, "ProjectileMan")
		.def(pkState, constructor<void>())
		.def(pkState, "CreateNewProjectile", &lwProjectileMan::CreateNewProjectile)
		.def(pkState, "DeleteProjectile", &lwProjectileMan::DeleteProjectile)
		.def(pkState, "DeleteProjectileByUID", &lwProjectileMan::DeleteProjectileByUID)
		.def(pkState, "GetProjectile", &lwProjectileMan::GetProjectile)
		;

	return true;
}
lwProjectile	lwProjectileMan::GetProjectile(lwGUID kPilotGUID,int iActionInstanceID,int iUID)
{
	return	lwProjectile(g_kProjectileMan.GetProjectile(kPilotGUID(),iActionInstanceID,iUID));
}	

lwProjectile	lwProjectileMan::CreateNewProjectile(char const* kProjectileID,lwAction kParentAction,lwGUID kParentPilotGuid)
{
	return	lwProjectile(g_kProjectileMan.CreateNewProjectile(kProjectileID,kParentAction(),kParentPilotGuid()));
}

void	lwProjectileMan::DeleteProjectile(lwProjectile kProjectile)
{
	g_kProjectileMan.DeleteProjectile(kProjectile());
}
void	lwProjectileMan::DeleteProjectileByUID(lwGUID kPilotGUID,int iActionInstanceID,int iUID)
{
	g_kProjectileMan.DeleteProjectile(kPilotGUID(),iActionInstanceID,iUID);
}
