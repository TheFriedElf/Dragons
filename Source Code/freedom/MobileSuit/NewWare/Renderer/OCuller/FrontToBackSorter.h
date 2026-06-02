
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : FrontToBackSorter.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_FRONTTOBACKSORTER_H__
#define _RENDERER_OCULLER_FRONTTOBACKSORTER_H__

#include <NiCamera.h>
#include <NiGeometry.h>

#include "../Kernel/RenderGroup.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{


class FrontToBackSorter
{
public:
    FrontToBackSorter();
    virtual ~FrontToBackSorter();


    void Begin( NiCamera const* pkCamera );
    void AddObject( NiGeometry& kGeometry );
    void AddObjectArray( NiVisibleArray& kArray );
    void End();
    void Clear();

    bool IsSorting() const { return m_pkCamera? true: false; };


protected:
    virtual bool ResolveType( NiGeometry& kGeometry, NiCamera const* pkCamera ) = 0;
    virtual void DoClear() = 0;
    virtual void DoAccumulate( NiGeometry* pkItem ) = 0;


private:
    void Sort();
    void SortByDepth( Kernel::SortItems& kItems );

    void SortObjectsByDepth( int l, int r, Kernel::SortItems& kItems );
    float ChoosePivot( int l, int r, float const* pfDepths ) const;


private:
    NiCamera const* m_pkCamera;

    Kernel::SortItems m_kAllItems;
};


} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_FRONTTOBACKSORTER_H__
