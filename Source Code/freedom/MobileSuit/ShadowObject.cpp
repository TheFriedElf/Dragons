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

#include "stdafx.h"
#include <NiApplication.h>
#include <NiSystem.h>

#include "ShadowObject.h"
#include "MasterPropertyAccumulator.h"
//---------------------------------------------------------------------------
ShadowObject::ShadowObject() :
    m_kVisible(128, 128),
    m_kCuller(&m_kVisible)
{
    // The subset of the main scene graph used to cast a shadow
    m_spCaster = 0;

    // the direction of the infinite light casting the shadow
    m_dir = NiPoint3(0.0f, -1.0f, 0.0f);

    // the camera used to project the shadow caster to create the shadow mask
    // this camera is a distant perspective camera with a narrow FOV, to 
    // approximate a parallel camera.
    m_spCamera = NiNew NiCamera;
    m_spCamera->SetTranslate(0.0f, 0, 5000.0f);
    m_spCamera->Update(0.0f);

    NiFrustum frust = m_spCamera->GetViewFrustum();
    frust.m_fFar  = 10000.0f;
    frust.m_fNear =   100.0f;
    m_spCamera->SetViewFrustum(frust);

    m_spSort = NiNew MasterPropertyAccumulator;

    // The following (master) properties are attached to the caster scene
    // prior to rendering the shadow mask (and detached immediately 
    // afterwards).  These properties ensure that the shadow caster appears
    // opaque and flat, the color of the desired shadow.  If these were not
    // attached, the shadow could contain surface colors of the caster's
    // textures, which would look incorrect.
    NiVertexColorPropertyPtr spVC = NiNew NiVertexColorProperty;
    spVC->SetSourceMode(NiVertexColorProperty::SOURCE_IGNORE);
    spVC->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
    m_spSort->m_propertyList.AddHead((NiProperty*)spVC);

    NiMaterialPropertyPtr spMat = NiNew NiMaterialProperty;
	spMat->SetAmbientColor(NiColor::BLACK);
    spMat->SetEmittance(NiColor(0.0f, 0.0f, 0.0f));
	spMat->SetDiffuseColor(NiColor::BLACK);
	spMat->SetSpecularColor(NiColor::BLACK);
	spMat->SetShineness(0);
    spMat->SetAlpha(1.0f);
    m_spSort->m_propertyList.AddHead((NiProperty*)spMat);

    NiTexturingProperty* pkTex = NiNew NiTexturingProperty(
        NiApplication::ConvertMediaFilename("white.bmp"));
    m_spSort->m_propertyList.AddHead(pkTex);

    NiShadeProperty* pkShade = NiNew NiShadeProperty;
    pkShade->SetSmooth(false);
    m_spSort->m_propertyList.AddHead(pkShade);
}
//---------------------------------------------------------------------------
ShadowObject* ShadowObject::Create(NiRenderer* pRenderer, 
    unsigned int uiDetail)
{
    ShadowObject* pThis = NiNew ShadowObject;

    pThis->m_spRenderer = pRenderer;

    if(!pThis->m_spRenderer)
    {
        delete pThis;
        return NULL;
    }

    // the offscreen renderer used to render the shadow mask.  The 
    // frontbuffer of this renderer is a texture, and is used as the shadow 
    // mask.  Note that the detail level controls the size of the texture
    // A larger backbuffer will increase the shadow quality, at the price
    // of lowered performance due to increased fill rate demands upon the
    // hardware

    NiTexture::FormatPrefs kPrefs;
    const NiRenderTargetGroup* pkRTGroup = 
        pRenderer->GetDefaultRenderTargetGroup();
    const NiPixelFormat* pkPixelFormat = pkRTGroup->GetPixelFormat(0);
    if (pkPixelFormat->GetBitsPerPixel() == 16)
        kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::HIGH_COLOR_16;
    else
        kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;

	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;

    pThis->m_spRenderedTexture = NiRenderedTexture::Create(1<<uiDetail, 
        1<<uiDetail, pThis->m_spRenderer, kPrefs);

    if(!pThis->m_spRenderedTexture)
    {
        //NiOutputDebugString("Cannot create offscreen framebuffer\n");
        delete pThis;
        return NULL;
    }

    pThis->m_spRenderTargetGroup = NiRenderTargetGroup::Create(
        pThis->m_spRenderedTexture->GetBuffer(), pThis->m_spRenderer, true,
        true);

    // Create the parallel projected shadow effect that will be the visual
    // representation of the shadow in the main scene
    pThis->m_spShadow = NiNew NiTextureEffect;
    pThis->m_spShadow->SetEffectTexture(pThis->m_spRenderedTexture);
    pThis->m_spShadow->SetTextureFilter(
        NiTexturingProperty::FILTER_BILERP);
    pThis->m_spShadow->SetTextureClamp(
        NiTexturingProperty::CLAMP_S_CLAMP_T);
    pThis->m_spShadow->SetTextureType(NiTextureEffect::PROJECTED_SHADOW);
    pThis->m_spShadow->SetTextureCoordGen(NiTextureEffect::WORLD_PARALLEL);

    // update the background color to the renderer
    const NiColor bgColor(1.0f, 1.0f, 1.0f);
    pThis->m_spRenderer->SetBackgroundColor(bgColor);

	pThis->m_spSE_DownAlpha = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	pThis->m_spSE_DownAlpha->Insert(4);
    pThis->m_spSE_DownAlpha->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    pThis->m_spSE_DownAlpha->UpdateBound();
    pThis->m_spSE_DownAlpha->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

	NiShaderPtr spDownSample = NiShaderFactory::GetInstance()->RetrieveShader("DownAlpha", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spDownSample); 
	pThis->m_spSE_DownAlpha->ApplyAndSetActiveMaterial(pkMaterial);
 
	NiTexturingProperty * pkProperty = NiNew NiTexturingProperty;
	pkProperty->SetApplyMode(NiTexturingProperty::APPLY_REPLACE); 
	pkProperty->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	pkProperty->SetShaderMap(0, NiNew NiTexturingProperty::ShaderMap(pThis->m_spRenderedTexture, 0));
  
	pThis->m_spSE_DownAlpha->AttachProperty(pkProperty);
	pThis->m_spSE_DownAlpha->UpdateEffects();
    pThis->m_spSE_DownAlpha->UpdateProperties();
    pThis->m_spSE_DownAlpha->Update(0.0f);	

    return pThis;
}
//---------------------------------------------------------------------------
ShadowObject::~ShadowObject()
{
    m_spSort = 0;

    m_spCaster = 0;
    m_spShadow = 0;
    m_spCamera = 0;
    m_spRenderer = 0;

	m_spSE_DownAlpha = 0;
}
//---------------------------------------------------------------------------
void ShadowObject::ClickAndStuff(float fTime)
{
    if(!(m_spRenderer && m_spCaster && m_spRenderedTexture))
        return;

    // move the shadow camera (and thus the apparent light)
    UpdateShadowCamera(fTime);

	m_spRenderer->SetBackgroundColor(NiColor(1,1,1));
    m_spRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup,
        NiRenderer::CLEAR_ALL);

    // Attach the master property sorter before rendering to ensure that the
    // override properties will be used to draw the object
	NiAccumulatorPtr spOldSorter = m_spRenderer->GetSorter();
    m_spRenderer->SetSorter( m_spSort );

    NiDrawScene( m_spCamera, m_spCaster, m_kCuller );

	m_spRenderer->SetSorter( spOldSorter );

    m_spRenderer->EndUsingRenderTargetGroup();

	m_spRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_NONE); 
	m_spRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spSE_DownAlpha->Draw(m_spRenderer->GetRenderer());
 	m_spRenderer->EndUsingRenderTargetGroup();

    // update the drawn representations of the shadows
    UpdateProjection();

	NiPlane	kClipPlane[2];

	kClipPlane[0].SetNormal(NiPoint3(0,0,1));
	kClipPlane[0].SetConstant(m_spCaster->GetWorldTranslate().z-60);
	kClipPlane[1].SetNormal(NiPoint3(0,0,-1));
	kClipPlane[1].SetConstant(m_spCaster->GetWorldTranslate().z+10);

	m_spShadow->SetClippingPlaneEnable(true);
	m_spShadow->SetModelClippingPlane(kClipPlane[0]);
}
//---------------------------------------------------------------------------
void ShadowObject::UpdateShadowCamera(float fTime)
{
    // this function moves the shadow camera so that it appears to view the
    // target (caster) from infinity, facing a fixed direction.  This is done
    // by  moving the camera so the the desired fixed direction vector is
    // always coincident with the line through the caster's bounding volume
    // center and the camera location
    if(!m_spCamera)
        return;

    // get the "look at" point
    NiPoint3 target = m_spCaster->GetWorldBound().GetCenter();

    // translate the camera to a distant point such that the camera is looking
    // directly at the target point
    m_spCamera->SetTranslate(target - (m_dir*5000.0f));

    m_spCamera->Update(fTime);

    // find a perpendicular vector to the camera direction and use it as the 
    // camera's new up vector.  Make the camera look at the target point with
    // the given up vector
    float fXSqr = m_dir.x*m_dir.x;
    float fYSqr = m_dir.y*m_dir.y;
    float fZSqr = m_dir.z*m_dir.z;
    if(fXSqr < fYSqr)
    {
        if(fZSqr < fXSqr)
            m_spCamera->LookAtWorldPoint(target, NiPoint3::UNIT_Z);
        else
            m_spCamera->LookAtWorldPoint(target, NiPoint3::UNIT_X);
    }
    else
    {
        if(fZSqr < fYSqr)
            m_spCamera->LookAtWorldPoint(target, NiPoint3::UNIT_Z);
        else
            m_spCamera->LookAtWorldPoint(target, NiPoint3::UNIT_Y);
    }

    m_spCamera->Update(fTime);

    // set the field of view of the camera to enclose the bounding sphere of 
    // the caster object.  The FOV is enlarged by a further 5% on each edge
    // to ensure that the edge pixels of the shadow mask are the background
    // color.  If any edge pixel is not the background color, the clamped
    // texture addressing will cause these dark pixels to be stretched across
    // then entire scene.
    float fFOV = m_spCaster->GetWorldBound().GetRadius() / 
        (target - m_spCamera->GetWorldLocation()).Length() * 1.05f;

    NiFrustum frust = m_spCamera->GetViewFrustum();
    frust.m_fLeft = -fFOV;
    frust.m_fRight = fFOV;
    frust.m_fTop = fFOV;
    frust.m_fBottom = -fFOV;
    m_spCamera->SetViewFrustum(frust);
}
//---------------------------------------------------------------------------
void ShadowObject::UpdateProjection()
{
    // note: this treats the shadow projector as a parallel projector, even
    // though the shadow camera is perspective.  We get away with this in
    // most cases because the camera center is so far away

    if(!(m_spCamera && m_spCaster))
        return;

    // the width of the entire shadow volume is equal to the diameter
    // of the world bound of the shadow caster, plus 5% on each end
    float fWidth = m_spCaster->GetWorldBound().GetRadius() * 2.1f;
    
    // this is a right parallel projection - we only need to compute the 
    // dot product of each point with the two projection plane basis
    // vectors.  Note that these are scaled by the width of the shadow
    // volume to avoid having to do the multiply for each vertex
    // Note also that we have to invert the Y axis to handle the flipped
    // nature of the shadow mask
    NiPoint3 VPU = -m_spCamera->GetWorldUpVector()/fWidth;
    NiPoint3 VPR = m_spCamera->GetWorldRightVector()/fWidth;

    // the shift by 0.5 is to move the range of texture coords inside the
    // shadow volume from [-0.5,0.5] to [0.0,1.0]
    // these origin calculations allow us to avoid subtracting VRL from 
    // each vertex in the loop
    NiPoint3 const &VRL = m_spCamera->GetWorldLocation();
    float fXOrigin = VRL*VPR - 0.5f;
    float fYOrigin = VRL*VPU - 0.5f;

    NiMatrix3 mat;

    mat.SetRow( 0, VPR );
    mat.SetRow( 1, VPU );
    mat.SetRow( 2, NiPoint3::ZERO );

    NiPoint3 trans(-fXOrigin, -fYOrigin, 0.0f);

    m_spShadow->SetModelProjectionMatrix(mat);
    m_spShadow->SetModelProjectionTranslation(trans);
    m_spShadow->Update(0.0f);
}
//---------------------------------------------------------------------------
