
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : ArtificialDrawer.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/02/19 LeeJR First Created
//

#include "stdafx.h"
#include "ArtificialDrawer.h"

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
//#include "../../PgWater.h"
//#include "../../PgHome.h"
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

class ArtificialDrawer::Pimple
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
    void DrawEffectedObjects( float fFrameTime )
    {
		if(!g_pkWorld)
		{
			return;
		}
        if ( g_pkWorld->GetAVObjectAlphaProcessManager() )
            g_pkWorld->GetAVObjectAlphaProcessManager()->Draw( m_pkPgRenderer, m_spCamera, 
                                                               fFrameTime, m_kVisibleScene );
    }

    void DrawObjects( float fFrameTime )
    {
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->DrawObjects(m_pkPgRenderer, m_spCamera, fFrameTime);
    }

    void DrawProjectiles( float fFrameTime )
    {
        g_kProjectileMan.Draw( m_pkPgRenderer, m_spCamera, fFrameTime );

        g_spTrailNodeMan->SetAppCulled( false );
        m_pkPgRenderer->PartialRenderClick_Deprecated( g_spTrailNodeMan );
        g_spTrailNodeMan->SetAppCulled( true );
    }

    void DrawEnvironments( float fFrameTime )
    {
        g_kEnvironmentMan.Draw( m_pkPgRenderer, m_spCamera, fFrameTime );
    }

    void DrawBalloonText( float fFrameTime )
    {
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
        _DrawImmediateActor( fFrameTime );
        _DrawImmediateEtc( fFrameTime );
        _DrawNoZTest( fFrameTime );
    }

    void DrawScreenElements()
    {
        g_kFxStudio.DrawScreenElements( m_pkPgRenderer->GetRenderer(), m_spCamera );
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
        m_kSceneCuller.Begin( m_spCamera );
        m_kVisibleScene.RemoveAll();

        assert( pkSorter );
        pkSorter->StartAccumulating( m_spCamera );
    }
    void FinishAccumulating( NiAccumulator* pkSorter )
    {
        assert( pkSorter );
        pkSorter->FinishAccumulating();
    }


private:
    void _DrawImmediateActor( float fFrameTime )
    {
		if(!g_pkWorld)
		{
			return;
		}
		g_pkWorld->DrawImmediateActor(m_pkPgRenderer, m_spCamera, fFrameTime);
    }

    void _DrawImmediateEtc( float fFrameTime )
    {
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

ArtificialDrawer::ArtificialDrawer()
    : m_pkPimple(new Pimple(g_kRenderMan.GetRenderer())), 
      m_spAccumulator(NiNew Kernel::GroupAccumulator(std::auto_ptr<Kernel::RenderStateBlockCache>(new Kernel::RenderStateBlockCache))), 
      m_pkPgRenderer(g_kRenderMan.GetRenderer())
{
    assert( m_pkPimple );
    assert( m_spAccumulator );
}

//-----------------------------------------------------------------------------------

ArtificialDrawer::~ArtificialDrawer()
{
    delete m_pkPimple;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool ArtificialDrawer::Build()
{
    assert( m_spAccumulator );
    m_spAccumulator->Reset(); // 배경은 그리지 않으므로 Reset만!

    assert( m_pkPimple );
    m_pkPimple->Create( false ); // Occlusion Culling 비활성화!
    return true;
}

//-----------------------------------------------------------------------------------

bool ArtificialDrawer::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    //DrawRenderToTexture( fFrameTime );

    m_pkPimple->DrawPrepare( kStreamPack.GetCamera() );

    // Draw shadows.
    //if ( kStreamPack.UseProjShadow() )
    //    DrawActorShadow();
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

void ArtificialDrawer::DrawRenderToTexture( float fFrameTime )
{
    g_kHomeRenewMgr.Draw( m_pkPgRenderer, fFrameTime );
    //g_pkWorld->DrawEmporiaGate( m_pkPgRenderer, fFrameTime );
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::DrawActorShadow()
{
    PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
    if ( pkPlayerActor )
        pkPlayerActor->RenderShadowObject();
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::DrawSpotLightAffectedShadow( float fAccumTime )
{
	if(!g_pkWorld)
	{
		return;
	}
    if ( g_pkWorld->GetSpotLightMgr() )
        g_pkWorld->GetSpotLightMgr()->ClickAndStuff( fAccumTime );
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::SetFrameBufferClearColor( NiColor const& kColor )
{
    assert( m_pkPgRenderer );
    m_pkPgRenderer->SetBackgroundColor( kColor );
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::BeginPostEffectFrame( float fFrameTime )
{
    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameStart( fFrameTime, m_pkPgRenderer, NiRenderer::CLEAR_ALL );
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::EndPostEffectFrame( float fFrameTime ) 
{
    assert( m_pkPgRenderer->m_pkPostProcessMan );
    m_pkPgRenderer->m_pkPostProcessMan->OnRenderFrameEnd( fFrameTime, m_pkPgRenderer );
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::BeginFrustum()
{
    m_pkPimple->PrepareViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::EndFrustum()
{
    m_pkPimple->ResetViewFrustumFarRange();
}

//-----------------------------------------------------------------------------------

void ArtificialDrawer::DrawFrame( float fFrameTime, StreamPack const& kStreamPack )
{
    assert( m_pkPimple );

    m_pkPimple->ApplyViewFrustumDistanceFogRange();

    m_pkPimple->StartAccumulating( m_spAccumulator );

        m_pkPimple->DrawEffectedObjects( fFrameTime );

    m_pkPimple->ApplyViewFrustumDistanceRange();

        m_pkPimple->DrawObjects( fFrameTime );
        m_pkPimple->DrawProjectiles( fFrameTime );
        m_pkPimple->DrawEnvironments( fFrameTime );

    m_pkPimple->FinishAccumulating( m_spAccumulator );

        m_pkPimple->DrawBalloonText( fFrameTime );
        m_pkPimple->DrawImmediate( fFrameTime );
        m_pkPimple->DrawScreenElements();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
