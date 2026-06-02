
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : ApplyProperty.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#ifndef _SCENE_APPLYPROPERTY_H__
#define _SCENE_APPLYPROPERTY_H__


namespace NewWare
{

namespace Scene
{


namespace ApplyProperty
{

    namespace Attatch
    {
        NiFogProperty* CreateFogProperty( NiAVObject* pkAVObject, bool bEnable, 
                                          NiFogProperty::FogFunction eFunc, float fStart, float fEnd, 
                                          float fDensity, bool bUpdateProperties = false );
        void SwitchFogProperty( bool bAttach, NiAVObject* pkAVObject, bool bEnable, 
                                NiFogProperty* pkSrcProp, bool bUpdateProperties = false );
    } //namespace Attatch


    namespace Override
    {
        void EnableFogProperty( NiAVObject* pkAVObject, bool bEnable, 
                                NiFogProperty* pkSrcProp, bool bUpdateProperties = false );

        void EnableZBufferProperty( NiAVObject* pkAVObject, bool bTest, bool bWrite, bool bUpdateProperties = false );


        bool RepairFalseAlphaGeometry( NiGeometry* pkGeometry );
    } //namespace Override


    NiPropertyStatePtr CopyPropertyState( NiPropertyState* pkSource );

} //namespace ApplyProperty


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_APPLYPROPERTY_H__
