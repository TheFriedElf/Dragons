// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2006 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net


#include "stdafx.h"
#include "PlanarOccluder.h"

#include <NiCamera.h>
#include <NiGeometry.h>
#include <NiNode.h>

NiImplementRTTI(PlanarOccluder, Occluder);

//---------------------------------------------------------------------------
PlanarOccluder::PlanarOccluder()
{
}
//---------------------------------------------------------------------------
PlanarOccluder::PlanarOccluder(NiGeometry* pkPlane) 
{
    assert (pkPlane != 0);
    m_iCameraWhichSide = 0;
    FillData(pkPlane);
}
//---------------------------------------------------------------------------
PlanarOccluder::~PlanarOccluder()
{
}
//---------------------------------------------------------------------------
void PlanarOccluder::Update(NiCamera* pkCamera)
{

	NiFrustumPlanes	kPlanes(*pkCamera);

    unsigned int i;
    for (i = 0; i < NiFrustumPlanes::MAX_PLANES; i++)
    {
        int iSide = m_kWorldBound.WhichSide(
            kPlanes.GetPlane(i));

        if (iSide == NiPlane::NEGATIVE_SIDE)
        {
            // The object is not visible since it is on the negative
            // side of the plane.
            SetVisible(false);
			return;
        }

    }

	SetVisible(true);

    m_iCameraWhichSide = m_kOcclusionPlane.WhichSide(
	    pkCamera->GetWorldLocation());

    return;
}
//---------------------------------------------------------------------------
bool PlanarOccluder::IsOccluded(NiCamera* pkCamera, const NiBound& kBound)
{
	if(GetVisible() == false)
	{
		return	false;
	}

    NiPoint3 kTestLocation;
    bool bCull = false;
    kTestLocation = kBound.GetCenter() + kBound.GetRadius() *
	m_kOcclusionPlane.GetNormal();
    
    bCull = m_kOcclusionPlane.WhichSide(kTestLocation) != m_iCameraWhichSide;

    if (bCull)
    {
	kTestLocation = kBound.GetCenter() - 
	    kBound.GetRadius() * m_kOcclusionPlane.GetNormal();
	if (m_kOcclusionPlane.WhichSide(kTestLocation) == m_iCameraWhichSide)
	    bCull = false;
    }
    if (!bCull)
	return false;
    
    return CheckPlaneExtents(pkCamera, kBound);
}
//---------------------------------------------------------------------------
bool PlanarOccluder::CheckPlaneExtents(NiCamera* pkCamera,
    const NiBound& kBound)
{
    // The full bound falls on the opposite side of the plane from the
    // camera. We just need to check and see if each point intersects the
    // plane within it's extents.
    NiPoint3 kCenter = kBound.GetCenter();
    float fRadius = kBound.GetRadius();

    NiPoint3 kTestPoint;

    // Bound offset up.
    kTestPoint = kCenter + fRadius * pkCamera->GetWorldUpVector();
    if (!TestExtent(pkCamera, kTestPoint))
        return false;

    // Bound offset down.
    kTestPoint = kCenter - fRadius * pkCamera->GetWorldUpVector();
    if (!TestExtent(pkCamera, kTestPoint))
        return false;

    // Bound offset right.
    kTestPoint = kCenter + fRadius * pkCamera->GetWorldRightVector();
    if (!TestExtent(pkCamera, kTestPoint))
        return false;

    // Bound offset left.
    kTestPoint = kCenter - fRadius * pkCamera->GetWorldRightVector();
    if (!TestExtent(pkCamera, kTestPoint))
        return false;

    // If we reach here, then all the offset bound points are within the
    // plane extents so the object is occluded.
    return true;
}
//---------------------------------------------------------------------------
bool PlanarOccluder::TestExtent(NiCamera* pkCamera,
    const NiPoint3& kTestPoint) 
{
    // Find intersection with the geometric occlusion plane.
    NiPoint3 kCameraLoc = pkCamera->GetWorldTranslate();
    NiPoint3 kPlaneNormal = m_kOcclusionPlane.GetNormal();
    float fPlaneConstant = m_kOcclusionPlane.GetConstant();

    NiPoint3 kTemp = (kTestPoint - kCameraLoc);
    float fBottomTerm = kTemp.Dot(kPlaneNormal);

    if (fBottomTerm < 0.001f && fBottomTerm > -0.001f)
        return true;

    float fTopTerm = fPlaneConstant - kPlaneNormal.Dot(kCameraLoc);

    float fU = fTopTerm / fBottomTerm;

    NiPoint3 kIntersection = kCameraLoc + fU * (kTestPoint - kCameraLoc);

    // Project a vector from the plane center to kIntersection onto right
    // and up vectors to make sure that the projection length is not 
    // greater than the stored magnitude. Since the world up and right 
    // are normalized, we can do this with a dot product.
    kTemp = kIntersection - m_kCenter;

    float fRightMag = kTemp.Dot(m_kWorldRight);
    // If this passes, then we're outside the geometric plane extents.
    if (NiAbs(fRightMag) > m_fRightMag)
        return false;

    float fUpMag = kTemp.Dot(m_kWorldUp);
    // If this passes, then we're outside the geometric plane extents.
    if (NiAbs(fUpMag) > m_fUpMag)
        return false;

    return true;
}
//---------------------------------------------------------------------------
void PlanarOccluder::FillData(NiGeometry* pkPlane)
{
    NiGeometryData* pkData = pkPlane->GetModelData();
    assert (pkData->GetVertexCount() >= 3);

    NiPoint3 kPoint[3];
    kPoint[0] = *pkData->GetVertices();
    kPoint[1] = *(pkData->GetVertices() + 1);
    kPoint[2] = *(pkData->GetVertices() + 2);

    kPoint[0] = pkPlane->GetWorldTransform() * kPoint[0];
    kPoint[1] = pkPlane->GetWorldTransform() * kPoint[1];
    kPoint[2] = pkPlane->GetWorldTransform() * kPoint[2];

    m_kWorldUp = kPoint[1] - kPoint[0];
    m_kWorldRight = kPoint[2] - kPoint[0];
    m_fUpMag = m_kWorldUp.Unitize() * 0.5f;
    m_fRightMag = m_kWorldRight.Unitize() * 0.5f;

    NiPoint3 kNormal = m_kWorldUp.Cross(m_kWorldRight);

    m_kOcclusionPlane = NiPlane(kNormal, kPoint[0]);

	m_kCenter = pkPlane->GetWorldBound().GetCenter();

	m_kWorldBound.SetCenterAndRadius(m_kCenter,sqrt(m_fRightMag*m_fRightMag+m_fUpMag*m_fUpMag));

}
//---------------------------------------------------------------------------
bool	PlanarOccluder::Merge(NiGeometry *pkPlane)
{
    NiGeometryData* pkData = pkPlane->GetModelData();
    assert (pkData->GetVertexCount() >= 3);

    NiPoint3 kPoint[3];
    kPoint[0] = *pkData->GetVertices();
    kPoint[1] = *(pkData->GetVertices() + 1);
    kPoint[2] = *(pkData->GetVertices() + 2);

    kPoint[0] = pkPlane->GetWorldTransform() * kPoint[0];
    kPoint[1] = pkPlane->GetWorldTransform() * kPoint[1];
    kPoint[2] = pkPlane->GetWorldTransform() * kPoint[2];

    NiPoint3 kWorldUp = kPoint[1] - kPoint[0];
    NiPoint3 kWorldRight = kPoint[2] - kPoint[0];

	//	kWorldUp and kWorldRight must be same with m_kWorldUp and m_kWorldRight

    float fUpMag = kWorldUp.Unitize() * 0.5f;
    float fRightMag = kWorldRight.Unitize() * 0.5f;

	NiPoint3 kCenter = pkPlane->GetWorldBound().GetCenter();

	NiPoint3	p3 = m_kCenter - m_kWorldRight*m_fRightMag + m_kWorldUp*m_fUpMag;	//	the left top corner of this occluder
	NiPoint3	p8 = kCenter + kWorldRight*fRightMag + kWorldUp*fUpMag;	//	the right top corner of the new occluder
	NiPoint3	p6 = m_kCenter + m_kWorldRight*m_fRightMag - m_kWorldUp*m_fUpMag;	//	the right bottom corner of this occluder

	NiPoint3	v1 = p8 - p3;
	v1.Unitize();
	float	fCos = v1.Dot(m_kWorldRight);

	fRightMag = (p8 - p3).Length()*fCos*0.5;

	v1 = p8 - p6;
	v1.Unitize();
	fCos = v1.Dot(m_kWorldUp);

	fUpMag = (p8 - p6).Length() * fCos*0.5;

	m_fRightMag = m_fRightMag>fRightMag ? m_fRightMag : fRightMag;
	m_fUpMag = m_fUpMag>fUpMag ? m_fUpMag : fUpMag;

	m_kCenter = (p3+m_kWorldRight*m_fRightMag-m_kWorldUp*m_fUpMag);
	m_kWorldBound.SetCenterAndRadius(m_kCenter,sqrt(m_fRightMag*m_fRightMag+m_fUpMag*m_fUpMag));

	return	true;
}
