
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : DrawWorkflow.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_DRAWWORKFLOW_H__
#define _RENDERER_DRAWWORKFLOW_H__

#include "../Tools/Singleton.h"
#include "../Tools/Noncopyable.h"
#include "../Tools/Compiles.h"
#include "DrawBound.h"


namespace NewWare
{

namespace Renderer
{

namespace Task { 
    class Flipper;
} //namespace Task
class StreamPack;


class DrawWorkflow : public Tools::Singleton<DrawWorkflow>, 
                     public Tools::Noncopyable, 
                     Sealed(Tools)
{
public:
    explicit DrawWorkflow( NiRenderer* pkRenderer );
    ~DrawWorkflow();


    bool Create( DrawBound::DrawMode eMode );
    bool Execute( float fFrameTime, StreamPack const& kStreamPack );
    void Destroy();


    NiAccumulator* GetAccumulator() const { assert( m_pkDrawBound ); return m_pkDrawBound->GetAccumulator(); };
    DrawBound::DrawMode GetDrawMode() const
    {
        if ( m_pkThreadFlipper )
            return DrawBound::DRAWMODE_THREAD;
        else if ( m_pkDrawBound )
            return m_pkDrawBound->GetDrawMode();
        else
            return DrawBound::DRAWMODE_NONE;
    }


private:
    bool Organize( DrawBound::DrawMode eMode );

    bool RenderCPUBound( float fFrameTime, StreamPack const& kStreamPack );


private:
    NiRenderer* m_pkRenderer;

    DrawBound* m_pkDrawBound;
    Task::Flipper* m_pkThreadFlipper;
};


void SetRenderStateTagExtraDataNumber( NiGeometry& kObject, int iRenderStateNumber );
void RefreshRenderStateBlockCaches();
void DrawActor( PgRenderer* pkRenderer, PgActor* pkActor );


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_DRAWWORKFLOW_H__
