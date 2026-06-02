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

#ifndef NIDX9SELECT_H
#define NIDX9SELECT_H

#include "NiApplication.h"
#if defined(_DX9)

#include <NiOS.h>
#include <NiRenderer.h>
#include <NiUniversalTypes.h>

class NiDX9AdapterDesc;
class NiDX9DeviceDesc;
class NiDX9SystemDesc;

//---------------------------------------------------------------------------
class NiDX9Select 
{
public:
    static NiRendererPtr CreateRenderer(NiWindowRef pTopLevelWnd, 
        NiWindowRef pRenderWnd, bool bDialog, unsigned int uiBitDepth, 
        unsigned int uiWidth, unsigned int uiHeight, unsigned int uiRefreshRate,bool bStencil, 
        bool bMultiThread, bool bRefRast, bool bSWVertex, bool& bNVPerfHUD,
        bool& bFullscreen,bool &bVSync);

protected:
    typedef enum {
        RETURN_OK,
        RETURN_FAIL,
        RETURN_CANCEL
    } ReturnVal;

    enum { IDC_STATIC = 0xffff };
    enum { IDC_ADAPTER_COMBO = 1000 };
    enum { IDC_MODE_COMBO = 1001 };
    enum { IDC_RESOLUTION_COMBO = 1002 };
    enum { IDC_ZBUFFER_CHECK = 1003 };
    enum { IDC_MULTISAMPLE_COMBO = 1004 };
	enum { IDC_BLOOM_CHECK = 1005 };
	enum { IDC_USE_PACK_CHECK = 1006 };
	enum { IDC_USE_SOUND_CHECK = 1007 };
	enum { IDC_LOCALE_COMBO = 1008 };
	enum { IDC_USE_DEBUG_CHECK = 1009 };

    static NiRenderer* DialogCreateRenderer(NiWindowRef pTopLevelWnd, 
        NiWindowRef pRenderWnd, unsigned int uiDefaultWidth, 
        unsigned int uiDefaultHeight, bool bStencil, bool bMultiThread, 
        bool bRefRast, bool bSWVertex, bool& bNVPerfHUD, bool& bFullscreen);

    static ReturnVal Select(NiWindowRef pWnd, 
        unsigned int uiDefaultWidth, unsigned int uiDefaultHeight,
        bool& bFullscreen, bool& b32BitZBuffer, void const* &pkAdapter,
        void const* &pkDevice, void const*& pkMode, unsigned int& uiFBMode);
    
	 static void EnumerateLocale(NiWindowRef pDialog);

	 static void EnumerateAdapters(NiWindowRef pDialog);
    static void EnumerateResolutions(NiWindowRef pDialog);
    static void EnumerateMultisamples(NiWindowRef pDialog);
    static bool UpdateLocale(NiWindowRef pDialog);
	 static bool UpdateAdapter(NiWindowRef pDialog);
    static bool UpdateResolution(NiWindowRef pDialog);
    static bool UpdateMultisample(NiWindowRef pDialog);
    static BOOL CALLBACK Callback(NiWindowRef pDialog, unsigned int uiMsg,
        WPARAM wParam, LPARAM lParam);
    static void CopyToWideChar(unsigned short** ppusStrOut, 
        char const* pcStrIn);
    static void AddDialogControl(unsigned short** ppusTemplate, 
        unsigned int uiStyle, short sSX, short sSY, short sCX, short sCY, 
        unsigned short usID, char const* pcStrClassName, 
        char const* pcStrTitle);
    static DLGTEMPLATE* BuildAdapterSelectTemplate();

    static const NiDX9DeviceDesc* ms_pkDevice;
    static void const* ms_pkMode;
    static const NiDX9AdapterDesc* ms_pkAdapter;
    static const NiDX9SystemDesc* ms_pkInfo;
    static bool ms_bFullscreen;
    static bool ms_b32BitZBuffer;
    static unsigned int ms_uiMultisample;
    static bool ms_bDialogOpen;
    static unsigned int ms_uiDefaultWidth;
    static unsigned int ms_uiDefaultHeight;
	static bool ms_bBloom;
};

#endif //#if defined(_DX9)

#endif
