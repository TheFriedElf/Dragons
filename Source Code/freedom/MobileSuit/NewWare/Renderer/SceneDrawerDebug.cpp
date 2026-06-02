
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : SceneDrawerDebug.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/04/08 LeeJR First Created
//

#include "stdafx.h"
#include "SceneDrawerDebug.h"

#include "StreamPack.h"

#include "SceneCuller.h"
#include "DrawUtils.h"
#include "DrawOptions.h"

#include "Kernel/GroupAccumulator.h"
#include "Kernel/RenderStateBlockCache.h"

#include "../../PgRenderer.h"
#include "../../PgRenderMan.h"
#include "../../PgSpotLight.h"
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

class SceneDrawerDebug::Pimple
{
public:
    explicit Pimple( PgRenderer* pkRenderer ) : m_pkPgRenderer(pkRenderer) { /* Nothing */ };
    ~Pimple() { /* Nothing */ };


    void Create( bool bOcclusionCullMode )
    {
        m_kSceneCuller.Create( bOcclusionCullMode );
    }

    void DrawPrepare( NiCameraPtr spCamera )
    {
        m_spCamera = spCamera;
    }


public:
    void DrawSky()
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        NiNode* pkSkyRoot = g_pkWorld->GetSkyRoot();
        if ( NULL == pkSkyRoot || pkSkyRoot->GetChildCount() == 0 )
            return;

        ResetViewFrustumFarRange();

        DrawUtils::PartialRenderImmediate( m_spCamera, m_kSceneCuller.GetFrustumCuller(), 
                                           pkSkyRoot, m_kVisibleScene, true );
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

    void DrawBackground( bool bDumpScene )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetDrawBG() == false )
            return;

        g_iTriangleCount = 0;

        if ( false == g_bUseDebugInfo )
        {
            PROFILE_CODE( m_kSceneCuller.Commit(g_pkWorld->GetPSRoomGroupRoot(),   true) );
            PROFILE_CODE( m_kSceneCuller.Commit(g_pkWorld->GetStaticNodeRoot(),    false) );
            PROFILE_CODE( m_kSceneCuller.Commit(g_pkWorld->GetSelectiveNodeRoot(), false) );
            PROFILE_CODE( m_kSceneCuller.Commit(g_pkWorld->GetDynamicNodeRoot(),   false) );
        }
        else
        {
            m_kSceneCuller.Commit( g_pkWorld->GetSceneRoot(), true );
        }

        m_kSceneCuller.End();
        NiVisibleArray& kVisibleSet = m_kSceneCuller.GetPVSGeometry();
        NiDrawVisibleArrayAppend( kVisibleSet );

        if ( g_bShowTriangleCount )
            g_iTriangleCount += DrawUtils::GetTriangleCount( kVisibleSet );
    }

    void DrawObjects( float fFrameTime )
    {
        PROFILE_FUNC();
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->DrawObjects(m_pkPgRenderer, m_spCamera, fFrameTime);
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

        m_kSceneCuller.Begin( m_spCamera );
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

    NiCameraPtr m_spCamera;
    NiFrustum m_kFrustum;

    SceneCuller m_kSceneCuller;
    NiVisibleArray m_kVisibleScene;
};

// Pimple idiom
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

SceneDrawerDebug::SceneDrawerDebug()
    : m_pkPimple(new Pimple(g_kRenderMan.GetRenderer())), 
      m_spAccumulator(NiNew NiAlphaAccumulator), 
      m_pkPgRenderer(g_kRenderMan.GetRenderer())
{
    assert( m_pkPimple );
    assert( m_spAccumulator );
}

//-----------------------------------------------------------------------------------

SceneDrawerDebug::~SceneDrawerDebug()
{
    delete m_pkPimple;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool SceneDrawerDebug::Build()
{
    PROFILE_FUNC();

    assert( m_pkPimple );
    m_pkPimple->Create( false ); // Occlusion Culling 비활성화!
    return true;
}

//-----------------------------------------------------------------------------------

bool SceneDrawerDebug::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    DrawRenderToTexture( fFrameTime );

    m_pkPimple->DrawPrepare( kStreamPack.GetCamera() );

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

void SceneDrawerDebug::DrawRenderToTexture( float fFrameTime )
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

void SceneDrawerDebug::DrawActorShadow()
{
    PROFILE_FUNC();

    PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
    if ( pkPlayerActor )
        pkPlayerActor->RenderShadowObject();
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::DrawSpotLightAffectedShadow( float fAccumTime )
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

void SceneDrawerDebug::SetFrameBufferClearColor( NiColor const& kColor )
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer );
    m_pkPgRenderer->SetBackgroundColor( kColor );
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::BeginPostEffectFrame( float fFrameTime )
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameStart( fFrameTime, m_pkPgRenderer, NiRenderer::CLEAR_ZBUFFER );
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::EndPostEffectFrame( float fFrameTime ) 
{
    PROFILE_FUNC();

    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameEnd( fFrameTime, m_pkPgRenderer );
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::BeginFrustum()
{
    m_pkPimple->PrepareViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::EndFrustum()
{
    m_pkPimple->ResetViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void SceneDrawerDebug::DrawFrame( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    assert( m_pkPimple );
	if(!g_pkWorld)
	{
		return;
	}
    m_pkPimple->DrawSky();

    // 1. Step
    m_pkPimple->ApplyViewFrustumDistanceFogRange();

        m_pkPimple->DrawWater( fFrameTime );
        m_pkPimple->DrawHome( fFrameTime );

    m_pkPimple->StartAccumulating( m_spAccumulator );

        m_pkPimple->DrawEffectedObjects( fFrameTime );
        m_pkPimple->DrawBackground( kStreamPack.GetDumpScene() );

    if ( kStreamPack.IsRenderObjectFocusFilter() && g_pkWorld->EnableFocusFilter() )
    {
        m_pkPimple->FinishAccumulating( m_spAccumulator );
        m_pkPimple->DrawFocusFilter();
        m_pkPimple->StartAccumulating( m_spAccumulator );
    }


    // 2. Step
    m_pkPimple->ApplyViewFrustumDistanceRange();

        m_pkPimple->DrawObjects( fFrameTime );
        m_pkPimple->DrawProjectiles( fFrameTime );
        m_pkPimple->DrawEnvironments( fFrameTime );

    m_pkPimple->FinishAccumulating( m_spAccumulator );

        m_pkPimple->DrawBalloonText( fFrameTime );
        m_pkPimple->DrawImmediate( fFrameTime );
        m_pkPimple->DrawScreenElements();

    if ( false == kStreamPack.IsRenderObjectFocusFilter() && g_pkWorld->EnableFocusFilter() )
    {
        m_pkPimple->DrawFocusFilter();
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
