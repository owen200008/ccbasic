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
	*\return TRUE加锁成功 FALSE加锁失败
	*/
	virtual BOOL Lock(DWORD dwTimeout = INFINITE);

	//! 解锁
	/*! 
	*\return TRUE解锁成功 FALSE解锁失败
	*/
	virtual BOOL Unlock() = 0;

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

class CSemaphore : public CBasicSyncObject
{

// Constructor
public:
	CSemaphore(LONG lInitialCount = 1, LONG lMaxCount = 1,
		LPCTSTR pstrName=NULL);

// Implementation
public:
	virtual ~CSemaphore();
	virtual BOOL Unlock();
	virtual BOOL Unlock(LONG lCount, LPLONG lprevCount = NULL);
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
	CMutex(BOOL bInitiallyOwn = FALSE, LPCTSTR lpszName = NULL);

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
	virtual BOOL Lock(DWORD dwTimeout = INFINITE);
#endif
	
// Implementation
public:
	virtual ~CMutex();
	BOOL Unlock();
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
	CEvent(BOOL bInitiallyOwn = FALSE, BOOL bManualReset = FALSE,
		LPCTSTR lpszNAme = NULL);

// Operations
public:
	BOOL SetEvent();
	BOOL PulseEvent();
	BOOL ResetEvent();
	BOOL Unlock();

// Implementation
public:
	virtual ~CEvent();
};

/////////////////////////////////////////////////////////////////////////////
// CCriticalSection
#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
typedef struct _RBASIC_CRITICAL_SECTION {

	DWORD OwningThread;			//!< 记录线程ID
	HANDLE LockSemaphore;		//!< 信号量对象

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
	BOOL Unlock();

	//! 加锁
	/*! 
	*\return TRUE加锁成功 FALSE加锁失败
	*/
	BOOL Lock();

	//! 加锁
	/*! 
	*\param dwTimeOut 超时时间
	*\return TRUE加锁成功 FALSE加锁失败
	*/
	BOOL Lock(DWORD dwTimeout);

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
	CSingleLock(CBasicSyncObject* pObject, BOOL bInitialLock = FALSE);

// Operations
public:
	//! 加锁
	/*! 
	*\param dwTimeOut 超时时间,单位毫秒。INFINITE表示不设超时时间
	*\return TRUE加锁成功 FALSE加锁失败
	*/
	BOOL Lock(DWORD dwTimeOut = INFINITE);

	//! 解锁
	/*! 
	*\return TRUE解锁成功 FALSE解锁失败
	*/
	BOOL Unlock();

// Implementation
public:
	~CSingleLock();

protected:
	CBasicSyncObject*	m_pObject;		//!< 同步对象
	BOOL			m_bAcquired;	//!< 是否被占用标记
};

//////////////////////////////////////////////////////////////////////////
//自旋锁
struct SpinLock
{
	int m_nLock;
	SpinLock()
	{
		m_nLock = 0;
	}
};

class CSpinLockFunc
{
public:
	CSpinLockFunc(SpinLock* pLock, BOOL bInitialLock = FALSE);
	virtual ~CSpinLockFunc();

	void Lock();
	void LockAndSleep(unsigned short usSleep = 100);
	bool LockNoWait();
	void UnLock();
	bool IsLock();
protected:
	SpinLock* 		m_pLock;
	bool			m_bAcquired;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////

//! 原子操作，变量递增
/*! 
*\param lpAddend 变量指针
*\return *lpAddend + 1
*\remarks *lpAddend = *lpAddend + 1
*/
LONG BasicInterlockedIncrement (LONG volatile *lpAddend);

//! 原子操作，变量递减
/*! 
*\param lpAddend 变量指针
*\return *lpAddend - 1
*\remarks *lpAddend = *lpAddend - 1
*/
LONG BasicInterlockedDecrement (LONG volatile *lpAddend);

//! 原子操作，变量赋值
/*! 
*\param Target	目标变量
*\param Value	赋值变量
*\return Value
*\remarks *Target = Value
*/
LONG BasicInterlockedExchange (LONG volatile *Target, LONG Value);

//! 原子操作，变量相加
/*! 
*\param Addend	目标变量
*\param Value	加操作变量
*\return *Addend初始值
*\remarks *Addend = *Addend + Value
*/
LONG BasicInterlockedExchangeAdd (LONG volatile *Addend, LONG Value);
LONG BasicInterlockedExchangeSub (LONG volatile *Addend, LONG Value);
//! 原子操作，变量比较
/*! 
*\param Destination	目标变量
*\param Exchange	赋值变量
*\param Comperand	比较变量
*\return *Destination初始值
*\remarks 如Comperand==*Destination,则执行*Destination=Exchange
*/
LONG BasicInterlockedCompareExchange (LONG volatile *Destination, LONG Exchange, LONG Comperand);

//! 创建事件对象
/*! 
*\param bManualReset	复位方式
*  <ul>
*  <li> 复位方式
*     <ol>
*     <li> TRUE		必须用BasicResetEvent手工恢复到无信号状态
*     <li> FALSE	事件被等待线程释放后，自动恢复到无信号状态
*     </ol>
*  </ul>
*\param bInitialState	初始状态，如TRUE初始为有信号状态，否则为无信号状态
*\param lpName			事件对象命名，可以是无名
*\return 事件对象句柄，如失败返回NULL
*/
HANDLE BasicCreateEvent(BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName);

//! 设置事件信号
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
BOOL BasicSetEvent(HANDLE hEvent);

//! 重置事件到无信号状态
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
BOOL BasicResetEvent(HANDLE hEvent);

//! 销毁事件信号
/*! 
*\param hEvent	事件对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
BOOL BasicDestoryEvent(HANDLE hEvent);

//! 销毁对象句柄
/*! 
*\param hObject	对象句柄
*\return 如操作成功返回非零值，否则返回0
*/
BOOL BasicCloseHandle(HANDLE hObject);


//! 等待事件信号
/*! 
*\param hHandle	对象句柄
*\param dwMilliseconds 超时时间，单位毫秒。-1为不超时。
*\return 有信号状态返回WAIT_OBJECT_0，超时返回WAIT_TIMEOUT
*/
DWORD BasicWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

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
