
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneDrawerLegacy.cpp
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
#include "SceneDrawerLegacy.h"

#include "StreamPack.h"

#include "DrawUtils.h"
#include "DrawOptions.h"

#include "../../PgRenderer.h"
#include "../../PgRenderMan.h"
#include "../../PgSpotLight.h"
#include "../../PgBatchRender.h"
#include "../../PgAlphaAccumulator.h"
#include "../../PgPSRoomGroup.h"
#include "../../PgPostProcessMan.h"
#include "../../PgAVObjectAlphaProcessManager.h"
#include "../../PgWater.h"
#include "../../PgHome.h"
#include "../../PgWorld.h"
#include "../../PgActor.h"
#include "../../PgLinkedPlane.h"
#include "../../PgProjectileMan.h"
#include "../../PgTextBalloon.h"
#include "../../PgSkillTargetMan.h"
#include "../../PgZoneDrawing.h"
#include "../../PgDamageNumMan.h"
#include "../../PgPilotMan.h"
#include "../../PgOption.h"
#include "../../PgEnvironmentMan.h"
#include "../../PgFxStudio.h"
#include "../../PgHomeRenew.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
// Pimple idiom

class SceneDrawerLegacy::Pimple
{
public:
    explicit Pimple( PgRenderer* pkRenderer ) 
        : m_pkPgRenderer(pkRenderer), m_pkBatchRender(NULL), m_kCuller(NULL) { /* Nothing */ };
    ~Pimple() { /* Nothing */ };


    void DrawPrepare( NiCameraPtr spCamera, PgBatchRender* pkBatchRender )
    {
        m_spCamera = spCamera;
        m_pkBatchRender = pkBatchRender;
    }


public:
    void DrawSky()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( 0 == g_pkWorld->GetSkyRoot() )
            return;

        ResetViewFrustumFarRange();

        //g_pkWorld->GetSkyRoot()->SetAppCulled( false );
        DrawUtils::PartialRenderImmediate( m_spCamera, m_kCuller, 
                                           g_pkWorld->GetSkyRoot(), m_kVisibleScene, true );
        //g_pkWorld->GetSkyRoot()->SetAppCulled( true );
    }

    void DrawWater( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetWater() )
            g_pkWorld->GetWater()->RenderImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
    }

    void DrawHome( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetHome() )
            g_pkWorld->GetHome()->Draw( m_pkPgRenderer, m_spCamera, fFrameTime );
    }

    void DrawEffectedObjects( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetAVObjectAlphaProcessManager() )
            g_pkWorld->GetAVObjectAlphaProcessManager()->Draw( m_pkPgRenderer, m_spCamera, 
                                                               fFrameTime, m_kVisibleScene );
    }

    void DrawBackground()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetDrawBG() == false )
            return;

        g_iTriangleCount = 0;

        if ( g_bUseDebugInfo )
        {
            DrawUtils::GetPVSFromCullScene( m_spCamera, m_kCuller, g_pkWorld->GetSceneRoot(), m_kVisibleScene, true );
            NiDrawVisibleArrayAppend( m_kVisibleScene );
        }
        else
        {
            DrawUtils::GetPVSFromCullScene( m_spCamera, m_kCuller, g_pkWorld->GetPSRoomGroupRoot(),   m_kVisibleScene, true );
            DrawUtils::GetPVSFromCullScene( m_spCamera, m_kCuller, g_pkWorld->GetStaticNodeRoot(),    m_kVisibleScene, false );
            DrawUtils::GetPVSFromCullScene( m_spCamera, m_kCuller, g_pkWorld->GetSelectiveNodeRoot(), m_kVisibleScene, false );
            DrawUtils::GetPVSFromCullScene( m_spCamera, m_kCuller, g_pkWorld->GetDynamicNodeRoot(),   m_kVisibleScene, false );

            if ( g_bUseBatchRender )
                _DrawBatching();
            else
                _DrawNonBatching();
        }
    }

    void DrawObjects( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        m_pkBatchRender->ClearVisibleBatchGroup();
		g_pkWorld->DrawObjects(m_pkPgRenderer, m_spCamera, fFrameTime);
    }

    void DrawBatching()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        PgAlphaAccumulator* pkAccum = (PgAlphaAccumulator*)m_pkPgRenderer->GetSorter();
        assert( NiIsKindOf(PgAlphaAccumulator, m_pkPgRenderer->GetSorter()) );
        pkAccum->SetBatchRender( NULL );

        m_pkBatchRender->Draw( m_pkPgRenderer, m_spCamera, g_pkWorld->GetAccumTime() );
    }

    void DrawProjectiles( float fFrameTime )
    {
        PROFILE_FUNC();

        g_kProjectileMan.Draw( m_pkPgRenderer, m_spCamera, fFrameTime );

        g_spTrailNodeMan->SetAppCulled( false );
        m_pkPgRenderer->PartialRenderClick_Deprecated( g_spTrailNodeMan );
        g_spTrailNodeMan->SetAppCulled( true );
    }

    void DrawEnvironments( float fFrameTime )
    {
        PROFILE_FUNC();

        g_kEnvironmentMan.Draw( m_pkPgRenderer, m_spCamera, fFrameTime );
    }

    void DrawBalloonText( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetDrawWorldText() == false )
            return;

        g_kBalloonMan3D.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
        g_pkWorld->DrawWorldText( m_pkPgRenderer, m_spCamera );
    }

    void DrawImmediate( float fFrameTime )
    {
        PROFILE_FUNC();

        _DrawImmediateActor( fFrameTime );
        _DrawImmediateEtc( fFrameTime );
        _DrawNoZTest( fFrameTime );
    }

    void DrawScreenElements()
    {
        PROFILE_FUNC();

        g_kFxStudio.DrawScreenElements( m_pkPgRenderer->GetRenderer(), m_spCamera );
    }

    void DrawFocusFilter()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        g_pkWorld->DrawFocusFilter( m_pkPgRenderer );
    }


public:
    void PrepareViewFrustumFarRange()
    {
		if(!g_pkWorld)
		{
			return;
		}
        m_spCamera->SetViewFrustum( g_kFrustum );
        g_pkWorld->AdjustCameraAspectRatio( m_spCamera );

        m_pkPgRenderer->SetCameraData( m_spCamera );

        m_kFrustum = m_spCamera->GetViewFrustum();
    }
    void ResetViewFrustumFarRange()
    {
        m_kFrustum.m_fFar = DrawOptions::FRUSTUM_FAR_DEF_LANGE;
        m_spCamera->SetViewFrustum( m_kFrustum );
    }

    void ApplyViewFrustumDistanceFogRange()
    {
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetFogProperty() && g_pkWorld->UseFogEndFar() )
            m_kFrustum.m_fFar = g_pkWorld->GetFogProperty()->GetFogEnd();
        else
            m_kFrustum.m_fFar = DrawOptions::FRUSTUM_FAR_DEF_LANGE;

        m_spCamera->SetViewFrustum( m_kFrustum );
    }
    void ApplyViewFrustumDistanceRange()
    {
		if(!g_pkWorld)
		{
			return;
		}
        if ( false == g_pkWorld->UseViewDistanceRange() )
            return;

        if ( GET_OPTION_VIEW_DISTANCE == false )
            m_kFrustum.m_fFar = g_pkWorld->GetViewDistanceRange( 0 );
        else
            m_kFrustum.m_fFar = g_pkWorld->GetViewDistanceRange( 1 );

        m_spCamera->SetViewFrustum( m_kFrustum );
    }


public:
    void StartAccumulating( NiAccumulator* pkSorter )
    {
        PROFILE_FUNC();

        m_kVisibleScene.RemoveAll();
        assert( pkSorter );
        pkSorter->StartAccumulating( m_spCamera );
    }
    void FinishAccumulating( NiAccumulator* pkSorter )
    {
        PROFILE_FUNC();

        assert( pkSorter );
        pkSorter->FinishAccumulating();
    }


private:
    void _DrawBatching()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_bShowTriangleCount )
            g_iTriangleCount += DrawUtils::GetTriangleCount( m_kVisibleScene );

        m_pkBatchRender->ClearVisibleBatchGroup();

        PgAlphaAccumulator* pkAccum = (PgAlphaAccumulator*)m_pkPgRenderer->GetSorter();
        assert( NiIsKindOf(PgAlphaAccumulator, m_pkPgRenderer->GetSorter()) );
        pkAccum->SetBatchRender( m_pkBatchRender );
        NiDrawVisibleArrayAppend( m_kVisibleScene );
        pkAccum->SetBatchRender( NULL );

        m_pkBatchRender->Draw( m_pkPgRenderer, m_spCamera, g_pkWorld->GetAccumTime() );

        m_kVisibleScene.RemoveAll();
    }

    void _DrawNonBatching()
    {
        PROFILE_FUNC();

        if ( g_bShowTriangleCount )
            g_iTriangleCount += DrawUtils::GetTriangleCount( m_kVisibleScene );

        NiDrawVisibleArrayAppend( m_kVisibleScene );
    }

    void _DrawImmediateActor( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->DrawImmediateActor(m_pkPgRenderer, m_spCamera, fFrameTime);
    }

    void _DrawImmediateEtc( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->m_pkDamageNumMan )
            g_pkWorld->m_pkDamageNumMan->Draw( m_pkPgRenderer, m_spCamera, fFrameTime );

        g_kLinkedPlaneGroupMan.DrawImmediate( m_pkPgRenderer );

        g_kProjectileMan.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
        g_kEnvironmentMan.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );

#ifndef USE_INB
        g_kSkillTargetMan.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
		g_kMonSkillTargetMan.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
        g_kZoneDrawer.DrawImmediate( m_pkPgRenderer, m_spCamera, fFrameTime );
#endif //#ifndef USE_INB
    }

    void _DrawNoZTest( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->DrawNoZTest(m_pkPgRenderer, m_spCamera, fFrameTime);
    }


private:
    PgRenderer* m_pkPgRenderer;
    PgBatchRender* m_pkBatchRender;

    NiCameraPtr m_spCamera;
    NiFrustum m_kFrustum;

    NiCullingProcess m_kCuller;
    NiVisibleArray m_kVisibleScene;
};

// Pimple idiom
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

namespace { PgBatchRender* g_pkLegacyBatchRender; }

SceneDrawerLegacy::SceneDrawerLegacy()
    : m_pkPimple(new Pimple(g_kRenderMan.GetRenderer())), 
      m_spAccumulator(NiNew PgAlphaAccumulator), 
      m_pkBatchRender(NiNew PgBatchRender), 
      m_pkPgRenderer(g_kRenderMan.GetRenderer())
{
    assert( m_pkPimple );
    assert( m_spAccumulator );
    //m_spAccumulator->SetSortByClosestPoint( true );
    assert( m_pkBatchRender );

    g_pkLegacyBatchRender = m_pkBatchRender;
}

//-----------------------------------------------------------------------------------

SceneDrawerLegacy::~SceneDrawerLegacy()
{
    g_pkLegacyBatchRender = NULL;
    NiDelete m_pkBatchRender;

    delete m_pkPimple;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool SceneDrawerLegacy::Build()
{
    PROFILE_FUNC();
	if(!g_pkWorld)
	{
		return false;
	}
    if ( g_bUseBatchRender )
    {
        assert( m_pkBatchRender );
        m_pkBatchRender->ClearVisibleBatchGroup();
        m_pkBatchRender->AddObject( g_pkWorld->GetPSRoomGroupRoot() );
        m_pkBatchRender->AddObject( g_pkWorld->GetStaticNodeRoot() );
        m_pkBatchRender->AddObject( g_pkWorld->GetSelectiveNodeRoot() );
        m_pkBatchRender->ClearAllBatchGroupNodes();
        m_pkBatchRender->ClearVisibleBatchGroup();
    }
    return true;
}

//-----------------------------------------------------------------------------------

bool SceneDrawerLegacy::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    DrawRenderToTexture( fFrameTime );

    m_pkPimple->DrawPrepare( kStreamPack.GetCamera(), m_pkBatchRender );

    // Draw shadows.
    if ( kStreamPack.UseProjShadow() )
        DrawActorShadow();
    DrawSpotLightAffectedShadow( kStreamPack.GetAccumTime() );

    // Draw frame.
    SetFrameBufferClearColor( kStreamPack.GetClearFBufferColor() );
    BeginPostEffectFrame( fFrameTime );
    BeginFrustum();
    DrawFrame( fFrameTime, kStreamPack );
    EndFrustum();
    EndPostEffectFrame( fFrameTime );

    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SceneDrawerLegacy::DrawRenderToTexture( float fFrameTime )
{
    PROFILE_FUNC();
	if(!g_pkWorld)
	{
		return;
	}
    g_kHomeRenewMgr.Draw( m_pkPgRenderer, fFrameTime );
    g_pkWorld->DrawEmporiaGate( m_pkPgRenderer, fFrameTime );
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::DrawActorShadow()
{
    PROFILE_FUNC();

    PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
    if ( pkPlayerActor )
        pkPlayerActor->RenderShadowObject();
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::DrawSpotLightAffectedShadow( float fAccumTime )
{
    PROFILE_FUNC();
	if(!g_pkWorld)
	{
		return;
	}
    if ( g_pkWorld->GetSpotLightMgr() )
        g_pkWorld->GetSpotLightMgr()->ClickAndStuff( fAccumTime );
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::SetFrameBufferClearColor( NiColor const& kColor )
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer );
    m_pkPgRenderer->SetBackgroundColor( kColor );
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::BeginPostEffectFrame( float fFrameTime )
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameStart( fFrameTime, m_pkPgRenderer, NiRenderer::CLEAR_ZBUFFER );
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::EndPostEffectFrame( float fFrameTime ) 
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameEnd( fFrameTime, m_pkPgRenderer );
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::BeginFrustum()
{
    m_pkPimple->PrepareViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::EndFrustum()
{
    m_pkPimple->ResetViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void SceneDrawerLegacy::DrawFrame( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    assert( m_pkPimple );

    m_pkPimple->DrawSky();

    // 1. Step
    m_pkPimple->ApplyViewFrustumDistanceFogRange();

        m_pkPimple->DrawWater( fFrameTime );
        m_pkPimple->DrawHome( fFrameTime );

    m_pkPimple->StartAccumulating( m_spAccumulator );

        m_pkPimple->DrawEffectedObjects( fFrameTime );
        m_pkPimple->DrawBackground();

    if ( g_pkWorld
		&& kStreamPack.IsRenderObjectFocusFilter() && g_pkWorld->EnableFocusFilter() )
    {
        m_pkPimple->FinishAccumulating( m_spAccumulator );
        m_pkPimple->DrawFocusFilter();
        m_pkPimple->StartAccumulating( m_spAccumulator );
    }


    // 2. Step
    m_pkPimple->ApplyViewFrustumDistanceRange();

        m_pkPimple->DrawObjects( fFrameTime );
        m_pkPimple->DrawBatching();
        m_pkPimple->DrawProjectiles( fFrameTime );
        m_pkPimple->DrawEnvironments( fFrameTime );

    m_pkPimple->FinishAccumulating( m_spAccumulator );

        m_pkPimple->DrawBalloonText( fFrameTime );
        m_pkPimple->DrawImmediate( fFrameTime );
        m_pkPimple->DrawScreenElements();

    if ( g_pkWorld
		&& false == kStreamPack.IsRenderObjectFocusFilter() && g_pkWorld->EnableFocusFilter() )
    {
        //m_pkPimple->FinishAccumulating( m_spAccumulator );
        m_pkPimple->DrawFocusFilter();
        //m_pkPimple->StartAccumulating( m_spAccumulator );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

PgBatchRender* GetLegacyBatchRender()
{
    return g_pkLegacyBatchRender;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
