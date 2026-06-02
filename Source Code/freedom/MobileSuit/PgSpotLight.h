#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGSPOTLIGHT_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGSPOTLIGHT_H
#include "NiMain.H"


class	PgSpotLight : public NiMemObject
{

public:
    static PgSpotLight* Create(NiRenderer* pRenderer, 
        unsigned int uiDetail = 6);

    ~PgSpotLight();
    
    void ClickAndStuff(float fTime);

	NiNode*	GetCaster()	{	return	m_spCaster;	}
    
    void SetLightDirection(NiPoint3 const &dir);
    NiPoint3 const &GetLightDirection();

    NiTextureEffect* GetTextureEffect();

	void	SetBGColor(const NiColor &kColor);

protected:
    PgSpotLight();
    void UpdateShadowCamera(float fTime);
    void UpdateProjection();

    NiCameraPtr m_spCamera;

    NiRendererPtr m_spRenderer;

    NiRenderedTexturePtr m_spRenderedTexture;
    NiRenderTargetGroupPtr m_spRenderTargetGroup;

    NiTextureEffectPtr m_spTextureEffect;
    NiNodePtr m_spCaster;
    NiPoint3 m_dir;
    unsigned int m_uiIndex;

    NiVisibleArray m_kVisible;
    NiCullingProcess m_kCuller;


	NiColorA	m_kBGColor;

};

//---------------------------------------------------------------------------
inline void PgSpotLight::SetLightDirection(NiPoint3 const &dir) 
{ 
    m_dir = dir; 
    m_dir.Unitize(); 
}
//---------------------------------------------------------------------------
inline NiPoint3 const &PgSpotLight::GetLightDirection() 
{ 
    return m_dir; 
}
//---------------------------------------------------------------------------
inline NiTextureEffect* PgSpotLight::GetTextureEffect() 
{ 
    return m_spTextureEffect; 
}
//---

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGSPOTLIGHT_H