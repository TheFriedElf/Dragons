
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderGroup.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#include "stdafx.h"
#include "RenderGroup.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

SortItems::SortItems()
    : iNumItems(0), 
      iMaxItems(0), 
      ppkItems(NULL), 
      pfDepths(NULL)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

SortItems::~SortItems()
{
    NiFree( ppkItems );
    NiFree( pfDepths );

    Clear();
}

//-----------------------------------------------------------------------------------

void SortItems::Clear()
{
    // DO NOT delete the NiGeometries!
    while ( kItemList.GetSize() )
        kItemList.RemoveHead();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

const int RenderGroup::NUMBER_ZERO = (std::numeric_limits<int>::min)();

//-----------------------------------------------------------------------------------

RenderGroup::RenderGroup()
    : iNumber(NUMBER_ZERO), 
      dwTextureHashKey(0)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

RenderGroup::~RenderGroup()
{
    Clear();
}

//-----------------------------------------------------------------------------------

void RenderGroup::Clear()
{
    iNumber = NUMBER_ZERO;
    dwTextureHashKey = 0;
    kSortItems.Clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
