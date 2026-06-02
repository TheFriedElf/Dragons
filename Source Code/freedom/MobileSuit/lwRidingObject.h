#pragma once

#include "PgScripting.h"
#include "PgRidingObject.H"
#include "lwGUID.h"
#include "lwActor.H"

LW_CLASS(PgRidingObject, RidingObject)

	lwPoint3	GetAttactedObjectPos();
	void	SetAttachedObject(char const *objname);

LW_CLASS_END;
