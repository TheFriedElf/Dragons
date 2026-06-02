
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DomainNodeManager.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/19 LeeJR First Created
//

#ifndef _SCENE_DOMAINNODEMANAGER_H__
#define _SCENE_DOMAINNODEMANAGER_H__

#include <array>

class PgPSRoomGroup;


namespace NewWare
{

namespace Scene
{


/** Node Hierarchical Design Diagram
    SceneRoot -+- Static Node Root
               +- Dynamic Node Root - Dynamic Node Entities(+12)
               +- Room Group Node Root
               +- Selective Node Root
               +- Domain Nodes(+15)
*/
class DomainNodeManager
{
public:
    enum DomainNode
    {
        DOMAIN_NODE_PATH = 0, 
        DOMAIN_NODE_PHYSX, 
        DOMAIN_NODE_CHARACTER_SPAWN, 
        DOMAIN_NODE_CAMERA_WALLS, 
        DOMAIN_NODE_SKYBOX, 
        DOMAIN_NODE_TRIGGER, 
        DOMAIN_NODE_TRAP, 
        DOMAIN_NODE_WATER, 
        DOMAIN_NODE_LADDER, 
        DOMAIN_NODE_ROPE, 
        DOMAIN_NODE_ROOM, 
        DOMAIN_NODE_OPTIMIZATION_0, 
        DOMAIN_NODE_OPTIMIZATION_1, 
        DOMAIN_NODE_OPTIMIZATION_2, 
        DOMAIN_NODE_PERMISSION_AREA, 

        DOMAIN_NODE_TOTALCOUNT
    };

    enum DynamicNodeEntity
    {
        DYNAMIC_NODE_ENTITY_PLAYER = 0, 
        DYNAMIC_NODE_ENTITY_MONSTER, 
        DYNAMIC_NODE_ENTITY_GROUNDBOX, 
        DYNAMIC_NODE_ENTITY_PET, 
        DYNAMIC_NODE_ENTITY_NPC, 
        DYNAMIC_NODE_ENTITY_PUPPET, 
        DYNAMIC_NODE_ENTITY_ENTITY, // 유닛에게 속한 소환 객체
        DYNAMIC_NODE_ENTITY_SHINESTONE, 
        DYNAMIC_NODE_ENTITY_OBJECT, // 월드상의 반응하는 물건
        DYNAMIC_NODE_ENTITY_EFFECT, 
        DYNAMIC_NODE_ENTITY_FURNITURE, 
        DYNAMIC_NODE_ENTITY_MYHOME, 

        DYNAMIC_NODE_ENTITY_TOTALCOUNT
    };


public:
    DomainNodeManager();
    ~DomainNodeManager();


    void Create();
    void Destroy();


    NiNode* GetSceneRoot() const { assert( m_spSceneRoot ); return m_spSceneRoot; };
    void AttatchToSceneRoot( NiAVObject* pkAVObject, bool bFirstAvail = false, 
                             bool bUpdateAll = false ) { 
        assert( pkAVObject );
        m_spSceneRoot->AttachChild( pkAVObject, bFirstAvail );
        if ( bUpdateAll )
            UpdateToNode( m_spSceneRoot, true, true, true, true );
    }

    NiNode* GetStaticNodeRoot() const { assert( m_spStaticNodeRoot ); return m_spStaticNodeRoot; };
    NiNode* GetDynamicNodeRoot() const { assert( m_spDynamicNodeRoot ); return m_spDynamicNodeRoot; };
    NiNode* GetSelectiveNodeRoot() const { assert( m_spSelectiveNodeRoot ); return m_spSelectiveNodeRoot; };
    PgPSRoomGroup* GetRoomGroupNodeRoot() const 
        { assert( m_spRoomGroupNodeRoot ); return NiSmartPointerCast(PgPSRoomGroup, m_spRoomGroupNodeRoot); };


    NiNode* GetDomainNode( DomainNode eKind ) const { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT && m_aspDomainNode[eKind] );
        return m_aspDomainNode[eKind];
    }
    void AttatchToDomainNode( DomainNode eKind, NiAVObject* pkAVObject, bool bFirstAvail = false, 
                              bool bUpdateAll = false ) { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT );
        GetDomainNode(eKind)->AttachChild( pkAVObject, bFirstAvail );
        if ( bUpdateAll )
            UpdateToDomainNode( eKind, true, true, true, true );
    }
    void DetachToDomainNode( DomainNode eKind, NiAVObject* pkAVObject, bool bUpdateAll = false ) { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT );
        GetDomainNode(eKind)->DetachChild( pkAVObject );
        if ( bUpdateAll )
            UpdateToDomainNode( eKind, true, true, true, true );
    }
    void UpdateToDomainNode( DomainNode eKind, bool bCtrls = true, bool bProps = true, 
                             bool bNode = true, bool bEffects = true ) { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT );
        UpdateToNode( GetDomainNode(eKind), bCtrls, bProps, bNode, bEffects );
    }


    NiNode* GetDynamicNodeEntity( DynamicNodeEntity eKind ) const { 
        assert( eKind < DYNAMIC_NODE_ENTITY_TOTALCOUNT && m_aspDynamicNodeEntity[eKind] );
        return m_aspDynamicNodeEntity[eKind];
    }
    void AttatchToDynamicNodeEntity( DynamicNodeEntity eKind, NiAVObject* pkAVObject, 
                                     bool bFirstAvail = false, bool bUpdateAll = false ) { 
        assert( eKind < DYNAMIC_NODE_ENTITY_TOTALCOUNT );
        GetDynamicNodeEntity(eKind)->AttachChild( pkAVObject, bFirstAvail );
        if ( bUpdateAll )
            UpdateToDynamicNodeEntity( eKind, true, true, true, true );
    }
    void DetachToDynamicNodeEntity( DynamicNodeEntity eKind, NiAVObject* pkAVObject, 
                                    bool bUpdateAll = false ) { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT );
        GetDynamicNodeEntity(eKind)->DetachChild( pkAVObject );
        if ( bUpdateAll )
            UpdateToDynamicNodeEntity( eKind, true, true, true, true );
    }
    void UpdateToDynamicNodeEntity( DynamicNodeEntity eKind, bool bCtrls = true, bool bProps = true, 
                                    bool bNode = true, bool bEffects = true ) { 
        assert( eKind < DOMAIN_NODE_TOTALCOUNT );
        UpdateToNode( GetDynamicNodeEntity(eKind), bCtrls, bProps, bNode, bEffects );
    }


protected:
    void BuildRootNode();
    void BuildDomainNode();
    void BuildDynamicNodeEntity();

    void ApplyInvisibleToDomainNode( DomainNode eIdx, NiNode* pkNode );
    void AttatchDummyNodeToDomainNode( DomainNode eIdx, NiNode* pkNode );
    void TweakPropertyAtDomainNode( DomainNode eIdx, NiNode* pkNode );

    char const* const GetTypeString( DomainNode eIdx ) const 
        { assert( eIdx < DOMAIN_NODE_TOTALCOUNT ); return DOMAIN_NODE_NAME[eIdx][0]; };
    char const* const GetNameString( DomainNode eIdx ) const 
        { assert( eIdx < DOMAIN_NODE_TOTALCOUNT ); return DOMAIN_NODE_NAME[eIdx][1]; };

    void UpdateToNode( NiNode* pkNode, bool bCtrls = true, bool bProps = true, 
                       bool bNode = true, bool bEffects = true );


private:
    NiNodePtr m_spSceneRoot;

    NiNodePtr m_spStaticNodeRoot;
    NiNodePtr m_spDynamicNodeRoot;
    NiNodePtr m_spRoomGroupNodeRoot;
    NiNodePtr m_spSelectiveNodeRoot;

    std::array<NiNodePtr, DOMAIN_NODE_TOTALCOUNT> m_aspDomainNode;
    std::array<NiNodePtr, DYNAMIC_NODE_ENTITY_TOTALCOUNT> m_aspDynamicNodeEntity;

    static char const* const SCENEROOT_NODE_NAME;
    static char const* const STATIC_NODE_NAME;
    static char const* const DYNAMIC_NODE_NAME;
    static char const* const ROOMGROUP_NODE_NAME;
    static char const* const SELECTIVE_NODE_NAME;

    static char const* const DOMAIN_NODE_NAME[DOMAIN_NODE_TOTALCOUNT][2];
};


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_DOMAINNODEMANAGER_H__
