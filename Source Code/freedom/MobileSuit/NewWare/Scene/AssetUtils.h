
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetUtils.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/21 LeeJR First Created
//

#ifndef _SCENE_ASSETUTILS_H__
#define _SCENE_ASSETUTILS_H__


namespace NewWare
{

namespace Scene
{


namespace AssetUtils
{

    namespace GSA
    {

        typedef std::list< NiEntityInterface* >       NiEntityInterfaceList;
        typedef NiEntityInterfaceList::const_iterator NiEntityInterfaceListCIter;
        typedef NiEntityInterfaceList::iterator       NiEntityInterfaceListIter;

        NiDefaultErrorHandler& GetDefaultErrorHandler();

        bool Load( char const* streamName, char const* gsaFilename, 
                   NiEntityStreaming*& pkStream, NiEntityErrorInterface& kError = GetDefaultErrorHandler() );

        NiEntityComponentInterface* GetComponentByName( NiEntityInterface* pkEntity, NiFixedString const& kName );

    } //namespace GSA



    namespace NIF
    {
        typedef std::map< std::string, NiNodePtr > NiNodeMap;
        typedef NiNodeMap::value_type              NiNodeMapValueType;
        typedef NiNodeMap::iterator                NiNodeMapIter;
        typedef NiNodeMap::const_iterator          NiNodeMapCIter;

        typedef std::list< NiNodePtr >     NiNodeList;
        typedef NiNodeList::iterator       NiNodeListIter;
        typedef NiNodeList::const_iterator NiNodeListCIter;

        void RecursivePrepack( NiAVObject* pkAVObject, bool bStartController = true );

        NiNode* Load( NiStream& kStream, char const* filename );
        inline NiNodePtr Load( char const* filename )
        {
            assert( filename );
            NiStream kStream;
            return Load( kStream, filename );
        }

        inline NiNodePtr LoadAndPrepack( char const* fileName )
        {
            NiNodePtr spNode = Load( fileName );
            if ( 0 == spNode )
                return 0;

            spNode->UpdateProperties();
            spNode->UpdateEffects();
            RecursivePrepack( spNode, true );
            return spNode;
        }

    } //namespace NIF

} //namespace AssetUtils


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_ASSETUTILS_H__
