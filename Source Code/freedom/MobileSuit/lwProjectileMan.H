#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILEMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILEMAN_H

class	PgProjectileMan;
class	lwProjectile;
class	lwAction;
class	lwPilot;
class	lwGUID;
class	lwProjectileMan
{
public:

	lwProjectileMan();

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	lwProjectile	CreateNewProjectile(char const* kProjectileID,lwAction kParentAction,lwGUID kParentPilotGuid);
	lwProjectile	GetProjectile(lwGUID kPilotGUID,int iActionInstanceID,int iUID);
	void	DeleteProjectile(lwProjectile kProjectile);
	void	DeleteProjectileByUID(lwGUID kPilotGUID,int iActionInstanceID,int iUID);

};

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILEMAN_H