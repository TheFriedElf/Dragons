
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionQueryD3D.h
// Description : Wrapper class for platform specific occlusion queries.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONQUERYD3D_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONQUERYD3D_H__


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{


class OcclusionQueryD3D
{
public:
    OcclusionQueryD3D();
    ~OcclusionQueryD3D();


    bool IsValid() const { return m_bValid; };

    void Release()
    {
        if ( m_pkQuery )
        {
            assert( IsValid() );
            m_pkQuery->Release();
            m_pkQuery = NULL;
        }
        m_bValid = false;
    }

    void IssueQueryBegin()
    {
        assert( m_pkQuery && IsValid() );
        m_pkQuery->Issue( D3DISSUE_BEGIN );
    }
    void IssueQueryEnd()
    {
        assert( m_pkQuery && IsValid() );
        m_pkQuery->Issue( D3DISSUE_END );
    }

    void GetQueryResult( bool bWaitForResult, int& iVisiblePixels, bool& bAvailability );

    void Recreate()
    {
        if ( !IsValid() && !m_pkQuery )
            m_bValid = CreateDX9Query();
    }


private:
    bool CreateDX9Query();


private:
    bool m_bValid;
    IDirect3DQuery9* m_pkQuery;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONQUERYD3D_H__
