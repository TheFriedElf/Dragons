#include "StdAfx.h"
#include "lwQuaternion.h"

lwQuaternion lwQuaternion4(float x, float y, float z, float w)
{
	NiQuaternion kQuat;
	kQuat.SetX(x);
	kQuat.SetY(y);
	kQuat.SetZ(z);
	kQuat.SetW(w);

	return (lwQuaternion)kQuat;
}

lwQuaternion::lwQuaternion(const NiQuaternion &rkQuat)
{
	m_kQuaternion = rkQuat;
}

lwQuaternion::lwQuaternion(float x, float y, float z, float w)
{
	m_kQuaternion.SetX(x);
	m_kQuaternion.SetY(y);
	m_kQuaternion.SetZ(z);
	m_kQuaternion.SetW(w);
}

lwQuaternion::lwQuaternion(float fAngle, lwPoint3 kAxis)
{
	m_kQuaternion.FromAngleAxis(fAngle, kAxis());
}

//! 스크립팅 시스템에 등록한다.
bool lwQuaternion::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "Quaternion4", &lwQuaternion4);

	class_<lwQuaternion>(pkState, "Quaternion")
//		.def(pkState, constructor<float, float, float, float>())
		.def(pkState, constructor<float, lwPoint3>())
		.def(pkState, "GetX", &lwQuaternion::GetX)
		.def(pkState, "GetY", &lwQuaternion::GetY)
		.def(pkState, "GetZ", &lwQuaternion::GetZ)
		.def(pkState, "GetW", &lwQuaternion::GetW)
		.def(pkState, "SetX", &lwQuaternion::SetX)
		.def(pkState, "SetY", &lwQuaternion::SetY)
		.def(pkState, "SetZ", &lwQuaternion::SetZ)
		.def(pkState, "SetW", &lwQuaternion::SetW)
		.def(pkState, "Multiply", &lwQuaternion::Multiply)
		;

	return true;
}

lwQuaternion lwQuaternion::Multiply(lwQuaternion Quat)
{
	return lwQuaternion(m_kQuaternion * Quat());
}

//! Get x,y,z
float lwQuaternion::GetX()
{
	return m_kQuaternion.GetX();
}

float lwQuaternion::GetY()
{
	return m_kQuaternion.GetY();
}

float lwQuaternion::GetZ()
{
	return m_kQuaternion.GetZ();
}

float lwQuaternion::GetW()
{
	return m_kQuaternion.GetW();
}

//! Set x,y,z
void lwQuaternion::SetX(float x)
{
	m_kQuaternion.SetX(x);
}

void lwQuaternion::SetY(float y)
{
	m_kQuaternion.SetY(y);
}

void lwQuaternion::SetZ(float z)
{
	m_kQuaternion.SetZ(z);
}

void lwQuaternion::SetW(float w)
{
	m_kQuaternion.SetW(w);
}

NiQuaternion &lwQuaternion::operator()()
{
	return m_kQuaternion;
}