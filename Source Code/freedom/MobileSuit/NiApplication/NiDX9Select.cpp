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

#include "./NiDX9Select.h"

#if defined(_DX9)

#include <NiDX9Renderer.h>
#include <NiDX9Headers.h>
#include <NiDX9SystemDesc.h>
#include <NiRTLib.h>
#include <NiTList.h>

#ifdef NIDX9RENDERER_IMPORT
    //#pragma comment(lib, "NiDX9Renderer" NI_DLL_SUFFIX ".lib")
	#pragma comment(lib, "NiDX9Renderer" "22VC80S" ".lib") //임시
#else   // #ifdef NIDX9RENDERER_IMPORT
    #pragma comment(lib, "NiDX9Renderer.lib")
#endif  // #ifdef NIDX9RENDERER_IMPORT

const NiDX9DeviceDesc* NiDX9Select::ms_pkDevice = 0;
void const* NiDX9Select::ms_pkMode = 0;
const NiDX9AdapterDesc* NiDX9Select::ms_pkAdapter = 0;
const NiDX9SystemDesc* NiDX9Select::ms_pkInfo = 0;
bool NiDX9Select::ms_bFullscreen = false;
bool NiDX9Select::ms_b32BitZBuffer = false;
unsigned int NiDX9Select::ms_uiMultisample = NiDX9Renderer::FBMODE_DEFAULT;
bool NiDX9Select::ms_bDialogOpen = false;
unsigned int NiDX9Select::ms_uiDefaultWidth = XUI::EXV_DEFAULT_SCREEN_WIDTH;
unsigned int NiDX9Select::ms_uiDefaultHeight = XUI::EXV_DEFAULT_SCREEN_HEIGHT;
bool NiDX9Select::ms_bBloom = false;

void NiDX9Select::EnumerateLocale(NiWindowRef pDialog)
{
	static std::vector<std::wstring> vecLocale;
	static bool bIsInit = false;
	if(!bIsInit)
	{
		bIsInit = true;
		vecLocale.push_back((std::wstring)_T("kor"));
		vecLocale.push_back((std::wstring)_T("jpn")); 
		vecLocale.push_back((std::wstring)_T("chs"));
		vecLocale.push_back((std::wstring)_T("cht"));
	}

	int iBaseIndex = SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, 
			CB_ADDSTRING, 0, (LPARAM)_T("OS Locale"));

	for(unsigned int i = 0; vecLocale.size() > i; i++)
	{
		int iIndex = SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, 
			CB_ADDSTRING, 0, (LPARAM)vecLocale.at(i).c_str());
		
		SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)&vecLocale.at(i));
	}
	
	SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, CB_SETCURSEL, (WPARAM)iBaseIndex, 0L);
}

//---------------------------------------------------------------------------
void NiDX9Select::EnumerateAdapters(NiWindowRef pDialog)
{
    SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, CB_RESETCONTENT, 0, 0);

    unsigned int uiAdapterCount = ms_pkInfo->GetAdapterCount();

    bool bAdaptersExist = false;
    for (unsigned int i = 0; i < uiAdapterCount; i++)
    {
        const NiDX9AdapterDesc* pkAdapter = ms_pkInfo->GetAdapter(i);
        assert(pkAdapter);

        const NiDX9DeviceDesc* pkDevice = pkAdapter->GetDevice(D3DDEVTYPE_HAL);
        if (!pkDevice)
            continue;

        // we skip fullscreen - only devices unless we are looking
        // at fullscreen mode.  Also, we skip SW devices for now.
        // Later, we will add just the primary device as a SW device
        if (!ms_bFullscreen)
        {
            // test if device can render to window in the current display mode
            if (!pkAdapter->CanDeviceRenderWindowed(D3DDEVTYPE_HAL))
                continue;
        }

        // Add the Adapter name to the combo box 
        int iIndex = SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, 
            CB_ADDSTRING, 0, (LPARAM)UNI(pkAdapter->GetStringDescription()));
        SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, 
            CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)pkAdapter);

        bAdaptersExist = true;
    }

    SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, CB_SETCURSEL, (WPARAM)0, 
        0L);

    if (bAdaptersExist)
    {
        ms_pkAdapter = (const NiDX9AdapterDesc*)SendDlgItemMessage(pDialog, 
            IDC_ADAPTER_COMBO, CB_GETITEMDATA, (WPARAM)0, (LPARAM)0);
        ms_pkDevice = ms_pkAdapter->GetDevice(D3DDEVTYPE_HAL);
    }
    else
    {
        ms_pkAdapter = 0;
        ms_pkDevice = 0;
    }
}
//---------------------------------------------------------------------------
void NiDX9Select::EnumerateResolutions(NiWindowRef pDialog)
{
    SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, CB_RESETCONTENT, 0, 0);

    // walk list adding items
    if (!ms_bFullscreen)
        return;

    if (!ms_pkAdapter)
        return;

    unsigned int uiModeCount = ms_pkAdapter->GetModeCount();

    int i16Default = -1;
    int i32Default = -1;
    bool bResolutionsExist = false;
    
    for (unsigned int i = 0; i < uiModeCount; i++)
    {
        const NiDX9AdapterDesc::ModeDesc* pkMode = ms_pkAdapter->GetMode(i);

 		//if(pkMode->m_uiWidth != XUI::EXV_DEFAULT_SCREEN_WIDTH && pkMode->m_uiHeight != XUI::EXV_DEFAULT_SCREEN_HEIGHT)
 		//{
 		//	continue;
 		//}

		if(pkMode->m_uiWidth != XUIMgr.GetResolutionSize().x && pkMode->m_uiHeight != XUIMgr.GetResolutionSize().y)
		{
			continue;
		}

        char acStr[256];
        NiSprintf(acStr, 256, "%u x %u x %u bpp", pkMode->m_uiWidth, 
            pkMode->m_uiHeight, pkMode->m_uiBPP);
        
        // Add the mode string to the combo box 
        // Note - (could also use pStrDesc)
        int iIndex = SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, 
            CB_ADDSTRING, 0, (LPARAM)UNI(acStr));
        SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, 
            CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)pkMode);

        if ((pkMode->m_uiWidth == ms_uiDefaultWidth) && 
            (pkMode->m_uiHeight == ms_uiDefaultHeight))
        {
            if (pkMode->m_uiBPP == 32)
                i32Default = iIndex;
            else if (pkMode->m_uiBPP == 16)
                i16Default = iIndex;
        }

        bResolutionsExist = true;
    }

    int iDefault = (i32Default > -1 ? i32Default : i16Default);
    SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, CB_SETCURSEL, 
        (WPARAM)iDefault, 0L);

    if (bResolutionsExist)
    {
        ms_pkMode = (void const*)SendDlgItemMessage(pDialog,
            IDC_RESOLUTION_COMBO, CB_GETITEMDATA, 0, 0);
    }
    else
    {
        ms_pkMode = 0;
    }
}
//---------------------------------------------------------------------------
void NiDX9Select::EnumerateMultisamples(NiWindowRef pDialog)
{
    SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, CB_RESETCONTENT, 0, 0);

    // walk list adding items
    if (!ms_pkDevice)
        return;

    D3DFORMAT eFormat;
    if (ms_pkMode)
    {
        NiDX9AdapterDesc::ModeDesc* pkModeDesc = 
            (NiDX9AdapterDesc::ModeDesc*)ms_pkMode;
        eFormat = pkModeDesc->m_eD3DFormat;
    }
    else
    {
        assert (!ms_bFullscreen);
        unsigned int uiBitDepth = (ms_b32BitZBuffer ? 32 : 16);
        NiDX9Renderer::FrameBufferFormat eNiFormat = 
            ms_pkDevice->GetNearestFrameBufferFormat(!ms_bFullscreen, 
            uiBitDepth);
        eFormat = NiDX9Renderer::GetD3DFormat(eNiFormat);
    }

    const NiDX9DeviceDesc::DisplayFormatInfo* pkDFI = 
        ms_pkDevice->GetFormatInfo(eFormat);

    NiDX9Renderer::DepthStencilFormat eNiDSFormat = 
        pkDFI->FindClosestDepthStencil(ms_b32BitZBuffer ? 32 : 16, 0);
    D3DFORMAT eDSFormat = NiDX9Renderer::GetD3DFormat(eNiDSFormat);
    
    // Fill in dialog box
    char acStr[256];
    NiSprintf(acStr, 256, "No Multisampling");
    
    // Add the mode string to the combo box 
    // Note - (could also use pStrDesc)
    int iBaseIndex = SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
        CB_ADDSTRING, 0, (LPARAM)UNI(acStr));
    SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
        CB_SETITEMDATA, (WPARAM)iBaseIndex, (LPARAM)0);

    unsigned int uiQuality = pkDFI->GetNonmaskableMultiSampleQuality(
        !ms_bFullscreen, eFormat, eDSFormat);
    unsigned int i;
    for (i = 0; i < uiQuality; i++)
    {
        NiSprintf(acStr, 256, "Nonmaskable Level %d", i);
    
        int iIndex = SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
            CB_ADDSTRING, 0, (LPARAM)UNI(acStr));
        SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
            CB_SETITEMDATA, (WPARAM)iIndex, 
            (LPARAM)(NiDX9Renderer::FBMODE_MULTISAMPLES_NONMASKABLE | i));
    }

    for (i = 2; i < 17; i++)
    {
        D3DMULTISAMPLE_TYPE eType = (D3DMULTISAMPLE_TYPE)i;

        if (pkDFI->IsMultiSampleValid(!ms_bFullscreen, eType,
            eFormat, eDSFormat))
        {
            NiSprintf(acStr, 256, "%d Multisamples", i);
        
            int iIndex = SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
                CB_ADDSTRING, 0, (LPARAM)UNI(acStr));
            SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
                CB_SETITEMDATA, (WPARAM)iIndex, 
                (LPARAM)(NiDX9Renderer::FBMODE_MULTISAMPLES_2 - 2 + i));
        }
    }

    SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, CB_SETCURSEL, 
        (WPARAM)iBaseIndex, 0L);

    ms_uiMultisample = NiDX9Renderer::FBMODE_DEFAULT;
}

//---------------------------------------------------------------------------
bool NiDX9Select::UpdateLocale(NiWindowRef pDialog)
{
	bool bReturn = true;

	int iIndex = SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, CB_GETCURSEL, 0, 0);
	if (iIndex >= 0)
	{
		std::wstring const* pStr =
			(std::wstring const* )SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, CB_GETITEMDATA, iIndex, 0);
		
		if(pStr)
		{
			g_strLocale = *pStr;
			return bReturn;
		}
	}
    return bReturn;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool NiDX9Select::UpdateAdapter(NiWindowRef pDialog)
{
    bool bReturn = true;

    int iIndex = SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, 
        CB_GETCURSEL, 0, 0);
    if (iIndex >= 0)
    {
        const NiDX9AdapterDesc* pkAdapter = 
            (const NiDX9AdapterDesc*)SendDlgItemMessage(pDialog, 
            IDC_ADAPTER_COMBO, CB_GETITEMDATA, iIndex, 0);
        bReturn = (pkAdapter != ms_pkAdapter);

        ms_pkAdapter = pkAdapter;
        ms_pkDevice = ms_pkAdapter->GetDevice(D3DDEVTYPE_HAL);
    }
    else
    {
        ms_pkDevice = 0;
    }

    return bReturn;
}
//---------------------------------------------------------------------------
bool NiDX9Select::UpdateResolution(NiWindowRef pDialog)
{
    bool bReturn = true;

    int iIndex = SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, 
        CB_GETCURSEL, 0, 0);
    if (iIndex >= 0)
    {
        void* pvMode = (void*)SendDlgItemMessage(pDialog, 
            IDC_RESOLUTION_COMBO, CB_GETITEMDATA, iIndex, 0);
        bReturn = (pvMode != ms_pkMode);

        ms_pkMode = pvMode;
    }
    else
    {
        ms_pkMode = 0;
    }

    return bReturn;
}
//---------------------------------------------------------------------------
bool NiDX9Select::UpdateMultisample(NiWindowRef pDialog)
{
    bool bReturn = true;

    int iIndex = SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
        CB_GETCURSEL, 0, 0);
    if (iIndex >= 0)
    {
        unsigned int uiMultisample = (unsigned int)SendDlgItemMessage(
            pDialog, IDC_MULTISAMPLE_COMBO, CB_GETITEMDATA, iIndex, 0);
        bReturn = (uiMultisample != ms_uiMultisample);

        ms_uiMultisample = uiMultisample;
    }
    else
    {
        ms_uiMultisample = NiDX9Renderer::FBMODE_DEFAULT;
    }

    return bReturn;
}
//---------------------------------------------------------------------------
BOOL CALLBACK NiDX9Select::Callback(NiWindowRef pDialog, unsigned int uiMsg, 
    WPARAM wParam, LPARAM lParam)
{
    switch(uiMsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowText(pDialog, L"Select DX9 Display Device");

            SendDlgItemMessage(pDialog, IDC_MODE_COMBO, CB_RESETCONTENT, 0, 
                0);

            // Add "windowed" and "fullscreen" to the mode combo box
            // and set the item data to be true for fullscreen and
            // false for windowed mode
            int iIndex = SendDlgItemMessage(pDialog, IDC_MODE_COMBO, 
                CB_ADDSTRING, 0, (LPARAM)L"Windowed");
            SendDlgItemMessage(pDialog, IDC_MODE_COMBO, CB_SETITEMDATA, 
                (WPARAM)iIndex, (LPARAM)false);

            SendDlgItemMessage(pDialog, IDC_MODE_COMBO, CB_SETCURSEL, 
                (WPARAM)0, 0L);
            ms_bFullscreen = false;

            iIndex = SendDlgItemMessage(pDialog, IDC_MODE_COMBO, 
                CB_ADDSTRING, 0, (LPARAM)L"Fullscreen");
            SendDlgItemMessage(pDialog, IDC_MODE_COMBO, CB_SETITEMDATA, 
                (WPARAM)iIndex, (LPARAM)true);

            ms_bDialogOpen = true;

            // Find all of the Adapters and add them to the
            // combo box interface
			EnumerateLocale(pDialog);
            EnumerateAdapters(pDialog);
            EnumerateResolutions(pDialog);
            EnumerateMultisamples(pDialog);

            // Check 24/32-bit Zbuffer as default
            CheckDlgButton(pDialog, IDC_ZBUFFER_CHECK, BST_CHECKED);
			// Pack Data Use Default
#ifdef _DEBUG
			CheckDlgButton(pDialog, IDC_USE_PACK_CHECK, BST_UNCHECKED);
			CheckDlgButton(pDialog, IDC_USE_SOUND_CHECK, BST_UNCHECKED);
			CheckDlgButton(pDialog, IDC_USE_DEBUG_CHECK, BST_UNCHECKED);
#else
			CheckDlgButton(pDialog, IDC_USE_PACK_CHECK, BST_CHECKED);
			CheckDlgButton(pDialog, IDC_USE_SOUND_CHECK, BST_CHECKED);
			CheckDlgButton(pDialog, IDC_USE_DEBUG_CHECK, BST_UNCHECKED);
#endif
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_MODE_COMBO:
                {
                    if (!ms_bDialogOpen)
						  {
                        return TRUE;
							}

                    // if the user has changed the screen mode, then we
                    // must regenerate the Adapter list box, as we will
                    // enumerate different sets of devices based on the
                    // settings (e.g. in windowed mode, we only enumerate
                    // windowed display cards)
                    int iIndex = SendDlgItemMessage(pDialog, IDC_MODE_COMBO, 
                        CB_GETCURSEL, 0, 0);

                    bool bOldMode = ms_bFullscreen;
                    if (iIndex >= 0)
                    {
                        ms_bFullscreen = SendDlgItemMessage(pDialog, 
                            IDC_MODE_COMBO, CB_GETITEMDATA, iIndex, 0) ? 
                            true : false;
                    }

                    if (bOldMode != ms_bFullscreen)
                    {
                        EnumerateAdapters(pDialog);
                        EnumerateResolutions(pDialog);
                        EnumerateMultisamples(pDialog);
                    }

                    return TRUE;
                }
                case IDC_ADAPTER_COMBO:
                {
                    if (!ms_bDialogOpen)
                        return TRUE;

                    // only regenerate resolutions and multisamples if
                    // the Adapter has changed
                    bool bNewAdapter = UpdateAdapter(pDialog);

                    if (bNewAdapter)
                    {
                        EnumerateResolutions(pDialog);
                        EnumerateMultisamples(pDialog);
                    }

                    return TRUE;
                }
                case IDC_RESOLUTION_COMBO:
                {
                    if (!ms_bDialogOpen)
                        return TRUE;

                    bool bNewResolution = UpdateResolution(pDialog);

                    if (bNewResolution)
                    {
                        EnumerateMultisamples(pDialog);
                    }

                    return TRUE;
                }
                case IDOK:
                {
							// on acceptance, find the selected Adapter
							UpdateLocale(pDialog);
							UpdateAdapter(pDialog);
							UpdateResolution(pDialog);
							UpdateMultisample(pDialog);

							ms_bDialogOpen = false;

							ms_b32BitZBuffer = (BST_CHECKED & SendDlgItemMessage(pDialog, IDC_ZBUFFER_CHECK, BM_GETSTATE, 0, 0));
							ms_bBloom = (BST_CHECKED & SendDlgItemMessage(pDialog, IDC_BLOOM_CHECK, BM_GETSTATE, 0, 0));
                  			g_bUsePackData = (BST_CHECKED & SendDlgItemMessage(pDialog, IDC_USE_PACK_CHECK, BM_GETSTATE, 0, 0));
							g_bUseSound = (BST_CHECKED & SendDlgItemMessage(pDialog, IDC_USE_SOUND_CHECK, BM_GETSTATE, 0, 0));
							g_bUseDebugInfo = (BST_CHECKED & SendDlgItemMessage(pDialog, IDC_USE_DEBUG_CHECK, BM_GETSTATE, 0, 0));

							SendDlgItemMessage(pDialog, IDC_LOCALE_COMBO, 
								CB_RESETCONTENT, 0, 0);

                    SendDlgItemMessage(pDialog, IDC_MODE_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    EndDialog(pDialog, 1);
                    return TRUE;
                }
                case IDCANCEL:
                {
                    // on cancel, mark the box as cancelled and return
                    ms_bDialogOpen = false;

                    SendDlgItemMessage(pDialog, IDC_MODE_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_ADAPTER_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_RESOLUTION_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    SendDlgItemMessage(pDialog, IDC_MULTISAMPLE_COMBO, 
                        CB_RESETCONTENT, 0, 0);
                    EndDialog(pDialog, 0);
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}
//---------------------------------------------------------------------------
void NiDX9Select::CopyToWideChar(unsigned short** ppusStrOut, 
    char const* pcStrIn)
{
    unsigned int uiLen  = lstrlenA(pcStrIn);
    unsigned short* pStrOut = *ppusStrOut;

    if (uiLen)
        uiLen = MultiByteToWideChar(CP_ACP, 0, pcStrIn, uiLen, 
		(LPWSTR)pStrOut, uiLen);
    pStrOut[uiLen++] = L'\0'; // Add the null terminator
    *ppusStrOut += uiLen;
}
//---------------------------------------------------------------------------
void NiDX9Select::AddDialogControl(unsigned short** ppusTemplate, 
    unsigned int uiStyle, short sSX, short sSY, short sCX, short sCY, 
    unsigned short usID, char const* pcStrClassName, char const* pcStrTitle)
{
    // DWORD align the current ptr
    DLGITEMTEMPLATE* pkTemplate = (DLGITEMTEMPLATE*)
        (((((ULONG)(*ppusTemplate))+3)>>2)<<2);

    pkTemplate->style = uiStyle | WS_CHILD | WS_VISIBLE;
    pkTemplate->dwExtendedStyle = 0L;
    pkTemplate->x = sSX;
    pkTemplate->y = sSY;
    pkTemplate->cx = sCX;
    pkTemplate->cy = sCY;
    pkTemplate->id = usID;

    *ppusTemplate = (unsigned short*)(++pkTemplate); // Advance ptr

    CopyToWideChar((unsigned short**)ppusTemplate, pcStrClassName);//Set Name
    CopyToWideChar((unsigned short**)ppusTemplate, pcStrTitle);    //Set Title

    (*ppusTemplate)++;          // Skip Extra Stuff
}
//---------------------------------------------------------------------------
DLGTEMPLATE* NiDX9Select::BuildAdapterSelectTemplate()
{
	// Allocate ample memory for building the template
	DLGTEMPLATE* pkDlgTemplate = NiExternalNew DLGTEMPLATE[100];
	if (0 == pkDlgTemplate)
	return 0;
	ZeroMemory(pkDlgTemplate, 100 * sizeof(DLGTEMPLATE));

	// Fill in the DLGTEMPLATE info
	DLGTEMPLATE* pkDT = pkDlgTemplate;
	pkDT->style = DS_MODALFRAME | DS_NOIDLEMSG | DS_SETFOREGROUND | DS_3DLOOK 
	| DS_CENTER | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU 
	| DS_SETFONT;
	pkDT->dwExtendedStyle = 0L;
	pkDT->cdit = 17;//컨트롤 갯수
	pkDT->x = 0;
	pkDT->y = 0;
	pkDT->cx = 200;
	pkDT->cy = 160;

	size_t const GabComboY = 15;
	size_t const GabButtonY = 10;

	size_t OffsetY = 5;

	// Add menu array, class array, dlg title, font size and font name
	unsigned short* pusW = (unsigned short*)(++pkDT);
	*pusW++ = L'\0';         // Set Menu array to nothing
	*pusW++ = L'\0';         // Set Class array to nothing
	CopyToWideChar((unsigned short**)&pusW, 
	"Select Desired DirectX9 Device");
	*pusW++ = 8;// Font Size
	CopyToWideChar(&pusW, "Arial"); // Font Name

	// Add the "Locale:" text
	AddDialogControl(&pusW, SS_LEFT, 5, OffsetY, 41, OffsetY+8, IDC_STATIC, "STATIC",	"Locale:");
	// Add the Localization select combobox
	AddDialogControl(&pusW, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 55, OffsetY, 133, 45, IDC_LOCALE_COMBO, "COMBOBOX", "");

	OffsetY += GabComboY;

	// Add the "Mode:" text
	AddDialogControl(&pusW, SS_LEFT, 5, OffsetY, 41, OffsetY+8, IDC_STATIC, "STATIC",	"Mode:");
	// Add the mode select combobox
	AddDialogControl(&pusW, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 55, OffsetY, 133, 45, IDC_MODE_COMBO, "COMBOBOX", "");

	OffsetY += GabComboY;
	
	// Add the "Resolution:" text
	AddDialogControl(&pusW, SS_LEFT, 5, OffsetY, 41, 13, IDC_STATIC, "STATIC", "Resolution:");
	// Add the resolution select combobox
	AddDialogControl(&pusW, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 55, OffsetY, 133, 45, 
		IDC_RESOLUTION_COMBO, "COMBOBOX", "");

	OffsetY += GabComboY;

	// Add the "Adapter:" text
	AddDialogControl(&pusW, SS_LEFT, 5, OffsetY, 41, 13, IDC_STATIC, "STATIC",
		"Adapter:");
	// Add the Adapter select combobox
	AddDialogControl(&pusW, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 55, OffsetY, 133, 45, IDC_ADAPTER_COMBO, "COMBOBOX", "");

	OffsetY += GabComboY;

	// Add the "MultiSample:" text
	AddDialogControl(&pusW, SS_LEFT, 5, OffsetY, 41, 13, IDC_STATIC, "STATIC", "multi sample:");
	// Add the multisample select combobox
	AddDialogControl(&pusW, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 55, OffsetY,
		133, 45, IDC_MULTISAMPLE_COMBO, "COMBOBOX", "");

	OffsetY += GabComboY;

	AddDialogControl(&pusW, BS_AUTOCHECKBOX | WS_TABSTOP, 40, OffsetY, 150, 14, 
		IDC_ZBUFFER_CHECK, "BUTTON", 	"Enable 24/32 bit z-buffer");

	OffsetY += GabButtonY;

	AddDialogControl(&pusW, BS_AUTOCHECKBOX | WS_TABSTOP, 40, OffsetY, 150, 14, 
		IDC_BLOOM_CHECK, "BUTTON",	"bloom");

	OffsetY += GabButtonY;

	AddDialogControl(&pusW, BS_AUTOCHECKBOX | WS_TABSTOP, 40, OffsetY, 150, 14, 
		IDC_USE_PACK_CHECK, "BUTTON", "Pack Data On/Off");

	OffsetY += GabButtonY;

	AddDialogControl(&pusW, BS_AUTOCHECKBOX | WS_TABSTOP, 40, OffsetY, 150, 14, 
		IDC_USE_SOUND_CHECK, "BUTTON", "Sound On/Off");

	OffsetY += GabButtonY;

	AddDialogControl(&pusW, BS_AUTOCHECKBOX | WS_TABSTOP, 40, OffsetY, 150, 14, 
		IDC_USE_DEBUG_CHECK, "BUTTON", "Debug Info On/Off");

	OffsetY += GabComboY;

	// Add the cancel button
	AddDialogControl(&pusW, BS_PUSHBUTTON | WS_TABSTOP, 123, OffsetY, 51, 14, IDCANCEL, "BUTTON", "CANCEL");

	// Add the okay button
	AddDialogControl(&pusW, BS_PUSHBUTTON | WS_TABSTOP, 40, OffsetY, 51, 14, IDOK, "BUTTON", "OK");

	return pkDlgTemplate;
}
//---------------------------------------------------------------------------
NiDX9Select::ReturnVal NiDX9Select::Select(NiWindowRef pkWnd, 
    unsigned int uiDefaultWidth, unsigned int uiDefaultHeight,
    bool& bFullscreen, bool& b32BitZBuffer, void const*& pvAdapter, 
    void const*& pvDevice, void const*& pkMode, unsigned int& uiFBMode)
{
    bFullscreen = false;

    ms_uiDefaultWidth = uiDefaultWidth;
    ms_uiDefaultHeight = uiDefaultHeight;

    // Pop up a dialog box for the user's choice of Adapter/device/mode
    NiInstanceRef pInstance = (NiInstanceRef)
        GetWindowLong(pkWnd, GWL_HINSTANCE);

    // Create dynamic dialog template
    DLGTEMPLATE* pkDlgSelect = BuildAdapterSelectTemplate();
    unsigned int uiReturn = 0;
    if (pkDlgSelect)
    {
        // Create dialog box from template
        uiReturn = DialogBoxIndirectParam(pInstance, pkDlgSelect, pkWnd, 
            (DLGPROC)Callback, 0L);
        NiExternalDelete[] pkDlgSelect;
    }

    if (uiReturn)
    {
        pvAdapter = ms_pkAdapter;
        pvDevice = ms_pkDevice;

        bFullscreen = ms_bFullscreen;

        if (ms_bFullscreen)
            pkMode = ms_pkMode;
        else
            pkMode = 0;

        b32BitZBuffer = ms_b32BitZBuffer;
        
        uiFBMode = ms_uiMultisample;

        return RETURN_OK;
    }
    else
    {
        return RETURN_CANCEL;
    }
}
//---------------------------------------------------------------------------
NiRenderer* NiDX9Select::DialogCreateRenderer(NiWindowRef pkTopLevelWnd, 
    NiWindowRef pkRenderWnd, unsigned int uiDefaultWidth, 
    unsigned int uiDefaultHeight, bool bStencil, bool bMultiThread, 
    bool bRefRast, bool bSWVertex, bool& bNVPerfHUD, bool& bFullscreen)
{
    NiRenderer* pkRenderer = 0;

    // save the position and size of the window in case we have to
    // put the window back there later
    RECT kRect;
    GetWindowRect(pkTopLevelWnd, &kRect);

	NiDX9Renderer::PresentationInterval kPresentationInterval = g_bVSync ? NiDX9Renderer::PRESENT_INTERVAL_ONE : NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;

    while (!pkRenderer)
    {
        unsigned int eFlags = 0;
        
        if (bStencil)
            eFlags |= NiDX9Renderer::USE_STENCIL;

        if (bMultiThread)
            eFlags |= NiDX9Renderer::USE_MULTITHREADED;

        const NiDX9AdapterDesc::ModeDesc* pkMode = 0;
        const NiDX9DeviceDesc* pkDevice = 0;
        const NiDX9AdapterDesc* pkAdapter = 0;
        bool bUse32BitZBuffer = 0;
        unsigned int uiFBMode = 0;

        // select and set up the user's desired DirectDraw object
        // and return the desired renderer mode and display mode
        NiDX9Select::ReturnVal eRet = NiDX9Select::Select(pkTopLevelWnd,
            uiDefaultWidth, uiDefaultHeight,  bFullscreen, bUse32BitZBuffer, 
            (void const *&)pkAdapter, (void const *&)pkDevice, 
            (void const *&)pkMode, uiFBMode);

        if (eRet == NiDX9Select::RETURN_CANCEL)
            return false;

        if (!bUse32BitZBuffer)
            eFlags |= NiDX9Renderer::USE_16BITBUFFERS;

        // select fullscreen or windowed, based on user selection
        if (bFullscreen)
        {
            eFlags |= NiDX9Renderer::USE_FULLSCREEN;
 
            if (pkAdapter && pkDevice && pkMode)
            {
                NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;

                if (bRefRast || pkDevice->GetDeviceType() == D3DDEVTYPE_REF) 
                    eDesc = NiDX9Renderer::DEVDESC_REF;
                else if (bSWVertex)
                    eDesc = NiDX9Renderer::DEVDESC_HAL_SWVERTEX;

                // Handle nVidia's NVPerfHUD adapter in a special manner
                char const* pcAdapterName = pkAdapter->GetStringDescription();
                if (strstr(pcAdapterName, "NVPerfHUD"))
                {
                    bNVPerfHUD = true;
                    if (bRefRast || bSWVertex)
                        eDesc = NiDX9Renderer::DEVDESC_REF;
                    else
                        eDesc = NiDX9Renderer::DEVDESC_REF_HWVERTEX;
                }

                // create and return the renderer
                pkRenderer = NiDX9Renderer::Create(
                    pkMode->m_uiWidth, pkMode->m_uiHeight, 
                    (NiDX9Renderer::FlagType)eFlags, 
                    pkTopLevelWnd, pkTopLevelWnd, pkAdapter->GetAdapterIndex(),
                    eDesc, NiDX9Renderer::GetNiFBFormat(
                    pkMode->m_eD3DFormat),NiDX9Renderer::DSFMT_UNKNOWN,
                    kPresentationInterval,
                    NiDX9Renderer::SWAPEFFECT_DISCARD, uiFBMode | NiDX9Renderer::FBMODE_LOCKABLE);
                // Change PRESENT_INTERVAL_IMMEDIATE to PRESENT_INTERVAL_IMMEDIATE to
                // lock to v-sync.
            }
            else
            {
                // create and return the renderer
                pkRenderer = NiDX9Renderer::Create(640, 480, 
                    (NiDX9Renderer::FlagType)eFlags, 
                    pkTopLevelWnd, pkTopLevelWnd);
            }
        }
        else
        {
            NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;
            if (bRefRast || pkDevice->GetDeviceType() == D3DDEVTYPE_REF) 
                eDesc = NiDX9Renderer::DEVDESC_REF;
            else if (bSWVertex)
                eDesc = NiDX9Renderer::DEVDESC_HAL_SWVERTEX;

            // Handle nVidia's NVPerfHUD adapter in a special manner
            char const* pcAdapterName = pkAdapter->GetStringDescription();
            if (strstr(pcAdapterName, "NVPerfHUD"))
            {
                if (bRefRast)
                    eDesc = NiDX9Renderer::DEVDESC_REF;
                else if (bSWVertex)
                    eDesc = NiDX9Renderer::DEVDESC_REF_MIXEDVERTEX;
                else
                    eDesc = NiDX9Renderer::DEVDESC_REF_HWVERTEX;
            }

            // create and return the renderer
            pkRenderer = NiDX9Renderer::Create(0, 0, 
                (NiDX9Renderer::FlagType)eFlags, pkRenderWnd, pkTopLevelWnd, 
                pkAdapter->GetAdapterIndex(), eDesc, 
                NiDX9Renderer::FBFMT_UNKNOWN, NiDX9Renderer::DSFMT_UNKNOWN,
                kPresentationInterval,
                NiDX9Renderer::SWAPEFFECT_DISCARD, uiFBMode | NiDX9Renderer::FBMODE_LOCKABLE);
            // Change PRESENT_INTERVAL_IMMEDIATE to PRESENT_INTERVAL_IMMEDIATE to
            // lock to v-sync.
        }

        if (!pkRenderer)
        {
            // Put the window back where it was if the renderer
            // creation failed.  If this is not done, then the
            // window could be left at the size of the last attempted
            // fullscreen renderer
            MoveWindow(pkTopLevelWnd, kRect.left, kRect.top, 
                kRect.right - kRect.left, kRect.bottom-kRect.top, TRUE);
        }
    }

	//bBloom = ms_bBloom;

    return pkRenderer;
}
//---------------------------------------------------------------------------
NiRendererPtr NiDX9Select::CreateRenderer(NiWindowRef pkTopLevelWnd, 
    NiWindowRef pkRenderWnd, bool bDialog, unsigned int uiBitDepth, 
    unsigned int uiWidth, unsigned int uiHeight, unsigned int uiRefreshRate,bool bStencil,
    bool bMultiThread, bool bRefRast, bool bSWVertex, bool& bNVPerfHUD,
    bool& bFullscreen,bool &bVSync)
{
    ms_pkInfo = NiDX9Renderer::GetSystemDesc();

    if (ms_pkInfo == 0)
        return 0;

    NiRendererPtr spRenderer = 0;

	NiDX9Renderer::PresentationInterval kPresentationInterval = bVSync ? NiDX9Renderer::PRESENT_INTERVAL_ONE : NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;

    if (!bDialog || uiBitDepth != 0)
    {
        unsigned int eFlags = 0;
    
        if (bStencil)
            eFlags |= NiDX9Renderer::USE_STENCIL;

        if (bMultiThread)
            eFlags |= NiDX9Renderer::USE_MULTITHREADED;

        unsigned int uiAdapter = D3DADAPTER_DEFAULT;
        NiDX9Renderer::DeviceDesc eDevType = NiDX9Renderer::DEVDESC_PURE;
        if (bNVPerfHUD)
        {
            // Look for 'NVIDIA NVPerfHUD' adapter
            // If it is present, override default settings
            assert (ms_pkInfo);
            unsigned int uiAdapterCount = ms_pkInfo->GetAdapterCount();

            unsigned int i;
            for (i = 0; i < uiAdapterCount; i++)
            {
                const NiDX9AdapterDesc* pkAdapter = ms_pkInfo->GetAdapter(i);
                char const* pcAdapterName = pkAdapter->GetStringDescription();

                if (strstr(pcAdapterName,"PerfHUD"))
                {
                    bNVPerfHUD = true;
                    uiAdapter = i;
                    eDevType = NiDX9Renderer::DEVDESC_REF;
                    break;
                }
            }
        }

        if (uiBitDepth == 16 || uiBitDepth == 32)
        {
            eFlags |= NiDX9Renderer::USE_FULLSCREEN;

            if (uiBitDepth == 16)
                eFlags |= NiDX9Renderer::USE_16BITBUFFERS;

			//NILOG(PGLOG_MINOR, "select %d, %d, %d, %d, %d\n", uiWidth, uiHeight, eFlags, uiAdapter, eDevType);
            spRenderer = NiDX9Renderer::Create(uiWidth, uiHeight, 
                (NiDX9Renderer::FlagType)eFlags, 
                pkTopLevelWnd, pkTopLevelWnd, uiAdapter, eDevType, 
                NiDX9Renderer::FBFMT_UNKNOWN, 
                NiDX9Renderer::DSFMT_UNKNOWN,
				kPresentationInterval, NiDX9Renderer::SWAPEFFECT_DISCARD, NiDX9Renderer::FBMODE_MULTISAMPLES_2,
				1,uiRefreshRate);
				//NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE,NiDX9Renderer::SWAPEFFECT_DISCARD, NiDX9Renderer::FBMODE_LOCKABLE);
            // Change PRESENT_INTERVAL_IMMEDIATE to PRESENT_INTERVAL_IMMEDIATE to
            // lock to v-sync.
        }
        else        {
            if (bFullscreen)
            {
                eFlags |= NiDX9Renderer::USE_FULLSCREEN;
    
                spRenderer = NiDX9Renderer::Create(uiWidth, uiHeight, 
                    (NiDX9Renderer::FlagType)eFlags, 
                    pkTopLevelWnd, pkTopLevelWnd, uiAdapter, eDevType, 
                    NiDX9Renderer::FBFMT_UNKNOWN, 
                    NiDX9Renderer::DSFMT_UNKNOWN,
                    kPresentationInterval,NiDX9Renderer::SWAPEFFECT_DISCARD, NiDX9Renderer::FBMODE_MULTISAMPLES_2,
					1,uiRefreshRate);
                // Change PRESENT_INTERVAL_IMMEDIATE to PRESENT_INTERVAL_IMMEDIATE to
                // lock to v-sync.
            }
            else
            {
                spRenderer = NiDX9Renderer::Create(0, 0, 
                    (NiDX9Renderer::FlagType)eFlags, 
                    pkRenderWnd, pkRenderWnd, uiAdapter, eDevType, 
                    NiDX9Renderer::FBFMT_UNKNOWN, 
                    NiDX9Renderer::DSFMT_UNKNOWN,
                    kPresentationInterval,NiDX9Renderer::SWAPEFFECT_DISCARD, NiDX9Renderer::FBMODE_MULTISAMPLES_2,
					1,uiRefreshRate);
                // Change PRESENT_INTERVAL_IMMEDIATE to PRESENT_INTERVAL_IMMEDIATE to
                // lock to v-sync.
            }
        }
    }    
    else
    {
        spRenderer = DialogCreateRenderer(pkTopLevelWnd, pkRenderWnd, 
            uiWidth, uiHeight, bStencil, bMultiThread, bRefRast, bSWVertex, 
            bNVPerfHUD, bFullscreen);
    }

	//bBloom = ms_bBloom;
    
    return spRenderer;
}
//---------------------------------------------------------------------------

#endif //#if defined(_DX9)
