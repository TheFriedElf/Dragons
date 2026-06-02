#ifndef FREEDOM_DRAGONICA_UTIL_COMPUTERINFO_H
#define FREEDOM_DRAGONICA_UTIL_COMPUTERINFO_H

#pragma comment(lib, "ddraw.lib")

struct CPUINFO
{
	char	strName[48];
	int		iMhz;
	int		iRam; // mb
	int		iNumProcess;
	int		iProcessorArchitecture;
	int		iProcessorType;
};

struct GPUINFO
{
	char	strName[512];
	char	strVersion[20];
	int		iVRam;
	int		iVRamFree;
	int		iWidth;
	int		iHeight;
	int		iFullScreeen;
};

enum OSTYPE
{
	OS_UNKNOWN=0,
	OS_WIN95,
	OS_WIN98,
	OS_WIN2K,
	OS_WINXP,
	OS_WINNT,
	OS_WINVISTA,
	OS_WIN2003,
	OS_MAX_NUM,
};

enum DEVICE_GRADE
{
	DEVICE_GRADE_UNKNOWN = -1,
	DEVICE_GRADE_VERY_LOW,
	DEVICE_GRADE_LOW,
	DEVICE_GRADE_MEDIUM,
	DEVICE_GRADE_HIGH,
	DEVICE_GRADE_VERY_HIGH,
	MAX_DEVICE_GRADE_NUM,
};

struct OSINFO
{
	OSTYPE	iType;
	int		iVersion1;
	int		iVersion2;
	int		iVersion3;
};

class PgComputerInfo
{
public:
	static void CheckComputerInfo(LPDIRECT3D9 pd3dDevice, bool bCheckGPU = true, bool bCheckOS = true, bool bCheckCPU = true);
	static CPUINFO&	GetCPUInfo() { return mCPUInfo;}
	static GPUINFO&	GetGPUInfo() { return mGPUInfo;}
	static OSINFO&	GetOSInfo() { return mOSInfo;}	
	static void InitMemStatus();
	static float GetVideoMemUsage();
	static float GetSystemMemUsage();
	static int GetIdealProcessorForThread(int threadCount);
	static DEVICE_GRADE GetCpuGrade() { return mCPUGrade; }
	static DEVICE_GRADE GetGpuGrade() { return mGPUGrade; }
	static DEVICE_GRADE GetMemGrade() { return mMEMGrade; }
protected:
	PgComputerInfo() {};

	static void setGPUInfo(LPDIRECT3D9 pd3dDevice);
	static void setCPUInfo();
	static void setOSInfo();
	static float getCurrentVideoMemory();
	static float getCurrentSystemMemory();
	static DWORD getUsedVideoMemory();	
	static size_t countMemUsage();

	static CPUINFO mCPUInfo;
	static GPUINFO mGPUInfo;
	static OSINFO mOSInfo;
	static float mInitVideoMem;
	static float mInitSystemMem;
	static DEVICE_GRADE mGPUGrade;
	static DEVICE_GRADE mCPUGrade;
	static DEVICE_GRADE mMEMGrade;
};
#endif //FREEDOM_DRAGONICA_UTIL_COMPUTERINFO_H