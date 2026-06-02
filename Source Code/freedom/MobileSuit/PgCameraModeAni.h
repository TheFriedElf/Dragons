#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEANI_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEANI_H

#include "PgICameraMode.h"
#include "PgActor.h"

//카메라 자체에 애니가 지정된 카메라 움직임대로 카메라를 움직임
class PgCameraModeAni : public PgICameraMode
{
public:
	PgCameraModeAni(NiCamera *pkCamera);
	virtual ~PgCameraModeAni();
	virtual bool Update(float fFrameTime);

	void SetAniCamera(NiCamera* pkAniCamera);

	bool GetUseActorPositionAsSoundListener() { return m_bUseActorPositionAsSoundListener; }
	void SetUseActorPositionAsSoundListener(bool bUse) { m_bUseActorPositionAsSoundListener = bUse; }
	NiAVObject* GetActorRoot() { return m_pkActorRoot; }
	void SetActorRoot(NiAVObject* pkRoot) { m_pkActorRoot = pkRoot; }
protected :
	NiCamera* m_pkAniCamera;
	NiFrustum m_kOldFrustum;
	bool m_bUseActorPositionAsSoundListener;
	NiAVObject* m_pkActorRoot;
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEANI_H