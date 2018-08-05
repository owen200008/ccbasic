/***********************************************************************************************
// 文件名:     sysinfo.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 8:55:21
// 内容描述:   取得系统信息系列函数
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SYSINFO_H
#define BASIC_SYSINFO_H

//
//取得系统信息系列函数
//

__NS_BASIC_START


//!获取是否有键盘消息
_BASIC_DLL_API_C int _BASIC_DLL_API BasicKBHit();

//!取得操作系统版本
/*!
*\param strOSVer	返回操作系统版本信息
*\return 成功返回0，失败返回非0
*/
_BASIC_DLL_API_C int _BASIC_DLL_API BasicGetOSystemV(CBasicString& strOSVer);
//

//
//!取得CPU内核个数
/*!
*\return 返回CPU内核个数
*/
_BASIC_DLL_API_C  int _BASIC_DLL_API BasicGetCpuNumber();
//

//
//!取得CPU利用率,单位百分比
/*!
*\return 返回CPU利用率百分比*100
*/
_BASIC_DLL_API_C  int _BASIC_DLL_API BasicGetCPUUse();
//

//
//! 取得内存信息，单位K
/*!
*\param dwPhysicalMemory	物理内存
*\param dwAvailMemory		可用内存
*\param dwUsedMemory		使用内存
*\param dwVirtualMemory	虚拟内存
*/
_BASIC_DLL_API_C  void _BASIC_DLL_API BasicGetMemoryInfo(DWORD& dwPhysicalMemory,
                                                         DWORD& dwAvailMemory,
                                                         DWORD& dwUsedMemory,
                                                         DWORD& dwVirtualMemory);

//! 取得进程使用内存 单位K
/*!
*\param  hProcess，进程句柄，如hProcess==NULL,则取当前进程
*\param  bKeepHandle，如果应用程序需要长期定时调用，则置为true
*/
_BASIC_DLL_API_C  DWORD _BASIC_DLL_API BasicGetProcessMem(HANDLE hProcess, bool bKeepHandle = false);
//

//! 取得硬盘信息
/*!
*\param  pszDiskBuffer 硬盘信息保存空间
*\param  nBufferLen 硬盘信息保存空间长度
*/
_BASIC_DLL_API_C  DWORD _BASIC_DLL_API BasicGetDiskInfo(char* pszDiskBuffer, int nBufferLen);
//

//! 取路径所在磁盘的剩余空间
/*!
*\param lpszPath 指定路径
*\return 返回剩余磁盘空间，单位M
*/
_BASIC_DLL_API_C  long _BASIC_DLL_API BasicGetDiskFreeinfo(const char* lpszPath);


//! 取得系统启动时间 
/*!
*\return 返回时间值，单位：毫秒
*\remark 用于测量耗时
*/
_BASIC_DLL_API_C  DWORD _BASIC_DLL_API BasicGetTickTime();
//! 取得系统启动时间，超过DWORD
_BASIC_DLL_API_C  double _BASIC_DLL_API BasicGetTickTimeCount();

//! 取得模块名，包括全路径
/*!
*\param hModule  如hModule==NULL，则取当前主程序名，
*\return 模块名
*/
CBasicString _BASIC_DLL_API BasicGetModuleName(HANDLE hModule = NULL);
CWBasicString _BASIC_DLL_API WBasicGetModuleName(HANDLE hModule);
//! 取得模块名，包括全路径
/*!
*\param hModule  如hModule==NULL，则取当前主程序名，
*\param pszBuffer 返回数据的空间
*\param nBufLen 返回数据的空间长度
*\return  返回实际长度
*/
long _BASIC_DLL_API BasicGetModuleName(HANDLE hModule, char* pszBuffer, int nBufLen);
//! 取得模块名，不包括全路径
/*!
*\param hModule  如hModule==NULL，则取当前主程序名，
*\param bExt 是否包括扩展名
*\return 模块名
*/
CBasicString _BASIC_DLL_API BasicGetModuleTitle(HANDLE hModule = NULL, bool bExt = false);
CWBasicString _BASIC_DLL_API WBasicGetModuleTitle(HANDLE hModule = NULL, bool bExt = false);

//! 取得模块路径
/*!
*\param hModule  如hModule==NULL，则取当前主程序路径
*\return 模块路径
*/
CBasicString _BASIC_DLL_API BasicGetModulePath(HANDLE hModule = NULL);
CWBasicString _BASIC_DLL_API WBasicGetModulePath(HANDLE hModule = NULL);

#define	BASIC_PSL_RET_ERROR				-1			// 出错，不确定状态
#define	BASIC_PSL_RET_NOT_EXIST			0			// 进程不存在
#define	BASIC_PSL_RET_STILL_LIVE		1			// 进程存在
#define	BASIC_PSL_RET_NO_RIGHT			2			// 进程在但是没有权限

//! 判断某个进程是否结束
/*!
*\param dwProcessID	进程ID
*/
_BASIC_DLL_API_C  bool _BASIC_DLL_API BasicProcessIsTerminated(DWORD dwProcessID);

//! 修改系统时间
/*!
* \param tTime:需要设置的时间，精确到秒
* \return 修改成功或者失败
*/
_BASIC_DLL_API_C  bool _BASIC_DLL_API BasicSetSysTime(time_t tTime);

#ifdef __BASICWINDOWS
//
//!进程信息结构
typedef struct   tagPROCESSLIST{
    DWORD	m_dwProcessID;				//!< 进程ID
    DWORD	m_dwParentProcessID;		//!< 父进程ID
    DWORD	m_dwThreadCnt;				//!< 线程数
    DWORD	m_dwModuleID;				//!< 模块ID
    TCHAR	m_szExeFile[MAX_PATH];		//!< 执行文件名(不带路径)
    TCHAR	m_szExePath[MAX_PATH];		//!< 文件所有目录
    tagPROCESSLIST*		m_pNext;		//!< 下个结点
                                        // 
    tagPROCESSLIST(){
        memset(this, 0, sizeof(*this));
    }
} PROCESSLIST;


//! 创建进程信息列表
/*!
*\return 进程信息链表
*/
PROCESSLIST* BasicCreateProcessEntry();

//! 释放进程信息列表
/*!
*\param pList 进程信息链表
*/
void BasicReleaseProcessEntry(PROCESSLIST* pList);
#endif
//////////////////////////////////////////////////////////////////////////
//!网络相关信息

#define MAX_IP						(int)16			//!< 支持的IP地址数
// !取IP地址和子网掩码结构
typedef struct   tagLocalAddr{
    char		m_szIP[MAX_IP];		//!< IP地址
    char		m_szMask[MAX_IP];	//!< 掩码
    tagLocalAddr(){
        memset(this, 0, sizeof(*this));
    }
} LOCALADDR, *PLOCALADDR;

//! 取本机IP地址和子网掩码
/*!
*\param pBuffer cbBuffer 输入IP地址信息空间
*\return 返回IP地址数量
*/
_BASIC_DLL_API int BasicGetLocalAddrInfo(PLOCALADDR pBuffer, int cbBuffer);

///////////////////////////////////////////////////////////////////////////////
//!动态库调用的函数

//! 加载动态库
/*!
*\param lpszLibFileName	动态库文件名
*\return 成功返回非零动态库句柄，失败返回NULL
*/
_BASIC_DLL_API void* BasicLoadLibrary(const char* lpszLibFileName);

//! 释放动态库
/*!
*\param hModule	动态库句柄
*\return 成功返回0，否则失败
*/
_BASIC_DLL_API long	BasicFreeLibrary(void* hModule);

//!取动态库函数入口
/*!
*\param hModule	动态库句柄
*\param lpszProcName 函数名
*\return 成功返回非零函数地址，失败返回NULL
*/
_BASIC_DLL_API void*	BasicGetProcAddress(void* hModule, const char* lpszProcName);
/*
* \brief 用户计算进程的CPU使用率
*/
class  _BASIC_DLL_API CProcessInfo{
public:
    CProcessInfo(DWORD nProcessId);
    virtual ~CProcessInfo();

    /*
    * \brief 获取进程的CPU占用率，返回值为百分比
    */
    int GetProcessCpu();
protected:
    DWORD m_nProcessId;
#ifdef __BASICWINDOWS
    LONGLONG m_nLastSystemTime;
    LONGLONG m_nLastTime;
    DWORD m_nCpuCount;
#else
    LONGLONG m_nLastUtime;
    LONGLONG m_nLastStime;
    LONGLONG m_nCutime;
    LONGLONG m_nCstime;
    FILE *fpidstat;
#endif
};

//! 取高精度的系统计数，从系统启动开始。单位：微秒（10-6 秒）。
/*!
*\return  如果成功返回 从系统启动到现在的微秒数。
*\remarks 这个函数需要硬件支持（HRT），主要用于性能记录中的时间记录
*/
_BASIC_DLL_API double BasicGetHighPerformanceCounter();

//! 获取取机器的特征码
/*!
*/
_BASIC_DLL_API bool BasicGetMachineSerial(CBasicString& str);


__NS_BASIC_END

#endif 


