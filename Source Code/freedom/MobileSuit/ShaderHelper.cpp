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
//---------------------------------------------------------------------------

// Precompiled Header
#include "stdafx.h"
#include "ShaderHelper.h"
#ifdef USE_GAMEBRYO_DLL
#define _USRDLL
#include "NiD3DShaderLibraryInterface.h"
#endif

//---------------------------------------------------------------------------
bool ShaderHelper::SetupShaderSystem(
    char* apcProgramDirectory[], unsigned int uiNumProgramDirectories, 
    char* apcShaderDirectories[], unsigned int uiNumShaderDirectories)
{
    assert (NiD3DShaderProgramFactory::GetInstance());
	NiShaderFactory::RegisterErrorCallback(ShaderErrorCallback);

    for (unsigned int i = 0; i < uiNumProgramDirectories; i++)
    {
        NiD3DShaderProgramFactory::GetInstance()->AddProgramDirectory(
            apcProgramDirectory[i]);
    }

    m_uiShaderDirectoryCount = uiNumShaderDirectories;
    m_ppcShaderDirectories = apcShaderDirectories;

    // First, we will run the NSF parser. This is done to make sure that
    // any text-based shader files that have been modified are re-compiled
    // to binary before loading all the binary representations.
    if (!RunShaderParsers())
    {
        PgError("Failed to run shader parsers!");
        return false;
    }

    if (!RegisterShaderLibraries())
    {
        PgError("Failed to register shader libraries!");
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool ShaderHelper::RunShaderParsers()
{
    NiShaderFactory::RegisterRunParserCallback(NSFRunParser);

    for (unsigned int i = 0; i < m_uiShaderDirectoryCount; i++)
    {
        unsigned int uiCount = NiShaderFactory::LoadAndRunParserLibrary(
            0, m_ppcShaderDirectories[i], true);

        if (uiCount == 0)
        {
            _PgOutputDebugString("NSF parser library failed to parse any shaders!");
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool ShaderHelper::RegisterShaderLibraries()
{
    NiShaderFactory::RegisterClassCreationCallback(NSBLibraryClassCreate);

    if (!NiShaderFactory::LoadAndRegisterShaderLibrary(0, 
        m_uiShaderDirectoryCount, m_ppcShaderDirectories, true))
    {
        _PgOutputDebugString("NSB shader library failed to load any shaders!");
    }

    NiShaderFactory::RegisterClassCreationCallback(FXLibraryClassCreate);

    if (!NiShaderFactory::LoadAndRegisterShaderLibrary(0, 
        m_uiShaderDirectoryCount, m_ppcShaderDirectories, true))
    {
        _PgOutputDebugString("FX shader library failed to load any shaders!");
    }

    return true;
}
//---------------------------------------------------------------------------
bool ShaderHelper::NSBLibraryClassCreate(char const* pcLibFile, 
    NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
    bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary)
{
    *ppkLibrary = 0;

    // Create the NSB Shader Library.
    return 
#ifdef USE_GAMEBRYO_DLL
		LoadShaderLibrary
#else
		NSBShaderLib_LoadShaderLibrary
#endif
			(pkRenderer, iDirectoryCount, apcDirectories, bRecurseSubFolders, ppkLibrary);
}
//---------------------------------------------------------------------------
bool ShaderHelper::FXLibraryClassCreate(char const* pcLibFile, 
    NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
    bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary)
{
    *ppkLibrary = 0;

    // Create the FX Shader Library.
    return
#ifdef USE_GAMEBRYO_DLL
		LoadShaderLibrary
#else
		NiD3DXEffectShaderLib_LoadShaderLibrary
#endif
			(pkRenderer, iDirectoryCount, apcDirectories, bRecurseSubFolders, ppkLibrary);
}
//---------------------------------------------------------------------------
unsigned int ShaderHelper::NSFRunParser(char const* pcLibFile, 
    NiRenderer* pkRenderer, char const* pcDirectory, 
    bool bRecurseSubFolders)
{
    // Run the NSF Parser.
    return
#ifdef USE_GAMEBRYO_DLL
		RunShaderParser
#else
		NSFParserLib_RunShaderParser
#endif
			(pcDirectory, bRecurseSubFolders);
}
//---------------------------------------------------------------------------
unsigned int ShaderHelper::ShaderErrorCallback(char const* pcError, 
    NiShaderError eError, bool bRecoverable)
{
    PgError3("[ShaderHelper] Shader Error %s, %d, %d", pcError, eError, bRecoverable);
    return 0;
}
//---------------------------------------------------------------------------