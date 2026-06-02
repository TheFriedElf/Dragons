
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Line3DDrawer.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_LINE3DDRAWER_H__
#define _RENDERER_LINE3DDRAWER_H__

#include <NiLines.h>


namespace NewWare
{

namespace Renderer
{


class Line3DDrawer
{
public:
    Line3DDrawer();
    ~Line3DDrawer();


    void Init();
    void Draw( NiPoint3 const& kStart, NiPoint3 const& kEnd, NiColorA const& kColor = NiColorA::WHITE );

    NiLines* GetLines() const { return m_spLines; };

    void ToggleDrawable() { m_bDrawable = !m_bDrawable; };
    bool IsDrawable() const { return m_bDrawable; };


private:
    void CreateLines();
    void DeleteLines();


private:
    bool m_bDrawable;

    unsigned short m_usPosition;
    unsigned short m_usNumVertices;
    NiLinesData* m_pkLinesData;
    NiLinesPtr m_spLines;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_LINE3DDRAWER_H__
