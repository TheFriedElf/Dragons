#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2F_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2F_H
#include "BM/Point.h"
class lwPoint2F
{
public:
	lwPoint2F(POINT2F const &rkPoint2f);
	lwPoint2F(float const x, float const y);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	float GetX();
	float GetY();

	void SetX(float x);
	void SetY(float y);

	void IncX(float x);
	void IncY(float y);

	POINT2F &operator()();

	void Unitize();

protected:
	POINT2F m_kPoint2f;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2F_H