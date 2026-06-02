
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneSpacePartition.h
// Description : Simple Scene Space Partitioning - Loding time.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#ifndef _SCENE_SCENESPACEPARTITION_H__
#define _SCENE_SCENESPACEPARTITION_H__


namespace NewWare
{

namespace Scene
{


class SceneSpacePartition
{
private:
    enum { DEFAULT_CHILDNODE_COUNT = 16 };
    enum { 
        X_SECTOR_RANGE = 400, 
        Y_SECTOR_COUNT = 5 
    };


public:
    SceneSpacePartition();
    ~SceneSpacePartition();


    void Build( NiNode* pkAttachTo, NiNode*& pkSrcNode );


private:
    NiNodePtr CreateBuildNode( char const* nodeName ) const;
    void DoBuildNode( NiNode* pkBuildNode, NiNode const* pkSrcNode, NiBound const& kBound ) const;
    NiNodePtr OrganizeNode( NiNode const* pkBuildNode, char const* nodeName ) const;

    bool GetChildNodeIndex( unsigned int& uiChildX, unsigned int& uiChildY, NiPoint3 const& kPos, 
                            float fXRange, float fYRange, float fFirstStartX, float fFirstStartY ) const;


private:
    unsigned int m_uiChildNodeCount;
};


void OutputDebugSceneWorldBound( NiNode* pkScene );


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_SCENESPACEPARTITION_H__
