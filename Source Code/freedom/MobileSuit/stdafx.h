
#ifndef FREEDOM_DRAGONICA_PCH_STDAFX_H
#define FREEDOM_DRAGONICA_PCH_STDAFX_H

// 아래 지정된 플랫폼에 우선하는 플랫폼을 대상으로 하는 경우 다음 정의를 수정하십시오.
// 다른 플랫폼에 사용되는 해당 값의 최신 정보는 MSDN을 참조하십시오.
#ifndef WINVER				// Windows XP 이상에서만 기능을 사용할 수 있습니다.
#define WINVER 0x0501		// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_WINNT		// Windows XP 이상에서만 기능을 사용할 수 있습니다.                   
#define _WIN32_WINNT 0x0501	// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 이상에서만 기능을 사용할 수 있습니다.
#define _WIN32_WINDOWS 0x0410 // Windows Me 이상에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_IE			// IE 6.0 이상에서만 기능을 사용할 수 있습니다.
#define _WIN32_IE 0x0600	// 다른 버전의 IE에 맞도록 적합한 값으로 변경해 주십시오.
#endif


// 무시하는 경고 (이 외의 경고는 모두 오류로 처리한다!)
#pragma warning(disable : 4819)		// 코드 페이지 워닝
#pragma warning(disable : 4267)		// usngined int --> size_t 워닝
#pragma warning(disable : 4996)
#pragma warning(error : 4715)


// Windows 헤더 파일:
#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX				// PhysX의 min/max을 사용합니다.
#include <windows.h>

// C-Runtime library
#include <stdlib.h>
#include <crtdbg.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <xutility>

// STL library
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <queue>

// System library
#include <winsock2.h>
#include <MMSystem.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

//	FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// Visual Memory Leak Detector
//#include <vld.h>


#ifdef _DEBUG
    #define SAFE_DELETE(p)      { ::OutputDebugStringA("Del log " __FUNCTION__ "\n");       if(p) { delete (p); (p)=NULL; } }
    #define SAFE_DELETE_NI(p)   { ::OutputDebugStringA("Del log ni " __FUNCTION__ "\n");    if(p) { NiDelete (p); (p)=NULL; } }
#else
    #define SAFE_DELETE(p)      { if(p) { delete (p); (p)=NULL; } }
    #define SAFE_DELETE_NI(p)   { if(p) { NiDelete (p); (p)=NULL; } }
#endif


#include "BM/STLSupport.h"
#include "randomlib/Common.h"
#include "Loki/singleton.h"
#include "cximage/ximage.h"
#include "zlib/zlib.h"
#include "lua_tinker/common.h"
#include "LuaDev/LuaDebuggerLIB.h"
#include "tinyxml/tinyxml.h"

//// BM
//
#include "BM/BM.h"
#include "BM/Guid.h"
#include "BM/vstring.h"
#include "BM/Point.h"
#include "BM/TimeCheck.h"
#include "BM/Stream.h"
#include "BM/LocalMgr.h"
//// CEL
//
#include "CEL/CEL.h"
#include "CEL/CoreCenter.h"
#include "CEL/ConsoleCommander.h"

#include "DataPack/common.h"

//// XUI
//
#include "XUI/XUI_Include.h"
#include "XUI/XUI_Manager.h"
#include "XUI/XUI_Resource_Manager.h"
#include "XUI/XUI_Font.h"

#include "CSIME/Common.h"

//! Definitions
#include "Definitions.h"

#include "DefColor.h"

//// Imports gamebryo libs
//
//#define USE_GAMEBRYO_DLL
#ifdef USE_GAMEBRYO_DLL
#define NISYSTEM_IMPORT
#define NIMAIN_IMPORT
#define NIINPUT_IMPORT
#define NIANIMATION_IMPORT
#define NICOLLISION_IMPORT
#define NIBINARYSHADERLIB_IMPORT
#define NSBSHADERLIB_IMPORT
#define NSFPARSERLIB_IMPORT
#define NID3DXEFFECTSHADERLIB_IMPORT
#define NIPARTICLE_IMPORT
#define NIAUDIO_IMPORT
#define NIENTITY_IMPORT
#define NIPHYSX_IMPORT
#define NIOPTIMIZATION_IMPORT
#define NIDX9RENDERER_IMPORT

//#if defined)EXTERNAL_RELEASE) || defined(USE_INB)
#pragma comment(lib, "NiSystem22VC80S.lib")
#pragma comment(lib, "NiMain22VC80S.lib")
#pragma comment(lib, "NiInput22VC80S.lib")
#pragma comment(lib, "NiAnimation22VC80S.lib")
#pragma comment(lib, "NiCollision22VC80S.lib")
#pragma comment(lib, "NiBinaryShaderLibDX922VC80S.lib")
#pragma comment(lib, "NSBShaderLibDX922VC80S.lib")
#pragma comment(lib, "NSFParserLibDX922VC80S.lib")
#pragma comment(lib, "NiD3DXEffectShaderLibDX922VC80S.lib")
#pragma comment(lib, "NiParticle22VC80S.lib")
#pragma comment(lib, "NiMilesAudio22VC80S.lib")
#pragma comment(lib, "NiEntity22VC80S.lib")
#pragma comment(lib, "NiPhysX22VC80S.lib")
#pragma comment(lib, "NiOptimization22VC80S.lib")
//#else
//#pragma comment(lib, "NiSystem22VC80R.lib")
//#pragma comment(lib, "NiMain22VC80R.lib")
//#pragma comment(lib, "NiInput22VC80R.lib")
//#pragma comment(lib, "NiAnimation22VC80R.lib")
//#pragma comment(lib, "NiCollision22VC80R.lib")
//#pragma comment(lib, "NiBinaryShaderLibDX922VC80R.lib")
//#pragma comment(lib, "NSBShaderLibDX922VC80R.lib")
//#pragma comment(lib, "NSFParserLibDX922VC80R.lib")
//#pragma comment(lib, "NiD3DXEffectShaderLibDX922VC80R.lib")
//#pragma comment(lib, "NiParticle22VC80R.lib")
//#pragma comment(lib, "NiMilesAudio22VC80R.lib")
//#pragma comment(lib, "NiEntity22VC80R.lib")
//#pragma comment(lib, "NiPhysX22VC80R.lib")
//#pragma comment(lib, "NiOptimization22VC80R.lib")
//#endif


#else

#pragma comment(lib, "NiSystem.lib")
#pragma comment(lib, "NiMain.lib")
#pragma comment(lib, "NiInput.lib")
#pragma comment(lib, "NiAnimation.lib")
#pragma comment(lib, "NiCollision.lib")
#pragma comment(lib, "NiBinaryShaderLibDX9.lib")
#pragma comment(lib, "NSBShaderLibDX9.lib")
#pragma comment(lib, "NSFParserLibDX9.lib")
#pragma comment(lib, "NiD3DXEffectShaderLibDX9.lib")
#pragma comment(lib, "NiParticle.lib")
#pragma comment(lib, "NiMilesAudio.lib")
#pragma comment(lib, "NiEntity.lib")
#pragma comment(lib, "NiPhysX.lib")
#pragma comment(lib, "NiOptimization.lib")
#pragma comment(lib, "NiPortal.lib")
#pragma comment(lib, "FxStudio_Reference_Emergent_Win32.lib")
#pragma comment(lib, "FxStudioRT_Static.lib")
#pragma comment(lib, "NiFont.lib")
#endif

#include <NiLog.h>
#include <NiDI8InputSystem.h>
#include <NiAnimation.h>
#include <NiCollision.h>
#include <NiDirectXVersion.h>
#include <NiDX9Renderer.h>
#include <NiDX9SystemDesc.h>
#include <NiFont.h>
#include <Ni2DString.h>
#include <NiParticle.h>
#include <NiAudio.h>
#include <NiMilesAudio.h>
#include <NiD3DShaderFactory.h>
#include <NiLight.h>
#include <NiBillboardNode.h>
#include <NiStringsExtraData.h>
#include <NiPick.h>
#include <NiLoopedThread.h>
#include <NiEntity.h>
#include <NiFactories.h>
#include <NiPortal.H>

#ifndef USE_GAMEBRYO_DLL
#define _LIB
#endif
    #include <NSBShaderLib.h>
    #include <NSFParserLib.h>
    #include <NiD3DXEffectShaderLib.h>
#ifndef USE_GAMEBRYO_DLL
#undef _LIB
#endif

#pragma comment(lib, "mss32.lib")
#pragma comment(lib, "NxCooking.lib")
#pragma comment(lib, "NxCharacter.lib")
#pragma comment(lib, "PhysXLoader.lib")
#pragma comment(lib, "D3DX9.lib")

#ifdef _MT_
	#pragma comment(lib, "cximage_MT.lib")
	#pragma comment(lib, "Jpeg_MT.lib")
	#pragma	comment(lib, "SimpEnc_MT.lib")
	#pragma comment(lib, "BugTrap_MT.lib")
#endif

#ifdef _MTd_
	#pragma comment(lib, "cximage_MTd.lib")
	#pragma comment(lib, "Jpeg_MTd.lib")
	#pragma	comment(lib, "SimpEnc_MTd.lib")
	#pragma comment(lib, "BugTrap_MTd.lib")
#endif

#ifdef _MTo_
	#pragma comment(lib, "cximage_MTo.lib")
	#pragma comment(lib, "Jpeg_MTo.lib")
	#pragma	comment(lib, "SimpEnc_MTo.lib")
	#pragma comment(lib, "BugTrap_MTo.lib")
#endif

#ifdef _MD_
	#pragma comment(lib, "cximage_MD.lib")
	#pragma comment(lib, "Jpeg_MD.lib")
	#pragma	comment(lib, "SimpEnc_MD.lib")
	#pragma comment(lib, "BugTrap_MD.lib")
#endif

#ifdef _MDd_
	#pragma comment(lib, "cximage_MDd.lib")
	#pragma comment(lib, "Jpeg_MDd.lib")
	#pragma	comment(lib, "SimpEnc_MDd.lib")
	#pragma comment(lib, "BugTrap_MDd.lib")
#endif

#ifdef _MDo_
	#pragma comment(lib, "cximage_MDo.lib")
	#pragma comment(lib, "Jpeg_MDo.lib")
	#pragma	comment(lib, "SimpEnc_MDo.lib")
	#pragma comment(lib, "BugTrap_MDo.lib")
	#include <NiBuildConfiguration.h>
#endif

#include <ControllerManager.h> 
#include <NxCapsuleController.h>
#include <NxBoxController.h>
#include <NxCooking.h>
#include <NiPhysX.h>

#include "Utility/ComputerInfo.h"
#include "Lohengrin/Lohengrin.h"
#include "Collins/Collins.h"
#include "Variant/Variant.h"

#ifdef _DEBUG
		#define PG_USE_SOUND 
#else
		#define PG_USE_SOUND
#endif

#include "PgError.h"
#include "PgTextTable.h"

#include "DataPack/PgDataPackManager.h"
#include "variant/PgTotalObjectMgr.H"
#include "variant/item.h"

extern bool g_bUsePackData;
extern bool g_bUseProjectionShadow;
extern bool g_bUseProjectionLight;
extern bool g_bUseUIOffscreenRendering;
extern bool g_bUseExtraPackData;
extern bool g_bUseLoadGsa2;
extern bool g_bUseSound;
extern bool g_bUseFog;
extern bool g_bUseAMPool;
extern bool g_bUseNewClone;
extern unsigned int g_iUseAddUnitThread;
extern bool g_bShowCharacterPosition;
extern bool g_bUseColorShadow;
extern bool g_bShowChannelInfo;
extern bool g_bUseDebugInfo;
extern bool g_bUseAMReuse;
extern bool g_bUseThreadDelete;
extern bool g_bUseItemCache;
extern int g_iUseDebugConsole;
extern bool g_bUseVariableActorUpdate;
extern int	g_iSyncPosSpeed;
extern	bool	g_bUseDirectInput;
extern	bool	g_bSendSmallAreaInfo;
extern bool g_bUseActorControllerUpdateOptimize;
extern float g_fBGMVolume;
extern float g_fEffectVolume;
extern std::wstring g_strLocale;
extern bool g_bShowTriangleCount;
extern int g_iTriangleCount;
extern std::wstring g_strFinishLinkURL;
extern bool g_bUseCoupleUI;
extern	bool	g_bDrawCircleShadow ;
extern	bool	g_bUseBatchRender;
extern	bool	g_bVSync;
extern	bool	g_bUseMTSGU;
extern int	g_iSideJobBoardMoveCost;
extern bool g_bUseLoginFailExit;
extern int g_iF7DamageHideStage;
extern float g_fWeaponSize;
extern bool g_bUseImportActionData;
extern bool g_bWM_CLOSE;
#ifndef USE_INB
extern int g_iOutPutDebugLevel;
#endif

extern std::wstring const FONT_TEXT;
extern std::wstring const FONT_CHAT;
extern std::wstring const FONT_WARN;
extern std::wstring const FONT_NAME;
extern std::wstring const FONT_GUILDNAME;
extern std::wstring const FONT_TITLENAME;
extern std::wstring const FONT_PORTAL_TITLE;
extern std::wstring const FONT_FTCTS_16;

extern int g_bToggleUIDraw;

extern NiFrustum g_kFrustum;

#include "FreedomDef.h"
//
#include "PgNodeMng.h"
#include "Themida/ThemidaSDK.h"

#define DUMMY_LOG 
#include "NMClass/NMPCH.h"

// NewWare headers
#include "NewWare/NewWare.h"

extern char const* GetClientName();
extern wchar_t const* GetClientNameW();

#endif // FREEDOM_DRAGONICA_PCH_STDAFX_H