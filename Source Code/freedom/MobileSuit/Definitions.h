#ifndef FREEDOM_DRAGONICA_PCH_DEFINITIONS_H
#define FREEDOM_DRAGONICA_PCH_DEFINITIONS_H

///////////////////////////////// Global Definitions //////////////////////////////////////
#define PG_EPSILON 0.0001f

//#define PG_CHARACTER_CAPSULE_HEIGHT	40.0f
//#define PG_CHARACTER_CAPSULE_RADIUS	5.0f
//#define PG_CHARACTER_Z_ADJUST (PG_CHARACTER_CAPSULE_HEIGHT * 0.5f + PG_CHARACTER_CAPSULE_RADIUS)
#define MAX_THREAD_NUM 3
#define INVALID_Z_HEIGHT -100000

#define PG_EXCEPTION_FOLDER	TEXT("./Log/Exception")

#define WM_PG_STAGEDONE_NOTIFY		WM_USER + 100
#define WM_PG_REQUEST_INIT_IME		WM_USER + 101
#define WM_PG_REQUEST_INIT_SCRIPT	WM_USER + 102
#define WM_PG_REQUEST_INIT_SOUND	WM_USER + 103

#define PG_MAX_NB_ABV_SHAPES		10				// 한 엑터가 가질 수 있는 ABV 최대 수

/////////////////////////////////////// Functions/Contents //////////////////////////////////
//! 프로그램이 처음 시작할 때 로딩을 여러 Thread에서 하도록 함
#define PG_USE_THREAD_LOADING

//! PgAMPool과 PgWorkerThread를 켜고 끔.
#define PG_USE_WORKER_THREAD

//! Exception Log를 남기도록 함.
#ifndef _DEBUG
#define PG_EXCEPTION_ENABLED
#endif

#ifdef EXTERNAL_RELEASE
#ifndef PG_EXCEPTION_ENABLED
#define PG_EXCEPTION_ENABLED
#endif
#endif

#define PG_USE_ACTOR_AUTOMOVE_EDGE

//! 서버와 전체 시간을 동기화
#define PG_SYNC_ENTIRE_TIME

////////////////////////////////////// Logs ////////////////////////////////////////////////
// LOG ID NIMESSAGE_MAX_INTERNAL_INDEX + 1(8) ~ NIMESSAGE_MAX_INTERNAL_INDEX + 25(32)
#define PGLOG_ERROR		NIMESSAGE_MAX_INTERNAL_INDEX + 1
#define PGLOG_WARNING	NIMESSAGE_MAX_INTERNAL_INDEX + 2
#define PGLOG_NETWORK	NIMESSAGE_MAX_INTERNAL_INDEX + 3
#define PGLOG_LOGIC		NIMESSAGE_MAX_INTERNAL_INDEX + 4
#define PGLOG_LOG		NIMESSAGE_MAX_INTERNAL_INDEX + 5
#define PGLOG_THREAD	NIMESSAGE_MAX_INTERNAL_INDEX + 6
#define PGLOG_SCRIPT	NIMESSAGE_MAX_INTERNAL_INDEX + 7
#define PGLOG_STAT		NIMESSAGE_MAX_INTERNAL_INDEX + 8
#define PGLOG_ASSERT	NIMESSAGE_MAX_INTERNAL_INDEX + 9
#define PGLOG_MINOR		NIMESSAGE_MAX_INTERNAL_INDEX + 10
#define PGLOG_MEMIO		NIMESSAGE_MAX_INTERNAL_INDEX + 11
#define PGLOG_CONSOLE	NIMESSAGE_MAX_INTERNAL_INDEX + 12
#define PGLOG_MESSAGE	NIMESSAGE_MAX_INTERNAL_INDEX + 13
#define PGLOG_WINMESSAGE	NIMESSAGE_MAX_INTERNAL_INDEX + 14

#define PG_LOG_ENABLED
#ifdef NISHIPPING
//#undef PG_LOG_ENABLED
#endif

#ifdef PG_LOG_ENABLED

#define PG_FLUSH_LOG	NiLogger::FlushAllLogs();
#define PG_CLOSE_LOG	NiLogger::FlushAllLogs(); \
						NiLogger::CloseAllLogs();
#undef NI_LOGGER_DISABLE
#if defined(NDEBUG)
#undef NI_RELEASE_MODE_LOGGING
#define NI_RELEASE_MODE_LOGGING 1
#else
#define NI_RELEASE_MODE_LOGGING 0
#endif

#else

#define PG_FLUSH_LOG	__noop;
#define PG_CLOSE_LOG	__noop;
#define NI_LOGGER_DISABLE
#define NI_RELEASE_MODE_LOGGING 0
#endif

////////////////////////////////////// Macros ///////////////////////////////////////////////
//! ASSERT MACRO
#ifdef EXTERNAL_RELEASE
#define PG_ASSERT_LOG(x)	/##/
#define PG_WARNING_LOG(x)	/##/
#else
#define PG_ASSERT_LOG(x)	if ((x) == false) {NILOG(PGLOG_ASSERT, "[ASSERT](%s-%d) " #x "\n", __FUNCTION__, __LINE__); assert(x);}
#define PG_WARNING_LOG(x)	if ((x) == false) {NILOG(PGLOG_WARNING, "[WARNING](%s-%d) " #x "\n", __FUNCTION__, __LINE__);}
#endif

//! Exception Macro
#ifdef PG_EXCEPTION_ENABLED
//! 이런거 좋은건 아닌거 같다. 하지만 소스가 깔끔해짐;; 
#define PG_TRY_BLOCK	try {
#define PG_CATCH_BLOCK	\
	} catch (CSystemException* e) { \
		g_pkApp->RestoreStickyKeySetting();	\
		SETDIR_ERROR_REPORT(PG_EXCEPTION_FOLDER); \
		e->WriteAdditionalInfo(); \
		SAFE_DELETE(e); \
		ExceptionUserAlert(); \
		g_pkApp->stopAllThread(); \
		g_pkApp->terminateAllThread(); \
		PG_FLUSH_LOG; \
		g_pkApp->ShowPointer();		\
		g_pkApp->QuitApplication(); \
	}

#define PG_CATCH_BLOCK_NO_EXIT	\
	} catch (CSystemException* e) { \
		SETDIR_ERROR_REPORT(PG_EXCEPTION_FOLDER); \
		e->WriteAdditionalInfo(); \
		SAFE_DELETE(e); \
	}

#ifndef USE_INB
#define PG_RAISE_EXCEPTION	\
	::RaiseException(1, 0, 0, NULL);
#else
#define PG_RAISE_EXCEPTION
#endif

#else

#define PG_TRY_BLOCK
#define PG_CATCH_BLOCK
#define PG_CATCH_BLOCK_NO_EXIT
#define PG_RAISE_EXCEPTION

#endif

// Pure Virtual Function Macro
#ifndef USE_INB
#define PG_PURE_VIRTUAL_FUNCTION	=0;
#else
#define PG_PURE_VIRTUAL_FUNCTION	{ PG_RAISE_EXCEPTION };
#endif

//! Mem Macro
// Check Performance (logs, routines, etc)
//#define CHECK_PERFORMANCE

#ifdef CHECK_PERFORMANCE
#define PG_MEM_LOG	NILOG(PGLOG_MEMIO, "[MEMORY] System Usage %.3fMB Video Usage %.3fMB\n", PgComputerInfo::GetSystemMemUsage(), PgComputerInfo::GetVideoMemUsage());
#else
#define PG_MEM_LOG	/##/;
#endif

//! etc
inline bool IsZero(float f)	{ if (f < PG_EPSILON && f > -PG_EPSILON) return true; else return false; }

#ifdef EXTERNAL_RELEASE
//#define NOGODCMD
#undef CHECK_PERFORMANCE
#endif

#ifdef USE_INB
#define NOGODCMD
#endif

#endif // FREEDOM_DRAGONICA_PCH_DEFINITIONS_H