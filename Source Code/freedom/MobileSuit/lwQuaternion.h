#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUATERNION_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUATERNION_H

#include "lwPoint3.h"

class lwQuaternion
{
public:
	lwQuaternion(const NiQuaternion &rkQuat);
	lwQuaternion(float fAngle, lwPoint3 kAxis);
	lwQuaternion(float x, float y, float z, float w);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);
	
	lwQuaternion Multiply(lwQuaternion Quat);
	//! Get x,y,z
	float GetX();
	float GetY();
	float GetZ();
	float GetW();

	//! Set x,y,z
	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);
	void SetW(float w);

	NiQuaternion &operator()();

protected:
	NiQuaternion m_kQuaternion;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUATERNION_H