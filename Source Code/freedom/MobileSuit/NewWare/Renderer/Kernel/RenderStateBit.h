
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderStateBit.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERSTATEBIT_H__
#define _RENDERER_KERNEL_RENDERSTATEBIT_H__

#include "../../Tools/BitFlag.h"


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


/** Render-State Bit diagram (32Bit type)
      1 Bit - Skinning flag
      2 Bit - Vertex Color flag
      3 Bit - Normals flag
      9~11Bit - Texture sets (3Bit - 7)
     11~32Bit - reserve!!!!
*/
namespace RenderStateBit
{
    typedef DWORD StateBitType;

    enum
    {
        SKINNING_MASK       = (1 << 0), 
        VERTEXCOLOR_MASK    = (1 << 1), 
        NORMALS_MASK        = (1 << 2), 

        TEXTURESETS_SHIFT   = 8, 
        TEXTURESETS_BITS    = 0x07, 
        TEXTURESETS_MASK    = (TEXTURESETS_BITS << TEXTURESETS_SHIFT), 
    };

    inline bool GetSkinning( Tools::BitFlag<StateBitType> const& kFlag ) { return kFlag.GetBit( SKINNING_MASK ); };
    inline void SetSkinning( Tools::BitFlag<StateBitType>& kFlag, bool bSkinning ) { (kFlag.SetBit)( bSkinning, SKINNING_MASK ); };

    inline bool GetVertexColor( Tools::BitFlag<StateBitType> const& kFlag ) { return kFlag.GetBit( VERTEXCOLOR_MASK ); };
    inline void SetVertexColor( Tools::BitFlag<StateBitType>& kFlag, bool bVColor ) { (kFlag.SetBit)( bVColor, VERTEXCOLOR_MASK ); };

    inline bool GetNormals( Tools::BitFlag<StateBitType> const& kFlag ) { return kFlag.GetBit( NORMALS_MASK ); };
    inline void SetNormals( Tools::BitFlag<StateBitType>& kFlag, bool bNormals ) { (kFlag.SetBit)( bNormals, NORMALS_MASK ); };

    inline WORD GetTextureSets( Tools::BitFlag<StateBitType> const& kFlag ) { return (WORD)kFlag.GetField( TEXTURESETS_MASK, TEXTURESETS_SHIFT ); };
    inline void SetTextureSets( Tools::BitFlag<StateBitType>& kFlag, WORD sets ) { 
        assert( sets <= (WORD)TEXTURESETS_BITS );
        kFlag.SetField( ((BYTE)sets)&TEXTURESETS_BITS, TEXTURESETS_MASK, TEXTURESETS_SHIFT );
    }

} //namespace RenderStateBit


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERSTATEBIT_H__
