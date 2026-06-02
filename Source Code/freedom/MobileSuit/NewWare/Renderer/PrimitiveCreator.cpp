
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : PrimitiveCreator.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#include "stdafx.h"
#include "PrimitiveCreator.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

NiTriShape* PrimitiveCreator::CreateSphere( NiAVObject* pkObject, bool bAttachCommonProp )
{
    assert( pkObject );

    NiPoint3 kCenter;
    NiPoint3 kExtents;
    int aiAxisOrder[3];
    if ( GetCenterAndExtents(pkObject, kCenter, kExtents, aiAxisOrder) == false )
        return NULL;

    // set radius equal to largest axis
    float fRadius = kExtents[ aiAxisOrder[0] ];

    NiSphere kSphere;
    kSphere.m_kCenter = kCenter;
    kSphere.m_fRadius = fRadius;
    return CreateSphereMesh( kSphere, bAttachCommonProp );
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateBox( NiAVObject* pkObject, bool bAttachCommonProp )
{
    assert( pkObject );

    NiPoint3 kCenter;
    NiPoint3 kExtents;
    int aiAxisOrder[3];
    if ( GetCenterAndExtents(pkObject, kCenter, kExtents, aiAxisOrder) == false )
        return NULL;

    NiBox kBox;
    //kBox.CreateFromData( ((NiTriShape*)pkObject)->GetVertexCount(), ((NiTriShape*)pkObject)->GetVertices() );
    kBox.m_kCenter = kCenter;
    kBox.m_afExtent[0] = kExtents[0];
    kBox.m_afExtent[1] = kExtents[1];
    kBox.m_afExtent[2] = kExtents[2];
    kBox.m_akAxis[0] = NiPoint3::UNIT_X;
    kBox.m_akAxis[1] = NiPoint3::UNIT_Y;
    kBox.m_akAxis[2] = NiPoint3::UNIT_Z;
    return CreateBoxMesh( kBox, bAttachCommonProp );
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateCapsule( NiAVObject* pkObject, bool bAttachCommonProp )
{
    assert( pkObject );

    NiPoint3 kCenter;
    NiPoint3 kExtents;
    int aiAxisOrder[3];
    if ( GetCenterAndExtents(pkObject, kCenter, kExtents, aiAxisOrder) == false )
        return NULL;

    // radius is second longest axis, and extent along largest axis
    float fRadius = kExtents[aiAxisOrder[1]];
    float fExtent = kExtents[aiAxisOrder[0]];
    NiPoint3 kAxis = NiPoint3::ZERO;
    kAxis[aiAxisOrder[0]] = 1.0f;

    // calculate capsule
    NiCapsule kCapsule;
    kCapsule.m_fRadius = fRadius;
    kCapsule.m_kSegment.m_kOrigin    = kCenter - kAxis * (fExtent - kCapsule.m_fRadius);
    kCapsule.m_kSegment.m_kDirection = kAxis * (fExtent - kCapsule.m_fRadius) * 2.0f;
    return CreateCapsuleMesh( kCapsule, bAttachCommonProp );
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreatePlane( NiAVObject* pkObject, bool bAttachCommonProp )
{
    assert( pkObject );

    NiPoint3 kCenter;
    NiPoint3 kExtents;
    int aiAxisOrder[3];
    if ( GetCenterAndExtents(pkObject, kCenter, kExtents, aiAxisOrder) == false )
        return NULL;

    NiPoint3 kAxis = NiPoint3::ZERO;
    kAxis[aiAxisOrder[2]] = -1.0f;

    return CreatePlaneMesh( NiHalfSpaceBV(kCenter, kAxis), 
                            pkObject->GetWorldBound().GetRadius(), bAttachCommonProp );
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateOBB( NiAVObject* pkObject, bool bAttachCommonProp )
{
    assert( pkObject );

    return CreateOBBMesh( pkObject, bAttachCommonProp );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiTriShape* PrimitiveCreator::CreateSphereMesh( NiSphere const& kSphere, bool bAttachCommonProp, 
                                                NiColor const& kColor, float const fMult )
{
    // create a kSphere by one subdivision of an icosahedron
    float const fGold = 0.5f * (NiSqrt(5.0f) + 1.0f);

    NiPoint3 kCenter = kSphere.m_kCenter;

    NiPoint3 akIcoVerts[12] = { 
        NiPoint3(fGold,  1.0f,  0.0f), 
        NiPoint3(-fGold,  1.0f,  0.0f), 
        NiPoint3(fGold, -1.0f,  0.0f), 
        NiPoint3(-fGold, -1.0f,  0.0f), 
        NiPoint3(1.0f,  0.0f,  fGold), 
        NiPoint3(1.0f,  0.0f, -fGold), 
        NiPoint3(-1.0f,  0.0f,  fGold), 
        NiPoint3(-1.0f,  0.0f, -fGold), 
        NiPoint3(0.0f,  fGold,  1.0f), 
        NiPoint3(0.0f, -fGold,  1.0f), 
        NiPoint3(0.0f,  fGold, -1.0f), 
        NiPoint3(0.0f, -fGold, -1.0f)
    };
    unsigned short const ausIcoPolys[] = { 
        0, 8, 4, 
        0, 5, 10, 
        2, 4, 9, 
        2, 11, 5, 
        1, 6, 8, 
        1, 10, 7, 
        3, 9, 6, 
        3, 7, 11, 
        0, 10, 8, 
        1, 8, 10, 
        2, 9, 11, 
        3, 11, 9, 
        4, 2, 0, 
        5, 0, 2, 
        6, 1, 3, 
        7, 3, 1, 
        8, 6, 4, 
        9, 4, 6, 
        10, 5, 7, 
        11, 7, 5
    };

    unsigned int uiNumVerts = 12;
    unsigned int uiNumTris = 20;
    unsigned int uiTotalVerts = 72;
    unsigned int uiTotalTris = 80;

    // Create an array to store all of the points and two arrays (one for
    // scratch space) to store the triangle connectivity.
    NiPoint3* pkPoints = NiNew NiPoint3[uiTotalVerts];
    unsigned int uiTotalTris_X_3 = uiTotalTris * 3;
    unsigned short* pusConn = NiAlloc( unsigned short, uiTotalTris_X_3 );
    unsigned short* pusConnTmp = NiAlloc( unsigned short, uiTotalTris_X_3 );
    unsigned int uiDestSize = uiTotalTris_X_3 * sizeof(*pusConn);

    // starting with an icosahedron, normalize the vertices
    for ( unsigned int i = 0; i < uiNumVerts; ++i )
    {
        akIcoVerts[i].Unitize();
        pkPoints[i] = akIcoVerts[i];
    }

    // start with the icosahedron connectivity, subdivide once
    NiMemcpy( pusConn, uiDestSize, ausIcoPolys, uiDestSize );

    // store traversal pointers for the connectivity arrays
    unsigned short* pusSrc = pusConn;
    unsigned short* pusDest = pusConnTmp;

    for ( unsigned int t = 0; t < uiNumTris; ++t )
    {
        // get the indices to the triangle's vertices
        unsigned short v1 = *pusSrc++, v2 = *pusSrc++, v3 = *pusSrc++;

        // compute the three new verts as triangle edge midpoints and
        // normalize the points back onto the kSphere
        pkPoints[uiNumVerts] = pkPoints[v1] + pkPoints[v2];
        pkPoints[uiNumVerts].Unitize();
        pkPoints[uiNumVerts + 1] = pkPoints[v2] + pkPoints[v3];
        pkPoints[uiNumVerts + 1].Unitize();
        pkPoints[uiNumVerts + 2] = pkPoints[v3] + pkPoints[v1];
        pkPoints[uiNumVerts + 2].Unitize();

        // create 4 new triangles to retessellate the old triangle
        *pusDest++ = v1;
        *pusDest++ = uiNumVerts;
        *pusDest++ = uiNumVerts + 2;

        *pusDest++ = v2;
        *pusDest++ = uiNumVerts + 1;
        *pusDest++ = uiNumVerts;

        *pusDest++ = v3;
        *pusDest++ = uiNumVerts + 2;
        *pusDest++ = uiNumVerts + 1;

        *pusDest++ = uiNumVerts;
        *pusDest++ = uiNumVerts + 1;
        *pusDest++ = uiNumVerts + 2;

        // update the number of vertices
        uiNumVerts += 3;
    }

    // swap the two temporary connectivity arrays
    unsigned short* pusTmp = pusConn;
    pusConn = pusConnTmp;
    pusConnTmp = pusTmp;

    // update the number of tris
    uiNumTris *= 4;

    // scale the vertices and translate
    NiPoint3* pkTmpPoints = pkPoints;
    for ( unsigned int i = 0; i < uiNumVerts; ++i )
    {
        *pkTmpPoints = (*pkTmpPoints) * kSphere.m_fRadius * fMult + kCenter;
        ++pkTmpPoints;
    }

    // free the setup arrays
    NiFree( pusConnTmp );

    NiTriShape* pkTriShape = 
        NiNew NiTriShape( uiNumVerts, pkPoints, 0, 0, 0, 0, NiGeometryData::NBT_METHOD_NONE, uiNumTris, pusConn );
    if ( bAttachCommonProp )
        AttachCommonProperty( pkTriShape, kColor );
    return pkTriShape;
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateBoxMesh( NiBox const& kBox, bool bAttachCommonProp, 
                                             NiColor const& kColor, float const fMult )
{
    float const afExtent[3] = { 
        kBox.m_afExtent[0] * fMult, 
        kBox.m_afExtent[1] * fMult, 
        kBox.m_afExtent[2] * fMult
    };

    unsigned int const uiNumVerts = 8;
    unsigned int const uiNumTris = 12;

    NiPoint3* pkPoints = NiNew NiPoint3[uiNumVerts];
    pkPoints[0] = kBox.m_kCenter + kBox.m_akAxis[0] * afExtent[0] - kBox.m_akAxis[1] * afExtent[1] - kBox.m_akAxis[2] * afExtent[2];
    pkPoints[1] = kBox.m_kCenter + kBox.m_akAxis[0] * afExtent[0] + kBox.m_akAxis[1] * afExtent[1] - kBox.m_akAxis[2] * afExtent[2];
    pkPoints[2] = kBox.m_kCenter - kBox.m_akAxis[0] * afExtent[0] + kBox.m_akAxis[1] * afExtent[1] - kBox.m_akAxis[2] * afExtent[2];
    pkPoints[3] = kBox.m_kCenter - kBox.m_akAxis[0] * afExtent[0] - kBox.m_akAxis[1] * afExtent[1] - kBox.m_akAxis[2] * afExtent[2];
    pkPoints[4] = kBox.m_kCenter + kBox.m_akAxis[0] * afExtent[0] - kBox.m_akAxis[1] * afExtent[1] + kBox.m_akAxis[2] * afExtent[2];
    pkPoints[5] = kBox.m_kCenter + kBox.m_akAxis[0] * afExtent[0] + kBox.m_akAxis[1] * afExtent[1] + kBox.m_akAxis[2] * afExtent[2];
    pkPoints[6] = kBox.m_kCenter - kBox.m_akAxis[0] * afExtent[0] + kBox.m_akAxis[1] * afExtent[1] + kBox.m_akAxis[2] * afExtent[2];
    pkPoints[7] = kBox.m_kCenter - kBox.m_akAxis[0] * afExtent[0] - kBox.m_akAxis[1] * afExtent[1] + kBox.m_akAxis[2] * afExtent[2];

    unsigned short* pusConn = NiAlloc( unsigned short, 3 * uiNumTris );
    pusConn[0]  = 3;  pusConn[1]  = 2;  pusConn[2]  = 1;
    pusConn[3]  = 3;  pusConn[4]  = 1;  pusConn[5]  = 0;
    pusConn[6]  = 0;  pusConn[7]  = 1;  pusConn[8]  = 5;
    pusConn[9]  = 0;  pusConn[10] = 5;  pusConn[11] = 4;
    pusConn[12] = 4;  pusConn[13] = 5;  pusConn[14] = 6;
    pusConn[15] = 4;  pusConn[16] = 6;  pusConn[17] = 7;
    pusConn[18] = 7;  pusConn[19] = 6;  pusConn[20] = 2;
    pusConn[21] = 7;  pusConn[22] = 2;  pusConn[23] = 3;
    pusConn[24] = 1;  pusConn[25] = 2;  pusConn[26] = 6;
    pusConn[27] = 1;  pusConn[28] = 6;  pusConn[29] = 5;
    pusConn[30] = 0;  pusConn[31] = 4;  pusConn[32] = 7;
    pusConn[33] = 0;  pusConn[34] = 7;  pusConn[35] = 3;

    NiTriShape* pkTriShape = 
        NiNew NiTriShape( uiNumVerts, pkPoints, 0, 0, 0, 0, NiGeometryData::NBT_METHOD_NONE, uiNumTris, pusConn );
    if ( bAttachCommonProp )
        AttachCommonProperty( pkTriShape, kColor );
    return pkTriShape;
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateCapsuleMesh( NiCapsule const& kCapsule, bool bAttachCommonProp, 
                                                 NiColor const& kColor, float const fMult )
{
    float const fRadius = kCapsule.m_fRadius * fMult;
    NiPoint3 kDirection = kCapsule.m_kSegment.m_kDirection;

    if ( kDirection == NiPoint3::ZERO )
    {
        NiSphere kSphere;
        kSphere.m_kCenter = kCapsule.m_kSegment.m_kOrigin;
        kSphere.m_fRadius = fRadius;
        return CreateSphereMesh( kSphere, bAttachCommonProp, kColor, fMult );
    }

    // compute unit-length kCapsule kAxis
    NiPoint3 kAxis = kDirection;
    kAxis.Unitize();

    // compute end points of kCapsule kAxis
    NiPoint3 kEndPt0 = kCapsule.m_kSegment.m_kOrigin - (kDirection + fRadius * kAxis) * (fMult - 1.0f);
    NiPoint3 kEndPt1 = kCapsule.m_kSegment.m_kOrigin + kCapsule.m_kSegment.m_kDirection * fMult;

    // create two basis vectors that are perpendicular to the kCapsule kAxis
    NiPoint3 kBasis1;
    if ( NiAbs(kAxis.x) >= NiAbs(kAxis.y) && NiAbs(kAxis.x) >= NiAbs(kAxis.z) )
    {
        kBasis1.x = -kAxis.y;
        kBasis1.y = kAxis.x;
        kBasis1.z = kAxis.z;
    }
    else
    {
        kBasis1.x = kAxis.x;
        kBasis1.y = kAxis.z;
        kBasis1.z = -kAxis.y;
    }

    kBasis1.Unitize();
    NiPoint3 kBasis0 = kBasis1.Cross( kAxis );
    kBasis0 *= fRadius;
    kBasis1 *= fRadius;

    // cross section of kCapsule is 16-sided regular polygon
    unsigned int const uiNumSides = 16;
    unsigned int uiNumVerts = 4 * uiNumSides + 2;
    unsigned int uiNumTris = 8 * uiNumSides;
    NiPoint3* pkPoints = NiNew NiPoint3[uiNumVerts];
    unsigned short* pusConn = NiAlloc( unsigned short, uiNumTris * 3 );

    pkPoints[uiNumVerts-2] = kEndPt0 - kAxis * fRadius;
    pkPoints[uiNumVerts-1] = kEndPt1 + kAxis * fRadius;

    float fCapScale = NiSqrt( 0.5f );
    NiPoint3 kCap0 = kEndPt0 - kAxis * (fRadius * fCapScale);
    NiPoint3 kCap1 = kEndPt1 + kAxis * (fRadius * fCapScale);

    float fIncr = 2.0f * NI_PI / (float)uiNumSides;
    float fTheta = 0.0f;
    for ( unsigned int i = 0; i < uiNumSides; ++i )
    {
        NiPoint3 kOffset = kBasis0 * NiCos( fTheta ) + kBasis1 * NiSin( fTheta );

        pkPoints[i] = kCap0 + kOffset * fCapScale;
        pkPoints[i + uiNumSides] = kEndPt0 + kOffset;
        pkPoints[i + uiNumSides * 2] = kEndPt1 + kOffset;
        pkPoints[i + uiNumSides * 3] = kCap1 + kOffset * fCapScale;

        fTheta += fIncr;
    }

    unsigned short* pusConnTmp = pusConn;
    for ( unsigned int i = 0; i < uiNumSides-1; ++i )
    {
        // side
        for ( unsigned int j = 0; j < 3; ++j )
        {
            *pusConnTmp++ = i + uiNumSides * j;
            *pusConnTmp++ = i + 1 + uiNumSides * j;
            *pusConnTmp++ = i + 1 + uiNumSides * (j + 1);

            *pusConnTmp++ = i + uiNumSides * j;
            *pusConnTmp++ = i + 1 + uiNumSides * (j + 1);
            *pusConnTmp++ = i + uiNumSides * (j + 1);
        }

        // end caps
        *pusConnTmp++ = i + 1;
        *pusConnTmp++ = i;
        *pusConnTmp++ = uiNumVerts - 2;

        *pusConnTmp++ = i + uiNumSides * 3;
        *pusConnTmp++ = i + 1 + uiNumSides * 3;
        *pusConnTmp++ = uiNumVerts - 1;
    }

    for ( unsigned int j = 0; j < 3; ++j )
    {
        *pusConnTmp++ = uiNumSides * (j + 1) - 1;
        *pusConnTmp++ = uiNumSides * j;
        *pusConnTmp++ = uiNumSides * (j + 1);

        *pusConnTmp++ = uiNumSides * (j + 1) - 1;
        *pusConnTmp++ = uiNumSides * (j + 1);
        *pusConnTmp++ = uiNumSides * (j + 2) - 1;
    }

    *pusConnTmp++ = 0;
    *pusConnTmp++ = uiNumSides - 1;
    *pusConnTmp++ = uiNumVerts - 2;

    *pusConnTmp++ = uiNumSides * 4 - 1;
    *pusConnTmp++ = uiNumSides * 3;
    *pusConnTmp++ = uiNumVerts - 1;

    NiTriShape* pkTriShape = 
        NiNew NiTriShape( uiNumVerts, pkPoints, 0, 0, 0, 0, NiGeometryData::NBT_METHOD_NONE, uiNumTris, pusConn );
    if ( bAttachCommonProp )
        AttachCommonProperty( pkTriShape, kColor );
    return pkTriShape;
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreatePlaneMesh( NiHalfSpaceBV const& kHalfSpaceBV, float const fRadius, 
                                               bool bAttachCommonProp, NiColor const& kColor, float const fMult )
{
    NiPoint3 kAxis   = kHalfSpaceBV.GetPlane().GetNormal();
    NiPoint3 kCenter = kHalfSpaceBV.GetCenter();

    // create two basis vectors that are perpendicular to the normal
    NiPoint3 kBasis1;
    if ( NiAbs(kAxis.x) >= NiAbs(kAxis.y) && NiAbs(kAxis.x) >= NiAbs(kAxis.z) )
    {
        kBasis1.x = -kAxis.y;
        kBasis1.y = kAxis.x;
        kBasis1.z = kAxis.z;
    }
    else
    {
        kBasis1.x = kAxis.x;
        kBasis1.y = kAxis.z;
        kBasis1.z = -kAxis.y;
    }

    kBasis1.Unitize();
    NiPoint3 kBasis0 = kBasis1.Cross( kAxis );

    // use fMult to determine the size of the plane representation
    kBasis0 *= fRadius;
    kBasis1 *= fRadius;
    NiPoint3 kNorm0 = kBasis0 * 0.05f;
    NiPoint3 kNorm1 = kBasis1 * 0.05f;
    NiPoint3 kNorm2 = kAxis * fRadius * 0.5f;
    NiPoint3 kOffset = kNorm2 * (1.0f - fMult);

    unsigned int const uiNumVerts = 12;
    unsigned int const uiNumTris = 6;

    NiPoint3* pkPoints = NiNew NiPoint3[uiNumVerts];

    // plane
    pkPoints[0] = kCenter + kBasis0 + kBasis1 + kOffset;
    pkPoints[1] = kCenter + kBasis0 - kBasis1 + kOffset;
    pkPoints[2] = kCenter - kBasis0 - kBasis1 + kOffset;
    pkPoints[3] = kCenter - kBasis0 + kBasis1 + kOffset;
    // normal base
    pkPoints[4] = kCenter + kNorm0 + kNorm1 + kOffset;
    pkPoints[5] = kCenter + kNorm0 - kNorm1 + kOffset; 
    pkPoints[6] = kCenter - kNorm0 - kNorm1 + kOffset; 
    pkPoints[7] = kCenter - kNorm0 + kNorm1 + kOffset;
    // normal tip
    pkPoints[8] = kCenter + kNorm0 + kNorm1 + kNorm2;
    pkPoints[9] = kCenter + kNorm0 - kNorm1 + kNorm2; 
    pkPoints[10] = kCenter - kNorm0 - kNorm1 + kNorm2; 
    pkPoints[11] = kCenter - kNorm0 + kNorm1 + kNorm2; 

    unsigned short* pusConn = NiAlloc( unsigned short, 3 * uiNumTris );
    pusConn[0]  = 0;  pusConn[1]  = 3;  pusConn[2]  = 1;
    pusConn[3]  = 3;  pusConn[4]  = 2;  pusConn[5]  = 1;
    pusConn[6]  = 8;  pusConn[7]  = 10; pusConn[8]  = 4;
    pusConn[9]  = 10; pusConn[10] = 6;  pusConn[11] = 4;
    pusConn[12] = 9;  pusConn[13] = 11; pusConn[14] = 5;
    pusConn[15] = 11; pusConn[16] = 7;  pusConn[17] = 5;

    NiTriShape* pkTriShape = 
        NiNew NiTriShape( uiNumVerts, pkPoints, 0, 0, 0, 0, NiGeometryData::NBT_METHOD_NONE, uiNumTris, pusConn );
    if ( bAttachCommonProp )
        AttachCommonProperty( pkTriShape, kColor );
    return pkTriShape;
}

//-----------------------------------------------------------------------------------

NiTriShape* PrimitiveCreator::CreateOBBMesh( NiAVObject* pkObject, bool bAttachCommonProp, 
                                             NiColor const& kColor, float const fMult )
{
    assert( pkObject );

    NiTriShape* pkGeom = NiDynamicCast(NiTriShape, pkObject);
    if ( pkGeom )
    {
        NiBox kBox;
        kBox.CreateFromData( pkGeom->GetVertexCount(), pkGeom->GetVertices() );
        return CreateBoxMesh( kBox, bAttachCommonProp, kColor, fMult );
    }
    return NULL;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void PrimitiveCreator::AttachCommonProperty( NiAVObject* pkObject, NiColor const& kColor )
{
    assert( pkObject );

    NiWireframeProperty* pkWire = NiNew NiWireframeProperty;
    pkWire->SetWireframe( true );
    pkObject->AttachProperty( pkWire );

    NiVertexColorProperty* pkVC = NiNew NiVertexColorProperty;
    pkVC->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
    pkVC->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
    pkObject->AttachProperty( pkVC );

    NiMaterialProperty* pkMat = NiNew NiMaterialProperty;
    pkMat->SetEmittance( kColor );
    pkObject->AttachProperty( pkMat );
}

//-----------------------------------------------------------------------------------

bool PrimitiveCreator::GetCenterAndExtents( NiAVObject* pkObject, NiPoint3& kCenter, 
                                            NiPoint3& kExtents, int aiAxisOrder[3] )
{
    NiTriBasedGeom* pkTriGeom = NiDynamicCast(NiTriBasedGeom, pkObject);
    if ( NULL == pkTriGeom )
    {
        kExtents = NiPoint3::ZERO;
        aiAxisOrder[0] = 0;
        aiAxisOrder[1] = 1;
        aiAxisOrder[2] = 2;
        return false;
    }

    unsigned int const uiNumVerts = pkTriGeom->GetVertexCount();
    assert( uiNumVerts );

    NiPoint3 const* pkVerts = pkTriGeom->GetVertices();
    NiPoint3 kMin = pkVerts[0];
    NiPoint3 kMax = pkVerts[0];

    for ( unsigned int ui = 1; ui < uiNumVerts; ++ui )
    {
        if ( pkVerts[ui].x < kMin.x )
            kMin.x = pkVerts[ui].x;
        if ( pkVerts[ui].y < kMin.y )
            kMin.y = pkVerts[ui].y;
        if ( pkVerts[ui].z < kMin.z )
            kMin.z = pkVerts[ui].z;

        if ( pkVerts[ui].x > kMax.x )
            kMax.x = pkVerts[ui].x;
        if ( pkVerts[ui].y > kMax.y )
            kMax.y = pkVerts[ui].y;
        if ( pkVerts[ui].z > kMax.z )
            kMax.z = pkVerts[ui].z;
    }

    kCenter = (kMin + kMax) / 2.0f;
    kExtents = kMax - kCenter;
    if ( kExtents[0] > kExtents[1] )
    {
        if ( kExtents[0] > kExtents[2] )
        {
            aiAxisOrder[0] = 0;
            if ( kExtents[1] > kExtents[2] )
            {
                aiAxisOrder[1] = 1;
                aiAxisOrder[2] = 2;
            }
            else
            {
                aiAxisOrder[1] = 2;
                aiAxisOrder[2] = 1;
            }
        }
        else
        {
            aiAxisOrder[0] = 2;
            aiAxisOrder[1] = 0;
            aiAxisOrder[2] = 1;
        }
    }
    else 
    {
        if ( kExtents[1] > kExtents[2] )
        {
            aiAxisOrder[0] = 1;
            if ( kExtents[0] > kExtents[2] )
            {
                aiAxisOrder[1] = 0;
                aiAxisOrder[2] = 2;
            }
            else
            {
                aiAxisOrder[1] = 2;
                aiAxisOrder[2] = 0;
            }
        }
        else 
        {
            aiAxisOrder[0] = 2;
            aiAxisOrder[1] = 1;
            aiAxisOrder[2] = 0;
        }
    }
    return true;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
