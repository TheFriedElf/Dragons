#pragma once

typedef int CActorPtr;

class CCamFollowMode
{
public:
	CCamFollowMode(NiCameraPtr spCamera, CActorPtr spActor);
	~CCamFollowMode(void);

public:
	bool GetTranslate(float fAccumTime, NiPoint3& m_rkLocOut);
	bool GetRotate(float fAccumTime, NiQuaternion& m_rkRotOut);

protected:
	NiCameraPtr m_spCamera;
	CActorPtr m_spActor;
};
