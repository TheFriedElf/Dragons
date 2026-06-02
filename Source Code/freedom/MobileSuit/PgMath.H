#ifndef FREEDOM_DRAGONICA_UTIL_PGMATH_H
#define FREEDOM_DRAGONICA_UTIL_PGMATH_H

#include "NiMain.h"

inline float GetfRandomRange(float fMin, float fMax);
inline NiPoint3 GetfPoint3Range(float fMinX, float fMaxX, float fMinY, float fMaxY, float fMinZ, float fMaxZ);

//	교차판정

//	직선과 평면과의 교차점을 리턴한다. 교차점이 없을 경우(평행) (-1,-1,-1) 리턴
NiPoint3	GetCollPos_LinePlane(NiPoint3 const &kLineP1,NiPoint3 const &kLineP2,NiPoint3 const &kPlaneP3,NiPoint3 const &kPlaneNormal);	

//	카메라 평면과 점과의 거리
float	GetDistanceFromCamPlane(NiCamera* pkCamera,NiPoint3 const &kPoint);

//	곡선
void	Catmull_Rom_Splines(NiPoint3 const &p0,NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,float fTime,NiPoint3 &vOut);
void	Bezier4(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,NiPoint3 const &p4,float fmu,NiPoint3 &vOut);
void	Bezier3(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,float fmu,NiPoint3 &vOut);

// 단위 변환
float	GB2CM(float fLengthInGB)	;//	게임브리오 길이를 cm 로 바꾼다.
float	CM2GB(float fLengthInCM);	//	cm 를 게임브리오 길이로 바꾼다.

void	SetPoint(POINT &kOutPoint,const	int iX,const	int iY);

bool	Get_Clipped_Region(NiPoint2 &ptLeftTop,NiPoint2 &ptWidthHeight,NiPoint2 &ptClippedAdjust,const NiPoint2 &ptClipLeftTop,const NiPoint2 &ptClipRightBottom);
NiPoint3	GetReflectionVec(NiPoint3 const &kNormal,NiPoint3 const &kDir);	//	kDir 의 kNormal 에 대한 반사벡터를 구한다.
inline float sgn(float a)
{
		if (a>0.0f) return (1.0f);
		if (a<0.0f) return (1.0f);
		return 0.0f;
}

struct	PgVector4D
{
	float	x,y,z,w;

	PgVector4D()	{}
	PgVector4D(float a,float b,float c,float d)
	{
		x = a;	y=b;	z=c;	w=d;
	}
	//	스칼라 곱
	PgVector4D	operator *(float s) const
	{
		return (PgVector4D(x*s,y*s,z*s,w*s));
	}
	//	내적
	float	operator *(const PgVector4D& v) const
	{
		return (x*v.x+y*v.y+z*v.z+w*v.w);
	}
};


//------------- FORWARD DECLARATIONS ---------------------------------------------------------------------------------------------------------------------------------
//------------- DEFINITIONS ------------------------------------------------------------------------------------------------------------------------------------------
//------------- DECLARATIONS -----------------------------------------------------------------------------------------------------------------------------------------

struct	PgMatrix4
{
	union 
	{
        struct 
		{
            float									m_f11, m_f12, m_f13, m_f14;					// First row
            float									m_f21, m_f22, m_f23, m_f24;					// Second row
            float									m_f31, m_f32, m_f33, m_f34;					// Third row
            float									m_f41, m_f42, m_f43, m_f44;					// Fourth row

        }												m_Elements;
        float										m_f[4][4];									// An array of floats that represent a 4x4 matrix
    };

	PgMatrix4 Clear( void )
	{
		// Sets matrix to 0.

		m_Elements.m_f11 = m_Elements.m_f12 = m_Elements.m_f13 = m_Elements.m_f14 =
		m_Elements.m_f21 = m_Elements.m_f22 = m_Elements.m_f23 = m_Elements.m_f24 =
		m_Elements.m_f31 = m_Elements.m_f32 = m_Elements.m_f33 = m_Elements.m_f34 =
		m_Elements.m_f41 = m_Elements.m_f42 = m_Elements.m_f43 = m_Elements.m_f44 = 0.0f;
		return (*this);
	}

	PgMatrix4 SetIdentity( void )
	{
		// Sets matrix to identity.

		m_Elements.m_f11 = m_Elements.m_f22 = m_Elements.m_f33 = m_Elements.m_f44 = 1.0f;
		m_Elements.m_f12 = m_Elements.m_f13 = m_Elements.m_f14 =
		m_Elements.m_f21 = m_Elements.m_f23 = m_Elements.m_f24 =
		m_Elements.m_f31 = m_Elements.m_f32 = m_Elements.m_f34 =
		m_Elements.m_f41 = m_Elements.m_f42 = m_Elements.m_f43 = 0.0f;
		return (*this);
	}

	PgMatrix4 Transpose( void )
	{
		// Transposes the matrix.

		PgMatrix4 origMatrix = (*this);
		m_Elements.m_f12 = origMatrix.m_Elements.m_f21;
		m_Elements.m_f13 = origMatrix.m_Elements.m_f31;
		m_Elements.m_f14 = origMatrix.m_Elements.m_f41;
		m_Elements.m_f21 = origMatrix.m_Elements.m_f12;
		m_Elements.m_f23 = origMatrix.m_Elements.m_f32;
		m_Elements.m_f24 = origMatrix.m_Elements.m_f42;
		m_Elements.m_f31 = origMatrix.m_Elements.m_f13;
		m_Elements.m_f32 = origMatrix.m_Elements.m_f23;
		m_Elements.m_f34 = origMatrix.m_Elements.m_f43;
		m_Elements.m_f41 = origMatrix.m_Elements.m_f14;
		m_Elements.m_f42 = origMatrix.m_Elements.m_f24;
		m_Elements.m_f43 = origMatrix.m_Elements.m_f34;
		return (*this);
	}

	PgMatrix4 Invert( void )
	{
		// Inverts the matrix.

		PgMatrix4 origMatrix = (*this);
		float fDet;
		float d12, d13, d23, d24, d34, d41;
		d12 =	origMatrix.m_Elements.m_f31*origMatrix.m_Elements.m_f42 -
				origMatrix.m_Elements.m_f41*origMatrix.m_Elements.m_f32;
		d13 =	origMatrix.m_Elements.m_f31*origMatrix.m_Elements.m_f43 -
				origMatrix.m_Elements.m_f41*origMatrix.m_Elements.m_f33;
		d23 =	origMatrix.m_Elements.m_f32*origMatrix.m_Elements.m_f43 -
				origMatrix.m_Elements.m_f42*origMatrix.m_Elements.m_f33;
		d24 =	origMatrix.m_Elements.m_f32*origMatrix.m_Elements.m_f44 -
				origMatrix.m_Elements.m_f42*origMatrix.m_Elements.m_f34;
		d34 =	origMatrix.m_Elements.m_f33*origMatrix.m_Elements.m_f44 -
				origMatrix.m_Elements.m_f43*origMatrix.m_Elements.m_f34;
		d41 =	origMatrix.m_Elements.m_f34*origMatrix.m_Elements.m_f41 -
				origMatrix.m_Elements.m_f44*origMatrix.m_Elements.m_f31;
		m_Elements.m_f11 =   origMatrix.m_Elements.m_f22*d34 - 
							 origMatrix.m_Elements.m_f23*d24 +
							 origMatrix.m_Elements.m_f24*d23;
		m_Elements.m_f12 = -(origMatrix.m_Elements.m_f21*d34 +
							 origMatrix.m_Elements.m_f23*d41 +
							 origMatrix.m_Elements.m_f24*d13);
		m_Elements.m_f13 =   origMatrix.m_Elements.m_f21*d24 +
							 origMatrix.m_Elements.m_f22*d41 +
							 origMatrix.m_Elements.m_f24*d12;
		m_Elements.m_f14 = -(origMatrix.m_Elements.m_f21*d23 -
							 origMatrix.m_Elements.m_f22*d13 +
							 origMatrix.m_Elements.m_f23*d12);
		fDet =	origMatrix.m_Elements.m_f11*m_Elements.m_f11 + 
				origMatrix.m_Elements.m_f12*m_Elements.m_f12 +
				origMatrix.m_Elements.m_f13*m_Elements.m_f13 +
				origMatrix.m_Elements.m_f14*m_Elements.m_f14;
		if( fDet==0.0f ) 
		{
			SetIdentity();
			return (*this);
		}
		fDet = 1.0f/fDet;
		m_Elements.m_f11 *= fDet;
		m_Elements.m_f12 *= fDet;
		m_Elements.m_f13 *= fDet;
		m_Elements.m_f14 *= fDet;
		m_Elements.m_f21 = -(origMatrix.m_Elements.m_f12*d34 -
							 origMatrix.m_Elements.m_f13*d24 +
							 origMatrix.m_Elements.m_f14*d23) * fDet;
		m_Elements.m_f22 =  (origMatrix.m_Elements.m_f11*d34 +
							 origMatrix.m_Elements.m_f13*d41 +
							 origMatrix.m_Elements.m_f14*d13) * fDet;
		m_Elements.m_f23 = -(origMatrix.m_Elements.m_f11*d24 +
							 origMatrix.m_Elements.m_f12*d41 +
							 origMatrix.m_Elements.m_f14*d12) * fDet;
		m_Elements.m_f24 =  (origMatrix.m_Elements.m_f11*d23 -
							 origMatrix.m_Elements.m_f12*d13 +
							 origMatrix.m_Elements.m_f13*d12) * fDet;
		d12 =	origMatrix.m_Elements.m_f11*origMatrix.m_Elements.m_f22 - 
				origMatrix.m_Elements.m_f21*origMatrix.m_Elements.m_f12;
		d13 =	origMatrix.m_Elements.m_f11*origMatrix.m_Elements.m_f23 - 
				origMatrix.m_Elements.m_f21*origMatrix.m_Elements.m_f13;
		d23 =	origMatrix.m_Elements.m_f12*origMatrix.m_Elements.m_f23 - 
				origMatrix.m_Elements.m_f22*origMatrix.m_Elements.m_f13;
		d24 =	origMatrix.m_Elements.m_f12*origMatrix.m_Elements.m_f24 - 
				origMatrix.m_Elements.m_f22*origMatrix.m_Elements.m_f14;
		d34 =	origMatrix.m_Elements.m_f13*origMatrix.m_Elements.m_f24 - 
				origMatrix.m_Elements.m_f23*origMatrix.m_Elements.m_f14;
		d41 =	origMatrix.m_Elements.m_f14*origMatrix.m_Elements.m_f21 - 
				origMatrix.m_Elements.m_f24*origMatrix.m_Elements.m_f11;
		m_Elements.m_f31 =  (origMatrix.m_Elements.m_f42*d34 -
							 origMatrix.m_Elements.m_f43*d24 +
							 origMatrix.m_Elements.m_f44*d23) * fDet;
		m_Elements.m_f32 = -(origMatrix.m_Elements.m_f41*d34 +
							 origMatrix.m_Elements.m_f43*d41 +
							 origMatrix.m_Elements.m_f44*d13) * fDet;
		m_Elements.m_f33 =  (origMatrix.m_Elements.m_f41*d24 +
							 origMatrix.m_Elements.m_f42*d41 +
							 origMatrix.m_Elements.m_f44*d12) * fDet;
		m_Elements.m_f34 = -(origMatrix.m_Elements.m_f41*d23 -
							 origMatrix.m_Elements.m_f42*d13 +
							 origMatrix.m_Elements.m_f43*d12) * fDet;
		m_Elements.m_f41 = -(origMatrix.m_Elements.m_f32*d34 -
							 origMatrix.m_Elements.m_f33*d24 +
							 origMatrix.m_Elements.m_f34*d23) * fDet;
		m_Elements.m_f42 =  (origMatrix.m_Elements.m_f31*d34 +
							 origMatrix.m_Elements.m_f33*d41 +
							 origMatrix.m_Elements.m_f34*d13) * fDet;
		m_Elements.m_f43 = -(origMatrix.m_Elements.m_f31*d24 +
							 origMatrix.m_Elements.m_f32*d41 +
							 origMatrix.m_Elements.m_f34*d12) * fDet;
		m_Elements.m_f44 =  (origMatrix.m_Elements.m_f31*d23 -
							 origMatrix.m_Elements.m_f32*d13 +
							 origMatrix.m_Elements.m_f33*d12) * fDet;
		return (*this);
	}

	PgMatrix4 AddMatrix( PgMatrix4& a_Matrix )
	{
		// Adds specified matrix to this one.

		m_Elements.m_f11 += a_Matrix.m_Elements.m_f11;
		m_Elements.m_f12 += a_Matrix.m_Elements.m_f12;
		m_Elements.m_f13 += a_Matrix.m_Elements.m_f13;
		m_Elements.m_f14 += a_Matrix.m_Elements.m_f14;
		m_Elements.m_f21 += a_Matrix.m_Elements.m_f21;
		m_Elements.m_f22 += a_Matrix.m_Elements.m_f22;
		m_Elements.m_f23 += a_Matrix.m_Elements.m_f23;
		m_Elements.m_f24 += a_Matrix.m_Elements.m_f24;
		m_Elements.m_f31 += a_Matrix.m_Elements.m_f31;
		m_Elements.m_f32 += a_Matrix.m_Elements.m_f32;
		m_Elements.m_f33 += a_Matrix.m_Elements.m_f33;
		m_Elements.m_f34 += a_Matrix.m_Elements.m_f34;
		m_Elements.m_f41 += a_Matrix.m_Elements.m_f41;
		m_Elements.m_f42 += a_Matrix.m_Elements.m_f42;
		m_Elements.m_f43 += a_Matrix.m_Elements.m_f43;
		m_Elements.m_f44 += a_Matrix.m_Elements.m_f44;
		return (*this);
	}

	PgMatrix4 MultiplyMatrixFromRightSide( PgMatrix4& a_Matrix )
	{
		// Multiplies specified matrix (a_Matrix) to this one from the right side.
		//@TODO: Implement faster version ...

		PgMatrix4 origMatrix = (*this);
		m_Elements.m_f11 =	origMatrix.m_Elements.m_f11*a_Matrix.m_Elements.m_f11 +
							origMatrix.m_Elements.m_f12*a_Matrix.m_Elements.m_f21 +
							origMatrix.m_Elements.m_f13*a_Matrix.m_Elements.m_f31 +
							origMatrix.m_Elements.m_f14*a_Matrix.m_Elements.m_f41;
		m_Elements.m_f21 =	origMatrix.m_Elements.m_f21*a_Matrix.m_Elements.m_f11 +
							origMatrix.m_Elements.m_f22*a_Matrix.m_Elements.m_f21 +
							origMatrix.m_Elements.m_f23*a_Matrix.m_Elements.m_f31 +
							origMatrix.m_Elements.m_f24*a_Matrix.m_Elements.m_f41;
		m_Elements.m_f31 =	origMatrix.m_Elements.m_f31*a_Matrix.m_Elements.m_f11 +
							origMatrix.m_Elements.m_f32*a_Matrix.m_Elements.m_f21 +
							origMatrix.m_Elements.m_f33*a_Matrix.m_Elements.m_f31 +
							origMatrix.m_Elements.m_f34*a_Matrix.m_Elements.m_f41;
		m_Elements.m_f41 =	origMatrix.m_Elements.m_f41*a_Matrix.m_Elements.m_f11 +
							origMatrix.m_Elements.m_f42*a_Matrix.m_Elements.m_f21 +
							origMatrix.m_Elements.m_f43*a_Matrix.m_Elements.m_f31 +
							origMatrix.m_Elements.m_f44*a_Matrix.m_Elements.m_f41;
		m_Elements.m_f12 =	origMatrix.m_Elements.m_f11*a_Matrix.m_Elements.m_f12 +
							origMatrix.m_Elements.m_f12*a_Matrix.m_Elements.m_f22 +
							origMatrix.m_Elements.m_f13*a_Matrix.m_Elements.m_f32 +
							origMatrix.m_Elements.m_f14*a_Matrix.m_Elements.m_f42;
		m_Elements.m_f22 =	origMatrix.m_Elements.m_f21*a_Matrix.m_Elements.m_f12 +
							origMatrix.m_Elements.m_f22*a_Matrix.m_Elements.m_f22 +
							origMatrix.m_Elements.m_f23*a_Matrix.m_Elements.m_f32 +
							origMatrix.m_Elements.m_f24*a_Matrix.m_Elements.m_f42;
		m_Elements.m_f32 =	origMatrix.m_Elements.m_f31*a_Matrix.m_Elements.m_f12 +
							origMatrix.m_Elements.m_f32*a_Matrix.m_Elements.m_f22 +
							origMatrix.m_Elements.m_f33*a_Matrix.m_Elements.m_f32 +
							origMatrix.m_Elements.m_f34*a_Matrix.m_Elements.m_f42;
		m_Elements.m_f42 =	origMatrix.m_Elements.m_f41*a_Matrix.m_Elements.m_f12 +
							origMatrix.m_Elements.m_f42*a_Matrix.m_Elements.m_f22 +
							origMatrix.m_Elements.m_f43*a_Matrix.m_Elements.m_f32 +
							origMatrix.m_Elements.m_f44*a_Matrix.m_Elements.m_f42;
		m_Elements.m_f13 =	origMatrix.m_Elements.m_f11*a_Matrix.m_Elements.m_f13 +
							origMatrix.m_Elements.m_f12*a_Matrix.m_Elements.m_f23 +
							origMatrix.m_Elements.m_f13*a_Matrix.m_Elements.m_f33 +
							origMatrix.m_Elements.m_f14*a_Matrix.m_Elements.m_f43;
		m_Elements.m_f23 =	origMatrix.m_Elements.m_f21*a_Matrix.m_Elements.m_f13 +
							origMatrix.m_Elements.m_f22*a_Matrix.m_Elements.m_f23 +
							origMatrix.m_Elements.m_f23*a_Matrix.m_Elements.m_f33 +
							origMatrix.m_Elements.m_f24*a_Matrix.m_Elements.m_f43;
		m_Elements.m_f33 =	origMatrix.m_Elements.m_f31*a_Matrix.m_Elements.m_f13 +
							origMatrix.m_Elements.m_f32*a_Matrix.m_Elements.m_f23 +
							origMatrix.m_Elements.m_f33*a_Matrix.m_Elements.m_f33 +
							origMatrix.m_Elements.m_f34*a_Matrix.m_Elements.m_f43;
		m_Elements.m_f43 =	origMatrix.m_Elements.m_f41*a_Matrix.m_Elements.m_f13 +
							origMatrix.m_Elements.m_f42*a_Matrix.m_Elements.m_f23 +
							origMatrix.m_Elements.m_f43*a_Matrix.m_Elements.m_f33 +
							origMatrix.m_Elements.m_f44*a_Matrix.m_Elements.m_f43;
		m_Elements.m_f14 =	origMatrix.m_Elements.m_f11*a_Matrix.m_Elements.m_f14 +
							origMatrix.m_Elements.m_f12*a_Matrix.m_Elements.m_f24 +
							origMatrix.m_Elements.m_f13*a_Matrix.m_Elements.m_f34 +
							origMatrix.m_Elements.m_f14*a_Matrix.m_Elements.m_f44;
		m_Elements.m_f24 =	origMatrix.m_Elements.m_f21*a_Matrix.m_Elements.m_f14 +
							origMatrix.m_Elements.m_f22*a_Matrix.m_Elements.m_f24 +
							origMatrix.m_Elements.m_f23*a_Matrix.m_Elements.m_f34 +
							origMatrix.m_Elements.m_f24*a_Matrix.m_Elements.m_f44;
		m_Elements.m_f34 =	origMatrix.m_Elements.m_f31*a_Matrix.m_Elements.m_f14 +
							origMatrix.m_Elements.m_f32*a_Matrix.m_Elements.m_f24 +
							origMatrix.m_Elements.m_f33*a_Matrix.m_Elements.m_f34 +
							origMatrix.m_Elements.m_f34*a_Matrix.m_Elements.m_f44;
		m_Elements.m_f44 =	origMatrix.m_Elements.m_f41*a_Matrix.m_Elements.m_f14 +
							origMatrix.m_Elements.m_f42*a_Matrix.m_Elements.m_f24 +
							origMatrix.m_Elements.m_f43*a_Matrix.m_Elements.m_f34 +
							origMatrix.m_Elements.m_f44*a_Matrix.m_Elements.m_f44;
		return (*this);
	}

	PgMatrix4 MultiplyMatrixFromLeftSide( PgMatrix4& a_Matrix )
	{
		// Multiplies specified matrix (a_Matrix) to this one from the left side.
		//@TODO: Implement faster version ...

		PgMatrix4 origMatrix = (*this);
		m_Elements.m_f11 =	a_Matrix.m_Elements.m_f11*origMatrix.m_Elements.m_f11 +
							a_Matrix.m_Elements.m_f12*origMatrix.m_Elements.m_f21 +
							a_Matrix.m_Elements.m_f13*origMatrix.m_Elements.m_f31 +
							a_Matrix.m_Elements.m_f14*origMatrix.m_Elements.m_f41;
		m_Elements.m_f21 =	a_Matrix.m_Elements.m_f21*origMatrix.m_Elements.m_f11 +
							a_Matrix.m_Elements.m_f22*origMatrix.m_Elements.m_f21 +
							a_Matrix.m_Elements.m_f23*origMatrix.m_Elements.m_f31 +
							a_Matrix.m_Elements.m_f24*origMatrix.m_Elements.m_f41;
		m_Elements.m_f31 =	a_Matrix.m_Elements.m_f31*origMatrix.m_Elements.m_f11 +
							a_Matrix.m_Elements.m_f32*origMatrix.m_Elements.m_f21 +
							a_Matrix.m_Elements.m_f33*origMatrix.m_Elements.m_f31 +
							a_Matrix.m_Elements.m_f34*origMatrix.m_Elements.m_f41;
		m_Elements.m_f41 =	a_Matrix.m_Elements.m_f41*origMatrix.m_Elements.m_f11 +
							a_Matrix.m_Elements.m_f42*origMatrix.m_Elements.m_f21 +
							a_Matrix.m_Elements.m_f43*origMatrix.m_Elements.m_f31 +
							a_Matrix.m_Elements.m_f44*origMatrix.m_Elements.m_f41;
		m_Elements.m_f12 =	a_Matrix.m_Elements.m_f11*origMatrix.m_Elements.m_f12 +
							a_Matrix.m_Elements.m_f12*origMatrix.m_Elements.m_f22 +
							a_Matrix.m_Elements.m_f13*origMatrix.m_Elements.m_f32 +
							a_Matrix.m_Elements.m_f14*origMatrix.m_Elements.m_f42;
		m_Elements.m_f22 =	a_Matrix.m_Elements.m_f21*origMatrix.m_Elements.m_f12 +
							a_Matrix.m_Elements.m_f22*origMatrix.m_Elements.m_f22 +
							a_Matrix.m_Elements.m_f23*origMatrix.m_Elements.m_f32 +
							a_Matrix.m_Elements.m_f24*origMatrix.m_Elements.m_f42;
		m_Elements.m_f32 =	a_Matrix.m_Elements.m_f31*origMatrix.m_Elements.m_f12 +
							a_Matrix.m_Elements.m_f32*origMatrix.m_Elements.m_f22 +
							a_Matrix.m_Elements.m_f33*origMatrix.m_Elements.m_f32 +
							a_Matrix.m_Elements.m_f34*origMatrix.m_Elements.m_f42;
		m_Elements.m_f42 =	a_Matrix.m_Elements.m_f41*origMatrix.m_Elements.m_f12 +
							a_Matrix.m_Elements.m_f42*origMatrix.m_Elements.m_f22 +
							a_Matrix.m_Elements.m_f43*origMatrix.m_Elements.m_f32 +
							a_Matrix.m_Elements.m_f44*origMatrix.m_Elements.m_f42;
		m_Elements.m_f13 =	a_Matrix.m_Elements.m_f11*origMatrix.m_Elements.m_f13 +
							a_Matrix.m_Elements.m_f12*origMatrix.m_Elements.m_f23 +
							a_Matrix.m_Elements.m_f13*origMatrix.m_Elements.m_f33 +
							a_Matrix.m_Elements.m_f14*origMatrix.m_Elements.m_f43;
		m_Elements.m_f23 =	a_Matrix.m_Elements.m_f21*origMatrix.m_Elements.m_f13 +
							a_Matrix.m_Elements.m_f22*origMatrix.m_Elements.m_f23 +
							a_Matrix.m_Elements.m_f23*origMatrix.m_Elements.m_f33 +
							a_Matrix.m_Elements.m_f24*origMatrix.m_Elements.m_f43;
		m_Elements.m_f33 =	a_Matrix.m_Elements.m_f31*origMatrix.m_Elements.m_f13 +
							a_Matrix.m_Elements.m_f32*origMatrix.m_Elements.m_f23 +
							a_Matrix.m_Elements.m_f33*origMatrix.m_Elements.m_f33 +
							a_Matrix.m_Elements.m_f34*origMatrix.m_Elements.m_f43;
		m_Elements.m_f43 =	a_Matrix.m_Elements.m_f41*origMatrix.m_Elements.m_f13 +
							a_Matrix.m_Elements.m_f42*origMatrix.m_Elements.m_f23 +
							a_Matrix.m_Elements.m_f43*origMatrix.m_Elements.m_f33 +
							a_Matrix.m_Elements.m_f44*origMatrix.m_Elements.m_f43;
		m_Elements.m_f14 =	a_Matrix.m_Elements.m_f11*origMatrix.m_Elements.m_f14 +
							a_Matrix.m_Elements.m_f12*origMatrix.m_Elements.m_f24 +
							a_Matrix.m_Elements.m_f13*origMatrix.m_Elements.m_f34 +
							a_Matrix.m_Elements.m_f14*origMatrix.m_Elements.m_f44;
		m_Elements.m_f24 =	a_Matrix.m_Elements.m_f21*origMatrix.m_Elements.m_f14 +
							a_Matrix.m_Elements.m_f22*origMatrix.m_Elements.m_f24 +
							a_Matrix.m_Elements.m_f23*origMatrix.m_Elements.m_f34 +
							a_Matrix.m_Elements.m_f24*origMatrix.m_Elements.m_f44;
		m_Elements.m_f34 =	a_Matrix.m_Elements.m_f31*origMatrix.m_Elements.m_f14 +
							a_Matrix.m_Elements.m_f32*origMatrix.m_Elements.m_f24 +
							a_Matrix.m_Elements.m_f33*origMatrix.m_Elements.m_f34 +
							a_Matrix.m_Elements.m_f34*origMatrix.m_Elements.m_f44;
		m_Elements.m_f44 =	a_Matrix.m_Elements.m_f41*origMatrix.m_Elements.m_f14 +
							a_Matrix.m_Elements.m_f42*origMatrix.m_Elements.m_f24 +
							a_Matrix.m_Elements.m_f43*origMatrix.m_Elements.m_f34 +
							a_Matrix.m_Elements.m_f44*origMatrix.m_Elements.m_f44;
		return (*this);
	}

	PgMatrix4 GeneratePitchRotate( float fPitch )
	{
		// Generates a pitch rotation matrix.

		this->SetIdentity();
		m_Elements.m_f22 = static_cast<float>( cos(fPitch) );
		m_Elements.m_f23 = static_cast<float>( sin(fPitch) );
		m_Elements.m_f32 = static_cast<float>( -sin(fPitch) );
		m_Elements.m_f33 = static_cast<float>( cos(fPitch) );
		return (*this);
	}

	PgMatrix4 GenerateYawRotate( float fYaw )
	{
		// Generates a yaw rotation matrix.

		this->SetIdentity();
		m_Elements.m_f11 = static_cast<float>( cos(fYaw) );
		m_Elements.m_f13 = static_cast<float>( -sin(fYaw) );
		m_Elements.m_f31 = static_cast<float>( sin(fYaw) );
		m_Elements.m_f33 = static_cast<float>( cos(fYaw) );
		return (*this);
	}

	PgMatrix4 GenerateRollRotate( float fRoll )
	{
		// Generates a roll rotation matrix.

		this->SetIdentity();
		m_Elements.m_f11 = static_cast<float>( cos(fRoll) );
		m_Elements.m_f12 = static_cast<float>( sin(fRoll) );
		m_Elements.m_f21 = static_cast<float>( -sin(fRoll) );
		m_Elements.m_f22 = static_cast<float>( cos(fRoll) );
		return (*this);
	}
};
D3DXMATRIX ModifyProjectionMatrix(const D3DXMATRIX &OriginalProjectionMatrix,const PgVector4D& ClipPlane);

float GetfRandomRange(float fMin, float fMax)
{
	if(fMin == fMax)
	{
		return fMin;
	}

	float const fRandom = ((float)BM::Rand_Range(UINT_MAX)/(float)UINT_MAX);

	return ( (fRandom * (float)fabs(fMax-fMin)) + fMin);
}

inline NiPoint3 GetfPoint3Range(float fMinX, float fMaxX, float fMinY, float fMaxY, float fMinZ, float fMaxZ)
{
	NiPoint3 const kRandom(GetfRandomRange(fMinX, fMaxX), GetfRandomRange(fMinY, fMaxY), GetfRandomRange(fMinZ, fMaxZ));

	return kRandom;
}
#endif // FREEDOM_DRAGONICA_UTIL_PGMATH_H