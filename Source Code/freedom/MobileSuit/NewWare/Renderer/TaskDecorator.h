
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : TaskDecorator.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/11/02 LeeJR First Created
//

#ifndef _RENDERER_TASKDECORATOR_H__
#define _RENDERER_TASKDECORATOR_H__

#include "DrawBound.h"


namespace NewWare
{

namespace Renderer
{


class TaskDecorator : public DrawBound
{
public:
    explicit TaskDecorator( DrawBound* pkDrawBound ) 
        : m_pkDrawBound(pkDrawBound) { /* Nothing */ };
    virtual ~TaskDecorator() { delete m_pkDrawBound; };


    virtual bool Build() { assert( m_pkDrawBound ); return m_pkDrawBound->Build(); };
    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack )
    {
        assert( m_pkDrawBound );
        return m_pkDrawBound->Execute( fFrameTime, kStreamPack );
    };

    virtual NiAccumulator* GetAccumulator() const
    {
        assert( m_pkDrawBound );
        return m_pkDrawBound->GetAccumulator();
    }

    virtual DrawMode GetDrawMode() const
    {
        assert( m_pkDrawBound );
        return m_pkDrawBound->GetDrawMode();
    }


private:
    DrawBound* m_pkDrawBound;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_TASKDECORATOR_H__
