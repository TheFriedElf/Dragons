
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : PlanarOccluder.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#include "stdafx.h"
#include "PlanarOccluder.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace SW
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

NiImplementRTTI(PlanarOccluder, Occluder);

//-----------------------------------------------------------------------------------

PlanarOccluder::PlanarOccluder()
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

PlanarOccluder::PlanarOccluder( NiGeometry* pkPlane )
    : m_spGeometry(pkPlane), 
      m_iCameraWhichSide(NiPlane::NO_SIDE)
{
    assert( pkPlane != NULL );
    FillData();
}

//-----------------------------------------------------------------------------------

PlanarOccluder::~PlanarOccluder()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void PlanarOccluder::Update( NiCamera* pkCamera )
{
    // Determine which side of the occlusion planes the camera is on for
    // fast dismissal during occlusion testing.
    if ( m_kLastTransform != m_spGeometry->GetWorldTransform() )
        FillData();

    NiPoint3 kCameraLoc = pkCamera->GetWorldLocation();
    m_iCameraWhichSide = m_kOcclusionPlane.WhichSide( kCameraLoc );

    // Generate the four world-space points of the occluder plane.
    NiPoint3 kCenter = m_spGeometry->GetWorldBound().GetCenter();
    NiPoint3 kR = m_fRightMag * m_kWorldRight;
    NiPoint3 kUR = kR + m_fUpMag * m_kWorldUp;
    NiPoint3 kLR = kR - m_fUpMag * m_kWorldUp;
    NiPoint3 akSidePoints[4] =
    {
        kCenter + kUR,
        kCenter + kLR,
        kCenter - kUR,
        kCenter - kLR
    };

    // akSideVec[i] == (akSidePoints[(i+1)%4]-akSidePoints[i]).Unitize()
    NiPoint3 akSideVecs[4] =
    {
        -m_kWorldUp,
        -m_kWorldRight,
        m_kWorldUp,
        m_kWorldRight
    };

    NiPoint3 kCameraVec = pkCamera->GetWorldDirection();

    // If the plane is mostly perpendicular to the camera...
    if ( m_iCameraWhichSide == NiPlane::NO_SIDE || 
         NiAbs(kCameraVec.Dot(m_kOcclusionPlane.GetNormal())) < 0.001f )
    {
        m_iCameraWhichSide = NiPlane::NO_SIDE;
        return;
    }

    // Generate frustum planes.  Each frustum plane contains one side
    // of the occluder plane and the camera dir.  These planes will all
    // face out if and only if the occlusion plane faces out (to the camera).
    bool bPerspective = !pkCamera->GetViewFrustum().m_bOrtho;
    for ( unsigned int i = 0; i < 4; ++i )
    {
        if ( bPerspective )
        {
            kCameraVec = akSidePoints[i] - kCameraLoc;
        }

        NiPoint3 kNorm = kCameraVec.UnitCross( akSideVecs[i] );
        m_akSides[i].SetNormal( kNorm );
        m_akSides[i].SetConstant( kNorm.Dot(akSidePoints[i]) );

        // Assert normals of the planes are all pointing the same way 
        // (in or out) as the occlusion plane. This will simplify 
        // calculations later.  This assertion may trigger if the logic
        // calculating the normal in FillData() has changed.
        assert( m_akSides[i].WhichSide(akSidePoints[(i+2)%4]) != m_iCameraWhichSide );
    }
}

//-----------------------------------------------------------------------------------

bool PlanarOccluder::IsOccluded( NiCamera* /*pkCamera*/, NiBound const& kBound )
{
    NiPoint3 kTestLocation;

    if ( m_iCameraWhichSide == NiPlane::NO_SIDE )
        return false;

    // If the plane is facing out (camera is on the positive side), then
    // bias the point along the normal and vice versa.
    float fRadius = NiSelect( m_iCameraWhichSide == NiPlane::POSITIVE_SIDE, 
                              kBound.GetRadius(), -kBound.GetRadius() );
    NiPoint3 kCenter = kBound.GetCenter();

    // Test if the bound crosses the occlusion plane
    kTestLocation = kCenter + fRadius * m_kOcclusionPlane.GetNormal();
    if ( m_kOcclusionPlane.WhichSide(kTestLocation) == m_iCameraWhichSide )
        return false;

    bool bCulled = true;
    for ( unsigned int i = 0; i < 4 && bCulled; i++ )
    {
        NiPoint3 kTestPoint = kCenter + fRadius * m_akSides[i].GetNormal();

        // If the furthest extent of the sphere in the direction of this plane
        // is not on the same side as the camera (i.e. crosses the plane), 
        // then it is culled by the occluder.
        bCulled &= m_akSides[i].WhichSide(kTestPoint) != m_iCameraWhichSide;
    }

    // If we reach here and bCulled is true, then the all four frustum planes
    // and the infinite occlusion plane have occluded the geometry.
    return bCulled;
}

//-----------------------------------------------------------------------------------

void PlanarOccluder::FillData()
{
    NiGeometryData* pkData = m_spGeometry->GetModelData();
    assert( pkData && pkData->GetVertexCount() >= 3 );

    NiPoint3 kPoint[3];
    kPoint[0] = *(pkData->GetVertices());
    kPoint[1] = *(pkData->GetVertices() + 1);
    kPoint[2] = *(pkData->GetVertices() + 2);

    kPoint[0] = m_spGeometry->GetWorldTransform() * kPoint[0];
    kPoint[1] = m_spGeometry->GetWorldTransform() * kPoint[1];
    kPoint[2] = m_spGeometry->GetWorldTransform() * kPoint[2];

    m_kWorldUp    = kPoint[1] - kPoint[0];
    m_kWorldRight = kPoint[2] - kPoint[0];
    m_fUpMag      = m_kWorldUp.Unitize() * 0.5f;
    m_fRightMag   = m_kWorldRight.Unitize() * 0.5f;

    NiPoint3 kNormal = m_kWorldUp.Cross( m_kWorldRight );

    m_kOcclusionPlane = NiPlane( kNormal, kPoint[0] );

    m_kLastTransform = m_spGeometry->GetWorldTransform();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace SW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
