
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderGroup.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERGROUP_H__
#define _RENDERER_KERNEL_RENDERGROUP_H__


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


struct SortItems
{
    SortItems();
    ~SortItems();

    void Clear();

    NiSortedObjectList kItemList;

    int iNumItems;
    int iMaxItems;
    NiGeometry** ppkItems;
    float* pfDepths;
};


struct RenderGroup
{
    RenderGroup();
    virtual ~RenderGroup();

    void Clear();

    int iNumber; // Alpha: Group number, Opaque: State number
    DWORD dwTextureHashKey; // Opaque texture hash-key (texture allocated memory address)
    SortItems kSortItems;

    static inline bool QSortMethod( RenderGroup const* a, RenderGroup const* b )
    {
        return a->dwTextureHashKey > b->dwTextureHashKey; // a is greater than the b
    }

    static const int NUMBER_ZERO;
};


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERGROUP_H__
