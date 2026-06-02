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
#include "stdafx.h"

#include "ShadowGeometry.h"

#include <NiAlphaProperty.h>
#include <NiCamera.h>
#include <NiShaderFactory.h>
#include <NiMaterialProperty.h>
#include <NiRenderedTexture.h>
#include <NiShadeProperty.h>
#include <NiSourceTexture.h>
#include <NiTriShape.h>
#include <NiTriShapeDynamicData.h>
#include <NiTriStrips.h>
#include <NiVertexColorProperty.h>
#include <NiZBufferProperty.h>

//---------------------------------------------------------------------------
ShadowGeometry::ShadowGeometry(unsigned int uiMaxTriangleCount) :
    m_kCastingObjects(4), m_bForceRender(false),
    m_kVisible(8, 8), m_kCuller(&m_kVisible)
{
    m_kLightDir = -NiPoint3::UNIT_Z;

    // the camera used to project the shadow caster to create the shadow mask
    // this camera is a distant perspective camera with a narrow FOV, to 
    // approximate a parallel camera.
    m_spCamera = NiNew NiCamera;
    m_spCamera->SetTranslate(0.0f, 0.0f, 0.0f);

    // Need to generate an orthonormal frame, with the X axis mapping to the
    // light direction.  The other dimensions do not matter, so long as they
    // define a right-handed orthonormal frame
    NiPoint3 kYVec = -m_kLightDir.Perpendicular();
    NiPoint3 kZVec = m_kLightDir.UnitCross(kYVec);

    // Rotate the camera based on the orthonormal frame
    NiMatrix3 kRotation(m_kLightDir, kYVec, kZVec);
    m_spCamera->SetRotate(kRotation);
    m_spCamera->Update(0.0f);

    NiFrustum kFrust = m_spCamera->GetViewFrustum();
    kFrust.m_fFar  = 10000.0f;
    kFrust.m_fNear =   1.0f;
    m_spCamera->SetViewFrustum(kFrust);

    m_spCasterMaterial = NiSingleShaderMaterial::Create("ShadowSkinned");
    PG_ASSERT_LOG(m_spCasterMaterial);

    m_spGaussBlurMaterial = NiSingleShaderMaterial::Create("ShadowGaussBlur");
    PG_ASSERT_LOG(m_spGaussBlurMaterial);
    
    // The following (master) properties are used to batch render the shadow
    // casting objects.  These properties ensure that the shadow casters 
    // appear opaque and flat, the color of the desired shadow.  If these 
    // were not used, the shadow could contain surface colors of the casters'
    // textures, which would look incorrect.
    m_spPropertyState = NiNew NiPropertyState;

    NiVertexColorProperty* pkVC = NiNew NiVertexColorProperty;
    pkVC->SetSourceMode(NiVertexColorProperty::SOURCE_IGNORE);
    pkVC->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
    m_spPropertyState->SetProperty(pkVC);

    NiMaterialProperty* pkMat = NiNew NiMaterialProperty;
    pkMat->SetEmittance(NiColor(0.6f, 0.6f, 0.6f));
    pkMat->SetAlpha(0.6f);
    m_spPropertyState->SetProperty(pkMat);

    NiTexturingProperty* pkTex = NiNew NiTexturingProperty;
    m_spPropertyState->SetProperty(pkTex);

    NiShadeProperty* pkShade = NiNew NiShadeProperty;
    pkShade->SetSmooth(false);
    m_spPropertyState->SetProperty(pkShade);

    if (uiMaxTriangleCount == 0)
        m_uiMaxTriangleCount = TRIANGLE_COUNT;
    else
        m_uiMaxTriangleCount = uiMaxTriangleCount;
    m_uiMaxVertexCount = 3 * m_uiMaxTriangleCount;

    // Create the shadow geometry - this is a dynamic trishape that will be
    // refilled each frame to include ground (recipient) triangles that fall
    // within the shadow frustum.  It must be marked as dynamic, as both the
    // geometry and the vertex/triangle counts will change each frame
    NiPoint3* pkVertex = NiNew NiPoint3[m_uiMaxVertexCount];
    PG_ASSERT_LOG(pkVertex);
    NiPoint2* pkTexC = NiNew NiPoint2[m_uiMaxVertexCount];
    PG_ASSERT_LOG(pkTexC);
    unsigned short* pusConnect = NiAlloc(unsigned short, 
        m_uiMaxTriangleCount * 3);
    PG_ASSERT_LOG(pusConnect);
    NiTriShapeDynamicData* pkTriData = NiNew NiTriShapeDynamicData(
        m_uiMaxVertexCount, pkVertex, 0, 0, pkTexC, 1, 
        NiGeometryData::NBT_METHOD_NONE, m_uiMaxTriangleCount, pusConnect);
    PG_ASSERT_LOG(pkTriData);
    m_spShadowGeometry = NiNew NiTriShape(pkTriData);
    PG_ASSERT_LOG(m_spShadowGeometry);
    m_spShadowGeometry->SetName("ShadowGeometry");
    m_spShadowGeometry->SetActiveVertexCount(0);
    m_spShadowGeometry->SetActiveTriangleCount(0);

    // The shadow is alpha blended, so it must not write the ZBuffer.
    NiZBufferProperty* pkZ = NiNew NiZBufferProperty;
    PG_ASSERT_LOG(pkZ);
    pkZ->SetZBufferTest(true);
    pkZ->SetZBufferWrite(false);
    m_spShadowGeometry->AttachProperty(pkZ);

    //if (WorldManager::Get()->GetNeedManualBlend())
    //{
    //    m_spShadowMaterial = NiSingleShaderMaterial::Create("ShadowNoAlpha");
    //}
    //else
    //{
        m_spShadowMaterial = NiSingleShaderMaterial::Create("ShadowDefault");

        // The shadow is a darkmap, so is multiplies the framebuffer color
        NiAlphaProperty* pkAlpha = NiNew NiAlphaProperty;
        pkAlpha->SetAlphaBlending(true);
        pkAlpha->SetSrcBlendMode(NiAlphaProperty::ALPHA_ZERO);
        pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_SRCCOLOR);
        m_spShadowGeometry->AttachProperty(pkAlpha);
    //}

    PG_ASSERT_LOG(m_spShadowMaterial);
    m_spShadowGeometry->ApplyAndSetActiveMaterial(m_spShadowMaterial);
}
//----------------------------------------------------------------------------
ShadowGeometry* ShadowGeometry::Create(unsigned int uiDetail, 
    unsigned int uiMaxTriangleCount)
{
    ShadowGeometry* pkThis = NiNew ShadowGeometry(uiMaxTriangleCount);
	unsigned int uiTexWidth;
	NiRenderer* pkRenderer = 0;

    // No texture file specified, so we must create a rendered texture
    // that will be update each frame
	pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

    // We match the texture to the depth of the backbuffer for optimal
    // performance
    NiTexture::FormatPrefs kPrefs;
    kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;

    uiTexWidth = 1 << uiDetail;

    NiRenderedTexture* pkRenderedTexture = NiRenderedTexture::Create( 
        uiTexWidth, uiTexWidth, pkRenderer, kPrefs);
    pkThis->m_spTexture = pkRenderedTexture;
    pkThis->m_spRenderTargetGroup = NiRenderTargetGroup::Create(
        pkRenderedTexture->GetBuffer(), pkRenderer, true, true);

    if (!pkThis->m_spTexture)
    {
        //WorldManager::Log("Cannot create shadow texture\n");
        SAFE_DELETE_NI(pkThis);
        return NULL;
    }

    //if (!WorldManager::Get()->GetNeedManualBlend())
    //{
        pkRenderedTexture = NiRenderedTexture::Create( 
            uiTexWidth, uiTexWidth, pkRenderer, kPrefs);
        pkThis->m_spBlurredTexture = pkRenderedTexture;
        pkThis->m_spBlurredRenderTargetGroup = NiRenderTargetGroup::Create(
            pkRenderedTexture->GetBuffer(), pkRenderer, true);

        if (!pkThis->m_spBlurredTexture)
        {
            //WorldManager::Log("Cannot create blurred shadow texture\n");
            SAFE_DELETE_NI(pkThis);
            return NULL;
        }

        // Add screen space blurring geometry using new NiScreenElements
        NiScreenElementsPtr spGeom = NiNew NiScreenElements(
            NiNew NiScreenElementsData(false, false, 1));
        PG_ASSERT_LOG(spGeom);
        int iQuadHandle = spGeom->Insert(4);

        spGeom->SetRectangle(iQuadHandle, 0, 0, 1.0f, 1.0f);
        spGeom->UpdateBound();
        spGeom->SetTextures(iQuadHandle, 0, 0,
            0, 1, 1);
        
        NiTexturingPropertyPtr spPSVSQuadTex = NiNew NiTexturingProperty; 
        PG_ASSERT_LOG(spPSVSQuadTex);
        spPSVSQuadTex->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
        spGeom->AttachProperty(spPSVSQuadTex);

        spGeom->ApplyAndSetActiveMaterial(pkThis->m_spGaussBlurMaterial);

        spPSVSQuadTex->SetBaseTexture(pkThis->m_spTexture);

        spGeom->Update(0.0f);
        spGeom->UpdateEffects();
        spGeom->UpdateProperties();

        pkThis->m_spBlurGeom = spGeom;
    //}

    // Since we may generate texture coordinates outside of the range [0,1],
    // we must use clamping to avoid strange wrapping artifacts.
    NiTexturingProperty::Map* pkMap = NiNew NiTexturingProperty::Map(
        pkThis->m_spTexture, 0, NiTexturingProperty::CLAMP_S_CLAMP_T,
        NiTexturingProperty::FILTER_BILERP);
    PG_ASSERT_LOG(pkMap);
    NiTexturingProperty* pkTex = NiNew NiTexturingProperty();
    PG_ASSERT_LOG(pkTex);
    pkTex->SetBaseMap(pkMap);
    pkTex->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);

    pkThis->m_spShadowGeometry->AttachProperty(pkTex);
    pkThis->m_spShadowGeometry->UpdateProperties();

    pkThis->UpdateSampleCoeffsGaussBlur();

    return pkThis;
}
//----------------------------------------------------------------------------
ShadowGeometry::~ShadowGeometry()
{
}
//---------------------------------------------------------------------------
void ShadowGeometry::SetCaster(NiAVObject* pkCaster)
{
	m_kCastingObjects.RemoveAll();

    m_spCaster = pkCaster;

    // Traverse the new caster to find all of the leaf geometry objects.
    // Note that this will not clear out the list of caster geometry, and
    // will thus _add_ caster geometry to the shadow system.  However, only
    // the most recently-set caster will affect the centerpoint and bounding
    // sphere of the shadow
    RecursiveStoreCasterObjects(pkCaster);
}
//---------------------------------------------------------------------------
void ShadowGeometry::RecursiveStoreCasterObjects(NiAVObject* pkObject)
{
    // Find all leaf geometry objects and add them to the caster geometry 
    // list, to be rendered as batch
    if (NiIsKindOf(NiGeometry, pkObject))
    {
        m_kCastingObjects.Add(pkObject);
    }
    else if (NiIsKindOf(NiNode, pkObject))
    {
        NiNode* pkNode = (NiNode*)pkObject;
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild = pkNode->GetAt(i);
            if (pkChild)
                RecursiveStoreCasterObjects(pkChild);
        }
    }
}
//----------------------------------------------------------------------------
void ShadowGeometry::Click(float fTime, NiCamera* pkSceneCamera, 
    bool bVisible, bool bUpdateImage)
{
    PG_ASSERT_LOG(m_spCaster && m_spTexture);

    // If the visible flag is false, clear out the shadow geometry and skip
    // rendering the shadow, as it won't matter to the final, rendered image.
    if (!bVisible)
    {
        m_spShadowGeometry->SetActiveVertexCount(0);
        m_spShadowGeometry->SetActiveTriangleCount(0);
        return;
    }

    // Other versions of the shadow camera cull the entire shadow process if
    // the cast shadow geometry was out of view.  Since we expect the shadow
    // to be in view basically all of the time in this case, we skip this 
    // step here and always update

    // base the light direction on the fires in the world.
    //m_kLightDir = WorldManager::Get()->GetSmoothedFireVector(
        //m_spCaster->GetWorldBound().GetCenter());
	m_kLightDir = NiPoint3::UNIT_Z * -1;

    // move the shadow camera (and thus the apparent light)
    UpdateShadowCamera(fTime);

    if (bUpdateImage)
    {
        NiRenderer* pkRenderer = NiRenderer::GetRenderer();
        PG_ASSERT_LOG(pkRenderer);

        // update the background color to the renderer
        //if (WorldManager::Get()->GetNeedManualBlend())
        //{
        //    NiColorA kShadowColor(0.0f, 0.0f, 0.0f, 1.0f);
        //    NiShaderFactory::UpdateGlobalShaderConstant(
        //        "g_afShadowColor", sizeof(float)*4, &kShadowColor);

        //    // "No alpha" _must_ clear to black and write a non-black color
        //    pkRenderer->SetBackgroundColor(NiColor::WHITE);
        //}
        //else
        //{
            NiColorA kShadowColor(0.1f, 0.1f, 0.1f, 1.0f);
            NiShaderFactory::UpdateGlobalShaderConstant(
                "g_afShadowColor", sizeof(float)*4, &kShadowColor);

            pkRenderer->SetBackgroundColor(NiColor::WHITE);
        //}

        pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup,
            NiRenderer::CLEAR_ALL);

        // renderer camera port settings
        pkRenderer->SetCameraData(m_spCamera);

        NiAccumulatorPtr spSorter = pkRenderer->GetSorter();
        pkRenderer->SetSorter( NULL );

        // Render all leaf shadow caster geometry in a batch, replacing their
        // built-in renderstates with the single shadow state.  This will 
        // cause all of the shadow casters to be rendered as dark gray, making
        // them look like shadows.
        pkRenderer->BeginBatch(m_spPropertyState, NULL);

        // This code assumes the following:
        // * The same shader can be used to render all shadow casters, and is
        // stored in m_spCasterShader
        // * The renderer uses the shader applied to the first batched object
        // for all subsequent objects in the batch
        // 
        NiGeometryPtr spFirstBatchedObject = NULL;
        const NiMaterial* pkFirstBatchedReplacedMaterial = NULL;

        for (unsigned int i = 0; i < m_kCastingObjects.GetSize(); i++)
        {
            NiGeometry* pkGeom = 
                NiSmartPointerCast(NiGeometry, m_kCastingObjects.GetAt(i));

            // Object must be palette skinned - later, we
            // should handle the two cases with different shaders and 
            // batches.  In addition, there is currently an issue that allows
            // an object that is not technically skinned (i.e. one-bone 
            // rigid) to have a skin instance, but no partitions.
            // We check the shader to ensure that the object is palette 
            // skinned, which is a requirement for the shader
            if(pkGeom->GetAppCulled() || !pkGeom->GetShaderFromMaterial()
			|| !pkGeom->GetSkinInstance() || !pkGeom->GetSkinInstance()->GetSkinPartition())
			{
				continue;
			}

            if (!spFirstBatchedObject)
            {
                spFirstBatchedObject = pkGeom;
                pkFirstBatchedReplacedMaterial = pkGeom->GetActiveMaterial();
                pkGeom->ApplyAndSetActiveMaterial(m_spCasterMaterial);
            }

            if (NiIsKindOf(NiTriStrips, pkGeom))
            {
                pkRenderer->BatchRenderStrips((NiTriStrips*)pkGeom);
            }
            else if (NiIsKindOf(NiTriShape, pkGeom))
            {
                pkRenderer->BatchRenderShape((NiTriShape*)pkGeom);
            }
        }

        pkRenderer->EndBatch();

        pkRenderer->EndUsingRenderTargetGroup();

        // return the original shader to the object whose shader we replaced
        if (spFirstBatchedObject)
        {
            spFirstBatchedObject->SetActiveMaterial(
                pkFirstBatchedReplacedMaterial);
        }

        //if (!WorldManager::Get()->GetNeedManualBlend())
        //{
            // Now, render the two blurring passes
            m_spBlurGeom->GetPropertyState()->GetTexturing()->SetBaseTexture(
                m_spTexture);
            pkRenderer->BeginUsingRenderTargetGroup(
                m_spBlurredRenderTargetGroup, NiRenderer::CLEAR_BACKBUFFER);
            pkRenderer->SetScreenSpaceCameraData();
            m_spBlurGeom->Draw(pkRenderer);
            pkRenderer->EndUsingRenderTargetGroup();

            m_spBlurGeom->GetPropertyState()->GetTexturing()->SetBaseTexture(
                m_spBlurredTexture);
            pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup,
                NiRenderer::CLEAR_BACKBUFFER);
            pkRenderer->SetScreenSpaceCameraData();
            m_spBlurGeom->Draw(pkRenderer);
            pkRenderer->EndUsingRenderTargetGroup();
        //}

        pkRenderer->SetSorter( spSorter );
    }

    // Determine which triangles fall inside of the shadow frustum.
    UpdateShadowGeometry();
}
//---------------------------------------------------------------------------
void ShadowGeometry::UpdateShadowCamera(float fTime)
{
    // this function moves the shadow camera so that it appears to view the
    // target (caster) from infinity, facing a fixed direction.  This is done
    // by  moving the camera so the the desired fixed direction vector is
    // always coincident with the line through the caster's bounding volume
    // center and the camera location
    if (!m_spCamera)
        return;

    // get the "look at" point
    NiPoint3 kTarget = m_spCaster->GetWorldBound().GetCenter();

    // Need to generate an orthonormal frame, with the X axis mapping to the
    // light direction.  The other dimensions do not matter, so long as they
    // define a right-handed orthonormal frame
    NiPoint3 kYVec = -m_kLightDir.Perpendicular();
    NiPoint3 kZVec = m_kLightDir.UnitCross(kYVec);

    // Rotate the camera based on the orthonormal frame
    NiMatrix3 kRotation(m_kLightDir, kYVec, kZVec);
    m_spCamera->SetRotate(kRotation);
    m_spCamera->Update(0.0f);

    // set the field of view of the camera to enclose the bounding sphere of 
    // the caster object.
    float fRadius = m_spCaster->GetWorldBound().GetRadius();

    NiFrustum kFrust = m_spCamera->GetViewFrustum();
    kFrust.m_bOrtho = true;
    kFrust.m_fLeft = -fRadius;
    kFrust.m_fRight = fRadius;
    kFrust.m_fTop = fRadius;
    kFrust.m_fBottom = -fRadius;
    kFrust.m_fNear = 1.0f;
    kFrust.m_fFar = 5.0f * fRadius;
    m_spCamera->SetViewFrustum(kFrust);

    m_fOOCameraWidth = 0.5f / (fRadius);

    // translate the camera to a point such that the camera is looking
    // directly at the target point
    m_kCameraCenter = kTarget - (m_kLightDir * fRadius * 2.0f);
    m_spCamera->SetTranslate(m_kCameraCenter);

    m_spCamera->Update(fTime);
}
//---------------------------------------------------------------------------
void ShadowGeometry::UpdateShadowGeometry()
{
    m_spShadowGeometry->SetActiveVertexCount(0);
    m_spShadowGeometry->SetActiveTriangleCount(0);

    // Ensure that the shadow caster camera is actually positioned over the
    // terrain
    //NiPoint3 kTemp;
    //if (!WorldManager::GetTerrainManager()->
    //    GetIntersection(m_spCamera->GetWorldLocation(), 
    //    m_spCamera->GetWorldDirection(), kTemp))
    //{
    //    // No terrain was found by pick operation.
    //    PG_ASSERT_LOG("Error: no pick intersection occurred");
    //}

    // Traverse the ground object using frustum culling for quick outs
    // The ground geometry should ideally be diced into a quad-tree 
    // or some other data structure to allow maximum benefits from
    // frustum culling.
    TraverseGroundGeometry(m_spGround);

    m_spShadowGeometry->GetModelData()->MarkAsChanged(
        NiGeometryData::VERTEX_MASK | NiGeometryData::TEXTURE_MASK | 
        NiTriBasedGeomData::TRIANGLE_INDEX_MASK |
        NiTriBasedGeomData::TRIANGLE_COUNT_MASK);

    UpdateShadowGeometryBound();
}
//---------------------------------------------------------------------------
void ShadowGeometry::UpdateShadowGeometryBound()
{
    // Manually set the bounding sphere of the shadow geometry to ensure
    // it is not culled by the engine
    float fRadius = m_spCaster->GetWorldBound().GetRadius();

    NiBound kSphere;
    kSphere.SetCenterAndRadius(m_spCaster->GetWorldBound().GetCenter(),
        fRadius * 1.05f);
    m_spShadowGeometry->SetModelBound(kSphere);

    m_spShadowGeometry->Update(0.0f);
}
//---------------------------------------------------------------------------
void ShadowGeometry::TraverseGroundGeometry(NiAVObject* pkObject)
{
    m_kVisible.RemoveAll();
    m_kCuller.Process(m_spCamera, pkObject, 0);
        
    const unsigned int uiQuantity = m_kVisible.GetCount();
    for (unsigned int i = 0; i < uiQuantity; i++)
    {
        NiGeometry* pkGeom = &m_kVisible.GetAt(i);
        AddToShadowGeometry(NiDynamicCast(NiTriBasedGeom, pkGeom));
    }
}
//---------------------------------------------------------------------------
void ShadowGeometry::AddToShadowGeometry(NiTriBasedGeom* pkTriGeom)
{
    PG_ASSERT_LOG(pkTriGeom);
   
    // This code assumes that model space vertices ARE in world space
    NiPoint3* pkWorldVerts = pkTriGeom->GetVertices();
    PG_ASSERT_LOG(pkWorldVerts);
    NiTriBasedGeomData* pkTriData = 
        (NiTriBasedGeomData*)pkTriGeom->GetModelData();
    PG_ASSERT_LOG(pkTriData);

    unsigned short s0;
    unsigned short s1;
    unsigned short s2;
    NiPoint3* pkP0;
    NiPoint3* pkP1;
    NiPoint3* pkP2;

    unsigned short usTris = pkTriData->GetTriangleCount();
    for (unsigned short i = 0; i < usTris; i++)
    {
        // Clip each world triangle to the camera's frustum
        pkTriData->GetTriangleIndices(i, s0, s1, s2);
        pkP0 = &pkWorldVerts[s0];
        pkP1 = &pkWorldVerts[s1];
        pkP2 = &pkWorldVerts[s2];

		CullTriAgainstCameraFrustum(*pkP0, *pkP1, *pkP2);
    }
}
//---------------------------------------------------------------------------
bool ShadowGeometry::GenerateCameraRay(unsigned int uiIndex, NiPoint3& kPt, 
    NiPoint3& kDir)
{
    const NiFrustum& kFrust = m_spCamera->GetViewFrustum();
    float fVx = (uiIndex & 0x1) ? kFrust.m_fRight :  kFrust.m_fLeft;
    float fVy = (uiIndex & 0x2) ? kFrust.m_fTop : kFrust.m_fBottom;

    // convert world view plane coordinates to ray with kDir and kOrigin
    // kDir: camera world location to view plane coordinate
    // kOrigin: camera world location
    m_spCamera->ViewPointToRay(fVx, fVy, kPt, kDir);
    return true;
}
//---------------------------------------------------------------------------
void ShadowGeometry::CullTriAgainstCameraFrustum(NiPoint3& kV0, 
    NiPoint3& kV1, NiPoint3& kV2)
{
    // This method does no clipping, but "extends" the reliance on 
    // CLAMP_S_CLAMP_T texture wrapping modes
    
    // Cull triangle vertices against camera planes.  If all three vertices
    // fall on the outside of any single plane, cull.  Skip the near and far
    // planes (indices i = 0,1).
    unsigned int i = NiFrustumPlanes::LEFT_PLANE;
    for (; i < NiFrustumPlanes::MAX_PLANES; i++)
    {
        const NiPlane& kPlane = m_kCuller.GetFrustumPlanes().GetPlane(i);

        // vert 0 in?
        if (kPlane.Distance(kV0) >= 0.0f) 
            continue;

        // vert 1 in?
        if (kPlane.Distance(kV1) >= 0.0f) 
            continue;

        // vert 2 in?
        if (kPlane.Distance(kV2) >= 0.0f) 
            continue;

        // all verts outside of a single plane - culled
        return;
    }

    NiPoint3 kE;
    
    NiPlane kTriPlane(kV0, kV1, kV2);
    NiPoint3 const &kNorm = kTriPlane.m_kNormal;

    // Note that camera is assumed to be ortho
    NiPoint3 akCamRayPts[4];
    NiPoint3 akCamRayDirs[4];
    NiPoint3 akCamRayIntersect[4];
    float fPlaneDistances[4];

    for (i = 0; i < 4; i++)
    {
        if (!GenerateCameraRay(i, akCamRayPts[i], akCamRayDirs[i]))
            goto CannotCull;

        fPlaneDistances[i] = kTriPlane.Distance(akCamRayPts[i]);
        // a point is in front of the camera
        if (fPlaneDistances[i] >= 0.0f)
            break;
    }

    // No intersection if all of the four ray points are behind the triangle
    // if all 4 frustum points are behind tri, then skip
    if (i == 4)
        return;

    // fill in the remaining points and distances
    for (/* */; i < 4; i++)
    {
        if (!GenerateCameraRay(i, akCamRayPts[i], akCamRayDirs[i]))
            goto CannotCull;

        fPlaneDistances[i] = kTriPlane.Distance(akCamRayPts[i]);
    }

    // Next, compute the intersection points of the frustum corners with the
    // plane of the triangle.  There will be four of them if the frustum
    // goes through the triangle.  If not, there may be far fewer.  However,
    // for now, if any view frustum edge doesn't intersect the plane of the
    // triangle, we give up and add the triangle.
    for (i = 0; i < 4; i++)
    {
        // compute the intersection of the plane of the triangle with the ray
        float const fNormDot = kNorm.Dot(akCamRayDirs[i]);
        
        // if the rays are in the same direction, no intersection (backfacing)
        if (fNormDot >= -1.0e-5f)
            goto CannotCull;
        
        akCamRayIntersect[i] 
            = akCamRayPts[i] - akCamRayDirs[i] 
            * (fPlaneDistances[i] / fNormDot);
    }
    
    // now, test each of the frustum intersection points against each of the
    // triangle edges.  If all 4 points are on the same side of a triangle
    // edge, cull the triangle

    // We will compute the cross product of the triangle edge and the vector
    // from one of the edge vertices to the frustum point, and then compute
    // the dot of that vector with the triangle normal.  The sign of this dot
    // is the CCW/CW value
    kE = kV1 - kV0;
    for (i = 0; i < 4; i++)
    {
        // if the dot triple product ((UxV)*W) is positive, then the frustum 
        // point is on the inside of the given triangle edge, and we cannot
        // cull based on this edge.
        if (kE.Cross(akCamRayIntersect[i] - kV1).Dot(kNorm) >= 1.0e-5f)
            break;
    }
    // all points on outside of single edge - culled
    if (i == 4)
        return;

    kE = kV2 - kV1;
    for (i = 0; i < 4; i++)
    {
        // if the dot triple product ((UxV)*W) is positive, then the frustum 
        // point is on the inside of the given triangle edge, and we cannot
        // cull based on this edge.
        if (kE.Cross(akCamRayIntersect[i] - kV2).Dot(kNorm) >= 1.0e-5f)
            break;
    }
    // all points on outside of edge - culled
    if (i == 4)
        return;

    kE = kV0 - kV2;
    for (i = 0; i < 4; i++)
    {
        // if the dot triple product ((UxV)*W) is positive, then the frustum 
        // point is on the inside of the given triangle edge, and we cannot
        // cull based on this edge.
        if (kE.Cross(akCamRayIntersect[i] - kV0).Dot(kNorm) >= 1.0e-5f)
            break;
    }
    // all points on outside of edge - culled
    if (i == 4)
        return;

CannotCull:

    NiPoint3 akTri[3];
    akTri[0] = kV0;
    akTri[1] = kV1;
    akTri[2] = kV2;

    AddShadowTriangle(akTri);
}    
//---------------------------------------------------------------------------
void ShadowGeometry::AddShadowTriangle(NiPoint3 akV[3])
{
    unsigned short usVertCount = m_spShadowGeometry->GetActiveVertexCount();
    unsigned short usTriCount = m_spShadowGeometry->GetActiveTriangleCount();

    if ((unsigned int)(usVertCount + 3) <= m_uiMaxVertexCount && 
        (unsigned int)(usTriCount + 1) <= m_uiMaxTriangleCount)
    {
        // For texture coordinates, generate s and t based on point Q's
        // distance along the RIGHT(R) and UP(U) relative to the 
        // projection of the camera's location(P) (assuming ortho projection
        // and a centered, square frustum, L = -R = T = -B) onto the ground
        // geometry.
        //
        // s = (R * (Q - P)) * scaling + 0.5
        // t = (U * (Q - P)) * scaling + 0.5
        NiPoint2* pkTex = m_spShadowGeometry->GetTextures();
        PG_ASSERT_LOG(pkTex);

        for (unsigned int i = 0; i < 3; i++)
        {
            NiPoint3 kDiff = akV[i] - m_kCameraCenter;

            pkTex[usVertCount + i].x = (m_spCamera->GetWorldRightVector() *
                kDiff) * m_fOOCameraWidth + 0.5f;
            // Negated direction is due to rendered texture being inverted
            pkTex[usVertCount + i].y = (-m_spCamera->GetWorldUpVector() *
                kDiff) * m_fOOCameraWidth + 0.5f;
        }

        // We know z-axis is up and in the case of MetalWars, the 
        // units are in inches, so we offset upward in z by a few inches
        float const fZOffset = 1.0f;
        akV[0].z += fZOffset;
        akV[1].z += fZOffset;
        akV[2].z += fZOffset;

        NiPoint3* pkVerts = m_spShadowGeometry->GetVertices();
        PG_ASSERT_LOG(pkVerts);
        pkVerts[usVertCount] = akV[0];        
        pkVerts[usVertCount + 1] = akV[1];        
        pkVerts[usVertCount + 2] = akV[2];        

        unsigned short usBase = usTriCount * 3;
        unsigned short* pusConnect = m_spShadowGeometry->GetTriList();
        PG_ASSERT_LOG(pusConnect);
        pusConnect[usBase] = usVertCount;
        pusConnect[usBase + 1] = usVertCount + 1;
        pusConnect[usBase + 2] = usVertCount + 2;

        m_spShadowGeometry->SetActiveVertexCount(usVertCount + 3);
        m_spShadowGeometry->SetActiveTriangleCount(usTriCount + 1);
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static float GaussianDistribution(float x, float y, float rho)
{
    float g = 1.0f / sqrtf(2.0f * NI_PI * rho * rho);
    g *= expf(-(x * x + y * y) / (2 * rho * rho));

    return g;
}
//---------------------------------------------------------------------------
void ShadowGeometry::UpdateSampleCoeffsGaussBlur()
{
    NiPoint2 akOffsets[13];
    float aakWeights[13][4];
    // Get the texture coordinate offsets to be used inside 
    // the GaussBlur5x5 pixel shader.
    float tu = 1.0f / (float)m_spTexture->GetWidth();
    float tv = 1.0f / (float)m_spTexture->GetHeight() ;

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
            akOffsets[index] = NiPoint2(x * tu, y * tv);
            float fGauss = GaussianDistribution((float)x, (float)y, 1.0f);
            aakWeights[index][0] = fGauss; 
            aakWeights[index][1] = fGauss; 
            aakWeights[index][2] = fGauss; 
            aakWeights[index][3] = fGauss; 

            totalWeight += aakWeights[index][0];

            index++;
        }
    }

    PG_ASSERT_LOG(index == 13);

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

    NiShaderFactory::UpdateGlobalShaderConstant("g_akGaussSampleOffsets",
        sizeof(NiPoint2) * 13, akOffsets);

    NiShaderFactory::UpdateGlobalShaderConstant("g_akGaussSampleWeights",
        sizeof(float) * 4 * 13, aakWeights);

}
