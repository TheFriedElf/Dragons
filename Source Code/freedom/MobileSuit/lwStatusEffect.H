#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSTATUSEFFECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSTATUSEFFECT_H
#include "PgScripting.h"
#include "PgStatusEffect.h"
#include "lwGUID.h"

LW_CLASS(PgStatusEffectMan, StatusEffectMan)

	void	AddStatusEffect(lwGUID	kPilotGUID,lwGUID	kCasterPilotGUID,int iActionInstanceID, int iEffectID,int iValue);
	void	RemoveStatusEffect(lwGUID	kPilotGUID,int iEffectID);
	void	RemoveAllStatusEffect(lwGUID kPilotGUID);

	int	AddStatusEffectToActor(lwGUID	kPilotGUID,char *strEffectXMLID,int iEffectID,int iEFfectKey,int iValue);
	void	RemoveStatusEffectFromActor(lwGUID	kPilotGUID,int iEffectID);
	void	RemoveStatusEffectFromActor2(lwGUID	kPilotGUID,int iInstanceID);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSTATUSEFFECT_H