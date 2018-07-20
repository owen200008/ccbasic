
//
//取得系统信息系列函数
//
//
#include "../inc/basic.h"
#ifdef __MAC
#include <sys/sysctl.h>
//#include <sys/disk.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
 
#include <mach/mach.h>
#include <mach/mach_error.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define Basic_statfs        statfs
#define Basic_getmntinfo    getmntinfo


__NS_BASIC_START
 
//!获取是否有键盘消息
int BasicKBHit()
{
    return 0;
}

//!取得操作系统版本
/*! 
*\param strOSVer	返回操作系统版本信息
*\return 成功返回0，失败返回非0
*/
_BASIC_DLL_API int BasicGetOSystemV(CBasicString& strOSVer)
{	
	struct utsname osbuf;
    	uname(&osbuf);
 	strOSVer.Format("%s %s", osbuf.sysname, osbuf.release);
	return 0;
}
//

//
//!取得CPU内核个数
/*!
 *\return 返回CPU内核个数
 */
_BASIC_DLL_API int BasicGetCpuNumber()
{
	int nName[2] = {CTL_HW,HW_NCPU};
	int nCPU = 0;
	size_t sSize = sizeof(nCPU);
	sysctl(nName, 2, &nCPU, &sSize, NULL, 0);
	return nCPU;
    //	return sysconf(_SC_NPROCESSORS_ONLN);
}
//

//
//!取得CPU内核个数
/*! 
*\return 返回CPU内核个数
*/
_BASIC_DLL_API int BasicGetNumberOfCpu()
{
	int nName[2] = {CTL_HW,HW_NCPU};
	int nCPU = 0;
	size_t sSize = sizeof(nCPU);
	sysctl(nName, 2, &nCPU, &sSize, NULL, 0);
	return nCPU;
//	return sysconf(_SC_NPROCESSORS_ONLN);
}
//


typedef unsigned long long TIC_t;
typedef          long long SIC_t;
typedef struct CPU_t {
	unsigned long new_userTicks, new_NiceTicks, new_SysTicks, new_IdleTicks ;
	unsigned long old_userTicks, old_NiceTicks, old_SysTicks, old_IdleTicks ;
	unsigned id;  // the CPU ID number
} CPU_t;


static CPU_t *pCpu = NULL;
static int nCpu_tot;

static CPU_t *cpus_refresh (CPU_t *pCpu)
{
	mach_msg_type_number_t numCpuCount;
	processor_info_array_t ayInfoArray;
	mach_msg_type_number_t numInfoCount;
	kern_return_t kr;
	processor_cpu_load_info_data_t* ayCpuLoadInfo;
	unsigned long old_ticks,new_ticks,old_totalTicks,new_totalTicks;
	int cpu,state;
	
	kr = host_processor_info(mach_host_self(),PROCESSOR_CPU_LOAD_INFO, &numCpuCount, &ayInfoArray, &numInfoCount);
	if (kr) 
		return NULL;
	
	nCpu_tot = numCpuCount;
	if(pCpu == NULL && nCpu_tot > 0)
	{
		pCpu = (CPU_t *)malloc((nCpu_tot + 1) * sizeof(CPU_t));
		memset(pCpu, 0, (nCpu_tot + 1) * sizeof(CPU_t));
		for (int i = 0; i < nCpu_tot; i++)
			pCpu[i].id = i;
		
	}
	
	ayCpuLoadInfo = (processor_cpu_load_info_data_t*) ayInfoArray;
	if(ayCpuLoadInfo == NULL)
		return NULL;

	
	for (int i = 0; i < nCpu_tot; i++)
	{
		pCpu[i].new_userTicks	= ayCpuLoadInfo[i].cpu_ticks[CPU_STATE_USER];
		pCpu[i].new_NiceTicks	= ayCpuLoadInfo[i].cpu_ticks[CPU_STATE_NICE];
		pCpu[i].new_SysTicks	= ayCpuLoadInfo[i].cpu_ticks[CPU_STATE_SYSTEM];
		pCpu[i].new_IdleTicks	= ayCpuLoadInfo[i].cpu_ticks[CPU_STATE_IDLE];
	}
	return pCpu;
}
	
//
//!取得CPU利用率,单位百分比
/*!
*\return 返回CPU利用率百分比*100
 */
int BasicGetCPUUse()
{
	pCpu = cpus_refresh(pCpu);
	
	if(pCpu == NULL)
		return 0;
	
	unsigned long nNewTotalTicks = 0;
	unsigned long nOldTotalTicks = 0;
	
	unsigned long nNewTicks = 0;
	unsigned long nOldTicks = 0;
	
	unsigned long nOldTotalSysTicks = 0;
	for(int i = 0; i< nCpu_tot; i++)
	{
		nNewTotalTicks	+= pCpu[i].new_userTicks + pCpu[i].new_NiceTicks + pCpu[i].new_SysTicks + pCpu[i].new_IdleTicks;
		nOldTotalTicks	+= pCpu[i].old_userTicks + pCpu[i].old_NiceTicks + pCpu[i].old_SysTicks + pCpu[i].old_IdleTicks;
		nNewTicks		+= pCpu[i].new_userTicks + pCpu[i].new_NiceTicks + pCpu[i].new_SysTicks;
		nOldTicks		+= pCpu[i].old_userTicks + pCpu[i].old_NiceTicks + pCpu[i].old_SysTicks;
			
		pCpu[i].old_userTicks	= pCpu[i].new_userTicks;
		pCpu[i].old_NiceTicks	= pCpu[i].new_NiceTicks;
		pCpu[i].old_SysTicks	= pCpu[i].new_SysTicks;
		pCpu[i].old_IdleTicks	= pCpu[i].new_IdleTicks;
	}
	
	int dUsage = (nNewTotalTicks - nOldTotalTicks > 0) ? (double)(nNewTicks - nOldTicks) / (nNewTotalTicks - nOldTotalTicks) *100 : 0;
	return dUsage;
}

//
//! 取得内存信息，单位K
/*!
 *\param dwPhysicalMemory	物理内存 
 *\param dwAvailMemory		可用内存
 *\param dwUsedMemory		使用内存
 *\param dwVirtualMemory	虚拟内存
 */
_BASIC_DLL_API void BasicGetMemoryInfo(DWORD& dwPhysicalMemory, 
				   DWORD& dwAvailMemory, 
				   DWORD& dwUsedMemory, 
				   DWORD& dwVirtualMemory)
{
	int nName[2] = {CTL_HW,HW_PHYSMEM};
	size_t sSize = sizeof(dwPhysicalMemory);

	//User HW_MEMSIZE by 64-bit
	sysctl(nName, 2, &dwPhysicalMemory, &sSize, NULL, 0);

	nName[1] = 
	sysctl(nName, 2, &dwPhysicalMemory, &sSize, NULL, 0);
}

//! 取得进程使用内存 单位K
/*!
 *\param  hProcess，进程句柄，如hProcess==NULL,则取当前进程
 *\param  bKeepHandle，如果应用程序需要长期定时调用，则置为true
 */
_BASIC_DLL_API DWORD BasicGetProcessMemory(HANDLE hProcess, bool bKeepHandle )
{
        struct task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
        task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
        return t_info.resident_size;
}

//! 取得硬盘信息
/*!
*\param  pszDiskBuffer 硬盘信息保存空间
*\param  nBufferLen 硬盘信息保存空间长度
 */
_BASIC_DLL_API DWORD BasicGetDiskInfo(TCHAR* pszDiskBuffer, int nBufferLen)
{
    
	struct  Basic_statfs *s = NULL;
	int nDiskCount = Basic_getmntinfo(&s, 0);
	TCHAR* p = pszDiskBuffer;
	int nLen = nBufferLen;
	for(int i=0; i<nDiskCount; i++)
	{
		int n = _stprintf_s(p, nLen, "%s %d/%d|",
			s[i].f_mntonname,
			(s[i].f_bavail * s[i].f_bsize) / (1024 * 1024),
			(s[i].f_blocks * s[i].f_bsize) / (1024 * 1024)
			);
		nLen -= n;
		p += n;
	}
	return nBufferLen - nLen;
}
//

//! 取路径所在磁盘的剩余空间
/*!
 *\param lpszPath 指定路径
 *\return 返回剩余磁盘空间，单位M
 */
_BASIC_DLL_API long BasicGetDiskFreeinfo(const char* lpszPath)
{
	long lRet = -1;
	struct Basic_statfs s;
	if(Basic_statfs(lpszPath, &s) < 0)
	{
		lRet = 0;
	}
	else
	{
		lRet = (s.f_bavail * s.f_bsize) / (1024 * 1024);
	}
	return lRet;
}


//! 取得系统启动时间 
/*!
 *\return 返回时间值，单位：毫秒
 *\remark 用于测量耗时
 */
DWORD BasicGetTickTime()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec*1000 + tp.tv_usec/1000;
}

double  BasicGetTickTimeCount()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	double dRet = tp.tv_sec;
	dRet *= 1000;
	return dRet + tp.tv_usec / 1000;
}

//! 取得模块名，包括全路径
/*!
 *\param hModule  如hModule==NULL，则取当前主程序名，
 *\return 模块名
 */
_BASIC_DLL_API CBasicString BasicGetModuleName(HANDLE hModule)
{
	char szPath[_MAX_PATH];
	BasicGetModuleName(hModule, szPath, _MAX_PATH);
	return CBasicString(szPath);
}

//! 取得模块名，包括全路径
/*!
 *\param hModule  如hModule==NULL，则取当前主程序名，
 *\param pszBuffer 返回数据的空间
 *\param nBufLen 返回数据的空间长度
 *\return  返回实际长度
 */
_BASIC_DLL_API long BasicGetModuleName(HANDLE hModule, char* pszBuffer, int nBufLen)
{
	//??unicode
	unsigned int nPathLen = nBufLen;
	return _NSGetExecutablePath(pszBuffer, &nPathLen);
}


//! 取得模块名，不包括全路径
/*!
 *\param hModule  如hModule==NULL，则取当前主程序名，
 *\param bExt 是否包括扩展名
 *\return 模块名
 */
_BASIC_DLL_API CBasicString BasicGetModuleTitle(HANDLE hModule, bool bExt)
{
	CBasicString strModule = BasicGetModuleName(hModule);
	int nPos = strModule.ReverseFind(PATHSPLIT);
	if(nPos >= 0)
		strModule = strModule.Mid(nPos + 1);
	return strModule;
}

//! 取得模块路径
/*!
 *\param hModule  如hModule==NULL，则取当前主程序路径
 *\return 模块路径
 */
_BASIC_DLL_API CBasicString BasicGetModulePath(HANDLE hModule)
{
	char szPath[_MAX_PATH];
	CBasicString strPath = basiclib::BasicGetModuleName(hModule);
	__tcscpyn(szPath, _MAX_PATH, strPath.c_str());
	int i = 0;
	for (i = strlen(szPath) - 1; i >= 0 && szPath[i] != PATHSPLIT; i--);
	i++;
	szPath[i] = '\0';
	return szPath;
}

//! 判断某个进程是否结束
/*!
*\param dwProcessID	进程ID
 */
_BASIC_DLL_API bool BasicProcessIsTerminated(DWORD dwProcessID)
{
	CBasicString strFile;
	CBasicString strCmd;
	strCmd.Format("ps -p %d", dwProcessID);
	FILE* pTmp = popen(strCmd.c_str(), "r");
	if (pTmp != NULL)
	{
		char szBuf[4096];
		int nReadLen = 0;
		while ((nReadLen = fread(szBuf, 1, sizeof(szBuf), pTmp)) > 0)
		{
			strFile += CBasicString(szBuf, nReadLen);
		}
		fclose(pTmp);
	}
	if (strFile.GetLength() <= 0)
	{
		return true;
	}
	CBasicStringArray ayItem;
	BasicSpliteString(strFile.c_str(), '\n', IntoContainer_s<CBasicStringArray>(ayItem));
	int nLineCnt = ayItem.GetSize();
	if (nLineCnt == 2)
	{
		return false;
	}
	return true;
}

//! 修改系统时间
/*!
* \param tTime:需要设置的时间，精确到秒
* \return 修改成功或者失败
 */
_BASIC_DLL_API bool BasicSetSysTime(time_t tTime)
{
	return true;
}

#define MAC_PROCESS_CMD				"ps -el"		// Linux
#define PS_CMD_COL_PID				"PID"		// Linux 
#define PS_CMD_COL_PPID				"PPID"		// Linux 
#define PS_CMD_COL_CMD				"CMD"		// Linux 
 

//! 取本机IP地址和子网掩码
/*!
 *\param pBuffer cbBuffer 输入IP地址信息空间
 *\return 返回IP地址数量
 */
_BASIC_DLL_API int BasicGetLocalAddrInfo(PLOCALADDR pBuffer, int cbBuffer)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		return 0;
	}
	struct ifreq ifr;
	bool found = true;
	int i = 0;
	int nIndex = 0;
	while(found && nIndex < cbBuffer)
	{
		sprintf(ifr.ifr_name, "en%d", i++);
		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
		{
			break;
		}
		struct sockaddr_in* pAddrTmp = (struct sockaddr_in*)&ifr.ifr_addr;
		strncpy(pBuffer[nIndex].m_szIP, inet_ntoa(pAddrTmp->sin_addr), MAX_IP);
		if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
		{
			break;
		}
		pAddrTmp= (struct sockaddr_in*)&ifr.ifr_addr;
		strncpy(pBuffer[nIndex].m_szMask, inet_ntoa(pAddrTmp->sin_addr), MAX_IP);
		if (strlen(pBuffer[nIndex].m_szIP) <= 0 || strlen(pBuffer[nIndex].m_szMask) <= 0)
		{
			found = false;
		}
		nIndex++;
	}
	close(sock);
	return nIndex;
}
 
//取网卡状态信息
int GetMacInfo(int& nAdapterIndex, char* pMac, int nLength)
{
	char szNet[8];
	memset(szNet, 0, 8);
	if(nAdapterIndex < 0)
	{
		nAdapterIndex = 0;
	}
	sprintf(szNet, "en%d", nAdapterIndex);
	struct ifaddrs *ifas = NULL ;
    struct ifaddrs *ifasTemp = NULL;
	if (getifaddrs(&ifas) != 0)
	{
		return -1;
	}
    
    ifasTemp = ifas;
    
	for (;ifasTemp != NULL; ifasTemp = (*ifasTemp).ifa_next)
	{
		if((ifasTemp->ifa_addr)->sa_family == AF_LINK)
		{
			struct sockaddr_dl *pAddr = (struct sockaddr_dl*)ifasTemp->ifa_addr;
			if(strcmp(szNet, ifasTemp->ifa_name) == 0)
			{
				char* basemac = &(pAddr->sdl_data[pAddr->sdl_nlen]);
				memcpy(pMac, basemac, pAddr->sdl_alen > nLength ? nLength : pAddr->sdl_alen);
				break;		
			}
		}
	}	

	freeifaddrs(ifas);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//!动态库调用的函数

//! 加载动态库
/*!
 *\param lpszLibFileName	动态库文件名
 *\return 成功返回非零动态库句柄，失败返回NULL
 */
_BASIC_DLL_API void*	BasicLoadLibrary(const char* lpszLibFileName)
{
	void* hDll = dlopen(lpszLibFileName, RTLD_NOW|RTLD_GLOBAL);
    if(hDll == NULL)
    {
        char* pError = dlerror();
        if(pError != NULL)
			basiclib::BasicLogEventErrorV("linking (%s) error occurred: (%s) \n", lpszLibFileName, pError);
    }
    return hDll;
}


//! 释放动态库
/*!
 *\param hModule	动态库句柄
 *\return 成功返回0，否则失败
 */
_BASIC_DLL_API long	BasicFreeLibrary(void* hModule)
{
	return dlclose(hModule);
}

//!取动态库函数入口
/*!
 *\param hModule	动态库句柄
 *\param lpszProcName 函数名
 *\return 成功返回非零函数地址，失败返回NULL
 */
_BASIC_DLL_API void*	BasicGetProcAddress(void* hModule, LPCTSTR lpszProcName)
{
    return dlsym(hModule, lpszProcName);
}

/*
 * \brief 用户计算进程的CPU使用率
 */
CProcessInfo::CProcessInfo(DWORD nProcessId)
{
	m_nProcessId = nProcessId;

	m_nLastUtime = 0;

	m_nLastStime = 0;

	m_nCutime = 0;

	m_nCstime = 0;

	fpidstat = NULL;


}
 
CProcessInfo::~CProcessInfo()
{
	
}

int CProcessInfo::GetProcessCpu()
{
	return BasicGetCPUUse();
}
 

//! 取高精度的系统计数，从系统启动开始。单位：微秒（10-6 秒）。
/*! 
*\return  如果成功返回 从系统启动到现在的微秒数。
*\remarks 这个函数需要硬件支持（HRT），主要用于性能记录中的时间记录
*/

_BASIC_DLL_API  double BasicGetHighPerformanceCounter()
{
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return ((double)tp.tv_sec*1000000 + tp.tv_usec);
}


#define cpuid(in,a,b,c,d)  asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));
bool getcpuid(char *id, size_t max){
    int i;
    unsigned long li, maxi, maxei, eax, ebx, ecx, edx, unused;

    cpuid(0, maxi, unused, unused, unused);
    maxi &= 0xffff;

    if(maxi < 3){
        return false;
    }

    cpuid(3, eax, ebx, ecx, edx);

    snprintf(id, max, "%08lx %08lx %08lx %08lx", eax, ebx, ecx, edx);
    return true;
}

//! 获取取机器的特征码
/*!
*/
bool BasicGetMachineSerial(CBasicString& str){
    char szBuf[256] = { 0 };
    if(!getcpuid(szBuf, 128)){
        return false;
    }
    str = basiclib::Basic_MD5(szBuf, 256);
    return true;
}

__NS_BASIC_END


#endif //__MAC
