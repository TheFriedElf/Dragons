
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderQueue.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/26 LeeJR First Created
//

#include "stdafx.h"
#include "RenderQueue.h"

#include "../DrawUtils.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/////////////////////////////////////////////////////////////////////////////////////////////
// PassGroup PImple

class RenderQueue::PassGroup
{
    friend class RenderQueue;

public:
    PassGroup() : m_pkCamera(NULL) { /* Nothing */ };
    ~PassGroup() { /* Nothing */ };


    void Begin( NiCamera* pkCamera )
    {
        m_pkCamera = pkCamera;
    }

    void DrawUnSortedList( NiRenderer* pkRenderer )
    {
        assert( pkRenderer );

        unsigned int const uiQuantity = m_kUnSortedList.GetCount();
        for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
        {
            NiGeometry& kObject = m_kUnSortedList.GetAt( ui );
            kObject.RenderImmediate( pkRenderer );
        }
    }

    void DrawSortedList( NiAccumulator* pkAccumulator )
    {
        assert( pkAccumulator && m_pkCamera );

        pkAccumulator->StartAccumulating( m_pkCamera );

        pkAccumulator->RegisterObjectArray( m_kSortedList );

        pkAccumulator->FinishAccumulating();
    }

    void DrawObjectList( NiRenderer* pkRenderer )
    {
        assert( pkRenderer );

        unsigned int const uiQuantity = m_kObjectList.GetCount();
        for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
        {
            NiGeometry& kObject = m_kObjectList.GetAt( ui );
            kObject.RenderImmediate( pkRenderer );
        }
    }

    void DrawLensFilter( NiRenderer* pkRenderer )
    {
        assert( pkRenderer );

        if ( m_spLensFilter )
            m_spLensFilter->Draw( pkRenderer );
    }

    void DrawOverlay( NiRenderer* pkRenderer )
    {
        assert( pkRenderer );

        unsigned int const uiQuantity = m_kOverlayList.GetCount();
        for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
        {
            NiGeometry& kObject = m_kOverlayList.GetAt( ui );
            kObject.RenderImmediate( pkRenderer );
        }
    }

    void End()
    {
        m_pkCamera = NULL;

        m_kUnSortedList.RemoveAll();
        m_kSortedList.RemoveAll();
        m_kObjectList.RemoveAll();
        m_spLensFilter = 0;
        m_kOverlayList.RemoveAll();
    }


private:
    NiCamera* m_pkCamera;

    NiVisibleArray m_kUnSortedList;
    NiVisibleArray m_kSortedList;
    NiVisibleArray m_kObjectList;
    NiScreenTexturePtr m_spLensFilter;
    NiVisibleArray m_kOverlayList;
};

// PassGroup PImple
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

RenderQueue::RenderQueue()
    : m_kCuller(NULL)
{
    for ( unsigned int ui = 0; ui < m_apkPassGroupList.size(); ++ui )
    {
        m_apkPassGroupList[ui] = new PassGroup;
        assert( m_apkPassGroupList[ui] );
    }
}

//-----------------------------------------------------------------------------------

RenderQueue::~RenderQueue()
{
    for ( unsigned int ui = 0; ui < m_apkPassGroupList.size(); ++ui )
    {
        delete m_apkPassGroupList[ui];
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void RenderQueue::Draw( NiRenderer* pkRenderer, NiCamera* pkCamera )
{
    for ( unsigned int ui = 0; ui < m_apkPassGroupList.size(); ++ui )
    {
        m_apkPassGroupList[ui]->Begin( pkCamera );

        m_apkPassGroupList[ui]->DrawUnSortedList( pkRenderer );
        m_apkPassGroupList[ui]->DrawSortedList( pkRenderer->GetSorter() );
        m_apkPassGroupList[ui]->DrawObjectList( pkRenderer );
        m_apkPassGroupList[ui]->DrawLensFilter( pkRenderer );
        m_apkPassGroupList[ui]->DrawOverlay( pkRenderer );

        m_apkPassGroupList[ui]->End();
    }
}

//-----------------------------------------------------------------------------------

void RenderQueue::Draw( PassGroupLayer eLayer, NiRenderer* pkRenderer, NiCamera* pkCamera )
{
    PassGroup* pkPassGroup = m_apkPassGroupList[eLayer];
    assert( pkPassGroup );

    pkPassGroup->Begin( pkCamera );

    pkPassGroup->DrawUnSortedList( pkRenderer );
    pkPassGroup->DrawSortedList( pkRenderer->GetSorter() );
    pkPassGroup->DrawObjectList( pkRenderer );
    pkPassGroup->DrawLensFilter( pkRenderer );
    pkPassGroup->DrawOverlay( pkRenderer );

    pkPassGroup->End();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

unsigned int RenderQueue::AddUnSorted( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera )
{
    assert( pkObject && pkCamera );

    return DrawUtils::GetPVSFromCullScene( pkCamera, m_kCuller, pkObject, 
                                           m_apkPassGroupList[eLayer]->m_kUnSortedList, false );
}

//-----------------------------------------------------------------------------------

unsigned int RenderQueue::AddSorted( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera )
{
    assert( pkObject && pkCamera );

    return DrawUtils::GetPVSFromCullScene( pkCamera, m_kCuller, pkObject, 
                                           m_apkPassGroupList[eLayer]->m_kSortedList, false );
}

//-----------------------------------------------------------------------------------

unsigned int RenderQueue::AddObject( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera )
{
    assert( pkObject && pkCamera );

    return DrawUtils::GetPVSFromCullScene( pkCamera, m_kCuller, pkObject, 
                                           m_apkPassGroupList[eLayer]->m_kObjectList, false );
}

//-----------------------------------------------------------------------------------

void RenderQueue::AddLensFilter( PassGroupLayer eLayer, NiScreenTexture* pkObject )
{
    assert( pkObject );

    m_apkPassGroupList[eLayer]->m_spLensFilter = pkObject;
}

//-----------------------------------------------------------------------------------

unsigned int RenderQueue::AddOverlay( PassGroupLayer eLayer, NiGeometry* pkObject, NiCamera* pkCamera )
{
    assert( pkObject && pkCamera );

    return DrawUtils::GetPVSFromCullScene( pkCamera, m_kCuller, pkObject, 
                                           m_apkPassGroupList[eLayer]->m_kOverlayList, false );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
