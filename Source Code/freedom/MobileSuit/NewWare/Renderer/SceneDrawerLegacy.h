
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneDrawerLegacy.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_SCENEDRAWERLEGACY_H__
#define _RENDERER_SCENEDRAWERLEGACY_H__

#include "DrawBound.h"
#include "../Tools/CreateProxy.h"

class PgAlphaAccumulator;
NiSmartPointer(PgAlphaAccumulator);
class PgBatchRender;


namespace NewWare
{

namespace Renderer
{

class StreamPack;


class SceneDrawerLegacy : public DrawBound, 
                          public Tools::CreateProxy<SceneDrawerLegacy, DrawBound>
{
public:
    SceneDrawerLegacy();
    virtual ~SceneDrawerLegacy();


    virtual bool Build();
    virtual bool Execute( float fFrameTime, StreamPack const& kStreamPack );

    virtual NiAccumulator* GetAccumulator() const { return NiSmartPointerCast(NiAccumulator, m_spAccumulator); };

    virtual const std::string GetClassName() const { return "SceneDrawerLegacy"; };
    virtual DrawMode GetDrawMode() const { return DrawBound::DRAWMODE_LEGACY; };


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

    PgAlphaAccumulatorPtr m_spAccumulator;
    PgBatchRender* m_pkBatchRender;

    PgRenderer* m_pkPgRenderer;
};

PgBatchRender* GetLegacyBatchRender();


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_SCENEDRAWERLEGACY_H__
