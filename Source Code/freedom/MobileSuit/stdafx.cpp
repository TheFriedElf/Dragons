
#include "stdafx.h"

#include "Pg2DString.h"
#include "Variant/PgUserMapInfo.h"

// TODO: 필요한 추가 헤더는
// 이 파일이 아닌 STDAFX.H에서 참조합니다.
bool g_bUsePackData = false;
bool g_bUseProjectionShadow = false;
bool g_bUseProjectionLight = false;
bool g_bUseUIOffscreenRendering = true;
bool g_bUseExtraPackData = false;
bool g_bUseLoadGsa2 = false;
bool g_bUseSound = false;
bool g_bUseFog = true;
bool g_bUseAMPool = true;
bool g_bUseNewClone = true;
unsigned int g_iUseAddUnitThread = 0;
bool g_bShowCharacterPosition = false;
bool g_bUseColorShadow = true;
bool g_bShowChannelInfo = false;//Channel
bool g_bUseDebugInfo = false;
bool g_bUseAMReuse = true;
bool g_bUseThreadDelete = true;
bool g_bUseItemCache = true;
int g_iUseDebugConsole = 0;
bool g_bUseVariableActorUpdate = true;
int	g_iSyncPosSpeed = 150;
float g_fBGMVolume = 1.0f;
float g_fEffectVolume = 1.0f;
std::wstring g_strLocale = _T("");
bool	g_bUseDirectInput = false;
bool	g_bSendSmallAreaInfo = true;
bool	g_bUseActorControllerUpdateOptimize = false;
bool g_bShowTriangleCount = false;
int g_iTriangleCount = 0;
std::wstring g_strFinishLinkURL = _T("");
bool g_bUseCoupleUI = true;
bool	g_bDrawCircleShadow = true;
bool	g_bUseBatchRender = true;
bool	g_bVSync = true;
bool	g_bUseMTSGU = true;
int		g_iSideJobBoardMoveCost = 0;
bool	g_bUseLoginFailExit = false;
int		g_iF7DamageHideStage = 20;
extern float g_fWeaponSize = 1.0f;
#ifndef USE_INB
int g_iOutPutDebugLevel = 0;
#endif

#ifdef EXTERNAL_RELEASE
bool g_bUseImportActionData = false;
#else
bool g_bUseImportActionData = false;
#endif


std::wstring const FONT_TEXT = _T("Font_Text");
std::wstring const FONT_CHAT = _T("Font_Chat");
std::wstring const FONT_WARN = _T("Font_Warn");
std::wstring const FONT_NAME = _T("Font_Name");//3d 이름
std::wstring const FONT_GUILDNAME = _T("Font_GuildName");//3d 길드 이름
std::wstring const FONT_TITLENAME = _T("Font_TitleName");//3d 호칭 이름
std::wstring const FONT_PORTAL_TITLE = _T("Font_Portal_Title");//3d 이름
std::wstring const FONT_FTCTS_16 = _T("FTcts_16");

int g_bToggleUIDraw = 3;
NiFrustum g_kFrustum;
//PG2DSTRING_POOL g_k2dStringPool(50);
bool g_bWM_CLOSE = false;