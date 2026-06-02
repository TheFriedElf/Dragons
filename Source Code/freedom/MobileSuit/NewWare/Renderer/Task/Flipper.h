
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Flipper.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_TASK_FLIPPER_H__
#define _RENDERER_TASK_FLIPPER_H__

#include "../TaskDecorator.h"
#include "../../Tools/CreateProxy.h"


namespace NewWare
{

namespace Renderer
{

namespace Task
{


class Flipper : public TaskDecorator, 
                public Tools::CreateProxy<Flipper, DrawBound>, 
                private Tools::Singleton<Flipper>
{
public:
    Flipper( DrawBound* pkDrawBound, NiRenderer* pkRenderer );
    virtual ~Flipper();


    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack );

    virtual const std::string GetClassName() const { return "Flipper"; };


    void ResumeEvent() { PROFILE_FUNC(); ::SetEvent( ms_hResume ); };
    void ExitEvent() { ::SetEvent( ms_hExit ); };
    void WaitForFinishWorkEvent()
    {
        PROFILE_FUNC();
        ::WaitForMultipleObjects( 1, &ms_hFinishWork, FALSE, INFINITE );
    }


private:
    bool RenderGPUBound()
    {
        assert( m_pkRenderer );
        if ( m_pkRenderer->EndFrame() == false )
            return false;
        return m_pkRenderer->DisplayFrame();
    }

    static void _cdecl RenderThread( void* arg );


private:
    NiRenderer* m_pkRenderer;

    static HANDLE ms_hResume;
    static HANDLE ms_hExit;
    static HANDLE ms_hFinishWork;
};


} //namespace Task

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_TASK_FLIPPER_H__
