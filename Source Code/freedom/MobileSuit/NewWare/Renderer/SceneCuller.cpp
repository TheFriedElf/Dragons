
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneCuller.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#include "stdafx.h"
#include "SceneCuller.h"

#include "OCuller/HW/OcclusionScene.h"

#include "DrawUtils.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

SceneCuller::SceneCuller()
    : m_pkCamera(NULL), 
      m_pkFrustumCuller(NULL), 
      m_bOcclusionCullMode(false), 
      m_pkOcclusionCuller(new OCuller::HW::OcclusionCuller), 
      m_pkOcclusionScene(new OCuller::HW::OcclusionScene(m_pkOcclusionCuller))
{
    m_pkFrustumCuller = NiNew NiCullingProcess( NULL );
    assert( m_pkFrustumCuller );

    assert( m_pkOcclusionCuller );
    assert( m_pkOcclusionScene );
}

//-----------------------------------------------------------------------------------

SceneCuller::~SceneCuller()
{
    NiDelete m_pkFrustumCuller;

    delete m_pkOcclusionScene;
    delete m_pkOcclusionCuller;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SceneCuller::Create( bool bOcclusionCullMode )
{
    m_bOcclusionCullMode = bOcclusionCullMode;

    m_pkOcclusionCuller->Reset();
    m_pkOcclusionScene->RemoveAllScenes();
}

//-----------------------------------------------------------------------------------

void SceneCuller::Build( NiAVObject* pkAVObject )
{
    assert( pkAVObject );

    if ( IsOcclusionCullMode() )
    {
        assert( m_pkOcclusionScene );
        m_pkOcclusionScene->AddScene( pkAVObject );
    }
}

//-----------------------------------------------------------------------------------

void SceneCuller::Begin( NiCamera const* pkCamera )
{
    PROFILE_FUNC();

    assert( pkCamera && IsCulling() == false );

    m_pkCamera = pkCamera;
    m_kFrustumVisibleArray.RemoveAll();
}

//-----------------------------------------------------------------------------------

unsigned int SceneCuller::Commit( NiAVObject* pkScene, bool bResetArray )
{
    assert( pkCamera && pkScene && IsCulling() );

    return DrawUtils::GetPVSFromCullScene( m_pkCamera, *m_pkFrustumCuller, pkScene, 
                                           m_kFrustumVisibleArray, bResetArray );
}

//-----------------------------------------------------------------------------------

void SceneCuller::AddCommit( NiVisibleArray& kVisibleArray )
{
    assert( IsCulling() );

    unsigned int const uiQuantity = kVisibleArray.GetCount();
    for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
    {
        AddCommit( kVisibleArray.GetAt(ui) );
    }
}

//-----------------------------------------------------------------------------------

void SceneCuller::End()
{
    PROFILE_FUNC();

    assert( IsCulling() );

    if ( IsOcclusionCullMode() )
    {
        assert( m_pkOcclusionCuller );
        m_pkOcclusionCuller->DoCull( m_pkCamera, m_kFrustumVisibleArray, true );
    }

    m_pkCamera = NULL;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
