#pragma once


#include <cassert>
#include <string>
#include "BM/STLSupport.h"

//#define USE_GAMEGUARD
//#define USE_HACKSHIELD

#ifdef EXTERNAL_RELEASE
	#ifdef USE_GAMEGUARD
		#include "./NProtect/NPGameLib.h"
	#endif

	#ifdef USE_HACKSHIELD
		#ifdef HACKSHIELD_UNAUTO	//기본 설정 오토툴 비허용
			#include "./AhnLabHackShield_UNAUTO/HShield.h"
			#include "./AhnLabHackShield_UNAUTO/HSUpChk.h"
			#include "./AhnLabHackShield_UNAUTO/HsUserUtil.h"
		#endif

		#ifdef HACKSHIELD_AUTO	//중국,일본 용 오토툴 허용
			#include "./AhnLabHackShield_AUTO/HShield.h"
			#include "./AhnLabHackShield_AUTO/HSUpChk.h"
			#include "./AhnLabHackShield_AUTO/HsUserUtil.h"
		#endif
	#endif
#endif
class PgAntiBeamShield
{
public:
	PgAntiBeamShield();
	virtual ~PgAntiBeamShield(void);

	void UnInit();

public:	
	HRESULT Init(const HWND hWnd);
	void OnLogin(std::wstring const &wstrID);
	bool CheckProcess();
		
	void CheckState( BM::Stream &rkPacket, BM::GUID const &guidSwitchKey = BM::GUID::NullData() );

	char const* GetInfo()const;

	CLASS_DECLARATION_S(std::wstring, ServiceLib);
	CLASS_DECLARATION( BM::GUID, m_guidKey, GuidKey );
protected:

#ifdef EXTERNAL_RELEASE
	#ifdef USE_GAMEGUARD
		CNPGameLib *m_pkNProtect;
	#endif
	#ifdef USE_HACKSHIELD
	public:
		static DWORD m_dwThreadID;
	protected:
	#endif
#endif
	DWORD m_dwTime;
};

#ifdef EXTERNAL_RELEASE
	#ifdef USE_GAMEGUARD
		extern BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg);
	#endif
	#ifdef USE_HACKSHIELD
		extern int __stdcall _AhnHS_CallBack(long lCode, long lParamSize, void* pParam);
	#endif
#endif

void CALLBACK _Antihack_UnhandledExceptionHandler(void);

#define g_kAppProtect SINGLETON_STATIC(PgAntiBeamShield)
