
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : CommonTools.h
// Description : Common tools for benchmark module
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_COMMONTOOLS_H__
#define _BENCHMARK_TOOLS_COMMONTOOLS_H__

#include "Singleton.h"
#include "Noncopyable.h"
#include "Compiles.h"


namespace NewWare
{

namespace Tools
{


class DebugMsgWriter : public Singleton<DebugMsgWriter>, 
                       public Noncopyable, 
                       Sealed(Tools)
{
public:
    enum { STRINGLINE_LENGTH = 4096 };
    enum WriteMode { BUFFERING_MODE, IMMEDIATE_MODE };

    typedef bool (DebugMsgWriter::*DoWriteFunc)( char const* );


private:
    typedef std::list< std::string >    StringList;
    typedef StringList::iterator        StringListIter;
    typedef StringList::const_iterator  StringListCIter;


public:
    DebugMsgWriter();
    ~DebugMsgWriter();


    bool Create( char const* filename, WriteMode eMode = IMMEDIATE_MODE, char const* openOption = "wt" );
    void Destory();

    bool cdecl Write( char const* format, ... );

    void Flush();

    WriteMode GetWriteMode() const { 
        return (&DebugMsgWriter::DoWriteBuffering == m_pfnDoWrite)? BUFFERING_MODE: IMMEDIATE_MODE;
    }


private:
    bool DoWriteBuffering( char const* msg );
    bool DoWriteImmediate( char const* msg );


private:
    std::string m_strFilename;
    FILE* m_hHandle;

    StringList m_lstMsgs;

    DoWriteFunc m_pfnDoWrite;
};


template < class T > 
class OneTimeAdapter : public Noncopyable, 
                       Sealed(Tools) 
{
public:
    OneTimeAdapter() 
        : m_bCreated(false), m_ptInstance(NULL) { /* Nothing */ };
    ~OneTimeAdapter() { Stop(); };


    void Start( void (*OnInit)(void) = NULL )
    {
        if ( false == m_bCreated )
        {
            m_ptInstance = new T;
            assert( m_ptInstance );
            
            if ( OnInit )
                OnInit();

            m_bCreated = true;
        }
    }
    void Stop( void (*OnExit)(void) = NULL )
    {
        if ( m_bCreated )
        {
            if ( OnExit )
                OnExit();

            delete m_ptInstance;
            m_ptInstance = NULL;

            m_bCreated = false;
        }
    }

    T* GetInstance() const { assert( m_ptInstance ); return m_ptInstance; };


private:
    bool m_bCreated;
    T* m_ptInstance;
};


namespace TimerUtils
{
    void ResetBaseTime();

    float GetCurrentTimeInSec();
    unsigned int GetPerformanceCounter();

    class StopWatch : Sealed(Tools)
    {
    public:
        StopWatch();
        ~StopWatch();


        void Reset() { m_fClickTimer = -1.0f; m_fClickTime = 0.0f; };

        void Start();
        void Stop();

        float GetTime() const { return m_fClickTime; };

    private:
        float m_fClickTimer;
        float m_fClickTime;
    };
} //namespace TimerUtils


bool cdecl OutputDebugFile( char const* format, ... );

void SceneNodeDeleteDebugLogout_DepthFirst( NiAVObject* pkSceneNodeRoot );


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_COMMONTOOLS_H__
