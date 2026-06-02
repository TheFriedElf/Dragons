
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DomainNodeManager.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/19 LeeJR First Created
//

#include "stdafx.h"
#include "DomainNodeManager.h"

#include "ApplyProperty.h"
#include "../../PgPSRoomGroup.h"


namespace NewWare
{

namespace Scene
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

char const* const DomainNodeManager::SCENEROOT_NODE_NAME = "Scene Root";
char const* const DomainNodeManager::STATIC_NODE_NAME = "STATIC_NODE_ROOT";
char const* const DomainNodeManager::DYNAMIC_NODE_NAME = "DYNAMIC_NODE_ROOT";
char const* const DomainNodeManager::ROOMGROUP_NODE_NAME = "ROOMGROUP_NODE_ROOT";
char const* const DomainNodeManager::SELECTIVE_NODE_NAME = "SELECTIVE_NODE_ROOT";

//-----------------------------------------------------------------------------------

char const* const DomainNodeManager::DOMAIN_NODE_NAME[DOMAIN_NODE_TOTALCOUNT][2] = { 
    {"Path", "paths"}, 
    {"PhysX", "physx"}, 
    {"CharacterSpawn", "char_spawns"}, 
    {"CameraWalls", "camera_walls"}, 
    {"SkyBox", "sky_boxes"}, 
    {"Trigger", "triggers"}, 
    {"Trap", "traps"}, 
    {"Water", "waters"}, 
    {"Ladder", "ladders"}, 
    {"Rope", "ropes"}, 
    {"Room", "rooms"}, 
    {"Optimization0", "optimize_0"}, 
    {"Optimization1", "optimize_1"}, 
    {"Optimization2", "optimize_2"}, 
    {"PermissionArea", "permission_area"}, 
};

//-----------------------------------------------------------------------------------

DomainNodeManager::DomainNodeManager()
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

DomainNodeManager::~DomainNodeManager()
{
    Destroy();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void DomainNodeManager::Create()
{
    Destroy();

    BuildRootNode();
    BuildDomainNode();
    BuildDynamicNodeEntity();

    UpdateToNode( m_spSceneRoot, true, true, true, true );
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::Destroy()
{
    m_spSceneRoot = 0;

    m_spStaticNodeRoot = 0;
    m_spDynamicNodeRoot = 0;
    m_spRoomGroupNodeRoot = 0;
    m_spSelectiveNodeRoot = 0;

    m_aspDomainNode.assign( 0 );
    m_aspDynamicNodeEntity.assign( 0 );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void DomainNodeManager::BuildRootNode()
{
    assert( 0 == m_spSceneRoot && 0 == m_spStaticNodeRoot && 0 == m_spDynamicNodeRoot && 
            0 == m_spRoomGroupNodeRoot && 0 == m_spSelectiveNodeRoot );

    m_spSceneRoot = NiNew NiNode;
    m_spSceneRoot->SetName( SCENEROOT_NODE_NAME );

    m_spStaticNodeRoot = NiNew NiNode;
    m_spStaticNodeRoot->SetName( STATIC_NODE_NAME );
    m_spSceneRoot->AttachChild( m_spStaticNodeRoot, true );

    m_spDynamicNodeRoot = NiNew NiNode;
    m_spDynamicNodeRoot->SetName( DYNAMIC_NODE_NAME );
    m_spSceneRoot->AttachChild( m_spDynamicNodeRoot, true );

    m_spRoomGroupNodeRoot = NiNew PgPSRoomGroup;
    m_spRoomGroupNodeRoot->SetName( ROOMGROUP_NODE_NAME );
    m_spSceneRoot->AttachChild( m_spRoomGroupNodeRoot, true );

    m_spSelectiveNodeRoot = NiNew NiNode;
    m_spSelectiveNodeRoot->SetName( SELECTIVE_NODE_NAME );
    m_spSceneRoot->AttachChild( m_spSelectiveNodeRoot, true );
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::BuildDomainNode()
{
    DomainNode eIdx;

    for ( unsigned int ui = 0; ui < DOMAIN_NODE_TOTALCOUNT; ++ui )
    {
        eIdx = (DomainNode)ui;
        assert( 0 == m_aspDomainNode[eIdx] );

        m_aspDomainNode[eIdx] = NiDynamicCast(NiNode, m_spSceneRoot->GetObjectByName(GetNameString(eIdx)));
        if ( 0 == m_aspDomainNode[eIdx] )
        {
            m_aspDomainNode[eIdx] = NiNew NiNode;
            m_aspDomainNode[eIdx]->SetName( GetNameString(eIdx) );
            m_spSceneRoot->AttachChild( m_aspDomainNode[eIdx], true );
        }

        ApplyInvisibleToDomainNode( eIdx, m_aspDomainNode[eIdx] );
        AttatchDummyNodeToDomainNode( eIdx, m_aspDomainNode[eIdx] );
        TweakPropertyAtDomainNode( eIdx, m_aspDomainNode[eIdx] );
    }
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::BuildDynamicNodeEntity()
{
    assert( m_spDynamicNodeRoot );

    for ( unsigned int ui = 0; ui < DYNAMIC_NODE_ENTITY_TOTALCOUNT; ++ui )
    {
        assert( m_aspDynamicNodeEntity[ui] );
        m_aspDynamicNodeEntity[ui] = NiNew NiNode;
        char szName[128];
        sprintf_s( szName, _countof(szName), "object_group_%d", ui );
        m_aspDynamicNodeEntity[ui]->SetName( szName );
        m_spDynamicNodeRoot->AttachChild( m_aspDynamicNodeEntity[ui], true );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void DomainNodeManager::ApplyInvisibleToDomainNode( DomainNode eIdx, NiNode* pkNode )
{
    assert( pkNode && eIdx < DOMAIN_NODE_TOTALCOUNT );

    switch ( eIdx )
    {
    case DOMAIN_NODE_PATH:
    case DOMAIN_NODE_PHYSX:
    case DOMAIN_NODE_CHARACTER_SPAWN:
    case DOMAIN_NODE_CAMERA_WALLS:
    //case DOMAIN_NODE_TRIGGER:
    //case DOMAIN_NODE_LADDER:
    //case DOMAIN_NODE_OPTIMIZATION_0:
    //case DOMAIN_NODE_OPTIMIZATION_1:
    //case DOMAIN_NODE_OPTIMIZATION_2:
    case DOMAIN_NODE_PERMISSION_AREA:
        pkNode->SetAppCulled( true );
        break;
    }
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::AttatchDummyNodeToDomainNode( DomainNode eIdx, NiNode* pkNode )
{
    assert( pkNode && eIdx < DOMAIN_NODE_TOTALCOUNT );

    switch ( eIdx )
    {
    case DOMAIN_NODE_PHYSX:
        if ( pkNode && pkNode->GetArrayCount() == 0 )
        {
            // @note - GSA맵 제작 특성상 PhysX의 Domain Node의 자식노드가 없을수 있기 
            //         때문에 Dummy Node를 추가해서 부쳐준다.
            pkNode->AttachChild( NiNew NiNode, true ); // [FIXME] ?????
        }
        break;
    }
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::TweakPropertyAtDomainNode( DomainNode eIdx, NiNode* pkNode )
{
    assert( pkNode && eIdx < DOMAIN_NODE_TOTALCOUNT );

    switch ( eIdx )
    {
    case DOMAIN_NODE_SKYBOX:
        ApplyProperty::Override::EnableZBufferProperty( pkNode, false, false, false );
        break;
    }
}

//-----------------------------------------------------------------------------------

void DomainNodeManager::UpdateToNode( NiNode* pkNode, bool bCtrls, bool bProps, bool bNode, bool bEffects )
{
    assert( pkNode );

    pkNode->Update( 0.0f, bCtrls );
    if ( bNode )
        pkNode->UpdateNodeBound();
    if ( bProps )
        pkNode->UpdateProperties();
    if ( bEffects )
        pkNode->UpdateEffects();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Scene

} //namespace NewWare
