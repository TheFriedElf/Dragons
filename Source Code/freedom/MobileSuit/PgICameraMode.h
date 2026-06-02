#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGICAMERAMODE_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGICAMERAMODE_H

class PgActor;
class PgInput;

//! 카메라 제어 모드들의 기본이 되는 인터페이스
class PgICameraMode : public NiMemObject
{
public:
	//! 기본 생성자
	PgICameraMode(NiCamera *pkCamera);
	
	//! 파괴자
	virtual ~PgICameraMode(void) = 0;

	//! 제어 중인 카메라를 리턴한다.
	NiCamera *GetCamera();
	
	//! 제어 중인 카메라를 변경한다.	
	void SetCamera(NiCamera *pkCamera);

	//! 카메라의 위치를 갱신한다.
	virtual bool Update(float fFrameTime) = 0;

	virtual	bool Input(PgInput *pkInput) {return false;}	//	return  true : 내부에서 인풋을 사용했음. false : 인풋을 사용하지 않았음.

	virtual void ResetCameraPos() {};
	virtual void SaveCameraPos() {};
	virtual void RestoreCameraPos() {};
	virtual void NotifyActorRemove(PgActor* pkActor);

	virtual PgActor* GetActor() { return m_pkActor; }
	virtual void SetActor(PgActor* pkActor) { m_pkActor = pkActor; }

protected:
	NiCamera *m_pkCamera;
	PgActor* m_pkActor;
};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGICAMERAMODE_H