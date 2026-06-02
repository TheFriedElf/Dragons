
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OverlayDrawer.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_TASK_OVERLAYDRAWER_H__
#define _RENDERER_TASK_OVERLAYDRAWER_H__

#include "../TaskDecorator.h"
#include "../../Tools/CreateProxy.h"


namespace NewWare
{

namespace Renderer
{

namespace Task
{


class OverlayDrawer : public TaskDecorator, 
                      public Tools::CreateProxy<OverlayDrawer, DrawBound>
{
public:
    explicit OverlayDrawer( DrawBound* pkDrawBound );
    virtual ~OverlayDrawer();


    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack );

    virtual const std::string GetClassName() const { return "OverDrawer"; };


protected:
    void DrawInGame( float fFrameTime );
    void DrawUIScreen( float fFrameTime, StreamPack const& kStreamPack );


private:
    PgRenderer* m_pkPgRenderer;
    NiAlphaAccumulatorPtr m_spAccumulator;
};


} //namespace Task

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_TASK_OVERLAYDRAWER_H__
