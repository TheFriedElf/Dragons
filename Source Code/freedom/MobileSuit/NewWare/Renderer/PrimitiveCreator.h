
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : PrimitiveCreator.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_PRIMITIVECREATOR_H__
#define _RENDERER_PRIMITIVECREATOR_H__


namespace NewWare
{

namespace Renderer
{


class PrimitiveCreator
{
public:
    static NiTriShape* CreateSphere( NiAVObject* pkObject, bool bAttachCommonProp = false );
    static NiTriShape* CreateBox( NiAVObject* pkObject, bool bAttachCommonProp = false );
    static NiTriShape* CreateCapsule( NiAVObject* pkObject, bool bAttachCommonProp = false );
    static NiTriShape* CreatePlane( NiAVObject* pkObject, bool bAttachCommonProp = false );
    static NiTriShape* CreateOBB( NiAVObject* pkObject, bool bAttachCommonProp = false );

    static NiTriShape* CreateSphereMesh( NiSphere const& kSphere, bool bAttachCommonProp = false, 
                    NiColor const& kColor = NiColor(0.0f, 0.0f, 1.0f), float const fMult = 1.0f );
    static NiTriShape* CreateBoxMesh( NiBox const& kBox, bool bAttachCommonProp = false, 
                    NiColor const& kColor = NiColor(0.0f, 0.0f, 1.0f), float const fMult = 1.0f );
    static NiTriShape* CreateCapsuleMesh( NiCapsule const& kCapsule, bool bAttachCommonProp = false, 
                    NiColor const& kColor = NiColor(0.0f, 0.0f, 1.0f), float const fMult = 1.0f );
    // @note - const float fRadius = pkObject->GetWorldBound().GetRadius();
    static NiTriShape* CreatePlaneMesh( NiHalfSpaceBV const& kHalfSpaceBV, float const fRadius, 
                    bool bAttachCommonProp = false, 
                    NiColor const& kColor = NiColor(0.0f, 0.0f, 1.0f), float const fMult = 1.0f );
    static NiTriShape* CreateOBBMesh( NiAVObject* pkObject, bool bAttachCommonProp = false, 
                    NiColor const& kColor = NiColor(0.0f, 0.0f, 1.0f), float const fMult = 1.0f );


private:
    static void AttachCommonProperty( NiAVObject* pkObject, NiColor const& kColor );
    static bool GetCenterAndExtents( NiAVObject* pkObject, NiPoint3& kCenter, NiPoint3& kExtents, int aiAxisOrder[3] );
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_PRIMITIVECREATOR_H__
