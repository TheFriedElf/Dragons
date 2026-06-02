#include "StdAfx.h"
#include "PgPostProcessMRT.h"
#include "PgRenderer.h"
#include "PgNifMan.h"


// The size of these two arrays must match up with MRTEffects.fx. Asserts
// will check this later in the code during UpdateShaderConstants.
#define ED_KERNEL_SIZE 8
static float g_afEdgeDetectPixelKernel[ED_KERNEL_SIZE] =
{
    0.0f,   1.0f,
    1.0f,   0.0f,
    0.0f,   -1.0f,
    -1.0f,  0.0f
};

#define BLUR_KERNEL_SIZE 13
static float g_afBlurPixelKernel[BLUR_KERNEL_SIZE] =
{
    -6.0f,
    -5.0f,
    -4.0f,
    -3.0f,
    -2.0f,
    -1.0f,
    0.0f,
    1.0f,
    2.0f,
    3.0f,
    4.0f,
    5.0f,
    6.0f,
};

//---------------------------------------------------------------------------
PgPostProcessMRT::PgPostProcessMRT(bool bActive) : PgIPostProcessEffect(bActive)
{
}
//---------------------------------------------------------------------------
bool PgPostProcessMRT::CreateEffect()
{
    // Be sure we can perform this effect
#if defined(_XENON)
    NiXenonRenderer* pkD3DRenderer = NiDynamicCast(NiXenonRenderer, 
        NiRenderer::GetRenderer());
#elif defined(WIN32)
    NiDX9Renderer* pkD3DRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());
#endif
    PG_ASSERT_LOG(pkD3DRenderer);

// We know that Xenon will pass this test, and CheckDeviceFormat actually
// returns false.
#if !defined(_XENON)
    HRESULT hr = pkD3DRenderer->GetDirect3D()->CheckDeviceFormat(
        pkD3DRenderer->GetAdapter(), pkD3DRenderer->GetDevType(),
        pkD3DRenderer->GetPresentParams()->BackBufferFormat,
        D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F);
    if (FAILED(hr))
    {
        // FAILED
        NiMessageBox("This hardware does not have the ability to create "
            "a floating point render target, which this sample requires.",
            "Error - Incompatible Hardware");
        return false;
    }
#endif 

    unsigned int uiWidth = pkD3DRenderer->GetDefaultRenderTargetGroup()->
        GetWidth(0);
    unsigned int uiHeight = pkD3DRenderer->GetDefaultRenderTargetGroup()->
        GetHeight(0);

    CreateRenderedTextures(uiWidth, uiHeight);

    // Load MRT Shaders.
    MRTShader* pkMRTShader;

    // Heat Shimmer.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = HEATSHIMMER;
	pkMRTShader->m_pcName = "MRT_HeatShimmer";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Depth of field with blur in X.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = DEPTHOFFIELDX;
	pkMRTShader->m_pcName = "MRT_DepthOfFieldX";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(
        pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Depth of field with blur in Y.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = DEPTHOFFIELDY;
	pkMRTShader->m_pcName = "MRT_DepthOfFieldY";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Gaussian blur in X.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = GAUSSBLURX;
	pkMRTShader->m_pcName = "MRT_GaussBlurX";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
	pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Gaussian blur in Y.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = GAUSSBLURY;
	pkMRTShader->m_pcName = "MRT_GaussBlurY";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Normal Edge Detection.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = EDGEDETECT;
	pkMRTShader->m_pcName = "MRT_EdgeDetect";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Night Vision.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = NIGHTVISION;
	pkMRTShader->m_pcName = "MRT_NightVision";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Infrared.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = INFRARED;
	pkMRTShader->m_pcName = "MRT_Infrared";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Magnify.
    pkMRTShader = NiNew MRTShader;
    pkMRTShader->m_uiID = MAGNIFY;
	pkMRTShader->m_pcName = "MRT_Magnify";
    pkMRTShader->m_spMaterial = NiSingleShaderMaterial::Create(pkMRTShader->m_pcName);
    PG_ASSERT_LOG(pkMRTShader->m_spMaterial);
    pkMRTShader->m_bActive = false;
    m_kMRTShaders.Add(pkMRTShader);

    // Load pass-through shader.
    m_spPassThroughMaterial = NiSingleShaderMaterial::Create("MRT_PassThrough");
    PG_ASSERT_LOG(m_spPassThroughMaterial);

    // Create screen quad and camera.
	if (!CreateScreenQuad(uiWidth, uiHeight))
    {
        return false;
    }

    CreateShaderTextures();

    m_bDebug = false;

    UpdateShaderConstants(uiWidth, uiHeight);

    return true;
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::CleanupEffect()
{
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        SAFE_DELETE_NI(m_kMRTShaders.GetAt(ui));
    }
    m_kMRTShaders.RemoveAll();

    CleanupRenderedTextures();
}
//---------------------------------------------------------------------------
NiRenderTargetGroup* PgPostProcessMRT::GetRenderTargetGroup()
{
    return m_spMRTRenderTarget;
}
//---------------------------------------------------------------------------
NiRenderedTexture* PgPostProcessMRT::GetTargetTexture()
{
    return m_spMRTColorsTexture;
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::RenderFrame(float fTime, PgRenderer* pkRenderer,
    NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture)
{
    PG_ASSERT_LOG(pkRenderTarget);

    NiRenderedTexture* pkTempTex = 0;
    NiRenderTargetGroup* pkTempTarget = 0;
    if (pkTexture)
    {
        // use the destination as a temp buffer
        pkTempTex = pkTexture;
        pkTempTarget = pkRenderTarget;
    }
    else
    {
        // cannot use the destination as a temp buffer - use local temp
        if (!m_spTempTexture)
        {   
			/*
            WorldManager::Log("PgPostProcessMRT::RenderFrame: had to create "
                "additional 64-bit temporary buffer\n");
				*/
            NiTexture::FormatPrefs kPrefs;

            // Use 32 bpp for color on Xenon as 1010102
#if defined(_XENON)
            kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_32;
#else
            kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
#endif

            unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
            unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

            m_spTempTexture = NiRenderedTexture::Create(uiWidth, 
				uiHeight, pkRenderer->GetRenderer(), kPrefs);
            PG_ASSERT_LOG(m_spTempTexture);

            m_spTempTarget = NiRenderTargetGroup::Create(
                m_spTempTexture->GetBuffer(), pkRenderer->GetRenderer(), false);
            PG_ASSERT_LOG(m_spTempTarget);
        }

        pkTempTex = m_spTempTexture;
        pkTempTarget = m_spTempTarget;
    }

    PG_ASSERT_LOG(pkTempTex);

    // Update screen quad.
    // m_spScreenQuad->Update(fTime);

    pkRenderer->SetBackgroundColor(NiColor::BLACK);

    // count the number of active passes and grab two specific shaders if
    // they are active
    MRTShader* pkActiveHeatShader = 0;
    MRTShader* pkActiveEdgeShader = 0;
    unsigned int uiEffectCount = 0;
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        MRTShader* pkMRTShader = m_kMRTShaders.GetAt(ui);
        if (pkMRTShader->m_bActive)
        {
            uiEffectCount++;

			/*
            if (pkMRTShader->m_uiMenuID == MENUID_HEATSHIMMER)
                pkActiveHeatShader = pkMRTShader;
            else if (pkMRTShader->m_uiMenuID == MENUID_EDGEDETECT)
                pkActiveEdgeShader = pkMRTShader;
			*/
        }
    }

    // if both heat and edge shaders are active, temporarily deactivate heat,
    // as both effect use the same temp buffer.  We will reactivate heat at
    // the end of this function
    if (pkActiveHeatShader && pkActiveEdgeShader)
    {
        pkActiveHeatShader->m_bActive = false;
        uiEffectCount--;
    }

    // There are two basic cases - either we are using the source color
    // buffer as one temp buffer and the target buffer as the other temp,
    // or else we are using the source color buffer as one temp buffer and
    // a local temp buffer as the other temp buffer.  The former saves VRAM,
    // but the latter avoids having to do an extra pass in the case of an
    // even number of effects.
    // For the examples below, S is the source color buffer, D is the dest
    // buffer, T is a temp buffer, -> is a rendering pass
    //
    // Zero effects enabled: S->D via pass-through shader
    // One effect enabled: S->D via an effect shader
    // Two effects enabled:
    //     Case 1 (no temp buffer): S->D->S->D (last pass is pass-through)
    //     Case 2 (temp buffer): S->T->D
    //
    // Three effects enabled: 
    //     Case 1 (no temp buffer): S->D->S->D
    //     Case 2 (temp buffer): S->T->S->D
    // Etc.
    bool bPassThroughPassNeeded = (uiEffectCount == 0) 
        || (pkTexture && !(uiEffectCount & 0x01));

    // Source to first pass is _always_ the existing color buffer
    NiTexture* pkReadTexture = m_spMRTColorsTexture;

    // If there are 1 or 0 active effects, then the destination buffer should
    // be the render target.  Otherwise, render to a temp buffer
    NiRenderTargetGroup* pkOutputTarget = 0;
    if (uiEffectCount > 1)
        pkOutputTarget = pkTempTarget;
    else
        pkOutputTarget = pkRenderTarget;

    unsigned int uiCount = 0;
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        MRTShader* pkMRTShader = m_kMRTShaders.GetAt(ui);
        if (pkMRTShader->m_bActive)
        {
            DrawPreProcess(pkRenderer, pkMRTShader);
            DrawPostProcess(pkRenderer, pkReadTexture,
                pkMRTShader->m_spMaterial, pkOutputTarget, 
                (uiCount < (uiEffectCount - 1)) || bPassThroughPassNeeded);

            // increment to next pass
            uiCount++;

            if (uiCount % 2)
            {
                pkReadTexture = pkTempTex;
                pkOutputTarget = m_spColorsTarget;
            }
            else
            {
                pkReadTexture = m_spMRTColorsTexture;
                pkOutputTarget = pkTempTarget;
            }

            // If we're about to render the last pass, then we have several
            // cases to handle.  Where possible, we want to render the final
            // effect pass into the destination buffer.  Normally, that's
            // possible.  However, if we need to read from the pkTempTexture
            // (i.e. it is an "odd" pass according to uiCount) in the final 
            // pass AND pkTempTarget is the render target (i.e. no temp 
            // buffer), then we need to render into m_spColorsTarget and do a
            // pass-through pass to copy into the render target buffer.  We 
            // need to do this extra pass because we can't use a related 
            // target/texture pair as source and dest for the same operation.
            if ((uiCount == (uiEffectCount - 1)) && !bPassThroughPassNeeded)
                pkOutputTarget = pkRenderTarget;
        }
    }

    if (m_bDebug)
    {
        DrawPostProcess(pkRenderer, m_spAuxTexture, 
            m_spPassThroughMaterial, pkRenderTarget, false);
    }
    else if (bPassThroughPassNeeded) // if 0 or 1 effects on, copy src to dest
    {
        DrawPostProcess(pkRenderer, pkReadTexture, m_spPassThroughMaterial,
            pkRenderTarget, false);
    }

    // if both heat and edge shaders are active, then we temporarily 
    // deactivated heat, as both effect use the same temp buffer.  We 
    // reactivate now
    if (pkActiveHeatShader)
    {
        pkActiveHeatShader->m_bActive = true;
        uiEffectCount++;
    }
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::CreateRenderedTextures(unsigned int uiWidth,
    unsigned int uiHeight)
{
#if defined(_XENON)
    NiXenonRenderer* pkRenderer = NiDynamicCast(NiXenonRenderer, 
        NiRenderer::GetRenderer());
#elif defined(WIN32)
    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
        NiRenderer::GetRenderer());
#endif
    PG_ASSERT_LOG(pkRenderer);

    PG_ASSERT_LOG((uiWidth % 8) == 0);
    PG_ASSERT_LOG((uiHeight % 8) == 0);

    NiTexture::FormatPrefs kPrefs;

    // Use 32bpp for color on Xenon as 1010102 format.
#if defined(_XENON)
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_32;
#else
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
#endif

    // Create the multiple render target (MRT) textures.
    m_spMRTColorsTexture = NiRenderedTexture::Create(uiWidth,
        uiHeight, pkRenderer, kPrefs);
    PG_ASSERT_LOG(m_spMRTColorsTexture);

    // Use 64bpp for normals and depth.
#if defined(_XENON)
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::FLOAT_COLOR_64;
#endif
    m_spMRTNormPosTexture = NiRenderedTexture::Create(uiWidth, 
        uiHeight, pkRenderer, kPrefs);
    PG_ASSERT_LOG(m_spMRTNormPosTexture);

    // Create the shimmer buffer
    NiTexture::FormatPrefs kShimmerPrefs;
    kShimmerPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
    m_spAuxTexture = NiRenderedTexture::Create(uiWidth, 
        uiHeight, pkRenderer, kShimmerPrefs);
    PG_ASSERT_LOG(m_spAuxTexture);

    // Get the main depth stencil buffer.
#if !defined(_XENON)
    NiDepthStencilBuffer* pkDSB = pkRenderer->GetDefaultRenderTargetGroup()->
        GetDepthStencilBuffer();
    PG_ASSERT_LOG(pkDSB);
#else
    // We need to be able to resolve the initial depth buffer.
    NiDepthStencilBuffer* pkDSB = 0;
    pkDSB = NiResolvableDepthStencilBuffer::CreateCompatible(
        m_spMRTColorsTexture->GetBuffer(), pkRenderer);
    PG_ASSERT_LOG(pkDSB);
#endif

    // Create the MRT render target.
    m_spMRTRenderTarget = NiRenderTargetGroup::Create(2, pkRenderer);
    m_spMRTRenderTarget->AttachBuffer(
        m_spMRTColorsTexture->GetBuffer(), 0);
    m_spMRTRenderTarget->AttachBuffer(
        m_spMRTNormPosTexture->GetBuffer(), 1);
    m_spMRTRenderTarget->AttachDepthStencilBuffer(pkDSB);

    // Test to be sure that the shared depth buffer is compatible
    bool bValidRTG = pkRenderer->
        ValidateRenderTargetGroup(m_spMRTRenderTarget);
    if (!bValidRTG)
    {
        pkDSB = NiDepthStencilBuffer::CreateCompatible(
            m_spMRTColorsTexture->GetBuffer(), pkRenderer);
        PG_ASSERT_LOG(pkDSB);
        m_spMRTRenderTarget->AttachDepthStencilBuffer(pkDSB);
    }

    // Create the other render targets.
#if defined(_XENON)
    // We have a resolvable Depth buffer for the color render. We need
    // the regular one here.
    pkDSB = pkRenderer->GetDefaultRenderTargetGroup()->
        GetDepthStencilBuffer();
#endif
    m_spAuxTarget = NiRenderTargetGroup::Create(
        m_spAuxTexture->GetBuffer(), pkRenderer, pkDSB);
    PG_ASSERT_LOG(m_spAuxTarget);
    m_spColorsTarget = NiRenderTargetGroup::Create(
        m_spMRTColorsTexture->GetBuffer(), pkRenderer, pkDSB);
    PG_ASSERT_LOG(m_spColorsTarget);
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::CleanupRenderedTextures()
{
    // Release D3D textures.
    NiRenderer* pkRenderer = NiRenderer::GetRenderer();
    pkRenderer->PurgeTexture(m_spMRTColorsTexture);
    m_spMRTColorsTexture = 0;
    pkRenderer->PurgeTexture(m_spMRTNormPosTexture);
    m_spMRTNormPosTexture = 0;
    pkRenderer->PurgeTexture(m_spAuxTexture);
    m_spAuxTexture = 0;

    // This texture is optional
    if (m_spTempTexture)
        pkRenderer->PurgeTexture(m_spTempTexture);
    m_spTempTexture = 0;

    // Release render targets
    m_spMRTRenderTarget = 0;
    m_spAuxTarget = 0;
    m_spTempTarget = 0;
    m_spColorsTarget = 0;
}
//---------------------------------------------------------------------------
bool PgPostProcessMRT::CreateScreenQuad(unsigned int uiWidth,
    unsigned int uiHeight)
{
    // Load the texturing property for the noise base map.
    NiStream kStream;
    if (!kStream.Load("../Data/noise.nif") || kStream.GetObjectCount() == 0)
    {
        return false;
    }
    PG_ASSERT_LOG(NiIsKindOf(NiNode, kStream.GetObjectAt(0)));
    NiNode* pkNode = (NiNode*) kStream.GetObjectAt(0);
    NiAVObject* pkPlane = pkNode->GetObjectByName("Plane01");
    PG_ASSERT_LOG(pkPlane);
    NiTexturingProperty* pkTexProp = (NiTexturingProperty*)
        pkPlane->GetProperty(NiProperty::TEXTURING);
    PG_ASSERT_LOG(pkTexProp);

    // Create screen quad.
    m_spScreenQuad = NiNew NiScreenElements(NiNew NiScreenElementsData(
        false, false, 1));
    int iPolygon = m_spScreenQuad->Insert(4);
    m_spScreenQuad->SetRectangle(iPolygon, 0, 0, 1.0f, 1.0f);
    m_spScreenQuad->UpdateBound();
    m_spScreenQuad->SetTextures(iPolygon, 0, 0.0f, 0.0f, 1.0f, 1.0f);

    // Attach texturing property to screen quad.
    m_spScreenQuad->AttachProperty(pkTexProp);
    m_spScreenQuad->UpdateEffects();
    m_spScreenQuad->UpdateProperties();

    return true;
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::CreateShaderTextures()
{
    NiD3DRenderer* pkRenderer = NiDynamicCast(NiD3DRenderer,
        NiRenderer::GetRenderer());
    PG_ASSERT_LOG(pkRenderer);

    // Load IR texture from disk.
    m_spIRTexture = g_kNifMan.GetTexture("../Data/Sahders/Textures/IRRamp.bmp");
    PG_ASSERT_LOG(m_spIRTexture);

}
//---------------------------------------------------------------------------
void PgPostProcessMRT::UpdateShaderConstants(unsigned int uiWidth,
    unsigned int uiHeight)
{
    unsigned int ui;

    // Blur sizes were tuned at a specific screen resolution,
    // however, we want the same relative size effect on all actual resolutions
	unsigned int uiTunedWidth = XUIMgr.GetResolutionSize().x;
	unsigned int uiTunedHeight = XUIMgr.GetResolutionSize().y;


#ifdef _DEBUG
    // Retrieve edge detect texel kernel to check size values.
    float* pfTemp = 0;
    unsigned int uiByteCount = 0;
    unsigned int uiFloatCount = 0;

    NiD3DShaderFactory::RetrieveGlobalShaderConstant(
        "g_vEdgeDetectTexelKernel", uiByteCount,
        (void const*&) pfTemp);
    PG_ASSERT_LOG(pfTemp && uiByteCount > 0);
    PG_ASSERT_LOG(uiByteCount == sizeof(g_afEdgeDetectPixelKernel));
    pfTemp = 0;
#endif

    // Compute edge detect texel kernel.
    float* pfEdgeDetectTexelKernel = NiAlloc(float, ED_KERNEL_SIZE);
    for (ui = 0; ui < ED_KERNEL_SIZE; ui += 2)
    {
        pfEdgeDetectTexelKernel[ui] = g_afEdgeDetectPixelKernel[ui] / uiWidth;
        pfEdgeDetectTexelKernel[ui + 1] = g_afEdgeDetectPixelKernel[ui + 1] /
            uiHeight;
    }
    NiD3DShaderFactory::UpdateGlobalShaderConstant(
        "g_vEdgeDetectTexelKernel", ED_KERNEL_SIZE * sizeof(float),
        pfEdgeDetectTexelKernel);
    NiFree(pfEdgeDetectTexelKernel);

#ifdef _DEBUG
    // Retrieve blur pixel kernel.
    pfTemp = 0;
    NiD3DShaderFactory::RetrieveGlobalShaderConstant(
        "g_vBlurTexelKernelX", uiByteCount, (void const*&) pfTemp);
    PG_ASSERT_LOG(pfTemp && uiByteCount > 0);
    PG_ASSERT_LOG(uiByteCount == sizeof(g_afBlurPixelKernel) * 2);
#endif

    // Compute blur texel kernels.
    float* pfBlurTexelKernelX = NiAlloc(float, BLUR_KERNEL_SIZE * 2);
    float* pfBlurTexelKernelY = NiAlloc(float, BLUR_KERNEL_SIZE * 2);
    for (ui = 0; ui < BLUR_KERNEL_SIZE; ui++)
    {
        pfBlurTexelKernelX[ui * 2] = g_afBlurPixelKernel[ui] / uiTunedWidth;
        pfBlurTexelKernelX[ui * 2 + 1] = 0;
        
        pfBlurTexelKernelY[ui * 2] = 0;
        pfBlurTexelKernelY[ui * 2 + 1] = 
            g_afBlurPixelKernel[ui] / uiTunedHeight;
    }
    NiD3DShaderFactory::UpdateGlobalShaderConstant(
        "g_vBlurTexelKernelX", BLUR_KERNEL_SIZE * sizeof(float) * 2, 
        pfBlurTexelKernelX);
    NiD3DShaderFactory::UpdateGlobalShaderConstant(
        "g_vBlurTexelKernelY", BLUR_KERNEL_SIZE * sizeof(float) * 2, 
        pfBlurTexelKernelY);
    NiFree(pfBlurTexelKernelX);
    NiFree(pfBlurTexelKernelY);

    float fShimmerBrightness = 5.0f;
    NiD3DShaderFactory::UpdateGlobalShaderConstant(
        "g_fShimmerBrightness", sizeof(float), 
        &fShimmerBrightness);

    UpdateBlurDiagonalCoeffs(uiTunedWidth, uiTunedHeight);
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::UpdateBlurDiagonalCoeffs(unsigned int uiWidth,
    unsigned int uiHeight)
{
    NiPoint2 akOffsets[13];
    float aakWeights[13][4];
    // Get the texture coordinate offsets to be used inside 
    // the BlurDiagonal pixel shader function.
    float tu = 1.0f / (float) uiWidth;
    float tv = 1.0f / (float) uiHeight;

    int const iKernelSize = 5;

    float totalWeight = 0.0f;
    int index = 0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            // Exclude pixels that do not lie in line diagonally from the
            // center pixel. This will produce more of a diagonal star effect
            // than a smooth Gaussian distribution.
            if (abs(x) != abs(y))
            {
                continue;
            }

            // Get the unscaled Gaussian intensity for this offset
            akOffsets[index] = NiPoint2(x * tu, y * tv);
            float fGauss = GetGaussianDistribution((float)x, (float)y, 1.0f);
            aakWeights[index][0] = fGauss; 
            aakWeights[index][1] = fGauss; 
            aakWeights[index][2] = fGauss; 
            aakWeights[index][3] = fGauss; 

            totalWeight += aakWeights[index][0];

            index++;
        }
    }

    PG_ASSERT_LOG(index == iKernelSize);

    // Divide the current weight by the total weight of all the samples; 
    // Gaussian blur kernels add to 1.0f to ensure that the intensity of 
    // the image isn't changed when the blur occurs. An optional 
    // multiplier variable is used to add or remove image intensity 
    // during the blur.
    float fScaling = 1.0f / totalWeight;
    for (int i = 0; i < index; i++)
    {
        aakWeights[i][0] *= fScaling;
        aakWeights[i][1] *= fScaling;
        aakWeights[i][2] *= fScaling;
        aakWeights[i][3] *= fScaling;
    }

    NiD3DShaderFactory::UpdateGlobalShaderConstant("g_vBlurDiagonalOffsets",
        sizeof(NiPoint2) * iKernelSize, akOffsets);

    NiD3DShaderFactory::UpdateGlobalShaderConstant("g_vBlurDiagonalWeights",
        sizeof(float) * 4 * iKernelSize, aakWeights);
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::DrawPreProcess(PgRenderer* pkRenderer,
    MRTShader* pkMRTShader)
{
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::DrawPostProcess(PgRenderer* pkRenderer,
    NiTexture* pkColorsTexture, NiMaterial* pkMaterial,
    NiRenderTargetGroup* pkRenderTarget, bool bSwap)
{
    PG_ASSERT_LOG(pkRenderer && pkColorsTexture && pkMaterial && pkRenderTarget);

    NiTexturingProperty* pkTP = NiDynamicCast(NiTexturingProperty, 
        m_spScreenQuad->GetProperty(NiProperty::TEXTURING));
    PG_ASSERT_LOG(pkTP);

    NiTexturingProperty::ShaderMap* pkShaderMap = pkTP->GetShaderMap(0);
    if (pkShaderMap)
    {
        pkShaderMap->SetTexture(pkColorsTexture);
    }
    else
    {
        pkShaderMap = NiNew NiTexturingProperty::ShaderMap(
            pkColorsTexture, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 0);
        PG_ASSERT_LOG(pkShaderMap);
        pkTP->SetShaderMap(0, pkShaderMap);
    }

    pkShaderMap = pkTP->GetShaderMap(1);
    if (pkShaderMap)
    {
        pkShaderMap->SetTexture(m_spMRTNormPosTexture);
    }
    else
    {
        pkShaderMap = NiNew NiTexturingProperty::ShaderMap(
            m_spMRTNormPosTexture, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 0);
        PG_ASSERT_LOG(pkShaderMap);
        pkTP->SetShaderMap(1, pkShaderMap);
    }

    pkShaderMap = pkTP->GetShaderMap(2);
    if (pkShaderMap)
    {
        pkShaderMap->SetTexture(m_spAuxTexture);
    }
    else
    {
        pkShaderMap = NiNew NiTexturingProperty::ShaderMap(
            m_spAuxTexture, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 0);
        PG_ASSERT_LOG(pkShaderMap);
        pkTP->SetShaderMap(2, pkShaderMap);
    }

    pkShaderMap = pkTP->GetShaderMap(3);
    if (pkShaderMap)
    {
        pkShaderMap->SetTexture(m_spIRTexture);
    }
    else
    {
        pkShaderMap = NiNew NiTexturingProperty::ShaderMap(
            m_spIRTexture, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
            NiTexturingProperty::FILTER_NEAREST, 0);
        PG_ASSERT_LOG(pkShaderMap);
        pkTP->SetShaderMap(3, pkShaderMap);
    }

    const NiMaterial* pkCurrentActiveMaterial = 
        m_spScreenQuad->GetActiveMaterial();
    if (pkMaterial)
    {
        m_spScreenQuad->ApplyAndSetActiveMaterial(pkMaterial);
    }

    pkRenderer->BeginUsingRenderTargetGroup(pkRenderTarget,
        NiRenderer::CLEAR_BACKBUFFER | NiRenderer::CLEAR_ZBUFFER |
        NiRenderer::CLEAR_STENCIL);

    // Draw full screen quad.
    pkRenderer->SetScreenSpaceCameraData();
	m_spScreenQuad->Draw(pkRenderer->GetRenderer());

    if (bSwap)
        pkRenderer->EndUsingRenderTargetGroup();

    m_spScreenQuad->SetActiveMaterial(pkCurrentActiveMaterial);
}
//---------------------------------------------------------------------------
void PgPostProcessMRT::SetAppCulledRecursive(NiAVObject* pkObject, bool bAppCulled)
{
    pkObject->SetAppCulled(bAppCulled);

    NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
    if (pkNode)
    {
        for (unsigned int ui = 0; ui < pkNode->GetArrayCount(); ui++)
        {
            NiAVObject* pkChild = pkNode->GetAt(ui);
            if (pkChild)
            {
                SetAppCulledRecursive(pkChild, bAppCulled);
            }
        }
    }
}

inline bool PgPostProcessMRT::GetMRTShaderActive(unsigned int uiID) const
{
    PG_ASSERT_LOG(uiID < MAX_MRTSHADERS);

    MRTShader* pkShader = 0;
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        pkShader = m_kMRTShaders.GetAt(ui);

        if (pkShader->m_uiID == uiID)
            break;
    }

    PG_ASSERT_LOG(pkShader);
    return pkShader->m_bActive;
}

inline void PgPostProcessMRT::SetMRTShaderActive(unsigned int uiID,
    bool bActive)
{
    PG_ASSERT_LOG(uiID < MAX_MRTSHADERS);

    MRTShader* pkShader = 0;
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        pkShader = m_kMRTShaders.GetAt(ui);

        if (pkShader->m_uiID == uiID)
            break;
    }

    PG_ASSERT_LOG(pkShader);
    pkShader->m_bActive = bActive;
}

inline char const* PgPostProcessMRT::GetMRTShaderName(unsigned int uiID) const
{
    PG_ASSERT_LOG(uiID < MAX_MRTSHADERS);

    MRTShader* pkShader = 0;
    for (unsigned int ui = 0; ui < m_kMRTShaders.GetSize(); ui++)
    {
        pkShader = m_kMRTShaders.GetAt(ui);

        if (pkShader->m_uiID == uiID)
            break;
    }

    PG_ASSERT_LOG(pkShader);
    return pkShader->m_pcName;
}

inline float PgPostProcessMRT::GetGaussianDistribution(float x, float y, float rho)
{
    float g = 1.0f / sqrtf(2.0f * NI_PI * rho * rho);
    g *= expf(-(x * x + y * y) / (2 * rho * rho));

    return g;
}