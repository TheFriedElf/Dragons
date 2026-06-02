
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionCuller.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONCULLER_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONCULLER_H__

#include <NiTMap.h>

#include "OcclusionQueryD3D.h"
#include "OcclusionDepthTestD3D.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{

class OcclusionRenderStateD3D;
class OcclusionDepthTestD3D;
class OcclusionSorter;


class OcclusionCuller
{
public:
    class OcclusionQuerys
    {
    public:
        OcclusionQuerys();
        ~OcclusionQuerys();

        bool CreateQueryObject( NiGeometry* pkKey )
        {
            assert( pkKey );
            OcclusionQueryD3D* pkQuery = new OcclusionQueryD3D;
            if ( pkQuery && pkQuery->IsValid() )
            {
                m_kQueryObjectPool.SetAt( pkKey, pkQuery );
                return true;
            }
            return false;
        }
        OcclusionQueryD3D* GetQueryObject( NiGeometry* pkKey ) const
        {
            assert( pkKey );
            OcclusionQueryD3D* pkQuery = NULL;
            m_kQueryObjectPool.GetAt( pkKey, pkQuery );
            return pkQuery;
        }
        void DeleteQueryObject( NiGeometry* pkKey )
        {
            assert( pkKey );
            OcclusionQueryD3D* pkQuery = NULL;
            if ( m_kQueryObjectPool.GetAt(pkKey, pkQuery) )
            {
                pkQuery->Release();
                delete pkQuery;
                m_kQueryObjectPool.RemoveAt( pkKey );
            }
        }
        void DeleteAllQueryObject()
        {
            OcclusionQueryD3D* pkQuery = NULL;
            NiGeometry* pkKey;

            NiTMapIterator iter = m_kQueryObjectPool.GetFirstPos();
            while ( iter )
            {
                m_kQueryObjectPool.GetNext( iter, pkKey, pkQuery );
                assert( pkQuery && pkQuery->IsValid() );
                pkQuery->Release();
                assert( !pkQuery->IsValid() );
                delete pkQuery;
            }
            m_kQueryObjectPool.RemoveAll();
        }

    public:
        // Internal use only!
        void HandleDeviceReset( bool bBeforeReset );

    private:
        NiTMap< NiGeometry*, OcclusionQueryD3D* > m_kQueryObjectPool;
    };
private:
    struct QueryGetResult
    {
        int iVisiblePixels;
        bool bAvailability;
    };


public:
    OcclusionCuller();
    ~OcclusionCuller();


    void Reset() { m_pkQuerys->DeleteAllQueryObject(); m_kOccludeEList.RemoveAll(); };

    unsigned int DoCull( NiCamera const* pkCamera, NiVisibleArray& kArray, bool const bWaitForResult = true );

    NiVisibleArray& GetResultPVSGeometry() { return m_kPVSGeometry; };

    OcclusionQuerys* GetOcclusionQuerys() const { return m_pkQuerys; };
    NiSingleShaderMaterial* GetDepthMaterial() const { return m_pkDepthTester->GetDepthMaterial(); };


protected:
    void SortFrontToBack( NiCamera const* pkCamera, NiVisibleArray& kArray );
    void QueryBegin();
    void QueryEnd();
    void FlushDepthDrawBatchs();
    unsigned int QueryGetResults( bool const bWaitForResult );
    void QueryOccludees();


private:
    void OcclusionQueryDrawTestDepth( NiGeometry* pkObject );
    void OcclusionQueryGetResult( bool bWaitForResult, NiGeometry* pkKey, QueryGetResult& kQGResult );


private:
    OcclusionQuerys* m_pkQuerys;
    OcclusionRenderStateD3D* m_pkRenderStater;
    OcclusionDepthTestD3D* m_pkDepthTester;
    OcclusionSorter* m_pkSorter;

    NiTPointerList<NiGeometry*> m_kOccludeEList;

    NiVisibleArray m_kPVSGeometry;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONCULLER_H__
