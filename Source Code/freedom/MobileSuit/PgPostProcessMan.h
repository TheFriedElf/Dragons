#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMAN_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMAN_H
#include "PgIPostProcessEffect.h"
class PgRenderer;

// from Gamebryo Metal-war sample
class PgPostProcessMan
	: public NiMemObject
{
public:
	~PgPostProcessMan(void);

	bool AddEffect(PgIPostProcessEffect* pkEffect);
	
	bool AddEffect(PgIPostProcessEffect::EFFECT_TYPE type);

    unsigned int GetNumEffects() const;
    PgIPostProcessEffect* GetEffectAt(unsigned int uiIndex) const;
	PgIPostProcessEffect* GetEffect(PgIPostProcessEffect::EFFECT_TYPE type) const;

	bool GetEffectActivated(PgIPostProcessEffect::EFFECT_TYPE type) const;
    
    void RemoveEffectAt(unsigned int uiIndex);
    void RemoveAllEffects();

    void OnRenderFrameStart( float fTime, PgRenderer* pkRenderer, unsigned int uiClearMode = NiRenderer::CLEAR_ZBUFFER );
    void OnRenderFrameEnd( float fTime, PgRenderer* pkRenderer );

    NiRenderTargetGroup* GetFirstRenderTargetGroup();
    NiRenderedTexture* GetFirstRenderedTexture();

private:
    NiTPrimitiveSet<PgIPostProcessEffect*> m_kEffects;
    NiRenderTargetGroup* m_pkCurrRenderTargetGroup;
    NiRenderedTexture* m_pkCurrRenderTargetTexture;
};
#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGPOSTPRCESSMAN_H