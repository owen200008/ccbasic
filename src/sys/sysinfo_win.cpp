
//
//取得系统信息系列函数
//
//
#include "../inc/basic.h"

#ifdef __BASICWINDOWS

#include <tlhelp32.h>
#include <Psapi.h>

#pragma comment(linker, "/defaultlib:psapi.lib")
#pragma comment(lib, "kernel32.lib")

//#include <ifmib.h>
#include <iptypes.h>
//#include <ipmib.h>
#include <Iphlpapi.h>
#include <conio.h>
#pragma comment(linker, "/defaultlib:Iphlpapi.lib")
__NS_BASIC_START

//!获取是否有键盘消息
int BasicKBHit()
{
	return _kbhit();
}
//
//取得CPU个数
int BasicGetCpuNumber()
{
	SYSTEM_INFO systemInfo;
	memset(&systemInfo, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
}

//
int BasicGetOSystemV(CBasicString& strOSVer)
{
	OSVERSIONINFO	osVerInfo;
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVerInfo);
	if (osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (osVerInfo.dwMajorVersion <= 4)
		{
			strOSVer = "Windows NT";				//4.0
		}
		else if (osVerInfo.dwMajorVersion == 5)
		{
			if (osVerInfo.dwMinorVersion == 2)
			{
				strOSVer = "Windows Server 2003";	//5.2
			}
			else if (osVerInfo.dwMinorVersion == 1)
			{
				strOSVer = "Windows XP";			//5.1
			}
			else
			{
				strOSVer = "Windows 2000";
			}
		}
		else if (osVerInfo.dwMajorVersion == 6)
		{
			if (osVerInfo.dwMinorVersion == 0)
			{
				strOSVer = "Windows Vista";			//6.0
			}
			else if (osVerInfo.dwMinorVersion == 1)
			{
				strOSVer = "Windows 7";				//6.1
			}
		}
		else
		{
			strOSVer = "Windows Unknown";
		}
	}
	else if (osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		strOSVer = "Windows 95/98";
	}
	else
	{
		strOSVer = "Windows 3.1";
	}
	return 0;
}

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

class CSystemInfo  
{
public:
	CSystemInfo();
	virtual ~CSystemInfo();

public:
	void GetMemoryInfo(DWORD &dwTotal, DWORD &dwTotalUse, DWORD &dwAvailPhys);
	BOOL GetCpuUsage(double& dUsage, DWORD& dwTimes);	//得到CPU使用率

protected:
	//取系统性能信息和系统时钟信息放到 m_SysPerfInfo和m_SysTimeInfo中
	BOOL QuerySysAndPerfInfo();		

	//structs
	typedef struct
	{
		DWORD   dwUnknown1;
		ULONG   uKeMaximumIncrement;
		ULONG   uPageSize;
		ULONG   uMmNumberOfPhysicalPages;
		ULONG   uMmLowestPhysicalPage;
		ULONG   uMmHighestPhysicalPage;
		ULONG   uAllocationGranularity;
		PVOID   pLowestUserAddress;
		PVOID   pMmHighestUserAddress;
		ULONG   uKeActiveProcessors;
		BYTE    bKeNumberProcessors;
		BYTE    bUnknown2;
		WORD    wUnknown3;
	} SYSTEM_BASIC_INFORMATION;

	typedef struct
	{
		LARGE_INTEGER   liIdleTime;
		DWORD           dwSpare[76];
	} SYSTEM_PERFORMANCE_INFORMATION;

	typedef struct
	{
		LARGE_INTEGER liKeBootTime;
		LARGE_INTEGER liKeSystemTime;
		LARGE_INTEGER liExpTimeZoneBias;
		ULONG         uCurrentTimeZoneId;
		DWORD         dwReserved;
	} SYSTEM_TIME_INFORMATION;

protected:
	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

	PROCNTQSI NtQuerySystemInformation;
	//value
	SYSTEM_PERFORMANCE_INFORMATION m_SysPerfInfo;	//系统性能信息
	SYSTEM_TIME_INFORMATION        m_SysTimeInfo;	//系统时钟信息
	SYSTEM_BASIC_INFORMATION       m_SysBaseInfo;	//系统基本信息
	LONG                           m_lStatus;
	//用于CPU使用率测试变量
	LARGE_INTEGER	m_liOldIdleTime;
	LARGE_INTEGER	m_liOldSystemTime;
	DWORD			m_dwOldTime;
	DWORD			m_dwUseTimes;
	//以后扩展为读具体一个进程的信息时用于指定进程
	//	CWBasicString			m_strProcName;
	//	long			m_lProcID;
	//option;
	BOOL	m_bSupport;

};

CSystemInfo::CSystemInfo()
{
	m_bSupport = FALSE;
	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle(_T("ntdll")),
		"NtQuerySystemInformation"
		);

	if (!NtQuerySystemInformation)
	{
		return;
	}
	m_lStatus = NtQuerySystemInformation(SystemBasicInformation, 
		&m_SysBaseInfo, sizeof(m_SysBaseInfo), NULL);
	if (m_lStatus != NO_ERROR)
	{
		return;
	}

	//第一次得到此时系统相关信息
	if( !QuerySysAndPerfInfo() )
	{
		return;
	}
	m_liOldIdleTime = m_SysPerfInfo.liIdleTime;
	m_liOldSystemTime = m_SysTimeInfo.liKeSystemTime; //第一次时间;
	m_bSupport = TRUE;
	m_dwUseTimes = 0;
}

CSystemInfo::~CSystemInfo()
{

}

BOOL CSystemInfo::QuerySysAndPerfInfo()
{
	m_lStatus = NtQuerySystemInformation(SystemTimeInformation, &m_SysTimeInfo, sizeof(m_SysTimeInfo), NULL);
	if (m_lStatus!=NO_ERROR)
		return FALSE;

	m_lStatus = NtQuerySystemInformation(SystemPerformanceInformation,&m_SysPerfInfo,sizeof(m_SysPerfInfo),NULL);
	if (m_lStatus != NO_ERROR)
		return FALSE;

	DWORD dwCurTime = GetTickCount();
	m_dwUseTimes = dwCurTime- m_dwOldTime;
	m_dwOldTime = dwCurTime;
	return TRUE;
}


//
//得到CPU使用率		dUsage,（使用率，百分比），多长时间；
//
BOOL CSystemInfo::GetCpuUsage(double &dUsage, DWORD& dwTimes)
{
	if( !m_bSupport )
	{
		return FALSE;
	}

	static double	s_dPrevUsage = 0;
	if ((GetTickCount() - m_dwOldTime) <= 5000)
	{//两次取的时间间隔小于1s时，用前一时刻取的值
		dUsage = s_dPrevUsage;
		dwTimes = m_dwUseTimes;
		return TRUE;
	}
	if (!QuerySysAndPerfInfo())
	{
		return FALSE;
	}
	double dIdleTime	= 0;
	double dSystemTime	= 0;
	if (m_liOldIdleTime.QuadPart != 0)
	{
		dIdleTime = Li2Double(m_SysPerfInfo.liIdleTime) - Li2Double(m_liOldIdleTime);
		dSystemTime = Li2Double(m_SysTimeInfo.liKeSystemTime) - Li2Double(m_liOldSystemTime);

		dIdleTime = dIdleTime / dSystemTime;
		//使用率 = 空闲时间的总数，再加上不同的程序
		dUsage = 100.0 - dIdleTime * 100.0 / (double)m_SysBaseInfo.bKeNumberProcessors;
		if(dUsage < 0.0)
		{
			dUsage = 0.0;
		}
		s_dPrevUsage = dUsage;
	}
	m_liOldIdleTime = m_SysPerfInfo.liIdleTime;
	m_liOldSystemTime = m_SysTimeInfo.liKeSystemTime;
	dwTimes = m_dwUseTimes;
	return TRUE;
}

//
//内存信息， 总物理内存，总使用内存，可用物理内存
//
/*
typedef struct _MEMORYSTATUSEX 
{
	DWORD dwLength; 
	DWORD dwMemoryLoad; 
	DWORDLONG ullTotalPhys; 
	DWORDLONG ullAvailPhys; 
	DWORDLONG ullTotalPageFile; 
	DWORDLONG ullAvailPageFile; 
	DWORDLONG ullTotalVirtual; 
	DWORDLONG ullAvailVirtual; 
	DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX; 
*/


typedef BOOL (WINAPI* GMS_EX)(LPMEMORYSTATUSEX lpBuffer);

//这里返回单位是 K
#define DIV (1024)

void CSystemInfo::GetMemoryInfo(DWORD &dwTotal, DWORD &dwTotalUse, DWORD &dwAvailPhys)
{//
	MEMORYSTATUSEX statex = { 0 };
	statex.dwLength = sizeof (statex);

	HMODULE hKernel32 =	GetModuleHandle(_T("kernel32.dll"));
	GMS_EX pfuncGlobalMemoryStatusEx = (GMS_EX)GetProcAddress(hKernel32,"GlobalMemoryStatusEx");
	if(pfuncGlobalMemoryStatusEx != NULL)
	{
		(*pfuncGlobalMemoryStatusEx)(&statex);
	}
	else
	{
		MEMORYSTATUS MemoryStatus = { 0 }; 
		MemoryStatus.dwLength = sizeof(MEMORYSTATUS); 	
		::GlobalMemoryStatus(&MemoryStatus);

		statex.ullTotalPhys	     = MemoryStatus.dwTotalPhys;
		statex.ullAvailPhys      = MemoryStatus.dwAvailPhys;
		statex.ullTotalPageFile	 = MemoryStatus.dwTotalPageFile;
		statex.ullAvailPageFile  = MemoryStatus.dwAvailPageFile;
		statex.ullTotalVirtual   = MemoryStatus.dwTotalVirtual; 
		statex.ullAvailVirtual   = MemoryStatus.dwAvailVirtual; 
	}

	dwTotal		= (DWORD)(statex.ullTotalPhys / DIV);
	dwTotalUse	= (DWORD)((statex.ullTotalPageFile - statex.ullAvailPageFile) / DIV);
	dwAvailPhys = (DWORD)(statex.ullAvailPhys / DIV);
}

CSystemInfo g_sysInfo;

//
//取得CPU利用率,单位百分比
int BasicGetCPUUse()
{
	double dUsage = 0;
	DWORD dwTime = 0;
	if(g_sysInfo.GetCpuUsage(dUsage, dwTime))
		return int(dUsage*100);
	return 0;
}
//

//取得内存信息，单位K
//参数：
//dwPhysicalMemory	物理内存 
//dwAvailMemory		可用内存
//dwUsedMemory		使用内存
//dwVirtualMemory	虚拟内存
void BasicGetMemoryInfo(DWORD& dwPhysicalMemory, 
				   DWORD& dwAvailMemory, 
				   DWORD& dwUsedMemory, 
				   DWORD& dwVirtualMemory)
{
	g_sysInfo.GetMemoryInfo(dwPhysicalMemory, dwUsedMemory, dwAvailMemory);
}

typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	DWORD PeakWorkingSetSize;
	DWORD WorkingSetSize;
	DWORD QuotaPeakPagedPoolUsage;
	DWORD QuotaPagedPoolUsage;
	DWORD QuotaPeakNonPagedPoolUsage;
	DWORD QuotaNonPagedPoolUsage;
	DWORD PagefileUsage;
	DWORD PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;
typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;

BOOL
WINAPI
GetProcessMemoryInfo(
					 HANDLE Process,
					 PPROCESS_MEMORY_COUNTERS ppsmemCounters,
					 DWORD cb
					 );

typedef BOOL (WINAPI * GETPROCESSMEMORYINFO_FUNC) (HANDLE process, PPROCESS_MEMORY_COUNTERS counters, DWORD cb);
//
//取得进程使用内存 单位K
DWORD BasicGetProcessMem(HANDLE hProcess, BOOL bKeepHandle)
{
	static GETPROCESSMEMORYINFO_FUNC sGetMemInfo = NULL;
	static HANDLE sProcess = hProcess;
	static HMODULE sPSModule = NULL;
	static BOOL sKeepHandle = FALSE;
	if(bKeepHandle)
	{
		//保留句柄
		sKeepHandle = TRUE;
	}
	if(sGetMemInfo == NULL)
	{
		sPSModule = LoadLibrary(_T("psapi.dll"));
		if(sPSModule)
		{
			sGetMemInfo = (GETPROCESSMEMORYINFO_FUNC)GetProcAddress(sPSModule, "GetProcessMemoryInfo");
		}
	}
	DWORD dwMem = 0;
	if(sGetMemInfo)
	{
		if(sProcess == NULL)
		{
			//取当前进程
			sProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
		}
		if(sProcess)
		{
			PROCESS_MEMORY_COUNTERS pmc;
			sGetMemInfo(sProcess, &pmc, sizeof(pmc));
			dwMem = max((DWORD)pmc.PagefileUsage, (DWORD)pmc.WorkingSetSize);
		}
	}
	if(!sKeepHandle)
	{
		if(sProcess && hProcess == NULL)
		{
			CloseHandle(sProcess);
			sProcess = NULL;
		}
		if(sPSModule)
		{
			FreeLibrary(sPSModule);
			sPSModule = NULL;
			sGetMemInfo = NULL;
		}
	}
	return dwMem/1024;
}
//

//
//取得硬盘信息
DWORD BasicGetDiskInfo(char* pszDiskBuffer, int nBufferLen)
{
	if(pszDiskBuffer == NULL || nBufferLen == 0)
		return 0;

	CBasicString strDiskInfo;
	char szTemp[64];

	char szAllPath[1024];
	GetLogicalDriveStringsA(_countof(szAllPath), szAllPath);

	const char* lpszPath = szAllPath;
	while(lpszPath[0] != '\0')
	{
		UINT uDrive = GetDriveTypeA(lpszPath);
		if(uDrive == DRIVE_FIXED || uDrive == DRIVE_RAMDISK)	//只处理硬盘和RAM盘
		{
			DWORD dwSectorsPerCluster = 0;
			DWORD dwBytesPerSector = 0;
			DWORD dwFreeClusters = 0;
			DWORD dwTotalClusters = 0;
			if(GetDiskFreeSpaceA(lpszPath, &dwSectorsPerCluster,
				&dwBytesPerSector, &dwFreeClusters, &dwTotalClusters))
			{
				long lFreeSpace  = (long)(((double)dwFreeClusters * dwSectorsPerCluster * dwBytesPerSector) / (1024 * 1024));
				long lTotalSpace = (long)(((double)dwTotalClusters * dwSectorsPerCluster * dwBytesPerSector) / (1024 * 1024));

				sprintf(szTemp, "%s %d/%d | ", lpszPath, lFreeSpace, lTotalSpace);
				strDiskInfo += szTemp;
			}
		}
		lpszPath += __tcslen(lpszPath) + 1;
	}

	int nInfoLen = strDiskInfo.GetLength();
	if(nBufferLen > nInfoLen)
	{
		__tcscpy(pszDiskBuffer, strDiskInfo.c_str());
		return nInfoLen;
	}
	return 0;
}
//

//取路径所在磁盘的剩余空间 单位M
long BasicGetDiskFreeinfo(const char* lpszPath)
{
	DWORD dwSectorsPerCluster = 0;
	DWORD dwBytesPerSector = 0;
	DWORD dwFreeClusters = 0;
	DWORD dwTotalClusters = 0;
	if(GetDiskFreeSpaceA(lpszPath, &dwSectorsPerCluster,
		&dwBytesPerSector, &dwFreeClusters, &dwTotalClusters))
	{
		long lFreeSpace  = (long)(((double)dwFreeClusters * dwSectorsPerCluster * dwBytesPerSector) / (1024 * 1024));
		return lFreeSpace;
	}
	return 999999;
}


//取得系统启动时间，单位：毫秒
DWORD BasicGetTickTime()
{
	//return ::GetTickCount();
	LARGE_INTEGER lFreq, lCounter;
	QueryPerformanceFrequency(&lFreq);
	QueryPerformanceCounter(&lCounter);
	return DWORD((((double)lCounter.QuadPart)/lFreq.QuadPart)*1000);
}

double BasicGetTickTimeCount()
{
    LARGE_INTEGER lFreq, lCounter;
    QueryPerformanceFrequency(&lFreq);
    QueryPerformanceCounter(&lCounter);
    return (((double)lCounter.QuadPart)/lFreq.QuadPart)*1000;
}

//取得模块名
//如hModule==NULL，则取当前主程序名
CBasicString BasicGetModuleName(HANDLE hModule)
{
	char szPath[_MAX_PATH];
	szPath[0] = '\0';
	GetModuleFileNameA((HMODULE)hModule, szPath, sizeof(szPath));
	return szPath;
}

long BasicGetModuleName(HANDLE hModule, char* pszBuffer, int nBufLen)
{
	return GetModuleFileNameA((HMODULE)hModule, pszBuffer, nBufLen);
}

CBasicString BasicGetModuleTitle(HANDLE hModule, BOOL bExt)
{
	CBasicString strModule = BasicGetModuleName(hModule);
	int nPos = strModule.ReverseFind(WIDEPATHSPLIT);
	if (nPos >= 0)
		strModule = strModule.Mid(nPos + 1);
	if (!bExt)
	{
		//去掉后缀名
		nPos = strModule.ReverseFind('.');
		if (nPos >= 0)
			strModule = strModule.Left(nPos);
	}
	return strModule;
}

//取得路径
CBasicString BasicGetModulePath(HANDLE hModule)
{
	char szPath[_MAX_PATH];
	szPath[0] = '\0';
	GetModuleFileNameA((HMODULE)hModule, szPath, sizeof(szPath));
	int i = 0;
	for (i = strlen(szPath) - 1; i >= 0 && szPath[i] != PATHSPLIT_S; i--);
	i++;
	szPath[i] = '\0';
	return szPath;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//取本机IP地址和子网掩码
int BasicGetLocalAddrInfo(PLOCALADDR pBuffer, int cbBuffer)
{
	HINSTANCE hDll = (HINSTANCE)BasicLoadLibrary("Iphlpapi.dll");
	if (hDll == NULL)
	{
		return 0;
	}
 
	char   pTabBuffer[10000];   
	memset(pTabBuffer, 0, sizeof(pTabBuffer));   
	PMIB_IPADDRTABLE   pTable   =   (PMIB_IPADDRTABLE)pTabBuffer;   
	DWORD   dwSize   =   sizeof(pTabBuffer);
	int nRet = 0;
	if(GetIpAddrTable(pTable,&dwSize,FALSE)   ==   NO_ERROR)   
	{   
		for(DWORD i = 0; i< pTable->dwNumEntries && nRet < cbBuffer; i++)   
		{  
			DWORD   addr   =   pTable->table[i].dwAddr;   
			DWORD   subnet   =   pTable->table[i].dwMask;     
			//   Filter   127.0.0.1   
			if   (addr   !=   0x0100007f)   
			{   
				sprintf(pBuffer[nRet].m_szIP,"%d.%d.%d.%d",(addr & 0xFF),((addr>>8) & 0xFF),((addr>>16)& 0xFF),((addr>>24) & 0xFF));  
				sprintf(pBuffer[nRet].m_szMask, "%d.%d.%d.%d", (subnet & 0xFF), ((subnet >> 8) & 0xFF), ((subnet >> 16) & 0xFF), ((subnet >> 24) & 0xFF));
				nRet++;
			}   
		}   
	}   
	BasicFreeLibrary(hDll);
	return   nRet;   
}

long BasicCheckProcess(DWORD dwProcessID)
{
	typedef DWORD (WINAPI *PGetProcessId)(HANDLE Process);
	static PGetProcessId pfnGetProcessId = (PGetProcessId)-1;
	if(pfnGetProcessId == (PGetProcessId)-1)
	{
		pfnGetProcessId = NULL;
		HMODULE hDllLib = GetModuleHandle(_T("Kernel32.dll"));
		if(hDllLib)
		{
			pfnGetProcessId = (PGetProcessId)::GetProcAddress(hDllLib, "GetProcessId");
		}
	}

	if(pfnGetProcessId == NULL)
	{
		return BASIC_PSL_RET_ERROR;
	}

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
	if(hProcess == NULL)
	{
		DWORD dwLastError = ::GetLastError();
		switch(dwLastError)
		{
		case ERROR_ACCESS_DENIED:
			{
				return BASIC_PSL_RET_NO_RIGHT;
				break;
			}
		default:
			{
				return BASIC_PSL_RET_NOT_EXIST;
				break;
			}
		}
	}	

	DWORD dwCheckPid = pfnGetProcessId(hProcess);
	long lRet = (dwCheckPid == dwProcessID) ? BASIC_PSL_RET_STILL_LIVE : BASIC_PSL_RET_NOT_EXIST;
	::CloseHandle(hProcess);

	return lRet;
}

//************************************************************************
// Author:    Mini.J @2009/3/13
// Method:    BasicProcessIsTerminated => 进程是否退出
//			查看进程列表中是否有传入的进程ID。如果存在说明未退出
// Returns:   BOOL => 
// Parameter: DWORD dwProcessID => 进程ID
//************************************************************************
BOOL BasicProcessIsTerminated(DWORD dwProcessID)
{
	long lCheck = BasicCheckProcess(dwProcessID);
	return lCheck == BASIC_PSL_RET_NOT_EXIST ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
void* BasicLoadLibrary(const char* lpszLibFileName)
{
	return LoadLibraryA(lpszLibFileName);
}
long BasicFreeLibrary(void* hModule)
{
	if(FreeLibrary((HMODULE)hModule))
	{
		return 0;
	}
	return -1;
}
void*	BasicGetProcAddress(void* hModule, const char* lpszProcName)
{
	return GetProcAddress((HMODULE)hModule, lpszProcName);
}

BOOL BasicSetSysTime(time_t tTime)
{
	CTime tm(tTime);
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wYear = tm.GetYear();
	st.wMonth = tm.GetMonth();
	st.wDay = tm.GetDay();
	st.wHour = tm.GetHour();
	st.wMinute = tm.GetMinute();
	st.wSecond = tm.GetSecond();
	return SetLocalTime(&st);
}

/*
 * \brief 时间转换函数
 */
ULONGLONG file_time_2_utc(const FILETIME* ftime)
{
	ULARGE_INTEGER li;
	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

/*
 * \brief CProcessInfo构造函数
 */
CProcessInfo::CProcessInfo(DWORD nProcessId)
{
	m_nProcessId = nProcessId;
	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;

	LONGLONG nSystem_time;
	LONGLONG nTime;

	m_nCpuCount = BasicGetCpuNumber();
	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, nProcessId);
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return ;
	}

	nSystem_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / m_nCpuCount;
	nTime = file_time_2_utc(&now);

	m_nLastSystemTime = nSystem_time;
	m_nLastTime = nTime;
	
}



//
//取得指定进程的CPU利用率，单位百分比
int CProcessInfo::GetProcessCpu()
{
	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;

	LONGLONG nSystem_time;
	LONGLONG nTime;
	LONGLONG nSystem_time_delta;
	LONGLONG nTime_delta;

	int nCpu = -1;
	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, m_nProcessId);
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return -1;
	}

	nSystem_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / m_nCpuCount;
	nTime = file_time_2_utc(&now);

	nSystem_time_delta = nSystem_time - m_nLastSystemTime;
	nTime_delta = nTime - m_nLastTime;
	if (nTime_delta == 0)
	{
		return -1;
	}
	nCpu = (int)(((nSystem_time_delta)*100 + nTime_delta/2)/nTime_delta);
	m_nLastSystemTime = nSystem_time;
	m_nLastTime = nTime;
	return nCpu;
}

CProcessInfo::~CProcessInfo()
{

}


double BasicGetHighPerformanceCounter()
{
        LARGE_INTEGER time;
        ::QueryPerformanceCounter(&time);

        static double fFreq = 0.0;
        if(fFreq == 0.0)
        {
                LARGE_INTEGER nFreq;
                QueryPerformanceFrequency(&nFreq);
                fFreq = nFreq.QuadPart / 1000000.0;
        }
        return (time.QuadPart / fFreq);
}


__NS_BASIC_END
#endif 


