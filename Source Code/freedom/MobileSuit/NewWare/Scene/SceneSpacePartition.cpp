
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneSpacePartition.cpp
// Description : Simple Scene Space Partitioning - Loding time.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#include "stdafx.h"
#include "SceneSpacePartition.h"

#include "ApplyTraversal.h"


namespace NewWare
{

namespace Scene
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

SceneSpacePartition::SceneSpacePartition()
    : m_uiChildNodeCount(DEFAULT_CHILDNODE_COUNT)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

SceneSpacePartition::~SceneSpacePartition()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SceneSpacePartition::Build( NiNode* pkAttachTo, NiNode*& pkSrcNode )
{
    assert( pkAttachTo && pkSrcNode );

    pkSrcNode->UpdateNodeBound();
    pkSrcNode->Update( 0.0f );
    NiBound const& kBound = pkSrcNode->GetWorldBound();
#ifndef EXTERNAL_RELEASE
    Tools::OutputDebugFile( "[BOUND] CENTER:%f,%f,%f | RADIUS:%f\n", 
            kBound.GetCenter().x, kBound.GetCenter().y, kBound.GetCenter().z, kBound.GetRadius() );
#endif //#ifndef EXTERNAL_RELEASE

    m_uiChildNodeCount = (unsigned int)((kBound.GetRadius() * 2.0f) / X_SECTOR_RANGE);

    NiNodePtr spBuildNode = CreateBuildNode( "STATIC_CHILD_NODE" );

    DoBuildNode( spBuildNode, pkSrcNode, kBound );

    NiNodePtr spNewNode = OrganizeNode( spBuildNode, "STATIC_NODE_ROOT" );
    pkAttachTo->AttachChild( spNewNode, true );

    NiDelete pkSrcNode;
    pkSrcNode = spNewNode;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiNodePtr SceneSpacePartition::CreateBuildNode( char const* nodeName ) const
{
    assert( nodeName );

    // Create build node. (X/Y scene sector)
    NiNode* pkBuildNode = NiNew NiNode;
    for ( unsigned int ui = 0; ui < (m_uiChildNodeCount * Y_SECTOR_COUNT); ++ui )
    {
        NiNode* pkChildNode = NiNew NiNode;
        pkChildNode->SetName( nodeName );

        pkBuildNode->AttachChild( pkChildNode, false );
    }
    return pkBuildNode;
}

//-----------------------------------------------------------------------------------

void SceneSpacePartition::DoBuildNode( NiNode* pkBuildNode, NiNode const* pkSrcNode, NiBound const& kBound ) const
{
    assert( pkBuildNode && pkSrcNode );

    NiPoint3 const& kCenter = kBound.GetCenter();
    float const fXRange = (kBound.GetRadius() * 2.0f) / m_uiChildNodeCount;
    float const fYRange = (kBound.GetRadius() * 2.0f) / Y_SECTOR_COUNT;
    float const fFirstStartX = kCenter.x - kBound.GetRadius();
    float const fFirstStartY = kCenter.y - kBound.GetRadius();

    // Build Node.
    for ( unsigned int ui = 0; ui < pkSrcNode->GetArrayCount(); ++ui )
    {
        NiAVObject* pkObject = pkSrcNode->GetAt( ui );

        unsigned int uiChildX = 0, uiChildY = 0;
        GetChildNodeIndex( uiChildX, uiChildY, pkObject->GetWorldTranslate(), 
                           fXRange, fYRange, fFirstStartX, fFirstStartY );

        NiNode* pkChildRoot = (NiNode*)pkBuildNode->GetAt( uiChildX + (m_uiChildNodeCount * uiChildY) );
        pkChildRoot->AttachChild( pkObject, false );

#ifndef EXTERNAL_RELEASE
        Tools::OutputDebugFile( "[%4d] ChildNode:%2d,%2d | Object:%.4f\n", 
                                ui, uiChildX, uiChildY, pkObject->GetWorldTranslate().x );
#endif //#ifndef EXTERNAL_RELEASE
    }
}

//-----------------------------------------------------------------------------------

NiNodePtr SceneSpacePartition::OrganizeNode( NiNode const* pkBuildNode, char const* nodeName ) const
{
    assert( pkBuildNode && nodeName );

    // Clearing empty node and organization.
    NiNode* pkNewNode = NiNew NiNode;
    assert( pkNewNode );
    pkNewNode->SetName( nodeName );

    unsigned int uiChildCount;
    for ( unsigned int ui = 0; ui < pkBuildNode->GetArrayCount(); ++ui )
    {
        NiNode* pkNode = (NiNode*)pkBuildNode->GetAt( ui );
        uiChildCount = pkNode->GetChildCount();

        if ( uiChildCount > 3 )
        {
            pkNewNode->AttachChild( pkNode, false );
        }
        else if ( uiChildCount > 0 )
        {
            for ( unsigned int uj = 0; uj < uiChildCount; ++uj )
                pkNewNode->AttachChild( pkNode->GetAt(uj), false );
        }
    }

    // Clearing physx node.
    NiNodePtr spGarbageCollector = NiNew NiNode;
    ApplyTraversal::Node::Detach( pkNewNode, "physx", spGarbageCollector );

    return pkNewNode;
}

//-----------------------------------------------------------------------------------

bool SceneSpacePartition::GetChildNodeIndex( unsigned int& uiChildX, unsigned int& uiChildY, 
                                             NiPoint3 const& kPos, float fXRange, float fYRange, 
                                             float fFirstStartX, float fFirstStartY ) const
{
    for ( unsigned int uiY = 0; uiY < Y_SECTOR_COUNT; ++uiY )
    {
        float const fStartY = fFirstStartY + (fYRange *  uiY);
        float const fEndY   = fFirstStartY + (fYRange * (uiY+1));

        if ( fStartY <= kPos.y && fEndY >= kPos.y )
        {
            uiChildY = uiY;

            for ( unsigned int uiX = 0; uiX < m_uiChildNodeCount; ++uiX )
            {
                float const fStartX = fFirstStartX + (fXRange *  uiX);
                float const fEndX   = fFirstStartX + (fXRange * (uiX+1));

                if ( fStartX <= kPos.x && fEndX >= kPos.x )
                {
                    uiChildX = uiX;
                    return true;
                }
            }
        }
    }

#ifndef EXTERNAL_RELEASE
    Tools::OutputDebugFile( "========= ERROR =============== \n" );
#endif //#ifndef EXTERNAL_RELEASE
    return false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OutputDebugSceneWorldBound( NiNode* pkScene )
{
#ifndef EXTERNAL_RELEASE
    assert( pkScene );

    for ( unsigned int ui = 0; ui < pkScene->GetArrayCount(); ++ui )
    {
        NiAVObject* pkObject = pkScene->GetAt( ui );
        if ( pkObject )
        {
            NiBound const& kBound = pkObject->GetWorldBound();
            Tools::OutputDebugFile( "[%3d-%3d] x:%.4f, y:%.4f, z:%.4f | r:%.4f\n", ui, 
                                    (NiIsExactKindOf(NiNode, pkObject)? ((NiNode*)pkObject)->GetChildCount(): 0), 
                                    kBound.GetCenter().x, kBound.GetCenter().y, kBound.GetCenter().z, 
                                    kBound.GetRadius() );
        }
    }
#endif //#ifndef EXTERNAL_RELEASE
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Scene

} //namespace NewWare
