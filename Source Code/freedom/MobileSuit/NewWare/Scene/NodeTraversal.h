
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : NodeTraversal.h
// Description : Node traversal for Gamebryo scene-graph node.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _SCENE_NODETRAVERSAL_H__
#define _SCENE_NODETRAVERSAL_H__


namespace NewWare
{

namespace Scene
{


namespace NodeTraversal
{

    namespace DepthFirst
    {

        template < class Functor > 
        inline void AllObjects_Downward( NiAVObject const* pkAVObj, Functor& kFunctor )
        {
            if ( NULL == pkAVObj )
                return;

            kFunctor( pkAVObj ); // @note - 노드 최상단부터 아래로 내려가면서 처리

            if ( NiIsKindOf(NiNode, pkAVObj) )
            {
                NiNode const* pkNode = static_cast<NiNode const*>(pkAVObj);
                unsigned int const uiChildCount = pkNode->GetArrayCount();
                for ( unsigned int ui = 0; ui < uiChildCount; ++ui )
                {
                    AllObjects_Downward( pkNode->GetAt(ui), kFunctor );
                }
            }
        }

        template < class Functor > 
        inline void AllObjects_Upward( NiAVObject const* pkAVObj, Functor& kFunctor )
        {
            if ( NULL == pkAVObj )
                return;

            if ( NiIsKindOf(NiNode, pkAVObj) )
            {
                NiNode const* pkNode = static_cast<NiNode const*>(pkAVObj);
                unsigned int const uiChildCount = pkNode->GetArrayCount();
                for ( unsigned int ui = 0; ui < uiChildCount; ++ui )
                {
                    AllObjects_Upward( pkNode->GetAt(ui), kFunctor );
                }
            }

            kFunctor( pkAVObj ); // @note - 노드 최하단부터 위로 올라가면서 처리
        }

        template < class Functor > 
        inline bool FirstStop( NiAVObject const* pkAVObj, Functor& kFunctor )
        {
            if ( NULL == pkAVObj )
                return true;

            if ( !kFunctor(pkAVObj) )
                return false;

            if ( NiIsKindOf(NiNode, pkAVObj) )
            {
                NiNode const* pkNode = static_cast<NiNode const*>(pkAVObj);
                unsigned int const uiChildCount = pkNode->GetArrayCount();
                for ( unsigned int ui = 0; ui < uiChildCount; ++ui )
                {
                    if ( !FirstStop(pkNode->GetAt(ui), kFunctor) )
                        return false;
                }
            }
            return true;
        }

        template < class Functor > 
        inline void PreRecurse( NiAVObject const* pkAVObj, Functor& kFunctor )
        {
            if ( NULL == pkAVObj )
                return;

            if ( !kFunctor(pkAVObj) )
                return;

            if ( NiIsKindOf(NiNode, pkAVObj) )
            {
                NiNode const* pkNode = static_cast<NiNode const*>(pkAVObj);
                unsigned int const uiChildCount = pkNode->GetArrayCount();
                for ( unsigned int ui = 0; ui < uiChildCount; ++ui )
                {
                    PreRecurse( pkNode->GetAt(ui), kFunctor );
                }
            }
        }

    } //namespace DepthFirst


    namespace WidthFirst
    {

        ;

    } //namespace WidthFirst

} //namespace NodeTraversal


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_NODETRAVERSAL_H__
