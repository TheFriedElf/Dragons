
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : ApplyTraversal.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#include "stdafx.h"
#include "ApplyTraversal.h"

#include "NodeTraversal.h"
#include "ApplyProperty.h"

#include <NiOptimize.h>


namespace NewWare
{

namespace Scene
{


namespace ApplyTraversal
{


/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace { 

    // @note - Why cpp implementation? : Duplicated SetAppCulled::_Functor members link error!!!
    template < class T > 
    void SetAppCulled( NiAVObject* pkAVObject, bool bAppCulled )
    {
        assert( pkAVObject );

        struct _Functor
        {
            explicit _Functor( bool bCulled ) : bAppCulled(bCulled) { /* Nothing */ };

            void operator() ( NiAVObject const* pkObject )
            {
                T* pkGeo = NiDynamicCast(T, pkObject);
                if ( pkGeo )
                    pkGeo->SetAppCulled( bAppCulled );
            }

            bool const bAppCulled;
        } kFunctor( bAppCulled );

        NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
    }

    // @note - Why cpp implementation? : Duplicated HasAVObject::_Functor members link error!!!
    template < class T > 
    bool HasAVObject( NiAVObject* pkAVObject )
    {
        assert( pkAVObject );

        struct _Functor
        {
            bool operator() ( NiAVObject const* pkObject )
            {
                return (NiIsKindOf(T, pkObject) == false); // if no T, continue traversal
            }
        } kFunctor;

        return (NodeTraversal::DepthFirst::FirstStop(pkAVObject, kFunctor) == false);
    }

} //namespace
//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace Geometry
{

void PrecacheGeometry( NiRenderer* pkRenderer, NiAVObject* pkAVObject, 
                       unsigned int uiBonesPerPartition, unsigned int uiBonesPerVertex )
{
    assert( pkRenderer && pkAVObject );

    struct _Functor
    {
        _Functor( NiRenderer* pkRender, unsigned int uiPerPartition, unsigned int uiPerVertex ) 
            : pkRenderer(pkRender), uiBonesPerPartition(uiPerPartition), uiBonesPerVertex(uiPerVertex)
        { assert( pkRenderer ); };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
                pkRenderer->PrecacheGeometry( pkGeo, uiBonesPerPartition, uiBonesPerVertex );
        }

        NiRenderer* pkRenderer;
        unsigned int const uiBonesPerPartition;
        unsigned int const uiBonesPerVertex;
    } kFunctor( pkRenderer, uiBonesPerPartition, uiBonesPerVertex );

    NodeTraversal::DepthFirst::AllObjects_Upward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void SetShaderConstantUpdateOptimizeFlag( NiAVObject* pkAVObject, bool bUseOptimize, bool bNeedOptimize )
{
    assert( pkAVObject );

    struct _Functor
    {
        _Functor( bool bUse, bool bNeed ) : bUseOptimize(bUse), bNeedOptimize(bNeed) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
            {
                pkGeo->SetUseShaderConstantUpdateOptimize( bUseOptimize );
                pkGeo->SetNeedUpdateAllShaderConstant( bNeedOptimize );
            }
        }

        bool const bUseOptimize;
        bool const bNeedOptimize;
    } kFunctor( bUseOptimize, bNeedOptimize );

    NodeTraversal::DepthFirst::AllObjects_Upward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void SetMaterialNeedsUpdate( NiAVObject* pkAVObject, bool bNeedsUpdate )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( bool bNeeds ) : bNeedsUpdate(bNeeds) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
                pkGeo->SetMaterialNeedsUpdate( bNeedsUpdate );
        }

        bool const bNeedsUpdate;
    } kFunctor( bNeedsUpdate );

    NodeTraversal::DepthFirst::AllObjects_Upward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void SetDefaultMaterialNeedsUpdateFlag( NiAVObject* pkAVObject, bool bOffNextUpdate )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( bool bUpdate ) : bOffNextUpdate(bUpdate) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
            {
                pkGeo->SetDefaultMaterialNeedsUpdateFlagOffNextUpdate( bOffNextUpdate == false );
                pkGeo->SetDefaultMaterialNeedsUpdateFlag( true );
                pkGeo->SetNeedUpdateAllShaderConstant( true );
            }
        }

        bool const bOffNextUpdate;
    } kFunctor( bOffNextUpdate );

    NodeTraversal::DepthFirst::AllObjects_Upward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void RemoveSkinPartitionAtSkinInstance( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo && pkGeo->GetSkinInstance() )
                pkGeo->GetSkinInstance()->SetSkinPartition( NULL );
        }
    } kFunctor;

    NodeTraversal::DepthFirst::AllObjects_Upward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

bool OptimizeActorSkins( NiAVObject* pkActorRoot, unsigned int uiBonesPerPartition )
{
    assert( pkActorRoot );
    if ( NULL == pkActorRoot )
        return false;

    CompactChildNode( NiDynamicCast(NiNode, pkActorRoot) );

    RemoveSkinPartitionAtSkinInstance( pkActorRoot );
    NiOptimize::OptimizeSkinData( pkActorRoot, true, true, uiBonesPerPartition, true );
    NiOptimize::WeldSkin( pkActorRoot );

    SetDefaultMaterialNeedsUpdateFlag( pkActorRoot, false );
    return true;
}

//-----------------------------------------------------------------------------------

void CompactChildNode( NiNode* pkNode )
{
    if ( NULL == pkNode )
        return;

    pkNode->CompactChildArray();

    unsigned int const uiChildCount = pkNode->GetArrayCount();
    for ( unsigned int ui = 0; ui < uiChildCount; ++ui )
    {
        NiAVObject* pkChild = pkNode->GetAt( ui );
        if ( NiIsKindOf(NiNode, pkChild) )
        {
            CompactChildNode( static_cast<NiNode*>(pkChild) );
        }
    }
}

//-----------------------------------------------------------------------------------

void HideParticleSystems( NiAVObject* pkAVObject, bool bHide )
{
    assert( pkAVObject );
    SetAppCulled<NiParticleSystem>( pkAVObject, bHide );
}

//-----------------------------------------------------------------------------------

void RunParticleGeneration( NiAVObject* pkAVObject, bool bStop )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( bool _bStop ) : bStop(_bStop) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiParticleSystem* pkParticle = NiDynamicCast(NiParticleSystem, pkObject);
            if ( pkParticle )
            {
                NiTimeController* pkTimeCtlr = pkParticle->GetControllers();
                while ( pkTimeCtlr )
                {
                    NiPSysEmitterCtlr* pkEmitter = NiDynamicCast(NiPSysEmitterCtlr, pkTimeCtlr);
                    if ( pkEmitter )
                        pkEmitter->SetActive( bStop == false );

                    pkTimeCtlr = pkTimeCtlr->GetNext();
                }
            }
        }

        bool const bStop;
    } kFunctor( bStop );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

bool HasParticle( NiAVObject* pkAVObject )
{
    assert( pkAVObject );
    return HasAVObject<NiParticleSystem>( pkAVObject );
}

//-----------------------------------------------------------------------------------

void RepairFalseAlpha( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
                ApplyProperty::Override::RepairFalseAlphaGeometry( pkGeo );
        }

    } kFunctor;

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

} //namespace Geometry
//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace TimeController
{

void RestartAnimation( NiAVObject* pkAVObject, float fCurrentAccumTime )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( float fAccumTime ) : fCurrentAccumTime(fAccumTime) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiTimeController* pkController = pkObject->GetControllers();
            while ( pkController )
            {
                pkController->SetPhase( -fCurrentAccumTime );
                pkController = pkController->GetNext();
            }
        }

        float fCurrentAccumTime;
    } kFunctor( fCurrentAccumTime );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

} //namespace TimeController
//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace Property
{

void EnableAlphaBlending( NiAVObject* pkAVObject, bool bEnable )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( bool _bEnable ) : bEnable(_bEnable) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiAlphaProperty* pkProp = 
                static_cast<NiAlphaProperty*>(pkObject->GetProperty(NiAlphaProperty::GetType()));
            if ( pkProp )
                pkProp->SetAlphaBlending( bEnable );
        }

        bool const bEnable;
    } kFunctor( bEnable );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void EnableZBuffer( NiAVObject* pkAVObject, bool bTest, bool bWrite )
{
    assert( pkAVObject );

    struct _Functor
    {
        _Functor( bool _bTest, bool _bWrite ) : bTest(_bTest), bWrite(_bWrite) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiZBufferProperty* pkProp = 
                static_cast<NiZBufferProperty*>(pkObject->GetProperty(NiZBufferProperty::GetType()));
            if ( pkProp )
            {
                pkProp->SetZBufferTest( bTest );
                pkProp->SetZBufferWrite( bWrite );
            }
        }

        bool const bTest;
        bool const bWrite;
    } kFunctor( bTest, bWrite );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void EnableFog( NiAVObject* pkAVObject, bool bEnable )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( bool _bEnable ) : bEnable(_bEnable) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiFogProperty* pkProp = 
                static_cast<NiFogProperty*>(pkObject->GetProperty(NiFogProperty::GetType()));
            if ( pkProp )
                pkProp->SetFog( bEnable );
        }

        bool const bEnable;
    } kFunctor( bEnable );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void DisableFogAtBillboardAndParticle( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        void operator() ( NiAVObject const* pkObject )
        {
            if ( NiIsKindOf(NiBillboardNode, pkObject) || NiIsKindOf(NiParticleSystem, pkObject) )
            {
                if ( pkObject->GetProperty(NiProperty::FOG) )
                    ((NiAVObject*)pkObject)->RemoveProperty( NiProperty::FOG );

                NiFogProperty* pkProp = NiNew NiFogProperty;
                assert( pkProp );
                pkProp->SetFog( false );

                ((NiAVObject*)pkObject)->AttachProperty( pkProp );
                ((NiAVObject*)pkObject)->UpdateProperties();
            }
        }
    } kFunctor;

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void DetachAlphaProperty( NiAVObject* pkAVObject, NiAlphaProperty* pkAlphaProp )
{
    assert( pkAVObject && pkAlphaProp );

    struct _Functor
    {
        explicit _Functor( NiAlphaProperty* _pkAlphaProp ) : pkAlphaProp(_pkAlphaProp) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiAlphaProperty* pkAlpha = 
                    static_cast<NiAlphaProperty*>(pkObject->GetProperty(NiAlphaProperty::GetType()));
            if ( pkAlpha && pkAlpha == pkAlphaProp )
            {
                ((NiAVObject*)pkObject)->DetachProperty( pkAlphaProp );
            }
        }

        NiAlphaProperty* pkAlphaProp;
    } kFunctor( pkAlphaProp );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void SetAlphaGroup( NiAVObject* pkAVObject, int iGroupNumber )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( int const _iGroupNumber ) : iGroupNumber(_iGroupNumber) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeo = NiDynamicCast(NiGeometry, pkObject);
            if ( pkGeo )
            {
                NiAlphaProperty* pkProp = 
                                static_cast<NiAlphaProperty*>(pkGeo->GetProperty(NiProperty::ALPHA));
                if ( pkProp )
                    pkProp->SetAlphaGroup( iGroupNumber );
            }
        }

        int const iGroupNumber;
    } kFunctor( iGroupNumber );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

} //namespace Property
//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace Node
{

unsigned int Detach( NiAVObject* pkAVObject, char const* matchName, NiNode* pkGarbageCollector )
{
    assert( pkAVObject && matchName && pkGarbageCollector );

    struct _Functor
    {
        _Functor( char const* matchName, NiNode* pkCollector ) 
            : kMatchName(matchName), pkGarbageCollector(pkCollector), uiCount(0) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            if ( pkObject->GetName() == kMatchName )
            {
                NiAVObject* pkAVObject = const_cast<NiAVObject*>(pkObject);

                NiNode* pkParent = pkAVObject->GetParent();
                if ( pkParent )
                {
                    pkGarbageCollector->AttachChild( pkParent->DetachChild(pkAVObject), false );
                    ++uiCount;
                }
            }
        }

        NiFixedString kMatchName;
        NiNode* pkGarbageCollector;
        unsigned int uiCount;
    } kFunctor(matchName, pkGarbageCollector);

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
    return kFunctor.uiCount;
}

} //namespace Node
//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace ApplyTraversal


} //namespace Scene

} //namespace NewWare
