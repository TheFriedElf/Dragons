
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : StreamPack.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_STREAMPACK_H__
#define _RENDERER_STREAMPACK_H__


namespace NewWare
{

namespace Renderer
{


class StreamPack
{
public:
    StreamPack( NiColor const& kClearColor, bool bDumpScene, float fAccumTime, 
                bool bUseProjShadow, bool bRenderObject, NiCamera* pkCamera );
    ~StreamPack() { /* Nothing */ };


    NiColor const& GetClearFBufferColor() const { return m_kClearFBufferColor; };
    bool GetDumpScene() const { return m_bDumpScene; };
    float GetAccumTime() const { return m_fAccumTime; };
    bool UseProjShadow() const { return m_bUseProjShadow; };
    bool IsRenderObjectFocusFilter() const { return m_bRenderObjectFocusFilter; };
    NiCamera* GetCamera() const { return m_spCamera; };


private:
    NiColor const& m_kClearFBufferColor;
    bool const m_bDumpScene;
    float const m_fAccumTime;
    bool const m_bUseProjShadow;
    bool const m_bRenderObjectFocusFilter;
    NiCameraPtr m_spCamera;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_STREAMPACK_H__
