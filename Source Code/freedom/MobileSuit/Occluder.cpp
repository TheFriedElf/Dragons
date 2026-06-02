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
#include "Occluder.h"

NiImplementRTTI(Occluder, NiObject);

//---------------------------------------------------------------------------
Occluder::Occluder():m_iGroup(-1)
{
}
//---------------------------------------------------------------------------
Occluder::~Occluder()
{
}
//---------------------------------------------------------------------------
bool	Occluder::Merge(NiGeometry *pkGeom)
{
	return	true;
}