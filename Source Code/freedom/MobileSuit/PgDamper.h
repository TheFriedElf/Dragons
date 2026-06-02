#ifndef FREEDOM_DRAGONICA_UTIL_PGDAMPER_H
#define FREEDOM_DRAGONICA_UTIL_PGDAMPER_H

class PgDamper
{
protected:
	PgDamper(void) {}

public:
	//! Ω∫«¡∏µ ¥Ô∆€
	static NiPoint3 SpringDamp(
		NiPoint3 const &rkFrom, 
		NiPoint3 const &rkTo, 
		NiPoint3 const &rkPrev, 
		float fDeltaTime, 
		float fSpringConst, 
		float fDampConst, 
		float fSpringLen,
		float fMaxLength = 0.0f,
		bool bCheckHeight = true);
};
#endif // FREEDOM_DRAGONICA_UTIL_PGDAMPER_H