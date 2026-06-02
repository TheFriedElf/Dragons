#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEMOVIE_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEMOVIE_H

#include "PgICameraMode.h"

class PgActor;

//Max에서 지정된 움직임대로 카메라를 움직임
class PgCameraModeMovie : public PgICameraMode
{
public:
	PgCameraModeMovie(NiCamera *pkCamera, NiActorManagerPtr spAM);
	~PgCameraModeMovie(void);

	bool SetTargetAnimation(int const &iNum, char const* szScript = 0);
	bool IsAnimationDone();

	virtual bool Update(float fFrameTime);
	
	void SetNextScript(char const *szScript);

	NiActorManager::SequenceID GetSequenceID()	{ return m_kSeqID; }

	void SetTranslate(NiPoint3 const &kTrn);

	void SetAMUpdate(bool bUpdate);

protected:

	NiActorManagerPtr m_spAM;
	NiActorManager::SequenceID m_kSeqID;	//현재 애니메이션 번호
	NiCamera *m_pkMovieCamera;
	std::string	m_szNextScript;				//카메라 애니가 끝나고 실행할 스크립트	
	bool m_bAMUpdate;
	NiFrustum m_kOldFrustum;
};
#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEMOVIE_H