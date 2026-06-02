#include "stdafx.h"
#include "lwPoint2.h"

lwPoint2::lwPoint2(POINT2 const &rkPoint2)
{
	m_kPoint2 = rkPoint2;
}

lwPoint2::lwPoint2(int const x, int const y) 
	: m_kPoint2(x, y)
{
}

bool lwPoint2::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwPoint2>(pkState, "Point2")
		.def(pkState, constructor<int, int>())
		.def(pkState, "SetX", &lwPoint2::SetX)
		.def(pkState, "SetY", &lwPoint2::SetY)
		.def(pkState, "GetX", &lwPoint2::GetX)
		.def(pkState, "GetY", &lwPoint2::GetY)
		.def(pkState, "IncX", &lwPoint2::IncX)
		.def(pkState, "IncY", &lwPoint2::IncY)
		.def(pkState, "IsEqual", &lwPoint2::IsEqual)
		;

	return true;
}

int lwPoint2::GetX()const
{
	return m_kPoint2.x;
}

int lwPoint2::GetY()const
{
	return m_kPoint2.y;
}

void lwPoint2::SetX(int x)
{
	m_kPoint2.x = x;
}

void lwPoint2::SetY(int y)
{
	m_kPoint2.y = y;
}

void lwPoint2::IncX(int x)
{
	m_kPoint2.x += x;
}

void lwPoint2::IncY(int y)
{
	m_kPoint2.y += y;
}

bool lwPoint2::IsEqual(lwPoint2 rhs)const
{
	return m_kPoint2 == rhs.m_kPoint2;
}

POINT2 &lwPoint2::operator()()
{
	return m_kPoint2;
}
