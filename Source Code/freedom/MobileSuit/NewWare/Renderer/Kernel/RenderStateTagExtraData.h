
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : RenderStateTagExtraData.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_KERNEL_RENDERSTATETAGEXTRADATA_H__
#define _RENDERER_KERNEL_RENDERSTATETAGEXTRADATA_H__


namespace NewWare
{

namespace Renderer
{

namespace Kernel
{


class RenderStateTagExtraData : public NiExtraData
{
    NiDeclareRTTI;

public:
    enum { 
        NOT_USED_RENDERSTATE = -100, 
        RESET_RENDERSTATE = -1, 
    };

public:
    RenderStateTagExtraData();


    bool IsResetRenderStateNumber() const { return RESET_RENDERSTATE == m_iRenderStateNumber; };
    void ResetRenderStateNumber() { m_iRenderStateNumber = RESET_RENDERSTATE; };

    int GetRenderStateNumber() const { return m_iRenderStateNumber; };
    void SetRenderStateNumber( int iNumber ) { m_iRenderStateNumber = iNumber; };

    DWORD GetTextureHashKey() const { return m_dwTextureHashKey; };
    void SetTextureHashKey( DWORD dwHashKey ) { m_dwTextureHashKey = dwHashKey; };

    DWORD GetCreationNumber() const { return m_dwCreationNumber; };
    void SetCreationNumber( DWORD dwNumber ) { m_dwCreationNumber = dwNumber; };


private:
    int m_iRenderStateNumber;
    DWORD m_dwTextureHashKey;
    DWORD m_dwCreationNumber;
};


} //namespace Kernel

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_KERNEL_RENDERSTATETAGEXTRADATA_H__
