
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : BatchRenderUtils.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_BATCHRENDERUTILS_H__
#define _RENDERER_KERNEL_BATCHRENDERUTILS_H__

#include "../../Scene/FindTraversal.h"
#include "RenderStateBlockCache.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{

struct RenderGroup;


// Opaque Batch Render utilities.
namespace BatchRenderUtils
{
    void DrawUnSortedRenderGroup( NiRenderer* pkRenderer, RenderGroup const& kGroup, 
                                  RenderStateBlockCache::RenderStateBlock const& kStateBlock );
    void DrawSortedRenderGroup( NiRenderer* pkRenderer, RenderGroup const& kGroup, 
                                RenderStateBlockCache::RenderStateBlock const& kStateBlock );


    inline bool HasGeomMorpherController( NiGeometry const& kObject ) { 
        return Scene::FindTraversal::FindController<NiGeomMorpherController>( &kObject ) != NULL;
    }
    inline bool HasTextureTransformController( NiGeometry const& kObject ) { 
        return Scene::FindTraversal::FindControllerFromPropertyList<NiTextureTransformController>( &kObject ) != NULL;
    }

    inline bool IsBatchable( NiGeometry& kObject ) { 
        return !(HasGeomMorpherController(kObject) || HasTextureTransformController(kObject));
    }

} //namespace BatchRenderUtils


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_BATCHRENDERUTILS_H__
