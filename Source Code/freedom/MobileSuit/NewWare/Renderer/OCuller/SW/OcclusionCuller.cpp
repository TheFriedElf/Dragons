
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionCuller.cpp
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
#include "OcclusionCuller.h"

#include "PlanarOccluder.h"

#include <NiCamera.h>
#include <NiGeometry.h>
#include <NiNode.h>
#include <NiEntity.h>


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

OcclusionCuller::OcclusionCuller()
    : NiCullingProcess(NULL, false)
{
    m_kSceneRootPointerName = "Scene Root Pointer";
}

//-----------------------------------------------------------------------------------

OcclusionCuller::~OcclusionCuller()
{
    RemoveAllOcclusionGeometry();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionCuller::Process( NiCamera const* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet )
{
    // Determine which side of the occlusion planes the camera is on for
    // fast dismissal during occlusion testing.
    unsigned int uiNumOccluders = m_kOccluders.GetEffectiveSize();
    for ( unsigned int ui = 0; ui < uiNumOccluders; ++ui )
    {
        Occluder* pkRecord = m_kOccluders.GetAt( ui );
        pkRecord->Update( (NiCamera*)pkCamera );
    }

    NiCullingProcess::Process( pkCamera, pkScene, pkVisibleSet );
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::Process( NiAVObject* pkObject )
{
    // Cull by occlusion objects first. Then proceed with normal processing
    // if object is not culled.

    unsigned int uiNumOccluders = m_kOccluders.GetEffectiveSize();
    // Test each anti-portal
    for ( unsigned int ui = 0; ui < uiNumOccluders; ++ui )
    {
        Occluder* pkRecord = m_kOccluders.GetAt( ui );
        if ( pkRecord->IsOccluded((NiCamera*)m_pkCamera, pkObject->GetWorldBound()) )
        {
            return;
        }
    }   

    NiCullingProcess::Process( pkObject );
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::RecursiveFindOcclusionGeometry( NiEntityInterface* pkEntity )
{
    if ( !pkEntity )
        return;

    unsigned int uiSceneRootCount;
    if ( pkEntity->GetElementCount(m_kSceneRootPointerName, uiSceneRootCount) )
    {
        for ( unsigned int ui = 0; ui < uiSceneRootCount; ++ui )
        {
            NiObject* pkObject;
            NIVERIFY( pkEntity->GetPropertyData(m_kSceneRootPointerName, pkObject, ui) );

            NiAVObject* pkSceneRoot = NiDynamicCast(NiAVObject, pkObject);
            if ( pkSceneRoot )
            {
                FindOcclusionGeometry( pkSceneRoot, false );
            }
        }
    }
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::FindOcclusionGeometry( NiScene* pkEntityScene )
{
    for ( unsigned int ui = 0; ui < pkEntityScene->GetEntityCount(); ++ui )
    {
        RecursiveFindOcclusionGeometry( pkEntityScene->GetEntityAt(ui) );
    }
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::FindOcclusionGeometry( NiAVObject* pkAVObject, bool bMarkedParent )
{
    if ( NiIsKindOf(NiGeometry, pkAVObject) )
    {
        if ( bMarkedParent || pkAVObject->GetName() && 
             strstr(pkAVObject->GetName(), "PlanarOccluder") )
        {
            Occluder* pkRecord = NiNew PlanarOccluder( (NiGeometry*)pkAVObject );
            m_kOccluders.AddFirstEmpty( pkRecord );
        }
    }
    else if ( NiIsKindOf(NiNode, pkAVObject) )
    {
        bool bMarked = false;
        if ( pkAVObject->GetName() && strstr(pkAVObject->GetName(), "Occluder") )
        {
            bMarked = true;
        }

        NiNode* pkNode = (NiNode*)pkAVObject;

        unsigned int uiChildren = pkNode->GetArrayCount();
        for ( unsigned int ui = 0; ui < uiChildren; ++ui )
        {
            NiAVObject* pkChild = pkNode->GetAt( ui );
            if ( pkChild )
                FindOcclusionGeometry( pkChild, bMarked );
        }
    }
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::RemoveAllOcclusionGeometry()
{
    for ( unsigned int ui = 0; ui < m_kOccluders.GetSize(); ++ui )
        NiDelete m_kOccluders.GetAt( ui );

    m_kOccluders.RemoveAll();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace SW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
