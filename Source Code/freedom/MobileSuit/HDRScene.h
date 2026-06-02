// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2006 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net

#ifndef HDRSCENE_H
#define HDRSCENE_H

#include "PgIPostProcessEffect.h"
//#include "MRTShadersEffect.h"
#include <NiMain.h>

#if defined(_XENON)
class NiXenonRenderer;
typedef NiXenonRenderer NiPlatformRenderer;
#elif defined(WIN32)
class NiDX9Renderer;
typedef NiDX9Renderer NiPlatformRenderer;
#elif defined(_PS3)
#include <NiPS3Renderer.h>
typedef NiPS3Renderer NiPlatformRenderer;
#endif

/*
 * HDRScene handles all operations of generating effects for "HDR lighting"
 * in this demo.
 * 
 * Specifically, it performs a sequence of steps to calculate an average
 * scene luminance.  This value is then used to perform a bright pass filter
 * on the rendered scene to control what in the image will generate "blooms".
 * The "blooms" are then created by successive blurring operations done on the
 * bright pass results.  Additionally, the active luminance value 
 * incorporates an adaptation control to "smooth" the active luminance and 
 * and prevent jarring jumps in brightness in the scene.  This adaptation
 * is done to mimic the human eye adaptation to bright sources.  Input
 * to the HDR scene comes from the floating point render target to which the
 * scene was rendered.  This target encodes HDR lighting values from light
 * sources that have large multipliers or HDR textures as their own 
 * representation (as in the case of the HDR diffuse and specular cube maps
 * that light the scene in MetalWars).  Output from the HDR operations is 
 * fed to a set of potential MRT-based effects.
 * 
 * This code and associated shaders are based directly on concepts from 
 * the Microsoft DirectX 9 "HDRLighting" sample.
*/

class HDRScene : public PgIPostProcessEffect
{
public:
    HDRScene(bool bActive);

    virtual int GetType() const { return PgIPostProcessEffect::HDR; };
    virtual bool CreateEffect();
    virtual void CleanupEffect();
    virtual NiRenderTargetGroup* GetRenderTargetGroup();
    virtual NiRenderedTexture* GetTargetTexture();
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer,
        NiRenderTargetGroup* pkRenderTarget, 
        NiRenderedTexture* pkTexture);

    enum MenuID {
        MENUID_TOGGLEHDR = 0,
        MENUID_DEBUGHDR,
        MENUID_DIFFLIGHTMULTIPLIER,
        MENUID_SPECLIGHTMULTIPLIER,
        MENUID_POINTLIGHTMULTIPLIER,
        MENUID_BRIGHTPASSTHRESHOLD,
        MENUID_BRIGHTPASSOFFSET,
        MENUID_MIDDLEGRAY,
        MENUID_BLOOMSCALE,
        MENUID_TOGGLEADAPTATION,
        MENUID_ADAPTATIONSCALE,
        MAX_IDS
    };

    void ToggleDebug();
    void ToggleHDR();
    void ToggleAdaptation();

    virtual void HandleReset() { m_bFirstLumaCalc = true; }


    class Point4 : public NiMemObject
    {
    public:
        Point4(float X, float Y, float Z, float W) : 
          x(X), y(Y), z(Z), w(W) { /* */ }
        Point4() { /**/ }
        float x, y, z, w;
    };

    class AuxBuffer : public NiMemObject
    {
    public:
        AuxBuffer() { m_pkRectDest = 0; }
        ~AuxBuffer() { SAFE_DELETE_NI(m_pkRectDest); }
        NiRenderedTexturePtr m_spTex;
        NiRenderTargetGroupPtr m_spRT;
        NiScreenElementsPtr m_spGeom;
        NiRect<int>* m_pkRectDest;
    };

    void CreateAuxBuffer(unsigned int uiWidth, unsigned int uiHeight,
        NiTexture::FormatPrefs& kPrefs, AuxBuffer& kBuffer);
    void CreateRenderedTextures();
    void CreateAuxBuffers();
    void CleanupAuxBuffer(AuxBuffer& kBuffer);
    void CleanupRenderedTextures();
    void CreateScreenSpaceDebug();

    // "Pass" draw routines
    void DrawHDRScene();
    void DrawHDRSceneScaled();
    void MeasureLuminance();
    void CalculateAdaptation(float fElapsedTime);
    void UpdateImmediateLuminance();
    void DrawBrightPassFromSceneScaled();
    void DrawStarSourceFromBrightPass();
    void DrawBloomSourceFromStarSource();
    void RenderBloom();
    void DrawFinalToDisplay(NiRenderTargetGroup* pkRenderTarget);

    // Static helper functions for sampling 
    static void GetSampleOffsets_DownScale4x4(unsigned int uiWidth, 
        unsigned int uiHeight, NiPoint2 akSampleOffsets[]);
    static void GetSampleOffsets_DownScale2x2(unsigned int uiWidth, 
        unsigned int uiHeight, NiPoint2 akSampleOffsets[]);
    static void GetSampleOffsets_GaussBlur5x5(unsigned int uiTexWidth,
        unsigned int uiTexHeight, NiPoint2 akTexCoordOffsets[],
        Point4 akSampleWeights[], float fMultiplier = 1.0f);
    static void GetSampleOffsets_Bloom(unsigned int uiTexSize,
        float afTexCoordOffset[], Point4 akColorWeight[],
        float fDeviation, float fMultiplier);
    static float GaussianDistribution(float x, float y, float rho);

    void UpdateSampleConstants(NiPoint2* pkOffsets, 
        Point4* pkWeights = 0);

    void CreateAuxBufferGeom(const AuxBuffer& kSrcBuffer, bool bSrcBorder, 
        AuxBuffer& kDestBuffer, bool bDestBorder, NiMaterial* pkMaterial,
        NiTexture* pkShaderMap0 = 0, NiTexture* pkShaderMap1 = 0);
    void DrawFullscreenEffect(AuxBuffer& kDestBuffer);

    NiPlatformRenderer* m_pkD3DRenderer;

    NiScreenElementsArray m_kDebugImages;

    // HDR textures for Ni and DX9
    unsigned int m_uiHDRSceneWidth;
    unsigned int m_uiHDRSceneHeight;

    enum 
    { 
        MAX_SAMPLES = 16, 
#ifdef _XENON_NO_SMALL_RENDERTARGETS
        // We insert additional stages to blur, since tiny viewports
        // don't yet work well on Xenon
        NUM_TONEMAP_TEXTURES = 8,
#else
        NUM_TONEMAP_TEXTURES = 4,
#endif
        NUM_BLOOM_TEXTURES = 3
    };

    NiDepthStencilBuffer* m_pkDepthStencil;

    // W x H
    AuxBuffer m_kHDRScene;

    // W/4 x H/4
    AuxBuffer m_kHDRSceneScaled;

    // 2^(2i) x 2^(2i):   (1x1, 4x4, 16x16, 64x64)
    AuxBuffer m_akToneMaps[NUM_TONEMAP_TEXTURES];

    // 1x1: These are pointers because we swap them
    AuxBuffer* m_pkAdaptedLumCur;
    AuxBuffer* m_pkAdaptedLumLast;

    // W/4+2 x W/4+2
    AuxBuffer m_kBrightPass;
    AuxBuffer m_kStarSource;

    // W/8+2 x W/8+2
    AuxBuffer m_kBloomSource;
    AuxBuffer m_aspBlooms[NUM_BLOOM_TEXTURES];

    // WxH: "Dummy" AuxBuffer for output render target
    AuxBuffer m_kOutputBuffer;

    // Materials
    NiMaterialPtr m_spDebugLumMaterial;
    NiMaterialPtr m_spDebugLumLogMaterial;
    NiMaterialPtr m_spPassThroughMaterial;
    NiMaterialPtr m_spCalculateAdaptedLumMaterial;
    NiMaterialPtr m_spFinalScenePassMaterial;

    float m_fLastFrameTime;

    bool m_bDebugPass;
    bool m_bAdaptLum;
    bool m_bFirstLumaCalc;
};

#endif  
