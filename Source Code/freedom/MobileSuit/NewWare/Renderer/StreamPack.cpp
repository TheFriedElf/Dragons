
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : StreamPack.cpp
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
#include "StreamPack.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

StreamPack::StreamPack( NiColor const& kClearColor, bool bDumpScene, float fAccumTime, 
                        bool bUseProjShadow, bool bRenderObject, NiCamera* pkCamera )
    : m_kClearFBufferColor(kClearColor), 
      m_bDumpScene(bDumpScene), 
      m_fAccumTime(fAccumTime), 
      m_bUseProjShadow(bUseProjShadow), 
      m_bRenderObjectFocusFilter(bRenderObject), 
      m_spCamera(pkCamera)
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
