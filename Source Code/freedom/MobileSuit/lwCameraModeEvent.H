#ifndef FREEDOM_DRAGONICA_SCRIPTING_CAMERA_LWCAMERAMODEEVENT_H
#define FREEDOM_DRAGONICA_SCRIPTING_CAMERA_LWCAMERAMODEEVENT_H
#include "PgScripting.h"
#include "PgCameraModeEvent.H"

class	lwGUID;
class	lwPoint3;

LW_CLASS(PgCameraModeEvent, CameraModeEvent)

	void	StartMoveCamera(float fMoveTime,float fCamDistance,float fCamMoveUpDistance,float fHorizRotAngle,float fVertRotAngle);
	
	void	SetTargetActor(lwGUID kTargetActor);
	void	SetTargetPos(lwPoint3 kTargetPos);

	int		GetState();

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_CAMERA_LWCAMERAMODEEVENT_H