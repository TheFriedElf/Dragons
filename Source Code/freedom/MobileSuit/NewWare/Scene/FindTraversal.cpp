
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : FindTraversal.cpp
// Description : Find traversal for Gamebryo object.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#include "stdafx.h"
#include "FindTraversal.h"

#include "NodeTraversal.h"


namespace NewWare
{

namespace Scene
{


namespace FindTraversal
{

/////////////////////////////////////////////////////////////////////////////////////////////
// Camera stuff

NiCamera* FindFirstCamera( NiNode const* pkNode )
{
    assert( pkNode );

    struct _Functor
    {
        _Functor() : pkCamera(NULL) { /* Nothing */ };

        bool operator() ( NiAVObject const* pkObject )
        {
            pkCamera = NiDynamicCast(NiCamera, pkObject);
            return (pkCamera == NULL); // if no camera, continue traversal
        }

        NiCamera* pkCamera;
    } kFunctor;

    NodeTraversal::DepthFirst::FirstStop( pkNode, kFunctor );
    return kFunctor.pkCamera;
}

//-----------------------------------------------------------------------------------

NiCamera* FindFirstCameraByName( NiNode const* pkNode, char const* name )
{
    assert( pkNode && name );

    struct _Functor
    {
        explicit _Functor( char const* name ) : pcName(name), pkCamera(NULL) { assert( pcName ); };

        bool operator() ( NiAVObject const* pkObject )
        {
            pkCamera = NiDynamicCast(NiCamera, pkObject);
            if ( pkCamera && pkCamera->GetName().EqualsNoCase(pcName) )
                return false;
            return true; // if no camera, continue traversal
        }

        char const* pcName;
        NiCamera* pkCamera;
    } kFunctor( name );

    NodeTraversal::DepthFirst::FirstStop( pkNode, kFunctor );
    return kFunctor.pkCamera;
}

//-----------------------------------------------------------------------------------

std::list<NiCamera*> FindCameras( NiNode const* pkNode )
{
    assert( pkNode );

    std::list<NiCamera*> lstNiCameraList;

    struct _Functor
    {
        explicit _Functor( std::list<NiCamera*>& lstCameras ) : lstNiCameraList(lstCameras) { /* Nothing */ };

        void operator() ( NiAVObject const* pkObject )
        {
            NiCamera* pkCamera = NiDynamicCast(NiCamera, pkObject);
            if ( pkCamera )
                lstNiCameraList.push_back( pkCamera );
        }

        std::list<NiCamera*>& lstNiCameraList;
    } kFunctor( lstNiCameraList );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkNode, kFunctor );
    return lstNiCameraList;
}

//-----------------------------------------------------------------------------------

std::list<NiCamera*> FindCamerasByName( NiNode const* pkNode, char const* name )
{
    assert( pkNode && name );

    std::list<NiCamera*> lstNiCameraList;

    struct _Functor
    {
        _Functor( std::list<NiCamera*>& lstCameras, char const* name ) 
            : lstNiCameraList(lstCameras), pcName(name) { assert( pcName ); };

        void operator() ( NiAVObject const* pkObject )
        {
            NiCamera* pkCamera = NiDynamicCast(NiCamera, pkObject);
            if ( pkCamera && pkCamera->GetName().EqualsNoCase(pcName) )
                lstNiCameraList.push_back( pkCamera );
        }

        std::list<NiCamera*>& lstNiCameraList;
        char const* pcName;
    } kFunctor( lstNiCameraList, name );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkNode, kFunctor );
    return lstNiCameraList;
}

// Camera stuff
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Light stuff

NiLight* FindFirstLight( NiNode const* pkNode )
{
    return NULL;
}

//-----------------------------------------------------------------------------------

NiLight* FindFirstLightByName( NiNode const* pkNode, char const* name )
{
    return NULL;
}

//-----------------------------------------------------------------------------------

std::list<NiLight*> FindLights( NiNode const* pkNode )
{
    std::list<NiLight*> lstNiLightList;
    return lstNiLightList;
}

//-----------------------------------------------------------------------------------

std::list<NiLight*> FindLightsByName( NiNode const* pkNode, char const* name )
{
    std::list<NiLight*> lstNiLightList;
    return lstNiLightList;
}

// Light stuff
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// NiAVObject stuff

NiAVObject* FindFirstAVObjectByName( NiNode const* pkNode, char const* name )
{
    assert( pkNode && name );

    struct _Functor
    {
        explicit _Functor( char const* name ) : pcName(name), pkAVObject(NULL) { assert( pcName ); };

        bool operator() ( NiAVObject const* pkObject )
        {
            if ( pkObject && pkObject->GetName().EqualsNoCase(pcName) )
            {
                pkAVObject = const_cast<NiAVObject*>(pkObject);
                return false;
            }
            return true; // if no equals name, continue traversal
        }

        char const* pcName;
        NiAVObject* pkAVObject;
    } kFunctor( name );

    NodeTraversal::DepthFirst::FirstStop( pkNode, kFunctor );
    return kFunctor.pkAVObject;
}

//-----------------------------------------------------------------------------------

std::list<NiAVObject*> FindAVObjectsByName( NiNode const* pkNode, char const* name )
{
    assert( pkNode && name );

    std::list<NiAVObject*> lstNiAVObjectList;

    struct _Functor
    {
        _Functor( std::list<NiAVObject*>& lstNiAVObjects, char const* name ) 
            : lstNiAVObjectList(lstNiAVObjects), pcName(name) { assert( pcName ); };

        void operator() ( NiAVObject const* pkObject )
        {
            if ( pkObject && pkObject->GetName().EqualsNoCase(pcName) )
                lstNiAVObjectList.push_back( const_cast<NiAVObject*>(pkObject) );
        }

        std::list<NiAVObject*>& lstNiAVObjectList;
        char const* pcName;
    } kFunctor( lstNiAVObjectList, name );

    NodeTraversal::DepthFirst::AllObjects_Downward( pkNode, kFunctor );
    return lstNiAVObjectList;
}

// NiAVObject stuff
/////////////////////////////////////////////////////////////////////////////////////////////

} //namespace FindTraversal


} //namespace Scene

} //namespace NewWare
