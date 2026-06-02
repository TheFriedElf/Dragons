
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetUtils.cpp
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
#include "AssetUtils.h"

#include "NodeTraversal.h"


namespace NewWare
{

namespace Scene
{


namespace AssetUtils
{


/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace GSA
{

namespace { 
    NiDefaultErrorHandler DEFAULT_ERROR_HANDLER;
} //namespace

NiDefaultErrorHandler& GetDefaultErrorHandler()
{
    return DEFAULT_ERROR_HANDLER;
}

//-----------------------------------------------------------------------------------

bool Load( char const* streamName, char const* gsaFilename, 
           NiEntityStreaming*& pkStream, NiEntityErrorInterface& kError )
{
    assert( streamName && gsaFilename );

    pkStream = NiFactories::GetStreamingFactory()->GetPersistent( streamName );
    if ( NULL == pkStream )
    {
        assert( 0 );
        return false;
    }

    pkStream->SetErrorHandler( &kError );

    if ( pkStream->Load(gsaFilename) == false )
    {
        assert( 0 );
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------

NiEntityComponentInterface* GetComponentByName( NiEntityInterface* pkEntity, NiFixedString const& kName )
{
    assert( pkEntity );

    for ( unsigned int ui = 0; ui < pkEntity->GetComponentCount(); ++ui )
    {
        NiEntityComponentInterface* pkComponent = pkEntity->GetComponentAt( ui );
        if ( pkComponent && pkComponent->GetName() == kName )
            return pkComponent;
    }
    return NULL;
}

} //namespace GSA
//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//
namespace NIF
{

void RecursivePrepack( NiAVObject* pkAVObject, bool bStartController )
{
    assert( pkAVObject );

    NiRenderer* pkRenderer = NiRenderer::GetRenderer();

    struct _Functor
    {
        _Functor( bool const _bStartController, NiRenderer* _pkRenderer ) 
            : bStartController(_bStartController), pkRenderer(_pkRenderer) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            if ( pkObject->GetAppCulled() )
                return;

            bool bDynamic = false;
            if ( !(pkRenderer->GetFlags() & NiRenderer::CAPS_HARDWARESKINNING) )
                bDynamic = true;

            if ( bStartController )
            {
                for ( NiTimeController* pkCtrl = pkObject->GetControllers(); pkCtrl; pkCtrl = pkCtrl->GetNext() )
                    pkCtrl->Start();
            }

            if ( NiIsKindOf(NiGeometry, pkObject) )
            {
                if ( NiIsKindOf(NiParticles, pkObject) )
                    bDynamic = true;

                // Search for morpher controllers.
                for ( NiTimeController* pkCtrl = pkObject->GetControllers(); pkCtrl; pkCtrl = pkCtrl->GetNext() )
                {
                    if ( NiIsKindOf(NiGeomMorpherController, pkCtrl) )
                        bDynamic = true;
                }

                NiGeometry* pkGeom = (NiGeometry*)pkObject;
                if ( pkGeom->GetModelData() )
                {
                    NiGeometryData::Consistency eFlags = bDynamic? NiGeometryData::VOLATILE: NiGeometryData::STATIC;
                    pkGeom->SetConsistency( eFlags );

                    pkGeom->GetModelData()->SetCompressFlags( NiGeometryData::COMPRESS_ALL );
                    pkGeom->GetModelData()->SetKeepFlags( NiGeometryData::KEEP_XYZ | 
                                                          NiGeometryData::KEEP_NORM | 
                                                          NiGeometryData::KEEP_INDICES );
                }

                //pkRenderer->PrecacheGeometry( pkGeom, 0, 0 );
            }
        }

        bool const bStartController;
        NiRenderer* pkRenderer;
    } kFunctor( bStartController, pkRenderer );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkAVObject, kFunctor );
}

//-----------------------------------------------------------------------------------

NiNode* Load( NiStream& kStream, char const* filename )
{
    assert( filename );

    if ( kStream.Load(filename) == false )
        return 0;
    if ( kStream.GetObjectCount() == 0 )
        return 0;

    return NiDynamicCast(NiNode, kStream.GetObjectAt(0));
}

} //namespace NIF
//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace AssetUtils


} //namespace Scene

} //namespace NewWare
