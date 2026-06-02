
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : FindTraversal.h
// Description : Find traversal for Gamebryo object.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/20 LeeJR First Created
//

#ifndef _SCENE_FINDTRAVERSAL_H__
#define _SCENE_FINDTRAVERSAL_H__


namespace NewWare
{

namespace Scene
{


namespace FindTraversal
{

    NiCamera* FindFirstCamera( NiNode const* pkNode );
    NiCamera* FindFirstCameraByName( NiNode const* pkNode, char const* name ); // No Case!!
    std::list<NiCamera*> FindCameras( NiNode const* pkNode );
    std::list<NiCamera*> FindCamerasByName( NiNode const* pkNode, char const* name ); // No Case!!

    NiLight* FindFirstLight( NiNode const* pkNode );
    NiLight* FindFirstLightByName( NiNode const* pkNode, char const* name ); // No Case!!
    std::list<NiLight*> FindLights( NiNode const* pkNode );
    std::list<NiLight*> FindLightsByName( NiNode const* pkNode, char const* name ); // No Case!!

    NiAVObject* FindFirstAVObjectByName( NiNode const* pkNode, char const* name ); // No Case!!
    std::list<NiAVObject*> FindAVObjectsByName( NiNode const* pkNode, char const* name ); // No Case!!


    template < class T > 
    inline T* FindController( NiObjectNET const* pkObj )
    {
        if ( NULL == pkObj )
            return NULL;

        NiTimeController* pkControllers = pkObj->GetControllers();
        while ( pkControllers )
        {
            if ( NiIsKindOf(T, pkControllers) )
                return static_cast<T*>(pkControllers);

            pkControllers = pkControllers->GetNext();
        }
        return NULL;
    }
    template < class T > 
    inline T* FindControllerAsNode( NiObjectNET const* pkObj )
    {
        T* pkFind = FindController<T>( pkObj );
        if ( pkFind )
            return pkFind;

        if ( NiIsKindOf(NiNode, pkObj) )
        {
            NiNode const* pkNode = static_cast<NiNode const*>(pkObj);
            unsigned int const uiChildCount = pkNode->GetArrayCount();
            for ( unsigned int uiChild = 0; uiChild < uiChildCount; ++uiChild )
            {
                NiAVObject const* pkChild = pkNode->GetAt( uiChild );
                pkFind = FindControllerAsNode<T>( pkChild );
                if ( pkFind )
                    return pkFind;
            }
        }
        return NULL;
    }

    template < class T > 
    inline T* FindControllerFromPropertyList( NiAVObject const* pkAVObj )
    {
        if ( NULL == pkAVObj )
            return NULL;

        NiPropertyList const& kPropertyList = pkAVObj->GetPropertyList();

        NiTListIterator kIter = kPropertyList.GetHeadPos();
        while ( kIter )
        {
            T* pkFind = FindController<T>( kPropertyList.GetNext(kIter) );
            if ( pkFind )
                return pkFind;
        }
        return NULL;
    }
    template < class T > 
    inline T* FindControllerNodeFromPropertyList( NiAVObject const* pkAVObj )
    {
        T* pkFind = FindControllerFromPropertyList<T>( pkAVObj );
        if ( pkFind )
            return pkFind;

        if ( NiIsKindOf(NiNode, pkAVObj) )
        {
            NiNode const* pkNode = static_cast<NiNode const*>(pkAVObj);
            unsigned int const uiChildCount = pkNode->GetArrayCount();
            for ( unsigned int uiChild = 0; uiChild < uiChildCount; ++uiChild )
            {
                NiAVObject const* pkChild = pkNode->GetAt( uiChild );
                pkFind = FindControllerNodeFromPropertyList<T>( pkChild );
                if ( pkFind )
                    return pkFind;
            }
        }
        return NULL;
    }

} //namespace FindTraversal


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_FINDTRAVERSAL_H__
