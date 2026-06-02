
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionTimeController.cpp
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
#include "OcclusionTimeController.h"

#include "../../PrimitiveCreator.h"

#include <NiParticleSystem.h>
#include <NiIntegerExtraData.h>


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

NiImplementRTTI(OcclusionTimeController, NiTimeController);

//-----------------------------------------------------------------------------------

const float OcclusionTimeController::OCCLUDER_LIMIT_SIZE = 10.0f; // 25.4cm
char const* const OcclusionTimeController::OCCLUDER_NAME = "Occluder";

//-----------------------------------------------------------------------------------

OcclusionTimeController::OcclusionTimeController( bool bIsOccluder, NiSingleShaderMaterial* pkDepthMaterial )
    : m_bIsOccluder(bIsOccluder), 
      m_bMutable(true), 
      m_bIsSkinned(false), 
      m_pkDepthMaterial(pkDepthMaterial)
{
    m_kTransform.MakeIdentity();

    assert( m_pkDepthMaterial );
}

//-----------------------------------------------------------------------------------

OcclusionTimeController::~OcclusionTimeController()
{
    if ( m_spProxy )
        m_spProxy->RemoveMaterial( m_pkDepthMaterial );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionTimeController::Update( float fTime )
{
    assert( m_spObject );

    if ( m_bIsSkinned )
        UpdateSkinnedTransform();
    else
        UpdateTransform();
}

//-----------------------------------------------------------------------------------

void OcclusionTimeController::SetTarget( NiObjectNET* pkTarget )
{
    NiTimeController::SetTarget( pkTarget );

    if ( pkTarget )
        CreateProxy();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionTimeController::ResolveOcclusionType( NiCamera const* pkCamera )
{
    assert( pkCamera );

    if ( IsMutable() == false )
        return;

    NiGeometry* pkTarget = static_cast<NiGeometry*>(GetTarget());
    assert( pkTarget );

    NiIntegerExtraData* pkExtraData = static_cast<NiIntegerExtraData*>(pkTarget->GetExtraData(OCCLUDER_NAME));
    assert( pkExtraData );

    // Every property state should have a valid alpha property
    NiAlphaProperty const* pkAlpha = pkTarget->GetPropertyState()->GetAlpha();
    assert( pkAlpha );
    if ( pkAlpha->GetAlphaBlending() )
    {
        pkExtraData->SetValue( OcclusionFlag::OCCLUDEE );
        m_bIsOccluder = false;
        return;
    }

    int iExtraFlag;
    if ( m_bIsOccluder )
        iExtraFlag = OcclusionFlag::OCCLUDER | OcclusionFlag::OCCLUDEE;
    else
        iExtraFlag = OcclusionFlag::OCCLUDEE;
    pkExtraData->SetValue( iExtraFlag );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool OcclusionTimeController::CreateProxy()
{
    NiGeometry* pkTarget = static_cast<NiGeometry*>(GetTarget());
    assert( pkTarget );

    if ( pkTarget->GetSkinInstance() || NiIsKindOf(NiParticleSystem, pkTarget) )
    {
        NiSphere kSphere;
        kSphere.m_kCenter = NiPoint3::ZERO;
        kSphere.m_fRadius = 1.0f;
        m_spProxy = PrimitiveCreator::CreateSphereMesh( kSphere, false );
        m_bIsSkinned = true;
    }
    else
    {
        m_spProxy = PrimitiveCreator::CreateBox( pkTarget, false );
        m_bIsSkinned = false;
    }
    assert( m_spProxy );
    if ( 0 == m_spProxy )
        return false;

    m_spProxy->Update( 0.0f );
    m_spProxy->UpdateProperties();
    m_spProxy->UpdateEffects();

    int iExtraFlag;
    if ( false == m_bIsOccluder || LessEqualThresholdSize() )
    {
        m_bMutable = false;
        m_bIsOccluder = false;
        iExtraFlag = OcclusionFlag::OCCLUDEE;
    }
    else
    {
        m_bMutable = true;
        iExtraFlag = OcclusionFlag::OCCLUDER | OcclusionFlag::OCCLUDEE;
    }

    pkTarget->AddExtraData( OCCLUDER_NAME, NiNew NiIntegerExtraData(iExtraFlag) );

    Update( 0.0f );

    m_spProxy->ApplyAndSetActiveMaterial( m_pkDepthMaterial );
    return true;
}

//-----------------------------------------------------------------------------------

void OcclusionTimeController::UpdateTransform()
{
    if ( 0 == m_spProxy )
        return;

    NiAVObject* pkTarget = static_cast<NiAVObject*>(GetTarget());
    assert( pkTarget );

    if ( pkTarget->GetWorldTransform() != m_kTransform )
    {
        m_kTransform = pkTarget->GetWorldTransform();

        m_spProxy->SetLocalTransform( m_kTransform );
        m_spProxy->Update( 0.0f );
    }
}

//-----------------------------------------------------------------------------------

void OcclusionTimeController::UpdateSkinnedTransform()
{
    if ( 0 == m_spProxy )
        return;

    NiGeometry* pkTarget = static_cast<NiGeometry*>(GetTarget());
    assert( pkTarget && pkTarget->GetSkinInstance() || NiIsKindOf(NiParticleSystem, pkTarget) );

    NiBound const& kWorldBound = pkTarget->GetWorldBound();
    NiPoint3 const& kPoint = kWorldBound.GetCenter();
    float const fRadius    = kWorldBound.GetRadius();

    NiTransform kTransform = pkTarget->GetWorldTransform();

    float fScale = kTransform.m_fScale;
    fScale += fRadius;

    kTransform.m_Translate = kPoint;
    kTransform.m_fScale = fScale;

    m_spProxy->SetLocalTransform( kTransform );
    m_spProxy->Update( 0.0f );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
