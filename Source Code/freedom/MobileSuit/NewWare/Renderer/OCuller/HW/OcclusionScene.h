
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionScene.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONSCENE_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONSCENE_H__

#include <NiTPointerList.h>
#include <NiTMap.h>
#include <NiScene.h>


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{

class OcclusionCuller;


class OcclusionScene : public Tools::Noncopyable
{
public:
    explicit OcclusionScene( OcclusionCuller* pkCuller );
    ~OcclusionScene();


    void AddScene( NiAVObject* pkScene );
    void AddScene( NiScene* pkScene, NiFixedString& kRootName );
    void RemoveScene( NiAVObject* pkScene );
    void RemoveScene( NiScene* pkScene, NiFixedString& kRootName );
    void RemoveAllScenes();


private:
    void AddObjects( NiAVObject* pkAVObject );
    void ReleaseObjects( NiAVObject* pkAVObject );


private:
    OcclusionCuller* m_pkOccCuller;

    NiTMap< NiScene*, NiFixedString > m_kEntityScenes;
    NiTPointerList< NiAVObjectPtr > m_kAVObjectScenes;
};


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONSCENE_H__
