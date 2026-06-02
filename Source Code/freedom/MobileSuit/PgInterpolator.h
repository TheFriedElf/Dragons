#ifndef FREEDOM_DRAGONICA_UTIL_PGINTERPLATOR_H
#define FREEDOM_DRAGONICA_UTIL_PGINTERPLATOR_H

class PgInterpolator
{
protected:
	PgInterpolator(void) {}

public:
	static NiPoint3 Lerp(NiPoint3 const &rkPointA, NiPoint3 &rkPointB, float fTime);
};
#endif // FREEDOM_DRAGONICA_UTIL_PGINTERPLATOR_H