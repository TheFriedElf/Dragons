
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009-2010 Barunson Interactive, Inc
//        Name : TextBoardTextures.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  10/02/19 LeeJR First Created
//

#ifndef _RENDERER_TEXTBOARDTEXTURES_H__
#define _RENDERER_TEXTBOARDTEXTURES_H__

#include "RenderedTexture.h"


namespace NewWare
{

namespace Renderer
{


class TextBoardTextures
{
private:
    class TextRenderedTexture : public RenderedTexture
    {
    public:
        TextRenderedTexture( PgRenderer* pkPgRenderer, TextBoardTextures* pkBoard );
        virtual ~TextRenderedTexture();

        virtual void Reset();
        void Init( XUI::CXUI_Font* pkFont, int iStringYPos, NiColorA const& kTextColor, NiColorA const& kOutColor );
        void ClickPrepare( std::wstring const& strText );

    protected:
        virtual void DoRenderToTexture( NiRenderer* pkRenderer );

    private:
        PgRenderer* m_pkPgRenderer;
        TextBoardTextures* m_pkBoard;

        Pg2DString* m_pkTextString;
        XUI::CXUI_Font* m_pkFont;
        int m_iStringYPos;
        NiColorA m_kTextColor;
        NiColorA m_kOutColor;
    };
    friend class TextRenderedTexture;

    struct TextElement
    {
        TextElement( std::wstring const& text, NiGeometry* geo ) : strText(text) { spGeom = geo; };
        std::wstring strText;
        NiGeometryPtr spGeom;
    };


public:
    explicit TextBoardTextures( PgRenderer* pkPgRenderer );
    ~TextBoardTextures();


    void Init( std::wstring const& strFontName, int iStringYPos, unsigned int uiWidth, unsigned int uiHeight, 
               NiColorA const& kTextColor, NiColorA const& kOutColor )
    {
        m_uiWidth = uiWidth; m_uiHeight = uiHeight;
        m_kTextRenderedTexture.Init( g_kFontMgr.GetFont(strFontName), iStringYPos, kTextColor, kOutColor );
    }
    void Reset();


    bool Add( NiRenderer* pkRenderer, NiAVObject* pkTargetObject, std::wstring const& strOwnerName );
    void ClickAll( NiRenderer* pkRenderer, NiTexture::FormatPrefs& kFormat, bool bScreenSpaceCamera = true, 
                   NiColorA const& kClearColor = NiColorA::WHITE, unsigned int uiClearMode = NiRenderer::CLEAR_ALL );


private:
    TextRenderedTexture m_kTextRenderedTexture;

    unsigned int m_uiWidth, m_uiHeight;

    std::list<TextElement> m_lstTextElementList;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_TEXTBOARDTEXTURES_H__
