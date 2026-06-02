
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : SceneDrawerDebug.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/04/08 LeeJR First Created
//

#ifndef _RENDERER_SCENEDRAWERDEBUG_H__
#define _RENDERER_SCENEDRAWERDEBUG_H__

#include "DrawBound.h"
#include "../Tools/CreateProxy.h"


namespace NewWare
{

namespace Renderer
{

class StreamPack;


class SceneDrawerDebug : public DrawBound, 
                         public Tools::CreateProxy<SceneDrawerDebug, DrawBound>
{
public:
    SceneDrawerDebug();
    virtual ~SceneDrawerDebug();


    virtual bool Build();
    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack );

    virtual NiAccumulator* GetAccumulator() const { return NiSmartPointerCast(NiAccumulator, m_spAccumulator); };

    virtual const std::string GetClassName() const { return "SceneDrawerDebug"; };
    virtual DrawMode GetDrawMode() const { return DrawBound::DRAWMODE_DEBUG; };


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

    NiAlphaAccumulatorPtr m_spAccumulator;

    PgRenderer* m_pkPgRenderer;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_SCENEDRAWERDEBUG_H__
