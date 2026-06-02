
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneProperty.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/21 LeeJR First Created
//

#include "stdafx.h"
#include "SceneProperty.h"

#include "ApplyProperty.h"
#include "ApplyTraversal.h"
#include "../Renderer/DrawOptions.h"


namespace NewWare
{

namespace Scene
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

SceneProperty::FogChannel::FogChannel()
    : m_bForceTurnOn(true), 
      m_bDebugAdjustable(false), 
      m_pkProperty(NULL), 
      m_bApplyToAffectedObject(true), 
      m_bUseEndFar(true)
{
    m_vAffectedObjectList.reserve( AFFECTEDOBJECT_DEFAULT_COUNT );
}

//-----------------------------------------------------------------------------------

void SceneProperty::FogChannel::TurnOn( NiNode* pkSceneRoot, bool bAdjustable )
{
    if ( NULL == m_pkProperty )
    {
        m_pkProperty = ApplyProperty::Attatch::CreateFogProperty( pkSceneRoot, true, 
                                NiFogProperty::FOG_Z_LINEAR, 500.0f, 
                                Renderer::DrawOptions::FRUSTUM_FAR_DEF_LANGE, 0.05f, false );

        for ( AffectedObjectListCIter citer = m_vAffectedObjectList.begin(); 
              citer != m_vAffectedObjectList.end(); ++citer )
        {
            ApplyProperty::Attatch::SwitchFogProperty( !m_bApplyToAffectedObject, (*citer), 
                                                       false, m_pkProperty, false );
        }

        pkSceneRoot->UpdateProperties();
        ApplyTraversal::Property::DisableFogAtBillboardAndParticle( pkSceneRoot );
    }
    else
    {
        m_pkProperty->SetFog( true );

        for ( AffectedObjectListCIter citer = m_vAffectedObjectList.begin(); 
              citer != m_vAffectedObjectList.end(); ++citer )
        {
            ApplyProperty::Attatch::SwitchFogProperty( !m_bApplyToAffectedObject, (*citer), 
                                                       false, m_pkProperty, false );
        }

        ApplyTraversal::Geometry::SetMaterialNeedsUpdate( pkSceneRoot, true );
    }
    m_bDebugAdjustable = bAdjustable;
}

//-----------------------------------------------------------------------------------

void SceneProperty::FogChannel::TurnOff( NiNode* pkSceneRoot )
{
    assert( pkSceneRoot );

    if ( m_pkProperty )
        m_pkProperty->SetFog( false );

    // fast access! (No-searching!!)
    for ( AffectedObjectListCIter citer = m_vAffectedObjectList.begin(); 
          citer != m_vAffectedObjectList.end(); ++citer )
    {
        NiFogProperty* pkProp = (NiFogProperty*)(*citer)->GetProperty( NiProperty::FOG );
        assert( pkProp );
        if ( pkProp )
            pkProp->SetFog( false );
    }
    m_bDebugAdjustable = false;

    ApplyTraversal::Geometry::SetMaterialNeedsUpdate( pkSceneRoot, true );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool SceneProperty::RemoveAffectedObjectFromFogChannel( NiAVObject* pkAVObject )
{
    assert( pkAVObject );
    AffectedObjectListIter iter = 
        std::find( m_kFog.m_vAffectedObjectList.begin(), m_kFog.m_vAffectedObjectList.end(), pkAVObject );
    if ( iter != m_kFog.m_vAffectedObjectList.end() )
    {
        m_kFog.m_vAffectedObjectList.erase( iter );
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------


//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Scene

} //namespace NewWare
