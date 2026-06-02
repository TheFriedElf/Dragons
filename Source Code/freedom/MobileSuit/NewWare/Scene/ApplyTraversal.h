
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : ApplyTraversal.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#ifndef _SCENE_APPLYTRAVERSAL_H__
#define _SCENE_APPLYTRAVERSAL_H__


namespace NewWare
{

namespace Scene
{


namespace ApplyTraversal
{

    namespace Geometry
    {
        // Geometry precaching - Unskinned geometry will pass 0 for the last two parameters.
        void PrecacheGeometry( NiRenderer* pkRenderer, NiAVObject* pkAVObject, 
                               unsigned int uiBonesPerPartition = 0, unsigned int uiBonesPerVertex = 0 );

        void SetShaderConstantUpdateOptimizeFlag( NiAVObject* pkAVObject, bool bUseOptimize, bool bNeedOptimize = true );
        void SetMaterialNeedsUpdate( NiAVObject* pkAVObject, bool bNeedsUpdate = true );
        void SetDefaultMaterialNeedsUpdateFlag( NiAVObject* pkAVObject, bool bOffNextUpdate );

        void RemoveSkinPartitionAtSkinInstance( NiAVObject* pkAVObject );
        bool OptimizeActorSkins( NiAVObject* pkActorRoot, unsigned int uiBonesPerPartition );

        void CompactChildNode( NiNode* pkNode );

        void HideParticleSystems( NiAVObject* pkAVObject, bool bHide );
        void RunParticleGeneration( NiAVObject* pkAVObject, bool bStop );
        bool HasParticle( NiAVObject* pkAVObject );

        void RepairFalseAlpha( NiAVObject* pkAVObject );
    } //namespace Geometry


    namespace TimeController
    {
        void RestartAnimation( NiAVObject* pkAVObject, float fCurrentAccumTime );
    } //namespace TimeController


    namespace Property
    {
        void EnableAlphaBlending( NiAVObject* pkAVObject, bool bEnable );

        void EnableZBuffer( NiAVObject* pkAVObject, bool bTest, bool bWrite );
        inline void EnableZBufferWrite( NiAVObject* pkAVObject, bool bEnable ) { 
            EnableZBuffer( pkAVObject, true, bEnable );
        }

        void EnableFog( NiAVObject* pkAVObject, bool bEnable );
        void DisableFogAtBillboardAndParticle( NiAVObject* pkAVObject );

        void DetachAlphaProperty( NiAVObject* pkAVObject, NiAlphaProperty* pkAlphaProp );

        void SetAlphaGroup( NiAVObject* pkAVObject, int iGroupNumber );
    } //namespace Property


    namespace Node
    {
        unsigned int Detach( NiAVObject* pkAVObject, char const* matchName, NiNode* pkGarbageCollector );
    } //namespace Node

} //namespace ApplyTraversal


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_APPLYTRAVERSAL_H__
