
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Occluder.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_SW_OCCLUDER_H__
#define _RENDERER_OCULLER_SW_OCCLUDER_H__


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace SW
{


class Occluder : public NiObject
{
    NiDeclareRTTI;

    Occluder();
    virtual ~Occluder();


public:
    virtual void Update( NiCamera* pkCamera ) = 0;
    virtual bool IsOccluded( NiCamera* pkCamera, NiBound const& kBound ) = 0;
};

NiSmartPointer(Occluder);


} //namespace SW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_SW_OCCLUDER_H__
