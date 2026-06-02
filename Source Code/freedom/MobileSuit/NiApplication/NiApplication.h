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

#ifndef NIAPPLICATION_H
#define NIAPPLICATION_H

// Gamebryo main engine
#include "NiMain.h"

// Many NiApplication subclasses use NiOutputDebugString and other
// system-related calls
#include <NiSystem.h>

// command line parsing
#include "NiCommand.h"

// application-level classes
#include "NiAppWindow.h"

// input devices
#include "NiInputSystem.h"
//#include "NiTurret.h"

// frame rate display in the hardware-accelerated window
#include "NiFrameRate.h"

// graph visualization for common statistics
#include <NiVisualTracker.h>
#include <NiCommonGraphCallbackObjects.h>

#include <NiUniversalTypes.h>
#include <NiVersion.h>

#if defined(WIN32)
#include <NiDirectXVersion.h>
#endif //#if defined(WIN32)

class NiApplication : public NiMemObject
{
//----- base application layer ----------------------------------------------
public:
    // the unique application instance
    static NiApplication* ms_pkApplication;

    // Create must be implemented in the NiApplication-derived class.
    static NiApplication* Create();

    NiApplication
    (
        char const* pcWindowCaption,       // caption of window
        unsigned int uiWidth,              // client window width
        unsigned int uiHeight,             // client window height
        unsigned int uiMenuID = 0,         // menu resource ID
        unsigned int uiNumStatusPanes = 0, // number of panes in status window
        unsigned int uiBitDepth = 0        // kColor depth of screen
    );
    virtual ~NiApplication();

#ifdef _PS3
    enum DisplayMode
    {
#define NIENUMMAP(enumerant, psgldevicetype) \
    DisplayMode_##enumerant
#include "NiPSGLDisplayModes.inl"
#undef NIENUMMAP
        NUM_DISPLAYMODES
    };
 
    enum MultisamplingMode
    {
#define NIENUMMAP(enumerant, psglmultisamplingmode) \
    MultisamplingMode_##enumerant
#include "NiPSGLMultisamplingModes.inl"
#undef NIENUMMAP
        NUM_MULTISAMPLINGMODES
    };
#endif

    // command line parsing
    static void SetCommandLine(char* pcCommandLine);
    static void SetCommandLine(int iArgc, char** ppcArgv);
    static NiCommand* GetCommand();
    static void CommandSelectRenderer();

    // Media Path
    static void SetMediaPath(char const* pcPath);
    static char const* GetMediaPath();  

    // Each call to this function will return the _same_ storage -
    // each call overwrites the previously returned string, so applications
    // must take care to copy the return immediately
    static char const* ConvertMediaFilename(char const* pcFilename);

    // Window methods
    NiWindowRef CreateMainWindow(int iWinMode, 
        NiWindowRef hWnd = 0) const;

    // selection of renderer and attributes
    void SetFullscreen(bool bOn);
    bool GetFullscreen() const;
	void SetBorderless(bool bOn);
	bool GetBorderless() const;
	bool GetMinimized() const;
    void SetStencil(bool bOn);
    bool GetStencil() const;
    void SetRendererDialog(bool bOn);
    bool GetRendererDialog() const;

	static void setLocalDirectory();

// 	static void			 SetIsDefaultResolution( bool const bDefaultResolution ) { m_bDefaultResolution = bDefaultResolution; }
// 	static const bool	 GetIsDefaultResolution()								 { return m_bDefaultResolution; }
// 	static void			 SetResolutionSize( POINT2 const& sptResolutionSize )	 { m_sptResolutionSize = sptResolutionSize; }
// 	static const POINT2& GetResolutionSize() 									 { return m_sptResolutionSize; }

    // Frame rate
    virtual void EnableFrameRate(bool bEnable);
    float GetMaxFrameRate();
    void SetMaxFrameRate(float fMax);
	bool GetFrameRateEnabled() { return m_bFrameRateEnabled; }
	NiFrameRate* GetFrameRate() { return m_pkFrameRate; }

    // VisualTrackers
    void SetShowAllTrackers(bool bShow);

    // This is called by WinMain after ShowWindow and UpdateWindow, but before
    // the idle loop.  The window handle is valid at this point, so any
    // NiApplication-derived class may safely use ms_hWnd in this routine.
    // The routine should be used for allocating resources and initializing
    // any data values.
    virtual bool Initialize();

    // Terminate is called after the idle loop exits. The application
    // should free up its resources in this routine.
    virtual void Terminate();

    virtual void CreateMenuBar();

    NiWindowRef GetWindowReference() const;
    NiWindowRef GetRenderWindowReference() const;
	void AdjustWindowRect(LPRECT lpWindowRect, LPRECT lpClientRect, DWORD &dwStyle, bool& bDesktopIsSmall, bool bFitToScreen = false, bool bOnlyCalcRect = false);
    static void SetInstanceReference(NiInstanceRef pInstance);
    static NiInstanceRef GetInstanceReference();
    static void SetAcceleratorReference(NiAcceleratorRef pAccel);
    static NiAcceleratorRef GetAcceleratorReference();

    NiAppWindow* GetAppWindow() const;
    unsigned int GetMenuID() const;    

    // Control cursor visibility
    void HidePointer();
    void ShowPointer();

    void MainLoop();
    void QuitApplication(void);

    // Events
    // Not all events apply to all platforms; the corresponding event handlers
    // will be simply not be called on those platforms.  Look in the event 
    // handler appropriate to your system to see how the respective events are 
    // processed before the virtual handlers are called.
    
//    NiInputSystem* GetInputSystem();

    // timing
    float GetCurrentTime();
    float GetLastTime();
    float GetAccumTime();
    float GetFrameTime();

    // updating the input sytem
    virtual void UpdateInput();

    // process the collected input. You would override this function to 
    // implement poll the devices states or check for action maps.
    virtual void ProcessInput();

    // check default keyboard/gamepad to show or hide the visual trackers.
    // override this to use your own keystroke or button settings.
    virtual void ProcessVisualTrackerInput();
    
    // idle processing
    virtual void OnIdle(void); // glut requires its display function to have
                               // an explicit "void"; parameter must match
    virtual void ResetFrameTimings();
    virtual void BeginFrame();

    // the begin/end wrappers are used to calculate the timings
    // used by the visual tracker.  If you do your own processing,
    // you should call the appropriate begin/end pair to add your timings
    // to those for the default update/cull/render process.  
    // Do not nest begin/end pairs, though, as that will assert.
    virtual void BeginUpdate();
    virtual void UpdateFrame();
    virtual void EndUpdate();

    virtual void BeginCull();
    virtual void CullFrame();
    virtual void EndCull();

    virtual void BeginRender();
    virtual void RenderFrame();
    virtual void EndRender();

    virtual void RenderScreenItems();
    virtual void RenderVisualTrackers();
    virtual void EndFrame();
    virtual void DisplayFrame();

    virtual void UpdateVisualTrackers();
	virtual void UpdateMetrics();

#if defined(WIN32)
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT uiMsg, WPARAM wParam,
        LPARAM lParam);
    
    // windows messages
    virtual bool OnWindowResize(int iWidth, int iHeight, 
        unsigned int uiSizeType, NiWindowRef pWnd);
    virtual bool OnWindowDestroy(NiWindowRef pWnd, bool bOption);
    virtual bool OnDefault(NiEventRef pEventRecord);
    virtual bool IsExclusiveMouse() const;
	virtual void OnActivate(bool bActivate);
#endif //#if defined(WIN32)

protected:
    virtual bool CreateScene();
    virtual bool CreateCamera();
    virtual bool CreateRenderer();
    virtual bool CreateInputSystem();    
    virtual NiInputSystem::CreateParams* GetInputSystemCreateParams();
    virtual void ConfigureInputDevices();
    virtual bool MeasureTime();
    virtual void DrawFrameRate();

    NiCamera* FindFirstCamera(NiNode* pkNode);
    void AdjustCameraAspectRatio(NiCamera* pkCamera);

    // basic objects
    NiNodePtr m_spScene;
    NiCameraPtr m_spCamera;
    NiRendererPtr m_spRenderer;
    NiInputSystemPtr m_spInputSystem;
    NiScreenElementsArray m_kScreenElements;
    NiScreenTextureArray m_kScreenTextures;

    // Application-related classes
    NiAppWindow* m_pkAppWindow;
    static NiInstanceRef ms_pInstance;
    static NiAcceleratorRef ms_pAccel;
    unsigned int m_uiMenuID;

    // renderer creation and properties
	static bool	  m_bDefaultResolution;
	static POINT2 m_sptResolutionSize;

    bool m_bFullscreen;	
	bool m_bBorderless;
    bool m_bStencil;
    bool m_bRendererDialog;
    bool m_bMultiThread;
    unsigned int m_uiBitDepth;
    NiFrameRate* m_pkFrameRate;
    bool m_bFrameRateEnabled;

    // frame rate counting and display
    int m_iMaxTimer;
    int m_iTimer;
    float m_fCurrentTime; // Time reported by system
    float m_fLastTime; // Last time reported by system
    float m_fAccumTime; // Time elapsed since application start
    float m_fFrameTime; // Time elapsed since previous frame
    int m_iClicks;
    float m_fFrameRate;
    float m_fLastFrameRateTime;
    int m_iLastFrameRateClicks;

    float m_fCullTime;
    float m_fRenderTime;
    float m_fUpdateTime;
    float m_fBeginUpdate;
    float m_fBeginCull;
    float m_fBeginRender;

    // Frame rate limitation
    float m_fMinFramePeriod;
    float m_fLastFrame;

    // Application control over visual tracker graph maximums
    float m_fVTPerformanceMax;
    float m_fVTMemoryMax;
    float m_fVTTimeMax;

    // command line
    static NiCommand* ms_pkCommand;

    // media path
    static char ms_acMediaPath[NI_MAX_PATH];
    static char ms_acTempMediaFilename[NI_MAX_PATH];

#if defined(_XENON)
    //  App-specific setup flags
    unsigned int m_uiSetupFlags;
#endif //#if defined(_XENON)

#if defined(WIN32) && !defined(_XENON)
    virtual bool Process();

	// Runtime-setting flags
    bool m_bRefRast;
    bool m_bSWVertex;
    bool m_bNVPerfHUD;
    bool m_bExclusiveMouse;
	bool m_bMinimized;
	bool m_bDestroying;
	bool m_bCheckMessageStat;
#else   //#if defined(WIN32) && !defined(_XENON)
    bool m_bQuitApp;
#endif  //#if defined(WIN32) && !defined(_XENON)

#ifdef _PS3
    PSGLdeviceParameters m_kPSGLDeviceParameters;
    bool m_bVSync;
#endif

    // culling (separate from display)
    NiVisibleArray m_kVisible;
    NiCullingProcess m_kCuller;

    // visual trackers
    NiTObjectArray<NiVisualTrackerPtr> m_kTrackers;
    bool m_bShowAllTrackers;
};

#include "NiApplication.inl"

#endif // NIAPPLICATION_H
