//异常处理 Linux
//
#include "../inc/basic.h"
#ifndef __BASICWINDOWS
#include "exception_linux.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __ANDROID
#include <execinfo.h>
#endif

#include <sys/resource.h>
#include <syslog.h>
#include <getopt.h>

#if !defined(__MAC) && !defined(__ANDROID)
#include <bfd.h>
//#include <libiberty.h>
#endif

#include <sys/stat.h>

#include <fstream>
#include <iostream>

//////////////////////////////////////////////////////////////////////
using std::ofstream;
using std::flush;

__NS_BASIC_START

CBasicExceptionLinux *g_pExceptionHandler = NULL;
int	g_nStartMode = 0;
GlobalShutdownFunc g_funcShutdown = NULL;
//
// 设置异常保护模式
//
void BASIC_SetExceptionMode(int nMode, int nInstance)
{
	g_nStartMode = nMode;

	if(nMode&BASIC_EXCEPTION_DISABLE)
	{
		//禁用异常保护
		return;
	}
	if(g_pExceptionHandler == NULL)
	{
		g_pExceptionHandler = new CBasicExceptionLinux;
	}
	//
	if(nMode&BASIC_EXCEPTION_NORESTART)
	{
		//不重启
	}
	//
	if(nMode&BASIC_EXCEPTION_NOLOG)
	{
		//不记录日志
	}
	//
	if(nMode&BASIC_DAEMONMODE)
	{
		g_pExceptionHandler->SetDaemonMode(nInstance);
	}
}

//
// 应用程序关闭前调用，忽略退出时出现的异常
//
void BasicClearException()
{
	if(g_pExceptionHandler != NULL)
	{
		g_pExceptionHandler->BeforeQuit();
		delete g_pExceptionHandler;
		g_pExceptionHandler = NULL;
	}
}

//
//重启进程
//
void BASIC_Restart()
{
	if(g_pExceptionHandler != NULL)
		g_pExceptionHandler->Restart();
}
///////////////////////////////////////////////////////////////////////
//取得程序运行的参数
#define PS_CMDLINE_BEGIN 6
//!!注意返回值需要用BASIC_FreePidCmdline释放
char **BASIC_GetPidCmdline(int nPid, const char *pszModule, int &nNum)
{
	nNum = 0;
	char szSystem[1024] = {'\0'};
	char **pszRet = NULL;
	sprintf(szSystem, "ps p %d --no-heading|awk '{print NF-%d+1;for (i=%d;i<=NF;i++) {print $i}}'", nPid, PS_CMDLINE_BEGIN, PS_CMDLINE_BEGIN);
	FILE *pfile = popen(szSystem, "r");
	char szValue[1024] = {'\0'};
	int nLineNum = 0;
	
	if (pfile)
	{	
		bool bGetLineNum = false;
		int i = 0;
		while(fgets(szValue, sizeof(szValue), pfile))
		{
			if (strlen(szValue) > 0  && i < nLineNum + 1 && bGetLineNum)
			{
				char *p =NULL;
				if ((p = strchr(szValue, '\n')) != NULL)
					*p = '\0';
				pszRet[i] = new char[strlen(szValue) + 1];
				strcpy(pszRet[i], szValue);				
			}				
			if (!bGetLineNum)
			{
				nLineNum = atoi(szValue);
				pszRet = new char*[nLineNum + 2];
				pszRet[0] = new char[strlen(pszModule) + 1];
				strcpy(pszRet[0], pszModule);
				bGetLineNum = true;
			}
			i++;	
		}
		pclose(pfile);
	}
	if (pszRet != NULL)
	{
		pszRet[nLineNum + 1] = NULL;
		nNum = nLineNum + 2;
	}
	return pszRet;
}

//!释放程序参数的内存，配合BASIC_GetPidCmdline使用
void BASIC_FreePidCmdline(char **pszRet, int nNum)
{
	int i = 0;
	for (; i < nNum; i++)
	{
		if (pszRet[i])
		{
			delete [] pszRet[i];
			pszRet[i] = NULL;
		}
	}
}

//设置可执行属性
bool SetExecuteAttrib(LPCTSTR szModule)
{
	struct stat buf;
	memset(&buf, 0, sizeof(buf));
	if (stat(szModule, &buf) != 0)
	{
		return false;
	}
	//每三位表示一个属性
	int nMode = buf.st_mode;
	if((nMode & (0100)) == 0)
	{
		nMode |= 0100;
	}
	if((nMode & (010)) == 0)
	{
		nMode |= 010;
	}
	if((nMode & (01)) == 0)
	{
		nMode |= 01;
	}
	chmod(szModule, nMode);
	return true;
}

//检查pid是否存在
int CheckPidIsExist(LPCTSTR lpszFile)
{
	int nRet = -1;
	FILE *fp = fopen(lpszFile, "rt+");
	if(fp)
	{
		char szBuf[32];
		memset(szBuf,0,32);
		int nReadNum = fread(szBuf,32,1,fp);
		if(nReadNum < 32)
		{
			int nReadPid = atoi(szBuf);
			if(kill(nReadPid,0) == 0)
			{
				//进程存在
				nRet = 1;
			}
			else
			{
				fprintf(stderr, "remove invalid lock file : [%d] %s \n",  nReadPid, lpszFile);
				syslog(LOG_CRIT, "remove invalid lock file : [%d] %s \n",  nReadPid, lpszFile);
				//进程不存在
				nRet = 0;
			}
			
		}
		//
		fclose(fp);
		fp = NULL;
		if(nRet == 0)
		{
			unlink(lpszFile);	
		}
	}
	return nRet;
}

///////////////////////////////////////////////////////////////////////////
static pid_t core_dump(int signo)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) 
	{
#ifdef _FORCE_CORE_DUMP
#ifndef _CORE_SIZE
#define _CORE_SIZE (256 * 1024 * 1024)
#endif /* _CORE_SIZE */
		struct rlimit limit = {
			rlim_cur = _CORE_SIZE,
			rlim_max = _CORE_SIZE };

		setrlimit(RLIMIT_CORE, &limit);
#endif /* _FORCE_CORE_DUMP */
                /* reset the signal handler to default handler,
                 * then raise the corresponding signal. */
		signal(signo, SIG_DFL);
		TRACE(_T("raise %d\n"),signo);
		raise(signo);
	}


	return pid;
}

static int _core_dump_signals[] = {
        SIGABRT, SIGFPE, SIGILL, SIGSEGV,
        SIGTRAP, SIGSYS, SIGBUS,
#ifdef SIGPOLL
	SIGPOLL,
#endif
	 SIGPROF, SIGXCPU, SIGXFSZ,
#ifdef SIGEMT
        SIGEMT
#endif
};

//要捕捉的signal
//不能捕捉到SIGKILL SIGSTOP,用kill -9 结束的进程必须手工删除/var/run/xxx.pid
static int _quit_signals[] = {
        SIGHUP, SIGINT, SIGQUIT, SIGTERM,
};


//忽略的信号
static int _ignore_signals[] = {
        SIGPIPE,	//tcp通讯需忽略SIGPIPE信号
        SIGTTOU,	//防止终端干扰
        SIGTTIN,	//防止终端干扰
        SIGTSTP,	//防止终端干扰
};
///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

void IgnoreSig(int nSig)
{
	//TRACE("Signal %d \n", nSig);
}

void BackTraceDetail()
{
	int nPID = Basic_GetCurrentProcessId();
	int nThreadID = BasicGetCurrentThreadId();
//2 use gdb for detail
	char szFile[MAX_PATH];
	memset(szFile, 0, MAX_PATH);
	if(BasicGetModuleName(NULL, szFile, MAX_PATH) <= 0)
	{
		return ;
	}

	char szTitle[MAX_PATH];
	memset(szTitle, 0, MAX_PATH);
	__tcscpyn(szTitle, MAX_PATH, szFile, MAX_PATH);
	char* pSplit = strrchr(szFile, PATHSPLIT_S);
	if(pSplit != NULL)
	{
		__tcscpyn(szTitle, MAX_PATH, pSplit+1, MAX_PATH-(pSplit+1-szFile));
	}

	char szDbgOut[MAX_PATH];
	memset(szDbgOut, 0, MAX_PATH);
	char szScript[MAX_PATH];
	memset(szScript, 0, MAX_PATH);
	snprintf(szDbgOut, MAX_PATH, "/tmp/%s.%d.%d", szTitle, nThreadID,nPID);
	snprintf(szScript, MAX_PATH, "/tmp/%s.script.%d.%d", szTitle, nThreadID, nPID);
	ofstream scriptFile(szScript, std::ios::out);
	scriptFile << "#!/bin/sh\n"
		<< "# debug thread: " << nThreadID << " \n"
		<< "gdb " << szFile << " " << nPID << " << EOF > "
		<<  szDbgOut << " 2>&1\n"
		<< "thread apply all bt full\n"
		<< "bt\ndetach\nquit\nEOF\n"
		<< flush
		;
	char szExe[MAX_PATH];
	memset(szExe, 0, MAX_PATH);
	snprintf(szExe, MAX_PATH, "/bin/sh %s", szScript);
	system(szExe);

//3 backtrace
#if !defined(__MAC) && !defined(__ANDROID)
	snprintf(szDbgOut, MAX_PATH, "/tmp/%s.%d.%d.log", szTitle, nThreadID,nPID);
	FILE* fEx = fopen(szDbgOut, "wt");
	if(fEx != NULL)
	{
		fprintf(fEx, "backtrace:%d/%d\n", nPID, nThreadID);

		stacktrace::call_stack stack;
		fprintf(fEx, stack.to_string().c_str());

		fclose(fEx);
	}
#endif //__MAC
//
}

void DumpStack(int nSig)
{
	static basiclib::CMutex g_dumpMutex;
	basiclib::CSingleLock lock(&g_dumpMutex, true);
	if(g_nStartMode == BIT_POS_DUMP)
	{
		return;
	}
	g_nStartMode = BIT_POS_DUMP;
	lock.Unlock();
	//syslog(LOG_CRIT, "Catch %d\n", nSig);
	signal(nSig, IgnoreSig);
	//
	//core_dump(nSig);
	//
	try
	{
		BackTraceDetail();
	}
	catch(...)
	{
		syslog(LOG_CRIT, "log detail fail \n");
	}
	//
	//restart if necessary
	if(g_pExceptionHandler != NULL)
		g_pExceptionHandler->Restart();
	//
	exit(-1);
}

void Quit(int nSig)
{
	syslog(LOG_CRIT, "Quit %d . \n", nSig);
	//
	if(g_funcShutdown)
	{
		g_funcShutdown();
	}
	//
	//do something before quit
	if(g_pExceptionHandler != NULL)
	{
		g_pExceptionHandler->BeforeQuit();
	}
	//
	exit(-nSig);
}


////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
CBasicExceptionLinux::CBasicExceptionLinux()
{
	m_pszLockFile[0] = '\0';
	m_pAllocStack = NULL;
	//捕捉信号
	CatchSignal();

	//获取命令行参数
	m_nParaNum = 0;
	m_pszCmdline = NULL;

}

CBasicExceptionLinux::~CBasicExceptionLinux()
{
	if(m_pAllocStack)
	{
		BasicDeallocate(m_pAllocStack);
		m_pAllocStack = NULL;
	}
	//释放参数指针的内存
	BASIC_FreePidCmdline(m_pszCmdline, m_nParaNum);
}

void CBasicExceptionLinux::BeforeQuit()
{
	struct rlimit limit;
	// close all the file descriptors except of stdin/stdout/stderr.
	getrlimit(RLIMIT_NOFILE, &limit);
	int i;
	for (i = 3; i < limit.rlim_cur; i ++)
	{
		close(i);
	}

	if(m_pszLockFile[0] != '\0')
	{
		int r = unlink(m_pszLockFile);
		if(r != 0)
			syslog(LOG_CRIT, "unlink (%s) [%d]\n", m_pszLockFile, r);
	}
	//syslog(LOG_CRIT, "exit (%s) \n", m_pszLockFile);
	//关闭系统日志
	closelog();
}

void CBasicExceptionLinux::Restart()
{
	//不能用CString
	char szModule[MAX_PATH];
	BasicGetModuleName(NULL, szModule, MAX_PATH);

	//如果文件被删除或替换，那么文件名后面会有" (deleted)"字符串  删除掉！
	const char* pDelExt = " (deleted)";
	char* pDel = strstr(szModule, pDelExt);
	int nExe = strlen(szModule);
	if (pDel != NULL && pDel + strlen(pDelExt) == szModule + nExe)
	{
		*pDel = '\0'; 
	}
	//上传完文件后,可能丢失文件可执行属性
	SetExecuteAttrib(szModule);

	syslog(LOG_CRIT, "Restart (%s)...\n", szModule);
	BeforeQuit();
	
	
	int nRet = execvp((LPCTSTR)szModule, (char* const*)m_pszCmdline);


	exit(-1);
}

//设置后台运行模式
void CBasicExceptionLinux::SetDaemonMode(int nInstance)
{
#ifndef __ANDROID
	//后台运行
	FILE *fp = NULL;
	pid_t pid = 0;

	char szModule[MAX_PATH];
	BasicGetModuleName(NULL, szModule, MAX_PATH);
	char* pFullPath = szModule;

	char* pModule = strrchr(pFullPath, PATHSPLIT);
	pModule = pModule==NULL ? (char*)pFullPath : pModule+1;


	//启用日志，记录在/var/log/messages
	openlog(pModule, LOG_CONS|LOG_PID, LOG_USER);

	m_nParaNum = 0;
	m_pszCmdline = BASIC_GetPidCmdline(getpid(), (const char*)szModule, m_nParaNum);

	if(nInstance > 0)
	{
		sprintf(m_pszLockFile, "/var/run/%s%d.pid", pModule, nInstance);
	}
	else
	{
		sprintf(m_pszLockFile, "/var/run/%s.pid", pModule);
	}

	int nTry = 0;
	while(1)
	{
		if (access(m_pszLockFile,R_OK)==0)
		{
			if(++nTry < 3)
			{
				BasicSleep(1000);
				//检查pid有效
				CheckPidIsExist(m_pszLockFile);
				continue;
			}

			fprintf(stderr, "Existing a copy of this daemon : %s try[%d]!\n", m_pszLockFile, nTry);
			fprintf(stderr, "remove previous process : killall %s\n", pModule);
			fprintf(stderr, "remove lock file : unlink %s\n", m_pszLockFile);

			syslog(LOG_CRIT, "Existing a copy of this daemon : %s try[%d]!\n", m_pszLockFile, nTry);
			closelog();

			exit(1);
		}
		break;
	}

	//关闭输入输出
	int fdtablesize = getdtablesize();
	for(int fd=0; fd<fdtablesize; fd++)
		close(fd);

	//进入后台运行
	pid = fork();

	if (pid > 0)
	{
		TRACE(_T("daemon on duty!\n"));

		fp = fopen(m_pszLockFile,"wt");
		if(fp)
		{
			fprintf(fp,"%d", pid);
			fclose(fp);
		}
		exit(0);
	}
	else if (pid<0)
	{
		TRACE(_T("Can't fork!\n"));
		exit(-1);
	}
	//syslog(LOG_CRIT, "Create daemon : %s \n", m_pszLockFile);
#endif
}


//捕捉信号
void CBasicExceptionLinux::CatchSignal()
{
	//最初设置异常堆栈
	if(!m_pAllocStack)
	{
		m_pAllocStack = BasicAllocate(ALLOCTO_STACK_MEMORY);
		if(m_pAllocStack)
		{
			stack_t x;
			x.ss_sp = m_pAllocStack;
			x.ss_size = ALLOCTO_STACK_MEMORY;
			x.ss_flags = SS_ONSTACK;
			if(sigaltstack(&x, NULL) < 0)
			{
				syslog(LOG_CRIT, "call sigaltstack error \n");
				exit(-1);	
			}
		}
	}
	int i;
	for (i = 0; i < ARRAY_SIZE(_ignore_signals); i ++)
	{
		signal(_ignore_signals[i], IgnoreSig);
	}
	//
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = DumpStack;
	act.sa_flags = SA_ONSTACK;
	//act.sa_flags = SA_SIGINFO;
	sigfillset(&act.sa_mask);
	for (i = 0; i < ARRAY_SIZE(_core_dump_signals); i ++)
	{
		if(sigaction(_core_dump_signals[i], &act, NULL) < 0)
		{
			TRACE(_T("install signal error\n"));
		}
	}
	//
	struct sigaction actquit;
	actquit.sa_handler = Quit;
	actquit.sa_flags = SS_ONSTACK;
	sigemptyset(&actquit.sa_mask);
	for (i = 0; i < ARRAY_SIZE(_quit_signals); i ++)
	{
		if(sigaction(_quit_signals[i], &actquit, NULL) < 0)
		{
			TRACE(_T("install quit signal error\n"));
		}
	}
	// unblock all the signals, because if the current process is
	// spawned in the previous signal handler, all the signals are
	// blocked. In order to make it sense of signals, we should
	// unblock them. Certainly, you should call this function as
	// early as possible. :)
	sigprocmask(SIG_UNBLOCK, &act.sa_mask, NULL);

}


__NS_BASIC_END
#endif

