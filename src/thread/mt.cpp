// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "../inc/basic.h"
#if (defined(__LINUX)|| defined(__MAC) || defined(__ANDROID))
#include <pthread.h>
#include "mt_linux.h"
#endif

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////
// Basic synchronization object
#ifndef UNUSED
#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif
#define UNUSED_ALWAYS(x) x
#endif

CBasicSyncObject::CBasicSyncObject(LPCTSTR pstrName)
{
    UNUSED(pstrName);   // unused in release builds

    m_hObject = NULL;

#ifdef _DEBUG
//    m_strName = pstrName;
#endif
}

CBasicSyncObject::~CBasicSyncObject()
{
    if (m_hObject != NULL)
    {
        ::CloseHandle(m_hObject);
        m_hObject = NULL;
    }
}

BOOL CBasicSyncObject::Lock(DWORD dwTimeout)
{
    if (BasicWaitForSingleObject(m_hObject, dwTimeout) == WAIT_OBJECT_0)
        return TRUE;
    else
        return FALSE;
}

#ifdef _DEBUG

void CBasicSyncObject::AssertValid() const
{
}

#endif


/////////////////////////////////////////////////////////////////////////////
// CSemaphore

CSemaphore::CSemaphore(LONG lInitialCount, LONG lMaxCount,
	LPCTSTR pstrName)
	:  CBasicSyncObject(pstrName)
{
	ASSERT(lMaxCount > 0);
	ASSERT(lInitialCount <= lMaxCount);

	m_hObject = ::CreateSemaphore(NULL, lInitialCount, lMaxCount,
		pstrName);
}

CSemaphore::~CSemaphore()
{
}

BOOL CSemaphore::Unlock(LONG lCount, LPLONG lpPrevCount /* =NULL */)
{
	return ::ReleaseSemaphore(m_hObject, lCount, lpPrevCount);
}

/////////////////////////////////////////////////////////////////////////////
// CMutex

CMutex::CMutex(BOOL bInitiallyOwn, LPCTSTR pstrName)
	: CBasicSyncObject(pstrName)
{
	m_hObject = ::CreateMutex(NULL, bInitiallyOwn, pstrName);
}

CMutex::~CMutex()
{
#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
	if (m_hObject != NULL)
	{
		::DestoryMutex(m_hObject);
		m_hObject = NULL;
	}
#endif
}

BOOL CMutex::Unlock()
{
	return ::ReleaseMutex(m_hObject);
}

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
BOOL CMutex::Lock(DWORD dwTimeout)
{
	return ::LockMutex(m_hObject, dwTimeout);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CEvent

CEvent::CEvent(BOOL bInitiallyOwn, BOOL bManualReset, LPCTSTR pstrName)
	: CBasicSyncObject(pstrName)
{
	m_hObject = BasicCreateEvent(bManualReset,
		bInitiallyOwn, pstrName);
}

CEvent::~CEvent()
{
	if (m_hObject != NULL)
	{
#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
		BasicDestoryEvent(m_hObject);
#else
		BasicCloseHandle(m_hObject);
#endif
		m_hObject = NULL;
	}
}

BOOL CEvent::Unlock()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSingleLock

CSingleLock::CSingleLock(CBasicSyncObject* pObject, BOOL bInitialLock)
{
	m_pObject = pObject;
	m_bAcquired = FALSE;
	if (bInitialLock)
		Lock();
}

BOOL CSingleLock::Lock(DWORD dwTimeOut /* = INFINITE */)
{
	ASSERT(!m_bAcquired);
	if (m_pObject)
		m_bAcquired = m_pObject->Lock(dwTimeOut);
	return m_bAcquired;
}

BOOL CSingleLock::Unlock()
{
	if (m_bAcquired && m_pObject)
		m_bAcquired = !m_pObject->Unlock();

	// successfully unlocking means it isn't acquired
	return !m_bAcquired;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//INLINE

CBasicSyncObject::operator HANDLE() const
    { return m_hObject;}

BOOL CSemaphore::Unlock()
    { return Unlock(1, NULL); }

BOOL CEvent::SetEvent()
    { ASSERT(m_hObject != NULL); return BasicSetEvent(m_hObject); }
BOOL CEvent::PulseEvent()
    { ASSERT(m_hObject != NULL); return ::PulseEvent(m_hObject); }
BOOL CEvent::ResetEvent()
    { ASSERT(m_hObject != NULL); return BasicResetEvent(m_hObject); }

CSingleLock::~CSingleLock()
    { Unlock(); }

/////////////////////////////////////////////////////////////////////////////
CCriticalSection::CCriticalSection() : CBasicSyncObject(NULL)
{
	::InitializeCriticalSection(&m_sect);
}

CCriticalSection::operator CRITICAL_SECTION*()
{
	return (CRITICAL_SECTION*) &m_sect;
}

CCriticalSection::~CCriticalSection()
{
	::DeleteCriticalSection(&m_sect);
}

BOOL CCriticalSection::Lock()
{
	::EnterCriticalSection(&m_sect);
	return TRUE;
}

/** 
*\brief CCriticalSection::Lock 加锁，支持超时
* 
*\param dwTimeout 超时时间，单位毫秒，-1不超时
*\return  
*/
BOOL CCriticalSection::Lock(DWORD dwTimeout)
{
	if(dwTimeout == (DWORD)-1)
		return Lock();
	DWORD dwCurrentSlice = 0;
	while(!::TryEnterCriticalSection(&m_sect))
	{
		if(dwCurrentSlice < dwTimeout)
		{
			BasicSleep(100);
			dwCurrentSlice += 100;
		}
		else
		{
			//超时
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CCriticalSection::Unlock()
{
	::LeaveCriticalSection(&m_sect);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
CSpinLockFunc::CSpinLockFunc(SpinLock* pLock, BOOL bInitialLock)
{
	m_pLock = pLock;
	m_bAcquired = false;
	if (bInitialLock)
		Lock();
}

CSpinLockFunc::~CSpinLockFunc()
{
	UnLock();
}

void CSpinLockFunc::Lock()
{
	if (!m_bAcquired)
	{
		DWORD dwThreadID = BasicGetCurrentThreadId();
		if (basiclib::BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 1))
		{
			if (dwThreadID == m_pLock->m_lockThreadID && IsLock())
				return;

			while (basiclib::BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 1)) {}
		}
		m_bAcquired = true;
		m_pLock->m_lockThreadID = dwThreadID;
	}
}
bool CSpinLockFunc::LockNoWait()
{
	if (!m_bAcquired)
	{
		DWORD dwThreadID = BasicGetCurrentThreadId();
		if (basiclib::BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 1))
		{
			if (dwThreadID == m_pLock->m_lockThreadID && IsLock())
				return true;
			return false;
		}
		m_bAcquired = true;
		m_pLock->m_lockThreadID = dwThreadID;
	}
	return true;
}

void CSpinLockFunc::LockAndSleep(unsigned short usSleep)
{
	if (!m_bAcquired)
	{
		DWORD dwThreadID = BasicGetCurrentThreadId();
		if (basiclib::BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 1))
		{
			if (dwThreadID == m_pLock->m_lockThreadID && IsLock())
				return;
			while (BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 1))
			{
				BasicSleep(usSleep);
			}
		}
		m_bAcquired = true;
		m_pLock->m_lockThreadID = BasicGetCurrentThreadId();
	}
}

void CSpinLockFunc::UnLock()
{
	if (m_bAcquired)
	{
		m_pLock->m_lockThreadID = 0;
		basiclib::BasicInterlockedExchange((LONG*)&(m_pLock->m_nLock), 0);
		m_bAcquired = false;
	}
}

bool CSpinLockFunc::IsLock()
{
	return basiclib::BasicInterlockedExchangeAdd((LONG*)&(m_pLock->m_nLock), 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __BASICWINDOWS

LONG BasicInterlockedIncrement (
							  LONG volatile *lpAddend
							  )
{
	return ::InterlockedIncrement(lpAddend);
}

LONG BasicInterlockedDecrement (
							  LONG volatile *lpAddend
							  )
{
	return ::InterlockedDecrement (lpAddend);
}

LONG BasicInterlockedExchange (
							 LONG volatile *Target,
							 LONG Value
							 )
{
	return ::InterlockedExchange(Target, Value);
}

LONG BasicInterlockedExchangeAdd (
								LONG volatile *Addend,
								LONG Value
								)
{
	return ::InterlockedExchangeAdd(Addend, Value);
}
LONG BasicInterlockedExchangeSub(LONG volatile *Addend, LONG Value)
{
	return ::InterlockedExchangeAdd(Addend, -Value);
}

LONG BasicInterlockedCompareExchange (
									LONG volatile *Destination,
									LONG Exchange,
									LONG Comperand
									)
{
	return ::InterlockedCompareExchange(Destination, Exchange, Comperand);
}

HANDLE BasicCreateEvent(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName)
{
	return ::CreateEvent(NULL, bManualReset, bInitialState, lpName);
}

BOOL BasicSetEvent(HANDLE hEvent)
{
	return ::SetEvent(hEvent);
}

BOOL BasicResetEvent(HANDLE hEvent)
{
	return ::ResetEvent(hEvent);
}

BOOL BasicCloseHandle(HANDLE hObject)
{
	return ::CloseHandle(hObject);
}

DWORD BasicWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
	return ::WaitForSingleObject(hHandle, dwMilliseconds);
}


#endif



__NS_BASIC_END
