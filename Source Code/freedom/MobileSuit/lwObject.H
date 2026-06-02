#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWOBJECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWOBJECT_H

#include "PgScripting.h"
#include "PgObject.H"
#include "lwGUID.h"
#include "lwActor.H"

LW_CLASS(PgObject, Object)
	lwActor GetActor();

	//	For Ridable
	lwPoint3	GetAttactedObjectPos();
	void	SetAttachedObject(char const *objname);

	lwPoint3	GetRidingObjectPosition(float fElapsedTime);

	//	For Breakable
	lwGUID	GetParentGroupGUID();
	int	GetVerticalLocation();
	void	Break();

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWOBJECT_H