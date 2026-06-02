
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : NewWare.cpp
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
#include "NewWare.h"


namespace NewWare
{
    namespace OptionFlag
    {
        namespace { 
            bool g_bDumpScene = false;
        } //namespace

        void EnableDumpScene( bool bEnable )
        {
            g_bDumpScene = bEnable;
        }
        bool EnableDumpScene()
        {
            return g_bDumpScene;
        }

    } //namespace OptionFlag


    namespace PerfProfiler
    {
        void OutputReport( char const* filename, bool bOutputDebugString )
        {
            assert( filename );

            PROFILER_UPDATE(); // Update all profiles
            PROFILER_OUTPUT( filename ); // print to file.

            if ( bOutputDebugString )
            {
                std::stringstream kOstream;
                PROFILER_OUTPUT( kOstream ); // print to string.
                ::OutputDebugStringA( kOstream.str().c_str() );
            }
        }
    } //namespace PerfProfiler

} //namespace NewWare
