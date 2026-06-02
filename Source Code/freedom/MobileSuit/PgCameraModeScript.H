#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODESCRIPT_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODESCRIPT_H

#include "PgICameraMode.H"

// 지정된 루아 스크립트로 카메라를 컨트롤한다.
class	PgCameraModeScript	:	public	PgICameraMode
{
	std::string	m_kScriptName;

public:

	PgCameraModeScript(NiCamera *pkCamera);

	//! 파괴자
	virtual ~PgCameraModeScript(void);
	//! 카메라의 위치를 갱신한다.
	bool Update(float fFrameTime);

	//!	카메라를 컨트롤할 루아 스크립트를 설정한다.
	void	SetControlScript(std::string const kScriptName);
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODESCRIPT_H