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

#ifndef NIOCCLUSIONCULLER_H
#define NIOCCLUSIONCULLER_H

#include <NiCullingProcess.h>
#include <NiPlane.h>
#include <NiPoint3.h>
#include <NiTArray.h>
#include <NiTransform.h>

#include "Occluder.h"

class NiGeometry;
class NiScene;
class NiEntityInterface;

class NiOcclusionCuller : public NiCullingProcess
{
public:
    NiOcclusionCuller();
    virtual ~NiOcclusionCuller();

    virtual void Process(const NiCamera* pkCamera, NiAVObject* pkScene, 
        NiVisibleArray* pkVisibleSet);

    void FindOcclusionGeometry(NiScene* pkEntityScene);
    void FindOcclusionGeometry(NiAVObject* pkAVObject, bool bMarkedParent,int iGroup=-1);
    void RemoveAllOcclusionGeometry();

protected:
    void RecursiveFindOcclusionGeometry(NiEntityInterface* pkEntity);

    virtual void Process(NiAVObject* pkObject);

	Occluder*	FindOccluder(int iGroupNum);

public:

	static	void	SetOcclusionCulling(bool bEnable)	{	m_bEnableOcclusionCulling = bEnable;	}
	static	bool	GetOcclusionCulling()	{	return	m_bEnableOcclusionCulling;	}

protected:

    NiTPrimitiveArray<Occluder*> m_kOccluders;

    NiFixedString m_kSceneRootPointerName;

	static	bool	m_bEnableOcclusionCulling;
	
};

NiSmartPointer(NiOcclusionGeometry);

#endif
