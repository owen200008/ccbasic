#ifdef __LINUX
//
//取得系统信息系列函数
//
//
#include "../inc/basic.h"

#include <sys/sysinfo.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/vfs.h>
#include <dlfcn.h>

#include <sys/vfs.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mntent.h>
#include <malloc.h>
#include <getopt.h>
#define HAVE_DECL_BASENAME 1
#ifndef __ANDROID
#include <libiberty.h>
#endif
#include <limits.h>
#include <inttypes.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

//!获取是否有键盘消息
int BasicKBHit()
{
#ifdef __x86_64__
	return 0;
#else
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
#endif
}
//
//取得CPU个数
int BasicGetNumberOfCpu()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

int BasicGetCpuNumber()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}
//
//取得操作系统版本
int BasicGetOSystemV(CBasicString& strOSVer)
{
    struct utsname osbuf;
    uname(&osbuf);
    strOSVer.Format("%s %s", osbuf.sysname, osbuf.release);
	return 0;
}


#define CACHE_TWEAK_FACTOR 64
#define SMLBUFSIZ ( 256 + CACHE_TWEAK_FACTOR)
#define PIDBUFSIZ ( 512 + CACHE_TWEAK_FACTOR)
#define std_err printf

typedef unsigned long long TIC_t;
typedef          long long SIC_t;
typedef struct CPU_t {   
   TIC_t u, n, s, i, w, x, y, z; // as represented in /proc/stat   
   TIC_t u_sav, s_sav, n_sav, i_sav, w_sav, x_sav, y_sav, z_sav; // in the order of our display   
   unsigned id;  // the CPU ID number   
} CPU_t;

#define Cpu_tot  1
CPU_t cpus[Cpu_tot+1];

static CPU_t *cpus_refresh ()   
{   
   static FILE *fstat = NULL;   
   int i;   
   int num;   
   // enough for a /proc/stat CPU line (not the intr line)   
   char buf[SMLBUFSIZ];   
   
   /* by opening this file once, we'll avoid the hit on minor page faults  
      (sorry Linux, but you'll have to close it for us) */   
   if (!fstat) {   
      if (!(fstat = fopen("/proc/stat", "r")))   
         std_err("Failed /proc/stat open ");   
      /* note: we allocate one more CPU_t than Cpu_tot so that the last slot  
               can hold tics representing the /proc/stat cpu summary (the first  
               line read) -- that slot supports our View_CPUSUM toggle */   
      //cpus = alloc_c((1 + Cpu_tot) * sizeof(CPU_t));   
      cpus[Cpu_tot].u_sav = 0;
   }   
   rewind(fstat);   
   fflush(fstat);   
   
   // first value the last slot with the cpu summary line   
   if (!fgets(buf, sizeof(buf), fstat))
	std_err("failed /proc/stat read");   
   cpus[Cpu_tot].x = 0;  // FIXME: can't tell by kernel version number   
   cpus[Cpu_tot].y = 0;  // FIXME: can't tell by kernel version number   
   cpus[Cpu_tot].z = 0;  // FIXME: can't tell by kernel version number   
   num = sscanf(buf, "cpu  %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",   
      &cpus[Cpu_tot].u,   
      &cpus[Cpu_tot].n,   
      &cpus[Cpu_tot].s,   
      &cpus[Cpu_tot].i,   
      &cpus[Cpu_tot].w,   
      &cpus[Cpu_tot].x,   
      &cpus[Cpu_tot].y,   
      &cpus[Cpu_tot].z   
   );   
   if (num < 4)   
         std_err("failed /proc/stat read");   
   
/*
   // and just in case we're 2.2.xx compiled without SMP support...   
   if (Cpu_tot == 1) {   
      cpus[1].id = 0;   
      memcpy(cpus, &cpus[1], sizeof(CPU_t));   
   }   
   
   // now value each separate cpu's tics   
   for (i = 0; 1 < Cpu_tot && i < Cpu_tot; i++) {   
      if (!fgets(buf, sizeof(buf), fstat))
	std_err("failed /proc/stat read");   
      cpus[i].x = 0;  // FIXME: can't tell by kernel version number   
      cpus[i].y = 0;  // FIXME: can't tell by kernel version number   
      cpus[i].z = 0;  // FIXME: can't tell by kernel version number   
      num = sscanf(buf, "cpu %u %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",   
         &cpus[i].id,   
         &cpus[i].u, &cpus[i].n, &cpus[i].s, &cpus[i].i, &cpus[i].w, &cpus[i].x, &cpus[i].y, &cpus[i].z   
      );   
      if (num < 4)   
            std_err("failed /proc/stat read");   
   }   
*/
   return cpus;   
} 

//
//取得CPU利用率,单位百分比
int BasicGetCPUUse()
{
	cpus_refresh ();
	CPU_t *cpu = &cpus[Cpu_tot];
	// we'll trim to zero if we get negative time ticks,   
	// which has happened with some SMP kernels (pre-2.4?)   
	#define TRIMz(x)  ((tz = (SIC_t)(x)) < 0 ? 0 : tz)   
	SIC_t u_frme, s_frme, n_frme, i_frme, w_frme, x_frme, y_frme, z_frme, tot_frme, tz;   
	float scale;   

	u_frme = cpu->u - cpu->u_sav;   
	s_frme = cpu->s - cpu->s_sav;   
	n_frme = cpu->n - cpu->n_sav;   
	i_frme = TRIMz(cpu->i - cpu->i_sav);   
	w_frme = cpu->w - cpu->w_sav;   
	x_frme = cpu->x - cpu->x_sav;   
	y_frme = cpu->y - cpu->y_sav;   
	z_frme = cpu->z - cpu->z_sav;   
	tot_frme = u_frme + s_frme + n_frme + i_frme + w_frme + x_frme + y_frme + z_frme;   
	if (tot_frme < 1)
		tot_frme = 1;   
	scale = 100.0 / (float)tot_frme;   

	int dUsage = cpu->u_sav>0 ? (int)(u_frme * scale * 100) : 0;

	//printf("CPU : %.3f, %d %d \n", dUsage, cpu->u, cpu->n);
   
   // display some kinda' cpu state percentages   
   // (who or what is explained by the passed prefix)   
/*
   show_special(   
      0,   
      fmtmk(   
         States_fmts,   
         pfx,   
         (float)u_frme * scale,   
         (float)s_frme * scale,   
         (float)n_frme * scale,   
         (float)i_frme * scale,   
         (float)w_frme * scale,   
         (float)x_frme * scale,   
         (float)y_frme * scale,   
         (float)z_frme * scale   
      )   
   );   
   Msg_row += 1;   
*/
   
   // remember for next time around   
   cpu->u_sav = cpu->u;   
   cpu->s_sav = cpu->s;   
   cpu->n_sav = cpu->n;   
   cpu->i_sav = cpu->i;   
   cpu->w_sav = cpu->w;   
   cpu->x_sav = cpu->x;   
   cpu->y_sav = cpu->y;   
   cpu->z_sav = cpu->z;   
   
#undef TRIMz   
	return dUsage;
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
	struct sysinfo info;
	if(sysinfo(&info) == -1)
	{
		return; 
	}
	dwPhysicalMemory = info.mem_unit * info.totalram/1024;
	dwUsedMemory = info.mem_unit * (info.totalram-(info.freeram+info.bufferram+info.sharedram)) / (1024);
	dwAvailMemory = info.mem_unit * (info.freeram+info.bufferram+info.sharedram) / (1024);
}


//
struct proc_t
{
        unsigned long size;
        unsigned long resident;
        unsigned long share;
        unsigned long trs;
        unsigned long lrs;
        unsigned long drs;
        unsigned long dt;
};

#define PAGE_SIZE 4096
//
//取得进程使用内存 单位K
DWORD BasicGetProcessMem(HANDLE hProcess, BOOL bKeepHandle)
{
	FILE* fstatm = NULL;
	static FILE* g_fstatm = NULL;
	char szStatFile[MAX_PATH];
	sprintf(szStatFile, "/proc/%d/statm", getpid());
	if(bKeepHandle)
	{
		if(g_fstatm == NULL)
		{
			g_fstatm = fopen(szStatFile, "r");
		}
		fstatm = g_fstatm;
	}
	else
	{
		fstatm = fopen(szStatFile, "r");
	}
	DWORD dwMem = 0;
	if(fstatm != NULL)
	{
		rewind(fstatm);
		fflush(fstatm);
		char buf[256];
		if (fgets(buf, sizeof(buf), fstatm))
		{
			proc_t P;
			int num = sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld",
				&P.size, &P.resident, &P.share, &P.trs, &P.lrs, &P.drs, &P.dt);
                        //PagefileUsage = (P.size * PAGE_SIZE);
                        //WorkingSetSize = (P.resident * PAGE_SIZE);
			dwMem = (MAX(P.size, P.resident) * PAGE_SIZE);
		}
		if(!bKeepHandle)
		{
			fclose(fstatm);
			fstatm = NULL;
		}
	}
	return dwMem/1024;
}
//

//
//取得硬盘信息
#ifndef UINTMAX_MAX
#define UINTMAX_MAX ((uintmax_t) -1)
#endif

#ifndef ME_DUMMY
#define ME_DUMMY(Fs_name,Fs_type) (!strcmp(Fs_type,"autofs")||!strcmp(Fs_type,"ignore"))
#endif

#undef STREQ
#define STREQ(a,b) (strcmp((a),(b)) == 0)

#ifndef ME_REMOTE
#define ME_REMOTE(Fs_name,Fs_type) (strchr((Fs_name),':') != 0 || ((Fs_name)[0] == '/' && (Fs_name)[1] == '/' && STREQ(Fs_type,"smbfs")))
#endif


#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

#define PROPAGATE_ALL_ONES(x) \
	((sizeof(x) < sizeof(uintmax_t) \
	&& (~ (x) == (sizeof(x) < sizeof(int) \
	? -(1L << (sizeof(x) * CHAR_BIT)) \
	: 0))) \
	? UINTMAX_MAX : (x))

#define EXTRACT_TOP_BIT(x) ((x) & ((uintmax_t)1 << (sizeof(x) * CHAR_BIT - 1)))
#define PROPAGATE_TOP_BIT(x) ((x) | ~ (EXTRACT_TOP_BIT(x) - 1))

//文件系统标示
struct mount_entry
{
	char *me_devname;
	char *me_mountdir;
	char *me_type;
	dev_t me_dev;
	unsigned int me_dummy : 1;
	unsigned int me_remote : 1;
	unsigned int me_type_malloced : 1;
	struct mount_entry *me_next;
};


//文件系统的信息
struct fs_usage
{
	int fsu_blocksize;
	uintmax_t fsu_blocks;
	uintmax_t fsu_bfree;
	uintmax_t fsu_bavail;
	int fsu_bavail_top_bit_set;
	uintmax_t fsu_files;
	uintmax_t fsu_ffree;

};

static struct mount_entry *g_mount_list = NULL;         //所有的文件系统

/*******************************************************
PROC:获取文件系统信息
输入：path路径
	  disk文件系统
	  fsp文件系统信息
输出：0表示成功 -1失败
*******************************************************/
int GetFSUsage(const char *path,const char *disk,struct fs_usage *fsp);

/*******************************************************
PROC:读取所有的文件系统
输出：当前系统文件系统的LIST
*******************************************************/
struct mount_entry* ReadFilesystemlist();

/*******************************************************
PROC:遍历显示文件系统
*******************************************************/
static CBasicString ShowAllEntries();

/*******************************************************
PROC:显示文件系统设备
*******************************************************/
static bool ShowDev(const char *disk, const char *mount_point,const char *fstype, int me_dummy, int me_remote,char *buf, int nBufLen);

static void FreeList(struct mount_entry *& mount_list);

static int xatoi(char *cp);

int GetFSUsage(const char *path,const char *disk,struct fs_usage *fsp)
{
	struct statfs fsd;
	if(statfs(path,&fsd) < 0)
	{
		return -1;
	}
	//  printf("blocksize:%d,T:%d,avail:%d,bfree:%d\n",fsd.f_bsize,fsd.f_blocks,fsd.f_bavail,fsd.f_bfree);
	fsp->fsu_blocksize = (fsd.f_frsize ? PROPAGATE_ALL_ONES(fsd.f_frsize) : PROPAGATE_ALL_ONES(fsd.f_bsize));
	fsp->fsu_blocks = PROPAGATE_ALL_ONES(fsd.f_blocks);
	fsp->fsu_bfree = PROPAGATE_ALL_ONES(fsd.f_bfree);
	fsp->fsu_bavail = PROPAGATE_TOP_BIT(fsd.f_bavail);
	fsp->fsu_bavail_top_bit_set = EXTRACT_TOP_BIT(fsd.f_bavail) != 0;
	fsp->fsu_files = PROPAGATE_ALL_ONES(fsd.f_files);
	fsp->fsu_ffree = PROPAGATE_ALL_ONES(fsd.f_ffree);

	return 0;
}

static bool ShowDev(const char *disk, const char *mount_point,const char *fstype, int me_dummy, int me_remote,
	char *buf, int nBufLen)
{
	struct fs_usage fsu;
	const char *stat_file;

	if(me_remote)
	{
		return false;
	}

	stat_file = mount_point ? mount_point : disk;
	if(GetFSUsage(stat_file,disk,&fsu) != 0)
	{
		return false;
	}

	if(fsu.fsu_blocks == 0)
	{
		return false;
	}

	int nLen = _stprintf_s(buf, nBufLen, "%s %d/%d| ", mount_point,
		int((fsu.fsu_bfree) * fsu.fsu_blocksize / 1024 /1024),
		int(fsu.fsu_blocks * fsu.fsu_blocksize / 1024 / 1024)
		);
	return true;
//	printf("devname: %s Total: %d used: %d \n",disk,total,used);
}

static int xatoi(char *cp)
{
	int val = 0;
	while(*cp)
	{
		if(*cp >= 'a' && *cp <= 'f')
		{
			val = val * 16 + *cp - 'a' + 10;
		}
		else if(*cp >= 'A' && *cp <= 'F')
		{
			val = val * 16 + *cp - 'A' + 10;
		}
		else if(*cp >= '0' && *cp <= '9')
		{
			val = val * 16 + *cp - '0';
		}
		else
		{
			break;
		}
		cp++;
	}
	return val;
}

#ifndef __ANDROID
struct mount_entry* ReadFilesystemlist()
{
	struct mount_entry *mount_list = NULL;
	struct mount_entry *me = NULL;
	struct mount_entry **mtail = &mount_list;

	//  #ifdef MOUNTED_GETMNTENT1
	{
		struct mntent *mnt;
		char *table = MOUNTED;
		FILE *fp;
		char *devopt;

		fp = setmntent(table,"r");
		if(fp == NULL)
		{
			return NULL;
		}

		while((mnt = getmntent(fp)))
		{
			me = (mount_entry *)BasicAllocate(sizeof *me);
			me->me_devname = BasicStrdup(mnt->mnt_fsname);
			me->me_mountdir = BasicStrdup(mnt->mnt_dir);
			me->me_type = BasicStrdup(mnt->mnt_type);
			me->me_type_malloced = 1;
			me->me_dummy = ME_DUMMY(me->me_devname,me->me_type);
			me->me_remote = ME_REMOTE(me->me_devname,me->me_type);
			devopt = strstr(mnt->mnt_opts,"dev=");
			if(devopt)
			{
				if(devopt[4] == '0' && (devopt[5] == 'x' || devopt[5] == 'X'))
				{
					me->me_dev = xatoi(devopt+6);
				}
				else
				{
					me->me_dev = xatoi(devopt+4);
				}
			}
			else
			{
				me->me_dev = (dev_t)-1;
			}
			*mtail = me;
			mtail = &me->me_next;
		}
		if(endmntent(fp) == 0)
		{
			goto free_then_fail;
		}
	}
	//  #endif

	*mtail = NULL;
	return mount_list;

free_then_fail:
	{
		*mtail = NULL;

		FreeList(mount_list);

		return NULL;
	}
}
#endif //__ANDROID

static void FreeList(struct mount_entry *& mount_list)
{
	struct mount_entry *me = NULL; 
	while(mount_list)
	{
		me = mount_list->me_next;
		BasicDeallocate(mount_list->me_devname);
		BasicDeallocate(mount_list->me_mountdir);
		if(mount_list->me_type_malloced)
		{
			BasicDeallocate(mount_list->me_type);
		}
		BasicDeallocate(mount_list); 
		mount_list = me;
	}	
}

static CBasicString ShowAllEntries()
{
	CBasicString strDiskInfo;
	TCHAR szTemp[1024];

	struct mount_entry *me;
	for(me = g_mount_list;me;me = me->me_next)
	{
		if(ShowDev(me->me_devname,me->me_mountdir,me->me_type,me->me_dummy,me->me_remote,szTemp, 1024))
		{
			strDiskInfo += szTemp;
		}
	}
	return strDiskInfo;
}

#ifndef __ANDROID
//获取所有磁盘空间信息
DWORD BasicGetDiskInfo(char* pszDiskBuffer, int nBufferLen)
{
	g_mount_list = ReadFilesystemlist();
	if(g_mount_list == NULL)
	{
		printf("List is NULL\n");
		return 1;
	}
	CBasicString strDiskInfo = ShowAllEntries();

	int nInfoLen = strDiskInfo.GetLength();
	__tcscpyn(pszDiskBuffer, nBufferLen, strDiskInfo.c_str());
	FreeList(g_mount_list);
	return nInfoLen;
}
#endif//__ANDROID
//

BOOL exec_command(char *pBuf,int nLen,char *format,...)
{
	if(nLen < 1024 || pBuf == NULL)
	{
		return FALSE;
	}
	BOOL bRet = FALSE;
	char szBuf[1024];

	va_list args;
	char cmd[512];
	va_start(args,format);
	vsprintf(cmd,format,args);
	va_end(args);
	
    	char *foo = cmd;
	if(FILE *inb = popen(foo,"r"))
	{
		if(!feof(inb))
		{
			int nReadLen = fread(szBuf,1,sizeof(szBuf),inb);
			if(nReadLen > 0 && nReadLen < sizeof(szBuf))
			{
				memcpy(pBuf,szBuf,nReadLen);
				pBuf[nReadLen] = '\0';
				bRet = TRUE;
			}
		}
	}	
	return bRet;
}

#ifndef __ANDROID
//取路径所在磁盘的剩余空间 M
long BasicGetDiskFreeinfo(const char* lpszPath)
{
	long lRet = -1;
	struct statfs64 s;
	if(statfs64(lpszPath, &s) < 0)
	{
		lRet = 0;
	}
	else
	{
		lRet = (s.f_bavail * s.f_bsize) / (1024 * 1024);
	}
	return lRet;
}
#endif //__ANDROID


//取得系统启动时间，单位：毫秒
DWORD BasicGetTickTime()
{
/*
	tms tm;
	return (DWORD)(((double)times(&tm))/CLOCKS_PER_SEC*1000);
*/
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec*1000 + tp.tv_usec/1000;
}

double BasicGetTickTimeCount()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    double dRet = tp.tv_sec*1000 + tp.tv_usec/1000;
    return dRet;
}

//取得模块名
//如hModule==NULL，则取当前主程序名
CBasicString BasicGetModuleName(HANDLE hModule)
{
	char exe[512];
	ssize_t len;

	len = BasicGetModuleName(hModule, exe, sizeof(exe) - 1);
	return exe;
}

long BasicGetModuleName(HANDLE hModule, char* pszBuffer, int nBufLen)
{
	ssize_t len;

	// find the file executable.
	len = readlink("/proc/self/exe", pszBuffer, nBufLen - 1);
	if (len == -1 || len == nBufLen - 1)
		return 0;
	pszBuffer[len] = '\0';

	return len;
}

//取得模块名，不包括全路径
CBasicString BasicGetModuleTitle(HANDLE hModule, BOOL bExt)
{
	CBasicString strModule = BasicGetModuleName(hModule);
	int nPos = strModule.ReverseFind(PATHSPLIT);
	if(nPos >= 0)
		strModule = strModule.Mid(nPos + 1);
	return strModule;
}


//取得路径
CBasicString BasicGetModulePath(HANDLE hModule)
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

//************************************************************************
// Author:    Mini.J @2009/3/13
// Method:    Basic_ProcessIsTerminated => 进程是否退出
//			查看进程列表中是否有传入的进程ID。如果存在说明未退出
// Returns:   BOOL => 
// Parameter: DWORD dwProcessID => 进程ID
//************************************************************************
BOOL BasicProcessIsTerminated(DWORD dwProcessID)
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
		return TRUE;
	}
	CBasicStringArray ayItem;
	BasicSpliteString(strFile.c_str(), '\n', IntoContainer_s<CBasicStringArray>(ayItem));
	int nLineCnt = ayItem.GetSize();
	if (nLineCnt == 2)
	{
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int BasicGetLocalAddrInfo(PLOCALADDR pBuffer, int cbBuffer)
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
		sprintf(ifr.ifr_name, "eth%d", i++);
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
			found = false;;
		}
		nIndex++;
	}
	close(sock);
	return nIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//动态库调用的函数
void* BasicLoadLibrary(const char* lpszLibFileName)
{
	void* hDll = dlopen(lpszLibFileName, RTLD_NOW|RTLD_GLOBAL);
    if(hDll == NULL)
    {
        const char* pError = dlerror();
        if(pError != NULL)
            basiclib::BasicLogEventErrorV("linking (%s) error occurred: (%s) \n", lpszLibFileName, pError);
    }
    return hDll;
}

long BasicFreeLibrary(void* hModule)
{
	return dlclose(hModule);
}

void* BasicGetProcAddress(void* hModule, LPCSTR lpszProcName)
{
    return dlsym(hModule, lpszProcName);
}

#ifndef __ANDROID
BOOL BasicSetSysTime(time_t tTime)
{
	time_t timeSet = tTime;
	return stime(&timeSet)==0 ? TRUE:FALSE;
}
#endif

CProcessInfo::CProcessInfo(DWORD nProcessId)
{
	m_nProcessId = nProcessId;
	m_nLastUtime = 0;
	m_nLastStime = 0;
	m_nCutime = 0;
	m_nCstime = 0;
	fpidstat = NULL;
}



//
//取得指定进程的CPU利用率，单位百分比
int CProcessInfo::GetProcessCpu()
{
	//先获取总的CPU的时间
	cpus_refresh ();
	CPU_t *cpu = &cpus[Cpu_tot];
	// we'll trim to zero if we get negative time ticks,   
	// which has happened with some SMP kernels (pre-2.4?)   
#define TRIMz(x)  ((tz = (SIC_t)(x)) < 0 ? 0 : tz)   
	SIC_t u_frme, s_frme, n_frme, i_frme, w_frme, x_frme, y_frme, z_frme, tot_frme, tz;   
	float scale;   

	u_frme = cpu->u - cpu->u_sav;   
	s_frme = cpu->s - cpu->s_sav;   
	n_frme = cpu->n - cpu->n_sav;   
	i_frme = TRIMz(cpu->i - cpu->i_sav);   
	w_frme = cpu->w - cpu->w_sav;   
	x_frme = cpu->x - cpu->x_sav;   
	y_frme = cpu->y - cpu->y_sav;   
	z_frme = cpu->z - cpu->z_sav;   
	tot_frme = u_frme + s_frme + n_frme + i_frme + w_frme + x_frme + y_frme + z_frme;   
	if (tot_frme < 1)
		tot_frme = 1;   

	cpu->u_sav = cpu->u;   
	cpu->s_sav = cpu->s;   
	cpu->n_sav = cpu->n;   
	cpu->i_sav = cpu->i;   
	cpu->w_sav = cpu->w;   
	cpu->x_sav = cpu->x;   
	cpu->y_sav = cpu->y;   
	cpu->z_sav = cpu->z; 

	//获取进程的CPU时间 
	// enough for a /proc/pid/stat line 
	char buf[PIDBUFSIZ];   

	/* by opening this file once, we'll avoid the hit on minor page faults  
	(sorry Linux, but you'll have to close it for us) */   
	if (!fpidstat) 
	{
		CBasicString strFileName;
		strFileName.Format("/proc/%d/stat", m_nProcessId);
		if (!(fpidstat = fopen(strFileName.c_str(), "r")))   
		{
			std_err("Failed /proc/%d/stat open ", m_nProcessId);
			return -1;
		}
	}   
	rewind(fpidstat);   
	fflush(fpidstat);   

	//将进程的stat文件读出来
	if (!fgets(buf, sizeof(buf), fpidstat))
	{
		std_err("failed /proc/%d/stat read", m_nProcessId);  
		return -1;
	}

	CBasicStringArray ayProcessData;
	BasicSpliteString(buf, ' ', IntoContainer_s<CBasicStringArray>(ayProcessData));
	if (ayProcessData.size() != 42)
	{
		return -1;
	}
	//获取文件中有用的值
	SIC_t nUtime = atof(ayProcessData[13].c_str());
	SIC_t nStime = atof(ayProcessData[14].c_str());
	SIC_t nCutime = atof(ayProcessData[15].c_str());
	SIC_t nCstime = atof(ayProcessData[16].c_str());
	SIC_t nSubValue = nUtime+nStime+nCutime+nCstime-m_nLastStime-m_nLastUtime-m_nCutime-m_nCstime;
	int nCpu = 100*((double)nSubValue)/((double)tot_frme);
	m_nLastUtime = nUtime;
	m_nLastStime = nStime;
	m_nCutime = nCutime;
	m_nCstime = nCstime;
	if (nCpu > 100)
	{
		nCpu = 0;
	}
	return nCpu;
}

CProcessInfo::~CProcessInfo()
{
	if (fpidstat)
	{
		fclose(fpidstat);
	}
	
}

double BasicGetHighPerformanceCounter()
{
        struct timespec tp;
        clock_gettime(CLOCK_MONOTONIC, &tp);

        return ((double)tp.tv_sec * 1000000 + (double)tp.tv_nsec / 1000);
}

__NS_BASIC_END


#endif //__LINUX
