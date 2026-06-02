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

#ifndef SHADOWGEOMETRY_H
#define SHADOWGEOMETRY_H

#include <NiTSet.h>
#include <NiPoint3.h>
#include <NiAVObject.h>
#include <NiCullingProcess.h>
#include <NiFrustumPlanes.h>
#include <NiScreenElements.h>
class NiTriBasedGeom;
NiSmartPointer(NiCamera);
NiSmartPointer(NiPropertyState);
NiSmartPointer(NiRenderTargetGroup);
NiSmartPointer(NiShader);
NiSmartPointer(NiTexture);
NiSmartPointer(NiTriShape);
/* 
 * The ShadowGeometry class supports automatic, projected shadows.  
 * The class includes the ability to generate the shadow image each frame 
 * from a set of “caster” geometry.  Alternatively, a static shadow may be 
 * read from an image file at load time.  The class also supports projection 
 * of the shadow onto a set of “recipient” geometry, culling pieces of 
 * geometry that do not need to receive the shadow.  This creates a 
 * high-quality, high-performance shadow effect with minimal application-level 
 * code.
*/

class ShadowGeometry : public NiMemObject
{
public:
    // If a filename is specified, the given image file will be loaded and
    // used as a constant-shape shadow.  Otherwise, the shadow will be 
    // rendered from the caster geometry each frame.
    // The detail value is the log2 of the size of the shadow texture, a 
    // value that is ignored if the shadow texture is loaded from file
    // The max triangle count is the max number of shadow recipient (ground)
    // triangles that can receive the shadow at once.  Set too small, this
    // could result in objects in the area of the shadow not being shadowed
    // correctly.
    static ShadowGeometry* Create(unsigned int uiDetail = 6, 
        unsigned int uiMaxTriangleCount = 0);

    ~ShadowGeometry();
    
    // Render the shadow texture
    void Click(float fTime, NiCamera* pkSceneCamera,
        bool bVisible = true, bool bUpdateImage = true);

    void SetCaster(NiAVObject* pkCaster);

    void SetGroundSceneGraph(NiAVObject* pkGround);
    NiTriShape* GetShadowGeometry() const;
    void ForceRender(); // necessary if alt-tab detected

protected:
    ShadowGeometry(unsigned int uiMaxTriangleCount);

    void UpdateShadowCamera(float fTime);
    void UpdateShadowGeometry();
    void UpdateShadowGeometryBound();
    void TraverseGroundGeometry(NiAVObject* pkObject);
    void AddToShadowGeometry(NiTriBasedGeom* pkGeom);
    void AddShadowTriangle(NiPoint3 akV[3]);
    void RecursiveStoreCasterObjects(NiAVObject* pkObject);
    bool GenerateCameraRay(unsigned int uiIndex, NiPoint3& kPt, 
        NiPoint3& kDir);
    void CullTriAgainstCameraFrustum(NiPoint3& kV0, 
        NiPoint3& kV1, NiPoint3& kV2);
    void UpdateSampleCoeffsGaussBlur();

    // Initial max triangle and vertex counts allowed when generating the
    // set of geometry subtended by the shadown (i.e. shadow "recipient"
    // geometry).  This method is independent of "caster" geometry count, as
    // the system renders the shadow to a texture (or uses a loaded texture)
    enum { TRIANGLE_COUNT = 60, VERTEX_COUNT = 180 };

    // Leaf geometry objects in the shadow caster scene.  If the shadow is
    // dynamically rendered, we will render these as a flat list (a batch
    // rendering).  They are found by traversing the m_spCaster for leaf 
    // geometry
    NiTObjectSet<NiAVObjectPtr> m_kCastingObjects;

    // The tree containing all objects that will cast a shadow upon the scene
    // Used only in the case that the shadow is dynamically rendered per frame
    NiAVObjectPtr m_spCaster;

    // A very distant camera, designed to represent an infinite light source
    NiCameraPtr m_spCamera;

    // The shadow texture, which will either be:
    // A) Rendered each frame to represent the shadow geometry as seen from 
    // the casting light's POV
    // B) Loaded from file at initialization time and used each frame
    NiTexturePtr m_spTexture;
    NiTexturePtr m_spBlurredTexture;

    // The render target used by the system to render the shadow geometry
    NiRenderTargetGroupPtr m_spRenderTargetGroup;
    NiRenderTargetGroupPtr m_spBlurredRenderTargetGroup;

    // Root of the scene upon which shadow will be cast (the "recipient")
    NiAVObjectPtr m_spGround;

    // Current recipient geometry, created from triangles in the m_spGround
    // scene that intersect the cast shadow volume.  This is a dynamic object
    // and will be refilled each frame
    NiTriShapePtr m_spShadowGeometry;

    NiScreenElementsPtr m_spBlurGeom;

    // A property state that causes the shadow caster to be rendered as dark
    // gray into the white background of the shadow image.  Used when drawing
    // the shadow caster geometry into the shadow texture (dynamically
    // rendered shadow image only - unused if the shadow is loaded from file)
    NiPropertyStatePtr m_spPropertyState;

    NiPoint3 m_kLightDir;
    NiPoint3 m_kCameraCenter;
    float m_fOOCameraWidth;

    // Limits on the amount of ground (recipient) geometry that can be added
    // to m_spShadowGeometry, to avoid overflowing (or having to resize) the
    // geometry arrays
    unsigned int m_uiMaxTriangleCount;
    unsigned int m_uiMaxVertexCount;

    // Disables fast culling of the shadow casters w.r.t. the shadow camera
    // Useful to ensure that the texture is rendered the first time around
    // or when the application loses focus (via Alt-Tab)
    bool m_bForceRender;

    NiMaterialPtr m_spCasterMaterial;
    NiMaterialPtr m_spShadowMaterial;
    NiMaterialPtr m_spGaussBlurMaterial;

    NiVisibleArray m_kVisible;
    NiCullingProcess m_kCuller;
    NiFrustumPlanes m_kFrPlanes;
};

#include "ShadowGeometry.inl"

#endif // SHADOWGEOMETRY_H
