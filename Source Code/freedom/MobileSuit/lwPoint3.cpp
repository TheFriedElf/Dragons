#include "stdafx.h"
#include "lwPoint3.h"

lwPoint3::lwPoint3(NiPoint3 const &rkPoint3)
{
	m_kPoint3 = rkPoint3;
}

lwPoint3::lwPoint3(float x, float y, float z) 
	: m_kPoint3(x, y, z)
{
}

bool lwPoint3::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwPoint3>(pkState, "Point3")
		.def(pkState, constructor<float, float, float>())
		.def(pkState, "SetX", &lwPoint3::SetX)
		.def(pkState, "SetY", &lwPoint3::SetY)
		.def(pkState, "SetZ", &lwPoint3::SetZ)
		.def(pkState, "GetX", &lwPoint3::GetX)
		.def(pkState, "GetY", &lwPoint3::GetY)
		.def(pkState, "GetZ", &lwPoint3::GetZ)
		.def(pkState, "Dot", &lwPoint3::Dot)
		.def(pkState, "Cross", &lwPoint3::Cross)
		.def(pkState, "Length", &lwPoint3::Length)
		.def(pkState, "Unitize", &lwPoint3::Unitize)
		.def(pkState, "Multiply", &lwPoint3::Multiply)
		.def(pkState, "Add", &lwPoint3::Add)
		.def(pkState, "Subtract", &lwPoint3::Subtract)

		.def(pkState, "_Multiply", &lwPoint3::Multiply2)
		.def(pkState, "_Add", &lwPoint3::Add2)
		.def(pkState, "_Subtract", &lwPoint3::Subtract2)

		.def(pkState, "Rotate", &lwPoint3::Rotate)
		.def(pkState, "IsZero", &lwPoint3::IsZero)
		.def(pkState, "IsEqual", &lwPoint3::IsEqual)
		.def(pkState, "Distance", &lwPoint3::Distance)
		;

	return true;
}
float	lwPoint3::Distance(lwPoint3 pt)
{
	return	(pt()-m_kPoint3).Length();
}
void	lwPoint3::Rotate(lwPoint3 kAxis,float fAngle)
{
	NiQuaternion kQuat(fAngle,kAxis());
	NiMatrix3	kRotMat;

	kQuat.ToRotation(kRotMat);

	m_kPoint3 = kRotMat*m_kPoint3;
}

float lwPoint3::GetX()const
{
	return m_kPoint3.x;
}

float lwPoint3::GetY()const
{
	return m_kPoint3.y;
}

float lwPoint3::GetZ()const
{
	return m_kPoint3.z;
}

void lwPoint3::SetX(float x)
{
	m_kPoint3.x = x;
}

void lwPoint3::SetY(float y)
{
	m_kPoint3.y = y;
}

void lwPoint3::SetZ(float z)
{
	m_kPoint3.z = z;
}

float lwPoint3::Dot(lwPoint3 &rhs)
{
	return m_kPoint3.Dot(NiPoint3(rhs.m_kPoint3.x, rhs.m_kPoint3.y, rhs.m_kPoint3.z));
}

lwPoint3 lwPoint3::Cross(lwPoint3 &rhs)
{
	m_kPoint3 = m_kPoint3.Cross(NiPoint3(rhs.m_kPoint3.x, rhs.m_kPoint3.y, rhs.m_kPoint3.z));
	return lwPoint3(m_kPoint3);
}

float lwPoint3::Length()const
{
	return m_kPoint3.Length();
}

void lwPoint3::Multiply(float fFactor)
{
	m_kPoint3 *= fFactor;
}
void lwPoint3::Add(lwPoint3 pt)
{
	m_kPoint3 += pt();
}

void lwPoint3::Subtract(lwPoint3 pt)
{
	m_kPoint3 -= pt();
}

void lwPoint3::Unitize()
{
	m_kPoint3.Unitize();
}

NiPoint3 &lwPoint3::operator()()
{
	return m_kPoint3;
}

bool lwPoint3::IsZero()const
{
	return (m_kPoint3 == NiPoint3::ZERO);
}

bool lwPoint3::IsEqual(lwPoint3 pt)const
{
	return (m_kPoint3 == pt());
}

lwPoint3 lwPoint3::Multiply2(float fFactor)
{
	return lwPoint3(m_kPoint3 * fFactor);
}
lwPoint3 lwPoint3::Add2(lwPoint3 pt)
{
	return lwPoint3(m_kPoint3 + pt());
}

lwPoint3 lwPoint3::Subtract2(lwPoint3 pt)
{
	return lwPoint3(m_kPoint3 - pt());
}
