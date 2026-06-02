
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : PlanarOccluder.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_SW_PLANAROCCLUDER_H__
#define _RENDERER_OCULLER_SW_PLANAROCCLUDER_H__

#include <NiCamera.h>
#include <NiPlane.h>
#include <NiPoint3.h>
#include <NiTransform.h>
#include <NiGeometry.h>

#include "Occluder.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace SW
{


class PlanarOccluder : public Occluder
{
    NiDeclareRTTI;

public:
    PlanarOccluder( NiGeometry* pkPlane );
    virtual ~PlanarOccluder();


    virtual void Update( NiCamera* pkCamera );
    virtual bool IsOccluded( NiCamera* /*pkCamera*/, NiBound const& kBound );

    NiGeometry* GetGeometry() const { return m_spGeometry; };


protected:
    // This protected constructor should only be called by the construction
    // of a subclass which can insure that it is set up correctly.
    PlanarOccluder();

    virtual void FillData();


protected:
    NiGeometryPtr m_spGeometry;
    NiTransform m_kLastTransform;
    NiPlane m_kOcclusionPlane;
    NiPoint3 m_kWorldUp;
    NiPoint3 m_kWorldRight;
    float m_fUpMag;
    float m_fRightMag;
    int m_iCameraWhichSide;
    NiPlane m_akSides[4];
};

NiSmartPointer(PlanarOccluder);


} //namespace SW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_SW_PLANAROCCLUDER_H__
