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

#ifndef OCCLUDER_H
#define OCCLUDER_H

#include <NiCamera.h>

class Occluder : public NiObject
{
    NiDeclareRTTI;

	int	m_iGroup;
	bool	m_bVisible;

    Occluder();
    virtual ~Occluder();

public:
    virtual void Update(NiCamera* pkCamera) = 0;
    virtual bool IsOccluded(NiCamera* pkCamera, const NiBound& kBound) = 0;

	int	GetGroup()	{	return	m_iGroup;	}
	void	SetGroup(int iGroup)	{	m_iGroup = iGroup;	}

	virtual	bool	Merge(NiGeometry *pkGeom);

	bool	GetVisible()	{	return	m_bVisible;	}
	void	SetVisible(bool bVisible)	{	m_bVisible = bVisible;	}

};

NiSmartPointer(Occluder);

#endif // OCCLUDER_H 
