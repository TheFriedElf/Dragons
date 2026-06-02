
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : FrontToBackSorter.cpp
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
#include "FrontToBackSorter.h"

#include <NiRenderer.h>


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

FrontToBackSorter::FrontToBackSorter()
    : m_pkCamera(NULL)
{
    /* Nothing */
}

//-----------------------------------------------------------------------------------

FrontToBackSorter::~FrontToBackSorter()
{
    /* Nothing */
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void FrontToBackSorter::Begin( NiCamera const* pkCamera )
{
    assert( pkCamera && IsSorting() == false );

    Clear();

    m_pkCamera = pkCamera;
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::AddObject( NiGeometry& kGeometry )
{
    assert( IsSorting() );

    if ( ResolveType(kGeometry, m_pkCamera) == false )
        return;

    m_kAllItems.kItemList.AddTail( &kGeometry );
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::AddObjectArray( NiVisibleArray& kArray )
{
    assert( IsSorting() );

    unsigned int const uiQuantity = kArray.GetCount();
    for ( unsigned int ui = 0; ui < uiQuantity; ++ui )
    {
        AddObject( kArray.GetAt(ui) );
    }
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::End()
{
    assert( IsSorting() );

    Sort();

    m_pkCamera = NULL;
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::Clear()
{
    assert( IsSorting() == false );

    m_kAllItems.Clear();

    DoClear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Quick sorting(ascending sort) functions stuff

void FrontToBackSorter::Sort()
{
    SortByDepth( m_kAllItems );

    int iCurrItem = m_kAllItems.iNumItems;
    NiGeometry* pkItem = (0 != iCurrItem)? m_kAllItems.ppkItems[--iCurrItem]: NULL;
    while ( pkItem )
    {
        DoAccumulate( pkItem );

        pkItem = (0 != iCurrItem)? m_kAllItems.ppkItems[--iCurrItem]: NULL;
    }
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::SortByDepth( Kernel::SortItems& kItems )
{
    kItems.iNumItems = kItems.kItemList.GetSize();
    if ( 0 == kItems.iNumItems )
        return;

    if ( kItems.iNumItems > kItems.iMaxItems )
    {
        kItems.iMaxItems = kItems.iNumItems;

        NiFree( kItems.ppkItems );
        kItems.ppkItems = NiAlloc( NiGeometry*, kItems.iMaxItems );
        assert( kItems.ppkItems );

        NiFree( kItems.pfDepths );
        kItems.pfDepths = NiAlloc( float, kItems.iMaxItems );
        assert( kItems.pfDepths );
    }

    NiTListIterator kPos = kItems.kItemList.GetHeadPos();

    assert( m_pkCamera );
    NiPoint3 kViewDir = m_pkCamera->GetWorldDirection();

    for ( int i = 0; i < kItems.iNumItems; ++i )
    {
        kItems.ppkItems[i] = kItems.kItemList.GetNext( kPos );
        kItems.pfDepths[i] = kItems.ppkItems[i]->GetWorldBound().GetCenter() * kViewDir;
    }

    SortObjectsByDepth( 0, kItems.iNumItems-1, kItems );
}

//-----------------------------------------------------------------------------------

void FrontToBackSorter::SortObjectsByDepth( int l, int r, Kernel::SortItems& kItems )
{
    if ( r > l )
    {
        int i, j;

        i = l - 1;
        j = r + 1;
        float fPivot = ChoosePivot( l, r, kItems.pfDepths );

        for ( ;; )
        {
            do 
            {
                --j;
            } while ( fPivot > kItems.pfDepths[j] );

            do
            {
                ++i;
            } while ( kItems.pfDepths[i] > fPivot );

            if ( j > i )
            {
                NiGeometry* pkObjTemp = kItems.ppkItems[i];
                kItems.ppkItems[i] = kItems.ppkItems[j];
                kItems.ppkItems[j] = pkObjTemp;
                float fTemp = kItems.pfDepths[i];
                kItems.pfDepths[i] = kItems.pfDepths[j];
                kItems.pfDepths[j] = fTemp;
            }
            else
            {
                break;
            }
        }

        if ( j == r )
        {
            SortObjectsByDepth( l, j-1, kItems );
        }
        else
        {
            SortObjectsByDepth( l, j, kItems );
            SortObjectsByDepth( j+1, r, kItems );
        }
    }
}

//-----------------------------------------------------------------------------------

float FrontToBackSorter::ChoosePivot( int l, int r, float const* pfDepths ) const
{
    // Check the first, middle, and last element. Choose the one which falls
    // between the other two. This has a good chance of discouraging 
    // quadratic behavior from qsort.
    // In the case when all three are equal, this code chooses the middle
    // element, which will prevent quadratic behavior for a list with 
    // all elements equal.

    int m = (l + r) >> 1;

    if ( pfDepths[l] > pfDepths[m] )
    {
        if ( pfDepths[m] > pfDepths[r] )
        {
            return pfDepths[m];
        }
        else
        {
            if ( pfDepths[l] > pfDepths[r] )
                return pfDepths[r];
            else
                return pfDepths[l];
        }
    }
    else
    {
        if ( pfDepths[l] > pfDepths[r] )
        {
            return pfDepths[l];
        }
        else
        {
            if ( pfDepths[m] > pfDepths[r] )
                return pfDepths[r];
            else
                return pfDepths[m];
        }
    }
}

// Quick sorting(ascending sort) functions stuff
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace OCuller

} //namespace Renderer

} //namespace NewWare
