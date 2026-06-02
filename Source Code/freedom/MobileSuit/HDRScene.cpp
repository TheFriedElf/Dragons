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

// Precompiled Header
#include <stdafx.h>

#include "HDRScene.h"

#if defined(WIN32)
#include <NiDirectXVersion.h>
#elif defined(_XENON)
#include <NiXenonRenderer.h>
#endif

#include <NiShaderFactory.h>

//---------------------------------------------------------------------------
HDRScene::HDRScene(bool bActive) : PgIPostProcessEffect(bActive),
    // this value should match the global one in the .fx -- otherwise
    // turning on and off HDR will have different results after first toggle
    m_pkDepthStencil(0),
    m_fLastFrameTime(0.0f),
    m_bDebugPass(false),
    m_bAdaptLum(true),
    m_bFirstLumaCalc(true)
{
    /* */
}
//---------------------------------------------------------------------------
bool HDRScene::CreateEffect()
{
    // Be sure we can perform this effect
#if defined(_XENON)
    m_pkD3DRenderer = NiDynamicCast(NiXenonRenderer, 
        NiRenderer::GetRenderer());
#elif defined(WIN32)
    m_pkD3DRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());
#elif defined(_PS3)
    m_pkD3DRenderer = NiDynamicCast(NiPS3Renderer, 
        NiRenderer::GetRenderer());
#endif
    assert(m_pkD3DRenderer);

    m_uiHDRSceneWidth = m_pkD3DRenderer->GetDefaultRenderTargetGroup()->
        GetWidth(0);
    m_uiHDRSceneHeight = m_pkD3DRenderer->GetDefaultRenderTargetGroup()->
        GetHeight(0);

    // Generate all the DX rendered textures that will be necessary for effect
    CreateAuxBuffers();

    m_spDebugLumMaterial = NiSingleShaderMaterial::Create("HDRScene_DebugLum");
    assert(m_spDebugLumMaterial);
    m_spDebugLumLogMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_DebugLumLog");
    assert(m_spDebugLumLogMaterial);
    m_spPassThroughMaterial = 
        NiSingleShaderMaterial::Create("MRT_PassThrough");
    assert(m_spPassThroughMaterial);
    m_spCalculateAdaptedLumMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_CalculateAdaptedLum");
    assert(m_spCalculateAdaptedLumMaterial);
    m_spFinalScenePassMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_FinalScenePass");
    assert(m_spFinalScenePassMaterial);

    CreateScreenSpaceDebug();
    return true;
}
//---------------------------------------------------------------------------
void HDRScene::CleanupEffect()
{
    CleanupRenderedTextures();
}
//---------------------------------------------------------------------------
#define HDR_DEBUG_NEXT_IMAGE() \
    if (!(++uiScreenCount % uiScreensAcross)) \
    { \
        fLeft = 0; \
        fTop -= fHeight; \
    } \
    else \
    { \
        fLeft += fWidth; \
    }
//---------------------------------------------------------------------------
void HDRScene::CreateScreenSpaceDebug()
{
    // Add screen space debug geometry using NiScreenGeometry
    const unsigned int uiScreensAcross = 7;
    float fLeft = 0;
    float fWidth = 1.0f / uiScreensAcross;
    float fHeight = fWidth * 0.75f;
    float fTop = 0.9f - fHeight;  // 1 == bottom of screen, .9 for TV safe zone
    unsigned int uiScreenCount = 0;

    // HDRScene
    NiScreenElements* pkSE = NiNew NiScreenElements(
        NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    int iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    NiTexturingProperty* pkTP = NiNew NiTexturingProperty;
    assert(pkTP);
    pkTP->SetBaseTexture(m_kHDRScene.m_spTex);
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // HDRSceneScaled
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_kHDRSceneScaled.m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // Luminance
    for (unsigned int ui = 0; ui < NUM_TONEMAP_TEXTURES; ui++)
    {
        pkSE = NiNew NiScreenElements(NiNew 
            NiScreenElementsData(false, false, 1));
        assert(pkSE);
        iHandle = pkSE->Insert(4);
        pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
        pkSE->UpdateBound();
        pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
        pkTP = NiNew NiTexturingProperty; 
        assert(pkTP);
        pkTP->SetBaseTexture(m_akToneMaps[
            NUM_TONEMAP_TEXTURES - 1 - ui].m_spTex);   
        pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
        pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
        pkSE->AttachProperty(pkTP);
        m_kDebugImages.Add(pkSE);
        pkSE->ApplyMaterial(m_spDebugLumLogMaterial);
        pkSE->ApplyMaterial(m_spDebugLumMaterial);
        pkSE->SetActiveMaterial((ui < NUM_TONEMAP_TEXTURES-1) ? 
            m_spDebugLumLogMaterial : m_spDebugLumMaterial);
        pkSE->UpdateEffects();
        pkSE->UpdateProperties();

        HDR_DEBUG_NEXT_IMAGE();
    }

    // Adapted Luminance
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_pkAdaptedLumCur->m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->ApplyMaterial(m_spDebugLumMaterial);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // Bright Pass
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_kBrightPass.m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // Star Source
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_kStarSource.m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // Bloom Source
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_kBloomSource.m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();

    // Bloom Final
    pkSE = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
    assert(pkSE);
    iHandle = pkSE->Insert(4);
    pkSE->SetRectangle(iHandle, fLeft, fTop, fWidth, fHeight);
    pkSE->UpdateBound();
    pkSE->SetTextures(iHandle, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTP = NiNew NiTexturingProperty; 
    assert(pkTP);
    pkTP->SetBaseTexture(m_aspBlooms[0].m_spTex);   
    pkTP->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    pkTP->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
    pkSE->AttachProperty(pkTP);
    m_kDebugImages.Add(pkSE);
    pkSE->UpdateEffects();
    pkSE->UpdateProperties();

    HDR_DEBUG_NEXT_IMAGE();
}
//---------------------------------------------------------------------------
void HDRScene::CreateAuxBuffer(unsigned int uiWidth, unsigned int uiHeight,
                               NiTexture::FormatPrefs& kPrefs,
                               AuxBuffer& kBuffer)
{
    assert(m_pkD3DRenderer);

    kBuffer.m_spTex = NiRenderedTexture::Create(uiWidth, uiHeight, 
        m_pkD3DRenderer, kPrefs);
    assert(kBuffer.m_spTex);

    // if needed, create the depth stencil buffer to share it across all 
    // subsequent render targets -- the first render target should be 
    // maximally sized for the ones used in this application, in order for
    // its depth surface to be useable for each subsequent render target
    if (!m_pkDepthStencil)
    {
#if defined(WIN32) || defined(_XENON)
        m_pkDepthStencil = m_pkD3DRenderer->GetDefaultRenderTargetGroup()->
            GetDepthStencilBuffer();
#elif defined(_PS3)
        m_pkDepthStencil = NiDepthStencilBuffer::CreateCompatible(
            kBuffer.m_spTex->GetBuffer(), m_pkD3DRenderer);
#endif
        assert(m_pkDepthStencil);
    }

    kBuffer.m_spRT =  NiRenderTargetGroup::Create(
        kBuffer.m_spTex->GetBuffer(), m_pkD3DRenderer, true, true);
    assert(kBuffer.m_spRT);

    // Test to be sure that the shared depth buffer is compatible
    bool bValidRTG = m_pkD3DRenderer->
        ValidateRenderTargetGroup(kBuffer.m_spRT);
    if (!bValidRTG)
    {
        m_pkDepthStencil = NiDepthStencilBuffer::CreateCompatible(
            kBuffer.m_spTex->GetBuffer(), m_pkD3DRenderer);
        assert(m_pkDepthStencil);
        kBuffer.m_spRT->AttachDepthStencilBuffer(m_pkDepthStencil);
    }
}
//---------------------------------------------------------------------------
void HDRScene::CreateRenderedTextures()
{
    assert(m_pkD3DRenderer);

    NiTexture::FormatPrefs kPrefs;

    assert((m_uiHDRSceneWidth % 8) == 0);
    assert((m_uiHDRSceneHeight % 8) == 0);

    // Specify buffer sizes
#if defined(_XENON)
    unsigned int uiSmallerWidths[2] = 
    { 
        m_uiHDRSceneWidth / 2, 
        m_uiHDRSceneWidth / 4
    };
    unsigned int uiSmallerHeights[2] = 
    { 
        m_uiHDRSceneHeight / 2, 
        m_uiHDRSceneHeight / 4
    };
#else
    unsigned int uiSmallerWidths[2] = 
    { 
        m_uiHDRSceneWidth / 4, 
        m_uiHDRSceneWidth / 8
    };
    unsigned int uiSmallerHeights[2] = 
    { 
        m_uiHDRSceneHeight / 4, 
        m_uiHDRSceneHeight / 8
    };
#endif

    // Full size HDR scene render target textures
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
    CreateAuxBuffer(m_uiHDRSceneWidth, m_uiHDRSceneHeight, kPrefs, 
        m_kHDRScene);

    // Scaled size HDR scene render target textures
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
    CreateAuxBuffer(uiSmallerWidths[0], uiSmallerHeights[0], kPrefs,
        m_kHDRSceneScaled);

    // Tone Map Textures
    // For each scale stage, create a texture to hold the intermediate results
    // of the luminance calculation
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::SINGLE_COLOR_32;
#ifdef _XENON_NO_SMALL_RENDERTARGETS
    // Xenon has trouble with very small viewports or framebuffers,
    // here a series of 16x16 buffers are created and blurred to get the 
    // lumanance of the scene.
    int i = 0;

    CreateAuxBuffer(1, 1, kPrefs, m_akToneMaps[i]);

    for (; i < NUM_TONEMAP_TEXTURES-1; i++)
    {
        CreateAuxBuffer(16, 16, kPrefs, m_akToneMaps[i]);
    }

    CreateAuxBuffer(64, 64, kPrefs, m_akToneMaps[i]);

#else
    for (int i = 0; i < NUM_TONEMAP_TEXTURES; i++)
    {
        int iSampleLen = 1 << (2 * i);

        CreateAuxBuffer(iSampleLen, iSampleLen, kPrefs, m_akToneMaps[i]);
    }
#endif

    // Two textures to track adapted luminance values for the user
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::SINGLE_COLOR_32;
    CreateAuxBuffer(1, 1, kPrefs, *m_pkAdaptedLumCur);

    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::SINGLE_COLOR_32;
    CreateAuxBuffer(1, 1, kPrefs, *m_pkAdaptedLumLast);

    // Bright pass texture
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
    CreateAuxBuffer(uiSmallerWidths[0] + 2, uiSmallerHeights[0] + 2,
        kPrefs, m_kBrightPass);

    // Star source texture
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
    CreateAuxBuffer(uiSmallerWidths[0] + 2, uiSmallerHeights[0] + 2,
        kPrefs, m_kStarSource);


    // When filtering is available, the bloom textures will use floating point
    // textures for improved quality.
#if defined(WIN32)
    LPDIRECT3D9 pkD = m_pkD3DRenderer->GetDirect3D();
    HRESULT hr = pkD->CheckDeviceFormat(
        m_pkD3DRenderer->GetAdapter(),
        m_pkD3DRenderer->GetDevType(),
        m_pkD3DRenderer->GetAdapterFormat(),
        D3DUSAGE_QUERY_FILTER,
        D3DRTYPE_TEXTURE,
        D3DFMT_A16B16G16R16F);

    if (SUCCEEDED(hr))
        kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
    else
        kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
#elif defined(_XENON)
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
#elif defined(_PS3)
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
#else
#error // unknown target
#endif

    // Bloom source texture
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    CreateAuxBuffer(uiSmallerWidths[1] + 2, uiSmallerHeights[1] + 2,
        kPrefs, m_kBloomSource);

    // Create the temporary and final blooming effect textures
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    for (int i = 1; i < NUM_BLOOM_TEXTURES; i++)
    {
        CreateAuxBuffer(uiSmallerWidths[1] + 2, uiSmallerHeights[1] + 2,
            kPrefs, m_aspBlooms[i]);
    }

    // Create the final blooming effect texture
    CreateAuxBuffer(uiSmallerWidths[1], uiSmallerHeights[1],
        kPrefs, m_aspBlooms[0]);
}
//---------------------------------------------------------------------------
void HDRScene::CleanupAuxBuffer(AuxBuffer& kBuffer)
{
    if (kBuffer.m_spTex)
        m_pkD3DRenderer->PurgeTexture(kBuffer.m_spTex);
    kBuffer.m_spTex = 0;

    kBuffer.m_spRT = 0;

    kBuffer.m_spGeom = 0;
}
//---------------------------------------------------------------------------
void HDRScene::CleanupRenderedTextures()
{
    // Release aux buffers
    CleanupAuxBuffer(m_kHDRScene);

    CleanupAuxBuffer(m_kHDRSceneScaled);

    for(int i = 0; i < NUM_TONEMAP_TEXTURES; i++)
        CleanupAuxBuffer(m_akToneMaps[i]);

    CleanupAuxBuffer(*m_pkAdaptedLumCur);
    SAFE_DELETE_NI(m_pkAdaptedLumCur);

    CleanupAuxBuffer(*m_pkAdaptedLumLast);
    SAFE_DELETE_NI(m_pkAdaptedLumLast);

    CleanupAuxBuffer(m_kBrightPass);

    CleanupAuxBuffer(m_kStarSource);

    CleanupAuxBuffer(m_kBloomSource);

    for(int i = 0; i < NUM_BLOOM_TEXTURES; i++)
        CleanupAuxBuffer(m_aspBlooms[i]);

    CleanupAuxBuffer(m_kOutputBuffer);
}
//---------------------------------------------------------------------------
void HDRScene::CreateAuxBuffers()
{
    m_pkAdaptedLumCur = NiNew AuxBuffer;
    m_pkAdaptedLumLast = NiNew AuxBuffer;

    CreateRenderedTextures();

    assert(m_pkD3DRenderer->GetDefaultRenderTargetGroup()->GetWidth(0) == 
        m_uiHDRSceneWidth);
    assert(m_pkD3DRenderer->GetDefaultRenderTargetGroup()->GetHeight(0) == 
        m_uiHDRSceneHeight);

    // Manually load materials

    // This "pass" creates a scaled-down version of the render target from
    // the previous pass.  The only object that is rendered is a 2D 
    // screen-size.  Output is a 1/4 frame-buffer sized 16-bit float 
    // per-channel rendered texture.
    
    // Create a 1/4 x 1/4 scale copy of the HDR texture. 
    // Since bloom textures are 1/8 x 1/8 scale, border texels of 
    // the HDR texture will be discarded 

    // Should be no need to do any offsets - we are mapping from the
    // entire buffer to the entire render target
    NiMaterial* spDownScale4x4Material = 
        NiSingleShaderMaterial::Create("HDRScene_DownScale4x4");
    assert(spDownScale4x4Material);

    CreateAuxBufferGeom(m_kHDRScene, false, m_kHDRSceneScaled, false,
        spDownScale4x4Material);

    // Setup texture from last "pass" (SceneScaled) that will be used 
    // for sampling in the pixel shader for this draw and generate 
    // the scaled texture.  After this pass, the 
    // g_apTexToneMap[NUM_TONEMAP_TEXTURES-1] texture will contain
    // a scaled, grayscale copy of the HDR scene. Individual texels 
    // contain the log of average luminance values for points sampled 
    // on the HDR texture.

    unsigned int uiCurrTexture = NUM_TONEMAP_TEXTURES - 1;

    NiMaterialPtr spSampleAvgLumMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_SampleAvgLum");
    assert(spSampleAvgLumMaterial);

    CreateAuxBufferGeom(m_kHDRSceneScaled, false, 
        m_akToneMaps[uiCurrTexture], false, spSampleAvgLumMaterial);

    uiCurrTexture--;

    NiMaterialPtr spResampleAvgLumMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_ResampleAvgLum");
    assert(spResampleAvgLumMaterial);

    NiMaterialPtr spGaussBlur5x5Material = 
        NiSingleShaderMaterial::Create("HDRScene_GaussBlur5x5");
    assert(spGaussBlur5x5Material);

    // Initialize the sample offsets for the iterative luminance passes
#ifdef _XENON_NO_SMALL_RENDERTARGETS
    // Xenon has trouble with very small viewports or framebuffers,
    // here a series of 16x16 buffers are created and blurred to get the 
    // lumanance of the scene.
    while (uiCurrTexture > 1)
    {
        // Draw quad to create each next scaled tone map 
        CreateAuxBufferGeom(m_akToneMaps[uiCurrTexture+1], false, 
            m_akToneMaps[uiCurrTexture], false, spResampleAvgLumMaterial);

        uiCurrTexture--;
    }

    CreateAuxBufferGeom(m_akToneMaps[uiCurrTexture+1], false, 
        m_akToneMaps[uiCurrTexture], false, spGaussBlur5x5Material);
    uiCurrTexture--;

#else
    while (uiCurrTexture > 0)
    {
        // Draw quad to create each next scaled tone map 
        CreateAuxBufferGeom(m_akToneMaps[uiCurrTexture+1], false, 
            m_akToneMaps[uiCurrTexture], false, spResampleAvgLumMaterial);

        uiCurrTexture--;
    }
#endif

    // Perform the final pass of the average luminance calculation. 
    // This pass scales the 4x4 log of average luminance texture from 
    // above and performs an exp() operation to return a single texel 
    // cooresponding to the average luminance of the scene in 
    // g_apTexToneMap[0]. 

    // Draw quad to create final luminance tone map 
    NiMaterialPtr spResampleAvgLumExpMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_ResampleAvgLumExp");
    assert(spResampleAvgLumExpMaterial);

    CreateAuxBufferGeom(m_akToneMaps[1], false, 
        m_akToneMaps[0], false, spResampleAvgLumExpMaterial);

    // This simulates the light adaptation that occurs when moving from a 
    // dark area to a bright area, or vice versa. The g_pTexAdaptedLum
    // texture stores a single texel cooresponding to the user's adapted 
    // level.

    // Draw quad to create final luminance tone map
    // We'll be swapping these back and forth, so we make them point to each 
    // other
    CreateAuxBufferGeom(*m_pkAdaptedLumCur, false, 
        *m_pkAdaptedLumLast, false, m_spCalculateAdaptedLumMaterial,
        m_akToneMaps[0].m_spTex);
    CreateAuxBufferGeom(*m_pkAdaptedLumLast, false, 
        *m_pkAdaptedLumCur, false, m_spCalculateAdaptedLumMaterial,
        m_akToneMaps[0].m_spTex);

    // Run the bright-pass filter on Scene Scaled texture and 
    // place the result in BrightPass
    // The bright-pass filter removes everything from the scene except 
    // lights and bright reflections
    NiMaterialPtr spBrightPassFilterMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_BrightPassFilter");
    assert(spBrightPassFilterMaterial);

    CreateAuxBufferGeom(m_kHDRSceneScaled, false, 
        m_kBrightPass, true, spBrightPassFilterMaterial,
        m_pkAdaptedLumCur->m_spTex);

    // Perform a 5x5 gaussian blur on BrightPass and place the result
    // in StarSource. The bright-pass filtered image is blurred before
    // being used for star operations to avoid aliasing artifacts.
    // The gaussian blur smooths out rough edges to avoid aliasing effects
    // when the star effect is run
    CreateAuxBufferGeom(m_kBrightPass, false, 
        m_kStarSource, true, spGaussBlur5x5Material);

    // Scale down StarSource by 1/2 x 1/2 and place the result in 
    // BloomSource
    // Create an exact 1/2 x 1/2 copy of the source texture

    // Draw bloom source from star source
    NiMaterialPtr spDownScale2x2Material = 
        NiSingleShaderMaterial::Create("HDRScene_DownScale2x2");
    assert(spDownScale2x2Material);

    CreateAuxBufferGeom(m_kStarSource, true, 
        m_kBloomSource, true, spDownScale2x2Material);

    // Render the blooming effect

    // Render the gaussian blur
    CreateAuxBufferGeom(m_kBloomSource, true, 
        m_aspBlooms[2], true, spGaussBlur5x5Material);

    ////////////////////////////
    // Bloom Pass for Horizontal
    NiMaterialPtr spBloomMaterial = 
        NiSingleShaderMaterial::Create("HDRScene_Bloom");
    assert(spBloomMaterial);

    CreateAuxBufferGeom(m_aspBlooms[2], true, 
        m_aspBlooms[1], true, spBloomMaterial);

    ////////////////////////////
    // Bloom Pass for Vertical
    CreateAuxBufferGeom(m_aspBlooms[1], true, 
        m_aspBlooms[0], false, spBloomMaterial);
}
//---------------------------------------------------------------------------
void HDRScene::ToggleDebug()
{
    m_bDebugPass = !m_bDebugPass;
}
//---------------------------------------------------------------------------
void HDRScene::ToggleHDR()
{
    SetActive(!GetActive()); 
}
//---------------------------------------------------------------------------
void HDRScene::ToggleAdaptation()
{
    m_bAdaptLum = !m_bAdaptLum;
}
//---------------------------------------------------------------------------
NiRenderTargetGroup* HDRScene::GetRenderTargetGroup()
{
    // Return floating point render target for scene
    // The subsequent application "pass" is the only one that draws 
    // 3D objects of the scene.  Output of that first application "pass" 
    // is a 16-bit float per-channel rendered texture.
    
    return m_kHDRScene.m_spRT;
}
//---------------------------------------------------------------------------
NiRenderedTexture* HDRScene::GetTargetTexture()
{
    // Return floating point render target for scene
    // The subsequent application "pass" is the only one that draws 
    // 3D objects of the scene.  Output of that first application "pass" 
    // is a 16-bit float per-channel rendered texture.
    
    return m_kHDRScene.m_spTex;
}
//---------------------------------------------------------------------------
void HDRScene::DrawHDRSceneScaled()
{
    // This "pass" creates a scaled-down version of the render target from
    // the previous pass.  The only object that is rendered is a 2D 
    // screen-size quad.  Output is a 1/4 frame-buffer sized 16-bit float 
    // per-channel rendered texture.
    
    // Create a 1/4 x 1/4 scale copy of the HDR texture. 
    // Since bloom textures are 1/8 x 1/8 scale, border texels of 
    // the HDR texture will be discarded 

    // Calculating down-sampling offset factors and make available to 
    // shaders
    // Get the sample offsets used within the pixel shader
    NiPoint2 akSampleOffsets[MAX_SAMPLES];
    GetSampleOffsets_DownScale4x4(m_pkD3DRenderer->
        GetDefaultRenderTargetGroup()->GetWidth(0), m_pkD3DRenderer->
        GetDefaultRenderTargetGroup()->GetHeight(0), akSampleOffsets);

    UpdateSampleConstants(akSampleOffsets);

    // Draw to create the scaled HDRScene
    DrawFullscreenEffect(m_kHDRSceneScaled);
}
//---------------------------------------------------------------------------
void HDRScene::MeasureLuminance()
{
    unsigned int uiCurrTexture = NUM_TONEMAP_TEXTURES - 1;

    // Initialize the sample offsets for the initial luminance pass.
    float fU = 1.0f / 
        (3.0f * m_akToneMaps[uiCurrTexture].m_spRT->GetWidth(0));
    float fV = 1.0f / 
        (3.0f * m_akToneMaps[uiCurrTexture].m_spRT->GetHeight(0));
    
    NiPoint2 akSampleOffsets[MAX_SAMPLES];
    int iIndex = 0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            akSampleOffsets[iIndex].x = x * fU;
            akSampleOffsets[iIndex].y = y * fV;

            iIndex++;
        }
    }
    assert(iIndex == 9);

    // Set shader constants
    UpdateSampleConstants(akSampleOffsets);
    
    // Setup texture from last "pass" (SceneScaled) that will be used 
    // for sampling in the pixel shader for this draw and generate 
    // the scaled texture.  After this pass, the 
    // g_apTexToneMap[NUM_TONEMAP_TEXTURES-1] texture will contain
    // a scaled, grayscale copy of the HDR scene. Individual texels 
    // contain the log of average luminance values for points sampled 
    // on the HDR texture.

    // Draw quad to create the first scaled tone map 
    DrawFullscreenEffect(m_akToneMaps[uiCurrTexture]);

    uiCurrTexture--;

    // Initialize the sample offsets for the iterative luminance passes
#ifdef _XENON_NO_SMALL_RENDERTARGETS
    // Xenon has trouble with very small viewports or framebuffers,
    // here a series of 16x16 buffers are created and blurred to get the 
    // lumanance of the scene.
    while (uiCurrTexture > NUM_TONEMAP_TEXTURES-1-2 /* do 2 stages */)
#else
    while (uiCurrTexture > 0)
#endif
    {
        GetSampleOffsets_DownScale4x4(
            m_akToneMaps[uiCurrTexture + 1].m_spRT->GetWidth(0), 
            m_akToneMaps[uiCurrTexture + 1].m_spRT->GetHeight(0),
            akSampleOffsets );
    
        // Each of these passes continue to scale down the log of average
        // luminance texture created above, storing intermediate results in 
        // m_aspToneMapRenderTargets[1] through 
        // m_aspToneMapRenderTargets[NUM_TONEMAP_TEXTURES-1].
        UpdateSampleConstants(akSampleOffsets);

        // Draw quad to create each next scaled tone map 
        DrawFullscreenEffect(m_akToneMaps[uiCurrTexture]);

        uiCurrTexture--;
    }

#ifdef _XENON_NO_SMALL_RENDERTARGETS
    // Blur the 16,16
    while (uiCurrTexture > 0)
    {
        Point4 akSampleWeights[MAX_SAMPLES];

        GetSampleOffsets_GaussBlur5x5(16, 16, akSampleOffsets, 
            akSampleWeights);

        UpdateSampleConstants(akSampleOffsets, akSampleWeights);

        // Draw quad to create each next scaled tone map 
        DrawFullscreenEffect(m_akToneMaps[uiCurrTexture]);

        uiCurrTexture--;
    }

    // Downsample to 1x1 to create final luminance map
    GetSampleOffsets_DownScale4x4(4, 4, akSampleOffsets );
#else
    // Downsample to 1x1 to create final luminance map
    GetSampleOffsets_DownScale4x4(m_akToneMaps[1].m_spRT->GetWidth(0), 
        m_akToneMaps[1].m_spRT->GetHeight(0), akSampleOffsets );
#endif

    // Perform the final pass of the average luminance calculation. 
    // This pass scales the 4x4 log of average luminance texture from 
    // above and performs an exp() operation to return a single texel 
    // cooresponding to the average luminance of the scene in 
    // g_apTexToneMap[0]. 

    // Set offset shader constants
    UpdateSampleConstants(akSampleOffsets);

    // Draw quad to create final luminance tone map 
    DrawFullscreenEffect(m_akToneMaps[0]);
}
//-----------------------------------------------------------------------------
void HDRScene::CalculateAdaptation(float fElapsedTime)
{
    // Increment the user's adapted luminance

    // Swap current & last luminance
    AuxBuffer* pkSwap = m_pkAdaptedLumLast;
    m_pkAdaptedLumLast = m_pkAdaptedLumCur;
    m_pkAdaptedLumCur = pkSwap;
    
    // This simulates the light adaptation that occurs when moving from a 
    // dark area to a bright area, or vice versa. The g_pTexAdaptedLum
    // texture stores a single texel cooresponding to the user's adapted 
    // level.
    float fTime = fElapsedTime - m_fLastFrameTime;
    NiShaderFactory::UpdateGlobalShaderConstant("gfElapsedTime",
        sizeof(float), &fTime);
    m_fLastFrameTime = fElapsedTime;

    // Draw quad to create final luminance tone map 
    DrawFullscreenEffect(*m_pkAdaptedLumCur);
}
//-----------------------------------------------------------------------------
void HDRScene::UpdateImmediateLuminance()
{
    // Adopt the immediate luminance as the adapted
    // We attach a pass-through shader, and then replace the current base
    // texture (i.e. the other adapted lum texture) with the current luma
    // We render, swap buffers, and then replace the original texture and 
    // shader
    NiScreenElements* pkGeom = m_pkAdaptedLumCur->m_spGeom;

    pkGeom->ApplyAndSetActiveMaterial(m_spPassThroughMaterial);

    NiTexturingPropertyPtr spTex = (NiTexturingProperty*)
        pkGeom->GetProperty(NiTexturingProperty::GetType());
    assert(spTex);
    spTex->SetBaseTexture(m_akToneMaps[0].m_spTex);

    m_pkD3DRenderer->BeginUsingRenderTargetGroup(m_pkAdaptedLumCur->m_spRT,
        NiRenderer::CLEAR_ALL);
    m_pkD3DRenderer->SetScreenSpaceCameraData();
    pkGeom->RenderImmediate(m_pkD3DRenderer);
    m_pkD3DRenderer->EndUsingRenderTargetGroup();

    spTex->SetBaseTexture(m_pkAdaptedLumLast->m_spTex);
    pkGeom->ApplyAndSetActiveMaterial(m_spCalculateAdaptedLumMaterial);

    pkGeom = m_pkAdaptedLumLast->m_spGeom;
    pkGeom->ApplyAndSetActiveMaterial(m_spPassThroughMaterial);

    spTex = (NiTexturingProperty*)
        pkGeom->GetProperty(NiTexturingProperty::GetType());
    assert(spTex);
    spTex->SetBaseTexture(m_akToneMaps[0].m_spTex);

    m_pkD3DRenderer->BeginUsingRenderTargetGroup(m_pkAdaptedLumLast->m_spRT,
        NiRenderer::CLEAR_ALL);
    m_pkD3DRenderer->SetScreenSpaceCameraData();
    pkGeom->RenderImmediate(m_pkD3DRenderer);
    m_pkD3DRenderer->EndUsingRenderTargetGroup();

    spTex->SetBaseTexture(m_pkAdaptedLumCur->m_spTex);
    pkGeom->ApplyAndSetActiveMaterial(m_spCalculateAdaptedLumMaterial);
}
//---------------------------------------------------------------------------
void HDRScene::DrawBrightPassFromSceneScaled()
{
    // Draw quad to create bright pass map
    DrawFullscreenEffect(m_kBrightPass);
}
//---------------------------------------------------------------------------
void HDRScene::DrawStarSourceFromBrightPass()
{
    // Perform a 5x5 gaussian blur on BrightPass and place the result
    // in StarSource. The bright-pass filtered image is blurred before
    // being used for star operations to avoid aliasing artifacts.

    // Calculating down-sampling offset factors and make available to 
    // shaders
    NiPoint2 akSampleOffsets[MAX_SAMPLES];
    Point4 akSampleWeights[MAX_SAMPLES];
    GetSampleOffsets_GaussBlur5x5(m_kBrightPass.m_spTex->GetWidth(),
        m_kBrightPass.m_spTex->GetHeight(), akSampleOffsets, akSampleWeights);

    UpdateSampleConstants(akSampleOffsets, akSampleWeights);

    // The gaussian blur smooths out rough edges to avoid aliasing effects
    // when the star effect is run

    // Draw star source
    DrawFullscreenEffect(m_kStarSource);
}
//---------------------------------------------------------------------------
void HDRScene::DrawBloomSourceFromStarSource()
{
    // Scale down StarSource by 1/2 x 1/2 and place the result in 
    // BloomSource
    
    NiPoint2 akSampleOffsets[MAX_SAMPLES];
    GetSampleOffsets_DownScale2x2(m_kBrightPass.m_spTex->GetWidth(),
        m_kBrightPass.m_spTex->GetHeight(), akSampleOffsets);

    UpdateSampleConstants(akSampleOffsets);

    // Create an exact 1/2 x 1/2 copy of the source texture
    // Draw bloom source from star source
    DrawFullscreenEffect(m_kBloomSource);
}
//---------------------------------------------------------------------------
void HDRScene::RenderBloom()
{
    /////////////////////////
    // Another gaussian blur 
    //
   
    NiPoint2 akSampleOffsets[MAX_SAMPLES];
    float afSampleOffsets[MAX_SAMPLES];
    Point4 akSampleWeights[MAX_SAMPLES];
    GetSampleOffsets_GaussBlur5x5(m_kBloomSource.m_spTex->GetWidth(), 
        m_kBloomSource.m_spTex->GetHeight(), akSampleOffsets, 
        akSampleWeights);

    UpdateSampleConstants(akSampleOffsets, akSampleWeights);

    // Render the gaussian blur
    DrawFullscreenEffect(m_aspBlooms[2]);

    ////////////////////////////
    // Bloom Pass for Horizontal
    GetSampleOffsets_Bloom(m_aspBlooms[2].m_spRT->GetWidth(0), 
        afSampleOffsets, akSampleWeights, 3.0f, 2.0f );
    for (int i = 0; i < MAX_SAMPLES; i++ )
        akSampleOffsets[i] = NiPoint2(afSampleOffsets[i], 0.0f);

    UpdateSampleConstants(akSampleOffsets, akSampleWeights);
    
    // Render the horizontal bloom
    DrawFullscreenEffect(m_aspBlooms[1]);

    ////////////////////////////
    // Bloom Pass for Vertical

    GetSampleOffsets_Bloom(m_aspBlooms[1].m_spRT->GetHeight(0), 
        afSampleOffsets, akSampleWeights, 3.0f, 2.0f);
    for (int i = 0; i < MAX_SAMPLES; i++ )
        akSampleOffsets[i] = NiPoint2(0.0f, afSampleOffsets[i]);

    UpdateSampleConstants(akSampleOffsets, akSampleWeights);

    // Render the vertical bloom
    DrawFullscreenEffect(m_aspBlooms[0]);
}   
//---------------------------------------------------------------------------
void HDRScene::DrawFinalToDisplay(NiRenderTargetGroup* pkRenderTarget)
{
    // This "pass" draws a frame buffer size quad that is the composition
    // of data created in previous "passes".  It's target is not a 
    // rendered texture, but rather the backbuffer.
    
    DrawFullscreenEffect(m_kOutputBuffer);
}
//---------------------------------------------------------------------------
void HDRScene::RenderFrame(float fTime, PgRenderer* pkRenderer,NiRenderTargetGroup* pkRenderTarget, 
    NiRenderedTexture*)
{
    assert(pkRenderTarget);

	
    // First render scene to a shadow map from the light POV.
    assert(m_pkD3DRenderer);

    if (!m_kOutputBuffer.m_spRT)
    {
        m_kOutputBuffer.m_spRT = pkRenderTarget;

        CreateAuxBufferGeom(m_kHDRScene, false, 
            m_kOutputBuffer, false, m_spFinalScenePassMaterial, 
            m_aspBlooms[0].m_spTex, m_pkAdaptedLumCur->m_spTex);
    }

    m_kOutputBuffer.m_spRT = pkRenderTarget;
   
    m_pkD3DRenderer->SetBackgroundColor(NiColor::BLACK);

    m_pkD3DRenderer->LockRenderer();

    // This "pass" creates a scaled-down version of the render target 
    // from the previous pass.  The only object that is rendered is a 2D 
    // screen-size quad via an NiScreenSpaceCamera.  Output is a 1/4
    // frame-buffer sized 16-bit float per-channel rendered texture.
    DrawHDRSceneScaled();

    // Calculate average luminance -- output is a 1x1 16-bit float 
    // one-channel point texture
    MeasureLuminance();

    if (m_bFirstLumaCalc)
    {
        UpdateImmediateLuminance();
        m_bFirstLumaCalc = false;
    }
    else
    {
        if (m_bAdaptLum)
            CalculateAdaptation(fTime);
    }

    // Run a high-pass filter over SceneScaled -- output is RGBA8 texture
    DrawBrightPassFromSceneScaled();

    // Perform a 5x5 gaussian blur on BrightPass and place the result
    // in StarSource. The bright-pass filtered image is blurred before
    // being used for star operations to avoid aliasing artifacts.
    DrawStarSourceFromBrightPass();

    // Scale down StarSource by 1/2 x 1/2 and place the result in 
    // BloomSource
    DrawBloomSourceFromStarSource();

    // Render the bloom effect
    RenderBloom();

    // This "pass" draws a frame buffer size quad that is the composition
    // of data created in previous "passes".  It's target is not a 
    // rendered texture, but rather the backbuffer.
    DrawFinalToDisplay(pkRenderTarget);

    // Draw debug data and debug text
    if (m_bDebugPass)
    {
        m_pkD3DRenderer->SetScreenSpaceCameraData();

        unsigned int i;
        for (i = 0; i < m_kDebugImages.GetSize(); i++)
        {
            NiScreenElements* pkSE = m_kDebugImages.GetAt(i);
            if (pkSE)
                pkSE->Draw(m_pkD3DRenderer);
        }
    }

    m_pkD3DRenderer->UnlockRenderer();
}
//---------------------------------------------------------------------------
void HDRScene::GetSampleOffsets_DownScale4x4(unsigned int uiWidth, 
    unsigned int uiHeight, NiPoint2 akSampleOffsets[])
{
    assert(akSampleOffsets != 0);

    float fU = 1.0f / uiWidth;
    float fV = 1.0f / uiHeight;

    // Sample from the 16 surrounding points. 
    // Since the center point will be in  the exact center of 16 texels, 
    // a 0.5f offset is needed to specify a texel center.
    int index = 0;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            akSampleOffsets[index].x = (x - 1.5f) * fU;
            akSampleOffsets[index].y = (y - 1.5f) * fV;

            index++;
        }
    }
}
//---------------------------------------------------------------------------
void HDRScene::GetSampleOffsets_GaussBlur5x5(unsigned int uiTexWidth,
    unsigned int uiTexHeight, NiPoint2 akTexCoordOffsets[],
    HDRScene::Point4 akSampleWeights[], float fMultiplier)
{
    // Get the texture coordinate offsets to be used inside 
    // the GaussBlur5x5 pixel shader.
    float tu = 1.0f / (float)uiTexWidth ;
    float tv = 1.0f / (float)uiTexHeight ;

    HDRScene::Point4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );
    
    float totalWeight = 0.0f;
    int index = 0;
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            // Exclude pixels with a block distance greater than 2. This will
            // create a kernel which approximates a 5x5 kernel using only 13
            // sample points instead of 25; this is necessary since 2.0 
            // shaders only support 16 texture grabs.
            if(abs(x) + abs(y) > 2)
                continue;

            // Get the unscaled Gaussian intensity for this offset
            akTexCoordOffsets[index] = NiPoint2(x * tu, y * tv);
            float fGauss = GaussianDistribution((float)x, (float)y, 1.0f);
            akSampleWeights[index].x = vWhite.x * fGauss; 
            akSampleWeights[index].y = vWhite.y * fGauss; 
            akSampleWeights[index].z = vWhite.z * fGauss; 
            akSampleWeights[index].w = vWhite.w * fGauss; 

            totalWeight += akSampleWeights[index].x;

            index++;
        }
    }

    // Divide the current weight by the total weight of all the samples; 
    // Gaussian blur kernels add to 1.0f to ensure that the intensity of 
    // the image isn't changed when the blur occurs. An optional 
    // multiplier variable is used to add or remove image intensity 
    // during the blur.
    for (int i = 0; i < index; i++)
    {
        akSampleWeights[i].x /= totalWeight;
        akSampleWeights[i].y /= totalWeight;
        akSampleWeights[i].z /= totalWeight;
        akSampleWeights[i].w /= totalWeight;

        akSampleWeights[i].x *= fMultiplier;
        akSampleWeights[i].y *= fMultiplier;
        akSampleWeights[i].z *= fMultiplier;
        akSampleWeights[i].w *= fMultiplier;
    }
}
//---------------------------------------------------------------------------
float HDRScene::GaussianDistribution(float x, float y, float rho)
{
    // Helper function for GetSampleOffsets function to compute the 2 
    // parameter Gaussian distrubution using the given standard deviation rho
    
    float g = 1.0f / sqrtf(2.0f * NI_PI * rho * rho);
    g *= expf(-(x * x + y * y) / (2 * rho * rho));

    return g;
}
//---------------------------------------------------------------------------
void HDRScene::GetSampleOffsets_DownScale2x2(unsigned int uiWidth, 
    unsigned int uiHeight, NiPoint2 akSampleOffsets[])
{
    // Get the texture coordinate offsets to be used inside the DownScale2x2
    // pixel shader.

    float tU = 1.0f / uiWidth;
    float tV = 1.0f / uiHeight;

    // Sample from the 4 surrounding points. Since the center point 
    // will be in the exact center of 4 texels, a 0.5f offset is needed 
    // to specify a texel  center.
    int index = 0;
    for (int y = 0; y < 2; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            akSampleOffsets[ index ].x = (x - 0.5f) * tU;
            akSampleOffsets[ index ].y = (y - 0.5f) * tV;
            
            index++;
        }
    }
}
//---------------------------------------------------------------------------
void HDRScene::GetSampleOffsets_Bloom(unsigned int uiTexSize,
    float afTexCoordOffset[], HDRScene::Point4 akColorWeight[],
    float fDeviation, float fMultiplier)
{
    // Get the texture coordinate offsets to be used inside the Bloom
    // pixel shader.
    float tu = 1.0f / (float)uiTexSize;

    // Fill the center texel
    float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation);
    akColorWeight[0].x = weight;
    akColorWeight[0].y = weight;
    akColorWeight[0].z = weight;
    akColorWeight[0].w = 1.0f;

    afTexCoordOffset[0] = 0.0f;
    
    // Fill the first half
    for (int i = 1; i < 8; i++)
    {
        // Get the Gaussian intensity for this offset
        weight = fMultiplier * 
            GaussianDistribution((float)i, 0, fDeviation);
        afTexCoordOffset[i] = i * tu;

        akColorWeight[i].x = weight; 
        akColorWeight[i].y = weight; 
        akColorWeight[i].z = weight; 
        akColorWeight[i].w = 1.0f; 
    }

    // Mirror to the second half
    for (int i = 8; i < 15; i++ )
    {
        akColorWeight[i].x = akColorWeight[i - 7].x;
        akColorWeight[i].y = akColorWeight[i - 7].y;
        akColorWeight[i].z = akColorWeight[i - 7].z;
        akColorWeight[i].w = akColorWeight[i - 7].w;
        afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
    }
}
//---------------------------------------------------------------------------
void HDRScene::UpdateSampleConstants(NiPoint2* pkOffsets, Point4* pkWeights)
{
    assert(pkOffsets);
    NiShaderFactory::UpdateGlobalShaderConstant("gakSampleOffsets",
        sizeof(NiPoint2) * MAX_SAMPLES, pkOffsets);

    if (pkWeights)
    {
        NiShaderFactory::UpdateGlobalShaderConstant("gakSampleWeights",
            sizeof(Point4) * MAX_SAMPLES, pkWeights);
    }
}
//---------------------------------------------------------------------------
void HDRScene::DrawFullscreenEffect(AuxBuffer& kDestBuffer)
{
    // Clear before scissor
    m_pkD3DRenderer->BeginUsingRenderTargetGroup(kDestBuffer.m_spRT,
        NiRenderer::CLEAR_ALL);

    if (kDestBuffer.m_pkRectDest)
    {
#if defined(WIN32) || defined(_XENON)
        // Enable scissor
        LPDIRECT3DDEVICE9 pkDevice = m_pkD3DRenderer->GetD3DDevice();
        assert(pkDevice);
        pkDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);

        RECT kRect;
        kRect.left = kDestBuffer.m_pkRectDest->m_left;
        kRect.top = kDestBuffer.m_pkRectDest->m_top;
        kRect.right = kDestBuffer.m_pkRectDest->m_right;
        kRect.bottom = kDestBuffer.m_pkRectDest->m_bottom;
        pkDevice->SetScissorRect(&kRect);
#endif

        // Draw quad
        m_pkD3DRenderer->SetScreenSpaceCameraData();
        kDestBuffer.m_spGeom->RenderImmediate(m_pkD3DRenderer);

#if defined(WIN32) || defined(_XENON)
        // Disable scissor
        pkDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
#endif
    }
    else
    {
        // Draw quad 
        m_pkD3DRenderer->SetScreenSpaceCameraData();
        kDestBuffer.m_spGeom->RenderImmediate(m_pkD3DRenderer);
    }

    // If we are not rendering to the externally-provided render target,
    // then swap the buffers to ensure that the texture is updated
    if (kDestBuffer.m_spRT != m_kOutputBuffer.m_spRT)
        m_pkD3DRenderer->EndUsingRenderTargetGroup();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/*void HDRScene::HandleMenuEvent(NiMenu* pkMenu, NiMenuItem* pkItem)
{
    float fSetting;
    switch(pkItem->GetIdentifier())
    {
        case MENUID_TOGGLEHDR:
            ToggleHDR();
            break;
        case MENUID_DEBUGHDR:
            ToggleDebug();
            break;
        case MENUID_DIFFLIGHTMULTIPLIER:
        {
            fSetting = 
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "g_fDiffLightMultiplier", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_SPECLIGHTMULTIPLIER:
        {
            fSetting = 
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "g_fSpecLightMultiplier", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_POINTLIGHTMULTIPLIER:
        {
            fSetting = 
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "g_fPointLightMultiplier", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_BRIGHTPASSTHRESHOLD:
        {
            fSetting = 
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "gfBrightPassThreshold", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_BRIGHTPASSOFFSET:
        {
            fSetting =
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "gfBrightPassOffset", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_MIDDLEGRAY:
        {
            fSetting =
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "gfMiddleGray", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_BLOOMSCALE:
        {
            fSetting = 
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "gfBloomScale", sizeof(float), 
                &fSetting);
            break;
        }
        case MENUID_TOGGLEADAPTATION:
        {
            ToggleAdaptation();
            break;
        }
        case MENUID_ADAPTATIONSCALE:
        {
            fSetting =
                ((NiMenuItemFloatData*)pkItem)->GetFloatData();
            NiShaderFactory::UpdateGlobalShaderConstant(
                "gfAdaptationScale", sizeof(float), 
                &fSetting);
            break;
        }
    }
}*/
//---------------------------------------------------------------------------
void HDRScene::CreateAuxBufferGeom(const AuxBuffer& kSrcBuffer, 
    bool bSrcBorder, AuxBuffer& kDestBuffer, bool bDestBorder, 
    NiMaterial* pkMaterial, NiTexture* pkShaderMap0, NiTexture* pkShaderMap1)
{
    // Get the correct texture coordinates to apply to the rendered quad 
    // in order to sample from the source rectangle and render into 
    // the destination rectangle.  Start with a default mapping of the 
    // complete source surface to complete destination surface
    NiRect<float> kCoords(0.0f, 1.0f, 0.0f, 1.0f);

    if (bSrcBorder)
    {
        // These delta values are the distance between source texel 
        // centers in texture address space
        float tU = 1.0f / kSrcBuffer.m_spTex->GetWidth();
        float tV = 1.0f / kSrcBuffer.m_spTex->GetHeight();

        kCoords.m_left += tU;
        kCoords.m_top += tV;
        kCoords.m_right -= tU;
        kCoords.m_bottom -= tV;
    }

    unsigned int uiDestWidth = kDestBuffer.m_spRT->GetWidth(0);
    unsigned int uiDestHeight = kDestBuffer.m_spRT->GetHeight(0);

    kDestBuffer.m_pkRectDest = 0;
    if (bDestBorder)
    {
        // These delta values are the distance between dest texel 
        // centers in texture address space
        float tU = 1.0f / uiDestWidth;
        float tV = 1.0f / uiDestHeight;
        kCoords.m_left -= tU;
        kCoords.m_top -= tV;
        kCoords.m_right += tU;
        kCoords.m_bottom += tV;

        kDestBuffer.m_pkRectDest = NiNew NiRect<int>;
        kDestBuffer.m_pkRectDest->m_left = 1;
        kDestBuffer.m_pkRectDest->m_top = 1;
        kDestBuffer.m_pkRectDest->m_right = uiDestWidth - 1;
        kDestBuffer.m_pkRectDest->m_bottom = uiDestHeight - 1;
    }

    // Add screen space debug geometry using new NiScreenElements
    NiScreenElementsPtr spGeom = NiNew NiScreenElements(
        NiNew NiScreenElementsData(false, false, 1));
    assert(spGeom);
    int iQuadHandle = spGeom->Insert(4);

    spGeom->SetRectangle(iQuadHandle, 0, 0, 1.0f, 1.0f);
    spGeom->UpdateBound();
    spGeom->SetTextures(iQuadHandle, 0, kCoords.m_left,
        kCoords.m_top, kCoords.m_right, kCoords.m_bottom);
    
    NiTexturingPropertyPtr spPSVSQuadTex = NiNew NiTexturingProperty; 
    assert(spPSVSQuadTex);
    spPSVSQuadTex->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
    spGeom->AttachProperty(spPSVSQuadTex);

    spPSVSQuadTex->SetBaseTexture(kSrcBuffer.m_spTex);

    if (pkShaderMap0)
    {
        NiTexturingProperty::ShaderMap* pkPSVSShaderMap0 
            = NiNew NiTexturingProperty::ShaderMap(
            pkShaderMap0, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 0);

        assert(pkPSVSShaderMap0);
        spPSVSQuadTex->SetShaderMap(0, pkPSVSShaderMap0);
    }

    if (pkShaderMap1)
    {
        NiTexturingProperty::ShaderMap* pkPSVSShaderMap1 
            = NiNew NiTexturingProperty::ShaderMap(
            pkShaderMap1, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 1);

        assert(pkPSVSShaderMap1);
        spPSVSQuadTex->SetShaderMap(1, pkPSVSShaderMap1);
    }

    kDestBuffer.m_spGeom = spGeom;

    if (pkMaterial)
        spGeom->ApplyAndSetActiveMaterial(pkMaterial);

    spGeom->Update(0.0f);
    spGeom->UpdateEffects();
    spGeom->UpdateProperties();
}
//---------------------------------------------------------------------------
