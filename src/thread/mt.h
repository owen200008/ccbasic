/***********************************************************************************************
// 文件名:     mt.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:00:18
// 内容描述:   同步对象定义文件
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_MT_H
#define BASIC_MT_H

/////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

//class CBasicObject;
	class CBasicSyncObject;
		class CSemaphore;
		class CMutex;
		class CEvent;
		class CCriticalSection;

class CSingleLock;

/////////////////////////////////////////////////////////////////////////////
//! 同步对象基类
/*! 
*  
*/

class _BASIC_DLL_API CBasicSyncObject : public CBasicObject
{

// Constructor
public:
	CBasicSyncObject(LPCTSTR pstrName);

// Attributes
public:
	operator HANDLE() const;
	HANDLE  m_hObject;

	//! 加锁
	/*! 
	*\param dwTimeOut 超时时间,单位毫秒。INFINITE表示不设超时时间
	*\return true加锁成功 false加锁失败
	*/
	virtual bool Lock(DWORD dwTimeout = INFINITE);

	//! 解锁
	/*! 
	*\return true解锁成功 false解锁失败
	*/
	virtual bool Unlock() = 0;

// Implementation
public:
	virtual ~CBasicSyncObject();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif
	friend class CSingleLock;
};

/////////////////////////////////////////////////////////////////////////////
//! 信号量类
/*! 
*  
*/

class _BASIC_DLL_API CSemaphore : public CBasicSyncObject
{

// Constructor
public:
	CSemaphore(LONG lInitialCount = 1, LONG lMaxCount = 1,
		LPCTSTR pstrName=NULL);

// Implementation
public:
	virtual ~CSemaphore();
	virtual bool Unlock();
	virtual bool Unlock(LONG lCount, LPLONG lprevCount = NULL);
};

/////////////////////////////////////////////////////////////////////////////
//! 互斥量类
/*! 
*  
*/

class _BASIC_DLL_API CMutex : public CBasicSyncObject
{

// Constructor
public:
	CMutex(bool bInitiallyOwn = false, LPCTSTR lpszName = NULL);

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
	virtual bool Lock(DWORD dwTimeout = INFINITE);
#endif
	
// Implementation
public:
	virtual ~CMutex();
	bool Unlock();
};

/////////////////////////////////////////////////////////////////////////////
//! 事件类
/*! 
*  
*/

class _BASIC_DLL_API CEvent : public CBasicSyncObject
{

// Constructor
public:
	CEvent(bool bInitiallyOwn = false, bool bManualReset = false,
		LPCTSTR lpszNAme = NULL);

// Operations
public:
	bool SetEvent();
	bool PulseEvent();
	bool ResetEvent();
	bool Unlock();

// Implementation
public:
	virtual ~CEvent();
};

/////////////////////////////////////////////////////////////////////////////
// CCriticalSection
#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
typedef struct _RBASIC_CRITICAL_SECTION {
	uint32_t	m_nAcquired;
	HANDLE		LockSemaphore;		//!< 信号量对象
} RBASIC_CRITICAL_SECTION, *PRBASIC_CRITICAL_SECTION;

typedef RBASIC_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRBASIC_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRBASIC_CRITICAL_SECTION LPCRITICAL_SECTION;

#endif

//! 临界区类
/*! 
*  
*/
class _BASIC_DLL_API CCriticalSection : public CBasicSyncObject
{

// Constructor
public:
	CCriticalSection();

// Attributes
public:
	operator CRITICAL_SECTION*();
	CRITICAL_SECTION m_sect;

// Operations
public:
	bool Unlock();

	//! 加锁
	/*! 
	*\return true加锁成功 false加锁失败
	*/
	bool Lock();

	//! 加锁
	/*! 
	*\param dwTimeOut 超时时间
	*\return true加锁成功 false加锁失败
	*/
	bool Lock(DWORD dwTimeout);

// Implementation
public:
	virtual ~CCriticalSection();
};

/////////////////////////////////////////////////////////////////////////////
//! 锁对象类
/*! 
*  
*/

class _BASIC_DLL_API CSingleLock
{
// Constructors
public:
	CSingleLock(CBasicSyncObject* pObject, bool bInitialLock = false);

// Operations
public:
	//! 加锁
	/*! 
	*\param dwTimeOut 超时时间,单位毫秒。INFINITE表示不设超时时间
	*\return true加锁成功 false加锁失败
	*/
	bool Lock(DWORD dwTimeOut = INFINITE);

	//! 解锁
	/*! 
	*\return true解锁成功 false解锁失败
	*/
	bool Unlock();

// Implementation
public:
	~CSingleLock();

protected:
	CBasicSyncObject*	m_pObject;		//!< 同步对象
	bool                m_bAcquired;	//!< 是否被占用标记
};


#ifdef __BASICWINDOWS
//适用于vista以及server 2008及以上系统
#define RWLOCK_VAR					SRWLOCK
#define INIT_RWLOCK_VAR(v)			InitializeSRWLock(&(v))
#define ENTER_READ_LOCK_VAR(v)		AcquireSRWLockShared(&(v))
#define LEAVE_READ_LOCK_VAR(v)		ReleaseSRWLockShared(&(v))
#define ENTER_WRITE_LOCK_VAR(v)		AcquireSRWLockExclusive(&(v))
#define LEAVE_WRITE_LOCK_VAR(v)		ReleaseSRWLockExclusive(&(v))
#define DELETE_RWLOCK_VAR(v)  
#else
#define RWLOCK_VAR					pthread_rwlock_t
#define INIT_RWLOCK_VAR(v)			pthread_rwlock_init(&(v), NULL)
#define ENTER_READ_LOCK_VAR(v)		pthread_rwlock_rdlock(&(v))
#define LEAVE_READ_LOCK_VAR(v)		pthread_rwlock_unlock(&(v))
#define ENTER_WRITE_LOCK_VAR(v)		pthread_rwlock_wrlock(&(v))
#define LEAVE_WRITE_LOCK_VAR(v)		pthread_rwlock_unlock(&(v))
#define DELETE_RWLOCK_VAR(v)		pthread_rwlock_destroy(&(v))
#endif 

struct _BASIC_DLL_API RWLock
{
	RWLOCK_VAR	lock;
	RWLock()
	{
		INIT_RWLOCK_VAR(lock);
	}
	~RWLock()
	{
		DELETE_RWLOCK_VAR(lock);
	}
};
//RWLock
class _BASIC_DLL_API CRWLockFunc
{
public:
	CRWLockFunc(RWLock* lock, bool bInitialWLock = false, bool bInitialRLock = false)
	{
		m_pLock = lock;
		m_bAcquiredRead = false;
		m_bAcquiredWrite = false;
		if (bInitialWLock)
			LockWrite();
		if (bInitialRLock)
			LockRead();
	}
	virtual ~CRWLockFunc()
	{
		UnLockRead();
		UnLockWrite();
	}

	void LockRead()
	{
		if (!m_bAcquiredRead)
		{
			ENTER_READ_LOCK_VAR(m_pLock->lock);
			m_bAcquiredRead = true;
		}
	}
	void UnLockRead()
	{
		if (m_bAcquiredRead)
		{
			LEAVE_READ_LOCK_VAR(m_pLock->lock);
			m_bAcquiredRead = false;
		}
	}
	void LockWrite()
	{
		if (!m_bAcquiredWrite)
		{
			ENTER_WRITE_LOCK_VAR(m_pLock->lock);
			m_bAcquiredWrite = true;
		}
	}
	void UnLockWrite()
	{
		if (m_bAcquiredWrite)
		{
			LEAVE_WRITE_LOCK_VAR(m_pLock->lock);
			m_bAcquiredWrite = false;
		}
	}

protected:
	RWLock* 		m_pLock;
	bool			m_bAcquiredRead;
	bool			m_bAcquiredWrite;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////

//! 原子操作，变量递增
/*! 
*\param lpAddend 变量指针
*\return *lpAddend + 1
*\remarks *lpAddend = *lpAddend + 1
*/
_BASIC_DLL_API LONG BasicInterlockedIncrement(LONG volatile *lpAddend);

//! 原子操作，变量递减
/*! 
*\param lpAddend 变量指针
*\return *lpAddend - 1
*\remarks *lpAddend = *lpAddend - 1
*/
_BASIC_DLL_API LONG BasicInterlockedDecrement(LONG volatile *lpAddend);

//! 原子操作，变量相加
/*! 
*\param Addend	目标变量
*\param Value	加操作变量
*\return *Addend初始值
*\remarks *Addend = *Addend + Value
*/
_BASIC_DLL_API LONG BasicInterlockedExchangeAdd(LONG volatile *Addend, LONG Value);
_BASIC_DLL_API LONG BasicInterlockedExchangeSub(LONG volatile *Addend, LONG Value);
//! 原子操作，变量比较
/*! 
*\param Destination	目标变量
*\param Exchange	赋值变量
*\param Comperand	比较变量
*\return 是否相等
*\remarks 如Comperand==*Destination,则执行*Destination=Exchange
*/
_BASIC_DLL_API bool BasicInterlockedCompareExchange(LONG volatile *Destination, LONG Exchange, LONG Comperand);

//! 创建事件对象
/*! 
*\param bManualReset	复位方式
*  <ul>
*  <li> 复位方式
*     <ol>
*     <li> true		必须用BasicResetEvent手工恢复到无信号状态
*     <li> false	事件被等待线程释放后，自动恢复到无信号状态
*     </ol>
*  </ul>
*\param bInitialState	初始状态，如true初始为有信号状态，否则为无信号状态
*\param lpName			事件对象命名，可以是无名
*\return 事件对象句柄，如失败返回NULL
*/
_BASIC_DLL_API HANDLE BasicCreateEvent(bool bManualReset, bool bInitialState, LPCTSTR lpName);

//! 设置事件信号
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
_BASIC_DLL_API bool BasicSetEvent(HANDLE hEvent);

//! 重置事件到无信号状态
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
_BASIC_DLL_API bool BasicResetEvent(HANDLE hEvent);

//! 销毁事件信号
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
_BASIC_DLL_API bool BasicDestoryEvent(HANDLE hEvent);

//! 销毁对象句柄
/*! 
*\param hObject	对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
_BASIC_DLL_API bool BasicCloseHandle(HANDLE hObject);


//! 等待事件信号
/*! 
*\param hHandle	对象句柄
*\param dwMilliseconds 超时时间，单位毫秒。-1为不超时。
*\return 有信号状态返回WAIT_OBJECT_0，超时返回WAIT_TIMEOUT
*/
_BASIC_DLL_API DWORD BasicWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

//信号状态返回值定义
#ifndef WAIT_OBJECT_0
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)
#define WAIT_OBJECT_0                    ((STATUS_WAIT_0 ) + 0 )	//!< 有信号状态返回
#define WAIT_FAILED                     (DWORD)0xFFFFFFFF			//!< 信号状态失败
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)
#define WAIT_TIMEOUT                        STATUS_TIMEOUT			//!< 超时返回
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (HANDLE)-1		//!< 无效对象句柄
#endif

__NS_BASIC_END

#endif 

/////////////////////////////////////////////////////////////////////////////
