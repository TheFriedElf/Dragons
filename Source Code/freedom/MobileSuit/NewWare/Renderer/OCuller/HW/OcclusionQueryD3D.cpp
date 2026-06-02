
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionQueryD3D.cpp
// Description : Wrapper class for platform specific occlusion queries.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#include "stdafx.h"
#include "OcclusionQueryD3D.h"

#include <NiDebug.h>
#include <NiDX9Renderer.h>


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

OcclusionQueryD3D::OcclusionQueryD3D()
    : m_bValid(false), 
      m_pkQuery(NULL)
{
    m_bValid = CreateDX9Query();
}

//-----------------------------------------------------------------------------------

OcclusionQueryD3D::~OcclusionQueryD3D()
{
    Release();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void OcclusionQueryD3D::GetQueryResult( bool bWaitForResult, int& iVisiblePixels, bool& bAvailability )
{
    assert( m_pkQuery && IsValid() );

	HRESULT hResult = S_OK;
    bAvailability = false;

    if ( bWaitForResult )
    {
        while ( (hResult = m_pkQuery->GetData(&iVisiblePixels, sizeof(DWORD), D3DGETDATA_FLUSH)) == S_FALSE )
        {
            /* Nothing */
        }
        bAvailability = true;
    }
    else
    {
        if ( (hResult = m_pkQuery->GetData(&iVisiblePixels, sizeof(DWORD), D3DGETDATA_FLUSH)) == S_OK )
            bAvailability = true;
    }

	// If device was lost.
	//if ( hResult == D3DERR_DEVICELOST || (hResult != S_OK && hResult != S_FALSE) )
	//if ( hResult != S_OK && result != S_FALSE )
    if ( hResult == D3DERR_DEVICELOST )
	{
		// Signal positive result.
		iVisiblePixels = 1;
		bAvailability = true;

		// Recreate query.
		//m_bValid = CreateDX9Query();
	}
}

//-----------------------------------------------------------------------------------

bool OcclusionQueryD3D::CreateDX9Query()
{
    NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, NiRenderer::GetRenderer());
    if ( NULL == pkRenderer )
    {
        assert( 0 );
        return false;
    }

    IDirect3DDevice9* pkDevice = pkRenderer->GetD3DDevice();
    if ( pkDevice )
    {
        HRESULT hRes = pkDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &m_pkQuery );
        if ( hRes == S_OK && m_pkQuery )
            return true;
    }

    assert( 0 );
    return false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
