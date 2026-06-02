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

#include "NiApplication.h"
#include "NiApplicationMetrics.h"
#include <NiFragmentMaterial.h>
#include <NiTNodeTraversal.h>
#include <commctrl.h>
#include "../PgMobileSuit.h"
#include "../lwUI.h"
// Win32 NiApplication used to automatically insert references to 
// NiSystem.lib and NiMain.lib via a #pragma comment directive.
// These libraries are still required by any NiApplication-derived 
// application, but they must now be linked in explicitly by the app.
// Use of the #pragma NiSystem.lib reference is incompatible with 
// NiSystem's global operator new overload, which is enabled
// if NI_USE_MEMORY_MANAGEMENT is defined.

#if defined(_DX9)
    #pragma message("Including NiDX9Renderer in NiApplication")
    #include "NiDX9Select.h"
#else
    #error NI ERROR:  Either _DX9 must be defined
#endif

#if NIMETRICS
	#ifdef NIMETRICSOUTPUT_IMPORT
		#pragma comment(lib, "NiMetricsOutput" NI_DLL_SUFFIX ".lib")
	#else
		#pragma comment(lib, "NiMetricsOutput.lib")
	#endif // #ifdef NIMETRICSOUTPUT_IMPORT
#endif // #if NIMETRICS

NiApplication* NiApplication::ms_pkApplication = 0;

NiInstanceRef NiApplication::ms_pInstance = 0;
NiAcceleratorRef NiApplication::ms_pAccel = 0;
NiCommand* NiApplication::ms_pkCommand = 0;

char NiApplication::ms_acMediaPath[NI_MAX_PATH] = "./../../Data/";
char NiApplication::ms_acTempMediaFilename[NI_MAX_PATH];

bool NiApplication::m_bDefaultResolution( true );
POINT2 NiApplication::m_sptResolutionSize( XUI::EXV_DEFAULT_SCREEN_WIDTH, XUI::EXV_DEFAULT_SCREEN_HEIGHT );

//#define PG_USE_WIN_MESSAGE_STAT

//---------------------------------------------------------------------------
NiApplication::NiApplication(char const* pcWindowCaption, 
    unsigned int uiWidth, unsigned int uiHeight, unsigned int uiMenuID, 
    unsigned int uiNumStatusPanes, unsigned int uiBitDepth) :
    m_kScreenElements(4, 4),
    m_kScreenTextures(4, 4),
    m_kVisible(1024, 1024),
    m_kCuller(&m_kVisible)
{
    m_pkAppWindow = NiNew NiAppWindow(pcWindowCaption, uiWidth, uiHeight, 
        uiNumStatusPanes);

    m_uiMenuID = uiMenuID;
    m_uiBitDepth = uiBitDepth;

    // performance measurements (display in status pane 0 if enabled)
    m_fLastTime = -1.0f;
    m_fAccumTime = 0.0f;
    m_fCurrentTime = 0.0f;
    m_fFrameTime = 0.0f;
    m_iClicks = 0;
    m_iTimer = 1;
    m_iMaxTimer = 30;
    m_fFrameRate = 0.0f;
    m_fLastFrameRateTime = 0.0f;
    m_iLastFrameRateClicks = 0;

    m_fMinFramePeriod = 1.0f / 100.0f;
    m_fLastFrame = 0.0f;
    m_fCullTime = 0.0f;
    m_fRenderTime = 0.0f;
    m_fUpdateTime = 0.0f;
    m_fBeginUpdate = 0.0f;
    m_fBeginCull = 0.0f;
    m_fBeginRender = 0.0f;

    // Renderer defaults, change these in the NiApplication-derived class
    // constructor from command line information or other.

	m_sptResolutionSize.x = 0;
	m_sptResolutionSize.y = 0;	

    if (m_uiBitDepth == 0)
        m_bFullscreen = false;
    else
        m_bFullscreen = true;
    m_bStencil = false;
    m_bRendererDialog = false;
    m_bMultiThread = true;

    m_pkFrameRate = 0;
    m_bFrameRateEnabled = false;

    // Set the renderer
    OSVERSIONINFO kInfo;
    kInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&kInfo);

    NiImageConverter::SetImageConverter(NiNew NiDevImageConverter);
    NiTexture::SetMipmapByDefault(true);

    // the unique application instance
    ms_pkApplication = this;

    m_bRefRast = false;
    m_bSWVertex = false;
    m_bExclusiveMouse = false;
    m_bNVPerfHUD = false;
    m_bShowAllTrackers = false;
	m_bMinimized = false;
	m_bDestroying = false;
#ifdef PG_USE_WIN_MESSAGE_STAT
	m_bCheckMessageStat = false;
#endif

	m_fVTPerformanceMax = 100.0f;
    m_fVTMemoryMax = 10000.0f;
    m_fVTTimeMax = 100.0f;
}
//---------------------------------------------------------------------------
NiApplication::~NiApplication()
{
    // Ensure image converter that was used during application is deleted now
    // to free any NiObject derived members contained in NiStreams of image
    // readers.  Otherwise NiApplication will claim object leaks.
    NiImageConverter::SetImageConverter(NiNew NiImageConverter);

    SAFE_DELETE_NI(ms_pkCommand);

    SAFE_DELETE_NI(m_pkAppWindow);


    ms_pkApplication = 0;
    ms_pkCommand = 0;
}
//---------------------------------------------------------------------------
void NiApplication::SetCommandLine(char* pcCommandLine)
{
    if (pcCommandLine && strlen(pcCommandLine) > 0)
        ms_pkCommand = NiNew NiCommand(pcCommandLine);
}
//---------------------------------------------------------------------------
void NiApplication::SetCommandLine(int iArgc, char** ppcArgv)
{
    ms_pkCommand = NiNew NiCommand(iArgc, ppcArgv);
}
//---------------------------------------------------------------------------
void NiApplication::CommandSelectRenderer()
{
    // process renderer-specific command line options
    if (ms_pkCommand)
    {
        if (ms_pkCommand->Boolean("full"))
        {
            ms_pkApplication->m_bFullscreen = true;
            ms_pkApplication->m_bRendererDialog = false;
        }
        if (ms_pkCommand->Boolean("refrast"))
            ms_pkApplication->m_bRefRast = true;
        else if (ms_pkCommand->Boolean("swvertex"))
            ms_pkApplication->m_bSWVertex = true;

        if (ms_pkCommand->Boolean("nvperf"))
            ms_pkApplication->m_bNVPerfHUD = true;
    }
}
//---------------------------------------------------------------------------
bool NiApplication::CreateScene()
{
    m_spScene = NiNew NiNode;
    return true;
}
//---------------------------------------------------------------------------
bool NiApplication::CreateCamera()
{
    m_spCamera = NiNew NiCamera;
    PG_ASSERT_LOG(m_spCamera);

    float fAspectRatio = 4.f/3.f;

    float fVerticalFieldOfViewDegrees = 20;
    float fVerticalFieldOfViewRad = NI_PI / 180 * fVerticalFieldOfViewDegrees;
    float fViewPlaneHalfHeight = tanf(fVerticalFieldOfViewRad / 2);
    float fViewPlaneHalfWidth = fViewPlaneHalfHeight * fAspectRatio;

    NiFrustum kFrustum = NiFrustum(
        -fViewPlaneHalfWidth, fViewPlaneHalfWidth, 
        fViewPlaneHalfHeight, -fViewPlaneHalfHeight,
        1.0f, 5000.0f);
    NiRect<float> kPort(0.0f, 1.0f, 1.0f, 0.0f);
    m_spCamera->SetViewFrustum(kFrustum);
    m_spCamera->SetViewPort(kPort);

	g_kFrustum = kFrustum;

    return true;
}
//---------------------------------------------------------------------------
struct FindFirstCameraFunctor
{
    NiCamera* m_pkCamera;

    FindFirstCameraFunctor() : m_pkCamera(0) {}

    bool operator() (NiAVObject* pkObject)
    {
        m_pkCamera = NiDynamicCast(NiCamera, pkObject);
        return m_pkCamera == 0; // if no camera, continue traversal
    }
};
//---------------------------------------------------------------------------
NiCamera* NiApplication::FindFirstCamera(NiNode* pkNode)
{
    FindFirstCameraFunctor kFindFirstCameraFunctor;
    NiTNodeTraversal::DepthFirst_FirstStop(
        pkNode,
        kFindFirstCameraFunctor);

    return kFindFirstCameraFunctor.m_pkCamera;
}
//---------------------------------------------------------------------------
void NiApplication::AdjustCameraAspectRatio(NiCamera* pkCamera)
{
    PG_ASSERT_LOG(m_spRenderer);
    if (!m_spRenderer)
        return;

    // Correct camera's aspect ratio
    Ni2DBuffer* pkBackbuffer = m_spRenderer->GetDefaultBackBuffer();
    pkCamera->AdjustAspectRatio(
        pkBackbuffer->GetWidth() / (float) pkBackbuffer->GetHeight());
}
//---------------------------------------------------------------------------
bool NiApplication::CreateRenderer()
{
    char const* pcWorkingDir = ConvertMediaFilename("Shader\\Generated");
    NiMaterial::SetDefaultWorkingDirectory(pcWorkingDir);

    NiWindowRef pWnd;
    if (m_bFullscreen)
        pWnd = m_pkAppWindow->GetWindowReference();
    else
        pWnd = m_pkAppWindow->GetRenderWindowReference();

    char acErrorStr[512];
	bool	bVSync = true;

#if defined(_DX9)
    m_spRenderer = NiDX9Select::CreateRenderer(
        m_pkAppWindow->GetWindowReference(), 
        m_pkAppWindow->GetRenderWindowReference(), m_bRendererDialog, 
        m_uiBitDepth, m_pkAppWindow->GetWidth(), 
		m_pkAppWindow->GetHeight(), NiDX9Renderer::REFRESHRATE_DEFAULT,m_bStencil,
        m_bMultiThread, m_bRefRast, m_bSWVertex, m_bNVPerfHUD,
        m_bFullscreen,bVSync);    
#endif

    if (m_spRenderer == 0)
    {
#if defined(_DX9)
        NiStrcpy(acErrorStr, 512, "DX9 Renderer Creation Failed");		
#endif
        return false;
    }
    else
    {
        m_spRenderer->SetBackgroundColor(NiColor(0.5f, 0.5f, 0.5f));
    }

    return true;
}
//---------------------------------------------------------------------------
bool NiApplication::CreateInputSystem()
{
	// Create and initialize parameters for the input system
	NiInputSystem::CreateParams* pkParams = GetInputSystemCreateParams();

	//	Create the input system
	m_spInputSystem = NiInputSystem::Create(pkParams);

    SAFE_DELETE_NI(pkParams);

	if (!m_spInputSystem)
    {
        PgError("CreateInputSystem: Creation failed.");
        return false;
    }
    // The creation of the input system automatically starts an enumeration
    // of the devices. 
	NiInputErr eErr = m_spInputSystem->CheckEnumerationStatus();
    switch(eErr)
    {
    case NIIERR_ENUM_NOTRUNNING:
        PG_ASSERT_LOG(!"EnumerateDevices failed?");
        return false;
    case NIIERR_ENUM_FAILED:
        PG_ASSERT_LOG(!"CheckEnumerationStatus> FAILED!");
        return false;
    case NIIERR_ENUM_COMPLETE:
    case NIIERR_ENUM_NOTCOMPLETE:
    default:
        break;
    }

    // On Win32, assume there is a mouse and keyboard
    if (!m_spInputSystem->OpenMouse() || !m_spInputSystem->OpenKeyboard())
    {
        PgError("CreateInputSystem: Mouse or keyboard failed to open.");
        return false;
    }

	PgTimeCheck kTimeCheck( dynamic_cast<PgLogWorker_Base*>(&g_kLogWorker), LT_CAUTION, __FUNCTIONW__, __LINE__);
    // Gamepad may not exist, but attempt to open any
    for (unsigned int iGamePad = 0; 
        iGamePad < NiInputSystem::MAX_GAMEPADS;
        iGamePad++
        )
    {
        m_spInputSystem->OpenGamePad(iGamePad,0);
    }
    return true;
}

//---------------------------------------------------------------------------
// This function is platform-specific...
// It is provided here to allow applications to setup the creation parameters
// for the input system to their liking, but still allow the NiApplication
// framework to create the input system.
//---------------------------------------------------------------------------
NiInputSystem::CreateParams* NiApplication::GetInputSystemCreateParams()
{
    NiDI8InputSystem::DI8CreateParams* pkParams = 
        NiNew NiDI8InputSystem::DI8CreateParams();
    PG_ASSERT_LOG(pkParams);

    pkParams->SetRenderer(m_spRenderer);
    pkParams->SetKeyboardUsage(
        NiInputSystem::FOREGROUND | NiInputSystem::NONEXCLUSIVE);

    unsigned int uiMouseFlags = NiInputSystem::FOREGROUND;
    if (m_bExclusiveMouse)
    {
        uiMouseFlags |= NiInputSystem::EXCLUSIVE;
    }
    else
    {
        uiMouseFlags |= NiInputSystem::NONEXCLUSIVE;
    }

    pkParams->SetMouseUsage(uiMouseFlags);
    pkParams->SetGamePadCount(2);
    pkParams->SetAxisRange(-100, +100);
    pkParams->SetOwnerInstance(GetInstanceReference());
    pkParams->SetOwnerWindow(GetWindowReference());

    return pkParams;
}
//---------------------------------------------------------------------------
void NiApplication::ConfigureInputDevices()
{
    m_spInputSystem->ConfigureDevices();
}
//---------------------------------------------------------------------------
bool NiApplication::MeasureTime()
{
    // start performance measurements
    if (m_fLastTime == -1.0f)
    {
        m_fLastTime = NiGetCurrentTimeInSec();
        m_fAccumTime = 0.0f;
        m_iClicks = 0;
    }

    // measure time
    m_fCurrentTime = NiGetCurrentTimeInSec();
    float fDeltaTime = m_fCurrentTime - m_fLastTime;

    // NVPerfHUD support!
    if (m_bNVPerfHUD && fDeltaTime == 0.0f)
        return true;

    if (fDeltaTime < 0.0f)
        fDeltaTime = 0.0f;
    m_fLastTime = m_fCurrentTime;
    m_fAccumTime += fDeltaTime;

    // frame rate limiter
	if(GetMinimized())
	{
		if (m_fAccumTime < (m_fLastFrame + 1.0f ))
			return false;
	}
	else
	{
		if (m_fAccumTime < (m_fLastFrame + m_fMinFramePeriod))
			return false;
	}


    m_fFrameTime = m_fAccumTime - m_fLastFrame;
    m_fLastFrame = m_fAccumTime;

    return true;
}
//---------------------------------------------------------------------------
void NiApplication::DrawFrameRate()
{
    if (--m_iTimer == 0)
    {
        if (m_fAccumTime > 0.0f)
        {
            if (m_fAccumTime > m_fLastFrameRateTime)
            {
                m_fFrameRate = (m_iClicks - m_iLastFrameRateClicks) / 
                    (m_fAccumTime - m_fLastFrameRateTime);
            }
        }
        else
        {
            m_fFrameRate = 0.0f;
        }

        char acMsg[32];
        NiSprintf(acMsg, 32, "fps: %.1f", m_fFrameRate);

        unsigned int uiPane = 0;
        m_pkAppWindow->WriteToStatusWindow(uiPane, acMsg);
        
        m_iTimer = m_iMaxTimer;
        m_fLastFrameRateTime = m_fAccumTime;
        m_iLastFrameRateClicks = m_iClicks;
    }
}
//---------------------------------------------------------------------------
void NiApplication::SetMaxFrameRate(float fMax)
{
    // convert from rate to period - if a bogus max framerate is passed in,
    // disable framerate limitation
    if(fMax < 1e-5f)
        m_fMinFramePeriod = 1e-5f;    
    else
        m_fMinFramePeriod = 1.0f / fMax;    
}
//---------------------------------------------------------------------------
bool NiApplication::Initialize()
{

	if (!CreateRenderer())
	{
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
void NiApplication::Terminate()
{
    if (m_pkFrameRate)
    {
        m_pkFrameRate->Shutdown();
        SAFE_DELETE_NI(m_pkFrameRate);
    }

    m_kScreenElements.RemoveAll();
    m_kScreenTextures.RemoveAll();

#if 0 //NIMETRICS
    for (unsigned int ui = 0; ui < m_kTrackers.GetSize(); ui++)
    {
        NiVisualTracker* pkTracker = m_kTrackers.GetAt(ui);
        PG_ASSERT_LOG(pkTracker != 0);
       m_kTrackers.RemoveAt(ui);
    }
#endif

    m_spInputSystem = 0;
    m_spScene = 0;
    m_spCamera = 0;
    m_spRenderer = 0;
}

void NiApplication::setLocalDirectory()
{
	TCHAR szPath[MAX_PATH];
	HRESULT hModuleName = GetModuleFileName(GetModuleHandle(0), szPath, MAX_PATH);
	std::wstring strPath = szPath;
	strPath = strPath.substr(0,strPath.rfind(_T('\\')));
	SetCurrentDirectory(strPath.c_str());
}

//---------------------------------------------------------------------------
void NiApplication::CreateMenuBar()
{
}
//---------------------------------------------------------------------------
void NiApplication::EnableFrameRate(bool bEnable)
{
    if (m_pkFrameRate)
    {
        if (m_bFrameRateEnabled != bEnable)
        {
            if (bEnable)
                m_kScreenElements.Add(m_pkFrameRate->GetElements());
            else
                m_kScreenElements.Remove(m_pkFrameRate->GetElements());
        }
           
        m_bFrameRateEnabled = bEnable;
    }
}
//---------------------------------------------------------------------------
void NiApplication::SetShowAllTrackers(bool bShow)
{
#if 0 //NIMETRICS
    for (unsigned int ui = 0; ui < m_kTrackers.GetSize(); ui++)
    {
        NiVisualTracker* pkTracker = m_kTrackers.GetAt(ui);
        PG_ASSERT_LOG(pkTracker != 0);
       pkTracker->SetShow(bShow);
    }
#endif
    m_bShowAllTrackers = bShow;
}
//---------------------------------------------------------------------------
void NiApplication::HidePointer()
{
	//while (ShowCursor(false) >= 0)
	//ShowCursor(false);
	SetCursor(NULL);
}
//---------------------------------------------------------------------------
void NiApplication::ShowPointer()
{
    //while (ShowCursor(true) < 0)
	ShowCursor(true);	
}
//---------------------------------------------------------------------------
void NiApplication::SetMediaPath(char const* pcPath)
{
    NiStrcpy(ms_acMediaPath, NI_MAX_PATH, pcPath);
}
//---------------------------------------------------------------------------
char const* NiApplication::GetMediaPath()
{
    return ms_acMediaPath;
}
//---------------------------------------------------------------------------
char const* NiApplication::ConvertMediaFilename(char const* pcFilename)
{
    NiSprintf(ms_acTempMediaFilename, NI_MAX_PATH, "%s%s", ms_acMediaPath, 
        pcFilename);
    NiPath::Standardize(ms_acTempMediaFilename);

    return ms_acTempMediaFilename;
}
//---------------------------------------------------------------------------
void NiApplication::UpdateInput()
{
    if (!m_spInputSystem)
    {
        return;
    }

    // If there are action maps defined the update with action maps
    // if not then just update all of the devices
    if (m_spInputSystem->GetMode() == NiInputSystem::MAPPED)
    {
        if (m_spInputSystem->UpdateActionMap() == NIIERR_DEVICELOST)
            m_spInputSystem->HandleDeviceChanges();
    }
    else
    {
        if (m_spInputSystem->UpdateAllDevices() == NIIERR_DEVICELOST)
            m_spInputSystem->HandleDeviceChanges();
    }
}
//---------------------------------------------------------------------------
void NiApplication::ProcessInput()
{
/*
	ProcessVisualTrackerInput();
    // Minimally, we provide a way to quit if the NiApplication derived app 
    // does not override with it's own ProcessInput.
    NiInputKeyboard* pkKeyboard = GetInputSystem()->GetKeyboard();
    if (pkKeyboard)
    {
        if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_ESCAPE))
        {
            QuitApplication();
        }
        else if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_F2))
        {
            // We have overriden Initialize so that we can configure our
            // input devices.
            ConfigureInputDevices();
        }
    }

    // Allow ANY gamepad to exit the app
    NiInputGamePad* pkGamePad;
    for (unsigned int uiPort = 0; uiPort < NiInputSystem::MAX_GAMEPADS; 
        uiPort++)
    {
        pkGamePad = m_spInputSystem->GetGamePad(uiPort);
        if (pkGamePad)
        {
            if (pkGamePad->ButtonIsDown(NiInputGamePad::NIGP_START) &&
                pkGamePad->ButtonIsDown(NiInputGamePad::NIGP_SELECT))
            {
                QuitApplication();
            }
        }
    }
	*/
}

//---------------------------------------------------------------------------
void NiApplication::ProcessVisualTrackerInput()
{
/*
    NiInputKeyboard* pkKeyboard = GetInputSystem()->GetKeyboard();
    if (pkKeyboard)
    {
        if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_F11))
        {
            SetShowAllTrackers(!m_bShowAllTrackers);
        }
    }

    // Allow ANY gamepad to show vis trackers
    NiInputGamePad* pkGamePad;
    for (unsigned int uiPort = 0; uiPort < NiInputSystem::MAX_GAMEPADS; 
        uiPort++)
    {
        pkGamePad = m_spInputSystem->GetGamePad(uiPort);
        if (pkGamePad)
        {
            if (pkGamePad->ButtonWasPressed(NiInputGamePad::NIGP_B))
            {
                SetShowAllTrackers(!m_bShowAllTrackers);
            }
        }
    }
*/
}
//---------------------------------------------------------------------------
void NiApplication::MainLoop()
{
    while (TRUE)
    {
        if (!Process())
		{
            break;
		}
    }
}
//---------------------------------------------------------------------------
void NiApplication::QuitApplication()
{
    PostMessage(GetAppWindow()->GetWindowReference(), WM_DESTROY, 0, 0);
}
//---------------------------------------------------------------------------
LRESULT CALLBACK NiApplication::WinProc(HWND hWnd, UINT uiMsg, 
    WPARAM wParam, LPARAM lParam)
{
    NiApplication* pkTheApp = NiApplication::ms_pkApplication;

    if (!pkTheApp)
        return DefWindowProc(hWnd, uiMsg, wParam, lParam);

	bool bSkipDefProc = false;
	XUIMgr.VHookMessage( hWnd, uiMsg, wParam, lParam, bSkipDefProc );

	//Alt+space 를 막기위한 flag
	static bool bIsAltSpaceDown = false;

	// 베트남어 예외처리를 위한 flag
	static bool bIsVietnamCharacter = false;
	if( VK_PAUSE == wParam )
	{
		bIsVietnamCharacter = true;
	}

    switch(uiMsg) 
    {
	case WM_CLOSE:
		{
			if(!g_bWM_CLOSE) 
			{
				lwCallCommonMsgYesNoBox(MB(TTW(201300)), lwPacket(), true, MBT_WM_CLOSE);
				return 0;
			}
		}break;
    case WM_DESTROY:
        {
			pkTheApp->m_bDestroying = true;
            pkTheApp->OnWindowDestroy(0, false);
            PostQuitMessage(0);
            return 0;
        }break;
    case WM_SIZE:
        {
            int iWidth = int(LOWORD(lParam));
            int iHeight = int(HIWORD(lParam));
            unsigned int uiSizeType = (unsigned int)(wParam);
            if (pkTheApp->OnWindowResize(iWidth, iHeight, uiSizeType, 0))
				{
                return 0;
				}
        }break;
	case WM_SYSKEYDOWN:
		{
			if(VK_SPACE == wParam)
			{//알트스페이스가 눌렸으면
				bIsAltSpaceDown = true;
			}
			else
			{
				NiEventRecord kEvent = {hWnd, uiMsg, wParam, lParam};
				if (pkTheApp->OnDefault(&kEvent))
				{
					return 0;
				}
			}
		}
		break;
	case WM_SYSKEYUP:
		{	
			if(VK_SPACE == wParam)
			{//알트스페이스가 떼어졌으면
				bIsAltSpaceDown = false;
			}
			else
			{
				NiEventRecord kEvent = {hWnd, uiMsg, wParam, lParam};
				if (pkTheApp->OnDefault(&kEvent))
				{
					return 0;
				}
			}
		}
		break;
	case WM_SETCURSOR:
		{
			POINT ptMouse;

			GetCursorPos(&ptMouse);
			HWND	hWnd = WindowFromPoint(ptMouse);
			if(hWnd)
			{
				if(hWnd == pkTheApp->GetWindowReference())
				{
					SetCursor(NULL);
				}
			}
			return	false;
		}
		break;
	case WM_IME_SETCONTEXT:
		{
			lParam = 0;
			//lParam &= (~ISC_SHOWUICANDIDATEWINDOW);
			//lParam &= (~ISC_SHOWUICOMPOSITIONWINDOW);
			//lParam &= (~ISC_SHOWUIGUIDELINE);
			//lParam &= (~ISC_SHOWUIALL);
			// 한/영 키 눌렀을 때 기본 ime 윈도우가 뜨는 것 방지
			//ImmIsUIMessage(NULL, uiMsg, wParam, lParam);
		}break;
	case WM_IME_NOTIFY:
		{
			switch(wParam)
			{
			case IMN_CHANGECANDIDATE://0x0003
			case IMN_OPENCANDIDATE://0x0005
				{
					return true;
				}break;
			}
		}
	case WM_CHAR:// 베트남어 예외처리
		{
			if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_VIETNAM )
			&& ( lParam & ( 0x1 << 24 ) || bIsVietnamCharacter ) )
			{
				bIsVietnamCharacter = false;
				return ::SendMessage( XUIMgr.GetIME().GetEditHWND(), uiMsg, wParam, lParam );
			}
		}//no break
    default:
        {
			NiEventRecord kEvent = {hWnd, uiMsg, wParam, lParam};
            if (pkTheApp->OnDefault(&kEvent))
			{
                return 0;
			}
        }break;
    }

	if (bSkipDefProc)
		return 0;
 
	if(false != bIsAltSpaceDown)
	{//알트 스페이스는 다른 시스템키 조합과는 다르게 메세지가 왔을 때 바로 리턴시켜서 끊어도
		//다음 루프에 키값은 그대로 다시 들어옴. 따라서 알트스페이스가 눌렸으면 기본프로시저로 넘어가는
		//VK_SPACE 코드값을 제거해줘야 함. 뭔가 다른 방법이 있을것도 같은데 
		//잘 동작해서 이걸로 일단 사용.
		wParam = 0;
	}
    return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}
//---------------------------------------------------------------------------
// OnWindowResize() handles WM_SIZE
//   iWidth = width
//   iHeight = height
//   uiSizeType = fwSizeType
//   pWnd = unused
bool NiApplication::OnWindowResize(int iWidth, int iHeight, 
    unsigned int uiSizeType, NiWindowRef pWnd)
{
    m_pkAppWindow->SetParentWidth(iWidth);
    m_pkAppWindow->SetParentHeight(iHeight);
	m_bMinimized = bool(uiSizeType & SIZE_MINIMIZED);

    // let application know of new client dimensions
    RECT kRect;
    unsigned int uiNumPanes = m_pkAppWindow->GetNumStatusPanes();
    if (uiNumPanes && m_pkAppWindow->GetStatusPanesExist())
    {
        // resize the status window
        HWND hStatusWnd = m_pkAppWindow->GetStatusWindowReference();

        WPARAM wParam = uiSizeType;
        LPARAM lParam = MAKELPARAM(iWidth, iHeight);
        SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);

        // specify number of piPanes and repaint
        GetClientRect(m_pkAppWindow->GetWindowReference(), &kRect);
        int* piPanes = NiAlloc(int, uiNumPanes);
        for (unsigned int i = 0; i < uiNumPanes-1; i++)
            piPanes[i] = (i+1)*kRect.right/uiNumPanes;
        piPanes[uiNumPanes-1] = kRect.right;

        wParam = WPARAM(uiNumPanes);
        lParam = LPARAM(piPanes);
        SendMessage(hStatusWnd, SB_SETPARTS, wParam, lParam);

        wParam = 0;
        lParam = 0;
        SendMessage(hStatusWnd, WM_PAINT, wParam, lParam);

        NiFree(piPanes);
    }

    //HWND hRendWnd = m_pkAppWindow->GetRenderWindowReference();

    //if (hRendWnd)
    //{
    //    // resize the rendering window 
    //    RECT kMainRect;
    //    GetClientRect(GetWindowReference(), &kMainRect);
    //    int iClientWidth = kMainRect.right - kMainRect.left;
    //    int iClientHeight = kMainRect.bottom - kMainRect.top;

    //    // adjust the size to correct for a status window, if one exists
    //    if (m_pkAppWindow->GetStatusPanesExist())
    //    {
    //        RECT kStatusRect;
    //        GetWindowRect(m_pkAppWindow->GetStatusWindowReference(), 
    //            &kStatusRect);

    //        iClientHeight -= kStatusRect.bottom - kStatusRect.top;
    //    }

    //    MoveWindow(hRendWnd, 0, 0, iClientWidth, iClientHeight, FALSE);

    //    // redraw the status window
    //    if (m_pkAppWindow->GetStatusPanesExist())
    //    {
    //        SendMessage(m_pkAppWindow->GetStatusWindowReference(), WM_PAINT, 
    //            0, 0);
    //    }

    //    // get the new size of the rendering window
    //    GetWindowRect(hRendWnd, &kRect);
    //    m_pkAppWindow->SetWidth(kRect.right - kRect.left);
    //    m_pkAppWindow->SetHeight(kRect.bottom - kRect.top);
    //}

    return true;
}
//---------------------------------------------------------------------------
// OnWindowDestroy() handles WM_DESTROY
//   pWnd = unused
//   bOption = unused
bool NiApplication::OnWindowDestroy(NiWindowRef pWnd, bool bOption)
{
    return false;
}
//---------------------------------------------------------------------------
// Check message loop, which in turn processes input and system calls.
// returns true to continue, false to exit
#ifdef PG_USE_WIN_MESSAGE_STAT
typedef struct stMessageStat
{
	MSG kMsg;
	float fTime;
	float fTime1;
	float fTime2;
	float fTime3;
} MessageStat;

typedef std::vector<MessageStat> MessageStatVec;
#endif

bool NiApplication::Process()
{
    PROFILE_FUNC();

    MSG kMsg;

	static float fMessageHandlingTimeBetweenFrame = 0.0f;
#ifdef PG_USE_WIN_MESSAGE_STAT	
	static MessageStatVec kStatVec;
	static const float fMaxMessageProcessingTimePerFrame = 0.2f;
#else
	static const float fMaxMessageProcessingTimePerFrame = 0.05f;
#endif

    if (fMessageHandlingTimeBetweenFrame < fMaxMessageProcessingTimePerFrame)
    {
		if (PeekMessage(&kMsg, 0, 0, 0, PM_REMOVE))
		{
#ifdef PG_USE_WIN_MESSAGE_STAT
			MessageStat kStat;
			float fTime = NiGetCurrentTimeInSec();
			float fTime2 = fTime;
#endif
			if (kMsg.message == WM_QUIT)
				return false;

			if (!TranslateAccelerator(NiApplication::ms_pkApplication->
				GetAppWindow()->GetWindowReference(), 
				NiApplication::ms_pkApplication->GetAcceleratorReference(), 
				&kMsg))
			{
#ifdef PG_USE_WIN_MESSAGE_STAT
				kStat.fTime1 = NiGetCurrentTimeInSec() - fTime2;
				fTime2 = NiGetCurrentTimeInSec();

				TranslateMessage(&kMsg);
				kStat.fTime2 = NiGetCurrentTimeInSec() - fTime2;
				fTime2 = NiGetCurrentTimeInSec();
				DispatchMessage(&kMsg);
				kStat.fTime3 = NiGetCurrentTimeInSec() - fTime2;
#else
				TranslateMessage(&kMsg);
				DispatchMessage(&kMsg);
#endif
			}
			
#ifdef PG_USE_WIN_MESSAGE_STAT
			kStat.kMsg = kMsg;
			kStat.fTime = (NiGetCurrentTimeInSec() - fTime);
			kStatVec.push_back(kStat);
			fMessageHandlingTimeBetweenFrame += (NiGetCurrentTimeInSec() - fTime);
#endif
		}
		else
		{
			NiApplication::ms_pkApplication->OnIdle();
#ifdef PG_USE_WIN_MESSAGE_STAT
			if (kStatVec.size() > 0 && m_bCheckMessageStat)
			{
				_PgOutputDebugStringLevel(9999, "=========== message handling between frame lagged! %d message, %f time\n", kStatVec.size(), fMessageHandlingTimeBetweenFrame);
				for (int i = 0; i < kStatVec.size(); ++i)
				{
					//if (kStatVec[i].fTime > 0.001f)
					_PgOutputDebugStringLevel(9999, "\t %d message(%d,%#X,%#X, %d) handling time %f(%f,%f,%f)\n", kStatVec[i].kMsg.message, kStatVec[i].kMsg.hwnd, kStatVec[i].kMsg.wParam, kStatVec[i].kMsg.lParam, kStatVec[i].kMsg.time, kStatVec[i].fTime, kStatVec[i].fTime1, kStatVec[i].fTime2, kStatVec[i].fTime3);
				}
			}
			fMessageHandlingTimeBetweenFrame = 0.0f;
			kStatVec.clear();
#endif
		}
	}
	else
	{
		NiApplication::ms_pkApplication->OnIdle();
#ifdef PG_USE_WIN_MESSAGE_STAT
		if (kStatVec.size() > 0 && fMessageHandlingTimeBetweenFrame > 0.1f)
		{
			_PgOutputDebugStringLevel(9999, "=========== message handling between frame lagged! %d message, %f time\n", kStatVec.size(), fMessageHandlingTimeBetweenFrame);
			for (int i = 0; i < kStatVec.size(); ++i)
			{
				if (kStatVec[i].fTime > 0.001f)
				{
					_PgOutputDebugStringLevel(9999, "\t %d message(%d,%#X,%#X, %d) handling time %f(%f,%f,%f)\n", kStatVec[i].kMsg.message, kStatVec[i].kMsg.hwnd, kStatVec[i].kMsg.wParam, kStatVec[i].kMsg.lParam, kStatVec[i].kMsg.time, kStatVec[i].fTime, kStatVec[i].fTime1, kStatVec[i].fTime2, kStatVec[i].fTime3);
				}
			}
		}
		fMessageHandlingTimeBetweenFrame = 0.0f;
		kStatVec.clear();
#endif
	}

    return true;
}

//---------------------------------------------------------------------------
// default message processing
bool NiApplication::OnDefault(NiEventRef pEventRecord)
{
	switch(pEventRecord->uiMsg)
	{
	case WM_NCACTIVATE:
		{
			//NILOG(PGLOG_LOG, "OnDefault::WM_NCACTIVATE(%d,%d)\n", pEventRecord->wParam, pEventRecord->lParam);
			OnActivate(pEventRecord->wParam != 0);
		}
	}
    return false;
}
void NiApplication::OnActivate(bool bActivate)
{
	return;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// idle processing
//---------------------------------------------------------------------------
void NiApplication::OnIdle(void)
{
    // MeasureTime returns false if the frame rate is over the pre-set limit
    if (!MeasureTime())
        return;

    ResetFrameTimings();
    
    BeginUpdate();
    UpdateFrame();
    EndUpdate();
    
    BeginFrame();
    RenderFrame();
    EndFrame();

    DisplayFrame();

    // UpdateVisualTrackers();
	UpdateMetrics();

    m_iClicks++;
}
//--------------------------------------------------------------------------
void NiApplication::UpdateVisualTrackers()
{
#if 0 //NIMETRICS
    for (unsigned int ui = 0; ui < m_kTrackers.GetSize(); ui++)
    {
        NiVisualTracker* pkTracker = m_kTrackers.GetAt(ui);
        PG_ASSERT_LOG(pkTracker != 0);
        pkTracker->Update();
    }
#endif
}
//--------------------------------------------------------------------------
void NiApplication::UpdateMetrics()
{
#if NIMETRICS
    NIMETRICS_MAIN_RECORDMETRICS();

	NIMETRICS_APPLICATION_ADDVALUE(FRAMERATE, m_fFrameRate);
	NIMETRICS_APPLICATION_ADDVALUE(CULL_TIME, m_fCullTime);
	NIMETRICS_APPLICATION_ADDVALUE(UPDATE_TIME, m_fUpdateTime);
	NIMETRICS_APPLICATION_ADDVALUE(RENDER_TIME, m_fRenderTime);

    NIMETRICS_UPDATE();
#endif
}
//--------------------------------------------------------------------------
void NiApplication::ResetFrameTimings()
{
    m_fUpdateTime = 0.0f;
    m_fCullTime = 0.0f;
    m_fRenderTime = 0.0f;
}
//---------------------------------------------------------------------------
void NiApplication::BeginUpdate()
{
    PG_ASSERT_LOG(m_fBeginUpdate == 0.0f);
    m_fBeginUpdate = NiGetCurrentTimeInSec();
}
//---------------------------------------------------------------------------
void NiApplication::EndUpdate()
{
    float fEndUpdate = NiGetCurrentTimeInSec();
    m_fUpdateTime += fEndUpdate - m_fBeginUpdate;
    m_fBeginUpdate = 0.0f;
}
//---------------------------------------------------------------------------
void NiApplication::BeginCull()
{
    PG_ASSERT_LOG(m_fBeginCull == 0.0f);
    m_fBeginCull = NiGetCurrentTimeInSec();
}
//---------------------------------------------------------------------------
void NiApplication::EndCull()
{
    float fEndCull = NiGetCurrentTimeInSec();
    m_fCullTime += fEndCull - m_fBeginCull;
    m_fBeginCull = 0.0f;
}
//---------------------------------------------------------------------------
void NiApplication::BeginRender()
{
    PG_ASSERT_LOG(m_fBeginRender == 0.0f);
    m_fBeginRender = NiGetCurrentTimeInSec();
}
//---------------------------------------------------------------------------
void NiApplication::EndRender()
{
    float fEndRender = NiGetCurrentTimeInSec();
    m_fRenderTime += fEndRender - m_fBeginRender;
    m_fBeginRender = 0.0f;
}
//---------------------------------------------------------------------------
void NiApplication::UpdateFrame()
{
    // Update the input system
    UpdateInput();

    // Handle user input
    ProcessInput();

    if (m_bFrameRateEnabled && m_pkFrameRate)
    {
        m_pkFrameRate->TakeSample();
        m_pkFrameRate->Update();
    }

	float	fFrameTime = GetFrameTime();
	if(fFrameTime!=0)
	{
	    m_fFrameRate = 1.0f / fFrameTime;
	}
}
//---------------------------------------------------------------------------
void NiApplication::BeginFrame()
{
    m_spRenderer->BeginFrame();
}
//---------------------------------------------------------------------------
void NiApplication::CullFrame()
{
    BeginCull();
    NiCullScene(m_spCamera, m_spScene, m_kCuller, m_kVisible);
    EndCull();
}
//---------------------------------------------------------------------------
void NiApplication::RenderFrame()
{
    m_spRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ALL);
    CullFrame();

    BeginRender();
    NiDrawVisibleArray(m_spCamera, m_kVisible);

    RenderScreenItems();
    EndRender();

    RenderVisualTrackers();
    m_spRenderer->EndUsingRenderTargetGroup();
}
//---------------------------------------------------------------------------
void NiApplication::RenderScreenItems()
{
    m_spRenderer->SetScreenSpaceCameraData();
    const unsigned int uiSESize = m_kScreenElements.GetSize();
    unsigned int i = 0;
    for (i = 0; i < uiSESize; i++)
    {
        NiScreenElements* pkElements = m_kScreenElements.GetAt(i);
        if (pkElements)
            pkElements->Draw(m_spRenderer);
    }


    const unsigned int uiSTSize = m_kScreenTextures.GetSize();
    for (i = 0; i < uiSTSize; i++)
    {
        NiScreenTexture* pkTexture = m_kScreenTextures.GetAt(i);
        if (pkTexture)
            pkTexture->Draw(m_spRenderer);
    }
}
//---------------------------------------------------------------------------
void NiApplication::RenderVisualTrackers()
{
#if 0 //NIMETRICS
    for (unsigned int ui = 0; ui < m_kTrackers.GetSize(); ui++)
    {
        NiVisualTracker* pkTracker = m_kTrackers.GetAt(ui);
        PG_ASSERT_LOG(pkTracker != 0);
        pkTracker->Draw();
    }
#endif
}
//---------------------------------------------------------------------------
void NiApplication::EndFrame()
{
    m_spRenderer->EndFrame();
}
//---------------------------------------------------------------------------
void NiApplication::DisplayFrame()
{
    m_spRenderer->DisplayFrame();
}
//---------------------------------------------------------------------------

