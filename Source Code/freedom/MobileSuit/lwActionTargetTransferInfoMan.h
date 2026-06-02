#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETTRASFERINFOMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETTRASFERINFOMAN_H

#include "PgScripting.h"

class	lwGUID;

LW_CLASS(PgActionTargetTransferInfoMan, ActionTargetTransferInfoMan)

	void	AddTransferInfo_Actor(lwGUID kOriginalActor,
		int iOriginalActionNo,int iOriginalActionInstanceID,
		lwGUID kTransferredActorGUID);

	void	AddTransferInfo_Projectile(lwGUID kOriginalActor,
		int iOriginalActionNo,int iOriginalActionInstanceID,
		int iProjectileUID);

	void	DeleteTransferInfo_Actor(lwGUID kOriginalActor,
		int iOriginalActionNo,int iOriginalActionInstanceID,
		lwGUID kTransferredActorGUID);

	void	DeleteTransferInfo_Projectile(lwGUID kOriginalActor,
		int iOriginalActionNo,int iOriginalActionInstanceID,
		int iProjectileUID);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETTRASFERINFOMAN_H