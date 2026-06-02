
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : NewWare.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/11/04 LeeJR First Created
//

#ifndef _NEWWARE_H__
#define _NEWWARE_H__


#include <Shiny/Shiny.h>
#ifdef _DEBUG
    #pragma comment(lib, "ShinyDebug.lib")
#else
    #pragma comment(lib, "Shiny.lib")
#endif //#ifdef _DEBUG

#include "Tools/CommonTools.h"


namespace NewWare
{
    namespace OptionFlag
    {
        void EnableDumpScene( bool bEnable );
        bool EnableDumpScene();

    } //namespace OptionFlag


    // Performance profiler
    namespace PerfProfiler
    {
        void OutputReport( char const* filename, bool bOutputDebugString = false );
    } //namespace PerfProfiler

} //namespace NewWare


#endif //_NEWWARE_H__
