#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT3_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT3_H

class lwPoint3
{
public:
	lwPoint3(NiPoint3 const &rkPoint3);
	lwPoint3(float x, float y, float z);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! Dot Product
	float Dot(lwPoint3& rhs);

	//! Cross Product
	lwPoint3 Cross(lwPoint3& rhs);

	//! Get x,y,z
	float GetX()const;
	float GetY()const;
	float GetZ()const;

	//! Set x,y,z
	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);

	//! Magnitude
	float Length()const;

	NiPoint3 &operator()();

	void Multiply(float fFactor);
	void Unitize();
	void Add(lwPoint3 pt);
	void Subtract(lwPoint3 pt);
	void Rotate(lwPoint3 kAxis,float fAngle);
	bool IsZero()const;
	bool IsEqual(lwPoint3 pt)const;
	float Distance(lwPoint3 pt);

	lwPoint3 Multiply2(float fFactor);
	lwPoint3 Add2(lwPoint3 pt);
	lwPoint3 Subtract2(lwPoint3 pt);


protected:
	NiPoint3 m_kPoint3;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWPOINT3_H