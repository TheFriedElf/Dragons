
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderGroupPool.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERGROUPPOOL_H__
#define _RENDERER_KERNEL_RENDERGROUPPOOL_H__

#include "../../Tools/MemPool.h"
#include "RenderGroup.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


enum { RENDERGROUP_POOL_COUNT = 3072 };

class RenderGroupPool : public RenderGroup, 
                        public Tools::MemPool<RenderGroup, RENDERGROUP_POOL_COUNT>
{
    /* Nothing */
};


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERGROUPPOOL_H__
