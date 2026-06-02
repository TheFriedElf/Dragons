
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionScene.cpp
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
#include "OcclusionScene.h"

#include "OcclusionCuller.h"
#include "OcclusionTimeController.h"

#include "../../../Scene/NodeTraversal.h"

#include <NiNode.h>
#include <NiGeometry.h>
#include <NiParticleSystem.h>
#include <NiWireframeProperty.h>


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

OcclusionScene::OcclusionScene( OcclusionCuller* pkCuller )
    : m_pkOccCuller(pkCuller)
{
    assert( m_pkOccCuller );
}

//-----------------------------------------------------------------------------------

OcclusionScene::~OcclusionScene()
{
    RemoveAllScenes();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionScene::AddScene( NiAVObject* pkScene )
{
    assert( pkScene );

    AddObjects( pkScene );
    m_kAVObjectScenes.AddHead( pkScene );
}

//-----------------------------------------------------------------------------------

void OcclusionScene::AddScene( NiScene* pkScene, NiFixedString& kRootName )
{
    assert( pkScene );

    for ( unsigned int ui = 0; ui < pkScene->GetEntityCount(); ++ui )
    {
        NiEntityInterface* pkEntity = pkScene->GetEntityAt( ui );
        unsigned int uiSceneRootCount = 0;
        if ( pkEntity->GetElementCount(kRootName, uiSceneRootCount) )
        {
            for ( unsigned int uj = 0; uj < uiSceneRootCount; ++uj )
            {
                NiObject* pkObject;
                NIVERIFY( pkEntity->GetPropertyData(kRootName, pkObject, uj) );

                NiAVObject* pkAVObject = NiDynamicCast(NiAVObject, pkObject);
                if ( pkAVObject )
                {
                    AddObjects( pkAVObject );
                }
            }
        }
    }
    m_kEntityScenes.SetAt( pkScene, kRootName );
}

//-----------------------------------------------------------------------------------

void OcclusionScene::RemoveScene( NiAVObject* pkScene )
{
    assert( pkScene );

    ReleaseObjects( pkScene );
    m_kAVObjectScenes.Remove( pkScene );
}

//-----------------------------------------------------------------------------------

void OcclusionScene::RemoveScene( NiScene* pkScene, NiFixedString& kRootName )
{
    assert( pkScene );

    for ( unsigned int ui = 0; ui < pkScene->GetEntityCount(); ++ui )
    {
        NiEntityInterface* pkEntity = pkScene->GetEntityAt( ui );
        unsigned int uiSceneRootCount = 0;
        if ( pkEntity->GetElementCount(kRootName, uiSceneRootCount) )
        {
            for ( unsigned int uj = 0; uj < uiSceneRootCount; ++uj )
            {
                NiObject* pkObject;
                NIVERIFY( pkEntity->GetPropertyData(kRootName, pkObject, uj) );

                NiAVObject* pkAVObject = NiDynamicCast(NiAVObject, pkObject);
                if ( pkAVObject )
                {
                    ReleaseObjects( pkAVObject );
                }
            }
        }
    }
    m_kEntityScenes.RemoveAt( pkScene );
}

//-----------------------------------------------------------------------------------

void OcclusionScene::RemoveAllScenes()
{
    if ( m_kEntityScenes.GetCount() > 0 )
    {
        NiScene* pkScene;
        NiFixedString name;

        NiTMapIterator iter = m_kEntityScenes.GetFirstPos();
        while ( iter )
        {
            m_kEntityScenes.GetNext( iter, pkScene, name );
            assert( pkScene );
            RemoveScene( pkScene, name );
        }
    }

    if ( !m_kAVObjectScenes.IsEmpty() )
    {
        NiTListIterator iter = m_kAVObjectScenes.GetHeadPos();
        while ( iter )
        {
            NiAVObject* pkAVObj = m_kAVObjectScenes.GetNext( iter );
            assert( pkAVObj );
            RemoveScene( pkAVObj );
        }
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionScene::AddObjects( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( OcclusionCuller* _pkCuller ) : pkCuller(_pkCuller) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiGeometry* pkGeometry = NiDynamicCast(NiGeometry, pkObject);
            if ( NULL == pkGeometry )
                return;

            bool const bIsVisual = pkGeometry->IsVisualObject() && !pkGeometry->GetAppCulled();
            bool const bIsParticle = NiIsKindOf(NiParticleSystem, pkGeometry);
            if ( bIsVisual || bIsParticle )
            {
                // If this is an alpha object don't use it as an occluder.
                NiAlphaProperty* pkAlpha = pkGeometry->GetPropertyState()->GetAlpha();
                assert( pkAlpha );
                bool bIsOccluder = !(pkAlpha->GetAlphaBlending());

                // Particle systems aren't occluders.
                if ( bIsParticle )
                    bIsOccluder = false;

                OcclusionTimeController* pkOccluTimeController = 
                            NiNew OcclusionTimeController( bIsOccluder, pkCuller->GetDepthMaterial() );
                assert( pkOccluTimeController );
                pkOccluTimeController->SetTarget( pkGeometry );

                pkCuller->GetOcclusionQuerys()->CreateQueryObject( pkGeometry );
            }
        }

        OcclusionCuller* pkCuller;
    } kFunctor( m_pkOccCuller );

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

void OcclusionScene::ReleaseObjects( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    struct _Functor
    {
        explicit _Functor( OcclusionCuller::OcclusionQuerys* _pkQuerys ) : pkQuerys(_pkQuerys) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            OcclusionTimeController* pkOccluTimeController = FindOcclusionTimeController( pkObject );
            if ( pkOccluTimeController )
            {
                pkOccluTimeController->SetTarget( NULL );
                pkQuerys->DeleteQueryObject( (NiGeometry*)pkObject );
            }
        }

        OcclusionCuller::OcclusionQuerys* pkQuerys;
    } kFunctor( m_pkOccCuller->GetOcclusionQuerys() );

    Scene::NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
