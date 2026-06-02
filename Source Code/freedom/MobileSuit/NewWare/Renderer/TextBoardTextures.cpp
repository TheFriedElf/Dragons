
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : TextBoardTextures.cpp
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
#include "TextBoardTextures.h"

#include "DrawWorkflow.h"
#include "Kernel/RenderStateTagExtraData.h"

#include "../../Pg2DString.h"
#include "../../PgRenderer.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

TextBoardTextures::TextRenderedTexture::TextRenderedTexture( PgRenderer* pkPgRenderer, TextBoardTextures* pkBoard )
    : m_pkPgRenderer(pkPgRenderer), 
      m_pkBoard(pkBoard), 
      m_pkTextString(NULL), 
      m_pkFont(NULL), 
      m_iStringYPos(0), 
      m_kTextColor(NiColorA::WHITE), 
      m_kOutColor(NiColorA::WHITE)
{
    assert( m_pkPgRenderer && m_pkBoard );
}

//-----------------------------------------------------------------------------------

TextBoardTextures::TextRenderedTexture::~TextRenderedTexture()
{
    delete m_pkTextString;
}

//-----------------------------------------------------------------------------------

void TextBoardTextures::TextRenderedTexture::Reset()
{
    delete m_pkTextString;
    m_pkTextString = NULL;

    m_pkFont = NULL;
    m_iStringYPos = 0;
    m_kTextColor = NiColorA::WHITE;
    m_kOutColor = NiColorA::WHITE;

    RenderedTexture::Reset();
}

//-----------------------------------------------------------------------------------

void TextBoardTextures::TextRenderedTexture::Init( XUI::CXUI_Font* pkFont, int iStringYPos, 
                                                   NiColorA const& kTextColor, NiColorA const& kOutColor )
{
    if ( NULL == m_pkTextString )
    {
        m_pkFont = pkFont;
        assert( m_pkFont );

        m_pkTextString = new Pg2DString( XUI::PgFontDef(m_pkFont), _T("") );
        assert( m_pkTextString );
    }

    m_iStringYPos = iStringYPos;
    m_kTextColor = kTextColor;
    m_kOutColor = kOutColor;
}

//-----------------------------------------------------------------------------------

void TextBoardTextures::TextRenderedTexture::ClickPrepare( std::wstring const& strText )
{
    assert( m_pkTextString && m_pkFont );

    m_pkTextString->SetText( m_pkFont, strText );
}

//-----------------------------------------------------------------------------------

void TextBoardTextures::TextRenderedTexture::DoRenderToTexture( NiRenderer* pkRenderer )
{
    assert( pkRenderer && m_pkTextString && m_pkPgRenderer );

    // 길드이름 중앙정렬을 위한 계산 (m_uiWidth = 이름 중앙 정렬을 위한 네임태그의 사이즈)
    const int iStringXPos = (m_pkBoard->m_uiWidth - m_pkTextString->GetSize().x)/2;

    m_pkTextString->Draw( m_pkPgRenderer, iStringXPos, m_iStringYPos, 
                          m_kTextColor, m_kOutColor, true, XUI::XTF_OUTLINE );
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================


/////////////////////////////////////////////////////////////////////////////////////////////
//

TextBoardTextures::TextBoardTextures( PgRenderer* pkPgRenderer )
    : m_kTextRenderedTexture(pkPgRenderer, this), 
      m_uiWidth(0), 
      m_uiHeight(0)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

TextBoardTextures::~TextBoardTextures()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void TextBoardTextures::Reset()
{
    m_kTextRenderedTexture.Reset();

    m_uiWidth = 0;
    m_uiHeight = 0;

    m_lstTextElementList.clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool TextBoardTextures::Add( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, std::wstring const& strOwnerName )
{
    assert( pkRenderer && pkTargetObject );

    NiGeometry* pkGeom = NiDynamicCast(NiGeometry, pkTargetObject);
    if ( NULL == pkGeom )
        return false;

    NiGeometryData* pkModelData = pkGeom->GetModelData();
    if ( NULL == pkModelData )
        return false;

    SetRenderStateTagExtraDataNumber( *pkGeom, Kernel::RenderStateTagExtraData::RESET_RENDERSTATE );

    m_lstTextElementList.push_back( TextElement((strOwnerName.empty()? TTW(201014): strOwnerName), pkGeom) );

    if ( NULL == pkModelData->GetTextures() )
    {
        NiPoint2* apkNewTextureUV = NiNew NiPoint2[4];
        if ( apkNewTextureUV )
        {
            // U=1.0, V=1.0
            apkNewTextureUV[0].x =1.0f;
            apkNewTextureUV[0].y =1.0f;
            // U=1.0, V=0.0
            apkNewTextureUV[1].x =1.0f;
            apkNewTextureUV[1].y =0.0f;
            // U=0.0, V=1.0
            apkNewTextureUV[2].x =0.0f;
            apkNewTextureUV[2].y =1.0f;
            // U=0.0, V=0.0
            apkNewTextureUV[3].x =0.0f;
            apkNewTextureUV[3].y =0.0f;
        }
        pkModelData->SetData( pkModelData->GetVertexCount(), pkModelData->GetVertices(), 
                              pkModelData->GetNormals(), pkModelData->GetColors(), apkNewTextureUV, 1, 
                              NiGeometryData::NBT_METHOD_NONE );
        // 변경 내용 적용
        pkModelData->SetConsistency( NiGeometryData::MUTABLE );
        pkModelData->MarkAsChanged( NiGeometryData::TEXTURE_MASK );
    }

    return true;
}

//-----------------------------------------------------------------------------------

void TextBoardTextures::ClickAll( NiRenderer* pkRenderer, NiTexture::FormatPrefs& kFormat, 
                                  bool bScreenSpaceCamera, NiColorA const& kClearColor, unsigned int uiClearMode )
{
    assert( pkRenderer && m_uiWidth > 0 && m_uiHeight > 0 );

    for ( std::list<TextElement>::iterator iter = m_lstTextElementList.begin(); 
          iter != m_lstTextElementList.end(); ++iter )
    {
        m_kTextRenderedTexture.ClickPrepare( (*iter).strText );
        m_kTextRenderedTexture.Click( pkRenderer, (*iter).spGeom, m_uiWidth, m_uiHeight, 
                                      kFormat, bScreenSpaceCamera, kClearColor, uiClearMode );
    }
    m_lstTextElementList.clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
