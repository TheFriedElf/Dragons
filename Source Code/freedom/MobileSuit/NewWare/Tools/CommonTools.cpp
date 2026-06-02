
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : CommonTools.cpp
// Description : Common tools for benchmark module
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#include "stdafx.h"
#include "CommonTools.h"

#include "../Scene/NodeTraversal.h"

namespace NewWare
{

namespace Tools
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

DebugMsgWriter::DebugMsgWriter()
    : m_hHandle(NULL), 
      m_pfnDoWrite(NULL)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

DebugMsgWriter::~DebugMsgWriter()
{
    Destory();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool DebugMsgWriter::Create( char const* filename, WriteMode eMode, char const* openOption )
{
    if ( NULL == filename || NULL == openOption )
    {
        assert( 0 );
        return false;
    }

    Destory();

    switch ( eMode )
    {
    case BUFFERING_MODE: m_pfnDoWrite = &DebugMsgWriter::DoWriteBuffering; break;
    case IMMEDIATE_MODE: m_pfnDoWrite = &DebugMsgWriter::DoWriteImmediate; break;
    default: assert( 0 ); return false;
    }

    m_hHandle = fopen( filename, openOption );
    assert( m_hHandle );
    return (NULL != m_hHandle);
}

//-----------------------------------------------------------------------------------

void DebugMsgWriter::Destory()
{
    Flush();

    if ( m_hHandle )
    {
        fclose( m_hHandle );
        m_hHandle = NULL;
    }

    m_pfnDoWrite = NULL;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool cdecl DebugMsgWriter::Write( char const* format, ... )
{
    assert( format );
    if ( NULL == m_hHandle || NULL == m_pfnDoWrite )
    {
        assert( 0 );
        return false;
    }

    char szBuffer[STRINGLINE_LENGTH] = {0,};

    va_list kArgs;
    va_start( kArgs, format );
    vsprintf_s( szBuffer, STRINGLINE_LENGTH-1, format, kArgs );
    szBuffer[STRINGLINE_LENGTH-1] = '\0';
    va_end( kArgs );

    return (this->*m_pfnDoWrite)( szBuffer );
}

//-----------------------------------------------------------------------------------

void DebugMsgWriter::Flush()
{
    assert( m_hHandle );

    for ( StringListCIter citer = m_lstMsgs.begin(); citer != m_lstMsgs.end(); ++citer )
    {
        fprintf( m_hHandle, (*citer).c_str() );
    }
    fflush( m_hHandle );
    m_lstMsgs.clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool DebugMsgWriter::DoWriteBuffering( char const* msg )
{
    assert( msg );

    m_lstMsgs.push_back( msg );
    return true;
}

//-----------------------------------------------------------------------------------

bool DebugMsgWriter::DoWriteImmediate( char const* msg )
{
    assert( msg && m_hHandle );

    fprintf( m_hHandle, msg );
    fflush( m_hHandle );
    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

namespace TimerUtils
{
    namespace { 
        LARGE_INTEGER g_uFreq;
        LARGE_INTEGER g_uInitial;
    }

    //-----------------------------------------------------------------------------------

    void ResetBaseTime()
    {
        ::QueryPerformanceFrequency( &g_uFreq );
        ::QueryPerformanceCounter( &g_uInitial );
    }

    //-----------------------------------------------------------------------------------

    float GetCurrentTimeInSec()
    {
        static bool bFirst = true;
        if ( bFirst )
        {
            ResetBaseTime();
            bFirst = false;
        }

        LARGE_INTEGER uCounter;
        ::QueryPerformanceCounter( &uCounter );
        return (float)((long double)(uCounter.QuadPart - g_uInitial.QuadPart) / (long double)g_uFreq.QuadPart);
    }

    //-----------------------------------------------------------------------------------

    unsigned int GetPerformanceCounter()
    {
        LARGE_INTEGER uCounter;
        ::QueryPerformanceCounter( &uCounter );
        return uCounter.LowPart;
    }

    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------

    StopWatch::StopWatch()
    {
        Reset();
    }

    //-----------------------------------------------------------------------------------

    StopWatch::~StopWatch()
    {
        /* Nothing */
    }

    //-----------------------------------------------------------------------------------

    void StopWatch::Start()
    {
        if ( -1.0f == m_fClickTimer )
        {
            m_fClickTimer = GetCurrentTimeInSec();
        }
    }

    //-----------------------------------------------------------------------------------

    void StopWatch::Stop()
    {
        if ( -1.0f != m_fClickTimer )
        {
            m_fClickTime  = GetCurrentTimeInSec() - m_fClickTimer;
            m_fClickTimer = -1.0f;
        }
    }

} //namespace TimerUtils

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool cdecl OutputDebugFile( char const* format, ... )
{
    std::shared_ptr<FILE> spFile(fopen("debug.log", "at"), fclose);
    if ( NULL == spFile )
    {
        assert( spFile );
        return false;
    }

    char szBuffer[4096] = {0,};

    va_list kArgs;
    va_start( kArgs, format );
    vsprintf_s( szBuffer, _countof(szBuffer)-1, format, kArgs );
    szBuffer[ _countof(szBuffer)-1 ] = '\0';
    va_end( kArgs );

    fprintf( spFile.get(), szBuffer );
    return true;
}

//-----------------------------------------------------------------------------------

#ifndef EXTERNAL_RELEASE
    #pragma optimize( "", off )
    void SceneNodeDeleteDebugLogout_DepthFirst( NiAVObject* pkSceneNodeRoot )
    {
        assert( pkSceneNodeRoot );

        static int s_iCount = 0;

        struct _Functor
        {
            void operator() ( NiAVObject const* pkObject )
            {
                NiAVObject* pkAVObject = const_cast<NiAVObject*>(pkObject);
                NiNode* pkNode = pkAVObject->GetParent();
                if ( pkNode )
                {
                    NiAVObjectPtr spAVObj = pkNode->DetachChild( pkAVObject );
                    OutputDebugFile( "[%04d] %s (Parent:%s)\n", s_iCount++, 
                                     (const char*)spAVObj->GetName(), (const char*)pkNode->GetName() );
                }
                else
                {
                    OutputDebugFile( "[WARNING] %s\n", (const char*)pkAVObject->GetName() );
                }
            }
        } kFunctor;
        Scene::NodeTraversal::DepthFirst::AllObjects_Upward( pkSceneNodeRoot, kFunctor );

        OutputDebugFile( "--------------------------------------------------------------------\n" );
    }
    #pragma optimize( "", on )
#else
    void SceneNodeDeleteDebugLogout_DepthFirst( NiAVObject* pkSceneNodeRoot )
    {
        /* Nothing - Skipping */
    }
#endif //#ifndef EXTERNAL_RELEASE

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Tools

} //namespace NewWare
