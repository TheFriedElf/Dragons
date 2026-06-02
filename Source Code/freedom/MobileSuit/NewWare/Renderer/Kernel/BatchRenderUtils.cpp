
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : BatchRenderUtils.cpp
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
#include "BatchRenderUtils.h"

#include "RenderGroup.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


namespace BatchRenderUtils
{

/////////////////////////////////////////////////////////////////////////////////////////////
//

void DrawUnSortedRenderGroup( NiRenderer* pkRenderer, RenderGroup const& kGroup, 
                              RenderStateBlockCache::RenderStateBlock const& kStateBlock )
{
    assert( pkRenderer );

    pkRenderer->BeginBatch( kStateBlock.spPropertyState, kStateBlock.spEffectState );

    NiSortedObjectList const& kItemList = kGroup.kSortItems.kItemList;
    NiGeometry* pkItem;

    // UnSorted rendering!
    NiTListIterator kPos = kItemList.GetHeadPos();
    while ( kPos )
    {
        pkItem = kItemList.GetNext( kPos );

        if ( NiIsExactKindOf(NiTriShape, pkItem) )
        {
            pkRenderer->BatchRenderShape( static_cast<NiTriShape*>(pkItem) );
        }
        else if ( NiIsExactKindOf(NiTriStrips, pkItem) )
        {
            pkRenderer->BatchRenderStrips( static_cast<NiTriStrips*>(pkItem) );
        }
        else
        {
            pkItem->RenderImmediate( pkRenderer );
        }
    }

    pkRenderer->EndBatch();
}

//-----------------------------------------------------------------------------------

void DrawSortedRenderGroup( NiRenderer* pkRenderer, RenderGroup const& kGroup, 
                            RenderStateBlockCache::RenderStateBlock const& kStateBlock )
{
    assert( pkRenderer );

    pkRenderer->BeginBatch( kStateBlock.spPropertyState, kStateBlock.spEffectState );

    SortItems const& kSortItems = kGroup.kSortItems;
    NiGeometry* pkItem;

    // Front to back sorted rendering!
    for ( int iIndex = 0; iIndex < kSortItems.iNumItems; ++iIndex )
    {
        pkItem = kSortItems.ppkItems[iIndex];

        if ( NiIsExactKindOf(NiTriShape, pkItem) )
        {
            pkRenderer->BatchRenderShape( static_cast<NiTriShape*>(pkItem) );
        }
        else if ( NiIsExactKindOf(NiTriStrips, pkItem) )
        {
            pkRenderer->BatchRenderStrips( static_cast<NiTriStrips*>(pkItem) );
        }
        else
        {
            pkItem->RenderImmediate( pkRenderer );
        }
    }

    pkRenderer->EndBatch();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////

} //namespace BatchRenderUtils


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare
