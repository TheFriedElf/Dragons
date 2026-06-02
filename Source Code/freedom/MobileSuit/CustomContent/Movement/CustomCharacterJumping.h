#pragma once

class lwActor;
class lwAction;

class CustomCharacterJumping
{
public:
	static bool ShouldPrioritizeJumpOnLanding(lwActor actor, lwAction action);
	static bool ShouldTreatMovingObjectUpAsFloor(float fMoveDeltaZ, bool bIsJumping);
};