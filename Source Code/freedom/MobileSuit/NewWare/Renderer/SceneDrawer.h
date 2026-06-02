
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneDrawer.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_SCENEDRAWER_H__
#define _RENDERER_SCENEDRAWER_H__

#include "DrawBound.h"
#include "../Tools/CreateProxy.h"


namespace NewWare
{

namespace Renderer
{

class StreamPack;

namespace Kernel { 
    class GroupAccumulator;
    NiSmartPointer(GroupAccumulator);
} //namespace Kernel


class SceneDrawer : public DrawBound, 
                    public Tools::CreateProxy<SceneDrawer, DrawBound>
{
public:
    SceneDrawer();
    virtual ~SceneDrawer();


    virtual bool Build();
    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack );

    virtual NiAccumulator* GetAccumulator() const { return NiSmartPointerCast(NiAccumulator, m_spAccumulator); };

    virtual const std::string GetClassName() const { return "SceneDrawer"; };
    virtual DrawMode GetDrawMode() const { return DrawBound::DRAWMODE_NOTHREAD; };


private:
    void DrawRenderToTexture( float fFrameTime );
    void DrawActorShadow();
    void DrawSpotLightAffectedShadow( float fAccumTime );

    void SetFrameBufferClearColor( NiColor const& kColor );
    void BeginPostEffectFrame( float fFrameTime );
    void BeginFrustum();
    void DrawFrame( float fFrameTime, StreamPack const& kStreamPack );
    void EndFrustum();
    void EndPostEffectFrame( float fFrameTime );


private:
    class Pimple; // Pimple idiom
    Pimple* m_pkPimple;

    Kernel::GroupAccumulatorPtr m_spAccumulator;

    PgRenderer* m_pkPgRenderer;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_SCENEDRAWER_H__
