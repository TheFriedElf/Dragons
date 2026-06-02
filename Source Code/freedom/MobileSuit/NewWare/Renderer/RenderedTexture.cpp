
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : RenderedTexture.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/02/19 LeeJR First Created
//

#include "stdafx.h"
#include "RenderedTexture.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

RenderedTexture::RenderedTexture()
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

RenderedTexture::~RenderedTexture()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool RenderedTexture::Click( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, 
                             unsigned int uiWidth, unsigned int uiHeight, NiTexture::FormatPrefs& kFormat, 
                             bool bScreenSpaceCamera, NiColorA const& kClearColor, unsigned int uiClearMode )
{
    assert( pkRenderer && pkTargetObject );

    if ( IsValidClickableState(pkRenderer) == false )
    {
#ifndef EXTERNAL_RELEASE
        _asm { int 3 }; // 랜더링 루프 내에서 호출해야 함!!
#endif //#ifndef EXTERNAL_RELEASE
        return false;
    }

    ApplyTextureToTargetObject( pkRenderer, pkTargetObject, uiWidth, uiHeight, kFormat );
    CreateRenderTargetGroup( pkRenderer );
    RenderToTexture( pkRenderer, bScreenSpaceCamera, kClearColor, uiClearMode );
    return true;
}

//-----------------------------------------------------------------------------------

void RenderedTexture::Reset()
{
    m_spTexture = 0;
    m_spRenderTargetGroup = 0;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void RenderedTexture::ApplyTextureToTargetObject( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, 
                        unsigned int uiWidth, unsigned int uiHeight, NiTexture::FormatPrefs& kFormat )
{
    assert( pkTargetObject && pkRenderer && uiWidth > 0 && uiHeight > 0 );

    m_spTexture = NiRenderedTexture::Create( uiWidth, uiHeight, pkRenderer, kFormat );
    assert( m_spTexture );

    NiTexturingProperty* pkTexturing = (NiTexturingProperty*)pkTargetObject->GetProperty( NiProperty::TEXTURING );
    if ( NULL == pkTexturing )
    {
        pkTexturing = NiNew NiTexturingProperty;
        assert( pkTexturing );

        NiTexturingProperty::Map* pkMap = NiNew NiTexturingProperty::Map();
        assert( pkMap );
        pkTexturing->SetBaseMap( pkMap );
        pkTargetObject->AttachProperty( pkTexturing );
        pkTargetObject->UpdateProperties();
    }
    pkTexturing->SetBaseTexture( m_spTexture );
    pkTexturing->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
}

//-----------------------------------------------------------------------------------

void RenderedTexture::CreateRenderTargetGroup( NiRenderer* pkRenderer )
{
    assert( m_spTexture && pkRenderer );

	if (NULL!=m_spTexture)
	{
	    	m_spRenderTargetGroup = NiRenderTargetGroup::Create( m_spTexture->GetBuffer(), pkRenderer, false, false );
		assert( m_spRenderTargetGroup );
	}
}

//-----------------------------------------------------------------------------------

void RenderedTexture::RenderToTexture( NiRenderer* pkRenderer, bool bScreenSpaceCamera, 
                                       NiColorA const& kClearColor, unsigned int uiClearMode )
{
    assert( m_spRenderTargetGroup && pkRenderer );

    NiColorA kSaveClearColor;
    pkRenderer->GetBackgroundColor( kSaveClearColor );

    pkRenderer->SetBackgroundColor( kClearColor );
    pkRenderer->BeginUsingRenderTargetGroup( m_spRenderTargetGroup, uiClearMode );

    if ( bScreenSpaceCamera )
        pkRenderer->SetScreenSpaceCameraData();

    DoRenderToTexture( pkRenderer );

    pkRenderer->EndUsingRenderTargetGroup();

    pkRenderer->SetBackgroundColor( kSaveClearColor );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
