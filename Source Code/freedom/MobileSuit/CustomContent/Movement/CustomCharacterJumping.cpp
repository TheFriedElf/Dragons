#include "stdafx.h"
#include "CustomCharacterJumping.h"
#include "lwActor.h"
#include "lwAction.h"
#include "PgAction.h"

extern bool lwKeyIsDown(int iKeyNum, bool bIsNotUKey);

bool CustomCharacterJumping::ShouldPrioritizeJumpOnLanding(lwActor actor, lwAction action)
{
	if (actor.IsNil() || action.IsNil())
	{
		return false;
	}

	return actor.IsMyActor()
		&& lwKeyIsDown(ACTIONKEY_JUMP, false)
		&& strcmp(action.GetParam(1), "jump_again") != 0;
}

bool CustomCharacterJumping::ShouldTreatMovingObjectUpAsFloor(float fMoveDeltaZ, bool bIsJumping)
{
	return fMoveDeltaZ > 0.0f && !bIsJumping;
}