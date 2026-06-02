
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 - 2010 Barunson Interactive, Inc
//        Name : AttachObjects.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * AttachSlot m_kAttachSlot/m_kAttachSlot_NoZTest 대체 모듈임.
//   * 대체 모듈 적용시 PgAVObjectAlphaProcessManager를 ProcessManager를 상속 받게 바꾸어야 함!
//     (참고: AttachParticleObjects::RemoveProcess()함수 내부의 객체 타입-케스팅 바꾸기)
// 
// Revisions :
//  10/01/14 LeeJR First Created
//

#include "stdafx.h"
#include "AttachObjects.h"

#include "../Renderer/DrawUtils.h"
#include "../Scene/ApplyTraversal.h"

#include "../../PgParticle.h"
#include "../../PgParticleProcessorFadeOut.h"
#include "../../PgParticleProcessorAttachToNode.h"
#include "../../PgParticleMan.h"
#include "../../PgAVObjectAlphaProcessManager.h"
#include "../../PgWorkerThread.h"


namespace NewWare
{

namespace Game
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

AttachObjects::AttachObjects()
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

AttachObjects::~AttachObjects()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void AttachObjects::DrawList( NiAVObjectList& mList, NiCamera const* pkCamera, 
                              NiCullingProcess& kCuller, NiVisibleArray& kVisibleSet )
{
    for ( NiAVObjectListCIter citer = mList.begin(); citer != mList.end(); ++citer )
    {
        NiAVObject* pkAVObject = citer->second;

        pkAVObject->SetAppCulled( false );
        Renderer::DrawUtils::GetPVSFromCullScene( pkCamera, kCuller, pkAVObject, kVisibleSet, false );
        pkAVObject->SetAppCulled( true );
    }
}

//-----------------------------------------------------------------------------------

bool AttachObjects::RemoveFrom( NiAVObjectList& mList, Key key, ProcessManager* pkProcessManager )
{
    NiAVObjectListIter iter = mList.find( key );
    if ( iter != mList.end() )
    {
        RemoveProcess( iter->second, pkProcessManager );
        m_mNormals.erase( iter );
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

void AttachObjects::_RemoveAll( NiAVObjectList& mList )
{
    for ( NiAVObjectListIter iter = mList.begin(); iter != mList.end(); ++iter )
    {
        Delete( iter->second );
    }
    mList.clear();
}

//-----------------------------------------------------------------------------------

void AttachObjects::Delete( NiAVObject* pkAVObject )
{
    NiNode* pkParent = pkAVObject->GetParent();
    if ( pkParent )
        THREAD_DELETE_OBJECT( pkParent->DetachChild(pkAVObject) );
    else
        THREAD_DELETE_OBJECT( pkAVObject );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiAVObject* AttachObjects::_GetNodeByKeyName( NiAVObjectList const& mList, Key key, char const* name ) const
{
    NiAVObjectListCIter citer = mList.find( key );
    if ( citer != mList.end() )
    {
        if ( strcmp(citer->second->GetName(), name) )
            return citer->second;
    }
    return NULL;
}

//-----------------------------------------------------------------------------------

NiAVObject*	AttachObjects::_GetNodeByExtraValue( NiAVObjectList const& mList, 
                                        std::string const& strValue, char const* extraDataName ) const
{
    NiAVObject* pkAVObject;
    NiStringExtraData* pkExtraData;
    for ( NiAVObjectListCIter citer = mList.begin(); citer != mList.end(); ++citer )
    {
        pkAVObject = citer->second;
        if ( pkAVObject )
        {
            pkExtraData = static_cast<NiStringExtraData*>(pkAVObject->GetExtraData(extraDataName));
            if ( pkExtraData && pkExtraData->GetValue() == strValue.c_str() )
                return pkAVObject;
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------------

bool AttachObjects::_GetWorldPositionByKeyName( NiPoint3& kPos, 
                                        NiAVObjectList const& mList, Key key, char const* name ) const
{
    NiAVObjectListCIter citer = mList.find( key );
    if ( citer != mList.end() )
    {
        NiAVObject* pkNode = citer->second->GetObjectByName( name );
        if ( pkNode )
        {
            kPos = pkNode->GetWorldTranslate();
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------

bool AttachObjects::_SetAlphaGroup( NiAVObjectList const& mList, Key key, int iGroupNumber )
{
    NiAVObjectListCIter citer = mList.find( key );
    if ( citer != mList.end() )
    {
        Scene::ApplyTraversal::Property::SetAlphaGroup( citer->second, iGroupNumber );
        return true;
    }
    return false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

bool AttachParticleObjects::Add( Key key, NiAVObject* pkAVObject )
{
    if ( NULL == pkAVObject || NiIsKindOf(PgParticle, pkAVObject) == false )
    {
        assert( 0 );
        return false;
    }

    PgParticle* pkParticle = static_cast<PgParticle*>(pkAVObject);
    if ( pkParticle->GetZTest() )
        m_mNormals.insert( NiAVObjectListValueType(key, pkParticle) );
    else
        m_mNoZTests.insert( NiAVObjectListValueType(key, pkParticle) );
    return true;
}

//-----------------------------------------------------------------------------------

bool AttachParticleObjects::Add( Key key, NiAVObject* pkAVObject, NiAVObject* pkAttachTarget )
{
    if ( NULL == pkAVObject || NiIsKindOf(PgParticle, pkAVObject) == false || NULL == pkAttachTarget )
    {
        assert( 0 );
        return false;
    }

    PgParticle* pkParticle = static_cast<PgParticle*>(pkAVObject);
    pkParticle->SetParticleProcessor( NiNew PgParticleProcessorAttachToNode(pkAttachTarget) );

    m_mNormals.insert( NiAVObjectListValueType(key, pkAVObject) );
    return true;
}

//-----------------------------------------------------------------------------------

void AttachParticleObjects::_FadeOut( NiAVObjectList const& mList, char const* extraDataName )
{
    assert( extraDataName );

    PgParticle* pkParticle;
    NiFloatExtraData* pkExtraData;
    for ( NiAVObjectListCIter citer = mList.begin(); citer != mList.end(); ++citer )
    {
        pkParticle = static_cast<PgParticle*>(NiSmartPointerCast(NiAVObject, citer->second));
        if ( pkParticle )
        {
            pkExtraData = static_cast<NiFloatExtraData*>(pkParticle->GetExtraData(extraDataName));
            if ( pkExtraData && 
                 pkParticle->GetParticleProcessor(PgParticleProcessor::PPID_FADE_OUT) == NULL )
            {
                pkParticle->SetParticleProcessor( NiNew PgParticleProcessorFadeOut(pkExtraData->GetValue()) );
            }
        }
    }
}

//-----------------------------------------------------------------------------------

bool AttachParticleObjects::_PlayGenerate( NiAVObjectList& mList, Key key, bool bGenerate, bool bRemove )
{
    NiAVObjectListIter iter = mList.find( key );
    if ( iter != mList.end() )
    {
        PgParticleMan::ChangeParticleGeneration( iter->second, bGenerate );

        if ( bRemove )
            mList.erase( iter );
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

bool AttachParticleObjects::_DetachAlphaProperty( NiAVObjectList const& mList, Key key, NiAlphaProperty* pkAlphaProp )
{
    NiAVObjectListCIter citer = mList.find( key );
    if ( citer != mList.end() )
    {
        NiAVObject* pkAVObject = citer->second;
        Scene::ApplyTraversal::Property::DetachAlphaProperty( pkAVObject, pkAlphaProp );
        pkAVObject->UpdateProperties();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

void AttachParticleObjects::_SetDefaultMaterialNeedsUpdateFlag( NiAVObjectList const& mList, bool bFlag )
{
    for ( NiAVObjectListCIter citer = mList.begin(); citer != mList.end(); ++citer )
    {
        Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( citer->second, bFlag );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void AttachParticleObjects::_UpdateLifetime( NiAVObjectList& mList, float fAccumTime, float fFrameTime, 
                                             ProcessManager* pkProcessManager )
{
    PgParticle* pkParticle;

    NiAVObjectListIter iter = mList.begin();
    while ( iter != mList.end() )
    {
        pkParticle = static_cast<PgParticle*>(NiSmartPointerCast(NiAVObject, iter->second));
        if ( pkParticle && pkParticle->IsFinished() )
        {
            RemoveProcess( pkParticle, pkProcessManager );
            iter = mList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }
}

//-----------------------------------------------------------------------------------

void AttachParticleObjects::_UpdateTransform( NiAVObjectList& mList, float fAccumTime, float fFrameTime )
{
    PgParticle* pkParticle;
    for ( NiAVObjectListIter iter = mList.begin(); iter != mList.end(); ++iter )
    {
        pkParticle = static_cast<PgParticle*>(NiSmartPointerCast(NiAVObject, iter->second));
        if ( pkParticle )
            pkParticle->Update( fAccumTime, fFrameTime );
    }
}

//-----------------------------------------------------------------------------------

void AttachParticleObjects::RemoveProcess( NiAVObject* pkAVObject, ProcessManager* pkProcessManager )
{
    assert( pkProcessManager );

    if ( pkProcessManager )
    {
        PgAVObjectAlphaProcessManager* pkAlphaProcessMgr = 
                                        //static_cast<PgAVObjectAlphaProcessManager*>(pkProcessManager);
                                        (PgAVObjectAlphaProcessManager*)pkProcessManager;
        pkAlphaProcessMgr->AddAVObject( pkAVObject, 0.4f, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB, 
                                        ALPHA_MANAGER_PROCESS_TYPE_OBJECT_DELETE, false, true );
    }
    else
    {
        Delete( pkAVObject );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Game

} //namespace NewWare
