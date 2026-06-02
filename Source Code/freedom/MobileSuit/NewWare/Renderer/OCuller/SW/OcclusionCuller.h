
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

#ifndef _RENDERER_OCULLER_SW_OCCLUSIONCULLER_H__
#define _RENDERER_OCULLER_SW_OCCLUSIONCULLER_H__

#include <NiCullingProcess.h>
#include <NiPlane.h>
#include <NiPoint3.h>
#include <NiTArray.h>
#include <NiTransform.h>

#include "Occluder.h"

class NiGeometry;
class NiScene;
class NiEntityInterface;


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace SW
{


class OcclusionCuller : public NiCullingProcess
{
public:
    OcclusionCuller();
    virtual ~OcclusionCuller();


    void FindOcclusionGeometry( NiScene* pkEntityScene );
    void FindOcclusionGeometry( NiAVObject* pkAVObject, bool bMarkedParent );
    void RemoveAllOcclusionGeometry();


protected:
    virtual void Process( NiCamera const* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet );

    void RecursiveFindOcclusionGeometry( NiEntityInterface* pkEntity );

    virtual void Process( NiAVObject* pkObject );


protected:
    NiTPrimitiveArray<Occluder*> m_kOccluders;

    NiFixedString m_kSceneRootPointerName;
};

NiSmartPointer(OcclusionGeometry);


} //namespace SW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_SW_OCCLUSIONCULLER_H__
