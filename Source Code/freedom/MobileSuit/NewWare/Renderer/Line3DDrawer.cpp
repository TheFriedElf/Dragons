
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Line3DDrawer.cpp
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
#include "Line3DDrawer.h"


namespace NewWare
{

namespace Renderer
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

Line3DDrawer::Line3DDrawer()
    : m_bDrawable(false), 
      m_usPosition(0), 
      m_usNumVertices(0), 
      m_pkLinesData(NULL)
{
    CreateLines();
}

//-----------------------------------------------------------------------------------

Line3DDrawer::~Line3DDrawer()
{
    DeleteLines();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void Line3DDrawer::Init()
{
    m_usPosition = 0;

    memset( m_pkLinesData->GetFlags(), false, sizeof(NiBool)*USHRT_MAX );
}

//-----------------------------------------------------------------------------------

void Line3DDrawer::Draw( NiPoint3 const& kStart, NiPoint3 const& kEnd, NiColorA const& kColor )
{
    if ( IsDrawable() == false )
        return;

    assert( m_usPosition + 2 < m_usNumVertices );

    if ( m_usPosition <= m_usNumVertices - 2 )
    {
        m_pkLinesData->GetVertices()[m_usPosition + 0] = kStart;
        m_pkLinesData->GetVertices()[m_usPosition + 1] = kEnd;
        m_pkLinesData->GetColors()[m_usPosition]       = kColor;

        m_pkLinesData->GetColors()[m_usPosition + 1] = m_pkLinesData->GetColors()[m_usPosition];
        m_pkLinesData->GetFlags() [m_usPosition]     = true;
        m_pkLinesData->GetFlags() [m_usPosition + 1] = false;

        m_usPosition += 2;

        m_pkLinesData->MarkAsChanged( NiGeometryData::VERTEX_MASK | NiGeometryData::COLOR_MASK );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void Line3DDrawer::CreateLines()
{
    m_usNumVertices = USHRT_MAX;
    NiPoint3* pkVerts  = NiNew NiPoint3[ m_usNumVertices ];
    NiColorA* pkColors = NiNew NiColorA[ m_usNumVertices ];
    NiBool* pbConnects = (NiBool*)NiMalloc( sizeof(NiBool)*m_usNumVertices );
    assert( pkVerts && pkColors && pbConnects );

    for ( unsigned int ui = 0; ui < m_usNumVertices; ++ui )
    {
        pkVerts[ui]    = NiPoint3::ZERO;
        pbConnects[ui] = false;
    }

    m_pkLinesData = NiNew NiLinesData( m_usNumVertices, pkVerts, pkColors, NULL, 0, 
                                       NiGeometryData::NBT_METHOD_NONE, pbConnects );
    m_spLines = NiNew NiLines( m_pkLinesData );
    m_usPosition = 0;

    NiVertexColorProperty* pkVColorProp = NiNew NiVertexColorProperty;
    if ( pkVColorProp )
    {
        pkVColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
        pkVColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
        m_spLines->AttachProperty( pkVColorProp );
    }

    m_spLines->Update( 0.0f );
    m_spLines->UpdateProperties();
    m_spLines->UpdateEffects();
}

//-----------------------------------------------------------------------------------

void Line3DDrawer::DeleteLines()
{
    if ( m_spLines )
        m_spLines = 0;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Renderer

} //namespace NewWare
