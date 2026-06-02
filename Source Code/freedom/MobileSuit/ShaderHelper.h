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

#ifndef SHADERHELPER_H
#define SHADERHELPER_H

#include <NiD3DDefines.h>

#include <NiShader.h>
#include <NiShaderFactory.h>
#include <NiShaderLibrary.h>
#include <NiShaderLibraryDesc.h>
#include <NiD3DShaderProgramFactory.h>
#include <NiD3DRendererHeaders.h>

class ShaderHelper : public NiMemObject
{
public:
    bool SetupShaderSystem(
        char* apcProgramDirectories[], unsigned int uiNumProgramDirectories,
        char* apcShaderDirectories[], unsigned int uiNumShaderDirectories);
    bool RunShaderParsers();
    bool RegisterShaderLibraries();

protected:
    static bool NSBLibraryClassCreate(char const* pcLibFile,
        NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
        bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary);
    static bool FXLibraryClassCreate(char const* pcLibFile,
        NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
        bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary);
    static unsigned int NSFRunParser(char const* pcLibFile, 
        NiRenderer* pkRenderer, char const* pcDirectory, 
        bool bRecurseSubFolders);
	static unsigned int ShaderErrorCallback(char const* pcError, 
		NiShaderError eError, bool bRecoverable);

    static char* ms_apcShaderParsers[];
    static char* ms_apcShaderLibraries[];

    unsigned int m_uiShaderDirectoryCount;
    char** m_ppcShaderDirectories;
};

#endif
