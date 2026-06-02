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

#ifndef PLANAROCCLUDER_H
#define PLANAROCCLUDER_H

#include <NiCamera.h>
#include <NiPlane.h>
#include <NiPoint3.h>
#include <NiTransform.h>
#include <NiGeometry.h>

#include "Occluder.h"

class PlanarOccluder : public Occluder
{
    NiDeclareRTTI;

public:
    PlanarOccluder(NiGeometry* pkPlane);
    virtual ~PlanarOccluder();

    virtual void Update(NiCamera* pkCamera);
    virtual bool IsOccluded(NiCamera* pkCamera, const NiBound& kBound);
	virtual	bool	Merge(NiGeometry *pkGeom);

protected:
    // This protected constructor should only be called by the construction
    // of a subclass which can insure that it is set up correctly.
    PlanarOccluder();

    virtual void FillData(NiGeometry* pkPlane);
    virtual bool CheckPlaneExtents(NiCamera* pkCamera, const NiBound& kBound);
    virtual bool TestExtent(NiCamera* pkCamera, const NiPoint3& kTestPoint);

	NiPoint3	m_kCenter;
	NiBound	m_kWorldBound;
    NiPlane m_kOcclusionPlane;
    NiPoint3 m_kWorldUp;
    NiPoint3 m_kWorldRight;
    float m_fUpMag;
    float m_fRightMag;
    int m_iCameraWhichSide;
};

NiSmartPointer(PlanarOccluder);

#endif // PLANAROCCLUDER_H 
