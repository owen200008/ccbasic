#include "../inc/basic.h"
#ifndef __ANDROID
#include <sys/timeb.h>
#endif
#if defined(IPHONE_AVCH_32)
#include <libkern/OSAtomic.h>
#endif
#if	(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
#include <pthread.h>
#include <errno.h>
__NS_BASIC_START
////////////////////////////////////////////////////////////////
//����ͬһ�̼߳���
int pthread_mutex_init_np(pthread_mutex_t *pmutex)
{
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
#if (defined(__LINUX) || defined(__ANDROID))
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
#endif
    //pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP|PTHREAD_MUTEX_ERRORCHECK_NP);

    int nRet = pthread_mutex_init(pmutex, &mutexattr);

    pthread_mutexattr_destroy(&mutexattr);

    return nRet;
}

struct sem_private_struct
{
	pthread_mutex_t		m_mutex;
	pthread_cond_t		m_condition;
	int			m_semCount;	
	bool			m_manualReset;
	//
	sem_private_struct()
	{
		m_semCount = 0;
		m_manualReset = false;
	}
};
typedef sem_private_struct *sem_private;

//////////////////////////////////////////////////////////////////////////////
//32λԭ�Ӳ���
#ifndef _USER64BITREGISTER_LINUX
LONG
BasicInterlockedExchangeAdd(
    LONG volatile *Addend,
    LONG Increment
    );

LONG
BasicInterlockedIncrement(
    LONG volatile *lpAddend
    )
{
#ifdef __MAC
    return BasicInterlockedExchangeAdd( lpAddend, 1 );
#else
	return BasicInterlockedExchangeAdd( lpAddend, 1 ) + 1;
#endif
}

LONG
BasicInterlockedDecrement(
    LONG volatile *lpAddend
    )
{
#ifdef __MAC
    return BasicInterlockedExchangeAdd( lpAddend, -1 );
#else
	return BasicInterlockedExchangeAdd( lpAddend, -1 ) - 1;
#endif
}

LONG
BasicInterlockedExchange(
    LONG volatile *Target,
    LONG Value
    )
{
    return BasicInterlockedCompareExchange(Target, Value, *Target);
}

LONG
BasicInterlockedCompareExchange(LONG volatile *dest, long exch, long comp)
{
#ifdef __MAC
	LONG lRet = *dest;
    bool ret = OSAtomicCompareAndSwap32((int32_t)comp, (int32_t)exch, (volatile int32_t*)dest);
    return lRet;
#else
 	LONG lRet = *dest;
 	__sync_val_compare_and_swap(dest, comp, exch);
  	return lRet;
#endif
}

LONG
BasicInterlockedExchangeAdd(
    LONG volatile *Addend,
    LONG Increment
    )
{
#ifdef __MAC
    return OSAtomicAdd32((int32_t)Increment, (volatile int32_t*)Addend);
#else
	return __sync_fetch_and_add(Addend, Increment);
#endif
}

LONG BasicInterlockedExchangeSub(LONG volatile *Addend, LONG Value)
{
#ifdef __MAC
	return OSAtomicAdd32((int32_t)-Increment, (volatile int32_t*)Addend);
#else
	return __sync_fetch_and_add(Addend, -Increment);
#endif
}

#endif
//////////////////////////////////////////////////////////////////////////////
//typedef struct _RBASIC_CRITICAL_SECTION {
//    PRBASIC_CRITICAL_SECTION_DEBUG DebugInfo;
//    LONG LockCount;
//    LONG RecursionCount;
//    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
//    HANDLE LockSemaphore;
//    DWORD SpinCount;
//} RBASIC_CRITICAL_SECTION, *PRBASIC_CRITICAL_SECTION;

VOID
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
	if(lpCriticalSection == NULL)
		return;
	memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
	lpCriticalSection->LockSemaphore = (pthread_mutex_t*)new pthread_mutex_t;
	pthread_mutex_init_np((pthread_mutex_t*)lpCriticalSection->LockSemaphore);
}

VOID
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
	pthread_mutex_lock((pthread_mutex_t*)lpCriticalSection->LockSemaphore);
	//
	lpCriticalSection->OwningThread = BasicGetCurrentThreadId();
}

BOOL
TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
	int irc = pthread_mutex_trylock((pthread_mutex_t*)lpCriticalSection->LockSemaphore);
	if (!irc)  
	{
		/* we now own the mutex  */
		lpCriticalSection->OwningThread = BasicGetCurrentThreadId();
		return true;
	}
	return false;
}

VOID
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
	DWORD hPrev = lpCriticalSection->OwningThread;
	if(hPrev != 0 && hPrev != BasicGetCurrentThreadId())
	{
		BasicTrace("pthread_mutex_unlock fail owner:%d,this:%d \n", hPrev, BasicGetCurrentThreadId());
		return;
	}
	lpCriticalSection->OwningThread = 0;

	int nUnlock = pthread_mutex_unlock((pthread_mutex_t*)lpCriticalSection->LockSemaphore);
	if(nUnlock != 0 && hPrev != 0)
	{
		BasicTrace("pthread_mutex_unlock fail(%d) owner:%d \n", nUnlock, hPrev);
	}
}

VOID
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )
{
	if(lpCriticalSection == NULL)
		return;
	if(lpCriticalSection->OwningThread != 0)
	{
		BasicTrace("DeleteCritcalSection (%d) \n", lpCriticalSection->OwningThread);
		basiclib::LeaveCriticalSection(lpCriticalSection);
	}
	pthread_mutex_t* pmutex = (pthread_mutex_t*)lpCriticalSection->LockSemaphore;
	pthread_mutex_destroy(pmutex);
	delete pmutex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
BASIC_CloseHandle(
    HANDLE hObject
    )
{
	if(hObject == NULL || hObject == INVALID_HANDLE_VALUE)
		return FALSE;
	//TRACE("!!!Don't call me, CloseHandle not support!\n");
	return TRUE;
}

BOOL
ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    )
{
	return true;
}


HANDLE
CreateSemaphore(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
    )
{
	return 0;
}

HANDLE
CreateMutex(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
    )
{
	pthread_mutex_t* mutex = new pthread_mutex_t;
	pthread_mutex_init_np (mutex);
	return (HANDLE)mutex;
}

BOOL
ReleaseMutex(
    HANDLE hMutex
    )
{
	pthread_mutex_unlock((pthread_mutex_t*)hMutex);
	return true;
}

//��ʱ��
//ʱ�䵥λ������
BOOL
LockMutex(HANDLE hMutex, DWORD dwTimeout)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)hMutex;
	if(dwTimeout == INFINITE)
	{
		int ret = pthread_mutex_lock(mutex);
		if(ret == 0)
			return true;
		return false;
	}
	else
	{
		struct timespec delay;
		delay.tv_sec = 0;   //��

		DWORD timeoutCount = 0, t = 1;
		while (timeoutCount <= dwTimeout ) 
		{		
			int irc = pthread_mutex_trylock(mutex);
			if (!irc)  
			{
				/* we now own the mutex  */
				return true;
			}
			else if (irc == EBUSY) /* check whether somebody else has the mutex */
			{
				if(dwTimeout == 0)	//test only
				{
					return false;
				}
				/* sleep for delay time */
				if(t > 10)
				{
					t = 10;
				}
				delay.tv_nsec = t*1000000;	//����
				nanosleep(&delay, NULL);
				timeoutCount += t++;
			}
			else
			{
				/* error  */
				break;
			}
		}
	}
	return false;
}

BOOL
DestoryMutex(HANDLE hMutex)
{
	pthread_mutex_t* pmutex = (pthread_mutex_t*)hMutex;
	pthread_mutex_unlock(pmutex);
	pthread_mutex_destroy(pmutex);
	delete pmutex;
	return true;
}

HANDLE
BasicCreateEvent(
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    )
{
	sem_private token = (sem_private) BasicAllocate(sizeof(sem_private_struct));
	token->m_manualReset = bManualReset;
	int rc = 0;
	if(rc = pthread_mutex_init_np(&(token->m_mutex)))
	{
		BasicDeallocate(token);
		return 0;
	}
	if(rc = pthread_cond_init(&(token->m_condition), NULL))
	{
		pthread_mutex_destroy( &(token->m_mutex) );
		BasicDeallocate(token);
		return 0;
	}
	token->m_semCount = 0;
	return (HANDLE)token;
}

BOOL
BasicSetEvent(
    HANDLE hEvent
    )
{
	if(hEvent == NULL)
		return false;
	sem_private token = (sem_private)hEvent;
	if(token->m_manualReset)
		pthread_cond_broadcast(&(token->m_condition));
	else
		pthread_cond_signal(&(token->m_condition));
	return true;
}

BOOL
BasicResetEvent(
    HANDLE hEvent
    )
{
	sem_private token = (sem_private)hEvent;
	pthread_mutex_lock(&(token->m_mutex));
	token->m_semCount ++;
	pthread_mutex_unlock(&token->m_mutex);
	return true;
}

BOOL
BasicDestoryEvent(HANDLE hEvent)
{
	sem_private token = (sem_private)hEvent;
	pthread_mutex_destroy(&(token->m_mutex));
	pthread_cond_destroy(&(token->m_condition));
	BasicDeallocate (token); 
	return true;
}

BOOL
PulseEvent(
    HANDLE hEvent
    )
{
	return true;
}

DWORD
MsgWaitForMultipleObjects(
    DWORD nCount,
    LPHANDLE pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask)
{
	return 0;
}

DWORD
BasicWaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    )
{
	if(hHandle == NULL)
		return WAIT_FAILED;
	sem_private token = (sem_private)hHandle;
	pthread_mutex_t* mutex = &token->m_mutex;
	int rc;
	struct timespec tm;
	long sec, millisec;
	if (rc = pthread_mutex_lock(mutex))
		return WAIT_FAILED;
	
	if(dwMilliseconds == INFINITE)
	{
		//while (token->semCount <= 0)
		{
			rc = pthread_cond_wait(&(token->m_condition), mutex);
			//if (rc && (errno != EINTR) )
			//	break;
		}
	}
	else
	{
		

		int timelimit = dwMilliseconds;
		sec = timelimit / 1000;
		millisec = timelimit % 1000;

#ifdef __ANDROID
		clock_gettime(CLOCK_REALTIME, &tm);
		tm.tv_sec += sec;
		tm.tv_nsec += millisec * 1000000;

		if (tm.tv_nsec > 999 * 1000000)
		{
			tm.tv_nsec -= 1000 * 1000000;
			tm.tv_sec++;
		}
#else

		struct timeb tp;
		ftime(&tp);
		tp.time += sec;
		tp.millitm += millisec;
		if (tp.millitm > 999)
		{
			tp.millitm -= 1000;
			tp.time++;
		}
		tm.tv_sec = tp.time;
		tm.tv_nsec = tp.millitm * 1000000;
#endif
		
		//while (token->semCount <= 0)
		{
			rc = pthread_cond_timedwait(&(token->m_condition), mutex, &tm);
			//if (rc && (errno != EINTR) )
				//break;
		}
	}
	if ( rc ) 
	{
		if ( pthread_mutex_unlock(mutex) )
			return WAIT_FAILED;
		
		if ( rc == ETIMEDOUT) /* we have a time out */
			return WAIT_TIMEOUT ;
		
		return WAIT_FAILED ;
	} 
	token->m_semCount--;

	if (rc = pthread_mutex_unlock(mutex))
		return WAIT_FAILED;
	return WAIT_OBJECT_0;
}

BOOL BasicCloseHandle(HANDLE hObject)
{

}

__NS_BASIC_END
#endif //__LINUX
