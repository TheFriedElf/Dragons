#pragma once

#include "PgScripting.h"
#include "PgBreakableObject.H"
#include "lwGUID.h"
#include "lwActor.H"

LW_CLASS(PgBreakableObject, BreakableObject)
	lwActor GetActor();
	lwGUID	GetParentGroupGUID();
	int	GetVerticalLocation();
	void	Break();
LW_CLASS_END;
