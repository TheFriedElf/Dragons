
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Flipper.cpp
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
#include "Flipper.h"


namespace NewWare
{

namespace Renderer
{

namespace Task
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

HANDLE Flipper::ms_hResume = NULL;
HANDLE Flipper::ms_hExit = NULL;
HANDLE Flipper::ms_hFinishWork = NULL;

//-----------------------------------------------------------------------------------

Flipper::Flipper( DrawBound* pkDrawBound, NiRenderer* pkRenderer )
    : TaskDecorator(pkDrawBound), 
      m_pkRenderer(pkRenderer)
{
    assert( m_pkRenderer );

    ms_hResume     = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    ms_hExit       = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    ms_hFinishWork = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    ::ResetEvent( ms_hResume );
    ::ResetEvent( ms_hExit );
    ::SetEvent( ms_hFinishWork );

    _beginthread( RenderThread, 0, this );
}

//-----------------------------------------------------------------------------------

Flipper::~Flipper()
{
    ExitEvent();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool Flipper::Execute( float fFrameTime, StreamPack const& kStreamPack )
{
    PROFILE_FUNC();

    if ( TaskDecorator::Execute(fFrameTime, kStreamPack) == false )
        return false;

    return RenderGPUBound();
}

//-----------------------------------------------------------------------------------

void _cdecl Flipper::RenderThread( void* arg )
{
    assert( arg );
    Flipper* pkFlipper = (Flipper*)arg;

    HANDLE ahHandle[2] = { ms_hResume, ms_hExit };
    while ( true )
    {
        DWORD dwResult = ::WaitForMultipleObjects( 2, ahHandle, FALSE, INFINITE );
        if ( dwResult == WAIT_OBJECT_0 + 1 )
            break;

        if ( pkFlipper->RenderGPUBound() == false )
            break;
        ::SetEvent( ms_hFinishWork );
    }

    ::CloseHandle( ms_hResume );
    ::CloseHandle( ms_hExit );
    ::CloseHandle( ms_hFinishWork );
    _endthread();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Task

} //namespace Renderer

} //namespace NewWare
