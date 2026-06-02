#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETINFO_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETINFO_H

#include "PgScripting.h"

class	lwGUID;
class	lwPoint3;
class	lwActionResult;

LW_CLASS(PgActionTargetInfo, ActionTargetInfo)

	lwGUID	GetTargetGUID();
	int	GetABVIndex();
	int	GetRemainHP();
	lwActionResult	GetActionResult();
	void	SetActionResult(lwActionResult kActionResult);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETINFO_H