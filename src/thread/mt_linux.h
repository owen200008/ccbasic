#ifndef _MT_LINUX_H
#define _MT_LINUX_H

//////////////////////////////////////////////////////////////////////////////////////////////////////
//__NS_BASIC_START

#ifndef LPSECURITY_ATTRIBUTES
#define LPSECURITY_ATTRIBUTES void*
#endif //LPSECURITY_ATTRIBUTES

#define RTL_CRITSECT_TYPE 0
#define RTL_RESOURCE_TYPE 1


#ifndef WAIT_OBJECT_0
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)
#define WAIT_OBJECT_0                    ((STATUS_WAIT_0 ) + 0 )
#endif
#define WAIT_FAILED                     (DWORD)0xFFFFFFFF
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)
#define WAIT_TIMEOUT                        STATUS_TIMEOUT


#ifndef MAXIMUM_WAIT_OBJECTS
#define MAXIMUM_WAIT_OBJECTS 64     // Maximum number of wait objects
#endif

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (HANDLE)-1
#endif


DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE *lpHandles, bool bWaitAll, DWORD dwMilliseconds);
DWORD MsgWaitForMultipleObjects(DWORD nCount, LPHANDLE pHandles, bool fWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask);

HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
bool ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);

HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
    bool bInitialOwner, LPCSTR lpName);
bool ReleaseMutex(HANDLE hMutex);
bool LockMutex(HANDLE hMutex, DWORD dwTimeout);
bool DestoryMutex(HANDLE hMutex);

HANDLE CreateEvent(bool bManualReset, bool bInitialState, LPCSTR lpName);
bool DestoryEvent(HANDLE hEvent);
bool SetEvent(HANDLE hEvent);
bool ResetEvent(HANDLE hEvent);
bool PulseEvent(HANDLE hEvent);

bool CloseHandle(HANDLE hObject);

VOID InitializeCriticalSection(basiclib::LPCRITICAL_SECTION lpCriticalSection );
VOID EnterCriticalSection(basiclib::LPCRITICAL_SECTION lpCriticalSection);
bool TryEnterCriticalSection(basiclib::LPCRITICAL_SECTION lpCriticalSection);
VOID LeaveCriticalSection(basiclib::LPCRITICAL_SECTION lpCriticalSection);
VOID DeleteCriticalSection(basiclib::LPCRITICAL_SECTION lpCriticalSection);

//__NS_BASIC_END

#endif
