#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEBOX_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEBOX_H

#include "PgICameraMode.h"

class PgInput;

class PgCameraModeBox : public PgICameraMode
{
public:
	PgCameraModeBox(NiCamera *pkCamera, PgActor *pkActor);
	virtual ~PgCameraModeBox();
	virtual void SetActor(PgActor *pkActor);
	virtual bool Update(float fFrameTime);
	NiPoint3 GetActorPosition();
	void SetCameraDistance(float fDistance) { m_fDistanceFromActor = fDistance; }

protected:
	bool UpdateTranslate(float fTime);

	float m_fDistanceFromActor;
	float m_fCameraMoveWidth;
	NiPoint3 m_kPrevTrn;

	float m_fNormalInterpolateTime;
	NiPoint3 m_kTargetPathNormal;
	NiPoint3 m_kCurrentPathNormal;
	NiPoint3 m_kLastPathNormal;

	bool m_bCameraSmoothMove;

	NiQuaternion m_kSpeed;

	NiPoint3 kZoomOutCameraPosition;
	NiQuaternion kZoomOutCameraRotation;
	NiPoint3 kZoomInCameraPosition;
	NiQuaternion kZoomInCameraRotation;
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEBOX_H