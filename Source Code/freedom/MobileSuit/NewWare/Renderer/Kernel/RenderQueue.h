
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderQueue.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/26 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERQUEUE_H__
#define _RENDERER_KERNEL_RENDERQUEUE_H__

#include <array>

namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


class RenderQueue : public Tools::Singleton<RenderQueue>
{
public:
    enum PassGroupLayer
    {
        PASSGROUP_1 = 0, 
        PASSGROUP_2, 

        PASSGROUP_MAX_COUNT
    };


public:
    RenderQueue();
    ~RenderQueue();


    void Draw( NiRenderer* pkRenderer, NiCamera* pkCamera );
    void Draw( PassGroupLayer eLayer, NiRenderer* pkRenderer, NiCamera* pkCamera );


public:
    unsigned int AddUnSorted( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera );
    unsigned int AddSorted( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera );
    unsigned int AddObject( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera );
    void AddLensFilter( PassGroupLayer eLayer, NiScreenTexture* pkObject );
    unsigned int AddOverlay( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera );


private:
    class PassGroup;
    std::array<PassGroup*, PASSGROUP_MAX_COUNT> m_apkPassGroupList;

    NiCullingProcess m_kCuller;
};


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERQUEUE_H__
