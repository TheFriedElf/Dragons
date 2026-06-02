// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2006 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net

#include "stdafx.h"
#include <NiCamera.h>
#include <NiGeometry.h>
#include <NiRenderer.h>

#include "MasterPropertyAccumulator.h"

NiImplementRTTI(MasterPropertyAccumulator, NiAccumulator);
//---------------------------------------------------------------------------
MasterPropertyAccumulator::~MasterPropertyAccumulator ()
{
    while(!m_propertyList.IsEmpty())
        m_propertyList.RemoveHead();
}
//---------------------------------------------------------------------------
void MasterPropertyAccumulator::RegisterObjectArray (NiVisibleArray& kArray)
{
    const unsigned int uiQuantity = kArray.GetCount();
    for (unsigned int i = 0; i < uiQuantity; i++)
        m_kObjects.AddTail(&kArray.GetAt(i));
}
//---------------------------------------------------------------------------
void MasterPropertyAccumulator::FinishAccumulating()
{
    NiRenderer* pkRenderer = NiRenderer::GetRenderer();
    assert(pkRenderer);
    if (!pkRenderer)
        return;

    while (!m_kObjects.IsEmpty())
    {
        NiGeometry* pkGeom = m_kObjects.RemoveHead();

        // push the new properties
        pkGeom->GetPropertyState()->SwapProperties(m_propertyList);

        NiDynamicEffectStatePtr spEffects = pkGeom->GetEffectState();
        pkGeom->SetEffectState(NULL);

        // render - because sorting is now disabled, it will simply draw the
        // object to the backbuffer
        pkGeom->RenderImmediate(pkRenderer);

        pkGeom->SetEffectState(spEffects);

        // pop the properties
        pkGeom->GetPropertyState()->SwapProperties(m_propertyList);
    }

    NiAccumulator::FinishAccumulating();
}
//---------------------------------------------------------------------------
// Cloning support
//---------------------------------------------------------------------------
NiObject* MasterPropertyAccumulator::CreateClone(
        NiCloningProcess& kCloning)
{
    MasterPropertyAccumulator* pDest = NiNew MasterPropertyAccumulator;
    assert(pDest);
    CopyMembers(pDest, kCloning);

    return pDest;
}
//---------------------------------------------------------------------------
void MasterPropertyAccumulator::CopyMembers(
    MasterPropertyAccumulator* pDest, NiCloningProcess& kCloning)
{
    NiAccumulator::CopyMembers(pDest, kCloning);

    // NiMasterPropertyAccumulator has no members that should be cloned...
}
//---------------------------------------------------------------------------
