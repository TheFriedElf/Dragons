
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionSorter.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONSORTER_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONSORTER_H__

#include <NiCamera.h>
#include <NiGeometry.h>

#include "../FrontToBackSorter.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{

typedef NiTPointerList<NiGeometry*> NiSortedObjectList;


class OcclusionSorter : public FrontToBackSorter
{
public:
    OcclusionSorter();
    virtual ~OcclusionSorter();


    NiSortedObjectList const& GetOccludeRList() const { assert( IsSorting() == false ); return m_kOccludeRList; };
    NiSortedObjectList const& GetOccludeEList() const { assert( IsSorting() == false ); return m_kOccludeEList; };


protected:
    virtual bool ResolveType( NiGeometry& kGeometry, NiCamera const* pkCamera );
    virtual void DoClear();
    virtual void DoAccumulate( NiGeometry* pkItem );


private:
    NiSortedObjectList m_kOccludeRList;
    NiSortedObjectList m_kOccludeEList;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONSORTER_H__
