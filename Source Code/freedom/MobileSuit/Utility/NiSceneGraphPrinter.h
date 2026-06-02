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

#ifndef NISCENEGRAPHPRINTER_H
#define NISCENEGRAPHPRINTER_H

#ifdef USE_NISCENEGRAPH

class NiObject;
#include "NiGeometry.h"
#include <map>
#include <iostream>
#include <fstream>

typedef struct _GeomInfo {
	void* address;
	char name[256];
	unsigned int vertexCount;
	unsigned int triCount;
	bool skinned;
	bool triShape;
	unsigned int count;
} GeomInfo;

typedef std::unordered_map<NiGeometryData*, GeomInfo> GeometryMap;

class NiSceneGraphPrinter
{
public:
	static void Process(NiNode* pSceneRootNode, char* pFileName);

    static void PrintID(NiObject* pkObject, unsigned short usIndent);
    static void PrintAppCulled(NiObject* pkObject, unsigned short usIndent);
    static void PrintTransforms(NiObject* pkObject, unsigned short usIndent);
    static void PrintBoundingSpheres(NiObject* pkObject,
        unsigned short usIndent);
    static void PrintProperties(NiObject* pkObject, unsigned short usIndent);
    static void PrintExtraData(NiObject* pkObject, unsigned short usIndent);
    static void PrintGeometryData(NiObject* pkObject,
        unsigned short usIndent);
    static void RecursivePrint(NiObject* pkObject, unsigned short usIndent);
    static void PrintTriSummary();
    static void SummaryPrint();
    static void OpenLog();
    static void CloseLog();
    static void InitStats();

    static bool ms_bShowAppCulled;
    static bool ms_bDropAppCulled;
    static bool ms_bShowBoundingSpheres;
    static bool ms_bShowMemoryAddress;
    static bool ms_bShowProperties;
    static bool ms_bShowTransforms;
    static bool ms_bShowExtraData;
    static bool ms_bShowGeometryData;
    static unsigned short ms_usTabStop;
    static unsigned int ms_uiObjectCount;
    static unsigned int ms_uiDepth;

    static unsigned int ms_uiTriShapeCount;
    static unsigned int ms_uiTriShape_TriCount;
    static unsigned int ms_uiTriShape_LowTriCount;
    static unsigned int ms_uiTriShape_HighTriCount;
    static unsigned int ms_uiTriShape_ZeroTriCount;
    static unsigned int ms_uiTriStripsCount;
    static unsigned int ms_uiTriStrips_TriCount;
    static unsigned int ms_uiTriStrips_LowTriCount;
    static unsigned int ms_uiTriStrips_HighTriCount;
    static unsigned int ms_uiTriStrips_ZeroTriCount;
    static std::ostream* ms_pkOstr;

private:
    static bool ms_bFirstTriShape;
    static bool ms_bFirstTriStrips;
};

#endif

#endif // #ifndef NISCENEGRAPHPRINTER_H
