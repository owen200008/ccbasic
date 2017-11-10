#include "../inc/basic.h"

__NS_BASIC_START
////////////////////////////////////////////////////////////////////////////////////////////////
CSpinLockFuncNoSameThreadSafe::CSpinLockFuncNoSameThreadSafe(SpinLock* pLock, bool bInitialLock)
{
	m_pLock = pLock;
	m_bAcquired = false;
	if (bInitialLock)
		Lock();
}
CSpinLockFuncNoSameThreadSafe::~CSpinLockFuncNoSameThreadSafe()
{
	UnLock();
}

void CSpinLockFuncNoSameThreadSafe::Lock()
{
	while(m_pLock->m_nLock.exchange(1)){
	}
	m_bAcquired = true;
}
bool CSpinLockFuncNoSameThreadSafe::LockNoWait()
{
	while(m_pLock->m_nLock.exchange(1)){
		return false;
	}
	m_bAcquired = true;
	return true;
}

void CSpinLockFuncNoSameThreadSafe::LockAndSleep(unsigned short usSleep)
{
	while(m_pLock->m_nLock.exchange(1)){
		BasicSleep(usSleep);
	}
	m_bAcquired = true;
}

void CSpinLockFuncNoSameThreadSafe::UnLock()
{
	if (m_bAcquired)
	{
		m_bAcquired = false;
		m_pLock->m_nLock.exchange(0);
	}
}

bool CSpinLockFuncNoSameThreadSafe::IsLock()
{
	return m_pLock->m_nLock.load() != 0;
}

CSpinLockFunc::CSpinLockFunc(SpinLock* pLock, bool bInitialLock){
	m_pLock = pLock;
	m_bAcquired = false;
	if(bInitialLock)
		Lock();
}
CSpinLockFunc::~CSpinLockFunc(){
	UnLock();
}

void CSpinLockFunc::Lock()
{
	DWORD dwThreadID = BasicGetCurrentThreadId();
	if(m_pLock->m_nLock.exchange(1)){
		if(dwThreadID == m_pLock->m_lockThreadID)
			return;

		while(m_pLock->m_nLock.exchange(1)){}
	}
	m_bAcquired = true;
	m_pLock->m_lockThreadID = dwThreadID;
}
bool CSpinLockFunc::LockNoWait()
{
	DWORD dwThreadID = BasicGetCurrentThreadId();
	if(m_pLock->m_nLock.exchange(1)){
		if(dwThreadID == m_pLock->m_lockThreadID)
			return true;
		return false;
	}
	m_bAcquired = true;
	m_pLock->m_lockThreadID = dwThreadID;
	return true;
}

void CSpinLockFunc::LockAndSleep(unsigned short usSleep)
{
	DWORD dwThreadID = BasicGetCurrentThreadId();
	if(m_pLock->m_nLock.exchange(1)){
		if(dwThreadID == m_pLock->m_lockThreadID)
			return;
		while(m_pLock->m_nLock.exchange(1)){
			BasicSleep(usSleep);
		}
	}
	m_bAcquired = true;
	m_pLock->m_lockThreadID = BasicGetCurrentThreadId();
}

void CSpinLockFunc::UnLock(){
	if (m_bAcquired){
		m_pLock->m_lockThreadID = 0;
		m_pLock->m_nLock.exchange(0);
		m_bAcquired = false;
	}
}

__NS_BASIC_END
