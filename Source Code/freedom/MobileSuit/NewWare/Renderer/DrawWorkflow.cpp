
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DrawWorkflow.cpp
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
#include "DrawWorkflow.h"

#include "DrawBound.h"

#include "SceneDrawerLegacy.h"
#include "SceneDrawer.h"
#include "ArtificialDrawer.h"
#include "SceneDrawerDebug.h"
#include "Task/OverlayDrawer.h"
#include "Task/Flipper.h"

// for void SetRenderStateTagExtraDataNumber()
#include "Kernel/GroupAccumulator.h"
#include "../../PgBatchRender.h"

// for void DrawActor()
#include "../../PgRenderer.h"
#include "../../PgAlphaAccumulator.h"
#include "../../PgActor.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

DrawWorkflow::DrawWorkflow( NiRenderer* pkRenderer )
    : m_pkRenderer(pkRenderer), 
      m_pkDrawBound(NULL), 
      m_pkThreadFlipper(NULL)
{
    assert( m_pkRenderer );
}

//-----------------------------------------------------------------------------------

DrawWorkflow::~DrawWorkflow()
{
    Destroy();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool DrawWorkflow::Create( DrawBound::DrawMode eMode )
{
    PROFILE_FUNC();

    if ( DrawBound::DRAWMODE_NONE == eMode )
        return false;

#ifndef EXTERNAL_RELEASE
    if ( DrawBound::DRAWMODE_THREAD == eMode )
    {
        __asm int 3; // 스레드 랜더링은 쓰면 안됨!
    }
#endif //#ifndef EXTERNAL_RELEASE

    if ( Organize(eMode) == false )
        return false;

    assert( m_pkDrawBound );
    return m_pkDrawBound->Build();
}

//-----------------------------------------------------------------------------------

bool DrawWorkflow::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    assert( m_pkDrawBound );

    if ( m_pkThreadFlipper ) // Is thread flipper?
    {
        m_pkThreadFlipper->WaitForFinishWorkEvent();
        if ( RenderCPUBound(fFrameTime, kStreamPack) == false )
            return false;
        m_pkThreadFlipper->ResumeEvent();
    }
    else
    {
        if ( GetDrawMode() == DrawBound::DRAWMODE_NONE )
        {
            __asm int 3;
            return false;
        }

        if ( RenderCPUBound(fFrameTime, kStreamPack) == false )
            return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------

void DrawWorkflow::Destroy()
{
    delete m_pkThreadFlipper;
    m_pkThreadFlipper = NULL;

    delete m_pkDrawBound;
    m_pkDrawBound = NULL;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool DrawWorkflow::Organize( DrawBound::DrawMode eMode )
{
    PROFILE_FUNC();

    if ( m_pkDrawBound && GetDrawMode() == eMode )
        return true;

    Destroy();

    switch ( eMode )
    {
    case DrawBound::DRAWMODE_LEGACY:
        m_pkDrawBound = SceneDrawerLegacy::Create(); // Draw scene.
        m_pkDrawBound = Task::OverlayDrawer::CreateDecorator( m_pkDrawBound ); // Draw overlay.
        m_pkDrawBound = Task::Flipper::CreateDecoratorArg1( m_pkDrawBound, m_pkRenderer ); // Present/Flipping to frame-buffer.
        break;

    case DrawBound::DRAWMODE_NOTHREAD:
        m_pkDrawBound = SceneDrawer::Create(); // Draw scene.
        m_pkDrawBound = Task::OverlayDrawer::CreateDecorator( m_pkDrawBound ); // Draw overlay.
        m_pkDrawBound = Task::Flipper::CreateDecoratorArg1( m_pkDrawBound, m_pkRenderer ); // Present/Flipping to frame-buffer.
        break;

    case DrawBound::DRAWMODE_THREAD:
        m_pkDrawBound = SceneDrawer::Create(); // Draw scene.
        m_pkDrawBound = Task::OverlayDrawer::CreateDecorator( m_pkDrawBound ); // Draw overlay.
        m_pkThreadFlipper = Task::Flipper::CreateDecoratorArg1( NULL, m_pkRenderer ); // Thread Present/Flipping to frame-buffer.
        break;

    case DrawBound::DRAWMODE_ARTIFICIAL:
        m_pkDrawBound = ArtificialDrawer::Create(); // Draw scene.
        m_pkDrawBound = Task::OverlayDrawer::CreateDecorator( m_pkDrawBound ); // Draw overlay.
        m_pkDrawBound = Task::Flipper::CreateDecoratorArg1( m_pkDrawBound, m_pkRenderer ); // Present/Flipping to frame-buffer.
        break;

    case DrawBound::DRAWMODE_DEBUG:
        m_pkDrawBound = SceneDrawerDebug::Create(); // Draw scene.
        m_pkDrawBound = Task::OverlayDrawer::CreateDecorator( m_pkDrawBound ); // Draw overlay.
        m_pkDrawBound = Task::Flipper::CreateDecoratorArg1( m_pkDrawBound, m_pkRenderer ); // Present/Flipping to frame-buffer.
        break;

    default:
        return false;
        break;
    }
    return true;
}

//-----------------------------------------------------------------------------------

bool DrawWorkflow::RenderCPUBound( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    assert( m_pkRenderer );

    m_pkRenderer->SetSorter( m_pkDrawBound->GetAccumulator() ); // 3D Scene default sorter!!

    if ( m_pkRenderer->BeginFrame() == false )
        return false;
    if ( m_pkDrawBound->Execute(fFrameTime, kStreamPack) == false )
        return false;
    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void SetRenderStateTagExtraDataNumber( NiGeometry& kObject, int iRenderStateNumber )
{
    PROFILE_FUNC();

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_NONE )
        return;

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_LEGACY )
        PgBatchRender::SetBatchGroupID( &kObject, iRenderStateNumber );
    else
        Kernel::SetRenderStateNumber( kObject, iRenderStateNumber );
}

//-----------------------------------------------------------------------------------

void RefreshRenderStateBlockCaches()
{
    PROFILE_FUNC();

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_NONE )
        return;

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_LEGACY )
    {
        PgBatchRender* pkBatchRender = GetLegacyBatchRender();
        if ( pkBatchRender )
        {
            pkBatchRender->Reset();
        }
    }
    else
    {
        Kernel::GroupAccumulator* pkAccumulator = 
            NiDynamicCast(Kernel::GroupAccumulator, GetInstanceProxy(DrawWorkflow)->GetAccumulator());
        if ( pkAccumulator )
        {
            pkAccumulator->Reset();
        }
    }
}

//-----------------------------------------------------------------------------------

void DrawActor( PgRenderer* pkRenderer, PgActor* pkActor )
{
    PROFILE_FUNC();

    assert( pkRenderer && pkActor );

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_NONE )
        return;

    if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() == DrawBound::DRAWMODE_LEGACY )
    {
        PgAlphaAccumulator* pkSorter = static_cast<PgAlphaAccumulator*>(pkRenderer->GetSorter());
        assert( pkSorter );

        PgBatchRender* pkBackupBatchRender = pkSorter->GetBatchRender();
        if ( pkActor->GetCanBatchRender() && g_bUseBatchRender )
        {
            pkSorter->SetBatchRender( GetLegacyBatchRender() );
        }

        pkSorter->RegisterObjectArray( *pkRenderer->GetVisibleArray_Deprecated() );

        pkSorter->SetBatchRender( pkBackupBatchRender );
    }
    else
    {
        Kernel::GroupAccumulator* pkSorter = static_cast<Kernel::GroupAccumulator*>(pkRenderer->GetSorter());
        assert( pkSorter );

        bool bBackupObserveOpaqueNoSortHint = pkSorter->GetObserveOpaqueNoSortHint();
        if ( pkActor->GetCanBatchRender() )
        {
            pkSorter->SetObserveOpaqueNoSortHint( true );
        }

        pkSorter->RegisterObjectArray( *pkRenderer->GetVisibleArray_Deprecated() );

        pkSorter->SetObserveOpaqueNoSortHint( bBackupObserveOpaqueNoSortHint );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
