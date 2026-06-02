#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMRT_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMRT_H
#include "PgIPostProcessEffect.h"

class PgRenderer;

class PgPostProcessMRT : public PgIPostProcessEffect
{
public:
    PgPostProcessMRT(bool bActive);

    virtual int GetType() const { return PgIPostProcessEffect::MRT; };
    virtual bool CreateEffect();
    virtual void CleanupEffect();
    virtual NiRenderTargetGroup* GetRenderTargetGroup();
    virtual NiRenderedTexture* GetTargetTexture();
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture);

    // NOTE -- code in the camera controller assumes that valid HUD effects
    // are contiguous in IDs.  For example, HEATSHIMMER is not considered
    // a "valid" HUD effect since the intent is to leave it on all the time.
    enum {
        HEATSHIMMER,
        DEPTHOFFIELDX,
        DEPTHOFFIELDY,
        GAUSSBLURX,
        GAUSSBLURY,
        NIGHTVISION,
        EDGEDETECT,
        INFRARED,
        MAGNIFY,
        MAX_MRTSHADERS
    };
    void SetMRTShaderActive(unsigned int uiID, bool bActive);
    bool GetMRTShaderActive(unsigned int uiID) const;
    char const* GetMRTShaderName(unsigned int uiID) const;

private:
    class MRTShader : public NiMemObject
    {
    public:
        unsigned int m_uiID;
        unsigned int m_uiMenuID;
        NiMaterialPtr m_spMaterial;
        char const* m_pcName;
        bool m_bActive;
    };

    void CreateRenderedTextures(unsigned int uiWidth, unsigned int uiHeight);
    void CleanupRenderedTextures();
    bool CreateScreenQuad(unsigned int uiWidth, unsigned int uiHeight);
    void CreateShaderTextures();
    void UpdateShaderConstants(unsigned int uiWidth, unsigned int uiHeight);
    void UpdateBlurDiagonalCoeffs(unsigned int uiWidth,
        unsigned int uiHeight);
    static float GetGaussianDistribution(float x, float y, float rho);

    void DrawPreProcess(PgRenderer* pkRenderer, 
        MRTShader* pkMRTShader);
    void DrawPostProcess(PgRenderer* pkRenderer,
        NiTexture* pkColorsTexture, NiMaterial* pkMaterial,
        NiRenderTargetGroup* pkRenderTarget, bool bSwap);

    void SetAppCulledRecursive(NiAVObject* pkObject, bool bAppCulled);

    // Screen space quad.
    NiScreenElementsPtr m_spScreenQuad;

    // Rendered textures.
    NiRenderedTexturePtr m_spMRTColorsTexture;
    // Used for Normal/Depth
    NiRenderedTexturePtr m_spMRTNormPosTexture;

    // Aux texture used for Shimmer Offsets (32-bit, not float)
    NiRenderedTexturePtr m_spAuxTexture;

    // Render targets.
    NiRenderTargetGroupPtr m_spMRTRenderTarget;
    NiRenderTargetGroupPtr m_spAuxTarget;
    NiRenderTargetGroupPtr m_spColorsTarget;

    // Required shader textures.
    NiTexturePtr m_spIRTexture;

    // optional rendering buffer - created only if needed
    NiRenderedTexturePtr m_spTempTexture;
    NiRenderTargetGroupPtr m_spTempTarget;

    // MRT Shaders.
    NiTPrimitiveSet<MRTShader*> m_kMRTShaders;

    // Pass-through material.
    NiMaterialPtr m_spPassThroughMaterial;

    bool m_bDebug;
};

#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMRT_H