#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2_H
#include "BM/Point.h"
class lwPoint2
{
public:
	lwPoint2(POINT2 const &rkPoint2);
	lwPoint2(int const x, int const y);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	int GetX()const;
	int GetY()const;

	void SetX(int x);
	void SetY(int y);

	void IncX(int x);
	void IncY(int y);
	bool IsEqual(lwPoint2 rhs)const;

	POINT2 &operator()();

	void Unitize();

protected:
	POINT2 m_kPoint2;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT2_H