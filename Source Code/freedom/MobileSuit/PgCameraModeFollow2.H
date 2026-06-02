#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW2_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW2_H

#include "PgICameraMode.h"

class PgActor;
class	PgInput;
class PgCameraModeFollow2: public PgICameraMode
{

	PgActor	*m_pkActor;

public:

	PgCameraModeFollow2(NiCamera *pkCamera,PgActor *pkActor);
	virtual	~PgCameraModeFollow2();

	bool Update(float fFrameTime);

public:

	bool	Input(PgInput *pkInput);	//	return  true : 내부에서 인풋을 사용했음. false : 인풋을 사용하지 않았음.

};

#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFOLLOW2_H