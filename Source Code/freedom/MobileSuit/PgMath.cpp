#include "stdafx.h"
#include "PgMath.h"
 void	SetPoint(POINT &kOutPoint,const	int iX,const	int iY)
{
	kOutPoint.x = iX;
	kOutPoint.y = iY;
}
float	GetDistanceFromCamPlane(NiCamera* pkCamera,NiPoint3 const &kPoint)
{
	const NiFrustum &kFrustum = pkCamera->GetViewFrustum();
	NxVec3 vFrontBox[4];

	NxVec3 vCamPos(pkCamera->GetWorldLocation().x,pkCamera->GetWorldLocation().y,pkCamera->GetWorldLocation().z);
	NxVec3 vCamD(pkCamera->GetWorldDirection().x,pkCamera->GetWorldDirection().y,pkCamera->GetWorldDirection().z);
	NxVec3 vCamR(pkCamera->GetWorldRightVector().x,pkCamera->GetWorldRightVector().y,pkCamera->GetWorldRightVector().z);
	NxVec3 vCamU(pkCamera->GetWorldUpVector().x,pkCamera->GetWorldUpVector().y,pkCamera->GetWorldUpVector().z);

	NxVec3 vFrontPos = vCamPos;
	vFrontBox[0] = vFrontPos+vCamR*kFrustum.m_fLeft+vCamU*kFrustum.m_fTop;
	vFrontBox[1] = vFrontPos+vCamR*kFrustum.m_fRight+vCamU*kFrustum.m_fTop;
	vFrontBox[2] = vFrontPos+vCamR*kFrustum.m_fRight+vCamU*kFrustum.m_fBottom;

	NxPlane plane(vFrontBox[0],vFrontBox[1],vFrontBox[2]);
	return	plane.distance(NxVec3(kPoint.x,kPoint.y,kPoint.z));
}

NiPoint3	GetCollPos_LinePlane(NiPoint3 const &kLineP1,NiPoint3 const &kLineP2,NiPoint3 const &kPlaneP3,NiPoint3 const &kPlaneNormal)
{
	NiPoint3	kCollPos(-1,-1,-1);

	float	fDot1 = (kLineP2 - kLineP1).Dot(kPlaneNormal);
	if(fDot1 == 0)
	{
		return	kCollPos;
	}
	float	fDot2 = (kPlaneP3 - kLineP1).Dot(kPlaneNormal);

	float	fU = fDot2 / fDot1;

	kCollPos = kLineP1 + fU*(kLineP2 - kLineP1);
	return	kCollPos;
}

float	GB2CM(float fLengthInGB)	//	게임브리오 길이를 cm 로 바꾼다.
{	
	return	fLengthInGB/0.3937f;	
}	
float	CM2GB(float fLengthInCM)	//	cm 를 게임브리오 길이로 바꾼다.
{	
	return	fLengthInCM*0.3937f;	
}	
NiPoint3	GetReflectionVec(NiPoint3 const &kNormal,NiPoint3 const &kDir)	//	kDir 의 kNormal 에 대한 반사벡터를 구한다.
{
	return	(2*(-kDir).Dot(kNormal))*kNormal+kDir;
}

void	Catmull_Rom_Splines(NiPoint3 const &p0,NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,float fTime,NiPoint3 &vOut)		//	0<=fTime<=1
{
	vOut = 0.5 *((2 * p1) + (-p0 + p2) * fTime +(2*p0 - 5*p1 + 4*p2 - p3) * fTime*fTime +	(-p0 + 3*p1- 3*p2 + p3) * fTime*fTime*fTime);
}
void	Bezier3(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,float fmu,NiPoint3 &vOut)
{
    float	mu2 =  fmu * fmu;
	float	mum1 = 1 - fmu;
	float	mum12 = mum1 * mum1;
    vOut.x = p1.x * mum12 + 2 * p2.x * mum1 * fmu + p3.x * mu2;
    vOut.y = p1.y * mum12 + 2 * p2.y * mum1 * fmu + p3.y * mu2;
    vOut.z = p1.z * mum12 + 2 * p2.z * mum1 * fmu + p3.z * mu2;
}

void	Bezier4(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,NiPoint3 const &p4,float fmu,NiPoint3 &vOut)
{
   double mum1,mum13,mu3;
   mum1 = 1 - fmu;
   mum13 = mum1 * mum1 * mum1;
   mu3 = fmu * fmu * fmu;

   vOut.x = (float)(mum13*p1.x + 3*fmu*mum1*mum1*p2.x + 3*fmu*fmu*mum1*p3.x + mu3*p4.x);
   vOut.y = (float)(mum13*p1.y + 3*fmu*mum1*mum1*p2.y + 3*fmu*fmu*mum1*p3.y + mu3*p4.y);
   vOut.z = (float)(mum13*p1.z + 3*fmu*mum1*mum1*p2.z + 3*fmu*fmu*mum1*p3.z + mu3*p4.z);
}

D3DXMATRIX ModifyProjectionMatrix(const D3DXMATRIX &OriginalProjectionMatrix,const PgVector4D& ClipPlane)
{

	D3DXMATRIX	matrix;
	PgVector4D	q;

	q.x = (sgn(ClipPlane.x) - matrix._31) / matrix._11;
	q.y = (sgn(ClipPlane.y) - matrix._32) / matrix._22;
	q.z = 1.0f;
	q.w = (1.0f - matrix._33) / matrix._43;

	PgVector4D	c = ClipPlane * (1.0f / (ClipPlane * q) );

	matrix._13 = c.x;
	matrix._23 = c.y;
	matrix._33 = c.z;
	matrix._43 = c.w;

	return	matrix;
}
bool	Get_Clipped_Region(NiPoint2 &ptLeftTop,NiPoint2 &ptWidthHeight,NiPoint2 &ptClippedAdjust,const NiPoint2 &ptClipLeftTop,const NiPoint2  &ptClipRightBottom)
{

	if(ptLeftTop.x>=ptClipRightBottom.x || ptLeftTop.y>=ptClipRightBottom.y) return false;	//	Not Visible
	if(ptLeftTop.x+ptWidthHeight.x<=ptClipLeftTop.x || ptLeftTop.y+ptWidthHeight.y<=ptClipLeftTop.y) return false;	//	not visible


	ptClippedAdjust.x = ptClippedAdjust.y = 0;
	if(ptLeftTop.x<ptClipLeftTop.x)
	{
		ptClippedAdjust.x = ptClipLeftTop.x - ptLeftTop.x;
		ptLeftTop.x = ptClipLeftTop.x;
		ptWidthHeight.x-=ptClippedAdjust.x;
	}
	if(ptLeftTop.y<ptClipLeftTop.y)
	{
		ptClippedAdjust.y = ptClipLeftTop.y - ptLeftTop.y;
		ptLeftTop.y = ptClipLeftTop.y;
		ptWidthHeight.y-=ptClippedAdjust.y;
	}

	return	true;
}
