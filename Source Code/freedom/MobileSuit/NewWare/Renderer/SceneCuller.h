
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneCuller.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _RENDERER_SCENECULLER_H__
#define _RENDERER_SCENECULLER_H__

#include "OCuller/HW/OcclusionCuller.h"


namespace NewWare
{

namespace Renderer
{

namespace OCuller { 
    namespace HW { 
        class OcclusionScene;
    } //namespace HW
} //namespace OCuller


class SceneCuller
{
public:
    SceneCuller();
    ~SceneCuller();


    void Create( bool bOcclusionCullMode );
    void Build( NiAVObject* pkAVObject );

    void Begin( NiCamera const* pkCamera );
    unsigned int Commit( NiAVObject* pkScene, bool bResetArray );
    void AddCommit( NiGeometry& kObject )
        { assert( IsCulling() ); m_kFrustumVisibleArray.Add( kObject ); };
    void AddCommit( NiVisibleArray& kVisibleArray );
    void End();

    NiCullingProcess* GetFrustumCuller() const { return m_pkFrustumCuller; };
    NiCullingProcess& GetFrustumCuller() { return *m_pkFrustumCuller; };

    NiVisibleArray& GetPVSGeometry()
    {
        assert( IsCulling() == false );
        if ( IsOcclusionCullMode() )
            return m_pkOcclusionCuller->GetResultPVSGeometry();
        else
            return m_kFrustumVisibleArray;
    }

    bool IsOcclusionCullMode() const { return m_bOcclusionCullMode; };


protected:
    bool IsCulling() const { return m_pkCamera? true: false; };


private:
    NiCamera const* m_pkCamera;
    NiCullingProcess* m_pkFrustumCuller;
    NiVisibleArray m_kFrustumVisibleArray;

    bool m_bOcclusionCullMode;
    OCuller::HW::OcclusionCuller* m_pkOcclusionCuller;
    OCuller::HW::OcclusionScene* m_pkOcclusionScene;
};


} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_SCENECULLER_H__
