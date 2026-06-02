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

#ifndef MASTERPROPERTYACCUMULATOR
#define MASTERPROPERTYACCUMULATOR

#include <NiAccumulator.h>
#include <NiProperty.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// This object takes a list of properties and pushes them into the 
// renderstate immediately before rendering.  Using this sorter, it is 
// possible to overide any property class for all drawn objects with the 
// desired global property
class MasterPropertyAccumulator : public NiAccumulator
{
    NiDeclareRTTI;
    NiDeclareClone(MasterPropertyAccumulator);

    // We do not declare or use streaming - note that this accumulator
    // will not be capable of streaming - see the Sorting sample for an 
    // example of a streamable Accumulator
    //
    // NiDeclareStream;

public:
    // creation and destruction
    MasterPropertyAccumulator () { /* */ }
    virtual ~MasterPropertyAccumulator ();

    virtual void RegisterObjectArray (NiVisibleArray& kArray);
    virtual void FinishAccumulating (); 

    NiPropertyList m_propertyList;

protected:
    NiTPointerList<NiGeometry*> m_kObjects;
};

#endif
