#include "stdafx.h"
#include <windows.h>
#include <initguid.h>
#include <ddraw.h>
#include "ComputerInfo.h"

#pragma comment(lib, "ddraw.lib")

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef amx
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

CPUINFO PgComputerInfo::mCPUInfo = { 0,};
GPUINFO PgComputerInfo::mGPUInfo = { 0,};
OSINFO PgComputerInfo::mOSInfo = { OS_UNKNOWN, 0, };
float PgComputerInfo::mInitSystemMem = 0.0f;
float PgComputerInfo::mInitVideoMem = 0.0f;
DEVICE_GRADE PgComputerInfo::mGPUGrade = DEVICE_GRADE_UNKNOWN;
DEVICE_GRADE PgComputerInfo::mCPUGrade = DEVICE_GRADE_UNKNOWN;
DEVICE_GRADE PgComputerInfo::mMEMGrade = DEVICE_GRADE_UNKNOWN;

void PgComputerInfo::InitMemStatus()
{
	mInitSystemMem = getCurrentSystemMemory();
	mInitVideoMem = getCurrentVideoMemory();
}

float PgComputerInfo::GetVideoMemUsage()
{
	if (mInitVideoMem != 0.0f)
		return getCurrentVideoMemory() - mInitVideoMem;
	return 0.f;
}

float PgComputerInfo::GetSystemMemUsage()
{
	if (mInitSystemMem != 0.0f)
		return getCurrentSystemMemory() - mInitSystemMem;
	return 0.f;
}

void PgComputerInfo::CheckComputerInfo(LPDIRECT3D9 pd3dDevice, bool bCheckGPU, bool bCheckOS, bool bCheckCPU)
{
	if (bCheckGPU && pd3dDevice != NULL)
		setGPUInfo(pd3dDevice);
	if (bCheckCPU)
		setCPUInfo();
	if (bCheckOS)
		setOSInfo();
}

float PgComputerInfo::getCurrentVideoMemory()
{
	return (getUsedVideoMemory() / (1024.0f * 1024.0f));
}

float PgComputerInfo::getCurrentSystemMemory()
{
	return (countMemUsage() / (1024.0f * 1024.0f));
}

int PgComputerInfo::GetIdealProcessorForThread(int threadCount)
{
	// it returns zero-based processor id 
	if (mCPUInfo.iNumProcess < 2)
	{
		return 0;
	}

	return (mCPUInfo.iNumProcess * 10 - 1 - threadCount) % mCPUInfo.iNumProcess; // 끝번 thread부터 역순으로 할당한다.
}

void PgComputerInfo::setCPUInfo()
{
	// CPU 숫자
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);
	mCPUInfo.iProcessorArchitecture = SI.wProcessorArchitecture;
	mCPUInfo.iNumProcess = SI.dwNumberOfProcessors;
	mCPUInfo.iProcessorType = SI.dwProcessorType;

	// CPU 이름
	char *pStr = mCPUInfo.strName;
	memset(mCPUInfo.strName, 0, sizeof(mCPUInfo.strName));
	try
	{
		__asm {
			mov eax, 0
				CPUID

				mov esi, pStr
				mov [esi], ebx
				mov [esi+4], edx
				mov [esi+8], ecx
		}
	}
	catch ( ... )
	{
		;
	}

	// 속도
	mCPUInfo.iMhz=0;
	
	LARGE_INTEGER ticksPerSec;
	QueryPerformanceFrequency(&ticksPerSec);
	mCPUInfo.iMhz = (int)((float)ticksPerSec.QuadPart / 1024.0f / 1024.0f);


	// 대충 분리한것
	if (mCPUInfo.iNumProcess > 1)
	{
		mCPUGrade = DEVICE_GRADE_HIGH;
	}
	else if (mCPUInfo.iMhz > 2000)
	{
		mCPUGrade = DEVICE_GRADE_MEDIUM;
	}
	else
	{
		mCPUGrade = DEVICE_GRADE_LOW;
	}

/*
	double dSecondsPerCycle;
	try
	{
		timeBeginPeriod( 1 );

		dSecondsPerCycle = 1.f;

		for( INT i=0; i<3; i++ )
		{
			LARGE_INTEGER	StartCycles,
				EndCycles;
			volatile DOUBLE	DoubleDummy = 0.0;
			volatile INT	IntDummy	= 0;
			__asm
			{
				rdtsc
					mov StartCycles.HighPart, edx
					mov StartCycles.LowPart, eax
			}

			DWORD	StartMsec	= BM::GetTime32(),
				EndMsec		= StartMsec;
			while( EndMsec-StartMsec < 200 )
			{
				DoubleDummy += sqrt(DoubleDummy) + 3.14;
				IntDummy	*= ((INT) (DoubleDummy) + 555) / 13;
				EndMsec = BM::GetTime32();
			}

			__asm
			{
				rdtsc
					mov EndCycles.HighPart, edx
					mov EndCycles.LowPart, eax
			}

			double	C1	= (double)(signed __int64)(((unsigned __int64)StartCycles.LowPart) + ((unsigned __int64)StartCycles.HighPart<<32)),
				C2	= (double)(signed __int64)(((unsigned __int64)EndCycles.LowPart) + ((unsigned __int64)EndCycles.HighPart<<32));

			dSecondsPerCycle = min( dSecondsPerCycle, 1.0 / (1000.f * ( C2 - C1 ) / (EndMsec - StartMsec)) );
		}
		timeEndPeriod( 1 );

		mCPUInfo.iMhz = (int)(0.000001 / dSecondsPerCycle);
	}
	catch( ... )
	{
		//debugf( NAME_Init, TEXT("Timestamp not supported (Possibly 486 or Cyrix processor)") );
		OutputDebugString(TEXT("PgComputerInfo::SetCPUInfo - Timestamp not supported (Possibly 486 or Cyrix processor)"));
		dSecondsPerCycle = 1;
	}
*/

	// 메모리
	mCPUInfo.iRam=0;
	MEMORYSTATUS M;
	GlobalMemoryStatus(&M);
	mCPUInfo.iRam = (int)(M.dwTotalPhys/1024.0f/1024.0f);

	if (mCPUInfo.iRam > 2000)
	{
		mMEMGrade = DEVICE_GRADE_HIGH;
	}
	else if (mCPUInfo.iRam > 500)
	{
		mMEMGrade = DEVICE_GRADE_MEDIUM;
	}
	else
		mMEMGrade = DEVICE_GRADE_LOW;
}

void PgComputerInfo::setGPUInfo(LPDIRECT3D9 pd3dDevice)
{
	mGPUInfo.iVRam=0;
	mGPUInfo.iVRamFree=0;
	strcpy_s(mGPUInfo.strName, 512, "");

	if (!pd3dDevice) return;

	LPDIRECTDRAW7 lpDD = NULL;
	DDSCAPS2      ddsCaps2; 
	DWORD         dwTotal; 
	DWORD         dwFree;
	DWORD		  OnboardVideoMemory=0x7fffffff;
	DWORD		  OnboardMax=0;
	HRESULT       hr; 

	// 이름
	if (pd3dDevice)
	{
		D3DADAPTER_IDENTIFIER9	DeviceIdentifier;
		if( FAILED( pd3dDevice->GetAdapterIdentifier(D3DADAPTER_DEFAULT,0,&DeviceIdentifier) ) )
			ZeroMemory( &DeviceIdentifier, sizeof(DeviceIdentifier) );
		strncpy_s(mGPUInfo.strName, 512, DeviceIdentifier.Description, 512);

		int Product = HIWORD(DeviceIdentifier.DriverVersion.HighPart);
		int Version = LOWORD(DeviceIdentifier.DriverVersion.HighPart);
		int SubVersion = HIWORD(DeviceIdentifier.DriverVersion.LowPart);
		int Build = LOWORD(DeviceIdentifier.DriverVersion.LowPart);

		sprintf_s(mGPUInfo.strVersion, 20, "%d.%d.%d.%d", Product, Version,SubVersion,Build);

		//DeviceIdentifier.Driver

		//{
		//	// Obtain driver version.  Try system dir first, then
		//	// try systemDir\drivers.
		//	WCHAR wszDriverPath[MAX_PATH];
		//	::GetSystemDirectory( wszDriverPath, MAX_PATH );
		//	StringCchCat( wszDriverPath, MAX_PATH, L"\\ati2dvag.dll" );
		//	//StringCchCat( wszDriverPath, MAX_PATH, wszModule );

		//	// If the file does not exist, use %SystemDir%\drivers.
		//	if( INVALID_FILE_ATTRIBUTES == GetFileAttributes( wszDriverPath ) )
		//	{
		//		::GetSystemDirectory( wszDriverPath, MAX_PATH );
		//		StringCchCat( wszDriverPath, MAX_PATH, L"\\drivers\\ati2dvag.dll" );
		//		//StringCchCat( wszDriverPath, MAX_PATH, wszModule );
		//	}

		//	DWORD dwVerHandle;
		//	DWORD dwBufferSize = GetFileVersionInfoSize( wszDriverPath, &dwVerHandle );
		//	LPVOID pBuffer = ::malloc( dwBufferSize );
		//	VS_FIXEDFILEINFO *pVer;
		//	UINT uVerSize;
		//	if( dwBufferSize && pBuffer )
		//	{
		//		if( GetFileVersionInfo( wszDriverPath, dwVerHandle, dwBufferSize, pBuffer ) &&
		//			VerQueryValue( pBuffer, L"\\", (LPVOID*)&pVer, &uVerSize ) )
		//		{
		//			// Retrieve version number
		//			//pSndDev->DriverVersionLowPart = pVer->dwFileVersionLS;
		//			//pSndDev->DriverVersionHighPart = pVer->dwFileVersionMS;
		//		}
		//	}

		//	::free( pBuffer );
		//}

		//strcpy( mGPUInfo.strDriver, DeviceIdentifier..DriverVersion);
	}

	hr = ::DirectDrawCreateEx(NULL, (VOID**)&lpDD, IID_IDirectDraw7, NULL );
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("PgComputerInfo::SetGPUInfo - DirectDrawCreateEx 실패"));
		return; 
	}

	//DDDEVICEIDENTIFIER2 di[4];
	////ZeroMemory(&di, sizeof(DDDEVICEIDENTIFIER2));
	//hr = lpDD->GetDeviceIdentifier(&di[1],0);
	//strcpy(pGPUInfo->strName, di[1].szDescription);

	// 비디오 메모리
	ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
	ddsCaps2.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
	lpDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
	if( dwTotal )
	{
		OnboardVideoMemory = min( OnboardVideoMemory, dwTotal );
		OnboardMax = max( OnboardMax, dwFree );
	}

	ZeroMemory( &ddsCaps2, sizeof(ddsCaps2) );
	ddsCaps2.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
	lpDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
	if( dwTotal )
	{
		OnboardVideoMemory = min( OnboardVideoMemory, dwTotal );
		OnboardMax = max( OnboardMax, dwFree );
	}

	ZeroMemory( &ddsCaps2, sizeof(ddsCaps2) );
	ddsCaps2.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
	lpDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
	if( dwTotal )
	{
		OnboardVideoMemory = min( OnboardVideoMemory, dwTotal );
		OnboardMax = max( OnboardMax, dwFree );
	}

	mGPUInfo.iVRam = (int)(OnboardVideoMemory/1024.0f/1024.0f);
	mGPUInfo.iVRamFree = (int)(OnboardMax/1024.0f/1024.0f);

	if(lpDD)
		lpDD->Release();


	//! Set Device Grade
	if (mGPUInfo.iVRam >= 100)
	{
		mGPUGrade = DEVICE_GRADE_HIGH;
	}
	else if (mGPUInfo.iVRam > 50)
	{
		mGPUGrade = DEVICE_GRADE_MEDIUM;
	}
	else
	{
		mGPUGrade = DEVICE_GRADE_LOW;
	}

	int number = 0;

	bool bStart = false;
	for (int i = 0; i < strlen(mGPUInfo.strName); i++)
	{
		if (isdigit(mGPUInfo.strName[i]))
		{
			if (bStart == false)
			{
				bStart = true;
				number = mGPUInfo.strName[i] - '0';
			}
			else
			{
				number = number * 10;
				number += (mGPUInfo.strName[i] - '0');
			}
		}
		else
		{
			if (bStart)
			{
				bStart = false;
				break;
			}
		}
	}

	NiString kGPUName(mGPUInfo.strName);
	if (kGPUName.ContainsNoCase("NVIDIA"))
	{
		if (number >= 7600)
			mGPUGrade = DEVICE_GRADE_HIGH;
		else if (number >= 6600)
			mGPUGrade = DEVICE_GRADE_MEDIUM;
		else
			mGPUGrade = DEVICE_GRADE_LOW;

	}
	else if (kGPUName.ContainsNoCase("RADEON") || kGPUName.ContainsNoCase("ATI"))
	{
		if (number >= 7000 || number < 800)
			mGPUGrade = DEVICE_GRADE_LOW;
		else if (number >= 1600)
			mGPUGrade = DEVICE_GRADE_MEDIUM;
		else
			mGPUGrade = DEVICE_GRADE_HIGH;
	}
	else if (kGPUName.ContainsNoCase("INTEL"))
	{
		if (number >= 1000)
			mGPUGrade = DEVICE_GRADE_HIGH;
		else
			mGPUGrade = DEVICE_GRADE_LOW;
	}
}

void PgComputerInfo::setOSInfo()
{
	mOSInfo.iType = OS_UNKNOWN;

	DWORD dwPlatformId, dwMajorVersion, dwMinorVersion, dwBuildNumber;

	//#if UNICODE
	//	if( Unicode && !UnicodeOS )
	//	{
	//		OSVERSIONINFOA Version;
	//		Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	//		GetVersionExA(&Version);
	//		dwPlatformId   = Version.dwPlatformId;
	//		dwMajorVersion = Version.dwMajorVersion;
	//		dwMinorVersion = Version.dwMinorVersion;
	//		dwBuildNumber  = Version.dwBuildNumber;
	//	}
	//	else
	//#endif

	{
		OSVERSIONINFO Version;
		Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&Version);
		dwPlatformId   = Version.dwPlatformId;
		dwMajorVersion = Version.dwMajorVersion;
		dwMinorVersion = Version.dwMinorVersion;
		dwBuildNumber  = Version.dwBuildNumber;

		mOSInfo.iVersion1 = Version.dwMajorVersion;
		mOSInfo.iVersion2 = Version.dwMinorVersion;
		mOSInfo.iVersion3 = Version.dwBuildNumber;
	}


	if( dwPlatformId==VER_PLATFORM_WIN32_NT )
	{
		if (dwMajorVersion == 6)
		{
			mOSInfo.iType = OS_WINVISTA;
		}
		else if( dwMajorVersion == 5 )
		{
			if (dwMinorVersion == 0)
			{
				mOSInfo.iType = OS_WIN2K;
			}
			else if (dwMinorVersion == 1)
			{
				mOSInfo.iType = OS_WINXP;
			}
			else if (dwMinorVersion == 2)
			{
				mOSInfo.iType = OS_WIN2003;
			}
		}
		else
		{
			mOSInfo.iType = OS_WINNT;
		}

		//UnicodeOS = 1;
	}
	else if( dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && (dwMajorVersion>4 || dwMinorVersion>=10) )
	{
		mOSInfo.iType = OS_WIN98;
	}
	else if( dwPlatformId==VER_PLATFORM_WIN32_WINDOWS )
	{
		mOSInfo.iType = OS_WIN95;
	}
	else
	{
		mOSInfo.iType = OS_UNKNOWN;
	}

	//switch(mOSInfo) {
	//case OS_UNKNOWN:
	//	NiMessageBox("OS_UNKNOWN","");
	//	break;
	//case OS_WIN95:
	//	NiMessageBox("OS_WIN95","");
	//	break;
	//case OS_WIN98:
	//	NiMessageBox("OS_WIN98","");
	//	break;
	//case OS_WIN2K:
	//	NiMessageBox("OS_WIN2K","");
	//	break;
	//case OS_WINXP:
	//	NiMessageBox("OS_WINXP","");
	//	break;
	//case OS_WINNT:
	//	NiMessageBox("OS_WINNT","");
	//	break;

	//default:
	//	break;
	//};
}

size_t PgComputerInfo::countMemUsage()
{ 
	MEMORY_BASIC_INFORMATION mbi; 
	size_t mem_used = 0; 
	PVOID      pvAddress = 0;

	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION)); 
	while (VirtualQuery(pvAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
	{
		if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE)
		{
			mem_used += mbi.RegionSize;
		}
		pvAddress = ((BYTE*)mbi.BaseAddress) + mbi.RegionSize;
	}

	return mem_used;
}

DWORD PgComputerInfo::getUsedVideoMemory()
{
	LPDIRECTDRAW7 lpDD = NULL;
	DDSCAPS2      ddsCaps2; 
	DWORD         dwTotal; 
	DWORD         dwFree;
	HRESULT       hr; 

	hr = ::DirectDrawCreateEx(NULL, (VOID**)&lpDD, IID_IDirectDraw7, NULL );
	if (FAILED(hr))
		return 0;

	// Initialize the structure.
	ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));

	ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM; 
	hr = lpDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
	if (FAILED(hr))
		return 0;

	if(lpDD)
		lpDD->Release();

	return dwTotal - dwFree;
}
