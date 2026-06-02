#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWEFFECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWEFFECT_H

#include "PgScripting.h"
#include "variant/Effect.h"
#include "lwGUID.h"

LW_CLASS(CEffect, Effect)
	int GetValue();
	void SetValue(int iValue);
	void SetCaster(lwGUID kGuid);
	lwGUID GetCaster();
	int GetAbil(int iAbil);
	int GetEffectNo();
	DWORD GetEndTime()const;
	int GetDurationTime()const;
LW_CLASS_END;

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWEFFECT_H