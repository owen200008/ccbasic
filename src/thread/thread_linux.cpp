//linux线程定义
// 编译选项 -lpthread

#include "../inc/basic.h"
#if	(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

//睡眠
//参数单位：毫秒
void BasicSleep(DWORD dwMilliseconds)
{
	//usleep(dwMilliseconds * 1000);
	//改用nanosleep
	timespec tmDelay;
	tmDelay.tv_sec = dwMilliseconds/1000;	//秒
	tmDelay.tv_nsec = (dwMilliseconds%1000)*1000000;
	nanosleep(&tmDelay, NULL);
}

//创建线程
HANDLE BasicCreateThread(
    LPBASIC_THREAD_START_ROUTINE lpStartAddress,
    void* lpParameter,
    LPDWORD lpThreadId
	)
{
	pthread_t a;
	pthread_attr_t *pattr = NULL;

/* 修改栈大小
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	size_t stack_size = 0;
	int nRet = pthread_attr_getstacksize(&attr, &stack_size);
	if(nRet == 0)
	{
		TRACE("default stack size %u, min %u \n", stack_size, PTHREAD_STACK_MIN);
		size_t newsize = MAX(PTHREAD_STACK_MIN*10, stack_size + 1024*1024);
		nRet = pthread_attr_setstacksize(&attr, newsize);
		if(nRet == 0)
		{
			TRACE("set thread size : %d\n", newsize);
			pattr = &attr;
		}
	}
#endif
*/

	int nCreate = pthread_create(&a, pattr, lpStartAddress, (void*)lpParameter);
	if(nCreate != 0)
	{
		TRACE("create thread fail [%d]!\n", nCreate);
	}

	if(lpThreadId != NULL)
		*lpThreadId = (DWORD)a;
	//TLTrace("BASIC_CreateThread:%d\n", (DWORD)a);
	return (HANDLE)a;
}


//等待线程退出
BOOL BasicWaitThread(
	HANDLE hThread,
	DWORD  dwWaitTime
	)
{
	if(hThread == NULL)
		return FALSE;
	pthread_join((pthread_t)hThread, NULL);
	return TRUE;
}

void BasicTerminateThread(HANDLE hThread)
{
}

#if (defined(__LINUX)  || defined(__ANDROID))
#define gettid() syscall(__NR_gettid)

#define _syscall0(type,name)\
		type name(void)\
		{\
			long __res;\
			__asm__volatile("int $0x80"\
			:"=a"(__res)\
			:"0"(__NR_##name));\
			__syscall_return(type,__res);\
		}

DWORD BasicGetCurrentThreadId()
{
	return (DWORD)gettid();
}
#else
DWORD BasicGetCurrentThreadId()
{
        return (DWORD)pthread_self();
}
#endif

HANDLE Basic_GetCurrentThread()
{
	return (HANDLE)pthread_self();
}

DWORD Basic_GetCurrentProcessId()
{
	return (DWORD)getpid();
}


////////////////////////////////////////////////////////////////////////
void* CBasic_Thread::Wrapper(void *ptr)
{
	CBasic_Thread *p = static_cast<CBasic_Thread *>(ptr);
	p->Run();
	return 0;
}


void CBasic_Thread::Start()
{
      pthread_create(&hnd_, 0, &CBasic_Thread::Wrapper,
                     static_cast<void *>(this));
}

void CBasic_Thread::Join()
{
      pthread_join(hnd_, 0);
}



__NS_BASIC_END

#endif //__LINUX
