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
#include <NiNode.h>
#include <NiEntity.h>

#include "NiOcclusionCuller.h"
#include "PlanarOccluder.h"

bool	NiOcclusionCuller::m_bEnableOcclusionCulling = true;

//---------------------------------------------------------------------------
NiOcclusionCuller::NiOcclusionCuller() : NiCullingProcess(0, false)
{
    m_kSceneRootPointerName = "Scene Root Pointer";
}
//---------------------------------------------------------------------------
NiOcclusionCuller::~NiOcclusionCuller()
{
    RemoveAllOcclusionGeometry();
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::Process(const NiCamera* pkCamera, NiAVObject* pkScene,
    NiVisibleArray* pkVisibleSet)
{
	if(GetOcclusionCulling())
	{
		// Determine which side of the occlusion planes the camera is on for
		// fast dismissal during occlusion testing.
		unsigned int uiNumOccluders = m_kOccluders.GetEffectiveSize();
		for (unsigned int ui = 0; ui < uiNumOccluders; ui++)
		{
			Occluder* pkRecord = m_kOccluders.GetAt(ui);
			pkRecord->Update((NiCamera*)pkCamera);
		}
	}

    NiCullingProcess::Process(pkCamera, pkScene, pkVisibleSet);
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::Process(NiAVObject* pkObject)
{
    // Cull by occlusion objects first. Then proceed with normal processing
    // if object is not culled.
	if(GetOcclusionCulling())
	{
		unsigned int uiNumOccluders = m_kOccluders.GetEffectiveSize();
		// Test each anti-portal
		for (unsigned int ui = 0; ui < uiNumOccluders; ui++)
		{
			Occluder* pkRecord = m_kOccluders.GetAt(ui);
			if (pkRecord->IsOccluded((NiCamera*)m_pkCamera,
				pkObject->GetWorldBound()))
			{
				return;
			}
		}   
	}

    NiCullingProcess::Process(pkObject);
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::RecursiveFindOcclusionGeometry(NiEntityInterface* 
    pkEntity)
{
    if (!pkEntity)
    {
        return;
    }

    unsigned int uiSceneRootCount;
    if (pkEntity->GetElementCount(m_kSceneRootPointerName, uiSceneRootCount))
    {
        for (unsigned int ui = 0; ui < uiSceneRootCount; ui++)
        {
            NiObject* pkObject;
            NiBool bSuccess = pkEntity->GetPropertyData(
                m_kSceneRootPointerName, pkObject, ui);
            assert(bSuccess);

            NiAVObject* pkSceneRoot = NiDynamicCast(NiAVObject, pkObject);
            if (pkSceneRoot)
            {
                FindOcclusionGeometry(pkSceneRoot, false);
            }
        }
    }
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::FindOcclusionGeometry(NiScene* pkEntityScene)
{
    for (unsigned int ui = 0; ui < pkEntityScene->GetEntityCount(); ui++)
    {
        RecursiveFindOcclusionGeometry(pkEntityScene->GetEntityAt(ui));
    }
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::FindOcclusionGeometry(NiAVObject* pkAVObject, 
    bool bMarkedParent,int iGroup)
{
	if (pkAVObject == NULL)
		return;

	NiIntegerExtraData *pkGroupData = (NiIntegerExtraData*)pkAVObject->GetExtraData("OCCLUSION_CULLING_GROUP");
	if(pkGroupData)
	{
		iGroup = pkGroupData->GetValue();
	}

    if (NiIsKindOf(NiGeometry, pkAVObject))
    {
        if (bMarkedParent || pkAVObject->GetName()=="PlanarOccluder")
        {
			Occluder* pkRecord = FindOccluder(iGroup);
			if(pkRecord)
			{
				if(pkRecord->Merge((NiGeometry*)pkAVObject))
				{
					return;	//	return if the merging have successed.
				}
			}
			
			pkRecord = NiNew PlanarOccluder(
				(NiGeometry*)pkAVObject);
			m_kOccluders.AddFirstEmpty(pkRecord);

			pkRecord->SetGroup(iGroup);

			NiGeometry* pkGeom = (NiGeometry*)pkAVObject;
			pkGeom->GetModelData()->SetKeepFlags(NiGeometryData::KEEP_ALL);
        }
    }
    else if (NiIsKindOf(NiNode, pkAVObject))
    {
        bool bMarked = false;
        if (pkAVObject->GetName()=="Occluder")
            bMarked = true;

        NiNode* pkNode = (NiNode*)pkAVObject;

        unsigned int uiChildren = pkNode->GetArrayCount();
        for (unsigned int ui = 0; ui < uiChildren; ui++)
        {
            NiAVObject* pkChild = pkNode->GetAt(ui);
            if (pkChild)
                FindOcclusionGeometry(pkChild, bMarked,iGroup);
        }
    }
}
Occluder*	NiOcclusionCuller::FindOccluder(int iGroupNum)
{
	if(iGroupNum<0)
	{
		return	NULL;
	}

    unsigned int uiNumOccluders = m_kOccluders.GetEffectiveSize();
    // Test each anti-portal
    for (unsigned int ui = 0; ui < uiNumOccluders; ui++)
    {
        Occluder* pkRecord = m_kOccluders.GetAt(ui);
		if(pkRecord->GetGroup() == iGroupNum)
		{
			return	pkRecord;
		}
    }  

	return	NULL;
}
//---------------------------------------------------------------------------
void NiOcclusionCuller::RemoveAllOcclusionGeometry()
{
    for (unsigned int ui = 0; ui < m_kOccluders.GetSize(); ui++)
        NiDelete m_kOccluders.GetAt(ui);
    m_kOccluders.RemoveAll();
}
//---------------------------------------------------------------------------
