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
#include <iostream>
#include <fstream>
#include "NiSceneGraphPrinter.h"

#ifdef USE_NISCENEGRAPH

bool NiSceneGraphPrinter::ms_bShowAppCulled = false;
bool NiSceneGraphPrinter::ms_bDropAppCulled = false;
bool NiSceneGraphPrinter::ms_bShowBoundingSpheres = false;
bool NiSceneGraphPrinter::ms_bShowMemoryAddress = false;
bool NiSceneGraphPrinter::ms_bShowProperties = false;
bool NiSceneGraphPrinter::ms_bShowTransforms = false;
bool NiSceneGraphPrinter::ms_bShowExtraData = false;
bool NiSceneGraphPrinter::ms_bShowGeometryData = false;
unsigned short NiSceneGraphPrinter::ms_usTabStop = 2;
unsigned int NiSceneGraphPrinter::ms_uiObjectCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiDepth = 0;

unsigned int NiSceneGraphPrinter::ms_uiTriShapeCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriShape_TriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriShape_LowTriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriShape_HighTriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriShape_ZeroTriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriStripsCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriStrips_TriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriStrips_LowTriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriStrips_HighTriCount = 0;
unsigned int NiSceneGraphPrinter::ms_uiTriStrips_ZeroTriCount = 0;

bool NiSceneGraphPrinter::ms_bFirstTriShape = true;
bool NiSceneGraphPrinter::ms_bFirstTriStrips = true;

std::unordered_map<NiGeometryData*, struct _GeomInfo>* g_pGeomMap = 0;
std::ostream* NiSceneGraphPrinter::ms_pkOstr = &std::cerr;

#define PRINT_INDENT(X) \
    { for (unsigned short i = 0; i < (X); i++) *ms_pkOstr << " "; }

void NiSceneGraphPrinter::Process(NiNode* pSceneRootNode, char* pFileName)
{
	if (pSceneRootNode == NULL)
		return;
	
	std::ofstream file;

	if (pFileName == NULL)
		ms_pkOstr = &std::cerr;
	else
	{
		file.open(pFileName);
		ms_pkOstr = &file;
	}

	int iCount = pSceneRootNode->GetArrayCount();

	if (iCount < 1) 
	{
		return;
	}

	g_pGeomMap = new GeometryMap;
	for (int i = 0; i < iCount; i++)
	{
		NiObject* pkObject = pSceneRootNode->GetAt(i);

		//if (NiIsKindOf(NiAVObject, pkObject))
		//	((NiAVObject*) pkObject)->Update(0.0f);

		NiSceneGraphPrinter::RecursivePrint(pkObject, 0);
	}

	NiSceneGraphPrinter::SummaryPrint();

	if (g_pGeomMap)
	{
		g_pGeomMap->clear();
		SAFE_DELETE(g_pGeomMap);
	}
	if (pFileName != NULL)
		file.close();
	return;
}

//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintID(NiObject* pkObject, unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    PRINT_INDENT(usIndent * ms_usTabStop);

    //
    // Print level
    //
    *ms_pkOstr << usIndent << " - ";

    //
    // Print type
    //
    *ms_pkOstr << pkObject->GetRTTI()->GetName();

    NiObjectNET *pNET = NiDynamicCast(NiObjectNET, pkObject);
    if (pNET != NULL)
    {
        //
        // Print name
        //
        *ms_pkOstr << ":<";
        
        if (pNET->GetName())
            *ms_pkOstr << pNET->GetName();
        
        *ms_pkOstr << ">";
    }

    if (ms_bShowMemoryAddress)
        *ms_pkOstr << " <0x" << pkObject << ">";

    *ms_pkOstr << std::endl;

    ms_uiObjectCount++;
    if (usIndent > ms_uiDepth)
        ms_uiDepth = usIndent;
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintAppCulled(NiObject* pkObject,
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiAVObject, pkObject))
        return;

    if (((NiAVObject*)pkObject)->GetAppCulled())
    {
        PRINT_INDENT((usIndent + 2) * ms_usTabStop);
        *ms_pkOstr << "CULLED (including sub-tree) by Application"
            << std::endl;
    }
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintTransforms(NiObject* pkObject, 
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiAVObject, pkObject))
        return;

    NiAVObject* pAV = (NiAVObject*)pkObject;
    NiPoint3 row;
    unsigned int i;

    for (i = 0; i < 3; i ++)
    {
        PRINT_INDENT((usIndent + 2) * ms_usTabStop);
        pAV->GetRotate().GetRow(i, row);
        *ms_pkOstr << "Rotate: <" << row.x << "," << row.y 
            << "," << row.z << ">" << std::endl;
    }
    
    PRINT_INDENT((usIndent + 2) * ms_usTabStop);
    *ms_pkOstr << "Translate: <" << pAV->GetTranslate().x
        << "," << pAV->GetTranslate().y << "," << pAV->GetTranslate().z
        << ">" << std::endl;

    PRINT_INDENT((usIndent + 2) * ms_usTabStop);
    *ms_pkOstr << "Scale: " << pAV->GetScale() << std::endl;
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintBoundingSpheres(NiObject* pkObject, 
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiAVObject, pkObject))
        return;

    NiBound bound = ((NiAVObject*)pkObject)->GetWorldBound();
    PRINT_INDENT((usIndent + 2) * ms_usTabStop);
    *ms_pkOstr << "World Bound: C <" << bound.GetCenter().x << ","
        << bound.GetCenter().y << "," << bound.GetCenter().z 
        << ">, R " << bound.GetRadius() << std::endl;
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintProperties(NiObject* pkObject, 
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiAVObject, pkObject))
        return;

    NiTListIterator kPos = ((NiAVObject*)pkObject)->GetPropertyList().
        GetHeadPos();

    while (kPos)
    {
        NiProperty* pkProp = ((NiAVObject*)pkObject)->GetPropertyList().
            GetNext(kPos);

        if (pkProp)
        {
            PRINT_INDENT((usIndent + 2) * ms_usTabStop);
            *ms_pkOstr << pkProp->GetRTTI()->GetName() << std::endl;
        }

    }
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintExtraData(NiObject* pkObject,
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiObjectNET, pkObject))
        return;

    NiObjectNET *pkObjectNET = (NiObjectNET *) pkObject;

    for (unsigned int i=0; i < pkObjectNET->GetExtraDataSize(); i++)
    {
        NiExtraData* pkData = pkObjectNET->GetExtraDataAt(i);

        PRINT_INDENT((usIndent + 2) * ms_usTabStop);
        *ms_pkOstr << pkData->GetRTTI()->GetName() << ": ";

        *ms_pkOstr << "Name: \"" << pkData->GetName() << "\" Value: ";
  
        if (NiIsKindOf(NiBooleanExtraData, pkData))
        {
            *ms_pkOstr << "<" <<
                (((NiBooleanExtraData*)pkData)->GetValue() ? "true" : "false")
                << ">";
        }
        else if (NiIsKindOf(NiColorExtraData, pkData))
        {
            *ms_pkOstr << "<"
                << ((NiColorExtraData*)pkData)->GetRed() << " "
                << ((NiColorExtraData*)pkData)->GetGreen() << " "
                << ((NiColorExtraData*)pkData)->GetBlue() << " "
                << ((NiColorExtraData*)pkData)->GetAlpha() << ">";
        }
        else if (NiIsKindOf(NiFloatExtraData, pkData))
        {
            *ms_pkOstr
                << "<" << ((NiFloatExtraData*)pkData)->GetValue() << ">";
        }
        else if (NiIsKindOf(NiFloatsExtraData, pkData))
        {
            unsigned int uiSize;
            float* afVals;
            ((NiFloatsExtraData*)pkData)->GetArray(uiSize, afVals);
            *ms_pkOstr << "<";
            for (unsigned int ui = 0; ui < uiSize; ui++)
            {
                if (ui != 0)
                    *ms_pkOstr << " ";
                *ms_pkOstr << afVals[ui];
            }
            *ms_pkOstr << ">";
        }
        else if (NiIsKindOf(NiIntegerExtraData, pkData))
        {
            *ms_pkOstr << "<"
                << ((NiIntegerExtraData*)pkData)->GetValue() << ">";
        }
        else if (NiIsKindOf(NiIntegersExtraData, pkData))
        {
            unsigned int uiSize;
            int* aiVals;
            ((NiIntegersExtraData*)pkData)->GetArray(uiSize, aiVals);
            *ms_pkOstr << "<";
            for (unsigned int ui = 0; ui < uiSize; ui++)
            {
                if (ui != 0)
                    *ms_pkOstr << " ";
                *ms_pkOstr << aiVals[ui];
            }
            *ms_pkOstr << ">";
        }
        else if (NiIsKindOf(NiStringExtraData, pkData))
        {
            *ms_pkOstr << "<\""
                << ((NiStringExtraData*)pkData)->GetValue() << "\">";
        }
        else if (NiIsKindOf(NiStringsExtraData, pkData))
        {
            unsigned int uiSize;
            char** apcVals;
            ((NiStringsExtraData*)pkData)->GetArray(uiSize, apcVals);
            *ms_pkOstr << "<";
            for (unsigned int ui = 0; ui < uiSize; ui++)
            {
                if (ui != 0)
                    *ms_pkOstr << " \"";
                else
                    *ms_pkOstr << "\"";
                *ms_pkOstr << apcVals[ui];
                *ms_pkOstr << "\"";
            }
            *ms_pkOstr << ">";
        }
        else if (NiIsKindOf(NiVectorExtraData, pkData))
        {
            *ms_pkOstr << "<"
                << ((NiVectorExtraData*)pkData)->GetX() << " "
                << ((NiVectorExtraData*)pkData)->GetY() << " "
                << ((NiVectorExtraData*)pkData)->GetZ() << " "
                << ((NiVectorExtraData*)pkData)->GetW() << ">";
        }
        *ms_pkOstr << std::endl;
    }
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintGeometryData(NiObject* pkObject, 
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;

    if (! NiIsKindOf(NiGeometry, pkObject))
        return;

	GeomInfo info;
	ZeroMemory(&info, sizeof(GeomInfo));
	info.count++;
    NiGeometry* pkGeometry = (NiGeometry*)pkObject;
    NiGeometryData* pkGeomData = pkGeometry->GetModelData();

	info.address = pkGeomData;
	strncpy_s(info.name, sizeof(info.name), pkGeometry->GetName(), 256);

    PRINT_INDENT((usIndent + 2) * ms_usTabStop);
    *ms_pkOstr << "NiGeometryData ";
    if (ms_bShowMemoryAddress)
        *ms_pkOstr << "<0x" << pkGeomData << "> ";

    if (pkGeometry->GetSkinInstance())
    {
		info.skinned = true;
        *ms_pkOstr << "SKINNED ";
    }
    else
    {
		info.skinned = false;
        *ms_pkOstr << "        ";
    }

    if (pkGeomData)
    {
        unsigned short usVertCount = pkGeomData->GetVertexCount();
        unsigned short usActiveVertCount = pkGeomData->GetActiveVertexCount();

		info.vertexCount = usVertCount;

        *ms_pkOstr << "Vertex Count: " << usVertCount << "(" << 
            usActiveVertCount << ")" << std::endl;

        PRINT_INDENT((usIndent + 2) * ms_usTabStop);

        if (pkGeomData->GetNormals())
            *ms_pkOstr << "NORMALS  ";
        else
            *ms_pkOstr << "         ";
        if (pkGeomData->GetColors())
            *ms_pkOstr << "COLORS   ";
        else
            *ms_pkOstr << "         ";
        if (pkGeomData->GetTextures())
        {
            *ms_pkOstr << "UVS (" <<
                pkGeomData->GetTextureSets() << ")";
        }
        *ms_pkOstr << std::endl;

        if (NiIsKindOf(NiTriBasedGeomData, pkGeomData))
        {
            NiTriBasedGeomData* pkTBGD = (NiTriBasedGeomData*)pkGeomData;

            unsigned short usTriCount = pkTBGD->GetTriangleCount();
            unsigned short usActiveTriCount =
                pkTBGD->GetActiveTriangleCount();

			info.triCount = usTriCount;

            PRINT_INDENT((usIndent + 2) * ms_usTabStop);
            *ms_pkOstr << "Triangle Count: " << usTriCount << 
                "(" << usActiveTriCount << ")" << std::endl;

            if (NiIsKindOf(NiTriShapeData, pkGeomData))
            {
				info.triShape = true;
                ms_uiTriShapeCount++;

                ms_uiTriShape_TriCount += usTriCount;
                if (ms_bFirstTriShape)
                {
                    ms_uiTriShape_LowTriCount = usTriCount;
                    ms_bFirstTriShape = false;
                }
                else if (usTriCount < ms_uiTriShape_LowTriCount)
                    ms_uiTriShape_LowTriCount = usTriCount;
                if (usTriCount > ms_uiTriShape_HighTriCount)
                    ms_uiTriShape_HighTriCount = usTriCount;
                if (usTriCount == 0)
                    ms_uiTriShape_ZeroTriCount++;
            }
            else if (NiIsKindOf(NiTriStripsData, pkGeomData))
            {
				info.triShape = false;
                ms_uiTriStripsCount++;

                ms_uiTriStrips_TriCount += usTriCount;
                if (ms_bFirstTriStrips)
                {
                    ms_uiTriStrips_LowTriCount = usTriCount;
                    ms_bFirstTriStrips = false;
                }
                else if (usTriCount < ms_uiTriStrips_LowTriCount)
                    ms_uiTriStrips_LowTriCount = usTriCount;
                if (usTriCount > ms_uiTriStrips_HighTriCount)
                    ms_uiTriStrips_HighTriCount = usTriCount;
                if (usTriCount == 0)
                    ms_uiTriStrips_ZeroTriCount++;
            }
        }
    }
    else
        *ms_pkOstr << std::endl;

	GeometryMap::iterator iter = g_pGeomMap->find(pkGeomData);
	if (iter == g_pGeomMap->end())
		g_pGeomMap->insert(std::make_pair(pkGeomData, info));
	else
		iter->second.count++;
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::RecursivePrint(NiObject* pkObject,
    unsigned short usIndent)
{
	if (pkObject == NULL)
		return;
    //
    // Print current node
    // 
    PrintID(pkObject, usIndent);

    if (ms_bShowAppCulled)
        PrintAppCulled(pkObject, usIndent);

    if (ms_bShowTransforms)
        PrintTransforms(pkObject, usIndent);

    if (ms_bShowBoundingSpheres)
        PrintBoundingSpheres(pkObject, usIndent);

    if (ms_bShowProperties)
        PrintProperties(pkObject, usIndent);

    if (ms_bShowExtraData)
        PrintExtraData(pkObject, usIndent);

    if (ms_bShowGeometryData)
        PrintGeometryData(pkObject, usIndent);

    if (NiIsKindOf(NiObjectNET, pkObject))
    {
        NiTimeController* pControl = 
            ((NiObjectNET *) pkObject)->GetControllers();

        for (/**/; pControl; pControl = pControl->GetNext())
            RecursivePrint(pControl, usIndent+1);
    }

    //  Don't print AppCulled subtree if ms_bDropAppCulled is true
    if (ms_bDropAppCulled && NiIsKindOf(NiAVObject, pkObject) &&
            ((NiAVObject*)pkObject)->GetAppCulled())
    {
        return;
    }

    if (NiIsKindOf(NiNode, pkObject))
    {
        NiNode* pNode = (NiNode*)pkObject;

        //
        // Iterate over all children
        //
        for (unsigned int i = 0; i < pNode->GetArrayCount(); i++)
        {
            NiAVObject* pChild = pNode->GetAt(i);

            if (pChild)
                RecursivePrint(pChild, usIndent + 1);
        }
    }
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::SummaryPrint()
{
    *ms_pkOstr << "Total Object Count: " << ms_uiObjectCount 
        << ", Tree Depth: " << (ms_uiDepth + 1) << "\n\n";

    if (ms_bShowGeometryData)
        PrintTriSummary();

    InitStats();
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::PrintTriSummary()
{
    *ms_pkOstr << "TriShape:" << std::endl;
    *ms_pkOstr << "    Data objects = " << ms_uiTriShapeCount << std::endl;
    *ms_pkOstr << "    TriCount     = " << ms_uiTriShape_TriCount << 
        std::endl;
    *ms_pkOstr << "    Avg Ratio    = ";
    *ms_pkOstr << (((float)(ms_uiTriShape_TriCount) /
        (float)ms_uiObjectCount)) << std::endl;
    *ms_pkOstr << "    Low TriCount = " << ms_uiTriShape_LowTriCount << 
        std::endl;
    *ms_pkOstr << "    High TriCount= " << ms_uiTriShape_HighTriCount << 
        std::endl;
    *ms_pkOstr << "    Zero         = " << ms_uiTriShape_ZeroTriCount << 
        std::endl;
    *ms_pkOstr << "TriStrips:" << std::endl;
    *ms_pkOstr << "    Data objects = " << ms_uiTriStripsCount << std::endl;
    *ms_pkOstr << "    TriCount     = " << ms_uiTriStrips_TriCount << 
        std::endl;
    *ms_pkOstr << "    Avg Ratio    = ";
    *ms_pkOstr << (((float)(ms_uiTriStrips_TriCount) /
        (float)ms_uiObjectCount)) << std::endl;
    *ms_pkOstr << "    Low TriCount = " << ms_uiTriStrips_LowTriCount << 
        std::endl;
    *ms_pkOstr << "    High TriCount= " << ms_uiTriStrips_HighTriCount << 
        std::endl;
    *ms_pkOstr << "    Zero         = " << ms_uiTriStrips_ZeroTriCount << 
        std::endl;

	int i = 1;
	for (GeometryMap::iterator iter = g_pGeomMap->begin(); iter != g_pGeomMap->end(); iter++)
	{
		*ms_pkOstr << i << ":  name:" << iter->second.name << " addr:0x" << iter->second.address << " count:" << iter->second.count <<
			" skinned:" << iter->second.skinned << " triShape:" << iter->second.triShape << 
			" vert:" << iter->second.vertexCount << " tri:" << iter->second.triCount << std::endl;
		i++;
	}
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::InitStats()
{
    // reset summary statistics
    ms_uiObjectCount = ms_uiDepth = ms_uiTriShapeCount = 
        ms_uiTriShape_TriCount = ms_uiTriShape_HighTriCount = 
        ms_uiTriShape_ZeroTriCount = ms_uiTriStripsCount = 
        ms_uiTriStrips_TriCount = ms_uiTriStrips_HighTriCount =
        ms_uiTriStrips_ZeroTriCount = 0;

    ms_uiTriShape_LowTriCount = ms_uiTriStrips_LowTriCount = 0;

    ms_bFirstTriShape = ms_bFirstTriStrips = true;
}

//---------------------------------------------------------------------------
void NiSceneGraphPrinter::OpenLog()
{
    if (ms_pkOstr == &std::cout)
    {
#if defined(_XENON)
        ms_pkOstr = new std::ofstream("D:\\NiSceneGraph.txt", std::ios::app);
#else   //#if defined(_XENON)
        ms_pkOstr = new std::ofstream("NiSceneGraph.txt", std::ios::app);
#endif  //#if defined(_XENON)
    }

    InitStats();
}
//---------------------------------------------------------------------------
void NiSceneGraphPrinter::CloseLog()
{
    if (ms_pkOstr != &std::cout)
	 {
        SAFE_DELETE(ms_pkOstr);
	 }
    ms_pkOstr = &std::cout;
}
//---------------------------------------------------------------------------

#endif