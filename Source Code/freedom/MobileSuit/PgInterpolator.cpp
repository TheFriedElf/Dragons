#include "StdAfx.h"
#include "PgInterpolator.h"

NiPoint3 PgInterpolator::Lerp(NiPoint3 const &rkPointA, NiPoint3 &rkPointB, float fPerc)
{
	return NiPoint3(
		rkPointA.x + fPerc * (rkPointB.x - rkPointA.x),
		rkPointA.y + fPerc * (rkPointB.y - rkPointA.y),
		rkPointA.z + fPerc * (rkPointB.z - rkPointA.z)
		);
}