#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEEVENT_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEEVENT_H


#include "PgICameraMode.H"

class	PgCameraModeEvent	:	public	PgICameraMode
{
public:

	enum	STATE
	{
		S_NONE=0,
		S_MOVE_TO_TARGET_POS,
		S_STOPPED
	};

private:

	STATE	m_kState;

	float	m_fMoveStartTime;
	float	m_fTotalMoveTime;

	BM::GUID m_kTargetActor;
	NiPoint3	m_kTargetPos;
	NiPoint3	m_kTargetPathNormal;

	NiPoint3	m_kStartPathNormal;
	NiPoint3	m_kStartCamPos;

	float	m_fCamDistance;	//	타겟으로부터의 카메라 거리
	float	m_fCamMoveUpDistance;	//	수직 상승 거리
	float	m_fHorizRotAngle;	//	수직축을 중심으로한 수평 회전 각도
	float	m_fVertRotAngle;	//	수평축을 중심으로한 수직 회전 각도

	NiPoint3	m_kLookTargetPos;	//	카메라가 바라볼 목표 위치
	NiPoint3	m_kCamTargetPos;	//	카메라가 위치할 목표 위치

	//	카메라 가속 이동 관련 변수들
	float	m_fTotalMoveDistance;
	float	m_fA,m_fB;

public:

	PgCameraModeEvent(NiCamera *pkCamera);

	bool Update(float fFrameTime);

public:

	//	All Angles are in degree.
	void	SetTargetActor(BM::GUID const &kTargetActor);
	void	SetTargetPos(NiPoint3 const &kTargetPos);

	void	StartMoveCamera(float fMoveTime,float fCamDistance=-1,float fCamMoveUpDistance=-1,float fHorizRotAngle=-1,float fVertRotAngle=-1);

	STATE	GetState()	{	return	m_kState;	}

private:

	void	UpdateMoveToTargetPos(float fFrameTime);
	void	UpdateMoveToTargetActor(float fFrameTime);
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEEVENT_H