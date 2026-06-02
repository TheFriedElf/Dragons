
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionRenderStateD3D.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONRENDERSTATED3D_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONRENDERSTATED3D_H__


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


class OcclusionRenderStateD3D
{
public:
    enum State
    {
        NOT_SET, 
        COLOR_DRAW_STATE, 
        DEPTH_DRAW_STATE, 
        DEPTH_TEST_STATE
    };


public:
    OcclusionRenderStateD3D();


    bool SetRenderState( State const eState );


private:
    void ColorDrawState( NiDX9RenderState* pkRenderState );
    void DepthDrawState( NiDX9RenderState* pkRenderState );
    void DepthTestState( NiDX9RenderState* pkRenderState );


private:
    State m_eStateCache;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONRENDERSTATED3D_H__
