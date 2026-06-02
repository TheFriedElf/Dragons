
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DrawBound.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_DRAWBOUND_H__
#define _RENDERER_DRAWBOUND_H__

#include "../Tools/Noncopyable.h"


namespace NewWare
{

namespace Renderer
{

class StreamPack;


class DrawBound : public Tools::Noncopyable
{
public:
    enum DrawMode
    {
        DRAWMODE_NONE = -1,

        DRAWMODE_LEGACY = 0,    // 예전 그리기 모드.
        DRAWMODE_NOTHREAD,      // 일반적인 그리기 모드(가장 많이씀) - 가장 안정적임.
        DRAWMODE_THREAD,        // 최적화 그리기 모드(스레드 이용). - 주의: 현재 드래고니카가 Update/Render가 분리(UI가 가장 문제)가 안되어서 스레드 모드 이용 불가!(그래픽 깨짐 발생)

        DRAWMODE_ARTIFICIAL,    // 인위적인 연출용 그리기 모드 #1.
        DRAWMODE_DEBUG,         // 표준(Debug)용으로 그리기 모드.
    };


public:
    virtual ~DrawBound() = 0 { /* Nothing */ };


    virtual bool Build() = 0; // Forward calling.
    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack ) = 0; // Forward calling.

    virtual NiAccumulator* GetAccumulator() const = 0; // Forward calling.

    virtual const std::string GetClassName() const = 0;
    virtual DrawMode GetDrawMode() const = 0;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_DRAWBOUND_H__
