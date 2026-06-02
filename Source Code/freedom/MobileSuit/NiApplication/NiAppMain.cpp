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

// Precompiled Header
#include "stdafx.h"

#include "NiApplication.h"
#include <NiFile.h>
#include <NiSystem.h>
#include "../resource.h"
#include "../PgWebLogin.h"
#include "../PgAppProtect.h"
#include "../PgCmdlineParse.h"
#include "../PgChatWindow.h"
#include "../CreateUsingNiNew.inl"
#include "../PgProcessChecker.h"
#include "../PgMessageUtil.h"
#include "../PgMemTracker.H"
#include "../PgWorld.h"


int APIENTRY _tWinMain(HINSTANCE hI, HINSTANCE hPI, LPTSTR cmdline, int iWinMode)
{
#ifdef _MEMORY_TRACKING
	g_pkMemoryTrack = new PgMemoryTrack();
	g_pkMemoryTrack->Init(BM::OUTPUT_FILE_AND_CONSOLE, _T("./MemoryTrack"), _T("MemoryTrack.txt"));
#endif
    // for determining if there are 'object leaks'
    unsigned int uiInitialCount = NiRefObject::GetTotalObjectCount();

	NiInit();	
	NiApplication::setLocalDirectory();
	
	POINT2 ptResolutionSize( XUI::EXV_DEFAULT_SCREEN_WIDTH, XUI::EXV_DEFAULT_SCREEN_HEIGHT );
	TiXmlDocument kDoc;

	bool bNotUseUserFile = false;

	if( kDoc.LoadFile( STR_OPTION_USER_FILE ) )
	{
		TiXmlNode*	Node = kDoc.FirstChild(XML_ELEMENT_CONFIG);
		if( Node )
		{
			TiXmlNode*	ChildNode = Node->FirstChild();
			while(ChildNode != NULL)
			{	
				TiXmlNode*	AttrNode = ChildNode->FirstChild();
				while(AttrNode != NULL)
				{
					const TiXmlElement*	Elem = AttrNode->ToElement();
					if( Elem )
					{
						char const*	szName	= Elem->Attribute(XML_ATTR_NAME);
						int			nValue	= 0;
						Elem->Attribute(XML_ATTR_VALUE, &nValue);

						if(strcmp(ChildNode->Value(), XML_ELEMENT_GAME) == 0)
						{
							if (!strcmp(szName, "BORDERLESS") && nValue == 1)
							{
								ptResolutionSize.x = GetSystemMetrics(SM_CXSCREEN);
								ptResolutionSize.y = GetSystemMetrics(SM_CYSCREEN);
								break;
							}
							else if(strcmp(szName, "RESOLUTION_WIDTH") == 0)	
							{ 
								ptResolutionSize.x = nValue;
							}
							else if(strcmp(szName, "RESOLUTION_HEIGHT") == 0)	
							{ 
								ptResolutionSize.y = nValue;
							}
						}					
					}
					AttrNode = AttrNode->NextSibling();
				}
				ChildNode = ChildNode->NextSibling();
			}			
		}

		kDoc.Clear();
	}
	else
	{
		ptResolutionSize.x = GetSystemMetrics(SM_CXSCREEN);
		ptResolutionSize.y = GetSystemMetrics(SM_CYSCREEN);
		bNotUseUserFile = true;
	}

	if( ptResolutionSize.x != XUI::EXV_DEFAULT_SCREEN_WIDTH || ptResolutionSize.y != XUI::EXV_DEFAULT_SCREEN_HEIGHT )
	{
		XUIMgr.SetIsDefaultResolution(false);
	}	
	
 	XUIMgr.SetResolutionSize( ptResolutionSize );
 	
 	POINT2 ptGab;
 	ptGab.x = ptResolutionSize.x - XUI::EXV_DEFAULT_SCREEN_WIDTH;
 	ptGab.y = ptResolutionSize.y - XUI::EXV_DEFAULT_SCREEN_HEIGHT;
 
 	XUIMgr.CorrectionLocationByResolution( ptGab );

#ifndef EXTERNAL_RELEASE
    // If cmdline.txt file exists in the work directory, read it and 
    // set it as the command line
    NiFile* pkCmdlineFile = 0;
    if (!cmdline || _tcslen(cmdline) == 0)
    {
        pkCmdlineFile = NiNew NiFile("./cmdline.txt", NiFile::READ_ONLY);
    }

    if (pkCmdlineFile && *pkCmdlineFile)
    {
        // artificial 1024-character limit
        char* pcCmdline = NiAlloc(char, 1024);
        int iLength = 0;
        char c = 0;
        // Read until limit is reached or end of file or end of string
        while (iLength < 1023) 
        {
            if (!(pkCmdlineFile->Read(&c, 1) || c == '\0'))
                break;

            // Factor out newlines and carriage returns
            // Replace with ' ' in case newline seperates valid parameters
            if (c == '\n' || c == '\r')
                c = ' ';
            pcCmdline[iLength++] = c;
        }
        pcCmdline[iLength] = '\0';

        // let the application know about the command line
        NiApplication::SetCommandLine(pcCmdline);

        NiFree(pcCmdline);
    }
    else
#endif
    {
        // let the application know about the command line
#if defined(UNICODE) || defined(_UNICODE)
        NiApplication::SetCommandLine((char*)MB(cmdline));
#else
		NiApplication::SetCommandLine(cmdline);
#endif
    }

#ifndef EXTERNAL_RELEASE
	if (!cmdline || _tcslen(cmdline) == 0)
    {
        SAFE_DELETE_NI(pkCmdlineFile);
    }
#endif
	CoInitialize(NULL);
//NC 전용.
//#ifdef VER_KOREA
//	/GameID:"PlayNCLauncher" /_SessKey:"6a2de42a2f97dc118fbd0014c25456c0" /MacAddr:"0018F302D536" /ServerAddr:"222.231.10.215"
//	MessageBox(NULL, cmdline, L"", 0);
//#endif
//	/ServerAddr:"000.000.000.000" /ServerPort:"0000"
	g_kCmdLineParse.Init(cmdline);// 현재는 중국
    // create the application
    NiApplication* pkTheApp = NiApplication::Create();
    if (!pkTheApp)
    {
        NiMessageBox("Unable to create application", 
            NiAppWindow::GetWindowClassName());
        NiShutdown();    
        return 0;
    }
	
	// g_kLocal을 사용해야 해서 NiApplication::Create() 이후에.
	g_kWebLogin.Init(cmdline);

    // Set reference
    NiApplication::SetInstanceReference(hI);

    // process the command line for renderer selection
    NiApplication::CommandSelectRenderer();

    // Now, we are ready to initialize the window and the main loop.

	std::wstring kClassName = UNI(NiAppWindow::GetWindowClassName());

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= NiApplication::WinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hI;
	wcex.hIcon			= LoadIcon(hI, MAKEINTRESOURCE(IDI_MOBILESUIT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszClassName	= kClassName.c_str();
	wcex.hIconSm		= LoadIcon(hI, MAKEINTRESOURCE(IDI_MOBILESUIT));

    // register menu (if requested)
    if (pkTheApp->GetMenuID())
        wcex.lpszMenuName = MAKEINTRESOURCE(pkTheApp->GetMenuID());
    else
        wcex.lpszMenuName = 0;

	RegisterClassEx(&wcex);

	PgChatWindow::RegisterWindowClass(hI,wcex);

    // create the application window
    NiWindowRef pWnd = pkTheApp->CreateMainWindow(iWinMode);
	
	////////////////////////////////////////////////////////////////
	//window 만들어진 이후 바로 GameGuardInit
#ifdef USE_HACKSHIELD
	LoadLibrary( L"NxExtensions.dll" );
	LoadLibrary( L"PhysXCore.dll" );
#endif
	const HRESULT hRet = g_kAppProtect.Init(pkTheApp->GetWindowReference());
	if(S_OK != hRet)
	{//에라
		std::wstring kTempStr;
		PgMessageUtil::GetFromMessageDotIni(_T("MESSAGE"), _T("InitAppProtectFailed"), kTempStr, _T("AppProtect luanch failed"));
		::MessageBox(NULL, kTempStr.c_str(), GetClientNameW(), MB_OK);
		return false;
	}

	ShowWindow(pkTheApp->GetWindowReference(), iWinMode);
    UpdateWindow(pkTheApp->GetWindowReference());

    // allow the application to initialize before starting the message pump
    if(pkTheApp->Initialize())
    {
        pkTheApp->MainLoop();
    }

    pkTheApp->Terminate();

    SAFE_DELETE_NI(pkTheApp);

    NiShutdown();    

	//if(g_kLocal.ServiceRegion() == LOCAL_MGR::NC_CHINA)
	//{
	//	if(g_kVersionCheck.PatcherCall())
	//	{
	//		g_kVersionCheck.PatcherLoad(g_kVersionCheck.ChinaPatchAddr().c_str());
	//	}
	//}

#ifndef USE_INB
    // for determining if there are 'object leaks'
    unsigned int uiFinalCount = NiRefObject::GetTotalObjectCount();
    char acMsg[256];
    NiSprintf(acMsg, 256,
        "\nGamebryo NiRefObject counts:  initial = %u, final = %u. ", 
        uiInitialCount, uiFinalCount);
    OutputDebugString(UNI(acMsg));
    if (uiFinalCount > uiInitialCount)
    {
        unsigned int uiDiff = uiFinalCount - uiInitialCount;
        NiSprintf(acMsg, 256, "Application is leaking %u objects\n\n", uiDiff);
        OutputDebugString(UNI(acMsg));
    }
    else
    {
        OutputDebugString(L"Application has no object leaks.\n\n");
    }
#endif

#ifdef _MEMORY_TRACKING
	if (g_pkMemoryTrack)
	{
		g_pkMemoryTrack->DisplayState(g_kLogWorker, LT_INFO, true);
	}
	PgMemoryTrack* pkTrack = g_pkMemoryTrack;
	g_pkMemoryTrack = NULL;
	delete pkTrack;

#endif    

#ifndef EXTERNAL_RELEASE
    // Output Performance profiling.
    NewWare::PerfProfiler::OutputReport( "profile.log", true );
#endif //#ifndef EXTERNAL_RELEASE

    return 0;
}
//---------------------------------------------------------------------------
