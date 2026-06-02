
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionCuller.cpp
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
#include "OcclusionCuller.h"

#include "OcclusionRenderStateD3D.h"
#include "OcclusionTimeController.h"
#include "OcclusionSorter.h"


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

namespace { 
    bool D3DResetNotify( bool bBeforeReset, void* pvData )
    {
        if ( pvData )
        {
            OcclusionCuller::OcclusionQuerys* pkQuery = (OcclusionCuller::OcclusionQuerys*)pvData;
            pkQuery->HandleDeviceReset( bBeforeReset );
        }
        return true;
    }
} //namespace

//-----------------------------------------------------------------------------------

void OcclusionCuller::OcclusionQuerys::HandleDeviceReset( bool bBeforeReset )
{
    OcclusionQueryD3D* pkQuery;
    NiGeometry* pkKey;

    NiTMapIterator iter = m_kQueryObjectPool.GetFirstPos();
    while ( iter )
    {
        m_kQueryObjectPool.GetNext( iter, pkKey, pkQuery );
        assert( pkQuery );

        if ( bBeforeReset )
            pkQuery->Release();
        else
            pkQuery->Recreate();
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

OcclusionCuller::OcclusionQuerys::OcclusionQuerys()
{
    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );
    pkRenderer->AddResetNotificationFunc( D3DResetNotify, this );
}

//-----------------------------------------------------------------------------------

OcclusionCuller::OcclusionQuerys::~OcclusionQuerys()
{
    DeleteAllQueryObject();

    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    assert( pkRenderer );
    pkRenderer->RemoveResetNotificationFunc( D3DResetNotify );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

OcclusionCuller::OcclusionCuller()
    : m_pkQuerys(new OcclusionQuerys), 
      m_pkRenderStater(new OcclusionRenderStateD3D), 
      m_pkDepthTester(new OcclusionDepthTestD3D), 
      m_pkSorter(new OcclusionSorter)
{
    assert( m_pkQuerys );
    assert( m_pkRenderStater );
    assert( m_pkDepthTester );
    assert( m_pkSorter );

    m_pkDepthTester->Init();
}

//-----------------------------------------------------------------------------------

OcclusionCuller::~OcclusionCuller()
{
    delete m_pkSorter;

    m_pkDepthTester->Exit();
    delete m_pkDepthTester;

    delete m_pkRenderStater;
    delete m_pkQuerys;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

unsigned int OcclusionCuller::DoCull( NiCamera const* pkCamera, NiVisibleArray& kArray, bool const bWaitForResult )
{
    PROFILE_FUNC();

    assert( pkCamera );

    SortFrontToBack( pkCamera, kArray );

    QueryBegin();

    FlushDepthDrawBatchs();

    // Check the query result from the previous frame.
    unsigned int const uiCulledCount = QueryGetResults( bWaitForResult );

    QueryOccludees();

    QueryEnd();

    return uiCulledCount;
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::SortFrontToBack( NiCamera const* pkCamera, NiVisibleArray& kArray )
{
    PROFILE_FUNC();

    assert( pkCamera && m_pkSorter );

    m_pkSorter->Begin( pkCamera );
    m_pkSorter->AddObjectArray( kArray );
    m_pkSorter->End();
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::QueryBegin()
{
    PROFILE_FUNC();

    m_kPVSGeometry.RemoveAll();

    assert( m_pkDepthTester );
    m_pkDepthTester->UpdateViewProjMatrix();
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::QueryEnd()
{
    PROFILE_FUNC();

    assert( m_pkRenderStater );
    m_pkRenderStater->SetRenderState( OcclusionRenderStateD3D::COLOR_DRAW_STATE );
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::FlushDepthDrawBatchs()
{
    PROFILE_FUNC();

    assert( m_pkSorter && m_pkRenderStater );

    NiRenderer* pkRenderer = NiRenderer::GetRenderer();
    assert( pkRenderer );

    m_pkRenderStater->SetRenderState( OcclusionRenderStateD3D::DEPTH_DRAW_STATE );

    NiGeometry* pkObject;

    NiTPointerList<NiGeometry*> const& kOccludeRList = m_pkSorter->GetOccludeRList();
    NiTListIterator iter = kOccludeRList.GetHeadPos();
    while ( iter )
    {
        pkObject = kOccludeRList.GetNext( iter );
        assert( pkObject );

        PROFILE_CODE( pkObject->RenderImmediate(pkRenderer) );
    }
}

//-----------------------------------------------------------------------------------

unsigned int OcclusionCuller::QueryGetResults( bool const bWaitForResult )
{
    PROFILE_FUNC();

    NiGeometry* pkObject;
    QueryGetResult kQGResult;
    unsigned int uiCulledCount = 0;

    NiTListIterator iter = m_kOccludeEList.GetHeadPos();
    while ( iter )
    {
        pkObject = m_kOccludeEList.GetNext( iter );

        OcclusionQueryGetResult( bWaitForResult, pkObject, kQGResult );

        if ( kQGResult.bAvailability && kQGResult.iVisiblePixels > 3 )
        {
            m_kPVSGeometry.Add( *pkObject );
        }
        else
        {
            ++uiCulledCount;
        }
    }
    m_kOccludeEList.RemoveAll();

    return uiCulledCount;
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::QueryOccludees()
{
    PROFILE_FUNC();

    assert( m_pkSorter );

    NiGeometry* pkObject;
    OcclusionQueryD3D* pkQuery;

    NiSortedObjectList const& kOccludeEList = m_pkSorter->GetOccludeEList();
    NiTListIterator iter = kOccludeEList.GetHeadPos();
    while ( iter )
    {
        pkObject = kOccludeEList.GetNext( iter );

        PROFILE_CODE( pkQuery = m_pkQuerys->GetQueryObject(pkObject) );
        assert( pkQuery && pkQuery->IsValid() );

        PROFILE_CODE( pkQuery->IssueQueryBegin() );

        OcclusionQueryDrawTestDepth( pkObject );

        PROFILE_CODE( pkQuery->IssueQueryEnd() );

        PROFILE_CODE( m_kOccludeEList.AddTail(pkObject) );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionCuller::OcclusionQueryDrawTestDepth( NiGeometry* pkObject )
{
    PROFILE_FUNC();

    assert( m_pkRenderStater && m_pkDepthTester && pkObject );

    NiTriShape* pkTriShape = GetOccludeeProxy( pkObject );
    if ( NULL == pkTriShape )
    {
        assert( 0 );
        return;
    }

    // Color and depth writes disabled.
    m_pkRenderStater->SetRenderState( OcclusionRenderStateD3D::DEPTH_TEST_STATE );
    m_pkDepthTester->DrawTestDepth( pkTriShape );
}

//-----------------------------------------------------------------------------------

void OcclusionCuller::OcclusionQueryGetResult( bool bWaitForResult, NiGeometry* pkKey, QueryGetResult& kQGResult )
{
    OcclusionQueryD3D* pkQuery = m_pkQuerys->GetQueryObject( pkKey );
    assert( pkQuery && pkQuery->IsValid() );

    pkQuery->GetQueryResult( bWaitForResult, kQGResult.iVisiblePixels, kQGResult.bAvailability );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
