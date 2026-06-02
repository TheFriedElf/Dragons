
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OverlayDrawer.cpp
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
#include "OverlayDrawer.h"

#include "../../../PgRenderer.h"
#include "../../../PgRenderMan.h"
#include "../../../PgMissionComplete.h"

#include "../../../PgWorld.h"
#include "../../../PgUIScene.h"


namespace NewWare
{

namespace Renderer
{

namespace Task
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

OverlayDrawer::OverlayDrawer( DrawBound* pkDrawBound )
    : TaskDecorator(pkDrawBound), 
      m_pkPgRenderer(g_kRenderMan.GetRenderer()), 
      m_spAccumulator(NiNew NiAlphaAccumulator)
{
    assert( m_pkPgRenderer );
}

//-----------------------------------------------------------------------------------

OverlayDrawer::~OverlayDrawer()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool OverlayDrawer::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    if ( TaskDecorator::Execute(fFrameTime, kStreamPack) == false )
        return false;

    m_pkPgRenderer->SetSorter( m_spAccumulator ); // UI default alpha sorter!!

    DrawInGame( fFrameTime );
    DrawUIScreen( fFrameTime, kStreamPack );
    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OverlayDrawer::DrawInGame( float fFrameTime )
{
    PROFILE_FUNC();

    g_kMissionComplete.Draw( m_pkPgRenderer, fFrameTime );
    g_kBossComplete.Draw( m_pkPgRenderer, fFrameTime );
	g_kExpeditionComplete.Draw( m_pkPgRenderer, fFrameTime );
	if(g_pkWorld)
	{
		g_pkWorld->DrawDebugInfo( m_pkPgRenderer, fFrameTime );
	}
}

//-----------------------------------------------------------------------------------

void OverlayDrawer::DrawUIScreen( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    g_kUIScene.Draw( m_pkPgRenderer, fFrameTime );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Task

} //namespace Renderer

} //namespace NewWare
