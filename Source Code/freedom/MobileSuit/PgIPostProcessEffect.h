#ifndef FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGIPOSTPRCESSEFFECT_H
#define FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGIPOSTPRCESSEFFECT_H

class PgRenderer;

class PgIPostProcessEffect
	: public NiMemObject
{
public:
    PgIPostProcessEffect(bool bActive);
    virtual ~PgIPostProcessEffect();

	enum EFFECT_TYPE
    { 
        HDR,
		BRIGHT_BLOOM,
        MRT,
		FSAA,
		//FSAA2,
		DEPTH_OF_FIELD,
		RADIAL_BLUR,
        MAX_TYPES
    };
	virtual int GetType() const = 0;

    virtual bool CreateEffect() = 0;
    virtual void CleanupEffect() = 0;
    virtual NiRenderTargetGroup* GetRenderTargetGroup() = 0;
    virtual NiRenderedTexture* GetTargetTexture() = 0;
    virtual void RenderFrame(float fTime, PgRenderer* pkRenderer, NiRenderTargetGroup* pkRenderTarget, NiRenderedTexture* pkTexture) = 0;
	
	virtual void	SetParam(char const *ParamName,float fValue);
	virtual float GetParam(char const *ParamName);

    bool GetActive() const;
    void SetActive(bool bActive);

private:
    bool m_bActive;
};
#endif // FREEDOM_DRAGONICA_RENDER_POSTPROCESS_PGIPOSTPRCESSEFFECT_H