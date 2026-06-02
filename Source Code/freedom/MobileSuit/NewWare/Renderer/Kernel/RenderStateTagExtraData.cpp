
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderStateTagExtraData.cpp
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
#include "RenderStateTagExtraData.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

NiImplementRTTI(RenderStateTagExtraData, NiExtraData);

//-----------------------------------------------------------------------------------

RenderStateTagExtraData::RenderStateTagExtraData()
    : m_iRenderStateNumber(RESET_RENDERSTATE), 
      m_dwTextureHashKey(0), 
      m_dwCreationNumber(0xFFFFFFFF)
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
