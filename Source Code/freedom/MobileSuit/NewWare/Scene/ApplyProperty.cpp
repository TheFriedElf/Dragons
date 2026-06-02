
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : ApplyProperty.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#include "stdafx.h"
#include "ApplyProperty.h"

#include "ApplyTraversal.h"


namespace NewWare
{

namespace Scene
{


namespace ApplyProperty
{


/////////////////////////////////////////////////////////////////////////////////////////////
// Attach namespace
namespace Attatch
{

NiFogProperty* CreateFogProperty( NiAVObject* pkAVObject, bool bEnable, NiFogProperty::FogFunction eFunc, 
                                  float fStart, float fEnd, float fDensity, bool bUpdateProperties )
{
    assert( pkAVObject );

    NiFogProperty* pkFogProperty = NiNew NiFogProperty;
    pkFogProperty->SetFog( bEnable );
    pkFogProperty->SetFogFunction( eFunc );
    pkFogProperty->SetFogStart( fStart );
    pkFogProperty->SetFogEnd( fEnd );
    pkFogProperty->SetFogDensity( fDensity );

    pkAVObject->AttachProperty( pkFogProperty );
    if ( bUpdateProperties )
        pkAVObject->UpdateProperties();

    return pkFogProperty;
}

//-----------------------------------------------------------------------------------

void SwitchFogProperty( bool bAttach, NiAVObject* pkAVObject, bool bEnable, 
                        NiFogProperty* pkSrcProp, bool bUpdateProperties )
{
    assert( pkAVObject );

    if( bAttach )
    {
        assert( pkSrcProp );
        Override::EnableFogProperty( pkAVObject, bEnable, pkSrcProp, bUpdateProperties );
    }
    else
    {
        NiFogProperty* pkProp = 
                    static_cast<NiFogProperty*>(pkAVObject->GetProperty(NiFogProperty::GetType()));
        if ( pkProp )
            pkAVObject->DetachProperty( pkProp );
    }
}

} //namespace Attatch
// Attach namespace
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Override namespace
namespace Override
{

void EnableFogProperty( NiAVObject* pkAVObject, bool bEnable, 
                        NiFogProperty* pkSrcProp, bool bUpdateProperties )
{
    assert( pkAVObject );

    NiFogProperty* pkProp = 
                    static_cast<NiFogProperty*>(pkAVObject->GetProperty(NiFogProperty::GetType()));
    if ( NULL == pkProp )
    {
        assert( pkSrcProp );
        pkProp = static_cast<NiFogProperty*>(pkSrcProp->Clone());
        assert( pkProp );

        pkAVObject->AttachProperty( pkProp );
        if ( bUpdateProperties )
            pkAVObject->UpdateProperties();
    }

    pkProp->SetFog( bEnable );
}

//-----------------------------------------------------------------------------------

void EnableZBufferProperty( NiAVObject* pkAVObject, bool bTest, bool bWrite, bool bUpdateProperties )
{
    assert( pkAVObject );

    NiZBufferProperty* pkProp = 
        static_cast<NiZBufferProperty*>(pkAVObject->GetProperty(NiZBufferProperty::GetType()));
    if ( NULL == pkProp )
    {
        pkProp = NiNew NiZBufferProperty;
        assert( pkProp );

        pkAVObject->AttachProperty( pkProp );
        if ( bUpdateProperties )
            pkAVObject->UpdateProperties();
    }

    pkProp->SetZBufferTest( bTest );
    pkProp->SetZBufferWrite( bWrite );
}

//-----------------------------------------------------------------------------------

bool RepairFalseAlphaGeometry( NiGeometry* pkGeometry )
{
    assert( pkGeometry );

    NiPropertyState* pkPropState = pkGeometry->GetPropertyState();
    assert( pkPropState );

    NiAlphaProperty const* pkAlphaProp = pkPropState->GetAlpha();
    assert( pkAlphaProp );
    if ( pkAlphaProp && pkAlphaProp->GetAlphaBlending() )
    {
        NiTexturingProperty const* pkTexProp = pkPropState->GetTexturing();
        if ( NULL == pkTexProp )
        {
            //pkPropState->SetProperty( NiAlphaProperty::GetDefault() );
            return true;
        }
        NiTexture const* pkTex = pkTexProp->GetBaseTexture();
        if ( NULL == pkTex )
        {
            //pkPropState->SetProperty( NiAlphaProperty::GetDefault() );
            return true;
        }

        if ( NiPixelFormat::FORMAT_RGB == pkTex->GetPixelFormat()->GetFormat() )
        {
            if ( pkAlphaProp->GetSrcBlendMode()  == NiAlphaProperty::ALPHA_SRCALPHA && 
                 pkAlphaProp->GetDestBlendMode() == NiAlphaProperty::ALPHA_INVSRCALPHA )
            {
#ifndef EXTERNAL_RELEASE
                //if ( NiIsKindOf(NiSourceTexture, pkTex) )
                //{
                //    Tools::OutputDebugFile( "RGB - %s\n", ((NiSourceTexture*)pkTex)->GetFilename() );
                //}
#endif //#ifndef EXTERNAL_RELEASE

                pkPropState->SetProperty( NiAlphaProperty::GetDefault() );
                return true;
            }
        }
        else if ( NiPixelFormat::FORMAT_DXT1 == pkTex->GetPixelFormat()->GetFormat() )
        {
            if ( pkAlphaProp->GetSrcBlendMode()  == NiAlphaProperty::ALPHA_SRCALPHA && 
                 pkAlphaProp->GetDestBlendMode() == NiAlphaProperty::ALPHA_INVSRCALPHA )
            {
#ifndef EXTERNAL_RELEASE
                //if ( NiIsKindOf(NiSourceTexture, pkTex) )
                //{
                //    Tools::OutputDebugFile( "DXT1 - %s, mode:%d, ref:%d\n", 
                //                            ((NiSourceTexture*)pkTex)->GetFilename(), 
                //                            pkAlphaProp->GetTestMode(), pkAlphaProp->GetTestRef() );
                //}
#endif //#ifndef EXTERNAL_RELEASE

                NiAlphaProperty* pkNewAlphaProp = NiNew NiAlphaProperty;

                pkNewAlphaProp->SetAlphaBlending( false );
                pkNewAlphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_SRCALPHA );
                pkNewAlphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_INVSRCALPHA );

                pkNewAlphaProp->SetAlphaTesting( true );
                pkNewAlphaProp->SetTestMode( pkAlphaProp->GetTestMode() );
                pkNewAlphaProp->SetTestRef( pkAlphaProp->GetTestRef() );

                pkPropState->SetProperty( pkNewAlphaProp );
                return true;
            }
        }
    }
    return false;
}

} //namespace Override
// Override namespace
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiPropertyStatePtr CopyPropertyState( NiPropertyState* pkSource )
{
    if ( NULL == pkSource )
    {
        assert( pkSource );
        return 0;
    }

    NiPropertyStatePtr spPropertyState = NiNew NiPropertyState();
    for ( unsigned int ui = 0; ui < NiProperty::MAX_TYPES; ++ui )
    {
        NiProperty* pkProperty = (*pkSource)[ ui ];
        if ( pkProperty )
        {
            spPropertyState->SetProperty( static_cast<NiProperty*>(pkProperty->Clone()) );
        }
    }
    return spPropertyState;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace ApplyProperty


} //namespace Scene

} //namespace NewWare
