
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionSorter.cpp
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
#include "OcclusionSorter.h"

#include "OcclusionTimeController.h"

#include <NiRenderer.h>


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

OcclusionSorter::OcclusionSorter()
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

OcclusionSorter::~OcclusionSorter()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool OcclusionSorter::ResolveType( NiGeometry& kGeometry, NiCamera const* pkCamera )
{
    assert( pkCamera );

    return ResolveOcclusionType( kGeometry, pkCamera );
}

//-----------------------------------------------------------------------------------

void OcclusionSorter::DoClear()
{
    // DO NOT delete the NiGeometries!
    while ( m_kOccludeRList.GetSize() )
        m_kOccludeRList.RemoveHead();
    while ( m_kOccludeEList.GetSize() )
        m_kOccludeEList.RemoveHead();
}

//-----------------------------------------------------------------------------------

void OcclusionSorter::DoAccumulate( NiGeometry* pkItem )
{
    assert( pkItem );

    NiIntegerExtraData* pkExtraData = 
        static_cast<NiIntegerExtraData*>(pkItem->GetExtraData(OcclusionTimeController::OCCLUDER_NAME));
    assert( pkExtraData );

    if ( pkExtraData->GetValue() & HW::OcclusionFlag::OCCLUDER )
        m_kOccludeRList.AddTail( pkItem );
    if ( pkExtraData->GetValue() & HW::OcclusionFlag::OCCLUDEE )
        m_kOccludeEList.AddTail( pkItem );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
