#include "../inc/basic.h"

#ifdef __BASICWINDOWS

__NS_BASIC_START

#ifndef WINBASEAPI
#define WINBASEAPI
#endif
#ifndef WINAPI
#define WINAPI
#endif

WINBASEAPI
VOID
WINAPI
Sleep(
	  IN DWORD dwMilliseconds
	  );

/*
WINBASEAPI
__out
HANDLE
WINAPI
CreateThread(
			 __in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
			 __in      SIZE_T dwStackSize,
			 __in      LPTHREAD_START_ROUTINE lpStartAddress,
			 __in_opt  LPVOID lpParameter,
			 __in      DWORD dwCreationFlags,
			 __out_opt LPDWORD lpThreadId
			 );

*/

WINBASEAPI
DWORD
WINAPI
GetCurrentThreadId(
				   VOID
				   );

WINBASEAPI
DWORD
WINAPI
GetCurrentProcessId(
					VOID
					);

//////////////////////////////////////////////////////////////////////////////////////////
void BasicSleep(DWORD dwMilliseconds)
{
	::Sleep(dwMilliseconds);
}


//创建线程
HANDLE BasicCreateThread(
    LPBASIC_THREAD_START_ROUTINE lpStartAddress,
    void* lpParameter,
    LPDWORD lpThreadId
    )
{
	return ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)lpStartAddress, (void*)lpParameter, 0, lpThreadId);
}

//等待线程退出
BOOL BasicWaitThread(
				   HANDLE hThread,
				   DWORD  dwWaitTime/* = INFINITE*/
				   )
{
	if(hThread == NULL)
		return TRUE;
	return ::WaitForSingleObject(hThread, dwWaitTime) == WAIT_OBJECT_0;
}

void BasicTerminateThread(HANDLE hThread)
{
	if(hThread == NULL)
		return;
	::TerminateThread(hThread, 0);
}

DWORD BasicGetCurrentThreadId()
{
	return (DWORD)::GetCurrentThreadId();
}

HANDLE Basic_GetCurrentThread()
{
	return ::GetCurrentThread();
}

DWORD Basic_GetCurrentProcessId()
{
	return (DWORD)::GetCurrentProcessId();
}


////////////////////////////////////////////////////////////////////////
THREAD_RETURN CBasic_Thread::Wrapper(void *ptr)
{
	CBasic_Thread *p = static_cast<CBasic_Thread *>(ptr);
	p->Run();
	return 0;
}


void CBasic_Thread::Start()
{
	DWORD id;
	hnd_ = BasicCreateThread(&CBasic_Thread::Wrapper, this, &id);
}

void CBasic_Thread::Join()
{
	WaitForSingleObject(hnd_, INFINITE);
	CloseHandle(hnd_);
}

/////////////////////////////////////////////////////////////////////////////////////////////
CBasicThreadTLS::CBasicThreadTLS()
{
	m_bCreate = false;
	m_key = TLS_OUT_OF_INDEXES;
}

CBasicThreadTLS::~CBasicThreadTLS()
{
	if (m_bCreate){
		TlsFree(m_key);
	}
}

bool CBasicThreadTLS::CreateTLS()
{
	m_key = TlsAlloc();
	m_bCreate = m_key != TLS_OUT_OF_INDEXES;
	return m_bCreate;
}

void* CBasicThreadTLS::GetValue()
{
	return TlsGetValue(m_key);
}
BOOL CBasicThreadTLS::SetValue(void* pValue)
{
	return TlsSetValue(m_key, pValue);
}

__NS_BASIC_END
#endif //__BASICWINDOWS


