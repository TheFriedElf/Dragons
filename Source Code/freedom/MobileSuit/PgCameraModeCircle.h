#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODECIRCLE_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODECIRCLE_H

#include "PgICameraMode.h"
#include "PgCameraMan.h"

class PgCameraModeCircle : public PgICameraMode
{
public:
	PgCameraModeCircle(NiCamera *pkCamera, PgActor *pkActor);
	virtual ~PgCameraModeCircle(void);
	virtual bool Update(float fFrameTime);

	NiPoint3 &GetSpanTrn();
	void SetSpanTrn(NiPoint3 &rkSpanTrn);
	NiPoint3 GetActorPosition();

	void SetCameraAdjustInfo(CameraAdjustInfo& kAdjustInfo) { m_kCameraAdjustInfo = kAdjustInfo; }

protected:
	bool UpdateTranslate(float fTime);
	bool UpdateRotate(float fTime);

protected:
	NiPoint3 m_kPrevTrn;
	NiPoint3 m_kFloor;
	CameraAdjustInfo m_kCameraAdjustInfo;
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODECIRCLE_H