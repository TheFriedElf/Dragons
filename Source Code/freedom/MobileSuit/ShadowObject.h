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

#ifndef SHADOWOBJECT_H
#define SHADOWOBJECT_H

#include <NiColor.h>
#include <NiNode.h>

NiSmartPointer(NiCamera);
NiSmartPointer(MasterPropertyAccumulator);
NiSmartPointer(NiRenderedTexture);
NiSmartPointer(NiRenderTargetGroup);
NiSmartPointer(NiRenderer);
NiSmartPointer(NiTextureEffect);

class ShadowObject : public NiMemObject
{

public:
    static ShadowObject* Create(NiRenderer* pRenderer, 
        unsigned int uiDetail = 6);

    ~ShadowObject();
    
    void ClickAndStuff(float fTime);

    void SetCaster(NiNode* pCaster);
    
    void SetLightDirection(NiPoint3 const &dir);
    NiPoint3 const &GetLightDirection();

    NiTextureEffect* GetShadowEffect();

protected:
    ShadowObject();
    void UpdateShadowCamera(float fTime);
    void UpdateProjection();

    NiCameraPtr m_spCamera;

    NiRendererPtr m_spRenderer;

    NiRenderedTexturePtr m_spRenderedTexture;
    NiRenderTargetGroupPtr m_spRenderTargetGroup;

	NiScreenElementsPtr	m_spSE_DownAlpha;

    NiTextureEffectPtr m_spShadow;
    NiNodePtr m_spCaster;
    NiPoint3 m_dir;
    unsigned int m_uiIndex;

    MasterPropertyAccumulatorPtr m_spSort;

    NiVisibleArray m_kVisible;
    NiCullingProcess m_kCuller;
};
//---------------------------------------------------------------------------
inline void ShadowObject::SetCaster(NiNode* pCaster)
{
    m_spCaster = pCaster;
}
//---------------------------------------------------------------------------
inline void ShadowObject::SetLightDirection(NiPoint3 const &dir) 
{ 
    m_dir = dir; 
    m_dir.Unitize(); 
}
//---------------------------------------------------------------------------
inline NiPoint3 const &ShadowObject::GetLightDirection() 
{ 
    return m_dir; 
}
//---------------------------------------------------------------------------
inline NiTextureEffect* ShadowObject::GetShadowEffect() 
{ 
    return m_spShadow; 
}
//---------------------------------------------------------------------------

#endif // SHADOWOBJECT_H

