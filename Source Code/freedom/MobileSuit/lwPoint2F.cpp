#include "stdafx.h"
#include "lwPoint2F.h"

lwPoint2F::lwPoint2F(POINT2F const &rkPoint2f)
{
	m_kPoint2f = rkPoint2f;
}

lwPoint2F::lwPoint2F(float const x, float const y) 
	: m_kPoint2f(x, y)
{
}

bool lwPoint2F::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwPoint2F>(pkState, "Point2F")
		.def(pkState, constructor<float, float>())
		.def(pkState, "SetX", &lwPoint2F::SetX)
		.def(pkState, "SetY", &lwPoint2F::SetY)
		.def(pkState, "GetX", &lwPoint2F::GetX)
		.def(pkState, "GetY", &lwPoint2F::GetY)
		.def(pkState, "IncX", &lwPoint2F::IncX)
		.def(pkState, "IncY", &lwPoint2F::IncY)
		;

	return true;
}

float lwPoint2F::GetX()
{
	return m_kPoint2f.x;
}

float lwPoint2F::GetY()
{
	return m_kPoint2f.y;
}

void lwPoint2F::SetX(float x)
{
	m_kPoint2f.x = x;
}

void lwPoint2F::SetY(float y)
{
	m_kPoint2f.y = y;
}

void lwPoint2F::IncX(float x)
{
	m_kPoint2f.x += x;
}

void lwPoint2F::IncY(float y)
{
	m_kPoint2f.y += y;
}

POINT2F&lwPoint2F::operator()()
{
	return m_kPoint2f;
}
