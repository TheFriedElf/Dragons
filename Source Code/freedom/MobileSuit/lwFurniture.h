#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWFURNITURE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWFURNITURE_H

class PgFurniture;

class lwFurniture
{
public:
	lwFurniture(PgFurniture *pkFurniture);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! NULL 객체인가?
	bool IsNil();

	//! 진짜 객체를 반환한다.
	PgFurniture *lwFurniture::operator()();

	//! 가구를 돌린다.
	void IncRotate(float fAngle);

protected:
	PgFurniture *m_pkFurniture;
};
#endif //FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWFURNITURE_H